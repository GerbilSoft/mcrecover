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

#include "config.gcbanner.h"
#include "util/git.h"

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

// Wii banner handler.
#include "wibn.hpp"

/**
 * File type enum.
 */
enum filetype_t {
	FT_UNKNOWN = 0,
	FT_BNR1,
	FT_BNR2,
	FT_WIBN_RAW,	// Dolphin banner.bin
	FT_WIBN_CRYPT,	// Wii encrypted save file

	FT_MAX
};

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
 * Convert a filetype to a string.
 * @param filetype File type.
 * @return String.
 */
static inline string filetype_str(filetype_t filetype)
{
	switch (filetype) {
		case FT_BNR1:
			return "BNR1";
		case FT_BNR2:
			return "BNR2";
		case FT_WIBN_RAW:
			return "WIBN_raw";
		case FT_WIBN_CRYPT:
			return "WIBN_crypt";
		default:
			return "????";
	}
}

/**
 * Identify a given file.
 * @param f File to check.
 * @return File type.
 */
static filetype_t identify_file(FILE *f)
{
	// Check the beginning of the file for a magic number.
	fseek(f, 0, SEEK_SET);
	uint32_t magic_num;
	size_t ret_sz = fread(&magic_num, 1, sizeof(magic_num), f);
	if (ret_sz != sizeof(magic_num))
		return FT_UNKNOWN;

	// Check the magic number.
	magic_num = be32_to_cpu(magic_num);
	switch (magic_num) {
		case BANNER_MAGIC_BNR1:
			return FT_BNR1;
		case BANNER_MAGIC_BNR2:
			return FT_BNR2;
		case BANNER_WIBN_MAGIC:
			return FT_WIBN_RAW;
		default:
			break;
	}

	// Check for a Wii encrypted save file.
	if (!identify_WIBN_crypt(f)) {
		// Encrypted WIBN.
		return FT_WIBN_CRYPT;
	}

	// Unknown file format.
	return FT_UNKNOWN;
}

/**
 * Verify the filesize of a banner.
 * NOTE: This function prints a message on error.
 * @param f File to check.
 * @param ft Expected filetype.
 * @return 0 on success; non-zero on error.
 */
static int verify_filesize(FILE *f, filetype_t ft)
{
	fseek(f, 0, SEEK_END);
	int sz = (int)ftell(f);
	fseek(f, 0, SEEK_SET);

	string ft_str = filetype_str(ft);
	switch (ft) {
		case FT_BNR1:
			if (sz != (int)sizeof(banner_bnr1_t)) {
				fprintf(stderr, "*** ERROR: Banner is type %s, but filesize is %d bytes; expected %d bytes\n",
					ft_str.c_str(), sz, (int)sizeof(banner_bnr1_t));
				return EXIT_FAILURE;
			}
			break;

		case FT_BNR2:
			if (sz != (int)sizeof(banner_bnr2_t)) {
				fprintf(stderr, "*** ERROR: Banner is type %s, but filesize is %d bytes; expected %d bytes\n",
					ft_str.c_str(), sz, (int)sizeof(banner_bnr2_t));
				return EXIT_FAILURE;
			}
			break;

		case FT_WIBN_RAW: {
			int testSz = (sz - BANNER_WIBN_STRUCT_SIZE);
			if (testSz < 0 || testSz % BANNER_WIBN_ICON_SIZE != 0) {
				fprintf(stderr, "*** ERROR: Banner is type %s, but filesize is %d bytes; expected (%d + n*%d) bytes\n",
					ft_str.c_str(), sz,
					BANNER_WIBN_STRUCT_SIZE,
					BANNER_WIBN_ICON_SIZE);
				return EXIT_FAILURE;
			}
			break;
		}

		case FT_WIBN_CRYPT:
			if (sz < 0xF0C0) {
				fprintf(stderr, "*** ERROR: Banner is type %s, but filesize is %d bytes; expected at least %d bytes\n",
					ft_str.c_str(), sz, 0xF0C0);
				return EXIT_FAILURE;
			}
			break;

		default:
			fprintf(stderr, "*** ERROR: Unknown filetype.\n");
			return EXIT_FAILURE;
	}

	// Filesize is valid.
	return EXIT_SUCCESS;
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
		fprintf(stderr, "*** ERROR: read %u bytes from banner; expected %u bytes\n",
			(unsigned int)ret_sz, (unsigned int)sizeof(banner));
		return nullptr;
	}

	// Convert the image data.
	GcImage *gcBanner = GcImage::fromRGB5A3(
				BANNER_IMAGE_W, BANNER_IMAGE_H,
				banner.banner, sizeof(banner.banner));
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
			"Part of GCN MemCard Recover.\n"
			"Copyright (c) 2012-2014 by David Korth.\n"
			"\n"
			"mcrecover version: " MCRECOVER_VERSION_STRING "\n"
#ifdef MCRECOVER_GIT_VERSION
			MCRECOVER_GIT_VERSION "\n"
#ifdef MCRECOVER_GIT_DESCRIBE
			MCRECOVER_GIT_DESCRIBE "\n"
#endif /* MCRECOVER_GIT_DESCRIBE */
#endif /* MCRECOVER_GIT_DESCRIBE */
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

	// Check the file type.
	filetype_t ft = identify_file(f_opening_bnr);
	if (ft <= FT_UNKNOWN || ft >= FT_MAX) {
		fprintf(stderr, "*** ERROR: unable to identify filetype\n");
		fclose(f_opening_bnr);
		return EXIT_FAILURE;
	}

	// Check the file size.
	int ret = verify_filesize(f_opening_bnr, ft);
	if (ret != 0) {
		fprintf(stderr, "*** ERROR: file is invalid\n");
		fclose(f_opening_bnr);
		return EXIT_FAILURE;
	}

	// Get the banner image.
	fseek(f_opening_bnr, 0, SEEK_SET);
	std::auto_ptr<GcImage> gcBanner(nullptr);
	switch (ft) {
		case FT_BNR1:
		case FT_BNR2:
			// BNR1 and BNR2 use the same structure
			// for the image data.
			gcBanner.reset(read_banner_BNR1(f_opening_bnr));
			break;

		case FT_WIBN_RAW:
			// Wii banner image. (banner.bin)
			gcBanner.reset(read_banner_WIBN_raw(f_opening_bnr));
			break;

		case FT_WIBN_CRYPT:
			// Wii banner image. (Wii encrypted save file)
			gcBanner.reset(read_banner_WIBN_crypt(f_opening_bnr));
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
	ret = gcImageWriter.write(gcBanner.get(), GcImageWriter::IMGF_PNG);
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
	size_t ret_sz = fwrite(pngImageData->data(), 1, pngImageData->size(), f_image_png);
	if (ret_sz != pngImageData->size()) {
		fprintf(stderr, "*** ERROR: wrote %u bytes to image; expected %u bytes\n",
			(unsigned int)ret_sz, (unsigned int)pngImageData->size());
		fclose(f_opening_bnr);
		fclose(f_image_png);
		return EXIT_FAILURE;
	}

	// Success!
	fclose(f_opening_bnr);
	fclose(f_image_png);
	printf("%s (%s) -> %s [OK]\n", opening_bnr_filename,
	       filetype_str(ft).c_str(), image_png_filename);
	return EXIT_SUCCESS;
}
