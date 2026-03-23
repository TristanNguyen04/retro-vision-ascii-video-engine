#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <stdio.h>
#include <stddef.h>

int io_read_bytes(FILE * fp, void * dst, size_t size);
int io_skip_bytes(FILE * fp, long count);

#endif
