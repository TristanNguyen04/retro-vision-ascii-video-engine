#ifndef BMP_H
#define BMP_H

#include <stdio.h>

typedef enum {
    BMP_OK = 0,
    BMP_ERR_NULL_ARG,
    BMP_ERR_OPEN_FAILED,
    BMP_ERR_READ_FAILED,
    BMP_ERR_SEEK_FAILED,
    BMP_ERR_BAD_SIGNATURE,
    BMP_ERR_UNSUPPORTED_DIB,
    BMP_ERR_UNSUPPORTED_PLANES,
    BMP_ERR_UNSUPPORTED_COMPRESSION,
    BMP_ERR_UNSUPPORTED_BITS,
    BMP_ERR_INVALID_DIMENSIONS,
    BMP_ERR_INVALID_OFFSET,
    BMP_ERR_ROW_SIZE_OVERFLOW,
    BMP_ERR_IMAGE_SIZE_OVERFLOW,
    BMP_ERR_MEMORY
} BmpError;

typedef struct {
    unsigned int file_size;
    unsigned int pixel_offset;

    unsigned int dib_size;
    int width;
    int height;
    unsigned short planes;
    unsigned short bits_per_pixel;
    unsigned int compression;
    unsigned int image_size;

    unsigned int row_stride;
    int is_top_down;

    unsigned char * pixel_data;
} BmpImage;

/**
 * Initialize a BmpImage structure to a safe empty state
 */
void bmp_init(BmpImage * bmp);

/**
 * Release memory owned by the BmpImage and reset it
 * Safe to call multiple times
 */
void bmp_free(BmpImage * bmp);

/**
 * Load a "24-bit" uncompressed BMP file into memory
 * 
 * On success:
 *  - returns BMP_OK
 *  - fills out 'bmp'
 *  - pixel_data is stored in top-down visual row order
 *  - caller must call bmp_free()
 * 
 * On failure:
 *  - returns nonzero error code
 *  - 'bmp' is left in a safe state
 */
BmpError bmp_load(const char * filename, BmpImage * bmp);

/**
 * Read one visual row from an already opened BMP file
 * 
 * Requirements:
 *  - fp points to BMP file
 *  - bmp contains valid parsed metadata
 *  - row_buffer has size at least (width * 3) bytes
 * 
 * Output:
 *  - row_buffer receives one of 'BGR' pixels
 *  - padding bytes are skipped internally
 *  - row_index is in visual order:
 *      0 = top row
 *      height - 1 = bottom row
 */
BmpError bmp_read_row(FILE * fp, const BmpImage * bmp, unsigned int row_index, unsigned char * row_buffer);

/**
 * Convert error code to human-readable string
 */
const char * bmp_error_string(BmpError err);

#endif
