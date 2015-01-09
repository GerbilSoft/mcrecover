PROJECT(mcrecover)
cmake_minimum_required(VERSION 2.6)

# mcrecover source and binary directories.
# Binary directory is required for moc-generated headers.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../")

# libgctools source and binary directories.
# Binary directory is needed for byteorder.h.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../libgctools")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../libgctools")

# Data directory.
IF(WIN32 OR APPLE)
	# Win32 and Mac OS X don't use `make install`.
	SET(MCRECOVER_DATA_DIRECTORY "")
	SET(MCRECOVER_TRANSLATIONS_DIRECTORY "")
ELSE()
	# Unix system.
	SET(MCRECOVER_DATA_DIRECTORY "${CMAKE_INSTALL_PREFIX}/share/mcrecover/data")
	SET(MCRECOVER_TRANSLATIONS_DIRECTORY "${CMAKE_INSTALL_PREFIX}/share/mcrecover/translations")
ENDIF()

# Find Qt4.
# Components used:
# - QtCore
# - QtGui
# - QtXml
# - QtDBus (optional; used for DockManager / Unity API)
SET(MCRECOVER_QT4_MIN_VERSION 4.6.0)
SET(MCRECOVER_QT_COMPONENTS QtCore QtGui QtXml)
IF(ENABLE_DBUS)
	SET(MCRECOVER_QT_COMPONENTS ${MCRECOVER_QT_COMPONENTS} QtDBus)
ENDIF(ENABLE_DBUS)

FIND_PACKAGE(Qt4 ${MCRECOVER_QT4_MIN_VERSION}
	COMPONENTS ${MCRECOVER_QT_COMPONENTS})
IF(NOT QT_QTCORE_FOUND)
	MESSAGE(FATAL_ERROR "QtCore not found! Please install Qt ${MCRECOVER_QT4_MIN_VERSION} or higher.")
ELSEIF(NOT QT_QTGUI_FOUND)
	MESSAGE(FATAL_ERROR "QtGui not found! Please install Qt ${MCRECOVER_QT4_MIN_VERSION} or higher.")
ENDIF()

INCLUDE(${QT_USE_FILE})

####################################
# Qt plugins. (Static build only!) #
####################################
IF(QT_IS_STATIC)
	IF(QT_QTCORE_LIBRARY_DEBUG)
		SET(QT_PLUGINS_SUFFIX "d")
	ENDIF(QT_QTCORE_LIBRARY_DEBUG)
	FIND_LIBRARY(QT_PLUGINS_CODECS_QJPCODECS
		qjpcodecs${QT_PLUGINS_SUFFIX}
		PATHS ${QT_PLUGINS_DIR}/codecs)
	IF(NOT QT_PLUGINS_CODECS_QJPCODECS)
		MESSAGE(WARNING "qjpcodecs not found; Japanese text will not be decoded properly.")
	ENDIF()
ENDIF(QT_IS_STATIC)

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

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES("${mcrecover_base_BINARY_DIR}")

# PCRE
INCLUDE(CheckPCRE)
INCLUDE_DIRECTORIES(${PCRE_INCLUDE_DIR})

# libpng (AboutDialog only; indirectly through libgctools)
INCLUDE(CheckPNG)
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})

# zlib (AboutDialog only!)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.mcrecover.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.mcrecover.h")

# Sources.
SET(mcrecover_SRCS
	mcrecover.cpp
	GcnMcFileDb.cpp
	SearchThread.cpp
	SearchThreadWorker.cpp
	PcreRegex.cpp
	GcnDateTime.cpp
	IconAnimHelper.cpp
	McRecoverQApplication.cpp
	VarReplace.cpp
	TaskbarButtonManager/TaskbarButtonManager.cpp
	GcToolsQt.cpp
	TranslationManager.cpp
	config/ConfigStore.cpp
	config/ConfigDefaults.cpp
	PathFuncs.cpp
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
	widgets/MemCardFileView.cpp
	widgets/CardView.cpp
	widgets/QTreeViewOpt.cpp
	widgets/MessageWidget.cpp
	widgets/TableSelect.cpp
	widgets/MessageWidgetStack.cpp
	)

SET(mcrecover_CARD_SRCS
	card/Card.cpp
	card/GcnCard.cpp
	card/File.cpp
	card/GcnFile.cpp
	card/MemCardModel.cpp
	card/MemCardItemDelegate.cpp
	card/MemCardSortFilterProxyModel.cpp
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

# Headers with Qt4 objects.
SET(mcrecover_MOC_H
	GcnMcFileDb.hpp
	SearchThread.hpp
	SearchThreadWorker.hpp
	IconAnimHelper.hpp
	McRecoverQApplication.hpp
	TaskbarButtonManager/TaskbarButtonManager.hpp
	config/ConfigStore.hpp
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
	widgets/MemCardFileView.hpp
	widgets/CardView.hpp
	widgets/QTreeViewOpt.hpp
	widgets/MessageWidget.hpp
	widgets/TableSelect.hpp
	widgets/MessageWidgetStack.hpp
	)

SET(mcrecover_CARD_MOC_H
	card/Card.hpp
	card/GcnCard.hpp
	card/File.hpp
	card/GcnFile.hpp
	card/MemCardModel.hpp
	card/MemCardItemDelegate.hpp
	card/MemCardSortFilterProxyModel.hpp
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
	widgets/MemCardFileView.ui
	widgets/CardView.ui
	widgets/TableSelect.ui
	)

# Generate the header files from the UI files.
QT4_WRAP_UI(mcrecover_UIS_H
	${mcrecover_WINDOW_UIS}
	${mcrecover_WIDGET_UIS}
	)

# Create MOC source files for classes that need them.
QT4_WRAP_CPP(mcrecover_MOC_SRCS
	${mcrecover_MOC_H}
	${mcrecover_WINDOW_MOC_H}
	${mcrecover_WIDGET_MOC_H}
	${mcrecover_CARD_MOC_H}
	${mcrecover_SEKRIT_MOC_H}
	)

IF(QT_QTDBUS_FOUND)
	# QtDBus found.
	# Enable DockManager / Unity API support.
	# NOTE: The D-BUS interfaces are generated in ${CMAKE_CURRENT_BINARY_DIR},
	# *not* ${CMAKE_CURRENT_BINARY_DIR}/dbus/.
	INCLUDE(QtDBusInterfaceNoNS)
	QT4_ADD_DBUS_INTERFACES_NONS(
		mcrecover_DBUS_SRCS
		INCLUDE dbus/DBusMetatypes.hpp
		dbus/net.launchpad.DockItem.xml
		dbus/net.launchpad.DockManager.xml
		)

	IF(UNIX AND NOT APPLE)
		SET(mcrecover_DBUS_SRCS
			${mcrecover_DBUS_SRCS}
			TaskbarButtonManager/DockManager.cpp
			)
		SET(mcrecover_DBUS_MOC_H
			TaskbarButtonManager/DockManager.hpp
			)

		# Create MOC source files for classes that need them.
		QT4_WRAP_CPP(mcrecover_DBUS_MOC_SRCS ${mcrecover_DBUS_MOC_H})
	ENDIF(UNIX AND NOT APPLE)
ENDIF(QT_QTDBUS_FOUND)

#################
# Translations. #
#################

ADD_SUBDIRECTORY(translations)
# TODO: Mac OS X bundle location.
# TODO: "make install" location.

######################
# Qt resource files. #
######################

SET(mcrecover_RCC_SRCS
	resources/mcrecover/mcrecover.qrc
	resources/oxygen/oxygen.qrc
	resources/flags/flags.qrc
	)

# Add the resource files to the project.
QT4_ADD_RESOURCES(
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
	${mcrecover_WINDOW_SRCS}
	${mcrecover_WIDGET_SRCS}
	${mcrecover_CARD_SRCS}
	${mcrecover_SEKRIT_SRCS}
	${mcrecover_OS_SRCS}
	${mcrecover_MOC_SRCS}
	${mcrecover_UIS_H}
	${mcrecover_RCC_O}
	${mcrecover_DBUS_SRCS}
	${mcrecover_DBUS_MOC_SRCS}
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(mcrecover)

# libgctools
TARGET_LINK_LIBRARIES(mcrecover gctools)

# extlib
SET(MCRECOVER_EXTLIB
	${PNG_LIBRARY}	# Required for libpng info in AboutDialog.
	${ZLIB_LIBRARY}	# Required for zlib info in AboutDialog.
	${PCRE_PCRE_LIBRARY}
	)
TARGET_LINK_LIBRARIES(mcrecover ${MCRECOVER_EXTLIB})

# Qt libraries
# NOTE: Libraries have to be linked in reverse order.
IF(QT_PLUGINS_CODECS_QJPCODECS)
	TARGET_LINK_LIBRARIES(mcrecover ${QT_PLUGINS_CODECS_QJPCODECS})
ENDIF(QT_PLUGINS_CODECS_QJPCODECS)
IF(QT_QTDBUS_FOUND)
	TARGET_LINK_LIBRARIES(mcrecover ${QT_QTDBUS_LIBRARY})
ENDIF(QT_QTDBUS_FOUND)
TARGET_LINK_LIBRARIES(mcrecover
	${QT_QTGUI_LIBRARY}
	${QT_QTCORE_LIBRARY}
	)

# extlib (needed twice due to weird dependencies with Qt/static)
TARGET_LINK_LIBRARIES(mcrecover ${MCRECOVER_EXTLIB})

# OS-specific libraries
TARGET_LINK_LIBRARIES(mcrecover
	${WIN32_LIBS}
	${APPLE_LIBS}
	)

# Translations.
ADD_DEPENDENCIES(mcrecover translations_target)

# NOTE: $<TARGET_FILE:mcrecover> is preferred,
# but this doesn't seem to work on Ubuntu 10,04.
# (cmake_2.8.0-5ubuntu1_i386)
GET_PROPERTY(MCRECOVER_EXE_LOCATION TARGET mcrecover PROPERTY LOCATION)

# Split debug information.
IF(SPLIT_DEBUG)
	INCLUDE(SplitDebugInformation)
	SPLIT_DEBUG_INFORMATION(mcrecover)
ENDIF(SPLIT_DEBUG)

# Compress the EXE.
IF(COMPRESS_EXE)
	INCLUDE(CompressExeWithUpx)
	COMPRESS_EXE_WITH_UPX(mcrecover)
ENDIF(COMPRESS_EXE)

# Qt options:
# - Fast QString concatenation. (Qt 4.6+, plus 4.8-specific version)
# - Disable implicit QString ASCII casts.
SET_TARGET_PROPERTIES(mcrecover PROPERTIES
	COMPILE_DEFINITIONS "QT_USE_FAST_CONCATENATION;QT_USE_FAST_OPERATOR_PLUS;QT_USE_QSTRINGBUILDER;QT_NO_CAST_FROM_ASCII;QT_NO_CAST_TO_ASCII")

# PCRE compile flags.
SET_TARGET_PROPERTIES(mcrecover PROPERTIES
	COMPILE_FLAGS "${PCRE_DEFINITIONS}")

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

INCLUDE(DirInstallPaths)

INSTALL(TARGETS mcrecover
	RUNTIME DESTINATION "${DIR_INSTALL_EXE}"
	LIBRARY DESTINATION "${DIR_INSTALL_DLL}"
	ARCHIVE DESTINATION "${DIR_INSTALL_LIB}"
	COMPONENT "program"
	)
IF(INSTALL_DEBUG)
IF(MSVC)
	INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/mcrecover.pdb"
		DESTINATION "${DIR_INSTALL_EXE_DEBUG}"
		COMPONENT "debug"
		)
ELSEIF(SPLIT_DEBUG)
	INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/mcrecover.debug"
		DESTINATION "${DIR_INSTALL_EXE_DEBUG}"
		COMPONENT "debug"
		)
ENDIF()
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

# System translations.
# Install on Windows and Mac OS X, or if we're using statically-linked Qt.
# Dynamically-linked Qt uses system translations.
IF(QT_IS_STATIC OR APPLE OR NOT UNIX)
	FILE(GLOB QT_SYS_TRANSLATIONS "${QT_TRANSLATIONS_DIR}/qt_??.qm")
	INSTALL(FILES ${QT_SYS_TRANSLATIONS}
		DESTINATION ${DIR_INSTALL_TRANSLATIONS}
		COMPONENT "translations"
		)
ENDIF(QT_IS_STATIC OR APPLE OR NOT UNIX)
