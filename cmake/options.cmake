# Build options.

# Link-time optimization.
 # Link-time optimization.
# FIXME: Not working in clang builds and Ubuntu's gcc...
IF(MSVC)
	SET(LTO_DEFAULT ON)
ELSE()
	SET(LTO_DEFAULT OFF)
ENDIF()
OPTION(ENABLE_LTO "Enable link-time optimization in release builds." ${LTO_DEFAULT})

# Split debug information into a separate file.
OPTION(SPLIT_DEBUG "Split debug information into a separate file." 1)

# Install the split debug file.
OPTION(INSTALL_DEBUG "Install the split debug files." ON)
IF(INSTALL_DEBUG AND NOT SPLIT_DEBUG)
	# Cannot install debug files if we're not splitting them.
	SET(INSTALL_DEBUG OFF CACHE INTERNAL "Install the split debug files." FORCE)
ENDIF(INSTALL_DEBUG AND NOT SPLIT_DEBUG)

# Compress the executable with UPX.
OPTION(COMPRESS_EXE "Compress the executable with UPX." 0)

# Enable D-Bus for DockManager / Unity API.
IF(UNIX AND NOT APPLE)
	OPTION(ENABLE_DBUS "Enable D-Bus support for DockManager / Unity API." 1)
ELSE(UNIX AND NOT APPLE)
	SET(ENABLE_DBUS 0 CACHE INTERNAL "Enable D-Bus support for DockManager / Unity API." FORCE)
ENDIF(UNIX AND NOT APPLE)

# Internal libpng.
# NOTE: Also controls internal zlib usage.
# TODO: Separate it so int libpng / ext zlib can be used?
# TODO: Add OPTION(USE_PNG) to disable libpng support entirely?
IF(NOT WIN32)
	OPTION(USE_INTERNAL_PNG	"Always use the internal copy of libpng." 0)
ELSE(NOT WIN32)
	# TODO: Allow use of external libpng on Win32?
	SET(USE_INTERNAL_PNG 1)
ENDIF(NOT WIN32)

# giflib.
OPTION(USE_GIF			"Use giflib for GIF exports." 1)
IF(NOT WIN32)
	OPTION(USE_INTERNAL_GIF	"Always use the internal copy of giflib." 0)
ELSE(NOT WIN32)
	# TODO: Allow use of external giflib on Win32?
	SET(USE_INTERNAL_GIF 1)
ENDIF(NOT WIN32)

# Translations.
OPTION(ENABLE_NLS "Enable NLS using Qt's built-in localization system." ON)
