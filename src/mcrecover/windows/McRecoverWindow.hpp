/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * McRecoverWindow.hpp: Main window.                                       *
 *                                                                         *
 * Copyright (c) 2012-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __MCRECOVER_MCRECOVERWINDOW_HPP__
#define __MCRECOVER_MCRECOVERWINDOW_HPP__

#include <QMainWindow>

// Qt includes.
#include <QtCore/QString>
#include <QItemSelection>

// MemCard Recover classes.
class MemCardFile;

class McRecoverWindowPrivate;
class McRecoverWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit McRecoverWindow(QWidget *parent = 0);
	virtual ~McRecoverWindow();

protected:
	McRecoverWindowPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(McRecoverWindow)
private:
	typedef QMainWindow super;
	Q_DISABLE_COPY(McRecoverWindow)

public:
	/**
	 * File type
	 */
	enum class FileType {
		Unknown = -1,

		GCN = 0,	// GameCube memory card
		GCI = 1,	// GameCube save file
		VMS = 2,	// Dreamcast memory card
	};

	/**
	 * Open a GameCube Memory Card image.
	 * @param filename Filename
	 * @param type Type hint from the Open dialog.
	 */
	void openCard(const QString &filename, FileType type = FileType::Unknown);

	/**
	 * Close the currently-opened GameCube Memory Card image.
	 * @param noMsg If true, don't show a message in the status bar.
	 */
	void closeCard(bool noMsg = false);

	protected:
	// State change event (Used for switching the UI language at runtime.)
	void changeEvent(QEvent *event) final;

	// QMainWindow virtual functions: drag and drop.
	void dragEnterEvent(QDragEnterEvent *event) final;
	void dropEvent(QDropEvent *event) final;

	// Window show event
	void showEvent(QShowEvent *event) final;

	// Window close event
	void closeEvent(QCloseEvent *event) final;

#ifdef Q_OS_WIN
	// Windows message handler. Used for TaskbarButtonManager.
	bool nativeEvent(const QByteArray &eventType, void *message, long *result) final;
#endif /* Q_OS_WIN */

protected slots:
	// UI busy functions
	void markUiBusy(void);
	void markUiNotBusy(void);

protected slots:
	// Actions
	void on_actionOpen_triggered(void);
	void on_actionClose_triggered(void);
	void on_actionScan_triggered(void);
	void on_actionExit_triggered(void);
	void on_actionAbout_triggered(void);

	// Save actions
	void on_actionSave_triggered(void);
	void on_actionSaveAll_triggered(void);

	/**
	 * Set the preferred region.
	 * This slot is triggered by a QSignalMapper that
	 * maps the various QActions.
	 * @param preferredRegion Preferred region (actually char)
	 */
	void setPreferredRegion_slot(int preferredRegion);

	/**
	 * Set the preferred region.
	 * This version is used by ConfigDefaults.
	 * @param preferredRegion Preferred region (actually char)
	 */
	void setPreferredRegion_slot(const QVariant &preferredRegion);

	// MemCardModel slots
	void memCardModel_layoutChanged(void);
	void memCardModel_rowsInserted(void);

	// SearchThread has finished
	void searchThread_searchFinished_slot(int lostFilesFound);

	// lstFileList slots
	void lstFileList_selectionModel_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	/**
	 * Set the animated icon format.
	 * This slot is triggered by a QSignalMapper that
	 * maps the various QActions.
	 * @param animIconFormat Animated icon format
	 */
	void setAnimIconFormat_slot(int animIconFormat);

	/**
	 * UI language was changed by the configuration.
	 * @param animIconFormat Animated icon format
	 */
	void setAnimIconFormat_cfg_slot(const QVariant &animIconFormat);

	/**
	 * UI language was changed by the user.
	 * @param locale Locale tag, e.g. "en_US".
	 */
	void on_menuLanguage_languageChanged(const QString &locale);

	/**
	 * UI language was changed by the configuration.
	 * @param locale Locale tag, e.g. "en_US".
	 */
	void setTranslation_cfg_slot(const QVariant &locale);

	/**
	 * "Search Used Blocks" was changed by the user.
	 * @param checked True if checked; false if not.
	 */
	void on_actionSearchUsedBlocks_triggered(bool checked);

	/**
	 * "Search Used Blocks" was changed by the configuration.
	 * @param checked True if checked; false if not.
	 */
	void searchUsedBlocks_cfg_slot(const QVariant &checked);

	/**
	 * "Allow Write" checkbox was changed by the user.
	 * @param checked True if checked; false if not.
	 */
	void chkAllowWrite_clicked(bool checked);
};

#endif /* __MCRECOVER_MCRECOVERWINDOW_HPP__ */
