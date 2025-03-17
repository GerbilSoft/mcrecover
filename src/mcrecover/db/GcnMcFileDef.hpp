/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnMcFileDef.hpp: GCN Memory Card File Definition class.                *
 *                                                                         *
 * Copyright (c) 2013-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

// C includes
#include <stdint.h>
#include <string.h>

// C++ includes
#include <vector>

// Qt includes
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QRegularExpression>

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
	// Game name
	QString gameName;

	// File information
	// Distinguishes between different types
	// of files saved by a single game.
	QString fileInfo;

	// ID6 (gamecode, company)
	union {
		char id6[6];
		struct {
			char gamecode[4];
			char company[2];
		};
	};

	// Regions this file definition applies to
	uint8_t regions;

	struct {
		/**
		 * Comment search address
		 * This is also used for dirEntry.
		 */
		uint32_t address;

		QString gameDesc;	// regex
		QString fileDesc;	// regex

		// Regular expressions
		QRegularExpression gameDesc_regex;
		QRegularExpression fileDesc_regex;
	} search;

	/**
	 * Checksum definitions
	 * Some files have more than one checksum.
	 */
	std::vector<Checksum::ChecksumDef> checksumDefs;

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
	 * Variable modifiers
	 * - Key: Variable ID.
	 * - Value: Variable modifier definition.
	 */
	QHash<QString, VarModifierDef> varModifiers;

	// Make sure all fields are initialized.
	GcnMcFileDef()
	{
		this->regions = 0;
		memset(id6, 0, sizeof(id6));

		search.address = 0;

		dirEntry.bannerFormat = 0;
		dirEntry.iconAddress = 0;
		dirEntry.iconFormat = 0;
		dirEntry.iconSpeed = 0;
		dirEntry.permission = 0;
		dirEntry.length = 0;
	}
};
