#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../tests_helper.h"
#include "compressions/algorithms/huffman.h"

static void compute_freq(const char *input,
                         unsigned char data[],
                         unsigned int freq[],
                         int *size) {

    unsigned int count[256] = {0};
    int i, idx = 0;

    for (i = 0; input[i]; i++) {
        count[(unsigned char)input[i]]++;
    }

    for (i = 0; i < 256; i++) {
        if (count[i] > 0) {
            data[idx] = (unsigned char)i;
            freq[idx] = count[i];
            idx++;
        }
    }

    *size = idx;
}

void debug_print_fsm(const HuffmanFSM *fsm) {
    int s, i, k;
    int table_width = 1 << fsm->K;

    printf("\n=== FSM DEBUG ===\n");

    for (s = 0; s < fsm->num_states; s++) {
        printf("STATE %d:\n", s);

        for (i = 0; i < table_width; i++) {
            FSMEntry *e = &fsm->table[s * table_width + i];

            printf("  input=%d -> next=%d, bits_used=%d, symbols=",
                   i, e->next_state, e->bits_used);

            if (e->symbol_count == 0) {
                printf("[]");
            } else {
                printf("[");
                for (k = 0; k < e->symbol_count; k++) {
                    printf("%c", e->symbols[k]);
                }
                printf("]");
            }

            printf("\n");
        }
    }
}

void print_huffman_tree(HuffmanNode *root, int depth) {
    int i;
    if (!root)
        return;

    /* print right subtree first */
    print_huffman_tree(root->right, depth + 1);

    /* indentation */
    for (i = 0; i < depth; i++) {
        printf("    ");
    }

    /* print node */
    if (!root->left && !root->right) {
        /* leaf node */
        if (root->data == '\n')
            printf("(\\n:%d)\n", root->freq);
        else
            printf("('%c':%d)\n", root->data, root->freq);
    } else {
        /* internal node */
        printf("(*:%d)\n", root->freq);
    }

    /* print left subtree */
    print_huffman_tree(root->left, depth + 1);
}

static int run_test(const char *input, int K) {
    HuffmanNode *root;
    HuffmanFSM *fsm;
    char *encoded, *decoded;
    int ok;

    unsigned char data[256];
    unsigned int freq[256];
    int size;

    compute_freq(input, data, freq, &size);

    root = huffman_build_tree(data, freq, size);
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

    printf("done\n");

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

/* static int test_empty() {
    return run_test("", 4);
} */

static int test_long() {
    return run_test("ABCDEFABCDEFABCDEFABCDEFABCDEF", 4);
}

static int test_symbols() {
    return run_test("@@@@##@@@.", 4);
}

static int test_diff_k() {
    return run_test("ABCDEFABCDEFABCDEFABCDEFABCDEF", 1) && run_test("ABCDEFABCDEFABCDEFABCDEFABCDEF", 4) && run_test("ABCDEFABCDEFABCDEFABCDEFABCDEF", 5);
}

int main() {
    test_report("huffman basic", test_basic());
    test_report("huffman repeated", test_repeated());
    test_report("huffman mixed", test_mixed());
    test_report("huffman single", test_single_char());
    /* test_report("huffman empty", test_empty()); */
    test_report("huffman long", test_long());
    test_report("huffman symbols", test_symbols());
    test_report("huffman different k", test_diff_k());

    test_summary();
    return test_failed_count() == 0 ? 0 : 1;
}