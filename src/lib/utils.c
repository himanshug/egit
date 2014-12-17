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
