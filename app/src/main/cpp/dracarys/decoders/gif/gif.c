//
// Created by shenjun on 2019-07-29.
//

#include "gif.h"

#define RUNTIME_EXCEPTION_CLASS_NAME "java/lang/RuntimeException"
#define OUT_OF_MEMORY_ERROR_CLASS_NAME "java/lang/OutOfMemoryError"
#define NULL_POINTER_EXCEPTION_CLASS_NAME "java/lang/NullPointerException"

GifInfo* openGifFile(char *filename) {
    if (filename == NULL) {
        LOGE("Input source is null");
        return NULL_GIF_INFO
    }
    FILE *file = fopen(filename, "rbe");
    if (file == NULL) {
        LOGE("open input source failed");
        return NULL_GIF_INFO;
    }

    struct stat64 st;
    const long long sourceLength = stat64(filename, &st) == 0 ? st.st_size : -1;

    GifInfo *const info = createGifInfoFromFile(file, sourceLength);
    if (info == NULL) {
        fclose(file);
    }
    return info;
}

uint_fast8_t fileRead(GifFileType *gif, GifByteType *bytes, uint_fast8_t size) {
    FILE *file = (FILE *) gif->UserData;
    return (uint_fast8_t) fread(bytes, 1, size, file);
}

int fileRewind(GifInfo *info) {
    if (fseeko(info->gifFilePtr->UserData, info->startPos, SEEK_SET) == 0) {
        return 0;
    }
    info->gifFilePtr->Error = D_GIF_ERR_REWIND_FAILED;
    return -1;
}

static GifInfo *createGifInfoFromFile(FILE *file, const long long sourceLength) {
    GifSourceDescriptor descriptor = {
            .rewindFunc = fileRewind,
            .sourceLength = sourceLength
    };
    descriptor.GifFileIn = DGifOpen(file, &fileRead, &descriptor.Error);
    descriptor.startPos = ftell(file);

    return createGifInfo(&descriptor);
}

inline void throwException(JNIEnv *env, enum Exception exception, char *message) {
    if (errno == ENOMEM) {
        exception = OUT_OF_MEMORY_ERROR;
    }

    const char *exceptionClassName;
    switch (exception) {
        case OUT_OF_MEMORY_ERROR:
            exceptionClassName = OUT_OF_MEMORY_ERROR_CLASS_NAME;
            break;
        case NULL_POINTER_EXCEPTION:
            exceptionClassName = NULL_POINTER_EXCEPTION_CLASS_NAME;
            break;
        case RUNTIME_EXCEPTION_ERRNO:
            exceptionClassName = RUNTIME_EXCEPTION_CLASS_NAME;
            char fullMessage[NL_TEXTMAX] = "";
            strncat(fullMessage, message, NL_TEXTMAX);

            char errnoMessage[NL_TEXTMAX];
            if (strerror_r(errno, errnoMessage, NL_TEXTMAX) == 0) {
                strncat(fullMessage, errnoMessage, NL_TEXTMAX);
            }
            message = fullMessage;
            break;
        case RUNTIME_EXCEPTION_BARE:
        default:
            exceptionClassName = RUNTIME_EXCEPTION_CLASS_NAME;
    }

    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
        return;
    }

    jclass exClass = (*env)->FindClass(env, exceptionClassName);
    if (exClass != NULL) {
        (*env)->ThrowNew(env, exClass, message);
    }
}