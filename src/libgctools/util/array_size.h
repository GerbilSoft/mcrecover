/***************************************************************************
 * GameCube Tools Library.                                                 *
 * array_size.h: ARRAY_SIZE() macro.                                       *
 *                                                                         *
 * Copyright (c) 2014 by David Korth.                                      *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBGCTOOLS_UTIL_ARRAY_SIZE_H__
#define __LIBGCTOOLS_UTIL_ARRAY_SIZE_H__

/**
 * Number of elements in an array.
 *
 * Includes a static check for pointers to make sure
 * a dynamically-allocated array wasn't specified.
 * Reference: http://stackoverflow.com/questions/8018843/macro-definition-array-size
 */
#define ARRAY_SIZE(x) \
	((int)(((sizeof(x) / sizeof(x[0]))) / \
		(size_t)(!(sizeof(x) % sizeof(x[0])))))

#endif /* __LIBGCTOOLS_UTIL_ARRAY_SIZE_H__ */
