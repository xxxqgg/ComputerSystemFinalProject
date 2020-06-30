//
// Created by Jerry Zhang on 2020-06-30.
//

#ifndef FILE_SYSTEM_CONFIG_H
#define FILE_SYSTEM_CONFIG_H
#include "marcos.h"
#include "FileSystemTypes.h"

int shared_memory_key;
Disk *disk;
char current_path[MAX_FILE_NAME_LEN];
DIR* current_dir;
FCB* current_fcb;
int shared_memory_size;
int FAT1[MAX_BLOCK_NUM];
int FAT2[MAX_BLOCK_NUM];
int FAT_LEN = sizeof(FAT1);

#endif