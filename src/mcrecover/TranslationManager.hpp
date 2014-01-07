/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * TranslationManager.hpp: Qt translation manager.                         *
 *                                                                         *
 * Copyright (c) 2014 by David Korth.                                      *
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

#ifndef __MCRECOVER_TRANSLATIONMGR_HPP__
#define __MCRECOVER_TRANSLATIONMGR_HPP__

// Qt includes.
#include <QtCore/QString>

class TranslationManagerPrivate;

class TranslationManager
{
	private:
		TranslationManager();
		~TranslationManager();

	private:
		friend class TranslationManagerPriate;
		TranslationManagerPrivate *const d;
		Q_DISABLE_COPY(TranslationManager)

	public:
		static TranslationManager *instance(void);

		/**
		 * Set the translation.
		 * @param locale Locale, e.g. "en_US". (Empty string is untranslated.)
		 */
		void setTranslation(QString locale);
};

#endif /* __MCRECOVER_TRANSLATIONMGR_HPP__ */
