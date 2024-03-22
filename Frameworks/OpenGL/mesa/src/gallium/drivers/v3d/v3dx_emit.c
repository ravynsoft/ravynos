/*
 * Copyright © 2014-2017 Broadcom
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
#include "util/half_float.h"
#include "v3d_context.h"
#include "broadcom/common/v3d_macros.h"
#include "broadcom/cle/v3dx_pack.h"
#include "broadcom/common/v3d_util.h"
#include "broadcom/compiler/v3d_compiler.h"

static uint8_t
v3d_factor(enum pipe_blendfactor factor, bool dst_alpha_one)
{
        /* We may get a bad blendfactor when blending is disabled. */
        if (factor == 0)
                return V3D_BLEND_FACTOR_ZERO;

        switch (factor) {
        case PIPE_BLENDFACTOR_ZERO:
                return V3D_BLEND_FACTOR_ZERO;
        case PIPE_BLENDFACTOR_ONE:
                return V3D_BLEND_FACTOR_ONE;
        case PIPE_BLENDFACTOR_SRC_COLOR:
                return V3D_BLEND_FACTOR_SRC_COLOR;
        case PIPE_BLENDFACTOR_INV_SRC_COLOR:
                return V3D_BLEND_FACTOR_INV_SRC_COLOR;
        case PIPE_BLENDFACTOR_DST_COLOR:
                return V3D_BLEND_FACTOR_DST_COLOR;
        case PIPE_BLENDFACTOR_INV_DST_COLOR:
                return V3D_BLEND_FACTOR_INV_DST_COLOR;
        case PIPE_BLENDFACTOR_SRC_ALPHA:
                return V3D_BLEND_FACTOR_SRC_ALPHA;
        case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
                return V3D_BLEND_FACTOR_INV_SRC_ALPHA;
        case PIPE_BLENDFACTOR_DST_ALPHA:
                return (dst_alpha_one ?
                        V3D_BLEND_FACTOR_ONE :
                        V3D_BLEND_FACTOR_DST_ALPHA);
        case PIPE_BLENDFACTOR_INV_DST_ALPHA:
                return (dst_alpha_one ?
                        V3D_BLEND_FACTOR_ZERO :
                        V3D_BLEND_FACTOR_INV_DST_ALPHA);
        case PIPE_BLENDFACTOR_CONST_COLOR:
                return V3D_BLEND_FACTOR_CONST_COLOR;
        case PIPE_BLENDFACTOR_INV_CONST_COLOR:
                return V3D_BLEND_FACTOR_INV_CONST_COLOR;
        case PIPE_BLENDFACTOR_CONST_ALPHA:
                return V3D_BLEND_FACTOR_CONST_ALPHA;
        case PIPE_BLENDFACTOR_INV_CONST_ALPHA:
                return V3D_BLEND_FACTOR_INV_CONST_ALPHA;
        case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
                return (dst_alpha_one ?
                        V3D_BLEND_FACTOR_ZERO :
                        V3D_BLEND_FACTOR_SRC_ALPHA_SATURATE);
        default:
                unreachable("Bad blend factor");
        }
}

static uint32_t
translate_colormask(struct v3d_context *v3d, uint32_t colormask, int rt)
{
        if (v3d->swap_color_rb & (1 << rt)) {
                colormask = ((colormask & (2 | 8)) |
                             ((colormask & 1) << 2) |
                             ((colormask & 4) >> 2));
        }

        return (~colormask) & 0xf;
}

static void
emit_rt_blend(struct v3d_context *v3d, struct v3d_job *job,
              struct pipe_blend_state *blend, int rt, uint8_t rt_mask,
              bool blend_dst_alpha_one)
{
        struct pipe_rt_blend_state *rtblend = &blend->rt[rt];

        /* We don't need to emit blend state for disabled RTs. */
        if (!rtblend->blend_enable)
                return;

        cl_emit(&job->bcl, BLEND_CFG, config) {
                config.render_target_mask = rt_mask;

                config.color_blend_mode = rtblend->rgb_func;
                config.color_blend_dst_factor =
                        v3d_factor(rtblend->rgb_dst_factor,
                                   blend_dst_alpha_one);
                config.color_blend_src_factor =
                        v3d_factor(rtblend->rgb_src_factor,
                                   blend_dst_alpha_one);

                config.alpha_blend_mode = rtblend->alpha_func;
                config.alpha_blend_dst_factor =
                        v3d_factor(rtblend->alpha_dst_factor,
                                   blend_dst_alpha_one);
                config.alpha_blend_src_factor =
                        v3d_factor(rtblend->alpha_src_factor,
                                   blend_dst_alpha_one);
        }
}

static void
emit_flat_shade_flags(struct v3d_job *job,
                      int varying_offset,
                      uint32_t varyings,
                      enum V3DX(Varying_Flags_Action) lower,
                      enum V3DX(Varying_Flags_Action) higher)
{
        cl_emit(&job->bcl, FLAT_SHADE_FLAGS, flags) {
                flags.varying_offset_v0 = varying_offset;
                flags.flat_shade_flags_for_varyings_v024 = varyings;
                flags.action_for_flat_shade_flags_of_lower_numbered_varyings =
                        lower;
                flags.action_for_flat_shade_flags_of_higher_numbered_varyings =
                        higher;
        }
}

static void
emit_noperspective_flags(struct v3d_job *job,
                         int varying_offset,
                         uint32_t varyings,
                         enum V3DX(Varying_Flags_Action) lower,
                         enum V3DX(Varying_Flags_Action) higher)
{
        cl_emit(&job->bcl, NON_PERSPECTIVE_FLAGS, flags) {
                flags.varying_offset_v0 = varying_offset;
                flags.non_perspective_flags_for_varyings_v024 = varyings;
                flags.action_for_non_perspective_flags_of_lower_numbered_varyings =
                        lower;
                flags.action_for_non_perspective_flags_of_higher_numbered_varyings =
                        higher;
        }
}

static void
emit_centroid_flags(struct v3d_job *job,
                    int varying_offset,
                    uint32_t varyings,
                    enum V3DX(Varying_Flags_Action) lower,
                    enum V3DX(Varying_Flags_Action) higher)
{
        cl_emit(&job->bcl, CENTROID_FLAGS, flags) {
                flags.varying_offset_v0 = varying_offset;
                flags.centroid_flags_for_varyings_v024 = varyings;
                flags.action_for_centroid_flags_of_lower_numbered_varyings =
                        lower;
                flags.action_for_centroid_flags_of_higher_numbered_varyings =
                        higher;
        }
}

static bool
emit_varying_flags(struct v3d_job *job, uint32_t *flags,
                   void (*flag_emit_callback)(struct v3d_job *job,
                                              int varying_offset,
                                              uint32_t flags,
                                              enum V3DX(Varying_Flags_Action) lower,
                                              enum V3DX(Varying_Flags_Action) higher))
{
        struct v3d_context *v3d = job->v3d;
        bool emitted_any = false;

        for (int i = 0; i < ARRAY_SIZE(v3d->prog.fs->prog_data.fs->flat_shade_flags); i++) {
                if (!flags[i])
                        continue;

                if (emitted_any) {
                        flag_emit_callback(job, i, flags[i],
                                           V3D_VARYING_FLAGS_ACTION_UNCHANGED,
                                           V3D_VARYING_FLAGS_ACTION_UNCHANGED);
                } else if (i == 0) {
                        flag_emit_callback(job, i, flags[i],
                                           V3D_VARYING_FLAGS_ACTION_UNCHANGED,
                                           V3D_VARYING_FLAGS_ACTION_ZEROED);
                } else {
                        flag_emit_callback(job, i, flags[i],
                                           V3D_VARYING_FLAGS_ACTION_ZEROED,
                                           V3D_VARYING_FLAGS_ACTION_ZEROED);
                }
                emitted_any = true;
        }

        return emitted_any;
}

static inline struct v3d_uncompiled_shader *
get_tf_shader(struct v3d_context *v3d)
{
        if (v3d->prog.bind_gs)
                return v3d->prog.bind_gs;
        else
                return v3d->prog.bind_vs;
}

void
v3dX(emit_state)(struct pipe_context *pctx)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_job *job = v3d->job;
        bool rasterizer_discard = v3d->rasterizer->base.rasterizer_discard;

        if (v3d->dirty & (V3D_DIRTY_SCISSOR | V3D_DIRTY_VIEWPORT |
                          V3D_DIRTY_RASTERIZER)) {
                float *vpscale = v3d->viewport.scale;
                float *vptranslate = v3d->viewport.translate;
                float vp_minx = -fabsf(vpscale[0]) + vptranslate[0];
                float vp_maxx = fabsf(vpscale[0]) + vptranslate[0];
                float vp_miny = -fabsf(vpscale[1]) + vptranslate[1];
                float vp_maxy = fabsf(vpscale[1]) + vptranslate[1];

                /* Clip to the scissor if it's enabled, but still clip to the
                 * drawable regardless since that controls where the binner
                 * tries to put things.
                 *
                 * Additionally, always clip the rendering to the viewport,
                 * since the hardware does guardband clipping, meaning
                 * primitives would rasterize outside of the view volume.
                 */
                uint32_t minx, miny, maxx, maxy;
                if (!v3d->rasterizer->base.scissor) {
                        minx = MAX2(vp_minx, 0);
                        miny = MAX2(vp_miny, 0);
                        maxx = MIN2(vp_maxx, job->draw_width);
                        maxy = MIN2(vp_maxy, job->draw_height);
                } else {
                        minx = MAX2(vp_minx, v3d->scissor.minx);
                        miny = MAX2(vp_miny, v3d->scissor.miny);
                        maxx = MIN2(vp_maxx, v3d->scissor.maxx);
                        maxy = MIN2(vp_maxy, v3d->scissor.maxy);
                }

                cl_emit(&job->bcl, CLIP_WINDOW, clip) {
                        clip.clip_window_left_pixel_coordinate = minx;
                        clip.clip_window_bottom_pixel_coordinate = miny;
                        if (maxx > minx && maxy > miny) {
                                clip.clip_window_width_in_pixels = maxx - minx;
                                clip.clip_window_height_in_pixels = maxy - miny;
                        }
                }

                job->draw_min_x = MIN2(job->draw_min_x, minx);
                job->draw_min_y = MIN2(job->draw_min_y, miny);
                job->draw_max_x = MAX2(job->draw_max_x, maxx);
                job->draw_max_y = MAX2(job->draw_max_y, maxy);

                if (!v3d->rasterizer->base.scissor) {
                    job->scissor.disabled = true;
                } else if (!job->scissor.disabled &&
                           (v3d->dirty & V3D_DIRTY_SCISSOR)) {
                        if (job->scissor.count < MAX_JOB_SCISSORS) {
                                job->scissor.rects[job->scissor.count].min_x =
                                        v3d->scissor.minx;
                                job->scissor.rects[job->scissor.count].min_y =
                                        v3d->scissor.miny;
                                job->scissor.rects[job->scissor.count].max_x =
                                        v3d->scissor.maxx - 1;
                                job->scissor.rects[job->scissor.count].max_y =
                                        v3d->scissor.maxy - 1;
                                job->scissor.count++;
                        } else {
                                job->scissor.disabled = true;
                                perf_debug("Too many scissor rects.");
                        }
                }
        }

        if (v3d->dirty & (V3D_DIRTY_RASTERIZER |
                          V3D_DIRTY_ZSA |
                          V3D_DIRTY_BLEND |
                          V3D_DIRTY_COMPILED_FS)) {
                cl_emit(&job->bcl, CFG_BITS, config) {
                        config.enable_forward_facing_primitive =
                                !rasterizer_discard &&
                                !(v3d->rasterizer->base.cull_face &
                                  PIPE_FACE_FRONT);
                        config.enable_reverse_facing_primitive =
                                !rasterizer_discard &&
                                !(v3d->rasterizer->base.cull_face &
                                  PIPE_FACE_BACK);
                        /* This seems backwards, but it's what gets the
                         * clipflat test to pass.
                         */
                        config.clockwise_primitives =
                                v3d->rasterizer->base.front_ccw;

                        config.enable_depth_offset =
                                v3d->rasterizer->base.offset_tri;

                        /* V3D follows GL behavior where the sample mask only
                         * applies when MSAA is enabled.  Gallium has sample
                         * mask apply anyway, and the MSAA blit shaders will
                         * set sample mask without explicitly setting
                         * rasterizer oversample.  Just force it on here,
                         * since the blit shaders are the only way to have
                         * !multisample && samplemask != 0xf.
                         */
                        config.rasterizer_oversample_mode =
                                v3d->rasterizer->base.multisample ||
                                v3d->sample_mask != 0xf;

                        config.direct3d_provoking_vertex =
                                v3d->rasterizer->base.flatshade_first;

                        config.blend_enable = v3d->blend->blend_enables;

                        /* Note: EZ state may update based on the compiled FS,
                         * along with ZSA
                         */
#if V3D_VERSION == 42
                        config.early_z_updates_enable =
                                (job->ez_state != V3D_EZ_DISABLED);
#endif
                        if (v3d->zsa->base.depth_enabled) {
                                config.z_updates_enable =
                                        v3d->zsa->base.depth_writemask;
#if V3D_VERSION == 42
                                config.early_z_enable =
                                        config.early_z_updates_enable;
#endif
                                config.depth_test_function =
                                        v3d->zsa->base.depth_func;
                        } else {
                                config.depth_test_function = PIPE_FUNC_ALWAYS;
                        }

                        config.stencil_enable =
                                v3d->zsa->base.stencil[0].enabled;

                        /* Use nicer line caps when line smoothing is
                         * enabled
                         */
                        config.line_rasterization =
                                v3d_line_smoothing_enabled(v3d) ?
                                V3D_LINE_RASTERIZATION_PERP_END_CAPS :
                                V3D_LINE_RASTERIZATION_DIAMOND_EXIT;

#if V3D_VERSION >= 71
                        /* The following follows the logic implemented in v3dv
                         * plus the definition of depth_clip_near/far and
                         * depth_clamp.
                         *
                         * Note: some extensions are not supported by v3d
                         * (like ARB_depth_clamp) that would affect this, but
                         * the values on rasterizer are taking that into
                         * account.
                         */
                        config.z_clipping_mode = v3d->rasterizer->base.depth_clip_near ||
                           v3d->rasterizer->base.depth_clip_far ?
                           V3D_Z_CLIP_MODE_MIN_ONE_TO_ONE : V3D_Z_CLIP_MODE_NONE;
#endif
                }
        }

        if (v3d->dirty & V3D_DIRTY_RASTERIZER &&
            v3d->rasterizer->base.offset_tri) {
                if (v3d->screen->devinfo.ver == 42 &&
                    job->zsbuf &&
                    job->zsbuf->format == PIPE_FORMAT_Z16_UNORM) {
                        cl_emit_prepacked_sized(&job->bcl,
                                                v3d->rasterizer->depth_offset_z16,
                                                cl_packet_length(DEPTH_OFFSET));
                } else {
                        cl_emit_prepacked_sized(&job->bcl,
                                                v3d->rasterizer->depth_offset,
                                                cl_packet_length(DEPTH_OFFSET));
                }
        }

        if (v3d->dirty & V3D_DIRTY_RASTERIZER) {
                cl_emit(&job->bcl, POINT_SIZE, point_size) {
                        point_size.point_size = v3d->rasterizer->point_size;
                }

                cl_emit(&job->bcl, LINE_WIDTH, line_width) {
                        line_width.line_width = v3d_get_real_line_width(v3d);
                }
        }

        if (v3d->dirty & V3D_DIRTY_VIEWPORT) {
#if V3D_VERSION == 42
                cl_emit(&job->bcl, CLIPPER_XY_SCALING, clip) {
                        clip.viewport_half_width_in_1_256th_of_pixel =
                                v3d->viewport.scale[0] * 256.0f;
                        clip.viewport_half_height_in_1_256th_of_pixel =
                                v3d->viewport.scale[1] * 256.0f;
                }
#endif
#if V3D_VERSION >= 71
                cl_emit(&job->bcl, CLIPPER_XY_SCALING, clip) {
                        clip.viewport_half_width_in_1_64th_of_pixel =
                                v3d->viewport.scale[0] * 64.0f;
                        clip.viewport_half_height_in_1_64th_of_pixel =
                                v3d->viewport.scale[1] * 64.0f;
                }
#endif


                cl_emit(&job->bcl, CLIPPER_Z_SCALE_AND_OFFSET, clip) {
                        clip.viewport_z_offset_zc_to_zs =
                                v3d->viewport.translate[2];
                        clip.viewport_z_scale_zc_to_zs =
                                v3d->viewport.scale[2];
                }
                cl_emit(&job->bcl, CLIPPER_Z_MIN_MAX_CLIPPING_PLANES, clip) {
                        float z1 = (v3d->viewport.translate[2] -
                                    v3d->viewport.scale[2]);
                        float z2 = (v3d->viewport.translate[2] +
                                    v3d->viewport.scale[2]);
                        clip.minimum_zw = MIN2(z1, z2);
                        clip.maximum_zw = MAX2(z1, z2);
                }

                cl_emit(&job->bcl, VIEWPORT_OFFSET, vp) {
                        float vp_fine_x = v3d->viewport.translate[0];
                        float vp_fine_y = v3d->viewport.translate[1];
                        int32_t vp_coarse_x = 0;
                        int32_t vp_coarse_y = 0;

                        /* The fine coordinates must be unsigned, but coarse
                         * can be signed.
                         */
                        if (unlikely(vp_fine_x < 0)) {
                                int32_t blocks_64 =
                                        DIV_ROUND_UP(fabsf(vp_fine_x), 64);
                                vp_fine_x += 64.0f * blocks_64;
                                vp_coarse_x -= blocks_64;
                        }

                        if (unlikely(vp_fine_y < 0)) {
                                int32_t blocks_64 =
                                        DIV_ROUND_UP(fabsf(vp_fine_y), 64);
                                vp_fine_y += 64.0f * blocks_64;
                                vp_coarse_y -= blocks_64;
                        }

                        vp.fine_x = vp_fine_x;
                        vp.fine_y = vp_fine_y;
                        vp.coarse_x = vp_coarse_x;
                        vp.coarse_y = vp_coarse_y;
                }
        }

        if (v3d->dirty & V3D_DIRTY_BLEND) {
                struct v3d_blend_state *blend = v3d->blend;

                if (blend->blend_enables) {
                        cl_emit(&job->bcl, BLEND_ENABLES, enables) {
                                enables.mask = blend->blend_enables;
                        }

                        const uint32_t max_rts =
                                V3D_MAX_RENDER_TARGETS(v3d->screen->devinfo.ver);
                        if (blend->base.independent_blend_enable) {
                                for (int i = 0; i < max_rts; i++)
                                        emit_rt_blend(v3d, job, &blend->base, i,
                                                      (1 << i),
                                                      v3d->blend_dst_alpha_one & (1 << i));
                        } else if (v3d->blend_dst_alpha_one &&
                                   util_bitcount(v3d->blend_dst_alpha_one) < job->nr_cbufs) {
                                /* Even if we don't have independent per-RT
                                 * blending, we may have a combination of RT
                                 * formats were some RTs have an alpha channel
                                 * and others don't. Since this affects how
                                 * blending is performed, we also need to emit
                                 * independent blend configurations in this
                                 * case: one for RTs with alpha and one for
                                 * RTs without.
                                 */
                                emit_rt_blend(v3d, job, &blend->base, 0,
                                              ((1 << max_rts) - 1) &
                                                   v3d->blend_dst_alpha_one,
                                              true);
                                emit_rt_blend(v3d, job, &blend->base, 0,
                                              ((1 << max_rts) - 1) &
                                                   ~v3d->blend_dst_alpha_one,
                                              false);
                        } else {
                                emit_rt_blend(v3d, job, &blend->base, 0,
                                              (1 << max_rts) - 1,
                                              v3d->blend_dst_alpha_one);
                        }
                }
        }

        if (v3d->dirty & V3D_DIRTY_BLEND) {
                struct pipe_blend_state *blend = &v3d->blend->base;

                const uint32_t max_rts =
                        V3D_MAX_RENDER_TARGETS(v3d->screen->devinfo.ver);
                cl_emit(&job->bcl, COLOR_WRITE_MASKS, mask) {
                        for (int i = 0; i < max_rts; i++) {
                                int rt = blend->independent_blend_enable ? i : 0;
                                int rt_mask = blend->rt[rt].colormask;

                                mask.mask |= translate_colormask(v3d, rt_mask,
                                                                 i) << (4 * i);
                        }
                }
        }

        /* GFXH-1431: On V3D 3.x, writing BLEND_CONFIG resets the constant
         * color.
         */
        if (v3d->dirty & V3D_DIRTY_BLEND_COLOR) {
                cl_emit(&job->bcl, BLEND_CONSTANT_COLOR, color) {
                        color.red_f16 = (v3d->swap_color_rb ?
                                          v3d->blend_color.hf[2] :
                                          v3d->blend_color.hf[0]);
                        color.green_f16 = v3d->blend_color.hf[1];
                        color.blue_f16 = (v3d->swap_color_rb ?
                                           v3d->blend_color.hf[0] :
                                           v3d->blend_color.hf[2]);
                        color.alpha_f16 = v3d->blend_color.hf[3];
                }
        }

        if (v3d->dirty & (V3D_DIRTY_ZSA | V3D_DIRTY_STENCIL_REF)) {
                struct pipe_stencil_state *front = &v3d->zsa->base.stencil[0];
                struct pipe_stencil_state *back = &v3d->zsa->base.stencil[1];

                if (front->enabled) {
                        cl_emit_with_prepacked(&job->bcl, STENCIL_CFG,
                                               v3d->zsa->stencil_front, config) {
                                config.stencil_ref_value =
                                        v3d->stencil_ref.ref_value[0];
                        }
                }

                if (back->enabled) {
                        cl_emit_with_prepacked(&job->bcl, STENCIL_CFG,
                                               v3d->zsa->stencil_back, config) {
                                config.stencil_ref_value =
                                        v3d->stencil_ref.ref_value[1];
                        }
                }
        }

        if (v3d->dirty & V3D_DIRTY_FLAT_SHADE_FLAGS) {
                if (!emit_varying_flags(job,
                                        v3d->prog.fs->prog_data.fs->flat_shade_flags,
                                        emit_flat_shade_flags)) {
                        cl_emit(&job->bcl, ZERO_ALL_FLAT_SHADE_FLAGS, flags);
                }
        }

        if (v3d->dirty & V3D_DIRTY_NOPERSPECTIVE_FLAGS) {
                if (!emit_varying_flags(job,
                                        v3d->prog.fs->prog_data.fs->noperspective_flags,
                                        emit_noperspective_flags)) {
                        cl_emit(&job->bcl, ZERO_ALL_NON_PERSPECTIVE_FLAGS, flags);
                }
        }

        if (v3d->dirty & V3D_DIRTY_CENTROID_FLAGS) {
                if (!emit_varying_flags(job,
                                        v3d->prog.fs->prog_data.fs->centroid_flags,
                                        emit_centroid_flags)) {
                        cl_emit(&job->bcl, ZERO_ALL_CENTROID_FLAGS, flags);
                }
        }

        /* Set up the transform feedback data specs (which VPM entries to
         * output to which buffers).
         */
        if (v3d->dirty & (V3D_DIRTY_STREAMOUT |
                          V3D_DIRTY_RASTERIZER |
                          V3D_DIRTY_PRIM_MODE)) {
                struct v3d_streamout_stateobj *so = &v3d->streamout;
                if (so->num_targets) {
                        bool psiz_per_vertex = (v3d->prim_mode == MESA_PRIM_POINTS &&
                                                v3d->rasterizer->base.point_size_per_vertex);
                        struct v3d_uncompiled_shader *tf_shader =
                                get_tf_shader(v3d);
                        uint16_t *tf_specs = (psiz_per_vertex ?
                                              tf_shader->tf_specs_psiz :
                                              tf_shader->tf_specs);

                        bool tf_enabled = v3d_transform_feedback_enabled(v3d);
                        job->tf_enabled |= tf_enabled;

                        cl_emit(&job->bcl, TRANSFORM_FEEDBACK_SPECS, tfe) {
                                tfe.number_of_16_bit_output_data_specs_following =
                                        tf_shader->num_tf_specs;
                                tfe.enable = tf_enabled;
                        };
                        for (int i = 0; i < tf_shader->num_tf_specs; i++) {
                                cl_emit_prepacked(&job->bcl, &tf_specs[i]);
                        }
                } else {
                        cl_emit(&job->bcl, TRANSFORM_FEEDBACK_SPECS, tfe) {
                                tfe.enable = false;
                        };
                }
        }

        /* Set up the transform feedback buffers. */
        if (v3d->dirty & V3D_DIRTY_STREAMOUT) {
                struct v3d_uncompiled_shader *tf_shader = get_tf_shader(v3d);
                struct v3d_streamout_stateobj *so = &v3d->streamout;
                for (int i = 0; i < so->num_targets; i++) {
                        struct pipe_stream_output_target *target =
                                so->targets[i];
                        struct v3d_resource *rsc = target ?
                                v3d_resource(target->buffer) : NULL;
                        struct pipe_shader_state *ss = &tf_shader->base;
                        struct pipe_stream_output_info *info = &ss->stream_output;
                        uint32_t offset = target ?
                                v3d_stream_output_target(target)->offset * info->stride[i] * 4 : 0;

                        if (!target)
                                continue;

                        cl_emit(&job->bcl, TRANSFORM_FEEDBACK_BUFFER, output) {
                                output.buffer_address =
                                        cl_address(rsc->bo,
                                                   target->buffer_offset +
                                                   offset);
                                output.buffer_size_in_32_bit_words =
                                        (target->buffer_size - offset) >> 2;
                                output.buffer_number = i;
                        }
                        if (target) {
                                v3d_job_add_tf_write_resource(v3d->job,
                                                              target->buffer);
                        }
                        /* XXX: buffer_size? */
                }
        }

        if (v3d->dirty & V3D_DIRTY_OQ) {
                cl_emit(&job->bcl, OCCLUSION_QUERY_COUNTER, counter) {
                        if (v3d->active_queries && v3d->current_oq) {
                                counter.address = cl_address(v3d->current_oq, 0);
                        }
                }
        }

        if (v3d->dirty & V3D_DIRTY_SAMPLE_STATE) {
                cl_emit(&job->bcl, SAMPLE_STATE, state) {
                        /* Note: SampleCoverage was handled at the
                         * frontend level by converting to sample_mask.
                         */
                        state.coverage = 1.0;
                        state.mask = job->msaa ? v3d->sample_mask : 0xf;
                }
        }
}
