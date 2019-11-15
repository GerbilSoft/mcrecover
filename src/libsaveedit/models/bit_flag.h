/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * bit_flag.h: Bit flag description struct.                                *
 *                                                                         *
 * Copyright (c) 2015-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBSAVEEDIT_MODELS_BIT_FLAG_H__
#define __LIBSAVEEDIT_MODELS_BIT_FLAG_H__

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

#endif /* __LIBSAVEEDIT_MODELS_BIT_FLAG_H__ */
