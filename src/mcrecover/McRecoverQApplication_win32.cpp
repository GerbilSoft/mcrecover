/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * McRecoverQApplication_win32.cpp: QApplication subclass.                 *
 * Win32-specific functions.                                               *
 *                                                                         *
 * Copyright (c) 2013 by David Korth.                                      *
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

#ifndef _WIN32
#error McRecoverQApplication_win32.cpp should only be compiled on Win32!
#endif

#include "McRecoverQApplication.hpp"

// Include "mcrecover.hpp" first for mcrecover_main().
#include "mcrecover.hpp"

// C includes. (C++ namespace)
#include <cstring>

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// DEP policy. (requires _WIN32_WINNT >= 0x0600)
#ifndef PROCESS_DEP_ENABLE
#define PROCESS_DEP_ENABLE 0x1
#endif
#ifndef PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION
#define PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION 0x2
#endif

// QtCore includes.
#include <QtCore/qt_windows.h>
#include <QtCore/QVector>

// qWinMain declaration.
extern void qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QVector<char *> &);

/**
 * Enable extra security options.
 * Reference: http://msdn.microsoft.com/en-us/library/bb430720.aspx
 * @return 0 on success; non-zero on error.
 */
static int SetSecurityOptions(void)
{
	HMODULE hKernel32 = LoadLibraryA("kernel32.dll");
	if (!hKernel32)
		return -1;

	// Enable DEP/NX. (WinXP SP3, Vista, and later.)
	// NOTE: DEP/NX should be specified in the PE header
	// using ld's --nxcompat, but we'll set it manually here,
	// just in case the linker doesn't support it.
	typedef BOOL (WINAPI *PFNSETDEP)(DWORD dwFlags);
	PFNSETDEP pfnSetDep = (PFNSETDEP)GetProcAddress(hKernel32, "SetProcessDEPPolicy");
	if (pfnSetDep)
		pfnSetDep(PROCESS_DEP_ENABLE);

	// Remove the current directory from the DLL search path.
	typedef BOOL (WINAPI *PFNSETDLLDIRA)(LPCSTR lpPathName);
	PFNSETDLLDIRA pfnSetDllDirectoryA = (PFNSETDLLDIRA)GetProcAddress(hKernel32, "SetDllDirectoryA");
	if (pfnSetDllDirectoryA)
		pfnSetDllDirectoryA("");

	// Terminate the process if heap corruption is detected.
	// NOTE: Parameter 2 is usually type enum HEAP_INFORMATION_CLASS,
	// but this type isn't present in older versions of MinGW, so we're
	// using int instead.
	typedef BOOL (WINAPI *PFNHSI)
		(HANDLE HeapHandle, int HeapInformationClass,
		 PVOID HeapInformation, SIZE_T HeapInformationLength);
	PFNHSI pfnHeapSetInformation = (PFNHSI)GetProcAddress(hKernel32, "HeapSetInformation");
	if (pfnHeapSetInformation) {
		// HeapEnableTerminationOnCorruption == 1
		pfnHeapSetInformation(NULL, 1, NULL, 0);
	}

	if (hKernel32)
		FreeLibrary(hKernel32);

	return 0;
}

/**
 * Main entry point on Win32.
 * Code based on libqtmain-4.7.1.
 * Windows CE-specific parts have been removed.
 * @param hInst Instance.
 * @param hPrevInst Previous instance. (Unused on Win32)
 * @param lpCmdLine Command line parameters. (ANSI)
 * @param nCmdShow Main window show parameter.
 * @return Return code.
 */
extern "C"
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	// Enable extra security options.
	SetSecurityOptions();

	// Show a warning if an app-based operating system is in use.
	OSVERSIONINFOA osVersionInfo;
	osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
	if (GetVersionExA(&osVersionInfo)) {
		if (osVersionInfo.dwMajorVersion > 6 ||
		    (osVersionInfo.dwMajorVersion == 6 &&
		     osVersionInfo.dwMinorVersion >= 2))
		{
			MessageBoxA(NULL,
				"You are using an app-based operating system.\n"
				"GCN MemCard Recover is a real program, not an app.\n"
				"As such, proper operation cannot be guaranteed.",
				"App-based OS detected", MB_ICONEXCLAMATION);
		}
	}

	// NOTE: lpCmdLine does not include the program's name.
	// TODO: qtmain converts GetCommandLineW()'s output to local 8-bit.
	// Should we just use GetCommandLineA() unconditionally?
	Q_UNUSED(lpCmdLine)
	QByteArray cmdParam;
	wchar_t *cmdLineW = GetCommandLineW();
	if (cmdLineW)
		cmdParam = QString::fromWCharArray(cmdLineW).toLocal8Bit();
	else
		cmdParam = QByteArray(GetCommandLineA());

	// Tokenize the command line parameters.
	int argc = 0;
	QVector<char*> argv;
	argv.reserve(8);
	qWinMain(hInst, hPrevInst, cmdParam.data(), nCmdShow, argc, argv);

	// Call the real main function.
	return mcrecover_main(argc, argv.data());
}

// QtGui includes.
#include <QtGui/QFont>

/**
 * Win32 event filter.
 * @param msg Win32 message.
 * @param result Return value for the window procedure.
 * @return True if we're handling the message; false if we should let Qt handle the message.
 */
bool McRecoverQApplication::winEventFilter(MSG *msg, long *result)
{
	Q_UNUSED(result)

	if (msg->message == WM_SETTINGCHANGE &&
	    msg->wParam == SPI_SETNONCLIENTMETRICS)
	{
		// WM_SETTINGCHANGE / SPI_SETNONCLIENTMETRICS.
		// Update the Qt font.
		SetFont_Win32();
	} else if (msg->message == WM_THEMECHANGED) {
		// WM_THEMECHANGED: XP/Vista theme has changed.
		emit themeChanged();
	}

	// Allow QApplication to handle this message anyway.
	return false;
}

/**
 * Set the Qt font to match the system font.
 */
void McRecoverQApplication::SetFont_Win32(void)
{
	// Get the Win32 message font.
	NONCLIENTMETRICSA ncm;
	ncm.cbSize = sizeof(ncm);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

	int nFontSize = 0;
	HDC hDC = ::GetDC(NULL);

	// Calculate the font size in points.
	// http://www.codeguru.com/forum/showthread.php?t=476244
	if (ncm.lfMessageFont.lfHeight < 0) {
		nFontSize = -::MulDiv(ncm.lfMessageFont.lfHeight,
				      72, ::GetDeviceCaps(hDC, LOGPIXELSY));
	} else {
		TEXTMETRIC tm;
		memset(&tm, 0x00, sizeof(tm));
		::GetTextMetrics(hDC, &tm);

		nFontSize = -::MulDiv(ncm.lfMessageFont.lfHeight - tm.tmInternalLeading,
				      72, ::GetDeviceCaps(hDC, LOGPIXELSY));
	}

	// TODO: Scale Windows font weights to Qt font weights.

	// TODO: Menus always use the message font, and they already
	// respond to WM_SETTINGCHANGE. Make menus use the menu font.

	// Create the QFont.
	QFont qAppFont(QLatin1String(ncm.lfMessageFont.lfFaceName),
			nFontSize, -1, ncm.lfMessageFont.lfItalic);

	// Set the Qt application font.
	QApplication::setFont(qAppFont);
}
