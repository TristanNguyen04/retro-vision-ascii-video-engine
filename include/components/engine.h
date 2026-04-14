#ifndef ENGINE_H
#define ENGINE_H

typedef enum{
    ENGINE_OK = 0,
    ENGINE_ERR_NULL_ARG,
    ENGINE_ERR_CONFIG,
    ENGINE_ERR_SEQUENCE,
    ENGINE_ERR_WAV,
    ENGINE_ERR_BMP,
    ENGINE_ERR_ASCII,
    ENGINE_ERR_RENDER_OPEN,
    ENGINE_ERR_LOG_OPEN,
    ENGINE_ERR_RENDER_WRITE,
    ENGINE_ERR_LOG_WRITE,
    ENGINE_ERR_INTERNAL
} EngineError;

/**
 * Central engine to run end-to-end.
 * 
 * Inputs:
 *  - config_path: validated through config layer
 *  - wav_path: audio source
 *  - render_path: output render text file
 *  - log_path: processing log file
 * 
 * Behaviour:
 *  - loads config and WAV
 *  - iterates configured frame range (via config file)
 *  - loads BMP frames from generated paths
 *  - maps frame index to WAV sample window
 *  - computes highlight state
 *  - renders ASCII
 *  - writes render output and log
 */

EngineError engine_run(
    const char * config_path,
    const char * wav_path,
    const char * render_path,
    const char * render_compress_path, 
    const char * log_path
);

/**
 * Convert error code to human-readable string
 */
const char * engine_error_string(EngineError err);

#endif
