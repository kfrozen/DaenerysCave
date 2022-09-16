//
// Created by shenjun on 2019-10-08.
//

#ifndef AMINO_ANDROID_WEBP_H
#define AMINO_ANDROID_WEBP_H

#endif //AMINO_ANDROID_WEBP_H

#include <stdint.h>
#include "dracarys/decoders/webp/libwebp/webp/decode.h"
#include "dracarys/decoders/webp/libwebp/webp/demux.h"
#include <include/CommonTools.h>


typedef struct WebpInfo {
    WebPDemuxer *demuxer;
    WebPData *data;
    uint32_t width;
    uint32_t height;
    uint32_t loopCount;
    uint32_t frameCount;
    uint32_t formatFlag;

    int *keyFrameArray;
    int *durationArray;
    int duration;

    WebPIterator* curIterator;
    WebPDecoderConfig *config;
    unsigned char *preservedBuffer;
    int lastFrameIndex;

} WebpInfo;

WebpInfo *openWebPFile(char *filePath, int screenWidth, int screenHeight);

int getWebPFrameAt(WebpInfo *info, int64_t frameTimeMs, unsigned char *pixels);

void cleanUpWebPResource(WebpInfo *info);