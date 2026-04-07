#include "compressions/algorithms/rle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void append_encoded(char *output, int *idx, char c, int count) {
    *idx += sprintf(output + *idx, "%c%d", c, count);
}

static int contains_digit(const char *s) {
    while (*s) {
        if ('0' <= *s && *s <= '9') {
            return 1;
        }
        s++;
    }
    return 0;
}

char *rle_compress(const char *input) {
    int i;
    char c;
    int len;
    char *output;
    int out_idx = 0;

    RLECFSM fsm = {STATE_START, 0, 0};

    if (!input)
        return NULL;

    if (contains_digit(input))
        return NULL;

    len = strlen(input);
    output = malloc(len * 2 + 1); /* worse case is len*2+1 */

    if (!output)
        return NULL;

    for (i = 0; i < len; i++) {
        c = input[i];

        switch (fsm.state) {
        case STATE_START:
            fsm.current_char = c;
            fsm.count = 1;
            fsm.state = STATE_COUNTING;
            break;

        case STATE_COUNTING:
            if (c == fsm.current_char) {
                fsm.count++;
            } else {
                append_encoded(output, &out_idx, fsm.current_char, fsm.count);

                fsm.current_char = c;
                fsm.count = 1;
            }
            break;
        }
    }

    if (fsm.state == STATE_COUNTING) {
        append_encoded(output, &out_idx, fsm.current_char, fsm.count);
    }

    output[out_idx] = '\0';
    return output;
}

#include <stdlib.h>

char *rle_decompress(const char *encoded) {

    int i, j, out_idx;
    char c;
    int capacity = 64;
    char *output, *tmp;

    RLEDFSM fsm = {RLE_DSTATE_START, 0};

    if (!encoded)
        return NULL;

    output = malloc(capacity);
    if (!output)
        return NULL;

    i = 0;
    out_idx = 0;

    while (encoded[i]) {
        c = encoded[i];

        switch (fsm.state) {

        case RLE_DSTATE_START:
            if ('0' <= c && c <= '9') {
                free(output);
                return NULL;
            }

            fsm.current_char = c;
            fsm.run_length = 0;
            fsm.state = RLE_DSTATE_READ_NUM;
            i++;
            break;

        case RLE_DSTATE_READ_NUM:
            if ('0' <= c && c <= '9') {
                fsm.run_length = fsm.run_length * 10 + (c - '0');
                i++;
            } else {
                fsm.state = RLE_DSTATE_WRITE;
            }
            break;

        case RLE_DSTATE_WRITE:
            if (fsm.run_length <= 0) {
                free(output);
                return NULL;
            }

            if (out_idx + fsm.run_length >= capacity) {
                capacity = (out_idx + fsm.run_length) * 2;
                tmp = realloc(output, capacity);
                if (!tmp) {
                    free(output);
                    return NULL;
                }
                output = tmp;
            }

            for (j = 0; j < fsm.run_length; j++) {
                output[out_idx++] = fsm.current_char;
            }

            fsm.state = RLE_DSTATE_START;
            break;
        }
    }

    if (fsm.state == RLE_DSTATE_READ_NUM) {
        fsm.state = RLE_DSTATE_WRITE;
    }

    if (fsm.state == RLE_DSTATE_WRITE) {
        if (fsm.run_length <= 0) {
            free(output);
            return NULL;
        }

        if (out_idx + fsm.run_length >= capacity) {
            capacity = (out_idx + fsm.run_length) * 2;
            tmp = realloc(output, capacity);
            if (!tmp) {
                free(output);
                return NULL;
            }
            output = tmp;
        }

        for (j = 0; j < fsm.run_length; j++) {
            output[out_idx++] = fsm.current_char;
        }
    }

    output[out_idx] = '\0';
    return output;
}