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

void fs::install() {
    // Add 3 to Boot start address to ignore the JMP instruction that is 3 bytes long. After it is the boot sector.
    // memutils::memcpy((void*) &bootSector, (void*) (BOOT_START_ADDR + 3), sizeof(Fat32BootSector));
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