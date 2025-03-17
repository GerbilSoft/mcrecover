/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * EditorWidget.hpp: Editor widget base class.                             *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBSAVEEDIT_EDITORWIDGET_HPP__
#define __LIBSAVEEDIT_EDITORWIDGET_HPP__

// C includes (C++ namespace)
#include <cassert>

// Qt includes
#include <QWidget>

// TODO: Namespace?
class File;
Q_DECLARE_OPAQUE_POINTER(File*)
Q_DECLARE_METATYPE(File*)

class EditorWidgetPrivate;
class EditorWidget : public QWidget
{
	Q_OBJECT

	// TODO: NOTIFY signal for file?
	Q_PROPERTY(File* file READ file WRITE setFile /*NOTIFY fileChanged*/)
	Q_PROPERTY(int currentSaveSlot READ currentSaveSlot WRITE setCurrentSaveSlot NOTIFY currentSaveSlotChanged);
	Q_PROPERTY(int saveSlots READ saveSlots NOTIFY saveSlotsChanged)
	Q_PROPERTY(bool generalSettings READ hasGeneralSettings NOTIFY generalSettingsChanged)

	Q_PROPERTY(bool modified READ isModified NOTIFY hasBeenModified)

protected:
	/**
	 * Create an EditorWidget.
	 * This EditorWidget is NOT valid by itself, and must be
	 * subclassed in order to implement certain functions.
	 * @param d EditorWidgetPrivate-derived private class
	 * @param parent Parent widget
	 */
	EditorWidget(EditorWidgetPrivate *d, QWidget *parent = nullptr);
public:
	virtual ~EditorWidget();

private:
	typedef QWidget super;
protected:
	EditorWidgetPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(EditorWidget)
private:
	Q_DISABLE_COPY(EditorWidget)

public:
	/** Public functions **/

	/**
	 * Get the file currently being edited.
	 * @return File being edited, or nullptr if none.
	 */
	File *file(void) const;

	/**
	 * Set the File to edit.
	 * This function MUST be overridden by subclasses.
	 *
	 * @param file File to edit
	 * If the file isn't valid, it won't be set;
	 * check file() afterwards to verify.
	 *
	 * @return 0 on success; non-zero on error (and file will not be set).
	 * TODO: Error code constants?
	 */
	virtual int setFile(File *file) = 0;

	// TODO: "Save" functions.

	/**
	 * Get the number of save slots available in this editor.
	 * @return Number of save slots
	 */
	int saveSlots(void) const;

	/**
	 * Does this editor have a "general" settings section in addition to slots?
	 * @return True if this editor has a "general" settings section.
	 */
	bool hasGeneralSettings(void) const;

	/**
	 * Get the current save slot.
	 * @return Current save slot (-1 for "general" settings)
	 */
	int currentSaveSlot(void) const;

public slots:
	/**
	 * Set the current save slot.
	 * @param saveSlot New save slot (-1 for "general" settings)
	 * TODO: Return the selected save slot?
	 */
	void setCurrentSaveSlot(int saveSlot);

	/**
	 * Save the data to the file.
	 * @return 0 on success; negative POSIX error code on error.
	 */
	virtual int save(void) = 0;

	/**
	 * Reload the save data.
	 */
	virtual void reload(void) = 0;

protected:
	/**
	 * Set the current save slot. [INTERNAL FUNCTION]
	 *
	 * This is called by the base class when the
	 * setCurrentSaveSlot() function is called.
	 * Subclasses should load the appropriate data
	 * from the specified save slot.
	 *
	 * If an error occurs, the save slot will not be changed.
	 *
	 * @param saveSlot New save slot (-1 for "general" settings)
	 * @return 0 on success; non-zero on error.
	 */
	virtual int setCurrentSaveSlot_int(int saveSlot) = 0;

signals:
	/**
	 * Number of save slots has changed.
	 * @param saveSlots New number of save slots
	 */
	void saveSlotsChanged(int saveSlots);

	/**
	 * Status of the "general" settings section has changed.
	 * @param hasGeneralSave True if this editor has a "general" settings section.
	 */
	void generalSettingsChanged(bool generalSettings);

	/**
	 * Current save slot has changed.
	 * @param saveSlot New save slot (-1 for "general" settings)
	 */
	void currentSaveSlotChanged(int saveSlot);

	/** File modified state **/
signals:
	/**
	 * Modification status has changed.
	 * @param modified New modified status
	 */
	void hasBeenModified(bool modified);

public:
	/**
	 * Has this widget been modified?
	 * @return True if modified; false if not.
	 */
	inline bool isModified(void) const;

protected:
	/**
	 * Change the 'modified' state.
	 * This function must be called instead of modifying
	 * the variable directly in order to handle signals.
	 * @param modified New 'modified' state
	 */
	inline void setModified(bool modified);

private:
	bool m_modified;
};

/**
 * Has this widget been modified?
 * @return True if modified; false if not.
 */
inline bool EditorWidget::isModified(void) const
{
	return m_modified;
}

/**
 * Change the 'modified' state.
 * This function must be called instead of modifying
 * the variable directly in order to handle signals.
 * @param modified New 'modified' state.
 */
inline void EditorWidget::setModified(bool modified)
{
	if (m_modified == modified) {
		return;
	}

	m_modified = modified;
	emit hasBeenModified(modified);
}

#endif /* __LIBSAVEEDIT_EDITORWIDGET_HPP__ */
