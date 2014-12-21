#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "utils.h"

#define COMMIT_HASH_LEN 40
#define MAX_REF_NAME_LEN 256

#define BUFFER_LEN 65536

char buffer[BUFFER_LEN];

struct ref_spec {
    char commit[COMMIT_HASH_LEN];

    //on instantiation ref must be pointing to a valid NUL
    //terminated C string
    char ref[MAX_REF_NAME_LEN];

    struct ref_spec* next;
};

int read_pkt_line(int fd) {
    read_n(fd, buffer, 4);
    buffer[4] = CHAR_NULL;
    int len = strtol(buffer, NULL, 16);

    len -= 4;
    if(len > 0)
        read_n(fd, buffer, len);

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
    strcpy(first->ref, buffer+COMMIT_HASH_LEN+1);

    struct ref_spec* prev = first;
    while((len = read_pkt_line(fd)) > 0) {
        struct ref_spec* curr = malloc(sizeof(struct ref_spec));
        if(curr == NULL)
            die(1, "couldn't allocate memory for ref_spec struct");
        memcpy(curr->commit, buffer, COMMIT_HASH_LEN);

        if(buffer[len-1] == CHAR_LF)
            buffer[len-1] = CHAR_NULL;
        else
            buffer[len] = CHAR_NULL;
        strcpy(curr->ref, buffer + COMMIT_HASH_LEN + 1);

        prev->next = curr;
        prev = curr;
    }

    return first;
}

int send_flush_pkt(int fd) {
    return dprintf(fd, "0000");
}

void send_negotiation_request(int fd, struct ref_spec* spec) {
    int first = 0;
    struct ref_spec* curr = spec;
    while(curr != NULL) {
        if(!ends_with(curr->ref,"^{}") &&
                (starts_with(curr->ref,"refs/heads/") || starts_with(curr->ref,"refs/tags/"))) {

            if(first == 0) {
                sprintf(buffer, "want %.*s %s\n", COMMIT_HASH_LEN, spec->commit,
                    "multi_ack_detailed side-band-64k thin-pack ofs-delta agent=git/1.8.2");
                first++;
            } else
                sprintf(buffer, "want %.*s\n", COMMIT_HASH_LEN, spec->commit);
                write_pkt_line(fd, buffer);
        }
        curr = curr->next;
    }
    send_flush_pkt(fd);
    sprintf(buffer, "done\n");
    write_pkt_line(fd, buffer);
}

void read_pack_file(int fd, char *path) {
    int pcfile_fd = open(path, O_CREAT | O_WRONLY, S_IRWXU);
    if(pcfile_fd < 0)
        die(1, "couldn't create packfile");

    int len;
    while((len = read_pkt_line(fd)) > 0) {
        if(buffer[0] == 1) {
            //it is pack file data
            write(pcfile_fd, buffer, len);
        } else {
            //other information
            dprintf(STD_ERR, "remote: %.*s\n", len, buffer);
        }
    }
    fsync(pcfile_fd);
    close(pcfile_fd);
}
