# Convert text files to native line endings.
# This only has an effect on Win32 platforms, since the software is developed
# on Linux and hence uses Unix line endings.

# Search for unix2dos.
FIND_PROGRAM(UNIX2DOS unix2dos)

# Parameters:
# - _filenames: Variable to store converted (or as-is) filenames in.
# - Additional: Files to convert.
MACRO(CONVERT_TEXT_FILES_TO_NATIVE _filenames)
	IF(WIN32)
		# Win32: Convert text files to use Windows line endings.
		UNSET(${_filenames})
		FOREACH(_current_FILE ${ARGN})
			SET(CURRENT_SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE}")
			SET(CURRENT_OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${_current_FILE}")

			IF(UNIX2DOS)
				# Use UNIX2DOS.
				ADD_CUSTOM_COMMAND(
					OUTPUT "${CURRENT_OUTPUT_FILE}"
					COMMAND ${UNIX2DOS} -n "${CURRENT_SOURCE_FILE}" "${CURRENT_OUTPUT_FILE}"
					)
			ELSE(UNIX2DOS)
				# UNIX2DOS wasn't found. Use miniu2d.
				# TODO
			ENDIF(UNIX2DOS)

			SET(${_filenames} ${${_filenames}} "${CURRENT_OUTPUT_FILE}")
			UNSET(CURRENT_OUTPUT_FILE)
		ENDFOREACH(_current_FILE)

		# Make sure the targets are always built.
		ADD_CUSTOM_TARGET(TEXT_FILES_${_filenames} ALL
			DEPENDS ${${_filenames}}
			)
	ELSE(WIN32)
		# Not Win32. Don't do anything.
		SET(${_filenames} ${ARGN})
	ENDIF(WIN32)
ENDMACRO(CONVERT_TEXT_FILES_TO_NATIVE)
