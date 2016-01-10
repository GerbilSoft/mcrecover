/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAData.h: Sonic Adventure - Miscellaneous data.                         *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SADATA_H__
#define __MCRECOVER_EDIT_SONICADVENTURE_SADATA_H__

// NOTE: Many of the event flags are unused, so instead of using
// an array, we're using a struct of event flags.
// FIXME: Alignment on 64-bit?
#include "bit_flag.h"

#define SA_EVENT_FLAGS_COUNT 127
extern const bit_flag_t sa_event_flags_desc[SA_EVENT_FLAGS_COUNT+1];

#define SADX_MISSION_FLAGS_COUNT 60
extern const bit_flag_t sadx_mission_flags_desc[SADX_MISSION_FLAGS_COUNT+1];

/**
 * Character mapping for missions.
 * 0 == Sonic, 1 == Tails, 2 == Knuckles,
 * 3 == Amy, 4 == Gamma, 5 == Big
 */
extern const uint8_t sadx_mission_flags_char[SADX_MISSION_FLAGS_COUNT];

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SADATA_H__ */
