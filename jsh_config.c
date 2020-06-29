//
// Created by Jerry Zhang on 2020-05-09.
//
#include "jsh_config.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>

#define BUFF_MAX_SIZE 256

char *get_prompt() {
//    char username[BUFF_MAX_SIZE];
    char current_dir[BUFF_MAX_SIZE];
//    getlogin_r(username, sizeof(username));

    // TODO: change current dir later
    getcwd(current_dir, sizeof(current_dir));
    char *username = getpwuid(getuid())->pw_name;

    if (configuration.prompt == NULL) {
        configuration.prompt = (char*) malloc(4 * BUFF_MAX_SIZE * sizeof(char));
    }

    strcpy(configuration.prompt, "\033[0;34m");
    strcat(configuration.prompt, username);
    strcat(configuration.prompt, "@");
    strcat(configuration.prompt, current_dir);
    strcat(configuration.prompt, "(Proudly using JSH)\n> ");
    strcat(configuration.prompt, "\033[0m");
    return configuration.prompt;
}
