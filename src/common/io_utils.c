#include "common/io_utils.h"

int io_read_bytes(FILE * fp, void * dst, size_t size){
    if(fp == NULL || dst == NULL){
        return 0;
    }

    return fread(dst, 1, size, fp) == size;
}

int io_skip_bytes(FILE * fp, long count){
    if(fp == NULL || count < 0){
        return 0;
    }

    return fseek(fp, count, SEEK_CUR) == 0;
}

int io_write_bytes(FILE *fp, const void *src, size_t size) {
    if (fp == NULL || src == NULL) {
        return 0;
    }
    return fwrite(src, 1, size, fp) == size;
}

int io_write_u16_le(FILE *fp, unsigned int value) {
    unsigned char b[2];

    b[0] = (unsigned char)(value & 0xFFU);
    b[1] = (unsigned char)((value >> 8) & 0xFFU);

    return io_write_bytes(fp, b, 2);
}

int io_write_u32_le(FILE *fp, unsigned long value) {
    unsigned char b[4];

    b[0] = (unsigned char)(value & 0xFFUL);
    b[1] = (unsigned char)((value >> 8) & 0xFFUL);
    b[2] = (unsigned char)((value >> 16) & 0xFFUL);
    b[3] = (unsigned char)((value >> 24) & 0xFFUL);

    return io_write_bytes(fp, b, 4);
}

int io_write_s32_le(FILE * fp, int value){
    return io_write_u32_le(fp, (unsigned long)((unsigned int)value));
}

int io_write_text_file(const char *filename, const char *text) {
    FILE *fp;
    size_t length;

    if (filename == NULL || text == NULL) {
        return 0;
    }

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        return 0;
    }

    length = strlen(text);

    if (!io_write_bytes(fp, text, length)) {
        fclose(fp);
        return 0;
    }

    fclose(fp);
    return 1;
}
