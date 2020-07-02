//
// Created by Jerry Zhang on 2020-06-30.
//
/**
 * 代码执行顺序
 * 1. init - 从共享内存读出磁盘数据（如果没有则创建）
 * 2. format - 格式化磁盘为 FAT32
 * 3. init DIR - 初始化 /
 * 4. xxx
 */
#include "FileSystemAPI.h"
#include "FileSystemTypes.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "Global.h"
#include <unistd.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>


byte *read_from_disk(FCB *fcb);

bool write_to_disk(FCB *fcb, void *data);

bool did_format();

bool is_dir_empty(FCB *fcb);

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

bool __rm_fcb(FCB *fcb) {
    int block_index = fcb->base_index;
    int prev_index;
    while (disk->FAT1[block_index] != -1 && disk->FAT1[block_index] != 0) {
        prev_index = block_index;
        block_index = disk->FAT1[block_index];
        disk->FAT1[prev_index] = 0;
        disk->FAT2[prev_index] = 0;
    }
    disk->FAT1[block_index] = 0;
    disk->FAT2[block_index] = 0;
    return true;
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
        printf("Disk size = %d\n", shared_memory_size);
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

    self.file_size = sizeof(DIR);
    self.is_free = false;
    self.is_dir = true;
    dir->content[0] = self;
    // Dir FCB
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
/**
 * 读取根目录
 * 通过判断 根目录是否存在，并且根目录的第 0 个目录是否为 "." 判断是否初始化了格式。
 * @return
 */
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
/**
 * 将数据写入磁盘中，此处还没有设计好，size 参数应该可以不用，大小记录在 fcb.file_size 中
 * @param fcb
 * @param data 数据指针
 * @param size
 * @return
 */
bool write_to_disk(FCB *fcb, void *data) {
    int size = fcb->file_size;
    printf("size = %d\n", size);
    fcb->file_size = size;
    int written_size = 0;
    fcb->file_size = size;
    int block = fcb->base_index;
    /**
     * 如果该位置曾今有数据，将这部分数据抹除
     * 要抹除是为了更新数据时，将数据写入原地址中。
     */

    while (disk->FAT1[block] != -1 && disk->FAT1[block] != 0) {
        int prev = block;
        block = disk->FAT1[block];
        disk->FAT1[prev] = 0;
        disk->FAT2[prev] = 0;
    }
    disk->FAT1[block] = 0;
    disk->FAT2[block] = 0;
    /**
     * 当数据大小小于 一个 BLOCK 的大小，直接写入
     */
    if (size <= BLOCK_SIZE) {
        int block_index = fcb->base_index;
        if (block_index == -1) {

            return false;
        }
        disk->FAT1[block_index] = -1;
        disk->FAT2[block_index] = -1;
        memcpy(disk->data[block_index], data, size);
    } else {
        // 当数据大小大于一个 block，分批写入

        // TODO: Count available block???
        int block_index = fcb->base_index;
        if (block_index == -1) {
            return false;
        }
        // 首先将第一块写入
        memcpy(disk->data[block_index], data, BLOCK_SIZE);
        written_size = BLOCK_SIZE;
        int next_block_index = find_empty_block_in_FAT(block_index);
        disk->FAT1[block_index] = next_block_index;
        disk->FAT2[block_index] = next_block_index;
        // 当文件剩余大小有一整块时，写入一块
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
        // block size = 4 A = 11
        // 处理文件剩余大小不满一整块时的情况
        if (written_size < size) {
            block_index = next_block_index;
            if (block_index == -1) {
                return false;
            }
            disk->FAT1[block_index] = -1;
            disk->FAT2[block_index] = -1;
            memcpy(disk->data[block_index], data + written_size, size - written_size);
        } else if (written_size == size) {
            // 当文件刚好写入，设置最后写入的块的 FAT 为 -1
            disk->FAT1[block_index] = -1;
            disk->FAT2[block_index] = -1;
        } else {
            // Error!
            fprintf(stderr, "Error: write file error. written size too large.\n");
            return false;
        }
    }
    return true;
}

/**
 * 从磁盘中读取文件，读到内存中
 * @param fcb
 * @return
 */
byte *read_from_disk(FCB *fcb) {
    // 根据文件大小，申请内存
    byte *data = malloc(fcb->file_size);
    // 如果文件大小小于磁盘一块的大小，直接读入
    if (fcb->file_size <= BLOCK_SIZE) {
        memcpy(data, disk->data[fcb->base_index], fcb->file_size);
    } else {
        // 分块读入
        int read_size = 0;
        int block_index = fcb->base_index;
        while (read_size + BLOCK_SIZE <= fcb->file_size) {
            if (block_index == -1) {
                return NULL;
            }

            // 通过 data+read_size 的方式，分块将文件块读入内存
            memcpy(data + read_size, disk->data[block_index], BLOCK_SIZE);
            read_size += BLOCK_SIZE;
            block_index = disk->FAT1[block_index];
        }
        // 处理剩余大小不满一个磁盘块大小的情况
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
/**
 * 格式化磁盘
 * @param args
 * @return
 */
bool format_disk(char **args) {
    if (disk == NULL) {
        fprintf(stderr, "You have to mount disk first!\n");
        return false;
    }
    // 首先初始化 root 目录，root 目录父目录设置为自己
    // File size = sizeof(DIR) 0
    DIR *root = init_dir(0, 0);
    memset(disk->FAT1, 0, sizeof(disk->FAT1));
    memset(disk->FAT2, 0, sizeof(disk->FAT2));
    // root 目录的 FCB 的基地址设置为0，之后在任何地方，读取基地址为 0，file_size 为一个目录文件大小的 FCB，都可以直接读取到根目录。
    FCB *root_fcb = malloc(sizeof(FCB));
    root_fcb->file_size = sizeof(DIR);
    root_fcb->base_index = 0;
    // 1. base index
    // 2. file size
    root_fcb->is_free = false;
    root_fcb->is_dir = true;
    // 将目录信息写入磁盘
    write_to_disk(root_fcb, root);
    // TODO: Should we make current dir local variable???
    current_dir = (DIR *) read_from_disk(root_fcb);
    current_fcb = root_fcb;
    strcpy(current_path, "/");
    return true;
}

bool fs_mkdir(char **args) {
    if (args[1] == NULL) {
        printf("Not a valid position.\n");
        printf("Usage: fs_mkdir dirname.\n");
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

    bool res = write_to_disk(&current_dir->content[index_in_current_dir], new_dir);
    write_to_disk(current_fcb, current_dir);
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
            int file_size = 0;
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
    // .
    // ..
    } else {
        current_fcb->base_index = current_dir->content[content_index].base_index;
        strcat(current_path, current_dir->content[content_index].name);
        strcat(current_path, "/");
    }
    read_current_dir();
    return true;
}

bool touch(char **args) {
    if (args[1] == NULL || args[2] != NULL) {
        fprintf(stderr, "Usage: touch filename\n");
        return false;
    }
    read_current_dir();
    int index_in_dir = find_index_in_current_dir_by_name(args[1]);
    if (index_in_dir != -1) {
        fprintf(stderr, "%s already exists\n", args[1]);
        return false;
    }
    index_in_dir = find_empty_position_in_current_dir();
    if (index_in_dir == -1) {
        fprintf(stderr, "The folder is full\n");
        return false;
    }
    int index_in_disk = find_empty_block_in_FAT(-1);
    if (index_in_disk == -1) {
        fprintf(stderr, "The disk is full.\n");
        return false;
    }

    current_dir->content[index_in_dir].base_index = index_in_disk;
    current_dir->content[index_in_dir].is_dir = false;
    strcpy(current_dir->content[index_in_dir].name, args[1]);
    current_dir->content[index_in_dir].is_free = false;
    current_dir->content[index_in_dir].file_size = sizeof("");

    // 在创建文件时，创建好信号量。
    char write[BUFFER_SIZE];
    sprintf(write, "%s%dwrite", current_dir->content[index_in_dir].name, current_dir->content[index_in_dir].base_index);

    char read[BUFFER_SIZE];
    sprintf(read, "%s%dread", current_dir->content[index_in_dir].name, current_dir->content[index_in_dir].base_index);
    sem_open(write, O_CREAT, 0644, 1);
    sem_open(read, O_CREAT, 0644, 0);


    write_to_disk(&current_dir->content[index_in_dir], "");

    write_to_disk(current_fcb, current_dir);
    return true;
}

void detach() {
    int res = shmdt(disk);
    if (res != -1) {
        printf("Successfully detach disk.\n");
    } else {
        printf("There is error detaching disk.\n");
    }
}

bool rm(char **args) {
    if (args[1] == NULL || args[2] != NULL) {
        fprintf(stderr, "Error. Usage: rm file_name.\n");
        return false;
    }
    read_current_dir();
    int index_in_folder = find_index_in_current_dir_by_name(args[1]);
    if (index_in_folder == -1) {
        fprintf(stderr, "Error. %s not exist.\n", args[1]);
        return false;
    }
    if (current_dir->content[index_in_folder].is_dir) {
        fprintf(stderr, "Error. %s is a dir, please use rmdir to remove it.\n", args[1]);
        return false;
    }

    // 在删除文件时，删除掉信号量
    char write[BUFFER_SIZE];
    sprintf(write, "%s%dwrite", current_dir->content[index_in_folder].name, current_dir->content[index_in_folder].base_index);

    char read[BUFFER_SIZE];
    sprintf(read, "%s%dread", current_dir->content[index_in_folder].name, current_dir->content[index_in_folder].base_index);
    sem_unlink(write);
    sem_unlink(read);

    __rm_fcb(&current_dir->content[index_in_folder]);
    current_dir->content[index_in_folder].is_free = true;
    write_to_disk(current_fcb, current_dir);
    return true;
}

bool write_data(char **args) {
    if (args[1] == NULL || args[2] == NULL || args[3] != NULL) {
        fprintf(stderr, "write usage:\n");
        fprintf(stderr, "write filename [a|w]\n");
        fprintf(stderr, "a for [a]ppend, w for [w]rite");
        return false;
    }
    if (strcmp(args[2], "w") != 0 && strcmp(args[2], "a") != 0) {
        fprintf(stderr, "write usage:\n");
        fprintf(stderr, "write filename [a|w]\n");
        fprintf(stderr, "a for [a]ppend, w for [w]rite");
    }
    read_current_dir();
    int index_in_dir = find_index_in_current_dir_by_name(args[1]);
    if (index_in_dir == -1) {
        fprintf(stderr, "%s not found\n", args[1]);
        return false;
    }

    // 写文件，需要信号量同步
    char write[BUFFER_SIZE];
    sprintf(write, "%s%dwrite", current_dir->content[index_in_dir].name, current_dir->content[index_in_dir].base_index);

    char read[BUFFER_SIZE];
    sprintf(read, "%s%dread", current_dir->content[index_in_dir].name, current_dir->content[index_in_dir].base_index);
    sem_open(write,0);
    sem_open(read,0);
    sem_t* write_sem = sem_open(write,0);
    sem_t* read_sem = sem_open(read,0);
    sem_wait(write_sem);
    read_current_dir();
    printf("Please input your data:\n");
    if (strcmp(args[2], "w") == 0) {
        char buffer[BUFFER_SIZE];
        fgets(buffer, BUFFER_SIZE, stdin);
        printf("BUFFER:\n");
        printf("%s", buffer);
        current_dir->content[index_in_dir].file_size = strlen(buffer) * sizeof(char);
        write_to_disk(&current_dir->content[index_in_dir], buffer);
    } else if (strcmp(args[2], "a") == 0) {
        char *buffer = malloc(current_dir->content[index_in_dir].file_size + BUFFER_SIZE);
        char *current_data = (char *) read_from_disk(&current_dir->content[index_in_dir]);
        memcpy(buffer, current_data, current_dir->content[index_in_dir].file_size);
        char input_buffer[BUFFER_SIZE];
        fgets(input_buffer, BUFFER_SIZE, stdin);
        memcpy(buffer + current_dir->content[index_in_dir].file_size, input_buffer, BUFFER_SIZE);
        current_dir->content[index_in_dir].file_size = strlen(buffer) * sizeof(char);
        write_to_disk(&current_dir->content[index_in_dir], buffer);
        free(current_data);
        free(buffer);
    }
    sem_post(write_sem);
    write_to_disk(current_fcb, current_dir);
    return true;
}

bool cat(char **args) {
    if (args[1] == NULL || args[2] != NULL) {
        fprintf(stderr, "Usage: touch filename\n");
        return false;
    }
    read_current_dir();
    int index_in_dir = find_index_in_current_dir_by_name(args[1]);
    if (index_in_dir == -1) {
        fprintf(stderr, "%s not found\n", args[1]);
        return false;
    }

    // 读文件，需要信号量同步
    char write[BUFFER_SIZE];
    sprintf(write, "%s%dwrite", current_dir->content[index_in_dir].name, current_dir->content[index_in_dir].base_index);

    char read[BUFFER_SIZE];
    sprintf(read, "%s%dread", current_dir->content[index_in_dir].name, current_dir->content[index_in_dir].base_index);
    sem_t* write_sem = sem_open(write,0);
    sem_t* read_sem = sem_open(read,0);
    int read_count = -1;

    sem_getvalue(read_sem, &read_count);
    printf("readcount = %d\n", read_count);
    if (read_count > 0) {
        sem_post(read_sem);
    }
    else {
        sem_wait(write_sem);
        sem_post(read_sem);
    }

    read_current_dir();
    sleep(2);
    char *data = (char *) read_from_disk(&current_dir->content[index_in_dir]);
    printf("%s\n", data);
    free(data);

    // 结束时，释放
    sem_getvalue(read_sem, &read_count);
    if (read_count < 1) {
        fprintf(stderr, "Error! in sem, readcount = %d\n", read_count);
    }

    if (read_count == 1) {
        sem_post(write_sem);

    }
    sem_wait(read_sem);
    return true;
}

bool rm_dir(char **args) {

    if (args[1] == NULL || args[2] != NULL) {
        fprintf(stderr, "Error! cd usage: cd dir_name\n");
        return false;
    }
    read_current_dir();
    int content_index = find_index_in_current_dir_by_name(args[1]);
    if (content_index == -1) {
        fprintf(stderr, "Error. %s not found.\n", args[1]);
        return false;
    }
    if (current_dir->content[content_index].is_dir == false) {
        fprintf(stderr, "Error. %s is not a directory.\n", args[1]);
        return false;
    }
    if (is_dir_empty(&current_dir->content[content_index]) == false) {
        fprintf(stderr, "The dir is not empty!\n");
        return false;
    }
    __rm_fcb(&current_dir->content[content_index]);
    current_dir->content[content_index].is_free = true;
    current_dir->content[content_index].base_index = -1;
    current_dir->content[content_index].file_size = 0;
    write_to_disk(current_fcb, current_dir);

    return true;
}

bool is_dir_empty(FCB *fcb) {
    DIR *dir = (DIR *) read_from_disk(fcb);
    if (dir == NULL) {
        return false;
    }
    for (int i = 2; i < MAX_CONTENT_NUM; i++) {
        if (dir->content[i].is_free == false) {
            free(dir);
            return false;
        }
    }
    free(dir);
    return true;
}
bool chname(char **args) {
    if (args[1] == NULL || args[2] == NULL || args[3] != NULL) {
        fprintf(stderr, "Error! chname usage: chname old_name new_name\n");
        return false;
    }
    read_current_dir();
    int content_index = find_index_in_current_dir_by_name(args[1]);
    if (content_index == -1) {
        fprintf(stderr, "Error. %s not found.\n", args[1]);
        return false;
    }

    // 更改文件名，需要信号量同步
    char write[BUFFER_SIZE];
    sprintf(write, "%s%dwrite", current_dir->content[content_index].name, current_dir->content[content_index].base_index);

    char read[BUFFER_SIZE];
    sprintf(read, "%s%dread", current_dir->content[content_index].name, current_dir->content[content_index].base_index);
    sem_t* write_sem = sem_open(write,0);
    sem_t* read_sem = sem_open(read,0);
    sem_wait(write_sem);

    strcpy(current_dir->content[content_index].name, args[2]);
    write_to_disk(current_fcb, current_dir);


    // 为新的文件名创建新的信号量

    sprintf(write, "%s%dwrite", current_dir->content[content_index].name, current_dir->content[content_index].base_index);
    sprintf(read, "%s%dread", current_dir->content[content_index].name, current_dir->content[content_index].base_index);
    sem_open(write, O_CREAT, 0644, 1);
    sem_open(read, O_CREAT, 0644, 0);

    // 释放旧的信号量
    sem_close(write_sem);
    sem_close(read_sem);
    return true;
}