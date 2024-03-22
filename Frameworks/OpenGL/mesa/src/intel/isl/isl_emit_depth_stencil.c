/*
 * Copyright 2016 Intel Corporation
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice (including the next
 *  paragraph) shall be included in all copies or substantial portions of the
 *  Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#include <stdint.h>

#define __gen_address_type uint64_t
#define __gen_user_data void

static uint64_t
__gen_combine_address(__attribute__((unused)) void *data,
                      __attribute__((unused)) void *loc, uint64_t addr,
                      uint32_t delta)
{
   return addr + delta;
}

#include "genxml/gen_macros.h"
#include "genxml/genX_pack.h"

#include "isl_priv.h"

static const uint32_t isl_encode_ds_surftype[] = {
#if GFX_VER >= 9
   /* From the SKL PRM, "3DSTATE_DEPTH_STENCIL::SurfaceType":
    *
    *    "If depth/stencil is enabled with 1D render target, depth/stencil
    *    surface type needs to be set to 2D surface type and height set to 1.
    *    Depth will use (legacy) TileY and stencil will use TileW. For this
    *    case only, the Surface Type of the depth buffer can be 2D while the
    *    Surface Type of the render target(s) are 1D, representing an
    *    exception to a programming note above.
    */
   [ISL_SURF_DIM_1D] = SURFTYPE_2D,
#else
   [ISL_SURF_DIM_1D] = SURFTYPE_1D,
#endif
   [ISL_SURF_DIM_2D] = SURFTYPE_2D,
   [ISL_SURF_DIM_3D] = SURFTYPE_3D,
};

#if GFX_VER >= 9
static const uint8_t isl_encode_tiling[] = {
#if GFX_VERx10 >= 125
   [ISL_TILING_4]          = TILE4,
   [ISL_TILING_64]         = TILE64,
#else
   [ISL_TILING_Y0]         = NONE,
   [ISL_TILING_SKL_Yf]     = TILEYF,
   [ISL_TILING_SKL_Ys]     = TILEYS,
   [ISL_TILING_ICL_Yf]     = TILEYF,
   [ISL_TILING_ICL_Ys]     = TILEYS,
#endif
};
#endif /* GFX_VER >= 9 */

void
isl_genX(emit_depth_stencil_hiz_s)(const struct isl_device *dev, void *batch,
                                   const struct isl_depth_stencil_hiz_emit_info *restrict info)
{
   if (info->depth_surf && info->stencil_surf) {
      if (!dev->info->has_hiz_and_separate_stencil) {
         assert(info->depth_surf == info->stencil_surf);
         assert(info->depth_address == info->stencil_address);
      }
      assert(info->depth_surf->dim == info->stencil_surf->dim);
   }

   if (info->depth_surf) {
      assert((info->depth_surf->usage & ISL_SURF_USAGE_DEPTH_BIT));
      if (info->depth_surf->dim == ISL_SURF_DIM_3D) {
         assert(info->view->base_array_layer + info->view->array_len <=
                info->depth_surf->logical_level0_px.depth);
      } else {
         assert(info->view->base_array_layer + info->view->array_len <=
                info->depth_surf->logical_level0_px.array_len);
      }
   }

   if (info->stencil_surf) {
      assert((info->stencil_surf->usage & ISL_SURF_USAGE_STENCIL_BIT));
      if (info->stencil_surf->dim == ISL_SURF_DIM_3D) {
         assert(info->view->base_array_layer + info->view->array_len <=
                info->stencil_surf->logical_level0_px.depth);
      } else {
         assert(info->view->base_array_layer + info->view->array_len <=
                info->stencil_surf->logical_level0_px.array_len);
      }
   }

   struct GENX(3DSTATE_DEPTH_BUFFER) db = {
      GENX(3DSTATE_DEPTH_BUFFER_header),
#if GFX_VER >= 6
      .MOCS = info->mocs,
#endif
   };

   if (info->depth_surf) {
      db.SurfaceType = isl_encode_ds_surftype[info->depth_surf->dim];
      db.SurfaceFormat = isl_surf_get_depth_format(dev, info->depth_surf);
      db.Width = info->depth_surf->logical_level0_px.width - 1;
      db.Height = info->depth_surf->logical_level0_px.height - 1;
      if (db.SurfaceType == SURFTYPE_3D)
         db.Depth = info->depth_surf->logical_level0_px.depth - 1;
   } else if (info->stencil_surf) {
      /* On Gfx12+ 3DSTATE_STENCIL_BUFFER has its own fields for all of
       * this. No need to replicate it here.
       */
#if GFX_VER < 12
      db.SurfaceType = isl_encode_ds_surftype[info->stencil_surf->dim];
      db.SurfaceFormat = D32_FLOAT;
      db.Width = info->stencil_surf->logical_level0_px.width - 1;
      db.Height = info->stencil_surf->logical_level0_px.height - 1;
      if (db.SurfaceType == SURFTYPE_3D)
         db.Depth = info->stencil_surf->logical_level0_px.depth - 1;
#else
      db.SurfaceType = SURFTYPE_NULL;
      db.SurfaceFormat = D32_FLOAT;
#endif
   } else {
      db.SurfaceType = SURFTYPE_NULL;
      db.SurfaceFormat = D32_FLOAT;
   }

   if (info->depth_surf || info->stencil_surf) {
      /* These are based entirely on the view */
      db.RenderTargetViewExtent = info->view->array_len - 1;
      db.LOD                  = info->view->base_level;
      db.MinimumArrayElement  = info->view->base_array_layer;

      /* From the Haswell PRM docs for 3DSTATE_DEPTH_BUFFER::Depth
       *
       *    "This field specifies the total number of levels for a volume
       *    texture or the number of array elements allowed to be accessed
       *    starting at the Minimum Array Element for arrayed surfaces. If the
       *    volume texture is MIP-mapped, this field specifies the depth of
       *    the base MIP level."
       *
       * For 3D surfaces, we set it to the correct depth above.  For non-3D
       * surfaces, this is the same as RenderTargetViewExtent.
       */
      if (db.SurfaceType != SURFTYPE_3D)
         db.Depth = db.RenderTargetViewExtent;
   }

   if (info->depth_surf) {
#if GFX_VER >= 7
      db.DepthWriteEnable = true;
#endif
      db.SurfaceBaseAddress = info->depth_address;

#if GFX_VERx10 >= 125
      db.TiledMode = isl_encode_tiling[info->depth_surf->tiling];
      db.MipTailStartLOD = info->depth_surf->miptail_start_level;
#if GFX_VERx10 < 20
      db.CompressionMode = isl_aux_usage_has_ccs(info->hiz_usage);
#endif
      db.RenderCompressionFormat =
         isl_get_render_compression_format(info->depth_surf->format);
#elif GFX_VER >= 9
      /* Gen9+ depth is always Y-tiled but it may be Y0, Yf, or Ys. */
      assert(isl_tiling_is_any_y(info->depth_surf->tiling));
      db.TiledResourceMode = isl_encode_tiling[info->depth_surf->tiling];
      db.MipTailStartLOD = info->depth_surf->miptail_start_level;
#elif GFX_VER >= 7
      /* Gen7+ depth is always Y-tiled.  We don't even have a bit for it */
#else
      assert(info->depth_surf->tiling == ISL_TILING_Y0);
      db.TiledSurface = true;
      db.TileWalk = TILEWALK_YMAJOR;
      db.MIPMapLayoutMode = MIPLAYOUT_BELOW;
#endif

      db.SurfacePitch = info->depth_surf->row_pitch_B - 1;
#if GFX_VER >= 8
      db.SurfaceQPitch =
         isl_surf_get_array_pitch_el_rows(info->depth_surf) >> 2;
#endif

#if GFX_VER == 12
      db.ControlSurfaceEnable = db.DepthBufferCompressionEnable =
         isl_aux_usage_has_ccs(info->hiz_usage);
#endif
   }

#if GFX_VER == 5 || GFX_VER == 6
   const bool separate_stencil =
      info->stencil_surf && info->stencil_surf->format == ISL_FORMAT_R8_UINT;
   if (separate_stencil || info->hiz_usage == ISL_AUX_USAGE_HIZ) {
      assert(ISL_DEV_USE_SEPARATE_STENCIL(dev));
      /* From the IronLake PRM, Vol 2 Part 1:
       *
       *    3DSTATE_DEPTH_BUFFER::Separate Stencil Buffer Enable
       *    If this field is enabled, Hierarchical Depth Buffer Enable must
       *    also be enabled.
       *
       *    3DSTATE_DEPTH_BUFFER::Tiled Surface
       *    When Hierarchical Depth Buffer is enabled, this bit must be set.
       */
      db.SeparateStencilBufferEnable = true;
      db.HierarchicalDepthBufferEnable = true;
      db.TiledSurface = true;
   }
#endif

#if GFX_VER >= 6
   struct GENX(3DSTATE_STENCIL_BUFFER) sb = {
      GENX(3DSTATE_STENCIL_BUFFER_header),
      .MOCS = info->mocs,
   };
#else
#  define sb db
#endif

   if (info->stencil_surf) {
#if GFX_VER >= 7 && GFX_VER < 12
      db.StencilWriteEnable = true;
#endif
#if GFX_VERx10 >= 125
#if GFX_VER < 20
      sb.CompressionMode = isl_aux_usage_has_ccs(info->stencil_aux_usage);
      sb.RenderCompressionFormat =
         isl_get_render_compression_format(info->stencil_surf->format);
#else
      sb.CompressionFormat =
         isl_get_render_compression_format(info->stencil_surf->format);
#endif
#endif
#if GFX_VER >= 12
      sb.TiledMode = isl_encode_tiling[info->stencil_surf->tiling];
      sb.MipTailStartLOD = info->stencil_surf->miptail_start_level;
      sb.StencilWriteEnable = true;
      sb.SurfaceType = SURFTYPE_2D;
      sb.Width = info->stencil_surf->logical_level0_px.width - 1;
      sb.Height = info->stencil_surf->logical_level0_px.height - 1;
      sb.Depth = sb.RenderTargetViewExtent = info->view->array_len - 1;
      sb.SurfLOD = info->view->base_level;
      sb.MinimumArrayElement = info->view->base_array_layer;
      assert(info->stencil_aux_usage == ISL_AUX_USAGE_NONE ||
             info->stencil_aux_usage == ISL_AUX_USAGE_STC_CCS);
#if GFX_VER < 20
      sb.StencilCompressionEnable =
         info->stencil_aux_usage == ISL_AUX_USAGE_STC_CCS;
      sb.ControlSurfaceEnable = sb.StencilCompressionEnable;
#endif
#elif GFX_VERx10 >= 75
      sb.StencilBufferEnable = true;
#endif
      sb.SurfaceBaseAddress = info->stencil_address;
      sb.SurfacePitch = info->stencil_surf->row_pitch_B - 1;
#if GFX_VER >= 8
      sb.SurfaceQPitch =
         isl_surf_get_array_pitch_el_rows(info->stencil_surf) >> 2;
#endif
   } else {
#if GFX_VER >= 12
      sb.SurfaceType = SURFTYPE_NULL;

      /* The docs seem to indicate that if surf-type is null, then we may need
       * to match the depth-buffer value for `Depth`. It may be a
       * documentation bug, since the other fields don't require this.
       *
       * TODO: Confirm documentation and remove setting of `Depth` if not
       * required.
       */
      sb.Depth = db.Depth;
#endif
   }

#if GFX_VER >= 6
   struct GENX(3DSTATE_HIER_DEPTH_BUFFER) hiz = {
      GENX(3DSTATE_HIER_DEPTH_BUFFER_header),
      .MOCS = info->mocs,
   };
#if GFX_VER < 20
   struct GENX(3DSTATE_CLEAR_PARAMS) clear = {
      GENX(3DSTATE_CLEAR_PARAMS_header),
   };
#endif

   assert(info->hiz_usage == ISL_AUX_USAGE_NONE ||
          isl_aux_usage_has_hiz(info->hiz_usage));
   if (isl_aux_usage_has_hiz(info->hiz_usage)) {
      assert(GFX_VER >= 12 || info->hiz_usage == ISL_AUX_USAGE_HIZ);
      db.HierarchicalDepthBufferEnable = true;

      hiz.SurfaceBaseAddress = info->hiz_address;
      hiz.SurfacePitch = info->hiz_surf->row_pitch_B - 1;

#if GFX_VERx10 >= 125
      /* From 3DSTATE_HIER_DEPTH_BUFFER_BODY::TiledMode,
       *
       *    HZ buffer only supports Tile4 mode
       *
       * and from Bspec 47009, "Hierarchical Depth Buffer",
       *
       *    The format of the data in the hierarchical depth buffer is not
       *    documented here, as this surface needs only to be allocated by
       *    software.
       *
       * We choose to apply the second quote to the first. ISL describes HiZ
       * with a tiling that has the same extent as Tile4 (128Bx32), but a
       * different internal layout. This has two benefits: 1) it allows us to
       * have the correct allocation size and 2) we can continue to use a
       * tiling that was determined to exist on some prior platforms.
       */
      assert(info->hiz_surf->tiling == ISL_TILING_HIZ);
      hiz.TiledMode = TILE4;
#elif GFX_VERx10 >= 120
      /* From 3DSTATE_HIER_DEPTH_BUFFER_BODY::TiledMode,
       *
       *     HZ buffer only supports Tile Y mode.
       *
       * and
       *
       *    Value | Name
       *    ----------------------------------------
       *    0h    | No tiled resource (Tile Y Mode).
       */
      assert(info->hiz_surf->tiling == ISL_TILING_HIZ);
      hiz.TiledMode = NONE;
#endif

#if GFX_VER >= 12
      hiz.HierarchicalDepthBufferWriteThruEnable =
         info->hiz_usage == ISL_AUX_USAGE_HIZ_CCS_WT;

      /* The bspec docs for this bit are fairly unclear about exactly what is
       * and isn't supported with HiZ write-through.  It's fairly clear that
       * you can't sample from a multisampled depth buffer with CCS.  This
       * limitation isn't called out explicitly but the docs for the CCS_E
       * value of RENDER_SURFACE_STATE::AuxiliarySurfaceMode say:
       *
       *    "If Number of multisamples > 1, programming this value means MSAA
       *    compression is enabled for that surface. Auxiliary surface is MSC
       *    with tile y."
       *
       * Since this interpretation ignores whether the surface is
       * depth/stencil or not and since multisampled depth buffers use
       * ISL_MSAA_LAYOUT_INTERLEAVED which is incompatible with MCS
       * compression, this means that we can't even specify MSAA depth CCS in
       * RENDER_SURFACE_STATE::AuxiliarySurfaceMode.  The BSpec also says, for
       * 3DSTATE_HIER_DEPTH_BUFFER::HierarchicalDepthBufferWriteThruEnable,
       *
       *    "This bit must NOT be set for >1x MSAA modes, since sampler
       *    doesn't support sampling from >1x MSAA depth buffer."
       *
       * Again, this is all focused around what the sampler can do and not
       * what the depth hardware can do.
       *
       * Reading even more internal docs which can't be quoted here makes it
       * pretty clear that, even if it's not currently called out in the
       * BSpec, HiZ+CCS write-through isn't intended to work with MSAA and we
       * shouldn't try to use it.  Treat it as if it's disallowed even if the
       * BSpec doesn't explicitly document that.
       */
      if (hiz.HierarchicalDepthBufferWriteThruEnable)
         assert(info->depth_surf->samples == 1);
#endif

#if GFX_VER >= 8
      /* From the SKL PRM Vol2a:
       *
       *    The interpretation of this field is dependent on Surface Type
       *    as follows:
       *    - SURFTYPE_1D: distance in pixels between array slices
       *    - SURFTYPE_2D/CUBE: distance in rows between array slices
       *    - SURFTYPE_3D: distance in rows between R - slices
       *
       * Unfortunately, the docs aren't 100% accurate here.  They fail to
       * mention that the 1-D rule only applies to linear 1-D images.
       * Since depth and HiZ buffers are always tiled, they are treated as
       * 2-D images.  Prior to Sky Lake, this field is always in rows.
       */
      hiz.SurfaceQPitch =
         isl_surf_get_array_pitch_sa_rows(info->hiz_surf) >> 2;
#endif

#if GFX_VER < 20
      clear.DepthClearValueValid = true;
#if GFX_VER >= 8
      clear.DepthClearValue = info->depth_clear_value;
#else
      switch (info->depth_surf->format) {
      case ISL_FORMAT_R32_FLOAT: {
         union { float f; uint32_t u; } fu;
         fu.f = info->depth_clear_value;
         clear.DepthClearValue = fu.u;
         break;
      }
      case ISL_FORMAT_R24_UNORM_X8_TYPELESS:
         clear.DepthClearValue = info->depth_clear_value * ((1u << 24) - 1);
         break;
      case ISL_FORMAT_R16_UNORM:
         clear.DepthClearValue = info->depth_clear_value * ((1u << 16) - 1);
         break;
      default:
         unreachable("Invalid depth type");
      }
#endif
#endif
   }
#endif /* GFX_VER >= 6 */

   /* Pack everything into the batch */
   uint32_t *dw = batch;
   GENX(3DSTATE_DEPTH_BUFFER_pack)(NULL, dw, &db);
   dw += GENX(3DSTATE_DEPTH_BUFFER_length);

#if GFX_VER >= 6
   GENX(3DSTATE_STENCIL_BUFFER_pack)(NULL, dw, &sb);
   dw += GENX(3DSTATE_STENCIL_BUFFER_length);

   GENX(3DSTATE_HIER_DEPTH_BUFFER_pack)(NULL, dw, &hiz);
   dw += GENX(3DSTATE_HIER_DEPTH_BUFFER_length);

#if GFX_VER < 20
   GENX(3DSTATE_CLEAR_PARAMS_pack)(NULL, dw, &clear);
   dw += GENX(3DSTATE_CLEAR_PARAMS_length);
#endif /* GFX_VER < 20 */
#endif /* GFX_VER >= 6 */
}
