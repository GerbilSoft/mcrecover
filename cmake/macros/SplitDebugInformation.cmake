# Split debug information from an executable into a separate file.
# SPLIT_DEBUG_INFORMATION(EXE_TARGET)
#
# References:
# - http://cmake.3232098.n2.nabble.com/Save-stripped-debugging-information-td6819195.html
# - http://sourceware.org/bugzilla/show_bug.cgi?id=14527
#   - If debug symbols are stripped before .gnu_debuglink is added,
#     the section will be truncated to .gnu_deb, and hence won't
#     be recognized by gdb.
# - FIXME: If the above .gnu_debuglink workaround is used, Windows XP
#   and Windows 7 will claim that the executable isn't a valid Win32
#   executable. (Wine ignores it and works fine!)
#
MACRO(SPLIT_DEBUG_INFORMATION EXE_TARGET)
	# NOTE: $<TARGET_FILE:gcbanner> is preferred,
	# but this doesn't seem to work on Ubuntu 10.04.
	# (cmake_2.8.0-5ubuntu1_i386)
	GET_PROPERTY(SPLITDEBUG_EXE_LOCATION TARGET ${EXE_TARGET} PROPERTY LOCATION)

	FIND_PROGRAM(OBJCOPY objcopy)
	ADD_CUSTOM_COMMAND(TARGET ${EXE_TARGET} POST_BUILD
		COMMAND ${OBJCOPY} --only-keep-debug
			${SPLITDEBUG_EXE_LOCATION} ${CMAKE_CURRENT_BINARY_DIR}/${EXE_TARGET}.debug
		COMMAND ${OBJCOPY} --strip-debug
			${SPLITDEBUG_EXE_LOCATION}
		COMMAND ${OBJCOPY} --add-gnu-debuglink=${EXE_TARGET}.debug
			${SPLITDEBUG_EXE_LOCATION}
		)
	UNSET(SPLITDEBUG_EXE_LOCATION)
ENDMACRO(SPLIT_DEBUG_INFORMATION)
