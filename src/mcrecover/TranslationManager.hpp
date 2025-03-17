/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * TranslationManager.hpp: Qt translation manager.                         *
 *                                                                         *
 * Copyright (c) 2014-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __MCRECOVER_TRANSLATIONMGR_HPP__
#define __MCRECOVER_TRANSLATIONMGR_HPP__

// Qt includes.
#include <QtCore/QString>
#include <QtCore/QMap>

class TranslationManagerPrivate;

class TranslationManager
{
private:
	TranslationManager();
	~TranslationManager();

protected:
	TranslationManagerPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(TranslationManager)
private:
	Q_DISABLE_COPY(TranslationManager)

public:
	static TranslationManager *instance(void);

	/**
	 * Set the translation.
	 * @param locale Locale, e.g. "en_US". (Empty string is untranslated.)
	 */
	void setTranslation(const QString &locale);

	// TODO: Add a function to get the current translation?

	/**
	 * Enumerate available translations.
	 * NOTE: This only checks MemCard Recover translations.
	 * If a Qt translation exists but MemCard Recover doesn't have
	 * that translation, it won't show up.
	 * @return Map of available translations. (Key == locale, Value == description)
	 */
	QMap<QString, QString> enumerate(void) const;
};

#endif /* __MCRECOVER_TRANSLATIONMGR_HPP__ */
