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
    DeltaCFSM fsm = {DELTA_CSTATE_START, 0, 0};

    if (!prev || !curr)
        return NULL;

    len_prev = strlen(prev);
    len_curr = strlen(curr);

    if (len_prev != len_curr)
        return NULL;

    output = malloc(len_curr * 4 + 1);

    if (!output)
        return NULL;

    out_idx = 0;

    for (i = 0; i < len_curr; i++) {
        same = (prev[i] == curr[i]);

        switch (fsm.state) {
        case DELTA_CSTATE_START:
            if (same) {
                fsm.state = DELTA_CSTATE_MATCH;
                fsm.run_length = 1;
            } else {
                fsm.state = DELTA_CSTATE_DIFF;
                fsm.run_length = 1;
                fsm.diff_start = i;
            }
            break;
        case DELTA_CSTATE_MATCH:
            if (same) {
                fsm.run_length++;
            } else {
                emit_copy(output, &out_idx, fsm.run_length);

                fsm.state = DELTA_CSTATE_DIFF;
                fsm.run_length = 1;
                fsm.diff_start = i;
            }
            break;
        case DELTA_CSTATE_DIFF:
            if (!same) {
                fsm.run_length++;
            } else {
                emit_change(output, &out_idx, curr, fsm.diff_start, fsm.run_length);

                fsm.state = DELTA_CSTATE_MATCH;
                fsm.run_length = 1;
            }
            break;
        }
    }
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

    output[out_idx] = '\0';
    return output;
}

char *delta_decompress(const char *prev, const char *encoded) {
    int i;
    int len;
    int encoded_idx, out_idx, prev_idx; /* index in encoded, output, prev */
    char *output;
    char c;
    DeltaDFSM fsm = {DELTA_DSTATE_START, 0};

    if (!prev || !encoded)
        return NULL;

    len = strlen(prev);
    output = malloc(len + 1);
    if (!output)
        return NULL;

    encoded_idx = 0;
    out_idx = 0;
    prev_idx = 0;

    while (encoded[encoded_idx]) {
        c = encoded[encoded_idx];

        switch (fsm.state) {
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

        case DELTA_DSTATE_READ_COPY_NUM:
            if ('0' <= c && c <= '9') {
                fsm.run_length = fsm.run_length * 10 + (c - '0');
                encoded_idx++;
            } else {
                fsm.state = DELTA_DSTATE_COPY;
            }
            break;

        case DELTA_DSTATE_READ_INSERT_NUM:
            if ('0' <= c && c <= '9') {
                fsm.run_length = fsm.run_length * 10 + (c - '0');
                encoded_idx++;
            } else {
                fsm.state = DELTA_DSTATE_INSERT;
            }
            break;

        case DELTA_DSTATE_COPY:
            for (i = 0; i < fsm.run_length; i++) {
                output[out_idx++] = prev[prev_idx++];
            }
            fsm.state = DELTA_DSTATE_START;
            break;

        case DELTA_DSTATE_INSERT:
            for (i = 0; i < fsm.run_length; i++) {
                output[out_idx++] = encoded[encoded_idx++];
                prev_idx++;
            }
            fsm.state = DELTA_DSTATE_START;
            break;
        }
    }

    /* Handle case where number ends exactly at end of string */
    if (fsm.state == DELTA_DSTATE_COPY || fsm.state == DELTA_DSTATE_READ_COPY_NUM) {
        for (i = 0; i < fsm.run_length; i++) {
            output[out_idx++] = prev[prev_idx++];
        }
    } else if (fsm.state == DELTA_DSTATE_INSERT || fsm.state == DELTA_DSTATE_READ_INSERT_NUM) {
        /* invalid: missing inserted chars */
        free(output);
        return NULL;
    }

    output[out_idx] = '\0';
    return output;
}
