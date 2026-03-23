#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <stdio.h>
#include <stddef.h>

int io_read_bytes(FILE *fp, void *dst, size_t size);
int io_skip_bytes(FILE *fp, long count);

int io_write_bytes(FILE *fp, const void *src, size_t size);
int io_write_u16_le(FILE *fp, unsigned int value);
int io_write_u32_le(FILE *fp, unsigned long value);

#endif
