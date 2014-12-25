#include <stdlib.h>

#include "utils.h"
#include "dbg.h"
#include "git-core.h"

static const int BUFFER_LEN = 65536;
static char buffer[65536]; //CC does not allow using const BUFFER_LEN here

void check_out_tree(char *sha1) {
    FILE* f = open_file_from_object_db(sha1);

    struct obj_hdr hdr;
    parse_obj_hdr(f, &hdr);

    check_die(strcmp(hdr.type, OBJ_TREE_STR) == 0, 1, "not a tree object");

    struct tree_obj_entry te;
    parse_tree_obj_entry(f, &te);

    if(starts_with(te.mode, "040")) { //its a tree
        check_die(mkdir(te.path, DIR_MODE) >= 0, 1, "failed to create dir [%s]", te.path);
        check_die(chdir(te.path) == 0, 1, "failed to switch to newly created directory [%s]", te.path);
        check_out_tree(te.sha1);
        check_die(chdir(PARENT_DIR) == 0, 1, "failed to switch to parent directory ");
    } else if(starts_with(te.mode, "100")) { //its a blob
        FILE* src = open_file_from_object_db(te.sha1);
        parse_obj_hdr(src, &hdr);
        check_die(strcmp(hdr.type, OBJ_BLOB_STR) == 0, 1, "not a blob object");

        FILE* dst = fopen(te.path, ends_with(te.mode, "644") ? "rb" : "rb+");
        int len;
        while((len = fread(src, buffer, 1, BUFFER_LEN)) > 0) {
            fwrite(dst, buffer, BUFFER_LEN, 1);
        }
        fflush(dst);
        fclose(dst);
    } else {
        check_die(NULL, 1, "mode not supported [%s]", te.mode);
    }
}
void check_out_commit(char *sha1) {
    FILE* f = open_file_from_object_db(sha1);
    fread_till(f, NULL, CHAR_NULL); //skip the object header
    fread_till(f, buffer, ' ');
    check_die(memcmp(buffer, OBJ_TREE_STR, strlen(OBJ_TREE_STR)) == 0, 1, "commit refers to non tree obj");
    fread_n(f, buffer, SHA1_HEX_LEN);
    check_out_tree(buffer);
}

char* get_commit_from_ref(char *ref) {
    FILE* f;
    if(ref == NULL) {
        f = fopen(".git/HEAD", "rb");
    } else {
        f = fopen(ref, "rb");
    }
    check_die(f != NULL, 1, "failed to open ref");

    fread_n(f, buffer, 5);
    if(memcmp(buffer, "ref", 3) == 0) {
        //its a symbolic reference
        fread_till(f, buffer, CHAR_LF);
        return get_commit_from_ref;
    } else {
        fread_n(f, buffer, SHA1_HEX_LEN - 5);
        return buffer;
    }
}

void check_out() {
    char *sha1 = get_commit_from_ref(".git/HEAD");
    check_out_commit(sha1);
}
