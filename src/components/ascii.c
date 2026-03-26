#include "components/ascii.h"

#include <stdlib.h>
#include <string.h>

#define ASCII_HIGHLIGHT_BONUS 32U

static void ascii_reset(AsciiFrame * frame);
static int ascii_palette_length(const char * palette);
static int ascii_validate_image(const BmpImage * bmp);
static unsigned int ascii_apply_highlight(unsigned int gray, int highlight);

void ascii_init(AsciiFrame * frame){
    if(frame == NULL){
        return;
    }

    ascii_reset(frame);
}

void ascii_free(AsciiFrame * frame){
    unsigned int i;

    if(frame == NULL){
        return;
    }

    if(frame->lines != NULL){
        for(i = 0U; i < frame->line_count; i++){
            free(frame->lines[i]);
        }
    }

    free(frame->lines);
    ascii_reset(frame);
}

unsigned int ascii_grayscale_from_bgr(
    unsigned char b,
    unsigned char g,
    unsigned char r
){
    /**
     * Integer approximation of:
     *  gray = 0.299*r + 0.587*g + 0.114*b
     */
    return (299U *(unsigned int)r +
           587U * (unsigned int)g + 
           114U * (unsigned int)b) / 1000U;
}

char ascii_map_gray_to_char(unsigned int gray, const char * palette){
    int palette_len;
    unsigned int index;

    if(palette == NULL || palette[0] == '\0'){
        return '?';
    }

    if(gray > 255U){
        gray = 255U;
    }

    palette_len = ascii_palette_length(palette);
    if(palette_len <= 0){
        return '?';
    }

    if(palette_len == 1){
        return palette[0];
    }

    index = (gray * (unsigned int)(palette_len - 1)) / 255U;
    return palette[index];
}

AsciiError ascii_render_image(
    const BmpImage * bmp,
    const char * palette,
    AsciiFrame * frame
){
    return ascii_render_image_with_highlight(bmp, palette, 0, frame);
}

AsciiError ascii_render_image_with_highlight(
    const BmpImage * bmp,
    const char * palette,
    int highlight,
    AsciiFrame * frame
){
    unsigned int x, y, width, height, index, gray;
    int palette_len;

    if(bmp == NULL || palette == NULL || frame == NULL){
        return ASCII_ERR_NULL_ARG;
    }

    ascii_free(frame);

    if(!ascii_validate_image(bmp)){
        return ASCII_ERR_INVALID_IMAGE;
    }

    palette_len = ascii_palette_length(palette);
    if(palette_len <= 0){
        return ASCII_ERR_INVALID_PALETTE;
    }

    width = (unsigned int) bmp->width;
    height = (unsigned int) bmp->height;

    frame->lines = (char **) malloc(sizeof(char *) * height);
    if(frame->lines == NULL){
        ascii_free(frame);
        return ASCII_ERR_MEMORY;
    }

    frame->line_count = height;
    frame->width = width;
    frame->height = height;

    for(y = 0U; y < height; y++){
        frame->lines[y] = (char *) malloc(sizeof(char) * (width + 1U));
        if(frame->lines[y] == NULL){
            ascii_free(frame);
            return ASCII_ERR_MEMORY;
        }

        for(x = 0U; x < width; x++){
            index = (y * width + x) * 3U;

            gray = ascii_grayscale_from_bgr(
                bmp->pixel_data[index],
                bmp->pixel_data[index + 1U],
                bmp->pixel_data[index + 2U]
            );

            gray = ascii_apply_highlight(gray, highlight);

            frame->lines[y][x] = ascii_map_gray_to_char(gray, palette);
        }

        frame->lines[y][width] = '\0';
    }

    return ASCII_OK;
}

const char * ascii_error_string(AsciiError err){
    switch(err){
        case ASCII_OK:
            return "[ASCII] success";
        case ASCII_ERR_NULL_ARG:
            return "[ASCII] null argument";
        case ASCII_ERR_INVALID_IMAGE:
            return "[ASCII] invalid BMP image";
        case ASCII_ERR_INVALID_PALETTE:
            return "[ASCII] invalid ASCII palette";
        case ASCII_ERR_MEMORY:
            return "[ASCII] memory allocation failed";
        default:
            break;
    }

    return "[ASCII] unknown ASCII error";
}

static void ascii_reset(AsciiFrame * frame){
    frame->lines = NULL;
    frame->line_count = 0U;
    frame->width = 0U;
    frame->height = 0U;
}

static int ascii_palette_length(const char * palette){
    if(palette == NULL){
        return 0;
    }

    return (int)strlen(palette);
}

static int ascii_validate_image(const BmpImage * bmp){
    if(bmp == NULL){
        return 0;
    }

    if(bmp->width <= 0 || bmp->height <= 0){
        return 0;
    }

    if(bmp->pixel_data == NULL){
        return 0;
    }

    if(bmp->bits_per_pixel != 24U){
        return 0;
    }

    return 1;
}

static unsigned int ascii_apply_highlight(unsigned int gray, int highlight){
    if(!highlight){
        return gray;
    }

    gray += ASCII_HIGHLIGHT_BONUS;

    if(gray > 255U){
        gray = 255U;
    }

    return gray;
}
