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
 * @file crocus_resource.c
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
#include "util/u_threaded_context.h"
#include "util/u_transfer.h"
#include "util/u_transfer_helper.h"
#include "util/u_upload_mgr.h"
#include "util/ralloc.h"
#include "util/u_memory.h"
#include "crocus_batch.h"
#include "crocus_context.h"
#include "crocus_resource.h"
#include "crocus_screen.h"
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
};

static const uint64_t priority_to_modifier[] = {
   [MODIFIER_PRIORITY_INVALID] = DRM_FORMAT_MOD_INVALID,
   [MODIFIER_PRIORITY_LINEAR] = DRM_FORMAT_MOD_LINEAR,
   [MODIFIER_PRIORITY_X] = I915_FORMAT_MOD_X_TILED,
   [MODIFIER_PRIORITY_Y] = I915_FORMAT_MOD_Y_TILED,
};

static bool
modifier_is_supported(const struct intel_device_info *devinfo,
                      enum pipe_format pfmt, unsigned bind,
                      uint64_t modifier)
{
   /* XXX: do something real */
   switch (modifier) {
   case I915_FORMAT_MOD_Y_TILED:
      if (bind & PIPE_BIND_SCANOUT)
         return false;
      return devinfo->ver >= 6;
   case I915_FORMAT_MOD_X_TILED:
   case DRM_FORMAT_MOD_LINEAR:
      return true;
   case DRM_FORMAT_MOD_INVALID:
   default:
      return false;
   }
}

static uint64_t
select_best_modifier(struct intel_device_info *devinfo,
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

static enum isl_surf_dim
crocus_target_to_isl_surf_dim(enum pipe_texture_target target)
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

static isl_surf_usage_flags_t
pipe_bind_to_isl_usage(unsigned bindings)
{
   isl_surf_usage_flags_t usage = 0;

   if (bindings & PIPE_BIND_RENDER_TARGET)
      usage |= ISL_SURF_USAGE_RENDER_TARGET_BIT;

   if (bindings & PIPE_BIND_SAMPLER_VIEW)
      usage |= ISL_SURF_USAGE_TEXTURE_BIT;

   if (bindings & (PIPE_BIND_SHADER_IMAGE | PIPE_BIND_SHADER_BUFFER))
      usage |= ISL_SURF_USAGE_STORAGE_BIT;

   if (bindings & PIPE_BIND_SCANOUT)
      usage |= ISL_SURF_USAGE_DISPLAY_BIT;
   return usage;
}

static bool
crocus_resource_configure_main(const struct crocus_screen *screen,
                               struct crocus_resource *res,
                               const struct pipe_resource *templ,
                               uint64_t modifier, uint32_t row_pitch_B)
{
   const struct intel_device_info *devinfo = &screen->devinfo;
   const struct util_format_description *format_desc =
      util_format_description(templ->format);
   const bool has_depth = util_format_has_depth(format_desc);
   isl_surf_usage_flags_t usage = pipe_bind_to_isl_usage(templ->bind);
   isl_tiling_flags_t tiling_flags = ISL_TILING_ANY_MASK;

   /* TODO: This used to be because there wasn't BLORP to handle Y-tiling. */
   if (devinfo->ver < 6 && !util_format_is_depth_or_stencil(templ->format))
      tiling_flags &= ~ISL_TILING_Y0_BIT;

   if (modifier != DRM_FORMAT_MOD_INVALID) {
      res->mod_info = isl_drm_modifier_get_info(modifier);

      tiling_flags = 1 << res->mod_info->tiling;
   } else {
      if (templ->bind & PIPE_BIND_RENDER_TARGET && devinfo->ver < 6)
         tiling_flags &= ISL_TILING_LINEAR_BIT | ISL_TILING_X_BIT;
      /* Use linear for staging buffers */
      if (templ->usage == PIPE_USAGE_STAGING ||
          templ->bind & (PIPE_BIND_LINEAR | PIPE_BIND_CURSOR) )
         tiling_flags = ISL_TILING_LINEAR_BIT;
      else if (templ->bind & PIPE_BIND_SCANOUT)
         tiling_flags = screen->devinfo.has_tiling_uapi ?
            ISL_TILING_X_BIT : ISL_TILING_LINEAR_BIT;
   }

   if (templ->target == PIPE_TEXTURE_CUBE ||
       templ->target == PIPE_TEXTURE_CUBE_ARRAY)
      usage |= ISL_SURF_USAGE_CUBE_BIT;

   if (templ->usage != PIPE_USAGE_STAGING) {
      if (templ->format == PIPE_FORMAT_S8_UINT)
         usage |= ISL_SURF_USAGE_STENCIL_BIT;
      else if (has_depth) {
         /* combined DS only on gen4/5 */
         if (devinfo->ver < 6) {
            if (templ->format == PIPE_FORMAT_Z24X8_UNORM ||
                templ->format == PIPE_FORMAT_Z24_UNORM_S8_UINT ||
                templ->format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT)
               usage |= ISL_SURF_USAGE_STENCIL_BIT;
         }
         usage |= ISL_SURF_USAGE_DEPTH_BIT;
      }

      if (templ->format == PIPE_FORMAT_S8_UINT)
         tiling_flags = ISL_TILING_W_BIT;
   }

   /* Disable aux for external memory objects. */
   if (!res->mod_info && res->external_format != PIPE_FORMAT_NONE)
      usage |= ISL_SURF_USAGE_DISABLE_AUX_BIT;

   const enum isl_format format =
      crocus_format_for_usage(&screen->devinfo, templ->format, usage).fmt;

   if (row_pitch_B == 0 && templ->usage == PIPE_USAGE_STAGING &&
       templ->target == PIPE_TEXTURE_2D &&
       devinfo->ver < 6) {
      /* align row pitch to 4 so we can keep using BLT engine */
      row_pitch_B = util_format_get_stride(templ->format, templ->width0);
      row_pitch_B = ALIGN(row_pitch_B, 4);
   }

   const struct isl_surf_init_info init_info = {
      .dim = crocus_target_to_isl_surf_dim(templ->target),
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

   /*
    * Don't create staging surfaces that will use > half the aperture
    * since staging implies you are sending to another resource,
    * which there is no way to fit both into aperture.
    */
   if (templ->usage == PIPE_USAGE_STAGING)
      if (res->surf.size_B > screen->aperture_threshold / 2)
         return false;

   res->internal_format = templ->format;

   return true;
}

static void
crocus_query_dmabuf_modifiers(struct pipe_screen *pscreen,
                              enum pipe_format pfmt,
                              int max,
                              uint64_t *modifiers,
                              unsigned int *external_only,
                              int *count)
{
   struct crocus_screen *screen = (void *) pscreen;
   const struct intel_device_info *devinfo = &screen->devinfo;

   uint64_t all_modifiers[] = {
      DRM_FORMAT_MOD_LINEAR,
      I915_FORMAT_MOD_X_TILED,
      I915_FORMAT_MOD_Y_TILED,
   };

   int supported_mods = 0;

   for (int i = 0; i < ARRAY_SIZE(all_modifiers); i++) {
      if (!modifier_is_supported(devinfo, pfmt, 0, all_modifiers[i]))
         continue;

      if (supported_mods < max) {
         if (modifiers)
            modifiers[supported_mods] = all_modifiers[i];

         if (external_only)
            external_only[supported_mods] = util_format_is_yuv(pfmt);
      }

      supported_mods++;
   }

   *count = supported_mods;
}

static struct pipe_resource *
crocus_resource_get_separate_stencil(struct pipe_resource *p_res)
{
   return _crocus_resource_get_separate_stencil(p_res);
}

static void
crocus_resource_set_separate_stencil(struct pipe_resource *p_res,
                                     struct pipe_resource *stencil)
{
   assert(util_format_has_depth(util_format_description(p_res->format)));
   pipe_resource_reference(&p_res->next, stencil);
}

void
crocus_resource_disable_aux(struct crocus_resource *res)
{
   crocus_bo_unreference(res->aux.bo);
   free(res->aux.state);

   res->aux.usage = ISL_AUX_USAGE_NONE;
   res->aux.has_hiz = 0;
   res->aux.surf.size_B = 0;
   res->aux.surf.levels = 0;
   res->aux.bo = NULL;
   res->aux.state = NULL;
}

static void
crocus_resource_destroy(struct pipe_screen *screen,
                        struct pipe_resource *resource)
{
   struct crocus_resource *res = (struct crocus_resource *)resource;

   if (resource->target == PIPE_BUFFER)
      util_range_destroy(&res->valid_buffer_range);

   if (res->shadow)
      pipe_resource_reference((struct pipe_resource **)&res->shadow, NULL);
   crocus_resource_disable_aux(res);

   threaded_resource_deinit(resource);
   crocus_bo_unreference(res->bo);
   crocus_pscreen_unref(res->orig_screen);
   free(res);
}

static struct crocus_resource *
crocus_alloc_resource(struct pipe_screen *pscreen,
                      const struct pipe_resource *templ)
{
   struct crocus_resource *res = calloc(1, sizeof(struct crocus_resource));
   if (!res)
      return NULL;

   res->base.b = *templ;
   res->base.b.screen = pscreen;
   res->orig_screen = crocus_pscreen_ref(pscreen);
   pipe_reference_init(&res->base.b.reference, 1);
   threaded_resource_init(&res->base.b, false);

   if (templ->target == PIPE_BUFFER)
      util_range_init(&res->valid_buffer_range);

   return res;
}

unsigned
crocus_get_num_logical_layers(const struct crocus_resource *res, unsigned level)
{
   if (res->surf.dim == ISL_SURF_DIM_3D)
      return u_minify(res->surf.logical_level0_px.depth, level);
   else
      return res->surf.logical_level0_px.array_len;
}

static enum isl_aux_state **
create_aux_state_map(struct crocus_resource *res, enum isl_aux_state initial)
{
   assert(res->aux.state == NULL);

   uint32_t total_slices = 0;
   for (uint32_t level = 0; level < res->surf.levels; level++)
      total_slices += crocus_get_num_logical_layers(res, level);

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
      const unsigned level_layers = crocus_get_num_logical_layers(res, level);
      for (uint32_t a = 0; a < level_layers; a++)
         *(s++) = initial;
   }
   assert((void *)s == data + total_size);

   return per_level_arr;
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
crocus_resource_configure_aux(struct crocus_screen *screen,
                              struct crocus_resource *res,
                              uint64_t *aux_size_B,
                              uint32_t *alloc_flags)
{
   const struct intel_device_info *devinfo = &screen->devinfo;

   /* Modifiers with compression are not supported. */
   assert(!res->mod_info ||
          !isl_drm_modifier_has_aux(res->mod_info->modifier));

   const bool has_mcs = devinfo->ver >= 7 && !res->mod_info &&
      isl_surf_get_mcs_surf(&screen->isl_dev, &res->surf, &res->aux.surf);

   const bool has_hiz = devinfo->ver >= 6 && !res->mod_info &&
      isl_surf_get_hiz_surf(&screen->isl_dev, &res->surf, &res->aux.surf);

   const bool has_ccs =
      devinfo->ver >= 7 && !res->mod_info &&
      isl_surf_get_ccs_surf(&screen->isl_dev, &res->surf, NULL,
                            &res->aux.surf, 0);

   /* Having more than one type of compression is impossible */
   assert(has_ccs + has_mcs + has_hiz <= 1);

   if (has_mcs) {
      res->aux.usage = ISL_AUX_USAGE_MCS;
   } else if (has_hiz) {
      res->aux.usage = ISL_AUX_USAGE_HIZ;
   } else if (has_ccs) {
      if (isl_format_supports_ccs_d(devinfo, res->surf.format))
         res->aux.usage = ISL_AUX_USAGE_CCS_D;
   }

   enum isl_aux_state initial_state = ISL_AUX_STATE_AUX_INVALID;
   *aux_size_B = 0;
   *alloc_flags = 0;
   assert(!res->aux.bo);

   switch (res->aux.usage) {
   case ISL_AUX_USAGE_NONE:
      res->aux.surf.levels = 0;
      return true;
   case ISL_AUX_USAGE_HIZ:
      initial_state = ISL_AUX_STATE_AUX_INVALID;
      break;
   case ISL_AUX_USAGE_MCS:
      /* The Ivybridge PRM, Vol 2 Part 1 p326 says:
       *
       *    "When MCS buffer is enabled and bound to MSRT, it is required
       *     that it is cleared prior to any rendering."
       *
       * Since we only use the MCS buffer for rendering, we just clear it
       * immediately on allocation.  The clear value for MCS buffers is all
       * 1's, so we simply memset it to 0xff.
       */
      initial_state = ISL_AUX_STATE_CLEAR;
      break;
   case ISL_AUX_USAGE_CCS_D:
      /* When CCS_E is used, we need to ensure that the CCS starts off in
       * a valid state.  From the Sky Lake PRM, "MCS Buffer for Render
       * Target(s)":
       *
       *    "If Software wants to enable Color Compression without Fast
       *     clear, Software needs to initialize MCS with zeros."
       *
       * A CCS value of 0 indicates that the corresponding block is in the
       * pass-through state which is what we want.
       *
       * For CCS_D, do the same thing.  On Gen9+, this avoids having any
       * undefined bits in the aux buffer.
       */
      initial_state = ISL_AUX_STATE_PASS_THROUGH;
      *alloc_flags |= BO_ALLOC_ZEROED;
      break;
   default:
      unreachable("non-crocus aux");
   }

   /* Create the aux_state for the auxiliary buffer. */
   res->aux.state = create_aux_state_map(res, initial_state);
   if (!res->aux.state)
      return false;

   /* Increase the aux offset if the main and aux surfaces will share a BO. */
   res->aux.offset = (uint32_t)align64(res->surf.size_B, res->aux.surf.alignment_B);
   uint64_t size = res->aux.surf.size_B;

   /* Allocate space in the buffer for storing the clear color. On modern
    * platforms (gen > 9), we can read it directly from such buffer.
    *
    * On gen <= 9, we are going to store the clear color on the buffer
    * anyways, and copy it back to the surface state during state emission.
    *
    * Also add some padding to make sure the fast clear color state buffer
    * starts at a 4K alignment. We believe that 256B might be enough, but due
    * to lack of testing we will leave this as 4K for now.
    */
   size = align64(size, 4096);
   *aux_size_B = size;

   if (isl_aux_usage_has_hiz(res->aux.usage)) {
      for (unsigned level = 0; level < res->surf.levels; ++level) {
         uint32_t width = u_minify(res->surf.phys_level0_sa.width, level);
         uint32_t height = u_minify(res->surf.phys_level0_sa.height, level);

         /* Disable HiZ for LOD > 0 unless the width/height are 8x4 aligned.
          * For LOD == 0, we can grow the dimensions to make it work.
          */
         if (devinfo->verx10 < 75 ||
             (level == 0 || ((width & 7) == 0 && (height & 3) == 0)))
            res->aux.has_hiz |= 1 << level;
      }
   }

   return true;
}

/**
 * Initialize the aux buffer contents.
 *
 * Returns false on unexpected error (e.g. mapping a BO failed).
 */
static bool
crocus_resource_init_aux_buf(struct crocus_resource *res, uint32_t alloc_flags)
{
   if (!(alloc_flags & BO_ALLOC_ZEROED)) {
      void *map = crocus_bo_map(NULL, res->aux.bo, MAP_WRITE | MAP_RAW);

      if (!map)
         return false;

      if (crocus_resource_get_aux_state(res, 0, 0) != ISL_AUX_STATE_AUX_INVALID) {
         uint8_t memset_value = isl_aux_usage_has_mcs(res->aux.usage) ? 0xFF : 0;
         memset((char*)map + res->aux.offset, memset_value,
                res->aux.surf.size_B);
      }

      crocus_bo_unmap(res->aux.bo);
   }

   return true;
}

/**
 * Allocate the initial aux surface for a resource based on aux.usage
 *
 * Returns false on unexpected error (e.g. allocation failed, or invalid
 * configuration result).
 */
static bool
crocus_resource_alloc_separate_aux(struct crocus_screen *screen,
                                   struct crocus_resource *res)
{
   uint32_t alloc_flags;
   uint64_t size;
   if (!crocus_resource_configure_aux(screen, res, &size, &alloc_flags))
      return false;

   if (size == 0)
      return true;

   /* Allocate the auxiliary buffer.  ISL has stricter set of alignment rules
    * the drm allocator.  Therefore, one can pass the ISL dimensions in terms
    * of bytes instead of trying to recalculate based on different format
    * block sizes.
    */
   res->aux.bo = crocus_bo_alloc_tiled(screen->bufmgr, "aux buffer", size, 4096,
                                       isl_tiling_to_i915_tiling(res->aux.surf.tiling),
                                       res->aux.surf.row_pitch_B, alloc_flags);
   if (!res->aux.bo) {
      return false;
   }

   if (!crocus_resource_init_aux_buf(res, alloc_flags))
      return false;

   return true;
}

static struct pipe_resource *
crocus_resource_create_for_buffer(struct pipe_screen *pscreen,
                                  const struct pipe_resource *templ)
{
   struct crocus_screen *screen = (struct crocus_screen *)pscreen;
   struct crocus_resource *res = crocus_alloc_resource(pscreen, templ);

   assert(templ->target == PIPE_BUFFER);
   assert(templ->height0 <= 1);
   assert(templ->depth0 <= 1);
   assert(templ->format == PIPE_FORMAT_NONE ||
          util_format_get_blocksize(templ->format) == 1);

   res->internal_format = templ->format;
   res->surf.tiling = ISL_TILING_LINEAR;

   const char *name = templ->target == PIPE_BUFFER ? "buffer" : "miptree";

   res->bo = crocus_bo_alloc(screen->bufmgr, name, templ->width0);
   if (!res->bo) {
      crocus_resource_destroy(pscreen, &res->base.b);
      return NULL;
   }

   return &res->base.b;
}

static struct pipe_resource *
crocus_resource_create_with_modifiers(struct pipe_screen *pscreen,
                                      const struct pipe_resource *templ,
                                      const uint64_t *modifiers,
                                      int modifiers_count)
{
   struct crocus_screen *screen = (struct crocus_screen *)pscreen;
   struct intel_device_info *devinfo = &screen->devinfo;
   struct crocus_resource *res = crocus_alloc_resource(pscreen, templ);

   if (!res)
      return NULL;

   uint64_t modifier =
      select_best_modifier(devinfo, templ, modifiers, modifiers_count);

   if (modifier == DRM_FORMAT_MOD_INVALID && modifiers_count > 0) {
      fprintf(stderr, "Unsupported modifier, resource creation failed.\n");
      goto fail;
   }

   if (templ->usage == PIPE_USAGE_STAGING &&
       templ->bind == PIPE_BIND_DEPTH_STENCIL &&
       devinfo->ver < 6)
      goto fail;

   const bool isl_surf_created_successfully =
      crocus_resource_configure_main(screen, res, templ, modifier, 0);
   if (!isl_surf_created_successfully)
      goto fail;

   const char *name = "miptree";

   unsigned int flags = 0;
   if (templ->usage == PIPE_USAGE_STAGING)
      flags |= BO_ALLOC_COHERENT;

   /* Scanout buffers need to be WC. */
   if (templ->bind & PIPE_BIND_SCANOUT)
      flags |= BO_ALLOC_SCANOUT;

   uint64_t aux_size = 0;
   uint32_t aux_preferred_alloc_flags;

   if (!crocus_resource_configure_aux(screen, res, &aux_size,
                                      &aux_preferred_alloc_flags)) {
      goto fail;
   }

   /* Modifiers require the aux data to be in the same buffer as the main
    * surface, but we combine them even when a modifiers is not being used.
    */
   const uint64_t bo_size =
      MAX2(res->surf.size_B, res->aux.offset + aux_size);
   uint32_t alignment = MAX2(4096, res->surf.alignment_B);
   res->bo = crocus_bo_alloc_tiled(screen->bufmgr, name, bo_size, alignment,
                                   isl_tiling_to_i915_tiling(res->surf.tiling),
                                   res->surf.row_pitch_B, flags);

   if (!res->bo)
      goto fail;

   if (aux_size > 0) {
      res->aux.bo = res->bo;
      crocus_bo_reference(res->aux.bo);
      if (!crocus_resource_init_aux_buf(res, flags))
         goto fail;
   }

   if (templ->format == PIPE_FORMAT_S8_UINT && !(templ->usage == PIPE_USAGE_STAGING) &&
       devinfo->ver == 7 && (templ->bind & PIPE_BIND_SAMPLER_VIEW)) {
      struct pipe_resource templ_shadow = (struct pipe_resource) {
         .usage = 0,
         .bind = PIPE_BIND_SAMPLER_VIEW,
         .width0 = res->base.b.width0,
         .height0 = res->base.b.height0,
         .depth0 = res->base.b.depth0,
         .last_level = res->base.b.last_level,
         .nr_samples = res->base.b.nr_samples,
         .nr_storage_samples = res->base.b.nr_storage_samples,
         .array_size = res->base.b.array_size,
         .format = PIPE_FORMAT_R8_UINT,
         .target = res->base.b.target,
      };
      res->shadow = (struct crocus_resource *)screen->base.resource_create(&screen->base, &templ_shadow);
      assert(res->shadow);
   }

   return &res->base.b;

fail:
   crocus_resource_destroy(pscreen, &res->base.b);
   return NULL;

}

static struct pipe_resource *
crocus_resource_create(struct pipe_screen *pscreen,
                       const struct pipe_resource *templ)
{
   if (templ->target == PIPE_BUFFER)
      return crocus_resource_create_for_buffer(pscreen, templ);
   else
      return crocus_resource_create_with_modifiers(pscreen, templ, NULL, 0);
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
crocus_resource_from_user_memory(struct pipe_screen *pscreen,
                                 const struct pipe_resource *templ,
                                 void *user_memory)
{
   struct crocus_screen *screen = (struct crocus_screen *)pscreen;
   struct crocus_bufmgr *bufmgr = screen->bufmgr;
   struct crocus_resource *res = crocus_alloc_resource(pscreen, templ);
   if (!res)
      return NULL;

   assert(templ->target == PIPE_BUFFER);

   res->internal_format = templ->format;
   res->bo = crocus_bo_create_userptr(bufmgr, "user",
                                      user_memory, templ->width0);
   if (!res->bo) {
      free(res);
      return NULL;
   }

   util_range_add(&res->base.b, &res->valid_buffer_range, 0, templ->width0);

   return &res->base.b;
}

static struct pipe_resource *
crocus_resource_from_handle(struct pipe_screen *pscreen,
                            const struct pipe_resource *templ,
                            struct winsys_handle *whandle,
                            unsigned usage)
{
   assert(templ->target != PIPE_BUFFER);

   struct crocus_screen *screen = (struct crocus_screen *)pscreen;
   struct crocus_bufmgr *bufmgr = screen->bufmgr;
   struct crocus_resource *res = crocus_alloc_resource(pscreen, templ);

   if (!res)
      return NULL;

   switch (whandle->type) {
   case WINSYS_HANDLE_TYPE_FD:
      res->bo = crocus_bo_import_dmabuf(bufmgr, whandle->handle,
                                        whandle->modifier);
      break;
   case WINSYS_HANDLE_TYPE_SHARED:
      res->bo = crocus_bo_gem_create_from_name(bufmgr, "winsys image",
                                               whandle->handle);
      break;
   default:
      unreachable("invalid winsys handle type");
   }
   if (!res->bo)
      return NULL;

   res->offset = whandle->offset;
   res->external_format = whandle->format;

   assert(whandle->plane < util_format_get_num_planes(whandle->format));
   const uint64_t modifier =
      whandle->modifier != DRM_FORMAT_MOD_INVALID ?
      whandle->modifier : tiling_to_modifier(res->bo->tiling_mode);

   UNUSED const bool isl_surf_created_successfully =
      crocus_resource_configure_main(screen, res, templ, modifier,
                                     whandle->stride);
   assert(isl_surf_created_successfully);
   assert(res->bo->tiling_mode ==
          isl_tiling_to_i915_tiling(res->surf.tiling));

   // XXX: create_ccs_buf_for_image?
   if (whandle->modifier == DRM_FORMAT_MOD_INVALID) {
      if (!crocus_resource_alloc_separate_aux(screen, res))
         goto fail;
   } else {
      assert(!isl_drm_modifier_has_aux(whandle->modifier));
   }

   return &res->base.b;

fail:
   crocus_resource_destroy(pscreen, &res->base.b);
   return NULL;
}

static struct pipe_resource *
crocus_resource_from_memobj(struct pipe_screen *pscreen,
                            const struct pipe_resource *templ,
                            struct pipe_memory_object *pmemobj,
                            uint64_t offset)
{
   struct crocus_screen *screen = (struct crocus_screen *)pscreen;
   struct crocus_memory_object *memobj = (struct crocus_memory_object *)pmemobj;
   struct crocus_resource *res = crocus_alloc_resource(pscreen, templ);

   if (!res)
      return NULL;

   /* Disable Depth, and combined Depth+Stencil for now. */
   if (util_format_has_depth(util_format_description(templ->format)))
      return NULL;

   if (templ->flags & PIPE_RESOURCE_FLAG_TEXTURING_MORE_LIKELY) {
      UNUSED const bool isl_surf_created_successfully =
         crocus_resource_configure_main(screen, res, templ, DRM_FORMAT_MOD_INVALID, 0);
      assert(isl_surf_created_successfully);
   }

   res->bo = memobj->bo;
   res->offset = offset;
   res->external_format = memobj->format;

   crocus_bo_reference(memobj->bo);

   return &res->base.b;
}

static void
crocus_flush_resource(struct pipe_context *ctx, struct pipe_resource *resource)
{
   struct crocus_context *ice = (struct crocus_context *)ctx;
   struct crocus_resource *res = (void *) resource;

   /* Modifiers with compression are not supported. */
   assert(!res->mod_info ||
          !isl_drm_modifier_has_aux(res->mod_info->modifier));

   crocus_resource_prepare_access(ice, res,
                                  0, INTEL_REMAINING_LEVELS,
                                  0, INTEL_REMAINING_LAYERS,
                                  ISL_AUX_USAGE_NONE, false);
}

static void
crocus_resource_disable_aux_on_first_query(struct pipe_resource *resource,
                                           unsigned usage)
{
   struct crocus_resource *res = (struct crocus_resource *)resource;

   /* Modifiers with compression are not supported. */
   assert(!res->mod_info ||
          !isl_drm_modifier_has_aux(res->mod_info->modifier));

   /* Disable aux usage if explicit flush not set and this is the first time
    * we are dealing with this resource.
    */
   if ((!(usage & PIPE_HANDLE_USAGE_EXPLICIT_FLUSH) && res->aux.usage != 0) &&
       p_atomic_read(&resource->reference.count) == 1) {
      crocus_resource_disable_aux(res);
   }
}

static bool
crocus_resource_get_param(struct pipe_screen *pscreen,
                          struct pipe_context *context,
                          struct pipe_resource *resource,
                          unsigned plane,
                          unsigned layer,
                          unsigned level,
                          enum pipe_resource_param param,
                          unsigned handle_usage,
                          uint64_t *value)
{
   struct crocus_screen *screen = (struct crocus_screen *)pscreen;
   struct crocus_resource *res = (struct crocus_resource *)resource;

   /* Modifiers with compression are not supported. */
   assert(!res->mod_info ||
          !isl_drm_modifier_has_aux(res->mod_info->modifier));

   bool result;
   unsigned handle;

   struct crocus_bo *bo = res->bo;

   crocus_resource_disable_aux_on_first_query(resource, handle_usage);

   switch (param) {
   case PIPE_RESOURCE_PARAM_NPLANES: {
      unsigned count = 0;
      for (struct pipe_resource *cur = resource; cur; cur = cur->next)
         count++;
      *value = count;
      return true;
      }
   case PIPE_RESOURCE_PARAM_STRIDE:
      *value = res->surf.row_pitch_B;
      return true;
   case PIPE_RESOURCE_PARAM_OFFSET:
      *value = 0;
      return true;
   case PIPE_RESOURCE_PARAM_MODIFIER:
      *value = res->mod_info ? res->mod_info->modifier :
               tiling_to_modifier(isl_tiling_to_i915_tiling(res->surf.tiling));
      return true;
   case PIPE_RESOURCE_PARAM_HANDLE_TYPE_SHARED:
      result = crocus_bo_flink(bo, &handle) == 0;
      if (result)
         *value = handle;
      return result;
   case PIPE_RESOURCE_PARAM_HANDLE_TYPE_KMS: {
      /* Because we share the same drm file across multiple crocus_screen, when
       * we export a GEM handle we must make sure it is valid in the DRM file
       * descriptor the caller is using (this is the FD given at screen
       * creation).
       */
      uint32_t handle;
      if (crocus_bo_export_gem_handle_for_device(bo, screen->winsys_fd, &handle))
         return false;
      *value = handle;
      return true;
   }
   case PIPE_RESOURCE_PARAM_HANDLE_TYPE_FD:
      result = crocus_bo_export_dmabuf(bo, (int *) &handle) == 0;
      if (result)
         *value = handle;
      return result;
   default:
      return false;
   }
}

static bool
crocus_resource_get_handle(struct pipe_screen *pscreen,
                           struct pipe_context *ctx,
                           struct pipe_resource *resource,
                           struct winsys_handle *whandle,
                           unsigned usage)
{
   struct crocus_screen *screen = (struct crocus_screen *) pscreen;
   struct crocus_resource *res = (struct crocus_resource *)resource;

   /* Modifiers with compression are not supported. */
   assert(!res->mod_info ||
          !isl_drm_modifier_has_aux(res->mod_info->modifier));

   crocus_resource_disable_aux_on_first_query(resource, usage);

   struct crocus_bo *bo;
   /* If this is a buffer, stride should be 0 - no need to special case */
   whandle->stride = res->surf.row_pitch_B;
   bo = res->bo;
   whandle->format = res->external_format;
   whandle->modifier =
      res->mod_info ? res->mod_info->modifier
                    : tiling_to_modifier(res->bo->tiling_mode);

#ifndef NDEBUG
   enum isl_aux_usage allowed_usage = ISL_AUX_USAGE_NONE;

   if (res->aux.usage != allowed_usage) {
      enum isl_aux_state aux_state = crocus_resource_get_aux_state(res, 0, 0);
      assert(aux_state == ISL_AUX_STATE_RESOLVED ||
             aux_state == ISL_AUX_STATE_PASS_THROUGH);
   }
#endif

   switch (whandle->type) {
   case WINSYS_HANDLE_TYPE_SHARED:
      return crocus_bo_flink(bo, &whandle->handle) == 0;
   case WINSYS_HANDLE_TYPE_KMS: {
      /* Because we share the same drm file across multiple crocus_screen, when
       * we export a GEM handle we must make sure it is valid in the DRM file
       * descriptor the caller is using (this is the FD given at screen
       * creation).
       */
      uint32_t handle;
      if (crocus_bo_export_gem_handle_for_device(bo, screen->winsys_fd, &handle))
         return false;
      whandle->handle = handle;
      return true;
   }
   case WINSYS_HANDLE_TYPE_FD:
      return crocus_bo_export_dmabuf(bo, (int *) &whandle->handle) == 0;
   }

   return false;
}

static bool
resource_is_busy(struct crocus_context *ice,
                 struct crocus_resource *res)
{
   bool busy = crocus_bo_busy(res->bo);

   for (int i = 0; i < ice->batch_count; i++)
      busy |= crocus_batch_references(&ice->batches[i], res->bo);

   return busy;
}

void
crocus_replace_buffer_storage(struct pipe_context *ctx,
                              struct pipe_resource *p_dst,
                              struct pipe_resource *p_src,
                              unsigned num_rebinds,
                              uint32_t rebind_mask,
                              uint32_t delete_buffer_id)
{
   struct crocus_screen *screen = (void *) ctx->screen;
   struct crocus_context *ice = (void *) ctx;
   struct crocus_resource *dst = (void *) p_dst;
   struct crocus_resource *src = (void *) p_src;

   assert(memcmp(&dst->surf, &src->surf, sizeof(dst->surf)) == 0);

   struct crocus_bo *old_bo = dst->bo;

   /* Swap out the backing storage */
   crocus_bo_reference(src->bo);
   dst->bo = src->bo;

   /* Rebind the buffer, replacing any state referring to the old BO's
    * address, and marking state dirty so it's reemitted.
    */
   screen->vtbl.rebind_buffer(ice, dst);

   crocus_bo_unreference(old_bo);
}

static void
crocus_invalidate_resource(struct pipe_context *ctx,
                           struct pipe_resource *resource)
{
   struct crocus_screen *screen = (void *) ctx->screen;
   struct crocus_context *ice = (void *) ctx;
   struct crocus_resource *res = (void *) resource;

   if (resource->target != PIPE_BUFFER)
      return;

   /* If it's already invalidated, don't bother doing anything. */
   if (res->valid_buffer_range.start > res->valid_buffer_range.end)
      return;

   if (!resource_is_busy(ice, res)) {
      /* The resource is idle, so just mark that it contains no data and
       * keep using the same underlying buffer object.
       */
      util_range_set_empty(&res->valid_buffer_range);
      return;
   }

   /* Otherwise, try and replace the backing storage with a new BO. */

   /* We can't reallocate memory we didn't allocate in the first place. */
   if (res->bo->userptr)
      return;

   struct crocus_bo *old_bo = res->bo;
   struct crocus_bo *new_bo =
      crocus_bo_alloc(screen->bufmgr, res->bo->name, resource->width0);

   if (!new_bo)
      return;

   /* Swap out the backing storage */
   res->bo = new_bo;

   /* Rebind the buffer, replacing any state referring to the old BO's
    * address, and marking state dirty so it's reemitted.
    */
   screen->vtbl.rebind_buffer(ice, res);

   util_range_set_empty(&res->valid_buffer_range);

   crocus_bo_unreference(old_bo);
}

static void
crocus_flush_staging_region(struct pipe_transfer *xfer,
                            const struct pipe_box *flush_box)
{
   if (!(xfer->usage & PIPE_MAP_WRITE))
      return;

   struct crocus_transfer *map = (void *) xfer;

   struct pipe_box src_box = *flush_box;

   /* Account for extra alignment padding in staging buffer */
   if (xfer->resource->target == PIPE_BUFFER)
      src_box.x += xfer->box.x % CROCUS_MAP_BUFFER_ALIGNMENT;

   struct pipe_box dst_box = (struct pipe_box) {
      .x = xfer->box.x + flush_box->x,
      .y = xfer->box.y + flush_box->y,
      .z = xfer->box.z + flush_box->z,
      .width = flush_box->width,
      .height = flush_box->height,
      .depth = flush_box->depth,
   };

   crocus_copy_region(map->blorp, map->batch, xfer->resource, xfer->level,
                      dst_box.x, dst_box.y, dst_box.z, map->staging, 0,
                      &src_box);
}

static void
crocus_unmap_copy_region(struct crocus_transfer *map)
{
   crocus_resource_destroy(map->staging->screen, map->staging);

   map->ptr = NULL;
}

static void
crocus_map_copy_region(struct crocus_transfer *map)
{
   struct pipe_screen *pscreen = &map->batch->screen->base;
   struct pipe_transfer *xfer = &map->base.b;
   struct pipe_box *box = &xfer->box;
   struct crocus_resource *res = (void *) xfer->resource;

   unsigned extra = xfer->resource->target == PIPE_BUFFER ?
                    box->x % CROCUS_MAP_BUFFER_ALIGNMENT : 0;

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

   if (xfer->resource->target == PIPE_BUFFER)
      templ.target = PIPE_BUFFER;
   else if (templ.array_size > 1)
      templ.target = PIPE_TEXTURE_2D_ARRAY;
   else
      templ.target = PIPE_TEXTURE_2D;

   map->staging = crocus_resource_create(pscreen, &templ);

   /* If we fail to create a staging resource, the caller will fallback
    * to mapping directly on the CPU.
    */
   if (!map->staging)
      return;

   if (templ.target != PIPE_BUFFER) {
      struct isl_surf *surf = &((struct crocus_resource *) map->staging)->surf;
      xfer->stride = isl_surf_get_row_pitch_B(surf);
      xfer->layer_stride = isl_surf_get_array_pitch(surf);
   }

   if (!(xfer->usage & PIPE_MAP_DISCARD_RANGE)) {
      crocus_copy_region(map->blorp, map->batch, map->staging, 0, extra, 0, 0,
                         xfer->resource, xfer->level, box);
      /* Ensure writes to the staging BO land before we map it below. */
      crocus_emit_pipe_control_flush(map->batch,
                                     "transfer read: flush before mapping",
                                     PIPE_CONTROL_RENDER_TARGET_FLUSH |
                                     PIPE_CONTROL_CS_STALL);
   }

   struct crocus_bo *staging_bo = crocus_resource_bo(map->staging);

   if (crocus_batch_references(map->batch, staging_bo))
      crocus_batch_flush(map->batch);

   map->ptr =
      crocus_bo_map(map->dbg, staging_bo, xfer->usage & MAP_FLAGS) + extra;

   map->unmap = crocus_unmap_copy_region;
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

void
crocus_resource_get_image_offset(struct crocus_resource *res,
                                 uint32_t level, uint32_t z,
                                 uint32_t *x, uint32_t *y)
{
   get_image_offset_el(&res->surf, level, z, x, y);
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
s8_offset(uint32_t stride, uint32_t x, uint32_t y, bool swizzled)
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

   if (swizzled) {
      /* adjust for bit6 swizzling */
      if (((byte_x / 8) % 2) == 1) {
         if (((byte_y / 8) % 2) == 0) {
            u += 64;
         } else {
            u -= 64;
         }
      }
   }

   return u;
}

static void
crocus_unmap_s8(struct crocus_transfer *map)
{
   struct pipe_transfer *xfer = &map->base.b;
   const struct pipe_box *box = &xfer->box;
   struct crocus_resource *res = (struct crocus_resource *) xfer->resource;
   struct isl_surf *surf = &res->surf;

   if (xfer->usage & PIPE_MAP_WRITE) {
      uint8_t *untiled_s8_map = map->ptr;
      uint8_t *tiled_s8_map =
         crocus_bo_map(map->dbg, res->bo, (xfer->usage | MAP_RAW) & MAP_FLAGS);

      for (int s = 0; s < box->depth; s++) {
         unsigned x0_el, y0_el;
         get_image_offset_el(surf, xfer->level, box->z + s, &x0_el, &y0_el);

         for (uint32_t y = 0; y < box->height; y++) {
            for (uint32_t x = 0; x < box->width; x++) {
               ptrdiff_t offset = s8_offset(surf->row_pitch_B,
                                            x0_el + box->x + x,
                                            y0_el + box->y + y,
                                            map->has_swizzling);
               tiled_s8_map[offset] =
                  untiled_s8_map[s * xfer->layer_stride + y * xfer->stride + x];
            }
         }
      }
   }

   free(map->buffer);
}

static void
crocus_map_s8(struct crocus_transfer *map)
{
   struct pipe_transfer *xfer = &map->base.b;
   const struct pipe_box *box = &xfer->box;
   struct crocus_resource *res = (struct crocus_resource *) xfer->resource;
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
   if (!(xfer->usage & PIPE_MAP_DISCARD_RANGE)) {
      uint8_t *untiled_s8_map = map->ptr;
      uint8_t *tiled_s8_map =
         crocus_bo_map(map->dbg, res->bo, (xfer->usage | MAP_RAW) & MAP_FLAGS);

      for (int s = 0; s < box->depth; s++) {
         unsigned x0_el, y0_el;
         get_image_offset_el(surf, xfer->level, box->z + s, &x0_el, &y0_el);

         for (uint32_t y = 0; y < box->height; y++) {
            for (uint32_t x = 0; x < box->width; x++) {
               ptrdiff_t offset = s8_offset(surf->row_pitch_B,
                                            x0_el + box->x + x,
                                            y0_el + box->y + y,
                                            map->has_swizzling);
               untiled_s8_map[s * xfer->layer_stride + y * xfer->stride + x] =
                  tiled_s8_map[offset];
            }
         }
      }
   }

   map->unmap = crocus_unmap_s8;
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
crocus_unmap_tiled_memcpy(struct crocus_transfer *map)
{
   struct pipe_transfer *xfer = &map->base.b;
   const struct pipe_box *box = &xfer->box;
   struct crocus_resource *res = (struct crocus_resource *) xfer->resource;
   struct isl_surf *surf = &res->surf;

   if (xfer->usage & PIPE_MAP_WRITE) {
      char *dst =
         crocus_bo_map(map->dbg, res->bo, (xfer->usage | MAP_RAW) & MAP_FLAGS);

      for (int s = 0; s < box->depth; s++) {
         unsigned x1, x2, y1, y2;
         tile_extents(surf, box, xfer->level, s, &x1, &x2, &y1, &y2);

         void *ptr = map->ptr + s * xfer->layer_stride;

         isl_memcpy_linear_to_tiled(x1, x2, y1, y2, dst, ptr,
                                    surf->row_pitch_B, xfer->stride,
                                    map->has_swizzling,
                                    surf->tiling, ISL_MEMCPY);
      }
   }
   os_free_aligned(map->buffer);
   map->buffer = map->ptr = NULL;
}

static void
crocus_map_tiled_memcpy(struct crocus_transfer *map)
{
   struct pipe_transfer *xfer = &map->base.b;
   const struct pipe_box *box = &xfer->box;
   struct crocus_resource *res = (struct crocus_resource *) xfer->resource;
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

   if (!(xfer->usage & PIPE_MAP_DISCARD_RANGE)) {
      char *src =
         crocus_bo_map(map->dbg, res->bo, (xfer->usage | MAP_RAW) & MAP_FLAGS);

      for (int s = 0; s < box->depth; s++) {
         unsigned x1, x2, y1, y2;
         tile_extents(surf, box, xfer->level, s, &x1, &x2, &y1, &y2);

         /* Use 's' rather than 'box->z' to rebase the first slice to 0. */
         void *ptr = map->ptr + s * xfer->layer_stride;

         isl_memcpy_tiled_to_linear(x1, x2, y1, y2, ptr, src, xfer->stride,
                                    surf->row_pitch_B,
                                    map->has_swizzling,
                                    surf->tiling,
#if defined(USE_SSE41)
                                    util_get_cpu_caps()->has_sse4_1 ? ISL_MEMCPY_STREAMING_LOAD :
#endif
                                    ISL_MEMCPY);
      }
   }

   map->unmap = crocus_unmap_tiled_memcpy;
}

static void
crocus_map_direct(struct crocus_transfer *map)
{
   struct pipe_transfer *xfer = &map->base.b;
   struct pipe_box *box = &xfer->box;
   struct crocus_resource *res = (struct crocus_resource *) xfer->resource;

   void *ptr = crocus_bo_map(map->dbg, res->bo, xfer->usage & MAP_FLAGS);

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
can_promote_to_async(const struct crocus_resource *res,
                     const struct pipe_box *box,
                     unsigned usage)
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

static void *
crocus_transfer_map(struct pipe_context *ctx,
                    struct pipe_resource *resource,
                    unsigned level,
                    unsigned usage,
                    const struct pipe_box *box,
                    struct pipe_transfer **ptransfer)
{
   struct crocus_context *ice = (struct crocus_context *)ctx;
   struct crocus_resource *res = (struct crocus_resource *)resource;
   struct isl_surf *surf = &res->surf;
   struct crocus_screen *screen = (struct crocus_screen *)ctx->screen;

   if (usage & PIPE_MAP_DISCARD_WHOLE_RESOURCE) {
      /* Replace the backing storage with a fresh buffer for non-async maps */
      if (!(usage & (PIPE_MAP_UNSYNCHRONIZED |
                     TC_TRANSFER_MAP_NO_INVALIDATE)))
         crocus_invalidate_resource(ctx, resource);

      /* If we can discard the whole resource, we can discard the range. */
      usage |= PIPE_MAP_DISCARD_RANGE;
   }

   if (!(usage & PIPE_MAP_UNSYNCHRONIZED) &&
       can_promote_to_async(res, box, usage)) {
      usage |= PIPE_MAP_UNSYNCHRONIZED;
   }

   bool map_would_stall = false;

   if (!(usage & PIPE_MAP_UNSYNCHRONIZED)) {
      map_would_stall = resource_is_busy(ice, res) ||
         crocus_has_invalid_primary(res, level, 1, box->z, box->depth);


      if (map_would_stall && (usage & PIPE_MAP_DONTBLOCK) &&
                             (usage & PIPE_MAP_DIRECTLY))
         return NULL;
   }

   if (surf->tiling != ISL_TILING_LINEAR &&
       (usage & PIPE_MAP_DIRECTLY))
      return NULL;

   struct crocus_transfer *map;
   if (usage & TC_TRANSFER_MAP_THREADED_UNSYNC)
      map = slab_zalloc(&ice->transfer_pool_unsync);
   else
      map = slab_zalloc(&ice->transfer_pool);

   struct pipe_transfer *xfer = &map->base.b;

   if (!map)
      return NULL;

   map->dbg = &ice->dbg;

   map->has_swizzling = screen->devinfo.has_bit6_swizzle;
   pipe_resource_reference(&xfer->resource, resource);
   xfer->level = level;
   xfer->usage = usage;
   xfer->box = *box;
   *ptransfer = xfer;

   map->dest_had_defined_contents =
      util_ranges_intersect(&res->valid_buffer_range, box->x,
                            box->x + box->width);

   if (usage & PIPE_MAP_WRITE)
      util_range_add(&res->base.b, &res->valid_buffer_range, box->x, box->x + box->width);

   /* Avoid using GPU copies for persistent/coherent buffers, as the idea
    * there is to access them simultaneously on the CPU & GPU.  This also
    * avoids trying to use GPU copies for our u_upload_mgr buffers which
    * contain state we're constructing for a GPU draw call, which would
    * kill us with infinite stack recursion.
    */
   bool no_gpu = usage & (PIPE_MAP_PERSISTENT |
                          PIPE_MAP_COHERENT |
                          PIPE_MAP_DIRECTLY);

   /* GPU copies are not useful for buffer reads.  Instead of stalling to
    * read from the original buffer, we'd simply copy it to a temporary...
    * then stall (a bit longer) to read from that buffer.
    *
    * Images are less clear-cut.  Color resolves are destructive, removing
    * the underlying compression, so we'd rather blit the data to a linear
    * temporary and map that, to avoid the resolve.  (It might be better to
    * a tiled temporary and use the tiled_memcpy paths...)
    */
   if (!(usage & PIPE_MAP_DISCARD_RANGE) &&
       !crocus_has_invalid_primary(res, level, 1, box->z, box->depth))
      no_gpu = true;

   const struct isl_format_layout *fmtl = isl_format_get_layout(surf->format);
   if (fmtl->txc == ISL_TXC_ASTC)
      no_gpu = true;

   if (map_would_stall && !no_gpu) {
      /* If we need a synchronous mapping and the resource is busy, or needs
       * resolving, we copy to/from a linear temporary buffer using the GPU.
       */
      map->batch = &ice->batches[CROCUS_BATCH_RENDER];
      map->blorp = &ice->blorp;
      crocus_map_copy_region(map);
   }

   /* If we've requested a direct mapping, or crocus_map_copy_region failed
    * to create a staging resource, then map it directly on the CPU.
    */
   if (!map->ptr) {
      if (resource->target != PIPE_BUFFER) {
         crocus_resource_access_raw(ice, res,
                                    level, box->z, box->depth,
                                    usage & PIPE_MAP_WRITE);
      }

      if (!(usage & PIPE_MAP_UNSYNCHRONIZED)) {
         for (int i = 0; i < ice->batch_count; i++) {
            if (crocus_batch_references(&ice->batches[i], res->bo))
               crocus_batch_flush(&ice->batches[i]);
         }
      }

      if (surf->tiling == ISL_TILING_W) {
         /* TODO: Teach crocus_map_tiled_memcpy about W-tiling... */
         crocus_map_s8(map);
      } else if (surf->tiling != ISL_TILING_LINEAR && screen->devinfo.ver > 4) {
         crocus_map_tiled_memcpy(map);
      } else {
         crocus_map_direct(map);
      }
   }

   return map->ptr;
}

static void
crocus_transfer_flush_region(struct pipe_context *ctx,
                             struct pipe_transfer *xfer,
                             const struct pipe_box *box)
{
   struct crocus_context *ice = (struct crocus_context *)ctx;
   struct crocus_resource *res = (struct crocus_resource *) xfer->resource;
   struct crocus_transfer *map = (void *) xfer;

   if (map->staging)
      crocus_flush_staging_region(xfer, box);

   uint32_t history_flush = 0;

   if (res->base.b.target == PIPE_BUFFER) {
      if (map->staging)
         history_flush |= PIPE_CONTROL_RENDER_TARGET_FLUSH;

      if (map->dest_had_defined_contents)
         history_flush |= crocus_flush_bits_for_history(res);

      util_range_add(&res->base.b, &res->valid_buffer_range, box->x, box->x + box->width);
   }

   if (history_flush & ~PIPE_CONTROL_CS_STALL) {
      for (int i = 0; i < ice->batch_count; i++) {
         struct crocus_batch *batch = &ice->batches[i];

         if (!batch->command.bo)
            continue;
         if (batch->contains_draw || batch->cache.render->entries) {
            crocus_batch_maybe_flush(batch, 24);
            crocus_emit_pipe_control_flush(batch,
                                           "cache history: transfer flush",
                                           history_flush);
         }
      }
   }

   /* Make sure we flag constants dirty even if there's no need to emit
    * any PIPE_CONTROLs to a batch.
    */
   crocus_dirty_for_history(ice, res);
}

static void
crocus_transfer_unmap(struct pipe_context *ctx, struct pipe_transfer *xfer)
{
   struct crocus_context *ice = (struct crocus_context *)ctx;
   struct crocus_transfer *map = (void *) xfer;

   if (!(xfer->usage & (PIPE_MAP_FLUSH_EXPLICIT |
                        PIPE_MAP_COHERENT))) {
      struct pipe_box flush_box = {
         .x = 0, .y = 0, .z = 0,
         .width  = xfer->box.width,
         .height = xfer->box.height,
         .depth  = xfer->box.depth,
      };
      crocus_transfer_flush_region(ctx, xfer, &flush_box);
   }

   if (map->unmap)
      map->unmap(map);

   pipe_resource_reference(&xfer->resource, NULL);
   /* transfer_unmap is always called from the driver thread, so we have to
    * use transfer_pool, not transfer_pool_unsync.  Freeing an object into a
    * different pool is allowed, however.
    */
   slab_free(&ice->transfer_pool, map);
}

/**
 * Mark state dirty that needs to be re-emitted when a resource is written.
 */
void
crocus_dirty_for_history(struct crocus_context *ice,
                         struct crocus_resource *res)
{
   uint64_t stage_dirty = 0ull;

   if (res->bind_history & PIPE_BIND_CONSTANT_BUFFER) {
      stage_dirty |= ((uint64_t)res->bind_stages) << CROCUS_SHIFT_FOR_STAGE_DIRTY_CONSTANTS;
   }

   ice->state.stage_dirty |= stage_dirty;
}

/**
 * Produce a set of PIPE_CONTROL bits which ensure data written to a
 * resource becomes visible, and any stale read cache data is invalidated.
 */
uint32_t
crocus_flush_bits_for_history(struct crocus_resource *res)
{
   uint32_t flush = PIPE_CONTROL_CS_STALL;

   if (res->bind_history & PIPE_BIND_CONSTANT_BUFFER) {
      flush |= PIPE_CONTROL_CONST_CACHE_INVALIDATE |
               PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE;
   }

   if (res->bind_history & PIPE_BIND_SAMPLER_VIEW)
      flush |= PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE;

   if (res->bind_history & (PIPE_BIND_VERTEX_BUFFER | PIPE_BIND_INDEX_BUFFER))
      flush |= PIPE_CONTROL_VF_CACHE_INVALIDATE;

   if (res->bind_history & (PIPE_BIND_SHADER_BUFFER | PIPE_BIND_SHADER_IMAGE))
      flush |= PIPE_CONTROL_DATA_CACHE_FLUSH;

   return flush;
}

void
crocus_flush_and_dirty_for_history(struct crocus_context *ice,
                                   struct crocus_batch *batch,
                                   struct crocus_resource *res,
                                   uint32_t extra_flags,
                                   const char *reason)
{
   if (res->base.b.target != PIPE_BUFFER)
      return;

   uint32_t flush = crocus_flush_bits_for_history(res) | extra_flags;

   crocus_emit_pipe_control_flush(batch, reason, flush);

   crocus_dirty_for_history(ice, res);
}

bool
crocus_resource_set_clear_color(struct crocus_context *ice,
                                struct crocus_resource *res,
                                union isl_color_value color)
{
   if (memcmp(&res->aux.clear_color, &color, sizeof(color)) != 0) {
      res->aux.clear_color = color;
      return true;
   }

   return false;
}

union isl_color_value
crocus_resource_get_clear_color(const struct crocus_resource *res)
{
   assert(res->aux.bo);

   return res->aux.clear_color;
}

static enum pipe_format
crocus_resource_get_internal_format(struct pipe_resource *p_res)
{
   struct crocus_resource *res = (void *) p_res;
   return res->internal_format;
}

static const struct u_transfer_vtbl transfer_vtbl = {
   .resource_create       = crocus_resource_create,
   .resource_destroy      = crocus_resource_destroy,
   .transfer_map          = crocus_transfer_map,
   .transfer_unmap        = crocus_transfer_unmap,
   .transfer_flush_region = crocus_transfer_flush_region,
   .get_internal_format   = crocus_resource_get_internal_format,
   .set_stencil           = crocus_resource_set_separate_stencil,
   .get_stencil           = crocus_resource_get_separate_stencil,
};

static bool
crocus_is_dmabuf_modifier_supported(struct pipe_screen *pscreen,
                                    uint64_t modifier, enum pipe_format pfmt,
                                    bool *external_only)
{
   struct crocus_screen *screen = (void *) pscreen;
   const struct intel_device_info *devinfo = &screen->devinfo;

   if (modifier_is_supported(devinfo, pfmt, 0, modifier)) {
      if (external_only)
         *external_only = false;

      return true;
   }

   return false;
}

static unsigned int
crocus_get_dmabuf_modifier_planes(struct pipe_screen *pscreen, uint64_t modifier,
                                  enum pipe_format format)
{
   return util_format_get_num_planes(format);
}

static struct pipe_memory_object *
crocus_memobj_create_from_handle(struct pipe_screen *pscreen,
                                 struct winsys_handle *whandle,
                                 bool dedicated)
{
   struct crocus_screen *screen = (struct crocus_screen *)pscreen;
   struct crocus_memory_object *memobj = CALLOC_STRUCT(crocus_memory_object);
   struct crocus_bo *bo;
   const struct isl_drm_modifier_info *mod_inf;

   if (!memobj)
      return NULL;

   switch (whandle->type) {
   case WINSYS_HANDLE_TYPE_SHARED:
      bo = crocus_bo_gem_create_from_name(screen->bufmgr, "winsys image",
                                        whandle->handle);
      break;
   case WINSYS_HANDLE_TYPE_FD:
      mod_inf = isl_drm_modifier_get_info(whandle->modifier);
      if (mod_inf) {
         bo = crocus_bo_import_dmabuf(screen->bufmgr, whandle->handle,
                                    whandle->modifier);
      } else {
         /* If we can't get information about the tiling from the
          * kernel we ignore it. We are going to set it when we
          * create the resource.
          */
         bo = crocus_bo_import_dmabuf_no_mods(screen->bufmgr,
                                            whandle->handle);
      }

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
crocus_memobj_destroy(struct pipe_screen *pscreen,
                      struct pipe_memory_object *pmemobj)
{
   struct crocus_memory_object *memobj = (struct crocus_memory_object *)pmemobj;

   crocus_bo_unreference(memobj->bo);
   free(memobj);
}

void
crocus_init_screen_resource_functions(struct pipe_screen *pscreen)
{
   struct crocus_screen *screen = (void *) pscreen;
   pscreen->query_dmabuf_modifiers = crocus_query_dmabuf_modifiers;
   pscreen->is_dmabuf_modifier_supported = crocus_is_dmabuf_modifier_supported;
   pscreen->get_dmabuf_modifier_planes = crocus_get_dmabuf_modifier_planes;
   pscreen->resource_create_with_modifiers =
      crocus_resource_create_with_modifiers;
   pscreen->resource_create = u_transfer_helper_resource_create;
   pscreen->resource_from_user_memory = crocus_resource_from_user_memory;
   pscreen->resource_from_handle = crocus_resource_from_handle;
   pscreen->resource_from_memobj = crocus_resource_from_memobj;
   pscreen->resource_get_handle = crocus_resource_get_handle;
   pscreen->resource_get_param = crocus_resource_get_param;
   pscreen->resource_destroy = u_transfer_helper_resource_destroy;
   pscreen->memobj_create_from_handle = crocus_memobj_create_from_handle;
   pscreen->memobj_destroy = crocus_memobj_destroy;

   enum u_transfer_helper_flags transfer_flags = U_TRANSFER_HELPER_MSAA_MAP;
   if (screen->devinfo.ver >= 6) {
      transfer_flags |= U_TRANSFER_HELPER_SEPARATE_Z32S8 |
               U_TRANSFER_HELPER_SEPARATE_STENCIL;
   }

   pscreen->transfer_helper =
      u_transfer_helper_create(&transfer_vtbl, transfer_flags);
}

void
crocus_init_resource_functions(struct pipe_context *ctx)
{
   ctx->flush_resource = crocus_flush_resource;
   ctx->clear_buffer = u_default_clear_buffer;
   ctx->invalidate_resource = crocus_invalidate_resource;
   ctx->buffer_map = u_transfer_helper_transfer_map;
   ctx->texture_map = u_transfer_helper_transfer_map;
   ctx->transfer_flush_region = u_transfer_helper_transfer_flush_region;
   ctx->buffer_unmap = u_transfer_helper_transfer_unmap;
   ctx->texture_unmap = u_transfer_helper_transfer_unmap;
   ctx->buffer_subdata = u_default_buffer_subdata;
   ctx->texture_subdata = u_default_texture_subdata;
}
