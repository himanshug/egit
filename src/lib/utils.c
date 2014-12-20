#ifndef UTILS_H_
#include "utils.h"
#endif

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
