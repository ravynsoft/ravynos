/*
 * Copyright 2018 Collabora Ltd.
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

#ifndef ZINK_CONTEXT_H
#define ZINK_CONTEXT_H

#include "util/u_rect.h"
#include "zink_types.h"
#include "vk_enum_to_str.h"

#define GFX_SHADER_BITS (VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | \
                         VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | \
                         VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | \
                         VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | \
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)

#define pipe_buffer_write "use tc_buffer_write to avoid breaking threaded context"

#ifdef __cplusplus
extern "C" {
#endif

struct blitter_context;
struct list_head;

struct zink_blend_state;
struct zink_depth_stencil_alpha_state;
struct zink_gfx_program;
struct zink_rasterizer_state;
struct zink_resource;
struct zink_vertex_elements_state;

#define perf_debug(ctx, ...) do {                      \
   util_debug_message(&ctx->dbg, PERF_INFO, __VA_ARGS__); \
} while(0)


static inline struct zink_resource *
zink_descriptor_surface_resource(struct zink_descriptor_surface *ds)
{
   return ds->is_buffer ?
          zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB ? zink_resource(ds->db.pres) : zink_resource(ds->bufferview->pres) :
          (struct zink_resource*)ds->surface->base.texture;
}

static inline bool
zink_fb_clear_enabled(const struct zink_context *ctx, unsigned idx)
{
   if (idx == PIPE_MAX_COLOR_BUFS)
      return ctx->clears_enabled & PIPE_CLEAR_DEPTHSTENCIL;
   return ctx->clears_enabled & (PIPE_CLEAR_COLOR0 << idx);
}

static inline uint32_t
zink_program_cache_stages(uint32_t stages_present)
{
   return (stages_present & ((1 << MESA_SHADER_TESS_CTRL) |
                             (1 << MESA_SHADER_TESS_EVAL) |
                             (1 << MESA_SHADER_GEOMETRY))) >> 1;
}

static ALWAYS_INLINE bool
zink_is_zsbuf_used(const struct zink_context *ctx)
{
   return ctx->blitting || tc_renderpass_info_is_zsbuf_used(&ctx->dynamic_fb.tc_info);
}

static ALWAYS_INLINE bool
zink_is_zsbuf_write(const struct zink_context *ctx)
{
   if (!zink_is_zsbuf_used(ctx))
      return false;
   return ctx->dynamic_fb.tc_info.zsbuf_write_fs || ctx->dynamic_fb.tc_info.zsbuf_write_dsa ||
          ctx->dynamic_fb.tc_info.zsbuf_clear || ctx->dynamic_fb.tc_info.zsbuf_clear_partial;
}

void
zink_fence_wait(struct pipe_context *ctx);

void
zink_wait_on_batch(struct zink_context *ctx, uint64_t batch_id);
void
zink_reset_ds3_states(struct zink_context *ctx);
bool
zink_check_batch_completion(struct zink_context *ctx, uint64_t batch_id);
VkCommandBuffer
zink_get_cmdbuf(struct zink_context *ctx, struct zink_resource *src, struct zink_resource *dst);
unsigned
zink_update_rendering_info(struct zink_context *ctx);
void
zink_flush_queue(struct zink_context *ctx);
void
zink_update_fbfetch(struct zink_context *ctx);
bool
zink_resource_access_is_write(VkAccessFlags flags);

void
zink_resource_buffer_barrier(struct zink_context *ctx, struct zink_resource *res, VkAccessFlags flags, VkPipelineStageFlags pipeline);
void
zink_resource_buffer_barrier2(struct zink_context *ctx, struct zink_resource *res, VkAccessFlags flags, VkPipelineStageFlags pipeline);
bool
zink_resource_image_needs_barrier(struct zink_resource *res, VkImageLayout new_layout, VkAccessFlags flags, VkPipelineStageFlags pipeline);
void
zink_resource_image_barrier_init(VkImageMemoryBarrier *imb, struct zink_resource *res, VkImageLayout new_layout, VkAccessFlags flags, VkPipelineStageFlags pipeline);
void
zink_resource_image_barrier2_init(VkImageMemoryBarrier2 *imb, struct zink_resource *res, VkImageLayout new_layout, VkAccessFlags flags, VkPipelineStageFlags pipeline);
void
zink_resource_image_barrier(struct zink_context *ctx, struct zink_resource *res,
                      VkImageLayout new_layout, VkAccessFlags flags, VkPipelineStageFlags pipeline);
void
zink_resource_image_barrier2(struct zink_context *ctx, struct zink_resource *res, VkImageLayout new_layout, VkAccessFlags flags, VkPipelineStageFlags pipeline);
bool
zink_check_unordered_transfer_access(struct zink_resource *res, unsigned level, const struct pipe_box *box);
bool
zink_check_valid_buffer_src_access(struct zink_context *ctx, struct zink_resource *res, unsigned offset, unsigned size);
void
zink_resource_image_transfer_dst_barrier(struct zink_context *ctx, struct zink_resource *res, unsigned level, const struct pipe_box *box, bool unsync);
bool
zink_resource_buffer_transfer_dst_barrier(struct zink_context *ctx, struct zink_resource *res, unsigned offset, unsigned size);
void
zink_synchronization_init(struct zink_screen *screen);
void
zink_update_descriptor_refs(struct zink_context *ctx, bool compute);
void
zink_init_vk_sample_locations(struct zink_context *ctx, VkSampleLocationsInfoEXT *loc);

void
zink_batch_rp(struct zink_context *ctx);

void
zink_batch_no_rp(struct zink_context *ctx);
void
zink_batch_no_rp_safe(struct zink_context *ctx);

VkImageView
zink_prep_fb_attachment(struct zink_context *ctx, struct zink_surface *surf, unsigned i);
void
zink_update_vk_sample_locations(struct zink_context *ctx);

static inline VkPipelineStageFlags
zink_pipeline_flags_from_pipe_stage(gl_shader_stage pstage)
{
   switch (pstage) {
   case MESA_SHADER_VERTEX:
      return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
   case MESA_SHADER_FRAGMENT:
      return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
   case MESA_SHADER_GEOMETRY:
      return VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
   case MESA_SHADER_TESS_CTRL:
      return VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
   case MESA_SHADER_TESS_EVAL:
      return VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
   case MESA_SHADER_COMPUTE:
      return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
   default:
      unreachable("unknown shader stage");
   }
}

void
zink_rebind_all_buffers(struct zink_context *ctx);
void
zink_rebind_all_images(struct zink_context *ctx);

void
zink_parse_tc_info(struct zink_context *ctx);
void
zink_flush_memory_barrier(struct zink_context *ctx, bool is_compute);
void
zink_init_draw_functions(struct zink_context *ctx, struct zink_screen *screen);
void
zink_init_grid_functions(struct zink_context *ctx);
struct zink_context *
zink_tc_context_unwrap(struct pipe_context *pctx, bool threaded);

void
zink_update_barriers(struct zink_context *ctx, bool is_compute,
                     struct pipe_resource *index, struct pipe_resource *indirect, struct pipe_resource *indirect_draw_count);


bool
zink_cmd_debug_marker_begin(struct zink_context *ctx, VkCommandBuffer cmdbuf, const char *fmt, ...);
void
zink_cmd_debug_marker_end(struct zink_context *ctx, VkCommandBuffer cmdbuf,bool emitted);
void
zink_copy_buffer(struct zink_context *ctx, struct zink_resource *dst, struct zink_resource *src,
                 unsigned dst_offset, unsigned src_offset, unsigned size);

VkIndirectCommandsLayoutTokenNV *
zink_dgc_add_token(struct zink_context *ctx, VkIndirectCommandsTokenTypeNV type, void **mem);
void
zink_flush_dgc(struct zink_context *ctx);

static ALWAYS_INLINE void
zink_flush_dgc_if_enabled(struct zink_context *ctx)
{
   if (unlikely(zink_debug & ZINK_DEBUG_DGC))
      zink_flush_dgc(ctx);
}

#ifdef __cplusplus
}
#endif

#ifndef __cplusplus
VkPipelineStageFlags
zink_pipeline_flags_from_stage(VkShaderStageFlagBits stage);

struct pipe_context *
zink_context_create(struct pipe_screen *pscreen, void *priv, unsigned flags);

void
zink_context_query_init(struct pipe_context *ctx);

void
zink_blit_begin(struct zink_context *ctx, enum zink_blit_flags flags);
void
zink_blit_barriers(struct zink_context *ctx, struct zink_resource *src, struct zink_resource *dst, bool whole_dst);

void
zink_blit(struct pipe_context *pctx,
          const struct pipe_blit_info *info);

bool
zink_blit_region_fills(struct u_rect region, unsigned width, unsigned height);

bool
zink_blit_region_covers(struct u_rect region, struct u_rect covers);

static inline struct u_rect
zink_rect_from_box(const struct pipe_box *box)
{
   return (struct u_rect){box->x, box->x + box->width, box->y, box->y + box->height};
}

static inline VkComponentSwizzle
zink_component_mapping(enum pipe_swizzle swizzle)
{
   switch (swizzle) {
   case PIPE_SWIZZLE_X: return VK_COMPONENT_SWIZZLE_R;
   case PIPE_SWIZZLE_Y: return VK_COMPONENT_SWIZZLE_G;
   case PIPE_SWIZZLE_Z: return VK_COMPONENT_SWIZZLE_B;
   case PIPE_SWIZZLE_W: return VK_COMPONENT_SWIZZLE_A;
   case PIPE_SWIZZLE_0: return VK_COMPONENT_SWIZZLE_ZERO;
   case PIPE_SWIZZLE_1: return VK_COMPONENT_SWIZZLE_ONE;
   default:
      unreachable("unexpected swizzle");
   }
}

void
zink_update_shadow_samplerviews(struct zink_context *ctx, unsigned mask);

enum pipe_swizzle
zink_clamp_void_swizzle(const struct util_format_description *desc, enum pipe_swizzle swizzle);

bool
zink_resource_rebind(struct zink_context *ctx, struct zink_resource *res);

void
zink_rebind_framebuffer(struct zink_context *ctx, struct zink_resource *res);
void
zink_set_null_fs(struct zink_context *ctx);

void
zink_copy_image_buffer(struct zink_context *ctx, struct zink_resource *dst, struct zink_resource *src,
                       unsigned dst_level, unsigned dstx, unsigned dsty, unsigned dstz,
                       unsigned src_level, const struct pipe_box *src_box, enum pipe_map_flags map_flags);

void
zink_destroy_buffer_view(struct zink_screen *screen, struct zink_buffer_view *buffer_view);

struct pipe_surface *
zink_get_dummy_pipe_surface(struct zink_context *ctx, int samples_index);
struct zink_surface *
zink_get_dummy_surface(struct zink_context *ctx, int samples_index);

void
debug_describe_zink_buffer_view(char *buf, const struct zink_buffer_view *ptr);

static inline void
zink_buffer_view_reference(struct zink_screen *screen,
                           struct zink_buffer_view **dst,
                           struct zink_buffer_view *src)
{
   struct zink_buffer_view *old_dst = dst ? *dst : NULL;

   if (pipe_reference_described(old_dst ? &old_dst->reference : NULL, &src->reference,
                                (debug_reference_descriptor)debug_describe_zink_buffer_view))
      zink_destroy_buffer_view(screen, old_dst);
   if (dst) *dst = src;
}
#endif

#endif
