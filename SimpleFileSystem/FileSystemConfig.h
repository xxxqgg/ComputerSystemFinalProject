//
// Created by Jerry Zhang on 2020-06-30.
//

#ifndef FILE_SYSTEM_CONFIG_H
#define FILE_SYSTEM_CONFIG_H
#include "marcos.h"
#include "FileSystemTypes.h"

int shared_memory_key;
int shared_memory_size;
Disk *disk;
char current_path[MAX_FILE_NAME_LEN];
DIR* current_dir;
short FAT1[MAX_BLOCK_NUM];
short FAT2[MAX_BLOCK_NUM];
int FAT_LEN = sizeof(FAT1);

#endif