#include "utils.h"
#include "minunit.h"

#include "git_proto.c"

void* test_read_pkt_line() {
    int p[2];
    if(pipe(p)) {
        die(1, "couldn't create pipe");
    }

    int pid = fork();
    if(pid != 0) {
        /* parent process */
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
    } else {
        /* child process */
        close(p[0]);
        write(p[1], "0006a\n", 6);
        write(p[1], "0005a", 5);
        write(p[1], "000bfoobar\n", 11);
        write(p[1], "0004", 4);

        close(p[1]);
        exit(0);
    }
}

void* test_write_pkt_line() {
    int p[2];
    if(pipe(p)) {
        die(1, "couldn't create pipe");
    }

    int pid = fork();
    if(pid != 0) {
        /* parent process */
        close(p[1]);
        int len;

        len = 6;
        read(p[0], buffer, len);
        mu_assert(!memcmp("0006a\n",buffer,len), "test1 failed");

        len = 5;
        read(p[0], buffer, len);
        mu_assert(!memcmp("0005a",buffer,len), "test2 failed");

        len = 11;
        read(p[0], buffer, len);
        mu_assert(!memcmp("000bfoobar\n",buffer,len), "test3 failed");

        len = 4;
        read(p[0], buffer, len);
        mu_assert(!memcmp("0004",buffer,len), "test4 failed");

        close(p[0]);
    } else {
        /* child process */
        close(p[0]);

        write_pkt_line(p[1], "a\n");
        write_pkt_line(p[1], "a");
        write_pkt_line(p[1], "foobar\n");
        write_pkt_line(p[1], "");

        close(p[1]);
        exit(0);
    }
}
/* add your tests here */
char *all_tests() {
    mu_suite_start();

    mu_run_test(test_read_pkt_line);
    mu_run_test(test_write_pkt_line);

    return NULL;
}

RUN_TESTS(all_tests);
