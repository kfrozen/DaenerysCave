#ifndef PNG_FUNCTION_LAB_DECODER_IMAGE_H
#define PNG_FUNCTION_LAB_DECODER_IMAGE_H

#include <assert.h>
#include <string.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "png.h"
#include "dracarys/decoders/NVmageDecoder.h"

/* Returns the decoded image data, or aborts if there's an error during decoding. */
DecodedRawFrameData get_raw_image_data_from_png(const void* png_data, const int png_data_size);
void release_raw_image_data(const DecodedRawFrameData* data);

#endif //PNG_FUNCTION_LAB_DECODER_IMAGE_H
