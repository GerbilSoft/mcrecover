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
