# Check for libpng.
# If libpng isn't found, extlib/libpng/ will be used instead.
IF(NOT HAVE_PNG)

IF(WIN32)
	MESSAGE(STATUS "Win32: using internal libpng")
ELSEIF(USE_INTERNAL_PNG)
	MESSAGE(STATUS "Using internal libpng")
ELSE()
	# TODO: Make libpng support optional?
	FIND_PACKAGE(ZLIB REQUIRED)
	FIND_PACKAGE(PNG REQUIRED)
ENDIF()
SET(HAVE_ZLIB ${ZLIB_FOUND})
SET(HAVE_PNG ${PNG_FOUND})

IF(NOT PNG_FOUND)
	# libpng wasn't found.
	# NOTE: PNG_LIBRARY will need to be updated if upgrading past libpng-1.6.
	SET(USE_INTERNAL_PNG 1)
	SET(PNG_LIBRARY png16_static)
	SET(PNG_FOUND 1)
	SET(HAVE_PNG 1)
	INCLUDE(CheckZLIBInternal)
	SET(PNG_INCLUDE_DIR
		"${CMAKE_CURRENT_SOURCE_DIR}/extlib/libpng/"
		"${CMAKE_CURRENT_BINARY_DIR}/extlib/libpng/"
		)
ENDIF(NOT PNG_FOUND)

ENDIF(NOT HAVE_PNG)
