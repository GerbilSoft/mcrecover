/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * byteswap.h: Byteswapping functions.                                     *
 *                                                                         *
 * Copyright (c) 2008-2011 by David Korth                                  *
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

#ifndef __MCRECOVER_BYTESWAP_H__
#define __MCRECOVER_BYTESWAP_H__

// NOTE: This file is generated at compile-time.
// It's located in the binary directory.
#include "util/byteorder.h"

#define __swab16(x) (((x) << 8) | ((x) >> 8))

#define __swab32(x) \
	(((x) << 24) | ((x) >> 24) | \
		(((x) & 0x0000FF00UL) << 8) | \
		(((x) & 0x00FF0000UL) >> 8))

#define __swab64(x) \
	(((x) << 56) | ((x) >> 56) | \
		(((x) & 0x000000000000FF00ULL) << 40) | \
		(((x) & 0x0000000000FF0000ULL) << 24) | \
		(((x) & 0x00000000FF000000ULL) << 8) | \
		(((x) & 0x000000FF00000000ULL) >> 8) | \
		(((x) & 0x0000FF0000000000ULL) >> 24) | \
		(((x) & 0x00FF000000000000ULL) >> 40))

#if MCRECOVER_BYTEORDER == MCRECOVER_LIL_ENDIAN
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
#else /* MCRECOVER_BYTEORDER == MCRECOVER_BIG_ENDIAN */
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

#endif /* __MCRECOVER_BYTESWAP_H__ */
