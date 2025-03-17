/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SADataUI.c: Sonic Adventure - UI data.                                  *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * Original data from SASave by MainMemory.                                *
 *                                                                         *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "SAData.h"

/**
 * Character icons. (Main characters only)
 */
const char *const sa_ui_char_icons[SA_UI_CHAR_ICONS_COUNT] = {
	":/sonic/SA1/sonic.png",
	":/sonic/SA1/tails.png",
	":/sonic/SA1/knuckles.png",
	":/sonic/SA1/amy.png",
	":/sonic/SA1/gamma.png",
	":/sonic/SA1/big.png",
};

/**
 * Character names. (Main characters only)
 */
const char *const sa_ui_char_names[SA_UI_CHAR_NAMES_COUNT] = {
	"Sonic", "Tails", "Knuckles", "Amy", "Gamma", "Big"
};

/**
 * Character icons. (With Super Sonic)
 * This array includes a placeholder for an unused character.
 */
const char *const sa_ui_char_icons_super[SA_UI_CHAR_ICONS_SUPER_COUNT] = {
	":/sonic/SA1/sonic.png",
	"",				// Unused
	":/sonic/SA1/tails.png",
	":/sonic/SA1/knuckles.png",
	":/sonic/SA1/sonic.png",	// Super Sonic
	":/sonic/SA1/amy.png",
	":/sonic/SA1/gamma.png",
	":/sonic/SA1/big.png",
};

/**
 * Character names. (With Super Sonic)
 * This array includes a placeholder for an unused character.
 */
const char *const sa_ui_char_names_super[SA_UI_CHAR_NAMES_SUPER_COUNT] = {
	"Sonic", "Unused", "Tails", "Knuckles",
	"Super Sonic", "Amy", "Gamma", "Big"
};

/**
 * Qt CSS for Emblem checkboxes.
 * Reference: https://stackoverflow.com/questions/5962503/qt-checkbox-toolbutton-with-custom-distinct-check-unchecked-icons
 */
const char sa_ui_css_emblem_checkbox[] =
	"QCheckBox::indicator {\n"
	"\twidth: 28px;\n"
	"\theight: 20px;\n"
	"}\n"
	"QCheckBox::indicator:checked {\n"
	"\timage: url(:/sonic/emblem.png);\n"
	"}\n"
	"QCheckBox::indicator:unchecked {\n"
	"\timage: url(:/sonic/noemblem.png);\n"
	"}\n"
	"QCheckBox::indicator:checked:hover {\n"
	"\timage: url(:/sonic/emblem.highlight.png);\n"
	"}\n"
	"QCheckBox::indicator:unchecked:hover {\n"
	"\timage: url(:/sonic/noemblem.highlight.png);\n"
	"}\n"
	"QCheckBox::indicator:checked:pressed {\n"
	"\timage: url(:/sonic/emblem.shadow.png);\n"
	"}\n"
	"QCheckBox::indicator:unchecked:pressed {\n"
	"\timage: url(:/sonic/noemblem.shadow.png);\n"
	"}\n";

/**
 * Qt CSS for Emblem checkboxes. (large size)
 * Reference: https://stackoverflow.com/questions/5962503/qt-checkbox-toolbutton-with-custom-distinct-check-unchecked-icons
 */
const char sa_ui_css_emblem_checkbox_large[] =
	"QCheckBox::indicator {\n"
	"\twidth: 51px;\n"
	"\theight: 36px;\n"
	"}\n"
	"QCheckBox::indicator:checked {\n"
	"\timage: url(:/sonic/emblem.png);\n"
	"}\n"
	"QCheckBox::indicator:unchecked {\n"
	"\timage: url(:/sonic/noemblem.png);\n"
	"}\n"
	"QCheckBox::indicator:checked:hover {\n"
	"\timage: url(:/sonic/emblem.highlight.png);\n"
	"}\n"
	"QCheckBox::indicator:unchecked:hover {\n"
	"\timage: url(:/sonic/noemblem.highlight.png);\n"
	"}\n"
	"QCheckBox::indicator:checked:pressed {\n"
	"\timage: url(:/sonic/emblem.shadow.png);\n"
	"}\n"
	"QCheckBox::indicator:unchecked:pressed {\n"
	"\timage: url(:/sonic/noemblem.shadow.png);\n"
	"}\n";
