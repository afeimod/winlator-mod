/*
 * Copyright (c) 2011-2018, The Linux Foundation. All rights reserved.

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

#ifndef __GR_ADRENO_INFO_H__
#define __GR_ADRENO_INFO_H__

#include <stddef.h>
#include <stdint.h>

namespace gralloc {

// Adreno Pixel Formats
typedef enum {
  ADRENO_PIXELFORMAT_UNKNOWN = 0,
  ADRENO_PIXELFORMAT_R16G16B16A16_FLOAT = 10,
  ADRENO_PIXELFORMAT_R10G10B10A2_UNORM = 24,  // Vertex, Normalized GL_UNSIGNED_INT_10_10_10_2_OES
  ADRENO_PIXELFORMAT_R8G8B8A8 = 28,
  ADRENO_PIXELFORMAT_R8G8B8A8_SRGB = 29,
  ADRENO_PIXELFORMAT_D32_FLOAT = 40,
  ADRENO_PIXELFORMAT_D24_UNORM_S8_UINT = 45,
  ADRENO_PIXELFORMAT_D16_UNORM = 55,
  ADRENO_PIXELFORMAT_B5G6R5 = 85,
  ADRENO_PIXELFORMAT_B5G5R5A1 = 86,
  ADRENO_PIXELFORMAT_B8G8R8A8_UNORM = 87,
  ADRENO_PIXELFORMAT_B8G8R8A8 = 90,
  ADRENO_PIXELFORMAT_B8G8R8A8_SRGB = 91,
  ADRENO_PIXELFORMAT_B8G8R8X8_SRGB = 93,
  ADRENO_PIXELFORMAT_NV12 = 103,
  ADRENO_PIXELFORMAT_P010 = 104,
  ADRENO_PIXELFORMAT_YUY2 = 107,
  ADRENO_PIXELFORMAT_B4G4R4A4 = 115,
  ADRENO_PIXELFORMAT_NV12_EXT = 506,       // NV12 with non-std alignment and offsets
  ADRENO_PIXELFORMAT_R8G8B8X8 = 507,       //  GL_RGB8 (Internal)
  ADRENO_PIXELFORMAT_R8G8B8 = 508,         //  GL_RGB8
  ADRENO_PIXELFORMAT_A1B5G5R5 = 519,       //  GL_RGB5_A1
  ADRENO_PIXELFORMAT_R8G8B8X8_SRGB = 520,  //  GL_SRGB8
  ADRENO_PIXELFORMAT_R8G8B8_SRGB = 521,    //  GL_SRGB8
  ADRENO_PIXELFORMAT_A2B10G10R10_UNORM = 532,
  // Vertex, Normalized GL_UNSIGNED_INT_10_10_10_2_OES
  ADRENO_PIXELFORMAT_R10G10B10X2_UNORM = 537,
  ADRENO_PIXELFORMAT_D24_UNORM_X8_UINT = 548,
  ADRENO_PIXELFORMAT_D24_UNORM = 549,
  ADRENO_PIXELFORMAT_D32_FLOAT_X24S8_UINT = 551,
  ADRENO_PIXELFORMAT_S8_UINT = 552,
  ADRENO_PIXELFORMAT_ASTC_4X4 = 568,         // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_5X4 = 569,         // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_5X5 = 570,         // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_6X5 = 571,         // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_6X6 = 572,         // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_8X5 = 573,         // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_8X6 = 574,         // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_8X8 = 575,         // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_10X5 = 576,        // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_10X6 = 577,        // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_10X8 = 578,        // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_10X10 = 579,       // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_12X10 = 580,       // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_12X12 = 581,       // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_4X4_SRGB = 582,    // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_5X4_SRGB = 583,    // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_5X5_SRGB = 584,    // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_6X5_SRGB = 585,    // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_6X6_SRGB = 586,    // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_8X5_SRGB = 587,    // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_8X6_SRGB = 588,    // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_8X8_SRGB = 589,    // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_10X5_SRGB = 590,   // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_10X6_SRGB = 591,   // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_10X8_SRGB = 592,   // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_10X10_SRGB = 593,  // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_12X10_SRGB = 594,  // ASTC Compressed
  ADRENO_PIXELFORMAT_ASTC_12X12_SRGB = 595,  // ASTC Compressed
  // Vertex, Normalized GL_UNSIGNED_INT_10_10_10_2_OES
  ADRENO_PIXELFORMAT_R5G6B5 = 610,    //  RGBA version of B5G6R5
  ADRENO_PIXELFORMAT_R5G5B5A1 = 611,  //  RGBA version of B5G5R5A1
  ADRENO_PIXELFORMAT_R4G4B4A4 = 612,  //  RGBA version of B4G4R4A4
  ADRENO_PIXELFORMAT_UYVY = 614,      //  YUV 4:2:2 packed progressive (1 plane)
  ADRENO_PIXELFORMAT_NV21 = 619,
  ADRENO_PIXELFORMAT_Y8U8V8A8 = 620,  // YUV 4:4:4 packed (1 plane)
  ADRENO_PIXELFORMAT_Y8 = 625,        //  Single 8-bit luma only channel YUV format
  ADRENO_PIXELFORMAT_TP10 = 654,      // YUV 4:2:0 planar 10 bits/comp (2 planes)
} ADRENOPIXELFORMAT;

typedef enum {
  SURFACE_TILE_MODE_DISABLE = 0x0,  // used for linear surface
  SURFACE_TILE_MODE_ENABLE = 0x1    // used for tiled surface
} surface_tile_mode_t;

class AdrenoMemInfo {
 public:
  /*
   * Function to compute aligned width and aligned height based on
   * width, height, format and usage flags.
   *
   * @return aligned width, aligned height
   */
  void GetAlignedWidthAndHeight(int width, int height, int format, int usage,
                                unsigned int* aligned_w, unsigned int* aligned_h, bool ubwc_enabled,
                                bool tile_enabled);

  /*
   * Function to compute the adreno aligned width and aligned height
   * based on the width and format.
   *
   * @return aligned width, aligned height
   */
  void AlignUnCompressedRGB(int width, int height, int format, int tileEnabled,
                            unsigned int* aligned_w, unsigned int* aligned_h);

  /*
   * Function to compute the adreno aligned width and aligned height
   * based on the width and format.
   *
   * @return aligned width, aligned height
   */
  void AlignCompressedRGB(int width, int height, int format, unsigned int* aligned_w,
                          unsigned int* aligned_h);

  /*
   * Function to compute the pixel alignment requirement.
   *
   * @return alignment
   */
  uint32_t GetGpuPixelAlignment();

  /*
   * Function to query whether GPU supports UBWC for given HAL format
   * @return > 0 : supported
   *           0 : not supported
   */
  bool IsUBWCSupportedByGPU(int format);

  /*
   * Function to check if GPU supports PI or not
   */
  bool IsPISupportedByGPU(int format, uint64_t usage);
  /*
   * Function to get the corresponding Adreno format for given HAL format
   */
  ADRENOPIXELFORMAT GetGpuPixelFormat(int hal_format);

  /*
   * Function to get graphics metadata blob size
   * @return graphics metadata size
   */
  uint32_t AdrenoGetMetadataBlobSize();

  /*
   * Function to populate the graphics metadata blob
   * @return 1 : Successful
   *         2 : Unsuccessful
   */
  int AdrenoInitMemoryLayout(void* metadata_blob, int width, int height, int depth, int format,
                             int num_samples, int isUBWC, uint64_t usage, uint32_t num_planes);
  /*
   * Function to get buffer size for based on graphcis metadata
   * @return buffer size
   */
  uint32_t AdrenoGetAlignedGpuBufferSize(void* metadata_blob);

  /*
   * Function to check if adreno size calculation APIs are avaliable
   * @return true  : Avaliable
   *         false : Unavaliable
   */
  bool AdrenoSizeAPIAvaliable();

  static AdrenoMemInfo* GetInstance();

 private:
  AdrenoMemInfo();
  ~AdrenoMemInfo();
  // link(s)to adreno surface padding library.
  int (*LINK_adreno_compute_padding)(int width, int bpp, int surface_tile_height,
                                     int screen_tile_height, int padding_threshold) = NULL;
  void (*LINK_adreno_compute_aligned_width_and_height)(int width, int height, int bpp,
                                                       int tile_mode, int raster_mode,
                                                       int padding_threshold, int* aligned_w,
                                                       int* aligned_h) = NULL;
  void (*LINK_adreno_compute_fmt_aligned_width_and_height)(int width, int height, int plane_id,
                                                           int format, int num_samples,
                                                           int tile_mode, int raster_mode,
                                                           int padding_threshold, int* aligned_w,
                                                           int* aligned_h) = NULL;
  void (*LINK_adreno_compute_compressedfmt_aligned_width_and_height)(
      int width, int height, int format, int tile_mode, int raster_mode, int padding_threshold,
      int* aligned_w, int* aligned_h, int* bpp) = NULL;
  int (*LINK_adreno_isUBWCSupportedByGpu)(ADRENOPIXELFORMAT format) = NULL;
  unsigned int (*LINK_adreno_get_gpu_pixel_alignment)() = NULL;

  uint32_t (*LINK_adreno_get_metadata_blob_size)() = NULL;
  int (*LINK_adreno_init_memory_layout)(void* metadata_blob, int width, int height, int depth,
                                        ADRENOPIXELFORMAT format, int num_samples,
                                        surface_tile_mode_t tile_mode, uint64_t usage,
                                        uint32_t num_planes) = NULL;
  uint32_t (*LINK_adreno_get_aligned_gpu_buffer_size)(void* metadata_blob) = NULL;
  int (*LINK_adreno_isPISupportedByGpu)(int format, uint64_t usage) = NULL;

  bool gfx_ubwc_disable_ = false;
  bool gfx_ahardware_buffer_disable_ = false;
  void* libadreno_utils_ = NULL;

  static AdrenoMemInfo* s_instance;
};

}  // namespace gralloc

#endif  // __GR_ADRENO_INFO_H__
