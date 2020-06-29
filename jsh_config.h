//
// Created by Jerry Zhang on 2020-05-09.
//
#ifndef JSH_CONFIG_H
#define JSH_CONFIG_H
typedef struct jsh_configuration {
    char *prompt;
} jsh_configuration;
static jsh_configuration configuration;
char *get_prompt();
#endif