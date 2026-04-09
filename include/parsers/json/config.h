#ifndef CONFIG_H
#define CONFIG_H

#include "parsers/json/json.h"

typedef enum {
    CONFIG_OK = 0,
    CONFIG_ERR_NULL_ARG,
    CONFIG_ERR_JSON_LOAD,
    CONFIG_ERR_MISSING_KEY,
    CONFIG_ERR_UNKNOWN_KEY,
    CONFIG_ERR_WRONG_TYPE,
    CONFIG_ERR_STRING_TOO_LONG,
    CONFIG_ERR_INVALID_VALUE,
    CONFIG_ERR_INVALID_RANGE
} ConfigError;

typedef struct {
    char frames_dir[256];
    char frame_prefix[64];
    char frame_extension[16];
    unsigned int frame_digits;

    unsigned int fps;
    unsigned int start_frame;
    unsigned int end_frame;

    char palette[128];
    unsigned int threshold;

    char compress_algorithm[64];
    unsigned int huffman_K;

} EngineConfig;

/**
 * Initialize a EngineConfig to a safe empty state
 * Do not apply defaults here yet.
 */
void config_init(EngineConfig *config);

/**
 * Load Retro-Vision engine config from JSON file.
 *
 * Behaviour:
 *      - parses JSON through the generic JSON layer
 *      - applies project defaults for attributes here
 *      - validates required keys, allowed keys, types, and value ranges
 *
 *  On success:
 *      - returns CONFIG_OK
 *      - fills 'config'
 *
 *  On failure:
 *      - returns nonzero error code
 *      - leaves 'config' in a safe state
 */
ConfigError config_load(const char *filename, EngineConfig *config);

/**
 * Convert error code to human-readable string.
 */
const char *config_error_string(ConfigError err);

#endif
