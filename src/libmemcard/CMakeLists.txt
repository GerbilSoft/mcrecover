# Memory Card library.
# NOTE: Uses Qt.
PROJECT(libmemcard)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}")

# Find Qt5.
# FIXME: Figure out a minimum Qt5 version.
# QtWin::fromHICON was added in Qt 5.2.
SET(Qt5_NO_LINK_QTMAIN 1)
FIND_PACKAGE(Qt5 ${QT_REQUIRED} COMPONENTS Core Gui Widgets Xml)
IF(ENABLE_DBUS)
	# QtDBus is used for DockManager.
	FIND_PACKAGE(Qt5DBus)
	# FIXME: HAVE_Qt5DBus isn't working...
	SET(QtDBus_FOUND ${Qt5DBus_FOUND})
ENDIF(ENABLE_DBUS)

# Sources.
SET(libmemcard_SRCS
	# Miscellaneous
	GcnDateTime.cpp
	GcToolsQt.cpp
	IconAnimHelper.cpp

	# Memory Card model
	MemCardModel.cpp
	MemCardItemDelegate.cpp
	MemCardSortFilterProxyModel.cpp

	# Memory Card objects
	Card.cpp
	File.cpp
	GcnCard.cpp
	GcnFile.cpp
	VmuCard.cpp
	VmuFile.cpp
	)
# Headers.
SET(libmemcard_H
	# Miscellaneous
	GcToolsQt.hpp
	GcnDateTime.hpp
	GcnSearchData.hpp
	)
# Headers with Qt objects.
SET(libmemcard_MOC_H
	# Miscellaneous
	IconAnimHelper.hpp

	# Memory Card model
	MemCardModel.hpp
	MemCardItemDelegate.hpp
	MemCardSortFilterProxyModel.hpp

	# Memory Card objects
	Card.hpp
	File.hpp
	GcnCard.hpp
	GcnFile.hpp
	VmuCard.hpp
	VmuFile.hpp
	)
QT5_WRAP_CPP(libmemcard_MOC_SRCS ${libmemcard_MOC_H})

######################
# Build the library. #
######################

ADD_LIBRARY(memcard STATIC
	${libmemcard_SRCS} ${libmemcard_H}
	${libmemcard_MOC_SRCS}
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
TARGET_LINK_LIBRARIES(memcard Qt5::Widgets Qt5::Gui Qt5::Core)

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
