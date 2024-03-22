/*
 * Copyright 2010 Christoph Bumiller
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "util/u_draw.h"
#include "util/u_inlines.h"
#include "util/u_prim.h"
#include "util/format/u_format.h"
#include "translate/translate.h"

#include "nv50/nv50_context.h"
#include "nv50/nv50_query_hw.h"
#include "nv50/nv50_resource.h"

#include "nv50/nv50_3d.xml.h"

void
nv50_vertex_state_delete(struct pipe_context *pipe,
                         void *hwcso)
{
   struct nv50_vertex_stateobj *so = hwcso;

   if (so->translate)
      so->translate->release(so->translate);
   FREE(hwcso);
}

void *
nv50_vertex_state_create(struct pipe_context *pipe,
                         unsigned num_elements,
                         const struct pipe_vertex_element *elements)
{
    struct nv50_vertex_stateobj *so;
    struct translate_key transkey;
    unsigned i;

    so = CALLOC(1, sizeof(*so) +
                num_elements * sizeof(struct nv50_vertex_element));
    if (!so)
        return NULL;
    so->num_elements = num_elements;
    so->instance_elts = 0;
    so->instance_bufs = 0;
    so->need_conversion = false;

    memset(so->vb_access_size, 0, sizeof(so->vb_access_size));

    for (i = 0; i < PIPE_MAX_ATTRIBS; ++i)
       so->min_instance_div[i] = 0xffffffff;

    transkey.nr_elements = 0;
    transkey.output_stride = 0;

    for (i = 0; i < num_elements; ++i) {
        const struct pipe_vertex_element *ve = &elements[i];
        const unsigned vbi = ve->vertex_buffer_index;
        unsigned size;
        enum pipe_format fmt = ve->src_format;

        so->element[i].pipe = elements[i];
        so->element[i].state = nv50_vertex_format[fmt].vtx;

        if (!so->element[i].state) {
            switch (util_format_get_nr_components(fmt)) {
            case 1: fmt = PIPE_FORMAT_R32_FLOAT; break;
            case 2: fmt = PIPE_FORMAT_R32G32_FLOAT; break;
            case 3: fmt = PIPE_FORMAT_R32G32B32_FLOAT; break;
            case 4: fmt = PIPE_FORMAT_R32G32B32A32_FLOAT; break;
            default:
                assert(0);
                FREE(so);
                return NULL;
            }
            so->element[i].state = nv50_vertex_format[fmt].vtx;
            so->need_conversion = true;
            util_debug_message(&nouveau_context(pipe)->debug, FALLBACK,
                               "Converting vertex element %d, no hw format %s",
                               i, util_format_name(ve->src_format));
        }
        so->element[i].state |= i;
        so->strides[vbi] = ve->src_stride;
        if (!ve->src_stride)
            so->vbo_constant |= 1 << vbi;

        size = util_format_get_blocksize(fmt);
        if (so->vb_access_size[vbi] < (ve->src_offset + size))
           so->vb_access_size[vbi] = ve->src_offset + size;

        if (1) {
            unsigned j = transkey.nr_elements++;

            transkey.element[j].type = TRANSLATE_ELEMENT_NORMAL;
            transkey.element[j].input_format = ve->src_format;
            transkey.element[j].input_buffer = vbi;
            transkey.element[j].input_offset = ve->src_offset;
            transkey.element[j].instance_divisor = ve->instance_divisor;

            transkey.element[j].output_format = fmt;
            transkey.element[j].output_offset = transkey.output_stride;
            transkey.output_stride += (util_format_get_stride(fmt, 1) + 3) & ~3;

            if (unlikely(ve->instance_divisor)) {
               so->instance_elts |= 1 << i;
               so->instance_bufs |= 1 << vbi;
               if (ve->instance_divisor < so->min_instance_div[vbi])
                  so->min_instance_div[vbi] = ve->instance_divisor;
            }
        }
    }

    so->translate = translate_create(&transkey);
    so->vertex_size = transkey.output_stride / 4;
    so->packet_vertex_limit = NV04_PFIFO_MAX_PACKET_LEN /
       MAX2(so->vertex_size, 1);

    return so;
}

#define NV50_3D_VERTEX_ATTRIB_INACTIVE              \
   NV50_3D_VERTEX_ARRAY_ATTRIB_TYPE_FLOAT |         \
   NV50_3D_VERTEX_ARRAY_ATTRIB_FORMAT_32_32_32_32 | \
   NV50_3D_VERTEX_ARRAY_ATTRIB_CONST

static void
nv50_emit_vtxattr(struct nv50_context *nv50, struct pipe_vertex_buffer *vb,
                  struct pipe_vertex_element *ve, unsigned attr)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   const void *data = (const uint8_t *)vb->buffer.user + ve->src_offset;
   float v[4];
   const unsigned nc = util_format_get_nr_components(ve->src_format);

   assert(vb->is_user_buffer);

   util_format_unpack_rgba(ve->src_format, v, data, 1);

   switch (nc) {
   case 4:
      BEGIN_NV04(push, NV50_3D(VTX_ATTR_4F_X(attr)), 4);
      PUSH_DATAf(push, v[0]);
      PUSH_DATAf(push, v[1]);
      PUSH_DATAf(push, v[2]);
      PUSH_DATAf(push, v[3]);
      break;
   case 3:
      BEGIN_NV04(push, NV50_3D(VTX_ATTR_3F_X(attr)), 3);
      PUSH_DATAf(push, v[0]);
      PUSH_DATAf(push, v[1]);
      PUSH_DATAf(push, v[2]);
      break;
   case 2:
      BEGIN_NV04(push, NV50_3D(VTX_ATTR_2F_X(attr)), 2);
      PUSH_DATAf(push, v[0]);
      PUSH_DATAf(push, v[1]);
      break;
   case 1:
      if (attr == nv50->vertprog->vp.edgeflag) {
         BEGIN_NV04(push, NV50_3D(EDGEFLAG), 1);
         PUSH_DATA (push, v[0] ? 1 : 0);
      }
      BEGIN_NV04(push, NV50_3D(VTX_ATTR_1F(attr)), 1);
      PUSH_DATAf(push, v[0]);
      break;
   default:
      assert(0);
      break;
   }
}

static inline void
nv50_user_vbuf_range(struct nv50_context *nv50, unsigned vbi,
                     uint32_t *base, uint32_t *size)
{
   assert(vbi < PIPE_MAX_ATTRIBS);
   if (unlikely(nv50->vertex->instance_bufs & (1 << vbi))) {
      const uint32_t div = nv50->vertex->min_instance_div[vbi];
      *base = nv50->instance_off * nv50->vertex->strides[vbi];
      *size = (nv50->instance_max / div) * nv50->vertex->strides[vbi] +
         nv50->vertex->vb_access_size[vbi];
   } else {
      /* NOTE: if there are user buffers, we *must* have index bounds */
      assert(nv50->vb_elt_limit != ~0);
      *base = nv50->vb_elt_first * nv50->vertex->strides[vbi];
      *size = nv50->vb_elt_limit * nv50->vertex->strides[vbi] +
         nv50->vertex->vb_access_size[vbi];
   }
}

static void
nv50_upload_user_buffers(struct nv50_context *nv50,
                         uint64_t addrs[], uint32_t limits[])
{
   unsigned b;

   assert(nv50->num_vtxbufs <= PIPE_MAX_ATTRIBS);
   for (b = 0; b < nv50->num_vtxbufs; ++b) {
      struct nouveau_bo *bo;
      const struct pipe_vertex_buffer *vb = &nv50->vtxbuf[b];
      uint32_t base, size;

      if (!(nv50->vbo_user & (1 << b)) || !nv50->vertex->strides[b])
         continue;
      nv50_user_vbuf_range(nv50, b, &base, &size);

      limits[b] = base + size - 1;
      addrs[b] = nouveau_scratch_data(&nv50->base, vb->buffer.user, base, size,
                                      &bo);
      if (addrs[b])
         BCTX_REFN_bo(nv50->bufctx_3d, 3D_VERTEX_TMP, NOUVEAU_BO_GART |
                      NOUVEAU_BO_RD, bo);
   }
   nv50->base.vbo_dirty = true;
}

static void
nv50_update_user_vbufs(struct nv50_context *nv50)
{
   uint64_t address[PIPE_MAX_ATTRIBS];
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   unsigned i;
   uint32_t written = 0;

   for (i = 0; i < nv50->vertex->num_elements; ++i) {
      struct pipe_vertex_element *ve = &nv50->vertex->element[i].pipe;
      const unsigned b = ve->vertex_buffer_index;
      struct pipe_vertex_buffer *vb;
      uint32_t base, size;

      assert(b < PIPE_MAX_ATTRIBS);
      vb = &nv50->vtxbuf[b];

      if (!(nv50->vbo_user & (1 << b)))
         continue;

      if (!ve->src_stride) {
         nv50_emit_vtxattr(nv50, vb, ve, i);
         continue;
      }
      nv50_user_vbuf_range(nv50, b, &base, &size);

      if (!(written & (1 << b))) {
         struct nouveau_bo *bo;
         const uint32_t bo_flags = NOUVEAU_BO_GART | NOUVEAU_BO_RD;
         written |= 1 << b;
         address[b] = nouveau_scratch_data(&nv50->base, vb->buffer.user,
                                           base, size, &bo);
         if (address[b])
            BCTX_REFN_bo(nv50->bufctx_3d, 3D_VERTEX_TMP, bo_flags, bo);
      }

      BEGIN_NV04(push, NV50_3D(VERTEX_ARRAY_LIMIT_HIGH(i)), 2);
      PUSH_DATAh(push, address[b] + base + size - 1);
      PUSH_DATA (push, address[b] + base + size - 1);
      BEGIN_NV04(push, NV50_3D(VERTEX_ARRAY_START_HIGH(i)), 2);
      PUSH_DATAh(push, address[b] + ve->src_offset);
      PUSH_DATA (push, address[b] + ve->src_offset);
   }
   nv50->base.vbo_dirty = true;
}

static inline void
nv50_release_user_vbufs(struct nv50_context *nv50)
{
   if (nv50->vbo_user) {
      nouveau_bufctx_reset(nv50->bufctx_3d, NV50_BIND_3D_VERTEX_TMP);
      nouveau_scratch_done(&nv50->base);
   }
}

void
nv50_vertex_arrays_validate(struct nv50_context *nv50)
{
   uint64_t addrs[PIPE_MAX_ATTRIBS];
   uint32_t limits[PIPE_MAX_ATTRIBS];
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   struct nv50_vertex_stateobj *vertex = nv50->vertex;
   struct nv50_vertex_element *ve;
   uint32_t mask;
   uint32_t refd = 0;
   unsigned i;
   const unsigned n = MAX2(vertex->num_elements, nv50->state.num_vtxelts);

   if (unlikely(vertex->need_conversion))
      nv50->vbo_fifo = ~0;
   else
   if (nv50->vbo_user & ~nv50->vbo_constant)
      nv50->vbo_fifo = nv50->vbo_push_hint ? ~0 : 0;
   else
      nv50->vbo_fifo = 0;

   if (!nv50->vbo_fifo) {
      /* if vertex buffer was written by GPU - flush VBO cache */
      assert(nv50->num_vtxbufs <= PIPE_MAX_ATTRIBS);
      for (i = 0; i < nv50->num_vtxbufs; ++i) {
         struct nv04_resource *buf = nv04_resource(nv50->vtxbuf[i].buffer.resource);
         if (!nv50->vtxbuf[i].is_user_buffer &&
             buf && buf->status & NOUVEAU_BUFFER_STATUS_GPU_WRITING) {
            buf->status &= ~NOUVEAU_BUFFER_STATUS_GPU_WRITING;
            nv50->base.vbo_dirty = true;
         }
      }
   }

   /* update vertex format state */
   BEGIN_NV04(push, NV50_3D(VERTEX_ARRAY_ATTRIB(0)), n);
   if (nv50->vbo_fifo) {
      nv50->state.num_vtxelts = vertex->num_elements;
      for (i = 0; i < vertex->num_elements; ++i)
         PUSH_DATA (push, vertex->element[i].state);
      for (; i < n; ++i)
         PUSH_DATA (push, NV50_3D_VERTEX_ATTRIB_INACTIVE);
      for (i = 0; i < n; ++i) {
         BEGIN_NV04(push, NV50_3D(VERTEX_ARRAY_FETCH(i)), 1);
         PUSH_DATA (push, 0);
      }
      return;
   }
   for (i = 0; i < vertex->num_elements; ++i) {
      const unsigned b = vertex->element[i].pipe.vertex_buffer_index;

      assert(b < PIPE_MAX_ATTRIBS);
      ve = &vertex->element[i];

      if (likely(vertex->strides[b]) || !(nv50->vbo_user & (1 << b)))
         PUSH_DATA(push, ve->state);
      else
         PUSH_DATA(push, ve->state | NV50_3D_VERTEX_ARRAY_ATTRIB_CONST);
   }
   for (; i < n; ++i)
      PUSH_DATA(push, NV50_3D_VERTEX_ATTRIB_INACTIVE);

   /* update per-instance enables */
   mask = vertex->instance_elts ^ nv50->state.instance_elts;
   while (mask) {
      const int i = ffs(mask) - 1;
      mask &= ~(1 << i);
      BEGIN_NV04(push, NV50_3D(VERTEX_ARRAY_PER_INSTANCE(i)), 1);
      PUSH_DATA (push, (vertex->instance_elts >> i) & 1);
   }
   nv50->state.instance_elts = vertex->instance_elts;

   if (nv50->vbo_user & ~nv50->vbo_constant)
      nv50_upload_user_buffers(nv50, addrs, limits);

   /* update buffers and set constant attributes */
   for (i = 0; i < vertex->num_elements; ++i) {
      uint64_t address, limit;
      const unsigned b = vertex->element[i].pipe.vertex_buffer_index;
      struct pipe_vertex_buffer *vb;

      assert(b < PIPE_MAX_ATTRIBS);
      ve = &vertex->element[i];
      vb = &nv50->vtxbuf[b];

      if (unlikely(nv50->vbo_constant & (1 << b))) {
         BEGIN_NV04(push, NV50_3D(VERTEX_ARRAY_FETCH(i)), 1);
         PUSH_DATA (push, 0);
         nv50_emit_vtxattr(nv50, vb, &ve->pipe, i);
         continue;
      } else
      if (nv50->vbo_user & (1 << b)) {
         address = addrs[b] + ve->pipe.src_offset;
         limit = addrs[b] + limits[b];
      } else
      if (!vb->buffer.resource) {
         BEGIN_NV04(push, NV50_3D(VERTEX_ARRAY_FETCH(i)), 1);
         PUSH_DATA (push, 0);
         continue;
      } else {
         struct nv04_resource *buf = nv04_resource(vb->buffer.resource);
         if (!(refd & (1 << b))) {
            refd |= 1 << b;
            BCTX_REFN(nv50->bufctx_3d, 3D_VERTEX, buf, RD);
         }
         address = buf->address + vb->buffer_offset + ve->pipe.src_offset;
         limit = buf->address + buf->base.width0 - 1;
      }

      if (unlikely(ve->pipe.instance_divisor)) {
         BEGIN_NV04(push, NV50_3D(VERTEX_ARRAY_FETCH(i)), 4);
         PUSH_DATA (push, NV50_3D_VERTEX_ARRAY_FETCH_ENABLE | vertex->strides[b]);
         PUSH_DATAh(push, address);
         PUSH_DATA (push, address);
         PUSH_DATA (push, ve->pipe.instance_divisor);
      } else {
         BEGIN_NV04(push, NV50_3D(VERTEX_ARRAY_FETCH(i)), 3);
         PUSH_DATA (push, NV50_3D_VERTEX_ARRAY_FETCH_ENABLE | vertex->strides[b]);
         PUSH_DATAh(push, address);
         PUSH_DATA (push, address);
      }
      BEGIN_NV04(push, NV50_3D(VERTEX_ARRAY_LIMIT_HIGH(i)), 2);
      PUSH_DATAh(push, limit);
      PUSH_DATA (push, limit);
   }
   for (; i < nv50->state.num_vtxelts; ++i) {
      BEGIN_NV04(push, NV50_3D(VERTEX_ARRAY_FETCH(i)), 1);
      PUSH_DATA (push, 0);
   }
   nv50->state.num_vtxelts = vertex->num_elements;
}

#define NV50_PRIM_GL_CASE(n) \
   case MESA_PRIM_##n: return NV50_3D_VERTEX_BEGIN_GL_PRIMITIVE_##n

static inline unsigned
nv50_prim_gl(unsigned prim)
{
   switch (prim) {
   NV50_PRIM_GL_CASE(POINTS);
   NV50_PRIM_GL_CASE(LINES);
   NV50_PRIM_GL_CASE(LINE_LOOP);
   NV50_PRIM_GL_CASE(LINE_STRIP);
   NV50_PRIM_GL_CASE(TRIANGLES);
   NV50_PRIM_GL_CASE(TRIANGLE_STRIP);
   NV50_PRIM_GL_CASE(TRIANGLE_FAN);
   NV50_PRIM_GL_CASE(QUADS);
   NV50_PRIM_GL_CASE(QUAD_STRIP);
   NV50_PRIM_GL_CASE(POLYGON);
   NV50_PRIM_GL_CASE(LINES_ADJACENCY);
   NV50_PRIM_GL_CASE(LINE_STRIP_ADJACENCY);
   NV50_PRIM_GL_CASE(TRIANGLES_ADJACENCY);
   NV50_PRIM_GL_CASE(TRIANGLE_STRIP_ADJACENCY);
   default:
      return NV50_3D_VERTEX_BEGIN_GL_PRIMITIVE_POINTS;
      break;
   }
}

/* For pre-nva0 transform feedback. */
static const uint8_t nv50_pipe_prim_to_prim_size[MESA_PRIM_COUNT + 1] =
{
   [MESA_PRIM_POINTS] = 1,
   [MESA_PRIM_LINES] = 2,
   [MESA_PRIM_LINE_LOOP] = 2,
   [MESA_PRIM_LINE_STRIP] = 2,
   [MESA_PRIM_TRIANGLES] = 3,
   [MESA_PRIM_TRIANGLE_STRIP] = 3,
   [MESA_PRIM_TRIANGLE_FAN] = 3,
   [MESA_PRIM_QUADS] = 3,
   [MESA_PRIM_QUAD_STRIP] = 3,
   [MESA_PRIM_POLYGON] = 3,
   [MESA_PRIM_LINES_ADJACENCY] = 2,
   [MESA_PRIM_LINE_STRIP_ADJACENCY] = 2,
   [MESA_PRIM_TRIANGLES_ADJACENCY] = 3,
   [MESA_PRIM_TRIANGLE_STRIP_ADJACENCY] = 3
};

static void
nv50_draw_arrays(struct nv50_context *nv50,
                 unsigned mode, unsigned start, unsigned count,
                 unsigned instance_count)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   unsigned prim;

   if (nv50->state.index_bias) {
      BEGIN_NV04(push, NV50_3D(VB_ELEMENT_BASE), 1);
      PUSH_DATA (push, 0);
      if (nv50->screen->base.class_3d >= NV84_3D_CLASS) {
         BEGIN_NV04(push, NV84_3D(VERTEX_ID_BASE), 1);
         PUSH_DATA (push, 0);
      }
      nv50->state.index_bias = 0;
   }

   prim = nv50_prim_gl(mode);

   while (instance_count--) {
      BEGIN_NV04(push, NV50_3D(VERTEX_BEGIN_GL), 1);
      PUSH_DATA (push, prim);
      BEGIN_NV04(push, NV50_3D(VERTEX_BUFFER_FIRST), 2);
      PUSH_DATA (push, start);
      PUSH_DATA (push, count);
      BEGIN_NV04(push, NV50_3D(VERTEX_END_GL), 1);
      PUSH_DATA (push, 0);

      prim |= NV50_3D_VERTEX_BEGIN_GL_INSTANCE_NEXT;
   }
}

static void
nv50_draw_elements_inline_u08(struct nouveau_pushbuf *push, const uint8_t *map,
                              unsigned start, unsigned count)
{
   map += start;

   if (count & 3) {
      unsigned i;
      BEGIN_NI04(push, NV50_3D(VB_ELEMENT_U32), count & 3);
      for (i = 0; i < (count & 3); ++i)
         PUSH_DATA(push, *map++);
      count &= ~3;
   }
   while (count) {
      unsigned i, nr = MIN2(count, NV04_PFIFO_MAX_PACKET_LEN * 4) / 4;

      BEGIN_NI04(push, NV50_3D(VB_ELEMENT_U8), nr);
      for (i = 0; i < nr; ++i) {
         PUSH_DATA(push,
                   (map[3] << 24) | (map[2] << 16) | (map[1] << 8) | map[0]);
         map += 4;
      }
      count -= nr * 4;
   }
}

static void
nv50_draw_elements_inline_u16(struct nouveau_pushbuf *push, const uint16_t *map,
                              unsigned start, unsigned count)
{
   map += start;

   if (count & 1) {
      count &= ~1;
      BEGIN_NV04(push, NV50_3D(VB_ELEMENT_U32), 1);
      PUSH_DATA (push, *map++);
   }
   while (count) {
      unsigned i, nr = MIN2(count, NV04_PFIFO_MAX_PACKET_LEN * 2) / 2;

      BEGIN_NI04(push, NV50_3D(VB_ELEMENT_U16), nr);
      for (i = 0; i < nr; ++i) {
         PUSH_DATA(push, (map[1] << 16) | map[0]);
         map += 2;
      }
      count -= nr * 2;
   }
}

static void
nv50_draw_elements_inline_u32(struct nouveau_pushbuf *push, const uint32_t *map,
                              unsigned start, unsigned count)
{
   map += start;

   while (count) {
      const unsigned nr = MIN2(count, NV04_PFIFO_MAX_PACKET_LEN);

      BEGIN_NI04(push, NV50_3D(VB_ELEMENT_U32), nr);
      PUSH_DATAp(push, map, nr);

      map += nr;
      count -= nr;
   }
}

static void
nv50_draw_elements_inline_u32_short(struct nouveau_pushbuf *push,
                                    const uint32_t *map,
                                    unsigned start, unsigned count)
{
   map += start;

   if (count & 1) {
      count--;
      BEGIN_NV04(push, NV50_3D(VB_ELEMENT_U32), 1);
      PUSH_DATA (push, *map++);
   }
   while (count) {
      unsigned i, nr = MIN2(count, NV04_PFIFO_MAX_PACKET_LEN * 2) / 2;

      BEGIN_NI04(push, NV50_3D(VB_ELEMENT_U16), nr);
      for (i = 0; i < nr; ++i) {
         PUSH_DATA(push, (map[1] << 16) | map[0]);
         map += 2;
      }
      count -= nr * 2;
   }
}

static void
nv50_draw_elements(struct nv50_context *nv50, bool shorten,
                   const struct pipe_draw_info *info,
                   unsigned mode, unsigned start, unsigned count,
                   unsigned instance_count, int32_t index_bias,
		   unsigned index_size)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   unsigned prim;

   prim = nv50_prim_gl(mode);

   if (index_bias != nv50->state.index_bias) {
      BEGIN_NV04(push, NV50_3D(VB_ELEMENT_BASE), 1);
      PUSH_DATA (push, index_bias);
      if (nv50->screen->base.class_3d >= NV84_3D_CLASS) {
         BEGIN_NV04(push, NV84_3D(VERTEX_ID_BASE), 1);
         PUSH_DATA (push, index_bias);
      }
      nv50->state.index_bias = index_bias;
   }

   if (!info->has_user_indices) {
      struct nv04_resource *buf = nv04_resource(info->index.resource);
      unsigned pb_start;
      unsigned pb_bytes;
      const unsigned base = buf->offset & ~3;

      start += (buf->offset & 3) >> (index_size >> 1);

      assert(nouveau_resource_mapped_by_gpu(info->index.resource));

      /* This shouldn't have to be here. The going theory is that the buffer
       * is being filled in by PGRAPH, and it's not done yet by the time it
       * gets submitted to PFIFO, which in turn starts immediately prefetching
       * the not-yet-written data. Ideally this wait would only happen on
       * pushbuf submit, but it's probably not a big performance difference.
       */
      if (buf->fence_wr)
         nouveau_fence_wait(buf->fence_wr, &nv50->base.debug);

      while (instance_count--) {
         BEGIN_NV04(push, NV50_3D(VERTEX_BEGIN_GL), 1);
         PUSH_DATA (push, prim);

         PUSH_SPACE_EX(push, 16, 0, 1);
         PUSH_REF1(push, buf->bo, NOUVEAU_BO_RD | buf->domain);

         switch (index_size) {
         case 4:
            BEGIN_NL50(push, NV50_3D(VB_ELEMENT_U32), count);
            nouveau_pushbuf_data(push, buf->bo, base + start * 4, count * 4);
            break;
         case 2:
            pb_start = (start & ~1) * 2;
            pb_bytes = ((start + count + 1) & ~1) * 2 - pb_start;

            BEGIN_NV04(push, NV50_3D(VB_ELEMENT_U16_SETUP), 1);
            PUSH_DATA (push, (start << 31) | count);
            BEGIN_NL50(push, NV50_3D(VB_ELEMENT_U16), pb_bytes / 4);
            nouveau_pushbuf_data(push, buf->bo, base + pb_start, pb_bytes);
            BEGIN_NV04(push, NV50_3D(VB_ELEMENT_U16_SETUP), 1);
            PUSH_DATA (push, 0);
            break;
         default:
            assert(index_size == 1);
            pb_start = start & ~3;
            pb_bytes = ((start + count + 3) & ~3) - pb_start;

            BEGIN_NV04(push, NV50_3D(VB_ELEMENT_U8_SETUP), 1);
            PUSH_DATA (push, (start << 30) | count);
            BEGIN_NL50(push, NV50_3D(VB_ELEMENT_U8), pb_bytes / 4);
            nouveau_pushbuf_data(push, buf->bo, base + pb_start, pb_bytes);
            BEGIN_NV04(push, NV50_3D(VB_ELEMENT_U8_SETUP), 1);
            PUSH_DATA (push, 0);
            break;
         }
         BEGIN_NV04(push, NV50_3D(VERTEX_END_GL), 1);
         PUSH_DATA (push, 0);

         prim |= NV50_3D_VERTEX_BEGIN_GL_INSTANCE_NEXT;
      }
   } else {
      const void *data = info->index.user;

      while (instance_count--) {
         BEGIN_NV04(push, NV50_3D(VERTEX_BEGIN_GL), 1);
         PUSH_DATA (push, prim);
         switch (index_size) {
         case 1:
            nv50_draw_elements_inline_u08(push, data, start, count);
            break;
         case 2:
            nv50_draw_elements_inline_u16(push, data, start, count);
            break;
         case 4:
            if (shorten)
               nv50_draw_elements_inline_u32_short(push, data, start, count);
            else
               nv50_draw_elements_inline_u32(push, data, start, count);
            break;
         default:
            assert(0);
            return;
         }
         BEGIN_NV04(push, NV50_3D(VERTEX_END_GL), 1);
         PUSH_DATA (push, 0);

         prim |= NV50_3D_VERTEX_BEGIN_GL_INSTANCE_NEXT;
      }
   }
   NOUVEAU_DRV_STAT(&nv50->screen->base, draw_calls_indexed, 1);
}

static void
nva0_draw_stream_output(struct nv50_context *nv50,
                        const struct pipe_draw_info *info,
                        const struct pipe_draw_indirect_info *indirect)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   struct nv50_so_target *so = nv50_so_target(indirect->count_from_stream_output);
   struct nv04_resource *res = nv04_resource(so->pipe.buffer);
   unsigned num_instances = info->instance_count;
   unsigned mode = nv50_prim_gl(info->mode);

   if (unlikely(nv50->screen->base.class_3d < NVA0_3D_CLASS)) {
      /* A proper implementation without waiting doesn't seem possible,
       * so don't bother.
       */
      NOUVEAU_ERR("draw_stream_output not supported on pre-NVA0 cards\n");
      return;
   }

   if (res->status & NOUVEAU_BUFFER_STATUS_GPU_WRITING) {
      res->status &= ~NOUVEAU_BUFFER_STATUS_GPU_WRITING;
      PUSH_SPACE(push, 4);
      BEGIN_NV04(push, SUBC_3D(NV50_GRAPH_SERIALIZE), 1);
      PUSH_DATA (push, 0);
      BEGIN_NV04(push, NV50_3D(VERTEX_ARRAY_FLUSH), 1);
      PUSH_DATA (push, 0);
   }

   assert(num_instances);
   do {
      PUSH_SPACE(push, 8);
      BEGIN_NV04(push, NV50_3D(VERTEX_BEGIN_GL), 1);
      PUSH_DATA (push, mode);
      BEGIN_NV04(push, NVA0_3D(DRAW_TFB_BASE), 1);
      PUSH_DATA (push, 0);
      BEGIN_NV04(push, NVA0_3D(DRAW_TFB_STRIDE), 1);
      PUSH_DATA (push, so->stride);
      nv50_hw_query_pushbuf_submit(nv50, NVA0_3D_DRAW_TFB_BYTES,
                                   nv50_query(so->pq), 0x4);
      BEGIN_NV04(push, NV50_3D(VERTEX_END_GL), 1);
      PUSH_DATA (push, 0);

      mode |= NV50_3D_VERTEX_BEGIN_GL_INSTANCE_NEXT;
   } while (--num_instances);
}

static void
nv50_draw_vbo_kick_notify(struct nouveau_context *context)
{
   _nouveau_fence_update(context->screen, true);
}

void
nv50_draw_vbo(struct pipe_context *pipe, const struct pipe_draw_info *info,
              unsigned drawid_offset,
              const struct pipe_draw_indirect_info *indirect,
              const struct pipe_draw_start_count_bias *draws,
              unsigned num_draws)
{
   if (num_draws > 1) {
      util_draw_multi(pipe, info, drawid_offset, indirect, draws, num_draws);
      return;
   }

   if (!indirect && (!draws[0].count || !info->instance_count))
      return;

   /* We don't actually support indirect draws, so add a fallback for ES 3.1's
    * benefit.
    */
   if (indirect && indirect->buffer) {
      util_draw_indirect(pipe, info, indirect);
      return;
   }

   struct nv50_context *nv50 = nv50_context(pipe);
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   bool tex_dirty = false;
   int s;

   if (info->index_size && !info->has_user_indices)
      BCTX_REFN(nv50->bufctx_3d, 3D_INDEX, nv04_resource(info->index.resource), RD);

   /* NOTE: caller must ensure that (min_index + index_bias) is >= 0 */
   if (info->index_bounds_valid) {
      nv50->vb_elt_first = info->min_index + (info->index_size ? draws->index_bias : 0);
      nv50->vb_elt_limit = info->max_index - info->min_index;
   } else {
      nv50->vb_elt_first = 0;
      nv50->vb_elt_limit = ~0;
   }
   nv50->instance_off = info->start_instance;
   nv50->instance_max = info->instance_count - 1;

   /* For picking only a few vertices from a large user buffer, push is better,
    * if index count is larger and we expect repeated vertices, suggest upload.
    */
   nv50->vbo_push_hint = /* the 64 is heuristic */
      !(info->index_size && ((nv50->vb_elt_limit + 64) < draws[0].count));

   if (nv50->dirty_3d & (NV50_NEW_3D_ARRAYS | NV50_NEW_3D_VERTEX))
      nv50->vbo_constant = nv50->vertex->vbo_constant & nv50->vbo_user;
   if (nv50->vbo_user && !(nv50->dirty_3d & (NV50_NEW_3D_ARRAYS | NV50_NEW_3D_VERTEX))) {
      if (!!nv50->vbo_fifo != nv50->vbo_push_hint)
         nv50->dirty_3d |= NV50_NEW_3D_ARRAYS;
      else
      if (!nv50->vbo_fifo)
         nv50_update_user_vbufs(nv50);
   }

   if (unlikely(nv50->num_so_targets && !nv50->gmtyprog))
      nv50->state.prim_size = nv50_pipe_prim_to_prim_size[info->mode];

   simple_mtx_lock(&nv50->screen->state_lock);
   nv50_state_validate_3d(nv50, ~0);

   nv50->base.kick_notify = nv50_draw_vbo_kick_notify;

   for (s = 0; s < NV50_MAX_3D_SHADER_STAGES && !nv50->cb_dirty; ++s) {
      if (nv50->constbuf_coherent[s])
         nv50->cb_dirty = true;
   }

   /* If there are any coherent constbufs, flush the cache */
   if (nv50->cb_dirty) {
      BEGIN_NV04(push, NV50_3D(CODE_CB_FLUSH), 1);
      PUSH_DATA (push, 0);
      nv50->cb_dirty = false;
   }

   for (s = 0; s < NV50_MAX_3D_SHADER_STAGES && !tex_dirty; ++s) {
      if (nv50->textures_coherent[s])
         tex_dirty = true;
   }

   if (tex_dirty) {
      BEGIN_NV04(push, NV50_3D(TEX_CACHE_CTL), 1);
      PUSH_DATA (push, 0x20);
   }

   if (nv50->screen->base.class_3d >= NVA0_3D_CLASS &&
       nv50->seamless_cube_map != nv50->state.seamless_cube_map) {
      nv50->state.seamless_cube_map = nv50->seamless_cube_map;
      BEGIN_NV04(push, SUBC_3D(NVA0_3D_TEX_MISC), 1);
      PUSH_DATA (push, nv50->seamless_cube_map ? NVA0_3D_TEX_MISC_SEAMLESS_CUBE_MAP : 0);
   }

   if (nv50->vertprog->mul_zero_wins != nv50->state.mul_zero_wins) {
      nv50->state.mul_zero_wins = nv50->vertprog->mul_zero_wins;
      BEGIN_NV04(push, NV50_3D(UNK1690), 1);
      PUSH_DATA (push, 0x00010000 * !!nv50->state.mul_zero_wins);
   }

   /* Make starting/pausing streamout work pre-NVA0 enough for ES3.0. This
    * means counting vertices in a vertex shader when it has so outputs.
    */
   if (nv50->screen->base.class_3d < NVA0_3D_CLASS &&
       nv50->vertprog->stream_output.num_outputs) {
      for (int i = 0; i < nv50->num_so_targets; i++) {
         nv50->so_used[i] += info->instance_count *
            u_stream_outputs_for_vertices(info->mode, draws[0].count) *
            nv50->vertprog->stream_output.stride[i] * 4;
      }
   }

   if (nv50->vbo_fifo) {
      nv50_push_vbo(nv50, info, indirect, &draws[0]);
      goto cleanup;
   }

   if (nv50->state.instance_base != info->start_instance) {
      nv50->state.instance_base = info->start_instance;
      /* NOTE: this does not affect the shader input, should it ? */
      BEGIN_NV04(push, NV50_3D(VB_INSTANCE_BASE), 1);
      PUSH_DATA (push, info->start_instance);
   }

   nv50->base.vbo_dirty |= !!nv50->vtxbufs_coherent;

   if (nv50->base.vbo_dirty) {
      BEGIN_NV04(push, NV50_3D(VERTEX_ARRAY_FLUSH), 1);
      PUSH_DATA (push, 0);
      nv50->base.vbo_dirty = false;
   }

   if (info->index_size) {
      bool shorten = info->index_bounds_valid && info->max_index <= 65535;

      if (info->primitive_restart != nv50->state.prim_restart) {
         if (info->primitive_restart) {
            BEGIN_NV04(push, NV50_3D(PRIM_RESTART_ENABLE), 2);
            PUSH_DATA (push, 1);
            PUSH_DATA (push, info->restart_index);

            if (info->restart_index > 65535)
               shorten = false;
         } else {
            BEGIN_NV04(push, NV50_3D(PRIM_RESTART_ENABLE), 1);
            PUSH_DATA (push, 0);
         }
         nv50->state.prim_restart = info->primitive_restart;
      } else
      if (info->primitive_restart) {
         BEGIN_NV04(push, NV50_3D(PRIM_RESTART_INDEX), 1);
         PUSH_DATA (push, info->restart_index);

         if (info->restart_index > 65535)
            shorten = false;
      }

      nv50_draw_elements(nv50, shorten, info,
                         info->mode, draws[0].start, draws[0].count,
                         info->instance_count, draws->index_bias, info->index_size);
   } else
   if (unlikely(indirect && indirect->count_from_stream_output)) {
      nva0_draw_stream_output(nv50, info, indirect);
   } else {
      nv50_draw_arrays(nv50,
                       info->mode, draws[0].start, draws[0].count,
                       info->instance_count);
   }

cleanup:
   PUSH_KICK(push);
   simple_mtx_unlock(&nv50->screen->state_lock);

   nv50->base.kick_notify = nv50_default_kick_notify;

   nv50_release_user_vbufs(nv50);

   nouveau_pushbuf_bufctx(push, NULL);

   nouveau_bufctx_reset(nv50->bufctx_3d, NV50_BIND_3D_INDEX);
}
