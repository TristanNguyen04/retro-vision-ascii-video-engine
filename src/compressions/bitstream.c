#include "compressions/bitstream.h"
#include <stdlib.h>

void bitwriter_init(BitWriter *bw) {
    bw->capacity_bytes = 16;
    bw->bit_count = 0;
    bw->data = malloc(bw->capacity_bytes);
}

static int bitwrite_ensure_capacity(BitWriter *bw, size_t bits_needed) {
    size_t total_bits = bw->bit_count + bits_needed; /* total bits after writing */

    /* Convert required bits to bytes (round up) */
    size_t needed_bytes = (total_bits + 7) / 8;

    unsigned char *tmp;
    size_t new_cap;

    /* Check if current buffer is sufficient*/
    if (needed_bytes > bw->capacity_bytes) {
        new_cap = bw->capacity_bytes * 2;
        tmp = realloc(bw->data, new_cap);
        if (!tmp)
            return 0;

        /* Update buffer pointer and capacity */
        bw->data = tmp;
        bw->capacity_bytes = new_cap;
    }

    /* Enough capacity available */
    return 1;
}
void bitwriter_write_bits(BitWriter *bw, unsigned int bits, int count) {
    int i;
    int bit, bit_offset;
    size_t byte_index;

    /*
       Write bits from most significant bit (MSB) to least (LSB)
    */
    for (i = count - 1; i >= 0; i--) {

        /* Ensure there is space for at least 1 more bit */
        if (!bitwrite_ensure_capacity(bw, 1))
            return;

        /* Extract i-th bit from input (MSB first) */
        bit = (bits >> i) & 1;

        /* Determine which byte and bit position to write to */
        byte_index = bw->bit_count / 8;       /* target byte */
        bit_offset = 7 - (bw->bit_count % 8); /* position within byte (MSB-first) */

        /*
           If starting a new byte, initialize it to 0
        */
        if (bw->bit_count % 8 == 0)
            bw->data[byte_index] = 0;

        /*
           Set the bit at the correct position
        */
        bw->data[byte_index] |= (bit << bit_offset);

        /* Move to next bit position */
        bw->bit_count++;
    }
}

void bitreader_init(BitReader *br, const unsigned char *data, size_t bits) {
    br->data = data;
    br->bit_count = bits;
    br->position = 0;
}

void bitreader_read_bits(BitReader *br, int k, unsigned int *out) {
    unsigned int result = 0; /* stores the reconstructed k-bit value */
    size_t byte_index;
    int bit, bit_offset;
    int i;

    /* Check if enough bits remain to read k bits */
    if (br->position + k > br->bit_count)
        return;

    /*
       Step 1: Read k bits sequentially
    */
    for (i = 0; i < k; i++) {

        /* Determine current byte and bit position */
        byte_index = br->position / 8;       /* which byte */
        bit_offset = 7 - (br->position % 8); /* bit position (MSB-first) */

        /* Extract the bit from the byte */
        bit = (br->data[byte_index] >> bit_offset) & 1;

        /* Shift result left and append the bit */
        result = (result << 1) | bit;

        /* Move to next bit position */
        br->position++;
    }

    /* Store the final k-bit result */
    *out = result;

    return;
}

void bitwriter_free(BitWriter *bw) {
    free(bw->data);
}
