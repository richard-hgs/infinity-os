#include "fs.h"
#include "string.h"
#include "../../../build/programs/user/shell/shell.bin.h"

FileNode fileList[] = {
    { "shell.exe", shell_bin_len, shell_bin }
};

const unsigned int filesCount = sizeof(fileList) / sizeof(FileNode);

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