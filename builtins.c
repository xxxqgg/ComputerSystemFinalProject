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
    fprintf(stdout, "HELP:\n");
    fprintf(stdout, "Type command to execute\n");
    fprintf(stdout, "help to display this message:\n");
    fprintf(stdout, "exit to quit the program\n");
    return 1;
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
        return -1;
    for (int i = 0; i < num_of_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return builtins[i](args);
        }
    }
    return -1;
}