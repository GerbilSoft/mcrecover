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
					i = GIFLIB_42;
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
#endif /* USE_INTERNAL_GIF */
