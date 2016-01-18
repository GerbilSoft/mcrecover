# Build options.

# Link-time optimization.
OPTION(ENABLE_LTO	"Enable link-time optimization. (Release builds only)"  0)

# Split debug information into a separate file.
OPTION(SPLIT_DEBUG	"Split debug information into a separate file." 1)

# Install the split debug file.
OPTION(INSTALL_DEBUG	"Install the split debug file." 1)

# Compress the executable with UPX.
OPTION(COMPRESS_EXE	"Compress the executable with UPX." 0)

# Enable DBus for DockManager / Unity API.
IF(UNIX AND NOT APPLE)
OPTION(ENABLE_DBUS	"Enable DBUS support for DockManager / Unity API." 1)
ELSE(UNIX AND NOT APPLE)
SET(ENABLE_DBUS 0)
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

# Internal PCRE.
OPTION(USE_INTERNAL_PCRE	"Always use the internal copy of PCRE." 0)

# giflib.
OPTION(USE_GIF			"Use giflib for GIF exports." 1)
IF(NOT WIN32)
	OPTION(USE_INTERNAL_GIF	"Always use the internal copy of giflib." 0)
ELSE(NOT WIN32)
	# TODO: Allow use of external giflib on Win32?
	SET(USE_INTERNAL_GIF 1)
ENDIF(NOT WIN32)
