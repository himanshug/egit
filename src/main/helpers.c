#include <stdlib.h>
#include <stdio.h>

#define BUFFER_LEN 65536

char buffer[BUFFER_LEN];

int write_pkt_line(int fd, char *str) {
    int len = 4 + strlen(str);
    return dprintf(fd, "%04x%s", len, str);
}

int send_proto_request(int fd, char *host, char *repo) {
    sprintf(buffer, "git-upload-pack /%s\0host=%s\0", repo, host);
    return write_pkt_line(fd, buffer);
}

int send_flush_pkt(int fd) {
    return dprintf(fd, "0000");
}
