/***************************************************************************
 * GameCube Tools Library.                                                 *
 * GcImage.hpp: GameCube image format handler. (PRIVATE)                   *
 *                                                                         *
 * Copyright (c) 2012-2015 by David Korth.                                 *
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
