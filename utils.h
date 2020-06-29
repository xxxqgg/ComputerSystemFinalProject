#ifndef UTILS_H
#define UTILS_H
int count_orrurrence(const char *string, char c);
void exit_on_failure(int result, char *message);
int getLine (char *prompt, char *buff, size_t sz);
// Macro for readline result
#define READLINE_OK       0
#define NO_INPUT 1
#define TOO_LONG 2

#endif