# Memory Card library.
# NOTE: Uses Qt.
PROJECT(libmemcard)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}")

SET(CMAKE_AUTOMOC ON)
IF(QT_VERSION EQUAL 6)
	# Find Qt6
	SET(Qt6_NO_LINK_QTMAIN 1)
	FIND_PACKAGE(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
ELSEIF(QT_VERSION EQUAL 5)
	# Find Qt5
	# FIXME: Figure out a minimum Qt5 version.
	# QtWin::fromHICON was added in Qt 5.2.
	SET(Qt5_NO_LINK_QTMAIN 1)
	FIND_PACKAGE(Qt5 5.2.0 REQUIRED COMPONENTS Core Gui Widgets)
ELSE()
	MESSAGE(FATAL_ERROR "Unsupported Qt version: ${QT_VERSION}")
ENDIF()
SET(QT_NS Qt${QT_VERSION})

# Sources
SET(libmemcard_SRCS
	# Miscellaneous
	GcToolsQt.cpp
	IconAnimHelper.cpp
	TimeFuncs.cpp

	# Memory Card model
	MemCardModel.cpp
	MemCardItemDelegate.cpp
	MemCardSortFilterProxyModel.cpp

	# Memory Card objects
	Card.cpp
	File.cpp
	GcnCard.cpp
	GciCard.cpp
	GcnFile.cpp
	VmuCard.cpp
	VmuFile.cpp
	)

# Headers
SET(libmemcard_H
	# Miscellaneous
	GcToolsQt.hpp
	GcnSearchData.hpp
	TimeFuncs.hpp
	)

######################
# Build the library. #
######################

ADD_LIBRARY(memcard STATIC
	${libmemcard_SRCS} ${libmemcard_H}
	)
ADD_DEPENDENCIES(memcard git_version)
SET_MSVC_DEBUG_PATH(memcard)

TARGET_INCLUDE_DIRECTORIES(memcard
	PUBLIC	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	PRIVATE	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>
	)

# libgctools
TARGET_LINK_LIBRARIES(memcard gctools)

# Qt libraries
# NOTE: Libraries have to be linked in reverse order.
TARGET_LINK_LIBRARIES(memcard ${QT_NS}::Widgets ${QT_NS}::Gui ${QT_NS}::Core)

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
