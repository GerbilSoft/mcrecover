/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * Checksum.hpp: Checksum algorithm class.                                 *
 *                                                                         *
 * Copyright (c) 2013 by David Korth.                                      *
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

#ifndef __MCRECOVER_CHECKSUM_HPP__
#define __MCRECOVER_CHECKSUM_HPP__

// C includes.
#include <stdint.h>

// Qt includes.
#include <QtCore/QString>

class Checksum
{
	private:
		// Static class.
		Checksum();
		~Checksum();
		Checksum(const Checksum &);
		Checksum &operator=(const Checksum &);

	public:
		/**
		 * Checksum algorithms.
		 */
		enum ChkAlgorithm {
			CHKALG_NONE = 0,
			CHKALG_CRC16,
			CHKALG_CRC32,
			CHKALG_ADDBYTES32,
			CHKALG_SONICCHAOGARDEN,

			CHKALG_MAX
		};

		/**
		 * Checksum status.
		 */
		enum ChkStatus {
			CHKST_UNKNOWN = 0,	// Unknown checksum.
			CHKST_INVALID,		// Checksum is invalid.
			CHKST_GOOD		// Checksum is good.
		};

		// Checksum definition struct.
		struct ChecksumDef {
			ChkAlgorithm algorithm;
			uint32_t address;	// Checksum address.
			uint32_t poly;		// Polynomial for CRC algorithms.
			uint32_t start;		// Checksummed area: start.
			uint32_t length;	// Checksummed area: length.

			ChecksumDef()
				{ clear(); }

			void clear(void) {
				algorithm = CHKALG_NONE;
				address = 0;
				poly = 0;
				start = 0;
				length = 0;
			}
		};

		// Chao Garden checksum struct.
		struct ChaoGardenChecksumData {
			/**
			 * The following bytes MUST be 0 when evaluating the checksum:
			 * - checksum_0, checksum_1, checksum_2, checksum_3
			 * - random_3
			 */
			uint8_t checksum_1;     // Checksum byte 1. (bits 15-8)
			uint8_t random_0;       // Random byte 0.
			uint8_t checksum_3;     // Checksum byte 3. (bits 31-24)
			uint8_t random_3;       // Random byte 3. [MUST BE ZERO INITIALLY]
			uint8_t random_1;       // Random byte 1.
			uint8_t checksum_0;     // Checksum byte 0. (bits 7-0)
			uint8_t random_2;       // Random byte 2.
			uint8_t checksum_2;     // Checksum byte 2. (bits 23-16)
		};

		/** Default polynomials. **/

		static const uint16_t CRC16_POLY_CCITT = 0x8408;
		static const uint32_t CRC32_POLY_ZLIB = 0xEDB88320;

		/** Algorithms. **/

		/**
		 * CRC-16 algorithm.
		 * @param buf Data buffer.
		 * @param siz Length of data buffer.
		 * @param poly Polynomial.
		 * @return Checksum.
		 */
		static uint16_t Crc16(const uint8_t *buf, uint32_t siz, uint16_t poly = CRC16_POLY_CCITT);

		/**
		 * AddBytes32 algorithm.
		 * Adds all bytes together in a uint32_t.
		 * @param buf Data buffer.
		 * @param siz Length of data buffer.
		 * @return Checksum.
		 */
		static uint32_t AddBytes32(const uint8_t *buf, uint32_t siz);

		/**
		 * SonicChaoGarden algorithm.
		 * @param buf Data buffer.
		 * @param siz Length of data buffer.
		 * @return Checksum.
		 */
		static uint32_t SonicChaoGarden(const uint8_t *buf, uint32_t siz);

		/** General functions. **/

		/**
		 * Get the checksum for a block of data.
		 * @param algorithm Checksum algorithm.
		 * @param buf Data buffer.
		 * @param siz Length of data buffer.
		 * @param poly Polynomial. (Not needed for some algorithms.)
		 * @return Checksum.
		 */
		static uint32_t Exec(ChkAlgorithm algorithm, const void *buf, uint32_t siz, uint32_t poly = 0);

		/**
		 * Get a ChkAlgorithm from a checksum algorithm name.
		 * @param algorithm Checksum algorithm name.
		 * @return ChkAlgorithm. (If unknown, returns CHKALG_NONE.)
		 */
		static ChkAlgorithm ChkAlgorithmFromString(QString algorithm);

		/**
		 * Get a checksum algorithm name from a ChkAlgorithm.
		 * @param algorithm ChkAlgorithm.
		 * @return Checksum algorithm name, or empty string if CHKALG_NONE or unknown.
		 */
		static QString ChkAlgorithmToString(ChkAlgorithm algorithm);
};

#endif /* __MCRECOVER_CHECKSUM_HPP__ */
