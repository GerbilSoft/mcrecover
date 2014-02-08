# Convert text files to native line endings.
# This only has an effect on Win32 platforms, since the software is developed
# on Linux and hence uses Unix line endings.

# NOTE: CONFIGURE_FILE()'s NEWLINE_STYLE parameter was
# added in cmake-2.8.7.
IF(WIN32)
	CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)
ENDIF(WIN32)

# Parameters:
# - _filenames: Variable to store converted (or as-is) filenames in.
# - Additional: Files to convert.
MACRO(CONVERT_TEXT_FILES_TO_NATIVE _filenames)
	IF(WIN32)
		# Win32: Convert text files to use Windows line endings.
		UNSET(${_filenames})
		FOREACH(_current_FILE ${ARGN})
			CONFIGURE_FILE(${_current_FILE}
				"${CMAKE_CURRENT_BINARY_DIR}/${_current_FILE}"
				NEWLINE_STYLE CRLF
				)
			SET(${_filenames} ${${_filenames}} "${CMAKE_CURRENT_BINARY_DIR}/${_current_FILE}")
		ENDFOREACH(_current_FILE)
	ELSE(WIN32)
		# Not Win32. Don't do anything.
		SET(${_filenames} ${ARGN})
	ENDIF(WIN32)
ENDMACRO(CONVERT_TEXT_FILES_TO_NATIVE)
