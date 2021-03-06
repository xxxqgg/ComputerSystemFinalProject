//
// Created by Jerry Zhang on 2020-05-09.
//
#include "jsh_config.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include "SimpleFileSystem/FileSystemAPI.h"

#define BUFF_MAX_SIZE 256

char *get_prompt() {
//    char username[BUFF_MAX_SIZE];

//    getlogin_r(username, sizeof(username));


    char *username = getpwuid(getuid())->pw_name;

    if (configuration.prompt == NULL) {
        configuration.prompt = (char *) malloc(4 * BUFF_MAX_SIZE * sizeof(char));
    }

    strcpy(configuration.prompt, "\033[0;34m");
    strcat(configuration.prompt, username);
    strcat(configuration.prompt, "@");
    strcat(configuration.prompt, get_pwd());
    strcat(configuration.prompt, "(Proudly using JSH)\n> ");
    strcat(configuration.prompt, "\033[0m");
    return configuration.prompt;
}
