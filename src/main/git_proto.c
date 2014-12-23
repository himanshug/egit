#ifndef GIT_PROTO_C
#define GIT_PROTO_C

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include "dbg.h"
#include "utils.h"
#include "zlib_helper.c"
#include "sha1_helper.c"

#define OBJ_COMMIT 1
#define OBJ_TREE 2
#define OBJ_BLOB 3
#define OBJ_TAG 4
#define OBJ_OFS_DELTA 5
#define OBJ_REF_DELTA 6

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
            write(pcfile_fd, buffer+1, len-1);
        } else {
            //other information
            dprintf(STD_ERR, "remote: %.*s\n", len, buffer);
        }
    }
    fsync(pcfile_fd);
    close(pcfile_fd);
}

uint32_t uint32_from_big_endian(char *buff) {
    uint32_t byte3 = buff[0] << 24;
    uint32_t byte2 = buff[1] << 16;
    uint32_t byte1 = buff[2] << 8;
    uint32_t byte0 = buff[3];
    return (byte0 | byte1 | byte2 | byte3);
}

void parse_pack_file(char *path) {
    debug("parsing packfile %s", path);

    FILE *pf = fopen(path, "rb");
    if(pf == NULL)
        die(1, "couldn't open pack file");

    // packet file header
    fread_n(pf, buffer, 4);
    if(memcmp(buffer, "PACK", 4) != 0)
        die(1, "invalid pack file, 1st 4 bytes are not PACK");

    fseek(pf, 4, SEEK_CUR); //skip packfile version

    fread_n(pf, buffer, 4);
    uint32_t num_objects = uint32_from_big_endian(buffer);
    debug("found %d objects in the pack file", num_objects);

    uint32_t i;
    for(i = 0; i < num_objects; i++) {
        //parse each object
        fread_n(pf, buffer, 1);
        char tmp = buffer[0];

        char is_msb_set = tmp & 0x80;
        char obj_type = (tmp >> 4) & 0x07;
        //CAUTION: this is assuming that git object size can fit inside 32 bits
        //is this assumption correct?
        uint32_t obj_size = (tmp & 0x0f);
        uint32_t shift = 4;
        while(tmp & 0x80) { //if MSB is set
            fread_n(pf, buffer, 1);
            tmp = buffer[0];
            obj_size += (tmp & 0x7f) << shift; //0'd the MSB bit
            shift += 7;
        }
        debug("parsing object of type %d , size %d", obj_type, obj_size);

        FILE* dest;
        FILE* encodedDest;
        dest = tmpfile();
        if(dest == NULL)
            die(1, "couldn't open dest file");

        switch(obj_type) {
        case OBJ_COMMIT :
            sprintf(buffer, "commit %d", obj_size);
        case OBJ_TREE :
            sprintf(buffer, "commit %d", obj_size);
        case OBJ_BLOB :
            sprintf(buffer, "commit %d", obj_size);
        case OBJ_TAG :
            sprintf(buffer, "commit %d", obj_size);

            fwrite(buffer, strlen(buffer), 1, dest);
            fwrite("\0", 1, 1, dest);
            inf(pf, dest);
            fseek(dest, 0, SEEK_SET); //go to beginning of dest stream
            char *hash = calc_sha1(dest);
            printf("sha1 is %.*s\n", COMMIT_HASH_LEN, hash);

            sprintf(buffer, "/tmp/test/o%d", i);
            encodedDest = fopen(buffer, "w+b");
            if(encodedDest == NULL)
                die(1, "couldn't open encodedDest file");

            fseek(dest, 0, SEEK_SET); //go to beginning of dest stream
            def(dest, encodedDest);

            fclose(dest);

            fflush(encodedDest);
            fclose(encodedDest);
            break;
        case OBJ_OFS_DELTA :
        case OBJ_REF_DELTA :
            printf("Its a deltified object");
            exit(1);
            break;
        default:
            fprintf(stderr, "Unknown object type %d\n", obj_type);
            exit(1);
        }


    }
}

#endif
