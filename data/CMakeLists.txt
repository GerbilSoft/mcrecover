PROJECT(data)
cmake_minimum_required(VERSION 2.6)

#################
# Installation. #
#################

SET(DATABASE_FILES
	GcnMcFileDb.xml
	)

INSTALL(FILES ${DATABASE_FILES}
	DESTINATION share/mcrecover/data
	)

IF(UNIX AND NOT APPLE)
	# FreeDesktop.org desktop file specification.
	INSTALL(FILES mcrecover.desktop
		DESTINATION share/applications
		)
	# /usr/share/pixmaps
	INSTALL(FILES mcrecover.xpm
		DESTINATION share/pixmaps
		)
ENDIF(UNIX AND NOT APPLE)
