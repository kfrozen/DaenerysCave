//
// Created by shenjun on 2019-07-29.
//

#include <jni.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/cdefs.h>
#include <sys/stat.h>
#include "dracarys/decoders/gif/giflib/gif_lib.h"
#include <include/CommonTools.h>
#include <malloc.h>

#define GET_ADDR(bm, width, left, top) ((bm) + (top) * (width) + (left))

#define OOME_MESSAGE "Failed to allocate native memory"
#define DEFAULT_FRAME_DURATION_MS 100
#define NULL_GIF_INFO NULL;

/**
 * Decoding error - no frames
 */
#define D_GIF_ERR_NO_FRAMES        1000
/**
* Decoding error - invalid GIF screen size
*/
#define D_GIF_ERR_INVALID_SCR_DIMS    1001
/**
* Decoding error - input source rewind failed
*/
#define D_GIF_ERR_REWIND_FAILED    1004
/**
* Decoding error - invalid and/or indirect byte buffer specified
*/
#define D_GIF_ERR_INVALID_BYTE_BUFFER    1005

enum Exception {
    RUNTIME_EXCEPTION_ERRNO, RUNTIME_EXCEPTION_BARE, OUT_OF_MEMORY_ERROR, NULL_POINTER_EXCEPTION
};

typedef struct {
    GifColorType rgb;
    uint8_t alpha;
} argb;

typedef struct GifInfo GifInfo;

typedef int
(*RewindFunc)(GifInfo *);

struct GifInfo {
    void (*destructor)(GifInfo *, JNIEnv *);

    GifFileType *gifFilePtr;
    GifWord originalWidth, originalHeight;
    uint_fast16_t sampleSize;
    long long lastFrameRemainder;
//    long long nextStartTime;
    uint_fast32_t currentIndex;
    GraphicsControlBlock *controlBlock;
    argb *backupPtr;
    long long startPos;
    unsigned char *rasterBits;
    uint_fast32_t rasterSize;
    char *comment;
    uint_fast16_t loopCount;
    uint_fast16_t currentLoop;
    RewindFunc rewindFunction;
//    jfloat speedFactor;
    uint32_t stride;
//    jlong sourceLength;
    bool isOpaque;
//    void *frameBufferDescriptor;
    int lastDecodedIndex;
    uint_fast32_t lastDecodedDuration;
};

typedef struct {
    GifFileType *GifFileIn;
    int Error;
    long long startPos;
    RewindFunc rewindFunc;
    jlong sourceLength;
} GifSourceDescriptor;

GifInfo *openGifFile(char *filePath);

/**
* @return the real time, in ms
*/
//long getRealTime(void);

/**
* Frees dynamically allocated memory
*/
void cleanUp(GifInfo *info);

int fileRewind(GifInfo *info);

void throwException(JNIEnv *env, enum Exception exception, char *message);

GifInfo *createGifInfo(GifSourceDescriptor *descriptor);

static GifInfo *createGifInfoFromFile(FILE *file, long long sourceLength);

void setGCBDefaults(GraphicsControlBlock *gcb);

void DDGifSlurp(GifInfo *info, bool decode, bool exitAfterFrame);

static int getComment(GifByteType *Bytes, GifInfo *);

static int readExtensions(int ExtFunction, GifByteType *ExtData, GifInfo *info);

//long long
//calculateInvalidationDelay(GifInfo *info, long renderStartTime, uint_fast32_t frameDuration);

void prepareCanvas(const argb *bm, GifInfo *info);

uint_fast32_t getBitmap(argb *bm, GifInfo *info);

#ifdef __cplusplus
extern "C" {
#endif
ColorMapObject *getDefColorMap(void);
#ifdef __cplusplus
}
#endif

static bool checkIfCover(const SavedImage *target, const SavedImage *covered);

static void disposeFrameIfNeeded(argb *bm, GifInfo *info);

static void drawFrame(argb *bm, GifInfo *info, SavedImage *frame);

void drawNextBitmap(argb *bm, GifInfo *info);

uint_fast32_t getFrameDuration(GifInfo *info);

uint_fast32_t seek(GifInfo *info, uint_fast32_t desiredIndex, void *pixels);

uint_fast32_t getGifFrameAt(GifInfo *info, int desiredPos, void *pixels);

uint_fast32_t renderGifFrame(GifInfo *info, void *pixels);