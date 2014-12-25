#include "sha1_helper.h"

#include <stdlib.h>
#include <openssl/sha.h>
#include <stdint.h>

#include "dbg.h"

const int BUFFER_LEN = 65536;
static char buffer[65536];

char *sha1_bytes_to_hex_str(unsigned char *bytes) {
    //encode to base 16 and return
    int i;
    for (i = 0; i < SHA1_NUM_BYTES; i++) {
         static char hex[] = "0123456789abcdef";
         unsigned int val = 0x00000000 | bytes[i];
         char *pos = buffer + i*2;
         *pos++ = hex[val >> 4];
         *pos = hex[val & 0x0f];
    }
    return buffer;
}

char* calc_sha1(FILE *f) {
    static unsigned char md[SHA1_NUM_BYTES];
    memset(md, 0, SHA1_NUM_BYTES);

    SHA_CTX context;
    SHA1_Init(&context);
    uint32_t len;
    while((len = fread(buffer, 1, BUFFER_LEN, f)) > 0) {
        SHA1_Update(&context, buffer, len);
    }
    SHA1_Final(md, &context);

    return sha1_bytes_to_hex_str(md);
}
