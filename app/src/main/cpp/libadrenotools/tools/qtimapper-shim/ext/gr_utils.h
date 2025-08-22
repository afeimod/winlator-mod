/*
 * Copyright (c) 2011-2016,2018-2019, The Linux Foundation. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __GR_UTILS_H__
#define __GR_UTILS_H__

#include "gralloc_priv.h"
#include "types/common.h"

#define SZ_2M 0x200000
#define SZ_1M 0x100000
#define SZ_4K 0x1000

#define SIZE_4K 4096
#define SIZE_8K 8192

#ifdef SLAVE_SIDE_CP
#define SECURE_ALIGN SZ_1M
#else  // MASTER_SIDE_CP
#define SECURE_ALIGN SZ_4K
#endif

#define INT(exp) static_cast<int>(exp)
#define UINT(exp) static_cast<unsigned int>(exp)

using android::hardware::graphics::common::V1_1::BufferUsage;

namespace gralloc {
struct BufferInfo {
  BufferInfo(int w, int h, int f, uint64_t usage = 0)
      : width(w), height(h), format(f), layer_count(1), usage(usage) {}
  int width;
  int height;
  int format;
  int layer_count;
  uint64_t usage;
};

template <class Type1, class Type2>
inline Type1 ALIGN(Type1 x, Type2 align) {
  return (Type1)((x + (Type1)align - 1) & ~((Type1)align - 1));
}

enum PlaneComponent {
  /* luma */
  PLANE_COMPONENT_Y = 1 << 0,
  /* chroma blue */
  PLANE_COMPONENT_Cb = 1 << 1,
  /* chroma red */
  PLANE_COMPONENT_Cr = 1 << 2,

  /* red */
  PLANE_COMPONENT_R = 1 << 10,
  /* green */
  PLANE_COMPONENT_G = 1 << 11,
  /* blue */
  PLANE_COMPONENT_B = 1 << 12,

  /* alpha */
  PLANE_COMPONENT_A = 1 << 20,

  /* raw data plane */
  PLANE_COMPONENT_RAW = 1 << 30,

  /* meta information plane */
  PLANE_COMPONENT_META = 1 << 31,
};

struct PlaneLayoutInfo {
  /** Components represented the type of plane. */
  PlaneComponent component;

  /** horizontal subsampling. Must be a positive power of 2. */
  uint32_t h_subsampling;

  /** vertical subsampling. Must be a positive power of 2. */
  uint32_t v_subsampling;

  /** offset to the first byte of the top-left pixel of the plane
   *  and it is calculated from the start of the buffer.
   *  Add base of the handle with offset to get the first byte of the plane.
   */
  uint32_t offset;

  /** step is the distance in bytes from one pixel value to the next. */
  int32_t step;

  /** stride of the plane in pixels */
  int32_t stride;

  /** stride of the plane in in bytes */
  int32_t stride_bytes;

  /** plane height or vertical stride */
  int32_t scanlines;

  /** size of the plane in bytes */
  uint32_t size;
};

bool IsCompressedRGBFormat(int format);
bool IsUncompressedRGBFormat(int format);
uint32_t GetBppForUncompressedRGB(int format);
bool CpuCanAccess(uint64_t usage);
bool CpuCanRead(uint64_t usage);
bool CpuCanWrite(uint64_t usage);
unsigned int GetSize(const BufferInfo& d, unsigned int alignedw, unsigned int alignedh);
void GetRGBPlaneInfo(const BufferInfo& info, int32_t format, int32_t width, int32_t height,
                     int32_t flags, int* plane_count, PlaneLayoutInfo* plane_info);
unsigned int GetRgbMetaSize(int format, uint32_t width, uint32_t height, uint64_t usage);
bool IsUBwcFormat(int format);
bool IsUBwcSupported(int format);
bool IsUBwcPISupported(int format, uint64_t usage);
bool IsUBwcEnabled(int format, uint64_t usage);
void GetRgbUBwcBlockSize(uint32_t bpp, int* block_width, int* block_height);
unsigned int GetRgbUBwcMetaBufferSize(int width, int height, uint32_t bpp);
unsigned int GetUBwcSize(int width, int height, int format, unsigned int alignedw,
                         unsigned int alignedh);
uint32_t GetDataAlignment(int format, uint64_t usage);
bool CanUseAdrenoForSize(int buffer_type, uint64_t usage);
bool GetAdrenoSizeAPIStatus();
bool UseUncached(int format, uint64_t usage);
uint64_t GetHandleFlags(int format, uint64_t usage);
int GetImplDefinedFormat(uint64_t usage, int format);
int GetCustomFormatFlags(int format, uint64_t usage, int* custom_format, uint64_t* priv_flags);
bool IsGPUFlagSupported(uint64_t usage);
bool HasAlphaComponent(int32_t format);
}  // namespace gralloc

#endif  // __GR_UTILS_H__
