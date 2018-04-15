/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SADataEvents.c: Sonic Adventure - Events data.                          *
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

// NOTE: Many of the event flags are unused, so instead of using
// an array, we're using a struct of event flags.
// FIXME: Alignment on 64-bit?
#include "../models/bit_flag.h"

#ifndef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(ctx, str) str
#endif

// Sonic Adventure event flags data.
// Borrowed from SASave.
// TODO: Auto-generate a .h file from SASave's Events.ini?
const bit_flag_t sa_event_flags_desc[SA_EVENT_FLAGS_COUNT+1] = {
	// General
	{65, QT_TRANSLATE_NOOP("SADataEvents", "Sonic unlocked in Adventure")},
	{66, QT_TRANSLATE_NOOP("SADataEvents", "Tails unlocked in Adventure")},
	{67, QT_TRANSLATE_NOOP("SADataEvents", "Knuckles unlocked in Adventure")},
	{68, QT_TRANSLATE_NOOP("SADataEvents", "Amy unlocked in Adventure")},
	{69, QT_TRANSLATE_NOOP("SADataEvents", "Gamma unlocked in Adventure")},
	{70, QT_TRANSLATE_NOOP("SADataEvents", "Big unlocked in Adventure")},
	{71, QT_TRANSLATE_NOOP("SADataEvents", "Super Sonic unlocked in Adventure")},
	{72, QT_TRANSLATE_NOOP("SADataEvents", "Sonic's story complete")},
	{73, QT_TRANSLATE_NOOP("SADataEvents", "Tails' story complete")},
	{74, QT_TRANSLATE_NOOP("SADataEvents", "Knuckles' story complete")},
	{75, QT_TRANSLATE_NOOP("SADataEvents", "Amy's story complete")},
	{76, QT_TRANSLATE_NOOP("SADataEvents", "Gamma's story complete")},
	{77, QT_TRANSLATE_NOOP("SADataEvents", "Big's story complete")},
	{78, QT_TRANSLATE_NOOP("SADataEvents", "Super Sonic's story complete")},
	{79, QT_TRANSLATE_NOOP("SADataEvents", "Gold Chao egg taken")},
	{80, QT_TRANSLATE_NOOP("SADataEvents", "Silver Chao egg taken")},
	{81, QT_TRANSLATE_NOOP("SADataEvents", "Black Chao egg taken")},

	// Sonic
	{128, QT_TRANSLATE_NOOP("SADataEvents", "Police barricade removed")},
	{129, QT_TRANSLATE_NOOP("SADataEvents", "Station Square hotel open")},
	{130, QT_TRANSLATE_NOOP("SADataEvents", "Station Square station open")},
	{131, QT_TRANSLATE_NOOP("SADataEvents", "Car covering sewer removed")},
	{132, QT_TRANSLATE_NOOP("SADataEvents", "Casino always open")},
	{133, QT_TRANSLATE_NOOP("SADataEvents", "Ice stone available")},
	{134, QT_TRANSLATE_NOOP("SADataEvents", "Door from Hotel to Casino open")},
	{135, QT_TRANSLATE_NOOP("SADataEvents", "Casinopolis open")},
	{136, QT_TRANSLATE_NOOP("SADataEvents", "Door from Station to Casino open")},
	{137, QT_TRANSLATE_NOOP("SADataEvents", "Twinkle Park open")},
	{138, QT_TRANSLATE_NOOP("SADataEvents", "Twinkle Circuit open")},
	{140, QT_TRANSLATE_NOOP("SADataEvents", "Speed Highway open")},
	{141, QT_TRANSLATE_NOOP("SADataEvents", "Light Speed Shoes obtained")},
	{142, QT_TRANSLATE_NOOP("SADataEvents", "Crystal Ring obtained")},
	{145, QT_TRANSLATE_NOOP("SADataEvents", "Egg Carrier transformed")},
	{146, QT_TRANSLATE_NOOP("SADataEvents", "Egg Carrier open")},
	{147, QT_TRANSLATE_NOOP("SADataEvents", "Egg Carrier sunk")},
	{148, QT_TRANSLATE_NOOP("SADataEvents", "Windy Valley open")},
	{150, QT_TRANSLATE_NOOP("SADataEvents", "Angel Island open")},
	{151, QT_TRANSLATE_NOOP("SADataEvents", "Ice Cap open")},
	{153, QT_TRANSLATE_NOOP("SADataEvents", "Red Mountain open")},
	{154, QT_TRANSLATE_NOOP("SADataEvents", "Door to Cliff open")},
	{155, QT_TRANSLATE_NOOP("SADataEvents", "Mystic Ruins Jungle open")},
	{156, QT_TRANSLATE_NOOP("SADataEvents", "Lost World open")},
	{157, QT_TRANSLATE_NOOP("SADataEvents", "Final Egg field open")},
	{159, QT_TRANSLATE_NOOP("SADataEvents", "Ancient Light obtained")},
	{160, QT_TRANSLATE_NOOP("SADataEvents", "Final Egg open")},
	{161, QT_TRANSLATE_NOOP("SADataEvents", "SS-MR Train open")},
	{162, QT_TRANSLATE_NOOP("SADataEvents", "Boat to Egg Carrier open")},
	{163, QT_TRANSLATE_NOOP("SADataEvents", "Raft to Egg Carrier open")},
	{164, QT_TRANSLATE_NOOP("SADataEvents", "Gamma defeated")},
	{165, QT_TRANSLATE_NOOP("SADataEvents", "Knuckles defeated")},
	{166, QT_TRANSLATE_NOOP("SADataEvents", "Emerald Coast clear")},
	{167, QT_TRANSLATE_NOOP("SADataEvents", "Windy Valley clear")},
	{168, QT_TRANSLATE_NOOP("SADataEvents", "Casinopolis clear")},
	{169, QT_TRANSLATE_NOOP("SADataEvents", "Twinkle Park clear")},
	{170, QT_TRANSLATE_NOOP("SADataEvents", "Speed Highway clear")},
	{171, QT_TRANSLATE_NOOP("SADataEvents", "Red Mountain clear")},
	{172, QT_TRANSLATE_NOOP("SADataEvents", "Ice Cap clear")},
	{173, QT_TRANSLATE_NOOP("SADataEvents", "Sky Deck clear")},
	{174, QT_TRANSLATE_NOOP("SADataEvents", "Lost World clear")},
	{175, QT_TRANSLATE_NOOP("SADataEvents", "Final Egg clear")},
	{176, QT_TRANSLATE_NOOP("SADataEvents", "Chaos 0 clear")},
	{177, QT_TRANSLATE_NOOP("SADataEvents", "Chaos 4 clear")},
	{178, QT_TRANSLATE_NOOP("SADataEvents", "Chaos 6 clear")},
	{179, QT_TRANSLATE_NOOP("SADataEvents", "Egg Hornet clear")},
	{180, QT_TRANSLATE_NOOP("SADataEvents", "Egg Viper clear")},
	{181, QT_TRANSLATE_NOOP("SADataEvents", "Sky Chase Act 1 clear")},
	{182, QT_TRANSLATE_NOOP("SADataEvents", "Sky Chase Act 2 clear")},

	// Tails
	{193, QT_TRANSLATE_NOOP("SADataEvents", "Station Square hotel open")},
	{197, QT_TRANSLATE_NOOP("SADataEvents", "Casinopolis open")},
	{202, QT_TRANSLATE_NOOP("SADataEvents", "Station Square station open")},
	{205, QT_TRANSLATE_NOOP("SADataEvents", "Jet Anklet obtained")},
	{210, QT_TRANSLATE_NOOP("SADataEvents", "Egg Carrier sunk")},
	{211, QT_TRANSLATE_NOOP("SADataEvents", "Windy Valley open")},
	{214, QT_TRANSLATE_NOOP("SADataEvents", "Ice Cap open")},
	{219, QT_TRANSLATE_NOOP("SADataEvents", "Rhythm Badge obtained")},
	{220, QT_TRANSLATE_NOOP("SADataEvents", "SS-MR Train open")},
	{225, QT_TRANSLATE_NOOP("SADataEvents", "Windy Valley clear")},
	{226, QT_TRANSLATE_NOOP("SADataEvents", "Casinopolis clear")},
	{227, QT_TRANSLATE_NOOP("SADataEvents", "Speed Highway clear")},
	{228, QT_TRANSLATE_NOOP("SADataEvents", "Ice Cap clear")},
	{229, QT_TRANSLATE_NOOP("SADataEvents", "Sky Deck clear")},
	{230, QT_TRANSLATE_NOOP("SADataEvents", "Sand Hill clear")},
	{231, QT_TRANSLATE_NOOP("SADataEvents", "Chaos 4 clear")},
	{232, QT_TRANSLATE_NOOP("SADataEvents", "Egg Walker clear")},
	{233, QT_TRANSLATE_NOOP("SADataEvents", "Egg Hornet clear")},
	{234, QT_TRANSLATE_NOOP("SADataEvents", "Sky Chase Act 1 clear")},
	{235, QT_TRANSLATE_NOOP("SADataEvents", "Sky Chase Act 2 clear")},
	{236, QT_TRANSLATE_NOOP("SADataEvents", "Emerald Coast clear")},
	{237, QT_TRANSLATE_NOOP("SADataEvents", "Red Mountain clear")},

	// Knuckles
	{259, QT_TRANSLATE_NOOP("SADataEvents", "Casinopolis open")},
	{271, QT_TRANSLATE_NOOP("SADataEvents", "Egg Carrier sunk")},
	{281, QT_TRANSLATE_NOOP("SADataEvents", "Shovel Claw obtained")},
	{282, QT_TRANSLATE_NOOP("SADataEvents", "Fighting Gloves obtained")},
	{287, QT_TRANSLATE_NOOP("SADataEvents", "Casinopolis clear")},
	{288, QT_TRANSLATE_NOOP("SADataEvents", "Speed Highway clear")},
	{289, QT_TRANSLATE_NOOP("SADataEvents", "Red Mountain clear")},
	{290, QT_TRANSLATE_NOOP("SADataEvents", "Lost World clear")},
	{291, QT_TRANSLATE_NOOP("SADataEvents", "Chaos 2 clear")},
	{292, QT_TRANSLATE_NOOP("SADataEvents", "Chaos 6 clear")},
	{293, QT_TRANSLATE_NOOP("SADataEvents", "Chaos 4 clear")},
	{294, QT_TRANSLATE_NOOP("SADataEvents", "Sky Deck clear")},

	// Amy
	{333, QT_TRANSLATE_NOOP("SADataEvents", "Egg Carrier sunk")},
	{338, QT_TRANSLATE_NOOP("SADataEvents", "Warrior Feather obtained")},
	{342, QT_TRANSLATE_NOOP("SADataEvents", "Twinkle Park clear")},
	{343, QT_TRANSLATE_NOOP("SADataEvents", "Hot Shelter clear")},
	{344, QT_TRANSLATE_NOOP("SADataEvents", "Final Egg clear")},
	{345, QT_TRANSLATE_NOOP("SADataEvents", "Zero clear")},
	{350, QT_TRANSLATE_NOOP("SADataEvents", "Long Hammer")},

	// Gamma
	{393, QT_TRANSLATE_NOOP("SADataEvents", "Jet Booster obtained")},
	{394, QT_TRANSLATE_NOOP("SADataEvents", "Laser Blaster obtained")},
	{396, QT_TRANSLATE_NOOP("SADataEvents", "Egg Carrier sunk")},
	{400, QT_TRANSLATE_NOOP("SADataEvents", "Windy Valley open")},
	{411, QT_TRANSLATE_NOOP("SADataEvents", "Emerald Coast clear")},
	{412, QT_TRANSLATE_NOOP("SADataEvents", "Windy Valley clear")},
	{413, QT_TRANSLATE_NOOP("SADataEvents", "Red Mountain clear")},
	{414, QT_TRANSLATE_NOOP("SADataEvents", "Hot Shelter clear")},
	{415, QT_TRANSLATE_NOOP("SADataEvents", "Final Egg clear")},
	{416, QT_TRANSLATE_NOOP("SADataEvents", "E-101 clear")},
	{417, QT_TRANSLATE_NOOP("SADataEvents", "E-101mkII clear")},

	// Big
	{459, QT_TRANSLATE_NOOP("SADataEvents", "Egg Carrier sunk")},
	{464, QT_TRANSLATE_NOOP("SADataEvents", "Life Ring obtained")},
	{465, QT_TRANSLATE_NOOP("SADataEvents", "Power Rod obtained")},
	{469, QT_TRANSLATE_NOOP("SADataEvents", "Emerald Coast clear")},
	{470, QT_TRANSLATE_NOOP("SADataEvents", "Hot Shelter clear")},
	{471, QT_TRANSLATE_NOOP("SADataEvents", "Twinkle Park clear")},
	{472, QT_TRANSLATE_NOOP("SADataEvents", "Ice Cap clear")},
	{473, QT_TRANSLATE_NOOP("SADataEvents", "Chaos 6 clear")},

	// End of list.
	{-1, nullptr}
};
