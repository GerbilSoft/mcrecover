/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * TimeFuncs.hpp: Time conversion functions.                               *
 *                                                                         *
 * Copyright (c) 2012-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
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
