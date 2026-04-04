#include "components/engine.h"

#include <stdio.h>

int main(int argc, char **argv) {
    EngineError err;

    if (argc != 5) {
        printf("Usage: %s <config.json> <audio.wav> <render.txt> <process.log>\n",
               argv[0]);
        return 1;
    }

    err = engine_run(argv[1], argv[2], argv[3], argv[4]);

    if (err != ENGINE_OK) {
        printf("engine error: %s\n", engine_error_string(err));
        return 1;
    }

    printf("engine completed successfully\n");
    printf("render output: %s\n", argv[3]);
    printf("log output: %s\n", argv[4]);

    return 0;
}
