#ifndef SHA1_HELPER_C
#define SHA1_HELPER_C

#include <stdlib.h>
#include <openssl/sha.h>
#include <stdint.h>

#include "utils.h"

#define SHA1_DIGEST_LEN 20

#define SHA1_HELPER_BUFFER_LEN 65536

char sha1_helper_buffer[SHA1_HELPER_BUFFER_LEN];

char* calc_sha1(FILE *f) {
    unsigned char md[SHA_DIGEST_LENGTH];
    memset(md, 0, SHA_DIGEST_LENGTH);

    SHA_CTX context;
    SHA1_Init(&context);
    uint32_t len;
    while((len = fread(sha1_helper_buffer, 1, SHA1_HELPER_BUFFER_LEN, f)) > 0) {
        SHA1_Update(&context, sha1_helper_buffer, len);
    }
    SHA1_Final(md, &context);

    char *hashstr = malloc(2*SHA_DIGEST_LENGTH);
    if(hashstr == NULL)
        die(1, "memory error");

    //encode to base 16 and return
    int i;
    for (i = 0; i < SHA_DIGEST_LENGTH; i++) {
         static char hex[] = "0123456789abcdef";
         unsigned int val = md[i];
         char *pos = hashstr + i*2;
         *pos++ = hex[val >> 4];
         *pos = hex[val & 0x0f];
    }
    return hashstr;
}

#endif
