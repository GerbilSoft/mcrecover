/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * ByteFlagsView.hpp: Byte Flags editor.                                   *
 *                                                                         *
 * Copyright (c) 2015-2016 by David Korth.                                 *
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

#ifndef __MCRECOVER_EDIT_MODELS_BYTEFLAGSVIEW_HPP__
#define __MCRECOVER_EDIT_MODELS_BYTEFLAGSVIEW_HPP__

#include <QWidget>

class ByteFlagsModel;

class ByteFlagsViewPrivate;
class ByteFlagsView : public QWidget
{
	Q_OBJECT

	Q_PROPERTY(ByteFlagsModel* byteFlagsModel READ byteFlagsModel WRITE setByteFlagsModel)
	Q_PROPERTY(int pageSize READ pageSize)

	public:
		explicit ByteFlagsView(QWidget *parent = 0);
		~ByteFlagsView();

	private:
		typedef QWidget super;
		ByteFlagsViewPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(ByteFlagsView)
		Q_DISABLE_COPY(ByteFlagsView)

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		virtual void changeEvent(QEvent *event) final;

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

		// TODO: Page count?
};

#endif /* __MCRECOVER_EDIT_MODELS_BYTEFLAGSVIEW_HPP__ */
