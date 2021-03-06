PROJECT(mcrecover)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}")

# Data directory.
INCLUDE(DirInstallPaths)
IF(WIN32 OR APPLE)
	# Win32 and Mac OS X don't use `make install`.
	SET(MCRECOVER_DATA_DIRECTORY "")
	SET(MCRECOVER_TRANSLATIONS_DIRECTORY "")
ELSE()
	# Unix system.
	SET(MCRECOVER_DATA_DIRECTORY "${CMAKE_INSTALL_PREFIX}/${DIR_INSTALL_DATA}")
	SET(MCRECOVER_TRANSLATIONS_DIRECTORY "${CMAKE_INSTALL_PREFIX}/${DIR_INSTALL_TRANSLATIONS}")
ENDIF()

# Find Qt5.
# FIXME: Figure out a minimum Qt5 version.
# QtWin::fromHICON was added in Qt 5.2.
SET(Qt5_NO_LINK_QTMAIN 1)
FIND_PACKAGE(Qt5 5.2.0 REQUIRED COMPONENTS Core Gui Widgets Xml)
IF(ENABLE_DBUS)
	# QtDBus is used for DockManager.
	FIND_PACKAGE(Qt5DBus)
	# FIXME: HAVE_Qt5DBus isn't working...
	SET(QtDBus_FOUND ${Qt5DBus_FOUND})
ENDIF(ENABLE_DBUS)

IF(WIN32 AND QT_IS_STATIC)
	# Win32: Qt Static requires some extra libraries.
	SET(WIN32_LIBS ws2_32 winmm imm32)
ENDIF(WIN32 AND QT_IS_STATIC)

# Link to Carbon on Mac OS X.
# TODO: Use Cocoa instead if we're compiling against Qt/Cocoa.
IF(APPLE)
	#SET(GUI_TYPE MACOSX_BUNDLE)
	IF(QT_MAC_USE_COCOA)
		# Qt is using Cocoa.
		FIND_LIBRARY(FOUNDATION_LIBRARY Foundation)
		MARK_AS_ADVANCED(FOUNDATION_LIBRARY)
		SET(APPLE_LIBS ${FOUNDATION_LIBRARY})
	ELSE(QT_MAC_USE_COCOA)
		# Qt is using Carbon.
		INCLUDE_DIRECTORIES ( /Developer/Headers/FlatCarbon )
		FIND_LIBRARY(CARBON_LIBRARY Carbon)
		MARK_AS_ADVANCED(CARBON_LIBRARY)
		SET(APPLE_LIBS ${CARBON_LIBRARY})
	ENDIF(QT_MAC_USE_COCOA)
ENDIF(APPLE)

# libpng (AboutDialog only; linked indirectly through libgctools)
INCLUDE(CheckPNG)
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
ADD_DEFINITIONS(${ZLIB_DEFINITIONS} ${PNG_DEFINITIONS})

# giflib (AboutDialog only; linked indirectly through libgctools)
INCLUDE(CheckGIF)
INCLUDE_DIRECTORIES(${GIF_INCLUDE_DIR})
ADD_DEFINITIONS(${GIF_DEFINITIONS})

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.mcrecover.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.mcrecover.h")

# Sources.
SET(mcrecover_SRCS
	mcrecover.cpp
	McRecoverQApplication.cpp
	VarReplace.cpp
	TranslationManager.cpp
	config/ConfigStore.cpp
	config/ConfigDefaults.cpp
	PathFuncs.cpp
	)

SET(mcrecover_DB_SRCS
	db/GcnMcFileDb.cpp
	db/GcnSearchThread.cpp
	db/GcnSearchWorker.cpp
	db/GcnCheckFiles.cpp
	)
SET(mcrecover_DB_H
	db/GcnMcFileDef.hpp
	)

SET(mcrecover_WINDOW_SRCS
	windows/McRecoverWindow.cpp
	windows/AboutDialog.cpp
	windows/XmlTemplateDialog.cpp
	windows/XmlTemplateDialogManager.cpp
	windows/FormatNewMemCardDialog.cpp
	)

SET(mcrecover_WIDGET_SRCS
	widgets/StatusBarManager.cpp
	widgets/FileView.cpp
	widgets/CardView.cpp
	widgets/QTreeViewOpt.cpp
	widgets/MessageWidget.cpp
	widgets/TableSelect.cpp
	widgets/MessageWidgetStack.cpp
	widgets/LanguageMenu.cpp
	)

# Shh... it's a secret to everybody.
SET(mcrecover_SEKRIT_SRCS
	sekrit/HerpDerpEggListener.cpp
	sekrit/HackDetection.cpp
	)

# Win32-specific sources.
IF(WIN32)
	SET(mcrecover_OS_SRCS
		McRecoverQApplication_win32.cpp
		resources/win32/mcrecover.rc
		)
ENDIF(WIN32)

# Headers with Qt objects.
SET(mcrecover_MOC_H
	McRecoverQApplication.hpp
	config/ConfigStore.hpp
	)

SET(mcrecover_DB_MOC_H
	db/GcnMcFileDb.hpp
	db/GcnSearchThread.hpp
	db/GcnSearchWorker.hpp
	db/GcnCheckFiles.hpp
	)

SET(mcrecover_WINDOW_MOC_H
	windows/McRecoverWindow.hpp
	windows/AboutDialog.hpp
	windows/XmlTemplateDialog.hpp
	windows/XmlTemplateDialogManager.hpp
	windows/FormatNewMemCardDialog.hpp
	)

SET(mcrecover_WIDGET_MOC_H
	widgets/StatusBarManager.hpp
	widgets/FileView.hpp
	widgets/CardView.hpp
	widgets/QTreeViewOpt.hpp
	widgets/MessageWidget.hpp
	widgets/TableSelect.hpp
	widgets/MessageWidgetStack.hpp
	widgets/LanguageMenu.hpp
	)

SET(mcrecover_SEKRIT_MOC_H
	sekrit/HerpDerpEggListener.hpp
	sekrit/HackDetection.hpp
	)

# UI files.
SET(mcrecover_WINDOW_UIS
	windows/McRecoverWindow.ui
	windows/AboutDialog.ui
	windows/XmlTemplateDialog.ui
	windows/FormatNewMemCardDialog.ui
	)

SET(mcrecover_WIDGET_UIS
	widgets/FileView.ui
	widgets/CardView.ui
	)

# Generate the header files from the UI files.
SET(mcrecover_UIS
	${mcrecover_WINDOW_UIS}
	${mcrecover_WIDGET_UIS}
	${mcrecover_EDIT_UIS}
	)
QT5_WRAP_UI(mcrecover_UIS_H ${mcrecover_UIS})

# Create MOC source files for classes that need them.
SET(mcrecover_MOC_H
	${mcrecover_MOC_H}
	${mcrecover_DB_MOC_H}
	${mcrecover_WINDOW_MOC_H}
	${mcrecover_WIDGET_MOC_H}
	${mcrecover_EDIT_MOC_H}
	${mcrecover_SEKRIT_MOC_H}
	)
QT5_WRAP_CPP(mcrecover_MOC_SRCS ${mcrecover_MOC_H})

# TaskbarButtonManager
SET(mcrecover_TBM_SRCS
	TaskbarButtonManager/TaskbarButtonManager.cpp
	TaskbarButtonManager/TaskbarButtonManagerFactory.cpp
	)
SET(mcrecover_TBM_MOC_H
	TaskbarButtonManager/TaskbarButtonManager.hpp
	)
SET(mcrecover_TBM_H TaskbarButtonManager/TaskbarButtonManager_p.hpp)

IF(WIN32)
	# Windows: Use ITaskbarList3. (Win7+)
	SET(mcrecover_TBM_SRCS
		${mcrecover_TBM_SRCS}
		TaskbarButtonManager/Win7TaskbarList.cpp
		)
	SET(mcrecover_TBM_MOC_H
		${mcrecover_TBM_MOC_H}
		TaskbarButtonManager/Win7TaskbarList.hpp
		)
ELSEIF(UNIX AND NOT APPLE)
	# Unity Launcher API.
	# dlopen() is required.
	SET(mcrecover_NEEDS_DL 1)
	SET(mcrecover_TBM_SRCS
		${mcrecover_TBM_SRCS}
		TaskbarButtonManager/UnityLauncher.cpp
		)
	SET(mcrecover_TBM_MOC_H
		${mcrecover_TBM_MOC_H}
		TaskbarButtonManager/UnityLauncher.hpp
		)

	IF(QtDBus_FOUND)
		SET(DBus_XML_SRCS
			dbus/net.launchpad.DockItem.xml
			dbus/net.launchpad.DockManager.xml
			)
		SET_SOURCE_FILES_PROPERTIES(${DBus_XML_SRCS} PROPERTIES
			INCLUDE dbus/DBusMetatypes.hpp
			)

		# Enable DockManager / Unity API support.
		# NOTE: The D-BUS interfaces are generated in ${CMAKE_CURRENT_BINARY_DIR},
		# *not* ${CMAKE_CURRENT_BINARY_DIR}/dbus/.
		QT5_ADD_DBUS_INTERFACES(
			mcrecover_DBUS_INTERFACE_SRCS
			${DBus_XML_SRCS}
			)
		SET(mcrecover_TBM_SRCS
			${mcrecover_TBM_SRCS}
			${mcrecover_DBUS_INTERFACE_SRCS}
			TaskbarButtonManager/DockManager.cpp
			)
		SET(mcrecover_TBM_MOC_H
			${mcrecover_TBM_MOC_H}
			TaskbarButtonManager/DockManager.hpp
			)
	ENDIF(QtDBus_FOUND)
ENDIF()

IF(mcrecover_TBM_MOC_H)
	# Create MOC source files for classes that need them.
	QT5_WRAP_CPP(mcrecover_TBM_MOC_SRCS ${mcrecover_TBM_MOC_H})
ENDIF(mcrecover_TBM_MOC_H)

######################
# Qt resource files. #
######################

SET(mcrecover_RCC_SRCS
	resources/mcrecover/mcrecover.qrc
	resources/oxygen/oxygen.qrc
	resources/flags/flags.qrc
	resources/sonic/sonic.qrc
	resources/hw/hw.qrc
	)

QT5_ADD_RESOURCES(
	mcrecover_RCC_O ${mcrecover_RCC_SRCS}
	OPTIONS -no-compress
	)

#########################
# Build the executable. #
#########################

# TODO: Set Win32 when compiling release build
# to disable the command prompt window.
ADD_EXECUTABLE(mcrecover WIN32 MACOSX_BUNDLE
	${mcrecover_SRCS}
	${mcrecover_DB_SRCS} ${mcrecover_DB_H}
	${mcrecover_WINDOW_SRCS}
	${mcrecover_WIDGET_SRCS}
	${mcrecover_EDIT_SRCS} ${mcrecover_EDIT_H}
	${mcrecover_SEKRIT_SRCS}
	${mcrecover_OS_SRCS}
	${mcrecover_MOC_SRCS}
	${mcrecover_UIS_H}
	${mcrecover_RCC_O}
	${mcrecover_TBM_SRCS} ${mcrecover_TBM_H}
	${mcrecover_TBM_MOC_SRCS}
	)
ADD_DEPENDENCIES(mcrecover git_version)
DO_SPLIT_DEBUG(mcrecover)
SET_WINDOWS_SUBSYSTEM(mcrecover WIN32)
SET_WINDOWS_NO_MANIFEST(mcrecover)
SET_WINDOWS_ENTRYPOINT(mcrecover WinMain OFF)

TARGET_INCLUDE_DIRECTORIES(mcrecover
	PUBLIC	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	PRIVATE	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>
	)

# Other GCN MemCard Recover libraries.
# TODO: Make libsaveedit optional?
TARGET_LINK_LIBRARIES(mcrecover gctools memcard saveedit)

# extlib
SET(MCRECOVER_EXTLIB
	${PNG_LIBRARY}	# Required for libpng info in AboutDialog.
	${ZLIB_LIBRARY}	# Required for zlib info in AboutDialog.
	)
TARGET_LINK_LIBRARIES(mcrecover ${MCRECOVER_EXTLIB})

# Qt libraries
# NOTE: Libraries have to be linked in reverse order.
IF(QtDBus_FOUND)
	TARGET_LINK_LIBRARIES(mcrecover Qt5::DBus)
ENDIF(QtDBus_FOUND)
TARGET_LINK_LIBRARIES(mcrecover Qt5::Widgets Qt5::Gui Qt5::Core)

# extlib (needed twice due to weird dependencies with Qt/static)
TARGET_LINK_LIBRARIES(mcrecover ${MCRECOVER_EXTLIB})

# OS-specific libraries
TARGET_LINK_LIBRARIES(mcrecover ${WIN32_LIBS} ${APPLE_LIBS})

# Link in libdl if it's required for dlopen()
# and we have a component that uses it.
IF(mcrecover_NEEDS_DL AND CMAKE_DL_LIBS)
	TARGET_LINK_LIBRARIES(mcrecover ${CMAKE_DL_LIBS})
ENDIF(mcrecover_NEEDS_DL AND CMAKE_DL_LIBS)

# Translations.
IF(ENABLE_NLS)
	ADD_DEPENDENCIES(mcrecover translations_target)
ENDIF(ENABLE_NLS)

# Compress the EXE.
IF(COMPRESS_EXE)
	INCLUDE(CompressExeWithUpx)
	COMPRESS_EXE_WITH_UPX(mcrecover)
ENDIF(COMPRESS_EXE)

# Define -DQT_NO_DEBUG in release builds.
SET(CMAKE_C_FLAGS_RELEASE   "-DQT_NO_DEBUG ${CMAKE_C_FLAGS_RELEASE}")
SET(CMAKE_CXX_FLAGS_RELEASE "-DQT_NO_DEBUG ${CMAKE_CXX_FLAGS_RELEASE}")

# Qt options:
# - Fast QString concatenation. (Qt 4.6+, plus 4.8-specific version)
# - Disable implicit QString ASCII casts.
ADD_DEFINITIONS(-DQT_USE_FAST_CONCATENATION
	-DQT_USE_FAST_OPERATOR_PLUS
	-DQT_USE_QSTRINGBUILDER
	-DQT_NO_CAST_FROM_ASCII
	-DQT_NO_CAST_TO_ASCII
	-DQT_STRICT_ITERATORS
	-DQT_NO_URL_CAST_FROM_STRING
	)

# Win32: image version.
INCLUDE(Win32ImageVersionLinkerFlags)
WIN32_IMAGE_VERSION_LINKER_FLAGS(${VERSION_MAJOR} ${VERSION_MINOR})

# Mac OS X bundle information.
SET(MACOSX_BUNDLE_GUI_IDENTIFIER "gsft.gerbilsoft.McRecover")
SET(MACOSX_BUNDLE_BUNDLE_NAME "GCN MemCard Recover")
#SET(MACOSX_BUNDLE_ICON_FILE "mcrecover.icns")
SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "0.0.0")
SET(MACOSX_BUNDLE_LONG_VERSION_STRING "0.0.0")

# Mac OS X: Set a custom info.plist file for the application bundle.
# TODO
#SET_TARGET_PROPERTIES(mcrecover
#	PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/mac/Info-CMake.plist")

#################
# Installation. #
#################

INSTALL(TARGETS mcrecover
	RUNTIME DESTINATION "${DIR_INSTALL_EXE}"
	LIBRARY DESTINATION "${DIR_INSTALL_DLL}"
	ARCHIVE DESTINATION "${DIR_INSTALL_LIB}"
	COMPONENT "program"
	)
# Check if a split debug file should be installed.
IF(INSTALL_DEBUG)
	# FIXME: Generator expression $<TARGET_PROPERTY:${_target},PDB> didn't work with CPack-3.6.1.
	GET_TARGET_PROPERTY(DEBUG_FILENAME mcrecover PDB)
	IF(DEBUG_FILENAME)
		INSTALL(FILES "${DEBUG_FILENAME}"
			DESTINATION "${DIR_INSTALL_EXE_DEBUG}"
			COMPONENT "debug"
			)
	ENDIF(DEBUG_FILENAME)
ENDIF(INSTALL_DEBUG)

# FreeDesktop.org icon specification.
IF(UNIX AND NOT APPLE)
	FOREACH(ICON_SIZE 16x16 22x22 24x24 32x32 48x48 64x64 128x128)
		INSTALL(FILES resources/mcrecover/${ICON_SIZE}/mcrecover.png
			DESTINATION share/icons/hicolor/${ICON_SIZE}/apps
			COMPONENT "desktop-icon"
			)
	ENDFOREACH()

	# FreeDesktop.org desktop file specification.
	INSTALL(FILES resources/mcrecover/xdg/mcrecover.desktop
		DESTINATION share/applications
		COMPONENT "desktop-icon"
		)
	# /usr/share/pixmaps
	INSTALL(FILES resources/mcrecover/xdg/mcrecover.xpm
		DESTINATION share/pixmaps
		COMPONENT "desktop-icon"
		)
ENDIF(UNIX AND NOT APPLE)
