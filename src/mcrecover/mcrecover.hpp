/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * mcrecover.hpp: Main program.                                            *
 *                                                                         *
 * Copyright (c) 2011-2013 by David Korth.                                 *
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

#ifndef __MCRECOVER_MCRECOVER_HPP__
#define __MCRECOVER_MCRECOVER_HPP__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32
#define mcrecover_main main
#endif

/**
 * Main entry point.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @return Return value.
 */
int mcrecover_main(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif /* __MCRECOVER_MCRECOVER_HPP__ */
