//
// Created by shenjun on 2019-10-08.
//

#include "dracarys/decoders/webp/libwebp/utils/utils.h"
#include "dracarys/decoders/webp/libwebp/enc/histogram_enc.h"
#include "dracarys/decoders/webp/libwebp/webp/decode.h"
#include "webp.h"


WebPData *initWebPDataFromFile(char *filePath) {
    FILE *f = fopen(filePath, "rb");
    uint8_t riff_header[RIFF_HEADER_SIZE];
    if (fread(riff_header, 1, RIFF_HEADER_SIZE, f) != RIFF_HEADER_SIZE) {
        LOGE("WebP header load failed");
        return NULL;
    }
    WebPData *data = malloc(sizeof(WebPData));
    data->size = CHUNK_HEADER_SIZE + GetLE32(riff_header + TAG_SIZE);
    data->bytes = malloc(data->size);
    memcpy((void *) data->bytes, riff_header, RIFF_HEADER_SIZE);

    // Read rest of the bytes.
    void *remaining_bytes = (void *) (data->bytes + RIFF_HEADER_SIZE);
    size_t remaining_size = data->size - RIFF_HEADER_SIZE;
    if (fread(remaining_bytes, 1, remaining_size, f) != remaining_size) {
        LOGE("WebP full load failed");
        if (data->bytes != NULL) {
            free((uint8_t *) data->bytes);
            data->bytes = NULL;
        }
        data->bytes = NULL;
        free(data);
        return NULL;
    }
    return data;
}

// Returns 1 if the frame covers full canvas.
static int isFullFrame(const WebPIterator frame, int canvasWidth, int canvasHeight) {
    return (frame.width == canvasWidth && frame.height == canvasHeight);
}

void initKeyFrameArray(WebpInfo *info) {
    const int canvasWidth = info->width;
    const int canvasHeight = info->height;
    WebPIterator prev;
    WebPIterator curr;
    // Note: WebPDemuxGetFrame() uses base-1 counting.
    int ok = WebPDemuxGetFrame(info->demuxer, 1, &curr);

    int totalDuration = 0;
    info->keyFrameArray[0] = 1;// 0th frame is always a key frame.
    info->durationArray[0] = curr.duration;
    totalDuration += curr.duration;
    for (int i = 1; i < info->frameCount; i++) {
        prev = curr;
        ok = WebPDemuxGetFrame(info->demuxer, i + 1, &curr);  // Get ith frame.
        if ((!curr.has_alpha || curr.blend_method == WEBP_MUX_NO_BLEND) &&
            isFullFrame(curr, canvasWidth, canvasHeight)) {
            info->keyFrameArray[i] = 1;
        } else {
            info->keyFrameArray[i] = (prev.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND) &&
                             (isFullFrame(prev, canvasWidth, canvasHeight) || info->keyFrameArray[i - 1]);
        }
        info->durationArray[i] = curr.duration;
        totalDuration += curr.duration;
    }
    WebPDemuxReleaseIterator(&prev);
    WebPDemuxReleaseIterator(&curr);

    info->duration = totalDuration;
}


int decodeFrame(WebpInfo *info, WebPIterator* iterator, unsigned char *pixels) {
    info->config->output.u.RGBA.rgba = pixels;
//    LOGE("dddddddd bytes %c%c%c%c, size = %d", iterator->fragment.bytes[0], iterator->fragment.bytes[1], iterator->fragment.bytes[2], iterator->fragment.bytes[3], iterator->fragment.size);
    if (WebPDecode(iterator->fragment.bytes, iterator->fragment.size, info->config) != VP8_STATUS_OK) {
//        WebPFreeDecBuffer(&info->config->output);
        return 0;
    }
//    WebPFreeDecBuffer(&info->config->output);
    return 1;
}

WebpInfo *openWebPFile(char *filePath, int screenWidth, int screenHeight) {
    WebPData *data = initWebPDataFromFile(filePath);
    if (data == NULL) {
        return NULL;
    }
    WebpInfo *info = malloc(sizeof(WebpInfo));
    memset(info, 0, sizeof(WebpInfo));
    info->data = data;
    info->demuxer = WebPDemux(data);
    if (info->demuxer == NULL) {
        return NULL;
    }
    info->width = WebPDemuxGetI(info->demuxer, WEBP_FF_CANVAS_WIDTH);
    info->height = WebPDemuxGetI(info->demuxer, WEBP_FF_CANVAS_HEIGHT);
    info->loopCount = WebPDemuxGetI(info->demuxer, WEBP_FF_LOOP_COUNT);
    info->formatFlag = WebPDemuxGetI(info->demuxer, WEBP_FF_FORMAT_FLAGS);
    info->frameCount = WebPDemuxGetI(info->demuxer, WEBP_FF_FRAME_COUNT);

    info->curIterator = malloc(sizeof(WebPIterator));

    info->config = malloc(sizeof(WebPDecoderConfig));
    WebPInitDecoderConfig(info->config);
    info->config->output.is_external_memory = 1;
    info->config->output.width = info->width;
    info->config->output.height = info->height;
    info->config->output.colorspace = MODE_RGBA;
    info->config->output.u.RGBA.stride = info->width * 4;
    info->config->output.u.RGBA.size = info->width * info->height * 4;
    info->config->options.use_scaling = 1;
    info->config->options.scaled_width = info->width;
    info->config->options.scaled_height = info->height;

    info->lastFrameIndex = -1;
    info->preservedBuffer = malloc(info->width * info->height * 4);

    info->keyFrameArray = malloc(info->frameCount * sizeof(int));
    info->durationArray = malloc(info->frameCount * sizeof(int));
    initKeyFrameArray(info);
    return info;
}

int getWebPFrameAt(WebpInfo *info, int64_t frameTimeMs, unsigned char *pixels) {
    int realTimeMs = (int) (frameTimeMs % info->duration);
    int index = 0;
    for (int i = 0; i < info->frameCount; ++i) {
        realTimeMs -= info->durationArray[i];
        if (realTimeMs <= 0) {
            index = i;
            break;
        }
    }
    if (index == info->lastFrameIndex) {
        return 1;
    }
    //todo decode from last key frame
//    int startIndex = 0;
//    for (int i = index - 1; i >= 0; i--) {
//        if (info->keyFrameArray[i] == 1) {
//            startIndex = i;
//            break;
//        }
//    }

    int ok = WebPDemuxGetFrame(info->demuxer, index + 1, info->curIterator);
    if (ok <= 0) {
        return 0;
    }
    int res = decodeFrame(info, info->curIterator, pixels);
    if (res > 0) {
        info->lastFrameIndex = index;
    }
    return res;
}

void cleanUpWebPResource(WebpInfo *info) {
    if (info->data != NULL) {
        if (info->data->bytes != NULL) {
            free((uint8_t *) info->data->bytes);
            info->data->bytes = NULL;
        }
        free(info->data);
        info->data = NULL;
    }
    if (info->preservedBuffer != NULL) {
        free(info->preservedBuffer);
        info->preservedBuffer = NULL;
    }
    if (info->keyFrameArray != NULL) {
        free(info->keyFrameArray);
        info->keyFrameArray = NULL;
    }
    if (info->durationArray != NULL) {
        free(info->durationArray);
        info->durationArray = NULL;
    }
    if (info->curIterator != NULL) {
        free(info->curIterator);
        info->curIterator = NULL;
    }
    if (info->config != NULL) {
        free(info->config);
        info->config = NULL;
    }
    WebPDemuxDelete(info->demuxer);
    info->demuxer = NULL;
}