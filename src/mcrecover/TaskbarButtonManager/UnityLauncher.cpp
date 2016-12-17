/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * UnityLauncher.cpp: Unity Launcher implementation.                       *
 *                                                                         *
 * Copyright (c) 2013-2016 by David Korth.                                 *
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

#include "UnityLauncher.hpp"

// Qt includes.
#include <QWidget>

// dlopen()
#include <dlfcn.h>

#define DESKTOP_FILENAME "mcrecover.desktop"

/** UnityLauncherPrivate **/

#include "TaskbarButtonManager_p.hpp"
class UnityLauncherPrivate : public TaskbarButtonManagerPrivate
{
	public:
		explicit UnityLauncherPrivate(UnityLauncher *const q);
		virtual ~UnityLauncherPrivate();

	private:
		typedef TaskbarButtonManagerPrivate super;
		Q_DECLARE_PUBLIC(UnityLauncher)
		Q_DISABLE_COPY(UnityLauncherPrivate)

	public:
		/**
		 * Update the Unity Launcher Entry.
		 */
		void update(void);

	private:
		// libunity handle.
		// NOTE: This should NOT be closed, ever!
		// Closing and reopening libunity causes a crash
		// because it tries to re-register itself.
		static void *libunity;

		/**
		 * Attempt to open libunity.
		 * @return libunity handle, or nullptr if libunity could not be opened.
		 */
		static void *open_libunity(void);

		/**
		 * Check if Unity is running.
		 * @return True if Unity is running; false if not.
		 */
		static bool is_unity_running(void);
		
		// Unity Launcher API.
		// References:
		// - https://help.ubuntu.com/community/UnityLaunchersAndDesktopFiles
		// - https://wiki.ubuntu.com/Unity/LauncherAPI
		// - https://code.google.com/p/chromium/codesearch#chromium/src/chrome/browser/ui/libgtk2ui/unity_service.cc
		// TODO: gchar*?
		typedef struct _UnityInspector UnityInspector;
		typedef UnityInspector* (*unity_inspector_get_default_func)(void);
		typedef int (*unity_inspector_get_unity_running_func)(UnityInspector* self);

		typedef struct _UnityLauncherEntry UnityLauncherEntry;
		typedef UnityLauncherEntry* (*unity_launcher_entry_get_for_desktop_id_func)(const char *desktop_id);
		typedef void (*unity_launcher_entry_set_progress_func)(UnityLauncherEntry *self, double progress);
		// NOTE: 'visible' is gboolean, which is gint.
		typedef void (*unity_launcher_entry_set_progress_visible_func)(UnityLauncherEntry *self, int visible);

		// Function pointers.
		static unity_launcher_entry_get_for_desktop_id_func entry_get_for_desktop_id;
		static unity_launcher_entry_set_progress_func entry_set_progress;
		static unity_launcher_entry_set_progress_visible_func entry_set_progress_visible;

		// Unity Launcher Entry.
		UnityLauncherEntry *entry;
};

// libunity handle.
// NOTE: This should NOT be closed, ever!
// Closing and reopening libunity causes a crash
// because it tries to re-register itself.
void *UnityLauncherPrivate::libunity = nullptr;

// Function pointers.
UnityLauncherPrivate::unity_launcher_entry_get_for_desktop_id_func
	UnityLauncherPrivate::entry_get_for_desktop_id = nullptr;
UnityLauncherPrivate::unity_launcher_entry_set_progress_func
	UnityLauncherPrivate::entry_set_progress = nullptr;
UnityLauncherPrivate::unity_launcher_entry_set_progress_visible_func
	UnityLauncherPrivate::entry_set_progress_visible = nullptr;

UnityLauncherPrivate::UnityLauncherPrivate(UnityLauncher *const q)
	: super(q)
	, entry(nullptr)
{
	// Make sure libunity is open.
	if (!open_libunity()) {
		// Could not open libunity.
		return;
	}

	// Check if all of the required function pointers are available.
	if (!entry_get_for_desktop_id ||
	    !entry_set_progress ||
	    !entry_set_progress_visible)
	{
		// One or more of the required function pointers are missing.
		entry = nullptr;
		return;
	}

	// Get the desktop entry.
	// TODO: Set the filename somewhere else?
	// NOTE: Our desktop entry isn't working.
	// "firefox.desktop" works; however, Unity updates
	// *after* we're done scanning, so we may need to
	// add a slight delay...
	entry = entry_get_for_desktop_id(DESKTOP_FILENAME);
	if (!entry) {
		// Unable to get the desktop entry.
		dlclose(libunity);
		libunity = nullptr;
		return;
	}

	// No update should be needed here, since the
	// entry should be in its default state.
}

UnityLauncherPrivate::~UnityLauncherPrivate()
{
	// Unload libunity.
	if (libunity) {
		dlclose(libunity);
	}
}

/**
 * Attempt to open libunity.
 * @return libunity handle, or nullptr if libunity could not be opened.
 */
void *UnityLauncherPrivate::open_libunity(void)
{
	if (libunity) {
		// libunity is already open.
		return libunity;
	}

	// Attempt to open libunity.
	// NOTE: libunity doesn't install an unversioned symlink,
	// so we have to try multiple versions. (Chromium tries 4, 6, and 9.)
	// TODO: Increase maximum libunity version later?
	char libunity_filename[32];
	for (int i = 12; i >= 0; i--) {
		snprintf(libunity_filename, sizeof(libunity_filename),
			 "libunity.so.%d", i);
		// TODO: Use RTLD_LAZY for APNG and GIF?
		libunity = dlopen(libunity_filename, RTLD_LOCAL|RTLD_LAZY);
		if (libunity) {
			// Found libunity.
			break;
		}
	}

	if (libunity) {
		// libunity is open.
		// Get the required function pointers.
		entry_get_for_desktop_id =
			reinterpret_cast<unity_launcher_entry_get_for_desktop_id_func>(
				dlsym(libunity, "unity_launcher_entry_get_for_desktop_id"));
		entry_set_progress =
			reinterpret_cast<unity_launcher_entry_set_progress_func>(
				dlsym(libunity, "unity_launcher_entry_set_progress"));
		entry_set_progress_visible =
			reinterpret_cast<unity_launcher_entry_set_progress_visible_func>(
				dlsym(libunity, "unity_launcher_entry_set_progress_visible"));
	}

	return libunity;
}

/**
 * Check if Unity is running.
 * @return True if Unity is running; false if not.
 */
bool UnityLauncherPrivate::is_unity_running(void)
{
	// Make sure libunity is open.
	if (!open_libunity()) {
		// Could not open libunity.
		return false;
	}

	bool isRunning = false;
	unity_inspector_get_default_func inspector_get_default =
		reinterpret_cast<unity_inspector_get_default_func>(
			dlsym(libunity, "unity_inspector_get_default"));
	if (inspector_get_default) {
		UnityInspector *inspector = inspector_get_default();
		if (inspector) {
			unity_inspector_get_unity_running_func get_unity_running =
				reinterpret_cast<unity_inspector_get_unity_running_func>(
					dlsym(libunity, "unity_inspector_get_unity_running"));
			if (get_unity_running) {
				isRunning = !!get_unity_running(inspector);
			}
		}
	}
	return isRunning;
}

/**
 * Update the Unity Launcher Entry status.
 */
void UnityLauncherPrivate::update(void)
{
	if (!entry)
		return;

	// Update the Unity Launcher Entry.
	// TODO: Optimize DockManager to use progressBar* directly?
	if (progressBarValue < 0) {
		// Progress bar is hidden.
		entry_set_progress_visible(entry, false);
	} else {
		// Progress bar is visible.
		float value = (float)progressBarValue / (float)progressBarMax;
		entry_set_progress(entry, value);
		entry_set_progress_visible(entry, true);
	}

	typedef void (*ufn)(UnityLauncherEntry *self, int urgent);
	ufn u = (ufn)dlsym(libunity, "unity_launcher_entry_set_urgent");
	u(entry, 1);
}

/** UnityLauncher **/

UnityLauncher::UnityLauncher(QObject* parent)
	: super(new UnityLauncherPrivate(this), parent)
{ }

/**
 * Is this TaskbarButtonManager usable?
 * @return True if usable; false if not.
 */
bool UnityLauncher::IsUsable(void)
{
	return UnityLauncherPrivate::is_unity_running();
}

/**
 * Set the window this TaskbarButtonManager should manage.
 * This must be a top-level window in order to work properly.
 *
 * Subclasses should reimplement this function if special
 * initialization is required to set up the taskbar button.
 *
 * TODO: Make a separate protected function that setWindow() calls?
 *
 * @param window Window.
 */
void UnityLauncher::setWindow(QWidget *window)
{
	// FIXME: Ignored; launcher entries are per-application.
	// Maybe use this to set the desktop entry filename?
	Q_UNUSED(window)
}

/**
 * Update the taskbar button.
 */
void UnityLauncher::update(void)
{
	Q_D(UnityLauncher);
	d->update();
}
