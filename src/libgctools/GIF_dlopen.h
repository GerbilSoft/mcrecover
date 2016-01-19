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

#ifndef __LIBGCTOOLS_GIF_DLOPEN_H__
#define __LIBGCTOOLS_GIF_DLOPEN_H__

#ifdef _GIF_LIB_H_
#error Do not include gif_lib.h when using GIF_dlopen.h
#endif /* _GIF_LIB_H_ */

#include "config.libgctools.h"

/**
 * Get the GIFLIB version.
 * @return GIFLIB version, e.g. 51, 50, 42, 41, 40, or 0 if not available.
 */
#ifdef USE_INTERNAL_GIF
#include <gif_lib.h>
#define GifDlVersion() (GIFLIB_MAJOR * 10 + GIFLIB_MINOR)
#define GifDlGetUserData(GifFile) (GifFile)->UserData
#define GifDlGetColorMapArray(Object) (Object)->Colors
#define GifDlGetColorMapCount(Object) (Object)->ColorCount
#define GifDlSetColorMapCount(Object, colorCount) (Object)->ColorCount = (colorCount)
#endif /* USE_INTERNAL_GIF */

// API is based on giflib-5.1.2.
// Older versions of giflib will have compatibility functions
// applied to make thme work like newer versions.

// If we're using the internal giflib, we don't want to use
// the dlopen() redirection. Instead, #define the dlopen()
// functions to simply use the standard functions.
// TODO: Define the rest of gif_lib.h's functions.

#ifndef USE_INTERNAL_GIF
#if !defined(_MSC_VER) || _MSC_VER >= 1800
/* MSVC: stdbool.h was first available in MSVC 2013. */
/* Other: Assume stdbool.h is available. */
#include <stdbool.h>
#else
/* Older MSVC; stdbool.h is not available. */
#ifndef __cplusplus
typedef unsigned char bool;
#define true 1
#define false 0
#endif /* __cplusplus */
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define GIF_ERROR   0
#define GIF_OK      1

#define GIF_STAMP "GIFVER"          /* First chars in file - GIF stamp.  */
#define GIF_STAMP_LEN sizeof(GIF_STAMP) - 1
#define GIF_VERSION_POS 3           /* Version first character in stamp. */
#define GIF87_STAMP "GIF87a"        /* First chars in file - GIF stamp.  */
#define GIF89_STAMP "GIF89a"        /* First chars in file - GIF stamp.  */

typedef unsigned char GifPixelType;
typedef unsigned char *GifRowType;
typedef unsigned char GifByteType;
typedef unsigned int GifPrefixType;
typedef int GifWord;

typedef struct GifColorType {
    GifByteType Red, Green, Blue;
} GifColorType;

typedef struct ExtensionBlock {
    int ByteCount;
    GifByteType *Bytes; /* on malloc(3) heap */
    int Function;       /* The block function code */
#define CONTINUE_EXT_FUNC_CODE    0x00    /* continuation subblock */
#define COMMENT_EXT_FUNC_CODE     0xfe    /* comment */
#define GRAPHICS_EXT_FUNC_CODE    0xf9    /* graphics control (GIF89) */
#define PLAINTEXT_EXT_FUNC_CODE   0x01    /* plaintext */
#define APPLICATION_EXT_FUNC_CODE 0xff    /* application block */
} ExtensionBlock;

/**
 * Check what version of giflib is available.
 * This function will load giflib if it hasn't been loaded yet.
 * @return giflib version (51, 50, 42, 41, 40), or 0 if giflib isn't available.
 */
int GifDlVersion(void);

// Opaque types for compatibility purposes.
typedef struct GifFileType GifFileType;
typedef struct ColorMapObject ColorMapObject;

/**
 * Get the UserData pointer from the specified GifFile.
 * @param GifFile GIF file.
 * @return UserData pointer.
 */
void *GifDlGetUserData(const GifFileType *GifFile);

/**
 * Get the Colors[] array from a ColorMapObject.
 * @param Object ColorMapObject.
 * @return Colors[] array.
 */
GifColorType *GifDlGetColorMapArray(ColorMapObject *Object);

/**
 * Get the color count from a ColorMapObject.
 * @param Object ColorMapObject.
 * @return Color count.
 */
int GifDlGetColorMapCount(const ColorMapObject *Object);

/**
 * Set the color count in a ColorMapObject.
 * @param Object ColorMapObject.
 * @param colorCount Color count.
 */
void GifDlSetColorMapCount(ColorMapObject *Object, int colorCount);

/* func type to read gif data from arbitrary sources (TVT) */
typedef int (*InputFunc) (GifFileType *GifFile, GifByteType *buf, int len);

/* func type to write gif data to arbitrary targets.
 * Returns count of bytes written. (MRB)
 */
typedef int (*OutputFunc) (GifFileType *GifFile, const GifByteType *buf, int len);

/******************************************************************************
 GIF encoding routines
******************************************************************************/

/* Main entry points */
GifFileType *EGifDlOpenFileName(const char *GifFileName,
				const bool GifTestExistence, int *Error);
GifFileType *EGifDlOpenFileHandle(const int GifFileHandle, int *Error);
GifFileType *EGifDlOpen(void *userPtr, OutputFunc writeFunc, int *Error);
int EGifDlSpew(GifFileType * GifFile);
const char *EGifDlGetGifVersion(GifFileType *GifFile); /* new in 5.x */
int EGifDlCloseFile(GifFileType *GifFile, int *ErrorCode);

#ifndef _IN_GIF_DLOPEN_C_
#define E_GIF_SUCCEEDED          0
#define E_GIF_ERR_OPEN_FAILED    1    /* And EGif possible errors. */
#define E_GIF_ERR_WRITE_FAILED   2
#define E_GIF_ERR_HAS_SCRN_DSCR  3
#define E_GIF_ERR_HAS_IMAG_DSCR  4
#define E_GIF_ERR_NO_COLOR_MAP   5
#define E_GIF_ERR_DATA_TOO_BIG   6
#define E_GIF_ERR_NOT_ENOUGH_MEM 7
#define E_GIF_ERR_DISK_IS_FULL   8
#define E_GIF_ERR_CLOSE_FAILED   9
#define E_GIF_ERR_NOT_WRITEABLE  10
#endif /* _IN_GIF_DLOPEN_C_ */

/* These are legacy.  You probably do not want to call them directly */
int EGifDlPutScreenDesc(GifFileType *GifFile,
			const int GifWidth, const int GifHeight, 
			const int GifColorRes,
			const int GifBackGround,
			const ColorMapObject *GifColorMap);
int EGifDlPutImageDesc(GifFileType *GifFile, 
		       const int GifLeft, const int GifTop,
		       const int GifWidth, const int GifHeight, 
		       const bool GifInterlace,
		       const ColorMapObject *GifColorMap);
void EGifDlSetGifVersion(GifFileType *GifFile, const bool gif89);
int EGifDlPutLine(GifFileType *GifFile, GifPixelType *GifLine,
                int GifLineLen);
int EGifDlPutPixel(GifFileType *GifFile, const GifPixelType GifPixel);
int EGifDlPutComment(GifFileType *GifFile, const char *GifComment);
int EGifDlPutExtensionLeader(GifFileType *GifFile, const int GifExtCode);
int EGifDlPutExtensionBlock(GifFileType *GifFile,
			    const int GifExtLen, const void *GifExtension);
int EGifDlPutExtensionTrailer(GifFileType *GifFile);
int EGifDlPutExtension(GifFileType *GifFile, const int GifExtCode, 
		       const int GifExtLen,
		       const void *GifExtension);
int EGifDlPutCode(GifFileType *GifFile, int GifCodeSize,
		  const GifByteType *GifCodeBlock);
int EGifDlPutCodeNext(GifFileType *GifFile,
		      const GifByteType *GifCodeBlock);

/******************************************************************************
 Error handling and reporting.
******************************************************************************/
extern const char *GifDlErrorString(int ErrorCode);     /* new in 2012 - ESR */

/*****************************************************************************
 Everything below this point is new after version 1.2, supporting `slurp
 mode' for doing I/O in two big belts with all the image-bashing in core.
******************************************************************************/

/******************************************************************************
 Color map handling from gif_alloc.c
******************************************************************************/

extern ColorMapObject *GifDlMakeMapObject(int ColorCount,
					  const GifColorType *ColorMap);
extern void GifDlFreeMapObject(ColorMapObject *Object);
extern ColorMapObject *GifDlUnionColorMap(const ColorMapObject *ColorIn1,
					  const ColorMapObject *ColorIn2,
					  GifPixelType ColorTransIn2[]);
extern int GifDlBitSize(int n);

#ifdef __cplusplus
}
#endif

#else /* USE_INTERNAL_GIF */

// Using internal giflib.
// Define macros for the Dl functions that redirect them
// to the standard functions instead.

/* Main entry points */
#define EGifDlOpenFileName(GifFileName, GifTestExistence, Error) \
	EGifOpenFileName(GifFileName, GifTestExistence, Error)
#define EGifDlOpenFileHandle(GifFileHandle, Error) \
	EGifOpenFileHandle(GifFileHandle, Error)
#define EGifDlOpen(userPtr, writeFunc, Error) \
	EGifOpen(userPtr, writeFunc, Error)
#define EGifDlSpew(GifFile) \
	EGifSpew(GifFile)
#define EGifDlGetGifVersion(GifFile) \
	EGifGetGifVersion(GifFile)
#define EGifDlCloseFile(GifFile, ErrorCode) \
	EGifCloseFile(GifFile, ErrorCode)

/* These are legacy.  You probably do not want to call them directly */
#define EGifDlPutScreenDesc(GifFile, GifWidth, GifHeight, GifColorRes, GifBackGround, GifColorMap) \
	EGifPutScreenDesc(GifFile, GifWidth, GifHeight, GifColorRes, GifBackGround, GifColorMap)
#define EGifDlPutImageDesc(GifFile, GifLeft, GifTop, GifWidth, GifHeight, GifInterlace, GifColorMap) \
	EGifPutImageDesc(GifFile, GifLeft, GifTop, GifWidth, GifHeight, GifInterlace, GifColorMap)
#define EGifDlSetGifVersion(GifFile, gif89) \
	EGifSetGifVersion(GifFile, gif89)
#define EGifDlPutLine(GifFile, GifLine, GifLineLen) \
	EGifPutLine(GifFile, GifLine, GifLineLen)
#define EGifDlPutPixel(GifFile, GifPixel) \
	EGifPutPixel(GifFile, GifPixel)
#define EGifDlPutComment(GifFile, GifComment) \
	EGifPutComment(GifFile, GifComment)
#define EGifDlPutExtensionLeader(GifFile, GifExtCode) \
	EGifPutExtensionLeader(GifFile, GifExtCode)
#define EGifDlPutExtensionBlock(GifFile, GifExtLen, GifExtension) \
	EGifPutExtensionBlock(GifFile, GifExtLen, GifExtension)
#define EGifDlPutExtensionTrailer(GifFile) \
	EGifPutExtensionTrailer(GifFile)
#define EGifDlPutExtension(GifFile, GifExtCode, GifExtLen, GifExtension) \
	EGifPutExtension(GifFile, GifExtCode, GifExtLen, GifExtension)
#define EGifDlPutCode(GifFile, GifCodeSize, GifCodeBlock) \
	EGifPutCode(GifFile, GifCodeSize, GifCodeBlock)
#define EGifDlPutCodeNext(GifFile, GifCodeBlock) \
	EGifDlPutCodeNext(GifFile, GifCodeBlock)

/******************************************************************************
 Error handling and reporting.
******************************************************************************/
#define GifDlErrorString(ErrorCode) \
	GifErrorString(ErrorCode)

/*****************************************************************************
 Everything below this point is new after version 1.2, supporting `slurp
 mode' for doing I/O in two big belts with all the image-bashing in core.
******************************************************************************/

/******************************************************************************
 Color map handling from gif_alloc.c
******************************************************************************/

#define GifDlMakeMapObject(ColorCount, ColorMap) \
	GifMakeMapObject(ColorCount, ColorMap)
#define GifDlFreeMapObject(Object) \
	GifFreeMapObject(Object)
#define GifDlUnionColorMap(ColorIn1, ColorIn2, ColorTransIn2) \
	GifUnionColorMap(ColorIn1, ColorIn2, ColorTransIn2)
#define GifDlBitSize(n) \
	GifBitSize(n)

#endif /* USE_INTERNAL_GIF */

#endif /* __LIBGCTOOLS_GIF_DLOPEN_H__ */
