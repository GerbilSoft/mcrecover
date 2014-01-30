/***************************************************************************
 * GameCube Banner Extraction Utility.                                     *
 * wibn.hpp: Wii banner handler.                                           *
 *                                                                         *
 * Copyright (c) 2014 by David Korth.                                      *
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

#ifndef __GCBANNER_WIBN_HPP__
#define __GCBANNER_WIBN_HPP__

// C includes. (C++ namespace)
#include <cstdio>

class GcImage;
class GcImageWriter;

/**
 * Check if the specified file is WIBN_crypt.
 * @return 0 if WIBN_crypt; non-zero if not.
 */
int identify_WIBN_crypt(FILE *f);

GcImage *read_banner_WIBN_raw(FILE *f);
GcImage *read_banner_WIBN_crypt(FILE *f);

int read_icon_WIBN_raw(FILE *f, GcImageWriter *gcImageWriter);
int read_icon_WIBN_crypt(FILE *f, GcImageWriter *gcImageWriter);

#endif /* __GCBANNER_WIBN_HPP__ */