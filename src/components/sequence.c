#include "components/sequence.h"

#include <stdio.h>
#include <string.h>

static int sequence_is_valid_frame_number(const EngineConfig * config, unsigned int frame_number);

SequenceError sequence_validate(const EngineConfig * config){
    size_t frames_dir_len;
    size_t frame_prefix_len;
    size_t frame_extension_len;

    if(config == NULL){
        return SEQUENCE_ERR_NULL_ARG;
    }

    frames_dir_len = strlen(config->frames_dir);
    frame_prefix_len = strlen(config->frame_prefix);
    frame_extension_len = strlen(config->frame_extension);

    if(frames_dir_len == 0U || frame_prefix_len == 0U || frame_extension_len == 0U){
        return SEQUENCE_ERR_INVALID_CONFIG;
    }

    if(config->frame_digits > 4U){
        return SEQUENCE_ERR_INVALID_CONFIG;
    }

    if(config->start_frame == 0U){
        return SEQUENCE_ERR_INVALID_RANGE;
    }

    if(config->end_frame < config->start_frame){
        return SEQUENCE_ERR_INVALID_RANGE;
    }

    return SEQUENCE_OK;
}

SequenceError sequence_frame_count(const EngineConfig * config, unsigned int * out_count){
    SequenceError err;

    if(config == NULL || out_count == NULL){
        return SEQUENCE_ERR_NULL_ARG;
    }

    err = sequence_validate(config);
    if(err != SEQUENCE_OK){
        return err;
    }

    *out_count = config->end_frame - config->start_frame + 1U;
    return SEQUENCE_OK;
}

SequenceError sequence_relative_index(const EngineConfig * config, unsigned int frame_number, unsigned int * out_index){
    SequenceError err;

    if(config == NULL || out_index == NULL){
        return SEQUENCE_ERR_NULL_ARG;
    }

    err = sequence_validate(config);
    if(err != SEQUENCE_OK){
        return err;
    }

    if(!sequence_is_valid_frame_number(config, frame_number)){
        return SEQUENCE_ERR_INVALID_FRAME_NUMBER;
    }

    *out_index = frame_number - config->start_frame;

    return SEQUENCE_OK;
}

SequenceError sequence_build_frame_path(
    const EngineConfig * config,
    unsigned int frame_number,
    char * out_path,
    size_t out_size
){
    int written;
    SequenceError err;

    if(config == NULL || out_path == NULL || out_size == 0U){
        return SEQUENCE_ERR_NULL_ARG;
    }

    err = sequence_validate(config);
    if(err != SEQUENCE_OK){
        return err;
    }
    
    if(!sequence_is_valid_frame_number(config, frame_number)){
        return SEQUENCE_ERR_INVALID_FRAME_NUMBER;
    }

    written = snprintf(
        out_path,
        out_size,
        "%s/%s%0*u%s",
        config->frames_dir,
        config->frame_prefix,
        (int)config->frame_digits,
        frame_number,
        config->frame_extension
    );

    

    if(written < 0 || (size_t)written >= out_size){
        if(out_size > 0U){
            out_path[0] = '\0';
        }
        return SEQUENCE_ERR_PATH_TOO_LONG;
    }

    return SEQUENCE_OK;
}

const char * sequence_error_string(SequenceError err){
    switch(err){
        case SEQUENCE_OK:
            return "success";
        case SEQUENCE_ERR_NULL_ARG:
            return "null argument";
        case SEQUENCE_ERR_INVALID_CONFIG:
            return "invalid sequence config";
        case SEQUENCE_ERR_INVALID_FRAME_NUMBER:
            return "invalid frame number";
        case SEQUENCE_ERR_INVALID_RANGE:
            return "invalid frame range";
        case SEQUENCE_ERR_PATH_TOO_LONG:
            return "sequence path too long";
        default:
            break;
    }

    return "unknown sequence error";
}

static int sequence_is_valid_frame_number(const EngineConfig * config, unsigned int frame_number){
    if(config == NULL){
        return 0;
    }

    return frame_number >= config->start_frame && frame_number <= config->end_frame;
}

