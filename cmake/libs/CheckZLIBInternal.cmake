# Check for internal zlib.
# NOTE: This is only used if internal PNG is being used.
IF(USE_INTERNAL_PNG AND NOT USE_INTERNAL_ZLIB)

MESSAGE(STATUS "Using internal ZLIB due to internal PNG dependency.")

SET(ZLIB_LIBRARY zlibstatic)
# NOTE: Re-enable these if we use zlib directly.
#SET(ZLIB_FOUND 1)
#SET(HAVE_ZLIB 1)
SET(USE_INTERNAL_ZLIB 1)
SET(ZLIB_INCLUDE_DIR
	"${CMAKE_CURRENT_SOURCE_DIR}/extlib/zlib/"
	"${CMAKE_CURRENT_BINARY_DIR}/extlib/zlib/"
	)

ENDIF(USE_INTERNAL_PNG AND NOT USE_INTERNAL_ZLIB)
