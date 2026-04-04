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

/**
 * @brief Compress a string using Run-Length Encoding (RLE).
 *
 * This function encodes consecutive repeated characters in the input string
 * into a compact form of (character, count). For example:
 *
 *   "AAAABBBCCDAA" -> "A4B3C2D1A2"
 *
 * @return A newly allocated null-terminated string containing the RLE-compressed result.
 *         The caller is responsible for freeing the returned string using free().
 *
 */
char *rle_compress(const char *input);

#endif