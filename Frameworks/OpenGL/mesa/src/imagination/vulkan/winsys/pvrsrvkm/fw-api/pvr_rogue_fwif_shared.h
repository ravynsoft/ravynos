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

#ifndef PVR_ROGUE_FWIF_SHARED_H
#define PVR_ROGUE_FWIF_SHARED_H

#include <stdbool.h>
#include <stdint.h>

#include "util/macros.h"

/** Indicates the number of RTDATAs per RTDATASET. */
#define ROGUE_FWIF_NUM_RTDATAS 2U
#define ROGUE_FWIF_NUM_GEOMDATAS 1U
#define ROGUE_FWIF_NUM_RTDATA_FREELISTS 2U
#define ROGUE_NUM_GEOM_CORES 1U

#define ROGUE_NUM_GEOM_CORES_SIZE 2U

/**
 * Maximum number of UFOs in a CCB command.
 * The number is based on having 32 sync prims (as originally), plus 32 sync
 * checkpoints.
 * Once the use of sync prims is no longer supported, we will retain
 * the same total (64) as the number of sync checkpoints which may be
 * supporting a fence is not visible to the client driver and has to
 * allow for the number of different timelines involved in fence merges.
 */
#define ROGUE_FWIF_CCB_CMD_MAX_UFOS (32U + 32U)

/**
 * This is a generic limit imposed on any DM (TA,3D,CDM,TDM,2D,TRANSFER)
 * command passed through the bridge.
 * Just across the bridge in the server, any incoming kick command size is
 * checked against this maximum limit.
 * In case the incoming command size is larger than the specified limit,
 * the bridge call is retired with error.
 */
#define ROGUE_FWIF_DM_INDEPENDENT_KICK_CMD_SIZE (1024U)

struct rogue_fwif_dev_addr {
   uint32_t addr;
};

struct rogue_fwif_dma_addr {
   alignas(8) uint64_t dev_vaddr;
   struct rogue_fwif_dev_addr fw_addr;
   uint32_t padding;
};

/**
 * \brief Command data for fence & update types Client CCB commands.
 */
struct rogue_fwif_ufo {
   /** Address to be checked/updated. */
   struct rogue_fwif_dev_addr ufo_addr;
   /** Value to check-against/update-to. */
   uint32_t value;
};

struct rogue_fwif_cleanup_ctl {
   /** Number of commands received by the FW. */
   alignas(8) uint32_t submitted_cmds;

   /** Number of commands executed by the FW. */
   uint32_t executed_cmds;
};

#define ROGUE_FWIF_PRBUFFER_START 0U
#define ROGUE_FWIF_PRBUFFER_ZSBUFFER 0U
#define ROGUE_FWIF_PRBUFFER_MSAABUFFER 1U
#define ROGUE_FWIF_PRBUFFER_MAXSUPPORTED 2U

enum rogue_fwif_prbuffer_state {
   ROGUE_FWIF_PRBUFFER_UNBACKED = 0,
   ROGUE_FWIF_PRBUFFER_BACKED,
   ROGUE_FWIF_PRBUFFER_BACKING_PENDING,
   ROGUE_FWIF_PRBUFFER_UNBACKING_PENDING,
};

/**
 * \brief On-demand Z/S/MSAA buffers.
 */
struct rogue_fwif_prbuffer {
   /** Buffer ID. */
   alignas(8) uint32_t buffer_id;
   /** Needs on-demand Z/S/MSAA buffer allocation. */
   alignas(4) bool on_demand;
   /** Z/S/MSAA - Buffer state. */
   enum rogue_fwif_prbuffer_state state;
   /** Cleanup state. */
   struct rogue_fwif_cleanup_ctl cleanup_state;
   /** Compatibility and other flags. */
   uint32_t pr_buffer_flags;
};

/**
 * Used to share frame numbers across UM-KM-FW,
 * frame number is set in UM,
 * frame number is required in both KM for HTB and FW for FW trace.
 *
 * May be used to house Kick flags in the future.
 */
struct rogue_fwif_cmd_common {
   /** Associated frame number. */
   uint32_t frame_num;
};

/**
 * TA and 3D commands require set of firmware addresses that are stored in the
 * Kernel. Client has handle(s) to Kernel containers storing these addresses,
 * instead of raw addresses. We have to patch/write these addresses in KM to
 * prevent UM from controlling FW addresses directly.
 * Structures for TA and 3D commands are shared between Client and Firmware
 * (both single-BVNC). Kernel is implemented in a multi-BVNC manner, so it can't
 * use TA|3D CMD type definitions directly. Therefore we have a SHARED block
 * that is shared between UM-KM-FW across all BVNC configurations.
 */
struct rogue_fwif_cmd_ta_3d_shared {
   /** Common command attributes. */
   struct rogue_fwif_cmd_common cmn;

   /**
    * RTData associated with this command, this is used for context
    * selection and for storing out HW-context, when TA is switched out for
    * continuing later.
    */
   struct rogue_fwif_dev_addr hw_rt_data;

   /** Supported PR Buffers like Z/S/MSAA Scratch. */
   struct rogue_fwif_dev_addr pr_buffers[ROGUE_FWIF_PRBUFFER_MAXSUPPORTED];
};

/**
 * Client Circular Command Buffer (CCCB) control structure.
 * This is shared between the KM driver and the Firmware and holds byte offsets
 * into the CCCB as well as the wrapping mask to aid wrap around. A given
 * snapshot of this queue with Cmd 1 running on the GPU might be:
 *
 *          Roff                           Doff                 Woff
 * [..........|-1----------|=2===|=3===|=4===|~5~~~~|~6~~~~|~7~~~~|..........]
 *            <      runnable commands       ><   !ready to run   >
 *
 * Cmd 1    : Currently executing on the GPU data master.
 * Cmd 2,3,4: Fence dependencies met, commands runnable.
 * Cmd 5... : Fence dependency not met yet.
 */
struct rogue_fwif_cccb_ctl {
   /** Host write offset into CCB. This must be aligned to 16 bytes. */
   alignas(8) uint32_t write_offset;

   /**
    * Firmware read offset into CCB. Points to the command that is runnable
    * on GPU, if R!=W.
    */
   uint32_t read_offset;

   /**
    * Firmware fence dependency offset. Points to commands not ready, i.e.
    * fence dependencies are not met.
    */
   uint32_t dep_offset;

   /** Offset wrapping mask, total capacity in bytes of the CCB-1. */
   uint32_t wrap_mask;

   /* Only used if SUPPORT_AGP is present. */
   uint32_t read_offset2;

   /* Only used if SUPPORT_AGP4 is present. */
   uint32_t read_offset3;
   /* Only used if SUPPORT_AGP4 is present. */
   uint32_t read_offset4;

   uint32_t padding;
};

#define ROGUE_FW_LOCAL_FREELIST 0U
#define ROGUE_FW_GLOBAL_FREELIST 1U
#define ROGUE_FW_MAX_FREELISTS (ROGUE_FW_GLOBAL_FREELIST + 1U)
#define ROGUE_FW_MAX_HWFREELISTS 2U

/**
 * \brief Geom DM or TA register controls for context switch.
 */
struct rogue_fwif_ta_regs_cswitch {
   /** The base address of the VDM's context state buffer. */
   uint64_t vdm_context_state_base_addr;
   uint64_t vdm_context_state_resume_addr;
   /** The base address of the TA's context state buffer. */
   uint64_t ta_context_state_base_addr;

   struct {
      /** VDM context store task 0. */
      uint64_t vdm_context_store_task0;
      /** VDM context store task 1. */
      uint64_t vdm_context_store_task1;
      /** VDM context store task 2. */
      uint64_t vdm_context_store_task2;

      /* VDM resume state update controls. */
      /** VDM context resume task 0. */
      uint64_t vdm_context_resume_task0;
      /** VDM context resume task 1. */
      uint64_t vdm_context_resume_task1;
      /** VDM context resume task 2. */
      uint64_t vdm_context_resume_task2;

      uint64_t vdm_context_store_task3;
      uint64_t vdm_context_store_task4;

      uint64_t vdm_context_resume_task3;
      uint64_t vdm_context_resume_task4;
   } ta_state[2];
};

#define ROGUE_FWIF_TAREGISTERS_CSWITCH_SIZE \
   sizeof(struct rogue_fwif_taregisters_cswitch)

struct rogue_fwif_cdm_regs_cswitch {
   uint64_t cdm_context_pds0;
   uint64_t cdm_context_pds1;
   uint64_t cdm_terminate_pds;
   uint64_t cdm_terminate_pds1;

   /* CDM resume controls. */
   uint64_t cdm_resume_pds0;
   uint64_t cdm_context_pds0_b;
   uint64_t cdm_resume_pds0_b;
};

/**
 * \brief Render context static register controls for context switch.
 */
struct rogue_fwif_static_rendercontext_state {
   /** Geom registers for ctx switch. */
   alignas(8) struct rogue_fwif_ta_regs_cswitch
      ctx_switch_geom_regs[ROGUE_NUM_GEOM_CORES_SIZE];
};

#define ROGUE_FWIF_STATIC_RENDERCONTEXT_SIZE \
   sizeof(struct rogue_fwif_static_rendercontext_state)

struct rogue_fwif_static_computecontext_state {
   /** CDM registers for ctx switch. */
   alignas(8) struct rogue_fwif_cdm_regs_cswitch ctx_switch_regs;
};

#define ROGUE_FWIF_STATIC_COMPUTECONTEXT_SIZE \
   sizeof(struct rogue_fwif_static_computecontext_state)

/**
 * /brief Context reset reason. Last reset reason for a reset context.
 */
enum rogue_context_reset_reason {
   /** No reset reason recorded. */
   ROGUE_CONTEXT_RESET_REASON_NONE = 0,
   /** Caused a reset due to locking up. */
   ROGUE_CONTEXT_RESET_REASON_GUILTY_LOCKUP = 1,
   /** Affected by another context locking up. */
   ROGUE_CONTEXT_RESET_REASON_INNOCENT_LOCKUP = 2,
   /** Overran the global deadline. */
   ROGUE_CONTEXT_RESET_REASON_GUILTY_OVERRUNING = 3,
   /** Affected by another context overrunning. */
   ROGUE_CONTEXT_RESET_REASON_INNOCENT_OVERRUNING = 4,
   /** Forced reset to ensure scheduling requirements. */
   ROGUE_CONTEXT_RESET_REASON_HARD_CONTEXT_SWITCH = 5,
   /** FW page fault (no HWR). */
   ROGUE_CONTEXT_RESET_REASON_FW_PAGEFAULT = 13,
   /** FW execution error (GPU reset requested). */
   ROGUE_CONTEXT_RESET_REASON_FW_EXEC_ERR = 14,
   /** Host watchdog detected FW error. */
   ROGUE_CONTEXT_RESET_REASON_HOST_WDG_FW_ERR = 15,
   /** Geometry DM OOM event is not allowed. */
   ROGUE_CONTEXT_GEOM_OOM_DISABLED = 16,
};

/**
 * \brief Context reset data shared with the host.
 */
struct rogue_context_reset_reason_data {
   /** Reset reason. */
   enum rogue_context_reset_reason reset_reason;
   /** External Job ID. */
   uint32_t reset_ext_job_ref;
};

#endif /* PVR_ROGUE_FWIF_SHARED_H */
