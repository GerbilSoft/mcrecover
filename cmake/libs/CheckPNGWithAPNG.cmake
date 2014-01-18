# Check for libpng.
# If libpng isn't found, extlib/libpng/ will be used instead.
# TODO: Check for zlib too.
IF(NOT HAVE_PNG)

IF(WIN32)
	MESSAGE(STATUS "Win32: using internal PNG")
ELSE(WIN32)
	# Only search for libpng on non-Win32 platforms.
	# Use the built-in libpng on Win32.
	# TODO: Remove REQUIRED after internal PNG is added.
	FIND_PACKAGE(PNG REQUIRED)
ENDIF(WIN32)
SET(HAVE_PNG ${PNG_FOUND})

IF(HAVE_PNG)
	# Check for APNG support.
	# TODO: Only on shared version; internal version always has APNG.
	INCLUDE(CheckCSourceCompiles)
	SET(SAFE_CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES})
	SET(SAFE_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
	SET(CMAKE_REQUIRED_INCLUDES ${PNG_INCLUDE_DIR})
	SET(CMAKE_REQUIRED_LIBRARIES ${PNG_LIBRARY})
	CHECK_C_SOURCE_COMPILES("
	#include <png.h>
	#if !defined(PNG_APNG_SUPPORTED)
	#error png.h is missing PNG_APNG_SUPPORTED
	#elif !defined(PNG_READ_APNG_SUPPORTED)
	#error png.h is missing PNG_READ_APNG_SUPPORTED
	#elif !defined(PNG_WRITE_APNG_SUPPORTED)
	#error png.h is missing PNG_WRITE_APNG_SUPPORTED
	#endif

	int main(void) {
		int a = png_get_acTL(NULL, NULL, NULL, NULL);
		int b = png_set_acTL(NULL, NULL, 0, 0);
		return 0;
	}" HAVE_PNG_APNG)
	SET(CMAKE_REQUIRED_INCLUDES ${SAFE_CMAKE_REQUIRED_INCLUDES})
	SET(CMAKE_REQUIRED_LIBRARIES ${SAFE_CMAKE_REQUIRED_LIBRARIES})
	UNSET(SAFE_CMAKE_REQUIRED_INCLUDES)
	UNSET(SAFE_CMAKE_REQUIRED_LIBRARIES)
ENDIF(HAVE_PNG)

IF(NOT PNG_FOUND OR NOT HAVE_PNG_APNG)
	# libpng wasn't found, or libpng doesn't support APNG.
	# TODO: Internal libpng.
	#SET(PNG_LIBRARY libpngstatic)
	#SET(PNG_FOUND 1)
	#SET(HAVE_PNG 1)
	#SET(PNG_INCLUDE_DIR
	#	"${CMAKE_CURRENT_SOURCE_DIR}/extlib/libpng/"
	#	"${CMAKE_CURRENT_BINARY_DIR}/extlib/libpng/"
	#	)
	#SET(USE_INTERNAL_PNG 1)
	#IF(NOT WIN32)
	#	MESSAGE(STATUS "PNG library not found; using internal PNG.")
	#ENDIF(NOT WIN32)
ENDIF(NOT PNG_FOUND OR NOT HAVE_PNG_APNG)

ENDIF(NOT HAVE_PNG)
