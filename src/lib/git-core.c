#include "git-core.h"

#include "dbg.h"
#include "utils.h"

#include "sha1_helper.h"

const char *OBJ_TYPES[] = { NULL, OBJ_COMMIT_STR, OBJ_TREE_STR, OBJ_BLOB_STR, OBJ_TAG_STR };
const int DIR_MODE = S_IRWXU | S_IRWXG;

static unsigned char buffer[65536];

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

int parse_tree_obj_entry(FILE* f, struct tree_obj_entry* entry) {
    if(fread(entry->mode, 1, 1, f) <= 0 || entry->mode[0] == '\n')
        return -1;

    fread_till(f, entry->mode+1, ' ');
    int n = fread_till(f, entry->path, '\0');
    entry->path[n] = '\0';

    fread_n(f, buffer, SHA1_NUM_BYTES);
    memcpy(entry->sha1_bytes, buffer, SHA1_NUM_BYTES);
    return 0;
}

void unpack_objects(char *packfile) {
    debug("parsing packfile %s", packfile);

    FILE *pf = fopen(packfile, "rb");
    check_die(pf != NULL, 1, "couldn't open pack file %s", packfile);

    /* 12 bytes packet file header */
    /* 4 bytes PACK literal, 4 bytes version, 4 bytes count of objects */
    fread_n(pf, buffer, 4);
    check_die((memcmp(buffer, "PACK", 4) == 0), 1, "invalid pack file, 1st 4 bytes are not PACK");

    fseek(pf, 4, SEEK_CUR); //skip packfile version

    fread_n(pf, buffer, 4);
    uint32_t num_objects = uint32_from_big_endian(buffer);
    debug("found %d objects in the pack file", num_objects);

    /* parse the objects present */
    uint32_t i;
    for(i = 0; i < num_objects; i++) {

        /* object header --> object type and uncompressed object size */
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

        /* Read the deflated object using zlib, inflate it and create the
         * "encoded" object to be stored in .git/objects/info/ */

        //TODO: add support for deltified objects in the packfile
        check_die(obj_type <= OBJ_TAG_NUM, 1, "deltified objects are not supported yet");

        FILE* dest;
        dest = tmpfile();
        check_die(dest != NULL, 1, "couldn't open dest file");

        fprintf(dest,"%s %d", OBJ_TYPES[obj_type], obj_size);
        fwrite("\0", 1, 1, dest);
        inf(pf, dest); //read and inflate object content from pack file to dest stream

        fseek(dest, 0, SEEK_SET); //set read ptr to start of stream
        write_file_to_object_db(dest);
        fclose(dest);
    }
}

struct index_entry* init_index_entry(char *filename, char *sha1_bytes) {
    struct index_entry* ie = malloc(sizeof(struct index_entry));
    check_die(ie != NULL, 1, "failed to allocate mem for index_entry for [%s]", filename);

    struct stat buf;
    check_die(stat(filename, &buf) == 0, 1, "couldn't stat [%s]", filename);
    ie->ctime = buf.st_ctim.tv_sec;
    ie->ctime_ns = buf.st_ctim.tv_nsec;
    ie->mtime = buf.st_mtim.tv_sec;
    ie->mtime_ns = buf.st_mtim.tv_nsec;
    ie->dev = buf.st_dev;
    ie->ino = buf.st_ino;
    ie->mode = buf.st_mode;
    ie->uid = buf.st_uid;
    ie->gid = buf.st_gid;
    ie->fsize = buf.st_size;
    memcpy(ie->sha1, sha1_bytes, SHA1_NUM_BYTES);
    ie->flag = (0x0fff & strlen(filename)); //as per version 2
    ie->path = strdup(filename);
    check_die(ie->path != NULL, 1, "failed to dup path [%s]", filename);
    return ie;
}

void free_index_entry(struct index_entry* e) {
    free(e->path);
    free(e);
}

void write_index(FILE* f, struct index_entry* arr[], size_t narr) {
    //write index header
    fwrite("DIRC", 4, 1, f);

    uint32_t tmp = htonl(2);
    fwrite(&tmp, 4, 1, f);

    tmp = htonl(narr);
    fwrite(&tmp, 4, 1, f);

    //write each index entry
    int i;
    for(i = 0; i < narr; i++) {
        struct index_entry* e = arr[i];
        tmp = htonl(e->ctime);
        fwrite(&tmp, 4, 1, f);

        tmp = htonl(e->ctime_ns);
        fwrite(&tmp, 4, 1, f);

        tmp = htonl(e->mtime);
        fwrite(&tmp, 4, 1, f);

        tmp = htonl(e->mtime_ns);
        fwrite(&tmp, 4, 1, f);

        tmp = htonl(e->dev);
        fwrite(&tmp, 4, 1, f);

        tmp = htonl(e->ino);
        fwrite(&tmp, 4, 1, f);

        tmp = htonl(e->mode);
        fwrite(&tmp, 4, 1, f);

        tmp = htonl(e->uid);
        fwrite(&tmp, 4, 1, f);

        tmp = htonl(e->gid);
        fwrite(&tmp, 4, 1, f);

        tmp = htonl(e->fsize);
        fwrite(&tmp, 4, 1, f);

        fwrite(e->sha1, SHA1_NUM_BYTES, 1, f);

        tmp = htons(e->flag);
        fwrite(&tmp, 2, 1, f);

        fwrite(e->path, strlen(e->path), 1, f);
        int padding_len = 8 - (22 + strlen(e->path))%8;
        memset(buffer, 0, padding_len);
        fwrite(buffer, padding_len, 1, f);
    }

    fseek(f, 0, SEEK_SET); //set pointer to beginning for calculating sha1
    unsigned char *sha1 = calc_sha1(f);
    fwrite(sha1, SHA1_NUM_BYTES, 1, f);
}

int compare_index_entry(struct index_entry** e1, struct index_entry** e2) {
    struct index_entry* p1 = *e1;
    struct index_entry* p2 = *e2;

    int r = strcmp(p1->path, p2->path);
    if(r == 0) {
        uint16_t s1 = p1->flag & 0x3000;
        uint16_t s2 = p2->flag & 0x3000;
        return memcmp(&s1, &s2, 1);
    } else {
        return r;
    }
}

char *sha1_hex_str_to_filename(char *sha1) {
    sprintf(buffer, ".git/objects/%.*s/%.*s", 2, sha1, SHA1_HEX_LEN-2, sha1+2);
    return buffer;
}

void write_file_to_object_db(FILE* f) {
    char *hash = sha1_bytes_to_hex_str(calc_sha1(f));
    sprintf(buffer, ".git/objects/%.*s", 2, hash);
    create_dir_if_not_exists(buffer);

    sprintf(buffer, ".git/objects/%.*s/%.*s", 2, hash, 38, hash+2);
    FILE* deflated_f = fopen(buffer, "w+b");
    check_die(deflated_f != NULL, 1, "failed to create file %s", buffer);

    fseek(f, 0, SEEK_SET);
    def(f, deflated_f);

    fflush(deflated_f);
    fclose(deflated_f);
}

/* Returns inflated stream */
FILE* open_file_from_object_db(char *sha1) {
    char *fname = sha1_hex_str_to_filename(sha1);
    FILE* f = fopen(fname, "rb");
    check_die(f != NULL, 1, "failed to open object-db file [%.*s]", SHA1_HEX_LEN, sha1);

    FILE* tmp = tmpfile();
    check_die(tmp != NULL, 1, "failed to create tmp file");

    //write inflated stream to tmp
    inf(f, tmp);
    fseek(tmp, 0, SEEK_SET);
    return tmp;
}
