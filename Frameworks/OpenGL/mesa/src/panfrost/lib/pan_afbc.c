/*
 * Copyright (C) 2019 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *   Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

#include "pan_texture.h"

/* Arm FrameBuffer Compression (AFBC) is a lossless compression scheme natively
 * implemented in Mali GPUs (as well as many display controllers paired with
 * Mali GPUs, etc). Where possible, Panfrost prefers to use AFBC for both
 * rendering and texturing. In most cases, this is a performance-win due to a
 * dramatic reduction in memory bandwidth and cache locality compared to a
 * linear resources.
 *
 * AFBC divides the framebuffer into 16x16 tiles (other sizes possible, TODO:
 * do we need to support this?). So, the width and height each must be aligned
 * up to 16 pixels. This is inherently good for performance; note that for a 4
 * byte-per-pixel format like RGBA8888, that means that rows are 16*4=64 byte
 * aligned, which is the cache-line size.
 *
 * For each AFBC-compressed resource, there is a single contiguous
 * (CPU/GPU-shared) buffer. This buffer itself is divided into two parts:
 * header and body, placed immediately after each other.
 *
 * The AFBC header contains 16 bytes of metadata per tile.
 *
 * The AFBC body is the same size as the original linear resource (padded to
 * the nearest tile). Although the body comes immediately after the header, it
 * must also be cache-line aligned, so there can sometimes be a bit of padding
 * between the header and body.
 *
 * As an example, a 64x64 RGBA framebuffer contains 64/16 = 4 tiles horizontally
 * and 4 tiles vertically. There are 4*4=16 tiles in total, each containing 16
 * bytes of metadata, so there is a 16*16=256 byte header. 64x64 is already
 * tile aligned, so the body is 64*64 * 4 bytes per pixel = 16384 bytes of
 * body.
 *
 * From userspace, Panfrost needs to be able to calculate these sizes. It
 * explicitly does not and can not know the format of the data contained within
 * this header and body. The GPU has native support for AFBC encode/decode. For
 * an internal FBO or a framebuffer used for scanout with an AFBC-compatible
 * winsys/display-controller, the buffer is maintained AFBC throughout flight,
 * and the driver never needs to know the internal data. For edge cases where
 * the driver really does need to read/write from the AFBC resource, we
 * generate a linear staging buffer and use the GPU to blit AFBC<--->linear.
 */

static enum pipe_format
unswizzled_format(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_A8_UNORM:
   case PIPE_FORMAT_L8_UNORM:
   case PIPE_FORMAT_I8_UNORM:
      return PIPE_FORMAT_R8_UNORM;

   case PIPE_FORMAT_L8A8_UNORM:
      return PIPE_FORMAT_R8G8_UNORM;

   case PIPE_FORMAT_B8G8R8_UNORM:
      return PIPE_FORMAT_R8G8B8_UNORM;

   case PIPE_FORMAT_R8G8B8X8_UNORM:
   case PIPE_FORMAT_B8G8R8A8_UNORM:
   case PIPE_FORMAT_B8G8R8X8_UNORM:
   case PIPE_FORMAT_A8R8G8B8_UNORM:
   case PIPE_FORMAT_X8R8G8B8_UNORM:
   case PIPE_FORMAT_X8B8G8R8_UNORM:
   case PIPE_FORMAT_A8B8G8R8_UNORM:
      return PIPE_FORMAT_R8G8B8A8_UNORM;

   case PIPE_FORMAT_B5G6R5_UNORM:
      return PIPE_FORMAT_R5G6B5_UNORM;

   case PIPE_FORMAT_B5G5R5A1_UNORM:
      return PIPE_FORMAT_R5G5B5A1_UNORM;

   case PIPE_FORMAT_R10G10B10X2_UNORM:
   case PIPE_FORMAT_B10G10R10A2_UNORM:
   case PIPE_FORMAT_B10G10R10X2_UNORM:
      return PIPE_FORMAT_R10G10B10A2_UNORM;

   case PIPE_FORMAT_A4B4G4R4_UNORM:
   case PIPE_FORMAT_B4G4R4A4_UNORM:
      return PIPE_FORMAT_R4G4B4A4_UNORM;

   default:
      return format;
   }
}

/* AFBC supports compressing a few canonical formats. Additional formats are
 * available by using a canonical internal format. Given a PIPE format, find
 * the canonical AFBC internal format if it exists, or NONE if the format
 * cannot be compressed. */

enum pan_afbc_mode
panfrost_afbc_format(unsigned arch, enum pipe_format format)
{
   /* sRGB does not change the pixel format itself, only the
    * interpretation. The interpretation is handled by conversion hardware
    * independent to the compression hardware, so we can compress sRGB
    * formats by using the corresponding linear format.
    */
   format = util_format_linear(format);

   /* Luminance-alpha not supported for AFBC on v7+ */
   switch (format) {
   case PIPE_FORMAT_A8_UNORM:
   case PIPE_FORMAT_L8_UNORM:
   case PIPE_FORMAT_I8_UNORM:
   case PIPE_FORMAT_L8A8_UNORM:
      if (arch >= 7)
         return PAN_AFBC_MODE_INVALID;
      else
         break;
   default:
      break;
   }

   /* We handle swizzling orthogonally to AFBC */
   format = unswizzled_format(format);

   /* clang-format off */
   switch (format) {
   case PIPE_FORMAT_R8_UNORM:          return PAN_AFBC_MODE_R8;
   case PIPE_FORMAT_R8G8_UNORM:        return PAN_AFBC_MODE_R8G8;
   case PIPE_FORMAT_R8G8B8_UNORM:      return PAN_AFBC_MODE_R8G8B8;
   case PIPE_FORMAT_R8G8B8A8_UNORM:    return PAN_AFBC_MODE_R8G8B8A8;
   case PIPE_FORMAT_R5G6B5_UNORM:      return PAN_AFBC_MODE_R5G6B5;
   case PIPE_FORMAT_R5G5B5A1_UNORM:    return PAN_AFBC_MODE_R5G5B5A1;
   case PIPE_FORMAT_R10G10B10A2_UNORM: return PAN_AFBC_MODE_R10G10B10A2;
   case PIPE_FORMAT_R4G4B4A4_UNORM:    return PAN_AFBC_MODE_R4G4B4A4;
   case PIPE_FORMAT_Z16_UNORM:         return PAN_AFBC_MODE_R8G8;

   case PIPE_FORMAT_Z24_UNORM_S8_UINT: return PAN_AFBC_MODE_R8G8B8A8;
   case PIPE_FORMAT_Z24X8_UNORM:       return PAN_AFBC_MODE_R8G8B8A8;
   case PIPE_FORMAT_X24S8_UINT:        return PAN_AFBC_MODE_R8G8B8A8;

   default:                            return PAN_AFBC_MODE_INVALID;
   }
   /* clang-format on */
}

/* A format may be compressed as AFBC if it has an AFBC internal format */

bool
panfrost_format_supports_afbc(const struct panfrost_device *dev,
                              enum pipe_format format)
{
   return panfrost_afbc_format(dev->arch, format) != PAN_AFBC_MODE_INVALID;
}

/* The lossless colour transform (AFBC_FORMAT_MOD_YTR) requires RGB. */

bool
panfrost_afbc_can_ytr(enum pipe_format format)
{
   const struct util_format_description *desc = util_format_description(format);

   /* YTR is only defined for RGB(A) */
   if (desc->nr_channels != 3 && desc->nr_channels != 4)
      return false;

   /* The fourth channel if it exists doesn't matter */
   return desc->colorspace == UTIL_FORMAT_COLORSPACE_RGB;
}

/* Only support packing for RGB formats for now. */

bool
panfrost_afbc_can_pack(enum pipe_format format)
{
   const struct util_format_description *desc = util_format_description(format);

   if (desc->nr_channels != 1 && desc->nr_channels != 3 &&
       desc->nr_channels != 4)
      return false;

   return desc->colorspace == UTIL_FORMAT_COLORSPACE_RGB;
}

/*
 * Check if the device supports AFBC with tiled headers (and hence also solid
 * colour blocks).
 */
bool
panfrost_afbc_can_tile(const struct panfrost_device *dev)
{
   return (dev->arch >= 7);
}
