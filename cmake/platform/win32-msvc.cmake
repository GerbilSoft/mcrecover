# Win32-specific CFLAGS/CXXFLAGS.
# For Microsoft Visual C++ compilers.

# Basic platform flags:
# - wchar_t should be a distinct type.
# - Enable strict type checking in the Windows headers.
# - Set minimum Windows version to Windows 2000. (Windows NT 5.0)
SET(MCRECOVER_C_FLAGS_WIN32 "-Zc:wchar_t -DSTRICT -D_WIN32_WINNT=0x0500")

# Release build: Prefer static libraries.
IF(CMAKE_BUILD_TYPE MATCHES ^release)
	SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ENDIF(CMAKE_BUILD_TYPE MATCHES ^release)

# Append the CFLAGS.
SET(MCRECOVER_BASE_C_FLAGS_COMMON "${MCRECOVER_BASE_C_FLAGS_COMMON} ${MCRECOVER_C_FLAGS_WIN32}")
SET(MCRECOVER_BASE_CXX_FLAGS_COMMON "${MCRECOVER_BASE_CXX_FLAGS_COMMON} ${MCRECOVER_C_FLAGS_WIN32}")
UNSET(MCRECOVER_C_FLAGS_WIN32)
