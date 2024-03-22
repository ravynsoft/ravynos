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

/**
 * @file
 * Shaders for VMware SVGA winsys.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 * @author Thomas Hellstrom <thellstrom@vmware.com>
 */

#ifndef VMW_SHADER_H_
#define VMW_SHADER_H_

#include "util/compiler.h"
#include "util/u_atomic.h"
#include "util/u_inlines.h"

struct vmw_svga_winsys_shader
{
   int32_t validated;
   struct pipe_reference refcnt;

   struct vmw_winsys_screen *screen;
   struct svga_winsys_buffer *buf;
   uint32_t shid;
};

static inline struct svga_winsys_gb_shader *
svga_winsys_shader(struct vmw_svga_winsys_shader *shader)
{
   assert(!shader || shader->shid != SVGA3D_INVALID_ID);
   return (struct svga_winsys_gb_shader *)shader;
}

static inline struct vmw_svga_winsys_shader *
vmw_svga_winsys_shader(struct svga_winsys_gb_shader *shader)
{
   return (struct vmw_svga_winsys_shader *)shader;
}

void
vmw_svga_winsys_shader_reference(struct vmw_svga_winsys_shader **pdst,
                                  struct vmw_svga_winsys_shader *src);

struct vmw_svga_winsys_shader *
vmw_svga_shader_create(struct svga_winsys_screen *sws,
                       SVGA3dShaderType type,
                       const uint32 *bytecode,
                       uint32 bytecodeLen,
                       const SVGA3dDXShaderSignatureHeader *sgnInfo,
                       uint32 sgnLen);

#endif /* VMW_SHADER_H_ */
