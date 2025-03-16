/***************************************************************************
 * GameCube Tools Library.                                                 *
 * Checksum.cpp: Checksum algorithm class.                                 *
 *                                                                         *
 * Copyright (c) 2013-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "Checksum.hpp"
#include "SonicChaoGarden.inc.h"

#include "util/byteswap.h"

// C includes (C++ namespace)
#include <cassert>
#include <cstdio>
#include <cstring>

// C++ includes
#include <memory>
#include <string>
#include <vector>
using std::string;
using std::unique_ptr;
using std::vector;

namespace Checksum {

/** Algorithms **/

/**
 * CRC-16 algorithm
 * @param buf Data buffer
 * @param siz Length of data buffer
 * @param poly Polynomial
 * @return Checksum
 */
uint16_t Crc16(const uint8_t *buf, uint32_t siz, uint16_t poly)
{
	// TODO: Add optimized version for poly == CRC16_POLY_CCITT.
	uint16_t crc = 0xFFFF;

	for (; siz != 0; siz--, buf++) {
		crc ^= (*buf & 0xFF);
		for (int i = 8; i > 0; i--) {
			if (crc & 1)
				crc = ((crc >> 1) ^ poly);
			else
				crc >>= 1;
		}
	}

	return ~crc;
}

/**
 * AddInvDual16 algorithm
 *
 * Adds 16-bit words together in a uint16_t.
 * First word is a simple addition.
 * Second word adds (word ^ 0xFFFF).
 * If either word equals 0xFFFF, it's changed to 0.
 *
 * @param buf Data buffer
 * @param siz Length of data buffer
 * @param endian Endianness of the data
 * @return Checksum
 */
uint32_t AddInvDual16(const uint16_t *buf, uint32_t siz, ChkEndian endian)
{
	// We're operating on words, not bytes.
	// siz is in bytes, so we have to divide it by two.
	siz /= 2;

	// NOTE: Integer overflow/underflow is expected here.
	uint16_t chk1 = 0;
	uint16_t chk2 = (uint16_t)(-(int)siz);

	if (endian != ChkEndian::Little) {
		// Big-endian system. (PowerPC, etc.)
		// Do four words at a time.
		// TODO: Optimize byteswapping?
		for (; siz > 4; siz -= 4, buf += 4) {
			chk1 += be16_to_cpu(buf[0]);
			chk1 += be16_to_cpu(buf[1]);
			chk1 += be16_to_cpu(buf[2]);
			chk1 += be16_to_cpu(buf[3]);
		}

		// Remaining words.
		for (; siz != 0; siz--, buf++) {
			chk1 += be16_to_cpu(*buf);
		}
	} else {
		// Little-endian system. (x86, SH-4, etc.)
		// Do four words at a time.
		// TODO: Optimize byteswapping?
		for (; siz > 4; siz -= 4, buf += 4) {
			chk1 += le16_to_cpu(buf[0]);
			chk1 += le16_to_cpu(buf[1]);
			chk1 += le16_to_cpu(buf[2]);
			chk1 += le16_to_cpu(buf[3]);
		}

		// Remaining words.
		for (; siz != 0; siz--, buf++) {
			chk1 += le16_to_cpu(*buf);
		}
	}
	
	// sum(word ^ 0xFFFF) = sum(0xFFFF - word) = 0xFFFF * siz - sum(word)
	// On 16 bits using two's complement, 0xFFFF = -1, so chk2 can be simplified as -siz - chk1.
	chk2 -= chk1;

	// 0xFFFF is an invalid checksum value.
	// Reset it to 0 if it shows up.
	if (chk1 == 0xFFFF)
		chk1 = 0;
	if (chk2 == 0xFFFF)
		chk2 = 0;

	// Combine the checksum into a dword.
	// chk1 == high word; chk2 == low word.
	return ((chk1 << 16) | chk2);
}

/**
 * AddBytes32 algorithm
 * Adds all bytes together in a uint32_t.
 * @param buf Data buffer
 * @param siz Length of data buffer
 * @return Checksum
 */
uint32_t AddBytes32(const uint8_t *buf, uint32_t siz)
{
	uint32_t checksum = 0;

	// Do four bytes at a time.
	for (; siz > 4; siz -= 4, buf += 4) {
		checksum += buf[0];
		checksum += buf[1];
		checksum += buf[2];
		checksum += buf[3];
	}

	// Remaining bytes.
	for (; siz != 0; siz--, buf++)
		checksum += *buf;

	return checksum;
}

/**
 * SonicChaoGarden algorithm
 * @param buf Data buffer
 * @param siz Length of data buffer
 * @return Checksum
 */
uint32_t SonicChaoGarden(const uint8_t *buf, uint32_t siz)
{
	// Ported from MainMemory's C# SADX/SA2B Chao Garden checksum code.
	const uint32_t a4 = 0x686F6765;
	uint32_t v4 = 0x6368616F;

	for (; siz != 0; siz--, buf++) {
		v4 = SonicChaoGarden_CRC32_Table[*buf ^ (v4 & 0xFF)] ^ (v4 >> 8);
	}

	return (a4 ^ v4);
}

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
uint16_t DreamcastVMU(const uint8_t *buf, uint32_t siz, uint32_t crc_addr)
{
	// TODO: Optimize this?
	// Reference: http://mc.pp.se/dc/vms/fileheader.html
	unsigned int n = 0;
	for (uint32_t i = 0; i < siz; i++) {
		uint8_t chr = buf[i];
		if (i == crc_addr || i == (crc_addr + 1)) {
			// CRC address. Pretend it's 0.
			chr = 0;
		}

		n ^= (chr << 8);
		for (int c = 0; c < 8; c++) {
			if (n & 0x8000)
				n = (n << 1) ^ 4129;
			else
				n = (n << 1);
		}
	}

	return (n & 0xFFFF);
}

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
uint32_t PokemonXD(const uint8_t *buf, uint32_t siz, uint32_t crc_addr, uint32_t *pChkExpect)
{
	// TODO: There's four 32-bit checksums. We should rework
	// the checksum system so we don't have to decrypt the
	// save data four times, once for each checksum.
	static const uint32_t checksum_size = (0x9FF4*4)+8;
	if (siz < (0x9FF4*4)+8) {
		// Incorrect buffer size.
		if (pChkExpect) {
			*pChkExpect = 0;
		}
		return ~0U;
	}

	// All fields are in big-endian.
	struct PokemonXDHeader {
		uint32_t magic;		// [0x000] 0x01010100
		uint32_t save_count;	// [0x004] Number of times the game has been saved.
		uint16_t enc_keys[4];	// [0x008] Encryption keys

		// The following data is all encrypted.
		uint32_t checksum[4];	// [0x010] Checksums
	};

	// Decryption buffer.
	unique_ptr<uint8_t[]> decbuf(new uint8_t[checksum_size]);
	memcpy(decbuf.get(), buf, 16);

	// Decrypt the data.
	PokemonXDHeader *const pHdr = reinterpret_cast<PokemonXDHeader*>(decbuf.get());
	uint16_t keys[4];
	keys[0] = be16_to_cpu(pHdr->enc_keys[0]);
	keys[1] = be16_to_cpu(pHdr->enc_keys[1]);
	keys[2] = be16_to_cpu(pHdr->enc_keys[2]);
	keys[3] = be16_to_cpu(pHdr->enc_keys[3]);

	const uint16_t *psrcbuf16 = reinterpret_cast<const uint16_t*>(buf) + 8;
	uint16_t *pdestbuf16 = reinterpret_cast<uint16_t*>(decbuf.get()) + 8;
	for (size_t i = 16; i < checksum_size; i += 8) {
		for (unsigned int j = 0; j < 4; j++, psrcbuf16++, pdestbuf16++) {
			uint16_t tmp = be16_to_cpu(*psrcbuf16);
			tmp -= keys[j];
			*pdestbuf16 = cpu_to_be16(tmp);
		}

		// Advance the keys.
		const uint16_t a = keys[0] + 0x43;
		const uint16_t b = keys[1] + 0x29;
		const uint16_t c = keys[2] + 0x17;
		const uint16_t d = keys[3] + 0x13;

		keys[0] = (a & 0xf) | ((b << 4) & 0xf0) | ((c << 8) & 0xf00) | ((d << 12) & 0xf000);
		keys[1] = ((a >> 4) & 0xf) | (b & 0xf0) | ((c << 4) & 0xf00) | ((d << 8) & 0xf000);
		keys[2] = (c & 0xf00) | ((b & 0xf00) >> 4) | ((a & 0xf00) >> 8) | ((d << 4) & 0xf000);
		keys[3] = ((a >> 12) & 0xf) | ((b >> 8) & 0xf0) | ((c >> 4) & 0xf00) | (d & 0xf000);
	}

	// Get the expected checksum.
	// NOTE: Checksum is stored weirdly:
	// - ID is reversed.
	// - Checksum is stored wordswapped.
	// We'll use crc_addr as the checksum ID in the header,
	// then do a reverse when checking the actual data area.
	const unsigned int chkID = (crc_addr >> 2) & 3;
	uint32_t chk_expect = be32_to_cpu(pHdr->checksum[chkID]);
	chk_expect = (chk_expect << 16) | (chk_expect >> 16);
	if (pChkExpect) {
		*pChkExpect = chk_expect;
	}

	// Calculate the actual checksum.
	// NOTE: Checksum values should be zeroed out here.
	pHdr->checksum[0] = 0;
	pHdr->checksum[1] = 0;
	pHdr->checksum[2] = 0;
	pHdr->checksum[3] = 0;

	// Calculate only the specified checksum.
	uint32_t chk_actual = 0;
	psrcbuf16 = reinterpret_cast<const uint16_t*>(&decbuf[0x08]);
	psrcbuf16 += ((chkID ^ 3) * (0x9FF4/2));
	for (size_t i = 0; i < 0x9FF4; i += 2, psrcbuf16++) {
		chk_actual += (uint32_t)be16_to_cpu(*psrcbuf16);
	}
	return chk_actual;
}

/** General functions **/

/**
 * Get the checksum for a block of data.
 * @param algorithm Checksum algorithm
 * @param buf Data buffer
 * @param siz Length of data buffer
 * @param endian Endianness of the data
 * @param param Algorithm parameter, e.g. polynomial or sum.
 * @return Checksum
 */
uint32_t Exec(ChkAlgorithm algorithm, const void *buf, uint32_t siz, ChkEndian endian, uint32_t param)
{
	switch (algorithm) {
		default:
		case ChkAlgorithm::None:
			break;

		case ChkAlgorithm::CRC16:
			if (param == 0)
				param = CRC16_POLY_CCITT;
			return Crc16(static_cast<const uint8_t*>(buf),
				     siz, (uint16_t)(param & 0xFFFF));

		case ChkAlgorithm::CRC32:
			// TODO: Implement CRC32 once I encounter a file that uses it.
			break;

		case ChkAlgorithm::AddInvDual16:
			return AddInvDual16(static_cast<const uint16_t*>(buf), siz, endian);

		case ChkAlgorithm::AddBytes32:
			return AddBytes32(static_cast<const uint8_t*>(buf), siz);

		case ChkAlgorithm::SonicChaoGarden:
			return SonicChaoGarden(static_cast<const uint8_t*>(buf), siz);

		case ChkAlgorithm::DreamcastVMU:
			// If param is 0, assume a default CRC address of 0x46.
			// (NOTE: Headers in game files are at 0x200,
			//  but the CRC field is unused for game files.)
			if (param == 0)
				param = 0x46;
			return DreamcastVMU(static_cast<const uint8_t*>(buf), siz, param);

		case ChkAlgorithm::PokemonXD:
			// NOTE: Expected checksum is discarded.
			return PokemonXD(static_cast<const uint8_t*>(buf), siz, 0, nullptr);
	}

	// Unknown algorithm.
	return 0;
}

/**
 * Get a ChkAlgorithm from a checksum algorithm name.
 * @param algorithm Checksum algorithm name
 * @return ChkAlgorithm (If unknown, returns ChkAlgorithm::None.)
 */
ChkAlgorithm ChkAlgorithmFromString(const char *algorithm)
{
	// FIXME: Case-insensitive comparison?
	if (!strcmp(algorithm, "crc16") ||
	    !strcmp(algorithm, "crc-16"))
	{
		return ChkAlgorithm::CRC16;
	}
	else if (!strcmp(algorithm, "crc32") ||
		 !strcmp(algorithm, "crc-32"))
	{
		return ChkAlgorithm::CRC32;
	} else if (!strcmp(algorithm, "addinvdual16")) {
		return ChkAlgorithm::AddInvDual16;
	} else if (!strcmp(algorithm, "addbytes32")) {
		return ChkAlgorithm::AddBytes32;
	}
	else if (!strcmp(algorithm, "sonicchaogarden") ||
		 !strcmp(algorithm, "sonic chao garden"))
	{
		return ChkAlgorithm::SonicChaoGarden;
	}
	else if (!strcmp(algorithm, "dreamcastvmu") ||
		 !strcmp(algorithm, "dreamcast vmu") ||
		 !strcmp(algorithm, "dcvmu") ||
		 !strcmp(algorithm, "dc vmu"))
	{
		return ChkAlgorithm::DreamcastVMU;
	}
	else if (!strcmp(algorithm, "pokemonxd") ||
		 !strcmp(algorithm, "pokémonxd"))
	{
		return ChkAlgorithm::PokemonXD;
	}

	// Unknown algorithm name.
	return ChkAlgorithm::None;
}

/**
 * Get a checksum algorithm name from a ChkAlgorithm.
 * @param algorithm ChkAlgorithm
 * @return Checksum algorithm name, or nullptr if ChkAlgorithm::None or unknown.
 */
const char *ChkAlgorithmToString(ChkAlgorithm algorithm)
{
	switch (algorithm) {
		default:
		case ChkAlgorithm::None:
			return nullptr;

		case ChkAlgorithm::CRC16:
			return "CRC-16";
		case ChkAlgorithm::CRC32:
			return "CRC-32";
		case ChkAlgorithm::AddInvDual16:
			return "AddInvDual16";
		case ChkAlgorithm::AddBytes32:
			return "AddBytes32";
		case ChkAlgorithm::SonicChaoGarden:
			return "SonicChaoGarden";
		case ChkAlgorithm::DreamcastVMU:
			return "DreamcastVMU";
	}
}

/**
 * Get a nicely formatted checksum algorithm name from a ChkAlgorithm.
 * @param algorithm ChkAlgorithm
 * @return Checksum algorithm name, or nullptr if ChkAlgorithm::None or unknown.
 */
const char *ChkAlgorithmToStringFormatted(ChkAlgorithm algorithm)
{
	switch (algorithm) {
		default:
		case ChkAlgorithm::None:
			return nullptr;

		case ChkAlgorithm::CRC16:
			return "CRC-16";
		case ChkAlgorithm::CRC32:
			return "CRC-32";
		case ChkAlgorithm::AddInvDual16:
			return "AddInvDual16";
		case ChkAlgorithm::AddBytes32:
			return "AddBytes32";
		case ChkAlgorithm::SonicChaoGarden:
			return "Sonic Chao Garden";
		case ChkAlgorithm::DreamcastVMU:
			return "Dreamcast VMU";
		case ChkAlgorithm::PokemonXD:
			return "Pokémon XD";
	}
}

/**
 * Get the checksum field width.
 * @param checksumValues Checksum values to check
 * @return 4 for 16-bit checksums; 8 for 32-bit checksums.
 */
int ChecksumFieldWidth(const vector<ChecksumValue>& checksumValues)
{
	if (checksumValues.empty()) {
		return 4;
	}

	for (auto iter = checksumValues.cbegin();
	     iter != checksumValues.cend(); ++iter)
	{
		if (iter->expected > 0xFFFF || iter->actual > 0xFFFF) {
			// Checksums are 32-bit.
			return 8;
		}
	}

	// Checksums are 16-bit.
	return 4;
}

/**
 * Get the checksum status.
 * @param checksumValues Checksum values to check
 * @return Checksum status
 */
ChkStatus ChecksumStatus(const vector<ChecksumValue>& checksumValues)
{
	if (checksumValues.empty()) {
		return ChkStatus::Unknown;
	}

	for (auto iter = checksumValues.cbegin();
	     iter != checksumValues.cend(); ++iter)
	{
		if (iter->expected != iter->actual) {
			return ChkStatus::Invalid;
		}
	}

	// All checksums are good.
	return ChkStatus::Good;
}

/**
 * Format checksum values as HTML for display purposes.
 * @param checksumValues Checksum values to format
 * @return QVector containing one or two HTML strings.
 * - String 0 contains the actual checksums.
 * - String 1, if present, contains the expected checksums.
 */
vector<string> ChecksumValuesFormatted(const vector<ChecksumValue>& checksumValues)
{
	// Checksum colors.
	// TODO: Better colors?
	static const char s_chkHtmlLinebreak[] = "<br/>";

	// Get the checksum values.
	const int fieldWidth = ChecksumFieldWidth(checksumValues);
	// Assume 34 characters per checksum entry.
	const int reserveSize = ((34 + fieldWidth + 5) * (int)checksumValues.size());

	// Get the checksum status.
	const ChkStatus checksumStatus = ChecksumStatus(checksumValues);

	string s_chkActual_all; s_chkActual_all.reserve(reserveSize);
	string s_chkExpected_all;
	if (checksumStatus == ChkStatus::Invalid) {
		s_chkExpected_all.reserve(reserveSize);
	}

	int i = 0;
	for (auto iter = checksumValues.cbegin();
	     iter != checksumValues.cend(); ++iter, ++i)
	{
		if (i > 0) {
			// Add linebreaks or spaces to the checksum strings.
			if ((i % 2) && fieldWidth <= 4) {
				// Odd checksum index, 16-bit checksum.
				// Add a space.
				s_chkActual_all += ' ';
				s_chkExpected_all += ' ';
			} else {
				// Add a linebreak.
				s_chkActual_all += s_chkHtmlLinebreak;
				s_chkExpected_all += s_chkHtmlLinebreak;
			}
		}

		char s_chkActual[12];
		char s_chkExpected[12];
		snprintf(s_chkActual, sizeof(s_chkActual), "%0*X", fieldWidth, iter->actual);
		snprintf(s_chkExpected, sizeof(s_chkExpected), "%0*X", fieldWidth, iter->expected);

		// Check if the checksum is valid.
		if (iter->actual == iter->expected) {
			// Checksum is valid.
			s_chkActual_all += "<span style='color: #080'>";
			s_chkActual_all += + s_chkActual;
			s_chkActual_all += "</span>";
			if (checksumStatus == ChkStatus::Invalid) {
				s_chkExpected_all += "<span style='color: #080'>";
				s_chkExpected_all += s_chkExpected;
				s_chkExpected_all += "</span>";
			}
		} else {
			// Checksum is invalid.
			s_chkActual_all += "<span style='color: #F00'>";
			s_chkActual_all += s_chkActual;
			s_chkActual_all += "</span>";
			if (checksumStatus == ChkStatus::Invalid) {
				s_chkExpected_all += "<span style='color: #F00'>";
				s_chkExpected_all += s_chkExpected;
				s_chkExpected_all += "</span>";
			}
		}
	}

	// Return the checksum strings.
	vector<string> ret;
	ret.push_back(s_chkActual_all);
	if (checksumStatus == ChkStatus::Invalid) {
		ret.push_back(s_chkExpected_all);
	}
	return ret;
}

}
