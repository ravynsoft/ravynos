
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "util/u_inlines.h"
#include "util/format/u_format.h"
#include "translate/translate.h"

#include "nv50/nv50_context.h"
#include "nv50/nv50_resource.h"

#include "nv50/nv50_3d.xml.h"

struct push_context {
   struct nouveau_pushbuf *push;

   const void *idxbuf;

   float edgeflag;
   int edgeflag_attr;

   uint32_t vertex_words;
   uint32_t packet_vertex_limit;

   struct translate *translate;

   bool primitive_restart;

   bool need_vertex_id;
   int32_t index_bias;

   uint32_t prim;
   uint32_t restart_index;
   uint32_t start_instance;
   uint32_t instance_id;
};

static inline unsigned
prim_restart_search_i08(uint8_t *elts, unsigned push, uint8_t index)
{
   unsigned i;
   for (i = 0; i < push; ++i)
      if (elts[i] == index)
         break;
   return i;
}

static inline unsigned
prim_restart_search_i16(uint16_t *elts, unsigned push, uint16_t index)
{
   unsigned i;
   for (i = 0; i < push; ++i)
      if (elts[i] == index)
         break;
   return i;
}

static inline unsigned
prim_restart_search_i32(uint32_t *elts, unsigned push, uint32_t index)
{
   unsigned i;
   for (i = 0; i < push; ++i)
      if (elts[i] == index)
         break;
   return i;
}

static void
emit_vertices_i08(struct push_context *ctx, unsigned start, unsigned count)
{
   uint8_t *elts = (uint8_t *)ctx->idxbuf + start;

   while (count) {
      unsigned push = MIN2(count, ctx->packet_vertex_limit);
      unsigned size, nr;

      nr = push;
      if (ctx->primitive_restart)
         nr = prim_restart_search_i08(elts, push, ctx->restart_index);

      size = ctx->vertex_words * nr;

      if (unlikely(ctx->need_vertex_id)) {
         BEGIN_NV04(ctx->push, NV84_3D(VERTEX_ID_BASE), 1);
         PUSH_DATA (ctx->push, *elts + ctx->index_bias);
      }

      BEGIN_NI04(ctx->push, NV50_3D(VERTEX_DATA), size);

      ctx->translate->run_elts8(ctx->translate, elts, nr,
                                ctx->start_instance, ctx->instance_id,
                                ctx->push->cur);

      ctx->push->cur += size;
      count -= nr;
      elts += nr;

      if (nr != push) {
         count--;
         elts++;
         BEGIN_NV04(ctx->push, NV50_3D(VB_ELEMENT_U32), 1);
         PUSH_DATA (ctx->push, ctx->restart_index);
      }
   }
}

static void
emit_vertices_i16(struct push_context *ctx, unsigned start, unsigned count)
{
   uint16_t *elts = (uint16_t *)ctx->idxbuf + start;

   while (count) {
      unsigned push = MIN2(count, ctx->packet_vertex_limit);
      unsigned size, nr;

      nr = push;
      if (ctx->primitive_restart)
         nr = prim_restart_search_i16(elts, push, ctx->restart_index);

      size = ctx->vertex_words * nr;

      if (unlikely(ctx->need_vertex_id)) {
         BEGIN_NV04(ctx->push, NV84_3D(VERTEX_ID_BASE), 1);
         PUSH_DATA (ctx->push, *elts + ctx->index_bias);
      }

      BEGIN_NI04(ctx->push, NV50_3D(VERTEX_DATA), size);

      ctx->translate->run_elts16(ctx->translate, elts, nr,
                                 ctx->start_instance, ctx->instance_id,
                                 ctx->push->cur);

      ctx->push->cur += size;
      count -= nr;
      elts += nr;

      if (nr != push) {
         count--;
         elts++;
         BEGIN_NV04(ctx->push, NV50_3D(VB_ELEMENT_U32), 1);
         PUSH_DATA (ctx->push, ctx->restart_index);
      }
   }
}

static void
emit_vertices_i32(struct push_context *ctx, unsigned start, unsigned count)
{
   uint32_t *elts = (uint32_t *)ctx->idxbuf + start;

   while (count) {
      unsigned push = MIN2(count, ctx->packet_vertex_limit);
      unsigned size, nr;

      nr = push;
      if (ctx->primitive_restart)
         nr = prim_restart_search_i32(elts, push, ctx->restart_index);

      size = ctx->vertex_words * nr;

      if (unlikely(ctx->need_vertex_id)) {
         BEGIN_NV04(ctx->push, NV84_3D(VERTEX_ID_BASE), 1);
         PUSH_DATA (ctx->push, *elts + ctx->index_bias);
      }

      BEGIN_NI04(ctx->push, NV50_3D(VERTEX_DATA), size);

      ctx->translate->run_elts(ctx->translate, elts, nr,
                               ctx->start_instance, ctx->instance_id,
                               ctx->push->cur);

      ctx->push->cur += size;
      count -= nr;
      elts += nr;

      if (nr != push) {
         count--;
         elts++;
         BEGIN_NV04(ctx->push, NV50_3D(VB_ELEMENT_U32), 1);
         PUSH_DATA (ctx->push, ctx->restart_index);
      }
   }
}

static void
emit_vertices_seq(struct push_context *ctx, unsigned start, unsigned count)
{
   uint32_t elts = 0;

   while (count) {
      unsigned push = MIN2(count, ctx->packet_vertex_limit);
      unsigned size = ctx->vertex_words * push;

      if (unlikely(ctx->need_vertex_id)) {
         /* For non-indexed draws, gl_VertexID goes up after each vertex. */
         BEGIN_NV04(ctx->push, NV84_3D(VERTEX_ID_BASE), 1);
         PUSH_DATA (ctx->push, elts++);
      }

      BEGIN_NI04(ctx->push, NV50_3D(VERTEX_DATA), size);

      ctx->translate->run(ctx->translate, start, push,
                          ctx->start_instance, ctx->instance_id,
                          ctx->push->cur);
      ctx->push->cur += size;
      count -= push;
      start += push;
   }
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
   /*
   NV50_PRIM_GL_CASE(PATCHES); */
   default:
      return NV50_3D_VERTEX_BEGIN_GL_PRIMITIVE_POINTS;
      break;
   }
}

void
nv50_push_vbo(struct nv50_context *nv50, const struct pipe_draw_info *info,
              const struct pipe_draw_indirect_info *indirect,
              const struct pipe_draw_start_count_bias *draw)
{
   struct push_context ctx;
   unsigned i, index_size;
   unsigned inst_count = info->instance_count;
   unsigned vert_count = draw->count;
   bool apply_bias = info->index_size && draw->index_bias;

   ctx.push = nv50->base.pushbuf;
   ctx.translate = nv50->vertex->translate;

   ctx.need_vertex_id = nv50->screen->base.class_3d >= NV84_3D_CLASS &&
      nv50->vertprog->vp.need_vertex_id && (nv50->vertex->num_elements < 32);
   ctx.index_bias = info->index_size ? draw->index_bias : 0;
   ctx.instance_id = 0;

   /* For indexed draws, gl_VertexID must be emitted for every vertex. */
   ctx.packet_vertex_limit =
      ctx.need_vertex_id ? 1 : nv50->vertex->packet_vertex_limit;
   ctx.vertex_words = nv50->vertex->vertex_size;

   assert(nv50->num_vtxbufs <= PIPE_MAX_ATTRIBS);
   for (i = 0; i < nv50->num_vtxbufs; ++i) {
      const struct pipe_vertex_buffer *vb = &nv50->vtxbuf[i];
      const uint8_t *data;

      if (unlikely(!vb->is_user_buffer)) {
         if (!vb->buffer.resource)
            continue;

         data = nouveau_resource_map_offset(&nv50->base,
            nv04_resource(vb->buffer.resource), vb->buffer_offset, NOUVEAU_BO_RD);
      } else
         data = vb->buffer.user;

      if (apply_bias && likely(!(nv50->vertex->instance_bufs & (1 << i))))
         data += (ptrdiff_t)(info->index_size ? draw->index_bias : 0) * nv50->vertex->strides[i];

      ctx.translate->set_buffer(ctx.translate, i, data, nv50->vertex->strides[i], ~0);
   }

   if (info->index_size) {
      if (!info->has_user_indices) {
         ctx.idxbuf = nouveau_resource_map_offset(&nv50->base,
            nv04_resource(info->index.resource), 0, NOUVEAU_BO_RD);
      } else {
         ctx.idxbuf = info->index.user;
      }
      if (!ctx.idxbuf)
         return;
      index_size = info->index_size;
      ctx.primitive_restart = info->primitive_restart;
      ctx.restart_index = info->restart_index;
   } else {
      if (unlikely(indirect && indirect->count_from_stream_output)) {
         struct pipe_context *pipe = &nv50->base.pipe;
         struct nv50_so_target *targ;
         targ = nv50_so_target(indirect->count_from_stream_output);
         if (!targ->pq) {
            NOUVEAU_ERR("draw_stream_output not supported on pre-NVA0 cards\n");
            return;
         }
         pipe->get_query_result(pipe, targ->pq, true, (void *)&vert_count);
         vert_count /= targ->stride;
      }
      ctx.idxbuf = NULL;
      index_size = 0;
      ctx.primitive_restart = false;
      ctx.restart_index = 0;
   }

   ctx.start_instance = info->start_instance;
   ctx.prim = nv50_prim_gl(info->mode);

   if (info->primitive_restart) {
      BEGIN_NV04(ctx.push, NV50_3D(PRIM_RESTART_ENABLE), 2);
      PUSH_DATA (ctx.push, 1);
      PUSH_DATA (ctx.push, info->restart_index);
   } else
   if (nv50->state.prim_restart) {
      BEGIN_NV04(ctx.push, NV50_3D(PRIM_RESTART_ENABLE), 1);
      PUSH_DATA (ctx.push, 0);
   }
   nv50->state.prim_restart = info->primitive_restart;

   while (inst_count--) {
      BEGIN_NV04(ctx.push, NV50_3D(VERTEX_BEGIN_GL), 1);
      PUSH_DATA (ctx.push, ctx.prim);
      switch (index_size) {
      case 0:
         emit_vertices_seq(&ctx, draw->start, vert_count);
         break;
      case 1:
         emit_vertices_i08(&ctx, draw->start, vert_count);
         break;
      case 2:
         emit_vertices_i16(&ctx, draw->start, vert_count);
         break;
      case 4:
         emit_vertices_i32(&ctx, draw->start, vert_count);
         break;
      default:
         assert(0);
         break;
      }
      BEGIN_NV04(ctx.push, NV50_3D(VERTEX_END_GL), 1);
      PUSH_DATA (ctx.push, 0);

      ctx.instance_id++;
      ctx.prim |= NV50_3D_VERTEX_BEGIN_GL_INSTANCE_NEXT;
   }

   if (unlikely(ctx.need_vertex_id)) {
      /* Reset gl_VertexID to prevent future indexed draws to be confused. */
      BEGIN_NV04(ctx.push, NV84_3D(VERTEX_ID_BASE), 1);
      PUSH_DATA (ctx.push, nv50->state.index_bias);
   }
}
