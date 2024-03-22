/**********************************************************
 * Copyright 2009-2015 VMware, Inc.  All rights reserved.
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


#include "svga_cmd.h"
#include "util/u_debug.h"
#include "util/u_memory.h"

#include "vmw_context.h"
#include "vmw_shader.h"
#include "vmw_buffer.h"
#include "vmw_screen.h"

void
vmw_svga_winsys_shader_reference(struct vmw_svga_winsys_shader **pdst,
                                 struct vmw_svga_winsys_shader *src)
{
   struct pipe_reference *src_ref;
   struct pipe_reference *dst_ref;
   struct vmw_svga_winsys_shader *dst;

   if(pdst == NULL || *pdst == src)
      return;

   dst = *pdst;

   src_ref = src ? &src->refcnt : NULL;
   dst_ref = dst ? &dst->refcnt : NULL;

   if (pipe_reference(dst_ref, src_ref)) {
      struct svga_winsys_screen *sws = &dst->screen->base;

      if (!sws->have_vgpu10)
         vmw_ioctl_shader_destroy(dst->screen, dst->shid);
#ifdef DEBUG
      /* to detect dangling pointers */
      assert(p_atomic_read(&dst->validated) == 0);
      dst->shid = SVGA3D_INVALID_ID;
#endif
      sws->buffer_destroy(sws, dst->buf);
      FREE(dst);
   }

   *pdst = src;
}


/**
 * A helper function to create a shader object and upload the
 * shader bytecode and signature if specified to the shader memory.
 */
struct vmw_svga_winsys_shader *
vmw_svga_shader_create(struct svga_winsys_screen *sws,
                       SVGA3dShaderType type,
                       const uint32 *bytecode,
                       uint32 bytecodeLen,
                       const SVGA3dDXShaderSignatureHeader *sgnInfo,
                       uint32 sgnLen)
{
   struct vmw_svga_winsys_shader *shader;
   void *map;

   shader = CALLOC_STRUCT(vmw_svga_winsys_shader);
   if (!shader)
      return NULL;

   pipe_reference_init(&shader->refcnt, 1);
   p_atomic_set(&shader->validated, 0);
   shader->screen = vmw_winsys_screen(sws);
   shader->buf = sws->buffer_create(sws, 64,
                                    SVGA_BUFFER_USAGE_SHADER,
                                    bytecodeLen + sgnLen);
   if (!shader->buf) {
      FREE(shader);
      return NULL;
   }

   map = sws->buffer_map(sws, shader->buf, PIPE_MAP_WRITE);
   if (!map) {
      FREE(shader);
      return NULL;
   }

   /* copy the shader bytecode */
   memcpy(map, bytecode, bytecodeLen);

   /* if shader signature is specified, append it to the bytecode. */
   if (sgnLen) {
      assert(sws->have_sm5);
      map = (char *)map + bytecodeLen;
      memcpy(map, sgnInfo, sgnLen);
   }
   sws->buffer_unmap(sws, shader->buf);

   return shader;
}
