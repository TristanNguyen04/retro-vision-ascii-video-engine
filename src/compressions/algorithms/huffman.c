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

HuffmanNode *huffman_build_tree(unsigned char data[], unsigned int freq[], int size) {
    int i;

    /* Pointers for nodes during construction */
    HuffmanNode *node, *left, *right, *merged;

    /* Min-heap used to always extract nodes with smallest frequency */
    MinHeap *heap = minheap_create(size);

    /* Special-case variables for single-character input */
    HuffmanNode *only, *dummy, *root;

    /* Step 1: Insert all characters with non-zero frequency
       into the min-heap as individual leaf nodes */
    for (i = 0; i < size; i++) {
        if (freq[i] <= 0)
            continue; /* skip characters that do not appear */

        node = create_node(data[i], freq[i]); /* create leaf node */
        minheap_insert(heap, node, freq[i]);  /* insert into heap */
    }

    /* Step 2: Handle edge case where only one unique character exists */
    if (heap->size == 1) {
        /* Extract the only node */
        only = (HuffmanNode *)(minheap_extract_min(heap).data);

        /* Create a dummy node to form a valid binary tree */
        dummy = create_node('\0', 0);

        /* Create root node combining the only node and dummy */
        root = create_node('$', only->freq);
        root->left = only;
        root->right = dummy;

        return root;
    }

    /* Step 3: Build Huffman tree using greedy merging */
    while (heap->size > 1) {

        /* Extract two nodes with smallest frequencies */
        left = (HuffmanNode *)(minheap_extract_min(heap).data);
        right = (HuffmanNode *)(minheap_extract_min(heap).data);

        /* Create a new internal node with combined frequency */
        merged = create_node('$', left->freq + right->freq);

        /* Assign children (left/right order does not affect correctness) */
        merged->left = left;
        merged->right = right;

        /* Insert merged node back into heap */
        minheap_insert(heap, merged, merged->freq);
    }

    /* Step 4: Final root extraction */
    if (heap->size == 0)
        return NULL; /* no valid input */

    /* The remaining node in heap is the root of the Huffman tree */
    node = (HuffmanNode *)(minheap_extract_min(heap).data);

    /* Free heap structure (tree nodes are still in use) */
    minheap_free(heap);

    return node;
}

static void generate_codes_rec(HuffmanNode *node, unsigned int bits, int depth, HuffmanCode table[256]) {
    unsigned char c;

    if (!node)
        return;

    /*
       If this is a leaf node:
       - It represents a character
       - Store the accumulated bit sequence and its length
    */
    if (!node->left && !node->right) {
        c = (unsigned char)node->data;

        table[c].bits = bits;    /* Huffman code (bit sequence) */
        table[c].length = depth; /* Number of bits in the code */
        return;
    }

    /*
       Traverse left subtree:
       - Append '0' to the current bit sequence
       - Achieved by shifting bits left by 1
    */
    generate_codes_rec(node->left, bits << 1, depth + 1, table);

    /*
       Traverse right subtree:
       - Append '1' to the current bit sequence
       - Shift left and set least significant bit to 1
    */
    generate_codes_rec(node->right, (bits << 1) | 1, depth + 1, table);
}

void huffman_generate_codes(HuffmanNode *root, HuffmanCode table[256]) {
    int i;

    /*
       Step 1: Initialize the code table
       - Set all entries to empty (no bits, length = 0)
       - Ensures unused characters have no assigned codes
    */
    for (i = 0; i < 256; i++) {
        table[i].bits = 0;
        table[i].length = 0;
    }

    /*
       Step 2: Generate Huffman codes recursively
       - Start from root of the Huffman tree
       - bits = 0 (initial empty code)
       - depth = 0 (no bits yet)
       - Recursion fills table with codes for each character
    */
    generate_codes_rec(root, 0, 0, table);
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

char *huffman_encode(HuffmanNode *root, const char *text, int K) {
    int capacity = 128; /* initial buffer size for encoded string */
    int length = 0;     /* current length of encoded output */
    char *encoded, *tmp;
    char buffer[100]; /* temporary buffer to store code for one character */
    int buf_len;
    int i;
    int remainder, pad;

    /* Allocate initial memory for encoded string */
    encoded = malloc(capacity);
    if (!encoded)
        return NULL;

    encoded[0] = '\0'; /* initialize as empty string */

    /*
       Step 1: Encode each character in input text
    */
    for (i = 0; text[i] != '\0'; i++) {

        /* Find Huffman code (as string of '0' and '1') for character */
        find_code(root, text[i], buffer, 0);

        buf_len = strlen(buffer); /* length of code */

        /* Ensure enough capacity for new bits */
        if (buf_len + length + 1 > capacity) {
            while (buf_len + length + 1 > capacity)
                capacity *= 2; /* grow buffer */

            tmp = realloc(encoded, capacity);
            if (!tmp) {
                free(encoded);
                return NULL;
            }
            encoded = tmp;
        }

        /* Append code to output */
        memcpy(encoded + length, buffer, buf_len);
        length += buf_len;
        encoded[length] = '\0';
    }

    /*
       Step 2: Pad output so total length is a multiple of K
       (required for K-bit FSM decoding)
    */
    remainder = length % K;

    if (remainder != 0) {
        pad = K - remainder; /* number of padding bits needed */

        /* Ensure enough capacity for padding */
        if (length + pad + 1 > capacity) {
            while (length + pad + 1 > capacity)
                capacity *= 2;

            tmp = realloc(encoded, capacity);
            if (!tmp) {
                free(encoded);
                return NULL;
            }
            encoded = tmp;
        }

        /* Add padding bits ('0') */
        for (i = 0; i < pad; i++) {
            encoded[length++] = '0';
        }
    }

    /* Null-terminate final encoded string */
    encoded[length] = '\0';

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
    int num_states = count_nodes(root); /* total number of nodes -> FSM states */
    int table_width = 1 << K;           /* number of possible K-bit inputs (2^K) */
    HuffmanNode **nodes;                /* array mapping state index -> tree node */
    int idx, bits_used, bit, next_state;
    int s, i, b, t; /* iteration vars */
    HuffmanNode *curr;
    FSMEntry *entry;
    HuffmanFSM *fsm;

    /*
       Validate input
    */
    if (K <= 0 || !root)
        return NULL;

    /* Allocate FSM structure */
    fsm = malloc(sizeof(HuffmanFSM));
    if (!fsm)
        return NULL;

    fsm->num_states = num_states;
    fsm->K = K;

    /* Allocate FSM transition table:
       size = num_states × 2^K */
    fsm->table = malloc(sizeof(FSMEntry) * num_states * table_width);
    if (!fsm->table) {
        free(fsm);
        return NULL;
    }

    /*
       Step 1: Map each tree node to a unique FSM state
    */
    nodes = malloc(sizeof(HuffmanNode *) * num_states);
    idx = 0;
    assign_states(root, nodes, &idx); /* fills nodes[] */

    /*
       Step 2: Build FSM transition table
    */
    for (s = 0; s < num_states; s++) {      /* for each state */
        for (i = 0; i < table_width; i++) { /* for each K-bit input */

            curr = nodes[s]; /* start from current state node */
            bits_used = 0;

            entry = &fsm->table[s * table_width + i];
            entry->symbol_count = 0; /* reset output symbols */

            /*
               Simulate traversal using K-bit lookahead
            */
            for (b = 0; b < K; b++) {

                /* Extract next bit from input i (MSB first) */
                bit = (i >> (K - 1 - b)) & 1;

                /* Traverse tree based on bit */
                if (bit == 0)
                    curr = curr->left;
                else
                    curr = curr->right;

                /* Handle invalid/null traversal */
                if (!curr) {
                    bits_used = b + 1; /* consumed bits so far */
                    curr = root;       /* reset to root */
                    break;
                }

                bits_used++;

                /*
                   If a leaf node is reached:
                   - Emit symbol
                   - Reset to root for next decoding
                */
                if (is_leaf(curr)) {
                    if (entry->symbol_count < MAX_SYMBOLS) {
                        entry->symbols[entry->symbol_count++] = curr->data;
                    }
                    curr = root; /* reset after emitting */
                }
            }

            /*
               Step 3: Determine next FSM state
               (based on current node after simulation)
            */
            next_state = 0;
            for (t = 0; t < num_states; t++) {
                if (nodes[t] == curr) {
                    next_state = t;
                    break;
                }
            }

            /* Store transition results */
            entry->bits_used = bits_used;   /* actual bits consumed */
            entry->next_state = next_state; /* resulting state */
        }
    }

    /* Cleanup temporary mapping */
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
char *huffman_decode(HuffmanFSM *fsm, const char *encoded, int original_len) {
    int capacity = 128; /* initial output buffer size */
    int length = 0;     /* number of decoded characters */
    int pos, state, K, table_width, bits;
    int k;
    FSMEntry fsm_entry;

    /* Allocate output buffer */
    char *output = malloc(capacity);
    if (!output)
        return NULL;

    pos = 0;   /* current position in encoded bit string */
    state = 0; /* start from root state */

    K = fsm->K;           /* number of bits used for lookup */
    table_width = 1 << K; /* total possible K-bit inputs */

    /*
       Step 1: Decode until original length is reached
    */
    while (length < original_len) {

        /* Read next K bits (lookahead, not all may be consumed) */
        bits = read_k_bits(encoded, pos, K);

        /* Lookup FSM transition using current state and K-bit input */
        fsm_entry = fsm->table[state * table_width + bits];

        /*
           Step 2: Output decoded symbols (may be multiple)
        */
        for (k = 0; k < fsm_entry.symbol_count; k++) {

            /* Ensure enough capacity for output */
            if (length + 1 >= capacity) {
                capacity *= 2;
                output = realloc(output, capacity);
            }

            /* Append decoded character */
            output[length++] = fsm_entry.symbols[k];

            /* Stop if original length is reached */
            if (length == original_len)
                break;
        }

        /*
           Step 3: Update FSM state and bit position
        */
        state = fsm_entry.next_state; /* move to next state */

        /* Advance by actual bits consumed (may be < K) */
        pos += fsm_entry.bits_used;
    }

    /* Null-terminate final decoded string */
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
