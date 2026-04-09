#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <stdlib.h>

typedef struct {
    unsigned char *data;
    size_t bit_count;
    size_t capacity_bytes;
} BitWriter;

typedef struct {
    const unsigned char *data;
    size_t bit_count;
    size_t position;
} BitReader;

void bitwriter_init(BitWriter *bw);

void bitwriter_write_bits(BitWriter *bw, unsigned int bits, int count);

void bitreader_init(BitReader *br, const unsigned char *data, size_t bits);

void bitreader_read_bits(BitReader *br, int k, unsigned int *out);

void bitwriter_free(BitWriter *bw);

#endif