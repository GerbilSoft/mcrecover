# - Try to find the PCRE regular expression library
# Once done this will define
#
#  PCRE_FOUND - system has the PCRE library
#  PCRE_INCLUDE_DIR - the PCRE include directory
#  PCRE_LIBRARIES - The libraries needed to use PCRE

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (PCRE_INCLUDE_DIR AND PCRE_PCREPOSIX_LIBRARY AND PCRE_PCRE_LIBRARY)
  # Already in cache, be silent
  set(PCRE_FIND_QUIETLY TRUE)
endif (PCRE_INCLUDE_DIR AND PCRE_PCREPOSIX_LIBRARY AND PCRE_PCRE_LIBRARY)


if (NOT WIN32)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  find_package(PkgConfig)

  pkg_check_modules(PC_PCRE libpcre)

  set(PCRE_DEFINITIONS ${PC_PCRE_CFLAGS_OTHER})

endif (NOT WIN32)

find_path(PCRE_INCLUDE_DIR pcre.h 
          HINTS ${PC_PCRE_INCLUDEDIR} ${PC_PCRE_INCLUDE_DIRS} 
          PATH_SUFFIXES pcre)

find_library(PCRE_PCRE_LIBRARY NAMES pcre pcred HINTS ${PC_PCRE_LIBDIR} ${PC_PCRE_LIBRARY_DIRS})

find_library(PCRE_PCREPOSIX_LIBRARY NAMES pcreposix pcreposixd HINTS ${PC_PCRE_LIBDIR} ${PC_PCRE_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PCRE DEFAULT_MSG PCRE_INCLUDE_DIR PCRE_PCRE_LIBRARY PCRE_PCREPOSIX_LIBRARY )

set(PCRE_LIBRARIES ${PCRE_PCRE_LIBRARY} ${PCRE_PCREPOSIX_LIBRARY})

# Check if PCRE is static.
UNSET(PCRE_STATIC)
IF(WIN32)
	IF(MINGW)
		# MinGW DLL import libraries end in .dll.a.
		# Other .a libraries are static.
		IF(NOT PCRE_PCRE_LIBRARY MATCHES .dll.a$)
			SET(PCRE_STATIC 1)
		ENDIF()
	ELSE()
		# TODO: MSVC support.
		# For now, assume the library is static.
		MESSAGE(WARNING "MSVC in use; assuming PCRE is static.")
		SET(PCRE_STATIC 1)
	ENDIF()
ELSE(WIN32)
	# .a on other platforms is usually static.
	IF(PCRE_PCRE_LIBRARY MATCHES .a$)
		SET(PCRE_STATIC 1)
	ENDIF()
ENDIF(WIN32)

IF(PCRE_STATIC)
	SET(PCRE_DEFINITIONS ${PCRE_DEFINITIONS} -DPCRE_STATIC)
ENDIF(PCRE_STATIC)

mark_as_advanced(PCRE_INCLUDE_DIR PCRE_LIBRARIES PCRE_PCREPOSIX_LIBRARY PCRE_PCRE_LIBRARY PCRE_STATIC)
