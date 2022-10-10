//
// Created by tanghaomeng on 2022/10/4.
//

#ifndef DAENERYSCAVE_VIDEOPROJECT_H
#define DAENERYSCAVE_VIDEOPROJECT_H

#ifdef __cplusplus
extern "C++" {
#endif /* __cplusplus */

#include <stdlib.h>
#include <string>
#include "CommonTools.h"
#include "../decoders/NVmageDecoder.h"

using namespace std;

class VideoProject {

};

class Track {

};

class Clip {
    public:
    Clip(string id, bool isGap, char *mediaPath, ExtraInfo mediaExtraInfo) {
        this->id = id;
        this->isGap = isGap;
        this->mediaExtraInfo = mediaExtraInfo;
        if (!isGap) {
            mediaRawData = (NVImageRawData *) malloc(sizeof(NVImageRawData));
            mediaRawData->filePath = mediaPath;
            mediaRawData->dataDecoded = 0;
            mediaRawData->data = NULL;
            mediaRawData->meta = NULL;
            mediaRawData->decodedStaticImageData = NULL;
            mediaRawData->extraInfo = mediaExtraInfo;
        }
        clipRange = new Range(mediaExtraInfo.videoTrimStartMs, mediaExtraInfo.videoTrimDurationMs);
    };

    ~Clip() {
        if (mediaRawData) {
            free(mediaRawData);
            mediaRawData = NULL;
        }
        if (displayRange) {
            delete displayRange;
            displayRange = NULL;
        }
    }

    //getter
    string getId() {
        return id;
    }

    bool isGapClip() {
        return isGap;
    }

    Range* getClipRange() {
        return clipRange;
    }

    Range* getDisplayRange() {
        return displayRange;
    }

    //apis
    void initClip() {
        if (!mediaRawData) {
            return;
        }
        initInput(mediaRawData->filePath, mediaRawData);
    }

    void updateDisplayRange(long startInMs) {
        if (!displayRange) {
            displayRange = new Range(startInMs, mediaExtraInfo.videoTrimDurationMs);
            return;
        }
        displayRange->start = startInMs;
        displayRange->duration = mediaExtraInfo.videoTrimDurationMs;
    }

    private:
    bool isGap;
    string id;
    NVImageRawData *mediaRawData;
    ExtraInfo mediaExtraInfo;
    Range *clipRange;
    Range *displayRange;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //DAENERYSCAVE_VIDEOPROJECT_H
