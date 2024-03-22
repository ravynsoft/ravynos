/*
 * Copyright (c) 2011 Intel Corporation
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

#include <stdlib.h>
#include <math.h>

#include "util/u_debug.h"
#include "util/macros.h"
#include "util/u_math.h"
#include "compiler/shader_enums.h"

#include "intel_l3_config.h"

/**
 * The following diagram shows how we partition the URB:
 *
 *        16kb or 32kb               Rest of the URB space
 *   __________-__________   _________________-_________________
 *  /                     \ /                                   \
 * +-------------------------------------------------------------+
 * |  VS/HS/DS/GS/FS Push  |           VS/HS/DS/GS URB           |
 * |       Constants       |               Entries               |
 * +-------------------------------------------------------------+
 *
 * Push constants must be stored at the beginning of the URB space,
 * while URB entries can be stored anywhere.  We choose to lay them
 * out in pipeline order (VS -> HS -> DS -> GS).
 */

/**
 * Decide how to partition the URB among the various stages.
 *
 * \param[in] push_constant_bytes - space allocate for push constants.
 * \param[in] urb_size_bytes - total size of the URB (from L3 config).
 * \param[in] tess_present - are tessellation shaders active?
 * \param[in] gs_present - are geometry shaders active?
 * \param[in] entry_size - the URB entry size (from the shader compiler)
 * \param[out] entries - the number of URB entries for each stage
 * \param[out] start - the starting offset for each stage
 * \param[out] deref_block_size - deref block size for 3DSTATE_SF
 * \param[out] constrained - true if we wanted more space than we had
 */
void
intel_get_urb_config(const struct intel_device_info *devinfo,
                     const struct intel_l3_config *l3_cfg,
                     bool tess_present, bool gs_present,
                     const unsigned entry_size[4],
                     unsigned entries[4], unsigned start[4],
                     enum intel_urb_deref_block_size *deref_block_size,
                     bool *constrained)
{
   unsigned urb_size_kB = intel_get_l3_config_urb_size(devinfo, l3_cfg);

   /* RCU_MODE register for Gfx12LP in BSpec says:
    *
    *    "HW reserves 4KB of URB space per bank for Compute Engine out of the
    *    total storage available in L3. SW must consider that 4KB of storage
    *    per bank will be reduced from what is programmed for the URB space
    *    in L3 for Render Engine executed workloads.
    *
    *    Example: When URB space programmed is 64KB (per bank) for Render
    *    Engine, the actual URB space available for operation is only 60KB
    *    (per bank). Similarly when URB space programmed is 128KB (per bank)
    *    for render engine, the actual URB space available for operation is
    *    only 124KB (per bank). More detailed description available in "L3
    *    Cache" section of the B-Spec."
    */
   if (devinfo->verx10 == 120 && devinfo->has_compute_engine) {
      assert(devinfo->num_slices == 1);
      urb_size_kB -= 4 * devinfo->l3_banks;
   }

   const unsigned push_constant_kB = devinfo->max_constant_urb_size_kb;

   const bool active[4] = { true, tess_present, tess_present, gs_present };

   /* URB allocations must be done in 8k chunks. */
   const unsigned chunk_size_kB = 8;
   const unsigned chunk_size_bytes = chunk_size_kB * 1024;

   const unsigned push_constant_chunks = push_constant_kB / chunk_size_kB;
   const unsigned urb_chunks = urb_size_kB / chunk_size_kB;

   /* From p35 of the Ivy Bridge PRM (section 1.7.1: 3DSTATE_URB_GS):
    *
    *     VS Number of URB Entries must be divisible by 8 if the VS URB Entry
    *     Allocation Size is less than 9 512-bit URB entries.
    *
    * Similar text exists for HS, DS and GS.
    */
   unsigned granularity[4];
   for (int i = MESA_SHADER_VERTEX; i <= MESA_SHADER_GEOMETRY; i++) {
      granularity[i] = (entry_size[i] < 9) ? 8 : 1;
   }

   unsigned min_entries[4] = {
      /* VS has a lower limit on the number of URB entries.
       *
       * From the Broadwell PRM, 3DSTATE_URB_VS instruction:
       * "When tessellation is enabled, the VS Number of URB Entries must be
       *  greater than or equal to 192."
       */
      [MESA_SHADER_VERTEX] = tess_present && devinfo->ver == 8 ?
         192 : devinfo->urb.min_entries[MESA_SHADER_VERTEX],

      /* There are two constraints on the minimum amount of URB space we can
       * allocate:
       *
       * (1) We need room for at least 2 URB entries, since we always operate
       * the GS in DUAL_OBJECT mode.
       *
       * (2) We can't allocate less than nr_gs_entries_granularity.
       */
      [MESA_SHADER_GEOMETRY] = gs_present ? 2 : 0,

      [MESA_SHADER_TESS_CTRL] = tess_present ? 1 : 0,

      [MESA_SHADER_TESS_EVAL] = tess_present ?
         devinfo->urb.min_entries[MESA_SHADER_TESS_EVAL] : 0,
   };

   /* Min VS Entries isn't a multiple of 8 on Cherryview/Broxton; round up.
    * Round them all up.
    */
   for (int i = MESA_SHADER_VERTEX; i <= MESA_SHADER_GEOMETRY; i++) {
      min_entries[i] = ALIGN(min_entries[i], granularity[i]);
   }

   unsigned entry_size_bytes[4];
   for (int i = MESA_SHADER_VERTEX; i <= MESA_SHADER_GEOMETRY; i++) {
      entry_size_bytes[i] = 64 * entry_size[i];
   }

   /* Initially, assign each stage the minimum amount of URB space it needs,
    * and make a note of how much additional space it "wants" (the amount of
    * additional space it could actually make use of).
    */
   unsigned chunks[4];
   unsigned wants[4];
   unsigned total_needs = push_constant_chunks;
   unsigned total_wants = 0;

   for (int i = MESA_SHADER_VERTEX; i <= MESA_SHADER_GEOMETRY; i++) {
      if (active[i]) {
         chunks[i] = DIV_ROUND_UP(min_entries[i] * entry_size_bytes[i],
                                  chunk_size_bytes);

         wants[i] =
            DIV_ROUND_UP(devinfo->urb.max_entries[i] * entry_size_bytes[i],
                         chunk_size_bytes) - chunks[i];
      } else {
         chunks[i] = 0;
         wants[i] = 0;
      }

      total_needs += chunks[i];
      total_wants += wants[i];
   }

   assert(total_needs <= urb_chunks);

   *constrained = total_needs + total_wants > urb_chunks;

   /* Mete out remaining space (if any) in proportion to "wants". */
   unsigned remaining_space = MIN2(urb_chunks - total_needs, total_wants);

   if (remaining_space > 0) {
      for (int i = MESA_SHADER_VERTEX;
           total_wants > 0 && i <= MESA_SHADER_TESS_EVAL; i++) {
         unsigned additional = (unsigned)
            roundf(wants[i] * (((float) remaining_space) / total_wants));
         chunks[i] += additional;
         remaining_space -= additional;
         total_wants -= wants[i];
      }

      chunks[MESA_SHADER_GEOMETRY] += remaining_space;
   }

   /* Sanity check that we haven't over-allocated. */
   unsigned total_chunks = push_constant_chunks;
   for (int i = MESA_SHADER_VERTEX; i <= MESA_SHADER_GEOMETRY; i++) {
      total_chunks += chunks[i];
   }
   assert(total_chunks <= urb_chunks);

   /* Finally, compute the number of entries that can fit in the space
    * allocated to each stage.
    */
   for (int i = MESA_SHADER_VERTEX; i <= MESA_SHADER_GEOMETRY; i++) {
      entries[i] = chunks[i] * chunk_size_bytes / entry_size_bytes[i];

      /* Since we rounded up when computing wants[], this may be slightly
       * more than the maximum allowed amount, so correct for that.
       */
      entries[i] = MIN2(entries[i], devinfo->urb.max_entries[i]);

      /* Ensure that we program a multiple of the granularity. */
      entries[i] = ROUND_DOWN_TO(entries[i], granularity[i]);

      /* Finally, sanity check to make sure we have at least the minimum
       * number of entries needed for each stage.
       */
      assert(entries[i] >= min_entries[i]);
   }

   /* Lay out the URB in pipeline order: push constants, VS, HS, DS, GS. */
   int first_urb = push_constant_chunks;

   /* From the BDW PRM: for 3DSTATE_URB_*: VS URB Starting Address
    *
    *    "Value: [4,48] Device [SliceCount] GT 1"
    *
    * From the ICL PRMs and above :
    *
    *    "If CTXT_SR_CTL::POSH_Enable is clear and Push Constants are required
    *     or Device[SliceCount] GT 1, the lower limit is 4."
    *
    *    "If Push Constants are not required andDevice[SliceCount] == 1, the
    *     lower limit is 0."
    */
   if ((devinfo->ver == 8 && devinfo->num_slices == 1) ||
       (devinfo->ver >= 11 && push_constant_chunks > 0 && devinfo->num_slices == 1))
      first_urb = MAX2(first_urb, 4);

   int next_urb = first_urb;
   for (int i = MESA_SHADER_VERTEX; i <= MESA_SHADER_GEOMETRY; i++) {
      if (entries[i]) {
         start[i] = next_urb;
         next_urb += chunks[i];
      } else {
         /* Put disabled stages at the beginning of the valid range */
         start[i] = first_urb;
      }
   }

   if (deref_block_size) {
      if (devinfo->ver >= 12) {
         /* From the Gfx12 BSpec:
          *
          *    "Deref Block size depends on the last enabled shader and number
          *    of handles programmed for that shader
          *
          *       1) For GS last shader enabled cases, the deref block is
          *          always set to a per poly(within hardware)
          *
          *    If the last enabled shader is VS or DS.
          *
          *       1) If DS is last enabled shader then if the number of DS
          *          handles is less than 324, need to set per poly deref.
          *
          *       2) If VS is last enabled shader then if the number of VS
          *          handles is less than 192, need to set per poly deref"
          *
          * The default is 32 so we assume that's the right choice if we're
          * not in one of the explicit cases listed above.
          */
         if (gs_present) {
            *deref_block_size = INTEL_URB_DEREF_BLOCK_SIZE_PER_POLY;
         } else if (tess_present) {
            if (entries[MESA_SHADER_TESS_EVAL] < 324)
               *deref_block_size = INTEL_URB_DEREF_BLOCK_SIZE_PER_POLY;
            else
               *deref_block_size = INTEL_URB_DEREF_BLOCK_SIZE_32;
         } else {
            if (entries[MESA_SHADER_VERTEX] < 192)
               *deref_block_size = INTEL_URB_DEREF_BLOCK_SIZE_PER_POLY;
            else
               *deref_block_size = INTEL_URB_DEREF_BLOCK_SIZE_32;
         }
      } else {
         *deref_block_size = 0;
      }
   }
}

struct intel_mesh_urb_allocation
intel_get_mesh_urb_config(const struct intel_device_info *devinfo,
                          const struct intel_l3_config *l3_cfg,
                          unsigned tue_size_dw, unsigned mue_size_dw)
{
   struct intel_mesh_urb_allocation r = {0};

   /* Allocation Size must be aligned to 64B. */
   r.task_entry_size_64b = DIV_ROUND_UP(tue_size_dw * 4, 64);
   r.mesh_entry_size_64b = DIV_ROUND_UP(mue_size_dw * 4, 64);

   assert(r.task_entry_size_64b <= 1024);
   assert(r.mesh_entry_size_64b <= 1024);

   /* Per-slice URB size. */
   unsigned total_urb_kb = intel_get_l3_config_urb_size(devinfo, l3_cfg);

   /* Programming Note in bspec requires all the slice to have the same number
    * of entries, so we need to discount the space for constants for all of
    * them.  See 3DSTATE_URB_ALLOC_MESH and 3DSTATE_URB_ALLOC_TASK.
    */
   unsigned push_constant_kb = devinfo->mesh_max_constant_urb_size_kb;
   /* 3DSTATE_URB_ALLOC_MESH_BODY says
    *
    *    MESH URB Starting Address SliceN
    *       This field specifies the offset (from the start of the URB memory
    *       in slices beyond Slice0) of the MESH URB allocation, specified in
    *       multiples of 8 KB.
    */
   push_constant_kb = ALIGN(push_constant_kb, 8);
   total_urb_kb -= push_constant_kb;
   const unsigned total_urb_avail_mesh_task_kb = total_urb_kb;

   /* TODO(mesh): Take push constant size as parameter instead of considering always
    * the max? */

   float task_urb_share = 0.0f;
   if (r.task_entry_size_64b > 0) {
      /* By default, split memory between TASK and MESH proportionally to
       * their entry sizes. Environment variable allow us to tweak it.
       *
       * TODO(mesh): Re-evaluate if this is a good default once there are more
       * workloads.
       */
      static int task_urb_share_percentage = -1;
      if (task_urb_share_percentage == -1) {
         task_urb_share_percentage =
            MIN2(debug_get_num_option("INTEL_MESH_TASK_URB_SHARE", -2), 100);
      }

      if (task_urb_share_percentage >= 0) {
         task_urb_share = task_urb_share_percentage / 100.0f;
      } else {
         task_urb_share = (float)r.task_entry_size_64b / (r.task_entry_size_64b + r.mesh_entry_size_64b);
      }
   }

   /* 3DSTATE_URB_ALLOC_MESH_BODY and 3DSTATE_URB_ALLOC_TASK_BODY says
    *
    *   MESH Number of URB Entries must be divisible by 8 if the MESH/TASK URB
    *   Entry Allocation Size is less than 9 512-bit URB entries.
    */
   const unsigned min_mesh_entries = r.mesh_entry_size_64b < 9 ? 8 : 1;
   const unsigned min_task_entries = r.task_entry_size_64b < 9 ? 8 : 1;
   const unsigned min_mesh_urb_kb = ALIGN(r.mesh_entry_size_64b * min_mesh_entries * 64, 1024) / 1024;
   const unsigned min_task_urb_kb = ALIGN(r.task_entry_size_64b * min_task_entries * 64, 1024) / 1024;

   total_urb_kb -= (min_mesh_urb_kb + min_task_urb_kb);

   /* split the remaining urb_kbs */
   unsigned task_urb_kb = total_urb_kb * task_urb_share;
   unsigned mesh_urb_kb = total_urb_kb - task_urb_kb;

   /* sum minimum + split urb_kbs */
   mesh_urb_kb += min_mesh_urb_kb;

   /* 3DSTATE_URB_ALLOC_TASK_BODY says
    *    MESH Number of URB Entries SliceN
    *       This field specifies the offset (from the start of the URB memory
    *       in slices beyond Slice0) of the TASK URB allocation, specified in
    *       multiples of 8 KB.
    */
   if ((total_urb_avail_mesh_task_kb - ALIGN(mesh_urb_kb, 8)) >= min_task_entries) {
      mesh_urb_kb = ALIGN(mesh_urb_kb, 8);
   } else {
      mesh_urb_kb = ROUND_DOWN_TO(mesh_urb_kb, 8);
   }

   /* TODO(mesh): Could we avoid allocating URB for Mesh if rasterization is
    * disabled? */

   unsigned next_address_8kb = push_constant_kb / 8;
   assert(push_constant_kb % 8 == 0);

   r.mesh_starting_address_8kb = next_address_8kb;
   r.mesh_entries = MIN2((mesh_urb_kb * 16) / r.mesh_entry_size_64b, 1548);
   r.mesh_entries = r.mesh_entry_size_64b < 9 ? ROUND_DOWN_TO(r.mesh_entries, 8) : r.mesh_entries;

   next_address_8kb += mesh_urb_kb / 8;
   assert(mesh_urb_kb % 8 == 0);

   r.task_starting_address_8kb = next_address_8kb;
   task_urb_kb = total_urb_avail_mesh_task_kb - mesh_urb_kb;
   if (r.task_entry_size_64b > 0) {
      r.task_entries = MIN2((task_urb_kb * 16) / r.task_entry_size_64b, 1548);
      r.task_entries = r.task_entry_size_64b < 9 ? ROUND_DOWN_TO(r.task_entries, 8) : r.task_entries;
   }

   r.deref_block_size = r.mesh_entries > 32 ?
      INTEL_URB_DEREF_BLOCK_SIZE_MESH :
      INTEL_URB_DEREF_BLOCK_SIZE_PER_POLY;

   assert(mesh_urb_kb + task_urb_kb <= total_urb_avail_mesh_task_kb);
   assert(mesh_urb_kb >= min_mesh_urb_kb);
   assert(task_urb_kb >= min_task_urb_kb);

   return r;
}
