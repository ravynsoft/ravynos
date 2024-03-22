/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/


#include "main/arrayobj.h"
#include "main/image.h"
#include "main/macros.h"
#include "main/varray.h"

#include "vbo/vbo.h"

#include "st_context.h"
#include "st_atom.h"
#include "st_cb_bitmap.h"
#include "st_draw.h"
#include "st_program.h"
#include "st_util.h"

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "util/u_inlines.h"
#include "util/u_draw.h"
#include "util/format/u_format.h"

#include "draw/draw_private.h"
#include "draw/draw_context.h"


/**
 * Set the (private) draw module's post-transformed vertex format when in
 * GL_SELECT or GL_FEEDBACK mode or for glRasterPos.
 */
static void
set_feedback_vertex_format(struct gl_context *ctx)
{
#if 0
   struct st_context *st = st_context(ctx);
   struct vertex_info vinfo;
   GLuint i;

   memset(&vinfo, 0, sizeof(vinfo));

   if (ctx->RenderMode == GL_SELECT) {
      assert(ctx->RenderMode == GL_SELECT);
      vinfo.num_attribs = 1;
      vinfo.format[0] = FORMAT_4F;
      vinfo.interp_mode[0] = INTERP_LINEAR;
   }
   else {
      /* GL_FEEDBACK, or glRasterPos */
      /* emit all attribs (pos, color, texcoord) as GLfloat[4] */
      vinfo.num_attribs = st->state.vs->cso->state.num_outputs;
      for (i = 0; i < vinfo.num_attribs; i++) {
         vinfo.format[i] = FORMAT_4F;
         vinfo.interp_mode[i] = INTERP_LINEAR;
      }
   }

   draw_set_vertex_info(st->draw, &vinfo);
#endif
}


/**
 * Called by VBO to draw arrays when in selection or feedback mode and
 * to implement glRasterPos.
 * This function mirrors the normal st_draw_vbo().
 * Look at code refactoring some day.
 */
void
st_feedback_draw_vbo(struct gl_context *ctx,
                     struct pipe_draw_info *info,
                     unsigned drawid_offset,
                     const struct pipe_draw_indirect_info *indirect,
                     const struct pipe_draw_start_count_bias *draws,
                     unsigned num_draws)
{
   struct st_context *st = st_context(ctx);
   struct pipe_context *pipe = st->pipe;
   struct draw_context *draw = st_get_draw_context(st);
   struct gl_vertex_program *vp;
   struct st_common_variant *vp_variant;
   struct pipe_vertex_buffer vbuffers[PIPE_MAX_SHADER_INPUTS];
   unsigned num_vbuffers = 0;
   struct cso_velems_state velements;
   struct pipe_transfer *vb_transfer[PIPE_MAX_ATTRIBS] = {NULL};
   struct pipe_transfer *ib_transfer = NULL;
   GLuint i;
   const void *mapped_indices = NULL;

   if (!draw)
      return;

   /* must get these after state validation! */
   struct st_common_variant_key key;
   /* We have to use memcpy to make sure that all bits are copied. */
   memcpy(&key, &st->vp_variant->key, sizeof(key));
   key.is_draw_shader = true;

   vp = (struct gl_vertex_program *)ctx->VertexProgram._Current;
   vp_variant = st_get_common_variant(st, &vp->Base, &key);

   /*
    * Set up the draw module's state.
    *
    * We'd like to do this less frequently, but the normal state-update
    * code sends state updates to the pipe, not to our private draw module.
    */
   assert(draw);
   draw_set_viewport_states(draw, 0, 1, &st->state.viewport[0]);
   draw_set_clip_state(draw, &st->state.clip);
   draw_set_rasterizer_state(draw, &st->state.rasterizer, NULL);
   draw_bind_vertex_shader(draw, vp_variant->base.driver_shader);
   set_feedback_vertex_format(ctx);

   /* Must setup these after state validation! */
   /* Setup arrays */
   st_setup_arrays(st, vp, vp_variant, &velements, vbuffers, &num_vbuffers);
   /* Setup current values as userspace arrays */
   st_setup_current_user(st, vp, vp_variant, &velements, vbuffers, &num_vbuffers);

   /* Map all buffers and tell draw about their mapping */
   for (unsigned buf = 0; buf < num_vbuffers; ++buf) {
      struct pipe_vertex_buffer *vbuffer = &vbuffers[buf];

      if (vbuffer->is_user_buffer) {
         draw_set_mapped_vertex_buffer(draw, buf, vbuffer->buffer.user, ~0);
      } else {
         void *map = pipe_buffer_map(pipe, vbuffer->buffer.resource,
                                     PIPE_MAP_READ, &vb_transfer[buf]);
         draw_set_mapped_vertex_buffer(draw, buf, map,
                                       vbuffer->buffer.resource->width0);
      }
   }

   draw_set_vertex_buffers(draw, num_vbuffers, 0, vbuffers);
   draw_set_vertex_elements(draw, vp->num_inputs, velements.velems);

   if (info->index_size) {
      if (info->has_user_indices) {
         mapped_indices = info->index.user;
      } else {
         mapped_indices = pipe_buffer_map(pipe, info->index.resource,
                                          PIPE_MAP_READ, &ib_transfer);
      }

      draw_set_indexes(draw, (uint8_t *)mapped_indices, info->index_size, ~0);
   }

   /* set constant buffer 0 */
   struct gl_program_parameter_list *params = vp->Base.Parameters;

   /* Update the constants which come from fixed-function state, such as
    * transformation matrices, fog factors, etc.
    *
    * It must be done here if the state tracker doesn't update state vars
    * in gl_program_parameter_list because allow_constbuf0_as_real_buffer
    * is set.
    */
   if (st->prefer_real_buffer_in_constbuf0 && params->StateFlags)
      _mesa_load_state_parameters(st->ctx, params);

   draw_set_constant_buffer_stride(draw, sizeof(float));
   draw_set_mapped_constant_buffer(draw, PIPE_SHADER_VERTEX, 0,
                                   params->ParameterValues,
                                   params->NumParameterValues * 4);

   /* set uniform buffers */
   const struct gl_program *prog = &vp->Base;
   struct pipe_transfer *ubo_transfer[PIPE_MAX_CONSTANT_BUFFERS] = {0};
   assert(prog->info.num_ubos <= ARRAY_SIZE(ubo_transfer));

   for (unsigned i = 0; i < prog->sh.NumUniformBlocks; i++) {
      struct gl_buffer_binding *binding =
         &st->ctx->UniformBufferBindings[prog->sh.UniformBlocks[i]->Binding];
      struct gl_buffer_object *st_obj = binding->BufferObject;
      struct pipe_resource *buf = st_obj->buffer;

      if (!buf)
         continue;

      unsigned offset = binding->Offset;
      unsigned size = buf->width0 - offset;

      /* AutomaticSize is FALSE if the buffer was set with BindBufferRange.
       * Take the minimum just to be sure.
       */
      if (!binding->AutomaticSize)
         size = MIN2(size, (unsigned) binding->Size);

      void *ptr = pipe_buffer_map_range(pipe, buf, offset, size,
                                        PIPE_MAP_READ, &ubo_transfer[i]);

      draw_set_mapped_constant_buffer(draw, PIPE_SHADER_VERTEX, 1 + i, ptr,
                                      size);
   }

   /* shader buffers */
   /* TODO: atomic counter buffers */
   struct pipe_transfer *ssbo_transfer[PIPE_MAX_SHADER_BUFFERS] = {0};

   for (unsigned i = 0; i < prog->info.num_ssbos; i++) {
      struct gl_buffer_binding *binding =
         &st->ctx->ShaderStorageBufferBindings[
            prog->sh.ShaderStorageBlocks[i]->Binding];
      struct gl_buffer_object *st_obj = binding->BufferObject;
      struct pipe_resource *buf = st_obj->buffer;

      if (!buf)
         continue;

      unsigned offset = binding->Offset;
      unsigned size = buf->width0 - binding->Offset;

      /* AutomaticSize is FALSE if the buffer was set with BindBufferRange.
       * Take the minimum just to be sure.
       */
      if (!binding->AutomaticSize)
         size = MIN2(size, (unsigned) binding->Size);

      void *ptr = pipe_buffer_map_range(pipe, buf, offset, size,
                                        PIPE_MAP_READ, &ssbo_transfer[i]);

      draw_set_mapped_shader_buffer(draw, PIPE_SHADER_VERTEX,
                                    i, ptr, size);
   }

   /* samplers */
   struct pipe_sampler_state *samplers[PIPE_MAX_SAMPLERS];
   for (unsigned i = 0; i < st->state.num_vert_samplers; i++)
      samplers[i] = &st->state.vert_samplers[i];

   draw_set_samplers(draw, PIPE_SHADER_VERTEX, samplers,
                     st->state.num_vert_samplers);

   /* sampler views */
   struct pipe_sampler_view *views[PIPE_MAX_SAMPLERS];
   unsigned num_views =
      st_get_sampler_views(st, PIPE_SHADER_VERTEX, prog, views);

   draw_set_sampler_views(draw, PIPE_SHADER_VERTEX, views, num_views);

   struct pipe_transfer *sv_transfer[PIPE_MAX_SAMPLERS][PIPE_MAX_TEXTURE_LEVELS];

   for (unsigned i = 0; i < num_views; i++) {
      struct pipe_sampler_view *view = views[i];
      if (!view)
         continue;

      struct pipe_resource *res = view->texture;
      unsigned width0 = res->width0;
      unsigned num_layers = res->depth0;
      unsigned first_level = 0;
      unsigned last_level = 0;
      uint32_t row_stride[PIPE_MAX_TEXTURE_LEVELS];
      uint32_t img_stride[PIPE_MAX_TEXTURE_LEVELS];
      uint32_t mip_offset[PIPE_MAX_TEXTURE_LEVELS];
      uintptr_t mip_addr[PIPE_MAX_TEXTURE_LEVELS];
      uintptr_t base_addr;

      if (res->target != PIPE_BUFFER) {
         first_level = view->u.tex.first_level;
         last_level = view->u.tex.last_level;
         num_layers = view->u.tex.last_layer - view->u.tex.first_layer + 1;
         base_addr = UINTPTR_MAX;

         for (unsigned j = first_level; j <= last_level; j++) {
            unsigned map_layers = res->target == PIPE_TEXTURE_3D ?
                                     util_num_layers(res, j) : num_layers;

            sv_transfer[i][j] = NULL;
            mip_addr[j] = (uintptr_t)
                          pipe_texture_map_3d(pipe, res, j,
                                               PIPE_MAP_READ, 0, 0,
                                               view->u.tex.first_layer,
                                               u_minify(res->width0, j),
                                               u_minify(res->height0, j),
                                               map_layers, &sv_transfer[i][j]);
            row_stride[j] = sv_transfer[i][j]->stride;
            img_stride[j] = sv_transfer[i][j]->layer_stride;

            /* Get the minimum address, because the draw module takes only
             * 1 address for the whole texture + uint32 offsets for mip levels,
             * so we need to convert mapped resource pointers into that scheme.
             */
            base_addr = MIN2(base_addr, mip_addr[j]);
         }
         for (unsigned j = first_level; j <= last_level; j++) {
            /* TODO: The draw module should accept pointers for mipmap levels
             * instead of offsets. This is unlikely to work on 64-bit archs.
             */
            assert(mip_addr[j] - base_addr <= UINT32_MAX);
            mip_offset[j] = mip_addr[j] - base_addr;
         }
      } else {
         width0 = view->u.buf.size / util_format_get_blocksize(view->format);

         /* probably don't really need to fill that out */
         mip_offset[0] = 0;
         row_stride[0] = 0;
         img_stride[0] = 0;

         sv_transfer[i][0] = NULL;
         base_addr = (uintptr_t)
                     pipe_buffer_map_range(pipe, res, view->u.buf.offset,
                                           view->u.buf.size,
                                           PIPE_MAP_READ,
                                           &sv_transfer[i][0]);
      }

      draw_set_mapped_texture(draw, PIPE_SHADER_VERTEX, i, width0,
                              res->height0, num_layers, first_level,
                              last_level, 0, 0, (void*)base_addr, row_stride,
                              img_stride, mip_offset);
   }

   /* shader images */
   struct pipe_image_view images[PIPE_MAX_SHADER_IMAGES];
   struct pipe_transfer *img_transfer[PIPE_MAX_SHADER_IMAGES] = {0};

   for (unsigned i = 0; i < prog->info.num_images; i++) {
      struct pipe_image_view *img = &images[i];

      st_convert_image_from_unit(st, img, prog->sh.ImageUnits[i],
                                 prog->sh.image_access[i]);

      struct pipe_resource *res = img->resource;
      if (!res)
         continue;

      unsigned width, height, num_layers, row_stride, img_stride;
      void *addr;

      if (res->target != PIPE_BUFFER) {
         width = u_minify(res->width0, img->u.tex.level);
         height = u_minify(res->height0, img->u.tex.level);
         num_layers = img->u.tex.last_layer - img->u.tex.first_layer + 1;

         addr = pipe_texture_map_3d(pipe, res, img->u.tex.level,
                                     PIPE_MAP_READ, 0, 0,
                                     img->u.tex.first_layer,
                                     width, height, num_layers,
                                     &img_transfer[i]);
         row_stride = img_transfer[i]->stride;
         img_stride = img_transfer[i]->layer_stride;
      } else {
         width = img->u.buf.size / util_format_get_blocksize(img->format);

         /* probably don't really need to fill that out */
         row_stride = 0;
         img_stride = 0;
         height = num_layers = 1;

         addr = pipe_buffer_map_range(pipe, res, img->u.buf.offset,
                                      img->u.buf.size, PIPE_MAP_READ,
                                      &img_transfer[i]);
      }

      draw_set_mapped_image(draw, PIPE_SHADER_VERTEX, i, width, height,
                            num_layers, addr, row_stride, img_stride, 0, 0);
   }
   draw_set_images(draw, PIPE_SHADER_VERTEX, images, prog->info.num_images);

   /* draw here */
   for (i = 0; i < num_draws; i++) {
      draw_vbo(draw, info, info->increment_draw_id ? i : 0, indirect,
               &draws[i], 1, ctx->TessCtrlProgram.patch_vertices);
   }

   /* unmap images */
   for (unsigned i = 0; i < prog->info.num_images; i++) {
      if (img_transfer[i]) {
         draw_set_mapped_image(draw, PIPE_SHADER_VERTEX, i, 0, 0, 0, NULL, 0, 0, 0, 0);
         if (img_transfer[i]->resource->target == PIPE_BUFFER)
            pipe_buffer_unmap(pipe, img_transfer[i]);
         else
            pipe_texture_unmap(pipe, img_transfer[i]);
      }
   }

   /* unmap sampler views */
   for (unsigned i = 0; i < num_views; i++) {
      struct pipe_sampler_view *view = views[i];

      if (view) {
         if (view->texture->target != PIPE_BUFFER) {
            for (unsigned j = view->u.tex.first_level;
                 j <= view->u.tex.last_level; j++) {
               pipe_texture_unmap(pipe, sv_transfer[i][j]);
            }
         } else {
            pipe_buffer_unmap(pipe, sv_transfer[i][0]);
         }

         pipe_sampler_view_reference(&views[i], NULL);
      }
   }

   draw_set_samplers(draw, PIPE_SHADER_VERTEX, NULL, 0);
   draw_set_sampler_views(draw, PIPE_SHADER_VERTEX, NULL, 0);

   for (unsigned i = 0; i < prog->info.num_ssbos; i++) {
      if (ssbo_transfer[i]) {
         draw_set_mapped_constant_buffer(draw, PIPE_SHADER_VERTEX, 1 + i,
                                         NULL, 0);
         pipe_buffer_unmap(pipe, ssbo_transfer[i]);
      }
   }

   for (unsigned i = 0; i < prog->info.num_ubos; i++) {
      if (ubo_transfer[i]) {
         draw_set_mapped_constant_buffer(draw, PIPE_SHADER_VERTEX, 1 + i,
                                         NULL, 0);
         pipe_buffer_unmap(pipe, ubo_transfer[i]);
      }
   }

   /*
    * unmap vertex/index buffers
    */
   if (info->index_size) {
      draw_set_indexes(draw, NULL, 0, 0);
      if (ib_transfer)
         pipe_buffer_unmap(pipe, ib_transfer);
   }

   for (unsigned buf = 0; buf < num_vbuffers; ++buf) {
      if (vb_transfer[buf])
         pipe_buffer_unmap(pipe, vb_transfer[buf]);
      draw_set_mapped_vertex_buffer(draw, buf, NULL, 0);
      if (!vbuffers[buf].is_user_buffer)
         pipe_resource_reference(&vbuffers[buf].buffer.resource, NULL);
   }
   draw_set_vertex_buffers(draw, 0, num_vbuffers, NULL);

   draw_bind_vertex_shader(draw, NULL);
}

void
st_feedback_draw_vbo_multi_mode(struct gl_context *ctx,
                                struct pipe_draw_info *info,
                                const struct pipe_draw_start_count_bias *draws,
                                const unsigned char *mode,
                                unsigned num_draws)
{
   for (unsigned i = 0; i < num_draws; i++) {
      info->mode = mode[i];
      st_feedback_draw_vbo(ctx, info, 0, NULL, &draws[i], 1);
   }
}
