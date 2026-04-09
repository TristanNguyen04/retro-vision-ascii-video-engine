#include "components/render_compress.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO: file name and structure to be discussed */

void render_compress_ctx_init(RenderCompressContext *ctx) {
    if (ctx == NULL) {
        return;
    }

    ctx->width = 0;
    ctx->height = 0;
    ctx->compression = 0;
    ctx->huffman_K = 0;

    ctx->fsm = NULL;

    memset(ctx->symbol_freq, 0, sizeof(ctx->symbol_freq));
    memset(ctx->codes, 0, sizeof(ctx->codes));
    memset(ctx->code_lengths, 0, sizeof(ctx->code_lengths));
}

RenderError render_write_header(FILE *fp, const RenderCompressContext *ctx) {
    FSMEntry *entry;
    int i, s, k;
    int table_width;

    if (!fp || !ctx)
        return RENDER_ERR_NULL_ARG;

    fprintf(fp, "FORMAT ASCII_VIDEO\n");
    fprintf(fp, "WIDTH %u\n", 640);  /* ctx->width); */
    fprintf(fp, "HEIGHT %u\n", 360); /* ctx->height); */
    fprintf(fp, "COMPRESSION %d\n", ctx->compression);

    if (ctx->compression == COMPRESS_HUFFMAN) {
        table_width = (1 << ctx->fsm->K);
        fprintf(fp, "K %d\n", ctx->fsm->K);
        fprintf(fp, "NUM_STATES %d\n", ctx->fsm->num_states);

        fprintf(fp, "FSM_BEGIN\n");

        for (s = 0; s < ctx->fsm->num_states; s++) {
            for (i = 0; i < table_width; i++) {
                entry = &ctx->fsm->table[s * table_width + i];

                fprintf(fp, "%d %d %d %d %d ",
                        s, i,
                        entry->next_state,
                        entry->bits_used,
                        entry->symbol_count);

                if (entry->symbol_count > 0) {
                    for (k = 0; k < entry->symbol_count; k++) {
                        fprintf(fp, "%d ", (unsigned char)entry->symbols[k]); /* print ascii as symbol might contain ' ' */
                    }
                } else {
                    fprintf(fp, "-");
                }

                fprintf(fp, "\n");
            }
        }

        fprintf(fp, "FSM_END\n");
    }

    return RENDER_OK;
}

RenderError render_write_frame_compressed(FILE *fp, const CompressedFrame *frame) {
    size_t bytes;
    if (!fp || !frame)
        return RENDER_ERR_NULL_ARG;

    fprintf(fp, "FRAME %u\n", frame->frame_number);
    fprintf(fp, "TIME %.3f\n", frame->timestamp);
    fprintf(fp, "HIGHLIGHT %d\n", frame->highlight);

    fprintf(fp, "DATA_BITS %lu\n", (unsigned long)frame->data_bits);
    fprintf(fp, "DATA_BEGIN\n");

    bytes = (frame->data_bits + 7) / 8;
    fwrite(frame->data, 1, bytes, fp);

    fprintf(fp, "\nDATA_END\n");

    return RENDER_OK;
}

RenderError render_read_header(FILE *fp, RenderCompressContext *ctx) {
    char line[256];
    int compression;
    int i;

    int s, input, next, bits_used, out_len;
    int table_width = 0;
    FSMEntry *entry;

    char *ptr = line;
    int n;

    if (!fp || !ctx)
        return RENDER_ERR_NULL_ARG;

    memset(ctx, 0, sizeof(*ctx));

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "WIDTH", 5) == 0)
            sscanf(line, "WIDTH %u", &ctx->width);

        else if (strncmp(line, "HEIGHT", 6) == 0)
            sscanf(line, "HEIGHT %u", &ctx->height);

        else if (strncmp(line, "COMPRESSION", 11) == 0) {
            sscanf(line, "COMPRESSION %d", &compression);
            ctx->compression = compression;
        }

        else if (strncmp(line, "K", 1) == 0) {
            ctx->fsm = malloc(sizeof(HuffmanFSM));
            sscanf(line, "K %d", &ctx->fsm->K);
        }

        else if (strncmp(line, "NUM_STATES", 10) == 0) {
            sscanf(line, "NUM_STATES %d", &ctx->fsm->num_states);
            table_width = 1 << ctx->fsm->K;
            ctx->fsm->table = malloc(
                ctx->fsm->num_states * table_width * sizeof(FSMEntry));
            if (!ctx->fsm->table)
                return 1; /* TODO: change to valid error */
        }

        else if (strncmp(line, "FSM_BEGIN", 9) == 0) {

            while (fgets(line, sizeof(line), fp)) {
                if (strncmp(line, "FSM_END", 7) == 0)
                    break;

                ptr = line;

                /* ---- parse first 4 integers ---- */
                if (sscanf(ptr, "%d %d %d %d %d%n",
                           &s, &input, &next, &bits_used, &out_len, &n) < 4) {
                    return 1; /* RENDER_ERR_BAD_FORMAT; */
                }

                ptr += n;

                entry = &ctx->fsm->table[s * table_width + input];

                entry->next_state = next;
                entry->symbol_count = out_len;

                /* ---- parse symbols (ASCII integers) ---- */
                for (i = 0; i < out_len; i++) {
                    int val;

                    if (sscanf(ptr, "%d%n", &val, &n) != 1) {
                        return 1; /* RENDER_ERR_BAD_FORMAT; */
                    }

                    ptr += n;

                    entry->symbols[i] = (char)val;
                }
            }

            break; /* header done */
        }
    }

    return RENDER_OK;
}

RenderError render_read_frame_compressed(FILE *fp, CompressedFrame *frame) {
    char line[256];
    size_t bytes;
    unsigned long tmp;

    if (!fp || !frame)
        return RENDER_ERR_NULL_ARG;

    memset(frame, 0, sizeof(*frame));

    while (fgets(line, sizeof(line), fp)) {

        if (strncmp(line, "FRAME", 5) == 0)
            sscanf(line, "FRAME %u", &frame->frame_number);

        else if (strncmp(line, "TIME", 4) == 0)
            sscanf(line, "TIME %lf", &frame->timestamp);

        else if (strncmp(line, "HIGHLIGHT", 9) == 0)
            sscanf(line, "HIGHLIGHT %d", &frame->highlight);

        else if (strncmp(line, "DATA_BITS", 9) == 0) {
            if (sscanf(line, "DATA_BITS %lu", &tmp) == 1)
                frame->data_bits = (size_t)tmp;
        }

        else if (strncmp(line, "DATA_BEGIN", 10) == 0) {
            bytes = (frame->data_bits + 7) / 8;

            frame->data = malloc(bytes);
            fread(frame->data, 1, bytes, fp);

            fgets(line, sizeof(line), fp); /* consume newline */
            fgets(line, sizeof(line), fp); /* DATA_END */

            return RENDER_OK;
        }
    }

    return 1; /* RENDER_ERR_READ_FAILED; */
}

void render_compress_ctx_free(RenderCompressContext *ctx) {
    if (ctx == NULL)
        return;
    if (ctx->fsm != NULL) {
        huffman_free_fsm(ctx->fsm);
    }
}