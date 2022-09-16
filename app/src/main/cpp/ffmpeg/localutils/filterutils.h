//
// Created by troy on 2019/5/29.
//

#ifndef AMINO_FILTERUTILS_H
#define AMINO_FILTERUTILS_H

#endif //AMINO_FILTERUTILS_H
#ifdef __cplusplus
extern "C" {
#endif
#include <jni.h>
#include <unistd.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#ifdef __cplusplus
}
#endif

int init_filters(const char *filters_descr, AVFormatContext *fmt_ctx, AVCodecContext *dec_ctx, AVFilterContext **buffersink_ctx,
                 AVFilterContext **buffersrc_ctx, AVFilterGraph **filter_graph, int video_stream_index);
