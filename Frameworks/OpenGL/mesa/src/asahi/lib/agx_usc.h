/*
 * Copyright 2022 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#ifndef AGX_USC_H
#define AGX_USC_H

#include "asahi/lib/agx_pack.h"
#include "asahi/lib/pool.h"

/* Opaque structure representing a USC program being constructed */
struct agx_usc_builder {
   struct agx_ptr T;
   uint8_t *head;

#ifndef NDEBUG
   size_t size;
#endif
};

static struct agx_usc_builder
agx_alloc_usc_control(struct agx_pool *pool, unsigned num_reg_bindings)
{
   STATIC_ASSERT(AGX_USC_UNIFORM_HIGH_LENGTH == AGX_USC_UNIFORM_LENGTH);
   STATIC_ASSERT(AGX_USC_TEXTURE_LENGTH == AGX_USC_UNIFORM_LENGTH);
   STATIC_ASSERT(AGX_USC_SAMPLER_LENGTH == AGX_USC_UNIFORM_LENGTH);

   size_t size = AGX_USC_UNIFORM_LENGTH * num_reg_bindings;

   size += AGX_USC_SHARED_LENGTH;
   size += AGX_USC_SHADER_LENGTH;
   size += AGX_USC_REGISTERS_LENGTH;
   size += MAX2(AGX_USC_NO_PRESHADER_LENGTH, AGX_USC_PRESHADER_LENGTH);
   size += AGX_USC_FRAGMENT_PROPERTIES_LENGTH;

   struct agx_usc_builder b = {
      .T = agx_pool_alloc_aligned(pool, size, 64),

#ifndef NDEBUG
      .size = size,
#endif
   };

   b.head = (uint8_t *)b.T.cpu;

   return b;
}

static bool
agx_usc_builder_validate(struct agx_usc_builder *b, size_t size)
{
#ifndef NDEBUG
   assert(((b->head - (uint8_t *)b->T.cpu) + size) <= b->size);
#endif

   return true;
}

#define agx_usc_pack(b, struct_name, template)                                 \
   for (bool it =                                                              \
           agx_usc_builder_validate((b), AGX_USC_##struct_name##_LENGTH);      \
        it; it = false, (b)->head += AGX_USC_##struct_name##_LENGTH)           \
      agx_pack((b)->head, USC_##struct_name, template)

static void
agx_usc_uniform(struct agx_usc_builder *b, unsigned start_halfs,
                unsigned size_halfs, uint64_t buffer)
{
   assert((start_halfs + size_halfs) <= (1 << 9) && "uniform file overflow");
   assert(size_halfs <= 64 && "caller's responsibility to split");

   if (start_halfs & BITFIELD_BIT(8)) {
      agx_usc_pack(b, UNIFORM_HIGH, cfg) {
         cfg.start_halfs = start_halfs & BITFIELD_MASK(8);
         cfg.size_halfs = size_halfs;
         cfg.buffer = buffer;
      }
   } else {
      agx_usc_pack(b, UNIFORM, cfg) {
         cfg.start_halfs = start_halfs;
         cfg.size_halfs = size_halfs;
         cfg.buffer = buffer;
      }
   }
}

static uint32_t
agx_usc_fini(struct agx_usc_builder *b)
{
   assert(b->T.gpu <= (1ull << 32) && "pipelines must be in low memory");
   return b->T.gpu;
}

static void
agx_usc_shared_none(struct agx_usc_builder *b)
{
   agx_usc_pack(b, SHARED, cfg) {
      cfg.layout = AGX_SHARED_LAYOUT_VERTEX_COMPUTE;
      cfg.bytes_per_threadgroup = 65536;
   }
}

#endif
