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

    return node;
}

static int find_code(HuffmanNode *root, char target, char *buffer, int depth) {
    if (!root) return; 

    if (is_leaf(root) && root->data == target) {
        buffer[depth] = '\0';
        return 1;
    }

    buffer[depth] = '0';
    if (find_code(root->left, target, buffer, depth+1)) {
        return 1;
    }

    buffer[depth] = '1';
    if (find_code(root->right, target, buffer, depth+1)) {
        return 1;
    }

    return 0;
}

char *huffman_encode(HuffmanNode *root, const char *text) {
    int capacity = 128;
    int length = 0;
    char *encoded, *tmp;
    char buffer[100];
    int buf_len;
    int i;

    encoded = malloc(capacity);
    if (!encoded)
        return NULL;

    encoded[0] = '\0';

    for (i = 0; text[i] != '\0'; i++) {
        find_code(root, text[i], buffer, 0);

        buf_len = strlen(buffer);

        if (buf_len + length + 1 > capacity) {
            while (buf_len + length + 1 > capacity) 
                capacity *= 2;
            tmp = realloc(encoded, capacity);
            if (!tmp) {
                free(encoded);
                return NULL;
            }
            encoded = tmp;
        }

        memcpy(encoded + length, buffer, buf_len);
        length += buf_len;
        encoded[length] = '\0';
    }

    return encoded;
}

/* TODO: huffman_deocde with FSM */

void huffman_free_tree(HuffmanNode *root) {
    if (!root) 
        return;

    huffman_free_tree(root->left);
    huffman_free_tree(root->right);
    free(root);
}