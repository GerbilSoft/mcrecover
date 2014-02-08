/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * ConfigDefaults.hpp: Configuration defaults.                             *
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

#ifndef __MCRECOVER_CONFIG_CONFIGDEFAULTS_HPP__
#define __MCRECOVER_CONFIG_CONFIGDEFAULTS_HPP__

// C includes.
#include <stdint.h>

// Qt includes.
#include <QtCore/QHash>

class ConfigDefaults
{
	public:
		static ConfigDefaults *Instance(void);

	private:
		ConfigDefaults();
		~ConfigDefaults();
		Q_DISABLE_COPY(ConfigDefaults)

		static ConfigDefaults *ms_Instance;

	public:
		// Default configuration filename.
		static const char *const DefaultConfigFilename;

		// Default settings.
		// TODO: Make this private.
		struct DefaultSetting {
			const char *key;
			const char *value;
			uint8_t hex_digits;	// If non-zero, saves as hexadecimal with this many digits.

			// Flags.
			enum DefaultSettingFlags {
				/**
				 * If true, allow a setting to be reset to the current value,
				 * which will result in property change signals being emitted
				 * regardless of whether or not the setting has actually changed.
				 *
				 * This is useful for e.g. "Savestate/saveSlot", since the user
				 * should be able to press the key corresponding to the current
				 * save slot in order to see the preview image for that savestate.
				 */
				DEF_ALLOW_SAME_VALUE	= (1 << 0),

				/**
				 * If true, don't save this setting in the configuration file.
				 * This will be used for some "hidden" settings that cannot
				 * be changed in the GUI, e.g. the option to show the
				 * VScroll Bug and Zero-Length DMA bug checkboxes in
				 * GeneralConfigWindow.
				 */
				DEF_NO_SAVE		= (1 << 1),
			};

			// Flags. Uses DefaultSettingFlags values.
			uint8_t flags;

			// Parameter validation.
			enum ValidationType {
				VT_NONE,		// No validation.
				VT_BOOL,		// Boolean; normalize to true/false.
				VT_COLOR,		// QColor.
				VT_RANGE,		// Integer range.
				
				VT_MAX
			};
			ValidationType validation;
			int range_min;
			int range_max;
		};
		static const DefaultSetting DefaultSettings[];

		/**
		 * Get a DefaultSetting struct.
		 * @param key Setting key.
		 * @return DefaultSetting struct, or nullptr if not found.
		 */
		const DefaultSetting *get(const QString &key) const;

	private:
		// Internal settings hash.
		QHash<QString, const DefaultSetting*> defaultSettingsHash;
};

/**
 * Get a DefaultSetting struct.
 * @param key Setting key.
 * @return DefaultSetting struct, or nullptr if not found.
 */
inline const ConfigDefaults::DefaultSetting *ConfigDefaults::get(const QString &key) const
	{ return defaultSettingsHash.value(key, nullptr); }

#endif /* __MCRECOVER_CONFIG_CONFIGDEFAULTS_HPP__ */
