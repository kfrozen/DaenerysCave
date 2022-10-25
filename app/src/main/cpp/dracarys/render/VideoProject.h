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
#include <list>
#include <ctime>
#include "CommonTools.h"
#include "NVmageDecoder.h"

using namespace std;

enum TRACK_TYPE {
    NORMAL_TRACK = 0, MAIN_TRACK = 1, LIVE_TRACK = 2
};

class Clip {
    public:
    Clip(long id, bool isGap, char *mediaPath, ExtraInfo* mediaExtraInfo) {
        LOGI("Clip %ld constructor invoked", id);
        this->id = id;
        this->isGap = isGap;
        this->mediaExtraInfo = mediaExtraInfo;
        if (!isGap) {
            this->mediaRawData = (NVImageRawData *) malloc(sizeof(NVImageRawData));
            this->mediaRawData->filePath = mediaPath;
            this->mediaRawData->dataDecoded = 0;
            this->mediaRawData->data = NULL;
            this->mediaRawData->meta = NULL;
            this->mediaRawData->decodedStaticImageData = NULL;
            this->mediaRawData->extraInfo = mediaExtraInfo;
            //this->mediaRawData->release = releaseRawImageData;
        }
        this->clipRange = new Range(mediaExtraInfo->videoTrimStartMs, mediaExtraInfo->videoTrimDurationMs);
    };

    ~Clip() {
        LOGI("Clip %ld destructor invoked", id);
        if (mediaRawData) {
            releaseRawImageData(mediaRawData);
            //mediaRawData->release(mediaRawData);
            free(mediaRawData);
            mediaRawData = NULL;
        }
        if (clipRange) {
            delete clipRange;
            clipRange = NULL;
        }
        if (displayRange) {
            delete displayRange;
            displayRange = NULL;
        }
        if (mediaExtraInfo) {
            if (isGap) {
                free(mediaExtraInfo);
            }
            this->mediaExtraInfo = NULL;
        }
    }

    //setter
    void setExternalId(string extId) {
        this->externalId = extId;
    }

    //getter
    long getId() {
        return id;
    }

    string getExternalId() {
        return externalId;
    }

    bool isGapClip() {
        return isGap;
    }

    Range getClipRange() {
        return *clipRange;
    }

    Range getDisplayRange() {
        return *displayRange;
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
            displayRange = new Range(startInMs, mediaExtraInfo->videoTrimDurationMs);
            return;
        }
        displayRange->start = startInMs;
        displayRange->duration = mediaExtraInfo->videoTrimDurationMs;
    }

    private:
    bool isGap;
    long id;
    string externalId;
    NVImageRawData *mediaRawData;
    ExtraInfo* mediaExtraInfo;
    Range *clipRange;
    Range *displayRange;
};

class Track {
    public:
    Track(long id, enum TRACK_TYPE trackType) {
        LOGI("Track %ld constructor invoked", id);
        this->id = id;
        this->trackType = trackType;
    }

    ~Track() {
        LOGI("Track %ld destructor invoked", id);
        if (!clipList.empty()) {
            clipList.clear();
        }
    }

    //getter
    long getId() {
        return  this->id;
    }

    TRACK_TYPE getTrackType() {
        return this->trackType;
    }

    //apis
    Clip* appendClip(bool isGap, char *mediaPath, ExtraInfo* mediaExtraInfo) {
        return insertClip(clipList.size(), isGap, mediaPath, mediaExtraInfo);
    }

    Clip* insertClip(int index, bool isGap, char *mediaPath, ExtraInfo* mediaExtraInfo) {
        if (index < 0 || index > getClipSize()) {
            LOGE("insertClip: invalid index %d", index);
            return NULL;
        }
        if (this->trackType == LIVE_TRACK && (isGap || mediaExtraInfo->type != LIVE)) {
            LOGE("insertClip: Clip with path %s is not LIVE clip, and cannot fit into a LIVE track", mediaPath);
            return NULL;
        }
        long clipId = time(0);
        Clip* clip = new Clip(clipId, isGap, mediaPath, mediaExtraInfo);
        clipList.insert(getIteratorByIndex(index), clip);
        refreshClipRange();
        return clip;
    }

    long getTrackDurationMs() {
        if (clipList.empty()) {
            return 0L;
        }
        long duration = 0L;
        for(list<Clip*>::iterator it = clipList.begin(); it != clipList.end(); it++){
            duration += (*it)->getDisplayRange().duration;
        }
    }

    int getClipSize() {
        return clipList.size();
    }

    Clip* getClipByIndex(int index) {
        if (index < 0 || index >= getClipSize()) {
            return NULL;
        }
        return *getIteratorByIndex(index);
    }

    void removeClipByIndex(int index) {
        if (index < 0 || index >= getClipSize()) {
            return;
        }
        clipList.erase(getIteratorByIndex(index));
        refreshClipRange();
    }

    private:
    long id;
    enum TRACK_TYPE trackType;
    list<Clip*> clipList;

    list<Clip*>::iterator getIteratorByIndex(int index) {
        list<Clip*>::iterator it = clipList.begin();
        if (index < 0 || index >= getClipSize()) {
            return it;
        }
        for(int i=0; i<index; i++){
            ++it;
        }
        return it;
    }

    void refreshClipRange() {
        if (clipList.empty()) {
            return;
        }
        long accumulatedStartMs = 0L;
        for(list<Clip*>::iterator it = clipList.begin(); it != clipList.end(); it++){
            Clip* clip = *it;
            clip->updateDisplayRange(accumulatedStartMs);
            accumulatedStartMs += clip->getClipRange().duration;
        }
    }
};

class VideoProject {
    public:
    VideoProject() {
        LOGI("VideoProject constructor invoked");

    }

    ~VideoProject() {
        LOGI("VideoProject destructor invoked");
        if (!trackList.empty()) {
            trackList.clear();
        }
    }

    /**
     * Add a new media clip to this project. This clip will be in a standalone track.
     * @param mediaPath absolute local path for media
     * @param startPts the start pts for this clip in the whole project
     * @param mediaExtraInfo
     * @return the id of the created clip
     * */
    long addClip(char *mediaPath, long startPts, ExtraInfo* mediaExtraInfo);

    /**
     * Append a new media clip to the tail of current main track, a new track will be created
     * if the main track absent.
     * @param mediaPath absolute local path for media
     * @param mediaExtraInfo
     * @return the id of the created clip
     * */
    long appendClipToMainTrack(char *mediaPath, ExtraInfo* mediaExtraInfo);

    private:
    list<Track*> trackList;

};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //DAENERYSCAVE_VIDEOPROJECT_H
