/**********************************************************
 * Copyright 2008-2022 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#include "util/compiler.h"
#include "util/u_inlines.h"
#include "pipe/p_defines.h"
#include "util/u_helpers.h"
#include "util/u_memory.h"
#include "util/u_math.h"

#include "svga_context.h"
#include "svga_draw.h"
#include "svga_draw_private.h"
#include "svga_debug.h"
#include "svga_screen.h"
#include "svga_resource.h"
#include "svga_resource_buffer.h"
#include "svga_resource_texture.h"
#include "svga_sampler_view.h"
#include "svga_shader.h"
#include "svga_surface.h"
#include "svga_winsys.h"
#include "svga_cmd.h"


struct svga_hwtnl *
svga_hwtnl_create(struct svga_context *svga)
{
   struct svga_hwtnl *hwtnl = CALLOC_STRUCT(svga_hwtnl);
   if (!hwtnl)
      goto fail;

   hwtnl->svga = svga;

   hwtnl->cmd.swc = svga->swc;

   return hwtnl;

fail:
   return NULL;
}


void
svga_hwtnl_destroy(struct svga_hwtnl *hwtnl)
{
   unsigned i, j;

   for (i = 0; i < MESA_PRIM_COUNT; i++) {
      for (j = 0; j < IDX_CACHE_MAX; j++) {
         pipe_resource_reference(&hwtnl->index_cache[i][j].buffer, NULL);
      }
   }

   for (i = 0; i < hwtnl->cmd.vbuf_count; i++)
      pipe_vertex_buffer_unreference(&hwtnl->cmd.vbufs[i]);

   for (i = 0; i < hwtnl->cmd.prim_count; i++)
      pipe_resource_reference(&hwtnl->cmd.prim_ib[i], NULL);

   FREE(hwtnl);
}


void
svga_hwtnl_set_flatshade(struct svga_hwtnl *hwtnl,
                         bool flatshade, bool flatshade_first)
{
   struct svga_screen *svgascreen = svga_screen(hwtnl->svga->pipe.screen);

   /* User-specified PV */
   hwtnl->api_pv = (flatshade && !flatshade_first) ? PV_LAST : PV_FIRST;

   /* Device supported PV */
   if (svgascreen->haveProvokingVertex) {
      /* use the mode specified by the user */
      hwtnl->hw_pv = hwtnl->api_pv;
   }
   else {
      /* the device only support first provoking vertex */
      hwtnl->hw_pv = PV_FIRST;
   }
}


void
svga_hwtnl_set_fillmode(struct svga_hwtnl *hwtnl, unsigned mode)
{
   hwtnl->api_fillmode = mode;
}


void
svga_hwtnl_vertex_decls(struct svga_hwtnl *hwtnl,
                        unsigned count,
                        const SVGA3dVertexDecl * decls,
                        const unsigned *buffer_indexes,
                        SVGA3dElementLayoutId layout_id)
{
   assert(hwtnl->cmd.prim_count == 0);
   hwtnl->cmd.vdecl_count = count;
   hwtnl->cmd.vdecl_layout_id = layout_id;
   memcpy(hwtnl->cmd.vdecl, decls, count * sizeof(*decls));
   memcpy(hwtnl->cmd.vdecl_buffer_index, buffer_indexes,
          count * sizeof(unsigned));
}


/**
 * Specify vertex buffers for hardware drawing.
 */
void
svga_hwtnl_vertex_buffers(struct svga_hwtnl *hwtnl,
                          unsigned count, struct pipe_vertex_buffer *buffers)
{
   struct pipe_vertex_buffer *dst = hwtnl->cmd.vbufs;
   const struct pipe_vertex_buffer *src = buffers;
   unsigned i;

   for (i = 0; i < count; i++) {
      pipe_vertex_buffer_reference(&dst[i], &src[i]);
   }

   /* release old buffer references */
   for ( ; i < hwtnl->cmd.vbuf_count; i++) {
      pipe_vertex_buffer_unreference(&dst[i]);
      /* don't bother zeroing stride/offset fields */
   }

   hwtnl->cmd.vbuf_count = count;
}


/**
 * Determine whether the specified buffer is referred in the primitive queue,
 * for which no commands have been written yet.
 */
bool
svga_hwtnl_is_buffer_referred(struct svga_hwtnl *hwtnl,
                              struct pipe_resource *buffer)
{
   unsigned i;

   if (svga_buffer_is_user_buffer(buffer)) {
      return false;
   }

   if (!hwtnl->cmd.prim_count) {
      return false;
   }

   for (i = 0; i < hwtnl->cmd.vbuf_count; ++i) {
      if (hwtnl->cmd.vbufs[i].buffer.resource == buffer) {
         return true;
      }
   }

   for (i = 0; i < hwtnl->cmd.prim_count; ++i) {
      if (hwtnl->cmd.prim_ib[i] == buffer) {
         return true;
      }
   }

   return false;
}


static enum pipe_error
draw_vgpu9(struct svga_hwtnl *hwtnl)
{
   struct svga_winsys_context *swc = hwtnl->cmd.swc;
   struct svga_context *svga = hwtnl->svga;
   enum pipe_error ret;
   struct svga_winsys_surface *vb_handle[SVGA3D_INPUTREG_MAX];
   struct svga_winsys_surface *ib_handle[QSZ];
   struct svga_winsys_surface *handle;
   SVGA3dVertexDecl *vdecl;
   SVGA3dPrimitiveRange *prim;
   unsigned i;

   /* Re-validate those sampler views with backing copy
    * of texture whose original copy has been updated.
    * This is done here at draw time because the texture binding might not
    * have modified, hence validation is not triggered at state update time,
    * and yet the texture might have been updated in another context, so
    * we need to re-validate the sampler view in order to update the backing
    * copy of the updated texture.
    */
   if (svga->state.hw_draw.num_backed_views) {
      for (i = 0; i < svga->state.hw_draw.num_views; i++) {
         struct svga_hw_view_state *view = &svga->state.hw_draw.views[i];
         struct svga_texture *tex = svga_texture(view->texture);
         struct svga_sampler_view *sv = view->v;
         if (sv && tex && sv->handle != tex->handle && sv->age < tex->age)
            svga_validate_sampler_view(svga, view->v);
      }
   }

   for (i = 0; i < hwtnl->cmd.vdecl_count; i++) {
      unsigned j = hwtnl->cmd.vdecl_buffer_index[i];
      handle = svga_buffer_handle(svga, hwtnl->cmd.vbufs[j].buffer.resource,
                                  PIPE_BIND_VERTEX_BUFFER);
      if (!handle)
         return PIPE_ERROR_OUT_OF_MEMORY;

      vb_handle[i] = handle;
   }

   for (i = 0; i < hwtnl->cmd.prim_count; i++) {
      if (hwtnl->cmd.prim_ib[i]) {
         handle = svga_buffer_handle(svga, hwtnl->cmd.prim_ib[i],
                                     PIPE_BIND_INDEX_BUFFER);
         if (!handle)
            return PIPE_ERROR_OUT_OF_MEMORY;
      }
      else
         handle = NULL;

      ib_handle[i] = handle;
   }

   if (svga->rebind.flags.rendertargets) {
      ret = svga_reemit_framebuffer_bindings(svga);
      if (ret != PIPE_OK) {
         return ret;
      }
   }

   if (svga->rebind.flags.texture_samplers) {
      ret = svga_reemit_tss_bindings(svga);
      if (ret != PIPE_OK) {
         return ret;
      }
   }

   if (svga->rebind.flags.vs) {
      ret = svga_reemit_vs_bindings(svga);
      if (ret != PIPE_OK) {
         return ret;
      }
   }

   if (svga->rebind.flags.fs) {
      ret = svga_reemit_fs_bindings(svga);
      if (ret != PIPE_OK) {
         return ret;
      }
   }

   SVGA_DBG(DEBUG_DMA, "draw to sid %p, %d prims\n",
            svga->curr.framebuffer.cbufs[0] ?
            svga_surface(svga->curr.framebuffer.cbufs[0])->handle : NULL,
            hwtnl->cmd.prim_count);

   ret = SVGA3D_BeginDrawPrimitives(swc,
                                    &vdecl,
                                    hwtnl->cmd.vdecl_count,
                                    &prim, hwtnl->cmd.prim_count);
   if (ret != PIPE_OK)
      return ret;

   memcpy(vdecl,
          hwtnl->cmd.vdecl,
          hwtnl->cmd.vdecl_count * sizeof hwtnl->cmd.vdecl[0]);

   for (i = 0; i < hwtnl->cmd.vdecl_count; i++) {
      /* check for 4-byte alignment */
      assert(vdecl[i].array.offset % 4 == 0);
      assert(vdecl[i].array.stride % 4 == 0);

      /* Given rangeHint is considered to be relative to indexBias, and
       * indexBias varies per primitive, we cannot accurately supply an
       * rangeHint when emitting more than one primitive per draw command.
       */
      if (hwtnl->cmd.prim_count == 1) {
         vdecl[i].rangeHint.first = hwtnl->cmd.min_index[0];
         vdecl[i].rangeHint.last = hwtnl->cmd.max_index[0] + 1;
      }
      else {
         vdecl[i].rangeHint.first = 0;
         vdecl[i].rangeHint.last = 0;
      }

      swc->surface_relocation(swc,
                              &vdecl[i].array.surfaceId,
                              NULL, vb_handle[i], SVGA_RELOC_READ);
   }

   memcpy(prim,
          hwtnl->cmd.prim, hwtnl->cmd.prim_count * sizeof hwtnl->cmd.prim[0]);

   for (i = 0; i < hwtnl->cmd.prim_count; i++) {
      swc->surface_relocation(swc,
                              &prim[i].indexArray.surfaceId,
                              NULL, ib_handle[i], SVGA_RELOC_READ);
      pipe_resource_reference(&hwtnl->cmd.prim_ib[i], NULL);
   }

   SVGA_FIFOCommitAll(swc);

   hwtnl->cmd.prim_count = 0;

   return PIPE_OK;
}


static SVGA3dSurfaceFormat
xlate_index_format(unsigned indexWidth)
{
   if (indexWidth == 2) {
      return SVGA3D_R16_UINT;
   }
   else if (indexWidth == 4) {
      return SVGA3D_R32_UINT;
   }
   else {
      assert(!"Bad indexWidth");
      return SVGA3D_R32_UINT;
   }
}


/**
 * A helper function to validate sampler view resources to ensure any
 * pending updates to buffers will be emitted before they are referenced
 * at draw or dispatch time. It also rebinds the resources if needed.
 */
enum pipe_error
svga_validate_sampler_resources(struct svga_context *svga,
                                enum svga_pipe_type pipe_type)
{
   enum pipe_shader_type shader, first_shader, last_shader;

   assert(svga_have_vgpu10(svga));

   if (pipe_type == SVGA_PIPE_GRAPHICS) {
      first_shader = PIPE_SHADER_VERTEX;
      last_shader = PIPE_SHADER_COMPUTE;
   }
   else {
      assert(svga_have_gl43(svga));
      first_shader = PIPE_SHADER_COMPUTE;
      last_shader = first_shader+1;
   }

   for (shader = first_shader; shader < last_shader; shader++) {
      unsigned count = svga->curr.num_sampler_views[shader];
      unsigned i;
      struct svga_winsys_surface *surfaces[PIPE_MAX_SAMPLERS];
      enum pipe_error ret;

      /*
       * Reference bound sampler resources to ensure pending updates are
       * noticed by the device.
       */
      for (i = 0; i < count; i++) {
         struct svga_pipe_sampler_view *sv =
            svga_pipe_sampler_view(svga->curr.sampler_views[shader][i]);

         if (sv) {
            if (sv->base.texture->target == PIPE_BUFFER) {
               surfaces[i] = svga_buffer_handle(svga, sv->base.texture,
                                                PIPE_BIND_SAMPLER_VIEW);
            }
            else {
               surfaces[i] = svga_texture(sv->base.texture)->handle;
            }
         }
         else {
            surfaces[i] = NULL;
         }
      }

      if (shader == PIPE_SHADER_FRAGMENT &&
          svga->curr.rast->templ.poly_stipple_enable) {
         const unsigned unit =
            svga_fs_variant(svga->state.hw_draw.fs)->pstipple_sampler_unit;
         struct svga_pipe_sampler_view *sv =
            svga->polygon_stipple.sampler_view;

         assert(sv);
         surfaces[unit] = svga_texture(sv->base.texture)->handle;
         count = MAX2(count, unit+1);
      }

      /* rebind the shader resources if needed */
      if (svga->rebind.flags.texture_samplers) {
         for (i = 0; i < count; i++) {
            if (surfaces[i]) {
               ret = svga->swc->resource_rebind(svga->swc,
                                                surfaces[i],
                                                NULL,
                                                SVGA_RELOC_READ);
               if (ret != PIPE_OK)
                  return ret;
            }
         }
      }
   }
   svga->rebind.flags.texture_samplers = false;

   return PIPE_OK;
}


/**
 * A helper function to validate constant buffers to ensure any
 * pending updates to the buffers will be emitted before they are referenced
 * at draw or dispatch time. It also rebinds the resources if needed.
 */
enum pipe_error
svga_validate_constant_buffers(struct svga_context *svga,
                               enum svga_pipe_type pipe_type)
{
   enum pipe_shader_type shader, first_shader, last_shader;

   assert(svga_have_vgpu10(svga));

   if (pipe_type == SVGA_PIPE_GRAPHICS) {
      first_shader = PIPE_SHADER_VERTEX;
      last_shader = PIPE_SHADER_COMPUTE;
   }
   else {
      assert(svga_have_gl43(svga));
      first_shader = PIPE_SHADER_COMPUTE;
      last_shader = first_shader + 1;
   }

   for (shader = first_shader; shader < last_shader; shader++) {

      enum pipe_error ret;
      struct svga_buffer *buffer;

      /* Rebind the default constant buffer if needed */
      if (svga->rebind.flags.constbufs) {
         buffer = svga_buffer(svga->state.hw_draw.constbuf[shader][0]);
         if (buffer) {
            ret = svga->swc->resource_rebind(svga->swc,
                                             buffer->handle,
                                             NULL,
                                             SVGA_RELOC_READ);
            if (ret != PIPE_OK)
               return ret;
         }
      }

      struct svga_winsys_surface *handle;
      unsigned enabled_constbufs;

      /*
       * Reference other bound constant buffers to ensure pending updates are
       * noticed by the device.
       */
      enabled_constbufs = svga->state.hw_draw.enabled_constbufs[shader] & ~1u;
      while (enabled_constbufs) {
         unsigned i = u_bit_scan(&enabled_constbufs);
         buffer = svga_buffer(svga->curr.constbufs[shader][i].buffer);

         /* If the constant buffer has hw storage, get the buffer winsys handle.
          * Rebind the resource if needed.
          */
         if (buffer && !buffer->use_swbuf)
            handle = svga_buffer_handle(svga, &buffer->b,
                                        PIPE_BIND_CONSTANT_BUFFER);
         else
            handle = svga->state.hw_draw.constbufoffsets[shader][i].handle;

         if (svga->rebind.flags.constbufs && handle) {
            ret = svga->swc->resource_rebind(svga->swc,
                                             handle,
                                             NULL,
                                             SVGA_RELOC_READ);
            if (ret != PIPE_OK)
               return ret;
         }
      }

      /* Reference raw constant buffers as they are not included in the
       * hw constant buffers list.
       */
      unsigned enabled_rawbufs = svga->state.hw_draw.enabled_rawbufs[shader] & ~1u;
      while (enabled_rawbufs) {
         unsigned i = u_bit_scan(&enabled_rawbufs);
         buffer = svga_buffer(svga->state.hw_draw.rawbufs[shader][i].buffer);

         assert(buffer != NULL);
         handle = svga_buffer_handle(svga, &buffer->b,
                                     PIPE_BIND_SAMPLER_VIEW);

         if (svga->rebind.flags.constbufs && handle) {
            ret = svga->swc->resource_rebind(svga->swc,
                                             handle,
                                             NULL,
                                             SVGA_RELOC_READ);
            if (ret != PIPE_OK)
               return ret;
         }
      }
   }
   svga->rebind.flags.constbufs = false;

   return PIPE_OK;
}


/**
 * A helper function to validate image view resources to ensure any
 * pending updates to buffers will be emitted before they are referenced
 * at draw or dispatch time. It also rebinds the resources if needed.
 */
enum pipe_error
svga_validate_image_views(struct svga_context *svga,
                          enum svga_pipe_type pipe_type)
{
   enum pipe_shader_type shader, first_shader, last_shader;
   bool rebind = svga->rebind.flags.images;
   enum pipe_error ret;

   assert(svga_have_gl43(svga));

   if (pipe_type == SVGA_PIPE_GRAPHICS) {
      first_shader = PIPE_SHADER_VERTEX;
      last_shader = PIPE_SHADER_COMPUTE;
   }
   else {
      first_shader = PIPE_SHADER_COMPUTE;
      last_shader = first_shader + 1;
   }

   for (shader = first_shader; shader < last_shader; shader++) {
      ret = svga_validate_image_view_resources(svga,
               svga->state.hw_draw.num_image_views[shader],
               &svga->state.hw_draw.image_views[shader][0], rebind);

      if (ret != PIPE_OK)
         return ret;
   }

   svga->rebind.flags.images = false;

   return PIPE_OK;
}


/**
 * A helper function to validate shader buffer and atomic buffer resources to
 * ensure any pending updates to buffers will be emitted before they are
 * referenced at draw or dispatch time. It also rebinds the resources if needed.
 */
enum pipe_error
svga_validate_shader_buffers(struct svga_context *svga,
                             enum svga_pipe_type pipe_type)
{
   enum pipe_shader_type shader, first_shader, last_shader;
   bool rebind = svga->rebind.flags.shaderbufs;
   enum pipe_error ret;

   assert(svga_have_gl43(svga));

   if (pipe_type == SVGA_PIPE_GRAPHICS) {
      first_shader = PIPE_SHADER_VERTEX;
      last_shader = PIPE_SHADER_COMPUTE;
   }
   else {
      first_shader = PIPE_SHADER_COMPUTE;
      last_shader = first_shader + 1;
   }

   for (shader = first_shader; shader < last_shader; shader++) {
      ret = svga_validate_shader_buffer_resources(svga,
               svga->state.hw_draw.num_shader_buffers[shader],
               &svga->state.hw_draw.shader_buffers[shader][0], rebind);

      if (ret != PIPE_OK)
         return ret;
   }

   svga->rebind.flags.shaderbufs = false;

   ret = svga_validate_shader_buffer_resources(svga,
               svga->state.hw_draw.num_atomic_buffers,
               svga->state.hw_draw.atomic_buffers,
               svga->rebind.flags.atomicbufs);

   if (ret != PIPE_OK)
      return ret;

   svga->rebind.flags.atomicbufs = false;

   return PIPE_OK;
}


/**
 * Was the last command put into the command buffer a drawing command?
 * We use this to determine if we can skip emitting buffer re-bind
 * commands when we have a sequence of drawing commands that use the
 * same vertex/index buffers with no intervening commands.
 *
 * The first drawing command will bind the vertex/index buffers.  If
 * the immediately following command is also a drawing command using the
 * same buffers, we shouldn't have to rebind them.
 */
static bool
last_command_was_draw(const struct svga_context *svga)
{
   switch (SVGA3D_GetLastCommand(svga->swc)) {
   case SVGA_3D_CMD_DX_DRAW:
   case SVGA_3D_CMD_DX_DRAW_INDEXED:
   case SVGA_3D_CMD_DX_DRAW_INSTANCED:
   case SVGA_3D_CMD_DX_DRAW_INDEXED_INSTANCED:
   case SVGA_3D_CMD_DX_DRAW_AUTO:
   case SVGA_3D_CMD_DX_DRAW_INDEXED_INSTANCED_INDIRECT:
   case SVGA_3D_CMD_DX_DRAW_INSTANCED_INDIRECT:
      return true;
   default:
      return false;
   }
}


/**
 * A helper function to compare vertex buffers.
 * They are equal if the vertex buffer attributes and the vertex buffer
 * resources are identical.
 */
static bool
vertex_buffers_equal(unsigned count,
                     SVGA3dVertexBuffer_v2 *pVBufAttr1,
                     struct pipe_resource **pVBuf1,
                     SVGA3dVertexBuffer_v2 *pVBufAttr2,
                     struct pipe_resource **pVBuf2)
{
   return (memcmp(pVBufAttr1, pVBufAttr2,
                  count * sizeof(*pVBufAttr1)) == 0) &&
          (memcmp(pVBuf1, pVBuf2, count * sizeof(*pVBuf1)) == 0);
}


/*
 * Prepare the vertex buffers for a drawing command.
 */
static enum pipe_error
validate_vertex_buffers(struct svga_hwtnl *hwtnl,
                   const struct pipe_stream_output_target *so_vertex_count)
{
   struct svga_context *svga = hwtnl->svga;
   struct pipe_resource *vbuffers[SVGA3D_INPUTREG_MAX];
   struct svga_winsys_surface *vbuffer_handles[SVGA3D_INPUTREG_MAX];
   struct svga_winsys_surface *so_vertex_count_handle = NULL;
   const unsigned vbuf_count = so_vertex_count ? 1 : hwtnl->cmd.vbuf_count;
   SVGA3dVertexBuffer_v2 vbuffer_attrs[PIPE_MAX_ATTRIBS];
   int last_vbuf = -1;
   unsigned i;

   assert(svga_have_vgpu10(svga));

   /* setup vertex attribute input layout */
   if (svga->state.hw_draw.layout_id != hwtnl->cmd.vdecl_layout_id) {
      enum pipe_error ret =
         SVGA3D_vgpu10_SetInputLayout(svga->swc,
                                      hwtnl->cmd.vdecl_layout_id);
      if (ret != PIPE_OK)
         return ret;

      svga->state.hw_draw.layout_id = hwtnl->cmd.vdecl_layout_id;
   }

   /* Get handle for each referenced vertex buffer, unless we're using a
    * stream-out buffer to specify the drawing information (DrawAuto).
    * Also set up the buffer attributes.
    */
   if (so_vertex_count) {
      so_vertex_count_handle = svga_buffer_handle(svga,
                                                  so_vertex_count->buffer,
                                                  (PIPE_BIND_VERTEX_BUFFER |
                                                   PIPE_BIND_STREAM_OUTPUT));
      if (!so_vertex_count_handle)
         return PIPE_ERROR_OUT_OF_MEMORY;

      /* Set IA slot0 input buffer to the SO buffer */
      assert(vbuf_count == 1);
      vbuffer_attrs[0].stride = svga->curr.velems->strides[0];
      vbuffer_attrs[0].offset = hwtnl->cmd.vbufs[0].buffer_offset;
      vbuffer_attrs[0].sid = 0;
      assert(so_vertex_count->buffer != NULL);
      vbuffer_attrs[0].sizeInBytes = svga_buffer(so_vertex_count->buffer)->size;
      vbuffers[0] = so_vertex_count->buffer;
      vbuffer_handles[0] = so_vertex_count_handle;

      i = 1;
   }
   else {
      for (i = 0; i < vbuf_count; i++) {
         struct svga_buffer *sbuf =
            svga_buffer(hwtnl->cmd.vbufs[i].buffer.resource);

         vbuffer_attrs[i].stride = svga->curr.velems->strides[i];
         vbuffer_attrs[i].offset = hwtnl->cmd.vbufs[i].buffer_offset;
         vbuffer_attrs[i].sid = 0;

         if (sbuf) {
            vbuffer_handles[i] = svga_buffer_handle(svga, &sbuf->b,
                                                    PIPE_BIND_VERTEX_BUFFER);
            assert(sbuf->key.flags & SVGA3D_SURFACE_BIND_VERTEX_BUFFER);
            if (vbuffer_handles[i] == NULL)
               return PIPE_ERROR_OUT_OF_MEMORY;
            vbuffers[i] = &sbuf->b;
            last_vbuf = i;

            vbuffer_attrs[i].sizeInBytes = sbuf->size;
         }
         else {
            vbuffers[i] = NULL;
            vbuffer_handles[i] = NULL;
            vbuffer_attrs[i].sizeInBytes = 0;
         }
      }
   }

   /* Unbind the unreferenced the vertex buffer handles */
   for (; i < svga->state.hw_draw.num_vbuffers; i++) {
      vbuffers[i] = NULL;
      vbuffer_handles[i] = NULL;
      vbuffer_attrs[i].sid = 0;
      vbuffer_attrs[i].stride = 0;
      vbuffer_attrs[i].offset = 0;
      vbuffer_attrs[i].sizeInBytes = 0;
   }

   /* Get handle for each referenced vertex buffer */
   for (i = 0; i < vbuf_count; i++) {
      struct svga_buffer *sbuf =
         svga_buffer(hwtnl->cmd.vbufs[i].buffer.resource);

      if (sbuf) {
         vbuffer_handles[i] = svga_buffer_handle(svga, &sbuf->b,
                                                 PIPE_BIND_VERTEX_BUFFER);
         assert(sbuf->key.flags & SVGA3D_SURFACE_BIND_VERTEX_BUFFER);
         if (vbuffer_handles[i] == NULL)
            return PIPE_ERROR_OUT_OF_MEMORY;
         vbuffers[i] = &sbuf->b;
         last_vbuf = i;
      }
      else {
         vbuffers[i] = NULL;
         vbuffer_handles[i] = NULL;
      }
   }

   for (; i < svga->state.hw_draw.num_vbuffers; i++) {
      vbuffers[i] = NULL;
      vbuffer_handles[i] = NULL;
   }

   /* setup vertex attribute input layout */
   if (svga->state.hw_draw.layout_id != hwtnl->cmd.vdecl_layout_id) {
      enum pipe_error ret =
         SVGA3D_vgpu10_SetInputLayout(svga->swc,
                                      hwtnl->cmd.vdecl_layout_id);
      if (ret != PIPE_OK)
         return ret;

      svga->state.hw_draw.layout_id = hwtnl->cmd.vdecl_layout_id;
   }

   /* Get handle for the stream out buffer */
   if (so_vertex_count) {
      so_vertex_count_handle = svga_buffer_handle(svga,
                                                  so_vertex_count->buffer,
                                                  (PIPE_BIND_VERTEX_BUFFER |
                                                   PIPE_BIND_STREAM_OUTPUT));
      if (!so_vertex_count_handle)
         return PIPE_ERROR_OUT_OF_MEMORY;
   }
   else {
      so_vertex_count_handle = NULL;
   }

   /* setup vertex buffers */
   {
      /* If any of the vertex buffer state has changed, issue
       * the SetVertexBuffers command. Otherwise, we will just
       * need to rebind the resources.
       */
      if (vbuf_count != svga->state.hw_draw.num_vbuffers ||
          !vertex_buffers_equal(vbuf_count,
                                vbuffer_attrs,
                                vbuffers,
                                svga->state.hw_draw.vbuffer_attrs,
                                svga->state.hw_draw.vbuffers)) {

         unsigned num_vbuffers;

         /* get the max of the current bound vertex buffers count and
          * the to-be-bound vertex buffers count, so as to unbind
          * the unused vertex buffers.
          */
         num_vbuffers = MAX2(vbuf_count, svga->state.hw_draw.num_vbuffers);

         if (num_vbuffers > 0) {
            SVGA3dVertexBuffer_v2 *pbufAttrs = vbuffer_attrs;
            struct svga_winsys_surface **pbufHandles = vbuffer_handles;
            unsigned numVBuf = 0;
            bool emitVBufs =
               !svga_sws(svga)->have_index_vertex_buffer_offset_cmd ||
               svga->rebind.flags.vertexbufs;

            /* Loop through the vertex buffer lists to only emit
             * those vertex buffers that are not already in the
             * corresponding entries in the device's vertex buffer list.
             */
            for (i = 0; i < num_vbuffers; i++) {
               bool emit =
                  vertex_buffers_equal(1,
                                       &vbuffer_attrs[i],
                                       &vbuffers[i],
                                       &svga->state.hw_draw.vbuffer_attrs[i],
                                       &svga->state.hw_draw.vbuffers[i]);

               /* Check if we can use the SetVertexBuffersOffsetAndSize command */
               emitVBufs = emitVBufs ||
                              (vbuffers[i] != svga->state.hw_draw.vbuffers[i]);

               if (!emit && i == num_vbuffers-1) {
                  /* Include the last vertex buffer in the next emit
                   * if it is different.
                   */
                  emit = true;
                  numVBuf++;
                  i++;
               }

               if (emit) {
                  /* numVBuf can only be 0 if the first vertex buffer
                   * is the same as the one in the device's list.
                   * In this case, there is nothing to send yet.
                   */
                  if (numVBuf) {
                     enum pipe_error ret;

                     /* If all vertex buffers handle are the same as the one
                      * in the device, just use the
                      * SetVertexBuffersOffsetAndSize comand.
                      */
                     if (emitVBufs) {
                        ret = SVGA3D_vgpu10_SetVertexBuffers(svga->swc,
                                                             numVBuf,
                                                             i - numVBuf,
                                                             pbufAttrs, pbufHandles);
                     } else {
                        ret = SVGA3D_vgpu10_SetVertexBuffersOffsetAndSize(svga->swc,
                                                             numVBuf,
                                                             i - numVBuf,
                                                             pbufAttrs);
                     }
                     if (ret != PIPE_OK)
                        return ret;
                  }
                  pbufAttrs += (numVBuf + 1);
                  pbufHandles += (numVBuf + 1);
                  numVBuf = 0;
               }
               else
                  numVBuf++;
            }

            /* save the number of vertex buffers sent to the device, not
             * including trailing unbound vertex buffers.
             */
            svga->state.hw_draw.num_vbuffers = last_vbuf + 1;
            memcpy(svga->state.hw_draw.vbuffer_attrs, vbuffer_attrs,
                   num_vbuffers * sizeof(vbuffer_attrs[0]));
            for (i = 0; i < num_vbuffers; i++) {
               pipe_resource_reference(&svga->state.hw_draw.vbuffers[i],
                                       vbuffers[i]);
            }
         }
      }
      else {
         /* Even though we can avoid emitting the redundant SetVertexBuffers
          * command, we still need to reference the vertex buffers surfaces.
          */
         for (i = 0; i < vbuf_count; i++) {
            if (vbuffer_handles[i] && !last_command_was_draw(svga)) {
               enum pipe_error ret =
                  svga->swc->resource_rebind(svga->swc, vbuffer_handles[i],
                                             NULL, SVGA_RELOC_READ);
               if (ret != PIPE_OK)
                  return ret;
            }
         }
      }
   }

   svga->rebind.flags.vertexbufs = false;

   return PIPE_OK;
}


/*
 * Prepare the index buffer for a drawing command.
 */
static enum pipe_error
validate_index_buffer(struct svga_hwtnl *hwtnl,
                      const SVGA3dPrimitiveRange *range,
                      struct pipe_resource *ib)
{
   struct svga_context *svga = hwtnl->svga;
   struct svga_winsys_surface *ib_handle =
      svga_buffer_handle(svga, ib, PIPE_BIND_INDEX_BUFFER);
   enum pipe_error ret;

   if (!ib_handle)
      return PIPE_ERROR_OUT_OF_MEMORY;

   struct svga_buffer *sbuf = svga_buffer(ib);
   assert(sbuf->key.flags & SVGA3D_SURFACE_BIND_INDEX_BUFFER);
   (void) sbuf; /* silence unused var warning */

   SVGA3dSurfaceFormat indexFormat = xlate_index_format(range->indexWidth);

   if (ib != svga->state.hw_draw.ib ||
       indexFormat != svga->state.hw_draw.ib_format ||
       range->indexArray.offset != svga->state.hw_draw.ib_offset) {

      assert(indexFormat != SVGA3D_FORMAT_INVALID);

      if ((ib == svga->state.hw_draw.ib) &&
          svga_sws(hwtnl->svga)->have_index_vertex_buffer_offset_cmd &&
          !svga->rebind.flags.indexbuf) {

         ret = SVGA3D_vgpu10_SetIndexBufferOffsetAndSize(svga->swc,
                                                         indexFormat,
                                                         range->indexArray.offset,
                                                         sbuf->size);
         if (ret != PIPE_OK)
            return ret;
      }
      else {

         ret = SVGA3D_vgpu10_SetIndexBuffer(svga->swc, ib_handle,
                                            indexFormat,
                                            range->indexArray.offset);
         if (ret != PIPE_OK)
            return ret;
      }

      pipe_resource_reference(&svga->state.hw_draw.ib, ib);
      svga->state.hw_draw.ib_format = indexFormat;
      svga->state.hw_draw.ib_offset = range->indexArray.offset;
   }
   else {
      /* Even though we can avoid emitting the redundant SetIndexBuffer
       * command, we still need to reference the index buffer surface.
       */
      if (!last_command_was_draw(svga)) {
         enum pipe_error ret = svga->swc->resource_rebind(svga->swc,
                                                          ib_handle,
                                                          NULL,
                                                          SVGA_RELOC_READ);
         if (ret != PIPE_OK)
            return ret;
      }
   }

   svga->rebind.flags.indexbuf = false;

   return PIPE_OK;
}


static enum pipe_error
draw_vgpu10(struct svga_hwtnl *hwtnl,
            const SVGA3dPrimitiveRange *range,
            unsigned vcount,
            unsigned min_index, unsigned max_index,
            struct pipe_resource *ib,
            unsigned start_instance, unsigned instance_count,
            const struct pipe_draw_indirect_info *indirect,
            const struct pipe_stream_output_target *so_vertex_count)
{
   struct svga_context *svga = hwtnl->svga;
   struct svga_winsys_surface *indirect_handle;
   enum pipe_error ret;

   assert(svga_have_vgpu10(svga));
   assert(hwtnl->cmd.prim_count == 0);

   /* We need to reemit all the current resource bindings along with the Draw
    * command to be sure that the referenced resources are available for the
    * Draw command, just in case the surfaces associated with the resources
    * are paged out.
    */
   if (svga->rebind.val) {
      ret = svga_rebind_framebuffer_bindings(svga);
      if (ret != PIPE_OK)
         return ret;

      ret = svga_rebind_shaders(svga);
      if (ret != PIPE_OK)
         return ret;

      /* Rebind stream output targets */
      ret = svga_rebind_stream_output_targets(svga);
      if (ret != PIPE_OK)
         return ret;

      /* No need to explicitly rebind index buffer and vertex buffers here.
       * Even if the same index buffer or vertex buffers are referenced for this
       * draw and we skip emitting the redundant set command, we will still
       * reference the associated resources.
       */
   }

   ret = svga_validate_sampler_resources(svga, SVGA_PIPE_GRAPHICS);
   if (ret != PIPE_OK)
      return ret;

   ret = svga_validate_constant_buffers(svga, SVGA_PIPE_GRAPHICS);
   if (ret != PIPE_OK)
      return ret;

   if (svga_have_gl43(svga)) {
      ret = svga_validate_image_views(svga, SVGA_PIPE_GRAPHICS);
      if (ret != PIPE_OK)
         return ret;

      ret = svga_validate_shader_buffers(svga, SVGA_PIPE_GRAPHICS);
      if (ret != PIPE_OK)
         return ret;

      if (svga->rebind.flags.uav) {
         ret= svga_rebind_uav(svga);
         if (ret != PIPE_OK)
            return ret;
      }
   }

   ret = validate_vertex_buffers(hwtnl, so_vertex_count);
   if (ret != PIPE_OK)
      return ret;

   if (ib) {
      ret = validate_index_buffer(hwtnl, range, ib);
      if (ret != PIPE_OK)
         return ret;
   }

   if (indirect) {
      indirect_handle = svga_buffer_handle(svga, indirect->buffer,
                                           PIPE_BIND_COMMAND_ARGS_BUFFER);
      if (!indirect_handle)
         return PIPE_ERROR_OUT_OF_MEMORY;
   }
   else {
      indirect_handle = NULL;
   }

   /* Set primitive type (line, tri, etc) */
   if (svga->state.hw_draw.topology != range->primType) {
      ret = SVGA3D_vgpu10_SetTopology(svga->swc, range->primType);
      if (ret != PIPE_OK)
         return ret;

      svga->state.hw_draw.topology = range->primType;
   }

   if (ib) {
      /* indexed drawing */
      if (indirect) {
         ret = SVGA3D_sm5_DrawIndexedInstancedIndirect(svga->swc,
                                                       indirect_handle,
                                                       indirect->offset);
      }
      else if (instance_count > 1) {
         ret = SVGA3D_vgpu10_DrawIndexedInstanced(svga->swc,
                                                  vcount,
                                                  instance_count,
                                                  0, /* startIndexLocation */
                                                  range->indexBias,
                                                  start_instance);
      }
      else {
         /* non-instanced drawing */
         ret = SVGA3D_vgpu10_DrawIndexed(svga->swc,
                                         vcount,
                                         0,      /* startIndexLocation */
                                         range->indexBias);
      }
      if (ret != PIPE_OK) {
         return ret;
      }
   }
   else {
      /* non-indexed drawing */
      if (svga->state.hw_draw.ib_format != SVGA3D_FORMAT_INVALID ||
          svga->state.hw_draw.ib != NULL) {
         /* Unbind previously bound index buffer */
         ret = SVGA3D_vgpu10_SetIndexBuffer(svga->swc, NULL,
                                            SVGA3D_FORMAT_INVALID, 0);
         if (ret != PIPE_OK)
            return ret;
         pipe_resource_reference(&svga->state.hw_draw.ib, NULL);
         svga->state.hw_draw.ib_format = SVGA3D_FORMAT_INVALID;
      }

      assert(svga->state.hw_draw.ib == NULL);

      if (so_vertex_count) {
         /* Stream-output drawing */
         ret = SVGA3D_vgpu10_DrawAuto(svga->swc);
      }
      else if (indirect) {
         ret = SVGA3D_sm5_DrawInstancedIndirect(svga->swc,
                                                indirect_handle,
                                                indirect->offset);
      }
      else if (instance_count > 1) {
         ret = SVGA3D_vgpu10_DrawInstanced(svga->swc,
                                           vcount,
                                           instance_count,
                                           range->indexBias,
                                           start_instance);
      }
      else {
         /* non-instanced */
         ret = SVGA3D_vgpu10_Draw(svga->swc,
                                  vcount,
                                  range->indexBias);
      }
      if (ret != PIPE_OK) {
         return ret;
      }
   }

   hwtnl->cmd.prim_count = 0;

   return PIPE_OK;
}



/**
 * Emit any pending drawing commands to the command buffer.
 * When we receive VGPU9 drawing commands we accumulate them and don't
 * immediately emit them into the command buffer.
 * This function needs to be called before we change state that could
 * effect those pending draws.
 */
enum pipe_error
svga_hwtnl_flush(struct svga_hwtnl *hwtnl)
{
   enum pipe_error ret = PIPE_OK;

   SVGA_STATS_TIME_PUSH(svga_sws(hwtnl->svga), SVGA_STATS_TIME_HWTNLFLUSH);

   if (!svga_have_vgpu10(hwtnl->svga) && hwtnl->cmd.prim_count) {
      /* we only queue up primitive for VGPU9 */
      ret = draw_vgpu9(hwtnl);
   }

   SVGA_STATS_TIME_POP(svga_screen(hwtnl->svga->pipe.screen)->sws);
   return ret;
}


void
svga_hwtnl_set_index_bias(struct svga_hwtnl *hwtnl, int index_bias)
{
   hwtnl->index_bias = index_bias;
}



/***********************************************************************
 * Internal functions:
 */

/**
 * For debugging only.
 */
static void
check_draw_params(struct svga_hwtnl *hwtnl,
                  const SVGA3dPrimitiveRange *range,
                  unsigned min_index, unsigned max_index,
                  struct pipe_resource *ib)
{
   unsigned i;

   assert(!svga_have_vgpu10(hwtnl->svga));

   for (i = 0; i < hwtnl->cmd.vdecl_count; i++) {
      unsigned j = hwtnl->cmd.vdecl_buffer_index[i];
      const struct pipe_vertex_buffer *vb = &hwtnl->cmd.vbufs[j];
      unsigned size = vb->buffer.resource ? vb->buffer.resource->width0 : 0;
      unsigned offset = hwtnl->cmd.vdecl[i].array.offset;
      unsigned stride = hwtnl->cmd.vdecl[i].array.stride;
      int index_bias = (int) range->indexBias + hwtnl->index_bias;
      unsigned width;

      if (size == 0)
         continue;

      assert(vb);
      assert(size);
      assert(offset < size);
      assert(min_index <= max_index);
      (void) width;
      (void) stride;
      (void) offset;
      (void) size;

      switch (hwtnl->cmd.vdecl[i].identity.type) {
      case SVGA3D_DECLTYPE_FLOAT1:
         width = 4;
         break;
      case SVGA3D_DECLTYPE_FLOAT2:
         width = 4 * 2;
         break;
      case SVGA3D_DECLTYPE_FLOAT3:
         width = 4 * 3;
         break;
      case SVGA3D_DECLTYPE_FLOAT4:
         width = 4 * 4;
         break;
      case SVGA3D_DECLTYPE_D3DCOLOR:
         width = 4;
         break;
      case SVGA3D_DECLTYPE_UBYTE4:
         width = 1 * 4;
         break;
      case SVGA3D_DECLTYPE_SHORT2:
         width = 2 * 2;
         break;
      case SVGA3D_DECLTYPE_SHORT4:
         width = 2 * 4;
         break;
      case SVGA3D_DECLTYPE_UBYTE4N:
         width = 1 * 4;
         break;
      case SVGA3D_DECLTYPE_SHORT2N:
         width = 2 * 2;
         break;
      case SVGA3D_DECLTYPE_SHORT4N:
         width = 2 * 4;
         break;
      case SVGA3D_DECLTYPE_USHORT2N:
         width = 2 * 2;
         break;
      case SVGA3D_DECLTYPE_USHORT4N:
         width = 2 * 4;
         break;
      case SVGA3D_DECLTYPE_UDEC3:
         width = 4;
         break;
      case SVGA3D_DECLTYPE_DEC3N:
         width = 4;
         break;
      case SVGA3D_DECLTYPE_FLOAT16_2:
         width = 2 * 2;
         break;
      case SVGA3D_DECLTYPE_FLOAT16_4:
         width = 2 * 4;
         break;
      default:
         assert(0);
         width = 0;
         break;
      }

      if (index_bias >= 0) {
         assert(offset + index_bias * stride + width <= size);
      }

      /*
       * min_index/max_index are merely conservative guesses, so we can't
       * make buffer overflow detection based on their values.
       */
   }

   assert(range->indexWidth == range->indexArray.stride);

   if (ib) {
      ASSERTED unsigned size = ib->width0;
      ASSERTED unsigned offset = range->indexArray.offset;
      ASSERTED unsigned stride = range->indexArray.stride;
      ASSERTED unsigned count;

      assert(size);
      assert(offset < size);
      assert(stride);

      switch (range->primType) {
      case SVGA3D_PRIMITIVE_POINTLIST:
         count = range->primitiveCount;
         break;
      case SVGA3D_PRIMITIVE_LINELIST:
         count = range->primitiveCount * 2;
         break;
      case SVGA3D_PRIMITIVE_LINESTRIP:
         count = range->primitiveCount + 1;
         break;
      case SVGA3D_PRIMITIVE_TRIANGLELIST:
         count = range->primitiveCount * 3;
         break;
      case SVGA3D_PRIMITIVE_TRIANGLESTRIP:
         count = range->primitiveCount + 2;
         break;
      case SVGA3D_PRIMITIVE_TRIANGLEFAN:
         count = range->primitiveCount + 2;
         break;
      default:
         assert(0);
         count = 0;
         break;
      }

      assert(offset + count * stride <= size);
   }
}


/**
 * All drawing filters down into this function, either directly
 * on the hardware path or after doing software vertex processing.
 * \param indirect  if non-null, get the vertex count, first vertex, etc.
 *                  from a buffer.
 * \param so_vertex_count  if non-null, get the vertex count from a
 *                         stream-output target.
 */
enum pipe_error
svga_hwtnl_prim(struct svga_hwtnl *hwtnl,
                const SVGA3dPrimitiveRange *range,
                unsigned vcount,
                unsigned min_index, unsigned max_index,
                struct pipe_resource *ib,
                unsigned start_instance, unsigned instance_count,
                const struct pipe_draw_indirect_info *indirect,
                const struct pipe_stream_output_target *so_vertex_count)
{
   enum pipe_error ret = PIPE_OK;

   SVGA_STATS_TIME_PUSH(svga_sws(hwtnl->svga), SVGA_STATS_TIME_HWTNLPRIM);

   if (svga_have_vgpu10(hwtnl->svga)) {
      /* draw immediately */
      SVGA_RETRY(hwtnl->svga, draw_vgpu10(hwtnl, range, vcount, min_index,
                                          max_index, ib, start_instance,
                                          instance_count, indirect,
                                          so_vertex_count));
   }
   else {
      /* batch up drawing commands */
      assert(indirect == NULL);
#ifdef DEBUG
      check_draw_params(hwtnl, range, min_index, max_index, ib);
      assert(start_instance == 0);
      assert(instance_count <= 1);
#else
      (void) check_draw_params;
#endif

      if (hwtnl->cmd.prim_count + 1 >= QSZ) {
         ret = svga_hwtnl_flush(hwtnl);
         if (ret != PIPE_OK)
            goto done;
      }

      /* min/max indices are relative to bias */
      hwtnl->cmd.min_index[hwtnl->cmd.prim_count] = min_index;
      hwtnl->cmd.max_index[hwtnl->cmd.prim_count] = max_index;

      hwtnl->cmd.prim[hwtnl->cmd.prim_count] = *range;
      hwtnl->cmd.prim[hwtnl->cmd.prim_count].indexBias += hwtnl->index_bias;

      pipe_resource_reference(&hwtnl->cmd.prim_ib[hwtnl->cmd.prim_count], ib);
      hwtnl->cmd.prim_count++;
   }

done:
   SVGA_STATS_TIME_POP(svga_screen(hwtnl->svga->pipe.screen)->sws);
   return ret;
}


/**
 * Return TRUE if there are pending primitives.
 */
bool
svga_hwtnl_has_pending_prim(struct svga_hwtnl *hwtnl)
{
   return hwtnl->cmd.prim_count > 0;
}
