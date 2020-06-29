#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "jsh_config.h"
#include "utils.h"
#include "jsh_api.h"


#define LINE_BUFFER_SIZE 1024


void main_loop();

int main() {
    printf("Hello, World!\n");
    main_loop();
    return 0;
}

void main_loop() {
    char buffer[LINE_BUFFER_SIZE];
    char **args;
    while (true) {
        int read_result = jsh_read_line(buffer, sizeof(buffer));
        if (read_result != READLINE_OK) {
            break;
        }
        args = jsh_split_line(buffer);
        for (int i = 0; args[i] != NULL; i++) {
            printf("%s\n", args[i]);
        }

        free(args);
    }
}