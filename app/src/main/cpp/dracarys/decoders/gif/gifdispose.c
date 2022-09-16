//
// Created by shenjun on 2019-07-30.
//
#include "gif.h"

void cleanUp(GifInfo *info) {
    free(info->backupPtr);
    info->backupPtr = NULL;
    free(info->controlBlock);
    info->controlBlock = NULL;
    free(info->rasterBits);
    info->rasterBits = NULL;
    free(info->comment);
    info->comment = NULL;

    DGifCloseFile(info->gifFilePtr);
//    free(info);
}
