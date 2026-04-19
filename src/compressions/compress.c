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

    /* Initialize output structure to zero */
    memset(&out, 0, sizeof(out));

    /* Input validation */
    if (!ctx || !frame)
        return out;

    /* Flatten 2D ASCII frame into 1D string */
    flat = flatten_ascii_frame(frame);
    if (!flat)
        return out;

    /* Initialize bit writer for binary output */
    bitwriter_init(&bw);

    /*
       Step 1: Apply selected compression method
    */
    switch (ctx->compression) {

    /*  Huffman  */
    case COMPRESS_HUFFMAN:
        for (i = 0; flat[i]; i++) {

            /* Get precomputed Huffman code for character */
            c = (unsigned char)flat[i];
            bits = ctx->codes[c];
            len = ctx->code_lengths[c];

            /* Invalid if no code exists */
            if (len == 0) {
                free(flat);
                bitwriter_free(&bw);
                memset(&out, 0, sizeof(out));
                return out;
            }

            /* Write variable-length Huffman code */
            bitwriter_write_bits(&bw, bits, len);
        }
        break;

    /*  RLE  */
    case COMPRESS_RLE:
        encoded = rle_compress(flat);
        if (!encoded) {
            free(flat);
            bitwriter_free(&bw);
            memset(&out, 0, sizeof(out));
            return out;
        }

        /* Write encoded string as bytes */
        for (i = 0; encoded[i]; i++) {
            bitwriter_write_bits(&bw, (unsigned char)encoded[i], 8);
        }

        free(encoded);
        break;

    /*  Delta  */
    case COMPRESS_DELTA:

        /* If no previous frame, store raw frame */
        if (!ctx->delta_has_prev_frame || !ctx->delta_prev_frame) {
            for (i = 0; flat[i]; i++) {
                bitwriter_write_bits(&bw, (unsigned char)flat[i], 8);
            }
        } else {
            /* Encode differences relative to previous frame */
            encoded = delta_compress(ctx->delta_prev_frame, flat);
            if (!encoded) {
                free(flat);
                bitwriter_free(&bw);
                memset(&out, 0, sizeof(out));
                return out;
            }

            /* Write delta-encoded string */
            for (i = 0; encoded[i]; i++) {
                bitwriter_write_bits(&bw, (unsigned char)encoded[i], 8);
            }

            free(encoded);
        }
        break;

    /*  No Compression  */
    case COMPRESS_NONE:
    default:
        /* Directly write raw ASCII bytes */
        for (i = 0; flat[i]; i++) {
            bitwriter_write_bits(&bw, (unsigned char)flat[i], 8);
        }
        break;
    }

    /*
       Step 2: Update previous frame (for delta encoding)
    */
    if (ctx->compression == COMPRESS_DELTA) {

        /* Free previous stored frame */
        if (ctx->delta_prev_frame) {
            free(ctx->delta_prev_frame);
            ctx->delta_prev_frame = NULL;
        }

        /* Store current frame as previous frame */
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

    /* Free temporary flattened frame */
    free(flat);

    /*
       Step 3: Finalize output
    */
    out.data = bw.data;           /* pointer to compressed binary data */
    out.data_bits = bw.bit_count; /* number of valid bits */

    return out;
}