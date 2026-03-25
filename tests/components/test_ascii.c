#include "components/ascii.h"
#include "parsers/bmp.h"
#include "../tests_helper.h"

#include <string.h>
#include <stdlib.h>

static void fill_valid_bmp_1x1(BmpImage *bmp, unsigned char b, unsigned char g, unsigned char r);
static void fill_valid_bmp_2x2(BmpImage *bmp, const unsigned char *pixels);

static int test_ascii_init_and_free(void);
static int test_ascii_grayscale_from_bgr_black(void);
static int test_ascii_grayscale_from_bgr_white(void);
static int test_ascii_grayscale_from_bgr_red(void);
static int test_ascii_map_gray_to_char_null_palette(void);
static int test_ascii_map_gray_to_char_empty_palette(void);
static int test_ascii_map_gray_to_char_single_char(void);
static int test_ascii_map_gray_to_char_low_mid_high(void);
static int test_ascii_render_image_null_args(void);
static int test_ascii_render_image_invalid_image(void);
static int test_ascii_render_image_invalid_palette_null(void);
static int test_ascii_render_image_invalid_palette_empty(void);
static int test_ascii_render_image_1x1(void);
static int test_ascii_render_image_2x2(void);
static int test_ascii_render_image_with_highlight_changes_output(void);
static int test_ascii_error_string(void);

static void fill_valid_bmp_1x1(BmpImage *bmp, unsigned char b, unsigned char g, unsigned char r) {
    bmp_init(bmp);

    bmp->width = 1;
    bmp->height = 1;
    bmp->bits_per_pixel = 24U;
    bmp->pixel_data = (unsigned char *)malloc(3U);

    if (bmp->pixel_data != NULL) {
        bmp->pixel_data[0] = b;
        bmp->pixel_data[1] = g;
        bmp->pixel_data[2] = r;
    }
}

static void fill_valid_bmp_2x2(BmpImage *bmp, const unsigned char *pixels) {
    bmp_init(bmp);

    bmp->width = 2;
    bmp->height = 2;
    bmp->bits_per_pixel = 24U;
    bmp->pixel_data = (unsigned char *)malloc(12U);

    if (bmp->pixel_data != NULL && pixels != NULL) {
        memcpy(bmp->pixel_data, pixels, 12U);
    }
}

static int test_ascii_init_and_free(void) {
    AsciiFrame frame;

    frame.lines = (char **)1;
    frame.line_count = 99U;
    frame.width = 88U;
    frame.height = 77U;

    ascii_init(&frame);

    if (frame.lines != NULL ||
        frame.line_count != 0U ||
        frame.width != 0U ||
        frame.height != 0U) {
        return 0;
    }

    ascii_free(&frame);

    return frame.lines == NULL &&
           frame.line_count == 0U &&
           frame.width == 0U &&
           frame.height == 0U;
}

static int test_ascii_grayscale_from_bgr_black(void) {
    return ascii_grayscale_from_bgr(0U, 0U, 0U) == 0U;
}

static int test_ascii_grayscale_from_bgr_white(void) {
    return ascii_grayscale_from_bgr(255U, 255U, 255U) == 255U;
}

static int test_ascii_grayscale_from_bgr_red(void) {
    unsigned int gray;

    gray = ascii_grayscale_from_bgr(0U, 0U, 255U);

    return gray == 76U;
}

static int test_ascii_map_gray_to_char_null_palette(void) {
    return ascii_map_gray_to_char(100U, NULL) == '?';
}

static int test_ascii_map_gray_to_char_empty_palette(void) {
    return ascii_map_gray_to_char(100U, "") == '?';
}

static int test_ascii_map_gray_to_char_single_char(void) {
    return ascii_map_gray_to_char(100U, "@") == '@';
}

static int test_ascii_map_gray_to_char_low_mid_high(void) {
    const char *palette;

    palette = "@.-";

    return ascii_map_gray_to_char(0U, palette) == '@' &&
           ascii_map_gray_to_char(128U, palette) == '.' &&
           ascii_map_gray_to_char(255U, palette) == '-';
}

static int test_ascii_render_image_null_args(void) {
    BmpImage bmp;
    AsciiFrame frame;

    fill_valid_bmp_1x1(&bmp, 0U, 0U, 0U);
    ascii_init(&frame);

    if (bmp.pixel_data == NULL) {
        bmp_free(&bmp);
        return 0;
    }

    if (ascii_render_image(NULL, "@.", &frame) != ASCII_ERR_NULL_ARG) {
        bmp_free(&bmp);
        ascii_free(&frame);
        return 0;
    }

    if (ascii_render_image(&bmp, NULL, &frame) != ASCII_ERR_NULL_ARG) {
        bmp_free(&bmp);
        ascii_free(&frame);
        return 0;
    }

    if (ascii_render_image(&bmp, "@.", NULL) != ASCII_ERR_NULL_ARG) {
        bmp_free(&bmp);
        ascii_free(&frame);
        return 0;
    }

    bmp_free(&bmp);
    ascii_free(&frame);
    return 1;
}

static int test_ascii_render_image_invalid_image(void) {
    BmpImage bmp;
    AsciiFrame frame;

    bmp_init(&bmp);
    ascii_init(&frame);

    bmp.width = 0;
    bmp.height = 1;
    bmp.bits_per_pixel = 24U;
    bmp.pixel_data = (unsigned char *)1;

    return ascii_render_image(&bmp, "@.", &frame) == ASCII_ERR_INVALID_IMAGE;
}

static int test_ascii_render_image_invalid_palette_null(void) {
    BmpImage bmp;
    AsciiFrame frame;
    int ok;

    fill_valid_bmp_1x1(&bmp, 0U, 0U, 0U);
    ascii_init(&frame);

    if (bmp.pixel_data == NULL) {
        bmp_free(&bmp);
        return 0;
    }

    ok = (ascii_render_image(&bmp, NULL, &frame) == ASCII_ERR_NULL_ARG);

    bmp_free(&bmp);
    ascii_free(&frame);
    return ok;
}

static int test_ascii_render_image_invalid_palette_empty(void) {
    BmpImage bmp;
    AsciiFrame frame;
    int ok;

    fill_valid_bmp_1x1(&bmp, 0U, 0U, 0U);
    ascii_init(&frame);

    if (bmp.pixel_data == NULL) {
        bmp_free(&bmp);
        return 0;
    }

    ok = (ascii_render_image(&bmp, "", &frame) == ASCII_ERR_INVALID_PALETTE);

    bmp_free(&bmp);
    ascii_free(&frame);
    return ok;
}

static int test_ascii_render_image_1x1(void) {
    BmpImage bmp;
    AsciiFrame frame;
    int ok;

    /*
     * Black pixel -> grayscale 0 -> first palette char '@'
     */
    fill_valid_bmp_1x1(&bmp, 0U, 0U, 0U);
    ascii_init(&frame);

    if (bmp.pixel_data == NULL) {
        bmp_free(&bmp);
        return 0;
    }

    ok = (ascii_render_image(&bmp, "@.", &frame) == ASCII_OK);

    if (ok) {
        ok = ok &&
            frame.lines != NULL &&
            frame.line_count == 1U &&
            frame.width == 1U &&
            frame.height == 1U &&
            strcmp(frame.lines[0], "@") == 0;
    }

    bmp_free(&bmp);
    ascii_free(&frame);
    return ok;
}

static int test_ascii_render_image_2x2(void) {
    BmpImage bmp;
    AsciiFrame frame;
    unsigned char pixels[12];
    int ok;

    /*
     * Top row:
     *   black  -> '@'
     *   white  -> '.'
     * Bottom row:
     *   black  -> '@'
     *   white  -> '.'
     */
    pixels[0] = 0U;   pixels[1] = 0U;   pixels[2] = 0U;
    pixels[3] = 255U; pixels[4] = 255U; pixels[5] = 255U;
    pixels[6] = 0U;   pixels[7] = 0U;   pixels[8] = 0U;
    pixels[9] = 255U; pixels[10] = 255U; pixels[11] = 255U;

    fill_valid_bmp_2x2(&bmp, pixels);
    ascii_init(&frame);

    if (bmp.pixel_data == NULL) {
        bmp_free(&bmp);
        return 0;
    }

    ok = (ascii_render_image(&bmp, "@.", &frame) == ASCII_OK);

    if (ok) {
        ok = ok &&
            frame.lines != NULL &&
            frame.line_count == 2U &&
            frame.width == 2U &&
            frame.height == 2U &&
            strcmp(frame.lines[0], "@.") == 0 &&
            strcmp(frame.lines[1], "@.") == 0;
    }

    bmp_free(&bmp);
    ascii_free(&frame);
    return ok;
}

static int test_ascii_render_image_with_highlight_changes_output(void) {
    BmpImage bmp;
    AsciiFrame normal_frame;
    AsciiFrame highlight_frame;
    int ok;

    /*
     * Mid-gray pixel:
     * use a longer palette so highlight can visibly shift output
     */
    fill_valid_bmp_1x1(&bmp, 128U, 128U, 128U);
    ascii_init(&normal_frame);
    ascii_init(&highlight_frame);

    if (bmp.pixel_data == NULL) {
        bmp_free(&bmp);
        return 0;
    }

    ok = (ascii_render_image_with_highlight(&bmp, "@#*+=-:. ", 0, &normal_frame) == ASCII_OK) &&
         (ascii_render_image_with_highlight(&bmp, "@#*+=-:. ", 1, &highlight_frame) == ASCII_OK);

    if (ok) {
        ok = ok &&
            normal_frame.lines != NULL &&
            highlight_frame.lines != NULL &&
            normal_frame.lines[0][0] != '\0' &&
            highlight_frame.lines[0][0] != '\0' &&
            normal_frame.lines[0][0] != highlight_frame.lines[0][0];
    }

    bmp_free(&bmp);
    ascii_free(&normal_frame);
    ascii_free(&highlight_frame);
    return ok;
}

static int test_ascii_error_string(void) {
    return strcmp(ascii_error_string(ASCII_OK), "success") == 0 &&
           strcmp(ascii_error_string(ASCII_ERR_INVALID_PALETTE),
                  "invalid ASCII palette") == 0;
}

int main(void) {
    test_report("ascii_init and ascii_free", test_ascii_init_and_free());
    test_report("ascii_grayscale_from_bgr black", test_ascii_grayscale_from_bgr_black());
    test_report("ascii_grayscale_from_bgr white", test_ascii_grayscale_from_bgr_white());
    test_report("ascii_grayscale_from_bgr red", test_ascii_grayscale_from_bgr_red());
    test_report("ascii_map_gray_to_char null palette", test_ascii_map_gray_to_char_null_palette());
    test_report("ascii_map_gray_to_char empty palette", test_ascii_map_gray_to_char_empty_palette());
    test_report("ascii_map_gray_to_char single char", test_ascii_map_gray_to_char_single_char());
    test_report("ascii_map_gray_to_char low mid high", test_ascii_map_gray_to_char_low_mid_high());
    test_report("ascii_render_image null args", test_ascii_render_image_null_args());
    test_report("ascii_render_image invalid image", test_ascii_render_image_invalid_image());
    test_report("ascii_render_image invalid palette null", test_ascii_render_image_invalid_palette_null());
    test_report("ascii_render_image invalid palette empty", test_ascii_render_image_invalid_palette_empty());
    test_report("ascii_render_image 1x1", test_ascii_render_image_1x1());
    test_report("ascii_render_image 2x2", test_ascii_render_image_2x2());
    test_report("ascii_render_image_with_highlight changes output",
                test_ascii_render_image_with_highlight_changes_output());
    test_report("ascii_error_string", test_ascii_error_string());

    test_summary();
    return test_failed_count() == 0 ? 0 : 1;
}
