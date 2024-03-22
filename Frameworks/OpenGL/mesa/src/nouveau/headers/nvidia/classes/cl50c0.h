/*
 * Copyright (c) 2003-2004, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _cl_nv50_compute_h_
#define _cl_nv50_compute_h_

/* This file is generated - do not edit. */

#include "nvtypes.h"

#define NV50_COMPUTE    0x50C0

#define NV50C0_SET_OBJECT                                                                                  0x0000
#define NV50C0_SET_OBJECT_POINTER                                                                            15:0

#define NV50C0_NO_OPERATION                                                                                0x0100
#define NV50C0_NO_OPERATION_V                                                                                31:0

#define NV50C0_NOTIFY                                                                                      0x0104
#define NV50C0_NOTIFY_TYPE                                                                                   31:0
#define NV50C0_NOTIFY_TYPE_WRITE_ONLY                                                                  0x00000000
#define NV50C0_NOTIFY_TYPE_WRITE_THEN_AWAKEN                                                           0x00000001

#define NV50C0_WAIT_FOR_IDLE                                                                               0x0110
#define NV50C0_WAIT_FOR_IDLE_V                                                                               31:0

#define NV50C0_PM_TRIGGER                                                                                  0x0140
#define NV50C0_PM_TRIGGER_V                                                                                  31:0

#define NV50C0_SET_CONTEXT_DMA_NOTIFY                                                                      0x0180
#define NV50C0_SET_CONTEXT_DMA_NOTIFY_HANDLE                                                                 31:0

#define NV50C0_SET_CTX_DMA_GLOBAL_MEM                                                                      0x01a0
#define NV50C0_SET_CTX_DMA_GLOBAL_MEM_HANDLE                                                                 31:0

#define NV50C0_SET_CTX_DMA_SEMAPHORE                                                                       0x01a4
#define NV50C0_SET_CTX_DMA_SEMAPHORE_HANDLE                                                                  31:0

#define NV50C0_SET_CTX_DMA_SHADER_THREAD_MEMORY                                                            0x01b8
#define NV50C0_SET_CTX_DMA_SHADER_THREAD_MEMORY_HANDLE                                                       31:0

#define NV50C0_SET_CTX_DMA_SHADER_THREAD_STACK                                                             0x01bc
#define NV50C0_SET_CTX_DMA_SHADER_THREAD_STACK_HANDLE                                                        31:0

#define NV50C0_SET_CTX_DMA_SHADER_PROGRAM                                                                  0x01c0
#define NV50C0_SET_CTX_DMA_SHADER_PROGRAM_HANDLE                                                             31:0

#define NV50C0_SET_CTX_DMA_TEXTURE_SAMPLER                                                                 0x01c4
#define NV50C0_SET_CTX_DMA_TEXTURE_SAMPLER_HANDLE                                                            31:0

#define NV50C0_SET_CTX_DMA_TEXTURE_HEADERS                                                                 0x01c8
#define NV50C0_SET_CTX_DMA_TEXTURE_HEADERS_HANDLE                                                            31:0

#define NV50C0_SET_CTX_DMA_TEXTURE                                                                         0x01cc
#define NV50C0_SET_CTX_DMA_TEXTURE_HANDLE                                                                    31:0

#define NV50C0_DECRYPTION_CONTROL(j)                                                              (0x0200+(j)*16)
#define NV50C0_DECRYPTION_CONTROL_ALGORITHM                                                                  15:0
#define NV50C0_DECRYPTION_CONTROL_ALGORITHM_NV17_COMPATIBLE                                            0x00000000
#define NV50C0_DECRYPTION_CONTROL_KEY_COUNT                                                                 23:16

#define NV50C0_DECRYPTION_QUERY_SESSION_KEY(j)                                                    (0x0204+(j)*16)
#define NV50C0_DECRYPTION_QUERY_SESSION_KEY_V                                                                31:0

#define NV50C0_DECRYPTION_GET_SESSION_KEY(j)                                                      (0x0208+(j)*16)
#define NV50C0_DECRYPTION_GET_SESSION_KEY_V                                                                  31:0

#define NV50C0_DECRYPTION_SET_ENCRYPTION(j)                                                       (0x020c+(j)*16)
#define NV50C0_DECRYPTION_SET_ENCRYPTION_V                                                                   31:0

#define NV50C0_SET_CTA_PROGRAM_A                                                                           0x0210
#define NV50C0_SET_CTA_PROGRAM_A_OFFSET_UPPER                                                                 7:0

#define NV50C0_SET_CTA_PROGRAM_B                                                                           0x0214
#define NV50C0_SET_CTA_PROGRAM_B_OFFSET_LOWER                                                                31:0

#define NV50C0_SET_SHADER_THREAD_STACK_A                                                                   0x0218
#define NV50C0_SET_SHADER_THREAD_STACK_A_OFFSET_UPPER                                                         7:0

#define NV50C0_SET_SHADER_THREAD_STACK_B                                                                   0x021c
#define NV50C0_SET_SHADER_THREAD_STACK_B_OFFSET_LOWER                                                        31:0

#define NV50C0_SET_SHADER_THREAD_STACK_C                                                                   0x0220
#define NV50C0_SET_SHADER_THREAD_STACK_C_SIZE                                                                 3:0
#define NV50C0_SET_SHADER_THREAD_STACK_C_SIZE__0                                                       0x00000000
#define NV50C0_SET_SHADER_THREAD_STACK_C_SIZE__1                                                       0x00000001
#define NV50C0_SET_SHADER_THREAD_STACK_C_SIZE__2                                                       0x00000002
#define NV50C0_SET_SHADER_THREAD_STACK_C_SIZE__4                                                       0x00000003
#define NV50C0_SET_SHADER_THREAD_STACK_C_SIZE__8                                                       0x00000004
#define NV50C0_SET_SHADER_THREAD_STACK_C_SIZE__16                                                      0x00000005
#define NV50C0_SET_SHADER_THREAD_STACK_C_SIZE__32                                                      0x00000006
#define NV50C0_SET_SHADER_THREAD_STACK_C_SIZE__64                                                      0x00000007
#define NV50C0_SET_SHADER_THREAD_STACK_C_SIZE__128                                                     0x00000008
#define NV50C0_SET_SHADER_THREAD_STACK_C_SIZE__256                                                     0x00000009
#define NV50C0_SET_SHADER_THREAD_STACK_C_SIZE__512                                                     0x0000000A
#define NV50C0_SET_SHADER_THREAD_STACK_C_SIZE__1024                                                    0x0000000B
#define NV50C0_SET_SHADER_THREAD_STACK_C_SIZE__2048                                                    0x0000000C
#define NV50C0_SET_SHADER_THREAD_STACK_C_SIZE__4096                                                    0x0000000D

#define NV50C0_SET_API_CALL_LIMIT                                                                          0x0224
#define NV50C0_SET_API_CALL_LIMIT_CTA                                                                         3:0
#define NV50C0_SET_API_CALL_LIMIT_CTA__0                                                               0x00000000
#define NV50C0_SET_API_CALL_LIMIT_CTA__1                                                               0x00000001
#define NV50C0_SET_API_CALL_LIMIT_CTA__2                                                               0x00000002
#define NV50C0_SET_API_CALL_LIMIT_CTA__4                                                               0x00000003
#define NV50C0_SET_API_CALL_LIMIT_CTA__8                                                               0x00000004
#define NV50C0_SET_API_CALL_LIMIT_CTA__16                                                              0x00000005
#define NV50C0_SET_API_CALL_LIMIT_CTA__32                                                              0x00000006
#define NV50C0_SET_API_CALL_LIMIT_CTA__64                                                              0x00000007
#define NV50C0_SET_API_CALL_LIMIT_CTA__128                                                             0x00000008
#define NV50C0_SET_API_CALL_LIMIT_CTA_NO_CHECK                                                         0x0000000F

#define NV50C0_SET_SHADER_L1_CACHE_CONTROL                                                                 0x0228
#define NV50C0_SET_SHADER_L1_CACHE_CONTROL_ICACHE_PREFETCH_ENABLE                                             0:0
#define NV50C0_SET_SHADER_L1_CACHE_CONTROL_ICACHE_PREFETCH_ENABLE_FALSE                                0x00000000
#define NV50C0_SET_SHADER_L1_CACHE_CONTROL_ICACHE_PREFETCH_ENABLE_TRUE                                 0x00000001
#define NV50C0_SET_SHADER_L1_CACHE_CONTROL_ICACHE_PIXEL_ASSOCIATIVITY                                         7:4
#define NV50C0_SET_SHADER_L1_CACHE_CONTROL_ICACHE_NONPIXEL_ASSOCIATIVITY                                     11:8
#define NV50C0_SET_SHADER_L1_CACHE_CONTROL_DCACHE_PIXEL_ASSOCIATIVITY                                       15:12
#define NV50C0_SET_SHADER_L1_CACHE_CONTROL_DCACHE_NONPIXEL_ASSOCIATIVITY                                    19:16

#define NV50C0_SET_TEX_SAMPLER_POOL_A                                                                      0x022c
#define NV50C0_SET_TEX_SAMPLER_POOL_A_OFFSET_UPPER                                                            7:0

#define NV50C0_SET_TEX_SAMPLER_POOL_B                                                                      0x0230
#define NV50C0_SET_TEX_SAMPLER_POOL_B_OFFSET_LOWER                                                           31:0

#define NV50C0_SET_TEX_SAMPLER_POOL_C                                                                      0x0234
#define NV50C0_SET_TEX_SAMPLER_POOL_C_MAXIMUM_INDEX                                                          19:0

#define NV50C0_LOAD_CONSTANT_SELECTOR                                                                      0x0238
#define NV50C0_LOAD_CONSTANT_SELECTOR_TABLE_INDEX                                                             7:0
#define NV50C0_LOAD_CONSTANT_SELECTOR_CONSTANT_INDEX                                                         23:8

#define NV50C0_LOAD_CONSTANT(i)                                                                    (0x023c+(i)*4)
#define NV50C0_LOAD_CONSTANT_V                                                                               31:0

#define NV50C0_INVALIDATE_SAMPLER_CACHE                                                                    0x027c
#define NV50C0_INVALIDATE_SAMPLER_CACHE_LINES                                                                 0:0
#define NV50C0_INVALIDATE_SAMPLER_CACHE_LINES_ALL                                                      0x00000000
#define NV50C0_INVALIDATE_SAMPLER_CACHE_LINES_ONE                                                      0x00000001
#define NV50C0_INVALIDATE_SAMPLER_CACHE_TAG                                                                  25:4

#define NV50C0_INVALIDATE_TEXTURE_HEADER_CACHE                                                             0x0280
#define NV50C0_INVALIDATE_TEXTURE_HEADER_CACHE_LINES                                                          0:0
#define NV50C0_INVALIDATE_TEXTURE_HEADER_CACHE_LINES_ALL                                               0x00000000
#define NV50C0_INVALIDATE_TEXTURE_HEADER_CACHE_LINES_ONE                                               0x00000001
#define NV50C0_INVALIDATE_TEXTURE_HEADER_CACHE_TAG                                                           25:4

#define NV50C0_SET_SM_TIMEOUT_INTERVAL                                                                     0x0288
#define NV50C0_SET_SM_TIMEOUT_INTERVAL_COUNTER_BIT                                                            5:0

#define NV50C0_TEST_FOR_COMPUTE                                                                            0x028c
#define NV50C0_TEST_FOR_COMPUTE_V                                                                            31:0

#define NV50C0_SET_SHADER_SCHEDULING                                                                       0x0290
#define NV50C0_SET_SHADER_SCHEDULING_MODE                                                                     0:0
#define NV50C0_SET_SHADER_SCHEDULING_MODE_OLDEST_THREAD_FIRST                                          0x00000000
#define NV50C0_SET_SHADER_SCHEDULING_MODE_ROUND_ROBIN                                                  0x00000001

#define NV50C0_SET_SHADER_THREAD_MEMORY_A                                                                  0x0294
#define NV50C0_SET_SHADER_THREAD_MEMORY_A_OFFSET_UPPER                                                        7:0

#define NV50C0_SET_SHADER_THREAD_MEMORY_B                                                                  0x0298
#define NV50C0_SET_SHADER_THREAD_MEMORY_B_OFFSET_LOWER                                                       31:0

#define NV50C0_SET_SHADER_THREAD_MEMORY_C                                                                  0x029c
#define NV50C0_SET_SHADER_THREAD_MEMORY_C_SIZE                                                                3:0
#define NV50C0_SET_SHADER_THREAD_MEMORY_C_SIZE__0                                                      0x00000000
#define NV50C0_SET_SHADER_THREAD_MEMORY_C_SIZE__1                                                      0x00000001
#define NV50C0_SET_SHADER_THREAD_MEMORY_C_SIZE__2                                                      0x00000002
#define NV50C0_SET_SHADER_THREAD_MEMORY_C_SIZE__4                                                      0x00000003
#define NV50C0_SET_SHADER_THREAD_MEMORY_C_SIZE__8                                                      0x00000004
#define NV50C0_SET_SHADER_THREAD_MEMORY_C_SIZE__16                                                     0x00000005
#define NV50C0_SET_SHADER_THREAD_MEMORY_C_SIZE__32                                                     0x00000006
#define NV50C0_SET_SHADER_THREAD_MEMORY_C_SIZE__64                                                     0x00000007
#define NV50C0_SET_SHADER_THREAD_MEMORY_C_SIZE__128                                                    0x00000008
#define NV50C0_SET_SHADER_THREAD_MEMORY_C_SIZE__256                                                    0x00000009
#define NV50C0_SET_SHADER_THREAD_MEMORY_C_SIZE__512                                                    0x0000000A
#define NV50C0_SET_SHADER_THREAD_MEMORY_C_SIZE__1024                                                   0x0000000B
#define NV50C0_SET_SHADER_THREAD_MEMORY_C_SIZE__2048                                                   0x0000000C
#define NV50C0_SET_SHADER_THREAD_MEMORY_C_SIZE__4096                                                   0x0000000D

#define NV50C0_SET_WORK_DISTRIBUTION                                                                       0x02a0
#define NV50C0_SET_WORK_DISTRIBUTION_V                                                                        3:0
#define NV50C0_SET_WORK_DISTRIBUTION_V_HARDWARE_POLICY                                                 0x00000000
#define NV50C0_SET_WORK_DISTRIBUTION_V_WIDE_DYNAMIC                                                    0x00000001
#define NV50C0_SET_WORK_DISTRIBUTION_V_DEEP_DYNAMIC                                                    0x00000002
#define NV50C0_SET_WORK_DISTRIBUTION_V_WIDE_FIXED                                                      0x00000003
#define NV50C0_SET_WORK_DISTRIBUTION_V_DEEP_FIXED                                                      0x00000004
#define NV50C0_SET_WORK_DISTRIBUTION_V_FILL_WIDE_DYNAMIC                                               0x00000005
#define NV50C0_SET_WORK_DISTRIBUTION_V_FILL_DEEP_DYNAMIC                                               0x00000006
#define NV50C0_SET_WORK_DISTRIBUTION_V_FILL_WIDE_FIXED                                                 0x00000007
#define NV50C0_SET_WORK_DISTRIBUTION_V_FILL_DEEP_FIXED                                                 0x00000008

#define NV50C0_LOAD_CONSTANT_BUFFER_TABLE_A                                                                0x02a4
#define NV50C0_LOAD_CONSTANT_BUFFER_TABLE_A_OFFSET_UPPER                                                      7:0

#define NV50C0_LOAD_CONSTANT_BUFFER_TABLE_B                                                                0x02a8
#define NV50C0_LOAD_CONSTANT_BUFFER_TABLE_B_OFFSET_LOWER                                                     31:0

#define NV50C0_LOAD_CONSTANT_BUFFER_TABLE_C                                                                0x02ac
#define NV50C0_LOAD_CONSTANT_BUFFER_TABLE_C_SIZE                                                             15:0
#define NV50C0_LOAD_CONSTANT_BUFFER_TABLE_C_ENTRY                                                           23:16

#define NV50C0_SET_SHADER_ERROR_TRAP_CONTROL                                                               0x02b0
#define NV50C0_SET_SHADER_ERROR_TRAP_CONTROL_MASTER_MASK                                                      0:0
#define NV50C0_SET_SHADER_ERROR_TRAP_CONTROL_MASTER_MASK_FALSE                                         0x00000000
#define NV50C0_SET_SHADER_ERROR_TRAP_CONTROL_MASTER_MASK_TRUE                                          0x00000001
#define NV50C0_SET_SHADER_ERROR_TRAP_CONTROL_SUBSET_MASK                                                     31:1

#define NV50C0_SET_CTA_RESOURCE_ALLOCATION                                                                 0x02b4
#define NV50C0_SET_CTA_RESOURCE_ALLOCATION_THREAD_COUNT                                                      15:0
#define NV50C0_SET_CTA_RESOURCE_ALLOCATION_BARRIER_COUNT                                                    23:16

#define NV50C0_SET_CTA_THREAD_CONTROL                                                                      0x02b8
#define NV50C0_SET_CTA_THREAD_CONTROL_ALLOW_CONVOY_LAUNCH                                                     0:0
#define NV50C0_SET_CTA_THREAD_CONTROL_ALLOW_CONVOY_LAUNCH_FALSE                                        0x00000000
#define NV50C0_SET_CTA_THREAD_CONTROL_ALLOW_CONVOY_LAUNCH_TRUE                                         0x00000001

#define NV50C0_SET_PHASE_ID_CONTROL                                                                        0x02bc
#define NV50C0_SET_PHASE_ID_CONTROL_WINDOW_SIZE                                                               2:0
#define NV50C0_SET_PHASE_ID_CONTROL_LOCK_PHASE                                                                6:4

#define NV50C0_SET_CTA_REGISTER_COUNT                                                                      0x02c0
#define NV50C0_SET_CTA_REGISTER_COUNT_V                                                                       7:0

#define NV50C0_SET_TEX_HEADER_POOL_A                                                                       0x02c4
#define NV50C0_SET_TEX_HEADER_POOL_A_OFFSET_UPPER                                                             7:0

#define NV50C0_SET_TEX_HEADER_POOL_B                                                                       0x02c8
#define NV50C0_SET_TEX_HEADER_POOL_B_OFFSET_LOWER                                                            31:0

#define NV50C0_SET_TEX_HEADER_POOL_C                                                                       0x02cc
#define NV50C0_SET_TEX_HEADER_POOL_C_MAXIMUM_INDEX                                                           21:0

#define NV50C0_SET_SHADER_PERFORMANCE_COUNTER_VALUE(i)                                             (0x02d0+(i)*4)
#define NV50C0_SET_SHADER_PERFORMANCE_COUNTER_VALUE_V                                                        31:0

#define NV50C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL(i)                                           (0x02e0+(i)*4)
#define NV50C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_EDGE                                                    0:0
#define NV50C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_BLOCK                                                   6:4
#define NV50C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_BLOCK_ACE                                        0x00000000
#define NV50C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_BLOCK_DIS                                        0x00000001
#define NV50C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_BLOCK_DSM                                        0x00000002
#define NV50C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_BLOCK_PIC                                        0x00000003
#define NV50C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_BLOCK_STP                                        0x00000004
#define NV50C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_BLOCK_XIU                                        0x00000005
#define NV50C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_FUNC                                                   23:8
#define NV50C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_EVENT                                                 31:24

#define NV50C0_SET_SHADER_PERFORMANCE_COUNTER_TRAP_CONTROL                                                 0x02f0
#define NV50C0_SET_SHADER_PERFORMANCE_COUNTER_TRAP_CONTROL_MASK                                               3:0

#define NV50C0_RESET_CTA_TRACKING_RAM                                                                      0x02f4
#define NV50C0_RESET_CTA_TRACKING_RAM_V                                                                      31:0

#define NV50C0_INITIALIZE                                                                                  0x02f8
#define NV50C0_INITIALIZE_INIT_CTA_SHAPE                                                                      0:0
#define NV50C0_INITIALIZE_INIT_CTA_SHAPE_FALSE                                                         0x00000000
#define NV50C0_INITIALIZE_INIT_CTA_SHAPE_TRUE                                                          0x00000001

#define NV50C0_SET_SHADER_THREAD_MEMORY_THROTTLE                                                           0x02fc
#define NV50C0_SET_SHADER_THREAD_MEMORY_THROTTLE_MAX_TIDS_PER_SM                                              2:0
#define NV50C0_SET_SHADER_THREAD_MEMORY_THROTTLE_MAX_TIDS_PER_SM__1                                    0x00000000
#define NV50C0_SET_SHADER_THREAD_MEMORY_THROTTLE_MAX_TIDS_PER_SM__2                                    0x00000001
#define NV50C0_SET_SHADER_THREAD_MEMORY_THROTTLE_MAX_TIDS_PER_SM__4                                    0x00000002
#define NV50C0_SET_SHADER_THREAD_MEMORY_THROTTLE_MAX_TIDS_PER_SM__8                                    0x00000003
#define NV50C0_SET_SHADER_THREAD_MEMORY_THROTTLE_MAX_TIDS_PER_SM__16                                   0x00000004
#define NV50C0_SET_SHADER_THREAD_MEMORY_THROTTLE_MAX_TIDS_PER_SM_HW_MAX                                0x00000007

#define NV50C0_SET_SHADER_THREAD_MEMORY_THROTTLE_CONTROL                                                   0x0300
#define NV50C0_SET_SHADER_THREAD_MEMORY_THROTTLE_CONTROL_V                                                    2:0
#define NV50C0_SET_SHADER_THREAD_MEMORY_THROTTLE_CONTROL_V_USE_THROTTLE_MAX                            0x00000000
#define NV50C0_SET_SHADER_THREAD_MEMORY_THROTTLE_CONTROL_V_USE_HW_MAX                                  0x00000001

#define NV50C0_SET_SHADER_THREAD_STACK_THROTTLE                                                            0x0304
#define NV50C0_SET_SHADER_THREAD_STACK_THROTTLE_MAX_TIDS_PER_SM                                               2:0
#define NV50C0_SET_SHADER_THREAD_STACK_THROTTLE_MAX_TIDS_PER_SM__1                                     0x00000000
#define NV50C0_SET_SHADER_THREAD_STACK_THROTTLE_MAX_TIDS_PER_SM__2                                     0x00000001
#define NV50C0_SET_SHADER_THREAD_STACK_THROTTLE_MAX_TIDS_PER_SM__4                                     0x00000002
#define NV50C0_SET_SHADER_THREAD_STACK_THROTTLE_MAX_TIDS_PER_SM__8                                     0x00000003
#define NV50C0_SET_SHADER_THREAD_STACK_THROTTLE_MAX_TIDS_PER_SM__16                                    0x00000004
#define NV50C0_SET_SHADER_THREAD_STACK_THROTTLE_MAX_TIDS_PER_SM_HW_MAX                                 0x00000007

#define NV50C0_SET_SHADER_THREAD_STACK_THROTTLE_CONTROL                                                    0x0308
#define NV50C0_SET_SHADER_THREAD_STACK_THROTTLE_CONTROL_V                                                     2:0
#define NV50C0_SET_SHADER_THREAD_STACK_THROTTLE_CONTROL_V_USE_THROTTLE_MAX                             0x00000000
#define NV50C0_SET_SHADER_THREAD_STACK_THROTTLE_CONTROL_V_USE_HW_MAX                                   0x00000001

#define NV50C0_PREFETCH_SHADER_INSTRUCTIONS                                                                0x030c
#define NV50C0_PREFETCH_SHADER_INSTRUCTIONS_CTA                                                               0:0
#define NV50C0_PREFETCH_SHADER_INSTRUCTIONS_CTA_FALSE                                                  0x00000000
#define NV50C0_PREFETCH_SHADER_INSTRUCTIONS_CTA_TRUE                                                   0x00000001

#define NV50C0_SET_REPORT_SEMAPHORE_A                                                                      0x0310
#define NV50C0_SET_REPORT_SEMAPHORE_A_OFFSET_UPPER                                                            7:0

#define NV50C0_SET_REPORT_SEMAPHORE_B                                                                      0x0314
#define NV50C0_SET_REPORT_SEMAPHORE_B_OFFSET_LOWER                                                           31:0

#define NV50C0_SET_REPORT_SEMAPHORE_C                                                                      0x0318
#define NV50C0_SET_REPORT_SEMAPHORE_C_PAYLOAD                                                                31:0

#define NV50C0_SET_REPORT_SEMAPHORE_D                                                                      0x031c
#define NV50C0_SET_REPORT_SEMAPHORE_D_OPERATION                                                               1:0
#define NV50C0_SET_REPORT_SEMAPHORE_D_OPERATION_UNUSED                                                 0x00000000
#define NV50C0_SET_REPORT_SEMAPHORE_D_RELEASE                                                                 2:2
#define NV50C0_SET_REPORT_SEMAPHORE_D_RELEASE_UNUSED                                                   0x00000000
#define NV50C0_SET_REPORT_SEMAPHORE_D_ACQUIRE                                                                 3:3
#define NV50C0_SET_REPORT_SEMAPHORE_D_ACQUIRE_UNUSED                                                   0x00000000
#define NV50C0_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION                                                       7:4
#define NV50C0_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_UNUSED                                         0x00000000
#define NV50C0_SET_REPORT_SEMAPHORE_D_COMPARISON                                                              8:8
#define NV50C0_SET_REPORT_SEMAPHORE_D_COMPARISON_UNUSED                                                0x00000000
#define NV50C0_SET_REPORT_SEMAPHORE_D_AWAKEN_ENABLE                                                           9:9
#define NV50C0_SET_REPORT_SEMAPHORE_D_AWAKEN_ENABLE_FALSE                                              0x00000000
#define NV50C0_SET_REPORT_SEMAPHORE_D_AWAKEN_ENABLE_TRUE                                               0x00000001
#define NV50C0_SET_REPORT_SEMAPHORE_D_REPORT                                                                14:10
#define NV50C0_SET_REPORT_SEMAPHORE_D_REPORT_UNUSED                                                    0x00000000
#define NV50C0_SET_REPORT_SEMAPHORE_D_STRUCTURE_SIZE                                                        15:15
#define NV50C0_SET_REPORT_SEMAPHORE_D_STRUCTURE_SIZE_FOUR_WORDS                                        0x00000000
#define NV50C0_SET_REPORT_SEMAPHORE_D_STRUCTURE_SIZE_ONE_WORD                                          0x00000001

#define NV50C0_SET_LAUNCH_ENABLE_A                                                                         0x0320
#define NV50C0_SET_LAUNCH_ENABLE_A_OFFSET_UPPER                                                               7:0

#define NV50C0_SET_LAUNCH_ENABLE_B                                                                         0x0324
#define NV50C0_SET_LAUNCH_ENABLE_B_OFFSET_LOWER                                                              31:0

#define NV50C0_SET_LAUNCH_ENABLE_C                                                                         0x0328
#define NV50C0_SET_LAUNCH_ENABLE_C_MODE                                                                       2:0
#define NV50C0_SET_LAUNCH_ENABLE_C_MODE_FALSE                                                          0x00000000
#define NV50C0_SET_LAUNCH_ENABLE_C_MODE_TRUE                                                           0x00000001
#define NV50C0_SET_LAUNCH_ENABLE_C_MODE_CONDITIONAL                                                    0x00000002
#define NV50C0_SET_LAUNCH_ENABLE_C_MODE_RENDER_IF_EQUAL                                                0x00000003
#define NV50C0_SET_LAUNCH_ENABLE_C_MODE_RENDER_IF_NOT_EQUAL                                            0x00000004

#define NV50C0_SET_CUBEMAP_ADDRESS_MODE_OVERRIDE                                                           0x032c
#define NV50C0_SET_CUBEMAP_ADDRESS_MODE_OVERRIDE_ENABLE                                                      31:0
#define NV50C0_SET_CUBEMAP_ADDRESS_MODE_OVERRIDE_ENABLE_FALSE                                          0x00000000
#define NV50C0_SET_CUBEMAP_ADDRESS_MODE_OVERRIDE_ENABLE_TRUE                                           0x00000001

#define NV50C0_PIPE_NOP                                                                                    0x0330
#define NV50C0_PIPE_NOP_V                                                                                    31:0

#define NV50C0_SET_SPARE00                                                                                 0x0340
#define NV50C0_SET_SPARE00_V                                                                                 31:0

#define NV50C0_SET_SPARE01                                                                                 0x0344
#define NV50C0_SET_SPARE01_V                                                                                 31:0

#define NV50C0_SET_SPARE02                                                                                 0x0348
#define NV50C0_SET_SPARE02_V                                                                                 31:0

#define NV50C0_SET_SPARE03                                                                                 0x034c
#define NV50C0_SET_SPARE03_V                                                                                 31:0

#define NV50C0_SET_GLOBAL_COLOR_KEY                                                                        0x0358
#define NV50C0_SET_GLOBAL_COLOR_KEY_ENABLE                                                                   31:0
#define NV50C0_SET_GLOBAL_COLOR_KEY_ENABLE_FALSE                                                       0x00000000
#define NV50C0_SET_GLOBAL_COLOR_KEY_ENABLE_TRUE                                                        0x00000001

#define NV50C0_RESET_REF_COUNT                                                                             0x035c
#define NV50C0_RESET_REF_COUNT_REF_CNT                                                                        3:0

#define NV50C0_WAIT_REF_COUNT                                                                              0x0360
#define NV50C0_WAIT_REF_COUNT_COMPARE                                                                         7:4
#define NV50C0_WAIT_REF_COUNT_COMPARE_COUNT_QUIESENT                                                   0x00000000
#define NV50C0_WAIT_REF_COUNT_COMPARE_VALUE_EQUAL                                                      0x00000001
#define NV50C0_WAIT_REF_COUNT_COMPARE_VALUE_CLOCKHAND                                                  0x00000002
#define NV50C0_WAIT_REF_COUNT_REF_CNT                                                                        11:8

#define NV50C0_SET_REF_COUNT_VALUE                                                                         0x0364
#define NV50C0_SET_REF_COUNT_VALUE_V                                                                         31:0

#define NV50C0_LAUNCH                                                                                      0x0368
#define NV50C0_LAUNCH_V                                                                                      31:0

#define NV50C0_SET_LAUNCH_ID                                                                               0x036c
#define NV50C0_SET_LAUNCH_ID_REF_CNT                                                                          3:0

#define NV50C0_SET_LAUNCH_CONTROL                                                                          0x0370
#define NV50C0_SET_LAUNCH_CONTROL_LAUNCH                                                                      7:0
#define NV50C0_SET_LAUNCH_CONTROL_LAUNCH_MANUAL_LAUNCH                                                 0x00000000
#define NV50C0_SET_LAUNCH_CONTROL_LAUNCH_AUTO_LAUNCH                                                   0x00000001

#define NV50C0_SET_PARAMETER_SIZE                                                                          0x0374
#define NV50C0_SET_PARAMETER_SIZE_AUTO_LAUNCH_INDEX                                                           7:0
#define NV50C0_SET_PARAMETER_SIZE_COUNT                                                                      15:8

#define NV50C0_SET_SAMPLER_BINDING                                                                         0x0378
#define NV50C0_SET_SAMPLER_BINDING_V                                                                          0:0
#define NV50C0_SET_SAMPLER_BINDING_V_INDEPENDENTLY                                                     0x00000000
#define NV50C0_SET_SAMPLER_BINDING_V_VIA_HEADER_BINDING                                                0x00000001

#define NV50C0_SET_SHADER_CONTROL                                                                          0x037c
#define NV50C0_SET_SHADER_CONTROL_DEFAULT_PARTIAL                                                             0:0
#define NV50C0_SET_SHADER_CONTROL_DEFAULT_PARTIAL_ZERO                                                 0x00000000
#define NV50C0_SET_SHADER_CONTROL_DEFAULT_PARTIAL_INFINITY                                             0x00000001
#define NV50C0_SET_SHADER_CONTROL_ZERO_TIMES_ANYTHING_IS_ZERO                                               16:16
#define NV50C0_SET_SHADER_CONTROL_ZERO_TIMES_ANYTHING_IS_ZERO_FALSE                                    0x00000000
#define NV50C0_SET_SHADER_CONTROL_ZERO_TIMES_ANYTHING_IS_ZERO_TRUE                                     0x00000001

#define NV50C0_INVALIDATE_SHADER_CACHE                                                                     0x0380
#define NV50C0_INVALIDATE_SHADER_CACHE_V                                                                      1:0
#define NV50C0_INVALIDATE_SHADER_CACHE_V_ALL                                                           0x00000000
#define NV50C0_INVALIDATE_SHADER_CACHE_V_L1                                                            0x00000001
#define NV50C0_INVALIDATE_SHADER_CACHE_V_L1_DATA                                                       0x00000002
#define NV50C0_INVALIDATE_SHADER_CACHE_V_L1_INSTRUCTION                                                0x00000003

#define NV50C0_SET_RASTER_CONTROL                                                                          0x0384
#define NV50C0_SET_RASTER_CONTROL_PROGRAM                                                                     7:0
#define NV50C0_SET_RASTER_CONTROL_PROGRAM_DISABLE                                                      0x00000000
#define NV50C0_SET_RASTER_CONTROL_FIXED                                                                      15:8
#define NV50C0_SET_RASTER_CONTROL_FIXED_DISABLE                                                        0x00000000
#define NV50C0_SET_RASTER_CONTROL_FIXED_SIMPLE                                                         0x00000001
#define NV50C0_SET_RASTER_CONTROL_FIXED_DXVA_RUN_CODED                                                 0x00000002
#define NV50C0_SET_RASTER_CONTROL_DECRYPTION                                                                23:16
#define NV50C0_SET_RASTER_CONTROL_DECRYPTION_DISABLE                                                   0x00000000
#define NV50C0_SET_RASTER_CONTROL_DECRYPTION_ENABLE                                                    0x00000001

#define NV50C0_SET_CTA_FLAGS                                                                               0x0388
#define NV50C0_SET_CTA_FLAGS_V                                                                               15:0

#define NV50C0_SET_CTA_RASTER_SIZE                                                                         0x03a4
#define NV50C0_SET_CTA_RASTER_SIZE_WIDTH                                                                     15:0
#define NV50C0_SET_CTA_RASTER_SIZE_HEIGHT                                                                   31:16

#define NV50C0_SET_CTA_GRF_SIZE                                                                            0x03a8
#define NV50C0_SET_CTA_GRF_SIZE_V                                                                            31:0

#define NV50C0_SET_CTA_THREAD_DIMENSION_A                                                                  0x03ac
#define NV50C0_SET_CTA_THREAD_DIMENSION_A_D0                                                                 15:0
#define NV50C0_SET_CTA_THREAD_DIMENSION_A_D1                                                                31:16

#define NV50C0_SET_CTA_THREAD_DIMENSION_B                                                                  0x03b0
#define NV50C0_SET_CTA_THREAD_DIMENSION_B_D2                                                                 15:0

#define NV50C0_SET_CTA_PROGRAM_START                                                                       0x03b4
#define NV50C0_SET_CTA_PROGRAM_START_OFFSET                                                                  23:0

#define NV50C0_SET_CTA_REGISTER_ALLOCATION                                                                 0x03b8
#define NV50C0_SET_CTA_REGISTER_ALLOCATION_V                                                                 31:0
#define NV50C0_SET_CTA_REGISTER_ALLOCATION_V_THICK                                                     0x00000001
#define NV50C0_SET_CTA_REGISTER_ALLOCATION_V_THIN                                                      0x00000002

#define NV50C0_SET_CTA_TEXTURE                                                                             0x03bc
#define NV50C0_SET_CTA_TEXTURE_MAX_ACTIVE_SAMPLERS                                                            3:0
#define NV50C0_SET_CTA_TEXTURE_MAX_ACTIVE_SAMPLERS__1                                                  0x00000000
#define NV50C0_SET_CTA_TEXTURE_MAX_ACTIVE_SAMPLERS__2                                                  0x00000001
#define NV50C0_SET_CTA_TEXTURE_MAX_ACTIVE_SAMPLERS__4                                                  0x00000002
#define NV50C0_SET_CTA_TEXTURE_MAX_ACTIVE_SAMPLERS__8                                                  0x00000003
#define NV50C0_SET_CTA_TEXTURE_MAX_ACTIVE_SAMPLERS__16                                                 0x00000004
#define NV50C0_SET_CTA_TEXTURE_MAX_ACTIVE_HEADERS                                                             7:4
#define NV50C0_SET_CTA_TEXTURE_MAX_ACTIVE_HEADERS__1                                                   0x00000000
#define NV50C0_SET_CTA_TEXTURE_MAX_ACTIVE_HEADERS__2                                                   0x00000001
#define NV50C0_SET_CTA_TEXTURE_MAX_ACTIVE_HEADERS__4                                                   0x00000002
#define NV50C0_SET_CTA_TEXTURE_MAX_ACTIVE_HEADERS__8                                                   0x00000003
#define NV50C0_SET_CTA_TEXTURE_MAX_ACTIVE_HEADERS__16                                                  0x00000004
#define NV50C0_SET_CTA_TEXTURE_MAX_ACTIVE_HEADERS__32                                                  0x00000005
#define NV50C0_SET_CTA_TEXTURE_MAX_ACTIVE_HEADERS__64                                                  0x00000006
#define NV50C0_SET_CTA_TEXTURE_MAX_ACTIVE_HEADERS__128                                                 0x00000007

#define NV50C0_BIND_CTA_TEXTURE_SAMPLER                                                                    0x03c0
#define NV50C0_BIND_CTA_TEXTURE_SAMPLER_VALID                                                                 0:0
#define NV50C0_BIND_CTA_TEXTURE_SAMPLER_VALID_FALSE                                                    0x00000000
#define NV50C0_BIND_CTA_TEXTURE_SAMPLER_VALID_TRUE                                                     0x00000001
#define NV50C0_BIND_CTA_TEXTURE_SAMPLER_SAMPLER_SLOT                                                         11:4
#define NV50C0_BIND_CTA_TEXTURE_SAMPLER_INDEX                                                               24:12

#define NV50C0_BIND_CTA_TEXTURE_HEADER                                                                     0x03c4
#define NV50C0_BIND_CTA_TEXTURE_HEADER_VALID                                                                  0:0
#define NV50C0_BIND_CTA_TEXTURE_HEADER_VALID_FALSE                                                     0x00000000
#define NV50C0_BIND_CTA_TEXTURE_HEADER_VALID_TRUE                                                      0x00000001
#define NV50C0_BIND_CTA_TEXTURE_HEADER_TEXTURE_SLOT                                                           8:1
#define NV50C0_BIND_CTA_TEXTURE_HEADER_INDEX                                                                 30:9

#define NV50C0_BIND_CONSTANT_BUFFER                                                                        0x03c8
#define NV50C0_BIND_CONSTANT_BUFFER_VALID                                                                     3:0
#define NV50C0_BIND_CONSTANT_BUFFER_VALID_FALSE                                                        0x00000000
#define NV50C0_BIND_CONSTANT_BUFFER_VALID_TRUE                                                         0x00000001
#define NV50C0_BIND_CONSTANT_BUFFER_SHADER_TYPE                                                               7:4
#define NV50C0_BIND_CONSTANT_BUFFER_SHADER_TYPE_CTA                                                    0x00000000
#define NV50C0_BIND_CONSTANT_BUFFER_SHADER_SLOT                                                              11:8
#define NV50C0_BIND_CONSTANT_BUFFER_TABLE_ENTRY                                                             19:12

#define NV50C0_PREFETCH_TEXTURE_SAMPLER                                                                    0x03cc
#define NV50C0_PREFETCH_TEXTURE_SAMPLER_INDEX                                                                21:0

#define NV50C0_INVALIDATE_TEXTURE_DATA_CACHE                                                               0x03d0
#define NV50C0_INVALIDATE_TEXTURE_DATA_CACHE_LEVELS                                                           5:4
#define NV50C0_INVALIDATE_TEXTURE_DATA_CACHE_LEVELS_L1_ONLY                                            0x00000000
#define NV50C0_INVALIDATE_TEXTURE_DATA_CACHE_LEVELS_L2_ONLY                                            0x00000001
#define NV50C0_INVALIDATE_TEXTURE_DATA_CACHE_LEVELS_L1_AND_L2                                          0x00000002

#define NV50C0_SET_SHADER_EXCEPTIONS                                                                       0x03ec
#define NV50C0_SET_SHADER_EXCEPTIONS_ENABLE                                                                   0:0
#define NV50C0_SET_SHADER_EXCEPTIONS_ENABLE_FALSE                                                      0x00000000
#define NV50C0_SET_SHADER_EXCEPTIONS_ENABLE_TRUE                                                       0x00000001

#define NV50C0_SET_GLOBAL_MEM_A(j)                                                                (0x0400+(j)*32)
#define NV50C0_SET_GLOBAL_MEM_A_OFFSET_UPPER                                                                  7:0

#define NV50C0_SET_GLOBAL_MEM_B(j)                                                                (0x0404+(j)*32)
#define NV50C0_SET_GLOBAL_MEM_B_OFFSET_LOWER                                                                 31:0

#define NV50C0_SET_GLOBAL_MEM_SIZE(j)                                                             (0x0408+(j)*32)
#define NV50C0_SET_GLOBAL_MEM_SIZE_BLOCK_PITCH                                                               31:0

#define NV50C0_SET_GLOBAL_MEM_LIMIT(j)                                                            (0x040c+(j)*32)
#define NV50C0_SET_GLOBAL_MEM_LIMIT_MAX                                                                      31:0

#define NV50C0_SET_GLOBAL_MEM_FORMAT(j)                                                           (0x0410+(j)*32)
#define NV50C0_SET_GLOBAL_MEM_FORMAT_MEM_LAYOUT                                                               0:0
#define NV50C0_SET_GLOBAL_MEM_FORMAT_MEM_LAYOUT_BLOCKLINEAR                                            0x00000000
#define NV50C0_SET_GLOBAL_MEM_FORMAT_MEM_LAYOUT_PITCH                                                  0x00000001
#define NV50C0_SET_GLOBAL_MEM_FORMAT_BLOCK_LINEAR_WIDTH                                                       7:4
#define NV50C0_SET_GLOBAL_MEM_FORMAT_BLOCK_LINEAR_WIDTH_ONE_GOB                                        0x00000000
#define NV50C0_SET_GLOBAL_MEM_FORMAT_BLOCK_LINEAR_HEIGHT                                                     11:8
#define NV50C0_SET_GLOBAL_MEM_FORMAT_BLOCK_LINEAR_HEIGHT_ONE_GOB                                       0x00000000
#define NV50C0_SET_GLOBAL_MEM_FORMAT_BLOCK_LINEAR_HEIGHT_TWO_GOBS                                      0x00000001
#define NV50C0_SET_GLOBAL_MEM_FORMAT_BLOCK_LINEAR_HEIGHT_FOUR_GOBS                                     0x00000002
#define NV50C0_SET_GLOBAL_MEM_FORMAT_BLOCK_LINEAR_HEIGHT_EIGHT_GOBS                                    0x00000003
#define NV50C0_SET_GLOBAL_MEM_FORMAT_BLOCK_LINEAR_HEIGHT_SIXTEEN_GOBS                                  0x00000004
#define NV50C0_SET_GLOBAL_MEM_FORMAT_BLOCK_LINEAR_HEIGHT_THIRTYTWO_GOBS                                0x00000005

#define NV50C0_PARAMETER(i)                                                                        (0x0600+(i)*4)
#define NV50C0_PARAMETER_V                                                                                   31:0

#define NV50C0_SET_SPARE_NOOP00                                                                            0x0700
#define NV50C0_SET_SPARE_NOOP00_V                                                                            31:0

#define NV50C0_SET_SPARE_NOOP01                                                                            0x0704
#define NV50C0_SET_SPARE_NOOP01_V                                                                            31:0

#define NV50C0_SET_SPARE_NOOP02                                                                            0x0708
#define NV50C0_SET_SPARE_NOOP02_V                                                                            31:0

#define NV50C0_SET_SPARE_NOOP03                                                                            0x070c
#define NV50C0_SET_SPARE_NOOP03_V                                                                            31:0

#define NV50C0_SET_SPARE_NOOP04                                                                            0x0710
#define NV50C0_SET_SPARE_NOOP04_V                                                                            31:0

#define NV50C0_SET_SPARE_NOOP05                                                                            0x0714
#define NV50C0_SET_SPARE_NOOP05_V                                                                            31:0

#define NV50C0_SET_SPARE_NOOP06                                                                            0x0718
#define NV50C0_SET_SPARE_NOOP06_V                                                                            31:0

#define NV50C0_SET_SPARE_NOOP07                                                                            0x071c
#define NV50C0_SET_SPARE_NOOP07_V                                                                            31:0

#define NV50C0_SET_SPARE_NOOP08                                                                            0x0720
#define NV50C0_SET_SPARE_NOOP08_V                                                                            31:0

#define NV50C0_SET_SPARE_NOOP09                                                                            0x0724
#define NV50C0_SET_SPARE_NOOP09_V                                                                            31:0

#define NV50C0_SET_SPARE_NOOP10                                                                            0x0728
#define NV50C0_SET_SPARE_NOOP10_V                                                                            31:0

#define NV50C0_SET_SPARE_NOOP11                                                                            0x072c
#define NV50C0_SET_SPARE_NOOP11_V                                                                            31:0

#define NV50C0_SET_SPARE_NOOP12                                                                            0x0730
#define NV50C0_SET_SPARE_NOOP12_V                                                                            31:0

#define NV50C0_SET_SPARE_NOOP13                                                                            0x0734
#define NV50C0_SET_SPARE_NOOP13_V                                                                            31:0

#define NV50C0_SET_SPARE_NOOP14                                                                            0x0738
#define NV50C0_SET_SPARE_NOOP14_V                                                                            31:0

#define NV50C0_SET_SPARE_NOOP15                                                                            0x073c
#define NV50C0_SET_SPARE_NOOP15_V                                                                            31:0

#endif /* _cl_nv50_compute_h_ */
