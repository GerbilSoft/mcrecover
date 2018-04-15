# Localization files.
PROJECT(locale)

# mcrecover translations subproject.
FIND_PACKAGE(Qt5 ${QT_REQUIRED} COMPONENTS Core)
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

# System translations.
# Install on Windows and Mac OS X, or if we're using statically-linked Qt.
# Dynamically-linked Qt uses system translations.
IF(ENABLE_NLS AND (QT_IS_STATIC OR APPLE OR NOT UNIX))
	FILE(GLOB QT_SYS_TRANSLATIONS "${QT_TRANSLATIONS_DIR}/qt_??.qm")
	INSTALL(FILES ${QT_SYS_TRANSLATIONS}
		DESTINATION ${DIR_INSTALL_TRANSLATIONS}
		COMPONENT "translations"
		)
ENDIF(ENABLE_NLS AND (QT_IS_STATIC OR APPLE OR NOT UNIX))
