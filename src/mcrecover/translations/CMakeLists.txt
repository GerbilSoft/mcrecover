PROJECT(mcrecover_translations)

# mcrecover translations subproject.
IF(QT_VERSION EQUAL 5)
	FIND_PACKAGE(Qt5LinguistTools REQUIRED)
	# Version-specific macros.
	MACRO(QTX_ADD_TRANSLATION)
		QT5_ADD_TRANSLATION(${ARGV})
	ENDMACRO(QTX_ADD_TRANSLATION)
ELSEIF(QT_VERSION EQUAL 4)
	FIND_PACKAGE(Qt4 COMPONENTS QtCore REQUIRED)
	# Version-specific macros.
	MACRO(QTX_ADD_TRANSLATION)
		QT4_ADD_TRANSLATION(${ARGV})
	ENDMACRO(QTX_ADD_TRANSLATION)
ELSE()
	MESSAGE(FATAL_ERROR "Unsupported Qt version: ${QT_VERSION}")
ENDIF()

# Compile the translations.
SET(TRANSLATION_FILES
	mcrecover_en.ts
	mcrecover_1337.ts
	mcrecover_en_GB.ts
	mcrecover_es.ts
	mcrecover_ru.ts
	)

QTX_ADD_TRANSLATION(QM_FILES ${TRANSLATION_FILES})

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
