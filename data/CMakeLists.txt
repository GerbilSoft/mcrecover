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

# FreeDesktop.org desktop file specification.
IF(UNIX AND NOT APPLE)
	INSTALL(FILES mcrecover.desktop
		DESTINATION share/applications
		)
ENDIF(UNIX AND NOT APPLE)
