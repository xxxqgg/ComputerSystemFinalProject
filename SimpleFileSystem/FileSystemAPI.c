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

byte *read_from_disk(FCB *fcb);

bool write_to_disk(FCB *fcb, void *data, int size);

bool did_format();

static void read_current_dir() {
    if (current_dir != NULL) {
        free(current_dir);
    }
    current_dir = (DIR *) read_from_disk(current_fcb);
}

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
    read_current_dir();

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
    read_current_dir();
    for (int i = 0; i < MAX_CONTENT_NUM; i++) {
        if (current_dir->content[i].is_free == true) {
            return i;
        }
    }
    return -1;
}

int find_empty_block_in_FAT(int using_index) {
    // FAT[0] for root
    for (int i = 0; i < MAX_BLOCK_NUM; i++) {
        // Check for using index
        // 由于我们找下一块时，当前块还未写入，因此我们需要在找的时候，禁止返回当前块。
        if (i != using_index && disk->FAT1[i] == 0)
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
        FCB *root_fcb = malloc(sizeof(FCB));
        root_fcb->file_size = sizeof(DIR);
        root_fcb->base_index = 0;
        root_fcb->is_free = false;
        root_fcb->is_dir = true;
        current_fcb = root_fcb;
        read_current_dir();
    }
    return true;
}

DIR *init_dir(int block_index, int father_block_index) {
    DIR *dir = malloc(sizeof(DIR));
    // Init .
    FCB self;
    strcpy(self.name, ".");
    self.base_index = block_index;
//TODO: File size for dir ?

    self.file_size = sizeof(DIR);
    self.is_free = false;
    self.is_dir = true;
    dir->content[0] = self;

    FCB parent;
    strcpy(parent.name, "..");
    parent.base_index = father_block_index;
    parent.file_size = sizeof(DIR);
    parent.is_free = false;
    parent.is_dir = true;
    dir->content[1] = parent;
    for (int i = 2; i < MAX_CONTENT_NUM; i++) {
        dir->content[i].is_free = true;
    }
    return dir;
}

bool did_format() {
    if (disk == NULL) {
        return false;
    }
    FCB *root_fcb = malloc(sizeof(FCB));
    root_fcb->file_size = sizeof(DIR);
    root_fcb->base_index = 0;
    root_fcb->is_free = false;
    root_fcb->is_dir = true;
    current_dir = (DIR *) read_from_disk(root_fcb);
    free(root_fcb);
    if (current_dir == NULL) {
        return false;
    }
    if (strcmp(current_dir->content[0].name, ".") == 0) {
        return true;
    }
    return false;
}

bool write_to_disk(FCB *fcb, void *data, int size) {
    printf("size = %d\n", size);
    printf("block_size = %d\n", BLOCK_SIZE);
    fcb->file_size = size;
    int written_size = 0;
    fcb->file_size = size;
    int block = fcb->base_index;
    while (disk->FAT1[block] != -1 && disk->FAT1[block] != 0) {
        int prev = block;
        block = disk->FAT1[block];
        disk->FAT1[prev] = 0;
        disk->FAT2[prev] = 0;
    }
    disk->FAT1[block] = 0;
    disk->FAT2[block] = 0;
    if (size <= BLOCK_SIZE) {
        int block_index = fcb->base_index;
        if (block_index == -1) {
            return false;
        }
        disk->FAT1[block_index] = -1;
        disk->FAT2[block_index] = -1;
        memcpy(disk->data[block_index], data, size);
    } else {
        // TODO: Count available block???
        int block_index = fcb->base_index;
        if (block_index == -1) {
            return false;
        }

        memcpy(disk->data[block_index], data, BLOCK_SIZE);
        written_size = BLOCK_SIZE;
        int next_block_index = find_empty_block_in_FAT(block_index);
        disk->FAT1[block_index] = next_block_index;
        disk->FAT2[block_index] = next_block_index;
        printf("%d\n", written_size + BLOCK_SIZE);
        while (written_size + BLOCK_SIZE <= size) {
            block_index = next_block_index;
            next_block_index = find_empty_block_in_FAT(block_index);
            if (next_block_index == -1 && written_size + BLOCK_SIZE > size) {
                return false;
            }
            disk->FAT1[block_index] = next_block_index;
            disk->FAT2[block_index] = next_block_index;
            memcpy(disk->data[block_index], data + written_size, BLOCK_SIZE);
            written_size += BLOCK_SIZE;
        }
        if (written_size < size) {
            block_index = next_block_index;
            if (block_index == -1) {
                return false;
            }
            disk->FAT1[block_index] = -1;
            disk->FAT2[block_index] = -1;
            memcpy(disk->data[block_index], data + written_size, size - written_size);
        }
    }
    return true;
}

byte *read_from_disk(FCB *fcb) {
    byte *data = malloc(fcb->file_size);
    if (fcb->file_size <= BLOCK_SIZE) {
        memcpy(data, disk->data[fcb->base_index], fcb->file_size);

    } else {
        int read_size = 0;
        int block_index = fcb->base_index;
        while (read_size + BLOCK_SIZE <= fcb->file_size) {
            if (block_index == -1) {
                return NULL;
            }
            memcpy(data + read_size, disk->data[block_index], BLOCK_SIZE);
            read_size += BLOCK_SIZE;
            block_index = disk->FAT1[block_index];
        }
        if (read_size < fcb->file_size) {
            if (block_index == -1) {
                return NULL;
            }
            memcpy(data + read_size, disk->data[block_index], fcb->file_size - read_size);
        }
    }
    return data;
}

char *get_pwd() {
    return current_path;
}

bool format_disk(char **args) {
    if (disk == NULL) {
        fprintf(stderr, "You have to mount disk first!\n");
        return false;
    }
    DIR *root = init_dir(0, 0);
    memset(disk->FAT1, 0, sizeof(disk->FAT1));
    memset(disk->FAT2, 0, sizeof(disk->FAT2));
    FCB *root_fcb = malloc(sizeof(FCB));
    root_fcb->file_size = sizeof(DIR);
    root_fcb->base_index = 0;
    root_fcb->is_free = false;
    root_fcb->is_dir = true;
    write_to_disk(root_fcb, root, root_fcb->file_size);
    // TODO: Should we make current dir local variable???
    current_dir = (DIR *) read_from_disk(root_fcb);
    current_fcb = root_fcb;
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
    int index_in_disk = find_empty_block_in_FAT(-1);
    if (index_in_current_dir == -1 || index_in_disk == -1) {
        if (index_in_current_dir == -1) {
            fprintf(stderr, "There is no empty position in current dir!\n");
        }
        if (index_in_disk == -1) {
            fprintf(stderr, "There is no empty position on disk!\n");
        }
        return false;
    }
    read_current_dir();
    current_dir->content[index_in_current_dir].is_free = false;
    strcpy(current_dir->content[index_in_current_dir].name, args[1]);
    current_dir->content[index_in_current_dir].is_dir = true;
    current_dir->content[index_in_current_dir].base_index = index_in_disk;
    current_dir->content[index_in_current_dir].file_size = sizeof(DIR);
    DIR *new_dir = init_dir(index_in_disk, current_dir->content[0].base_index);
    bool res = write_to_disk(&current_dir->content[index_in_current_dir], new_dir,
                             current_dir->content[index_in_current_dir].file_size);
    write_to_disk(current_fcb, current_dir, current_fcb->file_size);
    return res;
}

bool ls(char **args) {
    printf("%-20s%-20s%-10s\n", "Name", "Type", "Size");
    read_current_dir();
    if (current_dir == NULL) {
        return false;
    }
    for (int i = 0; i < MAX_CONTENT_NUM; i++) {
        if (current_dir->content[i].is_free == false) {
            char *type;
            if (current_dir->content[i].is_dir) {
                type = "Dir";
            } else {
                type = "File";
            }
            printf("%-20s%-20s%-10d\n", current_dir->content[i].name, type, current_dir->content[i].file_size);
        }
    }
    return true;
}

bool cd(char **args) {
    read_current_dir();
    if (args[1] == NULL || args[2] != NULL) {
        fprintf(stderr, "Error! cd usage: cd dir_name\n");
        return false;
    }
    int content_index = find_index_in_current_dir_by_name(args[1]);
    if (content_index == -1) {
        fprintf(stderr, "Error. %s not found.\n", args[1]);
        return false;
    }
    if (current_dir->content[content_index].is_dir == false) {
        fprintf(stderr, "Error. %s is not a directory.\n", args[1]);
        return false;
    }
    if (strcmp(".", args[1]) == 0) {
        return true;
    } else if (strcmp("..", args[1]) == 0) {
        current_fcb->base_index = current_dir->content[content_index].base_index;
        if (strcmp(current_path, "/") != 0) {
            current_path[strlen(current_path) - 1] = 0;
            char *loc = strrchr(current_path, '/');
            *(loc + 1) = 0;
        }
    } else {
        current_fcb->base_index = current_dir->content[content_index].base_index;
        strcat(current_path, current_dir->content[content_index].name);
        strcat(current_path, "/");
    }
    read_current_dir();
    return true;
}