/*
 * Copyright © 2023 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdint.h>
#include <vulkan/vulkan_core.h>

#include "pvr_bo.h"
#include "pvr_private.h"
#include "pvr_robustness.h"
#include "util/u_math.h"

enum pvr_robustness_buffer_format {
   PVR_ROBUSTNESS_BUFFER_FORMAT_UINT64,
   PVR_ROBUSTNESS_BUFFER_FORMAT_UINT32,
   PVR_ROBUSTNESS_BUFFER_FORMAT_UINT16,
   PVR_ROBUSTNESS_BUFFER_FORMAT_UINT8,
   PVR_ROBUSTNESS_BUFFER_FORMAT_SINT64,
   PVR_ROBUSTNESS_BUFFER_FORMAT_SINT32,
   PVR_ROBUSTNESS_BUFFER_FORMAT_SINT16,
   PVR_ROBUSTNESS_BUFFER_FORMAT_SINT8,
   PVR_ROBUSTNESS_BUFFER_FORMAT_SFLOAT64,
   PVR_ROBUSTNESS_BUFFER_FORMAT_SFLOAT32,
   PVR_ROBUSTNESS_BUFFER_FORMAT_SFLOAT16,
   PVR_ROBUSTNESS_BUFFER_FORMAT_A8B8G8R8_UINT,
   PVR_ROBUSTNESS_BUFFER_FORMAT_A8B8G8R8_SINT,
   PVR_ROBUSTNESS_BUFFER_FORMAT_A2R10G10B10_UINT,
   PVR_ROBUSTNESS_BUFFER_FORMAT_A2R10G10B10_SINT,
   PVR_ROBUSTNESS_BUFFER_FORMAT_R4G4B4A4_UNORM,
   PVR_ROBUSTNESS_BUFFER_FORMAT_R5G5B5A1_UNORM,
   PVR_ROBUSTNESS_BUFFER_FORMAT_A1R5G5B5_UNORM,
   PVR_ROBUSTNESS_BUFFER_FORMAT_COUNT
};

/* Offsets in bytes of the [0, 0, 0, 1] vectors within the robustness buffer */
static uint16_t robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_COUNT];

VkResult pvr_init_robustness_buffer(struct pvr_device *device)
{
   uint16_t offset = 0;
   uint8_t *robustness_buffer_map;
   VkResult result;

#define ROBUSTNESS_BUFFER_OFFSET_ALIGN16(cur_offset, add) \
   ((uint16_t)ALIGN((cur_offset + (uint16_t)(add)), 16))

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_UINT64] = offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, sizeof(uint64_t) * 4);

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_UINT32] = offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, sizeof(uint32_t) * 4);

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_UINT16] = offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, sizeof(uint16_t) * 4);

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_UINT8] = offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, sizeof(uint8_t) * 4);

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_SINT64] = offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, sizeof(int64_t) * 4);

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_SINT32] = offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, sizeof(int32_t) * 4);

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_SINT16] = offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, sizeof(int16_t) * 4);

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_SINT8] = offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, sizeof(int8_t) * 4);

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_SFLOAT64] = offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, sizeof(double) * 4);

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_SFLOAT32] = offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, sizeof(float) * 4);

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_SFLOAT16] = offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, sizeof(uint16_t) * 4);

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_A8B8G8R8_UINT] =
      offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, 4);

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_A8B8G8R8_SINT] =
      offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, 4);

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_A2R10G10B10_UINT] =
      offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, 4);

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_A2R10G10B10_SINT] =
      offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, 4);

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_R4G4B4A4_UNORM] =
      offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, 2);

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_R5G5B5A1_UNORM] =
      offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, 2);

   robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_A1R5G5B5_UNORM] =
      offset;
   offset = ROBUSTNESS_BUFFER_OFFSET_ALIGN16(offset, 2);

#undef ROBUSTNESS_BUFFER_OFFSET_ALIGN16

   result = pvr_bo_alloc(device,
                         device->heaps.general_heap,
                         offset,
                         16,
                         PVR_BO_ALLOC_FLAG_CPU_MAPPED,
                         &device->robustness_buffer);
   if (result != VK_SUCCESS)
      return result;

   robustness_buffer_map = device->robustness_buffer->bo->map;

#define ROBUSTNESS_BUFFER_RGBA(format, type, zero, one)                     \
   do {                                                                     \
      type *const buffer =                                                  \
         (type *)robustness_buffer_map + robustness_buffer_offsets[format]; \
      buffer[0] = (type)zero;                                               \
      buffer[1] = (type)zero;                                               \
      buffer[2] = (type)zero;                                               \
      buffer[3] = (type)one;                                                \
   } while (0)

#define ROBUSTNESS_BUFFER_ABGR(format, type, zero, one)                     \
   do {                                                                     \
      type *const buffer =                                                  \
         (type *)robustness_buffer_map + robustness_buffer_offsets[format]; \
      buffer[0] = (type)one;                                                \
      buffer[1] = (type)zero;                                               \
      buffer[2] = (type)zero;                                               \
      buffer[3] = (type)zero;                                               \
   } while (0)

#define ROBUSTNESS_BUFFER_PACKED(format, type, val)                         \
   do {                                                                     \
      type *const buffer =                                                  \
         (type *)robustness_buffer_map + robustness_buffer_offsets[format]; \
      *buffer = (type)val;                                                  \
   } while (0)

   ROBUSTNESS_BUFFER_RGBA(PVR_ROBUSTNESS_BUFFER_FORMAT_UINT64,
                          uint64_t,
                          0ull,
                          UINT64_MAX);
   ROBUSTNESS_BUFFER_RGBA(PVR_ROBUSTNESS_BUFFER_FORMAT_UINT32,
                          uint32_t,
                          0ul,
                          UINT32_MAX);
   ROBUSTNESS_BUFFER_RGBA(PVR_ROBUSTNESS_BUFFER_FORMAT_UINT16,
                          uint16_t,
                          0u,
                          UINT16_MAX);
   ROBUSTNESS_BUFFER_RGBA(PVR_ROBUSTNESS_BUFFER_FORMAT_UINT8,
                          uint8_t,
                          0u,
                          UINT8_MAX);

   ROBUSTNESS_BUFFER_RGBA(PVR_ROBUSTNESS_BUFFER_FORMAT_SINT64,
                          int64_t,
                          0ull,
                          INT64_MAX);
   ROBUSTNESS_BUFFER_RGBA(PVR_ROBUSTNESS_BUFFER_FORMAT_SINT32,
                          int32_t,
                          0ul,
                          INT32_MAX);
   ROBUSTNESS_BUFFER_RGBA(PVR_ROBUSTNESS_BUFFER_FORMAT_SINT16,
                          int16_t,
                          0u,
                          INT16_MAX);
   ROBUSTNESS_BUFFER_RGBA(PVR_ROBUSTNESS_BUFFER_FORMAT_SINT8,
                          int8_t,
                          0u,
                          INT8_MAX);

   ROBUSTNESS_BUFFER_RGBA(PVR_ROBUSTNESS_BUFFER_FORMAT_SFLOAT64,
                          uint64_t,
                          0x0000000000000000ull,
                          0x3ff0000000000000ull);
   ROBUSTNESS_BUFFER_RGBA(PVR_ROBUSTNESS_BUFFER_FORMAT_SFLOAT32,
                          float,
                          0.0f,
                          1.0f);
   ROBUSTNESS_BUFFER_RGBA(PVR_ROBUSTNESS_BUFFER_FORMAT_SFLOAT16,
                          uint16_t,
                          0x0000,
                          0x3c00);

   ROBUSTNESS_BUFFER_ABGR(PVR_ROBUSTNESS_BUFFER_FORMAT_A8B8G8R8_UINT,
                          uint8_t,
                          0u,
                          UINT8_MAX);
   ROBUSTNESS_BUFFER_ABGR(PVR_ROBUSTNESS_BUFFER_FORMAT_A8B8G8R8_UINT,
                          int8_t,
                          0u,
                          INT8_MAX);

   ROBUSTNESS_BUFFER_PACKED(PVR_ROBUSTNESS_BUFFER_FORMAT_A2R10G10B10_UINT,
                            uint32_t,
                            0xC0000000u);
   ROBUSTNESS_BUFFER_PACKED(PVR_ROBUSTNESS_BUFFER_FORMAT_A2R10G10B10_SINT,
                            uint32_t,
                            0x40000000u);
   ROBUSTNESS_BUFFER_PACKED(PVR_ROBUSTNESS_BUFFER_FORMAT_R4G4B4A4_UNORM,
                            uint16_t,
                            0x000Fu);
   ROBUSTNESS_BUFFER_PACKED(PVR_ROBUSTNESS_BUFFER_FORMAT_R5G5B5A1_UNORM,
                            uint16_t,
                            0x0001u);
   ROBUSTNESS_BUFFER_PACKED(PVR_ROBUSTNESS_BUFFER_FORMAT_A1R5G5B5_UNORM,
                            uint16_t,
                            0x8000u);

#undef ROBUSTNESS_BUFFER_RGBA
#undef ROBUSTNESS_BUFFER_ABGR
#undef ROBUSTNESS_BUFFER_PACKED

   return VK_SUCCESS;
}

void pvr_robustness_buffer_finish(struct pvr_device *device)
{
   pvr_bo_free(device, device->robustness_buffer);
}

uint16_t pvr_get_robustness_buffer_format_offset(VkFormat format)
{
   switch (format) {
   case VK_FORMAT_R64G64B64A64_SFLOAT:
      return robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_SFLOAT64];

   case VK_FORMAT_R32G32B32A32_SFLOAT:
      return robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_SFLOAT32];

   case VK_FORMAT_R16G16B16A16_SFLOAT:
      return robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_SFLOAT16];

   case VK_FORMAT_R64G64B64A64_UINT:
      return robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_UINT64];

   case VK_FORMAT_R32G32B32A32_UINT:
      return robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_UINT32];

   case VK_FORMAT_R16G16B16A16_UNORM:
   case VK_FORMAT_R16G16B16A16_USCALED:
   case VK_FORMAT_R16G16B16A16_UINT:
      return robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_UINT16];

   case VK_FORMAT_R8G8B8A8_UNORM:
   case VK_FORMAT_R8G8B8A8_USCALED:
   case VK_FORMAT_R8G8B8A8_UINT:
   case VK_FORMAT_R8G8B8A8_SRGB:
   case VK_FORMAT_B8G8R8A8_UNORM:
   case VK_FORMAT_B8G8R8A8_USCALED:
   case VK_FORMAT_B8G8R8A8_UINT:
   case VK_FORMAT_B8G8R8A8_SRGB:
      return robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_UINT8];

   case VK_FORMAT_R64G64B64A64_SINT:
      return robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_SINT64];

   case VK_FORMAT_R32G32B32A32_SINT:
      return robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_SINT32];

   case VK_FORMAT_R16G16B16A16_SNORM:
   case VK_FORMAT_R16G16B16A16_SSCALED:
   case VK_FORMAT_R16G16B16A16_SINT:
      return robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_SINT16];

   case VK_FORMAT_R8G8B8A8_SNORM:
   case VK_FORMAT_R8G8B8A8_SSCALED:
   case VK_FORMAT_R8G8B8A8_SINT:
   case VK_FORMAT_B8G8R8A8_SNORM:
   case VK_FORMAT_B8G8R8A8_SSCALED:
   case VK_FORMAT_B8G8R8A8_SINT:
      return robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_SINT8];

   case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
   case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
   case VK_FORMAT_A8B8G8R8_UINT_PACK32:
   case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
      return robustness_buffer_offsets
         [PVR_ROBUSTNESS_BUFFER_FORMAT_A8B8G8R8_UINT];

   case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
   case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
   case VK_FORMAT_A8B8G8R8_SINT_PACK32:
      return robustness_buffer_offsets
         [PVR_ROBUSTNESS_BUFFER_FORMAT_A8B8G8R8_SINT];

   case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
   case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
   case VK_FORMAT_A2R10G10B10_UINT_PACK32:
   case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
   case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
   case VK_FORMAT_A2B10G10R10_UINT_PACK32:
      return robustness_buffer_offsets
         [PVR_ROBUSTNESS_BUFFER_FORMAT_A2R10G10B10_UINT];

   case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
   case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
   case VK_FORMAT_A2R10G10B10_SINT_PACK32:
   case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
   case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
   case VK_FORMAT_A2B10G10R10_SINT_PACK32:
      return robustness_buffer_offsets
         [PVR_ROBUSTNESS_BUFFER_FORMAT_A2R10G10B10_SINT];

   case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
   case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
      return robustness_buffer_offsets
         [PVR_ROBUSTNESS_BUFFER_FORMAT_R4G4B4A4_UNORM];

   case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
   case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
      return robustness_buffer_offsets
         [PVR_ROBUSTNESS_BUFFER_FORMAT_R5G5B5A1_UNORM];

   case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
      return robustness_buffer_offsets
         [PVR_ROBUSTNESS_BUFFER_FORMAT_A1R5G5B5_UNORM];

   default:
      return robustness_buffer_offsets[PVR_ROBUSTNESS_BUFFER_FORMAT_UINT64];
   }
}
