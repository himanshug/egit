#include <stdlib.h>
#include <stdio.h>

#define BUFFER_LEN 65536

char buffer[BUFFER_LEN];

void read_n_chars(int fd, char *buff, int n) {
    int len = 0;
    while(len != n) {
        int l = read(fd, (buff+len), n-len);
        if(l <= 0)
            die(3, "failed to read file descriptor");
        len += l;
    }
}

int read_pkt_line(int fd) {
    read_n_chars(fd, buffer, 4);
    buffer[4] = '\0';
    int len = strtol(buffer, NULL, 16);

    len -= 4;
    if(len > 0)
        read_n_chars(fd, buffer, len);

    return len;
}

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
