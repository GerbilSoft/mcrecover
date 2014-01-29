/***************************************************************************
 * GameCube Banner Extraction Utility.                                     *
 * wibn.cpp: Wii banner handler.                                           *
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

#include "wibn.hpp"

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

// Wii encryption keys.
#include "rijndael.h"
static const uint8_t Wii_SDKey[16] =
	{0xAB, 0x01, 0xB9, 0xD8, 0xE1, 0x62, 0x2B, 0x08,
         0xAF, 0xBA, 0xD8, 0x4D, 0xBF, 0xC2, 0xA5, 0x5D};
static const uint8_t Wii_SDIV[16] =
	{0x21, 0x67, 0x12, 0xE6, 0xAA, 0x1F, 0x68, 0x9F,
	 0x95, 0xC5, 0xA2, 0x23, 0x24, 0xDC, 0x6A, 0x98};

/**
 * Check if the specified file is WIBN_crypt.
 * @return 0 if WIBN_crypt; non-zero if not.
 */
int identify_WIBN_crypt(FILE* f)
{
	uint8_t header_encrypted[64];
	uint8_t header_decrypted[64];
	fseek(f, 0, SEEK_SET);
	size_t ret_sz = fread(header_encrypted, 1, sizeof(header_encrypted), f);
	if (ret_sz != sizeof(header_encrypted))
		return -1;

	// Decrypt the header.
	aes_set_key(Wii_SDKey);
	uint8_t iv[sizeof(Wii_SDIV)];
	memcpy(iv, Wii_SDIV, sizeof(iv));
	aes_decrypt(iv, header_encrypted, header_decrypted, sizeof(header_encrypted));

	// Check the magic number.
	uint32_t magic_num;
	memcpy(&magic_num, &header_decrypted[0x20], sizeof(magic_num));
	magic_num = be32_to_cpu(magic_num);
	if (magic_num == BANNER_WIBN_MAGIC) {
		// Encrypted WIBN.
		return 0;
	}

	// Not WIBN_crypt.
	return 1;
}

/**
 * Read the banner image from a WIBN banner.
 * Internal function; requires loaded, decrypted data.
 * @param banner Pointer to WIBN banner.
 * @param crypted If true, this is an encrypted banner.
 * @return GcImage containing the banner image, or nullptr on error.
 */
static GcImage *read_banner_WIBN_internal(const banner_wibn_t *banner, bool crypted)
{
	// Convert the image data.
	GcImage *gcBanner = GcImage::fromRGB5A3(
				BANNER_WIBN_IMAGE_W, BANNER_WIBN_IMAGE_H,
				banner->banner, sizeof(banner->banner));
	if (!gcBanner) {
		fprintf(stderr, "*** ERROR: Could not convert %s banner image from RGB5A3.\n",
			(crypted ? "WIBN_crypt" : "WIBN_raw"));
		return nullptr;
	}

	return gcBanner;
}

/**
 * Read the banner image from a WIBN banner.
 * Raw version; for extracted banner.bin files.
 * @param f File containing the banner.
 * @return GcImage containing the banner image, or nullptr on error.
 */
GcImage *read_banner_WIBN_raw(FILE *f)
{
	// Read the banner.
	banner_wibn_t banner;
	memset(&banner, 0x00, sizeof(banner));
	fseek(f, 0, SEEK_SET);
	size_t ret_sz = fread(&banner, 1, sizeof(banner), f);

	// Banner size varies depending on number of icons.
	int numIcons = -1;
	if (ret_sz >= BANNER_WIBN_STRUCT_SIZE) {
		int sz = (int)(ret_sz - BANNER_WIBN_STRUCT_SIZE);
		if (sz % BANNER_WIBN_ICON_SIZE == 0) {
			numIcons = sz / BANNER_WIBN_ICON_SIZE;
			if (numIcons > 8) {
				// Too many icons.
				// This shouldn't happen...
				numIcons = -1;
			}
		}
	}

	if (numIcons < 0) {
		fprintf(stderr, "*** ERROR: read %u bytes from banner; expected %u + (%u * n) bytes\n",
			(unsigned int)ret_sz, BANNER_WIBN_STRUCT_SIZE, BANNER_WIBN_ICON_SIZE);
		return nullptr;
	}

	// Convert the image data.
	return read_banner_WIBN_internal(&banner, false);
}

/**
 * Read the banner image from a WIBN banner.
 * Encrypted version; for encrypted Wii save files.
 * @param f File containing the banner.
 * @return GcImage containing the banner image, or nullptr on error.
 */
GcImage *read_banner_WIBN_crypt(FILE *f)
{
	// Read the encrypted banner data.
	banner_wibn_t banner;
	uint8_t banner_encrypted[sizeof(banner)];

	// Read the banner.
	fseek(f, 0, SEEK_SET);
	size_t ret_sz = fread(banner_encrypted, 1, sizeof(banner_encrypted), f);
	if (ret_sz != sizeof(banner_encrypted)) {
		fprintf(stderr, "*** ERROR: read %u bytes from banner; expected %u bytes\n",
			(unsigned int)ret_sz, (unsigned int)sizeof(banner_encrypted));
		return nullptr;
	}

	// Decrypt the banner.
	aes_set_key(Wii_SDKey);
	uint8_t iv[sizeof(Wii_SDIV)];
	memcpy(iv, Wii_SDIV, sizeof(iv));
	aes_decrypt(iv, banner_encrypted, (uint8_t*)&banner, sizeof(banner_encrypted));

	// Convert the image data.
	return read_banner_WIBN_internal(&banner, true);
}
