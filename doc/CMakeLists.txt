PROJECT(doc)
cmake_minimum_required(VERSION 2.6)

#################
# Installation. #
#################

INCLUDE(DirInstallPaths)

SET(LICENSE_FILES
	gpl-2.0.txt
	lgpl-2.1.txt
	cc-by-sa-3.0.txt
	bsd-3.txt
	)

SET(DOC_FILES
	GcnMcFileDb.EXAMPLE.xml
	)

INSTALL(FILES ${LICENSE_FILES} ${DOC_FILES}
	DESTINATION "${DIR_INSTALL_DOC}"
	)
