#ifndef GIT_CORE_H_
#define GIT_CORE_H_

#include <sys/stat.h>
#include <stdlib.h>

const int OBJ_COMMIT_NUM=1;
const int OBJ_TREE_NUM=2;
const int OBJ_BLOB_NUM=3;
const int OBJ_TAG_NUM=4;
const int OBJ_OFS_DELTA_NUM=5;
const int OBJ_REF_DELTA_NUM=6;

const char* OBJ_COMMIT_STR = "commit";
const char* OBJ_TREE_STR = "tree";
const char* OBJ_BLOB_STR = "blob";
const char* OBJ_TAG_STR = "tag";

const char *OBJ_TYPES[] = { NULL, OBJ_COMMIT_STR, OBJ_TREE_STR, OBJ_BLOB_STR, OBJ_TAG_STR};

const int DIR_MODE = S_IRWXU | S_IRWXG;

const char *PARENT_DIR = "..";

const char *GIT_HOME = ".git";
const char *GIT_HEAD = ".git/HEAD";

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
    char mode[6];
    char sha1[SHA1_HEX_LEN];
    char path[256];
};
void parse_tree_obj_entry(FILE* f, struct tree_obj_entry* entry);

char *sha1_hex_str_to_filename(char *sha1);
void write_file_to_object_db(FILE* f);
FILE* open_file_from_object_db(char *sha1); //this returns the inflated stream

#endif /* GIT_CORE_H_ */
