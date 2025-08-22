/*
 * Copyright 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/NativeHandle.h>

#include "types/common.h"

using namespace android::hardware;
using namespace android;

/* Simplified version of hidl::base::V1_0::IBase */
struct IBase : virtual public RefBase {
  virtual bool isRemote() const;

  using interfaceChain_cb = std::function<void(const hidl_vec<hidl_string>& descriptors)>;
  virtual Return<void> interfaceChain(interfaceChain_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  virtual Return<void> debug(const hidl_handle& fd, const hidl_vec<hidl_string>& options) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  using interfaceDescriptor_cb = std::function<void(const hidl_string& descriptor)>;
  virtual Return<void> interfaceDescriptor(interfaceDescriptor_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  using getHashChain_cb = std::function<void(const hidl_vec<hidl_array<uint8_t, 32>>& hashchain)>;
  virtual Return<void> getHashChain(getHashChain_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  virtual Return<void> setHALInstrumentation() {
    ALOGE("%s", __func__);
    std::terminate();
  }

  virtual Return<bool> linkToDeath(const sp<hidl_death_recipient>& recipient, uint64_t cookie) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  virtual Return<void> ping() {
    ALOGE("%s", __func__);
    std::terminate();
  }

  using getDebugInfo_cb = std::function<void(const hidl::base::V1_0::DebugInfo& info)>;
  virtual Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb) {
    ALOGE("%s", __func__);
    std::terminate();
  }

  virtual Return<void> notifySyspropsChanged() {
    ALOGE("%s", __func__);
    std::terminate();
  }

  virtual Return<bool> unlinkToDeath(const sp<hidl_death_recipient>& recipient) {
    ALOGE("%s", __func__);
    std::terminate();
  }
};
