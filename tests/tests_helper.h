#ifndef TESTS_HELPER_H
#define TESTS_HELPER_H

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

void test_report(const char * name, int passed);
void test_summary(void);
int test_failed_count(void);

int make_temp_name(char *buffer, size_t size);
int file_contains_text(FILE *fp, const char *text);

#endif
