PROJECT(mcrecover)
cmake_minimum_required(VERSION 2.6)

# McRecover source and binary directories.
# Binary directory is needed for byteorder.h.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# Find Qt4.
# Components used:
# - QtCore
# - QtGui
SET(MCRECOVER_QT4_MIN_VERSION 4.1.0)
FIND_PACKAGE(Qt4 ${MCRECOVER_QT4_MIN_VERSION} COMPONENTS QtCore QtGui QtXml)
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
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/byteorder.h.in" "${CMAKE_CURRENT_BINARY_DIR}/byteorder.h")

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
	SearchDialog.cpp
	SearchThreadWorker.cpp
	PcreRegex.cpp
	GcnDateTime.cpp
	)

# Headers with Qt4 objects.
SET(mcrecover_MOC_HEADERS
	MemCard.hpp
	MemCardFile.hpp
	McRecoverWindow.hpp
	MemCardModel.hpp
	GcnMcFileDb.hpp
	SearchThread.hpp
	SearchDialog.hpp
	SearchThreadWorker.hpp
	)

# UI files.
SET(mcrecover_UIS
	ui/McRecoverWindow.ui
	ui/SearchDialog.ui
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
TARGET_LINK_LIBRARIES(mcrecover ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${PCRE_LIBRARIES})
# TODO: Translations.
#ADD_DEPENDENCIES(gens-qt4 translations_target)

# Fast QString concatenation.
# Introduced with Qt 4.6.
SET_TARGET_PROPERTIES(mcrecover
	PROPERTIES COMPILE_DEFINITIONS "QT_USE_FAST_CONCATENATION;QT_USE_FAST_OPERATOR_PLUS")

# Fast QString concatenation. (Qt 4.8 version)
# Introduced with Qt 4.6.
SET_TARGET_PROPERTIES(mcrecover
	PROPERTIES COMPILE_DEFINITIONS "QT_USE_QSTRINGBUILDER")

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
