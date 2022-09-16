//
// Created by troy on 2019-07-22.
//

#ifndef LIBPNG_FOR_ANDROID_NVIMAGE_H
#define LIBPNG_FOR_ANDROID_NVIMAGE_H

#include <GLES2/gl2.h>

typedef uint8_t byte;

enum INPUT_IMAGE_TYPE {
    UNKNOWN = -1, PNG = 0, JPEG = 1, GIF = 2, VIDEO = 3, WEBP = 4,
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
    int64_t videoTrimEndMs;
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
} NVImageRawData;

void initInput(char *filePath, NVImageRawData *imageRawData, int screenWidth, int screenHeight);

void decodeRawImageData(NVImageRawData* data, float segmentTimeSec, int screenWidth, int screenHeight);

void releaseRawImageData(NVImageRawData* data);

#endif //LIBPNG_FOR_ANDROID_NVIMAGE_H




