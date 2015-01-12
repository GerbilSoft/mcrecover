/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SALevelStats.hpp: Sonic Adventure - Level Stats editor.                 *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SALEVELSTATS_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_SALEVELSTATS_HPP__

#include <QtGui/QWidget>

struct _sa_save_file;

class SALevelStatsPrivate;
class SALevelStats : public QWidget
{
	Q_OBJECT

	public:
		SALevelStats(QWidget *parent = 0);
		~SALevelStats();

	protected:
		SALevelStatsPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(SALevelStats)
	private:
		Q_DISABLE_COPY(SALevelStats)

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);

	protected slots:
		// Character was changed.
		void on_cboCharacter_currentIndexChanged(int index);
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SALEVELSTATS_HPP__ */
