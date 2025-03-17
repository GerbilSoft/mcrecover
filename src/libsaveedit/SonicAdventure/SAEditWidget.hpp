/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SAEditWidget.hpp: Sonic Adventure - SA1/SADX edit widget base class.    *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include <QWidget>
#include <cassert>

struct _sa_save_slot;

class SAEditWidget : public QWidget
{
	Q_OBJECT

	Q_PROPERTY(bool modified READ isModified NOTIFY hasBeenModified)

	public:
		explicit SAEditWidget(QWidget *parent = 0)
			: super(parent)
			, m_modified(false)
			, m_suspendHasBeenModified(0) { }

	private:
		typedef QWidget super;
		Q_DISABLE_COPY(SAEditWidget)

	signals:
		/**
		 * Modification status has changed.
		 * @param modified New modified status.
		 */
		void hasBeenModified(bool modified);
		
	private:
		// NOTE: This is modified by the save functions,
		// so it must be mutable.
		mutable bool m_modified;

	public:
		/**
		 * Has this widget been modified?
		 * @return True if modified; false if not.
		 */
		inline bool isModified(void) const
		{
			return m_modified;
		}

	protected:
		/**
		 * Change the 'modified' state.
		 * This function must be called instead of modifying
		 * the variable directly in order to handle signals.
		 * @param modified New 'modified' state.
		 */
		void setModified(bool modified);

	protected slots:
		/**
		 * Connect a widget's modified signal to this slot
		 * to automatically set the modified state.
		 */
		void widgetModifiedSlot(void);

	private:
		// Temporarily suspend modification signals.
		// Increment this when loading data,
		// and decrement when finished.
		int m_suspendHasBeenModified;

	protected:
		/**
		 * Suspend the hasBeenModified signal.
		 * Increments a counter; call the unsuspend
		 * function the same number of times.
		 */
		inline void suspendHasBeenModified(void)
		{
			m_suspendHasBeenModified++;
		}

		/**
		 * Unsuspend the hasBeenModified signal.
		 * Decrements a counter; when it reaches 0,
		 * the signal is re-enabled.
		 */
		inline void unsuspendHasBeenModified(void)
		{
			assert(m_suspendHasBeenModified > 0);
			m_suspendHasBeenModified--;
		}

	public:
		/**
		 * Clear the loaded data.
		 * TODO: Make this function pure virtual.
		 */
		virtual void clear(void) { };

	public:
		/**
		 * Load data from a Sonic Adventure save slot.
		 * @param sa_save Sonic Adventure save slot.
		 * The data must have already been byteswapped to host-endian.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int load(const _sa_save_slot *sa_save) = 0;

		/**
		 * Save data to a Sonic Adventure save slot.
		 * @param sa_save Sonic Adventure save slot.
		 * The data will be in host-endian format.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int save(_sa_save_slot *sa_save) = 0;
};
