# Save Editor library.
# NOTE: Uses Qt.
PROJECT(libsaveedit)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}")

# Find Qt5.
# FIXME: Figure out a minimum Qt5 version.
# QtWin::fromHICON was added in Qt 5.2.
SET(Qt5_NO_LINK_QTMAIN 1)
FIND_PACKAGE(Qt5 ${QT_REQUIRED} COMPONENTS Core Gui Widgets)

# Sources.
SET(libsaveedit_SRCS
	# Base editor window and widget.
	EditorWindow.cpp
	EditorWidget.cpp
	EditorWidgetFactory.cpp

	# Item models.
	models/BitFlags.cpp
	models/BitFlagsModel.cpp
	models/ByteFlags.cpp
	models/ByteFlagsModel.cpp
	models/PageFilterModel.cpp

	# Item view widgets.
	widgets/BitFlagsView.cpp
	widgets/ByteFlagsView.cpp
	widgets/CenteredCheckBoxDelegate.cpp

	# Sonic Adventure (DX) editor.
	SonicAdventure/SAEditor.cpp
	SonicAdventure/SALevelStats.cpp
	SonicAdventure/SALevelClearCount.cpp
	SonicAdventure/SAEventFlags.cpp
	SonicAdventure/SANPCFlags.cpp
	SonicAdventure/SADXMissionFlags.cpp
	SonicAdventure/SAGeneral.cpp
	SonicAdventure/SADataEvents.c
	SonicAdventure/SADXDataMissions.c
	SonicAdventure/SADataLevels.c
	SonicAdventure/SADataUI.c
	SonicAdventure/SAAdventure.cpp
	SonicAdventure/SAMiscEmblems.cpp
	SonicAdventure/TimeCodeEdit.cpp
	SonicAdventure/SASubGames.cpp
	)

# Headers.
SET(libsaveedit_H
	EditorWidgetFactory.hpp
	models/bit_flag.h
	SonicAdventure/SAData.h
	)

# Headers with Qt objects.
SET(libsaveedit_MOC_H
	# Base editor window and widget.
	EditorWindow.hpp
	EditorWidget.hpp

	# Item models.
	models/BitFlags.hpp
	models/BitFlagsModel.hpp
	models/ByteFlags.hpp
	models/ByteFlagsModel.hpp
	models/PageFilterModel.hpp

	# Item view widgets.
	widgets/BitFlagsView.hpp
	widgets/ByteFlagsView.hpp
	widgets/CenteredCheckBoxDelegate.hpp

	# Sonic Adventure (DX) editor.
	SonicAdventure/SAEditor.hpp
	SonicAdventure/SALevelStats.hpp
	SonicAdventure/SALevelClearCount.hpp
	SonicAdventure/SAEventFlags.hpp
	SonicAdventure/SANPCFlags.hpp
	SonicAdventure/SADXMissionFlags.hpp
	SonicAdventure/SAGeneral.hpp
	SonicAdventure/SAAdventure.hpp
	SonicAdventure/SAMiscEmblems.hpp
	SonicAdventure/TimeCodeEdit.hpp
	SonicAdventure/SASubGames.hpp
	SonicAdventure/SAEditWidget.hpp
	SonicAdventure/SADXEditWidget.hpp
	)
QT5_WRAP_CPP(libsaveedit_MOC_SRCS ${libsaveedit_MOC_H})

# UI files.
SET(libsaveedit_UIS
	# Base editor window.
	EditorWindow.ui

	# Item view widgets.
	widgets/BitFlagsView.ui
	widgets/ByteFlagsView.ui

	# Sonic Adventure (DX) editor.
	SonicAdventure/SAEditor.ui
	SonicAdventure/SALevelStats.ui
	SonicAdventure/SALevelClearCount.ui
	SonicAdventure/SAGeneral.ui
	SonicAdventure/SAAdventure.ui
	SonicAdventure/SAMiscEmblems.ui
	SonicAdventure/TimeCodeEdit.ui
	SonicAdventure/SASubGames.ui
	)
QT5_WRAP_UI(libsaveedit_UIS_H ${libsaveedit_UIS})

######################
# Build the library. #
######################

ADD_LIBRARY(saveedit STATIC
	${libsaveedit_SRCS} ${libsaveedit_H}
	${libsaveedit_MOC_SRCS}
	${libsaveedit_UIS_H}
	)
ADD_DEPENDENCIES(saveedit git_version)
SET_MSVC_DEBUG_PATH(saveedit)

TARGET_INCLUDE_DIRECTORIES(saveedit
	PUBLIC	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	PRIVATE	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>
	)

# Other GCN MemCard Reocver libraries.
TARGET_LINK_LIBRARIES(saveedit gctools memcard)

# Qt libraries
# NOTE: Libraries have to be linked in reverse order.
TARGET_LINK_LIBRARIES(saveedit Qt5::Widgets Qt5::Gui Qt5::Core)

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
