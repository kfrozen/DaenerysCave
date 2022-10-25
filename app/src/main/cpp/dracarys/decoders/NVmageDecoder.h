//
// Created by troy on 2019-07-22.
//

#ifndef LIBPNG_FOR_ANDROID_NVIMAGE_H
#define LIBPNG_FOR_ANDROID_NVIMAGE_H

#include <GLES2/gl2.h>

typedef uint8_t byte;

enum INPUT_IMAGE_TYPE {
    GAP = -2, UNKNOWN = -1, PNG = 0, JPEG = 1, GIF = 2, VIDEO = 3, WEBP = 4, LIVE = 5,
};

typedef struct {
    int width;
    int height;
    int size;
    GLenum gl_color_format;
    void* data;
    int error;
} DecodedRawFrameData;

typedef struct {
    enum INPUT_IMAGE_TYPE type;
    int64_t videoTrimStartMs;
    int64_t videoTrimDurationMs;
    int screenWidth;
    int screenHeight;
} ExtraInfo;

typedef struct {
    // input
    char *filePath;
    void* meta;
//    void* bundle; //Currently only for gif decoding
    int metaSize; //Only for static pictures like png or jpeg
    ExtraInfo* extraInfo;
    // decoded data
    int width;
    int height;
    int size;
    GLenum gl_color_format;
    void* data;
    void* decodedStaticImageData; //Only for static pictures like png or jpeg
    int dataDecoded; //Only for static pictures like png or jpeg
    int error;
    void (*release)(void *self);
} NVImageRawData;

void initInput(char *filePath, NVImageRawData *imageRawData);

void decodeRawImageData(NVImageRawData* data, float segmentTimeSec, int screenWidth, int screenHeight);

void releaseRawImageData(void* self);

#endif //LIBPNG_FOR_ANDROID_NVIMAGE_H




