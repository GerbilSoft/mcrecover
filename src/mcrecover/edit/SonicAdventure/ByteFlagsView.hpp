/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * ByteFlagsView.hpp: Byte Flags editor.                                   *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_BYTEFLAGSVIEW_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_BYTEFLAGSVIEW_HPP__

#include <QtGui/QWidget>

class ByteFlagsModel;

class ByteFlagsViewPrivate;
class ByteFlagsView : public QWidget
{
	Q_OBJECT

	Q_PROPERTY(ByteFlagsModel* byteFlagsModel READ byteFlagsModel WRITE setByteFlagsModel)
	// TODO: Function to set tab titles.
	// TODO: Signal for pageSizeChanged()?
	Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize)

	public:
		ByteFlagsView(QWidget *parent = 0);
		~ByteFlagsView();

	protected:
		ByteFlagsViewPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(ByteFlagsView)
	private:
		Q_DISABLE_COPY(ByteFlagsView)

	public:
		/** Model access. **/

		/**
		 * Get the ByteFlagsModel this widget is editing.
		 * @return ByteFlagsModel.
		 */
		ByteFlagsModel *byteFlagsModel(void) const;

		/**
		 * Set the ByteFlagsModel to edit.
		 * @param byteFlagsModel ByteFlagsModel.
		 */
		void setByteFlagsModel(ByteFlagsModel *model);

		/** Data access. **/

		/**
		 * Get the page size.
		 * @return Page size.
		 */
		int pageSize(void) const;

		/**
		 * Set the page size.
		 * @param pageSize Page size.
		 */
		void setPageSize(int pageSize);

		// TODO: Page count?
		// TODO: Set tab names.
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_BYTEFLAGSVIEW_HPP__ */
