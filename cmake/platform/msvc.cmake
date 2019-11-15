# Microsoft Visual C++
IF(MSVC_VERSION LESS 1600)
	MESSAGE(FATAL_ERROR "MSVC 2010 (10.0) or later is required.")
ENDIF()

# CMake has a bunch of defaults, including /Od for debug and /O2 for release.
# Remove some default CFLAGS/CXXFLAGS.
STRING(REPLACE "/GR" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
STRING(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Disable useless warnings:
# - MSVC "logo" messages
# - C4355: 'this' used in base member initializer list (used for Qt Dpointer pattern)
# - MSVCRT "deprecated" functions
# - std::tr1 deprecation
# Increase some warnings to errors:
# - C4013: function undefined; this is allowed in C, but will
#   probably cause a linker error.
# - C4024: 'function': different types for formal and actual parameter n
# - C4047: 'function': 'parameter' differs in levels of indirection from 'argument'
SET(MCR_C_FLAGS_COMMON "/nologo /wd4355 /wd4482 /we4013 /we4024 /we4047 -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE")
SET(MCR_CXX_FLAGS_COMMON "${MCR_C_FLAGS_COMMON} -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")
# NOTE: /TSAWARE is automatically set for Windows 2000 and later. (as of at least Visual Studio .NET 2003)
# NOTE 2: /TSAWARE is not applicable for DLLs.
SET(MCR_EXE_LINKER_FLAGS_COMMON "/NOLOGO /DYNAMICBASE /NXCOMPAT /LARGEADDRESSAWARE")
SET(MCR_SHARED_LINKER_FLAGS_COMMON "${MCR_EXE_LINKER_FLAGS_COMMON}")
SET(MCR_MODULE_LINKER_FLAGS_COMMON "${MCR_EXE_LINKER_FLAGS_COMMON}")

# Disable C++ RTTI and asynchronous exceptions.
SET(MCR_CXX_FLAGS_COMMON "${MCR_CXX_FLAGS_COMMON} -GR- -EHsc")

# Test for "/sdl" and "/guard:cf".
INCLUDE(CheckCCompilerFlag)
FOREACH(FLAG_TEST "/sdl" "/guard:cf")
	# CMake doesn't like certain characters in variable names.
	STRING(REGEX REPLACE "/|:" "_" FLAG_TEST_VARNAME "${FLAG_TEST}")

	CHECK_C_COMPILER_FLAG("${FLAG_TEST}" CFLAG_${FLAG_TEST_VARNAME})
	IF(CFLAG_${FLAG_TEST_VARNAME})
		SET(MCR_C_FLAGS_COMMON "${MCR_C_FLAGS_COMMON} ${FLAG_TEST}")
		SET(MCR_CXX_FLAGS_COMMON "${MCR_CXX_FLAGS_COMMON} ${FLAG_TEST}")
		IF(FLAG_TEST STREQUAL "/guard:cf")
			# "/guard:cf" must be added to linker flags as well.
			SET(MCR_EXE_LINKER_FLAGS_COMMON "${MCR_EXE_LINKER_FLAGS_COMMON} ${FLAG_TEST}")
			SET(MCR_SHARED_LINKER_FLAGS_COMMON "${MCR_SHARED_LINKER_FLAGS_COMMON} ${FLAG_TEST}")
			SET(MCR_MODULE_LINKER_FLAGS_COMMON "${MCR_MODULE_LINKER_FLAGS_COMMON} ${FLAG_TEST}")
		ENDIF(FLAG_TEST STREQUAL "/guard:cf")
	ENDIF(CFLAG_${FLAG_TEST_VARNAME})
	UNSET(CFLAG_${FLAG_TEST_VARNAME})
ENDFOREACH()

# Disable warning C4996 (deprecated), then re-enable it.
# Otherwise, it gets handled as an error due to /sdl.
SET(MCR_C_FLAGS_COMMON "${MCR_C_FLAGS_COMMON} /wd4996 /w34996")
SET(MCR_CXX_FLAGS_COMMON "${MCR_CXX_FLAGS_COMMON} /wd4996 /w34996")

# MSVC 2015 uses thread-safe statics by default.
# This doesn't work on XP, so disable it.
IF(MSVC_VERSION GREATER 1899)
	SET(MCR_C_FLAGS_COMMON   "${MCR_C_FLAGS_COMMON} /Zc:threadSafeInit-")
	SET(MCR_CXX_FLAGS_COMMON "${MCR_CXX_FLAGS_COMMON} /Zc:threadSafeInit-")
ENDIF()

# Disable the RC and MASM "logo".
# FIXME: Setting CMAKE_RC_FLAGS causes msbuild to fail,
# since CMake already sets /NOLOGO there.
# - Also disabling /NOLOGO for MASM just in case.
#SET(CMAKE_RC_FLAGS "/NOLOGO")
#SET(CMAKE_ASM_MASM_FLAGS "/NOLOGO")

# FIXME: MSVC 2015's 32-bit masm has problems when using msbuild:
# - The default /W3 fails for seemingly no reason. /W0 fixes it.
# - Compilation fails due to no SAFESEH handlers in inffas32.asm.
# NOTE: We're enabling these for all MSVC platforms, not just 32-bit.
# NOTE 2: We need to cache this in order to prevent random build failures
# caused by an empty string being cached instead.
SET(CMAKE_ASM_MASM_FLAGS "/W0 /safeseh" CACHE STRING
	"Flags used by the assembler during all build types.")

# Check if CMAKE_SIZEOF_VOID_P is set correctly.
IF(NOT CMAKE_SIZEOF_VOID_P)
	# CMAKE_SIZEOF_VOID_P isn't set.
	# Set it based on CMAKE_SYSTEM_PROCESSOR.
	# FIXME: This won't work if we're cross-compiling, e.g. using
	# the x86_amd64 or amd64_x86 toolchains.
	IF(CMAKE_CL_64)
		SET(CMAKE_SIZEOF_VOID_P 8)
	ELSE()
		SET(CMAKE_SIZEOF_VOID_P 4)
	ENDIF()
ENDIF(NOT CMAKE_SIZEOF_VOID_P)

# Debug/release flags.
SET(MCR_C_FLAGS_DEBUG			"/Zi")
SET(MCR_CXX_FLAGS_DEBUG			"/Zi")
SET(MCR_EXE_LINKER_FLAGS_DEBUG		"/DEBUG /INCREMENTAL")
SET(MCR_SHARED_LINKER_FLAGS_DEBUG	"${MCR_EXE_LINKER_FLAGS_DEBUG}")
SET(MCR_MODULE_LINKER_FLAGS_DEBUG	"${MCR_EXE_LINKER_FLAGS_DEBUG}")
SET(MCR_C_FLAGS_RELEASE			"/Zi")
SET(MCR_CXX_FLAGS_RELEASE		"/Zi")
SET(MCR_EXE_LINKER_FLAGS_RELEASE	"/DEBUG /INCREMENTAL:NO /OPT:ICF,REF")
SET(MCR_SHARED_LINKER_FLAGS_RELEASE	"${MCR_EXE_LINKER_FLAGS_RELEASE}")
SET(MCR_MODULE_LINKER_FLAGS_RELEASE	"${MCR_EXE_LINKER_FLAGS_RELEASE}")

# Check for link-time optimization. (Release builds only.)
IF(ENABLE_LTO)
	SET(MCR_C_FLAGS_RELEASE			"${MCR_C_FLAGS_RELEASE} /GL")
	SET(MCR_CXX_FLAGS_RELEASE		"${MCR_CXX_FLAGS_RELEASE} /GL")
	SET(MCR_EXE_LINKER_FLAGS_RELEASE	"${MCR_EXE_LINKER_FLAGS_RELEASE} /LTCG")
	SET(MCR_SHARED_LINKER_FLAGS_RELEASE	"${MCR_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
	SET(MCR_MODULE_LINKER_FLAGS_RELEASE	"${MCR_MODULE_LINKER_FLAGS_RELEASE} /LTCG")
ENDIF(ENABLE_LTO)
