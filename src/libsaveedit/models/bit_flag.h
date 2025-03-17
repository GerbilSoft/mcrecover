/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * bit_flag.h: Bit flag description struct.                                *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

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
