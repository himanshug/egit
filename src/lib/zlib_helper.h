#ifndef ZLIB_HELPER_H_
#define ZLIB_HELPER_H_

#include <stdio.h>

int def(FILE *source, FILE *dest); //deflate
int inf(FILE *source, FILE *dest); //inflate

#endif /* ZLIB_HELPER_H_ */
