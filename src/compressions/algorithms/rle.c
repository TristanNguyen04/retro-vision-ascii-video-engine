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
    int out_idx = 0; /* index for writing into output buffer */

    /* FSM initialization:
       STATE_START: waiting for first character
       STATE_COUNTING: counting consecutive characters */
    RLECFSM fsm = {STATE_START, 0, 0};

    /*
       Input validation
    */
    if (!input)
        return NULL;

    /* Reject input containing digits (to avoid ambiguity in encoding) */
    if (contains_digit(input))
        return NULL;

    len = strlen(input);

    /* Allocate output buffer
       Worst case: every character is unique → "A1B1C1..." (2x size) */
    output = malloc(len * 2 + 1);
    if (!output)
        return NULL;

    /*
       Step 1: Process input using FSM
    */
    for (i = 0; i < len; i++) {
        c = input[i];

        switch (fsm.state) {

        /*
           Initial state:
           - Start a new run with current character
        */
        case STATE_START:
            fsm.current_char = c;
            fsm.count = 1;
            fsm.state = STATE_COUNTING;
            break;

        /*
           Counting state:
           - Continue counting if same character
           - Otherwise, encode previous run and start new one
        */
        case STATE_COUNTING:
            if (c == fsm.current_char) {
                fsm.count++; /* extend current run */
            } else {
                /* Output current run (char + count) */
                append_encoded(output, &out_idx, fsm.current_char, fsm.count);

                /* Start new run */
                fsm.current_char = c;
                fsm.count = 1;
            }
            break;
        }
    }

    /*
       Step 2: Flush the final run after loop ends
    */
    if (fsm.state == STATE_COUNTING) {
        append_encoded(output, &out_idx, fsm.current_char, fsm.count);
    }

    /* Null-terminate output string */
    output[out_idx] = '\0';

    return output;
}

char *rle_decompress(const char *encoded) {

    int i, j, out_idx;
    char c;
    int capacity = 64; /* initial output buffer size */
    char *output, *tmp;

    /* FSM initialization:
       RLE_DSTATE_START: expect character
       RLE_DSTATE_READ_NUM: read run length
       RLE_DSTATE_WRITE: output repeated characters */
    RLEDFSM fsm = {RLE_DSTATE_START, 0};

    /*
       Input validation
    */
    if (!encoded)
        return NULL;

    /* Allocate output buffer */
    output = malloc(capacity);
    if (!output)
        return NULL;

    i = 0;
    out_idx = 0; /* index for writing output */

    /*
       Step 1: Process encoded string using FSM
    */
    while (encoded[i]) {
        c = encoded[i];

        switch (fsm.state) {

        /*
           START state:
           - Expect a character (not a digit)
           - Initialize new run
        */
        case RLE_DSTATE_START:
            if ('0' <= c && c <= '9') {
                /* Invalid format: run cannot start with digit */
                free(output);
                return NULL;
            }

            fsm.current_char = c; /* store character */
            fsm.run_length = 0;   /* reset count */
            fsm.state = RLE_DSTATE_READ_NUM;
            i++;
            break;

        /*
           READ_NUM state:
           - Read digits to build run length
        */
        case RLE_DSTATE_READ_NUM:
            if ('0' <= c && c <= '9') {
                /* Accumulate multi-digit number */
                fsm.run_length = fsm.run_length * 10 + (c - '0');
                i++;
            } else {
                /* Finished reading number → move to write state */
                fsm.state = RLE_DSTATE_WRITE;
            }
            break;

        /*
           WRITE state:
           - Output current character repeated run_length times
        */
        case RLE_DSTATE_WRITE:
            if (fsm.run_length <= 0) {
                /* Invalid run length */
                free(output);
                return NULL;
            }

            /* Ensure enough capacity */
            if (out_idx + fsm.run_length >= capacity) {
                capacity = (out_idx + fsm.run_length) * 2;
                tmp = realloc(output, capacity);
                if (!tmp) {
                    free(output);
                    return NULL;
                }
                output = tmp;
            }

            /* Write repeated characters */
            for (j = 0; j < fsm.run_length; j++) {
                output[out_idx++] = fsm.current_char;
            }

            /* Reset to start state for next run */
            fsm.state = RLE_DSTATE_START;
            break;
        }
    }

    /*
       Step 2: Handle end-of-input edge cases
    */

    /* If input ends while reading number, force write */
    if (fsm.state == RLE_DSTATE_READ_NUM) {
        fsm.state = RLE_DSTATE_WRITE;
    }

    /* If in WRITE state, output final run */
    if (fsm.state == RLE_DSTATE_WRITE) {
        if (fsm.run_length <= 0) {
            free(output);
            return NULL;
        }

        /* Ensure enough capacity */
        if (out_idx + fsm.run_length >= capacity) {
            capacity = (out_idx + fsm.run_length) * 2;
            tmp = realloc(output, capacity);
            if (!tmp) {
                free(output);
                return NULL;
            }
            output = tmp;
        }

        /* Write final run */
        for (j = 0; j < fsm.run_length; j++) {
            output[out_idx++] = fsm.current_char;
        }
    }

    /* Null-terminate output string */
    output[out_idx] = '\0';

    return output;
}