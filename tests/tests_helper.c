#include "tests_helper.h"

#include <stdio.h>

static int tests_run = 0;
static int tests_failed = 0;
static unsigned int temp_file_counter = 0;

void test_report(const char * name, int passed){
    tests_run++;

    if(passed){
        printf("PASS: %s\n", name);
    } else {
        printf("FAIL: %s\n", name);
        tests_failed++;
    }
}

void test_summary(void){
    printf("\nTests run: %d\n", tests_run);
    printf("\nTests failed: %d\n", tests_failed);
}

int test_failed_count(void){
    return tests_failed;
}

int make_temp_name(char *buffer, size_t size) {
    int written;

    if (buffer == NULL || size == 0U) {
        return 0;
    }

    temp_file_counter++;

    written = snprintf(
        buffer,
        size,
        "build/tests/tmp_test_%u.bin",
        temp_file_counter
    );

    if (written < 0 || (size_t)written >= size) {
        return 0;
    }

    return 1;
}
