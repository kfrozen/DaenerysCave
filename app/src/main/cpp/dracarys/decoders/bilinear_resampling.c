//
// Created by shenjun on 2019-08-09.
//

#include "bilinear_resampling.h"
#include <include/CommonTools.h>
#include <malloc.h>
#include <string.h>

unsigned char getValueAtChannel(unsigned char *pixel, int width, int x, int y, int component, int channel) {
    return pixel[(y * width + x) * component + channel];
}

unsigned char lerp(unsigned char s, unsigned char e, float t) { return s + (e - s) * t; }

unsigned char blerp(unsigned char c00, unsigned char c10, unsigned char c01, unsigned char c11, float tx, float ty) {
    return lerp(lerp(c00, c10, tx), lerp(c01, c11, tx), ty);
}

void downsampleToFitScreenSize(NVImageRawData *src, unsigned char *pixels, int screenWidth,
                               int screenHeight) {
    float rate = MIN(1.0F * src->width / screenWidth, 1.0F * src->height / screenHeight);
    if (rate <= 1) {
        unsigned char *dst = malloc((size_t) src->size);
        memcpy(dst, pixels, (size_t) src->size);
        src->data = dst;
        return;
    }

    int pixelComponent = src->size / src->width / src->height;
    int newWidth = (int) (src->width / rate);
    int newHeight = (int) (src->height / rate);
    int newSize = newWidth * newHeight * pixelComponent;

    unsigned char *dst = malloc((size_t) newSize);

    for (int y = 0; y < newHeight; y++) {
        float gy = (float) y / (newHeight) * (src->height - 1);
        int gyi = (int) gy;
        int gyiNext = MIN(gyi + 1, src->height - 1);

        for (int x = 0; x < newWidth; x++) {
            float gx = (float) x / (newWidth) * (src->width - 1);
            int gxi = (int) gx;
            int gxiNext = MIN(gxi + 1, src->width - 1);

            for (int i = 0; i < pixelComponent; i++) {
                unsigned char c00 = getValueAtChannel(pixels, src->width, gxi, gyi, pixelComponent, i);
                unsigned char c10 = getValueAtChannel(pixels, src->width, gxiNext, gyi, pixelComponent, i);
                unsigned char c01 = getValueAtChannel(pixels, src->width, gxi, gyiNext, pixelComponent, i);
                unsigned char c11 = getValueAtChannel(pixels, src->width, gxiNext, gyiNext, pixelComponent, i);

                dst[(y * newWidth + x) * pixelComponent + i] = blerp(c00, c10, c01, c11, gx - gxi, gy - gyi);
            }
        }
    }
    src->width = newWidth;
    src->height = newHeight;
    src->size = newSize;
    src->data = dst;
}