/*
 * (C) Copyright IBM Corporation 2004, 2005
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * IBM,
 * AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <inttypes.h>
#include <assert.h>
#include <string.h>

#include "util/compiler.h"

#include "glxclient.h"
#include "indirect.h"
#include <GL/glxproto.h>
#include "glxextensions.h"
#include "indirect_vertex_array.h"
#include "indirect_vertex_array_priv.h"

#define __GLX_PAD(n) (((n)+3) & ~3)

/**
 * \file indirect_vertex_array.c
 * Implement GLX protocol for vertex arrays and vertex buffer objects.
 *
 * The most important function in this fill is \c fill_array_info_cache.
 * The \c array_state_vector contains a cache of the ARRAY_INFO data sent
 * in the DrawArrays protocol.  Certain operations, such as enabling or
 * disabling an array, can invalidate this cache.  \c fill_array_info_cache
 * fills-in this data.  Additionally, it examines the enabled state and
 * other factors to determine what "version" of DrawArrays protocoal can be
 * used.
 *
 * Current, only two versions of DrawArrays protocol are implemented.  The
 * first version is the "none" protocol.  This is the fallback when the
 * server does not support GL 1.1 / EXT_vertex_arrays.  It is implemented
 * by sending batches of immediate mode commands that are equivalent to the
 * DrawArrays protocol.
 *
 * The other protocol that is currently implemented is the "old" protocol.
 * This is the GL 1.1 DrawArrays protocol.  The only difference between GL
 * 1.1 and EXT_vertex_arrays is the opcode used for the DrawArrays command.
 * This protocol is called "old" because the ARB is in the process of
 * defining a new protocol, which will probably be called either "new" or
 * "vbo", to support multiple texture coordinate arrays, generic attributes,
 * and vertex buffer objects.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

static void emit_DrawArrays_none(GLenum mode, GLint first, GLsizei count);
static void emit_DrawArrays_old(GLenum mode, GLint first, GLsizei count);

static void emit_DrawElements_none(GLenum mode, GLsizei count, GLenum type,
                                   const GLvoid * indices);
static void emit_DrawElements_old(GLenum mode, GLsizei count, GLenum type,
                                  const GLvoid * indices);


static GLubyte *emit_element_none(GLubyte * dst,
                                  const struct array_state_vector *arrays,
                                  unsigned index);
static GLubyte *emit_element_old(GLubyte * dst,
                                 const struct array_state_vector *arrays,
                                 unsigned index);
static struct array_state *get_array_entry(const struct array_state_vector
                                           *arrays, GLenum key,
                                           unsigned index);
static void fill_array_info_cache(struct array_state_vector *arrays);
static GLboolean validate_mode(struct glx_context * gc, GLenum mode);
static GLboolean validate_count(struct glx_context * gc, GLsizei count);
static GLboolean validate_type(struct glx_context * gc, GLenum type);


/**
 * Table of sizes, in bytes, of a GL types.  All of the type enums are be in
 * the range 0x1400 - 0x140F.  That includes types added by extensions (i.e.,
 * \c GL_HALF_FLOAT_NV).  This elements of this table correspond to the
 * type enums masked with 0x0f.
 *
 * \notes
 * \c GL_HALF_FLOAT_NV is not included.  Neither are \c GL_2_BYTES,
 * \c GL_3_BYTES, or \c GL_4_BYTES.
 */
const GLuint __glXTypeSize_table[16] = {
   1, 1, 2, 2, 4, 4, 4, 0, 0, 0, 8, 0, 0, 0, 0, 0
};


/**
 * Free the per-context array state that was allocated with
 * __glXInitVertexArrayState().
 */
void
__glXFreeVertexArrayState(struct glx_context * gc)
{
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;

   if (arrays) {
      free(arrays->stack);
      arrays->stack = NULL;
      free(arrays->arrays);
      arrays->arrays = NULL;
      free(arrays);
      state->array_state = NULL;
   }
}


/**
 * Initialize vertex array state of a GLX context.
 *
 * \param gc  GLX context whose vertex array state is to be initialized.
 *
 * \warning
 * This function may only be called after struct glx_context::gl_extension_bits,
 * struct glx_context::server_minor, and __GLXcontext::server_major have been
 * initialized.  These values are used to determine what vertex arrays are
 * supported.
 */
void
__glXInitVertexArrayState(struct glx_context * gc)
{
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays;

   unsigned array_count;
   int texture_units = 1, vertex_program_attribs = 0;
   unsigned i, j;

   GLboolean got_fog = GL_FALSE;
   GLboolean got_secondary_color = GL_FALSE;


   arrays = calloc(1, sizeof(struct array_state_vector));
   state->array_state = arrays;

   if (arrays == NULL) {
      __glXSetError(gc, GL_OUT_OF_MEMORY);
      return;
   }

   arrays->old_DrawArrays_possible = !state->NoDrawArraysProtocol;
   arrays->new_DrawArrays_possible = GL_FALSE;
   arrays->DrawArrays = NULL;

   arrays->active_texture_unit = 0;


   /* Determine how many arrays are actually needed.  Only arrays that
    * are supported by the server are create.  For example, if the server
    * supports only 2 texture units, then only 2 texture coordinate arrays
    * are created.
    *
    * At the very least, GL_VERTEX_ARRAY, GL_NORMAL_ARRAY,
    * GL_COLOR_ARRAY, GL_INDEX_ARRAY, GL_TEXTURE_COORD_ARRAY, and
    * GL_EDGE_FLAG_ARRAY are supported.
    */

   array_count = 5;

   if (__glExtensionBitIsEnabled(gc, GL_EXT_fog_coord_bit)
       || (gc->server_major > 1) || (gc->server_minor >= 4)) {
      got_fog = GL_TRUE;
      array_count++;
   }

   if (__glExtensionBitIsEnabled(gc, GL_EXT_secondary_color_bit)
       || (gc->server_major > 1) || (gc->server_minor >= 4)) {
      got_secondary_color = GL_TRUE;
      array_count++;
   }

   if (__glExtensionBitIsEnabled(gc, GL_ARB_multitexture_bit)
       || (gc->server_major > 1) || (gc->server_minor >= 3)) {
      __indirect_glGetIntegerv(GL_MAX_TEXTURE_UNITS, &texture_units);
   }

   if (__glExtensionBitIsEnabled(gc, GL_ARB_vertex_program_bit)) {
      __indirect_glGetProgramivARB(GL_VERTEX_PROGRAM_ARB,
                                   GL_MAX_PROGRAM_ATTRIBS_ARB,
                                   &vertex_program_attribs);
   }

   arrays->num_texture_units = texture_units;
   arrays->num_vertex_program_attribs = vertex_program_attribs;
   array_count += texture_units + vertex_program_attribs;
   arrays->num_arrays = array_count;
   arrays->arrays = calloc(array_count, sizeof(struct array_state));

   if (arrays->arrays == NULL) {
      state->array_state = NULL;
      free(arrays);
      __glXSetError(gc, GL_OUT_OF_MEMORY);
      return;
   }

   arrays->arrays[0].data_type = GL_FLOAT;
   arrays->arrays[0].count = 3;
   arrays->arrays[0].key = GL_NORMAL_ARRAY;
   arrays->arrays[0].normalized = GL_TRUE;
   arrays->arrays[0].old_DrawArrays_possible = GL_TRUE;

   arrays->arrays[1].data_type = GL_FLOAT;
   arrays->arrays[1].count = 4;
   arrays->arrays[1].key = GL_COLOR_ARRAY;
   arrays->arrays[1].normalized = GL_TRUE;
   arrays->arrays[1].old_DrawArrays_possible = GL_TRUE;

   arrays->arrays[2].data_type = GL_FLOAT;
   arrays->arrays[2].count = 1;
   arrays->arrays[2].key = GL_INDEX_ARRAY;
   arrays->arrays[2].old_DrawArrays_possible = GL_TRUE;

   arrays->arrays[3].data_type = GL_UNSIGNED_BYTE;
   arrays->arrays[3].count = 1;
   arrays->arrays[3].key = GL_EDGE_FLAG_ARRAY;
   arrays->arrays[3].old_DrawArrays_possible = GL_TRUE;

   for (i = 0; i < texture_units; i++) {
      arrays->arrays[4 + i].data_type = GL_FLOAT;
      arrays->arrays[4 + i].count = 4;
      arrays->arrays[4 + i].key = GL_TEXTURE_COORD_ARRAY;

      arrays->arrays[4 + i].old_DrawArrays_possible = (i == 0);
      arrays->arrays[4 + i].index = i;
   }

   i = 4 + texture_units;

   if (got_fog) {
      arrays->arrays[i].data_type = GL_FLOAT;
      arrays->arrays[i].count = 1;
      arrays->arrays[i].key = GL_FOG_COORDINATE_ARRAY;
      arrays->arrays[i].old_DrawArrays_possible = GL_TRUE;
      i++;
   }

   if (got_secondary_color) {
      arrays->arrays[i].data_type = GL_FLOAT;
      arrays->arrays[i].count = 3;
      arrays->arrays[i].key = GL_SECONDARY_COLOR_ARRAY;
      arrays->arrays[i].old_DrawArrays_possible = GL_TRUE;
      arrays->arrays[i].normalized = GL_TRUE;
      i++;
   }


   for (j = 0; j < vertex_program_attribs; j++) {
      const unsigned idx = (vertex_program_attribs - (j + 1));


      arrays->arrays[idx + i].data_type = GL_FLOAT;
      arrays->arrays[idx + i].count = 4;
      arrays->arrays[idx + i].key = GL_VERTEX_ATTRIB_ARRAY_POINTER;

      arrays->arrays[idx + i].old_DrawArrays_possible = 0;
      arrays->arrays[idx + i].index = idx;
   }

   i += vertex_program_attribs;


   /* Vertex array *must* be last because of the way that
    * emit_DrawArrays_none works.
    */

   arrays->arrays[i].data_type = GL_FLOAT;
   arrays->arrays[i].count = 4;
   arrays->arrays[i].key = GL_VERTEX_ARRAY;
   arrays->arrays[i].old_DrawArrays_possible = GL_TRUE;

   assert((i + 1) == arrays->num_arrays);

   arrays->stack_index = 0;
   arrays->stack = malloc(sizeof(struct array_stack_state)
                          * arrays->num_arrays
                          * __GL_CLIENT_ATTRIB_STACK_DEPTH);

   if (arrays->stack == NULL) {
      state->array_state = NULL;
      free(arrays->arrays);
      free(arrays);
      __glXSetError(gc, GL_OUT_OF_MEMORY);
      return;
   }
}


/**
 * Calculate the size of a single vertex for the "none" protocol.  This is
 * essentially the size of all the immediate-mode commands required to
 * implement the enabled vertex arrays.
 */
static size_t
calculate_single_vertex_size_none(const struct array_state_vector *arrays)
{
   size_t single_vertex_size = 0;
   unsigned i;


   for (i = 0; i < arrays->num_arrays; i++) {
      if (arrays->arrays[i].enabled) {
         single_vertex_size += arrays->arrays[i].header[0];
      }
   }

   return single_vertex_size;
}


/**
 * Emit a single element using non-DrawArrays protocol.
 */
GLubyte *
emit_element_none(GLubyte * dst,
                  const struct array_state_vector * arrays, unsigned index)
{
   unsigned i;


   for (i = 0; i < arrays->num_arrays; i++) {
      if (arrays->arrays[i].enabled) {
         const size_t offset = index * arrays->arrays[i].true_stride;

         /* The generic attributes can have more data than is in the
          * elements.  This is because a vertex array can be a 2 element,
          * normalized, unsigned short, but the "closest" immediate mode
          * protocol is for a 4Nus.  Since the sizes are small, the
          * performance impact on modern processors should be negligible.
          */
         (void) memset(dst, 0, arrays->arrays[i].header[0]);

         (void) memcpy(dst, arrays->arrays[i].header, 4);

         dst += 4;

         if (arrays->arrays[i].key == GL_TEXTURE_COORD_ARRAY &&
             arrays->arrays[i].index > 0) {
            /* Multi-texture coordinate arrays require the texture target
             * to be sent.  For doubles it is after the data, for everything
             * else it is before.
             */
            GLenum texture = arrays->arrays[i].index + GL_TEXTURE0;
            if (arrays->arrays[i].data_type == GL_DOUBLE) {
               (void) memcpy(dst, ((GLubyte *) arrays->arrays[i].data) + offset,
                             arrays->arrays[i].element_size);
               dst += arrays->arrays[i].element_size;
               (void) memcpy(dst, &texture, 4);
               dst += 4;
            } else {
               (void) memcpy(dst, &texture, 4);
               dst += 4;
               (void) memcpy(dst, ((GLubyte *) arrays->arrays[i].data) + offset,
                             arrays->arrays[i].element_size);
               dst += __GLX_PAD(arrays->arrays[i].element_size);
            }
         } else if (arrays->arrays[i].key == GL_VERTEX_ATTRIB_ARRAY_POINTER) {
            /* Vertex attribute data requires the index sent first.
             */
            (void) memcpy(dst, &arrays->arrays[i].index, 4);
            dst += 4;
            (void) memcpy(dst, ((GLubyte *) arrays->arrays[i].data) + offset,
                          arrays->arrays[i].element_size);
            dst += __GLX_PAD(arrays->arrays[i].element_size);
         } else {
            (void) memcpy(dst, ((GLubyte *) arrays->arrays[i].data) + offset,
                          arrays->arrays[i].element_size);
            dst += __GLX_PAD(arrays->arrays[i].element_size);
         }
      }
   }

   return dst;
}


/**
 * Emit a single element using "old" DrawArrays protocol from
 * EXT_vertex_arrays / OpenGL 1.1.
 */
GLubyte *
emit_element_old(GLubyte * dst,
                 const struct array_state_vector * arrays, unsigned index)
{
   unsigned i;


   for (i = 0; i < arrays->num_arrays; i++) {
      if (arrays->arrays[i].enabled) {
         const size_t offset = index * arrays->arrays[i].true_stride;

         (void) memcpy(dst, ((GLubyte *) arrays->arrays[i].data) + offset,
                       arrays->arrays[i].element_size);

         dst += __GLX_PAD(arrays->arrays[i].element_size);
      }
   }

   return dst;
}


struct array_state *
get_array_entry(const struct array_state_vector *arrays,
                GLenum key, unsigned index)
{
   unsigned i;

   for (i = 0; i < arrays->num_arrays; i++) {
      if ((arrays->arrays[i].key == key)
          && (arrays->arrays[i].index == index)) {
         return &arrays->arrays[i];
      }
   }

   return NULL;
}


static GLboolean
allocate_array_info_cache(struct array_state_vector *arrays,
                          size_t required_size)
{
#define MAX_HEADER_SIZE 20
   if (arrays->array_info_cache_buffer_size < required_size) {
      GLubyte *temp = realloc(arrays->array_info_cache_base,
                              required_size + MAX_HEADER_SIZE);

      if (temp == NULL) {
         return GL_FALSE;
      }

      arrays->array_info_cache_base = temp;
      arrays->array_info_cache = temp + MAX_HEADER_SIZE;
      arrays->array_info_cache_buffer_size = required_size;
   }

   arrays->array_info_cache_size = required_size;
   return GL_TRUE;
}


/**
 */
void
fill_array_info_cache(struct array_state_vector *arrays)
{
   GLboolean old_DrawArrays_possible;
   unsigned i;


   /* Determine how many arrays are enabled.
    */

   arrays->enabled_client_array_count = 0;
   old_DrawArrays_possible = arrays->old_DrawArrays_possible;
   for (i = 0; i < arrays->num_arrays; i++) {
      if (arrays->arrays[i].enabled) {
         arrays->enabled_client_array_count++;
         old_DrawArrays_possible &= arrays->arrays[i].old_DrawArrays_possible;
      }
   }

   if (arrays->new_DrawArrays_possible) {
      assert(!arrays->new_DrawArrays_possible);
   }
   else if (old_DrawArrays_possible) {
      const size_t required_size = arrays->enabled_client_array_count * 12;
      uint32_t *info;


      if (!allocate_array_info_cache(arrays, required_size)) {
         return;
      }


      info = (uint32_t *) arrays->array_info_cache;
      for (i = 0; i < arrays->num_arrays; i++) {
         if (arrays->arrays[i].enabled) {
            *(info++) = arrays->arrays[i].data_type;
            *(info++) = arrays->arrays[i].count;
            *(info++) = arrays->arrays[i].key;
         }
      }

      arrays->DrawArrays = emit_DrawArrays_old;
      arrays->DrawElements = emit_DrawElements_old;
   }
   else {
      arrays->DrawArrays = emit_DrawArrays_none;
      arrays->DrawElements = emit_DrawElements_none;
   }

   arrays->array_info_cache_valid = GL_TRUE;
}


/**
 * Emit a \c glDrawArrays command using the "none" protocol.  That is,
 * emit immediate-mode commands that are equivalent to the requested
 * \c glDrawArrays command.  This is used with servers that don't support
 * the OpenGL 1.1 / EXT_vertex_arrays DrawArrays protocol or in cases where
 * vertex state is enabled that is not compatible with that protocol.
 */
void
emit_DrawArrays_none(GLenum mode, GLint first, GLsizei count)
{
   struct glx_context *gc = __glXGetCurrentContext();
   const __GLXattribute *state =
      (const __GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;

   size_t single_vertex_size;
   GLubyte *pc;
   unsigned i;
   static const uint16_t begin_cmd[2] = { 8, X_GLrop_Begin };
   static const uint16_t end_cmd[2] = { 4, X_GLrop_End };


   single_vertex_size = calculate_single_vertex_size_none(arrays);

   pc = gc->pc;

   (void) memcpy(pc, begin_cmd, 4);
   *(int *) (pc + 4) = mode;

   pc += 8;

   for (i = 0; i < count; i++) {
      if ((pc + single_vertex_size) >= gc->bufEnd) {
         pc = __glXFlushRenderBuffer(gc, pc);
      }

      pc = emit_element_none(pc, arrays, first + i);
   }

   if ((pc + 4) >= gc->bufEnd) {
      pc = __glXFlushRenderBuffer(gc, pc);
   }

   (void) memcpy(pc, end_cmd, 4);
   pc += 4;

   gc->pc = pc;
   if (gc->pc > gc->limit) {
      (void) __glXFlushRenderBuffer(gc, gc->pc);
   }
}


/**
 * Emit the header data for the GL 1.1 / EXT_vertex_arrays DrawArrays
 * protocol.
 *
 * \param gc                    GLX context.
 * \param arrays                Array state.
 * \param elements_per_request  Location to store the number of elements that
 *                              can fit in a single Render / RenderLarge
 *                              command.
 * \param total_request         Total number of requests for a RenderLarge
 *                              command.  If a Render command is used, this
 *                              will be zero.
 * \param mode                  Drawing mode.
 * \param count                 Number of vertices.
 *
 * \returns
 * A pointer to the buffer for array data.
 */
static GLubyte *
emit_DrawArrays_header_old(struct glx_context * gc,
                           struct array_state_vector *arrays,
                           size_t * elements_per_request,
                           unsigned int *total_requests,
                           GLenum mode, GLsizei count)
{
   size_t command_size;
   size_t single_vertex_size;
   const unsigned header_size = 16;
   unsigned i;
   GLubyte *pc;


   /* Determine the size of the whole command.  This includes the header,
    * the ARRAY_INFO data and the array data.  Once this size is calculated,
    * it will be known whether a Render or RenderLarge command is needed.
    */

   single_vertex_size = 0;
   for (i = 0; i < arrays->num_arrays; i++) {
      if (arrays->arrays[i].enabled) {
         single_vertex_size += __GLX_PAD(arrays->arrays[i].element_size);
      }
   }

   command_size = arrays->array_info_cache_size + header_size
      + (single_vertex_size * count);


   /* Write the header for either a Render command or a RenderLarge
    * command.  After the header is written, write the ARRAY_INFO data.
    */

   if (command_size > gc->maxSmallRenderCommandSize) {
      /* maxSize is the maximum amount of data can be stuffed into a single
       * packet.  sz_xGLXRenderReq is added because bufSize is the maximum
       * packet size minus sz_xGLXRenderReq.
       */
      const size_t maxSize = (gc->bufSize + sz_xGLXRenderReq)
         - sz_xGLXRenderLargeReq;
      unsigned vertex_requests;


      /* Calculate the number of data packets that will be required to send
       * the whole command.  To do this, the number of vertices that
       * will fit in a single buffer must be calculated.
       *
       * The important value here is elements_per_request.  This is the
       * number of complete array elements that will fit in a single
       * buffer.  There may be some wasted space at the end of the buffer,
       * but splitting elements across buffer boundaries would be painful.
       */

      elements_per_request[0] = maxSize / single_vertex_size;

      vertex_requests = (count + elements_per_request[0] - 1)
         / elements_per_request[0];

      *total_requests = vertex_requests + 1;


      __glXFlushRenderBuffer(gc, gc->pc);

      command_size += 4;

      pc = ((GLubyte *) arrays->array_info_cache) - (header_size + 4);
      *(uint32_t *) (pc + 0) = command_size;
      *(uint32_t *) (pc + 4) = X_GLrop_DrawArrays;
      *(uint32_t *) (pc + 8) = count;
      *(uint32_t *) (pc + 12) = arrays->enabled_client_array_count;
      *(uint32_t *) (pc + 16) = mode;

      __glXSendLargeChunk(gc, 1, *total_requests, pc,
                          header_size + 4 + arrays->array_info_cache_size);

      pc = gc->pc;
   }
   else {
      if ((gc->pc + command_size) >= gc->bufEnd) {
         (void) __glXFlushRenderBuffer(gc, gc->pc);
      }

      pc = gc->pc;
      *(uint16_t *) (pc + 0) = command_size;
      *(uint16_t *) (pc + 2) = X_GLrop_DrawArrays;
      *(uint32_t *) (pc + 4) = count;
      *(uint32_t *) (pc + 8) = arrays->enabled_client_array_count;
      *(uint32_t *) (pc + 12) = mode;

      pc += header_size;

      (void) memcpy(pc, arrays->array_info_cache,
                    arrays->array_info_cache_size);
      pc += arrays->array_info_cache_size;

      *elements_per_request = count;
      *total_requests = 0;
   }


   return pc;
}


/**
 */
void
emit_DrawArrays_old(GLenum mode, GLint first, GLsizei count)
{
   struct glx_context *gc = __glXGetCurrentContext();
   const __GLXattribute *state =
      (const __GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;

   GLubyte *pc;
   size_t elements_per_request;
   unsigned total_requests = 0;
   unsigned i;


   pc = emit_DrawArrays_header_old(gc, arrays, &elements_per_request,
                                   &total_requests, mode, count);


   /* Write the arrays.
    */

   if (total_requests == 0) {
      assert(elements_per_request >= count);

      for (i = 0; i < count; i++) {
         pc = emit_element_old(pc, arrays, i + first);
      }

      assert(pc <= gc->bufEnd);

      gc->pc = pc;
      if (gc->pc > gc->limit) {
         (void) __glXFlushRenderBuffer(gc, gc->pc);
      }
   }
   else {
      unsigned req;


      for (req = 2; req <= total_requests; req++) {
         if (count < elements_per_request) {
            elements_per_request = count;
         }

         pc = gc->pc;
         for (i = 0; i < elements_per_request; i++) {
            pc = emit_element_old(pc, arrays, i + first);
         }

         first += elements_per_request;

         __glXSendLargeChunk(gc, req, total_requests, gc->pc, pc - gc->pc);

         count -= elements_per_request;
      }
   }
}


void
emit_DrawElements_none(GLenum mode, GLsizei count, GLenum type,
                       const GLvoid * indices)
{
   struct glx_context *gc = __glXGetCurrentContext();
   const __GLXattribute *state =
      (const __GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;
   static const uint16_t begin_cmd[2] = { 8, X_GLrop_Begin };
   static const uint16_t end_cmd[2] = { 4, X_GLrop_End };

   GLubyte *pc;
   size_t single_vertex_size;
   unsigned i;


   single_vertex_size = calculate_single_vertex_size_none(arrays);


   if ((gc->pc + single_vertex_size) >= gc->bufEnd) {
      gc->pc = __glXFlushRenderBuffer(gc, gc->pc);
   }

   pc = gc->pc;

   (void) memcpy(pc, begin_cmd, 4);
   *(int *) (pc + 4) = mode;

   pc += 8;

   for (i = 0; i < count; i++) {
      unsigned index = 0;

      if ((pc + single_vertex_size) >= gc->bufEnd) {
         pc = __glXFlushRenderBuffer(gc, pc);
      }

      switch (type) {
      case GL_UNSIGNED_INT:
         index = (unsigned) (((GLuint *) indices)[i]);
         break;
      case GL_UNSIGNED_SHORT:
         index = (unsigned) (((GLushort *) indices)[i]);
         break;
      case GL_UNSIGNED_BYTE:
         index = (unsigned) (((GLubyte *) indices)[i]);
         break;
      }
      pc = emit_element_none(pc, arrays, index);
   }

   if ((pc + 4) >= gc->bufEnd) {
      pc = __glXFlushRenderBuffer(gc, pc);
   }

   (void) memcpy(pc, end_cmd, 4);
   pc += 4;

   gc->pc = pc;
   if (gc->pc > gc->limit) {
      (void) __glXFlushRenderBuffer(gc, gc->pc);
   }
}


/**
 */
void
emit_DrawElements_old(GLenum mode, GLsizei count, GLenum type,
                      const GLvoid * indices)
{
   struct glx_context *gc = __glXGetCurrentContext();
   const __GLXattribute *state =
      (const __GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;

   GLubyte *pc;
   size_t elements_per_request;
   unsigned total_requests = 0;
   unsigned i;
   unsigned req;
   unsigned req_element = 0;


   pc = emit_DrawArrays_header_old(gc, arrays, &elements_per_request,
                                   &total_requests, mode, count);


   /* Write the arrays.
    */

   req = 2;
   while (count > 0) {
      if (count < elements_per_request) {
         elements_per_request = count;
      }

      switch (type) {
      case GL_UNSIGNED_INT:{
            const GLuint *ui_ptr = (const GLuint *) indices + req_element;

            for (i = 0; i < elements_per_request; i++) {
               const GLint index = (GLint) * (ui_ptr++);
               pc = emit_element_old(pc, arrays, index);
            }
            break;
         }
      case GL_UNSIGNED_SHORT:{
            const GLushort *us_ptr = (const GLushort *) indices + req_element;

            for (i = 0; i < elements_per_request; i++) {
               const GLint index = (GLint) * (us_ptr++);
               pc = emit_element_old(pc, arrays, index);
            }
            break;
         }
      case GL_UNSIGNED_BYTE:{
            const GLubyte *ub_ptr = (const GLubyte *) indices + req_element;

            for (i = 0; i < elements_per_request; i++) {
               const GLint index = (GLint) * (ub_ptr++);
               pc = emit_element_old(pc, arrays, index);
            }
            break;
         }
      }

      if (total_requests != 0) {
         __glXSendLargeChunk(gc, req, total_requests, gc->pc, pc - gc->pc);
         pc = gc->pc;
         req++;
      }

      count -= elements_per_request;
      req_element += elements_per_request;
   }


   assert((total_requests == 0) || ((req - 1) == total_requests));

   if (total_requests == 0) {
      assert(pc <= gc->bufEnd);

      gc->pc = pc;
      if (gc->pc > gc->limit) {
         (void) __glXFlushRenderBuffer(gc, gc->pc);
      }
   }
}


/**
 * Validate that the \c mode parameter to \c glDrawArrays, et. al. is valid.
 * If it is not valid, then an error code is set in the GLX context.
 *
 * \returns
 * \c GL_TRUE if the argument is valid, \c GL_FALSE if is not.
 */
static GLboolean
validate_mode(struct glx_context * gc, GLenum mode)
{
   switch (mode) {
   case GL_POINTS:
   case GL_LINE_STRIP:
   case GL_LINE_LOOP:
   case GL_LINES:
   case GL_TRIANGLE_STRIP:
   case GL_TRIANGLE_FAN:
   case GL_TRIANGLES:
   case GL_QUAD_STRIP:
   case GL_QUADS:
   case GL_POLYGON:
      break;
   default:
      __glXSetError(gc, GL_INVALID_ENUM);
      return GL_FALSE;
   }

   return GL_TRUE;
}


/**
 * Validate that the \c count parameter to \c glDrawArrays, et. al. is valid.
 * A value less than zero is invalid and will result in \c GL_INVALID_VALUE
 * being set.  A value of zero will not result in an error being set, but
 * will result in \c GL_FALSE being returned.
 *
 * \returns
 * \c GL_TRUE if the argument is valid, \c GL_FALSE if it is not.
 */
static GLboolean
validate_count(struct glx_context * gc, GLsizei count)
{
   if (count < 0) {
      __glXSetError(gc, GL_INVALID_VALUE);
   }

   return (count > 0);
}


/**
 * Validate that the \c type parameter to \c glDrawElements, et. al. is
 * valid.  Only \c GL_UNSIGNED_BYTE, \c GL_UNSIGNED_SHORT, and
 * \c GL_UNSIGNED_INT are valid.
 *
 * \returns
 * \c GL_TRUE if the argument is valid, \c GL_FALSE if it is not.
 */
static GLboolean
validate_type(struct glx_context * gc, GLenum type)
{
   switch (type) {
   case GL_UNSIGNED_INT:
   case GL_UNSIGNED_SHORT:
   case GL_UNSIGNED_BYTE:
      return GL_TRUE;
   default:
      __glXSetError(gc, GL_INVALID_ENUM);
      return GL_FALSE;
   }
}


void
__indirect_glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
   struct glx_context *gc = __glXGetCurrentContext();
   const __GLXattribute *state =
      (const __GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;


   if (validate_mode(gc, mode) && validate_count(gc, count)) {
      if (!arrays->array_info_cache_valid) {
         fill_array_info_cache(arrays);
      }

      arrays->DrawArrays(mode, first, count);
   }
}


void
__indirect_glArrayElement(GLint index)
{
   struct glx_context *gc = __glXGetCurrentContext();
   const __GLXattribute *state =
      (const __GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;

   size_t single_vertex_size;


   single_vertex_size = calculate_single_vertex_size_none(arrays);

   if ((gc->pc + single_vertex_size) >= gc->bufEnd) {
      gc->pc = __glXFlushRenderBuffer(gc, gc->pc);
   }

   gc->pc = emit_element_none(gc->pc, arrays, index);

   if (gc->pc > gc->limit) {
      (void) __glXFlushRenderBuffer(gc, gc->pc);
   }
}


void
__indirect_glDrawElements(GLenum mode, GLsizei count, GLenum type,
                          const GLvoid * indices)
{
   struct glx_context *gc = __glXGetCurrentContext();
   const __GLXattribute *state =
      (const __GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;


   if (validate_mode(gc, mode) && validate_count(gc, count)
       && validate_type(gc, type)) {
      if (!arrays->array_info_cache_valid) {
         fill_array_info_cache(arrays);
      }

      arrays->DrawElements(mode, count, type, indices);
   }
}


void
__indirect_glDrawRangeElements(GLenum mode, GLuint start, GLuint end,
                               GLsizei count, GLenum type,
                               const GLvoid * indices)
{
   struct glx_context *gc = __glXGetCurrentContext();
   const __GLXattribute *state =
      (const __GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;


   if (validate_mode(gc, mode) && validate_count(gc, count)
       && validate_type(gc, type)) {
      if (end < start) {
         __glXSetError(gc, GL_INVALID_VALUE);
         return;
      }

      if (!arrays->array_info_cache_valid) {
         fill_array_info_cache(arrays);
      }

      arrays->DrawElements(mode, count, type, indices);
   }
}


void
__indirect_glMultiDrawArrays(GLenum mode, const GLint *first,
                                const GLsizei *count, GLsizei primcount)
{
   struct glx_context *gc = __glXGetCurrentContext();
   const __GLXattribute *state =
      (const __GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;
   GLsizei i;


   if (validate_mode(gc, mode)) {
      if (!arrays->array_info_cache_valid) {
         fill_array_info_cache(arrays);
      }

      for (i = 0; i < primcount; i++) {
         if (validate_count(gc, count[i])) {
            arrays->DrawArrays(mode, first[i], count[i]);
         }
      }
   }
}


void
__indirect_glMultiDrawElements(GLenum mode, const GLsizei * count,
                               GLenum type, const GLvoid * const * indices,
                               GLsizei primcount)
{
   struct glx_context *gc = __glXGetCurrentContext();
   const __GLXattribute *state =
      (const __GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;
   GLsizei i;


   if (validate_mode(gc, mode) && validate_type(gc, type)) {
      if (!arrays->array_info_cache_valid) {
         fill_array_info_cache(arrays);
      }

      for (i = 0; i < primcount; i++) {
         if (validate_count(gc, count[i])) {
            arrays->DrawElements(mode, count[i], type, indices[i]);
         }
      }
   }
}


/* The HDR_SIZE macro argument is the command header size (4 bytes)
 * plus any additional index word e.g. for texture units or vertex
 * attributes.
 */
#define COMMON_ARRAY_DATA_INIT(a, PTR, TYPE, STRIDE, COUNT, NORMALIZED, HDR_SIZE, OPCODE) \
  do {                                                                  \
    (a)->data = PTR;                                                    \
    (a)->data_type = TYPE;                                              \
    (a)->user_stride = STRIDE;                                          \
    (a)->count = COUNT;                                                 \
    (a)->normalized = NORMALIZED;                                       \
                                                                        \
    (a)->element_size = __glXTypeSize( TYPE ) * COUNT;                  \
    (a)->true_stride = (STRIDE == 0)                                    \
      ? (a)->element_size : STRIDE;                                     \
                                                                        \
    (a)->header[0] = __GLX_PAD(HDR_SIZE + (a)->element_size);           \
    (a)->header[1] = OPCODE;                                            \
  } while(0)


void
__indirect_glVertexPointer(GLint size, GLenum type, GLsizei stride,
                           const GLvoid * pointer)
{
   static const uint16_t short_ops[5] = {
      0, 0, X_GLrop_Vertex2sv, X_GLrop_Vertex3sv, X_GLrop_Vertex4sv
   };
   static const uint16_t int_ops[5] = {
      0, 0, X_GLrop_Vertex2iv, X_GLrop_Vertex3iv, X_GLrop_Vertex4iv
   };
   static const uint16_t float_ops[5] = {
      0, 0, X_GLrop_Vertex2fv, X_GLrop_Vertex3fv, X_GLrop_Vertex4fv
   };
   static const uint16_t double_ops[5] = {
      0, 0, X_GLrop_Vertex2dv, X_GLrop_Vertex3dv, X_GLrop_Vertex4dv
   };
   uint16_t opcode;
   struct glx_context *gc = __glXGetCurrentContext();
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;
   struct array_state *a;


   if (size < 2 || size > 4 || stride < 0) {
      __glXSetError(gc, GL_INVALID_VALUE);
      return;
   }

   switch (type) {
   case GL_SHORT:
      opcode = short_ops[size];
      break;
   case GL_INT:
      opcode = int_ops[size];
      break;
   case GL_FLOAT:
      opcode = float_ops[size];
      break;
   case GL_DOUBLE:
      opcode = double_ops[size];
      break;
   default:
      __glXSetError(gc, GL_INVALID_ENUM);
      return;
   }

   a = get_array_entry(arrays, GL_VERTEX_ARRAY, 0);
   assert(a != NULL);
   COMMON_ARRAY_DATA_INIT(a, pointer, type, stride, size, GL_FALSE, 4,
                          opcode);

   if (a->enabled) {
      arrays->array_info_cache_valid = GL_FALSE;
   }
}


void
__indirect_glNormalPointer(GLenum type, GLsizei stride,
                           const GLvoid * pointer)
{
   uint16_t opcode;
   struct glx_context *gc = __glXGetCurrentContext();
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;
   struct array_state *a;


   if (stride < 0) {
      __glXSetError(gc, GL_INVALID_VALUE);
      return;
   }

   switch (type) {
   case GL_BYTE:
      opcode = X_GLrop_Normal3bv;
      break;
   case GL_SHORT:
      opcode = X_GLrop_Normal3sv;
      break;
   case GL_INT:
      opcode = X_GLrop_Normal3iv;
      break;
   case GL_FLOAT:
      opcode = X_GLrop_Normal3fv;
      break;
   case GL_DOUBLE:
      opcode = X_GLrop_Normal3dv;
      break;
   default:
      __glXSetError(gc, GL_INVALID_ENUM);
      return;
   }

   a = get_array_entry(arrays, GL_NORMAL_ARRAY, 0);
   assert(a != NULL);
   COMMON_ARRAY_DATA_INIT(a, pointer, type, stride, 3, GL_TRUE, 4, opcode);

   if (a->enabled) {
      arrays->array_info_cache_valid = GL_FALSE;
   }
}


void
__indirect_glColorPointer(GLint size, GLenum type, GLsizei stride,
                          const GLvoid * pointer)
{
   static const uint16_t byte_ops[5] = {
      0, 0, 0, X_GLrop_Color3bv, X_GLrop_Color4bv
   };
   static const uint16_t ubyte_ops[5] = {
      0, 0, 0, X_GLrop_Color3ubv, X_GLrop_Color4ubv
   };
   static const uint16_t short_ops[5] = {
      0, 0, 0, X_GLrop_Color3sv, X_GLrop_Color4sv
   };
   static const uint16_t ushort_ops[5] = {
      0, 0, 0, X_GLrop_Color3usv, X_GLrop_Color4usv
   };
   static const uint16_t int_ops[5] = {
      0, 0, 0, X_GLrop_Color3iv, X_GLrop_Color4iv
   };
   static const uint16_t uint_ops[5] = {
      0, 0, 0, X_GLrop_Color3uiv, X_GLrop_Color4uiv
   };
   static const uint16_t float_ops[5] = {
      0, 0, 0, X_GLrop_Color3fv, X_GLrop_Color4fv
   };
   static const uint16_t double_ops[5] = {
      0, 0, 0, X_GLrop_Color3dv, X_GLrop_Color4dv
   };
   uint16_t opcode;
   struct glx_context *gc = __glXGetCurrentContext();
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;
   struct array_state *a;


   if (size < 3 || size > 4 || stride < 0) {
      __glXSetError(gc, GL_INVALID_VALUE);
      return;
   }

   switch (type) {
   case GL_BYTE:
      opcode = byte_ops[size];
      break;
   case GL_UNSIGNED_BYTE:
      opcode = ubyte_ops[size];
      break;
   case GL_SHORT:
      opcode = short_ops[size];
      break;
   case GL_UNSIGNED_SHORT:
      opcode = ushort_ops[size];
      break;
   case GL_INT:
      opcode = int_ops[size];
      break;
   case GL_UNSIGNED_INT:
      opcode = uint_ops[size];
      break;
   case GL_FLOAT:
      opcode = float_ops[size];
      break;
   case GL_DOUBLE:
      opcode = double_ops[size];
      break;
   default:
      __glXSetError(gc, GL_INVALID_ENUM);
      return;
   }

   a = get_array_entry(arrays, GL_COLOR_ARRAY, 0);
   assert(a != NULL);
   COMMON_ARRAY_DATA_INIT(a, pointer, type, stride, size, GL_TRUE, 4, opcode);

   if (a->enabled) {
      arrays->array_info_cache_valid = GL_FALSE;
   }
}


void
__indirect_glIndexPointer(GLenum type, GLsizei stride, const GLvoid * pointer)
{
   uint16_t opcode;
   struct glx_context *gc = __glXGetCurrentContext();
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;
   struct array_state *a;


   if (stride < 0) {
      __glXSetError(gc, GL_INVALID_VALUE);
      return;
   }

   switch (type) {
   case GL_UNSIGNED_BYTE:
      opcode = X_GLrop_Indexubv;
      break;
   case GL_SHORT:
      opcode = X_GLrop_Indexsv;
      break;
   case GL_INT:
      opcode = X_GLrop_Indexiv;
      break;
   case GL_FLOAT:
      opcode = X_GLrop_Indexfv;
      break;
   case GL_DOUBLE:
      opcode = X_GLrop_Indexdv;
      break;
   default:
      __glXSetError(gc, GL_INVALID_ENUM);
      return;
   }

   a = get_array_entry(arrays, GL_INDEX_ARRAY, 0);
   assert(a != NULL);
   COMMON_ARRAY_DATA_INIT(a, pointer, type, stride, 1, GL_FALSE, 4, opcode);

   if (a->enabled) {
      arrays->array_info_cache_valid = GL_FALSE;
   }
}


void
__indirect_glEdgeFlagPointer(GLsizei stride, const GLvoid * pointer)
{
   struct glx_context *gc = __glXGetCurrentContext();
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;
   struct array_state *a;


   if (stride < 0) {
      __glXSetError(gc, GL_INVALID_VALUE);
      return;
   }


   a = get_array_entry(arrays, GL_EDGE_FLAG_ARRAY, 0);
   assert(a != NULL);
   COMMON_ARRAY_DATA_INIT(a, pointer, GL_UNSIGNED_BYTE, stride, 1, GL_FALSE,
                          4, X_GLrop_EdgeFlagv);

   if (a->enabled) {
      arrays->array_info_cache_valid = GL_FALSE;
   }
}


void
__indirect_glTexCoordPointer(GLint size, GLenum type, GLsizei stride,
                             const GLvoid * pointer)
{
   static const uint16_t short_ops[5] = {
      0, X_GLrop_TexCoord1sv, X_GLrop_TexCoord2sv, X_GLrop_TexCoord3sv,
      X_GLrop_TexCoord4sv
   };
   static const uint16_t int_ops[5] = {
      0, X_GLrop_TexCoord1iv, X_GLrop_TexCoord2iv, X_GLrop_TexCoord3iv,
      X_GLrop_TexCoord4iv
   };
   static const uint16_t float_ops[5] = {
      0, X_GLrop_TexCoord1fv, X_GLrop_TexCoord2fv, X_GLrop_TexCoord3fv,
      X_GLrop_TexCoord4fv
   };
   static const uint16_t double_ops[5] = {
      0, X_GLrop_TexCoord1dv, X_GLrop_TexCoord2dv, X_GLrop_TexCoord3dv,
      X_GLrop_TexCoord4dv
   };

   static const uint16_t mshort_ops[5] = {
      0, X_GLrop_MultiTexCoord1svARB, X_GLrop_MultiTexCoord2svARB,
      X_GLrop_MultiTexCoord3svARB, X_GLrop_MultiTexCoord4svARB
   };
   static const uint16_t mint_ops[5] = {
      0, X_GLrop_MultiTexCoord1ivARB, X_GLrop_MultiTexCoord2ivARB,
      X_GLrop_MultiTexCoord3ivARB, X_GLrop_MultiTexCoord4ivARB
   };
   static const uint16_t mfloat_ops[5] = {
      0, X_GLrop_MultiTexCoord1fvARB, X_GLrop_MultiTexCoord2fvARB,
      X_GLrop_MultiTexCoord3fvARB, X_GLrop_MultiTexCoord4fvARB
   };
   static const uint16_t mdouble_ops[5] = {
      0, X_GLrop_MultiTexCoord1dvARB, X_GLrop_MultiTexCoord2dvARB,
      X_GLrop_MultiTexCoord3dvARB, X_GLrop_MultiTexCoord4dvARB
   };

   uint16_t opcode;
   struct glx_context *gc = __glXGetCurrentContext();
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;
   struct array_state *a;
   unsigned header_size;
   unsigned index;


   if (size < 1 || size > 4 || stride < 0) {
      __glXSetError(gc, GL_INVALID_VALUE);
      return;
   }

   index = arrays->active_texture_unit;
   if (index == 0) {
      switch (type) {
      case GL_SHORT:
         opcode = short_ops[size];
         break;
      case GL_INT:
         opcode = int_ops[size];
         break;
      case GL_FLOAT:
         opcode = float_ops[size];
         break;
      case GL_DOUBLE:
         opcode = double_ops[size];
         break;
      default:
         __glXSetError(gc, GL_INVALID_ENUM);
         return;
      }

      header_size = 4;
   }
   else {
      switch (type) {
      case GL_SHORT:
         opcode = mshort_ops[size];
         break;
      case GL_INT:
         opcode = mint_ops[size];
         break;
      case GL_FLOAT:
         opcode = mfloat_ops[size];
         break;
      case GL_DOUBLE:
         opcode = mdouble_ops[size];
         break;
      default:
         __glXSetError(gc, GL_INVALID_ENUM);
         return;
      }

      header_size = 8;
   }

   a = get_array_entry(arrays, GL_TEXTURE_COORD_ARRAY, index);
   assert(a != NULL);
   COMMON_ARRAY_DATA_INIT(a, pointer, type, stride, size, GL_FALSE,
                          header_size, opcode);

   if (a->enabled) {
      arrays->array_info_cache_valid = GL_FALSE;
   }
}


void
__indirect_glSecondaryColorPointer(GLint size, GLenum type, GLsizei stride,
                                      const GLvoid * pointer)
{
   uint16_t opcode;
   struct glx_context *gc = __glXGetCurrentContext();
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;
   struct array_state *a;


   if (size != 3 || stride < 0) {
      __glXSetError(gc, GL_INVALID_VALUE);
      return;
   }

   switch (type) {
   case GL_BYTE:
      opcode = 4126;
      break;
   case GL_UNSIGNED_BYTE:
      opcode = 4131;
      break;
   case GL_SHORT:
      opcode = 4127;
      break;
   case GL_UNSIGNED_SHORT:
      opcode = 4132;
      break;
   case GL_INT:
      opcode = 4128;
      break;
   case GL_UNSIGNED_INT:
      opcode = 4133;
      break;
   case GL_FLOAT:
      opcode = 4129;
      break;
   case GL_DOUBLE:
      opcode = 4130;
      break;
   default:
      __glXSetError(gc, GL_INVALID_ENUM);
      return;
   }

   a = get_array_entry(arrays, GL_SECONDARY_COLOR_ARRAY, 0);
   if (a == NULL) {
      __glXSetError(gc, GL_INVALID_OPERATION);
      return;
   }

   COMMON_ARRAY_DATA_INIT(a, pointer, type, stride, size, GL_TRUE, 4, opcode);

   if (a->enabled) {
      arrays->array_info_cache_valid = GL_FALSE;
   }
}


void
__indirect_glFogCoordPointer(GLenum type, GLsizei stride,
                                const GLvoid * pointer)
{
   uint16_t opcode;
   struct glx_context *gc = __glXGetCurrentContext();
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;
   struct array_state *a;


   if (stride < 0) {
      __glXSetError(gc, GL_INVALID_VALUE);
      return;
   }

   switch (type) {
   case GL_FLOAT:
      opcode = 4124;
      break;
   case GL_DOUBLE:
      opcode = 4125;
      break;
   default:
      __glXSetError(gc, GL_INVALID_ENUM);
      return;
   }

   a = get_array_entry(arrays, GL_FOG_COORD_ARRAY, 0);
   if (a == NULL) {
      __glXSetError(gc, GL_INVALID_OPERATION);
      return;
   }

   COMMON_ARRAY_DATA_INIT(a, pointer, type, stride, 1, GL_FALSE, 4, opcode);

   if (a->enabled) {
      arrays->array_info_cache_valid = GL_FALSE;
   }
}


void
__indirect_glVertexAttribPointer(GLuint index, GLint size,
                                    GLenum type, GLboolean normalized,
                                    GLsizei stride, const GLvoid * pointer)
{
   static const uint16_t short_ops[5] = {
        0, X_GLrop_VertexAttrib1svARB, X_GLrop_VertexAttrib2svARB,
        X_GLrop_VertexAttrib3svARB, X_GLrop_VertexAttrib4svARB
   };
   static const uint16_t float_ops[5] = {
        0, X_GLrop_VertexAttrib1fvARB, X_GLrop_VertexAttrib2fvARB,
        X_GLrop_VertexAttrib3fvARB, X_GLrop_VertexAttrib4fvARB
   };
   static const uint16_t double_ops[5] = {
        0, X_GLrop_VertexAttrib1dvARB, X_GLrop_VertexAttrib2dvARB,
        X_GLrop_VertexAttrib3dvARB, X_GLrop_VertexAttrib4dvARB
   };

   uint16_t opcode;
   struct glx_context *gc = __glXGetCurrentContext();
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);
   struct array_state_vector *arrays = state->array_state;
   struct array_state *a;
   unsigned true_immediate_count;
   unsigned true_immediate_size;


   if ((size < 1) || (size > 4) || (stride < 0)
       || (index > arrays->num_vertex_program_attribs)) {
      __glXSetError(gc, GL_INVALID_VALUE);
      return;
   }

   if (normalized && (type != GL_FLOAT) && (type != GL_DOUBLE)) {
      switch (type) {
      case GL_BYTE:
         opcode = X_GLrop_VertexAttrib4NbvARB;
         break;
      case GL_UNSIGNED_BYTE:
         opcode = X_GLrop_VertexAttrib4NubvARB;
         break;
      case GL_SHORT:
         opcode = X_GLrop_VertexAttrib4NsvARB;
         break;
      case GL_UNSIGNED_SHORT:
         opcode = X_GLrop_VertexAttrib4NusvARB;
         break;
      case GL_INT:
         opcode = X_GLrop_VertexAttrib4NivARB;
         break;
      case GL_UNSIGNED_INT:
         opcode = X_GLrop_VertexAttrib4NuivARB;
         break;
      default:
         __glXSetError(gc, GL_INVALID_ENUM);
         return;
      }

      true_immediate_count = 4;
   }
   else {
      true_immediate_count = size;

      switch (type) {
      case GL_BYTE:
         opcode = X_GLrop_VertexAttrib4bvARB;
         true_immediate_count = 4;
         break;
      case GL_UNSIGNED_BYTE:
         opcode = X_GLrop_VertexAttrib4ubvARB;
         true_immediate_count = 4;
         break;
      case GL_SHORT:
         opcode = short_ops[size];
         break;
      case GL_UNSIGNED_SHORT:
         opcode = X_GLrop_VertexAttrib4usvARB;
         true_immediate_count = 4;
         break;
      case GL_INT:
         opcode = X_GLrop_VertexAttrib4ivARB;
         true_immediate_count = 4;
         break;
      case GL_UNSIGNED_INT:
         opcode = X_GLrop_VertexAttrib4uivARB;
         true_immediate_count = 4;
         break;
      case GL_FLOAT:
         opcode = float_ops[size];
         break;
      case GL_DOUBLE:
         opcode = double_ops[size];
         break;
      default:
         __glXSetError(gc, GL_INVALID_ENUM);
         return;
      }
   }

   a = get_array_entry(arrays, GL_VERTEX_ATTRIB_ARRAY_POINTER, index);
   if (a == NULL) {
      __glXSetError(gc, GL_INVALID_OPERATION);
      return;
   }

   COMMON_ARRAY_DATA_INIT(a, pointer, type, stride, size, normalized, 8,
                          opcode);

   true_immediate_size = __glXTypeSize(type) * true_immediate_count;
   a->header[0] = __GLX_PAD(8 + true_immediate_size);

   if (a->enabled) {
      arrays->array_info_cache_valid = GL_FALSE;
   }
}


/**
 * I don't have 100% confidence that this is correct.  The different rules
 * about whether or not generic vertex attributes alias "classic" vertex
 * attributes (i.e., attrib1 ?= primary color) between ARB_vertex_program,
 * ARB_vertex_shader, and NV_vertex_program are a bit confusing.  My
 * feeling is that the client-side doesn't have to worry about it.  The
 * client just sends all the data to the server and lets the server deal
 * with it.
 */
void
__indirect_glVertexAttribPointerNV(GLuint index, GLint size,
                                   GLenum type, GLsizei stride,
                                   const GLvoid * pointer)
{
   struct glx_context *gc = __glXGetCurrentContext();
   GLboolean normalized = GL_FALSE;


   switch (type) {
   case GL_UNSIGNED_BYTE:
      if (size != 4) {
         __glXSetError(gc, GL_INVALID_VALUE);
         return;
      }
      normalized = GL_TRUE;
      FALLTHROUGH;
   case GL_SHORT:
   case GL_FLOAT:
   case GL_DOUBLE:
      __indirect_glVertexAttribPointer(index, size, type,
                                          normalized, stride, pointer);
      return;
   default:
      __glXSetError(gc, GL_INVALID_ENUM);
      return;
   }
}


void
__indirect_glClientActiveTexture(GLenum texture)
{
   struct glx_context *const gc = __glXGetCurrentContext();
   __GLXattribute *const state =
      (__GLXattribute *) (gc->client_state_private);
   struct array_state_vector *const arrays = state->array_state;
   const GLint unit = (GLint) texture - GL_TEXTURE0;


   if ((unit < 0) || (unit >= arrays->num_texture_units)) {
      __glXSetError(gc, GL_INVALID_ENUM);
      return;
   }

   arrays->active_texture_unit = unit;
}


/**
 * Modify the enable state for the selected array
 */
GLboolean
__glXSetArrayEnable(__GLXattribute * state, GLenum key, unsigned index,
                    GLboolean enable)
{
   struct array_state_vector *arrays = state->array_state;
   struct array_state *a;


   /* Texture coordinate arrays have an implicit index set when the
    * application calls glClientActiveTexture.
    */
   if (key == GL_TEXTURE_COORD_ARRAY) {
      index = arrays->active_texture_unit;
   }

   a = get_array_entry(arrays, key, index);

   if ((a != NULL) && (a->enabled != enable)) {
      a->enabled = enable;
      arrays->array_info_cache_valid = GL_FALSE;
   }

   return (a != NULL);
}


void
__glXArrayDisableAll(__GLXattribute * state)
{
   struct array_state_vector *arrays = state->array_state;
   unsigned i;


   for (i = 0; i < arrays->num_arrays; i++) {
      arrays->arrays[i].enabled = GL_FALSE;
   }

   arrays->array_info_cache_valid = GL_FALSE;
}


/**
 */
GLboolean
__glXGetArrayEnable(const __GLXattribute * const state,
                    GLenum key, unsigned index, GLintptr * dest)
{
   const struct array_state_vector *arrays = state->array_state;
   const struct array_state *a =
      get_array_entry((struct array_state_vector *) arrays,
                      key, index);

   if (a != NULL) {
      *dest = (GLintptr) a->enabled;
   }

   return (a != NULL);
}


/**
 */
GLboolean
__glXGetArrayType(const __GLXattribute * const state,
                  GLenum key, unsigned index, GLintptr * dest)
{
   const struct array_state_vector *arrays = state->array_state;
   const struct array_state *a =
      get_array_entry((struct array_state_vector *) arrays,
                      key, index);

   if (a != NULL) {
      *dest = (GLintptr) a->data_type;
   }

   return (a != NULL);
}


/**
 */
GLboolean
__glXGetArraySize(const __GLXattribute * const state,
                  GLenum key, unsigned index, GLintptr * dest)
{
   const struct array_state_vector *arrays = state->array_state;
   const struct array_state *a =
      get_array_entry((struct array_state_vector *) arrays,
                      key, index);

   if (a != NULL) {
      *dest = (GLintptr) a->count;
   }

   return (a != NULL);
}


/**
 */
GLboolean
__glXGetArrayStride(const __GLXattribute * const state,
                    GLenum key, unsigned index, GLintptr * dest)
{
   const struct array_state_vector *arrays = state->array_state;
   const struct array_state *a =
      get_array_entry((struct array_state_vector *) arrays,
                      key, index);

   if (a != NULL) {
      *dest = (GLintptr) a->user_stride;
   }

   return (a != NULL);
}


/**
 */
GLboolean
__glXGetArrayPointer(const __GLXattribute * const state,
                     GLenum key, unsigned index, void **dest)
{
   const struct array_state_vector *arrays = state->array_state;
   const struct array_state *a =
      get_array_entry((struct array_state_vector *) arrays,
                      key, index);


   if (a != NULL) {
      *dest = (void *) (a->data);
   }

   return (a != NULL);
}


/**
 */
GLboolean
__glXGetArrayNormalized(const __GLXattribute * const state,
                        GLenum key, unsigned index, GLintptr * dest)
{
   const struct array_state_vector *arrays = state->array_state;
   const struct array_state *a =
      get_array_entry((struct array_state_vector *) arrays,
                      key, index);


   if (a != NULL) {
      *dest = (GLintptr) a->normalized;
   }

   return (a != NULL);
}


/**
 */
GLuint
__glXGetActiveTextureUnit(const __GLXattribute * const state)
{
   return state->array_state->active_texture_unit;
}


void
__glXPushArrayState(__GLXattribute * state)
{
   struct array_state_vector *arrays = state->array_state;
   struct array_stack_state *stack =
      &arrays->stack[(arrays->stack_index * arrays->num_arrays)];
   unsigned i;

   /* XXX are we pushing _all_ the necessary fields? */
   for (i = 0; i < arrays->num_arrays; i++) {
      stack[i].data = arrays->arrays[i].data;
      stack[i].data_type = arrays->arrays[i].data_type;
      stack[i].user_stride = arrays->arrays[i].user_stride;
      stack[i].count = arrays->arrays[i].count;
      stack[i].key = arrays->arrays[i].key;
      stack[i].index = arrays->arrays[i].index;
      stack[i].enabled = arrays->arrays[i].enabled;
   }

   arrays->active_texture_unit_stack[arrays->stack_index] =
      arrays->active_texture_unit;

   arrays->stack_index++;
}


void
__glXPopArrayState(__GLXattribute * state)
{
   struct array_state_vector *arrays = state->array_state;
   struct array_stack_state *stack;
   unsigned i;


   arrays->stack_index--;
   stack = &arrays->stack[(arrays->stack_index * arrays->num_arrays)];

   for (i = 0; i < arrays->num_arrays; i++) {
      switch (stack[i].key) {
      case GL_NORMAL_ARRAY:
         __indirect_glNormalPointer(stack[i].data_type,
                                    stack[i].user_stride, stack[i].data);
         break;
      case GL_COLOR_ARRAY:
         __indirect_glColorPointer(stack[i].count,
                                   stack[i].data_type,
                                   stack[i].user_stride, stack[i].data);
         break;
      case GL_INDEX_ARRAY:
         __indirect_glIndexPointer(stack[i].data_type,
                                   stack[i].user_stride, stack[i].data);
         break;
      case GL_EDGE_FLAG_ARRAY:
         __indirect_glEdgeFlagPointer(stack[i].user_stride, stack[i].data);
         break;
      case GL_TEXTURE_COORD_ARRAY:
         arrays->active_texture_unit = stack[i].index;
         __indirect_glTexCoordPointer(stack[i].count,
                                      stack[i].data_type,
                                      stack[i].user_stride, stack[i].data);
         break;
      case GL_SECONDARY_COLOR_ARRAY:
         __indirect_glSecondaryColorPointer(stack[i].count,
                                               stack[i].data_type,
                                               stack[i].user_stride,
                                               stack[i].data);
         break;
      case GL_FOG_COORDINATE_ARRAY:
         __indirect_glFogCoordPointer(stack[i].data_type,
                                         stack[i].user_stride, stack[i].data);
         break;

      }

      __glXSetArrayEnable(state, stack[i].key, stack[i].index,
                          stack[i].enabled);
   }

   arrays->active_texture_unit =
      arrays->active_texture_unit_stack[arrays->stack_index];
}
