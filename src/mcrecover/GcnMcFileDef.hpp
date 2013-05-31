/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnMcFileDef.hpp: GCN Memory Card File Definition class.                *
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
 
#ifndef __MCRECOVER_GCNMCFILEDEF_HPP__
#define __MCRECOVER_GCNMCFILEDEF_HPP__

// C includes.
#include <stdint.h>

// libpcre
#include <pcre.h>

// Qt includes.
#include <QtCore/QString>

class GcnMcFileDef {
	public:
		enum regions_t {
			REGION_JPN = (1 << 0),
			REGION_USA = (1 << 1),
			REGION_EUR = (1 << 2),
			REGION_KOR = (1 << 3),
		};

		QString description;
		QString gamecode;
		QString company;

		// Regions this file definition applies to.
		uint8_t regions;

		struct {
			uint32_t address;
			QString gameDesc;	// regexp
			QString fileDesc;	// regexp

			// compiled regexps
			pcre *gameDesc_regexp;
			pcre *fileDesc_regexp;
		} search;

		struct {
			QString filename;
			uint8_t bannerFormat;
			uint32_t iconAddress;
			uint16_t iconFormat;
			uint16_t iconSpeed;
			uint8_t permission;
			uint16_t length;	// Length, in blocks.
			uint32_t commentAddress;
		} dirEntry;

		// Make sure all fields are initialized.
		GcnMcFileDef() {
			this->regions = 0;

			search.address = 0;
			search.gameDesc_regexp = NULL;
			search.fileDesc_regexp = NULL;

			dirEntry.bannerFormat = 0;
			dirEntry.iconAddress = 0;
			dirEntry.iconFormat = 0;
			dirEntry.iconSpeed = 0;
			dirEntry.permission = 0;
			dirEntry.length = 0;
			dirEntry.commentAddress = 0;
		}

		~GcnMcFileDef() {
			// Delete allocated PCRE regexps.
			if (search.gameDesc_regexp)
				pcre_free(search.gameDesc_regexp);
			if (search.fileDesc_regexp)
				pcre_free(search.fileDesc_regexp);
		}
};

#endif /* __MCRECOVER_GCNMCFILEDEF_HPP__ */
