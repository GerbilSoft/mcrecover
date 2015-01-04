# Convert text files to native line endings.
# This only has an effect on Win32 platforms, since the software is developed
# on Linux and hence uses Unix line endings.

IF(WIN32 AND NOT MSVC)
	# Search for unix2dos.
	FIND_PROGRAM(UNIX2DOS unix2dos)
ENDIF(WIN32 AND NOT MSVC)

# Parameters:
# - _filenames: Variable to store converted (or as-is) filenames in.
# - Additional: Files to convert.
MACRO(CONVERT_TEXT_FILES_TO_NATIVE _filenames)
	IF(WIN32 AND NOT MSVC)
		# Win32: Convert text files to use Windows line endings.
		IF(NOT UNIX2DOS)
			GET_PROPERTY(MINIU2D_EXE_LOCATION TARGET miniu2d PROPERTY LOCATION)
		ENDIF(NOT UNIX2DOS)

		UNSET(${_filenames})
		FOREACH(_current_FILE ${ARGN})
			SET(CURRENT_SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE}")
			SET(CURRENT_OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${_current_FILE}")

			IF(UNIX2DOS)
				# Use unix2dos.
				# FIXME: Not working with nmake...
				ADD_CUSTOM_COMMAND(
					OUTPUT "${CURRENT_OUTPUT_FILE}"
					COMMAND ${UNIX2DOS} <"${CURRENT_SOURCE_FILE}" >"${CURRENT_OUTPUT_FILE}"
					)
			ELSE(UNIX2DOS)
				# unix2dos wasn't found. Use miniu2d.
				# NOTE: This won't work if cross-compiling, since
				# cmake doesn't support compiling for the `build' system.
				IF(CMAKE_CROSSCOMPILING)
					MESSAGE(FATAL_ERROR "unix2dos not found and we're cross-compiling; cannot convert documentation")
				ENDIF(CMAKE_CROSSCOMPILING)
				ADD_CUSTOM_COMMAND(
					OUTPUT "${CURRENT_OUTPUT_FILE}"
					DEPENDS miniu2d
					COMMAND "${MINIU2D_EXE_LOCATION}" "${CURRENT_SOURCE_FILE}" "${CURRENT_OUTPUT_FILE}"
					)
			ENDIF(UNIX2DOS)

			SET(${_filenames} ${${_filenames}} "${CURRENT_OUTPUT_FILE}")
			UNSET(CURRENT_OUTPUT_FILE)
		ENDFOREACH(_current_FILE)

		# Make sure the targets are always built.
		ADD_CUSTOM_TARGET(TEXT_FILES_${_filenames} ALL
			DEPENDS ${${_filenames}}
			)

		IF(NOT UNIX2DOS)
			UNSET(MINIU2D_EXE_LOCATION)
		ENDIF(NOT UNIX2DOS)
	ELSE(WIN32 AND NOT MSVC)
		# Not Win32, or compiling with MSVC. Don't do anything.
		SET(${_filenames} ${ARGN})
	ENDIF(WIN32 AND NOT MSVC)
ENDMACRO(CONVERT_TEXT_FILES_TO_NATIVE)
