#ifndef DELTA_H
#define DELTA_H

typedef enum {
    STATE_START,
    STATE_COPY,
    STATE_CHANGE
} DeltaState;

typedef struct {
    DeltaState state;
    int count;     /* run length */
    int run_start; /* start index of CHANGE */
} DeltaMachine;

/**
 * @brief Compute delta between two equal-length strings.
 *
 * Encodes differences using:
 *   =<count>        -> same characters (copy from prev)
 *   +<count><chars> -> changed characters from curr
 *
 * @param prev Previous frame (null-terminated)
 * @param curr Current frame (null-terminated)
 *
 * @return Newly allocated delta string, or NULL on error.
 *
 * @note
 * - prev and curr must have the same length.
 * - Caller must free the returned string.
 */
char *delta_compress(const char *prev, const char *curr);

#endif