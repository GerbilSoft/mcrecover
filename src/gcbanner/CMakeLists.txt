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

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.gcbanner.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.gcbanner.h")

# Sources.
SET(gcbanner_SRCS
	gcbanner.cpp
	rijndael.c
	wibn.cpp
	)

#########################
# Build the executable. #
#########################

ADD_EXECUTABLE(gcbanner ${gcbanner_SRCS})

# libgctools
TARGET_LINK_LIBRARIES(gcbanner gctools)

# Split debug information.
INCLUDE(SplitDebugInformation)
IF(SPLIT_DEBUG)
	#SPLIT_DEBUG_INFORMATION(gcbanner)
ENDIF(SPLIT_DEBUG)

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
ELSE(MSVC)
	INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/gcbanner.debug"
		DESTINATION "${DIR_INSTALL_EXE_DEBUG}"
		COMPONENT "debug"
		)
ENDIF(MSVC)
ENDIF(INSTALL_DEBUG)
