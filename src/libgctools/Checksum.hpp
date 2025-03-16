/***************************************************************************
 * GameCube Tools Library.                                                 *
 * Checksum.hpp: Checksum algorithm class.                                 *
 *                                                                         *
 * Copyright (c) 2013-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBGCTOOLS_CHECKSUM_HPP__
#define __LIBGCTOOLS_CHECKSUM_HPP__

// C includes.
#include <stdint.h>

// C++ includes.
#include <string>
#include <vector>

namespace Checksum {

/**
 * Checksum algorithms
 */
enum class ChkAlgorithm {
	None = 0,
	CRC16,
	CRC32,
	AddInvDual16,
	AddBytes32,
	SonicChaoGarden,
	DreamcastVMU,
	PokemonXD,

	Max
};

/**
 * Checksum status
 */
enum class ChkStatus {
	Unknown = 0,	// Unknown checksum.
	Invalid,	// Checksum is invalid.
	Good,		// Checksum is good.
};

/**
 * Checksum data endianness
 */
enum class ChkEndian {
	Big = 0,	// Big-endian (PowerPC, etc.)
	Little = 1,	// Little-endian (x86, SH-4, etc.)
};

// Checksum definition struct
struct ChecksumDef {
	ChkAlgorithm algorithm;
	uint32_t address;	// Checksum address.
	uint32_t param;		// Algorithm parameter, e.g. "poly" or "sum".
	uint32_t start;		// Checksummed area: start.
	uint32_t length;	// Checksummed area: length.
	ChkEndian endian;	// Endianness.

	ChecksumDef() { clear(); }

	void clear(void)
	{
		algorithm = ChkAlgorithm::None;
		address = 0;
		param = 0;
		start = 0;
		length = 0;
		endian = ChkEndian::Big;
	}
};

// Checksum value struct
struct ChecksumValue {
	uint32_t expected;
	uint32_t actual;
};

// Chao Garden checksum struct
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

/** Default polynomials **/

static constexpr uint16_t CRC16_POLY_CCITT = 0x8408;
static constexpr uint32_t CRC32_POLY_ZLIB = 0xEDB88320;

/** Default sums **/

static constexpr uint16_t ADDSUBDUAL16_SUM_GCN_MEMCARD = 0xFFFF;

/** Algorithms **/

/**
 * CRC-16 algorithm
 * @param buf Data buffer
 * @param siz Length of data buffer
 * @param poly Polynomial
 * @return Checksum
 */
uint16_t Crc16(const uint8_t *buf, uint32_t siz, uint16_t poly = CRC16_POLY_CCITT);

/**
 * AddInvDual16 algorithm
 *
 * Adds 16-bit words together in a uint16_t.
 * First word is a simple addition.
 * Second word adds (word ^ 0xFFFF).
 * If either word equals 0xFFFF, it's changed to 0.
 *
 * @param buf Data buffer
 * @param siz Length of data buffer.
 * @param endian Endianness of the data.
 * @return Checksum.
 */
uint32_t AddInvDual16(const uint16_t *buf, uint32_t siz, ChkEndian endian);

/**
 * AddBytes32 algorithm
 * Adds all bytes together in a uint32_t
 * @param buf Data buffer
 * @param siz Length of data buffer
 * @return Checksum
 */
uint32_t AddBytes32(const uint8_t *buf, uint32_t siz);

/**
 * SonicChaoGarden algorithm
 * @param buf Data buffer
 * @param siz Length of data buffer
 * @return Checksum
 */
uint32_t SonicChaoGarden(const uint8_t *buf, uint32_t siz);

/**
 * Dreamcast VMU algorithm
 * Based on FCS-16.
 *
 * NOTE: The CRC is stored within the header.
 * Specify the address in crc_addr in order to
 * handle this properly. (Set to -1 to skip.)
 * The usual address is 0x46.
 *
 * @param buf Data buffer
 * @param siz Length of data buffer
 * @param crc_addr Address of CRC in header
 * @return Checksum
 */
uint16_t DreamcastVMU(const uint8_t *buf, uint32_t siz, uint32_t crc_addr = -1);

/**
 * Pokémon XD algorithm
 * Reference: https://github.com/TuxSH/PkmGCTools/blob/master/LibPkmGC/src/LibPkmGC/XD/SaveEditing/SaveSlot.cpp
 *
 * The data area is "encrypted", so it has to be decrypted before
 * a checksum can be calculated.
 *
 * @param buf		[in] Data buffer
 * @param siz		[in] Length of data buffer
 * @param crc_addr	[in] CRC address (Should be 0x10, 0x14, 0x18, 0x1C.)
 * @param pChkExpect	[out] Expected checksum, decrypted
 * @return Actual checksum, decrypted
 */
uint32_t PokemonXD(const uint8_t *buf, uint32_t siz, uint32_t crc_addr, uint32_t *pChkExpect);

/** General functions. **/

/**
 * Get the checksum for a block of data.
 * @param algorithm Checksum algorithm
 * @param buf Data buffer
 * @param siz Length of data buffer
 * @param endian Endianness of the data
 * @param param Algorithm parameter, e.g. polynomial or sum.
 * @return Checksum
 */
uint32_t Exec(ChkAlgorithm algorithm, const void *buf, uint32_t siz, ChkEndian endian, uint32_t param = 0);

/**
 * Get a ChkAlgorithm from a checksum algorithm name.
 * @param algorithm Checksum algorithm name
 * @return ChkAlgorithm (If unknown, returns ChkAlgorithm::None.)
 */
ChkAlgorithm ChkAlgorithmFromString(const char *algorithm);

/**
 * Get a checksum algorithm name from a ChkAlgorithm.
 * @param algorithm ChkAlgorithm
 * @return Checksum algorithm name, or nullptr if ChkAlgorithm::None or unknown.
 */
const char *ChkAlgorithmToString(ChkAlgorithm algorithm);

/**
 * Get a nicely formatted checksum algorithm name from a ChkAlgorithm.
 * @param algorithm ChkAlgorithm
 * @return Checksum algorithm name, or nullptr if ChkAlgorithm::None or unknown.
 */
const char *ChkAlgorithmToStringFormatted(ChkAlgorithm algorithm);

/**
 * Get the checksum field width.
 * @param checksumValues Checksum values to check
 * @return 4 for 16-bit checksums; 8 for 32-bit checksums.
 */
int ChecksumFieldWidth(const std::vector<ChecksumValue>& checksumValues);

/**
 * Get the checksum status.
 * @param checksumValues Checksum values to check
 * @return Checksum status
 */
ChkStatus ChecksumStatus(const std::vector<ChecksumValue>& checksumValues);

/**
 * Format checksum values as HTML for display purposes.
 * @param checksumValues Checksum values to format
 * @return QVector containing one or two HTML strings.
 * - String 0 contains the actual checksums.
 * - String 1, if present, contains the expected checksums.
 */
std::vector<std::string> ChecksumValuesFormatted(const std::vector<ChecksumValue>& checksumValues);

}

#endif /* __LIBGCTOOLS_CHECKSUM_HPP__ */
