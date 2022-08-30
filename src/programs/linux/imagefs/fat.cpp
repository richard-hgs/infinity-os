// stdio
#include <string.h>
#include <stdio.h>

#include "fat.h"

/**
 * @brief Check if a fat cluster is FREE(unused)
 */
#define CLUSTER_IS_FREE(val) val == 0x00000000

/**
 * @brief Check if fat cluster is the EOC end of cluster chain
 */
#define CLUSTER_IS_EOC(val) (val >= 0x0FFFFFF8 && val <= 0x0FFFFFFF)

/**
 * @brief Check if fat cluster is bad and should not be used
 */
#define CLUSTER_IS_BAD(val) val == 0x0FFFFFF7

/**
 * @brief Check if dir (Fat32Directory) is free
 */
#define IS_DIR_FREE(dir) (dir.DIR_Name[0] == FAT_LDIR_UNUSED || dir.DIR_Name[0] == FAT_LDIR_LAST_AND_UNUSED)

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

int fat::listEntries(FILE *storage) {
    FatBS32_t bs32;
    FatBSHeader_t bsHeader;
    Fat32FsInfo_t fsInfo32;
    Fat32Directory_t dirEntry32;
    Fat32LongDirectory_t lngDirEntry32;
    Fat32LongDirectory_t lngDirEntries32[19];
    int i;
    uint32_t tmpInt;
    uint32_t tmpInt2;
    uint8_t mChecksum;
    char tmpStr[256];
    
    // Read the header
    if (fread(&bsHeader, 1, sizeof(bsHeader), storage) != sizeof(bsHeader)) {
        return FAT_ERROR_READ_BS_HEADER;
    }

    bs32.header = bsHeader;

    // Determine FAT type 12/16 or 32 bits
    if (bsHeader.BPB_FATSz16 > 0) {
        // Fat 12/16 unsuported
        return FAT_ERROR_UNSUPORTED_FORMAT;
    } else {
        // Fat 32 supported
        // Set offset to end of header and continue reading bs32 struct
        if (fread((void*) (((size_t) &bs32) + sizeof(bsHeader)), 1, sizeof(bs32) - sizeof(bsHeader), storage) != sizeof(bs32) - sizeof(bsHeader)) {
            return FAT_ERROR_READ_BS32;
        }

        // Set file offset at the beginning of the BPB_FSInfo sector
        fseek(storage, bs32.BPB_FSInfo * bs32.header.BPB_BytesPerSec, SEEK_SET);

        // Read FSInfo struct
        if (fread(&fsInfo32, 1, sizeof(fsInfo32), storage) != sizeof(fsInfo32)) {
            return FAT_ERROR_READ_FSINFO;
        }

        // Validate FSInfo
        if (fsInfo32.FSI_LeadSig != FAT_FSINFO_LEAD_SIGNATURE) {
            // Invalid lead signature
            return FAT_ERROR_READ_FSINFO_INVALID_LEAD_SIGNATURE;
        }
        if (fsInfo32.FSI_StrucSig != FAT_FSINFO_STRUCT_SIGNATURE) {
            // Invalid struct signature
            return FAT_ERROR_READ_FSINFO_INVALID_STRUCT_SIGNATURE;
        }

        // Amount of sectors used by the fat entries
        uint32_t fatSizeSectors = bs32.header.BPB_NumFATs * bs32.BPB_FATSz32;
        // Amount of sectors used by the root dir
        uint32_t rootDirSectors = ((bs32.header.BPB_RootEntCnt * 32) + (bs32.header.BPB_BytesPerSec - 1)) / bs32.header.BPB_BytesPerSec;
        // Sector where the data sectors starts
        uint32_t firstDataSector = bs32.header.BPB_RsvdSecCnt + fatSizeSectors + rootDirSectors;
        // Root directory sector location in data sectors
        uint32_t firstSectorOfCluster = ((bs32.BPB_RootClus - 2) * bs32.header.BPB_SecPerClus) + firstDataSector;

        mChecksum = 0;
        tmpInt = 0;
        for (i=0; i<30; i++) {
            // Offset relative to FAT storage
            uint32_t offset = (i * 32) + (firstSectorOfCluster * bs32.header.BPB_BytesPerSec);

            // Set file offset at the beginning of the Root dir sectors
            fseek(storage, offset, SEEK_SET);

            // Read the first root directory entry
            readNextDirEntry(storage, &dirEntry32);

            switch(dirEntry32.DIR_Attr) {
                case FAT_DIR_ATTR_LONG_NAME:
                    lngDirEntry32 = *(Fat32LongDirectory_t*) &dirEntry32;
                    if (lngDirEntry32.LDIR_Ord != FAT_LDIR_LAST_AND_UNUSED && lngDirEntry32.LDIR_Ord != FAT_LDIR_UNUSED) {
                        // If file is active and wasn't deleted
                        if (mChecksum != lngDirEntry32.LDIR_Chksum) { // It is a new long name reset tmpStr that will holds the Full name of the next directory entry.
                            fprintf(stdout, "  - checksum: 0x%02X - 0x%02X\n", mChecksum, lngDirEntry32.LDIR_Chksum);
                            tmpInt = 0;
                            mChecksum = lngDirEntry32.LDIR_Chksum;
                        }
                        lngDirEntries32[tmpInt++] = lngDirEntry32;
                    }

                    // printLongDirEntry(lngDirEntry32);
                break;
                default:
                    // Print dir entry
                    // printDirEntry(dirEntry32);
                    
                    // If it is an active dir entry that wasn't deleted
                    if (!IS_DIR_FREE(dirEntry32)) {
                        // Check if long name checksum matches with dir short name
                        if (mChecksum == checksum(dirEntry32.DIR_Name)) {
                            // Read long name entries in desc order
                            tmpInt2 = 0;
                            while(tmpInt--) {
                                lngDirEntry32 = lngDirEntries32[tmpInt];
                                tmpInt2 = longNameStrCpy(lngDirEntry32, tmpInt2, tmpStr);
                            }
                            fprintf(stdout, "  - DIR_FullName: %.*s\n", tmpInt, tmpStr);
                        } else {
                            fprintf(stdout, "  - DIR_ShortName: %.*s\n", 11, dirEntry32.DIR_Name);
                        }
                    }
                    tmpInt = 0;
                    mChecksum = 0;
                break;
            }
            


            // fatVal(storage, bs32, 2, &tmpInt);
            // fprintf(stdout, "FAT value: 0x%08X\n\n", tmpInt);
        }

        // tmpStr[tmpInt++] = '\0';
        // fprintf(stdout, "FullName: %s\n", tmpStr);
    }


    // ------------------- PRINT INFORMATIONS -------------------
    fprintf(stdout, "BS - HEADER:\n");
    fprintf(stdout, "  - BS_JmpBoot      : 0x%02X 0x%02X 0x%02X\n", bs32.header.BS_JmpBoot[0], bs32.header.BS_JmpBoot[1], bs32.header.BS_JmpBoot[2]);
    fprintf(stdout, "  - BS_OEMName      : %.*s\n", 8, bs32.header.BS_OEMName);
    fprintf(stdout, "  - BPB_BytesPerSec : %d\n", bs32.header.BPB_BytesPerSec);
    fprintf(stdout, "  - BPB_SecPerClus  : %d\n", bs32.header.BPB_SecPerClus);
    fprintf(stdout, "  - BPB_RsvdSecCnt  : %d\n", bs32.header.BPB_RsvdSecCnt);
    fprintf(stdout, "  - BPB_NumFATs     : %d\n", bs32.header.BPB_NumFATs);
    fprintf(stdout, "  - BPB_RootEntCnt  : %d\n", bs32.header.BPB_RootEntCnt);
    fprintf(stdout, "  - BPB_TotSec16    : %d\n", bs32.header.BPB_TotSec16);
    fprintf(stdout, "  - BPB_Media       : %d\n", bs32.header.BPB_Media);
    fprintf(stdout, "  - BPB_FATSz16     : %d\n", bs32.header.BPB_FATSz16);
    fprintf(stdout, "  - BPB_SecPerTrk   : %d\n", bs32.header.BPB_SecPerTrk);
    fprintf(stdout, "  - BPB_NumHeads    : %d\n", bs32.header.BPB_NumHeads);
    fprintf(stdout, "  - BPB_HiddSec     : %d\n", bs32.header.BPB_HiddSec);
    fprintf(stdout, "  - BPB_TotSec32    : %d\n", bs32.header.BPB_TotSec32);

    fprintf(stdout, "\nBS - BS32:\n");
    fprintf(stdout, "  - BPB_FATSz32   : %d\n", bs32.BPB_FATSz32);
    fprintf(stdout, "  - BPB_ExtFlags  : 0x%02X\n", bs32.BPB_ExtFlags);
    fprintf(stdout, "  - BPB_FSVer     : 0x%02X\n", bs32.BPB_FSVer);
    fprintf(stdout, "  - BPB_RootClus  : %d\n", bs32.BPB_RootClus);
    fprintf(stdout, "  - BPB_FSInfo    : %d\n", bs32.BPB_FSInfo);
    fprintf(stdout, "  - BPB_BkBootSec : %d\n", bs32.BPB_BkBootSec);
    fprintf(stdout, "  - BPB_Reserved  : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
       bs32.BPB_Reserved[0], bs32.BPB_Reserved[1], bs32.BPB_Reserved[2], bs32.BPB_Reserved[3],
       bs32.BPB_Reserved[4], bs32.BPB_Reserved[5], bs32.BPB_Reserved[6], bs32.BPB_Reserved[7],
       bs32.BPB_Reserved[8], bs32.BPB_Reserved[9], bs32.BPB_Reserved[10], bs32.BPB_Reserved[11]
    );
    fprintf(stdout, "  - BS_DrvNum     : %d\n", bs32.BS_DrvNum);
    fprintf(stdout, "  - BS_Reserved1  : %d\n", bs32.BS_Reserved1);
    fprintf(stdout, "  - BS_BootSig    : 0x%02X\n", bs32.BS_BootSig);
    fprintf(stdout, "  - BS_VolID      : 0x%04X\n", bs32.BS_VolID);
    fprintf(stdout, "  - BS_VolLab     : %.*s\n", 11, bs32.BS_VolLab);
    fprintf(stdout, "  - BS_FilSysType : %.*s\n", 8, bs32.BS_FilSysType);
    fprintf(stdout, "  - BS_BootCode32 : ***\n");
    fprintf(stdout, "  - BS_BootSign   : 0x%04X\n", bs32.BS_BootSign);

    fprintf(stdout, "\nFSINFO - 32:\n");
    fprintf(stdout, "  - FSI_LeadSig    : 0x%08X\n", fsInfo32.FSI_LeadSig);
    fprintf(stdout, "  - FSI_Reserved1  : ***\n");
    fprintf(stdout, "  - FSI_StrucSig   : 0x%08X\n", fsInfo32.FSI_StrucSig);
    fprintf(stdout, "  - FSI_Free_Count : %d\n", fsInfo32.FSI_Free_Count);
    fprintf(stdout, "  - FSI_Nxt_Free   : %d\n", fsInfo32.FSI_Nxt_Free);
    fprintf(stdout, "  - FSI_Reserved2  : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n", 
        fsInfo32.FSI_Reserved2[0], fsInfo32.FSI_Reserved2[1], fsInfo32.FSI_Reserved2[2], fsInfo32.FSI_Reserved2[3], 
        fsInfo32.FSI_Reserved2[4], fsInfo32.FSI_Reserved2[5], fsInfo32.FSI_Reserved2[6], fsInfo32.FSI_Reserved2[7], 
        fsInfo32.FSI_Reserved2[8],fsInfo32.FSI_Reserved2[9], fsInfo32.FSI_Reserved2[10], fsInfo32.FSI_Reserved2[11]
    );
    fprintf(stdout, "  - FSI_TrailSig   : 0x%08X\n", fsInfo32.FSI_TrailSig);

    /*
        uint32_t FSI_LeadSig;               // 0x41615252. This is a lead signature used to validate that this is in fact an FSInfo sector.
        unsigned char FSI_Reserved1[480];   // Reserved. This field should be always initialized to zero.
        uint32_t FSI_StrucSig;              // 0x61417272. Another signature that is more localized in the sector to the location of the fields that are used.
        uint32_t FSI_Free_Count;            // This field indicates the last known free cluster count on the volume. If the value is 0xFFFFFFFF, it is actually unknown. This is not necessarily correct, so that the FAT driver needs to make sure it is valid for the volume.
        uint32_t FSI_Nxt_Free;              // This field gives a hint for the FAT driver, the cluster number at which the driver should start looking for free clusters. Because a FAT32 FAT is large, it can be rather time consuming if there are a lot of allocated clusters at the start of the FAT and the driver starts looking for a free cluster starting at the first cluster. Typically this value is set to the last cluster number that the driver allocated. If the value is 0xFFFFFFFF, there is no hint and the driver should start looking at cluster 2. This may not be correct, so that the FAT driver needs to make sure it is valid for the volume.
        unsigned char FSI_Reserved2[12];    // Reserved. This field should be always initialized to zero.
        uint32_t FSI_TrailSig;              // 0xAA550000. This trail signature is used to validate that this is in fact an FSInfo sector.
    */

    return FAT_NO_ERROR;
}

int fat::readNextDirEntry(FILE *storage, Fat32Directory_t *fat32Dir) {
    // printf("file seek: 0x%02X\n", ftell(storage));
    if (fread(fat32Dir, 1, sizeof(Fat32Directory_t), storage) != sizeof(Fat32Directory_t)) {
        return FAT_ERROR_READ_DIRECTORY;
    }

    return FAT_NO_ERROR;
}

int fat::fatVal(FILE *storage, FatBS32_t bs32, uint32_t clusterNum, uint32_t *fatValue) {
    uint32_t firstSectorOfFatEntries = bs32.header.BPB_RsvdSecCnt; // FAT 32 has 2 reserved fat entries

    // Set file offset at the beginning of the fat entry being readed
    fseek(storage, firstSectorOfFatEntries * bs32.header.BPB_BytesPerSec + (clusterNum * 4), SEEK_SET);

    // Read
    if (fread(fatValue, 1, 4, storage)) {
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