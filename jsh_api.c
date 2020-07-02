//
// Created by Jerry Zhang on 2020-06-29.
//
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jsh_api.h"
#include "utils.h"
#include "jsh_config.h"
#include "builtins.h"
#include "SimpleFileSystem/FileSystemAPI.h"

int jsh_read_line(char *buffer, int size) {
    return getLine(get_prompt(), buffer, size);
}

char **jsh_split_line(char *line) {
    char *separator = " ";
    int num_of_token = count_orrurrence(line, separator[0]) + 2;

    char **res = malloc(sizeof(char *) * num_of_token);
    char *token = strtok(line, separator);
    int index = 0;
    while (token != NULL) {
        res[index++] = token;
        token = strtok(NULL, separator);
    }
    res[index] = NULL;

    return res;
}

int jsh_execute(char **args) {
    if (args == NULL || args[0] == NULL)
        return true;
    if (is_builtin(args)) {
        return exec_builtin(args);
    }
    if (is_file_system_api(args)) {
        return exec_api(args);
    }
    return true;
}
