# Build options.

# Link-time optimization.
OPTION(ENABLE_LTO	"Enable link-time optimization. (Release builds only)"	0)

# Split debug information into a separate file.
OPTION(SPLIT_DEBUG	"Split debug information into a separate file."		1)

# Install the split debug file.
OPTION(INSTALL_DEBUG	"Install the split debug file."				1)

# Compress the executable with UPX.
OPTION(COMPRESS_EXE	"Compress the executable with UPX."			0)

# Enable DBus for DockManager / Unity API.
IF(UNIX AND NOT APPLE)
OPTION(ENABLE_DBUS	"Enable DBUS support for DockManager / Unity API."	1)
ELSE(UNIX AND NOT APPLE)
SET(ENABLE_DBUS 0)
ENDIF(UNIX AND NOT APPLE)
