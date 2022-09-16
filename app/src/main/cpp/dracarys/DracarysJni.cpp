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
}

static ColorMapObject *defaultCmap;

//static ANativeWindow *window = NULL;
//static JavaVM *jvm;
//static jclass jClazz = NULL;

ColorMapObject *getDefColorMap(void) {
    return GifMakeMapObject(8, NULL);
}
