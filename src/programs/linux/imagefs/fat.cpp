// stdio
#include <string.h>
#include <stdio.h>

#include "fat.h"

/*
   PENDRIVE
   System Volume Information
   Folder1
   File1.txt
   File2 long name text size.txt
   Media
   2022-07-31 15-04-47.mkv.sfk
 */

/**
 * @brief Check if a fat cluster is FREE(unused)
 */
#define CLUSTER_IS_FREE(val) (val == 0x00000000)

/**
 * @brief Check if fat cluster is the EOC end of cluster chain
 */
#define CLUSTER_IS_EOC(val) (val >= 0x0FFFFFF8 && val <= 0x0FFFFFFF)

/**
 * @brief Check if fat cluster is bad and should not be used
 */
#define CLUSTER_IS_BAD(val) (val == 0x0FFFFFF7)

/**
 * @brief Check if dir (Fat32Directory) is free
 */
#define IS_DIR_FREE(dir) (dir.DIR_Name[0] == FAT_LDIR_UNUSED || dir.DIR_Name[0] == FAT_LDIR_LAST_AND_UNUSED)

/**
 * @brief Amount of sectors used by FAT - File allocation table entries
 */
#define FAT_SEC_SIZE(bs32) (bs32.header.BPB_NumFATs * bs32.BPB_FATSz32)

/**
 * @brief Amount of sectors used by FAT - Root Directory
 */
#define ROOT_DIR_SEC_SIZE(bs32) (((bs32.header.BPB_RootEntCnt * 32) + (bs32.header.BPB_BytesPerSec - 1)) / bs32.header.BPB_BytesPerSec)

/**
 * @brief The first sector where FAT - Data and Dir entries starts
 */
#define FIRST_DATA_SEC(bs32) (bs32.header.BPB_RsvdSecCnt + FAT_SEC_SIZE(bs32) + ROOT_DIR_SEC_SIZE(bs32))

/**
 * @brief The first data sector that references the first FAT cluster entry
 * 
 * OBS: Since this is an exhaustive math function it's recommended to store the FIRST_SEC_OF_CLUSTER
 *      and perform the calc manually inside exhaustive conditions like loops.
 */
#define FIRST_SEC_OF_CLUSTER(bs32) (((bs32.BPB_RootClus - 2) * bs32.header.BPB_SecPerClus) + FIRST_DATA_SEC(bs32))

/**
 * @brief The offset in bytes in the storage device where this specific dir entry index is located
 * 
 * OBS: Since this is an exhaustive math function it's recommended to store the FIRST_SEC_OF_CLUSTER
 *      and perform the calc manually inside exhaustive conditions like loops.
 */
#define DIR_ENTRY_BIN_OFFSET(bs32, index) ((index * 32) + (FIRST_SEC_OF_CLUSTER(bs32) * bs32.header.BPB_BytesPerSec))

#define DIR_FIRST_CLUSTER_HI(sector)

/** 
 * @brief This is the table for bs32 drives. NOTE that this table includes
 * entries for disk sizes smaller than 512 MB even though typically
 * only the entries for disks >= 512 MB in size are used.
 * The way this table is accessed is to look for the first entry
 * in the table for which the disk size is less than or equal
 * to the DiskSize field in that table entry. For this table to
 * work properly BPB_RsvdSecCnt must be 32, and BPB_NumFATs
 * must be 2. Any of these values being different may require the first 
 * table entries DiskSize value to be changed otherwise the cluster count 
 * may be to low for bs32.
 */
// DskSzToSecPerClus DskTablebs32[] = {
// // OFFSET WHERE DIRS START | DISK SIZE    
//     { 66600,                        0 },  /* disks up to 32.5 MB, the 0 value for SecPerClusVal trips an error */
//     { 532480,                       1 },  /* disks up to 260 MB, .5k cluster */
//     { 16777216,                     8 },  /* disks up to 8 GB, 4k cluster */
//     { 33554432,                    16 },  /* disks up to 16 GB, 8k cluster */
//     { 67108864,                    32 },  /* disks up to 32 GB, 16k cluster */
//     { 0xFFFFFFFF,                  64 }   /* disks greater than 32GB, 32k cluster */
// };

/**
 * @brief Returns an unsigned byte checksum computed on an unsigned byte array.
 *        The array must be 11 bytes long and is assumed to contain a name stored 
 *        in the format of a MS-DOS directory entry.
 * 
 * @param sfn 
 * @return unsigned char Sum - An 8-bit unsigned checksum of the array pointed to by sfn
 */
uint8_t checksum(uint8_t *sfn) {
	uint8_t crc = 0;
	uint8_t count = 11;
	do {
		crc = ((crc & 1) << 7) + (crc >> 1) + *sfn++;
	} while (--count);
	
	return crc;
}

/**
 * @brief Returns an unsigned byte checksum computed on an unsigned byte array.
 *        The array must be 11 bytes long and is assumed to contain a name stored 
 *        in the format of a MS-DOS directory entry.
 * 
 * @param val1 Int value 1 to compare
 * @param val2 Int value 2 to compare 
 * @return the max value against the two numbers
 */
int max(int val1, int val2) {
    return val1 > val2 ? val1 : val2;
}

void fat::create(uint32_t diskTotSec, uint16_t bytesPerSec, uint32_t fatSizeInSec, uint8_t mediaType, FatBS32* fatBs) {
    memcpy(fatBs->header.BS_JmpBoot, (unsigned char*) "\xEB\x58\x90", 3);
    memcpy(fatBs->header.BS_OEMName, "MSDOS5.0", 8);
    fatBs->header.BPB_BytesPerSec = bytesPerSec; // Bytes per sector
    fatBs->header.BPB_SecPerClus = 0x1;
    fatBs->header.BPB_RsvdSecCnt = 0x1;
    fatBs->header.BPB_NumFATs = 0x2;             // Fat count
    fatBs->header.BPB_RootEntCnt = 0x0;
    fatBs->header.BPB_TotSec16 = 0x0;
    fatBs->header.BPB_Media = mediaType;
    fatBs->header.BPB_FATSz16 = 0x0;
    fatBs->header.BPB_SecPerTrk = 0x12;      // 18
    fatBs->header.BPB_NumHeads = 0x1;
    fatBs->header.BPB_HiddSec = 0x0;
    fatBs->header.BPB_TotSec32 = diskTotSec;

    fatBs->BPB_FATSz32 = fatSizeInSec;
    fatBs->BPB_ExtFlags = 0x0;
    fatBs->BPB_FSVer = 0x0;
    fatBs->BPB_RootClus = 0x2;  // Root folder starts at cluster 2 of the FAT table
    fatBs->BPB_FSInfo = 0x1;    // FSInfo structure start sector
    fatBs->BPB_BkBootSec = 0x6; // Backup of the boot sector starts at sector 6
    memset(fatBs->BPB_Reserved, '\0', sizeof(unsigned char) * 12);
}

int fat::listEntries(FILE *storage, char* path) {
    FatBS32_t bs32;
    FatBSHeader_t bsHeader;
    Fat32FsInfo_t fsInfo32;
    Fat32Directory_t dirEntry32;
    Fat32LongDirectory_t lngDirEntry32;
    Fat32LongDirectory_t lngDirEntries32[19];
    int pathLength = strlen(path);
    int pathCharOffset;
    int clusterNum = 2;
    int dirEntryIndex;
    int errCode;
    uint32_t firstDataSector;
    uint32_t firstSectorOfCluster;
    uint32_t fatValue = 0;
    uint32_t lngDirEntries32Len;
    uint32_t tmpInt = 0;
    uint8_t mChecksum;
    uint8_t printDirs = 0;  // 0 = Don't print dirs, 1 = Print dirs
    uint8_t dirNameLength;
    char tmpStr[256];
    char pathPart[256];
    char *dirName = 0;
    char c;

    errCode = readBaseFatInfo(storage, &bsHeader, &bs32, &fsInfo32);
    if (errCode) {
        // Error while reading fat structure
        return errCode;
    }
    
    // Sector where the data sectors starts
    firstDataSector = FIRST_DATA_SEC(bs32);
    // Root directory sector location in data sectors
    firstSectorOfCluster = FIRST_SEC_OF_CLUSTER(bs32);

    fprintf(stdout, "Listing path \"%s\" entries: \n", path);

    if (strcmp(path, "/") == 0) {
        printDirs = 1;
    }

    // Split path names
    pathLength += 1;
    pathCharOffset = 0;
    c = 0;
    while(pathLength-- || printDirs) {
        if (printDirs == 0) {
            c = *path++;
        }
        if (c == '/' && pathCharOffset == 0) {
            // Ignore first path if it equals "/" root path
            continue;
        } else if (c == '/' || pathLength == 0 || printDirs == 1) {
            // Path found read directory three until all parts of path reached then list entries in the specified path
            *(pathPart + pathCharOffset) = 0; // Add NULL char to pathPart string end.
            // fprintf(stdout, "PathPart: %s - isLast: %d\n", pathPart, pathLength == 0);

            // Get current cluster fat value
            errCode = fat::fatVal(storage, bs32, clusterNum, &fatValue);
            if (errCode) {
                return errCode;
            }

            // fprintf(stdout, "--- FAT VALUE: 0x%08X\n", fatValue);

            if (CLUSTER_IS_BAD(fatValue)) {
                // Current cluster is bad
            } else if (CLUSTER_IS_FREE(fatValue)) {
                // Current cluster is free
            }
            
            // Read directory entries for current cluster number to find the pathPart directory name
            mChecksum = 0;
            lngDirEntries32Len = 0;
            dirEntryIndex = 0;
            do {
                // Offset relative to FAT storage
                uint32_t offset = (dirEntryIndex * 32) + (firstSectorOfCluster * bs32.header.BPB_BytesPerSec);

                // Set file offset at the beginning of the Root dir sectors
                fseek(storage, offset, SEEK_SET);

                // Read the first root directory entry
                readNextDirEntry(storage, &dirEntry32);

                // fprintf(stdout, "attrEquals: %d\n", (dirEntry32.DIR_Attr == FAT_DIR_ATTR_LONG_NAME));
                if (dirEntry32.DIR_Attr == FAT_DIR_ATTR_LONG_NAME) {
                        lngDirEntry32 = *(Fat32LongDirectory_t*) &dirEntry32;
                        if (lngDirEntry32.LDIR_Ord != FAT_LDIR_LAST_AND_UNUSED && lngDirEntry32.LDIR_Ord != FAT_LDIR_UNUSED) {
                            // If file is active and wasn't deleted
                            if (mChecksum != lngDirEntry32.LDIR_Chksum) { // It is a new long name reset tmpStr that will holds the Full name of the next directory entry.
                                // fprintf(stdout, "  - checksum: 0x%02X - 0x%02X\n", mChecksum, lngDirEntry32.LDIR_Chksum);
                                lngDirEntries32Len = 0;
                                mChecksum = lngDirEntry32.LDIR_Chksum;
                            }
                            lngDirEntries32[lngDirEntries32Len++] = lngDirEntry32;
                        }

                        // printLongDirEntry(lngDirEntry32);
                } else {
                    // Print dir entry
                    // printDirEntry(dirEntry32);
                    if (dirEntry32.DIR_Name[0] == FAT_LDIR_LAST_AND_UNUSED) {
                        // fprintf(stdout, "  - End Of DIR(/)\n");
                        if (printDirs == 1) {
                            printDirs = 2;
                        }
                        break;
                    } else if (dirEntry32.DIR_Name[0] != FAT_LDIR_UNUSED) {         // If it is an active dir entry
                        
                        if (mChecksum == checksum(dirEntry32.DIR_Name)) {           // Check if long name checksum matches with dir short name
                            // Read long name entries in desc order
                            tmpInt = 0;
                            while(lngDirEntries32Len--) {
                                lngDirEntry32 = lngDirEntries32[lngDirEntries32Len];
                                tmpInt = longNameStrCpy(lngDirEntry32, tmpInt, tmpStr);
                            }

                            dirName = tmpStr;
                            dirNameLength = tmpInt;
                        } else {
                            // Read dir name entries
                            dirName = (char*) dirEntry32.DIR_Name;
                            dirNameLength = 11;
                        }

                        if (printDirs == 1) {
                            fprintf(stdout, "   %.*s\n", dirNameLength, dirName);
                        }

                        if ((dirEntry32.DIR_Attr & FAT_DIR_ATTR_DIRECTORY) == FAT_DIR_ATTR_DIRECTORY) { // Is a directory
                            if (strncmp(dirName, pathPart, dirNameLength) == 0) {
                                // Path folder name found
                                // fprintf(stdout, "  - PathFound: %.*s\n", 11, dirEntry32.DIR_Name);
                                if (pathLength == 0) {
                                    // Is last path print next folder entries
                                    printDirs = 1;
                                }
                                firstSectorOfCluster = dirEntry32.DIR_FstClusHI;
                                firstSectorOfCluster = (firstSectorOfCluster << 16) | dirEntry32.DIR_FstClusLO;
                                clusterNum = firstSectorOfCluster;
                                // fprintf(stdout, "  - clusterNum: %d\n", firstSectorOfCluster);
                                firstSectorOfCluster = ((firstSectorOfCluster - 2) * bs32.header.BPB_SecPerClus) + firstDataSector;

                                // Exit directory loop since we already found the directory we are looking for
                                break;
                            }
                        }
                    }
                    lngDirEntries32Len = 0;
                    mChecksum = 0;
                }
            } while(dirEntryIndex++ || dirEntryIndex);

            if (printDirs == 2) {
                break;
            }

            if (printDirs == 0 && pathLength == 0 && CLUSTER_IS_EOC(fatValue)) {
                // End of cluster reached
                // Path not found return error
                return FAT_ERROR_LIST_PATH_NOT_FOUND;
            }

            pathCharOffset = 0;
        } else {
            // Append chars until split char reached
            *(pathPart + pathCharOffset) = c;
            pathCharOffset++;
        }
    }

    return FAT_NO_ERROR;
}

int fat::readFileEntry(FILE *storage, char* path) {
    // fprintf(stdout, "%.*s\n", dirNameLength, dirName);
//    Fat32Directory_t rootDir;
    int result = FAT_NO_ERROR;
//
//    fprintf(stdout, "reading file contents: %s\n", path);
//
//    result = findDirEntry(storage, path, &rootDir);

    Fat32RootDir_t rootDir;
    result = getRootDir(storage, &rootDir);

    return FAT_NO_ERROR;
}

int fat::findDirEntry(FILE *storage, char* path, Fat32Directory_t* dirEntry) {
    FatBS32_t bs32;
    FatBSHeader_t bsHeader;
    Fat32FsInfo_t fsInfo32;
    int result = FAT_NO_ERROR;

    // --------------- SEARCH DIR PATH ---------------
    int dirEntryIndex;
    uint8_t mChecksum;
    Fat32Directory_t dirEntry32;
    Fat32LongDirectory_t lngDirEntry32;
    Fat32LongDirectory_t lngDirEntries32[19];
    uint32_t lngDirEntries32Len;
    char tmpStr[256];
    int clusterNum = 2;
    uint32_t firstDataSector;
    uint32_t firstSectorOfCluster;
    uint8_t dirNameLength;
    char *dirName = 0;
    // --------------- SEARCH DIR PATH ---------------


    // --------------- SPLIT PATH VARS ---------------
    char pathPart[256];
    int pathLength = strlen(path);
    int pathCharOffset;
    char c;
    // --------------- SPLIT PATH VARS ---------------


    if (!(result = readBaseFatInfo(storage, &bsHeader, &bs32, &fsInfo32))) {
        // Read succeeded
        firstSectorOfCluster = FIRST_SEC_OF_CLUSTER(bs32);

        // Split path names
        pathLength += 1;
        pathCharOffset = 0;
        c = 0;
        while(pathLength--) {
            if (c == '/' && pathCharOffset == 0) {
                continue;
            } else if (c == '/' || pathLength == 0) {
                // Path split part found

                // Path found read directory three until all parts of path reached then list entries in the specified path
                *(pathPart + pathCharOffset) = 0; // Add NULL char to pathPart string end.

                // Read directory entries for current cluster number to find the pathPart directory name
                mChecksum = 0;
                lngDirEntries32Len = 0;
                dirEntryIndex = 0;

                do {
                    // Offset relative to FAT storage
                    uint32_t offset = (dirEntryIndex * 32) + (firstSectorOfCluster * bs32.header.BPB_BytesPerSec);

                    // Set file offset at the beginning of the Root dir sectors
                    fseek(storage, offset, SEEK_SET);

                    // Read the first root directory entry
                    readNextDirEntry(storage, &dirEntry32);

                    if (dirEntry32.DIR_Attr == FAT_DIR_ATTR_LONG_NAME) {
                        lngDirEntry32 = *(Fat32LongDirectory_t*) &dirEntry32;
                        if (lngDirEntry32.LDIR_Ord != FAT_LDIR_LAST_AND_UNUSED && lngDirEntry32.LDIR_Ord != FAT_LDIR_UNUSED) {
                            // If file is active and wasn't deleted
                            if (mChecksum != lngDirEntry32.LDIR_Chksum) { // It is a new long name reset tmpStr that will holds the Full name of the next directory entry.
                                // fprintf(stdout, "  - checksum: 0x%02X - 0x%02X\n", mChecksum, lngDirEntry32.LDIR_Chksum);
                                lngDirEntries32Len = 0;
                                mChecksum = lngDirEntry32.LDIR_Chksum;
                            }
                            lngDirEntries32[lngDirEntries32Len++] = lngDirEntry32;
                        }

                        // printLongDirEntry(lngDirEntry32);
                    } else {
                        if (dirEntry32.DIR_Name[0] == FAT_LDIR_LAST_AND_UNUSED) { // End of dir reached
                            // fprintf(stdout, "  - End Of DIR(/)\n");
                            break; // Break the dir loop to keep searching in dir tree
                        } else if (dirEntry32.DIR_Name[0] != FAT_LDIR_UNUSED) {
                            if (mChecksum == checksum(dirEntry32.DIR_Name)) {           // Check if long name checksum matches with dir short name
                                // Read long name entries in desc order
                                dirNameLength = 0;
                                while(lngDirEntries32Len--) {
                                    lngDirEntry32 = lngDirEntries32[lngDirEntries32Len];
                                    dirNameLength = longNameStrCpy(lngDirEntry32, dirNameLength, tmpStr);
                                }

                                dirName = tmpStr;
                            } else {
                                // Read dir name entries
                                dirName = (char*) dirEntry32.DIR_Name;
                                dirNameLength = 11;
                            }

                            fprintf(stdout, "   %.*s\n", dirNameLength, dirName);

                            if ((dirEntry32.DIR_Attr & FAT_DIR_ATTR_DIRECTORY) == FAT_DIR_ATTR_DIRECTORY) { // Is a directory
                                if (strncmp(dirName, pathPart, dirNameLength) == 0) {
                                    // Path folder name found. We need to list the folders inside this new folder until we find the name we are looking for
                                    // fprintf(stdout, "  - PathFound: %.*s\n", 11, dirEntry32.DIR_Name);
                                    firstSectorOfCluster = dirEntry32.DIR_FstClusHI;
                                    firstSectorOfCluster = (firstSectorOfCluster << 16) | dirEntry32.DIR_FstClusLO;
                                    clusterNum = firstSectorOfCluster;
                                    // fprintf(stdout, "  - clusterNum: %d\n", firstSectorOfCluster);
                                    firstSectorOfCluster = ((firstSectorOfCluster - 2) * bs32.header.BPB_SecPerClus) + firstDataSector;

                                    // Exit directory loop since we already found the directory we are looking for
                                    break;
                                }
                            }
                        }
                    }

                    dirEntryIndex++;
                } while(true);
            } else {
                // Append chars until split char reached
                *(pathPart + pathCharOffset) = c;
                pathCharOffset++;
            }
        }
    }

    return result;
}

int fat::readNextDirEntry(FILE *storage, Fat32Directory_t *fat32Dir) {
    // printf("file seek: 0x%02X\n", ftell(storage));
    if (fread(fat32Dir, 1, sizeof(Fat32Directory_t), storage) != sizeof(Fat32Directory_t)) {
        return FAT_ERROR_READ_DIRECTORY;
    }

    return FAT_NO_ERROR;
}

// int fat::readNextDirEntryFull(FILE *storage, Fat32DirFull_t *fat32Dir) {
//     Fat32LongDirectory_t lngDirEntries32[19];

//     do {
//         if (fread(fat32Dir->dirEntry, 1, sizeof(Fat32Directory_t), storage) != sizeof(Fat32Directory_t)) {
//             return FAT_ERROR_READ_DIRECTORY;
//         }
//     } while(fat32Dir->dirEntry.DIR_Attr == FAT_DIR_ATTR_LONG_NAME);

//     return FAT_NO_ERROR;
// }

int fat::fatVal(FILE *storage, FatBS32_t bs32, uint32_t clusterNum, uint32_t *fatValue) {
    uint32_t firstSectorOfFatEntries = bs32.header.BPB_RsvdSecCnt; // FAT 32 has 2 reserved fat entries

    // Set file offset at the beginning of the fat entry being readed
    fseek(storage, firstSectorOfFatEntries * bs32.header.BPB_BytesPerSec + (clusterNum * 4), SEEK_SET);

    // Read
    if (fread(fatValue, 1, 4, storage) != 4) {
        return FAT_ERROR_READ_FAT_ENTRY;
    }

    return FAT_NO_ERROR;
}

void fat::printDirEntry(Fat32Directory_t fat32Dir) {
    char dirAttrStr[80] = {0};

    dirAttrToStr(fat32Dir.DIR_Attr, dirAttrStr);

    fprintf(stdout, "DIRECTORY - 32:\n");
    fprintf(stdout, "  - DIR_Name         : %.*s\n", 11, fat32Dir.DIR_Name);
    fprintf(stdout, "  - DIR_Attr         : 0x%02X - %s\n", fat32Dir.DIR_Attr, dirAttrStr);
    fprintf(stdout, "  - DIR_NTRes        : 0x%02X\n", fat32Dir.DIR_NTRes);
    fprintf(stdout, "  - DIR_CrtTimeTenth : %d\n", fat32Dir.DIR_CrtTimeTenth);
    fprintf(stdout, "  - DIR_CrtTime      : %d\n", fat32Dir.DIR_CrtTime);
    fprintf(stdout, "  - DIR_CrtDate      : %d\n", fat32Dir.DIR_CrtDate);
    fprintf(stdout, "  - DIR_LstAccDate   : %d\n", fat32Dir.DIR_LstAccDate);
    fprintf(stdout, "  - DIR_FstClusHI    : %d\n", fat32Dir.DIR_FstClusHI);
    fprintf(stdout, "  - DIR_WrtTime      : %d\n", fat32Dir.DIR_WrtTime);
    fprintf(stdout, "  - DIR_WrtDate      : %d\n", fat32Dir.DIR_WrtDate);
    fprintf(stdout, "  - DIR_FstClusLO    : %d\n", fat32Dir.DIR_FstClusLO);
    fprintf(stdout, "  - DIR_FileSize     : %d\n", fat32Dir.DIR_FileSize);
    fprintf(stdout, "  - DIR_FstClusFull  : %d%d\n", fat32Dir.DIR_FstClusHI, fat32Dir.DIR_FstClusLO);
    fprintf(stdout, "  - DIR_Name_Checksum: 0x%02X\n", checksum(fat32Dir.DIR_Name));
}

void fat::printLongDirEntry(Fat32LongDirectory_t fat32LongDir) {
    char dirAttrStr[80] = {0};
    char tmpStr[256];
    int tmpInt;

    dirAttrToStr(fat32LongDir.LDIR_Attr, dirAttrStr);

    fprintf(stdout, "LONG DIRECTORY - 32:\n");
    fprintf(stdout, "  - LDIR_Ord         : %d%s\n", fat32LongDir.LDIR_Ord, (fat32LongDir.LDIR_Ord & 0x40) == 0x40 ? " - LAST LONG ENTRY" : "");
    tmpInt = longNameStrCpy(fat32LongDir.LDIR_Name1, 10, 0, tmpStr);
    tmpStr[tmpInt] = '\0';
    if (tmpInt < 5) {
        // End of string reached
        tmpInt = 0;
    }
 
    fprintf(stdout, "length: %d\n", tmpInt);
    fprintf(stdout, "  - LDIR_Name1       : %s\n", tmpStr);
    fprintf(stdout, "  - LDIR_Attr        : 0x%02X - %s\n", fat32LongDir.LDIR_Attr, dirAttrStr);
    fprintf(stdout, "  - LDIR_Type        : %d\n", fat32LongDir.LDIR_Type);
    fprintf(stdout, "  - LDIR_Chksum      : 0x%02X\n", fat32LongDir.LDIR_Chksum);
    if (tmpInt > 0) {
        tmpInt = longNameStrCpy(fat32LongDir.LDIR_Name2, 12, 0, tmpStr);
        tmpStr[tmpInt] = '\0';
        if (tmpInt < 6) {
            // End of string reached
            tmpInt = 0;
        }
    } else {
        tmpStr[0] = '\0';
    }
    fprintf(stdout, "  - LDIR_Name2       : %s\n", tmpStr);
    fprintf(stdout, "  - LDIR_FstClusLO   : %d\n", fat32LongDir.LDIR_FstClusLO);
    if (tmpInt > 0) {
        tmpInt = longNameStrCpy(fat32LongDir.LDIR_Name3, 4, 0, tmpStr);
        tmpStr[tmpInt] = '\0';
        if (tmpInt < 2) {
            tmpInt = 0;
        }
    } else {
        tmpStr[0] = '\0';
    }
    fprintf(stdout, "  - LDIR_Name3       : %s\n", tmpStr);
    tmpInt = longNameStrCpy(fat32LongDir, 0, tmpStr);
    fprintf(stdout, "  - LDIR_Full_Name   : %s - %d\n", tmpStr, tmpInt);
}

void fat::dirAttrToStr(uint8_t dirAttr, char *dirAttrStr) {
    if ((dirAttr | FAT_DIR_ATTR_READ_ONLY) == dirAttr) {
        if (strlen(dirAttrStr) > 0) {
            strcat(dirAttrStr, " | ");
        }
        strcat(dirAttrStr, "READ_ONLY");
    }
    if ((dirAttr | FAT_DIR_ATTR_HIDDEN) == dirAttr) {
        if (strlen(dirAttrStr) > 0) {
            strcat(dirAttrStr, " | ");
        }
        strcat(dirAttrStr, "HIDDEN");
    }
    if ((dirAttr | FAT_DIR_ATTR_SYSTEM) == dirAttr) {
        if (strlen(dirAttrStr) > 0) {
            strcat(dirAttrStr, " | ");
        }
        strcat(dirAttrStr, "SYSTEM");
    }
    if ((dirAttr | FAT_DIR_ATTR_VOLUME_ID) == dirAttr) {
        if (strlen(dirAttrStr) > 0) {
            strcat(dirAttrStr, " | ");
        }
        strcat(dirAttrStr, "VOLUME_ID");
    }
    if ((dirAttr | FAT_DIR_ATTR_DIRECTORY) == dirAttr) {
        if (strlen(dirAttrStr) > 0) {
            strcat(dirAttrStr, " | ");
        }
        strcat(dirAttrStr, "DIRECTORY");
    }
    if ((dirAttr | FAT_DIR_ATTR_ARCHIVE) == dirAttr) {
        if (strlen(dirAttrStr) > 0) {
            strcat(dirAttrStr, " | ");
        }
        strcat(dirAttrStr, "ARCHIVE");
    }
    if ((dirAttr | FAT_DIR_ATTR_LONG_NAME) == dirAttr) {
        if (strlen(dirAttrStr) > 0) {
            strcat(dirAttrStr, " | ");
        }
        strcat(dirAttrStr, "LONG_NAME");
    }

    if (strlen(dirAttrStr) == 0) {
        strcat(dirAttrStr, "UNDEFINED");
    }
}

int fat::longNameStrCpy(unsigned char *longName, int lsize, int outOffset, char *fullName) {
    int i;
    int count = 0;
    fullName += outOffset;
    for (i=0; i<lsize; i++) {
        if (i == 0 && *longName == 0) {
            *fullName++ = 0;
            break;
        } else if (i % 2 == 0) { // Ignore long name dot chars
            *fullName++ = (char) *longName;
            if (*longName == 0) {
                break;
            } else {
                count++;
            }
        }
        longName++;
    }
    return count;
}

int fat::longNameStrCpy(Fat32LongDirectory_t fat32LongDir, int outOffset, char *fullName) {
    int tmpInt = longNameStrCpy(fat32LongDir.LDIR_Name1, 10, outOffset, fullName);
    if (tmpInt >= 5) {
        tmpInt += longNameStrCpy(fat32LongDir.LDIR_Name2, 12, outOffset + tmpInt, fullName);
    }
    if (tmpInt >= 11) {
        tmpInt += longNameStrCpy(fat32LongDir.LDIR_Name3, 4, outOffset + tmpInt, fullName);
    }
    fullName[outOffset + tmpInt] = '\0';

    return outOffset + tmpInt;
}

// ==========================================================================================
// ================================== ARCHITECTURE FAT 32 ===================================
// ==========================================================================================


int fat::getRootDir(FILE *storage, Fat32RootDir_t* rootDirEntry) {
    uint32_t firstSectorOfCluster;
    int result = FAT_NO_ERROR;

    if (!(result = readBaseFatInfo(storage, &rootDirEntry->bsHeader, &rootDirEntry->bs32, &rootDirEntry->fsInfo32))) {
        // Read succeeded
        firstSectorOfCluster = FIRST_SEC_OF_CLUSTER(rootDirEntry->bs32);

        firstSectorOfCluster = 326780;

        // Generate a struct of the root dir to facilitate the navigation
        rootDirEntry->dirEntry.DIR_Name[0] = '/';
        rootDirEntry->dirEntry.DIR_Attr = FAT_DIR_ATTR_DIRECTORY;
        rootDirEntry->dirEntry.DIR_NTRes = 0;
        rootDirEntry->dirEntry.DIR_CrtTimeTenth = 0;
        rootDirEntry->dirEntry.DIR_CrtTime = 0;
        rootDirEntry->dirEntry.DIR_CrtDate = 0;
        rootDirEntry->dirEntry.DIR_LstAccDate = 0;
        rootDirEntry->dirEntry.DIR_FstClusHI = 0;
        rootDirEntry->dirEntry.DIR_WrtTime = 0;
        rootDirEntry->dirEntry.DIR_WrtDate = 0;
        rootDirEntry->dirEntry.DIR_FileSize = 0;
        rootDirEntry->dirEntry.DIR_FstClusHI = (firstSectorOfCluster & 0xFFFF0000) >> 16; // Get high 16 bits of the sector where root dir begins
        rootDirEntry->dirEntry.DIR_FstClusLO = firstSectorOfCluster & 0xFFFF;             // Get low 16 bits of the sector where root dir begins

        fprintf(stdout, "DIR_FstClusHI: %d\n", rootDirEntry->dirEntry.DIR_FstClusHI);
        fprintf(stdout, "DIR_FstClusLO: %d\n", rootDirEntry->dirEntry.DIR_FstClusLO);
    }

    return FAT_ERROR_READ_BS_HEADER;
}

int fat::readBaseFatInfo(FILE *storage, FatBSHeader_t *bsHeader, FatBS32_t* bs32, Fat32FsInfo_t* fsInfo32) {
    // Read the header
    if (fread(bsHeader, 1, sizeof(FatBSHeader_t), storage) != sizeof(FatBSHeader_t)) {
        return FAT_ERROR_READ_BS_HEADER;
    }

    bs32->header = *bsHeader;

    // Determine FAT type 12/16 or 32 bits
    if (bsHeader->BPB_FATSz16 > 0) {
        // Fat 12/16 unsuported
        return FAT_ERROR_UNSUPORTED_FORMAT;
    } else {
        // Fat 32 supported
        // Set offset to end of header and continue reading bs32 struct
        if (fread((void*) (((size_t) bs32) + sizeof(FatBSHeader_t)), 1, sizeof(FatBS32_t) - sizeof(FatBSHeader_t), storage) != sizeof(FatBS32_t) - sizeof(FatBSHeader_t)) {
            return FAT_ERROR_READ_BS32;
        }

        // Set file offset at the beginning of the BPB_FSInfo sector
        fseek(storage, bs32->BPB_FSInfo * bs32->header.BPB_BytesPerSec, SEEK_SET);

        // Read FSInfo struct
        if (fread(fsInfo32, 1, sizeof(Fat32FsInfo_t), storage) != sizeof(Fat32FsInfo_t)) {
            return FAT_ERROR_READ_FSINFO;
        }

        // Validate FSInfo
        if (fsInfo32->FSI_LeadSig != FAT_FSINFO_LEAD_SIGNATURE) {
            // Invalid lead signature
            return FAT_ERROR_READ_FSINFO_INVALID_LEAD_SIGNATURE;
        }
        if (fsInfo32->FSI_StrucSig != FAT_FSINFO_STRUCT_SIGNATURE) {
            // Invalid struct signature
            return FAT_ERROR_READ_FSINFO_INVALID_STRUCT_SIGNATURE;
        }
    }

    // ------------------- PRINT INFORMATIONS -------------------
    // fprintf(stdout, "BS - HEADER:\n");
    // fprintf(stdout, "  - BS_JmpBoot      : 0x%02X 0x%02X 0x%02X\n", bs32.header.BS_JmpBoot[0], bs32.header.BS_JmpBoot[1], bs32.header.BS_JmpBoot[2]);
    // fprintf(stdout, "  - BS_OEMName      : %.*s\n", 8, bs32.header.BS_OEMName);
    // fprintf(stdout, "  - BPB_BytesPerSec : %d\n", bs32.header.BPB_BytesPerSec);
    // fprintf(stdout, "  - BPB_SecPerClus  : %d\n", bs32.header.BPB_SecPerClus);
    // fprintf(stdout, "  - BPB_RsvdSecCnt  : %d\n", bs32.header.BPB_RsvdSecCnt);
    // fprintf(stdout, "  - BPB_NumFATs     : %d\n", bs32.header.BPB_NumFATs);
    // fprintf(stdout, "  - BPB_RootEntCnt  : %d\n", bs32.header.BPB_RootEntCnt);
    // fprintf(stdout, "  - BPB_TotSec16    : %d\n", bs32.header.BPB_TotSec16);
    // fprintf(stdout, "  - BPB_Media       : %d\n", bs32.header.BPB_Media);
    // fprintf(stdout, "  - BPB_FATSz16     : %d\n", bs32.header.BPB_FATSz16);
    // fprintf(stdout, "  - BPB_SecPerTrk   : %d\n", bs32.header.BPB_SecPerTrk);
    // fprintf(stdout, "  - BPB_NumHeads    : %d\n", bs32.header.BPB_NumHeads);
    // fprintf(stdout, "  - BPB_HiddSec     : %d\n", bs32.header.BPB_HiddSec);
    // fprintf(stdout, "  - BPB_TotSec32    : %d\n", bs32.header.BPB_TotSec32);

    // fprintf(stdout, "\nBS - BS32:\n");
    // fprintf(stdout, "  - BPB_FATSz32   : %d\n", bs32.BPB_FATSz32);
    // fprintf(stdout, "  - BPB_ExtFlags  : 0x%02X\n", bs32.BPB_ExtFlags);
    // fprintf(stdout, "  - BPB_FSVer     : 0x%02X\n", bs32.BPB_FSVer);
    // fprintf(stdout, "  - BPB_RootClus  : %d\n", bs32.BPB_RootClus);
    // fprintf(stdout, "  - BPB_FSInfo    : %d\n", bs32.BPB_FSInfo);
    // fprintf(stdout, "  - BPB_BkBootSec : %d\n", bs32.BPB_BkBootSec);
    // fprintf(stdout, "  - BPB_Reserved  : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
    //    bs32.BPB_Reserved[0], bs32.BPB_Reserved[1], bs32.BPB_Reserved[2], bs32.BPB_Reserved[3],
    //    bs32.BPB_Reserved[4], bs32.BPB_Reserved[5], bs32.BPB_Reserved[6], bs32.BPB_Reserved[7],
    //    bs32.BPB_Reserved[8], bs32.BPB_Reserved[9], bs32.BPB_Reserved[10], bs32.BPB_Reserved[11]
    // );
    // fprintf(stdout, "  - BS_DrvNum     : %d\n", bs32.BS_DrvNum);
    // fprintf(stdout, "  - BS_Reserved1  : %d\n", bs32.BS_Reserved1);
    // fprintf(stdout, "  - BS_BootSig    : 0x%02X\n", bs32.BS_BootSig);
    // fprintf(stdout, "  - BS_VolID      : 0x%04X\n", bs32.BS_VolID);
    // fprintf(stdout, "  - BS_VolLab     : %.*s\n", 11, bs32.BS_VolLab);
    // fprintf(stdout, "  - BS_FilSysType : %.*s\n", 8, bs32.BS_FilSysType);
    // fprintf(stdout, "  - BS_BootCode32 : ***\n");
    // fprintf(stdout, "  - BS_BootSign   : 0x%04X\n", bs32.BS_BootSign);

    // fprintf(stdout, "\nFSINFO - 32:\n");
    // fprintf(stdout, "  - FSI_LeadSig    : 0x%08X\n", fsInfo32.FSI_LeadSig);
    // fprintf(stdout, "  - FSI_Reserved1  : ***\n");
    // fprintf(stdout, "  - FSI_StrucSig   : 0x%08X\n", fsInfo32.FSI_StrucSig);
    // fprintf(stdout, "  - FSI_Free_Count : %d\n", fsInfo32.FSI_Free_Count);
    // fprintf(stdout, "  - FSI_Nxt_Free   : %d\n", fsInfo32.FSI_Nxt_Free);
    // fprintf(stdout, "  - FSI_Reserved2  : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n", 
    //     fsInfo32.FSI_Reserved2[0], fsInfo32.FSI_Reserved2[1], fsInfo32.FSI_Reserved2[2], fsInfo32.FSI_Reserved2[3], 
    //     fsInfo32.FSI_Reserved2[4], fsInfo32.FSI_Reserved2[5], fsInfo32.FSI_Reserved2[6], fsInfo32.FSI_Reserved2[7], 
    //     fsInfo32.FSI_Reserved2[8],fsInfo32.FSI_Reserved2[9], fsInfo32.FSI_Reserved2[10], fsInfo32.FSI_Reserved2[11]
    // );
    // fprintf(stdout, "  - FSI_TrailSig   : 0x%08X\n", fsInfo32.FSI_TrailSig);

    return FAT_NO_ERROR;
}
