//
// Created by shenjun on 2019-07-30.
//

#include "gif.h"

GifInfo *createGifInfo(GifSourceDescriptor *descriptor) {
    if (descriptor->startPos < 0) {
        descriptor->Error = D_GIF_ERR_NOT_READABLE;
    }
    if (descriptor->Error != 0 || descriptor->GifFileIn == NULL) {
        bool readErrno = descriptor->rewindFunc == fileRewind &&
                         (descriptor->Error == D_GIF_ERR_NOT_READABLE ||
                          descriptor->Error == D_GIF_ERR_READ_FAILED);
        LOGE("read gif file failed %d", descriptor->Error);
        DGifCloseFile(descriptor->GifFileIn);
        return NULL;
    }

    GifInfo *info = malloc(sizeof(GifInfo));
    if (info == NULL) {
        DGifCloseFile(descriptor->GifFileIn);
        LOGE(OOME_MESSAGE);
        return NULL;
    }
    info->controlBlock = malloc(sizeof(GraphicsControlBlock));
    if (info->controlBlock == NULL) {
        DGifCloseFile(descriptor->GifFileIn);
        free(info);
        LOGE(OOME_MESSAGE);
        return NULL;
    }
    setGCBDefaults(info->controlBlock);
    info->destructor = NULL;
    info->gifFilePtr = descriptor->GifFileIn;
    info->startPos = descriptor->startPos;
    info->currentIndex = 0;
//    info->nextStartTime = 0;
    info->lastFrameRemainder = -1;
    info->comment = NULL;
    info->loopCount = 1;
    info->currentLoop = 0;
//    info->speedFactor = 1.0f;
//    info->sourceLength = descriptor->sourceLength;

    info->backupPtr = NULL;
    info->rewindFunction = descriptor->rewindFunc;
//    info->frameBufferDescriptor = NULL;
    info->isOpaque = false;
    info->sampleSize = 1;

    DDGifSlurp(info, false, false);
    info->rasterBits = NULL;
    info->rasterSize = 0;
    info->originalHeight = info->gifFilePtr->SHeight;
    info->originalWidth = info->gifFilePtr->SWidth;

    info->lastDecodedIndex = -1;
    info->lastDecodedDuration = 0;

    if (descriptor->GifFileIn->SWidth < 1 || descriptor->GifFileIn->SHeight < 1) {
        cleanUp(info);
        LOGE("read gif file failed %d", D_GIF_ERR_INVALID_SCR_DIMS);
        return NULL;
    }
    if (descriptor->GifFileIn->Error == D_GIF_ERR_NOT_ENOUGH_MEM) {
        cleanUp(info);
        LOGE(OOME_MESSAGE);
        return NULL;
    }
#if defined(STRICT_FORMAT_89A)
    descriptor->Error = descriptor->GifFileIn->Error;
#endif

    if (descriptor->GifFileIn->ImageCount == 0) {
        descriptor->Error = D_GIF_ERR_NO_FRAMES;
    } else if (descriptor->GifFileIn->Error == D_GIF_ERR_REWIND_FAILED) {
        descriptor->Error = D_GIF_ERR_REWIND_FAILED;
    }
    if (descriptor->Error != 0) {
        cleanUp(info);
        LOGE("read gif file failed %d", descriptor->Error);
        return NULL;
    }
    return info;
}

void setGCBDefaults(GraphicsControlBlock *gcb) {
    gcb->DelayTime = DEFAULT_FRAME_DURATION_MS;
    gcb->TransparentColor = NO_TRANSPARENT_COLOR;
    gcb->DisposalMode = DISPOSAL_UNSPECIFIED;
}