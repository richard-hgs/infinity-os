/**
 * @file prsfs.h
 * @author Infinity Technology
 * @date 02/29/2020
 * @brief My simple file table header file.
 ****************************************************************
 */

#ifndef _PRSFS_H_
#define _PRSFS_H_

/* Size of a sector in bytes. */
#define BLOCK_SIZE 512
/* Size of file system table on disk in sectors. */
#define TABLE_SIZE 7
/* How many table copies on disk. */
#define TABLE_COUNT 1
/* Starting sector of table. */
#define TABLE_START 1
/* Max files allowed on disk. */
#define MAXFILES 32
/* Simple file table structure. */
struct file {
	unsigned char filename[8];		/* file name */
	unsigned char extension[3];		/* file extension */
	unsigned char _unused;			/* padding (not used) */
	unsigned short start;			/* starting LBA sector */
	unsigned short count;			/* total sectors of file */
};
typedef struct file file_t;

/* ------------------------ Table Functions ------------------------- */

/* Initialize file table structure */
void init_table(file_t *table);
/* Add entry to file table */
void add_entry_table(file_t *table, const char *filename);
/* Write table to disk image */
int write_table(int fout, file_t *table);
/* Print table to stdout in hexadecimal */
void print_table(file_t *table);
/* Loop through all file entries, with custom function. */
void loop_table(file_t *table, void (*callback)(file_t *, int id));

/* ------------------------ Misc Functions -------------------------- */

/* Convert table entry file name to 8.3 format */
char *get_filename(const unsigned char *filename);
/* Convert 8.3 file name to file system file name */
char *convert_filename(const char *filename);
/* Get filename from current file. */
const char **get_fname(void);

#endif
