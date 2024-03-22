/**************************************************************************

Copyright 2002-2008 VMware, Inc.

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
 */



/* Display list compiler attempts to store lists of vertices with the
 * same vertex layout.  Additionally it attempts to minimize the need
 * for execute-time fixup of these vertex lists, allowing them to be
 * cached on hardware.
 *
 * There are still some circumstances where this can be thwarted, for
 * example by building a list that consists of one very long primitive
 * (eg Begin(Triangles), 1000 vertices, End), and calling that list
 * from inside a different begin/end object (Begin(Lines), CallList,
 * End).
 *
 * In that case the code will have to replay the list as individual
 * commands through the Exec dispatch table, or fix up the copied
 * vertices at execute-time.
 *
 * The other case where fixup is required is when a vertex attribute
 * is introduced in the middle of a primitive.  Eg:
 *  Begin(Lines)
 *  TexCoord1f()           Vertex2f()
 *  TexCoord1f() Color3f() Vertex2f()
 *  End()
 *
 *  If the current value of Color isn't known at compile-time, this
 *  primitive will require fixup.
 *
 *
 * The list compiler currently doesn't attempt to compile lists
 * containing EvalCoord or EvalPoint commands.  On encountering one of
 * these, compilation falls back to opcodes.
 *
 * This could be improved to fallback only when a mix of EvalCoord and
 * Vertex commands are issued within a single primitive.
 *
 * The compilation process works as follows. All vertex attributes
 * except position are copied to vbo_save_context::attrptr (see ATTR_UNION).
 * 'attrptr' are pointers to vbo_save_context::vertex ordered according to the enabled
 * attributes (se upgrade_vertex).
 * When the position attribute is received, all the attributes are then 
 * copied to the vertex_store (see the end of ATTR_UNION).
 * The vertex_store is simply an extensible float array.
 * When the vertex list needs to be compiled (see compile_vertex_list),
 * several transformations are performed:
 *   - some primitives are merged together (eg: two consecutive GL_TRIANGLES
 * with 3 vertices can be merged in a single GL_TRIANGLES with 6 vertices).
 *   - an index buffer is built.
 *   - identical vertices are detected and only one is kept.
 * At the end of this transformation, the index buffer and the vertex buffer
 * are uploaded in vRAM in the same buffer object.
 * This buffer object is shared between multiple display list to allow
 * draw calls merging later.
 *
 * The layout of this buffer for two display lists is:
 *    V0A0|V0A1|V1A0|V1A1|P0I0|P0I1|V0A0V0A1V0A2|V1A1V1A1V1A2|...
 *                                 ` new list starts
 *        - VxAy: vertex x, attributes y
 *        - PxIy: draw x, index y
 *
 * To allow draw call merging, display list must use the same VAO, including
 * the same Offset in the buffer object. To achieve this, the start values of
 * the primitive are shifted and the indices adjusted (see offset_diff and
 * start_offset in compile_vertex_list).
 *
 * Display list using the loopback code (see vbo_save_playback_vertex_list_loopback),
 * can't be drawn with an index buffer so this transformation is disabled
 * in this case.
 */


#include "util/glheader.h"
#include "main/arrayobj.h"
#include "main/bufferobj.h"
#include "main/context.h"
#include "main/dlist.h"
#include "main/enums.h"
#include "main/eval.h"
#include "main/macros.h"
#include "main/draw_validate.h"
#include "main/api_arrayelt.h"
#include "main/dispatch.h"
#include "main/state.h"
#include "main/varray.h"
#include "util/bitscan.h"
#include "util/u_memory.h"
#include "util/hash_table.h"
#include "gallium/auxiliary/indices/u_indices.h"
#include "util/u_prim.h"

#include "gallium/include/pipe/p_state.h"

#include "vbo_private.h"
#include "api_exec_decl.h"
#include "api_save.h"

#ifdef ERROR
#undef ERROR
#endif

/* An interesting VBO number/name to help with debugging */
#define VBO_BUF_ID  12345

static void GLAPIENTRY
_save_Materialfv(GLenum face, GLenum pname, const GLfloat *params);

static void GLAPIENTRY
_save_EvalCoord1f(GLfloat u);

static void GLAPIENTRY
_save_EvalCoord2f(GLfloat u, GLfloat v);

/*
 * NOTE: Old 'parity' issue is gone, but copying can still be
 * wrong-footed on replay.
 */
static GLuint
copy_vertices(struct gl_context *ctx,
              const struct vbo_save_vertex_list *node,
              const fi_type * src_buffer)
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;
   struct _mesa_prim *prim = &node->cold->prims[node->cold->prim_count - 1];
   GLuint sz = save->vertex_size;

   if (prim->end || !prim->count || !sz)
      return 0;

   const fi_type *src = src_buffer + prim->start * sz;
   assert(save->copied.buffer == NULL);
   save->copied.buffer = malloc(sizeof(fi_type) * sz * prim->count);

   unsigned r = vbo_copy_vertices(ctx, prim->mode, prim->start, &prim->count,
                                  prim->begin, sz, true, save->copied.buffer, src);
   if (!r) {
      free(save->copied.buffer);
      save->copied.buffer = NULL;
   }
   return r;
}


static struct vbo_save_primitive_store *
realloc_prim_store(struct vbo_save_primitive_store *store, int prim_count)
{
   if (store == NULL)
      store = CALLOC_STRUCT(vbo_save_primitive_store);

   uint32_t old_size = store->size;
   store->size = prim_count;
   assert (old_size < store->size);
   store->prims = realloc(store->prims, store->size * sizeof(struct _mesa_prim));
   memset(&store->prims[old_size], 0, (store->size - old_size) * sizeof(struct _mesa_prim));

   return store;
}


static void
reset_counters(struct gl_context *ctx)
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;

   save->vertex_store->used = 0;
   save->prim_store->used = 0;
   save->dangling_attr_ref = GL_FALSE;
}

/**
 * For a list of prims, try merging prims that can just be extensions of the
 * previous prim.
 */
static void
merge_prims(struct gl_context *ctx, struct _mesa_prim *prim_list,
            GLuint *prim_count)
{
   GLuint i;
   struct _mesa_prim *prev_prim = prim_list;

   for (i = 1; i < *prim_count; i++) {
      struct _mesa_prim *this_prim = prim_list + i;

      vbo_try_prim_conversion(&this_prim->mode, &this_prim->count);

      if (vbo_merge_draws(ctx, true,
                          prev_prim->mode, this_prim->mode,
                          prev_prim->start, this_prim->start,
                          &prev_prim->count, this_prim->count,
                          prev_prim->basevertex, this_prim->basevertex,
                          &prev_prim->end,
                          this_prim->begin, this_prim->end)) {
         /* We've found a prim that just extend the previous one.  Tack it
          * onto the previous one, and let this primitive struct get dropped.
          */
         continue;
      }

      /* If any previous primitives have been dropped, then we need to copy
       * this later one into the next available slot.
       */
      prev_prim++;
      if (prev_prim != this_prim)
         *prev_prim = *this_prim;
   }

   *prim_count = prev_prim - prim_list + 1;
}


/**
 * Convert GL_LINE_LOOP primitive into GL_LINE_STRIP so that drivers
 * don't have to worry about handling the _mesa_prim::begin/end flags.
 * See https://bugs.freedesktop.org/show_bug.cgi?id=81174
 */
static void
convert_line_loop_to_strip(struct vbo_save_context *save,
                           struct vbo_save_vertex_list *node)
{
   struct _mesa_prim *prim = &node->cold->prims[node->cold->prim_count - 1];

   assert(prim->mode == GL_LINE_LOOP);

   if (prim->end) {
      /* Copy the 0th vertex to end of the buffer and extend the
       * vertex count by one to finish the line loop.
       */
      const GLuint sz = save->vertex_size;
      /* 0th vertex: */
      const fi_type *src = save->vertex_store->buffer_in_ram + prim->start * sz;
      /* end of buffer: */
      fi_type *dst = save->vertex_store->buffer_in_ram + (prim->start + prim->count) * sz;

      memcpy(dst, src, sz * sizeof(float));

      prim->count++;
      node->cold->vertex_count++;
      save->vertex_store->used += sz;
   }

   if (!prim->begin) {
      /* Drawing the second or later section of a long line loop.
       * Skip the 0th vertex.
       */
      prim->start++;
      prim->count--;
   }

   prim->mode = GL_LINE_STRIP;
}


/* Compare the present vao if it has the same setup. */
static bool
compare_vao(gl_vertex_processing_mode mode,
            const struct gl_vertex_array_object *vao,
            const struct gl_buffer_object *bo, GLintptr buffer_offset,
            GLuint stride, GLbitfield64 vao_enabled,
            const GLubyte size[VBO_ATTRIB_MAX],
            const GLenum16 type[VBO_ATTRIB_MAX],
            const GLuint offset[VBO_ATTRIB_MAX])
{
   if (!vao)
      return false;

   /* If the enabled arrays are not the same we are not equal. */
   if (vao_enabled != vao->Enabled)
      return false;

   /* Check the buffer binding at 0 */
   if (vao->BufferBinding[0].BufferObj != bo)
      return false;
   /* BufferBinding[0].Offset != buffer_offset is checked per attribute */
   if (vao->BufferBinding[0].Stride != stride)
      return false;
   assert(vao->BufferBinding[0].InstanceDivisor == 0);

   /* Retrieve the mapping from VBO_ATTRIB to VERT_ATTRIB space */
   const GLubyte *const vao_to_vbo_map = _vbo_attribute_alias_map[mode];

   /* Now check the enabled arrays */
   GLbitfield mask = vao_enabled;
   while (mask) {
      const int attr = u_bit_scan(&mask);
      const unsigned char vbo_attr = vao_to_vbo_map[attr];
      const GLenum16 tp = type[vbo_attr];
      const GLintptr off = offset[vbo_attr] + buffer_offset;
      const struct gl_array_attributes *attrib = &vao->VertexAttrib[attr];
      if (attrib->RelativeOffset + vao->BufferBinding[0].Offset != off)
         return false;
      if (attrib->Format.User.Type != tp)
         return false;
      if (attrib->Format.User.Size != size[vbo_attr])
         return false;
      assert(!attrib->Format.User.Bgra);
      assert(attrib->Format.User.Normalized == GL_FALSE);
      assert(attrib->Format.User.Integer == vbo_attrtype_to_integer_flag(tp));
      assert(attrib->Format.User.Doubles == vbo_attrtype_to_double_flag(tp));
      assert(attrib->BufferBindingIndex == 0);
   }

   return true;
}


/* Create or reuse the vao for the vertex processing mode. */
static void
update_vao(struct gl_context *ctx,
           gl_vertex_processing_mode mode,
           struct gl_vertex_array_object **vao,
           struct gl_buffer_object *bo, GLintptr buffer_offset,
           GLuint stride, GLbitfield64 vbo_enabled,
           const GLubyte size[VBO_ATTRIB_MAX],
           const GLenum16 type[VBO_ATTRIB_MAX],
           const GLuint offset[VBO_ATTRIB_MAX])
{
   /* Compute the bitmasks of vao_enabled arrays */
   GLbitfield vao_enabled = _vbo_get_vao_enabled_from_vbo(mode, vbo_enabled);

   /*
    * Check if we can possibly reuse the exisiting one.
    * In the long term we should reset them when something changes.
    */
   if (compare_vao(mode, *vao, bo, buffer_offset, stride,
                   vao_enabled, size, type, offset))
      return;

   /* The initial refcount is 1 */
   _mesa_reference_vao(ctx, vao, NULL);
   *vao = _mesa_new_vao(ctx, ~((GLuint)0));

   /*
    * assert(stride <= ctx->Const.MaxVertexAttribStride);
    * MaxVertexAttribStride is not set for drivers that does not
    * expose GL 44 or GLES 31.
    */

   /* Bind the buffer object at binding point 0 */
   _mesa_bind_vertex_buffer(ctx, *vao, 0, bo, buffer_offset, stride, false,
                            false);

   /* Retrieve the mapping from VBO_ATTRIB to VERT_ATTRIB space
    * Note that the position/generic0 aliasing is done in the VAO.
    */
   const GLubyte *const vao_to_vbo_map = _vbo_attribute_alias_map[mode];
   /* Now set the enable arrays */
   GLbitfield mask = vao_enabled;
   while (mask) {
      const int vao_attr = u_bit_scan(&mask);
      const GLubyte vbo_attr = vao_to_vbo_map[vao_attr];
      assert(offset[vbo_attr] <= ctx->Const.MaxVertexAttribRelativeOffset);

      _vbo_set_attrib_format(ctx, *vao, vao_attr, buffer_offset,
                             size[vbo_attr], type[vbo_attr], offset[vbo_attr]);
      _mesa_vertex_attrib_binding(ctx, *vao, vao_attr, 0);
   }
   _mesa_enable_vertex_array_attribs(ctx, *vao, vao_enabled);
   assert(vao_enabled == (*vao)->Enabled);
   assert((vao_enabled & ~(*vao)->VertexAttribBufferMask) == 0);

   /* Finalize and freeze the VAO */
   _mesa_set_vao_immutable(ctx, *vao);
}

static void wrap_filled_vertex(struct gl_context *ctx);

/* Grow the vertex storage to accomodate for vertex_count new vertices */
static void
grow_vertex_storage(struct gl_context *ctx, int vertex_count)
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;
   assert (save->vertex_store);

   int new_size = (save->vertex_store->used +
                   vertex_count * save->vertex_size) * sizeof(GLfloat);

   /* Limit how much memory we allocate. */
   if (save->prim_store->used > 0 &&
       vertex_count > 0 &&
       new_size > VBO_SAVE_BUFFER_SIZE) {
      wrap_filled_vertex(ctx);
      new_size = VBO_SAVE_BUFFER_SIZE;
   }

   if (new_size > save->vertex_store->buffer_in_ram_size) {
      save->vertex_store->buffer_in_ram_size = new_size;
      save->vertex_store->buffer_in_ram = realloc(save->vertex_store->buffer_in_ram,
                                                  save->vertex_store->buffer_in_ram_size);
      if (save->vertex_store->buffer_in_ram == NULL)
         save->out_of_memory = true;
   }
}

struct vertex_key {
   unsigned vertex_size;
   fi_type *vertex_attributes;
};

static uint32_t _hash_vertex_key(const void *key)
{
   struct vertex_key *k = (struct vertex_key*)key;
   unsigned sz = k->vertex_size;
   assert(sz);
   return _mesa_hash_data(k->vertex_attributes, sz * sizeof(float));
}

static bool _compare_vertex_key(const void *key1, const void *key2)
{
   struct vertex_key *k1 = (struct vertex_key*)key1;
   struct vertex_key *k2 = (struct vertex_key*)key2;
   /* All the compared vertices are going to be drawn with the same VAO,
    * so we can compare the attributes. */
   assert (k1->vertex_size == k2->vertex_size);
   return memcmp(k1->vertex_attributes,
                 k2->vertex_attributes,
                 k1->vertex_size * sizeof(float)) == 0;
}

static void _free_entry(struct hash_entry *entry)
{
   free((void*)entry->key);
}

/* Add vertex to the vertex buffer and return its index. If this vertex is a duplicate
 * of an existing vertex, return the original index instead.
 */
static uint32_t
add_vertex(struct vbo_save_context *save, struct hash_table *hash_to_index,
           uint32_t index, fi_type *new_buffer, uint32_t *max_index)
{
   /* If vertex deduplication is disabled return the original index. */
   if (!hash_to_index)
      return index;

   fi_type *vert = save->vertex_store->buffer_in_ram + save->vertex_size * index;

   struct vertex_key *key = malloc(sizeof(struct vertex_key));
   key->vertex_size = save->vertex_size;
   key->vertex_attributes = vert;

   struct hash_entry *entry = _mesa_hash_table_search(hash_to_index, key);
   if (entry) {
      free(key);
      /* We found an existing vertex with the same hash, return its index. */
      return (uintptr_t) entry->data;
   } else {
      /* This is a new vertex. Determine a new index and copy its attributes to the vertex
       * buffer. Note that 'new_buffer' is created at each list compilation so we write vertices
       * starting at index 0.
       */
      uint32_t n = _mesa_hash_table_num_entries(hash_to_index);
      *max_index = MAX2(n, *max_index);

      memcpy(&new_buffer[save->vertex_size * n],
             vert,
             save->vertex_size * sizeof(fi_type));

      _mesa_hash_table_insert(hash_to_index, key, (void*)(uintptr_t)(n));

      /* The index buffer is shared between list compilations, so add the base index to get
       * the final index.
       */
      return n;
   }
}


static uint32_t
get_vertex_count(struct vbo_save_context *save)
{
   if (!save->vertex_size)
      return 0;
   return save->vertex_store->used / save->vertex_size;
}


/**
 * Insert the active immediate struct onto the display list currently
 * being built.
 */
static void
compile_vertex_list(struct gl_context *ctx)
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;
   struct vbo_save_vertex_list *node;

   /* Allocate space for this structure in the display list currently
    * being compiled.
    */
   node = (struct vbo_save_vertex_list *)
      _mesa_dlist_alloc_vertex_list(ctx, !save->dangling_attr_ref && !save->no_current_update);

   if (!node)
      return;

   node->cold = calloc(1, sizeof(*node->cold));

   /* Make sure the pointer is aligned to the size of a pointer */
   assert((GLintptr) node % sizeof(void *) == 0);

   const GLsizei stride = save->vertex_size*sizeof(GLfloat);

   node->cold->vertex_count = get_vertex_count(save);
   node->cold->wrap_count = save->copied.nr;
   node->cold->prims = malloc(sizeof(struct _mesa_prim) * save->prim_store->used);
   memcpy(node->cold->prims, save->prim_store->prims, sizeof(struct _mesa_prim) * save->prim_store->used);
   node->cold->ib.obj = NULL;
   node->cold->prim_count = save->prim_store->used;

   if (save->no_current_update) {
      node->cold->current_data = NULL;
   }
   else {
      GLuint current_size = save->vertex_size - save->attrsz[0];
      node->cold->current_data = NULL;

      if (current_size) {
         node->cold->current_data = malloc(current_size * sizeof(GLfloat));
         if (node->cold->current_data) {
            const char *buffer = (const char *)save->vertex_store->buffer_in_ram;
            unsigned attr_offset = save->attrsz[0] * sizeof(GLfloat);
            unsigned vertex_offset = 0;

            if (node->cold->vertex_count)
               vertex_offset = (node->cold->vertex_count - 1) * stride;

            memcpy(node->cold->current_data, buffer + vertex_offset + attr_offset,
                   current_size * sizeof(GLfloat));
         } else {
            _mesa_error(ctx, GL_OUT_OF_MEMORY, "Current value allocation");
            save->out_of_memory = true;
         }
      }
   }

   assert(save->attrsz[VBO_ATTRIB_POS] != 0 || node->cold->vertex_count == 0);

   if (save->dangling_attr_ref)
      ctx->ListState.Current.UseLoopback = true;

   /* Copy duplicated vertices
    */
   save->copied.nr = copy_vertices(ctx, node, save->vertex_store->buffer_in_ram);

   if (node->cold->prims[node->cold->prim_count - 1].mode == GL_LINE_LOOP) {
      convert_line_loop_to_strip(save, node);
   }

   merge_prims(ctx, node->cold->prims, &node->cold->prim_count);

   GLintptr buffer_offset = 0;
   GLuint start_offset = 0;

   /* Create an index buffer. */
   node->cold->min_index = node->cold->max_index = 0;
   if (node->cold->vertex_count == 0 || node->cold->prim_count == 0)
      goto end;

   /* We won't modify node->prims, so use a const alias to avoid unintended
    * writes to it. */
   const struct _mesa_prim *original_prims = node->cold->prims;

   int end = original_prims[node->cold->prim_count - 1].start +
             original_prims[node->cold->prim_count - 1].count;
   int total_vert_count = end - original_prims[0].start;

   node->cold->min_index = node->cold->prims[0].start;
   node->cold->max_index = end - 1;

   /* converting primitive types may result in many more indices */
   bool all_prims_supported = (ctx->Const.DriverSupportedPrimMask & BITFIELD_MASK(MESA_PRIM_COUNT)) == BITFIELD_MASK(MESA_PRIM_COUNT);
   int max_index_count = total_vert_count * (all_prims_supported ? 2 : 3);
   uint32_t* indices = (uint32_t*) malloc(max_index_count * sizeof(uint32_t));
   void *tmp_indices = all_prims_supported ? NULL : malloc(max_index_count * sizeof(uint32_t));
   struct _mesa_prim *merged_prims = NULL;

   int idx = 0;
   struct hash_table *vertex_to_index = NULL;
   fi_type *temp_vertices_buffer = NULL;

   /* The loopback replay code doesn't use the index buffer, so we can't
    * dedup vertices in this case.
    */
   if (!ctx->ListState.Current.UseLoopback) {
      vertex_to_index = _mesa_hash_table_create(NULL, _hash_vertex_key, _compare_vertex_key);
      temp_vertices_buffer = malloc(save->vertex_store->buffer_in_ram_size);
   }

   uint32_t max_index = 0;

   int last_valid_prim = -1;
   /* Construct indices array. */
   for (unsigned i = 0; i < node->cold->prim_count; i++) {
      assert(original_prims[i].basevertex == 0);
      GLubyte mode = original_prims[i].mode;
      bool converted_prim = false;
      unsigned index_size;
      bool outputting_quads = !!(ctx->Const.DriverSupportedPrimMask &
                                 (BITFIELD_MASK(MESA_PRIM_QUADS) | BITFIELD_MASK(MESA_PRIM_QUAD_STRIP)));
      unsigned verts_per_primitive = outputting_quads ? 4 : 3;

      int vertex_count = original_prims[i].count;
      if (!vertex_count) {
         continue;
      }

      /* Increase indices storage if the original estimation was too small. */
      if (idx + verts_per_primitive * vertex_count > max_index_count) {
         max_index_count = max_index_count + verts_per_primitive * vertex_count;
         indices = (uint32_t*) realloc(indices, max_index_count * sizeof(uint32_t));
         tmp_indices = all_prims_supported ? NULL : realloc(tmp_indices, max_index_count * sizeof(uint32_t));
      }

      /* Line strips may get converted to lines */
      if (mode == GL_LINE_STRIP)
         mode = GL_LINES;

      if (!(ctx->Const.DriverSupportedPrimMask & BITFIELD_BIT(mode))) {
         unsigned new_count;
         u_generate_func trans_func;
         enum mesa_prim pmode = (enum mesa_prim)mode;
         u_index_generator(ctx->Const.DriverSupportedPrimMask,
                           pmode, original_prims[i].start, vertex_count,
                           PV_LAST, PV_LAST,
                           &pmode, &index_size, &new_count,
                           &trans_func);
         if (new_count > 0)
            trans_func(original_prims[i].start, new_count, tmp_indices);
         vertex_count = new_count;
         mode = (GLubyte)pmode;
         converted_prim = true;
      }

      /* If 2 consecutive prims use the same mode => merge them. */
      bool merge_prims = last_valid_prim >= 0 &&
                         mode == merged_prims[last_valid_prim].mode &&
                         mode != GL_LINE_LOOP && mode != GL_TRIANGLE_FAN &&
                         mode != GL_QUAD_STRIP && mode != GL_POLYGON &&
                         mode != GL_PATCHES;

/* index generation uses uint16_t if the index count is small enough */
#define CAST_INDEX(BASE, SIZE, IDX) ((SIZE == 2 ? (uint32_t)(((uint16_t*)BASE)[IDX]) : ((uint32_t*)BASE)[IDX]))
      /* To be able to merge consecutive triangle strips we need to insert
       * a degenerate triangle.
       */
      if (merge_prims &&
          mode == GL_TRIANGLE_STRIP) {
         /* Insert a degenerate triangle */
         assert(merged_prims[last_valid_prim].mode == GL_TRIANGLE_STRIP);
         unsigned tri_count = merged_prims[last_valid_prim].count - 2;

         indices[idx] = indices[idx - 1];
         indices[idx + 1] = add_vertex(save, vertex_to_index,
                                       converted_prim ? CAST_INDEX(tmp_indices, index_size, 0) : original_prims[i].start,
                                       temp_vertices_buffer, &max_index);
         idx += 2;
         merged_prims[last_valid_prim].count += 2;

         if (tri_count % 2) {
            /* Add another index to preserve winding order */
            indices[idx++] = add_vertex(save, vertex_to_index,
                                        converted_prim ? CAST_INDEX(tmp_indices, index_size, 0) : original_prims[i].start,
                                        temp_vertices_buffer, &max_index);
            merged_prims[last_valid_prim].count++;
         }
      }

      int start = idx;

      /* Convert line strips to lines if it'll allow if the previous
       * prim mode is GL_LINES (so merge_prims is true) or if the next
       * primitive mode is GL_LINES or GL_LINE_LOOP.
       */
      if (original_prims[i].mode == GL_LINE_STRIP &&
          (merge_prims ||
           (i < node->cold->prim_count - 1 &&
            (original_prims[i + 1].mode == GL_LINE_STRIP ||
             original_prims[i + 1].mode == GL_LINES)))) {
         for (unsigned j = 0; j < vertex_count; j++) {
            indices[idx++] = add_vertex(save, vertex_to_index,
                                        converted_prim ? CAST_INDEX(tmp_indices, index_size, j) : original_prims[i].start + j,
                                        temp_vertices_buffer, &max_index);
            /* Repeat all but the first/last indices. */
            if (j && j != vertex_count - 1) {
               indices[idx++] = add_vertex(save, vertex_to_index,
                                           converted_prim ? CAST_INDEX(tmp_indices, index_size, j) : original_prims[i].start + j,
                                           temp_vertices_buffer, &max_index);
            }
         }
      } else {
         /* We didn't convert to LINES, so restore the original mode */
         if (!converted_prim)
            mode = original_prims[i].mode;

         for (unsigned j = 0; j < vertex_count; j++) {
            indices[idx++] = add_vertex(save, vertex_to_index,
                                        converted_prim ? CAST_INDEX(tmp_indices, index_size, j) : original_prims[i].start + j,
                                        temp_vertices_buffer, &max_index);
         }
      }

      /* Duplicate the last vertex for incomplete primitives */
      if (vertex_count > 0) {
         unsigned min_vert = u_prim_vertex_count(mode)->min;
         for (unsigned j = vertex_count; j < min_vert; j++) {
            indices[idx++] = add_vertex(save, vertex_to_index,
                                       converted_prim ? CAST_INDEX(tmp_indices, index_size, vertex_count - 1) :
                                                         original_prims[i].start + vertex_count - 1,
                                       temp_vertices_buffer, &max_index);
         }
      }

#undef CAST_INDEX
      if (merge_prims) {
         /* Update vertex count. */
         merged_prims[last_valid_prim].count += idx - start;
      } else {
         /* Keep this primitive */
         last_valid_prim += 1;
         assert(last_valid_prim <= i);
         merged_prims = realloc(merged_prims, (1 + last_valid_prim) * sizeof(struct _mesa_prim));
         merged_prims[last_valid_prim] = original_prims[i];
         merged_prims[last_valid_prim].start = start;
         merged_prims[last_valid_prim].count = idx - start;
      }
      merged_prims[last_valid_prim].mode = mode;

      /* converted prims will filter incomplete primitives and may have no indices */
      assert((idx > 0 || converted_prim) && idx <= max_index_count);
   }

   unsigned merged_prim_count = last_valid_prim + 1;
   node->cold->ib.ptr = NULL;
   node->cold->ib.count = idx;
   node->cold->ib.index_size_shift = (GL_UNSIGNED_INT - GL_UNSIGNED_BYTE) >> 1;

   /* How many bytes do we need to store the indices and the vertices */
   total_vert_count = vertex_to_index ? (max_index + 1) : idx;
   unsigned total_bytes_needed = idx * sizeof(uint32_t) +
                                 total_vert_count * save->vertex_size * sizeof(fi_type);

   const GLintptr old_offset = save->VAO[0] ?
      save->VAO[0]->BufferBinding[0].Offset + save->VAO[0]->VertexAttrib[VERT_ATTRIB_POS].RelativeOffset : 0;
   if (old_offset != save->current_bo_bytes_used && stride > 0) {
      GLintptr offset_diff = save->current_bo_bytes_used - old_offset;
      while (offset_diff > 0 &&
             save->current_bo_bytes_used < save->current_bo->Size &&
             offset_diff % stride != 0) {
         save->current_bo_bytes_used++;
         offset_diff = save->current_bo_bytes_used - old_offset;
      }
   }
   buffer_offset = save->current_bo_bytes_used;

   /* Can we reuse the previous bo or should we allocate a new one? */
   int available_bytes = save->current_bo ? save->current_bo->Size - save->current_bo_bytes_used : 0;
   if (total_bytes_needed > available_bytes) {
      if (save->current_bo)
         _mesa_reference_buffer_object(ctx, &save->current_bo, NULL);
      save->current_bo = _mesa_bufferobj_alloc(ctx, VBO_BUF_ID + 1);
      bool success = _mesa_bufferobj_data(ctx,
                                          GL_ELEMENT_ARRAY_BUFFER_ARB,
                                          MAX2(total_bytes_needed, VBO_SAVE_BUFFER_SIZE),
                                          NULL,
                                          GL_STATIC_DRAW_ARB, GL_MAP_WRITE_BIT |
                                          MESA_GALLIUM_VERTEX_STATE_STORAGE,
                                          save->current_bo);
      if (!success) {
         _mesa_reference_buffer_object(ctx, &save->current_bo, NULL);
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "IB allocation");
         save->out_of_memory = true;
      } else {
         save->current_bo_bytes_used = 0;
         available_bytes = save->current_bo->Size;
      }
      buffer_offset = 0;
   } else {
      assert(old_offset <= buffer_offset);
      const GLintptr offset_diff = buffer_offset - old_offset;
      if (offset_diff > 0 && stride > 0 && offset_diff % stride == 0) {
         /* The vertex size is an exact multiple of the buffer offset.
          * This means that we can use zero-based vertex attribute pointers
          * and specify the start of the primitive with the _mesa_prim::start
          * field.  This results in issuing several draw calls with identical
          * vertex attribute information.  This can result in fewer state
          * changes in drivers.  In particular, the Gallium CSO module will
          * filter out redundant vertex buffer changes.
          */
         /* We cannot immediately update the primitives as some methods below
          * still need the uncorrected start vertices
          */
         start_offset = offset_diff/stride;
         assert(old_offset == buffer_offset - offset_diff);
         buffer_offset = old_offset;
      }

      /* Correct the primitive starts, we can only do this here as copy_vertices
       * and convert_line_loop_to_strip above consume the uncorrected starts.
       * On the other hand the _vbo_loopback_vertex_list call below needs the
       * primitives to be corrected already.
       */
      for (unsigned i = 0; i < node->cold->prim_count; i++) {
         node->cold->prims[i].start += start_offset;
      }
      /* start_offset shifts vertices (so v[0] becomes v[start_offset]), so we have
       * to apply this transformation to all indices and max_index.
       */
      for (unsigned i = 0; i < idx; i++)
         indices[i] += start_offset;
      max_index += start_offset;
   }

   _mesa_reference_buffer_object(ctx, &node->cold->ib.obj, save->current_bo);

   /* Upload the vertices first (see buffer_offset) */
   _mesa_bufferobj_subdata(ctx,
                           save->current_bo_bytes_used,
                           total_vert_count * save->vertex_size * sizeof(fi_type),
                           vertex_to_index ? temp_vertices_buffer : save->vertex_store->buffer_in_ram,
                           node->cold->ib.obj);
   save->current_bo_bytes_used += total_vert_count * save->vertex_size * sizeof(fi_type);
   node->cold->bo_bytes_used = save->current_bo_bytes_used;

  if (vertex_to_index) {
      _mesa_hash_table_destroy(vertex_to_index, _free_entry);
      free(temp_vertices_buffer);
   }

   /* Since we append the indices to an existing buffer, we need to adjust the start value of each
    * primitive (not the indices themselves). */
   if (!ctx->ListState.Current.UseLoopback) {
      save->current_bo_bytes_used += align(save->current_bo_bytes_used, 4) - save->current_bo_bytes_used;
      int indices_offset = save->current_bo_bytes_used / 4;
      for (int i = 0; i < merged_prim_count; i++) {
         merged_prims[i].start += indices_offset;
      }
   }

   /* Then upload the indices. */
   if (node->cold->ib.obj) {
      _mesa_bufferobj_subdata(ctx,
                              save->current_bo_bytes_used,
                              idx * sizeof(uint32_t),
                              indices,
                              node->cold->ib.obj);
      save->current_bo_bytes_used += idx * sizeof(uint32_t);
   } else {
      node->cold->vertex_count = 0;
      node->cold->prim_count = 0;
   }

   /* Prepare for DrawGallium */
   memset(&node->cold->info, 0, sizeof(struct pipe_draw_info));
   /* The other info fields will be updated in vbo_save_playback_vertex_list */
   node->cold->info.index_size = 4;
   node->cold->info.instance_count = 1;
   node->cold->info.index.resource = node->cold->ib.obj->buffer;
   if (merged_prim_count == 1) {
      node->cold->info.mode = merged_prims[0].mode;
      node->start_count.start = merged_prims[0].start;
      node->start_count.count = merged_prims[0].count;
      node->start_count.index_bias = 0;
      node->modes = NULL;
   } else {
      node->modes = malloc(merged_prim_count * sizeof(unsigned char));
      node->start_counts = malloc(merged_prim_count * sizeof(struct pipe_draw_start_count_bias));
      for (unsigned i = 0; i < merged_prim_count; i++) {
         node->start_counts[i].start = merged_prims[i].start;
         node->start_counts[i].count = merged_prims[i].count;
         node->start_counts[i].index_bias = 0;
         node->modes[i] = merged_prims[i].mode;
      }
   }
   node->num_draws = merged_prim_count;
   if (node->num_draws > 1) {
      bool same_mode = true;
      for (unsigned i = 1; i < node->num_draws && same_mode; i++) {
         same_mode = node->modes[i] == node->modes[0];
      }
      if (same_mode) {
         /* All primitives use the same mode, so we can simplify a bit */
         node->cold->info.mode = node->modes[0];
         free(node->modes);
         node->modes = NULL;
      }
   }

   free(indices);
   free(tmp_indices);
   free(merged_prims);

end:
   node->draw_begins = node->cold->prims[0].begin;

   if (!save->current_bo) {
      save->current_bo = _mesa_bufferobj_alloc(ctx, VBO_BUF_ID + 1);
      bool success = _mesa_bufferobj_data(ctx,
                                          GL_ELEMENT_ARRAY_BUFFER_ARB,
                                          VBO_SAVE_BUFFER_SIZE,
                                          NULL,
                                          GL_STATIC_DRAW_ARB, GL_MAP_WRITE_BIT |
                                          MESA_GALLIUM_VERTEX_STATE_STORAGE,
                                          save->current_bo);
      if (!success)
         save->out_of_memory = true;
   }

   GLuint offsets[VBO_ATTRIB_MAX];
   for (unsigned i = 0, offset = 0; i < VBO_ATTRIB_MAX; ++i) {
      offsets[i] = offset;
      offset += save->attrsz[i] * sizeof(GLfloat);
   }
   /* Create a pair of VAOs for the possible VERTEX_PROCESSING_MODEs
    * Note that this may reuse the previous one of possible.
    */
   for (gl_vertex_processing_mode vpm = VP_MODE_FF; vpm < VP_MODE_MAX; ++vpm) {
      /* create or reuse the vao */
      update_vao(ctx, vpm, &save->VAO[vpm],
                 save->current_bo, buffer_offset, stride,
                 save->enabled, save->attrsz, save->attrtype, offsets);
      /* Reference the vao in the dlist */
      node->cold->VAO[vpm] = NULL;
      _mesa_reference_vao(ctx, &node->cold->VAO[vpm], save->VAO[vpm]);
   }

   /* Prepare for DrawGalliumVertexState */
   if (node->num_draws && ctx->Driver.DrawGalliumVertexState) {
      for (unsigned i = 0; i < VP_MODE_MAX; i++) {
         uint32_t enabled_attribs = _vbo_get_vao_filter(i) &
                                    node->cold->VAO[i]->_EnabledWithMapMode;

         node->state[i] =
            ctx->Driver.CreateGalliumVertexState(ctx, node->cold->VAO[i],
                                                 node->cold->ib.obj,
                                                 enabled_attribs);
         node->private_refcount[i] = 0;
         node->enabled_attribs[i] = enabled_attribs;
      }

      node->ctx = ctx;
      node->mode = node->cold->info.mode;
      assert(node->cold->info.index_size == 4);
   }

   /* Deal with GL_COMPILE_AND_EXECUTE:
    */
   if (ctx->ExecuteFlag) {
      /* _vbo_loopback_vertex_list doesn't use the index buffer, so we have to
       * use buffer_in_ram (which contains all vertices) instead of current_bo
       * (which contains deduplicated vertices *when* UseLoopback is false).
       *
       * The problem is that the VAO offset is based on current_bo's layout,
       * so we have to use a temp value.
       */
      struct gl_vertex_array_object *vao = node->cold->VAO[VP_MODE_SHADER];
      GLintptr original = vao->BufferBinding[0].Offset;
      /* 'start_offset' has been added to all primitives 'start', so undo it here. */
      vao->BufferBinding[0].Offset = -(GLintptr)(start_offset * stride);
      _vbo_loopback_vertex_list(ctx, node, save->vertex_store->buffer_in_ram);
      vao->BufferBinding[0].Offset = original;
   }

   /* Reset our structures for the next run of vertices:
    */
   reset_counters(ctx);
}


/**
 * This is called when we fill a vertex buffer before we hit a glEnd().
 * We
 * TODO -- If no new vertices have been stored, don't bother saving it.
 */
static void
wrap_buffers(struct gl_context *ctx)
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;
   GLint i = save->prim_store->used - 1;
   GLenum mode;

   assert(i < (GLint) save->prim_store->size);
   assert(i >= 0);

   /* Close off in-progress primitive.
    */
   save->prim_store->prims[i].count = (get_vertex_count(save) - save->prim_store->prims[i].start);
   mode = save->prim_store->prims[i].mode;

   /* store the copied vertices, and allocate a new list.
    */
   compile_vertex_list(ctx);

   /* Restart interrupted primitive
    */
   save->prim_store->prims[0].mode = mode;
   save->prim_store->prims[0].begin = 0;
   save->prim_store->prims[0].end = 0;
   save->prim_store->prims[0].start = 0;
   save->prim_store->prims[0].count = 0;
   save->prim_store->used = 1;
}


/**
 * Called only when buffers are wrapped as the result of filling the
 * vertex_store struct.
 */
static void
wrap_filled_vertex(struct gl_context *ctx)
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;
   unsigned numComponents;

   /* Emit a glEnd to close off the last vertex list.
    */
   wrap_buffers(ctx);

   assert(save->vertex_store->used == 0 && save->vertex_store->used == 0);

   /* Copy stored stored vertices to start of new list.
    */
   numComponents = save->copied.nr * save->vertex_size;

   fi_type *buffer_ptr = save->vertex_store->buffer_in_ram;
   if (numComponents) {
      assert(save->copied.buffer);
      memcpy(buffer_ptr,
             save->copied.buffer,
             numComponents * sizeof(fi_type));
      free(save->copied.buffer);
      save->copied.buffer = NULL;
   }
   save->vertex_store->used = numComponents;
}


static void
copy_to_current(struct gl_context *ctx)
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;
   GLbitfield64 enabled = save->enabled & (~BITFIELD64_BIT(VBO_ATTRIB_POS));

   while (enabled) {
      const int i = u_bit_scan64(&enabled);
      assert(save->attrsz[i]);

      if (save->attrtype[i] == GL_DOUBLE ||
          save->attrtype[i] == GL_UNSIGNED_INT64_ARB)
         memcpy(save->current[i], save->attrptr[i], save->attrsz[i] * sizeof(GLfloat));
      else
         COPY_CLEAN_4V_TYPE_AS_UNION(save->current[i], save->attrsz[i],
                                     save->attrptr[i], save->attrtype[i]);
   }
}


static void
copy_from_current(struct gl_context *ctx)
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;
   GLbitfield64 enabled = save->enabled & (~BITFIELD64_BIT(VBO_ATTRIB_POS));

   while (enabled) {
      const int i = u_bit_scan64(&enabled);

      switch (save->attrsz[i]) {
      case 4:
         save->attrptr[i][3] = save->current[i][3];
         FALLTHROUGH;
      case 3:
         save->attrptr[i][2] = save->current[i][2];
         FALLTHROUGH;
      case 2:
         save->attrptr[i][1] = save->current[i][1];
         FALLTHROUGH;
      case 1:
         save->attrptr[i][0] = save->current[i][0];
         break;
      case 0:
         unreachable("Unexpected vertex attribute size");
      }
   }
}


/**
 * Called when we increase the size of a vertex attribute.  For example,
 * if we've seen one or more glTexCoord2f() calls and now we get a
 * glTexCoord3f() call.
 * Flush existing data, set new attrib size, replay copied vertices.
 */
static void
upgrade_vertex(struct gl_context *ctx, GLuint attr, GLuint newsz)
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;
   GLuint oldsz;
   GLuint i;
   fi_type *tmp;

   /* Store the current run of vertices, and emit a GL_END.  Emit a
    * BEGIN in the new buffer.
    */
   if (save->vertex_store->used)
      wrap_buffers(ctx);
   else
      assert(save->copied.nr == 0);

   /* Do a COPY_TO_CURRENT to ensure back-copying works for the case
    * when the attribute already exists in the vertex and is having
    * its size increased.
    */
   copy_to_current(ctx);

   /* Fix up sizes:
    */
   oldsz = save->attrsz[attr];
   save->attrsz[attr] = newsz;
   save->enabled |= BITFIELD64_BIT(attr);

   save->vertex_size += newsz - oldsz;

   /* Recalculate all the attrptr[] values:
    */
   tmp = save->vertex;
   for (i = 0; i < VBO_ATTRIB_MAX; i++) {
      if (save->attrsz[i]) {
         save->attrptr[i] = tmp;
         tmp += save->attrsz[i];
      }
      else {
         save->attrptr[i] = NULL;       /* will not be dereferenced. */
      }
   }

   /* Copy from current to repopulate the vertex with correct values.
    */
   copy_from_current(ctx);

   /* Replay stored vertices to translate them to new format here.
    *
    * If there are copied vertices and the new (upgraded) attribute
    * has not been defined before, this list is somewhat degenerate,
    * and will need fixup at runtime.
    */
   if (save->copied.nr) {
      assert(save->copied.buffer);
      const fi_type *data = save->copied.buffer;
      grow_vertex_storage(ctx, save->copied.nr);
      fi_type *dest = save->vertex_store->buffer_in_ram;

      /* Need to note this and fix up later. This can be done in
       * ATTR_UNION (by copying the new attribute values to the
       * vertices we're copying here) or at runtime (or loopback).
       */
      if (attr != VBO_ATTRIB_POS && save->currentsz[attr][0] == 0) {
         assert(oldsz == 0);
         save->dangling_attr_ref = GL_TRUE;
      }

      for (i = 0; i < save->copied.nr; i++) {
         GLbitfield64 enabled = save->enabled;
         while (enabled) {
            const int j = u_bit_scan64(&enabled);
            assert(save->attrsz[j]);
            if (j == attr) {
               int k;
               const fi_type *src = oldsz ? data : save->current[attr];
               int copy = oldsz ? oldsz : newsz;
               for (k = 0; k < copy; k++)
                  dest[k] = src[k];
               for (; k < newsz; k++) {
                  switch (save->attrtype[j]) {
                     case GL_FLOAT:
                        dest[k] = FLOAT_AS_UNION(k == 3);
                        break;
                     case GL_INT:
                        dest[k] = INT_AS_UNION(k == 3);
                        break;
                     case GL_UNSIGNED_INT:
                        dest[k] = UINT_AS_UNION(k == 3);
                        break;
                     default:
                        dest[k] = FLOAT_AS_UNION(k == 3);
                        assert(!"Unexpected type in upgrade_vertex");
                        break;
                  }
               }
               dest += newsz;
               data += oldsz;
            } else {
               GLint sz = save->attrsz[j];
               for (int k = 0; k < sz; k++)
                  dest[k] = data[k];
               data += sz;
               dest += sz;
            }
         }
      }

      save->vertex_store->used += save->vertex_size * save->copied.nr;
      free(save->copied.buffer);
      save->copied.buffer = NULL;
   }
}


/**
 * This is called when the size of a vertex attribute changes.
 * For example, after seeing one or more glTexCoord2f() calls we
 * get a glTexCoord4f() or glTexCoord1f() call.
 */
static bool
fixup_vertex(struct gl_context *ctx, GLuint attr,
             GLuint sz, GLenum newType)
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;
   bool new_attr_is_bigger = sz > save->attrsz[attr];

   if (new_attr_is_bigger ||
       newType != save->attrtype[attr]) {
      /* New size is larger.  Need to flush existing vertices and get
       * an enlarged vertex format.
       */
      upgrade_vertex(ctx, attr, sz);
   }
   else if (sz < save->active_sz[attr]) {
      GLuint i;
      const fi_type *id = vbo_get_default_vals_as_union(save->attrtype[attr]);

      /* New size is equal or smaller - just need to fill in some
       * zeros.
       */
      for (i = sz; i <= save->attrsz[attr]; i++)
         save->attrptr[attr][i - 1] = id[i - 1];
   }

   save->active_sz[attr] = sz;

   grow_vertex_storage(ctx, 1);

   return new_attr_is_bigger;
}


/**
 * Reset the current size of all vertex attributes to the default
 * value of 0.  This signals that we haven't yet seen any per-vertex
 * commands such as glNormal3f() or glTexCoord2f().
 */
static void
reset_vertex(struct gl_context *ctx)
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;

   while (save->enabled) {
      const int i = u_bit_scan64(&save->enabled);
      assert(save->attrsz[i]);
      save->attrsz[i] = 0;
      save->active_sz[i] = 0;
   }

   save->vertex_size = 0;
}


/**
 * If index=0, does glVertexAttrib*() alias glVertex() to emit a vertex?
 * It depends on a few things, including whether we're inside or outside
 * of glBegin/glEnd.
 */
static inline bool
is_vertex_position(const struct gl_context *ctx, GLuint index)
{
   return (index == 0 &&
           _mesa_attr_zero_aliases_vertex(ctx) &&
           _mesa_inside_dlist_begin_end(ctx));
}



#define ERROR(err)   _mesa_compile_error(ctx, err, __func__);


/* Only one size for each attribute may be active at once.  Eg. if
 * Color3f is installed/active, then Color4f may not be, even if the
 * vertex actually contains 4 color coordinates.  This is because the
 * 3f version won't otherwise set color[3] to 1.0 -- this is the job
 * of the chooser function when switching between Color4f and Color3f.
 */
#define ATTR_UNION(A, N, T, C, V0, V1, V2, V3)                  \
do {                                                            \
   struct vbo_save_context *save = &vbo_context(ctx)->save;     \
   int sz = (sizeof(C) / sizeof(GLfloat));                      \
                                                                \
   if (save->active_sz[A] != N) {                               \
      bool had_dangling_ref = save->dangling_attr_ref;          \
      if (fixup_vertex(ctx, A, N * sz, T) &&                    \
          !had_dangling_ref && save->dangling_attr_ref &&       \
          A != VBO_ATTRIB_POS) {                                \
         fi_type *dest = save->vertex_store->buffer_in_ram;     \
         /* Copy the new attr values to the already copied      \
          * vertices.                                           \
          */                                                    \
         for (int i = 0; i < save->copied.nr; i++) {            \
            GLbitfield64 enabled = save->enabled;               \
            while (enabled) {                                   \
               const int j = u_bit_scan64(&enabled);            \
               if (j == A) {                                    \
                  if (N>0) ((C*) dest)[0] = V0;                 \
                  if (N>1) ((C*) dest)[1] = V1;                 \
                  if (N>2) ((C*) dest)[2] = V2;                 \
                  if (N>3) ((C*) dest)[3] = V3;                 \
               }                                                \
               dest += save->attrsz[j];                         \
            }                                                   \
         }                                                      \
         save->dangling_attr_ref = false;                       \
      }                                                         \
   }                                                            \
                                                                \
   {                                                            \
      C *dest = (C *)save->attrptr[A];                          \
      if (N>0) dest[0] = V0;                                    \
      if (N>1) dest[1] = V1;                                    \
      if (N>2) dest[2] = V2;                                    \
      if (N>3) dest[3] = V3;                                    \
      save->attrtype[A] = T;                                    \
   }                                                            \
                                                                \
   if ((A) == VBO_ATTRIB_POS) {                                 \
      fi_type *buffer_ptr = save->vertex_store->buffer_in_ram + \
                            save->vertex_store->used;           \
                                                                \
      for (int i = 0; i < save->vertex_size; i++)               \
        buffer_ptr[i] = save->vertex[i];                        \
                                                                \
      save->vertex_store->used += save->vertex_size;            \
      unsigned used_next = (save->vertex_store->used +          \
                            save->vertex_size) * sizeof(float); \
      if (used_next > save->vertex_store->buffer_in_ram_size)   \
         grow_vertex_storage(ctx, get_vertex_count(save));      \
   }                                                            \
} while (0)

#define TAG(x) _save_##x

#include "vbo_attrib_tmp.h"


#define MAT( ATTR, N, face, params )                            \
do {                                                            \
   if (face != GL_BACK)                                         \
      MAT_ATTR( ATTR, N, params ); /* front */                  \
   if (face != GL_FRONT)                                        \
      MAT_ATTR( ATTR + 1, N, params ); /* back */               \
} while (0)


/**
 * Save a glMaterial call found between glBegin/End.
 * glMaterial calls outside Begin/End are handled in dlist.c.
 */
static void GLAPIENTRY
_save_Materialfv(GLenum face, GLenum pname, const GLfloat *params)
{
   GET_CURRENT_CONTEXT(ctx);

   if (face != GL_FRONT && face != GL_BACK && face != GL_FRONT_AND_BACK) {
      _mesa_compile_error(ctx, GL_INVALID_ENUM, "glMaterial(face)");
      return;
   }

   switch (pname) {
   case GL_EMISSION:
      MAT(VBO_ATTRIB_MAT_FRONT_EMISSION, 4, face, params);
      break;
   case GL_AMBIENT:
      MAT(VBO_ATTRIB_MAT_FRONT_AMBIENT, 4, face, params);
      break;
   case GL_DIFFUSE:
      MAT(VBO_ATTRIB_MAT_FRONT_DIFFUSE, 4, face, params);
      break;
   case GL_SPECULAR:
      MAT(VBO_ATTRIB_MAT_FRONT_SPECULAR, 4, face, params);
      break;
   case GL_SHININESS:
      if (*params < 0 || *params > ctx->Const.MaxShininess) {
         _mesa_compile_error(ctx, GL_INVALID_VALUE, "glMaterial(shininess)");
      }
      else {
         MAT(VBO_ATTRIB_MAT_FRONT_SHININESS, 1, face, params);
      }
      break;
   case GL_COLOR_INDEXES:
      MAT(VBO_ATTRIB_MAT_FRONT_INDEXES, 3, face, params);
      break;
   case GL_AMBIENT_AND_DIFFUSE:
      MAT(VBO_ATTRIB_MAT_FRONT_AMBIENT, 4, face, params);
      MAT(VBO_ATTRIB_MAT_FRONT_DIFFUSE, 4, face, params);
      break;
   default:
      _mesa_compile_error(ctx, GL_INVALID_ENUM, "glMaterial(pname)");
      return;
   }
}


static void
vbo_init_dispatch_save_begin_end(struct gl_context *ctx);


/* Cope with EvalCoord/CallList called within a begin/end object:
 *     -- Flush current buffer
 *     -- Fallback to opcodes for the rest of the begin/end object.
 */
static void
dlist_fallback(struct gl_context *ctx)
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;

   if (save->vertex_store->used || save->prim_store->used) {
      if (save->prim_store->used > 0 && save->vertex_store->used > 0) {
         assert(save->vertex_size);
         /* Close off in-progress primitive. */
         GLint i = save->prim_store->used - 1;
         save->prim_store->prims[i].count =
            get_vertex_count(save) -
            save->prim_store->prims[i].start;
      }

      /* Need to replay this display list with loopback,
       * unfortunately, otherwise this primitive won't be handled
       * properly:
       */
      save->dangling_attr_ref = GL_TRUE;

      compile_vertex_list(ctx);
   }

   copy_to_current(ctx);
   reset_vertex(ctx);
   if (save->out_of_memory) {
      vbo_install_save_vtxfmt_noop(ctx);
   }
   else {
      _mesa_init_dispatch_save_begin_end(ctx);
   }
   ctx->Driver.SaveNeedFlush = GL_FALSE;
}


static void GLAPIENTRY
_save_EvalCoord1f(GLfloat u)
{
   GET_CURRENT_CONTEXT(ctx);
   dlist_fallback(ctx);
   CALL_EvalCoord1f(ctx->Dispatch.Save, (u));
}

static void GLAPIENTRY
_save_EvalCoord1fv(const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   dlist_fallback(ctx);
   CALL_EvalCoord1fv(ctx->Dispatch.Save, (v));
}

static void GLAPIENTRY
_save_EvalCoord2f(GLfloat u, GLfloat v)
{
   GET_CURRENT_CONTEXT(ctx);
   dlist_fallback(ctx);
   CALL_EvalCoord2f(ctx->Dispatch.Save, (u, v));
}

static void GLAPIENTRY
_save_EvalCoord2fv(const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   dlist_fallback(ctx);
   CALL_EvalCoord2fv(ctx->Dispatch.Save, (v));
}

static void GLAPIENTRY
_save_EvalPoint1(GLint i)
{
   GET_CURRENT_CONTEXT(ctx);
   dlist_fallback(ctx);
   CALL_EvalPoint1(ctx->Dispatch.Save, (i));
}

static void GLAPIENTRY
_save_EvalPoint2(GLint i, GLint j)
{
   GET_CURRENT_CONTEXT(ctx);
   dlist_fallback(ctx);
   CALL_EvalPoint2(ctx->Dispatch.Save, (i, j));
}

static void GLAPIENTRY
_save_CallList(GLuint l)
{
   GET_CURRENT_CONTEXT(ctx);
   dlist_fallback(ctx);
   CALL_CallList(ctx->Dispatch.Save, (l));
}

static void GLAPIENTRY
_save_CallLists(GLsizei n, GLenum type, const GLvoid * v)
{
   GET_CURRENT_CONTEXT(ctx);
   dlist_fallback(ctx);
   CALL_CallLists(ctx->Dispatch.Save, (n, type, v));
}



/**
 * Called when a glBegin is getting compiled into a display list.
 * Updating of ctx->Driver.CurrentSavePrimitive is already taken care of.
 */
void
vbo_save_NotifyBegin(struct gl_context *ctx, GLenum mode,
                     bool no_current_update)
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;
   const GLuint i = save->prim_store->used++;

   ctx->Driver.CurrentSavePrimitive = mode;

   if (!save->prim_store || i >= save->prim_store->size) {
      save->prim_store = realloc_prim_store(save->prim_store, i * 2);
   }
   save->prim_store->prims[i].mode = mode & VBO_SAVE_PRIM_MODE_MASK;
   save->prim_store->prims[i].begin = 1;
   save->prim_store->prims[i].end = 0;
   save->prim_store->prims[i].start = get_vertex_count(save);
   save->prim_store->prims[i].count = 0;

   save->no_current_update = no_current_update;

   vbo_init_dispatch_save_begin_end(ctx);

   /* We need to call vbo_save_SaveFlushVertices() if there's state change */
   ctx->Driver.SaveNeedFlush = GL_TRUE;
}


static void GLAPIENTRY
_save_End(void)
{
   GET_CURRENT_CONTEXT(ctx);
   struct vbo_save_context *save = &vbo_context(ctx)->save;
   const GLint i = save->prim_store->used - 1;

   ctx->Driver.CurrentSavePrimitive = PRIM_OUTSIDE_BEGIN_END;
   save->prim_store->prims[i].end = 1;
   save->prim_store->prims[i].count = (get_vertex_count(save) - save->prim_store->prims[i].start);

   /* Swap out this vertex format while outside begin/end.  Any color,
    * etc. received between here and the next begin will be compiled
    * as opcodes.
    */
   if (save->out_of_memory) {
      vbo_install_save_vtxfmt_noop(ctx);
   }
   else {
      _mesa_init_dispatch_save_begin_end(ctx);
   }
}


static void GLAPIENTRY
_save_Begin(GLenum mode)
{
   GET_CURRENT_CONTEXT(ctx);
   (void) mode;
   _mesa_compile_error(ctx, GL_INVALID_OPERATION, "Recursive glBegin");
}


static void GLAPIENTRY
_save_PrimitiveRestartNV(void)
{
   GET_CURRENT_CONTEXT(ctx);
   struct vbo_save_context *save = &vbo_context(ctx)->save;

   if (save->prim_store->used == 0) {
      /* We're not inside a glBegin/End pair, so calling glPrimitiverRestartNV
       * is an error.
       */
      _mesa_compile_error(ctx, GL_INVALID_OPERATION,
                          "glPrimitiveRestartNV called outside glBegin/End");
   } else {
      /* get current primitive mode */
      GLenum curPrim = save->prim_store->prims[save->prim_store->used - 1].mode;
      bool no_current_update = save->no_current_update;

      /* restart primitive */
      CALL_End(ctx->Dispatch.Current, ());
      vbo_save_NotifyBegin(ctx, curPrim, no_current_update);
   }
}


void GLAPIENTRY
save_Rectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
   GET_CURRENT_CONTEXT(ctx);
   struct _glapi_table *dispatch = ctx->Dispatch.Current;

   vbo_save_NotifyBegin(ctx, GL_QUADS, false);
   CALL_Vertex2f(dispatch, (x1, y1));
   CALL_Vertex2f(dispatch, (x2, y1));
   CALL_Vertex2f(dispatch, (x2, y2));
   CALL_Vertex2f(dispatch, (x1, y2));
   CALL_End(dispatch, ());
}


void GLAPIENTRY
save_Rectdv(const GLdouble *v1, const GLdouble *v2)
{
   save_Rectf((GLfloat) v1[0], (GLfloat) v1[1], (GLfloat) v2[0], (GLfloat) v2[1]);
}

void GLAPIENTRY
save_Rectfv(const GLfloat *v1, const GLfloat *v2)
{
   save_Rectf(v1[0], v1[1], v2[0], v2[1]);
}

void GLAPIENTRY
save_Recti(GLint x1, GLint y1, GLint x2, GLint y2)
{
   save_Rectf((GLfloat) x1, (GLfloat) y1, (GLfloat) x2, (GLfloat) y2);
}

void GLAPIENTRY
save_Rectiv(const GLint *v1, const GLint *v2)
{
   save_Rectf((GLfloat) v1[0], (GLfloat) v1[1], (GLfloat) v2[0], (GLfloat) v2[1]);
}

void GLAPIENTRY
save_Rects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
   save_Rectf((GLfloat) x1, (GLfloat) y1, (GLfloat) x2, (GLfloat) y2);
}

void GLAPIENTRY
save_Rectsv(const GLshort *v1, const GLshort *v2)
{
   save_Rectf((GLfloat) v1[0], (GLfloat) v1[1], (GLfloat) v2[0], (GLfloat) v2[1]);
}

void GLAPIENTRY
save_DrawArrays(GLenum mode, GLint start, GLsizei count)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_vertex_array_object *vao = ctx->Array.VAO;
   struct vbo_save_context *save = &vbo_context(ctx)->save;
   GLint i;

   if (!_mesa_is_valid_prim_mode(ctx, mode)) {
      _mesa_compile_error(ctx, GL_INVALID_ENUM, "glDrawArrays(mode)");
      return;
   }
   if (count < 0) {
      _mesa_compile_error(ctx, GL_INVALID_VALUE, "glDrawArrays(count<0)");
      return;
   }

   if (save->out_of_memory)
      return;

   grow_vertex_storage(ctx, count);

   /* Make sure to process any VBO binding changes */
   _mesa_update_state(ctx);

   _mesa_vao_map_arrays(ctx, vao, GL_MAP_READ_BIT);

   vbo_save_NotifyBegin(ctx, mode, true);

   for (i = 0; i < count; i++)
      _mesa_array_element(ctx, start + i);
   CALL_End(ctx->Dispatch.Current, ());

   _mesa_vao_unmap_arrays(ctx, vao);
}


void GLAPIENTRY
save_MultiDrawArrays(GLenum mode, const GLint *first,
                      const GLsizei *count, GLsizei primcount)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint i;

   if (!_mesa_is_valid_prim_mode(ctx, mode)) {
      _mesa_compile_error(ctx, GL_INVALID_ENUM, "glMultiDrawArrays(mode)");
      return;
   }

   if (primcount < 0) {
      _mesa_compile_error(ctx, GL_INVALID_VALUE,
                          "glMultiDrawArrays(primcount<0)");
      return;
   }

   unsigned vertcount = 0;
   for (i = 0; i < primcount; i++) {
      if (count[i] < 0) {
         _mesa_compile_error(ctx, GL_INVALID_VALUE,
                             "glMultiDrawArrays(count[i]<0)");
         return;
      }
      vertcount += count[i];
   }

   grow_vertex_storage(ctx, vertcount);

   for (i = 0; i < primcount; i++) {
      if (count[i] > 0) {
         save_DrawArrays(mode, first[i], count[i]);
      }
   }
}


static void
array_element(struct gl_context *ctx,
              GLint basevertex, GLuint elt, unsigned index_size_shift)
{
   /* Section 10.3.5 Primitive Restart:
    * [...]
    *    When one of the *BaseVertex drawing commands specified in section 10.5
    * is used, the primitive restart comparison occurs before the basevertex
    * offset is added to the array index.
    */
   /* If PrimitiveRestart is enabled and the index is the RestartIndex
    * then we call PrimitiveRestartNV and return.
    */
   if (ctx->Array._PrimitiveRestart[index_size_shift] &&
       elt == ctx->Array._RestartIndex[index_size_shift]) {
      CALL_PrimitiveRestartNV(ctx->Dispatch.Current, ());
      return;
   }

   _mesa_array_element(ctx, basevertex + elt);
}


/* Could do better by copying the arrays and element list intact and
 * then emitting an indexed prim at runtime.
 */
void GLAPIENTRY
save_DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type,
                             const GLvoid * indices, GLint basevertex)
{
   GET_CURRENT_CONTEXT(ctx);
   struct vbo_save_context *save = &vbo_context(ctx)->save;
   struct gl_vertex_array_object *vao = ctx->Array.VAO;
   struct gl_buffer_object *indexbuf = vao->IndexBufferObj;
   GLint i;

   if (!_mesa_is_valid_prim_mode(ctx, mode)) {
      _mesa_compile_error(ctx, GL_INVALID_ENUM, "glDrawElements(mode)");
      return;
   }
   if (count < 0) {
      _mesa_compile_error(ctx, GL_INVALID_VALUE, "glDrawElements(count<0)");
      return;
   }
   if (type != GL_UNSIGNED_BYTE &&
       type != GL_UNSIGNED_SHORT &&
       type != GL_UNSIGNED_INT) {
      _mesa_compile_error(ctx, GL_INVALID_VALUE, "glDrawElements(count<0)");
      return;
   }

   if (save->out_of_memory)
      return;

   grow_vertex_storage(ctx, count);

   /* Make sure to process any VBO binding changes */
   _mesa_update_state(ctx);

   _mesa_vao_map(ctx, vao, GL_MAP_READ_BIT);

   if (indexbuf)
      indices =
         ADD_POINTERS(indexbuf->Mappings[MAP_INTERNAL].Pointer, indices);

   vbo_save_NotifyBegin(ctx, mode, true);

   switch (type) {
   case GL_UNSIGNED_BYTE:
      for (i = 0; i < count; i++)
         array_element(ctx, basevertex, ((GLubyte *) indices)[i], 0);
      break;
   case GL_UNSIGNED_SHORT:
      for (i = 0; i < count; i++)
         array_element(ctx, basevertex, ((GLushort *) indices)[i], 1);
      break;
   case GL_UNSIGNED_INT:
      for (i = 0; i < count; i++)
         array_element(ctx, basevertex, ((GLuint *) indices)[i], 2);
      break;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM, "glDrawElements(type)");
      break;
   }

   CALL_End(ctx->Dispatch.Current, ());

   _mesa_vao_unmap(ctx, vao);
}

void GLAPIENTRY
save_DrawElements(GLenum mode, GLsizei count, GLenum type,
                   const GLvoid * indices)
{
   save_DrawElementsBaseVertex(mode, count, type, indices, 0);
}


void GLAPIENTRY
save_DrawRangeElements(GLenum mode, GLuint start, GLuint end,
                            GLsizei count, GLenum type,
                            const GLvoid * indices)
{
   GET_CURRENT_CONTEXT(ctx);
   struct vbo_save_context *save = &vbo_context(ctx)->save;

   if (!_mesa_is_valid_prim_mode(ctx, mode)) {
      _mesa_compile_error(ctx, GL_INVALID_ENUM, "glDrawRangeElements(mode)");
      return;
   }
   if (count < 0) {
      _mesa_compile_error(ctx, GL_INVALID_VALUE,
                          "glDrawRangeElements(count<0)");
      return;
   }
   if (type != GL_UNSIGNED_BYTE &&
       type != GL_UNSIGNED_SHORT &&
       type != GL_UNSIGNED_INT) {
      _mesa_compile_error(ctx, GL_INVALID_ENUM, "glDrawRangeElements(type)");
      return;
   }
   if (end < start) {
      _mesa_compile_error(ctx, GL_INVALID_VALUE,
                          "glDrawRangeElements(end < start)");
      return;
   }

   if (save->out_of_memory)
      return;

   save_DrawElements(mode, count, type, indices);
}

void GLAPIENTRY
save_DrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end,
                                 GLsizei count, GLenum type,
                                 const GLvoid *indices, GLint basevertex)
{
   GET_CURRENT_CONTEXT(ctx);

   if (end < start) {
      _mesa_compile_error(ctx, GL_INVALID_VALUE,
                          "glDrawRangeElementsBaseVertex(end < start)");
      return;
   }

   save_DrawElementsBaseVertex(mode, count, type, indices, basevertex);
}

void GLAPIENTRY
save_MultiDrawElements(GLenum mode, const GLsizei *count, GLenum type,
                       const GLvoid * const *indices, GLsizei primcount)
{
   GET_CURRENT_CONTEXT(ctx);
   struct _glapi_table *dispatch = ctx->Dispatch.Current;
   GLsizei i;

   int vertcount = 0;
   for (i = 0; i < primcount; i++) {
      vertcount += count[i];
   }
   grow_vertex_storage(ctx, vertcount);

   for (i = 0; i < primcount; i++) {
      if (count[i] > 0) {
         CALL_DrawElements(dispatch, (mode, count[i], type, indices[i]));
      }
   }
}


void GLAPIENTRY
save_MultiDrawElementsBaseVertex(GLenum mode, const GLsizei *count,
                                  GLenum type,
                                  const GLvoid * const *indices,
                                  GLsizei primcount,
                                  const GLint *basevertex)
{
   GET_CURRENT_CONTEXT(ctx);
   struct _glapi_table *dispatch = ctx->Dispatch.Current;
   GLsizei i;

   int vertcount = 0;
   for (i = 0; i < primcount; i++) {
      vertcount += count[i];
   }
   grow_vertex_storage(ctx, vertcount);

   for (i = 0; i < primcount; i++) {
      if (count[i] > 0) {
         CALL_DrawElementsBaseVertex(dispatch, (mode, count[i], type,
                                     indices[i],
                                     basevertex[i]));
      }
   }
}


static void
vbo_init_dispatch_save_begin_end(struct gl_context *ctx)
{
#define NAME_AE(x) _mesa_##x
#define NAME_CALLLIST(x) _save_##x
#define NAME(x) _save_##x
#define NAME_ES(x) _save_##x

   struct _glapi_table *tab = ctx->Dispatch.Save;
   #include "api_beginend_init.h"
}


void
vbo_save_SaveFlushVertices(struct gl_context *ctx)
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;

   /* Noop when we are actually active:
    */
   if (ctx->Driver.CurrentSavePrimitive <= PRIM_MAX)
      return;

   if (save->vertex_store->used || save->prim_store->used)
      compile_vertex_list(ctx);

   copy_to_current(ctx);
   reset_vertex(ctx);
   ctx->Driver.SaveNeedFlush = GL_FALSE;
}


/**
 * Called from glNewList when we're starting to compile a display list.
 */
void
vbo_save_NewList(struct gl_context *ctx, GLuint list, GLenum mode)
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;

   (void) list;
   (void) mode;

   if (!save->prim_store)
      save->prim_store = realloc_prim_store(NULL, 8);

   if (!save->vertex_store)
      save->vertex_store = CALLOC_STRUCT(vbo_save_vertex_store);

   reset_vertex(ctx);
   ctx->Driver.SaveNeedFlush = GL_FALSE;
}


/**
 * Called from glEndList when we're finished compiling a display list.
 */
void
vbo_save_EndList(struct gl_context *ctx)
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;

   /* EndList called inside a (saved) Begin/End pair?
    */
   if (_mesa_inside_dlist_begin_end(ctx)) {
      if (save->prim_store->used > 0) {
         GLint i = save->prim_store->used - 1;
         ctx->Driver.CurrentSavePrimitive = PRIM_OUTSIDE_BEGIN_END;
         save->prim_store->prims[i].end = 0;
         save->prim_store->prims[i].count = get_vertex_count(save) - save->prim_store->prims[i].start;
      }

      /* Make sure this vertex list gets replayed by the "loopback"
       * mechanism:
       */
      save->dangling_attr_ref = GL_TRUE;
      vbo_save_SaveFlushVertices(ctx);

      /* Swap out this vertex format while outside begin/end.  Any color,
       * etc. received between here and the next begin will be compiled
       * as opcodes.
       */
      _mesa_init_dispatch_save_begin_end(ctx);
   }

   assert(save->vertex_size == 0);
}

/**
 * Called during context creation/init.
 */
static void
current_init(struct gl_context *ctx)
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;
   GLint i;

   for (i = VBO_ATTRIB_POS; i <= VBO_ATTRIB_EDGEFLAG; i++) {
      save->currentsz[i] = &ctx->ListState.ActiveAttribSize[i];
      save->current[i] = (fi_type *) ctx->ListState.CurrentAttrib[i];
   }

   for (i = VBO_ATTRIB_FIRST_MATERIAL; i <= VBO_ATTRIB_LAST_MATERIAL; i++) {
      const GLuint j = i - VBO_ATTRIB_FIRST_MATERIAL;
      assert(j < MAT_ATTRIB_MAX);
      save->currentsz[i] = &ctx->ListState.ActiveMaterialSize[j];
      save->current[i] = (fi_type *) ctx->ListState.CurrentMaterial[j];
   }
}


/**
 * Initialize the display list compiler.  Called during context creation.
 */
void
vbo_save_api_init(struct vbo_save_context *save)
{
   struct gl_context *ctx = gl_context_from_vbo_save(save);

   current_init(ctx);
}
