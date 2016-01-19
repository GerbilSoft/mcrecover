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
ADD_DEFINITIONS(${ZLIB_DEFINITIONS} ${PNG_DEFINITIONS})

# giflib
INCLUDE(CheckGIF)
INCLUDE_DIRECTORIES(${GIF_INCLUDE_DIR})
ADD_DEFINITIONS(${GIF_DEFINITIONS})

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

# GIF-specific sources.
IF(USE_GIF)
	SET(libgctools_GIF_SRCS
		GcImageWriter_GIF.cpp
		GIF_dlopen.c
		)
	SET(libgctools_GIF_H
		GIF_dlopen.h
		)

	IF(NOT USE_INTERNAL_GIF)
		# quantize.c is from giflib-5.1.
		# It's included here because giflib-4.2 removed it,
		# and it was readded in giflib-5.0. Hence, we can't
		# rely on it being available in giflib.
		# (Unless we're using the internal giflib, that is...)
		SET(libgctools_GIF_SRCS ${libgctools_GIF_SRCS} quantize.c)
	ENDIF(NOT USE_INTERNAL_GIF)
ENDIF(USE_GIF)

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
	${libgctools_GIF_SRCS} ${libgctools_GIF_H}
	)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(gctools)
TARGET_LINK_LIBRARIES(gctools ${gctools_DL_LIBS})
IF(HAVE_PNG)
	TARGET_LINK_LIBRARIES(gctools ${PNG_LIBRARY})
ENDIF(HAVE_PNG)
IF(USE_GIF AND USE_INTERNAL_GIF)
	TARGET_LINK_LIBRARIES(gctools ${GIF_LIBRARY})
ENDIF(USE_GIF AND USE_INTERNAL_GIF)
