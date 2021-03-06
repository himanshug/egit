#include "git-init-db.h"

#include <stdlib.h>
#include <sys/stat.h>

#include "utils.h"

/* sets up empty git repository by setting following .git folder in [given] directory */

/*
.git
|-- branches/
|-- config
|-- description
|-- HEAD
|-- hooks/
|-- info/
|   `-- exclude
|-- objects
|   |-- info/
|   `-- pack/
`-- refs
    |-- heads/
    |-- remotes/
    `-- tags/

cat .git/HEAD
ref: refs/heads/master
 */

static char buffer[4096];

void create_dirs(char *base, char *paths[], int size) {
    int i;
    for(i = 0; i < size; i++) {
        sprintf(buffer, "%s/%s", base, paths[i]);
        check_die(mkdir(buffer, S_IRWXU | S_IRWXG) >= 0, 1, "failed to create dir [%s]\n", buffer);
    }
}

void create_files(char *base, char *paths[], int size) {
    int i;
    for(i = 0; i < size; i++) {
        sprintf(buffer, "%s/%s", base, paths[i]);
        FILE *f = fopen(buffer, "w+");
        check_die(f != NULL, 1, "failed to create file [%s]\n", buffer);
        fclose(f);
    }
}

/* Assumption: if given, base directory exists already */
void init_db(char *base) {
    char path[(base ? strlen(base) : 0) + 6];
    if(base) {
        sprintf(path, "%s/.git", base);
    } else {
        sprintf(path, ".git");
    }

    struct stat buf;
    if(stat(path,&buf) == 0) {
        fprintf(stderr, "path [%s] already exists.\n", path);
        exit(123);
    }

    char *dirs[] = {"", "branches", "hooks", "info", "objects", "objects/info", "objects/pack", "refs",
            "refs/heads", "refs/remotes", "refs/tags"};
    create_dirs(path, dirs, 11);

    char *files[] = {"config", "description", "HEAD", "info/exclude" };
    create_files(path, files, 4);

    sprintf(buffer, "%s/HEAD", path);
    FILE* head = fopen(buffer,"wb+");
    check_die(head != NULL, 1, "failed to open %s", buffer);
    fprintf(head, "ref: refs/heads/master\n");
    fflush(head);
    fclose(head);
}

/*
int main(int argc, char *argv[]) {
    init_db(argv[1]);
}
*/
