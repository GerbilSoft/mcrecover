/***************************************************************************
 * GameCube Tools Library.                                                 *
 * banner.h: GameCube banner definitions.                                  *
 * Contains definitions for opening.bnr.                                   *
 *                                                                         *
 * Copyright (c) 2013-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

/**
 * Reference:
 * - http://hitmen.c02.at/files/yagcd/yagcd/chap14.html
 */

#pragma once

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
#define BANNER_MAGIC_BNR1 0x424E5231	/* 'BNR1' */
#define BANNER_MAGIC_BNR2 0x424E5232	/* 'BNR2' */

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
	uint16_t banner[0x1800>>1];	// Banner image. (96x32, RGB5A3)
	banner_comment_t comment;
} banner_bnr1_t;
#pragma pack()

// BNR2
#pragma pack(1)
typedef struct PACKED _banner_bnr2_t
{
	uint32_t magic;			// BANNER_MAGIC_BNR2
	uint8_t reserved[0x1C];
	uint16_t banner[0x1800>>1];	// Banner image. (96x32, RGB5A3)
	banner_comment_t comments[6];
} banner_bnr2_t;
#pragma pack()

/**
 * WIBN (Wii Banner)
 * Reference: http://wiibrew.org/wiki/Savegame_Files
 * NOTE: This may be located at one of two places:
 * - 0x0000: banner.bin extracted via SaveGame Manager GX
 * - 0x0020: Savegame extracted via Wii System Menu
 */

// Magic numbers.
#define BANNER_WIBN_MAGIC		0x5749424E	/* 'WIBN' */
#define BANNER_WIBN_ADDRESS_RAW		0x0000		/* banner.bin from SaveGame Manager GX */
#define BANNER_WIBN_ADDRESS_ENCRYPTED	0x0020		/* extracted from Wii System Menu */

// Flags.
#define BANNER_WIBN_FLAGS_NOCOPY	0x01
#define BANNER_WIBN_FLAGS_ICON_BOUNCE	0x10

// Banner size.
#define BANNER_WIBN_IMAGE_W 192
#define BANNER_WIBN_IMAGE_H 64

// Icon size.
#define BANNER_WIBN_ICON_W 48
#define BANNER_WIBN_ICON_H 48

// Struct size.
#define BANNER_WIBN_ICON_SIZE 0x1200
#define BANNER_WIBN_STRUCT_SIZE 24736
#define BANNER_WIBN_STRUCT_SIZE_ICONS(icons) \
	(BANNER_WIBN_STRUCT_SIZE + ((icons)*BANNER_WIBN_ICON_SIZE))

#pragma pack(1)
typedef struct PACKED _banner_wibn_t
{
	uint32_t magic;			// BANNER_MAGIC_WIBN
	uint32_t flags;
	uint16_t iconDelay;		// Similar to GCN.
	uint8_t reserved[22];
	uint16_t gameTitle[32];		// Game title. (UTF-16 BE)
	uint16_t gameSubTitle[32];	// Game subtitle. (UTF-16 BE)
	uint16_t banner[0x6000>>1];	// Banner image. (192x64, RGB5A3)
	uint16_t icon[8][0x1200>>1];	// Icons. (48x48, RGB5A3) [optional]
} banner_wibn_t;
#pragma pack()

#ifdef __cplusplus
}
#endif
