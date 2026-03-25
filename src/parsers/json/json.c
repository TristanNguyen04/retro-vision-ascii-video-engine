#include "parsers/json/json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    const char * text;
    size_t length;
    size_t pos;
} JsonParser;

static void json_reset(JsonObject * obj);

static JsonError json_read_file(const char * filename, char ** buffer, size_t * length);

static void json_skip_whitespace(JsonParser * parser);
static int json_has_duplicate_key(const JsonObject * obj, const char * key);
static int json_peek(const JsonParser * parser);
static int json_consume(JsonParser * parser, char ch);

static JsonError json_parse_object(JsonParser * parser, JsonObject * obj);
static JsonError json_parse_string(JsonParser * parser, char * out, size_t out_capacity, JsonError too_long_error);
static JsonError json_parse_int(JsonParser * parser, int * out_value);

static JsonError json_add_pair(JsonObject * obj, const JsonPair * pair);


void json_init(JsonObject * obj){
    if(obj == NULL){
        return;
    }

    json_reset(obj);
}

void json_free(JsonObject * obj){
    if(obj == NULL){
        return;
    }

    free(obj->pairs);
    obj->pairs = NULL;
    obj->count = 0U;
}

JsonError json_load(const char * filename, JsonObject * obj){
    char * buffer;
    size_t length;
    JsonParser parser;
    JsonError err;

    if(filename == NULL || obj == NULL){
        return JSON_ERR_NULL_ARG;
    }

    json_free(obj);

    buffer = NULL;
    length = 0U;

    err = json_read_file(filename, &buffer, &length);
    if(err != JSON_OK){
        return err;
    }

    if(length == 0U){
        free(buffer);
        return JSON_ERR_EMPTY_INPUT;
    }

    parser.text = buffer;
    parser.length = length;
    parser.pos = 0U;

    err = json_parse_object(&parser, obj);
    if(err != JSON_OK){
        free(buffer);
        json_free(obj);
        return err;
    }

    json_skip_whitespace(&parser);
    if(parser.pos != parser.length){
        free(buffer);
        json_free(obj);
        return JSON_ERR_TRAILING_CONTENT;
    }

    free(buffer);
    return JSON_OK;
}

const JsonPair * json_find_pair(const JsonObject * obj, const char * key){
    size_t i;

    if(obj == NULL || key == NULL){
        return NULL;
    }

    for(i = 0U; i < obj->count; i++){
        if(strcmp(obj->pairs[i].key, key) == 0){
            return &obj->pairs[i];
        }
    }

    return NULL;
}

const char *json_error_string(JsonError err) {
    switch (err) {
        case JSON_OK:
            return "success";
        case JSON_ERR_NULL_ARG:
            return "null argument";
        case JSON_ERR_OPEN_FAILED:
            return "failed to open JSON file";
        case JSON_ERR_READ_FAILED:
            return "failed to read JSON file";
        case JSON_ERR_MEMORY:
            return "memory allocation failed";
        case JSON_ERR_EMPTY_INPUT:
            return "empty JSON input";
        case JSON_ERR_EXPECTED_OBJECT_START:
            return "expected '{'";
        case JSON_ERR_EXPECTED_STRING_KEY:
            return "expected string key";
        case JSON_ERR_EXPECTED_COLON:
            return "expected ':'";
        case JSON_ERR_EXPECTED_VALUE:
            return "expected value";
        case JSON_ERR_EXPECTED_COMMA_OR_OBJECT_END:
            return "expected ',' or '}'";
        case JSON_ERR_DUPLICATE_KEY:
            return "duplicate key";
        case JSON_ERR_KEY_TOO_LONG:
            return "JSON key too long";
        case JSON_ERR_STRING_TOO_LONG:
            return "JSON string too long";
        case JSON_ERR_INVALID_STRING:
            return "invalid JSON string";
        case JSON_ERR_INVALID_NUMBER:
            return "invalid JSON number";
        case JSON_ERR_UNSUPPORTED_VALUE:
            return "unsupported JSON value";
        case JSON_ERR_TRAILING_CONTENT:
            return "trailing content after JSON object";
        default:
            break;
    }

    return "unknown JSON error";
}

static void json_reset(JsonObject * obj){
    obj->pairs = NULL;
    obj->count = 0U;
}

static JsonError json_read_file(const char * filename, char ** buffer, size_t * length){
    FILE * fp;
    long file_size;
    size_t bytes_read;

    if(filename == NULL || buffer == NULL || length == NULL){
        return JSON_ERR_NULL_ARG;
    }

    *buffer = NULL;
    *length = 0U;

    fp = fopen(filename, "rb");
    if(fp == NULL){
        return JSON_ERR_OPEN_FAILED;
    }

    if(fseek(fp, 0L, SEEK_END) != 0){
        fclose(fp);
        return JSON_ERR_READ_FAILED;
    }

    file_size = ftell(fp);
    if(file_size < 0L){
        fclose(fp);
        return JSON_ERR_READ_FAILED;
    }

    if(fseek(fp, 0L, SEEK_SET) != 0){
        fclose(fp);
        return JSON_ERR_READ_FAILED;
    }

    *buffer = (char *) malloc((size_t)file_size + 1U);
    if(*buffer == NULL){
        fclose(fp);
        return JSON_ERR_MEMORY;
    }

    bytes_read = fread(*buffer, 1U, (size_t)file_size, fp);
    fclose(fp);

    if(bytes_read != (size_t)file_size){
        free(*buffer);
        *buffer = NULL;
        return JSON_ERR_READ_FAILED;
    }

    (*buffer)[bytes_read] = '\0';
    *length = bytes_read;

    return JSON_OK;
}

static void json_skip_whitespace(JsonParser * parser){
    while(parser->pos < parser->length && isspace((unsigned char) parser->text[parser->pos])){
        parser->pos++;
    }
}

static int json_has_duplicate_key(const JsonObject * obj, const char * key){
    size_t i;

    if(obj == NULL || key == NULL){
        return 0;
    }

    for(i = 0U; i < obj->count; i++){
        if(strcmp(obj->pairs[i].key, key) == 0){
            return 1;
        }
    }

    return 0;
}

static int json_peek(const JsonParser * parser){
    if(parser->pos >= parser->length){
        return EOF;
    }

    return (unsigned char) parser->text[parser->pos];
}

static int json_consume(JsonParser * parser, char ch){
    if(parser->pos < parser->length && parser->text[parser->pos] == ch){
        parser->pos++;
        return 1;
    }

    return 0;
}

static JsonError json_parse_object(JsonParser * parser, JsonObject * obj){
    JsonPair pair;
    JsonError err;
    int ch;

    json_skip_whitespace(parser);

    if(!json_consume(parser, '{')){
        return JSON_ERR_EXPECTED_OBJECT_START;
    }

    json_skip_whitespace(parser);

    if(json_consume(parser, '}')){
        return JSON_OK;
    }

    for(;;){
        memset(&pair, 0, sizeof(pair));

        json_skip_whitespace(parser);

        if(json_peek(parser) != '"'){
            return JSON_ERR_EXPECTED_STRING_KEY;
        }

        err = json_parse_string(parser, pair.key, sizeof(pair.key), JSON_ERR_KEY_TOO_LONG);

        if(err != JSON_OK){
            return err;
        }

        if(json_has_duplicate_key(obj, pair.key)){
            return JSON_ERR_DUPLICATE_KEY;
        }

        json_skip_whitespace(parser);

        if(!json_consume(parser, ':')){
            return JSON_ERR_EXPECTED_COLON;
        }

        json_skip_whitespace(parser);
        ch = json_peek(parser);

        if(ch == '"'){
            pair.type = JSON_VALUE_STRING;
            err = json_parse_string(parser, pair.string_value, sizeof(pair.string_value), JSON_ERR_STRING_TOO_LONG);

            if(err != JSON_OK){
                return err;
            }
        } else if(ch == '-' || isdigit((unsigned char) ch)){
            pair.type = JSON_VALUE_INT;
            err = json_parse_int(parser, &pair.int_value);

            if(err != JSON_OK){
                return err;
            }
        } else {
            return JSON_ERR_EXPECTED_VALUE;
        }

        err = json_add_pair(obj, &pair);

        if(err != JSON_OK){
            return err;
        }

        json_skip_whitespace(parser);

        if(json_consume(parser, '}')){
            break;
        }

        if(!json_consume(parser, ',')){
            return JSON_ERR_EXPECTED_COMMA_OR_OBJECT_END;
        }
    }

    return JSON_OK;
}

static JsonError json_parse_string(JsonParser * parser, char * out, size_t out_capacity, JsonError too_long_error){
    size_t out_len;
    int ch;

    if(parser == NULL || out == NULL || out_capacity == 0U){
        return JSON_ERR_NULL_ARG;
    }

    if(!json_consume(parser, '"')){
        return JSON_ERR_INVALID_STRING;
    }

    out_len = 0U;

    while(parser->pos < parser->length){
        ch = (unsigned char) parser->text[parser->pos++];

        if(ch == '"'){
            out[out_len] = '\0';
            return JSON_OK;
        }

        if(ch == '\\'){
            if(parser->pos >= parser->length){
                return JSON_ERR_INVALID_STRING;
            }

            ch = (unsigned char) parser->text[parser->pos++];

            if(ch == '"' || ch == '\\' || ch == '/'){
                /* supported minimal escapes */
            } else if(ch == 'n'){
                ch = '\n';
            } else if(ch == 't'){
                ch = '\t';
            } else {
                return JSON_ERR_INVALID_STRING;
            }
        }

        if((unsigned char) ch < 0x20U){
            return JSON_ERR_INVALID_STRING;
        }

        if(out_len + 1U >= out_capacity){
            return too_long_error;
        }

        out[out_len++] = (char) ch;
    }

    return JSON_ERR_INVALID_STRING;
}

static JsonError json_parse_int(JsonParser * parser, int * out_value){
    long value;
    int negative;
    int ch;

    if(parser == NULL || out_value == NULL){
        return JSON_ERR_NULL_ARG;
    }

    value = 0L;
    negative = 0;

    if(json_consume(parser, '-')){
        negative = 1;
    }

    ch = json_peek(parser);
    if(!isdigit((unsigned char) ch)){
        return JSON_ERR_INVALID_NUMBER;
    }

    while(parser->pos < parser->length){
        ch = json_peek(parser);
        if(!isdigit((unsigned char) ch)){
            break;
        }

        value = (value * 10L) + (long)(ch - '0');

        parser->pos++;
    }

    if(negative == 1){
        value = -value;
    }

    *out_value = (int) value;

    return JSON_OK;
}

static JsonError json_add_pair(JsonObject * obj, const JsonPair * pair){
    JsonPair * new_pairs;
    size_t new_count;

    if(obj == NULL || pair == NULL){
        return JSON_ERR_NULL_ARG;
    }

    new_count = obj->count + 1U;

    new_pairs = (JsonPair *) realloc(obj->pairs, new_count * sizeof(JsonPair));
    if(new_pairs == NULL){
        return JSON_ERR_MEMORY;
    }

    obj->pairs = new_pairs;
    obj->pairs[obj->count] = *pair;
    obj->count = new_count;

    return JSON_OK;
}
