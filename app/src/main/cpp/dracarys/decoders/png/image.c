#include "image.h"

typedef struct {
	const png_byte* data;
	const png_size_t size;
	const int error;
} DataHandle;

typedef struct {
	const DataHandle data;
	png_size_t offset;
} ReadDataHandle;

typedef struct {
	const png_uint_32 width;
	const png_uint_32 height;
	const int color_type;
} PngInfo;

static void read_png_data_callback(
	png_structp png_ptr, png_byte* png_data, png_size_t read_length);
static PngInfo read_and_update_info(const png_structp png_ptr, const png_infop info_ptr);
static DataHandle read_entire_png_image(
	const png_structp png_ptr, const png_infop info_ptr, const png_uint_32 height);
static GLenum get_gl_color_format(const int png_color_format);

static DecodedRawFrameData getErrorRawImageData() {
	return (DecodedRawFrameData) {
		720, 1280, 0, GL_RGBA, NULL, 1
	};
}

DecodedRawFrameData get_raw_image_data_from_png(const void* png_data, const int png_data_size) {
	if (png_data == NULL || png_data_size <= 8 || !png_check_sig((void*)png_data, 8)) {
		return getErrorRawImageData();
	}
//	assert(png_data != NULL && png_data_size > 8);
//	assert(png_check_sig((void*)png_data, 8));

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		return getErrorRawImageData();
	}
	//assert(png_ptr != NULL);
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		return getErrorRawImageData();
	}
//	assert(info_ptr != NULL);

	ReadDataHandle png_data_handle = (ReadDataHandle) {{png_data, png_data_size}, 0};
	png_set_read_fn(png_ptr, &png_data_handle, read_png_data_callback);

	if (setjmp(png_jmpbuf(png_ptr))) {
//		CRASH("Error reading PNG file!");
	}

	const PngInfo png_info = read_and_update_info(png_ptr, info_ptr);
	const DataHandle raw_image = read_entire_png_image(png_ptr, info_ptr, png_info.height);

	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	if (raw_image.error) {
		return getErrorRawImageData();
	}

	return (DecodedRawFrameData) {
        png_info.width,
        png_info.height,
        raw_image.size,
        get_gl_color_format(png_info.color_type),
        raw_image.data,
        0};
}

void release_raw_image_data(const DecodedRawFrameData* data) {
	if (data != NULL) {
		free((void*)data->data);
	}
}

static void read_png_data_callback(png_structp png_ptr, png_byte* raw_data, png_size_t read_length) {
	ReadDataHandle* handle = png_get_io_ptr(png_ptr);
	const png_byte* png_src = handle->data.data + handle->offset;

	memcpy(raw_data, png_src, read_length);
	handle->offset += read_length;
}

static PngInfo read_and_update_info(const png_structp png_ptr, const png_infop info_ptr) {
	png_uint_32 width, height;
	int bit_depth, color_type;

	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

	// Convert transparency to full alpha
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);

	// Convert grayscale, if needed.
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png_ptr);

	// Convert paletted images, if needed.
	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);

	// Add alpha channel, if there is none (rationale: GL_RGBA is faster than GL_RGB on many GPUs)
	if (color_type == PNG_COLOR_TYPE_PALETTE || color_type == PNG_COLOR_TYPE_RGB)
	   png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);

	// Ensure 8-bit packing
	if (bit_depth < 8)
	   png_set_packing(png_ptr);
	else if (bit_depth == 16)
		png_set_scale_16(png_ptr);

	png_read_update_info(png_ptr, info_ptr);

	// Read the new color type after updates have been made.
	color_type = png_get_color_type(png_ptr, info_ptr);

	return (PngInfo) {width, height, color_type};
}

static DataHandle read_entire_png_image(const png_structp png_ptr, const png_infop info_ptr, const png_uint_32 height) {
	const png_size_t row_size = png_get_rowbytes(png_ptr, info_ptr);
	const int data_length = row_size * height;
	if (row_size <= 0) {
		return (DataHandle) {NULL, 0, 1};
	}
//	assert(row_size > 0);

	png_byte* raw_image = malloc(data_length);
	if (raw_image == NULL) {
		return (DataHandle) {NULL, 0, 1};;
	}
//	assert(raw_image != NULL);

	png_byte* row_ptrs[height];

	png_uint_32 i;
	for (i = 0; i < height; i++) {
		row_ptrs[i] = raw_image + i * row_size;
	}

	png_read_image(png_ptr, &row_ptrs[0]);

	return (DataHandle) {raw_image, data_length, 0};
}

static GLenum get_gl_color_format(const int png_color_format) {
	switch (png_color_format) {
		case PNG_COLOR_TYPE_GRAY:
			return GL_LUMINANCE;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			return GL_RGBA;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			return GL_LUMINANCE_ALPHA;
	}
	return 0;
}
