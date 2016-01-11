/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * TimeCodeEdit.hpp: Slot selection widget.                                *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_TIMECODEEDIT_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_TIMECODEEDIT_HPP__

#include <QtGui/QWidget>

// C includes.
#include <stdint.h>

struct _sa_time_code;

class TimeCodeEditPrivate;
class TimeCodeEdit : public QWidget
{
	Q_OBJECT

	// TODO: Register sa_time_code as a Qt type?
	//Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)

	public:
		TimeCodeEdit(QWidget *parent = 0);
		~TimeCodeEdit();

	protected:
		TimeCodeEditPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(TimeCodeEdit)
	private:
		Q_DISABLE_COPY(TimeCodeEdit)

	signals:
		/**
		 * Value has changed. (minutes, seconds, frames)
		 * @param minutes Minutes.
		 * @param seconds Seconds.
		 * @param frames Frames.
		 */
		void valueChanged(int minutes, int seconds, int frames);

		/**
		 * Value has changed. (hours)
		 * @param hours Hours.
		 */
		void valueChangedHours(int hours);

	public:
		/** Public functions. **/

		/**
		 * Set the minutes/seconds/frames using an sa_time_code.
		 *
		 * If hours are visible, and minutes is larger than 59,
		 * hours will be adjusted; if hours is larger than 99,
		 * hours will be clamped to 99.
		 *
		 * If hours are not visible, and minutes is larger than 99,
		 * minutes will be clamped to 99.
		 *
		 * @param time_code [in] sa_time_code.
		 */
		void setValue(const _sa_time_code *time_code);

		/**
		 * Get the minutes/seconds/frames as an sa_time_code.
		 * @param time_code [out] sa_time_code.
		 */
		void value(_sa_time_code *time_code) const;

		/**
		 * Set the time in NTSC frames. (1/60th of a second)
		 * @param ntscFrames Time in NTSC frames.
		 */
		void setValueInNtscFrames(uint32_t ntscFrames);

		/**
		 * Get the time in NTSC frames. (1/60th of a second)
		 * @return Time in NTSC frames.
		 */
		uint32_t valueInNtscFrames(void) const;

		/**
		 * Get the hours.
		 * @return Hours. (If hours is not visible, this will return 0.)
		 */
		int hours(void) const;

		/**
		 * Get the minutes.
		 * @return Minutes.
		 */
		int minutes(void) const;

		/**
		 * Get the seconds.
		 * @return Seconds.
		 */
		int seconds(void) const;

		/**
		 * Get the frames.
		 * @return Frames.
		 */
		int frames(void) const;

		/**
		 * Set the hours field visibility.
		 * @param showHours If true, show hours.
		 */
		void setShowHours(bool showHours);

		/**
		 * Is the hours field visible?
		 * @return True if it is; false if it isn't.
		 */
		bool isShowHours(void) const;

	public slots:
		/** Public slots. **/

		/**
		 * Set the minutes/seconds/frames.
		 *
		 * If hours are visible, and minutes is larger than 59,
		 * hours will be adjusted; if hours is larger than 99,
		 * hours will be clamped to 99.
		 *
		 * If hours are not visible, and minutes is larger than 99,
		 * minutes will be clamped to 99.
		 *
		 * @param minutes [in] Minutes.
		 * @param seconds [in] Seconds.
		 * @param frames  [in] Frames.
		 */
		void setValue(int minutes, int seconds, int frames);

		/**
		 * Set the hours value.
		 *
		 * If hours isn't visible, nothing will be done.
		 *
		 * @param hours [in] Hours.
		 */
		void setValueHours(int hours);

	protected slots:
		/**
		 * One of the minutes/seconds/frames spinboxes has been changed.
		 */
		void spinMSFChanged(void);

		/**
		 * The hours spinbox has been changed.
		 */
		void spinHoursChanged(void);
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_TIMECODEEDIT_HPP__ */
