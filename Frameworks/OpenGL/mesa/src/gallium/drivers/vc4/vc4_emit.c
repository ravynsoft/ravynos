/*
 * Copyright © 2014 Broadcom
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

#include "vc4_context.h"

void
vc4_emit_state(struct pipe_context *pctx)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct vc4_job *job = vc4->job;

        if (vc4->dirty & (VC4_DIRTY_SCISSOR | VC4_DIRTY_VIEWPORT |
                          VC4_DIRTY_RASTERIZER)) {
                float *vpscale = vc4->viewport.scale;
                float *vptranslate = vc4->viewport.translate;
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
                if (!vc4->rasterizer->base.scissor) {
                        minx = MAX2(vp_minx, 0);
                        miny = MAX2(vp_miny, 0);
                        maxx = MAX2(MIN2(vp_maxx, job->draw_width), minx);
                        maxy = MAX2(MIN2(vp_maxy, job->draw_height), miny);
                } else {
                        minx = MAX2(vp_minx, vc4->scissor.minx);
                        miny = MAX2(vp_miny, vc4->scissor.miny);
                        maxx = MAX2(MIN2(vp_maxx, vc4->scissor.maxx), minx);
                        maxy = MAX2(MIN2(vp_maxy, vc4->scissor.maxy), miny);
                }

                cl_emit(&job->bcl, CLIP_WINDOW, clip) {
                        clip.clip_window_left_pixel_coordinate = minx;
                        clip.clip_window_bottom_pixel_coordinate = miny;
                        clip.clip_window_height_in_pixels = maxy - miny;
                        clip.clip_window_width_in_pixels = maxx - minx;
                }

                job->draw_min_x = MIN2(job->draw_min_x, minx);
                job->draw_min_y = MIN2(job->draw_min_y, miny);
                job->draw_max_x = MAX2(job->draw_max_x, maxx);
                job->draw_max_y = MAX2(job->draw_max_y, maxy);
        }

        if (vc4->dirty & (VC4_DIRTY_RASTERIZER |
                          VC4_DIRTY_ZSA |
                          VC4_DIRTY_COMPILED_FS)) {
                uint8_t ez_enable_mask_out = ~0;
                uint8_t rasosm_mask_out = ~0;

                struct vc4_cl_out *bcl = cl_start(&job->bcl);
                /* HW-2905: If the RCL ends up doing a full-res load when
                 * multisampling, then early Z tracking may end up with values
                 * from the previous tile due to a HW bug.  Disable it to
                 * avoid that.
                 *
                 * We should be able to skip this when the Z is cleared, but I
                 * was seeing bad rendering on glxgears -samples 4 even in
                 * that case.
                 */
                if (job->msaa || vc4->prog.fs->disable_early_z)
                        ez_enable_mask_out &= ~VC4_CONFIG_BITS_EARLY_Z;

                /* Don't set the rasterizer to oversample if we're doing our
                 * binning and load/stores in single-sample mode.  This is for
                 * the samples == 1 case, where vc4 doesn't do any
                 * multisampling behavior.
                 */
                if (!job->msaa) {
                        rasosm_mask_out &=
                                ~VC4_CONFIG_BITS_RASTERIZER_OVERSAMPLE_4X;
                }

                cl_u8(&bcl, VC4_PACKET_CONFIGURATION_BITS);
                cl_u8(&bcl,
                      (vc4->rasterizer->config_bits[0] |
                       vc4->zsa->config_bits[0]) & rasosm_mask_out);
                cl_u8(&bcl,
                      vc4->rasterizer->config_bits[1] |
                      vc4->zsa->config_bits[1]);
                cl_u8(&bcl,
                      (vc4->rasterizer->config_bits[2] |
                       vc4->zsa->config_bits[2]) & ez_enable_mask_out);
                cl_end(&job->bcl, bcl);
        }

        if (vc4->dirty & VC4_DIRTY_RASTERIZER) {
                cl_emit_prepacked(&job->bcl, &vc4->rasterizer->packed);
        }

        if (vc4->dirty & VC4_DIRTY_VIEWPORT) {
                cl_emit(&job->bcl, CLIPPER_XY_SCALING, clip) {
                        clip.viewport_half_width_in_1_16th_of_pixel =
                                vc4->viewport.scale[0] * 16.0f;
                        clip.viewport_half_height_in_1_16th_of_pixel =
                                vc4->viewport.scale[1] * 16.0f;
                }

                cl_emit(&job->bcl, CLIPPER_Z_SCALE_AND_OFFSET, clip) {
                        clip.viewport_z_offset_zc_to_zs =
                                vc4->viewport.translate[2];
                        clip.viewport_z_scale_zc_to_zs =
                                vc4->viewport.scale[2];
                }

                cl_emit(&job->bcl, VIEWPORT_OFFSET, vp) {
                        vp.viewport_centre_x_coordinate =
                                vc4->viewport.translate[0];
                        vp.viewport_centre_y_coordinate =
                                vc4->viewport.translate[1];
                }
        }

        if (vc4->dirty & VC4_DIRTY_FLAT_SHADE_FLAGS) {
                cl_emit(&job->bcl, FLAT_SHADE_FLAGS, flags) {
                        if (vc4->rasterizer->base.flatshade)
                                flags.flat_shading_flags =
                                        vc4->prog.fs->color_inputs;
                }
        }
}
