/**************************************************************************
 *
 * Copyright 2019 Red Hat.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **************************************************************************/

#ifndef LP_STATE_CS_H
#define LP_STATE_CS_H

#include "util/u_thread.h"
#include "pipe/p_state.h"

#include "gallivm/lp_bld.h"
#include "gallivm/lp_bld_sample.h" /* for struct lp_sampler_static_state */
#include "lp_jit.h"
#include "lp_state_fs.h"

struct lp_compute_shader_variant;

struct lp_compute_shader_variant_key
{
   unsigned nr_samplers:8;
   unsigned nr_sampler_views:8;
   unsigned nr_images:8;
};

#define LP_CS_MAX_VARIANT_KEY_SIZE                                      \
   (sizeof(struct lp_compute_shader_variant_key) +                     \
    PIPE_MAX_SHADER_SAMPLER_VIEWS * sizeof(struct lp_sampler_static_state) + \
    PIPE_MAX_SHADER_IMAGES * sizeof(struct lp_image_static_state))

static inline size_t
lp_cs_variant_key_size(unsigned nr_samplers, unsigned nr_images)
{
   return (sizeof(struct lp_compute_shader_variant_key) +
           nr_samplers * sizeof(struct lp_sampler_static_state) +
           nr_images * sizeof(struct lp_image_static_state));
}

static inline struct lp_sampler_static_state *
lp_cs_variant_key_samplers(const struct lp_compute_shader_variant_key *key)
{
   return (struct lp_sampler_static_state *)&(key[1]);
}

static inline struct lp_image_static_state *
lp_cs_variant_key_images(const struct lp_compute_shader_variant_key *key)
{
   return (struct lp_image_static_state *)
      &(lp_cs_variant_key_samplers(key)[MAX2(key->nr_samplers, key->nr_sampler_views)]);
}

struct lp_cs_variant_list_item
{
   struct list_head list;
   struct lp_compute_shader_variant *base;
};

struct lp_compute_shader_variant
{
   struct gallivm_state *gallivm;

   LLVMTypeRef jit_cs_context_type;
   LLVMTypeRef jit_cs_context_ptr_type;
   LLVMTypeRef jit_cs_thread_data_type;
   LLVMTypeRef jit_resources_type;
   LLVMTypeRef jit_resources_ptr_type;
   LLVMTypeRef jit_cs_thread_data_ptr_type;

   /* for mesh shaders */
   LLVMTypeRef jit_vertex_header_type;
   LLVMTypeRef jit_vertex_header_ptr_type;
   LLVMTypeRef jit_prim_type;
   LLVMValueRef function;
   lp_jit_cs_func jit_function;

   /* Total number of LLVM instructions generated */
   unsigned nr_instrs;

   struct lp_cs_variant_list_item list_item_global, list_item_local;

   struct lp_compute_shader *shader;

   /* For debugging/profiling purposes */
   unsigned no;

   /* key is variable-sized, must be last */
   struct lp_compute_shader_variant_key key;
};

struct lp_compute_shader {
   struct pipe_shader_state base;

   struct lp_cs_variant_list_item variants;

   struct draw_mesh_shader *draw_mesh_data;
   uint32_t req_local_mem;

   /* For debugging/profiling purposes */
   unsigned variant_key_size;
   unsigned no;
   unsigned variants_created;
   unsigned variants_cached;
   bool zero_initialize_shared_memory;

   int max_global_buffers;
   struct pipe_resource **global_buffers;
};

struct lp_cs_exec {
   struct lp_jit_cs_context jit_context;
   struct lp_jit_resources jit_resources;
   struct lp_compute_shader_variant *variant;
};

struct lp_cs_context {
   struct pipe_context *pipe;

   struct {
      struct lp_cs_exec current;
      struct pipe_resource *current_tex[PIPE_MAX_SHADER_SAMPLER_VIEWS];
      unsigned current_tex_num;
   } cs;

   /** compute shader constants */
   struct {
      struct pipe_constant_buffer current;
      unsigned stored_size;
      const void *stored_data;
   } constants[LP_MAX_TGSI_CONST_BUFFERS];

   /** compute shader buffers */
   struct {
      struct pipe_shader_buffer current;
   } ssbos[LP_MAX_TGSI_SHADER_BUFFERS];

   struct {
      struct pipe_image_view current;
   } images[LP_MAX_TGSI_SHADER_IMAGES];

   const void *input;
};

struct lp_cs_context *lp_csctx_create(struct pipe_context *pipe);
void lp_csctx_destroy(struct lp_cs_context *csctx);

#endif
