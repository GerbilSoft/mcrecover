/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * ConfigDefaults.hpp: Configuration defaults.                             *
 *                                                                         *
 * Copyright (c) 2008-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "ConfigDefaults.hpp"

/**
 * Single instance of ConfigDefaults
 */
ConfigDefaults *ConfigDefaults::ms_Instance = nullptr;

/**
 * Default configuration filename
 */
const char *const ConfigDefaults::DefaultConfigFilename = "mcrecover.conf";

/**
 * Default settings
 */
const ConfigDefaults::DefaultSetting ConfigDefaults::DefaultSettings[] =
{
	/** Super hidden settings! **/
	{"iKnowWhatImDoingAndWillVoidTheWarranty", "false", 0, DefaultSetting::DEF_NO_SAVE, DefaultSetting::ValidationType::Boolean, 0, 0},

	/** General settings. **/
	{"lastPath",		"", 0, 0,	DefaultSetting::ValidationType::None, 0, 0},
	{"preferredRegion",	"E", 0, 0,	DefaultSetting::ValidationType::None, 0, 0},
	{"searchUsedBlocks",	"false", 0, 0,	DefaultSetting::ValidationType::Boolean, 0, 0},
	{"animIconFormat",	"APNG", 0, 0,	DefaultSetting::ValidationType::None, 0, 0},
	{"language",		"", 0, 0,	DefaultSetting::ValidationType::None, 0, 0},
	{"fileType",		"0", 0, 0,	DefaultSetting::ValidationType::None, 0, 0},

	/** End of array. **/
	{nullptr, nullptr, 0, 0, DefaultSetting::ValidationType::None, 0, 0}
};

ConfigDefaults::ConfigDefaults()
{
	// Populate the default settings hash.
	defaultSettingsHash.clear();
	for (const DefaultSetting *def = &DefaultSettings[0];
	     def->key != nullptr; def++)
	{
		const QString key = QLatin1String(def->key);
		defaultSettingsHash.insert(key, def);
	}
}

/**
 * Return a single instance of ConfigDefaults.
 * @return Single instance of ConfigDefaults
 */
ConfigDefaults *ConfigDefaults::Instance(void)
{
	if (ms_Instance)
		return ms_Instance;
	
	// Initialize ms_Instance.
	ms_Instance = new ConfigDefaults();
	return ms_Instance;
}
