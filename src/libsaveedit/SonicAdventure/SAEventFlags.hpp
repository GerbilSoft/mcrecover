/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SAEventFlags.hpp: Sonic Adventure - Event flags.                        *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include "../models/BitFlags.hpp"

class SAEventFlags : public BitFlags
{
	// TODO: Should this actually inherit from QObject?
	Q_OBJECT

	public:
		explicit SAEventFlags(QObject *parent = 0);

	private:
		typedef BitFlags super;
		Q_DISABLE_COPY(SAEventFlags)

	public:
		/**
		 * Get a description of the type of flag that is represented by the class.
		 * @return Flag type, e.g. "Event".
		 */
		QString flagType(void) const final;

		/**
		 * Get the desired page size for the BitFlagsModel.
		 * @return Page size.
		 */
		int pageSize(void) const final;

		/**
		 * Get the name for a given page of data.
		 *
		 * If pagination is enabled (pageSize > 0), this function is
		 * used to determine the text for the corresponding tab.
		 *
		 * @param page Page number.
		 * @return Page name.
		 */
		QString pageName(int page) const final;
};
