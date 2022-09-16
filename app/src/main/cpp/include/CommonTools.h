#ifndef SONG_STUDIO_COMMON
#define SONG_STUDIO_COMMON
#include <android/log.h> 
#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, "NV_EGL", __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, "NV_EGL", __VA_ARGS__)
typedef signed short SInt16;
typedef unsigned char byte;
#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))
#define AUDIO_PCM_OUTPUT_CHANNEL 1
#define ACCOMPANY_PCM_OUTPUT_CHANNEL 2

#endif //SONG_STUDIO_COMMON
