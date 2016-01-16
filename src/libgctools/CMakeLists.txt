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
INCLUDE(CheckPNG)
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})

# Write the config.h file.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/config.libgctools.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.libgctools.h")

# Sources.
SET(libgctools_SRCS
	GcImage.cpp
	Checksum.cpp
	GcImageWriter.cpp
	GcImageLoader.cpp
	DcImageLoader.cpp
	)
SET(libgctools_H
	GcImage.hpp
	GcImage_p.hpp
	Checksum.hpp
	GcImageWriter.hpp
	GcImageWriter_p.hpp
	GcImageLoader.hpp
	DcImageLoader.hpp
	)

# PNG-specific sources.
IF(HAVE_PNG)
	SET(libgctools_PNG_SRCS
		GcImageWriter_PNG.cpp
		APNG_dlopen.c
		)
	SET(libgctools_PNG_H
		APNG_dlopen.h
		)
ENDIF(HAVE_PNG)

# If we're using an external libpng, link in libdl.
IF(NOT USE_INTERNAL_PNG)
	SET(gctools_DL_LIBS ${CMAKE_DL_LIBS})
ENDIF(NOT USE_INTERNAL_PNG)

######################
# Build the library. #
######################

ADD_LIBRARY(gctools STATIC
	${libgctools_SRCS} ${libgctools_H}
	${libgctools_PNG_SRCS} ${libgctools_PNG_H}
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(gctools)
TARGET_LINK_LIBRARIES(gctools ${PNG_LIBRARY} ${gctools_DL_LIBS})

# Definitions for third-party libraries.
ADD_DEFINITIONS(${ZLIB_DEFINITIONS} ${PNG_DEFINITIONS})
