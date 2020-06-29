#include "builtins.h"
#include <stdio.h>
#include "unistd.h"

int cd(char **args) {
    if (args[1] == NULL)
        return -1;
    chdir(args[1]);
    return 1;
}

int jsh_exit(char **args) {
    exit(0);
}

int help(char **args) {
    printf("%-10s  %s\n", "help", "输出帮助");
    printf("%-10s  %s\n", "exit", "退出系统");
    printf("%-10s  %s\n", "init", "初始化系统，挂载磁盘");
    printf("%-10s  %s\n", "pwd", "输出当前路径");
    printf("%-10s  %s\n", "ls", "显示当前路径中全部文件");
    printf("%-10s  %s\n", "mkdir", "make directories");
    printf("%-10s  %s\n", "cd", "change working directory");
    printf("%-10s  %s\n", "create", "create new file");
    printf("%-10s  %s\n", "rm", "remove files");
    printf("%-10s  %s\n", "rmdir", "remove directory");
    printf("%-10s  %s\n", "open", "open a file");
    printf("%-10s  %s\n", "close", "close a file");
    printf("%-10s  %s\n", "write", "write to fd");
    printf("%-10s  %s\n", "read", "read from fd");
    return true;
}

bool is_builtin(char **args) {
    if (args == NULL || args[0] == NULL)
        return false;
    for (int i = 0; i < num_of_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0)
            return true;
    }

    return false;
}

int exec_builtin(char **args) {
    if (args == NULL || args[0] == NULL)
        return false;
    for (int i = 0; i < num_of_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return builtins[i](args);
        }
    }
    return false;
}