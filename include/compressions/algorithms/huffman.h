#ifndef HUFFMAN_H
#define HUFFMAN_H

typedef struct HuffmanNode {
    char data;
    int freq;
    struct HuffmanNode *left, *right;
} HuffmanNode;

typedef struct {
    char symbol;
    int has_symbol;
    int bits_used;
    int next_state;
} FSMEntry;

typedef struct {
    FSMEntry *table; /* flat array: states x (1 << K) */
    int num_states;
    int K;
} HuffmanFSM;

HuffmanNode *huffman_build(char data[], int freq[], int size);

void huffman_generate_codes(HuffmanNode *root);

char *huffman_encode(HuffmanNode *root, const char *text);

HuffmanFSM *huffman_build_fsm(HuffmanNode *root, int K);

char *huffman_decode(HuffmanFSM *fsm, const char *encoded);

void huffman_free_tree(HuffmanNode *root);

void huffman_free_fsm(HuffmanFSM *fsm);

#endif