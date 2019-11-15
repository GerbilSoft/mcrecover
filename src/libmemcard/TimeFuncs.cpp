/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * TimeFuncs.hpp: Time conversion functions.                               *
 *                                                                         *
 * Copyright (c) 2012-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "TimeFuncs.hpp"

// Card includes.
#include "vmu.h"

namespace TimeFuncs {

/**
 * Time difference between Unix and GCN epochs, in seconds.
 * GCN epoch is:  2000/01/01 12:00 AM UTC.
 * Unix epoch is: 1970/01/01 12:00 AM UTC.
 */
static const uint32_t GCN_EPOCH = 0x386D4380;

/**
 * Convert a GCN timestamp to QDateTime.
 *
 * NOTE: GCN doesn't support timezones, so these times
 * will all be set to UTC.
 *
 * @param gcnTimestamp GCN timestamp.
 * @return QDateTime.
 */
QDateTime fromGcnTimestamp(uint32_t gcnTimestamp)
{
	return QDateTime::fromMSecsSinceEpoch(
		((qint64)gcnTimestamp + GCN_EPOCH) * 1000, Qt::UTC);
}

/**
 * Convert a QDateTime to GCN timestamp.
 *
 * NOTE: GCN doesn't support timezones, so these times
 * will all be handled as UTC.
 *
 * @param qDateTime QDateTime.
 * @return GCN timestamp.
 */
uint32_t toGcnTimestamp(const QDateTime &qDateTime)
{
	// TODO: How will we handle 32-bit overflow?
	// TODO: How will we handle timestamps earlier than 2000/01/01?
	return (uint32_t)(qDateTime.toMSecsSinceEpoch() / 1000) - GCN_EPOCH;
}

/**
 * Convert a VMU timestamp to QDateTime.
 *
 * NOTE: Dreamcast doesn't support timezones, so these times
 * will all be set to UTC.
 *
 * @param vmuTimestamp VMU timestamp.
 * @return QDateTime.
 */
QDateTime fromVmuTimestamp(const vmu_timestamp &vmuTimestamp)
{
	// Timestamp is stored as BCD.
	// TODO: Better way to convert to/from BCD?
	int year =   ((vmuTimestamp.century >> 4) * 1000) +
		     ((vmuTimestamp.century & 0xF) * 100) +
		     ((vmuTimestamp.year >> 4) * 10) +
		      (vmuTimestamp.year & 0xF);
	int month =  ((vmuTimestamp.month >> 4) * 10) +
		      (vmuTimestamp.month & 0xF);
	int day =    ((vmuTimestamp.day >> 4) * 10) +
		      (vmuTimestamp.day & 0xF);
	int hour =   ((vmuTimestamp.hour >> 4) * 10) +
		      (vmuTimestamp.hour & 0xF);
	int minute = ((vmuTimestamp.minute >> 4) * 10) +
		      (vmuTimestamp.minute & 0xF);
	int second = ((vmuTimestamp.second >> 4) * 10) +
		      (vmuTimestamp.second & 0xF);

	// TODO: Verify that the values are in range.
	return QDateTime(QDate(year, month, day),
			 QTime(hour, minute, second), Qt::UTC);
}

}
