PROJECT(mcrecover_base)
# CMake 2.8.11 adds TARGET_INCLUDE_DIRECTORIES() and Qt4::xxx targets, similar to Qt5.
# CMake 2.8.12 automates adding compile flags for Qt5, e.g. -fPIC on Linux.
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

# UNSET() was added in cmake-2.6.3.
# Define an equivalent macro for older versions.
MACRO(UNSET var)
	SET(${var})
ENDMACRO(UNSET)

list (APPEND CMAKE_MODULE_PATH
	"${mcrecover_base_SOURCE_DIR}/cmake/modules"
	"${mcrecover_base_SOURCE_DIR}/cmake/macros"
	"${mcrecover_base_SOURCE_DIR}/cmake/libs"
	)

# Set default build options.
INCLUDE(cmake/options.cmake)

# Program information.
SET(DESCRIPTION "GCN MemCard Recover")
SET(AUTHOR "David Korth")
SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 2)
SET(VERSION_PATCH 1)
SET(VERSION_DEVEL 1)
IF(VERSION_PATCH)
	SET(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
ELSE(VERSION_PATCH)
	SET(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}")
ENDIF(VERSION_PATCH)
IF(VERSION_DEVEL)
	SET(VERSION_STRING "${VERSION_STRING}+")
ENDIF(VERSION_DEVEL)
SET(VERSION_STRING_WIN32 "${VERSION_MAJOR},${VERSION_MINOR},${VERSION_PATCH},${VERSION_DEVEL}")

# Git version information.
FIND_PROGRAM(POSIX_SH sh)
IF(POSIX_SH)
	# sh is available.
	# Run the git version script.
	IF(WIN32)
		SET(ENV{SHELLOPTS} igncr)
	ENDIF(WIN32)
	ADD_CUSTOM_TARGET(git_version ALL
		${POSIX_SH} ${mcrecover_base_SOURCE_DIR}/git_version.sh
		-s "${mcrecover_base_SOURCE_DIR}"
		-o "${mcrecover_base_BINARY_DIR}/git_version.h"
		VERBATIM
		)
ELSE(POSIX_SH)
	# sh isn't available.
	# Create a blank git_version.h.
	FILE(WRITE "${mcrecover_base_BINARY_DIR}/git_version.h"
		"/* dummy file; POSIX sh is not available */\n")
ENDIF(POSIX_SH)

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

# Check for required libraries.
# NOTE: This has to be done here in addition to
# the project subdirectories; otherwise, the
# results may be lost.
INCLUDE(CheckPCRE)
INCLUDE(CheckPNG)
INCLUDE(CheckGIF)

# Project subdirectories.
ADD_SUBDIRECTORY(extlib)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(data)
ADD_SUBDIRECTORY(doc)

# CPack settings.
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GCN MemCard Recover")
SET(CPACK_PACKAGE_NAME "mcrecover")
SET(CPACK_PACKAGE_VENDOR ${AUTHOR})
SET(CPACK_PACKAGE_CONTACT "David Korth <gerbilsoft@gerbilsoft.com>")
SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
SET(CPACK_PACKAGE_VERSION ${VERSION_STRING})

# CPack: Debian package settings.
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.4), libgcc1 (>= 1:4.1.1), libstdc++6 (>= 4.1.1), libqtcore4 (>= 4:4.6.0), libqt4-xml (>= 4:4.6.0), libqtgui4 (>= 4:4.6.0), libpcre3")
SET(CPACK_DEBIAN_PACKAGE_BUILDS_DEPENDS "libqt4-dev (>= 4:4.6.0), libpcre3-dev")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET(CPACK_DEBIAN_PACKAGE_SECTION "games")
SET(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

# TODO: DESCRIPTION and WELCOME files.
#SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/doc/DESCRIPTION.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/doc/gpl-2.0.txt")
SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/doc/README.txt")
#SET(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/doc/WELCOME.txt")

# CPack: Source package settings.
# NOTE: Double-escape is required because the unescaped
# string # is written to CPackSourceConfig.cmake, which
# is then unescaped.
SET(CPACK_SOURCE_GENERATOR "TGZ")
SET(CPACK_SOURCE_IGNORE_FILES
	"build.*/"
	"build.*\\\\.sh"
	"\\\\.git/"
	"\\\\.gitignore"
	"FOR FUTURE USE/"
	".kdev4/"
	)

IF(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i.|x)86\$")
	SET(CPACK_PACKAGE_SYSTEM_PROCESSOR "i386")
ELSEIF(CMAKE_SYSTEM_PROCESSOR MATCHES "^x86_64\$")
	SET(CPACK_PACKAGE_SYSTEM_PROCESSOR "amd64")
ELSE()
	SET(CPACK_PACKAGE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR})
ENDIF()

IF(UNIX AND NOT APPLE)
	SET(CPACK_GENERATOR "DEB")
	SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_PACKAGE_SYSTEM_PROCESSOR}")
	SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}")
ELSEIF(APPLE)
	# (TODO for mcrecover-v0.3)
ELSEIF(WIN32)
	SET(CPACK_GENERATOR "ZIP")
	SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}-win32")
	SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
ENDIF()

# Components.
SET(CPACK_COMPONENTS_ALL program debug database translations doc desktop-icon)
SET(CPACK_COMPONENT_PROGRAM_DISPLAY_NAME "Programs")
SET(CPACK_COMPONENT_DEBUG_DISPLAY_NAME "Debugging Symbols")
SET(CPACK_COMPONENT_DATABASE_DISPLAY_NAME "Memory Card File Databases")
SET(CPACK_COMPONENT_TRANSLATIONS_DISPLAY_NAME "Translations")
SET(CPACK_COMPONENT_DOC_DISPLAY_NAME "Documentation")
SET(CPACK_COMPONENT_DESKTOP-ICON_DISPLAY_NAME "XDG Desktop File")

# Initialize CPack.
INCLUDE(CPack)
