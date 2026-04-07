#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../tests_helper.h"
#include "compressions/algorithms/huffman.h"

static int run_test(const char *input, int K) {
    char data[] = {'A', 'B', 'C', 'D', 'E', 'F'};
    int freq[] = {5, 9, 12, 13, 16, 45};
    int size = 6;
    HuffmanNode *root;
    HuffmanFSM *fsm;
    char *encoded, *decoded;
    int ok;

    root = huffman_build_tree(data, freq, size);
    /* huffman_generate_codes(root); */
    fsm = huffman_build_fsm(root, K);
    if (!fsm) {
        huffman_free_fsm(fsm);
        huffman_free_tree(root);
        return 0;
    }

    encoded = huffman_encode(root, input, K);
    decoded = huffman_decode(fsm, encoded, strlen(input));
    ok = (decoded && strcmp(decoded, input) == 0);

    if (!ok) {
        printf("FAILED: input:%s decoded:%s\n", input, decoded);
    }

    free(encoded);
    free(decoded);
    huffman_free_fsm(fsm);
    huffman_free_tree(root);

    return ok;
}

static int test_basic() {
    return run_test("ABCDEF", 4);
}

static int test_repeated() {
    return run_test("AAAAAA", 4);
}

static int test_mixed() {
    return run_test("ABACABADABACABA", 4);
}

static int test_single_char() {
    return run_test("A", 4);
}

static int test_empty() {
    return run_test("", 4);
}

static int test_long() {
    return run_test("ABCDEFABCDEFABCDEFABCDEFABCDEF", 4);
}

static int test_diff_k() {
    return run_test("ABCDEFABCDEFABCDEFABCDEFABCDEF", 1) && run_test("ABCDEFABCDEFABCDEFABCDEFABCDEF", 4) && run_test("ABCDEFABCDEFABCDEFABCDEFABCDEF", 5);
}

int main() {
    test_report("huffman basic", test_basic());
    test_report("huffman repeated", test_repeated());
    test_report("huffman mixed", test_mixed());
    test_report("huffman single", test_single_char());
    test_report("huffman empty", test_empty());
    test_report("huffman long", test_long());
    test_report("huffman different k", test_diff_k());

    test_summary();
    return test_failed_count() == 0 ? 0 : 1;
}