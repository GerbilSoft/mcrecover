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

IF(USE_QT4)
	# Find Qt 4.
	SET(MCRECOVER_QT4_MIN_VERSION 4.6.0)
	SET(MCRECOVER_QT4_COMPONENTS QtCore QtGui QtXml)
	IF(ENABLE_DBUS)
		# QtDBus is used for DockManager.
		# TODO: Unity API?
		SET(MCRECOVER_QT4_COMPONENTS ${MCRECOVER_QT4_COMPONENTS} QtDBus)
	ENDIF(ENABLE_DBUS)

	FIND_PACKAGE(Qt4 ${MCRECOVER_QT4_MIN_VERSION}
		COMPONENTS ${MCRECOVER_QT4_COMPONENTS})
	IF(NOT QT_QTCORE_FOUND)
		MESSAGE(FATAL_ERROR "QtCore not found! Please install Qt ${MCRECOVER_QT4_MIN_VERSION} or higher.")
	ELSEIF(NOT QT_QTGUI_FOUND)
		MESSAGE(FATAL_ERROR "QtGui not found! Please install Qt ${MCRECOVER_QT4_MIN_VERSION} or higher.")
	ENDIF()

	IF(QT_QTDBUS_FOUND)
		SET(QtDBus_FOUND 1)
	ENDIF(QT_QTDBUS_FOUND)

	INCLUDE(${QT_USE_FILE})
ELSEIF(USE_QT5)
	# Find Qt 5.
	# FIXME: Figure out a minimum Qt5 version.
	SET(QT5_EXECUTABLE_COMPILE_FLAGS "")
	FOREACH(QT5_MODULE Qt5Core Qt5Gui Qt5Widgets Qt5Xml)
		FIND_PACKAGE(${QT5_MODULE} REQUIRED)
		INCLUDE_DIRECTORIES(${${QT5_MODULE}_INCLUDE_DIRS})

		# If Qt 5 is built with -reduce-relocations, we must build with -fPIC.
		# Qt's CMake modules provide the CFLAGS for this.
		SET(QT5_EXECUTABLE_COMPILE_FLAGS "${QT5_EXECUTABLE_COMPILE_FLAGS} ${${QT5_MODULE}_EXECUTABLE_COMPILE_FLAGS}")
	ENDFOREACH()

	IF(ENABLE_DBUS)
		# QtDBus is used for DockManager.
		FIND_PACKAGE(Qt5DBus)
		# FIXME: HAVE_Qt5DBus isn't working...
		IF(Qt5DBus_FOUND)
			SET(QtDBus_FOUND 1)
			INCLUDE_DIRECTORIES(${Qt5DBus_INCLUDE_DIRS})

			# If Qt 5 is built with -reduce-relocations, we must build with -fPIC.
			# Qt's CMake modules provide the CFLAGS for this.
			SET(QT5_EXECUTABLE_COMPILE_FLAGS "${QT5_EXECUTABLE_COMPILE_FLAGS} ${Qt5DBus_EXECUTABLE_COMPILE_FLAGS}")
		ENDIF(Qt5DBus_FOUND)
	ENDIF(ENABLE_DBUS)

	# Add Qt 5 compile flags.
	# TODO: Also linker flags?
	SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${QT5_EXECUTABLE_COMPILE_FLAGS}")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${QT5_EXECUTABLE_COMPILE_FLAGS}")
ENDIF()

####################################
# Qt plugins. (Static build only!) #
####################################
IF(QT_IS_STATIC)
	IF(USE_QT4)
		IF(QT_QTCORE_LIBRARY_DEBUG)
			SET(QT_PLUGINS_SUFFIX "d")
		ENDIF(QT_QTCORE_LIBRARY_DEBUG)
		FIND_LIBRARY(QT_PLUGINS_CODECS_QJPCODECS
			qjpcodecs${QT_PLUGINS_SUFFIX}
			PATHS ${QT_PLUGINS_DIR}/codecs)
		IF(QT_PLUGINS_CODECS_QJPCODECS)
			SET(HAVE_QT_STATIC_PLUGIN_QJPCODECS 1)
		ELSE()
			MESSAGE(WARNING "qjpcodecs not found; Japanese text will not be decoded properly.")
		ENDIF()
	ELSEIF(USE_QT5)
		MESSAGE(FATAL_ERROR "TODO: qjpcodecs static linkage for Qt 5.")
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
ADD_DEFINITIONS(${PCRE_DEFINITIONS})

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
	GcnMcFileDb.cpp
	SearchThread.cpp
	SearchThreadWorker.cpp
	PcreRegex.cpp
	GcnDateTime.cpp
	IconAnimHelper.cpp
	McRecoverQApplication.cpp
	VarReplace.cpp
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
	widgets/FileView.cpp
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
	card/VmuCard.cpp
	card/VmuFile.cpp
	)

# File edtors.
SET(mcrecover_EDIT_SRCS
	edit/EditorWindow.cpp
	edit/EditorWidget.cpp
	edit/EditorWidgetFactory.cpp
	edit/SonicAdventure/SAEditor.cpp
	edit/SonicAdventure/SALevelStats.cpp
	edit/SonicAdventure/SALevelClearCount.cpp
	edit/SonicAdventure/SAEventFlags.cpp
	edit/SonicAdventure/BitFlagsModel.cpp
	edit/SonicAdventure/BitFlagsView.cpp
	edit/SonicAdventure/PageFilterModel.cpp
	edit/SonicAdventure/BitFlags.cpp
	edit/SonicAdventure/SANPCFlags.cpp
	edit/SonicAdventure/ByteFlags.cpp
	edit/SonicAdventure/SADXMissionFlags.cpp
	edit/SonicAdventure/ByteFlagsModel.cpp
	edit/SonicAdventure/ByteFlagsView.cpp
	edit/SonicAdventure/CenteredCheckBoxDelegate.cpp
	edit/SonicAdventure/SAGeneral.cpp
	edit/SonicAdventure/SADataEvents.c
	edit/SonicAdventure/SADXDataMissions.c
	edit/SonicAdventure/SADataLevels.c
	edit/SonicAdventure/SADataUI.c
	edit/SonicAdventure/SAAdventure.cpp
	edit/SonicAdventure/SAMiscEmblems.cpp
	edit/SonicAdventure/TimeCodeEdit.cpp
	edit/SonicAdventure/SASubGames.cpp
	)
SET(mcrecover_EDIT_H
	edit/EditorWidgetFactory.hpp
	edit/SonicAdventure/SAData.h
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
	widgets/FileView.hpp
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
	card/VmuCard.hpp
	card/VmuFile.hpp
	)

# File edtors.
SET(mcrecover_EDIT_MOC_H
	edit/EditorWindow.hpp
	edit/EditorWidget.hpp
	edit/SonicAdventure/SAEditor.hpp
	edit/SonicAdventure/SALevelStats.hpp
	edit/SonicAdventure/SALevelClearCount.hpp
	edit/SonicAdventure/SAEventFlags.hpp
	edit/SonicAdventure/BitFlagsModel.hpp
	edit/SonicAdventure/BitFlagsView.hpp
	edit/SonicAdventure/PageFilterModel.hpp
	edit/SonicAdventure/BitFlags.hpp
	edit/SonicAdventure/SANPCFlags.hpp
	edit/SonicAdventure/ByteFlags.hpp
	edit/SonicAdventure/SADXMissionFlags.hpp
	edit/SonicAdventure/ByteFlagsModel.hpp
	edit/SonicAdventure/ByteFlagsView.hpp
	edit/SonicAdventure/CenteredCheckBoxDelegate.hpp
	edit/SonicAdventure/SAGeneral.hpp
	edit/SonicAdventure/SAAdventure.hpp
	edit/SonicAdventure/SAMiscEmblems.hpp
	edit/SonicAdventure/TimeCodeEdit.hpp
	edit/SonicAdventure/SASubGames.hpp
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
	widgets/TableSelect.ui
	)

# File edtors.
SET(mcrecover_EDIT_UIS
	edit/EditorWindow.ui
	edit/SonicAdventure/SAEditor.ui
	edit/SonicAdventure/SALevelStats.ui
	edit/SonicAdventure/SALevelClearCount.ui
	edit/SonicAdventure/BitFlagsView.ui
	edit/SonicAdventure/ByteFlagsView.ui
	edit/SonicAdventure/SAGeneral.ui
	edit/SonicAdventure/SAAdventure.ui
	edit/SonicAdventure/SAMiscEmblems.ui
	edit/SonicAdventure/TimeCodeEdit.ui
	edit/SonicAdventure/SASubGames.ui
	)

# Generate the header files from the UI files.
SET(mcrecover_UIS
	${mcrecover_WINDOW_UIS}
	${mcrecover_WIDGET_UIS}
	${mcrecover_EDIT_UIS}
	)
IF(USE_QT4)
	QT4_WRAP_UI(mcrecover_UIS_H ${mcrecover_UIS})
ELSEIF(USE_QT5)
	QT5_WRAP_UI(mcrecover_UIS_H ${mcrecover_UIS})
ENDIF()

# Create MOC source files for classes that need them.
SET(mcrecover_MOC_H
	${mcrecover_MOC_H}
	${mcrecover_WINDOW_MOC_H}
	${mcrecover_WIDGET_MOC_H}
	${mcrecover_CARD_MOC_H}
	${mcrecover_EDIT_MOC_H}
	${mcrecover_SEKRIT_MOC_H}
	)
IF(USE_QT4)
	QT4_WRAP_CPP(mcrecover_MOC_SRCS ${mcrecover_MOC_H})
ELSEIF(USE_QT5)
	QT5_WRAP_CPP(mcrecover_MOC_SRCS ${mcrecover_MOC_H})
ENDIF()

# TaskbarButtonManager
SET(mcrecover_TBM_SRCS TaskbarButtonManager/TaskbarButtonManager.cpp)
SET(mcrecover_TBM_MOC_H TaskbarButtonManager/TaskbarButtonManager.hpp)
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
ELSEIF(QtDBus_FOUND AND UNIX AND NOT APPLE)
	IF(USE_QT4)
		# Qt4: QtDBus found.
		# Enable DockManager / Unity API support.
		# NOTE: The D-BUS interfaces are generated in ${CMAKE_CURRENT_BINARY_DIR},
		# *not* ${CMAKE_CURRENT_BINARY_DIR}/dbus/.
		INCLUDE(QtDBusInterfaceNoNS)
		QT4_ADD_DBUS_INTERFACES_NONS(
			mcrecover_TBM_SRCS
			INCLUDE dbus/DBusMetatypes.hpp
			dbus/net.launchpad.DockItem.xml
			dbus/net.launchpad.DockManager.xml
			)
		SET(mcrecover_TBM_SRCS
			${mcrecover_TBM_SRCS}
			TaskbarButtonManager/DockManager.cpp
			)
		SET(mcrecover_TBM_MOC_H
			${mcrecover_TBM_MOC_H}
			TaskbarButtonManager/DockManager.hpp
			)
	ELSEIF(USE_QT5)
		# Qt5: QtDBus found.
		# Enable DockManager / Unity API support.
		# NOTE: The D-BUS interfaces are generated in ${CMAKE_CURRENT_BINARY_DIR},
		# *not* ${CMAKE_CURRENT_BINARY_DIR}/dbus/.
		INCLUDE(QtDBusInterfaceNoNS)
		# TODO: Qt5 version.
		IF(0)
		INCLUDE(QtDBusInterfaceNoNS)
		QT4_ADD_DBUS_INTERFACES_NONS(
			mcrecover_TBM_SRCS
			INCLUDE dbus/DBusMetatypes.hpp
			dbus/net.launchpad.DockItem.xml
			dbus/net.launchpad.DockManager.xml
			)
		SET(mcrecover_TBM_SRCS
			${mcrecover_TBM_SRCS}
			TaskbarButtonManager/DockManager.cpp
			)
		SET(mcrecover_TBM_MOC_H
			${mcrecover_TBM_MOC_H}
			TaskbarButtonManager/DockManager.hpp
			)
		ENDIF(0)
	ENDIF()
ENDIF()

IF(mcrecover_TBM_MOC_H)
	# Create MOC source files for classes that need them.
	IF(USE_QT4)
		QT4_WRAP_CPP(mcrecover_TBM_MOC_SRCS ${mcrecover_TBM_MOC_H})
	ELSEIF(USE_QT5)
		QT5_WRAP_CPP(mcrecover_TBM_MOC_SRCS ${mcrecover_TBM_MOC_H})
	ENDIF()
ENDIF(mcrecover_TBM_MOC_H)

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
	resources/sonic/sonic.qrc
	)

IF(USE_QT4)
	QT4_ADD_RESOURCES(
		mcrecover_RCC_O ${mcrecover_RCC_SRCS}
		OPTIONS -no-compress
		)
ELSEIF(USE_QT5)
	QT5_ADD_RESOURCES(
		mcrecover_RCC_O ${mcrecover_RCC_SRCS}
		OPTIONS -no-compress
		)
ENDIF()

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
	${mcrecover_EDIT_SRCS} ${mcrecover_EDIT_H}
	${mcrecover_SEKRIT_SRCS}
	${mcrecover_OS_SRCS}
	${mcrecover_MOC_SRCS}
	${mcrecover_UIS_H}
	${mcrecover_RCC_O}
	${mcrecover_TBM_SRCS} ${mcrecover_TBM_H}
	${mcrecover_TBM_MOC_SRCS}
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(mcrecover)

# libgctools
TARGET_LINK_LIBRARIES(mcrecover gctools)

# extlib
SET(MCRECOVER_EXTLIB
	${PNG_LIBRARY}	# Required for libpng info in AboutDialog.
	${ZLIB_LIBRARY}	# Required for zlib info in AboutDialog.
	)
TARGET_LINK_LIBRARIES(mcrecover ${MCRECOVER_EXTLIB})

# PCRE
IF(HAVE_PCRE16)
	TARGET_LINK_LIBRARIES(mcrecover ${PCRE_PCRE16_LIBRARY})
ELSE(HAVE_PCRE16)
	TARGET_LINK_LIBRARIES(mcrecover ${PCRE_PCRE_LIBRARY})
ENDIF(HAVE_PCRE16)

# Qt libraries
# NOTE: Libraries have to be linked in reverse order.
IF(USE_QT4)
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
ELSEIF(USE_QT5)
	# TODO: qjpcodecs for static linkage.
	IF(Qt5DBus_FOUND)
		TARGET_LINK_LIBRARIES(mcrecover Qt5::DBus)
	ENDIF(Qt5DBus_FOUND)
	TARGET_LINK_LIBRARIES(mcrecover Qt5::Widgets Qt5::Gui Qt5::Core)
ENDIF()

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
