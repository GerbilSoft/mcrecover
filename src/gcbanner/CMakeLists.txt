PROJECT(gcbanner)
cmake_minimum_required(VERSION 2.6)

# gcbanner source and binary directories.
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

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES("${mcrecover_base_BINARY_DIR}")

# Check for getopt_long().
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckSymbolExists)
CHECK_INCLUDE_FILES(getopt.h HAVE_GETOPT_H)
CHECK_SYMBOL_EXISTS(getopt_long "getopt.h" HAVE_GETOPT_LONG)
IF(HAVE_GETOPT_H AND HAVE_GETOPT_LONG)
	# System has getopt_long().
	UNSET(getopt_SRCS)
ELSE()
	# System is missing getopt_long().
	# Use vlc's getopt_long().
	SET(getopt_SRCS
		vlc_getopt/getopt.c
		)
ENDIF()

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.gcbanner.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.gcbanner.h")

# Sources.
SET(gcbanner_SRCS
	gcbanner.cpp
	rijndael.c
	wibn.cpp
	)

# Win32-specific sources.
IF(WIN32)
	SET(gcbanner_OS_SRCS
		win32/gcbanner.rc
		)
ENDIF(WIN32)

#########################
# Build the executable. #
#########################

SET(GCBANNER_PDB_FILE "${CMAKE_CURRENT_BINARY_DIR}/gcbanner.pdb")

IF(MSVC)
	# CMake seems to use weird settings for the PDB file.
	# (at least version 2.8.12.2; 3.0.0 might be different)
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_C}"
		"/Fd${GCBANNER_PDB_FILE}"
		CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")
	STRING(REGEX REPLACE
		"/Fd<OBJECT_DIR>/${_FS_CXX}"
		"/Fd${GCBANNER_PDB_FILE}"
		CMAKE_CXX_COMPILE_OBJECT "${CMAKE_CXX_COMPILE_OBJECT}")
ENDIF(MSVC)

ADD_EXECUTABLE(gcbanner
	${gcbanner_SRCS}
	${gcbanner_OS_SRCS}
	${getopt_SRCS}
	)

# libgctools
TARGET_LINK_LIBRARIES(gcbanner gctools)

# PDB and program version.
SET_TARGET_PROPERTIES(gcbanner
	PROPERTIES
		PDB "${GCBANNER_PDB_FILE}"
		VERSION "${VERSION_MAJOR}.${VERSION_MINOR}"
	)

# Split debug information.
IF(SPLIT_DEBUG)
	INCLUDE(SplitDebugInformation)
	SPLIT_DEBUG_INFORMATION(gcbanner)
ENDIF(SPLIT_DEBUG)

# Compress the EXE.
IF(COMPRESS_EXE)
	INCLUDE(CompressExeWithUpx)
	COMPRESS_EXE_WITH_UPX(gcbanner)
ENDIF(COMPRESS_EXE)

#################
# Installation. #
#################

INCLUDE(DirInstallPaths)

INSTALL(TARGETS gcbanner
	RUNTIME DESTINATION "${DIR_INSTALL_EXE}"
	LIBRARY DESTINATION "${DIR_INSTALL_DLL}"
	ARCHIVE DESTINATION "${DIR_INSTALL_LIB}"
	COMPONENT "program"
	)
IF(INSTALL_DEBUG)
IF(MSVC)
	INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/gcbanner.pdb"
		DESTINATION "${DIR_INSTALL_EXE_DEBUG}"
		COMPONENT "debug"
		)
ELSEIF(SPLIT_DEBUG)
	INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/gcbanner.debug"
		DESTINATION "${DIR_INSTALL_EXE_DEBUG}"
		COMPONENT "debug"
		)
ENDIF(MSVC)
ENDIF(INSTALL_DEBUG)
