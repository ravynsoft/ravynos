/*
 * Copyright © 2021 Raspberry Pi Ltd
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

#include "v3dv_private.h"
#include "broadcom/common/v3d_macros.h"
#include "broadcom/cle/v3dx_pack.h"
#include "broadcom/compiler/v3d_compiler.h"

void
v3dX(job_emit_noop)(struct v3dv_job *job)
{
   v3dv_job_start_frame(job, 1, 1, 1, true, true, 1,
                        V3D_INTERNAL_BPP_32, 4, false);
   v3dX(job_emit_binning_flush)(job);

   struct v3dv_cl *rcl = &job->rcl;
   v3dv_cl_ensure_space_with_branch(rcl, 200 + 1 * 256 *
                                    cl_packet_length(SUPERTILE_COORDINATES));

   cl_emit(rcl, TILE_RENDERING_MODE_CFG_COMMON, config) {
      config.early_z_disable = true;
      config.image_width_pixels = 1;
      config.image_height_pixels = 1;
      config.number_of_render_targets = 1;
      config.multisample_mode_4x = false;
#if V3D_VERSION == 42
      config.maximum_bpp_of_all_render_targets = V3D_INTERNAL_BPP_32;
#endif
#if V3D_VERSION >= 71
      config.log2_tile_width = 3; /* Tile size 64 */
      config.log2_tile_height = 3; /* Tile size 64 */
#endif
   }

#if V3D_VERSION == 42
   cl_emit(rcl, TILE_RENDERING_MODE_CFG_COLOR, rt) {
      rt.render_target_0_internal_bpp = V3D_INTERNAL_BPP_32;
      rt.render_target_0_internal_type = V3D_INTERNAL_TYPE_8;
      rt.render_target_0_clamp = V3D_RENDER_TARGET_CLAMP_NONE;
   }
#endif
#if V3D_VERSION >= 71
   cl_emit(rcl, TILE_RENDERING_MODE_CFG_RENDER_TARGET_PART1, rt) {
      rt.internal_bpp = V3D_INTERNAL_BPP_32;
      rt.internal_type_and_clamping = V3D_RENDER_TARGET_TYPE_CLAMP_8;
      rt.stride = 1; /* Unused RT */
   }
#endif

   cl_emit(rcl, TILE_RENDERING_MODE_CFG_ZS_CLEAR_VALUES, clear) {
      clear.z_clear_value = 1.0f;
      clear.stencil_clear_value = 0;
   };

   cl_emit(rcl, TILE_LIST_INITIAL_BLOCK_SIZE, init) {
      init.use_auto_chained_tile_lists = true;
      init.size_of_first_block_in_chained_tile_lists =
         TILE_ALLOCATION_BLOCK_SIZE_64B;
   }

   cl_emit(rcl, MULTICORE_RENDERING_TILE_LIST_SET_BASE, list) {
      list.address = v3dv_cl_address(job->tile_alloc, 0);
   }

   cl_emit(rcl, MULTICORE_RENDERING_SUPERTILE_CFG, config) {
      config.number_of_bin_tile_lists = 1;
      config.total_frame_width_in_tiles = 1;
      config.total_frame_height_in_tiles = 1;
      config.supertile_width_in_tiles = 1;
      config.supertile_height_in_tiles = 1;
      config.total_frame_width_in_supertiles = 1;
      config.total_frame_height_in_supertiles = 1;
   }

   struct v3dv_cl *icl = &job->indirect;
   v3dv_cl_ensure_space(icl, 200, 1);
   struct v3dv_cl_reloc tile_list_start = v3dv_cl_get_address(icl);

   cl_emit(icl, TILE_COORDINATES_IMPLICIT, coords);

   cl_emit(icl, END_OF_LOADS, end);

   cl_emit(icl, BRANCH_TO_IMPLICIT_TILE_LIST, branch);

   cl_emit(icl, STORE_TILE_BUFFER_GENERAL, store) {
      store.buffer_to_store = NONE;
   }

   cl_emit(icl, END_OF_TILE_MARKER, end);

   cl_emit(icl, RETURN_FROM_SUB_LIST, ret);

   cl_emit(rcl, START_ADDRESS_OF_GENERIC_TILE_LIST, branch) {
      branch.start = tile_list_start;
      branch.end = v3dv_cl_get_address(icl);
   }

   cl_emit(rcl, SUPERTILE_COORDINATES, coords) {
      coords.column_number_in_supertiles = 0;
      coords.row_number_in_supertiles = 0;
   }

   cl_emit(rcl, END_OF_RENDERING, end);
}
