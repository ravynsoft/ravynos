/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PVR_JOB_CONTEXT_H
#define PVR_JOB_CONTEXT_H

#include "pvr_common.h"
#include "pvr_private.h"
#include "pvr_transfer_frag_store.h"
#include "pvr_types.h"
#include "pvr_uscgen.h"
#include "pvr_winsys.h"

/* Support PDS code/data loading/storing to the 'B' shared register state
 * buffers.
 */
#define ROGUE_NUM_SHADER_STATE_BUFFERS 2U

/* TODO: Add reset framework support. */
struct pvr_reset_cmd {
};

struct rogue_sr_programs {
   struct pvr_bo *store_load_state_bo;

   struct {
      uint8_t unified_size;

      struct pvr_suballoc_bo *store_program_bo;

      struct pvr_suballoc_bo *load_program_bo;
   } usc;

   struct {
      struct pvr_pds_upload store_program;
      struct pvr_pds_upload load_program;
   } pds;
};

/******************************************************************************
   Render context
 ******************************************************************************/

struct pvr_render_ctx {
   struct pvr_device *device;

   struct pvr_winsys_render_ctx *ws_ctx;

   /* Buffer to hold the VDM call stack */
   struct pvr_bo *vdm_callstack_bo;

   struct pvr_render_ctx_switch {
      /* Buffer to hold the VDM context resume control stream. */
      struct pvr_bo *vdm_state_bo;

      struct pvr_bo *geom_state_bo;

      struct pvr_render_ctx_programs {
         /* Context switch persistent state programs. */
         struct rogue_pt_programs {
            /* Buffer used to hold the persistent state. */
            struct pvr_bo *store_resume_state_bo;

            /* PDS program to store out the persistent state in
             * 'store_resume_state_bo'.
             */
            struct pvr_pds_upload pds_store_program;

            /* PDS program to load in the persistent state in
             * 'store_resume_state_bo'.
             */
            struct pvr_pds_upload pds_resume_program;
         } pt;

         /* Context switch shared register programs. */
         struct rogue_sr_programs sr;

      } programs[ROGUE_NUM_SHADER_STATE_BUFFERS];
   } ctx_switch;

   /* Reset framework. */
   struct pvr_reset_cmd reset_cmd;
};

/******************************************************************************
   Compute context
 ******************************************************************************/

struct pvr_compute_ctx {
   struct pvr_device *device;

   struct pvr_winsys_compute_ctx *ws_ctx;

   struct pvr_compute_ctx_switch {
      struct pvr_bo *compute_state_bo;

      struct rogue_sr_programs sr[ROGUE_NUM_SHADER_STATE_BUFFERS];

      struct pvr_pds_upload sr_fence_terminate_program;
   } ctx_switch;

   /* Reset framework. */
   struct pvr_reset_cmd reset_cmd;
};

/******************************************************************************
   Transfer context
 ******************************************************************************/

/* TODO: Can we move these to pds code headers? */
/* Maximum number of DMAs in the PDS TexState/Uniform program. */
#define PVR_TRANSFER_MAX_UNIFORM_DMA 1U
#define PVR_TRANSFER_MAX_TEXSTATE_DMA 2U

#if (PVR_TRANSFER_MAX_TEXSTATE_DMA >= PVR_PDS_MAX_NUM_DMA_KICKS) || \
   (PVR_TRANSFER_MAX_UNIFORM_DMA >= PVR_PDS_MAX_NUM_DMA_KICKS)
#   error \
      "Transfer queue can not support more DMA kicks than supported by PDS codegen."
#endif

struct pvr_transfer_ctx {
   struct pvr_device *device;

   /* Reset framework. */
   struct pvr_reset_cmd reset_cmd;

   struct pvr_winsys_transfer_ctx *ws_ctx;

   struct pvr_transfer_frag_store frag_store;

   struct pvr_suballoc_bo *usc_eot_bos[PVR_TRANSFER_MAX_RENDER_TARGETS];

   struct pvr_pds_upload pds_unitex_code[PVR_TRANSFER_MAX_TEXSTATE_DMA]
                                        [PVR_TRANSFER_MAX_UNIFORM_DMA];
};

/******************************************************************************
   Function prototypes
 ******************************************************************************/

VkResult pvr_render_ctx_create(struct pvr_device *device,
                               enum pvr_winsys_ctx_priority priority,
                               struct pvr_render_ctx **const ctx_out);
void pvr_render_ctx_destroy(struct pvr_render_ctx *ctx);

VkResult pvr_compute_ctx_create(struct pvr_device *const device,
                                enum pvr_winsys_ctx_priority priority,
                                struct pvr_compute_ctx **const ctx_out);
void pvr_compute_ctx_destroy(struct pvr_compute_ctx *ctx);

VkResult pvr_transfer_ctx_create(struct pvr_device *const device,
                                 enum pvr_winsys_ctx_priority priority,
                                 struct pvr_transfer_ctx **const ctx_out);
void pvr_transfer_ctx_destroy(struct pvr_transfer_ctx *const ctx);

#endif /* PVR_JOB_CONTEXT_H */
