//
// Created by tanghaomeng on 2022/10/4.
//

#ifndef DAENERYSCAVE_COMMONTOOLS_H
#define DAENERYSCAVE_COMMONTOOLS_H

#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, "NV_EGL", __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, "NV_EGL", __VA_ARGS__)

class Range {
    public:
    long start = 0;
    long duration = 0;
    Range(long start, long duration) {
        this->start = start;
        this->duration = duration;
    };
};

#endif //DAENERYSCAVE_COMMONTOOLS_H
