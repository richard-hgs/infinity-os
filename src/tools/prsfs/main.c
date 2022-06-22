/**
 * @file main.c
 * @author Infinity Technology
 * @date 02/29/2020
 * @brief Write my file system to the disk image.
 *********************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#include "prsfs.h"
#include "unused.h"

/* Macro to print error message */
#define ERROR_PRINT(x) fprintf(stderr, "Error on file [%s]: %s\n", \
	x, strerror(errno))
/* Loop through all entries in table, getting sizes.
 */
static void set_info_entry(file_t *entry, int id)
{
	static unsigned short start = (TABLE_START+TABLE_SIZE)*TABLE_COUNT;
	const char **filenames = get_fname();
	static size_t i = 0, j = 0, k = 0;
	off_t off;
	int fd;

	if(i >= MAXFILES && j >= TABLE_SIZE) return;
	errno = 0;
	if((fd = open(filenames[k], O_RDONLY, 0644)) < 0) {
		ERROR_PRINT(filenames[k]);
		return;
	}
	errno = 0;
	off = lseek(fd, 0, SEEK_END);
	if(off < 0) {
		ERROR_PRINT(filenames[k]);
		close(fd);
		return;
	}
	++k;
	entry->start = start;
	entry->count = ((off % 512) == 0 ? (off/512) : (off/512)+1);
	start += ((off % 512) == 0 ? (off/512) : (off/512)+1);
	close(fd);
	++i;
	if(i >= MAXFILES) { i = 0; ++j; }
}
/* Loop through all entries in table, converting filenames.
 */
static void print_entry(file_t *entry, int id)
{
	printf(
		"=================================================================\n"
		"\t\t*** Entry %d ***\n"
		"=================================================================\n"
		"File name: %s\n"
		"Start sector: %u\n"
		"Sector count: %u\n",
		id+1, get_filename(entry->filename), entry->start, entry->count);
}
/* Program to test my file system table.
 */
int main(int argc, char **argv)
{
	file_t table[TABLE_SIZE*MAXFILES];
	const char *image;
	int fd;

	if(argc < 3) {
		fprintf(stderr, "Usage: %s <image> <filename.ext> [...]\n", argv[0]);
		return 1;
	}
	if(argc-1 >= MAXFILES) {
		fprintf(stderr, "Error: Cannot store more than %d in file system.\n",
			MAXFILES);
		return 2;
	}
	init_table(table);
	image = *++argv;
	if((fd = open(image, O_WRONLY, 0644)) < 0) {
		ERROR_PRINT(image);
		return 3;
	}
	while(*++argv != NULL)
		add_entry_table(table, *argv);
	loop_table(table, set_info_entry);
	write_table(fd, table);
	loop_table(table, print_entry);
	close(fd);
	return 0;
}
