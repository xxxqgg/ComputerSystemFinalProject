#include "builtins.h"
#include <stdio.h>
#include "unistd.h"
#include "SimpleFileSystem/FileSystemAPI.h"


int jsh_exit(char **args) {
    detach();
    exit(0);
}

int help(char **args) {
    printf("%-10s  %s\n", "help", "输出帮助");
    printf("%-10s  %s\n", "exit", "退出系统");
    printf("%-10s  %s\n", "init", "初始化系统，挂载磁盘");
    printf("%-10s  %s\n", "format", "格式化文件系统为 FAT");
    printf("%-10s  %s\n", "ls", "显示当前路径中全部文件");
    printf("%-10s  %s\n", "mkdir dirname", "创建目录");
    printf("%-10s  %s\n", "cd dirname", "前往目录");
    printf("%-10s  %s\n", "touch filename", "创建新的文件");
    printf("%-10s  %s\n", "write filename [a|w]", "写文件，write filename w 代表清空文件中所有数据，从头写入，write filename a 代表以附加的方式写入文件");
    printf("%-10s  %s\n", "rmdir filename", "删除目录，目录必须为空");
    printf("%-10s  %s\n", "cat filename", "输出文件内容");
    printf("%-10s  %s\n", "chname old new", "将 old 文件/文件夹的名字改为 new");
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