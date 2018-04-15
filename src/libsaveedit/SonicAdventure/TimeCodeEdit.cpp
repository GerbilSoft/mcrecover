/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * TimeCodeEdit.cpp: sa_time_code editor widget.                           *
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

#include "TimeCodeEdit.hpp"

// Sonic Adventure save file definitions.
#include "sa_defs.h"

/** TimeCodeEditPrivate **/

#include "ui_TimeCodeEdit.h"
class TimeCodeEditPrivate
{
	public:
		explicit TimeCodeEditPrivate(TimeCodeEdit *q);
		~TimeCodeEditPrivate();

	protected:
		TimeCodeEdit *const q_ptr;
		Q_DECLARE_PUBLIC(TimeCodeEdit)
	private:
		Q_DISABLE_COPY(TimeCodeEditPrivate)

	public:
		Ui_TimeCodeEdit ui;

		/**
		 * Current display mode.
		 */
		TimeCodeEdit::DisplayMode displayMode;

		// Suppress signals when modifying the QSpinBoxes.
		bool suppressSignals;

		/**
		 * Update the display mode.
		 * @param displayMode New display mode.
		 */
		void updateDisplayMode(TimeCodeEdit::DisplayMode displayMode);
};

TimeCodeEditPrivate::TimeCodeEditPrivate(TimeCodeEdit *q)
	: q_ptr(q)
	, displayMode(TimeCodeEdit::DM_MSF)
	, suppressSignals(false)
{ }

TimeCodeEditPrivate::~TimeCodeEditPrivate()
{ }

/**
 * Update the display mode.
 * @param displayMode New display mode.
 */
void TimeCodeEditPrivate::updateDisplayMode(TimeCodeEdit::DisplayMode displayMode)
{
	if (this->displayMode == displayMode)
		return;
	suppressSignals = true;

	// TODO: DM_MSF, DM_HMSF should both display
	// frames in centiseconds, with conversions that
	// match SADX itself.
	switch (displayMode) {
		case TimeCodeEdit::DM_MSF:
		default:
			// Minutes, Seconds, Frames.
			ui.spnHours->hide();
			ui.spnMinutes->setRange(0, 99);
			ui.spnMinutes->setSingleStep(1);
			ui.spnMinutes->setSuffix(QString());
			ui.spnSeconds->setRange(0, 59);
			ui.spnSeconds->setSingleStep(1);
			ui.spnSeconds->setSuffix(QString());
			ui.spnFrames->setRange(0, 59);
			ui.spnFrames->setSingleStep(1);
			ui.spnFrames->setSuffix(QString());

			if (this->displayMode == TimeCodeEdit::DM_HMSF) {
				// Switching from DM_HMSF to DM_MSF.
				// Attempt to add the hours to the minutes.
				int hours = ui.spnHours->value();
				if (hours > 0) {
					int minutes = ui.spnMinutes->value();
					minutes += (hours * 60);
					ui.spnMinutes->setValue(minutes);
				}
				// TODO: Emit valueChanged()?
			}
			break;

		case TimeCodeEdit::DM_HMSF:
			// Hours, Minutes, Seconds, Frames.
			ui.spnHours->setRange(0, 11930);
			ui.spnHours->setSingleStep(1);
			ui.spnHours->setSuffix(QString());
			if (this->displayMode == TimeCodeEdit::DM_MSF ||
			    this->displayMode == TimeCodeEdit::DM_MSC)
			{
				// Switching from DM_MSF or DM_MSC to DM_HMSF.
				// Convert minutes into minutes and hours.
				int minutes = ui.spnMinutes->value();
				if (minutes > 59) {
					ui.spnHours->setValue(minutes % 60);
					ui.spnMinutes->setValue(minutes / 60);
					// TODO: Emit valueChanged()?
				}
			}
			ui.spnHours->show();

			ui.spnMinutes->setRange(0, 59);
			ui.spnMinutes->setSingleStep(1);
			ui.spnMinutes->setSuffix(QString());
			ui.spnSeconds->setRange(0, 59);
			ui.spnSeconds->setSingleStep(1);
			ui.spnSeconds->setSuffix(QString());
			ui.spnFrames->setRange(0, 59);
			ui.spnFrames->setSingleStep(1);
			ui.spnFrames->setSuffix(QString());
			break;

		case TimeCodeEdit::DM_MSC:
			// Minutes, Seconds, Centiseconds.
			ui.spnHours->hide();
			ui.spnMinutes->setRange(0, 99);
			ui.spnMinutes->setSingleStep(1);
			ui.spnMinutes->setSuffix(QString());
			ui.spnSeconds->setRange(0, 59);
			ui.spnSeconds->setSingleStep(1);
			ui.spnSeconds->setSuffix(QString());
			ui.spnFrames->setRange(0, 99);
			ui.spnFrames->setSingleStep(1);
			ui.spnFrames->setSuffix(QString());

			if (this->displayMode == TimeCodeEdit::DM_HMSF) {
				// Switching from DM_HMSF to DM_MSC.
				// Attempt to add the hours to the minutes.
				int hours = ui.spnHours->value();
				if (hours > 0) {
					int minutes = ui.spnMinutes->value();
					minutes += (hours * 60);
					ui.spnMinutes->setValue(minutes);
				}
				// TODO: Emit valueChanged()?
			}
			break;

		case TimeCodeEdit::DM_WEIGHT:
			// Weights. (for Big the Cat)
			ui.spnHours->hide();

			// Add suffixes to indicate that the values are weights.
			// NOTE: Conversion from DM_MSF, DM_HMSF, and DM_MSC
			// is impossible, so unless the widget owner sets
			// the value afterwards, the display will be undefined.
			QString suffix = QLatin1String("g");
			ui.spnMinutes->setRange(0, 655350);
			ui.spnMinutes->setSingleStep(10);
			ui.spnMinutes->setSuffix(suffix);
			ui.spnSeconds->setRange(0, 655350);
			ui.spnSeconds->setSingleStep(10);
			ui.spnSeconds->setSuffix(suffix);
			ui.spnFrames->setRange(0, 655350);
			ui.spnFrames->setSingleStep(10);
			ui.spnFrames->setSuffix(suffix);
			break;
	}

	this->displayMode = displayMode;
	suppressSignals = false;
}

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
void TimeCodeEdit::setValue(const sa_time_code *time_code)
{
	Q_D(TimeCodeEdit);
	if (d->displayMode == DM_WEIGHT) {
		// Display is in weight mode.
		return;
	}

	// Validate the time code first.
	if (time_code->seconds > 59 ||
	    time_code->frames > d->ui.spnFrames->maximum())
	{
		// Time code is invalid.
		return;
	}

	// NOTE: Suppressing signals this way is not thread-safe.
	d->suppressSignals = true;

	if (d->displayMode == DM_HMSF) {
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
	// TODO: For DM_MSF, convert frames to centiseconds.
	d->ui.spnFrames->setValue(time_code->frames);

	// Allow signals.
	d->suppressSignals = false;
}

/**
 * Get the minutes/seconds/frames as an sa_time_code.
 *
 * If DM_WEIGHT, this function will do nothing.
 *
 * @param time_code [out] sa_time_code.
 */
void TimeCodeEdit::value(sa_time_code *time_code) const
{
	Q_D(const TimeCodeEdit);
	if (d->displayMode == DM_WEIGHT) {
		// Display is in weight mode.
		return;
	}

	time_code->minutes = d->ui.spnMinutes->value();
	if (d->displayMode == DM_HMSF) {
		// Include hours in the time code.
		time_code->minutes += (d->ui.spnHours->value() * 60);
	}

	time_code->seconds = d->ui.spnSeconds->value();
	time_code->frames = d->ui.spnFrames->value();
}

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
void TimeCodeEdit::setValue(const uint16_t weights[3])
{
	Q_D(TimeCodeEdit);
	if (d->displayMode != DM_WEIGHT) {
		// Display is not in weight mode.
		return;
	}

	// NOTE: Suppressing signals this way is not thread-safe.
	d->suppressSignals = true;

	// Set the weights.
	// TODO: Is the casting required?
	d->ui.spnMinutes->setValue((uint32_t)weights[0] * 10);
	d->ui.spnSeconds->setValue((uint32_t)weights[1] * 10);
	d->ui.spnFrames->setValue((uint32_t)weights[2] * 10);

	d->suppressSignals = false;
}

/**
 * Get the three weights.
 *
 * If not DM_WEIGHT, this function will do nothing.
 *
 * @param weights [out] Array of 3 uint16_t to put the weights in.
 */
void TimeCodeEdit::value(uint16_t weights[3]) const
{
	Q_D(const TimeCodeEdit);
	if (d->displayMode != DM_WEIGHT) {
		// Display is not in weight mode.
		return;
	}

	// Get the weights.
	weights[0] = d->ui.spnMinutes->value() / 10;
	weights[1] = d->ui.spnSeconds->value() / 10;
	weights[2] = d->ui.spnFrames->value() / 10;
}

/**
 * Set the time in NTSC frames. (1/60th of a second)
 *
 * This only works in DM_MSF and DM_HMSF.
 * TODO: Convert to centiseconds for DM_MSC?
 *
 * @param ntscFrames Time in NTSC frames.
 */
void TimeCodeEdit::setValueInNtscFrames(uint32_t ntscFrames)
{
	Q_D(TimeCodeEdit);
	if (d->displayMode == DM_WEIGHT) {
		// Display is in weight mode.
		return;
	}

	d->suppressSignals = true;
	d->ui.spnFrames->setValue(ntscFrames % 60);
	ntscFrames /= 60;
	d->ui.spnSeconds->setValue(ntscFrames % 60);
	ntscFrames /= 60;

	// FIXME: Value may be too large for these fields...
	if (d->displayMode == DM_HMSF) {
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
 *
 * If not DM_MSF or DM_HMSF, this function will return 0.
 * TODO: Convert from centiseconds for DM_MSC?
 *
 * @return Time in NTSC frames.
 */
uint32_t TimeCodeEdit::valueInNtscFrames(void) const
{
	Q_D(const TimeCodeEdit);
	if (d->displayMode == DM_WEIGHT) {
		// Display is in weight mode.
		return 0;
	}

	uint32_t ntscFrames;
	ntscFrames  =  d->ui.spnFrames->value();
	ntscFrames += (d->ui.spnSeconds->value() * 60);
	ntscFrames += (d->ui.spnMinutes->value() * 60 * 60);
	if (d->displayMode == DM_HMSF) {
		ntscFrames += (d->ui.spnHours->value() * 60 * 60 * 60);
	}
	return ntscFrames;
}

// TODO: Add setHours(), setMinutes(), setSeconds(), and setFrames()?

/**
 * Get the hours.
 *
 * If not DM_HMSF, this function will return 0.
 *
 * @return Hours.
 */
int TimeCodeEdit::hours(void) const
{
	Q_D(const TimeCodeEdit);
	if (d->displayMode == DM_HMSF) {
		return d->ui.spnHours->value();
	}
	return 0;
}

/**
 * Get the minutes.
 *
 * If not DM_MSF, DM_HMSF, or DM_MSC, this function will return 0.
 *
 * @return Minutes.
 */
int TimeCodeEdit::minutes(void) const
{
	Q_D(const TimeCodeEdit);
	if (d->displayMode != DM_WEIGHT) {
		return d->ui.spnMinutes->value();
	}
	return 0;
}

/**
 * Get the seconds.
 *
 * If not DM_MSF, DM_HMSF, or DM_MSC, this function will return 0.
 *
 * @return Seconds.
 */
int TimeCodeEdit::seconds(void) const
{
	Q_D(const TimeCodeEdit);
	if (d->displayMode != DM_WEIGHT) {
		return d->ui.spnSeconds->value();
	}
	return 0;
}

/**
 * Get the frames.
 *
 * If not DM_MSF, DM_HMSF, or DM_MSC, this function will return 0.
 * TODO: Convert from centiseconds for DM_MSC?
 *
 * @return Frames.
 */
int TimeCodeEdit::frames(void) const
{
	Q_D(const TimeCodeEdit);
	if (d->displayMode != DM_WEIGHT) {
		return d->ui.spnFrames->value();
	}
	return 0;
}

/**
 * Set the display mode.
 * @param displayMode New display mode.
 */
void TimeCodeEdit::setDisplayMode(DisplayMode displayMode)
{
	Q_D(TimeCodeEdit);
	// FIXME: d->updateDisplayMode() also checks this...
	if (d->displayMode == displayMode)
		return;
	d->updateDisplayMode(displayMode);
	emit displayModeChanged(displayMode);
}

/**
 * Get the display mode.
 * @return Display mode.
 */
TimeCodeEdit::DisplayMode TimeCodeEdit::displayMode(void) const
{
	Q_D(const TimeCodeEdit);
	return d->displayMode;
}

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
 * If not DM_HMSF, this function will do nothing
 *
 * @param hours [in] Hours.
 */
void TimeCodeEdit::setValueHours(int hours)
{
	Q_D(TimeCodeEdit);
	if (d->displayMode != DM_HMSF)
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
		// TODO: Weight version?
		if (d->displayMode != DM_WEIGHT) {
			emit valueChanged(d->ui.spnMinutes->value(),
					  d->ui.spnSeconds->value(),
					  d->ui.spnFrames->value());
		}
	}
}

/**
 * The hours spinbox has been changed.
 */
void TimeCodeEdit::spinHoursChanged(void)
{
	Q_D(const TimeCodeEdit);
	if (!d->suppressSignals) {
		// TODO: Weight version?
		if (d->displayMode != DM_WEIGHT) {
			emit valueChangedHours(d->ui.spnHours->value());
		}
	}
}
