// stdio
#include <string.h>
#include <stdio.h>

#include "fat.h"

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
    
    // Read the header
    if (fread(&bsHeader, sizeof(bsHeader), 1, storage) == 0) {
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
        if (fread((void*) (((size_t) &bs32) + sizeof(bsHeader)), sizeof(bs32) - sizeof(bsHeader), 1, storage) == 0) {
            return FAT_ERROR_READ_BS32;
        }
    }
    
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
    fprintf(stdout, "  - BS_BootSig    : %d\n", bs32.BS_BootSig);
    fprintf(stdout, "  - BS_VolID      : 0x%04X\n", bs32.BS_VolID);
    fprintf(stdout, "  - BS_VolLab     : %.*s\n", 11, bs32.BS_VolLab);
    fprintf(stdout, "  - BS_FilSysType : %.*s\n", 8, bs32.BS_FilSysType);
    fprintf(stdout, "  - BS_BootCode32 : ***\n");
    fprintf(stdout, "  - BS_BootSign   : 0x%04X\n", bs32.BS_BootSign);

    return FAT_NO_ERROR;
}