/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * ConfigStore.hpp: Configuration store.                                   *
 *                                                                         *
 * Copyright (c) 2011-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __MCRECOVER_CONFIG_CONFIGSTORE_HPP__
#define __MCRECOVER_CONFIG_CONFIGSTORE_HPP__

// Qt includes.
#include <QtCore/QObject>

class ConfigStorePrivate;

class ConfigStore : public QObject
{
Q_OBJECT
	
public:
	explicit ConfigStore(QObject *parent = 0);
	~ConfigStore();

private:
	ConfigStorePrivate *const d_ptr;
	Q_DECLARE_PRIVATE(ConfigStore)
private:
	Q_DISABLE_COPY(ConfigStore)

public:
	/**
	 * Reset all settings to defaults.
	 */
	void reset(void);

	/**
	 * Set a property.
	 * @param key Property name
	 * @param value Property value
	 */
	void set(const QString &key, const QVariant &value);

	/**
	 * Get a property.
	 * @param key Property name
	 * @return Property value
	 */
	QVariant get(const QString &key) const;

	/**
	 * Get a property.
	 * Converts hexadecimal string values to unsigned int.
	 * @param key Property name
	 * @return Property value
	 */
	unsigned int getUInt(const QString &key) const;

	/**
	 * Get a property.
	 * Converts hexadecimal string values to signed int.
	 * @param key Property name
	 * @return Property value
	 */
	int getInt(const QString &key) const;

	/**
	 * Get the default configuration path.
	 * @return Default configuration path
	 */
	static QString ConfigPath(void);

	/**
	 * Load the configuration file.
	 * @param filename Configuration filename
	 * @return 0 on success; non-zero on error.
	 */
	int load(const QString &filename);

	/**
	 * Load the configuration file.
	 * No filename specified; use the default filename.
	 * @return 0 on success; non-zero on error.
	 */
	int load(void);

	/**
	 * Save the configuration file.
	 * @param filename Filename
	 * @return 0 on success; non-zero on error.
	 */
	int save(const QString &filename) const;

	/**
	 * Save the configuration file.
	 * No filename specified; use the default filename.
	 * @return 0 on success; non-zero on error.
	 */
	int save(void) const;

	/**
	 * Register an object for property change notification.
	 * @param property Property to watch
	 * @param object QObject to register
	 * @param method Method name
	 */
	void registerChangeNotification(const QString &property, QObject *object, const char *method);

	/**
	 * Unregister an object for property change notification.
	 * @param property Property to watch
	 * @param object QObject to register
	 * @param method Method name
	 */
	void unregisterChangeNotification(const QString &property, QObject *object, const char *method);

	/**
	 * Notify all registered objects that configuration settings have changed.
	 * Useful when starting the program.
	 */
	void notifyAll(void);
};

#endif /* __MCRECOVER_CONFIG_CONFIGSTORE_HPP__ */
