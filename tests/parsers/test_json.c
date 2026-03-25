#include "parsers/json.h"
#include "common/io_utils.h"
#include "../tests_helper.h"

#include <stdio.h>

static int test_json_init_and_free(void);
static int test_json_load_null_args(void);
static int test_json_load_open_failed(void);
static int test_json_load_empty_object(void);
static int test_json_load_valid_flat_object(void);
static int test_json_load_whitespace_object(void);
static int test_json_load_missing_object_start(void);
static int test_json_load_missing_string_key(void);
static int test_json_load_missing_colon(void);
static int test_json_load_missing_value(void);
static int test_json_load_missing_comma(void);
static int test_json_load_duplicate_key(void);
static int test_json_load_trailing_content(void);
static int test_json_load_unsupported_nested_object(void);
static int test_json_load_invalid_string_escape(void);
static int test_json_load_key_too_long(void);
static int test_json_load_string_too_long(void);
static int test_json_find_pair_missing_key(void);
static int test_json_error_string(void);

static int test_json_init_and_free(void) {
    JsonObject obj;

    obj.pairs = (JsonPair *)1;
    obj.count = 99U;

    json_init(&obj);

    if (obj.pairs != NULL || obj.count != 0U) {
        return 0;
    }

    json_free(&obj);

    if (obj.pairs != NULL || obj.count != 0U) {
        return 0;
    }

    return 1;
}

static int test_json_load_null_args(void) {
    JsonObject obj;

    json_init(&obj);

    if (json_load(NULL, &obj) != JSON_ERR_NULL_ARG) {
        json_free(&obj);
        return 0;
    }

    if (json_load("dummy.json", NULL) != JSON_ERR_NULL_ARG) {
        json_free(&obj);
        return 0;
    }

    json_free(&obj);
    return 1;
}

static int test_json_load_open_failed(void) {
    JsonObject obj;
    int ok;

    json_init(&obj);
    ok = (json_load("definitely_missing_file_12345.json", &obj) == JSON_ERR_OPEN_FAILED);
    json_free(&obj);

    return ok;
}

static int test_json_load_empty_object(void) {
    char filename[128];
    JsonObject obj;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(filename, "{}")) {
        return 0;
    }

    json_init(&obj);
    ok = (json_load(filename, &obj) == JSON_OK) &&
         obj.count == 0U;

    json_free(&obj);
    remove(filename);

    return ok;
}

static int test_json_load_valid_flat_object(void) {
    char filename[128];
    JsonObject obj;
    const JsonPair *pair_name;
    const JsonPair *pair_fps;
    const JsonPair *pair_start;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "{"
            "\"frames_dir\":\"frames\","
            "\"fps\":30,"
            "\"start_frame\":1"
            "}"
        )) {
        return 0;
    }

    json_init(&obj);
    ok = (json_load(filename, &obj) == JSON_OK);

    if (ok) {
        pair_name = json_find_pair(&obj, "frames_dir");
        pair_fps = json_find_pair(&obj, "fps");
        pair_start = json_find_pair(&obj, "start_frame");

        ok = ok &&
            obj.count == 3U &&
            pair_name != NULL &&
            pair_name->type == JSON_VALUE_STRING &&
            strcmp(pair_name->string_value, "frames") == 0 &&
            pair_fps != NULL &&
            pair_fps->type == JSON_VALUE_INT &&
            pair_fps->int_value == 30 &&
            pair_start != NULL &&
            pair_start->type == JSON_VALUE_INT &&
            pair_start->int_value == 1;
    }

    json_free(&obj);
    remove(filename);

    return ok;
}

static int test_json_load_whitespace_object(void) {
    char filename[128];
    JsonObject obj;
    const JsonPair *pair;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(
            filename,
            "  \n\t { \n"
            "  \"palette\"   :   \"@#*\" , \n"
            "  \"threshold\" : 5000 \n"
            "} \n\t "
        )) {
        return 0;
    }

    json_init(&obj);
    ok = (json_load(filename, &obj) == JSON_OK);

    if (ok) {
        pair = json_find_pair(&obj, "threshold");

        ok = ok &&
            obj.count == 2U &&
            pair != NULL &&
            pair->type == JSON_VALUE_INT &&
            pair->int_value == 5000;
    }

    json_free(&obj);
    remove(filename);

    return ok;
}

static int test_json_load_missing_object_start(void) {
    char filename[128];
    JsonObject obj;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(filename, "\"fps\":30}")) {
        return 0;
    }

    json_init(&obj);
    ok = (json_load(filename, &obj) == JSON_ERR_EXPECTED_OBJECT_START);

    json_free(&obj);
    remove(filename);

    return ok;
}

static int test_json_load_missing_string_key(void) {
    char filename[128];
    JsonObject obj;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(filename, "{fps:30}")) {
        return 0;
    }

    json_init(&obj);
    ok = (json_load(filename, &obj) == JSON_ERR_EXPECTED_STRING_KEY);

    json_free(&obj);
    remove(filename);

    return ok;
}

static int test_json_load_missing_colon(void) {
    char filename[128];
    JsonObject obj;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(filename, "{\"fps\" 30}")) {
        return 0;
    }

    json_init(&obj);
    ok = (json_load(filename, &obj) == JSON_ERR_EXPECTED_COLON);

    json_free(&obj);
    remove(filename);

    return ok;
}

static int test_json_load_missing_value(void) {
    char filename[128];
    JsonObject obj;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(filename, "{\"fps\":}")) {
        return 0;
    }

    json_init(&obj);
    ok = (json_load(filename, &obj) == JSON_ERR_EXPECTED_VALUE);

    json_free(&obj);
    remove(filename);

    return ok;
}

static int test_json_load_missing_comma(void) {
    char filename[128];
    JsonObject obj;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(filename, "{\"fps\":30 \"start_frame\":1}")) {
        return 0;
    }

    json_init(&obj);
    ok = (json_load(filename, &obj) == JSON_ERR_EXPECTED_COMMA_OR_OBJECT_END);

    json_free(&obj);
    remove(filename);

    return ok;
}

static int test_json_load_duplicate_key(void) {
    char filename[128];
    JsonObject obj;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(filename, "{\"fps\":30,\"fps\":60}")) {
        return 0;
    }

    json_init(&obj);
    ok = (json_load(filename, &obj) == JSON_ERR_DUPLICATE_KEY);

    json_free(&obj);
    remove(filename);

    return ok;
}

static int test_json_load_trailing_content(void) {
    char filename[128];
    JsonObject obj;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(filename, "{\"fps\":30} xyz")) {
        return 0;
    }

    json_init(&obj);
    ok = (json_load(filename, &obj) == JSON_ERR_TRAILING_CONTENT);

    json_free(&obj);
    remove(filename);

    return ok;
}

static int test_json_load_unsupported_nested_object(void) {
    char filename[128];
    JsonObject obj;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(filename, "{\"seq\":{\"fps\":30}}")) {
        return 0;
    }

    json_init(&obj);
    ok = (json_load(filename, &obj) == JSON_ERR_EXPECTED_VALUE);

    json_free(&obj);
    remove(filename);

    return ok;
}

static int test_json_load_invalid_string_escape(void) {
    char filename[128];
    JsonObject obj;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(filename, "{\"name\":\"bad\\qescape\"}")) {
        return 0;
    }

    json_init(&obj);
    ok = (json_load(filename, &obj) == JSON_ERR_INVALID_STRING);

    json_free(&obj);
    remove(filename);

    return ok;
}

static int test_json_load_key_too_long(void) {
    char filename[128];
    JsonObject obj;
    char json_text[256];
    char long_key[128];
    unsigned int i;
    int ok;

    for (i = 0U; i < 70U; i++) {
        long_key[i] = 'a';
    }
    long_key[70] = '\0';

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    snprintf(json_text, sizeof(json_text), "{\"%s\":1}", long_key);

    if (!io_write_text_file(filename, json_text)) {
        return 0;
    }

    json_init(&obj);
    ok = (json_load(filename, &obj) == JSON_ERR_KEY_TOO_LONG);

    json_free(&obj);
    remove(filename);

    return ok;
}

static int test_json_load_string_too_long(void) {
    char filename[128];
    JsonObject obj;
    char json_text[512];
    char long_value[320];
    unsigned int i;
    int ok;

    for (i = 0U; i < 300U; i++) {
        long_value[i] = 'b';
    }
    long_value[300] = '\0';

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    snprintf(json_text, sizeof(json_text), "{\"name\":\"%s\"}", long_value);

    if (!io_write_text_file(filename, json_text)) {
        return 0;
    }

    json_init(&obj);
    ok = (json_load(filename, &obj) == JSON_ERR_STRING_TOO_LONG);

    json_free(&obj);
    remove(filename);

    return ok;
}

static int test_json_find_pair_missing_key(void) {
    char filename[128];
    JsonObject obj;
    const JsonPair *pair;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!io_write_text_file(filename, "{\"fps\":30}")) {
        return 0;
    }

    json_init(&obj);
    ok = (json_load(filename, &obj) == JSON_OK);
    if (!ok) {
        json_free(&obj);
        remove(filename);
        return 0;
    }

    pair = json_find_pair(&obj, "missing_key");
    ok = (pair == NULL);

    json_free(&obj);
    remove(filename);

    return ok;
}

static int test_json_error_string(void) {
    return strcmp(json_error_string(JSON_OK), "success") == 0 &&
           strcmp(json_error_string(JSON_ERR_DUPLICATE_KEY), "duplicate key") == 0;
}

int main(void) {
    test_report("json_init and json_free", test_json_init_and_free());
    test_report("json_load null args", test_json_load_null_args());
    test_report("json_load open failed", test_json_load_open_failed());
    test_report("json_load empty object", test_json_load_empty_object());
    test_report("json_load valid flat object", test_json_load_valid_flat_object());
    test_report("json_load whitespace object", test_json_load_whitespace_object());
    test_report("json_load missing object start", test_json_load_missing_object_start());
    test_report("json_load missing string key", test_json_load_missing_string_key());
    test_report("json_load missing colon", test_json_load_missing_colon());
    test_report("json_load missing value", test_json_load_missing_value());
    test_report("json_load missing comma", test_json_load_missing_comma());
    test_report("json_load duplicate key", test_json_load_duplicate_key());
    test_report("json_load trailing content", test_json_load_trailing_content());
    test_report("json_load unsupported nested object", test_json_load_unsupported_nested_object());
    test_report("json_load invalid string escape", test_json_load_invalid_string_escape());
    test_report("json_load key too long", test_json_load_key_too_long());
    test_report("json_load string too long", test_json_load_string_too_long());
    test_report("json_find_pair missing key", test_json_find_pair_missing_key());
    test_report("json_error_string", test_json_error_string());

    test_summary();
    return test_failed_count() == 0 ? 0 : 1;
}
