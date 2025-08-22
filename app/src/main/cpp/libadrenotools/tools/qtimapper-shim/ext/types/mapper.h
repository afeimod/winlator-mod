#ifndef HIDL_GENERATED_ANDROID_HARDWARE_GRAPHICS_MAPPER_V2_0_TYPES_H
#define HIDL_GENERATED_ANDROID_HARDWARE_GRAPHICS_MAPPER_V2_0_TYPES_H

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hardware {
namespace graphics {
namespace mapper {
namespace V2_0 {

// Forward declaration for forward reference support:
enum class Error : int32_t;
struct YCbCrLayout;

enum class Error : int32_t {
  NONE = 0,
  BAD_DESCRIPTOR = 1,
  BAD_BUFFER = 2,
  BAD_VALUE = 3,
  NO_RESOURCES = 5,
  UNSUPPORTED = 7,
};

/**
 * A buffer descriptor is an implementation-defined opaque data returned by
 * createDescriptor. It describes the properties of a buffer and is consumed
 * by the allocator.
 */
typedef ::android::hardware::hidl_vec<uint32_t> BufferDescriptor;

/**
 * Structure for describing YCbCr formats for consumption by applications.
 * This is used with PixelFormat::YCBCR_*_888.
 *
 * Buffer chroma subsampling is defined in the format.
 * e.g. PixelFormat::YCBCR_420_888 has subsampling 4:2:0.
 *
 * Buffers must have a 8 bit depth.
 *
 * y, cb, and cr point to the first byte of their respective planes.
 *
 * Stride describes the distance in bytes from the first value of one row of
 * the image to the first value of the next row. It includes the width of the
 * image plus padding.
 * yStride is the stride of the luma plane.
 * cStride is the stride of the chroma planes.
 *
 * chromaStep is the distance in bytes from one chroma pixel value to the
 * next. This is 2 bytes for semiplanar (because chroma values are interleaved
 * and each chroma value is one byte) and 1 for planar.
 */
struct YCbCrLayout final {
  void* y;
  void* cb;
  void* cr;
  uint32_t yStride;
  uint32_t cStride;
  uint32_t chromaStep;
};

//
// type declarations for package
//

template <typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::android::hardware::graphics::mapper::V2_0::Error o);

constexpr int32_t operator|(const ::android::hardware::graphics::mapper::V2_0::Error lhs,
                            const ::android::hardware::graphics::mapper::V2_0::Error rhs) {
  return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs,
                            const ::android::hardware::graphics::mapper::V2_0::Error rhs) {
  return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::android::hardware::graphics::mapper::V2_0::Error lhs,
                            const int32_t rhs) {
  return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::android::hardware::graphics::mapper::V2_0::Error lhs,
                            const ::android::hardware::graphics::mapper::V2_0::Error rhs) {
  return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs,
                            const ::android::hardware::graphics::mapper::V2_0::Error rhs) {
  return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::android::hardware::graphics::mapper::V2_0::Error lhs,
                            const int32_t rhs) {
  return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t& operator|=(int32_t& v,
                              const ::android::hardware::graphics::mapper::V2_0::Error e) {
  v |= static_cast<int32_t>(e);
  return v;
}
constexpr int32_t& operator&=(int32_t& v,
                              const ::android::hardware::graphics::mapper::V2_0::Error e) {
  v &= static_cast<int32_t>(e);
  return v;
}

static inline std::string toString(
    const ::android::hardware::graphics::mapper::V2_0::YCbCrLayout& o);
// operator== and operator!= are not generated for YCbCrLayout

//
// type header definitions for package
//

template <>
inline std::string toString<::android::hardware::graphics::mapper::V2_0::Error>(int32_t o) {
  using ::android::hardware::details::toHexString;
  std::string os;
  ::android::hardware::hidl_bitfield<::android::hardware::graphics::mapper::V2_0::Error> flipped =
      0;
  bool first = true;
  if ((o & ::android::hardware::graphics::mapper::V2_0::Error::NONE) ==
      static_cast<int32_t>(::android::hardware::graphics::mapper::V2_0::Error::NONE)) {
    os += (first ? "" : " | ");
    os += "NONE";
    first = false;
    flipped |= ::android::hardware::graphics::mapper::V2_0::Error::NONE;
  }
  if ((o & ::android::hardware::graphics::mapper::V2_0::Error::BAD_DESCRIPTOR) ==
      static_cast<int32_t>(::android::hardware::graphics::mapper::V2_0::Error::BAD_DESCRIPTOR)) {
    os += (first ? "" : " | ");
    os += "BAD_DESCRIPTOR";
    first = false;
    flipped |= ::android::hardware::graphics::mapper::V2_0::Error::BAD_DESCRIPTOR;
  }
  if ((o & ::android::hardware::graphics::mapper::V2_0::Error::BAD_BUFFER) ==
      static_cast<int32_t>(::android::hardware::graphics::mapper::V2_0::Error::BAD_BUFFER)) {
    os += (first ? "" : " | ");
    os += "BAD_BUFFER";
    first = false;
    flipped |= ::android::hardware::graphics::mapper::V2_0::Error::BAD_BUFFER;
  }
  if ((o & ::android::hardware::graphics::mapper::V2_0::Error::BAD_VALUE) ==
      static_cast<int32_t>(::android::hardware::graphics::mapper::V2_0::Error::BAD_VALUE)) {
    os += (first ? "" : " | ");
    os += "BAD_VALUE";
    first = false;
    flipped |= ::android::hardware::graphics::mapper::V2_0::Error::BAD_VALUE;
  }
  if ((o & ::android::hardware::graphics::mapper::V2_0::Error::NO_RESOURCES) ==
      static_cast<int32_t>(::android::hardware::graphics::mapper::V2_0::Error::NO_RESOURCES)) {
    os += (first ? "" : " | ");
    os += "NO_RESOURCES";
    first = false;
    flipped |= ::android::hardware::graphics::mapper::V2_0::Error::NO_RESOURCES;
  }
  if ((o & ::android::hardware::graphics::mapper::V2_0::Error::UNSUPPORTED) ==
      static_cast<int32_t>(::android::hardware::graphics::mapper::V2_0::Error::UNSUPPORTED)) {
    os += (first ? "" : " | ");
    os += "UNSUPPORTED";
    first = false;
    flipped |= ::android::hardware::graphics::mapper::V2_0::Error::UNSUPPORTED;
  }
  if (o != flipped) {
    os += (first ? "" : " | ");
    os += toHexString(o & (~flipped));
  }
  os += " (";
  os += toHexString(o);
  os += ")";
  return os;
}

static inline std::string toString(::android::hardware::graphics::mapper::V2_0::Error o) {
  using ::android::hardware::details::toHexString;
  if (o == ::android::hardware::graphics::mapper::V2_0::Error::NONE) {
    return "NONE";
  }
  if (o == ::android::hardware::graphics::mapper::V2_0::Error::BAD_DESCRIPTOR) {
    return "BAD_DESCRIPTOR";
  }
  if (o == ::android::hardware::graphics::mapper::V2_0::Error::BAD_BUFFER) {
    return "BAD_BUFFER";
  }
  if (o == ::android::hardware::graphics::mapper::V2_0::Error::BAD_VALUE) {
    return "BAD_VALUE";
  }
  if (o == ::android::hardware::graphics::mapper::V2_0::Error::NO_RESOURCES) {
    return "NO_RESOURCES";
  }
  if (o == ::android::hardware::graphics::mapper::V2_0::Error::UNSUPPORTED) {
    return "UNSUPPORTED";
  }
  std::string os;
  os += toHexString(static_cast<int32_t>(o));
  return os;
}

static inline std::string toString(
    const ::android::hardware::graphics::mapper::V2_0::YCbCrLayout& o) {
  using ::android::hardware::toString;
  std::string os;
  os += "{";
  os += ".y = ";
  os += ::android::hardware::toString(o.y);
  os += ", .cb = ";
  os += ::android::hardware::toString(o.cb);
  os += ", .cr = ";
  os += ::android::hardware::toString(o.cr);
  os += ", .yStride = ";
  os += ::android::hardware::toString(o.yStride);
  os += ", .cStride = ";
  os += ::android::hardware::toString(o.cStride);
  os += ", .chromaStep = ";
  os += ::android::hardware::toString(o.chromaStep);
  os += "}";
  return os;
}

// operator== and operator!= are not generated for YCbCrLayout

}  // namespace V2_0
}  // namespace mapper
}  // namespace graphics
}  // namespace hardware
}  // namespace android

//
// global type declarations for package
//

namespace android {
namespace hardware {
namespace details {
template <>
constexpr std::array<::android::hardware::graphics::mapper::V2_0::Error, 6>
    hidl_enum_values<::android::hardware::graphics::mapper::V2_0::Error> = {
        ::android::hardware::graphics::mapper::V2_0::Error::NONE,
        ::android::hardware::graphics::mapper::V2_0::Error::BAD_DESCRIPTOR,
        ::android::hardware::graphics::mapper::V2_0::Error::BAD_BUFFER,
        ::android::hardware::graphics::mapper::V2_0::Error::BAD_VALUE,
        ::android::hardware::graphics::mapper::V2_0::Error::NO_RESOURCES,
        ::android::hardware::graphics::mapper::V2_0::Error::UNSUPPORTED,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HARDWARE_GRAPHICS_MAPPER_V2_0_TYPES_H
