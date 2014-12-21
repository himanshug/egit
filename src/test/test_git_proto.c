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

void* test_read_ref_advertisement() {
    int fd = open("res/test/sample_ref_advertisement", O_RDONLY);
    struct ref_spec* curr = read_ref_advertisement(fd);

    mu_assert(!memcmp("3b1031798a00fdf9b574b5857b1721bc4b0e6bac",curr->commit, COMMIT_HASH_LEN), "test11 failed");
    mu_assert(!strcmp("HEAD",curr->ref), "test12 failed");

    curr = curr->next;
    mu_assert(!memcmp("3b1031798a00fdf9b574b5857b1721bc4b0e6bac",curr->commit, COMMIT_HASH_LEN), "test21 failed");
    mu_assert(!strcmp("refs/heads/master",curr->ref), "test22 failed");

    curr = curr->next;
    mu_assert(!memcmp("c4bf7555e2eb4a2b55c7404c742e7e95017ec850",curr->commit, COMMIT_HASH_LEN), "test31 failed");
    mu_assert(!strcmp("refs/remotes/origin/master",curr->ref), "test32 failed");

    curr = curr->next;
    mu_assert(curr == NULL , "test4 failed");
}

/* add your tests here */
char *all_tests() {
    mu_suite_start();

    mu_run_test(test_read_pkt_line);
    mu_run_test(test_write_pkt_line);
    mu_run_test(test_read_ref_advertisement);

    return NULL;
}

RUN_TESTS(all_tests);
