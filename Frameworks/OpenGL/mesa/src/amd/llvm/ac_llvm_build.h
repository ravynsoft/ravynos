/*
 * Copyright 2016 Bas Nieuwenhuizen
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef AC_LLVM_BUILD_H
#define AC_LLVM_BUILD_H

#include "ac_llvm_util.h"
#include "ac_shader_abi.h"
#include "ac_shader_args.h"
#include "ac_shader_util.h"
#include "amd_family.h"
#include "compiler/nir/nir.h"
#include <llvm-c/Core.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum
{
   AC_ADDR_SPACE_FLAT = 0, /* Slower than global. */
   AC_ADDR_SPACE_GLOBAL = 1,
   AC_ADDR_SPACE_GDS = 2,
   AC_ADDR_SPACE_LDS = 3,
   AC_ADDR_SPACE_CONST = 4,       /* Global allowing SMEM. */
   AC_ADDR_SPACE_CONST_32BIT = 6, /* same as CONST, but the pointer type has 32 bits */
};

#define AC_WAIT_LGKM   (1 << 0) /* LDS, GDS, constant, message */
#define AC_WAIT_VLOAD  (1 << 1) /* VMEM load/sample instructions */
#define AC_WAIT_VSTORE (1 << 2) /* VMEM store instructions */
#define AC_WAIT_EXP    (1 << 3) /* EXP instructions */

struct ac_llvm_flow;
struct ac_llvm_compiler;
struct radeon_info;

struct ac_llvm_flow_state {
   struct ac_llvm_flow *stack;
   unsigned depth_max;
   unsigned depth;
};

struct ac_llvm_pointer {
   union {
      LLVMValueRef value;
      LLVMValueRef v;
   };
   /* Doesn't support complex types (pointer to pointer to etc...),
    * but this isn't a problem since there's no place where this
    * would be required.
    */
   union {
      LLVMTypeRef pointee_type;
      LLVMTypeRef t;
   };
};

struct ac_llvm_context {
   LLVMContextRef context;
   LLVMModuleRef module;
   LLVMBuilderRef builder;

   struct ac_llvm_pointer main_function;

   LLVMTypeRef voidt;
   LLVMTypeRef i1;
   LLVMTypeRef i8;
   LLVMTypeRef i16;
   LLVMTypeRef i32;
   LLVMTypeRef i64;
   LLVMTypeRef i128;
   LLVMTypeRef intptr;
   LLVMTypeRef f16;
   LLVMTypeRef f32;
   LLVMTypeRef f64;
   LLVMTypeRef v4i8;
   LLVMTypeRef v2i16;
   LLVMTypeRef v4i16;
   LLVMTypeRef v2f16;
   LLVMTypeRef v4f16;
   LLVMTypeRef v2i32;
   LLVMTypeRef v3i32;
   LLVMTypeRef v4i32;
   LLVMTypeRef v2f32;
   LLVMTypeRef v3f32;
   LLVMTypeRef v4f32;
   LLVMTypeRef v8i32;
   LLVMTypeRef iN_wavemask;
   LLVMTypeRef iN_ballotmask;

   LLVMValueRef i8_0;
   LLVMValueRef i8_1;
   LLVMValueRef i16_0;
   LLVMValueRef i16_1;
   LLVMValueRef i32_0;
   LLVMValueRef i32_1;
   LLVMValueRef i64_0;
   LLVMValueRef i64_1;
   LLVMValueRef i128_0;
   LLVMValueRef i128_1;
   LLVMValueRef f16_0;
   LLVMValueRef f16_1;
   LLVMValueRef f32_0;
   LLVMValueRef f32_1;
   LLVMValueRef f64_0;
   LLVMValueRef f64_1;
   LLVMValueRef i1true;
   LLVMValueRef i1false;

   /* Since ac_nir_translate makes a local copy of ac_llvm_context, there
    * are two ac_llvm_contexts. Declare a pointer here, so that the control
    * flow stack is shared by both ac_llvm_contexts.
    */
   struct ac_llvm_flow_state *flow;

   unsigned range_md_kind;
   unsigned invariant_load_md_kind;
   unsigned uniform_md_kind;
   unsigned fpmath_md_kind;
   LLVMValueRef empty_md;
   LLVMValueRef three_md;

   const struct radeon_info *info;
   enum amd_gfx_level gfx_level;

   unsigned wave_size;
   unsigned ballot_mask_bits;

   unsigned float_mode;

   bool exports_color_null;
   bool exports_mrtz;

   struct ac_llvm_pointer lds;

   LLVMValueRef ring_offsets;
   int ring_offsets_index;
};

void ac_llvm_context_init(struct ac_llvm_context *ctx, struct ac_llvm_compiler *compiler,
                          const struct radeon_info *info, enum ac_float_mode float_mode,
                          unsigned wave_size, unsigned ballot_mask_bits, bool exports_color_null,
                          bool exports_mrtz);

void ac_llvm_context_dispose(struct ac_llvm_context *ctx);

int ac_get_llvm_num_components(LLVMValueRef value);

int ac_get_elem_bits(struct ac_llvm_context *ctx, LLVMTypeRef type);

LLVMValueRef ac_llvm_extract_elem(struct ac_llvm_context *ac, LLVMValueRef value, int index);

unsigned ac_get_type_size(LLVMTypeRef type);

LLVMTypeRef ac_to_integer_type(struct ac_llvm_context *ctx, LLVMTypeRef t);
LLVMValueRef ac_to_integer(struct ac_llvm_context *ctx, LLVMValueRef v);
LLVMValueRef ac_to_integer_or_pointer(struct ac_llvm_context *ctx, LLVMValueRef v);
LLVMTypeRef ac_to_float_type(struct ac_llvm_context *ctx, LLVMTypeRef t);
LLVMValueRef ac_to_float(struct ac_llvm_context *ctx, LLVMValueRef v);

LLVMValueRef ac_build_intrinsic(struct ac_llvm_context *ctx, const char *name,
                                LLVMTypeRef return_type, LLVMValueRef *params, unsigned param_count,
                                unsigned attrib_mask);

void ac_build_type_name_for_intr(LLVMTypeRef type, char *buf, unsigned bufsize);

LLVMValueRef ac_build_phi(struct ac_llvm_context *ctx, LLVMTypeRef type, unsigned count_incoming,
                          LLVMValueRef *values, LLVMBasicBlockRef *blocks);

void ac_build_s_barrier(struct ac_llvm_context *ctx, gl_shader_stage stage);
void ac_build_optimization_barrier(struct ac_llvm_context *ctx, LLVMValueRef *pgpr, bool sgpr);

LLVMValueRef ac_build_shader_clock(struct ac_llvm_context *ctx, mesa_scope scope);

LLVMValueRef ac_build_ballot(struct ac_llvm_context *ctx, LLVMValueRef value);
LLVMValueRef ac_get_i1_sgpr_mask(struct ac_llvm_context *ctx, LLVMValueRef value);

LLVMValueRef ac_build_vote_all(struct ac_llvm_context *ctx, LLVMValueRef value);

LLVMValueRef ac_build_vote_any(struct ac_llvm_context *ctx, LLVMValueRef value);

LLVMValueRef ac_build_vote_eq(struct ac_llvm_context *ctx, LLVMValueRef value);

LLVMValueRef ac_build_varying_gather_values(struct ac_llvm_context *ctx, LLVMValueRef *values,
                                            unsigned value_count, unsigned component);

LLVMValueRef ac_build_gather_values_extended(struct ac_llvm_context *ctx, LLVMValueRef *values,
                                             unsigned value_count, unsigned value_stride,
                                             bool always_vector);
LLVMValueRef ac_build_gather_values(struct ac_llvm_context *ctx, LLVMValueRef *values,
                                    unsigned value_count);

LLVMValueRef ac_build_concat(struct ac_llvm_context *ctx, LLVMValueRef a, LLVMValueRef b);

LLVMValueRef ac_extract_components(struct ac_llvm_context *ctx, LLVMValueRef value, unsigned start,
                                   unsigned channels);

LLVMValueRef ac_build_expand(struct ac_llvm_context *ctx, LLVMValueRef value,
                             unsigned src_channels, unsigned dst_channels);

LLVMValueRef ac_build_expand_to_vec4(struct ac_llvm_context *ctx, LLVMValueRef value,
                                     unsigned num_channels);
LLVMValueRef ac_build_round(struct ac_llvm_context *ctx, LLVMValueRef value);

LLVMValueRef ac_build_fdiv(struct ac_llvm_context *ctx, LLVMValueRef num, LLVMValueRef den);

LLVMValueRef ac_build_fast_udiv(struct ac_llvm_context *ctx, LLVMValueRef num,
                                LLVMValueRef multiplier, LLVMValueRef pre_shift,
                                LLVMValueRef post_shift, LLVMValueRef increment);
LLVMValueRef ac_build_fast_udiv_nuw(struct ac_llvm_context *ctx, LLVMValueRef num,
                                    LLVMValueRef multiplier, LLVMValueRef pre_shift,
                                    LLVMValueRef post_shift, LLVMValueRef increment);
LLVMValueRef ac_build_fast_udiv_u31_d_not_one(struct ac_llvm_context *ctx, LLVMValueRef num,
                                              LLVMValueRef multiplier, LLVMValueRef post_shift);

LLVMValueRef ac_build_fs_interp(struct ac_llvm_context *ctx, LLVMValueRef llvm_chan,
                                LLVMValueRef attr_number, LLVMValueRef params, LLVMValueRef i,
                                LLVMValueRef j);

LLVMValueRef ac_build_fs_interp_f16(struct ac_llvm_context *ctx, LLVMValueRef llvm_chan,
                                    LLVMValueRef attr_number, LLVMValueRef params, LLVMValueRef i,
                                    LLVMValueRef j, bool high_16bits);

LLVMValueRef ac_build_fs_interp_mov(struct ac_llvm_context *ctx, unsigned parameter,
                                    LLVMValueRef llvm_chan, LLVMValueRef attr_number,
                                    LLVMValueRef params);

LLVMValueRef ac_build_gep_ptr(struct ac_llvm_context *ctx, LLVMTypeRef type, LLVMValueRef base_ptr,
                              LLVMValueRef index);

LLVMValueRef ac_build_pointer_add(struct ac_llvm_context *ctx, LLVMTypeRef type, LLVMValueRef ptr,
                                  LLVMValueRef index);

LLVMTypeRef ac_build_gep0_type(LLVMTypeRef pointee_type, LLVMValueRef index);
LLVMValueRef ac_build_gep0(struct ac_llvm_context *ctx, struct ac_llvm_pointer ptr, LLVMValueRef index);

void ac_build_indexed_store(struct ac_llvm_context *ctx, struct ac_llvm_pointer ptr, LLVMValueRef index,
                            LLVMValueRef value);

LLVMValueRef ac_build_load(struct ac_llvm_context *ctx, struct ac_llvm_pointer ptr, LLVMValueRef index);
LLVMValueRef ac_build_load_invariant(struct ac_llvm_context *ctx, struct ac_llvm_pointer ptr,
                                     LLVMValueRef index);
LLVMValueRef ac_build_load_to_sgpr(struct ac_llvm_context *ctx, struct ac_llvm_pointer ptr,
                                   LLVMValueRef index);

LLVMValueRef ac_build_load_to_sgpr_uint_wraparound(struct ac_llvm_context *ctx, struct ac_llvm_pointer ptr,
                                                   LLVMValueRef index);

void ac_build_buffer_store_dword(struct ac_llvm_context *ctx, LLVMValueRef rsrc, LLVMValueRef vdata,
                                 LLVMValueRef vindex, LLVMValueRef voffset, LLVMValueRef soffset,
                                 enum gl_access_qualifier access);

void ac_build_buffer_store_format(struct ac_llvm_context *ctx, LLVMValueRef rsrc, LLVMValueRef data,
                                  LLVMValueRef vindex, LLVMValueRef voffset, enum gl_access_qualifier access);

LLVMValueRef ac_build_buffer_load(struct ac_llvm_context *ctx, LLVMValueRef rsrc, int num_channels,
                                  LLVMValueRef vindex, LLVMValueRef voffset, LLVMValueRef soffset,
                                  LLVMTypeRef channel_type, enum gl_access_qualifier access,
                                  bool can_speculate, bool allow_smem);

LLVMValueRef ac_build_buffer_load_format(struct ac_llvm_context *ctx, LLVMValueRef rsrc,
                                         LLVMValueRef vindex, LLVMValueRef voffset,
                                         unsigned num_channels, enum gl_access_qualifier access,
                                         bool can_speculate, bool d16, bool tfe);

LLVMValueRef ac_build_buffer_load_short(struct ac_llvm_context *ctx, LLVMValueRef rsrc,
                                        LLVMValueRef voffset, LLVMValueRef soffset,
                                        enum gl_access_qualifier access);

LLVMValueRef ac_build_buffer_load_byte(struct ac_llvm_context *ctx, LLVMValueRef rsrc,
                                       LLVMValueRef voffset, LLVMValueRef soffset,
                                       enum gl_access_qualifier access);

LLVMValueRef ac_build_safe_tbuffer_load(struct ac_llvm_context *ctx, LLVMValueRef rsrc,
                                        LLVMValueRef vindex, LLVMValueRef voffset,
                                        LLVMValueRef soffset,
                                        const enum pipe_format format,
                                        unsigned channel_bit_size,
                                        unsigned const_offset,
                                        unsigned align_offset,
                                        unsigned align_mul,
                                        unsigned num_channels,
                                        enum gl_access_qualifier access,
                                        bool can_speculate);

void ac_build_buffer_store_short(struct ac_llvm_context *ctx, LLVMValueRef rsrc,
                                 LLVMValueRef vdata, LLVMValueRef voffset, LLVMValueRef soffset,
                                 enum gl_access_qualifier access);

void ac_build_buffer_store_byte(struct ac_llvm_context *ctx, LLVMValueRef rsrc, LLVMValueRef vdata,
                                LLVMValueRef voffset, LLVMValueRef soffset, enum gl_access_qualifier access);

void ac_set_range_metadata(struct ac_llvm_context *ctx, LLVMValueRef value, unsigned lo,
                           unsigned hi);
LLVMValueRef ac_get_thread_id(struct ac_llvm_context *ctx);

#define AC_TID_MASK_TOP_LEFT 0xfffffffc
#define AC_TID_MASK_TOP      0xfffffffd
#define AC_TID_MASK_LEFT     0xfffffffe

LLVMValueRef ac_build_ddxy(struct ac_llvm_context *ctx, uint32_t mask, int idx, LLVMValueRef val);

void ac_build_sendmsg(struct ac_llvm_context *ctx, uint32_t imm, LLVMValueRef m0_content);

LLVMValueRef ac_build_imsb(struct ac_llvm_context *ctx, LLVMValueRef arg, LLVMTypeRef dst_type);

LLVMValueRef ac_build_umsb(struct ac_llvm_context *ctx, LLVMValueRef arg, LLVMTypeRef dst_type,
                           bool rev);
LLVMValueRef ac_build_fmin(struct ac_llvm_context *ctx, LLVMValueRef a, LLVMValueRef b);
LLVMValueRef ac_build_fmax(struct ac_llvm_context *ctx, LLVMValueRef a, LLVMValueRef b);
LLVMValueRef ac_build_imin(struct ac_llvm_context *ctx, LLVMValueRef a, LLVMValueRef b);
LLVMValueRef ac_build_imax(struct ac_llvm_context *ctx, LLVMValueRef a, LLVMValueRef b);
LLVMValueRef ac_build_umin(struct ac_llvm_context *ctx, LLVMValueRef a, LLVMValueRef b);
LLVMValueRef ac_build_umax(struct ac_llvm_context *ctx, LLVMValueRef a, LLVMValueRef b);
LLVMValueRef ac_build_clamp(struct ac_llvm_context *ctx, LLVMValueRef value);

struct ac_export_args {
   LLVMValueRef out[4];
   unsigned target;
   unsigned enabled_channels;
   bool compr;
   bool done;
   bool valid_mask;
};

void ac_build_export(struct ac_llvm_context *ctx, struct ac_export_args *a);

void ac_build_export_null(struct ac_llvm_context *ctx, bool uses_discard);

enum ac_image_opcode
{
   ac_image_sample,
   ac_image_gather4,
   ac_image_load,
   ac_image_load_mip,
   ac_image_store,
   ac_image_store_mip,
   ac_image_get_lod,
   ac_image_get_resinfo,
   ac_image_atomic,
   ac_image_atomic_cmpswap,
};

enum ac_atomic_op
{
   ac_atomic_swap,
   ac_atomic_add,
   ac_atomic_sub,
   ac_atomic_smin,
   ac_atomic_umin,
   ac_atomic_smax,
   ac_atomic_umax,
   ac_atomic_and,
   ac_atomic_or,
   ac_atomic_xor,
   ac_atomic_inc_wrap,
   ac_atomic_dec_wrap,
   ac_atomic_fmin,
   ac_atomic_fmax,
};

struct ac_image_args {
   enum ac_image_opcode opcode;
   enum ac_atomic_op atomic; /* for the ac_image_atomic opcode */
   enum ac_image_dim dim;
   enum gl_access_qualifier access;
   unsigned dmask : 4;
   bool unorm : 1;
   bool level_zero : 1;
   bool d16 : 1;        /* GFX8+: data and return values are 16-bit */
   bool a16 : 1;        /* GFX9+: address components except compare, offset and bias are 16-bit */
   bool g16 : 1;        /* GFX10+: derivatives are 16-bit; GFX<=9: must be equal to a16 */
   bool tfe : 1;
   unsigned attributes; /* additional call-site specific AC_FUNC_ATTRs */

   LLVMValueRef resource;
   LLVMValueRef sampler;
   LLVMValueRef data[2]; /* data[0] is source data (vector); data[1] is cmp for cmpswap */
   LLVMValueRef offset;
   LLVMValueRef bias;
   LLVMValueRef compare;
   LLVMValueRef derivs[6];
   LLVMValueRef coords[4];
   LLVMValueRef lod; // also used by ac_image_get_resinfo
   LLVMValueRef min_lod;
};

LLVMValueRef ac_build_image_opcode(struct ac_llvm_context *ctx, struct ac_image_args *a);
LLVMValueRef ac_build_image_get_sample_count(struct ac_llvm_context *ctx, LLVMValueRef rsrc);
LLVMValueRef ac_build_cvt_pkrtz_f16(struct ac_llvm_context *ctx, LLVMValueRef args[2]);
LLVMValueRef ac_build_cvt_pknorm_i16(struct ac_llvm_context *ctx, LLVMValueRef args[2]);
LLVMValueRef ac_build_cvt_pknorm_u16(struct ac_llvm_context *ctx, LLVMValueRef args[2]);
LLVMValueRef ac_build_cvt_pknorm_i16_f16(struct ac_llvm_context *ctx, LLVMValueRef args[2]);
LLVMValueRef ac_build_cvt_pknorm_u16_f16(struct ac_llvm_context *ctx, LLVMValueRef args[2]);
LLVMValueRef ac_build_cvt_pk_i16(struct ac_llvm_context *ctx, LLVMValueRef args[2], unsigned bits,
                                 bool hi);
LLVMValueRef ac_build_cvt_pk_u16(struct ac_llvm_context *ctx, LLVMValueRef args[2], unsigned bits,
                                 bool hi);
LLVMValueRef ac_build_wqm_vote(struct ac_llvm_context *ctx, LLVMValueRef i1);
void ac_build_kill_if_false(struct ac_llvm_context *ctx, LLVMValueRef i1);
LLVMValueRef ac_build_bfe(struct ac_llvm_context *ctx, LLVMValueRef input, LLVMValueRef offset,
                          LLVMValueRef width, bool is_signed);
LLVMValueRef ac_build_imad(struct ac_llvm_context *ctx, LLVMValueRef s0, LLVMValueRef s1,
                           LLVMValueRef s2);
LLVMValueRef ac_build_fmad(struct ac_llvm_context *ctx, LLVMValueRef s0, LLVMValueRef s1,
                           LLVMValueRef s2);

void ac_build_waitcnt(struct ac_llvm_context *ctx, unsigned wait_flags);

LLVMValueRef ac_build_fract(struct ac_llvm_context *ctx, LLVMValueRef src0, unsigned bitsize);
LLVMValueRef ac_const_uint_vec(struct ac_llvm_context *ctx, LLVMTypeRef type, uint64_t value);
LLVMValueRef ac_build_isign(struct ac_llvm_context *ctx, LLVMValueRef src0);
LLVMValueRef ac_build_fsign(struct ac_llvm_context *ctx, LLVMValueRef src);
LLVMValueRef ac_build_bit_count(struct ac_llvm_context *ctx, LLVMValueRef src0);

LLVMValueRef ac_build_fsat(struct ac_llvm_context *ctx, LLVMValueRef src,
                           LLVMTypeRef type);

LLVMValueRef ac_build_bitfield_reverse(struct ac_llvm_context *ctx, LLVMValueRef src0);

LLVMValueRef ac_build_sudot_4x8(struct ac_llvm_context *ctx, LLVMValueRef s0, LLVMValueRef s1,
                                LLVMValueRef s2, bool clamp, unsigned neg_lo);

void ac_init_exec_full_mask(struct ac_llvm_context *ctx);

void ac_declare_lds_as_pointer(struct ac_llvm_context *ac);
LLVMValueRef ac_lds_load(struct ac_llvm_context *ctx, LLVMValueRef dw_addr);
void ac_lds_store(struct ac_llvm_context *ctx, LLVMValueRef dw_addr, LLVMValueRef value);

LLVMValueRef ac_find_lsb(struct ac_llvm_context *ctx, LLVMTypeRef dst_type, LLVMValueRef src0);

LLVMTypeRef ac_array_in_const_addr_space(LLVMTypeRef elem_type);
LLVMTypeRef ac_array_in_const32_addr_space(LLVMTypeRef elem_type);

void ac_build_bgnloop(struct ac_llvm_context *ctx, int lable_id);
void ac_build_break(struct ac_llvm_context *ctx);
void ac_build_continue(struct ac_llvm_context *ctx);
void ac_build_else(struct ac_llvm_context *ctx, int lable_id);
void ac_build_endif(struct ac_llvm_context *ctx, int lable_id);
void ac_build_endloop(struct ac_llvm_context *ctx, int lable_id);
void ac_build_ifcc(struct ac_llvm_context *ctx, LLVMValueRef cond, int label_id);

LLVMValueRef ac_build_alloca(struct ac_llvm_context *ac, LLVMTypeRef type, const char *name);
LLVMValueRef ac_build_alloca_undef(struct ac_llvm_context *ac, LLVMTypeRef type, const char *name);
LLVMValueRef ac_build_alloca_init(struct ac_llvm_context *ac, LLVMValueRef val, const char *name);

LLVMValueRef ac_cast_ptr(struct ac_llvm_context *ctx, LLVMValueRef ptr, LLVMTypeRef type);

LLVMValueRef ac_trim_vector(struct ac_llvm_context *ctx, LLVMValueRef value, unsigned count);

LLVMValueRef ac_unpack_param(struct ac_llvm_context *ctx, LLVMValueRef param, unsigned rshift,
                             unsigned bitwidth);

LLVMValueRef ac_build_ds_swizzle(struct ac_llvm_context *ctx, LLVMValueRef src, unsigned mask);

LLVMValueRef ac_build_readlane_no_opt_barrier(struct ac_llvm_context *ctx, LLVMValueRef src,
                                              LLVMValueRef lane);

LLVMValueRef ac_build_readlane(struct ac_llvm_context *ctx, LLVMValueRef src, LLVMValueRef lane);

LLVMValueRef ac_build_writelane(struct ac_llvm_context *ctx, LLVMValueRef src, LLVMValueRef value,
                                LLVMValueRef lane);

LLVMValueRef ac_build_mbcnt_add(struct ac_llvm_context *ctx, LLVMValueRef mask, LLVMValueRef add_src);
LLVMValueRef ac_build_mbcnt(struct ac_llvm_context *ctx, LLVMValueRef mask);

LLVMValueRef ac_build_wqm(struct ac_llvm_context *ctx, LLVMValueRef src);

LLVMValueRef ac_build_inclusive_scan(struct ac_llvm_context *ctx, LLVMValueRef src, nir_op op);

LLVMValueRef ac_build_exclusive_scan(struct ac_llvm_context *ctx, LLVMValueRef src, nir_op op);

LLVMValueRef ac_build_reduce(struct ac_llvm_context *ctx, LLVMValueRef src, nir_op op,
                             unsigned cluster_size);

LLVMValueRef ac_build_quad_swizzle(struct ac_llvm_context *ctx, LLVMValueRef src, unsigned lane0,
                                   unsigned lane1, unsigned lane2, unsigned lane3);

LLVMValueRef ac_build_shuffle(struct ac_llvm_context *ctx, LLVMValueRef src, LLVMValueRef index);

LLVMValueRef ac_build_frexp_exp(struct ac_llvm_context *ctx, LLVMValueRef src0, unsigned bitsize);

LLVMValueRef ac_build_frexp_mant(struct ac_llvm_context *ctx, LLVMValueRef src0, unsigned bitsize);

LLVMValueRef ac_build_canonicalize(struct ac_llvm_context *ctx, LLVMValueRef src0,
                                   unsigned bitsize);

LLVMValueRef ac_build_ddxy_interp(struct ac_llvm_context *ctx, LLVMValueRef interp_ij);

LLVMValueRef ac_build_load_helper_invocation(struct ac_llvm_context *ctx);

LLVMValueRef ac_build_call(struct ac_llvm_context *ctx, LLVMTypeRef fn_type, LLVMValueRef func,
                           LLVMValueRef *args, unsigned num_args);

LLVMValueRef ac_build_atomic_rmw(struct ac_llvm_context *ctx, LLVMAtomicRMWBinOp op,
                                 LLVMValueRef ptr, LLVMValueRef val, const char *sync_scope);

LLVMValueRef ac_build_atomic_cmp_xchg(struct ac_llvm_context *ctx, LLVMValueRef ptr,
                                      LLVMValueRef cmp, LLVMValueRef val, const char *sync_scope);

void ac_export_mrt_z(struct ac_llvm_context *ctx, LLVMValueRef depth, LLVMValueRef stencil,
                     LLVMValueRef samplemask, LLVMValueRef mrt0_alpha, bool is_last,
                     struct ac_export_args *args);

struct ac_ngg_prim {
   unsigned num_vertices;
   LLVMValueRef isnull;
   LLVMValueRef index[3];
   LLVMValueRef edgeflags;
   LLVMValueRef passthrough;
};

LLVMTypeRef ac_arg_type_to_pointee_type(struct ac_llvm_context *ctx, enum ac_arg_type type);

static inline LLVMValueRef ac_get_arg(struct ac_llvm_context *ctx, struct ac_arg arg)
{
   assert(arg.used);
   if (arg.arg_index == ctx->ring_offsets_index)
      return ctx->ring_offsets;
   int offset = arg.arg_index > ctx->ring_offsets_index ? -1 : 0;
   return LLVMGetParam(ctx->main_function.value, arg.arg_index + offset);
}

static inline struct ac_llvm_pointer
ac_get_ptr_arg(struct ac_llvm_context *ctx, const struct ac_shader_args *args, struct ac_arg arg)
{
   struct ac_llvm_pointer ptr;
   ptr.pointee_type = ac_arg_type_to_pointee_type(ctx, args->args[arg.arg_index].type);
   ptr.value = ac_get_arg(ctx, arg);
   return ptr;
}

enum ac_llvm_calling_convention
{
   AC_LLVM_AMDGPU_VS = 87,
   AC_LLVM_AMDGPU_GS = 88,
   AC_LLVM_AMDGPU_PS = 89,
   AC_LLVM_AMDGPU_CS = 90,
   AC_LLVM_AMDGPU_HS = 93,
};

struct ac_llvm_pointer ac_build_main(const struct ac_shader_args *args, struct ac_llvm_context *ctx,
                                     enum ac_llvm_calling_convention convention, const char *name,
                                     LLVMTypeRef ret_type, LLVMModuleRef module);
void ac_build_s_endpgm(struct ac_llvm_context *ctx);

LLVMValueRef ac_build_is_inf_or_nan(struct ac_llvm_context *ctx, LLVMValueRef a);

void ac_build_dual_src_blend_swizzle(struct ac_llvm_context *ctx,
                                     struct ac_export_args *mrt0,
                                     struct ac_export_args *mrt1);

#ifdef __cplusplus
}
#endif

#endif
