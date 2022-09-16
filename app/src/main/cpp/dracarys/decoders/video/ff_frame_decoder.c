//
// Created by shenjun on 2019-08-12.
//
#ifdef __cplusplus
extern "C" {
#endif
#include "ff_frame_decoder.h"
#include <include/libavutil/imgutils.h>
#include <libyuv/libyuv.h>
#ifdef __cplusplus
}
#endif

FFVideoFrameInfo *openVideoFile(char *filePath, int screenWidth, int screenHeight) {

    FFVideoFrameInfo *info = malloc(sizeof(FFVideoFrameInfo));
    memset(info, 0, sizeof(FFVideoFrameInfo));
    info->fmt_ctx = NULL;
    info->video_dec_ctx = NULL;
    info->buffersrc_ctx = NULL;
    info->buffersink_ctx = NULL;
    info->filter_graph = NULL;
    info->pkt = av_packet_alloc();
    info->current_frame = av_frame_alloc();
    info->temp_frame = av_frame_alloc();
    info->current_timestamp = -1;
    info->current_frame_pts = -1;
    info->next_key_frame_pts = -1;
    info->img_convert_ctx = NULL;

    int ret, rotate = 0;
    AVStream *video_stream = NULL;

    if ((ret = avformat_open_input(&info->fmt_ctx, filePath, NULL, NULL)) < 0) {
        LOGE("Cannot open input file, ret = %d", ret);
        goto end;
    }
    if ((ret = avformat_find_stream_info(info->fmt_ctx, NULL)) < 0) {
        LOGE("Cannot find stream info, ret = %d", ret);
        goto end;
    }
    if (open_codec_context(&info->video_stream_idx, &info->video_dec_ctx, info->fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
        video_stream = info->fmt_ctx->streams[info->video_stream_idx];
        info->video_time_base = &video_stream->time_base;
        info->duration = video_stream->duration;
        info->width = info->video_dec_ctx->width;
        info->height = info->video_dec_ctx->height;

        if (info->duration <= 0) {
            goto end;
        }
        if (video_stream->metadata != NULL && av_dict_count(video_stream->metadata) > 0) {
            AVDictionaryEntry *rotateEntry = av_dict_get(video_stream->metadata, "rotate", NULL, 0);
            if (rotateEntry != NULL) {
                rotate = atoi(rotateEntry->value);
            }
        }
        const char *filter_descr = NULL;
        size_t rotate_descr_length = 0;
        if (rotate > 0) { // need transpose filter
            switch (rotate) {
                case 90:
                    filter_descr = "transpose=clock";
                    info->width = info->video_dec_ctx->height;
                    info->height = info->video_dec_ctx->width;
                    break;
                case 270:
                    filter_descr = "transpose=cclock";
                    info->width = info->video_dec_ctx->height;
                    info->height = info->video_dec_ctx->width;
                    break;
                default:
                    break;
            }
            if (filter_descr) {
                rotate_descr_length = strlen(filter_descr);
            }
        }

        if (rotate_descr_length > 0) {
            ret = init_filters(filter_descr, info->fmt_ctx, info->video_dec_ctx, &info->buffersink_ctx,
                               &info->buffersrc_ctx, &info->filter_graph, info->video_stream_idx);
            if (ret < 0) {
                goto end;
            }
        }
        info->img_convert_ctx = sws_getCachedContext(info->img_convert_ctx, info->width,
                                                     info->height, info->video_dec_ctx->pix_fmt,
                                                     info->width, info->height, AV_PIX_FMT_RGBA,
                                                     SWS_FAST_BILINEAR, NULL, NULL, NULL);
    }

    if (!video_stream) {
        ret = -1;
        goto end;
    }
    if (!info->current_frame || !info->temp_frame) {
        ret = -1;
        goto end;
    }
    return info;

    end :
    if (info != NULL) {
        cleanUpVideoResource(info);
        free(info);
    }
    return NULL;
}

int64_t roundTimeToBase(FFVideoFrameInfo *info, int64_t timeMs) {
    return (((int64_t) timeMs) * info->video_time_base->den) / (1000 * info->video_time_base->num);
}

int copyYUVData(enum AVPixelFormat fmt, struct SwsContext * swsContext, AVFrame *frame, unsigned char *pixels) {
    if (fmt == AV_PIX_FMT_YUV420P) {
        I420ToABGR(frame->data[0], frame->linesize[0], frame->data[1], frame->linesize[1],
                   frame->data[2], frame->linesize[2], pixels, frame->width * 4, frame->width,
                   frame->height);
    } else {
        uint8_t *planes[4] = {pixels};
        int strides[4] = {frame->width * 4};
        sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, planes, strides);
    }
//    int offset = 0;
//    for (int i = 0; i < frame->height; i++) {
//        memcpy(pixels + offset, frame->data[0] + frame->linesize[0] * i, frame->width);
//        offset += frame->width;
//    }
//    for (int i = 0; i < frame->height / 2; i++) {
//        memcpy(pixels + offset, frame->data[1] + frame->linesize[1] * i, frame->width / 2);
//        offset += frame->width / 2;
//    }
//    for (int i = 0; i < frame->height / 2; i++) {
//        memcpy(pixels + offset, frame->data[2] + frame->linesize[2] * i, frame->width / 2);
//        offset += frame->width / 2;
//    }
}

int readNextFrameAfter(FFVideoFrameInfo *info, int64_t timeStamp, unsigned char *pixels) {
    int ret;
    int got_frame = 0;
    int read_ret;
    while ((read_ret = av_read_frame(info->fmt_ctx, info->pkt)) >= 0) {
        if (info->pkt->stream_index == info->video_stream_idx) {
            ret = avcodec_decode_video2(info->video_dec_ctx, info->current_frame, &got_frame, info->pkt);
            if (ret < 0) {
                av_frame_unref(info->current_frame);
                av_packet_unref(info->pkt);
                break;
            }
            if (got_frame && info->current_frame->pts + info->current_frame->pkt_duration >= timeStamp) {
                info->current_frame_pts = info->current_frame->pts;
                info->current_timestamp = timeStamp;
                if (info->buffersrc_ctx != NULL && info->buffersink_ctx != NULL) {
                    if (av_buffersrc_add_frame_flags(info->buffersrc_ctx, info->current_frame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
                        av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
                        av_frame_unref(info->current_frame);
                        av_packet_unref(info->pkt);
                        break;
                    }
                    ret = av_buffersink_get_frame(info->buffersink_ctx, info->temp_frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF || ret < 0) {
                        av_frame_unref(info->temp_frame);
                        av_frame_unref(info->current_frame);
                        av_packet_unref(info->pkt);
                        break;
                    }
                    copyYUVData(info->video_dec_ctx->pix_fmt, info->img_convert_ctx, info->temp_frame, pixels);
                    av_frame_unref(info->temp_frame);
                } else {
                    copyYUVData(info->video_dec_ctx->pix_fmt, info->img_convert_ctx, info->current_frame, pixels);
                }
                av_frame_unref(info->current_frame);
                return 1;
            }
        }
        av_packet_unref(info->pkt);
    }
    if (read_ret == AVERROR_EOF) {
        return 1;
    }
    return -1;
}

int64_t getKeyFramePts(FFVideoFrameInfo *info, int64_t timestamp, int flags) {
    av_seek_frame(info->fmt_ctx, info->video_stream_idx, timestamp, flags);
    int64_t pts = -1;
    int ret;
    while ((ret = av_read_frame(info->fmt_ctx, info->pkt)) >= 0) {
        if (info->pkt->stream_index == info->video_stream_idx && info->pkt->flags == 1) {
            pts = info->pkt->pts;
            av_packet_unref(info->pkt);
            break;
        }
        av_packet_unref(info->pkt);
    }
    if (flags == 0 && ret == AVERROR_EOF) {
        return info->duration;
    }
    return pts;
}

int getVideoFrameAt(FFVideoFrameInfo *info, int64_t frameTimeMs, int64_t trimStartTimeMs,
                    int64_t trimEndTimeMs, int screenWidth, int screenHeight, unsigned char *pixels) {
    int64_t trimStart = roundTimeToBase(info, trimStartTimeMs);
    int64_t trimEnd = roundTimeToBase(info, trimEndTimeMs);
    int64_t frameTime = roundTimeToBase(info, frameTimeMs);

    int64_t seekTimestamp = trimStart + (frameTime % (MIN(trimEnd, info->duration) - trimStart));

    if (seekTimestamp == info->current_timestamp) {
        return 1;
    }
    if (seekTimestamp < info->current_frame_pts || seekTimestamp > info->next_key_frame_pts) {
        avcodec_flush_buffers(info->video_dec_ctx);
        info->next_key_frame_pts = getKeyFramePts(info, seekTimestamp, 0);
        info->current_frame_pts = getKeyFramePts(info, seekTimestamp, AVSEEK_FLAG_BACKWARD);
        av_seek_frame(info->fmt_ctx, info->video_stream_idx, seekTimestamp, AVSEEK_FLAG_BACKWARD);
    }
    return readNextFrameAfter(info, seekTimestamp, pixels);
}


void cleanUpVideoResource(FFVideoFrameInfo *info) {
    if (info->filter_graph) {
        avfilter_graph_free(&info->filter_graph);
    }
    if (info->current_frame) {
        av_frame_free(&info->current_frame);
    }
    if (info->temp_frame) {
        av_frame_free(&info->temp_frame);
    }
    if (info->video_dec_ctx) {
        avcodec_free_context(&info->video_dec_ctx);
    }
    if (info->fmt_ctx) {
        avformat_close_input(&info->fmt_ctx);
    }
    if (info->pkt) {
        av_packet_free(&info->pkt);
    }
    if (info->img_convert_ctx) {
        sws_freeContext(info->img_convert_ctx);
        info->img_convert_ctx = NULL;
    }
}