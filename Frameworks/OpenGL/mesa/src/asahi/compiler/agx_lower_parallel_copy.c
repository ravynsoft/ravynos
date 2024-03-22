/*
 * Copyright 2022 Alyssa Rosenzweig
 * Copyright 2021 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "agx_builder.h"
#include "agx_compiler.h"

/*
 * Emits code for
 *
 *    for (int i = 0; i < n; ++i)
 *       registers[dests[i]] = registers[srcs[i]];
 *
 * ...with all copies happening in parallel.
 *
 * That is, emit machine instructions equivalent to a parallel copy. This is
 * used to lower not only parallel copies but also collects and splits, which
 * also have parallel copy semantics.
 *
 * We only handles register-register copies, not general agx_index sources. This
 * suffices for its internal use for register allocation.
 */
static void
do_copy(agx_builder *b, const struct agx_copy *copy)
{
   agx_index dst = agx_register(copy->dest, copy->src.size);

   if (copy->src.type == AGX_INDEX_IMMEDIATE)
      agx_mov_imm_to(b, dst, copy->src.value);
   else
      agx_mov_to(b, dst, copy->src);
}

static void
do_swap(agx_builder *b, const struct agx_copy *copy)
{
   assert(copy->src.type == AGX_INDEX_REGISTER && "only GPRs are swapped");

   if (copy->dest == copy->src.value)
      return;

   /* We can swap lo/hi halves of a 32-bit register with a 32-bit extr */
   if (copy->src.size == AGX_SIZE_16 &&
       (copy->dest >> 1) == (copy->src.value >> 1)) {

      assert(((copy->dest & 1) == (1 - (copy->src.value & 1))) &&
             "no trivial swaps, and only 2 halves of a register");

      /* r0 = extr r0, r0, #16
       *    = (((r0 << 32) | r0) >> 16) & 0xFFFFFFFF
       *    = (((r0 << 32) >> 16) & 0xFFFFFFFF) | (r0 >> 16)
       *    = (r0l << 16) | r0h
       */
      agx_index reg32 = agx_register(copy->dest & ~1, AGX_SIZE_32);
      agx_extr_to(b, reg32, reg32, reg32, agx_immediate(16), 0);
      return;
   }

   agx_index x = agx_register(copy->dest, copy->src.size);
   agx_index y = copy->src;

   agx_xor_to(b, x, x, y);
   agx_xor_to(b, y, x, y);
   agx_xor_to(b, x, x, y);
}

struct copy_ctx {
   /* Number of copies being processed */
   unsigned entry_count;

   /* For each physreg, the number of pending copy entries that use it as a
    * source. Once this drops to zero, then the physreg is unblocked and can
    * be moved to.
    */
   unsigned physreg_use_count[AGX_NUM_REGS];

   /* For each physreg, the pending copy_entry that uses it as a dest. */
   struct agx_copy *physreg_dest[AGX_NUM_REGS];

   struct agx_copy entries[AGX_NUM_REGS];
};

static bool
entry_blocked(struct agx_copy *entry, struct copy_ctx *ctx)
{
   for (unsigned i = 0; i < agx_size_align_16(entry->src.size); i++) {
      if (ctx->physreg_use_count[entry->dest + i] != 0)
         return true;
   }

   return false;
}

static bool
is_real(struct agx_copy *entry)
{
   return entry->src.type == AGX_INDEX_REGISTER;
}

/* TODO: Generalize to other bit sizes */
static void
split_32bit_copy(struct copy_ctx *ctx, struct agx_copy *entry)
{
   assert(!entry->done);
   assert(is_real(entry));
   assert(agx_size_align_16(entry->src.size) == 2);
   struct agx_copy *new_entry = &ctx->entries[ctx->entry_count++];

   new_entry->dest = entry->dest + 1;
   new_entry->src = entry->src;
   new_entry->src.value += 1;
   new_entry->done = false;
   entry->src.size = AGX_SIZE_16;
   new_entry->src.size = AGX_SIZE_16;
   ctx->physreg_dest[entry->dest + 1] = new_entry;
}

void
agx_emit_parallel_copies(agx_builder *b, struct agx_copy *copies,
                         unsigned num_copies)
{
   /* First, lower away 64-bit copies to smaller chunks, since we don't have
    * 64-bit ALU so we always want to split.
    */
   struct agx_copy *copies2 = calloc(sizeof(copies[0]), num_copies * 2);
   unsigned num_copies2 = 0;

   for (unsigned i = 0; i < num_copies; ++i) {
      struct agx_copy copy = copies[i];

      if (copy.src.size == AGX_SIZE_64) {
         copy.src.size = AGX_SIZE_32;
         copies2[num_copies2++] = copy;

         if (copy.src.type == AGX_INDEX_IMMEDIATE) {
            static_assert(sizeof(copy.src.value) * 8 == 32, "known size");
            copy.src.value = 0;
         } else {
            assert(copy.src.type == AGX_INDEX_REGISTER ||
                   copy.src.type == AGX_INDEX_UNIFORM);

            copy.src.value += 2;
         }

         copy.dest += 2;
         copies2[num_copies2++] = copy;
      } else {
         copies2[num_copies2++] = copy;
      }
   }

   copies = copies2;
   num_copies = num_copies2;

   /* Set up the bookkeeping */
   struct copy_ctx _ctx = {.entry_count = num_copies};
   struct copy_ctx *ctx = &_ctx;

   memset(ctx->physreg_dest, 0, sizeof(ctx->physreg_dest));
   memset(ctx->physreg_use_count, 0, sizeof(ctx->physreg_use_count));

   for (unsigned i = 0; i < ctx->entry_count; i++) {
      struct agx_copy *entry = &copies[i];

      ctx->entries[i] = *entry;

      for (unsigned j = 0; j < agx_size_align_16(entry->src.size); j++) {
         if (is_real(entry))
            ctx->physreg_use_count[entry->src.value + j]++;

         /* Copies should not have overlapping destinations. */
         assert(!ctx->physreg_dest[entry->dest + j]);
         ctx->physreg_dest[entry->dest + j] = &ctx->entries[i];
      }
   }

   /* Try to vectorize aligned 16-bit copies to use 32-bit operations instead */
   for (unsigned i = 0; i < ctx->entry_count; i++) {
      struct agx_copy *entry = &ctx->entries[i];
      if (entry->src.size != AGX_SIZE_16)
         continue;

      if ((entry->dest & 1) || (entry->src.value & 1))
         continue;

      if (entry->src.type != AGX_INDEX_UNIFORM &&
          entry->src.type != AGX_INDEX_REGISTER)
         continue;

      unsigned next_dest = entry->dest + 1;
      assert(next_dest < ARRAY_SIZE(ctx->physreg_dest) && "aligned reg");

      struct agx_copy *next_copy = ctx->physreg_dest[next_dest];
      if (!next_copy)
         continue;

      assert(next_copy->dest == next_dest && "data structure invariant");
      assert(next_copy->src.size == AGX_SIZE_16 && "unaligned copy");

      if (next_copy->src.type != entry->src.type)
         continue;

      if (next_copy->src.value != (entry->src.value + 1))
         continue;

      /* Vectorize the copies */
      ctx->physreg_dest[next_dest] = entry;
      entry->src.size = AGX_SIZE_32;
      next_copy->done = true;
   }

   bool progress = true;
   while (progress) {
      progress = false;

      /* Step 1: resolve paths in the transfer graph. This means finding
       * copies whose destination aren't blocked by something else and then
       * emitting them, continuing this process until every copy is blocked
       * and there are only cycles left.
       *
       * TODO: We should note that src is also available in dest to unblock
       * cycles that src is involved in.
       */

      for (unsigned i = 0; i < ctx->entry_count; i++) {
         struct agx_copy *entry = &ctx->entries[i];
         if (!entry->done && !entry_blocked(entry, ctx)) {
            entry->done = true;
            progress = true;
            do_copy(b, entry);
            for (unsigned j = 0; j < agx_size_align_16(entry->src.size); j++) {
               if (is_real(entry))
                  ctx->physreg_use_count[entry->src.value + j]--;
               ctx->physreg_dest[entry->dest + j] = NULL;
            }
         }
      }

      if (progress)
         continue;

      /* Step 2: Find partially blocked copies and split them. In the
       * mergedregs case, we can 32-bit copies which are only blocked on one
       * 16-bit half, and splitting them helps get things moving.
       *
       * We can skip splitting copies if the source isn't a register,
       * however, because it does not unblock anything and therefore doesn't
       * contribute to making forward progress with step 1. These copies
       * should still be resolved eventually in step 1 because they can't be
       * part of a cycle.
       */
      for (unsigned i = 0; i < ctx->entry_count; i++) {
         struct agx_copy *entry = &ctx->entries[i];
         if (entry->done || (agx_size_align_16(entry->src.size) != 2))
            continue;

         if (((ctx->physreg_use_count[entry->dest] == 0 ||
               ctx->physreg_use_count[entry->dest + 1] == 0)) &&
             is_real(entry)) {
            split_32bit_copy(ctx, entry);
            progress = true;
         }
      }
   }

   /* Step 3: resolve cycles through swapping.
    *
    * At this point, the transfer graph should consist of only cycles.
    * The reason is that, given any physreg n_1 that's the source of a
    * remaining entry, it has a destination n_2, which (because every
    * copy is blocked) is the source of some other copy whose destination
    * is n_3, and so we can follow the chain until we get a cycle. If we
    * reached some other node than n_1:
    *
    *  n_1 -> n_2 -> ... -> n_i
    *          ^             |
    *          |-------------|
    *
    *  then n_2 would be the destination of 2 copies, which is illegal
    *  (checked above in an assert). So n_1 must be part of a cycle:
    *
    *  n_1 -> n_2 -> ... -> n_i
    *  ^                     |
    *  |---------------------|
    *
    *  and this must be only cycle n_1 is involved in, because any other
    *  path starting from n_1 would also have to end in n_1, resulting in
    *  a node somewhere along the way being the destination of 2 copies
    *  when the 2 paths merge.
    *
    *  The way we resolve the cycle is through picking a copy (n_1, n_2)
    *  and swapping n_1 and n_2. This moves n_1 to n_2, so n_2 is taken
    *  out of the cycle:
    *
    *  n_1 -> ... -> n_i
    *  ^              |
    *  |--------------|
    *
    *  and we can keep repeating this until the cycle is empty.
    */

   for (unsigned i = 0; i < ctx->entry_count; i++) {
      struct agx_copy *entry = &ctx->entries[i];
      if (entry->done)
         continue;

      assert(is_real(entry));

      /* catch trivial copies */
      if (entry->dest == entry->src.value) {
         entry->done = true;
         continue;
      }

      do_swap(b, entry);

      /* Split any blocking copies whose sources are only partially
       * contained within our destination.
       */
      if (agx_size_align_16(entry->src.size) == 1) {
         for (unsigned j = 0; j < ctx->entry_count; j++) {
            struct agx_copy *blocking = &ctx->entries[j];

            if (blocking->done)
               continue;

            if (blocking->src.value <= entry->dest &&
                blocking->src.value + 1 >= entry->dest &&
                agx_size_align_16(blocking->src.size) == 2) {
               split_32bit_copy(ctx, blocking);
            }
         }
      }

      /* Update sources of blocking copies.
       *
       * Note: at this point, every blocking copy's source should be
       * contained within our destination.
       */
      for (unsigned j = 0; j < ctx->entry_count; j++) {
         struct agx_copy *blocking = &ctx->entries[j];
         if (blocking->src.value >= entry->dest &&
             blocking->src.value <
                entry->dest + agx_size_align_16(entry->src.size)) {
            blocking->src.value =
               entry->src.value + (blocking->src.value - entry->dest);
         }
      }

      entry->done = true;
   }

   free(copies2);
}
