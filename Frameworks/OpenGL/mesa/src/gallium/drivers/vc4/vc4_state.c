/*
 * Copyright © 2014 Broadcom
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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

#include "pipe/p_state.h"
#include "util/u_framebuffer.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_helpers.h"

#include "vc4_context.h"

static void *
vc4_generic_cso_state_create(const void *src, uint32_t size)
{
        void *dst = calloc(1, size);
        if (!dst)
                return NULL;
        memcpy(dst, src, size);
        return dst;
}

static void
vc4_generic_cso_state_delete(struct pipe_context *pctx, void *hwcso)
{
        free(hwcso);
}

static void
vc4_set_blend_color(struct pipe_context *pctx,
                    const struct pipe_blend_color *blend_color)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        vc4->blend_color.f = *blend_color;
        for (int i = 0; i < 4; i++)
                vc4->blend_color.ub[i] = float_to_ubyte(blend_color->color[i]);
        vc4->dirty |= VC4_DIRTY_BLEND_COLOR;
}

static void
vc4_set_stencil_ref(struct pipe_context *pctx,
                    const struct pipe_stencil_ref stencil_ref)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        vc4->stencil_ref = stencil_ref;
        vc4->dirty |= VC4_DIRTY_STENCIL_REF;
}

static void
vc4_set_clip_state(struct pipe_context *pctx,
                   const struct pipe_clip_state *clip)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        vc4->clip = *clip;
        vc4->dirty |= VC4_DIRTY_CLIP;
}

static void
vc4_set_sample_mask(struct pipe_context *pctx, unsigned sample_mask)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        vc4->sample_mask = sample_mask & ((1 << VC4_MAX_SAMPLES) - 1);
        vc4->dirty |= VC4_DIRTY_SAMPLE_MASK;
}

static uint16_t
float_to_187_half(float f)
{
        return fui(f) >> 16;
}

static void *
vc4_create_rasterizer_state(struct pipe_context *pctx,
                            const struct pipe_rasterizer_state *cso)
{
        struct vc4_rasterizer_state *so;
        struct V3D21_DEPTH_OFFSET depth_offset = { V3D21_DEPTH_OFFSET_header };
        struct V3D21_POINT_SIZE point_size = { V3D21_POINT_SIZE_header };
        struct V3D21_LINE_WIDTH line_width = { V3D21_LINE_WIDTH_header };

        so = CALLOC_STRUCT(vc4_rasterizer_state);
        if (!so)
                return NULL;

        so->base = *cso;

        if (!(cso->cull_face & PIPE_FACE_FRONT))
                so->config_bits[0] |= VC4_CONFIG_BITS_ENABLE_PRIM_FRONT;
        if (!(cso->cull_face & PIPE_FACE_BACK))
                so->config_bits[0] |= VC4_CONFIG_BITS_ENABLE_PRIM_BACK;

        /* Workaround: HW-2726 PTB does not handle zero-size points (BCM2835,
         * BCM21553).
         */
        point_size.point_size = MAX2(cso->point_size, .125f);

        line_width.line_width = cso->line_width;

        if (cso->front_ccw)
                so->config_bits[0] |= VC4_CONFIG_BITS_CW_PRIMITIVES;

        if (cso->offset_tri) {
                so->config_bits[0] |= VC4_CONFIG_BITS_ENABLE_DEPTH_OFFSET;

                depth_offset.depth_offset_units =
                        float_to_187_half(cso->offset_units);
                depth_offset.depth_offset_factor =
                        float_to_187_half(cso->offset_scale);
        }

        if (cso->multisample)
                so->config_bits[0] |= VC4_CONFIG_BITS_RASTERIZER_OVERSAMPLE_4X;

        V3D21_DEPTH_OFFSET_pack(NULL, so->packed.depth_offset, &depth_offset);
        V3D21_POINT_SIZE_pack(NULL, so->packed.point_size, &point_size);
        V3D21_LINE_WIDTH_pack(NULL, so->packed.line_width, &line_width);

        if (cso->tile_raster_order_fixed) {
                so->tile_raster_order_flags |= VC4_SUBMIT_CL_FIXED_RCL_ORDER;
                if (cso->tile_raster_order_increasing_x) {
                        so->tile_raster_order_flags |=
                                VC4_SUBMIT_CL_RCL_ORDER_INCREASING_X;
                }
                if (cso->tile_raster_order_increasing_y) {
                        so->tile_raster_order_flags |=
                                VC4_SUBMIT_CL_RCL_ORDER_INCREASING_Y;
                }
        }

        return so;
}

/* Blend state is baked into shaders. */
static void *
vc4_create_blend_state(struct pipe_context *pctx,
                       const struct pipe_blend_state *cso)
{
        return vc4_generic_cso_state_create(cso, sizeof(*cso));
}

/**
 * The TLB_STENCIL_SETUP data has a little bitfield for common writemask
 * values, so you don't have to do a separate writemask setup.
 */
static uint8_t
tlb_stencil_setup_writemask(uint8_t mask)
{
        switch (mask) {
        case 0x1: return 0;
        case 0x3: return 1;
        case 0xf: return 2;
        case 0xff: return 3;
        default: return 0xff;
        }
}

static uint32_t
tlb_stencil_setup_bits(const struct pipe_stencil_state *state,
                       uint8_t writemask_bits)
{
        static const uint8_t op_map[] = {
                [PIPE_STENCIL_OP_ZERO] = 0,
                [PIPE_STENCIL_OP_KEEP] = 1,
                [PIPE_STENCIL_OP_REPLACE] = 2,
                [PIPE_STENCIL_OP_INCR] = 3,
                [PIPE_STENCIL_OP_DECR] = 4,
                [PIPE_STENCIL_OP_INVERT] = 5,
                [PIPE_STENCIL_OP_INCR_WRAP] = 6,
                [PIPE_STENCIL_OP_DECR_WRAP] = 7,
        };
        uint32_t bits = 0;

        if (writemask_bits != 0xff)
                bits |= writemask_bits << 28;
        bits |= op_map[state->zfail_op] << 25;
        bits |= op_map[state->zpass_op] << 22;
        bits |= op_map[state->fail_op] << 19;
        bits |= state->func << 16;
        /* Ref is filled in at uniform upload time */
        bits |= state->valuemask << 0;

        return bits;
}

static void *
vc4_create_depth_stencil_alpha_state(struct pipe_context *pctx,
                                     const struct pipe_depth_stencil_alpha_state *cso)
{
        struct vc4_depth_stencil_alpha_state *so;

        so = CALLOC_STRUCT(vc4_depth_stencil_alpha_state);
        if (!so)
                return NULL;

        so->base = *cso;

        /* We always keep the early Z state correct, since a later state using
         * early Z may want it.
         */
        so->config_bits[2] |= VC4_CONFIG_BITS_EARLY_Z_UPDATE;

        if (cso->depth_enabled) {
                if (cso->depth_writemask) {
                        so->config_bits[1] |= VC4_CONFIG_BITS_Z_UPDATE;
                }
                so->config_bits[1] |= (cso->depth_func <<
                                       VC4_CONFIG_BITS_DEPTH_FUNC_SHIFT);

                /* We only handle early Z in the < direction because otherwise
                 * we'd have to runtime guess which direction to set in the
                 * render config.
                 */
                if ((cso->depth_func == PIPE_FUNC_LESS ||
                     cso->depth_func == PIPE_FUNC_LEQUAL) &&
                    (!cso->stencil[0].enabled ||
                     (cso->stencil[0].zfail_op == PIPE_STENCIL_OP_KEEP &&
                      (!cso->stencil[1].enabled ||
                       cso->stencil[1].zfail_op == PIPE_STENCIL_OP_KEEP)))) {
                        so->config_bits[2] |= VC4_CONFIG_BITS_EARLY_Z;
                }
        } else {
                so->config_bits[1] |= (PIPE_FUNC_ALWAYS <<
                                       VC4_CONFIG_BITS_DEPTH_FUNC_SHIFT);
        }

        if (cso->stencil[0].enabled) {
                const struct pipe_stencil_state *front = &cso->stencil[0];
                const struct pipe_stencil_state *back = &cso->stencil[1];

                uint8_t front_writemask_bits =
                        tlb_stencil_setup_writemask(front->writemask);
                uint8_t back_writemask = front->writemask;
                uint8_t back_writemask_bits = front_writemask_bits;

                so->stencil_uniforms[0] =
                        tlb_stencil_setup_bits(front, front_writemask_bits);
                if (back->enabled) {
                        back_writemask = back->writemask;
                        back_writemask_bits =
                                tlb_stencil_setup_writemask(back->writemask);

                        so->stencil_uniforms[0] |= (1 << 30);
                        so->stencil_uniforms[1] =
                                tlb_stencil_setup_bits(back, back_writemask_bits);
                        so->stencil_uniforms[1] |= (2 << 30);
                } else {
                        so->stencil_uniforms[0] |= (3 << 30);
                }

                if (front_writemask_bits == 0xff ||
                    back_writemask_bits == 0xff) {
                        so->stencil_uniforms[2] = (front->writemask |
                                                   (back_writemask << 8));
                }
        }

        return so;
}

static void
vc4_set_polygon_stipple(struct pipe_context *pctx,
                        const struct pipe_poly_stipple *stipple)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        vc4->stipple = *stipple;
        vc4->dirty |= VC4_DIRTY_STIPPLE;
}

static void
vc4_set_scissor_states(struct pipe_context *pctx,
                       unsigned start_slot,
                       unsigned num_scissors,
                       const struct pipe_scissor_state *scissor)
{
        struct vc4_context *vc4 = vc4_context(pctx);

        vc4->scissor = *scissor;
        vc4->dirty |= VC4_DIRTY_SCISSOR;
}

static void
vc4_set_viewport_states(struct pipe_context *pctx,
                        unsigned start_slot,
                        unsigned num_viewports,
                        const struct pipe_viewport_state *viewport)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        vc4->viewport = *viewport;
        vc4->dirty |= VC4_DIRTY_VIEWPORT;
}

static void
vc4_set_vertex_buffers(struct pipe_context *pctx,
                       unsigned count,
                       unsigned unbind_num_trailing_slots,
                       bool take_ownership,
                       const struct pipe_vertex_buffer *vb)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct vc4_vertexbuf_stateobj *so = &vc4->vertexbuf;

        util_set_vertex_buffers_mask(so->vb, &so->enabled_mask, vb,
                                     count, unbind_num_trailing_slots,
                                     take_ownership);
        so->count = util_last_bit(so->enabled_mask);

        vc4->dirty |= VC4_DIRTY_VTXBUF;
}

static void
vc4_blend_state_bind(struct pipe_context *pctx, void *hwcso)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        vc4->blend = hwcso;
        vc4->dirty |= VC4_DIRTY_BLEND;
}

static void
vc4_rasterizer_state_bind(struct pipe_context *pctx, void *hwcso)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct vc4_rasterizer_state *rast = hwcso;

        if (vc4->rasterizer && rast &&
            vc4->rasterizer->base.flatshade != rast->base.flatshade) {
                vc4->dirty |= VC4_DIRTY_FLAT_SHADE_FLAGS;
        }

        vc4->rasterizer = hwcso;
        vc4->dirty |= VC4_DIRTY_RASTERIZER;
}

static void
vc4_zsa_state_bind(struct pipe_context *pctx, void *hwcso)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        vc4->zsa = hwcso;
        vc4->dirty |= VC4_DIRTY_ZSA;
}

static void *
vc4_vertex_state_create(struct pipe_context *pctx, unsigned num_elements,
                        const struct pipe_vertex_element *elements)
{
        struct vc4_vertex_stateobj *so = CALLOC_STRUCT(vc4_vertex_stateobj);

        if (!so)
                return NULL;

        memcpy(so->pipe, elements, sizeof(*elements) * num_elements);
        so->num_elements = num_elements;

        return so;
}

static void
vc4_vertex_state_bind(struct pipe_context *pctx, void *hwcso)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        vc4->vtx = hwcso;
        vc4->dirty |= VC4_DIRTY_VTXSTATE;
}

static void
vc4_set_constant_buffer(struct pipe_context *pctx,
                        enum pipe_shader_type shader, uint index,
                        bool take_ownership,
                        const struct pipe_constant_buffer *cb)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct vc4_constbuf_stateobj *so = &vc4->constbuf[shader];

        /* Note that the gallium frontend can unbind constant buffers by
         * passing NULL here.
         */
        if (unlikely(!cb)) {
                so->enabled_mask &= ~(1 << index);
                so->dirty_mask &= ~(1 << index);
                return;
        }

        if (index == 1 && so->cb[index].buffer_size != cb->buffer_size)
                vc4->dirty |= VC4_DIRTY_UBO_1_SIZE;

        util_copy_constant_buffer(&so->cb[index], cb, take_ownership);

        so->enabled_mask |= 1 << index;
        so->dirty_mask |= 1 << index;
        vc4->dirty |= VC4_DIRTY_CONSTBUF;
}

static void
vc4_set_framebuffer_state(struct pipe_context *pctx,
                          const struct pipe_framebuffer_state *framebuffer)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct pipe_framebuffer_state *cso = &vc4->framebuffer;

        vc4->job = NULL;

        util_copy_framebuffer_state(cso, framebuffer);

        /* Nonzero texture mipmap levels are laid out as if they were in
         * power-of-two-sized spaces.  The renderbuffer config infers its
         * stride from the width parameter, so we need to configure our
         * framebuffer.  Note that if the z/color buffers were mismatched
         * sizes, we wouldn't be able to do this.
         */
        if (cso->cbufs[0] && cso->cbufs[0]->u.tex.level) {
                struct vc4_resource *rsc =
                        vc4_resource(cso->cbufs[0]->texture);
                cso->width =
                        (rsc->slices[cso->cbufs[0]->u.tex.level].stride /
                         rsc->cpp);
        } else if (cso->zsbuf && cso->zsbuf->u.tex.level){
                struct vc4_resource *rsc =
                        vc4_resource(cso->zsbuf->texture);
                cso->width =
                        (rsc->slices[cso->zsbuf->u.tex.level].stride /
                         rsc->cpp);
        }

        vc4->dirty |= VC4_DIRTY_FRAMEBUFFER;
}

static struct vc4_texture_stateobj *
vc4_get_stage_tex(struct vc4_context *vc4, enum pipe_shader_type shader)
{
        switch (shader) {
        case PIPE_SHADER_FRAGMENT:
                vc4->dirty |= VC4_DIRTY_FRAGTEX;
                return &vc4->fragtex;
                break;
        case PIPE_SHADER_VERTEX:
                vc4->dirty |= VC4_DIRTY_VERTTEX;
                return &vc4->verttex;
                break;
        default:
                fprintf(stderr, "Unknown shader target %d\n", shader);
                abort();
        }
}

static uint32_t translate_wrap(uint32_t p_wrap, bool using_nearest)
{
        switch (p_wrap) {
        case PIPE_TEX_WRAP_REPEAT:
                return 0;
        case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
                return 1;
        case PIPE_TEX_WRAP_MIRROR_REPEAT:
                return 2;
        case PIPE_TEX_WRAP_CLAMP_TO_BORDER:
                return 3;
        case PIPE_TEX_WRAP_CLAMP:
                return (using_nearest ? 1 : 3);
        default:
                fprintf(stderr, "Unknown wrap mode %d\n", p_wrap);
                assert(!"not reached");
                return 0;
        }
}

static void *
vc4_create_sampler_state(struct pipe_context *pctx,
                         const struct pipe_sampler_state *cso)
{
        static const uint8_t minfilter_map[6] = {
                VC4_TEX_P1_MINFILT_NEAR_MIP_NEAR,
                VC4_TEX_P1_MINFILT_LIN_MIP_NEAR,
                VC4_TEX_P1_MINFILT_NEAR_MIP_LIN,
                VC4_TEX_P1_MINFILT_LIN_MIP_LIN,
                VC4_TEX_P1_MINFILT_NEAREST,
                VC4_TEX_P1_MINFILT_LINEAR,
        };
        static const uint32_t magfilter_map[] = {
                [PIPE_TEX_FILTER_NEAREST] = VC4_TEX_P1_MAGFILT_NEAREST,
                [PIPE_TEX_FILTER_LINEAR] = VC4_TEX_P1_MAGFILT_LINEAR,
        };
        bool either_nearest =
                (cso->mag_img_filter == PIPE_TEX_MIPFILTER_NEAREST ||
                 cso->min_img_filter == PIPE_TEX_MIPFILTER_NEAREST);
        struct vc4_sampler_state *so = CALLOC_STRUCT(vc4_sampler_state);

        if (!so)
                return NULL;

        memcpy(so, cso, sizeof(*cso));

        so->texture_p1 =
                (VC4_SET_FIELD(magfilter_map[cso->mag_img_filter],
                               VC4_TEX_P1_MAGFILT) |
                 VC4_SET_FIELD(minfilter_map[cso->min_mip_filter * 2 +
                                             cso->min_img_filter],
                               VC4_TEX_P1_MINFILT) |
                 VC4_SET_FIELD(translate_wrap(cso->wrap_s, either_nearest),
                               VC4_TEX_P1_WRAP_S) |
                 VC4_SET_FIELD(translate_wrap(cso->wrap_t, either_nearest),
                               VC4_TEX_P1_WRAP_T));

        return so;
}

static void
vc4_sampler_states_bind(struct pipe_context *pctx,
                        enum pipe_shader_type shader, unsigned start,
                        unsigned nr, void **hwcso)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct vc4_texture_stateobj *stage_tex = vc4_get_stage_tex(vc4, shader);

        assert(start == 0);
        unsigned i;
        unsigned new_nr = 0;

        for (i = 0; i < nr; i++) {
                if (hwcso[i])
                        new_nr = i + 1;
                stage_tex->samplers[i] = hwcso[i];
        }

        for (; i < stage_tex->num_samplers; i++) {
                stage_tex->samplers[i] = NULL;
        }

        stage_tex->num_samplers = new_nr;
}

static struct pipe_sampler_view *
vc4_create_sampler_view(struct pipe_context *pctx, struct pipe_resource *prsc,
                        const struct pipe_sampler_view *cso)
{
        struct vc4_sampler_view *so = CALLOC_STRUCT(vc4_sampler_view);
        struct vc4_resource *rsc = vc4_resource(prsc);

        if (!so)
                return NULL;

        so->base = *cso;

        so->base.texture = NULL;
        pipe_resource_reference(&so->base.texture, prsc);
        so->base.reference.count = 1;
        so->base.context = pctx;

        /* There is no hardware level clamping, and the start address of a
         * texture may be misaligned, so in that case we have to copy to a
         * temporary.
         *
         * Also, Raspberry Pi doesn't support sampling from raster textures,
         * so we also have to copy to a temporary then.
         */
        if ((cso->u.tex.first_level &&
             (cso->u.tex.first_level != cso->u.tex.last_level)) ||
            rsc->vc4_format == VC4_TEXTURE_TYPE_RGBA32R ||
            rsc->vc4_format == ~0) {
                struct vc4_resource *shadow_parent = rsc;
                struct pipe_resource tmpl = {
                        .target = prsc->target,
                        .format = prsc->format,
                        .width0 = u_minify(prsc->width0,
                                           cso->u.tex.first_level),
                        .height0 = u_minify(prsc->height0,
                                            cso->u.tex.first_level),
                        .bind = PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET,
                        .last_level = cso->u.tex.last_level - cso->u.tex.first_level,
                        .nr_samples = prsc->nr_samples,
                };

                /* Create the shadow texture.  The rest of the texture
                 * parameter setup will use the shadow.
                 */
                prsc = vc4_resource_create(pctx->screen, &tmpl);
                if (!prsc) {
                        free(so);
                        return NULL;
                }
                rsc = vc4_resource(prsc);
                vc4_bo_label(vc4_screen(pctx->screen), rsc->bo,
                            "tiling shadow %dx%d",
                             tmpl.width0, tmpl.height0);

                /* Flag it as needing update of the contents from the parent. */
                rsc->writes = shadow_parent->writes - 1;
                assert(rsc->vc4_format != VC4_TEXTURE_TYPE_RGBA32R);

                so->texture = prsc;
        } else {
                pipe_resource_reference(&so->texture, prsc);

                if (cso->u.tex.first_level) {
                        so->force_first_level = true;
                }
        }

        so->texture_p0 =
                (VC4_SET_FIELD((rsc->slices[0].offset +
                                cso->u.tex.first_layer *
                                rsc->cube_map_stride) >> 12, VC4_TEX_P0_OFFSET) |
                 VC4_SET_FIELD(rsc->vc4_format & 15, VC4_TEX_P0_TYPE) |
                 VC4_SET_FIELD(so->force_first_level ?
                               cso->u.tex.last_level :
                               cso->u.tex.last_level -
                               cso->u.tex.first_level, VC4_TEX_P0_MIPLVLS) |
                 VC4_SET_FIELD(cso->target == PIPE_TEXTURE_CUBE,
                               VC4_TEX_P0_CMMODE));
        so->texture_p1 =
                (VC4_SET_FIELD(rsc->vc4_format >> 4, VC4_TEX_P1_TYPE4) |
                 VC4_SET_FIELD(prsc->height0 & 2047, VC4_TEX_P1_HEIGHT) |
                 VC4_SET_FIELD(prsc->width0 & 2047, VC4_TEX_P1_WIDTH));

        if (prsc->format == PIPE_FORMAT_ETC1_RGB8)
                so->texture_p1 |= VC4_TEX_P1_ETCFLIP_MASK;

        return &so->base;
}

static void
vc4_sampler_view_destroy(struct pipe_context *pctx,
                         struct pipe_sampler_view *pview)
{
        struct vc4_sampler_view *view = vc4_sampler_view(pview);
        pipe_resource_reference(&pview->texture, NULL);
        pipe_resource_reference(&view->texture, NULL);
        free(view);
}

static void
vc4_set_sampler_views(struct pipe_context *pctx,
                      enum pipe_shader_type shader,
                      unsigned start, unsigned nr,
                      unsigned unbind_num_trailing_slots,
                      bool take_ownership,
                      struct pipe_sampler_view **views)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct vc4_texture_stateobj *stage_tex = vc4_get_stage_tex(vc4, shader);
        unsigned i;
        unsigned new_nr = 0;

        assert(start == 0);

        for (i = 0; i < nr; i++) {
                if (views[i])
                        new_nr = i + 1;
                if (take_ownership) {
                        pipe_sampler_view_reference(&stage_tex->textures[i], NULL);
                        stage_tex->textures[i] = views[i];
                } else {
                        pipe_sampler_view_reference(&stage_tex->textures[i], views[i]);
                }
        }

        for (; i < stage_tex->num_textures; i++) {
                pipe_sampler_view_reference(&stage_tex->textures[i], NULL);
        }

        stage_tex->num_textures = new_nr;
}

void
vc4_state_init(struct pipe_context *pctx)
{
        pctx->set_blend_color = vc4_set_blend_color;
        pctx->set_stencil_ref = vc4_set_stencil_ref;
        pctx->set_clip_state = vc4_set_clip_state;
        pctx->set_sample_mask = vc4_set_sample_mask;
        pctx->set_constant_buffer = vc4_set_constant_buffer;
        pctx->set_framebuffer_state = vc4_set_framebuffer_state;
        pctx->set_polygon_stipple = vc4_set_polygon_stipple;
        pctx->set_scissor_states = vc4_set_scissor_states;
        pctx->set_viewport_states = vc4_set_viewport_states;

        pctx->set_vertex_buffers = vc4_set_vertex_buffers;

        pctx->create_blend_state = vc4_create_blend_state;
        pctx->bind_blend_state = vc4_blend_state_bind;
        pctx->delete_blend_state = vc4_generic_cso_state_delete;

        pctx->create_rasterizer_state = vc4_create_rasterizer_state;
        pctx->bind_rasterizer_state = vc4_rasterizer_state_bind;
        pctx->delete_rasterizer_state = vc4_generic_cso_state_delete;

        pctx->create_depth_stencil_alpha_state = vc4_create_depth_stencil_alpha_state;
        pctx->bind_depth_stencil_alpha_state = vc4_zsa_state_bind;
        pctx->delete_depth_stencil_alpha_state = vc4_generic_cso_state_delete;

        pctx->create_vertex_elements_state = vc4_vertex_state_create;
        pctx->delete_vertex_elements_state = vc4_generic_cso_state_delete;
        pctx->bind_vertex_elements_state = vc4_vertex_state_bind;

        pctx->create_sampler_state = vc4_create_sampler_state;
        pctx->delete_sampler_state = vc4_generic_cso_state_delete;
        pctx->bind_sampler_states = vc4_sampler_states_bind;

        pctx->create_sampler_view = vc4_create_sampler_view;
        pctx->sampler_view_destroy = vc4_sampler_view_destroy;
        pctx->set_sampler_views = vc4_set_sampler_views;
}
