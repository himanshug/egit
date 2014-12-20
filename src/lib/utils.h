#ifndef UTILS_H_
#define UTILS_H_

#include <errno.h>
#include <stdio.h>

#define CHAR_NULL '\0'
#define CHAR_LF '\n'

#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2

void die(int code, char *msg);

/* str utils */
int starts_with(const char *str, const char *prefix);
int ends_with(const char *str, const char *suffix);

/* blocking call, read exactly n chars from file descriptor */
void read_n(const int fd, const void *buff, const int n);

#endif /* UTILS_H_ */
