/*
 * Copyright (C) 2020 Collabora Ltd.
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
 *
 * Authors (Collabora):
 *      Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

#include "bi_builder.h"
#include "compiler.h"

/* Arguments common to worklist, passed by value for convenience */

struct bi_worklist {
   /* # of instructions in the block */
   unsigned count;

   /* Instructions in the block */
   bi_instr **instructions;

   /* Bitset of instructions in the block ready for scheduling */
   BITSET_WORD *worklist;

   /* The backwards dependency graph. nr_dependencies is the number of
    * unscheduled instructions that must still be scheduled after (before)
    * this instruction. dependents are which instructions need to be
    * scheduled before (after) this instruction. */
   unsigned *dep_counts;
   BITSET_WORD **dependents;
};

/* State of a single tuple and clause under construction */

struct bi_reg_state {
   /* Number of register writes */
   unsigned nr_writes;

   /* Register reads, expressed as (equivalence classes of)
    * sources. Only 3 reads are allowed, but up to 2 may spill as
    * "forced" for the next scheduled tuple, provided such a tuple
    * can be constructed */
   bi_index reads[5];
   unsigned nr_reads;

   /* The previous tuple scheduled (= the next tuple executed in the
    * program) may require certain writes, in order to bypass the register
    * file and use a temporary passthrough for the value. Up to 2 such
    * constraints are architecturally satisfiable */
   unsigned forced_count;
   bi_index forceds[2];
};

struct bi_tuple_state {
   /* Is this the last tuple in the clause */
   bool last;

   /* Scheduled ADD instruction, or null if none */
   bi_instr *add;

   /* Reads for previous (succeeding) tuple */
   bi_index prev_reads[5];
   unsigned nr_prev_reads;
   bi_tuple *prev;

   /* Register slot state for current tuple */
   struct bi_reg_state reg;

   /* Constants are shared in the tuple. If constant_count is nonzero, it
    * is a size for constant count. Otherwise, fau is the slot read from
    * FAU, or zero if none is assigned. Ordinarily FAU slot 0 reads zero,
    * but within a tuple, that should be encoded as constant_count != 0
    * and constants[0] = constants[1] = 0 */
   unsigned constant_count;

   union {
      uint32_t constants[2];
      enum bir_fau fau;
   };

   unsigned pcrel_idx;
};

struct bi_const_state {
   unsigned constant_count;
   bool pcrel; /* applies to first const */
   uint32_t constants[2];

   /* Index of the constant into the clause */
   unsigned word_idx;
};

enum bi_ftz_state {
   /* No flush-to-zero state assigned yet */
   BI_FTZ_STATE_NONE,

   /* Never flush-to-zero */
   BI_FTZ_STATE_DISABLE,

   /* Always flush-to-zero */
   BI_FTZ_STATE_ENABLE,
};

/* At this point, pseudoinstructions have been lowered so sources/destinations
 * are limited to what's physically supported.
 */
#define BI_MAX_PHYS_SRCS  4
#define BI_MAX_PHYS_DESTS 2

struct bi_clause_state {
   /* Has a message-passing instruction already been assigned? */
   bool message;

   /* Indices already accessed, this needs to be tracked to avoid hazards
    * around message-passing instructions */
   unsigned access_count;
   bi_index accesses[(BI_MAX_PHYS_SRCS + BI_MAX_PHYS_DESTS) * 16];

   unsigned tuple_count;
   struct bi_const_state consts[8];

   /* Numerical state of the clause */
   enum bi_ftz_state ftz;
};

/* Determines messsage type by checking the table and a few special cases. Only
 * case missing is tilebuffer instructions that access depth/stencil, which
 * require a Z_STENCIL message (to implement
 * ARM_shader_framebuffer_fetch_depth_stencil) */

static enum bifrost_message_type
bi_message_type_for_instr(bi_instr *ins)
{
   enum bifrost_message_type msg = bi_opcode_props[ins->op].message;
   bool ld_var_special = (ins->op == BI_OPCODE_LD_VAR_SPECIAL);

   if (ld_var_special && ins->varying_name == BI_VARYING_NAME_FRAG_Z)
      return BIFROST_MESSAGE_Z_STENCIL;

   if (msg == BIFROST_MESSAGE_LOAD && ins->seg == BI_SEG_UBO)
      return BIFROST_MESSAGE_ATTRIBUTE;

   return msg;
}

/* Attribute, texture, and UBO load (attribute message) instructions support
 * bindless, so just check the message type */

ASSERTED static bool
bi_supports_dtsel(bi_instr *ins)
{
   switch (bi_message_type_for_instr(ins)) {
   case BIFROST_MESSAGE_ATTRIBUTE:
      return ins->op != BI_OPCODE_LD_GCLK_U64;
   case BIFROST_MESSAGE_TEX:
      return true;
   default:
      return false;
   }
}

/* Adds an edge to the dependency graph */

static void
bi_push_dependency(unsigned parent, unsigned child, BITSET_WORD **dependents,
                   unsigned *dep_counts)
{
   if (!BITSET_TEST(dependents[parent], child)) {
      BITSET_SET(dependents[parent], child);
      dep_counts[child]++;
   }
}

static void
add_dependency(struct util_dynarray *table, unsigned index, unsigned child,
               BITSET_WORD **dependents, unsigned *dep_counts)
{
   assert(index < 64);
   util_dynarray_foreach(table + index, unsigned, parent)
      bi_push_dependency(*parent, child, dependents, dep_counts);
}

static void
mark_access(struct util_dynarray *table, unsigned index, unsigned parent)
{
   assert(index < 64);
   util_dynarray_append(&table[index], unsigned, parent);
}

static bool
bi_is_sched_barrier(bi_instr *I)
{
   switch (I->op) {
   case BI_OPCODE_BARRIER:
   case BI_OPCODE_DISCARD_F32:
      return true;
   default:
      return false;
   }
}

static void
bi_create_dependency_graph(struct bi_worklist st, bool inorder, bool is_blend)
{
   struct util_dynarray last_read[64], last_write[64];

   for (unsigned i = 0; i < 64; ++i) {
      util_dynarray_init(&last_read[i], NULL);
      util_dynarray_init(&last_write[i], NULL);
   }

   /* Initialize dependency graph */
   for (unsigned i = 0; i < st.count; ++i) {
      st.dependents[i] = calloc(BITSET_WORDS(st.count), sizeof(BITSET_WORD));

      st.dep_counts[i] = 0;
   }

   unsigned prev_msg = ~0;

   /* Populate dependency graph */
   for (signed i = st.count - 1; i >= 0; --i) {
      bi_instr *ins = st.instructions[i];

      bi_foreach_src(ins, s) {
         if (ins->src[s].type != BI_INDEX_REGISTER)
            continue;
         unsigned count = bi_count_read_registers(ins, s);

         for (unsigned c = 0; c < count; ++c)
            add_dependency(last_write, ins->src[s].value + c, i, st.dependents,
                           st.dep_counts);
      }

      /* Keep message-passing ops in order. (This pass only cares
       * about bundling; reordering of message-passing instructions
       * happens during earlier scheduling.) */

      if (bi_message_type_for_instr(ins)) {
         if (prev_msg != ~0)
            bi_push_dependency(prev_msg, i, st.dependents, st.dep_counts);

         prev_msg = i;
      }

      /* Handle schedule barriers, adding All the deps */
      if (inorder || bi_is_sched_barrier(ins)) {
         for (unsigned j = 0; j < st.count; ++j) {
            if (i == j)
               continue;

            bi_push_dependency(MAX2(i, j), MIN2(i, j), st.dependents,
                               st.dep_counts);
         }
      }

      bi_foreach_dest(ins, d) {
         assert(ins->dest[d].type == BI_INDEX_REGISTER);
         unsigned dest = ins->dest[d].value;

         unsigned count = bi_count_write_registers(ins, d);

         for (unsigned c = 0; c < count; ++c) {
            add_dependency(last_read, dest + c, i, st.dependents,
                           st.dep_counts);
            add_dependency(last_write, dest + c, i, st.dependents,
                           st.dep_counts);
            mark_access(last_write, dest + c, i);
         }
      }

      /* Blend shaders are allowed to clobber R0-R15. Treat these
       * registers like extra destinations for scheduling purposes.
       */
      if (ins->op == BI_OPCODE_BLEND && !is_blend) {
         for (unsigned c = 0; c < 16; ++c) {
            add_dependency(last_read, c, i, st.dependents, st.dep_counts);
            add_dependency(last_write, c, i, st.dependents, st.dep_counts);
            mark_access(last_write, c, i);
         }
      }

      bi_foreach_src(ins, s) {
         if (ins->src[s].type != BI_INDEX_REGISTER)
            continue;

         unsigned count = bi_count_read_registers(ins, s);

         for (unsigned c = 0; c < count; ++c)
            mark_access(last_read, ins->src[s].value + c, i);
      }
   }

   /* If there is a branch, all instructions depend on it, as interblock
    * execution must be purely in-order */

   bi_instr *last = st.instructions[st.count - 1];
   if (last->branch_target || last->op == BI_OPCODE_JUMP) {
      for (signed i = st.count - 2; i >= 0; --i)
         bi_push_dependency(st.count - 1, i, st.dependents, st.dep_counts);
   }

   /* Free the intermediate structures */
   for (unsigned i = 0; i < 64; ++i) {
      util_dynarray_fini(&last_read[i]);
      util_dynarray_fini(&last_write[i]);
   }
}

/* Scheduler pseudoinstruction lowerings to enable instruction pairings.
 * Currently only support CUBEFACE -> *CUBEFACE1/+CUBEFACE2
 */

static bi_instr *
bi_lower_cubeface(bi_context *ctx, struct bi_clause_state *clause,
                  struct bi_tuple_state *tuple)
{
   bi_instr *pinstr = tuple->add;
   bi_builder b = bi_init_builder(ctx, bi_before_instr(pinstr));
   bi_instr *cubeface1 = bi_cubeface1_to(&b, pinstr->dest[0], pinstr->src[0],
                                         pinstr->src[1], pinstr->src[2]);

   pinstr->op = BI_OPCODE_CUBEFACE2;
   pinstr->dest[0] = pinstr->dest[1];
   bi_drop_dests(pinstr, 1);

   pinstr->src[0] = cubeface1->dest[0];
   bi_drop_srcs(pinstr, 1);

   return cubeface1;
}

/* Psuedo arguments are (rbase, address lo, address hi). We need *ATOM_C.i32 to
 * have the arguments (address lo, address hi, rbase), and +ATOM_CX to have the
 * arguments (rbase, address lo, address hi, rbase) */

static bi_instr *
bi_lower_atom_c(bi_context *ctx, struct bi_clause_state *clause,
                struct bi_tuple_state *tuple)
{
   bi_instr *pinstr = tuple->add;
   bi_builder b = bi_init_builder(ctx, bi_before_instr(pinstr));
   bi_instr *atom_c = bi_atom_c_return_i32(&b, pinstr->src[1], pinstr->src[2],
                                           pinstr->src[0], pinstr->atom_opc);

   if (bi_is_null(pinstr->dest[0]))
      atom_c->op = BI_OPCODE_ATOM_C_I32;

   bi_instr *atom_cx =
      bi_atom_cx_to(&b, pinstr->dest[0], pinstr->src[0], pinstr->src[1],
                    pinstr->src[2], pinstr->src[0], pinstr->sr_count);
   tuple->add = atom_cx;
   bi_remove_instruction(pinstr);

   return atom_c;
}

static bi_instr *
bi_lower_atom_c1(bi_context *ctx, struct bi_clause_state *clause,
                 struct bi_tuple_state *tuple)
{
   bi_instr *pinstr = tuple->add;
   bi_builder b = bi_init_builder(ctx, bi_before_instr(pinstr));
   bi_instr *atom_c = bi_atom_c1_return_i32(&b, pinstr->src[0], pinstr->src[1],
                                            pinstr->atom_opc);

   if (bi_is_null(pinstr->dest[0]))
      atom_c->op = BI_OPCODE_ATOM_C1_I32;

   bi_instr *atom_cx =
      bi_atom_cx_to(&b, pinstr->dest[0], bi_null(), pinstr->src[0],
                    pinstr->src[1], bi_dontcare(&b), pinstr->sr_count);
   tuple->add = atom_cx;
   bi_remove_instruction(pinstr);

   return atom_c;
}

static bi_instr *
bi_lower_seg_add(bi_context *ctx, struct bi_clause_state *clause,
                 struct bi_tuple_state *tuple)
{
   bi_instr *pinstr = tuple->add;
   bi_builder b = bi_init_builder(ctx, bi_before_instr(pinstr));

   bi_instr *fma = bi_seg_add_to(&b, pinstr->dest[0], pinstr->src[0],
                                 pinstr->preserve_null, pinstr->seg);

   pinstr->op = BI_OPCODE_SEG_ADD;
   pinstr->src[0] = pinstr->src[1];
   bi_drop_srcs(pinstr, 1);

   assert(pinstr->dest[0].type == BI_INDEX_REGISTER);
   pinstr->dest[0].value += 1;

   return fma;
}

static bi_instr *
bi_lower_dtsel(bi_context *ctx, struct bi_clause_state *clause,
               struct bi_tuple_state *tuple)
{
   bi_instr *add = tuple->add;
   bi_builder b = bi_init_builder(ctx, bi_before_instr(add));

   bi_instr *dtsel =
      bi_dtsel_imm_to(&b, bi_temp(b.shader), add->src[0], add->table);
   assert(add->nr_srcs >= 1);
   add->src[0] = dtsel->dest[0];

   assert(bi_supports_dtsel(add));
   return dtsel;
}

/* Flatten linked list to array for O(1) indexing */

static bi_instr **
bi_flatten_block(bi_block *block, unsigned *len)
{
   if (list_is_empty(&block->instructions))
      return NULL;

   *len = list_length(&block->instructions);
   bi_instr **instructions = malloc(sizeof(bi_instr *) * (*len));

   unsigned i = 0;

   bi_foreach_instr_in_block(block, ins)
      instructions[i++] = ins;

   return instructions;
}

/* The worklist would track instructions without outstanding dependencies. For
 * debug, force in-order scheduling (no dependency graph is constructed).
 */

static struct bi_worklist
bi_initialize_worklist(bi_block *block, bool inorder, bool is_blend)
{
   struct bi_worklist st = {};
   st.instructions = bi_flatten_block(block, &st.count);

   if (!st.count)
      return st;

   st.dependents = calloc(st.count, sizeof(st.dependents[0]));
   st.dep_counts = calloc(st.count, sizeof(st.dep_counts[0]));

   bi_create_dependency_graph(st, inorder, is_blend);
   st.worklist = calloc(BITSET_WORDS(st.count), sizeof(BITSET_WORD));

   for (unsigned i = 0; i < st.count; ++i) {
      if (st.dep_counts[i] == 0)
         BITSET_SET(st.worklist, i);
   }

   return st;
}

static void
bi_free_worklist(struct bi_worklist st)
{
   free(st.dep_counts);
   free(st.dependents);
   free(st.instructions);
   free(st.worklist);
}

static void
bi_update_worklist(struct bi_worklist st, unsigned idx)
{
   assert(st.dep_counts[idx] == 0);

   if (!st.dependents[idx])
      return;

   /* Iterate each dependent to remove one dependency (`done`),
    * adding dependents to the worklist where possible. */

   unsigned i;
   BITSET_FOREACH_SET(i, st.dependents[idx], st.count) {
      assert(st.dep_counts[i] != 0);
      unsigned new_deps = --st.dep_counts[i];

      if (new_deps == 0)
         BITSET_SET(st.worklist, i);
   }

   free(st.dependents[idx]);
}

/* Scheduler predicates */

/* IADDC.i32 can implement IADD.u32 if no saturation or swizzling is in use */
static bool
bi_can_iaddc(bi_instr *ins)
{
   return (ins->op == BI_OPCODE_IADD_U32 && !ins->saturate &&
           ins->src[0].swizzle == BI_SWIZZLE_H01 &&
           ins->src[1].swizzle == BI_SWIZZLE_H01);
}

/*
 * The encoding of *FADD.v2f16 only specifies a single abs flag. All abs
 * encodings are permitted by swapping operands; however, this scheme fails if
 * both operands are equal. Test for this case.
 */
static bool
bi_impacted_abs(bi_instr *I)
{
   return I->src[0].abs && I->src[1].abs &&
          bi_is_word_equiv(I->src[0], I->src[1]);
}

bool
bi_can_fma(bi_instr *ins)
{
   /* +IADD.i32 -> *IADDC.i32 */
   if (bi_can_iaddc(ins))
      return true;

   /* +MUX -> *CSEL */
   if (bi_can_replace_with_csel(ins))
      return true;

   /* *FADD.v2f16 has restricted abs modifiers, use +FADD.v2f16 instead */
   if (ins->op == BI_OPCODE_FADD_V2F16 && bi_impacted_abs(ins))
      return false;

   /* TODO: some additional fp16 constraints */
   return bi_opcode_props[ins->op].fma;
}

static bool
bi_impacted_fadd_widens(bi_instr *I)
{
   enum bi_swizzle swz0 = I->src[0].swizzle;
   enum bi_swizzle swz1 = I->src[1].swizzle;

   return (swz0 == BI_SWIZZLE_H00 && swz1 == BI_SWIZZLE_H11) ||
          (swz0 == BI_SWIZZLE_H11 && swz1 == BI_SWIZZLE_H11) ||
          (swz0 == BI_SWIZZLE_H11 && swz1 == BI_SWIZZLE_H00);
}

bool
bi_can_add(bi_instr *ins)
{
   /* +FADD.v2f16 lacks clamp modifier, use *FADD.v2f16 instead */
   if (ins->op == BI_OPCODE_FADD_V2F16 && ins->clamp)
      return false;

   /* +FCMP.v2f16 lacks abs modifier, use *FCMP.v2f16 instead */
   if (ins->op == BI_OPCODE_FCMP_V2F16 && (ins->src[0].abs || ins->src[1].abs))
      return false;

   /* +FADD.f32 has restricted widens, use +FADD.f32 for the full set */
   if (ins->op == BI_OPCODE_FADD_F32 && bi_impacted_fadd_widens(ins))
      return false;

   /* TODO: some additional fp16 constraints */
   return bi_opcode_props[ins->op].add;
}

/* Architecturally, no single instruction has a "not last" constraint. However,
 * pseudoinstructions writing multiple destinations (expanding to multiple
 * paired instructions) can run afoul of the "no two writes on the last clause"
 * constraint, so we check for that here.
 *
 * Exception to the exception: TEXC_DUAL, which writes to multiple sets of
 * staging registers. Staging registers bypass the usual register write
 * mechanism so this restriction does not apply.
 */

static bool
bi_must_not_last(bi_instr *ins)
{
   return (ins->nr_dests >= 2) && (ins->op != BI_OPCODE_TEXC_DUAL);
}

/* Check for a message-passing instruction. +DISCARD.f32 is special-cased; we
 * treat it as a message-passing instruction for the purpose of scheduling
 * despite no passing no logical message. Otherwise invalid encoding faults may
 * be raised for unknown reasons (possibly an errata).
 */

bool
bi_must_message(bi_instr *ins)
{
   return (bi_opcode_props[ins->op].message != BIFROST_MESSAGE_NONE) ||
          (ins->op == BI_OPCODE_DISCARD_F32);
}

static bool
bi_fma_atomic(enum bi_opcode op)
{
   switch (op) {
   case BI_OPCODE_ATOM_C_I32:
   case BI_OPCODE_ATOM_C_I64:
   case BI_OPCODE_ATOM_C1_I32:
   case BI_OPCODE_ATOM_C1_I64:
   case BI_OPCODE_ATOM_C1_RETURN_I32:
   case BI_OPCODE_ATOM_C1_RETURN_I64:
   case BI_OPCODE_ATOM_C_RETURN_I32:
   case BI_OPCODE_ATOM_C_RETURN_I64:
   case BI_OPCODE_ATOM_POST_I32:
   case BI_OPCODE_ATOM_POST_I64:
   case BI_OPCODE_ATOM_PRE_I64:
      return true;
   default:
      return false;
   }
}

bool
bi_reads_zero(bi_instr *ins)
{
   return !(bi_fma_atomic(ins->op) || ins->op == BI_OPCODE_IMULD);
}

bool
bi_reads_temps(bi_instr *ins, unsigned src)
{
   switch (ins->op) {
   /* Cannot permute a temporary */
   case BI_OPCODE_CLPER_I32:
   case BI_OPCODE_CLPER_OLD_I32:
      return src != 0;

   /* ATEST isn't supposed to be restricted, but in practice it always
    * wants to source its coverage mask input (source 0) from register 60,
    * which won't work properly if we put the input in a temp. This
    * requires workarounds in both RA and clause scheduling.
    */
   case BI_OPCODE_ATEST:
      return src != 0;

   case BI_OPCODE_IMULD:
      return false;
   default:
      return true;
   }
}

static bool
bi_impacted_t_modifiers(bi_instr *I, unsigned src)
{
   assert(src < I->nr_srcs);
   enum bi_swizzle swizzle = I->src[src].swizzle;

   switch (I->op) {
   case BI_OPCODE_F16_TO_F32:
   case BI_OPCODE_F16_TO_S32:
   case BI_OPCODE_F16_TO_U32:
   case BI_OPCODE_MKVEC_V2I16:
   case BI_OPCODE_S16_TO_F32:
   case BI_OPCODE_S16_TO_S32:
   case BI_OPCODE_U16_TO_F32:
   case BI_OPCODE_U16_TO_U32:
      return (swizzle != BI_SWIZZLE_H00);

   case BI_OPCODE_BRANCH_F32:
   case BI_OPCODE_LOGB_F32:
   case BI_OPCODE_ILOGB_F32:
   case BI_OPCODE_FADD_F32:
   case BI_OPCODE_FCMP_F32:
   case BI_OPCODE_FREXPE_F32:
   case BI_OPCODE_FREXPM_F32:
   case BI_OPCODE_FROUND_F32:
      return (swizzle != BI_SWIZZLE_H01);

   case BI_OPCODE_IADD_S32:
   case BI_OPCODE_IADD_U32:
   case BI_OPCODE_ISUB_S32:
   case BI_OPCODE_ISUB_U32:
   case BI_OPCODE_IADD_V4S8:
   case BI_OPCODE_IADD_V4U8:
   case BI_OPCODE_ISUB_V4S8:
   case BI_OPCODE_ISUB_V4U8:
      return (src == 1) && (swizzle != BI_SWIZZLE_H01);

   case BI_OPCODE_S8_TO_F32:
   case BI_OPCODE_S8_TO_S32:
   case BI_OPCODE_U8_TO_F32:
   case BI_OPCODE_U8_TO_U32:
      return (swizzle != BI_SWIZZLE_B0000);

   case BI_OPCODE_V2S8_TO_V2F16:
   case BI_OPCODE_V2S8_TO_V2S16:
   case BI_OPCODE_V2U8_TO_V2F16:
   case BI_OPCODE_V2U8_TO_V2U16:
      return (swizzle != BI_SWIZZLE_B0022);

   case BI_OPCODE_IADD_V2S16:
   case BI_OPCODE_IADD_V2U16:
   case BI_OPCODE_ISUB_V2S16:
   case BI_OPCODE_ISUB_V2U16:
      return (src == 1) && (swizzle >= BI_SWIZZLE_H11);

#if 0
        /* Restriction on IADD in 64-bit clauses on G72 */
        case BI_OPCODE_IADD_S64:
        case BI_OPCODE_IADD_U64:
                return (src == 1) && (swizzle != BI_SWIZZLE_D0);
#endif

   default:
      return false;
   }
}

bool
bi_reads_t(bi_instr *ins, unsigned src)
{
   /* Branch offset cannot come from passthrough */
   if (bi_opcode_props[ins->op].branch)
      return src != 2;

   /* Table can never read passthrough */
   if (bi_opcode_props[ins->op].table)
      return false;

   /* Staging register reads may happen before the succeeding register
    * block encodes a write, so effectively there is no passthrough */
   if (bi_is_staging_src(ins, src))
      return false;

   /* Bifrost cores newer than Mali G71 have restrictions on swizzles on
    * same-cycle temporaries. Check the list for these hazards. */
   if (bi_impacted_t_modifiers(ins, src))
      return false;

   /* Descriptor must not come from a passthrough */
   switch (ins->op) {
   case BI_OPCODE_LD_CVT:
   case BI_OPCODE_LD_TILE:
   case BI_OPCODE_ST_CVT:
   case BI_OPCODE_ST_TILE:
   case BI_OPCODE_TEXC:
   case BI_OPCODE_TEXC_DUAL:
      return src != 2;
   case BI_OPCODE_BLEND:
      return src != 2 && src != 3;

   /* +JUMP can't read the offset from T */
   case BI_OPCODE_JUMP:
      return false;

   /* Else, just check if we can read any temps */
   default:
      return bi_reads_temps(ins, src);
   }
}

/* Counts the number of 64-bit constants required by a clause. TODO: We
 * might want to account for merging, right now we overestimate, but
 * that's probably fine most of the time */

static unsigned
bi_nconstants(struct bi_clause_state *clause)
{
   unsigned count_32 = 0;

   for (unsigned i = 0; i < ARRAY_SIZE(clause->consts); ++i)
      count_32 += clause->consts[i].constant_count;

   return DIV_ROUND_UP(count_32, 2);
}

/* Would there be space for constants if we added one tuple? */

static bool
bi_space_for_more_constants(struct bi_clause_state *clause)
{
   return (bi_nconstants(clause) < 13 - (clause->tuple_count + 1));
}

/* Updates the FAU assignment for a tuple. A valid FAU assignment must be
 * possible (as a precondition), though not necessarily on the selected unit;
 * this is gauranteed per-instruction by bi_lower_fau and per-tuple by
 * bi_instr_schedulable */

static bool
bi_update_fau(struct bi_clause_state *clause, struct bi_tuple_state *tuple,
              bi_instr *instr, bool fma, bool destructive)
{
   /* Maintain our own constants, for nondestructive mode */
   uint32_t copied_constants[2], copied_count;
   unsigned *constant_count = &tuple->constant_count;
   uint32_t *constants = tuple->constants;
   enum bir_fau fau = tuple->fau;

   if (!destructive) {
      memcpy(copied_constants, tuple->constants,
             (*constant_count) * sizeof(constants[0]));
      copied_count = tuple->constant_count;

      constant_count = &copied_count;
      constants = copied_constants;
   }

   bi_foreach_src(instr, s) {
      bi_index src = instr->src[s];

      if (src.type == BI_INDEX_FAU) {
         bool no_constants = *constant_count == 0;
         bool no_other_fau = (fau == src.value) || !fau;
         bool mergable = no_constants && no_other_fau;

         if (destructive) {
            assert(mergable);
            tuple->fau = src.value;
         } else if (!mergable) {
            return false;
         }

         fau = src.value;
      } else if (src.type == BI_INDEX_CONSTANT) {
         /* No need to reserve space if we have a fast 0 */
         if (src.value == 0 && fma && bi_reads_zero(instr))
            continue;

         /* If there is a branch target, #0 by convention is the
          * PC-relative offset to the target */
         bool pcrel = instr->branch_target && src.value == 0;
         bool found = false;

         for (unsigned i = 0; i < *constant_count; ++i) {
            found |= (constants[i] == src.value) && (i != tuple->pcrel_idx);
         }

         /* pcrel constants are unique, so don't match */
         if (found && !pcrel)
            continue;

         bool no_fau = (*constant_count > 0) || !fau;
         bool mergable = no_fau && ((*constant_count) < 2);

         if (destructive) {
            assert(mergable);

            if (pcrel)
               tuple->pcrel_idx = *constant_count;
         } else if (!mergable)
            return false;

         constants[(*constant_count)++] = src.value;
      }
   }

   /* Constants per clause may be limited by tuple count */
   bool room_for_constants =
      (*constant_count == 0) || bi_space_for_more_constants(clause);

   if (destructive)
      assert(room_for_constants);
   else if (!room_for_constants)
      return false;

   return true;
}

/* Given an in-progress tuple, a candidate new instruction to add to the tuple,
 * and a source (index) from that candidate, determine whether this source is
 * "new", in the sense of requiring an additional read slot. That is, checks
 * whether the specified source reads from the register file via a read slot
 * (determined by its type and placement) and whether the source was already
 * specified by a prior read slot (to avoid double counting) */

static bool
bi_tuple_is_new_src(bi_instr *instr, struct bi_reg_state *reg, unsigned src_idx)
{
   assert(src_idx < instr->nr_srcs);
   bi_index src = instr->src[src_idx];

   /* Only consider sources which come from the register file */
   if (!(src.type == BI_INDEX_NORMAL || src.type == BI_INDEX_REGISTER))
      return false;

   /* Staging register reads bypass the usual register file mechanism */
   if (bi_is_staging_src(instr, src_idx))
      return false;

   /* If a source is already read in the tuple, it is already counted */
   for (unsigned t = 0; t < reg->nr_reads; ++t)
      if (bi_is_word_equiv(src, reg->reads[t]))
         return false;

   /* If a source is read in _this instruction_, it is already counted */
   for (unsigned t = 0; t < src_idx; ++t)
      if (bi_is_word_equiv(src, instr->src[t]))
         return false;

   return true;
}

/* Given two tuples in source order, count the number of register reads of the
 * successor, determined as the number of unique words accessed that aren't
 * written by the predecessor (since those are tempable).
 */

static unsigned
bi_count_succ_reads(bi_index t0, bi_index t1, bi_index *succ_reads,
                    unsigned nr_succ_reads)
{
   unsigned reads = 0;

   for (unsigned i = 0; i < nr_succ_reads; ++i) {
      bool unique = true;

      for (unsigned j = 0; j < i; ++j)
         if (bi_is_word_equiv(succ_reads[i], succ_reads[j]))
            unique = false;

      if (!unique)
         continue;

      if (bi_is_word_equiv(succ_reads[i], t0))
         continue;

      if (bi_is_word_equiv(succ_reads[i], t1))
         continue;

      reads++;
   }

   return reads;
}

/* Not all instructions can read from the staging passthrough (as determined by
 * reads_t), check if a given pair of instructions has such a restriction. Note
 * we also use this mechanism to prevent data races around staging register
 * reads, so we allow the input source to potentially be vector-valued */

static bool
bi_has_staging_passthrough_hazard(bi_index fma, bi_instr *add)
{
   bi_foreach_src(add, s) {
      bi_index src = add->src[s];

      if (src.type != BI_INDEX_REGISTER)
         continue;

      unsigned count = bi_count_read_registers(add, s);
      bool read = false;

      for (unsigned d = 0; d < count; ++d)
         read |= bi_is_equiv(fma, bi_register(src.value + d));

      if (read && !bi_reads_t(add, s))
         return true;
   }

   return false;
}

/* Likewise for cross-tuple passthrough (reads_temps) */

static bool
bi_has_cross_passthrough_hazard(bi_tuple *succ, bi_instr *ins)
{
   if (ins->nr_dests == 0)
      return false;

   bi_foreach_instr_in_tuple(succ, pins) {
      bi_foreach_src(pins, s) {
         if (bi_is_word_equiv(ins->dest[0], pins->src[s]) &&
             !bi_reads_temps(pins, s))
            return true;
      }
   }

   return false;
}

/* Is a register written other than the staging mechanism? ATEST is special,
 * writing to both a staging register and a regular register (fixed packing).
 * BLEND is special since it has to write r48 the normal way even if it never
 * gets read. This depends on liveness analysis, as a register is not needed
 * for a write that will be discarded after one tuple. */

static unsigned
bi_write_count(bi_instr *instr, uint64_t live_after_temp)
{
   if (instr->op == BI_OPCODE_ATEST || instr->op == BI_OPCODE_BLEND)
      return 1;

   unsigned count = 0;

   bi_foreach_dest(instr, d) {
      if (d == 0 && bi_opcode_props[instr->op].sr_write)
         continue;

      assert(instr->dest[0].type == BI_INDEX_REGISTER);
      if (live_after_temp & BITFIELD64_BIT(instr->dest[0].value))
         count++;
   }

   return count;
}

/*
 * Test if an instruction required flush-to-zero mode. Currently only supported
 * for f16<-->f32 conversions to implement fquantize16
 */
static bool
bi_needs_ftz(bi_instr *I)
{
   return (I->op == BI_OPCODE_F16_TO_F32 ||
           I->op == BI_OPCODE_V2F32_TO_V2F16) &&
          I->ftz;
}

/*
 * Test if an instruction would be numerically incompatible with the clause. At
 * present we only consider flush-to-zero modes.
 */
static bool
bi_numerically_incompatible(struct bi_clause_state *clause, bi_instr *instr)
{
   return (clause->ftz != BI_FTZ_STATE_NONE) &&
          ((clause->ftz == BI_FTZ_STATE_ENABLE) != bi_needs_ftz(instr));
}

/* Instruction placement entails two questions: what subset of instructions in
 * the block can legally be scheduled? and of those which is the best? That is,
 * we seek to maximize a cost function on a subset of the worklist satisfying a
 * particular predicate. The necessary predicate is determined entirely by
 * Bifrost's architectural limitations and is described in the accompanying
 * whitepaper. The cost function is a heuristic. */

static bool
bi_instr_schedulable(bi_instr *instr, struct bi_clause_state *clause,
                     struct bi_tuple_state *tuple, uint64_t live_after_temp,
                     bool fma)
{
   /* The units must match */
   if ((fma && !bi_can_fma(instr)) || (!fma && !bi_can_add(instr)))
      return false;

   /* There can only be one message-passing instruction per clause */
   if (bi_must_message(instr) && clause->message)
      return false;

   /* Some instructions have placement requirements */
   if (bi_opcode_props[instr->op].last && !tuple->last)
      return false;

   if (bi_must_not_last(instr) && tuple->last)
      return false;

   /* Numerical properties must be compatible with the clause */
   if (bi_numerically_incompatible(clause, instr))
      return false;

   /* Message-passing instructions are not guaranteed write within the
    * same clause (most likely they will not), so if a later instruction
    * in the clause accesses the destination, the message-passing
    * instruction can't be scheduled */
   if (bi_opcode_props[instr->op].sr_write) {
      bi_foreach_dest(instr, d) {
         unsigned nr = bi_count_write_registers(instr, d);
         assert(instr->dest[d].type == BI_INDEX_REGISTER);
         unsigned reg = instr->dest[d].value;

         for (unsigned i = 0; i < clause->access_count; ++i) {
            bi_index idx = clause->accesses[i];
            for (unsigned d = 0; d < nr; ++d) {
               if (bi_is_equiv(bi_register(reg + d), idx))
                  return false;
            }
         }
      }
   }

   if (bi_opcode_props[instr->op].sr_read && !bi_is_null(instr->src[0])) {
      unsigned nr = bi_count_read_registers(instr, 0);
      assert(instr->src[0].type == BI_INDEX_REGISTER);
      unsigned reg = instr->src[0].value;

      for (unsigned i = 0; i < clause->access_count; ++i) {
         bi_index idx = clause->accesses[i];
         for (unsigned d = 0; d < nr; ++d) {
            if (bi_is_equiv(bi_register(reg + d), idx))
               return false;
         }
      }
   }

   /* If FAU is already assigned, we may not disrupt that. Do a
    * non-disruptive test update */
   if (!bi_update_fau(clause, tuple, instr, fma, false))
      return false;

   /* If this choice of FMA would force a staging passthrough, the ADD
    * instruction must support such a passthrough */
   if (tuple->add && instr->nr_dests &&
       bi_has_staging_passthrough_hazard(instr->dest[0], tuple->add))
      return false;

   /* If this choice of destination would force a cross-tuple passthrough, the
    * next tuple must support that */
   if (tuple->prev && bi_has_cross_passthrough_hazard(tuple->prev, instr))
      return false;

   /* Register file writes are limited */
   unsigned total_writes = tuple->reg.nr_writes;
   total_writes += bi_write_count(instr, live_after_temp);

   /* Last tuple in a clause can only write a single value */
   if (tuple->last && total_writes > 1)
      return false;

   /* Register file reads are limited, so count unique */

   unsigned unique_new_srcs = 0;

   bi_foreach_src(instr, s) {
      if (bi_tuple_is_new_src(instr, &tuple->reg, s))
         unique_new_srcs++;
   }

   unsigned total_srcs = tuple->reg.nr_reads + unique_new_srcs;

   bool can_spill_to_moves = (!tuple->add);
   can_spill_to_moves &=
      (bi_nconstants(clause) < 13 - (clause->tuple_count + 2));
   can_spill_to_moves &= (clause->tuple_count < 7);

   /* However, we can get an extra 1 or 2 sources by inserting moves */
   if (total_srcs > (can_spill_to_moves ? 4 : 3))
      return false;

   /* Count effective reads for the successor */
   unsigned succ_reads = 0;

   if (instr->nr_dests) {
      bool has_t1 = tuple->add && tuple->add->nr_dests;
      succ_reads = bi_count_succ_reads(instr->dest[0],
                                       has_t1 ? tuple->add->dest[0] : bi_null(),
                                       tuple->prev_reads, tuple->nr_prev_reads);
   }

   /* Successor must satisfy R+W <= 4, so we require W <= 4-R */
   if ((signed)total_writes > (4 - (signed)succ_reads))
      return false;

   return true;
}

static signed
bi_instr_cost(bi_instr *instr, struct bi_tuple_state *tuple)
{
   signed cost = 0;

   /* Instructions that can schedule to either FMA or to ADD should be
    * deprioritized since they're easier to reschedule elsewhere */
   if (bi_can_fma(instr) && bi_can_add(instr))
      cost++;

   /* Message-passing instructions impose constraints on the registers
    * later in the clause, so schedule them as late within a clause as
    * possible (<==> prioritize them since we're backwards <==> decrease
    * cost) */
   if (bi_must_message(instr))
      cost--;

   /* Last instructions are big constraints (XXX: no effect on shader-db) */
   if (bi_opcode_props[instr->op].last)
      cost -= 2;

   return cost;
}

static unsigned
bi_choose_index(struct bi_worklist st, struct bi_clause_state *clause,
                struct bi_tuple_state *tuple, uint64_t live_after_temp,
                bool fma)
{
   unsigned i, best_idx = ~0;
   signed best_cost = INT_MAX;

   BITSET_FOREACH_SET(i, st.worklist, st.count) {
      bi_instr *instr = st.instructions[i];

      if (!bi_instr_schedulable(instr, clause, tuple, live_after_temp, fma))
         continue;

      signed cost = bi_instr_cost(instr, tuple);

      /* Tie break in favour of later instructions, under the
       * assumption this promotes temporary usage (reducing pressure
       * on the register file). This is a side effect of a prepass
       * scheduling for pressure. */

      if (cost <= best_cost) {
         best_idx = i;
         best_cost = cost;
      }
   }

   return best_idx;
}

static void
bi_pop_instr(struct bi_clause_state *clause, struct bi_tuple_state *tuple,
             bi_instr *instr, uint64_t live_after_temp, bool fma)
{
   bi_update_fau(clause, tuple, instr, fma, true);

   assert(clause->access_count + instr->nr_srcs + instr->nr_dests <=
          ARRAY_SIZE(clause->accesses));

   memcpy(clause->accesses + clause->access_count, instr->src,
          sizeof(instr->src[0]) * instr->nr_srcs);
   clause->access_count += instr->nr_srcs;

   memcpy(clause->accesses + clause->access_count, instr->dest,
          sizeof(instr->dest[0]) * instr->nr_dests);
   clause->access_count += instr->nr_dests;

   tuple->reg.nr_writes += bi_write_count(instr, live_after_temp);

   bi_foreach_src(instr, s) {
      if (bi_tuple_is_new_src(instr, &tuple->reg, s))
         tuple->reg.reads[tuple->reg.nr_reads++] = instr->src[s];
   }

   /* This could be optimized to allow pairing integer instructions with
    * special flush-to-zero instructions, but punting on this until we have
    * a workload that cares.
    */
   clause->ftz =
      bi_needs_ftz(instr) ? BI_FTZ_STATE_ENABLE : BI_FTZ_STATE_DISABLE;
}

/* Choose the best instruction and pop it off the worklist. Returns NULL if no
 * instruction is available. This function is destructive. */

static bi_instr *
bi_take_instr(bi_context *ctx, struct bi_worklist st,
              struct bi_clause_state *clause, struct bi_tuple_state *tuple,
              uint64_t live_after_temp, bool fma)
{
   if (tuple->add && tuple->add->op == BI_OPCODE_CUBEFACE)
      return bi_lower_cubeface(ctx, clause, tuple);
   else if (tuple->add && tuple->add->op == BI_OPCODE_ATOM_RETURN_I32)
      return bi_lower_atom_c(ctx, clause, tuple);
   else if (tuple->add && tuple->add->op == BI_OPCODE_ATOM1_RETURN_I32)
      return bi_lower_atom_c1(ctx, clause, tuple);
   else if (tuple->add && tuple->add->op == BI_OPCODE_SEG_ADD_I64)
      return bi_lower_seg_add(ctx, clause, tuple);
   else if (tuple->add && tuple->add->table)
      return bi_lower_dtsel(ctx, clause, tuple);

   /* TODO: Optimize these moves */
   if (!fma && tuple->nr_prev_reads > 3) {
      /* Only spill by one source for now */
      assert(tuple->nr_prev_reads == 4);

      /* Pick a source to spill */
      bi_index src = tuple->prev_reads[0];

      /* Schedule the spill */
      bi_builder b = bi_init_builder(ctx, bi_before_tuple(tuple->prev));
      bi_instr *mov = bi_mov_i32_to(&b, src, src);
      bi_pop_instr(clause, tuple, mov, live_after_temp, fma);
      return mov;
   }

#ifndef NDEBUG
   /* Don't pair instructions if debugging */
   if ((bifrost_debug & BIFROST_DBG_NOSCHED) && tuple->add)
      return NULL;
#endif

   unsigned idx = bi_choose_index(st, clause, tuple, live_after_temp, fma);

   if (idx >= st.count)
      return NULL;

   /* Update state to reflect taking the instruction */
   bi_instr *instr = st.instructions[idx];

   BITSET_CLEAR(st.worklist, idx);
   bi_update_worklist(st, idx);
   bi_pop_instr(clause, tuple, instr, live_after_temp, fma);

   /* Fixups */
   bi_builder b = bi_init_builder(ctx, bi_before_instr(instr));

   if (instr->op == BI_OPCODE_IADD_U32 && fma) {
      assert(bi_can_iaddc(instr));
      bi_instr *iaddc = bi_iaddc_i32_to(&b, instr->dest[0], instr->src[0],
                                        instr->src[1], bi_zero());

      bi_remove_instruction(instr);
      instr = iaddc;
   } else if (fma && bi_can_replace_with_csel(instr)) {
      bi_instr *csel = bi_csel_from_mux(&b, instr, false);

      bi_remove_instruction(instr);
      instr = csel;
   }

   return instr;
}

/* Variant of bi_rewrite_index_src_single that uses word-equivalence, rewriting
 * to a passthrough register. If except_sr is true, the staging sources are
 * skipped, so staging register reads are not accidentally encoded as
 * passthrough (which is impossible) */

static void
bi_use_passthrough(bi_instr *ins, bi_index old, enum bifrost_packed_src new,
                   bool except_sr)
{
   /* Optional for convenience */
   if (!ins)
      return;

   assert(!bi_is_null(old));

   bi_foreach_src(ins, i) {
      if ((i == 0 || i == 4) && except_sr)
         continue;

      if (bi_is_word_equiv(ins->src[i], old)) {
         ins->src[i].type = BI_INDEX_PASS;
         ins->src[i].value = new;
         ins->src[i].offset = 0;
      }
   }
}

/* Rewrites an adjacent pair of tuples _prec_eding and _succ_eding to use
 * intertuple passthroughs where necessary. Passthroughs are allowed as a
 * post-condition of scheduling. Note we rewrite ADD first, FMA second --
 * opposite the order of execution. This is deliberate -- if both FMA and ADD
 * write to the same logical register, the next executed tuple will get the
 * latter result. There's no interference issue under the assumption of correct
 * register allocation. */

static void
bi_rewrite_passthrough(bi_tuple prec, bi_tuple succ)
{
   bool sr_read = succ.add ? bi_opcode_props[succ.add->op].sr_read : false;

   if (prec.add && prec.add->nr_dests) {
      bi_use_passthrough(succ.fma, prec.add->dest[0], BIFROST_SRC_PASS_ADD,
                         false);
      bi_use_passthrough(succ.add, prec.add->dest[0], BIFROST_SRC_PASS_ADD,
                         sr_read);
   }

   if (prec.fma && prec.fma->nr_dests) {
      bi_use_passthrough(succ.fma, prec.fma->dest[0], BIFROST_SRC_PASS_FMA,
                         false);
      bi_use_passthrough(succ.add, prec.fma->dest[0], BIFROST_SRC_PASS_FMA,
                         sr_read);
   }
}

static void
bi_rewrite_fau_to_pass(bi_tuple *tuple)
{
   bi_foreach_instr_and_src_in_tuple(tuple, ins, s) {
      if (ins->src[s].type != BI_INDEX_FAU)
         continue;

      bi_index pass = bi_passthrough(ins->src[s].offset ? BIFROST_SRC_FAU_HI
                                                        : BIFROST_SRC_FAU_LO);

      bi_replace_src(ins, s, pass);
   }
}

static void
bi_rewrite_zero(bi_instr *ins, bool fma)
{
   bi_index zero = bi_passthrough(fma ? BIFROST_SRC_STAGE : BIFROST_SRC_FAU_LO);

   bi_foreach_src(ins, s) {
      bi_index src = ins->src[s];

      if (src.type == BI_INDEX_CONSTANT && src.value == 0)
         bi_replace_src(ins, s, zero);
   }
}

/* Assumes #0 to {T, FAU} rewrite has already occurred */

static void
bi_rewrite_constants_to_pass(bi_tuple *tuple, uint64_t constant, bool pcrel)
{
   bi_foreach_instr_and_src_in_tuple(tuple, ins, s) {
      if (ins->src[s].type != BI_INDEX_CONSTANT)
         continue;

      uint32_t cons = ins->src[s].value;

      ASSERTED bool lo = (cons == (constant & 0xffffffff));
      bool hi = (cons == (constant >> 32ull));

      /* PC offsets always live in the upper half, set to zero by
       * convention before pack time. (This is safe, since if you
       * wanted to compare against zero, you would use a BRANCHZ
       * instruction instead.) */
      if (cons == 0 && ins->branch_target != NULL) {
         assert(pcrel);
         hi = true;
         lo = false;
      } else if (pcrel) {
         hi = false;
      }

      assert(lo || hi);

      bi_replace_src(
         ins, s, bi_passthrough(hi ? BIFROST_SRC_FAU_HI : BIFROST_SRC_FAU_LO));
   }
}

/* Constructs a constant state given a tuple state. This has the
 * postcondition that pcrel applies to the first constant by convention,
 * and PC-relative constants will be #0 by convention here, so swap to
 * match if needed */

static struct bi_const_state
bi_get_const_state(struct bi_tuple_state *tuple)
{
   struct bi_const_state consts = {
      .constant_count = tuple->constant_count,
      .constants[0] = tuple->constants[0],
      .constants[1] = tuple->constants[1],
      .pcrel = tuple->add && tuple->add->branch_target,
   };

   /* pcrel applies to the first constant by convention, and
    * PC-relative constants will be #0 by convention here, so swap
    * to match if needed */
   if (consts.pcrel && consts.constants[0]) {
      assert(consts.constant_count == 2);
      assert(consts.constants[1] == 0);

      consts.constants[1] = consts.constants[0];
      consts.constants[0] = 0;
   }

   return consts;
}

/* Merges constants in a clause, satisfying the following rules, assuming no
 * more than one tuple has pcrel:
 *
 * 1. If a tuple has two constants, they must be packed together. If one is
 * pcrel, it must be the high constant to use the M1=4 modification [sx64(E0) +
 * (PC << 32)]. Otherwise choose an arbitrary order.
 *
 * 4. If a tuple has one constant, it may be shared with an existing
 * pair that already contains that constant, or it may be combined with another
 * (distinct) tuple of a single constant.
 *
 * This gaurantees a packing is possible. The next routine handles modification
 * related swapping, to satisfy format 12 and the lack of modification for
 * tuple count 5/8 in EC0.
 */

static uint64_t
bi_merge_u32(uint32_t c0, uint32_t c1, bool pcrel)
{
   /* At this point in the constant merge algorithm, pcrel constants are
    * treated as zero, so pcrel implies at least one constants is zero */
   assert(!pcrel || (c0 == 0 || c1 == 0));

   /* Order: pcrel, maximum non-pcrel, minimum non-pcrel */
   uint32_t hi = pcrel ? 0 : MAX2(c0, c1);
   uint32_t lo = (c0 == hi) ? c1 : c0;

   /* Merge in the selected order */
   return lo | (((uint64_t)hi) << 32ull);
}

static unsigned
bi_merge_pairs(struct bi_const_state *consts, unsigned tuple_count,
               uint64_t *merged, unsigned *pcrel_pair)
{
   unsigned merge_count = 0;

   for (unsigned t = 0; t < tuple_count; ++t) {
      if (consts[t].constant_count != 2)
         continue;

      unsigned idx = ~0;
      uint64_t val = bi_merge_u32(consts[t].constants[0],
                                  consts[t].constants[1], consts[t].pcrel);

      /* Skip the pcrel pair if assigned, because if one is assigned,
       * this one is not pcrel by uniqueness so it's a mismatch */
      for (unsigned s = 0; s < merge_count; ++s) {
         if (merged[s] == val && (*pcrel_pair) != s) {
            idx = s;
            break;
         }
      }

      if (idx == ~0) {
         idx = merge_count++;
         merged[idx] = val;

         if (consts[t].pcrel)
            (*pcrel_pair) = idx;
      }

      consts[t].word_idx = idx;
   }

   return merge_count;
}

static unsigned
bi_merge_singles(struct bi_const_state *consts, unsigned tuple_count,
                 uint64_t *pairs, unsigned pair_count, unsigned *pcrel_pair)
{
   bool pending = false, pending_pcrel = false;
   uint32_t pending_single = 0;

   for (unsigned t = 0; t < tuple_count; ++t) {
      if (consts[t].constant_count != 1)
         continue;

      uint32_t val = consts[t].constants[0];
      unsigned idx = ~0;

      /* Try to match, but don't match pcrel with non-pcrel, even
       * though we can merge a pcrel with a non-pcrel single */
      for (unsigned i = 0; i < pair_count; ++i) {
         bool lo = ((pairs[i] & 0xffffffff) == val);
         bool hi = ((pairs[i] >> 32) == val);
         bool match = (lo || hi);
         match &= ((*pcrel_pair) != i);
         if (match && !consts[t].pcrel) {
            idx = i;
            break;
         }
      }

      if (idx == ~0) {
         idx = pair_count;

         if (pending && pending_single != val) {
            assert(!(pending_pcrel && consts[t].pcrel));
            bool pcrel = pending_pcrel || consts[t].pcrel;

            if (pcrel)
               *pcrel_pair = idx;

            pairs[pair_count++] = bi_merge_u32(pending_single, val, pcrel);

            pending = pending_pcrel = false;
         } else {
            pending = true;
            pending_pcrel = consts[t].pcrel;
            pending_single = val;
         }
      }

      consts[t].word_idx = idx;
   }

   /* Shift so it works whether pending_pcrel is set or not */
   if (pending) {
      if (pending_pcrel)
         *pcrel_pair = pair_count;

      pairs[pair_count++] = ((uint64_t)pending_single) << 32ull;
   }

   return pair_count;
}

static unsigned
bi_merge_constants(struct bi_const_state *consts, uint64_t *pairs,
                   unsigned *pcrel_idx)
{
   unsigned pair_count = bi_merge_pairs(consts, 8, pairs, pcrel_idx);
   return bi_merge_singles(consts, 8, pairs, pair_count, pcrel_idx);
}

/* Swap two constants at word i and i+1 by swapping their actual positions and
 * swapping all references so the meaning of the clause is preserved */

static void
bi_swap_constants(struct bi_const_state *consts, uint64_t *pairs, unsigned i)
{
   uint64_t tmp_pair = pairs[i + 0];
   pairs[i + 0] = pairs[i + 1];
   pairs[i + 1] = tmp_pair;

   for (unsigned t = 0; t < 8; ++t) {
      if (consts[t].word_idx == i)
         consts[t].word_idx = (i + 1);
      else if (consts[t].word_idx == (i + 1))
         consts[t].word_idx = i;
   }
}

/* Given merged constants, one of which might be PC-relative, fix up the M
 * values so the PC-relative constant (if it exists) has the M1=4 modification
 * and other constants are used as-is (which might require swapping) */

static unsigned
bi_apply_constant_modifiers(struct bi_const_state *consts, uint64_t *pairs,
                            unsigned *pcrel_idx, unsigned tuple_count,
                            unsigned constant_count)
{
   unsigned start = bi_ec0_packed(tuple_count) ? 1 : 0;

   /* Clauses with these tuple counts lack an M field for the packed EC0,
    * so EC0 cannot be PC-relative, which might require swapping (and
    * possibly adding an unused constant) to fit */

   if (*pcrel_idx == 0 && (tuple_count == 5 || tuple_count == 8)) {
      constant_count = MAX2(constant_count, 2);
      *pcrel_idx = 1;
      bi_swap_constants(consts, pairs, 0);
   }

   /* EC0 might be packed free, after that constants are packed in pairs
    * (with clause format 12), with M1 values computed from the pair */

   for (unsigned i = start; i < constant_count; i += 2) {
      bool swap = false;
      bool last = (i + 1) == constant_count;

      unsigned A1 = (pairs[i] >> 60);
      unsigned B1 = (pairs[i + 1] >> 60);

      if (*pcrel_idx == i || *pcrel_idx == (i + 1)) {
         /* PC-relative constant must be E0, not E1 */
         swap = (*pcrel_idx == (i + 1));

         /* Set M1 = 4 by noting (A - B) mod 16 = 4 is
          * equivalent to A = (B + 4) mod 16 and that we can
          * control A */
         unsigned B = swap ? A1 : B1;
         unsigned A = (B + 4) & 0xF;
         pairs[*pcrel_idx] |= ((uint64_t)A) << 60;

         /* Swapped if swap set, identity if swap not set */
         *pcrel_idx = i;
      } else {
         /* Compute M1 value if we don't swap */
         unsigned M1 = (16 + A1 - B1) & 0xF;

         /* For M1 = 0 or M1 >= 8, the constants are unchanged,
          * we have 0 < (A1 - B1) % 16 < 8, which implies (B1 -
          * A1) % 16 >= 8, so swapping will let them be used
          * unchanged */
         swap = (M1 != 0) && (M1 < 8);

         /* However, we can't swap the last constant, so we
          * force M1 = 0 instead for this case */
         if (last && swap) {
            pairs[i + 1] |= pairs[i] & (0xfull << 60);
            swap = false;
         }
      }

      if (swap) {
         assert(!last);
         bi_swap_constants(consts, pairs, i);
      }
   }

   return constant_count;
}

/* Schedule a single clause. If no instructions remain, return NULL. */

static bi_clause *
bi_schedule_clause(bi_context *ctx, bi_block *block, struct bi_worklist st,
                   uint64_t *live)
{
   struct bi_clause_state clause_state = {0};
   bi_clause *clause = rzalloc(ctx, bi_clause);
   bi_tuple *tuple = NULL;

   const unsigned max_tuples = ARRAY_SIZE(clause->tuples);

   /* TODO: Decide flow control better */
   clause->flow_control = BIFROST_FLOW_NBTB;

   /* The last clause can only write one instruction, so initialize that */
   struct bi_reg_state reg_state = {};
   bi_index prev_reads[5] = {bi_null()};
   unsigned nr_prev_reads = 0;

   /* We need to track future liveness. The main *live set tracks what is
    * live at the current point int he program we are scheduling, but to
    * determine temp eligibility, we instead want what will be live after
    * the next tuple in the program. If you scheduled forwards, you'd need
    * a crystall ball for this. Luckily we schedule backwards, so we just
    * delay updates to the live_after_temp by an extra tuple. */
   uint64_t live_after_temp = *live;
   uint64_t live_next_tuple = live_after_temp;

   do {
      struct bi_tuple_state tuple_state = {
         .last = (clause->tuple_count == 0),
         .reg = reg_state,
         .nr_prev_reads = nr_prev_reads,
         .prev = tuple,
         .pcrel_idx = ~0,
      };

      assert(nr_prev_reads < ARRAY_SIZE(prev_reads));
      memcpy(tuple_state.prev_reads, prev_reads, sizeof(prev_reads));

      unsigned idx = max_tuples - clause->tuple_count - 1;

      tuple = &clause->tuples[idx];

      if (clause->message && bi_opcode_props[clause->message->op].sr_read &&
          !bi_is_null(clause->message->src[0])) {
         unsigned nr = bi_count_read_registers(clause->message, 0);
         live_after_temp |=
            (BITFIELD64_MASK(nr) << clause->message->src[0].value);
      }

      /* Since we schedule backwards, we schedule ADD first */
      tuple_state.add = bi_take_instr(ctx, st, &clause_state, &tuple_state,
                                      live_after_temp, false);
      tuple->fma = bi_take_instr(ctx, st, &clause_state, &tuple_state,
                                 live_after_temp, true);
      tuple->add = tuple_state.add;

      /* Update liveness from the new instructions */
      if (tuple->add)
         *live = bi_postra_liveness_ins(*live, tuple->add);

      if (tuple->fma)
         *live = bi_postra_liveness_ins(*live, tuple->fma);

      /* Rotate in the new per-tuple liveness */
      live_after_temp = live_next_tuple;
      live_next_tuple = *live;

      /* We may have a message, but only one per clause */
      if (tuple->add && bi_must_message(tuple->add)) {
         assert(!clause_state.message);
         clause_state.message = true;

         clause->message_type = bi_message_type_for_instr(tuple->add);
         clause->message = tuple->add;

         /* We don't need to set dependencies for blend shaders
          * because the BLEND instruction in the fragment
          * shader should have already done the wait */
         if (!ctx->inputs->is_blend) {
            switch (tuple->add->op) {
            case BI_OPCODE_ATEST:
               clause->dependencies |= (1 << BIFROST_SLOT_ELDEST_DEPTH);
               break;
            case BI_OPCODE_LD_TILE:
            case BI_OPCODE_ST_TILE:
               clause->dependencies |= (1 << BIFROST_SLOT_ELDEST_COLOUR);
               break;
            case BI_OPCODE_BLEND:
               clause->dependencies |= (1 << BIFROST_SLOT_ELDEST_DEPTH);
               clause->dependencies |= (1 << BIFROST_SLOT_ELDEST_COLOUR);
               break;
            default:
               break;
            }
         }
      }

      clause_state.consts[idx] = bi_get_const_state(&tuple_state);

      /* Before merging constants, eliminate zeroes, otherwise the
       * merging will fight over the #0 that never gets read (and is
       * never marked as read by update_fau) */
      if (tuple->fma && bi_reads_zero(tuple->fma))
         bi_rewrite_zero(tuple->fma, true);

      /* Rewrite away FAU, constant write is deferred */
      if (!tuple_state.constant_count) {
         tuple->fau_idx = tuple_state.fau;
         bi_rewrite_fau_to_pass(tuple);
      }

      /* Use passthrough register for cross-stage accesses. Since
       * there are just FMA and ADD stages, that means we rewrite to
       * passthrough the sources of the ADD that read from the
       * destination of the FMA */

      if (tuple->fma && tuple->fma->nr_dests) {
         bi_use_passthrough(tuple->add, tuple->fma->dest[0], BIFROST_SRC_STAGE,
                            false);
      }

      /* Don't add an empty tuple, unless the worklist has nothing
       * but a (pseudo)instruction failing to schedule due to a "not
       * last instruction" constraint */

      int some_instruction = __bitset_ffs(st.worklist, BITSET_WORDS(st.count));
      bool not_last = (some_instruction > 0) &&
                      bi_must_not_last(st.instructions[some_instruction - 1]);

      bool insert_empty = tuple_state.last && not_last;

      if (!(tuple->fma || tuple->add || insert_empty))
         break;

      clause->tuple_count++;

      /* Adding enough tuple might overflow constants */
      if (!bi_space_for_more_constants(&clause_state))
         break;

#ifndef NDEBUG
      /* Don't schedule more than 1 tuple if debugging */
      if ((bifrost_debug & BIFROST_DBG_NOSCHED) && !insert_empty)
         break;
#endif

      /* Link through the register state */
      STATIC_ASSERT(sizeof(prev_reads) == sizeof(tuple_state.reg.reads));
      memcpy(prev_reads, tuple_state.reg.reads, sizeof(prev_reads));
      nr_prev_reads = tuple_state.reg.nr_reads;
      clause_state.tuple_count++;
   } while (clause->tuple_count < 8);

   /* Don't schedule an empty clause */
   if (!clause->tuple_count)
      return NULL;

   /* Before merging, rewrite away any tuples that read only zero */
   for (unsigned i = max_tuples - clause->tuple_count; i < max_tuples; ++i) {
      bi_tuple *tuple = &clause->tuples[i];
      struct bi_const_state *st = &clause_state.consts[i];

      if (st->constant_count == 0 || st->constants[0] || st->constants[1] ||
          st->pcrel)
         continue;

      bi_foreach_instr_in_tuple(tuple, ins)
         bi_rewrite_zero(ins, false);

      /* Constant has been demoted to FAU, so don't pack it separately */
      st->constant_count = 0;

      /* Default */
      assert(tuple->fau_idx == BIR_FAU_ZERO);
   }

   uint64_t constant_pairs[8] = {0};
   unsigned pcrel_idx = ~0;
   unsigned constant_words =
      bi_merge_constants(clause_state.consts, constant_pairs, &pcrel_idx);

   constant_words = bi_apply_constant_modifiers(
      clause_state.consts, constant_pairs, &pcrel_idx, clause->tuple_count,
      constant_words);

   clause->pcrel_idx = pcrel_idx;

   for (unsigned i = max_tuples - clause->tuple_count; i < max_tuples; ++i) {
      bi_tuple *tuple = &clause->tuples[i];

      /* If no constants, leave FAU as it is, possibly defaulting to 0 */
      if (clause_state.consts[i].constant_count == 0)
         continue;

      /* FAU is already handled */
      assert(!tuple->fau_idx);

      unsigned word_idx = clause_state.consts[i].word_idx;
      assert(word_idx <= 8);

      /* We could try to merge regardless of bottom bits as well, but
       * that's probably diminishing returns */
      uint64_t pair = constant_pairs[word_idx];
      unsigned lo = pair & 0xF;

      tuple->fau_idx = bi_constant_field(word_idx) | lo;
      bi_rewrite_constants_to_pass(tuple, pair, word_idx == pcrel_idx);
   }

   clause->constant_count = constant_words;
   memcpy(clause->constants, constant_pairs, sizeof(constant_pairs));

   /* Branches must be last, so this can be factored out */
   bi_instr *last = clause->tuples[max_tuples - 1].add;
   clause->next_clause_prefetch = !last || (last->op != BI_OPCODE_JUMP);
   clause->block = block;

   clause->ftz = (clause_state.ftz == BI_FTZ_STATE_ENABLE);

   /* We emit in reverse and emitted to the back of the tuples array, so
    * move it up front for easy indexing */
   memmove(clause->tuples, clause->tuples + (max_tuples - clause->tuple_count),
           clause->tuple_count * sizeof(clause->tuples[0]));

   /* Use passthrough register for cross-tuple accesses. Note this is
    * after the memmove, so this is forwards. Skip the first tuple since
    * there is nothing before it to passthrough */

   for (unsigned t = 1; t < clause->tuple_count; ++t)
      bi_rewrite_passthrough(clause->tuples[t - 1], clause->tuples[t]);

   return clause;
}

static void
bi_schedule_block(bi_context *ctx, bi_block *block)
{
   list_inithead(&block->clauses);

   /* Copy list to dynamic array */
   struct bi_worklist st = bi_initialize_worklist(
      block, bifrost_debug & BIFROST_DBG_INORDER, ctx->inputs->is_blend);

   if (!st.count) {
      bi_free_worklist(st);
      return;
   }

   /* We need to track liveness during scheduling in order to determine whether
    * we can use temporary (passthrough) registers */
   uint64_t live = block->reg_live_out;

   /* Schedule as many clauses as needed to fill the block */
   bi_clause *u = NULL;
   while ((u = bi_schedule_clause(ctx, block, st, &live)))
      list_add(&u->link, &block->clauses);

   /* Back-to-back bit affects only the last clause of a block,
    * the rest are implicitly true */
   if (!list_is_empty(&block->clauses)) {
      bi_clause *last_clause =
         list_last_entry(&block->clauses, bi_clause, link);
      if (bi_reconverge_branches(block))
         last_clause->flow_control = BIFROST_FLOW_NBTB_UNCONDITIONAL;
   }

   /* Reorder instructions to match the new schedule. First remove
    * existing instructions and then recreate the list */

   bi_foreach_instr_in_block_safe(block, ins) {
      list_del(&ins->link);
   }

   bi_foreach_clause_in_block(block, clause) {
      for (unsigned i = 0; i < clause->tuple_count; ++i) {
         bi_foreach_instr_in_tuple(&clause->tuples[i], ins) {
            list_addtail(&ins->link, &block->instructions);
         }
      }
   }

   block->scheduled = true;

#ifndef NDEBUG
   unsigned i;
   bool incomplete = false;

   BITSET_FOREACH_SET(i, st.worklist, st.count) {
      bi_print_instr(st.instructions[i], stderr);
      incomplete = true;
   }

   if (incomplete)
      unreachable("The above instructions failed to schedule.");
#endif

   bi_free_worklist(st);
}

static bool
bi_check_fau_src(bi_instr *ins, unsigned s, uint32_t *constants,
                 unsigned *cwords, bi_index *fau)
{
   assert(s < ins->nr_srcs);
   bi_index src = ins->src[s];

   /* Staging registers can't have FAU accesses */
   if (bi_is_staging_src(ins, s))
      return (src.type != BI_INDEX_CONSTANT) && (src.type != BI_INDEX_FAU);

   if (src.type == BI_INDEX_CONSTANT) {
      /* Allow fast zero */
      if (src.value == 0 && bi_opcode_props[ins->op].fma && bi_reads_zero(ins))
         return true;

      if (!bi_is_null(*fau))
         return false;

      /* Else, try to inline a constant */
      for (unsigned i = 0; i < *cwords; ++i) {
         if (src.value == constants[i])
            return true;
      }

      if (*cwords >= 2)
         return false;

      constants[(*cwords)++] = src.value;
   } else if (src.type == BI_INDEX_FAU) {
      if (*cwords != 0)
         return false;

      /* Can only read from one pair of FAU words */
      if (!bi_is_null(*fau) && (src.value != fau->value))
         return false;

      /* If there is a target, we'll need a PC-relative constant */
      if (ins->branch_target)
         return false;

      *fau = src;
   }

   return true;
}

void
bi_lower_fau(bi_context *ctx)
{
   bi_foreach_instr_global_safe(ctx, ins) {
      bi_builder b = bi_init_builder(ctx, bi_before_instr(ins));

      uint32_t constants[2];
      unsigned cwords = 0;
      bi_index fau = bi_null();

      /* ATEST must have the ATEST datum encoded, not any other
       * uniform. See to it this is the case. */
      if (ins->op == BI_OPCODE_ATEST)
         fau = ins->src[2];

      /* Dual texturing requires the texture operation descriptor
       * encoded as an immediate so we can fix up.
       */
      if (ins->op == BI_OPCODE_TEXC_DUAL) {
         assert(ins->src[3].type == BI_INDEX_CONSTANT);
         constants[cwords++] = ins->src[3].value;
      }

      /* Phis get split up into moves so are unrestricted */
      if (ins->op == BI_OPCODE_PHI)
         continue;

      bi_foreach_src(ins, s) {
         if (bi_check_fau_src(ins, s, constants, &cwords, &fau))
            continue;

         bi_index copy = bi_mov_i32(&b, ins->src[s]);
         bi_replace_src(ins, s, copy);
      }
   }
}

/* Only v7 allows specifying a dependency on the tilebuffer for the first
 * clause of a shader. v6 requires adding a NOP clause with the depedency. */

static void
bi_add_nop_for_atest(bi_context *ctx)
{
   /* Only needed on v6 */
   if (ctx->arch >= 7)
      return;

   if (list_is_empty(&ctx->blocks))
      return;

   /* Fetch the first clause of the shader */
   bi_block *block = list_first_entry(&ctx->blocks, bi_block, link);
   bi_clause *clause = bi_next_clause(ctx, block, NULL);

   if (!clause || !(clause->dependencies & ((1 << BIFROST_SLOT_ELDEST_DEPTH) |
                                            (1 << BIFROST_SLOT_ELDEST_COLOUR))))
      return;

   /* Add a NOP so we can wait for the dependencies required by the first
    * clause */

   bi_instr *I = rzalloc(ctx, bi_instr);
   I->op = BI_OPCODE_NOP;

   bi_clause *new_clause = ralloc(ctx, bi_clause);
   *new_clause = (bi_clause){
      .flow_control = BIFROST_FLOW_NBTB,
      .next_clause_prefetch = true,
      .block = clause->block,

      .tuple_count = 1,
      .tuples[0] =
         {
            .fma = I,
         },
   };

   list_add(&new_clause->link, &clause->block->clauses);
}

void
bi_schedule(bi_context *ctx)
{
   /* Fed into both scheduling and DCE */
   bi_postra_liveness(ctx);

   bi_foreach_block(ctx, block) {
      bi_schedule_block(ctx, block);
   }

   bi_opt_dce_post_ra(ctx);
   bi_add_nop_for_atest(ctx);
}
