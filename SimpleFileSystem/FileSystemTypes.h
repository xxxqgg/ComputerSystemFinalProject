//
// Created by Jerry Zhang on 2020-06-30.
//

#ifndef COMPUTERFINALPROJECT_FILESYSTEMTYPES_H
#define COMPUTERFINALPROJECT_FILESYSTEMTYPES_H

#include "marcos.h"
#include <string.h>
#include <stdbool.h>

typedef unsigned char byte;
typedef struct FCB {
        char name[MAX_FILE_NAME_LEN];
        bool is_free;

        // Only when is free is True, is_dir may be true.
        bool is_dir;
        int file_size;
        int base_index;
} FCB;

// DIR store in ONE BLOCK, size = 4KB
typedef struct DIR {
    // TODO:
    FCB content[MAX_CONTENT_NUM];
} DIR;

// 4198400
typedef struct Disk {
    int FAT1[MAX_BLOCK_NUM];
    int FAT2[MAX_BLOCK_NUM]; // SIZE: 2048
    byte data[MAX_BLOCK_NUM][BLOCK_SIZE];
} Disk;


#endif //COMPUTERFINALPROJECT_FILESYSTEMTYPES_H
