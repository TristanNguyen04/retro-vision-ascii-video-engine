#include "components/preview.h"
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
 -> parse header
 -> reconstruct context (compression type + FSM if Huffman)
 -> for each frame:
      read DATA_BITS + binary
      decompress (or not)
      print
*/

static void clear_screen(void) {
#ifdef _WIN32
    /*
       Windows: use system command to clear console
    */
    system("cls");
#else
    /*
       Unix/Linux/macOS:
       \033[2J -> clear entire screen
       \033[H  -> move cursor to top-left (home position)
    */
    printf("\033[2J\033[H");
#endif
}

int parse_header(FILE *fp, RenderCompressContext *ctx) {
    char line[256];
    int tmp;

    /*
       Read file line by line until header ends
    */
    while (fgets(line, sizeof(line), fp)) {

        /*
           Parse frame width
        */
        if (strncmp(line, "WIDTH", 5) == 0)
            sscanf(line, "WIDTH %u", &ctx->width);

        /*
           Parse frame height
        */
        else if (strncmp(line, "HEIGHT", 6) == 0)
            sscanf(line, "HEIGHT %u", &ctx->height);

        /*
           Parse compression type
        */
        else if (strncmp(line, "COMPRESSION", 11) == 0) {
            sscanf(line, "COMPRESSION %d", &tmp);
            ctx->compression = (CompressionType)tmp;
        }

        /*
           Parse Huffman parameter K (bits per step)
        */
        else if (strncmp(line, "K", 1) == 0)
            sscanf(line, "K %d", &ctx->fsm->K);

        /*
           Parse number of FSM states
        */
        else if (strncmp(line, "NUM_STATES", 10) == 0)
            sscanf(line, "NUM_STATES %d", &ctx->fsm->num_states);

        /*
           Start of FSM section -> delegate parsing
        */
        else if (strncmp(line, "FSM_BEGIN", 9) == 0)
            return parse_fsm(fp, ctx); /* continue parsing FSM */

        /*
           Reached first frame -> header parsing complete
        */
        else if (strncmp(line, "FRAME", 5) == 0) {

            /* Rewind file pointer so FRAME can be processed later */
            fseek(fp, -strlen(line), SEEK_CUR);

            return 1;
        }
    }

    /* Successfully parsed header (or reached EOF) */
    return 1;
}

int parse_fsm(FILE *fp, RenderCompressContext *ctx) {
    char line[512];
    int s, input, next, bits_used, out_len;
    int n, k;
    char *ptr;
    FSMEntry *entry;

    /* Number of possible inputs per state = 2^K */
    int table_width = (1 << ctx->fsm->K);

    /* Allocate FSM table:
       size = num_states × 2^K */
    ctx->fsm->table = malloc(sizeof(FSMEntry) *
                             ctx->fsm->num_states * table_width);

    /*
       Read FSM section line by line
    */
    while (fgets(line, sizeof(line), fp)) {

        /* End of FSM section */
        if (strncmp(line, "FSM_END", 7) == 0)
            return 1;

        ptr = line;

        /*
           Parse fixed fields:
           state input next_state bits_used output_length
        */
        if (sscanf(ptr, "%d %d %d %d %d%n",
                   &s, &input, &next, &bits_used, &out_len, &n) < 5)
            return 0;

        ptr += n; /* move pointer past parsed values */

        /* Locate corresponding FSM table entry */
        entry = &ctx->fsm->table[s * table_width + input];

        /* Store transition information */
        entry->next_state = next;
        entry->bits_used = bits_used;
        entry->symbol_count = out_len;

        /*
           Parse output symbols (if any)
        */
        if (out_len > 0) {

            for (k = 0; k < out_len; k++) {
                int val;

                /* Read next symbol (ASCII value) */
                if (sscanf(ptr, "%d%n", &val, &n) < 1)
                    return 0;

                ptr += n;

                /* Store symbol */
                entry->symbols[k] = (char)val;
            }
        } else {
            /* No output symbols for this transition */
            entry->symbol_count = 0;
        }
    }

    /* If FSM_END not found → invalid format */
    return 0;
}
int read_frame(FILE *fp, CompressedFrame *frame) {
    char line[256];
    size_t byte_count;
    unsigned long tmp;
    size_t bytes_read;
    int ch;

    /*
       Read frame section line by line
    */
    while (fgets(line, sizeof(line), fp)) {

        /*
           Parse frame number
        */
        if (strncmp(line, "FRAME", 5) == 0)
            sscanf(line, "FRAME %u", &frame->frame_number);

        /*
           Parse timestamp
        */
        else if (strncmp(line, "TIME", 4) == 0)
            sscanf(line, "TIME %lf", &frame->timestamp);

        /*
           Parse number of valid bits in data
        */
        else if (strncmp(line, "DATA_BITS", 9) == 0) {
            sscanf(line, "DATA_BITS %lu", &tmp);
            frame->data_bits = (size_t)tmp;
        }

        /*
           Start of binary data section
        */
        else if (strncmp(line, "DATA_BEGIN", 10) == 0) {

            /* Compute number of bytes needed to store bits */
            byte_count = (frame->data_bits + 7) / 8;

            /* Allocate buffer for binary data */
            frame->data = malloc(byte_count);
            if (!frame->data)
                return 0;

            /* Read raw binary data */
            bytes_read = fread(frame->data, 1, byte_count, fp);
            if (bytes_read != byte_count)
                return 0;

            /*
               Skip remaining characters until newline
               (ensures correct alignment after binary read)
            */
            while ((ch = fgetc(fp)) != '\n' && ch != EOF)
                ;
        }

        /*
           End of current frame
        */
        else if (strncmp(line, "DATA_END", 8) == 0) {
            return 1;
        }
    }

    /* Return 0 if frame not fully read (EOF or error) */
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

    /*
       Step 0: Check command-line arguments
    */
    if (argc < 2) {
        printf("Usage: %s <compressed_file>\n", argv[0]);
        return 1;
    }

    /* Open compressed file in binary mode */
    fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    /*
       Step 1: Initialize context and frame structures
    */
    memset(&ctx, 0, sizeof(ctx));
    memset(&frame, 0, sizeof(frame));

    /* Allocate FSM (used for Huffman decoding if needed) */
    ctx.fsm = malloc(sizeof(*ctx.fsm));
    if (!ctx.fsm) {
        fclose(fp);
        return 1;
    }
    memset(ctx.fsm, 0, sizeof(*ctx.fsm));

    /*
       Step 2: Parse file header (metadata + optional FSM)
    */
    if (!parse_header(fp, &ctx)) {
        printf("Failed to parse header\n");
        exit_code = 1;
        goto cleanup;
    }

    printf("Parsed header:\n");
    printf("WIDTH=%u HEIGHT=%u COMPRESSION=%d\n",
           ctx.width, ctx.height, ctx.compression);

    /* Clear screen and disable buffering for smooth playback */
    clear_screen();
    setbuf(stdout, NULL);

    /*
       Step 3: Read and decode frames in a loop
    */
    while (1) {

        /* Reset frame structure */
        memset(&frame, 0, sizeof(frame));

        /* Read next frame; break if no more frames */
        if (!read_frame(fp, &frame)) {
            break;
        }

        /*
           Decode frame based on compression type
        */
        if (ctx.compression == COMPRESS_NONE) {

            /* No compression: directly copy raw bytes */
            bytes = (frame.data_bits + 7) / 8;
            decoded = malloc(bytes + 1);
            if (!decoded) {
                exit_code = 1;
                goto cleanup;
            }

            memcpy(decoded, frame.data, bytes);
            decoded[bytes] = '\0';

        } else {

            /* Use appropriate decompression function */
            decoded = decompress_frame(&ctx, &frame);
            if (!decoded) {
                exit_code = 1;
                goto cleanup;
            }
        }

        /*
           Step 4: Display frame
        */

        printf("\n");

        /* Move cursor to top-left (for animation effect) */
        printf("\033[H");

        /* Print frame metadata */
        printf("FRAME %u | TIME %.3f\n",
               frame.frame_number, frame.timestamp);

        /* Prepare buffer for formatted output */
        frame_size = ctx.width * ctx.height + ctx.height;
        buffer = malloc(frame_size + 1);
        if (!buffer) {
            exit_code = 1;
            goto cleanup;
        }

        /* Reconstruct frame line by line */
        pos = 0;
        for (r = 0; r < ctx.height; r++) {
            memcpy(buffer + pos, decoded + r * (ctx.width + 1), ctx.width + 1);
            pos += ctx.width + 1;
        }
        buffer[pos] = '\0';

        /* Print frame to stdout */
        fwrite(buffer, 1, pos, stdout);

        free(buffer);
        buffer = NULL;

        /* Small delay for playback */
        sleep_ms(100);

        /* Cleanup per-frame allocations */
        free(decoded);
        decoded = NULL;

        free(frame.data);
        frame.data = NULL;

        frame_count++;
    }

    /*
       Step 5: Final summary
    */
    printf("\nTotal frames: %d\n", frame_count);

cleanup:
    /*
       Cleanup all allocated resources
    */

    /* (Optional!): debug print FSM */
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

    /* Free FSM structure */
    if (ctx.fsm) {
        huffman_free_fsm(ctx.fsm);
        ctx.fsm = NULL;
    }

    /* Close file */
    if (fp) {
        fclose(fp);
    }

    return exit_code;
}