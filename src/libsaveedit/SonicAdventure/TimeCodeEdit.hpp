/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * TimeCodeEdit.hpp: sa_time_code editor widget.                           *
 *                                                                         *
 * Copyright (c) 2016-2018 by David Korth.                                 *
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
 * You should have received a copy of the GNU General Public License       *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 ***************************************************************************/

#ifndef __LIBSAVEEDIT_SONICADVENTURE_TIMECODEEDIT_HPP__
#define __LIBSAVEEDIT_SONICADVENTURE_TIMECODEEDIT_HPP__

#include <QtCore/QMetaType>
#include <QWidget>

// C includes.
#include <stdint.h>

struct _sa_time_code;

class TimeCodeEditPrivate;
class TimeCodeEdit : public QWidget
{
	Q_OBJECT

	Q_ENUMS(DisplayMode)

	// TODO: Register sa_time_code as a Qt type?
	//Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
	Q_PROPERTY(DisplayMode displayMode READ displayMode WRITE setDisplayMode NOTIFY displayModeChanged)

	public:
		explicit TimeCodeEdit(QWidget *parent = 0);
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
		 * Display mode.
		 */
		enum DisplayMode {
			DM_MSF,		// Minutes, Seconds, Frames (1/60th)
			DM_HMSF,	// Hours, Minutes, Seconds, Frames
			DM_WEIGHT,	// Weight (for Big the Cat)
			DM_MSC,		// Minutes, Seconds, Centiseconds

			// TODO: Scores? (show 3 scores at once)
			// TODO: Register as a Qt enum?
		};

		/**
		 * Set the minutes/seconds/frames using an sa_time_code.
		 *
		 * If DM_MSF or DM_MSC, and minutes is larger than 99,
		 * minutes will be clamped to 99.
		 *
		 * If DM_HMSF, and minutes is larger than 59, hours will
		 * be adjusted. If hours is larger than 99, hours will
		 * be clamped to 99.
		 *
		 * If DM_WEIGHT, this function will do nothing.
		 *
		 * @param time_code [in] sa_time_code.
		 */
		void setValue(const _sa_time_code *time_code);

		/**
		 * Get the minutes/seconds/frames as an sa_time_code.
		 *
		 * If DM_WEIGHT, this function will do nothing.
		 *
		 * @param time_code [out] sa_time_code.
		 */
		void value(_sa_time_code *time_code) const;

		/**
		 * Set the three weights.
		 *
		 * The three values are the weight divided by 10.
		 * Range: [0, 65535]
		 *
		 * If not DM_WEIGHT, this function will do nothing.
		 *
		 * @param weights [in] Array of 3 uint16_t weight values.
		 */
		void setValue(const uint16_t weights[3]);

		/**
		 * Get the three weights.
		 *
		 * If not DM_WEIGHT, this function will do nothing.
		 *
		 * @param weights [out] Array of 3 uint16_t to put the weights in.
		 */
		void value(uint16_t weights[3]) const;

		/**
		 * Set the time in NTSC frames. (1/60th of a second)
		 *
		 * This only works in DM_MSF and DM_HMSF.
		 * TODO: Convert to centiseconds for DM_MSC?
		 *
		 * @param ntscFrames Time in NTSC frames.
		 */
		void setValueInNtscFrames(uint32_t ntscFrames);

		/**
		 * Get the time in NTSC frames. (1/60th of a second)
		 *
		 * If not DM_MSF or DM_HMSF, this function will return 0.
		 * TODO: Convert from centiseconds for DM_MSC?
		 *
		 * @return Time in NTSC frames.
		 */
		uint32_t valueInNtscFrames(void) const;

		/**
		 * Get the hours.
		 *
		 * If not DM_HMSF, this function will return 0.
		 *
		 * @return Hours.
		 */
		int hours(void) const;

		/**
		 * Get the minutes.
		 *
		 * If not DM_MSF, DM_HMSF, or DM_MSC, this function will return 0.
		 *
		 * @return Minutes.
		 */
		int minutes(void) const;

		/**
		 * Get the seconds.
		 *
		 * If not DM_MSF, DM_HMSF, or DM_MSC, this function will return 0.
		 *
		 * @return Seconds.
		 */
		int seconds(void) const;

		/**
		 * Get the frames.
		 *
		 * If not DM_MSF, DM_HMSF, or DM_MSC, this function will return 0.
		 * TODO: Convert from centiseconds for DM_MSC?
		 *
		 * @return Frames.
		 */
		int frames(void) const;

		/**
		 * Set the display mode.
		 * @param displayMode New display mode.
		 */
		void setDisplayMode(DisplayMode displayMode);

		/**
		 * Get the display mode.
		 * @return Display mode.
		 */
		DisplayMode displayMode(void) const;

	public slots:
		/** Public slots. **/

		/**
		 * Set the minutes/seconds/frames.
		 *
		 * If DM_MSF or DM_MSC, and minutes is larger than 99,
		 * minutes will be clamped to 99.
		 *
		 * If DM_HMSF, and minutes is larger than 59, hours will
		 * be adjusted. If hours is larger than 99, hours will
		 * be clamped to 99.
		 *
		 * If DM_WEIGHT, this function will do nothing.
		 *
		 * @param minutes [in] Minutes.
		 * @param seconds [in] Seconds.
		 * @param frames  [in] Frames.
		 */
		void setValue(int minutes, int seconds, int frames);

		/**
		 * Set the hours value.
		 *
		 * If not DM_HMSF, this function will do nothing
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

	signals:
		/**
		 * displayMode has changed.
		 * @param displayMode New displayMode value.
		 */
		void displayModeChanged(DisplayMode displayMode);
};

Q_DECLARE_METATYPE(TimeCodeEdit::DisplayMode)

#endif /* __LIBSAVEEDIT_SONICADVENTURE_TIMECODEEDIT_HPP__ */
