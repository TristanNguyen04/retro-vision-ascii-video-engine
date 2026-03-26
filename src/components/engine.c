#include "components/engine.h"

#include "components/sequence.h"
#include "components/ascii.h"
#include "components/render.h"

#include "parsers/json/config.h"
#include "parsers/wav.h"
#include "parsers/bmp.h"

#include <stdio.h>
#include <string.h>

typedef enum {
    ENGINE_STATE_INIT = 0,
    ENGINE_STATE_LOAD_CONFIG,
    ENGINE_STATE_LOAD_WAV,
    ENGINE_STATE_PREPARE_SEQUENCE,
    ENGINE_STATE_OPEN_OUTPUTS,
    ENGINE_STATE_PROCESS_FRAME,
    ENGINE_STATE_WRITE_SUMMARY,
    ENGINE_STATE_DONE,
    ENGINE_STATE_ERROR
} EngineState;

typedef struct {
    EngineConfig config;
    WavFile wav;

    FILE * render_fp;
    FILE * log_fp;

    unsigned int current_frame;
    unsigned int total_frames;

    EngineState state;
    EngineError error;
} EngineContext;

static void engine_init(EngineContext * ctx);
static void engine_cleanup(EngineContext * ctx);

static int engine_log(EngineContext * ctx, const char * message);
static int engine_log_config(EngineContext * ctx);
static int engine_log_wav(EngineContext * ctx);
static int engine_log_summary(EngineContext * ctx);

static EngineError engine_open_outputs(
    EngineContext * ctx, 
    const char * render_path,
    const char * log_path
);

static EngineError engine_prepare_sequence(EngineContext * ctx);
static EngineError engine_process_one_frame(EngineContext * ctx);

const char * engine_error_string(EngineError err){
    switch(err){
        case ENGINE_OK:
            return "[ENGINE] success";
        case ENGINE_ERR_NULL_ARG:
            return "[ENGINE] null argument";
        case ENGINE_ERR_CONFIG:
            return "[ENGINE] failed to load config";
        case ENGINE_ERR_SEQUENCE:
            return "[ENGINE] sequence error";
        case ENGINE_ERR_WAV:
            return "[ENGINE] failed to load WAV";
        case ENGINE_ERR_BMP:
            return "[ENGINE] failed to load BMP frame";
        case ENGINE_ERR_RENDER_OPEN:
            return "[ENGINE] failed to open render output";
        case ENGINE_ERR_LOG_OPEN:
            return "[ENGINE] failed to open log output";
        case ENGINE_ERR_RENDER_WRITE:
            return "[ENGINE] failed to write render output";
        case ENGINE_ERR_LOG_WRITE:
            return "[ENGINE] failed to write log output";
        case ENGINE_ERR_INTERNAL:
            return "[ENGINE] internal engine error";
        default:
            break;
    }

    return "[ENGINE] unknown engine error";
}

EngineError engine_run(
    const char * config_path,
    const char * wav_path,
    const char * render_path,
    const char * log_path
){
    EngineContext ctx;
    ConfigError config_err;
    WavError wav_err;

    if(
        config_path == NULL ||
        wav_path == NULL ||
        render_path == NULL ||
        log_path == NULL 
    ){
        return ENGINE_ERR_NULL_ARG;
    }

    engine_init(&ctx);

    ctx.state = ENGINE_STATE_LOAD_CONFIG;
    ctx.error = ENGINE_OK;

    while(ctx.state != ENGINE_STATE_DONE && ctx.state != ENGINE_STATE_ERROR){
        switch(ctx.state){
            case ENGINE_STATE_LOAD_CONFIG:
                config_err = config_load(config_path, &ctx.config);
                if(config_err != CONFIG_OK){
                    ctx.error = ENGINE_ERR_CONFIG;
                    ctx.state = ENGINE_STATE_ERROR;
                } else {
                    ctx.state = ENGINE_STATE_LOAD_WAV;
                }
                break;

            case ENGINE_STATE_LOAD_WAV:
                wav_err = wav_load(wav_path, &ctx.wav);
                if(wav_err != WAV_OK){
                    ctx.error = ENGINE_ERR_WAV;
                    ctx.state = ENGINE_STATE_ERROR;
                } else {
                    ctx.state = ENGINE_STATE_PREPARE_SEQUENCE;
                }
                break;

            case ENGINE_STATE_PREPARE_SEQUENCE:
                ctx.error = engine_prepare_sequence(&ctx);

                if(ctx.error != ENGINE_OK){
                    ctx.state = ENGINE_STATE_ERROR;
                } else {
                    ctx.state = ENGINE_STATE_OPEN_OUTPUTS;
                }
                break;
            
            case ENGINE_STATE_OPEN_OUTPUTS:
                ctx.error = engine_open_outputs(&ctx, render_path, log_path);

                if(ctx.error != ENGINE_OK){
                    ctx.state = ENGINE_STATE_ERROR;
                } else {
                    if(!engine_log(&ctx, "[ENGINE] engine started")){
                        ctx.error = ENGINE_ERR_LOG_WRITE;
                        ctx.state = ENGINE_STATE_ERROR;
                    } else if(!engine_log_config(&ctx)){
                        ctx.error = ENGINE_ERR_LOG_WRITE;
                        ctx.state = ENGINE_STATE_ERROR;
                    } else if(!engine_log_wav(&ctx)){
                        ctx.error = ENGINE_ERR_LOG_WRITE;
                        ctx.state = ENGINE_STATE_ERROR;
                    } else {
                        ctx.state = ENGINE_STATE_PROCESS_FRAME;
                    }
                }

                break;

            case ENGINE_STATE_PROCESS_FRAME:
                if(ctx.current_frame > ctx.config.end_frame){
                    ctx.state = ENGINE_STATE_WRITE_SUMMARY;
                } else {
                    ctx.error = engine_process_one_frame(&ctx);

                    if(ctx.error != ENGINE_OK){
                        ctx.state = ENGINE_STATE_ERROR;
                    } else {
                        ctx.current_frame++;
                    }
                }

                break;

            default:
                ctx.error = ENGINE_ERR_INTERNAL;
                ctx.state = ENGINE_STATE_ERROR;
                break;
        }          
    }

    engine_cleanup(&ctx);
    if(ctx.state == ENGINE_STATE_ERROR){
        return ctx.error;
    }

    return ENGINE_OK;
}

static void engine_init(EngineContext * ctx){
    if(ctx == NULL){
        return;
    }

    config_init(&ctx->config);
    wav_init(&ctx->wav);

    ctx->render_fp = NULL;
    ctx->log_fp = NULL;

    ctx->current_frame = 0U;
    ctx->total_frames = 0U;

    ctx->state = ENGINE_STATE_INIT;
    ctx->error = ENGINE_OK;
}

static void engine_cleanup(EngineContext * ctx){
    if(ctx == NULL){
        return;
    }

    if(ctx->render_fp != NULL){
        fclose(ctx->render_fp);
        ctx->render_fp = NULL;
    }

    if(ctx->log_fp != NULL){
        fclose(ctx->log_fp);
        ctx->log_fp = NULL;
    }

    wav_free(&ctx->wav);
    config_init(&ctx->config);
}

static int engine_log(EngineContext * ctx, const char * message){
    if(ctx == NULL || ctx->log_fp == NULL || message == NULL){
        return 0;
    }

    return fprintf(ctx->log_fp, "%s\n", message) >= 0;
}

static int engine_log_config(EngineContext * ctx){
    if(ctx == NULL || ctx->log_fp == NULL){
        return 0;
    }

    return fprintf(
        ctx->log_fp,
        "CONFIG frames_dir=%s frame_prefix=%s frame_extension=%s frame_digits=%u fps=%u start_frame=%u end_frame=%u palette=%s threshold=%u\n",
        ctx->config.frames_dir,
        ctx->config.frame_prefix,
        ctx->config.frame_extension,
        ctx->config.frame_digits,
        ctx->config.fps,
        ctx->config.start_frame,
        ctx->config.end_frame,
        ctx->config.palette,
        ctx->config.threshold
    ) >= 0;
}

static int engine_log_wav(EngineContext * ctx){
    if(ctx == NULL || ctx->log_fp == NULL){
        return 0;
    }

    return fprintf(
        ctx->log_fp,
        "WAV audio_format=%u channels=%u sample_rate=%u byte_rate=%u block_align=%u bits_per_sample=%u data_size=%u sample_count_per_channel=%u\n",
        ctx->wav.audio_format,
        ctx->wav.num_channels,
        ctx->wav.sample_rate,
        ctx->wav.byte_rate,
        ctx->wav.block_align,
        ctx->wav.bits_per_sample,
        ctx->wav.data_size,
        ctx->wav.sample_count_per_channel
    ) >= 0;
}

static int engine_log_summary(EngineContext * ctx){
    if(ctx == NULL || ctx->log_fp == NULL){
        return 0;
    }

    return fprintf(
        ctx->log_fp,
        "SUMMARY processed_frames=%u start_frame=%u end_frame=%u\n",
        ctx->total_frames,
        ctx->config.start_frame,
        ctx->config.end_frame
    ) >= 0;
}

static EngineError engine_open_outputs(
    EngineContext * ctx,
    const char * render_path,
    const char * log_path
){
    if(ctx == NULL || render_path == NULL || log_path == NULL){
        return ENGINE_ERR_NULL_ARG;
    }

    ctx->render_fp = fopen(render_path, "w");
    if(ctx->render_fp == NULL){
        return ENGINE_ERR_RENDER_OPEN;
    }

    ctx->log_fp = fopen(log_path, "w");
    if(ctx->log_fp == NULL){
        fclose(ctx->render_fp);
        ctx->render_fp = NULL;
        return ENGINE_ERR_LOG_OPEN;
    }

    return ENGINE_OK;
}

static EngineError engine_prepare_sequence(EngineContext * ctx){
    SequenceError seq_err;
    
    if(ctx == NULL){
        return ENGINE_ERR_NULL_ARG;
    }

    seq_err = sequence_frame_count(&ctx->config, &ctx->total_frames);
    if(seq_err != SEQUENCE_OK){
        return ENGINE_ERR_SEQUENCE;
    }

    ctx->current_frame = ctx->config.start_frame;

    return ENGINE_OK;
}

static EngineError engine_process_one_frame(EngineContext * ctx){
    char frame_path[512];
    SequenceError seq_err;

    BmpImage bmp;
    BmpError bmp_err;
    unsigned int relative_index;
    unsigned int start_sample;
    unsigned int end_sample;

    unsigned int avg_amp;
    int highlight;

    AsciiFrame ascii_frame;
    AsciiError ascii_err;

    RenderError render_err;

    double timestamp;

    if(ctx == NULL){
        return ENGINE_ERR_NULL_ARG;
    }

    seq_err = sequence_build_frame_path(
        &ctx->config,
        ctx->current_frame,
        frame_path,
        sizeof(frame_path)
    );

    if(seq_err != SEQUENCE_OK){
        return ENGINE_ERR_SEQUENCE;
    }

    seq_err = sequence_relative_index(
        &ctx->config,
        ctx->current_frame,
        &relative_index
    );
    if(seq_err != SEQUENCE_OK){
        return ENGINE_ERR_SEQUENCE;
    }

    bmp_init(&bmp);
    ascii_init(&ascii_frame);

    bmp_err = bmp_load(frame_path, &bmp);
    if(bmp_err != BMP_OK){
        bmp_free(&bmp);
        ascii_free(&ascii_frame);
        return ENGINE_ERR_BMP;
    }

    start_sample = (relative_index * ctx->wav.sample_rate) / ctx->config.fps;
    end_sample = ((relative_index + 1U) * ctx->wav.sample_rate) / ctx->config.fps;

    avg_amp = wav_average_amplitude(&ctx->wav, start_sample, end_sample);
    highlight = (avg_amp > ctx->config.threshold) ? 1 : 0;

    ascii_err = ascii_render_image_with_highlight(
        &bmp,
        ctx->config.palette,
        highlight,
        &ascii_frame
    );
    if(ascii_err != ASCII_OK){
        bmp_free(&bmp);
        ascii_free(&ascii_frame);
        return ENGINE_ERR_ASCII;
    }

    timestamp = (double) relative_index / (double)ctx->config.fps;

    render_err = render_write_frame(
        ctx->render_fp,
        ctx->current_frame,
        timestamp,
        highlight,
        &ascii_frame
    );
    if(render_err != RENDER_OK){
        bmp_free(&bmp);
        ascii_free(&ascii_frame);
        return ENGINE_ERR_RENDER_WRITE;
    }

    if(ctx->log_fp != NULL){
        if(fprintf(
            ctx->log_fp,
            "FRAME frame=%u path=%s relative_index=%u start_sample=%u end_sample=%u avg_amp=%u highlight=%d\n",
            ctx->current_frame,
            frame_path,
            relative_index,
            start_sample,
            end_sample,
            avg_amp,
            highlight
        ) < 0){
            bmp_free(&bmp);
            ascii_free(&ascii_frame);
            return ENGINE_ERR_LOG_WRITE;
        }
    }

    bmp_free(&bmp);
    ascii_free(&ascii_frame);
    return ENGINE_OK;
}
