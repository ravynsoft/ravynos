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

#ifndef _cl_fermi_compute_b_h_
#define _cl_fermi_compute_b_h_

/* AUTO GENERATED FILE -- DO NOT EDIT */
/* Command: ../../class/bin/sw_header.pl fermi_compute_b */

#include "nvtypes.h"

#define FERMI_COMPUTE_B    0x91C0

#define NV91C0_SET_OBJECT                                                                                  0x0000
#define NV91C0_SET_OBJECT_CLASS_ID                                                                           15:0
#define NV91C0_SET_OBJECT_ENGINE_ID                                                                         20:16

#define NV91C0_NO_OPERATION                                                                                0x0100
#define NV91C0_NO_OPERATION_V                                                                                31:0

#define NV91C0_SET_NOTIFY_A                                                                                0x0104
#define NV91C0_SET_NOTIFY_A_ADDRESS_UPPER                                                                     7:0

#define NV91C0_SET_NOTIFY_B                                                                                0x0108
#define NV91C0_SET_NOTIFY_B_ADDRESS_LOWER                                                                    31:0

#define NV91C0_NOTIFY                                                                                      0x010c
#define NV91C0_NOTIFY_TYPE                                                                                   31:0
#define NV91C0_NOTIFY_TYPE_WRITE_ONLY                                                                  0x00000000
#define NV91C0_NOTIFY_TYPE_WRITE_THEN_AWAKEN                                                           0x00000001

#define NV91C0_WAIT_FOR_IDLE                                                                               0x0110
#define NV91C0_WAIT_FOR_IDLE_V                                                                               31:0

#define NV91C0_LOAD_MME_INSTRUCTION_RAM_POINTER                                                            0x0114
#define NV91C0_LOAD_MME_INSTRUCTION_RAM_POINTER_V                                                            31:0

#define NV91C0_LOAD_MME_INSTRUCTION_RAM                                                                    0x0118
#define NV91C0_LOAD_MME_INSTRUCTION_RAM_V                                                                    31:0

#define NV91C0_LOAD_MME_START_ADDRESS_RAM_POINTER                                                          0x011c
#define NV91C0_LOAD_MME_START_ADDRESS_RAM_POINTER_V                                                          31:0

#define NV91C0_LOAD_MME_START_ADDRESS_RAM                                                                  0x0120
#define NV91C0_LOAD_MME_START_ADDRESS_RAM_V                                                                  31:0

#define NV91C0_SET_MME_SHADOW_RAM_CONTROL                                                                  0x0124
#define NV91C0_SET_MME_SHADOW_RAM_CONTROL_MODE                                                                1:0
#define NV91C0_SET_MME_SHADOW_RAM_CONTROL_MODE_METHOD_TRACK                                            0x00000000
#define NV91C0_SET_MME_SHADOW_RAM_CONTROL_MODE_METHOD_TRACK_WITH_FILTER                                0x00000001
#define NV91C0_SET_MME_SHADOW_RAM_CONTROL_MODE_METHOD_PASSTHROUGH                                      0x00000002
#define NV91C0_SET_MME_SHADOW_RAM_CONTROL_MODE_METHOD_REPLAY                                           0x00000003

#define NV91C0_SET_GLOBAL_RENDER_ENABLE_A                                                                  0x0130
#define NV91C0_SET_GLOBAL_RENDER_ENABLE_A_OFFSET_UPPER                                                        7:0

#define NV91C0_SET_GLOBAL_RENDER_ENABLE_B                                                                  0x0134
#define NV91C0_SET_GLOBAL_RENDER_ENABLE_B_OFFSET_LOWER                                                       31:0

#define NV91C0_SET_GLOBAL_RENDER_ENABLE_C                                                                  0x0138
#define NV91C0_SET_GLOBAL_RENDER_ENABLE_C_MODE                                                                2:0
#define NV91C0_SET_GLOBAL_RENDER_ENABLE_C_MODE_FALSE                                                   0x00000000
#define NV91C0_SET_GLOBAL_RENDER_ENABLE_C_MODE_TRUE                                                    0x00000001
#define NV91C0_SET_GLOBAL_RENDER_ENABLE_C_MODE_CONDITIONAL                                             0x00000002
#define NV91C0_SET_GLOBAL_RENDER_ENABLE_C_MODE_RENDER_IF_EQUAL                                         0x00000003
#define NV91C0_SET_GLOBAL_RENDER_ENABLE_C_MODE_RENDER_IF_NOT_EQUAL                                     0x00000004

#define NV91C0_SEND_GO_IDLE                                                                                0x013c
#define NV91C0_SEND_GO_IDLE_V                                                                                31:0

#define NV91C0_PM_TRIGGER                                                                                  0x0140
#define NV91C0_PM_TRIGGER_V                                                                                  31:0

#define NV91C0_SET_INSTRUMENTATION_METHOD_HEADER                                                           0x0150
#define NV91C0_SET_INSTRUMENTATION_METHOD_HEADER_V                                                           31:0

#define NV91C0_SET_INSTRUMENTATION_METHOD_DATA                                                             0x0154
#define NV91C0_SET_INSTRUMENTATION_METHOD_DATA_V                                                             31:0

#define NV91C0_SET_SHADER_LOCAL_MEMORY_LOW_SIZE                                                            0x0204
#define NV91C0_SET_SHADER_LOCAL_MEMORY_LOW_SIZE_V                                                            23:0

#define NV91C0_SET_SHADER_LOCAL_MEMORY_HIGH_SIZE                                                           0x0208
#define NV91C0_SET_SHADER_LOCAL_MEMORY_HIGH_SIZE_V                                                           23:0

#define NV91C0_SET_SHADER_LOCAL_MEMORY_CRS_SIZE                                                            0x020c
#define NV91C0_SET_SHADER_LOCAL_MEMORY_CRS_SIZE_V                                                            20:0

#define NV91C0_SET_BINDING_CONTROL_TEXTURE                                                                 0x0210
#define NV91C0_SET_BINDING_CONTROL_TEXTURE_MAX_ACTIVE_SAMPLERS                                                3:0
#define NV91C0_SET_BINDING_CONTROL_TEXTURE_MAX_ACTIVE_SAMPLERS__1                                      0x00000000
#define NV91C0_SET_BINDING_CONTROL_TEXTURE_MAX_ACTIVE_SAMPLERS__2                                      0x00000001
#define NV91C0_SET_BINDING_CONTROL_TEXTURE_MAX_ACTIVE_SAMPLERS__4                                      0x00000002
#define NV91C0_SET_BINDING_CONTROL_TEXTURE_MAX_ACTIVE_SAMPLERS__8                                      0x00000003
#define NV91C0_SET_BINDING_CONTROL_TEXTURE_MAX_ACTIVE_SAMPLERS__16                                     0x00000004
#define NV91C0_SET_BINDING_CONTROL_TEXTURE_MAX_ACTIVE_HEADERS                                                 7:4
#define NV91C0_SET_BINDING_CONTROL_TEXTURE_MAX_ACTIVE_HEADERS__1                                       0x00000000
#define NV91C0_SET_BINDING_CONTROL_TEXTURE_MAX_ACTIVE_HEADERS__2                                       0x00000001
#define NV91C0_SET_BINDING_CONTROL_TEXTURE_MAX_ACTIVE_HEADERS__4                                       0x00000002
#define NV91C0_SET_BINDING_CONTROL_TEXTURE_MAX_ACTIVE_HEADERS__8                                       0x00000003
#define NV91C0_SET_BINDING_CONTROL_TEXTURE_MAX_ACTIVE_HEADERS__16                                      0x00000004
#define NV91C0_SET_BINDING_CONTROL_TEXTURE_MAX_ACTIVE_HEADERS__32                                      0x00000005
#define NV91C0_SET_BINDING_CONTROL_TEXTURE_MAX_ACTIVE_HEADERS__64                                      0x00000006
#define NV91C0_SET_BINDING_CONTROL_TEXTURE_MAX_ACTIVE_HEADERS__128                                     0x00000007

#define NV91C0_SET_SHADER_SHARED_MEMORY_WINDOW                                                             0x0214
#define NV91C0_SET_SHADER_SHARED_MEMORY_WINDOW_BASE_ADDRESS                                                  31:0

#define NV91C0_INVALIDATE_SHADER_CACHES                                                                    0x021c
#define NV91C0_INVALIDATE_SHADER_CACHES_INSTRUCTION                                                           0:0
#define NV91C0_INVALIDATE_SHADER_CACHES_INSTRUCTION_FALSE                                              0x00000000
#define NV91C0_INVALIDATE_SHADER_CACHES_INSTRUCTION_TRUE                                               0x00000001
#define NV91C0_INVALIDATE_SHADER_CACHES_DATA                                                                  4:4
#define NV91C0_INVALIDATE_SHADER_CACHES_DATA_FALSE                                                     0x00000000
#define NV91C0_INVALIDATE_SHADER_CACHES_DATA_TRUE                                                      0x00000001
#define NV91C0_INVALIDATE_SHADER_CACHES_UNIFORM                                                               8:8
#define NV91C0_INVALIDATE_SHADER_CACHES_UNIFORM_FALSE                                                  0x00000000
#define NV91C0_INVALIDATE_SHADER_CACHES_UNIFORM_TRUE                                                   0x00000001
#define NV91C0_INVALIDATE_SHADER_CACHES_CONSTANT                                                            12:12
#define NV91C0_INVALIDATE_SHADER_CACHES_CONSTANT_FALSE                                                 0x00000000
#define NV91C0_INVALIDATE_SHADER_CACHES_CONSTANT_TRUE                                                  0x00000001
#define NV91C0_INVALIDATE_SHADER_CACHES_LOCKS                                                                 1:1
#define NV91C0_INVALIDATE_SHADER_CACHES_LOCKS_FALSE                                                    0x00000000
#define NV91C0_INVALIDATE_SHADER_CACHES_LOCKS_TRUE                                                     0x00000001
#define NV91C0_INVALIDATE_SHADER_CACHES_FLUSH_DATA                                                            2:2
#define NV91C0_INVALIDATE_SHADER_CACHES_FLUSH_DATA_FALSE                                               0x00000000
#define NV91C0_INVALIDATE_SHADER_CACHES_FLUSH_DATA_TRUE                                                0x00000001

#define NV91C0_BIND_TEXTURE_SAMPLER                                                                        0x0228
#define NV91C0_BIND_TEXTURE_SAMPLER_VALID                                                                     0:0
#define NV91C0_BIND_TEXTURE_SAMPLER_VALID_FALSE                                                        0x00000000
#define NV91C0_BIND_TEXTURE_SAMPLER_VALID_TRUE                                                         0x00000001
#define NV91C0_BIND_TEXTURE_SAMPLER_SAMPLER_SLOT                                                             11:4
#define NV91C0_BIND_TEXTURE_SAMPLER_INDEX                                                                   24:12

#define NV91C0_BIND_TEXTURE_HEADER                                                                         0x022c
#define NV91C0_BIND_TEXTURE_HEADER_VALID                                                                      0:0
#define NV91C0_BIND_TEXTURE_HEADER_VALID_FALSE                                                         0x00000000
#define NV91C0_BIND_TEXTURE_HEADER_VALID_TRUE                                                          0x00000001
#define NV91C0_BIND_TEXTURE_HEADER_TEXTURE_SLOT                                                               8:1
#define NV91C0_BIND_TEXTURE_HEADER_INDEX                                                                     30:9

#define NV91C0_BIND_EXTRA_TEXTURE_SAMPLER                                                                  0x0230
#define NV91C0_BIND_EXTRA_TEXTURE_SAMPLER_VALID                                                               0:0
#define NV91C0_BIND_EXTRA_TEXTURE_SAMPLER_VALID_FALSE                                                  0x00000000
#define NV91C0_BIND_EXTRA_TEXTURE_SAMPLER_VALID_TRUE                                                   0x00000001
#define NV91C0_BIND_EXTRA_TEXTURE_SAMPLER_SAMPLER_SLOT                                                       11:4
#define NV91C0_BIND_EXTRA_TEXTURE_SAMPLER_INDEX                                                             24:12

#define NV91C0_BIND_EXTRA_TEXTURE_HEADER                                                                   0x0234
#define NV91C0_BIND_EXTRA_TEXTURE_HEADER_VALID                                                                0:0
#define NV91C0_BIND_EXTRA_TEXTURE_HEADER_VALID_FALSE                                                   0x00000000
#define NV91C0_BIND_EXTRA_TEXTURE_HEADER_VALID_TRUE                                                    0x00000001
#define NV91C0_BIND_EXTRA_TEXTURE_HEADER_TEXTURE_SLOT                                                         8:1
#define NV91C0_BIND_EXTRA_TEXTURE_HEADER_INDEX                                                               30:9

#define NV91C0_SET_CTA_RASTER_SIZE_A                                                                       0x0238
#define NV91C0_SET_CTA_RASTER_SIZE_A_WIDTH                                                                   15:0
#define NV91C0_SET_CTA_RASTER_SIZE_A_HEIGHT                                                                 31:16

#define NV91C0_SET_CTA_RASTER_SIZE_B                                                                       0x023c
#define NV91C0_SET_CTA_RASTER_SIZE_B_DEPTH                                                                   15:0
#define NV91C0_SET_CTA_RASTER_SIZE_B_WIDTH_UPPER                                                            31:16

#define NV91C0_INVALIDATE_TEXTURE_HEADER_CACHE_NO_WFI                                                      0x0244
#define NV91C0_INVALIDATE_TEXTURE_HEADER_CACHE_NO_WFI_LINES                                                   0:0
#define NV91C0_INVALIDATE_TEXTURE_HEADER_CACHE_NO_WFI_LINES_ALL                                        0x00000000
#define NV91C0_INVALIDATE_TEXTURE_HEADER_CACHE_NO_WFI_LINES_ONE                                        0x00000001
#define NV91C0_INVALIDATE_TEXTURE_HEADER_CACHE_NO_WFI_TAG                                                    25:4

#define NV91C0_SET_SHADER_SHARED_MEMORY_SIZE                                                               0x024c
#define NV91C0_SET_SHADER_SHARED_MEMORY_SIZE_V                                                               17:0

#define NV91C0_SET_CTA_THREAD_COUNT                                                                        0x0250
#define NV91C0_SET_CTA_THREAD_COUNT_V                                                                        15:0

#define NV91C0_SET_CTA_BARRIER_COUNT                                                                       0x0254
#define NV91C0_SET_CTA_BARRIER_COUNT_V                                                                        7:0

#define NV91C0_TEST_FOR_COMPUTE                                                                            0x028c
#define NV91C0_TEST_FOR_COMPUTE_V                                                                            31:0

#define NV91C0_BEGIN_GRID                                                                                  0x029c
#define NV91C0_BEGIN_GRID_V                                                                                   0:0

#define NV91C0_SET_WORK_DISTRIBUTION                                                                       0x02a0
#define NV91C0_SET_WORK_DISTRIBUTION_MAX_BATCH_SIZE                                                         16:13
#define NV91C0_SET_WORK_DISTRIBUTION_FIXED_MODE                                                               4:4
#define NV91C0_SET_WORK_DISTRIBUTION_FIXED_MODE_FALSE                                                  0x00000000
#define NV91C0_SET_WORK_DISTRIBUTION_FIXED_MODE_TRUE                                                   0x00000001
#define NV91C0_SET_WORK_DISTRIBUTION_MAX_STANDBY_CTAS                                                        12:5

#define NV91C0_SET_CTA_REGISTER_COUNT                                                                      0x02c0
#define NV91C0_SET_CTA_REGISTER_COUNT_V                                                                       7:0

#define NV91C0_SET_GA_TO_VA_MAPPING_MODE                                                                   0x02c4
#define NV91C0_SET_GA_TO_VA_MAPPING_MODE_V                                                                    0:0
#define NV91C0_SET_GA_TO_VA_MAPPING_MODE_V_DISABLE                                                     0x00000000
#define NV91C0_SET_GA_TO_VA_MAPPING_MODE_V_ENABLE                                                      0x00000001

#define NV91C0_LOAD_GA_TO_VA_MAPPING_ENTRY                                                                 0x02c8
#define NV91C0_LOAD_GA_TO_VA_MAPPING_ENTRY_VIRTUAL_ADDRESS_UPPER                                              7:0
#define NV91C0_LOAD_GA_TO_VA_MAPPING_ENTRY_GENERIC_ADDRESS_UPPER                                            23:16
#define NV91C0_LOAD_GA_TO_VA_MAPPING_ENTRY_READ_ENABLE                                                      30:30
#define NV91C0_LOAD_GA_TO_VA_MAPPING_ENTRY_READ_ENABLE_FALSE                                           0x00000000
#define NV91C0_LOAD_GA_TO_VA_MAPPING_ENTRY_READ_ENABLE_TRUE                                            0x00000001
#define NV91C0_LOAD_GA_TO_VA_MAPPING_ENTRY_WRITE_ENABLE                                                     31:31
#define NV91C0_LOAD_GA_TO_VA_MAPPING_ENTRY_WRITE_ENABLE_FALSE                                          0x00000000
#define NV91C0_LOAD_GA_TO_VA_MAPPING_ENTRY_WRITE_ENABLE_TRUE                                           0x00000001

#define NV91C0_SET_TEX_HEADER_EXTENDED_DIMENSIONS                                                          0x02e0
#define NV91C0_SET_TEX_HEADER_EXTENDED_DIMENSIONS_ENABLE                                                      0:0
#define NV91C0_SET_TEX_HEADER_EXTENDED_DIMENSIONS_ENABLE_FALSE                                         0x00000000
#define NV91C0_SET_TEX_HEADER_EXTENDED_DIMENSIONS_ENABLE_TRUE                                          0x00000001

#define NV91C0_SET_L1_CONFIGURATION                                                                        0x0308
#define NV91C0_SET_L1_CONFIGURATION_DIRECTLY_ADDRESSABLE_MEMORY                                               2:0
#define NV91C0_SET_L1_CONFIGURATION_DIRECTLY_ADDRESSABLE_MEMORY_SIZE_16KB                              0x00000001
#define NV91C0_SET_L1_CONFIGURATION_DIRECTLY_ADDRESSABLE_MEMORY_SIZE_32KB                              0x00000002
#define NV91C0_SET_L1_CONFIGURATION_DIRECTLY_ADDRESSABLE_MEMORY_SIZE_48KB                              0x00000003

#define NV91C0_SET_RENDER_ENABLE_CONTROL                                                                   0x030c
#define NV91C0_SET_RENDER_ENABLE_CONTROL_CONDITIONAL_LOAD_CONSTANT_BUFFER                                     0:0
#define NV91C0_SET_RENDER_ENABLE_CONTROL_CONDITIONAL_LOAD_CONSTANT_BUFFER_FALSE                        0x00000000
#define NV91C0_SET_RENDER_ENABLE_CONTROL_CONDITIONAL_LOAD_CONSTANT_BUFFER_TRUE                         0x00000001

#define NV91C0_WAIT_REF_COUNT                                                                              0x0360
#define NV91C0_WAIT_REF_COUNT_REF_CNT                                                                        11:8
#define NV91C0_WAIT_REF_COUNT_FLUSH_SYS_MEM                                                                   0:0
#define NV91C0_WAIT_REF_COUNT_FLUSH_SYS_MEM_FALSE                                                      0x00000000
#define NV91C0_WAIT_REF_COUNT_FLUSH_SYS_MEM_TRUE                                                       0x00000001

#define NV91C0_LAUNCH                                                                                      0x0368
#define NV91C0_LAUNCHCTA_PARAM                                                                               31:0

#define NV91C0_SET_LAUNCH_ID                                                                               0x036c
#define NV91C0_SET_LAUNCH_ID_REF_CNT                                                                          3:0

#define NV91C0_SET_CTA_THREAD_DIMENSION_A                                                                  0x03ac
#define NV91C0_SET_CTA_THREAD_DIMENSION_A_D0                                                                 15:0
#define NV91C0_SET_CTA_THREAD_DIMENSION_A_D1                                                                31:16

#define NV91C0_SET_CTA_THREAD_DIMENSION_B                                                                  0x03b0
#define NV91C0_SET_CTA_THREAD_DIMENSION_B_D2                                                                 15:0

#define NV91C0_SET_CTA_PROGRAM_START                                                                       0x03b4
#define NV91C0_SET_CTA_PROGRAM_START_OFFSET                                                                  31:0

#define NV91C0_SET_FALCON00                                                                                0x0500
#define NV91C0_SET_FALCON00_V                                                                                31:0

#define NV91C0_SET_FALCON01                                                                                0x0504
#define NV91C0_SET_FALCON01_V                                                                                31:0

#define NV91C0_SET_FALCON02                                                                                0x0508
#define NV91C0_SET_FALCON02_V                                                                                31:0

#define NV91C0_SET_FALCON03                                                                                0x050c
#define NV91C0_SET_FALCON03_V                                                                                31:0

#define NV91C0_SET_FALCON04                                                                                0x0510
#define NV91C0_SET_FALCON04_V                                                                                31:0

#define NV91C0_SET_FALCON05                                                                                0x0514
#define NV91C0_SET_FALCON05_V                                                                                31:0

#define NV91C0_SET_FALCON06                                                                                0x0518
#define NV91C0_SET_FALCON06_V                                                                                31:0

#define NV91C0_SET_FALCON07                                                                                0x051c
#define NV91C0_SET_FALCON07_V                                                                                31:0

#define NV91C0_SET_FALCON08                                                                                0x0520
#define NV91C0_SET_FALCON08_V                                                                                31:0

#define NV91C0_SET_FALCON09                                                                                0x0524
#define NV91C0_SET_FALCON09_V                                                                                31:0

#define NV91C0_SET_FALCON10                                                                                0x0528
#define NV91C0_SET_FALCON10_V                                                                                31:0

#define NV91C0_SET_FALCON11                                                                                0x052c
#define NV91C0_SET_FALCON11_V                                                                                31:0

#define NV91C0_SET_FALCON12                                                                                0x0530
#define NV91C0_SET_FALCON12_V                                                                                31:0

#define NV91C0_SET_FALCON13                                                                                0x0534
#define NV91C0_SET_FALCON13_V                                                                                31:0

#define NV91C0_SET_FALCON14                                                                                0x0538
#define NV91C0_SET_FALCON14_V                                                                                31:0

#define NV91C0_SET_FALCON15                                                                                0x053c
#define NV91C0_SET_FALCON15_V                                                                                31:0

#define NV91C0_SET_FALCON16                                                                                0x0540
#define NV91C0_SET_FALCON16_V                                                                                31:0

#define NV91C0_SET_FALCON17                                                                                0x0544
#define NV91C0_SET_FALCON17_V                                                                                31:0

#define NV91C0_SET_FALCON18                                                                                0x0548
#define NV91C0_SET_FALCON18_V                                                                                31:0

#define NV91C0_SET_FALCON19                                                                                0x054c
#define NV91C0_SET_FALCON19_V                                                                                31:0

#define NV91C0_SET_FALCON20                                                                                0x0550
#define NV91C0_SET_FALCON20_V                                                                                31:0

#define NV91C0_SET_FALCON21                                                                                0x0554
#define NV91C0_SET_FALCON21_V                                                                                31:0

#define NV91C0_SET_FALCON22                                                                                0x0558
#define NV91C0_SET_FALCON22_V                                                                                31:0

#define NV91C0_SET_FALCON23                                                                                0x055c
#define NV91C0_SET_FALCON23_V                                                                                31:0

#define NV91C0_SET_FALCON24                                                                                0x0560
#define NV91C0_SET_FALCON24_V                                                                                31:0

#define NV91C0_SET_FALCON25                                                                                0x0564
#define NV91C0_SET_FALCON25_V                                                                                31:0

#define NV91C0_SET_FALCON26                                                                                0x0568
#define NV91C0_SET_FALCON26_V                                                                                31:0

#define NV91C0_SET_FALCON27                                                                                0x056c
#define NV91C0_SET_FALCON27_V                                                                                31:0

#define NV91C0_SET_FALCON28                                                                                0x0570
#define NV91C0_SET_FALCON28_V                                                                                31:0

#define NV91C0_SET_FALCON29                                                                                0x0574
#define NV91C0_SET_FALCON29_V                                                                                31:0

#define NV91C0_SET_FALCON30                                                                                0x0578
#define NV91C0_SET_FALCON30_V                                                                                31:0

#define NV91C0_SET_FALCON31                                                                                0x057c
#define NV91C0_SET_FALCON31_V                                                                                31:0

#define NV91C0_SET_MAX_SM_COUNT                                                                            0x0758
#define NV91C0_SET_MAX_SM_COUNT_V                                                                             8:0

#define NV91C0_SET_SHADER_LOCAL_MEMORY_WINDOW                                                              0x077c
#define NV91C0_SET_SHADER_LOCAL_MEMORY_WINDOW_BASE_ADDRESS                                                   31:0

#define NV91C0_SET_GRID_PARAM                                                                              0x0780
#define NV91C0_SET_GRID_PARAM_V                                                                              31:0

#define NV91C0_SET_SHADER_LOCAL_MEMORY_A                                                                   0x0790
#define NV91C0_SET_SHADER_LOCAL_MEMORY_A_ADDRESS_UPPER                                                        7:0

#define NV91C0_SET_SHADER_LOCAL_MEMORY_B                                                                   0x0794
#define NV91C0_SET_SHADER_LOCAL_MEMORY_B_ADDRESS_LOWER                                                       31:0

#define NV91C0_SET_SHADER_LOCAL_MEMORY_C                                                                   0x0798
#define NV91C0_SET_SHADER_LOCAL_MEMORY_C_SIZE_UPPER                                                           5:0

#define NV91C0_SET_SHADER_LOCAL_MEMORY_D                                                                   0x079c
#define NV91C0_SET_SHADER_LOCAL_MEMORY_D_SIZE_LOWER                                                          31:0

#define NV91C0_SET_SHADER_LOCAL_MEMORY_E                                                                   0x07a0
#define NV91C0_SET_SHADER_LOCAL_MEMORY_E_DEFAULT_SIZE_PER_WARP                                               25:0

#define NV91C0_END_GRID                                                                                    0x0a04
#define NV91C0_END_GRID_V                                                                                     0:0

#define NV91C0_SET_LAUNCH_SIZE                                                                             0x0a08
#define NV91C0_SET_LAUNCH_SIZE_V                                                                             31:0

#define NV91C0_SET_API_VISIBLE_CALL_LIMIT                                                                  0x0d64
#define NV91C0_SET_API_VISIBLE_CALL_LIMIT_CTA                                                                 3:0
#define NV91C0_SET_API_VISIBLE_CALL_LIMIT_CTA__0                                                       0x00000000
#define NV91C0_SET_API_VISIBLE_CALL_LIMIT_CTA__1                                                       0x00000001
#define NV91C0_SET_API_VISIBLE_CALL_LIMIT_CTA__2                                                       0x00000002
#define NV91C0_SET_API_VISIBLE_CALL_LIMIT_CTA__4                                                       0x00000003
#define NV91C0_SET_API_VISIBLE_CALL_LIMIT_CTA__8                                                       0x00000004
#define NV91C0_SET_API_VISIBLE_CALL_LIMIT_CTA__16                                                      0x00000005
#define NV91C0_SET_API_VISIBLE_CALL_LIMIT_CTA__32                                                      0x00000006
#define NV91C0_SET_API_VISIBLE_CALL_LIMIT_CTA__64                                                      0x00000007
#define NV91C0_SET_API_VISIBLE_CALL_LIMIT_CTA__128                                                     0x00000008
#define NV91C0_SET_API_VISIBLE_CALL_LIMIT_CTA_NO_CHECK                                                 0x0000000F

#define NV91C0_SET_SHADER_CACHE_CONTROL                                                                    0x0d94
#define NV91C0_SET_SHADER_CACHE_CONTROL_ICACHE_PREFETCH_ENABLE                                                0:0
#define NV91C0_SET_SHADER_CACHE_CONTROL_ICACHE_PREFETCH_ENABLE_FALSE                                   0x00000000
#define NV91C0_SET_SHADER_CACHE_CONTROL_ICACHE_PREFETCH_ENABLE_TRUE                                    0x00000001

#define NV91C0_SET_SM_TIMEOUT_INTERVAL                                                                     0x0de4
#define NV91C0_SET_SM_TIMEOUT_INTERVAL_COUNTER_BIT                                                            5:0

#define NV91C0_SET_SPARE_NOOP12                                                                            0x0f44
#define NV91C0_SET_SPARE_NOOP12_V                                                                            31:0

#define NV91C0_SET_SPARE_NOOP13                                                                            0x0f48
#define NV91C0_SET_SPARE_NOOP13_V                                                                            31:0

#define NV91C0_SET_SPARE_NOOP14                                                                            0x0f4c
#define NV91C0_SET_SPARE_NOOP14_V                                                                            31:0

#define NV91C0_SET_SPARE_NOOP15                                                                            0x0f50
#define NV91C0_SET_SPARE_NOOP15_V                                                                            31:0

#define NV91C0_SET_FORCE_ONE_TEXTURE_UNIT                                                                  0x1004
#define NV91C0_SET_FORCE_ONE_TEXTURE_UNIT_ENABLE                                                              0:0
#define NV91C0_SET_FORCE_ONE_TEXTURE_UNIT_ENABLE_FALSE                                                 0x00000000
#define NV91C0_SET_FORCE_ONE_TEXTURE_UNIT_ENABLE_TRUE                                                  0x00000001

#define NV91C0_SET_SPARE_NOOP00                                                                            0x1040
#define NV91C0_SET_SPARE_NOOP00_V                                                                            31:0

#define NV91C0_SET_SPARE_NOOP01                                                                            0x1044
#define NV91C0_SET_SPARE_NOOP01_V                                                                            31:0

#define NV91C0_SET_SPARE_NOOP02                                                                            0x1048
#define NV91C0_SET_SPARE_NOOP02_V                                                                            31:0

#define NV91C0_SET_SPARE_NOOP03                                                                            0x104c
#define NV91C0_SET_SPARE_NOOP03_V                                                                            31:0

#define NV91C0_SET_SPARE_NOOP04                                                                            0x1050
#define NV91C0_SET_SPARE_NOOP04_V                                                                            31:0

#define NV91C0_SET_SPARE_NOOP05                                                                            0x1054
#define NV91C0_SET_SPARE_NOOP05_V                                                                            31:0

#define NV91C0_SET_SPARE_NOOP06                                                                            0x1058
#define NV91C0_SET_SPARE_NOOP06_V                                                                            31:0

#define NV91C0_SET_SPARE_NOOP07                                                                            0x105c
#define NV91C0_SET_SPARE_NOOP07_V                                                                            31:0

#define NV91C0_SET_SPARE_NOOP08                                                                            0x1060
#define NV91C0_SET_SPARE_NOOP08_V                                                                            31:0

#define NV91C0_SET_SPARE_NOOP09                                                                            0x1064
#define NV91C0_SET_SPARE_NOOP09_V                                                                            31:0

#define NV91C0_SET_SPARE_NOOP10                                                                            0x1068
#define NV91C0_SET_SPARE_NOOP10_V                                                                            31:0

#define NV91C0_SET_SPARE_NOOP11                                                                            0x106c
#define NV91C0_SET_SPARE_NOOP11_V                                                                            31:0

#define NV91C0_UNBIND_ALL                                                                                  0x10f4
#define NV91C0_UNBIND_ALL_TEXTURE_HEADERS                                                                     0:0
#define NV91C0_UNBIND_ALL_TEXTURE_HEADERS_FALSE                                                        0x00000000
#define NV91C0_UNBIND_ALL_TEXTURE_HEADERS_TRUE                                                         0x00000001
#define NV91C0_UNBIND_ALL_TEXTURE_SAMPLERS                                                                    4:4
#define NV91C0_UNBIND_ALL_TEXTURE_SAMPLERS_FALSE                                                       0x00000000
#define NV91C0_UNBIND_ALL_TEXTURE_SAMPLERS_TRUE                                                        0x00000001
#define NV91C0_UNBIND_ALL_CONSTANT_BUFFERS                                                                    8:8
#define NV91C0_UNBIND_ALL_CONSTANT_BUFFERS_FALSE                                                       0x00000000
#define NV91C0_UNBIND_ALL_CONSTANT_BUFFERS_TRUE                                                        0x00000001

#define NV91C0_SET_SAMPLER_BINDING                                                                         0x1234
#define NV91C0_SET_SAMPLER_BINDING_V                                                                          0:0
#define NV91C0_SET_SAMPLER_BINDING_V_INDEPENDENTLY                                                     0x00000000
#define NV91C0_SET_SAMPLER_BINDING_V_VIA_HEADER_BINDING                                                0x00000001

#define NV91C0_INVALIDATE_TEXTURE_DATA_CACHE_NO_WFI                                                        0x1288
#define NV91C0_INVALIDATE_TEXTURE_DATA_CACHE_NO_WFI_LINES                                                     0:0
#define NV91C0_INVALIDATE_TEXTURE_DATA_CACHE_NO_WFI_LINES_ALL                                          0x00000000
#define NV91C0_INVALIDATE_TEXTURE_DATA_CACHE_NO_WFI_LINES_ONE                                          0x00000001
#define NV91C0_INVALIDATE_TEXTURE_DATA_CACHE_NO_WFI_TAG                                                      25:4

#define NV91C0_SET_SHADER_SCHEDULING                                                                       0x12ac
#define NV91C0_SET_SHADER_SCHEDULING_MODE                                                                     0:0
#define NV91C0_SET_SHADER_SCHEDULING_MODE_OLDEST_THREAD_FIRST                                          0x00000000
#define NV91C0_SET_SHADER_SCHEDULING_MODE_ROUND_ROBIN                                                  0x00000001

#define NV91C0_INVALIDATE_SAMPLER_CACHE                                                                    0x1330
#define NV91C0_INVALIDATE_SAMPLER_CACHE_LINES                                                                 0:0
#define NV91C0_INVALIDATE_SAMPLER_CACHE_LINES_ALL                                                      0x00000000
#define NV91C0_INVALIDATE_SAMPLER_CACHE_LINES_ONE                                                      0x00000001
#define NV91C0_INVALIDATE_SAMPLER_CACHE_TAG                                                                  25:4

#define NV91C0_INVALIDATE_TEXTURE_HEADER_CACHE                                                             0x1334
#define NV91C0_INVALIDATE_TEXTURE_HEADER_CACHE_LINES                                                          0:0
#define NV91C0_INVALIDATE_TEXTURE_HEADER_CACHE_LINES_ALL                                               0x00000000
#define NV91C0_INVALIDATE_TEXTURE_HEADER_CACHE_LINES_ONE                                               0x00000001
#define NV91C0_INVALIDATE_TEXTURE_HEADER_CACHE_TAG                                                           25:4

#define NV91C0_INVALIDATE_TEXTURE_DATA_CACHE                                                               0x1338
#define NV91C0_INVALIDATE_TEXTURE_DATA_CACHE_LINES                                                            0:0
#define NV91C0_INVALIDATE_TEXTURE_DATA_CACHE_LINES_ALL                                                 0x00000000
#define NV91C0_INVALIDATE_TEXTURE_DATA_CACHE_LINES_ONE                                                 0x00000001
#define NV91C0_INVALIDATE_TEXTURE_DATA_CACHE_TAG                                                             25:4
#define NV91C0_INVALIDATE_TEXTURE_DATA_CACHE_LEVELS                                                           2:1
#define NV91C0_INVALIDATE_TEXTURE_DATA_CACHE_LEVELS_L1_ONLY                                            0x00000000

#define NV91C0_SET_GLOBAL_COLOR_KEY                                                                        0x1354
#define NV91C0_SET_GLOBAL_COLOR_KEY_ENABLE                                                                    0:0
#define NV91C0_SET_GLOBAL_COLOR_KEY_ENABLE_FALSE                                                       0x00000000
#define NV91C0_SET_GLOBAL_COLOR_KEY_ENABLE_TRUE                                                        0x00000001

#define NV91C0_INVALIDATE_SAMPLER_CACHE_NO_WFI                                                             0x1424
#define NV91C0_INVALIDATE_SAMPLER_CACHE_NO_WFI_LINES                                                          0:0
#define NV91C0_INVALIDATE_SAMPLER_CACHE_NO_WFI_LINES_ALL                                               0x00000000
#define NV91C0_INVALIDATE_SAMPLER_CACHE_NO_WFI_LINES_ONE                                               0x00000001
#define NV91C0_INVALIDATE_SAMPLER_CACHE_NO_WFI_TAG                                                           25:4

#define NV91C0_PERFMON_TRANSFER                                                                            0x1524
#define NV91C0_PERFMON_TRANSFER_V                                                                            31:0

#define NV91C0_SET_SHADER_EXCEPTIONS                                                                       0x1528
#define NV91C0_SET_SHADER_EXCEPTIONS_ENABLE                                                                   0:0
#define NV91C0_SET_SHADER_EXCEPTIONS_ENABLE_FALSE                                                      0x00000000
#define NV91C0_SET_SHADER_EXCEPTIONS_ENABLE_TRUE                                                       0x00000001

#define NV91C0_SET_RENDER_ENABLE_A                                                                         0x1550
#define NV91C0_SET_RENDER_ENABLE_A_OFFSET_UPPER                                                               7:0

#define NV91C0_SET_RENDER_ENABLE_B                                                                         0x1554
#define NV91C0_SET_RENDER_ENABLE_B_OFFSET_LOWER                                                              31:0

#define NV91C0_SET_RENDER_ENABLE_C                                                                         0x1558
#define NV91C0_SET_RENDER_ENABLE_C_MODE                                                                       2:0
#define NV91C0_SET_RENDER_ENABLE_C_MODE_FALSE                                                          0x00000000
#define NV91C0_SET_RENDER_ENABLE_C_MODE_TRUE                                                           0x00000001
#define NV91C0_SET_RENDER_ENABLE_C_MODE_CONDITIONAL                                                    0x00000002
#define NV91C0_SET_RENDER_ENABLE_C_MODE_RENDER_IF_EQUAL                                                0x00000003
#define NV91C0_SET_RENDER_ENABLE_C_MODE_RENDER_IF_NOT_EQUAL                                            0x00000004

#define NV91C0_SET_TEX_SAMPLER_POOL_A                                                                      0x155c
#define NV91C0_SET_TEX_SAMPLER_POOL_A_OFFSET_UPPER                                                            7:0

#define NV91C0_SET_TEX_SAMPLER_POOL_B                                                                      0x1560
#define NV91C0_SET_TEX_SAMPLER_POOL_B_OFFSET_LOWER                                                           31:0

#define NV91C0_SET_TEX_SAMPLER_POOL_C                                                                      0x1564
#define NV91C0_SET_TEX_SAMPLER_POOL_C_MAXIMUM_INDEX                                                          19:0

#define NV91C0_SET_TEX_HEADER_POOL_A                                                                       0x1574
#define NV91C0_SET_TEX_HEADER_POOL_A_OFFSET_UPPER                                                             7:0

#define NV91C0_SET_TEX_HEADER_POOL_B                                                                       0x1578
#define NV91C0_SET_TEX_HEADER_POOL_B_OFFSET_LOWER                                                            31:0

#define NV91C0_SET_TEX_HEADER_POOL_C                                                                       0x157c
#define NV91C0_SET_TEX_HEADER_POOL_C_MAXIMUM_INDEX                                                           21:0

#define NV91C0_SET_PROGRAM_REGION_A                                                                        0x1608
#define NV91C0_SET_PROGRAM_REGION_A_ADDRESS_UPPER                                                             7:0

#define NV91C0_SET_PROGRAM_REGION_B                                                                        0x160c
#define NV91C0_SET_PROGRAM_REGION_B_ADDRESS_LOWER                                                            31:0

#define NV91C0_SET_CUBEMAP_INTER_FACE_FILTERING                                                            0x1664
#define NV91C0_SET_CUBEMAP_INTER_FACE_FILTERING_MODE                                                          1:0
#define NV91C0_SET_CUBEMAP_INTER_FACE_FILTERING_MODE_USE_WRAP                                          0x00000000
#define NV91C0_SET_CUBEMAP_INTER_FACE_FILTERING_MODE_OVERRIDE_WRAP                                     0x00000001
#define NV91C0_SET_CUBEMAP_INTER_FACE_FILTERING_MODE_AUTO_SPAN_SEAM                                    0x00000002
#define NV91C0_SET_CUBEMAP_INTER_FACE_FILTERING_MODE_AUTO_CROSS_SEAM                                   0x00000003

#define NV91C0_SET_SHADER_CONTROL                                                                          0x1690
#define NV91C0_SET_SHADER_CONTROL_DEFAULT_PARTIAL                                                             0:0
#define NV91C0_SET_SHADER_CONTROL_DEFAULT_PARTIAL_ZERO                                                 0x00000000
#define NV91C0_SET_SHADER_CONTROL_DEFAULT_PARTIAL_INFINITY                                             0x00000001
#define NV91C0_SET_SHADER_CONTROL_ZERO_TIMES_ANYTHING_IS_ZERO                                               16:16
#define NV91C0_SET_SHADER_CONTROL_ZERO_TIMES_ANYTHING_IS_ZERO_FALSE                                    0x00000000
#define NV91C0_SET_SHADER_CONTROL_ZERO_TIMES_ANYTHING_IS_ZERO_TRUE                                     0x00000001
#define NV91C0_SET_SHADER_CONTROL_FP32_NAN_BEHAVIOR                                                           1:1
#define NV91C0_SET_SHADER_CONTROL_FP32_NAN_BEHAVIOR_LEGACY                                             0x00000000
#define NV91C0_SET_SHADER_CONTROL_FP32_NAN_BEHAVIOR_FP64_COMPATIBLE                                    0x00000001
#define NV91C0_SET_SHADER_CONTROL_FP32_F2I_NAN_BEHAVIOR                                                       2:2
#define NV91C0_SET_SHADER_CONTROL_FP32_F2I_NAN_BEHAVIOR_PASS_ZERO                                      0x00000000
#define NV91C0_SET_SHADER_CONTROL_FP32_F2I_NAN_BEHAVIOR_PASS_INDEFINITE                                0x00000001

#define NV91C0_BIND_CONSTANT_BUFFER                                                                        0x1694
#define NV91C0_BIND_CONSTANT_BUFFER_VALID                                                                     0:0
#define NV91C0_BIND_CONSTANT_BUFFER_VALID_FALSE                                                        0x00000000
#define NV91C0_BIND_CONSTANT_BUFFER_VALID_TRUE                                                         0x00000001
#define NV91C0_BIND_CONSTANT_BUFFER_SHADER_SLOT                                                              12:8

#define NV91C0_INVALIDATE_SHADER_CACHES_NO_WFI                                                             0x1698
#define NV91C0_INVALIDATE_SHADER_CACHES_NO_WFI_INSTRUCTION                                                    0:0
#define NV91C0_INVALIDATE_SHADER_CACHES_NO_WFI_INSTRUCTION_FALSE                                       0x00000000
#define NV91C0_INVALIDATE_SHADER_CACHES_NO_WFI_INSTRUCTION_TRUE                                        0x00000001
#define NV91C0_INVALIDATE_SHADER_CACHES_NO_WFI_GLOBAL_DATA                                                    4:4
#define NV91C0_INVALIDATE_SHADER_CACHES_NO_WFI_GLOBAL_DATA_FALSE                                       0x00000000
#define NV91C0_INVALIDATE_SHADER_CACHES_NO_WFI_GLOBAL_DATA_TRUE                                        0x00000001
#define NV91C0_INVALIDATE_SHADER_CACHES_NO_WFI_UNIFORM                                                        8:8
#define NV91C0_INVALIDATE_SHADER_CACHES_NO_WFI_UNIFORM_FALSE                                           0x00000000
#define NV91C0_INVALIDATE_SHADER_CACHES_NO_WFI_UNIFORM_TRUE                                            0x00000001
#define NV91C0_INVALIDATE_SHADER_CACHES_NO_WFI_CONSTANT                                                     12:12
#define NV91C0_INVALIDATE_SHADER_CACHES_NO_WFI_CONSTANT_FALSE                                          0x00000000
#define NV91C0_INVALIDATE_SHADER_CACHES_NO_WFI_CONSTANT_TRUE                                           0x00000001

#define NV91C0_INVALIDATE_CONSTANT_BUFFER_CACHE                                                            0x1930
#define NV91C0_INVALIDATE_CONSTANT_BUFFER_CACHE_THRU_L2                                                       0:0
#define NV91C0_INVALIDATE_CONSTANT_BUFFER_CACHE_THRU_L2_FALSE                                          0x00000000
#define NV91C0_INVALIDATE_CONSTANT_BUFFER_CACHE_THRU_L2_TRUE                                           0x00000001

#define NV91C0_SET_RENDER_ENABLE_OVERRIDE                                                                  0x1944
#define NV91C0_SET_RENDER_ENABLE_OVERRIDE_MODE                                                                1:0
#define NV91C0_SET_RENDER_ENABLE_OVERRIDE_MODE_USE_RENDER_ENABLE                                       0x00000000
#define NV91C0_SET_RENDER_ENABLE_OVERRIDE_MODE_ALWAYS_RENDER                                           0x00000001
#define NV91C0_SET_RENDER_ENABLE_OVERRIDE_MODE_NEVER_RENDER                                            0x00000002

#define NV91C0_PIPE_NOP                                                                                    0x1a2c
#define NV91C0_PIPE_NOP_V                                                                                    31:0

#define NV91C0_SET_SPARE00                                                                                 0x1a30
#define NV91C0_SET_SPARE00_V                                                                                 31:0

#define NV91C0_SET_SPARE01                                                                                 0x1a34
#define NV91C0_SET_SPARE01_V                                                                                 31:0

#define NV91C0_SET_SPARE02                                                                                 0x1a38
#define NV91C0_SET_SPARE02_V                                                                                 31:0

#define NV91C0_SET_SPARE03                                                                                 0x1a3c
#define NV91C0_SET_SPARE03_V                                                                                 31:0

#define NV91C0_SET_REPORT_SEMAPHORE_A                                                                      0x1b00
#define NV91C0_SET_REPORT_SEMAPHORE_A_OFFSET_UPPER                                                            7:0

#define NV91C0_SET_REPORT_SEMAPHORE_B                                                                      0x1b04
#define NV91C0_SET_REPORT_SEMAPHORE_B_OFFSET_LOWER                                                           31:0

#define NV91C0_SET_REPORT_SEMAPHORE_C                                                                      0x1b08
#define NV91C0_SET_REPORT_SEMAPHORE_C_PAYLOAD                                                                31:0

#define NV91C0_SET_REPORT_SEMAPHORE_D                                                                      0x1b0c
#define NV91C0_SET_REPORT_SEMAPHORE_D_OPERATION                                                               1:0
#define NV91C0_SET_REPORT_SEMAPHORE_D_OPERATION_RELEASE                                                0x00000000
#define NV91C0_SET_REPORT_SEMAPHORE_D_OPERATION_TRAP                                                   0x00000003
#define NV91C0_SET_REPORT_SEMAPHORE_D_AWAKEN_ENABLE                                                         20:20
#define NV91C0_SET_REPORT_SEMAPHORE_D_AWAKEN_ENABLE_FALSE                                              0x00000000
#define NV91C0_SET_REPORT_SEMAPHORE_D_AWAKEN_ENABLE_TRUE                                               0x00000001
#define NV91C0_SET_REPORT_SEMAPHORE_D_STRUCTURE_SIZE                                                        28:28
#define NV91C0_SET_REPORT_SEMAPHORE_D_STRUCTURE_SIZE_FOUR_WORDS                                        0x00000000
#define NV91C0_SET_REPORT_SEMAPHORE_D_STRUCTURE_SIZE_ONE_WORD                                          0x00000001
#define NV91C0_SET_REPORT_SEMAPHORE_D_FLUSH_DISABLE                                                           2:2
#define NV91C0_SET_REPORT_SEMAPHORE_D_FLUSH_DISABLE_FALSE                                              0x00000000
#define NV91C0_SET_REPORT_SEMAPHORE_D_FLUSH_DISABLE_TRUE                                               0x00000001

#define NV91C0_SET_CONSTANT_BUFFER_SELECTOR_A                                                              0x2380
#define NV91C0_SET_CONSTANT_BUFFER_SELECTOR_A_SIZE                                                           16:0

#define NV91C0_SET_CONSTANT_BUFFER_SELECTOR_B                                                              0x2384
#define NV91C0_SET_CONSTANT_BUFFER_SELECTOR_B_ADDRESS_UPPER                                                   7:0

#define NV91C0_SET_CONSTANT_BUFFER_SELECTOR_C                                                              0x2388
#define NV91C0_SET_CONSTANT_BUFFER_SELECTOR_C_ADDRESS_LOWER                                                  31:0

#define NV91C0_LOAD_CONSTANT_BUFFER_OFFSET                                                                 0x238c
#define NV91C0_LOAD_CONSTANT_BUFFER_OFFSET_V                                                                 15:0

#define NV91C0_LOAD_CONSTANT_BUFFER(i)                                                             (0x2390+(i)*4)
#define NV91C0_LOAD_CONSTANT_BUFFER_V                                                                        31:0

#define NV91C0_SET_SU_LD_ST_TARGET_A(j)                                                           (0x2700+(j)*32)
#define NV91C0_SET_SU_LD_ST_TARGET_A_OFFSET_UPPER                                                             7:0

#define NV91C0_SET_SU_LD_ST_TARGET_B(j)                                                           (0x2704+(j)*32)
#define NV91C0_SET_SU_LD_ST_TARGET_B_OFFSET_LOWER                                                            31:0

#define NV91C0_SET_SU_LD_ST_TARGET_C(j)                                                           (0x2708+(j)*32)
#define NV91C0_SET_SU_LD_ST_TARGET_C_WIDTH                                                                   31:0

#define NV91C0_SET_SU_LD_ST_TARGET_D(j)                                                           (0x270c+(j)*32)
#define NV91C0_SET_SU_LD_ST_TARGET_D_HEIGHT                                                                  16:0
#define NV91C0_SET_SU_LD_ST_TARGET_D_LAYOUT_IN_MEMORY                                                       20:20
#define NV91C0_SET_SU_LD_ST_TARGET_D_LAYOUT_IN_MEMORY_BLOCKLINEAR                                      0x00000000
#define NV91C0_SET_SU_LD_ST_TARGET_D_LAYOUT_IN_MEMORY_PITCH                                            0x00000001

#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT(j)                                                      (0x2710+(j)*32)
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_TYPE                                                                0:0
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_TYPE_COLOR                                                   0x00000000
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_TYPE_ZETA                                                    0x00000001
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR                                                              11:4
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_DISABLED                                               0x00000000
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RF32_GF32_BF32_AF32                                    0x000000C0
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RS32_GS32_BS32_AS32                                    0x000000C1
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RU32_GU32_BU32_AU32                                    0x000000C2
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RF32_GF32_BF32_X32                                     0x000000C3
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RS32_GS32_BS32_X32                                     0x000000C4
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RU32_GU32_BU32_X32                                     0x000000C5
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_R16_G16_B16_A16                                        0x000000C6
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RN16_GN16_BN16_AN16                                    0x000000C7
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RS16_GS16_BS16_AS16                                    0x000000C8
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RU16_GU16_BU16_AU16                                    0x000000C9
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RF16_GF16_BF16_AF16                                    0x000000CA
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RF32_GF32                                              0x000000CB
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RS32_GS32                                              0x000000CC
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RU32_GU32                                              0x000000CD
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RF16_GF16_BF16_X16                                     0x000000CE
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_A8R8G8B8                                               0x000000CF
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_A8RL8GL8BL8                                            0x000000D0
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_A2B10G10R10                                            0x000000D1
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_AU2BU10GU10RU10                                        0x000000D2
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_A8B8G8R8                                               0x000000D5
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_A8BL8GL8RL8                                            0x000000D6
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_AN8BN8GN8RN8                                           0x000000D7
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_AS8BS8GS8RS8                                           0x000000D8
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_AU8BU8GU8RU8                                           0x000000D9
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_R16_G16                                                0x000000DA
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RN16_GN16                                              0x000000DB
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RS16_GS16                                              0x000000DC
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RU16_GU16                                              0x000000DD
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RF16_GF16                                              0x000000DE
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_A2R10G10B10                                            0x000000DF
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_BF10GF11RF11                                           0x000000E0
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RS32                                                   0x000000E3
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RU32                                                   0x000000E4
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RF32                                                   0x000000E5
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_X8R8G8B8                                               0x000000E6
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_X8RL8GL8BL8                                            0x000000E7
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_R5G6B5                                                 0x000000E8
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_A1R5G5B5                                               0x000000E9
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_G8R8                                                   0x000000EA
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_GN8RN8                                                 0x000000EB
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_GS8RS8                                                 0x000000EC
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_GU8RU8                                                 0x000000ED
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_R16                                                    0x000000EE
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RN16                                                   0x000000EF
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RS16                                                   0x000000F0
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RU16                                                   0x000000F1
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RF16                                                   0x000000F2
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_R8                                                     0x000000F3
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RN8                                                    0x000000F4
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RS8                                                    0x000000F5
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RU8                                                    0x000000F6
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_A8                                                     0x000000F7
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_X1R5G5B5                                               0x000000F8
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_X8B8G8R8                                               0x000000F9
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_X8BL8GL8RL8                                            0x000000FA
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_Z1R5G5B5                                               0x000000FB
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_O1R5G5B5                                               0x000000FC
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_Z8R8G8B8                                               0x000000FD
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_O8R8G8B8                                               0x000000FE
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_R32                                                    0x000000FF
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_A16                                                    0x00000040
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_AF16                                                   0x00000041
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_AF32                                                   0x00000042
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_A8R8                                                   0x00000043
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_R16_A16                                                0x00000044
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RF16_AF16                                              0x00000045
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_RF32_AF32                                              0x00000046
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_COLOR_B8G8R8A8                                               0x00000047
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_ZETA                                                              16:12
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_ZETA_Z16                                                     0x00000013
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_ZETA_Z24S8                                                   0x00000014
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_ZETA_X8Z24                                                   0x00000015
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_ZETA_S8Z24                                                   0x00000016
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_ZETA_V8Z24                                                   0x00000018
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_ZETA_ZF32                                                    0x0000000A
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_ZETA_ZF32_X24S8                                              0x00000019
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_ZETA_X8Z24_X16V8S8                                           0x0000001D
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_ZETA_ZF32_X16V8X8                                            0x0000001E
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_ZETA_ZF32_X16V8S8                                            0x0000001F
#define NV91C0_SET_SU_LD_ST_TARGET_FORMAT_SUQ_PIXFMT                                                        25:17

#define NV91C0_SET_SU_LD_ST_TARGET_BLOCK_SIZE(j)                                                  (0x2714+(j)*32)
#define NV91C0_SET_SU_LD_ST_TARGET_BLOCK_SIZE_WIDTH                                                           3:0
#define NV91C0_SET_SU_LD_ST_TARGET_BLOCK_SIZE_WIDTH_ONE_GOB                                            0x00000000
#define NV91C0_SET_SU_LD_ST_TARGET_BLOCK_SIZE_HEIGHT                                                          7:4
#define NV91C0_SET_SU_LD_ST_TARGET_BLOCK_SIZE_HEIGHT_ONE_GOB                                           0x00000000
#define NV91C0_SET_SU_LD_ST_TARGET_BLOCK_SIZE_HEIGHT_TWO_GOBS                                          0x00000001
#define NV91C0_SET_SU_LD_ST_TARGET_BLOCK_SIZE_HEIGHT_FOUR_GOBS                                         0x00000002
#define NV91C0_SET_SU_LD_ST_TARGET_BLOCK_SIZE_HEIGHT_EIGHT_GOBS                                        0x00000003
#define NV91C0_SET_SU_LD_ST_TARGET_BLOCK_SIZE_HEIGHT_SIXTEEN_GOBS                                      0x00000004
#define NV91C0_SET_SU_LD_ST_TARGET_BLOCK_SIZE_HEIGHT_THIRTYTWO_GOBS                                    0x00000005

#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_VALUE(i)                                             (0x335c+(i)*4)
#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_VALUE_V                                                        31:0

#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_EVENT(i)                                             (0x337c+(i)*4)
#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_EVENT_EVENT                                                     7:0

#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_A(i)                                         (0x339c+(i)*4)
#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_A_EVENT0                                                2:0
#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_A_BIT_SELECT0                                           6:4
#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_A_EVENT1                                               10:8
#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_A_BIT_SELECT1                                         14:12
#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_A_EVENT2                                              18:16
#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_A_BIT_SELECT2                                         22:20
#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_A_EVENT3                                              26:24
#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_A_BIT_SELECT3                                         30:28

#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_B(i)                                         (0x33bc+(i)*4)
#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_B_EDGE                                                  0:0
#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_CONTROL_B_FUNC                                                 19:4

#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_TRAP_CONTROL                                                 0x33dc
#define NV91C0_SET_SHADER_PERFORMANCE_COUNTER_TRAP_CONTROL_MASK                                               7:0

#define NV91C0_SET_MME_SHADOW_SCRATCH(i)                                                           (0x3400+(i)*4)
#define NV91C0_SET_MME_SHADOW_SCRATCH_V                                                                      31:0

#define NV91C0_CALL_MME_MACRO(j)                                                                   (0x3800+(j)*8)
#define NV91C0_CALL_MME_MACRO_V                                                                              31:0

#define NV91C0_CALL_MME_DATA(j)                                                                    (0x3804+(j)*8)
#define NV91C0_CALL_MME_DATA_V                                                                               31:0

#endif /* _cl_fermi_compute_b_h_ */
