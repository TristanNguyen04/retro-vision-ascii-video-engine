#ifndef DELTA_H
#define DELTA_H

typedef enum {
    DELTA_CSTATE_START,
    DELTA_CSTATE_MATCH,
    DELTA_CSTATE_DIFF
} DeltaCompressState;

typedef struct {
    DeltaCompressState state;
    int run_length; /* length of current run */
    int diff_start; /* start index of diff segment */
} DeltaCFSM;

typedef enum {
    DELTA_DSTATE_START,
    DELTA_DSTATE_READ_COPY_NUM,
    DELTA_DSTATE_READ_INSERT_NUM,
    DELTA_DSTATE_COPY,
    DELTA_DSTATE_INSERT
} DeltaDecompressState;

typedef struct {
    DeltaDecompressState state;
    int run_length; /* parsed number (N in =N or +N) */
} DeltaDFSM;

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

/**
 * @brief Reconstruct current string from delta encoding.
 *
 * Decodes a delta string produced by delta_compress():
 *   =<count>        -> copy <count> characters from prev
 *   +<count><chars> -> insert <count> characters from encoded stream
 *
 * @param prev    Previous frame (null-terminated)
 * @param encoded Delta-encoded string (null-terminated)
 *
 * @return Newly allocated reconstructed string, or NULL on error.
 *
 * @note
 * - The output string will have the same length as prev.
 * - The encoded string must follow the valid delta format.
 * - Caller must free the returned string.
 */
char *delta_decompress(const char *prev, const char *encoded);

#endif