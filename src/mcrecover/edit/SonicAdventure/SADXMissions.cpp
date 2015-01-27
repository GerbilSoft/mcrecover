/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SADXMissions.hpp: Sonic Adventure DX - Mission editor.                  *
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

#include "SADXMissions.hpp"

// Qt widgets.
#include <QtGui/QHBoxLayout>
#include <QtGui/QCheckBox>

// Sonic Adventure save file definitions.
#include "sa_defs.h"

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** SADXMissionsPrivate **/

#include "ui_SADXMissions.h"
class SADXMissionsPrivate
{
	public:
		SADXMissionsPrivate(SADXMissions *q);
		~SADXMissionsPrivate();

	protected:
		SADXMissions *const q_ptr;
		Q_DECLARE_PUBLIC(SADXMissions)
	private:
		Q_DISABLE_COPY(SADXMissionsPrivate)

	public:
		Ui_SADXMissions ui;

		// Total of 60 missions.
		// FIXME: sizeof(sadx_extra_save_slot::missions) works in gcc,
		// but fails in MSVC 2010 with C2597.
		#define TOTAL_MISSIONS 60
		// Level widgets.
		struct {
			QLabel *lblMissionID;
			// TODO: Description, character icon.
			QCheckBox *chkData[3];
		} missionUI[TOTAL_MISSIONS];

		// Save file data.
		// TODO: struct in sa_defs.h
		uint8_t mission_data[TOTAL_MISSIONS];

		/**
		 * Clear the loaded data.
		 * This does NOT automatically update the UI.
		 */
		void clear(void);

		/**
		 * Initialize mission widgets.
		 */
		void initMissions(void);

		/**
		 * Update the widgets with the loaded data.
		 */
		void updateDisplay(void);

		/** Static read-only data **/

		// Mission data. (ASCII, untranslated)
		// TODO: Separate character/desc structs for
		// better memory usage on 64-bit?
		struct MissionData_t {
			// 0 == Sonic, 1 == Tails, 2 == Knuckles
			// 3 == Amy, 4 == Gamma, 5 == Big
			int chr;		// Character. (TODO: enum)
			const char *desc;	// Description.
		};
		static const MissionData_t missionInfo[TOTAL_MISSIONS];
};

// Mission data. (ASCII, untranslated)
// TODO: Move to a separate file?
// TODO: Verify mission text.
// Reference: http://www.ign.com/faqs/2004/sonic-adventure-missions-faq-474382
const SADXMissionsPrivate::MissionData_t SADXMissionsPrivate::missionInfo[TOTAL_MISSIONS] = {
	// NOTE: Mission numbering starts at 1.

	// Missions 1-10
	{0, "Bring the man who is standing in front of the hamburger shop!"},
	{0, "Get the balloon in the skies of the Mystic Ruins!"},
	{0, "Collect 100 rings, and go to Sonic's billboard by the pool!"},
	{1, "Weeds are growing all over my place! I must get rid of them!"},
	{2, "I lost my balloon! It's way up there now!"},
	{3, "He is going to drown! Help the man in the water!"},
	{4, "Lonely Metal Sonic needs a friend. Look carefully."},
	{5, "The medallion fell under there! No illegal parking please!"},
	{0, "Get the balloon floating behind the waterfall at the emerald sea."},
	{1, "What is that sparkling in the water?"},

	// Missions 11-20
	{0, "Destroy the windmill and proceed. Find the balloon in orbit!"},
	{2, "Who is a Chao good friends with? And what is hidden underneath?"},
	{0, "I can't take a shower like this! Do something!"},
	{5, "I am the keeper of this hotel! Catch me if you can!"},
	{0, "My medallions got swept away by the tornado! Somebody help me get them back!"},
	{1, "Get the flags from the floating islands!"},
	{0, "Aim and shoot all the medallions with a Sonic Ball."},
	{3, "During the night, at the amusement park, place your jumps on the top of one of the tables."},
	{3, "What is that behind the mirror?"},
	{0, "Get all the medallions within the time limit! It's real slippery, so be careful!"},

	// Missions 21-30
	{4, "Protect the Sonic doll from the Spinners surrounding it!"},
	{5, "Find the flag hidden in the secret passage under the emerald ocean!"},
	{0, "Go around the wooden horse and collect 10 balloons."},
	{1, "'I hate this dark and filthy place!' Can you find it?"},
	{2, "What is hidden under the lion's right hand?"},
	{2, "What is that on top of the ship's mast that the pirates are protecting?"},
	{0, "Collect 100 rings and head to the heliport!"},
	{0, "During the morning traffic, use the fountain to get the balloon."},
	{5, "I am the keeper of this canal! Catch me if you can!"},
	{0, "A fugitive have escaped from the jail of burning hell! Find the fugitive!"},

	// Missions 31-40
	{1, "Get the balloon as you float in the air along with the trash!"},
	{2, "Can you get the balloon that is hidden under the bridge?"},
	{0, "Shoot yourself out of the cannon and get the balloon!"},
	{0, "Can you find the balloon that is hidden on the ship's bridge?"},
	{5, "I am the keeper of this icy lake! Catch me if you can!"},
	{0, "Fighter aircraft are flying everywhere. Somebody get me out of here!"},
	{1, "Fly over the jungle, and get all the balloons!"},
	{2, "A message from an ancient people: In the direction where the burning arrow is pointing, you will see..."},
	{4, "Treasure hunt at the beach! Find all the medallions under a time limit!"},
	{0, "What is hidden in the area that the giant snake is staring at?"},

	// Missions 41-50
	{0, "Look real carefully just as you fall from the waterfall!"},
	{4, "I can't get into the bathroom. How could I've let something like this happen to me?"},
	{3, "Fortress of steel. High Jump on 3 narrow paths. Be careful not to fall."},
	{5, "I am the keeper of this ship! Catch me if you can!"},
	{0, "Go to a place where the rings are laid in the shape of Sonic's face!"},
	{0, "A secret base full of mechanical traps. Pay attention, and you might see..."},
	{1, "Get 10 balloons on the field under the time limit!"},
	{2, "Can you get the medallion that the giant Sonic is staring at?"},
	{0, "Scorch through the track, and get all the flags!"},
	{3, "Select a road that splits into 5 paths before time runs out!"},

	// Missions 51-60
	{4, "Gunman of the Windy Valley! Destroy all of the Spinners under a time limit!"},
	{5, "Get 3 flags in the jungle under the time limit!"},
	{0, "Pass the line of rings with 3 Super High Jumps on the ski slope!"},
	{1, "Slide downhill in a blizzard and get all of the flags!"},
	{0, "Run down the building to get all the balloons!"},
	{2, "Relentless eruptions occur in the flaming canyon. What could be hidden in the area she's staring at?"},
	{0, "Peak of the volcanic mountain! Watch out for the lava!"},
	{0, "The big rock will start rolling after you! Try to get all the flags"},
	{2, "Watch out for the barrels, and find the hidden flag inside the container!"},
	{5, "Something is hidden inside the dinosaur's mouth. Can you find it?"},
};

SADXMissionsPrivate::SADXMissionsPrivate(SADXMissions *q)
	: q_ptr(q)
{
	// Clear the data.
	clear();
}

SADXMissionsPrivate::~SADXMissionsPrivate()
{
	// TODO: Is this needed?
	// The level widgets are owned by this widget, so they
	// should be automatically deleted...
	for (int i = 0; i < NUM_ELEMENTS(missionUI); i++) {
		delete missionUI[i].lblMissionID;
		for (int j = 0; j < NUM_ELEMENTS(missionUI[i].chkData); j++) {
			delete missionUI[i].chkData[j];
		}
	}
}

/**
 * Clear the loaded data.
 * This does NOT automatically update the UI.
 */
void SADXMissionsPrivate::clear(void)
{
	memset(&mission_data, 0, sizeof(mission_data));
}

/**
 * Initialize mission widgets.
 */
void SADXMissionsPrivate::initMissions(void)
{
	// Create widgets for all levels.
	// TODO: Top or VCenter?
	// TODO: Scroll area is screwing with minimum width...
	Q_Q(SADXMissions);
	for (int mission = 0; mission < TOTAL_MISSIONS; mission++) {
		// Mission number.
		missionUI[mission].lblMissionID = new QLabel(q);
		missionUI[mission].lblMissionID->setText(QString::number(mission+1));
		missionUI[mission].lblMissionID->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		ui.gridMissions->addWidget(missionUI[mission].lblMissionID, mission+1, 0, Qt::AlignTop);

		// Checkbox for each mission state.
		for (int chk = 0; chk < NUM_ELEMENTS(missionUI[mission].chkData); chk++) {
			missionUI[mission].chkData[chk] = new QCheckBox(q);
			ui.gridMissions->addWidget(missionUI[mission].chkData[chk], mission+1, chk+4, Qt::AlignTop | Qt::AlignHCenter);
		}
	}
}

/**
 * Update the widgets with the loaded data.
 */
void SADXMissionsPrivate::updateDisplay(void)
{
	for (int mission = 0; mission < NUM_ELEMENTS(missionUI); mission++) {
		missionUI[mission].chkData[0]->setChecked(!!(mission_data[mission] & SADX_MISSION_UNLOCKED));
		missionUI[mission].chkData[1]->setChecked(!!(mission_data[mission] & SADX_MISSION_ACTIVE));
		missionUI[mission].chkData[2]->setChecked(!!(mission_data[mission] & SADX_MISSION_COMPLETED));
	}
}

/** SADXMissions **/

SADXMissions::SADXMissions(QWidget *parent)
	: QWidget(parent)
	, d_ptr(new SADXMissionsPrivate(this))
{
	Q_D(SADXMissions);
	d->ui.setupUi(this);

	// Fix alignment of the header labels.
	d->ui.gridMissions->setAlignment(d->ui.lblMissionHeader, Qt::AlignTop);
	d->ui.gridMissions->setAlignment(d->ui.lblUnlocked,      Qt::AlignTop);
	d->ui.gridMissions->setAlignment(d->ui.lblActive,        Qt::AlignTop);
	d->ui.gridMissions->setAlignment(d->ui.lblCompleted,     Qt::AlignTop);

	// Initialize the mission listing.
	d->initMissions();
}

SADXMissions::~SADXMissions()
{
	Q_D(SADXMissions);
	delete d;
}

/** Events. **/

/**
 * Widget state has changed.
 * @param event State change event.
 */
void SADXMissions::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(SADXMissions);
		d->ui.retranslateUi(this);
	}

	// Pass the event to the base class.
	this->QWidget::changeEvent(event);
}

/** Public functions. **/

/**
 * Load data from an SADX extra save slot.
 * @param sadx_extra_save SADX extra save slot.
 * The data must have already been byteswapped to host-endian.
 * @return 0 on success; non-zero on error.
 */
int SADXMissions::load(const sadx_extra_save_slot *sadx_extra_save)
{
	Q_D(SADXMissions);
	memcpy(d->mission_data, sadx_extra_save->missions, sizeof(d->mission_data));

	// Update the display.
	d->updateDisplay();
	return 0;
}

/**
 * Clear the loaded data.
 */
void SADXMissions::clear(void)
{
	Q_D(SADXMissions);
	d->clear();
	d->updateDisplay();
}
