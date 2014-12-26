#include <stdlib.h>

#include "utils.h"
#include "dbg.h"
#include "git-core.h"

static const int BUFFER_LEN = 65536;
static char buffer[65536]; //CC does not allow using const BUFFER_LEN here

void check_out_tree(char *sha1, char *basedir) {
    debug("check out tree [%.*s]", SHA1_HEX_LEN, sha1);
    FILE* f = open_file_from_object_db(sha1);

    struct obj_hdr hdr;
    parse_obj_hdr(f, &hdr);

    check_die(strcmp(hdr.type, OBJ_TREE_STR) == 0, 1, "not a tree object");

    struct tree_obj_entry te;
    while(parse_tree_obj_entry(f, &te) == 0) {
        sprintf(buffer, "%s/%s", basedir, te.path);
        //TODO: understand why  tree mode on the file is 400000 instead of 0400000
        if(memcmp(te.mode, "400", 3) == 0) { //its a tree
            check_die(mkdir(buffer, DIR_MODE) >= 0, 1, "failed to create dir [%s]", buffer);
            char *newbasedir = strdup(buffer);
            check_die(newbasedir != NULL, 1, "coudn't dup new basedir");
            check_out_tree(te.sha1, newbasedir);
            free(newbasedir);
        } else if(memcmp(te.mode, "100", 3) == 0) { //its a blob
            FILE* src = open_file_from_object_db(te.sha1);
            parse_obj_hdr(src, &hdr);
            check_die(strcmp(hdr.type, OBJ_BLOB_STR) == 0, 1, "not a blob object");

            FILE* dst = fopen(buffer, ends_with(te.mode, "644") ? "wb" : "wb+");
            check_die(dst != NULL, 1, "failed to open file [%s]", te.path);
            int len;
            while((len = fread(buffer, 1, BUFFER_LEN, src)) > 0) {
                fwrite(buffer, len, 1, dst);
            }
            fflush(dst);
            fclose(dst);
        } else {
            check_die(NULL, 1, "mode not supported [%.*s]", TREE_ENTRY_MODE_LEN, te.mode);
        }
    }
}
void check_out_commit(char *sha1) {
    debug("check out commit [%.*s]", SHA1_HEX_LEN, sha1);
    FILE* f = open_file_from_object_db(sha1);

    struct obj_hdr hdr;
    parse_obj_hdr(f, &hdr);
    check_die(strcmp(hdr.type, OBJ_COMMIT_STR) == 0, 1, "not a commit object");

    struct commit_obj commit;
    parse_commit_obj(f, &commit);

    check_out_tree(commit.tree_sha1, ".");
}

char* get_commit_from_ref(char *ref) {
    debug("get_commit_from_ref %s", ref);
    FILE* f = fopen(ref, "rb");
    check_die(f != NULL, 1, "failed to open ref [%s]", ref);

    fread_n(f, buffer, 5);
    if(memcmp(buffer, "ref", 3) == 0) {
        //its a symbolic reference
        memcpy(buffer, ".git/", 5);
        fread_till(f, buffer+5, CHAR_LF);
        return get_commit_from_ref(buffer);
    } else {
        fread_n(f, buffer+5, SHA1_HEX_LEN - 5);
        return buffer;
    }
}

void check_out() {
    char *sha1 = get_commit_from_ref(".git/HEAD");
    check_out_commit(sha1);
}
