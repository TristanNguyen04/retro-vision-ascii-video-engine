#include "parsers/bmp.h"

#include <stdio.h>

int main(int argc, char **argv) {
    unsigned int i, j;
    unsigned int max_rows = 5;
    unsigned int max_cols = 5;

    BmpError err;
    BmpImage bmp;

    if (argc != 2) {
        printf("Usage: %s <bmp_file>\n", argv[0]);
        return 1;
    }

    bmp_init(&bmp);

    err = bmp_load(argv[1], &bmp);

    printf("error: %s\n", bmp_error_string(err));

    if (err == BMP_OK) {
        printf("file_size: %u\n", bmp.file_size);
        printf("pixel_offset: %u\n", bmp.pixel_offset);

        printf("dib_size: %u\n", bmp.dib_size);
        printf("width: %d\n", bmp.width);
        printf("height: %d\n", bmp.height);
        printf("bits_per_pixel: %u\n", bmp.bits_per_pixel);
        printf("compression: %u\n", bmp.compression);
        printf("image_size: %u\n", bmp.image_size);

        printf("row_stride %u\n", bmp.row_stride);
        printf("is_top_down: %s\n", bmp.is_top_down != 0 ? "yes" : "no");

        printf("\n====== preview of pixel_data ======\n");

        if(max_rows >= bmp.height){
            max_rows = (unsigned int) bmp.height;
        }

        if(max_cols >= bmp.width){
            max_cols = (unsigned int) bmp.width;
        }

        for(i = 0; i < max_rows; i++){
            for(j = 0; j < max_cols; j++){
                unsigned int index = ((unsigned int) i * bmp.width + j) * 3U;

                unsigned char b = bmp.pixel_data[index];
                unsigned char g = bmp.pixel_data[index + 1U];
                unsigned char r = bmp.pixel_data[index + 2U];

                unsigned int gray = (r + g + b) / 3U;

                printf("(%3u,%3u,%3u) - grayscale: %u\n", b, g, r, gray);
            }
        }
    }

    bmp_free(&bmp);
    return (err == BMP_OK) ? 0 : 1;
}