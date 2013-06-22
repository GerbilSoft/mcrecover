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

// Qt includes.
#include <QtCore/QString>
#include <QtCore/QHash>

#include "PcreRegex.hpp"
#include "Checksum.hpp"
#include "VarModifierDef.hpp"

class GcnMcFileDef {
	public:
		enum regions_t {
			REGION_JPN = (1 << 0),
			REGION_USA = (1 << 1),
			REGION_EUR = (1 << 2),
			REGION_KOR = (1 << 3),
		};

	private:
		Q_DISABLE_COPY(GcnMcFileDef);

	public:
		// Game name.
		QString gameName;

		// File information.
		// Distinguishes between different types
		// of files saved by a single game.
		QString fileInfo;

		QString gamecode;
		QString company;

		// Regions this file definition applies to.
		uint8_t regions;

		struct {
			/**
			 * Comment search address.
			 * This is also used for dirEntry.
			 */
			uint32_t address;

			QString gameDesc;	// regex
			QString fileDesc;	// regex

			// Compiled regular expressions.
			PcreRegex gameDesc_regex;
			PcreRegex fileDesc_regex;
		} search;

		/**
		 * Checksum definitions.
		 * Some files have more than one checksum.
		 */
		QVector<Checksum::ChecksumDef> checksumDefs;

		struct {
			QString filename;
			uint8_t bannerFormat;
			uint32_t iconAddress;
			uint16_t iconFormat;
			uint16_t iconSpeed;
			uint8_t permission;
			uint16_t length;	// Length, in blocks.

			// NOTE: commentAddress is implied by search.address.
			//uint32_t commentAddress;
		} dirEntry;

		/**
		 * Variable modifiers.
		 * - Key: Variable ID.
		 * - Value: Variable modifier definition.
		 */
		QHash<QString, VarModifierDef> varModifiers;

		// Make sure all fields are initialized.
		GcnMcFileDef() {
			this->regions = 0;

			search.address = 0;

			dirEntry.bannerFormat = 0;
			dirEntry.iconAddress = 0;
			dirEntry.iconFormat = 0;
			dirEntry.iconSpeed = 0;
			dirEntry.permission = 0;
			dirEntry.length = 0;
		}
};

#endif /* __MCRECOVER_GCNMCFILEDEF_HPP__ */
