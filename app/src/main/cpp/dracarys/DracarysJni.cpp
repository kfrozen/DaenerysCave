//
// Created by tanghaomeng on 2022/9/15.
//
extern "C" {
#include "decoders/gif/gif.h"
#include "DracarysJni.h"
#include "android/bitmap.h"
#include <jni.h>
#include <stdlib.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "render/VideoProject.h"


static ColorMapObject *defaultCmap;

static ANativeWindow *window = NULL;
static JavaVM *jvm;
static jclass jClazz = NULL;


JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    jvm = vm;
    JNIEnv *env;
    if ((*vm).GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    (*vm).AttachCurrentThread(&env, NULL);
    defaultCmap = GifMakeMapObject(8, NULL);
    if (defaultCmap != NULL) {
        uint_fast16_t iColor;
        for (iColor = 1; iColor < 256; iColor++) {
            defaultCmap->Colors[iColor].Red = (GifByteType) iColor;
            defaultCmap->Colors[iColor].Green = (GifByteType) iColor;
            defaultCmap->Colors[iColor].Blue = (GifByteType) iColor;
        }
    } else {
        throwException(env, OUT_OF_MEMORY_ERROR, OOME_MESSAGE);
    }

    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) == -1) {
        //sanity check here instead of on each clock_gettime() call
        throwException(env, RUNTIME_EXCEPTION_BARE, "CLOCK_MONOTONIC_RAW is not present");
    }
    return JNI_VERSION_1_6;
}

__unused JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *__unused vm, void *__unused reserved) {
    GifFreeMapObject(defaultCmap);
}

ColorMapObject *getDefColorMap(void) {
    return defaultCmap;
}


}
