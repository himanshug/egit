#ifndef UTILS_H_
#include "utils.h"
#endif

#include <stdio.h>
#include <sys/stat.h>

/*
void die(int code, char *msg) {
    if(!errno) {
        perror(msg);
    } else {
        printf("Error: %s\n", msg);
    }
    exit(code);
}
*/

uint32_t uint32_from_big_endian(char *buff) {
    uint32_t byte3 = buff[0] << 24;
    uint32_t byte2 = buff[1] << 16;
    uint32_t byte1 = buff[2] << 8;
    uint32_t byte0 = buff[3];
    return (byte0 | byte1 | byte2 | byte3);
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
        check_die(l > 0, 1, "failed to read file descriptor");
        len += l;
    }
}

void fread_n(const FILE* f, const void *buff, const int n) {
    int len = 0;
    while(len != n) {
        int l = fread(buff+len, 1, n-len, f);
        check_die(l > 0, 1, "failed to read file stream");
        len += l;
    }
}

int fread_till(const FILE* f, const void *buff, const char end) {
    unsigned char ch;
    int i;
    for(i = 0; ; i++) {
        ch = getc(f);
        check_die(ch != EOF, 1, "premature end of file");
        if(ch == end) break;
        memcpy(buff+i, &ch, 1);
    }
    return i;
}

void create_dir_if_not_exists(char *path) {
    struct stat buf;
    if(stat(path,&buf) != 0 && mkdir(path, S_IRWXU | S_IRWXG) < 0) {
        fprintf(stderr, "failed to create dir[%s].\n", path);
        exit(123);
    }
}
