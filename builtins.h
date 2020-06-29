#ifndef BUILTINS_H
#define BUILTINS_H

#include "stdbool.h"
#include "stdlib.h"
#include "string.h"

int cd(char **args);
int jsh_exit(char **args);
int help(char **args);
static const char *builtin_str[] = {"cd", "exit", "help"};
static int (* const builtins[]) (char**) = {
        &cd,
        &jsh_exit,
        &help
};
static int num_of_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}
bool is_builtin(char **args);
int exec_builtin(char **args);
#endif