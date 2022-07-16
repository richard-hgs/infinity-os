#pragma once
#ifndef _FS_H_
#define _FS_H_

typedef struct {
    char name[32];
    unsigned int size;
    unsigned char *data;
} FileNode;

namespace fs {

}

#endif