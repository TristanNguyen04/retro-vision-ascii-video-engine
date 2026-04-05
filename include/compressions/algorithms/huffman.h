#ifndef HUFFMAN_H
#define HUFFMAN_H

typedef struct {
    char data;
    int freq;
    struct HuffmanNode *left, *right;
} HuffmanNode;

HuffmanNode *huffman_build(char data[], int freq[], int size);

void huffman_generate_codes(HuffmanNode *root);

char *huffman_encode(HuffmanNode *root, const char *text);

char *huffman_decode(HuffmanNode *root, const char *encoded);

void huffman_free_tree(HuffmanNode *root);

#endif