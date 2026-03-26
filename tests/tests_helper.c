#include "tests_helper.h"

#include <string.h>
#include <stdlib.h>

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

int file_contains_text(FILE *fp, const char *text) {
    long size;
    char *buffer;
    size_t read_count;
    int found;

    if (fp == NULL || text == NULL) {
        return 0;
    }

    if (fseek(fp, 0L, SEEK_END) != 0) {
        return 0;
    }

    size = ftell(fp);
    if (size < 0L) {
        return 0;
    }

    if (fseek(fp, 0L, SEEK_SET) != 0) {
        return 0;
    }

    buffer = (char *)malloc((size_t)size + 1U);
    if (buffer == NULL) {
        return 0;
    }

    read_count = fread(buffer, 1U, (size_t)size, fp);
    buffer[read_count] = '\0';

    found = (strstr(buffer, text) != NULL);

    free(buffer);
    return found;
}
