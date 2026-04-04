#include "compress.h"
#include "huffman.h"
#include "rle.h"

char *compress_data(const char *input, CompressionType type) {
    switch (type) {
    case COMPRESS_RLE:
        break;
    case COMPRESS_HUFFMAN:
        break;
    default:
        return strdup(input);
    }
}