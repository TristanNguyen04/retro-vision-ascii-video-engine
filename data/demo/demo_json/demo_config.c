#include "parsers/json/config.h"
#include "parsers/json/json.h"

#include <stdio.h>

void print_engine_config(const EngineConfig * config){
    if(config == NULL){
        return;
    }

    printf("frames_dir: %s\n", config->frames_dir);
    printf("frame_prefix: %s\n", config->frame_prefix);
    printf("frame_extension: %s\n", config->frame_extension);
    printf("frame_digits: %u\n", config->frame_digits);

    printf("fps: %u\n", config->fps);
    printf("start_frame: %u\n", config->start_frame);
    printf("end_frame: %u\n", config->end_frame);

    printf("palette: %s\n", config->palette);
    printf("threshold: %u\n", config->threshold);

    return;
}

int main(int argc, char **argv) {
    ConfigError err;
    EngineConfig config;

    if (argc != 2) {
        printf("Usage: %s <config_json_file>\n", argv[0]);
        return 1;
    }

    config_init(&config);

    err = config_load(argv[1], &config);

    printf("error: %s\n", config_error_string(err));

    if (err == CONFIG_OK) {
        print_engine_config(&config);
    }

    return (err == CONFIG_OK) ? 0 : 1;
}