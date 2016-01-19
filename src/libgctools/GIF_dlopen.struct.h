/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GIF_dlopen.h: giflib dlopen() wrapper. (struct definitions)             *
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

#ifndef __LIBGCTOOLS_GIF_DLOPEN_STRUCT_H__
#define __LIBGCTOOLS_GIF_DLOPEN_STRUCT_H__

#include "GIF_dlopen.h"

#ifdef __cplusplus
extern "C" {
#endif

// giflib function pointers.
typedef struct _giflib_t {
	int version;

	// TODO: Split into multiple structs if a lot of these
	// functions are the same?
	union {
		struct {
			// giflib-5.1 function pointers.
			// Mostly the same as giflib-5.0, but with
			// extra "error" parameters.
			GifFileType *(*EGifOpenFileName)(const char *GifFileName,
							 const bool GifTestExistence, int *Error);
			GifFileType *(*EGifOpenFileHandle)(const int GifFileHandle, int *Error);
			GifFileType *(*EGifOpen)(void *userPtr, OutputFunc writeFunc, int *Error);
			const char *(*EGifGetGifVersion)(GifFileType *GifFile);
			int (*EGifCloseFile)(GifFileType *GifFile, int *ErrorCode);

			void (*EGifSetGifVersion)(GifFileType *GifFile, const bool gif89);
			int (*EGifPutExtensionLeader)(GifFileType *GifFile, const int GifExtCode);
			int (*EGifPutExtensionBlock)(GifFileType *GifFile,
						     const int GifExtLen, const void *GifExtension);
			int (*EGifPutExtensionTrailer)(GifFileType *GifFile);

			const char *(*GifErrorString)(int ErrorCode);
		} v51;

		// TODO: v50-specific

		struct {
			// giflib-4.2 function pointers.
			// TODO: Verify that these are the same as 4.1 and 4.0.
			GifFileType *(*EGifOpenFileName)(const char *GifFileName,
							 bool GifTestExistance);
			GifFileType *(*EGifOpenFileHandle)(int GifFileHandle);
			GifFileType *(*EGifOpen)(void *userPtr, OutputFunc writeFunc);
			void (*EGifSetGifVersion)(const char *Version);
			int (*EGifCloseFile)(GifFileType * GifFile);

			int (*EGifPutExtensionFirst)(GifFileType * GifFile, int GifExtCode,
						     int GifExtLen, const void *GifExtension);
			int (*EGifPutExtensionNext)(GifFileType * GifFile, int GifExtCode,
						    int GifExtLen, const void *GifExtension);
			int (*EGifPutExtensionLast)(GifFileType * GifFile, int GifExtCode,
						    int GifExtLen, const void *GifExtension);

			int (*GifError)(void);
			char *(*GifErrorString)(void);
			int *(*GifLastError)(void);
		} v42;
	};

	struct {
		// Common function pointers that are identical
		// from giflib-4.2 through giflib-5.1.
		// (TODO: Check giflib-4.1.)
		int (*EGifSpew)(GifFileType * GifFile);

		int (*EGifPutScreenDesc)(GifFileType *GifFile,
					 const int GifWidth, const int GifHeight, 
					 const int GifColorRes,
					 const int GifBackGround,
					 const ColorMapObject *GifColorMap);
		int (*EGifPutImageDesc)(GifFileType *GifFile, 
					const int GifLeft, const int GifTop,
					const int GifWidth, const int GifHeight, 
					const bool GifInterlace,
					const ColorMapObject *GifColorMap);
		int (*EGifPutLine)(GifFileType *GifFile, GifPixelType *GifLine,
				int GifLineLen);
		int (*EGifPutPixel)(GifFileType *GifFile, const GifPixelType GifPixel);
		int (*EGifPutComment)(GifFileType *GifFile, const char *GifComment);
		int (*EGifPutExtension)(GifFileType *GifFile, const int GifExtCode, 
				     const int GifExtLen,
				     const void *GifExtension);
		int (*EGifPutCode)(GifFileType *GifFile, int GifCodeSize,
				const GifByteType *GifCodeBlock);
		int (*EGifPutCodeNext)(GifFileType *GifFile,
				    const GifByteType *GifCodeBlock);

		// NOTE: These functions exist in v4.2, v5.0, and v5.1;
		// however, their names were changed to Gif* in v5.0.
		// Arguments and functionality remain the same.
		ColorMapObject *(*GifMakeMapObject)(int ColorCount,
						    const GifColorType *ColorMap);
		void (*GifFreeMapObject)(ColorMapObject *Object);
		ColorMapObject *(*GifUnionColorMap)(const ColorMapObject *ColorIn1,
						    const ColorMapObject *ColorIn2,
						    GifPixelType ColorTransIn2[]);
		int (*GifBitSize)(int n);
	} common;
} giflib_t;

#ifdef __cplusplus
}
#endif

#endif /* __LIBGCTOOLS_GIF_DLOPEN_STRUCT_H__ */
