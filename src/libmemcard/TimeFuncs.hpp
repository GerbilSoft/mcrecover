/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * TimeFuncs.hpp: Time conversion functions.                               *
 *                                                                         *
 * Copyright (c) 2012-2018 by David Korth.                                 *
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
 * You should have received a copy of the GNU General Public License       *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 ***************************************************************************/

#ifndef __LIBMEMCARD_TIMEFUNCS_HPP__
#define __LIBMEMCARD_TIMEFUNCS_HPP__

// C includes.
#include <stdint.h>

// Qt includes.
#include <QtCore/QDateTime>

// from vmu.h
extern "C" struct _vmu_timestamp;

namespace TimeFuncs {

/**
 * Convert a GCN timestamp to QDateTime.
 *
 * NOTE: GCN doesn't support timezones, so these times
 * will all be set to UTC.
 *
 * @param gcnTimestamp GCN timestamp.
 * @return QDateTime.
 */
QDateTime fromGcnTimestamp(uint32_t gcnTimestamp);

/**
 * Convert a QDateTime to GCN timestamp.
 *
 * NOTE: GCN doesn't support timezones, so these times
 * will all be handled as UTC.
 *
 * @param qDateTime QDateTime.
 * @return GCN timestamp.
 */
uint32_t toGcnTimestamp(const QDateTime &qDateTime);

/**
 * Convert a VMU timestamp to QDateTime.
 *
 * NOTE: Dreamcast doesn't support timezones, so these times
 * will all be set to UTC.
 *
 * @param vmuTimestamp VMU timestamp.
 * @return QDateTime.
 */
QDateTime fromVmuTimestamp(const struct _vmu_timestamp &vmuTimestamp);

}

#endif /* __LIBMEMCARD_TIMEFUNCS_HPP__ */
