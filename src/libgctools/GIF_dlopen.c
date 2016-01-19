/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GIF_dlopen.h: giflib dlopen() wrapper.                                  *
 *                                                                         *
 * Copyright (c) 2014-2016 by David Korth.                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "config.libgctools.h"
#include "GIF_dlopen.h"

// C includes.
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef _WIN32
// Unix dlopen()
#include <dlfcn.h>
#else
// Windows LoadLibrary()
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#define dlopen(filename, flag)	LoadLibraryA(filename)
#define dlsym(handle, symbol)	((void*)GetProcAddress(handle, symbol))
#define dlclose(handle)		FreeLibrary(handle)
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef USE_INTERNAL_GIF
// DLL handle.
static void *giflib_dll = NULL;

// giflib structs.
#include "GIF_dlopen.struct.h"
static giflib_t giflib;

/**
 * Have we tried dlopen() yet?
 */
static bool have_tried_dlopen = false;

/**
 * Set GIF standard to GIF89a.
 * This is for compatibility with giflib-4.2,
 * which has a global GIF89a setting.
 */
static bool use_gif89a = false;

/**
 * GIF extension code state.
 * Used for compatibility with giflib-4.2's EGifPutExtension*() functions.
 * TODO: Associate gif_ext_code with a GifFile.
 * Note that giflib-4.2 isn't thread-safe, so this isn't
 * entirely necessary...
 */
static struct {
	int GifExtCode;
	bool first;
} ext_state;

/**
 * Attempt to initialize giflib.
 * @return 1 if giflib is available; 0 if not. (TODO: Error codes?)
 */
static int init_giflib(void)
{
	// Check for giflib .so versions 7 through 3.
	// This corresponds to 5.1, 5.0, 4.2, 4.1, and 4.0.
	// NOTE: .so version 5 doesn't exist (4.2 is still v4),
	char giflib_dll_filename[32];
	int i;

	if (giflib_dll) {
		// giflib is already open.
		return 1;
	}

	// External GIF library.
	giflib.version = GIFLIB_UNKNOWN;
	for (i = 7; i >= 3; i--) {
		if (i == 5) {
			// .so version 5 doesn't exist.
			continue;
		}

		// TODO: On Windows, it might just be called giflib.dll.
		// Identifying the actual version may be tricky.
#ifdef _WIN32
#error External giflib is not currently supported on Windows.
#endif
		snprintf(giflib_dll_filename, sizeof(giflib_dll_filename),
			 "libgif.so.%d", i);
		giflib_dll = dlopen(giflib_dll_filename, RTLD_LOCAL|RTLD_NOW);
		if (giflib_dll) {
			// Found giflib.
			giflib.version = i;
			if (i == GIFLIB_41) {
				// Check for giflib-4.2.
				if (dlsym(giflib_dll, "GifErrorString") != NULL &&
				    dlsym(giflib_dll, "GifLastError") != NULL)
				{
					// This is giflib-4.2.
					giflib.version = GIFLIB_42;
				}
			}
			break;
		}
	}

	if (!giflib_dll) {
		// Could not open giflib.
		// TODO: Return dlopen() error?
		return 0;
	}

	// Load common GIF symbols.
#define DLOPEN_GIF(section, symname) do { \
		/* FIXME: Casting. */ \
		giflib.section.symname = dlsym(giflib_dll, #symname); \
		if (!giflib.section.symname) { \
			/* Error loading a required symbol. */ \
			/* TODO: Try the next available soname? */ \
			giflib.version = GIFLIB_UNKNOWN; \
			dlclose(giflib_dll); \
			giflib_dll = NULL; \
			return 0; /* TODO: dlsym() error code? */ \
		} \
} while (0)
#define DLOPEN_GIF_NAME(section, symname, dllsymname) do { \
		/* FIXME: Casting. */ \
		giflib.section.symname = dlsym(giflib_dll, #dllsymname); \
		if (!giflib.section.symname) { \
			/* Error loading a required symbol. */ \
			/* TODO: Try the next available soname? */ \
			giflib.version = GIFLIB_UNKNOWN; \
			dlclose(giflib_dll); \
			giflib_dll = NULL; \
			return 0; /* TODO: dlsym() error code? */ \
		} \
} while (0)

	DLOPEN_GIF(common, EGifSpew);
	DLOPEN_GIF(common, EGifPutScreenDesc);
	DLOPEN_GIF(common, EGifPutLine);
	DLOPEN_GIF(common, EGifPutPixel);
	DLOPEN_GIF(common, EGifPutComment);
	DLOPEN_GIF(common, EGifPutExtension);
	DLOPEN_GIF(common, EGifPutCode);
	DLOPEN_GIF(common, EGifPutCodeNext);

	// EGifPutImageDesc is slightly different on v4.2+.
	if (giflib.version >= 42) {
		DLOPEN_GIF(v42_bool, EGifPutImageDesc);
	} else {
		DLOPEN_GIF(v40_int, EGifPutImageDesc);
	}
	
	// Common GIF symbols that were renamed in giflib-5.0.
	if (giflib.version >= 50) {
		DLOPEN_GIF(common, GifMakeMapObject);
		DLOPEN_GIF(common, GifFreeMapObject);
		DLOPEN_GIF(common, GifUnionColorMap);
		DLOPEN_GIF(common, GifBitSize);
	} else {
		DLOPEN_GIF_NAME(common, GifMakeMapObject, MakeMapObject);
		DLOPEN_GIF_NAME(common, GifFreeMapObject, FreeMapObject);
		DLOPEN_GIF_NAME(common, GifUnionColorMap, UnionColorMap);
		DLOPEN_GIF_NAME(common, GifBitSize, BitSize);
	}

	switch (giflib.version) {
		case GIFLIB_51:
			// giflib-5.1
			DLOPEN_GIF(v51, EGifOpenFileName);
			DLOPEN_GIF(v51, EGifOpenFileHandle);
			DLOPEN_GIF(v51, EGifOpen);
			DLOPEN_GIF(v51, EGifGetGifVersion);
			DLOPEN_GIF(v51, EGifCloseFile);
			DLOPEN_GIF(v51, EGifSetGifVersion);
			DLOPEN_GIF(v51, EGifPutExtensionLeader);
			DLOPEN_GIF(v51, EGifPutExtensionBlock);
			DLOPEN_GIF(v51, EGifPutExtensionTrailer);
			DLOPEN_GIF(v51, GifErrorString);
			break;

		case GIFLIB_50:
			// giflib-5.0
			DLOPEN_GIF(v50, EGifOpenFileName);
			DLOPEN_GIF(v50, EGifOpenFileHandle);
			DLOPEN_GIF(v50, EGifOpen);
			DLOPEN_GIF(v50, EGifGetGifVersion);
			DLOPEN_GIF(v50, EGifCloseFile);
			DLOPEN_GIF(v50, EGifSetGifVersion);
			DLOPEN_GIF(v50, EGifPutExtensionLeader);
			DLOPEN_GIF(v50, EGifPutExtensionBlock);
			DLOPEN_GIF(v50, EGifPutExtensionTrailer);
			DLOPEN_GIF(v50, GifErrorString);
			break;

		case GIFLIB_42:
			// giflib-4.2
			DLOPEN_GIF(v42, EGifOpenFileName);
			DLOPEN_GIF(v42, EGifOpenFileHandle);
			DLOPEN_GIF(v42, EGifOpen);
			DLOPEN_GIF(v42, EGifSetGifVersion);
			DLOPEN_GIF(v42, EGifCloseFile);
			DLOPEN_GIF(v42, EGifPutExtensionFirst);
			DLOPEN_GIF(v42, EGifPutExtensionNext);
			DLOPEN_GIF(v42, EGifPutExtensionLast);
			DLOPEN_GIF(v42, GifError);
			DLOPEN_GIF(v42, GifErrorString);
			DLOPEN_GIF(v42, GifLastError);
			break;

		case GIFLIB_41:
		case GIFLIB_40:
			// giflib-4.0, 4.1
			DLOPEN_GIF(v40, EGifOpenFileName);
			DLOPEN_GIF(v40, EGifOpenFileHandle);
			DLOPEN_GIF(v40, EGifOpen);
			DLOPEN_GIF(v40, EGifSetGifVersion);
			DLOPEN_GIF(v40, EGifCloseFile);
			DLOPEN_GIF(v40, EGifPutExtensionFirst);
			DLOPEN_GIF(v40, EGifPutExtensionNext);
			DLOPEN_GIF(v40, EGifPutExtensionLast);
			DLOPEN_GIF(v40, GifError);
			break;

		default:
			// Unknown giflib.
			giflib.version = GIFLIB_UNKNOWN;
			dlclose(giflib_dll);
			giflib_dll = NULL;
			return 0;
	}

	return 1;
}

/**
 * Check what version of giflib is available.
 * This function will load giflib if it hasn't been loaded yet.
 * @return giflib version (51, 50, 42, 41, 40), or 0 if giflib isn't available.
 */
int GifDlVersion(void)
{
	if (!have_tried_dlopen) {
		init_giflib();
		have_tried_dlopen = true;
	}

	// TODO: Use a lookup table?
	switch (giflib.version) {
		case GIFLIB_51: return 51;
		case GIFLIB_50: return 50;
		case GIFLIB_42: return 42;
		case GIFLIB_41: return 41;
		case GIFLIB_40: return 40;

		case GIFLIB_UNKNOWN:
		default:        return 0;
	}
}

/**
 * Get the UserData pointer from the specified GifFile.
 * @param GifFile GIF file.
 * @return UserData pointer.
 */
void *GifDlGetUserData(const GifFileType *GifFile)
{
	switch (giflib.version) {
		case GIFLIB_51:
		case GIFLIB_50:
			return ((const GifFileType_v50*)GifFile)->UserData;
		case GIFLIB_42:
		case GIFLIB_41:
		case GIFLIB_40:
			return ((const GifFileType_v40*)GifFile)->UserData;
		default:
			break;
	}

	// Unknown version of GIFLIB.
	return NULL;
}

// NOTE: No NULL pointer checking is done in these functions.
// GifDlVersion() MUST be called once on startup to determine
// if giflib is available. If it isn't available, do NOT call
// these functions!

/** Common giflib functions. **/

int EGifDlSpew(GifFileType * GifFile)
{
	return giflib.common.EGifSpew(GifFile);
}

int EGifDlPutScreenDesc(GifFileType *GifFile,
			const int GifWidth, const int GifHeight, 
			const int GifColorRes,
			const int GifBackGround,
			const ColorMapObject *GifColorMap)
{
	return giflib.common.EGifPutScreenDesc(GifFile, GifWidth, GifHeight,
			GifColorRes, GifBackGround, GifColorMap);
}

int EGifDlPutImageDesc(GifFileType *GifFile, 
		       const int GifLeft, const int GifTop,
		       const int GifWidth, const int GifHeight, 
		       const bool GifInterlace,
		       const ColorMapObject *GifColorMap)
{
	// Slight difference introduced in v4.2
	// due to switch from 'int' to 'bool'.
	if (giflib.version >= GIFLIB_42) {
		return giflib.v42_bool.EGifPutImageDesc(GifFile, GifLeft, GifTop,
				GifWidth, GifHeight, GifInterlace, GifColorMap);
	} else {
		return giflib.v40_int.EGifPutImageDesc(GifFile, GifLeft, GifTop,
				GifWidth, GifHeight, GifInterlace, GifColorMap);
	}
}

int EGifDlPutLine(GifFileType *GifFile, GifPixelType *GifLine,
		int GifLineLen)
{
	return giflib.common.EGifPutLine(GifFile, GifLine, GifLineLen);
}

int EGifDlPutPixel(GifFileType *GifFile, const GifPixelType GifPixel)
{
	return giflib.common.EGifPutPixel(GifFile, GifPixel);
}

int EGifDlPutComment(GifFileType *GifFile, const char *GifComment)
{
	return giflib.common.EGifPutComment(GifFile, GifComment);
}

int EGifDlPutExtension(GifFileType *GifFile, const int GifExtCode, 
		     const int GifExtLen,
		     const void *GifExtension)
{
	return giflib.common.EGifPutExtension(GifFile, GifExtCode,
			GifExtLen, GifExtension);
}

int EGifDlPutCode(GifFileType *GifFile, int GifCodeSize,
		const GifByteType *GifCodeBlock)
{
	return giflib.common.EGifPutCode(GifFile, GifCodeSize, GifCodeBlock);
}

int EGifDlPutCodeNext(GifFileType *GifFile,
		    const GifByteType *GifCodeBlock)
{
	return giflib.common.EGifPutCodeNext(GifFile, GifCodeBlock);
}

// NOTE: These functions exist in v4.2, v5.0, and v5.1;
// however, their names were changed to Gif* in v5.0.
// Arguments and functionality remain the same.

ColorMapObject *GifDlMakeMapObject(int ColorCount,
				   const GifColorType *ColorMap)
{
	return giflib.common.GifMakeMapObject(ColorCount, ColorMap);
}

void GifDlFreeMapObject(ColorMapObject *Object)
{
	giflib.common.GifFreeMapObject(Object);
}

ColorMapObject *GifDlUnionColorMap(const ColorMapObject *ColorIn1,
				   const ColorMapObject *ColorIn2,
				   GifPixelType ColorTransIn2[])
{
	return giflib.common.GifUnionColorMap(ColorIn1, ColorIn2, ColorTransIn2);
}

int GifDlBitSize(int n)
{
	return giflib.common.GifBitSize(n);
}

/** Functions that differ in various versions of giflib. **/

GifFileType *EGifDlOpenFileName(const char *GifFileName,
				const bool GifTestExistence, int *Error)
{
	switch (giflib.version) {
		case GIFLIB_51:
			return giflib.v51.EGifOpenFileName(
				GifFileName, GifTestExistence, Error);
		case GIFLIB_50:
			return giflib.v50.EGifOpenFileName(
				GifFileName, GifTestExistence, Error);
		case GIFLIB_42: {
			GifFileType *ret = giflib.v42.EGifOpenFileName(
				GifFileName, GifTestExistence);
			if (!ret && Error) {
				*Error = giflib.v42.GifError();
			}
			return ret;
		}

		case GIFLIB_41:
		case GIFLIB_40: {
			GifFileType *ret = giflib.v40.EGifOpenFileName(
				GifFileName, GifTestExistence);
			if (!ret && Error) {
				*Error = giflib.v40.GifError();
			}
			return ret;
		}

		default:
			break;
	}

	if (Error) {
		*Error = E_GIF_ERR_OPEN_FAILED;
	}
	return NULL;
}

GifFileType *EGifDlOpenFileHandle(const int GifFileHandle, int *Error)
{
	switch (giflib.version) {
		case GIFLIB_51:
			return giflib.v51.EGifOpenFileHandle(
				GifFileHandle, Error);
		case GIFLIB_50:
			return giflib.v50.EGifOpenFileHandle(
				GifFileHandle, Error);
		case GIFLIB_42: {
			GifFileType *ret = giflib.v42.EGifOpenFileHandle(
				GifFileHandle);
			if (!ret && Error) {
				*Error = giflib.v42.GifError();
			}
			return ret;
		}

		case GIFLIB_41:
		case GIFLIB_40: {
			GifFileType *ret = giflib.v40.EGifOpenFileHandle(
				GifFileHandle);
			if (!ret && Error) {
				*Error = giflib.v40.GifError();
			}
			return ret;
		}

		default:
			break;
	}

	if (Error) {
		*Error = E_GIF_ERR_OPEN_FAILED;
	}
	return NULL;
}

GifFileType *EGifDlOpen(void *userPtr, OutputFunc writeFunc, int *Error)
{
	switch (giflib.version) {
		case GIFLIB_51:
			return giflib.v51.EGifOpen(
				userPtr, writeFunc, Error);
		case GIFLIB_50:
			return giflib.v50.EGifOpen(
				userPtr, writeFunc, Error);
		case GIFLIB_42: {
			GifFileType *ret = giflib.v42.EGifOpen(
				userPtr, writeFunc);
			if (!ret && Error) {
				*Error = giflib.v42.GifError();
			}
			return ret;
		}

		case GIFLIB_41:
		case GIFLIB_40: {
			GifFileType *ret = giflib.v40.EGifOpen(
				userPtr, writeFunc);
			if (!ret && Error) {
				*Error = giflib.v40.GifError();
			}
			return ret;
		}

		default:
			break;
	}

	if (Error) {
		*Error = E_GIF_ERR_OPEN_FAILED;
	}
	return NULL;
}

const char *EGifDlGetGifVersion(GifFileType *GifFile)
{
	switch (giflib.version) {
		case GIFLIB_51:
			return giflib.v51.EGifGetGifVersion(GifFile);
		case GIFLIB_50:
			return giflib.v50.EGifGetGifVersion(GifFile);
		default:
			// giflib-4.2 and earlier have a global GIF version.
			return (use_gif89a ? GIF89_STAMP : GIF87_STAMP);
	}
}

int EGifDlCloseFile(GifFileType *GifFile, int *ErrorCode)
{
	switch (giflib.version) {
		case GIFLIB_51:
			return giflib.v51.EGifCloseFile(
				GifFile, ErrorCode);

		case GIFLIB_50: {
			int ret = giflib.v50.EGifCloseFile(GifFile);
			if (ret != GIF_OK) {
				// Get the error code, then free GifFile.
				if (ErrorCode) {
					*ErrorCode = ((GifFileType_v50*)GifFile)->Error;
				}
				// TODO: EGifCloseFile() again so we
				// don't free() across DLL boundaries?
				free(GifFile);
			}
			return ret;
		}

		case GIFLIB_42: {
			int ret = giflib.v42.EGifCloseFile(GifFile);
			if (ret != GIF_OK && ErrorCode) {
				*ErrorCode = giflib.v42.GifError();
			}
			return ret;
		}

		case GIFLIB_40:
		case GIFLIB_41: {
			int ret = giflib.v40.EGifCloseFile(GifFile);
			if (ret != GIF_OK && ErrorCode) {
				*ErrorCode = giflib.v40.GifError();
			}
			return ret;
		}

		default:
			break;
	}

	if (ErrorCode) {
		*ErrorCode = E_GIF_ERR_CLOSE_FAILED;
	}
	return GIF_ERROR;
}

void EGifDlSetGifVersion(GifFileType *GifFile, const bool gif89)
{
	use_gif89a = gif89;
	switch (giflib.version) {
		case GIFLIB_51:
			giflib.v51.EGifSetGifVersion(GifFile, gif89);
			break;
		case GIFLIB_50:
			giflib.v50.EGifSetGifVersion(GifFile, gif89);
			break;
		case GIFLIB_42:
			giflib.v42.EGifSetGifVersion(gif89 ? "89a" : "87a");
			break;
		case GIFLIB_41:
		case GIFLIB_40:
			giflib.v42.EGifSetGifVersion(gif89 ? "89a" : "87a");
			break;
		default:
			break;
	}
}

int EGifDlPutExtensionLeader(GifFileType *GifFile, const int GifExtCode)
{
	switch (giflib.version) {
		case GIFLIB_51:
			return giflib.v51.EGifPutExtensionLeader(GifFile, GifExtCode);
		case GIFLIB_50:
			return giflib.v50.EGifPutExtensionLeader(GifFile, GifExtCode);
		case GIFLIB_42:
		case GIFLIB_41:
		case GIFLIB_40:
			// Don't do anything yet.
			ext_state.GifExtCode = GifExtCode;
			ext_state.first = true;
			return GIF_OK;
		default:
			break;
	}

	return GIF_ERROR;
}

int EGifDlPutExtensionBlock(GifFileType *GifFile,
			    const int GifExtLen, const void *GifExtension)
{
	switch (giflib.version) {
		case GIFLIB_51:
			return giflib.v51.EGifPutExtensionBlock(GifFile, GifExtLen, GifExtension);
		case GIFLIB_50:
			return giflib.v50.EGifPutExtensionBlock(GifFile, GifExtLen, GifExtension);

		case GIFLIB_42: {
			int ret;
			if (ext_state.first) {
				// First extension block.
				ret = giflib.v42.EGifPutExtensionFirst(GifFile,
					ext_state.GifExtCode, GifExtLen, GifExtension);
				ext_state.first = false;
			} else {
				// Next extension block.
				ret = giflib.v42.EGifPutExtensionNext(GifFile,
					ext_state.GifExtCode, GifExtLen, GifExtension);
			}
			return ret;
		}

		case GIFLIB_41:
		case GIFLIB_40: {
			int ret;
			if (ext_state.first) {
				// First extension block.
				ret = giflib.v40.EGifPutExtensionFirst(GifFile,
					ext_state.GifExtCode, GifExtLen, GifExtension);
				ext_state.first = false;
			} else {
				// Next extension block.
				ret = giflib.v40.EGifPutExtensionNext(GifFile,
					ext_state.GifExtCode, GifExtLen, GifExtension);
			}
			return ret;
		}

		default:
			break;
	}

	return GIF_ERROR;
}

int EGifDlPutExtensionTrailer(GifFileType *GifFile)
{
	switch (giflib.version) {
		case GIFLIB_51:
			return giflib.v51.EGifPutExtensionTrailer(GifFile);
		case GIFLIB_50:
			return giflib.v50.EGifPutExtensionTrailer(GifFile);
		case GIFLIB_42:
			return giflib.v42.EGifPutExtensionLast(GifFile,
					ext_state.GifExtCode, 0, NULL);
		case GIFLIB_41:
		case GIFLIB_40:
			return giflib.v40.EGifPutExtensionLast(GifFile,
					ext_state.GifExtCode, 0, NULL);
		default:
			break;
	}

	return GIF_ERROR;
}

const char *GifDlErrorString(int ErrorCode)
{
	switch (giflib.version) {
		case GIFLIB_51:
			return giflib.v51.GifErrorString(ErrorCode);
		case GIFLIB_50:
			return giflib.v50.GifErrorString(ErrorCode);
		case GIFLIB_42:
		case GIFLIB_41:
		case GIFLIB_40:
		default:
			// TODO: giflib-4.2 only provides the error string
			// for the last error. v4.1 and v4.0 don't have
			// error strings at all.
			break;
	}

	return "Unknown";
}

#endif /* USE_INTERNAL_GIF */
