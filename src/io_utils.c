#include "io_utils.h"

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
