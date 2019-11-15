/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * mcrecover.hpp: Main program.                                            *
 *                                                                         *
 * Copyright (c) 2011-2013 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
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
