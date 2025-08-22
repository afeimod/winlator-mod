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

#include "types/mapper.h"

namespace vendor::qti::hardware::display::mapperextensions::V1_0 {
struct IQtiMapperExtensions;
}

namespace android::hardware::graphics::mapper {
namespace V2_0 {

struct IMapper : public IBase {
  struct BufferDescriptorInfo final {
    uint32_t width __attribute__((aligned(4)));
    uint32_t height __attribute__((aligned(4)));
    uint32_t layerCount __attribute__((aligned(4)));
    graphics::common::V1_0::PixelFormat format __attribute__((aligned(4)));
    hidl_bitfield<graphics::common::V1_0::BufferUsage> usage __attribute__((aligned(8)));
  };

  struct Rect final {
    int32_t left __attribute__((aligned(4)));
    int32_t top __attribute__((aligned(4)));
    int32_t width __attribute__((aligned(4)));
    int32_t height __attribute__((aligned(4)));
  };

  static sp<IMapper> getService(const std::string& /* serviceName */, const bool /* getStub */);
};

}  // namespace V2_0
namespace V3_0 {

struct IMapper : public IBase {
  static sp<IMapper> getService(const std::string& /* serviceName */, const bool /* getStub */);
};

}  // namespace V3_0
namespace V4_0 {

struct IMapper : public IBase {
  static sp<IMapper> getService(const std::string& /* serviceName */, const bool /* getStub */);
};

}  // namespace V4_0
}  // namespace android::hardware::graphics::mapper

namespace vendor::qti::hardware::display::mapper {
namespace V4_0 {

struct IQtiMapper : public IBase {
  static Return<sp<IQtiMapper>> castFrom(const sp<graphics::mapper::V4_0::IMapper>& /* parent */,
                                         bool /* emitError */);
};

}  // namespace V4_0
namespace V3_0 {

struct IQtiMapper : public IBase {
  static Return<sp<IQtiMapper>> castFrom(const sp<graphics::mapper::V3_0::IMapper>& /* parent */,
                                         bool /* emitError */);
};

}  // namespace V3_0
namespace V2_0 {

struct IQtiMapper : public IBase {
  using createDescriptor_cb = std::function<void(graphics::mapper::V2_0::Error error,
                                                 const hidl_vec<uint32_t>& descriptor)>;
  virtual Return<void> createDescriptor(
      const graphics::mapper::V2_0::IMapper::BufferDescriptorInfo& descriptorInfo,
      createDescriptor_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  using importBuffer_cb = std::function<void(graphics::mapper::V2_0::Error error, void* buffer)>;
  virtual Return<void> importBuffer(const hidl_handle& rawHandle, importBuffer_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  virtual Return<graphics::mapper::V2_0::Error> freeBuffer(void* buffer) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  using lock_cb = std::function<void(graphics::mapper::V2_0::Error error, void* data)>;
  virtual Return<void> lock(void* buffer,
                            hidl_bitfield<graphics::common::V1_0::BufferUsage> cpuUsage,
                            const graphics::mapper::V2_0::IMapper::Rect& accessRegion,
                            const hidl_handle& acquireFence, lock_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  using lockYCbCr_cb = std::function<void(graphics::mapper::V2_0::Error error,
                                          const graphics::mapper::V2_0::YCbCrLayout& layout)>;
  virtual Return<void> lockYCbCr(void* buffer,
                                 hidl_bitfield<graphics::common::V1_0::BufferUsage> cpuUsage,
                                 const graphics::mapper::V2_0::IMapper::Rect& accessRegion,
                                 const hidl_handle& acquireFence, lockYCbCr_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  using unlock_cb =
      std::function<void(graphics::mapper::V2_0::Error error, const hidl_handle& releaseFence)>;
  virtual Return<void> unlock(void* buffer, unlock_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  virtual Return<graphics::mapper::V2_0::Error> validateBufferSize(
      void* buffer, const graphics::mapper::V2_0::IMapper::BufferDescriptorInfo& descriptorInfo,
      uint32_t stride) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  using getTransportSize_cb =
      std::function<void(graphics::mapper::V2_0::Error error, uint32_t numFds, uint32_t numInts)>;
  virtual Return<void> getTransportSize(void* buffer, getTransportSize_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  using createDescriptor_2_1_cb = std::function<void(graphics::mapper::V2_0::Error error,
                                                     const hidl_vec<uint32_t>& descriptor)>;
  virtual Return<void> createDescriptor_2_1(
      const graphics::mapper::V2_0::IMapper::BufferDescriptorInfo& descriptorInfo,
      createDescriptor_2_1_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  using getMapperExtensions_cb =
      std::function<void(graphics::mapper::V2_0::Error error,
                         const ::android::sp<::vendor::qti::hardware::display::mapperextensions::
                                                 V1_0::IQtiMapperExtensions>& extensions)>;
  virtual Return<void> getMapperExtensions(getMapperExtensions_cb _hidl_cb);

  static Return<sp<IQtiMapper>> castFrom(const sp<graphics::mapper::V2_0::IMapper>& /* parent */,
                                         bool /* emitError */);
};

}  // namespace V2_0
}  // namespace vendor::qti::hardware::display::mapper
