/***************************************************************************
 * GameCube Banner Extraction Utility.                                     *
 * gcbanner.cpp: Main program.                                             *
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

// Banner struct.
#include "banner.h"
#include "util/byteswap.h"

// GameCube image handlers.
#include "GcImage.hpp"
#include "GcImageWriter.hpp"

// C includes. (C++ namespace)
#include <cstdio>
#include <cerrno>
#include <cstring>

// C includes.
#include <stdlib.h>

// C++ includes.
#include <vector>
#include <string>
#include <memory>
using std::vector;
using std::string;
using std::auto_ptr;

/**
 * Convert a magic number to a string.
 * @param magic_num Magic number. (DO NOT be32_to_cpu() this value!)
 * @return String.
 */
static inline string magic_num_str(uint32_t magic_num)
{
	return string((const char*)&magic_num, sizeof(magic_num));
}

/**
 * Read the banner image from a BNR1/BNR2 banner.
 * @param f File containing the banner.
 * @return GcImage containing the banner image, or nullptr on error.
 */
static GcImage *read_banner_BNR1(FILE *f)
{
	// Read the banner.
	banner_bnr1_t banner;
	memset(&banner, 0x00, sizeof(banner));
	fseek(f, 0, SEEK_SET);
	size_t ret_sz = fread(&banner, 1, sizeof(banner), f);
	if (ret_sz != sizeof(banner)) {
		fprintf(stderr, "*** ERROR: read %lu bytes from banner; expected %lu bytes\n",
			ret_sz, sizeof(banner));
		return nullptr;
	}

	// Convert the image data.
	GcImage *gcBanner = GcImage::fromRGB5A3(
				BANNER_IMAGE_W, BANNER_IMAGE_H,
				banner.image, sizeof(banner.image));
	if (!gcBanner) {
		fprintf(stderr, "*** ERROR: Could not convert %s banner image from RGB5A3.\n",
			magic_num_str(banner.magic).c_str());
		return nullptr;
	}

	return gcBanner;
}

/**
 * Main entry point.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @return Return value.
 */
int main(int argc, char *argv[])
{
	/**
	 * Syntax:
	 * ./gcbanner opening.bnr [image.png]
	 * - opening.bnr: GameCube banner to convert.
	 * - image.png: Output image. (If omitted, defaults to renamed banner.)
	 */

	if (argc == 1 || argc > 4) {
		fprintf(stderr,
			"GameCube Banner Extraction Utility\n"
			"Copyright (c) 2014 by David Korth.\n"
			"\n"
			"This program is licensed under the GNU GPL v2.\n"
			"See http://www.gnu.org/licenses/gpl-2.0.html for more information.\n"
			"\n"
			"Syntax: %s opening.bnr [image.png]\n"
			"- opening.bnr: GameCube banner to convert.\n"
			"- image.png: Output image. (If omitted, defaults to renamed banner.)\n"
			, argv[0]);
		return EXIT_FAILURE;
	}

	// Open the GameCube banner file.
	const char *opening_bnr_filename = argv[1];
	FILE *f_opening_bnr = fopen(argv[1], "rb");
	if (!f_opening_bnr) {
		fprintf(stderr, "*** ERROR opening file %s: %s\n",
			opening_bnr_filename, strerror(errno));
		return EXIT_FAILURE;
	}

	// Make sure this is a valid banner file.
	fseek(f_opening_bnr, 0, SEEK_END);
	long opening_bnr_sz = ftell(f_opening_bnr);
	fseek(f_opening_bnr, 0, SEEK_SET);

	if (opening_bnr_sz != sizeof(banner_bnr1_t) &&
	    opening_bnr_sz != sizeof(banner_bnr2_t)) {
		// Not a valid size.
		fprintf(stderr, "*** ERROR: %s has invalid size %ld; should be %lu or %lu\n",
			opening_bnr_filename, opening_bnr_sz,
			sizeof(banner_bnr1_t), sizeof(banner_bnr2_t));
		fclose(f_opening_bnr);
		return EXIT_FAILURE;
	}

	// Check the magic number.
	uint32_t magic_num;
	size_t ret_sz = fread(&magic_num, 1, sizeof(magic_num), f_opening_bnr);
	if (ret_sz != sizeof(magic_num)) {
		fprintf(stderr, "*** ERROR: could not read magic number from banner\n");
		fclose(f_opening_bnr);
		return EXIT_FAILURE;
	}

	// Convert the magic number into a printable string.
	const char *expectedMagic;
	char magic[5];
	memcpy(magic, &magic_num, 4);
	magic[4] = 0;
	magic_num = be32_to_cpu(magic_num);

	// Check the magic number.
	bool isMagicValid = true;
	if (opening_bnr_sz == sizeof(banner_bnr1_t)) {
		if (magic_num != BANNER_MAGIC_BNR1) {
			isMagicValid = false;
			expectedMagic = "BNR1";
		}
	} else if (opening_bnr_sz == sizeof(banner_bnr2_t)) {
		if (magic_num != BANNER_MAGIC_BNR2) {
			isMagicValid = false;
			expectedMagic = "BNR2";
		}		
	} else {
		// Should not get here...
		isMagicValid = false;
		expectedMagic = "????";
	}

	if (!isMagicValid) {
		fprintf(stderr, "*** ERROR: read magic number %s, expected %s\n",
			magic, expectedMagic);
		fclose(f_opening_bnr);
		return EXIT_FAILURE;
	}

	// Get the banner image.
	fseek(f_opening_bnr, 0, SEEK_SET);
	std::auto_ptr<GcImage> gcBanner(nullptr);
	switch (magic_num) {
		case BANNER_MAGIC_BNR1:
		case BANNER_MAGIC_BNR2:
			// BNR1 and BNR2 use the same structure
			// for the image data.
			gcBanner.reset(read_banner_BNR1(f_opening_bnr));
			break;

		default:
			gcBanner.reset(nullptr);
			break;
	}

	if (!gcBanner.get()) {
		fprintf(stderr, "*** ERROR: could not read banner image.\n");
		fclose(f_opening_bnr);
		return EXIT_FAILURE;
	}

	// Determine the destination filename.
	const char *image_png_filename;
	if (argc >= 3) {
		// image.png was specified.
		image_png_filename = argv[2];
	} else {
		// image.png was not specified.
		// Remove the extension from the current file (if any),
		// and replace it with .png.
		string png_filename(opening_bnr_filename);
		int dot_pos = png_filename.find_last_of('.');
		int slash_pos = png_filename.find_last_of('/');
#ifdef _WIN32
		int bslash_pos = png_filename.find_last_of('\\');
		if (bslash_pos > slash_pos)
			slash_pos = bslash_pos;
#endif /* _WIN32 */
		if (dot_pos > slash_pos) {
			// File extension. Remove it.
			png_filename.erase(dot_pos);
		}

		// Append the new extension.
		png_filename.append(".png");

		// strdup() it to image_png_filename.
		// NOTE: This results in a "memory leak", but since
		// the program is short-lived, we don't care.
		image_png_filename = strdup(png_filename.c_str());
	}

	// Open the destination file.
	// TODO: Delete on failure?
	FILE *f_image_png = fopen(image_png_filename, "wb");
	if (!f_image_png) {
		fprintf(stderr, "*** ERROR opening file %s: %s\n",
			image_png_filename, strerror(errno));
		fclose(f_opening_bnr);
		return EXIT_FAILURE;
	}

	// Write to PNG.
	GcImageWriter gcImageWriter;
	int ret = gcImageWriter.write(gcBanner.get(), GcImageWriter::IMGF_PNG);
	if (ret != 0) {
		fprintf(stderr, "*** ERROR: GcImageWriter::write() failed: %d\n", ret);
		fclose(f_opening_bnr);
		fclose(f_image_png);
		return EXIT_FAILURE;
	}

	// Get the PNG image data.
	const vector<uint8_t> *pngImageData = gcImageWriter.memBuffer();
	if (!pngImageData || pngImageData->empty()) {
		fprintf(stderr, "*** ERROR: GcImageWriter has no PNG image data.\n");
		fclose(f_opening_bnr);
		fclose(f_image_png);
		return EXIT_FAILURE;
	}

	// Write the PNG image data.
	ret_sz = fwrite(pngImageData->data(), 1, pngImageData->size(), f_image_png);
	if (ret_sz != pngImageData->size()) {
		fprintf(stderr, "*** ERROR: wrote %lu bytes to image; expected %lu bytes\n",
			ret_sz, pngImageData->size());
		fclose(f_opening_bnr);
		fclose(f_image_png);
		return EXIT_FAILURE;
	}

	// Success!
	printf("%s -> %s [OK]\n", opening_bnr_filename, image_png_filename);
	return EXIT_SUCCESS;
}
