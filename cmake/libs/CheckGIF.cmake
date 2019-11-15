# Check for giflib.
# Normally, the system giflib is loaded at runtime instead
# of being linked at compile time, except on Windows, in
# which case it's statically linked.
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
	SET(GIFUTIL_LIBRARY gifutil)
	SET(GIF_FOUND 1)
	SET(HAVE_GIF 1)
ENDIF(USE_GIF AND USE_INTERNAL_GIF)
