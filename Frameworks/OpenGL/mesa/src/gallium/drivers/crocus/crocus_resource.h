/*
 * Copyright 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef CROCUS_RESOURCE_H
#define CROCUS_RESOURCE_H

#include "pipe/p_state.h"
#include "util/u_inlines.h"
#include "util/u_range.h"
#include "util/u_threaded_context.h"
#include "intel/isl/isl.h"
#include "intel/dev/intel_device_info.h"
#include "crocus_bufmgr.h"

struct crocus_batch;
struct crocus_context;

#define CROCUS_MAX_MIPLEVELS 15

struct crocus_format_info {
   enum isl_format fmt;
   enum pipe_swizzle swizzles[4];
};

static inline enum isl_channel_select
pipe_to_isl_swizzle(const enum pipe_swizzle pswz, bool green_to_blue)
{
   unsigned swz = (pswz + 4) & 7;

   return (green_to_blue && swz == ISL_CHANNEL_SELECT_GREEN) ? ISL_CHANNEL_SELECT_BLUE : swz;
}

static inline struct isl_swizzle
pipe_to_isl_swizzles(const enum pipe_swizzle pswz[4])
{
   struct isl_swizzle swz;
   swz.r = pipe_to_isl_swizzle(pswz[0], false);
   swz.g = pipe_to_isl_swizzle(pswz[1], false);
   swz.b = pipe_to_isl_swizzle(pswz[2], false);
   swz.a = pipe_to_isl_swizzle(pswz[3], false);
   return swz;
}

static inline void
crocus_combine_swizzle(enum pipe_swizzle outswz[4],
                       const enum pipe_swizzle fswz[4],
                       const enum pipe_swizzle vswz[4])
{
   for (unsigned i = 0; i < 4; i++) {
      switch (vswz[i]) {
      case PIPE_SWIZZLE_X: outswz[i] = fswz[0]; break;
      case PIPE_SWIZZLE_Y: outswz[i] = fswz[1]; break;
      case PIPE_SWIZZLE_Z: outswz[i] = fswz[2]; break;
      case PIPE_SWIZZLE_W: outswz[i] = fswz[3]; break;
      case PIPE_SWIZZLE_1: outswz[i] = PIPE_SWIZZLE_1; break;
      case PIPE_SWIZZLE_0: outswz[i] = PIPE_SWIZZLE_0; break;
      default: unreachable("invalid swizzle");
      }
   }
}

/**
 * Resources represent a GPU buffer object or image (mipmap tree).
 *
 * They contain the storage (BO) and layout information (ISL surface).
 */
struct crocus_resource {
   struct threaded_resource base;
   enum pipe_format internal_format;

   /**
    * The ISL surface layout information for this resource.
    *
    * This is not filled out for PIPE_BUFFER resources, but is guaranteed
    * to be zeroed.  Note that this also guarantees that res->surf.tiling
    * will be ISL_TILING_LINEAR, so it's safe to check that.
    */
   struct isl_surf surf;

   /** Backing storage for the resource */
   struct crocus_bo *bo;

   /** offset at which data starts in the BO */
   uint64_t offset;

   /**
    * A bitfield of PIPE_BIND_* indicating how this resource was bound
    * in the past.  Only meaningful for PIPE_BUFFER; used for flushing.
    */
   unsigned bind_history;

   /**
    * A bitfield of MESA_SHADER_* stages indicating where this resource
    * was bound.
    */
   unsigned bind_stages;

   /**
    * For PIPE_BUFFER resources, a range which may contain valid data.
    *
    * This is a conservative estimate of what part of the buffer contains
    * valid data that we have to preserve.  The rest of the buffer is
    * considered invalid, and we can promote writes to that region to
    * be unsynchronized writes, avoiding blit copies.
    */
   struct util_range valid_buffer_range;

   /**
    * Auxiliary buffer information (CCS, MCS, or HiZ).
    */
   struct {
      /** The surface layout for the auxiliary buffer. */
      struct isl_surf surf;

      /** The buffer object containing the auxiliary data. */
      struct crocus_bo *bo;

      /** Offset into 'bo' where the auxiliary surface starts. */
      uint32_t offset;

      /**
       * Fast clear color for this surface.  For depth surfaces, the clear
       * value is stored as a float32 in the red component.
       */
      union isl_color_value clear_color;

      /**
       * \brief The type of auxiliary compression used by this resource.
       *
       * This describes the type of auxiliary compression that is intended to
       * be used by this resource.  An aux usage of ISL_AUX_USAGE_NONE means
       * that auxiliary compression is permanently disabled.  An aux usage
       * other than ISL_AUX_USAGE_NONE does not imply that auxiliary
       * compression will always be enabled for this surface.
       */
      enum isl_aux_usage usage;

      /**
       * \brief Maps miptree slices to their current aux state.
       *
       * This two-dimensional array is indexed as [level][layer] and stores an
       * aux state for each slice.
       */
      enum isl_aux_state **state;

      /**
       * If (1 << level) is set, HiZ is enabled for that miplevel.
       */
      uint16_t has_hiz;
   } aux;

   /**
    * \brief Shadow miptree for sampling when the main isn't supported by HW.
    *
    * To workaround various sampler bugs and limitations, we blit the main
    * texture into a new texture that can be sampled.
    *
    * This miptree may be used for:
    * - Stencil texturing (pre-BDW) as required by GL_ARB_stencil_texturing.
    */
   struct crocus_resource *shadow;
   bool shadow_needs_update;

   /**
    * For external surfaces, this is format that was used to create or import
    * the surface. For internal surfaces, this will always be
    * PIPE_FORMAT_NONE.
    */
   enum pipe_format external_format;

   /**
    * For external surfaces, this is DRM format modifier that was used to
    * create or import the surface.  For internal surfaces, this will always
    * be DRM_FORMAT_MOD_INVALID.
    */
   const struct isl_drm_modifier_info *mod_info;

   /**
    * The screen the resource was originally created with, stored for refcounting.
    */
   struct pipe_screen *orig_screen;
};

/**
 * A simple <resource, offset> tuple for storing a reference to a
 * piece of state stored in a GPU buffer object.
 */
struct crocus_state_ref {
   struct pipe_resource *res;
   uint32_t offset;
};

/**
 * Gallium CSO for sampler views (texture views).
 *
 * In addition to the normal pipe_resource, this adds an ISL view
 * which may reinterpret the format or restrict levels/layers.
 *
 * These can also be linear texture buffers.
 */
struct crocus_sampler_view {
   struct pipe_sampler_view base;
   struct isl_view view;
   struct isl_view gather_view;

   enum pipe_swizzle swizzle[4];
   union isl_color_value clear_color;

   /* A short-cut (not a reference) to the actual resource being viewed.
    * Multi-planar (or depth+stencil) images may have multiple resources
    * chained together; this skips having to traverse base->texture->*.
    */
   struct crocus_resource *res;
};

/**
 * Image view representation.
 */
struct crocus_image_view {
   struct pipe_image_view base;
   struct isl_view view;
};

/**
 * Gallium CSO for surfaces (framebuffer attachments).
 *
 * A view of a surface that can be bound to a color render target or
 * depth/stencil attachment.
 */
struct crocus_surface {
   struct pipe_surface base;
   struct isl_view view;
   struct isl_view read_view;
   struct isl_surf surf;
   union isl_color_value clear_color;

   struct pipe_resource *align_res;
};

/**
 * Transfer object - information about a buffer mapping.
 */
struct crocus_transfer {
   struct threaded_transfer base;
   struct util_debug_callback *dbg;
   void *buffer;
   void *ptr;

   /** A linear staging resource for GPU-based copy_region transfers. */
   struct pipe_resource *staging;
   struct blorp_context *blorp;
   struct crocus_batch *batch;

   bool dest_had_defined_contents;
   bool has_swizzling;

   void (*unmap)(struct crocus_transfer *);
};

/**
 * Memory Object
 */
struct crocus_memory_object {
   struct pipe_memory_object b;
   struct crocus_bo *bo;
   uint64_t format;
   unsigned stride;
};

/**
 * Unwrap a pipe_resource to get the underlying crocus_bo (for convenience).
 */
static inline struct crocus_bo *
crocus_resource_bo(struct pipe_resource *p_res)
{
   struct crocus_resource *res = (void *) p_res;
   return res->bo;
}

static inline uint32_t
crocus_mocs(const struct crocus_bo *bo,
            const struct isl_device *dev)
{
   return isl_mocs(dev, 0, bo && crocus_bo_is_external(bo));
}

struct crocus_format_info crocus_format_for_usage(const struct intel_device_info *,
                                                  enum pipe_format pf,
                                                  isl_surf_usage_flags_t usage);

static inline struct pipe_resource *
_crocus_resource_get_separate_stencil(struct pipe_resource *p_res)
{
   /* For packed depth-stencil, we treat depth as the primary resource
    * and store S8 as the "second plane" resource.
    */
   if (p_res->next && p_res->next->format == PIPE_FORMAT_S8_UINT)
      return p_res->next;

   return NULL;

}
static inline void
crocus_get_depth_stencil_resources(const struct intel_device_info *devinfo,
                                   struct pipe_resource *res,
                                   struct crocus_resource **out_z,
                                   struct crocus_resource **out_s)
{
   /* gen4/5 only supports packed ds */
   if (devinfo->ver < 6) {
      *out_z = (void *)res;
      *out_s = (void *)res;
      return;
   }

   if (res && res->format != PIPE_FORMAT_S8_UINT) {
      *out_z = (void *) res;
      *out_s = (void *) _crocus_resource_get_separate_stencil(res);
   } else {
      *out_z = NULL;
      *out_s = (void *) res;
   }
}


bool crocus_resource_set_clear_color(struct crocus_context *ice,
                                     struct crocus_resource *res,
                                     union isl_color_value color);
union isl_color_value
crocus_resource_get_clear_color(const struct crocus_resource *res);

void
crocus_replace_buffer_storage(struct pipe_context *ctx,
                              struct pipe_resource *p_dst,
                              struct pipe_resource *p_src,
                              unsigned num_rebinds,
                              uint32_t rebind_mask,
                              uint32_t delete_buffer_id);

void crocus_init_screen_resource_functions(struct pipe_screen *pscreen);

void crocus_dirty_for_history(struct crocus_context *ice,
                              struct crocus_resource *res);
uint32_t crocus_flush_bits_for_history(struct crocus_resource *res);

void crocus_flush_and_dirty_for_history(struct crocus_context *ice,
                                        struct crocus_batch *batch,
                                        struct crocus_resource *res,
                                        uint32_t extra_flags,
                                        const char *reason);

unsigned crocus_get_num_logical_layers(const struct crocus_resource *res,
                                       unsigned level);

void crocus_resource_disable_aux(struct crocus_resource *res);

#define INTEL_REMAINING_LAYERS UINT32_MAX
#define INTEL_REMAINING_LEVELS UINT32_MAX

void
crocus_hiz_exec(struct crocus_context *ice,
                struct crocus_batch *batch,
                struct crocus_resource *res,
                unsigned int level, unsigned int start_layer,
                unsigned int num_layers, enum isl_aux_op op,
                bool update_clear_depth);

/**
 * Prepare a miptree for access
 *
 * This function should be called prior to any access to miptree in order to
 * perform any needed resolves.
 *
 * \param[in]  start_level    The first mip level to be accessed
 *
 * \param[in]  num_levels     The number of miplevels to be accessed or
 *                            INTEL_REMAINING_LEVELS to indicate every level
 *                            above start_level will be accessed
 *
 * \param[in]  start_layer    The first array slice or 3D layer to be accessed
 *
 * \param[in]  num_layers     The number of array slices or 3D layers be
 *                            accessed or INTEL_REMAINING_LAYERS to indicate
 *                            every layer above start_layer will be accessed
 *
 * \param[in]  aux_supported  Whether or not the access will support the
 *                            miptree's auxiliary compression format;  this
 *                            must be false for uncompressed miptrees
 *
 * \param[in]  fast_clear_supported Whether or not the access will support
 *                                  fast clears in the miptree's auxiliary
 *                                  compression format
 */
void
crocus_resource_prepare_access(struct crocus_context *ice,
                               struct crocus_resource *res,
                               uint32_t start_level, uint32_t num_levels,
                               uint32_t start_layer, uint32_t num_layers,
                               enum isl_aux_usage aux_usage,
                               bool fast_clear_supported);

/**
 * Complete a write operation
 *
 * This function should be called after any operation writes to a miptree.
 * This will update the miptree's compression state so that future resolves
 * happen correctly.  Technically, this function can be called before the
 * write occurs but the caller must ensure that they don't interlace
 * crocus_resource_prepare_access and crocus_resource_finish_write calls to
 * overlapping layer/level ranges.
 *
 * \param[in]  level             The mip level that was written
 *
 * \param[in]  start_layer       The first array slice or 3D layer written
 *
 * \param[in]  num_layers        The number of array slices or 3D layers
 *                               written or INTEL_REMAINING_LAYERS to indicate
 *                               every layer above start_layer was written
 *
 * \param[in]  written_with_aux  Whether or not the write was done with
 *                               auxiliary compression enabled
 */
void
crocus_resource_finish_write(struct crocus_context *ice,
                             struct crocus_resource *res, uint32_t level,
                             uint32_t start_layer, uint32_t num_layers,
                             enum isl_aux_usage aux_usage);

/** Get the auxiliary compression state of a miptree slice */
enum isl_aux_state
crocus_resource_get_aux_state(const struct crocus_resource *res,
                              uint32_t level, uint32_t layer);

/**
 * Set the auxiliary compression state of a miptree slice range
 *
 * This function directly sets the auxiliary compression state of a slice
 * range of a miptree.  It only modifies data structures and does not do any
 * resolves.  This should only be called by code which directly performs
 * compression operations such as fast clears and resolves.  Most code should
 * use crocus_resource_prepare_access or crocus_resource_finish_write.
 */
void
crocus_resource_set_aux_state(struct crocus_context *ice,
                              struct crocus_resource *res, uint32_t level,
                              uint32_t start_layer, uint32_t num_layers,
                              enum isl_aux_state aux_state);

/**
 * Prepare a miptree for raw access
 *
 * This helper prepares the miptree for access that knows nothing about any
 * sort of compression whatsoever.  This is useful when mapping the surface or
 * using it with the blitter.
 */
static inline void
crocus_resource_access_raw(struct crocus_context *ice,
                           struct crocus_resource *res,
                           uint32_t level, uint32_t layer,
                           uint32_t num_layers,
                           bool write)
{
   crocus_resource_prepare_access(ice, res, level, 1, layer, num_layers,
                                  ISL_AUX_USAGE_NONE, false);
   if (write) {
      crocus_resource_finish_write(ice, res, level, layer, num_layers,
                                   ISL_AUX_USAGE_NONE);
   }
}

void
crocus_resource_get_image_offset(struct crocus_resource *res,
                                 uint32_t level, uint32_t z,
                                 uint32_t *x, uint32_t *y);
static inline enum isl_aux_usage
crocus_resource_texture_aux_usage(const struct crocus_resource *res)
{
   return res->aux.usage == ISL_AUX_USAGE_MCS ? ISL_AUX_USAGE_MCS : ISL_AUX_USAGE_NONE;
}

void crocus_resource_prepare_texture(struct crocus_context *ice,
                                     struct crocus_resource *res,
                                     enum isl_format view_format,
                                     uint32_t start_level, uint32_t num_levels,
                                     uint32_t start_layer, uint32_t num_layers);

bool crocus_has_invalid_primary(const struct crocus_resource *res,
                                unsigned start_level, unsigned num_levels,
                                unsigned start_layer, unsigned num_layers);

void crocus_resource_check_level_layer(const struct crocus_resource *res,
                                       uint32_t level, uint32_t layer);

bool crocus_resource_level_has_hiz(const struct crocus_resource *res,
                                   uint32_t level);
bool crocus_has_color_unresolved(const struct crocus_resource *res,
                                 unsigned start_level, unsigned num_levels,
                                 unsigned start_layer, unsigned num_layers);

bool crocus_render_formats_color_compatible(enum isl_format a,
                                          enum isl_format b,
                                          union isl_color_value color);
enum isl_aux_usage crocus_resource_render_aux_usage(struct crocus_context *ice,
                                                    struct crocus_resource *res,
                                                    uint32_t level,
                                                    enum isl_format render_fmt,
                                                    bool draw_aux_disabled);
void crocus_resource_prepare_render(struct crocus_context *ice,
                                    struct crocus_resource *res, uint32_t level,
                                    uint32_t start_layer, uint32_t layer_count,
                                    enum isl_aux_usage aux_usage);
void crocus_resource_finish_render(struct crocus_context *ice,
                                   struct crocus_resource *res, uint32_t level,
                                   uint32_t start_layer, uint32_t layer_count,
                                   enum isl_aux_usage aux_usage);
#endif
