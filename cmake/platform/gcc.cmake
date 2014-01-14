# gcc (and other Unix-like compilers, e.g. MinGW)

# Compiler flag modules.
INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCXXCompilerFlag)

# Check what flag is needed for C99 support.
INCLUDE(CheckC99CompilerFlag)
CHECK_C99_COMPILER_FLAG(MCRECOVER_C99_CFLAG)

# Check what flag is needed for C++ 2011 support.
INCLUDE(CheckCXX11CompilerFlag)
CHECK_CXX11_COMPILER_FLAG(MCRECOVER_CXX11_CXXFLAG)

# Disable C++ RTTI.
INCLUDE(CheckCXXNoRTTICompilerFlag)
CHECK_CXX_NO_RTTI_COMPILER_FLAG(MCRECOVER_CXX_NO_RTTI_CXXFLAG)

# Disable C++ exceptions.
INCLUDE(CheckCXXNoExceptionsCompilerFlag)
CHECK_CXX_NO_EXCEPTIONS_COMPILER_FLAG(MCRECOVER_CXX_NO_EXCEPTIONS_CXXFLAG)

# Check for link-time optimization.
# NOTE: LTO on MinGW is broken right now:
# `___mingw_raise_matherr' referenced in section `.text' of
# ../lib/libmingwex.a(lib32_libmingwex_a-log.o): defined in
# discarded section `.text' of lib32_libmingw32_a-merr.o (symbol from plugin)
IF(ENABLE_LTO)
	CHECK_C_COMPILER_FLAG("-flto" CFLAG_LTO)
	IF(CFLAG_LTO)
		SET(MCRECOVER_CFLAGS_LTO "-flto")
		IF(MINGW)
			# MinGW. Disable LTO partitioning, since it
			# causes problems with Qt.
			# Reference: http://sourceware.org/bugzilla/show_bug.cgi?id=12762
			CHECK_C_COMPILER_FLAG("-flto-partition=none" CFLAG_LTO_PARTITION)
			IF(CFLAG_LTO_PARTITION)
				SET(MCRECOVER_CFLAGS_LTO "${MCRECOVER_CFLAGS_LTO} -flto-partition=none")
			ENDIF(CFLAG_LTO_PARTITION)
			UNSET(CFLAG_LTO_PARTITION)
		ENDIF(MINGW)
		SET(MCRECOVER_LDFLAGS_LTO "${MCRECOVER_CFLAGS_LTO} -fuse-linker-plugin")
	ENDIF(CFLAG_LTO)
	UNSET(CFLAG_LTO)
ENDIF(ENABLE_LTO)

# Test for common CFLAGS and CXXFLAGS.
SET(MCRECOVER_CFLAGS_COMMON "")
SET(MCRECOVER_CXXFLAGS_COMMON "${MCRECOVER_CXX_NO_RTTI_CFLAG} ${MCRECOVER_CXX_NO_EXCEPTIONS_CFLAG}")
# NOTE: Commit visibility changes after the MSVC changes.
FOREACH(FLAG_TEST "-Wall" "-Wextra" "-fstrict-aliasing" "-fvisibility=hidden" "-fvisibility-inlines-hidden")
	CHECK_C_COMPILER_FLAG("${FLAG_TEST}" CFLAG_${FLAG_TEST})
	IF(CFLAG_${FLAG_TEST})
		SET(MCRECOVER_CFLAGS_COMMON "${MCRECOVER_CFLAGS_COMMON} ${FLAG_TEST}")
	ENDIF(CFLAG_${FLAG_TEST})
	UNSET(CFLAG_${FLAG_TEST})
	
	CHECK_CXX_COMPILER_FLAG("${FLAG_TEST}" CXXFLAG_${FLAG_TEST})
	IF(CXXFLAG_${FLAG_TEST})
		SET(MCRECOVER_CXXFLAGS_COMMON "${MCRECOVER_CXXFLAGS_COMMON} ${FLAG_TEST}")
	ENDIF(CXXFLAG_${FLAG_TEST})
	UNSET(CXXFLAG_${FLAG_TEST})
ENDFOREACH()

# Test for common LDFLAGS.
# TODO: Doesn't work on OS X. (which means it's not really testing it!)
SET(MCRECOVER_LDFLAGS_COMMON "")
IF(NOT APPLE)
	FOREACH(FLAG_TEST "-Wl,-O1" "-Wl,--sort-common" "-Wl,--as-needed")
		CHECK_C_COMPILER_FLAG("${FLAG_TEST}" LDFLAG_${FLAG_TEST})
		IF(LDFLAG_${FLAG_TEST})
			SET(MCRECOVER_LDFLAGS_COMMON "${MCRECOVER_LDFLAGS_COMMON} ${FLAG_TEST}")
		ENDIF(LDFLAG_${FLAG_TEST})
		UNSET(LDFLAG_${FLAG_TEST})
	ENDFOREACH()
ENDIF(NOT APPLE)

# Debug/release flags.
SET(MCRECOVER_CFLAGS_COMPILER_DEBUG "-O0 -ggdb")
SET(MCRECOVER_CXXFLAGS_COMPILER_DEBUG "-O0 -ggdb")
SET(MCRECOVER_CFLAGS_COMPILER_RELEASE "-O2 -ggdb")
SET(MCRECOVER_CXXFLAGS_COMPILER_RELEASE "-O2 -ggdb")
