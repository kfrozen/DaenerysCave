//
// Created by shenjun on 2019-07-31.
//
#include <include/CommonTools.h>
#include "dracarys/decoders/jpeg/jpeglib/jpeglib.h"
#include <stdio.h>
#include <setjmp.h>
#include <GLES2/gl2.h>


#ifndef AMINO_ANDROID_JPEG_H
#define AMINO_ANDROID_JPEG_H

#endif //AMINO_ANDROID_JPEG_H

typedef struct JpegInfo {
    void *infile;
    struct jpeg_decompress_struct *cinfo;

    int width;
    int height;
    int size;
    GLenum format;
    void *data;

} JpegInfo;

typedef struct JpegErrorMgr {
    struct jpeg_error_mgr pub;	/* "public" fields */

    jmp_buf setjmp_buffer;	/* for return to caller */
} JpegErrorMgr;

JpegInfo *openJpegFile(char *filename, int screenWidth, int screenHeight);

int decompressJpeg(JpegInfo *info);

void cleanUpJpegResource(JpegInfo *info);

int getExifOrientation(FILE *file);