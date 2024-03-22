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

#include "util/u_pack_color.h"
#include "util/u_upload_mgr.h"

#include "v3d_context.h"
#include "compiler/v3d_compiler.h"

/* We don't expect that the packets we use in this file change across across
 * hw versions, so we just include directly the v42 header
 */
#include "broadcom/cle/v3d_packet_v42_pack.h"

static uint32_t
get_texrect_scale(struct v3d_texture_stateobj *texstate,
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

static uint32_t
get_texture_size(struct v3d_texture_stateobj *texstate,
                 enum quniform_contents contents,
                 uint32_t data)
{
        struct pipe_sampler_view *texture = texstate->textures[data];
        switch (contents) {
        case QUNIFORM_TEXTURE_WIDTH:
                if (texture->target == PIPE_BUFFER) {
                        return texture->u.buf.size /
                                util_format_get_blocksize(texture->format);
                } else {
                        return u_minify(texture->texture->width0,
                                        texture->u.tex.first_level);
                }
        case QUNIFORM_TEXTURE_HEIGHT:
                return u_minify(texture->texture->height0,
                                texture->u.tex.first_level);
        case QUNIFORM_TEXTURE_DEPTH:
                assert(texture->target != PIPE_BUFFER);
                return u_minify(texture->texture->depth0,
                                texture->u.tex.first_level);
        case QUNIFORM_TEXTURE_ARRAY_SIZE:
                assert(texture->target != PIPE_BUFFER);
                if (texture->target != PIPE_TEXTURE_CUBE_ARRAY) {
                        return texture->texture->array_size;
                } else {
                        assert(texture->texture->array_size % 6 == 0);
                        return texture->texture->array_size / 6;
                }
        case QUNIFORM_TEXTURE_LEVELS:
                assert(texture->target != PIPE_BUFFER);
                return (texture->u.tex.last_level -
                        texture->u.tex.first_level) + 1;
        default:
                unreachable("Bad texture size field");
        }
}

static uint32_t
get_image_size(struct v3d_shaderimg_stateobj *shaderimg,
               enum quniform_contents contents,
               uint32_t data)
{
        struct v3d_image_view *image = &shaderimg->si[data];

        switch (contents) {
        case QUNIFORM_IMAGE_WIDTH:
                if (image->base.resource->target == PIPE_BUFFER) {
                        return image->base.u.buf.size /
                                util_format_get_blocksize(image->base.format);
                } else {
                        return u_minify(image->base.resource->width0,
                                        image->base.u.tex.level);
                }
        case QUNIFORM_IMAGE_HEIGHT:
                assert(image->base.resource->target != PIPE_BUFFER);
                return u_minify(image->base.resource->height0,
                                image->base.u.tex.level);
        case QUNIFORM_IMAGE_DEPTH:
                assert(image->base.resource->target != PIPE_BUFFER);
                return u_minify(image->base.resource->depth0,
                                image->base.u.tex.level);
        case QUNIFORM_IMAGE_ARRAY_SIZE:
                assert(image->base.resource->target != PIPE_BUFFER);
                if (image->base.resource->target != PIPE_TEXTURE_CUBE_ARRAY) {
                        return image->base.resource->array_size;
                } else {
                        assert(image->base.resource->array_size % 6 == 0);
                        return image->base.resource->array_size / 6;
                }
        default:
                unreachable("Bad texture size field");
        }
}

/** Writes the V3D 4.x TMU configuration parameter 0. */
static void
write_tmu_p0(struct v3d_job *job,
             struct v3d_cl_out **uniforms,
             struct v3d_texture_stateobj *texstate,
             uint32_t data)
{
        int unit = v3d_unit_data_get_unit(data);
        struct pipe_sampler_view *psview = texstate->textures[unit];
        struct v3d_sampler_view *sview = v3d_sampler_view(psview);
        /* GL_OES_texture_buffer spec:
         *     "If no buffer object is bound to the buffer texture, the
         *      results of the texel access are undefined."
         *
         * This can be interpreted as allowing any result to come back, but
         * not terminate the program (and some tests interpret that).
         *
         * FIXME: just return is not a full valid solution, as it could still
         * try to get a wrong address for the shader state address. Perhaps we
         * would need to set up a BO with a "default texture state"
         */
        if (sview == NULL)
                return;

        struct v3d_resource *rsc = v3d_resource(sview->texture);

        cl_aligned_reloc(&job->indirect, uniforms, sview->bo,
                         v3d_unit_data_get_offset(data));
        v3d_job_add_bo(job, rsc->bo);
}

static void
write_image_tmu_p0(struct v3d_job *job,
                   struct v3d_cl_out **uniforms,
                   struct v3d_shaderimg_stateobj *img,
                   uint32_t data)
{
        /* Extract the image unit from the top bits, and the compiler's
         * packed p0 from the bottom.
         */
        uint32_t unit = data >> 24;
        uint32_t p0 = data & 0x00ffffff;

        struct v3d_image_view *iview = &img->si[unit];
        struct v3d_resource *rsc = v3d_resource(iview->base.resource);

        cl_aligned_reloc(&job->indirect, uniforms,
                         v3d_resource(iview->tex_state)->bo,
                         iview->tex_state_offset | p0);
        v3d_job_add_bo(job, rsc->bo);
}

/** Writes the V3D 4.x TMU configuration parameter 1. */
static void
write_tmu_p1(struct v3d_job *job,
             struct v3d_cl_out **uniforms,
             struct v3d_texture_stateobj *texstate,
             uint32_t data)
{
        uint32_t unit = v3d_unit_data_get_unit(data);
        struct pipe_sampler_state *psampler = texstate->samplers[unit];
        struct v3d_sampler_state *sampler = v3d_sampler_state(psampler);
        struct pipe_sampler_view *psview = texstate->textures[unit];
        struct v3d_sampler_view *sview = v3d_sampler_view(psview);
        int variant = 0;

        /* If we are being asked by the compiler to write parameter 1, then we
         * need that. So if we are at this point, we should expect to have a
         * sampler and psampler. As an additional assert, we can check that we
         * are not on a texel buffer case, as these don't have a sampler.
         */
        assert(psview->target != PIPE_BUFFER);
        assert(sampler);
        assert(psampler);

        if (sampler->border_color_variants)
                variant = sview->sampler_variant;

        cl_aligned_reloc(&job->indirect, uniforms,
                         v3d_resource(sampler->sampler_state)->bo,
                         sampler->sampler_state_offset[variant] |
                         v3d_unit_data_get_offset(data));
}

struct v3d_cl_reloc
v3d_write_uniforms(struct v3d_context *v3d, struct v3d_job *job,
                   struct v3d_compiled_shader *shader,
                   enum pipe_shader_type stage)
{
        struct v3d_device_info *devinfo = &v3d->screen->devinfo;
        struct v3d_constbuf_stateobj *cb = &v3d->constbuf[stage];
        struct v3d_texture_stateobj *texstate = &v3d->tex[stage];
        struct v3d_uniform_list *uinfo = &shader->prog_data.base->uniforms;
        const uint32_t *gallium_uniforms = cb->cb[0].user_buffer;

        /* The hardware always pre-fetches the next uniform (also when there
         * aren't any), so we always allocate space for an extra slot. This
         * fixes MMU exceptions reported since Linux kernel 5.4 when the
         * uniforms fill up the tail bytes of a page in the indirect
         * BO. In that scenario, when the hardware pre-fetches after reading
         * the last uniform it will read beyond the end of the page and trigger
         * the MMU exception.
         */
        v3d_cl_ensure_space(&job->indirect, (uinfo->count + 1) * 4, 4);

        struct v3d_cl_reloc uniform_stream = cl_get_address(&job->indirect);
        v3d_bo_reference(uniform_stream.bo);

        struct v3d_cl_out *uniforms =
                cl_start(&job->indirect);

        for (int i = 0; i < uinfo->count; i++) {
                uint32_t data = uinfo->data[i];

                switch (uinfo->contents[i]) {
                case QUNIFORM_CONSTANT:
                        cl_aligned_u32(&uniforms, data);
                        break;
                case QUNIFORM_UNIFORM:
                        cl_aligned_u32(&uniforms, gallium_uniforms[data]);
                        break;
                case QUNIFORM_VIEWPORT_X_SCALE: {
                        float clipper_xy_granularity = V3DV_X(devinfo, CLIPPER_XY_GRANULARITY);
                        cl_aligned_f(&uniforms, v3d->viewport.scale[0] * clipper_xy_granularity);
                        break;
                }
                case QUNIFORM_VIEWPORT_Y_SCALE: {
                        float clipper_xy_granularity = V3DV_X(devinfo, CLIPPER_XY_GRANULARITY);
                        cl_aligned_f(&uniforms, v3d->viewport.scale[1] * clipper_xy_granularity);
                        break;
                }
                case QUNIFORM_VIEWPORT_Z_OFFSET:
                        cl_aligned_f(&uniforms, v3d->viewport.translate[2]);
                        break;
                case QUNIFORM_VIEWPORT_Z_SCALE:
                        cl_aligned_f(&uniforms, v3d->viewport.scale[2]);
                        break;

                case QUNIFORM_USER_CLIP_PLANE:
                        cl_aligned_f(&uniforms,
                                     v3d->clip.ucp[data / 4][data % 4]);
                        break;

                case QUNIFORM_TMU_CONFIG_P0:
                        write_tmu_p0(job, &uniforms, texstate, data);
                        break;

                case QUNIFORM_TMU_CONFIG_P1:
                        write_tmu_p1(job, &uniforms, texstate, data);
                        break;

                case QUNIFORM_IMAGE_TMU_CONFIG_P0:
                        write_image_tmu_p0(job, &uniforms,
                                           &v3d->shaderimg[stage], data);
                        break;

                case QUNIFORM_TEXRECT_SCALE_X:
                case QUNIFORM_TEXRECT_SCALE_Y:
                        cl_aligned_u32(&uniforms,
                                       get_texrect_scale(texstate,
                                                         uinfo->contents[i],
                                                         data));
                        break;

                case QUNIFORM_TEXTURE_WIDTH:
                case QUNIFORM_TEXTURE_HEIGHT:
                case QUNIFORM_TEXTURE_DEPTH:
                case QUNIFORM_TEXTURE_ARRAY_SIZE:
                case QUNIFORM_TEXTURE_LEVELS:
                        cl_aligned_u32(&uniforms,
                                       get_texture_size(texstate,
                                                        uinfo->contents[i],
                                                        data));
                        break;

                case QUNIFORM_IMAGE_WIDTH:
                case QUNIFORM_IMAGE_HEIGHT:
                case QUNIFORM_IMAGE_DEPTH:
                case QUNIFORM_IMAGE_ARRAY_SIZE:
                        cl_aligned_u32(&uniforms,
                                       get_image_size(&v3d->shaderimg[stage],
                                                      uinfo->contents[i],
                                                      data));
                        break;

                case QUNIFORM_LINE_WIDTH:
                        cl_aligned_f(&uniforms,
                                     v3d->rasterizer->base.line_width);
                        break;

                case QUNIFORM_AA_LINE_WIDTH:
                        cl_aligned_f(&uniforms, v3d_get_real_line_width(v3d));
                        break;

                case QUNIFORM_UBO_ADDR: {
                        uint32_t unit = v3d_unit_data_get_unit(data);
                        /* Constant buffer 0 may be a system memory pointer,
                         * in which case we want to upload a shadow copy to
                         * the GPU.
                        */
                        if (!cb->cb[unit].buffer) {
                                u_upload_data(v3d->uploader, 0,
                                              cb->cb[unit].buffer_size, 16,
                                              cb->cb[unit].user_buffer,
                                              &cb->cb[unit].buffer_offset,
                                              &cb->cb[unit].buffer);
                        }

                        cl_aligned_reloc(&job->indirect, &uniforms,
                                         v3d_resource(cb->cb[unit].buffer)->bo,
                                         cb->cb[unit].buffer_offset +
                                         v3d_unit_data_get_offset(data));
                        break;
                }

                case QUNIFORM_SSBO_OFFSET: {
                        struct pipe_shader_buffer *sb =
                                &v3d->ssbo[stage].sb[data];

                        cl_aligned_reloc(&job->indirect, &uniforms,
                                         v3d_resource(sb->buffer)->bo,
                                         sb->buffer_offset);
                        break;
                }

                case QUNIFORM_GET_SSBO_SIZE:
                        cl_aligned_u32(&uniforms,
                                       v3d->ssbo[stage].sb[data].buffer_size);
                        break;

                case QUNIFORM_TEXTURE_FIRST_LEVEL:
                        cl_aligned_f(&uniforms,
                                     texstate->textures[data]->u.tex.first_level);
                        break;

                case QUNIFORM_SPILL_OFFSET:
                        cl_aligned_reloc(&job->indirect, &uniforms,
                                         v3d->prog.spill_bo, 0);
                        break;

                case QUNIFORM_SPILL_SIZE_PER_THREAD:
                        cl_aligned_u32(&uniforms,
                                       v3d->prog.spill_size_per_thread);
                        break;

                case QUNIFORM_NUM_WORK_GROUPS:
                        cl_aligned_u32(&uniforms,
                                       v3d->compute_num_workgroups[data]);
                        break;

                case QUNIFORM_SHARED_OFFSET:
                        cl_aligned_reloc(&job->indirect, &uniforms,
                                         v3d->compute_shared_memory, 0);
                        break;

                case QUNIFORM_FB_LAYERS:
                        cl_aligned_u32(&uniforms, job->num_layers);
                        break;

                default:
                        unreachable("Unknown QUNIFORM");

                }
#if 0
                uint32_t written_val = *((uint32_t *)uniforms - 1);
                fprintf(stderr, "shader %p[%d]: 0x%08x / 0x%08x (%f) ",
                        shader, i, __gen_address_offset(&uniform_stream) + i * 4,
                        written_val, uif(written_val));
                vir_dump_uniform(uinfo->contents[i], data);
                fprintf(stderr, "\n");
#endif
        }

        cl_end(&job->indirect, uniforms);

        return uniform_stream;
}

void
v3d_set_shader_uniform_dirty_flags(struct v3d_compiled_shader *shader)
{
        uint64_t dirty = 0;

        for (int i = 0; i < shader->prog_data.base->uniforms.count; i++) {
                switch (shader->prog_data.base->uniforms.contents[i]) {
                case QUNIFORM_CONSTANT:
                        break;
                case QUNIFORM_UNIFORM:
                case QUNIFORM_UBO_ADDR:
                        dirty |= V3D_DIRTY_CONSTBUF;
                        break;

                case QUNIFORM_VIEWPORT_X_SCALE:
                case QUNIFORM_VIEWPORT_Y_SCALE:
                case QUNIFORM_VIEWPORT_Z_OFFSET:
                case QUNIFORM_VIEWPORT_Z_SCALE:
                        dirty |= V3D_DIRTY_VIEWPORT;
                        break;

                case QUNIFORM_USER_CLIP_PLANE:
                        dirty |= V3D_DIRTY_CLIP;
                        break;

                case QUNIFORM_TMU_CONFIG_P0:
                case QUNIFORM_TMU_CONFIG_P1:
                case QUNIFORM_TEXTURE_CONFIG_P1:
                case QUNIFORM_TEXTURE_FIRST_LEVEL:
                case QUNIFORM_TEXRECT_SCALE_X:
                case QUNIFORM_TEXRECT_SCALE_Y:
                case QUNIFORM_TEXTURE_WIDTH:
                case QUNIFORM_TEXTURE_HEIGHT:
                case QUNIFORM_TEXTURE_DEPTH:
                case QUNIFORM_TEXTURE_ARRAY_SIZE:
                case QUNIFORM_TEXTURE_LEVELS:
                case QUNIFORM_SPILL_OFFSET:
                case QUNIFORM_SPILL_SIZE_PER_THREAD:
                        /* We could flag this on just the stage we're
                         * compiling for, but it's not passed in.
                         */
                        dirty |= V3D_DIRTY_FRAGTEX | V3D_DIRTY_VERTTEX |
                                 V3D_DIRTY_GEOMTEX | V3D_DIRTY_COMPTEX;
                        break;

                case QUNIFORM_SSBO_OFFSET:
                case QUNIFORM_GET_SSBO_SIZE:
                        dirty |= V3D_DIRTY_SSBO;
                        break;

                case QUNIFORM_IMAGE_TMU_CONFIG_P0:
                case QUNIFORM_IMAGE_WIDTH:
                case QUNIFORM_IMAGE_HEIGHT:
                case QUNIFORM_IMAGE_DEPTH:
                case QUNIFORM_IMAGE_ARRAY_SIZE:
                        dirty |= V3D_DIRTY_SHADER_IMAGE;
                        break;

                case QUNIFORM_LINE_WIDTH:
                case QUNIFORM_AA_LINE_WIDTH:
                        dirty |= V3D_DIRTY_RASTERIZER;
                        break;

                case QUNIFORM_NUM_WORK_GROUPS:
                case QUNIFORM_SHARED_OFFSET:
                        /* Compute always recalculates uniforms. */
                        break;

                case QUNIFORM_FB_LAYERS:
                        dirty |= V3D_DIRTY_FRAMEBUFFER;
                        break;

                default:
                        assert(quniform_contents_is_texture_p0(shader->prog_data.base->uniforms.contents[i]));
                        dirty |= V3D_DIRTY_FRAGTEX | V3D_DIRTY_VERTTEX |
                                 V3D_DIRTY_GEOMTEX | V3D_DIRTY_COMPTEX;
                        break;
                }
        }

        shader->uniform_dirty_bits = dirty;
}
