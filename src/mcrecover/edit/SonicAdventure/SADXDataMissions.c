/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SADXDataMissions.c: Sonic Adventure - Missions data.                    *
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

// Mission flags.
const bit_flag_t sadx_mission_flags_desc[SADX_MISSION_FLAGS_COUNT+1] = {
	// Missions 1-10
	{0, "Bring the man who is standing in front of the hamburger shop!"},
	{1, "Get the balloon in the skies of the Mystic Ruins!"},
	{2, "Collect 100 rings, and go to Sonic's billboard by the pool!"},
	{3, "Weeds are growing all over my place! I must get rid of them!"},
	{4, "I lost my balloon! It's way up there now!"},
	{5, "He is going to drown! Help the man in the water!"},
	{6, "Lonely Metal Sonic needs a friend. Look carefully."},
	{7, "The medallion fell under there! No illegal parking please!"},
	{8, "Get the balloon floating behind the waterfall at the emerald sea."},
	{9, "What is that sparkling in the water?"},

	// Missions 11-20
	{10, "Destroy the windmill and proceed. Find the balloon in orbit!"},
	{11, "Who is a Chao good friends with? And what is hidden underneath?"},
	{12, "I can't take a shower like this! Do something!"},
	{13, "I am the keeper of this hotel! Catch me if you can!"},
	{14, "My medallions got swept away by the tornado! Somebody help me get them back!"},
	{15, "Get the flags from the floating islands!"},
	{16, "Aim and shoot all the medallions with a Sonic Ball."},
	{17, "During the night, at the amusement park, place your jumps on the top of one of the tables."},
	{18, "What is that behind the mirror?"},
	{19, "Get all the medallions within the time limit! It's real slippery, so be careful!"},

	// Missions 21-30
	{20, "Protect the Sonic doll from the Spinners surrounding it!"},
	{21, "Find the flag hidden in the secret passage under the emerald ocean!"},
	{22, "Go around the wooden horse and collect 10 balloons."},
	{23, "'I hate this dark and filthy place!' Can you find it?"},
	{24, "What is hidden under the lion's right hand?"},
	{25, "What is that on top of the ship's mast that the pirates are protecting?"},
	{26, "Collect 100 rings and head to the heliport!"},
	{27, "During the morning traffic, use the fountain to get the balloon."},
	{28, "I am the keeper of this canal! Catch me if you can!"},
	{29, "A fugitive have escaped from the jail of burning hell! Find the fugitive!"},

	// Missions 31-40
	{30, "Get the balloon as you float in the air along with the trash!"},
	{31, "Can you get the balloon that is hidden under the bridge?"},
	{32, "Shoot yourself out of the cannon and get the balloon!"},
	{33, "Can you find the balloon that is hidden on the ship's bridge?"},
	{34, "I am the keeper of this icy lake! Catch me if you can!"},
	{35, "Fighter aircraft are flying everywhere. Somebody get me out of here!"},
	{36, "Fly over the jungle, and get all the balloons!"},
	{37, "A message from an ancient people: In the direction where the burning arrow is pointing, you will see..."},
	{38, "Treasure hunt at the beach! Find all the medallions under a time limit!"},
	{39, "What is hidden in the area that the giant snake is staring at?"},

	// Missions 41-50
	{40, "Look real carefully just as you fall from the waterfall!"},
	{41, "I can't get into the bathroom. How could I've let something like this happen to me?"},
	{42, "Fortress of steel. High Jump on 3 narrow paths. Be careful not to fall."},
	{43, "I am the keeper of this ship! Catch me if you can!"},
	{44, "Go to a place where the rings are laid in the shape of Sonic's face!"},
	{45, "A secret base full of mechanical traps. Pay attention, and you might see..."},
	{46, "Get 10 balloons on the field under the time limit!"},
	{47, "Can you get the medallion that the giant Sonic is staring at?"},
	{48, "Scorch through the track, and get all the flags!"},
	{49, "Select a road that splits into 5 paths before time runs out!"},

	// Missions 51-60
	{50, "Gunman of the Windy Valley! Destroy all of the Spinners under a time limit!"},
	{51, "Get 3 flags in the jungle under the time limit!"},
	{52, "Pass the line of rings with 3 Super High Jumps on the ski slope!"},
	{53, "Slide downhill in a blizzard and get all of the flags!"},
	{54, "Run down the building to get all the balloons!"},
	{55, "Relentless eruptions occur in the flaming canyon. What could be hidden in the area she's staring at?"},
	{56, "Peak of the volcanic mountain! Watch out for the lava!"},
	{57, "The big rock will start rolling after you! Try to get all the flags"},
	{58, "Watch out for the barrels, and find the hidden flag inside the container!"},
	{59, "Something is hidden inside the dinosaur's mouth. Can you find it?"},

	// End of list.
	{-1, nullptr}
};

/**
 * Character mapping for missions.
 * 0 == Sonic, 1 == Tails, 2 == Knuckles,
 * 3 == Amy, 4 == Gamma, 5 == Big
 */
const uint8_t sadx_mission_flags_char[SADX_MISSION_FLAGS_COUNT] = {
	0, 0, 0, 1, 2, 3, 4, 5, 0, 1,	// Missions  1-10
	0, 2, 0, 5, 0, 1, 0, 3, 3, 0,	// Missions 11-20
	4, 5, 0, 1, 2, 2, 0, 0, 5, 0,	// Missions 21-30
	1, 2, 0, 0, 5, 0, 1, 2, 4, 0,	// Missions 31-40
	0, 4, 3, 5, 0, 0, 1, 2, 0, 3,	// Missions 41-50
	4, 5, 0, 1, 0, 2, 0, 0, 2, 5,	// Missions 51-60
};
