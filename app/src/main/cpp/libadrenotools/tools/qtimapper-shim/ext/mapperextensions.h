/*
 * Copyright (c) 2016-2022, The Linux Foundation. All rights reserved.

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

#pragma once

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

#include "gralloc_priv.h"
#include "types/mapperextensions.h"

namespace vendor::qti::hardware::display::mapperextensions {
namespace V1_0 {

struct IQtiMapperExtensions : public IBase {
  IQtiMapperExtensions(){};
};

}  // namespace V1_0
namespace V1_1 {
using Error = V1_0::Error;
using PlaneLayout = V1_0::PlaneLayout;

struct IQtiMapperExtensions : public IBase {
  IQtiMapperExtensions();

  gralloc1_device_t* gralloc;
  GRALLOC1_PFN_PERFORM perform;

  using getMapSecureBufferFlag_cb = std::function<void(
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error error, bool mapSecureBuffer)>;
  virtual ::android::hardware::Return<void> getMapSecureBufferFlag(
      void* buffer, getMapSecureBufferFlag_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  // Used, passthrough
  using getInterlacedFlag_cb = std::function<void(
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error error, bool interlaced)>;
  virtual ::android::hardware::Return<void> getInterlacedFlag(void* buffer,
                                                              getInterlacedFlag_cb _hidl_cb);

  using getCustomDimensions_cb =
      std::function<void(::vendor::qti::hardware::display::mapperextensions::V1_0::Error error,
                         int32_t stride, int32_t height)>;
  virtual ::android::hardware::Return<void> getCustomDimensions(void* buffer,
                                                                getCustomDimensions_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  // Used, passthrough
  using getRgbDataAddress_cb = std::function<void(
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error error, void* rgb_data)>;
  virtual ::android::hardware::Return<void> getRgbDataAddress(void* buffer,
                                                              getRgbDataAddress_cb _hidl_cb);

  using calculateBufferAttributes_cb =
      std::function<void(::vendor::qti::hardware::display::mapperextensions::V1_0::Error error,
                         int32_t aligned_width, int32_t aligned_height, bool ubwc_enabled)>;
  virtual ::android::hardware::Return<void> calculateBufferAttributes(
      int32_t width, int32_t height, int32_t format, uint64_t usage,
      calculateBufferAttributes_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  // Used, passhthrough
  using getColorSpace_cb = std::function<void(
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error error, int32_t color_space)>;
  virtual ::android::hardware::Return<void> getColorSpace(void* buffer, getColorSpace_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  // Used, passthrough with hidl fixups
  using getYuvPlaneInfo_cb = std::function<void(
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error error,
      const ::android::hardware::hidl_vec<
          ::vendor::qti::hardware::display::mapperextensions::V1_0::YCbCrLayout>& layout)>;
  virtual ::android::hardware::Return<void> getYuvPlaneInfo(void* buffer,
                                                            getYuvPlaneInfo_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  virtual ::android::hardware::Return<
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error>
  setSingleBufferMode(void* buffer, bool enable) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  using getCustomFormatFlags_cb =
      std::function<void(::vendor::qti::hardware::display::mapperextensions::V1_0::Error error,
                         int32_t format, uint64_t priv_flags)>;
  virtual ::android::hardware::Return<void> getCustomFormatFlags(int32_t format, uint64_t usage,
                                                                 getCustomFormatFlags_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  // Used, cast private data with no passthrough
  using getFd_cb = std::function<void(
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error error, int32_t fd)>;
  virtual ::android::hardware::Return<void> getFd(void* buffer, getFd_cb _hidl_cb);

  // Used, cast private data with no passthrough
  using getWidth_cb = std::function<void(
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error error, int32_t width)>;
  virtual ::android::hardware::Return<void> getWidth(void* buffer, getWidth_cb _hidl_cb);

  // Used, cast private data with no passthrough
  using getHeight_cb = std::function<void(
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error error, int32_t height)>;
  virtual ::android::hardware::Return<void> getHeight(void* buffer, getHeight_cb _hidl_cb);

  // Used, cast private data with no passthrough
  using getOffset_cb = std::function<void(
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error error, uint64_t offset)>;
  virtual ::android::hardware::Return<void> getOffset(void* buffer, getOffset_cb _hidl_cb);

  // Used, cast private data with no passthrough
  using getSize_cb = std::function<void(
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error error, uint64_t size)>;
  virtual ::android::hardware::Return<void> getSize(void* buffer, getSize_cb _hidl_cb);

  using getUnalignedWidth_cb =
      std::function<void(::vendor::qti::hardware::display::mapperextensions::V1_0::Error error,
                         int32_t unaligned_width)>;
  virtual ::android::hardware::Return<void> getUnalignedWidth(void* buffer,
                                                              getUnalignedWidth_cb _hidl_cb);

  using getUnalignedHeight_cb =
      std::function<void(::vendor::qti::hardware::display::mapperextensions::V1_0::Error error,
                         int32_t unaligned_height)>;
  virtual ::android::hardware::Return<void> getUnalignedHeight(void* buffer,
                                                               getUnalignedHeight_cb _hidl_cb);

  using getLayerCount_cb = std::function<void(
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error error, uint64_t layer_count)>;
  virtual ::android::hardware::Return<void> getLayerCount(void* buffer, getLayerCount_cb _hidl_cb);

  using getId_cb = std::function<void(
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error error, uint64_t id)>;
  virtual ::android::hardware::Return<void> getId(void* buffer, getId_cb _hidl_cb);

  using getUsageFlags_cb = std::function<void(
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error error, uint64_t usage)>;
  virtual ::android::hardware::Return<void> getUsageFlags(void* buffer, getUsageFlags_cb _hidl_cb);

  // Used, cast private data with no passthrough
  using getFormat_cb = std::function<void(
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error error, int32_t format)>;
  virtual ::android::hardware::Return<void> getFormat(void* buffer, getFormat_cb _hidl_cb);

  // Used, cast private data with no passthrough
  using getPrivateFlags_cb = std::function<void(
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error error, int32_t flags)>;
  virtual ::android::hardware::Return<void> getPrivateFlags(void* buffer,
                                                            getPrivateFlags_cb _hidl_cb);

  using getSurfaceMetadata_cb = std::function<void(
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error error, void* metadata)>;
  virtual ::android::hardware::Return<void> getSurfaceMetadata(void* buffer,
                                                               getSurfaceMetadata_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  // Used, PAIN
  using getFormatLayout_cb = std::function<void(
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error error, uint64_t size,
      const ::android::hardware::hidl_vec<
          ::vendor::qti::hardware::display::mapperextensions::V1_0::PlaneLayout>& plane_info)>;
  virtual ::android::hardware::Return<void> getFormatLayout(int32_t format, uint64_t usage,
                                                            int32_t flags, int32_t width,
                                                            int32_t height,
                                                            getFormatLayout_cb _hidl_cb);

  // V1.1, Used, passthrough
  virtual ::android::hardware::Return<
      ::vendor::qti::hardware::display::mapperextensions::V1_0::Error>
  getSurfaceMetadata_V1(void* buffer, void* metadata) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  static Return<sp<IQtiMapperExtensions>> castFrom(
      android::sp<V1_0::IQtiMapperExtensions> const& /* parent */, bool /* emitError */);
};

}  // namespace V1_1
}  // namespace vendor::qti::hardware::display::mapperextensions
