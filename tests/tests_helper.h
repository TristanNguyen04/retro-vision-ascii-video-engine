#ifndef TESTS_HELPER_H
#define TESTS_HELPER_H

#include <stddef.h>

void test_report(const char * name, int passed);
void test_summary(void);
int test_failed_count(void);

int make_temp_name(char *buffer, size_t size);

#endif
