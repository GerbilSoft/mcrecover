# Win32-specific CFLAGS/CXXFLAGS.
# For MinGW compilers.

# Set minimum Windows version to Windows 2000. (Windows NT 5.0)
SET(MCRECOVER_CFLAG_WIN32_WINNT "-D_WIN32_WINNT=0x0500")

# Release build: Prefer static libraries.
IF(CMAKE_BUILD_TYPE MATCHES ^release)
	SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ENDIF(CMAKE_BUILD_TYPE MATCHES ^release)

# Initialize CFLAGS/CXXFLAGS.
SET(MCRECOVER_CFLAGS_PLATFORM "-fshort-wchar ${MCRECOVER_CFLAG_WIN32_WINNT}")
SET(MCRECOVER_CXXFLAGS_PLATFORM "-fshort-wchar ${MCRECOVER_CFLAG_WIN32_WINNT}")

# Test for static libgcc/libstdc++.
SET(MCRECOVER_LDFLAGS_PLATFORM "")
FOREACH(FLAG_TEST "-static-libgcc" "-static-libstdc++" "-Wl,--large-address-aware" "-Wl,--nxcompat" "-Wl,--dynamicbase")
	# CMake doesn't like "+" characters in variable names.
	STRING(REPLACE "+" "_" FLAG_TEST_VARNAME "${FLAG_TEST}")

	CHECK_C_COMPILER_FLAG("${FLAG_TEST}" LDFLAG_${FLAG_TEST_VARNAME})
	IF(LDFLAG_${FLAG_TEST_VARNAME})
		SET(MCRECOVER_LDFLAGS_PLATFORM "${MCRECOVER_LDFLAGS_PLATFORM} ${FLAG_TEST}")
	ENDIF(LDFLAG_${FLAG_TEST_VARNAME})
	UNSET(LDFLAG_${FLAG_TEST_VARNAME})
	UNSET(FLAG_TEST_VARNAME)
ENDFOREACH()

# Enable windres support on MinGW.
# http://www.cmake.org/Bug/view.php?id=4068
IF(MINGW)
	SET(CMAKE_RC_COMPILER_INIT windres)
	ENABLE_LANGUAGE(RC)
	
	# NOTE: Setting CMAKE_RC_OUTPUT_EXTENSION doesn't seem to work.
	# Force windres to output COFF, even though it'll use the .res extension.
	SET(CMAKE_RC_OUTPUT_EXTENSION .obj)
	SET(CMAKE_RC_COMPILE_OBJECT
		"<CMAKE_RC_COMPILER> --output-format=coff <FLAGS> <DEFINES> -o <OBJECT> <SOURCE>")
ENDIF(MINGW)

# Unset variables we don't need elsewhere.
UNSET(MCRECOVER_CFLAG_WIN32_WINNT)
