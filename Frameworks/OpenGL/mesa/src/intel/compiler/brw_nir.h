/*
 * Copyright © 2015 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef BRW_NIR_H
#define BRW_NIR_H

#include "brw_reg.h"
#include "compiler/nir/nir.h"
#include "brw_compiler.h"
#include "nir_builder.h"

#ifdef __cplusplus
extern "C" {
#endif

int type_size_vec4(const struct glsl_type *type, bool bindless);
int type_size_dvec4(const struct glsl_type *type, bool bindless);

static inline int
type_size_scalar_bytes(const struct glsl_type *type, bool bindless)
{
   return glsl_count_dword_slots(type, bindless) * 4;
}

static inline int
type_size_vec4_bytes(const struct glsl_type *type, bool bindless)
{
   return type_size_vec4(type, bindless) * 16;
}

/* Flags set in the instr->pass_flags field by i965 analysis passes */
enum {
   BRW_NIR_NON_BOOLEAN           = 0x0,

   /* Indicates that the given instruction's destination is a boolean
    * value but that it needs to be resolved before it can be used.
    * On Gen <= 5, CMP instructions return a 32-bit value where the bottom
    * bit represents the actual true/false value of the compare and the top
    * 31 bits are undefined.  In order to use this value, we have to do a
    * "resolve" operation by replacing the value of the CMP with -(x & 1)
    * to sign-extend the bottom bit to 0/~0.
    */
   BRW_NIR_BOOLEAN_NEEDS_RESOLVE = 0x1,

   /* Indicates that the given instruction's destination is a boolean
    * value that has intentionally been left unresolved.  Not all boolean
    * values need to be resolved immediately.  For instance, if we have
    *
    *    CMP r1 r2 r3
    *    CMP r4 r5 r6
    *    AND r7 r1 r4
    *
    * We don't have to resolve the result of the two CMP instructions
    * immediately because the AND still does an AND of the bottom bits.
    * Instead, we can save ourselves instructions by delaying the resolve
    * until after the AND.  The result of the two CMP instructions is left
    * as BRW_NIR_BOOLEAN_UNRESOLVED.
    */
   BRW_NIR_BOOLEAN_UNRESOLVED    = 0x2,

   /* Indicates a that the given instruction's destination is a boolean
    * value that does not need a resolve.  For instance, if you AND two
    * values that are BRW_NIR_BOOLEAN_NEEDS_RESOLVE then we know that both
    * values will be 0/~0 before we get them and the result of the AND is
    * also guaranteed to be 0/~0 and does not need a resolve.
    */
   BRW_NIR_BOOLEAN_NO_RESOLVE    = 0x3,

   /* A mask to mask the boolean status values off of instr->pass_flags */
   BRW_NIR_BOOLEAN_MASK          = 0x3,
};

void brw_nir_analyze_boolean_resolves(nir_shader *nir);

struct brw_nir_compiler_opts {
   /* Soft floating point implementation shader */
   const nir_shader *softfp64;

   /* Whether robust image access is enabled */
   bool robust_image_access;

   /* Input vertices for TCS stage (0 means dynamic) */
   unsigned input_vertices;
};

/* UBO surface index can come in 2 flavors :
 *    - nir_intrinsic_resource_intel
 *    - anything else
 *
 * In the first case, checking that the surface index is const requires
 * checking resource_intel::src[1]. In any other case it's a simple
 * nir_src_is_const().
 *
 * This function should only be called on src[0] of load_ubo intrinsics.
 */
static inline bool
brw_nir_ubo_surface_index_is_pushable(nir_src src)
{
   nir_intrinsic_instr *intrin =
      src.ssa->parent_instr->type == nir_instr_type_intrinsic ?
      nir_instr_as_intrinsic(src.ssa->parent_instr) : NULL;

   if (intrin && intrin->intrinsic == nir_intrinsic_resource_intel) {
      return (nir_intrinsic_resource_access_intel(intrin) &
              nir_resource_intel_pushable);
   }

   return nir_src_is_const(src);
}

static inline unsigned
brw_nir_ubo_surface_index_get_push_block(nir_src src)
{
   if (nir_src_is_const(src))
      return nir_src_as_uint(src);

   if (!brw_nir_ubo_surface_index_is_pushable(src))
      return UINT32_MAX;

   assert(src.ssa->parent_instr->type == nir_instr_type_intrinsic);

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(src.ssa->parent_instr);
   assert(intrin->intrinsic == nir_intrinsic_resource_intel);

   return nir_intrinsic_resource_block_intel(intrin);
}

/* This helper return the binding table index of a surface access (any
 * buffer/image/etc...). It works off the source of one of the intrinsics
 * (load_ubo, load_ssbo, store_ssbo, load_image, store_image, etc...).
 *
 * If the source is constant, then this is the binding table index. If we're
 * going through a resource_intel intel intrinsic, then we need to check
 * src[1] of that intrinsic.
 */
static inline unsigned
brw_nir_ubo_surface_index_get_bti(nir_src src)
{
   if (nir_src_is_const(src))
      return nir_src_as_uint(src);

   assert(src.ssa->parent_instr->type == nir_instr_type_intrinsic);

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(src.ssa->parent_instr);
   if (!intrin || intrin->intrinsic != nir_intrinsic_resource_intel)
      return UINT32_MAX;

   /* In practice we could even drop this intrinsic because the bindless
    * access always operate from a base offset coming from a push constant, so
    * they can never be constant.
    */
   if (nir_intrinsic_resource_access_intel(intrin) &
       nir_resource_intel_bindless)
      return UINT32_MAX;

   if (!nir_src_is_const(intrin->src[1]))
      return UINT32_MAX;

   return nir_src_as_uint(intrin->src[1]);
}

void brw_preprocess_nir(const struct brw_compiler *compiler,
                        nir_shader *nir,
                        const struct brw_nir_compiler_opts *opts);

void
brw_nir_link_shaders(const struct brw_compiler *compiler,
                     nir_shader *producer, nir_shader *consumer);

bool brw_nir_lower_cs_intrinsics(nir_shader *nir);
bool brw_nir_lower_alpha_to_coverage(nir_shader *shader,
                                     const struct brw_wm_prog_key *key,
                                     const struct brw_wm_prog_data *prog_data);
void brw_nir_lower_vs_inputs(nir_shader *nir,
                             bool edgeflag_is_last,
                             const uint8_t *vs_attrib_wa_flags);
void brw_nir_lower_vue_inputs(nir_shader *nir,
                              const struct brw_vue_map *vue_map);
void brw_nir_lower_tes_inputs(nir_shader *nir, const struct brw_vue_map *vue);
void brw_nir_lower_fs_inputs(nir_shader *nir,
                             const struct intel_device_info *devinfo,
                             const struct brw_wm_prog_key *key);
void brw_nir_lower_vue_outputs(nir_shader *nir);
void brw_nir_lower_tcs_outputs(nir_shader *nir, const struct brw_vue_map *vue,
                               enum tess_primitive_mode tes_primitive_mode);
void brw_nir_lower_fs_outputs(nir_shader *nir);

bool brw_nir_lower_conversions(nir_shader *nir);

bool brw_nir_lower_cmat(nir_shader *nir, unsigned subgroup_size);

bool brw_nir_lower_shading_rate_output(nir_shader *nir);

bool brw_nir_lower_sparse_intrinsics(nir_shader *nir);

struct brw_nir_lower_storage_image_opts {
   const struct intel_device_info *devinfo;

   bool lower_loads;
   bool lower_stores;
   bool lower_atomics;
   bool lower_get_size;
};

bool brw_nir_lower_storage_image(nir_shader *nir,
                                 const struct brw_nir_lower_storage_image_opts *opts);

bool brw_nir_lower_mem_access_bit_sizes(nir_shader *shader,
                                        const struct
                                        intel_device_info *devinfo);

bool brw_nir_lower_non_uniform_resource_intel(nir_shader *shader);

bool brw_nir_cleanup_resource_intel(nir_shader *shader);

void brw_postprocess_nir(nir_shader *nir,
                         const struct brw_compiler *compiler,
                         bool debug_enabled,
                         enum brw_robustness_flags robust_flags);

bool brw_nir_clamp_image_1d_2d_array_sizes(nir_shader *shader);

bool brw_nir_apply_attribute_workarounds(nir_shader *nir,
                                         const uint8_t *attrib_wa_flags);

bool brw_nir_apply_trig_workarounds(nir_shader *nir);

bool brw_nir_limit_trig_input_range_workaround(nir_shader *nir);

void brw_nir_apply_tcs_quads_workaround(nir_shader *nir);

bool brw_nir_lower_non_uniform_barycentric_at_sample(nir_shader *nir);

void brw_nir_apply_key(nir_shader *nir,
                       const struct brw_compiler *compiler,
                       const struct brw_base_prog_key *key,
                       unsigned max_subgroup_size);

unsigned brw_nir_api_subgroup_size(const nir_shader *nir,
                                   unsigned hw_subgroup_size);

enum brw_conditional_mod brw_cmod_for_nir_comparison(nir_op op);
enum lsc_opcode lsc_aop_for_nir_intrinsic(const nir_intrinsic_instr *atomic);
enum brw_reg_type brw_type_for_nir_type(const struct intel_device_info *devinfo,
                                        nir_alu_type type);

bool brw_nir_should_vectorize_mem(unsigned align_mul, unsigned align_offset,
                                  unsigned bit_size,
                                  unsigned num_components,
                                  nir_intrinsic_instr *low,
                                  nir_intrinsic_instr *high,
                                  void *data);

void brw_nir_analyze_ubo_ranges(const struct brw_compiler *compiler,
                                nir_shader *nir,
                                struct brw_ubo_range out_ranges[4]);

bool brw_nir_opt_peephole_ffma(nir_shader *shader);

bool brw_nir_opt_peephole_imul32x16(nir_shader *shader);

bool brw_nir_clamp_per_vertex_loads(nir_shader *shader);

bool brw_nir_lower_patch_vertices_in(nir_shader *shader, unsigned input_vertices);

bool brw_nir_blockify_uniform_loads(nir_shader *shader,
                                    const struct intel_device_info *devinfo);

void brw_nir_optimize(nir_shader *nir, bool is_scalar,
                      const struct intel_device_info *devinfo);

nir_shader *brw_nir_create_passthrough_tcs(void *mem_ctx,
                                           const struct brw_compiler *compiler,
                                           const struct brw_tcs_prog_key *key);

bool brw_nir_pulls_at_sample(nir_shader *shader);

#define BRW_NIR_FRAG_OUTPUT_INDEX_SHIFT 0
#define BRW_NIR_FRAG_OUTPUT_INDEX_MASK INTEL_MASK(0, 0)
#define BRW_NIR_FRAG_OUTPUT_LOCATION_SHIFT 1
#define BRW_NIR_FRAG_OUTPUT_LOCATION_MASK INTEL_MASK(31, 1)

bool brw_nir_move_interpolation_to_top(nir_shader *nir);
nir_def *brw_nir_load_global_const(nir_builder *b,
                                       nir_intrinsic_instr *load_uniform,
                                       nir_def *base_addr,
                                       unsigned off);

const struct glsl_type *brw_nir_get_var_type(const struct nir_shader *nir,
                                             nir_variable *var);

void brw_nir_adjust_payload(nir_shader *shader);

#ifdef __cplusplus
}
#endif

#endif /* BRW_NIR_H */
