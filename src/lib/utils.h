#ifndef UTILS_H_
#define UTILS_H_

#include <errno.h>
#include <stdio.h>
#include <stdint.h>

#include "dbg.h"

#define CHAR_NULL '\0'
#define CHAR_LF '\n'

#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2

uint32_t uint32_from_big_endian(char *buff);

/* str utils */
int starts_with(const char *str, const char *prefix);
int ends_with(const char *str, const char *suffix);

/* blocking call, read exactly n chars from file descriptor */
void read_n(const int fd, const void *buff, const int n);

/* read n bytes from given file stream */
void fread_n(const FILE* f, const void *buff, const int n);

/* read bytes till you see char end */
int fread_till(const FILE* f, const void *buff, const char end);

void create_dir_if_not_exists(char *path);

#endif /* UTILS_H_ */
