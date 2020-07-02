//
// Created by Jerry Zhang on 2020-06-30.
//

#ifndef COMPUTERFINALPROJECT_FILESYSTEMTYPES_H
#define COMPUTERFINALPROJECT_FILESYSTEMTYPES_H

#include "marcos.h"
#include <string.h>
#include <stdbool.h>

typedef unsigned char byte;
/**
 * FCB File control block，记录文件信息。
 *
 */
typedef struct FCB {
        char name[MAX_FILE_NAME_LEN];
        bool is_free;
        bool is_dir;
        int file_size;
        int base_index;
} FCB;

/**
 * 目录文件类型，每个目录占有相同大小的硬盘空间。
 */
// DIR store in ONE BLOCK, size = 4KB
typedef struct DIR {
    // . ..
//    FCB self;
//    FCB father;
    FCB content[MAX_CONTENT_NUM]; // . content[0] .. content[1]
} DIR;

/**
 * 磁盘数据类型
 * 磁盘根目录基地址在 0 号位，即 data[0]
 * 初始化磁盘时，需要首先将磁盘根目录从 0 号位读出来
 */
typedef struct Disk {
    int FAT1[MAX_BLOCK_NUM];
    int FAT2[MAX_BLOCK_NUM];
    byte data[MAX_BLOCK_NUM][BLOCK_SIZE];
} Disk;


#endif //COMPUTERFINALPROJECT_FILESYSTEMTYPES_H
