#include "components/sequence.h"
#include "parsers/json/config.h"
#include "../tests_helper.h"

#include <string.h>

static void fill_valid_config(EngineConfig *config);

static int test_sequence_validate_valid_config(void);
static int test_sequence_validate_null_arg(void);
static int test_sequence_validate_empty_frames_dir(void);
static int test_sequence_validate_empty_prefix(void);
static int test_sequence_validate_empty_extension(void);
static int test_sequence_validate_invalid_digits(void);
static int test_sequence_validate_invalid_start_frame(void);
static int test_sequence_validate_invalid_range(void);

static int test_sequence_frame_count_valid(void);
static int test_sequence_frame_count_single_frame(void);
static int test_sequence_frame_count_null_arg(void);
static int test_sequence_frame_count_invalid_config(void);

static int test_sequence_relative_index_valid_first(void);
static int test_sequence_relative_index_valid_middle(void);
static int test_sequence_relative_index_valid_last(void);
static int test_sequence_relative_index_null_arg(void);
static int test_sequence_relative_index_out_of_range_low(void);
static int test_sequence_relative_index_out_of_range_high(void);

static int test_sequence_build_frame_path_valid_first(void);
static int test_sequence_build_frame_path_valid_middle(void);
static int test_sequence_build_frame_path_valid_last(void);
static int test_sequence_build_frame_path_null_arg(void);
static int test_sequence_build_frame_path_out_of_range(void);
static int test_sequence_build_frame_path_small_buffer(void);

static int test_sequence_error_string(void);

static void fill_valid_config(EngineConfig *config) {
    config_init(config);

    strcpy(config->frames_dir, "frames");
    strcpy(config->frame_prefix, "frame_");
    strcpy(config->frame_extension, ".bmp");
    config->frame_digits = 4U;

    config->fps = 30U;
    config->start_frame = 1U;
    config->end_frame = 120U;

    strcpy(config->palette, "@#*+=-:.");
    config->threshold = 5000U;
}

static int test_sequence_validate_valid_config(void) {
    EngineConfig config;

    fill_valid_config(&config);

    return sequence_validate(&config) == SEQUENCE_OK;
}

static int test_sequence_validate_null_arg(void) {
    return sequence_validate(NULL) == SEQUENCE_ERR_NULL_ARG;
}

static int test_sequence_validate_empty_frames_dir(void) {
    EngineConfig config;

    fill_valid_config(&config);
    config.frames_dir[0] = '\0';

    return sequence_validate(&config) == SEQUENCE_ERR_INVALID_CONFIG;
}

static int test_sequence_validate_empty_prefix(void) {
    EngineConfig config;

    fill_valid_config(&config);
    config.frame_prefix[0] = '\0';

    return sequence_validate(&config) == SEQUENCE_ERR_INVALID_CONFIG;
}

static int test_sequence_validate_empty_extension(void) {
    EngineConfig config;

    fill_valid_config(&config);
    config.frame_extension[0] = '\0';

    return sequence_validate(&config) == SEQUENCE_ERR_INVALID_CONFIG;
}

static int test_sequence_validate_invalid_digits(void) {
    EngineConfig config;

    fill_valid_config(&config);
    config.frame_digits = 5U;

    return sequence_validate(&config) == SEQUENCE_ERR_INVALID_CONFIG;
}

static int test_sequence_validate_invalid_start_frame(void) {
    EngineConfig config;

    fill_valid_config(&config);
    config.start_frame = 0U;

    return sequence_validate(&config) == SEQUENCE_ERR_INVALID_RANGE;
}

static int test_sequence_validate_invalid_range(void) {
    EngineConfig config;

    fill_valid_config(&config);
    config.start_frame = 20U;
    config.end_frame = 10U;

    return sequence_validate(&config) == SEQUENCE_ERR_INVALID_RANGE;
}

static int test_sequence_frame_count_valid(void) {
    EngineConfig config;
    unsigned int count;

    fill_valid_config(&config);
    count = 0U;

    return sequence_frame_count(&config, &count) == SEQUENCE_OK &&
           count == 120U;
}

static int test_sequence_frame_count_single_frame(void) {
    EngineConfig config;
    unsigned int count;

    fill_valid_config(&config);
    config.start_frame = 7U;
    config.end_frame = 7U;
    count = 0U;

    return sequence_frame_count(&config, &count) == SEQUENCE_OK &&
           count == 1U;
}

static int test_sequence_frame_count_null_arg(void) {
    EngineConfig config;
    unsigned int count;

    fill_valid_config(&config);
    count = 0U;

    return sequence_frame_count(NULL, &count) == SEQUENCE_ERR_NULL_ARG &&
           sequence_frame_count(&config, NULL) == SEQUENCE_ERR_NULL_ARG;
}

static int test_sequence_frame_count_invalid_config(void) {
    EngineConfig config;
    unsigned int count;

    fill_valid_config(&config);
    config.frame_digits = 5U;
    count = 0U;

    return sequence_frame_count(&config, &count) == SEQUENCE_ERR_INVALID_CONFIG;
}

static int test_sequence_relative_index_valid_first(void) {
    EngineConfig config;
    unsigned int index;

    fill_valid_config(&config);
    index = 999U;

    return sequence_relative_index(&config, 1U, &index) == SEQUENCE_OK &&
           index == 0U;
}

static int test_sequence_relative_index_valid_middle(void) {
    EngineConfig config;
    unsigned int index;

    fill_valid_config(&config);
    index = 999U;

    return sequence_relative_index(&config, 42U, &index) == SEQUENCE_OK &&
           index == 41U;
}

static int test_sequence_relative_index_valid_last(void) {
    EngineConfig config;
    unsigned int index;

    fill_valid_config(&config);
    index = 999U;

    return sequence_relative_index(&config, 120U, &index) == SEQUENCE_OK &&
           index == 119U;
}

static int test_sequence_relative_index_null_arg(void) {
    EngineConfig config;
    unsigned int index;

    fill_valid_config(&config);
    index = 0U;

    return sequence_relative_index(NULL, 1U, &index) == SEQUENCE_ERR_NULL_ARG &&
           sequence_relative_index(&config, 1U, NULL) == SEQUENCE_ERR_NULL_ARG;
}

static int test_sequence_relative_index_out_of_range_low(void) {
    EngineConfig config;
    unsigned int index;

    fill_valid_config(&config);
    index = 999U;

    return sequence_relative_index(&config, 0U, &index) ==
               SEQUENCE_ERR_INVALID_FRAME_NUMBER &&
           index == 999U;
}

static int test_sequence_relative_index_out_of_range_high(void) {
    EngineConfig config;
    unsigned int index;

    fill_valid_config(&config);
    index = 999U;

    return sequence_relative_index(&config, 121U, &index) ==
               SEQUENCE_ERR_INVALID_FRAME_NUMBER &&
           index == 999U;
}

static int test_sequence_build_frame_path_valid_first(void) {
    EngineConfig config;
    char path[256];

    fill_valid_config(&config);
    path[0] = '\0';

    return sequence_build_frame_path(&config, 1U, path, sizeof(path)) ==
               SEQUENCE_OK &&
           strcmp(path, "frames/frame_0001.bmp") == 0;
}

static int test_sequence_build_frame_path_valid_middle(void) {
    EngineConfig config;
    char path[256];

    fill_valid_config(&config);
    path[0] = '\0';

    return sequence_build_frame_path(&config, 42U, path, sizeof(path)) ==
               SEQUENCE_OK &&
           strcmp(path, "frames/frame_0042.bmp") == 0;
}

static int test_sequence_build_frame_path_valid_last(void) {
    EngineConfig config;
    char path[256];

    fill_valid_config(&config);
    path[0] = '\0';

    return sequence_build_frame_path(&config, 120U, path, sizeof(path)) ==
               SEQUENCE_OK &&
           strcmp(path, "frames/frame_0120.bmp") == 0;
}

static int test_sequence_build_frame_path_null_arg(void) {
    EngineConfig config;
    char path[256];

    fill_valid_config(&config);

    return sequence_build_frame_path(NULL, 1U, path, sizeof(path)) ==
               SEQUENCE_ERR_NULL_ARG &&
           sequence_build_frame_path(&config, 1U, NULL, sizeof(path)) ==
               SEQUENCE_ERR_NULL_ARG &&
           sequence_build_frame_path(&config, 1U, path, 0U) ==
               SEQUENCE_ERR_NULL_ARG;
}

static int test_sequence_build_frame_path_out_of_range(void) {
    EngineConfig config;
    char path[256];

    fill_valid_config(&config);
    strcpy(path, "unchanged");

    return sequence_build_frame_path(&config, 121U, path, sizeof(path)) ==
               SEQUENCE_ERR_INVALID_FRAME_NUMBER &&
           strcmp(path, "unchanged") == 0;
}

static int test_sequence_build_frame_path_small_buffer(void) {
    EngineConfig config;
    char path[8];

    fill_valid_config(&config);
    strcpy(path, "abc");

    return sequence_build_frame_path(&config, 1U, path, sizeof(path)) ==
               SEQUENCE_ERR_PATH_TOO_LONG &&
           path[0] == '\0';
}

static int test_sequence_error_string(void) {
    return strcmp(sequence_error_string(SEQUENCE_OK), "[SEQUENCE] success") == 0 &&
           strcmp(sequence_error_string(SEQUENCE_ERR_INVALID_CONFIG),
                  "[SEQUENCE] invalid sequence config") == 0 &&
           strcmp(sequence_error_string(SEQUENCE_ERR_PATH_TOO_LONG),
                  "[SEQUENCE] sequence path too long") == 0;
}

int main(void) {
    test_report("sequence_validate valid config",
                test_sequence_validate_valid_config());
    test_report("sequence_validate null arg",
                test_sequence_validate_null_arg());
    test_report("sequence_validate empty frames_dir",
                test_sequence_validate_empty_frames_dir());
    test_report("sequence_validate empty prefix",
                test_sequence_validate_empty_prefix());
    test_report("sequence_validate empty extension",
                test_sequence_validate_empty_extension());
    test_report("sequence_validate invalid digits",
                test_sequence_validate_invalid_digits());
    test_report("sequence_validate invalid start_frame",
                test_sequence_validate_invalid_start_frame());
    test_report("sequence_validate invalid range",
                test_sequence_validate_invalid_range());

    test_report("sequence_frame_count valid",
                test_sequence_frame_count_valid());
    test_report("sequence_frame_count single frame",
                test_sequence_frame_count_single_frame());
    test_report("sequence_frame_count null arg",
                test_sequence_frame_count_null_arg());
    test_report("sequence_frame_count invalid config",
                test_sequence_frame_count_invalid_config());

    test_report("sequence_relative_index valid first",
                test_sequence_relative_index_valid_first());
    test_report("sequence_relative_index valid middle",
                test_sequence_relative_index_valid_middle());
    test_report("sequence_relative_index valid last",
                test_sequence_relative_index_valid_last());
    test_report("sequence_relative_index null arg",
                test_sequence_relative_index_null_arg());
    test_report("sequence_relative_index out of range low",
                test_sequence_relative_index_out_of_range_low());
    test_report("sequence_relative_index out of range high",
                test_sequence_relative_index_out_of_range_high());

    test_report("sequence_build_frame_path valid first",
                test_sequence_build_frame_path_valid_first());
    test_report("sequence_build_frame_path valid middle",
                test_sequence_build_frame_path_valid_middle());
    test_report("sequence_build_frame_path valid last",
                test_sequence_build_frame_path_valid_last());
    test_report("sequence_build_frame_path null arg",
                test_sequence_build_frame_path_null_arg());
    test_report("sequence_build_frame_path out of range",
                test_sequence_build_frame_path_out_of_range());
    test_report("sequence_build_frame_path small buffer",
                test_sequence_build_frame_path_small_buffer());

    test_report("sequence_error_string",
                test_sequence_error_string());

    test_summary();
    return test_failed_count() == 0 ? 0 : 1;
}
