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

    /*
       Input validation
    */
    if (!ctx || !ctx->fsm || !in)
        return NULL;

    /* Allocate string to store bit stream ('0'/'1') */
    bitstr = malloc(in->data_bits + 1);
    if (!bitstr)
        return NULL;

    /* Initialize bit reader with compressed binary data */
    bitreader_init(&br, in->data, in->data_bits);

    /*
       Step 1: Convert bitstream to string form
    */
    for (i = 0; i < in->data_bits; i++) {

        /* Read 1 bit at a time */
        bitreader_read_bits(&br, 1, &bit);

        /* Store as character '0' or '1' */
        bitstr[i] = bit ? '1' : '0';
    }

    bitstr[in->data_bits] = '\0';

    /*
       Step 2: Determine original frame length
       - width characters + newline per row
    */
    original_len = (ctx->width + 1) * ctx->height;

    /*
       Step 3: Decode bit string using Huffman FSM
    */
    decoded = huffman_decode(ctx->fsm, bitstr, original_len);

    /* Free temporary bit string */
    free(bitstr);

    return decoded;
}

char *decompress_frame_rle(const CompressedFrame *in) {
    BitReader br;
    char *encoded;
    char *decoded;
    unsigned int val;
    size_t num_chars, i;

    /* Input validation */
    if (!in || !in->data)
        return NULL;

    /* Number of characters = total bits / 8 (RLE stored as bytes) */
    num_chars = in->data_bits / 8;

    /* Allocate buffer for encoded RLE string */
    encoded = malloc(num_chars + 1);
    if (!encoded)
        return NULL;

    /* Initialize bit reader with compressed data */
    bitreader_init(&br, in->data, in->data_bits);

    /*
       Step 1: Read byte-aligned RLE encoded data
    */
    for (i = 0; i < num_chars; i++) {

        /* Read 8 bits (1 byte) at a time */
        bitreader_read_bits(&br, 8, &val);

        /* Store as character */
        encoded[i] = (char)val;
    }

    /* Null-terminate encoded string */
    encoded[num_chars] = '\0';

    /*
       Step 2: Decompress RLE string
    */
    decoded = rle_decompress(encoded);

    /* Free temporary encoded buffer */
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

    /* Input validation */
    if (!ctx || !in || !in->data)
        return NULL;

    /*
       Step 1: Handle first frame (no previous frame available)
    */
    if (!ctx->delta_has_prev_frame || !ctx->delta_prev_frame) {

        /* First frame is stored without delta → decode directly */
        decoded = decompress_frame_none(in);
        if (!decoded)
            return NULL;

        /* Store as previous frame for future delta decoding */
        if (!delta_store_prev_frame(ctx, decoded)) {
            free(decoded);
            return NULL;
        }

        return decoded;
    }

    /*
       Step 2: Read encoded delta data (byte-aligned)
    */

    /* Number of characters = total bits / 8 */
    num_chars = in->data_bits / 8;

    /* Allocate buffer for encoded delta string */
    encoded = malloc(num_chars + 1);
    if (!encoded)
        return NULL;

    /* Initialize bit reader */
    bitreader_init(&br, in->data, in->data_bits);

    /* Read encoded bytes */
    for (i = 0; i < num_chars; i++) {
        bitreader_read_bits(&br, 8, &val);
        encoded[i] = (char)val;
    }

    /* Null-terminate encoded string */
    encoded[num_chars] = '\0';

    /*
       Step 3: Apply delta decompression
    */
    decoded = delta_decompress(ctx->delta_prev_frame, encoded);

    /* Free temporary encoded buffer */
    free(encoded);

    if (!decoded)
        return NULL;

    /*
       Step 4: Update previous frame for next iteration
    */
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
