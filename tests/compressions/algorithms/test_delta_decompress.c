#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../tests_helper.h"
#include "compressions/algorithms/delta.h"

static int run_test(const char *prev, const char *encoded, const char *expected) {
    char *result = delta_decompress(prev, encoded);
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

    if (result != NULL)
        free(result);
    return ok;
}

static int test_basic_copy_only() {
    return run_test("ABCDEFG", "=7", "ABCDEFG");
}

static int test_basic_insert_only() {
    return run_test("ABCDEFG", "+7XXXXXXX", "XXXXXXX");
}

static int test_mixed() {
    return run_test("ABCDEFG", "=2+1X=1+1Y=2", "ABXDYFG");
}

static int test_empty_string() {
    return run_test("", "", "");
}

static int test_single_char_change() {
    return run_test("A", "+1Z", "Z");
}

static int test_single_char_copy() {
    return run_test("A", "=1", "A");
}

static int test_multi_digit_counts() {
    return run_test("AAAAAAAAAA", "=10", "AAAAAAAAAA");
}

static int test_invalid_format() {
    return run_test("ABC", "??", NULL);
}

static int test_null_prev() {
    return run_test("ABC", NULL, NULL);
}

static int test_null_encoded() {
    return run_test(NULL, "=1", NULL);
}

static int test_round_trip() {
    const char *prev = "ABCDEFG";
    const char *curr = "ABXDYFG";
    char *encoded = delta_compress(prev, curr);

    int ok = run_test(prev, encoded, curr);

    free(encoded);
    return ok;
}

int main() {
    test_report("delta uncompress basic_copy", test_basic_copy_only());
    test_report("delta uncompress basic_insert_only", test_basic_insert_only());
    test_report("delta uncompress mixed", test_mixed());
    test_report("delta uncompress empty_string", test_empty_string());
    test_report("delta uncompress single_char_change", test_single_char_change());
    test_report("delta uncompress single_char_copy", test_single_char_copy());
    test_report("delta uncompress multi_digit_counts", test_multi_digit_counts());
    test_report("delta uncompress invalid_format", test_invalid_format());
    test_report("delta uncompress null_prev", test_null_prev());
    test_report("delta uncompress null_encoded", test_null_encoded());
    test_report("delta uncompress round_trip", test_round_trip());

    test_summary();

    return test_failed_count() == 0 ? 0 : 1;
}