/*
 * Copyright © 2019 Google, Inc.
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

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"
#include "util/u_math.h"
#include "ir3_compiler.h"
#include "ir3_nir.h"

static inline bool
get_ubo_load_range(nir_shader *nir, nir_intrinsic_instr *instr,
                   uint32_t alignment, struct ir3_ubo_range *r)
{
   uint32_t offset = nir_intrinsic_range_base(instr);
   uint32_t size = nir_intrinsic_range(instr);

   /* If the offset is constant, the range is trivial (and NIR may not have
    * figured it out).
    */
   if (nir_src_is_const(instr->src[1])) {
      offset = nir_src_as_uint(instr->src[1]);
      size = nir_intrinsic_dest_components(instr) * 4;
   }

   /* If we haven't figured out the range accessed in the UBO, bail. */
   if (size == ~0)
      return false;

   r->start = ROUND_DOWN_TO(offset, alignment * 16);
   r->end = ALIGN(offset + size, alignment * 16);

   return true;
}

static bool
get_ubo_info(nir_intrinsic_instr *instr, struct ir3_ubo_info *ubo)
{
   if (nir_src_is_const(instr->src[0])) {
      ubo->block = nir_src_as_uint(instr->src[0]);
      ubo->bindless_base = 0;
      ubo->bindless = false;
      return true;
   } else {
      nir_intrinsic_instr *rsrc = ir3_bindless_resource(instr->src[0]);
      if (rsrc && nir_src_is_const(rsrc->src[0])) {
         ubo->block = nir_src_as_uint(rsrc->src[0]);
         ubo->bindless_base = nir_intrinsic_desc_set(rsrc);
         ubo->bindless = true;
         return true;
      }
   }
   return false;
}

/**
 * Finds the given instruction's UBO load in the UBO upload plan, if any.
 */
static const struct ir3_ubo_range *
get_existing_range(nir_intrinsic_instr *instr,
                   const struct ir3_ubo_analysis_state *state,
                   struct ir3_ubo_range *r)
{
   struct ir3_ubo_info ubo = {};

   if (!get_ubo_info(instr, &ubo))
      return NULL;

   for (int i = 0; i < state->num_enabled; i++) {
      const struct ir3_ubo_range *range = &state->range[i];
      if (!memcmp(&range->ubo, &ubo, sizeof(ubo)) && r->start >= range->start &&
          r->end <= range->end) {
         return range;
      }
   }

   return NULL;
}

/**
 * Merges together neighboring/overlapping ranges in the range plan with a
 * newly updated range.
 */
static void
merge_neighbors(struct ir3_ubo_analysis_state *state, int index)
{
   struct ir3_ubo_range *a = &state->range[index];

   /* index is always the first slot that would have neighbored/overlapped with
    * the new range.
    */
   for (int i = index + 1; i < state->num_enabled; i++) {
      struct ir3_ubo_range *b = &state->range[i];
      if (memcmp(&a->ubo, &b->ubo, sizeof(a->ubo)))
         continue;

      if (a->start > b->end || a->end < b->start)
         continue;

      /* Merge B into A. */
      a->start = MIN2(a->start, b->start);
      a->end = MAX2(a->end, b->end);

      /* Swap the last enabled range into B's now unused slot */
      *b = state->range[--state->num_enabled];
   }
}

/**
 * During the first pass over the shader, makes the plan of which UBO upload
 * should include the range covering this UBO load.
 *
 * We are passed in an upload_remaining of how much space is left for us in
 * the const file, and we make sure our plan doesn't exceed that.
 */
static void
gather_ubo_ranges(nir_shader *nir, nir_intrinsic_instr *instr,
                  struct ir3_ubo_analysis_state *state, uint32_t alignment,
                  uint32_t *upload_remaining)
{
   struct ir3_ubo_info ubo = {};
   if (!get_ubo_info(instr, &ubo))
      return;

   struct ir3_ubo_range r;
   if (!get_ubo_load_range(nir, instr, alignment, &r))
      return;

   /* See if there's an existing range for this UBO we want to merge into. */
   for (int i = 0; i < state->num_enabled; i++) {
      struct ir3_ubo_range *plan_r = &state->range[i];
      if (memcmp(&plan_r->ubo, &ubo, sizeof(ubo)))
         continue;

      /* Don't extend existing uploads unless they're
       * neighboring/overlapping.
       */
      if (r.start > plan_r->end || r.end < plan_r->start)
         continue;

      r.start = MIN2(r.start, plan_r->start);
      r.end = MAX2(r.end, plan_r->end);

      uint32_t added = (plan_r->start - r.start) + (r.end - plan_r->end);
      if (added >= *upload_remaining)
         return;

      plan_r->start = r.start;
      plan_r->end = r.end;
      *upload_remaining -= added;

      merge_neighbors(state, i);
      return;
   }

   if (state->num_enabled == ARRAY_SIZE(state->range))
      return;

   uint32_t added = r.end - r.start;
   if (added >= *upload_remaining)
      return;

   struct ir3_ubo_range *plan_r = &state->range[state->num_enabled++];
   plan_r->ubo = ubo;
   plan_r->start = r.start;
   plan_r->end = r.end;
   *upload_remaining -= added;
}

/* For indirect offset, it is common to see a pattern of multiple
 * loads with the same base, but different constant offset, ie:
 *
 *    vec1 32 ssa_33 = iadd ssa_base, const_offset
 *    vec4 32 ssa_34 = intrinsic load_uniform (ssa_33) (base=N, 0, 0)
 *
 * Detect this, and peel out the const_offset part, to end up with:
 *
 *    vec4 32 ssa_34 = intrinsic load_uniform (ssa_base) (base=N+const_offset,
 * 0, 0)
 *
 * Or similarly:
 *
 *    vec1 32 ssa_33 = imad24_ir3 a, b, const_offset
 *    vec4 32 ssa_34 = intrinsic load_uniform (ssa_33) (base=N, 0, 0)
 *
 * Can be converted to:
 *
 *    vec1 32 ssa_base = imul24 a, b
 *    vec4 32 ssa_34 = intrinsic load_uniform (ssa_base) (base=N+const_offset,
 * 0, 0)
 *
 * This gives the other opt passes something much easier to work
 * with (ie. not requiring value range tracking)
 */
static void
handle_partial_const(nir_builder *b, nir_def **srcp, int *offp)
{
   if ((*srcp)->parent_instr->type != nir_instr_type_alu)
      return;

   nir_alu_instr *alu = nir_instr_as_alu((*srcp)->parent_instr);

   if (alu->op == nir_op_imad24_ir3) {
      /* This case is slightly more complicated as we need to
       * replace the imad24_ir3 with an imul24:
       */
      if (!nir_src_is_const(alu->src[2].src))
         return;

      *offp += nir_src_as_uint(alu->src[2].src);
      *srcp = nir_imul24(b, nir_ssa_for_alu_src(b, alu, 0),
                         nir_ssa_for_alu_src(b, alu, 1));

      return;
   }

   if (alu->op != nir_op_iadd)
      return;

   if (nir_src_is_const(alu->src[0].src)) {
      *offp += nir_src_as_uint(alu->src[0].src);
      *srcp = alu->src[1].src.ssa;
   } else if (nir_src_is_const(alu->src[1].src)) {
      *srcp = alu->src[0].src.ssa;
      *offp += nir_src_as_uint(alu->src[1].src);
   }
}

/* Tracks the maximum bindful UBO accessed so that we reduce the UBO
 * descriptors emitted in the fast path for GL.
 */
static void
track_ubo_use(nir_intrinsic_instr *instr, nir_builder *b, int *num_ubos)
{
   if (ir3_bindless_resource(instr->src[0])) {
      assert(!b->shader->info.first_ubo_is_default_ubo); /* only set for GL */
      return;
   }

   if (nir_src_is_const(instr->src[0])) {
      int block = nir_src_as_uint(instr->src[0]);
      *num_ubos = MAX2(*num_ubos, block + 1);
   } else {
      *num_ubos = b->shader->info.num_ubos;
   }
}

static bool
lower_ubo_load_to_uniform(nir_intrinsic_instr *instr, nir_builder *b,
                          const struct ir3_ubo_analysis_state *state,
                          int *num_ubos, uint32_t alignment)
{
   b->cursor = nir_before_instr(&instr->instr);

   struct ir3_ubo_range r;
   if (!get_ubo_load_range(b->shader, instr, alignment, &r)) {
      track_ubo_use(instr, b, num_ubos);
      return false;
   }

   /* We don't lower dynamic block index UBO loads to load_uniform, but we
    * could probably with some effort determine a block stride in number of
    * registers.
    */
   const struct ir3_ubo_range *range = get_existing_range(instr, state, &r);
   if (!range) {
      track_ubo_use(instr, b, num_ubos);
      return false;
   }

   nir_def *ubo_offset = instr->src[1].ssa;
   int const_offset = 0;

   handle_partial_const(b, &ubo_offset, &const_offset);

   /* UBO offset is in bytes, but uniform offset is in units of
    * dwords, so we need to divide by 4 (right-shift by 2). For ldc the
    * offset is in units of 16 bytes, so we need to multiply by 4. And
    * also the same for the constant part of the offset:
    */
   const int shift = -2;
   nir_def *new_offset = ir3_nir_try_propagate_bit_shift(b, ubo_offset, -2);
   nir_def *uniform_offset = NULL;
   if (new_offset) {
      uniform_offset = new_offset;
   } else {
      uniform_offset = shift > 0
                          ? nir_ishl_imm(b, ubo_offset, shift)
                          : nir_ushr_imm(b, ubo_offset, -shift);
   }

   assert(!(const_offset & 0x3));
   const_offset >>= 2;

   const int range_offset = ((int)range->offset - (int)range->start) / 4;
   const_offset += range_offset;

   /* The range_offset could be negative, if if only part of the UBO
    * block is accessed, range->start can be greater than range->offset.
    * But we can't underflow const_offset.  If necessary we need to
    * insert nir instructions to compensate (which can hopefully be
    * optimized away)
    */
   if (const_offset < 0) {
      uniform_offset = nir_iadd_imm(b, uniform_offset, const_offset);
      const_offset = 0;
   }

   nir_def *uniform =
      nir_load_uniform(b, instr->num_components, instr->def.bit_size,
                       uniform_offset, .base = const_offset);

   nir_def_rewrite_uses(&instr->def, uniform);

   nir_instr_remove(&instr->instr);

   return true;
}

static bool
copy_ubo_to_uniform(nir_shader *nir, const struct ir3_const_state *const_state)
{
   const struct ir3_ubo_analysis_state *state = &const_state->ubo_state;

   if (state->num_enabled == 0 ||
       (state->num_enabled == 1 && !state->range[0].ubo.bindless &&
        state->range[0].ubo.block == const_state->constant_data_ubo))
      return false;

   nir_function_impl *preamble = nir_shader_get_preamble(nir);
   nir_builder _b = nir_builder_at(nir_after_impl(preamble));
   nir_builder *b = &_b;

   for (unsigned i = 0; i < state->num_enabled; i++) {
      const struct ir3_ubo_range *range = &state->range[i];

      /* The constant_data UBO is pushed in a different path from normal
       * uniforms, and the state is setup earlier so it makes more sense to let
       * the CP do it for us.
       */
      if (!range->ubo.bindless &&
          range->ubo.block == const_state->constant_data_ubo)
         continue;

      nir_def *ubo = nir_imm_int(b, range->ubo.block);
      if (range->ubo.bindless) {
         ubo = nir_bindless_resource_ir3(b, 32, ubo,
                                         .desc_set = range->ubo.bindless_base);
      }

      /* ldc.k has a range of only 256, but there are 512 vec4 constants.
       * Therefore we may have to split a large copy in two.
       */
      unsigned size = (range->end - range->start) / 16;
      for (unsigned offset = 0; offset < size; offset += 256) {
         nir_copy_ubo_to_uniform_ir3(b, ubo, nir_imm_int(b, range->start / 16 +
                                                         offset),
                                     .base = range->offset / 4 + offset * 4,
                                     .range = MIN2(size - offset, 256));
      }
   }

   return true;
}

static bool
instr_is_load_ubo(nir_instr *instr)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_op op = nir_instr_as_intrinsic(instr)->intrinsic;

   /* nir_lower_ubo_vec4 happens after this pass. */
   assert(op != nir_intrinsic_load_ubo_vec4);

   return op == nir_intrinsic_load_ubo;
}

void
ir3_nir_analyze_ubo_ranges(nir_shader *nir, struct ir3_shader_variant *v)
{
   struct ir3_const_state *const_state = ir3_const_state(v);
   struct ir3_ubo_analysis_state *state = &const_state->ubo_state;
   struct ir3_compiler *compiler = v->compiler;

   /* Limit our uploads to the amount of constant buffer space available in
    * the hardware, minus what the shader compiler may need for various
    * driver params.  We do this UBO-to-push-constant before the real
    * allocation of the driver params' const space, because UBO pointers can
    * be driver params but this pass usually eliminatings them.
    */
   struct ir3_const_state worst_case_const_state = {
      .preamble_size = const_state->preamble_size,
   };
   ir3_setup_const_state(nir, v, &worst_case_const_state);
   const uint32_t max_upload =
      (ir3_max_const(v) - worst_case_const_state.offsets.immediate) * 16;

   memset(state, 0, sizeof(*state));

   if (ir3_shader_debug & IR3_DBG_NOUBOOPT)
      return;

   uint32_t upload_remaining = max_upload;
   bool push_ubos = compiler->options.push_ubo_with_preamble;
   nir_foreach_function (function, nir) {
      if (function->impl && (!push_ubos || !function->is_preamble)) {
         nir_foreach_block (block, function->impl) {
            nir_foreach_instr (instr, block) {
               if (instr_is_load_ubo(instr))
                  gather_ubo_ranges(nir, nir_instr_as_intrinsic(instr), state,
                                    compiler->const_upload_unit,
                                    &upload_remaining);
            }
         }
      }
   }

   /* For now, everything we upload is accessed statically and thus will be
    * used by the shader. Once we can upload dynamically indexed data, we may
    * upload sparsely accessed arrays, at which point we probably want to
    * give priority to smaller UBOs, on the assumption that big UBOs will be
    * accessed dynamically.  Alternatively, we can track statically and
    * dynamically accessed ranges separately and upload static rangtes
    * first.
    */

   uint32_t offset = 0;
   for (uint32_t i = 0; i < state->num_enabled; i++) {
      uint32_t range_size = state->range[i].end - state->range[i].start;

      assert(offset <= max_upload);
      state->range[i].offset = offset + v->shader_options.num_reserved_user_consts * 16;
      assert(offset <= max_upload);
      offset += range_size;
   }
   state->size = offset;
}

bool
ir3_nir_lower_ubo_loads(nir_shader *nir, struct ir3_shader_variant *v)
{
   struct ir3_compiler *compiler = v->compiler;
   /* For the binning pass variant, we re-use the corresponding draw-pass
    * variants const_state and ubo state.  To make these clear, in this
    * pass it is const (read-only)
    */
   const struct ir3_const_state *const_state = ir3_const_state(v);
   const struct ir3_ubo_analysis_state *state = &const_state->ubo_state;

   int num_ubos = 0;
   bool progress = false;
   bool has_preamble = false;
   bool push_ubos = compiler->options.push_ubo_with_preamble;
   nir_foreach_function (function, nir) {
      if (function->impl) {
         if (function->is_preamble && push_ubos) {
            has_preamble = true;
            nir_metadata_preserve(function->impl, nir_metadata_all);
            continue;
         }
         nir_builder builder = nir_builder_create(function->impl);
         nir_foreach_block (block, function->impl) {
            nir_foreach_instr_safe (instr, block) {
               if (!instr_is_load_ubo(instr))
                  continue;
               progress |= lower_ubo_load_to_uniform(
                  nir_instr_as_intrinsic(instr), &builder, state, &num_ubos,
                  compiler->const_upload_unit);
            }
         }

         nir_metadata_preserve(
            function->impl, nir_metadata_block_index | nir_metadata_dominance);
      }
   }
   /* Update the num_ubos field for GL (first_ubo_is_default_ubo).  With
    * Vulkan's bindless, we don't use the num_ubos field, so we can leave it
    * incremented.
    */
   if (nir->info.first_ubo_is_default_ubo && !push_ubos && !has_preamble)
      nir->info.num_ubos = num_ubos;

   if (compiler->has_preamble && push_ubos)
      progress |= copy_ubo_to_uniform(nir, const_state);

   return progress;
}

static bool
fixup_load_uniform_filter(const nir_instr *instr, const void *arg)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;
   return nir_instr_as_intrinsic(instr)->intrinsic ==
          nir_intrinsic_load_uniform;
}

static nir_def *
fixup_load_uniform_instr(struct nir_builder *b, nir_instr *instr, void *arg)
{
   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

   /* We don't need to worry about non-indirect case: */
   if (nir_src_is_const(intr->src[0]))
      return NULL;

   const unsigned base_offset_limit = (1 << 9); /* 9 bits */
   unsigned base_offset = nir_intrinsic_base(intr);

   /* Or cases were base offset is lower than the hw limit: */
   if (base_offset < base_offset_limit)
      return NULL;

   b->cursor = nir_before_instr(instr);

   nir_def *offset = intr->src[0].ssa;

   /* We'd like to avoid a sequence like:
    *
    *   vec4 32 ssa_18 = intrinsic load_uniform (ssa_4) (1024, 0, 0)
    *   vec4 32 ssa_19 = intrinsic load_uniform (ssa_4) (1072, 0, 0)
    *   vec4 32 ssa_20 = intrinsic load_uniform (ssa_4) (1120, 0, 0)
    *
    * From turning into a unique offset value (which requires reloading
    * a0.x for each instruction).  So instead of just adding the constant
    * base_offset to the non-const offset, be a bit more clever and only
    * extract the part that cannot be encoded.  Afterwards CSE should
    * turn the result into:
    *
    *   vec1 32 ssa_5 = load_const (1024)
    *   vec4 32 ssa_6  = iadd ssa4_, ssa_5
    *   vec4 32 ssa_18 = intrinsic load_uniform (ssa_5) (0, 0, 0)
    *   vec4 32 ssa_19 = intrinsic load_uniform (ssa_5) (48, 0, 0)
    *   vec4 32 ssa_20 = intrinsic load_uniform (ssa_5) (96, 0, 0)
    */
   unsigned new_base_offset = base_offset % base_offset_limit;

   nir_intrinsic_set_base(intr, new_base_offset);
   offset = nir_iadd_imm(b, offset, base_offset - new_base_offset);

   nir_src_rewrite(&intr->src[0], offset);

   return NIR_LOWER_INSTR_PROGRESS;
}

/**
 * For relative CONST file access, we can only encode 10b worth of fixed offset,
 * so in cases where the base offset is larger, we need to peel it out into
 * ALU instructions.
 *
 * This should run late, after constant folding has had a chance to do it's
 * thing, so we can actually know if it is an indirect uniform offset or not.
 */
bool
ir3_nir_fixup_load_uniform(nir_shader *nir)
{
   return nir_shader_lower_instructions(nir, fixup_load_uniform_filter,
                                        fixup_load_uniform_instr, NULL);
}
static nir_def *
ir3_nir_lower_load_const_instr(nir_builder *b, nir_instr *in_instr, void *data)
{
   struct ir3_const_state *const_state = data;
   nir_intrinsic_instr *instr = nir_instr_as_intrinsic(in_instr);

   /* Pick a UBO index to use as our constant data.  Skip UBO 0 since that's
    * reserved for gallium's cb0.
    */
   if (const_state->constant_data_ubo == -1) {
      if (b->shader->info.num_ubos == 0)
         b->shader->info.num_ubos++;
      const_state->constant_data_ubo = b->shader->info.num_ubos++;
   }

   unsigned num_components = instr->num_components;
   unsigned bit_size = instr->def.bit_size;
   if (instr->def.bit_size == 16) {
      /* We can't do 16b loads -- either from LDC (32-bit only in any of our
       * traces, and disasm that doesn't look like it really supports it) or
       * from the constant file (where CONSTANT_DEMOTION_ENABLE means we get
       * automatic 32b-to-16b conversions when we ask for 16b from it).
       * Instead, we'll load 32b from a UBO and unpack from there.
       */
      num_components = DIV_ROUND_UP(num_components, 2);
      bit_size = 32;
   }
   unsigned base = nir_intrinsic_base(instr);
   nir_def *index = nir_imm_int(b, const_state->constant_data_ubo);
   nir_def *offset =
      nir_iadd_imm(b, instr->src[0].ssa, base);

   nir_def *result =
      nir_load_ubo(b, num_components, bit_size, index, offset,
                   .align_mul = nir_intrinsic_align_mul(instr),
                   .align_offset = nir_intrinsic_align_offset(instr),
                   .range_base = base, .range = nir_intrinsic_range(instr));

   if (instr->def.bit_size == 16) {
      result = nir_bitcast_vector(b, result, 16);
      result = nir_trim_vector(b, result, instr->num_components);
   }

   return result;
}

static bool
ir3_lower_load_const_filter(const nir_instr *instr, const void *data)
{
   return (instr->type == nir_instr_type_intrinsic &&
           nir_instr_as_intrinsic(instr)->intrinsic ==
              nir_intrinsic_load_constant);
}

/* Lowers load_constant intrinsics to UBO accesses so we can run them through
 * the general "upload to const file or leave as UBO access" code.
 */
bool
ir3_nir_lower_load_constant(nir_shader *nir, struct ir3_shader_variant *v)
{
   struct ir3_const_state *const_state = ir3_const_state(v);

   const_state->constant_data_ubo = -1;

   bool progress = nir_shader_lower_instructions(
      nir, ir3_lower_load_const_filter, ir3_nir_lower_load_const_instr,
      const_state);

   if (progress) {
      struct ir3_compiler *compiler = v->compiler;

      /* Save a copy of the NIR constant data to the variant for
       * inclusion in the final assembly.
       */
      v->constant_data_size =
         align(nir->constant_data_size,
               compiler->const_upload_unit * 4 * sizeof(uint32_t));
      v->constant_data = rzalloc_size(v, v->constant_data_size);
      memcpy(v->constant_data, nir->constant_data, nir->constant_data_size);
   }

   return progress;
}
