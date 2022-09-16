/*
 *  Copyright 2011 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef INCLUDE_LIBYUV_CONVERT_H_
#define INCLUDE_LIBYUV_CONVERT_H_

#include "basic_types.h"

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

// ARGB little endian (bgra in memory) to I420.
LIBYUV_API
int ARGBToI420(const uint8* src_frame, int src_stride_frame,
               uint8* dst_y, int dst_stride_y,
               uint8* dst_u, int dst_stride_u,
               uint8* dst_v, int dst_stride_v,
               int width, int height);

// BGRA little endian (argb in memory) to I420.
LIBYUV_API
int BGRAToI420(const uint8* src_frame, int src_stride_frame,
               uint8* dst_y, int dst_stride_y,
               uint8* dst_u, int dst_stride_u,
               uint8* dst_v, int dst_stride_v,
               int width, int height);

// ABGR little endian (rgba in memory) to I420.
LIBYUV_API
int ABGRToI420(const uint8* src_frame, int src_stride_frame,
               uint8* dst_y, int dst_stride_y,
               uint8* dst_u, int dst_stride_u,
               uint8* dst_v, int dst_stride_v,
               int width, int height);

// RGBA little endian (abgr in memory) to I420.
LIBYUV_API
int RGBAToI420(const uint8* src_frame, int src_stride_frame,
               uint8* dst_y, int dst_stride_y,
               uint8* dst_u, int dst_stride_u,
               uint8* dst_v, int dst_stride_v,
               int width, int height);

LIBYUV_API
int I420ToABGR(const uint8* src_y, int src_stride_y,
               const uint8* src_u, int src_stride_u,
               const uint8* src_v, int src_stride_v,
               uint8* dst_argb, int dst_stride_argb,
               int width, int height);

#ifdef HAVE_JPEG
// src_width/height provided by capture.
// dst_width/height for clipping determine final size.
LIBYUV_API
int MJPGToI420(const uint8* sample, size_t sample_size,
               uint8* dst_y, int dst_stride_y,
               uint8* dst_u, int dst_stride_u,
               uint8* dst_v, int dst_stride_v,
               int src_width, int src_height,
               int dst_width, int dst_height);

// Query size of MJPG in pixels.
LIBYUV_API
int MJPGSize(const uint8* sample, size_t sample_size,
             int* width, int* height);
#endif

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif

#endif  // INCLUDE_LIBYUV_CONVERT_H_
