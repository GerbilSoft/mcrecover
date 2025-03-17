/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * sa_defs.h: Sonic Adventure - structure definitions.                     *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

/**
 * This file uses information from MainMemory's SAsave editor.
 * - https://github.com/sonicretro/sa_tools/tree/master/SASave
 *
 * Additional references:
 * - https://rnhart.net/articles/sa-savefile.htm
 * - https://info.sonicretro.org/SCHG:Sonic_Adventure/Main_Save_File
 */

// All structs are in little-endian.

#ifndef __LIBSAVEEDIT_SONICADVENTURE_SA_DEFS_H__
#define __LIBSAVEEDIT_SONICADVENTURE_SA_DEFS_H__

#include <stdint.h>
#include "editcommon.h"

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
typedef struct _sa_time_code {
	union {
		uint8_t data[3];
		struct {
			uint8_t minutes;
			uint8_t seconds;
			uint8_t frames;
		};
	};
} sa_time_code;
ASSERT_STRUCT(sa_time_code, 3);

// Score data.
typedef union _sa_scores {
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
ASSERT_STRUCT(sa_scores, 128);

// Time data.
typedef union _sa_times {
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
ASSERT_STRUCT(sa_times, 84);

// Best weights. (Big only)
// Measured in 10s of grams.
// Three weights are stored per level.
typedef union _sa_weights {
	// NOTE: Big does not have times.
	uint16_t all[12];
	uint16_t levels[4][3];
} sa_weights;
ASSERT_STRUCT(sa_weights, 24);

// Best rings.
typedef union _sa_rings {
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
ASSERT_STRUCT(sa_rings, 64);

/**
 * [0x144] Mini-Game: Best scores.
 * Three scores are stored per mini-game.
 */
typedef union _sa_mini_game_scores {
	uint32_t all[27];	// all
	uint32_t game[9][3];	// per game
	struct {
		struct {
			uint32_t sonic[3];
			uint32_t tails[3];
		} sky_chase[2];
		struct {
			uint32_t sonic[3];
			uint32_t tails[3];
		} ice_cap;
		struct {
			uint32_t sonic[3];
			uint32_t tails[3];
		} sand_hill;
		uint32_t hedgehog_hammer[3];
	};
} sa_mini_game_scores;
ASSERT_STRUCT(sa_mini_game_scores, 108);

/**
 * [0x1B0] Mini-Game: Best times. (Twinkle Circuit)
 * Five times are stored per character:
 * - Best Time 1
 * - Best Time 2
 * - Best Time 3
 * - Lap 1 for Best Time 1
 * - Lap 2 for Best Time 1
 */
typedef union _sa_twinkle_circuit_times {
	sa_time_code all[30];	// all
	sa_time_code chr[6][5];	// per character
	struct {
		sa_time_code sonic[5];
		sa_time_code tails[5];
		sa_time_code knuckles[5];
		sa_time_code amy[5];
		sa_time_code gamma[5];
		sa_time_code big[5];
	};
} sa_twinkle_circuit_times;
ASSERT_STRUCT(sa_twinkle_circuit_times, 90);

/**
 * [0x20A] Mini-Game: Best times. (Boss Attack)
 * Three times are stored per character.
 */
typedef union _sa_boss_attack_times {
	sa_time_code all[18];	// all
	sa_time_code chr[6][3];	// per character
	struct {
		sa_time_code sonic[3];
		sa_time_code tails[3];
		sa_time_code knuckles[3];
		sa_time_code amy[3];
		sa_time_code gamma[3];
		sa_time_code big[3];
	};
} sa_boss_attack_times;
ASSERT_STRUCT(sa_boss_attack_times, 54);

// [0x251] Options: Choose the feature you want to edit.
#define SA_OPTIONS_MSG_VOICE_AND_TEXT	0x00
#define SA_OPTIONS_MSG_VOICE_ONLY	0x02
#define SA_OPTIONS_MSG_MASK		0x02
#define SA_OPTIONS_MSG_VALUE(x)		((x & SA_OPTIONS_MSG_MASK) >> 1)
#define SA_OPTIONS_VOICE_LANG_DEFAULT	0x00
#define SA_OPTIONS_VOICE_LANG_JP	0x04
#define SA_OPTIONS_VOICE_LANG_EN	0x08
#define SA_OPTIONS_VOICE_LANG_MASK	0x0C
#define SA_OPTIONS_VOICE_LANG_VALUE(x)	((x & SA_OPTIONS_VOICE_LANG_MASK) >> 2)
#define SA_OPTIONS_TEXT_LANG_DEFAULT	0x00
#define SA_OPTIONS_TEXT_LANG_JP		0x10
#define SA_OPTIONS_TEXT_LANG_EN		0x20
#define SA_OPTIONS_TEXT_LANG_FR		0x30
#define SA_OPTIONS_TEXT_LANG_ES		0x40
#define SA_OPTIONS_TEXT_LANG_DE		0x50
#define SA_OPTIONS_TEXT_LANG_MASK	0x70
#define SA_OPTIONS_TEXT_LANG_VALUE(x)	((x & SA_OPTIONS_TEXT_LANG_MASK) >> 4)

// [0x25A] Rumble feature.
#define SA_RUMBLE_FEATURE_OFF		0x00
#define SA_RUMBLE_FEATURE_ON		0x01
#define SA_RUMBLE_FEATURE_MASK		0x01
#define SA_RUMBLE_FEATURE_VALUE(x)	(x & SA_RUMBLE_FEATURE_MASK)

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

// Event flags.
typedef struct _sa_event_flags {
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
ASSERT_STRUCT(sa_event_flags, 8*8);

// NPC flags.
typedef struct _sa_npc_flags {
	union {
		uint8_t all[64];	// all
		// TODO: No one knows what any of
		// the NPC flags are...
	};
} sa_npc_flags;
ASSERT_STRUCT(sa_npc_flags, 64);

// Adventure Mode data.
typedef struct _sa_adventure_mode {
	// One set per character.
	// NOTE: Using int16_t for the unknown values,
	// since that matches MainMemory's SASave.
	struct {
		uint8_t time_of_day;
		uint8_t unused;
		int16_t unknown1;
		int16_t unknown2;
		uint16_t start_entrance;
		uint16_t start_level_and_act;
		int16_t unknown3;
	} chr[8];
} sa_adventure_mode;
ASSERT_STRUCT(sa_adventure_mode, 96);

// Level clear count.
typedef union _sa_level_clear_count {
	// All levels are available for all characters.
	uint8_t all[8][43];
	struct {
		uint8_t sonic[43];
		uint8_t unused1[43];
		uint8_t tails[43];
		uint8_t knuckles[43];
		uint8_t unused2[43];
		uint8_t amy[43];
		uint8_t gamma[43];
		uint8_t big[43];
	};
} sa_level_clear_count;
ASSERT_STRUCT(sa_level_clear_count, 8*43);

/**
 * Is the specified emblem set?
 * @param emblems sa_save_slot.emblems[] array.
 * @param idx Emblem index. [0, 129]
 * @return True if the emblem is set; false if not.
 */
#define SA_TEST_EMBLEM(emblems, idx) \
	(!!((emblems)[(idx)/8] & (1 << ((idx)%8))))

// Save slot. (SA1/SADX common data)
// GCN files have a single slot.
// Dreamcast files have three slots.
typedef struct _sa_save_slot {
	uint32_t crc;		// CRC (only low 16 bits are used)
	uint32_t playTime;	// Play time (1/60ths of a second)

	sa_scores scores;	// [0x008] Best scores.
	sa_times times;		// [0x088] Best times. (Does NOT include Big.)
	sa_weights weights;	// [0x0DC] Best weights. (Big only)
	uint8_t reserved1[16];	// [0x0F4] unknown
	sa_rings rings;		// [0x104] Best rings.

	// [0x144] Mini-Game: Best scores.
	sa_mini_game_scores mini_game_scores;

	// [0x1B0] Mini-Game: Best times. (Twinkle Circuit)
	sa_twinkle_circuit_times twinkle_circuit;

	// [0x20A] Mini-Game: Best times. (Boss Attack)
	// Three times are stored per character.
	sa_boss_attack_times boss_attack;

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
	sa_npc_flags npc;

	uint8_t reserved5[8];	// [0x2E0] unknown

	// [0x2E8] Adventure mode data. (one per character)
	sa_adventure_mode adventure_mode;

	// [0x348] Level clear count.
	sa_level_clear_count clear_count;
} sa_save_slot;
ASSERT_STRUCT(sa_save_slot, 1184);

/** SADX-specific data **/

// Missions.
#define SADX_MISSIONS_LEN 60
// 60 missions; each byte is interpreted as a bitfield.
#define SADX_MISSION_ACTIVE	(1 << 0)	/* Mission started. */
#define SADX_MISSION_UNLOCKED	(1 << 6)	/* Mission card found. */
#define SADX_MISSION_COMPLETED	(1 << 7)	/* Mission completed. */

/**
 * Mini-Game: Best scores. (Metal Sonic)
 * Three scores are stored per mini-game.
 */
typedef union _sadx_extra_mini_game_scores_metal {
	uint32_t all[6];	// all
	uint32_t game[2][3];	// per game
	struct {
		uint32_t ice_cap[3];
		uint32_t sand_hill[3];
	};
} sadx_extra_mini_game_scores_metal;
ASSERT_STRUCT(sadx_extra_mini_game_scores_metal, 24);

// Save slot. (SADX-specific data)
typedef struct _sadx_extra_save_slot {
	// NOTE: Offsets are relative to the main save file.

	// [0x4A0] Mission data
	uint8_t missions[60];

	uint32_t rings_black_market;	// [0x4DC] Black Market rings

	uint32_t scores_metal[10];	// [0x4E0] Scores (Metal Sonic)
	sa_time_code times_metal[10];	// [0x508] Best Times (Metal Sonic)
	uint16_t rings_metal[10];	// [0x526] Most Rings (Metal Sonic)
	uint16_t reserved1;		// [0x43A] unknown

	// Mini-games
	sadx_extra_mini_game_scores_metal mini_game_scores_metal;	// 0x53C
	sa_time_code twinkle_circuit_metal[5];				// 0x554
	sa_time_code boss_attack_metal[3];				// 0x563

	// [0x56C] Metal Sonic emblems. (bitfield)
	uint32_t emblems_metal;
} sadx_extra_save_slot;
ASSERT_STRUCT(sadx_extra_save_slot, 208);

#ifdef __cplusplus
}
#endif

#endif /* __LIBSAVEEDIT_SONICADVENTURE_SA_DEFS_H__ */
