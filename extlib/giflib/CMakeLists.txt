PROJECT(giflib C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)

SET(VERSION "5.2.1")

# giflib source files.
SET(giflib_SRCS
	lib/dgif_lib.c
	lib/egif_lib.c
	lib/gifalloc.c
	lib/gif_err.c
	lib/gif_font.c
	lib/gif_hash.c
	lib/openbsd-reallocarray.c
	)
# Internal header files.
SET(giflib_H_INT
	lib/gif_hash.h
	lib/gif_lib_private.h
	)
# Public header files.
SET(giflib_H_PUBLIC
	lib/gif_lib.h
	)

# gifutil source files.
SET(gifutil_SRCS
	lib/qprintf.c
	lib/quantize.c
	lib/getarg.c
	)
# Public header files.
SET(gifutil_H_PUBLIC
	lib/getarg.h
	)

######################
# Build the library. #
######################

ADD_LIBRARY(gif STATIC
	${giflib_SRCS}
	${giflib_H_INT}
	${giflib_H_PUBLIC}
	)
TARGET_INCLUDE_DIRECTORIES(gif INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib")

ADD_LIBRARY(gifutil STATIC
	${gifutil_SRCS}
	${gifutil_H_PUBLIC}
	)
TARGET_INCLUDE_DIRECTORIES(gifutil INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib")
