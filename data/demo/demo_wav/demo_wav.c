#include "parsers/wav.h"

#include <stdio.h>

int main(int argc, char **argv) {
    unsigned int i;

    unsigned int start = 4410000U; /* at t = 100 seconds */
    unsigned int end = start + 10U; /* around t = 100 seconds */

    WavError err;
    WavFile wav;

    if (argc != 2) {
        printf("Usage: %s <wav_file>\n", argv[0]);
        return 1;
    }

    wav_init(&wav);

    err = wav_load(argv[1], &wav);

    printf("error: %s\n", wav_error_string(err));

    if (err == WAV_OK) {
        printf("audio_format: %u\n", wav.audio_format);
        printf("num_channels: %u\n", wav.num_channels);
        printf("sample_rate: %u\n", wav.sample_rate);
        printf("byte_rate: %u\n", wav.byte_rate);
        printf("block_align: %u\n", wav.block_align);
        printf("bits_per_sample: %u\n", wav.bits_per_sample);
        printf("data_size: %u\n", wav.data_size);
        printf("sample_count_per_channel: %u\n", wav.sample_count_per_channel);
        printf("avg amplitude [0, 1470): %u\n", wav_average_amplitude(&wav, 0U, 1470U));
        printf("avg amplitude [1470, 2940): %u\n", wav_average_amplitude(&wav, 1470U, 2940U));
        printf("avg amplitude [44100, 45570): %u\n", wav_average_amplitude(&wav, 44100U, 45570U));
        printf("avg amplitude [88200, 89670): %u\n", wav_average_amplitude(&wav, 88200U, 89670U));

        printf("\n===Sample data at t = %d seconds===\n", start / 44100);
        if(wav.num_channels == 1){
            for(i = start; i < end && i < wav.sample_count_per_channel; i++){
                printf("sample %u: %d\n", i, wav.samples[i]); 
            }
        } else {
            for(i = start; i < end && i < wav.sample_count_per_channel; i++){
                printf("frame %u: L=%d, R=%d\n", i, wav.samples[i * 2U], wav.samples[i * 2U + 1U]);
            }
        }
    }

    wav_free(&wav);
    return (err == WAV_OK) ? 0 : 1;
}