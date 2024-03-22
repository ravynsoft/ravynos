#ifndef LP_STATE_SETUP_H
#define LP_STATE_SETUP_H

#include "lp_bld_interp.h"


struct llvmpipe_context;
struct lp_setup_variant;

struct lp_setup_variant_list_item
{
   struct list_head list;
   struct lp_setup_variant *base;
};


struct lp_setup_variant_key {
   unsigned size:16;
   unsigned num_inputs:8;
   int color_slot:8;
   int bcolor_slot:8;
   int spec_slot:8;
   int bspec_slot:8;
   unsigned flatshade_first:1;
   unsigned pixel_center_half:1;
   unsigned twoside:1;
   unsigned floating_point_depth:1;
   unsigned uses_constant_interp:1;
   unsigned multisample:1;
   unsigned pad:3;

   /* TODO: get those floats out of the key and use a jit_context for setup */
   float pgon_offset_units;
   float pgon_offset_scale;
   float pgon_offset_clamp;
   struct lp_shader_input inputs[PIPE_MAX_SHADER_INPUTS];
};


typedef void (*lp_jit_setup_triangle)(const float (*v0)[4],
                                      const float (*v1)[4],
                                      const float (*v2)[4],
                                      bool front_facing,
                                      float (*a0)[4],
                                      float (*dadx)[4],
                                      float (*dady)[4],
                                      const struct lp_setup_variant_key *key);


/* At this stage, for a given variant key, we create a
 * draw_vertex_info struct telling the draw module how to format the
 * vertices, and an llvm-generated function which calculates the
 * attribute interpolants (a0, dadx, dady) from three of those
 * vertices.
 */
struct lp_setup_variant {
   struct lp_setup_variant_key key;

   struct lp_setup_variant_list_item list_item_global;

   struct gallivm_state *gallivm;

   /* XXX: this is a pointer to the LLVM IR.  Once jit_function is
    * generated, we never need to use the IR again - need to find a
    * way to release this data without destroying the generated
    * assembly.
    */
   LLVMValueRef function;

   /* The actual generated setup function:
    */
   lp_jit_setup_triangle jit_function;

   unsigned no;
};


void
lp_delete_setup_variants(struct llvmpipe_context *lp);

void
lp_dump_setup_coef(const struct lp_setup_variant_key *key,
                   const float (*sa0)[4],
                   const float (*sdadx)[4],
                   const float (*sdady)[4]);

#endif
