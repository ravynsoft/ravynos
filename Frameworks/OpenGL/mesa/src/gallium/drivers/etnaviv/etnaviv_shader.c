/*
 * Copyright (c) 2012-2015 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Wladimir J. van der Laan <laanwj@gmail.com>
 */

#include "etnaviv_shader.h"

#include "etnaviv_compiler.h"
#include "etnaviv_context.h"
#include "etnaviv_debug.h"
#include "etnaviv_disasm.h"
#include "etnaviv_disk_cache.h"
#include "etnaviv_screen.h"
#include "etnaviv_util.h"

#include "nir/tgsi_to_nir.h"
#include "util/u_atomic.h"
#include "util/u_cpu_detect.h"
#include "util/u_math.h"
#include "util/u_memory.h"

/* Upload shader code to bo, if not already done */
static bool etna_icache_upload_shader(struct etna_context *ctx, struct etna_shader_variant *v)
{
   if (v->bo)
      return true;
   v->bo = etna_bo_new(ctx->screen->dev, v->code_size*4, DRM_ETNA_GEM_CACHE_WC);
   if (!v->bo)
      return false;

   void *buf = etna_bo_map(v->bo);
   etna_bo_cpu_prep(v->bo, DRM_ETNA_PREP_WRITE);
   memcpy(buf, v->code, v->code_size*4);
   etna_bo_cpu_fini(v->bo);
   DBG("Uploaded %s of %u words to bo %p", v->stage == MESA_SHADER_FRAGMENT ? "fs":"vs", v->code_size, v->bo);
   return true;
}

void
etna_dump_shader(const struct etna_shader_variant *shader)
{
   if (shader->stage == MESA_SHADER_VERTEX)
      printf("VERT\n");
   else
      printf("FRAG\n");

   etna_disasm(shader->code, shader->code_size, PRINT_RAW);

   printf("num loops: %i\n", shader->num_loops);
   printf("num temps: %i\n", shader->num_temps);
   printf("immediates:\n");
   for (int idx = 0; idx < shader->uniforms.count; ++idx) {
      printf(" [%i].%c = %f (0x%08x) (%d)\n",
             idx / 4,
             "xyzw"[idx % 4],
             *((float *)&shader->uniforms.data[idx]),
             shader->uniforms.data[idx],
             shader->uniforms.contents[idx]);
   }
   printf("inputs:\n");
   for (int idx = 0; idx < shader->infile.num_reg; ++idx) {
      printf(" [%i] name=%s comps=%i\n", shader->infile.reg[idx].reg,
               (shader->stage == MESA_SHADER_VERTEX) ?
               gl_vert_attrib_name(shader->infile.reg[idx].slot) :
               gl_varying_slot_name_for_stage(shader->infile.reg[idx].slot, shader->stage),
               shader->infile.reg[idx].num_components);
   }
   printf("outputs:\n");
   for (int idx = 0; idx < shader->outfile.num_reg; ++idx) {
      printf(" [%i] name=%s comps=%i\n", shader->outfile.reg[idx].reg,
               (shader->stage == MESA_SHADER_VERTEX) ?
               gl_varying_slot_name_for_stage(shader->outfile.reg[idx].slot, shader->stage) :
               gl_frag_result_name(shader->outfile.reg[idx].slot),
               shader->outfile.reg[idx].num_components);
   }
   printf("special:\n");
   if (shader->stage == MESA_SHADER_VERTEX) {
      printf("  vs_pos_out_reg=%i\n", shader->vs_pos_out_reg);
      printf("  vs_pointsize_out_reg=%i\n", shader->vs_pointsize_out_reg);
      printf("  vs_load_balancing=0x%08x\n", shader->vs_load_balancing);
   } else {
      printf("  ps_color_out_reg=%i\n", shader->ps_color_out_reg);
      printf("  ps_depth_out_reg=%i\n", shader->ps_depth_out_reg);
   }
   printf("  input_count_unk8=0x%08x\n", shader->input_count_unk8);
}

/* Link vs and fs together: fill in shader_state from vs and fs
 * as this function is called every time a new fs or vs is bound, the goal is to
 * do little processing as possible here, and to precompute as much as possible in
 * the vs/fs shader_object.
 *
 * XXX we could cache the link result for a certain set of VS/PS; usually a pair
 * of VS and PS will be used together anyway.
 */
static bool
etna_link_shaders(struct etna_context *ctx, struct compiled_shader_state *cs,
                  struct etna_shader_variant *vs, struct etna_shader_variant *fs)
{
   struct etna_shader_link_info link = { };

   assert(vs->stage == MESA_SHADER_VERTEX);
   assert(fs->stage == MESA_SHADER_FRAGMENT);

   etna_link_shader(&link, vs, fs);

   if (DBG_ENABLED(ETNA_DBG_LINKER_MSGS)) {
      debug_printf("link result:\n");
      debug_printf("  vs  -> fs  comps use     pa_attr\n");

      for (int idx = 0; idx < link.num_varyings; ++idx)
         debug_printf("  t%-2u -> t%-2u %-5.*s %u,%u,%u,%u 0x%08x\n",
                      link.varyings[idx].reg, idx + 1,
                      link.varyings[idx].num_components, "xyzw",
                      link.varyings[idx].use[0], link.varyings[idx].use[1],
                      link.varyings[idx].use[2], link.varyings[idx].use[3],
                      link.varyings[idx].pa_attributes);
   }

   /* set last_varying_2x flag if the last varying has 1 or 2 components */
   bool last_varying_2x = false;
   if (link.num_varyings > 0 && link.varyings[link.num_varyings - 1].num_components <= 2)
      last_varying_2x = true;

   cs->RA_CONTROL = VIVS_RA_CONTROL_UNK0 |
                    COND(last_varying_2x, VIVS_RA_CONTROL_LAST_VARYING_2X);

   cs->PA_ATTRIBUTE_ELEMENT_COUNT = VIVS_PA_ATTRIBUTE_ELEMENT_COUNT_COUNT(link.num_varyings);
   STATIC_ASSERT(VIVS_PA_SHADER_ATTRIBUTES__LEN >= ETNA_NUM_VARYINGS);
   for (int idx = 0; idx < link.num_varyings; ++idx)
      cs->PA_SHADER_ATTRIBUTES[idx] = link.varyings[idx].pa_attributes;

   cs->VS_END_PC = vs->code_size / 4;
   cs->VS_OUTPUT_COUNT = 1 + link.num_varyings; /* position + varyings */

   /* vs outputs (varyings) */
   DEFINE_ETNA_BITARRAY(vs_output, 16, 8) = {0};
   int varid = 0;
   etna_bitarray_set(vs_output, 8, varid++, vs->vs_pos_out_reg);
   for (int idx = 0; idx < link.num_varyings; ++idx)
      etna_bitarray_set(vs_output, 8, varid++, link.varyings[idx].reg);
   if (vs->vs_pointsize_out_reg >= 0)
      etna_bitarray_set(vs_output, 8, varid++, vs->vs_pointsize_out_reg); /* pointsize is last */

   for (int idx = 0; idx < ARRAY_SIZE(cs->VS_OUTPUT); ++idx)
      cs->VS_OUTPUT[idx] = vs_output[idx];

   if (vs->vs_pointsize_out_reg != -1) {
      /* vertex shader outputs point coordinate, provide extra output and make
       * sure PA config is
       * not masked */
      cs->PA_CONFIG = ~0;
      cs->VS_OUTPUT_COUNT_PSIZE = cs->VS_OUTPUT_COUNT + 1;
   } else {
      /* vertex shader does not output point coordinate, make sure thate
       * POINT_SIZE_ENABLE is masked
       * and no extra output is given */
      cs->PA_CONFIG = ~VIVS_PA_CONFIG_POINT_SIZE_ENABLE;
      cs->VS_OUTPUT_COUNT_PSIZE = cs->VS_OUTPUT_COUNT;
   }

   /* if fragment shader doesn't read pointcoord, disable it */
   if (link.pcoord_varying_comp_ofs == -1)
      cs->PA_CONFIG &= ~VIVS_PA_CONFIG_POINT_SPRITE_ENABLE;

   cs->VS_LOAD_BALANCING = vs->vs_load_balancing;
   cs->VS_START_PC = 0;

   cs->PS_END_PC = fs->code_size / 4;
   cs->PS_OUTPUT_REG = fs->ps_color_out_reg;
   cs->PS_INPUT_COUNT =
      VIVS_PS_INPUT_COUNT_COUNT(link.num_varyings + 1) | /* Number of inputs plus position */
      VIVS_PS_INPUT_COUNT_UNK8(fs->input_count_unk8);
   cs->PS_TEMP_REGISTER_CONTROL =
      VIVS_PS_TEMP_REGISTER_CONTROL_NUM_TEMPS(MAX2(fs->num_temps, link.num_varyings + 1));
   cs->PS_START_PC = 0;

   /* Precompute PS_INPUT_COUNT and TEMP_REGISTER_CONTROL in the case of MSAA
    * mode, avoids some fumbling in sync_context. */
   /* MSAA adds another input */
   cs->PS_INPUT_COUNT_MSAA =
      VIVS_PS_INPUT_COUNT_COUNT(link.num_varyings + 2) |
      VIVS_PS_INPUT_COUNT_UNK8(fs->input_count_unk8);
   /* MSAA adds another temp */
   cs->PS_TEMP_REGISTER_CONTROL_MSAA =
      VIVS_PS_TEMP_REGISTER_CONTROL_NUM_TEMPS(MAX2(fs->num_temps + 1, link.num_varyings + 2));

   uint32_t total_components = 0;
   DEFINE_ETNA_BITARRAY(num_components, ETNA_NUM_VARYINGS, 4) = {0};
   DEFINE_ETNA_BITARRAY(component_use, 4 * ETNA_NUM_VARYINGS, 2) = {0};
   for (int idx = 0; idx < link.num_varyings; ++idx) {
      const struct etna_varying *varying = &link.varyings[idx];

      etna_bitarray_set(num_components, 4, idx, varying->num_components);
      for (int comp = 0; comp < varying->num_components; ++comp) {
         etna_bitarray_set(component_use, 2, total_components, varying->use[comp]);
         total_components += 1;
      }
   }

   cs->GL_VARYING_TOTAL_COMPONENTS =
      VIVS_GL_VARYING_TOTAL_COMPONENTS_NUM(align(total_components, 2));
   cs->GL_VARYING_NUM_COMPONENTS[0] = num_components[0];
   cs->GL_VARYING_NUM_COMPONENTS[1] = num_components[1];
   cs->GL_VARYING_COMPONENT_USE[0] = component_use[0];
   cs->GL_VARYING_COMPONENT_USE[1] = component_use[1];

   cs->GL_HALTI5_SH_SPECIALS =
      0x7f7f0000 | /* unknown bits, probably other PS inputs */
      /* pointsize is last (see above) */
      VIVS_GL_HALTI5_SH_SPECIALS_VS_PSIZE_OUT((vs->vs_pointsize_out_reg != -1) ?
                                              cs->VS_OUTPUT_COUNT * 4 : 0x00) |
      VIVS_GL_HALTI5_SH_SPECIALS_PS_PCOORD_IN((link.pcoord_varying_comp_ofs != -1) ?
                                              link.pcoord_varying_comp_ofs : 0x7f);

   cs->writes_z = fs->ps_depth_out_reg >= 0;
   cs->uses_discard = fs->uses_discard;

   /* reference instruction memory */
   cs->vs_inst_mem_size = vs->code_size;
   cs->VS_INST_MEM = vs->code;

   cs->ps_inst_mem_size = fs->code_size;
   cs->PS_INST_MEM = fs->code;

   if (vs->needs_icache || fs->needs_icache) {
      /* If either of the shaders needs ICACHE, we use it for both. It is
       * either switched on or off for the entire shader processor.
       */
      if (!etna_icache_upload_shader(ctx, vs) ||
          !etna_icache_upload_shader(ctx, fs)) {
         assert(0);
         return false;
      }

      cs->VS_INST_ADDR.bo = vs->bo;
      cs->VS_INST_ADDR.offset = 0;
      cs->VS_INST_ADDR.flags = ETNA_RELOC_READ;
      cs->PS_INST_ADDR.bo = fs->bo;
      cs->PS_INST_ADDR.offset = 0;
      cs->PS_INST_ADDR.flags = ETNA_RELOC_READ;
   } else {
      /* clear relocs */
      memset(&cs->VS_INST_ADDR, 0, sizeof(cs->VS_INST_ADDR));
      memset(&cs->PS_INST_ADDR, 0, sizeof(cs->PS_INST_ADDR));
   }

   return true;
}

bool
etna_shader_link(struct etna_context *ctx)
{
   if (!ctx->shader.vs || !ctx->shader.fs)
      return false;

   /* re-link vs and fs if needed */
   return etna_link_shaders(ctx, &ctx->shader_state, ctx->shader.vs, ctx->shader.fs);
}

void
etna_destroy_shader(struct etna_shader_variant *shader)
{
   assert(shader);

   FREE(shader->code);
   FREE(shader->uniforms.data);
   FREE(shader->uniforms.contents);
   FREE(shader);
}

static bool
etna_shader_update_vs_inputs(struct compiled_shader_state *cs,
                             const struct etna_shader_variant *vs,
                             const struct compiled_vertex_elements_state *ves)
{
   unsigned num_temps, cur_temp, num_vs_inputs;

   if (!vs)
      return false;

   /* Number of vertex elements determines number of VS inputs. Otherwise,
    * the GPU crashes. Allocate any unused vertex elements to VS temporary
    * registers. */
   num_vs_inputs = MAX2(ves->num_elements, vs->infile.num_reg);
   if (num_vs_inputs != ves->num_elements) {
      BUG("Number of elements %u does not match the number of VS inputs %zu",
          ves->num_elements, vs->infile.num_reg);
      return false;
   }

   cur_temp = vs->num_temps;
   num_temps = num_vs_inputs - vs->infile.num_reg + cur_temp;

   cs->VS_INPUT_COUNT = VIVS_VS_INPUT_COUNT_COUNT(num_vs_inputs) |
                        VIVS_VS_INPUT_COUNT_UNK8(vs->input_count_unk8);
   cs->VS_TEMP_REGISTER_CONTROL =
      VIVS_VS_TEMP_REGISTER_CONTROL_NUM_TEMPS(num_temps);

   /* vs inputs (attributes) */
   DEFINE_ETNA_BITARRAY(vs_input, 16, 8) = {0};
   for (int idx = 0; idx < num_vs_inputs; ++idx) {
      if (idx < vs->infile.num_reg)
         etna_bitarray_set(vs_input, 8, idx, vs->infile.reg[idx].reg);
      else
         etna_bitarray_set(vs_input, 8, idx, cur_temp++);
   }

   if (vs->vs_id_in_reg >= 0) {
      cs->VS_INPUT_COUNT = VIVS_VS_INPUT_COUNT_COUNT(num_vs_inputs + 1) |
                           VIVS_VS_INPUT_COUNT_UNK8(vs->input_count_unk8) |
                           VIVS_VS_INPUT_COUNT_ID_ENABLE;

      etna_bitarray_set(vs_input, 8, num_vs_inputs, vs->vs_id_in_reg);

      cs->FE_HALTI5_ID_CONFIG =
         VIVS_FE_HALTI5_ID_CONFIG_VERTEX_ID_ENABLE |
         VIVS_FE_HALTI5_ID_CONFIG_INSTANCE_ID_ENABLE |
         VIVS_FE_HALTI5_ID_CONFIG_VERTEX_ID_REG(vs->vs_id_in_reg * 4) |
         VIVS_FE_HALTI5_ID_CONFIG_INSTANCE_ID_REG(vs->vs_id_in_reg * 4 + 1);
   }

   for (int idx = 0; idx < ARRAY_SIZE(cs->VS_INPUT); ++idx)
      cs->VS_INPUT[idx] = vs_input[idx];

   return true;
}

static inline const char *
etna_shader_stage(struct etna_shader *shader)
{
   switch (shader->nir->info.stage) {
   case MESA_SHADER_VERTEX:     return "VERT";
   case MESA_SHADER_FRAGMENT:   return "FRAG";
   case MESA_SHADER_COMPUTE:    return "CL";
   default:
      unreachable("invalid type");
      return NULL;
   }
}

static void
dump_shader_info(struct etna_shader_variant *v, struct util_debug_callback *debug)
{
   if (!DBG_ENABLED(ETNA_DBG_SHADERDB))
      return;

   util_debug_message(debug, SHADER_INFO,
         "%s shader: %u instructions, %u temps, "
         "%u immediates, %u loops",
         etna_shader_stage(v->shader),
         v->code_size / 4,
         v->num_temps,
         v->uniforms.count,
         v->num_loops);
}

bool
etna_shader_update_vertex(struct etna_context *ctx)
{
   return etna_shader_update_vs_inputs(&ctx->shader_state, ctx->shader.vs,
                                       ctx->vertex_elements);
}

static struct etna_shader_variant *
create_variant(struct etna_shader *shader,
               const struct etna_shader_key* const key)
{
   struct etna_shader_variant *v = CALLOC_STRUCT(etna_shader_variant);
   int ret;

   if (!v)
      return NULL;

   v->shader = shader;
   v->key = *key;
   v->id = ++shader->variant_count;

   if (etna_disk_cache_retrieve(shader->compiler, v))
      return v;

   ret = etna_compile_shader(v);
   if (!ret) {
      debug_error("compile failed!");
      goto fail;
   }

   etna_disk_cache_store(shader->compiler, v);

#ifdef DEBUG
   if (DBG_ENABLED(ETNA_DBG_DUMP_SHADERS))
      etna_dump_shader(v);
#endif

   return v;

fail:
   FREE(v);
   return NULL;
}

struct etna_shader_variant *
etna_shader_variant(struct etna_shader *shader,
                    const struct etna_shader_key* const key,
                    struct util_debug_callback *debug,
                    bool called_from_draw)
{
   struct etna_shader_variant *v;

   assert(shader->specs->fragment_sampler_count <= ARRAY_SIZE(key->tex_swizzle));

   for (v = shader->variants; v; v = v->next)
      if (etna_shader_key_equal(key, &v->key))
         return v;

   /* compile new variant if it doesn't exist already */
   v = create_variant(shader, key);
   if (v) {
      v->next = shader->variants;
      shader->variants = v;
      dump_shader_info(v, debug);
   }

   if (called_from_draw) {
      perf_debug_message(debug, SHADER_INFO,
                         "%s shader: recompiling at draw time: global "
                         "0x%08x\n",
                         etna_shader_stage(shader), key->global);
   }

   return v;
}

/**
 * Should initial variants be compiled synchronously?
 *
 * The only case where pipe_debug_message() is used in the initial-variants
 * path is with ETNA_MESA_DEBUG=shaderdb. So if either debug is disabled (ie.
 * debug.debug_message==NULL), or shaderdb stats are not enabled, we can
 * compile the initial shader variant asynchronously.
 */
static inline bool
initial_variants_synchronous(struct etna_context *ctx)
{
   return unlikely(ctx->base.debug.debug_message) || DBG_ENABLED(ETNA_DBG_SHADERDB);
}

static void
create_initial_variants_async(void *job, void *gdata, int thread_index)
{
   struct etna_shader *shader = job;
   struct util_debug_callback debug = {};
   static struct etna_shader_key key;

   etna_shader_variant(shader, &key, &debug, false);
}

static void *
etna_create_shader_state(struct pipe_context *pctx,
                         const struct pipe_shader_state *pss)
{
   struct etna_context *ctx = etna_context(pctx);
   struct etna_screen *screen = ctx->screen;
   struct etna_compiler *compiler = screen->compiler;
   struct etna_shader *shader = CALLOC_STRUCT(etna_shader);

   if (!shader)
      return NULL;

   shader->id = p_atomic_inc_return(&compiler->shader_count);
   shader->specs = &screen->specs;
   shader->compiler = screen->compiler;
   util_queue_fence_init(&shader->ready);

   shader->nir = (pss->type == PIPE_SHADER_IR_NIR) ? pss->ir.nir :
                  tgsi_to_nir(pss->tokens, pctx->screen, false);

   etna_disk_cache_init_shader_key(compiler, shader);

   if (initial_variants_synchronous(ctx)) {
      struct etna_shader_key key = {};
      etna_shader_variant(shader, &key, &ctx->base.debug, false);
   } else {
      struct etna_screen *screen = ctx->screen;
      util_queue_add_job(&screen->shader_compiler_queue, shader, &shader->ready,
                         create_initial_variants_async, NULL, 0);
   }

   return shader;
}

static void
etna_delete_shader_state(struct pipe_context *pctx, void *ss)
{
   struct etna_context *ctx = etna_context(pctx);
   struct etna_screen *screen = ctx->screen;
   struct etna_shader *shader = ss;
   struct etna_shader_variant *v, *t;

   util_queue_drop_job(&screen->shader_compiler_queue, &shader->ready);

   v = shader->variants;
   while (v) {
      t = v;
      v = v->next;
      if (t->bo)
         etna_bo_del(t->bo);

      etna_destroy_shader(t);
   }

   ralloc_free(shader->nir);
   util_queue_fence_destroy(&shader->ready);
   FREE(shader);
}

static void
etna_bind_fs_state(struct pipe_context *pctx, void *hwcso)
{
   struct etna_context *ctx = etna_context(pctx);

   ctx->shader.bind_fs = hwcso;
   ctx->dirty |= ETNA_DIRTY_SHADER;
}

static void
etna_bind_vs_state(struct pipe_context *pctx, void *hwcso)
{
   struct etna_context *ctx = etna_context(pctx);

   ctx->shader.bind_vs = hwcso;
   ctx->dirty |= ETNA_DIRTY_SHADER;
}

static void
etna_set_max_shader_compiler_threads(struct pipe_screen *pscreen,
                                     unsigned max_threads)
{
   struct etna_screen *screen = etna_screen(pscreen);

   util_queue_adjust_num_threads(&screen->shader_compiler_queue, max_threads,
                                 false);
}

static bool
etna_is_parallel_shader_compilation_finished(struct pipe_screen *pscreen,
                                             void *hwcso,
                                             enum pipe_shader_type shader_type)
{
   struct etna_shader *shader = (struct etna_shader *)hwcso;

   return util_queue_fence_is_signalled(&shader->ready);
}

void
etna_shader_init(struct pipe_context *pctx)
{
   pctx->create_fs_state = etna_create_shader_state;
   pctx->bind_fs_state = etna_bind_fs_state;
   pctx->delete_fs_state = etna_delete_shader_state;
   pctx->create_vs_state = etna_create_shader_state;
   pctx->bind_vs_state = etna_bind_vs_state;
   pctx->delete_vs_state = etna_delete_shader_state;
}

bool
etna_shader_screen_init(struct pipe_screen *pscreen)
{
   struct etna_screen *screen = etna_screen(pscreen);
   unsigned num_threads = util_get_cpu_caps()->nr_cpus - 1;

   /* Create at least one thread - even on single core CPU systems. */
   num_threads = MAX2(1, num_threads);

   screen->compiler = etna_compiler_create(pscreen->get_name(pscreen), &screen->specs);
   if (!screen->compiler)
      return false;

   pscreen->set_max_shader_compiler_threads = etna_set_max_shader_compiler_threads;
   pscreen->is_parallel_shader_compilation_finished = etna_is_parallel_shader_compilation_finished;

   return util_queue_init(&screen->shader_compiler_queue, "sh", 64, num_threads,
                          UTIL_QUEUE_INIT_RESIZE_IF_FULL | UTIL_QUEUE_INIT_SET_FULL_THREAD_AFFINITY,
                          NULL);
}

void
etna_shader_screen_fini(struct pipe_screen *pscreen)
{
   struct etna_screen *screen = etna_screen(pscreen);

   util_queue_destroy(&screen->shader_compiler_queue);
   etna_compiler_destroy(screen->compiler);
}
