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

/* This file is based on rgxdefs.h and should only contain object-like macros.
 * Any function-like macros or inline functions should instead appear in
 * rogue_hw_utils.h.
 */

#ifndef ROGUE_HW_DEFS_H
#define ROGUE_HW_DEFS_H

#include <stdint.h>

#include "util/macros.h"

#define ROGUE_BIF_PM_PHYSICAL_PAGE_SHIFT 12U
#define ROGUE_BIF_PM_PHYSICAL_PAGE_SIZE \
   BITFIELD_BIT(ROGUE_BIF_PM_PHYSICAL_PAGE_SHIFT)

/* ISP triangle merging constants. */
/* tan(15) (0x3E8930A3) */
#define ROGUE_ISP_MERGE_LOWER_LIMIT_NUMERATOR 0.267949f
/* tan(60) (0x3FDDB3D7) */
#define ROGUE_ISP_MERGE_UPPER_LIMIT_NUMERATOR 1.732051f
#define ROGUE_ISP_MERGE_SCALE_FACTOR 16.0f

#define ROGUE_MAX_INSTR_BYTES 32U

/* MList entry stride in bytes */
#define ROGUE_MLIST_ENTRY_STRIDE 4U

/* VCE & TE share virtual space and Alist. */
#define ROGUE_NUM_PM_ADDRESS_SPACES 2U

/* PM Maximum addressable limit (as determined by the size field of the
 * PM_*_FSTACK registers).
 */
#define ROGUE_PM_MAX_PB_VIRT_ADDR_SPACE UINT64_C(0x400000000)

/* Vheap entry size in bytes. */
#define ROGUE_PM_VHEAP_ENTRY_SIZE 4U

#define ROGUE_RTC_SIZE_IN_BYTES 256U

#define ROGUE_NUM_VCE 1U

#define ROGUE_NUM_TEAC 1U

#define ROGUE_NUM_TE 1U

/* Tail pointer size in bytes. */
#define ROGUE_TAIL_POINTER_SIZE 8U

/* Tail pointer cache line size. */
#define ROGUE_TE_TPC_CACHE_LINE_SIZE 64U

#define ROGUE_MAX_VERTEX_SHARED_REGISTERS 1024U

#define ROGUE_MAX_PIXEL_SHARED_REGISTERS 1024U

/* Number of CR_PDS_BGRND values that need setting up. */
#define ROGUE_NUM_CR_PDS_BGRND_WORDS 3U

/* Number of PBESTATE_REG_WORD values that need setting up. */
#define ROGUE_NUM_PBESTATE_REG_WORDS 3U

/* Number of PBESTATE_REG_WORD used in transfer.
 * The last word is not used.
 */
#define ROGUE_NUM_PBESTATE_REG_WORDS_FOR_TRANSFER 2U

/* Number of PBESTATE_STATE_WORD values that need setting up. */
#define ROGUE_NUM_PBESTATE_STATE_WORDS 2U

/* Number of TEXSTATE_IMAGE_WORD values that need setting up. */
#define ROGUE_NUM_TEXSTATE_IMAGE_WORDS 2U

/* Number of TEXSTATE_SAMPLER state words that need setting up. */
#define ROGUE_NUM_TEXSTATE_SAMPLER_WORDS 2U

/* 12 dwords reserved for shared register management. The first dword is the
 * number of shared register blocks to reload. Should be a multiple of 4 dwords,
 * size in bytes.
 */
#define ROGUE_LLS_SHARED_REGS_RESERVE_SIZE 48U

#define ROGUE_USC_TASK_PROGRAM_SIZE 512U

#define ROGUE_CSRM_LINE_SIZE_IN_DWORDS (64U * 4U * 4U)

/* The maximum amount of local memory which can be allocated by a single kernel
 * (in dwords/32-bit registers).
 *
 * ROGUE_CDMCTRL_USC_COMMON_SIZE_UNIT_SIZE is in bytes so we divide by four.
 */
#define ROGUE_MAX_PER_KERNEL_LOCAL_MEM_SIZE_REGS        \
   ((ROGUE_CDMCTRL_KERNEL0_USC_COMMON_SIZE_UNIT_SIZE *  \
     ROGUE_CDMCTRL_KERNEL0_USC_COMMON_SIZE_MAX_SIZE) >> \
    2)

#define ROGUE_MAX_INSTANCES_PER_TASK \
   (ROGUE_CDMCTRL_KERNEL8_MAX_INSTANCES_MAX_SIZE + 1U)

/* Optimal number for packing work groups into a slot. */
#define ROGUE_CDM_MAX_PACKED_WORKGROUPS_PER_TASK 8U

/* The maximum number of pixel task instances which might be running overlapped
 * with compute. Once we have 8 pixel task instances we have a complete set and
 * task will be able to run and allocations will be freed.
 */
#define ROGUE_MAX_OVERLAPPED_PIXEL_TASK_INSTANCES 7U

/* Size of the image state in 64-bit units. */
#define ROGUE_MAXIMUM_IMAGE_STATE_SIZE_IN_ULONGLONGS 2U

/* Size of the image state in dwords. The last 64-bit word is optional for
 * non-YUV textures.
 */
#define ROGUE_MAXIMUM_IMAGE_STATE_SIZE             \
   (ROGUE_MAXIMUM_IMAGE_STATE_SIZE_IN_ULONGLONGS * \
    (sizeof(uint64_t) / sizeof(uint32_t)))

#define PVR_NUM_PBE_EMIT_REGS 8U

#endif /* ROGUE_HW_DEFS_H */
