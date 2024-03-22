/*
 * Copyright © 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file iris_resource.c
 *
 * Resources are images, buffers, and other objects used by the GPU.
 *
 * XXX: explain resources
 */

#include <stdio.h>
#include <errno.h>
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "pipe/p_context.h"
#include "pipe/p_screen.h"
#include "util/os_memory.h"
#include "util/u_cpu_detect.h"
#include "util/u_inlines.h"
#include "util/format/u_format.h"
#include "util/u_memory.h"
#include "util/u_resource.h"
#include "util/u_threaded_context.h"
#include "util/u_transfer.h"
#include "util/u_transfer_helper.h"
#include "util/u_upload_mgr.h"
#include "util/ralloc.h"
#include "iris_batch.h"
#include "iris_context.h"
#include "iris_resource.h"
#include "iris_screen.h"
#include "intel/common/intel_aux_map.h"
#include "intel/dev/intel_debug.h"
#include "isl/isl.h"
#include "drm-uapi/drm_fourcc.h"
#include "drm-uapi/i915_drm.h"

enum modifier_priority {
   MODIFIER_PRIORITY_INVALID = 0,
   MODIFIER_PRIORITY_LINEAR,
   MODIFIER_PRIORITY_X,
   MODIFIER_PRIORITY_Y,
   MODIFIER_PRIORITY_Y_CCS,
   MODIFIER_PRIORITY_Y_GFX12_RC_CCS,
   MODIFIER_PRIORITY_Y_GFX12_RC_CCS_CC,
   MODIFIER_PRIORITY_4,
   MODIFIER_PRIORITY_4_DG2_RC_CCS,
   MODIFIER_PRIORITY_4_DG2_RC_CCS_CC,
   MODIFIER_PRIORITY_4_MTL_RC_CCS,
   MODIFIER_PRIORITY_4_MTL_RC_CCS_CC,
};

static const uint64_t priority_to_modifier[] = {
   [MODIFIER_PRIORITY_INVALID] = DRM_FORMAT_MOD_INVALID,
   [MODIFIER_PRIORITY_LINEAR] = DRM_FORMAT_MOD_LINEAR,
   [MODIFIER_PRIORITY_X] = I915_FORMAT_MOD_X_TILED,
   [MODIFIER_PRIORITY_Y] = I915_FORMAT_MOD_Y_TILED,
   [MODIFIER_PRIORITY_Y_CCS] = I915_FORMAT_MOD_Y_TILED_CCS,
   [MODIFIER_PRIORITY_Y_GFX12_RC_CCS] = I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS,
   [MODIFIER_PRIORITY_Y_GFX12_RC_CCS_CC] = I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS_CC,
   [MODIFIER_PRIORITY_4] = I915_FORMAT_MOD_4_TILED,
   [MODIFIER_PRIORITY_4_DG2_RC_CCS] = I915_FORMAT_MOD_4_TILED_DG2_RC_CCS,
   [MODIFIER_PRIORITY_4_DG2_RC_CCS_CC] = I915_FORMAT_MOD_4_TILED_DG2_RC_CCS_CC,
   [MODIFIER_PRIORITY_4_MTL_RC_CCS] = I915_FORMAT_MOD_4_TILED_MTL_RC_CCS,
   [MODIFIER_PRIORITY_4_MTL_RC_CCS_CC] = I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC,
};

static bool
modifier_is_supported(const struct intel_device_info *devinfo,
                      enum pipe_format pfmt, unsigned bind,
                      uint64_t modifier)
{
   /* Check for basic device support. */
   switch (modifier) {
   case DRM_FORMAT_MOD_LINEAR:
   case I915_FORMAT_MOD_X_TILED:
      break;
   case I915_FORMAT_MOD_Y_TILED:
      if (devinfo->ver <= 8 && (bind & PIPE_BIND_SCANOUT))
         return false;
      if (devinfo->verx10 >= 125)
         return false;
      break;
   case I915_FORMAT_MOD_Y_TILED_CCS:
      if (devinfo->ver <= 8 || devinfo->ver >= 12)
         return false;
      break;
   case I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS:
   case I915_FORMAT_MOD_Y_TILED_GEN12_MC_CCS:
   case I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS_CC:
      if (devinfo->verx10 != 120)
         return false;
      break;
   case I915_FORMAT_MOD_4_TILED:
      if (devinfo->verx10 < 125)
         return false;
      break;
   case I915_FORMAT_MOD_4_TILED_DG2_RC_CCS:
   case I915_FORMAT_MOD_4_TILED_DG2_MC_CCS:
   case I915_FORMAT_MOD_4_TILED_DG2_RC_CCS_CC:
      if (!intel_device_info_is_dg2(devinfo))
         return false;
      break;
   case I915_FORMAT_MOD_4_TILED_MTL_RC_CCS:
   case I915_FORMAT_MOD_4_TILED_MTL_MC_CCS:
   case I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC:
      if (!intel_device_info_is_mtl(devinfo))
         return false;
      break;
   case DRM_FORMAT_MOD_INVALID:
   default:
      return false;
   }

   bool no_ccs = INTEL_DEBUG(DEBUG_NO_CCS) || (bind & PIPE_BIND_CONST_BW);

   /* Check remaining requirements. */
   switch (modifier) {
   case I915_FORMAT_MOD_4_TILED_MTL_MC_CCS:
   case I915_FORMAT_MOD_4_TILED_DG2_MC_CCS:
   case I915_FORMAT_MOD_Y_TILED_GEN12_MC_CCS:
      if (no_ccs)
         return false;

      if (pfmt != PIPE_FORMAT_BGRA8888_UNORM &&
          pfmt != PIPE_FORMAT_RGBA8888_UNORM &&
          pfmt != PIPE_FORMAT_BGRX8888_UNORM &&
          pfmt != PIPE_FORMAT_RGBX8888_UNORM &&
          pfmt != PIPE_FORMAT_NV12 &&
          pfmt != PIPE_FORMAT_P010 &&
          pfmt != PIPE_FORMAT_P012 &&
          pfmt != PIPE_FORMAT_P016 &&
          pfmt != PIPE_FORMAT_YUYV &&
          pfmt != PIPE_FORMAT_UYVY) {
         return false;
      }
      break;
   case I915_FORMAT_MOD_4_TILED_MTL_RC_CCS:
   case I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC:
   case I915_FORMAT_MOD_4_TILED_DG2_RC_CCS_CC:
   case I915_FORMAT_MOD_4_TILED_DG2_RC_CCS:
   case I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS_CC:
   case I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS:
   case I915_FORMAT_MOD_Y_TILED_CCS: {
      if (no_ccs)
         return false;

      enum isl_format rt_format =
         iris_format_for_usage(devinfo, pfmt,
                               ISL_SURF_USAGE_RENDER_TARGET_BIT).fmt;

      if (rt_format == ISL_FORMAT_UNSUPPORTED ||
          !isl_format_supports_ccs_e(devinfo, rt_format))
         return false;
      break;
   }
   default:
      break;
   }

   return true;
}

static uint64_t
select_best_modifier(const struct intel_device_info *devinfo,
                     const struct pipe_resource *templ,
                     const uint64_t *modifiers,
                     int count)
{
   enum modifier_priority prio = MODIFIER_PRIORITY_INVALID;

   for (int i = 0; i < count; i++) {
      if (!modifier_is_supported(devinfo, templ->format, templ->bind,
                                 modifiers[i]))
         continue;

      switch (modifiers[i]) {
      case I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC:
         prio = MAX2(prio, MODIFIER_PRIORITY_4_MTL_RC_CCS_CC);
         break;
      case I915_FORMAT_MOD_4_TILED_MTL_RC_CCS:
         prio = MAX2(prio, MODIFIER_PRIORITY_4_MTL_RC_CCS);
         break;
      case I915_FORMAT_MOD_4_TILED_DG2_RC_CCS_CC:
         prio = MAX2(prio, MODIFIER_PRIORITY_4_DG2_RC_CCS_CC);
         break;
      case I915_FORMAT_MOD_4_TILED_DG2_RC_CCS:
         prio = MAX2(prio, MODIFIER_PRIORITY_4_DG2_RC_CCS);
         break;
      case I915_FORMAT_MOD_4_TILED:
         prio = MAX2(prio, MODIFIER_PRIORITY_4);
         break;
      case I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS_CC:
         prio = MAX2(prio, MODIFIER_PRIORITY_Y_GFX12_RC_CCS_CC);
         break;
      case I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS:
         prio = MAX2(prio, MODIFIER_PRIORITY_Y_GFX12_RC_CCS);
         break;
      case I915_FORMAT_MOD_Y_TILED_CCS:
         prio = MAX2(prio, MODIFIER_PRIORITY_Y_CCS);
         break;
      case I915_FORMAT_MOD_Y_TILED:
         prio = MAX2(prio, MODIFIER_PRIORITY_Y);
         break;
      case I915_FORMAT_MOD_X_TILED:
         prio = MAX2(prio, MODIFIER_PRIORITY_X);
         break;
      case DRM_FORMAT_MOD_LINEAR:
         prio = MAX2(prio, MODIFIER_PRIORITY_LINEAR);
         break;
      case DRM_FORMAT_MOD_INVALID:
      default:
         break;
      }
   }

   return priority_to_modifier[prio];
}

static inline bool is_modifier_external_only(enum pipe_format pfmt,
                                             uint64_t modifier)
{
   /* Only allow external usage for the following cases: YUV formats
    * and the media-compression modifier. The render engine lacks
    * support for rendering to a media-compressed surface if the
    * compression ratio is large enough. By requiring external usage
    * of media-compressed surfaces, resolves are avoided.
    */
   return util_format_is_yuv(pfmt) ||
          isl_drm_modifier_get_info(modifier)->supports_media_compression;
}

static void
iris_query_dmabuf_modifiers(struct pipe_screen *pscreen,
                            enum pipe_format pfmt,
                            int max,
                            uint64_t *modifiers,
                            unsigned int *external_only,
                            int *count)
{
   struct iris_screen *screen = (void *) pscreen;
   const struct intel_device_info *devinfo = screen->devinfo;

   uint64_t all_modifiers[] = {
      DRM_FORMAT_MOD_LINEAR,
      I915_FORMAT_MOD_X_TILED,
      I915_FORMAT_MOD_4_TILED,
      I915_FORMAT_MOD_4_TILED_DG2_RC_CCS,
      I915_FORMAT_MOD_4_TILED_DG2_MC_CCS,
      I915_FORMAT_MOD_4_TILED_DG2_RC_CCS_CC,
      I915_FORMAT_MOD_4_TILED_MTL_RC_CCS,
      I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC,
      I915_FORMAT_MOD_4_TILED_MTL_MC_CCS,
      I915_FORMAT_MOD_Y_TILED,
      I915_FORMAT_MOD_Y_TILED_CCS,
      I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS,
      I915_FORMAT_MOD_Y_TILED_GEN12_MC_CCS,
      I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS_CC,
   };

   int supported_mods = 0;

   for (int i = 0; i < ARRAY_SIZE(all_modifiers); i++) {
      if (!modifier_is_supported(devinfo, pfmt, 0, all_modifiers[i]))
         continue;

      if (supported_mods < max) {
         if (modifiers)
            modifiers[supported_mods] = all_modifiers[i];

         if (external_only) {
            external_only[supported_mods] =
               is_modifier_external_only(pfmt, all_modifiers[i]);
         }
      }

      supported_mods++;
   }

   *count = supported_mods;
}

static bool
iris_is_dmabuf_modifier_supported(struct pipe_screen *pscreen,
                                  uint64_t modifier, enum pipe_format pfmt,
                                  bool *external_only)
{
   struct iris_screen *screen = (void *) pscreen;
   const struct intel_device_info *devinfo = screen->devinfo;

   if (modifier_is_supported(devinfo, pfmt, 0, modifier)) {
      if (external_only)
         *external_only = is_modifier_external_only(pfmt, modifier);

      return true;
   }

   return false;
}

static unsigned int
iris_get_dmabuf_modifier_planes(struct pipe_screen *pscreen, uint64_t modifier,
                                enum pipe_format format)
{
   unsigned int planes = util_format_get_num_planes(format);

   switch (modifier) {
   case I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC:
   case I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS_CC:
      return 3;
   case I915_FORMAT_MOD_4_TILED_MTL_RC_CCS:
   case I915_FORMAT_MOD_4_TILED_MTL_MC_CCS:
   case I915_FORMAT_MOD_4_TILED_DG2_RC_CCS_CC:
   case I915_FORMAT_MOD_Y_TILED_GEN12_MC_CCS:
   case I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS:
   case I915_FORMAT_MOD_Y_TILED_CCS:
      return 2 * planes;
   case I915_FORMAT_MOD_4_TILED_DG2_RC_CCS:
   case I915_FORMAT_MOD_4_TILED_DG2_MC_CCS:
   default:
      return planes;
   }
}

enum isl_format
iris_image_view_get_format(struct iris_context *ice,
                           const struct pipe_image_view *img)
{
   struct iris_screen *screen = (struct iris_screen *)ice->ctx.screen;
   const struct intel_device_info *devinfo = screen->devinfo;

   isl_surf_usage_flags_t usage = ISL_SURF_USAGE_STORAGE_BIT;
   enum isl_format isl_fmt =
      iris_format_for_usage(devinfo, img->format, usage).fmt;

   if (img->shader_access & PIPE_IMAGE_ACCESS_READ) {
      /* On Gfx8, try to use typed surfaces reads (which support a
       * limited number of formats), and if not possible, fall back
       * to untyped reads.
       */
      if (devinfo->ver == 8 &&
          !isl_has_matching_typed_storage_image_format(devinfo, isl_fmt))
         return ISL_FORMAT_RAW;
      else
         return isl_lower_storage_image_format(devinfo, isl_fmt);
   }

   return isl_fmt;
}

static struct pipe_memory_object *
iris_memobj_create_from_handle(struct pipe_screen *pscreen,
                               struct winsys_handle *whandle,
                               bool dedicated)
{
   struct iris_screen *screen = (struct iris_screen *)pscreen;
   struct iris_memory_object *memobj = CALLOC_STRUCT(iris_memory_object);
   struct iris_bo *bo;

   if (!memobj)
      return NULL;

   switch (whandle->type) {
   case WINSYS_HANDLE_TYPE_SHARED:
      bo = iris_bo_gem_create_from_name(screen->bufmgr, "winsys image",
                                        whandle->handle);
      break;
   case WINSYS_HANDLE_TYPE_FD:
      bo = iris_bo_import_dmabuf(screen->bufmgr, whandle->handle,
                                 whandle->modifier);
      break;
   default:
      unreachable("invalid winsys handle type");
   }

   if (!bo) {
      free(memobj);
      return NULL;
   }

   memobj->b.dedicated = dedicated;
   memobj->bo = bo;
   memobj->format = whandle->format;
   memobj->stride = whandle->stride;

   return &memobj->b;
}

static void
iris_memobj_destroy(struct pipe_screen *pscreen,
                    struct pipe_memory_object *pmemobj)
{
   struct iris_memory_object *memobj = (struct iris_memory_object *)pmemobj;

   iris_bo_unreference(memobj->bo);
   free(memobj);
}

struct pipe_resource *
iris_resource_get_separate_stencil(struct pipe_resource *p_res)
{
   /* For packed depth-stencil, we treat depth as the primary resource
    * and store S8 as the "second plane" resource.
    */
   if (p_res->next && p_res->next->format == PIPE_FORMAT_S8_UINT)
      return p_res->next;

   return NULL;

}

static void
iris_resource_set_separate_stencil(struct pipe_resource *p_res,
                                   struct pipe_resource *stencil)
{
   assert(util_format_has_depth(util_format_description(p_res->format)));
   pipe_resource_reference(&p_res->next, stencil);
}

void
iris_get_depth_stencil_resources(struct pipe_resource *res,
                                 struct iris_resource **out_z,
                                 struct iris_resource **out_s)
{
   if (!res) {
      *out_z = NULL;
      *out_s = NULL;
      return;
   }

   if (res->format != PIPE_FORMAT_S8_UINT) {
      *out_z = (void *) res;
      *out_s = (void *) iris_resource_get_separate_stencil(res);
   } else {
      *out_z = NULL;
      *out_s = (void *) res;
   }
}

void
iris_resource_disable_aux(struct iris_resource *res)
{
   iris_bo_unreference(res->aux.bo);
   iris_bo_unreference(res->aux.clear_color_bo);
   free(res->aux.state);

   res->aux.usage = ISL_AUX_USAGE_NONE;
   res->aux.surf.size_B = 0;
   res->aux.bo = NULL;
   res->aux.extra_aux.surf.size_B = 0;
   res->aux.clear_color_bo = NULL;
   res->aux.state = NULL;
}

static unsigned
iris_resource_alloc_flags(const struct iris_screen *screen,
                          const struct pipe_resource *templ,
                          enum isl_aux_usage aux_usage)
{
   if (templ->flags & IRIS_RESOURCE_FLAG_DEVICE_MEM)
      return BO_ALLOC_PLAIN;

   unsigned flags = BO_ALLOC_PLAIN;

   switch (templ->usage) {
   case PIPE_USAGE_STAGING:
      flags |= BO_ALLOC_SMEM | BO_ALLOC_COHERENT;
      break;
   case PIPE_USAGE_STREAM:
      flags |= BO_ALLOC_SMEM;
      break;
   case PIPE_USAGE_DYNAMIC:
   case PIPE_USAGE_DEFAULT:
   case PIPE_USAGE_IMMUTABLE:
      /* Use LMEM for these if possible */
      break;
   }

   if (templ->bind & PIPE_BIND_SCANOUT)
      flags |= BO_ALLOC_SCANOUT;

   if (templ->flags & (PIPE_RESOURCE_FLAG_MAP_COHERENT |
                       PIPE_RESOURCE_FLAG_MAP_PERSISTENT))
      flags |= BO_ALLOC_SMEM | BO_ALLOC_COHERENT;

   if (screen->devinfo->verx10 >= 125 && screen->devinfo->has_local_mem &&
       isl_aux_usage_has_ccs(aux_usage)) {
      assert((flags & BO_ALLOC_SMEM) == 0);
      flags |= BO_ALLOC_LMEM;
   }

   if ((templ->bind & PIPE_BIND_SHARED) ||
       util_format_get_num_planes(templ->format) > 1)
      flags |= BO_ALLOC_NO_SUBALLOC;

   if (templ->bind & PIPE_BIND_PROTECTED)
      flags |= BO_ALLOC_PROTECTED;

   if (templ->bind & PIPE_BIND_SHARED) {
      flags |= BO_ALLOC_SHARED;

      /* We request that the bufmgr zero because, if a buffer gets re-used
       * from the pool, we don't want to leak random garbage from our process
       * to some other.
       */
      flags |= BO_ALLOC_ZEROED;
   }

   return flags;
}

static void
iris_resource_destroy(struct pipe_screen *screen,
                      struct pipe_resource *p_res)
{
   struct iris_resource *res = (struct iris_resource *) p_res;

   if (p_res->target == PIPE_BUFFER)
      util_range_destroy(&res->valid_buffer_range);

   iris_resource_disable_aux(res);

   threaded_resource_deinit(p_res);
   iris_bo_unreference(res->bo);
   iris_pscreen_unref(res->orig_screen);

   free(res);
}

static struct iris_resource *
iris_alloc_resource(struct pipe_screen *pscreen,
                    const struct pipe_resource *templ)
{
   struct iris_resource *res = calloc(1, sizeof(struct iris_resource));
   if (!res)
      return NULL;

   res->base.b = *templ;
   res->base.b.screen = pscreen;
   res->orig_screen = iris_pscreen_ref(pscreen);
   pipe_reference_init(&res->base.b.reference, 1);
   threaded_resource_init(&res->base.b, false);

   if (templ->target == PIPE_BUFFER)
      util_range_init(&res->valid_buffer_range);

   return res;
}

unsigned
iris_get_num_logical_layers(const struct iris_resource *res, unsigned level)
{
   if (res->surf.dim == ISL_SURF_DIM_3D)
      return u_minify(res->surf.logical_level0_px.depth, level);
   else
      return res->surf.logical_level0_px.array_len;
}

static enum isl_aux_state **
create_aux_state_map(struct iris_resource *res, enum isl_aux_state initial)
{
   assert(res->aux.state == NULL);

   uint32_t total_slices = 0;
   for (uint32_t level = 0; level < res->surf.levels; level++)
      total_slices += iris_get_num_logical_layers(res, level);

   const size_t per_level_array_size =
      res->surf.levels * sizeof(enum isl_aux_state *);

   /* We're going to allocate a single chunk of data for both the per-level
    * reference array and the arrays of aux_state.  This makes cleanup
    * significantly easier.
    */
   const size_t total_size =
      per_level_array_size + total_slices * sizeof(enum isl_aux_state);

   void *data = malloc(total_size);
   if (!data)
      return NULL;

   enum isl_aux_state **per_level_arr = data;
   enum isl_aux_state *s = data + per_level_array_size;
   for (uint32_t level = 0; level < res->surf.levels; level++) {
      per_level_arr[level] = s;
      const unsigned level_layers = iris_get_num_logical_layers(res, level);
      for (uint32_t a = 0; a < level_layers; a++)
         *(s++) = initial;
   }
   assert((void *)s == data + total_size);

   return per_level_arr;
}

static unsigned
iris_get_aux_clear_color_state_size(struct iris_screen *screen,
                                    struct iris_resource *res)
{
   if (!isl_aux_usage_has_fast_clears(res->aux.usage))
      return 0;

   assert(!isl_surf_usage_is_stencil(res->surf.usage));

   /* Depth packets can't specify indirect clear values. The only time depth
    * buffers can use indirect clear values is when they're accessed by the
    * sampler via render surface state objects.
    */
   if (isl_surf_usage_is_depth(res->surf.usage) &&
       !iris_sample_with_depth_aux(screen->devinfo, res))
      return 0;

   return screen->isl_dev.ss.clear_color_state_size;
}

static void
map_aux_addresses(struct iris_screen *screen, struct iris_resource *res,
                  enum pipe_format pfmt, unsigned plane)
{
   void *aux_map_ctx = iris_bufmgr_get_aux_map_context(screen->bufmgr);
   if (!aux_map_ctx)
      return;

   if (isl_aux_usage_has_ccs(res->aux.usage)) {
      const unsigned aux_offset = res->aux.extra_aux.surf.size_B > 0 ?
         res->aux.extra_aux.offset : res->aux.offset;
      const enum isl_format format =
         iris_format_for_usage(screen->devinfo, pfmt, res->surf.usage).fmt;
      const uint64_t format_bits =
         intel_aux_map_format_bits(res->surf.tiling, format, plane);
      const bool mapped =
         intel_aux_map_add_mapping(aux_map_ctx,
                                   res->bo->address + res->offset,
                                   res->aux.bo->address + aux_offset,
                                   res->surf.size_B, format_bits);
      assert(mapped);
      res->bo->aux_map_address = res->aux.bo->address;
   }
}

static bool
want_ccs_e_for_format(const struct intel_device_info *devinfo,
                      enum isl_format format)
{
   if (!isl_format_supports_ccs_e(devinfo, format))
      return false;

   const struct isl_format_layout *fmtl = isl_format_get_layout(format);

   /* Prior to TGL, CCS_E seems to significantly hurt performance with 32-bit
    * floating point formats.  For example, Paraview's "Wavelet Volume" case
    * uses both R32_FLOAT and R32G32B32A32_FLOAT, and enabling CCS_E for those
    * formats causes a 62% FPS drop.
    *
    * However, many benchmarks seem to use 16-bit float with no issues.
    */
   if (devinfo->ver <= 11 &&
       fmtl->channels.r.bits == 32 && fmtl->channels.r.type == ISL_SFLOAT)
      return false;

   return true;
}

static enum isl_surf_dim
target_to_isl_surf_dim(enum pipe_texture_target target)
{
   switch (target) {
   case PIPE_BUFFER:
   case PIPE_TEXTURE_1D:
   case PIPE_TEXTURE_1D_ARRAY:
      return ISL_SURF_DIM_1D;
   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_CUBE:
   case PIPE_TEXTURE_RECT:
   case PIPE_TEXTURE_2D_ARRAY:
   case PIPE_TEXTURE_CUBE_ARRAY:
      return ISL_SURF_DIM_2D;
   case PIPE_TEXTURE_3D:
      return ISL_SURF_DIM_3D;
   case PIPE_MAX_TEXTURE_TYPES:
      break;
   }
   unreachable("invalid texture type");
}

static bool
iris_resource_configure_main(const struct iris_screen *screen,
                             struct iris_resource *res,
                             const struct pipe_resource *templ,
                             uint64_t modifier, uint32_t row_pitch_B)
{
   res->mod_info = isl_drm_modifier_get_info(modifier);

   if (modifier != DRM_FORMAT_MOD_INVALID && res->mod_info == NULL)
      return false;

   isl_tiling_flags_t tiling_flags = 0;

   if (res->mod_info != NULL) {
      tiling_flags = 1 << res->mod_info->tiling;
   } else if (templ->usage == PIPE_USAGE_STAGING ||
              templ->bind & (PIPE_BIND_LINEAR | PIPE_BIND_CURSOR)) {
      tiling_flags = ISL_TILING_LINEAR_BIT;
   } else if (res->external_format != PIPE_FORMAT_NONE) {
      /* This came from iris_resource_from_memobj and didn't have
       * PIPE_BIND_LINEAR set, so "optimal" tiling is desired.  Let isl
       * select the tiling.  The implicit contract is that both drivers
       * will arrive at the same tiling by using the same code to decide.
       */
      assert(modifier == DRM_FORMAT_MOD_INVALID);
      tiling_flags = ISL_TILING_ANY_MASK;
   } else if (!screen->devinfo->has_tiling_uapi &&
              (templ->bind & (PIPE_BIND_SCANOUT | PIPE_BIND_SHARED))) {
      tiling_flags = ISL_TILING_LINEAR_BIT;
   } else if (templ->bind & PIPE_BIND_SCANOUT) {
      tiling_flags = ISL_TILING_X_BIT;
   } else {
      tiling_flags = ISL_TILING_ANY_MASK;
   }

   /* We don't support Yf or Ys tiling yet */
   tiling_flags &= ~ISL_TILING_STD_Y_MASK;
   assert(tiling_flags != 0);

   isl_surf_usage_flags_t usage = 0;

   if (res->mod_info && !isl_drm_modifier_has_aux(modifier))
      usage |= ISL_SURF_USAGE_DISABLE_AUX_BIT;

   else if (!res->mod_info && res->external_format != PIPE_FORMAT_NONE)
      usage |= ISL_SURF_USAGE_DISABLE_AUX_BIT;

   else if (templ->bind & PIPE_BIND_CONST_BW)
      usage |= ISL_SURF_USAGE_DISABLE_AUX_BIT;

   if (templ->usage == PIPE_USAGE_STAGING)
      usage |= ISL_SURF_USAGE_STAGING_BIT;

   if (templ->bind & PIPE_BIND_RENDER_TARGET)
      usage |= ISL_SURF_USAGE_RENDER_TARGET_BIT;

   if (templ->bind & PIPE_BIND_SAMPLER_VIEW)
      usage |= ISL_SURF_USAGE_TEXTURE_BIT;

   if (templ->bind & PIPE_BIND_SHADER_IMAGE)
      usage |= ISL_SURF_USAGE_STORAGE_BIT;

   if (templ->bind & PIPE_BIND_SCANOUT)
      usage |= ISL_SURF_USAGE_DISPLAY_BIT;

   if (templ->target == PIPE_TEXTURE_CUBE ||
       templ->target == PIPE_TEXTURE_CUBE_ARRAY) {
      usage |= ISL_SURF_USAGE_CUBE_BIT;
   }

   if (templ->usage != PIPE_USAGE_STAGING &&
       util_format_is_depth_or_stencil(templ->format)) {

      /* Should be handled by u_transfer_helper */
      assert(!util_format_is_depth_and_stencil(templ->format));

      usage |= templ->format == PIPE_FORMAT_S8_UINT ?
               ISL_SURF_USAGE_STENCIL_BIT : ISL_SURF_USAGE_DEPTH_BIT;
   }

   const enum isl_format format =
      iris_format_for_usage(screen->devinfo, templ->format, usage).fmt;

   const struct isl_surf_init_info init_info = {
      .dim = target_to_isl_surf_dim(templ->target),
      .format = format,
      .width = templ->width0,
      .height = templ->height0,
      .depth = templ->depth0,
      .levels = templ->last_level + 1,
      .array_len = templ->array_size,
      .samples = MAX2(templ->nr_samples, 1),
      .min_alignment_B = 0,
      .row_pitch_B = row_pitch_B,
      .usage = usage,
      .tiling_flags = tiling_flags
   };

   if (!isl_surf_init_s(&screen->isl_dev, &res->surf, &init_info))
      return false;

   res->internal_format = templ->format;

   return true;
}

static bool
iris_get_ccs_surf_or_support(const struct isl_device *dev,
                             const struct isl_surf *surf,
                             struct isl_surf *aux_surf,
                             struct isl_surf *extra_aux_surf)
{
   assert(extra_aux_surf->size_B == 0);

   struct isl_surf *ccs_surf;
   const struct isl_surf *hiz_or_mcs_surf;
   if (aux_surf->size_B > 0) {
      assert(aux_surf->usage & (ISL_SURF_USAGE_HIZ_BIT |
                                ISL_SURF_USAGE_MCS_BIT));
      hiz_or_mcs_surf = aux_surf;
      ccs_surf = extra_aux_surf;
   } else {
      hiz_or_mcs_surf = NULL;
      ccs_surf = aux_surf;
   }

   if (dev->info->has_flat_ccs) {
      /* CCS doesn't require VMA on XeHP. So, instead of creating a separate
       * surface, we can just return whether CCS is supported for the given
       * input surfaces.
       */
      return isl_surf_supports_ccs(dev, surf, hiz_or_mcs_surf);
   } else  {
      return isl_surf_get_ccs_surf(dev, surf, hiz_or_mcs_surf, ccs_surf, 0);
   }
}

/**
 * Configure aux for the resource, but don't allocate it. For images which
 * might be shared with modifiers, we must allocate the image and aux data in
 * a single bo.
 *
 * Returns false on unexpected error (e.g. allocation failed, or invalid
 * configuration result).
 */
static bool
iris_resource_configure_aux(struct iris_screen *screen,
                            struct iris_resource *res, bool imported)
{
   const struct intel_device_info *devinfo = screen->devinfo;

   const bool has_mcs =
      isl_surf_get_mcs_surf(&screen->isl_dev, &res->surf, &res->aux.surf);

   const bool has_hiz =
      isl_surf_get_hiz_surf(&screen->isl_dev, &res->surf, &res->aux.surf);

   const bool has_ccs =
      iris_get_ccs_surf_or_support(&screen->isl_dev, &res->surf,
                                   &res->aux.surf, &res->aux.extra_aux.surf);

   if (has_mcs) {
      assert(!res->mod_info);
      assert(!has_hiz);
      /* We are seeing failures with CCS compression on top of MSAA
       * compression, so just enable MSAA compression for now on DG2.
       */
      if (!intel_device_info_is_dg2(devinfo) && has_ccs) {
         res->aux.usage = ISL_AUX_USAGE_MCS_CCS;
      } else {
         res->aux.usage = ISL_AUX_USAGE_MCS;
      }
   } else if (has_hiz) {
      assert(!res->mod_info);
      assert(!has_mcs);
      if (!has_ccs) {
         res->aux.usage = ISL_AUX_USAGE_HIZ;
      } else if (res->surf.samples == 1 &&
                 (res->surf.usage & ISL_SURF_USAGE_TEXTURE_BIT)) {
         /* If this resource is single-sampled and will be used as a texture,
          * put the HiZ surface in write-through mode so that we can sample
          * from it.
          */
         res->aux.usage = ISL_AUX_USAGE_HIZ_CCS_WT;
      } else {
         res->aux.usage = ISL_AUX_USAGE_HIZ_CCS;
      }
   } else if (has_ccs) {
      if (isl_surf_usage_is_stencil(res->surf.usage)) {
         assert(!res->mod_info);
         res->aux.usage = ISL_AUX_USAGE_STC_CCS;
      } else if (res->mod_info && res->mod_info->supports_media_compression) {
         res->aux.usage = ISL_AUX_USAGE_MC;
      } else if (want_ccs_e_for_format(devinfo, res->surf.format)) {
         res->aux.usage = devinfo->ver < 12 ?
            ISL_AUX_USAGE_CCS_E : ISL_AUX_USAGE_FCV_CCS_E;
      } else {
         assert(isl_format_supports_ccs_d(devinfo, res->surf.format));
         res->aux.usage = ISL_AUX_USAGE_CCS_D;
      }
   }

   enum isl_aux_state initial_state;
   switch (res->aux.usage) {
   case ISL_AUX_USAGE_NONE:
      /* Having no aux buffer is only okay if there's no modifier with aux. */
      return !res->mod_info ||
             !isl_drm_modifier_has_aux(res->mod_info->modifier);
   case ISL_AUX_USAGE_HIZ:
   case ISL_AUX_USAGE_HIZ_CCS:
   case ISL_AUX_USAGE_HIZ_CCS_WT:
   case ISL_AUX_USAGE_MCS:
   case ISL_AUX_USAGE_MCS_CCS:
      /* Leave the auxiliary buffer uninitialized. We can ambiguate it before
       * accessing it later on, if needed.
       */
      initial_state = ISL_AUX_STATE_AUX_INVALID;
      break;
   case ISL_AUX_USAGE_CCS_D:
   case ISL_AUX_USAGE_CCS_E:
   case ISL_AUX_USAGE_FCV_CCS_E:
   case ISL_AUX_USAGE_STC_CCS:
   case ISL_AUX_USAGE_MC:
      if (imported) {
         assert(res->aux.usage != ISL_AUX_USAGE_STC_CCS);
         initial_state =
            isl_drm_modifier_get_default_aux_state(res->mod_info->modifier);
      } else if (devinfo->has_flat_ccs) {
         assert(res->aux.surf.size_B == 0);
         /* From Bspec 47709, "MCS/CCS Buffers for Render Target(s)":
          *
          *    "CCS surface does not require initialization. Illegal CCS
          *     [values] are treated as uncompressed memory."
          *
          * The above quote is from the render target section, but we assume
          * it applies to CCS in general (e.g., STC_CCS). The uninitialized
          * CCS may be in any aux state. We choose the one which is most
          * convenient.
          *
          * We avoid states with CLEAR because stencil does not support it.
          * Those states also create a dependency on the clear color, which
          * can have negative performance implications. Even though some
          * blocks may actually be encoded with CLEAR, we can get away with
          * ignoring them - there are no known issues that require fast
          * cleared blocks to be tracked and avoided.
          *
          * We specifically avoid the AUX_INVALID state because it could
          * trigger an ambiguate. BLORP does not have support for ambiguating
          * stencil. Also, ambiguating some LODs of mipmapped 8bpp surfaces
          * seems to stomp on neighboring miplevels.
          *
          * There is only one remaining aux state which can give us correct
          * behavior, COMPRESSED_NO_CLEAR.
          */
         initial_state = ISL_AUX_STATE_COMPRESSED_NO_CLEAR;
      } else {
         assert(res->aux.surf.size_B > 0);
         /* When CCS is used, we need to ensure that it starts off in a valid
          * state. From the Sky Lake PRM, "MCS Buffer for Render Target(s)":
          *
          *    "If Software wants to enable Color Compression without Fast
          *     clear, Software needs to initialize MCS with zeros."
          *
          * A CCS surface initialized to zero is in the pass-through state.
          * This state can avoid the need to ambiguate in some cases. We'll
          * map and zero the CCS later on in iris_resource_init_aux_buf.
          */
         initial_state = ISL_AUX_STATE_PASS_THROUGH;
      }
      break;
   default:
      unreachable("Unsupported aux mode");
   }

   /* Create the aux_state for the auxiliary buffer. */
   res->aux.state = create_aux_state_map(res, initial_state);
   if (!res->aux.state)
      return false;

   return true;
}

/**
 * Initialize the aux buffer contents.
 *
 * Returns false on unexpected error (e.g. mapping a BO failed).
 */
static bool
iris_resource_init_aux_buf(struct iris_screen *screen,
                           struct iris_resource *res)
{
   void *map = NULL;

   if (iris_resource_get_aux_state(res, 0, 0) != ISL_AUX_STATE_AUX_INVALID &&
       res->aux.surf.size_B > 0) {
      if (!map)
         map = iris_bo_map(NULL, res->bo, MAP_WRITE | MAP_RAW);
      if (!map)
         return false;

      memset((char*)map + res->aux.offset, 0, res->aux.surf.size_B);
   }

   if (res->aux.extra_aux.surf.size_B > 0) {
      if (!map)
         map = iris_bo_map(NULL, res->bo, MAP_WRITE | MAP_RAW);
      if (!map)
         return false;

      memset((char*)map + res->aux.extra_aux.offset,
             0, res->aux.extra_aux.surf.size_B);
   }

   if (map)
      iris_bo_unmap(res->bo);

   if (res->aux.surf.size_B > 0) {
      res->aux.bo = res->bo;
      iris_bo_reference(res->aux.bo);
      map_aux_addresses(screen, res, res->internal_format, 0);
   }

   if (iris_get_aux_clear_color_state_size(screen, res) > 0) {
      res->aux.clear_color_bo = res->bo;
      iris_bo_reference(res->aux.clear_color_bo);
      res->aux.clear_color_unknown = !res->aux.clear_color_bo->zeroed;
   }

   return true;
}

static uint32_t
iris_buffer_alignment(uint64_t size)
{
   /* Some buffer operations want some amount of alignment.  The largest
    * buffer texture pixel size is 4 * 4 = 16B.  OpenCL data is also supposed
    * to be aligned and largest OpenCL data type is a double16 which is
    * 8 * 16 = 128B.  Align to the largest power of 2 which fits in the size,
    * up to 128B.
    */
   uint32_t align = MAX2(4 * 4, 8 * 16);
   while (align > size)
      align >>= 1;

   return align;
}

static struct pipe_resource *
iris_resource_create_for_buffer(struct pipe_screen *pscreen,
                                const struct pipe_resource *templ)
{
   struct iris_screen *screen = (struct iris_screen *)pscreen;
   struct iris_resource *res = iris_alloc_resource(pscreen, templ);

   assert(templ->target == PIPE_BUFFER);
   assert(templ->height0 <= 1);
   assert(templ->depth0 <= 1);
   assert(templ->format == PIPE_FORMAT_NONE ||
          util_format_get_blocksize(templ->format) == 1);

   res->internal_format = templ->format;
   res->surf.tiling = ISL_TILING_LINEAR;

   enum iris_memory_zone memzone = IRIS_MEMZONE_OTHER;
   const char *name = templ->target == PIPE_BUFFER ? "buffer" : "miptree";
   if (templ->flags & IRIS_RESOURCE_FLAG_SHADER_MEMZONE) {
      memzone = IRIS_MEMZONE_SHADER;
      name = "shader kernels";
   } else if (templ->flags & IRIS_RESOURCE_FLAG_SURFACE_MEMZONE) {
      memzone = IRIS_MEMZONE_SURFACE;
      name = "surface state";
   } else if (templ->flags & IRIS_RESOURCE_FLAG_DYNAMIC_MEMZONE) {
      memzone = IRIS_MEMZONE_DYNAMIC;
      name = "dynamic state";
   } else if (templ->flags & IRIS_RESOURCE_FLAG_SCRATCH_MEMZONE) {
      memzone = IRIS_MEMZONE_SCRATCH;
      name = "scratch surface state";
   }

   unsigned flags = iris_resource_alloc_flags(screen, templ, res->aux.usage);

   res->bo = iris_bo_alloc(screen->bufmgr, name, templ->width0,
                           iris_buffer_alignment(templ->width0),
                           memzone, flags);

   if (!res->bo) {
      iris_resource_destroy(pscreen, &res->base.b);
      return NULL;
   }

   if (templ->bind & PIPE_BIND_SHARED) {
      iris_bo_mark_exported(res->bo);
      res->base.is_shared = true;
   }

   return &res->base.b;
}

static struct pipe_resource *
iris_resource_create_for_image(struct pipe_screen *pscreen,
                               const struct pipe_resource *templ,
                               const uint64_t *modifiers,
                               int modifiers_count,
                               unsigned row_pitch_B)
{
   struct iris_screen *screen = (struct iris_screen *)pscreen;
   const struct intel_device_info *devinfo = screen->devinfo;
   struct iris_resource *res = iris_alloc_resource(pscreen, templ);

   if (!res)
      return NULL;

   uint64_t modifier =
      select_best_modifier(devinfo, templ, modifiers, modifiers_count);

   if (modifier == DRM_FORMAT_MOD_INVALID && modifiers_count > 0) {
      fprintf(stderr, "Unsupported modifier, resource creation failed.\n");
      goto fail;
   }

   const bool isl_surf_created_successfully =
      iris_resource_configure_main(screen, res, templ, modifier, row_pitch_B);
   if (!isl_surf_created_successfully)
      goto fail;

   /* Don't create staging surfaces that will use over half the sram,
    * since staging implies you are copying data to another resource that's
    * at least as large, and then both wouldn't fit in system memory.
    *
    * Skip this for discrete cards, as the destination buffer might be in
    * device local memory while the staging buffer would be in system memory,
    * so both would fit.
    */
   if (templ->usage == PIPE_USAGE_STAGING && !devinfo->has_local_mem &&
       res->surf.size_B > (iris_bufmgr_sram_size(screen->bufmgr) / 2))
      goto fail;

   if (!iris_resource_configure_aux(screen, res, false))
      goto fail;

   const char *name = "miptree";
   enum iris_memory_zone memzone = IRIS_MEMZONE_OTHER;

   unsigned flags = iris_resource_alloc_flags(screen, templ, res->aux.usage);

   /* These are for u_upload_mgr buffers only */
   assert(!(templ->flags & (IRIS_RESOURCE_FLAG_SHADER_MEMZONE |
                            IRIS_RESOURCE_FLAG_SURFACE_MEMZONE |
                            IRIS_RESOURCE_FLAG_DYNAMIC_MEMZONE |
                            IRIS_RESOURCE_FLAG_SCRATCH_MEMZONE)));

   /* Modifiers require the aux data to be in the same buffer as the main
    * surface, but we combine them even when a modifier is not being used.
    */
   uint64_t bo_size = res->surf.size_B;

   /* Allocate space for the aux buffer. */
   if (res->aux.surf.size_B > 0) {
      res->aux.offset = (uint32_t)align64(bo_size, res->aux.surf.alignment_B);
      bo_size = res->aux.offset + res->aux.surf.size_B;
   }

   /* Allocate space for the extra aux buffer. */
   if (res->aux.extra_aux.surf.size_B > 0) {
      res->aux.extra_aux.offset =
         (uint32_t)align64(bo_size, res->aux.extra_aux.surf.alignment_B);
      bo_size = res->aux.extra_aux.offset + res->aux.extra_aux.surf.size_B;
   }

   /* Allocate space for the indirect clear color.
    *
    * Also add some padding to make sure the fast clear color state buffer
    * starts at a 4K alignment. We believe that 256B might be enough, but due
    * to lack of testing we will leave this as 4K for now.
    */
   if (iris_get_aux_clear_color_state_size(screen, res) > 0) {
      res->aux.clear_color_offset = align64(bo_size, 4096);
      bo_size = res->aux.clear_color_offset +
                iris_get_aux_clear_color_state_size(screen, res);
   }

   /* The ISL alignment already includes AUX-TT requirements, so no additional
    * attention required here :)
    */
   uint32_t alignment = MAX2(4096, res->surf.alignment_B);
   res->bo =
      iris_bo_alloc(screen->bufmgr, name, bo_size, alignment, memzone, flags);

   if (!res->bo)
      goto fail;

   if (res->aux.usage != ISL_AUX_USAGE_NONE &&
       !iris_resource_init_aux_buf(screen, res))
      goto fail;

   if (templ->bind & PIPE_BIND_SHARED) {
      iris_bo_mark_exported(res->bo);
      res->base.is_shared = true;
   }

   return &res->base.b;

fail:
   iris_resource_destroy(pscreen, &res->base.b);
   return NULL;
}

static struct pipe_resource *
iris_resource_create_with_modifiers(struct pipe_screen *pscreen,
                                    const struct pipe_resource *templ,
                                    const uint64_t *modifiers,
                                    int modifier_count)
{
   return iris_resource_create_for_image(pscreen, templ, modifiers,
                                         modifier_count, 0);
}

static struct pipe_resource *
iris_resource_create(struct pipe_screen *pscreen,
                     const struct pipe_resource *templ)
{
   if (templ->target == PIPE_BUFFER)
      return iris_resource_create_for_buffer(pscreen, templ);
   else
      return iris_resource_create_with_modifiers(pscreen, templ, NULL, 0);
}

static uint64_t
tiling_to_modifier(uint32_t tiling)
{
   static const uint64_t map[] = {
      [I915_TILING_NONE]   = DRM_FORMAT_MOD_LINEAR,
      [I915_TILING_X]      = I915_FORMAT_MOD_X_TILED,
      [I915_TILING_Y]      = I915_FORMAT_MOD_Y_TILED,
   };

   assert(tiling < ARRAY_SIZE(map));

   return map[tiling];
}

static struct pipe_resource *
iris_resource_from_user_memory(struct pipe_screen *pscreen,
                               const struct pipe_resource *templ,
                               void *user_memory)
{
   struct iris_screen *screen = (struct iris_screen *)pscreen;
   struct iris_bufmgr *bufmgr = screen->bufmgr;
   struct iris_resource *res = iris_alloc_resource(pscreen, templ);
   if (!res)
      return NULL;

   if (templ->target != PIPE_BUFFER &&
       templ->target != PIPE_TEXTURE_1D &&
       templ->target != PIPE_TEXTURE_2D)
      return NULL;

   if (templ->array_size > 1)
      return NULL;

   size_t res_size = templ->width0;
   if (templ->target != PIPE_BUFFER) {
      const uint32_t row_pitch_B =
         templ->width0 * util_format_get_blocksize(templ->format);
      res_size = templ->height0 * row_pitch_B;

      if (!iris_resource_configure_main(screen, res, templ,
                                        DRM_FORMAT_MOD_LINEAR,
                                        row_pitch_B)) {
         iris_resource_destroy(pscreen, &res->base.b);
         return NULL;
      }
      assert(res->surf.size_B <= res_size);
   }

   /* The userptr ioctl only works on whole pages.  Because we know that
    * things will exist in memory at a page granularity, we can expand the
    * range given by the client into the whole number of pages and use an
    * offset on the resource to make it looks like it starts at the user's
    * pointer.
    */
   size_t page_size = getpagesize();
   assert(util_is_power_of_two_nonzero(page_size));
   size_t offset = (uintptr_t)user_memory & (page_size - 1);
   void *mem_start = (char *)user_memory - offset;
   size_t mem_size = offset + res_size;
   mem_size = ALIGN_NPOT(mem_size, page_size);

   res->internal_format = templ->format;
   res->base.is_user_ptr = true;
   res->bo = iris_bo_create_userptr(bufmgr, "user", mem_start, mem_size,
                                    IRIS_MEMZONE_OTHER);
   res->offset = offset;
   if (!res->bo) {
      iris_resource_destroy(pscreen, &res->base.b);
      return NULL;
   }

   util_range_add(&res->base.b, &res->valid_buffer_range, 0, templ->width0);

   return &res->base.b;
}

static unsigned
get_num_planes(const struct pipe_resource *resource)
{
   unsigned count = 0;
   for (const struct pipe_resource *cur = resource; cur; cur = cur->next)
      count++;

   return count;
}

static unsigned
get_main_plane_for_plane(enum pipe_format format,
                         unsigned plane)
{
   if (format == PIPE_FORMAT_NONE) {
      /* Created dmabuf resources have this format. */
      return 0;
   } else if (isl_format_for_pipe_format(format) == ISL_FORMAT_UNSUPPORTED) {
      /* This format has been lowered to more planes than are native to it.
       * So, compression modifiers are not enabled and the plane index is used
       * as-is.
       */
      return plane;
   } else {
      unsigned int n_planes = util_format_get_num_planes(format);
      return plane % n_planes;
   }
}

static struct pipe_resource *
iris_resource_from_handle(struct pipe_screen *pscreen,
                          const struct pipe_resource *templ,
                          struct winsys_handle *whandle,
                          unsigned usage)
{
   struct iris_screen *screen = (struct iris_screen *)pscreen;
   const struct intel_device_info *devinfo = screen->devinfo;
   struct iris_bufmgr *bufmgr = screen->bufmgr;

   /* The gallium dri layer creates a pipe resource for each plane specified
    * by the format and modifier. Once all planes are present, we will merge
    * the separate parameters into the iris_resource(s) for the main plane(s).
    * Save the modifier import information now to reconstruct later.
    */
   struct iris_resource *res = iris_alloc_resource(pscreen, templ);
   if (!res)
      return NULL;

   switch (whandle->type) {
   case WINSYS_HANDLE_TYPE_FD:
      res->bo = iris_bo_import_dmabuf(bufmgr, whandle->handle,
                                      whandle->modifier);
      break;
   case WINSYS_HANDLE_TYPE_SHARED:
      res->bo = iris_bo_gem_create_from_name(bufmgr, "winsys image",
                                             whandle->handle);
      break;
   default:
      unreachable("invalid winsys handle type");
   }
   if (!res->bo)
      goto fail;

   res->offset = whandle->offset;
   res->surf.row_pitch_B = whandle->stride;

   if (whandle->plane == 0) {
      /* All planes are present. Fill out the main plane resource(s). */
      for (unsigned plane = 0; plane < util_resource_num(templ); plane++) {
         const unsigned main_plane =
            get_main_plane_for_plane(whandle->format, plane);
         struct iris_resource *main_res = (struct iris_resource *)
            util_resource_at_index(&res->base.b, main_plane);
         const struct iris_resource *plane_res = (struct iris_resource *)
            util_resource_at_index(&res->base.b, plane);

         if (isl_drm_modifier_plane_is_clear_color(whandle->modifier,
                                                   plane)) {
            /* Fill out the clear color fields. */
            assert(plane_res->bo->size >= plane_res->offset +
                   screen->isl_dev.ss.clear_color_state_size);

            iris_bo_reference(plane_res->bo);
            main_res->aux.clear_color_bo = plane_res->bo;
            main_res->aux.clear_color_offset = plane_res->offset;
            main_res->aux.clear_color_unknown = true;
         } else if (plane > main_plane) {
            /* Fill out some aux surface fields. */
            assert(isl_drm_modifier_has_aux(whandle->modifier));
            assert(!devinfo->has_flat_ccs);
            assert(plane_res->bo->size >= plane_res->offset +
                   main_res->aux.surf.size_B);
            assert(main_res->aux.surf.row_pitch_B ==
                   plane_res->surf.row_pitch_B);

            iris_bo_reference(plane_res->bo);
            main_res->aux.bo = plane_res->bo;
            main_res->aux.offset = plane_res->offset;
            map_aux_addresses(screen, main_res, whandle->format, main_plane);
         } else {
            /* Fill out fields that are convenient to initialize now. */
            assert(plane == main_plane);

            main_res->external_format = whandle->format;

            if (templ->target == PIPE_BUFFER) {
               main_res->surf.tiling = ISL_TILING_LINEAR;
               return &main_res->base.b;
            }

            uint64_t modifier;
            if (whandle->modifier == DRM_FORMAT_MOD_INVALID) {
               /* We have no modifier; match whatever GEM_GET_TILING says */
               uint32_t tiling;
               iris_gem_get_tiling(main_res->bo, &tiling);
               modifier = tiling_to_modifier(tiling);
            } else {
               modifier = whandle->modifier;
            }

            const bool isl_surf_created_successfully =
               iris_resource_configure_main(screen, main_res,
                                            &main_res->base.b, modifier,
                                            main_res->surf.row_pitch_B);
            if (!isl_surf_created_successfully)
               goto fail;

            assert(main_res->bo->size >= main_res->offset +
                   main_res->surf.size_B);

            if (!iris_resource_configure_aux(screen, main_res, true))
               goto fail;

            /* Add on a clear color BO if needed.
             *
             * Also add some padding to make sure the fast clear color state
             * buffer starts at a 4K alignment to avoid some unknown issues.
             * See the matching comment in iris_resource_create_for_image().
             */
            if (!main_res->mod_info->supports_clear_color &&
                iris_get_aux_clear_color_state_size(screen, main_res) > 0) {
               main_res->aux.clear_color_bo =
                  iris_bo_alloc(screen->bufmgr, "clear color buffer",
                                screen->isl_dev.ss.clear_color_state_size,
                                4096, IRIS_MEMZONE_OTHER, BO_ALLOC_ZEROED);
               if (!main_res->aux.clear_color_bo)
                  goto fail;
            }
         }
      }
   }

   return &res->base.b;

fail:
   iris_resource_destroy(pscreen, &res->base.b);
   return NULL;
}

static struct pipe_resource *
iris_resource_from_memobj(struct pipe_screen *pscreen,
                          const struct pipe_resource *templ,
                          struct pipe_memory_object *pmemobj,
                          uint64_t offset)
{
   struct iris_screen *screen = (struct iris_screen *)pscreen;
   struct iris_memory_object *memobj = (struct iris_memory_object *)pmemobj;
   struct iris_resource *res = iris_alloc_resource(pscreen, templ);

   if (!res)
      return NULL;

   res->bo = memobj->bo;
   res->offset = offset;
   res->external_format = templ->format;
   res->internal_format = templ->format;

   if (templ->flags & PIPE_RESOURCE_FLAG_TEXTURING_MORE_LIKELY) {
      UNUSED const bool isl_surf_created_successfully =
         iris_resource_configure_main(screen, res, templ, DRM_FORMAT_MOD_INVALID, 0);
      assert(isl_surf_created_successfully);
   }

   iris_bo_reference(memobj->bo);

   return &res->base.b;
}

/* Handle combined depth/stencil with memory objects.
 *
 * This function is modeled after u_transfer_helper_resource_create.
 */
static struct pipe_resource *
iris_resource_from_memobj_wrapper(struct pipe_screen *pscreen,
                                  const struct pipe_resource *templ,
                                  struct pipe_memory_object *pmemobj,
                                  uint64_t offset)
{
   enum pipe_format format = templ->format;

   /* Normal case, no special handling: */
   if (!(util_format_is_depth_and_stencil(format)))
      return iris_resource_from_memobj(pscreen, templ, pmemobj, offset);

   struct pipe_resource t = *templ;
   t.format = util_format_get_depth_only(format);

   struct pipe_resource *prsc =
      iris_resource_from_memobj(pscreen, &t, pmemobj, offset);
   if (!prsc)
      return NULL;

   struct iris_resource *res = (struct iris_resource *) prsc;

   /* Stencil offset in the buffer without aux. */
   uint64_t s_offset = offset +
      align64(res->surf.size_B, res->surf.alignment_B);

   prsc->format = format; /* frob the format back to the "external" format */

   t.format = PIPE_FORMAT_S8_UINT;
   struct pipe_resource *stencil =
      iris_resource_from_memobj(pscreen, &t, pmemobj, s_offset);
   if (!stencil) {
      iris_resource_destroy(pscreen, prsc);
      return NULL;
   }

   iris_resource_set_separate_stencil(prsc, stencil);
   return prsc;
}

static void
iris_flush_resource(struct pipe_context *ctx, struct pipe_resource *resource)
{
   struct iris_context *ice = (struct iris_context *)ctx;
   struct iris_resource *res = (void *) resource;
   const struct isl_drm_modifier_info *mod = res->mod_info;

   iris_resource_prepare_access(ice, res,
                                0, INTEL_REMAINING_LEVELS,
                                0, INTEL_REMAINING_LAYERS,
                                mod ? res->aux.usage : ISL_AUX_USAGE_NONE,
                                mod ? mod->supports_clear_color : false);

   if (!res->mod_info && res->aux.usage != ISL_AUX_USAGE_NONE) {
      /* flush_resource may be used to prepare an image for sharing external
       * to the driver (e.g. via eglCreateImage). To account for this, make
       * sure to get rid of any compression that a consumer wouldn't know how
       * to handle.
       */
      iris_foreach_batch(ice, batch) {
         if (iris_batch_references(batch, res->bo))
            iris_batch_flush(batch);
      }

      iris_resource_disable_aux(res);
   }
}

/**
 * Reallocate a (non-external) resource into new storage, copying the data
 * and modifying the original resource to point at the new storage.
 *
 * This is useful for e.g. moving a suballocated internal resource to a
 * dedicated allocation that can be exported by itself.
 */
static void
iris_reallocate_resource_inplace(struct iris_context *ice,
                                 struct iris_resource *old_res,
                                 unsigned new_bind_flag)
{
   struct pipe_screen *pscreen = ice->ctx.screen;

   if (iris_bo_is_external(old_res->bo))
      return;

   assert(old_res->mod_info == NULL);
   assert(old_res->bo == old_res->aux.bo || old_res->aux.bo == NULL);
   assert(old_res->bo == old_res->aux.clear_color_bo ||
          old_res->aux.clear_color_bo == NULL);
   assert(old_res->external_format == PIPE_FORMAT_NONE);

   struct pipe_resource templ = old_res->base.b;
   templ.bind |= new_bind_flag;

   struct iris_resource *new_res =
      (void *) pscreen->resource_create(pscreen, &templ);

   assert(iris_bo_is_real(new_res->bo));

   struct iris_batch *batch = &ice->batches[IRIS_BATCH_RENDER];

   if (old_res->base.b.target == PIPE_BUFFER) {
      struct pipe_box box = (struct pipe_box) {
         .width = old_res->base.b.width0,
         .height = 1,
      };

      iris_copy_region(&ice->blorp, batch, &new_res->base.b, 0, 0, 0, 0,
                       &old_res->base.b, 0, &box);
   } else {
      for (unsigned l = 0; l <= templ.last_level; l++) {
         struct pipe_box box = (struct pipe_box) {
            .width = u_minify(templ.width0, l),
            .height = u_minify(templ.height0, l),
            .depth = util_num_layers(&templ, l),
         };

         iris_copy_region(&ice->blorp, batch, &new_res->base.b, l, 0, 0, 0,
                          &old_res->base.b, l, &box);
      }
   }

   iris_flush_resource(&ice->ctx, &new_res->base.b);
   iris_foreach_batch(ice, batch) {
      if (iris_batch_references(batch, new_res->bo))
         iris_batch_flush(batch);
   }

   struct iris_bo *old_bo = old_res->bo;
   struct iris_bo *old_aux_bo = old_res->aux.bo;
   struct iris_bo *old_clear_color_bo = old_res->aux.clear_color_bo;

   /* Replace the structure fields with the new ones */
   old_res->base.b.bind = templ.bind;
   old_res->bo = new_res->bo;
   old_res->aux.surf = new_res->aux.surf;
   old_res->aux.bo = new_res->aux.bo;
   old_res->aux.offset = new_res->aux.offset;
   old_res->aux.extra_aux.surf = new_res->aux.extra_aux.surf;
   old_res->aux.extra_aux.offset = new_res->aux.extra_aux.offset;
   old_res->aux.clear_color_bo = new_res->aux.clear_color_bo;
   old_res->aux.clear_color_offset = new_res->aux.clear_color_offset;
   old_res->aux.usage = new_res->aux.usage;

   if (new_res->aux.state) {
      assert(old_res->aux.state);
      for (unsigned l = 0; l <= templ.last_level; l++) {
         unsigned layers = util_num_layers(&templ, l);
         for (unsigned z = 0; z < layers; z++) {
            enum isl_aux_state aux =
               iris_resource_get_aux_state(new_res, l, z);
            iris_resource_set_aux_state(ice, old_res, l, z, 1, aux);
         }
      }
   }

   /* old_res now points at the new BOs, make new_res point at the old ones
    * so they'll be freed when we unreference the resource below.
    */
   new_res->bo = old_bo;
   new_res->aux.bo = old_aux_bo;
   new_res->aux.clear_color_bo = old_clear_color_bo;

   pipe_resource_reference((struct pipe_resource **)&new_res, NULL);
}

static void
iris_resource_disable_suballoc_on_first_query(struct pipe_screen *pscreen,
                                              struct pipe_context *ctx,
                                              struct iris_resource *res)
{
   if (iris_bo_is_real(res->bo))
      return;

   assert(!(res->base.b.bind & PIPE_BIND_SHARED));

   bool destroy_context;
   if (ctx) {
      ctx = threaded_context_unwrap_sync(ctx);
      destroy_context = false;
   } else {
      /* We need to execute a blit on some GPU context, but the DRI layer
       * often doesn't give us one.  So we have to invent a temporary one.
       *
       * We can't store a permanent context in the screen, as it would cause
       * circular refcounting where screens reference contexts that reference
       * resources, while resources reference screens...causing nothing to be
       * freed.  So we just create and destroy a temporary one here.
       */
      ctx = iris_create_context(pscreen, NULL, 0);
      destroy_context = true;
   }

   struct iris_context *ice = (struct iris_context *)ctx;

   iris_reallocate_resource_inplace(ice, res, PIPE_BIND_SHARED);
   assert(res->base.b.bind & PIPE_BIND_SHARED);

   if (destroy_context)
      iris_destroy_context(ctx);
}


static void
iris_resource_disable_aux_on_first_query(struct pipe_resource *resource,
                                         unsigned usage)
{
   struct iris_resource *res = (struct iris_resource *)resource;
   bool mod_with_aux =
      res->mod_info && isl_drm_modifier_has_aux(res->mod_info->modifier);

   /* Disable aux usage if explicit flush not set and this is the first time
    * we are dealing with this resource and the resource was not created with
    * a modifier with aux.
    */
   if (!mod_with_aux &&
      (!(usage & PIPE_HANDLE_USAGE_EXPLICIT_FLUSH) && res->aux.usage != 0) &&
       p_atomic_read(&resource->reference.count) == 1) {
         iris_resource_disable_aux(res);
   }
}

static bool
iris_resource_get_param(struct pipe_screen *pscreen,
                        struct pipe_context *ctx,
                        struct pipe_resource *resource,
                        unsigned plane,
                        unsigned layer,
                        unsigned level,
                        enum pipe_resource_param param,
                        unsigned handle_usage,
                        uint64_t *value)
{
   struct iris_screen *screen = (struct iris_screen *)pscreen;
   struct iris_resource *base_res = (struct iris_resource *)resource;
   unsigned main_plane = get_main_plane_for_plane(base_res->external_format,
                                                  plane);
   struct iris_resource *res =
      (struct iris_resource *)util_resource_at_index(resource, main_plane);
   assert(res);

   bool mod_with_aux =
      res->mod_info && isl_drm_modifier_has_aux(res->mod_info->modifier);
   bool wants_aux = mod_with_aux && plane != main_plane;
   bool wants_cc = mod_with_aux &&
      isl_drm_modifier_plane_is_clear_color(res->mod_info->modifier, plane);
   bool result;
   unsigned handle;

   iris_resource_disable_aux_on_first_query(resource, handle_usage);
   iris_resource_disable_suballoc_on_first_query(pscreen, ctx, res);

   struct iris_bo *bo = wants_cc ? res->aux.clear_color_bo :
                        wants_aux ? res->aux.bo : res->bo;

   assert(iris_bo_is_real(bo));

   switch (param) {
   case PIPE_RESOURCE_PARAM_NPLANES:
      if (mod_with_aux) {
         *value = iris_get_dmabuf_modifier_planes(pscreen,
                                                  res->mod_info->modifier,
                                                  res->external_format);
      } else {
         *value = get_num_planes(&res->base.b);
      }
      return true;
   case PIPE_RESOURCE_PARAM_STRIDE:
      *value = wants_cc ? 64 :
               wants_aux ? res->aux.surf.row_pitch_B : res->surf.row_pitch_B;

      /* Mesa's implementation of eglCreateImage rejects strides of zero (see
       * dri2_check_dma_buf_attribs). Ensure we return a non-zero stride as
       * this value may be queried from GBM and passed into EGL.
       *
       * Also, although modifiers which use a clear color plane specify that
       * the plane's pitch should be ignored, some kernels have been found to
       * require 64-byte alignment.
       */
      assert(*value != 0 && (!wants_cc || *value % 64 == 0));

      return true;
   case PIPE_RESOURCE_PARAM_OFFSET:
      *value = wants_cc ? res->aux.clear_color_offset :
               wants_aux ? res->aux.offset : res->offset;
      return true;
   case PIPE_RESOURCE_PARAM_MODIFIER:
      *value = res->mod_info ? res->mod_info->modifier :
               tiling_to_modifier(isl_tiling_to_i915_tiling(res->surf.tiling));
      return true;
   case PIPE_RESOURCE_PARAM_HANDLE_TYPE_SHARED:
      if (!wants_aux)
         iris_gem_set_tiling(bo, &res->surf);

      result = iris_bo_flink(bo, &handle) == 0;
      if (result)
         *value = handle;
      return result;
   case PIPE_RESOURCE_PARAM_HANDLE_TYPE_KMS: {
      if (!wants_aux)
         iris_gem_set_tiling(bo, &res->surf);

      /* Because we share the same drm file across multiple iris_screen, when
       * we export a GEM handle we must make sure it is valid in the DRM file
       * descriptor the caller is using (this is the FD given at screen
       * creation).
       */
      uint32_t handle;
      if (iris_bo_export_gem_handle_for_device(bo, screen->winsys_fd, &handle))
         return false;
      *value = handle;
      return true;
   }

   case PIPE_RESOURCE_PARAM_HANDLE_TYPE_FD:
      if (!wants_aux)
         iris_gem_set_tiling(bo, &res->surf);

      result = iris_bo_export_dmabuf(bo, (int *) &handle) == 0;
      if (result)
         *value = handle;
      return result;
   default:
      return false;
   }
}

static bool
iris_resource_get_handle(struct pipe_screen *pscreen,
                         struct pipe_context *ctx,
                         struct pipe_resource *resource,
                         struct winsys_handle *whandle,
                         unsigned usage)
{
   struct iris_screen *screen = (struct iris_screen *) pscreen;
   struct iris_resource *res = (struct iris_resource *)resource;
   bool mod_with_aux =
      res->mod_info && isl_drm_modifier_has_aux(res->mod_info->modifier);

   iris_resource_disable_aux_on_first_query(resource, usage);
   iris_resource_disable_suballoc_on_first_query(pscreen, ctx, res);

   assert(iris_bo_is_real(res->bo));

   struct iris_bo *bo;
   if (res->mod_info &&
       isl_drm_modifier_plane_is_clear_color(res->mod_info->modifier,
                                             whandle->plane)) {
      bo = res->aux.clear_color_bo;
      whandle->offset = res->aux.clear_color_offset;
   } else if (mod_with_aux && whandle->plane > 0) {
      bo = res->aux.bo;
      whandle->stride = res->aux.surf.row_pitch_B;
      whandle->offset = res->aux.offset;
   } else {
      /* If this is a buffer, stride should be 0 - no need to special case */
      whandle->stride = res->surf.row_pitch_B;
      bo = res->bo;
   }

   whandle->format = res->external_format;
   whandle->modifier =
      res->mod_info ? res->mod_info->modifier
                    : tiling_to_modifier(isl_tiling_to_i915_tiling(res->surf.tiling));

#ifndef NDEBUG
   enum isl_aux_usage allowed_usage =
      (usage & PIPE_HANDLE_USAGE_EXPLICIT_FLUSH) || mod_with_aux ?
      res->aux.usage : ISL_AUX_USAGE_NONE;

   if (res->aux.usage != allowed_usage) {
      enum isl_aux_state aux_state = iris_resource_get_aux_state(res, 0, 0);
      assert(aux_state == ISL_AUX_STATE_RESOLVED ||
             aux_state == ISL_AUX_STATE_PASS_THROUGH);
   }
#endif

   switch (whandle->type) {
   case WINSYS_HANDLE_TYPE_SHARED:
      iris_gem_set_tiling(bo, &res->surf);
      return iris_bo_flink(bo, &whandle->handle) == 0;
   case WINSYS_HANDLE_TYPE_KMS: {
      iris_gem_set_tiling(bo, &res->surf);

      /* Because we share the same drm file across multiple iris_screen, when
       * we export a GEM handle we must make sure it is valid in the DRM file
       * descriptor the caller is using (this is the FD given at screen
       * creation).
       */
      uint32_t handle;
      if (iris_bo_export_gem_handle_for_device(bo, screen->winsys_fd, &handle))
         return false;
      whandle->handle = handle;
      return true;
   }
   case WINSYS_HANDLE_TYPE_FD:
      iris_gem_set_tiling(bo, &res->surf);
      return iris_bo_export_dmabuf(bo, (int *) &whandle->handle) == 0;
   }

   return false;
}

static bool
resource_is_busy(struct iris_context *ice,
                 struct iris_resource *res)
{
   bool busy = iris_bo_busy(res->bo);

   iris_foreach_batch(ice, batch)
      busy |= iris_batch_references(batch, res->bo);

   return busy;
}

void
iris_replace_buffer_storage(struct pipe_context *ctx,
                            struct pipe_resource *p_dst,
                            struct pipe_resource *p_src,
                            unsigned num_rebinds,
                            uint32_t rebind_mask,
                            uint32_t delete_buffer_id)
{
   struct iris_screen *screen = (void *) ctx->screen;
   struct iris_context *ice = (void *) ctx;
   struct iris_resource *dst = (void *) p_dst;
   struct iris_resource *src = (void *) p_src;

   assert(memcmp(&dst->surf, &src->surf, sizeof(dst->surf)) == 0);

   struct iris_bo *old_bo = dst->bo;

   /* Swap out the backing storage */
   iris_bo_reference(src->bo);
   dst->bo = src->bo;

   /* Rebind the buffer, replacing any state referring to the old BO's
    * address, and marking state dirty so it's reemitted.
    */
   screen->vtbl.rebind_buffer(ice, dst);

   iris_bo_unreference(old_bo);
}

/**
 * Discard a buffer's contents and replace it's backing storage with a
 * fresh, idle buffer if necessary.
 *
 * Returns true if the storage can be considered idle.
 */
static bool
iris_invalidate_buffer(struct iris_context *ice, struct iris_resource *res)
{
   struct iris_screen *screen = (void *) ice->ctx.screen;

   if (res->base.b.target != PIPE_BUFFER)
      return false;

   /* If it's already invalidated, don't bother doing anything.
    * We consider the storage to be idle, because either it was freshly
    * allocated (and not busy), or a previous call here was what cleared
    * the range, and that call replaced the storage with an idle buffer.
    */
   if (res->valid_buffer_range.start > res->valid_buffer_range.end)
      return true;

   if (!resource_is_busy(ice, res)) {
      /* The resource is idle, so just mark that it contains no data and
       * keep using the same underlying buffer object.
       */
      util_range_set_empty(&res->valid_buffer_range);
      return true;
   }

   /* Otherwise, try and replace the backing storage with a new BO. */

   /* We can't reallocate memory we didn't allocate in the first place. */
   if (res->bo->gem_handle && res->bo->real.userptr)
      return false;

   /* Nor can we allocate buffers we imported or exported. */
   if (iris_bo_is_external(res->bo))
      return false;

   struct iris_bo *old_bo = res->bo;
   unsigned flags = old_bo->real.protected ? BO_ALLOC_PROTECTED : BO_ALLOC_PLAIN;
   struct iris_bo *new_bo =
      iris_bo_alloc(screen->bufmgr, res->bo->name, res->base.b.width0,
                    iris_buffer_alignment(res->base.b.width0),
                    iris_memzone_for_address(old_bo->address),
                    flags);
   if (!new_bo)
      return false;

   /* Swap out the backing storage */
   res->bo = new_bo;

   /* Rebind the buffer, replacing any state referring to the old BO's
    * address, and marking state dirty so it's reemitted.
    */
   screen->vtbl.rebind_buffer(ice, res);

   util_range_set_empty(&res->valid_buffer_range);

   iris_bo_unreference(old_bo);

   /* The new buffer is idle. */
   return true;
}

static void
iris_invalidate_resource(struct pipe_context *ctx,
                         struct pipe_resource *resource)
{
   struct iris_context *ice = (void *) ctx;
   struct iris_resource *res = (void *) resource;

   iris_invalidate_buffer(ice, res);
}

static void
iris_flush_staging_region(struct pipe_transfer *xfer,
                          const struct pipe_box *flush_box)
{
   if (!(xfer->usage & PIPE_MAP_WRITE))
      return;

   struct iris_transfer *map = (void *) xfer;

   struct pipe_box src_box = *flush_box;

   /* Account for extra alignment padding in staging buffer */
   if (xfer->resource->target == PIPE_BUFFER)
      src_box.x += xfer->box.x % IRIS_MAP_BUFFER_ALIGNMENT;

   struct pipe_box dst_box = (struct pipe_box) {
      .x = xfer->box.x + flush_box->x,
      .y = xfer->box.y + flush_box->y,
      .z = xfer->box.z + flush_box->z,
      .width = flush_box->width,
      .height = flush_box->height,
      .depth = flush_box->depth,
   };

   iris_copy_region(map->blorp, map->batch, xfer->resource, xfer->level,
                    dst_box.x, dst_box.y, dst_box.z, map->staging, 0,
                    &src_box);
}

static void
iris_unmap_copy_region(struct iris_transfer *map)
{
   iris_resource_destroy(map->staging->screen, map->staging);

   map->ptr = NULL;
}

static void
iris_map_copy_region(struct iris_transfer *map)
{
   struct pipe_screen *pscreen = &map->batch->screen->base;
   struct pipe_transfer *xfer = &map->base.b;
   struct pipe_box *box = &xfer->box;
   struct iris_resource *res = (void *) xfer->resource;

   unsigned extra = xfer->resource->target == PIPE_BUFFER ?
                    box->x % IRIS_MAP_BUFFER_ALIGNMENT : 0;

   struct pipe_resource templ = (struct pipe_resource) {
      .usage = PIPE_USAGE_STAGING,
      .width0 = box->width + extra,
      .height0 = box->height,
      .depth0 = 1,
      .nr_samples = xfer->resource->nr_samples,
      .nr_storage_samples = xfer->resource->nr_storage_samples,
      .array_size = box->depth,
      .format = res->internal_format,
   };

   if (xfer->resource->target == PIPE_BUFFER) {
      templ.target = PIPE_BUFFER;
      map->staging = iris_resource_create_for_buffer(pscreen, &templ);
   } else {
      templ.target = templ.array_size > 1 ? PIPE_TEXTURE_2D_ARRAY
                                          : PIPE_TEXTURE_2D;

      unsigned row_pitch_B = 0;

#ifdef ANDROID
      /* Staging buffers for stall-avoidance blits don't always have the
       * same restrictions on stride as the original buffer.  For example,
       * the original buffer may be used for scanout, while the staging
       * buffer will not be.  So we may compute a smaller stride for the
       * staging buffer than the original.
       *
       * Normally, this is good, as it saves memory.  Unfortunately, for
       * Android, gbm_gralloc incorrectly asserts that the stride returned
       * by gbm_bo_map() must equal the result of gbm_bo_get_stride(),
       * which simply isn't always the case.
       *
       * Because gralloc is unlikely to be fixed, we hack around it in iris
       * by forcing the staging buffer to have a matching stride.
       */
      if (iris_bo_is_external(res->bo))
         row_pitch_B = res->surf.row_pitch_B;
#endif

      map->staging =
         iris_resource_create_for_image(pscreen, &templ, NULL, 0, row_pitch_B);
   }

   /* If we fail to create a staging resource, the caller will fallback
    * to mapping directly on the CPU.
    */
   if (!map->staging)
      return;

   if (templ.target != PIPE_BUFFER) {
      struct isl_surf *surf = &((struct iris_resource *) map->staging)->surf;
      xfer->stride = isl_surf_get_row_pitch_B(surf);
      xfer->layer_stride = isl_surf_get_array_pitch(surf);
   }

   if ((xfer->usage & PIPE_MAP_READ) ||
       (res->base.b.target == PIPE_BUFFER &&
        !(xfer->usage & PIPE_MAP_DISCARD_RANGE))) {
      iris_copy_region(map->blorp, map->batch, map->staging, 0, extra, 0, 0,
                       xfer->resource, xfer->level, box);
      /* Ensure writes to the staging BO land before we map it below. */
      iris_emit_pipe_control_flush(map->batch,
                                   "transfer read: flush before mapping",
                                   PIPE_CONTROL_RENDER_TARGET_FLUSH |
                                   PIPE_CONTROL_TILE_CACHE_FLUSH |
                                   PIPE_CONTROL_CS_STALL);
   }

   struct iris_bo *staging_bo = iris_resource_bo(map->staging);

   if (iris_batch_references(map->batch, staging_bo))
      iris_batch_flush(map->batch);

   assert(((struct iris_resource *)map->staging)->offset == 0);
   map->ptr =
      iris_bo_map(map->dbg, staging_bo, xfer->usage & MAP_FLAGS) + extra;

   map->unmap = iris_unmap_copy_region;
}

static void
get_image_offset_el(const struct isl_surf *surf, unsigned level, unsigned z,
                    unsigned *out_x0_el, unsigned *out_y0_el)
{
   ASSERTED uint32_t z0_el, a0_el;
   if (surf->dim == ISL_SURF_DIM_3D) {
      isl_surf_get_image_offset_el(surf, level, 0, z,
                                   out_x0_el, out_y0_el, &z0_el, &a0_el);
   } else {
      isl_surf_get_image_offset_el(surf, level, z, 0,
                                   out_x0_el, out_y0_el, &z0_el, &a0_el);
   }
   assert(z0_el == 0 && a0_el == 0);
}

/**
 * Get pointer offset into stencil buffer.
 *
 * The stencil buffer is W tiled. Since the GTT is incapable of W fencing, we
 * must decode the tile's layout in software.
 *
 * See
 *   - PRM, 2011 Sandy Bridge, Volume 1, Part 2, Section 4.5.2.1 W-Major Tile
 *     Format.
 *   - PRM, 2011 Sandy Bridge, Volume 1, Part 2, Section 4.5.3 Tiling Algorithm
 *
 * Even though the returned offset is always positive, the return type is
 * signed due to
 *    commit e8b1c6d6f55f5be3bef25084fdd8b6127517e137
 *    mesa: Fix return type of  _mesa_get_format_bytes() (#37351)
 */
static intptr_t
s8_offset(uint32_t stride, uint32_t x, uint32_t y)
{
   uint32_t tile_size = 4096;
   uint32_t tile_width = 64;
   uint32_t tile_height = 64;
   uint32_t row_size = 64 * stride / 2; /* Two rows are interleaved. */

   uint32_t tile_x = x / tile_width;
   uint32_t tile_y = y / tile_height;

   /* The byte's address relative to the tile's base addres. */
   uint32_t byte_x = x % tile_width;
   uint32_t byte_y = y % tile_height;

   uintptr_t u = tile_y * row_size
               + tile_x * tile_size
               + 512 * (byte_x / 8)
               +  64 * (byte_y / 8)
               +  32 * ((byte_y / 4) % 2)
               +  16 * ((byte_x / 4) % 2)
               +   8 * ((byte_y / 2) % 2)
               +   4 * ((byte_x / 2) % 2)
               +   2 * (byte_y % 2)
               +   1 * (byte_x % 2);

   return u;
}

static void
iris_unmap_s8(struct iris_transfer *map)
{
   struct pipe_transfer *xfer = &map->base.b;
   const struct pipe_box *box = &xfer->box;
   struct iris_resource *res = (struct iris_resource *) xfer->resource;
   struct isl_surf *surf = &res->surf;

   if (xfer->usage & PIPE_MAP_WRITE) {
      uint8_t *untiled_s8_map = map->ptr;
      uint8_t *tiled_s8_map = res->offset +
         iris_bo_map(map->dbg, res->bo, (xfer->usage | MAP_RAW) & MAP_FLAGS);

      for (int s = 0; s < box->depth; s++) {
         unsigned x0_el, y0_el;
         get_image_offset_el(surf, xfer->level, box->z + s, &x0_el, &y0_el);

         for (uint32_t y = 0; y < box->height; y++) {
            for (uint32_t x = 0; x < box->width; x++) {
               ptrdiff_t offset = s8_offset(surf->row_pitch_B,
                                            x0_el + box->x + x,
                                            y0_el + box->y + y);
               tiled_s8_map[offset] =
                  untiled_s8_map[s * xfer->layer_stride + y * xfer->stride + x];
            }
         }
      }
   }

   free(map->buffer);
}

static void
iris_map_s8(struct iris_transfer *map)
{
   struct pipe_transfer *xfer = &map->base.b;
   const struct pipe_box *box = &xfer->box;
   struct iris_resource *res = (struct iris_resource *) xfer->resource;
   struct isl_surf *surf = &res->surf;

   xfer->stride = surf->row_pitch_B;
   xfer->layer_stride = xfer->stride * box->height;

   /* The tiling and detiling functions require that the linear buffer has
    * a 16-byte alignment (that is, its `x0` is 16-byte aligned).  Here we
    * over-allocate the linear buffer to get the proper alignment.
    */
   map->buffer = map->ptr = malloc(xfer->layer_stride * box->depth);
   assert(map->buffer);

   /* One of either READ_BIT or WRITE_BIT or both is set.  READ_BIT implies no
    * INVALIDATE_RANGE_BIT.  WRITE_BIT needs the original values read in unless
    * invalidate is set, since we'll be writing the whole rectangle from our
    * temporary buffer back out.
    */
   if (xfer->usage & PIPE_MAP_READ) {
      uint8_t *untiled_s8_map = map->ptr;
      uint8_t *tiled_s8_map = res->offset +
         iris_bo_map(map->dbg, res->bo, (xfer->usage | MAP_RAW) & MAP_FLAGS);

      for (int s = 0; s < box->depth; s++) {
         unsigned x0_el, y0_el;
         get_image_offset_el(surf, xfer->level, box->z + s, &x0_el, &y0_el);

         for (uint32_t y = 0; y < box->height; y++) {
            for (uint32_t x = 0; x < box->width; x++) {
               ptrdiff_t offset = s8_offset(surf->row_pitch_B,
                                            x0_el + box->x + x,
                                            y0_el + box->y + y);
               untiled_s8_map[s * xfer->layer_stride + y * xfer->stride + x] =
                  tiled_s8_map[offset];
            }
         }
      }
   }

   map->unmap = iris_unmap_s8;
}

/* Compute extent parameters for use with tiled_memcpy functions.
 * xs are in units of bytes and ys are in units of strides.
 */
static inline void
tile_extents(const struct isl_surf *surf,
             const struct pipe_box *box,
             unsigned level, int z,
             unsigned *x1_B, unsigned *x2_B,
             unsigned *y1_el, unsigned *y2_el)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(surf->format);
   const unsigned cpp = fmtl->bpb / 8;

   assert(box->x % fmtl->bw == 0);
   assert(box->y % fmtl->bh == 0);

   unsigned x0_el, y0_el;
   get_image_offset_el(surf, level, box->z + z, &x0_el, &y0_el);

   *x1_B = (box->x / fmtl->bw + x0_el) * cpp;
   *y1_el = box->y / fmtl->bh + y0_el;
   *x2_B = (DIV_ROUND_UP(box->x + box->width, fmtl->bw) + x0_el) * cpp;
   *y2_el = DIV_ROUND_UP(box->y + box->height, fmtl->bh) + y0_el;
}

static void
iris_unmap_tiled_memcpy(struct iris_transfer *map)
{
   struct pipe_transfer *xfer = &map->base.b;
   const struct pipe_box *box = &xfer->box;
   struct iris_resource *res = (struct iris_resource *) xfer->resource;
   struct isl_surf *surf = &res->surf;

   const bool has_swizzling = false;

   if (xfer->usage & PIPE_MAP_WRITE) {
      char *dst = res->offset +
         iris_bo_map(map->dbg, res->bo, (xfer->usage | MAP_RAW) & MAP_FLAGS);

      for (int s = 0; s < box->depth; s++) {
         unsigned x1, x2, y1, y2;
         tile_extents(surf, box, xfer->level, s, &x1, &x2, &y1, &y2);

         void *ptr = map->ptr + s * xfer->layer_stride;

         isl_memcpy_linear_to_tiled(x1, x2, y1, y2, dst, ptr,
                                    surf->row_pitch_B, xfer->stride,
                                    has_swizzling, surf->tiling, ISL_MEMCPY);
      }
   }
   os_free_aligned(map->buffer);
   map->buffer = map->ptr = NULL;
}

static void
iris_map_tiled_memcpy(struct iris_transfer *map)
{
   struct pipe_transfer *xfer = &map->base.b;
   const struct pipe_box *box = &xfer->box;
   struct iris_resource *res = (struct iris_resource *) xfer->resource;
   struct isl_surf *surf = &res->surf;

   xfer->stride = ALIGN(surf->row_pitch_B, 16);
   xfer->layer_stride = xfer->stride * box->height;

   unsigned x1, x2, y1, y2;
   tile_extents(surf, box, xfer->level, 0, &x1, &x2, &y1, &y2);

   /* The tiling and detiling functions require that the linear buffer has
    * a 16-byte alignment (that is, its `x0` is 16-byte aligned).  Here we
    * over-allocate the linear buffer to get the proper alignment.
    */
   map->buffer =
      os_malloc_aligned(xfer->layer_stride * box->depth, 16);
   assert(map->buffer);
   map->ptr = (char *)map->buffer + (x1 & 0xf);

   const bool has_swizzling = false;

   if (xfer->usage & PIPE_MAP_READ) {
      char *src = res->offset +
         iris_bo_map(map->dbg, res->bo, (xfer->usage | MAP_RAW) & MAP_FLAGS);

      for (int s = 0; s < box->depth; s++) {
         unsigned x1, x2, y1, y2;
         tile_extents(surf, box, xfer->level, s, &x1, &x2, &y1, &y2);

         /* Use 's' rather than 'box->z' to rebase the first slice to 0. */
         void *ptr = map->ptr + s * xfer->layer_stride;

         isl_memcpy_tiled_to_linear(x1, x2, y1, y2, ptr, src, xfer->stride,
                                    surf->row_pitch_B, has_swizzling,
                                    surf->tiling,
#if defined(USE_SSE41)
                                    util_get_cpu_caps()->has_sse4_1 ?
                                    ISL_MEMCPY_STREAMING_LOAD :
#endif
                                    ISL_MEMCPY);
      }
   }

   map->unmap = iris_unmap_tiled_memcpy;
}

static void
iris_map_direct(struct iris_transfer *map)
{
   struct pipe_transfer *xfer = &map->base.b;
   struct pipe_box *box = &xfer->box;
   struct iris_resource *res = (struct iris_resource *) xfer->resource;

   void *ptr = res->offset +
      iris_bo_map(map->dbg, res->bo, xfer->usage & MAP_FLAGS);

   if (res->base.b.target == PIPE_BUFFER) {
      xfer->stride = 0;
      xfer->layer_stride = 0;

      map->ptr = ptr + box->x;
   } else {
      struct isl_surf *surf = &res->surf;
      const struct isl_format_layout *fmtl =
         isl_format_get_layout(surf->format);
      const unsigned cpp = fmtl->bpb / 8;
      unsigned x0_el, y0_el;

      assert(box->x % fmtl->bw == 0);
      assert(box->y % fmtl->bh == 0);
      get_image_offset_el(surf, xfer->level, box->z, &x0_el, &y0_el);

      x0_el += box->x / fmtl->bw;
      y0_el += box->y / fmtl->bh;

      xfer->stride = isl_surf_get_row_pitch_B(surf);
      xfer->layer_stride = isl_surf_get_array_pitch(surf);

      map->ptr = ptr + y0_el * xfer->stride + x0_el * cpp;
   }
}

static bool
can_promote_to_async(const struct iris_resource *res,
                     const struct pipe_box *box,
                     enum pipe_map_flags usage)
{
   /* If we're writing to a section of the buffer that hasn't even been
    * initialized with useful data, then we can safely promote this write
    * to be unsynchronized.  This helps the common pattern of appending data.
    */
   return res->base.b.target == PIPE_BUFFER && (usage & PIPE_MAP_WRITE) &&
          !(usage & TC_TRANSFER_MAP_NO_INFER_UNSYNCHRONIZED) &&
          !util_ranges_intersect(&res->valid_buffer_range, box->x,
                                 box->x + box->width);
}

static bool
prefer_cpu_access(const struct iris_resource *res,
                  const struct pipe_box *box,
                  enum pipe_map_flags usage,
                  unsigned level,
                  bool map_would_stall)
{
   const enum iris_mmap_mode mmap_mode = iris_bo_mmap_mode(res->bo);

   /* We must be able to map it. */
   if (mmap_mode == IRIS_MMAP_NONE)
      return false;

   const bool write = usage & PIPE_MAP_WRITE;
   const bool read = usage & PIPE_MAP_READ;
   const bool preserve =
      res->base.b.target == PIPE_BUFFER && !(usage & PIPE_MAP_DISCARD_RANGE);

   /* We want to avoid uncached reads because they are slow. */
   if (read && mmap_mode != IRIS_MMAP_WB)
      return false;

   /* We want to avoid stalling.  We can't avoid stalling for reads, though,
    * because the destination of a GPU staging copy would be busy and stall
    * in the exact same manner.  So don't consider it for those.
    *
    * For buffer maps which aren't invalidating the destination, the GPU
    * staging copy path would have to read the existing buffer contents in
    * order to preserve them, effectively making it a read.  But a direct
    * mapping would be able to just write the necessary parts without the
    * overhead of the copy.  It may stall, but we would anyway.
    */
   if (map_would_stall && !read && !preserve)
      return false;

   /* Use the GPU for writes if it would compress the data. */
   if (write && isl_aux_usage_has_compression(res->aux.usage))
      return false;

   /* Writes & Cached CPU reads are fine as long as the primary is valid. */
   return !iris_has_invalid_primary(res, level, 1, box->z, box->depth);
}

static void *
iris_transfer_map(struct pipe_context *ctx,
                  struct pipe_resource *resource,
                  unsigned level,
                  enum pipe_map_flags usage,
                  const struct pipe_box *box,
                  struct pipe_transfer **ptransfer)
{
   struct iris_context *ice = (struct iris_context *)ctx;
   struct iris_resource *res = (struct iris_resource *)resource;
   struct isl_surf *surf = &res->surf;

   /* From GL_AMD_pinned_memory issues:
    *
    *     4) Is glMapBuffer on a shared buffer guaranteed to return the
    *        same system address which was specified at creation time?
    *
    *        RESOLVED: NO. The GL implementation might return a different
    *        virtual mapping of that memory, although the same physical
    *        page will be used.
    *
    * So don't ever use staging buffers.
    */
   if (res->base.is_user_ptr)
      usage |= PIPE_MAP_PERSISTENT;

   /* Promote discarding a range to discarding the entire buffer where
    * possible.  This may allow us to replace the backing storage entirely
    * and let us do an unsynchronized map when we otherwise wouldn't.
    */
   if (resource->target == PIPE_BUFFER &&
       (usage & PIPE_MAP_DISCARD_RANGE) &&
       box->x == 0 && box->width == resource->width0) {
      usage |= PIPE_MAP_DISCARD_WHOLE_RESOURCE;
   }

   if (usage & PIPE_MAP_DISCARD_WHOLE_RESOURCE) {
      /* Replace the backing storage with a fresh buffer for non-async maps */
      if (!(usage & (PIPE_MAP_UNSYNCHRONIZED | TC_TRANSFER_MAP_NO_INVALIDATE))
          && iris_invalidate_buffer(ice, res))
         usage |= PIPE_MAP_UNSYNCHRONIZED;

      /* If we can discard the whole resource, we can discard the range. */
      usage |= PIPE_MAP_DISCARD_RANGE;
   }

   if (!(usage & PIPE_MAP_UNSYNCHRONIZED) &&
       can_promote_to_async(res, box, usage)) {
      usage |= PIPE_MAP_UNSYNCHRONIZED;
   }

   /* Avoid using GPU copies for persistent/coherent buffers, as the idea
    * there is to access them simultaneously on the CPU & GPU.  This also
    * avoids trying to use GPU copies for our u_upload_mgr buffers which
    * contain state we're constructing for a GPU draw call, which would
    * kill us with infinite stack recursion.
    */
   if (usage & (PIPE_MAP_PERSISTENT | PIPE_MAP_COHERENT))
      usage |= PIPE_MAP_DIRECTLY;

   /* We cannot provide a direct mapping of tiled resources, and we
    * may not be able to mmap imported BOs since they may come from
    * other devices that I915_GEM_MMAP cannot work with.
    */
   if ((usage & PIPE_MAP_DIRECTLY) &&
       (surf->tiling != ISL_TILING_LINEAR || iris_bo_is_imported(res->bo)))
      return NULL;

   bool map_would_stall = false;

   if (!(usage & PIPE_MAP_UNSYNCHRONIZED)) {
      map_would_stall =
         resource_is_busy(ice, res) ||
         iris_has_invalid_primary(res, level, 1, box->z, box->depth);

      if (map_would_stall && (usage & PIPE_MAP_DONTBLOCK) &&
                             (usage & PIPE_MAP_DIRECTLY))
         return NULL;
   }

   struct iris_transfer *map;

   if (usage & PIPE_MAP_THREAD_SAFE)
      map = CALLOC_STRUCT(iris_transfer);
   else if (usage & TC_TRANSFER_MAP_THREADED_UNSYNC)
      map = slab_zalloc(&ice->transfer_pool_unsync);
   else
      map = slab_zalloc(&ice->transfer_pool);

   if (!map)
      return NULL;

   struct pipe_transfer *xfer = &map->base.b;

   map->dbg = &ice->dbg;

   pipe_resource_reference(&xfer->resource, resource);
   xfer->level = level;
   xfer->usage = usage;
   xfer->box = *box;
   *ptransfer = xfer;

   if (usage & PIPE_MAP_WRITE)
      util_range_add(&res->base.b, &res->valid_buffer_range, box->x, box->x + box->width);

   if (prefer_cpu_access(res, box, usage, level, map_would_stall))
      usage |= PIPE_MAP_DIRECTLY;

   /* TODO: Teach iris_map_tiled_memcpy about Tile64... */
   if (res->surf.tiling == ISL_TILING_64)
      usage &= ~PIPE_MAP_DIRECTLY;

   if (!(usage & PIPE_MAP_DIRECTLY)) {
      /* If we need a synchronous mapping and the resource is busy, or needs
       * resolving, we copy to/from a linear temporary buffer using the GPU.
       */
      map->batch = &ice->batches[IRIS_BATCH_RENDER];
      map->blorp = &ice->blorp;
      iris_map_copy_region(map);
   }

   /* If we've requested a direct mapping, or iris_map_copy_region failed
    * to create a staging resource, then map it directly on the CPU.
    */
   if (!map->ptr) {
      if (resource->target != PIPE_BUFFER) {
         iris_resource_access_raw(ice, res, level, box->z, box->depth,
                                  usage & PIPE_MAP_WRITE);
      }

      if (!(usage & PIPE_MAP_UNSYNCHRONIZED)) {
         iris_foreach_batch(ice, batch) {
            if (iris_batch_references(batch, res->bo))
               iris_batch_flush(batch);
         }
      }

      if (surf->tiling == ISL_TILING_W) {
         /* TODO: Teach iris_map_tiled_memcpy about W-tiling... */
         iris_map_s8(map);
      } else if (surf->tiling != ISL_TILING_LINEAR) {
         iris_map_tiled_memcpy(map);
      } else {
         iris_map_direct(map);
      }
   }

   return map->ptr;
}

static void
iris_transfer_flush_region(struct pipe_context *ctx,
                           struct pipe_transfer *xfer,
                           const struct pipe_box *box)
{
   struct iris_context *ice = (struct iris_context *)ctx;
   struct iris_resource *res = (struct iris_resource *) xfer->resource;
   struct iris_transfer *map = (void *) xfer;

   if (map->staging)
      iris_flush_staging_region(xfer, box);

   if (res->base.b.target == PIPE_BUFFER) {
      util_range_add(&res->base.b, &res->valid_buffer_range, box->x, box->x + box->width);
   }

   /* Make sure we flag constants dirty even if there's no need to emit
    * any PIPE_CONTROLs to a batch.
    */
   iris_dirty_for_history(ice, res);
}

static void
iris_transfer_unmap(struct pipe_context *ctx, struct pipe_transfer *xfer)
{
   struct iris_context *ice = (struct iris_context *)ctx;
   struct iris_transfer *map = (void *) xfer;

   if (!(xfer->usage & (PIPE_MAP_FLUSH_EXPLICIT |
                        PIPE_MAP_COHERENT))) {
      struct pipe_box flush_box = {
         .x = 0, .y = 0, .z = 0,
         .width  = xfer->box.width,
         .height = xfer->box.height,
         .depth  = xfer->box.depth,
      };
      iris_transfer_flush_region(ctx, xfer, &flush_box);
   }

   if (map->unmap)
      map->unmap(map);

   pipe_resource_reference(&xfer->resource, NULL);

   if (xfer->usage & PIPE_MAP_THREAD_SAFE) {
      free(map);
   } else {
      /* transfer_unmap is called from the driver thread, so we have to use
       * transfer_pool, not transfer_pool_unsync.  Freeing an object into a
       * different pool is allowed, however.
       */
      slab_free(&ice->transfer_pool, map);
   }
}

/**
 * The pipe->texture_subdata() driver hook.
 *
 * Mesa's state tracker takes this path whenever possible, even with
 * PIPE_CAP_TEXTURE_TRANSFER_MODES set.
 */
static void
iris_texture_subdata(struct pipe_context *ctx,
                     struct pipe_resource *resource,
                     unsigned level,
                     unsigned usage,
                     const struct pipe_box *box,
                     const void *data,
                     unsigned stride,
                     uintptr_t layer_stride)
{
   struct iris_context *ice = (struct iris_context *)ctx;
   struct iris_resource *res = (struct iris_resource *)resource;
   const struct isl_surf *surf = &res->surf;

   assert(resource->target != PIPE_BUFFER);

   /* Just use the transfer-based path for linear buffers - it will already
    * do a direct mapping, or a simple linear staging buffer.
    *
    * Linear staging buffers appear to be better than tiled ones, too, so
    * take that path if we need the GPU to perform color compression, or
    * stall-avoidance blits.
    *
    * TODO: Teach isl_memcpy_linear_to_tiled about Tile64...
    */
   if (surf->tiling == ISL_TILING_LINEAR ||
       surf->tiling == ISL_TILING_64 ||
       isl_aux_usage_has_compression(res->aux.usage) ||
       resource_is_busy(ice, res) ||
       iris_bo_mmap_mode(res->bo) == IRIS_MMAP_NONE) {
      return u_default_texture_subdata(ctx, resource, level, usage, box,
                                       data, stride, layer_stride);
   }

   /* No state trackers pass any flags other than PIPE_MAP_WRITE */

   iris_resource_access_raw(ice, res, level, box->z, box->depth, true);

   iris_foreach_batch(ice, batch) {
      if (iris_batch_references(batch, res->bo))
         iris_batch_flush(batch);
   }

   uint8_t *dst = iris_bo_map(&ice->dbg, res->bo, MAP_WRITE | MAP_RAW);

   for (int s = 0; s < box->depth; s++) {
      const uint8_t *src = data + s * layer_stride;

      if (surf->tiling == ISL_TILING_W) {
         unsigned x0_el, y0_el;
         get_image_offset_el(surf, level, box->z + s, &x0_el, &y0_el);

         for (unsigned y = 0; y < box->height; y++) {
            for (unsigned x = 0; x < box->width; x++) {
               ptrdiff_t offset = s8_offset(surf->row_pitch_B,
                                            x0_el + box->x + x,
                                            y0_el + box->y + y);
               dst[offset] = src[y * stride + x];
            }
         }
      } else {
         unsigned x1, x2, y1, y2;

         tile_extents(surf, box, level, s, &x1, &x2, &y1, &y2);

         isl_memcpy_linear_to_tiled(x1, x2, y1, y2,
                                    (void *)dst, (void *)src,
                                    surf->row_pitch_B, stride,
                                    false, surf->tiling, ISL_MEMCPY);
      }
   }
}

/**
 * Mark state dirty that needs to be re-emitted when a resource is written.
 */
void
iris_dirty_for_history(struct iris_context *ice,
                       struct iris_resource *res)
{
   const uint64_t stages = res->bind_stages;
   uint64_t dirty = 0ull;
   uint64_t stage_dirty = 0ull;

   if (res->bind_history & PIPE_BIND_CONSTANT_BUFFER) {
      for (unsigned stage = 0; stage < MESA_SHADER_STAGES; stage++) {
         if (stages & (1u << stage)) {
            struct iris_shader_state *shs = &ice->state.shaders[stage];
            shs->dirty_cbufs |= ~0u;
         }
      }
      dirty |= IRIS_DIRTY_RENDER_MISC_BUFFER_FLUSHES |
               IRIS_DIRTY_COMPUTE_MISC_BUFFER_FLUSHES;
      stage_dirty |= (stages << IRIS_SHIFT_FOR_STAGE_DIRTY_CONSTANTS);
   }

   if (res->bind_history & (PIPE_BIND_SAMPLER_VIEW |
                            PIPE_BIND_SHADER_IMAGE)) {
      dirty |= IRIS_DIRTY_RENDER_RESOLVES_AND_FLUSHES |
               IRIS_DIRTY_COMPUTE_RESOLVES_AND_FLUSHES;
      stage_dirty |= (stages << IRIS_SHIFT_FOR_STAGE_DIRTY_BINDINGS);
   }

   if (res->bind_history & PIPE_BIND_SHADER_BUFFER) {
      dirty |= IRIS_DIRTY_RENDER_MISC_BUFFER_FLUSHES |
               IRIS_DIRTY_COMPUTE_MISC_BUFFER_FLUSHES;
      stage_dirty |= (stages << IRIS_SHIFT_FOR_STAGE_DIRTY_BINDINGS);
   }

   if (res->bind_history & PIPE_BIND_VERTEX_BUFFER)
      dirty |= IRIS_DIRTY_VERTEX_BUFFER_FLUSHES;

   if (ice->state.streamout_active && (res->bind_history & PIPE_BIND_STREAM_OUTPUT))
      dirty |= IRIS_DIRTY_SO_BUFFERS;

   ice->state.dirty |= dirty;
   ice->state.stage_dirty |= stage_dirty;
}

bool
iris_resource_set_clear_color(struct iris_context *ice,
                              struct iris_resource *res,
                              union isl_color_value color)
{
   if (res->aux.clear_color_unknown ||
       memcmp(&res->aux.clear_color, &color, sizeof(color)) != 0) {
      res->aux.clear_color = color;
      res->aux.clear_color_unknown = false;
      return true;
   }

   return false;
}

static enum pipe_format
iris_resource_get_internal_format(struct pipe_resource *p_res)
{
   struct iris_resource *res = (void *) p_res;
   return res->internal_format;
}

static const struct u_transfer_vtbl transfer_vtbl = {
   .resource_create       = iris_resource_create,
   .resource_destroy      = iris_resource_destroy,
   .transfer_map          = iris_transfer_map,
   .transfer_unmap        = iris_transfer_unmap,
   .transfer_flush_region = iris_transfer_flush_region,
   .get_internal_format   = iris_resource_get_internal_format,
   .set_stencil           = iris_resource_set_separate_stencil,
   .get_stencil           = iris_resource_get_separate_stencil,
};

void
iris_init_screen_resource_functions(struct pipe_screen *pscreen)
{
   pscreen->query_dmabuf_modifiers = iris_query_dmabuf_modifiers;
   pscreen->is_dmabuf_modifier_supported = iris_is_dmabuf_modifier_supported;
   pscreen->get_dmabuf_modifier_planes = iris_get_dmabuf_modifier_planes;
   pscreen->resource_create_with_modifiers =
      iris_resource_create_with_modifiers;
   pscreen->resource_create = u_transfer_helper_resource_create;
   pscreen->resource_from_user_memory = iris_resource_from_user_memory;
   pscreen->resource_from_handle = iris_resource_from_handle;
   pscreen->resource_from_memobj = iris_resource_from_memobj_wrapper;
   pscreen->resource_get_handle = iris_resource_get_handle;
   pscreen->resource_get_param = iris_resource_get_param;
   pscreen->resource_destroy = u_transfer_helper_resource_destroy;
   pscreen->memobj_create_from_handle = iris_memobj_create_from_handle;
   pscreen->memobj_destroy = iris_memobj_destroy;
   pscreen->transfer_helper =
      u_transfer_helper_create(&transfer_vtbl,
                               U_TRANSFER_HELPER_SEPARATE_Z32S8 |
                               U_TRANSFER_HELPER_SEPARATE_STENCIL |
                               U_TRANSFER_HELPER_MSAA_MAP);
}

void
iris_init_resource_functions(struct pipe_context *ctx)
{
   ctx->flush_resource = iris_flush_resource;
   ctx->invalidate_resource = iris_invalidate_resource;
   ctx->buffer_map = u_transfer_helper_transfer_map;
   ctx->texture_map = u_transfer_helper_transfer_map;
   ctx->transfer_flush_region = u_transfer_helper_transfer_flush_region;
   ctx->buffer_unmap = u_transfer_helper_transfer_unmap;
   ctx->texture_unmap = u_transfer_helper_transfer_unmap;
   ctx->buffer_subdata = u_default_buffer_subdata;
   ctx->clear_buffer = u_default_clear_buffer;
   ctx->texture_subdata = iris_texture_subdata;
}
