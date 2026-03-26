#include "components/render.h"

#include <string.h>

RenderError render_validate_frame(const AsciiFrame * frame){
    unsigned int i;

    if(frame == NULL){
        return RENDER_ERR_NULL_ARG;
    }

    if(frame->lines == NULL){
        return RENDER_ERR_INVALID_FRAME;
    }

    if(frame->width == 0U || frame->height == 0U){
        return RENDER_ERR_INVALID_FRAME;
    }

    if(frame->line_count != frame->height){
        return RENDER_ERR_INVALID_FRAME;
    }

    for(i = 0U; i < frame->line_count; i++){
        if(frame->lines[i] == NULL){
            return RENDER_ERR_INVALID_FRAME;
        }

        if(strlen(frame->lines[i]) != frame->width){
            return RENDER_ERR_INVALID_FRAME;
        }
    }

    return RENDER_OK;
}

RenderError render_write_frame(
    FILE * fp,
    unsigned int frame_number,
    double timestamp,
    int highlight,
    const AsciiFrame * frame
){
    unsigned int i;
    RenderError err;

    if(fp == NULL || frame == NULL){
        return RENDER_ERR_NULL_ARG;
    }

    err = render_validate_frame(frame);
    if(err != RENDER_OK){
        return err;
    }

    if(fprintf(fp, "FRAME %u\n", frame_number) < 0){
        return RENDER_ERR_WRITE_FAILED;
    }

    if(fprintf(fp, "TIME %.3f\n", timestamp) < 0){
        return RENDER_ERR_WRITE_FAILED;
    }

    if(fprintf(fp, "HIGHLIGHT %d\n", highlight ? 1 : 0) < 0){
        return RENDER_ERR_WRITE_FAILED;
    }

    if(fprintf(fp, "WIDTH %u\n", frame->width) < 0){
        return RENDER_ERR_WRITE_FAILED;
    }

    if(fprintf(fp, "HEIGHT %u\n", frame->height) < 0){
        return RENDER_ERR_WRITE_FAILED;
    }

    for(i = 0U; i < frame->line_count; i++){
        if(fprintf(fp, "%s\n", frame->lines[i]) < 0){
            return RENDER_ERR_WRITE_FAILED;
        }
    }

    if(fprintf(fp, "\n") < 0){
        return RENDER_ERR_WRITE_FAILED;
    }

    return RENDER_OK;
}

const char * render_error_string(RenderError err){
    switch(err){
        case RENDER_OK:
            return "[RENDER] success";
        case RENDER_ERR_NULL_ARG:
            return "[RENDER] null argument";
        case RENDER_ERR_INVALID_FRAME:
            return "[RENDER] invalid ASCII frame";
        case RENDER_ERR_WRITE_FAILED:
            return "[RENDER] failed to write render output";
        default:
            break;
    }

    return "[RENDER] unknown render error";
}

