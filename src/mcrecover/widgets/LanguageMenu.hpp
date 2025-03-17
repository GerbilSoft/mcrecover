/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * LanguageMenu.hpp: QMenu subclass for selecting a UI language.           *
 *                                                                         *
 * Copyright (c) 2012-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include <QMenu>

class LanguageMenuPrivate;
class LanguageMenu : public QMenu
{
	Q_OBJECT

	Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)

public:
	explicit LanguageMenu(QWidget *parent = 0);
	LanguageMenu(const QString &title, QWidget *parent = 0);
	virtual ~LanguageMenu();

private:
	typedef QMenu super;
	LanguageMenuPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(LanguageMenu)
	Q_DISABLE_COPY(LanguageMenu)

public:
	/**
	 * Get the current language.
	 * This will return an empty string if "System Default" is selected.
	 * @return Locale tag, e.g. "en_US".
	 */
	QString language(void) const;

public slots:
	/**
	 * Set the current language.
	 * @param locale Locale to set, or empty string for system default.
	 * @return True if set successfully; false if not found.
	 */
	bool setLanguage(const QString &locale);

signals:
	/**
	 * User has selected a different language.
	 * LanguageMenu will automatically load the new translation.
	 * @param locale Locale tag, e.g. "en_US".
	 */
	void languageChanged(const QString &locale);

protected:
	/**
	 * Widget state has changed.
	 * @param event State change event
	 */
	void changeEvent(QEvent *event) final;
};
