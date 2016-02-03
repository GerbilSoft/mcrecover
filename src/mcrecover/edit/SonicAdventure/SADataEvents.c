/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SADataEvents.c: Sonic Adventure - Events data.                          *
 *                                                                         *
 * Copyright (c) 2015-2016 by David Korth.                                 *
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
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "SAData.h"

// NOTE: Many of the event flags are unused, so instead of using
// an array, we're using a struct of event flags.
// FIXME: Alignment on 64-bit?
#include "../models/bit_flag.h"

// Sonic Adventure event flags data.
// Borrowed from SASave.
// TODO: Auto-generate a .h file from SASave's Events.ini?
const bit_flag_t sa_event_flags_desc[SA_EVENT_FLAGS_COUNT+1] = {
	// General
	{65, "Sonic unlocked in Adventure"},
	{66, "Tails unlocked in Adventure"},
	{67, "Knuckles unlocked in Adventure"},
	{68, "Amy unlocked in Adventure"},
	{69, "Gamma unlocked in Adventure"},
	{70, "Big unlocked in Adventure"},
	{71, "Super Sonic unlocked in Adventure"},
	{72, "Sonic's story complete"},
	{73, "Tails' story complete"},
	{74, "Knuckles' story complete"},
	{75, "Amy's story complete"},
	{76, "Gamma's story complete"},
	{77, "Big's story complete"},
	{78, "Super Sonic's story complete"},
	{79, "Gold Chao egg taken"},
	{80, "Silver Chao egg taken"},
	{81, "Black Chao egg taken"},

	// Sonic
	{128, "Police barricade removed"},
	{129, "Station Square hotel open"},
	{130, "Station Square station open"},
	{131, "Car covering sewer removed"},
	{132, "Casino always open"},
	{133, "Ice stone available"},
	{134, "Door from Hotel to Casino open"},
	{135, "Casinopolis open"},
	{136, "Door from Station to Casino open"},
	{137, "Twinkle Park open"},
	{138, "Twinkle Circuit open"},
	{140, "Speed Highway open"},
	{141, "Light Speed Shoes obtained"},
	{142, "Crystal Ring obtained"},
	{145, "Egg Carrier transformed"},
	{146, "Egg Carrier open"},
	{147, "Egg Carrier sunk"},
	{148, "Windy Valley open"},
	{150, "Angel Island open"},
	{151, "Ice Cap open"},
	{153, "Red Mountain open"},
	{154, "Door to Cliff open"},
	{155, "Mystic Ruins Jungle open"},
	{156, "Lost World open"},
	{157, "Final Egg field open"},
	{159, "Ancient Light obtained"},
	{160, "Final Egg open"},
	{161, "SS-MR Train open"},
	{162, "Boat to Egg Carrier open"},
	{163, "Raft to Egg Carrier open"},
	{164, "Gamma defeated"},
	{165, "Knuckles defeated"},
	{166, "Emerald Coast clear"},
	{167, "Windy Valley clear"},
	{168, "Casinopolis clear"},
	{169, "Twinkle Park clear"},
	{170, "Speed Highway clear"},
	{171, "Red Mountain clear"},
	{172, "Ice Cap clear"},
	{173, "Sky Deck clear"},
	{174, "Lost World clear"},
	{175, "Final Egg clear"},
	{176, "Chaos 0 clear"},
	{177, "Chaos 4 clear"},
	{178, "Chaos 6 clear"},
	{179, "Egg Hornet clear"},
	{180, "Egg Viper clear"},
	{181, "Sky Chase Act 1 clear"},
	{182, "Sky Chase Act 2 clear"},

	// Tails
	{193, "Station Square Hotel open"},
	{197, "Casinopolis open"},
	{202, "Station Square Station open"},
	{205, "Jet Anklet obtained"},
	{210, "Egg Carrier sunk"},
	{211, "Windy Valley open"},
	{214, "Ice Cap open"},
	{219, "Rhythm Badge obtained"},
	{220, "SS-MR Train open"},
	{225, "Windy Valley clear"},
	{226, "Casinopolis clear"},
	{227, "Speed Highway clear"},
	{228, "Ice Cap clear"},
	{229, "Sky Deck clear"},
	{230, "Sand Hill clear"},
	{231, "Chaos 4 clear"},
	{232, "Egg Walker clear"},
	{233, "Egg Hornet clear"},
	{234, "Sky Chase Act 1 clear"},
	{235, "Sky Chase Act 2 clear"},
	{236, "Emerald Coast clear"},
	{237, "Red Mountain clear"},

	// Knuckles
	{259, "Casinopolis open"},
	{271, "Egg Carrier sunk"},
	{281, "Shovel Claw obtained"},
	{282, "Fighting Gloves obtained"},
	{287, "Casinopolis clear"},
	{288, "Speed Highway clear"},
	{289, "Red Mountain clear"},
	{290, "Lost World clear"},
	{291, "Chaos 2 clear"},
	{292, "Chaos 6 clear"},
	{293, "Chaos 4 clear"},
	{294, "Sky Deck clear"},

	// Amy
	{333, "Egg Carrier sunk"},
	{338, "Warrior Feather obtained"},
	{342, "Twinkle Park clear"},
	{343, "Hot Shelter clear"},
	{344, "Final Egg clear"},
	{345, "Zero clear"},
	{350, "Long Hammer"},

	// Gamma
	{393, "Jet Booster obtained"},
	{394, "Laser Blaster obtained"},
	{396, "Egg Carrier sunk"},
	{400, "Windy Valley open"},
	{411, "Emerald Coast clear"},
	{412, "Windy Valley clear"},
	{413, "Red Mountain clear"},
	{414, "Hot Shelter clear"},
	{415, "Final Egg clear"},
	{416, "E-101 clear"},
	{417, "E-101mkII clear"},

	// Big
	{459, "Egg Carrier sunk"},
	{464, "Life Ring obtained"},
	{465, "Power Rod obtained"},
	{469, "Emerald Coast clear"},
	{470, "Hot Shelter clear"},
	{471, "Twinkle Park clear"},
	{472, "Ice Cap clear"},
	{473, "Chaos 6 clear"},

	// End of list.
	{-1, nullptr}
};


