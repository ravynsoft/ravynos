/**************************************************************************
 *
 * Copyright 2011 Marek Olšák <maraeo@gmail.com>
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
 * IN NO EVENT SHALL AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * This module uploads user buffers and translates the vertex buffers which
 * contain incompatible vertices (i.e. not supported by the driver/hardware)
 * into compatible ones, based on the Gallium CAPs.
 *
 * It does not upload index buffers.
 *
 * The module heavily uses bitmasks to represent per-buffer and
 * per-vertex-element flags to avoid looping over the list of buffers just
 * to see if there's a non-zero stride, or user buffer, or unsupported format,
 * etc.
 *
 * There are 3 categories of vertex elements, which are processed separately:
 * - per-vertex attribs (stride != 0, instance_divisor == 0)
 * - instanced attribs (stride != 0, instance_divisor > 0)
 * - constant attribs (stride == 0)
 *
 * All needed uploads and translations are performed every draw command, but
 * only the subset of vertices needed for that draw command is uploaded or
 * translated. (the module never translates whole buffers)
 *
 *
 * The module consists of two main parts:
 *
 *
 * 1) Translate (u_vbuf_translate_begin/end)
 *
 * This is pretty much a vertex fetch fallback. It translates vertices from
 * one vertex buffer to another in an unused vertex buffer slot. It does
 * whatever is needed to make the vertices readable by the hardware (changes
 * vertex formats and aligns offsets and strides). The translate module is
 * used here.
 *
 * Each of the 3 categories is translated to a separate buffer.
 * Only the [min_index, max_index] range is translated. For instanced attribs,
 * the range is [start_instance, start_instance+instance_count]. For constant
 * attribs, the range is [0, 1].
 *
 *
 * 2) User buffer uploading (u_vbuf_upload_buffers)
 *
 * Only the [min_index, max_index] range is uploaded (just like Translate)
 * with a single memcpy.
 *
 * This method works best for non-indexed draw operations or indexed draw
 * operations where the [min_index, max_index] range is not being way bigger
 * than the vertex count.
 *
 * If the range is too big (e.g. one triangle with indices {0, 1, 10000}),
 * the per-vertex attribs are uploaded via the translate module, all packed
 * into one vertex buffer, and the indexed draw call is turned into
 * a non-indexed one in the process. This adds additional complexity
 * to the translate part, but it prevents bad apps from bringing your frame
 * rate down.
 *
 *
 * If there is nothing to do, it forwards every command to the driver.
 * The module also has its own CSO cache of vertex element states.
 */

#include "util/u_vbuf.h"

#include "util/u_dump.h"
#include "util/format/u_format.h"
#include "util/u_helpers.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_prim_restart.h"
#include "util/u_screen.h"
#include "util/u_upload_mgr.h"
#include "indices/u_primconvert.h"
#include "translate/translate.h"
#include "translate/translate_cache.h"
#include "cso_cache/cso_cache.h"
#include "cso_cache/cso_hash.h"

struct u_vbuf_elements {
   unsigned count;
   struct pipe_vertex_element ve[PIPE_MAX_ATTRIBS];

   unsigned src_format_size[PIPE_MAX_ATTRIBS];

   /* If (velem[i].src_format != native_format[i]), the vertex buffer
    * referenced by the vertex element cannot be used for rendering and
    * its vertex data must be translated to native_format[i]. */
   enum pipe_format native_format[PIPE_MAX_ATTRIBS];
   unsigned native_format_size[PIPE_MAX_ATTRIBS];
   unsigned component_size[PIPE_MAX_ATTRIBS];
   /* buffer-indexed */
   unsigned strides[PIPE_MAX_ATTRIBS];

   /* Which buffers are used by the vertex element state. */
   uint32_t used_vb_mask;
   /* This might mean two things:
    * - src_format != native_format, as discussed above.
    * - src_offset % 4 != 0 (if the caps don't allow such an offset). */
   uint32_t incompatible_elem_mask; /* each bit describes a corresp. attrib  */
   /* Which buffer has at least one vertex element referencing it
    * incompatible. */
   uint32_t incompatible_vb_mask_any;
   /* Which buffer has all vertex elements referencing it incompatible. */
   uint32_t incompatible_vb_mask_all;
   /* Which buffer has at least one vertex element referencing it
    * compatible. */
   uint32_t compatible_vb_mask_any;
   uint32_t vb_align_mask[2]; //which buffers require 2/4 byte alignments
   /* Which buffer has all vertex elements referencing it compatible. */
   uint32_t compatible_vb_mask_all;

   /* Which buffer has at least one vertex element referencing it
    * non-instanced. */
   uint32_t noninstance_vb_mask_any;

   /* Which buffers are used by multiple vertex attribs. */
   uint32_t interleaved_vb_mask;

   /* Which buffer has a non-zero stride. */
   uint32_t nonzero_stride_vb_mask; /* each bit describes a corresp. buffer */

   /* Which buffer is incompatible (unaligned). */
   uint32_t incompatible_vb_mask; /* each bit describes a corresp. buffer */

   void *driver_cso;
};

enum {
   VB_VERTEX = 0,
   VB_INSTANCE = 1,
   VB_CONST = 2,
   VB_NUM = 3
};

struct u_vbuf {
   struct u_vbuf_caps caps;
   bool has_signed_vb_offset;

   struct pipe_context *pipe;
   struct translate_cache *translate_cache;
   struct cso_cache cso_cache;

   struct primconvert_context *pc;
   bool flatshade_first;

   /* This is what was set in set_vertex_buffers.
    * May contain user buffers. */
   struct pipe_vertex_buffer vertex_buffer[PIPE_MAX_ATTRIBS];
   uint32_t enabled_vb_mask;

   uint32_t unaligned_vb_mask[2]; //16/32bit

   /* Vertex buffers for the driver.
    * There are usually no user buffers. */
   struct pipe_vertex_buffer real_vertex_buffer[PIPE_MAX_ATTRIBS];
   uint32_t dirty_real_vb_mask; /* which buffers are dirty since the last
                                   call of set_vertex_buffers */

   /* Vertex elements. */
   struct u_vbuf_elements *ve, *ve_saved;

   /* Vertex elements used for the translate fallback. */
   struct cso_velems_state fallback_velems;
   /* If non-NULL, this is a vertex element state used for the translate
    * fallback and therefore used for rendering too. */
   bool using_translate;
   /* The vertex buffer slot index where translated vertices have been
    * stored in. */
   unsigned fallback_vbs[VB_NUM];
   unsigned fallback_vbs_mask;

   /* Which buffer is a user buffer. */
   uint32_t user_vb_mask; /* each bit describes a corresp. buffer */
   /* Which buffer is incompatible (unaligned). */
   uint32_t incompatible_vb_mask; /* each bit describes a corresp. buffer */
   /* Which buffers are allowed (supported by hardware). */
   uint32_t allowed_vb_mask;
};

static void *
u_vbuf_create_vertex_elements(struct u_vbuf *mgr, unsigned count,
                              const struct pipe_vertex_element *attribs);
static void u_vbuf_delete_vertex_elements(void *ctx, void *state,
                                          enum cso_cache_type type);

static const struct {
   enum pipe_format from, to;
} vbuf_format_fallbacks[] = {
   { PIPE_FORMAT_R32_FIXED,            PIPE_FORMAT_R32_FLOAT },
   { PIPE_FORMAT_R32G32_FIXED,         PIPE_FORMAT_R32G32_FLOAT },
   { PIPE_FORMAT_R32G32B32_FIXED,      PIPE_FORMAT_R32G32B32_FLOAT },
   { PIPE_FORMAT_R32G32B32A32_FIXED,   PIPE_FORMAT_R32G32B32A32_FLOAT },
   { PIPE_FORMAT_R16_FLOAT,            PIPE_FORMAT_R32_FLOAT },
   { PIPE_FORMAT_R16G16_FLOAT,         PIPE_FORMAT_R32G32_FLOAT },
   { PIPE_FORMAT_R16G16B16_FLOAT,      PIPE_FORMAT_R32G32B32_FLOAT },
   { PIPE_FORMAT_R16G16B16A16_FLOAT,   PIPE_FORMAT_R32G32B32A32_FLOAT },
   { PIPE_FORMAT_R64_FLOAT,            PIPE_FORMAT_R32_FLOAT },
   { PIPE_FORMAT_R64G64_FLOAT,         PIPE_FORMAT_R32G32_FLOAT },
   { PIPE_FORMAT_R64G64B64_FLOAT,      PIPE_FORMAT_R32G32B32_FLOAT },
   { PIPE_FORMAT_R64G64B64A64_FLOAT,   PIPE_FORMAT_R32G32B32A32_FLOAT },
   { PIPE_FORMAT_R32_UNORM,            PIPE_FORMAT_R32_FLOAT },
   { PIPE_FORMAT_R32G32_UNORM,         PIPE_FORMAT_R32G32_FLOAT },
   { PIPE_FORMAT_R32G32B32_UNORM,      PIPE_FORMAT_R32G32B32_FLOAT },
   { PIPE_FORMAT_R32G32B32A32_UNORM,   PIPE_FORMAT_R32G32B32A32_FLOAT },
   { PIPE_FORMAT_R32_SNORM,            PIPE_FORMAT_R32_FLOAT },
   { PIPE_FORMAT_R32G32_SNORM,         PIPE_FORMAT_R32G32_FLOAT },
   { PIPE_FORMAT_R32G32B32_SNORM,      PIPE_FORMAT_R32G32B32_FLOAT },
   { PIPE_FORMAT_R32G32B32A32_SNORM,   PIPE_FORMAT_R32G32B32A32_FLOAT },
   { PIPE_FORMAT_R32_USCALED,          PIPE_FORMAT_R32_FLOAT },
   { PIPE_FORMAT_R32G32_USCALED,       PIPE_FORMAT_R32G32_FLOAT },
   { PIPE_FORMAT_R32G32B32_USCALED,    PIPE_FORMAT_R32G32B32_FLOAT },
   { PIPE_FORMAT_R32G32B32A32_USCALED, PIPE_FORMAT_R32G32B32A32_FLOAT },
   { PIPE_FORMAT_R32_SSCALED,          PIPE_FORMAT_R32_FLOAT },
   { PIPE_FORMAT_R32G32_SSCALED,       PIPE_FORMAT_R32G32_FLOAT },
   { PIPE_FORMAT_R32G32B32_SSCALED,    PIPE_FORMAT_R32G32B32_FLOAT },
   { PIPE_FORMAT_R32G32B32A32_SSCALED, PIPE_FORMAT_R32G32B32A32_FLOAT },
   { PIPE_FORMAT_R16_UNORM,            PIPE_FORMAT_R32_FLOAT },
   { PIPE_FORMAT_R16G16_UNORM,         PIPE_FORMAT_R32G32_FLOAT },
   { PIPE_FORMAT_R16G16B16_UNORM,      PIPE_FORMAT_R32G32B32_FLOAT },
   { PIPE_FORMAT_R16G16B16A16_UNORM,   PIPE_FORMAT_R32G32B32A32_FLOAT },
   { PIPE_FORMAT_R16_SNORM,            PIPE_FORMAT_R32_FLOAT },
   { PIPE_FORMAT_R16G16_SNORM,         PIPE_FORMAT_R32G32_FLOAT },
   { PIPE_FORMAT_R16G16B16_SNORM,      PIPE_FORMAT_R32G32B32_FLOAT },
   { PIPE_FORMAT_R16G16B16_SINT,       PIPE_FORMAT_R32G32B32_SINT },
   { PIPE_FORMAT_R16G16B16_UINT,       PIPE_FORMAT_R32G32B32_UINT },
   { PIPE_FORMAT_R16G16B16A16_SNORM,   PIPE_FORMAT_R32G32B32A32_FLOAT },
   { PIPE_FORMAT_R16_USCALED,          PIPE_FORMAT_R32_FLOAT },
   { PIPE_FORMAT_R16G16_USCALED,       PIPE_FORMAT_R32G32_FLOAT },
   { PIPE_FORMAT_R16G16B16_USCALED,    PIPE_FORMAT_R32G32B32_FLOAT },
   { PIPE_FORMAT_R16G16B16A16_USCALED, PIPE_FORMAT_R32G32B32A32_FLOAT },
   { PIPE_FORMAT_R16_SSCALED,          PIPE_FORMAT_R32_FLOAT },
   { PIPE_FORMAT_R16G16_SSCALED,       PIPE_FORMAT_R32G32_FLOAT },
   { PIPE_FORMAT_R16G16B16_SSCALED,    PIPE_FORMAT_R32G32B32_FLOAT },
   { PIPE_FORMAT_R16G16B16A16_SSCALED, PIPE_FORMAT_R32G32B32A32_FLOAT },
   { PIPE_FORMAT_R8_UNORM,             PIPE_FORMAT_R32_FLOAT },
   { PIPE_FORMAT_R8G8_UNORM,           PIPE_FORMAT_R32G32_FLOAT },
   { PIPE_FORMAT_R8G8B8_UNORM,         PIPE_FORMAT_R32G32B32_FLOAT },
   { PIPE_FORMAT_R8G8B8A8_UNORM,       PIPE_FORMAT_R32G32B32A32_FLOAT },
   { PIPE_FORMAT_R8_SNORM,             PIPE_FORMAT_R32_FLOAT },
   { PIPE_FORMAT_R8G8_SNORM,           PIPE_FORMAT_R32G32_FLOAT },
   { PIPE_FORMAT_R8G8B8_SNORM,         PIPE_FORMAT_R32G32B32_FLOAT },
   { PIPE_FORMAT_R8G8B8A8_SNORM,       PIPE_FORMAT_R32G32B32A32_FLOAT },
   { PIPE_FORMAT_R8_USCALED,           PIPE_FORMAT_R32_FLOAT },
   { PIPE_FORMAT_R8G8_USCALED,         PIPE_FORMAT_R32G32_FLOAT },
   { PIPE_FORMAT_R8G8B8_USCALED,       PIPE_FORMAT_R32G32B32_FLOAT },
   { PIPE_FORMAT_R8G8B8A8_USCALED,     PIPE_FORMAT_R32G32B32A32_FLOAT },
   { PIPE_FORMAT_R8_SSCALED,           PIPE_FORMAT_R32_FLOAT },
   { PIPE_FORMAT_R8G8_SSCALED,         PIPE_FORMAT_R32G32_FLOAT },
   { PIPE_FORMAT_R8G8B8_SSCALED,       PIPE_FORMAT_R32G32B32_FLOAT },
   { PIPE_FORMAT_R8G8B8A8_SSCALED,     PIPE_FORMAT_R32G32B32A32_FLOAT },
};

void u_vbuf_get_caps(struct pipe_screen *screen, struct u_vbuf_caps *caps,
                     bool needs64b)
{
   unsigned i;

   memset(caps, 0, sizeof(*caps));

   /* I'd rather have a bitfield of which formats are supported and a static
    * table of the translations indexed by format, but since we don't have C99
    * we can't easily make a sparsely-populated table indexed by format.  So,
    * we construct the sparse table here.
    */
   for (i = 0; i < PIPE_FORMAT_COUNT; i++)
      caps->format_translation[i] = i;

   for (i = 0; i < ARRAY_SIZE(vbuf_format_fallbacks); i++) {
      enum pipe_format format = vbuf_format_fallbacks[i].from;
      unsigned comp_bits = util_format_get_component_bits(format, 0, 0);

      if ((comp_bits > 32) && !needs64b)
         continue;

      if (!screen->is_format_supported(screen, format, PIPE_BUFFER, 0, 0,
                                       PIPE_BIND_VERTEX_BUFFER)) {
         caps->format_translation[format] = vbuf_format_fallbacks[i].to;
         caps->fallback_always = true;
      }
   }

   caps->buffer_offset_unaligned =
      !screen->get_param(screen,
                         PIPE_CAP_VERTEX_BUFFER_OFFSET_4BYTE_ALIGNED_ONLY);
   caps->buffer_stride_unaligned =
     !screen->get_param(screen,
                        PIPE_CAP_VERTEX_BUFFER_STRIDE_4BYTE_ALIGNED_ONLY);
   caps->velem_src_offset_unaligned =
      !screen->get_param(screen,
                         PIPE_CAP_VERTEX_ELEMENT_SRC_OFFSET_4BYTE_ALIGNED_ONLY);
   caps->attrib_component_unaligned =
      !screen->get_param(screen,
                         PIPE_CAP_VERTEX_ATTRIB_ELEMENT_ALIGNED_ONLY);
   assert(caps->attrib_component_unaligned ||
          (caps->velem_src_offset_unaligned && caps->buffer_stride_unaligned && caps->buffer_offset_unaligned));
   caps->user_vertex_buffers =
      screen->get_param(screen, PIPE_CAP_USER_VERTEX_BUFFERS);
   caps->max_vertex_buffers =
      screen->get_param(screen, PIPE_CAP_MAX_VERTEX_BUFFERS);

   if (screen->get_param(screen, PIPE_CAP_PRIMITIVE_RESTART) ||
       screen->get_param(screen, PIPE_CAP_PRIMITIVE_RESTART_FIXED_INDEX)) {
      caps->rewrite_restart_index = screen->get_param(screen, PIPE_CAP_EMULATE_NONFIXED_PRIMITIVE_RESTART);
      caps->supported_restart_modes = screen->get_param(screen, PIPE_CAP_SUPPORTED_PRIM_MODES_WITH_RESTART);
      caps->supported_restart_modes |= BITFIELD_BIT(MESA_PRIM_PATCHES);
      if (caps->supported_restart_modes != BITFIELD_MASK(MESA_PRIM_COUNT))
         caps->fallback_always = true;
      caps->fallback_always |= caps->rewrite_restart_index;
   }
   caps->supported_prim_modes = screen->get_param(screen, PIPE_CAP_SUPPORTED_PRIM_MODES);
   if (caps->supported_prim_modes != BITFIELD_MASK(MESA_PRIM_COUNT))
      caps->fallback_always = true;

   if (!screen->is_format_supported(screen, PIPE_FORMAT_R8_UINT, PIPE_BUFFER, 0, 0, PIPE_BIND_INDEX_BUFFER))
      caps->fallback_always = caps->rewrite_ubyte_ibs = true;

   /* OpenGL 2.0 requires a minimum of 16 vertex buffers */
   if (caps->max_vertex_buffers < 16)
      caps->fallback_always = true;

   if (!caps->buffer_offset_unaligned ||
       !caps->buffer_stride_unaligned ||
       !caps->attrib_component_unaligned ||
       !caps->velem_src_offset_unaligned)
      caps->fallback_always = true;

   if (!caps->fallback_always && !caps->user_vertex_buffers)
      caps->fallback_only_for_user_vbuffers = true;
}

struct u_vbuf *
u_vbuf_create(struct pipe_context *pipe, struct u_vbuf_caps *caps)
{
   struct u_vbuf *mgr = CALLOC_STRUCT(u_vbuf);

   mgr->caps = *caps;
   mgr->pipe = pipe;
   if (caps->rewrite_ubyte_ibs || caps->rewrite_restart_index ||
       /* require all but patches */
       ((caps->supported_prim_modes & caps->supported_restart_modes & BITFIELD_MASK(MESA_PRIM_COUNT))) !=
                                      BITFIELD_MASK(MESA_PRIM_COUNT)) {
      struct primconvert_config cfg;
      cfg.fixed_prim_restart = caps->rewrite_restart_index;
      cfg.primtypes_mask = caps->supported_prim_modes;
      cfg.restart_primtypes_mask = caps->supported_restart_modes;
      mgr->pc = util_primconvert_create_config(pipe, &cfg);
   }
   mgr->translate_cache = translate_cache_create();
   memset(mgr->fallback_vbs, ~0, sizeof(mgr->fallback_vbs));
   mgr->allowed_vb_mask = u_bit_consecutive(0, mgr->caps.max_vertex_buffers);

   mgr->has_signed_vb_offset =
      pipe->screen->get_param(pipe->screen,
                              PIPE_CAP_SIGNED_VERTEX_BUFFER_OFFSET);

   cso_cache_init(&mgr->cso_cache, pipe);
   cso_cache_set_delete_cso_callback(&mgr->cso_cache,
                                     u_vbuf_delete_vertex_elements, pipe);

   return mgr;
}

/* u_vbuf uses its own caching for vertex elements, because it needs to keep
 * its own preprocessed state per vertex element CSO. */
static struct u_vbuf_elements *
u_vbuf_set_vertex_elements_internal(struct u_vbuf *mgr,
                                    const struct cso_velems_state *velems)
{
   struct pipe_context *pipe = mgr->pipe;
   unsigned key_size, hash_key;
   struct cso_hash_iter iter;
   struct u_vbuf_elements *ve;

   /* need to include the count into the stored state data too. */
   key_size = sizeof(struct pipe_vertex_element) * velems->count +
              sizeof(unsigned);
   hash_key = cso_construct_key(velems, key_size);
   iter = cso_find_state_template(&mgr->cso_cache, hash_key, CSO_VELEMENTS,
                                  velems, key_size);

   if (cso_hash_iter_is_null(iter)) {
      struct cso_velements *cso = MALLOC_STRUCT(cso_velements);
      memcpy(&cso->state, velems, key_size);
      cso->data = u_vbuf_create_vertex_elements(mgr, velems->count,
                                                velems->velems);

      iter = cso_insert_state(&mgr->cso_cache, hash_key, CSO_VELEMENTS, cso);
      ve = cso->data;
   } else {
      ve = ((struct cso_velements *)cso_hash_iter_data(iter))->data;
   }

   assert(ve);

   if (ve != mgr->ve)
      pipe->bind_vertex_elements_state(pipe, ve->driver_cso);

   return ve;
}

void u_vbuf_set_vertex_elements(struct u_vbuf *mgr,
                                const struct cso_velems_state *velems)
{
   mgr->ve = u_vbuf_set_vertex_elements_internal(mgr, velems);
}

void u_vbuf_set_flatshade_first(struct u_vbuf *mgr, bool flatshade_first)
{
   mgr->flatshade_first = flatshade_first;
}

void u_vbuf_unset_vertex_elements(struct u_vbuf *mgr)
{
   mgr->ve = NULL;
}

void u_vbuf_destroy(struct u_vbuf *mgr)
{
   struct pipe_screen *screen = mgr->pipe->screen;
   unsigned i;
   const unsigned num_vb = screen->get_shader_param(screen, PIPE_SHADER_VERTEX,
                                                    PIPE_SHADER_CAP_MAX_INPUTS);

   mgr->pipe->set_vertex_buffers(mgr->pipe, 0, num_vb, false, NULL);

   for (i = 0; i < PIPE_MAX_ATTRIBS; i++)
      pipe_vertex_buffer_unreference(&mgr->vertex_buffer[i]);
   for (i = 0; i < PIPE_MAX_ATTRIBS; i++)
      pipe_vertex_buffer_unreference(&mgr->real_vertex_buffer[i]);

   if (mgr->pc)
      util_primconvert_destroy(mgr->pc);

   translate_cache_destroy(mgr->translate_cache);
   cso_cache_delete(&mgr->cso_cache);
   FREE(mgr);
}

static enum pipe_error
u_vbuf_translate_buffers(struct u_vbuf *mgr, struct translate_key *key,
                         const struct pipe_draw_info *info,
                         const struct pipe_draw_start_count_bias *draw,
                         unsigned vb_mask, unsigned out_vb,
                         int start_vertex, unsigned num_vertices,
                         int min_index, bool unroll_indices)
{
   struct translate *tr;
   struct pipe_transfer *vb_transfer[PIPE_MAX_ATTRIBS] = {0};
   struct pipe_resource *out_buffer = NULL;
   uint8_t *out_map;
   unsigned out_offset, mask;

   /* Get a translate object. */
   tr = translate_cache_find(mgr->translate_cache, key);

   /* Map buffers we want to translate. */
   mask = vb_mask;
   while (mask) {
      struct pipe_vertex_buffer *vb;
      unsigned offset;
      uint8_t *map;
      unsigned i = u_bit_scan(&mask);
      unsigned stride = mgr->ve->strides[i];

      vb = &mgr->vertex_buffer[i];
      offset = vb->buffer_offset + stride * start_vertex;

      if (vb->is_user_buffer) {
         map = (uint8_t*)vb->buffer.user + offset;
      } else {
         unsigned size = stride ? num_vertices * stride
                                    : sizeof(double)*4;

         if (!vb->buffer.resource) {
            static uint64_t dummy_buf[4] = { 0 };
            tr->set_buffer(tr, i, dummy_buf, 0, 0);
            continue;
         }

         if (stride) {
            /* the stride cannot be used to calculate the map size of the buffer,
             * as it only determines the bytes between elements, not the size of elements
             * themselves, meaning that if stride < element_size, the mapped size will
             * be too small and conversion will overrun the map buffer
             *
             * instead, add the size of the largest possible attribute to the final attribute's offset
             * in order to ensure the map is large enough
             */
            unsigned last_offset = size - stride;
            size = MAX2(size, last_offset + sizeof(double)*4);
         }

         if (offset + size > vb->buffer.resource->width0) {
            /* Don't try to map past end of buffer.  This often happens when
             * we're translating an attribute that's at offset > 0 from the
             * start of the vertex.  If we'd subtract attrib's offset from
             * the size, this probably wouldn't happen.
             */
            size = vb->buffer.resource->width0 - offset;

            /* Also adjust num_vertices.  A common user error is to call
             * glDrawRangeElements() with incorrect 'end' argument.  The 'end
             * value should be the max index value, but people often
             * accidentally add one to this value.  This adjustment avoids
             * crashing (by reading past the end of a hardware buffer mapping)
             * when people do that.
             */
            num_vertices = (size + stride - 1) / stride;
         }

         map = pipe_buffer_map_range(mgr->pipe, vb->buffer.resource, offset, size,
                                     PIPE_MAP_READ, &vb_transfer[i]);
      }

      /* Subtract min_index so that indexing with the index buffer works. */
      if (unroll_indices) {
         map -= (ptrdiff_t)stride * min_index;
      }

      tr->set_buffer(tr, i, map, stride, info->max_index);
   }

   /* Translate. */
   if (unroll_indices) {
      struct pipe_transfer *transfer = NULL;
      const unsigned offset = draw->start * info->index_size;
      uint8_t *map;

      /* Create and map the output buffer. */
      u_upload_alloc(mgr->pipe->stream_uploader, 0,
                     key->output_stride * draw->count, 4,
                     &out_offset, &out_buffer,
                     (void**)&out_map);
      if (!out_buffer)
         return PIPE_ERROR_OUT_OF_MEMORY;

      if (info->has_user_indices) {
         map = (uint8_t*)info->index.user + offset;
      } else {
         map = pipe_buffer_map_range(mgr->pipe, info->index.resource, offset,
                                     draw->count * info->index_size,
                                     PIPE_MAP_READ, &transfer);
      }

      switch (info->index_size) {
      case 4:
         tr->run_elts(tr, (unsigned*)map, draw->count, 0, 0, out_map);
         break;
      case 2:
         tr->run_elts16(tr, (uint16_t*)map, draw->count, 0, 0, out_map);
         break;
      case 1:
         tr->run_elts8(tr, map, draw->count, 0, 0, out_map);
         break;
      }

      if (transfer) {
         pipe_buffer_unmap(mgr->pipe, transfer);
      }
   } else {
      /* Create and map the output buffer. */
      u_upload_alloc(mgr->pipe->stream_uploader,
                     mgr->has_signed_vb_offset ?
                        0 : key->output_stride * start_vertex,
                     key->output_stride * num_vertices, 4,
                     &out_offset, &out_buffer,
                     (void**)&out_map);
      if (!out_buffer)
         return PIPE_ERROR_OUT_OF_MEMORY;

      out_offset -= key->output_stride * start_vertex;

      tr->run(tr, 0, num_vertices, 0, 0, out_map);
   }

   /* Unmap all buffers. */
   mask = vb_mask;
   while (mask) {
      unsigned i = u_bit_scan(&mask);

      if (vb_transfer[i]) {
         pipe_buffer_unmap(mgr->pipe, vb_transfer[i]);
      }
   }

   /* Setup the new vertex buffer. */
   mgr->real_vertex_buffer[out_vb].buffer_offset = out_offset;

   /* Move the buffer reference. */
   pipe_vertex_buffer_unreference(&mgr->real_vertex_buffer[out_vb]);
   mgr->real_vertex_buffer[out_vb].buffer.resource = out_buffer;
   mgr->real_vertex_buffer[out_vb].is_user_buffer = false;

   return PIPE_OK;
}

static bool
u_vbuf_translate_find_free_vb_slots(struct u_vbuf *mgr,
                                    unsigned mask[VB_NUM])
{
   unsigned type;
   unsigned fallback_vbs[VB_NUM];
   /* Set the bit for each buffer which is incompatible, or isn't set. */
   uint32_t unused_vb_mask =
      mgr->ve->incompatible_vb_mask_all | mgr->incompatible_vb_mask | mgr->ve->incompatible_vb_mask |
      ~mgr->enabled_vb_mask;
   uint32_t unused_vb_mask_orig;
   bool insufficient_buffers = false;

   /* No vertex buffers available at all */
   if (!unused_vb_mask)
      return false;

   memset(fallback_vbs, ~0, sizeof(fallback_vbs));
   mgr->fallback_vbs_mask = 0;

   /* Find free slots for each type if needed. */
   unused_vb_mask_orig = unused_vb_mask;
   for (type = 0; type < VB_NUM; type++) {
      if (mask[type]) {
         uint32_t index;

         if (!unused_vb_mask) {
            insufficient_buffers = true;
            break;
         }

         index = ffs(unused_vb_mask) - 1;
         fallback_vbs[type] = index;
         mgr->fallback_vbs_mask |= 1 << index;
         unused_vb_mask &= ~(1 << index);
         /*printf("found slot=%i for type=%i\n", index, type);*/
      }
   }

   if (insufficient_buffers) {
      /* not enough vbs for all types supported by the hardware, they will have to share one
       * buffer */
      uint32_t index = ffs(unused_vb_mask_orig) - 1;
      /* When sharing one vertex buffer use per-vertex frequency for everything. */
      fallback_vbs[VB_VERTEX] = index;
      mgr->fallback_vbs_mask = 1 << index;
      mask[VB_VERTEX] = mask[VB_VERTEX] | mask[VB_CONST] | mask[VB_INSTANCE];
      mask[VB_CONST] = 0;
      mask[VB_INSTANCE] = 0;
   }

   for (type = 0; type < VB_NUM; type++) {
      if (mask[type]) {
         mgr->dirty_real_vb_mask |= 1 << fallback_vbs[type];
      }
   }

   memcpy(mgr->fallback_vbs, fallback_vbs, sizeof(fallback_vbs));
   return true;
}

static bool
u_vbuf_translate_begin(struct u_vbuf *mgr,
                       const struct pipe_draw_info *info,
                       const struct pipe_draw_start_count_bias *draw,
                       int start_vertex, unsigned num_vertices,
                       int min_index, bool unroll_indices,
                       uint32_t misaligned)
{
   unsigned mask[VB_NUM] = {0};
   struct translate_key key[VB_NUM];
   unsigned elem_index[VB_NUM][PIPE_MAX_ATTRIBS]; /* ... into key.elements */
   unsigned i, type;
   const unsigned incompatible_vb_mask = (misaligned | mgr->incompatible_vb_mask | mgr->ve->incompatible_vb_mask) &
                                         mgr->ve->used_vb_mask;

   const int start[VB_NUM] = {
      start_vertex,           /* VERTEX */
      info->start_instance,   /* INSTANCE */
      0                       /* CONST */
   };

   const unsigned num[VB_NUM] = {
      num_vertices,           /* VERTEX */
      info->instance_count,   /* INSTANCE */
      1                       /* CONST */
   };

   memset(key, 0, sizeof(key));
   memset(elem_index, ~0, sizeof(elem_index));

   /* See if there are vertex attribs of each type to translate and
    * which ones. */
   for (i = 0; i < mgr->ve->count; i++) {
      unsigned vb_index = mgr->ve->ve[i].vertex_buffer_index;

      if (!mgr->ve->ve[i].src_stride) {
         if (!(mgr->ve->incompatible_elem_mask & (1 << i)) &&
             !(incompatible_vb_mask & (1 << vb_index))) {
            continue;
         }
         mask[VB_CONST] |= 1 << vb_index;
      } else if (mgr->ve->ve[i].instance_divisor) {
         if (!(mgr->ve->incompatible_elem_mask & (1 << i)) &&
             !(incompatible_vb_mask & (1 << vb_index))) {
            continue;
         }
         mask[VB_INSTANCE] |= 1 << vb_index;
      } else {
         if (!unroll_indices &&
             !(mgr->ve->incompatible_elem_mask & (1 << i)) &&
             !(incompatible_vb_mask & (1 << vb_index))) {
            continue;
         }
         mask[VB_VERTEX] |= 1 << vb_index;
      }
   }

   assert(mask[VB_VERTEX] || mask[VB_INSTANCE] || mask[VB_CONST]);

   /* Find free vertex buffer slots. */
   if (!u_vbuf_translate_find_free_vb_slots(mgr, mask)) {
      return false;
   }

   unsigned min_alignment[VB_NUM] = {0};
   /* Initialize the translate keys. */
   for (i = 0; i < mgr->ve->count; i++) {
      struct translate_key *k;
      struct translate_element *te;
      enum pipe_format output_format = mgr->ve->native_format[i];
      unsigned bit, vb_index = mgr->ve->ve[i].vertex_buffer_index;
      bit = 1 << vb_index;

      if (!(mgr->ve->incompatible_elem_mask & (1 << i)) &&
          !(incompatible_vb_mask & (1 << vb_index)) &&
          (!unroll_indices || !(mask[VB_VERTEX] & bit))) {
         continue;
      }

      /* Set type to what we will translate.
       * Whether vertex, instance, or constant attribs. */
      for (type = 0; type < VB_NUM; type++) {
         if (mask[type] & bit) {
            break;
         }
      }
      assert(type < VB_NUM);
      if (mgr->ve->ve[i].src_format != output_format)
         assert(translate_is_output_format_supported(output_format));
      /*printf("velem=%i type=%i\n", i, type);*/

      /* Add the vertex element. */
      k = &key[type];
      elem_index[type][i] = k->nr_elements;

      te = &k->element[k->nr_elements];
      te->type = TRANSLATE_ELEMENT_NORMAL;
      te->instance_divisor = 0;
      te->input_buffer = vb_index;
      te->input_format = mgr->ve->ve[i].src_format;
      te->input_offset = mgr->ve->ve[i].src_offset;
      te->output_format = output_format;
      te->output_offset = k->output_stride;
      unsigned adjustment = 0;
      if (!mgr->caps.attrib_component_unaligned &&
          te->output_offset % mgr->ve->component_size[i] != 0) {
         unsigned aligned = align(te->output_offset, mgr->ve->component_size[i]);
         adjustment = aligned - te->output_offset;
         te->output_offset = aligned;
      }

      k->output_stride += mgr->ve->native_format_size[i] + adjustment;
      k->nr_elements++;
      min_alignment[type] = MAX2(min_alignment[type], mgr->ve->component_size[i]);
   }

   /* Translate buffers. */
   for (type = 0; type < VB_NUM; type++) {
      if (key[type].nr_elements) {
         enum pipe_error err;
         if (!mgr->caps.attrib_component_unaligned)
            key[type].output_stride = align(key[type].output_stride, min_alignment[type]);
         err = u_vbuf_translate_buffers(mgr, &key[type], info, draw,
                                        mask[type], mgr->fallback_vbs[type],
                                        start[type], num[type], min_index,
                                        unroll_indices && type == VB_VERTEX);
         if (err != PIPE_OK)
            return false;
      }
   }

   /* Setup new vertex elements. */
   for (i = 0; i < mgr->ve->count; i++) {
      for (type = 0; type < VB_NUM; type++) {
         if (elem_index[type][i] < key[type].nr_elements) {
            struct translate_element *te = &key[type].element[elem_index[type][i]];
            mgr->fallback_velems.velems[i].instance_divisor = mgr->ve->ve[i].instance_divisor;
            mgr->fallback_velems.velems[i].src_format = te->output_format;
            mgr->fallback_velems.velems[i].src_offset = te->output_offset;
            mgr->fallback_velems.velems[i].vertex_buffer_index = mgr->fallback_vbs[type];

            /* Fixup the stride for constant attribs. */
            if (type == VB_CONST)
               mgr->fallback_velems.velems[i].src_stride = 0;
            else
               mgr->fallback_velems.velems[i].src_stride = key[type].output_stride;

            /* elem_index[type][i] can only be set for one type. */
            assert(type > VB_INSTANCE || elem_index[type+1][i] == ~0u);
            assert(type > VB_VERTEX   || elem_index[type+2][i] == ~0u);
            break;
         }
      }
      /* No translating, just copy the original vertex element over. */
      if (type == VB_NUM) {
         memcpy(&mgr->fallback_velems.velems[i], &mgr->ve->ve[i],
                sizeof(struct pipe_vertex_element));
      }
   }

   mgr->fallback_velems.count = mgr->ve->count;

   u_vbuf_set_vertex_elements_internal(mgr, &mgr->fallback_velems);
   mgr->using_translate = true;
   return true;
}

static void u_vbuf_translate_end(struct u_vbuf *mgr)
{
   unsigned i;

   /* Restore vertex elements. */
   mgr->pipe->bind_vertex_elements_state(mgr->pipe, mgr->ve->driver_cso);
   mgr->using_translate = false;

   /* Unreference the now-unused VBOs. */
   for (i = 0; i < VB_NUM; i++) {
      unsigned vb = mgr->fallback_vbs[i];
      if (vb != ~0u) {
         pipe_resource_reference(&mgr->real_vertex_buffer[vb].buffer.resource, NULL);
         mgr->fallback_vbs[i] = ~0;
      }
   }
   /* This will cause the buffer to be unbound in the driver later. */
   mgr->dirty_real_vb_mask |= mgr->fallback_vbs_mask;
   mgr->fallback_vbs_mask = 0;
}

static void *
u_vbuf_create_vertex_elements(struct u_vbuf *mgr, unsigned count,
                              const struct pipe_vertex_element *attribs)
{
   struct pipe_vertex_element tmp[PIPE_MAX_ATTRIBS];
   util_lower_uint64_vertex_elements(&attribs, &count, tmp);

   struct pipe_context *pipe = mgr->pipe;
   unsigned i;
   struct pipe_vertex_element driver_attribs[PIPE_MAX_ATTRIBS];
   struct u_vbuf_elements *ve = CALLOC_STRUCT(u_vbuf_elements);
   uint32_t used_buffers = 0;

   ve->count = count;

   memcpy(ve->ve, attribs, sizeof(struct pipe_vertex_element) * count);
   memcpy(driver_attribs, attribs, sizeof(struct pipe_vertex_element) * count);

   /* Set the best native format in case the original format is not
    * supported. */
   for (i = 0; i < count; i++) {
      enum pipe_format format = ve->ve[i].src_format;
      unsigned vb_index_bit = 1 << ve->ve[i].vertex_buffer_index;

      ve->src_format_size[i] = util_format_get_blocksize(format);

      if (used_buffers & vb_index_bit)
         ve->interleaved_vb_mask |= vb_index_bit;

      used_buffers |= vb_index_bit;

      if (!ve->ve[i].instance_divisor) {
         ve->noninstance_vb_mask_any |= vb_index_bit;
      }

      format = mgr->caps.format_translation[format];

      driver_attribs[i].src_format = format;
      ve->native_format[i] = format;
      ve->native_format_size[i] =
            util_format_get_blocksize(ve->native_format[i]);

      const struct util_format_description *desc = util_format_description(format);
      bool is_packed = false;
      for (unsigned c = 0; c < desc->nr_channels; c++)
         is_packed |= desc->channel[c].size != desc->channel[0].size || desc->channel[c].size % 8 != 0;
      unsigned component_size = is_packed ?
                                ve->native_format_size[i] : (ve->native_format_size[i] / desc->nr_channels);
      ve->component_size[i] = component_size;

      if (ve->ve[i].src_format != format ||
          (!mgr->caps.velem_src_offset_unaligned &&
           ve->ve[i].src_offset % 4 != 0) ||
          (!mgr->caps.attrib_component_unaligned &&
           ve->ve[i].src_offset % component_size != 0)) {
         ve->incompatible_elem_mask |= 1 << i;
         ve->incompatible_vb_mask_any |= vb_index_bit;
      } else {
         ve->compatible_vb_mask_any |= vb_index_bit;
         if (component_size == 2) {
            ve->vb_align_mask[0] |= vb_index_bit;
            if (ve->ve[i].src_stride % 2 != 0)
               ve->incompatible_vb_mask |= vb_index_bit;
         }
         else if (component_size == 4) {
            ve->vb_align_mask[1] |= vb_index_bit;
            if (ve->ve[i].src_stride % 4 != 0)
               ve->incompatible_vb_mask |= vb_index_bit;
         }
      }
      ve->strides[ve->ve[i].vertex_buffer_index] = ve->ve[i].src_stride;
      if (ve->ve[i].src_stride) {
         ve->nonzero_stride_vb_mask |= 1 << ve->ve[i].vertex_buffer_index;
      }
      if (!mgr->caps.buffer_stride_unaligned && ve->ve[i].src_stride % 4 != 0)
         ve->incompatible_vb_mask |= vb_index_bit;
   }

   if (used_buffers & ~mgr->allowed_vb_mask) {
      /* More vertex buffers are used than the hardware supports.  In
       * principle, we only need to make sure that less vertex buffers are
       * used, and mark some of the latter vertex buffers as incompatible.
       * For now, mark all vertex buffers as incompatible.
       */
      ve->incompatible_vb_mask_any = used_buffers;
      ve->compatible_vb_mask_any = 0;
      ve->incompatible_elem_mask = u_bit_consecutive(0, count);
   }

   ve->used_vb_mask = used_buffers;
   ve->compatible_vb_mask_all = ~ve->incompatible_vb_mask_any & used_buffers;
   ve->incompatible_vb_mask_all = ~ve->compatible_vb_mask_any & used_buffers;

   /* Align the formats and offsets to the size of DWORD if needed. */
   if (!mgr->caps.velem_src_offset_unaligned) {
      for (i = 0; i < count; i++) {
         ve->native_format_size[i] = align(ve->native_format_size[i], 4);
         driver_attribs[i].src_offset = align(ve->ve[i].src_offset, 4);
      }
   }

   /* Only create driver CSO if no incompatible elements */
   if (!ve->incompatible_elem_mask) {
      ve->driver_cso =
         pipe->create_vertex_elements_state(pipe, count, driver_attribs);
   }

   return ve;
}

static void u_vbuf_delete_vertex_elements(void *ctx, void *state,
                                          enum cso_cache_type type)
{
   struct pipe_context *pipe = (struct pipe_context*)ctx;
   struct cso_velements *cso = (struct cso_velements*)state;
   struct u_vbuf_elements *ve = (struct u_vbuf_elements*)cso->data;

   if (ve->driver_cso)
      pipe->delete_vertex_elements_state(pipe, ve->driver_cso);
   FREE(ve);
   FREE(cso);
}

void u_vbuf_set_vertex_buffers(struct u_vbuf *mgr,
                               unsigned count,
                               unsigned unbind_num_trailing_slots,
                               bool take_ownership,
                               const struct pipe_vertex_buffer *bufs)
{
   unsigned i;
   /* which buffers are enabled */
   uint32_t enabled_vb_mask = 0;
   /* which buffers are in user memory */
   uint32_t user_vb_mask = 0;
   /* which buffers are incompatible with the driver */
   uint32_t incompatible_vb_mask = 0;
   /* which buffers are unaligned to 2/4 bytes */
   uint32_t unaligned_vb_mask[2] = {0};
   uint32_t mask = ~BITFIELD64_MASK(count + unbind_num_trailing_slots);

   if (!bufs) {
      struct pipe_context *pipe = mgr->pipe;
      /* Unbind. */
      unsigned total_count = count + unbind_num_trailing_slots;
      mgr->dirty_real_vb_mask &= mask;

      /* Zero out the bits we are going to rewrite completely. */
      mgr->user_vb_mask &= mask;
      mgr->incompatible_vb_mask &= mask;
      mgr->enabled_vb_mask &= mask;
      mgr->unaligned_vb_mask[0] &= mask;
      mgr->unaligned_vb_mask[1] &= mask;

      for (i = 0; i < total_count; i++) {
         unsigned dst_index = i;

         pipe_vertex_buffer_unreference(&mgr->vertex_buffer[dst_index]);
         pipe_vertex_buffer_unreference(&mgr->real_vertex_buffer[dst_index]);
      }

      pipe->set_vertex_buffers(pipe, count, unbind_num_trailing_slots, false, NULL);
      return;
   }

   for (i = 0; i < count; i++) {
      unsigned dst_index = i;
      const struct pipe_vertex_buffer *vb = &bufs[i];
      struct pipe_vertex_buffer *orig_vb = &mgr->vertex_buffer[dst_index];
      struct pipe_vertex_buffer *real_vb = &mgr->real_vertex_buffer[dst_index];

      if (!vb->buffer.resource) {
         pipe_vertex_buffer_unreference(orig_vb);
         pipe_vertex_buffer_unreference(real_vb);
         continue;
      }

      bool not_user = !vb->is_user_buffer && vb->is_user_buffer == orig_vb->is_user_buffer;
      /* struct isn't tightly packed: do not use memcmp */
      if (not_user &&
          orig_vb->buffer_offset == vb->buffer_offset && orig_vb->buffer.resource == vb->buffer.resource) {
         mask |= BITFIELD_BIT(dst_index);
         if (take_ownership) {
             pipe_vertex_buffer_unreference(orig_vb);
             /* the pointer was unset in the line above, so copy it back */
             orig_vb->buffer.resource = vb->buffer.resource;
         }
         if (mask == UINT32_MAX)
            return;
         continue;
      }

      if (take_ownership) {
         pipe_vertex_buffer_unreference(orig_vb);
         memcpy(orig_vb, vb, sizeof(*vb));
      } else {
         pipe_vertex_buffer_reference(orig_vb, vb);
      }

      enabled_vb_mask |= 1 << dst_index;

      if ((!mgr->caps.buffer_offset_unaligned && vb->buffer_offset % 4 != 0)) {
         incompatible_vb_mask |= 1 << dst_index;
         real_vb->buffer_offset = vb->buffer_offset;
         pipe_vertex_buffer_unreference(real_vb);
         real_vb->is_user_buffer = false;
         continue;
      }

      if (!mgr->caps.attrib_component_unaligned) {
         if (vb->buffer_offset % 2 != 0)
            unaligned_vb_mask[0] |= BITFIELD_BIT(dst_index);
         if (vb->buffer_offset % 4 != 0)
            unaligned_vb_mask[1] |= BITFIELD_BIT(dst_index);
      }

      if (!mgr->caps.user_vertex_buffers && vb->is_user_buffer) {
         user_vb_mask |= 1 << dst_index;
         real_vb->buffer_offset = vb->buffer_offset;
         pipe_vertex_buffer_unreference(real_vb);
         real_vb->is_user_buffer = false;
         continue;
      }

      pipe_vertex_buffer_reference(real_vb, vb);
   }

   for (i = 0; i < unbind_num_trailing_slots; i++) {
      unsigned dst_index = count + i;

      pipe_vertex_buffer_unreference(&mgr->vertex_buffer[dst_index]);
      pipe_vertex_buffer_unreference(&mgr->real_vertex_buffer[dst_index]);
   }


   /* Zero out the bits we are going to rewrite completely. */
   mgr->user_vb_mask &= mask;
   mgr->incompatible_vb_mask &= mask;
   mgr->enabled_vb_mask &= mask;
   mgr->unaligned_vb_mask[0] &= mask;
   mgr->unaligned_vb_mask[1] &= mask;

   mgr->user_vb_mask |= user_vb_mask;
   mgr->incompatible_vb_mask |= incompatible_vb_mask;
   mgr->enabled_vb_mask |= enabled_vb_mask;
   mgr->unaligned_vb_mask[0] |= unaligned_vb_mask[0];
   mgr->unaligned_vb_mask[1] |= unaligned_vb_mask[1];

   /* All changed buffers are marked as dirty, even the NULL ones,
    * which will cause the NULL buffers to be unbound in the driver later. */
   mgr->dirty_real_vb_mask |= ~mask;
}

static ALWAYS_INLINE bool
get_upload_offset_size(struct u_vbuf *mgr,
                       const struct pipe_vertex_buffer *vb,
                       struct u_vbuf_elements *ve,
                       const struct pipe_vertex_element *velem,
                       unsigned vb_index, unsigned velem_index,
                       int start_vertex, unsigned num_vertices,
                       int start_instance, unsigned num_instances,
                       unsigned *offset, unsigned *size)
{
   /* Skip the buffers generated by translate. */
   if ((1 << vb_index) & mgr->fallback_vbs_mask || !vb->is_user_buffer)
      return false;

   unsigned instance_div = velem->instance_divisor;
   *offset = vb->buffer_offset + velem->src_offset;

   if (!velem->src_stride) {
      /* Constant attrib. */
      *size = ve->src_format_size[velem_index];
   } else if (instance_div) {
      /* Per-instance attrib. */

      /* Figure out how many instances we'll render given instance_div.  We
       * can't use the typical div_round_up() pattern because the CTS uses
       * instance_div = ~0 for a test, which overflows div_round_up()'s
       * addition.
       */
      unsigned count = num_instances / instance_div;
      if (count * instance_div != num_instances)
         count++;

      *offset += velem->src_stride * start_instance;
      *size = velem->src_stride * (count - 1) + ve->src_format_size[velem_index];
   } else {
      /* Per-vertex attrib. */
      *offset += velem->src_stride * start_vertex;
      *size = velem->src_stride * (num_vertices - 1) + ve->src_format_size[velem_index];
   }
   return true;
}


static enum pipe_error
u_vbuf_upload_buffers(struct u_vbuf *mgr,
                      int start_vertex, unsigned num_vertices,
                      int start_instance, unsigned num_instances)
{
   unsigned i;
   struct u_vbuf_elements *ve = mgr->ve;
   unsigned nr_velems = ve->count;
   const struct pipe_vertex_element *velems =
         mgr->using_translate ? mgr->fallback_velems.velems : ve->ve;

   /* Faster path when no vertex attribs are interleaved. */
   if ((ve->interleaved_vb_mask & mgr->user_vb_mask) == 0) {
      for (i = 0; i < nr_velems; i++) {
         const struct pipe_vertex_element *velem = &velems[i];
         unsigned index = velem->vertex_buffer_index;
         struct pipe_vertex_buffer *vb = &mgr->vertex_buffer[index];
         unsigned offset, size;

         if (!get_upload_offset_size(mgr, vb, ve, velem, index, i, start_vertex,
                                     num_vertices, start_instance, num_instances,
                                     &offset, &size))
            continue;

         struct pipe_vertex_buffer *real_vb = &mgr->real_vertex_buffer[index];
         const uint8_t *ptr = mgr->vertex_buffer[index].buffer.user;

         u_upload_data(mgr->pipe->stream_uploader,
                       mgr->has_signed_vb_offset ? 0 : offset,
                       size, 4, ptr + offset, &real_vb->buffer_offset,
                       &real_vb->buffer.resource);
         if (!real_vb->buffer.resource)
            return PIPE_ERROR_OUT_OF_MEMORY;

         real_vb->buffer_offset -= offset;
      }
      return PIPE_OK;
   }

   unsigned start_offset[PIPE_MAX_ATTRIBS];
   unsigned end_offset[PIPE_MAX_ATTRIBS];
   uint32_t buffer_mask = 0;

   /* Slower path supporting interleaved vertex attribs using 2 loops. */
   /* Determine how much data needs to be uploaded. */
   for (i = 0; i < nr_velems; i++) {
      const struct pipe_vertex_element *velem = &velems[i];
      unsigned index = velem->vertex_buffer_index;
      struct pipe_vertex_buffer *vb = &mgr->vertex_buffer[index];
      unsigned first, size, index_bit;

      if (!get_upload_offset_size(mgr, vb, ve, velem, index, i, start_vertex,
                                  num_vertices, start_instance, num_instances,
                                  &first, &size))
         continue;

      index_bit = 1 << index;

      /* Update offsets. */
      if (!(buffer_mask & index_bit)) {
         start_offset[index] = first;
         end_offset[index] = first + size;
      } else {
         if (first < start_offset[index])
            start_offset[index] = first;
         if (first + size > end_offset[index])
            end_offset[index] = first + size;
      }

      buffer_mask |= index_bit;
   }

   /* Upload buffers. */
   while (buffer_mask) {
      unsigned start, end;
      struct pipe_vertex_buffer *real_vb;
      const uint8_t *ptr;

      i = u_bit_scan(&buffer_mask);

      start = start_offset[i];
      end = end_offset[i];
      assert(start < end);

      real_vb = &mgr->real_vertex_buffer[i];
      ptr = mgr->vertex_buffer[i].buffer.user;

      u_upload_data(mgr->pipe->stream_uploader,
                    mgr->has_signed_vb_offset ? 0 : start,
                    end - start, 4,
                    ptr + start, &real_vb->buffer_offset, &real_vb->buffer.resource);
      if (!real_vb->buffer.resource)
         return PIPE_ERROR_OUT_OF_MEMORY;

      real_vb->buffer_offset -= start;
   }

   return PIPE_OK;
}

static bool u_vbuf_need_minmax_index(const struct u_vbuf *mgr, uint32_t misaligned)
{
   /* See if there are any per-vertex attribs which will be uploaded or
    * translated. Use bitmasks to get the info instead of looping over vertex
    * elements. */
   return (mgr->ve->used_vb_mask &
           ((mgr->user_vb_mask |
             mgr->incompatible_vb_mask | mgr->ve->incompatible_vb_mask |
             misaligned |
             mgr->ve->incompatible_vb_mask_any) &
            mgr->ve->noninstance_vb_mask_any &
            mgr->ve->nonzero_stride_vb_mask)) != 0;
}

static bool u_vbuf_mapping_vertex_buffer_blocks(const struct u_vbuf *mgr, uint32_t misaligned)
{
   /* Return true if there are hw buffers which don't need to be translated.
    *
    * We could query whether each buffer is busy, but that would
    * be way more costly than this. */
   return (mgr->ve->used_vb_mask &
           (~mgr->user_vb_mask &
            ~mgr->incompatible_vb_mask &
            ~mgr->ve->incompatible_vb_mask &
            ~misaligned &
            mgr->ve->compatible_vb_mask_all &
            mgr->ve->noninstance_vb_mask_any &
            mgr->ve->nonzero_stride_vb_mask)) != 0;
}

static void
u_vbuf_get_minmax_index_mapped(const struct pipe_draw_info *info,
                               unsigned count,
                               const void *indices, unsigned *out_min_index,
                               unsigned *out_max_index)
{
   if (!count) {
      *out_min_index = 0;
      *out_max_index = 0;
      return;
   }

   switch (info->index_size) {
   case 4: {
      const unsigned *ui_indices = (const unsigned*)indices;
      unsigned max = 0;
      unsigned min = ~0u;
      if (info->primitive_restart) {
         for (unsigned i = 0; i < count; i++) {
            if (ui_indices[i] != info->restart_index) {
               if (ui_indices[i] > max) max = ui_indices[i];
               if (ui_indices[i] < min) min = ui_indices[i];
            }
         }
      }
      else {
         for (unsigned i = 0; i < count; i++) {
            if (ui_indices[i] > max) max = ui_indices[i];
            if (ui_indices[i] < min) min = ui_indices[i];
         }
      }
      *out_min_index = min;
      *out_max_index = max;
      break;
   }
   case 2: {
      const unsigned short *us_indices = (const unsigned short*)indices;
      unsigned short max = 0;
      unsigned short min = ~((unsigned short)0);
      if (info->primitive_restart) {
         for (unsigned i = 0; i < count; i++) {
            if (us_indices[i] != info->restart_index) {
               if (us_indices[i] > max) max = us_indices[i];
               if (us_indices[i] < min) min = us_indices[i];
            }
         }
      }
      else {
         for (unsigned i = 0; i < count; i++) {
            if (us_indices[i] > max) max = us_indices[i];
            if (us_indices[i] < min) min = us_indices[i];
         }
      }
      *out_min_index = min;
      *out_max_index = max;
      break;
   }
   case 1: {
      const unsigned char *ub_indices = (const unsigned char*)indices;
      unsigned char max = 0;
      unsigned char min = ~((unsigned char)0);
      if (info->primitive_restart) {
         for (unsigned i = 0; i < count; i++) {
            if (ub_indices[i] != info->restart_index) {
               if (ub_indices[i] > max) max = ub_indices[i];
               if (ub_indices[i] < min) min = ub_indices[i];
            }
         }
      }
      else {
         for (unsigned i = 0; i < count; i++) {
            if (ub_indices[i] > max) max = ub_indices[i];
            if (ub_indices[i] < min) min = ub_indices[i];
         }
      }
      *out_min_index = min;
      *out_max_index = max;
      break;
   }
   default:
      unreachable("bad index size");
   }
}

void u_vbuf_get_minmax_index(struct pipe_context *pipe,
                             const struct pipe_draw_info *info,
                             const struct pipe_draw_start_count_bias *draw,
                             unsigned *out_min_index, unsigned *out_max_index)
{
   struct pipe_transfer *transfer = NULL;
   const void *indices;

   if (info->has_user_indices) {
      indices = (uint8_t*)info->index.user +
                draw->start * info->index_size;
   } else {
      indices = pipe_buffer_map_range(pipe, info->index.resource,
                                      draw->start * info->index_size,
                                      draw->count * info->index_size,
                                      PIPE_MAP_READ, &transfer);
   }

   u_vbuf_get_minmax_index_mapped(info, draw->count, indices,
                                  out_min_index, out_max_index);

   if (transfer) {
      pipe_buffer_unmap(pipe, transfer);
   }
}

static void u_vbuf_set_driver_vertex_buffers(struct u_vbuf *mgr)
{
   struct pipe_context *pipe = mgr->pipe;
   unsigned count = util_last_bit(mgr->dirty_real_vb_mask);

   if (mgr->dirty_real_vb_mask == mgr->enabled_vb_mask &&
       mgr->dirty_real_vb_mask == mgr->user_vb_mask) {
      /* Fast path that allows us to transfer the VBO references to the driver
       * to skip atomic reference counting there. These are freshly uploaded
       * user buffers that can be discarded after this call.
       */
      pipe->set_vertex_buffers(pipe, count, 0, true, mgr->real_vertex_buffer);

      /* We don't own the VBO references now. Set them to NULL. */
      for (unsigned i = 0; i < count; i++) {
         assert(!mgr->real_vertex_buffer[i].is_user_buffer);
         mgr->real_vertex_buffer[i].buffer.resource = NULL;
      }
   } else {
      /* Slow path where we have to keep VBO references. */
      pipe->set_vertex_buffers(pipe, count, 0, false, mgr->real_vertex_buffer);
   }
   mgr->dirty_real_vb_mask = 0;
}

static void
u_vbuf_split_indexed_multidraw(struct u_vbuf *mgr, struct pipe_draw_info *info,
                               unsigned drawid_offset,
                               unsigned *indirect_data, unsigned stride,
                               unsigned draw_count)
{
   /* Increase refcount to be able to use take_index_buffer_ownership with
    * all draws.
    */
   if (draw_count > 1 && info->take_index_buffer_ownership)
      p_atomic_add(&info->index.resource->reference.count, draw_count - 1);

   assert(info->index_size);

   for (unsigned i = 0; i < draw_count; i++) {
      struct pipe_draw_start_count_bias draw;
      unsigned offset = i * stride / 4;

      draw.count = indirect_data[offset + 0];
      info->instance_count = indirect_data[offset + 1];
      draw.start = indirect_data[offset + 2];
      draw.index_bias = indirect_data[offset + 3];
      info->start_instance = indirect_data[offset + 4];

      u_vbuf_draw_vbo(mgr->pipe, info, drawid_offset, NULL, &draw, 1);
   }
}

void u_vbuf_draw_vbo(struct pipe_context *pipe, const struct pipe_draw_info *info,
                     unsigned drawid_offset,
                     const struct pipe_draw_indirect_info *indirect,
                     const struct pipe_draw_start_count_bias *draws,
                     unsigned num_draws)
{
   struct u_vbuf *mgr = pipe->vbuf;
   int start_vertex;
   unsigned min_index;
   unsigned num_vertices;
   bool unroll_indices = false;
   const uint32_t used_vb_mask = mgr->ve->used_vb_mask;
   uint32_t user_vb_mask = mgr->user_vb_mask & used_vb_mask;
   unsigned fixed_restart_index = info->index_size ? util_prim_restart_index_from_size(info->index_size) : 0;

   uint32_t misaligned = 0;
   if (!mgr->caps.attrib_component_unaligned) {
      for (unsigned i = 0; i < ARRAY_SIZE(mgr->unaligned_vb_mask); i++) {
         misaligned |= mgr->ve->vb_align_mask[i] & mgr->unaligned_vb_mask[i];
      }
   }
   const uint32_t incompatible_vb_mask =
      (mgr->incompatible_vb_mask | mgr->ve->incompatible_vb_mask | misaligned) & used_vb_mask;

   /* Normal draw. No fallback and no user buffers. */
   if (!incompatible_vb_mask &&
       !mgr->ve->incompatible_elem_mask &&
       !user_vb_mask &&
       (info->index_size != 1 || !mgr->caps.rewrite_ubyte_ibs) &&
       (!info->primitive_restart ||
        info->restart_index == fixed_restart_index ||
        !mgr->caps.rewrite_restart_index) &&
       (!info->primitive_restart || mgr->caps.supported_restart_modes & BITFIELD_BIT(info->mode)) &&
       mgr->caps.supported_prim_modes & BITFIELD_BIT(info->mode)) {

      /* Set vertex buffers if needed. */
      if (mgr->dirty_real_vb_mask & used_vb_mask) {
         u_vbuf_set_driver_vertex_buffers(mgr);
      }

      pipe->draw_vbo(pipe, info, drawid_offset, indirect, draws, num_draws);
      return;
   }

   /* Increase refcount to be able to use take_index_buffer_ownership with
    * all draws.
    */
   if (num_draws > 1 && info->take_index_buffer_ownership)
      p_atomic_add(&info->index.resource->reference.count, num_draws - 1);

   for (unsigned d = 0; d < num_draws; d++) {
      struct pipe_draw_info new_info = *info;
      struct pipe_draw_start_count_bias new_draw = draws[d];

      /* Handle indirect (multi)draws. */
      if (indirect && indirect->buffer) {
         unsigned draw_count = 0;

         /* num_draws can only be 1 with indirect draws. */
         assert(num_draws == 1);

         /* Get the number of draws. */
         if (indirect->indirect_draw_count) {
            pipe_buffer_read(pipe, indirect->indirect_draw_count,
                             indirect->indirect_draw_count_offset,
                             4, &draw_count);
         } else {
            draw_count = indirect->draw_count;
         }

         if (!draw_count)
            goto cleanup;

         unsigned data_size = (draw_count - 1) * indirect->stride +
                              (new_info.index_size ? 20 : 16);
         unsigned *data = malloc(data_size);
         if (!data)
            goto cleanup; /* report an error? */

         /* Read the used buffer range only once, because the read can be
          * uncached.
          */
         pipe_buffer_read(pipe, indirect->buffer, indirect->offset, data_size,
                          data);

         if (info->index_size) {
            /* Indexed multidraw. */
            unsigned index_bias0 = data[3];
            bool index_bias_same = true;

            /* If we invoke the translate path, we have to split the multidraw. */
            if (incompatible_vb_mask ||
                mgr->ve->incompatible_elem_mask) {
               u_vbuf_split_indexed_multidraw(mgr, &new_info, drawid_offset, data,
                                              indirect->stride, draw_count);
               free(data);
               /* We're done (as num_draws is 1), so return early. */
               return;
            }

            /* See if index_bias is the same for all draws. */
            for (unsigned i = 1; i < draw_count; i++) {
               if (data[i * indirect->stride / 4 + 3] != index_bias0) {
                  index_bias_same = false;
                  break;
               }
            }

            /* Split the multidraw if index_bias is different. */
            if (!index_bias_same) {
               u_vbuf_split_indexed_multidraw(mgr, &new_info, drawid_offset, data,
                                              indirect->stride, draw_count);
               free(data);
               /* We're done (as num_draws is 1), so return early. */
               return;
            }

            /* If we don't need to use the translate path and index_bias is
             * the same, we can process the multidraw with the time complexity
             * equal to 1 draw call (except for the index range computation).
             * We only need to compute the index range covering all draw calls
             * of the multidraw.
             *
             * The driver will not look at these values because indirect != NULL.
             * These values determine the user buffer bounds to upload.
             */
            new_draw.index_bias = index_bias0;
            new_info.index_bounds_valid = true;
            new_info.min_index = ~0u;
            new_info.max_index = 0;
            new_info.start_instance = ~0u;
            unsigned end_instance = 0;

            struct pipe_transfer *transfer = NULL;
            const uint8_t *indices;

            if (info->has_user_indices) {
               indices = (uint8_t*)info->index.user;
            } else {
               indices = (uint8_t*)pipe_buffer_map(pipe, info->index.resource,
                                                   PIPE_MAP_READ, &transfer);
            }

            for (unsigned i = 0; i < draw_count; i++) {
               unsigned offset = i * indirect->stride / 4;
               unsigned start = data[offset + 2];
               unsigned count = data[offset + 0];
               unsigned start_instance = data[offset + 4];
               unsigned instance_count = data[offset + 1];

               if (!count || !instance_count)
                  continue;

               /* Update the ranges of instances. */
               new_info.start_instance = MIN2(new_info.start_instance,
                                              start_instance);
               end_instance = MAX2(end_instance, start_instance + instance_count);

               /* Update the index range. */
               unsigned min, max;
               u_vbuf_get_minmax_index_mapped(&new_info, count,
                                              indices +
                                              new_info.index_size * start,
                                              &min, &max);

               new_info.min_index = MIN2(new_info.min_index, min);
               new_info.max_index = MAX2(new_info.max_index, max);
            }
            free(data);

            if (transfer)
               pipe_buffer_unmap(pipe, transfer);

            /* Set the final instance count. */
            new_info.instance_count = end_instance - new_info.start_instance;

            if (new_info.start_instance == ~0u || !new_info.instance_count)
               goto cleanup;
         } else {
            /* Non-indexed multidraw.
             *
             * Keep the draw call indirect and compute minimums & maximums,
             * which will determine the user buffer bounds to upload, but
             * the driver will not look at these values because indirect != NULL.
             *
             * This efficiently processes the multidraw with the time complexity
             * equal to 1 draw call.
             */
            new_draw.start = ~0u;
            new_info.start_instance = ~0u;
            unsigned end_vertex = 0;
            unsigned end_instance = 0;

            for (unsigned i = 0; i < draw_count; i++) {
               unsigned offset = i * indirect->stride / 4;
               unsigned start = data[offset + 2];
               unsigned count = data[offset + 0];
               unsigned start_instance = data[offset + 3];
               unsigned instance_count = data[offset + 1];

               new_draw.start = MIN2(new_draw.start, start);
               new_info.start_instance = MIN2(new_info.start_instance,
                                              start_instance);

               end_vertex = MAX2(end_vertex, start + count);
               end_instance = MAX2(end_instance, start_instance + instance_count);
            }
            free(data);

            /* Set the final counts. */
            new_draw.count = end_vertex - new_draw.start;
            new_info.instance_count = end_instance - new_info.start_instance;

            if (new_draw.start == ~0u || !new_draw.count || !new_info.instance_count)
               goto cleanup;
         }
      } else {
         if ((!indirect && !new_draw.count) || !new_info.instance_count)
            goto cleanup;
      }

      if (new_info.index_size) {
         /* See if anything needs to be done for per-vertex attribs. */
         if (u_vbuf_need_minmax_index(mgr, misaligned)) {
            unsigned max_index;

            if (new_info.index_bounds_valid) {
               min_index = new_info.min_index;
               max_index = new_info.max_index;
            } else {
               u_vbuf_get_minmax_index(mgr->pipe, &new_info, &new_draw,
                                       &min_index, &max_index);
            }

            assert(min_index <= max_index);

            start_vertex = min_index + new_draw.index_bias;
            num_vertices = max_index + 1 - min_index;

            /* Primitive restart doesn't work when unrolling indices.
             * We would have to break this drawing operation into several ones. */
            /* Use some heuristic to see if unrolling indices improves
             * performance. */
            if (!indirect &&
                !new_info.primitive_restart &&
                util_is_vbo_upload_ratio_too_large(new_draw.count, num_vertices) &&
                !u_vbuf_mapping_vertex_buffer_blocks(mgr, misaligned)) {
               unroll_indices = true;
               user_vb_mask &= ~(mgr->ve->nonzero_stride_vb_mask &
                                 mgr->ve->noninstance_vb_mask_any);
            }
         } else {
            /* Nothing to do for per-vertex attribs. */
            start_vertex = 0;
            num_vertices = 0;
            min_index = 0;
         }
      } else {
         start_vertex = new_draw.start;
         num_vertices = new_draw.count;
         min_index = 0;
      }

      /* Translate vertices with non-native layouts or formats. */
      if (unroll_indices ||
          incompatible_vb_mask ||
          mgr->ve->incompatible_elem_mask) {
         if (!u_vbuf_translate_begin(mgr, &new_info, &new_draw,
                                     start_vertex, num_vertices,
                                     min_index, unroll_indices, misaligned)) {
            debug_warn_once("u_vbuf_translate_begin() failed");
            goto cleanup;
         }

         if (unroll_indices) {
            if (!new_info.has_user_indices && info->take_index_buffer_ownership)
               pipe_drop_resource_references(new_info.index.resource, 1);
            new_info.index_size = 0;
            new_draw.index_bias = 0;
            new_info.index_bounds_valid = true;
            new_info.min_index = 0;
            new_info.max_index = new_draw.count - 1;
            new_draw.start = 0;
         }

         user_vb_mask &= ~(incompatible_vb_mask |
                           mgr->ve->incompatible_vb_mask_all);
      }

      /* Upload user buffers. */
      if (user_vb_mask) {
         if (u_vbuf_upload_buffers(mgr, start_vertex, num_vertices,
                                   new_info.start_instance,
                                   new_info.instance_count) != PIPE_OK) {
            debug_warn_once("u_vbuf_upload_buffers() failed");
            goto cleanup;
         }

         mgr->dirty_real_vb_mask |= user_vb_mask;
      }

      /*
      if (unroll_indices) {
         printf("unrolling indices: start_vertex = %i, num_vertices = %i\n",
                start_vertex, num_vertices);
         util_dump_draw_info(stdout, info);
         printf("\n");
      }

      unsigned i;
      for (i = 0; i < mgr->nr_vertex_buffers; i++) {
         printf("input %i: ", i);
         util_dump_vertex_buffer(stdout, mgr->vertex_buffer+i);
         printf("\n");
      }
      for (i = 0; i < mgr->nr_real_vertex_buffers; i++) {
         printf("real %i: ", i);
         util_dump_vertex_buffer(stdout, mgr->real_vertex_buffer+i);
         printf("\n");
      }
      */

      u_upload_unmap(pipe->stream_uploader);
      if (mgr->dirty_real_vb_mask)
         u_vbuf_set_driver_vertex_buffers(mgr);

      if ((new_info.index_size == 1 && mgr->caps.rewrite_ubyte_ibs) ||
          (new_info.primitive_restart &&
           ((new_info.restart_index != fixed_restart_index && mgr->caps.rewrite_restart_index) ||
           !(mgr->caps.supported_restart_modes & BITFIELD_BIT(new_info.mode)))) ||
          !(mgr->caps.supported_prim_modes & BITFIELD_BIT(new_info.mode))) {
         util_primconvert_save_flatshade_first(mgr->pc, mgr->flatshade_first);
         util_primconvert_draw_vbo(mgr->pc, &new_info, drawid_offset, indirect, &new_draw, 1);
      } else
         pipe->draw_vbo(pipe, &new_info, drawid_offset, indirect, &new_draw, 1);
      if (info->increment_draw_id)
         drawid_offset++;
   }

   if (mgr->using_translate) {
      u_vbuf_translate_end(mgr);
   }
   return;

cleanup:
   if (info->take_index_buffer_ownership) {
      struct pipe_resource *indexbuf = info->index.resource;
      pipe_resource_reference(&indexbuf, NULL);
   }
}

void u_vbuf_save_vertex_elements(struct u_vbuf *mgr)
{
   assert(!mgr->ve_saved);
   mgr->ve_saved = mgr->ve;
}

void u_vbuf_restore_vertex_elements(struct u_vbuf *mgr)
{
   if (mgr->ve != mgr->ve_saved) {
      struct pipe_context *pipe = mgr->pipe;

      mgr->ve = mgr->ve_saved;
      pipe->bind_vertex_elements_state(pipe,
                                       mgr->ve ? mgr->ve->driver_cso : NULL);
   }
   mgr->ve_saved = NULL;
}
