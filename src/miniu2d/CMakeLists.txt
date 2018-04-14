PROJECT(miniu2d)
# MiniU2D: Mini unix2dos program.
# Used for Win32 builds if the host doesn't have unix2dos.

# Sources.
SET(miniu2d_SRCS
	miniu2d.c
	)

#########################
# Build the executable. #
#########################

ADD_EXECUTABLE(miniu2d
	${miniu2d_SRCS}
	)
DO_SPLIT_DEBUG(miniu2d)
SET_WINDOWS_SUBSYSTEM(miniu2d CONSOLE)
SET_WINDOWS_ENTRYPOINT(miniu2d main OFF)
