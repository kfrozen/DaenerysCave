// Copyright 2014 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Specific inlined methods for boolean decoder [VP8GetBit() ...]
// This file should be included by the .c sources that actually need to call
// these methods.
//
// Author: Skal (pascal.massimino@gmail.com)

#ifndef WEBP_UTILS_BIT_READER_INL_UTILS_H_
#define WEBP_UTILS_BIT_READER_INL_UTILS_H_

#ifdef HAVE_CONFIG_H
#include "src/webp/config.h"
#endif

#include <string.h>  // for memcpy

#include "dracarys/decoders/webp/libwebp/dsp/dsp.h"
#include "bit_reader_utils.h"
#include "endian_inl_utils.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Derived type lbit_t = natural type for memory I/O

#ifdef __cplusplus
}    // extern "C"
#endif

#endif  // WEBP_UTILS_BIT_READER_INL_UTILS_H_
