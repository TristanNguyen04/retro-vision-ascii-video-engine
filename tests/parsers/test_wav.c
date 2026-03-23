#include "parsers/wav.h"
#include "tests_helper.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int write_pcm_wav_file(
    const char *filename,
    unsigned int num_channels,
    unsigned int sample_rate,
    unsigned int bits_per_sample,
    const short *samples,
    unsigned int raw_sample_count
) {
    FILE *fp;
    unsigned long data_size;
    unsigned long riff_size;
    unsigned int block_align;
    unsigned long byte_rate;
    unsigned int i;

    if (filename == NULL || samples == NULL) {
        return 0;
    }

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        return 0;
    }

    block_align = num_channels * (bits_per_sample / 8U);
    byte_rate = (unsigned long)sample_rate * (unsigned long)block_align;
    data_size = (unsigned long)raw_sample_count * sizeof(short);
    riff_size = 4UL + (8UL + 16UL) + (8UL + data_size);

    if (!io_write_bytes(fp, "RIFF", 4) ||
        !io_write_u32_le(fp, riff_size) ||
        !io_write_bytes(fp, "WAVE", 4) ||
        !io_write_bytes(fp, "fmt ", 4) ||
        !io_write_u32_le(fp, 16UL) ||
        !io_write_u16_le(fp, 1U) ||
        !io_write_u16_le(fp, num_channels) ||
        !io_write_u32_le(fp, sample_rate) ||
        !io_write_u32_le(fp, byte_rate) ||
        !io_write_u16_le(fp, block_align) ||
        !io_write_u16_le(fp, bits_per_sample) ||
        !io_write_bytes(fp, "data", 4) ||
        !io_write_u32_le(fp, data_size)) {
        fclose(fp);
        return 0;
    }

    for (i = 0; i < raw_sample_count; i++) {
        if (!io_write_u16_le(fp, (unsigned int)((unsigned short)samples[i]))) {
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return 1;
}

static int write_bad_riff_file(const char *filename) {
    FILE *fp;

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        return 0;
    }

    if (!io_write_bytes(fp, "RUFF", 4) ||
        !io_write_u32_le(fp, 0UL) ||
        !io_write_bytes(fp, "WAVE", 4)) {
        fclose(fp);
        return 0;
    }

    fclose(fp);
    return 1;
}

static int write_missing_fmt_file(const char *filename) {
    FILE *fp;

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        return 0;
    }

    if (!io_write_bytes(fp, "RIFF", 4) ||
        !io_write_u32_le(fp, 12UL) ||
        !io_write_bytes(fp, "WAVE", 4) ||
        !io_write_bytes(fp, "data", 4) ||
        !io_write_u32_le(fp, 4UL) ||
        !io_write_u16_le(fp, 1U) ||
        !io_write_u16_le(fp, 2U)) {
        fclose(fp);
        return 0;
    }

    fclose(fp);
    return 1;
}

static int write_8bit_wav_file(const char *filename) {
    FILE *fp;
    unsigned char samples[4];
    unsigned long data_size;
    unsigned long riff_size;

    samples[0] = 0U;
    samples[1] = 64U;
    samples[2] = 128U;
    samples[3] = 255U;

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        return 0;
    }

    data_size = 4UL;
    riff_size = 4UL + (8UL + 16UL) + (8UL + data_size);

    if (!io_write_bytes(fp, "RIFF", 4) ||
        !io_write_u32_le(fp, riff_size) ||
        !io_write_bytes(fp, "WAVE", 4) ||
        !io_write_bytes(fp, "fmt ", 4) ||
        !io_write_u32_le(fp, 16UL) ||
        !io_write_u16_le(fp, 1U) ||
        !io_write_u16_le(fp, 1U) ||
        !io_write_u32_le(fp, 8000UL) ||
        !io_write_u32_le(fp, 8000UL) ||
        !io_write_u16_le(fp, 1U) ||
        !io_write_u16_le(fp, 8U) ||
        !io_write_bytes(fp, "data", 4) ||
        !io_write_u32_le(fp, data_size) ||
        !io_write_bytes(fp, samples, 4)) {
        fclose(fp);
        return 0;
    }

    fclose(fp);
    return 1;
}

static int test_wav_init_and_free(void) {
    WavFile wav;

    wav.audio_format = 99U;
    wav.num_channels = 99U;
    wav.sample_rate = 99U;
    wav.byte_rate = 99U;
    wav.block_align = 99U;
    wav.bits_per_sample = 99U;
    wav.data_size = 99U;
    wav.sample_count_per_channel = 99U;
    wav.samples = (short *)1;

    wav_init(&wav);

    if (wav.audio_format != 0U || wav.num_channels != 0U ||
        wav.sample_rate != 0U || wav.byte_rate != 0U ||
        wav.block_align != 0U || wav.bits_per_sample != 0U ||
        wav.data_size != 0U || wav.sample_count_per_channel != 0U ||
        wav.samples != NULL) {
        return 0;
    }

    wav_free(&wav);

    return 1;
}

static int test_wav_load_null_args(void) {
    WavFile wav;

    wav_init(&wav);

    if (wav_load(NULL, &wav) != WAV_ERR_NULL_ARG) {
        return 0;
    }

    if (wav_load("dummy.wav", NULL) != WAV_ERR_NULL_ARG) {
        return 0;
    }

    wav_free(&wav);
    return 1;
}

static int test_wav_load_open_failed(void) {
    WavFile wav;

    wav_init(&wav);

    if (wav_load("definitely_missing_file_12345.wav", &wav) != WAV_ERR_OPEN_FAILED) {
        wav_free(&wav);
        return 0;
    }

    wav_free(&wav);
    return 1;
}

static int test_wav_load_valid_mono(void) {
    char filename[L_tmpnam];
    short samples[4];
    WavFile wav;
    int ok;

    samples[0] = 1000;
    samples[1] = -1000;
    samples[2] = 2000;
    samples[3] = -2000;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!write_pcm_wav_file(filename, 1U, 8000U, 16U, samples, 4U)) {
        return 0;
    }

    wav_init(&wav);
    ok = (wav_load(filename, &wav) == WAV_OK);

    if (ok) {
        ok = ok &&
            wav.audio_format == 1U &&
            wav.num_channels == 1U &&
            wav.sample_rate == 8000U &&
            wav.bits_per_sample == 16U &&
            wav.block_align == 2U &&
            wav.data_size == 8U &&
            wav.sample_count_per_channel == 4U &&
            wav.samples != NULL &&
            wav.samples[0] == 1000 &&
            wav.samples[1] == -1000 &&
            wav.samples[2] == 2000 &&
            wav.samples[3] == -2000;
    }

    wav_free(&wav);
    remove(filename);
    return ok;
}

static int test_wav_average_amplitude_mono(void) {
    char filename[L_tmpnam];
    short samples[4];
    WavFile wav;
    unsigned int avg;
    int ok;

    samples[0] = 1000;
    samples[1] = -1000;
    samples[2] = 3000;
    samples[3] = -3000;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!write_pcm_wav_file(filename, 1U, 8000U, 16U, samples, 4U)) {
        return 0;
    }

    wav_init(&wav);
    ok = (wav_load(filename, &wav) == WAV_OK);
    if (!ok) {
        remove(filename);
        return 0;
    }

    avg = wav_average_amplitude(&wav, 0U, 4U);

    wav_free(&wav);
    remove(filename);

    return avg == 2000U;
}

static int test_wav_load_valid_stereo_and_average(void) {
    char filename[L_tmpnam];
    short samples[4];
    WavFile wav;
    unsigned int avg;
    int ok;

    samples[0] = 1000;
    samples[1] = -1000;
    samples[2] = 3000;
    samples[3] = -3000;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!write_pcm_wav_file(filename, 2U, 8000U, 16U, samples, 4U)) {
        return 0;
    }

    wav_init(&wav);
    ok = (wav_load(filename, &wav) == WAV_OK);

    if (ok) {
        avg = wav_average_amplitude(&wav, 0U, 2U);
        ok = ok &&
            wav.num_channels == 2U &&
            wav.block_align == 4U &&
            wav.sample_count_per_channel == 2U &&
            avg == 2000U;
    }

    wav_free(&wav);
    remove(filename);
    return ok;
}

static int test_wav_load_bad_riff(void) {
    char filename[L_tmpnam];
    WavFile wav;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!write_bad_riff_file(filename)) {
        return 0;
    }

    wav_init(&wav);
    ok = (wav_load(filename, &wav) == WAV_ERR_BAD_RIFF);

    wav_free(&wav);
    remove(filename);
    return ok;
}

static int test_wav_load_missing_fmt(void) {
    char filename[L_tmpnam];
    WavFile wav;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!write_missing_fmt_file(filename)) {
        return 0;
    }

    wav_init(&wav);
    ok = (wav_load(filename, &wav) == WAV_ERR_MISSING_FMT);

    wav_free(&wav);
    remove(filename);
    return ok;
}

static int test_wav_load_unsupported_bits(void) {
    char filename[L_tmpnam];
    WavFile wav;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!write_8bit_wav_file(filename)) {
        return 0;
    }

    wav_init(&wav);
    ok = (wav_load(filename, &wav) == WAV_ERR_UNSUPPORTED_BITS);

    wav_free(&wav);
    remove(filename);
    return ok;
}

static int test_wav_average_amplitude_invalid_ranges(void) {
    WavFile wav;
    short samples[4];

    wav_init(&wav);
    wav.num_channels = 1U;
    wav.sample_count_per_channel = 4U;
    wav.samples = samples;

    samples[0] = 100;
    samples[1] = 200;
    samples[2] = 300;
    samples[3] = 400;

    if (wav_average_amplitude(&wav, 2U, 2U) != 0U) {
        return 0;
    }

    if (wav_average_amplitude(&wav, 5U, 6U) != 0U) {
        return 0;
    }

    if (wav_average_amplitude(&wav, 3U, 10U) != 400U) {
        return 0;
    }

    return 1;
}

int main(void) {
    test_report("wav_init and wav_free", test_wav_init_and_free());
    test_report("wav_load null args", test_wav_load_null_args());
    test_report("wav_load open failed", test_wav_load_open_failed());
    test_report("wav_load valid mono", test_wav_load_valid_mono());
    test_report("wav_average_amplitude mono", test_wav_average_amplitude_mono());
    test_report("wav_load valid stereo and average", test_wav_load_valid_stereo_and_average());
    test_report("wav_load bad RIFF", test_wav_load_bad_riff());
    test_report("wav_load missing fmt", test_wav_load_missing_fmt());
    test_report("wav_load unsupported bits", test_wav_load_unsupported_bits());
    test_report("wav_average_amplitude invalid ranges", test_wav_average_amplitude_invalid_ranges());

    test_summary();
    return test_failed_count() == 0 ? 0 : 1;
}

