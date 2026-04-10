#ifndef DECOMPRESS_H
#define DECOMPRESS_H

#include "components/render_compress.h"

char *decompress_frame(RenderCompressContext *ctx, const CompressedFrame *in);

char *decompress_frame_rle(const CompressedFrame *in);

char *decompress_frame_none(const CompressedFrame *in);

#endif