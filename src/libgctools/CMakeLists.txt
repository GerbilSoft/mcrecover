PROJECT(libgctools)

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}")

# libpng
INCLUDE(CheckPNG)
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
ADD_DEFINITIONS(${ZLIB_DEFINITIONS} ${PNG_DEFINITIONS})

# giflib
INCLUDE(CheckGIF)

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

	util/array_size.h
	util/bitstuff.h
	util/byteorder.h
	util/byteswap.h
	util/git.h
	)

# PNG-specific sources.
IF(HAVE_PNG)
	SET(libgctools_PNG_SRCS GcImageWriter_PNG.cpp)
	IF(NOT USE_INTERNAL_PNG)
		SET(libgctools_PNG_SRCS
			"${libgctools_PNG_SRCS}"
			APNG_dlopen.c
			)
		SET(libgctools_PNG_H
			APNG_dlopen.h
			)

		# libdl is needed for dlopen().
		SET(gctools_NEEDS_DL 1)
	ENDIF(NOT USE_INTERNAL_PNG)
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

		# libdl is needed for dlopen().
		SET(gctools_NEEDS_DL 1)
	ENDIF(NOT USE_INTERNAL_GIF)
ENDIF(USE_GIF)

######################
# Build the library. #
######################

ADD_LIBRARY(gctools STATIC
	${libgctools_SRCS} ${libgctools_H}
	${libgctools_PNG_SRCS} ${libgctools_PNG_H}
	${libgctools_GIF_SRCS} ${libgctools_GIF_H}
	)
ADD_DEPENDENCIES(gctools git_version)
INCLUDE(SetMSVCDebugPath)
SET_MSVC_DEBUG_PATH(gctools)
TARGET_INCLUDE_DIRECTORIES(gctools
	PUBLIC	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	)
SET_TARGET_PROPERTIES(gctools PROPERTIES EXCLUDE_FROM_ALL TRUE)

# External libraries.
IF(HAVE_PNG)
	TARGET_LINK_LIBRARIES(gctools ${PNG_LIBRARY})
ENDIF(HAVE_PNG)
IF(USE_GIF)
	IF(USE_INTERNAL_GIF)
		TARGET_LINK_LIBRARIES(gctools ${GIF_LIBRARY} ${GIFUTIL_LIBRARY})
	ELSE(USE_INTERNAL_GIF)
		# quantize.c needs gif_lib.h, which might not
		# be present on the build system.
		TARGET_INCLUDE_DIRECTORIES(gctools PRIVATE "${CMAKE_SOURCE_DIR}/extlib/giflib/lib")
	ENDIF()
ENDIF(USE_GIF)

# Link in libdl if it's required for dlopen()
# and we have a component that uses it.
IF(gctools_NEEDS_DL AND CMAKE_DL_LIBS)
	TARGET_LINK_LIBRARIES(gctools ${CMAKE_DL_LIBS})
ENDIF(gctools_NEEDS_DL AND CMAKE_DL_LIBS)
