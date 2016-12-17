/***************************************************************************
 * GameCube Banner Extraction Utility.                                     *
 * gcbanner.cpp: Main program.                                             *
 *                                                                         *
 * Copyright (c) 2014-2015 by David Korth.                                 *
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
#include "GcImageLoader.hpp"
#include "GcImageWriter.hpp"

// C includes.
#include <stdlib.h>

// C includes. (C++ namespace)
#include <cstdio>
#include <cerrno>
#include <cstring>

// C++ includes.
#include <locale>
using std::locale;

// getopt_long()
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#ifndef HAVE_GETOPT_LONG
#include "vlc_getopt/vlc_getopt.h"
// Wrappers to make vlc_getopt_long() act like GNU getopt_long().
static vlc_getopt_t vlc_getopt_state;
#define getopt_long(argc, argv, optstring, longopts, longindex) \
	vlc_getopt_long(argc, argv, optstring, longopts, longindex, &vlc_getopt_state)
#define option vlc_option
#define optarg vlc_getopt_state.arg
#define optind vlc_getopt_state.ind
#endif /* HAVE_GETOPT_LONG */

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
	FT_WIBN_CRYPT,	// Encrypted Wii save file

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

	// Check for an encrypted Wii save file.
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
	GcImage *gcBanner = GcImageLoader::fromRGB5A3(
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
 * Create a new filename based on a given filename.
 * @param filename Original filename.
 * @param suffix New suffix.
 * @return New filename.
 */
static string create_filename(const char *filename, const char *suffix)
{
	// image.png was not specified.
	// Remove the extension from the current file (if any),
	// and replace it with .png.
	string tmp_filename(filename);
	int dot_pos = tmp_filename.find_last_of('.');
	int slash_pos = tmp_filename.find_last_of('/');
#ifdef _WIN32
	int bslash_pos = tmp_filename.find_last_of('\\');
	if (bslash_pos > slash_pos)
		slash_pos = bslash_pos;
#endif /* _WIN32 */
	if (dot_pos > slash_pos) {
		// File extension. Remove it.
		tmp_filename.erase(dot_pos);
	}

	// Append the new extension.
	tmp_filename.append(suffix);
	return tmp_filename;
}

/**
 * Create a new filename based on a given filename.
 * @param filename Original filename.
 * @param suffix New suffix.
 * @return New filename.
 */
static inline string create_filename(const std::string &filename, const char *suffix)
{
	return create_filename(filename.c_str(), suffix);
}

/**
 * Extract the banner image from the specified GCN/Wii banner.
 * @param f opening.bnr file.
 * @param ft Filetype.
 * @param opening_bnr_filename opening.bnr filename.
 * @param banner_png_filename Filename for banner.png.
 */
static int extract_banner(FILE *f, filetype_t ft,
		const char *opening_bnr_filename,
		const char *banner_png_filename)
{
	fseek(f, 0, SEEK_SET);
	std::auto_ptr<GcImage> gcBanner(nullptr);
	switch (ft) {
		case FT_BNR1:
		case FT_BNR2:
			// BNR1 and BNR2 use the same structure
			// for the image data.
			gcBanner.reset(read_banner_BNR1(f));
			break;

		case FT_WIBN_RAW:
			// Wii banner image. (banner.bin)
			gcBanner.reset(read_banner_WIBN_raw(f));
			break;

		case FT_WIBN_CRYPT:
			// Wii banner image. (Encrypted Wii save file)
			gcBanner.reset(read_banner_WIBN_crypt(f));
			break;

		default:
			gcBanner.reset(nullptr);
			break;
	}

	if (!gcBanner.get()) {
		fprintf(stderr, "*** ERROR: could not read banner image.\n");
		return -1;
	}

	// Determine the destination filename.
	string s_banner_png_filename =
			(banner_png_filename
				? string(banner_png_filename)
				: create_filename(opening_bnr_filename, ".banner.png"));
		
	// Open the destination file.
	// TODO: Delete on failure?
	FILE *f_banner_png = fopen(s_banner_png_filename.c_str(), "wb");
	if (!f_banner_png) {
		fprintf(stderr, "*** ERROR opening file %s: %s\n",
			s_banner_png_filename.c_str(), strerror(errno));
		return -2;
	}

	// Write to PNG.
	GcImageWriter gcImageWriter;
	int ret = gcImageWriter.write(gcBanner.get(), GcImageWriter::IMGF_PNG);
	if (ret != 0) {
		fprintf(stderr, "*** ERROR: GcImageWriter::write() failed: %d\n", ret);
		fclose(f_banner_png);
		return -3;
	}

	// Get the PNG image data.
	const vector<uint8_t> *pngImageData = gcImageWriter.memBuffer();
	if (!pngImageData || pngImageData->empty()) {
		fprintf(stderr, "*** ERROR: GcImageWriter has no image data.\n");
		fclose(f_banner_png);
		return -4;
	}

	// Write the PNG image data.
	size_t ret_sz = fwrite(pngImageData->data(), 1, pngImageData->size(), f_banner_png);
	if (ret_sz != pngImageData->size()) {
		fprintf(stderr, "*** ERROR: wrote %u bytes to image; expected %u bytes\n",
			(unsigned int)ret_sz, (unsigned int)pngImageData->size());
		fclose(f_banner_png);
		return -5;
	}

	// Success!
	fclose(f_banner_png);
	printf("%s (%s) banner -> %s [%s] [OK]\n",
		opening_bnr_filename,
		filetype_str(ft).c_str(),
		s_banner_png_filename.c_str(),
		GcImageWriter::nameOfImageFormat(GcImageWriter::IMGF_PNG));
	return 0;
}

/**
 * Extract the icon from the specified GCN/Wii banner.
 * @param f opening.bnr file.
 * @param ft Filetype.
 * @param animImgf Animated image format.
 * @param opening_bnr_filename opening.bnr filename.
 * @param icon_png_filename Filename for icon.png.
 */
static int extract_icon(FILE *f, filetype_t ft,
		GcImageWriter::AnimImageFormat animImgf,
		const char *opening_bnr_filename,
		const char *icon_png_filename)
{
	fseek(f, 0, SEEK_SET);
	GcImageWriter gcImageWriter;
	const char *imgf_str = nullptr;
	int ret = -EINVAL;
	switch (ft) {
		case FT_BNR1:
		case FT_BNR2:
			// BNR1 and BNR2 don't have icons.
			printf("%s (%s) icon -> no icons\n", opening_bnr_filename,
				filetype_str(ft).c_str());
			return 0;

		case FT_WIBN_RAW:
			// Wii banner image. (banner.bin)
			ret = read_icon_WIBN_raw(f, &gcImageWriter, animImgf, &imgf_str);
			break;

		case FT_WIBN_CRYPT:
			// Wii banner image. (Encrypted Wii save file)
			ret = read_icon_WIBN_crypt(f, &gcImageWriter, animImgf, &imgf_str);
			break;

		default:
			break;
	}

	if (ret != 0 || !imgf_str || gcImageWriter.numFiles() <= 0) {
		fprintf(stderr, "*** ERROR: could not read icon.\n");
		return -1;
	}

	// Determine the destination filename.
	string s_icon_png_filename;
	const char *ext = GcImageWriter::extForAnimImageFormat(animImgf);
	if (icon_png_filename) {
		s_icon_png_filename = string(icon_png_filename);
	} else {
		string suffix(".icon.");
		suffix.append(ext);
		s_icon_png_filename = create_filename(opening_bnr_filename, suffix.c_str());
	}

	// TODO: Better error messages.

	// Open the destination file.
	// TODO: Delete on failure?
	for (int i = 0; i < gcImageWriter.numFiles(); i++) {
		string filename;
		if (gcImageWriter.numFiles() > 1) {
			// Multiple files.
			// Append the file number.
			char tmp[16];
			snprintf(tmp, sizeof(tmp), ".%02d.%s", i+1, ext);
			filename = create_filename(s_icon_png_filename, tmp);
		} else {
			// Single file.
			filename = s_icon_png_filename;
		}

		FILE *f_icon_png = fopen(filename.c_str(), "wb");
		if (!f_icon_png) {
			fprintf(stderr, "*** ERROR opening file %s: %s\n",
				filename.c_str(), strerror(errno));
			return -2;
		}

		// Get the image data.
		const vector<uint8_t> *pngData = gcImageWriter.memBuffer(i);
		if (!pngData || pngData->empty()) {
			fprintf(stderr, "*** ERROR: GcImageWriter has no data for image %d.\n", i);
			fclose(f_icon_png);
			return -3;
		}

		// Write the image data.
		size_t ret_sz = fwrite(pngData->data(), 1, pngData->size(), f_icon_png);
		if (ret_sz != pngData->size()) {
			fprintf(stderr, "*** ERROR: wrote %u bytes to image; expected %u bytes\n",
				(unsigned int)ret_sz, (unsigned int)pngData->size());
			fclose(f_icon_png);
			return -4;
		}

		// Image written successfully.
		fclose(f_icon_png);
	}

	// Success!
	printf("%s (%s) icon -> %s [%s] [OK]\n",
		opening_bnr_filename,
		filetype_str(ft).c_str(),
		s_icon_png_filename.c_str(),
		imgf_str);
	return 0;
}

/**
 * Get the default animated image format.
 * @return Default animated image format.
 */
static GcImageWriter::AnimImageFormat default_anim_image_format(void)
{
	for (int i = 1; i < GcImageWriter::ANIMGF_MAX; i++) {
		GcImageWriter::AnimImageFormat animImgf = (GcImageWriter::AnimImageFormat)i;
		if (GcImageWriter::isAnimImageFormatSupported(animImgf))
			return animImgf;
	}

	// No supported formats...
	return GcImageWriter::ANIMGF_UNKNOWN;
}

/**
 * Show program version information.
 */
static void show_version(void)
{
	printf("GameCube Banner Extraction Utility\n"
		"Part of GCN MemCard Recover.\n"
		"Copyright (c) 2012-2015 by David Korth.\n"
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
		"See http://www.gnu.org/licenses/gpl-2.0.html for more information.\n");
}

/**
 * Print supported animated image formats.
 * @param f FILE* for output.
 */
static void print_supported_animImgf(FILE *f)
{
	// Animated icon formats.
	GcImageWriter::AnimImageFormat defAnimImgf = default_anim_image_format();
	printf("Supported animated image formats:\n");
	for (int i = 1; i < GcImageWriter::ANIMGF_MAX; i++) {
		GcImageWriter::AnimImageFormat animImgf = (GcImageWriter::AnimImageFormat)i;
		if (GcImageWriter::isAnimImageFormatSupported(animImgf)) {
			fprintf(f, "- %s: %s",
				GcImageWriter::nameOfAnimImageFormat(animImgf),
				GcImageWriter::descOfAnimImageFormat(animImgf));
			if (animImgf == defAnimImgf)
				fprintf(f, " (default)");
			fprintf(f, "\n");
		}
	}
}

/**
 * Show program usage information.
 * @param argv0 argv[0]
 */
static void show_usage(const char *argv0)
{
	show_version();
	printf("\n"
		"Usage: %s opening.bnr [OPTION]... [banner.png] [icon.png]\n"
		"Extract a banner, icon, or both from a GameCube or Wii banner file.\n"
		"Supported formats: BNR1, BNR2, WIBN_raw, WIBN_crypt\n"
		"\n"
		"Options:\n"
		"  -b, --banner\t\t\tExtract the banner.\n"
		"  -B, --banner-as=FILENAME\tExtract the banner to FILENAME.\n"
		"  -i, --icon\t\t\tExtract the icon.\n"
		"  -I, --icon-as=FILENAME\tExtract the icon to FILENAME.\n"
		"  -f, --format=FORMAT\t\tFormat to use for animated icons.\n"
		"  -h, --help\t\t\tDisplay this help and exit.\n"
		"      --version\t\t\tOutput version information and exit.\n"
		"\n"
		"If banner.png and icon.png are specified, they override -B and -I.\n"
		"If -b or -i are specified and filenames are not specified, a filename\n"
		"will be generated based on the filename of opening.bnr.\n"
		"If only opening.bnr is specified, only the banner will be extracted.\n"
		"\n"
		, argv0);
	print_supported_animImgf(stdout);
}

/**
 * Main entry point.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @return Return value.
 */
int main(int argc, char *argv[])
{
	// Set the C and C++ locales.
	locale::global(locale(""));

	/**
	 * Syntax:
	 * ./gcbanner opening.bnr [image.png]
	 * - opening.bnr: GameCube banner to convert.
	 * - image.png: Output image. (If omitted, defaults to renamed banner.)
	 */
	static struct option long_options[] = {
		{"banner",	no_argument,		nullptr, 'b'},
		{"banner-as",	required_argument,	nullptr, 'B'},
		{"icon",	no_argument,		nullptr, 'i'},
		{"icon-as",	required_argument,	nullptr, 'I'},
		{"format",	required_argument,	nullptr, 'f'},
		{"help",    	no_argument,		nullptr, 'h'},
		// NOTE: -V is not valid.
		{"version",	no_argument,		nullptr, 'V'},
		{nullptr, 0, nullptr, 0}
	};

	// NOTE: If both are false after parsing all arguments,
	// default to doBanner = true.
	bool doBanner = false, doIcon = false;
	const char *banner_png_filename = nullptr;
	const char *icon_png_filename = nullptr;
	GcImageWriter::AnimImageFormat animImgf = default_anim_image_format();

	int c, option_index;
	while ((c = getopt_long(argc, argv, "bB:iI:f:h", long_options, &option_index)) != -1) {
		switch (c) {
			case 'b':
				doBanner = true;
				break;
			case 'B':
				doBanner = true;
				banner_png_filename = optarg;
				break;
			case 'i':
				doIcon = true;
				break;
			case 'I':
				doIcon = true;
				icon_png_filename = optarg;
				break;
			case 'f':
				animImgf = GcImageWriter::animImageFormatFromName(optarg);
				if (animImgf != GcImageWriter::ANIMGF_UNKNOWN) {
					// Check if the format is actually supported.
					if (!GcImageWriter::isAnimImageFormatSupported(animImgf))
						animImgf = GcImageWriter::ANIMGF_UNKNOWN;
				}
				if (animImgf == GcImageWriter::ANIMGF_UNKNOWN) {
					fprintf(stderr, "%s: invalid argument '%s' for 'animated image format'\n",
						argv[0], optarg);
					print_supported_animImgf(stderr);
					fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
					return EXIT_FAILURE;
				}
				break;
			case 'h':
				show_usage(argv[0]);
				return EXIT_SUCCESS;
			case 'V':
				show_version();
				return EXIT_SUCCESS;
			case '?':
			default:
				// Invalid option.
				fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
				return EXIT_FAILURE;
		}
	}

	// If neither -b/-B nor -i/-I were specified,
	// default to -b.
	if (!doBanner && !doIcon)
		doBanner = true;

	// Check if any filenames were specified.
	if (optind == argc) {
		fprintf(stderr, "%s: missing opening.bnr operand\n"
			"Try '%s --help' for more information.\n", argv[0], argv[0]);
		return EXIT_FAILURE;
	}

	const char *opening_bnr_filename = nullptr;
	if (optind+1 <= argc) {
		// opening.bnr
		opening_bnr_filename = argv[optind];
	}
	if (optind+2 <= argc) {
		// banner.png
		doBanner = true;
		banner_png_filename = argv[optind+1];
	}
	if (optind+3 <= argc) {
		// icon.png
		doIcon = true;
		icon_png_filename = argv[optind+2];
	}
	if (optind+4 <= argc) {
		// Too many parameters.
		fprintf(stderr, "%s: too many parameters specified\n"
			"Try '%s --help' for more information.\n", argv[0], argv[0]);
		return EXIT_FAILURE;
	}

	// Open the GameCube banner file.
	FILE *f_opening_bnr = fopen(opening_bnr_filename, "rb");
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

	// Extract the banner image.
	if (doBanner) {
		ret = extract_banner(f_opening_bnr, ft,
				opening_bnr_filename,
				banner_png_filename);
		if (ret != 0)
			goto end;
	}

	// Extract the icon.
	if (doIcon) {
		ret = extract_icon(f_opening_bnr, ft, animImgf,
				opening_bnr_filename,
				icon_png_filename);
		if (ret != 0)
			goto end;
	}

end:
	fclose(f_opening_bnr);
	return (ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
