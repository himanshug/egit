#ifndef SHA1_HELPER_H_
#define SHA1_HELPER_H_

#include <stdio.h>

#define SHA1_NUM_BYTES 20
#define SHA1_HEX_LEN 40

/* all the methods here return same buffer, so they should be dup'd by the
 * caller if necessary.
 */
unsigned char* calc_sha1(FILE *f);

unsigned char *sha1_hex_str_to_bytes(char *sha1);
char *sha1_bytes_to_hex_str(unsigned char *bytes);

#endif /* SHA1_HELPER_H_ */
