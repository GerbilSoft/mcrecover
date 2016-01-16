# Check for internal zlib.
# NOTE: This is only used if internal PNG is being used.
IF(USE_INTERNAL_PNG AND NOT HAVE_ZLIB)

MESSAGE(STATUS "Using internal ZLIB due to internal PNG dependency.")

SET(ZLIB_LIBRARY zlibstatic)
SET(ZLIB_DEFINITIONS -DZLIB_CONST)
SET(ZLIB_FOUND 1)
SET(HAVE_ZLIB 1)
SET(USE_INTERNAL_ZLIB 1)
SET(ZLIB_INCLUDE_DIR
	"${CMAKE_SOURCE_DIR}/extlib/zlib/"
	"${CMAKE_BINARY_DIR}/extlib/zlib/"
	)

ENDIF(USE_INTERNAL_PNG AND NOT HAVE_ZLIB)
