/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * McRecoverQApplication_win32.cpp: QApplication subclass.                 *
 * Win32-specific functions.                                               *
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

#ifndef _WIN32
#error McRecoverQApplication_win32.cpp should only be compiled on Win32!
#endif

#include "McRecoverQApplication.hpp"

// Include "mcrecover.hpp" first for mcrecover_main().
#include "mcrecover.hpp"

// C includes.
#include <stdint.h>

// C includes. (C++ namespace)
#include <cstring>

// Win32 includes.
#include <windows.h>
#include <shlobj.h>

// DEP policy. (requires _WIN32_WINNT >= 0x0600)
#ifndef PROCESS_DEP_ENABLE
#define PROCESS_DEP_ENABLE 0x1
#endif
#ifndef PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION
#define PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION 0x2
#endif

// WM_THEMECHANGED (requires _WIN32_WINNT >= 0x0501)
#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED 0x031A
#endif

// MSGFLT_ADD (requires _WIN32_WINNT >= 0x0600)
#ifndef MSGFLT_ADD
#define MSGFLT_ADD 1
#endif

// QtCore includes.
#include <QtCore/qt_windows.h>
#include <QtCore/QVector>

#if QT_VERSION >= 0x050000
// QtWinExtras provides the QtWin::fromHICON() function,
// which calls qt_pixmapFromWinHICON(). To reduce code size,
// just use qt_pixmapFromWinHICON() directly.
Q_GUI_EXPORT QPixmap qt_pixmapFromWinHICON(HICON icon);
#endif /* QT_VERSION >= 0x050000 */

// qWinMain declaration.
extern void qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QVector<char *> &);

/** McRecoverQApplicationWin32Private **/

class McRecoverQApplicationWin32Private
{
	private:
		McRecoverQApplicationWin32Private() { }
		~McRecoverQApplicationWin32Private() { }
		Q_DISABLE_COPY(McRecoverQApplicationWin32Private)

	public:
		/**
		 * Enable extra security options.
		 * Reference: http://msdn.microsoft.com/en-us/library/bb430720.aspx
		 * @return 0 on success; non-zero on error.
		 */
		static int SetSecurityOptions(void);

		/**
		 * Get an icon from a Win32 module.
		 * @param module Filename of the Win32 module.
		 * @param resId Resource identifier.
		 * @param size Icon size.
		 * @return Icon, as a QPixmap.
		 */
		static QPixmap GetIconFromModule(const QString &module, 
					uint16_t resId, const QSize &size);

		/**
		 * ChangeWindowMessageFilter() wrapper.
		 * Uses GetProcAddress() to dynamically call the function,
		 * since it was added starting with Windows Vista.
		 * @param message
		 * @param dwFlag
		 * @return TRUE if successful; otherwise, FALSE.
		 */
		static BOOL ChangeWindowMessageFilter_dl(UINT message, DWORD dwFlag);

		/**
		 * Register the "TaskbarButtonCreated" message.
		 * This is required in order to use ITaskbarList3 on Windows 7+.
		 */
		static void RegisterTaskbarButtonCreatedMessage(void);

		// TaskbarButtonCreated message.
		static UINT WM_TaskbarButtonCreated;
};

// TaskbarButtonCreated message.
UINT McRecoverQApplicationWin32Private::WM_TaskbarButtonCreated = 0;

/**
 * Enable extra security options.
 * Reference: http://msdn.microsoft.com/en-us/library/bb430720.aspx
 * @return 0 on success; non-zero on error.
 */
int McRecoverQApplicationWin32Private::SetSecurityOptions(void)
{
	HMODULE hKernel32 = LoadLibrary(L"kernel32.dll");
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
		pfnHeapSetInformation(nullptr, 1, nullptr, 0);
	}

	if (hKernel32)
		FreeLibrary(hKernel32);

	return 0;
}

/**
 * Get an icon from a Win32 module.
 * @param module Filename of the Win32 module.
 * @param resId Resource identifier.
 * @param size Icon size.
 * @return Icon, as a QPixmap.
 */
QPixmap McRecoverQApplicationWin32Private::GetIconFromModule(
		const QString &module, uint16_t resId, const QSize &size)
{
	QPixmap pixmap;
	HMODULE hDll = LoadLibraryW((LPCWSTR)module.utf16());
	if (!hDll)
		return pixmap;

	HICON hIcon = (HICON)LoadImageW(hDll, MAKEINTRESOURCEW(resId),
	IMAGE_ICON, size.width(), size.height(), 0);
	if (hIcon) {
#if QT_VERSION >= 0x050000
		pixmap = qt_pixmapFromWinHICON(hIcon);
#else /* !QT_VERSION >= 0x050000 */
		pixmap = QPixmap::fromWinHICON(hIcon);
#endif /* QT_VERSION >= 0x050000 */
		DestroyIcon(hIcon);
	}

	FreeLibrary(hDll);
	return pixmap;
}

/**
 * ChangeWindowMessageFilter() wrapper.
 * Uses GetProcAddress() to dynamically call the function,
 * since it was added starting with Windows Vista.
 * @param message
 * @param dwFlag
 * @return TRUE if successful; otherwise, FALSE.
 */
BOOL McRecoverQApplicationWin32Private::ChangeWindowMessageFilter_dl(UINT message, DWORD dwFlag)
{
	// Reference: https://msdn.microsoft.com/en-us/library/windows/desktop/dd388202(v=vs.85).aspx
	typedef BOOL (WINAPI *PFNCHANGEWINDOWMESSAGEFILTER)(__in UINT message, __in DWORD dwFlag);
	static PFNCHANGEWINDOWMESSAGEFILTER pfn = nullptr;
	static bool retrieved = false;

	if (!retrieved) {
		retrieved = true;
		HMODULE hUser32 = LoadLibrary(L"user32.dll");
		if (hUser32) {
			pfn = reinterpret_cast<PFNCHANGEWINDOWMESSAGEFILTER>(GetProcAddress(hUser32, "ChangeWindowMessageFilter"));
			if (!pfn) {
				// Not found.
				FreeLibrary(hUser32);
			}
		}
	}

	if (pfn) {
		return pfn(message, dwFlag);
	}

	// Could not obtain a handle to the function.
	// NOTE: Not going to set ERROR_MOD_NOT_FOUND if
	// user32 wasn't found, because that's not possible.
	SetLastError(ERROR_PROC_NOT_FOUND);
	return FALSE;
}

/**
 * Register the "TaskbarButtonCreated" message.
 * This is required in order to use ITaskbarList3 on Windows 7+.
 */
void McRecoverQApplicationWin32Private::RegisterTaskbarButtonCreatedMessage(void)
{
	// Reference: https://github.com/shellscape/Shellscape.Common/blob/master/Shellscape.Common/CodePackShim.cs
	WM_TaskbarButtonCreated = RegisterWindowMessageW(L"TaskbarButtonCreated");
	// Enable taskbar messages even if running elevated.
	// TODO: Why run mcrecover elevated?
	ChangeWindowMessageFilter_dl(WM_TaskbarButtonCreated, MSGFLT_ADD);
	ChangeWindowMessageFilter_dl(WM_COMMAND, MSGFLT_ADD);
	ChangeWindowMessageFilter_dl(WM_SYSCOMMAND, MSGFLT_ADD);
	ChangeWindowMessageFilter_dl(WM_ACTIVATE, MSGFLT_ADD);
}

/** McRecoverQApplication (Win32) **/

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
	McRecoverQApplicationWin32Private::SetSecurityOptions();

	// NOTE: lpCmdLine does not include the program's name.
	// NOTE 2: This is only used for argc/argv. QCoreApplication::arguments()
	// handles Unicode arguments correctly on Windows.

	// TODO: qtmain converts GetCommandLineW()'s output to local 8-bit.
	// Should we just use GetCommandLineA() unconditionally?
	Q_UNUSED(lpCmdLine)
	QByteArray cmdParam;
	wchar_t *cmdLineW = GetCommandLineW();
	if (cmdLineW) {
		// NOTE: Qt4/MSVC usually has wchar_t *not* defined as a native type,
		// but MSVC defaults to wchar_t as a native type.
		// Use the UTF-16 version to prevent compatibility issues.
		// TODO: Convert to UTF-8 instead of "local 8-bit"?
		cmdParam = QString(reinterpret_cast<const QChar*>(cmdLineW)).toLocal8Bit();
	} else {
		cmdParam = QByteArray(GetCommandLineA());
	}

	// Tokenize the command line parameters.
	int argc = 0;
	QVector<char*> argv;
	argv.reserve(8);
	qWinMain(hInst, hPrevInst, cmdParam.data(), nCmdShow, argc, argv);

	// Register the "TaskbarButtonCreated" message.
	McRecoverQApplicationWin32Private::RegisterTaskbarButtonCreatedMessage();

	// Call the real main function.
	return mcrecover_main(argc, argv.data());
}

/**
 * Get a Win32 icon.
 * @param iconId Win32 icon ID.
 * @param size Desired size.
 * @return QIcon.
 */
QIcon McRecoverQApplication::Win32Icon(Win32Icon_t iconId, const QSize &size)
{
	if (iconId <= W32ICON_NONE || iconId >= W32ICON_MAX)
		return QIcon();

	// Get the SYSTEM32 directory.
	// TODO: Win9x support, maybe...
	WCHAR sys32dir[MAX_PATH];
	HRESULT hr = SHGetFolderPathW(nullptr, CSIDL_SYSTEM, nullptr, 0, sys32dir);
	if (FAILED(hr))
		return QIcon();
	// NOTE: QChar* is functionally equivalent to char16_t,
	// which is the same as WCHAR on Windows.
	QString qsys32dir(reinterpret_cast<const QChar*>(sys32dir));

	// TODO: How does Windows handle icon sizes
	// that don't exist in the icon?
	QPixmap pixmap;
	switch (iconId) {
		case W32ICON_DEFRAG: {
			/**
			 * Check the following icons:
			 * - Win7:  SYSTEM32/dfrgui.exe;130
			 * - WinXP: SYSTEM32/dfrgres.dll;106
			 */
			QString dfFile = qsys32dir + QLatin1String("\\dfrgui.exe");
			pixmap = McRecoverQApplicationWin32Private::GetIconFromModule(dfFile, 130, size);
			if (pixmap.isNull()) {
				dfFile = qsys32dir + QLatin1String("\\dfrgres.dll");
				pixmap = McRecoverQApplicationWin32Private::GetIconFromModule(dfFile, 106, size);
			}

			break;
		}

		default:
			break;
	}

	return pixmap;
}

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

// QtGui includes.
#include <QtGui/QFont>

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
	HDC hDC = ::GetDC(nullptr);

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

/**
 * Get the message ID for TaskbarButtonCreated.
 * @return Message ID, or 0 if not registered.
 */
UINT McRecoverQApplication::WM_TaskbarButtonCreated(void)
{
	return McRecoverQApplicationWin32Private::WM_TaskbarButtonCreated;
}
