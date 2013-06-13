PROJECT(mcrecover_base)

# Version dependencies:
# - BREAK(): CMake 2.6.0
# - NASM: CMake 2.8.4
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)

# UNSET() was added in cmake-2.6.3.
# Define an equivalent macro for older versions.
MACRO(UNSET var)
	SET(${var})
ENDMACRO(UNSET)

list (APPEND CMAKE_MODULE_PATH
	"${mcrecover_base_SOURCE_DIR}/cmake/modules"
	"${mcrecover_base_SOURCE_DIR}/cmake/macros"
	)

# Program information.
SET(DESCRIPTION "GCN MemCard Recover")
SET(AUTHOR "David Korth")
SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "0")
SET(VERSION_PATCH "0")
SET(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
SET(VERSION_STRING_WIN32 "${VERSION_MAJOR},${VERSION_MINOR},${VERSION_PATCH},0")

# Git version information.
ADD_CUSTOM_TARGET(git_version ALL
	${mcrecover_base_SOURCE_DIR}/git_version.sh
	-s "${mcrecover_base_SOURCE_DIR}"
	-o "${mcrecover_base_BINARY_DIR}/git_version.h"
	VERBATIM
	)

# If no build type is set, default to "debug".
IF(CMAKE_BUILD_TYPE MATCHES ^$)
	SET(CMAKE_BUILD_TYPE "debug")
ENDIF()

# Don't bother with verbose makefiles for now.
# TODO: Make it an option?
#IF (CMAKE_BUILD_TYPE MATCHES ^debug$)
#	SET(CMAKE_VERBOSE_MAKEFILE ON)
#ENDIF()

# Check for platform-specific functionality.
INCLUDE(cmake/platform.cmake)

# Project subdirectories.
ADD_SUBDIRECTORY(src)
