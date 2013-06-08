/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * Checksum.cpp: Checksum algorithm class.                                 *
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

#include "Checksum.hpp"
#include "SonicChaoGarden.h"

#include "byteswap.h"


/** Algorithms. **/


/**
 * CRC-16 algorithm.
 * @param buf Data buffer.
 * @param siz Length of data buffer.
 * @param poly Polynomial.
 * @return Checksum.
 */
uint16_t Checksum::Crc16(const uint8_t *buf, uint32_t siz, uint16_t poly)
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
 * AddInvDual16 algorithm.
 * Adds 16-bit words together in a uint16_t.
 * First word is a simple addition.
 * Second word adds (word ^ 0xFFFF).
 * If either word equals 0xFFFF, it's changed to 0.
 * @param buf Data buffer.
 * @param siz Length of data buffer.
 * @return Checksum.
 */
uint32_t Checksum::AddInvDual16(const uint16_t *buf, uint32_t siz)
{
	// NOTE: Integer overflow/underflow is expected here.
	uint16_t chk1 = 0;
	uint16_t chk2 = 0;

	// We're operating on words, not bytes.
	// siz is in bytes, so we have to divide it by two.
	siz /= 2;

	// Do four words at a time.
	// TODO: Optimize byteswapping?
	for (; siz > 4; siz -= 4, buf += 4) {
		chk1 += be16_to_cpu(buf[0]); chk2 += (be16_to_cpu(buf[0]) ^ 0xFFFF);
		chk1 += be16_to_cpu(buf[1]); chk2 += (be16_to_cpu(buf[1]) ^ 0xFFFF);
		chk1 += be16_to_cpu(buf[2]); chk2 += (be16_to_cpu(buf[2]) ^ 0xFFFF);
		chk1 += be16_to_cpu(buf[3]); chk2 += (be16_to_cpu(buf[3]) ^ 0xFFFF);
	}

	// Remaining words.
	for (; siz != 0; siz--, buf++) {
		chk1 += be16_to_cpu(*buf);
		chk2 += (be16_to_cpu(*buf) ^ 0xFFFF);
	}

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
 * AddBytes32 algorithm.
 * Adds all bytes together in a uint32_t.
 * @param buf Data buffer.
 * @param siz Length of data buffer.
 * @return Checksum.
 */
uint32_t Checksum::AddBytes32(const uint8_t *buf, uint32_t siz)
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
 * SonicChaoGarden algorithm.
 * @param buf Data buffer.
 * @param siz Length of data buffer.
 * @return Checksum.
 */
uint32_t Checksum::SonicChaoGarden(const uint8_t *buf, uint32_t siz)
{
	// Ported from MainMemory's C# SADX/SA2B Chao Garden checksum code.
	const uint32_t a4 = 0x686F6765;
	uint32_t v4 = 0x6368616F;

	for (; siz != 0; siz--, buf++) {
		v4 = SonicChaoGarden_CRC32_Table[*buf ^ (v4 & 0xFF)] ^ (v4 >> 8);
	}

	return (a4 ^ v4);
}


/** General functions. **/


/**
 * Get the checksum for a block of data.
 * @param algorithm Checksum algorithm.
 * @param buf Data buffer.
 * @param siz Length of data buffer.
 * @param param Algorithm parameter, e.g. polynomial or sum.
 * @return Checksum.
 */
uint32_t Checksum::Exec(ChkAlgorithm algorithm, const void *buf, uint32_t siz, uint32_t param)
{
	switch (algorithm) {
		case CHKALG_CRC16:
			if (param == 0)
				param = CRC16_POLY_CCITT;
			return Crc16(reinterpret_cast<const uint8_t*>(buf),
				     siz, (uint16_t)(param & 0xFFFF));

		case CHKALG_ADDINVDUAL16:
			return AddInvDual16(reinterpret_cast<const uint16_t*>(buf), siz);

		case CHKALG_ADDBYTES32:
			return AddBytes32(reinterpret_cast<const uint8_t*>(buf), siz);

		case CHKALG_SONICCHAOGARDEN:
			return SonicChaoGarden(reinterpret_cast<const uint8_t*>(buf), siz);

		case CHKALG_CRC32:
			// TODO

		default:
			break;
	}

	// Unknown algorithm.
	return 0;
}


/**
 * Get a ChkAlgorithm from a checksum algorithm name.
 * @param algorithm Checksum algorithm name.
 * @return ChkAlgorithm. (If unknown, returns CHKALG_NONE.)
 */
Checksum::ChkAlgorithm Checksum::ChkAlgorithmFromString(QString algorithm)
{
	if (algorithm == QLatin1String("crc16") ||
	    algorithm == QLatin1String("crc-16"))
	{
		return CHKALG_CRC16;
	}
	else if (algorithm == QLatin1String("crc32") ||
		 algorithm == QLatin1String("crc-32"))
	{
		return CHKALG_CRC32;
	} else if (algorithm == QLatin1String("addinvdual16")) {
		return CHKALG_ADDINVDUAL16;
	} else if (algorithm == QLatin1String("addbytes32")) {
		return CHKALG_ADDBYTES32;
	}
	else if (algorithm == QLatin1String("sonicchaogarden") ||
		 algorithm == QLatin1String("sonic chao garden"))
	{
		return CHKALG_SONICCHAOGARDEN;
	}

	// Unknown algorithm name.
	return CHKALG_NONE;
}


/**
 * Get a checksum algorithm name from a ChkAlgorithm.
 * @param algorithm ChkAlgorithm.
 * @return Checksum algorithm name, or empty string if CHKALG_NONE or unknown.
 */
QString Checksum::ChkAlgorithmToString(ChkAlgorithm algorithm)
{
	switch (algorithm) {
		default:
		case CHKALG_NONE:
			return QString();

		case CHKALG_CRC16:
			return QLatin1String("CRC-16");
		case CHKALG_CRC32:
			return QLatin1String("CRC-32");
		case CHKALG_ADDINVDUAL16:
			return QLatin1String("AddInvDual16");
		case CHKALG_ADDBYTES32:
			return QLatin1String("AddBytes32");
		case CHKALG_SONICCHAOGARDEN:
			return QLatin1String("SonicChaoGarden");
	}
}


/**
 * Get a nicely formatted checksum algorithm name from a ChkAlgorithm.
 * @param algorithm ChkAlgorithm.
 * @return Checksum algorithm name, or empty string if CHKALG_NONE or unknown.
 */
QString Checksum::ChkAlgorithmToStringFormatted(ChkAlgorithm algorithm)
{
	switch (algorithm) {
		default:
		case CHKALG_NONE:
			return QString();

		case CHKALG_CRC16:
			return QLatin1String("CRC-16");
		case CHKALG_CRC32:
			return QLatin1String("CRC-32");
		case CHKALG_ADDINVDUAL16:
			return QLatin1String("AddInvDual16");
		case CHKALG_ADDBYTES32:
			return QLatin1String("AddBytes32");
		case CHKALG_SONICCHAOGARDEN:
			return QLatin1String("Sonic Chao Garden");
	}
}
