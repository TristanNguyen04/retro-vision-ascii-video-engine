#ifndef RLE_H
#define RLE_H

typedef enum {
    STATE_START,
    STATE_COUNTING
} RLECompressionState;

typedef struct {
    RLECompressionState state;
    char current_char;
    int count;
} RLECFSM;

typedef enum {
    RLE_DSTATE_START,
    RLE_DSTATE_READ_NUM,
    RLE_DSTATE_WRITE
} RLEDecompressState;

typedef struct {
    RLEDecompressState state;
    char current_char;
    int run_length;
} RLEDFSM;

/**
 * @brief Compress a string using Run-Length Encoding (RLE).
 *
 * This function encodes consecutive repeated characters in the input string
 * into a compact form of (character, count). For example:
 *
 *   "AAAABBBCCDAA" -> "A4B3C2D1A2"
 *
 * @return A newly allocated null-terminated string containing the RLE-compressed result,
 *         or NULL if an error occurs (e.g., input is NULL, contains invalid characters).
 *
 * @note
 * - The input string should not contain any numeric digit.
 * - Caller must free the returned string.
 */
char *rle_compress(const char *input);

/**
 * @brief Decompress a Run-Length Encoded (RLE) string.
 *
 * This function decodes a string encoded in (character, count) format
 * back into its original form. For example:
 *
 *   "A4B3C2D1A2" -> "AAAABBBCCDAA"
 *
 * @param encoded RLE-compressed string (null-terminated)
 *
 * @return A newly allocated null-terminated string containing the decompressed result,
 *         or NULL if an error occurs (e.g., input is NULL, invalid format, allocation failure).
 *
 * @note
 * - The encoded string must follow the format: <char><count>, where:
 *     - <char> is a non-digit character
 *     - <count> is a positive integer (can be multi-digit)
 * - Invalid formats (e.g., missing count, malformed numbers) will result in NULL.
 * - Caller must free the returned string.
 */
char *rle_decompress(const char *encoded);

#endif