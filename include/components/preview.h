#ifndef PREVIEW_H
#define PREVIEW_H

#include "components/render_compress.h"
#include <stdio.h>

int parse_header(FILE *fp, RenderCompressContext *ctx);
int parse_fsm(FILE *fp, RenderCompressContext *ctx);
int read_frame(FILE *fp, CompressedFrame *frame);

#endif
