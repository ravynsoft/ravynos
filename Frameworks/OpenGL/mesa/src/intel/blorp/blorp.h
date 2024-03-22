/*
 * Copyright © 2012 Intel Corporation
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

#ifndef BLORP_H
#define BLORP_H

#include <stdint.h>
#include <stdbool.h>

#include "isl/isl.h"

struct brw_stage_prog_data;

#ifdef __cplusplus
extern "C" {
#endif

enum blorp_op {
   BLORP_OP_BLIT,
   BLORP_OP_COPY,
   BLORP_OP_CCS_AMBIGUATE,
   BLORP_OP_CCS_COLOR_CLEAR,
   BLORP_OP_CCS_PARTIAL_RESOLVE,
   BLORP_OP_CCS_RESOLVE,
   BLORP_OP_HIZ_AMBIGUATE,
   BLORP_OP_HIZ_CLEAR,
   BLORP_OP_HIZ_RESOLVE,
   BLORP_OP_MCS_AMBIGUATE,
   BLORP_OP_MCS_COLOR_CLEAR,
   BLORP_OP_MCS_PARTIAL_RESOLVE,
   BLORP_OP_SLOW_COLOR_CLEAR,
   BLORP_OP_SLOW_DEPTH_CLEAR,
};

struct blorp_batch;
struct blorp_params;

struct blorp_config {
   bool use_mesh_shading;
   bool use_unrestricted_depth_range;
};

struct blorp_context {
   void *driver_ctx;

   const struct isl_device *isl_dev;

   const struct brw_compiler *compiler;

   bool enable_tbimr;

   bool (*lookup_shader)(struct blorp_batch *batch,
                         const void *key, uint32_t key_size,
                         uint32_t *kernel_out, void *prog_data_out);
   bool (*upload_shader)(struct blorp_batch *batch,
                         uint32_t stage,
                         const void *key, uint32_t key_size,
                         const void *kernel, uint32_t kernel_size,
                         const struct brw_stage_prog_data *prog_data,
                         uint32_t prog_data_size,
                         uint32_t *kernel_out, void *prog_data_out);
   void (*exec)(struct blorp_batch *batch, const struct blorp_params *params);

   struct blorp_config config;
};

void blorp_init(struct blorp_context *blorp, void *driver_ctx,
                struct isl_device *isl_dev, const struct blorp_config *config);
void blorp_finish(struct blorp_context *blorp);

enum blorp_batch_flags {
   /**
    * This flag indicates that blorp should *not* re-emit the depth and
    * stencil buffer packets.  Instead, the driver guarantees that all depth
    * and stencil images passed in will match what is currently set in the
    * hardware.
    */
   BLORP_BATCH_NO_EMIT_DEPTH_STENCIL = (1 << 0),

   /* This flag indicates that the blorp call should be predicated. */
   BLORP_BATCH_PREDICATE_ENABLE      = (1 << 1),

   /* This flag indicates that blorp should *not* update the indirect clear
    * color buffer.
    */
   BLORP_BATCH_NO_UPDATE_CLEAR_COLOR = (1 << 2),

   /* This flag indicates that blorp should use a compute program for the
    * operation.
    */
   BLORP_BATCH_USE_COMPUTE = (1 << 3),

   /** Use the hardware blitter to perform any operations in this batch */
   BLORP_BATCH_USE_BLITTER = (1 << 4),
};

struct blorp_batch {
   struct blorp_context *blorp;
   void *driver_batch;
   enum blorp_batch_flags flags;
};

void blorp_batch_init(struct blorp_context *blorp, struct blorp_batch *batch,
                      void *driver_batch, enum blorp_batch_flags flags);
void blorp_batch_finish(struct blorp_batch *batch);

static inline isl_surf_usage_flags_t
blorp_batch_isl_copy_usage(const struct blorp_batch *batch, bool is_dest)
{
   if (batch->flags & BLORP_BATCH_USE_COMPUTE)
      return is_dest ? ISL_SURF_USAGE_STORAGE_BIT : ISL_SURF_USAGE_TEXTURE_BIT;
   else if (batch->flags & BLORP_BATCH_USE_BLITTER)
      return is_dest ? ISL_SURF_USAGE_BLITTER_DST_BIT : ISL_SURF_USAGE_BLITTER_SRC_BIT;
   else
      return is_dest ? ISL_SURF_USAGE_RENDER_TARGET_BIT : ISL_SURF_USAGE_TEXTURE_BIT;
}

struct blorp_address {
   void *buffer;
   int64_t offset;
   unsigned reloc_flags;
   uint32_t mocs;

   /**
    * True if this buffer is intended to live in device-local memory.
    * This is only a performance hint; it's OK to set it to true even
    * if eviction has temporarily forced the buffer to system memory.
    */
   bool local_hint;
};

static inline bool
blorp_address_is_null(struct blorp_address address)
{
   return address.buffer == NULL && address.offset == 0;
}

struct blorp_surf
{
   const struct isl_surf *surf;
   struct blorp_address addr;

   const struct isl_surf *aux_surf;
   struct blorp_address aux_addr;
   enum isl_aux_usage aux_usage;

   union isl_color_value clear_color;

   /**
    * If set (bo != NULL), clear_color is ignored and the actual clear color
    * is fetched from this address.  On gfx7-8, this is all of dword 7 of
    * RENDER_SURFACE_STATE and is the responsibility of the caller to ensure
    * that it contains a swizzle of RGBA and resource min LOD of 0.
    */
   struct blorp_address clear_color_addr;

   /* Only allowed for simple 2D non-MSAA surfaces */
   uint32_t tile_x_sa, tile_y_sa;
};

enum blorp_filter {
   BLORP_FILTER_NONE,
   BLORP_FILTER_NEAREST,
   BLORP_FILTER_BILINEAR,
   BLORP_FILTER_SAMPLE_0,
   BLORP_FILTER_AVERAGE,
   BLORP_FILTER_MIN_SAMPLE,
   BLORP_FILTER_MAX_SAMPLE,
};

void
blorp_blit(struct blorp_batch *batch,
           const struct blorp_surf *src_surf,
           unsigned src_level, float src_layer,
           enum isl_format src_format, struct isl_swizzle src_swizzle,
           const struct blorp_surf *dst_surf,
           unsigned dst_level, unsigned dst_layer,
           enum isl_format dst_format, struct isl_swizzle dst_swizzle,
           float src_x0, float src_y0,
           float src_x1, float src_y1,
           float dst_x0, float dst_y0,
           float dst_x1, float dst_y1,
           enum blorp_filter filter,
           bool mirror_x, bool mirror_y);

void
blorp_copy_get_formats(const struct isl_device *isl_dev,
                       const struct isl_surf *src_surf,
                       const struct isl_surf *dst_surf,
                       enum isl_format *src_view_format,
                       enum isl_format *dst_view_format);

void
blorp_copy(struct blorp_batch *batch,
           const struct blorp_surf *src_surf,
           unsigned src_level, unsigned src_layer,
           const struct blorp_surf *dst_surf,
           unsigned dst_level, unsigned dst_layer,
           uint32_t src_x, uint32_t src_y,
           uint32_t dst_x, uint32_t dst_y,
           uint32_t src_width, uint32_t src_height);

void
blorp_buffer_copy(struct blorp_batch *batch,
                  struct blorp_address src,
                  struct blorp_address dst,
                  uint64_t size);

void
blorp_fast_clear(struct blorp_batch *batch,
                 const struct blorp_surf *surf,
                 enum isl_format format, struct isl_swizzle swizzle,
                 uint32_t level, uint32_t start_layer, uint32_t num_layers,
                 uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1);

bool
blorp_clear_supports_compute(struct blorp_context *blorp,
                             uint8_t color_write_disable, bool blend_enabled,
                             enum isl_aux_usage aux_usage);

bool
blorp_clear_supports_blitter(struct blorp_context *blorp,
                             const struct blorp_surf *surf,
                             uint8_t color_write_disable, bool blend_enabled);

bool
blorp_copy_supports_compute(struct blorp_context *blorp,
                            const struct isl_surf *src_surf,
                            const struct isl_surf *dst_surf,
                            enum isl_aux_usage dst_aux_usage);

bool
blorp_blit_supports_compute(struct blorp_context *blorp,
                            const struct isl_surf *src_surf,
                            const struct isl_surf *dst_surf,
                            enum isl_aux_usage dst_aux_usage);

bool
blorp_copy_supports_blitter(struct blorp_context *blorp,
                            const struct isl_surf *src_surf,
                            const struct isl_surf *dst_surf,
                            enum isl_aux_usage src_aux_usage,
                            enum isl_aux_usage dst_aux_usage);

void
blorp_clear(struct blorp_batch *batch,
            const struct blorp_surf *surf,
            enum isl_format format, struct isl_swizzle swizzle,
            uint32_t level, uint32_t start_layer, uint32_t num_layers,
            uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1,
            union isl_color_value clear_color,
            uint8_t color_write_disable);

void
blorp_clear_depth_stencil(struct blorp_batch *batch,
                          const struct blorp_surf *depth,
                          const struct blorp_surf *stencil,
                          uint32_t level, uint32_t start_layer,
                          uint32_t num_layers,
                          uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1,
                          bool clear_depth, float depth_value,
                          uint8_t stencil_mask, uint8_t stencil_value);
bool
blorp_can_hiz_clear_depth(const struct intel_device_info *devinfo,
                          const struct isl_surf *surf,
                          enum isl_aux_usage aux_usage,
                          uint32_t level, uint32_t layer,
                          uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1);
void
blorp_hiz_clear_depth_stencil(struct blorp_batch *batch,
                              const struct blorp_surf *depth,
                              const struct blorp_surf *stencil,
                              uint32_t level,
                              uint32_t start_layer, uint32_t num_layers,
                              uint32_t x0, uint32_t y0,
                              uint32_t x1, uint32_t y1,
                              bool clear_depth, float depth_value,
                              bool clear_stencil, uint8_t stencil_value);


void
blorp_gfx8_hiz_clear_attachments(struct blorp_batch *batch,
                                 uint32_t num_samples,
                                 uint32_t x0, uint32_t y0,
                                 uint32_t x1, uint32_t y1,
                                 bool clear_depth, bool clear_stencil,
                                 uint8_t stencil_value);
void
blorp_clear_attachments(struct blorp_batch *batch,
                        uint32_t binding_table_offset,
                        enum isl_format depth_format,
                        uint32_t num_samples,
                        uint32_t start_layer, uint32_t num_layers,
                        uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1,
                        bool clear_color, union isl_color_value color_value,
                        bool clear_depth, float depth_value,
                        uint8_t stencil_mask, uint8_t stencil_value);

void
blorp_ccs_resolve(struct blorp_batch *batch,
                  struct blorp_surf *surf, uint32_t level,
                  uint32_t start_layer, uint32_t num_layers,
                  enum isl_format format,
                  enum isl_aux_op resolve_op);

void
blorp_ccs_ambiguate(struct blorp_batch *batch,
                    struct blorp_surf *surf,
                    uint32_t level, uint32_t layer);

void
blorp_mcs_partial_resolve(struct blorp_batch *batch,
                          struct blorp_surf *surf,
                          enum isl_format format,
                          uint32_t start_layer, uint32_t num_layers);

void
blorp_mcs_ambiguate(struct blorp_batch *batch,
                    struct blorp_surf *surf,
                    uint32_t start_layer, uint32_t num_layers);

void
blorp_hiz_op(struct blorp_batch *batch, struct blorp_surf *surf,
             uint32_t level, uint32_t start_layer, uint32_t num_layers,
             enum isl_aux_op op);

#ifdef __cplusplus
} /* end extern "C" */
#endif /* __cplusplus */

#endif /* BLORP_H */
