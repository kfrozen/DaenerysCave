//
// Created by shenjun on 2019-07-30.
//

#include "gif.h"

bool reset(GifInfo *info) {
    if (info->rewindFunction(info) != 0) {
        return false;
    }
//    info->nextStartTime = 0;
    info->currentLoop = 0;
    info->currentIndex = 0;
    info->lastFrameRemainder = -1;
    info->lastDecodedIndex = -1;
    info->lastDecodedDuration = 0;
    return true;
}

uint_fast32_t getGifFrameAt(GifInfo *info, int desiredPos, void *pixels) {
    if (info == NULL) {
        return 0;
    }
    if (info->gifFilePtr->ImageCount == 1) {
        return renderGifFrame(info, pixels);
    }

    unsigned long sum = 0;
    unsigned long sumNext = 0;
    unsigned int desiredIndex;
    unsigned long fixedDesiredPos;
    for (desiredIndex = 0; desiredIndex < info->gifFilePtr->ImageCount - 1; desiredIndex++) {
        sumNext = sum + info->controlBlock[desiredIndex].DelayTime;
        if (sumNext > (unsigned long) desiredPos) {
            fixedDesiredPos = (unsigned long) desiredPos;
            break;
        }
        sum = sumNext;
    }

    // fix desiredPos if it is bigger than total gif play time
    if (desiredPos > sumNext) {
        fixedDesiredPos = desiredPos - (desiredPos / sum) * sum;
        sum = 0;
        for (desiredIndex = 0; desiredIndex < info->gifFilePtr->ImageCount - 1; desiredIndex++) {
            unsigned long newSum = sum + info->controlBlock[desiredIndex].DelayTime;
            if (newSum > fixedDesiredPos)
                break;
            sum = newSum;
        }
    }

    if (info->lastFrameRemainder != -1) {
        info->lastFrameRemainder = fixedDesiredPos - sum;
        if (desiredIndex == info->gifFilePtr->ImageCount - 1 &&
            info->lastFrameRemainder > (long long) info->controlBlock[desiredIndex].DelayTime)
            info->lastFrameRemainder = info->controlBlock[desiredIndex].DelayTime;
    }

    if (info->lastDecodedIndex == desiredIndex) {
        // no need to decode
        return info->lastDecodedDuration;
    }

    uint_fast32_t result = seek(info, desiredIndex, pixels);

    if (result > 0) {
        info->lastDecodedIndex = desiredIndex;
        info->lastDecodedDuration = result;
    }
//    info->nextStartTime = getRealTime() + (long) (info->lastFrameRemainder / info->speedFactor);

    return result;
}

uint_fast32_t seek(GifInfo *info, uint_fast32_t desiredIndex, void *pixels) {
    GifFileType *const gifFilePtr = info->gifFilePtr;
    if (desiredIndex < info->currentIndex || info->currentIndex == 0) {
        if (!reset(info)) {
            gifFilePtr->Error = D_GIF_ERR_REWIND_FAILED;
            return 0;
        }
        prepareCanvas(pixels, info);
    }
    if (desiredIndex >= gifFilePtr->ImageCount) {
        desiredIndex = gifFilePtr->ImageCount - 1;
    }

    uint_fast32_t i;
    for (i = desiredIndex; i > info->currentIndex; i--) {
        const GifImageDesc imageDesc = info->gifFilePtr->SavedImages[i].ImageDesc;
        if (gifFilePtr->SWidth == imageDesc.Width && gifFilePtr->SHeight == imageDesc.Height) {
            const GraphicsControlBlock controlBlock = info->controlBlock[i];
            if (controlBlock.TransparentColor == NO_TRANSPARENT_COLOR) {
                break;
            } else if (controlBlock.DisposalMode == DISPOSE_BACKGROUND) {
                break;
            }
        }
    }

    if (i > 0) {
        while (info->currentIndex < i - 1) {
            DDGifSlurp(info, false, true);
            ++info->currentIndex;
        }
    }

    do {
        DDGifSlurp(info, true, false);
        drawNextBitmap(pixels, info);
    } while (info->currentIndex++ < desiredIndex);
    --info->currentIndex;
    return getFrameDuration(info);
}

uint_fast32_t renderGifFrame(GifInfo * info, void *pixels) {
    if (info == NULL)
        return 0;
//    long renderStartTime = getRealTime();
    DDGifSlurp(info, true, false);
    if (info->currentIndex == 0) {
        prepareCanvas(pixels, info);
    }
    const uint_fast32_t frameDuration = getBitmap(pixels, info);
    return frameDuration;//calculateInvalidationDelay(info, renderStartTime, frameDuration);
}