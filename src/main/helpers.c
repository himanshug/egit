#include <stdlib.h>
#include <stdio.h>

#define COMMIT_HASH_LEN 40
#define MAX_REF_NAME_LEN 256

#define BUFFER_LEN 65536

char buffer[BUFFER_LEN];

struct ref_spec {
    char commit[COMMIT_HASH_LEN + 1];
    char ref[MAX_REF_NAME_LEN];
    struct ref_spec *next;
};

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

    //do not consider newline character in the end
    if(buffer[len] == '\n') len -= 1;

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

struct ref_spec* read_ref_advertisement(int fd) {
    int len = read_pkt_line(fd);
    struct ref_spec *first = malloc(sizeof(struct ref_spec));
    if(first == NULL)
        die(1, "couldn't allocate memory for ref_spec struct");

    memcpy(first->commit, buffer, COMMIT_HASH_LEN);
    *(first->commit+COMMIT_HASH_LEN) = '\0';
    int i;
    char *p = buffer + COMMIT_HASH_LEN + 1; //last 1 is for the SPACE char
    for(i = 0;;i++) {
        char c = *(p+i);
        if(c == '\0')
            break;
        *((first->ref)+i) = c;
    }
    *((first->ref)+i) = '\0';

    struct ref_spec* prev = first;
    while((len = read_pkt_line(fd)) > 0) {
        struct ref_spec* curr = (struct ref_spec*)malloc(sizeof(struct ref_spec));
        if(curr == NULL)
            die(1, "couldn't allocate memory for ref_spec struct");
        memcpy(curr->commit, buffer, COMMIT_HASH_LEN);
        *(curr->commit+COMMIT_HASH_LEN) = '\0';
        memcpy(curr->ref, buffer + COMMIT_HASH_LEN + 1, len - COMMIT_HASH_LEN - 1);
         *(curr->ref+(len - COMMIT_HASH_LEN - 1)) = '\0';

        prev->next = curr;
        prev = curr;
    }
    return first;
}

int send_flush_pkt(int fd) {
    return dprintf(fd, "0000");
}
