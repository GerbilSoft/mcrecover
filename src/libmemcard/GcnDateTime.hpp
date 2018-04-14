/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * GcnDateTime.hpp: QDateTime wrapper optimized for GCN timestamps.        *
 *                                                                         *
 * Copyright (c) 2013-2018 by David Korth.                                 *
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

#ifndef __LIBMEMCARD_GCNDATETIME_HPP__
#define __LIBMEMCARD_GCNDATETIME_HPP__

// C includes.
#include <stdint.h>

// Qt includes.
#include <QtCore/qconfig.h>
#include <QtCore/QDate>
#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QTime>
#include <QtCore/QMetaType>

// Stream operators.
#ifndef QT_NO_DATASTREAM
#include <QtCore/QDataStream>
#if !defined(QT_NO_DEBUG_STREAM) && !defined(QT_NO_DATESTRING)
#include <QtCore/QDebug>
#endif
#endif /* QT_NO_DATASTREAM */

// from vmu.h
extern "C" struct _vmu_timestamp;

class GcnDateTime
{
	public:
		GcnDateTime();
		explicit GcnDateTime(uint32_t gcnTimestamp);
		explicit GcnDateTime(const _vmu_timestamp &vmuTimestamp);
		explicit GcnDateTime(const QDateTime &other);
		GcnDateTime(const GcnDateTime &other);

		/**
		 * Time difference between Unix and GCN epochs, in seconds.
		 * GCN epoch is:  2000/01/01 12:00 AM UTC.
		 * Unix epoch is: 1970/01/01 12:00 AM UTC.
		 */
		static const uint32_t GCN_EPOCH = 0x386D4380;

		/**
		 * Set the QDateTime using a GCN timestamp.
		 * @param gcnTimestamp GCN timestamp.
		 */
		void setGcnTimestamp(uint32_t gcnTimestamp);

		/**
		 * Set the QDateTime using a Unix timestamp.
		 * FIXME: 64-bit timestamp?
		 * @param gcnTimestamp GCN timestamp.
		 */
		void setUnixTimestamp(uint32_t unixTimestamp);

		/**
		 * Set the QDateTime using a Dreamcast VMU timestamp.
		 * @param vmuTimestamp VMU timestamp.
		 */
		void setVmuTimestamp(const _vmu_timestamp &vmuTimestamp);

		/**
		 * Get the GCN timestamp from the QDateTime.
		 * @return GCN timestamp.
		 */
		uint32_t gcnTimestamp(void) const;

		/**
		 * Get the Unix timestamp from the QDateTime.
		 * FIXME: 64-bit timestamp?
		 * @return Unix timestamp.
		 */
		uint32_t unixTimestamp(void) const;

		/**
		 * Get the VMU timestamp from the QDateTime.
		 * @return VMU timestamp.
		 */
		_vmu_timestamp vmuTimestamp(void) const;

		/**
		 * Set the QDateTime using a QDateTime.
		 * @param qDateTime QDateTime.
		 */
		void setQDateTime(const QDateTime &qDateTime);

		/**
		 * Get the QDateTime from the QDateTime.
		 * @return QDateTime.
		 */
		QDateTime qDateTime(void) const;

		/**
		 * Set the date.
		 * @param date Date.
		 */
		void setDate(const QDate &date);

		/**
		 * Get the date.
		 * @return Date.
		 */
		QDate date(void) const;

		/**
		 * Set the time.
		 * @param time Time.
		 */
		void setTime(const QTime &time);

		/**
		 * Get the time.
		 * @return Time.
		 */
		QTime time(void) const;

		/**
		 * Get the current date and time.
		 * @return Current date and time.
		 */
		static GcnDateTime currentDateTime(void);

		/** Convert to string. **/
#ifndef QT_NO_DATESTRING
		QString toString(Qt::DateFormat f = Qt::TextDate) const;
		QString toString(const QString &format) const;
#endif

		/** Comparison operators: this vs. GcnDateTime **/
		inline bool operator==(const GcnDateTime &other) const
			{ return (this->m_dateTime == other.m_dateTime); }
		inline bool operator!=(const GcnDateTime &other) const
			{ return (this->m_dateTime != other.m_dateTime); }
		inline bool operator<(const GcnDateTime &other) const
			{ return (this->m_dateTime < other.m_dateTime); }
		inline bool operator<=(const GcnDateTime &other) const
			{ return (this->m_dateTime <= other.m_dateTime); }
		inline bool operator>(const GcnDateTime &other) const
			{ return (this->m_dateTime > other.m_dateTime); }
		inline bool operator>=(const GcnDateTime &other) const
			{ return (this->m_dateTime >= other.m_dateTime); }

		/** Comparison operators: this vs. QDateTime **/
		inline bool operator==(const QDateTime &other) const
			{ return (this->m_dateTime == other); }
		inline bool operator!=(const QDateTime &other) const
			{ return (this->m_dateTime != other); }
		inline bool operator<(const QDateTime &other) const
			{ return (this->m_dateTime < other); }
		inline bool operator<=(const QDateTime &other) const
			{ return (this->m_dateTime <= other); }
		inline bool operator>(const QDateTime &other) const
			{ return (this->m_dateTime > other); }
		inline bool operator>=(const QDateTime &other) const
			{ return (this->m_dateTime >= other); }

		/** Stream operators. (Friend relations) **/
#ifndef QT_NO_DATASTREAM
		friend QDataStream &operator<<(QDataStream &out, const GcnDateTime &date);
		friend QDataStream &operator>>(QDataStream &in, GcnDateTime &date);
#if !defined(QT_NO_DEBUG_STREAM) && !defined(QT_NO_DATESTRING)
		friend QDebug operator<<(QDebug dbg, const GcnDateTime &date);
#endif
#endif /* QT_NO_DATASTREAM */

	private:
		QDateTime m_dateTime;
};

Q_DECLARE_METATYPE(GcnDateTime)

/**
 * Initialize a null GcnDateTime.
 */
inline GcnDateTime::GcnDateTime()
{
	// GCN timestamps don't have timezones associated with them.
	m_dateTime.setTimeSpec(Qt::UTC);

	// Initialize the timestamp to the Unix epoch.
	// (1970/01/01 12:00 AM UTC)
	setGcnTimestamp(0);
}

/**
 * Create a GcnDateTime from a GCN timestamp.
 * @param gcnTimestamp GCN timestamp. (seconds since 2000/01/01 12:00 AM UTC)
 */
inline GcnDateTime::GcnDateTime(uint32_t gcnTimestamp)
{
	// GCN timestamps don't have timezones associated with them.
	m_dateTime.setTimeSpec(Qt::UTC);

	// Set the timestamp.
	setGcnTimestamp(gcnTimestamp);
}

/**
 * Create a GcnDateTime from a VMU timestamp.
 * @param vmuTimestamp VMU timestamp.
 */
inline GcnDateTime::GcnDateTime(const _vmu_timestamp &vmuTimestamp)
{
	// GCN timestamps don't have timezones associated with them.
	m_dateTime.setTimeSpec(Qt::UTC);

	// Set the timestamp.
	setVmuTimestamp(vmuTimestamp);
}

/**
 * Create a GcnDateTime from a QDateTime.
 * @param other QDateTime.
 */
inline GcnDateTime::GcnDateTime(const QDateTime &other)
	: m_dateTime(other)
{
	// GCN timestamps don't have timezones associated with them.
	m_dateTime.setTimeSpec(Qt::UTC);
}

/**
 * Create a GcnDateTime from another GcnDateTime.
 * @param other GcnDateTime.
 */
inline GcnDateTime::GcnDateTime(const GcnDateTime &other)
	: m_dateTime(other.qDateTime())
{
	// GCN timestamps don't have timezones associated with them.
	m_dateTime.setTimeSpec(Qt::UTC);
}

/**
 * Set the GcnDateTime using a GCN timestamp.
 * @param gcnTimestamp GCN timestamp.
 */
inline void GcnDateTime::setGcnTimestamp(uint32_t gcnTimestamp)
	{ m_dateTime.setTime_t(gcnTimestamp + GCN_EPOCH); }

/**
 * Set the GcnDateTime using a Unix timestamp.
 * FIXME: 64-bit timestamp?
 * @param unixTimestamp Unix timestamp.
 */
inline void GcnDateTime::setUnixTimestamp(uint32_t unixTimestamp)
	{ m_dateTime.setTime_t(unixTimestamp); }

/**
 * Get the GCN timestamp from the QDateTime.
 * @return GCN timestamp.
 */
inline uint32_t GcnDateTime::gcnTimestamp(void) const
	{ return (m_dateTime.toTime_t() - GCN_EPOCH); }

/**
 * Get the GCN timestamp from the QDateTime.
 * FIXME: 64-bit timestamp?
 * @return Unix timestamp.
 */
inline uint32_t GcnDateTime::unixTimestamp(void) const
	{ return m_dateTime.toTime_t(); }

/**
 * Set the QDateTime using a QDateTime.
 * @param qDateTime QDateTime.
 */
inline void GcnDateTime::setQDateTime(const QDateTime &qDateTime)
{
	m_dateTime = qDateTime;
	m_dateTime.setTimeSpec(Qt::UTC);
}

/**
 * Get the QDateTime from the QDateTime.
 * @return QDateTime.
 */
inline QDateTime GcnDateTime::qDateTime(void) const
	{ return m_dateTime; }

/**
 * Set the date.
 * @param date Date.
 */
inline void GcnDateTime::setDate(const QDate &date)
	{ m_dateTime.setDate(date); }

/**
 * Get the date.
 * @return Date.
 */
inline QDate GcnDateTime::date(void) const
	{ return m_dateTime.date(); }

/**
 * Set the time.
 * @param time Time.
 */
inline void GcnDateTime::setTime(const QTime &time)
	{ m_dateTime.setTime(time); }

/**
 * Get the time.
 * @return Time.
 */
inline QTime GcnDateTime::time(void) const
	{ return m_dateTime.time(); }

/**
 * Get the current date and time.
 * @return Current date and time.
 */
inline GcnDateTime GcnDateTime::currentDateTime(void)
	{ return GcnDateTime(QDateTime::currentDateTime()); }

/** Convert to string. **/
#ifndef QT_NO_DATESTRING
inline QString GcnDateTime::toString(Qt::DateFormat f) const
	{ return m_dateTime.toString(f); }
inline QString GcnDateTime::toString(const QString &format) const
	{ return m_dateTime.toString(format); }
#endif

#endif /* __MCRECOVER_GCNDATETIME_HPP__ */
