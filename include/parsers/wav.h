#ifndef WAV_H
#define WAV_H

#include "common/io_utils.h"

/**
 * Support WAV subset:
 * - RIFF/WAVE
 * - PCM only
 * - 16-bit samples only
 * - mono or stereo
 */

/*
 * Supported WAV subset:
 * - RIFF/WAVE
 * - PCM only
 * - 16-bit samples only
 * - mono or stereo
 */

typedef enum {
    WAV_OK = 0,
    WAV_ERR_NULL_ARG,
    WAV_ERR_OPEN_FAILED,
    WAV_ERR_READ_FAILED,
    WAV_ERR_SEEK_FAILED,
    WAV_ERR_BAD_RIFF,
    WAV_ERR_BAD_WAVE,
    WAV_ERR_MISSING_FMT,
    WAV_ERR_MISSING_DATA,
    WAV_ERR_UNSUPPORTED_FORMAT,
    WAV_ERR_UNSUPPORTED_BITS,
    WAV_ERR_UNSUPPORTED_CHANNELS,
    WAV_ERR_INVALID_BLOCK_ALIGN,
    WAV_ERR_MEMORY
} WavError;

typedef struct {
    unsigned short audio_format;
    unsigned short num_channels;
    unsigned int sample_rate;
    unsigned int byte_rate;
    unsigned short block_align;
    unsigned short bits_per_sample;
    unsigned int data_size;
    unsigned int sample_count_per_channel;
    short *samples;
} WavFile;

/**
 * Initialize a WavFile structure to a safe empty state.
 * Safe to call on an already-zeroed struct.
 */

void wav_init(WavFile * wav);

/**
 * Parse a WAV file from disk to memory
 * 
 * On success:
 *  - returns WAV_OK
 *  - files out 'wav'
 *  - caller must call wav_free()
 * 
 * On failure:
 *  - returns nonzero error code
 *  - 'wav' is left in a safe state
 */

WavError wav_load(const char * filename, WavFile * wav);

/**
 * Release memory owned by the WavFile and reset it
 * Safe to call multiple times.
 */

void wav_free(WavFile * wav);

/**
 * Convert error code to human-readable string.
 */

const char * wav_error_string(WavError err);

/**
 * Compute the mean absolute amplitude over a half-open frame:
 * [start_sample, end_sample)
 * 
 * Sample indexes are per-channel frame indexes, not raw shorts.
 * For stereo, one frame contains left+right samples.
 * 
 * Returns 0 if range is empty or invalid.
 */

unsigned int wav_average_amplitude(
    const WavFile * wav,
    unsigned int start_sample,
    unsigned int end_sample
);

#endif
