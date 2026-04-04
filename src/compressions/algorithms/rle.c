#include "compressions/algorithms/rle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * TODO: Validate palette does not have any number
 */

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

    RLEMachine fsm = {STATE_START, 0, 0};

    if (!input)
        return NULL;

    if (contains_digit(input)) {
        return NULL;
    }

    len = strlen(input);
    output = malloc(len * 2 + 1); /* worse case is len*2+1 */

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