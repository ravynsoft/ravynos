/*
 * Copyright (c) 2012-2015 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Wladimir J. van der Laan <laanwj@gmail.com>
 */

#ifndef H_ETNA_EMIT
#define H_ETNA_EMIT

#include "etnaviv_screen.h"
#include "etnaviv_util.h"
#include "hw/cmdstream.xml.h"

struct etna_context;

struct etna_coalesce {
   uint32_t start;
   uint32_t last_reg;
   uint32_t last_fixp;
};

static inline void
etna_emit_load_state(struct etna_cmd_stream *stream, const uint16_t offset,
                     const uint16_t count, const int fixp)
{
   uint32_t v;

   v = VIV_FE_LOAD_STATE_HEADER_OP_LOAD_STATE |
       COND(fixp, VIV_FE_LOAD_STATE_HEADER_FIXP) |
       VIV_FE_LOAD_STATE_HEADER_OFFSET(offset) |
       (VIV_FE_LOAD_STATE_HEADER_COUNT(count) &
        VIV_FE_LOAD_STATE_HEADER_COUNT__MASK);

   etna_cmd_stream_emit(stream, v);
}

static inline void
etna_set_state(struct etna_cmd_stream *stream, uint32_t address, uint32_t value)
{
   etna_cmd_stream_reserve(stream, 2);
   etna_emit_load_state(stream, address >> 2, 1, 0);
   etna_cmd_stream_emit(stream, value);
}

static inline void
etna_set_state_reloc(struct etna_cmd_stream *stream, uint32_t address,
                     const struct etna_reloc *reloc)
{
   etna_cmd_stream_reserve(stream, 2);
   etna_emit_load_state(stream, address >> 2, 1, 0);
   etna_cmd_stream_reloc(stream, reloc);
}

static inline void
etna_set_state_multi(struct etna_cmd_stream *stream, uint32_t base,
                     uint32_t num, const uint32_t *values)
{
   if (num == 0)
      return;

   etna_cmd_stream_reserve(stream, 1 + num + 1); /* 1 extra for potential alignment */
   etna_emit_load_state(stream, base >> 2, num, 0);

   for (uint32_t i = 0; i < num; i++)
      etna_cmd_stream_emit(stream, values[i]);

   /* add potential padding */
   if ((num % 2) == 0)
      etna_cmd_stream_emit(stream, 0);
}

void
etna_stall(struct etna_cmd_stream *stream, uint32_t from, uint32_t to);

static inline void
etna_draw_primitives(struct etna_cmd_stream *stream, uint32_t primitive_type,
                     uint32_t start, uint32_t count)
{
   etna_cmd_stream_reserve(stream, 4);

   etna_cmd_stream_emit(stream, VIV_FE_DRAW_PRIMITIVES_HEADER_OP_DRAW_PRIMITIVES);
   etna_cmd_stream_emit(stream, primitive_type);
   etna_cmd_stream_emit(stream, start);
   etna_cmd_stream_emit(stream, count);
}

static inline void
etna_draw_indexed_primitives(struct etna_cmd_stream *stream,
                             uint32_t primitive_type, uint32_t start,
                             uint32_t count, uint32_t offset)
{
   etna_cmd_stream_reserve(stream, 5 + 1);

   etna_cmd_stream_emit(stream, VIV_FE_DRAW_INDEXED_PRIMITIVES_HEADER_OP_DRAW_INDEXED_PRIMITIVES);
   etna_cmd_stream_emit(stream, primitive_type);
   etna_cmd_stream_emit(stream, start);
   etna_cmd_stream_emit(stream, count);
   etna_cmd_stream_emit(stream, offset);
   etna_cmd_stream_emit(stream, 0);
}

/* important: this takes a vertex count, not a primitive count */
static inline void
etna_draw_instanced(struct etna_cmd_stream *stream,
                    uint32_t indexed, uint32_t primitive_type,
                    uint32_t instance_count,
                    uint32_t vertex_count, uint32_t offset)
{
   etna_cmd_stream_reserve(stream, 3 + 1);
   etna_cmd_stream_emit(stream,
      VIV_FE_DRAW_INSTANCED_HEADER_OP_DRAW_INSTANCED |
      COND(indexed, VIV_FE_DRAW_INSTANCED_HEADER_INDEXED) |
      VIV_FE_DRAW_INSTANCED_HEADER_TYPE(primitive_type) |
      VIV_FE_DRAW_INSTANCED_HEADER_INSTANCE_COUNT_LO(instance_count & 0xffff));
   etna_cmd_stream_emit(stream,
      VIV_FE_DRAW_INSTANCED_COUNT_INSTANCE_COUNT_HI(instance_count >> 16) |
      VIV_FE_DRAW_INSTANCED_COUNT_VERTEX_COUNT(vertex_count));
   etna_cmd_stream_emit(stream,
      VIV_FE_DRAW_INSTANCED_START_INDEX(offset));
   etna_cmd_stream_emit(stream, 0);
}

static inline void
etna_coalesce_start(struct etna_cmd_stream *stream,
                    struct etna_coalesce *coalesce)
{
   coalesce->start = etna_cmd_stream_offset(stream);
   coalesce->last_reg = 0;
   coalesce->last_fixp = 0;
}

static inline void
etna_coalesce_end(struct etna_cmd_stream *stream,
                  struct etna_coalesce *coalesce)
{
   uint32_t end = etna_cmd_stream_offset(stream);
   uint32_t size = end - coalesce->start;

   if (size) {
      uint32_t offset = coalesce->start - 1;
      uint32_t value = etna_cmd_stream_get(stream, offset);

      value |= VIV_FE_LOAD_STATE_HEADER_COUNT(size);
      etna_cmd_stream_set(stream, offset, value);
   }

   /* append needed padding */
   if (end % 2 == 1)
      etna_cmd_stream_emit(stream, 0xdeadbeef);
}

static inline void
check_coalsence(struct etna_cmd_stream *stream, struct etna_coalesce *coalesce,
                uint32_t reg, uint32_t fixp)
{
   if (coalesce->last_reg != 0) {
      if (((coalesce->last_reg + 4) != reg) || (coalesce->last_fixp != fixp)) {
         etna_coalesce_end(stream, coalesce);
         etna_emit_load_state(stream, reg >> 2, 0, fixp);
         coalesce->start = etna_cmd_stream_offset(stream);
      }
   } else {
      etna_emit_load_state(stream, reg >> 2, 0, fixp);
      coalesce->start = etna_cmd_stream_offset(stream);
   }

   coalesce->last_reg = reg;
   coalesce->last_fixp = fixp;
}

static inline void
etna_coalsence_emit(struct etna_cmd_stream *stream,
                    struct etna_coalesce *coalesce, uint32_t reg,
                    uint32_t value)
{
   check_coalsence(stream, coalesce, reg, 0);
   etna_cmd_stream_emit(stream, value);
}

static inline void
etna_coalsence_emit_fixp(struct etna_cmd_stream *stream,
                         struct etna_coalesce *coalesce, uint32_t reg,
                         uint32_t value)
{
   check_coalsence(stream, coalesce, reg, 1);
   etna_cmd_stream_emit(stream, value);
}

static inline void
etna_coalsence_emit_reloc(struct etna_cmd_stream *stream,
                          struct etna_coalesce *coalesce, uint32_t reg,
                          const struct etna_reloc *r)
{
   if (r->bo) {
      check_coalsence(stream, coalesce, reg, 0);
      etna_cmd_stream_reloc(stream, r);
   }
}

void
etna_emit_state(struct etna_context *ctx);

#endif
