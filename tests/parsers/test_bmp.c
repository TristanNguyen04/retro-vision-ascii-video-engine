#include "parsers/bmp.h"
#include "common/io_utils.h"
#include "../tests_helper.h"

#include <stdio.h>
#include <string.h>

#define BMP_TEST_FILE_HEADER_SIZE 14U
#define BMP_TEST_INFO_HEADER_SIZE 40U
#define BMP_TEST_BYTES_PER_PIXEL 3U

static unsigned int compute_row_stride(unsigned int width);

static int write_bmp24_file(
    const char *filename,
    unsigned int width,
    unsigned int height,
    int top_down,
    const unsigned char *visual_pixels
);

static int write_bmp_with_custom_header(
    const char *filename,
    const char signature[2],
    unsigned int width,
    int height,
    unsigned short planes,
    unsigned short bits_per_pixel,
    unsigned int compression
);

static int pixels_equal(
    const unsigned char *a,
    const unsigned char *b,
    unsigned int size
);

static int test_bmp_init_and_free(void);
static int test_bmp_load_null_args(void);
static int test_bmp_load_open_failed(void);
static int test_bmp_load_valid_bottom_up_2x2(void);
static int test_bmp_load_valid_top_down_1x2(void);
static int test_bmp_load_row_padding_width_1(void);
static int test_bmp_load_bad_signature(void);
static int test_bmp_load_unsupported_bits(void);
static int test_bmp_load_unsupported_compression(void);
static int test_bmp_load_invalid_dimensions(void);
static int test_bmp_read_row_top_down(void);
static int test_bmp_error_string(void);

static unsigned int compute_row_stride(unsigned int width) {
    unsigned int row_bytes;

    row_bytes = width * BMP_TEST_BYTES_PER_PIXEL;
    return (row_bytes + 3U) & ~3U;
}

static int write_bmp24_file(
    const char *filename,
    unsigned int width,
    unsigned int height,
    int top_down,
    const unsigned char *visual_pixels
) {
    FILE *fp;
    unsigned int row_stride;
    unsigned int row_bytes;
    unsigned int image_size;
    unsigned int pixel_offset;
    unsigned int file_size;
    unsigned int disk_row;
    unsigned int visual_row;
    int height_field;
    unsigned char padding[3];

    if (filename == NULL || visual_pixels == NULL || width == 0U || height == 0U) {
        return 0;
    }

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        return 0;
    }

    row_bytes = width * BMP_TEST_BYTES_PER_PIXEL;
    row_stride = compute_row_stride(width);
    image_size = row_stride * height;
    pixel_offset = BMP_TEST_FILE_HEADER_SIZE + BMP_TEST_INFO_HEADER_SIZE;
    file_size = pixel_offset + image_size;
    height_field = top_down ? -(int)height : (int)height;

    padding[0] = 0U;
    padding[1] = 0U;
    padding[2] = 0U;

    if (!io_write_bytes(fp, "BM", 2U) ||
        !io_write_u32_le(fp, (unsigned long)file_size) ||
        !io_write_u16_le(fp, 0U) ||
        !io_write_u16_le(fp, 0U) ||
        !io_write_u32_le(fp, (unsigned long)pixel_offset) ||
        !io_write_u32_le(fp, (unsigned long)BMP_TEST_INFO_HEADER_SIZE) ||
        !io_write_s32_le(fp, (int)width) ||
        !io_write_s32_le(fp, height_field) ||
        !io_write_u16_le(fp, 1U) ||
        !io_write_u16_le(fp, 24U) ||
        !io_write_u32_le(fp, 0UL) ||
        !io_write_u32_le(fp, (unsigned long)image_size) ||
        !io_write_s32_le(fp, 0) ||
        !io_write_s32_le(fp, 0) ||
        !io_write_u32_le(fp, 0UL) ||
        !io_write_u32_le(fp, 0UL)) {
        fclose(fp);
        return 0;
    }

    for (disk_row = 0U; disk_row < height; disk_row++) {
        if (top_down) {
            visual_row = disk_row;
        } else {
            visual_row = height - 1U - disk_row;
        }

        if (!io_write_bytes(
                fp,
                visual_pixels + (visual_row * row_bytes),
                row_bytes
            )) {
            fclose(fp);
            return 0;
        }

        if (row_stride > row_bytes) {
            if (!io_write_bytes(fp, padding, row_stride - row_bytes)) {
                fclose(fp);
                return 0;
            }
        }
    }

    fclose(fp);
    return 1;
}

static int write_bmp_with_custom_header(
    const char *filename,
    const char signature[2],
    unsigned int width,
    int height,
    unsigned short planes,
    unsigned short bits_per_pixel,
    unsigned int compression
) {
    FILE *fp;
    unsigned int row_stride;
    unsigned int image_size;
    unsigned int pixel_offset;
    unsigned int file_size;
    unsigned char zero_row[4];

    if (filename == NULL || signature == NULL || width == 0U || height == 0) {
        return 0;
    }

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        return 0;
    }

    row_stride = compute_row_stride(width);
    image_size = row_stride * (unsigned int)((height < 0) ? -height : height);
    pixel_offset = BMP_TEST_FILE_HEADER_SIZE + BMP_TEST_INFO_HEADER_SIZE;
    file_size = pixel_offset + image_size;

    zero_row[0] = 0U;
    zero_row[1] = 0U;
    zero_row[2] = 0U;
    zero_row[3] = 0U;

    if (!io_write_bytes(fp, signature, 2U) ||
        !io_write_u32_le(fp, (unsigned long)file_size) ||
        !io_write_u16_le(fp, 0U) ||
        !io_write_u16_le(fp, 0U) ||
        !io_write_u32_le(fp, (unsigned long)pixel_offset) ||
        !io_write_u32_le(fp, (unsigned long)BMP_TEST_INFO_HEADER_SIZE) ||
        !io_write_s32_le(fp, (int)width) ||
        !io_write_s32_le(fp, height) ||
        !io_write_u16_le(fp, planes) ||
        !io_write_u16_le(fp, bits_per_pixel) ||
        !io_write_u32_le(fp, (unsigned long)compression) ||
        !io_write_u32_le(fp, (unsigned long)image_size) ||
        !io_write_s32_le(fp, 0) ||
        !io_write_s32_le(fp, 0) ||
        !io_write_u32_le(fp, 0UL) ||
        !io_write_u32_le(fp, 0UL)) {
        fclose(fp);
        return 0;
    }

    if (!io_write_bytes(fp, zero_row, row_stride)) {
        fclose(fp);
        return 0;
    }

    fclose(fp);
    return 1;
}

static int pixels_equal(
    const unsigned char *a,
    const unsigned char *b,
    unsigned int size
) {
    return memcmp(a, b, size) == 0;
}

static int test_bmp_init_and_free(void) {
    BmpImage bmp;

    bmp.file_size = 123U;
    bmp.pixel_offset = 456U;
    bmp.dib_size = 40U;
    bmp.width = 2;
    bmp.height = 2;
    bmp.planes = 1U;
    bmp.bits_per_pixel = 24U;
    bmp.compression = 0U;
    bmp.image_size = 16U;
    bmp.row_stride = 8U;
    bmp.is_top_down = 1;
    bmp.pixel_data = (unsigned char *)1;

    bmp_init(&bmp);

    if (bmp.file_size != 0U ||
        bmp.pixel_offset != 0U ||
        bmp.dib_size != 0U ||
        bmp.width != 0 ||
        bmp.height != 0 ||
        bmp.planes != 0U ||
        bmp.bits_per_pixel != 0U ||
        bmp.compression != 0U ||
        bmp.image_size != 0U ||
        bmp.row_stride != 0U ||
        bmp.is_top_down != 0 ||
        bmp.pixel_data != NULL) {
        return 0;
    }

    bmp_free(&bmp);
    return 1;
}

static int test_bmp_load_null_args(void) {
    BmpImage bmp;

    bmp_init(&bmp);

    if (bmp_load(NULL, &bmp) != BMP_ERR_NULL_ARG) {
        return 0;
    }

    if (bmp_load("dummy.bmp", NULL) != BMP_ERR_NULL_ARG) {
        return 0;
    }

    bmp_free(&bmp);
    return 1;
}

static int test_bmp_load_open_failed(void) {
    BmpImage bmp;

    bmp_init(&bmp);

    if (bmp_load("definitely_missing_file_12345.bmp", &bmp) != BMP_ERR_OPEN_FAILED) {
        bmp_free(&bmp);
        return 0;
    }

    bmp_free(&bmp);
    return 1;
}

static int test_bmp_load_valid_bottom_up_2x2(void) {
    char filename[128];
    BmpImage bmp;
    unsigned char visual_pixels[12];
    int ok;

    /*
     * Visual top row:
     *   red, green
     * Visual bottom row:
     *   blue, white
     *
     * Stored as BGR triples.
     */
    visual_pixels[0] = 0U;   visual_pixels[1] = 0U;   visual_pixels[2] = 255U;
    visual_pixels[3] = 0U;   visual_pixels[4] = 255U; visual_pixels[5] = 0U;
    visual_pixels[6] = 255U; visual_pixels[7] = 0U;   visual_pixels[8] = 0U;
    visual_pixels[9] = 255U; visual_pixels[10] = 255U; visual_pixels[11] = 255U;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!write_bmp24_file(filename, 2U, 2U, 0, visual_pixels)) {
        return 0;
    }

    bmp_init(&bmp);
    ok = (bmp_load(filename, &bmp) == BMP_OK);

    if (ok) {
        ok = ok &&
            bmp.width == 2 &&
            bmp.height == 2 &&
            bmp.bits_per_pixel == 24U &&
            bmp.compression == 0U &&
            bmp.row_stride == 8U &&
            bmp.pixel_offset == 54U &&
            bmp.pixel_data != NULL &&
            pixels_equal(bmp.pixel_data, visual_pixels, 12U);
    }

    bmp_free(&bmp);
    remove(filename);
    return ok;
}

static int test_bmp_load_valid_top_down_1x2(void) {
    char filename[128];
    BmpImage bmp;
    unsigned char visual_pixels[6];
    int ok;

    /*
     * Top row: black
     * Bottom row: yellow
     */
    visual_pixels[0] = 0U;   visual_pixels[1] = 0U;   visual_pixels[2] = 0U;
    visual_pixels[3] = 0U;   visual_pixels[4] = 255U; visual_pixels[5] = 255U;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!write_bmp24_file(filename, 1U, 2U, 1, visual_pixels)) {
        return 0;
    }

    bmp_init(&bmp);
    ok = (bmp_load(filename, &bmp) == BMP_OK);

    if (ok) {
        ok = ok &&
            bmp.width == 1 &&
            bmp.height == 2 &&
            bmp.row_stride == 4U &&
            bmp.pixel_data != NULL &&
            pixels_equal(bmp.pixel_data, visual_pixels, 6U);
    }

    bmp_free(&bmp);
    remove(filename);
    return ok;
}

static int test_bmp_load_row_padding_width_1(void) {
    char filename[128];
    BmpImage bmp;
    unsigned char visual_pixels[6];
    int ok;

    /*
     * Width = 1 -> row bytes = 3, stride = 4
     * This verifies padding is ignored and pixel bytes are correct.
     */
    visual_pixels[0] = 10U; visual_pixels[1] = 20U; visual_pixels[2] = 30U;
    visual_pixels[3] = 40U; visual_pixels[4] = 50U; visual_pixels[5] = 60U;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!write_bmp24_file(filename, 1U, 2U, 0, visual_pixels)) {
        return 0;
    }

    bmp_init(&bmp);
    ok = (bmp_load(filename, &bmp) == BMP_OK);

    if (ok) {
        ok = ok &&
            bmp.row_stride == 4U &&
            bmp.pixel_data != NULL &&
            pixels_equal(bmp.pixel_data, visual_pixels, 6U);
    }

    bmp_free(&bmp);
    remove(filename);
    return ok;
}

static int test_bmp_load_bad_signature(void) {
    char filename[128];
    BmpImage bmp;
    char bad_sig[2];
    int ok;

    bad_sig[0] = 'Z';
    bad_sig[1] = 'Z';

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!write_bmp_with_custom_header(filename, bad_sig, 1U, 1, 1U, 24U, 0U)) {
        return 0;
    }

    bmp_init(&bmp);
    ok = (bmp_load(filename, &bmp) == BMP_ERR_BAD_SIGNATURE);

    bmp_free(&bmp);
    remove(filename);
    return ok;
}

static int test_bmp_load_unsupported_bits(void) {
    char filename[128];
    BmpImage bmp;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!write_bmp_with_custom_header(filename, "BM", 1U, 1, 1U, 8U, 0U)) {
        return 0;
    }

    bmp_init(&bmp);
    ok = (bmp_load(filename, &bmp) == BMP_ERR_UNSUPPORTED_BITS);

    bmp_free(&bmp);
    remove(filename);
    return ok;
}

static int test_bmp_load_unsupported_compression(void) {
    char filename[128];
    BmpImage bmp;
    int ok;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!write_bmp_with_custom_header(filename, "BM", 1U, 1, 1U, 24U, 1U)) {
        return 0;
    }

    bmp_init(&bmp);
    ok = (bmp_load(filename, &bmp) == BMP_ERR_UNSUPPORTED_COMPRESSION);

    bmp_free(&bmp);
    remove(filename);
    return ok;
}

static int test_bmp_load_invalid_dimensions(void) {
    char filename[128];
    FILE *fp;
    unsigned int pixel_offset;
    int ok;
    BmpImage bmp;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        return 0;
    }

    pixel_offset = BMP_TEST_FILE_HEADER_SIZE + BMP_TEST_INFO_HEADER_SIZE;

    if (!io_write_bytes(fp, "BM", 2U) ||
        !io_write_u32_le(fp, (unsigned long)pixel_offset) ||
        !io_write_u16_le(fp, 0U) ||
        !io_write_u16_le(fp, 0U) ||
        !io_write_u32_le(fp, (unsigned long)pixel_offset) ||
        !io_write_u32_le(fp, (unsigned long)BMP_TEST_INFO_HEADER_SIZE) ||
        !io_write_s32_le(fp, 0) ||
        !io_write_s32_le(fp, 1) ||
        !io_write_u16_le(fp, 1U) ||
        !io_write_u16_le(fp, 24U) ||
        !io_write_u32_le(fp, 0UL) ||
        !io_write_u32_le(fp, 0UL) ||
        !io_write_s32_le(fp, 0) ||
        !io_write_s32_le(fp, 0) ||
        !io_write_u32_le(fp, 0UL) ||
        !io_write_u32_le(fp, 0UL)) {
        fclose(fp);
        remove(filename);
        return 0;
    }

    fclose(fp);

    bmp_init(&bmp);
    ok = (bmp_load(filename, &bmp) == BMP_ERR_INVALID_DIMENSIONS);

    bmp_free(&bmp);
    remove(filename);
    return ok;
}

static int test_bmp_read_row_top_down(void) {
    char filename[128];
    BmpImage bmp;
    FILE *fp;
    unsigned char visual_pixels[12];
    unsigned char row_buffer[6];
    int ok;

    /*
     * Top row:   cyan, magenta
     * Bottom row: black, white
     */
    visual_pixels[0] = 255U; visual_pixels[1] = 255U; visual_pixels[2] = 0U;
    visual_pixels[3] = 255U; visual_pixels[4] = 0U;   visual_pixels[5] = 255U;
    visual_pixels[6] = 0U;   visual_pixels[7] = 0U;   visual_pixels[8] = 0U;
    visual_pixels[9] = 255U; visual_pixels[10] = 255U; visual_pixels[11] = 255U;

    if (!make_temp_name(filename, sizeof(filename))) {
        return 0;
    }

    if (!write_bmp24_file(filename, 2U, 2U, 1, visual_pixels)) {
        return 0;
    }

    bmp_init(&bmp);
    ok = (bmp_load(filename, &bmp) == BMP_OK);
    if (!ok) {
        bmp_free(&bmp);
        remove(filename);
        return 0;
    }

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        bmp_free(&bmp);
        remove(filename);
        return 0;
    }

    ok = (bmp_read_row(fp, &bmp, 1U, row_buffer) == BMP_OK) &&
         pixels_equal(row_buffer, visual_pixels + 6U, 6U);

    fclose(fp);
    bmp_free(&bmp);
    remove(filename);
    return ok;
}

static int test_bmp_error_string(void) {
    return strcmp(bmp_error_string(BMP_OK), "success") == 0 &&
           strcmp(bmp_error_string(BMP_ERR_BAD_SIGNATURE), "invalid BMP signature") == 0;
}

int main(void) {
    test_report("bmp_init and bmp_free", test_bmp_init_and_free());
    test_report("bmp_load null args", test_bmp_load_null_args());
    test_report("bmp_load open failed", test_bmp_load_open_failed());
    test_report("bmp_load valid bottom-up 2x2", test_bmp_load_valid_bottom_up_2x2());
    test_report("bmp_load valid top-down 1x2", test_bmp_load_valid_top_down_1x2());
    test_report("bmp_load row padding width 1", test_bmp_load_row_padding_width_1());
    test_report("bmp_load bad signature", test_bmp_load_bad_signature());
    test_report("bmp_load unsupported bits", test_bmp_load_unsupported_bits());
    test_report("bmp_load unsupported compression", test_bmp_load_unsupported_compression());
    test_report("bmp_load invalid dimensions", test_bmp_load_invalid_dimensions());
    test_report("bmp_read_row top-down", test_bmp_read_row_top_down());
    test_report("bmp_error_string", test_bmp_error_string());

    test_summary();
    return test_failed_count() == 0 ? 0 : 1;
}
