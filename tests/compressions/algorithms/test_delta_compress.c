#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../tests_helper.h"
#include "compressions/algorithms/delta.h"

static int run_test(const char *prev, const char *curr, const char *expected) {
    char *result = delta_compress(prev, curr);
    int ok = 0;

    if (result == NULL && expected == NULL) {
        ok = 1;
    } else if (result == NULL || expected == NULL) {
        ok = 0;
    } else {
        ok = (strcmp(result, expected) == 0);
    }

    if (!ok) {
        printf("FAILED: result:%s, expected:%s \n", result, expected);
    }

    free(result);
    return ok;
}

static int test_all_same() {
    return run_test("AAAAAA", "AAAAAA", "=6");
}

static int test_all_different() {
    return run_test("AAAAAA", "BCDEFG", "+6BCDEFG");
}

static int test_single_change_end() {
    return run_test("AAAABBBB", "AAAABBBC", "=7+1C");
}

static int test_single_change_start() {
    return run_test("AAAABBBB", "CAAABBBB", "+1C=7");
}

static int test_middle_block_change() {
    return run_test("AAAABBBBCCCC", "AAAAXXXXCCCC", "=4+4XXXX=4");
}

static int test_alternating_changes() {
    return run_test("ABCDEF", "ABXDYF", "=2+1X=1+1Y=1");
}

static int test_small_scattered() {
    return run_test("ABCDEFGH", "ABXDEYGH", "=2+1X=2+1Y=2");
}

static int test_empty_strings() {
    return run_test("", "", "");
}

static int test_length_mismatch() {
    return run_test("AAA", "AAAA", NULL);
}

static int test_null_prev() {
    return run_test("AAAA", NULL, NULL);
}

static int test_null_curr() {
    return run_test(NULL, "AAAA", NULL);
}

static int test_ascii_frame_like() {
    const char *prev = "####....####....";
    const char *curr = "####....####...#";

    return run_test(prev, curr, "=15+1#");
}

int main() {
    test_report("delta compress all_same", test_all_same());
    test_report("delta compress all_different", test_all_different());
    test_report("delta compress single_change_end", test_single_change_end());
    test_report("delta compress single_change_start", test_single_change_start());
    test_report("delta compress middle_block_change", test_middle_block_change());
    test_report("delta compress alternating_changes", test_alternating_changes());
    test_report("delta compress small_scattered", test_small_scattered());
    test_report("delta compress empty_strings", test_empty_strings());
    test_report("delta compress length_mismatch", test_length_mismatch());
    test_report("delta compress null_prev", test_null_prev());
    test_report("delta compress null_curr", test_null_curr());
    test_report("delta compress ascii_frame_like", test_ascii_frame_like());

    test_summary();

    return test_failed_count() == 0 ? 0 : 1;
}