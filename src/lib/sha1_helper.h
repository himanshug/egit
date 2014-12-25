#ifndef SHA1_HELPER_H_
#define SHA1_HELPER_H_

const int SHA1_NUM_BYTES = 20;
const int SHA1_HEX_LEN = 2*SHA1_NUM_BYTES;

/* all the methods here return same buffer, so they should be dup'd by the
 * caller if necessary.
 */
char* calc_sha1(FILE *f);

char *sha1_hex_str_to_bytes(char *sha1);
char *sha1_bytes_to_hex_str(char *bytes);

#endif /* SHA1_HELPER_H_ */
