// libc
#include <stdint.h>
// stdlibs
#include "string.h"
#include "stdio.h"
// memory
#include "memutils.h"
// cpu
#include "paging.h"
// binaries programs
#include "../../../build/programs/user/shell/shell.bin.h"

#include "fs.h"

// ==================== VIRTUAL FILE SYSTEM =========================
FileNode fileList[] = {
    { "shell.exe", shell_bin_len, shell_bin }
};

const unsigned int filesCount = sizeof(fileList) / sizeof(FileNode);
// ==================== VIRTUAL FILE SYSTEM =========================

// ===================== FAT32 FILE SYSTEM ==========================
#define FAT_MEDIA_TYPE_FIXED 0xF8
#define FAT_MEDIA_TYPE_REMOVABLE 0xF0

struct Fat32BootSector {
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
} __attribute__((packed));

struct Fat32FsInfo {
    uint32_t FSI_LeadSig;
    unsigned char FSI_Reserved1[480];
    uint32_t FSI_StrucSig;
    uint32_t FSI_Free_Count;
    uint32_t FSI_Nxt_Free;
    unsigned char FSI_Reserved2[12];
    uint32_t FSI_TrailSig;
};

struct Fat32Directory {
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
};

// Structure that holds the fat type definitions
struct DskSzToSecPerClus {
    unsigned int DiskSize; // BPB_TotSec32
    uint8_t SecPerClusVal; // BPB_SecPerClus
};

Fat32BootSector bootSector;

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
// ===================== FAT32 FILE SYSTEM ==========================

void fs::install() {
    // Add 3 to Boot start address to ignore the JMP instruction that is 3 bytes long. After it is the boot sector.
    memutils::memcpy((void*) &bootSector, (void*) (BOOT_START_ADDR + 3), sizeof(Fat32BootSector));

    // stdio::kprintf("FS - BOOT SECTOR INFORMATION: \n");
    // stdio::kprintf("   - BS_OEMName     : %s\n", bootSector.BS_OEMName);
    // stdio::kprintf("   - BPB_BytesPerSec: %d bytes\n", bootSector.BPB_BytesPerSec);
    // stdio::kprintf("   - BPB_SecPerClus : %d sectors per cluster\n", bootSector.BPB_SecPerClus);
    // stdio::kprintf("   - BPB_RsvdSecCnt : %d reserved sectors count\n", bootSector.BPB_RsvdSecCnt);
    // stdio::kprintf("   - BPB_NumFATs    : %d FAT count\n", bootSector.BPB_NumFATs);
    // stdio::kprintf("   - BPB_RootEntCnt : %d Root dir entries count\n", bootSector.BPB_RootEntCnt);
    // stdio::kprintf("   - BPB_TotSec16   : %d FAT_16 Total sectors count\n", bootSector.BPB_TotSec16);
    // stdio::kprintf("   - BPB_Media      : 0x%x Media type\n", bootSector.BPB_Media);
    // stdio::kprintf("   - BPB_FATSz16    : %d FAT_16 size\n", bootSector.BPB_FATSz16);
    // stdio::kprintf("   - BPB_SecPerTrk  : %d Sectors per track\n", bootSector.BPB_SecPerTrk);
    // stdio::kprintf("   - BPB_NumHeads   : %d Heads\n", bootSector.BPB_NumHeads);
    // stdio::kprintf("   - BPB_HiddSec    : %d Hidden sectors\n", bootSector.BPB_HiddSec);
    // stdio::kprintf("   - BPB_TotSec32   : %d FAT_32 Total sectors count\n", bootSector.BPB_TotSec32);
    // stdio::kprintf("   - BPB_FATSz32    : %d FAT_32 size\n", bootSector.BPB_FATSz32);
    // stdio::kprintf("   - BPB_ExtFlags   : Flags: (0x%x) BIN:%016b\n", bootSector.BPB_ExtFlags, bootSector.BPB_ExtFlags);
    // stdio::kprintf("   - BPB_FSVer      : %x File system version\n", bootSector.BPB_FSVer);
    // stdio::kprintf("   - BPB_RootClus   : %d Sector of root dir cluster\n", bootSector.BPB_RootClus);
    // stdio::kprintf("   - BPB_FSInfo     : %d Sector of FSINFO struct in reserved area\n", bootSector.BPB_FSInfo);
    // stdio::kprintf("   - BPB_BkBootSec  : %d Sector of BOOT record backup in reserved area\n", bootSector.BPB_BkBootSec);
    // stdio::kprintf("   - BS_DrvNum      : %d Driver number\n", bootSector.BS_DrvNum);
    // stdio::kprintf("   - BS_BootSig     : 0x%x Boot Signature\n", bootSector.BS_BootSig);
    // stdio::kprintf("   - BS_VolID       : 0x%04x Volume ID\n", bootSector.BS_VolID);


}

// ==================== VIRTUAL FILE SYSTEM =========================
FileNode* fs::findFile(const char* fileName) {
    unsigned int i;
    for (i=0; i<filesCount; i++) {
        if (string::strcmp(fileList[i].name, fileName) == 0) {
            // File found
            return &fileList[i];
        }
    }
    return nullptr;
}
// ==================== VIRTUAL FILE SYSTEM =========================

void fs::test() {
    stdio::kprintf("FS - test");
}