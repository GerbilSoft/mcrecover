/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * DBusMetatypes.hpp: QtDBus metatypes.                                    *
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
