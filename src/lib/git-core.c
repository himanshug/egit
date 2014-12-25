#include "git-core.h"

#include "dbg.h"
#include "utils.h"

#include "sha1_helper.h"

static char buffer[65536];

void parse_obj_hdr(FILE* f, struct obj_hdr* hdr) {
    int n = fread_till(f, hdr->type, ' ');
    hdr->type[n] = '\0';

    n = fread_till(f, buffer, '\0');
    buffer[n] = '\0';
    hdr->size = atol(buffer);
    check_die(hdr->size > 0, 1, "found obj of negative size");
}

void parse_commit_obj(FILE* f, struct commit_obj* obj) {
    int n = fread_till(f, buffer, ' ');
    check_die(memcmp(buffer, OBJ_TREE_STR, strlen(OBJ_TREE_STR)) == 0, 1,
            "commit points to non-tree obj [%.*s]", n, buffer);
    fread_n(f, obj->tree_sha1, SHA1_HEX_LEN);
}

void parse_tree_obj_entry(FILE* f, struct tree_obj_entry* entry) {
    fread_till(f, entry->mode, ' ');
    int n = fread_till(f, entry->path, '\0');
    entry->path[n] = '\0';

    fread_n(f, buffer, SHA1_NUM_BYTES);
    memcpy(entry->sha1, sha1_bytes_to_hex_str(buffer), SHA1_HEX_LEN);
}

char *sha1_hex_str_to_filename(char *sha1) {
    sprintf(buffer, ".git/objects/%.*s/%.*s", 2, sha1, SHA1_HEX_LEN-2, sha1+2);
    return buffer;
}

void write_file_to_object_db(FILE* f) {
    check_die(0, 1, "TO BE IMPLEMENTED");
}



/* Returns inflated stream */
FILE* open_file_from_object_db(char *sha1) {
    FILE* f = fopen(sha1_hex_str_to_filename(sha1), "rb");
    check_die(f != NULL, 1, "failed read open object-db file [%.*s]", SHA1_HEX_LEN, sha1);

    FILE* tmp = tmpfile();
    check_die(tmp != NULL, 1, "failed to create tmp file");

    //write inflated stream to tmp
    inf(f, tmp);
    fseek(tmp, 0, SEEK_SET);
    return tmp;
}
