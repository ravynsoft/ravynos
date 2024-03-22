/*
 * Copyright 2018 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

/* This file implements tests on the si_clearbuffer function. */

#include "si_pipe.h"
#include "si_query.h"

#define MIN_SIZE   512
#define MAX_SIZE   (128 * 1024 * 1024)
#define SIZE_SHIFT 1
#define NUM_RUNS   128

static double get_MBps_rate(unsigned num_bytes, unsigned ns)
{
   return (num_bytes / (1024.0 * 1024.0)) / (ns / 1000000000.0);
}

void si_test_dma_perf(struct si_screen *sscreen)
{
   struct pipe_screen *screen = &sscreen->b;
   struct pipe_context *ctx = screen->context_create(screen, NULL, 0);
   struct si_context *sctx = (struct si_context *)ctx;
   const uint32_t clear_value = 0x12345678;
   static const unsigned cs_dwords_per_thread_list[] = {64, 32, 16, 8, 4, 2, 1};
   static const unsigned cs_waves_per_sh_list[] = {0, 4, 8, 16};

#define NUM_SHADERS ARRAY_SIZE(cs_dwords_per_thread_list)
#define NUM_METHODS (3 + 3 * NUM_SHADERS * ARRAY_SIZE(cs_waves_per_sh_list))

   static const char *method_str[] = {
      "CP MC   ",
      "CP L2   ",
      "CP L2   ",
   };
   static const char *placement_str[] = {
      /* Clear */
      "fill->VRAM",
      "fill->GTT ",
      /* Copy */
      "VRAM->VRAM",
      "VRAM->GTT ",
      "GTT ->VRAM",
   };

   printf("DMA rate is in MB/s for each size. Slow cases are skipped and print 0.\n");
   printf("Heap       ,Method  ,L2p,Wa,");
   for (unsigned size = MIN_SIZE; size <= MAX_SIZE; size <<= SIZE_SHIFT) {
      if (size >= 1024)
         printf("%6uKB,", size / 1024);
      else
         printf(" %6uB,", size);
   }
   printf("\n");

   /* results[log2(size)][placement][method][] */
   struct si_result {
      bool is_valid;
      bool is_cp;
      bool is_cs;
      unsigned cache_policy;
      unsigned dwords_per_thread;
      unsigned waves_per_sh;
      unsigned score;
      unsigned index; /* index in results[x][y][index] */
   } results[32][ARRAY_SIZE(placement_str)][NUM_METHODS] = {};

   /* Run benchmarks. */
   for (unsigned placement = 0; placement < ARRAY_SIZE(placement_str); placement++) {
      bool is_copy = placement >= 2;

      printf("-----------,--------,---,--,");
      for (unsigned size = MIN_SIZE; size <= MAX_SIZE; size <<= SIZE_SHIFT)
         printf("--------,");
      printf("\n");

      for (unsigned method = 0; method < NUM_METHODS; method++) {
         bool test_cp = method <= 2;
         bool test_cs = method >= 3;
         unsigned cs_method = method - 3;
         unsigned cs_waves_per_sh =
            test_cs ? cs_waves_per_sh_list[cs_method / (3 * NUM_SHADERS)] : 0;
         cs_method %= 3 * NUM_SHADERS;
         unsigned cache_policy =
            test_cp ? method % 3 : test_cs ? (cs_method / NUM_SHADERS) : 0;
         unsigned cs_dwords_per_thread =
            test_cs ? cs_dwords_per_thread_list[cs_method % NUM_SHADERS] : 0;

         if (sctx->gfx_level == GFX6) {
            /* GFX6 doesn't support CP DMA operations through L2. */
            if (test_cp && cache_policy != L2_BYPASS)
               continue;
            /* WAVES_PER_SH is in multiples of 16 on GFX6. */
            if (test_cs && cs_waves_per_sh % 16 != 0)
               continue;
         }

         /* SI_RESOURCE_FLAG_GL2_BYPASS setting RADEON_FLAG_GL2_BYPASS doesn't affect
          * chips before gfx9.
          */
         if (test_cs && cache_policy && sctx->gfx_level < GFX9)
            continue;

         printf("%s ,", placement_str[placement]);
         if (test_cs) {
            printf("CS x%-4u,%3s,", cs_dwords_per_thread,
                   cache_policy == L2_LRU ? "LRU" : cache_policy == L2_STREAM ? "Str" : "");
         } else {
            printf("%s,%3s,", method_str[method],
                   method == L2_LRU ? "LRU" : method == L2_STREAM ? "Str" : "");
         }
         if (test_cs && cs_waves_per_sh)
            printf("%2u,", cs_waves_per_sh);
         else
            printf("  ,");

         void *compute_shader = NULL;
         if (test_cs) {
            compute_shader = si_create_dma_compute_shader(sctx, cs_dwords_per_thread,
                                              cache_policy == L2_STREAM, is_copy);
         }

         double score = 0;
         for (unsigned size = MIN_SIZE; size <= MAX_SIZE; size <<= SIZE_SHIFT) {
            /* Don't test bigger sizes if it's too slow. Print 0. */
            if (size >= 512 * 1024 && score < 400 * (size / (4 * 1024 * 1024))) {
               printf("%7.0f ,", 0.0);
               continue;
            }

            enum pipe_resource_usage dst_usage, src_usage;
            struct pipe_resource *dst, *src;
            unsigned query_type = PIPE_QUERY_TIME_ELAPSED;
            unsigned flags = cache_policy == L2_BYPASS ? SI_RESOURCE_FLAG_GL2_BYPASS : 0;

            if (placement == 0 || placement == 2 || placement == 4)
               dst_usage = PIPE_USAGE_DEFAULT;
            else
               dst_usage = PIPE_USAGE_STREAM;

            if (placement == 2 || placement == 3)
               src_usage = PIPE_USAGE_DEFAULT;
            else
               src_usage = PIPE_USAGE_STREAM;

            dst = pipe_aligned_buffer_create(screen, flags, dst_usage, size, 256);
            src = is_copy ? pipe_aligned_buffer_create(screen, flags, src_usage, size, 256) : NULL;

            /* Wait for idle before testing, so that other processes don't mess up the results. */
            sctx->flags |= SI_CONTEXT_CS_PARTIAL_FLUSH |
                           SI_CONTEXT_FLUSH_AND_INV_CB |
                           SI_CONTEXT_FLUSH_AND_INV_DB;
            si_emit_cache_flush_direct(sctx);

            struct pipe_query *q = ctx->create_query(ctx, query_type, 0);
            ctx->begin_query(ctx, q);

            /* Run tests. */
            for (unsigned iter = 0; iter < NUM_RUNS; iter++) {
               if (test_cp) {
                  /* CP DMA */
                  if (is_copy) {
                     si_cp_dma_copy_buffer(sctx, dst, src, 0, 0, size, SI_OP_SYNC_BEFORE_AFTER,
                                           SI_COHERENCY_NONE, cache_policy);
                  } else {
                     si_cp_dma_clear_buffer(sctx, &sctx->gfx_cs, dst, 0, size, clear_value,
                                            SI_OP_SYNC_BEFORE_AFTER, SI_COHERENCY_NONE,
                                            cache_policy);
                  }
               } else {
                  /* Compute */
                  /* The memory accesses are coalesced, meaning that the 1st instruction writes
                   * the 1st contiguous block of data for the whole wave, the 2nd instruction
                   * writes the 2nd contiguous block of data, etc.
                   */
                  unsigned instructions_per_thread = MAX2(1, cs_dwords_per_thread / 4);
                  unsigned dwords_per_instruction = cs_dwords_per_thread / instructions_per_thread;
                  unsigned dwords_per_wave = cs_dwords_per_thread * 64;

                  unsigned num_dwords = size / 4;
                  unsigned num_instructions = DIV_ROUND_UP(num_dwords, dwords_per_instruction);

                  struct pipe_grid_info info = {};
                  info.block[0] = MIN2(64, num_instructions);
                  info.block[1] = 1;
                  info.block[2] = 1;
                  info.grid[0] = DIV_ROUND_UP(num_dwords, dwords_per_wave);
                  info.grid[1] = 1;
                  info.grid[2] = 1;

                  struct pipe_shader_buffer sb[2] = {};
                  sb[0].buffer = dst;
                  sb[0].buffer_size = size;

                  if (is_copy) {
                     sb[1].buffer = src;
                     sb[1].buffer_size = size;
                  } else {
                     for (unsigned i = 0; i < 4; i++)
                        sctx->cs_user_data[i] = clear_value;
                  }

                  ctx->set_shader_buffers(ctx, PIPE_SHADER_COMPUTE, 0, is_copy ? 2 : 1, sb, 0x1);
                  ctx->bind_compute_state(ctx, compute_shader);
                  sctx->cs_max_waves_per_sh = cs_waves_per_sh;

                  ctx->launch_grid(ctx, &info);

                  ctx->bind_compute_state(ctx, NULL);
                  sctx->cs_max_waves_per_sh = 0; /* disable the limit */
               }

               /* Flush L2, so that we don't just test L2 cache performance except for L2_LRU. */
               sctx->flags |= SI_CONTEXT_INV_VCACHE |
                              (cache_policy == L2_LRU ? 0 : SI_CONTEXT_INV_L2) |
                              SI_CONTEXT_CS_PARTIAL_FLUSH;
               si_emit_cache_flush_direct(sctx);
            }

            ctx->end_query(ctx, q);
            ctx->flush(ctx, NULL, PIPE_FLUSH_ASYNC);

            pipe_resource_reference(&dst, NULL);
            pipe_resource_reference(&src, NULL);

            /* Get results. */

            union pipe_query_result result;

            ctx->get_query_result(ctx, q, true, &result);
            ctx->destroy_query(ctx, q);

            score = get_MBps_rate(size, result.u64 / (double)NUM_RUNS);
            printf("%7.0f ,", score);
            fflush(stdout);

            struct si_result *r = &results[util_logbase2(size)][placement][method];
            r->is_valid = true;
            r->is_cp = test_cp;
            r->is_cs = test_cs;
            r->cache_policy = cache_policy;
            r->dwords_per_thread = cs_dwords_per_thread;
            r->waves_per_sh = cs_waves_per_sh;
            r->score = score;
            r->index = method;
         }
         puts("");

         if (compute_shader)
            ctx->delete_compute_state(ctx, compute_shader);
      }
   }

   puts("");
   puts("static struct si_method");
   printf("get_best_clear_for_%s(enum radeon_bo_domain dst, uint64_t size64, bool async, bool "
          "cached)\n",
          sctx->screen->info.name);
   puts("{");
   puts("   unsigned size = MIN2(size64, UINT_MAX);\n");

   /* Analyze results and find the best methods. */
   for (unsigned placement = 0; placement < ARRAY_SIZE(placement_str); placement++) {
      if (placement == 0)
         puts("   if (dst == RADEON_DOMAIN_VRAM) {");
      else if (placement == 1)
         puts("   } else { /* GTT */");
      else if (placement == 2) {
         puts("}");
         puts("");
         puts("static struct si_method");
         printf("get_best_copy_for_%s(enum radeon_bo_domain dst, enum radeon_bo_domain src,\n",
                sctx->screen->info.name);
         printf("                     uint64_t size64, bool async, bool cached)\n");
         puts("{");
         puts("   unsigned size = MIN2(size64, UINT_MAX);\n");
         puts("   if (src == RADEON_DOMAIN_VRAM && dst == RADEON_DOMAIN_VRAM) {");
      } else if (placement == 3)
         puts("   } else if (src == RADEON_DOMAIN_VRAM && dst == RADEON_DOMAIN_GTT) {");
      else
         puts("   } else { /* GTT -> VRAM */");

      for (unsigned mode = 0; mode < 3; mode++) {
         bool async = mode == 0;
         bool cached = mode == 1;

         if (async)
            puts("      if (async) { /* async compute */");
         else if (cached)
            puts("      if (cached) { /* gfx ring */");
         else
            puts("      } else { /* gfx ring - uncached */");

         /* The list of best chosen methods. */
         struct si_result *methods[32];
         unsigned method_max_size[32];
         unsigned num_methods = 0;

         for (unsigned size = MIN_SIZE; size <= MAX_SIZE; size <<= SIZE_SHIFT) {
            /* Find the best method. */
            struct si_result *best = NULL;

            for (unsigned i = 0; i < NUM_METHODS; i++) {
               struct si_result *r = &results[util_logbase2(size)][placement][i];

               if (!r->is_valid)
                  continue;

               /* Ban CP DMA clears via MC on <= GFX8. They are super slow
                * on GTT, which we can get due to BO evictions.
                */
               if (sctx->gfx_level <= GFX8 && placement == 1 && r->is_cp &&
                   r->cache_policy == L2_BYPASS)
                  continue;

               if (async) {
                  /* The following constraints for compute IBs try to limit
                   * resource usage so as not to decrease the performance
                   * of gfx IBs too much.
                   */

                  /* Don't use CP DMA on asynchronous rings, because
                   * the engine is shared with gfx IBs.
                   */
                  if (r->is_cp)
                     continue;

                  /* Don't use L2 caching on asynchronous rings to minimize
                   * L2 usage.
                   */
                  if (r->cache_policy == L2_LRU)
                     continue;

                  /* Asynchronous compute recommends waves_per_sh != 0
                   * to limit CU usage. */
                  if (r->is_cs && r->waves_per_sh == 0)
                     continue;
               } else {
                  if (cached && r->cache_policy == L2_BYPASS)
                     continue;
                  if (!cached && r->cache_policy == L2_LRU)
                     continue;
               }

               if (!best) {
                  best = r;
                  continue;
               }

               /* Assume some measurement error. Earlier methods occupy fewer
                * resources, so the next method is always more greedy, and we
                * don't want to select it due to a measurement error.
                */
               double min_improvement = 1.03;

               if (best->score * min_improvement < r->score)
                  best = r;
            }

            if (num_methods > 0) {
               unsigned prev_index = num_methods - 1;
               struct si_result *prev = methods[prev_index];
               struct si_result *prev_this_size =
                  &results[util_logbase2(size)][placement][prev->index];

               /* If the best one is also the best for the previous size,
                * just bump the size for the previous one.
                *
                * If there is no best, it means all methods were too slow
                * for this size and were not tested. Use the best one for
                * the previous size.
                */
               if (!best ||
                   /* If it's the same method as for the previous size: */
                   (prev->is_cp == best->is_cp &&
                    prev->is_cs == best->is_cs && prev->cache_policy == best->cache_policy &&
                    prev->dwords_per_thread == best->dwords_per_thread &&
                    prev->waves_per_sh == best->waves_per_sh) ||
                   /* If the method for the previous size is also the best
                    * for this size: */
                   (prev_this_size->is_valid && prev_this_size->score * 1.03 > best->score)) {
                  method_max_size[prev_index] = size;
                  continue;
               }
            }

            /* Add it to the list. */
            assert(num_methods < ARRAY_SIZE(methods));
            methods[num_methods] = best;
            method_max_size[num_methods] = size;
            num_methods++;
         }

         for (unsigned i = 0; i < num_methods; i++) {
            struct si_result *best = methods[i];
            unsigned size = method_max_size[i];

            /* The size threshold is between the current benchmarked
             * size and the next benchmarked size. */
            if (i < num_methods - 1)
               printf("         if (size <= %9u) ", (size + (size << SIZE_SHIFT)) / 2);
            else if (i > 0)
               printf("         else                   ");
            else
               printf("         ");
            printf("return ");

            assert(best);
            const char *cache_policy_str =
               best->cache_policy == L2_BYPASS ? "L2_BYPASS" :
               best->cache_policy == L2_LRU ? "L2_LRU   " : "L2_STREAM";

            if (best->is_cp) {
               printf("CP_DMA(%s);\n", cache_policy_str);
            }
            if (best->is_cs) {
               printf("COMPUTE(%s, %u, %u);\n", cache_policy_str,
                      best->dwords_per_thread, best->waves_per_sh);
            }
         }
      }
      puts("      }");
   }
   puts("   }");
   puts("}");

   ctx->destroy(ctx);
   exit(0);
}
