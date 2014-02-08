PROJECT(data)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#################
# Installation. #
#################

INCLUDE(DirInstallPaths)
INCLUDE(ConvertTextFilesToNative)

CONVERT_TEXT_FILES_TO_NATIVE(DATABASE_FILES
	GcnMcFileDb.USA.xml
	GcnMcFileDb.PAL.xml
	GcnMcFileDb.JPN.xml
	GcnMcFileDb.Unlicensed.xml
	GcnMcFileDb.Homebrew.xml
	)

INSTALL(FILES ${DATABASE_FILES}
	DESTINATION "${DIR_INSTALL_DATA}"
	COMPONENT "database"
	)
