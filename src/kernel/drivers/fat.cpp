// libc
#include <stdint.h>
// stdio
#include "string.h"
#include "stdio.h"    // DEBUG
#include "memutils.h" // DEBUG
#include "fat.h"

#define FAT_MEDIA_TYPE_FIXED        0xF8
#define FAT_MEDIA_TYPE_REMOVABLE    0xF0

/**
 * @brief Windows FAT Format Map Overview Disassembly
 * 
 * SECTORS 1-3: 
 *      WINDOWS USES 3 SECTORS FOR BOOT, SO WE MUST CONFIGURE THE 3 FIRST SECTORS AS BOOTABLE FINISHING WITH 0xAA55
 * 
 * 
 * SECTOR 1: 
 *      BS_JmpBoot: EB 58 90
 *      BS_OEMName: "MSDOS5.0"
 *      BPB_BytesPerSec: 0x200
 *      BPB_SecPerClus: 0x8
 *      BPB_RsvdSecCnt: 0xBCA    STARTS IN SECTOR 0
 *      BPB_NumFATs: 0x02
 *      BPB_RootEntCnt: 0x0
 *      BPB_TotSec16: 0x0
 *      BPB_Media: 0xF8
 *      BPB_FATSz16: 0x0
 *      BPB_SecPerTrk: 0x3F
 *      BPB_NumHeads: 0xFF
 *      BPB_HiddSec: 0x800
 *      BPB_TotSec32: 0xE8E800
 *      BPB_FATSz32: 0x3A1B
 *      BPB_ExtFlags: 0x0
 *      BPB_FSVer: 0x0
 *      BPB_RootClus: 0x2
 *      BPB_FSInfo: 0x1
 *      BPB_BkBootSec: 0x6
 *      BPB_Reserved: 0x0
 *      BS_DrvNum: 0x8
 *      BS_Reserved1: 0x0
 *      BS_BootSig: 0x29
 *      BS_VolID: 0x45B38192   Serial Number of Volume
 *      BS_VolLab: "NO NAME    "
 *      BS_FilSysType: "FAT32   "
 *      BS_BootCode32: ***
 *      BS_BootSign: 0X55AA
 * 
 * SECTOR 2 (BPB_FSInfo): 
 *      FSI_LeadSig: 0x41615252
 *      FSI_Reserved1: 0x0
 *      FSI_StrucSig: 0x61417272
 *      FSI_Free_Count: 0x1D0CFC * BPB_SecPerClus * BPB_BytesPerSec = FREE_SIZE_IN_BYTES
 *      FSI_Nxt_Free: 0x6
 *      FSI_Reserved2: 0x0
 *      FSI_TrailSig: 0x0000AA55
 * 
 * SECTOR 6-8:
 *      BOOT SECTOR BACKUP
 * 
 * SECTOR 12:
 *      ANOTHER BOOT SECTOR USED FOR SOME UNKNOWN REASON
 *     
 * SECTOR 32768 = (BPB_RsvdSecCnt + (BPB_NumFATs * BPB_FATSz32)):
 *      ROOT Fat32Directory entry sector
 */

/**
 * @brief FAT - Boot Sector
 *  - Common structure to all types of FAT(FAT12, FAT16 and FAT32)
 */
typedef struct FatBSHeader {
    unsigned char BS_JmpBoot[3];        // Jump instruction to the bootstrap code (x86 instruction) used by OS boot sequence
    unsigned char BS_OEMName[8];        // "MSWIN 4.1" is recommended but also "MSDOS 5.0" is often used. OS does not pay any attention to this field, but some FAT drivers do some reference.
    uint16_t BPB_BytesPerSec;           // Sector size in unit of byte. Valid values for this field are 512, 1024, 2048 or 4096. Many FAT drivers assume the sector size is 512 and do not check this field. For this reason, 512 should be used for maximum compatibility.
    uint8_t BPB_SecPerClus;             // Number of sectors per allocation unit. In the FAT file system, the allocation unit is called Cluster. This is a block of one or more consecutive sectors and the data area is managed in this unit. The number of sectors per cluster must be a power of 2. Therefore, valid values are 1, 2, 4,... and 128. However, any value whose cluster size (BPB_BytsPerSec * BPB_SecPerClus) exceeds 32 KB should not be used.
    uint16_t BPB_RsvdSecCnt;            // Number of sectors in reserved area. This field must not be 0 because there is the boot sector itself contains this BPB in the reserved area. To avoid compatibility problems, it should be 1 on FAT12/16 volume.
    uint8_t BPB_NumFATs;                // Number of FATs. The value of this field should always be 2. Also any value equal to or greater than 1 is valid but it is strongly recommended not to use values other than 2 to avoid compatibility problem.
    uint16_t BPB_RootEntCnt;            // On the FAT12/16 volumes, this field indicates number of 32-byte directory entries in the root directory. The value should be set a value that the size of root directory is aligned to the 2-sector boundary, BPB_RootEntCnt * 32 becomes even multiple of BPB_BytsPerSec. For maximum compatibility, this field should be set to 512 on the FAT16 volume. For FAT32 volumes, this field must be 0.
    uint16_t BPB_TotSec16;              // Total number of sectors of the volume in old 16-bit field. This value is the number of sectors including all four areas of the volume. When the number of sectors of the FAT12/16 volumes is 0x10000 or larger, an invalid value 0 is set in this field, and the true value is set to BPB_TotSec32. For FAT32 volumes, this field must always be 0.
    uint8_t BPB_Media;                  // The valid values for this field is 0xF0, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE and 0xFF. 0xF8 is the standard value for non-removable disks and 0xF0 is often used for non partitioned removable disks. Other important point is that the same value must be put in the lower 8-bits of FAT[0]. This comes from the media determination of MS-DOS Ver.1 and not used for any purpose any longer.
    uint16_t BPB_FATSz16;               // Number of sectors occupied by a FAT. This field is used for only FAT12/16 volumes. On the FAT32 volumes, it must be an invalid value 0 and BPB_FATSz32 is used instead. The size of the FAT area becomes BPB_FATSz?? * BPB_NumFATs sectors.
    uint16_t BPB_SecPerTrk;             // Number of sectors per track. This field is relevant only for media that have geometry and used for only disk BIOS of IBM PC.
    uint16_t BPB_NumHeads;              // Number of heads. This field is relevant only for media that have geometry and used for only disk BIOS of IBM PC.
    uint32_t BPB_HiddSec;               // Number of hidden physical sectors preceding the FAT volume. It is generally related to storage accessed by disk BIOS of IBM PC, and what kind of value is set is platform dependent. This field should always be 0 if the volume starts at the beginning of the storage, e.g. non-partitioned disks, such as floppy disk.
    uint32_t BPB_TotSec32;              // Total number of sectors of the FAT volume in new 32-bit field. This value is the number of sectors including all four areas of the volume. When the value on the FAT12/16 volume is less than 0x10000, this field must be invalid value 0 and the true value is set to BPB_TotSec16. On the FAT32 volume, this field is always valid and old field is not used.
} __attribute__((packed)) FatBSHeader_t;


typedef struct FatBS1216 {
    FatBSHeader header;
    uint8_t BS_DrvNum;                  // Drive number used by disk BIOS of IBM PC. This field is used in MS-DOS bootstrap, 0x00 for floppy disk and 0x80 for fixed disk. Actually it depends on the OS.
    uint8_t BS_Reserved;                // Reserved (used by Windows NT). It should be set 0 when create the volume.
    uint8_t BS_BootSig;                 // Extended boot signature (0x29). This is a signature byte indicates that the following three fields are present.
    uint32_t BS_VolID;                  // Volume serial number used with BS_VolLab to track a volume on the removable storage. It enables to detect a wrong media change by FAT driver. This value is typically generated with current time and date on formatting.
    unsigned char BS_VolLab[11];        // This field is the 11-byte volume label and it matches volume label recorded in the root directory. FAT driver should update this field when the volume label in the root directory is changed. MS-DOS does it but Windows does not do it. When volume label is not present, "NO NAME " should be set in this field.
    unsigned char BS_FilSysType[8];     // "FAT12   ", "FAT16   " or "FAT     ". Many people think that this string has any effect in determination of the FAT type but it is clearly a misrecognization. From the name of this field, you will find that this is not a part of BPB. Since this string is often incorrect or not set, Microsoft's FAT driver does not use this field to determine the FAT type. However, some old FAT drivers use this string to determine the FAT type, so that it should be set based on the FAT type of the volume to avoid compatibility problems.
    unsigned char BS_BootCode[448];     // Bootstrap program. It is platform dependent and filled with zero when not used.
    uint16_t BS_BootSign;               // 0xAA55. A boot signature indicating that this is a valid boot sector.
} __attribute__((packed)) FatBS1216_t;

typedef struct FatBS32 {
    FatBSHeader header;
    uint32_t BPB_FATSz32;               // Size of a FAT in unit of sector. The size of the FAT area is BPB_FATSz32 * BPB_NumFATs sector. This is an only field needs to be referred prior to determine the FAT type while this field exists in only FAT32 volume. But this is not a problem because BPB_FATSz16 is always invalid in FAT32 volume.
    uint16_t BPB_ExtFlags;              // Bit3-0: Active FAT starting from 0. Valid when bit7 is 1. Bit6-4: Reserved (0). Bit7: 0 means that each FAT are active and mirrored. 1 means that only one FAT indicated by bit3-0 is active. Bit15-8-4: Reserved (0). 
    uint16_t BPB_FSVer;                 // FAT32 version. Upper byte is major version number and lower byte is minor version number. This document describes FAT32 version 0.0. This field is for futuer extension of FAT32 volume to manage the filesystem verison. However, FAT32 volume will not be updated any longer.
    uint32_t BPB_RootClus;              // First cluster number of the root directory. It is usually set to 2, the first cluster of the volume, but it does not need to always be 2.
    uint16_t BPB_FSInfo;                // Sector of FSInfo structure in offset from top of the FAT32 volume. It is usually set to 1, next to the boot sector.
    uint16_t BPB_BkBootSec;             // Sector of backup boot sector in offset from top of the FAT32 volume. It is usually set to 6, next to the boot sector but 6 and any other value is not recommended.
    unsigned char BPB_Reserved[12];     // Reserved (0).
    uint8_t BS_DrvNum;                  // Same as the description of FatBS1216 field.
    uint8_t BS_Reserved1;               // Same as the description of FatBS1216 field.
    uint8_t BS_BootSig;                 // Same as the description of FatBS1216 field.
    uint32_t BS_VolID;                  // Same as the description of FatBS1216 field.
    unsigned char BS_VolLab[11];        // Same as the description of FatBS1216 field.
    unsigned char BS_FilSysType[8];     // Always "FAT32   " and has not any effect in determination of FAT type.
    unsigned char BS_BootCode32[420];   // Bootstrap program. It is platform dependent and filled with zero when not used.
    uint16_t BS_BootSign;               // 0xAA55. A boot signature indicating that this is a valid boot sector.
} __attribute__((packed)) FatBS32_t;

typedef struct Fat32FsInfo {
    uint32_t FSI_LeadSig;               // 0x41615252. This is a lead signature used to validate that this is in fact an FSInfo sector.
    unsigned char FSI_Reserved1[480];   // Reserved. This field should be always initialized to zero.
    uint32_t FSI_StrucSig;              // 0x61417272. Another signature that is more localized in the sector to the location of the fields that are used.
    uint32_t FSI_Free_Count;            // This field indicates the last known free cluster count on the volume. If the value is 0xFFFFFFFF, it is actually unknown. This is not necessarily correct, so that the FAT driver needs to make sure it is valid for the volume.
    uint32_t FSI_Nxt_Free;              // This field gives a hint for the FAT driver, the cluster number at which the driver should start looking for free clusters. Because a FAT32 FAT is large, it can be rather time consuming if there are a lot of allocated clusters at the start of the FAT and the driver starts looking for a free cluster starting at the first cluster. Typically this value is set to the last cluster number that the driver allocated. If the value is 0xFFFFFFFF, there is no hint and the driver should start looking at cluster 2. This may not be correct, so that the FAT driver needs to make sure it is valid for the volume.
    unsigned char FSI_Reserved2[12];    // Reserved. This field should be always initialized to zero.
    uint32_t FSI_TrailSig;              // 0xAA550000. This trail signature is used to validate that this is in fact an FSInfo sector.
} __attribute__((packed)) Fat32FsInfo_t;
                                        //  ________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
typedef struct Fat32Directory {         // | OFFSET | SIZE | DESCRIPTION
    unsigned char DIR_Name[11];         // |    0   |  11  | Short file name (SFN) of the object.
    uint8_t DIR_Attr;                   // |   11   |   1  | File attribute in combination of following flags. Upper 2 bits are reserved and must be zero. (0x01: ATTR_READ_ONLY (Read-only)), (0x02: ATTR_HIDDEN (Hidden)), (0x04: ATTR_SYSTEM (System)), (0x08: ATTR_VOLUME_ID (Volume label)), (0x10: ATTR_DIRECTORY (Directory)), (0x20: ATTR_ARCHIVE (Archive)), (0x0F: ATTR_LONG_FILE_NAME (LFN entry))
    uint8_t DIR_NTRes;                  // |   12   |   1  | Optional flags that indicates case information of the SFN. (0x08: Every alphabet in the body is low-case.), (0x10: Every alphabet in the extensiton is low-case.)
    uint8_t DIR_CrtTimeTenth;           // |   13   |   1  | Optional sub-second information corresponds to DIR_CrtTime. The time resolution of DIR_CrtTime is 2 seconds, so that this field gives a count of sub-second and its valid value range is from 0 to 199 in unit of 10 miliseconds. If not supported, set zero and do not change afterwards.
    uint16_t DIR_CrtTime;               // |   14   |   2  | Optional file creation time. If not supported, set zero and do not change afterwards.
    uint16_t DIR_CrtDate;               // |   16   |   2  | Optional file creation date. If not supported, set zero and do not change afterwards.
    uint16_t DIR_LstAccDate;            // |   18   |   2  | Optional last accesse date. There is no time information about last accesse time, so that the resolution of last accesse time is 1 day. If not supported, set zero and do not change afterwards.
    uint16_t DIR_FstClusHI;             // |   20   |   2  | Upeer part of cluster number. Always zero on the FAT12/16 volume.
    uint16_t DIR_WrtTime;               // |   22   |   2  | Last time when any change is made to the file (typically on closeing).
    uint16_t DIR_WrtDate;               // |   24   |   2  | Last data when any change is made to the file (typically on closeing).
    uint16_t DIR_FstClusLO;             // |   26   |   2  | Lower part of cluster number. Always zero if the file size is zero.
    uint32_t DIR_FileSize;              // |   28   |   4  | Size of the file in unit of byte. Not used when it is a directroy and the value must be always zero.
                                        //  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
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
// OFFSET WHERE DIRS START | DISK SIZE    
    { 66600,                        0 },  /* disks up to 32.5 MB, the 0 value for SecPerClusVal trips an error */
    { 532480,                       1 },  /* disks up to 260 MB, .5k cluster */
    { 16777216,                     8 },  /* disks up to 8 GB, 4k cluster */
    { 33554432,                    16 },  /* disks up to 16 GB, 8k cluster */
    { 67108864,                    32 },  /* disks up to 32 GB, 16k cluster */
    { 0xFFFFFFFF,                  64 }   /* disks greater than 32GB, 32k cluster */
};

void fat::create() {
    FatBS32 fatBs;
    FatBSHeader fatBsHeader = fatBs.header;
    
    memutils::memcpy(&fatBsHeader.BS_JmpBoot, (unsigned char*) "\xEB\x58\x90", 3);
    string::strcpy((char*) fatBsHeader.BS_OEMName, "MSDOS5.0");
    fatBsHeader.BPB_BytesPerSec = 0x200; // 512
    fatBsHeader.BPB_SecPerClus = 0x1;
    fatBsHeader.BPB_RsvdSecCnt = 0x1;
    fatBsHeader.BPB_NumFATs = 0x2;
    fatBsHeader.BPB_RootEntCnt = 0x0;
    fatBsHeader.BPB_TotSec16 = 0x0;
    fatBsHeader.BPB_Media = 0xf0;
    fatBsHeader.BPB_FATSz16 = 0x0;
    fatBsHeader.BPB_SecPerTrk = 0x12;    // 18
    fatBsHeader.BPB_NumHeads = 0x1;
    fatBsHeader.BPB_HiddSec = 0x0;
    fatBsHeader.BPB_TotSec32 = 0xB40;    // 2880

    fatBs.BPB_FATSz32 = 0x0;

    memutils::memHexDump(0, (void*) &fatBsHeader, sizeof(FatBSHeader), 17);
}