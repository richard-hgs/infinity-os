#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "fat.h"

enum class EnMode {
    UNDEFINED,
    CREATE,
    WRITE
};

enum class EnFormat {
    UNDEFINED,
    FAT32
};

/**
 * @brief Program to create a file system
 * 
 * USAGE: 
 *   - ARG 0            : imagefs       command name
 *   - ARG 1            : <mode>        'c'=CREATE, 'w'=WRITE
 *   - <mode 'c'> ARG 2 : <format>      fat32=Fat32 file system format
 *   - <mode 'c'> ARG 3 : <size>        Total storage sectors amount
 *   - <mode 'c'> ARG 4 : <secsize>     Size in bytes for one sector
 *   - <mode 'c'> ARG 5 : <file>        Output file that will store this new created file system
 * 
 * @param argc Arguments count
 * @param argv Arguments list
 * @return int Result code or Error code
 */
int main(int argc, char **argv) {
    bool printUsage = false;
    EnMode mode = EnMode::UNDEFINED;
    EnFormat format = EnFormat::UNDEFINED;
    int totalSec = 0;
    int bytesPerSec = 0;

    if (argc >= 2) {
        if (strcmp(argv[1], "w") == 0) {
            mode = EnMode::WRITE;
        } else if (strcmp(argv[1], "c") == 0) {
            mode = EnMode::CREATE;
        } else {
            fprintf(stderr, "\e[31mUnknow <mode> \"%s\" used. Use {c = CREATE, w = WRITE}. Use help to know more.\e[m\n", argv[1]);
            return 1;
        }
    }

    if (mode == EnMode::CREATE) { // Create a new file system architecture binary data
        if (argc < 6) {
            printUsage = true;
        } else {
            if (strcmp(argv[2], "fat32") == 0) {
                format = EnFormat::FAT32;
            } else {
                fprintf(stderr, "\e[31mUnknow <format> \"%s\" used. Use {fat32}. Use help to know more.\e[m\n", argv[1]);
                return 1;
            }

            totalSec = atoi(argv[3]);
            if (totalSec <= 0) {
                fprintf(stderr, "\e[31mUnknow <size> \"%s\" used. Size must be greather than 0.\e[m\n", argv[1]);
                return -1;
            }

            bytesPerSec = atoi(argv[4]);
            if (bytesPerSec <= 0) {
                fprintf(stderr, "\e[31mUnknow <secsize> \"%s\" used. Sector size must be greather than 0.\e[m\n", argv[1]);
                return -1;
            }

            fprintf(stdout, "Creating format.\n");

            // Begin the creation of the fat32 file system format.
            FatBS32 fatBS32 = {};
            fat::create(totalSec, bytesPerSec, 0x3A1B, &fatBS32);

            // fprintf(stdout, "fatBS32 - oemName: %s\n", (char*) fatBS32.header.BS_OEMName);
        }
    }

    if (printUsage) {
        fprintf(stderr, "\e[31mUsage %s <mode> <format> <size> <secsize> <file>\e[m\n", argv[0]);
        return 1;
    }

    fprintf(stdout, "Argument count: %i - mode: %i.\n", argc, static_cast<int>(mode));

    return 0;
}