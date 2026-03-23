#include "parsers/bmp.h"
#include "common/io_utils.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define BMP_FILE_HEADER_SIZE 14U
#define BMP_INFO_HEADER_SIZE 40U
#define BMP_SUPPORTED_BITS 24U
#define BMP_SUPPORTED_COMPRESSION 0U
#define BMP_SUPPORTED_PLANES 1U
#define BMP_BYTES_PER_PIXEL 3U

static void bmp_reset(BmpImage * bmp);

static int bmp_signature_equals(const unsigned char signature[2]);
static unsigned int bmp_abs_int(int value);

static BmpError bmp_read_file_header(
    FILE * fp, 
    unsigned int * file_size,
    unsigned int * pixel_offset
);

static BmpError bmp_read_info_header(
    FILE * fp,
    unsigned int * dib_size,
    int * width,
    int * height,
    unsigned short * planes,
    unsigned short * bits_per_pixel,
    unsigned int * compression,
    unsigned int * image_size
);

static BmpError bmp_validate_metadata(const BmpImage * bmp);
static BmpError bmp_compute_row_stride(int width, unsigned int * row_stride);
static BmpError bmp_compute_pixel_array_size(
    int width,
    int height,
    unsigned int * pixel_array_size
);

static long bmp_row_file_offset(const BmpImage * bmp, unsigned int row_index);

void bmp_init(BmpImage * bmp){
    if(bmp == NULL){
        return;
    }

    bmp_reset(bmp);
}

void bmp_free(BmpImage * bmp){
    if(bmp == NULL){
        return;
    }

    free(bmp->pixel_data);
    bmp->pixel_data = NULL;

    bmp_reset(bmp);
}

BmpError bmp_load(const char * filename, BmpImage * bmp){
    FILE * fp;
    BmpError err;
    unsigned int pixel_array_size;
    unsigned int visual_height;
    unsigned int row_bytes;
    unsigned int row_index;
    unsigned char * row_buffer;

    if(filename == NULL || bmp == NULL){
        return BMP_ERR_NULL_ARG;
    }

    bmp_free(bmp);

    fp = fopen(filename, "rb");
    if(fp == NULL){
        return BMP_ERR_OPEN_FAILED;
    }

    err = bmp_read_file_header(fp, &bmp->file_size, &bmp->pixel_offset);
    if(err != BMP_OK){
        fclose(fp);
        return err;
    }

    err = bmp_read_info_header(
        fp,
        &bmp->dib_size,
        &bmp->width,
        &bmp->height,
        &bmp->planes,
        &bmp->bits_per_pixel,
        &bmp->compression,
        &bmp->image_size
    );

    if(err != BMP_OK){
        fclose(fp);
        return err;
    }

    bmp->is_top_down = (bmp->height < 0) ? 1 : 0;

    err = bmp_validate_metadata(bmp);
    if(err != BMP_OK){
        fclose(fp);
        return err;
    }

    err = bmp_compute_row_stride(bmp->width, &bmp->row_stride);

    if(err != BMP_OK){
        fclose(fp);
        return err;
    }

    err = bmp_compute_pixel_array_size(
        bmp->width,
        bmp_abs_int(bmp->height),
        &pixel_array_size
    );

    if(err != BMP_OK){
        fclose(fp);
        return err;
    }

    if(bmp->pixel_offset < BMP_FILE_HEADER_SIZE + bmp->dib_size){
        fclose(fp);
        return BMP_ERR_INVALID_OFFSET;
    }

    bmp->pixel_data = (unsigned char *) malloc(pixel_array_size);
    if(bmp->pixel_data == NULL){
        fclose(fp);
        bmp_free(bmp);
        return BMP_ERR_MEMORY;
    }

    row_bytes = (unsigned int) bmp->width * BMP_BYTES_PER_PIXEL;
    visual_height = bmp_abs_int(bmp->height);

    row_buffer = (unsigned char *) malloc(row_bytes);
    if(row_buffer == NULL){
        fclose(fp);
        bmp_free(bmp);
        return BMP_ERR_MEMORY;
    }

    for(row_index = 0U; row_index < visual_height; row_index++){
        err = bmp_read_row(fp, bmp, row_index, row_buffer);
        if(err != BMP_OK){
            free(row_buffer);
            fclose(fp);
            bmp_free(bmp);
            return err;
        }

        memcpy(
            bmp->pixel_data + (row_index * row_bytes),
            row_buffer,
            row_bytes
        );
    }

    free(row_buffer);
    fclose(fp);

    bmp->height = (int) visual_height;
    bmp->is_top_down = 1;

    return BMP_OK;
}

BmpError bmp_read_row(
    FILE * fp,
    const BmpImage * bmp,
    unsigned int row_index,
    unsigned char * row_buffer
){
    unsigned int visual_height;
    unsigned int row_bytes;
    long row_offset;

    if(fp == NULL || bmp == NULL || row_buffer == NULL){
        return BMP_ERR_NULL_ARG;
    }

    if(bmp->width <= 0 || bmp->height == 0){
        return BMP_ERR_INVALID_DIMENSIONS;
    }

    visual_height = bmp_abs_int(bmp->height);

    if(row_index >= visual_height){
        return BMP_ERR_INVALID_DIMENSIONS;
    }

    row_bytes = (unsigned int) bmp->width * BMP_BYTES_PER_PIXEL;
    row_offset = bmp_row_file_offset(bmp, row_index);

    if(row_offset < 0L){
        return BMP_ERR_SEEK_FAILED;
    }

    if(fseek(fp, row_offset, SEEK_SET) != 0){
        return BMP_ERR_SEEK_FAILED;
    }

    if(!io_read_bytes(fp, row_buffer, row_bytes)){
        return BMP_ERR_READ_FAILED;
    }

    return BMP_OK;
}

const char * bmp_error_string(BmpError err){
    switch (err) {
        case BMP_OK:
            return "success";
        case BMP_ERR_NULL_ARG:
            return "null argument";
        case BMP_ERR_OPEN_FAILED:
            return "failed to open BMP file";
        case BMP_ERR_READ_FAILED:
            return "failed to read BMP file";
        case BMP_ERR_SEEK_FAILED:
            return "failed to seek in BMP file";
        case BMP_ERR_BAD_SIGNATURE:
            return "invalid BMP signature";
        case BMP_ERR_UNSUPPORTED_DIB:
            return "unsupported DIB header";
        case BMP_ERR_UNSUPPORTED_PLANES:
            return "unsupported BMP plane count";
        case BMP_ERR_UNSUPPORTED_COMPRESSION:
            return "unsupported BMP compression";
        case BMP_ERR_UNSUPPORTED_BITS:
            return "unsupported BMP bits per pixel";
        case BMP_ERR_INVALID_DIMENSIONS:
            return "invalid BMP dimensions";
        case BMP_ERR_INVALID_OFFSET:
            return "invalid BMP pixel offset";
        case BMP_ERR_ROW_SIZE_OVERFLOW:
            return "BMP row size overflow";
        case BMP_ERR_IMAGE_SIZE_OVERFLOW:
            return "BMP image size overflow";
        case BMP_ERR_MEMORY:
            return "memory allocation failed";
        default:
            break;
    }

    return "unknown BMP error";
}

static void bmp_reset(BmpImage *bmp) {
    bmp->file_size = 0U;
    bmp->pixel_offset = 0U;
    bmp->dib_size = 0U;
    bmp->width = 0;
    bmp->height = 0;
    bmp->planes = 0U;
    bmp->bits_per_pixel = 0U;
    bmp->compression = 0U;
    bmp->image_size = 0U;
    bmp->row_stride = 0U;
    bmp->is_top_down = 0;
    bmp->pixel_data = NULL;
}

static int bmp_signature_equals(const unsigned char signature[2]){
    return signature[0] == 'B' && signature[1] == 'M';
}

static unsigned int bmp_abs_int(int value){
    return value < 0 ? (unsigned int)(-value) : (unsigned int)value;
}

static BmpError bmp_read_file_header(
    FILE * fp,
    unsigned int * file_size,
    unsigned int * pixel_offset
){
    unsigned char signature[2];
    unsigned short reserved1;
    unsigned short reserved2;

    if(fp == NULL || file_size == NULL || pixel_offset == NULL){
        return BMP_ERR_NULL_ARG;
    }

    if(!io_read_bytes(fp, signature, 2U)){
        return BMP_ERR_READ_FAILED;
    }

    if(!bmp_signature_equals(signature)){
        return BMP_ERR_BAD_SIGNATURE;
    }

    if(!io_read_bytes(fp, file_size, sizeof(* file_size))){
        return BMP_ERR_READ_FAILED;
    }

    if(!io_read_bytes(fp, &reserved1, sizeof(reserved1))){
        return BMP_ERR_READ_FAILED;
    }

    if(!io_read_bytes(fp, &reserved2, sizeof(reserved2))){
        return BMP_ERR_READ_FAILED;
    }

    if(!io_read_bytes(fp, pixel_offset, sizeof(* pixel_offset))){
        return BMP_ERR_READ_FAILED;
    }

    return BMP_OK;
}

static BmpError bmp_read_info_header(
    FILE *fp,
    unsigned int *dib_size,
    int *width,
    int *height,
    unsigned short *planes,
    unsigned short *bits_per_pixel,
    unsigned int *compression,
    unsigned int *image_size
) {
    unsigned int x_pixels_per_meter;
    unsigned int y_pixels_per_meter;
    unsigned int colors_used;
    unsigned int colors_important;

    if (fp == NULL ||
        dib_size == NULL ||
        width == NULL ||
        height == NULL ||
        planes == NULL ||
        bits_per_pixel == NULL ||
        compression == NULL ||
        image_size == NULL) {
        return BMP_ERR_NULL_ARG;
    }

    if (!io_read_bytes(fp, dib_size, sizeof(*dib_size))) {
        return BMP_ERR_READ_FAILED;
    }

    if (*dib_size < BMP_INFO_HEADER_SIZE) {
        return BMP_ERR_UNSUPPORTED_DIB;
    }

    if (!io_read_bytes(fp, width, sizeof(*width))) {
        return BMP_ERR_READ_FAILED;
    }

    if (!io_read_bytes(fp, height, sizeof(*height))) {
        return BMP_ERR_READ_FAILED;
    }

    if (!io_read_bytes(fp, planes, sizeof(*planes))) {
        return BMP_ERR_READ_FAILED;
    }

    if (!io_read_bytes(fp, bits_per_pixel, sizeof(*bits_per_pixel))) {
        return BMP_ERR_READ_FAILED;
    }

    if (!io_read_bytes(fp, compression, sizeof(*compression))) {
        return BMP_ERR_READ_FAILED;
    }

    if (!io_read_bytes(fp, image_size, sizeof(*image_size))) {
        return BMP_ERR_READ_FAILED;
    }

    if (!io_read_bytes(fp, &x_pixels_per_meter, sizeof(x_pixels_per_meter))) {
        return BMP_ERR_READ_FAILED;
    }

    if (!io_read_bytes(fp, &y_pixels_per_meter, sizeof(y_pixels_per_meter))) {
        return BMP_ERR_READ_FAILED;
    }

    if (!io_read_bytes(fp, &colors_used, sizeof(colors_used))) {
        return BMP_ERR_READ_FAILED;
    }

    if (!io_read_bytes(fp, &colors_important, sizeof(colors_important))) {
        return BMP_ERR_READ_FAILED;
    }

    if (*dib_size > BMP_INFO_HEADER_SIZE) {
        if (!io_skip_bytes(fp, (long)(*dib_size - BMP_INFO_HEADER_SIZE))) {
            return BMP_ERR_SEEK_FAILED;
        }
    }

    return BMP_OK;
}

static BmpError bmp_validate_metadata(const BmpImage * bmp){
    if(bmp == NULL){
        return BMP_ERR_NULL_ARG;
    }

    if(bmp->width <= 0 || bmp->height == 0){
        return BMP_ERR_INVALID_DIMENSIONS;
    }

    if(bmp->planes != BMP_SUPPORTED_PLANES){
        return BMP_ERR_UNSUPPORTED_PLANES;
    }

    if(bmp->compression != BMP_SUPPORTED_COMPRESSION){
        return BMP_ERR_UNSUPPORTED_COMPRESSION;
    }

    if(bmp->bits_per_pixel != BMP_SUPPORTED_BITS){
        return BMP_ERR_UNSUPPORTED_BITS;
    }

    return BMP_OK;
}

static BmpError bmp_compute_row_stride(int width, unsigned int * row_stride){
    unsigned long raw_row_bytes;
    unsigned long padded_row_bytes;

    if(row_stride == NULL){
        return BMP_ERR_NULL_ARG;
    }

    if(width <= 0){
        return BMP_ERR_INVALID_DIMENSIONS;
    }

    raw_row_bytes = (unsigned long) width * BMP_BYTES_PER_PIXEL;
    padded_row_bytes = (raw_row_bytes + 3UL) & ~3UL;

    if(padded_row_bytes > (unsigned long) UINT_MAX){
        return BMP_ERR_ROW_SIZE_OVERFLOW;
    }

    *row_stride = (unsigned int) padded_row_bytes;

    return BMP_OK;
}

static BmpError bmp_compute_pixel_array_size(
    int width,
    int height,
    unsigned int * pixel_array_size
){
    unsigned long total_bytes;

    if(pixel_array_size == NULL){
        return BMP_ERR_NULL_ARG;
    }

    if(width <= 0 || height <= 0){
        return BMP_ERR_INVALID_DIMENSIONS;
    }

    total_bytes = (unsigned long) width *
                    (unsigned long) height *
                    (unsigned long) BMP_BYTES_PER_PIXEL;

    if(total_bytes > (unsigned long) UINT_MAX){
        return BMP_ERR_IMAGE_SIZE_OVERFLOW;
    }

    *pixel_array_size = (unsigned int) total_bytes;

    return BMP_OK;
}

static long bmp_row_file_offset(const BmpImage * bmp, unsigned int row_index){
    unsigned int visual_height;
    unsigned int disk_row_index;
    unsigned long offset;

    if(bmp == NULL){
        return -1L;
    }

    visual_height = bmp_abs_int(bmp->height);

    if(row_index >= visual_height){
        return -1L;
    }

    if(bmp->is_top_down){
        disk_row_index = row_index;
    } else {
        disk_row_index = visual_height - 1U - row_index;
    }

    offset = (unsigned long) bmp->pixel_offset +
                ((unsigned long) disk_row_index * (unsigned long)bmp->row_stride);

    if(offset > (unsigned long) LONG_MAX){
        return -1L;
    }

    return (long) offset;
}
