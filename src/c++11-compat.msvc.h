/***************************************************************************
 * c++11-compat.msvc.h: C++ 2011 compatibility header. (MSVC)              *
 *                                                                         *
 * Copyright (c) 2011-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#ifndef _MSC_VER
#error c++11-compat.msvc.h should only be included in MSVC builds.
#endif

/** C++ 2011 **/

#ifdef __cplusplus

/**
 * Enable compatibility for C++ 2011 features that aren't
 * present in older versions of MSVC.
 *
 * These are all automatically enabled when compiling C code.
 *
 * Reference: https://msdn.microsoft.com/en-us/library/hh567368.aspx
 */

#if (_MSC_VER < 1900)
/**
 * MSVC 2015 (14.0) added support for Unicode character types.
 * (char16_t, char32_t, related string types)
 */
#define CXX11_COMPAT_CHARTYPES

/**
 * MSVC 2015 also added support for constexpr.
 */
#define CXX11_COMPAT_CONSTEXPR
#endif

#if (_MSC_VER < 1700)
/**
 * MSVC 2010 (10.0) does support override, but not final.
 * However, it has a "sealed" keyword that works almost
 * the same way as final.
 *
 * 'sealed' is available starting with MSVC 2005 (8.0).
 * TODO: Verify MSVC 2002 and 2003.
 */
#if (_MSC_VER >= 1400)
#define final sealed
#endif
#endif

#if (_MSC_VER < 1600)
/**
 * MSVC 2008 (9.0) and older: No C++ 2011 support at all.
 * Probably won't compile at all due to lack of stdint.h.
 */
#define CXX11_COMPAT_NULLPTR
#define CXX11_COMPAT_OVERRIDE
#define CXX11_COMPAT_STATIC_ASSERT
#endif

#endif /* __cplusplus */

/**
 * MSVC doesn't have typeof(), but as of MSVC 2010,
 * it has decltype(), which is essentially the same thing.
 * TODO: Handle older versions.
 * Possible option for C++:
 * - http://www.nedproductions.biz/blog/implementing-typeof-in-microsofts-c-compiler
 */
#define typeof(x) decltype(x)
