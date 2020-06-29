#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

int count_orrurrence(const char *string, char c) {
    int i = 0;
    int len = sizeof(string);
    int count = 0;
    for (i = 0; i < len && string[i] != '\0'; i++) {
        if (string[i] == c) {
            count++;
        }
    }
    return count;
}

void exit_on_failure(int result, char *message) {
    if (result < 0) {
        if (message) {
            perror(message);
        }
        exit(1);
    }
}

int getLine(char *prompt, char *buff, size_t sz) {
    int ch, extra;

    // Get line with buffer overrun protection.
    if (prompt != NULL) {
        printf("%s", prompt);
        fflush(stdout);
    }
    if (fgets(buff, sz, stdin) == NULL)
        return NO_INPUT;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff) - 1] != '\n') {
        extra = 0;
        if (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? TOO_LONG : READLINE_OK;
    }

    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff) - 1] = 0;
    return READLINE_OK;
}