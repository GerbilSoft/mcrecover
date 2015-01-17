/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * sa_defs.h: Sonic Adventure - structure definitions.                     *
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
 * This file uses information from MainMemory's SAsave editor.
 * - https://github.com/sonicretro/sa_tools/tree/master/SASave
 *
 * Additional references:
 * - http://rnhart.net/articles/sa-savefile.htm
 * - http://info.sonicretro.org/SCHG:Sonic_Adventure/Main_Save_File
 */

// All structs are packed little-endian.

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SA_DEFS_H__
#define __MCRECOVER_EDIT_SONICADVENTURE_SA_DEFS_H__

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

// Sonic Adventure (Dreamcast)
// Starting address of the save files within the data area.
#define SA_SAVE_ADDRESS_DC_0 0x0480
#define SA_SAVE_ADDRESS_DC_1 0x0920
#define SA_SAVE_ADDRESS_DC_2 0x0DC0

// Sonic Adventure DX (GameCube)
// Starting address of the save file within the data area.
// This does NOT include the GCI header. (0x40 bytes)
#define SA_SAVE_ADDRESS_GCN 0x1440

// TODO: Byteswapping for unions with structs?

// 3-byte timecode.
#pragma pack(1)
typedef struct PACKED _sa_time_code
{
	union {
		uint8_t data[3];
		struct {
			uint8_t minutes;
			uint8_t seconds;
			uint8_t frames;
		};
	};
} sa_time_code;
#pragma pack()

// Score data.
#define SA_SCORES_LEN 128
#pragma pack(1)
typedef union PACKED _sa_scores {
	uint32_t all[32];
	struct {
		uint32_t sonic[10];
		uint32_t tails[5];
		uint32_t knuckles[5];
		uint32_t amy[3];
		uint32_t gamma[5];
		uint32_t big[4];
	};
} sa_scores;
#pragma pack()

// Time data.
#define SA_TIMES_LEN 84
#pragma pack(1)
typedef union PACKED _sa_times {
	// NOTE: Big does not have times.
	sa_time_code all[28];
	struct {
		sa_time_code sonic[10];
		sa_time_code tails[5];
		sa_time_code knuckles[5];
		sa_time_code amy[3];
		sa_time_code gamma[5];
	} times;
} sa_times;
#pragma pack()

// Best weights. (Big only)
// Measured in 10s of grams.
// Three weights are stored per level.
#define SA_WEIGHTS_LEN 24
#pragma pack(1)
typedef union PACKED _sa_weights {
	// NOTE: Big does not have times.
	uint16_t all[12];
	uint16_t levels[4][3];
} sa_weights;
#pragma pack()

// Best rings.
#define SA_RINGS_LEN 64
#pragma pack(1)
typedef union PACKED _sa_rings {
	uint16_t all[32];
	struct {
		uint16_t sonic[10];
		uint16_t tails[5];
		uint16_t knuckles[5];
		uint16_t amy[3];
		uint16_t gamma[5];
		uint16_t big[4];
	};
} sa_rings;
#pragma pack()

/**
 * Last character / menu voice.
 * Voice is indicated by V*.
 */
typedef enum {
	SA_LAST_CHAR_SONIC		= 0,
	SA_LAST_CHAR_SONIC_VDEFAULT	= 1,
	SA_LAST_CHAR_TAILS		= 2,
	SA_LAST_CHAR_KNUCKLES		= 3,
	SA_LAST_CHAR_SONIC_VDEFAULT2	= 4,
	SA_LAST_CHAR_AMY		= 5,
	SA_LAST_CHAR_GAMMA		= 6,
	SA_LAST_CHAR_BIG		= 7,
	SA_LAST_CHAR_SONIC_VEGGMAN	= 8,
	SA_LAST_CHAR_SONIC_VTIKAL	= 9,
} sa_last_char;

// Event Flags.
#define SA_EVENT_FLAGS_LEN 64
#define SA_EVENT_FLAGS_SECTIONS 8
#pragma pack(1)
typedef struct PACKED _sa_event_flags {
	union {
		uint8_t all[64];	// all
		uint8_t chr[8][8];	// per character
		struct {
			uint8_t unused[8];
			uint8_t general[8];
			uint8_t sonic[8];
			uint8_t tails[8];
			uint8_t knuckles[8];
			uint8_t amy[8];
			uint8_t gamma[8];
			uint8_t big[8];
		};
	};
} sa_event_flags;
#pragma pack()

// Adventure Mode data.
#define SA_ADVENTURE_MODE_LEN 96
#pragma pack(1)
typedef struct PACKED _sa_adventure_mode {
	// One set per character.
	struct {
		uint8_t time_of_day;
		uint8_t unused;
		uint16_t unknown1;
		uint16_t unknown2;
		uint16_t start_entrance;
		uint16_t start_level_and_act;
		uint16_t unknown3;
	} chr[8];
} sa_adventure_mode;
#pragma pack()

// Level clear count.
#define SA_LEVEL_CLEAR_COUNT_LEN 344
#pragma pack(1)
typedef struct PACKED _sa_level_clear_count {
	// All levels are available for all characters.
	uint8_t clear[8][43];
} sa_level_clear_count;
#pragma pack()

// Save slot.
// GCN files have a single slot.
// Dreamcast files have three slots.
#define SA_SAVE_SLOT_LEN 1184
#pragma pack(1)
typedef struct PACKED _sa_save_slot
{
	uint32_t crc;		// CRC (only low 16 bits are used)
	uint32_t playTime;	// Play time (1/60ths of a second)

	sa_scores scores;	// [0x008] Best scores.
	sa_times times;		// [0x088] Best times. (Does NOT include Big.)
	sa_weights weights;	// [0x0DC] Best weights. (Big only)
	uint8_t reserved1[16];	// [0x0F4] unknown
	sa_rings rings;		// [0x104] Best rings.

	// [0x144] Mini-Game: Best scores.
	// Three scores per mini-game.
	uint32_t miniGameScores[9][3];

	// [0x1B0] Mini-Game: Best times. (Twinkle Circuit)
	// Five times are stored per character:
	// - Best Time 1
	// - Best Time 2
	// - Best Time 3
	// - Lap 1 for Best Time 1
	// - Lap 2 for Best Time 1
	union {
		sa_time_code twinkleCircuitBestTimes[30];
		struct {
			sa_time_code sonic[5];
			sa_time_code tails[5];
			sa_time_code knuckles[5];
			sa_time_code amy[5];
			sa_time_code gamma[5];
			sa_time_code big[5];
		} twinkleCircuitTimes;
	};

	// [0x20A] Mini-Game: Best times. (Boss)
	// Thre times are stored per character.
	union {
		sa_time_code bossBestTimes[18];
		struct {
			sa_time_code sonic[3];
			sa_time_code tails[3];
			sa_time_code knuckles[3];
			sa_time_code amy[3];
			sa_time_code gamma[3];
			sa_time_code big[3];
		} bossTimes;
	};

	// [0x240] Emblems. (bitfield)
	uint8_t emblems[17];

	// [0x251] Options: Choose the feature you want to edit.
	uint8_t options;

	// [0x252] Lives. (1 entry per character)
	// Order: Sonic, Tails, Knuckles, Amy, Gamma, Big, Super Sonic
	uint8_t lives[7];

	uint8_t last_char;	// [0x259] Last character / menu voice.
	uint8_t rumble;		// [0x25A] Rumble feature
	uint8_t reserved3;	// [0x25B] unknown
	uint16_t last_level;	// [0x25C] Last completed level. (100 == none)
	uint8_t reserved4[2];	// [0x25E] unknown

	// [0x260] Event flags. (bitfield)
	sa_event_flags events;

	// [0x2A0] NPC flags. (bitfield)
	uint8_t npc_flags[64];

	uint8_t reserved5[8];	// [0x2E0] unknown

	// [0x2E8] Adventure mode data. (one per character)
	sa_adventure_mode adventure_mode;

	// [0x348] Level clear count.
	sa_level_clear_count clear_count;
} sa_save_slot;
#pragma pack()
	
#ifdef __cplusplus
}
#endif

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SA_DEFS_H__ */
