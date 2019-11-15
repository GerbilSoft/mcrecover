# Win32-specific CFLAGS/CXXFLAGS.
# For MinGW compilers.

# Enable "secure" API functions: *_s()
SET(MCR_C_FLAGS_WIN32 "${MCR_C_FLAGS_WIN32} -DMINGW_HAS_SECURE_API")

# Subsystem and minimum Windows version:
# - If 32-bit: 5.00
# - If 64-bit: 5.02
# GCN MemCard Recover does NOT support ANSI Windows.
# TODO: Does CMAKE_CREATE_*_EXE also affect DLLs?
IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
	# 64-bit, Unicode Windows only.
	# (There is no 64-bit ANSI Windows.)
	SET(CMAKE_CREATE_WIN32_EXE "-Wl,--subsystem,windows:5.02")
	SET(CMAKE_CREATE_CONSOLE_EXE "-Wl,--subsystem,console:5.02")
ELSE()
	# 32-bit, Unicode Windows only.
	SET(CMAKE_CREATE_WIN32_EXE "-Wl,--subsystem,windows:5.00")
	SET(CMAKE_CREATE_CONSOLE_EXE "-Wl,--subsystem,console:5.00")
ENDIF()

SET(MCR_EXE_LINKER_FLAGS_WIN32 "")
SET(MCR_SHARED_LINKER_FLAGS_WIN32 "")
SET(MCR_MODULE_LINKER_FLAGS_WIN32 "")

# Release build: Prefer static libraries.
# TODO: Use DLLs instead?
IF(CMAKE_BUILD_TYPE MATCHES ^release)
	SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ENDIF(CMAKE_BUILD_TYPE MATCHES ^release)

# Test for various linker flags.
# NOTE: --tsaware is only valid for EXEs, not DLLs.
# TODO: Make static linkage a CMake option: --static-libgcc, --static-libstdc++
+FOREACH(FLAG_TEST "-Wl,--large-address-aware" "-Wl,--nxcompat" "-Wl,--tsaware")
	# CMake doesn't like "+" characters in variable names.
	STRING(REPLACE "+" "_" FLAG_TEST_VARNAME "${FLAG_TEST}")

	CHECK_C_COMPILER_FLAG("${FLAG_TEST}" LDFLAG_${FLAG_TEST_VARNAME})
	IF(LDFLAG_${FLAG_TEST_VARNAME})
		SET(MCR_EXE_LINKER_FLAGS_WIN32 "${MCR_EXE_LINKER_FLAGS_WIN32} ${FLAG_TEST}")
	ENDIF(LDFLAG_${FLAG_TEST_VARNAME})
	UNSET(LDFLAG_${FLAG_TEST_VARNAME})
	UNSET(FLAG_TEST_VARNAME)
ENDFOREACH()
SET(MCR_SHARED_LINKER_FLAGS_WIN32 "${MCR_EXE_LINKER_FLAGS_WIN32}")
SET(MCR_MODULE_LINKER_FLAGS_WIN32 "${MCR_EXE_LINKER_FLAGS_WIN32}")

# EXE-only flags.
FOREACH(FLAG_TEST "-Wl,--tsaware")
	# CMake doesn't like "+" characters in variable names.
	STRING(REPLACE "+" "_" FLAG_TEST_VARNAME "${FLAG_TEST}")

	CHECK_C_COMPILER_FLAG("${FLAG_TEST}" LDFLAG_${FLAG_TEST_VARNAME})
	IF(LDFLAG_${FLAG_TEST_VARNAME})
		SET(MCR_EXE_LINKER_FLAGS_WIN32 "${MCR_EXE_LINKER_FLAGS_WIN32} ${FLAG_TEST}")
	ENDIF(LDFLAG_${FLAG_TEST_VARNAME})
	UNSET(LDFLAG_${FLAG_TEST_VARNAME})
	UNSET(FLAG_TEST_VARNAME)
ENDFOREACH()

# Test for dynamicbase (ASLR) support.
# Simply enabling --dynamicbase won't work; we also need to
# tell `ld` to generate the .reloc section. Also, there's
# a bug in `ld` where if it generates the .reloc section,
# it conveniently forgets the entry point.
# Reference: https://lists.libav.org/pipermail/libav-devel/2014-October/063871.html

# NOTE: Entry point is set using SET_WINDOWS_ENTRYPOINT()
# in platform.cmake due to ANSI/Unicode differences.
FOREACH(FLAG_TEST "-Wl,--dynamicbase,--pic-executable")
	# CMake doesn't like "+" characters in variable names.
	STRING(REPLACE "+" "_" FLAG_TEST_VARNAME "${FLAG_TEST}")

	CHECK_C_COMPILER_FLAG("${FLAG_TEST}" LDFLAG_${FLAG_TEST_VARNAME})
	IF(LDFLAG_${FLAG_TEST_VARNAME})
		# Entry point is only set for EXEs.
		# GNU `ld` always has the -e option.
		SET(MCR_EXE_LINKER_FLAGS_WIN32 "${MCR_EXE_LINKER_FLAGS_WIN32} ${FLAG_TEST}")
	ENDIF(LDFLAG_${FLAG_TEST_VARNAME})
	UNSET(LDFLAG_${FLAG_TEST_VARNAME})
	UNSET(FLAG_TEST_VARNAME)
ENDFOREACH()

# Enable windres support on MinGW.
# http://www.cmake.org/Bug/view.php?id=4068
SET(CMAKE_RC_COMPILER_INIT windres)
ENABLE_LANGUAGE(RC)

# NOTE: Setting CMAKE_RC_OUTPUT_EXTENSION doesn't seem to work.
# Force windres to output COFF, even though it'll use the .res extension.
SET(CMAKE_RC_OUTPUT_EXTENSION .obj)
SET(CMAKE_RC_COMPILE_OBJECT
	"<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> <INCLUDES> -o <OBJECT> <SOURCE>")

# Append the CFLAGS and LDFLAGS.
SET(MCR_C_FLAGS_COMMON			"${MCR_C_FLAGS_COMMON} ${MCR_C_FLAGS_WIN32}")
SET(MCR_CXX_FLAGS_COMMON		"${MCR_CXX_FLAGS_COMMON} ${MCR_C_FLAGS_WIN32}")
SET(MCR_EXE_LINKER_FLAGS_COMMON		"${MCR_EXE_LINKER_FLAGS_COMMON} ${MCR_EXE_LINKER_FLAGS_WIN32}")
SET(MCR_SHARED_LINKER_FLAGS_COMMON	"${MCR_SHARED_LINKER_FLAGS_COMMON} ${MCR_SHARED_LINKER_FLAGS_WIN32}")
SET(MCR_MODULE_LINKER_FLAGS_COMMON	"${MCR_MODULE_LINKER_FLAGS_COMMON} ${MCR_MODULE_LINKER_FLAGS_WIN32}")

# Unset temporary variables.
UNSET(MCR_C_FLAGS_WIN32)
UNSET(MCR_EXE_LINKER_FLAGS_WIN32)
UNSET(MCR_SHARED_LINKER_FLAGS_WIN32)
UNSET(MCR_MODULE_LINKER_FLAGS_WIN32)
