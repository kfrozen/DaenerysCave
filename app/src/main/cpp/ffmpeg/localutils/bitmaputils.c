//
// Created by troy on 2019/5/27.
//
#include "bitmaputils.h"

jobject create_bitmap(JNIEnv *env, int width, int height) {

    jclass clz_bitmap = (*env)->FindClass(env, "android/graphics/Bitmap");
    jmethodID mtd_bitmap = (*env)->GetStaticMethodID(env, clz_bitmap, "createBitmap",
            "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");

    jstring str_config = (*env)->NewStringUTF(env, "ARGB_8888");
    jclass clz_config = (*env)->FindClass(env, "android/graphics/Bitmap$Config");
    jmethodID mtd_config = (*env)->GetStaticMethodID(env,
            clz_config, "valueOf", "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
    jobject obj_config = (*env)->CallStaticObjectMethod(env, clz_config, mtd_config, str_config);

    jobject bitmap = (*env)->CallStaticObjectMethod(env,
            clz_bitmap, mtd_bitmap, width, height, obj_config);
    return bitmap;
}
