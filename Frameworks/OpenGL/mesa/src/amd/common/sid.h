/*
 * Copyright (C) 2011  Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SID_H
#define SID_H

#include "amdgfxregs.h"

/* si values */
#define SI_CONFIG_REG_OFFSET       0x00008000
#define SI_CONFIG_REG_END          0x0000B000
#define SI_SH_REG_OFFSET           0x0000B000
#define SI_SH_REG_END              0x0000C000
#define SI_CONTEXT_REG_OFFSET      0x00028000
#define SI_CONTEXT_REG_END         0x00030000
#define CIK_UCONFIG_REG_OFFSET     0x00030000
#define CIK_UCONFIG_REG_END        0x00040000
#define SI_UCONFIG_PERF_REG_OFFSET 0x00034000
#define SI_UCONFIG_PERF_REG_END    0x00038000

/* For register shadowing: */
#define SI_SH_REG_SPACE_SIZE           (SI_SH_REG_END - SI_SH_REG_OFFSET)
#define SI_CONTEXT_REG_SPACE_SIZE      (SI_CONTEXT_REG_END - SI_CONTEXT_REG_OFFSET)
#define SI_UCONFIG_REG_SPACE_SIZE      (CIK_UCONFIG_REG_END - CIK_UCONFIG_REG_OFFSET)
#define SI_UCONFIG_PERF_REG_SPACE_SIZE (SI_UCONFIG_PERF_REG_END - SI_UCONFIG_PERF_REG_OFFSET)

#define SI_SHADOWED_SH_REG_OFFSET      0
#define SI_SHADOWED_CONTEXT_REG_OFFSET SI_SH_REG_SPACE_SIZE
#define SI_SHADOWED_UCONFIG_REG_OFFSET (SI_SH_REG_SPACE_SIZE + SI_CONTEXT_REG_SPACE_SIZE)
#define SI_SHADOWED_REG_BUFFER_SIZE                                                                \
   (SI_SH_REG_SPACE_SIZE + SI_CONTEXT_REG_SPACE_SIZE + SI_UCONFIG_REG_SPACE_SIZE)

/* All registers defined in this packet section don't exist and the only
 * purpose of these definitions is to define packet encoding that
 * the IB parser understands, and also to have an accurate documentation.
 */
#define PKT3_NOP                                   0x10
#define PKT3_SET_BASE                              0x11
#define PKT3_CLEAR_STATE                           0x12
#define PKT3_INDEX_BUFFER_SIZE                     0x13
#define PKT3_DISPATCH_DIRECT                       0x15
#define PKT3_DISPATCH_INDIRECT                     0x16
#define PKT3_ATOMIC_MEM                            0x1E
#define   ATOMIC_OP(x)                                ((unsigned)((x)&0x7f) << 0)
#define     TC_OP_ATOMIC_SUB_32                       0x10
#define     TC_OP_ATOMIC_CMPSWAP_32                   0x48
#define   ATOMIC_COMMAND(x)                           ((unsigned)((x)&0x3) << 8)
#define   ATOMIC_COMMAND_SINGLE_PASS                  0x0
#define   ATOMIC_COMMAND_LOOP                         0x1
#define PKT3_OCCLUSION_QUERY                       0x1F /* GFX7+ */
#define PKT3_SET_PREDICATION                       0x20
#define   PREDICATION_DRAW_NOT_VISIBLE                (0 << 8)
#define   PREDICATION_DRAW_VISIBLE                    (1 << 8)
#define   PREDICATION_HINT_WAIT                       (0 << 12)
#define   PREDICATION_HINT_NOWAIT_DRAW                (1 << 12)
#define   PRED_OP(x)                                  ((x) << 16)
#define     PREDICATION_OP_CLEAR                      0x0
#define     PREDICATION_OP_ZPASS                      0x1
#define     PREDICATION_OP_PRIMCOUNT                  0x2
#define     PREDICATION_OP_BOOL64                     0x3
#define     PREDICATION_OP_BOOL32                     0x4
#define   PREDICATION_CONTINUE                        (1 << 31)
#define PKT3_COND_EXEC                             0x22
#define PKT3_PRED_EXEC                             0x23
#define PKT3_DRAW_INDIRECT                         0x24
#define PKT3_DRAW_INDEX_INDIRECT                   0x25
#define PKT3_INDEX_BASE                            0x26
#define PKT3_DRAW_INDEX_2                          0x27
#define PKT3_CONTEXT_CONTROL                       0x28
#define   CC0_LOAD_GLOBAL_CONFIG(x)                   (((unsigned)(x)&0x1) << 0)
#define   CC0_LOAD_PER_CONTEXT_STATE(x)               (((unsigned)(x)&0x1) << 1)
#define   CC0_LOAD_GLOBAL_UCONFIG(x)                  (((unsigned)(x)&0x1) << 15)
#define   CC0_LOAD_GFX_SH_REGS(x)                     (((unsigned)(x)&0x1) << 16)
#define   CC0_LOAD_CS_SH_REGS(x)                      (((unsigned)(x)&0x1) << 24)
#define   CC0_LOAD_CE_RAM(x)                          (((unsigned)(x)&0x1) << 28)
#define   CC0_UPDATE_LOAD_ENABLES(x)                  (((unsigned)(x)&0x1) << 31)
#define   CC1_SHADOW_GLOBAL_CONFIG(x)                 (((unsigned)(x)&0x1) << 0)
#define   CC1_SHADOW_PER_CONTEXT_STATE(x)             (((unsigned)(x)&0x1) << 1)
#define   CC1_SHADOW_GLOBAL_UCONFIG(x)                (((unsigned)(x)&0x1) << 15)
#define   CC1_SHADOW_GFX_SH_REGS(x)                   (((unsigned)(x)&0x1) << 16)
#define   CC1_SHADOW_CS_SH_REGS(x)                    (((unsigned)(x)&0x1) << 24)
#define   CC1_UPDATE_SHADOW_ENABLES(x)                (((unsigned)(x)&0x1) << 31)
#define PKT3_INDEX_TYPE                            0x2A /* GFX6-8 */
#define PKT3_DRAW_INDIRECT_MULTI                   0x2C
#define   R_2C3_DRAW_INDEX_LOC                     0x2C3
#define   S_2C3_COUNT_INDIRECT_ENABLE(x)              (((unsigned)(x)&0x1) << 30)
#define   S_2C3_DRAW_INDEX_ENABLE(x)                  (((unsigned)(x)&0x1) << 31)
#define PKT3_DRAW_INDEX_AUTO                       0x2D
#define PKT3_DRAW_INDEX_IMMD                       0x2E /* GFX6 only */
#define PKT3_NUM_INSTANCES                         0x2F
#define PKT3_DRAW_INDEX_MULTI_AUTO                 0x30
#define PKT3_INDIRECT_BUFFER_SI                    0x32 /* GFX6 only */
#define PKT3_INDIRECT_BUFFER_CONST                 0x33
#define PKT3_STRMOUT_BUFFER_UPDATE                 0x34
#define   STRMOUT_STORE_BUFFER_FILLED_SIZE            1
#define   STRMOUT_OFFSET_SOURCE(x)                    (((unsigned)(x)&0x3) << 1)
#define   STRMOUT_OFFSET_FROM_PACKET                  0
#define   STRMOUT_OFFSET_FROM_VGT_FILLED_SIZE         1
#define   STRMOUT_OFFSET_FROM_MEM                     2
#define   STRMOUT_OFFSET_NONE                         3
#define   STRMOUT_DATA_TYPE(x)                        (((unsigned)(x)&0x1) << 7)
#define   STRMOUT_SELECT_BUFFER(x)                    (((unsigned)(x)&0x3) << 8)
#define PKT3_DRAW_INDEX_OFFSET_2                   0x35
#define PKT3_WRITE_DATA                            0x37
#define PKT3_DRAW_INDEX_INDIRECT_MULTI             0x38
#define PKT3_MEM_SEMAPHORE                         0x39
#define PKT3_MPEG_INDEX                            0x3A /* GFX6 only */
#define PKT3_WAIT_REG_MEM                          0x3C
#define   WAIT_REG_MEM_EQUAL                          3
#define   WAIT_REG_MEM_NOT_EQUAL                      4
#define   WAIT_REG_MEM_GREATER_OR_EQUAL               5
#define   WAIT_REG_MEM_MEM_SPACE(x)                   (((unsigned)(x)&0x3) << 4)
#define   WAIT_REG_MEM_PFP                            (1 << 8)
#define PKT3_MEM_WRITE                             0x3D /* GFX6 only */
#define PKT3_INDIRECT_BUFFER                       0x3F /* GFX6+ */
#define PKT3_COPY_DATA                             0x40
#define   COPY_DATA_SRC_SEL(x)                        ((x)&0xf)
#define   COPY_DATA_REG                               0
#define   COPY_DATA_SRC_MEM                           1 /* only valid as source */
#define   COPY_DATA_TC_L2                             2
#define   COPY_DATA_GDS                               3
#define   COPY_DATA_PERF                              4
#define   COPY_DATA_IMM                               5
#define   COPY_DATA_TIMESTAMP                         9
#define   COPY_DATA_DST_SEL(x)                        (((unsigned)(x)&0xf) << 8)
#define   COPY_DATA_DST_MEM_GRBM                      1 /* sync across GRBM, deprecated */
#define   COPY_DATA_TC_L2                             2
#define   COPY_DATA_GDS                               3
#define   COPY_DATA_PERF                              4
#define   COPY_DATA_DST_MEM                           5
#define   COPY_DATA_COUNT_SEL                         (1 << 16)
#define   COPY_DATA_WR_CONFIRM                        (1 << 20)
#define   COPY_DATA_ENGINE_PFP                        (1 << 30)
/* 1. header
 * 2. SRC_ADDR_LO [31:0] or DATA [31:0]
 * 3. CP_SYNC [31] | SRC_SEL [30:29] | ENGINE [27] | DST_SEL [21:20] | SRC_ADDR_HI [15:0]
 * 4. DST_ADDR_LO [31:0]
 * 5. DST_ADDR_HI [15:0]
 * 6. COMMAND [29:22] | BYTE_COUNT [20:0]
 */
#define PKT3_CP_DMA                                0x41 /* GFX6 only */
#define PKT3_PFP_SYNC_ME                           0x42
#define PKT3_SURFACE_SYNC                          0x43 /* deprecated on GFX7, use ACQUIRE_MEM */
#define PKT3_ME_INITIALIZE                         0x44 /* GFX6 only */
#define PKT3_COND_WRITE                            0x45
#define PKT3_EVENT_WRITE                           0x46
#define   EVENT_TYPE(x)                               ((x) << 0)
/* 0 - any non-TS event
 * 1 - ZPASS_DONE
 * 2 - SAMPLE_PIPELINESTAT
 * 3 - SAMPLE_STREAMOUTSTAT*
 * 4 - *S_PARTIAL_FLUSH
 * 5 - TS events
 */
#define   EVENT_INDEX(x)                              ((x) << 8)
#define   PIXEL_PIPE_STATE_CNTL_COUNTER_ID(x)         ((x) << 3)
#define   PIXEL_PIPE_STATE_CNTL_STRIDE(x)             ((x) << 9)
/* 0 - 32 bits
 * 1 - 64 bits
 * 2 - 128 bits
 * 3 - 256 bits
 */
#define   PIXEL_PIPE_STATE_CNTL_INSTANCE_EN_LO(x)     ((x) << 11)
#define   PIXEL_PIPE_STATE_CNTL_INSTANCE_EN_HI(x)     ((x) >> 21)
#define PKT3_EVENT_WRITE_EOP                       0x47 /* GFX6-8 */
/* EVENT_WRITE_EOP (GFX6-8) & RELEASE_MEM (GFX9) */
#define   EVENT_TCL1_VOL_ACTION_ENA                   (1 << 12)
#define   EVENT_TC_VOL_ACTION_ENA                     (1 << 13)
#define   EVENT_TC_WB_ACTION_ENA                      (1 << 15)
#define   EVENT_TCL1_ACTION_ENA                       (1 << 16)
#define   EVENT_TC_ACTION_ENA                         (1 << 17)
#define   EVENT_TC_NC_ACTION_ENA                      (1 << 19) /* GFX9+ */
#define   EVENT_TC_WC_ACTION_ENA                      (1 << 20) /* GFX9+ */
#define   EVENT_TC_MD_ACTION_ENA                      (1 << 21) /* GFX9+ */
#define   EOP_DST_SEL(x)                              ((x) << 16)
#define     EOP_DST_SEL_MEM                           0
#define     EOP_DST_SEL_TC_L2                         1
#define   EOP_INT_SEL(x)                              ((x) << 24)
#define     EOP_INT_SEL_NONE                          0
#define     EOP_INT_SEL_SEND_DATA_AFTER_WR_CONFIRM    3
#define   EOP_DATA_SEL(x)                             ((x) << 29)
#define     EOP_DATA_SEL_DISCARD                      0
#define     EOP_DATA_SEL_VALUE_32BIT                  1
#define     EOP_DATA_SEL_VALUE_64BIT                  2
#define     EOP_DATA_SEL_TIMESTAMP                    3
#define     EOP_DATA_SEL_GDS                          5
#define   EOP_DATA_GDS(dw_offset, num_dwords)         ((dw_offset) | ((unsigned)(num_dwords) << 16))
#define PKT3_EVENT_WRITE_EOS                       0x48 /* GFX6-8 */
#define   EOS_DATA_SEL(x)                             ((x) << 29)
#define     EOS_DATA_SEL_APPEND_COUNT                 0
#define     EOS_DATA_SEL_GDS                          1
#define     EOS_DATA_SEL_VALUE_32BIT                  2
/* CP DMA bug: Any use of CP_DMA.DST_SEL=TC must be avoided when EOS packets
 * are used. Use DST_SEL=MC instead. For prefetch, use SRC_SEL=TC and
 * DST_SEL=MC. Only GFX7 chips are affected.
 */
#define PKT3_EVENT_WRITE_EOS                       0x48 /* GFX6-8, breaks CP DMA */
#define PKT3_RELEASE_MEM                           0x49 /* GFX9+ [any ring] or GFX8 [compute ring only] */
/* 1. header
 * 2. CP_SYNC [31] | SRC_SEL [30:29] | DST_SEL [21:20] | ENGINE [0]
 * 2. SRC_ADDR_LO [31:0] or DATA [31:0]
 * 3. SRC_ADDR_HI [31:0]
 * 4. DST_ADDR_LO [31:0]
 * 5. DST_ADDR_HI [31:0]
 * 6. COMMAND [29:22] | BYTE_COUNT [20:0]
 */
#define PKT3_DISPATCH_MESH_INDIRECT_MULTI          0x4C /* Indirect mesh shader only dispatch [GFX only], GFX10.3+ */
#define   S_4C1_XYZ_DIM_REG(x)                        ((x & 0xFFFF))
#define   S_4C1_DRAW_INDEX_REG(x)                     ((x & 0xFFFF) << 16)
#define   S_4C2_DRAW_INDEX_ENABLE(x)                  ((x & 1) << 31)
#define   S_4C2_COUNT_INDIRECT_ENABLE(x)              ((x & 1) << 30)
#define   S_4C2_THREAD_TRACE_MARKER_ENABLE(x)         ((x & 1) << 29)
#define   S_4C2_XYZ_DIM_ENABLE(x)                     ((x & 1) << 28) /* GFX11+ */
#define   S_4C2_MODE1_ENABLE(x)                       ((x & 1) << 27) /* GFX11+ */
#define PKT3_DISPATCH_TASKMESH_GFX                 0x4D /* Task + mesh shader dispatch [GFX side], GFX10.3+ */
#define   S_4D0_RING_ENTRY_REG(x)                     ((x & 0xFFFF) << 16)
#define   S_4D0_XYZ_DIM_REG(x)                        ((x & 0xFFFF))
#define   S_4D1_THREAD_TRACE_MARKER_ENABLE(x)         ((x & 1) << 31)
#define   S_4D1_XYZ_DIM_ENABLE(x)                     ((x & 1) << 30) /* GFX11+ */
#define   S_4D1_MODE1_ENABLE(x)                       ((x & 1) << 29) /* GFX11+ */
#define   S_4D1_LINEAR_DISPATCH_ENABLE(x)             ((x & 1) << 28) /* GFX11+ */
#define PKT3_DISPATCH_MESH_DIRECT                  0x4E /* Direct mesh shader only dispatch [GFX only], GFX11+ */
#define PKT3_DMA_DATA                              0x50 /* GFX7+ */
#define PKT3_CONTEXT_REG_RMW                       0x51 /* older firmware versions on older chips don't have this */
#define PKT3_ONE_REG_WRITE                         0x57 /* GFX6 only */
#define PKT3_ACQUIRE_MEM                           0x58 /* GFX7+ */
#define PKT3_REWIND                                0x59 /* GFX8+ [any ring] or GFX7 [compute ring only] */
#define PKT3_PRIME_UTCL2                           0x5D
#define PKT3_LOAD_UCONFIG_REG                      0x5E /* GFX7+ */
#define PKT3_LOAD_SH_REG                           0x5F
#define PKT3_LOAD_CONTEXT_REG                      0x61
#define PKT3_LOAD_SH_REG_INDEX                     0x63 /* GFX8+ */
#define PKT3_SET_CONFIG_REG                        0x68
#define PKT3_SET_CONTEXT_REG                       0x69
#define PKT3_SET_SH_REG                            0x76
#define PKT3_SET_SH_REG_OFFSET                     0x77
#define PKT3_SET_UCONFIG_REG                       0x79 /* GFX7+ */
#define PKT3_SET_UCONFIG_REG_INDEX                 0x7A /* new for GFX9, CP ucode version >= 26 */
#define PKT3_LOAD_CONST_RAM                        0x80
#define PKT3_WRITE_CONST_RAM                       0x81
#define PKT3_DUMP_CONST_RAM                        0x83
#define PKT3_INCREMENT_CE_COUNTER                  0x84
#define PKT3_INCREMENT_DE_COUNTER                  0x85
#define PKT3_WAIT_ON_CE_COUNTER                    0x86
#define PKT3_SET_SH_REG_INDEX                      0x9B
#define PKT3_LOAD_CONTEXT_REG_INDEX                0x9F /* GFX8+ */
#define PKT3_DISPATCH_TASK_STATE_INIT              0xA9 /* Tells the HW about the task control buffer, GFX10.3+ */
#define PKT3_DISPATCH_TASKMESH_DIRECT_ACE          0xAA /* Direct task + mesh shader dispatch [ACE side], GFX10.3+ */
#define PKT3_DISPATCH_TASKMESH_INDIRECT_MULTI_ACE  0xAD /* Indirect task + mesh shader dispatch [ACE side], GFX10.3+ */
#define   S_AD2_RING_ENTRY_REG(x)                     ((x & 0xFFFF))
#define   S_AD3_COUNT_INDIRECT_ENABLE(x)              ((x & 1) << 1)
#define   S_AD3_DRAW_INDEX_ENABLE(x)                  ((x & 1) << 2)
#define   S_AD3_XYZ_DIM_ENABLE(x)                     ((x & 1) << 3)
#define   S_AD3_DRAW_INDEX_REG(x)                     ((x & 0xFFFF) << 16)
#define   S_AD4_XYZ_DIM_REG(x)                        ((x & 0xFFFF))
#define PKT3_EVENT_WRITE_ZPASS                     0xB1 /* GFX11+ & PFP version >= 1458 */
#define   EVENT_WRITE_ZPASS_PFP_VERSION               1458
/* Use these on GFX11 with a high PFP firmware version (only dGPUs should have that, not APUs)
 * because they are the fastest SET packets there.
 *    SET_CONTEXT_REG_PAIRS_PACKED:
 *    SET_SH_REG_PAIRS_PACKED:
 *    SET_SH_REG_PAIRS_PACKED_N:
 *      Format: header, count, (offset0 | (offset1 << 16), value0, value1)^(count / 2)
 *      - "count" is the register count and must be aligned to 2.
 *      - Consecutive offsets must not be equal.
 *      - RESET_FILTER_CAM must be set to 1.
 *      - If the register count is odd, write the first register again at the end to make it even.
 *      - The SH_*_PACKED* variants require register shadowing to be enabled.
 *      - The *_N variant is identical to the non-N variant, but the maximum allowed "count" is 14
 *        and it's faster.
 */
#define PKT3_SET_CONTEXT_REG_PAIRS                 0xB8 /* GFX11+, don't use */
#define PKT3_SET_CONTEXT_REG_PAIRS_PACKED          0xB9 /* GFX11+ */
#define PKT3_SET_SH_REG_PAIRS                      0xBA /* GFX11+, don't use */
#define PKT3_SET_SH_REG_PAIRS_PACKED               0xBB /* GFX11+ */
#define PKT3_SET_SH_REG_PAIRS_PACKED_N             0xBD /* GFX11+ */

#define PKT_TYPE_S(x)         (((unsigned)(x)&0x3) << 30)
#define PKT_TYPE_G(x)         (((x) >> 30) & 0x3)
#define PKT_TYPE_C            0x3FFFFFFF
#define PKT_COUNT_S(x)        (((unsigned)(x)&0x3FFF) << 16)
#define PKT_COUNT_G(x)        (((x) >> 16) & 0x3FFF)
#define PKT_COUNT_C           0xC000FFFF
#define PKT3_IT_OPCODE_S(x)   (((unsigned)(x)&0xFF) << 8)
#define PKT3_IT_OPCODE_G(x)   (((x) >> 8) & 0xFF)
#define PKT3_IT_OPCODE_C      0xFFFF00FF
#define PKT3_PREDICATE(x)     (((x) >> 0) & 0x1)
#define PKT3_SHADER_TYPE_S(x) (((unsigned)(x) & 0x1) << 1)
#define PKT3_SHADER_TYPE_G(x) (((x) >> 1) & 0x1)
#define PKT3_RESET_FILTER_CAM_S(x) (((unsigned)(x) & 0x1) << 2)
#define PKT3_RESET_FILTER_CAM_G(x) (((unsigned)(x) >> 2) & 0x1)
#define PKT3(op, count, predicate)                                                                 \
   (PKT_TYPE_S(3) | PKT_COUNT_S(count) | PKT3_IT_OPCODE_S(op) | PKT3_PREDICATE(predicate))

#define PKT2_NOP_PAD PKT_TYPE_S(2)
#define PKT3_NOP_PAD PKT3(PKT3_NOP, 0x3fff, 0) /* header-only version */

/* SI async DMA packets */
#define SI_DMA_PACKET(cmd, sub_cmd, n)                                                             \
   ((((unsigned)(cmd)&0xF) << 28) | (((unsigned)(sub_cmd)&0xFF) << 20) |                           \
    (((unsigned)(n)&0xFFFFF) << 0))
/* SI async DMA Packet types */
#define SI_DMA_PACKET_WRITE               0x2
#define SI_DMA_PACKET_COPY                0x3
#define SI_DMA_COPY_MAX_BYTE_ALIGNED_SIZE 0xfffe0
/* The documentation says 0xffff8 is the maximum size in dwords, which is
 * 0x3fffe0 in bytes. */
#define SI_DMA_COPY_MAX_DWORD_ALIGNED_SIZE 0x3fffe0
#define SI_DMA_COPY_DWORD_ALIGNED          0x00
#define SI_DMA_COPY_BYTE_ALIGNED           0x40
#define SI_DMA_COPY_TILED                  0x8
#define SI_DMA_PACKET_INDIRECT_BUFFER      0x4
#define SI_DMA_PACKET_SEMAPHORE            0x5
#define SI_DMA_PACKET_FENCE                0x6
#define SI_DMA_PACKET_TRAP                 0x7
#define SI_DMA_PACKET_SRBM_WRITE           0x9
#define SI_DMA_PACKET_CONSTANT_FILL        0xd
#define SI_DMA_PACKET_NOP                  0xf

/* CIK async DMA packets */
#define SDMA_PACKET(op, sub_op, n)                                                                 \
   ((((unsigned)(n)&0xFFFF) << 16) | (((unsigned)(sub_op)&0xFF) << 8) |                            \
    (((unsigned)(op)&0xFF) << 0))
/* CIK async DMA packet types */
#define SDMA_OPCODE_NOP                            0x0
#define SDMA_OPCODE_COPY                           0x1
#define SDMA_COPY_SUB_OPCODE_LINEAR                0x0
#define SDMA_COPY_SUB_OPCODE_TILED                 0x1
#define SDMA_COPY_SUB_OPCODE_SOA                   0x3
#define SDMA_COPY_SUB_OPCODE_LINEAR_SUB_WINDOW     0x4
#define SDMA_COPY_SUB_OPCODE_TILED_SUB_WINDOW      0x5
#define SDMA_COPY_SUB_OPCODE_T2T_SUB_WINDOW        0x6
#define SDMA_OPCODE_WRITE                          0x2
#define SDMA_WRITE_SUB_OPCODE_LINEAR               0x0
#define SDMA_WRITE_SUB_OPCODE_TILED                0x1
#define SDMA_OPCODE_INDIRECT_BUFFER                0x4
#define SDMA_OPCODE_FENCE                          0x5
#define SDMA_FENCE_MTYPE_UC                        0x3
#define SDMA_OPCODE_TRAP                           0x6
#define SDMA_OPCODE_SEMAPHORE                      0x7
#define SDMA_OPCODE_POLL_REGMEM                    0x8
#define SDMA_POLL_MEM                              (1 << 31)
#define SDMA_POLL_INTERVAL_160_CLK                 0xa
#define SDMA_POLL_RETRY_INDEFINITELY               0xfff
#define SDMA_OPCODE_CONSTANT_FILL                  0xb
#define SDMA_OPCODE_TIMESTAMP                      0xd
#define SDMA_TS_SUB_OPCODE_SET_LOCAL_TIMESTAMP     0x0
#define SDMA_TS_SUB_OPCODE_GET_LOCAL_TIMESTAMP     0x1
#define SDMA_TS_SUB_OPCODE_GET_GLOBAL_TIMESTAMP    0x2
#define SDMA_OPCODE_SRBM_WRITE                     0xe

/* There is apparently an undocumented HW limitation that
 * prevents the HW from copying the last 255 bytes of (1 << 22) - 1
 */
#define SDMA_V2_0_COPY_MAX_BYTES 0x3fff00   /* almost 4 MB*/
#define SDMA_V5_2_COPY_MAX_BYTES 0x3fffff00 /* almost 1 GB */

#define SDMA_NOP_PAD SDMA_PACKET(SDMA_OPCODE_NOP, 0, 0) /* header-only version */

enum amd_cmp_class_flags
{
   S_NAN = 1 << 0,       // Signaling NaN
   Q_NAN = 1 << 1,       // Quiet NaN
   N_INFINITY = 1 << 2,  // Negative infinity
   N_NORMAL = 1 << 3,    // Negative normal
   N_SUBNORMAL = 1 << 4, // Negative subnormal
   N_ZERO = 1 << 5,      // Negative zero
   P_ZERO = 1 << 6,      // Positive zero
   P_SUBNORMAL = 1 << 7, // Positive subnormal
   P_NORMAL = 1 << 8,    // Positive normal
   P_INFINITY = 1 << 9   // Positive infinity
};

#endif /* _SID_H */
