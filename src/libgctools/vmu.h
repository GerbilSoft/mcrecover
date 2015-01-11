/***************************************************************************
 * GameCube Tools Library.                                                 *
 * vmu.h: Dreamcast VMU file system definitions.                           *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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
 * This file uses information from Marcus Comstedt's
 * Dreamcast programming documentation.
 * - http://mc.pp.se/dc/vms/flashmem.html
 * - http://mc.pp.se/dc/vms/fileheader.html
 */

// All structs are packed little-endian.

#ifndef __MCRECOVER_LIBGCTOOLS_VMU_H__
#define __MCRECOVER_LIBGCTOOLS_VMU_H__

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

// Block size. VMU uses 512-byte blocks.
#define VMU_BLOCK_SIZE 512

// VMU timestamp.
// All fields are stored as BCD.
#pragma pack(1)
typedef struct PACKED _vmu_timestamp
{
	uint8_t century;	// Century, e.g. 19 or 20
	uint8_t year;		// Year within century, e.g. 15
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t day_of_week;	// 0 == Monday, 6 == Sunday
} vmu_timestamp;
#pragma pack()

// Root block. (Located at block 255)
#define VMU_ROOT_BLOCK_ADDRESS 255

// vmu_root_block.color_type
#define VMU_COLOR_STANDARD	0	/* "Standard" color. */
#define VMU_COLOR_CUSTOM	1	/* Custom color. Use the color values. */

#pragma pack(1)
typedef struct PACKED _vmu_root_block
{
	// All bytes are 0x55 to indicate a properly formatted card.
	uint8_t format55[16];

	// 0x10
	// Color.
	uint8_t color_type;	// 0 == "standard"; 1 == custom (uses the below color values)
	uint8_t color_blue;
	uint8_t color_green;
	uint8_t color_red;
	uint8_t color_alpha;	// 0 == transparent; 255 == opaque

	uint8_t reserved1[27];	// all zero

	// 0x30
	// Timestamp.
	// Indicates when the card was formatted.
	vmu_timestamp timestamp;

	uint8_t reserved2[8];	// all zero

	// 0x40
	uint16_t reserved3[3];	// unknown (FF 00 00 00 FF 00)
	uint16_t fat_addr;	// FAT location (block number) (254)
	uint16_t fat_size;	// Size of FAT, in blocks (1)
	uint16_t dir_addr;	// Directory location (block number) (253)
	uint16_t dir_size;	// Size of directory, in blocks (13)
	uint16_t icon;		// VMS icon. (0-123) [TODO: Document!]
	uint16_t user_blocks;	// Total number of user blocks. (200)
	uint16_t reserved4[3];	// unknown (1F 00 00 00 80 00)
	uint8_t reserved5[424];	// all zero
} vmu_root_block;
#pragma pack()

/**
 * FAT block.
 * Usually located at block 254, but the actual address is
 * specified by the root block.
 *
 * NOTE: If the card is larger than 256 blocks (128 KB),
 * more than one FAT block may be present; however,
 * Sega never released a VMU with partitions larger than
 * 128 KB. (The 4x card simulated four 128 KB cards.)
 */
#define VMU_FAT_BLOCK_UNALLOCATED	0xFFFC
#define VMU_FAT_BLOCK_LAST_IN_FILE	0xFFFA

#pragma pack(1)
typedef struct PACKED _vmu_fat
{
	uint16_t fat[256];
} vmu_fat;
#pragma pack()

// Directory block.
// Usually starts at block 253.
// Note that the directory grows *down*, as do most files.
// Block 253 is dir block 0, block 252 is dir block 1, etc.

// Filetype
#define VMU_DIR_FILETYPE_NONE 0x00
#define VMU_DIR_FILETYPE_DATA 0x33
#define VMU_DIR_FILETYPE_GAME 0x33

// Copy-protected
#define VMU_DIR_COPY_OK		0x00
#define VMU_DIR_COPY_PROTECTED	0xFF

#define VMU_DIR_ENTRY_LEN	0x20
#define VMU_FILENAME_LEN	12

#pragma pack(1)
typedef struct PACKED _vmu_dir_entry
{
	uint8_t filetype;	// 0x00 == none, 0x33 == data, 0xCC == game
	uint8_t protect;	// 0x00 == copy ok, 0xFF == copy protected
	uint16_t address;	// Location of first block.
	char filename[VMU_FILENAME_LEN];	// Filename. (Not NULL-terminated)
	vmu_timestamp ctime;	// File creation time. (TODO: Not mtime?)
	uint16_t size;		// File size, in blocks.
	uint16_t header_addr;	// Offset of header (in blocks) from file start.
	uint8_t reserved[4];	// all zero
} vmu_dir_entry;
#pragma pack()

/**
 * VMU file header.
 * Located at block 0 in data files, and block 1 in game files.
 * Not present in ICONDATA_VMS.
 *
 * NOTE: Text encodings vary depending on system region.
 * There's no way to determine the system region based on
 * the card data, so it has to be determined heuristically.
 */
#define VMU_FILE_HEADER_LEN 96
#pragma pack(1)
typedef struct PACKED _vmu_file_header
{
	char desc_vmu[16];	// File description (VMU file menu) [JIS X 0201?]
	char desc_dc[32];	// File description (DC file menu) (cp1252 or Shift-JIS)
	char prg_name[16];	// Name of application that created the file. (cp1252?)
				// (NOTE: This seems to be blank in most files...)
	uint16_t icon_count;	// Number of icons. (>1 == animated)
	uint16_t icon_speed;	// Icon animation speed.
	uint16_t eyecatch_type;	// Graphic eyecatch type.
	uint16_t crc;		// CRC. (Ignored for game files.)
	uint32_t size;		// Size of actual file data, excluding header, icon(s), and eyecatch.
				// (Ignored for game files.)
	uint8_t reserved[20];	// all zero
} vmu_file_header;
#pragma pack()

// If icons are present, the file header is immediately followed by
// a 16-color ARGB4444 palette and the icons.

// Icon size.
#define VMU_ICON_W	32
#define VMU_ICON_H	32

// VMU icon palette: 16 colors, ARGB4444
#define VMU_ICON_PALETTE_LEN 32
#pragma pack(1)
typedef struct PACKED _vmu_icon_palette
{
	uint16_t palette[16];	// ARGB4444
	// Alpha: 0 == transparent, 15 == opaque
} vmu_icon_palette;
#pragma pack()

// VMU icon data: 32x32, 16-color palette
#define VMU_ICON_DATA_LEN 512
#pragma pack(1)
typedef struct PACKED _vmu_icon_data
{
	uint8_t icon[32*32/2];
} vmu_icon_data;
#pragma pack()

// If an eyecatch is present, it follows the icons.
// All colors are ARGB4444

// VMU eyecatch size.
#define VMU_EYECATCH_W	72
#define VMU_EYECATCH_H	56

// VMU eyecatch types.
#define VMU_FILE_EYECATCH_NONE		0
#define VMU_FILE_EYECATCH_TRUECOLOR	1
#define VMU_FILE_EYECATCH_PALETTE_256	2
#define VMU_FILE_EYECATCH_PALETTE_16	3

// If eyecatch is "none", then there's no data.
#define VMU_EYECATCH_NONE_LEN 0

// VMU eyecatch data: 72x56, ARGB4444 true color
#define VMU_EYECATCH_TRUECOLOR_LEN 8064
#pragma pack(1)
typedef struct PACKED _vmu_eyecatch_truecolor
{
	// Alpha: 0 == transparent, 15 == opaque
	uint16_t eyecatch[72*56];	// ARGB4444
} vmu_eyecatch_truecolor;
#pragma pack()

// VMU eyecatch data: 72x56, 256-color palette
#define VMU_EYECATCH_PALETTE_256_LEN 4544
#pragma pack(1)
typedef struct PACKED _vmu_eyecatch_palette_256
{
	// Alpha: 0 == transparent, 15 == opaque
	uint16_t palette[256];	// ARGB4444
	uint8_t eyecatch[72*56];
} vmu_eyecatch_palette_256;
#pragma pack()

// VMU eyecatch data: 72x56, 16-color palette
#define VMU_EYECATCH_PALETTE_16_LEN 2048
#pragma pack(1)
typedef struct PACKED _vmu_eyecatch_palette_16
{
	// Alpha: 0 == transparent, 15 == opaque
	uint16_t palette[16];	// ARGB4444
	uint8_t eyecatch[72*56/2];
} vmu_eyecatch_palette_16;
#pragma pack()

/**
 * VMU file header: ICONDATA_VMS
 * Located at block 0.
 *
 * The color icon uses the same format as VMU icons:
 * vmu_icon_palette, followed by vmu_icon_data.
 */
#define VMU_FILE_HEADER_LEN 96
#pragma pack(1)
typedef struct PACKED _vmu_card_icon_header
{
	char desc_vmu[16];	// File description (VMU file menu) [JIS X 0201?]
	uint32_t icon_mono;	// Offset of monochrome icon.
	uint32_t icon_color;	// Offset of color icon. (If 0, no color icon is present.)
} vmu_card_icon_header;
#pragma pack()

// VMU card icon data: 32x32, monochrome
#define VMU_ICON_DATA_LEN 512
#pragma pack(1)
typedef struct PACKED _vmu_card_icon_data
{
	uint8_t icon[32*32/8];
} vmu_card_icon_data;
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* __MCRECOVER_LIBGCTOOLS_VMU_H__ */
