PROJECT(src)
cmake_minimum_required(VERSION 2.6.0)

# Include the current source and binary directories.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

# C++11 compatibility header.
# NOTE: This must be included regardless of C++11 support in the compiler.
# gcc-4.6 supports some C++11, but is missing explicit virtual overrides.
IF(MSVC)
	SET(MCRECOVER_BASE_CXX11_COMPAT_HEADER "-FI${CMAKE_CURRENT_SOURCE_DIR}/c++11-compat.h")
ELSE(MSVC)
	SET(MCRECOVER_BASE_CXX11_COMPAT_HEADER "-include ${CMAKE_CURRENT_SOURCE_DIR}/c++11-compat.h")
ENDIF(MSVC)

# Add the C++11 compatibility header to CFLAGS/CXXFLAGS.
SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${MCRECOVER_BASE_CXX11_COMPAT_HEADER}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MCRECOVER_BASE_CXX11_COMPAT_HEADER}")

# Source Code subdirectories.
ADD_SUBDIRECTORY(libgctools)
ADD_SUBDIRECTORY(mcrecover)
ADD_SUBDIRECTORY(gcbanner)

IF(WIN32 AND NOT MSVC)
	FIND_PROGRAM(UNIX2DOS unix2dos)
	IF(NOT UNIX2DOS)
		ADD_SUBDIRECTORY(miniu2d)
	ENDIF(NOT UNIX2DOS)
ENDIF(WIN32 AND NOT MSVC)
