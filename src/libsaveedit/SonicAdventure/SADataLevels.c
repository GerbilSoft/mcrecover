/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SADataLevels.c: Sonic Adventure - Level data.                           *
 *                                                                         *
 * Copyright (c) 2015-2018 by David Korth.                                 *
 * Original data from SASave by MainMemory.                                *
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
 * You should have received a copy of the GNU General Public License       *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 ***************************************************************************/

#include "SAData.h"

/**
 * Level names. (ASCII, untranslated)
 * Contains Action Stages only, in the order of Sonic's story,
 * with "Hot Shelter" afterwards.
 */
const char *const sa_level_names_action[SA_LEVEL_NAMES_ACTION_COUNT] = {
	"Emerald Coast",	// 0
	"Windy Valley",		// 1
	"Casinopolis",		// 2
	"Ice Cap",		// 3
	"Twinkle Park",		// 4
	"Speed Highway",	// 5
	"Red Mountain",		// 6
	"Sky Deck",		// 7
	"Lost World",		// 8
	"Final Egg",		// 9
	"Hot Shelter"		// 10
};

/**
 * Level names. (ASCII, untranslated)
 * Contains all levels, not just Action Stages.
 * NOTE: Chao Gardens have line breaks for formatting purposes.
 */
const char *const sa_level_names_all[SA_LEVEL_NAMES_ALL_COUNT] = {
	// 0-9
	"Hedgehog Hammer",
	"Emerald Coast",
	"Windy Valley",
	"Twinkle Park",
	"Speed Highway",
	"Red Mountain",
	"Sky Deck",
	"Lost World",
	"Ice Cap",
	"Casinopolis",

	// 10-19
	"Final Egg",
	"Mystic Ruins (11)",
	"Hot Shelter",
	"Mystic Ruins (13)",
	"Mystic Ruins (14)",
	"Chaos 0",
	"Chaos 2",
	"Chaos 4",
	"Chaos 6",
	"Perfect Chaos",

	// 20-29
	"Egg Hornet",
	"Egg Walker",
	"Egg Viper",
	"ZERO",
	"E-101 Beta",
	"E-101 mkII",
	"Station Square (26)",
	"Station Square (27)",
	"Station Square (28)",
	"Egg Carrier (Outside)",

	// 30-39
	"Egg Carrier (30)",
	"Egg Carrier (31)",
	"Egg Carrier (Inside)",
	"Mystic Ruins (33)",
	"The Past",		// NOTE: Unused in some sections.
				// Was previously listed as "Mystic Ruins"
				// on SAGeneral.
	"Twinkle Circuit",
	"Sky Chase Act 1",
	"Sky Chase Act 2",
	"Sand Hill",
	"Station Square\nChao Garden",

	// 40-42
	"Egg Carrier\nChao Garden",
	"Mystic Ruins\nChao Garden",
	"Chao Race",
};
