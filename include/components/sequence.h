#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <stddef.h>
#include "parsers/json/config.h"

typedef enum {
    SEQUENCE_OK = 0,
    SEQUENCE_ERR_NULL_ARG,
    SEQUENCE_ERR_INVALID_CONFIG,
    SEQUENCE_ERR_INVALID_FRAME_NUMBER,
    SEQUENCE_ERR_INVALID_RANGE,
    SEQUENCE_ERR_PATH_TOO_LONG
} SequenceError;

/**
 * Validate the sequence-related parts of EngineConfig
 * Do no check whether files actually exists here.
 */

SequenceError sequence_validate(const EngineConfig * config);

/**
 * Compute total number of frames in the configured sequence
 * 
 * Example:
 *  start_frame = 1, end_frame = 120 --> count = 120
 */

SequenceError sequence_frame_count(const EngineConfig * config, unsigned int * out_count);

/**
 * Convert an absolute frame number into 0-based relative index.
 * 
 * Example:
 *  start_frame = 10, frame_number = 12 -> relative_index = 2
 */

SequenceError sequence_relative_index(const EngineConfig * config, unsigned int frame_number, unsigned int * out_index);

/**
 * Build BMP path for a frame number
 * 
 * Example:
 *  frames_dir      = "frames"
 *  frame_prefix    = "frame_"
 *  frame_digits    = 4
 *  frame_extension = ".bmp"
 *  frame_number    = 7
 * 
 * Produces:
 *  "frames/frame_0007.bmp"
 */

SequenceError sequence_build_frame_path(const EngineConfig * config, unsigned int frame_number, char * out_path, size_t out_size);

/**
 * Convert error code to human-readable string
 */
const char * sequence_error_string(SequenceError err);

#endif
