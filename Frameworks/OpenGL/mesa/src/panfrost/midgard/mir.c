/*
 * Copyright (C) 2019 Alyssa Rosenzweig <alyssa@rosenzweig.io>
 * Copyright (C) 2019-2020 Collabora, Ltd.
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

#include "compiler.h"
#include "midgard_ops.h"

void
mir_rewrite_index_src_single(midgard_instruction *ins, unsigned old,
                             unsigned new)
{
   mir_foreach_src(ins, i) {
      if (ins->src[i] == old)
         ins->src[i] = new;
   }
}

void
mir_rewrite_index_dst_single(midgard_instruction *ins, unsigned old,
                             unsigned new)
{
   if (ins->dest == old)
      ins->dest = new;
}

static void
mir_rewrite_index_src_single_swizzle(midgard_instruction *ins, unsigned old,
                                     unsigned new, unsigned *swizzle)
{
   for (unsigned i = 0; i < ARRAY_SIZE(ins->src); ++i) {
      if (ins->src[i] != old)
         continue;

      ins->src[i] = new;
      mir_compose_swizzle(ins->swizzle[i], swizzle, ins->swizzle[i]);
   }
}

void
mir_rewrite_index_src(compiler_context *ctx, unsigned old, unsigned new)
{
   mir_foreach_instr_global(ctx, ins) {
      mir_rewrite_index_src_single(ins, old, new);
   }
}

void
mir_rewrite_index_src_swizzle(compiler_context *ctx, unsigned old, unsigned new,
                              unsigned *swizzle)
{
   mir_foreach_instr_global(ctx, ins) {
      mir_rewrite_index_src_single_swizzle(ins, old, new, swizzle);
   }
}

void
mir_rewrite_index_dst(compiler_context *ctx, unsigned old, unsigned new)
{
   mir_foreach_instr_global(ctx, ins) {
      mir_rewrite_index_dst_single(ins, old, new);
   }

   /* Implicitly written before the shader */
   if (ctx->blend_input == old)
      ctx->blend_input = new;

   if (ctx->blend_src1 == old)
      ctx->blend_src1 = new;
}

void
mir_rewrite_index(compiler_context *ctx, unsigned old, unsigned new)
{
   mir_rewrite_index_src(ctx, old, new);
   mir_rewrite_index_dst(ctx, old, new);
}

unsigned
mir_use_count(compiler_context *ctx, unsigned value)
{
   unsigned used_count = 0;

   mir_foreach_instr_global(ctx, ins) {
      if (mir_has_arg(ins, value))
         ++used_count;
   }

   if (ctx->blend_input == value)
      ++used_count;

   if (ctx->blend_src1 == value)
      ++used_count;

   return used_count;
}

/* Checks if a value is used only once (or totally dead), which is an important
 * heuristic to figure out if certain optimizations are Worth It (TM) */

bool
mir_single_use(compiler_context *ctx, unsigned value)
{
   /* We can replicate constants in places so who cares */
   if (value == SSA_FIXED_REGISTER(REGISTER_CONSTANT))
      return true;

   return mir_use_count(ctx, value) <= 1;
}

bool
mir_nontrivial_mod(midgard_instruction *ins, unsigned i, bool check_swizzle)
{
   bool is_int = midgard_is_integer_op(ins->op);

   if (is_int) {
      if (ins->src_shift[i])
         return true;
   } else {
      if (ins->src_neg[i])
         return true;
      if (ins->src_abs[i])
         return true;
   }

   if (ins->dest_type != ins->src_types[i])
      return true;

   if (check_swizzle) {
      for (unsigned c = 0; c < 16; ++c) {
         if (!(ins->mask & (1 << c)))
            continue;
         if (ins->swizzle[i][c] != c)
            return true;
      }
   }

   return false;
}

bool
mir_nontrivial_outmod(midgard_instruction *ins)
{
   bool is_int = midgard_is_integer_op(ins->op);
   unsigned mod = ins->outmod;

   if (ins->dest_type != ins->src_types[1])
      return true;

   if (is_int)
      return mod != midgard_outmod_keeplo;
   else
      return mod != midgard_outmod_none;
}

/* 128 / sz = exp2(log2(128 / sz))
 *          = exp2(log2(128) - log2(sz))
 *          = exp2(7 - log2(sz))
 *          = 1 << (7 - log2(sz))
 */

static unsigned
mir_components_for_bits(unsigned bits)
{
   return 1 << (7 - util_logbase2(bits));
}

unsigned
mir_components_for_type(nir_alu_type T)
{
   unsigned sz = nir_alu_type_get_type_size(T);
   return mir_components_for_bits(sz);
}

uint16_t
mir_from_bytemask(uint16_t bytemask, unsigned bits)
{
   unsigned value = 0;
   unsigned count = bits / 8;

   for (unsigned c = 0, d = 0; c < 16; c += count, ++d) {
      bool a = (bytemask & (1 << c)) != 0;

      for (unsigned q = c; q < count; ++q)
         assert(((bytemask & (1 << q)) != 0) == a);

      value |= (a << d);
   }

   return value;
}

/* Rounds up a bytemask to fill a given component count. Iterate each
 * component, and check if any bytes in the component are masked on */

uint16_t
mir_round_bytemask_up(uint16_t mask, unsigned bits)
{
   unsigned bytes = bits / 8;
   unsigned maxmask = mask_of(bytes);
   unsigned channels = mir_components_for_bits(bits);

   for (unsigned c = 0; c < channels; ++c) {
      unsigned submask = maxmask << (c * bytes);

      if (mask & submask)
         mask |= submask;
   }

   return mask;
}

/* Grabs the per-byte mask of an instruction (as opposed to per-component) */

uint16_t
mir_bytemask(midgard_instruction *ins)
{
   unsigned type_size = nir_alu_type_get_type_size(ins->dest_type);
   return pan_to_bytemask(type_size, ins->mask);
}

void
mir_set_bytemask(midgard_instruction *ins, uint16_t bytemask)
{
   unsigned type_size = nir_alu_type_get_type_size(ins->dest_type);
   ins->mask = mir_from_bytemask(bytemask, type_size);
}

/*
 * Checks if we should use an upper destination override, rather than the lower
 * one in the IR. If yes, returns the bytes to shift by. If no, returns zero
 * for a lower override and negative for no override.
 */
signed
mir_upper_override(midgard_instruction *ins, unsigned inst_size)
{
   unsigned type_size = nir_alu_type_get_type_size(ins->dest_type);

   /* If the sizes are the same, there's nothing to override */
   if (type_size == inst_size)
      return -1;

   /* There are 16 bytes per vector, so there are (16/bytes)
    * components per vector. So the magic half is half of
    * (16/bytes), which simplifies to 8/bytes = 8 / (bits / 8) = 64 / bits
    * */

   unsigned threshold = mir_components_for_bits(type_size) >> 1;

   /* How many components did we shift over? */
   unsigned zeroes = __builtin_ctz(ins->mask);

   /* Did we hit the threshold? */
   return (zeroes >= threshold) ? threshold : 0;
}

/* Creates a mask of the components of a node read by an instruction, by
 * analyzing the swizzle with respect to the instruction's mask. E.g.:
 *
 *  fadd r0.xz, r1.yyyy, r2.zwyx
 *
 * will return a mask of Z/Y for r2
 */

static uint16_t
mir_bytemask_of_read_components_single(unsigned *swizzle, unsigned inmask,
                                       unsigned bits)
{
   unsigned cmask = 0;

   for (unsigned c = 0; c < MIR_VEC_COMPONENTS; ++c) {
      if (!(inmask & (1 << c)))
         continue;
      cmask |= (1 << swizzle[c]);
   }

   return pan_to_bytemask(bits, cmask);
}

uint16_t
mir_bytemask_of_read_components_index(midgard_instruction *ins, unsigned i)
{
   /* Conditional branches read one 32-bit component = 4 bytes (TODO: multi
    * branch??) */
   if (ins->compact_branch && ins->branch.conditional && (i == 0))
      return 0xF;

   /* ALU ops act componentwise so we need to pay attention to
    * their mask. Texture/ldst does not so we don't clamp source
    * readmasks based on the writemask */
   unsigned qmask = ~0;

   /* Handle dot products and things */
   if (ins->type == TAG_ALU_4 && !ins->compact_branch) {
      unsigned props = alu_opcode_props[ins->op].props;

      unsigned channel_override = GET_CHANNEL_COUNT(props);

      if (channel_override)
         qmask = mask_of(channel_override);
      else
         qmask = ins->mask;
   }

   return mir_bytemask_of_read_components_single(
      ins->swizzle[i], qmask, nir_alu_type_get_type_size(ins->src_types[i]));
}

uint16_t
mir_bytemask_of_read_components(midgard_instruction *ins, unsigned node)
{
   uint16_t mask = 0;

   if (node == ~0)
      return 0;

   mir_foreach_src(ins, i) {
      if (ins->src[i] != node)
         continue;
      mask |= mir_bytemask_of_read_components_index(ins, i);
   }

   return mask;
}

/* Register allocation occurs after instruction scheduling, which is fine until
 * we start needing to spill registers and therefore insert instructions into
 * an already-scheduled program. We don't have to be terribly efficient about
 * this, since spilling is already slow. So just semantically we need to insert
 * the instruction into a new bundle before/after the bundle of the instruction
 * in question */

static midgard_bundle
mir_bundle_for_op(compiler_context *ctx, midgard_instruction ins)
{
   midgard_instruction *u = mir_upload_ins(ctx, ins);

   midgard_bundle bundle = {
      .tag = ins.type,
      .instruction_count = 1,
      .instructions = {u},
   };

   if (bundle.tag == TAG_ALU_4) {
      assert(OP_IS_MOVE(u->op));
      u->unit = UNIT_VMUL;

      size_t bytes_emitted = sizeof(uint32_t) + sizeof(midgard_reg_info) +
                             sizeof(midgard_vector_alu);
      bundle.padding = ~(bytes_emitted - 1) & 0xF;
      bundle.control = ins.type | u->unit;
   }

   return bundle;
}

static unsigned
mir_bundle_idx_for_ins(midgard_instruction *tag, midgard_block *block)
{
   midgard_bundle *bundles = (midgard_bundle *)block->bundles.data;

   size_t count = (block->bundles.size / sizeof(midgard_bundle));

   for (unsigned i = 0; i < count; ++i) {
      for (unsigned j = 0; j < bundles[i].instruction_count; ++j) {
         if (bundles[i].instructions[j] == tag)
            return i;
      }
   }

   mir_print_instruction(tag);
   unreachable("Instruction not scheduled in block");
}

midgard_instruction *
mir_insert_instruction_before_scheduled(compiler_context *ctx,
                                        midgard_block *block,
                                        midgard_instruction *tag,
                                        midgard_instruction ins)
{
   unsigned before = mir_bundle_idx_for_ins(tag, block);
   size_t count = util_dynarray_num_elements(&block->bundles, midgard_bundle);
   UNUSED void *unused = util_dynarray_grow(&block->bundles, midgard_bundle, 1);

   midgard_bundle *bundles = (midgard_bundle *)block->bundles.data;
   memmove(bundles + before + 1, bundles + before,
           (count - before) * sizeof(midgard_bundle));
   midgard_bundle *before_bundle = bundles + before + 1;

   midgard_bundle new = mir_bundle_for_op(ctx, ins);
   memcpy(bundles + before, &new, sizeof(new));

   list_addtail(&new.instructions[0]->link,
                &before_bundle->instructions[0]->link);
   block->quadword_count += midgard_tag_props[new.tag].size;

   return new.instructions[0];
}

midgard_instruction *
mir_insert_instruction_after_scheduled(compiler_context *ctx,
                                       midgard_block *block,
                                       midgard_instruction *tag,
                                       midgard_instruction ins)
{
   /* We need to grow the bundles array to add our new bundle */
   size_t count = util_dynarray_num_elements(&block->bundles, midgard_bundle);
   UNUSED void *unused = util_dynarray_grow(&block->bundles, midgard_bundle, 1);

   /* Find the bundle that we want to insert after */
   unsigned after = mir_bundle_idx_for_ins(tag, block);

   /* All the bundles after that one, we move ahead by one */
   midgard_bundle *bundles = (midgard_bundle *)block->bundles.data;
   memmove(bundles + after + 2, bundles + after + 1,
           (count - after - 1) * sizeof(midgard_bundle));
   midgard_bundle *after_bundle = bundles + after;

   midgard_bundle new = mir_bundle_for_op(ctx, ins);
   memcpy(bundles + after + 1, &new, sizeof(new));
   list_add(
      &new.instructions[0]->link,
      &after_bundle->instructions[after_bundle->instruction_count - 1]->link);
   block->quadword_count += midgard_tag_props[new.tag].size;

   return new.instructions[0];
}

/* Flip the first-two arguments of a (binary) op. Currently ALU
 * only, no known uses for ldst/tex */

void
mir_flip(midgard_instruction *ins)
{
   unsigned temp = ins->src[0];
   ins->src[0] = ins->src[1];
   ins->src[1] = temp;

   assert(ins->type == TAG_ALU_4);

   temp = ins->src_types[0];
   ins->src_types[0] = ins->src_types[1];
   ins->src_types[1] = temp;

   temp = ins->src_abs[0];
   ins->src_abs[0] = ins->src_abs[1];
   ins->src_abs[1] = temp;

   temp = ins->src_neg[0];
   ins->src_neg[0] = ins->src_neg[1];
   ins->src_neg[1] = temp;

   temp = ins->src_invert[0];
   ins->src_invert[0] = ins->src_invert[1];
   ins->src_invert[1] = temp;

   unsigned temp_swizzle[16];
   memcpy(temp_swizzle, ins->swizzle[0], sizeof(ins->swizzle[0]));
   memcpy(ins->swizzle[0], ins->swizzle[1], sizeof(ins->swizzle[0]));
   memcpy(ins->swizzle[1], temp_swizzle, sizeof(ins->swizzle[0]));
}

/* Before squashing, calculate ctx->temp_count just by observing the MIR */

void
mir_compute_temp_count(compiler_context *ctx)
{
   unsigned max_index = 0;

   mir_foreach_instr_global(ctx, ins) {
      if (ins->dest < SSA_FIXED_MINIMUM)
         max_index = MAX2(max_index, ins->dest + 1);
   }

   if (ctx->blend_input != ~0)
      max_index = MAX2(max_index, ctx->blend_input + 1);

   if (ctx->blend_src1 != ~0)
      max_index = MAX2(max_index, ctx->blend_src1 + 1);

   ctx->temp_count = max_index;
}
