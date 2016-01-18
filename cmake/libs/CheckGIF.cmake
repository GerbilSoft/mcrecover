# Check for giflib.
# If giflib isn't found, extlib/giflib/ will be used instead.
# TODO: Option to enable/disable GIF support.
IF(NOT DEFINED HAVE_GIF)

IF(WIN32)
	MESSAGE(STATUS "Win32: using internal giflib")
ELSEIF(USE_INTERNAL_GIF)
	MESSAGE(STATUS "Using internal giflib")
ELSE()
	FIND_PACKAGE(GIF REQUIRED)
ENDIF()
SET(HAVE_GIF ${GIF_FOUND})

IF(NOT GIF_FOUND)
	# giflib wasn't found.
	# TODO: Internal giflib.
	#SET(USE_INTERNAL_GIF 1)
	#SET(GIF_LIBRARY giflib)
	#SET(GIF_DEFINITIONS -DPNG_STATIC)
	#SET(GIF_FOUND 1)
	#SET(HAVE_GIF 1)
	#SET(GIF_INCLUDE_DIR
	#	"${CMAKE_SOURCE_DIR}/extlib/giflib/"
	#	"${CMAKE_BINARY_DIR}/extlib/giflib/"
	#	)
ENDIF(NOT GIF_FOUND)

ENDIF(NOT DEFINED HAVE_GIF)
