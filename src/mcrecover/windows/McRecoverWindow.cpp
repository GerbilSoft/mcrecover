/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * McRecoverWindow.cpp: Main window.                                       *
 *                                                                         *
 * Copyright (c) 2012-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "config.mcrecover.h"
#include "McRecoverWindow.hpp"
#include "util/array_size.h"
#include "util/bitstuff.h"

#include "McRecoverQApplication.hpp"
#include "AboutDialog.hpp"

// GcnCard classes
#include "libmemcard/GcnCard.hpp"
#include "libmemcard/GcnFile.hpp"
#include "libmemcard/MemCardModel.hpp"
#include "libmemcard/MemCardItemDelegate.hpp"
#include "libmemcard/MemCardSortFilterProxyModel.hpp"

// GciCard
#include "libmemcard/GciCard.hpp"

// VmuCard
#include "libmemcard/VmuCard.hpp"

// File database
#include "db/GcnMcFileDb.hpp"
#include "db/GcnCheckFiles.hpp"

// Search classes
#include "db/GcnSearchThread.hpp"
#include "widgets/StatusBarManager.hpp"

// Taskbar Button Manager
#include "TaskbarButtonManager/TaskbarButtonManager.hpp"
#include "TaskbarButtonManager/TaskbarButtonManagerFactory.hpp"
#ifdef Q_OS_WIN
#include <windows.h>
#endif /* Q_OS_WIN */

// C includes (C++ namespace)
#include <cstdio>
#include <cassert>

// C++ includes
#include <vector>
using std::list;
using std::vector;

// Qt includes
#include <QtCore/QUrl>
#include <QtCore/QStack>
#include <QtCore/QVector>
#include <QtCore/QFile>
#include <QtCore/QLocale>
#include <QtCore/QMimeData>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QAction>
#include <QActionGroup>
#include <QCheckBox>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QToolBar>

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
	explicit McRecoverWindowPrivate(McRecoverWindow *q);
	~McRecoverWindowPrivate();

protected:
	McRecoverWindow *const q_ptr;
	Q_DECLARE_PUBLIC(McRecoverWindow)
private:
	Q_DISABLE_COPY(McRecoverWindowPrivate)

public:
	Ui::McRecoverWindow ui;

	// Memory Card
	Card *card;
	MemCardModel *model;
	MemCardSortFilterProxyModel *proxyModel;

	/**
	 * Format a file size
	 * @param size File size
	 * @return Formatted file size
	 */
	static QString formatFileSize(quint64 size);

	// Filename
	QString filename;
	QString displayFilename;	// filename without subdirectories

	// Initialized columns?
	bool cols_init;

	/**
	 * Update the memory card's QTreeView.
	 */
	void updateLstFileList(void);

	// Search thread
	GcnSearchThread *searchThread;

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

	// Status Bar Manager
	StatusBarManager *statusBarManager;

	/**
	 * Update the window title.
	 */
	void updateWindowTitle(void);

	/**
	 * Change the file extension of the specified file.
	 * @param filename Filename
	 * @param newExt New extension, including leading dot
	 * @return Filename with new extension
	 */
	static QString changeFileExtension(const QString &filename, const QString &newExt);

	/**
	 * Save the specified file(s).
	 * @param files List of file(s) to save
	 * @param path If specified, save file(s) to path using default GCI filenames.
	 */
	void saveFiles(const QVector<File*> &files, QString path = QString());

	// UI busy counter
	int uiBusyCounter;

	/**
	 * "Preferred Region" selection
	 * Possible values: 0, 'E', 'P', 'J', 'K'
	 * 0 indicates no preferred region.
	 */
	char preferredRegion;
	QLabel *lblPreferredRegion;
	QActionGroup *actgrpRegion;

	/**
	 * "Animated Icon Format" selection
	 */
	QActionGroup *actgrpAnimIconFormat;

	// Configuration
	ConfigStore *cfg;

	/**
	 * Get the last path.
	 * @return Last path
	 */
	QString lastPath(void) const;

	/**
	 * Set the last path.
	 * @param path Last path
	 */
	void setLastPath(const QString &path);

	/**
	 * Get the animated icon format to use.
	 * @return Animated icon format to use
	 */
	GcImageWriter::AnimImageFormat animIconFormat(void) const;

	/**
	 * "Allow Write" checkbox in the toolbar.
	 * TODO: Better name, and/or change to "Read Only"?
	 */
	QCheckBox *chkAllowWrite;

	// Shh... it's a secret to everybody.
	HerpDerpEggListener *herpDerp;

	/**
	 * Read a memory card file and try to guess
	 * what system it's for.
	 *
	 * NOTE: This function will actually just check for VMU.
	 * If the VMU header is missing, it will assume GCN.
	 *
	 * @param filename Memory card filename
	 * @return McRecoverWindow::FileType
	 */
	static McRecoverWindow::FileType checkCardType(const QString &filename);

	// Taskbar Button Manager
	TaskbarButtonManager *taskbarButtonManager;
};

McRecoverWindowPrivate::McRecoverWindowPrivate(McRecoverWindow *q)
	: q_ptr(q)
	, card(nullptr)
	, model(new MemCardModel(q))
	, proxyModel(new MemCardSortFilterProxyModel(q))
	, cols_init(false)
	, searchThread(new GcnSearchThread(q))
	, statusBarManager(nullptr)
	, uiBusyCounter(0)
	, preferredRegion(0)
	, lblPreferredRegion(nullptr)
	, actgrpRegion(new QActionGroup(q))
	, actgrpAnimIconFormat(new QActionGroup(q))
	, cfg(new ConfigStore(q))
	, chkAllowWrite(nullptr)
	, herpDerp(new HerpDerpEggListener(q))
	, taskbarButtonManager(nullptr)
{
	// Connect the MemCardModel slots.
	QObject::connect(model, &MemCardModel::layoutChanged,
			 q, &McRecoverWindow::memCardModel_layoutChanged);
	QObject::connect(model, &MemCardModel::rowsInserted,
			 q, &McRecoverWindow::memCardModel_rowsInserted);

	// Connect the SearchThread slots.
	QObject::connect(searchThread, &GcnSearchThread::searchFinished,
			 q, &McRecoverWindow::searchThread_searchFinished_slot);

	// Connect searchThread to the mark-as-busy slots.
	QObject::connect(searchThread, &GcnSearchThread::searchStarted,
			 q, &McRecoverWindow::markUiBusy);
	QObject::connect(searchThread, &GcnSearchThread::searchFinished,
			 q, &McRecoverWindow::markUiNotBusy);
	QObject::connect(searchThread, &GcnSearchThread::searchError,
			 q, &McRecoverWindow::markUiNotBusy);
	QObject::connect(searchThread, &QObject::destroyed,
			 q, &McRecoverWindow::markUiNotBusy);

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
	delete taskbarButtonManager;
}

/**
 * Format a file size
 * @param size File size
 * @return Formatted file size
 */
QString McRecoverWindowPrivate::formatFileSize(quint64 size)
{
	// TODO: Optimize this?
	if (size < (2ULL << 10))
		return McRecoverWindow::tr("%Ln byte(s)", "", (int)size).arg(size);
	else if (size < (2ULL << 20))
		return McRecoverWindow::tr("%L1 KiB").arg(size >> 10);
	else if (size < (2ULL << 30))
		return McRecoverWindow::tr("%L1 MiB").arg(size >> 20);
	else if (size < (2ULL << 40))
		return McRecoverWindow::tr("%L1 GiB").arg(size >> 30);
	else if (size < (2ULL << 50))
		return McRecoverWindow::tr("%L1 TiB").arg(size >> 40);
	else if (size < (2ULL << 60))
		return McRecoverWindow::tr("%L1 PiB").arg(size >> 50);
	else /*if (size < (2ULL << 70))*/
		return McRecoverWindow::tr("%L1 EiB").arg(size >> 60);
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

	// Connect QAction signals to the slot using a lambda function.
	Q_Q(McRecoverWindow);
	QObject::connect(ui.actionRegionUSA, &QAction::triggered, [q]() {
		q->setPreferredRegion_slot('E');
	});
	QObject::connect(ui.actionRegionPAL, &QAction::triggered, [q]() {
		q->setPreferredRegion_slot('P');
	});
	QObject::connect(ui.actionRegionJPN, &QAction::triggered, [q]() {
		q->setPreferredRegion_slot('J');
	});
	QObject::connect(ui.actionRegionKOR, &QAction::triggered, [q]() {
		q->setPreferredRegion_slot('K');
	});

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
			QObject::connect(animImgfActions[i], &QAction::triggered, [q, i]() {
				q->setAnimIconFormat_slot(i+1);
			});
		} else {
			// Image format is not available.
			// Disable the action.
			animImgfActions[i]->setEnabled(false);
		}
	}

	// Make sure the "About" button is right-aligned.
	QWidget *spacer = new QWidget(q);
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	ui.toolBar->insertWidget(ui.actionAbout, spacer);

	// "Allow Write" checkbox.
	// Disabled by default; enabled if a card with no errors is loaded.
	chkAllowWrite = new QCheckBox(McRecoverWindow::tr("Allow Writing"), q);
	chkAllowWrite->setObjectName(QLatin1String("chkAllowWrite"));
	chkAllowWrite->setToolTip(McRecoverWindow::tr(
		"If checked, enables writing to the memory card by importing\n"
		"files and/or using the built-in editors. This is a potentially\n"
		"dangerous operation, and can result in the corruption of files\n"
		"that have not been recovered.\n\n"
		"This option is not available if the opened memory card has errors."));
	chkAllowWrite->setEnabled(false);
	QObject::connect(chkAllowWrite, SIGNAL(clicked(bool)),
			 q, SLOT(chkAllowWrite_clicked(bool)));
	ui.toolBar->insertWidget(ui.actionAbout, chkAllowWrite);

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
		ui.actionScan->setEnabled(true);
		ui.actionSave->setEnabled(
			ui.lstFileList->selectionModel()->hasSelection());
		ui.actionSaveAll->setEnabled(card->fileCount() > 0);
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
 * @param filename Filename
 * @param newExt New extension, including leading dot
 * @return Filename with new extension
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
 * @param files List of file(s) to save
 * @param path If specified, save file(s) to path using default GCI filenames.
 */
void McRecoverWindowPrivate::saveFiles(const QVector<File*> &files, QString path)
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
	enum class OverwriteAllStatus {
		Unknown		= 0,
		YesToAll	= 1,
		NoToAll		= 2,
	};

	int filesSaved = 0;
	OverwriteAllStatus overwriteAll = OverwriteAllStatus::Unknown;

	if (files.size() == 1 && path.isEmpty()) {
		// Single file, path not specified.
		singleFile = true;
		overwriteAll = OverwriteAllStatus::YesToAll;
		File *file = files.at(0);

		const QString defFilename = lastPath() + QChar(L'/') +
						file->defaultExportFilename();

		// Prompt the user for a save location.
		// FIXME: What type of file?
		filename = QFileDialog::getSaveFileName(q,
				McRecoverWindow::tr("Save GCN Save File %1")
					.arg(file->filename()),	// Dialog title
				defFilename,			// Default filename
				// TODO: Remove extra space from the filename filter?
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

	foreach (File *file, files) {
		if (!singleFile) {
			const QString exportFilename = file->defaultExportFilename();
			filename = path + QChar(L'/') + exportFilename;

			// Check if the file exists.
			// NOTE: Not done in the case of a single file because
			// the "Save" dialog already prompted the user.
			if (QFile::exists(filename)) {
				if (overwriteAll == OverwriteAllStatus::Unknown) {
					bool overwrite = false;
					int ret = QMessageBox::warning(q,
						McRecoverWindow::tr("File Already Exists"),
						McRecoverWindow::tr("A file named \"%1\" already exists in the specified directory.\n\n"
								    "Do you want to overwrite it?")
								.arg(exportFilename),
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
							overwriteAll = OverwriteAllStatus::YesToAll;
							overwrite = true;
							break;

						case QMessageBox::NoToAll:
							// Don't overwrite this file or any other files.
							overwriteAll = OverwriteAllStatus::NoToAll;
							overwrite = false;
							break;
					}

					if (!overwrite)
						continue;
				} else if (overwriteAll == OverwriteAllStatus::NoToAll) {
					// Don't overwrite any files.
					continue;
				}
			}
		}

		// Save the file.
		int ret = file->exportToFile(filename);
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
			if (file->iconCount() >= 1) {
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
 * Get the last path.
 * @return Last path
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
 * @param path Last path
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
 * @return Animated icon format to use
 */
GcImageWriter::AnimImageFormat McRecoverWindowPrivate::animIconFormat(void) const
{
	// TODO: Cache the last animated icon format?

	QString fmt = cfg->get(QLatin1String("animIconFormat")).toString();
	GcImageWriter::AnimImageFormat animImgf =
		GcImageWriter::animImageFormatFromName(fmt.toLatin1().constData());
	if (!GcImageWriter::isAnimImageFormatSupported(animImgf)) {
		// Format is not supported.
		animImgf = GcImageWriter::AnimImageFormat::Unknown;
	}

	if (animImgf == GcImageWriter::AnimImageFormat::Unknown) {
		// Determine which format to use.
		for (int i = 1; i < (int)GcImageWriter::AnimImageFormat::Max; i++) {
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

/**
 * Read a memory card file and try to guess
 * what system it's for.
 *
 * NOTE: This function will actually just check for VMU.
 * If the VMU header is missing, it will assume GCN
 * if the filesize is a power of two, or GCI if it has
 * a 64-byte header.
 *
 * @param filename Memory card filename
 * @return McRecoverWindow::FileType
 */
McRecoverWindow::FileType McRecoverWindowPrivate::checkCardType(const QString &filename)
{
	QFile file(filename);

	const qint64 filesize = file.size();
	if (filesize == 131072) {
		// Possibly a Dreamcast VMU.
		// TODO: Support for 4x cards, though
		// 4x dumps are likely "four regular dumps".

		// Check if 0x1FE00 - 0x1FE0F is all 0x55.
		// If it is, then this is probably a VMU.
		if (!file.open(QIODevice::ReadOnly))
			goto not_vmu;
		if (!file.seek(0x1FE00))
			goto not_vmu;

		// Read the data.
		QByteArray ba = file.read(16);
		if (ba.size() != 16)
			goto not_vmu;

		// Make sure all 16 bytes are 0x55.
		const char *data = ba.data();
		for (int i = ba.size(); i > 0; i--, data++) {
			if (*data != 0x55) {
				goto not_vmu;
			}
		}

		// This is probably a VMU.
		return McRecoverWindow::FileType::VMS;
	}

not_vmu:
	// Check for a valid GCN memory card size:
	// - Minimum: 512 KiB
	// - Maximum: 16 MiB
	// - Must be a power of two.
	if (filesize >= 512*1024 && filesize <= 16*1024*1024 &&
	    isPow2((unsigned int)filesize))
	{
		// Power of two. This is probably a GCN memory card.
		return McRecoverWindow::FileType::GCN;
	}

	// Check for a valid GCN file size:
	// - Minimum: 8 KiB + 64
	// - Maximum: 16,344 KiB + 64
	// - Must be a multiple of 8 KiB, plus 64 bytes.
	if (filesize >= ((8*1024)+64) && filesize <= ((16344*1024)+64) &&
	    ((filesize - 64) % 8192) == 0)
	{
		// This is probably a GCI file.
		return McRecoverWindow::FileType::GCI;
	}

	// Unknown file type.
	return McRecoverWindow::FileType::Unknown;
}


/** McRecoverWindow **/

McRecoverWindow::McRecoverWindow(QWidget *parent)
	: super(parent)
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

	// Set up the main splitter sizes.
	// We want the card info panel to be 160px wide at startup.
	// TODO: Save positioning settings somewhere?
	static const int MemCardInfoPanelWidth = 256;
	QList<int> sizes;
	sizes.append(this->width() - MemCardInfoPanelWidth);
	sizes.append(MemCardInfoPanelWidth);
	d->ui.splitterMain->setSizes(sizes);

	// Set the main splitter stretch factors.
	// We want the QTreeView to stretch, but not the card info panel.
	d->ui.splitterMain->setStretchFactor(0, 1);
	d->ui.splitterMain->setStretchFactor(1, 0);

	// Initialize lstFileList's item delegate.
	d->ui.lstFileList->setItemDelegate(new MemCardItemDelegate(this));

	// Set the models.
	d->proxyModel->setSourceModel(d->model);
	d->ui.lstFileList->setModel(d->proxyModel);

	// Sort by COL_DESCRIPTION by default.
	// TODO: Disable sorting on specific columns.
	//d->proxyModel->setDynamicSortFilter(true);
	//d->ui.lstFileList->sortByColumn(MemCardModel::COL_DESCRIPTION, Qt::AscendingOrder);

	// Connect the lstFileList slots.
	connect(d->ui.lstFileList->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &McRecoverWindow::lstFileList_selectionModel_selectionChanged);

	// Initialize the UI.
	d->updateLstFileList();
	d->initToolbar();
	d->statusBarManager = new StatusBarManager(d->ui.statusBar, this);
	d->updateWindowTitle();

	// Shh... it's a secret to everybody.
	connect(d->ui.lstFileList, &QTreeViewOpt::keyPress,
		d->herpDerp, &HerpDerpEggListener::widget_keyPress);
	connect(d->ui.lstFileList, &QTreeViewOpt::focusOut,
		d->herpDerp, &HerpDerpEggListener::widget_focusOut);

#ifndef Q_OS_WIN
	// Initialize the Taskbar Button Manager. (Non-Windows systems)
	d->taskbarButtonManager = TaskbarButtonManagerFactory::createManager(this);
	d->statusBarManager->setTaskbarButtonManager(d->taskbarButtonManager);
#endif /* Q_OS_WIN */

	// Emit all configuration signals.
	d->cfg->notifyAll();
}

McRecoverWindow::~McRecoverWindow()
{
	delete d_ptr;
}

/**
 * Open a GameCube Memory Card image.
 * @param filename Filename
 * @param type Type hint from the Open dialog.
 */
void McRecoverWindow::openCard(const QString &filename, FileType type)
{
	Q_D(McRecoverWindow);

	if (d->card) {
		d->model->setCard(nullptr);
		d->ui.mcCardView->setCard(nullptr);
		d->ui.mcfFileView->setFile(nullptr);
		d->chkAllowWrite->setEnabled(false);
		d->chkAllowWrite->setChecked(false);
		delete d->card;
	}

	/** TODO: CardFactory **/

	// TODO: Use an enum for 'type'?
	if (type == FileType::Unknown) {
		// Check what type of card this is.
		type = d->checkCardType(filename);
		if (type == FileType::Unknown) {
			// Still unknown.
			// Assume GCN.
			type = FileType::GCN;
		}
	}

	// Open the specified memory card image.
	// TODO: Set this as the last path?
	const char *className;
	switch (type) {
		default:
		case FileType::GCN:
			className = "GcnCard";
			d->card = GcnCard::open(filename, this);
			break;
		case FileType::GCI:
			className = "GciCard";
			d->card = GciCard::open(filename, this);
			break;
		case FileType::VMS:
			className = "VmuCard";
			d->card = VmuCard::open(filename, this);
			break;
	}

	if (!d->card || !d->card->isOpen()) {
		// Could not open the card.
		static const QChar chrBullet(0x2022);  // U+2022: BULLET
		QString filename_noPath = QFileInfo(filename).fileName();

		QString errorString;
		if (d->card) {
			errorString = d->card->errorString();
			if (!errorString.isEmpty()) {
				// Qt error strings don't have a trailing '.'
				// TODO: Move this to Card's error string functions?
				errorString += QChar(L'.');
			}
		}

		QString errMsg = tr("An error occurred while opening %1:")
					.arg(filename_noPath) +
				QChar(L'\n') + chrBullet + QChar(L' ');
		if (!errorString.isEmpty()) {
			errMsg += errorString;
		} else {
			//: Failure message when opening a card. (%1 == class name)
			errMsg += tr("%1 failed.").arg(QLatin1String(className));
		}
		d->ui.msgWidget->showMessage(errMsg, MessageWidget::MsgIcon::Warning);
		closeCard(true);
		return;
	}

	d->filename = filename;

	// If GCN, check file checksums.
	// TODO: Run this in a separate thread after loading?
	if (type == FileType::GCN) {
		// TODO: Singleton database management class.
		// Get the database filenames.
		QVector<QString> dbFilenames = GcnMcFileDb::GetDbFilenames();
		if (!dbFilenames.isEmpty()) {
			// Load the databases.
			GcnCheckFiles checkFiles;
			int ret = checkFiles.loadGcnMcFileDbs(dbFilenames);
			if (ret == 0) {
				// Check the files.
				checkFiles.addChecksumDefs(qobject_cast<GcnCard*>(d->card));
			}
		}
	}

	d->model->setCard(d->card);

	// Extract the filename from the path.
	d->displayFilename = filename;
	int lastSlash = d->displayFilename.lastIndexOf(QChar(L'/'));
	if (lastSlash >= 0)
		d->displayFilename.remove(0, lastSlash + 1);

	// Set the CardView's Card to the
	// selected card in the QTreeView.
	d->ui.mcCardView->setCard(d->card);

	// Check for other card errors.
	// NOTE: These aren't retranslated if the UI is retranslated.
	QStringList sl_cardErrors;
	QFlags<GcnCard::Error> cardErrors = d->card->errors();
	QString cardSz = d->formatFileSize(d->card->filesize());
	if (cardErrors & GcnCard::MCE_HEADER_GARBAGE) {
		uint8_t bad_byte; int count; int total;
		if (!d->card->garbageInfo(&bad_byte, &count, &total)) {
			float pct = (float)count / (float)total * 100.0f;
			char hex_byte[8];
			snprintf(hex_byte, sizeof(hex_byte), "%02X", bad_byte);
			//: %1 is a percentage; %2 is a formatted size, e.g. "100 bytes" or "2 MB"; %3 is a two-digit hexadecimal number.
			sl_cardErrors += tr("The header appears to contain garbage. "
					"%1% of the %2 header is the same byte, 0x%3.")
				.arg(pct, 0, 'f', 2)
				.arg(d->formatFileSize(total), QLatin1String(hex_byte));
		}
	}
	if (cardErrors & GcnCard::MCE_SZ_TOO_SMALL) {
		QString minSz = d->formatFileSize(d->card->minBlocks() * d->card->blockSize());
		//: %1 and %2 are both formatted sizes, e.g. "100 bytes" or "2 MB".
		sl_cardErrors +=
			tr("The card image is too small. (Card image is %1; should be at least %2.)")
			.arg(cardSz).arg(minSz);
	}
	if (cardErrors & GcnCard::MCE_SZ_TOO_BIG) {
		QString maxSz = d->formatFileSize(d->card->maxBlocks() * d->card->blockSize());
		//: %1 and %2 are both formatted sizes, e.g. "100 bytes" or "2 MB".
		sl_cardErrors +=
			tr("The card image is too big. (Card image is %1; should be %2 or less.)")
			.arg(cardSz).arg(maxSz);
	}
	if (cardErrors & GcnCard::MCE_SZ_NON_POW2) {
		// TODO: Convert filesize to KB/MB/GB?
		//: %1 is a formatted size, e.g. "100 bytes" or "2 MB".
		sl_cardErrors +=
			tr("The card image size is not a power of two. (Card image is %1.)")
			.arg(cardSz);
	}
	if (cardErrors & GcnCard::MCE_INVALID_HEADER) {
		sl_cardErrors += tr("The header checksum is invalid.");
	}
	if (cardErrors & GcnCard::MCE_INVALID_DATS) {
		sl_cardErrors += tr("Both directory tables are invalid.");
	}
	if (cardErrors & GcnCard::MCE_INVALID_BATS) {
		sl_cardErrors += tr("Both block tables are invalid.");
	}

	if (!sl_cardErrors.isEmpty()) {
		// Errors detected.
		static const QChar chrBullet(0x2022);  // U+2022: BULLET
		QString msg;
		msg.reserve(2048);
		msg += tr("Error(s) have been detected in this %1 image:", "",
			sl_cardErrors.size()).arg(d->card->productName());
		foreach (const QString &str, sl_cardErrors) {
			msg += QChar(L'\n') + chrBullet + QChar(L' ') + str;
		}

		// Show a warning message.
		d->ui.msgWidget->showMessage(msg, MessageWidget::MsgIcon::Warning, 0, d->card);
	}

	// Can we allow writing to this memory card?
	// NOTE: Currently disabled in Release builds.
#ifndef NDEBUG
	if (!cardErrors && d->card->canMakeWritable()) {
		// No errors, and card can be made writable.
		d->chkAllowWrite->setChecked(!d->card->isReadOnly());
		d->chkAllowWrite->setEnabled(true);
	} else
#endif /* !NDEBUG */
	{
		// Card has errors or cannot be made writable.
		d->chkAllowWrite->setEnabled(false);
		d->chkAllowWrite->setChecked(false);
	}

	// Update the UI.
	d->updateLstFileList();
	d->statusBarManager->opened(filename, d->card->productName());
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
	QString productName;
	if (d->card) {
		productName = d->card->productName();
	}

	d->model->setCard(nullptr);
	d->ui.mcCardView->setCard(nullptr);
	d->ui.mcfFileView->setFile(nullptr);
	delete d->card;
	d->card = nullptr;

	// Disable the "Allow Write" checkbox.
	d->chkAllowWrite->setEnabled(false);
	d->chkAllowWrite->setChecked(false);

	// Clear the filenames.
	d->filename.clear();
	d->displayFilename.clear();

	// Update the UI.
	d->updateLstFileList();
	if (!noMsg) {
		d->statusBarManager->closed(productName);
	}
	d->updateWindowTitle();
}

/**
 * Widget state has changed.
 * @param event State change event.
 */
void McRecoverWindow::changeEvent(QEvent *event)
{
	Q_D(McRecoverWindow);

	switch (event->type()) {
		case QEvent::LanguageChange:
			// Retranslate the UI.
			d->ui.retranslateUi(this);
			d->updateLstFileList();
			d->updateWindowTitle();
			d->retranslateToolbar();
			break;

		case QEvent::WindowStateChange: {
			// TODO: Add onMinimized() / onMaximized() signals/slots to a base class?
			if (d->model) {
				QWindowStateChangeEvent *wsc_event = (QWindowStateChangeEvent*)event;
				if (!(wsc_event->oldState() & Qt::WindowMinimized) && isMinimized()) {
					// Window was just minimized.
					d->model->pauseAnimation();
					d->ui.mcfFileView->pauseAnimation();
				} else if ((wsc_event->oldState() & Qt::WindowMinimized) && !isMinimized()) {
					// Window was just un-minimized.
					d->model->resumeAnimation();
					d->ui.mcfFileView->resumeAnimation();
				}
			}
			break;
		}

		default:
			break;
	}

	// Pass the event to the base class.
	super::changeEvent(event);
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
 * Window show event.
 * @param event Window show event.
 */
void McRecoverWindow::showEvent(QShowEvent *event)
{
	Q_UNUSED(event);
	Q_D(McRecoverWindow);

	// Show icon, description, size, mtime, permission, and gamecode by default.
	// TODO: Allow the user to customize the columns, and save the
	// customized columns somewhere.
	if (!d->cols_init) {
		d->cols_init = true;
		d->ui.lstFileList->setColumnHidden(MemCardModel::COL_ICON, false);
		d->ui.lstFileList->setColumnHidden(MemCardModel::COL_BANNER, true);
		d->ui.lstFileList->setColumnHidden(MemCardModel::COL_DESCRIPTION, false);
		d->ui.lstFileList->setColumnHidden(MemCardModel::COL_SIZE, false);
		d->ui.lstFileList->setColumnHidden(MemCardModel::COL_MTIME, false);
		d->ui.lstFileList->setColumnHidden(MemCardModel::COL_MODE, false);
		d->ui.lstFileList->setColumnHidden(MemCardModel::COL_GAMEID, false);
		d->ui.lstFileList->setColumnHidden(MemCardModel::COL_FILENAME, true);
		d->ui.lstFileList->setColumnHidden(MemCardModel::COL_ISVALID, false);
		static_assert(MemCardModel::COL_ISVALID + 1 == MemCardModel::COL_MAX,
			"Default column visibility status needs to be updated!");
	}

	// Pass the event to the base class.
	super::showEvent(event);
}

/**
 * Window close event.
 * @param event Window close event.
 */
void McRecoverWindow::closeEvent(QCloseEvent *event)
{
	Q_D(McRecoverWindow);
	if (d->uiBusyCounter > 0) {
		// UI is busy. Ignore the close event.
		event->ignore();
		return;
	}

	// Pass the event to the base class.
	super::closeEvent(event);
}

#ifdef Q_OS_WIN
// Windows message handler. Used for TaskbarButtonManager.
bool McRecoverWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
	// Reference: http://nicug.blogspot.com/2011/03/windows-7-taskbar-extensions-in-qt.html
	Q_D(McRecoverWindow);
	if (((MSG*)message)->message == McRecoverQApplication::WM_TaskbarButtonCreated()) {
		// Initialize the Taskbar Button Manager.
		d->taskbarButtonManager = TaskbarButtonManagerFactory::createManager(this);
		if (d->taskbarButtonManager) {
			d->taskbarButtonManager->setWindow(this);
			d->statusBarManager->setTaskbarButtonManager(d->taskbarButtonManager);
		}
	}
	return false;
}
#endif /* Q_OS_WIN */

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

		// TODO: Disable the close button?
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

	// TODO: Remove the space before the "*.raw"?
	// On Linux, Qt shows an extra space after the filter name, since
	// it doesn't show the extension. Not sure about Windows...
	const QString gcnFilter = tr("GameCube Memory Card Image") + QLatin1String(" (*.raw)");
	const QString gciFilter = tr("GameCube Save File") + QLatin1String(" (*.gci)");
	const QString vmuFilter = tr("Dreamcast VMU Image") + QLatin1String(" (*.bin)");
	const QString allFilter = tr("All Files") + QLatin1String(" (*)");

	// NOTE: Using a QFileDialog instead of QFileDialog::getOpenFileName()
	// causes a non-native appearance on Windows. Hence, we should use
	// QFileDialog::getOpenFileName().
	const QString filters = gcnFilter + QLatin1String(";;") +
				gciFilter + QLatin1String(";;") +
				vmuFilter + QLatin1String(";;") +
				allFilter;

	// Set the default filter.
	QString selectedFilter;
	const QString fileTypeKey = QLatin1String("fileType");
	switch ((FileType)d->cfg->getInt(fileTypeKey)) {
		default:
		case FileType::Unknown:
			selectedFilter = allFilter;
			break;
		case FileType::GCN:
			selectedFilter = gcnFilter;
			break;
		case FileType::GCI:
			selectedFilter = gcnFilter;
			break;
		case FileType::VMS:
			selectedFilter = vmuFilter;
			break;
	}

	// FIXME: This isn't working on KDE 4.
	// This may be a Qt/KDE bug:
	// - https://git.reviewboard.kde.org/r/115478/
	// - https://mail.kde.org/pipermail/kde-frameworks-devel/2014-March/012659.html
	// - http://mail.kde.org/pipermail/kde-frameworks-devel/2014-February/010691.html
	//dialog->selectNameFilter(defaultFilter);

	// Get the filename.
	QString filename = QFileDialog::getOpenFileName(this,
			tr("Open GameCube Memory Card Image"),	// Dialog title
			d->lastPath(),				// Default filename
			filters,				// Filters
			&selectedFilter);			// Selected filter

	if (!filename.isEmpty()) {
		// Filename is selected.

		// Check the selected filename filter.
		FileType type = FileType::Unknown;	// TODO: Enum?
		if (selectedFilter == gcnFilter) {
			type = FileType::GCN;
		} else if (selectedFilter == gciFilter) {
			type = FileType::VMS;
		} else if (selectedFilter == vmuFilter) {
			type = FileType::VMS;
		} else if (selectedFilter == allFilter) {
			type = FileType::Unknown;	// Auto-detect
		}

		// Save configuration settings.
		d->setLastPath(filename);
		d->cfg->set(fileTypeKey, (int)type);

		// Open the memory card file.
		openCard(filename, type);
	}
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
	if (!d->card)
		return;
	// FIXME: Disable the button if the loaded card doesn't support scanning.
	GcnCard *gcnCard = qobject_cast<GcnCard*>(d->card);
	if (!gcnCard)
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
	// TODO: Singleton database management class.
	int ret = d->searchThread->loadGcnMcFileDbs(dbFilenames);
	if (ret != 0)
		return;

	// Remove "lost" files from the card.
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
	ret = d->searchThread->searchMemCard_async(gcnCard, d->preferredRegion, searchUsedBlocks);
	if (ret < 0) {
		// Error starting the thread.
		// Use the synchronous version.
		// TODO: Handle errors.
		// NOTE: Files will be added by searchThread_searchFinished_slot().
		ret = d->searchThread->searchMemCard(gcnCard, d->preferredRegion, searchUsedBlocks);
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

	QVector<File*> files;
	files.reserve(selList.size());

	foreach(QModelIndex idx, selList) {
		QModelIndex srcIdx = d->proxyModel->mapToSource(idx);
		File *file = d->card->getFile(srcIdx.row());
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

	const int numFiles = d->card->fileCount();
	if (numFiles <= 0)
		return;

	QVector<File*> files = d->card->getFiles(Card::FileTypes::All);
	if (files.isEmpty()) {
		// No files to save...
		return;
	}

	// Save the files.
	d->saveFiles(files);
}

/**
 * Set the preferred region.
 * This slot is triggered by a QSignalMapper that
 * maps the various QActions.
 * @param preferredRegion Preferred region (actually char)
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
	QString str = QChar(static_cast<uint16_t>(preferredRegion));
	d->cfg->set(QLatin1String("preferredRegion"), str);
}

/**
 * Set the preferred region.
 * This version is used by ConfigDefaults.
 * @param preferredRegion Preferred region (actually char)
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
	// A new file entry was added to the GcnCard.
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

	// FIXME: Move "lost files" code to Card?
	GcnCard *gcnCard = qobject_cast<GcnCard*>(d->card);
	if (!gcnCard)
		return;

	// Remove lost files from the card.
	d->card->removeLostFiles();

	// Get the files found list.
	list<GcnSearchData> filesFoundList = d->searchThread->filesFoundList();

	// Add the directory entries.
	QList<GcnFile*> files = gcnCard->addLostFiles(filesFoundList);
}

/**
 * lstFileList selectionModel: Current row selection has changed.
 * @param selected Selected index.
 * @param deselected Deselected index.
 */
void McRecoverWindow::lstFileList_selectionModel_selectionChanged(
	const QItemSelection& selected, const QItemSelection& deselected)
{
	Q_UNUSED(selected)
	Q_UNUSED(deselected)
	Q_D(McRecoverWindow);

	// FIXME: QItemSelection::indexes() *crashes* in MSVC debug builds. (Qt 4.8.6)
	// References: (search for "QModelIndexList assertion", no quotes)
	// - http://www.qtforum.org/article/13355/qt4-qtableview-assertion-failure.html#post66572
	// - http://www.qtcentre.org/threads/55614-QTableView-gt-selectionModel%20%20-gt-selection%20%20-indexes%20%20-crash#8766774666573257762
	// - https://forum.qt.io/topic/24664/crash-with-qitemselectionmodel-selectedindexes
	//QModelIndexList indexes = selected.indexes();
	int file_idx = -1;
	const File *file = nullptr;
	QItemSelectionModel *const selectionModel = d->ui.lstFileList->selectionModel();
	if (selectionModel->hasSelection()) {
		// TODO: If multiple files are selected, and one of the
		// files was just now unselected, this will still be the
		// unselected file.
		QModelIndex index = d->ui.lstFileList->selectionModel()->currentIndex();
		if (index.isValid()) {
			file_idx = d->proxyModel->mapToSource(index).row();
			file = d->card->getFile(file_idx);
		}
	}

	// If file(s) are selected, enable the Save action.
	d->ui.actionSave->setEnabled(file_idx >= 0);

	// Set the FileView's File to the
	// selected file in the QTreeView.
	// NOTE: Only handles the first selected file.
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
		static_cast<GcImageWriter::AnimImageFormat>(animIconFormat));
	QString s_fmt = (fmt ? QLatin1String(fmt) : QString());

	Q_D(McRecoverWindow);
	// d->cfg->set() will trigger a notification.
	d->cfg->set(QLatin1String("animIconFormat"), s_fmt);
}

/**
 * UI language was changed by the configuration.
 * @param animIconFormat Animated icon format
 */
void McRecoverWindow::setAnimIconFormat_cfg_slot(const QVariant &animIconFormat)
{
	Q_UNUSED(animIconFormat)

	Q_D(McRecoverWindow);
	GcImageWriter::AnimImageFormat animImgf = d->animIconFormat();
	switch (animImgf) {
		case GcImageWriter::AnimImageFormat::APNG:
			d->ui.actionAnimAPNG->setChecked(true);
			break;
		case GcImageWriter::AnimImageFormat::GIF:
			d->ui.actionAnimGIF->setChecked(true);
			break;
		case GcImageWriter::AnimImageFormat::PNG_FPF:
			d->ui.actionAnimPNGfpf->setChecked(true);
			break;
		case GcImageWriter::AnimImageFormat::PNG_VS:
			d->ui.actionAnimPNGhs->setChecked(true);
			break;
		case GcImageWriter::AnimImageFormat::PNG_HS:
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
 * @param locale Locale tag, e.g. "en_US".
 */
void McRecoverWindow::on_menuLanguage_languageChanged(const QString &locale)
{
	Q_D(McRecoverWindow);
	// TODO: Verify that the specified locale is valid.
	// (LanguageMenu::isLanguageSupported() or something?)
	// d->cfg->set() will trigger a notification.
	d->cfg->set(QLatin1String("language"), locale);
}

/**
 * UI language was changed by the configuration.
 * @param locale Locale tag, e.g. "en_US".
 */
void McRecoverWindow::setTranslation_cfg_slot(const QVariant &locale)
{
	Q_D(McRecoverWindow);
	d->ui.menuLanguage->setLanguage(locale.toString());
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

/**
 * "Allow Write" checkbox was changed by the user.
 * @param checked True if checked; false if not.
 */
void McRecoverWindow::chkAllowWrite_clicked(bool checked)
{
	Q_D(McRecoverWindow);
	if (!d->card || d->card->errors() != 0) {
		// Card isn't present, or card has errors.
		// This slot shouldn't have been triggered.
		d->chkAllowWrite->setEnabled(false);
		d->chkAllowWrite->setChecked(false);
		return;
	}

	// Switch the read-only mode.
	// NOTE: This box is true for "write", whereas setReadOnly is false for "write".
	int ret = d->card->setReadOnly(!checked);
	if (ret != 0) {
		// Could not switch the read-only mode.
		d->chkAllowWrite->setEnabled(false);
		d->chkAllowWrite->setChecked(!checked);

		static const QChar chrBullet(0x2022);  // U+2022: BULLET
		QString errMsg;
		if (checked) {
			errMsg = tr("An error occurred while attempting to allow writing:");
		} else {
			errMsg = tr("An error occurred while attempting to disallow writing:");
		}
		errMsg += QChar(L'\n') + chrBullet + QChar(L' ');

		QString errorString = d->card->errorString();
		if (!errorString.isEmpty()) {
			// Qt error strings don't have a trailing '.'
			// TODO: Move this to Card's error string functions?
			errMsg += errorString + QChar(L'.');
		} else {
			// Use strerror().
			errMsg += QLatin1String(strerror(-ret)) + QChar(L'.');
		}
		d->ui.msgWidget->showMessage(errMsg, MessageWidget::MsgIcon::Warning);
	}
}
