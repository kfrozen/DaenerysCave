//
// Created by shenjun on 2019-07-31.
//
#include "jpeg.h"

static GLenum getJpegColorFormat(J_COLOR_SPACE colorSpace) {
    //todo
    switch (colorSpace) {
        case JCS_GRAYSCALE:
            return GL_LUMINANCE;
        case JCS_RGB:
        case JCS_YCbCr:
        case JCS_CMYK:
        case JCS_YCCK:
        case JCS_BG_RGB:
        case JCS_BG_YCC:
            return GL_RGB;
        default:
            return 0;
    }
}

METHODDEF(void)
jpeg_error_exit(j_common_ptr cinfo) {
    /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
    JpegErrorMgr *myerr = (JpegErrorMgr *) cinfo->err;

    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    (*cinfo->err->output_message)(cinfo);

    /* Return control to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
}

JpegInfo *openJpegFile(char *filename, int screenWidth, int screenHeight) {
    struct jpeg_decompress_struct *cinfo = malloc(sizeof(struct jpeg_decompress_struct));
    struct JpegErrorMgr *jerr = malloc(sizeof(JpegErrorMgr));
    FILE *infile;

    if ((infile = fopen(filename, "rb")) == NULL) {
        fprintf(stderr, "can't open %s\n", filename);
        return NULL;
    }
    cinfo->err = jpeg_std_error(&jerr->pub);
    jerr->pub.error_exit = jpeg_error_exit;

    jpeg_create_decompress(cinfo);
    jpeg_stdio_src(cinfo, infile);
    (void) jpeg_read_header(cinfo, TRUE);

    float rate = MIN(1.0F * cinfo->image_width / screenWidth, 1.0F * cinfo->image_height / screenHeight);
    float num = 8 / rate;
    if (num < 8) {
        cinfo->scale_num = (unsigned int) num + 1;
        cinfo->scale_denom = 8;
        while (cinfo->scale_num % 2 == 0) {
            cinfo->scale_num /= 2;
            cinfo->scale_denom /= 2;
        }
    } else {
        cinfo->scale_num = 1;
        cinfo->scale_denom = 1;
    }

    JpegInfo *info = malloc(sizeof(JpegInfo));
    info->cinfo = cinfo;
    info->infile = infile;

    return info;
}

int decompressJpeg(JpegInfo *info) {
    if (info == NULL || info->cinfo == NULL) {
        LOGE("decompress jpeg failed");
        return 0;
    }
    int orientation = getExifOrientation(info->infile);
    JSAMPROW row_pointer[1];
    struct jpeg_decompress_struct *cinfo = info->cinfo;
    (void) jpeg_start_decompress(cinfo);
    size_t row_stride = cinfo->output_width * cinfo->output_components;
    size_t size = row_stride * cinfo->output_height;
    int lineIndex = 0;
    int i = 0;
    unsigned long location = 0;

    unsigned char *pixel = malloc(size);
    row_pointer[0] = (unsigned char *) malloc(row_stride);

    while (cinfo->output_scanline < cinfo->output_height) {
        jpeg_read_scanlines(cinfo, row_pointer, 1);
        for (i = 0; i < cinfo->output_width * cinfo->output_components; i++) {
            if (orientation == 0 || orientation == 1) {
                location = lineIndex * row_stride + i;
            } else if (orientation == 2) {
                location = lineIndex * row_stride + (cinfo->output_width - (i / cinfo->output_components) - 1) * cinfo->output_components + i % cinfo->output_components;
            } else if (orientation == 3) {
                location = (cinfo->output_height - lineIndex - 1) * row_stride + (cinfo->output_width - (i / cinfo->output_components) - 1) * cinfo->output_components + i % cinfo->output_components;
            } else if (orientation == 4) {
                location = (cinfo->output_height - lineIndex - 1) * row_stride + i;
            } else if (orientation == 5) {
                location = (i / cinfo->output_components) * cinfo->output_height * cinfo->output_components + lineIndex * cinfo->output_components + i % cinfo->output_components;
            } else if (orientation == 6) {
                location = (i / cinfo->output_components) * cinfo->output_height * cinfo->output_components + (cinfo->output_height - lineIndex - 1) * cinfo->output_components + i % cinfo->output_components;
            } else if (orientation == 7) {
                location = (cinfo->output_width - i / cinfo->output_components - 1) * cinfo->output_height * cinfo->output_components + (cinfo->output_height - lineIndex - 1) * cinfo->output_components + i % cinfo->output_components;
            } else if (orientation == 8) {
                location = (cinfo->output_width - i / cinfo->output_components - 1) * cinfo->output_height * cinfo->output_components + lineIndex * cinfo->output_components + i % cinfo->output_components;
            }
            pixel[location] = row_pointer[0][i];
        }
        lineIndex++;
    }


    if (orientation <= 4) {
        info->width = cinfo->output_width;
        info->height = cinfo->output_height;
    } else {
        info->width = cinfo->output_height;
        info->height = cinfo->output_width;
    }
    info->size = cinfo->output_width * cinfo->output_height * cinfo->output_components;
    info->format = getJpegColorFormat(cinfo->out_color_space);
    info->data = pixel;

    jpeg_finish_decompress(cinfo);

    free(row_pointer[0]);

    return 1;
}

void cleanUpJpegResource(JpegInfo *info) {
    if (info->cinfo != NULL) {
        jpeg_destroy_decompress(info->cinfo);
        info->cinfo = NULL;
    }
    if (info->infile != NULL) {
        fclose(info->infile);
        info->infile = NULL;
    }
    info->data = NULL;
//    free(info);
}
