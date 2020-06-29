//
// Created by Jerry Zhang on 2020-06-29.
//

#ifndef COMPUTERFINALPROJECT_JSH_API_H
#define COMPUTERFINALPROJECT_JSH_API_H
char **jsh_split_line(char *line);

int jsh_read_line(char *buffer, int size);

int jsh_execute(char **args);
#endif //COMPUTERFINALPROJECT_JSH_API_H
