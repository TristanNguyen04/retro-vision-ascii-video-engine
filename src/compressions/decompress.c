#include "components/render_compress.h"
#include "compressions/algorithms/delta.h"
#include "compressions/algorithms/huffman.h"
#include "compressions/algorithms/rle.h"
#include "compressions/bitstream.h"
#include "compressions/compress.h"

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

    for (i = 0; i < 10; i++) {
        printf("%c", bitstr[i]);
    }
    printf("\n");

    /* original length = width * height */
    original_len = ctx->width * ctx->height;

    decoded = huffman_decode(ctx->fsm, bitstr, original_len);

    free(bitstr);
    return decoded;
}

char *decompress_frame(const RenderCompressContext *ctx,
                       const CompressedFrame *in) {
    if (!ctx || !in)
        return NULL;

    switch (ctx->compression) {
    case COMPRESS_HUFFMAN:
        return decompress_frame_huffman(ctx, in);

    case COMPRESS_NONE:
    default:
        return decompress_frame_none(in);
    }
}