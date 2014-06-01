/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * McRecoverWindow.cpp: Main window.                                       *
 *                                                                         *
 * Copyright (c) 2012-2014 by David Korth.                                 *
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

#include "config.mcrecover.h"
#include "McRecoverWindow.hpp"
#include "util/array_size.h"

#include "McRecoverQApplication.hpp"
#include "AboutDialog.hpp"

// PcreRegex is needed to access pcre configuration.
#include "PcreRegex.hpp"

// MemCard classes.
#include "MemCard.hpp"
#include "MemCardFile.hpp"
#include "MemCardModel.hpp"
#include "MemCardItemDelegate.hpp"
#include "MemCardSortFilterProxyModel.hpp"

// File database.
#include "GcnMcFileDb.hpp"

// Search classes.
#include "SearchThread.hpp"
#include "widgets/StatusBarManager.hpp"

// Translation Manager.
#include "TranslationManager.hpp"

// C includes. (C++ namespace)
#include <cstdio>
#include <cassert>

// C++ includes.
#include <vector>
using std::vector;

// Qt includes.
#include <QtCore/QUrl>
#include <QtCore/QStack>
#include <QtCore/QVector>
#include <QtCore/QFile>
#include <QtCore/QSignalMapper>
#include <QtCore/QLocale>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QFileDialog>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QToolBar>

// GcImageWriter.
#include "GcImageWriter.hpp"

// Configuration.
#include "config/ConfigStore.hpp"
#include "PathFuncs.hpp"

// Shh... it's a secret to everybody.
#include "sekrit/HerpDerpEggListener.hpp"

/** McRecoverWindowPrivate **/

#include "ui_McRecoverWindow.h"
class McRecoverWindowPrivate
{
	public:
		McRecoverWindowPrivate(McRecoverWindow *q);
		~McRecoverWindowPrivate();

	protected:
		McRecoverWindow *const q_ptr;
		Q_DECLARE_PUBLIC(McRecoverWindow)
	private:
		Q_DISABLE_COPY(McRecoverWindowPrivate)

	public:
		Ui::McRecoverWindow ui;

		// Is scanning disabled?
		// (Usually due to PCRE not supporting UTF-8.)
		bool scanningDisabled;

		// Memory Card.
		MemCard *card;
		MemCardModel *model;
		MemCardSortFilterProxyModel *proxyModel;

		/**
		 * Show a warning if the card (or files on the card) are Japanese,
		 * but PCRE doesn't have Unicode character properties support.
		 */
		void showJpWarning(void);

		// Filename.
		QString filename;
		QString displayFilename;	// filename without subdirectories

		/**
		 * Update the memory card's QTreeView.
		 */
		void updateLstFileList(void);

		// Search thread.
		SearchThread *searchThread;

		/**
		 * Initialize the toolbar.
		 */
		void initToolbar(void);

		/**
		 * Retranslate the toolbar.
		 */
		void retranslateToolbar(void);

		/**
		 * Update the action enable status.
		 */
		void updateActionEnableStatus(void);

		// Status Bar Manager.
		StatusBarManager *statusBarManager;

		/**
		 * Update the window title.
		 */
		void updateWindowTitle(void);

		/**
		 * Change the file extension of the specified file.
		 * @param filename Filename.
		 * @param newExt New extension, including leading dot.
		 * @return Filename with new extension.
		 */
		static QString changeFileExtension(const QString &filename, const QString &newExt);

		/**
		 * Save the specified file(s).
		 * @param files List of file(s) to save.
		 * @param path If specified, save file(s) to path using default GCI filenames.
		 */
		void saveFiles(const QVector<MemCardFile*> &files, QString path = QString());

		// UI busy counter.
		int uiBusyCounter;

		/**
		 * "Preferred Region" selection.
		 * Possible values: 0, 'E', 'P', 'J', 'K'
		 * 0 indicates no preferred region.
		 */
		char preferredRegion;
		QLabel *lblPreferredRegion;
		QActionGroup *actgrpRegion;
		QSignalMapper *mapperPreferredRegion;

		// Translations.
		QAction *actTsSysDefault;
		// Key: Locale ID; value: QAction
		QHash<QString, QAction*> hashActionsTS;
		QActionGroup *actgrpTS;
		QSignalMapper *mapperTS;

		/**
		 * Get an icon for a given locale.
		 * @param locale Locale name, e.g. "en_US".
		 * @return Icon, or null QIcon if not found.
		 */
		static QIcon IconForLocale(const QString &locale);

		/**
		 * Retranslate the "System Default" language action.
		 */
		void rets_actTsSysDefault(void);

		/**
		 * Initialize the Translations menu.
		 */
		void initTsMenu(void);

		/**
		 * "Animated Icon Format" selection.
		 */
		QActionGroup *actgrpAnimIconFormat;
		QSignalMapper *mapperAnimIconFormat;

		// Configuration.
		ConfigStore *cfg;

		/**
		 * Get the last path.
		 * @return Last path.
		 */
		QString lastPath(void) const;

		/**
		 * Set the last path.
		 * @param path Last path.
		 */
		void setLastPath(const QString &path);

		/**
		 * Get the animated icon format to use.
		 * @return Animated icon format to use.
		 */
		GcImageWriter::AnimImageFormat animIconFormat(void) const;

		// Shh... it's a secret to everybody.
		HerpDerpEggListener *herpDerp;
};

McRecoverWindowPrivate::McRecoverWindowPrivate(McRecoverWindow *q)
	: q_ptr(q)
	, scanningDisabled(false)
	, card(nullptr)
	, model(new MemCardModel(q))
	, proxyModel(new MemCardSortFilterProxyModel(q))
	, searchThread(new SearchThread(q))
	, statusBarManager(nullptr)
	, uiBusyCounter(0)
	, preferredRegion(0)
	, lblPreferredRegion(nullptr)
	, actgrpRegion(new QActionGroup(q))
	, mapperPreferredRegion(new QSignalMapper(q))
	, actTsSysDefault(nullptr)
	, actgrpTS(nullptr)
	, mapperTS(new QSignalMapper(q))
	, actgrpAnimIconFormat(new QActionGroup(q))
	, mapperAnimIconFormat(new QSignalMapper(q))
	, cfg(new ConfigStore(q))
	, herpDerp(new HerpDerpEggListener(q))
{
	// Connect the MemCardModel slots.
	QObject::connect(model, SIGNAL(layoutChanged()),
			 q, SLOT(memCardModel_layoutChanged()));
	QObject::connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
			 q, SLOT(memCardModel_rowsInserted()));

	// Connect the SearchThread slots.
	QObject::connect(searchThread, SIGNAL(searchFinished(int)),
			 q, SLOT(searchThread_searchFinished_slot(int)));

	// Connect SearchThread to the mark-as-busy slots.
	QObject::connect(searchThread, SIGNAL(searchStarted(int,int,int)),
			 q, SLOT(markUiBusy()));
	QObject::connect(searchThread, SIGNAL(searchFinished(int)),
			 q, SLOT(markUiNotBusy()));
	QObject::connect(searchThread, SIGNAL(searchError(QString)),
			 q, SLOT(markUiNotBusy()));
	QObject::connect(searchThread, SIGNAL(destroyed(QObject*)),
			 q, SLOT(markUiNotBusy()));

	// Connect the QSignalMapper slot for "Preferred Region" selection.
	QObject::connect(mapperPreferredRegion, SIGNAL(mapped(int)),
			 q, SLOT(setPreferredRegion_slot(int)));

	// Connect the QSignalMapper slot for the animated icon format selection.
	QObject::connect(mapperAnimIconFormat, SIGNAL(mapped(int)),
			 q, SLOT(setAnimIconFormat_slot(int)));

	// Connect the QSignalMapper slot for translations.
	QObject::connect(mapperTS, SIGNAL(mapped(QString)),
			 q, SLOT(setTranslation_slot(QString)));

	// Configuration signals.
	cfg->registerChangeNotification(QLatin1String("preferredRegion"),
			q, SLOT(setPreferredRegion_slot(QVariant)));
	cfg->registerChangeNotification(QLatin1String("searchUsedBlocks"),
			q, SLOT(searchUsedBlocks_cfg_slot(QVariant)));
	cfg->registerChangeNotification(QLatin1String("animIconFormat"),
			 q, SLOT(setAnimIconFormat_cfg_slot(QVariant)));
	cfg->registerChangeNotification(QLatin1String("language"),
			q, SLOT(setTranslation_cfg_slot(QVariant)));
}

McRecoverWindowPrivate::~McRecoverWindowPrivate()
{
	// Save the configuration.
	cfg->save();

	// NOTE: Delete the MemCardModel first to prevent issues later.
	delete model;
	delete card;

	// TODO: Wait for searchThread to finish?
	delete searchThread;
}

/**
 * Show a warning if the card (or files on the card) are Japanese,
 * but PCRE doesn't have Unicode character properties support.
 */
void McRecoverWindowPrivate::showJpWarning(void)
{
	if (!PcreRegex::PCRE_has_UCP()) {
		// FIXME: MessageWidget text cannot be retranslated while it's still visible.
		// FIXME: Mac OS X will use a dylib in the application framework.
#ifdef PCRE_STATIC
		//: Statically-linked PCRE is missing Unicode character properties support.
		QString msg = McRecoverWindow::tr(
				"The internal PCRE library was not compiled with Unicode character properties support.\n"
				"Some files with Japanese descriptions might not be found when scanning.");
#else /* !PCRE_STATIC */
		//: Dynamically-linked PCRE is missing Unicode character properties support.
		QString msg = McRecoverWindow::tr(
				"The system PCRE library was not compiled with Unicode character properties support.\n"
				"Some files with Japanese descriptions might not be found when scanning.");
#endif /* PCRE_STATIC */
		ui.msgWidget->showMessage(msg, MessageWidget::ICON_WARNING);
	}
}

/**
 * Update the memory card's QTreeView.
 */
void McRecoverWindowPrivate::updateLstFileList(void)
{
	if (!card) {
		// Set the group box's title.
		ui.grpFileList->setTitle(McRecoverWindow::tr("No memory card loaded."));
	} else {
		// Show the filename.
		ui.grpFileList->setTitle(displayFilename);
	}

	// Show the QTreeView headers if a memory card is loaded.
	ui.lstFileList->setHeaderHidden(!card);

	// Update the action enable status.
	updateActionEnableStatus();

	// Resize the columns to fit the contents.
	int num_sections = model->columnCount();
	for (int i = 0; i < num_sections; i++)
		ui.lstFileList->resizeColumnToContents(i);
	ui.lstFileList->resizeColumnToContents(num_sections);
}

/**
 * Initialize the toolbar.
 */
void McRecoverWindowPrivate::initToolbar(void)
{
	// Set action icons.
	ui.actionOpen->setIcon(
		McRecoverQApplication::IconFromTheme(QLatin1String("document-open")));
	ui.actionClose->setIcon(
		McRecoverQApplication::IconFromTheme(QLatin1String("document-close")));
	ui.actionScan->setIcon(
		McRecoverQApplication::IconFromTheme(QLatin1String("edit-find")));
	ui.actionSave->setIcon(
		McRecoverQApplication::IconFromTheme(QLatin1String("document-save")));
	ui.actionSaveAll->setIcon(
		McRecoverQApplication::IconFromTheme(QLatin1String("document-save-all")));
	ui.actionExit->setIcon(
		McRecoverQApplication::IconFromTheme(QLatin1String("application-exit")));
	ui.actionAbout->setIcon(
		McRecoverQApplication::IconFromTheme(QLatin1String("help-about")));

	// Disable save actions by default.
	ui.actionSave->setEnabled(false);
	ui.actionSaveAll->setEnabled(false);

	// Add a label for the "Preferred region" buttons.
	lblPreferredRegion = new QLabel();
	ui.toolBar->insertWidget(ui.actionRegionUSA, lblPreferredRegion);

	// Set up the QActionGroup for the "Preferred region" buttons.
	actgrpRegion->addAction(ui.actionRegionUSA);
	actgrpRegion->addAction(ui.actionRegionPAL);
	actgrpRegion->addAction(ui.actionRegionJPN);
	actgrpRegion->addAction(ui.actionRegionKOR);

	// Connect QAction signals to the QSignalMapper.
	QObject::connect(ui.actionRegionUSA, SIGNAL(triggered()),
			 mapperPreferredRegion, SLOT(map()));
	QObject::connect(ui.actionRegionPAL, SIGNAL(triggered()),
			 mapperPreferredRegion, SLOT(map()));
	QObject::connect(ui.actionRegionJPN, SIGNAL(triggered()),
			 mapperPreferredRegion, SLOT(map()));
	QObject::connect(ui.actionRegionKOR, SIGNAL(triggered()),
			 mapperPreferredRegion, SLOT(map()));

	// Set the mappings in the QSignalMapper.
	mapperPreferredRegion->setMapping(ui.actionRegionUSA, 'E');
	mapperPreferredRegion->setMapping(ui.actionRegionPAL, 'P');
	mapperPreferredRegion->setMapping(ui.actionRegionJPN, 'J');
	mapperPreferredRegion->setMapping(ui.actionRegionKOR, 'K');

	// Set up the QActionGroup for the "Animated Icon Format" options.
	// Indexes correspond to GcImageWriter::AnimImageFormat enum values.
	QAction *const animImgfActions[] = {
		ui.actionAnimAPNG,
		ui.actionAnimGIF,
		ui.actionAnimPNGfpf,
		ui.actionAnimPNGvs,
		ui.actionAnimPNGhs,
	};

	// Initial setting will be set by a ConfigStore notification.
	for (int i = 0; i < ARRAY_SIZE(animImgfActions); i++) {
		actgrpAnimIconFormat->addAction(animImgfActions[i]);

		// Is this animated image format available?
		if (GcImageWriter::isAnimImageFormatSupported(
			(GcImageWriter::AnimImageFormat)(i+1)))
		{
			// Image format is available.
			QObject::connect(animImgfActions[i], SIGNAL(triggered()),
					 mapperAnimIconFormat, SLOT(map()));
			mapperAnimIconFormat->setMapping(animImgfActions[i], (i+1));
		} else {
			// Image format is not available.
			// Disable the action.
			animImgfActions[i]->setEnabled(false);
		}
	}

	// Make sure the "About" button is right-aligned.
	Q_Q(McRecoverWindow);
	QWidget *spacer = new QWidget(q);
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	ui.toolBar->insertWidget(ui.actionAbout, spacer);

	// Retranslate the toolbar.
	retranslateToolbar();
}

/**
 * Retranslate the toolbar.
 */
void McRecoverWindowPrivate::retranslateToolbar(void)
{
	// TODO: Better way to add space other than, well, adding space.
	lblPreferredRegion->setText(QChar(L' ') +
		McRecoverWindow::tr("Preferred Region:") + QChar(L' '));
}

/**
 * Update the "enabled" status of the QActions.
 */
void McRecoverWindowPrivate::updateActionEnableStatus(void)
{
	if (!card) {
		// No memory card image is loaded.
		ui.actionClose->setEnabled(false);
		ui.actionScan->setEnabled(false);
		ui.actionSave->setEnabled(false);
		ui.actionSaveAll->setEnabled(false);
	} else {
		// Memory card image is loaded.
		// TODO: Disable open, scan, and save (all) if we're scanning.
		ui.actionClose->setEnabled(true);
		if (!scanningDisabled)
			ui.actionScan->setEnabled(true);
		ui.actionSave->setEnabled(
			ui.lstFileList->selectionModel()->hasSelection());
		ui.actionSaveAll->setEnabled(card->numFiles() > 0);
	}
}

/**
 * Update the window title.
 */
void McRecoverWindowPrivate::updateWindowTitle(void)
{
	QString windowTitle;
	if (card) {
		windowTitle += displayFilename;
		windowTitle += QLatin1String(" - ");
	}
	windowTitle += QApplication::applicationName();

	Q_Q(McRecoverWindow);
	q->setWindowTitle(windowTitle);
}

/**
 * Change the file extension of the specified file.
 * @param filename Filename.
 * @param newExt New extension, including leading dot.
 * @return Filename with new extension.
 */
QString McRecoverWindowPrivate::changeFileExtension(const QString &filename, const QString &newExt)
{
	int dotPos = filename.lastIndexOf(QChar(L'.'));
	int slashPos = filename.lastIndexOf(QChar(L'/'));
	if (dotPos > 0 && dotPos > slashPos) {
		// Found a file extension dot.
		QString newFilename = filename.left(dotPos);
		newFilename += newExt;
		return newFilename;
	}

	// No extension found.
	// Append the new extension instead.
	return (filename + newExt);
}

/**
 * Save the specified file(s).
 * @param files List of file(s) to save.
 * @param path If specified, save file(s) to path using default GCI filenames.
 */
void McRecoverWindowPrivate::saveFiles(const QVector<MemCardFile*> &files, QString path)
{
	Q_Q(McRecoverWindow);

	if (files.isEmpty())
		return;

	const bool extractBanners = ui.actionExtractBanners->isChecked();
	const bool extractIcons = ui.actionExtractIcons->isChecked();

	bool singleFile = false;
	QString filename;

	QString extBanner, extIcon;
	if (extractBanners || extractIcons) {
		extBanner = QLatin1String(".banner");
		extIcon = QLatin1String(".icon");
	}

	// Save files using default filenames to the specified path.
	enum OverwriteAllStatus {
		OVERWRITEALL_UNKNOWN	= 0,
		OVERWRITEALL_YESTOALL	= 1,
		OVERWRITEALL_NOTOALL	= 2,
	};

	int filesSaved = 0;
	OverwriteAllStatus overwriteAll = OVERWRITEALL_UNKNOWN;

	if (files.size() == 1 && path.isEmpty()) {
		// Single file, path not specified.
		singleFile = true;
		overwriteAll = OVERWRITEALL_YESTOALL;
		MemCardFile *file = files.at(0);

		const QString defFilename = lastPath() + QChar(L'/') +
						file->defaultGciFilename();

		// Prompt the user for a save location.
		filename = QFileDialog::getSaveFileName(q,
				McRecoverWindow::tr("Save GCN Save File %1")
					.arg(file->filename()),	// Dialog title
				defFilename,			// Default filename
				McRecoverWindow::tr("GameCube Save Files") + QLatin1String(" (*.gci);;") +
				McRecoverWindow::tr("All Files") + QLatin1String(" (*)"));
		if (filename.isEmpty())
			return;

		// Set the last path.
		setLastPath(filename);
	} else if (files.size() > 1 && path.isEmpty()) {
		// Multiple files, path not specified.
		// Prompt the user for a save location.
		path = QFileDialog::getExistingDirectory(q,
				McRecoverWindow::tr("Save %Ln GCN Save File(s)", "", files.size()),
				lastPath());
		if (path.isEmpty())
			return;

		// Set the last path.
		setLastPath(path);
	}

	// Animted image format for icons.
	GcImageWriter::AnimImageFormat animImgf = animIconFormat();

	foreach (MemCardFile *file, files) {
		if (!singleFile)
			filename = path + QChar(L'/') + file->defaultGciFilename();
		QFile qfile(filename);

		// Check if the file exists.
		if (qfile.exists()) {
			if (overwriteAll == OVERWRITEALL_UNKNOWN) {
				bool overwrite = false;
				int ret = QMessageBox::warning(q,
					McRecoverWindow::tr("File Already Exists"),
					McRecoverWindow::tr("A file named \"%1\" already exists in the specified directory.\n\n"
							    "Do you want to overwrite it?").arg(filename),
					(QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll),
					QMessageBox::No);
				switch (ret) {
					case QMessageBox::Yes:
						// Overwrite this file.
						overwrite = true;
						break;

					default:
					case QMessageBox::No:
					case QMessageBox::Escape:
						// Don't overwrite this file.
						overwrite = false;
						break;

					case QMessageBox::YesToAll:
						// Overwrite this file and all other files.
						overwriteAll = OVERWRITEALL_YESTOALL;
						overwrite = true;
						break;

					case QMessageBox::NoToAll:
						// Don't overwrite this file or any other files.
						overwriteAll = OVERWRITEALL_NOTOALL;
						overwrite = false;
						break;
				}

				if (!overwrite)
					continue;
			} else if (overwriteAll == OVERWRITEALL_NOTOALL) {
				// Don't overwrite any files.
				continue;
			}
		}

		// Save the file.
		int ret = file->saveGci(filename);
		if (ret == 0) {
			// File saved successfully.
			filesSaved++;
		} else {
			// An error occurred while saving the file.
			// TODO: Error details.
		}

		// Extract the banner.
		if (extractBanners) {
			// TODO: Error handling and details.
			QString bannerFilename = changeFileExtension(filename, extBanner);
			file->saveBanner(bannerFilename);
		}

		// Extract the icon.
		if (extractIcons) {
			// TODO: Error handling and details.
			if (file->numIcons() >= 1) {
				// File has an icon.
				QString iconFilename = changeFileExtension(filename, extIcon);
				file->saveIcon(iconFilename, animImgf);
			}
		}
	}

	// Update the status bar.
	QDir dir;
	if (singleFile) {
		QFileInfo fileInfo(filename);
		dir = fileInfo.dir();
	} else {
		dir = QDir(path);
	}
	QString absolutePath = dir.absolutePath();

	// Make sure tha path has a trailing slash.
	if (!absolutePath.isEmpty() &&
		absolutePath.at(absolutePath.size() - 1) != QChar(L'/'))
	{
		absolutePath += QChar(L'/');
	}

	statusBarManager->filesSaved(filesSaved, absolutePath);
}

/**
 * Get an icon for a given locale.
 * @param locale Locale name, e.g. "en_US".
 * @return Icon, or null QIcon if not found.
 */
QIcon McRecoverWindowPrivate::IconForLocale(const QString &locale)
{
	// Check for an icon.
	// Check region, then language.
	QStringList tsParts = locale.split(QChar(L'_'), QString::SkipEmptyParts);
	QIcon flagIcon;
	for (int i = (tsParts.size() - 1); i >= 0; i--) {
		QString filename = QLatin1String(":/flags/flag-") +
				   tsParts.at(i).toLower() +
				   QLatin1String(".png");
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
void McRecoverWindowPrivate::rets_actTsSysDefault(void)
{
	if (!actTsSysDefault) {
		Q_Q(McRecoverWindow);
		actTsSysDefault = new QAction(q);
		actTsSysDefault->setCheckable(true);
		QObject::connect(actTsSysDefault, SIGNAL(triggered()),
				 mapperTS, SLOT(map()));
	}

	QString tsLocale = QLocale::system().name();

	//: Translation: System Default (retrieved from system settings)
	actTsSysDefault->setText(
		McRecoverWindow::tr("System Default (%1)", "ts-language")
				.arg(QLocale::system().name()));
	mapperTS->setMapping(actTsSysDefault, QString());

	// Check for an icon.
	QIcon flagIcon = IconForLocale(tsLocale);
	if (!flagIcon.isNull())
		actTsSysDefault->setIcon(flagIcon);
}

/**
 * Initialize the Translations menu.
 */
void McRecoverWindowPrivate::initTsMenu(void)
{
	Q_Q(McRecoverWindow);

	// Clear the Translations menu first.
	ui.menuLanguage->clear();
	if (actgrpTS)
		delete actgrpTS;
	qDeleteAll(hashActionsTS);
	hashActionsTS.clear();
	actgrpTS = new QActionGroup(q);

	// Add the system default translation.
	rets_actTsSysDefault();
	actgrpTS->addAction(actTsSysDefault);
	ui.menuLanguage->addAction(actTsSysDefault);

	// Add all other translations.
	ui.menuLanguage->addSeparator();
	QMap<QString, QString> tsMap = TranslationManager::instance()->enumerate();
	hashActionsTS.reserve(tsMap.size());
	foreach (QString tsLocale, tsMap.keys()) {
		QString tsLanguage = tsMap.value(tsLocale);
		QAction *actTs = new QAction(tsLanguage, q);
		actTs->setCheckable(true);

		// Check for an icon.
		QIcon flagIcon = IconForLocale(tsLocale);
		if (!flagIcon.isNull())
			actTs->setIcon(flagIcon);

		hashActionsTS.insert(tsLocale, actTs);
		actgrpTS->addAction(actTs);
		QObject::connect(actTs, SIGNAL(triggered()),
				 mapperTS, SLOT(map()));
		mapperTS->setMapping(actTs, tsLocale);
		ui.menuLanguage->addAction(actTs);
	}

	// Initial language is set by a ConfigStore notification,
	// connected to setTranslation_cfg_slot().
}

/**
 * Get the last path.
 * @return Last path.
 */
QString McRecoverWindowPrivate::lastPath(void) const
{
	// TODO: Cache the last absolute path?

	// NOTE: Path is stored using native separators.
	QString path = QDir::fromNativeSeparators(
		cfg->get(QLatin1String("lastPath")).toString());

	// If this is a relative path, convert to absolute.
	if (path.startsWith(QLatin1String("./"))) {
		// Path is relative to the application directory.
		QDir applicationDir(QCoreApplication::applicationDirPath());
		path.remove(0, 2);
		path = applicationDir.absoluteFilePath(path);
	} else if (path.startsWith(QLatin1String("~/"))) {
		// Path is relative to the user's home directory.
		path.remove(0, 2);
		path = QDir::home().absoluteFilePath(path);
	}

	return path;
}

/**
 * Set the last path.
 * @param path Last path.
 */
void McRecoverWindowPrivate::setLastPath(const QString &path)
{
	// TODO: Relative to application directory on Windows?
	QFileInfo fileInfo(path);
	QString lastPath;
	if (fileInfo.isDir())
		lastPath = path;
	else
		lastPath = fileInfo.dir().absolutePath();

	// Make this path relative to the application directory.
	lastPath = PathFuncs::makeRelativeToApplication(lastPath);
#ifndef Q_OS_WIN
	// Make this path relative to the user's home directory.
	lastPath = PathFuncs::makeRelativeToHome(lastPath);
#endif /* !Q_OS_WIN */

	cfg->set(QLatin1String("lastPath"),
		 QDir::toNativeSeparators(lastPath));
}

/**
 * Get the animated icon format to use.
 * @return Animated icon format to use.
 */
GcImageWriter::AnimImageFormat McRecoverWindowPrivate::animIconFormat(void) const
{
	// TODO: Cache the last animated icon format?

	QString fmt = cfg->get(QLatin1String("animIconFormat")).toString();
	GcImageWriter::AnimImageFormat animImgf =
		GcImageWriter::animImageFormatFromName(fmt.toLatin1().constData());
	if (!GcImageWriter::isAnimImageFormatSupported(animImgf)) {
		// Format is not supported.
		animImgf = GcImageWriter::ANIMGF_UNKNOWN;
	}

	if (animImgf == GcImageWriter::ANIMGF_UNKNOWN) {
		// Determine which format to use.
		for (int i = 1; i < GcImageWriter::ANIMGF_MAX; i++) {
			// Is this animated image format available?
			if (GcImageWriter::isAnimImageFormatSupported(
				(GcImageWriter::AnimImageFormat)i))
			{
				// Image format is available.
				animImgf = (GcImageWriter::AnimImageFormat)i;
				break;
			}
		}
	}

	return animImgf;
}

/** McRecoverWindow **/

McRecoverWindow::McRecoverWindow(QWidget *parent)
	: QMainWindow(parent)
	, d_ptr(new McRecoverWindowPrivate(this))
{
	Q_D(McRecoverWindow);
	d->ui.setupUi(this);

	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);

#ifdef Q_OS_MAC
	// Remove the window icon. (Mac "proxy icon")
	// TODO: Use the memory card file?
	this->setWindowIcon(QIcon());
#endif /* Q_OS_MAC */

#ifdef Q_OS_WIN
	// Hide the QMenuBar border on Win32.
	// FIXME: This causes the menu bar to be "truncated" when using
	// the Aero theme on Windows Vista and 7.
#if 0
	this->Ui_McRecoverWindow::menuBar->setStyleSheet(
		QLatin1String("QMenuBar { border: none }"));
#endif
#endif

	// Set up the QSplitter sizes.
	// We want the card info panel to be 160px wide at startup.
	// TODO: Save positioning settings somewhere?
	static const int MemCardInfoPanelWidth = 256;
	QList<int> sizes;
	sizes.append(this->width() - MemCardInfoPanelWidth);
	sizes.append(MemCardInfoPanelWidth);
	d->ui.splitter->setSizes(sizes);

	// Set the splitter stretch factors.
	// We want the QTreeView to stretch, but not the card info panel.
	d->ui.splitter->setStretchFactor(0, 1);
	d->ui.splitter->setStretchFactor(1, 0);

	// Initialize lstFileList's item delegate.
	d->ui.lstFileList->setItemDelegate(new MemCardItemDelegate(this));

	// Set the models.
	d->proxyModel->setSourceModel(d->model);
	d->ui.lstFileList->setModel(d->proxyModel);

	// Don't expand the last header column to fill the QTreeView.
	QHeaderView *header = d->ui.lstFileList->header();
	header->setStretchLastSection(false);

	// Sort by COL_DESCRIPTION by default.
	// TODO: Disable sorting on specific columns.
	//d->proxyModel->setDynamicSortFilter(true);
	//d->ui.lstFileList->sortByColumn(MemCardModel::COL_DESCRIPTION, Qt::AscendingOrder);

	// Show icon, description, size, mtime, permission, and gamecode by default.
	// TODO: Allow the user to customize the columns, and save the 
	// customized columns somewhere.
	d->ui.lstFileList->setColumnHidden(MemCardModel::COL_ICON, false);
	d->ui.lstFileList->setColumnHidden(MemCardModel::COL_BANNER, true);
	d->ui.lstFileList->setColumnHidden(MemCardModel::COL_DESCRIPTION, false);
	d->ui.lstFileList->setColumnHidden(MemCardModel::COL_SIZE, false);
	d->ui.lstFileList->setColumnHidden(MemCardModel::COL_MTIME, false);
	d->ui.lstFileList->setColumnHidden(MemCardModel::COL_PERMISSION, false);
	d->ui.lstFileList->setColumnHidden(MemCardModel::COL_GAMEID, false);
	d->ui.lstFileList->setColumnHidden(MemCardModel::COL_FILENAME, true);

	// Connect the lstFileList slots.
	connect(d->ui.lstFileList->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
		this, SLOT(lstFileList_selectionModel_currentRowChanged(QModelIndex,QModelIndex)));

	// Initialize the UI.
	d->updateLstFileList();
	d->initToolbar();
	d->initTsMenu();
	d->statusBarManager = new StatusBarManager(d->ui.statusBar, this);
	d->updateWindowTitle();

	// Check if PCRE supports UTF-8.
	if (!PcreRegex::PCRE_has_UTF8()) {
		// UTF-8 is not supported. Disable scanning.
		d->scanningDisabled = true;
		d->ui.actionScan->setEnabled(false);

		// FIXME: MessageWidget text cannot be retranslated while it's still visible.
		// FIXME: Mac OS X will use a dylib in the application framework.
#ifdef PCRE_STATIC
		//: Statically-linked PCRE is missing UTF-8 support.
		QString msg = tr("The internal PCRE library was not compiled with UTF-8 support.\n"
				"Scanning for lost files will not work.");
#else /* !PCRE_STATIC */
		//: Dynamically-linked PCRE is missing UTF-8 support.
		QString msg = tr("The system PCRE library was not compiled with UTF-8 support.\n"
				"Scanning for lost files will not work.");
#endif /* PCRE_STATIC */
		d->ui.msgWidget->showMessage(msg, MessageWidget::ICON_CRITICAL);
	}

	// Shh... it's a secret to everybody.
	QObject::connect(d->ui.lstFileList, SIGNAL(keyPress(QKeyEvent*)),
			 d->herpDerp, SLOT(widget_keyPress(QKeyEvent*)));
	QObject::connect(d->ui.lstFileList, SIGNAL(focusOut(QFocusEvent*)),
			 d->herpDerp, SLOT(widget_focusOut(QFocusEvent*)));

	// Emit all configuration signals.
	d->cfg->notifyAll();
}

McRecoverWindow::~McRecoverWindow()
{
	delete d_ptr;
}

/**
 * Open a GameCube Memory Card image.
 * @param filename Filename.
 */
void McRecoverWindow::openCard(const QString &filename)
{
	Q_D(McRecoverWindow);

	if (d->card) {
		d->model->setMemCard(nullptr);
		d->ui.mcCardView->setCard(nullptr);
		d->ui.mcfFileView->setFile(nullptr);
		delete d->card;
	}

	// Open the specified memory card image.
	// TODO: Set this as the last path?
	d->card = new MemCard(filename);
	if (!d->card->isOpen()) {
		// Could not open the card.
		static const QChar chrBullet(0x2022);  // U+2022: BULLET
		QString filename_noPath = QFileInfo(filename).fileName();
		QString errMsg = tr("An error occurred while opening %1:")
					.arg(filename_noPath) +
				QChar(L'\n') + chrBullet + QChar(L' ') +
				d->card->errorString();
		d->ui.msgWidget->showMessage(errMsg, MessageWidget::ICON_WARNING);
		closeCard(true);
		return;
	}

	d->filename = filename;
	d->model->setMemCard(d->card);

	// Extract the filename from the path.
	d->displayFilename = filename;
	int lastSlash = d->displayFilename.lastIndexOf(QChar(L'/'));
	if (lastSlash >= 0)
		d->displayFilename.remove(0, lastSlash + 1);

	// Set the MemCardView's MemCard to the
	// selected card in the QTreeView.
	d->ui.mcCardView->setCard(d->card);

	// Check if any of the files are Japanese.
	bool isJapanese = false;
	if (d->card->encoding() == SYS_FONT_ENCODING_SJIS) {
		isJapanese = true;
	} else {
		for (int i = 0; i < d->card->numFiles(); i++) {
			const MemCardFile *file = d->card->getFile(i);
			if (file->encoding() == SYS_FONT_ENCODING_SJIS) {
				isJapanese = true;
				break;
			}
		}
	}

	// If the card encoding or any files are Japanese,
	// show a warning if PCRE doesn't support UCP.
	if (isJapanese)
		d->showJpWarning();

	// Check for other card errors.
	// NOTE: These aren't retranslated if the UI is retranslated.
	QStringList sl_cardErrors;
	QFlags<MemCard::Error> cardErrors = d->card->errors();
	if (cardErrors & MemCard::MCE_SZ_TOO_SMALL) {
		sl_cardErrors +=
			tr("The card image is too small. (Card image is %L1 bytes; should be at least 512 KB.)")
			.arg(d->card->filesize());
	}
	if (cardErrors & MemCard::MCE_SZ_TOO_BIG) {
		// TODO: Convert filesize to KB/MB/GB?
		sl_cardErrors +=
			tr("The card image is too big. (Card image is %L1 bytes; should be 16 MB or less.)")
			.arg(d->card->filesize());
	}
	if (cardErrors & MemCard::MCE_SZ_NON_POW2) {
		// TODO: Convert filesize to KB/MB/GB?
		sl_cardErrors +=
			tr("The card image size is not a power of two. (Card image is %L1 bytes.)")
			.arg(d->card->filesize());
	}
	if (cardErrors & MemCard::MCE_INVALID_HEADER) {
		sl_cardErrors += tr("The header checksum is invalid.");
	}
	if (cardErrors & MemCard::MCE_INVALID_DATS) {
		sl_cardErrors += tr("Both directory tables are invalid.");
	}
	if (cardErrors & MemCard::MCE_INVALID_BATS) {
		sl_cardErrors += tr("Both block tables are invalid.");
	}

	if (!sl_cardErrors.isEmpty()) {
		// Errors detected.
		static const QChar chrBullet(0x2022);  // U+2022: BULLET
		QString msg;
		msg.reserve(2048);
		msg += tr("Error(s) have been detected in this Memory Card image:", "", sl_cardErrors.size());
		foreach (const QString &str, sl_cardErrors) {
			msg += QChar(L'\n') + chrBullet + QChar(L' ') + str;
		}

		// Show a warning message.
		d->ui.msgWidget->showMessage(msg, MessageWidget::ICON_WARNING);
	}

	// Update the UI.
	d->updateLstFileList();
	d->statusBarManager->opened(filename);
	d->updateWindowTitle();

	// FIXME: If a file is opened from the command line,
	// QTreeView sort-of selects the first file.
	// (Signal is emitted, but nothing is highlighted.)
}

/**
 * Close the currently-opened GameCube Memory Card image.
 * @param noMsg If true, don't show a message in the status bar.
 */
void McRecoverWindow::closeCard(bool noMsg)
{
	Q_D(McRecoverWindow);

	d->model->setMemCard(nullptr);
	d->ui.mcCardView->setCard(nullptr);
	d->ui.mcfFileView->setFile(nullptr);
	delete d->card;
	d->card = nullptr;

	// Clear the filenames.
	d->filename.clear();
	d->displayFilename.clear();

	// Update the UI.
	d->updateLstFileList();
	if (!noMsg)
		d->statusBarManager->closed();
	d->updateWindowTitle();
}

/**
 * Widget state has changed.
 * @param event State change event.
 */
void McRecoverWindow::changeEvent(QEvent *event)
{
	Q_D(McRecoverWindow);

	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		d->ui.retranslateUi(this);
		d->updateLstFileList();
		d->updateWindowTitle();
		d->retranslateToolbar();
		d->rets_actTsSysDefault();
	} else if (event->type() == QEvent::LocaleChange) {
		// Locale change usually requires a UI retranslation.
		QAction *actionTS = d->actgrpTS->checkedAction();
		if (actionTS)
			actionTS->trigger();
	}

	// Pass the event to the base class.
	this->QMainWindow::changeEvent(event);
}

/**
 * An item is being dragged onto the window.
 * @param event QDragEnterEvent describing the item.
 */
void McRecoverWindow::dragEnterEvent(QDragEnterEvent *event)
{
	if (!event->mimeData()->hasUrls())
		return;

	// One or more URls have been dragged onto the window.
	const QList<QUrl>& lstUrls = event->mimeData()->urls();
	if (lstUrls.size() != 1) {
		// More than one file has been dragged onto the window.
		// TODO: Add support for this; open one window per file.
		return;
	}

	// One URL has been dragged onto the window.
	const QUrl& url = lstUrls.at(0);

	// Make sure the URL is file://.
	// TODO: Add support for other protocols later.
	if (url.scheme() != QLatin1String("file"))
		return;

	// Override the proposed action with Copy, and accept it.
	event->setDropAction(Qt::CopyAction);
	event->accept();
}

/**
 * An item has been dropped onto the window.
 * @param event QDropEvent describing the item.
 */
void McRecoverWindow::dropEvent(QDropEvent *event)
{
	if (!event->mimeData()->hasUrls())
		return;

	// One or more URls have been dragged onto the window.
	const QList<QUrl>& lstUrls = event->mimeData()->urls();
	if (lstUrls.size() != 1) {
		// More than one file has been dropped onto the window.
		// TODO: Add support for this; open one window per file.
		return;
	}

	// One URL has been dropped onto the window.
	const QUrl& url = lstUrls.at(0);
	
	// Make sure the URL is file://.
	// TODO: Add support for other protocols later.
	if (url.scheme() != QLatin1String("file"))
		return;

	// Get the local filename.
	// NOTE: url.toLocalFile() returns an empty string if this isn't file://,
	// but we're already checking for file:// above...
	QString filename = url.toLocalFile();
	if (filename.isEmpty())
		return;

	// Override the proposed action with Copy, and accept it.
	event->setDropAction(Qt::CopyAction);
	event->accept();

	// Open the memory card file.
	openCard(filename);
}

/**
 * Mark the UI as busy.
 * Calls to this function stack, so if markUiBusy()
 * was called 3 times, markUiNotBusy() must be called 3 times.
 */
void McRecoverWindow::markUiBusy(void)
{
	Q_D(McRecoverWindow);
	d->uiBusyCounter++;
	if (d->uiBusyCounter == 1) {
		// UI is now busy.
		this->setCursor(Qt::WaitCursor);
		d->ui.menuBar->setEnabled(false);
		d->ui.toolBar->setEnabled(false);
		this->centralWidget()->setEnabled(false);
	}
}

/**
 * Mark the UI as not busy.
 * Calls to this function stack, so if markUiBusy()
 * was called 3 times, markUiNotBusy() must be called 3 times.
 */
void McRecoverWindow::markUiNotBusy(void)
{
	Q_D(McRecoverWindow);
	if (d->uiBusyCounter <= 0) {
		// We're already not busy.
		// Don't decrement the counter, though.
		return;
	}

	d->uiBusyCounter--;
	if (d->uiBusyCounter == 0) {
		// UI is no longer busy.
		d->ui.menuBar->setEnabled(true);
		d->ui.toolBar->setEnabled(true);
		this->centralWidget()->setEnabled(true);
		this->unsetCursor();
	}
}

/** UI widget slots. **/

/**
 * Open a memory card image.
 */
void McRecoverWindow::on_actionOpen_triggered(void)
{
	Q_D(McRecoverWindow);
	QString filename = QFileDialog::getOpenFileName(this,
			tr("Open GameCube Memory Card Image"),	// Dialog title
			d->lastPath(),				// Default filename
			tr("GameCube Memory Card Image") + QLatin1String(" (*.raw);;") +
			tr("All Files") + QLatin1String(" (*)"));

	if (filename.isEmpty())
		return;

	// Set the last path.
	d->setLastPath(filename);

	// Open the memory card file.
	openCard(filename);
}

/**
 * Close the currently-opened memory card image.
 */
void McRecoverWindow::on_actionClose_triggered(void)
{
	Q_D(McRecoverWindow);
	if (!d->card)
		return;

	closeCard();
}

/**
 * Scan for lost files.
 */
void McRecoverWindow::on_actionScan_triggered(void)
{
	Q_D(McRecoverWindow);
	if (!d->card || d->scanningDisabled)
		return;

	// Get the database filenames.
	QVector<QString> dbFilenames = GcnMcFileDb::GetDbFilenames();
	if (dbFilenames.isEmpty()) {
#ifdef Q_OS_WIN
		QString def_path_hint = tr(
			"The database files should be located in the data subdirectory in\n"
			"mcrecover.exe's program directory.");
#else
		QString def_path_hint = tr(
			"The database files should be located in %1.\n"
			"Alternatively, you can place your own version in ~/.config/mcrecover/data/")
			.arg(QLatin1String(MCRECOVER_DATA_DIRECTORY));
#endif

		QMessageBox::critical(this,
			tr("Database Load Error"),
			tr("No GCN MemCard file databases were found.") +
			QLatin1String("\n\n") + def_path_hint);
		return;
	}

	// Load the databases.
	// TODO: Optimize database loading:
	// - Only if the database is not loaded,
	//   or if the database file has been changed.
	int ret = d->searchThread->loadGcnMcFileDbs(dbFilenames);
	if (ret != 0)
		return;

	// Remove lost files from the card.
	d->card->removeLostFiles();

	// Update the status bar manager.
	d->statusBarManager->setSearchThread(d->searchThread);

	// Should we search used blocks?
	const bool searchUsedBlocks = d->ui.actionSearchUsedBlocks->isChecked();
	if (!searchUsedBlocks && d->card->freeBlocks() <= 0) {
		// TODO: Print a message in the status bar.
		// For now, the search thread will simply indicate
		// that the search has been cancelled.
	}

	// Search blocks for lost files.
	// TODO: Handle errors.
	ret = d->searchThread->searchMemCard_async(d->card, d->preferredRegion, searchUsedBlocks);
	if (ret < 0) {
		// Error starting the thread.
		// Use the synchronous version.
		// TODO: Handle errors.
		// NOTE: Files will be added by searchThread_searchFinished_slot().
		ret = d->searchThread->searchMemCard(d->card, d->preferredRegion, searchUsedBlocks);
	}
}

/**
 * Exit the program.
 * TODO: Separate close/exit for Mac OS X?
 */
void McRecoverWindow::on_actionExit_triggered(void)
{
	this->closeCard();
	this->close();
}

/**
 * Show the About dialog.
 */
void McRecoverWindow::on_actionAbout_triggered(void)
{
	AboutDialog::ShowSingle(this);
}

/** Save actions. **/

/**
 * Save the selected file(s).
 */
void McRecoverWindow::on_actionSave_triggered(void)
{
	Q_D(McRecoverWindow);
	QModelIndexList selList = d->ui.lstFileList->selectionModel()->selectedRows();
	if (selList.isEmpty())
		return;

	QVector<MemCardFile*> files;
	files.reserve(selList.size());

	foreach(QModelIndex idx, selList) {
		QModelIndex srcIdx = d->proxyModel->mapToSource(idx);
		MemCardFile *file = d->card->getFile(srcIdx.row());
		if (file != nullptr)
			files.append(file);
	}

	// If there's no files to save, don't do anything.
	if (files.isEmpty())
		return;

	// Save the files.
	d->saveFiles(files);
}

/**
 * Save all files.
 */
void McRecoverWindow::on_actionSaveAll_triggered(void)
{
	Q_D(McRecoverWindow);
	if (!d->card)
		return;

	const int numFiles = d->card->numFiles();
	if (numFiles <= 0)
		return;

	QVector<MemCardFile*> files;
	files.reserve(numFiles);

	for (int i = 0; i < numFiles; i++) {
		MemCardFile *file = d->card->getFile(i);
		if (file != nullptr)
			files.append(file);
	}

	// If there's no files to save, don't do anything.
	if (files.isEmpty())
		return;

	// Save the files.
	d->saveFiles(files);
}

/**
 * Set the preferred region.
 * This slot is triggered by a QSignalMapper that
 * maps the various QActions.
 * @param preferredRegion Preferred region. (actually char)
 */
void McRecoverWindow::setPreferredRegion_slot(int preferredRegion)
{
	Q_D(McRecoverWindow);
	d->preferredRegion = static_cast<char>(preferredRegion);

	/**
	 * Save the preferred region in the configuration.
	 *
	 * NOTE: Saving a QChar results in a wacky entry:
	 * - preferredRegion=@Variant(\0\0\0\a\0E)
	 * Convert it to QString to avoid this problem.
	 */
	QString str = QChar((uint16_t)preferredRegion);
	d->cfg->set(QLatin1String("preferredRegion"), str);
}

/**
 * Set the preferred region.
 * This version is used by ConfigDefaults.
 * @param preferredRegion Preferred region. (actually char)
 */
void McRecoverWindow::setPreferredRegion_slot(const QVariant &preferredRegion)
{
	uint8_t chr = 0;
	if (preferredRegion.canConvert(QVariant::String)) {
		QString str = preferredRegion.toString();
		if (str.size() == 1)
			chr = (uint8_t)str.at(0).unicode();
	} else if (preferredRegion.canConvert(QVariant::Char)) {
		chr = (uint8_t)preferredRegion.toChar().unicode();
	}

	Q_D(McRecoverWindow);
	switch (chr) {
		case 'E':
			d->ui.actionRegionUSA->setChecked(true);
			break;
		case 'P':
			d->ui.actionRegionPAL->setChecked(true);
			break;
		case 'J':
			d->ui.actionRegionJPN->setChecked(true);
			break;
		case 'K':
			d->ui.actionRegionKOR->setChecked(true);
			break;
		default:
			// TODO: Determine based on system locale.
			chr = 'E';
			d->ui.actionRegionUSA->setChecked(true);
			break;
	}

	// NOTE: If region is 'E', then preferredRegion won't be set,
	// since actionRegionUSA is checked by default.
	// Set it here to make sure it's set properly.
	d->preferredRegion = chr;
}

void McRecoverWindow::memCardModel_layoutChanged(void)
{
	// Update the QTreeView columns, etc.
	// FIXME: This doesn't work the first time a file is added...
	// (possibly needs a dataChanged() signal)
	Q_D(McRecoverWindow);
	d->updateLstFileList();
}

void McRecoverWindow::memCardModel_rowsInserted(void)
{
	// A new file entry was added to the MemCard.
	// Update the QTreeView columns.
	// FIXME: This doesn't work the first time a file is added...
	Q_D(McRecoverWindow);
	d->updateLstFileList();
}

/**
 * Search has completed.
 * @param lostFilesFound Number of "lost" files found.
 */
void McRecoverWindow::searchThread_searchFinished_slot(int lostFilesFound)
{
	Q_UNUSED(lostFilesFound)
	Q_D(McRecoverWindow);

	// Remove lost files from the card.
	d->card->removeLostFiles();

	// Get the files found list.
	QLinkedList<SearchData> filesFoundList = d->searchThread->filesFoundList();

	// Add the directory entries.
	bool isJapanese = false;
	foreach (const SearchData &searchData, filesFoundList) {
		MemCardFile *file = d->card->addLostFile(&(searchData.dirEntry), searchData.fatEntries);
		if (file) {
			if (file->encoding() == SYS_FONT_ENCODING_SJIS)
				isJapanese = true;

			// TODO: Add ChecksumData parameter to addLostFile.
			// Alternatively, add SearchData overload?
			if (file)
				file->setChecksumDefs(searchData.checksumDefs);
		}
	}

	// If the card encoding or any files are Japanese,
	// show a warning if PCRE doesn't support UCP.
	if (isJapanese || d->card->encoding() == SYS_FONT_ENCODING_SJIS)
		d->showJpWarning();
}

/**
 * lstFileList selectionModel: Current row selection has changed.
 * @param current Current index.
 * @param previous Previous index.
 */
void McRecoverWindow::lstFileList_selectionModel_currentRowChanged(
	const QModelIndex& current, const QModelIndex& previous)
{
	Q_UNUSED(previous)
	Q_D(McRecoverWindow);

	QModelIndex srcCurrent = d->proxyModel->mapToSource(current);

	// If file(s) are selected, enable the Save action.
	// FIXME: Selection model is empty due to the initial selection bug
	// that happens if a file was specified on the command line.
	//actionSave->setEnabled(lstFileList->selectionModel()->hasSelection());
	d->ui.actionSave->setEnabled(srcCurrent.row() >= 0);

	// Set the MemCardFileView's MemCardFile to the
	// selected file in the QTreeView.
	const MemCardFile *file = d->card->getFile(srcCurrent.row());
	d->ui.mcfFileView->setFile(file);

	// Shh... it's a secret to everybody.
	d->herpDerp->setSelGameID(file ? file->gameID() : QString());
}

/**
 * Animated icon format was changed by the user.
 * @param animIconFormat Animated icon format.
 */
void McRecoverWindow::setAnimIconFormat_slot(int animIconFormat)
{
	const char *fmt = GcImageWriter::nameOfAnimImageFormat(
				(GcImageWriter::AnimImageFormat)animIconFormat);
	QString s_fmt = (fmt ? QLatin1String(fmt) : QString());

	Q_D(McRecoverWindow);
	// d->cfg->set() will trigger a notification.
	d->cfg->set(QLatin1String("animIconFormat"), s_fmt);
}

/**
 * UI language was changed by the configuration.
 * @param animIconFormat Animated icon format.
 */
void McRecoverWindow::setAnimIconFormat_cfg_slot(const QVariant &animIconFormat)
{
	Q_UNUSED(animIconFormat)

	Q_D(McRecoverWindow);
	GcImageWriter::AnimImageFormat animImgf = d->animIconFormat();
	switch (animImgf) {
		case GcImageWriter::ANIMGF_APNG:
			d->ui.actionAnimAPNG->setChecked(true);
			break;
		case GcImageWriter::ANIMGF_GIF:
			d->ui.actionAnimGIF->setChecked(true);
			break;
		case GcImageWriter::ANIMGF_PNG_FPF:
			d->ui.actionAnimPNGfpf->setChecked(true);
			break;
		case GcImageWriter::ANIMGF_PNG_VS:
			d->ui.actionAnimPNGhs->setChecked(true);
			break;
		case GcImageWriter::ANIMGF_PNG_HS:
			d->ui.actionAnimPNGhs->setChecked(true);
			break;
		default:
			// Invalid format.
			d->ui.actionAnimAPNG->setChecked(false);
			d->ui.actionAnimGIF->setChecked(false);
			d->ui.actionAnimPNGfpf->setChecked(false);
			d->ui.actionAnimPNGhs->setChecked(false);
			d->ui.actionAnimPNGhs->setChecked(false);
			break;
	};
}

/**
 * UI language was changed by the user.
 * @param tsLocale Translation to use. (locale tag)
 */
void McRecoverWindow::setTranslation_slot(const QString &tsLocale)
{
	Q_D(McRecoverWindow);
	// If the locale isn't available, use the default.
	QString locale = (d->hashActionsTS.contains(tsLocale)
			  ? tsLocale
			  : QString());
	// d->cfg->set() will trigger a notification.
	d->cfg->set(QLatin1String("language"), locale);
}

/**
 * UI language was changed by the configuration.
 * @param tsLocale Translation to use. (locale tag)
 */
void McRecoverWindow::setTranslation_cfg_slot(const QVariant &tsLocale)
{
	Q_D(McRecoverWindow);
	QString locale = tsLocale.toString();
	QAction *action = d->hashActionsTS.value(locale);
	if (!action)
		locale = QString();

	// Set the UI language.
	TranslationManager::instance()->setTranslation(
		(!locale.isEmpty()
			? locale
			: QLocale::system().name()));
	// Mark the language as selected.
	if (action)
		action->setChecked(true);
	else
		d->actTsSysDefault->setChecked(true);
}

/**
 * "Search Used Blocks" was changed by the user.
 * @param checked True if checked; false if not.
 */
void McRecoverWindow::on_actionSearchUsedBlocks_triggered(bool checked)
{
	// Save the setting in the configuration.
	Q_D(McRecoverWindow);
	// d->cfg->set() will trigger a notification.
	d->cfg->set(QLatin1String("searchUsedBlocks"), checked);
}

/**
 * "Search Used Blocks" was changed by the configuration.
 * @param checked True if checked; false if not.
 */
void McRecoverWindow::searchUsedBlocks_cfg_slot(const QVariant &checked)
{
	Q_D(McRecoverWindow);
	d->ui.actionSearchUsedBlocks->setChecked(checked.toBool());
}
