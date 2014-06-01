# MiniU2D: Mini unix2dos program.
# Used for Win32 builds if the host doesn't have unix2dos.

PROJECT(miniu2d)
cmake_minimum_required(VERSION 2.6)

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
