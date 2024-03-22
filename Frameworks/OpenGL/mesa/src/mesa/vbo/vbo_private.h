/*
 * mesa 3-D graphics library
 *
 * Copyright (C) 1999-2006  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * Types, functions, etc which are private to the VBO module.
 */


#ifndef VBO_PRIVATE_H
#define VBO_PRIVATE_H


#include "vbo/vbo_attrib.h"
#include "vbo/vbo_exec.h"
#include "vbo/vbo_save.h"
#include "main/varray.h"
#include "main/macros.h"
#include "state_tracker/st_atom.h"


struct _glapi_table;

static inline struct vbo_context *
vbo_context(struct gl_context *ctx)
{
   return &ctx->vbo_context;
}


static inline const struct vbo_context *
vbo_context_const(const struct gl_context *ctx)
{
   return &ctx->vbo_context;
}


static inline struct gl_context *
gl_context_from_vbo_exec(struct vbo_exec_context *exec)
{
   return container_of(exec, struct gl_context, vbo_context.exec);
}


static inline const struct gl_context *
gl_context_from_vbo_exec_const(const struct vbo_exec_context *exec)
{
   return container_of(exec, struct gl_context, vbo_context.exec);
}


static inline struct gl_context *
gl_context_from_vbo_save(struct vbo_save_context *save)
{
   return container_of(save, struct gl_context, vbo_context.save);
}


/**
 * Array to apply the fixed function material aliasing map to
 * an attribute value used in vbo processing inputs to an attribute
 * as they appear in the vao.
 */
extern const GLubyte
_vbo_attribute_alias_map[VP_MODE_MAX][VERT_ATTRIB_MAX];


/**
 * Return if format is integer. The immediate mode commands only emit floats
 * for non-integer types, thus everything else is integer.
 */
static inline GLboolean
vbo_attrtype_to_integer_flag(GLenum format)
{
   switch (format) {
   case GL_FLOAT:
   case GL_DOUBLE:
      return GL_FALSE;
   case GL_INT:
   case GL_UNSIGNED_INT:
   case GL_UNSIGNED_INT64_ARB:
      return GL_TRUE;
   default:
      unreachable("Bad vertex attribute type");
      return GL_FALSE;
   }
}

static inline GLboolean
vbo_attrtype_to_double_flag(GLenum format)
{
   switch (format) {
   case GL_FLOAT:
   case GL_INT:
   case GL_UNSIGNED_INT:
      return GL_FALSE;
   case GL_UNSIGNED_INT64_ARB:
   case GL_DOUBLE:
      return GL_TRUE;
   default:
      unreachable("Bad vertex attribute type");
      return GL_FALSE;
   }
}


static inline void
vbo_set_vertex_format(struct gl_vertex_format* vertex_format,
                      GLubyte size, GLenum16 type)
{
   _mesa_set_vertex_format(vertex_format, size, type, GL_RGBA, GL_FALSE,
                           vbo_attrtype_to_integer_flag(type),
                           vbo_attrtype_to_double_flag(type));
}


/**
 * Return default component values for the given format.
 * The return type is an array of fi_types, because that's how we declare
 * the vertex storage : floats , integers or unsigned integers.
 */
static inline const fi_type *
vbo_get_default_vals_as_union(GLenum format)
{
   static const GLfloat default_float[4] = { 0, 0, 0, 1 };
   static const GLint default_int[4] = { 0, 0, 0, 1 };
   static const GLdouble default_double[4] = { 0, 0, 0, 1 };
   static const uint64_t default_uint64[4] = { 0, 0, 0, 1 };

   switch (format) {
   case GL_FLOAT:
      return (fi_type *)default_float;
   case GL_INT:
   case GL_UNSIGNED_INT:
      return (fi_type *)default_int;
   case GL_DOUBLE:
      return (fi_type *)default_double;
   case GL_UNSIGNED_INT64_ARB:
      return (fi_type *)default_uint64;
   default:
      unreachable("Bad vertex format");
      return NULL;
   }
}


/**
 * Compute the max number of vertices which can be stored in
 * a vertex buffer, given the current vertex size, and the amount
 * of space already used.
 */
static inline unsigned
vbo_compute_max_verts(const struct vbo_exec_context *exec)
{
   unsigned n = (gl_context_from_vbo_exec_const(exec)->Const.glBeginEndBufferSize -
                 exec->vtx.buffer_used) /
                (exec->vtx.vertex_size * sizeof(GLfloat));
   if (n == 0)
      return 0;
   /* Subtract one so we're always sure to have room for an extra
    * vertex for GL_LINE_LOOP -> GL_LINE_STRIP conversion.
    */
   n--;
   return n;
}


void
vbo_try_prim_conversion(GLubyte *mode, unsigned *count);

bool
vbo_merge_draws(struct gl_context *ctx, bool in_dlist,
                GLubyte mode0, GLubyte mode1,
                unsigned start0, unsigned start1,
                unsigned *count0, unsigned count1,
                unsigned basevertex0, unsigned basevertex1,
                bool *end0, bool begin1, bool end1);

unsigned
vbo_copy_vertices(struct gl_context *ctx,
                  GLenum mode,
                  unsigned start, unsigned *count, bool begin,
                  unsigned vertex_size,
                  bool in_dlist,
                  fi_type *dst,
                  const fi_type *src);

/**
 * Get the filter mask for vbo draws depending on the vertex_processing_mode.
 */
static inline GLbitfield
_vbo_get_vao_filter(gl_vertex_processing_mode vertex_processing_mode)
{
   if (vertex_processing_mode == VP_MODE_FF) {
      /* The materials mapped into the generic arrays */
      return VERT_BIT_FF_ALL | VERT_BIT_MAT_ALL;
   } else {
      return VERT_BIT_ALL;
   }
}


/**
 * Translate the bitmask of VBO_ATTRIB_BITs to VERT_ATTRIB_BITS.
 * Note that position/generic0 attribute aliasing is done
 * generically in the VAO.
 */
static inline GLbitfield
_vbo_get_vao_enabled_from_vbo(gl_vertex_processing_mode vertex_processing_mode,
                              GLbitfield64 enabled)
{
   if (vertex_processing_mode == VP_MODE_FF) {
      /* The materials mapped into the generic arrays */
      return (((GLbitfield)enabled) & VERT_BIT_FF_ALL)
         | (((GLbitfield)(enabled >> VBO_MATERIAL_SHIFT)) & VERT_BIT_MAT_ALL);
   } else {
      return enabled;
   }
}


/**
 * Set the vertex attrib for vbo draw use.
 */
static inline void
_vbo_set_attrib_format(struct gl_context *ctx,
                       struct gl_vertex_array_object *vao,
                       gl_vert_attrib attr, GLintptr buffer_offset,
                       GLubyte size, GLenum16 type, GLuint offset)
{
   const GLboolean integer = vbo_attrtype_to_integer_flag(type);
   const GLboolean doubles = vbo_attrtype_to_double_flag(type);

   if (doubles)
      size /= 2;
   _mesa_update_array_format(ctx, vao, attr, size, type, GL_RGBA,
                             GL_FALSE, integer, doubles, offset);

   if (vao->Enabled & VERT_BIT(attr)) {
      ctx->NewDriverState |= ST_NEW_VERTEX_ARRAYS;
      ctx->Array.NewVertexElements = true;
   }

   vao->VertexAttrib[attr].Ptr = ADD_POINTERS(buffer_offset, offset);
}


#endif /* VBO_PRIVATE_H */
