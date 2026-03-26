#ifndef RENDER_H
#define RENDER_H

#include <stdio.h>
#include "components/ascii.h"

typedef enum {
    RENDER_OK = 0,
    RENDER_ERR_NULL_ARG,
    RENDER_ERR_INVALID_FRAME,
    RENDER_ERR_WRITE_FAILED
} RenderError;

/**
 * Vaidate an AsciiFrame for rendering later
 */

RenderError render_validate_frame(const AsciiFrame * frame);

/**
 * Write one ASCII frame block to the given output file.
 * 
 * Output format:
 *  FRAME <number>
 *  TIME <seconds>
 *  HIGHLIGHT <0|1>
 *  WIDTH <width>
 *  HEIGHT <height>
 *  <line 0>
 *  <line 1>
 *  ...
 *  <line N>
 *  <blank line>
 */

RenderError render_write_frame(
    FILE * fp,
    unsigned int frame_number,
    double timestamp,
    int highlight,
    const AsciiFrame * frame
);

/**
 * Convert error code to human-readable string
 */
const char * render_error_string(RenderError err);

#endif
