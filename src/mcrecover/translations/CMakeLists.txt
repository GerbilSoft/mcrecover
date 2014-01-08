PROJECT(mcrecover_translations)
cmake_minimum_required(VERSION 2.6)

# mcrecover translations subproject.

# Compile the translations.
SET(TRANSLATION_FILES
	mcrecover_en.ts
	mcrecover_1337.ts
	)
QT4_ADD_TRANSLATION(QM_FILES ${TRANSLATION_FILES})

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

# TODO: Install system translations on Win32 and Apple.
