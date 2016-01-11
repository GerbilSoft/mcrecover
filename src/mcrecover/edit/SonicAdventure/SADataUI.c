/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SADataUI.c: Sonic Adventure - UI data.                                  *
 *                                                                         *
 * Copyright (c) 2015-2016 by David Korth.                                 *
 * Original data from SASave by MainMemory.                                *
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
 * Reference: http://stackoverflow.com/questions/5962503/qt-checkbox-toolbutton-with-custom-distinct-check-unchecked-icons
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
