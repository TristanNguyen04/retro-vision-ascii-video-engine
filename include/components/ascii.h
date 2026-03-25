#ifndef ASCII_H
#define ASCII_H

#include "parsers/bmp.h"

typedef enum {
    ASCII_OK = 0,
    ASCII_ERR_NULL_ARG,
    ASCII_ERR_INVALID_IMAGE,
    ASCII_ERR_INVALID_PALETTE,
    ASCII_ERR_MEMORY
} AsciiError;

typedef struct{
    char ** lines;
    unsigned int line_count;
    unsigned int width;
    unsigned int height;
} AsciiFrame;

/**
 * Initialize an AsciiFrame to a safe empty state
 */

void ascii_init(AsciiFrame * frame);

/**
 * Free memory owned by AsciiFrame and reset it
 * Safe to call multiple times
 */

void ascii_free(AsciiFrame * frame);

/**
 * Convert one BGR pixel to grayscale in range [0, 255]
 */

unsigned int ascii_grayscale_from_bgr(unsigned char b, unsigned char g, unsigned char r);

/**
 * Map grayscale in range [0, 255] to one character from palette
 * 
 * Palette rules:
 *  - palette must be non-empty
 *  - darker pixels map to earlier palette characters
 *  - brigher pixels map to later palette characters
 */
char ascii_map_gray_to_char(unsigned int gray, const char * palette);

/**
 * Render a BMP image into ASCII lines using the given palette
 * 
 * Output:
 *  - frame->lines contains one null-terminated string per image row
 *  - frame->width equals bmp->width
 *  - frame->height equals bmp->height
 *  - frame->line_count equals bmp->height
 * 
 * On success:
 *  - returns ASCII_OK
 *  - caller must call ascii_free()
 * 
 * On failure:
 *  - returns nonzero error code
 *  - frame is left in a safe state
 */

AsciiError ascii_render_image(const BmpImage * bmp, const char * palette, AsciiFrame * frame);

/**
 * Same as ascii_render_image, but allows a highlighted mode
 * 
 * Current behaviour:
 *  - if highlight is nonzero, brightness is shifted upward slightly
 *  - if highlight is zero, normal rendering is used
 * 
 * MVP version: This keeps the highlight logic simple and visible without requiring a second palette
 */

AsciiError ascii_render_image_with_highlight(
    const BmpImage * bmp,
    const char * palette,
    int highlight,
    AsciiFrame * frame
);

/**
 * Convert error code to human-readable string
 */
const char * ascii_error_string(AsciiError err);

#endif
