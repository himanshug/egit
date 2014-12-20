#include "utils.h"
#include "minunit.h"

#include "git_proto.c"

int test_read_pkt_line() {
    int p[2];
    if(pipe(p)) {
        die(1, "couldn't create pipe");
    }

    int pid = fork();
    if(pid == 0) {
        /* child process */
        close(p[1]);
        int len;

        len = read_pkt_line(p[0]);
        mu_assert(!memcmp("a\n",buffer,2), "test failed");

        len = read_pkt_line(p[0]);
        mu_assert(!memcmp("a",buffer,1), "test failed");

        len = read_pkt_line(p[0]);
        mu_assert(!memcmp("foobar\n",buffer,7), "test failed");

        len = read_pkt_line(p[0]);
        mu_assert(len == 0, "test failed");

        close(p[0]);
        exit(0);
    } else {
        /* parent process */
        close(p[0]);
        write(p[1], "0006a\n", 6);
        write(p[1], "0005a", 5);
        write(p[1], "000bfoobar\n", 11);
        write(p[1], "0004", 4);

        close(p[1]);
    }

    return 0;
}

/* add your tests here */
char *all_tests() {
    mu_suite_start();

    mu_run_test(test_read_pkt_line);

    return NULL;
}

RUN_TESTS(all_tests);
