//
// Created by tanghaomeng on 2022/10/25.
//

#include "VideoProject.h"

long VideoProject::addClip(char *mediaPath, long startPts, ExtraInfo* mediaExtraInfo) {
    LOGI("VideoProject::addClip invoked with media path = %s, start pts = %ld", mediaPath, startPts);
    long trackId = time(0);
    auto* track = new Track(trackId, NORMAL_TRACK);
    if (startPts > 0) {
        auto* gapClipExtraInfo = (ExtraInfo *) malloc(sizeof(ExtraInfo));
        gapClipExtraInfo->type = GAP;
        gapClipExtraInfo->videoTrimDurationMs = startPts;
        track->appendClip(true, nullptr, gapClipExtraInfo);
    }
    Clip* clip = track->appendClip(false, mediaPath, mediaExtraInfo);
    trackList.push_back(track);
    return clip->getId();
}

long VideoProject::appendClipToMainTrack(char *mediaPath, ExtraInfo* mediaExtraInfo) {
    LOGI("VideoProject::appendClipToMainTrack invoked with media path = %s", mediaPath);
    bool hasMainTrack = false;
    list<Track*>::iterator it = trackList.begin();
    for(int i=0; i<trackList.size(); i++){
        Track* track = *it;
        if (track->getTrackType() == MAIN_TRACK) {
            hasMainTrack = true;
            break;
        }
        ++it;
    }
    if (!hasMainTrack) {
        long trackId = time(0);
        auto* mainTrack = new Track(trackId, MAIN_TRACK);
        Clip* clip = mainTrack->appendClip(false, mediaPath, mediaExtraInfo);
        trackList.insert(trackList.begin(), mainTrack);
        return clip->getId();
    } else {
        auto* mainTrack = *it;
        Clip* clip = mainTrack->appendClip(false, mediaPath, mediaExtraInfo);
        return clip->getId();
    }
}