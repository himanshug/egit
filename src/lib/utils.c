#ifndef UTILS_H_
#include "utils.h"
#endif

#include <stdio.h>

void die(int code, char *msg) {
    if(!errno) {
        perror(msg);
    } else {
        printf("Error: %s\n", msg);
    }
    exit(code);
}

/* checks if str starts with prefix */
int starts_with(const char *str, const char *prefix) {
    size_t prefix_len = strlen(prefix);
    size_t str_len = strlen(str);
    return str_len < prefix_len ? 0 : strncmp(prefix, str, prefix_len) == 0;
}

/* checks if str starts with prefix */
int ends_with(const char *str, const char *suffix) {
    size_t suffix_len = strlen(suffix);
    size_t str_len = strlen(str);
    return str_len < suffix_len ? 0 : strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
}

void read_n(const int fd, const void *buff, const int n) {
    int len = 0;
    while(len != n) {
        int l = read(fd, (buff+len), n-len);
        if(l <= 0)
            die(1, "failed to read file descriptor");
        len += l;
    }
}

void fread_n(const FILE* f, const void *buff, const int n) {
    int len = 0;
    while(len != n) {
        int l = fread(buff+len, 1, n-len, f);
        if(l <= 0)
            die(1, "failed to read file stream");
        len += l;
    }
}
