/***************************************************************************
 * GameCube Tools Library.                                                 *
 * banner.h: GameCube banner definitions.                                  *
 * Contains definitions for opening.bnr.                                   *
 *                                                                         *
 * Copyright (c) 2013 by David Korth.                                      *
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

/**
 * Reference:
 * - http://hitmen.c02.at/files/yagcd/yagcd/chap14.html
 */

#ifndef __LIBGCTOOLS_BANNER_H__
#define __LIBGCTOOLS_BANNER_H__

#include <stdint.h>

// Packed struct attribute.
#if !defined(PACKED)
#if defined(__GNUC__)
#define PACKED __attribute__ ((packed))
#else
#define PACKED
#endif /* defined(__GNUC__) */
#endif /* !defined(PACKED) */

#ifdef __cplusplus
extern "C" {
#endif

// Magic numbers.
#define BANNER_MAGIC_BNR1 0x424E5231
#define BANNER_MAGIC_BNR2 0x424E5232

// Banner size.
#define BANNER_IMAGE_W 96
#define BANNER_IMAGE_H 32

// NOTE: Strings are encoded in either cp1252 or Shift-JIS,
// depending on the game region.

// Banner comment.
#pragma pack(1)
typedef struct PACKED _banner_comment_t
{
	char gamename[0x20];
	char company[0x20];
	char gamename_full[0x40];
	char company_full[0x40];
	char gamedesc[0x80];
} banner_comment_t;

// BNR1
#pragma pack(1)
typedef struct PACKED _banner_bnr1_t
{
	uint32_t magic;			// BANNER_MAGIC_BNR1
	uint8_t reserved[0x1C];
	uint16_t image[0x1800>>1];	// RGB5A3
	banner_comment_t comment;
} banner_bnr1_t;
#pragma pack()

// BNR2
#pragma pack(1)
typedef struct PACKED _banner_bnr2_t
{
	uint32_t magic;			// BANNER_MAGIC_BNR2
	uint8_t reserved[0x1C];
	uint16_t image[0x1800>>1];	// RGB5A3
	banner_comment_t comments[6];
} banner_bnr2_t;
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* __LIBGCTOOLS_BANNER_H__ */
