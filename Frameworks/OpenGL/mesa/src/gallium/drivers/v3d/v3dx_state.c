/*
 * Copyright © 2014-2017 Broadcom
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
#include "util/format/u_format.h"
#include "util/u_framebuffer.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/half_float.h"
#include "util/u_helpers.h"
#include "util/u_upload_mgr.h"

#include "v3d_context.h"
#include "broadcom/common/v3d_tiling.h"
#include "broadcom/common/v3d_macros.h"
#include "broadcom/common/v3d_util.h"
#include "broadcom/compiler/v3d_compiler.h"
#include "broadcom/cle/v3dx_pack.h"

static void
v3d_generic_cso_state_delete(struct pipe_context *pctx, void *hwcso)
{
        free(hwcso);
}

static void
v3d_set_blend_color(struct pipe_context *pctx,
                    const struct pipe_blend_color *blend_color)
{
        struct v3d_context *v3d = v3d_context(pctx);
        v3d->blend_color.f = *blend_color;
        for (int i = 0; i < 4; i++) {
                v3d->blend_color.hf[i] =
                        _mesa_float_to_half(blend_color->color[i]);
        }
        v3d->dirty |= V3D_DIRTY_BLEND_COLOR;
}

static void
v3d_set_stencil_ref(struct pipe_context *pctx,
                    const struct pipe_stencil_ref stencil_ref)
{
        struct v3d_context *v3d = v3d_context(pctx);
        v3d->stencil_ref = stencil_ref;
        v3d->dirty |= V3D_DIRTY_STENCIL_REF;
}

static void
v3d_set_clip_state(struct pipe_context *pctx,
                   const struct pipe_clip_state *clip)
{
        struct v3d_context *v3d = v3d_context(pctx);
        v3d->clip = *clip;
        v3d->dirty |= V3D_DIRTY_CLIP;
}

static void
v3d_set_sample_mask(struct pipe_context *pctx, unsigned sample_mask)
{
        struct v3d_context *v3d = v3d_context(pctx);
        v3d->sample_mask = sample_mask & ((1 << V3D_MAX_SAMPLES) - 1);
        v3d->dirty |= V3D_DIRTY_SAMPLE_STATE;
}

static void *
v3d_create_rasterizer_state(struct pipe_context *pctx,
                            const struct pipe_rasterizer_state *cso)
{
        struct v3d_rasterizer_state *so;

        so = CALLOC_STRUCT(v3d_rasterizer_state);
        if (!so)
                return NULL;

        so->base = *cso;

        /* Workaround: HW-2726 PTB does not handle zero-size points (BCM2835,
         * BCM21553).
         */
        so->point_size = MAX2(cso->point_size, .125f);

        STATIC_ASSERT(sizeof(so->depth_offset) >=
                      cl_packet_length(DEPTH_OFFSET));
        v3dx_pack(&so->depth_offset, DEPTH_OFFSET, depth) {
                depth.depth_offset_factor = cso->offset_scale;
                depth.depth_offset_units = cso->offset_units;
                depth.limit = cso->offset_clamp;
        }

        /* V3d 4.x treats polygon offset units based on a Z24 buffer, so we
         * need to scale up offset_units if we're only Z16.
         */
#if V3D_VERSION == 42
        v3dx_pack(&so->depth_offset_z16, DEPTH_OFFSET, depth) {
                depth.depth_offset_factor = cso->offset_scale;
                depth.depth_offset_units = cso->offset_units * 256.0;
                depth.limit = cso->offset_clamp;
        }
#endif

        return so;
}

/* Blend state is baked into shaders. */
static void *
v3d_create_blend_state(struct pipe_context *pctx,
                       const struct pipe_blend_state *cso)
{
        struct v3d_blend_state *so;

        so = CALLOC_STRUCT(v3d_blend_state);
        if (!so)
                return NULL;

        so->base = *cso;

        uint32_t max_rts = V3D_MAX_RENDER_TARGETS(V3D_VERSION);
        if (cso->independent_blend_enable) {
                for (int i = 0; i < max_rts; i++) {
                        so->blend_enables |= cso->rt[i].blend_enable << i;
                }
        } else {
                if (cso->rt[0].blend_enable)
                        so->blend_enables = (1 << max_rts) - 1;
        }

        return so;
}

static uint32_t
translate_stencil_op(enum pipe_stencil_op op)
{
        switch (op) {
        case PIPE_STENCIL_OP_KEEP:      return V3D_STENCIL_OP_KEEP;
        case PIPE_STENCIL_OP_ZERO:      return V3D_STENCIL_OP_ZERO;
        case PIPE_STENCIL_OP_REPLACE:   return V3D_STENCIL_OP_REPLACE;
        case PIPE_STENCIL_OP_INCR:      return V3D_STENCIL_OP_INCR;
        case PIPE_STENCIL_OP_DECR:      return V3D_STENCIL_OP_DECR;
        case PIPE_STENCIL_OP_INCR_WRAP: return V3D_STENCIL_OP_INCWRAP;
        case PIPE_STENCIL_OP_DECR_WRAP: return V3D_STENCIL_OP_DECWRAP;
        case PIPE_STENCIL_OP_INVERT:    return V3D_STENCIL_OP_INVERT;
        }
        unreachable("bad stencil op");
}

static void *
v3d_create_depth_stencil_alpha_state(struct pipe_context *pctx,
                                     const struct pipe_depth_stencil_alpha_state *cso)
{
        struct v3d_depth_stencil_alpha_state *so;

        so = CALLOC_STRUCT(v3d_depth_stencil_alpha_state);
        if (!so)
                return NULL;

        so->base = *cso;

        if (cso->depth_enabled) {
                switch (cso->depth_func) {
                case PIPE_FUNC_LESS:
                case PIPE_FUNC_LEQUAL:
                        so->ez_state = V3D_EZ_LT_LE;
                        break;
                case PIPE_FUNC_GREATER:
                case PIPE_FUNC_GEQUAL:
                        so->ez_state = V3D_EZ_GT_GE;
                        break;
                case PIPE_FUNC_NEVER:
                case PIPE_FUNC_EQUAL:
                        so->ez_state = V3D_EZ_UNDECIDED;
                        break;
                default:
                        so->ez_state = V3D_EZ_DISABLED;
                        break;
                }

                /* If stencil is enabled and it's not a no-op, then it would
                 * break EZ updates.
                 */
                if (cso->stencil[0].enabled &&
                    (cso->stencil[0].zfail_op != PIPE_STENCIL_OP_KEEP ||
                     cso->stencil[0].func != PIPE_FUNC_ALWAYS ||
                     (cso->stencil[1].enabled &&
                      (cso->stencil[1].zfail_op != PIPE_STENCIL_OP_KEEP ||
                       cso->stencil[1].func != PIPE_FUNC_ALWAYS)))) {
                        so->ez_state = V3D_EZ_DISABLED;
                }
        }

        const struct pipe_stencil_state *front = &cso->stencil[0];
        const struct pipe_stencil_state *back = &cso->stencil[1];

        if (front->enabled) {
                STATIC_ASSERT(sizeof(so->stencil_front) >=
                              cl_packet_length(STENCIL_CFG));
                v3dx_pack(&so->stencil_front, STENCIL_CFG, config) {
                        config.front_config = true;
                        /* If !back->enabled, then the front values should be
                         * used for both front and back-facing primitives.
                         */
                        config.back_config = !back->enabled;

                        config.stencil_write_mask = front->writemask;
                        config.stencil_test_mask = front->valuemask;

                        config.stencil_test_function = front->func;
                        config.stencil_pass_op =
                                translate_stencil_op(front->zpass_op);
                        config.depth_test_fail_op =
                                translate_stencil_op(front->zfail_op);
                        config.stencil_test_fail_op =
                                translate_stencil_op(front->fail_op);
                }
        }
        if (back->enabled) {
                STATIC_ASSERT(sizeof(so->stencil_back) >=
                              cl_packet_length(STENCIL_CFG));
                v3dx_pack(&so->stencil_back, STENCIL_CFG, config) {
                        config.front_config = false;
                        config.back_config = true;

                        config.stencil_write_mask = back->writemask;
                        config.stencil_test_mask = back->valuemask;

                        config.stencil_test_function = back->func;
                        config.stencil_pass_op =
                                translate_stencil_op(back->zpass_op);
                        config.depth_test_fail_op =
                                translate_stencil_op(back->zfail_op);
                        config.stencil_test_fail_op =
                                translate_stencil_op(back->fail_op);
                }
        }

        return so;
}

static void
v3d_set_polygon_stipple(struct pipe_context *pctx,
                        const struct pipe_poly_stipple *stipple)
{
        struct v3d_context *v3d = v3d_context(pctx);
        v3d->stipple = *stipple;
        v3d->dirty |= V3D_DIRTY_STIPPLE;
}

static void
v3d_set_scissor_states(struct pipe_context *pctx,
                       unsigned start_slot,
                       unsigned num_scissors,
                       const struct pipe_scissor_state *scissor)
{
        struct v3d_context *v3d = v3d_context(pctx);

        v3d->scissor = *scissor;
        v3d->dirty |= V3D_DIRTY_SCISSOR;
}

static void
v3d_set_viewport_states(struct pipe_context *pctx,
                        unsigned start_slot,
                        unsigned num_viewports,
                        const struct pipe_viewport_state *viewport)
{
        struct v3d_context *v3d = v3d_context(pctx);
        v3d->viewport = *viewport;
        v3d->dirty |= V3D_DIRTY_VIEWPORT;
}

static void
v3d_set_vertex_buffers(struct pipe_context *pctx,
                       unsigned count,
                       unsigned unbind_num_trailing_slots,
                       bool take_ownership,
                       const struct pipe_vertex_buffer *vb)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_vertexbuf_stateobj *so = &v3d->vertexbuf;

        util_set_vertex_buffers_mask(so->vb, &so->enabled_mask, vb,
                                     count, unbind_num_trailing_slots,
                                     take_ownership);
        so->count = util_last_bit(so->enabled_mask);

        v3d->dirty |= V3D_DIRTY_VTXBUF;
}

static void
v3d_blend_state_bind(struct pipe_context *pctx, void *hwcso)
{
        struct v3d_context *v3d = v3d_context(pctx);
        v3d->blend = hwcso;
        v3d->dirty |= V3D_DIRTY_BLEND;
}

static void
v3d_rasterizer_state_bind(struct pipe_context *pctx, void *hwcso)
{
        struct v3d_context *v3d = v3d_context(pctx);
        v3d->rasterizer = hwcso;
        v3d->dirty |= V3D_DIRTY_RASTERIZER;
}

static void
v3d_zsa_state_bind(struct pipe_context *pctx, void *hwcso)
{
        struct v3d_context *v3d = v3d_context(pctx);
        v3d->zsa = hwcso;
        v3d->dirty |= V3D_DIRTY_ZSA;
}


static bool
needs_default_attribute_values(void)
{
#if V3D_VERSION == 42
        /* FIXME: on vulkan we are able to refine even further, as we know in
         * advance when we create the pipeline if we have an integer vertex
         * attrib. Pending to check if we could do something similar here.
         */
        return true;
#endif
        return false;
}

static void *
v3d_vertex_state_create(struct pipe_context *pctx, unsigned num_elements,
                        const struct pipe_vertex_element *elements)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_vertex_stateobj *so = CALLOC_STRUCT(v3d_vertex_stateobj);

        if (!so)
                return NULL;

        memcpy(so->pipe, elements, sizeof(*elements) * num_elements);
        so->num_elements = num_elements;

        for (int i = 0; i < so->num_elements; i++) {
                const struct pipe_vertex_element *elem = &elements[i];
                const struct util_format_description *desc =
                        util_format_description(elem->src_format);
                uint32_t r_size = desc->channel[0].size;

                const uint32_t size =
                        cl_packet_length(GL_SHADER_STATE_ATTRIBUTE_RECORD);

                v3dx_pack(&so->attrs[i * size],
                          GL_SHADER_STATE_ATTRIBUTE_RECORD, attr) {
                        /* vec_size == 0 means 4 */
                        attr.vec_size = desc->nr_channels & 3;
                        attr.signed_int_type = (desc->channel[0].type ==
                                                UTIL_FORMAT_TYPE_SIGNED);

                        attr.normalized_int_type = desc->channel[0].normalized;
                        attr.read_as_int_uint = desc->channel[0].pure_integer;
                        attr.instance_divisor = MIN2(elem->instance_divisor,
                                                     0xffff);

                        switch (desc->channel[0].type) {
                        case UTIL_FORMAT_TYPE_FLOAT:
                                if (r_size == 32) {
                                        attr.type = ATTRIBUTE_FLOAT;
                                } else {
                                        assert(r_size == 16);
                                        attr.type = ATTRIBUTE_HALF_FLOAT;
                                }
                                break;

                        case UTIL_FORMAT_TYPE_SIGNED:
                        case UTIL_FORMAT_TYPE_UNSIGNED:
                                switch (r_size) {
                                case 32:
                                        attr.type = ATTRIBUTE_INT;
                                        break;
                                case 16:
                                        attr.type = ATTRIBUTE_SHORT;
                                        break;
                                case 10:
                                        attr.type = ATTRIBUTE_INT2_10_10_10;
                                        break;
                                case 8:
                                        attr.type = ATTRIBUTE_BYTE;
                                        break;
                                default:
                                        fprintf(stderr,
                                                "format %s unsupported\n",
                                                desc->name);
                                        attr.type = ATTRIBUTE_BYTE;
                                        abort();
                                }
                                break;

                        default:
                                fprintf(stderr,
                                        "format %s unsupported\n",
                                        desc->name);
                                abort();
                        }
                }
        }

        if (needs_default_attribute_values()) {
                /* Set up the default attribute values in case any of the vertex
                 * elements use them.
                 */
                uint32_t *attrs;
                u_upload_alloc(v3d->state_uploader, 0,
                               V3D_MAX_VS_INPUTS * sizeof(float), 16,
                               &so->defaults_offset, &so->defaults, (void **)&attrs);

                for (int i = 0; i < V3D_MAX_VS_INPUTS / 4; i++) {
                        attrs[i * 4 + 0] = 0;
                        attrs[i * 4 + 1] = 0;
                        attrs[i * 4 + 2] = 0;
                        if (i < so->num_elements &&
                            util_format_is_pure_integer(so->pipe[i].src_format)) {
                                attrs[i * 4 + 3] = 1;
                        } else {
                                attrs[i * 4 + 3] = fui(1.0);
                        }
                }
        } else {
                so->defaults = NULL;
                so->defaults_offset = 0;
        }

        u_upload_unmap(v3d->state_uploader);
        return so;
}

static void
v3d_vertex_state_delete(struct pipe_context *pctx, void *hwcso)
{
        struct v3d_vertex_stateobj *so = hwcso;

        pipe_resource_reference(&so->defaults, NULL);
        free(so);
}

static void
v3d_vertex_state_bind(struct pipe_context *pctx, void *hwcso)
{
        struct v3d_context *v3d = v3d_context(pctx);
        v3d->vtx = hwcso;
        v3d->dirty |= V3D_DIRTY_VTXSTATE;
}

static void
v3d_set_constant_buffer(struct pipe_context *pctx, enum pipe_shader_type shader, uint index,
                        bool take_ownership,
                        const struct pipe_constant_buffer *cb)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_constbuf_stateobj *so = &v3d->constbuf[shader];

        util_copy_constant_buffer(&so->cb[index], cb, take_ownership);

        /* Note that the gallium frontend can unbind constant buffers by
         * passing NULL here.
         */
        if (unlikely(!cb)) {
                so->enabled_mask &= ~(1 << index);
                so->dirty_mask &= ~(1 << index);
                return;
        }

        so->enabled_mask |= 1 << index;
        so->dirty_mask |= 1 << index;
        v3d->dirty |= V3D_DIRTY_CONSTBUF;
}

static void
v3d_set_framebuffer_state(struct pipe_context *pctx,
                          const struct pipe_framebuffer_state *framebuffer)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct pipe_framebuffer_state *cso = &v3d->framebuffer;

        v3d->job = NULL;

        util_copy_framebuffer_state(cso, framebuffer);

        v3d->swap_color_rb = 0;
        v3d->blend_dst_alpha_one = 0;
        for (int i = 0; i < v3d->framebuffer.nr_cbufs; i++) {
                struct pipe_surface *cbuf = v3d->framebuffer.cbufs[i];
                if (!cbuf)
                        continue;

                const struct util_format_description *desc =
                        util_format_description(cbuf->format);

                if (desc->swizzle[3] == PIPE_SWIZZLE_1)
                        v3d->blend_dst_alpha_one |= 1 << i;
        }

        v3d->dirty |= V3D_DIRTY_FRAMEBUFFER;
}

static enum V3DX(Wrap_Mode)
translate_wrap(uint32_t pipe_wrap)
{
        switch (pipe_wrap) {
        case PIPE_TEX_WRAP_REPEAT:
                return V3D_WRAP_MODE_REPEAT;
        case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
                return V3D_WRAP_MODE_CLAMP;
        case PIPE_TEX_WRAP_MIRROR_REPEAT:
                return V3D_WRAP_MODE_MIRROR;
        case PIPE_TEX_WRAP_CLAMP_TO_BORDER:
                return V3D_WRAP_MODE_BORDER;
        case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE:
                return V3D_WRAP_MODE_MIRROR_ONCE;
        default:
                unreachable("Unknown wrap mode");
        }
}

static void
v3d_upload_sampler_state_variant(void *map,
                                 const struct pipe_sampler_state *cso,
                                 enum v3d_sampler_state_variant variant)
{
        v3dx_pack(map, SAMPLER_STATE, sampler) {
                sampler.wrap_i_border = false;

                sampler.wrap_s = translate_wrap(cso->wrap_s);
                sampler.wrap_t = translate_wrap(cso->wrap_t);
                sampler.wrap_r = translate_wrap(cso->wrap_r);

                sampler.fixed_bias = cso->lod_bias;
                sampler.depth_compare_function = cso->compare_mode ?
                                                 cso->compare_func :
                                                 V3D_COMPARE_FUNC_NEVER;
                sampler.min_filter_nearest =
                        cso->min_img_filter == PIPE_TEX_FILTER_NEAREST;
                sampler.mag_filter_nearest =
                        cso->mag_img_filter == PIPE_TEX_FILTER_NEAREST;
                sampler.mip_filter_nearest =
                        cso->min_mip_filter != PIPE_TEX_MIPFILTER_LINEAR;

                sampler.min_level_of_detail = MIN2(MAX2(0, cso->min_lod),
                                                   15);
                sampler.max_level_of_detail = MIN2(MAX2(cso->max_lod,
                                                        cso->min_lod), 15);

                /* If we're not doing inter-miplevel filtering, we need to
                 * clamp the LOD so that we only sample from baselevel.
                 * However, we need to still allow the calculated LOD to be
                 * fractionally over the baselevel, so that the HW can decide
                 * between the min and mag filters.
                 */
                if (cso->min_mip_filter == PIPE_TEX_MIPFILTER_NONE) {
                        sampler.min_level_of_detail =
                                MIN2(sampler.min_level_of_detail, 1.0 / 256.0);
                        sampler.max_level_of_detail =
                                MIN2(sampler.max_level_of_detail, 1.0 / 256.0);
                }

                if (cso->max_anisotropy) {
                        sampler.anisotropy_enable = true;

                        if (cso->max_anisotropy > 8)
                                sampler.maximum_anisotropy = 3;
                        else if (cso->max_anisotropy > 4)
                                sampler.maximum_anisotropy = 2;
                        else if (cso->max_anisotropy > 2)
                                sampler.maximum_anisotropy = 1;
                }

                if (variant == V3D_SAMPLER_STATE_BORDER_0000) {
                        sampler.border_color_mode = V3D_BORDER_COLOR_0000;
                } else if (variant == V3D_SAMPLER_STATE_BORDER_0001) {
                        sampler.border_color_mode = V3D_BORDER_COLOR_0001;
                } else if (variant == V3D_SAMPLER_STATE_BORDER_1111) {
                        sampler.border_color_mode = V3D_BORDER_COLOR_1111;
                } else {
                        sampler.border_color_mode = V3D_BORDER_COLOR_FOLLOWS;

                        union pipe_color_union border;

                        /* First, reswizzle the border color for any
                         * mismatching we're doing between the texture's
                         * channel order in hardware (R) versus what it is at
                         * the GL level (ALPHA)
                         */
                        switch (variant) {
                        case V3D_SAMPLER_STATE_F16_BGRA:
                        case V3D_SAMPLER_STATE_F16_BGRA_UNORM:
                        case V3D_SAMPLER_STATE_F16_BGRA_SNORM:
                                border.i[0] = cso->border_color.i[2];
                                border.i[1] = cso->border_color.i[1];
                                border.i[2] = cso->border_color.i[0];
                                border.i[3] = cso->border_color.i[3];
                                break;

                        case V3D_SAMPLER_STATE_F16_A:
                        case V3D_SAMPLER_STATE_F16_A_UNORM:
                        case V3D_SAMPLER_STATE_F16_A_SNORM:
                        case V3D_SAMPLER_STATE_32_A:
                        case V3D_SAMPLER_STATE_32_A_UNORM:
                        case V3D_SAMPLER_STATE_32_A_SNORM:
                                border.i[0] = cso->border_color.i[3];
                                border.i[1] = 0;
                                border.i[2] = 0;
                                border.i[3] = 0;
                                break;

                        case V3D_SAMPLER_STATE_F16_LA:
                        case V3D_SAMPLER_STATE_F16_LA_UNORM:
                        case V3D_SAMPLER_STATE_F16_LA_SNORM:
                                border.i[0] = cso->border_color.i[0];
                                border.i[1] = cso->border_color.i[3];
                                border.i[2] = 0;
                                border.i[3] = 0;
                                break;

                        default:
                                border = cso->border_color;
                        }

                        /* Perform any clamping. */
                        switch (variant) {
                        case V3D_SAMPLER_STATE_F16_UNORM:
                        case V3D_SAMPLER_STATE_F16_BGRA_UNORM:
                        case V3D_SAMPLER_STATE_F16_A_UNORM:
                        case V3D_SAMPLER_STATE_F16_LA_UNORM:
                        case V3D_SAMPLER_STATE_32_UNORM:
                        case V3D_SAMPLER_STATE_32_A_UNORM:
                                for (int i = 0; i < 4; i++)
                                        border.f[i] = CLAMP(border.f[i], 0, 1);
                                break;

                        case V3D_SAMPLER_STATE_F16_SNORM:
                        case V3D_SAMPLER_STATE_F16_BGRA_SNORM:
                        case V3D_SAMPLER_STATE_F16_A_SNORM:
                        case V3D_SAMPLER_STATE_F16_LA_SNORM:
                        case V3D_SAMPLER_STATE_32_SNORM:
                        case V3D_SAMPLER_STATE_32_A_SNORM:
                                for (int i = 0; i < 4; i++)
                                        border.f[i] = CLAMP(border.f[i], -1, 1);
                                break;

                        case V3D_SAMPLER_STATE_1010102U:
                                border.ui[0] = CLAMP(border.ui[0],
                                                     0, (1 << 10) - 1);
                                border.ui[1] = CLAMP(border.ui[1],
                                                     0, (1 << 10) - 1);
                                border.ui[2] = CLAMP(border.ui[2],
                                                     0, (1 << 10) - 1);
                                border.ui[3] = CLAMP(border.ui[3],
                                                     0, 3);
                                break;

                        case V3D_SAMPLER_STATE_16U:
                                for (int i = 0; i < 4; i++)
                                        border.ui[i] = CLAMP(border.ui[i],
                                                             0, 0xffff);
                                break;

                        case V3D_SAMPLER_STATE_16I:
                                for (int i = 0; i < 4; i++)
                                        border.i[i] = CLAMP(border.i[i],
                                                            -32768, 32767);
                                break;

                        case V3D_SAMPLER_STATE_8U:
                                for (int i = 0; i < 4; i++)
                                        border.ui[i] = CLAMP(border.ui[i],
                                                             0, 0xff);
                                break;

                        case V3D_SAMPLER_STATE_8I:
                                for (int i = 0; i < 4; i++)
                                        border.i[i] = CLAMP(border.i[i],
                                                            -128, 127);
                                break;

                        default:
                                break;
                        }

#if V3D_VERSION == 42
                        /* The TMU in V3D 7.x always takes 32-bit floats and handles conversions
                         * for us. In V3D 4.x we need to manually convert floating point color
                         * values to the expected format.
                         */
                        if (variant < V3D_SAMPLER_STATE_32) {
                                border.ui[0] = _mesa_float_to_half(border.f[0]);
                                border.ui[1] = _mesa_float_to_half(border.f[1]);
                                border.ui[2] = _mesa_float_to_half(border.f[2]);
                                border.ui[3] = _mesa_float_to_half(border.f[3]);
                        }
#endif
                        sampler.border_color_word_0 = border.ui[0];
                        sampler.border_color_word_1 = border.ui[1];
                        sampler.border_color_word_2 = border.ui[2];
                        sampler.border_color_word_3 = border.ui[3];
                }
        }
}

static void *
v3d_create_sampler_state(struct pipe_context *pctx,
                         const struct pipe_sampler_state *cso)
{
        UNUSED struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_sampler_state *so = CALLOC_STRUCT(v3d_sampler_state);

        if (!so)
                return NULL;

        memcpy(so, cso, sizeof(*cso));

        enum V3DX(Wrap_Mode) wrap_s = translate_wrap(cso->wrap_s);
        enum V3DX(Wrap_Mode) wrap_t = translate_wrap(cso->wrap_t);
        enum V3DX(Wrap_Mode) wrap_r = translate_wrap(cso->wrap_r);

        bool uses_border_color = (wrap_s == V3D_WRAP_MODE_BORDER ||
                                  wrap_t == V3D_WRAP_MODE_BORDER ||
                                  wrap_r == V3D_WRAP_MODE_BORDER);

        so->border_color_variants = false;

        /* This is the variant with the default hardware settings */
        enum v3d_sampler_state_variant border_variant = V3D_SAMPLER_STATE_BORDER_0000;

        if (uses_border_color) {
                if (cso->border_color.ui[0] == 0 &&
                    cso->border_color.ui[1] == 0 &&
                    cso->border_color.ui[2] == 0 &&
                    cso->border_color.ui[3] == 0) {
                        border_variant = V3D_SAMPLER_STATE_BORDER_0000;
                } else if (cso->border_color.ui[0] == 0 &&
                           cso->border_color.ui[1] == 0 &&
                           cso->border_color.ui[2] == 0 &&
                           cso->border_color.ui[3] == 0x3F800000) {
                        border_variant = V3D_SAMPLER_STATE_BORDER_0001;
                } else if (cso->border_color.ui[0] == 0x3F800000 &&
                           cso->border_color.ui[1] == 0x3F800000 &&
                           cso->border_color.ui[2] == 0x3F800000 &&
                           cso->border_color.ui[3] == 0x3F800000) {
                        border_variant = V3D_SAMPLER_STATE_BORDER_1111;
                } else {
                        so->border_color_variants = true;
                }
        }

        void *map;
        int sampler_align = so->border_color_variants ? 32 : 8;
        int sampler_size = align(cl_packet_length(SAMPLER_STATE), sampler_align);
        int num_variants = (so->border_color_variants ? ARRAY_SIZE(so->sampler_state_offset) : 1);
        u_upload_alloc(v3d->state_uploader, 0,
                       sampler_size * num_variants,
                       sampler_align,
                       &so->sampler_state_offset[0],
                       &so->sampler_state,
                       &map);

        for (int i = 0; i < num_variants; i++) {
                so->sampler_state_offset[i] =
                        so->sampler_state_offset[0] + i * sampler_size;
                v3d_upload_sampler_state_variant(map + i * sampler_size,
                                                 cso,
                                                 so->border_color_variants ? i : border_variant);
        }

        return so;
}

static void
v3d_sampler_states_bind(struct pipe_context *pctx,
                        enum pipe_shader_type shader, unsigned start,
                        unsigned nr, void **hwcso)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_texture_stateobj *stage_tex = &v3d->tex[shader];

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

        v3d_flag_dirty_sampler_state(v3d, shader);
}

static void
v3d_sampler_state_delete(struct pipe_context *pctx,
                         void *hwcso)
{
        struct pipe_sampler_state *psampler = hwcso;
        struct v3d_sampler_state *sampler = v3d_sampler_state(psampler);

        pipe_resource_reference(&sampler->sampler_state, NULL);
        free(psampler);
}

static void
v3d_setup_texture_shader_state_from_buffer(struct V3DX(TEXTURE_SHADER_STATE) *tex,
                                           struct pipe_resource *prsc,
                                           enum pipe_format format,
                                           unsigned offset,
                                           unsigned size)
{
        struct v3d_resource *rsc = v3d_resource(prsc);

        tex->image_depth = 1;
        tex->image_width = size / util_format_get_blocksize(format);

        /* On 4.x, the height of a 1D texture is redefined to be the
         * upper 14 bits of the width (which is only usable with txf).
         */
        tex->image_height = tex->image_width >> 14;

        tex->image_width &= (1 << 14) - 1;
        tex->image_height &= (1 << 14) - 1;

        /* Note that we don't have a job to reference the texture's sBO
         * at state create time, so any time this sampler view is used
         * we need to add the texture to the job.
         */
        tex->texture_base_pointer =
                cl_address(NULL, rsc->bo->offset + offset);
}

static void
v3d_setup_texture_shader_state(const struct v3d_device_info *devinfo,
                               struct V3DX(TEXTURE_SHADER_STATE) *tex,
                               struct pipe_resource *prsc,
                               int base_level, int last_level,
                               int first_layer, int last_layer,
                               bool sampling_cube_array)
{
        /* Due to ARB_texture_view, a cubemap array can be seen as 2D texture
         * array.
         */
        assert(!sampling_cube_array ||
               prsc->target == PIPE_TEXTURE_CUBE_ARRAY ||
               prsc->target == PIPE_TEXTURE_2D_ARRAY);

        struct v3d_resource *rsc = v3d_resource(prsc);
        int msaa_scale = prsc->nr_samples > 1 ? 2 : 1;

        tex->image_width = prsc->width0 * msaa_scale;
        tex->image_height = prsc->height0 * msaa_scale;

       /* On 4.x, the height of a 1D texture is redefined to be the
         * upper 14 bits of the width (which is only usable with txf).
         */
        if (prsc->target == PIPE_TEXTURE_1D ||
            prsc->target == PIPE_TEXTURE_1D_ARRAY) {
                tex->image_height = tex->image_width >> 14;
        }

        tex->image_width &= (1 << 14) - 1;
        tex->image_height &= (1 << 14) - 1;

        if (prsc->target == PIPE_TEXTURE_3D) {
                tex->image_depth = prsc->depth0;
        } else {
                tex->image_depth = (last_layer - first_layer) + 1;
        }

        /* Empirical testing with CTS shows that when we are sampling from
         * cube arrays we want to set image depth to layers / 6, but not when
         * doing image load/store or sampling from 2d image arrays.
         */
        if (sampling_cube_array) {
                assert(tex->image_depth % 6 == 0);
                tex->image_depth /= 6;
        }

        tex->base_level = base_level;

        tex->max_level = last_level;
        /* Note that we don't have a job to reference the texture's sBO
         * at state create time, so any time this sampler view is used
         * we need to add the texture to the job.
         */
        const uint32_t base_offset = rsc->bo->offset +
                v3d_layer_offset(prsc, 0, first_layer);

        tex->texture_base_pointer = cl_address(NULL, base_offset);
        tex->array_stride_64_byte_aligned = rsc->cube_map_stride / 64;

#if V3D_VERSION >= 71
        tex->chroma_offset_x = 1;
        tex->chroma_offset_y = 1;
        /* See comment in XML field definition for rationale of the shifts */
        tex->texture_base_pointer_cb = base_offset >> 6;
        tex->texture_base_pointer_cr = base_offset >> 6;
#endif

        /* Since other platform devices may produce UIF images even
         * when they're not big enough for V3D to assume they're UIF,
         * we force images with level 0 as UIF to be always treated
         * that way.
         */
        tex->level_0_is_strictly_uif =
                (rsc->slices[0].tiling == V3D_TILING_UIF_XOR ||
                 rsc->slices[0].tiling == V3D_TILING_UIF_NO_XOR);
        tex->level_0_xor_enable = (rsc->slices[0].tiling == V3D_TILING_UIF_XOR);

        if (tex->level_0_is_strictly_uif)
                tex->level_0_ub_pad = rsc->slices[0].ub_pad;

        if (tex->uif_xor_disable ||
            tex->level_0_is_strictly_uif) {
                tex->extended = true;
        }
}

void
v3dX(create_texture_shader_state_bo)(struct v3d_context *v3d,
                                     struct v3d_sampler_view *so)
{
        struct pipe_resource *prsc = so->texture;
        struct v3d_resource *rsc = v3d_resource(prsc);
        const struct pipe_sampler_view *cso = &so->base;
        struct v3d_screen *screen = v3d->screen;

        void *map;

        assert(so->serial_id != rsc->serial_id);

        v3d_bo_unreference(&so->bo);
        so->bo = v3d_bo_alloc(v3d->screen,
                              cl_packet_length(TEXTURE_SHADER_STATE), "sampler");
        map = v3d_bo_map(so->bo);

        v3dx_pack(map, TEXTURE_SHADER_STATE, tex) {
                if (prsc->target != PIPE_BUFFER) {
                        v3d_setup_texture_shader_state(&v3d->screen->devinfo,
                                                       &tex, prsc,
                                                       cso->u.tex.first_level,
                                                       cso->u.tex.last_level,
                                                       cso->u.tex.first_layer,
                                                       cso->u.tex.last_layer,
                                                       cso->target == PIPE_TEXTURE_CUBE_ARRAY);
                } else {
                        v3d_setup_texture_shader_state_from_buffer(&tex, prsc,
                                                                   cso->format,
                                                                   cso->u.buf.offset,
                                                                   cso->u.buf.size);
                }

                bool is_srgb = util_format_is_srgb(cso->format);
#if V3D_VERSION == 42
                tex.srgb = is_srgb;
#endif
#if V3D_VERSION >= 71
                tex.transfer_func = is_srgb ? TRANSFER_FUNC_SRGB : TRANSFER_FUNC_NONE;
#endif

                tex.swizzle_r = v3d_translate_pipe_swizzle(so->swizzle[0]);
                tex.swizzle_g = v3d_translate_pipe_swizzle(so->swizzle[1]);
                tex.swizzle_b = v3d_translate_pipe_swizzle(so->swizzle[2]);
                tex.swizzle_a = v3d_translate_pipe_swizzle(so->swizzle[3]);

                tex.texture_type = v3d_get_tex_format(&screen->devinfo,
                                                      cso->format);
        };

        so->serial_id = rsc->serial_id;
}

static struct pipe_sampler_view *
v3d_create_sampler_view(struct pipe_context *pctx, struct pipe_resource *prsc,
                        const struct pipe_sampler_view *cso)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_screen *screen = v3d->screen;
        struct v3d_sampler_view *so = CALLOC_STRUCT(v3d_sampler_view);
        struct v3d_resource *rsc = v3d_resource(prsc);

        if (!so)
                return NULL;

        so->base = *cso;

        pipe_reference(NULL, &prsc->reference);

        /* Compute the sampler view's swizzle up front. This will be plugged
         * into either the sampler (for 16-bit returns) or the shader's
         * texture key (for 32)
         */
        uint8_t view_swizzle[4] = {
                cso->swizzle_r,
                cso->swizzle_g,
                cso->swizzle_b,
                cso->swizzle_a
        };
        const uint8_t *fmt_swizzle =
                v3d_get_format_swizzle(&screen->devinfo, so->base.format);
        util_format_compose_swizzles(fmt_swizzle, view_swizzle, so->swizzle);

        pipe_reference_init(&so->base.reference, 1);
        so->base.texture = prsc;
        so->base.context = pctx;

        if (rsc->separate_stencil &&
            cso->format == PIPE_FORMAT_X32_S8X24_UINT) {
                rsc = rsc->separate_stencil;
                prsc = &rsc->base;
        }

        /* If we're sampling depth from depth/stencil, demote the format to
         * just depth.  u_format will end up giving the answers for the
         * stencil channel, otherwise.
         */
        enum pipe_format sample_format = cso->format;
        if (sample_format == PIPE_FORMAT_S8_UINT_Z24_UNORM)
                sample_format = PIPE_FORMAT_X8Z24_UNORM;

        const struct util_format_description *desc =
                util_format_description(sample_format);

        if (util_format_is_pure_integer(sample_format) &&
            !util_format_has_depth(desc)) {
                int chan = util_format_get_first_non_void_channel(sample_format);
                if (util_format_is_pure_uint(sample_format)) {
                        switch (desc->channel[chan].size) {
                        case 32:
                                so->sampler_variant = V3D_SAMPLER_STATE_32;
                                break;
                        case 16:
                                so->sampler_variant = V3D_SAMPLER_STATE_16U;
                                break;
                        case 10:
                                so->sampler_variant = V3D_SAMPLER_STATE_1010102U;
                                break;
                        case 8:
                                so->sampler_variant = V3D_SAMPLER_STATE_8U;
                                break;
                        }
                } else {
                        switch (desc->channel[chan].size) {
                        case 32:
                                so->sampler_variant = V3D_SAMPLER_STATE_32;
                                break;
                        case 16:
                                so->sampler_variant = V3D_SAMPLER_STATE_16I;
                                break;
                        case 8:
                                so->sampler_variant = V3D_SAMPLER_STATE_8I;
                                break;
                        }
                }
        } else {
                if (v3d_get_tex_return_size(&screen->devinfo, sample_format) == 32) {
                        if (util_format_is_alpha(sample_format))
                                so->sampler_variant = V3D_SAMPLER_STATE_32_A;
                        else
                                so->sampler_variant = V3D_SAMPLER_STATE_32;
                } else {
                        if (util_format_is_luminance_alpha(sample_format))
                                so->sampler_variant = V3D_SAMPLER_STATE_F16_LA;
                        else if (util_format_is_alpha(sample_format))
                                so->sampler_variant = V3D_SAMPLER_STATE_F16_A;
                        else if (fmt_swizzle[0] == PIPE_SWIZZLE_Z)
                                so->sampler_variant = V3D_SAMPLER_STATE_F16_BGRA;
                        else
                                so->sampler_variant = V3D_SAMPLER_STATE_F16;

                }

                if (util_format_is_unorm(sample_format)) {
                        so->sampler_variant += (V3D_SAMPLER_STATE_F16_UNORM -
                                                V3D_SAMPLER_STATE_F16);
                } else if (util_format_is_snorm(sample_format)){
                        so->sampler_variant += (V3D_SAMPLER_STATE_F16_SNORM -
                                                V3D_SAMPLER_STATE_F16);
                }
        }

        /* V3D still doesn't support sampling from raster textures, so we will
         * have to copy to a temporary tiled texture.
         */
        if (!rsc->tiled && !(prsc->target == PIPE_TEXTURE_1D ||
                             prsc->target == PIPE_TEXTURE_1D_ARRAY ||
                             prsc->target == PIPE_BUFFER)) {
                struct v3d_resource *shadow_parent = rsc;
                struct pipe_resource tmpl = {
                        .target = prsc->target,
                        .format = prsc->format,
                        .width0 = u_minify(prsc->width0,
                                           cso->u.tex.first_level),
                        .height0 = u_minify(prsc->height0,
                                            cso->u.tex.first_level),
                        .depth0 = 1,
                        .array_size = 1,
                        .bind = PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET,
                        .last_level = cso->u.tex.last_level - cso->u.tex.first_level,
                        .nr_samples = prsc->nr_samples,
                };

                /* Create the shadow texture.  The rest of the sampler view
                 * setup will use the shadow.
                 */
                prsc = v3d_resource_create(pctx->screen, &tmpl);
                if (!prsc) {
                        free(so);
                        return NULL;
                }
                rsc = v3d_resource(prsc);

                /* Flag it as needing update of the contents from the parent. */
                rsc->writes = shadow_parent->writes - 1;
                assert(rsc->tiled);

                so->texture = prsc;
        } else {
                pipe_resource_reference(&so->texture, prsc);
        }

        v3dX(create_texture_shader_state_bo)(v3d, so);

        return &so->base;
}

static void
v3d_sampler_view_destroy(struct pipe_context *pctx,
                         struct pipe_sampler_view *psview)
{
        struct v3d_sampler_view *sview = v3d_sampler_view(psview);

        v3d_bo_unreference(&sview->bo);
        pipe_resource_reference(&psview->texture, NULL);
        pipe_resource_reference(&sview->texture, NULL);
        free(psview);
}

static void
v3d_set_sampler_views(struct pipe_context *pctx,
                      enum pipe_shader_type shader,
                      unsigned start, unsigned nr,
                      unsigned unbind_num_trailing_slots,
                      bool take_ownership,
                      struct pipe_sampler_view **views)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_texture_stateobj *stage_tex = &v3d->tex[shader];
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
                /* If our sampler serial doesn't match our texture serial it
                 * means the texture has been updated with a new BO, in which
                 * case we need to update the sampler state to point to the
                 * new BO as well
                 */
                if (stage_tex->textures[i]) {
                        struct v3d_sampler_view *so =
                                v3d_sampler_view(stage_tex->textures[i]);
                        struct v3d_resource *rsc = v3d_resource(so->texture);
                        if (so->serial_id != rsc->serial_id)
                                v3dX(create_texture_shader_state_bo)(v3d, so);
                }
        }

        for (; i < stage_tex->num_textures; i++) {
                pipe_sampler_view_reference(&stage_tex->textures[i], NULL);
        }

        stage_tex->num_textures = new_nr;

        v3d_flag_dirty_sampler_state(v3d, shader);
}

static struct pipe_stream_output_target *
v3d_create_stream_output_target(struct pipe_context *pctx,
                                struct pipe_resource *prsc,
                                unsigned buffer_offset,
                                unsigned buffer_size)
{
        struct v3d_stream_output_target *target;

        target = CALLOC_STRUCT(v3d_stream_output_target);
        if (!target)
                return NULL;

        pipe_reference_init(&target->base.reference, 1);
        pipe_resource_reference(&target->base.buffer, prsc);

        target->base.context = pctx;
        target->base.buffer_offset = buffer_offset;
        target->base.buffer_size = buffer_size;

        return &target->base;
}

static void
v3d_stream_output_target_destroy(struct pipe_context *pctx,
                                 struct pipe_stream_output_target *target)
{
        pipe_resource_reference(&target->buffer, NULL);
        free(target);
}

static void
v3d_set_stream_output_targets(struct pipe_context *pctx,
                              unsigned num_targets,
                              struct pipe_stream_output_target **targets,
                              const unsigned *offsets)
{
        struct v3d_context *ctx = v3d_context(pctx);
        struct v3d_streamout_stateobj *so = &ctx->streamout;
        unsigned i;

        assert(num_targets <= ARRAY_SIZE(so->targets));

        /* Update recorded vertex counts when we are ending the recording of
         * transform feedback. We do this when we switch primitive types
         * at draw time, but if we haven't switched primitives in our last
         * draw we need to do it here as well.
         */
        if (num_targets == 0 && so->num_targets > 0)
                v3d_update_primitive_counters(ctx);

        /* If offset is (unsigned) -1, it means continue appending to the
         * buffer at the existing offset.
         */
        for (i = 0; i < num_targets; i++) {
                if (offsets[i] != (unsigned)-1)
                        v3d_stream_output_target(targets[i])->offset = offsets[i];

                pipe_so_target_reference(&so->targets[i], targets[i]);
        }

        for (; i < so->num_targets; i++)
                pipe_so_target_reference(&so->targets[i], NULL);

        so->num_targets = num_targets;

        /* Create primitive counters BO if needed */
        if (num_targets > 0)
                v3d_ensure_prim_counts_allocated(ctx);

        ctx->dirty |= V3D_DIRTY_STREAMOUT;
}

static void
v3d_set_shader_buffers(struct pipe_context *pctx,
                       enum pipe_shader_type shader,
                       unsigned start, unsigned count,
                       const struct pipe_shader_buffer *buffers,
                       unsigned writable_bitmask)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_ssbo_stateobj *so = &v3d->ssbo[shader];
        unsigned mask = 0;

        if (buffers) {
                for (unsigned i = 0; i < count; i++) {
                        unsigned n = i + start;
                        struct pipe_shader_buffer *buf = &so->sb[n];

                        if ((buf->buffer == buffers[i].buffer) &&
                            (buf->buffer_offset == buffers[i].buffer_offset) &&
                            (buf->buffer_size == buffers[i].buffer_size))
                                continue;

                        mask |= 1 << n;

                        buf->buffer_offset = buffers[i].buffer_offset;
                        buf->buffer_size = buffers[i].buffer_size;
                        pipe_resource_reference(&buf->buffer, buffers[i].buffer);

                        if (buf->buffer)
                                so->enabled_mask |= 1 << n;
                        else
                                so->enabled_mask &= ~(1 << n);
                }
        } else {
                mask = ((1 << count) - 1) << start;

                for (unsigned i = 0; i < count; i++) {
                        unsigned n = i + start;
                        struct pipe_shader_buffer *buf = &so->sb[n];

                        pipe_resource_reference(&buf->buffer, NULL);
                }

                so->enabled_mask &= ~mask;
        }

        v3d->dirty |= V3D_DIRTY_SSBO;
}

static void
v3d_create_image_view_texture_shader_state(struct v3d_context *v3d,
                                           struct v3d_shaderimg_stateobj *so,
                                           int img)
{
        struct v3d_image_view *iview = &so->si[img];

        void *map;
        u_upload_alloc(v3d->uploader, 0, cl_packet_length(TEXTURE_SHADER_STATE),
                       32,
                       &iview->tex_state_offset,
                       &iview->tex_state,
                       &map);

        struct pipe_resource *prsc = iview->base.resource;

        v3dx_pack(map, TEXTURE_SHADER_STATE, tex) {
                if (prsc->target != PIPE_BUFFER) {
                        v3d_setup_texture_shader_state(&v3d->screen->devinfo,
                                                       &tex, prsc,
                                                       iview->base.u.tex.level,
                                                       iview->base.u.tex.level,
                                                       iview->base.u.tex.first_layer,
                                                       iview->base.u.tex.last_layer,
                                                       false);
                } else {
                        v3d_setup_texture_shader_state_from_buffer(&tex, prsc,
                                                                   iview->base.format,
                                                                   iview->base.u.buf.offset,
                                                                   iview->base.u.buf.size);
                }

                tex.swizzle_r = v3d_translate_pipe_swizzle(PIPE_SWIZZLE_X);
                tex.swizzle_g = v3d_translate_pipe_swizzle(PIPE_SWIZZLE_Y);
                tex.swizzle_b = v3d_translate_pipe_swizzle(PIPE_SWIZZLE_Z);
                tex.swizzle_a = v3d_translate_pipe_swizzle(PIPE_SWIZZLE_W);

                tex.texture_type = v3d_get_tex_format(&v3d->screen->devinfo,
                                                      iview->base.format);
        };
}

static void
v3d_set_shader_images(struct pipe_context *pctx,
                      enum pipe_shader_type shader,
                      unsigned start, unsigned count,
                      unsigned unbind_num_trailing_slots,
                      const struct pipe_image_view *images)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_shaderimg_stateobj *so = &v3d->shaderimg[shader];

        if (images) {
                for (unsigned i = 0; i < count; i++) {
                        unsigned n = i + start;
                        struct v3d_image_view *iview = &so->si[n];

                        if ((iview->base.resource == images[i].resource) &&
                            (iview->base.format == images[i].format) &&
                            (iview->base.access == images[i].access) &&
                            !memcmp(&iview->base.u, &images[i].u,
                                    sizeof(iview->base.u)))
                                continue;

                        util_copy_image_view(&iview->base, &images[i]);

                        if (iview->base.resource) {
                                so->enabled_mask |= 1 << n;
                                v3d_create_image_view_texture_shader_state(v3d,
                                                                           so,
                                                                           n);
                        } else {
                                so->enabled_mask &= ~(1 << n);
                                pipe_resource_reference(&iview->tex_state, NULL);
                        }
                }
        } else {
                for (unsigned i = 0; i < count; i++) {
                        unsigned n = i + start;
                        struct v3d_image_view *iview = &so->si[n];

                        pipe_resource_reference(&iview->base.resource, NULL);
                        pipe_resource_reference(&iview->tex_state, NULL);
                }

                if (count == 32)
                        so->enabled_mask = 0;
                else
                        so->enabled_mask &= ~(((1 << count) - 1) << start);
        }

        v3d->dirty |= V3D_DIRTY_SHADER_IMAGE;

        if (unbind_num_trailing_slots) {
                v3d_set_shader_images(pctx, shader, start + count,
                                      unbind_num_trailing_slots, 0, NULL);
        }
}

void
v3dX(state_init)(struct pipe_context *pctx)
{
        pctx->set_blend_color = v3d_set_blend_color;
        pctx->set_stencil_ref = v3d_set_stencil_ref;
        pctx->set_clip_state = v3d_set_clip_state;
        pctx->set_sample_mask = v3d_set_sample_mask;
        pctx->set_constant_buffer = v3d_set_constant_buffer;
        pctx->set_framebuffer_state = v3d_set_framebuffer_state;
        pctx->set_polygon_stipple = v3d_set_polygon_stipple;
        pctx->set_scissor_states = v3d_set_scissor_states;
        pctx->set_viewport_states = v3d_set_viewport_states;

        pctx->set_vertex_buffers = v3d_set_vertex_buffers;

        pctx->create_blend_state = v3d_create_blend_state;
        pctx->bind_blend_state = v3d_blend_state_bind;
        pctx->delete_blend_state = v3d_generic_cso_state_delete;

        pctx->create_rasterizer_state = v3d_create_rasterizer_state;
        pctx->bind_rasterizer_state = v3d_rasterizer_state_bind;
        pctx->delete_rasterizer_state = v3d_generic_cso_state_delete;

        pctx->create_depth_stencil_alpha_state = v3d_create_depth_stencil_alpha_state;
        pctx->bind_depth_stencil_alpha_state = v3d_zsa_state_bind;
        pctx->delete_depth_stencil_alpha_state = v3d_generic_cso_state_delete;

        pctx->create_vertex_elements_state = v3d_vertex_state_create;
        pctx->delete_vertex_elements_state = v3d_vertex_state_delete;
        pctx->bind_vertex_elements_state = v3d_vertex_state_bind;

        pctx->create_sampler_state = v3d_create_sampler_state;
        pctx->delete_sampler_state = v3d_sampler_state_delete;
        pctx->bind_sampler_states = v3d_sampler_states_bind;

        pctx->create_sampler_view = v3d_create_sampler_view;
        pctx->sampler_view_destroy = v3d_sampler_view_destroy;
        pctx->set_sampler_views = v3d_set_sampler_views;

        pctx->set_shader_buffers = v3d_set_shader_buffers;
        pctx->set_shader_images = v3d_set_shader_images;

        pctx->create_stream_output_target = v3d_create_stream_output_target;
        pctx->stream_output_target_destroy = v3d_stream_output_target_destroy;
        pctx->set_stream_output_targets = v3d_set_stream_output_targets;
}
