/*
 * Copyright 2022 Alyssa Rosenzweig
 * Copyright 2021 Collabora, Ltd.
 * Copyright 2014 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "util/compiler.h"
#include "agx_builder.h"

#define XXH_INLINE_ALL
#include "util/xxhash.h"

/*
 * This pass handles CSE'ing repeated expressions created in the process of
 * translating from NIR. Also, currently this is intra-block only, to make it
 * work over multiple block we'd need to bring forward dominance calculation.
 */

static inline uint32_t
HASH(uint32_t hash, unsigned data)
{
   return XXH32(&data, sizeof(data), hash);
}

/* Hash an instruction. XXH32 isn't too speedy, so this is hotspot. */
static uint32_t
hash_instr(const void *data)
{
   const agx_instr *I = data;
   uint32_t hash = 0;

   /* Explicitly skip destinations, except for size and type */
   agx_foreach_dest(I, d) {
      hash = HASH(hash, ((uint32_t)I->dest[d].type) |
                           (((uint32_t)I->dest[d].size) << 16));
   }

   /* Hash the source array as-is */
   hash = XXH32(I->src, sizeof(agx_index) * I->nr_srcs, hash);

   /* Hash everything else in the instruction starting from the opcode */
   hash = XXH32(&I->op, sizeof(agx_instr) - offsetof(agx_instr, op), hash);

   return hash;
}

static bool
instrs_equal(const void *_i1, const void *_i2)
{
   const agx_instr *i1 = _i1, *i2 = _i2;

   if (i1->op != i2->op)
      return false;
   if (i1->nr_srcs != i2->nr_srcs)
      return false;
   if (i1->nr_dests != i2->nr_dests)
      return false;

   /* Explicitly skip everything but size and type */
   agx_foreach_dest(i1, d) {
      if (i1->dest[d].type != i2->dest[d].type)
         return false;
      if (i1->dest[d].size != i2->dest[d].size)
         return false;
   }

   agx_foreach_src(i1, s) {
      agx_index s1 = i1->src[s], s2 = i2->src[s];

      if (memcmp(&s1, &s2, sizeof(s1)) != 0)
         return false;
   }

   if (i1->imm != i2->imm)
      return false;
   if (i1->invert_cond != i2->invert_cond)
      return false;
   if (i1->dim != i2->dim)
      return false;
   if (i1->offset != i2->offset)
      return false;
   if (i1->shadow != i2->shadow)
      return false;
   if (i1->shift != i2->shift)
      return false;
   if (i1->saturate != i2->saturate)
      return false;
   if (i1->mask != i2->mask)
      return false;

   return true;
}

/* Determines what instructions the above routines have to handle */
static bool
instr_can_cse(const agx_instr *I)
{
   return agx_opcodes_info[I->op].can_eliminate &&
          agx_opcodes_info[I->op].can_reorder;
}

void
agx_opt_cse(agx_context *ctx)
{
   struct set *instr_set = _mesa_set_create(NULL, hash_instr, instrs_equal);

   agx_foreach_block(ctx, block) {
      agx_index *replacement = calloc(sizeof(agx_index), ctx->alloc);
      _mesa_set_clear(instr_set, NULL);

      agx_foreach_instr_in_block(block, instr) {
         /* Rewrite as we go so we converge locally in 1 iteration */
         agx_foreach_ssa_src(instr, s) {
            agx_index repl = replacement[instr->src[s].value];
            if (!agx_is_null(repl))
               agx_replace_src(instr, s, repl);
         }

         if (!instr_can_cse(instr))
            continue;

         bool found;
         struct set_entry *entry =
            _mesa_set_search_or_add(instr_set, instr, &found);
         if (found) {
            const agx_instr *match = entry->key;

            agx_foreach_dest(instr, d) {
               replacement[instr->dest[d].value] = match->dest[d];
            }
         }
      }

      free(replacement);
   }

   _mesa_set_destroy(instr_set, NULL);
}
