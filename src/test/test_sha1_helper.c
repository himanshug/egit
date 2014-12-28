#include "sha1_helper.h"

#include "minunit.h"

void* test_calc_sha1() {
    FILE* f = fopen("res/test/do_not_modify", "rb");
    char *sha1 = sha1_bytes_to_hex_str(calc_sha1(f));
    mu_assert(memcmp(sha1, "6c7752f24e468ac8d605f1dce2fd41acc7ae91ec", SHA1_HEX_LEN) == 0,
            "sha1 dint match");
}

/* add your tests here */
char *all_tests() {
    mu_suite_start();

    mu_run_test(test_calc_sha1);

    return NULL;
}

RUN_TESTS(all_tests);
