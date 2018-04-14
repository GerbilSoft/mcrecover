PROJECT(mcrecover_translations)

# mcrecover translations subproject.
FIND_PACKAGE(Qt5LinguistTools REQUIRED)

# Compile the translations.
SET(TRANSLATION_FILES
	mcrecover_en.ts
	mcrecover_1337.ts
	mcrecover_en_GB.ts
	mcrecover_es.ts
	mcrecover_ru.ts
	)

QT5_ADD_TRANSLATION(QM_FILES ${TRANSLATION_FILES})

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
