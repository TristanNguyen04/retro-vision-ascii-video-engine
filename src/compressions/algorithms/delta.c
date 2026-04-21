#include "compressions/algorithms/delta.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void emit_copy(char *out, int *idx, int count) {
    if (count <= 0)
        return;
    *idx += sprintf(out + *idx, "=%d", count);
}

static void emit_change(char *out, int *idx, const char *curr, int start, int count) {
    int i;
    if (count <= 0)
        return;
    *idx += sprintf(out + *idx, "+%d", count);
    for (i = 0; i < count; i++) {
        out[(*idx)++] = curr[start + i];
    }
}

char *delta_compress(const char *prev, const char *curr) {
    int len_prev, len_curr;
    char *output;
    int out_idx;
    int i, same;

    /* FSM initialization:
       DELTA_CSTATE_START: initial state
       DELTA_CSTATE_MATCH: consecutive characters are the same
       DELTA_CSTATE_DIFF: consecutive characters are different */
    DeltaCFSM fsm = {DELTA_CSTATE_START, 0, 0};

    /*
       Input validation
    */
    if (!prev || !curr)
        return NULL;

    len_prev = strlen(prev);
    len_curr = strlen(curr);

    /* Frames must have the same length */
    if (len_prev != len_curr)
        return NULL;

    /* Allocate output buffer
       Worst case: many small changes -> larger encoded output */
    output = malloc(len_curr * 4 + 1);
    if (!output)
        return NULL;

    out_idx = 0;

    /*
       Step 1: Process both strings character by character
    */
    for (i = 0; i < len_curr; i++) {

        /* Check if current characters match */
        same = (prev[i] == curr[i]);

        switch (fsm.state) {

        /*
           START state:
           - Determine whether to begin MATCH or DIFF run
        */
        case DELTA_CSTATE_START:
            if (same) {
                fsm.state = DELTA_CSTATE_MATCH;
                fsm.run_length = 1;
            } else {
                fsm.state = DELTA_CSTATE_DIFF;
                fsm.run_length = 1;
                fsm.diff_start = i; /* mark start of difference */
            }
            break;

        /*
           MATCH state:
           - Continue counting if characters are the same
           - Otherwise, emit copy operation and switch to DIFF
        */
        case DELTA_CSTATE_MATCH:
            if (same) {
                fsm.run_length++; /* extend matching run */
            } else {
                /* Emit copy instruction (reuse previous frame data) */
                emit_copy(output, &out_idx, fsm.run_length);

                /* Start new difference run */
                fsm.state = DELTA_CSTATE_DIFF;
                fsm.run_length = 1;
                fsm.diff_start = i;
            }
            break;

        /*
           DIFF state:
           - Continue counting if characters differ
           - Otherwise, emit change operation and switch to MATCH
        */
        case DELTA_CSTATE_DIFF:
            if (!same) {
                fsm.run_length++; /* extend difference run */
            } else {
                /* Emit change instruction (store new data) */
                emit_change(output, &out_idx, curr, fsm.diff_start, fsm.run_length);

                /* Start new matching run */
                fsm.state = DELTA_CSTATE_MATCH;
                fsm.run_length = 1;
            }
            break;
        }
    }

    /*
       Step 2: Flush final run after loop ends
    */
    switch (fsm.state) {
    case DELTA_CSTATE_MATCH:
        emit_copy(output, &out_idx, fsm.run_length);
        break;
    case DELTA_CSTATE_DIFF:
        emit_change(output, &out_idx, curr, fsm.diff_start, fsm.run_length);
        break;
    default:
        break;
    }

    /* Null-terminate output string */
    output[out_idx] = '\0';

    return output;
}

char *delta_decompress(const char *prev, const char *encoded) {
    int i;
    int len;
    int encoded_idx, out_idx, prev_idx; /* index in encoded, output, prev */
    char *output;
    char c;

    /* FSM initialization:
       DELTA_DSTATE_START: expect operation ('=' or '+')
       DELTA_DSTATE_READ_COPY_NUM: read number for copy
       DELTA_DSTATE_READ_INSERT_NUM: read number for insert
       DELTA_DSTATE_COPY: copy from previous frame
       DELTA_DSTATE_INSERT: insert new characters */
    DeltaDFSM fsm = {DELTA_DSTATE_START, 0};

    /*
       Input validation
    */
    if (!prev || !encoded)
        return NULL;

    len = strlen(prev);

    /* Allocate output buffer (same length as previous frame) */
    output = malloc(len + 1);
    if (!output)
        return NULL;

    encoded_idx = 0; /* index in encoded string */
    out_idx = 0;     /* index in output */
    prev_idx = 0;    /* index in previous frame */

    /*
       Step 1: Process encoded stream using FSM
    */
    while (encoded[encoded_idx]) {
        c = encoded[encoded_idx];

        switch (fsm.state) {

        /*
           START state:
           - Expect operation symbol:
             '=' -> copy from previous frame
             '+' -> insert new characters
        */
        case DELTA_DSTATE_START:
            if (c == '=') {
                fsm.run_length = 0;
                fsm.state = DELTA_DSTATE_READ_COPY_NUM;
                encoded_idx++;
            } else if (c == '+') {
                fsm.run_length = 0;
                fsm.state = DELTA_DSTATE_READ_INSERT_NUM;
                encoded_idx++;
            } else {
                /* invalid format */
                free(output);
                return NULL;
            }
            break;

        /*
           READ_COPY_NUM state:
           - Read digits to determine copy length
        */
        case DELTA_DSTATE_READ_COPY_NUM:
            if ('0' <= c && c <= '9') {
                fsm.run_length = fsm.run_length * 10 + (c - '0');
                encoded_idx++;
            } else {
                /* Finished reading number -> perform copy */
                fsm.state = DELTA_DSTATE_COPY;
            }
            break;

        /*
           READ_INSERT_NUM state:
           - Read digits to determine insert length
        */
        case DELTA_DSTATE_READ_INSERT_NUM:
            if ('0' <= c && c <= '9') {
                fsm.run_length = fsm.run_length * 10 + (c - '0');
                encoded_idx++;
            } else {
                /* Finished reading number -> perform insert */
                fsm.state = DELTA_DSTATE_INSERT;
            }
            break;

        /*
           COPY state:
           - Copy run_length characters from previous frame
        */
        case DELTA_DSTATE_COPY:
            for (i = 0; i < fsm.run_length; i++) {
                output[out_idx++] = prev[prev_idx++];
            }
            fsm.state = DELTA_DSTATE_START;
            break;

        /*
           INSERT state:
           - Copy run_length characters from encoded stream
           - Advance both output and prev index
        */
        case DELTA_DSTATE_INSERT:
            for (i = 0; i < fsm.run_length; i++) {
                output[out_idx++] = encoded[encoded_idx++];
                prev_idx++;
            }
            fsm.state = DELTA_DSTATE_START;
            break;
        }
    }

    /*
       Step 2: Handle end-of-input edge cases
    */

    /* If ended after reading number -> perform copy */
    if (fsm.state == DELTA_DSTATE_COPY || fsm.state == DELTA_DSTATE_READ_COPY_NUM) {
        for (i = 0; i < fsm.run_length; i++) {
            output[out_idx++] = prev[prev_idx++];
        }

        /* If ended in insert without enough characters -> invalid */
    } else if (fsm.state == DELTA_DSTATE_INSERT || fsm.state == DELTA_DSTATE_READ_INSERT_NUM) {
        free(output);
        return NULL;
    }

    /* Null-terminate output */
    output[out_idx] = '\0';

    return output;
}
