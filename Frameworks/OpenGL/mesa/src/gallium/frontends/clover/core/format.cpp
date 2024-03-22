//
// Copyright 2012 Francisco Jerez
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//

#include "core/format.hpp"
#include "core/memory.hpp"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"

namespace clover {
   // see table 16 and 17 in the 3.0 CL spec under "5.3.1.1. Image Format Descriptor"
   // TODO optional channel orders:
   //  * CL_Rx
   //  * CL_RGx
   //  * CL_RGBx
   //  * CL_sRGBx
   #define _FF(c, b, g) \
      { { CL_R, c }, PIPE_FORMAT_R##b##_##g },                      \
      { { CL_A, c }, PIPE_FORMAT_A##b##_##g },                      \
      { { CL_RG, c }, PIPE_FORMAT_R##b##G##b##_##g },               \
      { { CL_RA, c }, PIPE_FORMAT_R##b##A##b##_##g },               \
      { { CL_RGB, c }, PIPE_FORMAT_R##b##G##b##B##b##_##g },        \
      { { CL_RGBA, c }, PIPE_FORMAT_R##b##G##b##B##b##A##b##_##g }
      // broken but also optional
      //{ { CL_LUMINANCE, c }, PIPE_FORMAT_L##b##_##g },
      //{ { CL_INTENSITY, c }, PIPE_FORMAT_I##b##_##g },

   #define _FI(c, b, g) \
      _FF(c##b, b, g)

   static const std::map<cl_image_format, pipe_format> formats {
      //required in CL 2.0 but broken
      //_FI(CL_SNORM_INT, 8, SNORM),
      //_FI(CL_SNORM_INT, 16, SNORM),
      _FI(CL_UNORM_INT, 8, UNORM),
      _FI(CL_UNORM_INT, 16, UNORM),
      _FI(CL_SIGNED_INT, 8, SINT),
      _FI(CL_SIGNED_INT, 16, SINT),
      _FI(CL_SIGNED_INT, 32, SINT),
      _FI(CL_UNSIGNED_INT, 8, UINT),
      _FI(CL_UNSIGNED_INT, 16, UINT),
      _FI(CL_UNSIGNED_INT, 32, UINT),
      _FF(CL_HALF_FLOAT, 16, FLOAT),
      _FF(CL_FLOAT, 32, FLOAT),

      // TODO: next three can be CL_RGBx as well
      { { CL_RGB, CL_UNORM_SHORT_565 }, PIPE_FORMAT_B5G6R5_UNORM },
      { { CL_RGB, CL_UNORM_SHORT_555 }, PIPE_FORMAT_B5G5R5A1_UNORM },
      { { CL_RGB, CL_UNORM_INT_101010 }, PIPE_FORMAT_B10G10R10X2_UNORM },

      { { CL_RGBA, CL_UNORM_INT_101010_2 }, PIPE_FORMAT_B10G10R10A2_UNORM },

      { { CL_ARGB, CL_UNORM_INT8 }, PIPE_FORMAT_A8R8G8B8_UNORM },
      { { CL_ARGB, CL_UNSIGNED_INT8 }, PIPE_FORMAT_A8R8G8B8_UINT },

      { { CL_BGRA, CL_SNORM_INT8 }, PIPE_FORMAT_B8G8R8A8_SNORM },
      { { CL_BGRA, CL_UNORM_INT8 }, PIPE_FORMAT_B8G8R8A8_UNORM },
      { { CL_BGRA, CL_SIGNED_INT8 }, PIPE_FORMAT_B8G8R8A8_SINT },
      { { CL_BGRA, CL_UNSIGNED_INT8 }, PIPE_FORMAT_B8G8R8A8_UINT },

      { { CL_ABGR, CL_SNORM_INT8 }, PIPE_FORMAT_A8B8G8R8_SNORM },
      { { CL_ABGR, CL_UNORM_INT8 }, PIPE_FORMAT_A8B8G8R8_UNORM },
      { { CL_ABGR, CL_SIGNED_INT8 }, PIPE_FORMAT_A8B8G8R8_SINT },
      { { CL_ABGR, CL_UNSIGNED_INT8 }, PIPE_FORMAT_A8B8G8R8_UINT },

      // disable for now as it needs CL C 2.0 support
      //{ { CL_DEPTH, CL_UNORM_INT16 }, PIPE_FORMAT_Z16_UNORM },
      //{ { CL_DEPTH, CL_FLOAT }, PIPE_FORMAT_Z32_FLOAT },

      // required in CL 2.0 but broken
      //{ { CL_sRGBA, CL_UNORM_INT8 }, PIPE_FORMAT_R8G8B8A8_SRGB },
      // optional but broken
      //{ { CL_sRGB, CL_UNORM_INT8 }, PIPE_FORMAT_R8G8B8_SRGB },
      //{ { CL_sBGRA, CL_UNORM_INT8 }, PIPE_FORMAT_B8G8R8A8_SRGB },
   };
   #undef _FF
   #undef _FI

   pipe_texture_target
   translate_target(cl_mem_object_type type) {
      switch (type) {
      case CL_MEM_OBJECT_BUFFER:
      case CL_MEM_OBJECT_IMAGE1D_BUFFER:
         return PIPE_BUFFER;
      case CL_MEM_OBJECT_IMAGE1D:
         return PIPE_TEXTURE_1D;
      case CL_MEM_OBJECT_IMAGE2D:
         return PIPE_TEXTURE_2D;
      case CL_MEM_OBJECT_IMAGE3D:
         return PIPE_TEXTURE_3D;
      case CL_MEM_OBJECT_IMAGE1D_ARRAY:
         return PIPE_TEXTURE_1D_ARRAY;
      case CL_MEM_OBJECT_IMAGE2D_ARRAY:
         return PIPE_TEXTURE_2D_ARRAY;
      default:
         throw error(CL_INVALID_VALUE);
      }
   }

   pipe_format
   translate_format(const cl_image_format &format) {
      auto it = formats.find(format);

      if (it == formats.end())
         throw error(CL_IMAGE_FORMAT_NOT_SUPPORTED);

      return it->second;
   }

   std::set<cl_image_format>
   supported_formats(const context &ctx, cl_mem_object_type type, cl_mem_flags flags) {
      std::set<cl_image_format> s;
      pipe_texture_target target = translate_target(type);
      unsigned bindings = 0;

      if (flags & (CL_MEM_READ_ONLY | CL_MEM_READ_WRITE | CL_MEM_KERNEL_READ_AND_WRITE))
         bindings |= PIPE_BIND_SAMPLER_VIEW;
      if (flags & (CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE | CL_MEM_KERNEL_READ_AND_WRITE))
         bindings |= PIPE_BIND_SHADER_IMAGE;

      for (auto f : formats) {
         if (all_of([=](const device &dev) {
                  return dev.pipe->is_format_supported(
                     dev.pipe, f.second, target, 1, 1, bindings);
               }, ctx.devices()))
            s.insert(f.first);
      }

      return s;
   }
}
