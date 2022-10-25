//
// Created by troy on 2019-07-22.
//
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <dracarys/decoders/png/image.h>
#include "NVmageDecoder.h"
#include "bilinear_resampling.h"
#include <decoders/gif/gif.h>
#include <decoders/jpeg/jpeg.h>
#include <dracarys/decoders/video/ff_frame_decoder.h>
#include <decoders/webp/webp.h>
#include <errno.h>

void initInput(char *filePath, NVImageRawData *imageRawData) {
    int inputType = imageRawData->extraInfo->type;
    int screenWidth = imageRawData->extraInfo->screenWidth;
    int screenHeight = imageRawData->extraInfo->screenHeight;
    if (inputType == PNG) {
        FILE *pngFile = fopen(filePath, "rb");
        if (pngFile == NULL) {
            imageRawData->error = 1;
            LOGE("open fail errno = %d reason = %s \n", errno, strerror(errno));
        } else {
            fseek(pngFile, 0, SEEK_END);
            int data_length = ftell(pngFile);
            rewind(pngFile);
            imageRawData->meta = malloc(data_length);
            memset(imageRawData->meta, 0, data_length);
            imageRawData->metaSize = fread(imageRawData->meta, 1, data_length, pngFile);
            imageRawData->error = 0;
            fclose(pngFile);
        }
    } else if (inputType == JPEG) {
        JpegInfo *info = openJpegFile(filePath, screenWidth, screenHeight);
        if (info != NULL) {
            imageRawData->meta = info;
            imageRawData->metaSize = 0;
            imageRawData->error = 0;
        } else {
            imageRawData->error = 1;
        }
    } else if (inputType == GIF) {
        imageRawData->metaSize = 0;
        GifInfo *info = openGifFile(filePath);
        if (info != NULL) {
            imageRawData->meta = info;
            imageRawData->width = (int) info->gifFilePtr->SWidth;
            imageRawData->height = (int) info->gifFilePtr->SHeight;
            imageRawData->gl_color_format = GL_RGBA;
            imageRawData->size = imageRawData->width * imageRawData->height * sizeof(argb);
            imageRawData->data = malloc((size_t) imageRawData->size);
            imageRawData->error = 0;
            info->stride = (uint32_t) info->gifFilePtr->SWidth;
        } else {
            imageRawData->error = 1;
        }
    } else if (inputType == VIDEO) {
        FFVideoFrameInfo *info = openVideoFile(filePath, screenWidth, screenHeight);
        if (info != NULL) {
            imageRawData->meta = info;
            imageRawData->metaSize = 0;
            imageRawData->width = info->width;
            imageRawData->height = info->height;
            imageRawData->gl_color_format = GL_RGBA;
            imageRawData->size = imageRawData->width * imageRawData->height * 4;
            imageRawData->data = malloc((size_t) imageRawData->size);
            memset(imageRawData->data, 0, (size_t) imageRawData->size);
            imageRawData->error = 0;
        } else {
            imageRawData->error = 1;
        }
    } else if (inputType == WEBP) {
        WebpInfo *info = openWebPFile(filePath, screenWidth, screenHeight);
        if (info != NULL) {
            imageRawData->meta = info;
            imageRawData->metaSize = 0;
            imageRawData->width = info->width;
            imageRawData->height = info->height;
            imageRawData->gl_color_format = GL_RGBA;
            imageRawData->size = imageRawData->width * imageRawData->height * 4;
            imageRawData->data = malloc((size_t) imageRawData->size);
            memset(imageRawData->data, 0, (size_t) imageRawData->size);
            imageRawData->error = 0;
        } else {
            imageRawData->error = 1;
        }
    } else {
        imageRawData->error = 1;
    }
}

void decodeRawImageData(NVImageRawData *data, float segmentTimeSec, int screenWidth, int screenHeight) {
    if (data->error == 1) {
        return;
    }
    int inputType = data->extraInfo->type;
    if (inputType == PNG) {
        if (data->dataDecoded == 0) {
            DecodedRawFrameData rawData;
            rawData = get_raw_image_data_from_png(data->meta, data->metaSize);
            if (&rawData != NULL && rawData.error == 0) {
                data->error = 0;
                data->width = rawData.width;
                data->height = rawData.height;
                data->gl_color_format = rawData.gl_color_format;
                data->size = rawData.size;
                downsampleToFitScreenSize(data, rawData.data, screenWidth, screenHeight);
                data->decodedStaticImageData = malloc(data->size);
                memset(data->decodedStaticImageData, 0, data->size);
                memcpy(data->decodedStaticImageData, data->data, data->size);
                data->dataDecoded = 1;
                if (rawData.data != NULL) {
                    free((void *) rawData.data);
                    rawData.data = NULL;
                }
            } else {
                data->dataDecoded = 0;
                data->error = 1;
            }
        } else {
            memset(data->data, 0, data->size);
            memcpy(data->data, data->decodedStaticImageData, data->size);
            data->error = 0;
        }
    } else if (inputType == JPEG) {
        if (data->dataDecoded == 0) {
            int result = decompressJpeg(data->meta);
            if (result > 0) {
                JpegInfo *info = data->meta;
                data->data = info->data;
                data->width = info->width;
                data->height = info->height;
                data->size = info->size;
                data->gl_color_format = info->format;
                data->error = 0;
                data->dataDecoded = 1;
            } else {
                data->dataDecoded = 0;
                data->error = 1;
            }
        } else {
            // do nothing here
        }
    } else if (inputType == GIF) {
        if (data->meta != NULL) {
            uint_fast32_t result = getGifFrameAt(data->meta, (int) (segmentTimeSec * 1000), data->data);
            if (result > 0) {
                data->error = 0;
            } else {
                data->error = 1;
            }
        } else {
            data->error = 1;
        }
    } else if (inputType == VIDEO) {
        if (data->meta != NULL) {
            int result = getVideoFrameAt(data->meta, (int64_t) (segmentTimeSec * 1000),
                                         data->extraInfo->videoTrimStartMs,
                                         data->extraInfo->videoTrimStartMs +data->extraInfo->videoTrimDurationMs,
                                         screenWidth, screenHeight, data->data);
            if (result > 0) {
                data->error = 0;
            } else {
                data->error = 1;
            }
        } else {
            data->error = 1;
        }
    } else if (inputType == WEBP) {
        if (data->meta != NULL) {
            int result = getWebPFrameAt(data->meta, (int64_t) (segmentTimeSec * 1000), data->data);
            if (result > 0) {
                data->error = 0;
            } else {
                data->error = 1;
            }
        } else {
            data->error = 1;
        }
    }
}

void releaseRawImageData(void *self) {
    NVImageRawData* data = (NVImageRawData*) self;
    if (data != NULL && data->error == 0) {
        if (data->data) {
            free(data->data);
            data->data = NULL;
        }
        int inputType = data->extraInfo->type;
        if (data->meta) {
            if (inputType == GIF) {
                cleanUp(data->meta);
            } else if (inputType == JPEG) {
                cleanUpJpegResource(data->meta);
            } else if (inputType == VIDEO) {
                cleanUpVideoResource(data->meta);
            } else if (inputType == WEBP) {
                cleanUpWebPResource(data->meta);
            }
            free(data->meta);
            data->meta = NULL;
        }

        if (inputType == PNG && data->dataDecoded == 1) {
            free(data->decodedStaticImageData);
            data->decodedStaticImageData = NULL;
        }
        data->extraInfo = NULL;
    }
}
