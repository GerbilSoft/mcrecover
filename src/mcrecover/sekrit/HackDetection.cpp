/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * HackDetection.cpp: * HACK DETECTION *                                   *
 *                                                                         *
 * Copyright (c) 2013-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "HackDetection.hpp"
#include "util/array_size.h"

// C includes.
#include <stdint.h>

// Qt includes.
#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtGui/QCloseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QRawFont>
#include <QtGui/QScreen>
#include <QApplication>

/** HackDetectionPrivate **/

class HackDetectionPrivate
{
public:
	explicit HackDetectionPrivate(HackDetection *q);

protected:
	HackDetection *const q_ptr;
	Q_DECLARE_PUBLIC(HackDetection)
private:
	Q_DISABLE_COPY(HackDetectionPrivate)

public:
	QRect winRect;

	// Screen index
	int screenIdx;
	void setScreen(int screenIdx);

	// Font
	QFont fntHack;
	QFont fntHackText;
	QFont fntHackStar;
	int hMargin;
	QChar chrStar;
	QPoint drpTranslate;
	void initFont(void);

	// "Hack Detection" message
	HackDetection::DetectType detectType;
	QString hdTitle;
	QString hdMessage;
	QString hdCloser;
	void initMessage(void);

	// There is no escape. Muahahahaha.
	bool allowEscape;
	bool escapeBlink;
	QTimer *tmrEscapeBlink;

	static constexpr int ESCAPE_TIMER = 10000;
	static constexpr int BLINK_TIMER = 500;
};

HackDetectionPrivate::HackDetectionPrivate(HackDetection* q)
	: q_ptr(q)
	, screenIdx(0)
	, hMargin(0)
	, detectType(HackDetection::DetectType::None)
	, allowEscape(false)
	, escapeBlink(false)
	, tmrEscapeBlink(new QTimer(q))
{
	// Assume 640x480 for now.
	winRect = QRect(0, 0, 640, 480);

	// Connect the timer signal.
	QObject::connect(tmrEscapeBlink, &QTimer::timeout,
		q, &HackDetection::tmrEscapeBlink_timeout);
}

/**
 * Set the screen.
 * @param screenIdx Screen index
 */
void HackDetectionPrivate::setScreen(int screenIdx)
{
	QList<QScreen*> screens = QGuiApplication::screens();
	QScreen *screen;
	if (screenIdx >= 0 && screenIdx < screens.size()) {
		screen = screens[screenIdx];
	} else {
		// Invalid. Assume the default screen.
		screen = QGuiApplication::primaryScreen();
	}

	this->screenIdx = screenIdx;

	// Get the screen dimensions.
	// NOTE: Don't get the desktop->screen(), since this may
	// be the rectangle for all monitors combined (at least
	// this is the case on Windows).
	winRect = screen->geometry();
}

/**
 * Initialize the font.
 */
void HackDetectionPrivate::initFont(void)
{
	/**
	 * Determine the font to use.
	 *
	 * Preferences:
	 * 1. DejaVu Sans Mono
	 * 2. Fixedsys
	 * 3. Courier New
	 */
	struct FontNameInfo {
		const char *name;
		uint8_t num;	// Size multiplier: Numerator
		uint8_t denom;	// Size multiplier: Denominator
		bool bold;	// Use bold?
		bool isFixedSys; // Fixedsys hack (Non-TTF only!)
	};
	static const FontNameInfo FontNames[] = {
		{"DejaVu Sans Mono", 1, 1, true, false},
		{"Consolas", 1, 1, true, false},
		{"Lucida Console", 1, 1, true, false},
		{"Fixedsys Excelsior 3.01", 7, 6, false, false},
		{"Fixedsys Excelsior 3.00", 7, 6, false, false},
		{"Fixedsys Excelsior 3.0", 7, 6, false, false},
		{"Fixedsys Excelsior 2.00", 7, 6, false, false},
		{"Fixedsys Excelsior 2.0", 7, 6, false, false},
		{"Fixedsys Excelsior 1.00", 7, 6, false, false},
		{"Fixedsys Excelsior 1.0", 7, 6, false, false},
		{"Fixedsys", 2, 3, false, true},
		{"Courier New", 1, 1, true, false},
	};

	int fontIdx = -1;
	for (int i = 0; i < ARRAY_SIZE(FontNames); i++) {
		fntHack = QFont(QLatin1String(FontNames[i].name));
		fntHack.setStyleHint(QFont::TypeWriter);
		if (fntHack.exactMatch()) {
			fontIdx = i;
			break;
		}
	}

	if (!fntHack.exactMatch() || fontIdx == -1) {
		// Cannot find an exact match.
		// Use the system default Monospace font.
		fontIdx = -1;
		fntHack = QFont(QLatin1String("Monospace"));
		fntHack.setStyleHint(QFont::TypeWriter);
	}

	// Make the font bold.
	fntHack.setBold(fontIdx >= 0 ? FontNames[fontIdx].bold : true);

	/**
	 * With 640x480, the original "Hack Detection"
	 * used an 18px font. Calculate the font size
	 * relative to 18/480.
	 */
	int fntPx = (winRect.height() * 18 / 480);
	if (fontIdx >= 0) {
		fntPx = (fntPx * FontNames[fontIdx].num / FontNames[fontIdx].denom);
		if (FontNames[fontIdx].isFixedSys) {
			int mod = (fntPx % 9);
			fntPx += (9 - mod);
		}
	}
	fntHack.setPixelSize(fntPx);

	/**
	 * Drop shadow should be 2px for 18px font.
	 */
	int drpSz = (fntPx / 9);
	drpTranslate = QPoint(drpSz, drpSz);

	/**
	 * Horizontal margin was around 40px on 640x480.
	 * Calculate the margin relative to 40/640.
	 */
	hMargin = (winRect.width() * 40 / 640);

	// Make the font italic for the text.
	fntHackText = fntHack;
	fntHackText.setItalic(true);

	// Check if either the italic or normal font has "BLACK STAR" (U+2605).
	QRawFont rawFont = QRawFont::fromFont(fntHackText);
	if (rawFont.supportsCharacter(0x2605)) {
		chrStar = QChar(0x2605);
		fntHackStar = fntHackText;
	} else {
		rawFont = QRawFont::fromFont(fntHack);
		if (rawFont.supportsCharacter(0x2605)) {
			chrStar = QChar(0x2605);
			fntHackStar = fntHack;
		} else {
			// Star not supported.
			chrStar = QChar(L'*');
			fntHackStar = fntHackText;
		}
	}
}

/**
 * Initialize the message.
 * This may be called if the UI language is changed.
 */
void HackDetectionPrivate::initMessage(void)
{
	// "Hack Detection" title.
	switch (detectType) {
		case HackDetection::DetectType::None:
		case HackDetection::DetectType::H:
		default:
			//: "Hack Detection" title.
			hdTitle = HackDetection::tr("Hack Detection", "DetectType::H").toUpper();
			break;

		case HackDetection::DetectType::Q:
			//: "Quack Detection" title.
			hdTitle = HackDetection::tr("Quack Detection", "DetectType::Q").toUpper();
			break;

		case HackDetection::DetectType::S:
			//: "'Snack Detection" title.
			hdTitle = HackDetection::tr("Snack Detection", "DetectType::S").toUpper();
			break;
	}

	// "Hack Detection" message. Preserve the linebreaks!
	switch (detectType) {
		case HackDetection::DetectType::None:
		case HackDetection::DetectType::H:
		case HackDetection::DetectType::Q:
		default:
			//: "Hack Detection" message. Preserve the linebreaks!
			hdMessage = HackDetection::tr(
				"One or more game resources were manipulated by an\n"
				"outside source. This is not allowed as specified in\n"
				"the game license.\n"
				"You must reinstall the game and accept the game\n"
				"license again, to continue to play the game.", "DetectType::H").toUpper();
			hdCloser = HackDetection::tr("Game halted.", "DetectType::H").toUpper();
			break;

		case HackDetection::DetectType::S:
			//: "Snack Detection" message. Preserve the linebreaks!
			hdMessage = HackDetection::tr(
				"One or more snack ingredients were manipulated by an\n"
				"outside sauce. This is not allowed as specified in\n"
				"the snack recipe.\n"
				"You must rebake the snack and accept the snack\n"
				"recipe again, to continue to eat the snack.", "DetectType::S").toUpper();
			hdCloser = HackDetection::tr("Snack salted.", "DetectType::S").toUpper();
			break;
	}

	if (allowEscape)
		hdCloser = HackDetection::tr("Press Escape to go back.").toUpper();
}

/** HackDetection **/

/**
 * Create a Hack Detection window.
 * Uses the default screen.
 * @param parent Parent
 */
HackDetection::HackDetection(QWidget *parent)
	: QWidget(parent,
		Qt::Window |
		Qt::WindowStaysOnTopHint |
		Qt::FramelessWindowHint |
		Qt::CustomizeWindowHint)
	, d_ptr(new HackDetectionPrivate(this))
{
	// Default screen index.
	init(-1);
}

/**
 * Create a Hack Detection window.
 * @param parent Parent
 * @param screen Screen index
 */
HackDetection::HackDetection(int screen, QWidget *parent)
	: QWidget(parent,
		Qt::Window |
		Qt::WindowStaysOnTopHint |
		Qt::FramelessWindowHint |
		Qt::CustomizeWindowHint)
	, d_ptr(new HackDetectionPrivate(this))
{
	init(screen);
}

/**
 * Initialize the Hack Detection window.
 * (Called from the constructor.)
 * @param screen Screen index
 */
void HackDetection::init(int screen)
{
	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);

#ifdef Q_OS_MAC
	// Remove the window icon. (Mac "proxy icon")
	this->setWindowIcon(QIcon());
#endif

	// We're painting the entire window.
	this->setAttribute(Qt::WA_NoSystemBackground, true);

	// Hide the mouse cursor.
	this->setCursor(QCursor(Qt::BlankCursor));

	Q_D(HackDetection);
	d->setScreen(screen);

	// Initialize the window size.
	this->setMinimumSize(d->winRect.size());
	this->setMaximumSize(d->winRect.size());
	this->setFixedSize(d->winRect.size());
	this->setBaseSize(d->winRect.size());

	// Initialize the window position.
	this->move(d->winRect.topLeft());

	// TODO: Set always on top.

	// Initialize the font.
	d->initFont();
	// Initialize the message.
	d->initMessage();
}

/**
 * Shut down the About Dialog.
 */
HackDetection::~HackDetection()
{
	delete d_ptr;
}

HackDetection::DetectType HackDetection::detectType(void) const
{
	Q_D(const HackDetection);
	return d->detectType;
}

void HackDetection::setDetectType(DetectType detectType)
{
	Q_D(HackDetection);
	d->detectType = detectType;

	// Update the message.
	d->initMessage();
	if (this->isVisible())
		this->update();
}

/** Properties **/

/**
 * Minimum size hint.
 * @return Minimum size hint
 */
QSize HackDetection::minimumSizeHint(void) const
{
	Q_D(const HackDetection);
	return d->winRect.size();
}

/**
 * Size hint.
 * @return Size hint
 */
QSize HackDetection::sizeHint(void) const
{
	Q_D(const HackDetection);
	return d->winRect.size();
}

/** Events **/

/**
 * Widget state has changed.
 * @param event State change event
 */
void HackDetection::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(HackDetection);
		d->initMessage();
		if (this->isVisible())
			this->update();
	}

	// Pass the event to the base class.
	this->QWidget::changeEvent(event);
}

/**
 * Show event
 * @param event QShowEvent
 */
void HackDetection::showEvent(QShowEvent *event)
{
	Q_UNUSED(event);
	// Make sure we're fullscreen.
	this->showFullScreen();

	Q_D(HackDetection);
	if (d->screenIdx == 0) {
		// Grab the keyboard and mouse.
		// NOTE: Keyboard grab isn't system-wide...
		grabKeyboard();
		grabMouse();
	}

	// Start the timer.
	d->allowEscape = false;
	d->escapeBlink = false;
	d->tmrEscapeBlink->setInterval(HackDetectionPrivate::ESCAPE_TIMER);
	d->tmrEscapeBlink->setSingleShot(true);
	d->tmrEscapeBlink->start();
}

/**
 * Paint event
 * @param event QPaintEvent
 */
void HackDetection::paintEvent(QPaintEvent *event)
{
	// TODO: Use clipping region?
	Q_UNUSED(event);

	QPainter painter(this);
	painter.setRenderHint(QPainter::TextAntialiasing);

	/** Colors **/

	// Background color: #006600
	QBrush brushBg(QColor(0x00, 0x66, 0x00));
	// Title text color: #FFFF00
	QColor colorTxtTitle(0xFF, 0xFF, 0x00);
	// Message text color: #FFFFFF
	QColor colorTxtMessage(0xFF, 0xFF, 0xFF);
	// Drop shadow text color: #000000
	QColor colorDropShadow(0x00, 0x00, 0x00);

	// Draw the background.
	painter.fillRect(this->rect(), brushBg);

	Q_D(HackDetection);

	// Initialize the font metrics.
	QFontMetrics mtrHackStar(d->fntHackStar);
	QFontMetrics mtrHackText(d->fntHackText);

	// Add stars to the title.
	const QString sStar(d->chrStar);
	QRect rectStar = mtrHackStar.boundingRect(d->winRect, 0, sStar);
	QRect rectTitle = mtrHackText.boundingRect(d->winRect, Qt::AlignHCenter, d->hdTitle);
	QRect rectMessage = mtrHackText.boundingRect(d->winRect, 0, d->hdMessage);
	QRect rectCloser = mtrHackText.boundingRect(d->winRect, 0, d->hdCloser);

	// Total height of the three messages.
	int height = (rectTitle.height() * 4) + rectMessage.height();
	int y = ((d->winRect.height() - height) / 2);

	// Center the messages on the screen.
	rectTitle.moveTop(y);
	rectTitle.moveLeft((d->winRect.width() - rectTitle.width()) / 2);
	rectMessage.moveTop(y + (rectTitle.height() * 2));
	rectMessage.moveLeft((d->winRect.width() - rectMessage.width()) / 2);
	rectCloser.moveTop(rectMessage.top() + rectMessage.height() + rectTitle.height());
	rectCloser.moveLeft(rectMessage.left());

	// Drop shadow rectangle.
	QRect drpRect;

	// Draw the stars.
	int star_xpos[2];
	star_xpos[0] = (rectTitle.left() - (rectStar.width() * 3 / 2));
	star_xpos[1] = (rectTitle.left() + rectTitle.width() + (rectStar.width() / 2));
	rectStar.moveTop(rectTitle.top());
	painter.setFont(d->fntHackStar);

	for (int i = 0; i < 2; i++) {
		rectStar.moveLeft(star_xpos[i]);
		// (drop shadow)
		drpRect = rectStar;
		drpRect.translate(d->drpTranslate);
		painter.setPen(colorDropShadow);
		painter.drawText(drpRect, 0, sStar);
		// (regular text)
		painter.setPen(colorTxtMessage);
		painter.drawText(rectStar, 0, sStar);
	}

	// Draw the title.
	painter.setFont(d->fntHackText);
	// (drop shadow)
	drpRect = rectTitle;
	drpRect.translate(d->drpTranslate);
	painter.setPen(colorDropShadow);
	painter.drawText(drpRect, 0, d->hdTitle);
	// (regular text)
	painter.setPen(colorTxtTitle);
	painter.drawText(rectTitle, 0, d->hdTitle);

	// Draw the message.
	// (drop shadow)
	drpRect = rectMessage;
	drpRect.translate(d->drpTranslate);
	painter.setPen(colorDropShadow);
	painter.drawText(drpRect, 0, d->hdMessage);
	// (regular text)
	painter.setPen(colorTxtMessage);
	painter.drawText(rectMessage, 0, d->hdMessage);

	// Draw the closer.
	// (drop shadow)
	if (!d->allowEscape || d->escapeBlink) {
		drpRect = rectCloser;
		drpRect.translate(d->drpTranslate);
		painter.setPen(colorDropShadow);
		painter.drawText(drpRect, 0, d->hdCloser);
		// (regular text)
		painter.setPen(colorTxtMessage);
		painter.drawText(rectCloser, 0, d->hdCloser);
	}
}

/**
 * Close event
 * @param event QCloseEvent
 */
void HackDetection::closeEvent(QCloseEvent *event)
{
	// Not going to make it *that* easy to get away...
	event->ignore();
}

/**
 * Key press event
 * @param event QKeyEvent
 */
void HackDetection::keyPressEvent(QKeyEvent *event)
{
	Q_D(HackDetection);
	if (d->allowEscape && event->key() == Qt::Key_Escape) {
		event->accept();
		if (d->screenIdx == 0) {
			// Release the keyboard and mouse.
			releaseKeyboard();
			releaseMouse();
		}
		delete this;
		return;
	}

	event->ignore();
}

/** Slots **/

/**
 * "Allow Escape" / Blink timer has expired.
 */
void HackDetection::tmrEscapeBlink_timeout(void)
{
	Q_D(HackDetection);
	if (!d->allowEscape) {
		d->allowEscape = true;
		d->escapeBlink = false;
		d->initMessage(); // Update the closer.
		d->tmrEscapeBlink->setInterval(HackDetectionPrivate::BLINK_TIMER);
		d->tmrEscapeBlink->setSingleShot(false);
		d->tmrEscapeBlink->start();
	} else {
		d->escapeBlink = !d->escapeBlink;
		// TODO: Only update() the closer.
	}
	update();
}
