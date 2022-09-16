//
// Created by shenjun on 2019-08-09.
//

#ifndef AMINO_ANDROID_BILINEAR_RESAMPLING_H
#define AMINO_ANDROID_BILINEAR_RESAMPLING_H

#endif //AMINO_ANDROID_BILINEAR_RESAMPLING_H

#include <stdint.h>
#include "NVmageDecoder.h"

void downsampleToFitScreenSize(NVImageRawData *src, unsigned char *pixels, int screenWidth, int screenHeight);