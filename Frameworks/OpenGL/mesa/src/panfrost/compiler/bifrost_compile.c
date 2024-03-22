/*
 * Copyright (C) 2020 Collabora Ltd.
 * Copyright (C) 2022 Alyssa Rosenzweig <alyssa@rosenzweig.io>
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

#include "compiler/glsl_types.h"
#include "compiler/glsl/glsl_to_nir.h"
#include "compiler/nir/nir_builder.h"
#include "util/u_debug.h"

#include "bifrost/disassemble.h"
#include "valhall/disassemble.h"
#include "valhall/va_compiler.h"
#include "bi_builder.h"
#include "bi_quirks.h"
#include "bifrost_compile.h"
#include "bifrost_nir.h"
#include "compiler.h"

/* clang-format off */
static const struct debug_named_value bifrost_debug_options[] = {
   {"msgs",       BIFROST_DBG_MSGS,		   "Print debug messages"},
   {"shaders",    BIFROST_DBG_SHADERS,	   "Dump shaders in NIR and MIR"},
   {"shaderdb",   BIFROST_DBG_SHADERDB,	"Print statistics"},
   {"verbose",    BIFROST_DBG_VERBOSE,	   "Disassemble verbosely"},
   {"internal",   BIFROST_DBG_INTERNAL,	"Dump even internal shaders"},
   {"nosched",    BIFROST_DBG_NOSCHED, 	"Force trivial bundling"},
   {"nopsched",   BIFROST_DBG_NOPSCHED,   "Disable scheduling for pressure"},
   {"inorder",    BIFROST_DBG_INORDER, 	"Force in-order bundling"},
   {"novalidate", BIFROST_DBG_NOVALIDATE, "Skip IR validation"},
   {"noopt",      BIFROST_DBG_NOOPT,      "Skip optimization passes"},
   {"noidvs",     BIFROST_DBG_NOIDVS,     "Disable IDVS"},
   {"nosb",       BIFROST_DBG_NOSB,       "Disable scoreboarding"},
   {"nopreload",  BIFROST_DBG_NOPRELOAD,  "Disable message preloading"},
   {"spill",      BIFROST_DBG_SPILL,      "Test register spilling"},
   DEBUG_NAMED_VALUE_END
};
/* clang-format on */

DEBUG_GET_ONCE_FLAGS_OPTION(bifrost_debug, "BIFROST_MESA_DEBUG",
                            bifrost_debug_options, 0)

/* How many bytes are prefetched by the Bifrost shader core. From the final
 * clause of the shader, this range must be valid instructions or zero. */
#define BIFROST_SHADER_PREFETCH 128

int bifrost_debug = 0;

#define DBG(fmt, ...)                                                          \
   do {                                                                        \
      if (bifrost_debug & BIFROST_DBG_MSGS)                                    \
         fprintf(stderr, "%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__);    \
   } while (0)

static bi_block *emit_cf_list(bi_context *ctx, struct exec_list *list);

static bi_index
bi_preload(bi_builder *b, unsigned reg)
{
   if (bi_is_null(b->shader->preloaded[reg])) {
      /* Insert at the beginning of the shader */
      bi_builder b_ = *b;
      b_.cursor = bi_before_block(bi_start_block(&b->shader->blocks));

      /* Cache the result */
      b->shader->preloaded[reg] = bi_mov_i32(&b_, bi_register(reg));
   }

   return b->shader->preloaded[reg];
}

static bi_index
bi_coverage(bi_builder *b)
{
   if (bi_is_null(b->shader->coverage))
      b->shader->coverage = bi_preload(b, 60);

   return b->shader->coverage;
}

/*
 * Vertex ID and Instance ID are preloaded registers. Where they are preloaded
 * changed from Bifrost to Valhall. Provide helpers that smooth over the
 * architectural difference.
 */
static inline bi_index
bi_vertex_id(bi_builder *b)
{
   return bi_preload(b, (b->shader->arch >= 9) ? 60 : 61);
}

static inline bi_index
bi_instance_id(bi_builder *b)
{
   return bi_preload(b, (b->shader->arch >= 9) ? 61 : 62);
}

static void
bi_emit_jump(bi_builder *b, nir_jump_instr *instr)
{
   bi_instr *branch = bi_jump(b, bi_zero());

   switch (instr->type) {
   case nir_jump_break:
      branch->branch_target = b->shader->break_block;
      break;
   case nir_jump_continue:
      branch->branch_target = b->shader->continue_block;
      break;
   default:
      unreachable("Unhandled jump type");
   }

   bi_block_add_successor(b->shader->current_block, branch->branch_target);
   b->shader->current_block->unconditional_jumps = true;
}

/* Builds a 64-bit hash table key for an index */
static uint64_t
bi_index_to_key(bi_index idx)
{
   static_assert(sizeof(idx) <= sizeof(uint64_t), "too much padding");

   uint64_t key = 0;
   memcpy(&key, &idx, sizeof(idx));
   return key;
}

/*
 * Extract a single channel out of a vector source. We split vectors with SPLIT
 * so we can use the split components directly, without emitting an extract.
 * This has advantages of RA, as the split can usually be optimized away.
 */
static bi_index
bi_extract(bi_builder *b, bi_index vec, unsigned channel)
{
   bi_index *components = _mesa_hash_table_u64_search(b->shader->allocated_vec,
                                                      bi_index_to_key(vec));

   /* No extract needed for scalars.
    *
    * This is a bit imprecise, but actual bugs (missing splits for vectors)
    * should be caught by the following assertion. It is too difficult to
    * ensure bi_extract is only called for real vectors.
    */
   if (components == NULL && channel == 0)
      return vec;

   assert(components != NULL && "missing bi_cache_collect()");
   return components[channel];
}

static void
bi_cache_collect(bi_builder *b, bi_index dst, bi_index *s, unsigned n)
{
   /* Lifetime of a hash table entry has to be at least as long as the table */
   bi_index *channels = ralloc_array(b->shader, bi_index, n);
   memcpy(channels, s, sizeof(bi_index) * n);

   _mesa_hash_table_u64_insert(b->shader->allocated_vec, bi_index_to_key(dst),
                               channels);
}

/*
 * Splits an n-component vector (vec) into n scalar destinations (dests) using a
 * split pseudo-instruction.
 *
 * Pre-condition: dests is filled with bi_null().
 */
static void
bi_emit_split_i32(bi_builder *b, bi_index dests[4], bi_index vec, unsigned n)
{
   /* Setup the destinations */
   for (unsigned i = 0; i < n; ++i) {
      dests[i] = bi_temp(b->shader);
   }

   /* Emit the split */
   if (n == 1) {
      bi_mov_i32_to(b, dests[0], vec);
   } else {
      bi_instr *I = bi_split_i32_to(b, n, vec);

      bi_foreach_dest(I, j)
         I->dest[j] = dests[j];
   }
}

static void
bi_emit_cached_split_i32(bi_builder *b, bi_index vec, unsigned n)
{
   bi_index dests[4] = {bi_null(), bi_null(), bi_null(), bi_null()};
   bi_emit_split_i32(b, dests, vec, n);
   bi_cache_collect(b, vec, dests, n);
}

/*
 * Emit and cache a split for a vector of a given bitsize. The vector may not be
 * composed of 32-bit words, but it will be split at 32-bit word boundaries.
 */
static void
bi_emit_cached_split(bi_builder *b, bi_index vec, unsigned bits)
{
   bi_emit_cached_split_i32(b, vec, DIV_ROUND_UP(bits, 32));
}

static void
bi_split_def(bi_builder *b, nir_def *def)
{
   bi_emit_cached_split(b, bi_def_index(def),
                        def->bit_size * def->num_components);
}

static bi_instr *
bi_emit_collect_to(bi_builder *b, bi_index dst, bi_index *chan, unsigned n)
{
   /* Special case: COLLECT of a single value is a scalar move */
   if (n == 1)
      return bi_mov_i32_to(b, dst, chan[0]);

   bi_instr *I = bi_collect_i32_to(b, dst, n);

   bi_foreach_src(I, i)
      I->src[i] = chan[i];

   bi_cache_collect(b, dst, chan, n);
   return I;
}

static bi_instr *
bi_collect_v2i32_to(bi_builder *b, bi_index dst, bi_index s0, bi_index s1)
{
   return bi_emit_collect_to(b, dst, (bi_index[]){s0, s1}, 2);
}

static bi_instr *
bi_collect_v3i32_to(bi_builder *b, bi_index dst, bi_index s0, bi_index s1,
                    bi_index s2)
{
   return bi_emit_collect_to(b, dst, (bi_index[]){s0, s1, s2}, 3);
}

static bi_index
bi_collect_v2i32(bi_builder *b, bi_index s0, bi_index s1)
{
   bi_index dst = bi_temp(b->shader);
   bi_collect_v2i32_to(b, dst, s0, s1);
   return dst;
}

static bi_index
bi_varying_src0_for_barycentric(bi_builder *b, nir_intrinsic_instr *intr)
{
   switch (intr->intrinsic) {
   case nir_intrinsic_load_barycentric_centroid:
   case nir_intrinsic_load_barycentric_sample:
      return bi_preload(b, 61);

   /* Need to put the sample ID in the top 16-bits */
   case nir_intrinsic_load_barycentric_at_sample:
      return bi_mkvec_v2i16(b, bi_half(bi_dontcare(b), false),
                            bi_half(bi_src_index(&intr->src[0]), false));

   /* Interpret as 8:8 signed fixed point positions in pixels along X and
    * Y axes respectively, relative to top-left of pixel. In NIR, (0, 0)
    * is the center of the pixel so we first fixup and then convert. For
    * fp16 input:
    *
    * f2i16(((x, y) + (0.5, 0.5)) * 2**8) =
    * f2i16((256 * (x, y)) + (128, 128)) =
    * V2F16_TO_V2S16(FMA.v2f16((x, y), #256, #128))
    *
    * For fp32 input, that lacks enough precision for MSAA 16x, but the
    * idea is the same. FIXME: still doesn't pass
    */
   case nir_intrinsic_load_barycentric_at_offset: {
      bi_index offset = bi_src_index(&intr->src[0]);
      bi_index f16 = bi_null();
      unsigned sz = nir_src_bit_size(intr->src[0]);

      if (sz == 16) {
         f16 = bi_fma_v2f16(b, offset, bi_imm_f16(256.0), bi_imm_f16(128.0));
      } else {
         assert(sz == 32);
         bi_index f[2];
         for (unsigned i = 0; i < 2; ++i) {
            f[i] =
               bi_fadd_rscale_f32(b, bi_extract(b, offset, i), bi_imm_f32(0.5),
                                  bi_imm_u32(8), BI_SPECIAL_NONE);
         }

         f16 = bi_v2f32_to_v2f16(b, f[0], f[1]);
      }

      return bi_v2f16_to_v2s16(b, f16);
   }

   case nir_intrinsic_load_barycentric_pixel:
   default:
      return b->shader->arch >= 9 ? bi_preload(b, 61) : bi_dontcare(b);
   }
}

static enum bi_sample
bi_interp_for_intrinsic(nir_intrinsic_op op)
{
   switch (op) {
   case nir_intrinsic_load_barycentric_centroid:
      return BI_SAMPLE_CENTROID;
   case nir_intrinsic_load_barycentric_sample:
   case nir_intrinsic_load_barycentric_at_sample:
      return BI_SAMPLE_SAMPLE;
   case nir_intrinsic_load_barycentric_at_offset:
      return BI_SAMPLE_EXPLICIT;
   case nir_intrinsic_load_barycentric_pixel:
   default:
      return BI_SAMPLE_CENTER;
   }
}

/* auto, 64-bit omitted */
static enum bi_register_format
bi_reg_fmt_for_nir(nir_alu_type T)
{
   switch (T) {
   case nir_type_float16:
      return BI_REGISTER_FORMAT_F16;
   case nir_type_float32:
      return BI_REGISTER_FORMAT_F32;
   case nir_type_int16:
      return BI_REGISTER_FORMAT_S16;
   case nir_type_uint16:
      return BI_REGISTER_FORMAT_U16;
   case nir_type_int32:
      return BI_REGISTER_FORMAT_S32;
   case nir_type_uint32:
      return BI_REGISTER_FORMAT_U32;
   default:
      unreachable("Invalid type for register format");
   }
}

/* Checks if the _IMM variant of an intrinsic can be used, returning in imm the
 * immediate to be used (which applies even if _IMM can't be used) */

static bool
bi_is_intr_immediate(nir_intrinsic_instr *instr, unsigned *immediate,
                     unsigned max)
{
   nir_src *offset = nir_get_io_offset_src(instr);

   if (!nir_src_is_const(*offset))
      return false;

   *immediate = nir_intrinsic_base(instr) + nir_src_as_uint(*offset);
   return (*immediate) < max;
}

static void bi_make_vec_to(bi_builder *b, bi_index final_dst, bi_index *src,
                           unsigned *channel, unsigned count, unsigned bitsize);

/* Bifrost's load instructions lack a component offset despite operating in
 * terms of vec4 slots. Usually I/O vectorization avoids nonzero components,
 * but they may be unavoidable with separate shaders in use. To solve this, we
 * lower to a larger load and an explicit copy of the desired components. */

static void
bi_copy_component(bi_builder *b, nir_intrinsic_instr *instr, bi_index tmp)
{
   unsigned component = nir_intrinsic_component(instr);
   unsigned nr = instr->num_components;
   unsigned total = nr + component;
   unsigned bitsize = instr->def.bit_size;

   assert(total <= 4 && "should be vec4");
   bi_emit_cached_split(b, tmp, total * bitsize);

   if (component == 0)
      return;

   bi_index srcs[] = {tmp, tmp, tmp};
   unsigned channels[] = {component, component + 1, component + 2};

   bi_make_vec_to(b, bi_def_index(&instr->def), srcs, channels, nr,
                  instr->def.bit_size);
}

static void
bi_emit_load_attr(bi_builder *b, nir_intrinsic_instr *instr)
{
   /* Disregard the signedness of an integer, since loading 32-bits into a
    * 32-bit register should be bit exact so should not incur any clamping.
    *
    * If we are reading as a u32, then it must be paired with an integer (u32 or
    * s32) source, so use .auto32 to disregard.
    */
   nir_alu_type T = nir_intrinsic_dest_type(instr);
   assert(T == nir_type_uint32 || T == nir_type_int32 || T == nir_type_float32);
   enum bi_register_format regfmt =
      T == nir_type_float32 ? BI_REGISTER_FORMAT_F32 : BI_REGISTER_FORMAT_AUTO;

   nir_src *offset = nir_get_io_offset_src(instr);
   unsigned component = nir_intrinsic_component(instr);
   enum bi_vecsize vecsize = (instr->num_components + component - 1);
   unsigned imm_index = 0;
   unsigned base = nir_intrinsic_base(instr);
   bool constant = nir_src_is_const(*offset);
   bool immediate = bi_is_intr_immediate(instr, &imm_index, 16);
   bi_index dest =
      (component == 0) ? bi_def_index(&instr->def) : bi_temp(b->shader);
   bi_instr *I;

   if (immediate) {
      I = bi_ld_attr_imm_to(b, dest, bi_vertex_id(b), bi_instance_id(b), regfmt,
                            vecsize, imm_index);
   } else {
      bi_index idx = bi_src_index(&instr->src[0]);

      if (constant)
         idx = bi_imm_u32(imm_index);
      else if (base != 0)
         idx = bi_iadd_u32(b, idx, bi_imm_u32(base), false);

      I = bi_ld_attr_to(b, dest, bi_vertex_id(b), bi_instance_id(b), idx,
                        regfmt, vecsize);
   }

   if (b->shader->arch >= 9)
      I->table = PAN_TABLE_ATTRIBUTE;

   bi_copy_component(b, instr, dest);
}

/*
 * ABI: Special (desktop GL) slots come first, tightly packed. General varyings
 * come later, sparsely packed. This handles both linked and separable shaders
 * with a common code path, with minimal keying only for desktop GL. Each slot
 * consumes 16 bytes (TODO: fp16, partial vectors).
 */
static unsigned
bi_varying_base_bytes(bi_context *ctx, nir_intrinsic_instr *intr)
{
   nir_io_semantics sem = nir_intrinsic_io_semantics(intr);
   uint32_t mask = ctx->inputs->fixed_varying_mask;

   if (sem.location >= VARYING_SLOT_VAR0) {
      unsigned nr_special = util_bitcount(mask);
      unsigned general_index = (sem.location - VARYING_SLOT_VAR0);

      return 16 * (nr_special + general_index);
   } else {
      return 16 * (util_bitcount(mask & BITFIELD_MASK(sem.location)));
   }
}

/*
 * Compute the offset in bytes of a varying with an immediate offset, adding the
 * offset to the base computed above. Convenience method.
 */
static unsigned
bi_varying_offset(bi_context *ctx, nir_intrinsic_instr *intr)
{
   nir_src *src = nir_get_io_offset_src(intr);
   assert(nir_src_is_const(*src) && "assumes immediate offset");

   return bi_varying_base_bytes(ctx, intr) + (nir_src_as_uint(*src) * 16);
}

static void
bi_emit_load_vary(bi_builder *b, nir_intrinsic_instr *instr)
{
   enum bi_sample sample = BI_SAMPLE_CENTER;
   enum bi_update update = BI_UPDATE_STORE;
   enum bi_register_format regfmt = BI_REGISTER_FORMAT_AUTO;
   bool smooth = instr->intrinsic == nir_intrinsic_load_interpolated_input;
   bi_index src0 = bi_null();

   unsigned component = nir_intrinsic_component(instr);
   enum bi_vecsize vecsize = (instr->num_components + component - 1);
   bi_index dest =
      (component == 0) ? bi_def_index(&instr->def) : bi_temp(b->shader);

   unsigned sz = instr->def.bit_size;

   if (smooth) {
      nir_intrinsic_instr *parent = nir_src_as_intrinsic(instr->src[0]);
      assert(parent);

      sample = bi_interp_for_intrinsic(parent->intrinsic);
      src0 = bi_varying_src0_for_barycentric(b, parent);

      assert(sz == 16 || sz == 32);
      regfmt = (sz == 16) ? BI_REGISTER_FORMAT_F16 : BI_REGISTER_FORMAT_F32;
   } else {
      assert(sz == 32);
      regfmt = BI_REGISTER_FORMAT_U32;

      /* Valhall can't have bi_null() here, although the source is
       * logically unused for flat varyings
       */
      if (b->shader->arch >= 9)
         src0 = bi_preload(b, 61);

      /* Gather info as we go */
      b->shader->info.bifrost->uses_flat_shading = true;
   }

   enum bi_source_format source_format =
      smooth ? BI_SOURCE_FORMAT_F32 : BI_SOURCE_FORMAT_FLAT32;

   nir_src *offset = nir_get_io_offset_src(instr);
   unsigned imm_index = 0;
   bool immediate = bi_is_intr_immediate(instr, &imm_index, 20);
   bi_instr *I = NULL;

   if (b->shader->malloc_idvs && immediate) {
      /* Immediate index given in bytes. */
      bi_ld_var_buf_imm_to(b, sz, dest, src0, regfmt, sample, source_format,
                           update, vecsize,
                           bi_varying_offset(b->shader, instr));
   } else if (immediate && smooth) {
      I = bi_ld_var_imm_to(b, dest, src0, regfmt, sample, update, vecsize,
                           imm_index);
   } else if (immediate && !smooth) {
      I = bi_ld_var_flat_imm_to(b, dest, BI_FUNCTION_NONE, regfmt, vecsize,
                                imm_index);
   } else {
      bi_index idx = bi_src_index(offset);
      unsigned base = nir_intrinsic_base(instr);

      if (b->shader->malloc_idvs) {
         /* Index needs to be in bytes, but NIR gives the index
          * in slots. For now assume 16 bytes per element.
          */
         bi_index idx_bytes = bi_lshift_or_i32(b, idx, bi_zero(), bi_imm_u8(4));
         unsigned vbase = bi_varying_base_bytes(b->shader, instr);

         if (vbase != 0)
            idx_bytes = bi_iadd_u32(b, idx, bi_imm_u32(vbase), false);

         bi_ld_var_buf_to(b, sz, dest, src0, idx_bytes, regfmt, sample,
                          source_format, update, vecsize);
      } else if (smooth) {
         if (base != 0)
            idx = bi_iadd_u32(b, idx, bi_imm_u32(base), false);

         I = bi_ld_var_to(b, dest, src0, idx, regfmt, sample, update, vecsize);
      } else {
         if (base != 0)
            idx = bi_iadd_u32(b, idx, bi_imm_u32(base), false);

         I = bi_ld_var_flat_to(b, dest, idx, BI_FUNCTION_NONE, regfmt, vecsize);
      }
   }

   /* Valhall usually uses machine-allocated IDVS. If this is disabled, use
    * a simple Midgard-style ABI.
    */
   if (b->shader->arch >= 9 && I != NULL)
      I->table = PAN_TABLE_ATTRIBUTE;

   bi_copy_component(b, instr, dest);
}

static bi_index
bi_make_vec8_helper(bi_builder *b, bi_index *src, unsigned *channel,
                    unsigned count)
{
   assert(1 <= count && count <= 4);

   bi_index bytes[4] = {bi_imm_u8(0), bi_imm_u8(0), bi_imm_u8(0), bi_imm_u8(0)};

   for (unsigned i = 0; i < count; ++i) {
      unsigned chan = channel ? channel[i] : 0;

      bytes[i] = bi_byte(bi_extract(b, src[i], chan >> 2), chan & 3);
   }

   if (b->shader->arch >= 9) {
      bi_index vec = bi_zero();

      if (count >= 3)
         vec = bi_mkvec_v2i8(b, bytes[2], bytes[3], vec);

      return bi_mkvec_v2i8(b, bytes[0], bytes[1], vec);
   } else {
      return bi_mkvec_v4i8(b, bytes[0], bytes[1], bytes[2], bytes[3]);
   }
}

static bi_index
bi_make_vec16_helper(bi_builder *b, bi_index *src, unsigned *channel,
                     unsigned count)
{
   unsigned chan0 = channel ? channel[0] : 0;
   bi_index w0 = bi_extract(b, src[0], chan0 >> 1);
   bi_index h0 = bi_half(w0, chan0 & 1);

   /* Zero extend */
   if (count == 1)
      return bi_mkvec_v2i16(b, h0, bi_imm_u16(0));

   /* Else, create a vector */
   assert(count == 2);

   unsigned chan1 = channel ? channel[1] : 0;
   bi_index w1 = bi_extract(b, src[1], chan1 >> 1);
   bi_index h1 = bi_half(w1, chan1 & 1);

   if (bi_is_word_equiv(w0, w1) && (chan0 & 1) == 0 && ((chan1 & 1) == 1))
      return bi_mov_i32(b, w0);
   else if (bi_is_word_equiv(w0, w1))
      return bi_swz_v2i16(b, bi_swz_16(w0, chan0 & 1, chan1 & 1));
   else
      return bi_mkvec_v2i16(b, h0, h1);
}

static void
bi_make_vec_to(bi_builder *b, bi_index dst, bi_index *src, unsigned *channel,
               unsigned count, unsigned bitsize)
{
   assert(bitsize == 8 || bitsize == 16 || bitsize == 32);
   unsigned shift = (bitsize == 32) ? 0 : (bitsize == 16) ? 1 : 2;
   unsigned chan_per_word = 1 << shift;

   assert(DIV_ROUND_UP(count * bitsize, 32) <= BI_MAX_SRCS &&
          "unnecessarily large vector should have been lowered");

   bi_index srcs[BI_MAX_VEC];

   for (unsigned i = 0; i < count; i += chan_per_word) {
      unsigned rem = MIN2(count - i, chan_per_word);
      unsigned *channel_offset = channel ? (channel + i) : NULL;

      if (bitsize == 32)
         srcs[i] = bi_extract(b, src[i], channel_offset ? *channel_offset : 0);
      else if (bitsize == 16)
         srcs[i >> 1] = bi_make_vec16_helper(b, src + i, channel_offset, rem);
      else
         srcs[i >> 2] = bi_make_vec8_helper(b, src + i, channel_offset, rem);
   }

   bi_emit_collect_to(b, dst, srcs, DIV_ROUND_UP(count, chan_per_word));
}

static inline bi_instr *
bi_load_ubo_to(bi_builder *b, unsigned bitsize, bi_index dest0, bi_index src0,
               bi_index src1)
{
   bi_instr *I;

   if (b->shader->arch >= 9) {
      I = bi_ld_buffer_to(b, bitsize, dest0, src0, src1);
      I->seg = BI_SEG_UBO;
   } else {
      I = bi_load_to(b, bitsize, dest0, src0, src1, BI_SEG_UBO, 0);
   }

   bi_emit_cached_split(b, dest0, bitsize);
   return I;
}

static void
bi_load_sample_id_to(bi_builder *b, bi_index dst)
{
   /* r61[16:23] contains the sampleID, mask it out. Upper bits
    * seem to read garbage (despite being architecturally defined
    * as zero), so use a 5-bit mask instead of 8-bits */

   bi_rshift_and_i32_to(b, dst, bi_preload(b, 61), bi_imm_u32(0x1f),
                        bi_imm_u8(16), false);
}

static bi_index
bi_load_sample_id(bi_builder *b)
{
   bi_index sample_id = bi_temp(b->shader);
   bi_load_sample_id_to(b, sample_id);
   return sample_id;
}

static bi_index
bi_pixel_indices(bi_builder *b, unsigned rt)
{
   /* We want to load the current pixel. */
   struct bifrost_pixel_indices pix = {.y = BIFROST_CURRENT_PIXEL, .rt = rt};

   uint32_t indices_u32 = 0;
   memcpy(&indices_u32, &pix, sizeof(indices_u32));
   bi_index indices = bi_imm_u32(indices_u32);

   /* Sample index above is left as zero. For multisampling, we need to
    * fill in the actual sample ID in the lower byte */

   if (b->shader->inputs->blend.nr_samples > 1)
      indices = bi_iadd_u32(b, indices, bi_load_sample_id(b), false);

   return indices;
}

/* Source color is passed through r0-r3, or r4-r7 for the second source when
 * dual-source blending. Preload the corresponding vector.
 */
static void
bi_emit_load_blend_input(bi_builder *b, nir_intrinsic_instr *instr)
{
   nir_io_semantics sem = nir_intrinsic_io_semantics(instr);
   unsigned base = (sem.location == VARYING_SLOT_VAR0) ? 4 : 0;
   unsigned size = nir_alu_type_get_type_size(nir_intrinsic_dest_type(instr));
   assert(size == 16 || size == 32);

   bi_index srcs[] = {bi_preload(b, base + 0), bi_preload(b, base + 1),
                      bi_preload(b, base + 2), bi_preload(b, base + 3)};

   bi_emit_collect_to(b, bi_def_index(&instr->def), srcs, size == 32 ? 4 : 2);
}

static void
bi_emit_blend_op(bi_builder *b, bi_index rgba, nir_alu_type T, bi_index rgba2,
                 nir_alu_type T2, unsigned rt)
{
   /* Reads 2 or 4 staging registers to cover the input */
   unsigned size = nir_alu_type_get_type_size(T);
   unsigned size_2 = nir_alu_type_get_type_size(T2);
   unsigned sr_count = (size <= 16) ? 2 : 4;
   unsigned sr_count_2 = (size_2 <= 16) ? 2 : 4;
   const struct panfrost_compile_inputs *inputs = b->shader->inputs;
   uint64_t blend_desc = inputs->blend.bifrost_blend_desc;
   enum bi_register_format regfmt = bi_reg_fmt_for_nir(T);

   /* Workaround for NIR-to-TGSI */
   if (b->shader->nir->info.fs.untyped_color_outputs)
      regfmt = BI_REGISTER_FORMAT_AUTO;

   if (inputs->is_blend && inputs->blend.nr_samples > 1) {
      /* Conversion descriptor comes from the compile inputs, pixel
       * indices derived at run time based on sample ID */
      bi_st_tile(b, rgba, bi_pixel_indices(b, rt), bi_coverage(b),
                 bi_imm_u32(blend_desc >> 32), regfmt, BI_VECSIZE_V4);
   } else if (b->shader->inputs->is_blend) {
      uint64_t blend_desc = b->shader->inputs->blend.bifrost_blend_desc;

      /* Blend descriptor comes from the compile inputs */
      /* Put the result in r0 */

      bi_blend_to(b, bi_temp(b->shader), rgba, bi_coverage(b),
                  bi_imm_u32(blend_desc), bi_imm_u32(blend_desc >> 32),
                  bi_null(), regfmt, sr_count, 0);
   } else {
      /* Blend descriptor comes from the FAU RAM. By convention, the
       * return address on Bifrost is stored in r48 and will be used
       * by the blend shader to jump back to the fragment shader */

      bi_blend_to(b, bi_temp(b->shader), rgba, bi_coverage(b),
                  bi_fau(BIR_FAU_BLEND_0 + rt, false),
                  bi_fau(BIR_FAU_BLEND_0 + rt, true), rgba2, regfmt, sr_count,
                  sr_count_2);
   }

   assert(rt < 8);
   b->shader->info.bifrost->blend[rt].type = T;

   if (T2)
      b->shader->info.bifrost->blend_src1_type = T2;
}

/* Blend shaders do not need to run ATEST since they are dependent on a
 * fragment shader that runs it. Blit shaders may not need to run ATEST, since
 * ATEST is not needed if early-z is forced, alpha-to-coverage is disabled, and
 * there are no writes to the coverage mask. The latter two are satisfied for
 * all blit shaders, so we just care about early-z, which blit shaders force
 * iff they do not write depth or stencil */

static bool
bi_skip_atest(bi_context *ctx, bool emit_zs)
{
   return (ctx->inputs->is_blit && !emit_zs) || ctx->inputs->is_blend;
}

static void
bi_emit_atest(bi_builder *b, bi_index alpha)
{
   b->shader->coverage =
      bi_atest(b, bi_coverage(b), alpha, bi_fau(BIR_FAU_ATEST_PARAM, false));
   b->shader->emitted_atest = true;
}

static void
bi_emit_fragment_out(bi_builder *b, nir_intrinsic_instr *instr)
{
   bool combined = instr->intrinsic == nir_intrinsic_store_combined_output_pan;

   unsigned writeout =
      combined ? nir_intrinsic_component(instr) : PAN_WRITEOUT_C;

   bool emit_blend = writeout & (PAN_WRITEOUT_C);
   bool emit_zs = writeout & (PAN_WRITEOUT_Z | PAN_WRITEOUT_S);

   unsigned loc = nir_intrinsic_io_semantics(instr).location;
   bi_index src0 = bi_src_index(&instr->src[0]);

   /* By ISA convention, the coverage mask is stored in R60. The store
    * itself will be handled by a subsequent ATEST instruction */
   if (loc == FRAG_RESULT_SAMPLE_MASK) {
      b->shader->coverage = bi_extract(b, src0, 0);
      return;
   }

   /* Emit ATEST if we have to, note ATEST requires a floating-point alpha
    * value, but render target #0 might not be floating point. However the
    * alpha value is only used for alpha-to-coverage, a stage which is
    * skipped for pure integer framebuffers, so the issue is moot. */

   if (!b->shader->emitted_atest && !bi_skip_atest(b->shader, emit_zs)) {
      nir_alu_type T = nir_intrinsic_src_type(instr);

      bi_index rgba = bi_src_index(&instr->src[0]);
      bi_index alpha;

      if (nir_src_num_components(instr->src[0]) < 4) {
         /* Don't read out-of-bounds */
         alpha = bi_imm_f32(1.0);
      } else if (T == nir_type_float16) {
         alpha = bi_half(bi_extract(b, rgba, 1), true);
      } else if (T == nir_type_float32) {
         alpha = bi_extract(b, rgba, 3);
      } else {
         alpha = bi_dontcare(b);
      }
      bi_emit_atest(b, alpha);
   }

   if (emit_zs) {
      bi_index z = bi_dontcare(b), s = bi_dontcare(b);

      if (writeout & PAN_WRITEOUT_Z)
         z = bi_src_index(&instr->src[2]);

      if (writeout & PAN_WRITEOUT_S)
         s = bi_src_index(&instr->src[3]);

      b->shader->coverage =
         bi_zs_emit(b, z, s, bi_coverage(b), writeout & PAN_WRITEOUT_S,
                    writeout & PAN_WRITEOUT_Z);
   }

   if (emit_blend) {
      unsigned rt = loc ? (loc - FRAG_RESULT_DATA0) : 0;
      bool dual = (writeout & PAN_WRITEOUT_2);
      bi_index color = bi_src_index(&instr->src[0]);
      bi_index color2 = dual ? bi_src_index(&instr->src[4]) : bi_null();
      nir_alu_type T2 = dual ? nir_intrinsic_dest_type(instr) : 0;

      /* Explicit copy since BLEND inputs are precoloured to R0-R3,
       * TODO: maybe schedule around this or implement in RA as a
       * spill */
      bool has_mrt =
         (b->shader->nir->info.outputs_written >> FRAG_RESULT_DATA1);

      if (has_mrt) {
         bi_index srcs[4] = {color, color, color, color};
         unsigned channels[4] = {0, 1, 2, 3};
         color = bi_temp(b->shader);
         bi_make_vec_to(
            b, color, srcs, channels, nir_src_num_components(instr->src[0]),
            nir_alu_type_get_type_size(nir_intrinsic_src_type(instr)));
      }

      bi_emit_blend_op(b, color, nir_intrinsic_src_type(instr), color2, T2, rt);
   }

   if (b->shader->inputs->is_blend) {
      /* Jump back to the fragment shader, return address is stored
       * in r48 (see above). On Valhall, only jump if the address is
       * nonzero. The check is free there and it implements the "jump
       * to 0 terminates the blend shader" that's automatic on
       * Bifrost.
       */
      if (b->shader->arch >= 8)
         bi_branchzi(b, bi_preload(b, 48), bi_preload(b, 48), BI_CMPF_NE);
      else
         bi_jump(b, bi_preload(b, 48));
   }
}

/**
 * In a vertex shader, is the specified variable a position output? These kinds
 * of outputs are written from position shaders when IDVS is enabled. All other
 * outputs are written from the varying shader.
 */
static bool
bi_should_remove_store(nir_intrinsic_instr *intr, enum bi_idvs_mode idvs)
{
   nir_io_semantics sem = nir_intrinsic_io_semantics(intr);

   switch (sem.location) {
   case VARYING_SLOT_POS:
   case VARYING_SLOT_PSIZ:
      return idvs == BI_IDVS_VARYING;
   default:
      return idvs == BI_IDVS_POSITION;
   }
}

static bool
bifrost_nir_specialize_idvs(nir_builder *b, nir_instr *instr, void *data)
{
   enum bi_idvs_mode *idvs = data;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

   if (intr->intrinsic != nir_intrinsic_store_output)
      return false;

   if (bi_should_remove_store(intr, *idvs)) {
      nir_instr_remove(instr);
      return true;
   }

   return false;
}

static void
bi_emit_store_vary(bi_builder *b, nir_intrinsic_instr *instr)
{
   /* In principle we can do better for 16-bit. At the moment we require
    * 32-bit to permit the use of .auto, in order to force .u32 for flat
    * varyings, to handle internal TGSI shaders that set flat in the VS
    * but smooth in the FS */

   ASSERTED nir_alu_type T = nir_intrinsic_src_type(instr);
   ASSERTED unsigned T_size = nir_alu_type_get_type_size(T);
   assert(T_size == 32 || (b->shader->arch >= 9 && T_size == 16));
   enum bi_register_format regfmt = BI_REGISTER_FORMAT_AUTO;

   unsigned imm_index = 0;
   bool immediate = bi_is_intr_immediate(instr, &imm_index, 16);

   /* Only look at the total components needed. In effect, we fill in all
    * the intermediate "holes" in the write mask, since we can't mask off
    * stores. Since nir_lower_io_to_temporaries ensures each varying is
    * written at most once, anything that's masked out is undefined, so it
    * doesn't matter what we write there. So we may as well do the
    * simplest thing possible. */
   unsigned nr = util_last_bit(nir_intrinsic_write_mask(instr));
   assert(nr > 0 && nr <= nir_intrinsic_src_components(instr, 0));

   bi_index data = bi_src_index(&instr->src[0]);

   /* To keep the vector dimensions consistent, we need to drop some
    * components. This should be coalesced.
    *
    * TODO: This is ugly and maybe inefficient. Would we rather
    * introduce a TRIM.i32 pseudoinstruction?
    */
   if (nr < nir_intrinsic_src_components(instr, 0)) {
      assert(T_size == 32 && "todo: 16-bit trim");

      bi_index chans[4] = {bi_null(), bi_null(), bi_null(), bi_null()};
      unsigned src_comps = nir_intrinsic_src_components(instr, 0);

      bi_emit_split_i32(b, chans, data, src_comps);

      bi_index tmp = bi_temp(b->shader);
      bi_instr *collect = bi_collect_i32_to(b, tmp, nr);

      bi_foreach_src(collect, w)
         collect->src[w] = chans[w];

      data = tmp;
   }

   bool psiz =
      (nir_intrinsic_io_semantics(instr).location == VARYING_SLOT_PSIZ);

   bi_index a[4] = {bi_null()};

   if (b->shader->arch <= 8 && b->shader->idvs == BI_IDVS_POSITION) {
      /* Bifrost position shaders have a fast path */
      assert(T == nir_type_float16 || T == nir_type_float32);
      unsigned regfmt = (T == nir_type_float16) ? 0 : 1;
      unsigned identity = (b->shader->arch == 6) ? 0x688 : 0;
      unsigned snap4 = 0x5E;
      uint32_t format = identity | (snap4 << 12) | (regfmt << 24);

      bi_st_cvt(b, data, bi_preload(b, 58), bi_preload(b, 59),
                bi_imm_u32(format), regfmt, nr - 1);
   } else if (b->shader->arch >= 9 && b->shader->idvs != BI_IDVS_NONE) {
      bi_index index = bi_preload(b, 59);

      if (psiz) {
         assert(T_size == 16 && "should've been lowered");
         index = bi_iadd_imm_i32(b, index, 4);
      }

      bi_index address = bi_lea_buf_imm(b, index);
      bi_emit_split_i32(b, a, address, 2);

      bool varying = (b->shader->idvs == BI_IDVS_VARYING);

      bi_store(b, nr * nir_src_bit_size(instr->src[0]), data, a[0], a[1],
               varying ? BI_SEG_VARY : BI_SEG_POS,
               varying ? bi_varying_offset(b->shader, instr) : 0);
   } else if (immediate) {
      bi_index address = bi_lea_attr_imm(b, bi_vertex_id(b), bi_instance_id(b),
                                         regfmt, imm_index);
      bi_emit_split_i32(b, a, address, 3);

      bi_st_cvt(b, data, a[0], a[1], a[2], regfmt, nr - 1);
   } else {
      bi_index idx = bi_iadd_u32(b, bi_src_index(nir_get_io_offset_src(instr)),
                                 bi_imm_u32(nir_intrinsic_base(instr)), false);
      bi_index address =
         bi_lea_attr(b, bi_vertex_id(b), bi_instance_id(b), idx, regfmt);
      bi_emit_split_i32(b, a, address, 3);

      bi_st_cvt(b, data, a[0], a[1], a[2], regfmt, nr - 1);
   }
}

static void
bi_emit_load_ubo(bi_builder *b, nir_intrinsic_instr *instr)
{
   nir_src *offset = nir_get_io_offset_src(instr);

   bool offset_is_const = nir_src_is_const(*offset);
   bi_index dyn_offset = bi_src_index(offset);
   uint32_t const_offset = offset_is_const ? nir_src_as_uint(*offset) : 0;

   bi_load_ubo_to(b, instr->num_components * instr->def.bit_size,
                  bi_def_index(&instr->def),
                  offset_is_const ? bi_imm_u32(const_offset) : dyn_offset,
                  bi_src_index(&instr->src[0]));
}

static void
bi_emit_load_push_constant(bi_builder *b, nir_intrinsic_instr *instr)
{
   assert(b->shader->inputs->no_ubo_to_push && "can't mix push constant forms");

   nir_src *offset = &instr->src[0];
   assert(nir_src_is_const(*offset) && "no indirect push constants");
   uint32_t base = nir_intrinsic_base(instr) + nir_src_as_uint(*offset);
   assert((base & 3) == 0 && "unaligned push constants");

   unsigned bits = instr->def.bit_size * instr->def.num_components;

   unsigned n = DIV_ROUND_UP(bits, 32);
   assert(n <= 4);
   bi_index channels[4] = {bi_null()};

   for (unsigned i = 0; i < n; ++i) {
      unsigned word = (base >> 2) + i;

      channels[i] = bi_fau(BIR_FAU_UNIFORM | (word >> 1), word & 1);
   }

   bi_emit_collect_to(b, bi_def_index(&instr->def), channels, n);
}

static bi_index
bi_addr_high(bi_builder *b, nir_src *src)
{
   return (nir_src_bit_size(*src) == 64) ? bi_extract(b, bi_src_index(src), 1)
                                         : bi_zero();
}

static void
bi_handle_segment(bi_builder *b, bi_index *addr_lo, bi_index *addr_hi,
                  enum bi_seg seg, int16_t *offset)
{
   /* Not needed on Bifrost or for global accesses */
   if (b->shader->arch < 9 || seg == BI_SEG_NONE)
      return;

   /* There is no segment modifier on Valhall. Instead, we need to
    * emit the arithmetic ourselves. We do have an offset
    * available, which saves an instruction for constant offsets.
    */
   bool wls = (seg == BI_SEG_WLS);
   assert(wls || (seg == BI_SEG_TL));

   enum bir_fau fau = wls ? BIR_FAU_WLS_PTR : BIR_FAU_TLS_PTR;

   bi_index base_lo = bi_fau(fau, false);

   if (offset && addr_lo->type == BI_INDEX_CONSTANT &&
       addr_lo->value == (int16_t)addr_lo->value) {
      *offset = addr_lo->value;
      *addr_lo = base_lo;
   } else {
      *addr_lo = bi_iadd_u32(b, base_lo, *addr_lo, false);
   }

   /* Do not allow overflow for WLS or TLS */
   *addr_hi = bi_fau(fau, true);
}

static void
bi_emit_load(bi_builder *b, nir_intrinsic_instr *instr, enum bi_seg seg)
{
   int16_t offset = 0;
   unsigned bits = instr->num_components * instr->def.bit_size;
   bi_index dest = bi_def_index(&instr->def);
   bi_index addr_lo = bi_extract(b, bi_src_index(&instr->src[0]), 0);
   bi_index addr_hi = bi_addr_high(b, &instr->src[0]);

   bi_handle_segment(b, &addr_lo, &addr_hi, seg, &offset);

   bi_load_to(b, bits, dest, addr_lo, addr_hi, seg, offset);
   bi_emit_cached_split(b, dest, bits);
}

static void
bi_emit_store(bi_builder *b, nir_intrinsic_instr *instr, enum bi_seg seg)
{
   /* Require contiguous masks, gauranteed by nir_lower_wrmasks */
   assert(nir_intrinsic_write_mask(instr) ==
          BITFIELD_MASK(instr->num_components));

   int16_t offset = 0;
   bi_index addr_lo = bi_extract(b, bi_src_index(&instr->src[1]), 0);
   bi_index addr_hi = bi_addr_high(b, &instr->src[1]);

   bi_handle_segment(b, &addr_lo, &addr_hi, seg, &offset);

   bi_store(b, instr->num_components * nir_src_bit_size(instr->src[0]),
            bi_src_index(&instr->src[0]), addr_lo, addr_hi, seg, offset);
}

/* Exchanges the staging register with memory */

static void
bi_emit_axchg_to(bi_builder *b, bi_index dst, bi_index addr, nir_src *arg,
                 enum bi_seg seg)
{
   assert(seg == BI_SEG_NONE || seg == BI_SEG_WLS);

   unsigned sz = nir_src_bit_size(*arg);
   assert(sz == 32 || sz == 64);

   bi_index data = bi_src_index(arg);

   bi_index addr_hi = (seg == BI_SEG_WLS) ? bi_zero() : bi_extract(b, addr, 1);

   if (b->shader->arch >= 9)
      bi_handle_segment(b, &addr, &addr_hi, seg, NULL);
   else if (seg == BI_SEG_WLS)
      addr_hi = bi_zero();

   bi_axchg_to(b, sz, dst, data, bi_extract(b, addr, 0), addr_hi, seg);
}

/* Exchanges the second staging register with memory if comparison with first
 * staging register passes */

static void
bi_emit_acmpxchg_to(bi_builder *b, bi_index dst, bi_index addr, nir_src *arg_1,
                    nir_src *arg_2, enum bi_seg seg)
{
   assert(seg == BI_SEG_NONE || seg == BI_SEG_WLS);

   /* hardware is swapped from NIR */
   bi_index src0 = bi_src_index(arg_2);
   bi_index src1 = bi_src_index(arg_1);

   unsigned sz = nir_src_bit_size(*arg_1);
   assert(sz == 32 || sz == 64);

   bi_index data_words[] = {
      bi_extract(b, src0, 0),
      sz == 32 ? bi_extract(b, src1, 0) : bi_extract(b, src0, 1),

      /* 64-bit */
      bi_extract(b, src1, 0),
      sz == 32 ? bi_extract(b, src1, 0) : bi_extract(b, src1, 1),
   };

   bi_index in = bi_temp(b->shader);
   bi_emit_collect_to(b, in, data_words, 2 * (sz / 32));
   bi_index addr_hi = (seg == BI_SEG_WLS) ? bi_zero() : bi_extract(b, addr, 1);

   if (b->shader->arch >= 9)
      bi_handle_segment(b, &addr, &addr_hi, seg, NULL);
   else if (seg == BI_SEG_WLS)
      addr_hi = bi_zero();

   bi_index out = bi_acmpxchg(b, sz, in, bi_extract(b, addr, 0), addr_hi, seg);
   bi_emit_cached_split(b, out, sz);

   bi_index inout_words[] = {bi_extract(b, out, 0),
                             sz == 64 ? bi_extract(b, out, 1) : bi_null()};

   bi_make_vec_to(b, dst, inout_words, NULL, sz / 32, 32);
}

static enum bi_atom_opc
bi_atom_opc_for_nir(nir_atomic_op op)
{
   /* clang-format off */
   switch (op) {
   case nir_atomic_op_iadd: return BI_ATOM_OPC_AADD;
   case nir_atomic_op_imin: return BI_ATOM_OPC_ASMIN;
   case nir_atomic_op_umin: return BI_ATOM_OPC_AUMIN;
   case nir_atomic_op_imax: return BI_ATOM_OPC_ASMAX;
   case nir_atomic_op_umax: return BI_ATOM_OPC_AUMAX;
   case nir_atomic_op_iand: return BI_ATOM_OPC_AAND;
   case nir_atomic_op_ior:  return BI_ATOM_OPC_AOR;
   case nir_atomic_op_ixor: return BI_ATOM_OPC_AXOR;
   default: unreachable("Unexpected computational atomic");
   }
   /* clang-format on */
}

/* Optimized unary atomics are available with an implied #1 argument */

static bool
bi_promote_atom_c1(enum bi_atom_opc op, bi_index arg, enum bi_atom_opc *out)
{
   /* Check we have a compatible constant */
   if (arg.type != BI_INDEX_CONSTANT)
      return false;

   if (!(arg.value == 1 || (arg.value == -1 && op == BI_ATOM_OPC_AADD)))
      return false;

   /* Check for a compatible operation */
   switch (op) {
   case BI_ATOM_OPC_AADD:
      *out = (arg.value == 1) ? BI_ATOM_OPC_AINC : BI_ATOM_OPC_ADEC;
      return true;
   case BI_ATOM_OPC_ASMAX:
      *out = BI_ATOM_OPC_ASMAX1;
      return true;
   case BI_ATOM_OPC_AUMAX:
      *out = BI_ATOM_OPC_AUMAX1;
      return true;
   case BI_ATOM_OPC_AOR:
      *out = BI_ATOM_OPC_AOR1;
      return true;
   default:
      return false;
   }
}

/*
 * Coordinates are 16-bit integers in Bifrost but 32-bit in NIR. We need to
 * translate between these forms (with MKVEC.v2i16).
 *
 * Aditionally on Valhall, cube maps in the attribute pipe are treated as 2D
 * arrays.  For uniform handling, we also treat 3D textures like 2D arrays.
 *
 * Our indexing needs to reflects this.
 */
static bi_index
bi_emit_image_coord(bi_builder *b, bi_index coord, unsigned src_idx,
                    unsigned coord_comps, bool is_array)
{
   assert(coord_comps > 0 && coord_comps <= 3);

   if (src_idx == 0) {
      if (coord_comps == 1 || (coord_comps == 2 && is_array))
         return bi_extract(b, coord, 0);
      else
         return bi_mkvec_v2i16(b, bi_half(bi_extract(b, coord, 0), false),
                               bi_half(bi_extract(b, coord, 1), false));
   } else {
      if (coord_comps == 3 && b->shader->arch >= 9)
         return bi_mkvec_v2i16(b, bi_imm_u16(0),
                               bi_half(bi_extract(b, coord, 2), false));
      else if (coord_comps == 2 && is_array && b->shader->arch >= 9)
         return bi_mkvec_v2i16(b, bi_imm_u16(0),
                               bi_half(bi_extract(b, coord, 1), false));
      else if (coord_comps == 3)
         return bi_extract(b, coord, 2);
      else if (coord_comps == 2 && is_array)
         return bi_extract(b, coord, 1);
      else
         return bi_zero();
   }
}

static bi_index
bi_emit_image_index(bi_builder *b, nir_intrinsic_instr *instr)
{
   nir_src src = instr->src[0];
   bi_index index = bi_src_index(&src);
   bi_context *ctx = b->shader;

   /* Images come after vertex attributes, so handle an explicit offset */
   unsigned offset = (ctx->stage == MESA_SHADER_VERTEX)
                        ? util_bitcount64(ctx->nir->info.inputs_read)
                        : 0;

   if (offset == 0)
      return index;
   else if (nir_src_is_const(src))
      return bi_imm_u32(nir_src_as_uint(src) + offset);
   else
      return bi_iadd_u32(b, index, bi_imm_u32(offset), false);
}

static void
bi_emit_image_load(bi_builder *b, nir_intrinsic_instr *instr)
{
   enum glsl_sampler_dim dim = nir_intrinsic_image_dim(instr);
   unsigned coord_comps = nir_image_intrinsic_coord_components(instr);
   bool array = nir_intrinsic_image_array(instr);
   ASSERTED unsigned nr_dim = glsl_get_sampler_dim_coordinate_components(dim);

   bi_index coords = bi_src_index(&instr->src[1]);
   bi_index xy = bi_emit_image_coord(b, coords, 0, coord_comps, array);
   bi_index zw = bi_emit_image_coord(b, coords, 1, coord_comps, array);
   bi_index dest = bi_def_index(&instr->def);
   enum bi_register_format regfmt =
      bi_reg_fmt_for_nir(nir_intrinsic_dest_type(instr));
   enum bi_vecsize vecsize = instr->num_components - 1;

   /* TODO: MSAA */
   assert(nr_dim != GLSL_SAMPLER_DIM_MS && "MSAA'd images not supported");

   if (b->shader->arch >= 9 && nir_src_is_const(instr->src[0])) {
      bi_instr *I = bi_ld_tex_imm_to(b, dest, xy, zw, regfmt, vecsize,
                                     nir_src_as_uint(instr->src[0]));

      I->table = PAN_TABLE_IMAGE;
   } else if (b->shader->arch >= 9) {
      unreachable("Indirect images on Valhall not yet supported");
   } else {
      bi_ld_attr_tex_to(b, dest, xy, zw, bi_emit_image_index(b, instr), regfmt,
                        vecsize);
   }

   bi_split_def(b, &instr->def);
}

static void
bi_emit_lea_image_to(bi_builder *b, bi_index dest, nir_intrinsic_instr *instr)
{
   enum glsl_sampler_dim dim = nir_intrinsic_image_dim(instr);
   bool array = nir_intrinsic_image_array(instr);
   ASSERTED unsigned nr_dim = glsl_get_sampler_dim_coordinate_components(dim);
   unsigned coord_comps = nir_image_intrinsic_coord_components(instr);

   /* TODO: MSAA */
   assert(nr_dim != GLSL_SAMPLER_DIM_MS && "MSAA'd images not supported");

   enum bi_register_format type =
      (instr->intrinsic == nir_intrinsic_image_store)
         ? bi_reg_fmt_for_nir(nir_intrinsic_src_type(instr))
         : BI_REGISTER_FORMAT_AUTO;

   bi_index coords = bi_src_index(&instr->src[1]);
   bi_index xy = bi_emit_image_coord(b, coords, 0, coord_comps, array);
   bi_index zw = bi_emit_image_coord(b, coords, 1, coord_comps, array);

   if (b->shader->arch >= 9 && nir_src_is_const(instr->src[0])) {
      bi_instr *I = bi_lea_tex_imm_to(b, dest, xy, zw, false,
                                      nir_src_as_uint(instr->src[0]));

      I->table = PAN_TABLE_IMAGE;
   } else if (b->shader->arch >= 9) {
      unreachable("Indirect images on Valhall not yet supported");
   } else {
      bi_instr *I = bi_lea_attr_tex_to(b, dest, xy, zw,
                                       bi_emit_image_index(b, instr), type);

      /* LEA_ATTR_TEX defaults to the secondary attribute table, but
       * our ABI has all images in the primary attribute table
       */
      I->table = BI_TABLE_ATTRIBUTE_1;
   }

   bi_emit_cached_split(b, dest, 3 * 32);
}

static bi_index
bi_emit_lea_image(bi_builder *b, nir_intrinsic_instr *instr)
{
   bi_index dest = bi_temp(b->shader);
   bi_emit_lea_image_to(b, dest, instr);
   return dest;
}

static void
bi_emit_image_store(bi_builder *b, nir_intrinsic_instr *instr)
{
   bi_index a[4] = {bi_null()};
   bi_emit_split_i32(b, a, bi_emit_lea_image(b, instr), 3);

   /* Due to SPIR-V limitations, the source type is not fully reliable: it
    * reports uint32 even for write_imagei. This causes an incorrect
    * u32->s32->u32 roundtrip which incurs an unwanted clamping. Use auto32
    * instead, which will match per the OpenCL spec. Of course this does
    * not work for 16-bit stores, but those are not available in OpenCL.
    */
   nir_alu_type T = nir_intrinsic_src_type(instr);
   assert(nir_alu_type_get_type_size(T) == 32);

   bi_st_cvt(b, bi_src_index(&instr->src[3]), a[0], a[1], a[2],
             BI_REGISTER_FORMAT_AUTO, instr->num_components - 1);
}

static void
bi_emit_atomic_i32_to(bi_builder *b, bi_index dst, bi_index addr, bi_index arg,
                      nir_atomic_op op)
{
   enum bi_atom_opc opc = bi_atom_opc_for_nir(op);
   enum bi_atom_opc post_opc = opc;
   bool bifrost = b->shader->arch <= 8;

   /* ATOM_C.i32 takes a vector with {arg, coalesced}, ATOM_C1.i32 doesn't
    * take any vector but can still output in RETURN mode */
   bi_index tmp_dest = bifrost ? bi_temp(b->shader) : dst;
   unsigned sr_count = bifrost ? 2 : 1;

   /* Generate either ATOM or ATOM1 as required */
   if (bi_promote_atom_c1(opc, arg, &opc)) {
      bi_atom1_return_i32_to(b, tmp_dest, bi_extract(b, addr, 0),
                             bi_extract(b, addr, 1), opc, sr_count);
   } else {
      bi_atom_return_i32_to(b, tmp_dest, arg, bi_extract(b, addr, 0),
                            bi_extract(b, addr, 1), opc, sr_count);
   }

   if (bifrost) {
      /* Post-process it */
      bi_emit_cached_split_i32(b, tmp_dest, 2);
      bi_atom_post_i32_to(b, dst, bi_extract(b, tmp_dest, 0),
                          bi_extract(b, tmp_dest, 1), post_opc);
   }
}

static void
bi_emit_load_frag_coord_zw(bi_builder *b, bi_index dst, unsigned channel)
{
   bi_ld_var_special_to(
      b, dst, bi_zero(), BI_REGISTER_FORMAT_F32, BI_SAMPLE_CENTER,
      BI_UPDATE_CLOBBER,
      (channel == 2) ? BI_VARYING_NAME_FRAG_Z : BI_VARYING_NAME_FRAG_W,
      BI_VECSIZE_NONE);
}

static void
bi_emit_ld_tile(bi_builder *b, nir_intrinsic_instr *instr)
{
   bi_index dest = bi_def_index(&instr->def);
   nir_alu_type T = nir_intrinsic_dest_type(instr);
   enum bi_register_format regfmt = bi_reg_fmt_for_nir(T);
   unsigned size = instr->def.bit_size;
   unsigned nr = instr->num_components;

   /* Get the render target */
   nir_io_semantics sem = nir_intrinsic_io_semantics(instr);
   unsigned loc = sem.location;
   assert(loc >= FRAG_RESULT_DATA0);
   unsigned rt = (loc - FRAG_RESULT_DATA0);

   bi_ld_tile_to(b, dest, bi_pixel_indices(b, rt), bi_coverage(b),
                 bi_src_index(&instr->src[0]), regfmt, nr - 1);
   bi_emit_cached_split(b, dest, size * nr);
}

static void
bi_emit_intrinsic(bi_builder *b, nir_intrinsic_instr *instr)
{
   bi_index dst = nir_intrinsic_infos[instr->intrinsic].has_dest
                     ? bi_def_index(&instr->def)
                     : bi_null();
   gl_shader_stage stage = b->shader->stage;

   switch (instr->intrinsic) {
   case nir_intrinsic_load_barycentric_pixel:
   case nir_intrinsic_load_barycentric_centroid:
   case nir_intrinsic_load_barycentric_sample:
   case nir_intrinsic_load_barycentric_at_sample:
   case nir_intrinsic_load_barycentric_at_offset:
      /* handled later via load_vary */
      break;
   case nir_intrinsic_load_interpolated_input:
   case nir_intrinsic_load_input:
      if (b->shader->inputs->is_blend)
         bi_emit_load_blend_input(b, instr);
      else if (stage == MESA_SHADER_FRAGMENT)
         bi_emit_load_vary(b, instr);
      else if (stage == MESA_SHADER_VERTEX)
         bi_emit_load_attr(b, instr);
      else
         unreachable("Unsupported shader stage");
      break;

   case nir_intrinsic_store_output:
      if (stage == MESA_SHADER_FRAGMENT)
         bi_emit_fragment_out(b, instr);
      else if (stage == MESA_SHADER_VERTEX)
         bi_emit_store_vary(b, instr);
      else
         unreachable("Unsupported shader stage");
      break;

   case nir_intrinsic_store_combined_output_pan:
      assert(stage == MESA_SHADER_FRAGMENT);
      bi_emit_fragment_out(b, instr);
      break;

   case nir_intrinsic_load_ubo:
      bi_emit_load_ubo(b, instr);
      break;

   case nir_intrinsic_load_push_constant:
      bi_emit_load_push_constant(b, instr);
      break;

   case nir_intrinsic_load_global:
   case nir_intrinsic_load_global_constant:
      bi_emit_load(b, instr, BI_SEG_NONE);
      break;

   case nir_intrinsic_store_global:
      bi_emit_store(b, instr, BI_SEG_NONE);
      break;

   case nir_intrinsic_load_scratch:
      bi_emit_load(b, instr, BI_SEG_TL);
      break;

   case nir_intrinsic_store_scratch:
      bi_emit_store(b, instr, BI_SEG_TL);
      break;

   case nir_intrinsic_load_shared:
      bi_emit_load(b, instr, BI_SEG_WLS);
      break;

   case nir_intrinsic_store_shared:
      bi_emit_store(b, instr, BI_SEG_WLS);
      break;

   case nir_intrinsic_barrier:
      if (nir_intrinsic_execution_scope(instr) != SCOPE_NONE) {
         assert(b->shader->stage != MESA_SHADER_FRAGMENT);
         assert(nir_intrinsic_execution_scope(instr) > SCOPE_SUBGROUP &&
                "todo: subgroup barriers (different divergence rules)");
         bi_barrier(b);
      }
      /* Blob doesn't seem to do anything for memory barriers, so no need to
       * check nir_intrinsic_memory_scope().
       */
      break;

   case nir_intrinsic_shared_atomic: {
      nir_atomic_op op = nir_intrinsic_atomic_op(instr);

      if (op == nir_atomic_op_xchg) {
         bi_emit_axchg_to(b, dst, bi_src_index(&instr->src[0]), &instr->src[1],
                          BI_SEG_WLS);
      } else {
         assert(nir_src_bit_size(instr->src[1]) == 32);

         bi_index addr = bi_src_index(&instr->src[0]);
         bi_index addr_hi;

         if (b->shader->arch >= 9) {
            bi_handle_segment(b, &addr, &addr_hi, BI_SEG_WLS, NULL);
            addr = bi_collect_v2i32(b, addr, addr_hi);
         } else {
            addr = bi_seg_add_i64(b, addr, bi_zero(), false, BI_SEG_WLS);
            bi_emit_cached_split(b, addr, 64);
         }

         bi_emit_atomic_i32_to(b, dst, addr, bi_src_index(&instr->src[1]), op);
      }

      bi_split_def(b, &instr->def);
      break;
   }

   case nir_intrinsic_global_atomic: {
      nir_atomic_op op = nir_intrinsic_atomic_op(instr);

      if (op == nir_atomic_op_xchg) {
         bi_emit_axchg_to(b, dst, bi_src_index(&instr->src[0]), &instr->src[1],
                          BI_SEG_NONE);
      } else {
         assert(nir_src_bit_size(instr->src[1]) == 32);

         bi_emit_atomic_i32_to(b, dst, bi_src_index(&instr->src[0]),
                               bi_src_index(&instr->src[1]), op);
      }

      bi_split_def(b, &instr->def);
      break;
   }

   case nir_intrinsic_image_texel_address:
      bi_emit_lea_image_to(b, dst, instr);
      break;

   case nir_intrinsic_image_load:
      bi_emit_image_load(b, instr);
      break;

   case nir_intrinsic_image_store:
      bi_emit_image_store(b, instr);
      break;

   case nir_intrinsic_global_atomic_swap:
      bi_emit_acmpxchg_to(b, dst, bi_src_index(&instr->src[0]), &instr->src[1],
                          &instr->src[2], BI_SEG_NONE);
      bi_split_def(b, &instr->def);
      break;

   case nir_intrinsic_shared_atomic_swap:
      bi_emit_acmpxchg_to(b, dst, bi_src_index(&instr->src[0]), &instr->src[1],
                          &instr->src[2], BI_SEG_WLS);
      bi_split_def(b, &instr->def);
      break;

   case nir_intrinsic_load_pixel_coord:
      /* Vectorized load of the preloaded i16vec2 */
      bi_mov_i32_to(b, dst, bi_preload(b, 59));
      break;

   case nir_intrinsic_load_frag_coord_zw:
      bi_emit_load_frag_coord_zw(b, dst, nir_intrinsic_component(instr));
      break;

   case nir_intrinsic_load_converted_output_pan:
      bi_emit_ld_tile(b, instr);
      break;

   case nir_intrinsic_discard_if:
      bi_discard_b32(b, bi_src_index(&instr->src[0]));
      break;

   case nir_intrinsic_discard:
      bi_discard_f32(b, bi_zero(), bi_zero(), BI_CMPF_EQ);
      break;

   case nir_intrinsic_load_sample_positions_pan:
      bi_collect_v2i32_to(b, dst, bi_fau(BIR_FAU_SAMPLE_POS_ARRAY, false),
                          bi_fau(BIR_FAU_SAMPLE_POS_ARRAY, true));
      break;

   case nir_intrinsic_load_sample_mask_in:
      /* r61[0:15] contains the coverage bitmap */
      bi_u16_to_u32_to(b, dst, bi_half(bi_preload(b, 61), false));
      break;

   case nir_intrinsic_load_sample_mask:
      bi_mov_i32_to(b, dst, bi_coverage(b));
      break;

   case nir_intrinsic_load_sample_id:
      bi_load_sample_id_to(b, dst);
      break;

   case nir_intrinsic_load_front_face:
      /* r58 == 0 means primitive is front facing */
      bi_icmp_i32_to(b, dst, bi_preload(b, 58), bi_zero(), BI_CMPF_EQ,
                     BI_RESULT_TYPE_M1);
      break;

   case nir_intrinsic_load_point_coord:
      bi_ld_var_special_to(b, dst, bi_zero(), BI_REGISTER_FORMAT_F32,
                           BI_SAMPLE_CENTER, BI_UPDATE_CLOBBER,
                           BI_VARYING_NAME_POINT, BI_VECSIZE_V2);
      bi_emit_cached_split_i32(b, dst, 2);
      break;

   /* It appears vertex_id is zero-based with Bifrost geometry flows, but
    * not with Valhall's memory-allocation IDVS geometry flow. We only support
    * the new flow on Valhall so this is lowered in NIR.
    */
   case nir_intrinsic_load_vertex_id:
   case nir_intrinsic_load_vertex_id_zero_base:
      assert(b->shader->malloc_idvs ==
             (instr->intrinsic == nir_intrinsic_load_vertex_id));

      bi_mov_i32_to(b, dst, bi_vertex_id(b));
      break;

   case nir_intrinsic_load_instance_id:
      bi_mov_i32_to(b, dst, bi_instance_id(b));
      break;

   case nir_intrinsic_load_subgroup_invocation:
      bi_mov_i32_to(b, dst, bi_fau(BIR_FAU_LANE_ID, false));
      break;

   case nir_intrinsic_load_local_invocation_id:
      bi_collect_v3i32_to(b, dst,
                          bi_u16_to_u32(b, bi_half(bi_preload(b, 55), 0)),
                          bi_u16_to_u32(b, bi_half(bi_preload(b, 55), 1)),
                          bi_u16_to_u32(b, bi_half(bi_preload(b, 56), 0)));
      break;

   case nir_intrinsic_load_workgroup_id:
      bi_collect_v3i32_to(b, dst, bi_preload(b, 57), bi_preload(b, 58),
                          bi_preload(b, 59));
      break;

   case nir_intrinsic_load_global_invocation_id:
   case nir_intrinsic_load_global_invocation_id_zero_base:
      bi_collect_v3i32_to(b, dst, bi_preload(b, 60), bi_preload(b, 61),
                          bi_preload(b, 62));
      break;

   case nir_intrinsic_shader_clock:
      bi_ld_gclk_u64_to(b, dst, BI_SOURCE_CYCLE_COUNTER);
      bi_split_def(b, &instr->def);
      break;

   default:
      fprintf(stderr, "Unhandled intrinsic %s\n",
              nir_intrinsic_infos[instr->intrinsic].name);
      assert(0);
   }
}

static void
bi_emit_load_const(bi_builder *b, nir_load_const_instr *instr)
{
   /* Make sure we've been lowered */
   assert(instr->def.num_components <= (32 / instr->def.bit_size));

   /* Accumulate all the channels of the constant, as if we did an
    * implicit SEL over them */
   uint32_t acc = 0;

   for (unsigned i = 0; i < instr->def.num_components; ++i) {
      unsigned v =
         nir_const_value_as_uint(instr->value[i], instr->def.bit_size);
      acc |= (v << (i * instr->def.bit_size));
   }

   bi_mov_i32_to(b, bi_get_index(instr->def.index), bi_imm_u32(acc));
}

static bi_index
bi_alu_src_index(bi_builder *b, nir_alu_src src, unsigned comps)
{
   unsigned bitsize = nir_src_bit_size(src.src);

   /* the bi_index carries the 32-bit (word) offset separate from the
    * subword swizzle, first handle the offset */

   unsigned offset = 0;

   assert(bitsize == 8 || bitsize == 16 || bitsize == 32);
   unsigned subword_shift = (bitsize == 32) ? 0 : (bitsize == 16) ? 1 : 2;

   for (unsigned i = 0; i < comps; ++i) {
      unsigned new_offset = (src.swizzle[i] >> subword_shift);

      if (i > 0)
         assert(offset == new_offset && "wrong vectorization");

      offset = new_offset;
   }

   bi_index idx = bi_extract(b, bi_src_index(&src.src), offset);

   /* Compose the subword swizzle with existing (identity) swizzle */
   assert(idx.swizzle == BI_SWIZZLE_H01);

   /* Bigger vectors should have been lowered */
   assert(comps <= (1 << subword_shift));

   if (bitsize == 16) {
      unsigned c0 = src.swizzle[0] & 1;
      unsigned c1 = (comps > 1) ? src.swizzle[1] & 1 : c0;
      idx.swizzle = BI_SWIZZLE_H00 + c1 + (c0 << 1);
   } else if (bitsize == 8) {
      /* 8-bit vectors not yet supported */
      assert(comps == 1 && "8-bit vectors not supported");
      idx.swizzle = BI_SWIZZLE_B0000 + (src.swizzle[0] & 3);
   }

   return idx;
}

static enum bi_round
bi_nir_round(nir_op op)
{
   switch (op) {
   case nir_op_fround_even:
      return BI_ROUND_NONE;
   case nir_op_ftrunc:
      return BI_ROUND_RTZ;
   case nir_op_fceil:
      return BI_ROUND_RTP;
   case nir_op_ffloor:
      return BI_ROUND_RTN;
   default:
      unreachable("invalid nir round op");
   }
}

/* Convenience for lowered transcendentals */

static bi_index
bi_fmul_f32(bi_builder *b, bi_index s0, bi_index s1)
{
   return bi_fma_f32(b, s0, s1, bi_imm_f32(-0.0f));
}

/* Approximate with FRCP_APPROX.f32 and apply a single iteration of
 * Newton-Raphson to improve precision */

static void
bi_lower_frcp_32(bi_builder *b, bi_index dst, bi_index s0)
{
   bi_index x1 = bi_frcp_approx_f32(b, s0);
   bi_index m = bi_frexpm_f32(b, s0, false, false);
   bi_index e = bi_frexpe_f32(b, bi_neg(s0), false, false);
   bi_index t1 = bi_fma_rscale_f32(b, m, bi_neg(x1), bi_imm_f32(1.0), bi_zero(),
                                   BI_SPECIAL_N);
   bi_fma_rscale_f32_to(b, dst, t1, x1, x1, e, BI_SPECIAL_NONE);
}

static void
bi_lower_frsq_32(bi_builder *b, bi_index dst, bi_index s0)
{
   bi_index x1 = bi_frsq_approx_f32(b, s0);
   bi_index m = bi_frexpm_f32(b, s0, false, true);
   bi_index e = bi_frexpe_f32(b, bi_neg(s0), false, true);
   bi_index t1 = bi_fmul_f32(b, x1, x1);
   bi_index t2 = bi_fma_rscale_f32(b, m, bi_neg(t1), bi_imm_f32(1.0),
                                   bi_imm_u32(-1), BI_SPECIAL_N);
   bi_fma_rscale_f32_to(b, dst, t2, x1, x1, e, BI_SPECIAL_N);
}

/* More complex transcendentals, see
 * https://gitlab.freedesktop.org/panfrost/mali-isa-docs/-/blob/master/Bifrost.adoc
 * for documentation */

static void
bi_lower_fexp2_32(bi_builder *b, bi_index dst, bi_index s0)
{
   bi_index t1 = bi_temp(b->shader);
   bi_instr *t1_instr = bi_fadd_f32_to(b, t1, s0, bi_imm_u32(0x49400000));
   t1_instr->clamp = BI_CLAMP_CLAMP_0_INF;

   bi_index t2 = bi_fadd_f32(b, t1, bi_imm_u32(0xc9400000));

   bi_instr *a2 = bi_fadd_f32_to(b, bi_temp(b->shader), s0, bi_neg(t2));
   a2->clamp = BI_CLAMP_CLAMP_M1_1;

   bi_index a1t = bi_fexp_table_u4(b, t1, BI_ADJ_NONE);
   bi_index t3 = bi_isub_u32(b, t1, bi_imm_u32(0x49400000), false);
   bi_index a1i = bi_arshift_i32(b, t3, bi_null(), bi_imm_u8(4));
   bi_index p1 = bi_fma_f32(b, a2->dest[0], bi_imm_u32(0x3d635635),
                            bi_imm_u32(0x3e75fffa));
   bi_index p2 = bi_fma_f32(b, p1, a2->dest[0], bi_imm_u32(0x3f317218));
   bi_index p3 = bi_fmul_f32(b, a2->dest[0], p2);
   bi_instr *x = bi_fma_rscale_f32_to(b, bi_temp(b->shader), p3, a1t, a1t, a1i,
                                      BI_SPECIAL_NONE);
   x->clamp = BI_CLAMP_CLAMP_0_INF;

   bi_instr *max = bi_fmax_f32_to(b, dst, x->dest[0], s0);
   max->sem = BI_SEM_NAN_PROPAGATE;
}

static void
bi_fexp_32(bi_builder *b, bi_index dst, bi_index s0, bi_index log2_base)
{
   /* Scale by base, Multiply by 2*24 and convert to integer to get a 8:24
    * fixed-point input */
   bi_index scale = bi_fma_rscale_f32(b, s0, log2_base, bi_negzero(),
                                      bi_imm_u32(24), BI_SPECIAL_NONE);
   bi_instr *fixed_pt = bi_f32_to_s32_to(b, bi_temp(b->shader), scale);
   fixed_pt->round = BI_ROUND_NONE; // XXX

   /* Compute the result for the fixed-point input, but pass along
    * the floating-point scale for correct NaN propagation */
   bi_fexp_f32_to(b, dst, fixed_pt->dest[0], scale);
}

static void
bi_lower_flog2_32(bi_builder *b, bi_index dst, bi_index s0)
{
   /* s0 = a1 * 2^e, with a1 in [0.75, 1.5) */
   bi_index a1 = bi_frexpm_f32(b, s0, true, false);
   bi_index ei = bi_frexpe_f32(b, s0, true, false);
   bi_index ef = bi_s32_to_f32(b, ei);

   /* xt estimates -log(r1), a coarse approximation of log(a1) */
   bi_index r1 = bi_flog_table_f32(b, s0, BI_MODE_RED, BI_PRECISION_NONE);
   bi_index xt = bi_flog_table_f32(b, s0, BI_MODE_BASE2, BI_PRECISION_NONE);

   /* log(s0) = log(a1 * 2^e) = e + log(a1) = e + log(a1 * r1) -
    * log(r1), so let x1 = e - log(r1) ~= e + xt and x2 = log(a1 * r1),
    * and then log(s0) = x1 + x2 */
   bi_index x1 = bi_fadd_f32(b, ef, xt);

   /* Since a1 * r1 is close to 1, x2 = log(a1 * r1) may be computed by
    * polynomial approximation around 1. The series is expressed around
    * 1, so set y = (a1 * r1) - 1.0 */
   bi_index y = bi_fma_f32(b, a1, r1, bi_imm_f32(-1.0));

   /* x2 = log_2(1 + y) = log_e(1 + y) * (1/log_e(2)), so approximate
    * log_e(1 + y) by the Taylor series (lower precision than the blob):
    * y - y^2/2 + O(y^3) = y(1 - y/2) + O(y^3) */
   bi_index loge =
      bi_fmul_f32(b, y, bi_fma_f32(b, y, bi_imm_f32(-0.5), bi_imm_f32(1.0)));

   bi_index x2 = bi_fmul_f32(b, loge, bi_imm_f32(1.0 / logf(2.0)));

   /* log(s0) = x1 + x2 */
   bi_fadd_f32_to(b, dst, x1, x2);
}

static void
bi_flog2_32(bi_builder *b, bi_index dst, bi_index s0)
{
   bi_index frexp = bi_frexpe_f32(b, s0, true, false);
   bi_index frexpi = bi_s32_to_f32(b, frexp);
   bi_index add = bi_fadd_lscale_f32(b, bi_imm_f32(-1.0f), s0);
   bi_fma_f32_to(b, dst, bi_flogd_f32(b, s0), add, frexpi);
}

static void
bi_lower_fpow_32(bi_builder *b, bi_index dst, bi_index base, bi_index exp)
{
   bi_index log2_base = bi_null();

   if (base.type == BI_INDEX_CONSTANT) {
      log2_base = bi_imm_f32(log2f(uif(base.value)));
   } else {
      log2_base = bi_temp(b->shader);
      bi_lower_flog2_32(b, log2_base, base);
   }

   return bi_lower_fexp2_32(b, dst, bi_fmul_f32(b, exp, log2_base));
}

static void
bi_fpow_32(bi_builder *b, bi_index dst, bi_index base, bi_index exp)
{
   bi_index log2_base = bi_null();

   if (base.type == BI_INDEX_CONSTANT) {
      log2_base = bi_imm_f32(log2f(uif(base.value)));
   } else {
      log2_base = bi_temp(b->shader);
      bi_flog2_32(b, log2_base, base);
   }

   return bi_fexp_32(b, dst, exp, log2_base);
}

/* Bifrost has extremely coarse tables for approximating sin/cos, accessible as
 * FSIN/COS_TABLE.u6, which multiplies the bottom 6-bits by pi/32 and
 * calculates the results. We use them to calculate sin/cos via a Taylor
 * approximation:
 *
 * f(x + e) = f(x) + e f'(x) + (e^2)/2 f''(x)
 * sin(x + e) = sin(x) + e cos(x) - (e^2)/2 sin(x)
 * cos(x + e) = cos(x) - e sin(x) - (e^2)/2 cos(x)
 */

#define TWO_OVER_PI  bi_imm_f32(2.0f / 3.14159f)
#define MPI_OVER_TWO bi_imm_f32(-3.14159f / 2.0)
#define SINCOS_BIAS  bi_imm_u32(0x49400000)

static void
bi_lower_fsincos_32(bi_builder *b, bi_index dst, bi_index s0, bool cos)
{
   /* bottom 6-bits of result times pi/32 approximately s0 mod 2pi */
   bi_index x_u6 = bi_fma_f32(b, s0, TWO_OVER_PI, SINCOS_BIAS);

   /* Approximate domain error (small) */
   bi_index e = bi_fma_f32(b, bi_fadd_f32(b, x_u6, bi_neg(SINCOS_BIAS)),
                           MPI_OVER_TWO, s0);

   /* Lookup sin(x), cos(x) */
   bi_index sinx = bi_fsin_table_u6(b, x_u6, false);
   bi_index cosx = bi_fcos_table_u6(b, x_u6, false);

   /* e^2 / 2 */
   bi_index e2_over_2 =
      bi_fma_rscale_f32(b, e, e, bi_negzero(), bi_imm_u32(-1), BI_SPECIAL_NONE);

   /* (-e^2)/2 f''(x) */
   bi_index quadratic =
      bi_fma_f32(b, bi_neg(e2_over_2), cos ? cosx : sinx, bi_negzero());

   /* e f'(x) - (e^2/2) f''(x) */
   bi_instr *I = bi_fma_f32_to(b, bi_temp(b->shader), e,
                               cos ? bi_neg(sinx) : cosx, quadratic);
   I->clamp = BI_CLAMP_CLAMP_M1_1;

   /* f(x) + e f'(x) - (e^2/2) f''(x) */
   bi_fadd_f32_to(b, dst, I->dest[0], cos ? cosx : sinx);
}

/*
 * The XOR lane op is useful for derivative calculations, but not all Bifrost
 * implementations have it. Add a safe helper that uses the hardware
 * functionality when available and lowers where unavailable.
 */
static bi_index
bi_clper_xor(bi_builder *b, bi_index s0, bi_index s1)
{
   if (!(b->shader->quirks & BIFROST_LIMITED_CLPER)) {
      return bi_clper_i32(b, s0, s1, BI_INACTIVE_RESULT_ZERO, BI_LANE_OP_XOR,
                          BI_SUBGROUP_SUBGROUP4);
   }

   bi_index lane_id = bi_fau(BIR_FAU_LANE_ID, false);
   bi_index lane = bi_lshift_xor_i32(b, lane_id, s1, bi_imm_u8(0));
   return bi_clper_old_i32(b, s0, lane);
}

static enum bi_cmpf
bi_translate_cmpf(nir_op op)
{
   switch (op) {
   case nir_op_ieq8:
   case nir_op_ieq16:
   case nir_op_ieq32:
   case nir_op_feq16:
   case nir_op_feq32:
      return BI_CMPF_EQ;

   case nir_op_ine8:
   case nir_op_ine16:
   case nir_op_ine32:
   case nir_op_fneu16:
   case nir_op_fneu32:
      return BI_CMPF_NE;

   case nir_op_ilt8:
   case nir_op_ilt16:
   case nir_op_ilt32:
   case nir_op_flt16:
   case nir_op_flt32:
   case nir_op_ult8:
   case nir_op_ult16:
   case nir_op_ult32:
      return BI_CMPF_LT;

   case nir_op_ige8:
   case nir_op_ige16:
   case nir_op_ige32:
   case nir_op_fge16:
   case nir_op_fge32:
   case nir_op_uge8:
   case nir_op_uge16:
   case nir_op_uge32:
      return BI_CMPF_GE;

   default:
      unreachable("invalid comparison");
   }
}

static bool
bi_nir_is_replicated(nir_alu_src *src)
{
   for (unsigned i = 1; i < nir_src_num_components(src->src); ++i) {
      if (src->swizzle[0] == src->swizzle[i])
         return false;
   }

   return true;
}

static void
bi_emit_alu(bi_builder *b, nir_alu_instr *instr)
{
   bi_index dst = bi_def_index(&instr->def);
   unsigned srcs = nir_op_infos[instr->op].num_inputs;
   unsigned sz = instr->def.bit_size;
   unsigned comps = instr->def.num_components;
   unsigned src_sz = srcs > 0 ? nir_src_bit_size(instr->src[0].src) : 0;

   /* Indicate scalarness */
   if (sz == 16 && comps == 1)
      dst.swizzle = BI_SWIZZLE_H00;

   /* First, match against the various moves in NIR. These are
    * special-cased because they can operate on vectors even after
    * lowering ALU to scalar. For Bifrost, bi_alu_src_index assumes the
    * instruction is no "bigger" than SIMD-within-a-register. These moves
    * are the exceptions that need to handle swizzles specially. */

   switch (instr->op) {
   case nir_op_vec2:
   case nir_op_vec3:
   case nir_op_vec4:
   case nir_op_vec8:
   case nir_op_vec16: {
      bi_index unoffset_srcs[16] = {bi_null()};
      unsigned channels[16] = {0};

      for (unsigned i = 0; i < srcs; ++i) {
         unoffset_srcs[i] = bi_src_index(&instr->src[i].src);
         channels[i] = instr->src[i].swizzle[0];
      }

      bi_make_vec_to(b, dst, unoffset_srcs, channels, srcs, sz);
      return;
   }

   case nir_op_unpack_32_2x16: {
      /* Should have been scalarized */
      assert(comps == 2 && sz == 16);

      bi_index vec = bi_src_index(&instr->src[0].src);
      unsigned chan = instr->src[0].swizzle[0];

      bi_mov_i32_to(b, dst, bi_extract(b, vec, chan));
      return;
   }

   case nir_op_unpack_64_2x32_split_x: {
      unsigned chan = (instr->src[0].swizzle[0] * 2) + 0;
      bi_mov_i32_to(b, dst,
                    bi_extract(b, bi_src_index(&instr->src[0].src), chan));
      return;
   }

   case nir_op_unpack_64_2x32_split_y: {
      unsigned chan = (instr->src[0].swizzle[0] * 2) + 1;
      bi_mov_i32_to(b, dst,
                    bi_extract(b, bi_src_index(&instr->src[0].src), chan));
      return;
   }

   case nir_op_pack_64_2x32_split:
      bi_collect_v2i32_to(b, dst,
                          bi_extract(b, bi_src_index(&instr->src[0].src),
                                     instr->src[0].swizzle[0]),
                          bi_extract(b, bi_src_index(&instr->src[1].src),
                                     instr->src[1].swizzle[0]));
      return;

   case nir_op_pack_64_2x32:
      bi_collect_v2i32_to(b, dst,
                          bi_extract(b, bi_src_index(&instr->src[0].src),
                                     instr->src[0].swizzle[0]),
                          bi_extract(b, bi_src_index(&instr->src[0].src),
                                     instr->src[0].swizzle[1]));
      return;

   case nir_op_pack_uvec2_to_uint: {
      bi_index src = bi_src_index(&instr->src[0].src);

      assert(sz == 32 && src_sz == 32);
      bi_mkvec_v2i16_to(
         b, dst, bi_half(bi_extract(b, src, instr->src[0].swizzle[0]), false),
         bi_half(bi_extract(b, src, instr->src[0].swizzle[1]), false));
      return;
   }

   case nir_op_pack_uvec4_to_uint: {
      bi_index src = bi_src_index(&instr->src[0].src);

      assert(sz == 32 && src_sz == 32);
      bi_mkvec_v4i8_to(
         b, dst, bi_byte(bi_extract(b, src, instr->src[0].swizzle[0]), 0),
         bi_byte(bi_extract(b, src, instr->src[0].swizzle[1]), 0),
         bi_byte(bi_extract(b, src, instr->src[0].swizzle[2]), 0),
         bi_byte(bi_extract(b, src, instr->src[0].swizzle[3]), 0));
      return;
   }

   case nir_op_mov: {
      bi_index idx = bi_src_index(&instr->src[0].src);
      bi_index unoffset_srcs[4] = {idx, idx, idx, idx};

      unsigned channels[4] = {
         comps > 0 ? instr->src[0].swizzle[0] : 0,
         comps > 1 ? instr->src[0].swizzle[1] : 0,
         comps > 2 ? instr->src[0].swizzle[2] : 0,
         comps > 3 ? instr->src[0].swizzle[3] : 0,
      };

      bi_make_vec_to(b, dst, unoffset_srcs, channels, comps, src_sz);
      return;
   }

   case nir_op_pack_32_2x16: {
      assert(comps == 1);

      bi_index idx = bi_src_index(&instr->src[0].src);
      bi_index unoffset_srcs[4] = {idx, idx, idx, idx};

      unsigned channels[2] = {instr->src[0].swizzle[0],
                              instr->src[0].swizzle[1]};

      bi_make_vec_to(b, dst, unoffset_srcs, channels, 2, 16);
      return;
   }

   case nir_op_f2f16:
   case nir_op_f2f16_rtz:
   case nir_op_f2f16_rtne: {
      assert(src_sz == 32);
      bi_index idx = bi_src_index(&instr->src[0].src);
      bi_index s0 = bi_extract(b, idx, instr->src[0].swizzle[0]);
      bi_index s1 =
         comps > 1 ? bi_extract(b, idx, instr->src[0].swizzle[1]) : s0;

      bi_instr *I = bi_v2f32_to_v2f16_to(b, dst, s0, s1);

      /* Override rounding if explicitly requested. Otherwise, the
       * default rounding mode is selected by the builder. Depending
       * on the float controls required by the shader, the default
       * mode may not be nearest-even.
       */
      if (instr->op == nir_op_f2f16_rtz)
         I->round = BI_ROUND_RTZ;
      else if (instr->op == nir_op_f2f16_rtne)
         I->round = BI_ROUND_NONE; /* Nearest even */

      return;
   }

   /* Vectorized downcasts */
   case nir_op_u2u16:
   case nir_op_i2i16: {
      if (!(src_sz == 32 && comps == 2))
         break;

      bi_index idx = bi_src_index(&instr->src[0].src);
      bi_index s0 = bi_extract(b, idx, instr->src[0].swizzle[0]);
      bi_index s1 = bi_extract(b, idx, instr->src[0].swizzle[1]);

      bi_mkvec_v2i16_to(b, dst, bi_half(s0, false), bi_half(s1, false));
      return;
   }

   /* While we do not have a direct V2U32_TO_V2F16 instruction, lowering to
    * MKVEC.v2i16 + V2U16_TO_V2F16 is more efficient on Bifrost than
    * scalarizing due to scheduling (equal cost on Valhall). Additionally
    * if the source is replicated the MKVEC.v2i16 can be optimized out.
    */
   case nir_op_u2f16:
   case nir_op_i2f16: {
      if (!(src_sz == 32 && comps == 2))
         break;

      nir_alu_src *src = &instr->src[0];
      bi_index idx = bi_src_index(&src->src);
      bi_index s0 = bi_extract(b, idx, src->swizzle[0]);
      bi_index s1 = bi_extract(b, idx, src->swizzle[1]);

      bi_index t =
         (src->swizzle[0] == src->swizzle[1])
            ? bi_half(s0, false)
            : bi_mkvec_v2i16(b, bi_half(s0, false), bi_half(s1, false));

      if (instr->op == nir_op_u2f16)
         bi_v2u16_to_v2f16_to(b, dst, t);
      else
         bi_v2s16_to_v2f16_to(b, dst, t);

      return;
   }

   case nir_op_i2i8:
   case nir_op_u2u8: {
      /* Acts like an 8-bit swizzle */
      bi_index idx = bi_src_index(&instr->src[0].src);
      unsigned factor = src_sz / 8;
      unsigned chan[4] = {0};

      for (unsigned i = 0; i < comps; ++i)
         chan[i] = instr->src[0].swizzle[i] * factor;

      bi_make_vec_to(b, dst, &idx, chan, comps, 8);
      return;
   }

   case nir_op_b32csel: {
      if (sz != 16)
         break;

      /* We allow vectorizing b32csel(cond, A, B) which can be
       * translated as MUX.v2i16, even though cond is a 32-bit vector.
       *
       * If the source condition vector is replicated, we can use
       * MUX.v2i16 directly, letting each component use the
       * corresponding half of the 32-bit source. NIR uses 0/~0
       * booleans so that's guaranteed to work (that is, 32-bit NIR
       * booleans are 16-bit replicated).
       *
       * If we're not replicated, we use the same trick but must
       * insert a MKVEC.v2i16 first to convert down to 16-bit.
       */
      bi_index idx = bi_src_index(&instr->src[0].src);
      bi_index s0 = bi_extract(b, idx, instr->src[0].swizzle[0]);
      bi_index s1 = bi_alu_src_index(b, instr->src[1], comps);
      bi_index s2 = bi_alu_src_index(b, instr->src[2], comps);

      if (!bi_nir_is_replicated(&instr->src[0])) {
         s0 = bi_mkvec_v2i16(
            b, bi_half(s0, false),
            bi_half(bi_extract(b, idx, instr->src[0].swizzle[1]), false));
      }

      bi_mux_v2i16_to(b, dst, s2, s1, s0, BI_MUX_INT_ZERO);
      return;
   }

   default:
      break;
   }

   bi_index s0 =
      srcs > 0 ? bi_alu_src_index(b, instr->src[0], comps) : bi_null();
   bi_index s1 =
      srcs > 1 ? bi_alu_src_index(b, instr->src[1], comps) : bi_null();
   bi_index s2 =
      srcs > 2 ? bi_alu_src_index(b, instr->src[2], comps) : bi_null();

   switch (instr->op) {
   case nir_op_ffma:
      bi_fma_to(b, sz, dst, s0, s1, s2);
      break;

   case nir_op_fmul:
      bi_fma_to(b, sz, dst, s0, s1, bi_negzero());
      break;

   case nir_op_fadd:
      bi_fadd_to(b, sz, dst, s0, s1);
      break;

   case nir_op_fsat: {
      bi_instr *I = bi_fclamp_to(b, sz, dst, s0);
      I->clamp = BI_CLAMP_CLAMP_0_1;
      break;
   }

   case nir_op_fsat_signed_mali: {
      bi_instr *I = bi_fclamp_to(b, sz, dst, s0);
      I->clamp = BI_CLAMP_CLAMP_M1_1;
      break;
   }

   case nir_op_fclamp_pos_mali: {
      bi_instr *I = bi_fclamp_to(b, sz, dst, s0);
      I->clamp = BI_CLAMP_CLAMP_0_INF;
      break;
   }

   case nir_op_fneg:
      bi_fabsneg_to(b, sz, dst, bi_neg(s0));
      break;

   case nir_op_fabs:
      bi_fabsneg_to(b, sz, dst, bi_abs(s0));
      break;

   case nir_op_fsin:
      bi_lower_fsincos_32(b, dst, s0, false);
      break;

   case nir_op_fcos:
      bi_lower_fsincos_32(b, dst, s0, true);
      break;

   case nir_op_fexp2:
      assert(sz == 32); /* should've been lowered */

      if (b->shader->quirks & BIFROST_NO_FP32_TRANSCENDENTALS)
         bi_lower_fexp2_32(b, dst, s0);
      else
         bi_fexp_32(b, dst, s0, bi_imm_f32(1.0f));

      break;

   case nir_op_flog2:
      assert(sz == 32); /* should've been lowered */

      if (b->shader->quirks & BIFROST_NO_FP32_TRANSCENDENTALS)
         bi_lower_flog2_32(b, dst, s0);
      else
         bi_flog2_32(b, dst, s0);

      break;

   case nir_op_fpow:
      assert(sz == 32); /* should've been lowered */

      if (b->shader->quirks & BIFROST_NO_FP32_TRANSCENDENTALS)
         bi_lower_fpow_32(b, dst, s0, s1);
      else
         bi_fpow_32(b, dst, s0, s1);

      break;

   case nir_op_frexp_exp:
      bi_frexpe_to(b, sz, dst, s0, false, false);
      break;

   case nir_op_frexp_sig:
      bi_frexpm_to(b, sz, dst, s0, false, false);
      break;

   case nir_op_ldexp:
      bi_ldexp_to(b, sz, dst, s0, s1);
      break;

   case nir_op_b8csel:
      bi_mux_v4i8_to(b, dst, s2, s1, s0, BI_MUX_INT_ZERO);
      break;

   case nir_op_b16csel:
      bi_mux_v2i16_to(b, dst, s2, s1, s0, BI_MUX_INT_ZERO);
      break;

   case nir_op_b32csel:
      bi_mux_i32_to(b, dst, s2, s1, s0, BI_MUX_INT_ZERO);
      break;

   case nir_op_extract_u8:
   case nir_op_extract_i8: {
      assert(comps == 1 && "should be scalarized");
      assert((src_sz == 16 || src_sz == 32) && "should be lowered");
      unsigned byte = nir_alu_src_as_uint(instr->src[1]);

      if (s0.swizzle == BI_SWIZZLE_H11) {
         assert(byte < 2);
         byte += 2;
      } else if (s0.swizzle != BI_SWIZZLE_H01) {
         assert(s0.swizzle == BI_SWIZZLE_H00);
      }

      assert(byte < 4);

      s0.swizzle = BI_SWIZZLE_H01;

      if (instr->op == nir_op_extract_i8)
         bi_s8_to_s32_to(b, dst, bi_byte(s0, byte));
      else
         bi_u8_to_u32_to(b, dst, bi_byte(s0, byte));
      break;
   }

   case nir_op_extract_u16:
   case nir_op_extract_i16: {
      assert(comps == 1 && "should be scalarized");
      assert(src_sz == 32 && "should be lowered");
      unsigned half = nir_alu_src_as_uint(instr->src[1]);
      assert(half == 0 || half == 1);

      if (instr->op == nir_op_extract_i16)
         bi_s16_to_s32_to(b, dst, bi_half(s0, half));
      else
         bi_u16_to_u32_to(b, dst, bi_half(s0, half));
      break;
   }

   case nir_op_insert_u16: {
      assert(comps == 1 && "should be scalarized");
      unsigned half = nir_alu_src_as_uint(instr->src[1]);
      assert(half == 0 || half == 1);

      if (half == 0)
         bi_u16_to_u32_to(b, dst, bi_half(s0, 0));
      else
         bi_mkvec_v2i16_to(b, dst, bi_imm_u16(0), bi_half(s0, 0));
      break;
   }

   case nir_op_ishl:
      bi_lshift_or_to(b, sz, dst, s0, bi_zero(), bi_byte(s1, 0));
      break;
   case nir_op_ushr:
      bi_rshift_or_to(b, sz, dst, s0, bi_zero(), bi_byte(s1, 0), false);
      break;

   case nir_op_ishr:
      if (b->shader->arch >= 9)
         bi_rshift_or_to(b, sz, dst, s0, bi_zero(), bi_byte(s1, 0), true);
      else
         bi_arshift_to(b, sz, dst, s0, bi_null(), bi_byte(s1, 0));
      break;

   case nir_op_imin:
   case nir_op_umin:
      bi_csel_to(b, nir_op_infos[instr->op].input_types[0], sz, dst, s0, s1, s0,
                 s1, BI_CMPF_LT);
      break;

   case nir_op_imax:
   case nir_op_umax:
      bi_csel_to(b, nir_op_infos[instr->op].input_types[0], sz, dst, s0, s1, s0,
                 s1, BI_CMPF_GT);
      break;

   case nir_op_fddx_must_abs_mali:
   case nir_op_fddy_must_abs_mali: {
      bi_index bit = bi_imm_u32(instr->op == nir_op_fddx_must_abs_mali ? 1 : 2);
      bi_index adjacent = bi_clper_xor(b, s0, bit);
      bi_fadd_to(b, sz, dst, adjacent, bi_neg(s0));
      break;
   }

   case nir_op_fddx:
   case nir_op_fddy:
   case nir_op_fddx_coarse:
   case nir_op_fddy_coarse:
   case nir_op_fddx_fine:
   case nir_op_fddy_fine: {
      unsigned axis;
      switch (instr->op) {
      case nir_op_fddx:
      case nir_op_fddx_coarse:
      case nir_op_fddx_fine:
         axis = 1;
         break;
      case nir_op_fddy:
      case nir_op_fddy_coarse:
      case nir_op_fddy_fine:
         axis = 2;
         break;
      default:
         unreachable("Invalid derivative op");
      }

      bi_index lane1, lane2;
      switch (instr->op) {
      case nir_op_fddx:
      case nir_op_fddx_fine:
      case nir_op_fddy:
      case nir_op_fddy_fine:
         lane1 = bi_lshift_and_i32(b, bi_fau(BIR_FAU_LANE_ID, false),
                                   bi_imm_u32(0x3 & ~axis), bi_imm_u8(0));

         lane2 = bi_iadd_u32(b, lane1, bi_imm_u32(axis), false);
         break;
      case nir_op_fddx_coarse:
      case nir_op_fddy_coarse:
         lane1 = bi_imm_u32(0);
         lane2 = bi_imm_u32(axis);
         break;
      default:
         unreachable("Invalid derivative op");
      }

      bi_index left, right;

      if (b->shader->quirks & BIFROST_LIMITED_CLPER) {
         left = bi_clper_old_i32(b, s0, lane1);
         right = bi_clper_old_i32(b, s0, lane2);
      } else {
         left = bi_clper_i32(b, s0, lane1, BI_INACTIVE_RESULT_ZERO,
                             BI_LANE_OP_NONE, BI_SUBGROUP_SUBGROUP4);

         right = bi_clper_i32(b, s0, lane2, BI_INACTIVE_RESULT_ZERO,
                              BI_LANE_OP_NONE, BI_SUBGROUP_SUBGROUP4);
      }

      bi_fadd_to(b, sz, dst, right, bi_neg(left));
      break;
   }

   case nir_op_f2f32:
      bi_f16_to_f32_to(b, dst, s0);
      break;

   case nir_op_fquantize2f16: {
      bi_instr *f16 = bi_v2f32_to_v2f16_to(b, bi_temp(b->shader), s0, s0);
      bi_instr *f32 = bi_f16_to_f32_to(b, dst, bi_half(f16->dest[0], false));

      f16->ftz = f32->ftz = true;
      break;
   }

   case nir_op_f2i32:
      if (src_sz == 32)
         bi_f32_to_s32_to(b, dst, s0);
      else
         bi_f16_to_s32_to(b, dst, s0);
      break;

   /* Note 32-bit sources => no vectorization, so 32-bit works */
   case nir_op_f2u16:
      if (src_sz == 32)
         bi_f32_to_u32_to(b, dst, s0);
      else
         bi_v2f16_to_v2u16_to(b, dst, s0);
      break;

   case nir_op_f2i16:
      if (src_sz == 32)
         bi_f32_to_s32_to(b, dst, s0);
      else
         bi_v2f16_to_v2s16_to(b, dst, s0);
      break;

   case nir_op_f2u32:
      if (src_sz == 32)
         bi_f32_to_u32_to(b, dst, s0);
      else
         bi_f16_to_u32_to(b, dst, s0);
      break;

   case nir_op_u2f16:
      if (src_sz == 32)
         bi_v2u16_to_v2f16_to(b, dst, bi_half(s0, false));
      else if (src_sz == 16)
         bi_v2u16_to_v2f16_to(b, dst, s0);
      else if (src_sz == 8)
         bi_v2u8_to_v2f16_to(b, dst, s0);
      break;

   case nir_op_u2f32:
      if (src_sz == 32)
         bi_u32_to_f32_to(b, dst, s0);
      else if (src_sz == 16)
         bi_u16_to_f32_to(b, dst, s0);
      else
         bi_u8_to_f32_to(b, dst, s0);
      break;

   case nir_op_i2f16:
      if (src_sz == 32)
         bi_v2s16_to_v2f16_to(b, dst, bi_half(s0, false));
      else if (src_sz == 16)
         bi_v2s16_to_v2f16_to(b, dst, s0);
      else if (src_sz == 8)
         bi_v2s8_to_v2f16_to(b, dst, s0);
      break;

   case nir_op_i2f32:
      assert(src_sz == 32 || src_sz == 16 || src_sz == 8);

      if (src_sz == 32)
         bi_s32_to_f32_to(b, dst, s0);
      else if (src_sz == 16)
         bi_s16_to_f32_to(b, dst, s0);
      else if (src_sz == 8)
         bi_s8_to_f32_to(b, dst, s0);
      break;

   case nir_op_i2i32:
      assert(src_sz == 32 || src_sz == 16 || src_sz == 8);

      if (src_sz == 32)
         bi_mov_i32_to(b, dst, s0);
      else if (src_sz == 16)
         bi_s16_to_s32_to(b, dst, s0);
      else if (src_sz == 8)
         bi_s8_to_s32_to(b, dst, s0);
      break;

   case nir_op_u2u32:
      assert(src_sz == 32 || src_sz == 16 || src_sz == 8);

      if (src_sz == 32)
         bi_mov_i32_to(b, dst, s0);
      else if (src_sz == 16)
         bi_u16_to_u32_to(b, dst, s0);
      else if (src_sz == 8)
         bi_u8_to_u32_to(b, dst, s0);

      break;

   case nir_op_i2i16:
      assert(src_sz == 8 || src_sz == 32);

      if (src_sz == 8)
         bi_v2s8_to_v2s16_to(b, dst, s0);
      else
         bi_mov_i32_to(b, dst, s0);
      break;

   case nir_op_u2u16:
      assert(src_sz == 8 || src_sz == 32);

      if (src_sz == 8)
         bi_v2u8_to_v2u16_to(b, dst, s0);
      else
         bi_mov_i32_to(b, dst, s0);
      break;

   case nir_op_b2i8:
   case nir_op_b2i16:
   case nir_op_b2i32:
      bi_mux_to(b, sz, dst, bi_imm_u8(0), bi_imm_uintN(1, sz), s0,
                BI_MUX_INT_ZERO);
      break;

   case nir_op_ieq8:
   case nir_op_ine8:
   case nir_op_ilt8:
   case nir_op_ige8:
   case nir_op_ieq16:
   case nir_op_ine16:
   case nir_op_ilt16:
   case nir_op_ige16:
   case nir_op_ieq32:
   case nir_op_ine32:
   case nir_op_ilt32:
   case nir_op_ige32:
      bi_icmp_to(b, nir_type_int, sz, dst, s0, s1, bi_translate_cmpf(instr->op),
                 BI_RESULT_TYPE_M1);
      break;

   case nir_op_ult8:
   case nir_op_uge8:
   case nir_op_ult16:
   case nir_op_uge16:
   case nir_op_ult32:
   case nir_op_uge32:
      bi_icmp_to(b, nir_type_uint, sz, dst, s0, s1,
                 bi_translate_cmpf(instr->op), BI_RESULT_TYPE_M1);
      break;

   case nir_op_feq32:
   case nir_op_feq16:
   case nir_op_flt32:
   case nir_op_flt16:
   case nir_op_fge32:
   case nir_op_fge16:
   case nir_op_fneu32:
   case nir_op_fneu16:
      bi_fcmp_to(b, sz, dst, s0, s1, bi_translate_cmpf(instr->op),
                 BI_RESULT_TYPE_M1);
      break;

   case nir_op_fround_even:
   case nir_op_fceil:
   case nir_op_ffloor:
   case nir_op_ftrunc:
      bi_fround_to(b, sz, dst, s0, bi_nir_round(instr->op));
      break;

   case nir_op_fmin:
      bi_fmin_to(b, sz, dst, s0, s1);
      break;

   case nir_op_fmax:
      bi_fmax_to(b, sz, dst, s0, s1);
      break;

   case nir_op_iadd:
      bi_iadd_to(b, nir_type_int, sz, dst, s0, s1, false);
      break;

   case nir_op_iadd_sat:
      bi_iadd_to(b, nir_type_int, sz, dst, s0, s1, true);
      break;

   case nir_op_uadd_sat:
      bi_iadd_to(b, nir_type_uint, sz, dst, s0, s1, true);
      break;

   case nir_op_ihadd:
      bi_hadd_to(b, nir_type_int, sz, dst, s0, s1, BI_ROUND_RTN);
      break;

   case nir_op_irhadd:
      bi_hadd_to(b, nir_type_int, sz, dst, s0, s1, BI_ROUND_RTP);
      break;

   case nir_op_uhadd:
      bi_hadd_to(b, nir_type_uint, sz, dst, s0, s1, BI_ROUND_RTN);
      break;

   case nir_op_urhadd:
      bi_hadd_to(b, nir_type_uint, sz, dst, s0, s1, BI_ROUND_RTP);
      break;

   case nir_op_ineg:
      bi_isub_to(b, nir_type_int, sz, dst, bi_zero(), s0, false);
      break;

   case nir_op_isub:
      bi_isub_to(b, nir_type_int, sz, dst, s0, s1, false);
      break;

   case nir_op_isub_sat:
      bi_isub_to(b, nir_type_int, sz, dst, s0, s1, true);
      break;

   case nir_op_usub_sat:
      bi_isub_to(b, nir_type_uint, sz, dst, s0, s1, true);
      break;

   case nir_op_imul:
      bi_imul_to(b, sz, dst, s0, s1);
      break;

   case nir_op_iabs:
      bi_iabs_to(b, sz, dst, s0);
      break;

   case nir_op_iand:
      bi_lshift_and_to(b, sz, dst, s0, s1, bi_imm_u8(0));
      break;

   case nir_op_ior:
      bi_lshift_or_to(b, sz, dst, s0, s1, bi_imm_u8(0));
      break;

   case nir_op_ixor:
      bi_lshift_xor_to(b, sz, dst, s0, s1, bi_imm_u8(0));
      break;

   case nir_op_inot:
      bi_lshift_or_to(b, sz, dst, bi_zero(), bi_not(s0), bi_imm_u8(0));
      break;

   case nir_op_frsq:
      if (sz == 32 && b->shader->quirks & BIFROST_NO_FP32_TRANSCENDENTALS)
         bi_lower_frsq_32(b, dst, s0);
      else
         bi_frsq_to(b, sz, dst, s0);
      break;

   case nir_op_frcp:
      if (sz == 32 && b->shader->quirks & BIFROST_NO_FP32_TRANSCENDENTALS)
         bi_lower_frcp_32(b, dst, s0);
      else
         bi_frcp_to(b, sz, dst, s0);
      break;

   case nir_op_uclz:
      bi_clz_to(b, sz, dst, s0, false);
      break;

   case nir_op_bit_count:
      assert(sz == 32 && src_sz == 32 && "should've been lowered");
      bi_popcount_i32_to(b, dst, s0);
      break;

   case nir_op_bitfield_reverse:
      assert(sz == 32 && src_sz == 32 && "should've been lowered");
      bi_bitrev_i32_to(b, dst, s0);
      break;

   case nir_op_ufind_msb: {
      bi_index clz = bi_clz(b, src_sz, s0, false);

      if (sz == 8)
         clz = bi_byte(clz, 0);
      else if (sz == 16)
         clz = bi_half(clz, false);

      bi_isub_u32_to(b, dst, bi_imm_u32(src_sz - 1), clz, false);
      break;
   }

   default:
      fprintf(stderr, "Unhandled ALU op %s\n", nir_op_infos[instr->op].name);
      unreachable("Unknown ALU op");
   }
}

/* Returns dimension with 0 special casing cubemaps. Shamelessly copied from
 * Midgard */
static unsigned
bifrost_tex_format(enum glsl_sampler_dim dim)
{
   switch (dim) {
   case GLSL_SAMPLER_DIM_1D:
   case GLSL_SAMPLER_DIM_BUF:
      return 1;

   case GLSL_SAMPLER_DIM_2D:
   case GLSL_SAMPLER_DIM_MS:
   case GLSL_SAMPLER_DIM_EXTERNAL:
   case GLSL_SAMPLER_DIM_RECT:
      return 2;

   case GLSL_SAMPLER_DIM_3D:
      return 3;

   case GLSL_SAMPLER_DIM_CUBE:
      return 0;

   default:
      DBG("Unknown sampler dim type\n");
      assert(0);
      return 0;
   }
}

static enum bi_dimension
valhall_tex_dimension(enum glsl_sampler_dim dim)
{
   switch (dim) {
   case GLSL_SAMPLER_DIM_1D:
   case GLSL_SAMPLER_DIM_BUF:
      return BI_DIMENSION_1D;

   case GLSL_SAMPLER_DIM_2D:
   case GLSL_SAMPLER_DIM_MS:
   case GLSL_SAMPLER_DIM_EXTERNAL:
   case GLSL_SAMPLER_DIM_RECT:
      return BI_DIMENSION_2D;

   case GLSL_SAMPLER_DIM_3D:
      return BI_DIMENSION_3D;

   case GLSL_SAMPLER_DIM_CUBE:
      return BI_DIMENSION_CUBE;

   default:
      unreachable("Unknown sampler dim type");
   }
}

static enum bifrost_texture_format_full
bi_texture_format(nir_alu_type T, enum bi_clamp clamp)
{
   switch (T) {
   case nir_type_float16:
      return BIFROST_TEXTURE_FORMAT_F16 + clamp;
   case nir_type_float32:
      return BIFROST_TEXTURE_FORMAT_F32 + clamp;
   case nir_type_uint16:
      return BIFROST_TEXTURE_FORMAT_U16;
   case nir_type_int16:
      return BIFROST_TEXTURE_FORMAT_S16;
   case nir_type_uint32:
      return BIFROST_TEXTURE_FORMAT_U32;
   case nir_type_int32:
      return BIFROST_TEXTURE_FORMAT_S32;
   default:
      unreachable("Invalid type for texturing");
   }
}

/* Array indices are specified as 32-bit uints, need to convert. In .z component
 * from NIR */
static bi_index
bi_emit_texc_array_index(bi_builder *b, bi_index idx, nir_alu_type T)
{
   /* For (u)int we can just passthrough */
   nir_alu_type base = nir_alu_type_get_base_type(T);
   if (base == nir_type_int || base == nir_type_uint)
      return idx;

   /* Otherwise we convert */
   assert(T == nir_type_float32);

   /* OpenGL ES 3.2 specification section 8.14.2 ("Coordinate Wrapping and
    * Texel Selection") defines the layer to be taken from clamp(RNE(r),
    * 0, dt - 1). So we use round RTE, clamping is handled at the data
    * structure level */

   bi_instr *I = bi_f32_to_u32_to(b, bi_temp(b->shader), idx);
   I->round = BI_ROUND_NONE;
   return I->dest[0];
}

/* TEXC's explicit and bias LOD modes requires the LOD to be transformed to a
 * 16-bit 8:8 fixed-point format. We lower as:
 *
 * F32_TO_S32(clamp(x, -16.0, +16.0) * 256.0) & 0xFFFF =
 * MKVEC(F32_TO_S32(clamp(x * 1.0/16.0, -1.0, 1.0) * (16.0 * 256.0)), #0)
 */

static bi_index
bi_emit_texc_lod_88(bi_builder *b, bi_index lod, bool fp16)
{
   /* Precompute for constant LODs to avoid general constant folding */
   if (lod.type == BI_INDEX_CONSTANT) {
      uint32_t raw = lod.value;
      float x = fp16 ? _mesa_half_to_float(raw) : uif(raw);
      int32_t s32 = CLAMP(x, -16.0f, 16.0f) * 256.0f;
      return bi_imm_u32(s32 & 0xFFFF);
   }

   /* Sort of arbitrary. Must be less than 128.0, greater than or equal to
    * the max LOD (16 since we cap at 2^16 texture dimensions), and
    * preferably small to minimize precision loss */
   const float max_lod = 16.0;

   bi_instr *fsat =
      bi_fma_f32_to(b, bi_temp(b->shader), fp16 ? bi_half(lod, false) : lod,
                    bi_imm_f32(1.0f / max_lod), bi_negzero());

   fsat->clamp = BI_CLAMP_CLAMP_M1_1;

   bi_index fmul =
      bi_fma_f32(b, fsat->dest[0], bi_imm_f32(max_lod * 256.0f), bi_negzero());

   return bi_mkvec_v2i16(b, bi_half(bi_f32_to_s32(b, fmul), false),
                         bi_imm_u16(0));
}

/* FETCH takes a 32-bit staging register containing the LOD as an integer in
 * the bottom 16-bits and (if present) the cube face index in the top 16-bits.
 * TODO: Cube face.
 */

static bi_index
bi_emit_texc_lod_cube(bi_builder *b, bi_index lod)
{
   return bi_lshift_or_i32(b, lod, bi_zero(), bi_imm_u8(8));
}

/* The hardware specifies texel offsets and multisample indices together as a
 * u8vec4 <offset, ms index>. By default all are zero, so if have either a
 * nonzero texel offset or a nonzero multisample index, we build a u8vec4 with
 * the bits we need and return that to be passed as a staging register. Else we
 * return 0 to avoid allocating a data register when everything is zero. */

static bi_index
bi_emit_texc_offset_ms_index(bi_builder *b, nir_tex_instr *instr)
{
   bi_index dest = bi_zero();

   int offs_idx = nir_tex_instr_src_index(instr, nir_tex_src_offset);
   if (offs_idx >= 0 && (!nir_src_is_const(instr->src[offs_idx].src) ||
                         nir_src_as_uint(instr->src[offs_idx].src) != 0)) {
      unsigned nr = nir_src_num_components(instr->src[offs_idx].src);
      bi_index idx = bi_src_index(&instr->src[offs_idx].src);
      dest = bi_mkvec_v4i8(
         b, (nr > 0) ? bi_byte(bi_extract(b, idx, 0), 0) : bi_imm_u8(0),
         (nr > 1) ? bi_byte(bi_extract(b, idx, 1), 0) : bi_imm_u8(0),
         (nr > 2) ? bi_byte(bi_extract(b, idx, 2), 0) : bi_imm_u8(0),
         bi_imm_u8(0));
   }

   int ms_idx = nir_tex_instr_src_index(instr, nir_tex_src_ms_index);
   if (ms_idx >= 0 && (!nir_src_is_const(instr->src[ms_idx].src) ||
                       nir_src_as_uint(instr->src[ms_idx].src) != 0)) {
      dest = bi_lshift_or_i32(b, bi_src_index(&instr->src[ms_idx].src), dest,
                              bi_imm_u8(24));
   }

   return dest;
}

/*
 * Valhall specifies specifies texel offsets, multisample indices, and (for
 * fetches) LOD together as a u8vec4 <offset.xyz, LOD>, where the third
 * component is either offset.z or multisample index depending on context. Build
 * this register.
 */
static bi_index
bi_emit_valhall_offsets(bi_builder *b, nir_tex_instr *instr)
{
   bi_index dest = bi_zero();

   int offs_idx = nir_tex_instr_src_index(instr, nir_tex_src_offset);
   int ms_idx = nir_tex_instr_src_index(instr, nir_tex_src_ms_index);
   int lod_idx = nir_tex_instr_src_index(instr, nir_tex_src_lod);

   /* Components 0-2: offsets */
   if (offs_idx >= 0 && (!nir_src_is_const(instr->src[offs_idx].src) ||
                         nir_src_as_uint(instr->src[offs_idx].src) != 0)) {
      unsigned nr = nir_src_num_components(instr->src[offs_idx].src);
      bi_index idx = bi_src_index(&instr->src[offs_idx].src);

      /* No multisample index with 3D */
      assert((nr <= 2) || (ms_idx < 0));

      /* Zero extend the Z byte so we can use it with MKVEC.v2i8 */
      bi_index z = (nr > 2)
                      ? bi_mkvec_v2i8(b, bi_byte(bi_extract(b, idx, 2), 0),
                                      bi_imm_u8(0), bi_zero())
                      : bi_zero();

      dest = bi_mkvec_v2i8(
         b, (nr > 0) ? bi_byte(bi_extract(b, idx, 0), 0) : bi_imm_u8(0),
         (nr > 1) ? bi_byte(bi_extract(b, idx, 1), 0) : bi_imm_u8(0), z);
   }

   /* Component 2: multisample index */
   if (ms_idx >= 0 && (!nir_src_is_const(instr->src[ms_idx].src) ||
                       nir_src_as_uint(instr->src[ms_idx].src) != 0)) {
      dest = bi_mkvec_v2i16(b, dest, bi_src_index(&instr->src[ms_idx].src));
   }

   /* Component 3: 8-bit LOD */
   if (lod_idx >= 0 &&
       (!nir_src_is_const(instr->src[lod_idx].src) ||
        nir_src_as_uint(instr->src[lod_idx].src) != 0) &&
       nir_tex_instr_src_type(instr, lod_idx) != nir_type_float) {
      dest = bi_lshift_or_i32(b, bi_src_index(&instr->src[lod_idx].src), dest,
                              bi_imm_u8(24));
   }

   return dest;
}

static void
bi_emit_cube_coord(bi_builder *b, bi_index coord, bi_index *face, bi_index *s,
                   bi_index *t)
{
   /* Compute max { |x|, |y|, |z| } */
   bi_index maxxyz = bi_temp(b->shader);
   *face = bi_temp(b->shader);

   bi_index cx = bi_extract(b, coord, 0), cy = bi_extract(b, coord, 1),
            cz = bi_extract(b, coord, 2);

   /* Use a pseudo op on Bifrost due to tuple restrictions */
   if (b->shader->arch <= 8) {
      bi_cubeface_to(b, maxxyz, *face, cx, cy, cz);
   } else {
      bi_cubeface1_to(b, maxxyz, cx, cy, cz);
      bi_cubeface2_v9_to(b, *face, cx, cy, cz);
   }

   /* Select coordinates */
   bi_index ssel =
      bi_cube_ssel(b, bi_extract(b, coord, 2), bi_extract(b, coord, 0), *face);
   bi_index tsel =
      bi_cube_tsel(b, bi_extract(b, coord, 1), bi_extract(b, coord, 2), *face);

   /* The OpenGL ES specification requires us to transform an input vector
    * (x, y, z) to the coordinate, given the selected S/T:
    *
    * (1/2 ((s / max{x,y,z}) + 1), 1/2 ((t / max{x, y, z}) + 1))
    *
    * We implement (s shown, t similar) in a form friendlier to FMA
    * instructions, and clamp coordinates at the end for correct
    * NaN/infinity handling:
    *
    * fsat(s * (0.5 * (1 / max{x, y, z})) + 0.5)
    *
    * Take the reciprocal of max{x, y, z}
    */
   bi_index rcp = bi_frcp_f32(b, maxxyz);

   /* Calculate 0.5 * (1.0 / max{x, y, z}) */
   bi_index fma1 = bi_fma_f32(b, rcp, bi_imm_f32(0.5f), bi_negzero());

   /* Transform the coordinates */
   *s = bi_temp(b->shader);
   *t = bi_temp(b->shader);

   bi_instr *S = bi_fma_f32_to(b, *s, fma1, ssel, bi_imm_f32(0.5f));
   bi_instr *T = bi_fma_f32_to(b, *t, fma1, tsel, bi_imm_f32(0.5f));

   S->clamp = BI_CLAMP_CLAMP_0_1;
   T->clamp = BI_CLAMP_CLAMP_0_1;
}

/* Emits a cube map descriptor, returning lower 32-bits and putting upper
 * 32-bits in passed pointer t. The packing of the face with the S coordinate
 * exploits the redundancy of floating points with the range restriction of
 * CUBEFACE output.
 *
 *     struct cube_map_descriptor {
 *         float s : 29;
 *         unsigned face : 3;
 *         float t : 32;
 *     }
 *
 * Since the cube face index is preshifted, this is easy to pack with a bitwise
 * MUX.i32 and a fixed mask, selecting the lower bits 29 from s and the upper 3
 * bits from face.
 */

static bi_index
bi_emit_texc_cube_coord(bi_builder *b, bi_index coord, bi_index *t)
{
   bi_index face, s;
   bi_emit_cube_coord(b, coord, &face, &s, t);
   bi_index mask = bi_imm_u32(BITFIELD_MASK(29));
   return bi_mux_i32(b, s, face, mask, BI_MUX_BIT);
}

/* Map to the main texture op used. Some of these (txd in particular) will
 * lower to multiple texture ops with different opcodes (GRDESC_DER + TEX in
 * sequence). We assume that lowering is handled elsewhere.
 */

static enum bifrost_tex_op
bi_tex_op(nir_texop op)
{
   switch (op) {
   case nir_texop_tex:
   case nir_texop_txb:
   case nir_texop_txl:
   case nir_texop_txd:
      return BIFROST_TEX_OP_TEX;
   case nir_texop_txf:
   case nir_texop_txf_ms:
   case nir_texop_tg4:
      return BIFROST_TEX_OP_FETCH;
   case nir_texop_txs:
   case nir_texop_lod:
   case nir_texop_query_levels:
   case nir_texop_texture_samples:
   case nir_texop_samples_identical:
      unreachable("should've been lowered");
   default:
      unreachable("unsupported tex op");
   }
}

/* Data registers required by texturing in the order they appear. All are
 * optional, the texture operation descriptor determines which are present.
 * Note since 3D arrays are not permitted at an API level, Z_COORD and
 * ARRAY/SHADOW are exlusive, so TEXC in practice reads at most 8 registers */

enum bifrost_tex_dreg {
   BIFROST_TEX_DREG_Z_COORD = 0,
   BIFROST_TEX_DREG_Y_DELTAS = 1,
   BIFROST_TEX_DREG_LOD = 2,
   BIFROST_TEX_DREG_GRDESC_HI = 3,
   BIFROST_TEX_DREG_SHADOW = 4,
   BIFROST_TEX_DREG_ARRAY = 5,
   BIFROST_TEX_DREG_OFFSETMS = 6,
   BIFROST_TEX_DREG_SAMPLER = 7,
   BIFROST_TEX_DREG_TEXTURE = 8,
   BIFROST_TEX_DREG_COUNT,
};

static void
bi_emit_texc(bi_builder *b, nir_tex_instr *instr)
{
   struct bifrost_texture_operation desc = {
      .op = bi_tex_op(instr->op),
      .offset_or_bias_disable = false, /* TODO */
      .shadow_or_clamp_disable = instr->is_shadow,
      .array = instr->is_array,
      .dimension = bifrost_tex_format(instr->sampler_dim),
      .format = bi_texture_format(instr->dest_type | instr->def.bit_size,
                                  BI_CLAMP_NONE), /* TODO */
      .mask = 0xF,
   };

   switch (desc.op) {
   case BIFROST_TEX_OP_TEX:
      desc.lod_or_fetch = BIFROST_LOD_MODE_COMPUTE;
      break;
   case BIFROST_TEX_OP_FETCH:
      desc.lod_or_fetch = (enum bifrost_lod_mode)(
         instr->op == nir_texop_tg4
            ? BIFROST_TEXTURE_FETCH_GATHER4_R + instr->component
            : BIFROST_TEXTURE_FETCH_TEXEL);
      break;
   default:
      unreachable("texture op unsupported");
   }

   /* 32-bit indices to be allocated as consecutive staging registers */
   bi_index dregs[BIFROST_TEX_DREG_COUNT] = {};
   bi_index cx = bi_null(), cy = bi_null();

   for (unsigned i = 0; i < instr->num_srcs; ++i) {
      bi_index index = bi_src_index(&instr->src[i].src);
      unsigned sz = nir_src_bit_size(instr->src[i].src);
      unsigned components = nir_src_num_components(instr->src[i].src);
      ASSERTED nir_alu_type base = nir_tex_instr_src_type(instr, i);
      nir_alu_type T = base | sz;

      switch (instr->src[i].src_type) {
      case nir_tex_src_coord:
         if (instr->sampler_dim == GLSL_SAMPLER_DIM_CUBE) {
            cx = bi_emit_texc_cube_coord(b, index, &cy);
         } else {
            /* Copy XY (for 2D+) or XX (for 1D) */
            cx = bi_extract(b, index, 0);
            cy = bi_extract(b, index, MIN2(1, components - 1));

            assert(components >= 1 && components <= 3);

            if (components == 3 && !desc.array) {
               /* 3D */
               dregs[BIFROST_TEX_DREG_Z_COORD] = bi_extract(b, index, 2);
            }
         }

         if (desc.array) {
            dregs[BIFROST_TEX_DREG_ARRAY] = bi_emit_texc_array_index(
               b, bi_extract(b, index, components - 1), T);
         }

         break;

      case nir_tex_src_lod:
         if (desc.op == BIFROST_TEX_OP_TEX &&
             nir_src_is_const(instr->src[i].src) &&
             nir_src_as_uint(instr->src[i].src) == 0) {
            desc.lod_or_fetch = BIFROST_LOD_MODE_ZERO;
         } else if (desc.op == BIFROST_TEX_OP_TEX) {
            assert(base == nir_type_float);

            assert(sz == 16 || sz == 32);
            dregs[BIFROST_TEX_DREG_LOD] =
               bi_emit_texc_lod_88(b, index, sz == 16);
            desc.lod_or_fetch = BIFROST_LOD_MODE_EXPLICIT;
         } else {
            assert(desc.op == BIFROST_TEX_OP_FETCH);
            assert(base == nir_type_uint || base == nir_type_int);
            assert(sz == 16 || sz == 32);

            dregs[BIFROST_TEX_DREG_LOD] = bi_emit_texc_lod_cube(b, index);
         }

         break;

      case nir_tex_src_bias:
         /* Upper 16-bits interpreted as a clamp, leave zero */
         assert(desc.op == BIFROST_TEX_OP_TEX);
         assert(base == nir_type_float);
         assert(sz == 16 || sz == 32);
         dregs[BIFROST_TEX_DREG_LOD] = bi_emit_texc_lod_88(b, index, sz == 16);
         desc.lod_or_fetch = BIFROST_LOD_MODE_BIAS;
         break;

      case nir_tex_src_ms_index:
      case nir_tex_src_offset:
         if (desc.offset_or_bias_disable)
            break;

         dregs[BIFROST_TEX_DREG_OFFSETMS] =
            bi_emit_texc_offset_ms_index(b, instr);
         if (!bi_is_equiv(dregs[BIFROST_TEX_DREG_OFFSETMS], bi_zero()))
            desc.offset_or_bias_disable = true;
         break;

      case nir_tex_src_comparator:
         dregs[BIFROST_TEX_DREG_SHADOW] = index;
         break;

      case nir_tex_src_texture_offset:
         dregs[BIFROST_TEX_DREG_TEXTURE] = index;
         break;

      case nir_tex_src_sampler_offset:
         dregs[BIFROST_TEX_DREG_SAMPLER] = index;
         break;

      default:
         unreachable("Unhandled src type in texc emit");
      }
   }

   if (desc.op == BIFROST_TEX_OP_FETCH &&
       bi_is_null(dregs[BIFROST_TEX_DREG_LOD])) {
      dregs[BIFROST_TEX_DREG_LOD] = bi_emit_texc_lod_cube(b, bi_zero());
   }

   /* Choose an index mode */

   bool direct_tex = bi_is_null(dregs[BIFROST_TEX_DREG_TEXTURE]);
   bool direct_samp = bi_is_null(dregs[BIFROST_TEX_DREG_SAMPLER]);
   bool direct = direct_tex && direct_samp;

   desc.immediate_indices = direct && (instr->sampler_index < 16);

   if (desc.immediate_indices) {
      desc.sampler_index_or_mode = instr->sampler_index;
      desc.index = instr->texture_index;
   } else {
      unsigned mode = 0;

      if (direct && instr->sampler_index == instr->texture_index) {
         mode = BIFROST_INDEX_IMMEDIATE_SHARED;
         desc.index = instr->texture_index;
      } else if (direct) {
         mode = BIFROST_INDEX_IMMEDIATE_SAMPLER;
         desc.index = instr->sampler_index;
         dregs[BIFROST_TEX_DREG_TEXTURE] =
            bi_mov_i32(b, bi_imm_u32(instr->texture_index));
      } else if (direct_tex) {
         assert(!direct_samp);
         mode = BIFROST_INDEX_IMMEDIATE_TEXTURE;
         desc.index = instr->texture_index;
      } else if (direct_samp) {
         assert(!direct_tex);
         mode = BIFROST_INDEX_IMMEDIATE_SAMPLER;
         desc.index = instr->sampler_index;
      } else {
         mode = BIFROST_INDEX_REGISTER;
      }

      mode |= (BIFROST_TEXTURE_OPERATION_SINGLE << 2);
      desc.sampler_index_or_mode = mode;
   }

   /* Allocate staging registers contiguously by compacting the array. */
   unsigned sr_count = 0;

   for (unsigned i = 0; i < ARRAY_SIZE(dregs); ++i) {
      if (!bi_is_null(dregs[i]))
         dregs[sr_count++] = dregs[i];
   }

   unsigned res_size = instr->def.bit_size == 16 ? 2 : 4;

   bi_index sr = sr_count ? bi_temp(b->shader) : bi_null();
   bi_index dst = bi_temp(b->shader);

   if (sr_count)
      bi_emit_collect_to(b, sr, dregs, sr_count);

   uint32_t desc_u = 0;
   memcpy(&desc_u, &desc, sizeof(desc_u));
   bi_instr *I =
      bi_texc_to(b, dst, sr, cx, cy, bi_imm_u32(desc_u),
                 !nir_tex_instr_has_implicit_derivative(instr), sr_count, 0);
   I->register_format = bi_reg_fmt_for_nir(instr->dest_type);

   bi_index w[4] = {bi_null(), bi_null(), bi_null(), bi_null()};
   bi_emit_split_i32(b, w, dst, res_size);
   bi_emit_collect_to(b, bi_def_index(&instr->def), w,
                      DIV_ROUND_UP(instr->def.num_components * res_size, 4));
}

/* Staging registers required by texturing in the order they appear (Valhall) */

enum valhall_tex_sreg {
   VALHALL_TEX_SREG_X_COORD = 0,
   VALHALL_TEX_SREG_Y_COORD = 1,
   VALHALL_TEX_SREG_Z_COORD = 2,
   VALHALL_TEX_SREG_Y_DELTAS = 3,
   VALHALL_TEX_SREG_ARRAY = 4,
   VALHALL_TEX_SREG_SHADOW = 5,
   VALHALL_TEX_SREG_OFFSETMS = 6,
   VALHALL_TEX_SREG_LOD = 7,
   VALHALL_TEX_SREG_GRDESC = 8,
   VALHALL_TEX_SREG_COUNT,
};

static void
bi_emit_tex_valhall(bi_builder *b, nir_tex_instr *instr)
{
   bool explicit_offset = false;
   enum bi_va_lod_mode lod_mode = BI_VA_LOD_MODE_COMPUTED_LOD;

   bool has_lod_mode = (instr->op == nir_texop_tex) ||
                       (instr->op == nir_texop_txl) ||
                       (instr->op == nir_texop_txb);

   /* 32-bit indices to be allocated as consecutive staging registers */
   bi_index sregs[VALHALL_TEX_SREG_COUNT] = {};
   bi_index sampler = bi_imm_u32(instr->sampler_index);
   bi_index texture = bi_imm_u32(instr->texture_index);
   uint32_t tables = (PAN_TABLE_SAMPLER << 11) | (PAN_TABLE_TEXTURE << 27);

   for (unsigned i = 0; i < instr->num_srcs; ++i) {
      bi_index index = bi_src_index(&instr->src[i].src);
      unsigned sz = nir_src_bit_size(instr->src[i].src);
      unsigned components = nir_src_num_components(instr->src[i].src);

      switch (instr->src[i].src_type) {
      case nir_tex_src_coord:
         if (instr->sampler_dim == GLSL_SAMPLER_DIM_CUBE) {
            sregs[VALHALL_TEX_SREG_X_COORD] = bi_emit_texc_cube_coord(
               b, index, &sregs[VALHALL_TEX_SREG_Y_COORD]);
         } else {
            assert(components >= 1 && components <= 3);

            /* Copy XY (for 2D+) or XX (for 1D) */
            sregs[VALHALL_TEX_SREG_X_COORD] = index;

            if (components >= 2)
               sregs[VALHALL_TEX_SREG_Y_COORD] = bi_extract(b, index, 1);

            if (components == 3 && !instr->is_array) {
               sregs[VALHALL_TEX_SREG_Z_COORD] = bi_extract(b, index, 2);
            }
         }

         if (instr->is_array) {
            sregs[VALHALL_TEX_SREG_ARRAY] =
               bi_extract(b, index, components - 1);
         }

         break;

      case nir_tex_src_lod:
         if (nir_src_is_const(instr->src[i].src) &&
             nir_src_as_uint(instr->src[i].src) == 0) {
            lod_mode = BI_VA_LOD_MODE_ZERO_LOD;
         } else if (has_lod_mode) {
            lod_mode = BI_VA_LOD_MODE_EXPLICIT;

            assert(sz == 16 || sz == 32);
            sregs[VALHALL_TEX_SREG_LOD] =
               bi_emit_texc_lod_88(b, index, sz == 16);
         }
         break;

      case nir_tex_src_bias:
         /* Upper 16-bits interpreted as a clamp, leave zero */
         assert(sz == 16 || sz == 32);
         sregs[VALHALL_TEX_SREG_LOD] = bi_emit_texc_lod_88(b, index, sz == 16);

         lod_mode = BI_VA_LOD_MODE_COMPUTED_BIAS;
         break;
      case nir_tex_src_ms_index:
      case nir_tex_src_offset:
         /* Handled below */
         break;

      case nir_tex_src_comparator:
         sregs[VALHALL_TEX_SREG_SHADOW] = index;
         break;

      case nir_tex_src_texture_offset:
         texture = index;
         break;

      case nir_tex_src_sampler_offset:
         sampler = index;
         break;

      default:
         unreachable("Unhandled src type in tex emit");
      }
   }

   /* Generate packed offset + ms index + LOD register. These default to
    * zero so we only need to encode if these features are actually in use.
    */
   bi_index offsets = bi_emit_valhall_offsets(b, instr);

   if (!bi_is_equiv(offsets, bi_zero())) {
      sregs[VALHALL_TEX_SREG_OFFSETMS] = offsets;
      explicit_offset = true;
   }

   /* Allocate staging registers contiguously by compacting the array. */
   unsigned sr_count = 0;

   for (unsigned i = 0; i < ARRAY_SIZE(sregs); ++i) {
      if (!bi_is_null(sregs[i]))
         sregs[sr_count++] = sregs[i];
   }

   bi_index idx = sr_count ? bi_temp(b->shader) : bi_null();

   if (sr_count)
      bi_make_vec_to(b, idx, sregs, NULL, sr_count, 32);

   bi_index image_src = bi_imm_u32(tables);
   image_src = bi_lshift_or_i32(b, sampler, image_src, bi_imm_u8(0));
   image_src = bi_lshift_or_i32(b, texture, image_src, bi_imm_u8(16));

   /* Only write the components that we actually read */
   unsigned mask = nir_def_components_read(&instr->def);
   unsigned comps_per_reg = instr->def.bit_size == 16 ? 2 : 1;
   unsigned res_size = DIV_ROUND_UP(util_bitcount(mask), comps_per_reg);

   enum bi_register_format regfmt = bi_reg_fmt_for_nir(instr->dest_type);
   enum bi_dimension dim = valhall_tex_dimension(instr->sampler_dim);
   bi_index dest = bi_temp(b->shader);

   switch (instr->op) {
   case nir_texop_tex:
   case nir_texop_txl:
   case nir_texop_txb:
      bi_tex_single_to(b, dest, idx, image_src, bi_zero(), instr->is_array, dim,
                       regfmt, instr->is_shadow, explicit_offset, lod_mode,
                       mask, sr_count);
      break;
   case nir_texop_txf:
   case nir_texop_txf_ms:
      bi_tex_fetch_to(b, dest, idx, image_src, bi_zero(), instr->is_array, dim,
                      regfmt, explicit_offset, mask, sr_count);
      break;
   case nir_texop_tg4:
      bi_tex_gather_to(b, dest, idx, image_src, bi_zero(), instr->is_array, dim,
                       instr->component, false, regfmt, instr->is_shadow,
                       explicit_offset, mask, sr_count);
      break;
   default:
      unreachable("Unhandled Valhall texture op");
   }

   /* The hardware will write only what we read, and it will into
    * contiguous registers without gaps (different from Bifrost). NIR
    * expects the gaps, so fill in the holes (they'll be copypropped and
    * DCE'd away later).
    */
   bi_index unpacked[4] = {bi_null(), bi_null(), bi_null(), bi_null()};

   bi_emit_cached_split_i32(b, dest, res_size);

   /* Index into the packed component array */
   unsigned j = 0;
   unsigned comps[4] = {0};
   unsigned nr_components = instr->def.num_components;

   for (unsigned i = 0; i < nr_components; ++i) {
      if (mask & BITFIELD_BIT(i)) {
         unpacked[i] = dest;
         comps[i] = j++;
      } else {
         unpacked[i] = bi_zero();
      }
   }

   bi_make_vec_to(b, bi_def_index(&instr->def), unpacked, comps,
                  instr->def.num_components, instr->def.bit_size);
}

/* Simple textures ops correspond to NIR tex or txl with LOD = 0 on 2D/cube
 * textures with sufficiently small immediate indices. Anything else
 * needs a complete texture op. */

static void
bi_emit_texs(bi_builder *b, nir_tex_instr *instr)
{
   int coord_idx = nir_tex_instr_src_index(instr, nir_tex_src_coord);
   assert(coord_idx >= 0);
   bi_index coords = bi_src_index(&instr->src[coord_idx].src);

   if (instr->sampler_dim == GLSL_SAMPLER_DIM_CUBE) {
      bi_index face, s, t;
      bi_emit_cube_coord(b, coords, &face, &s, &t);

      bi_texs_cube_to(b, instr->def.bit_size, bi_def_index(&instr->def), s, t,
                      face, instr->sampler_index, instr->texture_index);
   } else {
      bi_texs_2d_to(b, instr->def.bit_size, bi_def_index(&instr->def),
                    bi_extract(b, coords, 0), bi_extract(b, coords, 1),
                    instr->op != nir_texop_tex, /* zero LOD */
                    instr->sampler_index, instr->texture_index);
   }

   bi_split_def(b, &instr->def);
}

static bool
bi_is_simple_tex(nir_tex_instr *instr)
{
   if (instr->op != nir_texop_tex && instr->op != nir_texop_txl)
      return false;

   if (instr->dest_type != nir_type_float32 &&
       instr->dest_type != nir_type_float16)
      return false;

   if (instr->is_shadow || instr->is_array)
      return false;

   switch (instr->sampler_dim) {
   case GLSL_SAMPLER_DIM_2D:
   case GLSL_SAMPLER_DIM_EXTERNAL:
   case GLSL_SAMPLER_DIM_RECT:
      break;

   case GLSL_SAMPLER_DIM_CUBE:
      /* LOD can't be specified with TEXS_CUBE */
      if (instr->op == nir_texop_txl)
         return false;
      break;

   default:
      return false;
   }

   for (unsigned i = 0; i < instr->num_srcs; ++i) {
      if (instr->src[i].src_type != nir_tex_src_lod &&
          instr->src[i].src_type != nir_tex_src_coord)
         return false;
   }

   /* Indices need to fit in provided bits */
   unsigned idx_bits = instr->sampler_dim == GLSL_SAMPLER_DIM_CUBE ? 2 : 3;
   if (MAX2(instr->sampler_index, instr->texture_index) >= (1 << idx_bits))
      return false;

   int lod_idx = nir_tex_instr_src_index(instr, nir_tex_src_lod);
   if (lod_idx < 0)
      return true;

   nir_src lod = instr->src[lod_idx].src;
   return nir_src_is_const(lod) && nir_src_as_uint(lod) == 0;
}

static void
bi_emit_tex(bi_builder *b, nir_tex_instr *instr)
{
   /* If txf is used, we assume there is a valid sampler bound at index 0. Use
    * it for txf operations, since there may be no other valid samplers. This is
    * a workaround: txf does not require a sampler in NIR (so sampler_index is
    * undefined) but we need one in the hardware. This is ABI with the driver.
    */
   if (!nir_tex_instr_need_sampler(instr))
      instr->sampler_index = 0;

   if (b->shader->arch >= 9)
      bi_emit_tex_valhall(b, instr);
   else if (bi_is_simple_tex(instr))
      bi_emit_texs(b, instr);
   else
      bi_emit_texc(b, instr);
}

static void
bi_emit_phi(bi_builder *b, nir_phi_instr *instr)
{
   unsigned nr_srcs = exec_list_length(&instr->srcs);
   bi_instr *I = bi_phi_to(b, bi_def_index(&instr->def), nr_srcs);

   /* Deferred */
   I->phi = instr;
}

/* Look up the AGX block corresponding to a given NIR block. Used when
 * translating phi nodes after emitting all blocks.
 */
static bi_block *
bi_from_nir_block(bi_context *ctx, nir_block *block)
{
   return ctx->indexed_nir_blocks[block->index];
}

static void
bi_emit_phi_deferred(bi_context *ctx, bi_block *block, bi_instr *I)
{
   nir_phi_instr *phi = I->phi;

   /* Guaranteed by lower_phis_to_scalar */
   assert(phi->def.num_components == 1);

   nir_foreach_phi_src(src, phi) {
      bi_block *pred = bi_from_nir_block(ctx, src->pred);
      unsigned i = bi_predecessor_index(block, pred);
      assert(i < I->nr_srcs);

      I->src[i] = bi_src_index(&src->src);
   }

   I->phi = NULL;
}

static void
bi_emit_phis_deferred(bi_context *ctx)
{
   bi_foreach_block(ctx, block) {
      bi_foreach_instr_in_block(block, I) {
         if (I->op == BI_OPCODE_PHI)
            bi_emit_phi_deferred(ctx, block, I);
      }
   }
}

static void
bi_emit_instr(bi_builder *b, struct nir_instr *instr)
{
   switch (instr->type) {
   case nir_instr_type_load_const:
      bi_emit_load_const(b, nir_instr_as_load_const(instr));
      break;

   case nir_instr_type_intrinsic:
      bi_emit_intrinsic(b, nir_instr_as_intrinsic(instr));
      break;

   case nir_instr_type_alu:
      bi_emit_alu(b, nir_instr_as_alu(instr));
      break;

   case nir_instr_type_tex:
      bi_emit_tex(b, nir_instr_as_tex(instr));
      break;

   case nir_instr_type_jump:
      bi_emit_jump(b, nir_instr_as_jump(instr));
      break;

   case nir_instr_type_phi:
      bi_emit_phi(b, nir_instr_as_phi(instr));
      break;

   default:
      unreachable("should've been lowered");
   }
}

static bi_block *
create_empty_block(bi_context *ctx)
{
   bi_block *blk = rzalloc(ctx, bi_block);

   util_dynarray_init(&blk->predecessors, blk);

   return blk;
}

static bi_block *
emit_block(bi_context *ctx, nir_block *block)
{
   if (ctx->after_block) {
      ctx->current_block = ctx->after_block;
      ctx->after_block = NULL;
   } else {
      ctx->current_block = create_empty_block(ctx);
   }

   list_addtail(&ctx->current_block->link, &ctx->blocks);
   list_inithead(&ctx->current_block->instructions);

   bi_builder _b = bi_init_builder(ctx, bi_after_block(ctx->current_block));

   ctx->indexed_nir_blocks[block->index] = ctx->current_block;

   nir_foreach_instr(instr, block) {
      bi_emit_instr(&_b, instr);
   }

   return ctx->current_block;
}

static void
emit_if(bi_context *ctx, nir_if *nif)
{
   bi_block *before_block = ctx->current_block;

   /* Speculatively emit the branch, but we can't fill it in until later */
   bi_builder _b = bi_init_builder(ctx, bi_after_block(ctx->current_block));
   bi_instr *then_branch =
      bi_branchz_i16(&_b, bi_half(bi_src_index(&nif->condition), false),
                     bi_zero(), BI_CMPF_EQ);

   /* Emit the two subblocks. */
   bi_block *then_block = emit_cf_list(ctx, &nif->then_list);
   bi_block *end_then_block = ctx->current_block;

   /* Emit second block */

   bi_block *else_block = emit_cf_list(ctx, &nif->else_list);
   bi_block *end_else_block = ctx->current_block;
   ctx->after_block = create_empty_block(ctx);

   /* Now that we have the subblocks emitted, fix up the branches */

   assert(then_block);
   assert(else_block);

   then_branch->branch_target = else_block;

   /* Emit a jump from the end of the then block to the end of the else */
   _b.cursor = bi_after_block(end_then_block);
   bi_instr *then_exit = bi_jump(&_b, bi_zero());
   then_exit->branch_target = ctx->after_block;

   bi_block_add_successor(end_then_block, then_exit->branch_target);
   bi_block_add_successor(end_else_block, ctx->after_block); /* fallthrough */

   bi_block_add_successor(before_block,
                          then_branch->branch_target); /* then_branch */
   bi_block_add_successor(before_block, then_block);   /* fallthrough */
}

static void
emit_loop(bi_context *ctx, nir_loop *nloop)
{
   assert(!nir_loop_has_continue_construct(nloop));

   /* Remember where we are */
   bi_block *start_block = ctx->current_block;

   bi_block *saved_break = ctx->break_block;
   bi_block *saved_continue = ctx->continue_block;

   ctx->continue_block = create_empty_block(ctx);
   ctx->break_block = create_empty_block(ctx);
   ctx->after_block = ctx->continue_block;

   /* Emit the body itself */
   emit_cf_list(ctx, &nloop->body);

   /* Branch back to loop back */
   bi_builder _b = bi_init_builder(ctx, bi_after_block(ctx->current_block));
   bi_instr *I = bi_jump(&_b, bi_zero());
   I->branch_target = ctx->continue_block;
   bi_block_add_successor(start_block, ctx->continue_block);
   bi_block_add_successor(ctx->current_block, ctx->continue_block);

   ctx->after_block = ctx->break_block;

   /* Pop off */
   ctx->break_block = saved_break;
   ctx->continue_block = saved_continue;
   ++ctx->loop_count;
}

static bi_block *
emit_cf_list(bi_context *ctx, struct exec_list *list)
{
   bi_block *start_block = NULL;

   foreach_list_typed(nir_cf_node, node, node, list) {
      switch (node->type) {
      case nir_cf_node_block: {
         bi_block *block = emit_block(ctx, nir_cf_node_as_block(node));

         if (!start_block)
            start_block = block;

         break;
      }

      case nir_cf_node_if:
         emit_if(ctx, nir_cf_node_as_if(node));
         break;

      case nir_cf_node_loop:
         emit_loop(ctx, nir_cf_node_as_loop(node));
         break;

      default:
         unreachable("Unknown control flow");
      }
   }

   return start_block;
}

/* shader-db stuff */

struct bi_stats {
   unsigned nr_clauses, nr_tuples, nr_ins;
   unsigned nr_arith, nr_texture, nr_varying, nr_ldst;
};

static void
bi_count_tuple_stats(bi_clause *clause, bi_tuple *tuple, struct bi_stats *stats)
{
   /* Count instructions */
   stats->nr_ins += (tuple->fma ? 1 : 0) + (tuple->add ? 1 : 0);

   /* Non-message passing tuples are always arithmetic */
   if (tuple->add != clause->message) {
      stats->nr_arith++;
      return;
   }

   /* Message + FMA we'll count as arithmetic _and_ message */
   if (tuple->fma)
      stats->nr_arith++;

   switch (clause->message_type) {
   case BIFROST_MESSAGE_VARYING:
      /* Check components interpolated */
      stats->nr_varying +=
         (clause->message->vecsize + 1) *
         (bi_is_regfmt_16(clause->message->register_format) ? 1 : 2);
      break;

   case BIFROST_MESSAGE_VARTEX:
      /* 2 coordinates, fp32 each */
      stats->nr_varying += (2 * 2);
      FALLTHROUGH;
   case BIFROST_MESSAGE_TEX:
      stats->nr_texture++;
      break;

   case BIFROST_MESSAGE_ATTRIBUTE:
   case BIFROST_MESSAGE_LOAD:
   case BIFROST_MESSAGE_STORE:
   case BIFROST_MESSAGE_ATOMIC:
      stats->nr_ldst++;
      break;

   case BIFROST_MESSAGE_NONE:
   case BIFROST_MESSAGE_BARRIER:
   case BIFROST_MESSAGE_BLEND:
   case BIFROST_MESSAGE_TILE:
   case BIFROST_MESSAGE_Z_STENCIL:
   case BIFROST_MESSAGE_ATEST:
   case BIFROST_MESSAGE_JOB:
   case BIFROST_MESSAGE_64BIT:
      /* Nothing to do */
      break;
   };
}

/*
 * v7 allows preloading LD_VAR or VAR_TEX messages that must complete before the
 * shader completes. These costs are not accounted for in the general cycle
 * counts, so this function calculates the effective cost of these messages, as
 * if they were executed by shader code.
 */
static unsigned
bi_count_preload_cost(bi_context *ctx)
{
   /* Units: 1/16 of a normalized cycle, assuming that we may interpolate
    * 16 fp16 varying components per cycle or fetch two texels per cycle.
    */
   unsigned cost = 0;

   for (unsigned i = 0; i < ARRAY_SIZE(ctx->info.bifrost->messages); ++i) {
      struct bifrost_message_preload msg = ctx->info.bifrost->messages[i];

      if (msg.enabled && msg.texture) {
         /* 2 coordinate, 2 half-words each, plus texture */
         cost += 12;
      } else if (msg.enabled) {
         cost += (msg.num_components * (msg.fp16 ? 1 : 2));
      }
   }

   return cost;
}

static const char *
bi_shader_stage_name(bi_context *ctx)
{
   if (ctx->idvs == BI_IDVS_VARYING)
      return "MESA_SHADER_VARYING";
   else if (ctx->idvs == BI_IDVS_POSITION)
      return "MESA_SHADER_POSITION";
   else if (ctx->inputs->is_blend)
      return "MESA_SHADER_BLEND";
   else
      return gl_shader_stage_name(ctx->stage);
}

static char *
bi_print_stats(bi_context *ctx, unsigned size)
{
   struct bi_stats stats = {0};

   /* Count instructions, clauses, and tuples. Also attempt to construct
    * normalized execution engine cycle counts, using the following ratio:
    *
    * 24 arith tuples/cycle
    * 2 texture messages/cycle
    * 16 x 16-bit varying channels interpolated/cycle
    * 1 load store message/cycle
    *
    * These numbers seem to match Arm Mobile Studio's heuristic. The real
    * cycle counts are surely more complicated.
    */

   bi_foreach_block(ctx, block) {
      bi_foreach_clause_in_block(block, clause) {
         stats.nr_clauses++;
         stats.nr_tuples += clause->tuple_count;

         for (unsigned i = 0; i < clause->tuple_count; ++i)
            bi_count_tuple_stats(clause, &clause->tuples[i], &stats);
      }
   }

   float cycles_arith = ((float)stats.nr_arith) / 24.0;
   float cycles_texture = ((float)stats.nr_texture) / 2.0;
   float cycles_varying = ((float)stats.nr_varying) / 16.0;
   float cycles_ldst = ((float)stats.nr_ldst) / 1.0;

   float cycles_message = MAX3(cycles_texture, cycles_varying, cycles_ldst);
   float cycles_bound = MAX2(cycles_arith, cycles_message);

   /* Thread count and register pressure are traded off only on v7 */
   bool full_threads = (ctx->arch == 7 && ctx->info.work_reg_count <= 32);
   unsigned nr_threads = full_threads ? 2 : 1;

   /* Dump stats */
   char *str = ralloc_asprintf(
      NULL,
      "%s shader: "
      "%u inst, %u tuples, %u clauses, "
      "%f cycles, %f arith, %f texture, %f vary, %f ldst, "
      "%u quadwords, %u threads",
      bi_shader_stage_name(ctx), stats.nr_ins, stats.nr_tuples,
      stats.nr_clauses, cycles_bound, cycles_arith, cycles_texture,
      cycles_varying, cycles_ldst, size / 16, nr_threads);

   if (ctx->arch == 7) {
      ralloc_asprintf_append(&str, ", %u preloads", bi_count_preload_cost(ctx));
   }

   ralloc_asprintf_append(&str, ", %u loops, %u:%u spills:fills",
                          ctx->loop_count, ctx->spills, ctx->fills);

   return str;
}

static char *
va_print_stats(bi_context *ctx, unsigned size)
{
   unsigned nr_ins = 0;
   struct va_stats stats = {0};

   /* Count instructions */
   bi_foreach_instr_global(ctx, I) {
      nr_ins++;
      va_count_instr_stats(I, &stats);
   }

   /* Mali G78 peak performance:
    *
    * 64 FMA instructions per cycle
    * 64 CVT instructions per cycle
    * 16 SFU instructions per cycle
    * 8 x 32-bit varying channels interpolated per cycle
    * 4 texture instructions per cycle
    * 1 load/store operation per cycle
    */

   float cycles_fma = ((float)stats.fma) / 64.0;
   float cycles_cvt = ((float)stats.cvt) / 64.0;
   float cycles_sfu = ((float)stats.sfu) / 16.0;
   float cycles_v = ((float)stats.v) / 16.0;
   float cycles_t = ((float)stats.t) / 4.0;
   float cycles_ls = ((float)stats.ls) / 1.0;

   /* Calculate the bound */
   float cycles = MAX2(MAX3(cycles_fma, cycles_cvt, cycles_sfu),
                       MAX3(cycles_v, cycles_t, cycles_ls));

   /* Thread count and register pressure are traded off */
   unsigned nr_threads = (ctx->info.work_reg_count <= 32) ? 2 : 1;

   /* Dump stats */
   return ralloc_asprintf(NULL,
                          "%s shader: "
                          "%u inst, %f cycles, %f fma, %f cvt, %f sfu, %f v, "
                          "%f t, %f ls, %u quadwords, %u threads, %u loops, "
                          "%u:%u spills:fills",
                          bi_shader_stage_name(ctx), nr_ins, cycles, cycles_fma,
                          cycles_cvt, cycles_sfu, cycles_v, cycles_t, cycles_ls,
                          size / 16, nr_threads, ctx->loop_count, ctx->spills,
                          ctx->fills);
}

static int
glsl_type_size(const struct glsl_type *type, bool bindless)
{
   return glsl_count_attribute_slots(type, false);
}

/* Split stores to memory. We don't split stores to vertex outputs, since
 * nir_lower_io_to_temporaries will ensure there's only a single write.
 */

static bool
should_split_wrmask(const nir_instr *instr, UNUSED const void *data)
{
   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

   switch (intr->intrinsic) {
   case nir_intrinsic_store_ssbo:
   case nir_intrinsic_store_shared:
   case nir_intrinsic_store_global:
   case nir_intrinsic_store_scratch:
      return true;
   default:
      return false;
   }
}

/*
 * Some operations are only available as 32-bit instructions. 64-bit floats are
 * unsupported and ints are lowered with nir_lower_int64.  Certain 8-bit and
 * 16-bit instructions, however, are lowered here.
 */
static unsigned
bi_lower_bit_size(const nir_instr *instr, UNUSED void *data)
{
   if (instr->type != nir_instr_type_alu)
      return 0;

   nir_alu_instr *alu = nir_instr_as_alu(instr);

   switch (alu->op) {
   case nir_op_fexp2:
   case nir_op_flog2:
   case nir_op_fpow:
   case nir_op_fsin:
   case nir_op_fcos:
   case nir_op_bit_count:
   case nir_op_bitfield_reverse:
      return (nir_src_bit_size(alu->src[0].src) == 32) ? 0 : 32;
   default:
      return 0;
   }
}

/* Although Bifrost generally supports packed 16-bit vec2 and 8-bit vec4,
 * transcendentals are an exception. Also shifts because of lane size mismatch
 * (8-bit in Bifrost, 32-bit in NIR TODO - workaround!). Some conversions need
 * to be scalarized due to type size. */

static uint8_t
bi_vectorize_filter(const nir_instr *instr, const void *data)
{
   /* Defaults work for everything else */
   if (instr->type != nir_instr_type_alu)
      return 0;

   const nir_alu_instr *alu = nir_instr_as_alu(instr);

   switch (alu->op) {
   case nir_op_frcp:
   case nir_op_frsq:
   case nir_op_ishl:
   case nir_op_ishr:
   case nir_op_ushr:
   case nir_op_f2i16:
   case nir_op_f2u16:
   case nir_op_extract_u8:
   case nir_op_extract_i8:
   case nir_op_extract_u16:
   case nir_op_extract_i16:
   case nir_op_insert_u16:
      return 1;
   default:
      break;
   }

   /* Vectorized instructions cannot write more than 32-bit */
   int dst_bit_size = alu->def.bit_size;
   if (dst_bit_size == 16)
      return 2;
   else
      return 1;
}

static bool
bi_scalarize_filter(const nir_instr *instr, const void *data)
{
   if (instr->type != nir_instr_type_alu)
      return false;

   const nir_alu_instr *alu = nir_instr_as_alu(instr);

   switch (alu->op) {
   case nir_op_pack_uvec2_to_uint:
   case nir_op_pack_uvec4_to_uint:
      return false;
   default:
      return true;
   }
}

/* Ensure we write exactly 4 components */
static nir_def *
bifrost_nir_valid_channel(nir_builder *b, nir_def *in, unsigned channel,
                          unsigned first, unsigned mask)
{
   if (!(mask & BITFIELD_BIT(channel)))
      channel = first;

   return nir_channel(b, in, channel);
}

/* Lower fragment store_output instructions to always write 4 components,
 * matching the hardware semantic. This may require additional moves. Skipping
 * these moves is possible in theory, but invokes undefined behaviour in the
 * compiler. The DDK inserts these moves, so we will as well. */

static bool
bifrost_nir_lower_blend_components(struct nir_builder *b,
                                   nir_intrinsic_instr *intr, void *data)
{
   if (intr->intrinsic != nir_intrinsic_store_output)
      return false;

   nir_def *in = intr->src[0].ssa;
   unsigned first = nir_intrinsic_component(intr);
   unsigned mask = nir_intrinsic_write_mask(intr);

   assert(first == 0 && "shouldn't get nonzero components");

   /* Nothing to do */
   if (mask == BITFIELD_MASK(4))
      return false;

   b->cursor = nir_before_instr(&intr->instr);

   /* Replicate the first valid component instead */
   nir_def *replicated =
      nir_vec4(b, bifrost_nir_valid_channel(b, in, 0, first, mask),
               bifrost_nir_valid_channel(b, in, 1, first, mask),
               bifrost_nir_valid_channel(b, in, 2, first, mask),
               bifrost_nir_valid_channel(b, in, 3, first, mask));

   /* Rewrite to use our replicated version */
   nir_src_rewrite(&intr->src[0], replicated);
   nir_intrinsic_set_component(intr, 0);
   nir_intrinsic_set_write_mask(intr, 0xF);
   intr->num_components = 4;

   return true;
}

static nir_mem_access_size_align
mem_access_size_align_cb(nir_intrinsic_op intrin, uint8_t bytes,
                         uint8_t input_bit_size, uint32_t align,
                         uint32_t align_offset, bool offset_is_const,
                         const void *cb_data)
{
   align = nir_combined_align(align, align_offset);
   assert(util_is_power_of_two_nonzero(align));

   /* If the number of bytes is a multiple of 4, use 32-bit loads. Else if it's
    * a multiple of 2, use 16-bit loads. Else use 8-bit loads.
    */
   unsigned bit_size = (bytes & 1) ? 8 : (bytes & 2) ? 16 : 32;

   /* But if we're only aligned to 1 byte, use 8-bit loads. If we're only
    * aligned to 2 bytes, use 16-bit loads, unless we needed 8-bit loads due to
    * the size.
    */
   if (align == 1)
      bit_size = 8;
   else if (align == 2)
      bit_size = MIN2(bit_size, 16);

   return (nir_mem_access_size_align){
      .num_components = MIN2(bytes / (bit_size / 8), 4),
      .bit_size = bit_size,
      .align = bit_size / 8,
   };
}

static void
bi_optimize_nir(nir_shader *nir, unsigned gpu_id, bool is_blend)
{
   bool progress;

   do {
      progress = false;

      NIR_PASS(progress, nir, nir_lower_vars_to_ssa);
      NIR_PASS(progress, nir, nir_lower_wrmasks, should_split_wrmask, NULL);

      NIR_PASS(progress, nir, nir_copy_prop);
      NIR_PASS(progress, nir, nir_opt_remove_phis);
      NIR_PASS(progress, nir, nir_opt_dce);
      NIR_PASS(progress, nir, nir_opt_dead_cf);
      NIR_PASS(progress, nir, nir_opt_cse);
      NIR_PASS(progress, nir, nir_opt_peephole_select, 64, false, true);
      NIR_PASS(progress, nir, nir_opt_algebraic);
      NIR_PASS(progress, nir, nir_opt_constant_folding);

      NIR_PASS(progress, nir, nir_opt_undef);
      NIR_PASS(progress, nir, nir_lower_undef_to_zero);

      NIR_PASS(progress, nir, nir_opt_shrink_vectors);
      NIR_PASS(progress, nir, nir_opt_loop_unroll);
   } while (progress);

   /* TODO: Why is 64-bit getting rematerialized?
    * KHR-GLES31.core.shader_image_load_store.basic-allTargets-atomicFS */
   NIR_PASS(progress, nir, nir_lower_int64);

   /* We need to cleanup after each iteration of late algebraic
    * optimizations, since otherwise NIR can produce weird edge cases
    * (like fneg of a constant) which we don't handle */
   bool late_algebraic = true;
   while (late_algebraic) {
      late_algebraic = false;
      NIR_PASS(late_algebraic, nir, nir_opt_algebraic_late);
      NIR_PASS(progress, nir, nir_opt_constant_folding);
      NIR_PASS(progress, nir, nir_copy_prop);
      NIR_PASS(progress, nir, nir_opt_dce);
      NIR_PASS(progress, nir, nir_opt_cse);
   }

   /* This opt currently helps on Bifrost but not Valhall */
   if (gpu_id < 0x9000)
      NIR_PASS(progress, nir, bifrost_nir_opt_boolean_bitwise);

   NIR_PASS(progress, nir, nir_lower_alu_to_scalar, bi_scalarize_filter, NULL);
   NIR_PASS(progress, nir, nir_opt_vectorize, bi_vectorize_filter, NULL);
   NIR_PASS(progress, nir, nir_lower_bool_to_bitsize);

   /* Prepass to simplify instruction selection */
   late_algebraic = false;
   NIR_PASS(late_algebraic, nir, bifrost_nir_lower_algebraic_late);

   while (late_algebraic) {
      late_algebraic = false;
      NIR_PASS(late_algebraic, nir, nir_opt_algebraic_late);
      NIR_PASS(progress, nir, nir_opt_constant_folding);
      NIR_PASS(progress, nir, nir_copy_prop);
      NIR_PASS(progress, nir, nir_opt_dce);
      NIR_PASS(progress, nir, nir_opt_cse);
   }

   NIR_PASS(progress, nir, nir_lower_load_const_to_scalar);
   NIR_PASS(progress, nir, nir_opt_dce);

   if (nir->info.stage == MESA_SHADER_FRAGMENT) {
      NIR_PASS_V(nir, nir_shader_intrinsics_pass,
                 bifrost_nir_lower_blend_components,
                 nir_metadata_block_index | nir_metadata_dominance, NULL);
   }

   /* Backend scheduler is purely local, so do some global optimizations
    * to reduce register pressure. */
   nir_move_options move_all = nir_move_const_undef | nir_move_load_ubo |
                               nir_move_load_input | nir_move_comparisons |
                               nir_move_copies | nir_move_load_ssbo;

   NIR_PASS_V(nir, nir_opt_sink, move_all);
   NIR_PASS_V(nir, nir_opt_move, move_all);

   /* We might lower attribute, varying, and image indirects. Use the
    * gathered info to skip the extra analysis in the happy path. */
   bool any_indirects = nir->info.inputs_read_indirectly ||
                        nir->info.outputs_accessed_indirectly ||
                        nir->info.patch_inputs_read_indirectly ||
                        nir->info.patch_outputs_accessed_indirectly ||
                        nir->info.images_used[0];

   if (any_indirects) {
      nir_convert_to_lcssa(nir, true, true);
      NIR_PASS_V(nir, nir_divergence_analysis);
      NIR_PASS_V(nir, bi_lower_divergent_indirects,
                 pan_subgroup_size(gpu_id >> 12));
   }
}

static void
bi_opt_post_ra(bi_context *ctx)
{
   bi_foreach_instr_global_safe(ctx, ins) {
      if (ins->op == BI_OPCODE_MOV_I32 &&
          bi_is_equiv(ins->dest[0], ins->src[0]))
         bi_remove_instruction(ins);
   }
}

/* Dead code elimination for branches at the end of a block - only one branch
 * per block is legal semantically, but unreachable jumps can be generated.
 * Likewise on Bifrost we can generate jumps to the terminal block which need
 * to be lowered away to a jump to #0x0, which induces successful termination.
 * That trick doesn't work on Valhall, which needs a NOP inserted in the
 * terminal block instead.
 */
static void
bi_lower_branch(bi_context *ctx, bi_block *block)
{
   bool cull_terminal = (ctx->arch <= 8);
   bool branched = false;

   bi_foreach_instr_in_block_safe(block, ins) {
      if (!ins->branch_target)
         continue;

      if (branched) {
         bi_remove_instruction(ins);
         continue;
      }

      branched = true;

      if (!bi_is_terminal_block(ins->branch_target))
         continue;

      if (cull_terminal)
         ins->branch_target = NULL;
      else if (ins->branch_target)
         ins->branch_target->needs_nop = true;
   }
}

static void
bi_pack_clauses(bi_context *ctx, struct util_dynarray *binary, unsigned offset)
{
   unsigned final_clause = bi_pack(ctx, binary);

   /* If we need to wait for ATEST or BLEND in the first clause, pass the
    * corresponding bits through to the renderer state descriptor */
   bi_block *first_block = list_first_entry(&ctx->blocks, bi_block, link);
   bi_clause *first_clause = bi_next_clause(ctx, first_block, NULL);

   unsigned first_deps = first_clause ? first_clause->dependencies : 0;
   ctx->info.bifrost->wait_6 = (first_deps & (1 << 6));
   ctx->info.bifrost->wait_7 = (first_deps & (1 << 7));

   /* Pad the shader with enough zero bytes to trick the prefetcher,
    * unless we're compiling an empty shader (in which case we don't pad
    * so the size remains 0) */
   unsigned prefetch_size = BIFROST_SHADER_PREFETCH - final_clause;

   if (binary->size - offset) {
      memset(util_dynarray_grow(binary, uint8_t, prefetch_size), 0,
             prefetch_size);
   }
}

/*
 * Build a bit mask of varyings (by location) that are flatshaded. This
 * information is needed by lower_mediump_io, as we don't yet support 16-bit
 * flat varyings.
 *
 * Also varyings that are used as texture coordinates should be kept at fp32 so
 * the texture instruction may be promoted to VAR_TEX. In general this is a good
 * idea, as fp16 texture coordinates are not supported by the hardware and are
 * usually inappropriate. (There are both relevant CTS bugs here, even.)
 *
 * TODO: If we compacted the varyings with some fixup code in the vertex shader,
 * we could implement 16-bit flat varyings. Consider if this case matters.
 *
 * TODO: The texture coordinate handling could be less heavyhanded.
 */
static bool
bi_gather_texcoords(nir_builder *b, nir_instr *instr, void *data)
{
   uint64_t *mask = data;

   if (instr->type != nir_instr_type_tex)
      return false;

   nir_tex_instr *tex = nir_instr_as_tex(instr);

   int coord_idx = nir_tex_instr_src_index(tex, nir_tex_src_coord);
   if (coord_idx < 0)
      return false;

   nir_src src = tex->src[coord_idx].src;
   nir_scalar x = nir_scalar_resolved(src.ssa, 0);
   nir_scalar y = nir_scalar_resolved(src.ssa, 1);

   if (x.def != y.def)
      return false;

   nir_instr *parent = x.def->parent_instr;

   if (parent->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(parent);

   if (intr->intrinsic != nir_intrinsic_load_interpolated_input)
      return false;

   nir_io_semantics sem = nir_intrinsic_io_semantics(intr);
   *mask |= BITFIELD64_BIT(sem.location);
   return false;
}

static uint64_t
bi_fp32_varying_mask(nir_shader *nir)
{
   uint64_t mask = 0;

   assert(nir->info.stage == MESA_SHADER_FRAGMENT);

   nir_foreach_shader_in_variable(var, nir) {
      if (var->data.interpolation == INTERP_MODE_FLAT)
         mask |= BITFIELD64_BIT(var->data.location);
   }

   nir_shader_instructions_pass(nir, bi_gather_texcoords, nir_metadata_all,
                                &mask);

   return mask;
}

static bool
bi_lower_sample_mask_writes(nir_builder *b, nir_intrinsic_instr *intr,
                            void *data)
{
   if (intr->intrinsic != nir_intrinsic_store_output)
      return false;

   assert(b->shader->info.stage == MESA_SHADER_FRAGMENT);
   if (nir_intrinsic_io_semantics(intr).location != FRAG_RESULT_SAMPLE_MASK)
      return false;

   b->cursor = nir_before_instr(&intr->instr);

   nir_def *orig = nir_load_sample_mask(b);

   nir_src_rewrite(&intr->src[0],
                   nir_b32csel(b, nir_load_multisampled_pan(b),
                               nir_iand(b, orig, intr->src[0].ssa), orig));
   return true;
}

static bool
bi_lower_load_output(nir_builder *b, nir_intrinsic_instr *intr,
                     UNUSED void *data)
{
   if (intr->intrinsic != nir_intrinsic_load_output)
      return false;

   unsigned loc = nir_intrinsic_io_semantics(intr).location;
   assert(loc >= FRAG_RESULT_DATA0);
   unsigned rt = loc - FRAG_RESULT_DATA0;

   b->cursor = nir_before_instr(&intr->instr);

   nir_def *conversion = nir_load_rt_conversion_pan(
      b, .base = rt, .src_type = nir_intrinsic_dest_type(intr));

   nir_def *lowered = nir_load_converted_output_pan(
      b, intr->def.num_components, intr->def.bit_size, conversion,
      .dest_type = nir_intrinsic_dest_type(intr),
      .io_semantics = nir_intrinsic_io_semantics(intr));

   nir_def_rewrite_uses(&intr->def, lowered);
   return true;
}

bool
bifrost_nir_lower_load_output(nir_shader *nir)
{
   assert(nir->info.stage == MESA_SHADER_FRAGMENT);

   return nir_shader_intrinsics_pass(
      nir, bi_lower_load_output,
      nir_metadata_block_index | nir_metadata_dominance, NULL);
}

void
bifrost_preprocess_nir(nir_shader *nir, unsigned gpu_id)
{
   /* Lower gl_Position pre-optimisation, but after lowering vars to ssa
    * (so we don't accidentally duplicate the epilogue since mesa/st has
    * messed with our I/O quite a bit already) */

   NIR_PASS_V(nir, nir_lower_vars_to_ssa);

   if (nir->info.stage == MESA_SHADER_VERTEX) {
      NIR_PASS_V(nir, nir_lower_viewport_transform);
      NIR_PASS_V(nir, nir_lower_point_size, 1.0, 0.0);

      nir_variable *psiz = nir_find_variable_with_location(
         nir, nir_var_shader_out, VARYING_SLOT_PSIZ);
      if (psiz != NULL)
         psiz->data.precision = GLSL_PRECISION_MEDIUM;
   }

   /* Get rid of any global vars before we lower to scratch. */
   NIR_PASS_V(nir, nir_lower_global_vars_to_local);

   /* Valhall introduces packed thread local storage, which improves cache
    * locality of TLS access. However, access to packed TLS cannot
    * straddle 16-byte boundaries. As such, when packed TLS is in use
    * (currently unconditional for Valhall), we force vec4 alignment for
    * scratch access.
    */
   bool packed_tls = (gpu_id >= 0x9000);

   /* Lower large arrays to scratch and small arrays to bcsel */
   NIR_PASS_V(nir, nir_lower_vars_to_scratch, nir_var_function_temp, 256,
              packed_tls ? glsl_get_vec4_size_align_bytes
                         : glsl_get_natural_size_align_bytes);
   NIR_PASS_V(nir, nir_lower_indirect_derefs, nir_var_function_temp, ~0);

   NIR_PASS_V(nir, nir_split_var_copies);
   NIR_PASS_V(nir, nir_lower_var_copies);
   NIR_PASS_V(nir, nir_lower_vars_to_ssa);
   NIR_PASS_V(nir, nir_lower_io, nir_var_shader_in | nir_var_shader_out,
              glsl_type_size, 0);

   /* nir_lower[_explicit]_io is lazy and emits mul+add chains even for
    * offsets it could figure out are constant.  Do some constant folding
    * before bifrost_nir_lower_store_component below.
    */
   NIR_PASS_V(nir, nir_opt_constant_folding);

   if (nir->info.stage == MESA_SHADER_FRAGMENT) {
      NIR_PASS_V(nir, nir_lower_mediump_io,
                 nir_var_shader_in | nir_var_shader_out,
                 ~bi_fp32_varying_mask(nir), false);

      NIR_PASS_V(nir, nir_shader_intrinsics_pass, bi_lower_sample_mask_writes,
                 nir_metadata_block_index | nir_metadata_dominance, NULL);

      NIR_PASS_V(nir, bifrost_nir_lower_load_output);
   } else if (nir->info.stage == MESA_SHADER_VERTEX) {
      if (gpu_id >= 0x9000) {
         NIR_PASS_V(nir, nir_lower_mediump_io, nir_var_shader_out,
                    BITFIELD64_BIT(VARYING_SLOT_PSIZ), false);
      }

      NIR_PASS_V(nir, pan_nir_lower_store_component);
   }

   nir_lower_mem_access_bit_sizes_options mem_size_options = {
      .modes = nir_var_mem_ubo | nir_var_mem_ssbo | nir_var_mem_constant |
               nir_var_mem_task_payload | nir_var_shader_temp |
               nir_var_function_temp | nir_var_mem_global | nir_var_mem_shared,
      .callback = mem_access_size_align_cb,
   };
   NIR_PASS_V(nir, nir_lower_mem_access_bit_sizes, &mem_size_options);

   NIR_PASS_V(nir, nir_lower_ssbo);
   NIR_PASS_V(nir, pan_lower_sample_pos);
   NIR_PASS_V(nir, nir_lower_bit_size, bi_lower_bit_size, NULL);
   NIR_PASS_V(nir, nir_lower_64bit_phis);
   NIR_PASS_V(nir, pan_lower_helper_invocation);
   NIR_PASS_V(nir, nir_lower_int64);

   NIR_PASS_V(nir, nir_opt_idiv_const, 8);
   NIR_PASS_V(nir, nir_lower_idiv,
              &(nir_lower_idiv_options){.allow_fp16 = true});

   NIR_PASS_V(nir, nir_lower_tex,
              &(nir_lower_tex_options){
                 .lower_txs_lod = true,
                 .lower_txp = ~0,
                 .lower_tg4_broadcom_swizzle = true,
                 .lower_txd = true,
                 .lower_invalid_implicit_lod = true,
                 .lower_index_to_offset = true,
              });

   NIR_PASS_V(nir, nir_lower_image_atomics_to_global);
   NIR_PASS_V(nir, nir_lower_alu_to_scalar, bi_scalarize_filter, NULL);
   NIR_PASS_V(nir, nir_lower_load_const_to_scalar);
   NIR_PASS_V(nir, nir_lower_phis_to_scalar, true);
   NIR_PASS_V(nir, nir_lower_flrp, 16 | 32 | 64, false /* always_precise */);
   NIR_PASS_V(nir, nir_lower_var_copies);
   NIR_PASS_V(nir, nir_lower_alu);
   NIR_PASS_V(nir, nir_lower_frag_coord_to_pixel_coord);
}

static bi_context *
bi_compile_variant_nir(nir_shader *nir,
                       const struct panfrost_compile_inputs *inputs,
                       struct util_dynarray *binary, struct bi_shader_info info,
                       enum bi_idvs_mode idvs)
{
   bi_context *ctx = rzalloc(NULL, bi_context);

   /* There may be another program in the dynarray, start at the end */
   unsigned offset = binary->size;

   ctx->inputs = inputs;
   ctx->nir = nir;
   ctx->stage = nir->info.stage;
   ctx->quirks = bifrost_get_quirks(inputs->gpu_id);
   ctx->arch = inputs->gpu_id >> 12;
   ctx->info = info;
   ctx->idvs = idvs;
   ctx->malloc_idvs = (ctx->arch >= 9) && !inputs->no_idvs;

   if (idvs != BI_IDVS_NONE) {
      /* Specializing shaders for IDVS is destructive, so we need to
       * clone. However, the last (second) IDVS shader does not need
       * to be preserved so we can skip cloning that one.
       */
      if (offset == 0)
         ctx->nir = nir = nir_shader_clone(ctx, nir);

      NIR_PASS_V(nir, nir_shader_instructions_pass, bifrost_nir_specialize_idvs,
                 nir_metadata_block_index | nir_metadata_dominance, &idvs);

      /* After specializing, clean up the mess */
      bool progress = true;

      while (progress) {
         progress = false;

         NIR_PASS(progress, nir, nir_opt_dce);
         NIR_PASS(progress, nir, nir_opt_dead_cf);
      }
   }

   /* If nothing is pushed, all UBOs need to be uploaded */
   ctx->ubo_mask = ~0;

   list_inithead(&ctx->blocks);

   bool skip_internal = nir->info.internal;
   skip_internal &= !(bifrost_debug & BIFROST_DBG_INTERNAL);

   if (bifrost_debug & BIFROST_DBG_SHADERS && !skip_internal) {
      nir_print_shader(nir, stdout);
   }

   ctx->allocated_vec = _mesa_hash_table_u64_create(ctx);

   nir_foreach_function_impl(impl, nir) {
      nir_index_blocks(impl);

      ctx->indexed_nir_blocks =
         rzalloc_array(ctx, bi_block *, impl->num_blocks);

      ctx->ssa_alloc += impl->ssa_alloc;

      emit_cf_list(ctx, &impl->body);
      bi_emit_phis_deferred(ctx);
      break; /* TODO: Multi-function shaders */
   }

   /* Index blocks now that we're done emitting */
   bi_foreach_block(ctx, block) {
      block->index = ctx->num_blocks++;
   }

   bi_validate(ctx, "NIR -> BIR");

   /* If the shader doesn't write any colour or depth outputs, it may
    * still need an ATEST at the very end! */
   bool need_dummy_atest = (ctx->stage == MESA_SHADER_FRAGMENT) &&
                           !ctx->emitted_atest && !bi_skip_atest(ctx, false);

   if (need_dummy_atest) {
      bi_block *end = list_last_entry(&ctx->blocks, bi_block, link);
      bi_builder b = bi_init_builder(ctx, bi_after_block(end));
      bi_emit_atest(&b, bi_zero());
   }

   bool optimize = !(bifrost_debug & BIFROST_DBG_NOOPT);

   /* Runs before constant folding */
   bi_lower_swizzle(ctx);
   bi_validate(ctx, "Early lowering");

   /* Runs before copy prop */
   if (optimize && !ctx->inputs->no_ubo_to_push) {
      bi_opt_push_ubo(ctx);
   }

   if (likely(optimize)) {
      bi_opt_copy_prop(ctx);

      while (bi_opt_constant_fold(ctx))
         bi_opt_copy_prop(ctx);

      bi_opt_mod_prop_forward(ctx);
      bi_opt_mod_prop_backward(ctx);

      /* Push LD_VAR_IMM/VAR_TEX instructions. Must run after
       * mod_prop_backward to fuse VAR_TEX */
      if (ctx->arch == 7 && ctx->stage == MESA_SHADER_FRAGMENT &&
          !(bifrost_debug & BIFROST_DBG_NOPRELOAD)) {
         bi_opt_dead_code_eliminate(ctx);
         bi_opt_message_preload(ctx);
         bi_opt_copy_prop(ctx);
      }

      bi_opt_dead_code_eliminate(ctx);
      bi_opt_cse(ctx);
      bi_opt_dead_code_eliminate(ctx);
      if (!ctx->inputs->no_ubo_to_push)
         bi_opt_reorder_push(ctx);
      bi_validate(ctx, "Optimization passes");
   }

   bi_lower_opt_instructions(ctx);

   if (ctx->arch >= 9) {
      va_optimize(ctx);
      va_lower_isel(ctx);

      bi_foreach_instr_global_safe(ctx, I) {
         /* Phis become single moves so shouldn't be affected */
         if (I->op == BI_OPCODE_PHI)
            continue;

         va_lower_constants(ctx, I);

         bi_builder b = bi_init_builder(ctx, bi_before_instr(I));
         va_repair_fau(&b, I);
      }

      /* We need to clean up after constant lowering */
      if (likely(optimize)) {
         bi_opt_cse(ctx);
         bi_opt_dead_code_eliminate(ctx);
      }

      bi_validate(ctx, "Valhall passes");
   }

   bi_foreach_block(ctx, block) {
      bi_lower_branch(ctx, block);
   }

   if (bifrost_debug & BIFROST_DBG_SHADERS && !skip_internal)
      bi_print_shader(ctx, stdout);

   /* Analyze before register allocation to avoid false dependencies. The
    * skip bit is a function of only the data flow graph and is invariant
    * under valid scheduling. Helpers are only defined for fragment
    * shaders, so this analysis is only required in fragment shaders.
    */
   if (ctx->stage == MESA_SHADER_FRAGMENT)
      bi_analyze_helper_requirements(ctx);

   /* Fuse TEXC after analyzing helper requirements so the analysis
    * doesn't have to know about dual textures */
   if (likely(optimize)) {
      bi_opt_fuse_dual_texture(ctx);
   }

   /* Lower FAU after fusing dual texture, because fusing dual texture
    * creates new immediates that themselves may need lowering.
    */
   if (ctx->arch <= 8) {
      bi_lower_fau(ctx);
   }

   /* Lowering FAU can create redundant moves. Run CSE+DCE to clean up. */
   if (likely(optimize)) {
      bi_opt_cse(ctx);
      bi_opt_dead_code_eliminate(ctx);
   }

   bi_validate(ctx, "Late lowering");

   if (likely(!(bifrost_debug & BIFROST_DBG_NOPSCHED))) {
      bi_pressure_schedule(ctx);
      bi_validate(ctx, "Pre-RA scheduling");
   }

   bi_register_allocate(ctx);

   if (likely(optimize))
      bi_opt_post_ra(ctx);

   if (bifrost_debug & BIFROST_DBG_SHADERS && !skip_internal)
      bi_print_shader(ctx, stdout);

   if (ctx->arch >= 9) {
      va_assign_slots(ctx);
      va_insert_flow_control_nops(ctx);
      va_merge_flow(ctx);
      va_mark_last(ctx);
   } else {
      bi_schedule(ctx);
      bi_assign_scoreboard(ctx);

      /* Analyze after scheduling since we depend on instruction
       * order. Valhall calls as part of va_insert_flow_control_nops,
       * as the handling for clauses differs from instructions.
       */
      bi_analyze_helper_terminate(ctx);
      bi_mark_clauses_td(ctx);
   }

   if (bifrost_debug & BIFROST_DBG_SHADERS && !skip_internal)
      bi_print_shader(ctx, stdout);

   if (ctx->arch <= 8) {
      bi_pack_clauses(ctx, binary, offset);
   } else {
      bi_pack_valhall(ctx, binary);
   }

   if (bifrost_debug & BIFROST_DBG_SHADERS && !skip_internal) {
      if (ctx->arch <= 8) {
         disassemble_bifrost(stdout, binary->data + offset,
                             binary->size - offset,
                             bifrost_debug & BIFROST_DBG_VERBOSE);
      } else {
         disassemble_valhall(stdout, binary->data + offset,
                             binary->size - offset,
                             bifrost_debug & BIFROST_DBG_VERBOSE);
      }

      fflush(stdout);
   }

   if (!skip_internal &&
       ((bifrost_debug & BIFROST_DBG_SHADERDB) || inputs->debug)) {
      char *shaderdb;

      if (ctx->arch >= 9) {
         shaderdb = va_print_stats(ctx, binary->size - offset);
      } else {
         shaderdb = bi_print_stats(ctx, binary->size - offset);
      }

      if (bifrost_debug & BIFROST_DBG_SHADERDB)
         fprintf(stderr, "SHADER-DB: %s\n", shaderdb);

      if (inputs->debug)
         util_debug_message(inputs->debug, SHADER_INFO, "%s", shaderdb);

      ralloc_free(shaderdb);
   }

   return ctx;
}

static void
bi_compile_variant(nir_shader *nir,
                   const struct panfrost_compile_inputs *inputs,
                   struct util_dynarray *binary, struct pan_shader_info *info,
                   enum bi_idvs_mode idvs)
{
   struct bi_shader_info local_info = {
      .push = &info->push,
      .bifrost = &info->bifrost,
      .tls_size = info->tls_size,
      .push_offset = info->push.count,
   };

   unsigned offset = binary->size;

   /* If there is no position shader (gl_Position is not written), then
    * there is no need to build a varying shader either. This case is hit
    * for transform feedback only vertex shaders which only make sense with
    * rasterizer discard.
    */
   if ((offset == 0) && (idvs == BI_IDVS_VARYING))
      return;

   /* Software invariant: Only a secondary shader can appear at a nonzero
    * offset, to keep the ABI simple. */
   assert((offset == 0) ^ (idvs == BI_IDVS_VARYING));

   bi_context *ctx =
      bi_compile_variant_nir(nir, inputs, binary, local_info, idvs);

   /* A register is preloaded <==> it is live before the first block */
   bi_block *first_block = list_first_entry(&ctx->blocks, bi_block, link);
   uint64_t preload = first_block->reg_live_in;

   /* If multisampling is used with a blend shader, the blend shader needs
    * to access the sample coverage mask in r60 and the sample ID in r61.
    * Blend shaders run in the same context as fragment shaders, so if a
    * blend shader could run, we need to preload these registers
    * conservatively. There is believed to be little cost to doing so, so
    * do so always to avoid variants of the preload descriptor.
    *
    * We only do this on Valhall, as Bifrost has to update the RSD for
    * multisampling w/ blend shader anyway, so this is handled in the
    * driver. We could unify the paths if the cost is acceptable.
    */
   if (nir->info.stage == MESA_SHADER_FRAGMENT && ctx->arch >= 9)
      preload |= BITFIELD64_BIT(60) | BITFIELD64_BIT(61);

   info->ubo_mask |= ctx->ubo_mask;
   info->tls_size = MAX2(info->tls_size, ctx->info.tls_size);

   if (idvs == BI_IDVS_VARYING) {
      info->vs.secondary_enable = (binary->size > offset);
      info->vs.secondary_offset = offset;
      info->vs.secondary_preload = preload;
      info->vs.secondary_work_reg_count = ctx->info.work_reg_count;
   } else {
      info->preload = preload;
      info->work_reg_count = ctx->info.work_reg_count;
   }

   if (idvs == BI_IDVS_POSITION && !nir->info.internal &&
       nir->info.outputs_written & BITFIELD_BIT(VARYING_SLOT_PSIZ)) {
      /* Find the psiz write */
      bi_instr *write = NULL;

      bi_foreach_instr_global(ctx, I) {
         if (I->op == BI_OPCODE_STORE_I16 && I->seg == BI_SEG_POS) {
            write = I;
            break;
         }
      }

      assert(write != NULL);

      /* NOP it out, preserving its flow control. TODO: maybe DCE */
      if (write->flow) {
         bi_builder b = bi_init_builder(ctx, bi_before_instr(write));
         bi_instr *nop = bi_nop(&b);
         nop->flow = write->flow;
      }

      bi_remove_instruction(write);

      info->vs.no_psiz_offset = binary->size;
      bi_pack_valhall(ctx, binary);
   }

   ralloc_free(ctx);
}

/* Decide if Index-Driven Vertex Shading should be used for a given shader */
static bool
bi_should_idvs(nir_shader *nir, const struct panfrost_compile_inputs *inputs)
{
   /* Opt-out */
   if (inputs->no_idvs || bifrost_debug & BIFROST_DBG_NOIDVS)
      return false;

   /* IDVS splits up vertex shaders, not defined on other shader stages */
   if (nir->info.stage != MESA_SHADER_VERTEX)
      return false;

   /* Bifrost cannot write gl_PointSize during IDVS */
   if ((inputs->gpu_id < 0x9000) &&
       nir->info.outputs_written & BITFIELD_BIT(VARYING_SLOT_PSIZ))
      return false;

   /* Otherwise, IDVS is usually better */
   return true;
}

void
bifrost_compile_shader_nir(nir_shader *nir,
                           const struct panfrost_compile_inputs *inputs,
                           struct util_dynarray *binary,
                           struct pan_shader_info *info)
{
   bifrost_debug = debug_get_option_bifrost_debug();

   /* Combine stores late, to give the driver a chance to lower dual-source
    * blending as regular store_output intrinsics.
    */
   NIR_PASS_V(nir, pan_nir_lower_zs_store);

   bi_optimize_nir(nir, inputs->gpu_id, inputs->is_blend);

   info->tls_size = nir->scratch_size;
   info->vs.idvs = bi_should_idvs(nir, inputs);

   pan_nir_collect_varyings(nir, info);

   if (info->vs.idvs) {
      bi_compile_variant(nir, inputs, binary, info, BI_IDVS_POSITION);
      bi_compile_variant(nir, inputs, binary, info, BI_IDVS_VARYING);
   } else {
      bi_compile_variant(nir, inputs, binary, info, BI_IDVS_NONE);
   }

   if (gl_shader_stage_is_compute(nir->info.stage)) {
      /* Workgroups may be merged if the structure of the workgroup is
       * not software visible. This is true if neither shared memory
       * nor barriers are used. The hardware may be able to optimize
       * compute shaders that set this flag.
       */
      info->cs.allow_merging_workgroups = (nir->info.shared_size == 0) &&
                                          !nir->info.uses_control_barrier &&
                                          !nir->info.uses_memory_barrier;
   }

   info->ubo_mask &= (1 << nir->info.num_ubos) - 1;
}
