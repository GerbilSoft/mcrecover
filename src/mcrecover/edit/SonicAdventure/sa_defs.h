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

// Save file data.
#define SA_SAVE_FILE_LEN 1184
#pragma pack(1)
typedef struct PACKED _sa_save_file
{
	uint32_t crc;		// CRC (only low 16 bits are used)
	uint32_t playTime;	// Play time (1/60ths of a second)

	// Best scores.
	sa_scores scores;

	// Best times.
	// NOTE: Big does not have times.
	union {
		sa_time_code bestTimes[28];
		struct {
			sa_time_code sonic[10];
			sa_time_code tails[5];
			sa_time_code knuckles[5];
			sa_time_code amy[3];
			sa_time_code gamma[5];
		} times;
	};

	// Best weights. (Big only)
	// Measured in 10s of grams.
	// Three weights are stored per level.
	uint16_t bestWeights[4][3];

	// unknown
	uint8_t reserved1[16];

	// Best rings.
	union {
		uint16_t bestRings[32];
		struct {
			uint16_t sonic[10];
			uint16_t tails[5];
			uint16_t knuckles[5];
			uint16_t amy[3];
			uint16_t gamma[5];
			uint16_t big[4];
		} rings;
	};

	// Mini-Game: Best scores.
	// Three scores per mini-game.
	uint32_t miniGameScores[9][3];

	// Mini-Game: Best times. (Twinkle Circuit)
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

	// Mini-Game: Best times. (Boss)
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

	// unknown
	uint8_t reserved2[17];

	// Options: Choose the feature you want to edit.
	uint8_t options;

	// Lives. (1 entry per character)
	uint8_t lives[6];

	// unknown
	uint8_t reserved3[584];
} sa_save_file;
#pragma pack()
	
#ifdef __cplusplus
}
#endif

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SA_DEFS_H__ */
