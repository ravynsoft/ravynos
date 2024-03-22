/*
 * Copyright (C) 2022 Lima Project
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "drm-uapi/lima_drm.h"

#include "util/u_math.h"
#include "util/format/u_format.h"
#include "util/u_surface.h"
#include "util/u_inlines.h"
#include "util/hash_table.h"

#include "lima_context.h"
#include "lima_gpu.h"
#include "lima_resource.h"
#include "lima_texture.h"
#include "lima_format.h"
#include "lima_job.h"
#include "lima_screen.h"
#include "lima_bo.h"
#include "lima_parser.h"
#include "lima_util.h"
#include "lima_blit.h"

void
lima_pack_blit_cmd(struct lima_job *job,
                   struct util_dynarray *cmd_array,
                   struct pipe_surface *psurf,
                   const struct pipe_box *src,
                   const struct pipe_box *dst,
                   unsigned filter,
                   bool scissor,
                   unsigned sample_mask,
                   unsigned mrt_idx)
{
   #define lima_blit_render_state_offset 0x0000
   #define lima_blit_gl_pos_offset       0x0040
   #define lima_blit_varying_offset      0x0080
   #define lima_blit_tex_desc_offset     0x00c0
   #define lima_blit_tex_array_offset    0x0100
   #define lima_blit_buffer_size         0x0140

   struct lima_context *ctx = job->ctx;
   struct lima_surface *surf = lima_surface(psurf);
   int level = psurf->u.tex.level;
   unsigned first_layer = psurf->u.tex.first_layer;
   float fb_width = dst->width, fb_height = dst->height;

   uint32_t va;
   void *cpu = lima_job_create_stream_bo(
      job, LIMA_PIPE_PP, lima_blit_buffer_size, &va);

   struct lima_screen *screen = lima_screen(ctx->base.screen);

   uint32_t reload_shader_first_instr_size =
      ((uint32_t *)(screen->pp_buffer->map + pp_reload_program_offset))[0] & 0x1f;
   uint32_t reload_shader_va = screen->pp_buffer->va + pp_reload_program_offset;

   struct lima_render_state reload_render_state = {
      .alpha_blend = 0xf03b1ad2,
      .depth_test = 0x0000000e,
      .depth_range = 0xffff0000,
      .stencil_front = 0x00000007,
      .stencil_back = 0x00000007,
      .multi_sample = 0x00000007,
      .shader_address = reload_shader_va | reload_shader_first_instr_size,
      .varying_types = 0x00000001,
      .textures_address = va + lima_blit_tex_array_offset,
      .aux0 = 0x00004021,
      .varyings_address = va + lima_blit_varying_offset,
   };

   reload_render_state.multi_sample |= (sample_mask << 12);

   if (job->key.cbuf) {
      fb_width = job->key.cbuf->width;
      fb_height = job->key.cbuf->height;
   } else {
      fb_width = job->key.zsbuf->width;
      fb_height = job->key.zsbuf->height;
   }

   if (util_format_is_depth_or_stencil(psurf->format)) {
      reload_render_state.alpha_blend &= 0x0fffffff;
      if (psurf->format != PIPE_FORMAT_Z16_UNORM)
         reload_render_state.depth_test |= 0x400;
      if (surf->reload & PIPE_CLEAR_DEPTH)
         reload_render_state.depth_test |= 0x801;
      if (surf->reload & PIPE_CLEAR_STENCIL) {
         reload_render_state.depth_test |= 0x1000;
         reload_render_state.stencil_front = 0x0000024f;
         reload_render_state.stencil_back = 0x0000024f;
         reload_render_state.stencil_test = 0x0000ffff;
      }
   }

   memcpy(cpu + lima_blit_render_state_offset, &reload_render_state,
          sizeof(reload_render_state));

   lima_tex_desc *td = cpu + lima_blit_tex_desc_offset;
   memset(td, 0, lima_min_tex_desc_size);
   lima_texture_desc_set_res(ctx, td, psurf->texture, level, level,
                             first_layer, mrt_idx);
   td->format = lima_format_get_texel_reload(psurf->format);
   td->unnorm_coords = 1;
   td->sampler_dim = LIMA_SAMPLER_DIM_2D;
   td->min_img_filter_nearest = 1;
   td->mag_img_filter_nearest = 1;
   td->wrap_s = LIMA_TEX_WRAP_CLAMP_TO_EDGE;
   td->wrap_t = LIMA_TEX_WRAP_CLAMP_TO_EDGE;
   td->wrap_r = LIMA_TEX_WRAP_CLAMP_TO_EDGE;

   if (filter != PIPE_TEX_FILTER_NEAREST) {
      td->min_img_filter_nearest = 0;
      td->mag_img_filter_nearest = 0;
   }

   uint32_t *ta = cpu + lima_blit_tex_array_offset;
   ta[0] = va + lima_blit_tex_desc_offset;

   float reload_gl_pos[] = {
      dst->x + dst->width, dst->y,      0, 1,
      dst->x,              dst->y,      0, 1,
      dst->x, dst->y + dst->height,     0, 1,
   };
   memcpy(cpu + lima_blit_gl_pos_offset, reload_gl_pos,
          sizeof(reload_gl_pos));

   float reload_varying[] = {
      src->x + src->width, src->y,
      src->x,              src->y,
      src->x,              src->y + src->height,
      0, 0, /* unused */
   };
   memcpy(cpu + lima_blit_varying_offset, reload_varying,
          sizeof(reload_varying));

   PLBU_CMD_BEGIN(cmd_array, scissor ? 22 : 20);

   PLBU_CMD_VIEWPORT_LEFT(0);
   PLBU_CMD_VIEWPORT_RIGHT(fui(fb_width));
   PLBU_CMD_VIEWPORT_BOTTOM(0);
   PLBU_CMD_VIEWPORT_TOP(fui(fb_height));

   PLBU_CMD_RSW_VERTEX_ARRAY(
      va + lima_blit_render_state_offset,
      va + lima_blit_gl_pos_offset);


   if (scissor) {
      int minx = MIN2(dst->x, dst->x + dst->width);
      int maxx = MAX2(dst->x, dst->x + dst->width);
      int miny = MIN2(dst->y, dst->y + dst->height);
      int maxy = MAX2(dst->y, dst->y + dst->height);

      PLBU_CMD_SCISSORS(minx, maxx, miny, maxy);
      lima_damage_rect_union(&job->damage_rect, minx, maxx, miny, maxy);
   }

   PLBU_CMD_UNKNOWN2();
   PLBU_CMD_UNKNOWN1();

   PLBU_CMD_INDICES(screen->pp_buffer->va + pp_shared_index_offset);
   PLBU_CMD_INDEXED_DEST(va + lima_blit_gl_pos_offset);
   PLBU_CMD_DRAW_ELEMENTS(0xf, 0, 3);

   PLBU_CMD_END();

   lima_dump_command_stream_print(job->dump, cpu, lima_blit_buffer_size,
                                  false, "blit plbu cmd at va %x\n", va);
}

static struct pipe_surface *
lima_get_blit_surface(struct pipe_context *pctx,
                      struct pipe_resource *prsc,
                      unsigned level)
{
   struct pipe_surface tmpl;

   memset(&tmpl, 0, sizeof(tmpl));
   tmpl.format = prsc->format;
   tmpl.u.tex.level = level;
   tmpl.u.tex.first_layer = 0;
   tmpl.u.tex.last_layer = 0;

   return pctx->create_surface(pctx, prsc, &tmpl);
}

bool
lima_do_blit(struct pipe_context *pctx,
             const struct pipe_blit_info *info)
{
   struct lima_context *ctx = lima_context(pctx);
   unsigned reload_flags = PIPE_CLEAR_COLOR0;
   uint8_t identity[4] = { PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y,
                           PIPE_SWIZZLE_Z, PIPE_SWIZZLE_W };

   if (lima_debug & LIMA_DEBUG_NO_BLIT)
      return false;

   /* Blitting of swizzled formats (R and RG) isn't implemented yet */
   if (memcmp(identity,
              lima_format_get_texel_swizzle(info->src.resource->format),
              sizeof(identity)))
      return false;

   if (memcmp(identity,
              lima_format_get_texel_swizzle(info->dst.resource->format),
              sizeof(identity)))
      return false;

   if (util_format_is_depth_or_stencil(info->src.resource->format)) {
      const struct util_format_description *desc =
         util_format_description(info->src.resource->format);
      reload_flags = 0;
      if (util_format_has_depth(desc))
         reload_flags |= PIPE_CLEAR_DEPTH;
      if (util_format_has_stencil(desc))
         reload_flags |= PIPE_CLEAR_STENCIL;
   }

   if (!lima_format_pixel_supported(info->dst.resource->format))
      return false;

   if (!lima_format_texel_supported(info->src.resource->format))
      return false;

   if (info->dst.resource->target != PIPE_TEXTURE_2D ||
       info->src.resource->target != PIPE_TEXTURE_2D)
      return false;

   if (info->dst.box.x < 0 || info->dst.box.y < 0 ||
       info->src.box.x < 0 || info->src.box.y < 0)
      return false;

   if (info->src.box.depth != 1 ||
       info->dst.box.depth != 1)
      return false;

   /* Scissored blit isn't implemented yet */
   if (info->scissor_enable)
      return false;

   if ((reload_flags & PIPE_CLEAR_COLOR) && !(info->mask & PIPE_MASK_RGBA))
      return false;

   if ((reload_flags & PIPE_CLEAR_DEPTH) && !(info->mask & PIPE_MASK_Z))
      return false;

   if ((reload_flags & PIPE_CLEAR_STENCIL) && !(info->mask & PIPE_MASK_S))
      return false;

   struct pipe_surface *dst_surf =
         lima_get_blit_surface(pctx, info->dst.resource, info->dst.level);
   struct lima_surface *lima_dst_surf = lima_surface(dst_surf);

   struct pipe_surface *src_surf =
         lima_get_blit_surface(pctx, info->src.resource, info->src.level);

   struct lima_job *job;

   if (util_format_is_depth_or_stencil(info->dst.resource->format))
      job = lima_job_get_with_fb(ctx, NULL, dst_surf);
   else
      job = lima_job_get_with_fb(ctx, dst_surf, NULL);

   struct lima_resource *src_res = lima_resource(src_surf->texture);
   struct lima_resource *dst_res = lima_resource(dst_surf->texture);

   lima_flush_job_accessing_bo(ctx, src_res->bo, true);
   lima_flush_job_accessing_bo(ctx, dst_res->bo, true);

   lima_job_add_bo(job, LIMA_PIPE_PP, src_res->bo, LIMA_SUBMIT_BO_READ);
   _mesa_hash_table_insert(ctx->write_jobs, &dst_res->base, job);
   lima_job_add_bo(job, LIMA_PIPE_PP, dst_res->bo, LIMA_SUBMIT_BO_WRITE);

   if (info->src.resource->nr_samples > 1) {
      for (int i = 0; i < MIN2(info->src.resource->nr_samples, LIMA_MAX_SAMPLES); i++) {
         lima_pack_blit_cmd(job, &job->plbu_cmd_array,
                            src_surf, &info->src.box,
                            &info->dst.box, info->filter, true,
                            1 << i, i);
      }
   } else {
      lima_pack_blit_cmd(job, &job->plbu_cmd_array,
                         src_surf, &info->src.box,
                         &info->dst.box, info->filter, true,
                         0xf, 0);
   }

   bool tile_aligned = false;

   if (info->dst.box.x == 0 && info->dst.box.y == 0 &&
       info->dst.box.width == lima_dst_surf->base.width &&
       info->dst.box.height == lima_dst_surf->base.height)
      tile_aligned = true;

   if (info->dst.box.x % 16 == 0 && info->dst.box.y % 16 == 0 &&
       info->dst.box.width % 16 == 0 && info->dst.box.height % 16 == 0)
      tile_aligned = true;

   /* Reload if dest is not aligned to tile boundaries */
   if (!tile_aligned)
      lima_dst_surf->reload = reload_flags;
   else
      lima_dst_surf->reload = 0;

   job->resolve = reload_flags;

   lima_do_job(job);

   pipe_surface_reference(&dst_surf, NULL);
   pipe_surface_reference(&src_surf, NULL);

   return true;
}
