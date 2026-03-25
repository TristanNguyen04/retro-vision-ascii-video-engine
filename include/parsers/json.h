#ifndef JSON_H
#define JSON_H

#include <stddef.h>

#define JSON_MAX_KEY_STRENGTH 63
#define JSON_MAX_STRING_LENGTH 255

typedef enum {
    JSON_VALUE_STRING = 0,
    JSON_VALUE_INT
} JsonValueType;

typedef enum {
    JSON_OK = 0,
    JSON_ERR_NULL_ARG,
    JSON_ERR_OPEN_FAILED,
    JSON_ERR_READ_FAILED,
    JSON_ERR_MEMORY,
    JSON_ERR_EMPTY_INPUT,
    JSON_ERR_EXPECTED_OBJECT_START,
    JSON_ERR_EXPECTED_STRING_KEY,
    JSON_ERR_EXPECTED_COLON,
    JSON_ERR_EXPECTED_VALUE,
    JSON_ERR_EXPECTED_COMMA_OR_OBJECT_END,
    JSON_ERR_DUPLICATE_KEY,
    JSON_ERR_KEY_TOO_LONG,
    JSON_ERR_STRING_TOO_LONG,
    JSON_ERR_INVALID_STRING,
    JSON_ERR_INVALID_NUMBER,
    JSON_ERR_UNSUPPORTED_VALUE,
    JSON_ERR_TRAILING_CONTENT
} JsonError;

typedef struct {
    JsonValueType type;
    char key[JSON_MAX_KEY_STRENGTH + 1];
    char string_value[JSON_MAX_STRING_LENGTH + 1];
    int int_value;
} JsonPair;

typedef struct JsonPair {
    JsonPair * pairs;
    size_t count;
} JsonObject;

/**
 * Initialize a JsonObject to a safe empty state
 */

void json_init(JsonObject * obj);

/**
 * Free memory owned by JsonObject and reset it
 * Safe to call mutiple times
 */

void json_free(JsonObject * obj);

/**
 * Parse a restricted flat JSON object from file.
 * 
 * Supported subset:
 *  - one top-level objec
 *  - string keys
 *  - string values
 *  - integer values
 *  - no nesting
 *  - no arrays
 * 
 *  On success:
 *      - returns JSON_OK
 *      - fills 'obj'
 *  
 *  On failure:
 *      - returns nonzero error code
 *      - leaves 'obj' in a safe state
 */

JsonError json_load(const char * filename, JsonObject * obj);


/**
 * Look up a key in the parsed object.
 * Returns a pointer to pair if found, otherwise NULL
 */

const JsonPair * json_find_pair(const JsonObject * obj, const char * key);

/**
 * Convert error code to human-readable string.
 */

const char * json_error_string(JsonError err);

#endif
