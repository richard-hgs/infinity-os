/*
 * main.c - Main source file for boot sector writer and image creator.
 * 1 - Can create a file.img with the specified sectors-count.
 * 2 - Can write files into the first position of the file.img
 * 3 - Can't write multiple files, only one file is allowed
 *
 * Author: Infinity Technology
 * Date: 02/28/2020
 *
 *************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BLOCK_SIZE 512

/*
 * ------ PERMISSIONS (CHMOD) -------
 *  TYPE  ROOT  GROUP  USERS
 *   -	  ---    ---    ---
 * 
 * Permission   Binary		Decimal
 *   ---	  	 000		  0			// No permission defined
 *   --x	  	 001		  1			// Can only execute
 *   -w-	  	 010		  2			// Can only write
 *   -wx	  	 011		  3			// Can write and execute
 *   r--	  	 100		  4			// Can only read
 *   r-x	  	 101		  5			// Can read and execute
 *   rw-	  	 110		  6			// Can read and write
 *   rwx	  	 111		  7			// Can read, write and execute
 * 
 * TYPES:
 *   d => directory
 *   b => block archive
 *   c => special character file
 *   p => channel
 *   s => socket
 *   - => "normal" file
 * 
 * PERMISSIONS:
 *   rw- => the first part is the owner
 *   rw- => the seccond part is the group
 *   r-- => the last part is the users
 *   -------------------------------------
 *   r => read     permission (read);
 *   w => write    permission (write);
 *   x => execute  permission (execution);
 *   - => disabled permission (disabled).
 */

/**
 * Write sectors to floppy disk.
 * @param fout 		Output file-descriptor floppy.img
 * @param skip		The amount of sectors to be skipped
 * @param filename	The file name to be writted to the output-file
 */
int write_sectors(int fout, ssize_t skip, const char *filename)
{
	ssize_t nsect = 0, nbytes, count;
	char buf[BLOCK_SIZE];
	off_t off;
	int fd;

	/* Open the input-file and get file-descriptor */
	errno = 0;
	if((fd = open(filename, O_RDONLY, 0644)) < 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return 1;
	}

	/* Move the cursor to the end of the input file-contents */
	errno = 0;
	off = lseek(fd, 0, SEEK_END);
	if(off < 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		close(fd);
		return 1;
	}

	/* Move the cursor to the start of input file-contents */
	if(lseek(fd, 0, SEEK_SET) < 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		close(fd);
		return 1;
	}

	/* Calculate the number of sectors to be written based on the input file size */
	count = off / 512;
	count = (count == 0 ? 1 : count);

	/* Move the cursor to the first-sector + skip-sectors */
	errno = 0;
	off = lseek(fout, BLOCK_SIZE*skip, SEEK_SET);
	if(off < 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		close(fd);
		return 1;
	}

	/* While the number of sectors < max-sector AND number of bytes is greather than zero */
	while(nsect < count && (nbytes = read(fd, buf, sizeof(buf))) >= 0) {
		/* Write into the output-file the input-file bytes */ 
		if(write(fout, buf, nbytes) >= 0) {
			++nsect;
		}
	}
	close(fd);
	printf("Total sectors written %ld/%ld at %ld.\n",
		nsect, count, skip);
	return 0;
}
/* Create floppy disk size in sectors.
 */
int create_floppy(const char *filename, size_t count)
{
	size_t nsect = 0;
	char buf[BLOCK_SIZE];
	int fd;

	errno = 0;
	if((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return 1;
	}
	errno = 0;
	memset(buf, 0, sizeof(buf));
	while(nsect < count && write(fd, buf, BLOCK_SIZE) > 0) {
		nsect++;
	}
	close(fd);
	if(errno != 0) {
		fprintf(stderr, "Error: Total sectors written %lu/%lu\n",
			nsect, count);
		return 1;
	}
	printf("Total sectors written %lu/%lu\n", nsect, count);
	return 0;
}
/**
 * Program to create a floppy disk.
 * @param argc 	Argument count
 * @param argv	Arguments list
 * 				ARG0: imagefs			command name to execute
 * 				ARG1: <mode> 			'c'=CREATE, 'w'=WRITE
 * 				ARG2: <image>			image path ex: floppy.img
 * 				ARG3: <file>  			file path to write
 * 				ARG4: [skip-count]		
 */
int main(int argc, char **argv)
{
	char mode = 0;

	if(argc < 4 || argc > 5) {
		fprintf(stderr, "Usage: %s <mode> <image> <file> [skip-count]\n",
			argv[0]);
		return 1;
	}
	switch(argv[1][0]) {
		case 'c':
			create_floppy(argv[2], atoi(argv[3]));
		break;
		case 'w':
			if(argc == 4) {
				mode = 1;
			} else if(argc == 5) {
				mode = 2;
			} else {
				printf("Unknown write mode.\n");
				return 1;
			}
		break;
	default:
		printf("Unknown option '%c'.\n", argv[1][0]);
		return 1;
	}
	if(mode == 1) {
		/* WRITE MODE */
		int fd;

		/* Open the output floppy.img file */
		errno = 0;
		if((fd = open(argv[2], O_RDWR, 0644)) < 0) {
			fprintf(stderr, "Error: %s\n", strerror(errno));
			return 1;
		}

		/* Write the input-file(argv[3]) to the output floppy.img file */
		if(write_sectors(fd, 0, argv[3])) {
			close(fd);
			return 1;
		}
		close(fd);
	} else if(mode == 2) {
		/* WRITE MODE WITH SKIP SECTORS COUNT */
		int fd;

		/* Open the output floppy.img file */
		errno = 0;
		if((fd = open(argv[2], O_RDWR, 0644)) < 0) {
			fprintf(stderr, "Error: %s\n", strerror(errno));
			return 1;
		}

		/* Write the input-file(argv[3]) to the output floppy.img file and skip-sectors(argv[4]) */
		if(write_sectors(fd, atoi(argv[4]), argv[3])) {
			close(fd);
			return 1;
		}
		close(fd);
	}
	return 0;
}
