//
// Created by shenjun on 2019-08-12.
//
#ifdef __cplusplus
extern "C" {
#endif
#include <ffmpeg/ffmpeg.h>
#include <ffmpeg/localutils/filterutils.h>
#include <include/CommonTools.h>
#include <stdint.h>
#ifdef __cplusplus
}
#endif

#ifndef AMINO_ANDROID_FF_FRAME_DECODER_H
#define AMINO_ANDROID_FF_FRAME_DECODER_H

#endif //AMINO_ANDROID_FF_FRAME_DECODER_H


typedef struct FFVideoFrameInfo {

    AVFormatContext *fmt_ctx;
    int video_stream_idx;
    AVCodecContext *video_dec_ctx;
    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx;
    AVFilterGraph *filter_graph;
    struct SwsContext *img_convert_ctx;

    AVRational *video_time_base;

    AVFrame *current_frame;
    AVFrame *temp_frame;
    AVPacket *pkt;

    int64_t current_timestamp;
    int64_t current_frame_pts;
    int64_t next_key_frame_pts;

    int width;
    int height;
    int64_t duration;

} FFVideoFrameInfo;

FFVideoFrameInfo *openVideoFile(char *filePath, int screenWidth, int screenHeight);

int getVideoFrameAt(FFVideoFrameInfo *info, int64_t frameTimeMs, int64_t trimStartTimeMs, int64_t trimEndTimeMs, int screenWidth, int screenHeight, unsigned char * pixels);

void cleanUpVideoResource(FFVideoFrameInfo *info);