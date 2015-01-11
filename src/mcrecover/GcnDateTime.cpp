/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnDateTime.cpp: QDateTime wrapper optimized for GCN timestamps.        *
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

/**
 * NOTE: This file is only necessary because we can't mark
 * the stream operators as static inline.
 */

#include "GcnDateTime.hpp"

#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &out, const GcnDateTime &date)
	{ return operator<<(out, date.m_dateTime); }

QDataStream &operator>>(QDataStream &in, GcnDateTime &date)
{
	QDataStream &ret = operator>>(in, date.m_dateTime);
	date.m_dateTime.setTimeSpec(Qt::UTC);
	return ret;
}

#if !defined(QT_NO_DEBUG_STREAM) && !defined(QT_NO_DATESTRING)
QDebug operator<<(QDebug dbg, const GcnDateTime &date)
{
	dbg.nospace() << QLatin1String("GcnDateTime(") << date.m_dateTime.toString() << QChar(L')');
	return dbg.space();
}
#endif
#endif /* QT_NO_DATASTREAM */

// VMU timestamps.
#include "vmu.h"

/**
 * Set the QDateTime using a Dreamcast VMU timestamp.
 * @param vmuTimestamp VMU timestamp.
 */
void GcnDateTime::setVmuTimestamp(vmu_timestamp vmuTimestamp)
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
	m_dateTime.setDate(QDate(year, month, day));
	m_dateTime.setTime(QTime(hour, minute, second));
}

/**
 * Get the VMU timestamp from the QDateTime.
 * @return VMU timestamp.
 */
vmu_timestamp GcnDateTime::vmuTimestamp(void) const
{
	// Timestamp is stored as BCD.
	// TODO: Better way to convert to/from BCD?
	vmu_timestamp timestamp;

	// Convert the date.
	QDate date = m_dateTime.date();
	int year = date.year();
	timestamp.year = (year % 10);
	year /= 10;
	timestamp.year |= ((year % 10) << 4);
	year /= 10;
	timestamp.century = (year % 10);
	year /= 10;
	timestamp.century |= ((year % 10) << 4);

	int month = date.month();
	timestamp.month = (month % 10);
	month /= 10;
	timestamp.month |= ((month % 10) << 4);

	int day = date.day();
	timestamp.day = (day % 10);
	day /= 10;
	timestamp.day |= ((day % 10) << 4);

	// VMU: 0 == Monday, 6 == Sunday
	// Qt: 1 == Monday, 7 == Sunday
	timestamp.day_of_week = (date.dayOfWeek() - 1);

	// Convert the time.
	QTime time = m_dateTime.time();
	int hour = time.hour();
	timestamp.hour = (hour % 10);
	hour /= 10;
	timestamp.hour |= ((hour % 10) << 4);

	int minute = time.minute();
	timestamp.minute = (minute % 10);
	minute /= 10;
	timestamp.minute |= ((minute % 10) << 4);

	int second = time.second();
	timestamp.second = (second % 10);
	second /= 10;
	timestamp.second |= ((second % 10) << 4);

	return timestamp;
}
