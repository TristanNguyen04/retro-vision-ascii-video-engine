#include "parsers/wav.h"
#include "common/io_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define WAV_FMT_PCM 1
#define WAV_SUPPORTED_BITS 16
#define WAV_MIN_CHANNELS 1
#define WAV_MAX_CHANNELS 2

static int chunk_id_equals(const char id[4], const char * text);
static void wav_reset(WavFile * wav);

void wav_init(WavFile * wav){
    if(wav == NULL){
        return;
    }

    wav_reset(wav);
}

void wav_free(WavFile * wav){
    if(wav == NULL){
        return;
    }

    free(wav->samples);
    wav->samples = NULL;
    wav_reset(wav);
}

WavError wav_load(const char * filename, WavFile * wav){
    FILE * fp;
    char riff_id[4];
    unsigned int riff_size;
    char wave_id[4];
    int found_fmt;
    int found_data;
    unsigned int data_size;
    long data_offset;

    char chunk_id[4];
    unsigned int chunk_size;

    unsigned short audio_format;
    unsigned short num_channels;
    unsigned int sample_rate;
    unsigned int byte_rate;
    unsigned short block_align;
    unsigned short bits_per_sample;

    if(filename == NULL || wav == NULL){
        return WAV_ERR_NULL_ARG;
    }

    wav_free(wav);

    fp = fopen(filename, "rb");

    if(fp == NULL){
        return WAV_ERR_OPEN_FAILED;
    }

    if(
        !io_read_bytes(fp, riff_id, 4) ||
        !io_read_bytes(fp, &riff_size, sizeof(riff_size)) ||
        !io_read_bytes(fp, wave_id, 4)
    ){
        fclose(fp);
        return WAV_ERR_READ_FAILED;
    }

    if(!chunk_id_equals(riff_id, "RIFF")){
        fclose(fp);
        return WAV_ERR_BAD_RIFF;
    }

    if(!chunk_id_equals(wave_id, "WAVE")){
        fclose(fp);
        return WAV_ERR_BAD_WAVE;
    }

    found_fmt = 0;
    found_data = 0;
    data_size = 0U;
    data_offset = 0L;
    
    while(!found_data){
        if(!io_read_bytes(fp, chunk_id, 4)){
            break;
        }

        if(!io_read_bytes(fp, &chunk_size, sizeof(chunk_size))){
            fclose(fp);
            return WAV_ERR_READ_FAILED;
        }

        if(chunk_id_equals(chunk_id, "fmt ")){
            if(chunk_size < 16U){
                fclose(fp);
                return WAV_ERR_UNSUPPORTED_FORMAT;
            }

            if(
                !io_read_bytes(fp, &audio_format, sizeof(audio_format)) ||
                !io_read_bytes(fp, &num_channels, sizeof(num_channels)) ||
                !io_read_bytes(fp, &sample_rate, sizeof(sample_rate)) ||
                !io_read_bytes(fp, &byte_rate, sizeof(byte_rate)) ||
                !io_read_bytes(fp, &block_align, sizeof(block_align)) ||
                !io_read_bytes(fp, &bits_per_sample, sizeof(bits_per_sample))
            ){
                fclose(fp);
                return WAV_ERR_READ_FAILED;
            }

            if(chunk_size > 16U){
                if(!io_skip_bytes(fp, chunk_size - 16U)){
                    fclose(fp);
                    return WAV_ERR_SEEK_FAILED;
                }
            }

            if(bits_per_sample != WAV_SUPPORTED_BITS){
                fclose(fp);
                return WAV_ERR_UNSUPPORTED_BITS;
            }

            if(num_channels < WAV_MIN_CHANNELS || num_channels > WAV_MAX_CHANNELS){
                fclose(fp);
                return WAV_ERR_UNSUPPORTED_CHANNELS;
            }

            if(block_align != (unsigned short) (num_channels * (bits_per_sample / 8U))){
                fclose(fp);
                return WAV_ERR_INVALID_BLOCK_ALIGN;
            }

            wav->audio_format = audio_format;
            wav->num_channels = num_channels;
            wav->sample_rate = sample_rate;
            wav->byte_rate = byte_rate;
            wav->block_align = block_align;
            wav->bits_per_sample = bits_per_sample;

            found_fmt = 1;
        } else if(chunk_id_equals(chunk_id, "data")){
            data_offset = ftell(fp);

            if(data_offset < 0L){
                fclose(fp);
                return WAV_ERR_SEEK_FAILED;
            }

            data_size = chunk_size;

            if(!io_skip_bytes(fp, chunk_size)){
                fclose(fp);
                return WAV_ERR_SEEK_FAILED;
            }

            found_data = 1;
        } else {
            if(!io_skip_bytes(fp, chunk_size)){
                fclose(fp);
                return WAV_ERR_SEEK_FAILED;
            }
        }
    }

    if(!found_fmt){
        fclose(fp);
        return WAV_ERR_MISSING_FMT;
    }
    
    if(!found_data){
        fclose(fp);
        return WAV_ERR_MISSING_DATA;
    }

    if(fseek(fp, data_offset, SEEK_SET) != 0){
        fclose(fp);
        return WAV_ERR_SEEK_FAILED;
    }
    
    wav->data_size = data_size;
    wav->sample_count_per_channel = data_size / wav->block_align;
    
    if(data_size > 0U){
        size_t raw_sample_count;

        raw_sample_count = data_size / sizeof(short);

        wav->samples = (short *) malloc(raw_sample_count * sizeof(short));
        if(wav->samples == NULL){
            fclose(fp);
            wav_free(wav);
            return WAV_ERR_MEMORY;
        }

        if(fread(wav->samples, sizeof(short), raw_sample_count, fp) != raw_sample_count){
            fclose(fp);
            wav_free(wav);
            return WAV_ERR_READ_FAILED;
        }
    }

    fclose(fp);
    return WAV_OK;
}

const char * wav_error_string(WavError err){
    switch(err){
        case WAV_OK:
            return "success";
        case WAV_ERR_NULL_ARG:
            return "null argument";
        case WAV_ERR_OPEN_FAILED:
            return "failed to open WAV file";
        case WAV_ERR_READ_FAILED:
            return "failed to read WAV file";
        case WAV_ERR_SEEK_FAILED:
            return "failed to seek in WAV file";
        case WAV_ERR_BAD_RIFF:
            return "invalid RIFF header";
        case WAV_ERR_BAD_WAVE:
            return "invalid WAVE header";
        case WAV_ERR_MISSING_FMT:
            return "missing fmt chunk";
        case WAV_ERR_MISSING_DATA:
            return "missing data chunk";
        case WAV_ERR_UNSUPPORTED_FORMAT:
            return "unsupported WAV format";
        case WAV_ERR_UNSUPPORTED_BITS:
            return "unsupported bits per sample";
        case WAV_ERR_UNSUPPORTED_CHANNELS:
            return "unsupported channel count";
        case WAV_ERR_INVALID_BLOCK_ALIGN:
            return "invalid block alignment";
        case WAV_ERR_MEMORY:
            return "memory allocation failed";
        default:
            break;
    }

    return "unknown WAV error";
}

unsigned int wav_average_amplitude(const WavFile * wav, unsigned int start_sample, unsigned int end_sample){
    unsigned long sum = 0;
    unsigned int count;

    short s, left, right;
    unsigned int amp_left, amp_right;

    int i;

    if(wav == NULL || wav->samples == NULL){
        return 0U;
    }

    if(start_sample >= end_sample){
        return 0U;
    }

    if(start_sample >= wav->sample_count_per_channel){
        return 0U;
    }

    if(end_sample > wav->sample_count_per_channel){
        end_sample = wav->sample_count_per_channel;
    }

    count = end_sample - start_sample;

    if(wav->num_channels == 1U){
        for(i = start_sample; i < end_sample; i++){
            s = wav->samples[i];

            sum += s < 0 ? (unsigned long) (-s) : (unsigned long) s;
        }
    } else {
        for(i = start_sample; i < end_sample; i++){
            left = wav->samples[i * 2U];
            right = wav->samples[i * 2U + 1U];

            amp_left = (left < 0) ? (unsigned int) (-left) : (unsigned int) left;
            amp_right = (right < 0) ? (unsigned int) (-right) : (unsigned int) right;

            sum += (unsigned long) ((amp_left + amp_right) / 2U);
        }
    }

    if(count == 0U){
        return 0U;
    }

    return (unsigned int) (sum / count);
}

static int chunk_id_equals(const char id[4], const char * text){
    return memcmp(id, text, 4) == 0;
}

static void wav_reset(WavFile * wav){
    wav->audio_format = 0U;
    wav->num_channels = 0U;
    wav->sample_rate = 0U;
    wav->byte_rate = 0U;
    wav->block_align = 0U;
    wav->bits_per_sample = 0U;
    wav->data_size = 0U;
    wav->sample_count_per_channel = 0U;
    wav->samples = NULL;
}

