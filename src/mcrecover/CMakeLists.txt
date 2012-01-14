PROJECT(mcrecover)
cmake_minimum_required(VERSION 2.6)

# Find Qt4.
# Components used:
# - QtCore
# - QtGui
SET(MCRECOVER_QT4_MIN_VERSION 4.1.0)
FIND_PACKAGE(Qt4 ${MCRECOVER_QT4_MIN_VERSION} COMPONENTS QtCore QtGui)
IF(NOT QT_QTCORE_FOUND)
	MESSAGE(FATAL_ERROR "QtCore not found! Please install Qt ${MCRECOVER_QT4_MIN_VERSION} or higher.")
ELSEIF(NOT QT_QTGUI_FOUND)
	MESSAGE(FATAL_ERROR "QtGui not found! Please install Qt ${MCRECOVER_QT4_MIN_VERSION} or higher.")
ENDIF()

INCLUDE(${QT_USE_FILE})

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

# Sources.
SET(mcrecover_SRCS
	mcrecover.cpp
	)

# Headers with Qt4 objects.
SET(mcrecover_MOC_HEADERS
	)

# UI files.
SET(mcrecover_UIS
	ui/McRecoveryWindow.ui
	)

# Generate the header files from the UI files.
QT4_WRAP_UI(mcrecover_UIS_H ${mcrecover_UIS})

# Create .moc files for sources that need them.
QT4_WRAP_CPP(mcrecover_MOC_SOURCES ${mcrecover_MOC_HEADERS})

#################
# Translations. #
#################

# TODO
#ADD_SUBDIRECTORY(translations)
# TODO: Mac OS X bundle location.
# TODO: "make install" location.

#########################
# Build the executable. #
#########################

# TODO: Set Win32 when compiling release build
# to disable the command prompt window.
ADD_EXECUTABLE(mcrecover MACOSX_BUNDLE
	${mcrecover_SRCS}
	${mcrecover_COMPAT_SRCS}
	${mcrecover_MOC_SOURCES}
	${mcrecover_UIS_H}
	)
# TODO: Translations.
#ADD_DEPENDENCIES(gens-qt4 translations_target)

# Fast QString concatenation.
# Introduced with Qt 4.6.
SET_TARGET_PROPERTIES(mcrecover
	PROPERTIES COMPILE_DEFINITIONS "QT_USE_FAST_CONCATENATION;QT_USE_FAST_OPERATOR_PLUS")

# Disable implicit QString ASCII casts
SET_TARGET_PROPERTIES(mcrecover
	PROPERTIES COMPILE_DEFINITIONS "QT_NO_CAST_FROM_ASCII;QT_NO_CAST_TO_ASCII")

# Mac OS X bundle information.
SET(MACOSX_BUNDLE_GUI_IDENTIFIER "gsft.gerbilsoft.McRecover")
SET(MACOSX_BUNDLE_BUNDLE_NAME "GameCube Memory Card Recovery")
#SET(MACOSX_BUNDLE_ICON_FILE "mcrecover.icns")
SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "0.0.0")
SET(MACOSX_BUNDLE_LONG_VERSION_STRING "0.0.0")

# Mac OS X: Set a custom info.plist file for the application bundle.
# TODO
#SET_TARGET_PROPERTIES(mcrecover
#	PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/ui/resources/mac/Info-CMake.plist")
