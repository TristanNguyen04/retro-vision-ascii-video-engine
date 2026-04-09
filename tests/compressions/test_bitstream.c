#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../tests_helper.h"
#include "compressions/bitstream.h"

static int run_test(const char *bits_in, const char *expected) {
    BitWriter bw;
    BitReader br;
    unsigned int value;
    char result[1024];
    int i, len, ok = 0;

    bitwriter_init(&bw);

    len = strlen(bits_in);
    for (i = 0; i < len; i++) {
        if (bits_in[i] == '1')
            bitwriter_write_bits(&bw, 1, 1);
        else
            bitwriter_write_bits(&bw, 0, 1);
    }

    bitreader_init(&br, bw.data, bw.bit_count);

    for (i = 0; i < len; i++) {
        bitreader_read_bits(&br, 1, &value);
        result[i] = value ? '1' : '0';
    }
    result[len] = '\0';

    ok = strcmp(result, expected) == 0;

    if (!ok) {
        printf("FAIL: in=%s expected=%s got=%s\n", bits_in, expected, result);
    }

    bitwriter_free(&bw);
    return ok;
}

static int test_single_bits() {
    return run_test("101010", "101010");
}

static int test_all_zero() {
    return run_test("00000000", "00000000");
}

static int test_all_one() {
    return run_test("11111111", "11111111");
}

static int test_cross_byte() {
    return run_test("101010101", "101010101"); /* 9 bits */
}

static int test_multiple_bytes() {
    return run_test(
        "110011001010101011110000",
        "110011001010101011110000");
}

static int test_read_chunks() {
    BitWriter bw;
    BitReader br;
    unsigned int out1, out2;
    int ok = 0;

    bitwriter_init(&bw);

    /* Write 1101 0110 */
    bitwriter_write_bits(&bw, 13, 4); /* 0b1101 */
    bitwriter_write_bits(&bw, 6, 4);  /* 0b0110 */

    bitreader_init(&br, bw.data, bw.bit_count);

    bitreader_read_bits(&br, 4, &out1);
    bitreader_read_bits(&br, 4, &out2);

    ok = (out1 == 13 && out2 == 6);

    if (!ok) {
        printf("FAIL: chunk read got %u %u\n", out1, out2);
    }

    bitwriter_free(&bw);
    return ok;
}

static int test_capacity_expand() {
    char input[200];
    char expected[200];
    int i;

    for (i = 0; i < 150; i++) {
        input[i] = (i % 2) ? '1' : '0';
        expected[i] = input[i];
    }
    input[150] = '\0';
    expected[150] = '\0';

    return run_test(input, expected);
}

static int test_empty() {
    return run_test("", "");
}

static int run_test_bytes(const char *text, const char *expected) {
    BitWriter bw;
    BitReader br;
    unsigned int value;
    char result[1024];
    int i, len, ok = 0;

    bitwriter_init(&bw);

    len = strlen(text);

    /* Write each char as 8 bits */
    for (i = 0; i < len; i++) {
        bitwriter_write_bits(&bw, (unsigned char)text[i], 8);
    }

    bitreader_init(&br, bw.data, bw.bit_count);

    for (i = 0; i < len; i++) {
        bitreader_read_bits(&br, 8, &value);
        result[i] = (char)value;
    }
    result[len] = '\0';

    ok = strcmp(result, expected) == 0;

    if (!ok) {
        printf("FAIL: in=%s expected=%s got=%s\n", text, expected, result);
    }

    bitwriter_free(&bw);
    return ok;
}

static int test_ascii_basic() {
    return run_test_bytes("abc", "abc");
}

static int test_ascii_symbols() {
    return run_test_bytes("@@@@@@@@@@@####....@@@@@@", "@@@@@@@@@@@####....@@@@@@");
}

int main() {
    test_report("bitstream single_bits", test_single_bits());
    test_report("bitstream all_zero", test_all_zero());
    test_report("bitstream all_one", test_all_one());
    test_report("bitstream cross_byte", test_cross_byte());
    test_report("bitstream multiple_bytes", test_multiple_bytes());
    test_report("bitstream read_chunks", test_read_chunks());
    test_report("bitstream capacity_expand", test_capacity_expand());
    test_report("bitstream empty", test_empty());
    test_report("bitstream ascii_basic", test_ascii_basic());
    test_report("bitstream ascii_symbol", test_ascii_symbols());

    test_summary();

    return test_failed_count() == 0 ? 0 : 1;
}