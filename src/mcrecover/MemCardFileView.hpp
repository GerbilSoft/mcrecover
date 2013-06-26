/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * MemCardFileView.hpp: MemCardFile view widget.                           *
 *                                                                         *
 * Copyright (c) 2012-2013 by David Korth.                                 *
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

#ifndef __MCRECOVER_MEMCARDFILEVIEW_HPP__
#define __MCRECOVER_MEMCARDFILEVIEW_HPP__

#include <QtGui/QWidget>
#include "ui_MemCardFileView.h"

// MemCardFile class.
class MemCardFile;

class MemCardFileViewPrivate;

class MemCardFileView : public QWidget, public Ui::MemCardFileView
{
	Q_OBJECT
	
	public:
		MemCardFileView(QWidget *parent = 0);
		~MemCardFileView();

	private:
		friend class MemCardFileViewPrivate;
		MemCardFileViewPrivate *const d;
		Q_DISABLE_COPY(MemCardFileView);

	public:
		/**
		 * Get the MemCardFile being displayed.
		 * @return MemCardFile.
		 */
		const MemCardFile *file(void) const;

		/**
		 * Set the MemCardFile being displayed.
		 * @param file MemCardFile.
		 */
		void setFile(const MemCardFile *file);

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);

	protected slots:
		/**
		 * MemCardFile object was destroyed.
		 * @param obj QObject that was destroyed.
		 */
		void memCardFile_destroyed_slot(QObject *obj = 0);

		/**
		 * Animation timer slot.
		 */
		void animTimer_slot(void);
};

#endif /* __MCRECOVER_MEMCARDFILEVIEW_HPP__ */
