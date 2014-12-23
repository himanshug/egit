#include <stdio.h>

#include "zlib_helper.c"

void print_help() {
    fprintf(stderr, "Usage: zlib <inflate/deflate> </path/to/file>");
}

int main(int argc, char *argv[]) {
    if(argc < 3) {
        print_help();
    }

    FILE* f = fopen(argv[2], "rb");
    if(f == NULL) {
        fprintf(stderr, "Failed to open file %s\n", argv[2]);
    }

    if(strcmp(argv[1], "inflate") == 0) {
        fprintf(stderr, "inflating file %s\n", argv[2]);
        inf(f, stdout);
    } else if (strcmp(argv[1], "deflate") == 0) {
        fprintf(stderr, "deflating file %s\n", argv[2]);
        def(f, stdout);
    } else {
        print_help();
    }

    return 0;
}
