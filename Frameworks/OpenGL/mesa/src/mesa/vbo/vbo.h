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
 * \brief Public interface to the VBO module
 * \author Keith Whitwell
 */


#ifndef _VBO_H
#define _VBO_H

#include <stdbool.h>
#include "util/glheader.h"
#include "main/dd.h"
#include "main/draw.h"
#include "main/macros.h"
#include "vbo_attrib.h"
#include "gallium/include/pipe/p_state.h"

#ifdef __cplusplus
extern "C" {
#endif

struct gl_context;
struct pipe_draw_info;
struct pipe_draw_start_count_bias;

/**
 * Max number of primitives (number of glBegin/End pairs) per VBO.
 */
#define VBO_MAX_PRIM 64


/**
 * Current vertex processing mode: fixed function vs. shader.
 * In reality, fixed function is probably implemented by a shader but that's
 * not what we care about here.
 */
typedef enum
{
   VP_MODE_FF,     /**< legacy / fixed function */
   VP_MODE_SHADER, /**< ARB vertex program or GLSL vertex shader */
   VP_MODE_MAX     /**< for sizing arrays */
} gl_vertex_processing_mode;


struct vbo_exec_eval1_map {
   struct gl_1d_map *map;
   GLuint sz;
};

struct vbo_exec_eval2_map {
   struct gl_2d_map *map;
   GLuint sz;
};

struct vbo_exec_copied_vtx {
   fi_type buffer[VBO_ATTRIB_MAX * 4 * VBO_MAX_COPIED_VERTS];
   GLuint nr;
};

struct vbo_markers
{
   /**
    * If false and the primitive is a line loop, the first vertex is
    * the beginning of the line loop and it won't be drawn.
    * Instead, it will be moved to the end.
    *
    * Drivers shouldn't reset the line stipple pattern walker if begin is
    * false and mode is a line strip.
    */
   bool begin;

   /**
    * If true and the primitive is a line loop, it will be closed.
    */
   bool end;
};

struct vbo_exec_context
{
   struct {
      /* Multi draw where the mode can vary between draws. */
      struct pipe_draw_info info;
      struct pipe_draw_start_count_bias draw[VBO_MAX_PRIM];
      GLubyte mode[VBO_MAX_PRIM];            /**< primitive modes per draw */
      struct vbo_markers markers[VBO_MAX_PRIM];
      unsigned prim_count;

      struct gl_buffer_object *bufferobj;

      GLuint vertex_size;       /* in dwords */
      GLuint vertex_size_no_pos;

      fi_type *buffer_map;
      fi_type *buffer_ptr;              /* cursor, points into buffer */
      GLuint   buffer_used;             /* in bytes */
      unsigned buffer_offset;           /* only for persistent mappings */
      fi_type vertex[VBO_ATTRIB_MAX*4]; /* current vertex */

      GLuint vert_count;   /**< Number of vertices currently in buffer */
      GLuint max_vert;     /**< Max number of vertices allowed in buffer */
      struct vbo_exec_copied_vtx copied;

      GLbitfield64 enabled;             /**< mask of enabled vbo arrays. */

      /* Keep these packed in a structure for faster access. */
      struct {
         GLenum16 type;       /**< GL_FLOAT, GL_DOUBLE, GL_INT, etc */
         GLubyte active_size; /**< number of components, but can shrink */
         GLubyte size;        /**< number of components (1..4) */
      } attr[VBO_ATTRIB_MAX];

      /** pointers into the current 'vertex' array, declared above */
      fi_type *attrptr[VBO_ATTRIB_MAX];
   } vtx;

   struct {
      GLboolean recalculate_maps;
      struct vbo_exec_eval1_map map1[VERT_ATTRIB_MAX];
      struct vbo_exec_eval2_map map2[VERT_ATTRIB_MAX];
   } eval;

#ifndef NDEBUG
   GLint flush_call_depth;
#endif
};


struct vbo_save_context {
   GLbitfield64 enabled; /**< mask of enabled vbo arrays. */
   GLubyte attrsz[VBO_ATTRIB_MAX];  /**< 1, 2, 3 or 4 */
   GLenum16 attrtype[VBO_ATTRIB_MAX];  /**< GL_FLOAT, GL_INT, etc */
   GLubyte active_sz[VBO_ATTRIB_MAX];  /**< 1, 2, 3 or 4 */
   GLuint vertex_size;  /**< size in GLfloats */
   struct gl_vertex_array_object *VAO[VP_MODE_MAX];

   struct vbo_save_vertex_store *vertex_store;
   struct vbo_save_primitive_store *prim_store;
   struct gl_buffer_object *current_bo;
   unsigned current_bo_bytes_used;

   fi_type vertex[VBO_ATTRIB_MAX*4];	   /* current values */
   fi_type *attrptr[VBO_ATTRIB_MAX];

   struct {
      fi_type *buffer;
      GLuint nr;
   } copied;

   fi_type *current[VBO_ATTRIB_MAX]; /* points into ctx->ListState */
   GLubyte *currentsz[VBO_ATTRIB_MAX];

   GLboolean dangling_attr_ref;
   GLboolean out_of_memory;  /**< True if last VBO allocation failed */
   bool no_current_update;
};

GLboolean
_mesa_using_noop_vtxfmt(const struct _glapi_table *dispatch);

GLboolean
_vbo_CreateContext(struct gl_context *ctx);

void
_vbo_DestroyContext(struct gl_context *ctx);

void
vbo_init_dispatch_begin_end(struct gl_context *ctx);

void
vbo_init_dispatch_hw_select_begin_end(struct gl_context *ctx);

void
vbo_install_exec_vtxfmt_noop(struct gl_context *ctx);

void
vbo_install_save_vtxfmt_noop(struct gl_context *ctx);

void
vbo_exec_update_eval_maps(struct gl_context *ctx);

void
vbo_exec_FlushVertices(struct gl_context *ctx, GLuint flags);

void
vbo_save_SaveFlushVertices(struct gl_context *ctx);

void
vbo_save_NotifyBegin(struct gl_context *ctx, GLenum mode,
                     bool no_current_update);

void
vbo_save_NewList(struct gl_context *ctx, GLuint list, GLenum mode);

void
vbo_save_EndList(struct gl_context *ctx);

void
vbo_delete_minmax_cache(struct gl_buffer_object *bufferObj);

void
vbo_get_minmax_index_mapped(unsigned count, unsigned index_size,
                            unsigned restartIndex, bool restart,
                            const void *indices,
                            unsigned *min_index, unsigned *max_index);

void
vbo_get_minmax_index(struct gl_context *ctx, struct gl_buffer_object *obj,
                     const void *ptr, GLintptr offset, unsigned count,
                     unsigned index_size, bool primitive_restart,
                     unsigned restart_index, GLuint *min_index,
                     GLuint *max_index);

bool
vbo_get_minmax_indices_gallium(struct gl_context *ctx,
                               struct pipe_draw_info *info,
                               const struct pipe_draw_start_count_bias *draws,
                               unsigned num_draws);

const struct gl_array_attributes*
_vbo_current_attrib(const struct gl_context *ctx, gl_vert_attrib attr);

void GLAPIENTRY
_es_Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);

void GLAPIENTRY
_es_Normal3f(GLfloat x, GLfloat y, GLfloat z);

void GLAPIENTRY
_es_MultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);

void GLAPIENTRY
_es_Materialfv(GLenum face, GLenum pname, const GLfloat *params);

void GLAPIENTRY
_es_Materialf(GLenum face, GLenum pname, GLfloat param);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
