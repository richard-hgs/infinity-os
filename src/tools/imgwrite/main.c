/*
 * main.c - Main source file for imgwrite program.
 *
 * Author: Infinity Technology
 * Date: 12/17/2020
 *
 ***********************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#define MAXBUF 512

/* Entry point for IMGWRITE program.
 */
int main(int argc, char **argv)
{
	unsigned long bytes_written = 0, bytes_read = 0;
	unsigned long total_bytes = 0, total_checked = 0;
	char buf[MAXBUF], buf2[MAXBUF];
	FILE *fin, *fout;

	if(argc != 3) {
		fprintf(stderr, "Usage: %s <input-file> <output-file>\n", argv[0]);
		return 1;
	}

	/* Open files for reading and writing. */
	errno = 0;
	fin = fopen(argv[1], "rb");
	if(errno != 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return 1;
	}
	errno = 0;
	fout = fopen(argv[2], "wb");
	if(errno != 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		fclose(fin);
		return 1;
	}

	/* Copy file contents to output device/file. */
	while((bytes_read = fread(buf, 1, MAXBUF-1, fin)) > 0) {
		bytes_written = fwrite(buf, 1, bytes_read, fout);
		if(bytes_written != bytes_read) {
			fseek(fin, -bytes_read, SEEK_CUR);
			fseek(fout, -bytes_written, SEEK_CUR);
			fprintf(stderr, "Warning: Failed to write trying again.\n");
			continue;
		}
		total_bytes += bytes_written;
	}
	fflush(fout);
	fclose(fout);

	/* Rewind input and re-open output for reading. */
	rewind(fin);
	errno = 0;
	fout = fopen(argv[2], "rb");
	if(errno != 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		fclose(fin);
		return 1;
	}

	/* Verify contents of input file against output file. */
	while((bytes_read = fread(buf, sizeof(char), MAXBUF, fin)) > 0) {
		bytes_written = fread(buf2, sizeof(char), MAXBUF, fout);
		if(bytes_read != bytes_written) {
			fseek(fin, -bytes_read, SEEK_CUR);
			fseek(fout, -bytes_written, SEEK_CUR);
			fprintf(stderr, "Warning: Failed to read trying again.\n");
			continue;
		}
		if(memcmp(buf, buf2, bytes_written) == 0) {
			total_checked += bytes_written;
		} else {
			break;
		}
	}
	fclose(fin);
	fclose(fout);

	if(total_bytes != total_checked) {
		fprintf(stdout, "Failed to copy and/or verify.\n");
		return EXIT_FAILURE;
	}

	fprintf(stdout, "Successfully copied and verified.\n");
	return EXIT_SUCCESS;
}
