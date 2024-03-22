/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
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
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "pipe/p_screen.h"
#include "pipe/p_state.h"
#include "tgsi/tgsi_dump.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_string.h"

#include "nir/tgsi_to_nir.h"
#include "nir_serialize.h"

#include "freedreno_context.h"
#include "freedreno_util.h"

#include "ir3/ir3_cache.h"
#include "ir3/ir3_compiler.h"
#include "ir3/ir3_descriptor.h"
#include "ir3/ir3_gallium.h"
#include "ir3/ir3_nir.h"
#include "ir3/ir3_shader.h"

/**
 * The hardware cso for shader state
 *
 * Initially just a container for the ir3_shader, but this is where we'll
 * plumb in async compile.
 */
struct ir3_shader_state {
   struct ir3_shader *shader;

   /* Fence signalled when async compile is completed: */
   struct util_queue_fence ready;
};

/**
 * Should initial variants be compiled synchronously?
 *
 * The only case where util_debug_message() is used in the initial-variants
 * path is with FD_MESA_DEBUG=shaderdb.  So if either debug is disabled (ie.
 * debug.debug_message==NULL), or shaderdb stats are not enabled, we can
 * compile the initial shader variant asynchronously.
 */
static bool
initial_variants_synchronous(struct fd_context *ctx)
{
   return unlikely(ctx->debug.debug_message) || FD_DBG(SHADERDB) ||
          FD_DBG(SERIALC);
}

static void
dump_shader_info(struct ir3_shader_variant *v,
                 struct util_debug_callback *debug)
{
   if (!FD_DBG(SHADERDB))
      return;

   util_debug_message(
      debug, SHADER_INFO,
      "%s shader: %u inst, %u nops, %u non-nops, %u mov, %u cov, "
      "%u dwords, %u last-baryf, %u last-helper, %u half, %u full, %u constlen, "
      "%u cat0, %u cat1, %u cat2, %u cat3, %u cat4, %u cat5, %u cat6, %u cat7, "
      "%u stp, %u ldp, %u sstall, %u (ss), %u systall, %u (sy), %d waves, "
      "%d loops\n",
      ir3_shader_stage(v), v->info.instrs_count, v->info.nops_count,
      v->info.instrs_count - v->info.nops_count, v->info.mov_count,
      v->info.cov_count, v->info.sizedwords, v->info.last_baryf,
      v->info.last_helper, v->info.max_half_reg + 1, v->info.max_reg + 1,
      v->constlen,
      v->info.instrs_per_cat[0], v->info.instrs_per_cat[1],
      v->info.instrs_per_cat[2], v->info.instrs_per_cat[3],
      v->info.instrs_per_cat[4], v->info.instrs_per_cat[5],
      v->info.instrs_per_cat[6], v->info.instrs_per_cat[7],
      v->info.stp_count, v->info.ldp_count, v->info.sstall,
      v->info.ss, v->info.systall, v->info.sy, v->info.max_waves, v->loops);
}

static void
upload_shader_variant(struct ir3_shader_variant *v)
{
   struct ir3_compiler *compiler = v->compiler;

   assert(!v->bo);

   v->bo =
      fd_bo_new(compiler->dev, v->info.size, FD_BO_NOMAP,
                "%s:%s", ir3_shader_stage(v), v->name);

   /* Always include shaders in kernel crash dumps. */
   fd_bo_mark_for_dump(v->bo);

   fd_bo_upload(v->bo, v->bin, 0, v->info.size);
}

struct ir3_shader_variant *
ir3_shader_variant(struct ir3_shader *shader, struct ir3_shader_key key,
                   bool binning_pass, struct util_debug_callback *debug)
{
   struct ir3_shader_variant *v;
   bool created = false;

   MESA_TRACE_FUNC();

   /* Some shader key values may not be used by a given ir3_shader (for
    * example, fragment shader saturates in the vertex shader), so clean out
    * those flags to avoid recompiling.
    */
   ir3_key_clear_unused(&key, shader);

   v = ir3_shader_get_variant(shader, &key, binning_pass, false, &created);

   if (created) {
      if (shader->initial_variants_done) {
         perf_debug_message(debug, SHADER_INFO,
                            "%s shader: recompiling at draw time: global "
                            "0x%08x, vfsamples %x/%x, astc %x/%x\n",
                            ir3_shader_stage(v), key.global, key.vsamples,
                            key.fsamples, key.vastc_srgb, key.fastc_srgb);
      }

      dump_shader_info(v, debug);
      upload_shader_variant(v);

      if (v->binning) {
         upload_shader_variant(v->binning);
         dump_shader_info(v->binning, debug);
      }
   }

   return v;
}

static void
copy_stream_out(struct ir3_stream_output_info *i,
                const struct pipe_stream_output_info *p)
{
   STATIC_ASSERT(ARRAY_SIZE(i->stride) == ARRAY_SIZE(p->stride));
   STATIC_ASSERT(ARRAY_SIZE(i->output) == ARRAY_SIZE(p->output));

   i->streams_written = 0;
   i->num_outputs = p->num_outputs;
   for (int n = 0; n < ARRAY_SIZE(i->stride); n++) {
      i->stride[n] = p->stride[n];
      if (p->stride[n])
         i->streams_written |= BIT(n);
   }

   for (int n = 0; n < ARRAY_SIZE(i->output); n++) {
      i->output[n].register_index = p->output[n].register_index;
      i->output[n].start_component = p->output[n].start_component;
      i->output[n].num_components = p->output[n].num_components;
      i->output[n].output_buffer = p->output[n].output_buffer;
      i->output[n].dst_offset = p->output[n].dst_offset;
      i->output[n].stream = p->output[n].stream;
   }
}

static void
create_initial_variants(struct ir3_shader_state *hwcso,
                        struct util_debug_callback *debug)
{
   struct ir3_shader *shader = hwcso->shader;
   struct ir3_compiler *compiler = shader->compiler;
   nir_shader *nir = shader->nir;

   /* Compile standard variants immediately to try to avoid draw-time stalls
    * to run the compiler.
    */
   struct ir3_shader_key key = {
      .tessellation = IR3_TESS_NONE,
      .ucp_enables = MASK(nir->info.clip_distance_array_size),
      .msaa = true,
   };

   switch (nir->info.stage) {
   case MESA_SHADER_TESS_EVAL:
      key.tessellation = ir3_tess_mode(nir->info.tess._primitive_mode);
      break;

   case MESA_SHADER_TESS_CTRL:
      /* The primitive_mode field, while it exists for TCS, is not
       * populated (since separable shaders between TCS/TES are legal,
       * so TCS wouldn't have access to TES's declaration).  Make a
       * guess so that we shader-db something plausible for TCS.
       */
      if (nir->info.outputs_written & VARYING_BIT_TESS_LEVEL_INNER)
         key.tessellation = IR3_TESS_TRIANGLES;
      else
         key.tessellation = IR3_TESS_ISOLINES;
      break;

   case MESA_SHADER_GEOMETRY:
      key.has_gs = true;
      break;

   default:
      break;
   }

   key.safe_constlen = false;
   struct ir3_shader_variant *v = ir3_shader_variant(shader, key, false, debug);
   if (!v)
      return;

   if (v->constlen > compiler->max_const_safe) {
      key.safe_constlen = true;
      ir3_shader_variant(shader, key, false, debug);
   }

   /* For vertex shaders, also compile initial binning pass shader: */
   if (nir->info.stage == MESA_SHADER_VERTEX) {
      key.safe_constlen = false;
      v = ir3_shader_variant(shader, key, true, debug);
      if (!v)
         return;

      if (v->constlen > compiler->max_const_safe) {
         key.safe_constlen = true;
         ir3_shader_variant(shader, key, true, debug);
      }
   }

   shader->initial_variants_done = true;
}

static void
create_initial_variants_async(void *job, void *gdata, int thread_index)
{
   struct ir3_shader_state *hwcso = job;
   struct util_debug_callback debug = {};

   MESA_TRACE_FUNC();

   create_initial_variants(hwcso, &debug);
}

static void
create_initial_compute_variants_async(void *job, void *gdata, int thread_index)
{
   struct ir3_shader_state *hwcso = job;
   struct ir3_shader *shader = hwcso->shader;
   struct util_debug_callback debug = {};
   static struct ir3_shader_key key; /* static is implicitly zeroed */

   MESA_TRACE_FUNC();

   ir3_shader_variant(shader, key, false, &debug);
   shader->initial_variants_done = true;
}

/* a bit annoying that compute-shader and normal shader state objects
 * aren't a bit more aligned.
 */
void *
ir3_shader_compute_state_create(struct pipe_context *pctx,
                                const struct pipe_compute_state *cso)
{
   struct fd_context *ctx = fd_context(pctx);

   /* req_input_mem will only be non-zero for cl kernels (ie. clover).
    * This isn't a perfect test because I guess it is possible (but
    * uncommon) for none for the kernel parameters to be a global,
    * but ctx->set_global_bindings() can't fail, so this is the next
    * best place to fail if we need a newer version of kernel driver:
    */
   if ((cso->req_input_mem > 0) &&
       fd_device_version(ctx->dev) < FD_VERSION_BO_IOVA) {
      return NULL;
   }

   struct ir3_compiler *compiler = ctx->screen->compiler;
   nir_shader *nir;

   if (cso->ir_type == PIPE_SHADER_IR_NIR) {
      /* we take ownership of the reference: */
      nir = (nir_shader *)cso->prog;
   } else if (cso->ir_type == PIPE_SHADER_IR_NIR_SERIALIZED) {
      const nir_shader_compiler_options *options =
            ir3_get_compiler_options(compiler);
      const struct pipe_binary_program_header *hdr = cso->prog;
      struct blob_reader reader;

      blob_reader_init(&reader, hdr->blob, hdr->num_bytes);
      nir = nir_deserialize(NULL, options, &reader);

      ir3_finalize_nir(compiler, nir);
   } else {
      assert(cso->ir_type == PIPE_SHADER_IR_TGSI);
      if (ir3_shader_debug & IR3_DBG_DISASM) {
         tgsi_dump(cso->prog, 0);
      }
      nir = tgsi_to_nir(cso->prog, pctx->screen, false);
   }

   if (ctx->screen->gen >= 6)
      ir3_nir_lower_io_to_bindless(nir);

   enum ir3_wavesize_option api_wavesize = IR3_SINGLE_OR_DOUBLE;
   enum ir3_wavesize_option real_wavesize = IR3_SINGLE_OR_DOUBLE;

   if (ctx->screen->gen >= 6 && !ctx->screen->info->a6xx.supports_double_threadsize) {
      api_wavesize = IR3_SINGLE_ONLY;
      real_wavesize = IR3_SINGLE_ONLY;
   }

   struct ir3_shader *shader =
      ir3_shader_from_nir(compiler, nir, &(struct ir3_shader_options){
                              /* TODO: force to single on a6xx with legacy
                               * ballot extension that uses 64-bit masks
                               */
                              .api_wavesize = api_wavesize,
                              .real_wavesize = real_wavesize,
                          }, NULL);
   shader->cs.req_input_mem = align(cso->req_input_mem, 4) / 4;     /* byte->dword */
   shader->cs.req_local_mem = cso->static_shared_mem;

   struct ir3_shader_state *hwcso = calloc(1, sizeof(*hwcso));

   util_queue_fence_init(&hwcso->ready);
   hwcso->shader = shader;

   /* Immediately compile a standard variant.  We have so few variants in our
    * shaders, that doing so almost eliminates draw-time recompiles.  (This
    * is also how we get data from shader-db's ./run)
    */

   if (initial_variants_synchronous(ctx)) {
      static struct ir3_shader_key key; /* static is implicitly zeroed */
      ir3_shader_variant(shader, key, false, &ctx->debug);
      shader->initial_variants_done = true;
   } else {
      struct fd_screen *screen = ctx->screen;
      util_queue_add_job(&screen->compile_queue, hwcso, &hwcso->ready,
                         create_initial_compute_variants_async, NULL, 0);
   }

   return hwcso;
}

void *
ir3_shader_state_create(struct pipe_context *pctx,
                        const struct pipe_shader_state *cso)
{
   struct fd_context *ctx = fd_context(pctx);
   struct ir3_compiler *compiler = ctx->screen->compiler;
   struct ir3_shader_state *hwcso = calloc(1, sizeof(*hwcso));

   /*
    * Convert to nir (if necessary):
    */

   nir_shader *nir;
   if (cso->type == PIPE_SHADER_IR_NIR) {
      /* we take ownership of the reference: */
      nir = cso->ir.nir;
   } else {
      assert(cso->type == PIPE_SHADER_IR_TGSI);
      if (ir3_shader_debug & IR3_DBG_DISASM) {
         tgsi_dump(cso->tokens, 0);
      }
      nir = tgsi_to_nir(cso->tokens, pctx->screen, false);
   }

   if (ctx->screen->gen >= 6)
      ir3_nir_lower_io_to_bindless(nir);

   /*
    * Create ir3_shader:
    *
    * This part is cheap, it doesn't compile initial variants
    */

   struct ir3_stream_output_info stream_output = {};
   copy_stream_out(&stream_output, &cso->stream_output);

   hwcso->shader =
      ir3_shader_from_nir(compiler, nir, &(struct ir3_shader_options){
                              /* TODO: force to single on a6xx with legacy
                               * ballot extension that uses 64-bit masks
                               */
                              .api_wavesize = IR3_SINGLE_OR_DOUBLE,
                              .real_wavesize = IR3_SINGLE_OR_DOUBLE,
                          },
                          &stream_output);

   /*
    * Create initial variants to avoid draw-time stalls.  This is
    * normally done asynchronously, unless debug is enabled (which
    * will be the case for shader-db)
    */

   util_queue_fence_init(&hwcso->ready);

   if (initial_variants_synchronous(ctx)) {
      create_initial_variants(hwcso, &ctx->debug);
   } else {
      util_queue_add_job(&ctx->screen->compile_queue, hwcso, &hwcso->ready,
                         create_initial_variants_async, NULL, 0);
   }

   return hwcso;
}

void
ir3_shader_state_delete(struct pipe_context *pctx, void *_hwcso)
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_screen *screen = ctx->screen;
   struct ir3_shader_state *hwcso = _hwcso;
   struct ir3_shader *so = hwcso->shader;

   ir3_cache_invalidate(ctx->shader_cache, hwcso);

   /* util_queue_drop_job() guarantees that either:
    *  1) job did not execute
    *  2) job completed
    *
    * In either case the fence is signaled
    */
   util_queue_drop_job(&screen->compile_queue, &hwcso->ready);

   /* free the uploaded shaders, since this is handled outside of the
    * shared ir3 code (ie. not used by turnip):
    */
   for (struct ir3_shader_variant *v = so->variants; v; v = v->next) {
      fd_bo_del(v->bo);
      v->bo = NULL;

      if (v->binning && v->binning->bo) {
         fd_bo_del(v->binning->bo);
         v->binning->bo = NULL;
      }
   }

   ir3_shader_destroy(so);
   util_queue_fence_destroy(&hwcso->ready);
   free(hwcso);
}

struct ir3_shader *
ir3_get_shader(struct ir3_shader_state *hwcso)
{
   if (!hwcso)
      return NULL;

   MESA_TRACE_FUNC();

   struct ir3_shader *shader = hwcso->shader;
   perf_time (1000, "waited for %s:%s:%s variants",
              _mesa_shader_stage_to_abbrev(shader->type),
              shader->nir->info.name,
              shader->nir->info.label) {
      /* wait for initial variants to compile: */
      util_queue_fence_wait(&hwcso->ready);
   }

   return shader;
}

struct shader_info *
ir3_get_shader_info(struct ir3_shader_state *hwcso)
{
   if (!hwcso)
      return NULL;
   return &hwcso->shader->nir->info;
}

/* fixup dirty shader state in case some "unrelated" (from the state-
 * tracker's perspective) state change causes us to switch to a
 * different variant.
 */
void
ir3_fixup_shader_state(struct pipe_context *pctx, struct ir3_shader_key *key)
{
   struct fd_context *ctx = fd_context(pctx);

   if (!ir3_shader_key_equal(ctx->last.key, key)) {
      if (ir3_shader_key_changes_fs(ctx->last.key, key)) {
         fd_context_dirty_shader(ctx, PIPE_SHADER_FRAGMENT,
                                 FD_DIRTY_SHADER_PROG);
      }

      if (ir3_shader_key_changes_vs(ctx->last.key, key)) {
         fd_context_dirty_shader(ctx, PIPE_SHADER_VERTEX, FD_DIRTY_SHADER_PROG);
      }

      /* NOTE: currently only a6xx has gs/tess, but needs no
       * gs/tess specific lowering.
       */

      *ctx->last.key = *key;
   }
}

static char *
ir3_screen_finalize_nir(struct pipe_screen *pscreen, void *nir)
{
   struct fd_screen *screen = fd_screen(pscreen);

   MESA_TRACE_FUNC();

   ir3_nir_lower_io_to_temporaries(nir);
   ir3_finalize_nir(screen->compiler, nir);

   return NULL;
}

static void
ir3_set_max_shader_compiler_threads(struct pipe_screen *pscreen,
                                    unsigned max_threads)
{
   struct fd_screen *screen = fd_screen(pscreen);

   /* This function doesn't allow a greater number of threads than
    * the queue had at its creation.
    */
   util_queue_adjust_num_threads(&screen->compile_queue, max_threads,
                                 false);
}

static bool
ir3_is_parallel_shader_compilation_finished(struct pipe_screen *pscreen,
                                            void *shader,
                                            enum pipe_shader_type shader_type)
{
   struct ir3_shader_state *hwcso = (struct ir3_shader_state *)shader;

   return util_queue_fence_is_signalled(&hwcso->ready);
}

void
ir3_prog_init(struct pipe_context *pctx)
{
   pctx->create_vs_state = ir3_shader_state_create;
   pctx->delete_vs_state = ir3_shader_state_delete;

   pctx->create_tcs_state = ir3_shader_state_create;
   pctx->delete_tcs_state = ir3_shader_state_delete;

   pctx->create_tes_state = ir3_shader_state_create;
   pctx->delete_tes_state = ir3_shader_state_delete;

   pctx->create_gs_state = ir3_shader_state_create;
   pctx->delete_gs_state = ir3_shader_state_delete;

   pctx->create_fs_state = ir3_shader_state_create;
   pctx->delete_fs_state = ir3_shader_state_delete;
}

void
ir3_screen_init(struct pipe_screen *pscreen)
{
   struct fd_screen *screen = fd_screen(pscreen);

   struct ir3_compiler_options options = {
      .bindless_fb_read_descriptor =
         ir3_shader_descriptor_set(PIPE_SHADER_FRAGMENT),
      .bindless_fb_read_slot = IR3_BINDLESS_IMAGE_OFFSET +
                               IR3_BINDLESS_IMAGE_COUNT - 1 - screen->max_rts,
   };

   if (screen->gen >= 6) {
      options.lower_base_vertex = true;
   }
   screen->compiler =
      ir3_compiler_create(screen->dev, screen->dev_id, screen->info, &options);

   /* TODO do we want to limit things to # of fast cores, or just limit
    * based on total # of both big and little cores.  The little cores
    * tend to be in-order and probably much slower for compiling than
    * big cores.  OTOH if they are sitting idle, maybe it is useful to
    * use them?
    */
   unsigned num_threads = sysconf(_SC_NPROCESSORS_ONLN) / 2;

   /* Create at least one thread - even on single core CPU systems. */
   num_threads = MAX2(1, num_threads);

   util_queue_init(&screen->compile_queue, "ir3q", 64, num_threads,
                   UTIL_QUEUE_INIT_RESIZE_IF_FULL |
                      UTIL_QUEUE_INIT_SET_FULL_THREAD_AFFINITY, NULL);

   pscreen->finalize_nir = ir3_screen_finalize_nir;
   pscreen->set_max_shader_compiler_threads =
      ir3_set_max_shader_compiler_threads;
   pscreen->is_parallel_shader_compilation_finished =
      ir3_is_parallel_shader_compilation_finished;
}

void
ir3_screen_fini(struct pipe_screen *pscreen)
{
   struct fd_screen *screen = fd_screen(pscreen);

   util_queue_destroy(&screen->compile_queue);
   ir3_compiler_destroy(screen->compiler);
   screen->compiler = NULL;
}

void
ir3_update_max_tf_vtx(struct fd_context *ctx,
                      const struct ir3_shader_variant *v)
{
   struct fd_streamout_stateobj *so = &ctx->streamout;
   const struct ir3_stream_output_info *info = &v->stream_output;
   uint32_t maxvtxcnt = 0x7fffffff;

   if (v->stream_output.num_outputs == 0)
      maxvtxcnt = 0;
   if (so->num_targets == 0)
      maxvtxcnt = 0;

   /* offset to write to is:
    *
    *   total_vtxcnt = vtxcnt + offsets[i]
    *   offset = total_vtxcnt * stride[i]
    *
    *   offset =   vtxcnt * stride[i]       ; calculated in shader
    *            + offsets[i] * stride[i]   ; calculated at emit_tfbos()
    *
    * assuming for each vtx, each target buffer will have data written
    * up to 'offset + stride[i]', that leaves maxvtxcnt as:
    *
    *   buffer_size = (maxvtxcnt * stride[i]) + stride[i]
    *   maxvtxcnt   = (buffer_size - stride[i]) / stride[i]
    *
    * but shader is actually doing a less-than (rather than less-than-
    * equal) check, so we can drop the -stride[i].
    *
    * TODO is assumption about `offset + stride[i]` legit?
    */
   for (unsigned i = 0; i < so->num_targets; i++) {
      struct pipe_stream_output_target *target = so->targets[i];
      unsigned stride = info->stride[i] * 4; /* convert dwords->bytes */
      if (target) {
         uint32_t max = target->buffer_size / stride;
         maxvtxcnt = MIN2(maxvtxcnt, max);
      }
   }

   ctx->streamout.max_tf_vtx = maxvtxcnt;
}

void
ir3_get_private_mem(struct fd_context *ctx, const struct ir3_shader_variant *so)
{
   uint32_t fibers_per_sp = ctx->screen->info->fibers_per_sp;
   uint32_t num_sp_cores = ctx->screen->info->num_sp_cores;

   uint32_t per_fiber_size = so->pvtmem_size;
   if (per_fiber_size > ctx->pvtmem[so->pvtmem_per_wave].per_fiber_size) {
      if (ctx->pvtmem[so->pvtmem_per_wave].bo)
         fd_bo_del(ctx->pvtmem[so->pvtmem_per_wave].bo);

      uint32_t per_sp_size = ALIGN(per_fiber_size * fibers_per_sp, 1 << 12);
      uint32_t total_size = per_sp_size * num_sp_cores;

      ctx->pvtmem[so->pvtmem_per_wave].per_fiber_size = per_fiber_size;
      ctx->pvtmem[so->pvtmem_per_wave].per_sp_size = per_sp_size;
      ctx->pvtmem[so->pvtmem_per_wave].bo = fd_bo_new(
         ctx->screen->dev, total_size, FD_BO_NOMAP, "pvtmem_%s_%d",
         so->pvtmem_per_wave ? "per_wave" : "per_fiber", per_fiber_size);
   }
}
