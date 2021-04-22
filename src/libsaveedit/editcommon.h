/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * editcommon.h: Common definitions for editor components.                 *
 *                                                                         *
 * Copyright (c) 2015-2021 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

/**
 * This file uses information from MainMemory's SAsave editor.
 * - https://github.com/sonicretro/sa_tools/tree/master/SASave
 *
 * Additional references:
 * - http://rnhart.net/articles/sa-savefile.htm
 * - http://info.sonicretro.org/SCHG:Sonic_Adventure/Main_Save_File
 */

// All structs are packed little-endian.

#ifndef __LIBSAVEEDIT_EDITCOMMON_H__
#define __LIBSAVEEDIT_EDITCOMMON_H__

// PACKED struct attribute.
// Use in conjunction with #pragma pack(1).
#ifndef PACKED
#  ifdef __GNUC__
#    define PACKED __attribute__((packed))
#  else
#    define PACKED
#  endif
#endif /* PACKED */

/**
 * static_asserts size of a structure
 * Also defines a constant of form StructName_SIZE
 */
// TODO: Check MSVC support for static_assert() in C mode.
#ifndef ASSERT_STRUCT
#  if defined(__cplusplus)
#    define ASSERT_STRUCT(st,sz) /*enum { st##_SIZE = (sz), };*/ \
	static_assert(sizeof(st)==(sz),#st " is not " #sz " bytes.")
#  elif defined(__GNUC__) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#    define ASSERT_STRUCT(st,sz) /*enum { st##_SIZE = (sz), };*/ \
	_Static_assert(sizeof(st)==(sz),#st " is not " #sz " bytes.")
#  else
#    define ASSERT_STRUCT(st, sz)
#  endif
#endif /* ASSERT_STRUCT */

#endif /* __LIBSAVEEDIT_EDITCOMMON_H__ */
