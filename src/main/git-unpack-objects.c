#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "dbg.h"
#include "utils.h"
#include "constants.h"

#include "sha1_helper.c"
#include "zlib_helper.c"

static char buffer[65536];

void write_to_object_db(FILE* f) {
    char *hash = calc_sha1(f);
    sprintf(buffer, ".git/objects/%.*s", 2, hash);
    create_dir_if_not_exists(buffer);

    sprintf(buffer, ".git/objects/%.*s/%.*s", 2, hash, 38, hash+2);
    FILE* encodedObj = fopen(buffer, "w+b");
    if(encodedObj == NULL)
        die(1, "failed to create object db file");

    fseek(f, 0, SEEK_SET);
    def(f, encodedObj);

    fflush(encodedObj);
    fclose(encodedObj);
}

void parse_pack_file(char *path) {
    debug("parsing packfile %s", path);

    FILE *pf = fopen(path, "rb");
    if(pf == NULL)
        die(1, "couldn't open pack file");

    /* 12 bytes packet file header */
    /* 4 bytes PACK literal, 4 bytes version, 4 bytes count of objects */
    fread_n(pf, buffer, 4);
    if(memcmp(buffer, "PACK", 4) != 0)
        die(1, "invalid pack file, 1st 4 bytes are not PACK");

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
        if(obj_type > OBJ_TAG) {
            die(1, "deltified objects are not supported yet");
        }
        char *object_type_str[] = {"commit", "tree", "blob", "tag"};

        FILE* dest;
        FILE* encodedDest;
        dest = tmpfile();
        if(dest == NULL)
            die(1, "couldn't open dest file");

        fprintf(dest,"%s %d", object_type_str[obj_type-1], obj_size);
        fwrite("\0", 1, 1, dest);
        inf(pf, dest); //read and inflate object content from pack file to dest stream

        fseek(dest, 0, SEEK_SET); //set read ptr to start of stream
        write_to_object_db(dest);
        fclose(dest);
    }
}

int main(int argc, char *argv[]) {
    if(argc < 2)
        fprintf(stderr, "Usage: git-unpack-objects </path/to/packfile>");

    //ensure current directory is valid git repo with an object db
    struct stat buf;
    if(stat(".git/objects", &buf) != 0)
        fprintf(stderr, "Not a valid git repo");
    else
        parse_pack_file(argv[1]);
}
