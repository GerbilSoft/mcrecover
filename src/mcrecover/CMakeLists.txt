PROJECT(mcrecover)
cmake_minimum_required(VERSION 2.6)

# McRecover source and binary directories.
# Binary directory is needed for byteorder.h.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# Data directory.
IF(WIN32 OR APPLE)
	# Win32 and Mac OS X don't use `make install`.
	SET(MCRECOVER_DATA_DIRECTORY "")
ELSE()
	# Unix system.
	SET(MCRECOVER_DATA_DIRECTORY "${CMAKE_INSTALL_PREFIX}/share/mcrecover/data")
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
ELSEIF(NOT QT_QTXML_FOUND)
	MESSAGE(FATAL_ERROR "QtXml not found! Please install Qt ${MCRECOVER_QT4_MIN_VERSION} or higher.")
ENDIF()

INCLUDE(${QT_USE_FILE})

# PCRE
FIND_PACKAGE(PCRE REQUIRED)
INCLUDE_DIRECTORIES(PCRE_INCLUDE_DIR)

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

# Test system byteorder.
# TODO: Universal Binary support; detect PDP endian.
INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
IF(IS_BIG_ENDIAN)
        SET(MCRECOVER_BYTEORDER "MCRECOVER_BIG_ENDIAN")
ELSE(IS_BIG_ENDIAN)
        SET(MCRECOVER_BYTEORDER "MCRECOVER_LIL_ENDIAN")
ENDIF(IS_BIG_ENDIAN)
UNSET(IS_BIG_ENDIAN)
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/util/byteorder.h.in" "${CMAKE_CURRENT_BINARY_DIR}/util/byteorder.h")

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.mcrecover.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.mcrecover.h")

# Sources.
SET(mcrecover_SRCS
	mcrecover.cpp
	MemCard.cpp
	MemCardFile.cpp
	McRecoverWindow.cpp
	MemCardModel.cpp
	GcImage.cpp
	GcnMcFileDb.cpp
	SearchThread.cpp
	StatusBarManager.cpp
	SearchThreadWorker.cpp
	PcreRegex.cpp
	GcnDateTime.cpp
	MemCardFileView.cpp
	Checksum.cpp
	MemCardView.cpp
	IconAnimHelper.cpp
	McRecoverQApplication.cpp
	AboutDialog.cpp
	VarReplace.cpp
	MemCardItemDelegate.cpp
	FileComments.cpp
	QTreeViewOpt.cpp
	)

# Win32-specific sources.
IF(WIN32)
	SET(mcrecover_SRCS
		${mcrecover_SRCS}
		McRecoverQApplication_win32.cpp
		ui/resources/win32/mcrecover.rc
		)
ENDIF(WIN32)

# Headers with Qt4 objects.
SET(mcrecover_MOC_H
	MemCard.hpp
	MemCardFile.hpp
	McRecoverWindow.hpp
	MemCardModel.hpp
	GcnMcFileDb.hpp
	SearchThread.hpp
	StatusBarManager.hpp
	SearchThreadWorker.hpp
	MemCardFileView.hpp
	MemCardView.hpp
	IconAnimHelper.hpp
	McRecoverQApplication.hpp
	AboutDialog.hpp
	MemCardItemDelegate.hpp
	QTreeViewOpt.hpp
	)

# UI files.
SET(mcrecover_UIS
	ui/McRecoverWindow.ui
	ui/MemCardFileView.ui
	ui/MemCardView.ui
	ui/AboutDialog.ui
	)

# Generate the header files from the UI files.
QT4_WRAP_UI(mcrecover_UIS_H ${mcrecover_UIS})

# Create MOC source files for classes that need them.
QT4_WRAP_CPP(mcrecover_MOC_SRCS ${mcrecover_MOC_H})

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

# TODO
#ADD_SUBDIRECTORY(translations)
# TODO: Mac OS X bundle location.
# TODO: "make install" location.

######################
# Qt resource files. #
######################

SET(mcrecover_RCC_SRCS
	ui/resources/mcrecover/mcrecover.qrc
	ui/resources/oxygen/oxygen.qrc
	ui/resources/flags/flags.qrc
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
	${mcrecover_MOC_SRCS}
	${mcrecover_UIS_H}
	${mcrecover_RCC_O}
	${mcrecover_DBUS_SRCS}
	${mcrecover_DBUS_MOC_SRCS}
	)
TARGET_LINK_LIBRARIES(mcrecover
	${QT_XML_LIBRARY}
	${QT_QTGUI_LIBRARY}
	${QT_QTCORE_LIBRARY}
	${PCRE_PCRE_LIBRARY}
	${WIN32_LIBS}
	${APPLE_LIBS}
	)
IF(QT_QTDBUS_FOUND)
	TARGET_LINK_LIBRARIES(mcrecover ${QT_QTDBUS_LIBRARY})
ENDIF(QT_QTDBUS_FOUND)

# TODO: Translations.
#ADD_DEPENDENCIES(gens-qt4 translations_target)

# NOTE: $<TARGET_FILE:mcrecover> is preferred,
# but this doesn't seem to work on Ubuntu 10,04.
# (cmake_2.8.0-5ubuntu1_i386)
GET_PROPERTY(MCRECOVER_EXE_LOCATION TARGET mcrecover PROPERTY LOCATION)

# Split debug information.
# References:
# - http://cmake.3232098.n2.nabble.com/Save-stripped-debugging-information-td6819195.html
# - http://sourceware.org/bugzilla/show_bug.cgi?id=14527
#   - If debug symbols are stripped before .gnu_debuglink is added,
#     the section will be truncated to .gnu_deb, and hence won't
#     be recognized by gdb.
# - FIXME: If the above .gnu_debuglink workaround is used, Windows XP
#   and Windows 7 will claim that the executable isn't a valid Win32
#   executable. (Wine ignores it and works fine!)
IF(SPLIT_DEBUG)
	FIND_PROGRAM(OBJCOPY objcopy)
	ADD_CUSTOM_COMMAND(TARGET mcrecover POST_BUILD
		COMMAND ${OBJCOPY} --only-keep-debug
			${MCRECOVER_EXE_LOCATION} ${CMAKE_CURRENT_BINARY_DIR}/mcrecover.debug
		COMMAND ${OBJCOPY} --strip-debug
			${MCRECOVER_EXE_LOCATION}
		COMMAND ${OBJCOPY} --add-gnu-debuglink=mcrecover.debug
			${MCRECOVER_EXE_LOCATION}
		)
ENDIF(SPLIT_DEBUG)

IF(COMPRESS_EXE)
	FIND_PROGRAM(UPX upx)
	IF(WIN32)
		SET(UPX_OPTIONS "--compress-icons=0")
	ENDIF(WIN32)
	IF(UPX)
		ADD_CUSTOM_COMMAND(TARGET mcrecover POST_BUILD
			COMMAND ${UPX} --best --lzma
				${MCRECOVER_EXE_LOCATION}
			)
	ENDIF(UPX)
ENDIF(COMPRESS_EXE)

# Qt options:
# - Fast QString concatenation. (Qt 4.6+, plus 4.8-specific version)
# - Disable implicit QString ASCII casts.
SET_TARGET_PROPERTIES(mcrecover PROPERTIES
	COMPILE_DEFINITIONS "QT_USE_FAST_CONCATENATION;QT_USE_FAST_OPERATOR_PLUS;QT_USE_QSTRINGBUILDER;QT_NO_CAST_FROM_ASCII;QT_NO_CAST_TO_ASCII")

# PCRE compile flags.
SET_TARGET_PROPERTIES(mcrecover PROPERTIES
	COMPILE_FLAGS "${PCRE_DEFINITIONS}")

# Mac OS X bundle information.
SET(MACOSX_BUNDLE_GUI_IDENTIFIER "gsft.gerbilsoft.McRecover")
SET(MACOSX_BUNDLE_BUNDLE_NAME "GCN MemCard Recover")
#SET(MACOSX_BUNDLE_ICON_FILE "mcrecover.icns")
SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "0.0.0")
SET(MACOSX_BUNDLE_LONG_VERSION_STRING "0.0.0")

# Mac OS X: Set a custom info.plist file for the application bundle.
# TODO
#SET_TARGET_PROPERTIES(mcrecover
#	PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/ui/resources/mac/Info-CMake.plist")

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
	INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/mcrecover.debug"
		DESTINATION "${DIR_INSTALL_EXE_DEBUG}"
		COMPONENT "debug"
		)
ENDIF(INSTALL_DEBUG)

# FreeDesktop.org icon specification.
IF(UNIX AND NOT APPLE)
	FOREACH(ICON_SIZE 16x16 22x22 24x24 32x32 48x48 64x64 128x128)
		INSTALL(FILES ui/resources/mcrecover/${ICON_SIZE}/mcrecover.png
			DESTINATION share/icons/hicolor/${ICON_SIZE}/apps
			COMPONENT "desktop-icon"
			)
	ENDFOREACH()

	# FreeDesktop.org desktop file specification.
	INSTALL(FILES ui/resources/mcrecover/xdg/mcrecover.desktop
		DESTINATION share/applications
		COMPONENT "desktop-icon"
		)
	# /usr/share/pixmaps
	INSTALL(FILES ui/resources/mcrecover/xdg/mcrecover.xpm
		DESTINATION share/pixmaps
		COMPONENT "desktop-icon"
		)
ENDIF(UNIX AND NOT APPLE)
