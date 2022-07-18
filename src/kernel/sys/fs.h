#pragma once
#ifndef _FS_H_
#define _FS_H_

typedef struct {
    char name[32];
    unsigned int size;
    unsigned char *data;
} FileNode;

namespace fs {

    /**
     * @brief Locates a file in virtual file system.
     * 
     * @param fileName      File name being searched
     * @return FileNode     0=File not found, >0=File found
     */
    FileNode* findFile(const char* fileName);
}

#endif