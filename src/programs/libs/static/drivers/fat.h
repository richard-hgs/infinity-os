#pragma once
#ifndef _FAT_H_
#define _FAT_H_

/**
 * @brief FAT - File Allocation Table
 * 
 * BPB BOOT SECTOR - FAT12/FAT16/FAT32 Common fields:
 *  _________________________________________________________________________________________________
 * | offset | size | Current Value       |  Description                                              |
 * |    0   |   2  | 0xEB, 0x5A          | Jmp to boot entry point                                   |
 * |    2   |   1  | 0x90                | Required jmp end                                          |
 * |    3   |   8  | INFINITY            | Fat OEM signature                                         |
 * |   11   |   2  | 512 BYTES           | Bytes per sector                                          |
 * |   13   |   1  | 1 SECTOR            | Sectors per cluster                                       |
 * |   14   |   2  | 1 FAT32 OBLIG       | Reserved sectors                                          |
 * |   16   |   1  | 2 COMPATIBLE        | FAT count                                                 |
 * |   17   |   2  | 0 FAT32 OBLIG       | Amount of dir entries in root dir                         |
 * |   19   |   2  | 0 FAT32 OBLIG       | 16-bit total count of sectors on the volume               |
 * |   21   |   1  | f0 REMOVABLE        | Media type                                                |
 * |   22   |   2  | 0 FAT32 OBLIG       | 16 bit count of sectors used by one fat                   |
 * |   24   |   2  | 18 SECT/TRACK       | Sectors per track                                         |
 * |   26   |   2  | 1 HEAD              | Read/Write disk heads                                     |
 * |   28   |   4  | 0 HIDDEN SECT       | Count of hidden sectors                                   |
 * |   32   |   4  | 2880 SECTORS        | 32 bit total count of sectors on the volume               |
 * |   36   |   4  | 28 SECTORS          | Sectors used by one fat structure                         |
 * |   40   |   2  | 0 ACTIVE FAT        | Extended flags. Zero based number of active fat           |
 * |   42   |   2  | 0:0 MAJ/MIN VERSION | File system Major and Minor veresion                      |
 * |   44   |   4  | 2 ROOT FIRST CLUSTE | Root directory first cluster number                       |
 * |   48   |   2  | 1º SECTOR OF RESERV | Sector number of FSINFO structure in reserved area        |
 * |   50   |   2  | 6º SECTOR OF RESERV | Sector number of copy of the boot record in reserved area |
 * |   52   |  12  | RESERVED AREA       | Reserved for future expansion                             |
 * |   64   |   1  | DRIVER 0            | Holds the boot drive number                               |
 * |   65   |   1  | RESERVED 1          | Reserved, empty                                           |
 * |   66   |   1  | BOOT SIGNATURE 41   | Extended boot signature                                   |
 * |   67   |   4  | 0xFACE VOLUME ID    | Disk serial number                                        |
 * |   71   |  11  | Main Volume LABEL   | Volume label max length 11                                |
 * |   82   |   8  | FAT32 FSystem Type  | File System type                                          |
 *  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 * 
 */
namespace fat {
    /**
     * @brief Create a Fat32 formatation data to be writted in the storage device.
     * This will generate all required configuration for currently format being generated.
     * 
     * @param BPB_RsvdSecCnt Reserved sectors count before the first FAT entry. Specify reserved area for boot executable code.
     */
    void create();
}

#endif