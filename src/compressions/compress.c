#include "compressions/compress.h"
#include "components/render_compress.h"
#include "compressions/algorithms/delta.h"
#include "compressions/algorithms/huffman.h"
#include "compressions/algorithms/rle.h"
#include "compressions/bitstream.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CompressedFrame compress_frame(RenderCompressContext *ctx, const AsciiFrame *frame) {
    CompressedFrame out;
    char *flat = NULL;
    BitWriter bw;
    size_t i;
    unsigned char c, len;
    unsigned int bits;
    char *encoded = NULL;

    memset(&out, 0, sizeof(out));

    if (!ctx || !frame)
        return out;

    flat = flatten_ascii_frame(frame);
    if (!flat)
        return out;

    bitwriter_init(&bw);

    switch (ctx->compression) {
    case COMPRESS_HUFFMAN:
        for (i = 0; flat[i]; i++) {
            c = (unsigned char)flat[i];
            bits = ctx->codes[c];
            len = ctx->code_lengths[c];

            if (len == 0) {
                /* unknown symbol */
                free(flat);
                bitwriter_free(&bw);
                memset(&out, 0, sizeof(out));
                return out;
            }

            bitwriter_write_bits(&bw, bits, len);
        }
        break;
    case COMPRESS_RLE:
        encoded = rle_compress(flat);
        if (!encoded) {
            free(flat);
            bitwriter_free(&bw);
            memset(&out, 0, sizeof(out));
            return out;
        }
        for (i = 0; encoded[i]; i++) {
            bitwriter_write_bits(&bw, (unsigned char)encoded[i], 8);
        }
        free(encoded);
        break;
    case COMPRESS_DELTA:
        if (!ctx->delta_has_prev_frame || !ctx->delta_prev_frame) {
            for (i = 0; flat[i]; i++) {
                bitwriter_write_bits(&bw, (unsigned char)flat[i], 8);
            }
        } else {
            encoded = delta_compress(ctx->delta_prev_frame, flat);
            if (!encoded) {
                free(flat);
                bitwriter_free(&bw);
                memset(&out, 0, sizeof(out));
                return out;
            }
            for (i = 0; encoded[i]; i++) {
                bitwriter_write_bits(&bw, (unsigned char)encoded[i], 8);
            }
            free(encoded);
        }
        break;
    case COMPRESS_NONE:
    default:
        for (i = 0; flat[i]; i++) {
            bitwriter_write_bits(&bw, (unsigned char)flat[i], 8);
        }
        break;
    }

    if (ctx->compression == COMPRESS_DELTA) {
        if (ctx->delta_prev_frame) {
            free(ctx->delta_prev_frame);
            ctx->delta_prev_frame = NULL;
        }
        ctx->delta_prev_frame = malloc(strlen(flat) + 1);
        if (!ctx->delta_prev_frame) {
            free(flat);
            bitwriter_free(&bw);
            memset(&out, 0, sizeof(out));
            return out;
        }
        strcpy(ctx->delta_prev_frame, flat);
        ctx->delta_has_prev_frame = 1;
    }

    free(flat);

    out.data = bw.data;
    out.data_bits = bw.bit_count;

    return out;
}
