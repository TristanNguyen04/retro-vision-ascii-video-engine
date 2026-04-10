#ifndef COMPRESS_H
#define COMPRESS_H

#include "components/ascii.h"

typedef enum {
    COMPRESS_NONE,
    COMPRESS_RLE,
    COMPRESS_DELTA,
    COMPRESS_HUFFMAN
} CompressionType;

/* declaration in render_compress.h */
typedef struct RenderCompressContext RenderCompressContext;
typedef struct CompressedFrame CompressedFrame;

/**
 * Compress a frame using the given compresstion type algorithm
 */
CompressedFrame compress_frame(RenderCompressContext *ctx, const AsciiFrame *frame);

#endif