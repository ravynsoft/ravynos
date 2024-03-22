/*
 * Copyright © 2017 Broadcom
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

#include "util/format/u_format.h"
#include "util/macros.h"
#include "v3d_context.h"
#include "broadcom/common/v3d_macros.h"
#include "broadcom/common/v3d_tiling.h"
#include "broadcom/common/v3d_util.h"
#include "broadcom/cle/v3dx_pack.h"

#define PIPE_CLEAR_COLOR_BUFFERS (PIPE_CLEAR_COLOR0 |                   \
                                  PIPE_CLEAR_COLOR1 |                   \
                                  PIPE_CLEAR_COLOR2 |                   \
                                  PIPE_CLEAR_COLOR3)                    \

#define PIPE_FIRST_COLOR_BUFFER_BIT (ffs(PIPE_CLEAR_COLOR0) - 1)

static void
load_general(struct v3d_cl *cl, struct pipe_surface *psurf, int buffer,
             int layer, uint32_t pipe_bit, uint32_t *loads_pending)
{
        struct v3d_surface *surf = v3d_surface(psurf);
        bool separate_stencil = surf->separate_stencil && buffer == STENCIL;
        if (separate_stencil) {
                psurf = surf->separate_stencil;
                surf = v3d_surface(psurf);
        }

        struct v3d_resource *rsc = v3d_resource(psurf->texture);

        uint32_t layer_offset =
                v3d_layer_offset(&rsc->base, psurf->u.tex.level,
                                 psurf->u.tex.first_layer + layer);
        cl_emit(cl, LOAD_TILE_BUFFER_GENERAL, load) {
                load.buffer_to_load = buffer;
                load.address = cl_address(rsc->bo, layer_offset);

                load.memory_format = surf->tiling;
                if (separate_stencil)
                        load.input_image_format = V3D_OUTPUT_IMAGE_FORMAT_S8;
                else
                        load.input_image_format = surf->format;
                load.r_b_swap = surf->swap_rb;
                load.force_alpha_1 = util_format_has_alpha1(psurf->format);
                if (surf->tiling == V3D_TILING_UIF_NO_XOR ||
                    surf->tiling == V3D_TILING_UIF_XOR) {
                        load.height_in_ub_or_stride =
                                surf->padded_height_of_output_image_in_uif_blocks;
                } else if (surf->tiling == V3D_TILING_RASTER) {
                        struct v3d_resource_slice *slice =
                                &rsc->slices[psurf->u.tex.level];
                        load.height_in_ub_or_stride = slice->stride;
                }

                if (psurf->texture->nr_samples > 1)
                        load.decimate_mode = V3D_DECIMATE_MODE_ALL_SAMPLES;
                else
                        load.decimate_mode = V3D_DECIMATE_MODE_SAMPLE_0;

        }

        *loads_pending &= ~pipe_bit;
}

static void
store_general(struct v3d_job *job,
              struct v3d_cl *cl, struct pipe_surface *psurf,
              int layer, int buffer, int pipe_bit,
              uint32_t *stores_pending, bool general_color_clear,
              bool resolve_4x)
{
        struct v3d_surface *surf = v3d_surface(psurf);
        bool separate_stencil = surf->separate_stencil && buffer == STENCIL;
        if (separate_stencil) {
                psurf = surf->separate_stencil;
                surf = v3d_surface(psurf);
        }

        *stores_pending &= ~pipe_bit;

        struct v3d_resource *rsc = v3d_resource(psurf->texture);

        rsc->writes++;

        uint32_t layer_offset =
                v3d_layer_offset(&rsc->base, psurf->u.tex.level,
                                 psurf->u.tex.first_layer + layer);
        cl_emit(cl, STORE_TILE_BUFFER_GENERAL, store) {
                store.buffer_to_store = buffer;
                store.address = cl_address(rsc->bo, layer_offset);

                store.clear_buffer_being_stored = false;

                if (separate_stencil)
                        store.output_image_format = V3D_OUTPUT_IMAGE_FORMAT_S8;
                else
                        store.output_image_format = surf->format;

                store.r_b_swap = surf->swap_rb;
                store.memory_format = surf->tiling;

                if (surf->tiling == V3D_TILING_UIF_NO_XOR ||
                    surf->tiling == V3D_TILING_UIF_XOR) {
                        store.height_in_ub_or_stride =
                                surf->padded_height_of_output_image_in_uif_blocks;
                } else if (surf->tiling == V3D_TILING_RASTER) {
                        struct v3d_resource_slice *slice =
                                &rsc->slices[psurf->u.tex.level];
                        store.height_in_ub_or_stride = slice->stride;
                }

                assert(!resolve_4x || job->bbuf);
                if (psurf->texture->nr_samples > 1)
                        store.decimate_mode = V3D_DECIMATE_MODE_ALL_SAMPLES;
                else if (resolve_4x && job->bbuf->texture->nr_samples > 1)
                        store.decimate_mode = V3D_DECIMATE_MODE_4X;
                else
                        store.decimate_mode = V3D_DECIMATE_MODE_SAMPLE_0;
        }
}

static int
zs_buffer_from_pipe_bits(int pipe_clear_bits)
{
        switch (pipe_clear_bits & PIPE_CLEAR_DEPTHSTENCIL) {
        case PIPE_CLEAR_DEPTHSTENCIL:
                return ZSTENCIL;
        case PIPE_CLEAR_DEPTH:
                return Z;
        case PIPE_CLEAR_STENCIL:
                return STENCIL;
        default:
                return NONE;
        }
}

static void
v3d_rcl_emit_loads(struct v3d_job *job, struct v3d_cl *cl, int layer)
{
        /* When blitting, no color or zs buffer is loaded; instead the blit
         * source buffer is loaded for the aspects that we are going to blit.
         */
        assert(!job->bbuf || job->load == 0);
        assert(!job->bbuf || job->nr_cbufs <= 1);

        uint32_t loads_pending = job->bbuf ? job->store : job->load;

        for (int i = 0; i < job->nr_cbufs; i++) {
                uint32_t bit = PIPE_CLEAR_COLOR0 << i;
                if (!(loads_pending & bit))
                        continue;

                struct pipe_surface *psurf = job->bbuf ? job->bbuf : job->cbufs[i];
                assert(!job->bbuf || i == 0);

                if (!psurf)
                        continue;

                load_general(cl, psurf, RENDER_TARGET_0 + i, layer,
                             bit, &loads_pending);
        }

        if (loads_pending & PIPE_CLEAR_DEPTHSTENCIL) {
                assert(!job->early_zs_clear);
                struct pipe_surface *src = job->bbuf ? job->bbuf : job->zsbuf;
                struct v3d_resource *rsc = v3d_resource(src->texture);

                if (rsc->separate_stencil &&
                    (loads_pending & PIPE_CLEAR_STENCIL)) {
                        load_general(cl, src,
                                     STENCIL, layer,
                                     PIPE_CLEAR_STENCIL,
                                     &loads_pending);
                }

                if (loads_pending & PIPE_CLEAR_DEPTHSTENCIL) {
                        load_general(cl, src,
                                     zs_buffer_from_pipe_bits(loads_pending),
                                     layer,
                                     loads_pending & PIPE_CLEAR_DEPTHSTENCIL,
                                     &loads_pending);
                }
        }

        assert(!loads_pending);
        cl_emit(cl, END_OF_LOADS, end);
}

static void
v3d_rcl_emit_stores(struct v3d_job *job, struct v3d_cl *cl, int layer)
{
        bool general_color_clear = false;
        uint32_t stores_pending = job->store;

        /* For V3D 4.1, use general stores for all TLB stores.
         *
         * For V3D 3.3, we only use general stores to do raw stores for any
         * MSAA surfaces.  These output UIF tiled images where each 4x MSAA
         * pixel is a 2x2 quad, and the format will be that of the
         * internal_type/internal_bpp, rather than the format from GL's
         * perspective.  Non-MSAA surfaces will use
         * STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_EXTENDED.
         */
        assert(!job->bbuf || job->nr_cbufs <= 1);
        for (int i = 0; i < job->nr_cbufs; i++) {
                uint32_t bit = PIPE_CLEAR_COLOR0 << i;
                if (!(job->store & bit))
                        continue;

                struct pipe_surface *psurf = job->cbufs[i];
                if (!psurf)
                        continue;

                store_general(job, cl, psurf, layer, RENDER_TARGET_0 + i, bit,
                              &stores_pending, general_color_clear, job->bbuf);
        }

        if (job->store & PIPE_CLEAR_DEPTHSTENCIL && job->zsbuf) {
                assert(!job->early_zs_clear);
                struct v3d_resource *rsc = v3d_resource(job->zsbuf->texture);
                if (rsc->separate_stencil) {
                        if (job->store & PIPE_CLEAR_DEPTH) {
                                store_general(job, cl, job->zsbuf, layer,
                                              Z, PIPE_CLEAR_DEPTH,
                                              &stores_pending,
                                              general_color_clear,
                                              false);
                        }

                        if (job->store & PIPE_CLEAR_STENCIL) {
                                store_general(job, cl, job->zsbuf, layer,
                                              STENCIL, PIPE_CLEAR_STENCIL,
                                              &stores_pending,
                                              general_color_clear,
                                              false);
                        }
                } else {
                        store_general(job, cl, job->zsbuf, layer,
                                      zs_buffer_from_pipe_bits(job->store),
                                      job->store & PIPE_CLEAR_DEPTHSTENCIL,
                                      &stores_pending, general_color_clear,
                                      false);
                }
        }


        /* If we're emitting an RCL with GL_ARB_framebuffer_no_attachments,
         * we still need to emit some sort of store.
         */
        if (!job->store) {
                cl_emit(cl, STORE_TILE_BUFFER_GENERAL, store) {
                        store.buffer_to_store = NONE;
                }
        }

        assert(!stores_pending);

        /* GFXH-1461/GFXH-1689: The per-buffer store command's clear
         * buffer bit is broken for depth/stencil.  In addition, the
         * clear packet's Z/S bit is broken, but the RTs bit ends up
         * clearing Z/S.
         */
        if (job->clear) {
#if V3D_VERSION == 42
                cl_emit(cl, CLEAR_TILE_BUFFERS, clear) {
                        clear.clear_z_stencil_buffer = !job->early_zs_clear;
                        clear.clear_all_render_targets = true;
                }
#endif
#if V3D_VERSION >= 71
                cl_emit(cl, CLEAR_RENDER_TARGETS, clear);
#endif

        }
}

static void
v3d_rcl_emit_generic_per_tile_list(struct v3d_job *job, int layer)
{
        /* Emit the generic list in our indirect state -- the rcl will just
         * have pointers into it.
         */
        struct v3d_cl *cl = &job->indirect;
        v3d_cl_ensure_space(cl, 200, 1);
        struct v3d_cl_reloc tile_list_start = cl_get_address(cl);

        /* V3D 4.x/7.x only requires a single tile coordinates, and
         * END_OF_LOADS switches us between loading and rendering.
         */
        cl_emit(cl, TILE_COORDINATES_IMPLICIT, coords);

        v3d_rcl_emit_loads(job, cl, layer);

        /* The binner starts out writing tiles assuming that the initial mode
         * is triangles, so make sure that's the case.
         */
        cl_emit(cl, PRIM_LIST_FORMAT, fmt) {
                fmt.primitive_type = LIST_TRIANGLES;
        }

        /* PTB assumes that value to be 0, but hw will not set it. */
        cl_emit(cl, SET_INSTANCEID, set) {
           set.instance_id = 0;
        }

        cl_emit(cl, BRANCH_TO_IMPLICIT_TILE_LIST, branch);

        v3d_rcl_emit_stores(job, cl, layer);

        cl_emit(cl, END_OF_TILE_MARKER, end);

        cl_emit(cl, RETURN_FROM_SUB_LIST, ret);

        cl_emit(&job->rcl, START_ADDRESS_OF_GENERIC_TILE_LIST, branch) {
                branch.start = tile_list_start;
                branch.end = cl_get_address(cl);
        }
}

/* Note that for v71, render target cfg packets has just one field that
 * combined the internal type and clamp mode. For simplicity we keep just one
 * helper.
 *
 * Note: rt_type is in fact a "enum V3DX(Internal_Type)".
 *
 */
static uint32_t
v3dX(clamp_for_format_and_type)(uint32_t rt_type,
                                enum pipe_format format)
{
#if V3D_VERSION == 42
        if (util_format_is_srgb(format)) {
                return V3D_RENDER_TARGET_CLAMP_NORM;
        } else if (util_format_is_pure_integer(format)) {
                return V3D_RENDER_TARGET_CLAMP_INT;
        } else {
                return V3D_RENDER_TARGET_CLAMP_NONE;
        }
#endif
#if V3D_VERSION >= 71
        switch (rt_type) {
        case V3D_INTERNAL_TYPE_8I:
                return V3D_RENDER_TARGET_TYPE_CLAMP_8I_CLAMPED;
        case V3D_INTERNAL_TYPE_8UI:
                return V3D_RENDER_TARGET_TYPE_CLAMP_8UI_CLAMPED;
        case V3D_INTERNAL_TYPE_8:
                return V3D_RENDER_TARGET_TYPE_CLAMP_8;
        case V3D_INTERNAL_TYPE_16I:
                return V3D_RENDER_TARGET_TYPE_CLAMP_16I_CLAMPED;
        case V3D_INTERNAL_TYPE_16UI:
                return V3D_RENDER_TARGET_TYPE_CLAMP_16UI_CLAMPED;
        case V3D_INTERNAL_TYPE_16F:
                return util_format_is_srgb(format) ?
                        V3D_RENDER_TARGET_TYPE_CLAMP_16F_CLAMP_NORM :
                        V3D_RENDER_TARGET_TYPE_CLAMP_16F;
        case V3D_INTERNAL_TYPE_32I:
                return V3D_RENDER_TARGET_TYPE_CLAMP_32I_CLAMPED;
        case V3D_INTERNAL_TYPE_32UI:
                return V3D_RENDER_TARGET_TYPE_CLAMP_32UI_CLAMPED;
        case V3D_INTERNAL_TYPE_32F:
                return V3D_RENDER_TARGET_TYPE_CLAMP_32F;
        default:
                unreachable("Unknown internal render target type");
        }
        return V3D_RENDER_TARGET_TYPE_CLAMP_INVALID;
#endif
        unreachable("Wrong V3D_VERSION");
}

#if V3D_VERSION >= 71
static void
v3d_setup_render_target(struct v3d_job *job,
                        int cbuf,
                        uint32_t *rt_bpp,
                        uint32_t *rt_type_clamp)
{
        if (!job->cbufs[cbuf])
                return;

        struct v3d_surface *surf = v3d_surface(job->cbufs[cbuf]);
        *rt_bpp = surf->internal_bpp;
        if (job->bbuf) {
           struct v3d_surface *bsurf = v3d_surface(job->bbuf);
           *rt_bpp = MAX2(*rt_bpp, bsurf->internal_bpp);
        }
        *rt_type_clamp = v3dX(clamp_for_format_and_type)(surf->internal_type,
                                                         surf->base.format);
}
#endif

#if V3D_VERSION == 42
static void
v3d_setup_render_target(struct v3d_job *job,
                        int cbuf,
                        uint32_t *rt_bpp,
                        uint32_t *rt_type,
                        uint32_t *rt_clamp)
{
        if (!job->cbufs[cbuf])
                return;

        struct v3d_surface *surf = v3d_surface(job->cbufs[cbuf]);
        *rt_bpp = surf->internal_bpp;
        if (job->bbuf) {
           struct v3d_surface *bsurf = v3d_surface(job->bbuf);
           *rt_bpp = MAX2(*rt_bpp, bsurf->internal_bpp);
        }
        *rt_type = surf->internal_type;
        *rt_clamp = v3dX(clamp_for_format_and_type)(surf->internal_type,
                                                    surf->base.format);
}
#endif

static bool
supertile_in_job_scissors(struct v3d_job *job,
                          uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
   if (job->scissor.disabled || job->scissor.count == 0)
      return true;

   const uint32_t min_x = x * w;
   const uint32_t min_y = y * h;
   const uint32_t max_x = min_x + w - 1;
   const uint32_t max_y = min_y + h - 1;

   for (uint32_t i = 0; i < job->scissor.count; i++) {
           const uint32_t min_s_x = job->scissor.rects[i].min_x;
           const uint32_t min_s_y = job->scissor.rects[i].min_y;
           const uint32_t max_s_x = job->scissor.rects[i].max_x;
           const uint32_t max_s_y = job->scissor.rects[i].max_y;

           if (max_x < min_s_x || min_x > max_s_x ||
               max_y < min_s_y || min_y > max_s_y) {
                   continue;
           }

           return true;
   }

   return false;
}

static inline bool
do_double_initial_tile_clear(const struct v3d_job *job)
{
        /* Our rendering code emits an initial clear per layer, unlike the
         * Vulkan driver, which only executes a single initial clear for all
         * layers. This is because in GL we don't use the
         * 'clear_buffer_being_stored' bit when storing tiles, so each layer
         * needs the iniital clear. This is also why this helper, unlike the
         * Vulkan version, doesn't check the layer count to decide if double
         * clear for double buffer mode is required.
         */
        return job->double_buffer &&
               (job->draw_tiles_x > 1 || job->draw_tiles_y > 1);
}

static void
emit_render_layer(struct v3d_job *job, uint32_t layer)
{
        uint32_t supertile_w = 1, supertile_h = 1;

        /* If doing multicore binning, we would need to initialize each
         * core's tile list here.
         */
        uint32_t tile_alloc_offset =
                layer * job->draw_tiles_x * job->draw_tiles_y * 64;
        cl_emit(&job->rcl, MULTICORE_RENDERING_TILE_LIST_SET_BASE, list) {
                list.address = cl_address(job->tile_alloc, tile_alloc_offset);
        }

        cl_emit(&job->rcl, MULTICORE_RENDERING_SUPERTILE_CFG, config) {
                uint32_t frame_w_in_supertiles, frame_h_in_supertiles;
                const uint32_t max_supertiles = 256;

                /* Size up our supertiles until we get under the limit. */
                for (;;) {
                        frame_w_in_supertiles = DIV_ROUND_UP(job->draw_tiles_x,
                                                             supertile_w);
                        frame_h_in_supertiles = DIV_ROUND_UP(job->draw_tiles_y,
                                                             supertile_h);
                        if (frame_w_in_supertiles *
                                frame_h_in_supertiles < max_supertiles) {
                                break;
                        }

                        if (supertile_w < supertile_h)
                                supertile_w++;
                        else
                                supertile_h++;
                }

                config.number_of_bin_tile_lists = 1;
                config.total_frame_width_in_tiles = job->draw_tiles_x;
                config.total_frame_height_in_tiles = job->draw_tiles_y;

                config.supertile_width_in_tiles = supertile_w;
                config.supertile_height_in_tiles = supertile_h;

                config.total_frame_width_in_supertiles = frame_w_in_supertiles;
                config.total_frame_height_in_supertiles = frame_h_in_supertiles;
        }

        /* Start by clearing the tile buffer. */
        cl_emit(&job->rcl, TILE_COORDINATES, coords) {
                coords.tile_column_number = 0;
                coords.tile_row_number = 0;
        }

        /* Emit an initial clear of the tile buffers.  This is necessary
         * for any buffers that should be cleared (since clearing
         * normally happens at the *end* of the generic tile list), but
         * it's also nice to clear everything so the first tile doesn't
         * inherit any contents from some previous frame.
         *
         * Also, implement the GFXH-1742 workaround.  There's a race in
         * the HW between the RCL updating the TLB's internal type/size
         * and thespawning of the QPU instances using the TLB's current
         * internal type/size.  To make sure the QPUs get the right
         * state, we need 1 dummy store in between internal type/size
         * changes on V3D 3.x, and 2 dummy stores on 4.x.
         */
        for (int i = 0; i < 2; i++) {
                if (i > 0)
                        cl_emit(&job->rcl, TILE_COORDINATES, coords);
                cl_emit(&job->rcl, END_OF_LOADS, end);
                cl_emit(&job->rcl, STORE_TILE_BUFFER_GENERAL, store) {
                        store.buffer_to_store = NONE;
                }

                if (i == 0 || do_double_initial_tile_clear(job)) {
#if V3D_VERSION < 71
                        cl_emit(&job->rcl, CLEAR_TILE_BUFFERS, clear) {
                                clear.clear_z_stencil_buffer = !job->early_zs_clear;
                                clear.clear_all_render_targets = true;
                        }
#else
                        cl_emit(&job->rcl, CLEAR_RENDER_TARGETS, clear);
#endif
                }
                cl_emit(&job->rcl, END_OF_TILE_MARKER, end);
        }
        cl_emit(&job->rcl, FLUSH_VCD_CACHE, flush);

        v3d_rcl_emit_generic_per_tile_list(job, layer);

        /* XXX perf: We should expose GL_MESA_tile_raster_order to
         * improve X11 performance, but we should use Morton order
         * otherwise to improve cache locality.
         */
        uint32_t supertile_w_in_pixels = job->tile_width * supertile_w;
        uint32_t supertile_h_in_pixels = job->tile_height * supertile_h;
        uint32_t min_x_supertile = job->draw_min_x / supertile_w_in_pixels;
        uint32_t min_y_supertile = job->draw_min_y / supertile_h_in_pixels;

        uint32_t max_x_supertile = 0;
        uint32_t max_y_supertile = 0;
        if (job->draw_max_x != 0 && job->draw_max_y != 0) {
                max_x_supertile = (job->draw_max_x - 1) / supertile_w_in_pixels;
                max_y_supertile = (job->draw_max_y - 1) / supertile_h_in_pixels;
        }

        for (int y = min_y_supertile; y <= max_y_supertile; y++) {
                for (int x = min_x_supertile; x <= max_x_supertile; x++) {
                        if (supertile_in_job_scissors(job, x, y,
                                                      supertile_w_in_pixels,
                                                      supertile_h_in_pixels)) {
                                cl_emit(&job->rcl, SUPERTILE_COORDINATES, coords) {
                                      coords.column_number_in_supertiles = x;
                                      coords.row_number_in_supertiles = y;
                                }
                        }
                }
        }
}

void
v3dX(emit_rcl)(struct v3d_job *job)
{
        /* The RCL list should be empty. */
        assert(!job->rcl.bo);

        v3d_cl_ensure_space_with_branch(&job->rcl, 200 +
                                        MAX2(job->num_layers, 1) * 256 *
                                        cl_packet_length(SUPERTILE_COORDINATES));
        job->submit.rcl_start = job->rcl.bo->offset;
        v3d_job_add_bo(job, job->rcl.bo);

        /* Common config must be the first TILE_RENDERING_MODE_CFG
         * and Z_STENCIL_CLEAR_VALUES must be last.  The ones in between are
         * optional updates to the previous HW state.
         */
        cl_emit(&job->rcl, TILE_RENDERING_MODE_CFG_COMMON, config) {
                if (job->zsbuf) {
                        struct v3d_surface *surf = v3d_surface(job->zsbuf);
                        config.internal_depth_type = surf->internal_type;
                }

                if (job->decided_global_ez_enable) {
                        switch (job->first_ez_state) {
                        case V3D_EZ_UNDECIDED:
                        case V3D_EZ_LT_LE:
                                config.early_z_disable = false;
                                config.early_z_test_and_update_direction =
                                        EARLY_Z_DIRECTION_LT_LE;
                                break;
                        case V3D_EZ_GT_GE:
                                config.early_z_disable = false;
                                config.early_z_test_and_update_direction =
                                        EARLY_Z_DIRECTION_GT_GE;
                                break;
                        case V3D_EZ_DISABLED:
                                config.early_z_disable = true;
                        }
                } else {
                        assert(job->draw_calls_queued == 0);
                        config.early_z_disable = true;
                }

                assert(job->zsbuf || config.early_z_disable);

                job->early_zs_clear = (job->clear & PIPE_CLEAR_DEPTHSTENCIL) &&
                        !(job->load & PIPE_CLEAR_DEPTHSTENCIL) &&
                        !(job->store & PIPE_CLEAR_DEPTHSTENCIL);

                config.early_depth_stencil_clear = job->early_zs_clear;

                config.image_width_pixels = job->draw_width;
                config.image_height_pixels = job->draw_height;

                config.number_of_render_targets = MAX2(job->nr_cbufs, 1);

                assert(!job->msaa || !job->double_buffer);
                config.multisample_mode_4x = job->msaa;
                config.double_buffer_in_non_ms_mode = job->double_buffer;

#if V3D_VERSION == 42
                config.maximum_bpp_of_all_render_targets = job->internal_bpp;
#endif
#if V3D_VERSION >= 71
                config.log2_tile_width = log2_tile_size(job->tile_width);
                config.log2_tile_height = log2_tile_size(job->tile_height);

                /* FIXME: ideallly we would like next assert on the packet header (as is
                 * general, so also applies to GL). We would need to expand
                 * gen_pack_header for that.
                 */
                assert(config.log2_tile_width == config.log2_tile_height ||
                       config.log2_tile_width == config.log2_tile_height + 1);
#endif

        }

#if V3D_VERSION >= 71
        uint32_t base_addr = 0;

        /* If we don't have any color RTs, we sill need to emit one and flag
         * it as not used using stride = 1
         */
        if (job->nr_cbufs == 0) {
           cl_emit(&job->rcl, TILE_RENDERING_MODE_CFG_RENDER_TARGET_PART1, rt) {
              rt.stride = 1; /* Unused */
           }
        }
#endif
        for (int i = 0; i < job->nr_cbufs; i++) {
                struct pipe_surface *psurf = job->cbufs[i];
                if (!psurf) {
#if V3D_VERSION >= 71
                        cl_emit(&job->rcl, TILE_RENDERING_MODE_CFG_RENDER_TARGET_PART1, rt) {
                                rt.render_target_number = i;
                                rt.stride = 1; /* Unused */
                        }
#endif
                        continue;
                }

                struct v3d_surface *surf = v3d_surface(psurf);
                struct v3d_resource *rsc = v3d_resource(psurf->texture);

                UNUSED uint32_t config_pad = 0;
                UNUSED uint32_t clear_pad = 0;

                /* XXX: Set the pad for raster. */
                if (surf->tiling == V3D_TILING_UIF_NO_XOR ||
                    surf->tiling == V3D_TILING_UIF_XOR) {
                        int uif_block_height = v3d_utile_height(rsc->cpp) * 2;
                        uint32_t implicit_padded_height = (align(job->draw_height, uif_block_height) /
                                                           uif_block_height);
                        if (surf->padded_height_of_output_image_in_uif_blocks -
                            implicit_padded_height < 15) {
                                config_pad = (surf->padded_height_of_output_image_in_uif_blocks -
                                              implicit_padded_height);
                        } else {
                                config_pad = 15;
                                clear_pad = surf->padded_height_of_output_image_in_uif_blocks;
                        }
                }

#if V3D_VERSION == 42
                cl_emit(&job->rcl, TILE_RENDERING_MODE_CFG_CLEAR_COLORS_PART1,
                        clear) {
                        clear.clear_color_low_32_bits = job->clear_color[i][0];
                        clear.clear_color_next_24_bits = job->clear_color[i][1] & 0xffffff;
                        clear.render_target_number = i;
                };

                if (surf->internal_bpp >= V3D_INTERNAL_BPP_64) {
                        cl_emit(&job->rcl, TILE_RENDERING_MODE_CFG_CLEAR_COLORS_PART2,
                                clear) {
                                clear.clear_color_mid_low_32_bits =
                                        ((job->clear_color[i][1] >> 24) |
                                         (job->clear_color[i][2] << 8));
                                clear.clear_color_mid_high_24_bits =
                                        ((job->clear_color[i][2] >> 24) |
                                         ((job->clear_color[i][3] & 0xffff) << 8));
                                clear.render_target_number = i;
                        };
                }

                if (surf->internal_bpp >= V3D_INTERNAL_BPP_128 || clear_pad) {
                        cl_emit(&job->rcl, TILE_RENDERING_MODE_CFG_CLEAR_COLORS_PART3,
                                clear) {
                                clear.uif_padded_height_in_uif_blocks = clear_pad;
                                clear.clear_color_high_16_bits = job->clear_color[i][3] >> 16;
                                clear.render_target_number = i;
                        };
                }
#endif
#if V3D_VERSION >= 71
                cl_emit(&job->rcl, TILE_RENDERING_MODE_CFG_RENDER_TARGET_PART1, rt) {
                        rt.clear_color_low_bits = job->clear_color[i][0];
                        v3d_setup_render_target(job, i, &rt.internal_bpp,
                                                &rt.internal_type_and_clamping);
                        rt.stride =
                                v3d_compute_rt_row_row_stride_128_bits(job->tile_width,
                                                                       v3d_internal_bpp_words(rt.internal_bpp));
                        rt.base_address = base_addr;
                        rt.render_target_number = i;

                        base_addr += (job->tile_height * rt.stride) / 8;
                }

                if (surf->internal_bpp >= V3D_INTERNAL_BPP_64) {
                        cl_emit(&job->rcl, TILE_RENDERING_MODE_CFG_RENDER_TARGET_PART2, rt) {
                                rt.clear_color_mid_bits = /* 40 bits (32 + 8)  */
                                        ((uint64_t) job->clear_color[i][1]) |
                                        (((uint64_t) (job->clear_color[i][2] & 0xff)) << 32);
                                rt.render_target_number = i;
                        }
                }

                if (surf->internal_bpp >= V3D_INTERNAL_BPP_128) {
                        cl_emit(&job->rcl, TILE_RENDERING_MODE_CFG_RENDER_TARGET_PART3, rt) {
                                rt.clear_color_top_bits = /* 56 bits (24 + 32) */
                                        (((uint64_t) (job->clear_color[i][2] & 0xffffff00)) >> 8) |
                                        (((uint64_t) (job->clear_color[i][3])) << 24);
                                rt.render_target_number = i;
                        }
                }
#endif
        }

#if V3D_VERSION == 42
        cl_emit(&job->rcl, TILE_RENDERING_MODE_CFG_COLOR, rt) {
                v3d_setup_render_target(job, 0,
                                        &rt.render_target_0_internal_bpp,
                                        &rt.render_target_0_internal_type,
                                        &rt.render_target_0_clamp);
                v3d_setup_render_target(job, 1,
                                        &rt.render_target_1_internal_bpp,
                                        &rt.render_target_1_internal_type,
                                        &rt.render_target_1_clamp);
                v3d_setup_render_target(job, 2,
                                        &rt.render_target_2_internal_bpp,
                                        &rt.render_target_2_internal_type,
                                        &rt.render_target_2_clamp);
                v3d_setup_render_target(job, 3,
                                        &rt.render_target_3_internal_bpp,
                                        &rt.render_target_3_internal_type,
                                        &rt.render_target_3_clamp);
        }
#endif

        /* Ends rendering mode config. */
        cl_emit(&job->rcl, TILE_RENDERING_MODE_CFG_ZS_CLEAR_VALUES,
                clear) {
                clear.z_clear_value = job->clear_z;
                clear.stencil_clear_value = job->clear_s;
        };

        /* Always set initial block size before the first branch, which needs
         * to match the value from binning mode config.
         */
        cl_emit(&job->rcl, TILE_LIST_INITIAL_BLOCK_SIZE, init) {
                init.use_auto_chained_tile_lists = true;
                init.size_of_first_block_in_chained_tile_lists =
                        TILE_ALLOCATION_BLOCK_SIZE_64B;
        }

        /* ARB_framebuffer_no_attachments allows rendering to happen even when
         * the framebuffer has no attachments, the idea being that fragment
         * shaders can still do image load/store, ssbo, etc without having to
         * write to actual attachments, so always run at least one iteration
         * of the loop.
         */
        assert(job->num_layers > 0 || (job->load == 0 && job->store == 0));
        for (int layer = 0; layer < MAX2(1, job->num_layers); layer++)
                emit_render_layer(job, layer);

        cl_emit(&job->rcl, END_OF_RENDERING, end);
}
