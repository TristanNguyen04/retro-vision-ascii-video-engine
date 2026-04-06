#include "compressions/algorithms/huffman.h"
#include "common/minheap.h"
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
        left = (HuffmanNode *)(minheap_extract_min(heap).data);
        right = (HuffmanNode *)(minheap_extract_min(heap).data);
        merged = create_node('$', left->freq + right->freq);
        merged->left = left;
        merged->right = right;

        minheap_insert(heap, merged, merged->freq);
    }

    node = (HuffmanNode *)(minheap_extract_min(heap).data); /* root node */
    minheap_free(heap);

    return node;
}

static void generate_codes_rec(HuffmanNode *node, char *buffer, int depth) {
    if (!node)
        return;

    if (!node->left && !node->right) {
        buffer[depth] = '\0';
        printf("%c: %s\n", node->data, buffer);
        return;
    }

    buffer[depth] = '0';
    generate_codes_rec(node->left, buffer, depth + 1);

    buffer[depth] = '1';
    generate_codes_rec(node->right, buffer, depth + 1);
}

/* for debug and printing out the codes of each char */
void huffman_generate_codes(HuffmanNode *root) {
    char buffer[256];
    generate_codes_rec(root, buffer, 0);
}

static int find_code(HuffmanNode *root, char target, char *buffer, int depth) {
    if (!root)
        return 0;

    if (is_leaf(root) && root->data == target) {
        buffer[depth] = '\0';
        return 1;
    }

    buffer[depth] = '0';
    if (find_code(root->left, target, buffer, depth + 1)) {
        return 1;
    }

    buffer[depth] = '1';
    if (find_code(root->right, target, buffer, depth + 1)) {
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

static int count_nodes(HuffmanNode *root) {
    if (!root)
        return 0;
    return 1 + count_nodes(root->left) + count_nodes(root->right);
}

static void assign_states(HuffmanNode *root, HuffmanNode **nodes, int *idx) {
    if (!root)
        return;

    nodes[*idx] = root;
    (*idx)++;

    assign_states(root->left, nodes, idx);
    assign_states(root->right, nodes, idx);
}

HuffmanFSM *huffman_build_fsm(HuffmanNode *root, int K) {
    int num_states = count_nodes(root);
    int table_width = 1 << K;
    HuffmanNode **nodes;
    int idx, bits_used, has_symbol, bit, next_state;
    int s, i, b, t; /* iteration vars */
    char symbol;
    HuffmanNode *curr;
    FSMEntry *entry;
    HuffmanFSM *fsm;

    if (K <= 0)
        return NULL;

    fsm = malloc(sizeof(HuffmanFSM));
    if (!fsm)
        return NULL;

    fsm->num_states = num_states;
    fsm->K = K;
    fsm->table = malloc(sizeof(FSMEntry) * num_states * table_width);
    if (!fsm->table) {
        free(fsm);
        return NULL;
    }

    /* Map nodes to states */
    nodes = malloc(sizeof(HuffmanNode *) * num_states);
    idx = 0;
    assign_states(root, nodes, &idx);

    /* Build FSM */
    for (s = 0; s < num_states; s++) {
        for (i = 0; i < table_width; i++) {
            curr = nodes[s];
            bits_used = 0;
            symbol = 0;
            has_symbol = 0;

            for (b = 0; b < K; b++) {
                bit = (i >> (K - 1 - b)) & 1;

                if (bit == 0)
                    curr = curr->left;
                else
                    curr = curr->right;

                if (!curr) {
                    has_symbol = 0;
                    bits_used = b + 1;
                    curr = root;
                    break;
                }

                bits_used++;

                if (is_leaf(curr)) {
                    symbol = curr->data;
                    has_symbol = 1;
                    curr = root; /* reset after emitting */
                    break;
                }
            }

            next_state = 0;
            for (t = 0; t < num_states; t++) {
                if (nodes[t] == curr) {
                    next_state = t;
                    break;
                }
            }

            entry = &fsm->table[s * table_width + i];
            entry->symbol = symbol;
            entry->has_symbol = has_symbol;
            entry->bits_used = bits_used;
            entry->next_state = next_state;
        }
    }
    free(nodes);
    return fsm;
}

static int read_k_bits(const char *encoded, int pos, int K) {
    int val = 0;
    int i;
    for (i = 0; i < K; i++) {
        val <<= 1;
        if (encoded[pos + i] == '1')
            val |= 1;
    }
    return val;
}

/* huffman_deocde with FSM */
char *huffman_decode(HuffmanFSM *fsm, const char *encoded) {
    int capacity = 128;
    int length = 0;
    int pos, encoded_len, state, K, table_width, bits;
    FSMEntry fsm_entry;

    char *output = malloc(capacity);
    if (!output)
        return NULL;

    pos = 0; /* current pos in encoded bit string */
    encoded_len = strlen(encoded);

    state = 0; /* root */
    K = fsm->K;
    table_width = 1 << K;

    while (pos < encoded_len) {
        if (pos + K > encoded_len) {
            /* read 1 bit by 1 bit instead of K bits because the remaining length < K */
            bits = (encoded[pos] == '1') ? 1 : 0;

            fsm_entry = fsm->table[state * table_width + (bits << (K - 1))];

            if (fsm_entry.has_symbol) {
                if (length + 1 >= capacity) {
                    capacity *= 2;
                    output = realloc(output, capacity);
                }
                output[length++] = fsm_entry.symbol;
            }

            state = fsm_entry.next_state;
            pos += fsm_entry.bits_used;
            continue;
        }

        bits = read_k_bits(encoded, pos, K);
        fsm_entry = fsm->table[state * table_width + bits];
        if (fsm_entry.has_symbol) {
            if (length + 1 >= capacity) {
                capacity *= 2;
                output = realloc(output, capacity);
            }
            output[length++] = fsm_entry.symbol;
        }

        state = fsm_entry.next_state;
        pos += fsm_entry.bits_used;
    }

    output[length] = '\0';
    return output;
}

void huffman_free_fsm(HuffmanFSM *fsm) {
    if (!fsm)
        return;
    free(fsm->table);
    free(fsm);
}

void huffman_free_tree(HuffmanNode *root) {
    if (!root)
        return;

    huffman_free_tree(root->left);
    huffman_free_tree(root->right);
    free(root);
}