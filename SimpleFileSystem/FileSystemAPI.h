//
// Created by Jerry Zhang on 2020-06-30.
//

#ifndef COMPUTERFINALPROJECT_FILESYSTEMAPI_H
#define COMPUTERFINALPROJECT_FILESYSTEMAPI_H

#include <stdbool.h>

bool init_file_system(char **args);
bool format_disk(char **args);
bool mkdir(char **args);
bool ls(char **args);
static const char *api_str[] = {
        "init",
        "format",
        "mkdir",
        "ls",

};
static bool (* const apis[]) (char**) = {
        &init_file_system,
        &format_disk,
        &mkdir,
        &ls,
};
static int num_of_api() {
    return sizeof(api_str) / sizeof(char *);
}
bool is_file_system_api(char **args);

int exec_api(char **args);
char *get_pwd();

#endif //COMPUTERFINALPROJECT_FILESYSTEMAPI_H
