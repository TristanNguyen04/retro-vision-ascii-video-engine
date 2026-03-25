#include "parsers/json/config.h"
#include "parsers/json/json.h"

#include <string.h>

#define CONFIG_DEFAULT_FRAME_PREFIX "frame_"
#define CONFIG_DEFAULT_FRAME_EXTENSION ".bmp"
#define CONFIG_DEFAULT_FRAME_DIGITS 4U

static void config_apply_defaults(EngineConfig * config);
static ConfigError config_validate_allowed_keys(const JsonObject * obj);
static int config_is_allowed_key(const char * key);

static ConfigError config_get_string(const JsonObject * obj, const char * key, char * dst, size_t dst_size, int optional);
static ConfigError config_copy_string_value(const JsonPair * pair, char * dst, size_t dst_size);

static ConfigError config_get_uint(const JsonObject * obj, const char * key, unsigned int * out_value, int optional);
static ConfigError config_copy_uint_value(const JsonPair * pair, unsigned int * out_value);

static ConfigError config_validate(const EngineConfig * config);

void config_init(EngineConfig * config){
    if(config == NULL){
        return;
    }

    config->frames_dir[0] = '\0';
    config->frame_prefix[0] = '\0';
    config->frame_extension[0] = '\0';
    config->frame_digits = 0U;

    config->fps = 0U;
    config->start_frame = 0U;
    config->end_frame = 0U;

    config->palette[0] = '\0';
    config->threshold = 0U;
}

ConfigError config_load(const char * filename, EngineConfig * config){
    JsonObject obj;
    JsonError json_err;
    ConfigError config_err;

    if(filename == NULL || config == NULL){
        return CONFIG_ERR_NULL_ARG;
    }

    config_init(config);
    json_init(&obj);

    json_err = json_load(filename, &obj);
    if(json_err != JSON_OK){
        json_free(&obj);
        return CONFIG_ERR_JSON_LOAD;
    }

    json_err = config_validate_allowed_keys(&obj);
    if(json_err != CONFIG_OK){
        json_free(&obj);
        config_init(config);
        return json_err;
    }

    config_apply_defaults(config);

    json_err = config_get_string(&obj, "frames_dir", config->frames_dir, sizeof(config->frames_dir), 0);
    if(json_err != JSON_OK){
        json_free(&obj);
        config_init(config);
        return json_err;
    }

    json_err = config_get_uint(&obj, "fps", &config->fps, 0);
    if(json_err != JSON_OK){
        json_free(&obj);
        config_init(config);
        return json_err;
    }
    
    json_err = config_get_uint(&obj, "start_frame", &config->start_frame, 0);
    if(json_err != JSON_OK){
        json_free(&obj);
        config_init(config);
        return json_err;
    }

    json_err = config_get_uint(&obj, "end_frame", &config->end_frame, 0);
    if(json_err != JSON_OK){
        json_free(&obj);
        config_init(config);
        return json_err;
    }

    json_err = config_get_string(&obj, "palette", config->palette, sizeof(config->palette), 0);
    if(json_err != JSON_OK){
        json_free(&obj);
        config_init(config);
        return json_err;
    }

    json_err = config_get_uint(&obj, "threshold", &config->threshold, 0);
    if(json_err != JSON_OK){
        json_free(&obj);
        config_init(config);
        return json_err;
    }

    json_err = config_get_string(&obj, "frame_prefix", config->frame_prefix, sizeof(config->frame_prefix), 1);
    if(json_err != JSON_OK){
        json_free(&obj);
        config_init(config);
        return json_err;
    }

    json_err = config_get_string(&obj, "frame_extension", config->frame_extension, sizeof(config->frame_extension), 1);
    if(json_err != JSON_OK){
        json_free(&obj);
        config_init(config);
        return json_err;
    }

    json_err = config_get_uint(&obj, "frame_digits", &config->frame_digits, 1);
    if(json_err != JSON_OK){
        json_free(&obj);
        config_init(config);
        return json_err;
    }

    json_err = config_validate(config);
    if(json_err != JSON_OK){
        json_free(&obj);
        config_init(config);
        return json_err;
    }

    json_free(&obj);

    return CONFIG_OK;
}

const char *config_error_string(ConfigError err) {
    switch (err) {
        case CONFIG_OK:
            return "success";
        case CONFIG_ERR_NULL_ARG:
            return "null argument";
        case CONFIG_ERR_JSON_LOAD:
            return "failed to load JSON config";
        case CONFIG_ERR_MISSING_KEY:
            return "missing required config key";
        case CONFIG_ERR_UNKNOWN_KEY:
            return "unknown config key";
        case CONFIG_ERR_WRONG_TYPE:
            return "wrong config value type";
        case CONFIG_ERR_STRING_TOO_LONG:
            return "config string too long";
        case CONFIG_ERR_INVALID_VALUE:
            return "invalid config value";
        case CONFIG_ERR_INVALID_RANGE:
            return "invalid config range";
        default:
            break;
    }

    return "unknown config error";
}

static void config_apply_defaults(EngineConfig * config){
    config_init(config);

    strncpy(config->frame_prefix, CONFIG_DEFAULT_FRAME_PREFIX, strlen(CONFIG_DEFAULT_FRAME_PREFIX) + 1);
    config->frame_prefix[strlen(CONFIG_DEFAULT_FRAME_PREFIX)] = '\0';

    strncpy(config->frame_extension, CONFIG_DEFAULT_FRAME_EXTENSION, strlen(CONFIG_DEFAULT_FRAME_EXTENSION) + 1);
    config->frame_prefix[strlen(CONFIG_DEFAULT_FRAME_EXTENSION)] = '\0';

    config->frame_digits = CONFIG_DEFAULT_FRAME_DIGITS;
}

static ConfigError config_validate_allowed_keys(const JsonObject * obj){
    size_t i;

    if(obj == NULL){
        return CONFIG_ERR_NULL_ARG;
    }

    for(i = 0U; i < obj->count; i++){
        if(!config_is_allowed_key(obj->pairs[i].key)){
            return CONFIG_ERR_UNKNOWN_KEY;
        }
    }

    return CONFIG_OK;
}

static int config_is_allowed_key(const char * key){
    if(key == NULL){
        return 0;
    }

    return strcmp(key, "frames_dir") == 0 ||
           strcmp(key, "frame_prefix") == 0 ||
           strcmp(key, "frame_extension") == 0 ||
           strcmp(key, "frame_digits") == 0 ||
           strcmp(key, "fps") == 0 ||
           strcmp(key, "start_frame") == 0 ||
           strcmp(key, "end_frame") == 0 ||
           strcmp(key, "palette") == 0 ||
           strcmp(key, "threshold") == 0;
}

static ConfigError config_get_string(const JsonObject * obj, const char * key, char * dst, size_t dst_size, int optional){
    const JsonPair * pair;

    if(obj == NULL || key == NULL || dst == NULL || dst_size == 0U){
        return CONFIG_ERR_NULL_ARG;
    }

    pair = json_find_pair(obj, key);

    if(pair == NULL && optional == 0){
        return CONFIG_ERR_MISSING_KEY;
    } else if(pair == NULL && optional == 1){
        return CONFIG_OK;
    }

    return config_copy_string_value(pair, dst, dst_size);
}

static ConfigError config_copy_string_value(const JsonPair * pair, char * dst, size_t dst_size){
    size_t len;

    if(pair == NULL || dst == NULL || dst_size == 0U){
        return CONFIG_ERR_NULL_ARG;
    }

    if(pair->type != JSON_VALUE_STRING){
        return CONFIG_ERR_WRONG_TYPE;
    }

    len = strlen(pair->string_value);
    if(len >= dst_size){
        return CONFIG_ERR_STRING_TOO_LONG;
    }

    strncpy(dst, pair->string_value, strlen(pair->string_value) + 1);
    dst[strlen(pair->string_value)] = '\0';

    return CONFIG_OK;
}

static ConfigError config_get_uint(const JsonObject * obj, const char * key, unsigned int * out_value, int optional){
    const JsonPair * pair;

    if(obj == NULL || key == NULL){
        return CONFIG_ERR_NULL_ARG;
    }

    pair = json_find_pair(obj, key);
    if(pair == NULL && optional == 0){
        return CONFIG_ERR_MISSING_KEY;
    } else if(pair ==  NULL && optional == 1){
        return CONFIG_OK;
    }

    return config_copy_uint_value(pair, out_value);
}

static ConfigError config_copy_uint_value(const JsonPair * pair, unsigned int * out_value){
    if(pair == NULL || out_value == NULL){
        return CONFIG_ERR_NULL_ARG;
    }

    if(pair->type != JSON_VALUE_INT){
        return CONFIG_ERR_WRONG_TYPE;
    }

    if(pair->int_value < 0){
        return CONFIG_ERR_INVALID_VALUE;
    }

    *out_value = (unsigned int) pair->int_value;

    return CONFIG_OK;
}

static ConfigError config_validate(const EngineConfig * config){
    if(config == NULL){
        return CONFIG_ERR_NULL_ARG;
    }

    if(config->frames_dir[0] == '\0'){
        return CONFIG_ERR_INVALID_VALUE;
    }

    if(config->frames_dir[0] == '\0'){
        return CONFIG_ERR_INVALID_VALUE;
    }

    if(strcmp(config->frame_extension, ".bmp") != 0){
        return CONFIG_ERR_INVALID_VALUE;
    }

    if(config->frame_digits > 4U){
        return CONFIG_ERR_INVALID_VALUE;
    }

    if(config->fps == 0U){
        return CONFIG_ERR_INVALID_VALUE;
    }

    if(config->start_frame == 0U){
        return CONFIG_ERR_INVALID_VALUE;
    }

    if(config->end_frame < config->start_frame){
        return CONFIG_ERR_INVALID_RANGE;
    }

    if(strlen(config->palette) == 0U){
        return CONFIG_ERR_INVALID_VALUE;
    }

    return CONFIG_OK;
}
