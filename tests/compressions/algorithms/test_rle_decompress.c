#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../tests_helper.h"
#include "compressions/algorithms/rle.h"

static int run_test(const char *encoded, const char *expected) {
    char *result = rle_decompress(encoded);
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

    if (result)
        free(result);
    return ok;
}

static int test_basic(void) {
    return run_test("A4B3C2D1A2", "AAAABBBCCDAA");
}

static int test_single_run(void) {
    return run_test("Z5", "ZZZZZ");
}

static int test_multiple_runs(void) {
    return run_test("A1B1C1D1", "ABCD");
}

static int test_empty(void) {
    return run_test("", "");
}

static int test_multi_digit(void) {
    return run_test("A12", "AAAAAAAAAAAA");
}

static int test_null(void) {
    return run_test(NULL, NULL);
}

static int test_missing_count(void) {
    return run_test("A", NULL);
}

static int test_invalid_start_digit(void) {
    return run_test("4A", NULL);
}

static int test_invalid_no_number(void) {
    return run_test("ABCD", NULL);
}

static int test_zero_count(void) {
    return run_test("A0", NULL);
}

static int test_partial_invalid(void) {
    return run_test("A2B", NULL);
}

static int test_trailing_garbage(void) {
    return run_test("A2B3X", NULL);
}

int main(void) {
    test_report("rle uncompress basic", test_basic());
    test_report("rle uncompress single run", test_single_run());
    test_report("rle uncompress multiple runs", test_multiple_runs());
    test_report("rle uncompress empty", test_empty());
    test_report("rle uncompress multi digit", test_multi_digit());
    test_report("rle uncompress null", test_null());
    test_report("rle uncompress missing count", test_missing_count());
    test_report("rle uncompress invalid start digit", test_invalid_start_digit());
    test_report("rle uncompress invalid no number", test_invalid_no_number());
    test_report("rle uncompress zero count", test_zero_count());
    test_report("rle uncompress partial invalid", test_partial_invalid());
    test_report("rle uncompress trailing garbage", test_trailing_garbage());

    test_summary();

    return test_failed_count() == 0 ? 0 : 1;
}
