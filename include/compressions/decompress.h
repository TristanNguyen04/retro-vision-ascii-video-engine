#ifndef DECOMPRESS_H
#define DECOMPRESS_H

char *decompress_frame(const RenderCompressContext *ctx, const CompressedFrame *in);

char *decompress_frame_rle(const CompressedFrame *in);

char *decompress_frame_none(const CompressedFrame *in);

#endif