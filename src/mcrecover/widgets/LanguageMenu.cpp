/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * LanguageMenu.cpp: QMenu subclass for selecting a UI language.           *
 *                                                                         *
 * Copyright (c) 2012-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "LanguageMenu.hpp"

// Translation Manager.
#include "TranslationManager.hpp"

// Qt includes.
#include <QtCore/QEvent>
#include <QtCore/QFile>
#include <QtCore/QHash>
#include <QtCore/QLocale>
#include <QtCore/QSignalMapper>
#include <QtGui/QIcon>
#include <QAction>
#include <QActionGroup>

/** LanguageMenuPrivate **/

class LanguageMenuPrivate
{
public:
	explicit LanguageMenuPrivate(LanguageMenu *q);

private:
	LanguageMenu *const q_ptr;
	Q_DECLARE_PUBLIC(LanguageMenu)
	Q_DISABLE_COPY(LanguageMenuPrivate)

public:
	// "System Default" language
	QAction *actLanguageSysDefault;
	// Key: Locale ID; value: QAction
	QHash<QString, QAction*> hashActions;

	// Action group and signal mapper
	// TODO: Remove for Qt5/Qt6.
	QActionGroup *actgrp;
	QSignalMapper *mapper;

	// Current language (locale tag, e.g. "en_US")
	// If empty, we're using the System Default language.
	QString locale;

	/**
	 * Get an icon for a given locale.
	 * @param locale Locale tag, e.g. "en_US".
	 * @return Icon, or null QIcon if not found.
	 */
	static QIcon iconForLocale(const QString &locale);

	/**
	 * Clear the Language Menu.
	 */
	void clear(void);

	/**
	 * Retranslate the "System Default" language action.
	 */
	void retranslateSystemDefault(void);

	/**
	 * Initialize the Language menu.
	 */
	void init(void);
};

LanguageMenuPrivate::LanguageMenuPrivate(LanguageMenu *q)
	: q_ptr(q)
	, actLanguageSysDefault(nullptr)
	, actgrp(nullptr)
	, mapper(nullptr)
{
	init();
}

/**
 * Clear the Language Menu.
 */
void LanguageMenuPrivate::clear(void)
{
	// Remove actions from the QMenu.
	Q_Q(LanguageMenu);
	q->clear();		// This deletes the QActions!
	hashActions.clear();

	// Delete the QActionGroup and QSignalMapper.
	delete actgrp;
	actgrp = nullptr;
	delete mapper;
	mapper = nullptr;
}

/**
 * Get an icon for a given locale.
 * @param locale Locale tag, e.g. "en_US".
 * @return Icon, or null QIcon if not found.
 */
QIcon LanguageMenuPrivate::iconForLocale(const QString &locale)
{
	// Check for an icon.

	// Check region, then language.
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	QStringList locale_parts = locale.split(QChar(L'_'), Qt::SkipEmptyParts);
#else /* QT_VERSION < QT_VERSION_CHECK(5, 15, 0) */
	QStringList locale_parts = locale.split(QChar(L'_'), QString::SkipEmptyParts);
#endif /* QT_VERSION >= QT_VERSION_CHECK(5, 15, 0) */

	QIcon flagIcon;
	for (int i = (locale_parts.size() - 1); i >= 0; i--) {
		QString filename = QLatin1String(":/flags/") +
				   locale_parts.at(i).toLower() +
				   QLatin1String(".png");
		// TODO: Optimize by getting rid of QFile::exists()?
		if (QFile::exists(filename)) {
			flagIcon = QIcon(filename);
			break;
		}
	}

	return flagIcon;
}

/**
 * Retranslate the "System Default" language action.
 */
void LanguageMenuPrivate::retranslateSystemDefault(void)
{
	if (!actLanguageSysDefault) {
		Q_Q(LanguageMenu);
		actLanguageSysDefault = new QAction(q);
		actLanguageSysDefault->setCheckable(true);
		QObject::connect(actLanguageSysDefault, SIGNAL(triggered()),
				 mapper, SLOT(map()));
	}

	QString localeSys = QLocale::system().name();

	//: Translation: System Default (retrieved from system settings)
	actLanguageSysDefault->setText(LanguageMenu::tr("System Default (%1)", "ts-language").arg(localeSys));
	mapper->setMapping(actLanguageSysDefault, QString());

	// Check for an icon.
	QIcon flagIcon = iconForLocale(localeSys);
	if (!flagIcon.isNull()) {
		actLanguageSysDefault->setIcon(flagIcon);
	}
}

/**
 * Initialize the Language menu.
 */
void LanguageMenuPrivate::init(void)
{
	// Clear the Language menu first.
	this->clear();

	// Initialize the QActionGroup and QSignalMapper.
	Q_Q(LanguageMenu);
	actgrp = new QActionGroup(q);
	mapper = new QSignalMapper(q);
	QObject::connect(mapper, SIGNAL(mapped(QString)),
			 q, SLOT(setLanguage(QString)));

	// Add the system default translation.
	retranslateSystemDefault();
	actgrp->addAction(actLanguageSysDefault);
	q->addAction(actLanguageSysDefault);

	// Add all other translations.
	q->addSeparator();
	QMap<QString, QString> tsMap = TranslationManager::instance()->enumerate();
	hashActions.reserve(tsMap.size());
	foreach (const QString &locale, tsMap.keys()) {
		const QString &language = tsMap.value(locale);
		if (hashActions.contains(language)) {
			// FIXME: Duplicate language?
			continue;
		}

		QAction *action = new QAction(language, q);
		action->setCheckable(true);

		// Check for an icon.
		QIcon flagIcon = iconForLocale(locale);
		if (!flagIcon.isNull()) {
			action->setIcon(flagIcon);
		}

		hashActions.insert(locale, action);
		actgrp->addAction(action);
		QObject::connect(action, SIGNAL(triggered()),
				 mapper, SLOT(map()));
		mapper->setMapping(action, locale);
		q->addAction(action);
	}
}

/** LanguageMenu **/

LanguageMenu::LanguageMenu(QWidget *parent)
	: super(parent)
	, d_ptr(new LanguageMenuPrivate(this))
{ }

LanguageMenu::LanguageMenu(const QString &title, QWidget *parent)
	: super(title, parent)
	, d_ptr(new LanguageMenuPrivate(this))
{ }

LanguageMenu::~LanguageMenu()
{
	delete d_ptr;
}

/**
 * Get the current language.
 * This will return an empty string if "System Default" is selected.
 * @return Locale tag, e.g. "en_US".
 */
QString LanguageMenu::language(void) const
{
	Q_D(const LanguageMenu);
	return d->locale;
}

/**
 * Set the current language.
 * @param locale Locale to set, or empty string for system default.
 * @return True if set successfully; false if not found.
 */
bool LanguageMenu::setLanguage(const QString &locale)
{
	Q_D(LanguageMenu);

	// Find the matching locale action.
	bool ret = true;
	QAction *action = nullptr;
	if (!locale.isEmpty()) {
		// Check if the locale is currently set.
		// NOTE: Only checking if an actual locale is specified
		// instead of "System Default", since the system default
		// locale might change at runtime.
		if (d->locale == locale)
			return true;

		// NOTE: If not found, assuming "System Default".
		action = d->hashActions.value(locale);
		ret = (action != nullptr);
	}
	d->locale = (action ? locale : QString());

	// Set the UI language.
	TranslationManager::instance()->setTranslation(
		(action != nullptr
			? locale
			: QLocale::system().name()));

	// Mark the language as selected.
	if (action) {
		action->setChecked(true);
	} else {
		d->actLanguageSysDefault->setChecked(true);
	}

	// Notify listeners.
	emit languageChanged(d->locale);
	return ret;
}

/**
 * Widget state has changed.
 * @param event State change event
 */
void LanguageMenu::changeEvent(QEvent *event)
{
	Q_D(LanguageMenu);

	switch (event->type()) {
		case QEvent::LanguageChange:
			// Retranslate the UI.
			d->retranslateSystemDefault();
			break;

		case QEvent::LocaleChange: {
			// Locale change usually requires a UI retranslation.
			QAction *action = d->actgrp->checkedAction();
			if (action) {
				action->trigger();
			}
			break;
		}

		default:
			break;
	}

	// Pass the event to the base class.
	super::changeEvent(event);
}
