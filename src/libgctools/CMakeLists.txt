PROJECT(libgctools)
cmake_minimum_required(VERSION 2.6.0)

# McRecover source and binary directories.
# Binary directory is needed for byteorder.h.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# Test system byteorder.
# TODO: Universal Binary support; detect PDP endian.
INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
IF(IS_BIG_ENDIAN)
        SET(MCRECOVER_BYTEORDER "MCRECOVER_BIG_ENDIAN")
ELSE(IS_BIG_ENDIAN)
        SET(MCRECOVER_BYTEORDER "MCRECOVER_LIL_ENDIAN")
ENDIF(IS_BIG_ENDIAN)
UNSET(IS_BIG_ENDIAN)
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/util/byteorder.h.in" "${CMAKE_CURRENT_BINARY_DIR}/util/byteorder.h")

# libpng
# TODO: Internal libpng with APNG patch.
# TODO: If shared library, define shared library version in config file
# to make it possible to dlopen() APNG symbols later.
FIND_PACKAGE(PNG)
IF(PNG_FOUND)
	SET(HAVE_PNG 1)
ENDIF(PNG_FOUND)

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.libgctools.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.libgctools.h")

# Sources.
SET(libgctools_SRCS
	GcImage.cpp
	Checksum.cpp
	GcImageWriter.cpp
	)

# Build the library.
ADD_LIBRARY(gctools STATIC ${libgctools_SRCS})
TARGET_LINK_LIBRARIES(gctools ${PNG_LIBRARY})
SET_TARGET_PROPERTIES(gctools PROPERTIES
	COMPILE_DEFINITIONS "${PNG_DEFINITIONS}")
