/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * TimeCodeEdit.cpp: sa_time_code editor widget.                           *
 *                                                                         *
 * Copyright (c) 2016 by David Korth.                                      *
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

#include "TimeCodeEdit.hpp"

// Qt widgets.
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QButtonGroup>
#include <QtCore/QSignalMapper>

// Sonic Adventure save file definitions.
#include "sa_defs.h"

/** TimeCodeEditPrivate **/

#include "ui_TimeCodeEdit.h"
class TimeCodeEditPrivate
{
	public:
		TimeCodeEditPrivate(TimeCodeEdit *q);
		~TimeCodeEditPrivate();

	protected:
		TimeCodeEdit *const q_ptr;
		Q_DECLARE_PUBLIC(TimeCodeEdit)
	private:
		Q_DISABLE_COPY(TimeCodeEditPrivate)

	public:
		Ui_TimeCodeEdit ui;

		// NOTE: spnHours->isVisible() doesn't work if the
		// window isn't visible, so we're storing the
		// hours visibility property here.
		bool showHours;

		// Suppress signals when modifying the QSpinBoxes.
		bool suppressSignals;
};

TimeCodeEditPrivate::TimeCodeEditPrivate(TimeCodeEdit *q)
	: q_ptr(q)
	, showHours(false)
	, suppressSignals(false)
{ }

TimeCodeEditPrivate::~TimeCodeEditPrivate()
{ }

/** TimeCodeEdit **/

TimeCodeEdit::TimeCodeEdit(QWidget *parent)
	: QWidget(parent)
	, d_ptr(new TimeCodeEditPrivate(this))
{
	Q_D(TimeCodeEdit);
	d->ui.setupUi(this);

	// Don't show the hours field by default.
	d->ui.spnHours->hide();

	// Connect additional signals.
	// FIXME: Qt Designer won't let me connect a signal
	// from a QSpinBox to a TimeCodeEdit signal, so I
	// have to do it here.
	connect(d->ui.spnHours, SIGNAL(valueChanged(int)),
		this, SIGNAL(valueChangedHours(int)));
}

TimeCodeEdit::~TimeCodeEdit()
{
	Q_D(TimeCodeEdit);
	delete d;
}

/** Public functions. **/

/**
 * Set the minutes/seconds/frames using an sa_time_code.
 *
 * If the value is out of range, nothing will be done.
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
void TimeCodeEdit::setValue(const sa_time_code *time_code)
{
	Q_D(TimeCodeEdit);

	// Validate the time code first.
	if (time_code->seconds > 59 || time_code->frames > 59) {
		// Time code is invalid.
		return;
	}

	// NOTE: Suppressing signals this way is not thread-safe.
	d->suppressSignals = true;

	if (d->showHours) {
		// Handle more than 59 minutes as hours.
		// TODO: If more than 99 hours, this will be clamped.
		int hours = time_code->minutes / 60;
		d->ui.spnHours->setValue(hours);
		d->ui.spnMinutes->setValue(time_code->minutes % 60);
	} else {
		// No special processing for minutes.
		d->ui.spnMinutes->setValue(time_code->minutes);
	}

	d->ui.spnSeconds->setValue(time_code->seconds);
	// TODO: Convert to 1/100ths of a second?
	d->ui.spnFrames->setValue(time_code->frames);

	// Allow signals.
	d->suppressSignals = false;
}

/**
 * Get the minutes/seconds/frames as an sa_time_code.
 * @param time_code [out] sa_time_code.
 */
void TimeCodeEdit::value(sa_time_code *time_code) const
{
	Q_D(const TimeCodeEdit);

	time_code->minutes = d->ui.spnMinutes->value();
	if (d->showHours) {
		// Include hours in the time code.
		time_code->minutes += (d->ui.spnHours->value() * 60);
	}

	time_code->seconds = d->ui.spnSeconds->value();
	time_code->frames = d->ui.spnFrames->value();
}

/**
 * Set the time in NTSC frames. (1/60th of a second)
 * @param ntscFrames Time in NTSC frames.
 */
void TimeCodeEdit::setValueInNtscFrames(uint32_t ntscFrames)
{
	Q_D(TimeCodeEdit);

	d->suppressSignals = true;
	d->ui.spnFrames->setValue(ntscFrames % 60);
	ntscFrames /= 60;
	d->ui.spnSeconds->setValue(ntscFrames % 60);
	ntscFrames /= 60;

	// FIXME: Value may be too large for these fields...
	if (d->showHours) {
		// Set hours and minutes.
		d->ui.spnMinutes->setValue(ntscFrames % 60);
		ntscFrames /= 60;
		d->ui.spnHours->setValue(ntscFrames);
	} else {
		// Set minutes.
		d->ui.spnMinutes->setValue(ntscFrames);
	}

	d->suppressSignals = false;
}

/**
 * Get the time in NTSC frames. (1/60th of a second)
 * @return Time in NTSC frames.
 */
uint32_t TimeCodeEdit::valueInNtscFrames(void) const
{
	Q_D(const TimeCodeEdit);

	uint32_t ntscFrames;
	ntscFrames  =  d->ui.spnFrames->value();
	ntscFrames += (d->ui.spnSeconds->value() * 60);
	ntscFrames += (d->ui.spnMinutes->value() * 60 * 60);
	if (d->showHours) {
		ntscFrames += (d->ui.spnHours->value() * 60 * 60 * 60);
	}
	return ntscFrames;
}

// TODO: Add setHours(), setMinutes(), setSeconds(), and setFrames()?

/**
 * Get the hours.
 * @return Hours. (If hours is not visible, this will return 0.)
 */
int TimeCodeEdit::hours(void) const
{
	Q_D(const TimeCodeEdit);
	if (d->showHours) {
		return d->ui.spnHours->value();
	}
	return 0;
}

/**
 * Get the minutes.
 * @return Minutes.
 */
int TimeCodeEdit::minutes(void) const
{
	Q_D(const TimeCodeEdit);
	return d->ui.spnMinutes->value();
}

/**
 * Get the seconds.
 * @return Seconds.
 */
int TimeCodeEdit::seconds(void) const
{
	Q_D(const TimeCodeEdit);
	return d->ui.spnSeconds->value();
}

/**
 * Get the frames.
 * @return Frames.
 */
int TimeCodeEdit::frames(void) const
{
	Q_D(const TimeCodeEdit);
	return d->ui.spnFrames->value();
}

/**
 * Set the hours field visibility.
 * @param showHours If true, show hours.
 */
void TimeCodeEdit::setShowHours(bool showHours)
{
	Q_D(TimeCodeEdit);
	if (d->showHours == showHours)
		return;
	d->showHours = showHours;

	d->suppressSignals = true;
	if (showHours) {
		// Show the hours field.
		int minutes = d->ui.spnMinutes->value();
		if (minutes > 59) {
			d->ui.spnHours->setValue(minutes % 60);
			d->ui.spnMinutes->setValue(minutes / 60);
			d->ui.spnMinutes->setMaximum(59);
			// TODO: Emit valueChanged()?
		}
		d->ui.spnHours->show();
	} else {
		// Hide the hours field.
		d->ui.spnMinutes->setMaximum(99);
		int hours = d->ui.spnHours->value();
		if (hours > 0) {
			int minutes = d->ui.spnMinutes->value();
			minutes += (hours * 60);
			d->ui.spnMinutes->setValue(minutes);
		}
		d->ui.spnHours->hide();
	}
	d->suppressSignals = false;
}

/**
 * Is the hours field visible?
 * @return True if it is; false if it isn't.
 */
bool TimeCodeEdit::isShowHours(void) const
{
	Q_D(const TimeCodeEdit);
	return d->showHours;
}

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
void TimeCodeEdit::setValue(int minutes, int seconds, int frames)
{
	sa_time_code time_code;
	time_code.minutes = minutes;
	time_code.seconds = seconds;
	time_code.frames = frames;
	setValue(&time_code);
}

/**
 * Set the hours value.
 *
 * If hours isn't visible, nothing will be done.
 *
 * @param hours [in] Hours.
 */
void TimeCodeEdit::setValueHours(int hours)
{
	Q_D(TimeCodeEdit);
	if (!d->showHours)
		return;

	d->suppressSignals = true;
	d->ui.spnHours->setValue(hours);
	d->suppressSignals = false;
}

/** Protected slots. **/

/**
 * One of the minutes/seconds/frames spinboxes has been changed.
 */
void TimeCodeEdit::spinMSFChanged(void)
{
	Q_D(const TimeCodeEdit);
	if (!d->suppressSignals) {
		emit valueChanged(d->ui.spnMinutes->value(),
				  d->ui.spnSeconds->value(),
				  d->ui.spnFrames->value());
	}
}

/**
 * The hours spinbox has been changed.
 */
void TimeCodeEdit::spinHoursChanged(void)
{
	Q_D(const TimeCodeEdit);
	if (!d->suppressSignals) {
		emit valueChangedHours(d->ui.spnHours->value());
	}
}
