# Check for giflib.
# If giflib isn't found, extlib/giflib/ will be used instead.
# TODO: Option to enable/disable GIF support.
IF(USE_GIF AND USE_INTERNAL_GIF)
	IF(WIN32)
		MESSAGE(STATUS "Win32: using internal giflib")
	ELSEIF(USE_INTERNAL_GIF)
		MESSAGE(STATUS "Using internal giflib")
	ENDIF()

	# Use the internal copy of giflib.
	SET(USE_INTERNAL_GIF 1)
	SET(GIF_VERSION 5)
	SET(GIF_LIBRARY gif)
	SET(GIF_DEFINITIONS "")
	SET(GIF_FOUND 1)
	SET(HAVE_GIF 1)
	SET(GIF_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/extlib/giflib/lib/")
ENDIF(USE_GIF AND USE_INTERNAL_GIF)
