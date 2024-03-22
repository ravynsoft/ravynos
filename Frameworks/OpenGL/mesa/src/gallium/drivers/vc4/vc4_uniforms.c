/*
 * Copyright © 2014-2015 Broadcom
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

#include "util/u_pack_color.h"
#include "util/u_upload_mgr.h"
#include "util/format_srgb.h"

#include "vc4_context.h"
#include "vc4_qir.h"

static void
write_texture_p0(struct vc4_job *job,
                 struct vc4_cl_out **uniforms,
                 struct vc4_texture_stateobj *texstate,
                 uint32_t unit)
{
        struct vc4_sampler_view *sview =
                vc4_sampler_view(texstate->textures[unit]);
        struct vc4_resource *rsc = vc4_resource(sview->texture);

        cl_reloc(job, &job->uniforms, uniforms, rsc->bo, sview->texture_p0);
}

static void
write_texture_p1(struct vc4_job *job,
                 struct vc4_cl_out **uniforms,
                 struct vc4_texture_stateobj *texstate,
                 uint32_t unit)
{
        struct vc4_sampler_view *sview =
                vc4_sampler_view(texstate->textures[unit]);
        struct vc4_sampler_state *sampler =
                vc4_sampler_state(texstate->samplers[unit]);

        cl_aligned_u32(uniforms, sview->texture_p1 | sampler->texture_p1);
}

static void
write_texture_p2(struct vc4_job *job,
                 struct vc4_cl_out **uniforms,
                 struct vc4_texture_stateobj *texstate,
                 uint32_t data)
{
        uint32_t unit = data & 0xffff;
        struct pipe_sampler_view *texture = texstate->textures[unit];
        struct vc4_resource *rsc = vc4_resource(texture->texture);

        cl_aligned_u32(uniforms,
               VC4_SET_FIELD(VC4_TEX_P2_PTYPE_CUBE_MAP_STRIDE,
                             VC4_TEX_P2_PTYPE) |
               VC4_SET_FIELD(rsc->cube_map_stride >> 12, VC4_TEX_P2_CMST) |
               VC4_SET_FIELD((data >> 16) & 1, VC4_TEX_P2_BSLOD));
}

static void
write_texture_first_level(struct vc4_job *job,
                          struct vc4_cl_out **uniforms,
                          struct vc4_texture_stateobj *texstate,
                          uint32_t data)
{
        uint32_t unit = data & 0xffff;
        struct pipe_sampler_view *texture = texstate->textures[unit];

        cl_aligned_f(uniforms, texture->u.tex.first_level);
}

static void
write_texture_msaa_addr(struct vc4_job *job,
                 struct vc4_cl_out **uniforms,
                        struct vc4_texture_stateobj *texstate,
                        uint32_t unit)
{
        struct pipe_sampler_view *texture = texstate->textures[unit];
        struct vc4_resource *rsc = vc4_resource(texture->texture);

        cl_aligned_reloc(job, &job->uniforms, uniforms, rsc->bo, 0);
}


#define SWIZ(x,y,z,w) {          \
        PIPE_SWIZZLE_##x, \
        PIPE_SWIZZLE_##y, \
        PIPE_SWIZZLE_##z, \
        PIPE_SWIZZLE_##w  \
}

static void
write_texture_border_color(struct vc4_job *job,
                           struct vc4_cl_out **uniforms,
                           struct vc4_texture_stateobj *texstate,
                           uint32_t unit)
{
        struct pipe_sampler_state *sampler = texstate->samplers[unit];
        struct pipe_sampler_view *texture = texstate->textures[unit];
        struct vc4_resource *rsc = vc4_resource(texture->texture);
        union util_color uc;

        const struct util_format_description *tex_format_desc =
                util_format_description(texture->format);

        float border_color[4];
        for (int i = 0; i < 4; i++)
                border_color[i] = sampler->border_color.f[i];
        if (util_format_is_srgb(texture->format)) {
                for (int i = 0; i < 3; i++)
                        border_color[i] =
                                util_format_linear_to_srgb_float(border_color[i]);
        }

        /* Turn the border color into the layout of channels that it would
         * have when stored as texture contents.
         */
        float storage_color[4];
        util_format_unswizzle_4f(storage_color,
                                 border_color,
                                 tex_format_desc->swizzle);

        /* Now, pack so that when the vc4_format-sampled texture contents are
         * replaced with our border color, the vc4_get_format_swizzle()
         * swizzling will get the right channels.
         */
        if (util_format_is_depth_or_stencil(texture->format)) {
                uc.ui[0] = util_pack_z(PIPE_FORMAT_Z24X8_UNORM,
                                       sampler->border_color.f[0]) << 8;
        } else {
                switch (rsc->vc4_format) {
                default:
                case VC4_TEXTURE_TYPE_RGBA8888:
                        util_pack_color(storage_color,
                                        PIPE_FORMAT_R8G8B8A8_UNORM, &uc);
                        break;
                case VC4_TEXTURE_TYPE_RGBA4444:
                case VC4_TEXTURE_TYPE_RGBA5551:
                        util_pack_color(storage_color,
                                        PIPE_FORMAT_A8B8G8R8_UNORM, &uc);
                        break;
                case VC4_TEXTURE_TYPE_RGB565:
                        util_pack_color(storage_color,
                                        PIPE_FORMAT_B8G8R8A8_UNORM, &uc);
                        break;
                case VC4_TEXTURE_TYPE_ALPHA:
                        uc.ui[0] = float_to_ubyte(storage_color[0]) << 24;
                        break;
                case VC4_TEXTURE_TYPE_LUMALPHA:
                        uc.ui[0] = ((float_to_ubyte(storage_color[1]) << 24) |
                                    (float_to_ubyte(storage_color[0]) << 0));
                        break;
                }
        }

        cl_aligned_u32(uniforms, uc.ui[0]);
}

static uint32_t
get_texrect_scale(struct vc4_texture_stateobj *texstate,
                  enum quniform_contents contents,
                  uint32_t data)
{
        struct pipe_sampler_view *texture = texstate->textures[data];
        uint32_t dim;

        if (contents == QUNIFORM_TEXRECT_SCALE_X)
                dim = texture->texture->width0;
        else
                dim = texture->texture->height0;

        return fui(1.0f / dim);
}

void
vc4_write_uniforms(struct vc4_context *vc4, struct vc4_compiled_shader *shader,
                   struct vc4_constbuf_stateobj *cb,
                   struct vc4_texture_stateobj *texstate)
{
        struct vc4_shader_uniform_info *uinfo = &shader->uniforms;
        struct vc4_job *job = vc4->job;
        const uint32_t *gallium_uniforms = cb->cb[0].user_buffer;

        cl_ensure_space(&job->uniforms, (uinfo->count +
                                         uinfo->num_texture_samples) * 4);

        struct vc4_cl_out *uniforms =
                cl_start_shader_reloc(&job->uniforms,
                                      uinfo->num_texture_samples);

        for (int i = 0; i < uinfo->count; i++) {
                enum quniform_contents contents = uinfo->contents[i];
                uint32_t data = uinfo->data[i];

                switch (contents) {
                case QUNIFORM_CONSTANT:
                        cl_aligned_u32(&uniforms, data);
                        break;
                case QUNIFORM_UNIFORM:
                        cl_aligned_u32(&uniforms,
                                       gallium_uniforms[data]);
                        break;
                case QUNIFORM_VIEWPORT_X_SCALE:
                        cl_aligned_f(&uniforms, vc4->viewport.scale[0] * 16.0f);
                        break;
                case QUNIFORM_VIEWPORT_Y_SCALE:
                        cl_aligned_f(&uniforms, vc4->viewport.scale[1] * 16.0f);
                        break;

                case QUNIFORM_VIEWPORT_Z_OFFSET:
                        cl_aligned_f(&uniforms, vc4->viewport.translate[2]);
                        break;
                case QUNIFORM_VIEWPORT_Z_SCALE:
                        cl_aligned_f(&uniforms, vc4->viewport.scale[2]);
                        break;

                case QUNIFORM_USER_CLIP_PLANE:
                        cl_aligned_f(&uniforms,
                                     vc4->clip.ucp[data / 4][data % 4]);
                        break;

                case QUNIFORM_TEXTURE_CONFIG_P0:
                        write_texture_p0(job, &uniforms, texstate, data);
                        break;

                case QUNIFORM_TEXTURE_CONFIG_P1:
                        write_texture_p1(job, &uniforms, texstate, data);
                        break;

                case QUNIFORM_TEXTURE_CONFIG_P2:
                        write_texture_p2(job, &uniforms, texstate, data);
                        break;

                case QUNIFORM_TEXTURE_FIRST_LEVEL:
                        write_texture_first_level(job, &uniforms, texstate,
                                                  data);
                        break;

                case QUNIFORM_UBO0_ADDR:
                        /* Constant buffer 0 may be a system memory pointer,
                         * in which case we want to upload a shadow copy to
                         * the GPU.
                        */
                        if (!cb->cb[0].buffer) {
                                u_upload_data(vc4->uploader, 0,
                                              cb->cb[0].buffer_size, 16,
                                              cb->cb[0].user_buffer,
                                              &cb->cb[0].buffer_offset,
                                              &cb->cb[0].buffer);
                        }

                        cl_aligned_reloc(job, &job->uniforms,
                                         &uniforms,
                                         vc4_resource(cb->cb[0].buffer)->bo,
                                         cb->cb[0].buffer_offset +
                                         data);
                        break;

                case QUNIFORM_UBO1_ADDR: {
                        struct vc4_resource *rsc =
                                vc4_resource(cb->cb[1].buffer);

                        cl_aligned_reloc(job, &job->uniforms,
                                         &uniforms,
                                         rsc->bo, cb->cb[1].buffer_offset);
                        break;
                }

                case QUNIFORM_TEXTURE_MSAA_ADDR:
                        write_texture_msaa_addr(job, &uniforms, texstate, data);
                        break;

                case QUNIFORM_TEXTURE_BORDER_COLOR:
                        write_texture_border_color(job, &uniforms,
                                                   texstate, data);
                        break;

                case QUNIFORM_TEXRECT_SCALE_X:
                case QUNIFORM_TEXRECT_SCALE_Y:
                        cl_aligned_u32(&uniforms,
                                       get_texrect_scale(texstate,
                                                         uinfo->contents[i],
                                                         data));
                        break;

                case QUNIFORM_BLEND_CONST_COLOR_X:
                case QUNIFORM_BLEND_CONST_COLOR_Y:
                case QUNIFORM_BLEND_CONST_COLOR_Z:
                case QUNIFORM_BLEND_CONST_COLOR_W:
                        cl_aligned_f(&uniforms,
                                     CLAMP(vc4->blend_color.f.color[uinfo->contents[i] -
                                                                    QUNIFORM_BLEND_CONST_COLOR_X],
                                           0, 1));
                        break;

                case QUNIFORM_BLEND_CONST_COLOR_RGBA: {
                        const uint8_t *format_swiz =
                                vc4_get_format_swizzle(vc4->framebuffer.cbufs[0]->format);
                        uint32_t color = 0;
                        for (int i = 0; i < 4; i++) {
                                if (format_swiz[i] >= 4)
                                        continue;

                                color |= (vc4->blend_color.ub[format_swiz[i]] <<
                                          (i * 8));
                        }
                        cl_aligned_u32(&uniforms, color);
                        break;
                }

                case QUNIFORM_BLEND_CONST_COLOR_AAAA: {
                        uint8_t a = vc4->blend_color.ub[3];
                        cl_aligned_u32(&uniforms, ((a) |
                                                   (a << 8) |
                                                   (a << 16) |
                                                   (a << 24)));
                        break;
                }

                case QUNIFORM_STENCIL:
                        cl_aligned_u32(&uniforms,
                                       vc4->zsa->stencil_uniforms[data] |
                                       (data <= 1 ?
                                        (vc4->stencil_ref.ref_value[data] << 8) :
                                        0));
                        break;

                case QUNIFORM_SAMPLE_MASK:
                        cl_aligned_u32(&uniforms, vc4->sample_mask);
                        break;

                case QUNIFORM_UNIFORMS_ADDRESS:
                        /* This will be filled in by the kernel. */
                        cl_aligned_u32(&uniforms, 0xd0d0d0d0);
                        break;
                }

                if (false) {
                        uint32_t written_val = *((uint32_t *)uniforms - 1);
                        char *desc = qir_describe_uniform(uinfo->contents[i],
                                                          uinfo->data[i],
                                                          gallium_uniforms);

                        fprintf(stderr, "%p/%d: 0x%08x %s\n",
                                shader, i, written_val, desc);

                        ralloc_free(desc);
                }
        }

        cl_end(&job->uniforms, uniforms);
}

void
vc4_set_shader_uniform_dirty_flags(struct vc4_compiled_shader *shader)
{
        uint32_t dirty = 0;

        for (int i = 0; i < shader->uniforms.count; i++) {
                switch (shader->uniforms.contents[i]) {
                case QUNIFORM_CONSTANT:
                case QUNIFORM_UNIFORMS_ADDRESS:
                        break;
                case QUNIFORM_UNIFORM:
                case QUNIFORM_UBO0_ADDR:
                case QUNIFORM_UBO1_ADDR:
                        dirty |= VC4_DIRTY_CONSTBUF;
                        break;

                case QUNIFORM_VIEWPORT_X_SCALE:
                case QUNIFORM_VIEWPORT_Y_SCALE:
                case QUNIFORM_VIEWPORT_Z_OFFSET:
                case QUNIFORM_VIEWPORT_Z_SCALE:
                        dirty |= VC4_DIRTY_VIEWPORT;
                        break;

                case QUNIFORM_USER_CLIP_PLANE:
                        dirty |= VC4_DIRTY_CLIP;
                        break;

                case QUNIFORM_TEXTURE_CONFIG_P0:
                case QUNIFORM_TEXTURE_CONFIG_P1:
                case QUNIFORM_TEXTURE_CONFIG_P2:
                case QUNIFORM_TEXTURE_BORDER_COLOR:
                case QUNIFORM_TEXTURE_FIRST_LEVEL:
                case QUNIFORM_TEXTURE_MSAA_ADDR:
                case QUNIFORM_TEXRECT_SCALE_X:
                case QUNIFORM_TEXRECT_SCALE_Y:
                        /* We could flag this on just the stage we're
                         * compiling for, but it's not passed in.
                         */
                        dirty |= VC4_DIRTY_FRAGTEX | VC4_DIRTY_VERTTEX;
                        break;

                case QUNIFORM_BLEND_CONST_COLOR_X:
                case QUNIFORM_BLEND_CONST_COLOR_Y:
                case QUNIFORM_BLEND_CONST_COLOR_Z:
                case QUNIFORM_BLEND_CONST_COLOR_W:
                case QUNIFORM_BLEND_CONST_COLOR_RGBA:
                case QUNIFORM_BLEND_CONST_COLOR_AAAA:
                        dirty |= VC4_DIRTY_BLEND_COLOR;
                        break;

                case QUNIFORM_STENCIL:
                        dirty |= VC4_DIRTY_ZSA;
                        break;

                case QUNIFORM_SAMPLE_MASK:
                        dirty |= VC4_DIRTY_SAMPLE_MASK;
                        break;
                }
        }

        shader->uniform_dirty_bits = dirty;
}
