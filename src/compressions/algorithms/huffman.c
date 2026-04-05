#include "huffman.h"
#include "minheap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static HuffmanNode *create_node(char data, int freq) {
    HuffmanNode *node = malloc(sizeof(HuffmanNode));
    node->data = data;
    node->freq = freq;
    node->left = node->right = NULL;
    return node;
}

static int is_leaf(HuffmanNode *node) {
    return !node->left && !node->right;
}

HuffmanNode *huffman_build(char data[], int freq[], int size) {
    int i;
    HuffmanNode *node, *left, *right, *merged;
    MinHeap *heap = minheap_create(size);

    for (i = 0; i < size; i++) {
        node = create_node(data[i], freq[i]);
        minheap_insert(heap, node, freq[i]);
    }

    while (heap->size > 1) {
        left = (HuffmanNode *)minheap_extract_min(heap).data;
        right = (HuffmanNode *)minheap_extract_min(heap).data;
        merged = create_node('$', left->freq + right->freq);
        merged->left = left;
        merged->right = right;

        minheap_insert(heap, merged, merged->freq);
    }

    node = (HuffmanNode *)minheap_extract_min(heap).data; /* root node */
    minheap_free(heap);

    return node->data;
}

/* TODO */