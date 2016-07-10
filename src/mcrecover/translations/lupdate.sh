#!/bin/bash
# Qt lupdate script for mcrecover.
# Run this script to update the translations from the source code.
# NOTE: This script must be run from the translations/ directory!

# Source file extensions.
SRC_EXTS="h,hpp,c,cpp,m,mm,ui"

# Process the translation files.
TS_FILES=$(find . -maxdepth 1 -type f -name "*.ts")
if [[ -z "${TS_FILES}" ]]; then
	echo "*** ERROR: No translation files (*.ts) were found."
        echo "*** Create an empty translation file and try again."
	exit 1
fi

for TS_FILE in ${TS_FILES}; do
	if [ ! -s "${TS_FILE}" ]; then
		rm -f "${TS_FILE}"
	fi
	lupdate -extensions "${SRC_EXTS}" ../ -ts "${TS_FILE}"
done
