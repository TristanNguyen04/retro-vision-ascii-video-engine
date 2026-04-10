#include "components/reader.h"
#include "components/render_compress.h"
#include "compressions/algorithms/huffman.h"
#include "compressions/decompress.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep((ms) * 1000)
#endif

/*
pipeline:
read file
 → parse header
 → reconstruct context (compression type + FSM if Huffman)
 → for each frame:
      read DATA_BITS + binary
      decompress (or not)
      print
*/

static void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
#endif
}

int parse_header(FILE *fp, RenderCompressContext *ctx) {
    char line[256];
    int tmp;

    while (fgets(line, sizeof(line), fp)) {

        if (strncmp(line, "WIDTH", 5) == 0)
            sscanf(line, "WIDTH %u", &ctx->width);

        else if (strncmp(line, "HEIGHT", 6) == 0)
            sscanf(line, "HEIGHT %u", &ctx->height);

        else if (strncmp(line, "COMPRESSION", 11) == 0) {
            sscanf(line, "COMPRESSION %d", &tmp);
            ctx->compression = (CompressionType)tmp;
        }

        else if (strncmp(line, "K", 1) == 0)
            sscanf(line, "K %d", &ctx->fsm->K);

        else if (strncmp(line, "NUM_STATES", 10) == 0)
            sscanf(line, "NUM_STATES %d", &ctx->fsm->num_states);

        else if (strncmp(line, "FSM_BEGIN", 9) == 0)
            return parse_fsm(fp, ctx); /* continue parsing FSM */

        else if (strncmp(line, "FRAME", 5) == 0) {
            /* rewind so FRAME can be handled later */
            fseek(fp, -strlen(line), SEEK_CUR);
            return 1;
        }
    }

    return 1;
}

int parse_fsm(FILE *fp, RenderCompressContext *ctx) {
    char line[512];
    int s, input, next, bits_used, out_len;
    int n, k;
    char *ptr;
    FSMEntry *entry;

    int table_width = (1 << ctx->fsm->K);

    ctx->fsm->table = malloc(sizeof(FSMEntry) *
                             ctx->fsm->num_states * table_width);

    while (fgets(line, sizeof(line), fp)) {

        if (strncmp(line, "FSM_END", 7) == 0)
            return 1;

        ptr = line;

        if (sscanf(ptr, "%d %d %d %d %d%n",
                   &s, &input, &next, &bits_used, &out_len, &n) < 5)
            return 0;

        ptr += n;

        entry = &ctx->fsm->table[s * table_width + input];

        entry->next_state = next;
        entry->bits_used = bits_used;
        entry->symbol_count = out_len;

        if (out_len > 0) {
            entry->symbol_count = out_len;

            for (k = 0; k < out_len; k++) {
                int val;

                if (sscanf(ptr, "%d%n", &val, &n) < 1)
                    return 0;

                ptr += n;

                entry->symbols[k] = (char)val;
            }
        } else {
            entry->symbol_count = 0;
        }
    }

    return 0;
}

int read_frame(FILE *fp, CompressedFrame *frame) {
    char line[256];
    size_t byte_count;
    unsigned long tmp;
    size_t bytes_read;
    int ch;

    while (fgets(line, sizeof(line), fp)) {

        if (strncmp(line, "FRAME", 5) == 0)
            sscanf(line, "FRAME %u", &frame->frame_number);

        else if (strncmp(line, "TIME", 4) == 0)
            sscanf(line, "TIME %lf", &frame->timestamp);

        else if (strncmp(line, "DATA_BITS", 9) == 0) {
            sscanf(line, "DATA_BITS %lu", &tmp);
            frame->data_bits = (size_t)tmp;
        }

        else if (strncmp(line, "DATA_BEGIN", 10) == 0) {

            byte_count = (frame->data_bits + 7) / 8;
            frame->data = malloc(byte_count);
            if (!frame->data)
                return 0;

            bytes_read = fread(frame->data, 1, byte_count, fp);
            if (bytes_read != byte_count)
                return 0;

            /* IMPORTANT: read until newline AFTER binary safely */
            while ((ch = fgetc(fp)) != '\n' && ch != EOF)
                ;

        } else if (strncmp(line, "DATA_END", 8) == 0) {
            return 1;
        }
    }

    return 0;
}

static void debug_print_fsm(const HuffmanFSM *fsm) {
    int s, i, k;
    int table_width = 1 << fsm->K;

    printf("\n=== FSM DEBUG ===\n");

    for (s = 0; s < fsm->num_states; s++) {
        printf("STATE %d:\n", s);

        for (i = 0; i < table_width; i++) {
            FSMEntry *e = &fsm->table[s * table_width + i];

            printf("  input=%d -> next=%d, bits_used=%d, symbols=",
                   i, e->next_state, e->bits_used);

            if (e->symbol_count == 0) {
                printf("[]");
            } else {
                printf("[");
                for (k = 0; k < e->symbol_count; k++) {
                    printf("%c", e->symbols[k]);
                }
                printf("]");
            }

            printf("\n");
        }
    }
}

int main(int argc, char *argv[]) {
    FILE *fp = NULL;
    RenderCompressContext ctx;
    CompressedFrame frame;
    int frame_count = 0;
    size_t bytes;
    unsigned int r;
    char *decoded = NULL;
    size_t frame_size;
    char *buffer = NULL;
    size_t pos = 0;
    int exit_code = 0;

    if (argc < 2) {
        printf("Usage: %s <compressed_file>\n", argv[0]);
        return 1;
    }

    fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    /* init context */
    memset(&ctx, 0, sizeof(ctx));
    memset(&frame, 0, sizeof(frame));

    /* allocate FSM if needed */
    ctx.fsm = malloc(sizeof(*ctx.fsm));
    if (!ctx.fsm) {
        fclose(fp);
        return 1;
    }
    memset(ctx.fsm, 0, sizeof(*ctx.fsm));

    /* parse header */
    if (!parse_header(fp, &ctx)) {
        printf("Failed to parse header\n");
        exit_code = 1;
        goto cleanup;
    }

    printf("Parsed header:\n");
    printf("WIDTH=%u HEIGHT=%u COMPRESSION=%d\n",
           ctx.width, ctx.height, ctx.compression);

    clear_screen();
    setbuf(stdout, NULL);

    /* read frames */
    while (1) {
        memset(&frame, 0, sizeof(frame));

        if (!read_frame(fp, &frame)) {
            break;
        }

        if (ctx.compression == COMPRESS_NONE) {
            bytes = (frame.data_bits + 7) / 8;
            decoded = malloc(bytes + 1);
            if (!decoded) {
                exit_code = 1;
                goto cleanup;
            }
            memcpy(decoded, frame.data, bytes);
            decoded[bytes] = '\0';
        } else {
            decoded = decompress_frame(&ctx, &frame);
            if (!decoded) {
                exit_code = 1;
                goto cleanup;
            }
        }

        printf("\n");

        printf("\033[H");

        printf("FRAME %u | TIME %.3f\n",
               frame.frame_number, frame.timestamp);

        frame_size = ctx.width * ctx.height + ctx.height;
        buffer = malloc(frame_size + 1);
        if (!buffer) {
            exit_code = 1;
            goto cleanup;
        }

        pos = 0;
        for (r = 0; r < ctx.height; r++) {
            memcpy(buffer + pos, decoded + r * (ctx.width + 1), ctx.width + 1);
            pos += ctx.width + 1;
        }
        buffer[pos] = '\0';

        fwrite(buffer, 1, pos, stdout);
        free(buffer);

        sleep_ms(100);

        free(decoded);
        decoded = NULL;
        free(frame.data);
        frame.data = NULL;

        frame_count++;
    }

    printf("\nTotal frames: %d\n", frame_count);

cleanup:
    if (ctx.fsm) {
        debug_print_fsm(ctx.fsm);
    }
    if (buffer) {
        free(buffer);
    }
    if (decoded) {
        free(decoded);
    }
    if (frame.data) {
        free(frame.data);
    }
    if (ctx.fsm) {
        huffman_free_fsm(ctx.fsm);
        ctx.fsm = NULL;
    }
    if (fp) {
        fclose(fp);
    }

    return exit_code;
}