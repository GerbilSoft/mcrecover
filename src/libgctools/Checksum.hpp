/***************************************************************************
 * GameCube Tools Library.                                                 *
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

#ifndef __LIBGCTOOLS_CHECKSUM_HPP__
#define __LIBGCTOOLS_CHECKSUM_HPP__

// C includes.
#include <stdint.h>

// C++ includes.
#include <string>
#include <vector>

/**
 * GCN checksum algorithm class.
 * Contains algorithms for GCN save files.
 *
 * NOTE: All string parameters are ASCII.
 */
class Checksum
{
	private:
		// Static class.
		Checksum();
		~Checksum();
		// TODO: Copy Qt's Q_DISABLE_COPY() macro.
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
			CHKALG_ADDINVDUAL16,
			CHKALG_ADDBYTES32,
			CHKALG_SONICCHAOGARDEN,
			CHKALG_DREAMCASTVMU,

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

		/**
		 * Checksum data endianness.
		 */
		enum ChkEndian {
			CHKENDIAN_BIG = 0,	// Big-endian (PowerPC, etc.)
			CHKENDIAN_LITTLE = 1,	// Little-endian (x86, SH-4, etc.)
		};

		// Checksum definition struct.
		struct ChecksumDef {
			ChkAlgorithm algorithm;
			uint32_t address;	// Checksum address.
			uint32_t param;		// Algorithm parameter, e.g. "poly" or "sum".
			uint32_t start;		// Checksummed area: start.
			uint32_t length;	// Checksummed area: length.
			ChkEndian endian;	// Endianness.

			ChecksumDef()
				{ clear(); }

			void clear(void) {
				algorithm = CHKALG_NONE;
				address = 0;
				param = 0;
				start = 0;
				length = 0;
				endian = CHKENDIAN_BIG;
			}
		};

		// Checksum value struct.
		struct ChecksumValue {
			uint32_t expected;
			uint32_t actual;
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

		/** Default sums. **/

		static const uint16_t ADDSUBDUAL16_SUM_GCN_MEMCARD = 0xFFFF;

		/** Algorithms. **/

		/**
		 * CRC-16 algorithm.
		 * @param buf Data buffer.
		 * @param siz Length of data buffer.
		 * @param poly Polynomial.
		 * @return Checksum.
		 */
		static uint16_t Crc16(const uint8_t *buf, uint32_t siz,
				      uint16_t poly = CRC16_POLY_CCITT);

		/**
		 * AddInvDual16 algorithm.
		 * Adds 16-bit words together in a uint16_t.
		 * First word is a simple addition.
		 * Second word adds (word ^ 0xFFFF).
		 * If either word equals 0xFFFF, it's changed to 0.
		 * @param buf Data buffer.
		 * @param siz Length of data buffer.
		 * @param endian Endianness of the data.
		 * @return Checksum.
		 */
		static uint32_t AddInvDual16(const uint16_t *buf, uint32_t siz, ChkEndian endian);

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

		/**
		 * Dreamcast VMU algorithm.
		 * Based on FCS-16.
		 *
		 * NOTE: The CRC is stored within the header.
		 * Specify the address in crc_addr in order to
		 * handle this properly. (Set to -1 to skip.)
		 * The usual address is 0x46.
		 *
		 * @param buf Data buffer.
		 * @param siz Length of data buffer.
		 * @param crc_addr Address of CRC in header.
		 * @return Checksum.
		 */
		static uint16_t DreamcastVMU(const uint8_t *buf, uint32_t siz, uint32_t crc_addr = -1);

		/** General functions. **/

		/**
		 * Get the checksum for a block of data.
		 * @param algorithm Checksum algorithm.
		 * @param buf Data buffer.
		 * @param siz Length of data buffer.
		 * @param endian Endianness of the data.
		 * @param param Algorithm parameter, e.g. polynomial or sum.
		 * @return Checksum.
		 */
		static uint32_t Exec(ChkAlgorithm algorithm, const void *buf, uint32_t siz, ChkEndian endian, uint32_t param = 0);

		/**
		 * Get a ChkAlgorithm from a checksum algorithm name.
		 * @param algorithm Checksum algorithm name.
		 * @return ChkAlgorithm. (If unknown, returns CHKALG_NONE.)
		 */
		static ChkAlgorithm ChkAlgorithmFromString(const char *algorithm);

		/**
		 * Get a checksum algorithm name from a ChkAlgorithm.
		 * @param algorithm ChkAlgorithm.
		 * @return Checksum algorithm name, or nullptr if CHKALG_NONE or unknown.
		 */
		static const char *ChkAlgorithmToString(ChkAlgorithm algorithm);

		/**
		 * Get a nicely formatted checksum algorithm name from a ChkAlgorithm.
		 * @param algorithm ChkAlgorithm.
		 * @return Checksum algorithm name, or nullptr if CHKALG_NONE or unknown.
		 */
		static const char *ChkAlgorithmToStringFormatted(ChkAlgorithm algorithm);

		/**
		 * Get the checksum field width.
		 * @param checksumValues Checksum values to check.
		 * @return 4 for 16-bit checksums; 8 for 32-bit checksums.
		 */
		static int ChecksumFieldWidth(const std::vector<ChecksumValue>& checksumValues);

		/**
		 * Get the checksum status.
		 * @param checksumValues Checksum values to check.
		 * @return Checksum status.
		 */
		static ChkStatus ChecksumStatus(const std::vector<ChecksumValue>& checksumValues);

		/**
		 * Format checksum values as HTML for display purposes.
		 * @param checksumValues Checksum values to format.
		 * @return QVector containing one or two HTML strings.
		 * - String 0 contains the actual checksums.
		 * - String 1, if present, contains the expected checksums.
		 */
		static std::vector<std::string> ChecksumValuesFormatted(const std::vector<ChecksumValue>& checksumValues);
};

#endif /* __LIBGCTOOLS_CHECKSUM_HPP__ */
