#ifndef HUFFMAN_H
#define HUFFMAN_H

#define MAX_SYMBOLS 16

typedef struct HuffmanNode {
    char data;
    int freq;
    struct HuffmanNode *left, *right;
} HuffmanNode;

typedef struct {
    char symbols[MAX_SYMBOLS];
    int symbol_count;
    int bits_used;
    int next_state;
} FSMEntry;

typedef struct {
    FSMEntry *table; /* flat array: states x (1 << K) */
    int num_states;
    int K;
} HuffmanFSM;

typedef struct {
    unsigned int bits;
    int length;
} HuffmanCode;

HuffmanNode *huffman_build_tree(unsigned char data[], unsigned int freq[], int size);

void huffman_generate_codes(HuffmanNode *root, HuffmanCode table[256]);

char *huffman_encode(HuffmanNode *root, const char *text, int K);

HuffmanFSM *huffman_build_fsm(HuffmanNode *root, int K);

char *huffman_decode(HuffmanFSM *fsm, const char *encoded, int original_len);

void huffman_free_tree(HuffmanNode *root);

void huffman_free_fsm(HuffmanFSM *fsm);

#endif
