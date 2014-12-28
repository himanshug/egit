#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "dbg.h"
#include "utils.h"

#include "git-init-db.h"
#include "git-core.h"

#define COMMIT_HASH_LEN 40
#define MAX_REF_NAME_LEN 256

static char buffer[65536];

struct ref_spec {
    char commit[COMMIT_HASH_LEN];

    //on instantiation ref must be pointing to a valid NUL
    //terminated C string
    char ref[MAX_REF_NAME_LEN];

    struct ref_spec* next;
};

/* returns connected socket descriptor or -1 for error */
int connect_to_host(char *host, int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    check_die(fd >= 0, 1,"couldn't create socket");

    struct sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_port = htons(port);

    struct hostent *hostinfo = gethostbyname(host);
    check_die(hostinfo != NULL, 2, "couldn't find server host info");

    name.sin_addr = *((struct in_addr *)hostinfo->h_addr);
    check_die(connect(fd, (struct sockaddr *)&name, sizeof(name)) >= 0, 1,"coundn't bind socket");
    return fd;
}

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
    check_die(first != NULL, 1, "couldn't allocate memory for ref_spec struct");

    memcpy(first->commit, buffer, COMMIT_HASH_LEN);
    strcpy(first->ref, buffer+COMMIT_HASH_LEN+1);

    struct ref_spec* prev = first;
    while((len = read_pkt_line(fd)) > 0) {
        struct ref_spec* curr = malloc(sizeof(struct ref_spec));
        check_die(curr != NULL, 1, "couldn't allocate memory for ref_spec struct");
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
    check_die(pcfile_fd >= 0, 1, "couldn't create packfile");

    int len;
    while((len = read_pkt_line(fd)) > 0) {
        if(buffer[0] == 1) {
            //it is pack file data
            write(pcfile_fd, buffer+1, len-1);
        } else {
            //other information
            dprintf(STD_ERR, "remote: %.*s\n", len, buffer);
        }
    }
    fsync(pcfile_fd);
    close(pcfile_fd);
}

void create_refs(struct ref_spec *refs) {
    mkdir(".git/refs/remotes/origin", S_IRWXU | S_IRWXG);

    struct ref_spec* curr = refs;
    do {
        if(ends_with(curr->ref,"^{}")) continue;

        if(starts_with(curr->ref, "refs/tags")) {
            sprintf(buffer, ".git/%s", curr->ref);
        } else if(starts_with(curr->ref, "refs/heads")) {
            sprintf(buffer, ".git/refs/remotes/origin/%s", curr->ref+10);
        } else continue;

        FILE* f = fopen(buffer, "w+b");
        check_die(f != NULL, 1, "failed to create ref [%s]", buffer);
        fwrite(curr->commit, COMMIT_HASH_LEN, 1, f);
        fwrite("\n", 1, 1, f);
        fflush(f);
        fclose(f);

        if(strcmp(curr->ref, "refs/heads/master") == 0) {
            f = fopen(".git/refs/heads/master", "w+b");
            check_die(f != NULL, 1, "failed to create master ref");
            fwrite(curr->commit, COMMIT_HASH_LEN, 1, f);
            fwrite("\n", 1, 1, f);
            fflush(f);
            fclose(f);
        }
    } while((curr = curr->next) != NULL);
}

int main(int argc, char *argv[]) {
    char *host = "localhost";
    int port = 9418;
    char *repo = "testrepo";

    char *packfile = "/tmp/packfile";

    int fd = connect_to_host(host, port);
    send_proto_request(fd, host, repo);
    struct ref_spec* rs = read_ref_advertisement(fd);
//    send_flush_pkt(fd);
    send_negotiation_request(fd, rs);
    read_pack_file(fd, packfile);
    close(fd);

    //we have got the pack file and ref spec
    check_die(mkdir(repo, S_IRWXU | S_IRWXG) >= 0, 1, "failed to create dir [%s]\n", repo);
    init_db(repo);
    check_die(chdir(repo) == 0, 1, "failed to switch to newly created repo directory");

    unpack_objects(packfile);
    create_refs(rs);
    //TODO : free the ref-spec at this time
    check_out();
}
