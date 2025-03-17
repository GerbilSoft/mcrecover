/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * DBusMetatypes.hpp: QtDBus metatypes.                                    *
 *                                                                         *
 * Copyright (c) 2013-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

// QtDBus types
#include <QtDBus/QDBusMetaType>

typedef QList<QByteArray> QByteArrayList;
Q_DECLARE_METATYPE(QByteArrayList)

typedef QMap<QString, QVariantMap> QVariantMapMap;
Q_DECLARE_METATYPE(QVariantMapMap)

typedef QMap<QDBusObjectPath, QVariantMapMap> DBusManagerStruct;
Q_DECLARE_METATYPE(DBusManagerStruct)

/**
 * Register the Qt DBus metatypes.
 */
static inline void registerDBusMetatypes(void)
{
	qDBusRegisterMetaType<QByteArrayList>();
	qDBusRegisterMetaType<QVariantMapMap>();
	qDBusRegisterMetaType<DBusManagerStruct>();
}
