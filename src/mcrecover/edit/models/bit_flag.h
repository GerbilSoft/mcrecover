/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * bit_flag.h: Bit flag description struct.                                *
 *                                                                         *
 * Copyright (c) 2015-2016 by David Korth.                                 *
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

#ifndef __MCRECOVER_EDIT_MODELS_BIT_FLAG_H__
#define __MCRECOVER_EDIT_MODELS_BIT_FLAG_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _bit_flag_t {
	int event;
	const char *description;
} bit_flag_t;

#ifdef __cplusplus
}
#endif

#endif /* __MCRECOVER_EDIT_MODELS_BIT_FLAG_H__ */
