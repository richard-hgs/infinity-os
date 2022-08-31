#pragma once
#ifndef _FAT_H_
#define _FAT_H_

#include <stdint.h>
#include <fstream>

// FAT ERRORS
#define FAT_NO_ERROR                                    0
#define FAT_ERROR_READ_BS_HEADER                        1
#define FAT_ERROR_UNSUPORTED_FORMAT                     2
#define FAT_ERROR_READ_BS32                             3
#define FAT_ERROR_READ_FSINFO                           4
#define FAT_ERROR_READ_FSINFO_INVALID_LEAD_SIGNATURE    5
#define FAT_ERROR_READ_FSINFO_INVALID_STRUCT_SIGNATURE  6
#define FAT_ERROR_READ_DIRECTORY                        7
#define FAT_ERROR_READ_FAT_ENTRY                        8

// FAT - MEDIA TYPES
#define FAT_MEDIA_TYPE_FIXED        0xF8
#define FAT_MEDIA_TYPE_REMOVABLE    0xF0

// FAT - FSINFO - SIGNATURES
#define FAT_FSINFO_LEAD_SIGNATURE   0x41615252
#define FAT_FSINFO_STRUCT_SIGNATURE 0x61417272

// FAT - DIRECTORY - ATTRIBUTES
#define FAT_DIR_ATTR_READ_ONLY 0x01
#define FAT_DIR_ATTR_HIDDEN    0x02
#define FAT_DIR_ATTR_SYSTEM    0x04
#define FAT_DIR_ATTR_VOLUME_ID 0x08
#define FAT_DIR_ATTR_DIRECTORY 0x10
#define FAT_DIR_ATTR_ARCHIVE   0x20
#define FAT_DIR_ATTR_LONG_NAME FAT_DIR_ATTR_READ_ONLY | FAT_DIR_ATTR_HIDDEN | FAT_DIR_ATTR_SYSTEM | FAT_DIR_ATTR_VOLUME_ID
#define FAT_DIR_ATTR_LONG_NAME_MASK FAT_DIR_ATTR_READ_ONLY | FAT_DIR_ATTR_HIDDEN | FAT_DIR_ATTR_SYSTEM | FAT_DIR_ATTR_VOLUME_ID | FAT_DIR_ATTR_DIRECTORY | FAT_DIR_ATTR_ARCHIVE

// FAT - LONG DIRECTORY
#define FAT_LDIR_LAST_LONG_ENTRY_MASK 0x40
#define FAT_LDIR_LAST_AND_UNUSED 0x00
#define FAT_LDIR_UNUSED 0xE5

/**
 * @brief Windows FAT Format Map Overview Disassembly
 *  - FOR A PENDRIVE OF 8 GB OF SIZE:
 *    - 7,3 GB AVAILABLE
 *    - 4096 Bytes per allocation unit
 *    - 1.903.871 Allocation units available.
 *    - 32 bits for each fat entry.
 *    - Serial Number: 9281-B345
 * 
 * SECTORS 1-3: 
 *      WINDOWS USES 3 SECTORS FOR BOOT, SO WE MUST CONFIGURE THE 3 FIRST SECTORS AS BOOTABLE FINISHING WITH 0xAA55
 * 
 * 
 * SECTOR 1: 
 *      BS_JmpBoot      : EB 58 90
 *      BS_OEMName      : "MSDOS5.0"
 *      BPB_BytesPerSec : 0x200
 *      BPB_SecPerClus  : 0x8      EACH FILE USES 8 SECTORS
 *      BPB_RsvdSecCnt  : 0xBCA    STARTS IN SECTOR 0
 *      BPB_NumFATs     : 0x02
 *      BPB_RootEntCnt  : 0x0
 *      BPB_TotSec16    : 0x0
 *      BPB_Media       : 0xF8
 *      BPB_FATSz16     : 0x0
 *      BPB_SecPerTrk   : 0x3F
 *      BPB_NumHeads    : 0xFF
 *      BPB_HiddSec     : 0x800
 *      BPB_TotSec32    : 0xE8E800
 *      BPB_FATSz32     : 0x3A1B
 *      BPB_ExtFlags    : 0x0
 *      BPB_FSVer       : 0x0
 *      BPB_RootClus    : 0x2
 *      BPB_FSInfo      : 0x1
 *      BPB_BkBootSec   : 0x6
 *      BPB_Reserved    : 0x0
 *      BS_DrvNum       : 0x8
 *      BS_Reserved1    : 0x0
 *      BS_BootSig      : 0x29
 *      BS_VolID        : 0x45B38192   Serial Number of Volume
 *      BS_VolLab       : "NO NAME    "
 *      BS_FilSysType   : "FAT32   "
 *      BS_BootCode32   : ***
 *      BS_BootSign     : 0X55AA
 * 
 * SECTOR 2 (BPB_FSInfo): 
 *      FSI_LeadSig     : 0x41615252
 *      FSI_Reserved1   : 0x0
 *      FSI_StrucSig    : 0x61417272
 *      FSI_Free_Count  : 0x1D0CFC * BPB_SecPerClus * BPB_BytesPerSec = FREE_SIZE_IN_BYTES
 *      FSI_Nxt_Free    : 0x6
 *      FSI_Reserved2   : 0x0
 *      FSI_TrailSig    : 0x0000AA55
 * 
 * SECTOR 6-8:
 *      BOOT SECTOR BACKUP
 * 
 * SECTOR 12:
 *      ANOTHER BOOT SECTOR USED FOR SOME UNKNOWN REASON
 * 
 * SECTOR 3018 = (BPB_RsvdSecCnt)
 *      - File Allocation Table 1 Starts at this sector.
 *      SECTOR 3018 (0-4 bytes)             : RESERVED 1
 *      SECTOR 3018 (4-8 bytes)             : RESERVED 2
 *      SECTOR 3018 (8 byte) - SECTOR 32767 : FAT FILES AND DIRECTORIES ENTRIES
 * 
 *      CONTENTS :
 *          - 
 *     
 * SECTOR 32768 = (BPB_RsvdSecCnt + (BPB_NumFATs * BPB_FATSz32)):
 *      ROOT Fat32Directory entry sector
 * 
 *      SUPPOSING WE HAVE A STORAGE STRUCTURE LIKE THIS IN FAT32:
 *      .
 *      ├── System Volume Information/
 *      |   ├── IndexerVolumeGuid   CONTENT -> {0E347D23-EAE4-4C18-A4A6-B5E63A65BC0B}
 *      |   └── WPSettings.dat      CONTENT -> HEX(0C 00 00 00 C4 B7 A8 C2 44 3F A8 16)  ASCII(....Ä·¨ÂD?¨.)
 *      ├── Folder1/
 *      │   └── File3.txt           CONTENT -> Hello World 3  
 *      ├── File1.txt               CONTENT -> Hello World 1
 *      └── File2.txt               CONTENT -> Hello World 2
 * 
 *      FOLDER(Fat32Directory) - System Volume Information:
 *       - LOCATED SECTOR(32.792-32.799)
 *          DIR_Name[11]     : "PENDRIVE   "
 *          DIR_Attr         : 0x8 - ATTR_VOLUME_ID (Volume label)
 *          DIR_NTRes        : 0x0 
 *          DIR_CrtTimeTenth : 0x0
 *          DIR_CrtTime      : 0x0
 *          DIR_CrtDate      : 0x0
 *          DIR_LstAccDate   : 0x0
 *          DIR_FstClusHI    : 0x0
 *          DIR_WrtTime      : 0xB1AC
 *          DIR_WrtDate      : 0x550D
 *          DIR_FstClusLO    : 0x0
 *          DIR_FileSize     : 0x0
 *      
 *       ENTRY - SYSTEM~1:
 *          DIR_Name[11]     : "SYSTEM~1   "
 *          DIR_Attr         : 0x16 - ???
 *          DIR_NTRes        : 0x0 
 *          DIR_CrtTimeTenth : 0x81
 *          DIR_CrtTime      : 0xB1AB
 *          DIR_CrtDate      : 0x550D
 *          DIR_LstAccDate   : 0x550F
 *          DIR_FstClusHI    : 0x0
 *          DIR_WrtTime      : 0xB1AC
 *          DIR_WrtDate      : 0x550D
 *          DIR_FstClusLO    : 0x3
 *          DIR_FileSize     : 0x0
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

typedef struct Fat32LongDirectory {     // OFFSET | SIZE | DESCRIPTION
    uint8_t LDIR_Ord;                   //    0   |   1  | If it's a directory long name it uses Little Endian order. The order of this entry in the sequence of long dir entries associated with the short dir entry at the end of the long dir set. If masked with 0x40 (LAST_LONG_ENTRY), this indicates the entry is the last long dir entry in a set of long dir entries. All valid sets of long dir entries must begin with an entry having this mask.
    unsigned char LDIR_Name1[10];       //    1   |  10  | Characters 1-5 of the long-name sub-component in this dir entry.
    uint8_t LDIR_Attr;                  //   11   |   1  | Attributes - must be ATTR_LONG_NAME
    uint8_t LDIR_Type;                  //   12   |   1  | If zero, indicates a directory entry that is a sub-component of a long name. NOTE: Other values reserved for future extensions. Non-zero implies other dirent types.
    uint8_t LDIR_Chksum;                //   13   |   1  | Checksum of name in the short dir entry at the end of the long dir set.
    unsigned char LDIR_Name2[12];       //   14   |  12  | Characters 6-11 of the long-name sub-component in this dir entry.
    uint16_t LDIR_FstClusLO;            //   26   |   2  | Must be ZERO. This is an artifact of the FAT "first cluster" and must be zero for compatibility with existing disk utilities. It's meaningless in the context of a long dir entry.
    unsigned char LDIR_Name3[4];        //   28   |   4  | Characters 12-13 of the long-name sub-component in this dir entry.
} __attribute__((packed)) Fat32LongDirectory_t;

// Structure that holds the fat type definitions
typedef struct DskSzToSecPerClus {
    unsigned int DiskSize; // BPB_TotSec32
    uint8_t SecPerClusVal; // BPB_SecPerClus
} DskSzToSecPerClus_t;

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
 * FAT - File Allocation Table:
 *    - The allocation table on a FAT volume tracks each cluster in that volume, mapping the allocation chains associated with the 
 *      volume's directories and files. Each table entry corresponds to a cluster and is typically in one of four states:
 *      - FREE        - The cluster is unused
 *      - CONTENT     - The cluster is in an allocation chain associated with a specific file or directory; the table entry references to the next cluster in the chain.
 *      - EOC         - The cluster is the last cluster in an allocation chain associated with a specific file or directory.
 *      - BAD CLUSTER - The cluster is bad.
 * 
 *    - Each fat entry has 28 bits and not 32 bits. Only the 28 low bits should be used for detecting FREE, EOC and BAD CLUSTERS
 *    
 *    - FREE - Free Unused Cluster:
 *      - 0x00000000
 *      - Signals that the cluster is free to be used.
 * 
 *    - CONTENT - File or Directory:
 *      - 
 *      - Indicates that a file or directory is located in this cluster chain entry
 * 
 *    - EOC - End Of Clusterchain:
 *      - 0x0FFFFFF8 - 0x0FFFFFFF
 *      - Signals the end of the clusterchain of the current file being readed.
 *      - Also, the end-of-file number is actually anything equal to or greater than 0xFFFFFFF8
 *      - Files that fit in just one cluster will have only 0xFFFFFFFF in the fat at their cluster chain
 * 
 *    - Bad Cluster:
 *      - 0x0FFFFFF7
 *      - Signals that the cluster should not be placed on the free list because it's prone to disk errors.
 * 
 *    - RESERVED ENTRIES OF FAT TABLE :
 *      - FAT[0] = 0x0FFFFFF8: 
 *        - Contains the BPB_Media byte value in its low 8 bits and all other bits are set to 1.
 *      
 *      - FAT[1] = (ClnShutBitMask = 0x08000000 | HrdErrBitMask = 0x04000000):
 *        - The file system driver may use the high two bits of the FAT[1] entry for dirty volume flags (all other bits, are always left set to 1).
 *        - Is set by FORMAT to the EOC mark
 *        
 *        - ClnShutBitMask = 0x08000000 (HIGH BIT - HIGH):
 *            - If bit is 1, volume is “clean”.
 *            - If bit is 0, volume is “dirty”. This indicates that the file system driver did not
 *              Dismount the volume properly the last time it had the volume mounted. It
 *              would be a good idea to run a Chkdsk/Scandisk disk repair utility on it,
 *              because it may be damaged.
 * 
 *        - HrdErrBitMask = 0x04000000 (HIGH BIT - LOW):
 *            - If this bit is 1, no disk read/write errors were encountered.
 *            - If this bit is 0, the file system driver encountered a disk I/O error on the
 *              Volume the last time it was mounted, which is an indicator that some sectors
 *              may have gone bad on the volume. It would be a good idea to run a
 *              Chkdsk/Scandisk disk repair utility that does surface analysis on it to look
 *              for new bad sectors.
 *        
 *         - The last FAT entry is the CountofClusters + 1
 *
 * 
 * SIZE_LIMITS:
 *    FAT12 requirements : 3 sectors on each copy of FAT for every 1,024 clusters
 *    FAT16 requirements : 1 sector on each copy of FAT for every 256 clusters
 *    FAT32 requirements : 1 sector on each copy of FAT for every 128 clusters
 *    
 *    FAT12 range : 1 to 4,084 clusters : 1 to 12 sectors per copy of FAT
 *    FAT16 range : 4,085 to 65,524 clusters : 16 to 256 sectors per copy of FAT
 *    FAT32 range : 65,525 to 268,435,444 clusters : 512 to 2,097,152 sectors per copy of FAT
 *    
 *    FAT12 minimum : 1 sector per cluster × 1 clusters = 512 bytes (0.5 KB)
 *    FAT16 minimum : 1 sector per cluster × 4,085 clusters = 2,091,520 bytes (2,042.5 KB)
 *    FAT32 minimum : 1 sector per cluster × 65,525 clusters = 33,548,800 bytes (32,762.5 KB)
 *    
 *    FAT12 maximum : 64 sectors per cluster × 4,084 clusters = 133,824,512 bytes (≈ 127 MB)
 *    [FAT12 maximum : 128 sectors per cluster × 4,084 clusters = 267,694,024 bytes (≈ 255 MB)]
 *    
 *    FAT16 maximum : 64 sectors per cluster × 65,524 clusters = 2,147,090,432 bytes (≈2,047 MB)
 *    [FAT16 maximum : 128 sectors per cluster × 65,524 clusters = 4,294,180,864 bytes (≈4,095 MB)]
 *    
 *    FAT32 maximum : 8 sectors per cluster × 268,435,444 clusters = 1,099,511,578,624 bytes (≈1,024 GB)
 *    FAT32 maximum : 16 sectors per cluster × 268,173,557 clusters = 2,196,877,778,944 bytes (≈2,046 GB)
 *    [FAT32 maximum : 32 sectors per cluster × 134,152,181 clusters = 2,197,949,333,504 bytes (≈2,047 GB)]
 *    [FAT32 maximum : 64 sectors per cluster × 67,092,469 clusters = 2,198,486,024,192 bytes (≈2,047 GB)]
 *    [FAT32 maximum : 128 sectors per cluster × 33,550,325 clusters = 2,198,754,099,200 bytes (≈2,047 GB)]
 */
namespace fat {
    /**
     * @brief Create a Fat32 formatation data to be writted in the storage device.
     * This will generate all required configuration for currently format being generated.
     * 
     * @param rsvdSecCnt Reserved sectors count before the first FAT entry. Specify reserved area for boot executable code.
     */
    void create(uint32_t diskTotSec, uint16_t bytesPerSec, uint32_t fatSizeInSec, uint8_t mediaType, FatBS32* fatBs);

    /**
     * @brief List all entries of a given FAT storage that is inside a storage path
     * 
     * @param storage   FAT storage
     * @param path      Path to list files
     * @return int      0=NO_ERROR, or Error code
     */
    int listEntries(FILE *storage, char* path);

    /**
     * @brief Read the next directory entry of the current directory DATA region
     * 
     * @param storage   FAT storage
     * @param fat32Dir  OUT - Reference that will receive the readed dir
     * @return int      0=NO_ERROR, or Error code
     */
    int readNextDirEntry(FILE *storage, Fat32Directory_t *fat32Dir);

    /**
     * @brief Read the Fat Entry Value for given cluster number
     * 
     * @param storage           FAT storage
     * @param bs32              MBR Boot Sector info
     * @param clusterNum        Cluster number
     * @param fatValue          OUT - Reference to var that will save the readed fat value
     * @return int              0=NO_ERROR, or Error code
     */
    int fatVal(FILE *storage, FatBS32_t bs32, uint32_t clusterNum, uint32_t *fatValue);

    /**
     * @brief Print directory entry info
     * 
     * @param fat32Dir Directory entry
     */
    void printDirEntry(Fat32Directory_t fat32Dir);

    /**
     * @brief Print long directory entry info
     * 
     * @param fat32LongDir Long Directory entry
     */
    void printLongDirEntry(Fat32LongDirectory_t fat32LongDir);

    /**
     * @brief Cast dirAttr to string representation
     * 
     * @param dirAttr    Attribute to be converted
     * @param dirAttrStr OUT - Reference that will hold the attribute string value
     */
    void dirAttrToStr(uint8_t dirAttr, char *dirAttrStr);

    /**
     * @brief Copy long name to output buffer reference of fullName at the given outOffset of fullName
     * 
     * @param longName  Long name to be copied
     * @param lsize     Long name size
     * @param outOffset Output offset where to start writting the chars
     * @param fullName  Output buffer reference that will store the full name
     * @return Amount of chars copied
     */
    int longNameStrCpy(unsigned char *longName, int lsize, int outOffset, char *fullName);


    /**
     * @brief Copy fat32LongDir long name to output buffer reference of fullName at the give outOFfset of fullName
     * 
     * @param fat32LongDir  fat32LongDir name to be copied
     * @param outOffset     Output offset where to start writting the chars
     * @param fullName      Output buffer reference that will store the full name
     * @return int          
     */
    int longNameStrCpy(Fat32LongDirectory_t fat32LongDir, int outOffset, char *fullName);
}

#endif