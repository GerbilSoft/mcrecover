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

class ChecksumPrivate
{
	private:
		ChecksumPrivate();
		~ChecksumPrivate();
		ChecksumPrivate(const ChecksumPrivate &);
		ChecksumPrivate &operator=(const ChecksumPrivate &);

	public:
		static uint16_t Crc16(const uint8_t *buf, uint32_t siz, uint16_t poly = Checksum::CRC16_POLY_CCITT);
};


/**
 * CRC-16 algorithm.
 * @param buf Data buffer.
 * @param siz Length of data buffer.
 * @param poly Polynomial.
 * @return Checksum.
 */
uint16_t ChecksumPrivate::Crc16(const uint8_t *buf, uint32_t siz, uint16_t poly)
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
 * Get the checksum for a block of data.
 * @param algorithm Checksum algorithm.
 * @param buf Data buffer.
 * @param siz Length of data buffer.
 * @param poly Polynomial. (Not needed for some algorithms.)
 * @return Checksum.
 */
uint32_t Checksum::Exec(ChkAlgorithm algorithm, const void *buf, uint32_t siz, uint32_t poly)
{
	switch (algorithm) {
		case CHKALG_CRC16:
			if (poly == 0)
				poly = CRC16_POLY_CCITT;
			return ChecksumPrivate::Crc16(
				reinterpret_cast<const uint8_t*>(buf), siz, (uint16_t)(poly & 0xFFFF));

		case CHKALG_CRC32:
		case CHKALG_ALLBYTES32:
		case CHKALG_SONICCHAOGARDEN:
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
	} else if (algorithm == QLatin1String("allbytes32")) {
		return CHKALG_ALLBYTES32;
	} else if (algorithm == QLatin1String("sonicchaogarden")) {
		return CHKALG_SONICCHAOGARDEN;
	}

	// Unknown algorithm name.
	return CHKALG_NONE;
}
