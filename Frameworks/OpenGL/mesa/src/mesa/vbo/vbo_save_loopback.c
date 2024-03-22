/**************************************************************************
 *
 * Copyright 2005 VMware, Inc.
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

#include <stdio.h>
#include "main/context.h"
#include "util/glheader.h"
#include "main/enums.h"
#include "main/mesa_private.h"
#include "main/dispatch.h"
#include "glapi/glapi.h"

#include "vbo_private.h"


typedef void (*attr_func)(struct gl_context *ctx, GLint index, const GLfloat *);


/* This file makes heavy use of the aliasing of NV vertex attributes
 * with the legacy attributes, and also with ARB and Material
 * attributes as currently implemented.
 */
static void
VertexAttrib1fvNV(struct gl_context *ctx, GLint index, const GLfloat *v)
{
   CALL_VertexAttrib1fvNV(ctx->Dispatch.Exec, (index, v));
}


static void
VertexAttrib2fvNV(struct gl_context *ctx, GLint index, const GLfloat *v)
{
   CALL_VertexAttrib2fvNV(ctx->Dispatch.Exec, (index, v));
}


static void
VertexAttrib3fvNV(struct gl_context *ctx, GLint index, const GLfloat *v)
{
   CALL_VertexAttrib3fvNV(ctx->Dispatch.Exec, (index, v));
}


static void
VertexAttrib4fvNV(struct gl_context *ctx, GLint index, const GLfloat *v)
{
   CALL_VertexAttrib4fvNV(ctx->Dispatch.Exec, (index, v));
}


static attr_func vert_attrfunc[4] = {
   VertexAttrib1fvNV,
   VertexAttrib2fvNV,
   VertexAttrib3fvNV,
   VertexAttrib4fvNV
};


struct loopback_attr {
   enum vbo_attrib index;
   GLuint offset;
   attr_func func;
};


/**
 * Don't emit ends and begins on wrapped primitives.  Don't replay
 * wrapped vertices.  If we get here, it's probably because the
 * precalculated wrapping is wrong.
 */
static void
loopback_prim(struct gl_context *ctx,
              const GLubyte *buffer,
              const struct _mesa_prim *prim,
              GLuint wrap_count,
              GLuint stride,
              const struct loopback_attr *la, GLuint nr)
{
   GLuint start = prim->start;
   const GLuint end = start + prim->count;
   const GLubyte *data;

   if (0)
      printf("loopback prim %s(%s,%s) verts %d..%d  vsize %d\n",
             _mesa_lookup_prim_by_nr(prim->mode),
             prim->begin ? "begin" : "..",
             prim->end ? "end" : "..",
             start, end,
             stride);

   if (prim->begin) {
      CALL_Begin(ctx->Dispatch.Exec, (prim->mode));
   }
   else {
      start += wrap_count;
   }

   data = buffer + start * stride;

   for (GLuint j = start; j < end; j++) {
      for (GLuint k = 0; k < nr; k++)
         la[k].func(ctx, la[k].index, (const GLfloat *)(data + la[k].offset));

      data += stride;
   }

   if (prim->end) {
      CALL_End(ctx->Dispatch.Exec, ());
   }
}


static inline void
append_attr(GLuint *nr, struct loopback_attr la[], int i, int shift,
            const struct gl_vertex_array_object *vao)
{
   la[*nr].index = shift + i;
   la[*nr].offset = vao->VertexAttrib[i].RelativeOffset;
   la[*nr].func = vert_attrfunc[vao->VertexAttrib[i].Format.User.Size - 1];
   (*nr)++;
}


void
_vbo_loopback_vertex_list(struct gl_context *ctx,
                          const struct vbo_save_vertex_list* node,
                          fi_type *buffer)
{
   struct loopback_attr la[VBO_ATTRIB_MAX];
   GLuint nr = 0;

   /* All Legacy, NV, ARB and Material attributes are routed through
    * the NV attributes entrypoints:
    */
   const struct gl_vertex_array_object *vao = node->cold->VAO[VP_MODE_FF];
   GLbitfield mask = vao->Enabled & VERT_BIT_MAT_ALL;
   while (mask) {
      const int i = u_bit_scan(&mask);
      append_attr(&nr, la, i, VBO_MATERIAL_SHIFT, vao);
   }

   vao = node->cold->VAO[VP_MODE_SHADER];
   mask = vao->Enabled & ~(VERT_BIT_POS | VERT_BIT_GENERIC0);
   while (mask) {
      const int i = u_bit_scan(&mask);
      append_attr(&nr, la, i, 0, vao);
   }

   /* The last in the list should be the vertex provoking attribute */
   if (vao->Enabled & VERT_BIT_GENERIC0) {
      append_attr(&nr, la, VERT_ATTRIB_GENERIC0, 0, vao);
   } else if (vao->Enabled & VERT_BIT_POS) {
      append_attr(&nr, la, VERT_ATTRIB_POS, 0, vao);
   }

   const GLuint wrap_count = node->cold->wrap_count;
   const GLuint stride = _vbo_save_get_stride(node);

   /* Replay the primitives */
   const struct _mesa_prim *prims = node->cold->prims;
   const GLuint prim_count = node->cold->prim_count;

   for (GLuint i = 0; i < prim_count; i++) {
      loopback_prim(ctx, (GLubyte*)buffer + vao->BufferBinding[0].Offset,
                    &prims[i], wrap_count, stride, la, nr);
   }
}
