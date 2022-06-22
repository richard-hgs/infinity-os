/**
 * @file prsfs.c
 * @author Infinity Technology
 * @date 02/29/2020
 * @brief My simple file table source file.
 ****************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "prsfs.h"

/* Count of tables used. */
static unsigned long int _file_table;
/* Count of files in table. */
static unsigned long int _file_count;
/* Storage for current file. */
static const char *_prsfs_filename[MAXFILES*TABLE_SIZE*TABLE_COUNT];

/* ------------------------- Table Functions ------------------------- */

/* Initialize the file table.
 */
void init_table(file_t *table)
{
	unsigned long i;
	for(i = 0; i < TABLE_SIZE; i++)
		memset(table, 0, sizeof(file_t)*MAXFILES*TABLE_SIZE*TABLE_COUNT);
	_file_table = _file_count = 0;
}
/* Add file entry to table.
 */
void add_entry_table(file_t *table, const char *filename)
{
	if(filename == NULL) {
		return;
	} else if(_file_table >= TABLE_SIZE || _file_count >= MAXFILES) {
		fprintf(stderr, "Warning: Max file entries reached, %lu/%d.\n",
			_file_count, MAXFILES);
	} else {
		char *tmp;
		_prsfs_filename[_file_table+_file_count] = filename;
		if((tmp = strrchr(filename, '/')) != NULL) {
			memcpy(table[_file_table*sizeof(file_t)+_file_count].filename,
				convert_filename(++tmp), 11);
		} else {
			memcpy(table[_file_table*sizeof(file_t)+_file_count].filename,
				convert_filename(filename), 11);
		}
		table[_file_table*sizeof(file_t)+_file_count]._unused = 0;
		table[_file_table*sizeof(file_t)+_file_count].start = 0;
		table[_file_table*sizeof(file_t)+_file_count].count = 0;
		_file_count++;
		if(_file_count >= MAXFILES) _file_table++;
	}
}
/* Write table to file at second sector.
 */
int write_table(int fout, file_t *table)
{
	unsigned char buf[BLOCK_SIZE];
	const char **filenames = get_fname();
	size_t total_sectors = 0;
	int nbytes;
	size_t i, j;

	/* seek past the boot sector */
	errno = 0;
	lseek(fout, TABLE_START*512, SEEK_SET);
	if(errno != 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return 1;
	}
	/* write table to disk image */
	nbytes = write(fout, table, sizeof(file_t)*MAXFILES*TABLE_SIZE*TABLE_COUNT);
	if(nbytes < 0) {
		fprintf(stderr, "Error: Writing to disk retrying sector.\n");
		return 2;
	}
	total_sectors += ((nbytes % 512) == 0 ? (nbytes/512) : (nbytes/512)+1);
	printf("Table written to disk totaling %lu sectors.\n", total_sectors);
	/* write files to disk image */
	for(i = 0, j = 0; i < _file_count; i++) {
		const char *filename = get_filename(table[j*sizeof(file_t)+i].filename);
		unsigned short nsect;
		size_t total_bytes;
		off_t off;
		int fd;
		errno = 0;
		if((fd = open(*filenames++, O_RDONLY, 0644)) < 0) {
			fprintf(stderr, "File [%s] error: %s\n", filename, strerror(errno));
			continue;
		}
		nsect = 0;
		total_bytes = 0;
		while(nsect < table[j*sizeof(file_t)+i].count
				&& (nbytes = read(fd, buf, sizeof(buf))) > 0) {
			if((nbytes = write(fout, buf, nbytes)) > 0) {
				total_bytes += nbytes;
				nsect++;
			}
		}
		close(fd);
		total_sectors += nsect;
		printf("Wrote file %s at %u totaling %u/%u sectors.\n",
			filename, table[j*sizeof(file_t)+i].start, nsect,
			table[j*sizeof(file_t)+i].count);
		off = lseek(fout, 0, SEEK_CUR);
		if(off < 0)
			fprintf(stderr, "Warning [Tell %ld]: failed.\n", (off/512));
		off += (512*nsect)-total_bytes;
		printf("Total bytes remaining: %ld\n", (512*nsect)-total_bytes);
		off = lseek(fout, off, SEEK_SET);
		if(off < 0)
			fprintf(stderr, "Warning [Seek %ld]: failed.\n", off);
		if(i >= MAXFILES) { i = 0; j++; }
		if(j >= TABLE_SIZE) break;
	}
	if(j >= MAXFILES) {
		fprintf(stderr, "Error: Not enough room for data.\n");
		return 2;
	}
	printf("Total sectors written to disk %lu.\n", total_sectors);
	return 0;
}
/* Print table to stdout in hexadecimal.
 */
void print_table(file_t *table)
{
	unsigned long i, j, k;
	if(_file_table > 0) {
		for(i = 0; i < _file_table; i++) {
			for(j = 0; j < _file_count; j++) {
				for(k = 0; k < sizeof(file_t); k++) {
					printf("%x%s", table[i*sizeof(file_t)+j].filename[k],
						((k < sizeof(file_t)) ? " " : ""));
				}
				putchar('\n');
			}
		}
	} else {
		for(i = 0; i < _file_count; i++) {
			for(j = 0; j < sizeof(file_t); j++) {
				printf("%x%s", table[_file_table*sizeof(file_t)+i].filename[j],
					((j < sizeof(file_t)) ? " " : ""));
			}
			putchar('\n');
		}
	}
}
/* Loop through all table entries, using a custom function.
 */
void loop_table(file_t *table, void (*callback)(file_t *, int))
{
	unsigned long i, j;
	if(_file_table > 0) {
		for(i = 0; i < _file_table; i++) {
			for(j = 0; j < _file_count; j++) {
				(*callback)(&table[i*sizeof(file_t)+j], i);
			}
		}
	} else {
		for(i = 0; i < _file_count; i++)
			(*callback)(&table[_file_table*sizeof(file_t)+i], i);
	}
}

/* ------------------------- Misc Functions ------------------------- */

/* Convert filename to normal 8.3 format.
 */
char *get_filename(const unsigned char *filename)
{
	static char converted[13];
	int i, j;
	for(i = 0, j = 0; j < 8 && filename[j] != ' '; i++,j++)
		converted[i] = filename[j];
	while(j < 8 && filename[j] == ' ') j++;
	if(filename[j] != ' ')
		converted[i++] = '.';
	for(; j < 11 && filename[j] != ' '; i++,j++)
		converted[i] = filename[j];
	converted[i] = '\0';
	return converted;
}
/* Convert filename to file system filename.
 */
char *convert_filename(const char *filename)
{
	static char converted[13];
	int i, j;
	for(i = 0, j = 0; j < 8 && filename[j] != '.'; i++,j++)
		converted[i] = filename[j];
	while(i < 8 && filename[j] == '.')
		converted[i++] = ' ';
	++j;
	for(; j < 11 && filename[j] != ' '; i++,j++)
		converted[i] = filename[j];
	converted[i] = '\0';
	return converted;
}
/* Get current filename.
 */
const char **get_fname(void)
{
	return _prsfs_filename;
}
