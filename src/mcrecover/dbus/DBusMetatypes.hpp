/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * DBusMetatypes.hpp: QtDBus metatypes.                                    *
 *                                                                         *
 * Copyright (c) 2013 by David Korth.                                      *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __MCRECOVER_DBUS_DBUSMETATYPES_HPP__
#define __MCRECOVER_DBUS_DBUSMETATYPES_HPP__

 /** QtDBus types. **/
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

#endif /* __MCRECOVER_DBUS_DBUSMETATYPES_HPP__ */
