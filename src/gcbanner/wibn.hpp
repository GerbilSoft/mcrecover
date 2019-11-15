/***************************************************************************
 * GameCube Banner Extraction Utility.                                     *
 * wibn.hpp: Wii banner handler.                                           *
 *                                                                         *
 * Copyright (c) 2014 by David Korth.                                      *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __GCBANNER_WIBN_HPP__
#define __GCBANNER_WIBN_HPP__

// C includes. (C++ namespace)
#include <cstdio>

#include "GcImageWriter.hpp"

/**
 * Check if the specified file is WIBN_crypt.
 * @return 0 if WIBN_crypt; non-zero if not.
 */
int identify_WIBN_crypt(FILE *f);

GcImage *read_banner_WIBN_raw(FILE *f);
GcImage *read_banner_WIBN_crypt(FILE *f);

int read_icon_WIBN_raw(FILE *f, GcImageWriter *gcImageWriter,
			GcImageWriter::AnimImageFormat animImgf,
			const char **imgf_str);
int read_icon_WIBN_crypt(FILE *f, GcImageWriter *gcImageWriter,
			GcImageWriter::AnimImageFormat animImgf,
			const char **imgf_str);

#endif /* __GCBANNER_WIBN_HPP__ */
