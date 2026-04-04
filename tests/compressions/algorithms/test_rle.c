#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../tests_helper.h"
#include "compressions/algorithms/rle.h"

static int run_test(const char *input, const char *expected) {
    char *result = rle_compress(input);
    int ok = 0;

    if (result == NULL && expected == NULL) {
        ok = 1;
    } else if (result == NULL || expected == NULL) {
        ok = 0;
    } else {
        ok = (strcmp(result, expected) == 0);
    }

    free(result);
    return ok;
}

static int test_basic() {
    return run_test("AAAABBBCCDAA", "A4B3C2D1A2");
}

static int test_one_char() {
    return run_test("A", "A1");
}

static int test_no_repetition() {
    return run_test("ABCD", "A1B1C1D1");
}

static int test_all_same() {
    return run_test("AAAAAA", "A6");
}

static int test_empty_string() {
    return run_test("", "");
}

static int test_mixed() {
    return run_test("AAABBA", "A3B2A1");
}

static int test_with_spaces() {
    return run_test("   ", " 3");
}

static int test_symbols() {
    return run_test("!!!@@", "!3@2");
}

static int test_digits() {
    return run_test("123", NULL);
}

static int test_contains_digit() {
    return run_test("A123B", NULL);
}

int main() {
    test_report("rle basic", test_basic());
    test_report("rle one_char", test_one_char());
    test_report("rle no_repetition", test_no_repetition());
    test_report("rle all_same", test_all_same());
    test_report("rle empty_string", test_empty_string());
    test_report("rle mixed", test_mixed());
    test_report("rle spaces", test_with_spaces());
    test_report("rle symbols", test_symbols());
    test_report("rle digits", test_digits());
    test_report("rle contains_digit", test_contains_digit());

    test_summary();

    return test_failed_count() == 0 ? 0 : 1;
}