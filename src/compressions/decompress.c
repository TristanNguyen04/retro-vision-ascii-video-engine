#include "components/render_compress.h"
#include "compressions/algorithms/delta.h"
#include "compressions/algorithms/huffman.h"
#include "compressions/algorithms/rle.h"
#include "compressions/bitstream.h"
#include "compressions/compress.h"
#include <stdlib.h>
#include <string.h>

static int delta_store_prev_frame(RenderCompressContext *ctx, const char *decoded) {
    char *next_prev;

    if (!ctx || !decoded)
        return 0;

    next_prev = malloc(strlen(decoded) + 1);
    if (!next_prev)
        return 0;

    strcpy(next_prev, decoded);

    if (ctx->delta_prev_frame) {
        free(ctx->delta_prev_frame);
    }
    ctx->delta_prev_frame = next_prev;
    ctx->delta_has_prev_frame = 1;

    return 1;
}

char *decompress_frame_none(const CompressedFrame *in) {
    BitReader br;
    char *out;
    unsigned int val;
    size_t num_chars, i;

    if (!in || !in->data)
        return NULL;

    num_chars = in->data_bits / 8;

    out = malloc(num_chars + 1);
    if (!out)
        return NULL;

    bitreader_init(&br, in->data, in->data_bits);

    for (i = 0; i < num_chars; i++) {
        bitreader_read_bits(&br, 8, &val);
        out[i] = (char)val;
    }

    out[num_chars] = '\0';
    return out;
}

char *decompress_frame_huffman(const RenderCompressContext *ctx,
                               const CompressedFrame *in) {
    BitReader br;
    char *bitstr;
    char *decoded;
    size_t i;
    int original_len;
    unsigned int bit;

    if (!ctx || !ctx->fsm || !in)
        return NULL;

    bitstr = malloc(in->data_bits + 1);
    if (!bitstr)
        return NULL;

    bitreader_init(&br, in->data, in->data_bits);

    for (i = 0; i < in->data_bits; i++) {
        bitreader_read_bits(&br, 1, &bit);
        bitstr[i] = bit ? '1' : '0';
    }
    bitstr[in->data_bits] = '\0';

    /* original length = width * height + '\n' for each line */
    original_len = (ctx->width + 1) * ctx->height;

    decoded = huffman_decode(ctx->fsm, bitstr, original_len);

    free(bitstr);
    return decoded;
}

char *decompress_frame_rle(const CompressedFrame *in) {
    BitReader br;
    char *encoded;
    char *decoded;
    unsigned int val;
    size_t num_chars, i;

    if (!in || !in->data)
        return NULL;

    num_chars = in->data_bits / 8;
    encoded = malloc(num_chars + 1);
    if (!encoded)
        return NULL;

    bitreader_init(&br, in->data, in->data_bits);

    for (i = 0; i < num_chars; i++) {
        bitreader_read_bits(&br, 8, &val);
        encoded[i] = (char)val;
    }
    encoded[num_chars] = '\0';

    decoded = rle_decompress(encoded);
    free(encoded);
    return decoded;
}

char *decompress_frame_delta(RenderCompressContext *ctx,
                             const CompressedFrame *in) {
    BitReader br;
    char *encoded;
    char *decoded;
    unsigned int val;
    size_t num_chars, i;

    if (!ctx || !in || !in->data)
        return NULL;

    if (!ctx->delta_has_prev_frame || !ctx->delta_prev_frame) {
        decoded = decompress_frame_none(in);
        if (!decoded)
            return NULL;

        if (!delta_store_prev_frame(ctx, decoded)) {
            free(decoded);
            return NULL;
        }
        return decoded;
    }

    num_chars = in->data_bits / 8;
    encoded = malloc(num_chars + 1);
    if (!encoded)
        return NULL;

    bitreader_init(&br, in->data, in->data_bits);
    for (i = 0; i < num_chars; i++) {
        bitreader_read_bits(&br, 8, &val);
        encoded[i] = (char)val;
    }
    encoded[num_chars] = '\0';

    decoded = delta_decompress(ctx->delta_prev_frame, encoded);
    free(encoded);
    if (!decoded)
        return NULL;

    if (!delta_store_prev_frame(ctx, decoded)) {
        free(decoded);
        return NULL;
    }

    return decoded;
}

char *decompress_frame(RenderCompressContext *ctx,
                       const CompressedFrame *in) {
    if (!ctx || !in)
        return NULL;

    switch (ctx->compression) {
    case COMPRESS_HUFFMAN:
        return decompress_frame_huffman(ctx, in);
    case COMPRESS_RLE:
        return decompress_frame_rle(in);
    case COMPRESS_DELTA:
        return decompress_frame_delta(ctx, in);
    case COMPRESS_NONE:
    default:
        return decompress_frame_none(in);
    }
}