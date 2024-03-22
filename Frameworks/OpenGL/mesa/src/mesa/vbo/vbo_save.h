/**************************************************************************

Copyright 2002 VMware, Inc.

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
VMWARE AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Keith Whitwell <keithw@vmware.com>
 *
 */

#ifndef VBO_SAVE_H
#define VBO_SAVE_H

#include "mesa/main/dlist.h"
#include "vbo.h"
#include "vbo_attrib.h"

/* For display lists, this structure holds a run of vertices of the
 * same format, and a strictly well-formed set of begin/end pairs,
 * starting on the first vertex and ending at the last.  Vertex
 * copying on buffer breaks is precomputed according to these
 * primitives, though there are situations where the copying will need
 * correction at execute-time, perhaps by replaying the list as
 * immediate mode commands.
 *
 * On executing this list, the 'current' values may be updated with
 * the values of the final vertex, and often no fixup of the start of
 * the vertex list is required.
 *
 * Eval and other commands that don't fit into these vertex lists are
 * compiled using the fallback opcode mechanism provided by dlist.c.
 */
struct vbo_save_vertex_list {
   union gl_dlist_node header;

   /* Data used in vbo_save_playback_vertex_list */
   unsigned num_draws;
   uint8_t *modes;
   union {
      struct pipe_draw_start_count_bias *start_counts;
      struct pipe_draw_start_count_bias start_count;
   };
   uint8_t mode;
   bool draw_begins;

   int16_t private_refcount[VP_MODE_MAX];
   struct gl_context *ctx;
   struct pipe_vertex_state *state[VP_MODE_MAX];
   GLbitfield enabled_attribs[VP_MODE_MAX];

   /* Cold: used during construction or to handle edge-cases.
    * It's not part of the structure because we want display list nodes
    * to be tightly packed to get cache hits. Without this, performance would
    * decrease by an order of magnitude with 10k display lists.
    */
   struct {
      struct gl_vertex_array_object *VAO[VP_MODE_MAX];
      struct _mesa_index_buffer ib;

      struct pipe_draw_info info;

      /* Copy of the final vertex from node->vertex_store->bufferobj.
       * Keep this in regular (non-VBO) memory to avoid repeated
       * map/unmap of the VBO when updating GL current data.
       */
      fi_type *current_data;

      GLuint vertex_count;         /**< number of vertices in this list */
      GLuint wrap_count;		/* number of copied vertices at start */

      struct _mesa_prim *prims;
      GLuint prim_count;
      GLuint min_index, max_index;
      GLuint bo_bytes_used;
   } *cold;
};


/**
 * Return the stride in bytes of the display list node.
 */
static inline GLsizei
_vbo_save_get_stride(const struct vbo_save_vertex_list *node)
{
   return node->cold->VAO[0]->BufferBinding[0].Stride;
}

/* Default size for the buffer holding the vertices and the indices.
 * A bigger buffer helps reducing the number of draw calls but may
 * waste memory.
 * 1MB was picked because a lower value reduces viewperf snx tests
 * performance but larger values cause high VRAM usage (because
 * larger buffers will be shared by more display lists which reduces
 * the likelyhood of freeing the buffer).
 */
#define VBO_SAVE_BUFFER_SIZE (1024 * 1024)
#define VBO_SAVE_PRIM_MODE_MASK 0x3f

struct vbo_save_vertex_store {
   fi_type *buffer_in_ram;
   GLuint buffer_in_ram_size;
   GLuint used;           /**< Number of 4-byte words used in buffer */
};

struct vbo_save_primitive_store {
   struct _mesa_prim *prims;
   GLuint used;
   GLuint size;
};


void vbo_save_init(struct gl_context *ctx);
void vbo_save_destroy(struct gl_context *ctx);

/* save_loopback.c:
 */
void _vbo_loopback_vertex_list(struct gl_context *ctx,
                               const struct vbo_save_vertex_list* node,
                               fi_type *buffer);

/* Callbacks:
 */
void
vbo_save_playback_vertex_list(struct gl_context *ctx, void *data, bool copy_to_current);

void
vbo_save_playback_vertex_list_loopback(struct gl_context *ctx, void *data);

void
vbo_save_api_init(struct vbo_save_context *save);

#endif /* VBO_SAVE_H */
