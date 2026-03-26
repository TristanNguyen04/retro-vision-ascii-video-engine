#include "components/render.h"
#include "components/ascii.h"
#include "../tests_helper.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void fill_valid_ascii_frame_1x1(AsciiFrame *frame, const char *line0);
static void fill_valid_ascii_frame_2x2(AsciiFrame *frame, const char *line0, const char *line1);

static int test_render_validate_frame_null_arg(void);
static int test_render_validate_frame_valid_1x1(void);
static int test_render_validate_frame_null_lines(void);
static int test_render_validate_frame_zero_width(void);
static int test_render_validate_frame_zero_height(void);
static int test_render_validate_frame_line_count_mismatch(void);
static int test_render_validate_frame_null_line(void);
static int test_render_validate_frame_wrong_line_length(void);

static int test_render_write_frame_null_arg(void);
static int test_render_write_frame_invalid_frame(void);
static int test_render_write_frame_valid_1x1(void);
static int test_render_write_frame_valid_2x2(void);
static int test_render_write_frame_highlight_one(void);

static int test_render_error_string(void);

static void fill_valid_ascii_frame_1x1(AsciiFrame *frame, const char *line0) {
    ascii_init(frame);

    frame->lines = (char **)malloc(sizeof(char *));
    if (frame->lines == NULL) {
        return;
    }

    frame->lines[0] = (char *)malloc(2U * sizeof(char));
    if (frame->lines[0] == NULL) {
        free(frame->lines);
        frame->lines = NULL;
        return;
    }

    strcpy(frame->lines[0], line0);
    frame->line_count = 1U;
    frame->width = 1U;
    frame->height = 1U;
}

static void fill_valid_ascii_frame_2x2(AsciiFrame *frame, const char *line0, const char *line1) {
    ascii_init(frame);

    frame->lines = (char **)malloc(2U * sizeof(char *));
    if (frame->lines == NULL) {
        return;
    }

    frame->lines[0] = (char *)malloc(3U * sizeof(char));
    frame->lines[1] = (char *)malloc(3U * sizeof(char));

    if (frame->lines[0] == NULL || frame->lines[1] == NULL) {
        free(frame->lines[0]);
        free(frame->lines[1]);
        free(frame->lines);
        frame->lines = NULL;
        return;
    }

    strcpy(frame->lines[0], line0);
    strcpy(frame->lines[1], line1);

    frame->line_count = 2U;
    frame->width = 2U;
    frame->height = 2U;
}

static int test_render_validate_frame_null_arg(void) {
    return render_validate_frame(NULL) == RENDER_ERR_NULL_ARG;
}

static int test_render_validate_frame_valid_1x1(void) {
    AsciiFrame frame;
    int ok;

    fill_valid_ascii_frame_1x1(&frame, "@");
    if (frame.lines == NULL) {
        return 0;
    }

    ok = (render_validate_frame(&frame) == RENDER_OK);

    ascii_free(&frame);
    return ok;
}

static int test_render_validate_frame_null_lines(void) {
    AsciiFrame frame;

    ascii_init(&frame);
    frame.lines = NULL;
    frame.line_count = 1U;
    frame.width = 1U;
    frame.height = 1U;

    return render_validate_frame(&frame) == RENDER_ERR_INVALID_FRAME;
}

static int test_render_validate_frame_zero_width(void) {
    AsciiFrame frame;

    fill_valid_ascii_frame_1x1(&frame, "@");
    if (frame.lines == NULL) {
        return 0;
    }

    frame.width = 0U;

    if (render_validate_frame(&frame) != RENDER_ERR_INVALID_FRAME) {
        ascii_free(&frame);
        return 0;
    }

    ascii_free(&frame);
    return 1;
}

static int test_render_validate_frame_zero_height(void) {
    AsciiFrame frame;

    fill_valid_ascii_frame_1x1(&frame, "@");
    if (frame.lines == NULL) {
        return 0;
    }

    frame.height = 0U;

    if (render_validate_frame(&frame) != RENDER_ERR_INVALID_FRAME) {
        ascii_free(&frame);
        return 0;
    }

    ascii_free(&frame);
    return 1;
}

static int test_render_validate_frame_line_count_mismatch(void) {
    AsciiFrame frame;

    fill_valid_ascii_frame_1x1(&frame, "@");
    if (frame.lines == NULL) {
        return 0;
    }

    frame.line_count = 2U;

    if (render_validate_frame(&frame) != RENDER_ERR_INVALID_FRAME) {
        ascii_free(&frame);
        return 0;
    }

    ascii_free(&frame);
    return 1;
}

static int test_render_validate_frame_null_line(void) {
    AsciiFrame frame;

    fill_valid_ascii_frame_1x1(&frame, "@");
    if (frame.lines == NULL) {
        return 0;
    }

    free(frame.lines[0]);
    frame.lines[0] = NULL;

    if (render_validate_frame(&frame) != RENDER_ERR_INVALID_FRAME) {
        ascii_free(&frame);
        return 0;
    }

    ascii_free(&frame);
    return 1;
}

static int test_render_validate_frame_wrong_line_length(void) {
    AsciiFrame frame;

    fill_valid_ascii_frame_1x1(&frame, "@");
    if (frame.lines == NULL) {
        return 0;
    }

    free(frame.lines[0]);
    frame.lines[0] = (char *)malloc(3U * sizeof(char));
    if (frame.lines[0] == NULL) {
        ascii_free(&frame);
        return 0;
    }

    strcpy(frame.lines[0], "@@");

    if (render_validate_frame(&frame) != RENDER_ERR_INVALID_FRAME) {
        ascii_free(&frame);
        return 0;
    }

    ascii_free(&frame);
    return 1;
}

static int test_render_write_frame_null_arg(void) {
    AsciiFrame frame;
    FILE *fp;
    int ok;

    fill_valid_ascii_frame_1x1(&frame, "@");
    if (frame.lines == NULL) {
        return 0;
    }

    fp = tmpfile();
    if (fp == NULL) {
        ascii_free(&frame);
        return 0;
    }

    ok = render_write_frame(NULL, 1U, 0.0, 0, &frame) == RENDER_ERR_NULL_ARG &&
         render_write_frame(fp, 1U, 0.0, 0, NULL) == RENDER_ERR_NULL_ARG;

    fclose(fp);
    ascii_free(&frame);
    return ok;
}

static int test_render_write_frame_invalid_frame(void) {
    AsciiFrame frame;
    FILE *fp;
    int ok;

    ascii_init(&frame);
    frame.lines = NULL;
    frame.line_count = 1U;
    frame.width = 1U;
    frame.height = 1U;

    fp = tmpfile();
    if (fp == NULL) {
        return 0;
    }

    ok = (render_write_frame(fp, 1U, 0.0, 0, &frame) == RENDER_ERR_INVALID_FRAME);

    fclose(fp);
    return ok;
}

static int test_render_write_frame_valid_1x1(void) {
    AsciiFrame frame;
    FILE *fp;
    int ok;

    fill_valid_ascii_frame_1x1(&frame, "@");
    if (frame.lines == NULL) {
        return 0;
    }

    fp = tmpfile();
    if (fp == NULL) {
        ascii_free(&frame);
        return 0;
    }

    ok = (render_write_frame(fp, 1U, 0.0, 0, &frame) == RENDER_OK);

    if (ok) {
        ok = ok &&
            file_contains_text(fp, "FRAME 1\n") &&
            file_contains_text(fp, "TIME 0.000\n") &&
            file_contains_text(fp, "HIGHLIGHT 0\n") &&
            file_contains_text(fp, "WIDTH 1\n") &&
            file_contains_text(fp, "HEIGHT 1\n") &&
            file_contains_text(fp, "@\n");
    }

    fclose(fp);
    ascii_free(&frame);
    return ok;
}

static int test_render_write_frame_valid_2x2(void) {
    AsciiFrame frame;
    FILE *fp;
    int ok;

    fill_valid_ascii_frame_2x2(&frame, "@.", ".@");
    if (frame.lines == NULL) {
        return 0;
    }

    fp = tmpfile();
    if (fp == NULL) {
        ascii_free(&frame);
        return 0;
    }

    ok = (render_write_frame(fp, 42U, 1.4, 0, &frame) == RENDER_OK);

    if (ok) {
        ok = ok &&
            file_contains_text(fp, "FRAME 42\n") &&
            file_contains_text(fp, "TIME 1.400\n") &&
            file_contains_text(fp, "HIGHLIGHT 0\n") &&
            file_contains_text(fp, "WIDTH 2\n") &&
            file_contains_text(fp, "HEIGHT 2\n") &&
            file_contains_text(fp, "@.\n") &&
            file_contains_text(fp, ".@\n");
    }

    fclose(fp);
    ascii_free(&frame);
    return ok;
}

static int test_render_write_frame_highlight_one(void) {
    AsciiFrame frame;
    FILE *fp;
    int ok;

    fill_valid_ascii_frame_1x1(&frame, "#");
    if (frame.lines == NULL) {
        return 0;
    }

    fp = tmpfile();
    if (fp == NULL) {
        ascii_free(&frame);
        return 0;
    }

    ok = (render_write_frame(fp, 7U, 0.233, 1, &frame) == RENDER_OK);

    if (ok) {
        ok = ok &&
            file_contains_text(fp, "FRAME 7\n") &&
            file_contains_text(fp, "TIME 0.233\n") &&
            file_contains_text(fp, "HIGHLIGHT 1\n") &&
            file_contains_text(fp, "#\n");
    }

    fclose(fp);
    ascii_free(&frame);
    return ok;
}

static int test_render_error_string(void) {
    return strcmp(render_error_string(RENDER_OK), "[RENDER] success") == 0 &&
           strcmp(render_error_string(RENDER_ERR_INVALID_FRAME),
                  "[RENDER] invalid ASCII frame") == 0 &&
           strcmp(render_error_string(RENDER_ERR_WRITE_FAILED),
                  "[RENDER] failed to write render output") == 0;
}

int main(void) {
    test_report("render_validate_frame null arg",
                test_render_validate_frame_null_arg());
    test_report("render_validate_frame valid 1x1",
                test_render_validate_frame_valid_1x1());
    test_report("render_validate_frame null lines",
                test_render_validate_frame_null_lines());
    test_report("render_validate_frame zero width",
                test_render_validate_frame_zero_width());
    test_report("render_validate_frame zero height",
                test_render_validate_frame_zero_height());
    test_report("render_validate_frame line count mismatch",
                test_render_validate_frame_line_count_mismatch());
    test_report("render_validate_frame null line",
                test_render_validate_frame_null_line());
    test_report("render_validate_frame wrong line length",
                test_render_validate_frame_wrong_line_length());

    test_report("render_write_frame null arg",
                test_render_write_frame_null_arg());
    test_report("render_write_frame invalid frame",
                test_render_write_frame_invalid_frame());
    test_report("render_write_frame valid 1x1",
                test_render_write_frame_valid_1x1());
    test_report("render_write_frame valid 2x2",
                test_render_write_frame_valid_2x2());
    test_report("render_write_frame highlight one",
                test_render_write_frame_highlight_one());

    test_report("render_error_string",
                test_render_error_string());

    test_summary();
    return test_failed_count() == 0 ? 0 : 1;
}
