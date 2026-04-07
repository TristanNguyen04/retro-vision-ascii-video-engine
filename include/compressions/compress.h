#ifndef COMPRESS_H
#define COMPRESS_H

typedef enum {
    COMPRESS_NONE,
    COMPRESS_RLE,
    COMPRESS_DELTA,
    COMPRESS_HUFFMAN
} CompressionType;

/**
 * Compress a string input using the given compresstion type algorithm
 */
char *compress_data(const char *input, CompressionType type);

#endif