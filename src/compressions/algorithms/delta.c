#include "delta.h"
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
    DeltaMachine fsm = {STATE_START, 0, 0};

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
        case STATE_START:
            if (same) {
                fsm.state = STATE_COPY;
                fsm.count = 1;
            } else {
                fsm.state = STATE_CHANGE;
                fsm.count = 1;
                fsm.run_start = i;
            }
            break;
        case STATE_COPY:
            if (same) {
                fsm.count++;
            } else {
                emit_copy(output, &out_idx, fsm.count);

                fsm.state = STATE_CHANGE;
                fsm.count = 1;
                fsm.run_start = i;
            }
            break;
        case STATE_CHANGE:
            if (!same) {
                fsm.count++;
            } else {
                emit_change(output, &out_idx, curr, fsm.run_start, fsm.count);

                fsm.state = STATE_COPY;
                fsm.count = 1;
            }
            break;
        }
    }
    switch (fsm.state) {
    case STATE_COPY:
        emit_copy(output, &out_idx, fsm.count);
        break;
    case STATE_CHANGE:
        emit_change(output, &out_idx, curr, fsm.run_start, fsm.count);
        break;
    default:
        break;
    }

    output[out_idx] = '\0';
    return output;
}