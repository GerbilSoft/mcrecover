# Win32-specific CFLAGS/CXXFLAGS.
# For Microsoft Visual C++ compilers.

# Basic platform flags for MSVC:
# - wchar_t should be a distinct type. (MSVC 2002+)
IF(MSVC_VERSION GREATER 1200)
	SET(MCR_C_FLAGS_WIN32 "${MCR_C_FLAGS_WIN32} /Zc:wchar_t")
ENDIF()

# NOTE: This program is Unicode only on Windows.
# No ANSI support.

# Subsystem and minimum Windows version:
# - If 32-bit: 5.00
# - If 64-bit: 5.02
# GCN MemCard Recover does NOT support ANSI Windows.
# NOTE: MSVC 2008 and 2010 have a minimum subsystem value of 4.00.
# NOTE: MSVC 2012 and later have a minimum subsystem value of 5.01.
# NOTE: CMake sets /subsystem:windows or /subsystem:console itself,
# but with the MSBUILD generator, that's *before* this one, which
# results in console programs being linked as Windows.
# Use the SET_WINDOWS_SUBSYSTEM(_target _subsystem) function defined in
# platform.cmake. That macro will only do anything if building with
# MSVC, since the variables won't be defined anywhere else.
IF(CMAKE_CL_64)
	# 64-bit, Unicode Windows only.
	# (There is no 64-bit ANSI Windows.)
	# Minimum target version is Windows Server 2003 / XP 64-bit.
	SET(MCR_WIN32_SUBSYSTEM_VERSION "5.02")
ELSE(CMAKE_CL_64)
	# 32-bit, Unicode Windows only.
	IF(MSVC_VERSION GREATER 1600)
		# MSVC 2012 or later.
		# Minimum target version is Windows XP.
		SET(MCR_WIN32_SUBSYSTEM_VERSION "5.01")
	ELSE()
		# MSVC 2010 or earlier.
		# Minimum target version is Windows 2000.
		SET(MCR_WIN32_SUBSYSTEM_VERSION "5.00")
	ENDIF()
ENDIF(CMAKE_CL_64)
SET(MCR_LINKER_FLAGS_WIN32_EXE "/SUBSYSTEM:WINDOWS,${MCR_WIN32_SUBSYSTEM_VERSION}")
SET(MCR_LINKER_FLAGS_CONSOLE_EXE "/SUBSYSTEM:CONSOLE,${MCR_WIN32_SUBSYSTEM_VERSION}")
UNSET(MCR_WIN32_SUBSYSTEM_VERSION)

# Release build: Prefer static libraries.
# TODO: Use DLLs instead?
IF(CMAKE_BUILD_TYPE MATCHES ^release)
	SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ENDIF(CMAKE_BUILD_TYPE MATCHES ^release)

# Append the CFLAGS and LDFLAGS.
SET(MCR_C_FLAGS_COMMON			"${MCR_C_FLAGS_COMMON} ${MCR_C_FLAGS_WIN32}")
SET(MCR_CXX_FLAGS_COMMON		"${MCR_CXX_FLAGS_COMMON} ${MCR_C_FLAGS_WIN32} ${MCR_CXX_FLAGS_WIN32}")

# Unset temporary variables.
UNSET(MCR_C_FLAGS_WIN32)
