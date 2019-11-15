/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GIF_dlopen.h: giflib dlopen() wrapper. (struct definitions)             *
 *                                                                         *
 * Copyright (c) 2014-2019 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBGCTOOLS_GIF_DLOPEN_STRUCT_H__
#define __LIBGCTOOLS_GIF_DLOPEN_STRUCT_H__

#include "GIF_dlopen.h"

#ifdef __cplusplus
extern "C" {
#endif

// giflib version enum.
// Value maps to the soname.
typedef enum {
	GIFLIB_UNKNOWN = -1,

	GIFLIB_51 = 7,	// also used for 5.2
	GIFLIB_50 = 6,
	GIFLIB_42 = 5,	// actually 4, but 5 is unused
	GIFLIB_41 = 4,
	GIFLIB_40 = 3,  // TODO
} GifLib_Version_t;

// giflib function pointers.
typedef struct _giflib_t {
	GifLib_Version_t version;

	// TODO: Split into multiple structs if a lot of these
	// functions are the same?
	union {
		struct {
			// giflib-5.1 function pointers.
			// Similar to giflib-5.0, except EGifCloseFile has an ErrorCode parameter.
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

		struct {
			// giflib-5.0 function pointers.
			// Now with better error handling.
			GifFileType *(*EGifOpenFileName)(const char *GifFileName,
							 const bool GifTestExistence, int *Error);
			GifFileType *(*EGifOpenFileHandle)(const int GifFileHandle, int *Error);
			GifFileType *(*EGifOpen)(void *userPtr, OutputFunc writeFunc, int *Error);
			const char *(*EGifGetGifVersion)(GifFileType *GifFile);
			int (*EGifCloseFile)(GifFileType *GifFile);

			void (*EGifSetGifVersion)(GifFileType *GifFile, const bool gif89);
			int (*EGifPutExtensionLeader)(GifFileType *GifFile, const int GifExtCode);
			int (*EGifPutExtensionBlock)(GifFileType *GifFile,
						     const int GifExtLen, const void *GifExtension);
			int (*EGifPutExtensionTrailer)(GifFileType *GifFile);

			const char *(*GifErrorString)(int ErrorCode);
		} v50;

		struct {
			// giflib-4.2 function pointers.
			// Similar to v4.1, except it has additional error checking functions.
			// v4.1 also used VoidPtr for the void* arguments, which expanded to
			// char* on SYSV and void* everywhere else.
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

		struct {
			// giflib-4.0, 4.1 function pointers.
			// v4.1 added const qualifiers to some pointer arguments.
			// Note that const qualifiers don't break the ABI.
			GifFileType *(*EGifOpenFileName)(const char *GifFileName,
							 int GifTestExistance);
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
		} v40;
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

	union {
		// Function pointers that use 'bool' on v4.2+ and 'int' on v4.0 and v4.1.
		// TODO: Is 'bool' guaranteed to be the same size as 'int' when used as a
		// function parameter on all systems?
		// If so, this can be merged into common...
		struct {
			int (*EGifPutImageDesc)(GifFileType *GifFile,
						const int GifLeft, const int GifTop,
						const int GifWidth, const int GifHeight, 
						const bool GifInterlace,
						const ColorMapObject *GifColorMap);
		} v42_bool;

		struct {
			int (*EGifPutImageDesc)(GifFileType *GifFile,
						const int GifLeft, const int GifTop,
						const int GifWidth, const int GifHeight, 
						const int GifInterlace,
						const ColorMapObject *GifColorMap);
		} v40_int;
	};
} giflib_t;

// Opaque forward declarations.
typedef struct SavedImage SavedImage;

/**
 * ColorMapObject from giflib-5.2.1.
 * Compatible with v5.0.x, v5.1.x, v5.2.x.
 */
typedef struct ColorMapObject_v50 {
    int ColorCount;
    int BitsPerPixel;
    bool SortFlag;
    GifColorType *Colors;    /* on malloc(3) heap */
} ColorMapObject_v50;

/**
 * ColorMapObject from giflib-4.2.3.
 * Compatible with v4.0.x, v4.1.x, v4.2.x.
 */
typedef struct ColorMapObject_v40 {
    int ColorCount;
    int BitsPerPixel;
    GifColorType *Colors;    /* on malloc(3) heap */
} ColorMapObject_v40;

/**
 * GifImageDesc from giflib-5.2.1.
 * Compatible with v5.0.x, v5.1.x, v5.2.x.
 */
typedef struct GifImageDesc_v50 {
    GifWord Left, Top, Width, Height;   /* Current image dimensions. */
    bool Interlace;                     /* Sequential/Interlaced lines. */
    ColorMapObject_v50 *ColorMap;       /* The local color map */
} GifImageDesc_v50;

/**
 * GifImageDesc from giflib-4.1.
 * Compatible with v4.0.x, v4.1.x, v4.2.x.
 */
typedef struct GifImageDesc_v40 {
    GifWord Left, Top, Width, Height,   /* Current image dimensions. */
      Interlace;                    /* Sequential/Interlaced lines. */
    ColorMapObject_v40 *ColorMap;   /* The local color map */
} GifImageDesc_v40;

/**
 * GifFileType from giflib-5.2.1
 * Compatible with v5.0.x, v5.1.x, v5.2.x.
 */
typedef struct GifFileType_v50 {
    GifWord SWidth, SHeight;         /* Size of virtual canvas */
    GifWord SColorResolution;        /* How many colors can we generate? */
    GifWord SBackGroundColor;        /* Background color for virtual canvas */
    GifByteType AspectByte;	     /* Used to compute pixel aspect ratio */
    ColorMapObject_v50 *SColorMap;   /* Global colormap, NULL if nonexistent. */
    int ImageCount;                  /* Number of current image (both APIs) */
    GifImageDesc_v50 Image;          /* Current image (low-level API) */
    SavedImage *SavedImages;         /* Image sequence (high-level API) */
    int ExtensionBlockCount;         /* Count extensions past last image */
    ExtensionBlock *ExtensionBlocks; /* Extensions past last image */    
    int Error;			     /* Last error condition reported */
    void *UserData;                  /* hook to attach user data (TVT) */
    void *Private;                   /* Don't mess with this! */
} GifFileType_v50;

/**
 * GifFileType from giflib-4.2.
 * Compatible with v4.0.x, v4.1.x, v4.2.x.
 * NOTE: 4.0.x and 4.1.x originally used VoidPtr for the
 * UserData and Private fields. VoidPtr was defined to
 * char* on SYSV, and void* elsewhere. We're using void*,
 * since that's what later giflib uses.
 */
typedef struct GifFileType_v40 {
    GifWord SWidth, SHeight,        /* Screen dimensions. */
      SColorResolution,         /* How many colors can we generate? */
      SBackGroundColor;         /* I hope you understand this one... */
    ColorMapObject_v40 *SColorMap;  /* NULL if not exists. */
    int ImageCount;             /* Number of current image */
    GifImageDesc_v40 Image;     /* Block describing current image */
    struct SavedImage *SavedImages; /* Use this to accumulate file state */
    void *UserData;            /* hook to attach user data (TVT) */
    void *Private;             /* Don't mess with this! */
} GifFileType_v40;

#ifdef __cplusplus
}
#endif

#endif /* __LIBGCTOOLS_GIF_DLOPEN_STRUCT_H__ */
