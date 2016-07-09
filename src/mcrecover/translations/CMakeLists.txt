PROJECT(mcrecover_translations)
cmake_minimum_required(VERSION 2.6)

# mcrecover translations subproject.
IF(USE_QT4)
	FIND_PACKAGE(Qt4 COMPONENTS QtCore REQUIRED)
ELSEIF(USE_QT5)
	FIND_PACKAGE(Qt5LinguistTools REQUIRED)
ENDIF()

# Compile the translations.
SET(TRANSLATION_FILES
	mcrecover_en.ts
	mcrecover_1337.ts
	mcrecover_en_GB.ts
	mcrecover_es.ts
	mcrecover_ru.ts
	)

IF(USE_QT4)
	QT4_ADD_TRANSLATION(QM_FILES ${TRANSLATION_FILES})
ELSEIF(USE_QT5)
	QT5_ADD_TRANSLATION(QM_FILES ${TRANSLATION_FILES})
ENDIF()

# Make the destination files available in parent score.
SET(QM_FILES ${QM_FILES} PARENT_SCOPE)
ADD_CUSTOM_TARGET(translations_target DEPENDS ${QM_FILES})

#################
# Installation. #
#################

INCLUDE(DirInstallPaths)

INSTALL(FILES ${QM_FILES}
	DESTINATION "${DIR_INSTALL_TRANSLATIONS}"
	COMPONENT "translations"
	)
