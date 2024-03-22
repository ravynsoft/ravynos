/*
 * Copyright (C) 2021 Collabora Ltd.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "genxml/gen_macros.h"

#include "compiler/shader_enums.h"
#include "util/macros.h"

#include "vk_util.h"

#include "pan_desc.h"
#include "pan_earlyzs.h"
#include "pan_encoder.h"
#include "pan_pool.h"
#include "pan_shader.h"

#include "panvk_cs.h"
#include "panvk_private.h"
#include "panvk_varyings.h"

#include "vk_sampler.h"

static enum mali_mipmap_mode
panvk_translate_sampler_mipmap_mode(VkSamplerMipmapMode mode)
{
   switch (mode) {
   case VK_SAMPLER_MIPMAP_MODE_NEAREST:
      return MALI_MIPMAP_MODE_NEAREST;
   case VK_SAMPLER_MIPMAP_MODE_LINEAR:
      return MALI_MIPMAP_MODE_TRILINEAR;
   default:
      unreachable("Invalid mipmap mode");
   }
}

static unsigned
panvk_translate_sampler_address_mode(VkSamplerAddressMode mode)
{
   switch (mode) {
   case VK_SAMPLER_ADDRESS_MODE_REPEAT:
      return MALI_WRAP_MODE_REPEAT;
   case VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT:
      return MALI_WRAP_MODE_MIRRORED_REPEAT;
   case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:
      return MALI_WRAP_MODE_CLAMP_TO_EDGE;
   case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:
      return MALI_WRAP_MODE_CLAMP_TO_BORDER;
   case VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE:
      return MALI_WRAP_MODE_MIRRORED_CLAMP_TO_EDGE;
   default:
      unreachable("Invalid wrap");
   }
}

static mali_pixel_format
panvk_varying_hw_format(const struct panvk_device *dev,
                        const struct panvk_varyings_info *varyings,
                        gl_shader_stage stage, unsigned idx)
{
   const struct panfrost_device *pdev = &dev->physical_device->pdev;
   gl_varying_slot loc = varyings->stage[stage].loc[idx];

   switch (loc) {
   case VARYING_SLOT_PNTC:
   case VARYING_SLOT_PSIZ:
#if PAN_ARCH <= 6
      return (MALI_R16F << 12) | panfrost_get_default_swizzle(1);
#else
      return (MALI_R16F << 12) | MALI_RGB_COMPONENT_ORDER_R000;
#endif
   case VARYING_SLOT_POS:
#if PAN_ARCH <= 6
      return (MALI_SNAP_4 << 12) | panfrost_get_default_swizzle(4);
#else
      return (MALI_SNAP_4 << 12) | MALI_RGB_COMPONENT_ORDER_RGBA;
#endif
   default:
      if (varyings->varying[loc].format != PIPE_FORMAT_NONE)
         return pdev->formats[varyings->varying[loc].format].hw;
#if PAN_ARCH >= 7
      return (MALI_CONSTANT << 12) | MALI_RGB_COMPONENT_ORDER_0000;
#else
      return (MALI_CONSTANT << 12) | PAN_V6_SWIZZLE(0, 0, 0, 0);
#endif
   }
}

static void
panvk_emit_varying(const struct panvk_device *dev,
                   const struct panvk_varyings_info *varyings,
                   gl_shader_stage stage, unsigned idx, void *attrib)
{
   gl_varying_slot loc = varyings->stage[stage].loc[idx];

   pan_pack(attrib, ATTRIBUTE, cfg) {
      cfg.buffer_index = varyings->varying[loc].buf;
      cfg.offset = varyings->varying[loc].offset;
      cfg.format = panvk_varying_hw_format(dev, varyings, stage, idx);
   }
}

void
panvk_per_arch(emit_varyings)(const struct panvk_device *dev,
                              const struct panvk_varyings_info *varyings,
                              gl_shader_stage stage, void *descs)
{
   struct mali_attribute_packed *attrib = descs;

   for (unsigned i = 0; i < varyings->stage[stage].count; i++)
      panvk_emit_varying(dev, varyings, stage, i, attrib++);
}

static void
panvk_emit_varying_buf(const struct panvk_varyings_info *varyings,
                       enum panvk_varying_buf_id id, void *buf)
{
   unsigned buf_idx = panvk_varying_buf_index(varyings, id);

   pan_pack(buf, ATTRIBUTE_BUFFER, cfg) {
      unsigned offset = varyings->buf[buf_idx].address & 63;

      cfg.stride = varyings->buf[buf_idx].stride;
      cfg.size = varyings->buf[buf_idx].size + offset;
      cfg.pointer = varyings->buf[buf_idx].address & ~63ULL;
   }
}

void
panvk_per_arch(emit_varying_bufs)(const struct panvk_varyings_info *varyings,
                                  void *descs)
{
   struct mali_attribute_buffer_packed *buf = descs;

   for (unsigned i = 0; i < PANVK_VARY_BUF_MAX; i++) {
      if (varyings->buf_mask & (1 << i))
         panvk_emit_varying_buf(varyings, i, buf++);
   }
}

static void
panvk_emit_attrib_buf(const struct panvk_attribs_info *info,
                      const struct panvk_draw_info *draw,
                      const struct panvk_attrib_buf *bufs, unsigned buf_count,
                      unsigned idx, void *desc)
{
   const struct panvk_attrib_buf_info *buf_info = &info->buf[idx];

   assert(idx < buf_count);
   const struct panvk_attrib_buf *buf = &bufs[idx];
   mali_ptr addr = buf->address & ~63ULL;
   unsigned size = buf->size + (buf->address & 63);
   unsigned divisor = draw->padded_vertex_count * buf_info->instance_divisor;

   /* TODO: support instanced arrays */
   if (draw->instance_count <= 1) {
      pan_pack(desc, ATTRIBUTE_BUFFER, cfg) {
         cfg.type = MALI_ATTRIBUTE_TYPE_1D;
         cfg.stride = buf_info->per_instance ? 0 : buf_info->stride;
         cfg.pointer = addr;
         cfg.size = size;
      }
   } else if (!buf_info->per_instance) {
      pan_pack(desc, ATTRIBUTE_BUFFER, cfg) {
         cfg.type = MALI_ATTRIBUTE_TYPE_1D_MODULUS;
         cfg.divisor = draw->padded_vertex_count;
         cfg.stride = buf_info->stride;
         cfg.pointer = addr;
         cfg.size = size;
      }
   } else if (!divisor) {
      /* instance_divisor == 0 means all instances share the same value.
       * Make it a 1D array with a zero stride.
       */
      pan_pack(desc, ATTRIBUTE_BUFFER, cfg) {
         cfg.type = MALI_ATTRIBUTE_TYPE_1D;
         cfg.stride = 0;
         cfg.pointer = addr;
         cfg.size = size;
      }
   } else if (util_is_power_of_two_or_zero(divisor)) {
      pan_pack(desc, ATTRIBUTE_BUFFER, cfg) {
         cfg.type = MALI_ATTRIBUTE_TYPE_1D_POT_DIVISOR;
         cfg.stride = buf_info->stride;
         cfg.pointer = addr;
         cfg.size = size;
         cfg.divisor_r = __builtin_ctz(divisor);
      }
   } else {
      unsigned divisor_r = 0, divisor_e = 0;
      unsigned divisor_num =
         panfrost_compute_magic_divisor(divisor, &divisor_r, &divisor_e);
      pan_pack(desc, ATTRIBUTE_BUFFER, cfg) {
         cfg.type = MALI_ATTRIBUTE_TYPE_1D_NPOT_DIVISOR;
         cfg.stride = buf_info->stride;
         cfg.pointer = addr;
         cfg.size = size;
         cfg.divisor_r = divisor_r;
         cfg.divisor_e = divisor_e;
      }

      desc += pan_size(ATTRIBUTE_BUFFER);
      pan_pack(desc, ATTRIBUTE_BUFFER_CONTINUATION_NPOT, cfg) {
         cfg.divisor_numerator = divisor_num;
         cfg.divisor = buf_info->instance_divisor;
      }
   }
}

void
panvk_per_arch(emit_attrib_bufs)(const struct panvk_attribs_info *info,
                                 const struct panvk_attrib_buf *bufs,
                                 unsigned buf_count,
                                 const struct panvk_draw_info *draw,
                                 void *descs)
{
   struct mali_attribute_buffer_packed *buf = descs;

   for (unsigned i = 0; i < info->buf_count; i++) {
      panvk_emit_attrib_buf(info, draw, bufs, buf_count, i, buf);
      buf += 2;
   }
}

void
panvk_per_arch(emit_sampler)(const VkSamplerCreateInfo *pCreateInfo, void *desc)
{
   VkClearColorValue border_color =
      vk_sampler_border_color_value(pCreateInfo, NULL);

   pan_pack(desc, SAMPLER, cfg) {
      cfg.magnify_nearest = pCreateInfo->magFilter == VK_FILTER_NEAREST;
      cfg.minify_nearest = pCreateInfo->minFilter == VK_FILTER_NEAREST;
      cfg.mipmap_mode =
         panvk_translate_sampler_mipmap_mode(pCreateInfo->mipmapMode);
      cfg.normalized_coordinates = !pCreateInfo->unnormalizedCoordinates;

      cfg.lod_bias = pCreateInfo->mipLodBias;
      cfg.minimum_lod = pCreateInfo->minLod;
      cfg.maximum_lod = pCreateInfo->maxLod;
      cfg.wrap_mode_s =
         panvk_translate_sampler_address_mode(pCreateInfo->addressModeU);
      cfg.wrap_mode_t =
         panvk_translate_sampler_address_mode(pCreateInfo->addressModeV);
      cfg.wrap_mode_r =
         panvk_translate_sampler_address_mode(pCreateInfo->addressModeW);
      cfg.compare_function =
         panvk_per_arch(translate_sampler_compare_func)(pCreateInfo);
      cfg.border_color_r = border_color.uint32[0];
      cfg.border_color_g = border_color.uint32[1];
      cfg.border_color_b = border_color.uint32[2];
      cfg.border_color_a = border_color.uint32[3];
   }
}

static void
panvk_emit_attrib(const struct panvk_device *dev,
                  const struct panvk_draw_info *draw,
                  const struct panvk_attribs_info *attribs,
                  const struct panvk_attrib_buf *bufs, unsigned buf_count,
                  unsigned idx, void *attrib)
{
   const struct panfrost_device *pdev = &dev->physical_device->pdev;
   unsigned buf_idx = attribs->attrib[idx].buf;
   const struct panvk_attrib_buf_info *buf_info = &attribs->buf[buf_idx];

   pan_pack(attrib, ATTRIBUTE, cfg) {
      cfg.buffer_index = buf_idx * 2;
      cfg.offset = attribs->attrib[idx].offset + (bufs[buf_idx].address & 63);

      if (buf_info->per_instance)
         cfg.offset += draw->first_instance * buf_info->stride;

      cfg.format = pdev->formats[attribs->attrib[idx].format].hw;
   }
}

void
panvk_per_arch(emit_attribs)(const struct panvk_device *dev,
                             const struct panvk_draw_info *draw,
                             const struct panvk_attribs_info *attribs,
                             const struct panvk_attrib_buf *bufs,
                             unsigned buf_count, void *descs)
{
   struct mali_attribute_packed *attrib = descs;

   for (unsigned i = 0; i < attribs->attrib_count; i++)
      panvk_emit_attrib(dev, draw, attribs, bufs, buf_count, i, attrib++);
}

void
panvk_per_arch(emit_ubo)(mali_ptr address, size_t size, void *desc)
{
   pan_pack(desc, UNIFORM_BUFFER, cfg) {
      cfg.pointer = address;
      cfg.entries = DIV_ROUND_UP(size, 16);
   }
}

void
panvk_per_arch(emit_ubos)(const struct panvk_pipeline *pipeline,
                          const struct panvk_descriptor_state *state,
                          void *descs)
{
   struct mali_uniform_buffer_packed *ubos = descs;

   panvk_per_arch(emit_ubo)(state->sysvals_ptr, sizeof(state->sysvals),
                            &ubos[PANVK_SYSVAL_UBO_INDEX]);

   if (pipeline->layout->push_constants.size) {
      panvk_per_arch(emit_ubo)(
         state->push_constants,
         ALIGN_POT(pipeline->layout->push_constants.size, 16),
         &ubos[PANVK_PUSH_CONST_UBO_INDEX]);
   } else {
      memset(&ubos[PANVK_PUSH_CONST_UBO_INDEX], 0, sizeof(*ubos));
   }

   for (unsigned s = 0; s < pipeline->layout->vk.set_count; s++) {
      const struct panvk_descriptor_set_layout *set_layout =
         vk_to_panvk_descriptor_set_layout(pipeline->layout->vk.set_layouts[s]);
      const struct panvk_descriptor_set *set = state->sets[s];

      unsigned ubo_start =
         panvk_pipeline_layout_ubo_start(pipeline->layout, s, false);

      if (!set) {
         unsigned all_ubos = set_layout->num_ubos + set_layout->num_dyn_ubos;
         memset(&ubos[ubo_start], 0, all_ubos * sizeof(*ubos));
      } else {
         memcpy(&ubos[ubo_start], set->ubos,
                set_layout->num_ubos * sizeof(*ubos));

         unsigned dyn_ubo_start =
            panvk_pipeline_layout_ubo_start(pipeline->layout, s, true);

         for (unsigned i = 0; i < set_layout->num_dyn_ubos; i++) {
            const struct panvk_buffer_desc *bdesc =
               &state->dyn.ubos[pipeline->layout->sets[s].dyn_ubo_offset + i];

            mali_ptr address =
               panvk_buffer_gpu_ptr(bdesc->buffer, bdesc->offset);
            size_t size =
               panvk_buffer_range(bdesc->buffer, bdesc->offset, bdesc->size);
            if (size) {
               panvk_per_arch(emit_ubo)(address, size,
                                        &ubos[dyn_ubo_start + i]);
            } else {
               memset(&ubos[dyn_ubo_start + i], 0, sizeof(*ubos));
            }
         }
      }
   }
}

void
panvk_per_arch(emit_vertex_job)(const struct panvk_pipeline *pipeline,
                                const struct panvk_draw_info *draw, void *job)
{
   void *section = pan_section_ptr(job, COMPUTE_JOB, INVOCATION);

   memcpy(section, &draw->invocation, pan_size(INVOCATION));

   pan_section_pack(job, COMPUTE_JOB, PARAMETERS, cfg) {
      cfg.job_task_split = 5;
   }

   pan_section_pack(job, COMPUTE_JOB, DRAW, cfg) {
      cfg.state = pipeline->rsds[MESA_SHADER_VERTEX];
      cfg.attributes = draw->stages[MESA_SHADER_VERTEX].attributes;
      cfg.attribute_buffers = draw->stages[MESA_SHADER_VERTEX].attribute_bufs;
      cfg.varyings = draw->stages[MESA_SHADER_VERTEX].varyings;
      cfg.varying_buffers = draw->varying_bufs;
      cfg.thread_storage = draw->tls;
      cfg.offset_start = draw->offset_start;
      cfg.instance_size =
         draw->instance_count > 1 ? draw->padded_vertex_count : 1;
      cfg.uniform_buffers = draw->ubos;
      cfg.push_uniforms = draw->stages[PIPE_SHADER_VERTEX].push_constants;
      cfg.textures = draw->textures;
      cfg.samplers = draw->samplers;
   }
}

void
panvk_per_arch(emit_compute_job)(const struct panvk_pipeline *pipeline,
                                 const struct panvk_dispatch_info *dispatch,
                                 void *job)
{
   panfrost_pack_work_groups_compute(
      pan_section_ptr(job, COMPUTE_JOB, INVOCATION), dispatch->wg_count.x,
      dispatch->wg_count.y, dispatch->wg_count.z, pipeline->cs.local_size.x,
      pipeline->cs.local_size.y, pipeline->cs.local_size.z, false, false);

   pan_section_pack(job, COMPUTE_JOB, PARAMETERS, cfg) {
      cfg.job_task_split = util_logbase2_ceil(pipeline->cs.local_size.x + 1) +
                           util_logbase2_ceil(pipeline->cs.local_size.y + 1) +
                           util_logbase2_ceil(pipeline->cs.local_size.z + 1);
   }

   pan_section_pack(job, COMPUTE_JOB, DRAW, cfg) {
      cfg.state = pipeline->rsds[MESA_SHADER_COMPUTE];
      cfg.attributes = dispatch->attributes;
      cfg.attribute_buffers = dispatch->attribute_bufs;
      cfg.thread_storage = dispatch->tsd;
      cfg.uniform_buffers = dispatch->ubos;
      cfg.push_uniforms = dispatch->push_uniforms;
      cfg.textures = dispatch->textures;
      cfg.samplers = dispatch->samplers;
   }
}

static void
panvk_emit_tiler_primitive(const struct panvk_pipeline *pipeline,
                           const struct panvk_draw_info *draw, void *prim)
{
   pan_pack(prim, PRIMITIVE, cfg) {
      cfg.draw_mode = pipeline->ia.topology;
      if (pipeline->ia.writes_point_size)
         cfg.point_size_array_format = MALI_POINT_SIZE_ARRAY_FORMAT_FP16;

      cfg.first_provoking_vertex = true;
      if (pipeline->ia.primitive_restart)
         cfg.primitive_restart = MALI_PRIMITIVE_RESTART_IMPLICIT;
      cfg.job_task_split = 6;

      if (draw->index_size) {
         cfg.index_count = draw->index_count;
         cfg.indices = draw->indices;
         cfg.base_vertex_offset = draw->vertex_offset - draw->offset_start;

         switch (draw->index_size) {
         case 32:
            cfg.index_type = MALI_INDEX_TYPE_UINT32;
            break;
         case 16:
            cfg.index_type = MALI_INDEX_TYPE_UINT16;
            break;
         case 8:
            cfg.index_type = MALI_INDEX_TYPE_UINT8;
            break;
         default:
            unreachable("Invalid index size");
         }
      } else {
         cfg.index_count = draw->vertex_count;
         cfg.index_type = MALI_INDEX_TYPE_NONE;
      }
   }
}

static void
panvk_emit_tiler_primitive_size(const struct panvk_pipeline *pipeline,
                                const struct panvk_draw_info *draw,
                                void *primsz)
{
   pan_pack(primsz, PRIMITIVE_SIZE, cfg) {
      if (pipeline->ia.writes_point_size) {
         cfg.size_array = draw->psiz;
      } else {
         cfg.constant = draw->line_width;
      }
   }
}

static void
panvk_emit_tiler_dcd(const struct panvk_pipeline *pipeline,
                     const struct panvk_draw_info *draw, void *dcd)
{
   pan_pack(dcd, DRAW, cfg) {
      cfg.front_face_ccw = pipeline->rast.front_ccw;
      cfg.cull_front_face = pipeline->rast.cull_front_face;
      cfg.cull_back_face = pipeline->rast.cull_back_face;
      cfg.position = draw->position;
      cfg.state = draw->fs_rsd;
      cfg.attributes = draw->stages[MESA_SHADER_FRAGMENT].attributes;
      cfg.attribute_buffers = draw->stages[MESA_SHADER_FRAGMENT].attribute_bufs;
      cfg.viewport = draw->viewport;
      cfg.varyings = draw->stages[MESA_SHADER_FRAGMENT].varyings;
      cfg.varying_buffers = cfg.varyings ? draw->varying_bufs : 0;
      cfg.thread_storage = draw->tls;

      /* For all primitives but lines DRAW.flat_shading_vertex must
       * be set to 0 and the provoking vertex is selected with the
       * PRIMITIVE.first_provoking_vertex field.
       */
      if (pipeline->ia.topology == MALI_DRAW_MODE_LINES ||
          pipeline->ia.topology == MALI_DRAW_MODE_LINE_STRIP ||
          pipeline->ia.topology == MALI_DRAW_MODE_LINE_LOOP) {
         cfg.flat_shading_vertex = true;
      }

      cfg.offset_start = draw->offset_start;
      cfg.instance_size =
         draw->instance_count > 1 ? draw->padded_vertex_count : 1;
      cfg.uniform_buffers = draw->ubos;
      cfg.push_uniforms = draw->stages[PIPE_SHADER_FRAGMENT].push_constants;
      cfg.textures = draw->textures;
      cfg.samplers = draw->samplers;

      /* TODO: occlusion queries */
   }
}

void
panvk_per_arch(emit_tiler_job)(const struct panvk_pipeline *pipeline,
                               const struct panvk_draw_info *draw, void *job)
{
   void *section;

   section = pan_section_ptr(job, TILER_JOB, INVOCATION);
   memcpy(section, &draw->invocation, pan_size(INVOCATION));

   section = pan_section_ptr(job, TILER_JOB, PRIMITIVE);
   panvk_emit_tiler_primitive(pipeline, draw, section);

   section = pan_section_ptr(job, TILER_JOB, PRIMITIVE_SIZE);
   panvk_emit_tiler_primitive_size(pipeline, draw, section);

   section = pan_section_ptr(job, TILER_JOB, DRAW);
   panvk_emit_tiler_dcd(pipeline, draw, section);

   pan_section_pack(job, TILER_JOB, TILER, cfg) {
      cfg.address = draw->tiler_ctx->bifrost;
   }
   pan_section_pack(job, TILER_JOB, PADDING, padding)
      ;
}

void
panvk_per_arch(emit_viewport)(const VkViewport *viewport,
                              const VkRect2D *scissor, void *vpd)
{
   /* The spec says "width must be greater than 0.0" */
   assert(viewport->x >= 0);
   int minx = (int)viewport->x;
   int maxx = (int)(viewport->x + viewport->width);

   /* Viewport height can be negative */
   int miny = MIN2((int)viewport->y, (int)(viewport->y + viewport->height));
   int maxy = MAX2((int)viewport->y, (int)(viewport->y + viewport->height));

   assert(scissor->offset.x >= 0 && scissor->offset.y >= 0);
   miny = MAX2(scissor->offset.x, minx);
   miny = MAX2(scissor->offset.y, miny);
   maxx = MIN2(scissor->offset.x + scissor->extent.width, maxx);
   maxy = MIN2(scissor->offset.y + scissor->extent.height, maxy);

   /* Make sure we don't end up with a max < min when width/height is 0 */
   maxx = maxx > minx ? maxx - 1 : maxx;
   maxy = maxy > miny ? maxy - 1 : maxy;

   assert(viewport->minDepth >= 0.0f && viewport->minDepth <= 1.0f);
   assert(viewport->maxDepth >= 0.0f && viewport->maxDepth <= 1.0f);

   pan_pack(vpd, VIEWPORT, cfg) {
      cfg.scissor_minimum_x = minx;
      cfg.scissor_minimum_y = miny;
      cfg.scissor_maximum_x = maxx;
      cfg.scissor_maximum_y = maxy;
      cfg.minimum_z = MIN2(viewport->minDepth, viewport->maxDepth);
      cfg.maximum_z = MAX2(viewport->minDepth, viewport->maxDepth);
   }
}

static enum mali_register_file_format
bifrost_blend_type_from_nir(nir_alu_type nir_type)
{
   switch (nir_type) {
   case 0: /* Render target not in use */
      return 0;
   case nir_type_float16:
      return MALI_REGISTER_FILE_FORMAT_F16;
   case nir_type_float32:
      return MALI_REGISTER_FILE_FORMAT_F32;
   case nir_type_int32:
      return MALI_REGISTER_FILE_FORMAT_I32;
   case nir_type_uint32:
      return MALI_REGISTER_FILE_FORMAT_U32;
   case nir_type_int16:
      return MALI_REGISTER_FILE_FORMAT_I16;
   case nir_type_uint16:
      return MALI_REGISTER_FILE_FORMAT_U16;
   default:
      unreachable("Unsupported blend shader type for NIR alu type");
   }
}

void
panvk_per_arch(emit_blend)(const struct panvk_device *dev,
                           const struct panvk_pipeline *pipeline, unsigned rt,
                           void *bd)
{
   const struct pan_blend_state *blend = &pipeline->blend.state;
   const struct pan_blend_rt_state *rts = &blend->rts[rt];
   bool dithered = false;

   pan_pack(bd, BLEND, cfg) {
      if (!blend->rt_count || !rts->equation.color_mask) {
         cfg.enable = false;
         cfg.internal.mode = MALI_BLEND_MODE_OFF;
         continue;
      }

      cfg.srgb = util_format_is_srgb(rts->format);
      cfg.load_destination = pan_blend_reads_dest(blend->rts[rt].equation);
      cfg.round_to_fb_precision = !dithered;

      const struct panfrost_device *pdev = &dev->physical_device->pdev;
      const struct util_format_description *format_desc =
         util_format_description(rts->format);
      unsigned chan_size = 0;
      for (unsigned i = 0; i < format_desc->nr_channels; i++)
         chan_size = MAX2(format_desc->channel[i].size, chan_size);

      pan_blend_to_fixed_function_equation(blend->rts[rt].equation,
                                           &cfg.equation);

      /* Fixed point constant */
      float fconst = pan_blend_get_constant(
         pan_blend_constant_mask(blend->rts[rt].equation), blend->constants);
      u16 constant = fconst * ((1 << chan_size) - 1);
      constant <<= 16 - chan_size;
      cfg.constant = constant;

      if (pan_blend_is_opaque(blend->rts[rt].equation)) {
         cfg.internal.mode = MALI_BLEND_MODE_OPAQUE;
      } else {
         cfg.internal.mode = MALI_BLEND_MODE_FIXED_FUNCTION;

         cfg.internal.fixed_function.alpha_zero_nop =
            pan_blend_alpha_zero_nop(blend->rts[rt].equation);
         cfg.internal.fixed_function.alpha_one_store =
            pan_blend_alpha_one_store(blend->rts[rt].equation);
      }

      /* If we want the conversion to work properly,
       * num_comps must be set to 4
       */
      cfg.internal.fixed_function.num_comps = 4;
      cfg.internal.fixed_function.conversion.memory_format =
         panfrost_format_to_bifrost_blend(pdev, rts->format, dithered);
      cfg.internal.fixed_function.conversion.register_format =
         bifrost_blend_type_from_nir(pipeline->fs.info.bifrost.blend[rt].type);
      cfg.internal.fixed_function.rt = rt;
   }
}

void
panvk_per_arch(emit_blend_constant)(const struct panvk_device *dev,
                                    const struct panvk_pipeline *pipeline,
                                    unsigned rt, const float *constants,
                                    void *bd)
{
   float constant = constants[pipeline->blend.constant[rt].index];

   pan_pack(bd, BLEND, cfg) {
      cfg.enable = false;
      cfg.constant = constant * pipeline->blend.constant[rt].bifrost_factor;
   }
}

void
panvk_per_arch(emit_dyn_fs_rsd)(const struct panvk_pipeline *pipeline,
                                const struct panvk_cmd_state *state, void *rsd)
{
   pan_pack(rsd, RENDERER_STATE, cfg) {
      if (pipeline->dynamic_state_mask & (1 << VK_DYNAMIC_STATE_DEPTH_BIAS)) {
         cfg.depth_units = state->rast.depth_bias.constant_factor * 2.0f;
         cfg.depth_factor = state->rast.depth_bias.slope_factor;
         cfg.depth_bias_clamp = state->rast.depth_bias.clamp;
      }

      if (pipeline->dynamic_state_mask &
          (1 << VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK)) {
         cfg.stencil_front.mask = state->zs.s_front.compare_mask;
         cfg.stencil_back.mask = state->zs.s_back.compare_mask;
      }

      if (pipeline->dynamic_state_mask &
          (1 << VK_DYNAMIC_STATE_STENCIL_WRITE_MASK)) {
         cfg.stencil_mask_misc.stencil_mask_front =
            state->zs.s_front.write_mask;
         cfg.stencil_mask_misc.stencil_mask_back = state->zs.s_back.write_mask;
      }

      if (pipeline->dynamic_state_mask &
          (1 << VK_DYNAMIC_STATE_STENCIL_REFERENCE)) {
         cfg.stencil_front.reference_value = state->zs.s_front.ref;
         cfg.stencil_back.reference_value = state->zs.s_back.ref;
      }
   }
}

void
panvk_per_arch(emit_base_fs_rsd)(const struct panvk_device *dev,
                                 const struct panvk_pipeline *pipeline,
                                 void *rsd)
{
   const struct pan_shader_info *info = &pipeline->fs.info;

   pan_pack(rsd, RENDERER_STATE, cfg) {
      if (pipeline->fs.required) {
         pan_shader_prepare_rsd(info, pipeline->fs.address, &cfg);

         uint8_t rt_written =
            pipeline->fs.info.outputs_written >> FRAG_RESULT_DATA0;
         uint8_t rt_mask = pipeline->fs.rt_mask;
         cfg.properties.allow_forward_pixel_to_kill =
            pipeline->fs.info.fs.can_fpk && !(rt_mask & ~rt_written) &&
            !pipeline->ms.alpha_to_coverage && !pipeline->blend.reads_dest;

         bool writes_zs = pipeline->zs.z_write || pipeline->zs.s_test;
         bool zs_always_passes = !pipeline->zs.z_test && !pipeline->zs.s_test;
         bool oq = false; /* TODO: Occlusion queries */

         struct pan_earlyzs_state earlyzs =
            pan_earlyzs_get(pan_earlyzs_analyze(info), writes_zs || oq,
                            pipeline->ms.alpha_to_coverage, zs_always_passes);

         cfg.properties.pixel_kill_operation = earlyzs.kill;
         cfg.properties.zs_update_operation = earlyzs.update;
      } else {
         cfg.properties.depth_source = MALI_DEPTH_SOURCE_FIXED_FUNCTION;
         cfg.properties.allow_forward_pixel_to_kill = true;
         cfg.properties.allow_forward_pixel_to_be_killed = true;
         cfg.properties.zs_update_operation = MALI_PIXEL_KILL_STRONG_EARLY;
      }

      bool msaa = pipeline->ms.rast_samples > 1;
      cfg.multisample_misc.multisample_enable = msaa;
      cfg.multisample_misc.sample_mask =
         msaa ? pipeline->ms.sample_mask : UINT16_MAX;

      cfg.multisample_misc.depth_function =
         pipeline->zs.z_test ? pipeline->zs.z_compare_func : MALI_FUNC_ALWAYS;

      cfg.multisample_misc.depth_write_mask = pipeline->zs.z_write;
      cfg.multisample_misc.fixed_function_near_discard =
         !pipeline->rast.clamp_depth;
      cfg.multisample_misc.fixed_function_far_discard =
         !pipeline->rast.clamp_depth;
      cfg.multisample_misc.shader_depth_range_fixed = true;

      cfg.stencil_mask_misc.stencil_enable = pipeline->zs.s_test;
      cfg.stencil_mask_misc.alpha_to_coverage = pipeline->ms.alpha_to_coverage;
      cfg.stencil_mask_misc.alpha_test_compare_function = MALI_FUNC_ALWAYS;
      cfg.stencil_mask_misc.front_facing_depth_bias =
         pipeline->rast.depth_bias.enable;
      cfg.stencil_mask_misc.back_facing_depth_bias =
         pipeline->rast.depth_bias.enable;
      cfg.stencil_mask_misc.single_sampled_lines =
         pipeline->ms.rast_samples <= 1;

      if (!(pipeline->dynamic_state_mask &
            (1 << VK_DYNAMIC_STATE_DEPTH_BIAS))) {
         cfg.depth_units = pipeline->rast.depth_bias.constant_factor * 2.0f;
         cfg.depth_factor = pipeline->rast.depth_bias.slope_factor;
         cfg.depth_bias_clamp = pipeline->rast.depth_bias.clamp;
      }

      if (!(pipeline->dynamic_state_mask &
            (1 << VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK))) {
         cfg.stencil_front.mask = pipeline->zs.s_front.compare_mask;
         cfg.stencil_back.mask = pipeline->zs.s_back.compare_mask;
      }

      if (!(pipeline->dynamic_state_mask &
            (1 << VK_DYNAMIC_STATE_STENCIL_WRITE_MASK))) {
         cfg.stencil_mask_misc.stencil_mask_front =
            pipeline->zs.s_front.write_mask;
         cfg.stencil_mask_misc.stencil_mask_back =
            pipeline->zs.s_back.write_mask;
      }

      if (!(pipeline->dynamic_state_mask &
            (1 << VK_DYNAMIC_STATE_STENCIL_REFERENCE))) {
         cfg.stencil_front.reference_value = pipeline->zs.s_front.ref;
         cfg.stencil_back.reference_value = pipeline->zs.s_back.ref;
      }

      cfg.stencil_front.compare_function = pipeline->zs.s_front.compare_func;
      cfg.stencil_front.stencil_fail = pipeline->zs.s_front.fail_op;
      cfg.stencil_front.depth_fail = pipeline->zs.s_front.z_fail_op;
      cfg.stencil_front.depth_pass = pipeline->zs.s_front.pass_op;
      cfg.stencil_back.compare_function = pipeline->zs.s_back.compare_func;
      cfg.stencil_back.stencil_fail = pipeline->zs.s_back.fail_op;
      cfg.stencil_back.depth_fail = pipeline->zs.s_back.z_fail_op;
      cfg.stencil_back.depth_pass = pipeline->zs.s_back.pass_op;
   }
}

void
panvk_per_arch(emit_non_fs_rsd)(const struct panvk_device *dev,
                                const struct pan_shader_info *shader_info,
                                mali_ptr shader_ptr, void *rsd)
{
   assert(shader_info->stage != MESA_SHADER_FRAGMENT);

   pan_pack(rsd, RENDERER_STATE, cfg) {
      pan_shader_prepare_rsd(shader_info, shader_ptr, &cfg);
   }
}

void
panvk_per_arch(emit_tiler_context)(const struct panvk_device *dev,
                                   unsigned width, unsigned height,
                                   const struct panfrost_ptr *descs)
{
   const struct panfrost_device *pdev = &dev->physical_device->pdev;

   pan_pack(descs->cpu + pan_size(TILER_CONTEXT), TILER_HEAP, cfg) {
      cfg.size = panfrost_bo_size(pdev->tiler_heap);
      cfg.base = pdev->tiler_heap->ptr.gpu;
      cfg.bottom = pdev->tiler_heap->ptr.gpu;
      cfg.top = pdev->tiler_heap->ptr.gpu + panfrost_bo_size(pdev->tiler_heap);
   }

   pan_pack(descs->cpu, TILER_CONTEXT, cfg) {
      cfg.hierarchy_mask = 0x28;
      cfg.fb_width = width;
      cfg.fb_height = height;
      cfg.heap = descs->gpu + pan_size(TILER_CONTEXT);
   }
}
