/***************************************************************************
 * GameCube Tools Library.                                                 *
 * GcImage.hpp: GameCube image format handler. (PRIVATE)                   *
 *                                                                         *
 * Copyright (c) 2012-2016 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBGCTOOLS_GCIMAGE_P_HPP__
#define __LIBGCTOOLS_GCIMAGE_P_HPP__

#include "GcImage.hpp"

// C includes. (C++ namespace)
#include <cstdlib>
// C++ includes.
#include <vector>

class GcImagePrivate
{
	public:
		GcImagePrivate();
		~GcImagePrivate();
		// Copy constructor.
		GcImagePrivate(const GcImagePrivate &other);
	private:
		// Assign constructor. (TODO)
		GcImagePrivate &operator=(const GcImagePrivate &other);

	public:
		/**
		 * Initialize the GcImage.
		 * @param w Width.
		 * @param h Height.
		 * @param pxFmt Pixel format.
		 */
		void init(int w, int h, GcImage::PxFmt pxFmt);

		void *imageData;
		size_t imageData_len;
		std::vector<uint32_t> palette;
		GcImage::PxFmt pxFmt;
		int width;
		int height;
};

#endif /* __LIBGCTOOLS_GCIMAGE_P_HPP__ */
