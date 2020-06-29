//
// Created by Jerry Zhang on 2020-06-30.
//

#include "FileSystemAPI.h"
#include "FileSystemTypes.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "Global.h"

bool did_format();

bool is_file_system_api(char **args) {
    if (args == NULL || args[0] == NULL)
        return false;
    for (int i = 0; i < num_of_api(); i++) {
        if (strcmp(args[0], api_str[i]) == 0)
            return true;
    }
    return false;
}

int find_index_in_current_dir_by_name(char *name) {
    for (int i = 0; i < MAX_CONTENT_NUM; i++) {
        if (current_dir->content[i].is_free == false) {
            if (strcmp(name, current_dir->content[i].name) == 0) {
                return i;
            }
        }
    }
    return -1;
}

int find_empty_position_in_current_dir() {
    for (int i = 0; i < MAX_CONTENT_NUM; i++) {
        if (current_dir->content[i].is_free == true) {
            return i;
        }
    }
    return -1;
}

short find_empty_block_in_FAT() {
    // FAT[0] for root
    for (short i = 1; i < MAX_BLOCK_NUM; i++) {
        if (disk->FAT1[i] == 0)
            return i;

    }
    return -1;
}

int exec_api(char **args) {
    if (args == NULL || args[0] == NULL)
        return false;
    for (int i = 0; i < num_of_api(); i++) {
        if (strcmp(args[0], api_str[i]) == 0) {
            return apis[i](args);
        }
    }
    return false;
}

bool init_file_system(char **args) {
    printf("Initializing file system\n");
    printf("Shared memory key = %d\n", shared_memory_key);
    int shared_memory_id;
    shared_memory_id = shmget(shared_memory_key, shared_memory_size, IPC_CREAT | 0666);
    if (shared_memory_id < 0) {
        perror("Shared memory has not been created yet");
        exit(1);
    } else {
        printf("Shared memory init successfully with it's id = %d\n", shared_memory_id);
        disk = (Disk *) shmat(shared_memory_id, 0, 0);
        if (disk == NULL) {
            fprintf(stderr, "Error: disk mounting error!\n");
            return false;
        }
        printf("Disk mounted\n");
    }

    if (did_format()) {
        strcpy(current_path, "/");
        current_dir = (DIR *) disk->data[0];
    }
    return true;
}

DIR init_dir(short block_index, short father_block_index) {
    DIR dir;
    // Init .
    FCB self;
    strcpy(self.name, ".");
    self.base_index = block_index;
//TODO: File size for dir ?

    self.file_size = 0;
    self.is_free = false;
    self.is_dir = true;
    dir.content[0] = self;

    FCB parent;
    strcpy(parent.name, "..");
    parent.base_index = father_block_index;
    parent.file_size = 0;
    parent.is_free = false;
    parent.is_dir = true;
    dir.content[1] = parent;
    for (int i = 2; i < MAX_CONTENT_NUM; i++) {
        dir.content[i].is_free = true;
    }
    return dir;
}

bool did_format() {
    if (disk == NULL) {
        return false;
    }
    DIR *root = (DIR *) disk->data[0];
    if (strcmp(root->content[0].name, ".") == 0) {
        return true;
    }
    return false;
}

void write_to_disk(int block_index, void *data, int size) {
    printf("size = %d\n", size);
    memset(disk->data[block_index], 0, sizeof(disk->data[0]));
    memcpy(disk->data[block_index], data, size);
}

char *get_pwd() {
    return current_path;
}

bool format_disk(char **args) {
    if (disk == NULL) {
        fprintf(stderr, "You have to mount disk first!\n");
        return false;
    }
    DIR root = init_dir(0, 0);
    memset(disk->FAT1, 0, sizeof(disk->FAT1));
    memset(disk->FAT2, 0, sizeof(disk->FAT2));
    write_to_disk(0, &root, sizeof(root));
    // TODO: Should we make current dir local variable???
    current_dir = (DIR *) disk->data[0];
    strcpy(current_path, "/");
    return true;
}

bool mkdir(char **args) {
    if (args[1] == NULL) {
        printf("Not a valid position.\n");
        printf("Usage: mkdir dirname.\n");
        return false;
    }
    int index = find_index_in_current_dir_by_name(args[1]);
    if (index != -1) {
        fprintf(stderr, "%s, already exists!\n", args[1]);
        return false;
    }
    int index_in_current_dir = find_empty_position_in_current_dir();
    short index_in_disk = find_empty_block_in_FAT();
    if (index_in_current_dir == -1 || index_in_disk == -1) {
        if (index_in_current_dir == -1) {
            fprintf(stderr, "There is no empty position in current dir!\n");
        }
        if (index_in_disk == -1) {
            fprintf(stderr, "There is no empty position on disk!\n");
        }
        return false;

    }
    current_dir->content[index_in_current_dir].is_free = false;
    strcpy(current_dir->content[index_in_current_dir].name, args[1]);
    current_dir->content[index_in_current_dir].is_dir = true;
    current_dir->content[index_in_current_dir].base_index = index_in_disk;
    init_dir(index_in_disk, current_dir->content[0].base_index);

    return true;
}

bool ls(char **args) {
    printf("%20s%10s%5s\n", "Name", "Type", "Size");
    for (int i = 0; i < MAX_CONTENT_NUM; i++) {
        if (current_dir->content[i].is_free == false) {
            char *type;
            if (current_dir->content[i].is_dir) {
                type = "Dir";
            } else {
                type = "File";
            }
            printf("%20s%10s%5d\n", current_dir->content[i].name, type, current_dir->content[i].file_size);
        }
    }
    return true;
}