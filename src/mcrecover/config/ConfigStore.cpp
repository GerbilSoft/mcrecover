/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * ConfigStore.cpp: Configuration store.                                   *
 *                                                                         *
 * Copyright (c) 2011-2014 by David Korth.                                 *
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

#include "ConfigStore.hpp"
#include "util/git.h"

// Qt includes.
#include <QtCore/QCoreApplication>
#include <QtCore/QSettings>
#include <QtCore/QHash>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QPointer>
#include <QtCore/QVector>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include <QtCore/QObject>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaMethod>

// Default settings.
#include "ConfigDefaults.hpp"

class ConfigStorePrivate
{
	public:
		ConfigStorePrivate(ConfigStore *q);
		~ConfigStorePrivate();

	private:
		ConfigStore *const q;
		Q_DISABLE_COPY(ConfigStorePrivate)

	public:
		// Configuration path.
		QString configPath;

		/**
		 * Validate a property.
		 * @param key Property name.
		 * @param value Property value. (May be edited for validation.)
		 * @return Property value (possibly adjusted) if validated; invalid QVariant on failure.
		 */
		static QVariant Validate(const QString &name, const QVariant &value);

		/**
		 * Look up the method index of a SIGNAL() or SLOT() in a QObject.
		 * @param object Qt object.
		 * @param method Method name.
		 * @return Method index, or negative on error.
		 */
		static int LookupQtMethod(const QObject *object, const char *method);

		/**
		 * Invoke a Qt method by method index, with one QVariant parameter.
		 * @param object Qt object.
		 * @param method_idx Method index.
		 * @param param QVariant parameter.
		 */
		static void InvokeQtMethod(QObject *object, int method_idx, const QVariant &param);

		/** Internal variables. **/

		// Current settings.
		// TODO: Use const char* for the key instead of QString?
		QHash<QString, QVariant> settings;

		/**
		 * Signal mappings.
		 * Format:
		 * - Key: Property to watch.
		 * - Value: List of SignalMaps.
		 *   - SignalMap.object: Object to send signal to.
		 *   - SignalMap.method: Method name.
		 */
		struct SignalMap {
			QPointer<QObject> object;
			int method_idx;
		};
		QHash<QString, QVector<SignalMap>* > signalMaps;
		QMutex mtxSignalMaps;
};

/** ConfigStorePrivate **/

ConfigStorePrivate::ConfigStorePrivate(ConfigStore* q)
	: q(q)
{
	// Determine the configuration path.
	// TODO: Portable mode.
	// TODO: Fallback if the user directory isn't writable.
	QSettings settings(QSettings::IniFormat, QSettings::UserScope,
				QLatin1String("mcrecover"),
				QLatin1String("mcrecover"));

	// TODO: Figure out if QDir has a function to remove the filename portion of the pathname.
	QString configPath = settings.fileName();
	int slash_pos = configPath.lastIndexOf(QChar(L'/'));
#ifdef Q_OS_WIN
	int bslash_pos = configPath.lastIndexOf(QChar(L'\\'));
	if (bslash_pos > slash_pos)
		slash_pos = bslash_pos;
#endif /* Q_OS_WIN */
	if (slash_pos >= 0)
		configPath.remove(slash_pos + 1, configPath.size());

	// Make sure the directory exists.
	// If it doesn't exist, create it.
	QDir configDir(configPath);
	if (!configDir.exists())
		configDir.mkpath(configDir.absolutePath());

	// Save the main configuration path.
	configPath = configDir.absolutePath();
	if (!configPath.endsWith(QChar(L'/')))
		configPath.append(QChar(L'/'));
}

ConfigStorePrivate::~ConfigStorePrivate()
{
	// Delete all the signal map vectors.
	qDeleteAll(signalMaps);
	signalMaps.clear();
}

/**
 * Validate a property.
 * @param key Property name.
 * @param value Property value. (May be edited for validation.)
 * @return Property value (possibly adjusted) if validated; invalid QVariant on failure.
 */
QVariant ConfigStorePrivate::Validate(const QString &name, const QVariant &value)
{
	// Get the DefaultSetting entry for this property.
	// TODO: Lock the hash?
	const ConfigDefaults::DefaultSetting *def = ConfigDefaults::Instance()->get(name);
	if (!def)
		return -1;

	switch (def->validation) {
		case ConfigDefaults::DefaultSetting::VT_NONE:
		default:
			// No validation required.
			return value;

		case ConfigDefaults::DefaultSetting::VT_BOOL:
			if (!value.canConvert(QVariant::Bool))
				return QVariant();
			return QVariant(value.toBool());

		case ConfigDefaults::DefaultSetting::VT_RANGE: {
			if (!value.canConvert(QVariant::Int))
				return QVariant();
			int val = value.toString().toInt(nullptr, 0);
			if (val < def->range_min || val > def->range_max)
				return QVariant();
			return QVariant(val);
		}
	}

	// Should not get here...
	return QVariant();
}

/**
 * Look up the method index of a SIGNAL() or SLOT() in a QObject.
 * @param object Qt object.
 * @param method Method name.
 * @return Method index, or negative on error.
 */
int ConfigStorePrivate::LookupQtMethod(const QObject *object, const char *method)
{
	// Based on QMetaObject::invokeMethod().
	
	// NOTE: The first character of method indicates whether it's a signal or slot.
	// We don't need this information, so we use method+1.
	method++;
	
	int idx = object->metaObject()->indexOfMethod(method);
	if (idx < 0)
	{
		QByteArray norm = QMetaObject::normalizedSignature(method);
		idx = object->metaObject()->indexOfMethod(norm.constData());
	}
	
	if (idx < 0 || idx >= object->metaObject()->methodCount())
	{
		// TODO: Do verification in registerChangeNotification()?
		fprintf(stderr, "ConfigStorePrivate::LookupQtMethod(): "
			"No such method %s::%s",
			object->metaObject()->className(), method);
		return -1;
	}
	
	// Method index found.
	return idx;
}

/**
 * Invoke a Qt method by method index, with one QVariant parameter.
 * @param object Qt object.
 * @param method_idx Method index.
 * @param param QVariant parameter.
 */
void ConfigStorePrivate::InvokeQtMethod(QObject *object, int method_idx, const QVariant &param)
{
	// Based on QMetaObject::invokeMethod().
	QMetaMethod metaMethod = object->metaObject()->method(method_idx);
	metaMethod.invoke(object, Qt::AutoConnection,
		      QGenericReturnArgument(), Q_ARG(QVariant, param));
}

/** ConfigStore **/

ConfigStore::ConfigStore(QObject *parent)
	: QObject(parent)
	, d(new ConfigStorePrivate(this))
{
	// Initialize defaults and load user settings.
	reset();
	load();
}

ConfigStore::~ConfigStore()
{
	// Save the configuration.
	// TODO: Handle non-default filenames.
	save();

	delete d;
}

/**
 * Reset all settings to defaults.
 */
void ConfigStore::reset(void)
{
	// Initialize settings with DefaultSettings.
	d->settings.clear();
	for (const ConfigDefaults::DefaultSetting *def = &ConfigDefaults::DefaultSettings[0];
	     def->key != nullptr; def++)
	{
		d->settings.insert(QLatin1String(def->key),
			(def->value ? QLatin1String(def->value) : QString()));
	}
}

/**
 * Set a property.
 * @param key Property name.
 * @param value Property value.
 */
void ConfigStore::set(const QString &key, const QVariant &value)
{
#ifndef NDEBUG
	// Make sure this property exists.
	if (!d->settings.contains(key)) {
		// Property does not exist. Print a warning.
		// TODO: Make this an error, since it won't be saved?
		fprintf(stderr, "ConfigStore: Property '%s' has no default value. FIX THIS!",
			key.toUtf8().constData());
	}
#endif

	// Get the default value.
	const ConfigDefaults::DefaultSetting *def = ConfigDefaults::Instance()->get(key);
	if (!def)
		return;

	if (!(def->flags & ConfigDefaults::DefaultSetting::DEF_ALLOW_SAME_VALUE)) {
		// Check if the new value is the same as the old value.
		QVariant oldValue = d->settings.value(key);
		if (value == oldValue)
			return;
	}

	// Verify that this value passes validation.
	QVariant newValue = ConfigStorePrivate::Validate(key, value);
	if (!newValue.isValid())
		return;

	// Set the new value.
	d->settings.insert(key, newValue);

	// Invoke methods for registered objects.
	QMutexLocker mtxLocker(&d->mtxSignalMaps);
	QVector<ConfigStorePrivate::SignalMap> *signalMapVector = d->signalMaps.value(key, nullptr);
	if (!signalMapVector)
		return;

	// Process the signal map vector in reverse-order.
	// Reverse order makes it easier to remove deleted objects.
	// TODO: Use QLinkedList instead?
	for (int i = (signalMapVector->size() - 1); i >= 0; i--) {
		const ConfigStorePrivate::SignalMap *smap = &signalMapVector->at(i);
		if (smap->object.isNull()) {
			signalMapVector->remove(i);
		} else {
			// Invoke this method.
			ConfigStorePrivate::InvokeQtMethod(smap->object, smap->method_idx, newValue);
		}
	}
}

/**
 * Get a property.
 * @param key Property name.
 * @return Property value.
 */
QVariant ConfigStore::get(const QString &key) const
{
#ifndef NDEBUG
	// Make sure this property exists.
	if (!d->settings.contains(key)) {
		// Property does not exist. Print a warning.
		// TODO: Make this an error, since it won't be saved?
		fprintf(stderr, "ConfigStore: Property '%s' has no default value. FIX THIS!",
			key.toUtf8().constData());
	}
#endif

	return d->settings.value(key);
}

/**
 * Get a property.
 * Converts hexadecimal string values to unsigned int.
 * @param key Property name.
 * @return Property value.
 */
unsigned int ConfigStore::getUInt(const QString &key) const
{
	return get(key).toString().toUInt(nullptr, 0);
}

/**
 * Get a property.
 * Converts hexadecimal string values to signed int.
 * @param key Property name.
 * @return Property value.
 */
int ConfigStore::getInt(const QString &key) const
{
	return get(key).toString().toInt(nullptr, 0);
}

/**
 * Load the configuration file.
 * @param filename Configuration filename.
 * @return 0 on success; non-zero on error.
 */
int ConfigStore::load(const QString &filename)
{
	QSettings qSettings(filename, QSettings::IniFormat);

	// NOTE: Only known settings will be loaded.
	d->settings.clear();
	// TODO: Add function to get sizeof(DefaultSettings) from ConfigDefaults.
	d->settings.reserve(32);

	// Load known settings from the configuration file.
	for (const ConfigDefaults::DefaultSetting *def = &ConfigDefaults::DefaultSettings[0];
	     def->key != nullptr; def++)
	{
		const QString key = QLatin1String(def->key);
		QVariant value = qSettings.value(key, QLatin1String(def->value));

		// Validate this value.
		value = ConfigStorePrivate::Validate(key, value);
		if (!value.isValid()) {
			// Validation failed. Use the default value.
			value = QVariant(QLatin1String(def->value));
		}

		d->settings.insert(key, value);
	}

	// Finished loading settings.
	// NOTE: Caller must call emitAll() for settings to take effect.
	return 0;
}

/**
 * Load the configuration file.
 * No filename specified; use the default filename.
 * @return 0 on success; non-zero on error.
 */
int ConfigStore::load(void)
{
	const QString cfgFilename = d->configPath +
		QLatin1String(ConfigDefaults::DefaultConfigFilename);
	return load(cfgFilename);
}

/**
 * Save the configuration file.
 * @param filename Filename.
 * @return 0 on success; non-zero on error.
 */
int ConfigStore::save(const QString &filename) const
{
	QSettings qSettings(filename, QSettings::IniFormat);

	/** Application information. **/

	// Stored in the "General" section.
	// TODO: Move "General" settings to another section?
	// ("General" is always moved to the top of the file.)
	qSettings.setValue(QLatin1String("_Application"), QCoreApplication::applicationName());
	qSettings.setValue(QLatin1String("_Version"), QCoreApplication::applicationVersion());

#ifdef MCRECOVER_GIT_VERSION
	qSettings.setValue(QLatin1String("_VersionVcs"),
				QLatin1String(MCRECOVER_GIT_VERSION));
#ifdef MCRECOVER_GIT_DESCRIBE
	qSettings.setValue(QLatin1String("_VersionVcsExt"),
				QLatin1String(MCRECOVER_GIT_DESCRIBE));
#else
	qSettings.remove(QLatin1String("_VersionVcsExt"));
#endif /* MCRECOVER_GIT_DESCRIBE */
#else
	qSettings.remove(QLatin1String("_VersionVcs"));
	qSettings.remove(QLatin1String("_VersionVcsExt"));
#endif /* MCRECOVER_GIT_DESCRIBE */

	// NOTE: Only known settings will be saved.
	
	// Save known settings to the configuration file.
	for (const ConfigDefaults::DefaultSetting *def = &ConfigDefaults::DefaultSettings[0];
	     def->key != nullptr; def++)
	{
		if (def->flags & ConfigDefaults::DefaultSetting::DEF_NO_SAVE)
			continue;

		const QString key = QLatin1String(def->key);
		QVariant value = d->settings.value(key, QLatin1String(def->value));
		if (def->hex_digits > 0) {
			// Convert to hexadecimal.
			unsigned int uint_val = value.toString().toUInt(nullptr, 0);
			QString str = QLatin1String("0x") +
					QString::number(uint_val, 16).toUpper().rightJustified(4, QChar(L'0'));
			value = str;
		}

		qSettings.setValue(key, value);
	}

	return 0;
}

/**
 * Save the configuration file.
 * No filename specified; use the default filename.
 * @return 0 on success; non-zero on error.
 */
int ConfigStore::save(void) const
{
	const QString cfgFilename = d->configPath +
		QLatin1String(ConfigDefaults::DefaultConfigFilename);
	return save(cfgFilename);
}

/**
 * Register an object for property change notification.
 * @param property Property to watch.
 * @param object QObject to register.
 * @param method Method name.
 */
void ConfigStore::registerChangeNotification(const QString &property, QObject *object, const char *method)
{
	if (!object || !method || property.isEmpty())
		return;

	// Get the vector of signal maps for this property.
	QMutexLocker mtxLocker(&d->mtxSignalMaps);
	QVector<ConfigStorePrivate::SignalMap>* signalMapVector =
		d->signalMaps.value(property, nullptr);
	if (!signalMapVector) {
		// No vector found. Create one.
		signalMapVector = new QVector<ConfigStorePrivate::SignalMap>();
		d->signalMaps.insert(property, signalMapVector);
	}

	// Look up the method index.
	int method_idx = ConfigStorePrivate::LookupQtMethod(object, method);
	if (method_idx < 0)
		return;

	// Add this object and slot to the signal maps vector.
	ConfigStorePrivate::SignalMap smap;
	smap.object = object;
	smap.method_idx = method_idx;
	signalMapVector->append(smap);
}

/**
 * Unregister an object for property change notification.
 * @param property Property to watch.
 * @param object QObject to register.
 * @param method Method name.
 */
void ConfigStore::unregisterChangeNotification(const QString &property, QObject *object, const char *method)
{
	if (!object)
		return;

	// Get the vector of signal maps for this property.
	QMutexLocker mtxLocker(&d->mtxSignalMaps);
	QVector<ConfigStorePrivate::SignalMap>* signalMapVector =
		d->signalMaps.value(property, nullptr);
	if (!signalMapVector)
		return;

	// Get the method index.
	int method_idx = -1;
	if (method != nullptr) {
		method_idx = ConfigStorePrivate::LookupQtMethod(object, method);
		if (method_idx < 0)
			return;
	}

	// Process the signal map vector in reverse-order.
	// Reverse order makes it easier to remove deleted objects.
	// TODO: Use QLinkedList instead?
	for (int i = (signalMapVector->size() - 1); i >= 0; i--) {
		const ConfigStorePrivate::SignalMap *smap = &signalMapVector->at(i);
		if (smap->object.isNull()) {
			signalMapVector->remove(i);
		} else if (smap->object == object) {
			// Found the object.
			if (method == nullptr || method_idx == smap->method_idx) {
				// Found a matching signal map.
				signalMapVector->remove(i);
			}
		}
	}
}

/**
 * Notify all registered objects that configuration settings have changed.
 * Useful when starting the program.
 */
void ConfigStore::notifyAll(void)
{
	// Invoke methods for registered objects.
	QMutexLocker mtxLocker(&d->mtxSignalMaps);

	foreach (QString property, d->signalMaps.keys()) {
		QVector<ConfigStorePrivate::SignalMap> *signalMapVector =
			d->signalMaps.value(property);
		if (signalMapVector->isEmpty())
			continue;

		// Get the property value.
		const QVariant value = d->settings.value(property);

		// Process the signal map vector in reverse-order.
		// Reverse order makes it easier to remove deleted objects.
		// TODO: Use QLinkedList instead?
		for (int i = (signalMapVector->size() - 1); i >= 0; i--)
		{
			const ConfigStorePrivate::SignalMap *smap = &signalMapVector->at(i);
			if (smap->object.isNull()) {
				signalMapVector->remove(i);
			} else {
				// Invoke this method.
				ConfigStorePrivate::InvokeQtMethod(smap->object, smap->method_idx, value);
			}
		}
	}
}
