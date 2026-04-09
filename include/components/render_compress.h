#ifndef RENDER_COMPRESS_H
#define RENDER_COMPRESS_H

#include "components/render.h"
#include "compressions/algorithms/huffman.h"
#include "compressions/compress.h"
#include <stdlib.h>

struct RenderCompressContext {
    unsigned int width;
    unsigned int height;
    CompressionType compression;
    unsigned int huffman_K;

    HuffmanFSM *fsm;
    unsigned int symbol_freq[256];
    unsigned int codes[256];
    unsigned char code_lengths[256];
};

struct CompressedFrame {
    unsigned int frame_number;
    double timestamp;
    int highlight;

    size_t data_bits;
    unsigned char *data;
};

void render_compress_ctx_init(RenderCompressContext *ctx);

RenderError render_write_header(FILE *fp, const RenderCompressContext *ctx);

RenderError render_write_frame_compressed(FILE *fp, const CompressedFrame *frame);

RenderError render_read_header(FILE *fp, RenderCompressContext *ctx);

RenderError render_read_frame_compressed(FILE *fp, CompressedFrame *frame);

void render_compress_ctx_free(RenderCompressContext *ctx);

#endif