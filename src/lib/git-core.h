#ifndef GIT_CORE_H_
#define GIT_CORE_H_

#include <sys/stat.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "sha1_helper.h"

#define OBJ_COMMIT_NUM 1
#define OBJ_TREE_NUM 2
#define OBJ_BLOB_NUM 3
#define OBJ_TAG_NUM 4
#define OBJ_OFS_DELTA_NUM 5
#define OBJ_REF_DELTA_NUM 6

#define OBJ_COMMIT_STR "commit"
#define OBJ_TREE_STR "tree"
#define OBJ_BLOB_STR "blob"
#define OBJ_TAG_STR "tag"

#define TREE_ENTRY_MODE_LEN 6

#define PARENT_DIR ".."

const char *OBJ_TYPES[];
const int DIR_MODE;


//
//const char *GIT_HOME = ".git";
//const char *GIT_HEAD = ".git/HEAD";

struct obj_hdr {
    char type[32];
    uint32_t size;
};
void parse_obj_hdr(FILE* f, struct obj_hdr* hdr);

struct commit_obj {
    char tree_sha1[SHA1_HEX_LEN];
};
void parse_commit_obj(FILE* f, struct commit_obj* obj);

struct tree_obj_entry {
    char mode[TREE_ENTRY_MODE_LEN];
    unsigned char sha1_bytes[SHA1_NUM_BYTES];
    char path[256];
};
int parse_tree_obj_entry(FILE* f, struct tree_obj_entry* entry);

//This code supports version 2 of the index format.
struct index_entry {
    uint32_t ctime;
    uint32_t ctime_ns;
    uint32_t mtime;
    uint32_t mtime_ns;
    uint32_t dev;
    uint32_t ino;
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint32_t fsize;
    unsigned char sha1[SHA1_NUM_BYTES];
    uint16_t flag;
    char *path;
    struct index_entry* next;
};
struct index_entry* init_index_entry(char *filename, char *sha1_bytes);
void free_index_entry(struct index_entry* e);
int compare_index_entry(struct index_entry** e1, struct index_entry** e2);
void write_index(FILE* f, struct index_entry* arr[], size_t narr);


char *sha1_hex_str_to_filename(char *sha1);
void write_file_to_object_db(FILE* f);
FILE* open_file_from_object_db(char *sha1); //this returns the inflated stream

void check_out();

#endif /* GIT_CORE_H_ */
