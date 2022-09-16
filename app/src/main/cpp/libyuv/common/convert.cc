/*
 *  Copyright 2011 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "../include/convert.h"

#include "../include/basic_types.h"
#include "../include/cpu_id.h"
#include "../include/row.h"

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

#define SUBSAMPLE(v, a, s) (v < 0) ? (-((-v + a) >> s)) : ((v + a) >> s)
static __inline int Abs(int v) {
  return v >= 0 ? v : -v;
}

// Convert I422 to ARGB with matrix
    static int I420ToARGBMatrix(const uint8* src_y, int src_stride_y,
                                const uint8* src_u, int src_stride_u,
                                const uint8* src_v, int src_stride_v,
                                uint8* dst_argb, int dst_stride_argb,
                                const struct YuvConstants* yuvconstants, int width, int height) {
        int y;
        void (*I422ToARGBRow)(const uint8* y_buf,
                              const uint8* u_buf,
                              const uint8* v_buf,
                              uint8* rgb_buf,
                              const struct YuvConstants* yuvconstants,
                              int width) = I422ToARGBRow_C;
        if (!src_y || !src_u || !src_v || !dst_argb ||
            width <= 0 || height == 0) {
            return -1;
        }
        // Negative height means invert the image.
        if (height < 0) {
            height = -height;
            dst_argb = dst_argb + (height - 1) * dst_stride_argb;
            dst_stride_argb = -dst_stride_argb;
        }
#if defined(HAS_I422TOARGBROW_SSSE3)
        if (TestCpuFlag(kCpuHasSSSE3)) {
    I422ToARGBRow = I422ToARGBRow_Any_SSSE3;
    if (IS_ALIGNED(width, 8)) {
      I422ToARGBRow = I422ToARGBRow_SSSE3;
    }
  }
#endif
#if defined(HAS_I422TOARGBROW_AVX2)
        if (TestCpuFlag(kCpuHasAVX2)) {
    I422ToARGBRow = I422ToARGBRow_Any_AVX2;
    if (IS_ALIGNED(width, 16)) {
      I422ToARGBRow = I422ToARGBRow_AVX2;
    }
  }
#endif
#if defined(HAS_I422TOARGBROW_NEON)
        if (TestCpuFlag(kCpuHasNEON)) {
            I422ToARGBRow = I422ToARGBRow_Any_NEON;
            if (IS_ALIGNED(width, 8)) {
                I422ToARGBRow = I422ToARGBRow_NEON;
            }
        }
#endif
#if defined(HAS_I422TOARGBROW_DSPR2)
        if (TestCpuFlag(kCpuHasDSPR2) && IS_ALIGNED(width, 4) &&
      IS_ALIGNED(src_y, 4) && IS_ALIGNED(src_stride_y, 4) &&
      IS_ALIGNED(src_u, 2) && IS_ALIGNED(src_stride_u, 2) &&
      IS_ALIGNED(src_v, 2) && IS_ALIGNED(src_stride_v, 2) &&
      IS_ALIGNED(dst_argb, 4) && IS_ALIGNED(dst_stride_argb, 4)) {
    I422ToARGBRow = I422ToARGBRow_DSPR2;
  }
#endif
#if defined(HAS_I422TOARGBROW_MSA)
        if (TestCpuFlag(kCpuHasMSA)) {
    I422ToARGBRow = I422ToARGBRow_Any_MSA;
    if (IS_ALIGNED(width, 8)) {
      I422ToARGBRow = I422ToARGBRow_MSA;
    }
  }
#endif

        for (y = 0; y < height; ++y) {
            I422ToARGBRow(src_y, src_u, src_v, dst_argb, yuvconstants, width);
            dst_argb += dst_stride_argb;
            src_y += src_stride_y;
            if (y & 1) {
                src_u += src_stride_u;
                src_v += src_stride_v;
            }
        }
        return 0;
    }

// Convert ARGB to I420.
LIBYUV_API
int ARGBToI420(const uint8* src_argb, int src_stride_argb,
               uint8* dst_y, int dst_stride_y,
               uint8* dst_u, int dst_stride_u,
               uint8* dst_v, int dst_stride_v,
               int width, int height) {
  int y;
  void (*ARGBToUVRow)(const uint8* src_argb0, int src_stride_argb,
      uint8* dst_u, uint8* dst_v, int width) = ARGBToUVRow_C;
  void (*ARGBToYRow)(const uint8* src_argb, uint8* dst_y, int width) =
      ARGBToYRow_C;
  if (!src_argb ||
      !dst_y || !dst_u || !dst_v ||
      width <= 0 || height == 0) {
    return -1;
  }
  // Negative height means invert the image.
  if (height < 0) {
    height = -height;
    src_argb = src_argb + (height - 1) * src_stride_argb;
    src_stride_argb = -src_stride_argb;
  }
#if defined(HAS_ARGBTOYROW_SSSE3) && defined(HAS_ARGBTOUVROW_SSSE3)
  if (TestCpuFlag(kCpuHasSSSE3)) {
    ARGBToUVRow = ARGBToUVRow_Any_SSSE3;
    ARGBToYRow = ARGBToYRow_Any_SSSE3;
    if (IS_ALIGNED(width, 16)) {
      ARGBToUVRow = ARGBToUVRow_SSSE3;
      ARGBToYRow = ARGBToYRow_SSSE3;
    }
  }
#endif
#if defined(HAS_ARGBTOYROW_AVX2) && defined(HAS_ARGBTOUVROW_AVX2)
  if (TestCpuFlag(kCpuHasAVX2)) {
    ARGBToUVRow = ARGBToUVRow_Any_AVX2;
    ARGBToYRow = ARGBToYRow_Any_AVX2;
    if (IS_ALIGNED(width, 32)) {
      ARGBToUVRow = ARGBToUVRow_AVX2;
      ARGBToYRow = ARGBToYRow_AVX2;
    }
  }
#endif
#if defined(HAS_ARGBTOYROW_NEON)
  if (TestCpuFlag(kCpuHasNEON)) {
    ARGBToYRow = ARGBToYRow_Any_NEON;
    if (IS_ALIGNED(width, 8)) {
      ARGBToYRow = ARGBToYRow_NEON;
    }
  }
#endif
#if defined(HAS_ARGBTOUVROW_NEON)
  if (TestCpuFlag(kCpuHasNEON)) {
    ARGBToUVRow = ARGBToUVRow_Any_NEON;
    if (IS_ALIGNED(width, 16)) {
      ARGBToUVRow = ARGBToUVRow_NEON;
    }
  }
#endif
#if defined(HAS_ARGBTOYROW_MSA)
  if (TestCpuFlag(kCpuHasMSA)) {
    ARGBToYRow = ARGBToYRow_Any_MSA;
    if (IS_ALIGNED(width, 16)) {
      ARGBToYRow = ARGBToYRow_MSA;
    }
  }
#endif
#if defined(HAS_ARGBTOUVROW_MSA)
  if (TestCpuFlag(kCpuHasMSA)) {
    ARGBToUVRow = ARGBToUVRow_Any_MSA;
    if (IS_ALIGNED(width, 32)) {
      ARGBToUVRow = ARGBToUVRow_MSA;
    }
  }
#endif

  for (y = 0; y < height - 1; y += 2) {
    ARGBToUVRow(src_argb, src_stride_argb, dst_u, dst_v, width);
    ARGBToYRow(src_argb, dst_y, width);
    ARGBToYRow(src_argb + src_stride_argb, dst_y + dst_stride_y, width);
    src_argb += src_stride_argb * 2;
    dst_y += dst_stride_y * 2;
    dst_u += dst_stride_u;
    dst_v += dst_stride_v;
  }
  if (height & 1) {
    ARGBToUVRow(src_argb, 0, dst_u, dst_v, width);
    ARGBToYRow(src_argb, dst_y, width);
  }
  return 0;
}

// Convert BGRA to I420.
LIBYUV_API
int BGRAToI420(const uint8* src_bgra, int src_stride_bgra,
               uint8* dst_y, int dst_stride_y,
               uint8* dst_u, int dst_stride_u,
               uint8* dst_v, int dst_stride_v,
               int width, int height) {
  int y;
  void (*BGRAToUVRow)(const uint8* src_bgra0, int src_stride_bgra,
      uint8* dst_u, uint8* dst_v, int width) = BGRAToUVRow_C;
  void (*BGRAToYRow)(const uint8* src_bgra, uint8* dst_y, int width) =
      BGRAToYRow_C;
  if (!src_bgra ||
      !dst_y || !dst_u || !dst_v ||
      width <= 0 || height == 0) {
    return -1;
  }
  // Negative height means invert the image.
  if (height < 0) {
    height = -height;
    src_bgra = src_bgra + (height - 1) * src_stride_bgra;
    src_stride_bgra = -src_stride_bgra;
  }
#if defined(HAS_BGRATOYROW_SSSE3) && defined(HAS_BGRATOUVROW_SSSE3)
  if (TestCpuFlag(kCpuHasSSSE3)) {
    BGRAToUVRow = BGRAToUVRow_Any_SSSE3;
    BGRAToYRow = BGRAToYRow_Any_SSSE3;
    if (IS_ALIGNED(width, 16)) {
      BGRAToUVRow = BGRAToUVRow_SSSE3;
      BGRAToYRow = BGRAToYRow_SSSE3;
    }
  }
#endif
#if defined(HAS_BGRATOYROW_NEON)
  if (TestCpuFlag(kCpuHasNEON)) {
    BGRAToYRow = BGRAToYRow_Any_NEON;
    if (IS_ALIGNED(width, 8)) {
      BGRAToYRow = BGRAToYRow_NEON;
    }
  }
#endif
#if defined(HAS_BGRATOUVROW_NEON)
    if (TestCpuFlag(kCpuHasNEON)) {
      BGRAToUVRow = BGRAToUVRow_Any_NEON;
      if (IS_ALIGNED(width, 16)) {
        BGRAToUVRow = BGRAToUVRow_NEON;
      }
    }
#endif

  for (y = 0; y < height - 1; y += 2) {
    BGRAToUVRow(src_bgra, src_stride_bgra, dst_u, dst_v, width);
    BGRAToYRow(src_bgra, dst_y, width);
    BGRAToYRow(src_bgra + src_stride_bgra, dst_y + dst_stride_y, width);
    src_bgra += src_stride_bgra * 2;
    dst_y += dst_stride_y * 2;
    dst_u += dst_stride_u;
    dst_v += dst_stride_v;
  }
  if (height & 1) {
    BGRAToUVRow(src_bgra, 0, dst_u, dst_v, width);
    BGRAToYRow(src_bgra, dst_y, width);
  }
  return 0;
}

// Convert ABGR to I420.
LIBYUV_API
int ABGRToI420(const uint8* src_abgr, int src_stride_abgr,
               uint8* dst_y, int dst_stride_y,
               uint8* dst_u, int dst_stride_u,
               uint8* dst_v, int dst_stride_v,
               int width, int height) {
  int y;
  void (*ABGRToUVRow)(const uint8* src_abgr0, int src_stride_abgr,
      uint8* dst_u, uint8* dst_v, int width) = ABGRToUVRow_C;
  void (*ABGRToYRow)(const uint8* src_abgr, uint8* dst_y, int width) =
      ABGRToYRow_C;
  if (!src_abgr ||
      !dst_y || !dst_u || !dst_v ||
      width <= 0 || height == 0) {
    return -1;
  }
  // Negative height means invert the image.
  if (height < 0) {
    height = -height;
    src_abgr = src_abgr + (height - 1) * src_stride_abgr;
    src_stride_abgr = -src_stride_abgr;
  }
#if defined(HAS_ABGRTOYROW_SSSE3) && defined(HAS_ABGRTOUVROW_SSSE3)
  if (TestCpuFlag(kCpuHasSSSE3)) {
    ABGRToUVRow = ABGRToUVRow_Any_SSSE3;
    ABGRToYRow = ABGRToYRow_Any_SSSE3;
    if (IS_ALIGNED(width, 16)) {
      ABGRToUVRow = ABGRToUVRow_SSSE3;
      ABGRToYRow = ABGRToYRow_SSSE3;
    }
  }
#endif
#if defined(HAS_ABGRTOYROW_NEON)
  if (TestCpuFlag(kCpuHasNEON)) {
    ABGRToYRow = ABGRToYRow_Any_NEON;
    if (IS_ALIGNED(width, 8)) {
      ABGRToYRow = ABGRToYRow_NEON;
    }
  }
#endif
#if defined(HAS_ABGRTOUVROW_NEON)
  if (TestCpuFlag(kCpuHasNEON)) {
    ABGRToUVRow = ABGRToUVRow_Any_NEON;
    if (IS_ALIGNED(width, 16)) {
      ABGRToUVRow = ABGRToUVRow_NEON;
    }
  }
#endif

  for (y = 0; y < height - 1; y += 2) {
    ABGRToUVRow(src_abgr, src_stride_abgr, dst_u, dst_v, width);
    ABGRToYRow(src_abgr, dst_y, width);
    ABGRToYRow(src_abgr + src_stride_abgr, dst_y + dst_stride_y, width);
    src_abgr += src_stride_abgr * 2;
    dst_y += dst_stride_y * 2;
    dst_u += dst_stride_u;
    dst_v += dst_stride_v;
  }
  if (height & 1) {
    ABGRToUVRow(src_abgr, 0, dst_u, dst_v, width);
    ABGRToYRow(src_abgr, dst_y, width);
  }
  return 0;
}

// Convert RGBA to I420.
LIBYUV_API
int RGBAToI420(const uint8* src_rgba, int src_stride_rgba,
               uint8* dst_y, int dst_stride_y,
               uint8* dst_u, int dst_stride_u,
               uint8* dst_v, int dst_stride_v,
               int width, int height) {
  int y;
  void (*RGBAToUVRow)(const uint8* src_rgba0, int src_stride_rgba,
      uint8* dst_u, uint8* dst_v, int width) = RGBAToUVRow_C;
  void (*RGBAToYRow)(const uint8* src_rgba, uint8* dst_y, int width) =
      RGBAToYRow_C;
  if (!src_rgba ||
      !dst_y || !dst_u || !dst_v ||
      width <= 0 || height == 0) {
    return -1;
  }
  // Negative height means invert the image.
  if (height < 0) {
    height = -height;
    src_rgba = src_rgba + (height - 1) * src_stride_rgba;
    src_stride_rgba = -src_stride_rgba;
  }
#if defined(HAS_RGBATOYROW_SSSE3) && defined(HAS_RGBATOUVROW_SSSE3)
  if (TestCpuFlag(kCpuHasSSSE3)) {
    RGBAToUVRow = RGBAToUVRow_Any_SSSE3;
    RGBAToYRow = RGBAToYRow_Any_SSSE3;
    if (IS_ALIGNED(width, 16)) {
      RGBAToUVRow = RGBAToUVRow_SSSE3;
      RGBAToYRow = RGBAToYRow_SSSE3;
    }
  }
#endif
#if defined(HAS_RGBATOYROW_NEON)
  if (TestCpuFlag(kCpuHasNEON)) {
    RGBAToYRow = RGBAToYRow_Any_NEON;
    if (IS_ALIGNED(width, 8)) {
      RGBAToYRow = RGBAToYRow_NEON;
    }
  }
#endif
#if defined(HAS_RGBATOUVROW_NEON)
  if (TestCpuFlag(kCpuHasNEON)) {
    RGBAToUVRow = RGBAToUVRow_Any_NEON;
    if (IS_ALIGNED(width, 16)) {
      RGBAToUVRow = RGBAToUVRow_NEON;
    }
  }
#endif

  for (y = 0; y < height - 1; y += 2) {
    RGBAToUVRow(src_rgba, src_stride_rgba, dst_u, dst_v, width);
    RGBAToYRow(src_rgba, dst_y, width);
    RGBAToYRow(src_rgba + src_stride_rgba, dst_y + dst_stride_y, width);
    src_rgba += src_stride_rgba * 2;
    dst_y += dst_stride_y * 2;
    dst_u += dst_stride_u;
    dst_v += dst_stride_v;
  }
  if (height & 1) {
    RGBAToUVRow(src_rgba, 0, dst_u, dst_v, width);
    RGBAToYRow(src_rgba, dst_y, width);
  }
  return 0;
}

// Convert I420 to ABGR.
LIBYUV_API
int I420ToABGR(const uint8* src_y, int src_stride_y,
        const uint8* src_u, int src_stride_u,
        const uint8* src_v, int src_stride_v,
        uint8* dst_abgr, int dst_stride_abgr,
        int width, int height) {
  return I420ToARGBMatrix(src_y, src_stride_y,
                              src_v, src_stride_v,  // Swap U and V
                              src_u, src_stride_u,
                              dst_abgr, dst_stride_abgr,
                              &kYvuI601Constants,  // Use Yvu matrix
                              width, height);
}

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif
