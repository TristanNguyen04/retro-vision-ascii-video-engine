#include "parsers/json/config.h"
#include "common/io_utils.h"
#include "../../tests_helper.h"

#include <stdio.h>
#include <string.h>

static int test_config_init(void);
static int test_config_load_null_args(void);
static int test_config_load_open_failed(void);
static int test_config_load_minimal_valid(void);
static int test_config_load_full_valid(void);
static int test_config_load_missing_frames_dir(void);
static int test_config_load_missing_fps(void);
static int test_config_load_missing_start_frame(void);
static int test_config_load_missing_end_frame(void);
static int test_config_load_missing_palette(void);
static int test_config_load_missing_threshold(void);
static int test_config_load_unknown_key(void);
static int test_config_load_wrong_type_string_for_fps(void);
static int test_config_load_wrong_type_int_for_frames_dir(void);
static int test_config_load_negative_uint_field(void);
static int test_config_load_invalid_fps_zero(void);
static int test_config_load_invalid_start_frame_zero(void);
static int test_config_load_invalid_end_before_start(void);
static int test_config_load_invalid_extension(void);
static int test_config_load_invalid_frame_digits(void);
static int test_config_load_empty_frames_dir(void);
static int test_config_load_empty_palette(void);
static int test_config_error_string(void);

static int test_config_init(void) {
    EngineConfig config;

    strcpy(config.frames_dir, "abc");
    strcpy(config.frame_prefix, "abc");
    strcpy(config.frame_extension, ".xyz");
    config.frame_digits = 99U;
    config.fps = 60U;
    config.start_frame = 10U;
    config.end_frame = 20U;
    strcpy(config.palette, "@@@");
    config.threshold = 123U;

    config_init(&config);

    return config.frames_dir[0] == '\0' &&
           config.frame_prefix[0] == '\0' &&
           config.frame_extension[0] == '\0' &&
           config.frame_digits == 0U &&
           config.fps == 0U &&
           config.start_frame == 0U &&
           config.end_frame == 0U &&
           config.palette[0] == '\0' &&
           config.threshold == 0U;
}

static int test_config_load_null_args(void) {
    EngineConfig config;

    config_init(&config);

    if (config_load(NULL, &config) != CONFIG_ERR_NULL_ARG) {
        return 0;
    }

    if (config_load("dummy.json", NULL) != CONFIG_ERR_NULL_ARG) {
        return 0;
    }

    return 1;
}

static int test_config_load_open_failed(void) {
    EngineConfig config;

    config_init(&config);

    return config_load("definitely_missing_file_12345.json", &config) ==
           CONFIG_ERR_JSON_LOAD;
}

static int test_config_load_minimal_valid(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"frames\","
            "\"fps\":30,"
            "\"start_frame\":1,"
            "\"end_frame\":120,"
            "\"palette\":\"@#*+=-:.\","
            "\"threshold\":5000"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_OK);

    if (ok) {
        ok = ok &&
            strcmp(config.frames_dir, "frames") == 0 &&
            strcmp(config.frame_prefix, "frame_") == 0 &&
            strcmp(config.frame_extension, ".bmp") == 0 &&
            config.frame_digits == 4U &&
            config.fps == 30U &&
            config.start_frame == 1U &&
            config.end_frame == 120U &&
            strcmp(config.palette, "@#*+=-:.") == 0 &&
            config.threshold == 5000U;
    }

    remove(filename);
    return ok;
}

static int test_config_load_full_valid(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"video_frames\","
            "\"frame_prefix\":\"frame_\","
            "\"frame_extension\":\".bmp\","
            "\"frame_digits\":4,"
            "\"fps\":24,"
            "\"start_frame\":10,"
            "\"end_frame\":42,"
            "\"palette\":\" .:-=+*#%@\","
            "\"threshold\":7000"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_OK);

    if (ok) {
        ok = ok &&
            strcmp(config.frames_dir, "video_frames") == 0 &&
            strcmp(config.frame_prefix, "frame_") == 0 &&
            strcmp(config.frame_extension, ".bmp") == 0 &&
            config.frame_digits == 4U &&
            config.fps == 24U &&
            config.start_frame == 10U &&
            config.end_frame == 42U &&
            strcmp(config.palette, " .:-=+*#%@") == 0 &&
            config.threshold == 7000U;
    }

    remove(filename);
    return ok;
}

static int test_config_load_missing_frames_dir(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"fps\":30,"
            "\"start_frame\":1,"
            "\"end_frame\":10,"
            "\"palette\":\"@#\","
            "\"threshold\":1"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_ERR_MISSING_KEY);

    remove(filename);
    return ok;
}

static int test_config_load_missing_fps(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"frames\","
            "\"start_frame\":1,"
            "\"end_frame\":10,"
            "\"palette\":\"@#\","
            "\"threshold\":1"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_ERR_MISSING_KEY);

    remove(filename);
    return ok;
}

static int test_config_load_missing_start_frame(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"frames\","
            "\"fps\":30,"
            "\"end_frame\":10,"
            "\"palette\":\"@#\","
            "\"threshold\":1"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_ERR_MISSING_KEY);

    remove(filename);
    return ok;
}

static int test_config_load_missing_end_frame(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"frames\","
            "\"fps\":30,"
            "\"start_frame\":1,"
            "\"palette\":\"@#\","
            "\"threshold\":1"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_ERR_MISSING_KEY);

    remove(filename);
    return ok;
}

static int test_config_load_missing_palette(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"frames\","
            "\"fps\":30,"
            "\"start_frame\":1,"
            "\"end_frame\":10,"
            "\"threshold\":1"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_ERR_MISSING_KEY);

    remove(filename);
    return ok;
}

static int test_config_load_missing_threshold(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"frames\","
            "\"fps\":30,"
            "\"start_frame\":1,"
            "\"end_frame\":10,"
            "\"palette\":\"@#\""
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_ERR_MISSING_KEY);

    remove(filename);
    return ok;
}

static int test_config_load_unknown_key(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"frames\","
            "\"fps\":30,"
            "\"start_frame\":1,"
            "\"end_frame\":10,"
            "\"palette\":\"@#\","
            "\"threshold\":1,"
            "\"unknown_key\":123"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_ERR_UNKNOWN_KEY);

    remove(filename);
    return ok;
}

static int test_config_load_wrong_type_string_for_fps(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"frames\","
            "\"fps\":\"30\","
            "\"start_frame\":1,"
            "\"end_frame\":10,"
            "\"palette\":\"@#\","
            "\"threshold\":1"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_ERR_WRONG_TYPE);

    remove(filename);
    return ok;
}

static int test_config_load_wrong_type_int_for_frames_dir(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":123,"
            "\"fps\":30,"
            "\"start_frame\":1,"
            "\"end_frame\":10,"
            "\"palette\":\"@#\","
            "\"threshold\":1"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_ERR_WRONG_TYPE);

    remove(filename);
    return ok;
}

static int test_config_load_negative_uint_field(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"frames\","
            "\"fps\":30,"
            "\"start_frame\":-1,"
            "\"end_frame\":10,"
            "\"palette\":\"@#\","
            "\"threshold\":1"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_ERR_INVALID_VALUE);

    remove(filename);
    return ok;
}

static int test_config_load_invalid_fps_zero(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"frames\","
            "\"fps\":0,"
            "\"start_frame\":1,"
            "\"end_frame\":10,"
            "\"palette\":\"@#\","
            "\"threshold\":1"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_ERR_INVALID_VALUE);

    remove(filename);
    return ok;
}

static int test_config_load_invalid_start_frame_zero(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"frames\","
            "\"fps\":30,"
            "\"start_frame\":0,"
            "\"end_frame\":10,"
            "\"palette\":\"@#\","
            "\"threshold\":1"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_ERR_INVALID_VALUE);

    remove(filename);
    return ok;
}

static int test_config_load_invalid_end_before_start(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"frames\","
            "\"fps\":30,"
            "\"start_frame\":20,"
            "\"end_frame\":10,"
            "\"palette\":\"@#\","
            "\"threshold\":1"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_ERR_INVALID_RANGE);

    remove(filename);
    return ok;
}

static int test_config_load_invalid_extension(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"frames\","
            "\"frame_extension\":\".png\","
            "\"fps\":30,"
            "\"start_frame\":1,"
            "\"end_frame\":10,"
            "\"palette\":\"@#\","
            "\"threshold\":1"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_ERR_INVALID_VALUE);

    remove(filename);
    return ok;
}

static int test_config_load_invalid_frame_digits(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"frames\","
            "\"frame_digits\":5,"
            "\"fps\":30,"
            "\"start_frame\":1,"
            "\"end_frame\":10,"
            "\"palette\":\"@#\","
            "\"threshold\":1"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_ERR_INVALID_VALUE);

    remove(filename);
    return ok;
}

static int test_config_load_empty_frames_dir(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"\","
            "\"fps\":30,"
            "\"start_frame\":1,"
            "\"end_frame\":10,"
            "\"palette\":\"@#\","
            "\"threshold\":1"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_ERR_INVALID_VALUE);

    remove(filename);
    return ok;
}

static int test_config_load_empty_palette(void) {
    char filename[128];
    EngineConfig config;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"frames\","
            "\"fps\":30,"
            "\"start_frame\":1,"
            "\"end_frame\":10,"
            "\"palette\":\"\","
            "\"threshold\":1"
            "}"
        )) {
        return 0;
    }

    config_init(&config);
    ok = (config_load(filename, &config) == CONFIG_ERR_INVALID_VALUE);

    remove(filename);
    return ok;
}

static int test_config_error_string(void) {
    return strcmp(config_error_string(CONFIG_OK), "success") == 0 &&
           strcmp(config_error_string(CONFIG_ERR_UNKNOWN_KEY), "unknown config key") == 0 &&
           strcmp(config_error_string(CONFIG_ERR_INVALID_RANGE), "invalid config range") == 0;
}

int main(void) {
    test_report("config_init", test_config_init());
    test_report("config_load null args", test_config_load_null_args());
    test_report("config_load open failed", test_config_load_open_failed());
    test_report("config_load minimal valid", test_config_load_minimal_valid());
    test_report("config_load full valid", test_config_load_full_valid());
    test_report("config_load missing frames_dir", test_config_load_missing_frames_dir());
    test_report("config_load missing fps", test_config_load_missing_fps());
    test_report("config_load missing start_frame", test_config_load_missing_start_frame());
    test_report("config_load missing end_frame", test_config_load_missing_end_frame());
    test_report("config_load missing palette", test_config_load_missing_palette());
    test_report("config_load missing threshold", test_config_load_missing_threshold());
    test_report("config_load unknown key", test_config_load_unknown_key());
    test_report("config_load wrong type string for fps", test_config_load_wrong_type_string_for_fps());
    test_report("config_load wrong type int for frames_dir", test_config_load_wrong_type_int_for_frames_dir());
    test_report("config_load negative uint field", test_config_load_negative_uint_field());
    test_report("config_load invalid fps zero", test_config_load_invalid_fps_zero());
    test_report("config_load invalid start_frame zero", test_config_load_invalid_start_frame_zero());
    test_report("config_load invalid end before start", test_config_load_invalid_end_before_start());
    test_report("config_load invalid extension", test_config_load_invalid_extension());
    test_report("config_load invalid frame_digits", test_config_load_invalid_frame_digits());
    test_report("config_load empty frames_dir", test_config_load_empty_frames_dir());
    test_report("config_load empty palette", test_config_load_empty_palette());
    test_report("config_error_string", test_config_error_string());

    test_summary();
    return test_failed_count() == 0 ? 0 : 1;
}
