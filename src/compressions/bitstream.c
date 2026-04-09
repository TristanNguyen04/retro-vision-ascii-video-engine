#include "compressions/bitstream.h"
#include <stdlib.h>

void bitwriter_init(BitWriter *bw) {
    bw->capacity_bytes = 16;
    bw->bit_count = 0;
    bw->data = malloc(bw->capacity_bytes);
}

static int bitwrite_ensure_capacity(BitWriter *bw, size_t bits_needed) {
    size_t total_bits = bw->bit_count + bits_needed;
    size_t needed_bytes = (total_bits + 7) / 8;
    unsigned char *tmp;
    size_t new_cap;

    if (needed_bytes > bw->capacity_bytes) {
        new_cap = bw->capacity_bytes * 2;
        tmp = realloc(bw->data, new_cap);
        if (!tmp)
            return 0;
        bw->data = tmp;
        bw->capacity_bytes = new_cap;
    }
    return 1;
}

void bitwriter_write_bits(BitWriter *bw, unsigned int bits, int count) {
    int i;
    int bit, bit_offset;
    size_t byte_index;

    /* write from MSB to LSB */
    for (i = count - 1; i >= 0; i--) {

        if (!bitwrite_ensure_capacity(bw, 1))
            return;

        bit = (bits >> i) & 1;

        byte_index = bw->bit_count / 8;
        bit_offset = 7 - (bw->bit_count % 8);

        /* set 0 for new byte */
        if (bw->bit_count % 8 == 0)
            bw->data[byte_index] = 0;

        bw->data[byte_index] |= (bit << bit_offset);

        bw->bit_count++;
    }
}

void bitreader_init(BitReader *br, const unsigned char *data, size_t bits) {
    br->data = data;
    br->bit_count = bits;
    br->position = 0;
}

void bitreader_read_bits(BitReader *br, int k, unsigned int *out) {
    unsigned int result = 0;
    size_t byte_index;
    int bit, bit_offset;
    int i;

    if (br->position + k > br->bit_count)
        return; /* TODO: add error here, not enough bits */

    for (i = 0; i < k; i++) {
        byte_index = br->position / 8;
        bit_offset = 7 - (br->position % 8);

        bit = (br->data[byte_index] >> bit_offset) & 1;

        result = (result << 1) | bit;

        br->position++;
    }

    *out = result;
    return;
}

void bitwriter_free(BitWriter *bw) {
    free(bw->data);
}