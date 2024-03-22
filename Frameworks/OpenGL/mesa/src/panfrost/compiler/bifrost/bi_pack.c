/*
 * Copyright (C) 2020 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "bi_quirks.h"
#include "compiler.h"

/* This file contains the final passes of the compiler. Running after
 * scheduling and RA, the IR is now finalized, so we need to emit it to actual
 * bits on the wire (as well as fixup branches) */

static uint64_t
bi_pack_header(bi_clause *clause, bi_clause *next_1, bi_clause *next_2)
{
   /* next_dependencies are the union of the dependencies of successors'
    * dependencies */

   unsigned dependency_wait = next_1 ? next_1->dependencies : 0;
   dependency_wait |= next_2 ? next_2->dependencies : 0;

   /* Signal barriers (slot #7) immediately. This is not optimal but good
    * enough. Doing better requires extending the IR and scheduler.
    */
   if (clause->message_type == BIFROST_MESSAGE_BARRIER)
      dependency_wait |= BITFIELD_BIT(7);

   bool staging_barrier = next_1 ? next_1->staging_barrier : false;
   staging_barrier |= next_2 ? next_2->staging_barrier : 0;

   struct bifrost_header header = {
      .flow_control = (next_1 == NULL && next_2 == NULL) ? BIFROST_FLOW_END
                                                         : clause->flow_control,
      .terminate_discarded_threads = clause->td,
      .next_clause_prefetch = clause->next_clause_prefetch && next_1,
      .staging_barrier = staging_barrier,
      .staging_register = clause->staging_register,
      .dependency_wait = dependency_wait,
      .dependency_slot = clause->scoreboard_id,
      .message_type = clause->message_type,
      .next_message_type = next_1 ? next_1->message_type : 0,
      .flush_to_zero = clause->ftz ? BIFROST_FTZ_ALWAYS : BIFROST_FTZ_DISABLE,
   };

   uint64_t u = 0;
   memcpy(&u, &header, sizeof(header));
   return u;
}

/* Assigns a slot for reading, before anything is written */

static void
bi_assign_slot_read(bi_registers *regs, bi_index src)
{
   /* We only assign for registers */
   if (src.type != BI_INDEX_REGISTER)
      return;

   /* Check if we already assigned the slot */
   for (unsigned i = 0; i <= 1; ++i) {
      if (regs->slot[i] == src.value && regs->enabled[i])
         return;
   }

   if (regs->slot[2] == src.value && regs->slot23.slot2 == BIFROST_OP_READ)
      return;

   /* Assign it now */

   for (unsigned i = 0; i <= 1; ++i) {
      if (!regs->enabled[i]) {
         regs->slot[i] = src.value;
         regs->enabled[i] = true;
         return;
      }
   }

   if (!regs->slot23.slot3) {
      regs->slot[2] = src.value;
      regs->slot23.slot2 = BIFROST_OP_READ;
      return;
   }

   bi_print_slots(regs, stderr);
   unreachable("Failed to find a free slot for src");
}

static bi_registers
bi_assign_slots(bi_tuple *now, bi_tuple *prev)
{
   /* We assign slots for the main register mechanism. Special ops
    * use the data registers, which has its own mechanism entirely
    * and thus gets skipped over here. */

   bool read_dreg = now->add && bi_opcode_props[now->add->op].sr_read;
   bool write_dreg = prev->add && bi_opcode_props[prev->add->op].sr_write;

   /* First, assign reads */

   if (now->fma)
      bi_foreach_src(now->fma, src)
         bi_assign_slot_read(&now->regs, (now->fma)->src[src]);

   if (now->add) {
      bi_foreach_src(now->add, src) {
         /* This is not a real source, we shouldn't assign a
          * slot for it.
          */
         if (now->add->op == BI_OPCODE_BLEND && src == 4)
            continue;

         if (!(src == 0 && read_dreg))
            bi_assign_slot_read(&now->regs, (now->add)->src[src]);
      }
   }

   /* Next, assign writes. Staging writes are assigned separately, but
    * +ATEST wants its destination written to both a staging register
    * _and_ a regular write, because it may not generate a message */

   if (prev->add && prev->add->nr_dests &&
       (!write_dreg || prev->add->op == BI_OPCODE_ATEST)) {
      bi_index idx = prev->add->dest[0];

      if (idx.type == BI_INDEX_REGISTER) {
         now->regs.slot[3] = idx.value;
         now->regs.slot23.slot3 = BIFROST_OP_WRITE;
      }
   }

   if (prev->fma && prev->fma->nr_dests) {
      bi_index idx = prev->fma->dest[0];

      if (idx.type == BI_INDEX_REGISTER) {
         if (now->regs.slot23.slot3) {
            /* Scheduler constraint: cannot read 3 and write 2 */
            assert(!now->regs.slot23.slot2);
            now->regs.slot[2] = idx.value;
            now->regs.slot23.slot2 = BIFROST_OP_WRITE;
         } else {
            now->regs.slot[3] = idx.value;
            now->regs.slot23.slot3 = BIFROST_OP_WRITE;
            now->regs.slot23.slot3_fma = true;
         }
      }
   }

   return now->regs;
}

static enum bifrost_reg_mode
bi_pack_register_mode(bi_registers r)
{
   /* Handle idle as a special case */
   if (!(r.slot23.slot2 | r.slot23.slot3))
      return r.first_instruction ? BIFROST_IDLE_1 : BIFROST_IDLE;

   /* Otherwise, use the LUT */
   for (unsigned i = 0; i < ARRAY_SIZE(bifrost_reg_ctrl_lut); ++i) {
      if (memcmp(bifrost_reg_ctrl_lut + i, &r.slot23, sizeof(r.slot23)) == 0)
         return i;
   }

   bi_print_slots(&r, stderr);
   unreachable("Invalid slot assignment");
}

static uint64_t
bi_pack_registers(bi_registers regs)
{
   enum bifrost_reg_mode mode = bi_pack_register_mode(regs);
   struct bifrost_regs s = {0};
   uint64_t packed = 0;

   /* Need to pack 5-bit mode as a 4-bit field. The decoder moves bit 3 to bit 4
    * for first instruction and adds 16 when reg 2 == reg 3 */

   unsigned ctrl;
   bool r2_equals_r3 = false;

   if (regs.first_instruction) {
      /* Bit 3 implicitly must be clear for first instructions.
       * The affected patterns all write both ADD/FMA, but that
       * is forbidden for the last instruction (whose writes are
       * encoded by the first), so this does not add additional
       * encoding constraints */
      assert(!(mode & 0x8));

      /* Move bit 4 to bit 3, since bit 3 is clear */
      ctrl = (mode & 0x7) | ((mode & 0x10) >> 1);

      /* If we can let r2 equal r3, we have to or the hardware raises
       * INSTR_INVALID_ENC (it's unclear why). */
      if (!(regs.slot23.slot2 && regs.slot23.slot3))
         r2_equals_r3 = true;
   } else {
      /* We force r2=r3 or not for the upper bit */
      ctrl = (mode & 0xF);
      r2_equals_r3 = (mode & 0x10);
   }

   if (regs.enabled[1]) {
      /* Gotta save that bit!~ Required by the 63-x trick */
      assert(regs.slot[1] > regs.slot[0]);
      assert(regs.enabled[0]);

      /* Do the 63-x trick, see docs/disasm */
      if (regs.slot[0] > 31) {
         regs.slot[0] = 63 - regs.slot[0];
         regs.slot[1] = 63 - regs.slot[1];
      }

      assert(regs.slot[0] <= 31);
      assert(regs.slot[1] <= 63);

      s.ctrl = ctrl;
      s.reg1 = regs.slot[1];
      s.reg0 = regs.slot[0];
   } else {
      /* slot 1 disabled, so set to zero and use slot 1 for ctrl */
      s.ctrl = 0;
      s.reg1 = ctrl << 2;

      if (regs.enabled[0]) {
         /* Bit 0 upper bit of slot 0 */
         s.reg1 |= (regs.slot[0] >> 5);

         /* Rest of slot 0 in usual spot */
         s.reg0 = (regs.slot[0] & 0b11111);
      } else {
         /* Bit 1 set if slot 0 also disabled */
         s.reg1 |= (1 << 1);
      }
   }

   /* Force r2 =/!= r3 as needed */
   if (r2_equals_r3) {
      assert(regs.slot[3] == regs.slot[2] ||
             !(regs.slot23.slot2 && regs.slot23.slot3));

      if (regs.slot23.slot2)
         regs.slot[3] = regs.slot[2];
      else
         regs.slot[2] = regs.slot[3];
   } else if (!regs.first_instruction) {
      /* Enforced by the encoding anyway */
      assert(regs.slot[2] != regs.slot[3]);
   }

   s.reg2 = regs.slot[2];
   s.reg3 = regs.slot[3];
   s.fau_idx = regs.fau_idx;

   memcpy(&packed, &s, sizeof(s));
   return packed;
}

/* We must ensure slot 1 > slot 0 for the 63-x trick to function, so we fix
 * this up at pack time. (Scheduling doesn't care.) */

static void
bi_flip_slots(bi_registers *regs)
{
   if (regs->enabled[0] && regs->enabled[1] && regs->slot[1] < regs->slot[0]) {
      unsigned temp = regs->slot[0];
      regs->slot[0] = regs->slot[1];
      regs->slot[1] = temp;
   }
}

static inline enum bifrost_packed_src
bi_get_src_slot(bi_registers *regs, unsigned reg)
{
   if (regs->slot[0] == reg && regs->enabled[0])
      return BIFROST_SRC_PORT0;
   else if (regs->slot[1] == reg && regs->enabled[1])
      return BIFROST_SRC_PORT1;
   else if (regs->slot[2] == reg && regs->slot23.slot2 == BIFROST_OP_READ)
      return BIFROST_SRC_PORT2;
   else
      unreachable("Tried to access register with no port");
}

static inline enum bifrost_packed_src
bi_get_src_new(bi_instr *ins, bi_registers *regs, unsigned s)
{
   if (!ins || s >= ins->nr_srcs)
      return 0;

   bi_index src = ins->src[s];

   if (src.type == BI_INDEX_REGISTER)
      return bi_get_src_slot(regs, src.value);
   else if (src.type == BI_INDEX_PASS)
      return src.value;
   else {
      /* TODO make safer */
      return BIFROST_SRC_STAGE;
   }
}

static struct bi_packed_tuple
bi_pack_tuple(bi_clause *clause, bi_tuple *tuple, bi_tuple *prev,
              bool first_tuple, gl_shader_stage stage)
{
   bi_assign_slots(tuple, prev);
   tuple->regs.fau_idx = tuple->fau_idx;
   tuple->regs.first_instruction = first_tuple;

   bi_flip_slots(&tuple->regs);

   bool sr_read = tuple->add && bi_opcode_props[(tuple->add)->op].sr_read;

   uint64_t reg = bi_pack_registers(tuple->regs);
   uint64_t fma =
      bi_pack_fma(tuple->fma, bi_get_src_new(tuple->fma, &tuple->regs, 0),
                  bi_get_src_new(tuple->fma, &tuple->regs, 1),
                  bi_get_src_new(tuple->fma, &tuple->regs, 2),
                  bi_get_src_new(tuple->fma, &tuple->regs, 3));

   uint64_t add = bi_pack_add(
      tuple->add, bi_get_src_new(tuple->add, &tuple->regs, sr_read + 0),
      bi_get_src_new(tuple->add, &tuple->regs, sr_read + 1),
      bi_get_src_new(tuple->add, &tuple->regs, sr_read + 2), 0);

   if (tuple->add) {
      bi_instr *add = tuple->add;

      bool sr_write =
         bi_opcode_props[add->op].sr_write && !bi_is_null(add->dest[0]);

      if (sr_read && !bi_is_null(add->src[0])) {
         assert(add->src[0].type == BI_INDEX_REGISTER);
         clause->staging_register = add->src[0].value;

         if (sr_write)
            assert(bi_is_equiv(add->src[0], add->dest[0]));
      } else if (sr_write) {
         assert(add->dest[0].type == BI_INDEX_REGISTER);
         clause->staging_register = add->dest[0].value;
      }
   }

   struct bi_packed_tuple packed = {
      .lo = reg | (fma << 35) | ((add & 0b111111) << 58),
      .hi = add >> 6,
   };

   return packed;
}

/* A block contains at most one PC-relative constant, from a terminal branch.
 * Find the last instruction and if it is a relative branch, fix up the
 * PC-relative constant to contain the absolute offset. This occurs at pack
 * time instead of schedule time because the number of quadwords between each
 * block is not known until after all other passes have finished.
 */

static void
bi_assign_branch_offset(bi_context *ctx, bi_block *block)
{
   if (list_is_empty(&block->clauses))
      return;

   bi_clause *clause = list_last_entry(&block->clauses, bi_clause, link);
   bi_instr *br = bi_last_instr_in_clause(clause);

   if (!br->branch_target)
      return;

   /* Put it in the high place */
   int32_t qwords = bi_block_offset(ctx, clause, br->branch_target);
   int32_t bytes = qwords * 16;

   /* Copy so we can toy with the sign without undefined behaviour */
   uint32_t raw = 0;
   memcpy(&raw, &bytes, sizeof(raw));

   /* Clear off top bits for A1/B1 bits */
   raw &= ~0xF0000000;

   /* Put in top 32-bits */
   assert(clause->pcrel_idx < 8);
   clause->constants[clause->pcrel_idx] |= ((uint64_t)raw) << 32ull;
}

static void
bi_pack_constants(unsigned tuple_count, uint64_t *constants, unsigned word_idx,
                  unsigned constant_words, bool ec0_packed,
                  struct util_dynarray *emission)
{
   unsigned index = (word_idx << 1) + ec0_packed;

   /* Do more constants follow */
   bool more = (word_idx + 1) < constant_words;

   /* Indexed first by tuple count and second by constant word number,
    * indicates the position in the clause */
   unsigned pos_lookup[8][3] = {
      {0}, {1}, {3}, {2, 5}, {4, 8}, {7, 11, 14}, {6, 10, 13}, {9, 12},
   };

   /* Compute the pos, and check everything is reasonable */
   assert((tuple_count - 1) < 8);
   assert(word_idx < 3);
   unsigned pos = pos_lookup[tuple_count - 1][word_idx];
   assert(pos != 0 || (tuple_count == 1 && word_idx == 0));

   struct bifrost_fmt_constant quad = {
      .pos = pos,
      .tag = more ? BIFROST_FMTC_CONSTANTS : BIFROST_FMTC_FINAL,
      .imm_1 = constants[index + 0] >> 4,
      .imm_2 = constants[index + 1] >> 4,
   };

   util_dynarray_append(emission, struct bifrost_fmt_constant, quad);
}

uint8_t
bi_pack_literal(enum bi_clause_subword literal)
{
   assert(literal >= BI_CLAUSE_SUBWORD_LITERAL_0);
   assert(literal <= BI_CLAUSE_SUBWORD_LITERAL_7);

   return (literal - BI_CLAUSE_SUBWORD_LITERAL_0);
}

static inline uint8_t
bi_clause_upper(unsigned val, struct bi_packed_tuple *tuples,
                ASSERTED unsigned tuple_count)
{
   assert(val < tuple_count);

   /* top 3-bits of 78-bits is tuple >> 75 == (tuple >> 64) >> 11 */
   struct bi_packed_tuple tuple = tuples[val];
   return (tuple.hi >> 11);
}

uint8_t
bi_pack_upper(enum bi_clause_subword upper, struct bi_packed_tuple *tuples,
              ASSERTED unsigned tuple_count)
{
   assert(upper >= BI_CLAUSE_SUBWORD_UPPER_0);
   assert(upper <= BI_CLAUSE_SUBWORD_UPPER_7);

   return bi_clause_upper(upper - BI_CLAUSE_SUBWORD_UPPER_0, tuples,
                          tuple_count);
}

uint64_t
bi_pack_tuple_bits(enum bi_clause_subword idx, struct bi_packed_tuple *tuples,
                   ASSERTED unsigned tuple_count, unsigned offset,
                   unsigned nbits)
{
   assert(idx >= BI_CLAUSE_SUBWORD_TUPLE_0);
   assert(idx <= BI_CLAUSE_SUBWORD_TUPLE_7);

   unsigned val = (idx - BI_CLAUSE_SUBWORD_TUPLE_0);
   assert(val < tuple_count);

   struct bi_packed_tuple tuple = tuples[val];

   assert(offset + nbits < 78);
   assert(nbits <= 64);

   /* (X >> start) & m
    * = (((hi << 64) | lo) >> start) & m
    * = (((hi << 64) >> start) | (lo >> start)) & m
    * = { ((hi << (64 - start)) | (lo >> start)) & m if start <= 64
    *   { ((hi >> (start - 64)) | (lo >> start)) & m if start >= 64
    * = { ((hi << (64 - start)) & m) | ((lo >> start) & m) if start <= 64
    *   { ((hi >> (start - 64)) & m) | ((lo >> start) & m) if start >= 64
    *
    * By setting m = 2^64 - 1, we justify doing the respective shifts as
    * 64-bit integers. Zero special cased to avoid undefined behaviour.
    */

   uint64_t lo = (tuple.lo >> offset);
   uint64_t hi = (offset == 0)   ? 0
                 : (offset > 64) ? (tuple.hi >> (offset - 64))
                                 : (tuple.hi << (64 - offset));

   return (lo | hi) & ((1ULL << nbits) - 1);
}

static inline uint16_t
bi_pack_lu(enum bi_clause_subword word, struct bi_packed_tuple *tuples,
           ASSERTED unsigned tuple_count)
{
   return (word >= BI_CLAUSE_SUBWORD_UPPER_0)
             ? bi_pack_upper(word, tuples, tuple_count)
             : bi_pack_literal(word);
}

uint8_t
bi_pack_sync(enum bi_clause_subword t1, enum bi_clause_subword t2,
             enum bi_clause_subword t3, struct bi_packed_tuple *tuples,
             ASSERTED unsigned tuple_count, bool z)
{
   uint8_t sync = (bi_pack_lu(t3, tuples, tuple_count) << 0) |
                  (bi_pack_lu(t2, tuples, tuple_count) << 3);

   if (t1 == BI_CLAUSE_SUBWORD_Z)
      sync |= z << 6;
   else
      sync |= bi_pack_literal(t1) << 6;

   return sync;
}

static inline uint64_t
bi_pack_t_ec(enum bi_clause_subword word, struct bi_packed_tuple *tuples,
             ASSERTED unsigned tuple_count, uint64_t ec0)
{
   if (word == BI_CLAUSE_SUBWORD_CONSTANT)
      return ec0;
   else
      return bi_pack_tuple_bits(word, tuples, tuple_count, 0, 60);
}

static uint32_t
bi_pack_subwords_56(enum bi_clause_subword t, struct bi_packed_tuple *tuples,
                    ASSERTED unsigned tuple_count, uint64_t header,
                    uint64_t ec0, unsigned tuple_subword)
{
   switch (t) {
   case BI_CLAUSE_SUBWORD_HEADER:
      return (header & ((1 << 30) - 1));
   case BI_CLAUSE_SUBWORD_RESERVED:
      return 0;
   case BI_CLAUSE_SUBWORD_CONSTANT:
      return (ec0 >> 15) & ((1 << 30) - 1);
   default:
      return bi_pack_tuple_bits(t, tuples, tuple_count, tuple_subword * 15, 30);
   }
}

static uint16_t
bi_pack_subword(enum bi_clause_subword t, unsigned format,
                struct bi_packed_tuple *tuples, ASSERTED unsigned tuple_count,
                uint64_t header, uint64_t ec0, unsigned m0,
                unsigned tuple_subword)
{
   switch (t) {
   case BI_CLAUSE_SUBWORD_HEADER:
      return header >> 30;
   case BI_CLAUSE_SUBWORD_M:
      return m0;
   case BI_CLAUSE_SUBWORD_CONSTANT:
      return (format == 5 || format == 10) ? (ec0 & ((1 << 15) - 1))
                                           : (ec0 >> (15 + 30));
   case BI_CLAUSE_SUBWORD_UPPER_23:
      return (bi_clause_upper(2, tuples, tuple_count) << 12) |
             (bi_clause_upper(3, tuples, tuple_count) << 9);
   case BI_CLAUSE_SUBWORD_UPPER_56:
      return (bi_clause_upper(5, tuples, tuple_count) << 12) |
             (bi_clause_upper(6, tuples, tuple_count) << 9);
   case BI_CLAUSE_SUBWORD_UPPER_0 ... BI_CLAUSE_SUBWORD_UPPER_7:
      return bi_pack_upper(t, tuples, tuple_count) << 12;
   default:
      return bi_pack_tuple_bits(t, tuples, tuple_count, tuple_subword * 15, 15);
   }
}

/* EC0 is 60-bits (bottom 4 already shifted off) */
void
bi_pack_format(struct util_dynarray *emission, unsigned index,
               struct bi_packed_tuple *tuples, ASSERTED unsigned tuple_count,
               uint64_t header, uint64_t ec0, unsigned m0, bool z)
{
   struct bi_clause_format format = bi_clause_formats[index];

   uint8_t sync = bi_pack_sync(format.tag_1, format.tag_2, format.tag_3, tuples,
                               tuple_count, z);

   uint64_t s0_s3 = bi_pack_t_ec(format.s0_s3, tuples, tuple_count, ec0);

   uint16_t s4 = bi_pack_subword(format.s4, format.format, tuples, tuple_count,
                                 header, ec0, m0, 4);

   uint32_t s5_s6 =
      bi_pack_subwords_56(format.s5_s6, tuples, tuple_count, header, ec0,
                          (format.format == 2 || format.format == 7) ? 0 : 3);

   uint64_t s7 = bi_pack_subword(format.s7, format.format, tuples, tuple_count,
                                 header, ec0, m0, 2);

   /* Now that subwords are packed, split into 64-bit halves and emit */
   uint64_t lo = sync | ((s0_s3 & ((1ull << 56) - 1)) << 8);
   uint64_t hi = (s0_s3 >> 56) | ((uint64_t)s4 << 4) | ((uint64_t)s5_s6 << 19) |
                 ((uint64_t)s7 << 49);

   util_dynarray_append(emission, uint64_t, lo);
   util_dynarray_append(emission, uint64_t, hi);
}

static void
bi_pack_clause(bi_context *ctx, bi_clause *clause, bi_clause *next_1,
               bi_clause *next_2, struct util_dynarray *emission,
               gl_shader_stage stage)
{
   struct bi_packed_tuple ins[8] = {0};

   for (unsigned i = 0; i < clause->tuple_count; ++i) {
      unsigned prev = ((i == 0) ? clause->tuple_count : i) - 1;
      ins[i] = bi_pack_tuple(clause, &clause->tuples[i], &clause->tuples[prev],
                             i == 0, stage);

      bi_instr *add = clause->tuples[i].add;

      /* Different GPUs support different forms of the CLPER.i32
       * instruction. Check we use the right one for the target.
       */
      if (add && add->op == BI_OPCODE_CLPER_OLD_I32)
         assert(ctx->quirks & BIFROST_LIMITED_CLPER);
      else if (add && add->op == BI_OPCODE_CLPER_I32)
         assert(!(ctx->quirks & BIFROST_LIMITED_CLPER));
   }

   bool ec0_packed = bi_ec0_packed(clause->tuple_count);

   if (ec0_packed)
      clause->constant_count = MAX2(clause->constant_count, 1);

   unsigned constant_quads =
      DIV_ROUND_UP(clause->constant_count - (ec0_packed ? 1 : 0), 2);

   uint64_t header = bi_pack_header(clause, next_1, next_2);
   uint64_t ec0 = (clause->constants[0] >> 4);
   unsigned m0 = (clause->pcrel_idx == 0) ? 4 : 0;

   unsigned counts[8] = {
      1, 2, 3, 3, 4, 5, 5, 6,
   };

   unsigned indices[8][6] = {
      {1},          {0, 2},           {0, 3, 4},        {0, 3, 6},
      {0, 3, 7, 8}, {0, 3, 5, 9, 10}, {0, 3, 5, 9, 11}, {0, 3, 5, 9, 12, 13},
   };

   unsigned count = counts[clause->tuple_count - 1];

   for (unsigned pos = 0; pos < count; ++pos) {
      ASSERTED unsigned idx = indices[clause->tuple_count - 1][pos];
      assert(bi_clause_formats[idx].pos == pos);
      assert((bi_clause_formats[idx].tag_1 == BI_CLAUSE_SUBWORD_Z) ==
             (pos == count - 1));

      /* Whether to end the clause immediately after the last tuple */
      bool z = (constant_quads == 0);

      bi_pack_format(emission, indices[clause->tuple_count - 1][pos], ins,
                     clause->tuple_count, header, ec0, m0, z);
   }

   /* Pack the remaining constants */

   for (unsigned pos = 0; pos < constant_quads; ++pos) {
      bi_pack_constants(clause->tuple_count, clause->constants, pos,
                        constant_quads, ec0_packed, emission);
   }
}

static void
bi_collect_blend_ret_addr(bi_context *ctx, struct util_dynarray *emission,
                          const bi_clause *clause)
{
   /* No need to collect return addresses when we're in a blend shader. */
   if (ctx->inputs->is_blend)
      return;

   const bi_tuple *tuple = &clause->tuples[clause->tuple_count - 1];
   const bi_instr *ins = tuple->add;

   if (!ins || ins->op != BI_OPCODE_BLEND)
      return;

   unsigned loc = tuple->regs.fau_idx - BIR_FAU_BLEND_0;
   assert(loc < ARRAY_SIZE(ctx->info.bifrost->blend));
   assert(!ctx->info.bifrost->blend[loc].return_offset);
   ctx->info.bifrost->blend[loc].return_offset =
      util_dynarray_num_elements(emission, uint8_t);
   assert(!(ctx->info.bifrost->blend[loc].return_offset & 0x7));
}

/*
 * The second register destination of TEXC_DUAL is encoded into the texture
 * operation descriptor during register allocation. It's dropped as late as
 * possible (instruction packing) so the register remains recorded in the IR,
 * for clause scoreboarding and so on.
 */
static void
bi_lower_texc_dual(bi_context *ctx)
{
   bi_foreach_instr_global(ctx, I) {
      if (I->op == BI_OPCODE_TEXC_DUAL) {
         /* In hardware, TEXC has 1 destination */
         I->op = BI_OPCODE_TEXC;
         bi_drop_dests(I, 1);
      }
   }
}

unsigned
bi_pack(bi_context *ctx, struct util_dynarray *emission)
{
   unsigned previous_size = emission->size;

   bi_lower_texc_dual(ctx);

   bi_foreach_block(ctx, block) {
      bi_assign_branch_offset(ctx, block);

      bi_foreach_clause_in_block(block, clause) {
         bool is_last = (clause->link.next == &block->clauses);

         /* Get the succeeding clauses, either two successors of
          * the block for the last clause in the block or just
          * the next clause within the block */

         bi_clause *next = NULL, *next_2 = NULL;

         if (is_last) {
            next = bi_next_clause(ctx, block->successors[0], NULL);
            next_2 = bi_next_clause(ctx, block->successors[1], NULL);
         } else {
            next = bi_next_clause(ctx, block, clause);
         }

         previous_size = emission->size;

         bi_pack_clause(ctx, clause, next, next_2, emission, ctx->stage);

         if (!is_last)
            bi_collect_blend_ret_addr(ctx, emission, clause);
      }
   }

   return emission->size - previous_size;
}
