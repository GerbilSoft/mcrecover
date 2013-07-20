PROJECT(data)
cmake_minimum_required(VERSION 2.6)

#################
# Installation. #
#################

INCLUDE(DirInstallPaths)

SET(DATABASE_FILES
	GcnMcFileDb.xml
	)

INSTALL(FILES ${DATABASE_FILES}
	DESTINATION "${DIR_INSTALL_DATA}"
	)
