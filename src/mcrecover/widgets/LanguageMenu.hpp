/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * LanguageMenu.hpp: QMenu subclass for selecting a UI language.           *
 *                                                                         *
 * Copyright (c) 2012-2016 by David Korth.                                 *
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

#ifndef __MCRECOVER_WIDGETS_LANGUAGEMENU_HPP__
#define __MCRECOVER_WIDGETS_LANGUAGEMENU_HPP__

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
		 * @param event State change event.
		 */
		virtual void changeEvent(QEvent *event) final;
};

#endif /* __MCRECOVER_WIDGETS_LANGUAGEMENU_HPP__ */
