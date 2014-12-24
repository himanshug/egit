#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#define OBJ_COMMIT 1
#define OBJ_TREE 2
#define OBJ_BLOB 3
#define OBJ_TAG 4
#define OBJ_OFS_DELTA 5
#define OBJ_REF_DELTA 6

char* calc_sha1(FILE *f);
int def(FILE *source, FILE *dest);
int inf(FILE *source, FILE *dest);

#endif /* CONSTANTS_H_ */
