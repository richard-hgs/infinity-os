// libc
#include <stdint.h>
// stdio
#include "string.h"
#include "fat.h"

#define FAT_MEDIA_TYPE_FIXED        0xF8
#define FAT_MEDIA_TYPE_REMOVABLE    0xF0



typedef struct Fat32BootSector {
    unsigned char BS_JmpBoot[3];
    unsigned char BS_OEMName[8];
    uint16_t BPB_BytesPerSec;
    uint8_t BPB_SecPerClus;
    uint16_t BPB_RsvdSecCnt;
    uint8_t BPB_NumFATs;
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;
    uint32_t BPB_FATSz32;
    uint16_t BPB_ExtFlags;
    uint16_t BPB_FSVer;
    uint32_t BPB_RootClus;
    uint16_t BPB_FSInfo;
    uint16_t BPB_BkBootSec;
    unsigned char BPB_Reserved[12];
    uint8_t BS_DrvNum;
    uint8_t BS_Reserved1;
    uint8_t BS_BootSig;
    uint32_t BS_VolID;
    unsigned char BS_VolLab[11];
    unsigned char BS_FilSysType[8];
    unsigned char BS_BootCode32[420];
    uint16_t BS_BootSign;
} __attribute__((packed)) Fat32BootSector_t;

typedef struct Fat32FsInfo {
    uint32_t FSI_LeadSig;
    unsigned char FSI_Reserved1[480];
    uint32_t FSI_StrucSig;
    uint32_t FSI_Free_Count;
    uint32_t FSI_Nxt_Free;
    unsigned char FSI_Reserved2[12];
    uint32_t FSI_TrailSig;
} __attribute__((packed)) Fat32FsInfo_t;

typedef struct Fat32Directory {
    unsigned char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t DIR_NTRes;
    uint8_t DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI;
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;
} __attribute__((packed)) Fat32Directory_t;

// Structure that holds the fat type definitions
struct DskSzToSecPerClus {
    unsigned int DiskSize; // BPB_TotSec32
    uint8_t SecPerClusVal; // BPB_SecPerClus
};

/** 
 * @brief This is the table for FAT32 drives. NOTE that this table includes
 * entries for disk sizes smaller than 512 MB even though typically
 * only the entries for disks >= 512 MB in size are used.
 * The way this table is accessed is to look for the first entry
 * in the table for which the disk size is less than or equal
 * to the DiskSize field in that table entry. For this table to
 * work properly BPB_RsvdSecCnt must be 32, and BPB_NumFATs
 * must be 2. Any of these values being different may require the first 
 * table entries DiskSize value to be changed otherwise the cluster count 
 * may be to low for FAT32.
 */
DskSzToSecPerClus DskTableFAT32[] = {
    { 66600,         0},  /* disks up to 32.5 MB, the 0 value for SecPerClusVal trips an error */
    { 532480,        1},  /* disks up to 260 MB, .5k cluster */
    { 16777216,      8},  /* disks up to 8 GB, 4k cluster */
    { 33554432,     16},  /* disks up to 16 GB, 8k cluster */
    { 67108864,     32},  /* disks up to 32 GB, 16k cluster */
    { 0xFFFFFFFF,   64}   /* disks greater than 32GB, 32k cluster */
};

void fat::create() {
    Fat32BootSector_t fatBootSect;
    
    string::strcpy((char*) fatBootSect.BS_OEMName, "MSDOS5.0");

}