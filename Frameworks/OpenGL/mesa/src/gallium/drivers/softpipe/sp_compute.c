/*
 * Copyright 2016 Red Hat.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "pipe/p_shader_tokens.h"
#include "draw/draw_context.h"
#include "draw/draw_vertex.h"
#include "sp_context.h"
#include "sp_screen.h"
#include "sp_state.h"
#include "sp_texture.h"
#include "sp_tex_sample.h"
#include "sp_tex_tile_cache.h"

static void
cs_prepare(const struct sp_compute_shader *cs,
           struct tgsi_exec_machine *machine,
           int local_x, int local_y, int local_z,
           int g_w, int g_h, int g_d,
           int b_w, int b_h, int b_d,
           struct tgsi_sampler *sampler,
           struct tgsi_image *image,
           struct tgsi_buffer *buffer )
{
   int j;
   /*
    * Bind tokens/shader to the interpreter's machine state.
    */
   tgsi_exec_machine_bind_shader(machine,
                                 cs->tokens,
                                 sampler, image, buffer);

   if (machine->SysSemanticToIndex[TGSI_SEMANTIC_THREAD_ID] != -1) {
      unsigned i = machine->SysSemanticToIndex[TGSI_SEMANTIC_THREAD_ID];
      for (j = 0; j < TGSI_QUAD_SIZE; j++) {
         machine->SystemValue[i].xyzw[0].i[j] = local_x + j;
         machine->SystemValue[i].xyzw[1].i[j] = local_y;
         machine->SystemValue[i].xyzw[2].i[j] = local_z;
      }
   }

   if (machine->SysSemanticToIndex[TGSI_SEMANTIC_GRID_SIZE] != -1) {
      unsigned i = machine->SysSemanticToIndex[TGSI_SEMANTIC_GRID_SIZE];
      for (j = 0; j < TGSI_QUAD_SIZE; j++) {
         machine->SystemValue[i].xyzw[0].i[j] = g_w;
         machine->SystemValue[i].xyzw[1].i[j] = g_h;
         machine->SystemValue[i].xyzw[2].i[j] = g_d;
      }
   }

   if (machine->SysSemanticToIndex[TGSI_SEMANTIC_BLOCK_SIZE] != -1) {
      unsigned i = machine->SysSemanticToIndex[TGSI_SEMANTIC_BLOCK_SIZE];
      for (j = 0; j < TGSI_QUAD_SIZE; j++) {
         machine->SystemValue[i].xyzw[0].i[j] = b_w;
         machine->SystemValue[i].xyzw[1].i[j] = b_h;
         machine->SystemValue[i].xyzw[2].i[j] = b_d;
      }
   }
}

static bool
cs_run(const struct sp_compute_shader *cs,
       int g_w, int g_h, int g_d,
       struct tgsi_exec_machine *machine, bool restart)
{
   if (!restart) {
      if (machine->SysSemanticToIndex[TGSI_SEMANTIC_BLOCK_ID] != -1) {
         unsigned i = machine->SysSemanticToIndex[TGSI_SEMANTIC_BLOCK_ID];
         int j;
         for (j = 0; j < TGSI_QUAD_SIZE; j++) {
            machine->SystemValue[i].xyzw[0].i[j] = g_w;
            machine->SystemValue[i].xyzw[1].i[j] = g_h;
            machine->SystemValue[i].xyzw[2].i[j] = g_d;
         }
      }
   }

   tgsi_exec_machine_run(machine, restart ? machine->pc : 0);

   if (machine->pc != -1)
      return true;
   return false;
}

static void
run_workgroup(const struct sp_compute_shader *cs,
              int g_w, int g_h, int g_d, int num_threads,
              struct tgsi_exec_machine **machines)
{
   int i;
   bool grp_hit_barrier, restart_threads = false;

   do {
      grp_hit_barrier = false;
      for (i = 0; i < num_threads; i++) {
         grp_hit_barrier |= cs_run(cs, g_w, g_h, g_d, machines[i], restart_threads);
      }
      restart_threads = false;
      if (grp_hit_barrier) {
         grp_hit_barrier = false;
         restart_threads = true;
      }
   } while (restart_threads);
}

static void
cs_delete(const struct sp_compute_shader *cs,
          struct tgsi_exec_machine *machine)
{
   if (machine->Tokens == cs->tokens) {
      tgsi_exec_machine_bind_shader(machine, NULL, NULL, NULL, NULL);
   }
}

static void
fill_grid_size(struct pipe_context *context,
               const struct pipe_grid_info *info,
               uint32_t grid_size[3])
{
   struct pipe_transfer *transfer;
   uint32_t *params;
   if (!info->indirect) {
      grid_size[0] = info->grid[0];
      grid_size[1] = info->grid[1];
      grid_size[2] = info->grid[2];
      return;
   }
   params = pipe_buffer_map_range(context, info->indirect,
                                  info->indirect_offset,
                                  3 * sizeof(uint32_t),
                                  PIPE_MAP_READ,
                                  &transfer);

   if (!transfer)
      return;

   grid_size[0] = params[0];
   grid_size[1] = params[1];
   grid_size[2] = params[2];
   pipe_buffer_unmap(context, transfer);
}

void
softpipe_launch_grid(struct pipe_context *context,
                     const struct pipe_grid_info *info)
{
   struct softpipe_context *softpipe = softpipe_context(context);
   struct sp_compute_shader *cs = softpipe->cs;
   int num_threads_in_group;
   struct tgsi_exec_machine **machines;
   int bwidth, bheight, bdepth;
   int local_x, local_y, local_z, i;
   int g_w, g_h, g_d;
   uint32_t grid_size[3] = {0};
   void *local_mem = NULL;

   softpipe_update_compute_samplers(softpipe);
   bwidth = cs->info.properties[TGSI_PROPERTY_CS_FIXED_BLOCK_WIDTH];
   bheight = cs->info.properties[TGSI_PROPERTY_CS_FIXED_BLOCK_HEIGHT];
   bdepth = cs->info.properties[TGSI_PROPERTY_CS_FIXED_BLOCK_DEPTH];
   num_threads_in_group = DIV_ROUND_UP(bwidth, TGSI_QUAD_SIZE) * bheight * bdepth;

   fill_grid_size(context, info, grid_size);

   uint32_t shared_mem_size = cs->shader.static_shared_mem + info->variable_shared_mem;
   if (shared_mem_size) {
      local_mem = CALLOC(1, shared_mem_size);
   }

   machines = CALLOC(sizeof(struct tgsi_exec_machine *), num_threads_in_group);
   if (!machines) {
      FREE(local_mem);
      return;
   }

   /* initialise machines + GRID_SIZE + THREAD_ID  + BLOCK_SIZE */
   int idx = 0;
   for (local_z = 0; local_z < bdepth; local_z++) {
      for (local_y = 0; local_y < bheight; local_y++) {
         for (local_x = 0; local_x < bwidth; local_x += TGSI_QUAD_SIZE) {
            machines[idx] = tgsi_exec_machine_create(PIPE_SHADER_COMPUTE);

            machines[idx]->LocalMem = local_mem;
            machines[idx]->LocalMemSize = shared_mem_size;
            machines[idx]->NonHelperMask = (1 << (MIN2(TGSI_QUAD_SIZE, bwidth - local_x))) - 1;
            cs_prepare(cs, machines[idx],
                       local_x, local_y, local_z,
                       grid_size[0], grid_size[1], grid_size[2],
                       bwidth, bheight, bdepth,
                       (struct tgsi_sampler *)softpipe->tgsi.sampler[PIPE_SHADER_COMPUTE],
                       (struct tgsi_image *)softpipe->tgsi.image[PIPE_SHADER_COMPUTE],
                       (struct tgsi_buffer *)softpipe->tgsi.buffer[PIPE_SHADER_COMPUTE]);
            tgsi_exec_set_constant_buffers(machines[idx], PIPE_MAX_CONSTANT_BUFFERS,
                                           softpipe->mapped_constants[PIPE_SHADER_COMPUTE]);
            idx++;
         }
      }
   }

   for (g_d = 0; g_d < grid_size[2]; g_d++) {
      for (g_h = 0; g_h < grid_size[1]; g_h++) {
         for (g_w = 0; g_w < grid_size[0]; g_w++) {
            run_workgroup(cs, g_w, g_h, g_d, num_threads_in_group, machines);
         }
      }
   }

   if (softpipe->active_statistics_queries) {
      softpipe->pipeline_statistics.cs_invocations +=
          grid_size[0] * grid_size[1] * grid_size[2];
   }

   for (i = 0; i < num_threads_in_group; i++) {
      cs_delete(cs, machines[i]);
      tgsi_exec_machine_destroy(machines[i]);
   }

   FREE(local_mem);
   FREE(machines);
}
