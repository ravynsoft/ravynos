/*
 * Copyright 2015 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "isl_priv.h"
#include "compiler/brw_compiler.h"

bool
isl_is_storage_image_format(const struct intel_device_info *devinfo,
                            enum isl_format format)
{
   /* XXX: Maybe we should put this in the CSV? */

   switch (format) {
   case ISL_FORMAT_R32G32B32A32_UINT:
   case ISL_FORMAT_R32G32B32A32_SINT:
   case ISL_FORMAT_R32G32B32A32_FLOAT:
   case ISL_FORMAT_R32_UINT:
   case ISL_FORMAT_R32_SINT:
   case ISL_FORMAT_R32_FLOAT:
   case ISL_FORMAT_R16G16B16A16_UINT:
   case ISL_FORMAT_R16G16B16A16_SINT:
   case ISL_FORMAT_R16G16B16A16_FLOAT:
   case ISL_FORMAT_R32G32_UINT:
   case ISL_FORMAT_R32G32_SINT:
   case ISL_FORMAT_R32G32_FLOAT:
   case ISL_FORMAT_R8G8B8A8_UINT:
   case ISL_FORMAT_R8G8B8A8_SINT:
   case ISL_FORMAT_R16G16_UINT:
   case ISL_FORMAT_R16G16_SINT:
   case ISL_FORMAT_R16G16_FLOAT:
   case ISL_FORMAT_R8G8_UINT:
   case ISL_FORMAT_R8G8_SINT:
   case ISL_FORMAT_R16_UINT:
   case ISL_FORMAT_R16_FLOAT:
   case ISL_FORMAT_R16_SINT:
   case ISL_FORMAT_R8_UINT:
   case ISL_FORMAT_R8_SINT:
   case ISL_FORMAT_R10G10B10A2_UINT:
   case ISL_FORMAT_R10G10B10A2_UNORM:
   case ISL_FORMAT_R11G11B10_FLOAT:
   case ISL_FORMAT_R16G16B16A16_UNORM:
   case ISL_FORMAT_R16G16B16A16_SNORM:
   case ISL_FORMAT_R8G8B8A8_UNORM:
   case ISL_FORMAT_R8G8B8A8_SNORM:
   case ISL_FORMAT_R16G16_UNORM:
   case ISL_FORMAT_R16G16_SNORM:
   case ISL_FORMAT_R8G8_UNORM:
   case ISL_FORMAT_R8G8_SNORM:
   case ISL_FORMAT_R16_UNORM:
   case ISL_FORMAT_R16_SNORM:
   case ISL_FORMAT_R8_UNORM:
   case ISL_FORMAT_R8_SNORM:
      return true;
   default:
      return false;
   }
}

enum isl_format
isl_lower_storage_image_format(const struct intel_device_info *devinfo,
                               enum isl_format format)
{
   switch (format) {
   /* These are never lowered.  Up to BDW we'll have to fall back to untyped
    * surface access for 128bpp formats.
    */
   case ISL_FORMAT_R32G32B32A32_UINT:
   case ISL_FORMAT_R32G32B32A32_SINT:
   case ISL_FORMAT_R32G32B32A32_FLOAT:
   case ISL_FORMAT_R32_UINT:
   case ISL_FORMAT_R32_SINT:
      return format;

   /* The Skylake PRM's "Surface Formats" section says:
    *
    *   "The surface format for the typed atomic integer operations must
    *    be R32_UINT or R32_SINT."
    *
    * But checking the BSpec 1706, you find a different restriction. There the
    * wording is :
    *
    *    "The surface format must be one of R32_UINT, R32_SINT or R32_FLOAT"
    *
    * The confusion is probably related to atomic integer messages. For
    * example an IADD instruction would require a R32_UINT/R32_SINT surface.
    * But a CMPXCHG instruction does not really care about the type, it just
    * does bit to bit comparison and swap.
    *
    * The confusion seems to have propagated to the simulation environment.
    * Gfx12 has the same restrictions as Gfx11 regarding doing a CMPXCHG on a
    * R32_FLOAT surface, but the Gfx11 environment will report an error while
    * Gfx12 passes fine. More importantly HW doesn't seem to mind.
    */
   case ISL_FORMAT_R32_FLOAT:
      return format;

   /* From HSW to BDW the only 64bpp format supported for typed access is
    * RGBA_UINT16.  IVB falls back to untyped.
    */
   case ISL_FORMAT_R16G16B16A16_UINT:
   case ISL_FORMAT_R16G16B16A16_SINT:
   case ISL_FORMAT_R16G16B16A16_FLOAT:
   case ISL_FORMAT_R32G32_UINT:
   case ISL_FORMAT_R32G32_SINT:
   case ISL_FORMAT_R32G32_FLOAT:
      return (devinfo->ver >= 9 ? format :
              devinfo->verx10 >= 75 ?
              ISL_FORMAT_R16G16B16A16_UINT :
              ISL_FORMAT_R32G32_UINT);

   /* Up to BDW no SINT or FLOAT formats of less than 32 bits per component
    * are supported.  IVB doesn't support formats with more than one component
    * for typed access.  For 8 and 16 bpp formats IVB relies on the
    * undocumented behavior that typed reads from R_UINT8 and R_UINT16
    * surfaces actually do a 32-bit misaligned read.  The alternative would be
    * to use two surface state entries with different formats for each image,
    * one for reading (using R_UINT32) and another one for writing (using
    * R_UINT8 or R_UINT16), but that would complicate the shaders we generate
    * even more.
    */
   case ISL_FORMAT_R8G8B8A8_UINT:
   case ISL_FORMAT_R8G8B8A8_SINT:
      return (devinfo->ver >= 9 ? format :
              devinfo->verx10 >= 75 ?
              ISL_FORMAT_R8G8B8A8_UINT : ISL_FORMAT_R32_UINT);

   case ISL_FORMAT_R16G16_UINT:
   case ISL_FORMAT_R16G16_SINT:
   case ISL_FORMAT_R16G16_FLOAT:
      return (devinfo->ver >= 9 ? format :
              devinfo->verx10 >= 75 ?
              ISL_FORMAT_R16G16_UINT : ISL_FORMAT_R32_UINT);

   case ISL_FORMAT_R8G8_UINT:
   case ISL_FORMAT_R8G8_SINT:
      return (devinfo->ver >= 9 ? format :
              devinfo->verx10 >= 75 ?
              ISL_FORMAT_R8G8_UINT : ISL_FORMAT_R16_UINT);

   case ISL_FORMAT_R16_UINT:
   case ISL_FORMAT_R16_FLOAT:
   case ISL_FORMAT_R16_SINT:
      return (devinfo->ver >= 9 ? format : ISL_FORMAT_R16_UINT);

   case ISL_FORMAT_R8_UINT:
   case ISL_FORMAT_R8_SINT:
      return (devinfo->ver >= 9 ? format : ISL_FORMAT_R8_UINT);

   /* Here the PRMs are a bit out of date. But according to BSpec 47635
    * (Gfx12.5), the 2/10/10/10 and the 11/11/10 packed formats are supported
    * by the hardware.
    */
   case ISL_FORMAT_R10G10B10A2_UINT:
   case ISL_FORMAT_R10G10B10A2_UNORM:
   case ISL_FORMAT_R11G11B10_FLOAT:
      return devinfo->verx10 >= 125 ? format : ISL_FORMAT_R32_UINT;

   /* No normalized fixed-point formats are supported by the hardware until Gfx11. */
   case ISL_FORMAT_R16G16B16A16_UNORM:
   case ISL_FORMAT_R16G16B16A16_SNORM:
      if (devinfo->ver >= 11)
         return format;
      if (devinfo->ver >= 9)
         return ISL_FORMAT_R32G32_UINT;
      if (devinfo->verx10 >= 75)
         return ISL_FORMAT_R16G16B16A16_UINT;
      return ISL_FORMAT_R32G32_UINT;

   case ISL_FORMAT_R8G8B8A8_UNORM:
   case ISL_FORMAT_R8G8B8A8_SNORM:
      if (devinfo->ver >= 11)
         return format;
      if (devinfo->ver >= 9)
         return ISL_FORMAT_R32_UINT;
      if (devinfo->verx10 >= 75)
         return ISL_FORMAT_R8G8B8A8_UINT;
      return ISL_FORMAT_R32_UINT;

   case ISL_FORMAT_R16G16_UNORM:
   case ISL_FORMAT_R16G16_SNORM:
      if (devinfo->ver >= 11)
         return format;
      if (devinfo->ver >= 9)
         return ISL_FORMAT_R32_UINT;
      if (devinfo->verx10 >= 75)
         return ISL_FORMAT_R16G16_UINT;
      return ISL_FORMAT_R32_UINT;

   case ISL_FORMAT_R8G8_UNORM:
   case ISL_FORMAT_R8G8_SNORM:
      if (devinfo->ver >= 11)
         return format;
      if (devinfo->ver >= 9)
         return ISL_FORMAT_R16_UINT;
      if (devinfo->verx10 >= 75)
         return ISL_FORMAT_R8G8_UINT;
      return ISL_FORMAT_R16_UINT;

   case ISL_FORMAT_R16_UNORM:
   case ISL_FORMAT_R16_SNORM:
      return (devinfo->ver >= 11 ? format : ISL_FORMAT_R16_UINT);

   case ISL_FORMAT_R8_UNORM:
   case ISL_FORMAT_R8_SNORM:
      return (devinfo->ver >= 11 ? format : ISL_FORMAT_R8_UINT);

   default:
      assert(!"Unknown image format");
      return ISL_FORMAT_UNSUPPORTED;
   }
}

bool
isl_has_matching_typed_storage_image_format(const struct intel_device_info *devinfo,
                                            enum isl_format fmt)
{
   if (devinfo->ver >= 9) {
      return true;
   } else if (devinfo->verx10 >= 75) {
      return isl_format_get_layout(fmt)->bpb <= 64;
   } else {
      return isl_format_get_layout(fmt)->bpb <= 32;
   }
}

static const struct brw_image_param image_param_defaults = {
   /* Set the swizzling shifts to all-ones to effectively disable
    * swizzling -- See emit_address_calculation() in
    * brw_fs_surface_builder.cpp for a more detailed explanation of
    * these parameters.
    */
   .swizzling = { 0xff, 0xff },
};

void
isl_surf_fill_image_param(const struct isl_device *dev,
                          struct brw_image_param *param,
                          const struct isl_surf *surf,
                          const struct isl_view *view)
{
   *param = image_param_defaults;

   if (surf->dim != ISL_SURF_DIM_3D) {
      assert(view->base_array_layer + view->array_len <=
             surf->logical_level0_px.array_len);
   }
   param->size[0] = isl_minify(surf->logical_level0_px.w, view->base_level);
   param->size[1] = surf->dim == ISL_SURF_DIM_1D ?
                    view->array_len :
                    isl_minify(surf->logical_level0_px.h, view->base_level);
   param->size[2] = surf->dim == ISL_SURF_DIM_2D ?
                    view->array_len :
                    isl_minify(surf->logical_level0_px.d, view->base_level);

   uint32_t tile_z_el, phys_array_layer;
   isl_surf_get_image_offset_el(surf, view->base_level,
                                surf->dim == ISL_SURF_DIM_3D ?
                                   0 : view->base_array_layer,
                                surf->dim == ISL_SURF_DIM_3D ?
                                   view->base_array_layer : 0,
                                &param->offset[0],  &param->offset[1],
                                &tile_z_el, &phys_array_layer);
   assert(tile_z_el == 0);
   assert(phys_array_layer == 0);

   const int cpp = isl_format_get_layout(surf->format)->bpb / 8;
   param->stride[0] = cpp;
   param->stride[1] = surf->row_pitch_B / cpp;

   const struct isl_extent3d image_align_sa =
      isl_surf_get_image_alignment_sa(surf);
   if (ISL_GFX_VER(dev) < 9 && surf->dim == ISL_SURF_DIM_3D) {
      param->stride[2] = isl_align_npot(param->size[0], image_align_sa.w);
      param->stride[3] = isl_align_npot(param->size[1], image_align_sa.h);
   } else {
      param->stride[2] = 0;
      param->stride[3] = isl_surf_get_array_pitch_el_rows(surf);
   }

   switch (surf->tiling) {
   case ISL_TILING_LINEAR:
      /* image_param_defaults is good enough */
      break;

   case ISL_TILING_X:
      /* An X tile is a rectangular block of 512x8 bytes. */
      param->tiling[0] = isl_log2u(512 / cpp);
      param->tiling[1] = isl_log2u(8);

      if (dev->has_bit6_swizzling) {
         /* Right shifts required to swizzle bits 9 and 10 of the memory
          * address with bit 6.
          */
         param->swizzling[0] = 3;
         param->swizzling[1] = 4;
      }
      break;

   case ISL_TILING_Y0:
      /* The layout of a Y-tiled surface in memory isn't really fundamentally
       * different to the layout of an X-tiled surface, we simply pretend that
       * the surface is broken up in a number of smaller 16Bx32 tiles, each
       * one arranged in X-major order just like is the case for X-tiling.
       */
      param->tiling[0] = isl_log2u(16 / cpp);
      param->tiling[1] = isl_log2u(32);

      if (dev->has_bit6_swizzling) {
         /* Right shift required to swizzle bit 9 of the memory address with
          * bit 6.
          */
         param->swizzling[0] = 3;
         param->swizzling[1] = 0xff;
      }
      break;

   default:
      assert(!"Unhandled storage image tiling");
   }

   /* 3D textures are arranged in 2D in memory with 2^lod slices per row.  The
    * address calculation algorithm (emit_address_calculation() in
    * brw_fs_surface_builder.cpp) handles this as a sort of tiling with
    * modulus equal to the LOD.
    */
   param->tiling[2] = (ISL_GFX_VER(dev) < 9 && surf->dim == ISL_SURF_DIM_3D ?
                       view->base_level : 0);
}

void
isl_buffer_fill_image_param(const struct isl_device *dev,
                            struct brw_image_param *param,
                            enum isl_format format,
                            uint64_t size)
{
   *param = image_param_defaults;

   param->stride[0] = isl_format_get_layout(format)->bpb / 8;
   param->size[0] = size / param->stride[0];
}
