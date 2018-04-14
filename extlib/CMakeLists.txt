PROJECT(extlib)

# Set common properties for extlib targets.
INCLUDE(SetMSVCDebugPath)
FUNCTION(SET_EXTLIB_PROPERTIES)
	FOREACH(_target ${ARGV})
		IF(TARGET ${_target})
			DO_SPLIT_DEBUG(${_target})
			SET_TARGET_PROPERTIES(${_target} PROPERTIES EXCLUDE_FROM_ALL TRUE)
		ENDIF(TARGET ${_target})
	ENDFOREACH(_target ${ARGV})
ENDFUNCTION(SET_EXTLIB_PROPERTIES)

# zlib: Assembly optimizations.
# TODO: Test on all archs and fix Z_PREFIX issues.
# TODO: Add more assembly files from contrib/ besides the
# default ones that were listed in zlib's CMakeLists.txt.
STRING(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" arch)
IF(arch MATCHES "^(i.|x)86$|^x86_64$|^amd64$")
	# i386/amd64. Check sizeof(void*) for the actual architecture,
	# since building 32-bit on 64-bit isn't considered "cross-compiling",
	# so CMAKE_SYSTEM_PROCESSOR might not be accurate.
	# NOTE: Checking CMAKE_CL_64 instead of sizeof(void*) for MSVC builds.
	IF(MSVC AND CMAKE_CL_64)
		SET(AMD64 "ON")
	ELSEIF(NOT MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
		SET(AMD64 "ON")
	ELSE()
		SET(ASM686 "ON")
	ENDIF()
ENDIF(arch MATCHES "^(i.|x)86$|^x86_64$|^amd64$")
UNSET(arch)

# Don't install any of these libraries.
# TODO: Build as DLLs on Windows?
SET(SKIP_INSTALL_LIBRARIES ON)
SET(SKIP_INSTALL_HEADERS ON)
SET(SKIP_INSTALL_FILES ON)
SET(SKIP_INSTALL_ALL ON)
SET(BUILD_SHARED_LIBS OFF)

# TODO: If we're using static Qt, check if it has internal zlib/libpng,
# and see if we can use that.

IF(USE_INTERNAL_ZLIB)
	# Use the internal copy of zlib. (statically-linked)
	ADD_SUBDIRECTORY(zlib)
	# NOTE: Cannot remove targets from all builds
	# if they're being installed.
	#SET_EXTLIB_PROPERTIES(zlib)
	DO_SPLIT_DEBUG(zlib)
	SET_EXTLIB_PROPERTIES(zlibstatic)
ENDIF(USE_INTERNAL_ZLIB)

IF(USE_INTERNAL_PNG)
	# Use the internal copy of libpng. (statically-linked)
	SET(PNG_SHARED OFF)
	SET(PNG_STATIC ON)
	SET(PNG_TESTS OFF)
	SET(PNG_DEBUG OFF)
	ADD_SUBDIRECTORY(libpng)
	# NOTE: Cannot remove targets from all builds
	# if they're being installed.
	#SET_EXTLIB_PROPERTIES(png)
	DO_SPLIT_DEBUG(png)
	SET_EXTLIB_PROPERTIES(png_static)
ENDIF(USE_INTERNAL_PNG)

IF(USE_INTERNAL_GIF)
	# Use the internal copy of giflib. (statically-linked)
	ADD_SUBDIRECTORY(giflib)
	SET_EXTLIB_PROPERTIES(gif)
ENDIF(USE_INTERNAL_GIF)

SET(BUILD_SHARED_LIBS ${OLD_BUILD_SHARED_LIBS})
SET(SKIP_INSTALL_LIBRARIES ${OLD_SKIP_INSTALL_LIBRARIES})
SET(SKIP_INSTALL_HEADERS ${OLD_SKIP_INSTALL_HEADERS})
SET(SKIP_INSTALL_FILES ${OLD_SKIP_INSTALL_FILES})
SET(SKIP_INSTALL_ALL ${OLD_SKIP_INSTALL_ALL})
UNSET(OLD_BUILD_SHARED_LIBS)
UNSET(OLD_SKIP_INSTALL_LIBRARIES)
UNSET(OLD_SKIP_INSTALL_HEADERS)
UNSET(OLD_SKIP_INSTALL_FILES)
UNSET(OLD_SKIP_INSTALL_ALL)
