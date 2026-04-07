#include "compress.h"
#include "delta.h"
#include "rle.h"
#include <stdlib.h>

char *compress_data(const char *input, CompressionType type) {
    char *output = NULL;
    size_t before, after;

    if (!input)
        return NULL;

    before = strlen(input);

    switch (type) {
    case COMPRESS_RLE:
        output = rle_compress(input);
        break;
    case COMPRESS_DELTA:
        /* TODO: Need to get two consecutive frames */
        /* output = delta_compress(prev, curr); */
        break;
    default:
        output = strdup(input);
        break;
    }

    after = strlen(output);

    printf("Before: %zu bytes, After: %zu bytes, Ratio: %.2f%%\n",
           before,
           after,
           before ? (100.0 * after / before) : 0.0);

    return output;
}