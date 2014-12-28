#include <stdlib.h>

#include "utils.h"
#include "dbg.h"
#include "git-core.h"

static const int BUFFER_LEN = 65536;
static char buffer[65536]; //CC does not allow using const BUFFER_LEN here

static struct index_entry* index_entry_head;
static struct index_entry* index_entry_curr;
static size_t num_index_entries = 0;

void check_out_tree(char *sha1, char *basedir) {
    debug("check out tree [%.*s]", SHA1_HEX_LEN, sha1);
    FILE* f = open_file_from_object_db(sha1);

    struct obj_hdr hdr;
    parse_obj_hdr(f, &hdr);

    check_die(strcmp(hdr.type, OBJ_TREE_STR) == 0, 1, "not a tree object");

    struct tree_obj_entry te;
    while(parse_tree_obj_entry(f, &te) == 0) {
        if(basedir != NULL)
            sprintf(buffer, "%s/%s", basedir, te.path);
        else
            sprintf(buffer, "%s", te.path);
        //TODO: understand why  tree mode on the file is 400000 instead of 0400000
        if(memcmp(te.mode, "400", 3) == 0) { //its a tree
            check_die(mkdir(buffer, DIR_MODE) >= 0, 1, "failed to create dir [%s]", buffer);
            char *newbasedir = strdup(buffer);
            check_die(newbasedir != NULL, 1, "coudn't dup new basedir");
            check_out_tree(sha1_bytes_to_hex_str(te.sha1_bytes), newbasedir);
            free(newbasedir);
        } else if(memcmp(te.mode, "100", 3) == 0) { //its a blob
            FILE* src = open_file_from_object_db(sha1_bytes_to_hex_str(te.sha1_bytes));
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

            //create index entry
            if(basedir != NULL)
                sprintf(buffer, "%s/%s", basedir, te.path);
            else
                sprintf(buffer, "%s", te.path);
            num_index_entries++;
            if(index_entry_head == NULL) {
                index_entry_head = init_index_entry(buffer, te.sha1_bytes);
                index_entry_curr = index_entry_head;
            } else {
                index_entry_curr->next = init_index_entry(buffer, te.sha1_bytes);
                index_entry_curr = index_entry_curr->next;
            }
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

    check_out_tree(commit.tree_sha1, NULL);
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

void check_out(char *ref) {
    char *sha1 = get_commit_from_ref(ref);
    check_out_commit(sha1);

    struct index_entry* entries[num_index_entries];
    int i;

    struct index_entry* ie = index_entry_head;
    for(i = 0; i < num_index_entries; i++) {
        check_die(ie != NULL, 1, "found null index entry");
        entries[i] = ie;
        ie = ie->next;
    }

    qsort(entries, num_index_entries, sizeof(struct index_entry*), compare_index_entry);
    FILE* indexf = fopen(".git/index", "wb+");
    check_die(indexf != NULL, 1, "failed to open index file");
    write_index(indexf, entries, num_index_entries);
    fflush(indexf);
    fclose(indexf);
}
