/***************************************************************************
 * Mini Unix2DOS: Convert UNIX text files to DOS format.                   *
 *                                                                         *
 * Copyright (c) 2014 by David Korth.                                      *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/**
 * Main entry point.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @return Return value.
 */
int main(int argc, char *argv[])
{
	// File pointers.
	FILE *f_unix, *f_dos;
	// Text buffers.
	char unix_in[4096];
	char dos_out[8192];	// Double size for worst-case.

	/**
	 * Syntax:
	 * ./miniu2d unixfile.txt dosfile.txt
	 * - unixfile.txt: UNIX text file to convert.
	 * - dosfile.txt: DOS text file output.
	 */

	if (argc != 3) {
		if (argc < 3) {
			fprintf(stderr, "%s: not enough parameters\n", argv[0]);
		} else if (argc > 3) {
			fprintf(stderr, "%s: too many parameters\n", argv[0]);
		}
		fprintf(stderr, "syntax: %s unixfile.txt dosfile.txt\n"
			"- unixfile.txt will be converted from UNIX format to DOS format.\n"
			"- dosfile.txt will contain the converted text.\n", argv[0]);
		return EXIT_FAILURE;
	}

	// Open the files.
	f_unix = fopen(argv[1], "rb");
	if (!f_unix) {
		fprintf(stderr, "ERROR opening UNIX text file '%s': %s\n", argv[1], strerror(errno));
		return EXIT_FAILURE;
	}
	f_dos = fopen(argv[2], "wb");
	if (!f_dos) {
		fclose(f_unix);
		fprintf(stderr, "ERROR opening DOS text file '%s': %s\n", argv[2], strerror(errno));
		return EXIT_FAILURE;
	}

	// Process the file.
	char last_chr = 0;
	while (!feof(f_unix)) {
		size_t bytes_in, bytes_out, pos;

		bytes_in = fread(unix_in, 1, sizeof(unix_in), f_unix);
		if (bytes_in == 0) {
			// Error?
			if (ferror(f_unix)) {
				fprintf(stderr, "ERROR reading UNIX text file '%s': %s\n",
					argv[1], strerror(errno));
				fclose(f_unix);
				fclose(f_dos);
				return EXIT_FAILURE;
			}
		}

		// Process the bytes.
		bytes_out = 0;
		for (pos = 0; pos < bytes_in; pos++) {
			switch (unix_in[pos]) {
				case '\n': {
					if (last_chr != '\r') {
						// Last character was not CR.
						// Convert to CRLF.
						dos_out[bytes_out++] = '\r';
						dos_out[bytes_out++] = '\n';
					} else {
						// Last character was CR.
						// Output newline as-is.
						dos_out[bytes_out++] = '\n';
					}
					break;
				}

				default: {
					if (last_chr == '\r') {
						// Last character was CR.
						// Convert to CRLF.
						dos_out[bytes_out++] = '\r';
						dos_out[bytes_out++] = '\n';
					} else {
						// Other character.
						dos_out[bytes_out++] = unix_in[pos];
					}
				}
			}

			last_chr = unix_in[pos];
		}

		// Write the bytes.
		pos = fwrite(dos_out, 1, bytes_out, f_dos);
		if (pos != bytes_out) {
			// Short write!
			fprintf(stderr, "ERROR writing DOS text file '%s': short write\n", argv[1]);
			fclose(f_unix);
			fclose(f_dos);
			return EXIT_FAILURE;
		}
	}

	// Finished converting the file.
	fclose(f_unix);
	fclose(f_dos);
}
