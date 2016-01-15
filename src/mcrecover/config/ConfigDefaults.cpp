/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ConfigDefault.hpp: Default configuration.                               *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2012 by David Korth.                                 *
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

#include "ConfigDefaults.hpp"

/**
 * Single instance of ConfigDefaults.
 */
ConfigDefaults *ConfigDefaults::ms_Instance = nullptr;

/**
 * Default configuration filename.
 */
const char *const ConfigDefaults::DefaultConfigFilename = "mcrecover.conf";

/**
 * Default settings.
 */
const ConfigDefaults::DefaultSetting ConfigDefaults::DefaultSettings[] =
{
	/** Super hidden settings! **/
	{"iKnowWhatImDoingAndWillVoidTheWarranty", "false", 0, DefaultSetting::DEF_NO_SAVE, DefaultSetting::VT_BOOL, 0, 0},

	/** General settings. **/
	{"lastPath",		"", 0, 0,	DefaultSetting::VT_NONE, 0, 0},
	{"preferredRegion",	"E", 0, 0,	DefaultSetting::VT_NONE, 0, 0},
	{"searchUsedBlocks",	"false", 0, 0,	DefaultSetting::VT_BOOL, 0, 0},
	{"animIconFormat",	"APNG", 0, 0,	DefaultSetting::VT_NONE, 0, 0},
	{"language",		"", 0, 0,	DefaultSetting::VT_NONE, 0, 0},
	{"fileType",		"0", 0, 0,	DefaultSetting::VT_NONE, 0, 0},

	/** End of array. **/
	{nullptr, nullptr, 0, 0, DefaultSetting::VT_NONE, 0, 0}
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
 * @return Single instance of ConfigDefaults.
 */
ConfigDefaults *ConfigDefaults::Instance(void)
{
	if (ms_Instance)
		return ms_Instance;
	
	// Initialize ms_Instance.
	ms_Instance = new ConfigDefaults();
	return ms_Instance;
}
