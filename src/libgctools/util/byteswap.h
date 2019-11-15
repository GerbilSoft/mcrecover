/***************************************************************************
 * GameCube Tools Library.                                                 *
 * byteswap.h: Byteswapping functions.                                     *
 *                                                                         *
 * Copyright (c) 2008-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBGCTOOLS_UTIL_BYTESWAP_H__
#define __LIBGCTOOLS_UTIL_BYTESWAP_H__

/* C includes. */
#include <stdint.h>

/* Get the system byte order. */
#include "byteorder.h"

#if defined(_MSC_VER)

/* Use the MSVC byteswap intrinsics. */
#include <stdlib.h>
#define __swab16(x) _byteswap_ushort(x)
#define __swab32(x) _byteswap_ulong(x)
#define __swab64(x) _byteswap_uint64(x)

#elif defined(__GNUC__)

/* Use the gcc byteswap intrinsics. */
#define __swab16(x) __builtin_bswap16(x)
#define __swab32(x) __builtin_bswap32(x)
#define __swab64(x) __builtin_bswap64(x)

#else

/* Use the macros. */
#warning No intrinsics defined for this compiler. Byteswapping may be slow.

#define __swab16(x) ((uint16_t)(((x) << 8) | ((x) >> 8)))

#define __swab32(x) \
	((uint32_t)(((x) << 24) | ((x) >> 24) | \
		(((x) & 0x0000FF00UL) << 8) | \
		(((x) & 0x00FF0000UL) >> 8)))

#define __swab64(x) \
	((uint64_t)(((x) << 56) | ((x) >> 56) | \
		(((x) & 0x000000000000FF00ULL) << 40) | \
		(((x) & 0x0000000000FF0000ULL) << 24) | \
		(((x) & 0x00000000FF000000ULL) << 8) | \
		(((x) & 0x000000FF00000000ULL) >> 8) | \
		(((x) & 0x0000FF0000000000ULL) >> 24) | \
		(((x) & 0x00FF000000000000ULL) >> 40)))
#endif

#if SYS_BYTEORDER == SYS_LIL_ENDIAN
	#define be16_to_cpu(x)	__swab16(x)
	#define be32_to_cpu(x)	__swab32(x)
	#define be64_to_cpu(x)	__swab64(x)
	#define le16_to_cpu(x)	(x)
	#define le32_to_cpu(x)	(x)
	#define le64_to_cpu(x)	(x)

	#define cpu_to_be16(x)	__swab16(x)
	#define cpu_to_be32(x)	__swab32(x)
	#define cpu_to_be64(x)	__swab64(x)
	#define cpu_to_le16(x)	(x)
	#define cpu_to_le32(x)	(x)
	#define cpu_to_le64(x)	(x)
#else /* SYS_BYTEORDER == SYS_BIG_ENDIAN */
	#define be16_to_cpu(x)	(x)
	#define be32_to_cpu(x)	(x)
	#define be64_to_cpu(x)	(x)
	#define le16_to_cpu(x)	__swab16(x)
	#define le32_to_cpu(x)	__swab32(x)
	#define le64_to_cpu(x)	__swab64(x)

	#define cpu_to_be16(x)	(x)
	#define cpu_to_be32(x)	(x)
	#define cpu_to_be64(x)	(x)
	#define cpu_to_le16(x)	__swab16(x)
	#define cpu_to_le32(x)	__swab32(x)
	#define cpu_to_le64(x)	__swab64(x)
#endif

#endif /* __LIBGCTOOLS_UTIL_BYTESWAP_H__ */
