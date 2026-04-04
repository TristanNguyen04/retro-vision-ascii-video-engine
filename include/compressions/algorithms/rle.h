#ifndef RLE_H
#define RLE_H

typedef enum {
    STATE_START,
    STATE_COUNTING
} RLEState;

typedef struct {
    RLEState state;
    char current_char;
    int count;
} RLEMachine;

char *rle_compress(const char *input);

#endif