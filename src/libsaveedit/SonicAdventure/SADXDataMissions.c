/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SADXDataMissions.c: Sonic Adventure - Missions data.                    *
 *                                                                         *
 * Copyright (c) 2015-2018 by David Korth.                                 *
 * Original data from SASave by MainMemory.                                *
 *                                                                         *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "SAData.h"

#ifndef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(ctx, str) str
#endif

// Mission flags.
const bit_flag_t sadx_mission_flags_desc[SADX_MISSION_FLAGS_COUNT+1] = {
	// Missions 1-10
	{0, QT_TRANSLATE_NOOP("SADXDataMissions", "Bring the man who is standing in front of the hamburger shop!")},
	{1, QT_TRANSLATE_NOOP("SADXDataMissions", "Get the balloon in the skies of the Mystic Ruins!")},
	{2, QT_TRANSLATE_NOOP("SADXDataMissions", "Collect 100 rings, and go to Sonic's billboard by the pool!")},
	{3, QT_TRANSLATE_NOOP("SADXDataMissions", "Weeds are growing all over my place! I must get rid of them!")},
	{4, QT_TRANSLATE_NOOP("SADXDataMissions", "I lost my balloon! It's way up there now!")},
	{5, QT_TRANSLATE_NOOP("SADXDataMissions", "He is going to drown! Help the man in the water!")},
	{6, QT_TRANSLATE_NOOP("SADXDataMissions", "Lonely Metal Sonic needs a friend. Look carefully.")},
	{7, QT_TRANSLATE_NOOP("SADXDataMissions", "The medallion fell under there! No illegal parking please!")},
	{8, QT_TRANSLATE_NOOP("SADXDataMissions", "Get the balloon floating behind the waterfall at the emerald sea.")},
	{9, QT_TRANSLATE_NOOP("SADXDataMissions", "What is that sparkling in the water?")},

	// Missions 11-20
	{10, QT_TRANSLATE_NOOP("SADXDataMissions", "Destroy the windmill and proceed. Find the balloon in orbit!")},
	{11, QT_TRANSLATE_NOOP("SADXDataMissions", "Who is a Chao good friends with? And what is hidden underneath?")},
	{12, QT_TRANSLATE_NOOP("SADXDataMissions", "I can't take a shower like this! Do something!")},
	{13, QT_TRANSLATE_NOOP("SADXDataMissions", "I am the keeper of this hotel! Catch me if you can!")},
	{14, QT_TRANSLATE_NOOP("SADXDataMissions", "My medallions got swept away by the tornado! Somebody help me get them back!")},
	{15, QT_TRANSLATE_NOOP("SADXDataMissions", "Get the flags from the floating islands!")},
	{16, QT_TRANSLATE_NOOP("SADXDataMissions", "Aim and shoot all the medallions with a Sonic Ball.")},
	{17, QT_TRANSLATE_NOOP("SADXDataMissions", "During the night, at the amusement park, place your jumps on the top of one of the tables.")},
	{18, QT_TRANSLATE_NOOP("SADXDataMissions", "What is that behind the mirror?")},
	{19, QT_TRANSLATE_NOOP("SADXDataMissions", "Get all the medallions within the time limit! It's real slippery, so be careful!")},

	// Missions 21-30
	{20, QT_TRANSLATE_NOOP("SADXDataMissions", "Protect the Sonic doll from the Spinners surrounding it!")},
	{21, QT_TRANSLATE_NOOP("SADXDataMissions", "Find the flag hidden in the secret passage under the emerald ocean!")},
	{22, QT_TRANSLATE_NOOP("SADXDataMissions", "Go around the wooden horse and collect 10 balloons.")},
	{23, QT_TRANSLATE_NOOP("SADXDataMissions", "'I hate this dark and filthy place!' Can you find it?")},
	{24, QT_TRANSLATE_NOOP("SADXDataMissions", "What is hidden under the lion's right hand?")},
	{25, QT_TRANSLATE_NOOP("SADXDataMissions", "What is that on top of the ship's mast that the pirates are protecting?")},
	{26, QT_TRANSLATE_NOOP("SADXDataMissions", "Collect 100 rings and head to the heliport!")},
	{27, QT_TRANSLATE_NOOP("SADXDataMissions", "During the morning traffic, use the fountain to get the balloon.")},
	{28, QT_TRANSLATE_NOOP("SADXDataMissions", "I am the keeper of this canal! Catch me if you can!")},
	{29, QT_TRANSLATE_NOOP("SADXDataMissions", "A fugitive have escaped from the jail of burning hell! Find the fugitive!")},

	// Missions 31-40
	{30, QT_TRANSLATE_NOOP("SADXDataMissions", "Get the balloon as you float in the air along with the trash!")},
	{31, QT_TRANSLATE_NOOP("SADXDataMissions", "Can you get the balloon that is hidden under the bridge?")},
	{32, QT_TRANSLATE_NOOP("SADXDataMissions", "Shoot yourself out of the cannon and get the balloon!")},
	{33, QT_TRANSLATE_NOOP("SADXDataMissions", "Can you find the balloon that is hidden on the ship's bridge?")},
	{34, QT_TRANSLATE_NOOP("SADXDataMissions", "I am the keeper of this icy lake! Catch me if you can!")},
	{35, QT_TRANSLATE_NOOP("SADXDataMissions", "Fighter aircraft are flying everywhere. Somebody get me out of here!")},
	{36, QT_TRANSLATE_NOOP("SADXDataMissions", "Fly over the jungle, and get all the balloons!")},
	{37, QT_TRANSLATE_NOOP("SADXDataMissions", "A message from an ancient people: In the direction where the burning arrow is pointing, you will see...")},
	{38, QT_TRANSLATE_NOOP("SADXDataMissions", "Treasure hunt at the beach! Find all the medallions under a time limit!")},
	{39, QT_TRANSLATE_NOOP("SADXDataMissions", "What is hidden in the area that the giant snake is staring at?")},

	// Missions 41-50
	{40, QT_TRANSLATE_NOOP("SADXDataMissions", "Look real carefully just as you fall from the waterfall!")},
	{41, QT_TRANSLATE_NOOP("SADXDataMissions", "I can't get into the bathroom. How could I've let something like this happen to me?")},
	{42, QT_TRANSLATE_NOOP("SADXDataMissions", "Fortress of steel. High Jump on 3 narrow paths. Be careful not to fall.")},
	{43, QT_TRANSLATE_NOOP("SADXDataMissions", "I am the keeper of this ship! Catch me if you can!")},
	{44, QT_TRANSLATE_NOOP("SADXDataMissions", "Go to a place where the rings are laid in the shape of Sonic's face!")},
	{45, QT_TRANSLATE_NOOP("SADXDataMissions", "A secret base full of mechanical traps. Pay attention, and you might see...")},
	{46, QT_TRANSLATE_NOOP("SADXDataMissions", "Get 10 balloons on the field under the time limit!")},
	{47, QT_TRANSLATE_NOOP("SADXDataMissions", "Can you get the medallion that the giant Sonic is staring at?")},
	{48, QT_TRANSLATE_NOOP("SADXDataMissions", "Scorch through the track, and get all the flags!")},
	{49, QT_TRANSLATE_NOOP("SADXDataMissions", "Select a road that splits into 5 paths before time runs out!")},

	// Missions 51-60
	{50, QT_TRANSLATE_NOOP("SADXDataMissions", "Gunman of the Windy Valley! Destroy all of the Spinners under a time limit!")},
	{51, QT_TRANSLATE_NOOP("SADXDataMissions", "Get 3 flags in the jungle under the time limit!")},
	{52, QT_TRANSLATE_NOOP("SADXDataMissions", "Pass the line of rings with 3 Super High Jumps on the ski slope!")},
	{53, QT_TRANSLATE_NOOP("SADXDataMissions", "Slide downhill in a blizzard and get all of the flags!")},
	{54, QT_TRANSLATE_NOOP("SADXDataMissions", "Run down the building to get all the balloons!")},
	{55, QT_TRANSLATE_NOOP("SADXDataMissions", "Relentless eruptions occur in the flaming canyon. What could be hidden in the area she's staring at?")},
	{56, QT_TRANSLATE_NOOP("SADXDataMissions", "Peak of the volcanic mountain! Watch out for the lava!")},
	{57, QT_TRANSLATE_NOOP("SADXDataMissions", "The big rock will start rolling after you! Try to get all the flags")},
	{58, QT_TRANSLATE_NOOP("SADXDataMissions", "Watch out for the barrels, and find the hidden flag inside the container!")},
	{59, QT_TRANSLATE_NOOP("SADXDataMissions", "Something is hidden inside the dinosaur's mouth. Can you find it?")},

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
