# Build options.

# Link-time optimization.
OPTION(ENABLE_LTO	"Enable link-time optimization. (Release builds only)"	0)

# Split debug information into a separate file.
OPTION(SPLIT_DEBUG	"Split debug information into a separate file."		1)

# Compress the executable with UPX.
OPTION(COMPRESS_EXE	"Compress the executable with UPX."			0)
