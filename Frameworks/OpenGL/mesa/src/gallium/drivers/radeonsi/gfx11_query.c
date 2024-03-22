/*
 * Copyright 2018 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "si_pipe.h"
#include "si_query.h"
#include "sid.h"
#include "util/u_memory.h"
#include "util/u_suballoc.h"

#include <stddef.h>

static void emit_shader_query(struct si_context *sctx, unsigned index)
{
   assert(!list_is_empty(&sctx->shader_query_buffers));

   struct gfx11_sh_query_buffer *qbuf =
      list_last_entry(&sctx->shader_query_buffers, struct gfx11_sh_query_buffer, list);
   qbuf->head += sizeof(struct gfx11_sh_query_buffer_mem);
}

static void gfx11_release_query_buffers(struct si_context *sctx,
                                        struct gfx11_sh_query_buffer *first,
                                        struct gfx11_sh_query_buffer *last)
{
   while (first) {
      struct gfx11_sh_query_buffer *qbuf = first;
      if (first != last)
         first = list_entry(qbuf->list.next, struct gfx11_sh_query_buffer, list);
      else
         first = NULL;

      qbuf->refcount--;
      if (qbuf->refcount)
         continue;

      if (qbuf->list.next == &sctx->shader_query_buffers)
         continue; /* keep the most recent buffer; it may not be full yet */
      if (qbuf->list.prev == &sctx->shader_query_buffers)
         continue; /* keep the oldest buffer for recycling */

      list_del(&qbuf->list);
      si_resource_reference(&qbuf->buf, NULL);
      FREE(qbuf);
   }
}

static bool gfx11_alloc_query_buffer(struct si_context *sctx)
{
   if (si_is_atom_dirty(sctx, &sctx->atoms.s.shader_query))
      return true;

   struct gfx11_sh_query_buffer *qbuf = NULL;

   if (!list_is_empty(&sctx->shader_query_buffers)) {
      qbuf = list_last_entry(&sctx->shader_query_buffers, struct gfx11_sh_query_buffer, list);
      if (qbuf->head + sizeof(struct gfx11_sh_query_buffer_mem) <= qbuf->buf->b.b.width0)
         goto success;

      qbuf = list_first_entry(&sctx->shader_query_buffers, struct gfx11_sh_query_buffer, list);
      if (!qbuf->refcount &&
          !si_cs_is_buffer_referenced(sctx, qbuf->buf->buf, RADEON_USAGE_READWRITE) &&
          sctx->ws->buffer_wait(sctx->ws, qbuf->buf->buf, 0, RADEON_USAGE_READWRITE)) {
         /* Can immediately re-use the oldest buffer */
         list_del(&qbuf->list);
      } else {
         qbuf = NULL;
      }
   }

   if (!qbuf) {
      qbuf = CALLOC_STRUCT(gfx11_sh_query_buffer);
      if (unlikely(!qbuf))
         return false;

      struct si_screen *screen = sctx->screen;
      unsigned buf_size =
         MAX2(sizeof(struct gfx11_sh_query_buffer_mem), screen->info.min_alloc_size);
      qbuf->buf = si_resource(pipe_buffer_create(&screen->b, 0, PIPE_USAGE_STAGING, buf_size));
      if (unlikely(!qbuf->buf)) {
         FREE(qbuf);
         return false;
      }
   }

   /* The buffer is currently unused by the GPU. Initialize it.
    *
    * We need to set the high bit of all the primitive counters for
    * compatibility with the SET_PREDICATION packet.
    */
   uint64_t *results = sctx->ws->buffer_map(sctx->ws, qbuf->buf->buf, NULL,
                                            PIPE_MAP_WRITE | PIPE_MAP_UNSYNCHRONIZED);
   assert(results);

   for (unsigned i = 0, e = qbuf->buf->b.b.width0 / sizeof(struct gfx11_sh_query_buffer_mem); i < e;
        ++i) {
      for (unsigned j = 0; j < 16; ++j)
         results[32 * i + j] = (uint64_t)1 << 63;
      results[32 * i + 16] = 0;
   }

   list_addtail(&qbuf->list, &sctx->shader_query_buffers);
   qbuf->head = 0;
   qbuf->refcount = sctx->num_active_shader_queries;

success:;
   struct pipe_shader_buffer sbuf;
   sbuf.buffer = &qbuf->buf->b.b;
   sbuf.buffer_offset = qbuf->head;
   sbuf.buffer_size = sizeof(struct gfx11_sh_query_buffer_mem);
   si_set_internal_shader_buffer(sctx, SI_GS_QUERY_BUF, &sbuf);
   SET_FIELD(sctx->current_gs_state, GS_STATE_STREAMOUT_QUERY_ENABLED, 1);

   si_mark_atom_dirty(sctx, &sctx->atoms.s.shader_query);
   return true;
}

static void gfx11_sh_query_destroy(struct si_context *sctx, struct si_query *rquery)
{
   struct gfx11_sh_query *query = (struct gfx11_sh_query *)rquery;
   gfx11_release_query_buffers(sctx, query->first, query->last);
   FREE(query);
}

static bool gfx11_sh_query_begin(struct si_context *sctx, struct si_query *rquery)
{
   struct gfx11_sh_query *query = (struct gfx11_sh_query *)rquery;

   gfx11_release_query_buffers(sctx, query->first, query->last);
   query->first = query->last = NULL;

   if (unlikely(!gfx11_alloc_query_buffer(sctx)))
      return false;

   query->first = list_last_entry(&sctx->shader_query_buffers, struct gfx11_sh_query_buffer, list);
   query->first_begin = query->first->head;

   sctx->num_active_shader_queries++;
   query->first->refcount++;

   return true;
}

static bool gfx11_sh_query_end(struct si_context *sctx, struct si_query *rquery)
{
   struct gfx11_sh_query *query = (struct gfx11_sh_query *)rquery;

   if (unlikely(!query->first))
      return false; /* earlier out of memory error */

   query->last = list_last_entry(&sctx->shader_query_buffers, struct gfx11_sh_query_buffer, list);
   query->last_end = query->last->head;

   /* Signal the fence of the previous chunk */
   if (query->last_end != 0) {
      uint64_t fence_va = query->last->buf->gpu_address;
      fence_va += query->last_end - sizeof(struct gfx11_sh_query_buffer_mem);
      fence_va += offsetof(struct gfx11_sh_query_buffer_mem, fence);
      si_cp_release_mem(sctx, &sctx->gfx_cs, V_028A90_BOTTOM_OF_PIPE_TS, 0, EOP_DST_SEL_MEM,
                        EOP_INT_SEL_NONE, EOP_DATA_SEL_VALUE_32BIT, query->last->buf, fence_va,
                        0xffffffff, PIPE_QUERY_GPU_FINISHED);
   }

   sctx->num_active_shader_queries--;

   if (sctx->num_active_shader_queries <= 0 || !si_is_atom_dirty(sctx, &sctx->atoms.s.shader_query)) {
      si_set_internal_shader_buffer(sctx, SI_GS_QUERY_BUF, NULL);
      SET_FIELD(sctx->current_gs_state, GS_STATE_STREAMOUT_QUERY_ENABLED, 0);

      /* If a query_begin is followed by a query_end without a draw
       * in-between, we need to clear the atom to ensure that the
       * next query_begin will re-initialize the shader buffer. */
      si_set_atom_dirty(sctx, &sctx->atoms.s.shader_query, false);
   }

   return true;
}

static void gfx11_sh_query_add_result(struct gfx11_sh_query *query,
                                      struct gfx11_sh_query_buffer_mem *qmem,
                                      union pipe_query_result *result)
{
   static const uint64_t mask = ((uint64_t)1 << 63) - 1;

   switch (query->b.type) {
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      result->u64 += qmem->stream[query->stream].emitted_primitives & mask;
      break;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
      result->u64 += qmem->stream[query->stream].generated_primitives & mask;
      break;
   case PIPE_QUERY_SO_STATISTICS:
      result->so_statistics.num_primitives_written +=
         qmem->stream[query->stream].emitted_primitives & mask;
      result->so_statistics.primitives_storage_needed +=
         qmem->stream[query->stream].generated_primitives & mask;
      break;
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
      result->b |= qmem->stream[query->stream].emitted_primitives !=
                   qmem->stream[query->stream].generated_primitives;
      break;
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      for (unsigned stream = 0; stream < SI_MAX_STREAMS; ++stream) {
         result->b |= qmem->stream[stream].emitted_primitives !=
                      qmem->stream[stream].generated_primitives;
      }
      break;
   default:
      assert(0);
   }
}

static bool gfx11_sh_query_get_result(struct si_context *sctx, struct si_query *rquery, bool wait,
                                      union pipe_query_result *result)
{
   struct gfx11_sh_query *query = (struct gfx11_sh_query *)rquery;

   util_query_clear_result(result, query->b.type);

   if (unlikely(!query->first))
      return false; /* earlier out of memory error */
   assert(query->last);

   for (struct gfx11_sh_query_buffer *qbuf = query->last;;
        qbuf = list_entry(qbuf->list.prev, struct gfx11_sh_query_buffer, list)) {
      unsigned usage = PIPE_MAP_READ | (wait ? 0 : PIPE_MAP_DONTBLOCK);
      void *map;

      if (rquery->b.flushed)
         map = sctx->ws->buffer_map(sctx->ws, qbuf->buf->buf, NULL, usage);
      else
         map = si_buffer_map(sctx, qbuf->buf, usage);

      if (!map)
         return false;

      unsigned results_begin = 0;
      unsigned results_end = qbuf->head;
      if (qbuf == query->first)
         results_begin = query->first_begin;
      if (qbuf == query->last)
         results_end = query->last_end;

      while (results_begin != results_end) {
         struct gfx11_sh_query_buffer_mem *qmem = map + results_begin;
         results_begin += sizeof(*qmem);

         gfx11_sh_query_add_result(query, qmem, result);
      }

      if (qbuf == query->first)
         break;
   }

   return true;
}

static void gfx11_sh_query_get_result_resource(struct si_context *sctx, struct si_query *rquery,
                                               enum pipe_query_flags flags,
                                               enum pipe_query_value_type result_type,
                                               int index, struct pipe_resource *resource,
                                               unsigned offset)
{
   struct gfx11_sh_query *query = (struct gfx11_sh_query *)rquery;
   struct si_qbo_state saved_state = {};
   struct pipe_resource *tmp_buffer = NULL;
   unsigned tmp_buffer_offset = 0;

   if (!sctx->sh_query_result_shader) {
      sctx->sh_query_result_shader = gfx11_create_sh_query_result_cs(sctx);
      if (!sctx->sh_query_result_shader)
         return;
   }

   if (query->first != query->last) {
      u_suballocator_alloc(&sctx->allocator_zeroed_memory, 16, 16, &tmp_buffer_offset, &tmp_buffer);
      if (!tmp_buffer)
         return;
   }

   si_save_qbo_state(sctx, &saved_state);

   /* Pre-fill the constants configuring the shader behavior. */
   struct {
      uint32_t config;
      uint32_t offset;
      uint32_t chain;
      uint32_t result_count;
   } consts;
   struct pipe_constant_buffer constant_buffer = {};

   if (index >= 0) {
      switch (query->b.type) {
      case PIPE_QUERY_PRIMITIVES_GENERATED:
         consts.offset = 4 * sizeof(uint64_t) * query->stream + 2 * sizeof(uint64_t);
         consts.config = 0;
         break;
      case PIPE_QUERY_PRIMITIVES_EMITTED:
         consts.offset = 4 * sizeof(uint64_t) * query->stream + 3 * sizeof(uint64_t);
         consts.config = 0;
         break;
      case PIPE_QUERY_SO_STATISTICS:
         consts.offset = sizeof(uint32_t) * (4 * index + query->stream);
         consts.config = 0;
         break;
      case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
         consts.offset = 4 * sizeof(uint64_t) * query->stream;
         consts.config = 2;
         break;
      case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
         consts.offset = 0;
         consts.config = 3;
         break;
      default:
         unreachable("bad query type");
      }
   } else {
      /* Check result availability. */
      consts.offset = 0;
      consts.config = 1;
   }

   if (result_type == PIPE_QUERY_TYPE_I64 || result_type == PIPE_QUERY_TYPE_U64)
      consts.config |= 8;

   constant_buffer.buffer_size = sizeof(consts);
   constant_buffer.user_buffer = &consts;

   /* Pre-fill the SSBOs and grid. */
   struct pipe_shader_buffer ssbo[3];
   struct pipe_grid_info grid = {};

   ssbo[1].buffer = tmp_buffer;
   ssbo[1].buffer_offset = tmp_buffer_offset;
   ssbo[1].buffer_size = 16;

   ssbo[2] = ssbo[1];

   grid.block[0] = 1;
   grid.block[1] = 1;
   grid.block[2] = 1;
   grid.grid[0] = 1;
   grid.grid[1] = 1;
   grid.grid[2] = 1;

   struct gfx11_sh_query_buffer *qbuf = query->first;
   for (;;) {
      unsigned begin = qbuf == query->first ? query->first_begin : 0;
      unsigned end = qbuf == query->last ? query->last_end : qbuf->buf->b.b.width0;
      if (!end)
         continue;

      ssbo[0].buffer = &qbuf->buf->b.b;
      ssbo[0].buffer_offset = begin;
      ssbo[0].buffer_size = end - begin;

      consts.result_count = (end - begin) / sizeof(struct gfx11_sh_query_buffer_mem);
      consts.chain = 0;
      if (qbuf != query->first)
         consts.chain |= 1;
      if (qbuf != query->last)
         consts.chain |= 2;

      if (qbuf == query->last) {
         ssbo[2].buffer = resource;
         ssbo[2].buffer_offset = offset;
         ssbo[2].buffer_size = 8;
      }

      sctx->b.set_constant_buffer(&sctx->b, PIPE_SHADER_COMPUTE, 0, false, &constant_buffer);

      if (flags & PIPE_QUERY_WAIT) {
         uint64_t va;

         /* Wait for result availability. Wait only for readiness
          * of the last entry, since the fence writes should be
          * serialized in the CP.
          */
         va = qbuf->buf->gpu_address;
         va += end - sizeof(struct gfx11_sh_query_buffer_mem);
         va += offsetof(struct gfx11_sh_query_buffer_mem, fence);

         si_cp_wait_mem(sctx, &sctx->gfx_cs, va, 0x00000001, 0x00000001, 0);
      }

      /* ssbo[2] is either tmp_buffer or resource */
      assert(ssbo[2].buffer);
      si_launch_grid_internal_ssbos(sctx, &grid, sctx->sh_query_result_shader,
                                    SI_OP_SYNC_PS_BEFORE | SI_OP_SYNC_AFTER, SI_COHERENCY_SHADER,
                                    3, ssbo, (1 << 2) | (ssbo[1].buffer ? 1 << 1 : 0));

      if (qbuf == query->last)
         break;
      qbuf = list_entry(qbuf->list.next, struct gfx11_sh_query_buffer, list);
   }

   si_restore_qbo_state(sctx, &saved_state);
   pipe_resource_reference(&tmp_buffer, NULL);
}

static const struct si_query_ops gfx11_sh_query_ops = {
   .destroy = gfx11_sh_query_destroy,
   .begin = gfx11_sh_query_begin,
   .end = gfx11_sh_query_end,
   .get_result = gfx11_sh_query_get_result,
   .get_result_resource = gfx11_sh_query_get_result_resource,
};

struct pipe_query *gfx11_sh_query_create(struct si_screen *screen, enum pipe_query_type query_type,
                                         unsigned index)
{
   struct gfx11_sh_query *query = CALLOC_STRUCT(gfx11_sh_query);
   if (unlikely(!query))
      return NULL;

   query->b.ops = &gfx11_sh_query_ops;
   query->b.type = query_type;
   query->stream = index;

   return (struct pipe_query *)query;
}

void si_gfx11_init_query(struct si_context *sctx)
{
   list_inithead(&sctx->shader_query_buffers);
   sctx->atoms.s.shader_query.emit = emit_shader_query;
}

void si_gfx11_destroy_query(struct si_context *sctx)
{
   if (!sctx->shader_query_buffers.next)
      return;

   while (!list_is_empty(&sctx->shader_query_buffers)) {
      struct gfx11_sh_query_buffer *qbuf =
         list_first_entry(&sctx->shader_query_buffers, struct gfx11_sh_query_buffer, list);
      list_del(&qbuf->list);

      assert(!qbuf->refcount);
      si_resource_reference(&qbuf->buf, NULL);
      FREE(qbuf);
   }
}
