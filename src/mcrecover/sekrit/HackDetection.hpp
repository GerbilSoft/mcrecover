/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * HackDetection.hpp: * HACK DETECTION *                                   *
 *                                                                         *
 * Copyright (c) 2013-2018 by David Korth.                                 *
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

#ifndef __MCRECOVER_HACKDETECTION_HPP__
#define __MCRECOVER_HACKDETECTION_HPP__

#include <QWidget>

class HackDetectionPrivate;
class HackDetection : public QWidget
{
	Q_OBJECT

	public:
		/**
		 * Create a Hack Detection window.
		 * Uses the default screen.
		 * @param parent Parent.
		 */
		explicit HackDetection(QWidget *parent = nullptr);

		/**
		 * Create a Hack Detection window.
		 * @param screen Screen index.
		 * @param parent Parent.
		 */
		HackDetection(int screen, QWidget *parent = nullptr);

		~HackDetection();

	protected:
		/**
		 * Initialize the Hack Detection window.
		 * (Called from the constructor.)
		 * @param screen Screen index.
		 */
		void init(int screen);

	public:
		enum DetectType {
			DT_NONE,
			DT_H,
			DT_Q,
			DT_S,

			DT_MAX
		};
		DetectType detectType(void) const;
		void setDetectType(DetectType detectType);

	protected:
		HackDetectionPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(HackDetection)
	private:
		Q_DISABLE_COPY(HackDetection)

	public:
		// Size hints
		QSize minimumSizeHint(void) const final;
		QSize sizeHint(void) const final;

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event) final;

		/**
		 * Show event.
		 * @param event QShowEvent.
		 */
		void showEvent(QShowEvent *event) final;

		/**
		 * Paint event.
		 * @param event QPaintEvent.
		 */
		void paintEvent(QPaintEvent *event) final;

		/**
		 * Close event.
		 * @param event QCloseEvent.
		 */
		void closeEvent(QCloseEvent *event) final;

		/**
		 * Key press event.
		 * @param event QKeyEvent.
		 */
		void keyPressEvent(QKeyEvent *event) final;

	protected slots:
		/**
		 * "Allow Escape" / Blink timer has expired.
		 */
		void tmrEscapeBlink_timeout(void);
};

#endif /* __MCRECOVER_HACKDETECTION_HPP__ */
