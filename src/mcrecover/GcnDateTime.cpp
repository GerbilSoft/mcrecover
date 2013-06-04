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
