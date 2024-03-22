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

#ifndef _cl_fermi_memory_to_memory_format_a_h_
#define _cl_fermi_memory_to_memory_format_a_h_

/* AUTO GENERATED FILE -- DO NOT EDIT */
/* Command: ../../class/bin/sw_header.pl fermi_memory_to_memory_format_a */

#include "nvtypes.h"

#define FERMI_MEMORY_TO_MEMORY_FORMAT_A    0x9039

#define NV9039_SET_OBJECT                                                                                  0x0000
#define NV9039_SET_OBJECT_CLASS_ID                                                                           15:0
#define NV9039_SET_OBJECT_ENGINE_ID                                                                         20:16

#define NV9039_NO_OPERATION                                                                                0x0100
#define NV9039_NO_OPERATION_V                                                                                31:0

#define NV9039_SET_NOTIFY_A                                                                                0x0104
#define NV9039_SET_NOTIFY_A_ADDRESS_UPPER                                                                     7:0

#define NV9039_SET_NOTIFY_B                                                                                0x0108
#define NV9039_SET_NOTIFY_B_ADDRESS_LOWER                                                                    31:0

#define NV9039_NOTIFY                                                                                      0x010c
#define NV9039_NOTIFY_TYPE                                                                                   31:0
#define NV9039_NOTIFY_TYPE_WRITE_ONLY                                                                  0x00000000
#define NV9039_NOTIFY_TYPE_WRITE_THEN_AWAKEN                                                           0x00000001

#define NV9039_WAIT_FOR_IDLE                                                                               0x0110
#define NV9039_WAIT_FOR_IDLE_V                                                                               31:0

#define NV9039_LOAD_MME_INSTRUCTION_RAM_POINTER                                                            0x0114
#define NV9039_LOAD_MME_INSTRUCTION_RAM_POINTER_V                                                            31:0

#define NV9039_LOAD_MME_INSTRUCTION_RAM                                                                    0x0118
#define NV9039_LOAD_MME_INSTRUCTION_RAM_V                                                                    31:0

#define NV9039_LOAD_MME_START_ADDRESS_RAM_POINTER                                                          0x011c
#define NV9039_LOAD_MME_START_ADDRESS_RAM_POINTER_V                                                          31:0

#define NV9039_LOAD_MME_START_ADDRESS_RAM                                                                  0x0120
#define NV9039_LOAD_MME_START_ADDRESS_RAM_V                                                                  31:0

#define NV9039_SET_MME_SHADOW_RAM_CONTROL                                                                  0x0124
#define NV9039_SET_MME_SHADOW_RAM_CONTROL_MODE                                                                1:0
#define NV9039_SET_MME_SHADOW_RAM_CONTROL_MODE_METHOD_TRACK                                            0x00000000
#define NV9039_SET_MME_SHADOW_RAM_CONTROL_MODE_METHOD_TRACK_WITH_FILTER                                0x00000001
#define NV9039_SET_MME_SHADOW_RAM_CONTROL_MODE_METHOD_PASSTHROUGH                                      0x00000002
#define NV9039_SET_MME_SHADOW_RAM_CONTROL_MODE_METHOD_REPLAY                                           0x00000003

#define NV9039_SET_GLOBAL_RENDER_ENABLE_A                                                                  0x0130
#define NV9039_SET_GLOBAL_RENDER_ENABLE_A_OFFSET_UPPER                                                        7:0

#define NV9039_SET_GLOBAL_RENDER_ENABLE_B                                                                  0x0134
#define NV9039_SET_GLOBAL_RENDER_ENABLE_B_OFFSET_LOWER                                                       31:0

#define NV9039_SET_GLOBAL_RENDER_ENABLE_C                                                                  0x0138
#define NV9039_SET_GLOBAL_RENDER_ENABLE_C_MODE                                                                2:0
#define NV9039_SET_GLOBAL_RENDER_ENABLE_C_MODE_FALSE                                                   0x00000000
#define NV9039_SET_GLOBAL_RENDER_ENABLE_C_MODE_TRUE                                                    0x00000001
#define NV9039_SET_GLOBAL_RENDER_ENABLE_C_MODE_CONDITIONAL                                             0x00000002
#define NV9039_SET_GLOBAL_RENDER_ENABLE_C_MODE_RENDER_IF_EQUAL                                         0x00000003
#define NV9039_SET_GLOBAL_RENDER_ENABLE_C_MODE_RENDER_IF_NOT_EQUAL                                     0x00000004

#define NV9039_SEND_GO_IDLE                                                                                0x013c
#define NV9039_SEND_GO_IDLE_V                                                                                31:0

#define NV9039_PM_TRIGGER                                                                                  0x0140
#define NV9039_PM_TRIGGER_V                                                                                  31:0

#define NV9039_SET_INSTRUMENTATION_METHOD_HEADER                                                           0x0150
#define NV9039_SET_INSTRUMENTATION_METHOD_HEADER_V                                                           31:0

#define NV9039_SET_INSTRUMENTATION_METHOD_DATA                                                             0x0154
#define NV9039_SET_INSTRUMENTATION_METHOD_DATA_V                                                             31:0

#define NV9039_SET_SRC_BLOCK_SIZE                                                                          0x0204
#define NV9039_SET_SRC_BLOCK_SIZE_WIDTH                                                                       3:0
#define NV9039_SET_SRC_BLOCK_SIZE_WIDTH_ONE_GOB                                                        0x00000000
#define NV9039_SET_SRC_BLOCK_SIZE_HEIGHT                                                                      7:4
#define NV9039_SET_SRC_BLOCK_SIZE_HEIGHT_ONE_GOB                                                       0x00000000
#define NV9039_SET_SRC_BLOCK_SIZE_HEIGHT_TWO_GOBS                                                      0x00000001
#define NV9039_SET_SRC_BLOCK_SIZE_HEIGHT_FOUR_GOBS                                                     0x00000002
#define NV9039_SET_SRC_BLOCK_SIZE_HEIGHT_EIGHT_GOBS                                                    0x00000003
#define NV9039_SET_SRC_BLOCK_SIZE_HEIGHT_SIXTEEN_GOBS                                                  0x00000004
#define NV9039_SET_SRC_BLOCK_SIZE_HEIGHT_THIRTYTWO_GOBS                                                0x00000005
#define NV9039_SET_SRC_BLOCK_SIZE_DEPTH                                                                      11:8
#define NV9039_SET_SRC_BLOCK_SIZE_DEPTH_ONE_GOB                                                        0x00000000
#define NV9039_SET_SRC_BLOCK_SIZE_DEPTH_TWO_GOBS                                                       0x00000001
#define NV9039_SET_SRC_BLOCK_SIZE_DEPTH_FOUR_GOBS                                                      0x00000002
#define NV9039_SET_SRC_BLOCK_SIZE_DEPTH_EIGHT_GOBS                                                     0x00000003
#define NV9039_SET_SRC_BLOCK_SIZE_DEPTH_SIXTEEN_GOBS                                                   0x00000004
#define NV9039_SET_SRC_BLOCK_SIZE_DEPTH_THIRTYTWO_GOBS                                                 0x00000005

#define NV9039_SET_SRC_WIDTH                                                                               0x0208
#define NV9039_SET_SRC_WIDTH_V                                                                               31:0

#define NV9039_SET_SRC_HEIGHT                                                                              0x020c
#define NV9039_SET_SRC_HEIGHT_V                                                                              31:0

#define NV9039_SET_SRC_DEPTH                                                                               0x0210
#define NV9039_SET_SRC_DEPTH_V                                                                               31:0

#define NV9039_SET_SRC_LAYER                                                                               0x0214
#define NV9039_SET_SRC_LAYER_V                                                                               31:0

#define NV9039_SET_DST_BLOCK_SIZE                                                                          0x0220
#define NV9039_SET_DST_BLOCK_SIZE_WIDTH                                                                       3:0
#define NV9039_SET_DST_BLOCK_SIZE_WIDTH_ONE_GOB                                                        0x00000000
#define NV9039_SET_DST_BLOCK_SIZE_HEIGHT                                                                      7:4
#define NV9039_SET_DST_BLOCK_SIZE_HEIGHT_ONE_GOB                                                       0x00000000
#define NV9039_SET_DST_BLOCK_SIZE_HEIGHT_TWO_GOBS                                                      0x00000001
#define NV9039_SET_DST_BLOCK_SIZE_HEIGHT_FOUR_GOBS                                                     0x00000002
#define NV9039_SET_DST_BLOCK_SIZE_HEIGHT_EIGHT_GOBS                                                    0x00000003
#define NV9039_SET_DST_BLOCK_SIZE_HEIGHT_SIXTEEN_GOBS                                                  0x00000004
#define NV9039_SET_DST_BLOCK_SIZE_HEIGHT_THIRTYTWO_GOBS                                                0x00000005
#define NV9039_SET_DST_BLOCK_SIZE_DEPTH                                                                      11:8
#define NV9039_SET_DST_BLOCK_SIZE_DEPTH_ONE_GOB                                                        0x00000000
#define NV9039_SET_DST_BLOCK_SIZE_DEPTH_TWO_GOBS                                                       0x00000001
#define NV9039_SET_DST_BLOCK_SIZE_DEPTH_FOUR_GOBS                                                      0x00000002
#define NV9039_SET_DST_BLOCK_SIZE_DEPTH_EIGHT_GOBS                                                     0x00000003
#define NV9039_SET_DST_BLOCK_SIZE_DEPTH_SIXTEEN_GOBS                                                   0x00000004
#define NV9039_SET_DST_BLOCK_SIZE_DEPTH_THIRTYTWO_GOBS                                                 0x00000005

#define NV9039_SET_DST_WIDTH                                                                               0x0224
#define NV9039_SET_DST_WIDTH_V                                                                               31:0

#define NV9039_SET_DST_HEIGHT                                                                              0x0228
#define NV9039_SET_DST_HEIGHT_V                                                                              31:0

#define NV9039_SET_DST_DEPTH                                                                               0x022c
#define NV9039_SET_DST_DEPTH_V                                                                               31:0

#define NV9039_SET_DST_LAYER                                                                               0x0230
#define NV9039_SET_DST_LAYER_V                                                                               31:0

#define NV9039_OFFSET_OUT_UPPER                                                                            0x0238
#define NV9039_OFFSET_OUT_UPPER_VALUE                                                                         7:0

#define NV9039_OFFSET_OUT                                                                                  0x023c
#define NV9039_OFFSET_OUT_VALUE                                                                              31:0

#define NV9039_SET_SPARE_NOOP06                                                                            0x0240
#define NV9039_SET_SPARE_NOOP06_V                                                                            31:0

#define NV9039_SET_SPARE_NOOP03                                                                            0x0244
#define NV9039_SET_SPARE_NOOP03_V                                                                            31:0

#define NV9039_SET_SPARE_NOOP07                                                                            0x0248
#define NV9039_SET_SPARE_NOOP07_V                                                                            31:0

#define NV9039_SET_SPARE_NOOP05                                                                            0x024c
#define NV9039_SET_SPARE_NOOP05_V                                                                            31:0

#define NV9039_SET_SPARE_NOOP14                                                                            0x0250
#define NV9039_SET_SPARE_NOOP14_V                                                                            31:0

#define NV9039_SET_SPARE_NOOP04                                                                            0x0254
#define NV9039_SET_SPARE_NOOP04_V                                                                            31:0

#define NV9039_SET_SPARE_NOOP10                                                                            0x0258
#define NV9039_SET_SPARE_NOOP10_V                                                                            31:0

#define NV9039_SET_SPARE_NOOP02                                                                            0x025c
#define NV9039_SET_SPARE_NOOP02_V                                                                            31:0

#define NV9039_SET_SPARE_NOOP12                                                                            0x0260
#define NV9039_SET_SPARE_NOOP12_V                                                                            31:0

#define NV9039_SET_SPARE_NOOP00                                                                            0x0264
#define NV9039_SET_SPARE_NOOP00_V                                                                            31:0

#define NV9039_SET_SPARE_NOOP08                                                                            0x0268
#define NV9039_SET_SPARE_NOOP08_V                                                                            31:0

#define NV9039_SET_SPARE_NOOP13                                                                            0x026c
#define NV9039_SET_SPARE_NOOP13_V                                                                            31:0

#define NV9039_SET_SPARE_NOOP09                                                                            0x0270
#define NV9039_SET_SPARE_NOOP09_V                                                                            31:0

#define NV9039_SET_SPARE_NOOP15                                                                            0x0274
#define NV9039_SET_SPARE_NOOP15_V                                                                            31:0

#define NV9039_SET_SPARE_NOOP01                                                                            0x0278
#define NV9039_SET_SPARE_NOOP01_V                                                                            31:0

#define NV9039_SET_FALCON00                                                                                0x027c
#define NV9039_SET_FALCON00_V                                                                                31:0

#define NV9039_SET_FALCON01                                                                                0x0280
#define NV9039_SET_FALCON01_V                                                                                31:0

#define NV9039_SET_FALCON02                                                                                0x0284
#define NV9039_SET_FALCON02_V                                                                                31:0

#define NV9039_SET_FALCON03                                                                                0x0288
#define NV9039_SET_FALCON03_V                                                                                31:0

#define NV9039_SET_FALCON04                                                                                0x028c
#define NV9039_SET_FALCON04_V                                                                                31:0

#define NV9039_SET_FALCON05                                                                                0x0290
#define NV9039_SET_FALCON05_V                                                                                31:0

#define NV9039_SET_FALCON06                                                                                0x0294
#define NV9039_SET_FALCON06_V                                                                                31:0

#define NV9039_SET_FALCON07                                                                                0x0298
#define NV9039_SET_FALCON07_V                                                                                31:0

#define NV9039_SET_FALCON08                                                                                0x029c
#define NV9039_SET_FALCON08_V                                                                                31:0

#define NV9039_SET_FALCON09                                                                                0x02a0
#define NV9039_SET_FALCON09_V                                                                                31:0

#define NV9039_SET_FALCON10                                                                                0x02a4
#define NV9039_SET_FALCON10_V                                                                                31:0

#define NV9039_SET_FALCON11                                                                                0x02a8
#define NV9039_SET_FALCON11_V                                                                                31:0

#define NV9039_SET_FALCON12                                                                                0x02ac
#define NV9039_SET_FALCON12_V                                                                                31:0

#define NV9039_SET_FALCON13                                                                                0x02b0
#define NV9039_SET_FALCON13_V                                                                                31:0

#define NV9039_SET_FALCON14                                                                                0x02b4
#define NV9039_SET_FALCON14_V                                                                                31:0

#define NV9039_SET_FALCON15                                                                                0x02b8
#define NV9039_SET_FALCON15_V                                                                                31:0

#define NV9039_SET_FALCON16                                                                                0x02bc
#define NV9039_SET_FALCON16_V                                                                                31:0

#define NV9039_SET_FALCON17                                                                                0x02c0
#define NV9039_SET_FALCON17_V                                                                                31:0

#define NV9039_SET_FALCON18                                                                                0x02c4
#define NV9039_SET_FALCON18_V                                                                                31:0

#define NV9039_SET_FALCON19                                                                                0x02c8
#define NV9039_SET_FALCON19_V                                                                                31:0

#define NV9039_SET_FALCON20                                                                                0x02cc
#define NV9039_SET_FALCON20_V                                                                                31:0

#define NV9039_SET_FALCON21                                                                                0x02d0
#define NV9039_SET_FALCON21_V                                                                                31:0

#define NV9039_SET_FALCON22                                                                                0x02d4
#define NV9039_SET_FALCON22_V                                                                                31:0

#define NV9039_SET_FALCON23                                                                                0x02d8
#define NV9039_SET_FALCON23_V                                                                                31:0

#define NV9039_SET_FALCON24                                                                                0x02dc
#define NV9039_SET_FALCON24_V                                                                                31:0

#define NV9039_SET_FALCON25                                                                                0x02e0
#define NV9039_SET_FALCON25_V                                                                                31:0

#define NV9039_SET_FALCON26                                                                                0x02e4
#define NV9039_SET_FALCON26_V                                                                                31:0

#define NV9039_SET_FALCON27                                                                                0x02e8
#define NV9039_SET_FALCON27_V                                                                                31:0

#define NV9039_SET_FALCON28                                                                                0x02ec
#define NV9039_SET_FALCON28_V                                                                                31:0

#define NV9039_SET_FALCON29                                                                                0x02f0
#define NV9039_SET_FALCON29_V                                                                                31:0

#define NV9039_SET_FALCON30                                                                                0x02f4
#define NV9039_SET_FALCON30_V                                                                                31:0

#define NV9039_SET_FALCON31                                                                                0x02f8
#define NV9039_SET_FALCON31_V                                                                                31:0

#define NV9039_SET_SPARE_NOOP11                                                                            0x02fc
#define NV9039_SET_SPARE_NOOP11_V                                                                            31:0

#define NV9039_LAUNCH_DMA                                                                                  0x0300
#define NV9039_LAUNCH_DMA_SRC_INLINE                                                                          0:0
#define NV9039_LAUNCH_DMA_SRC_INLINE_FALSE                                                             0x00000000
#define NV9039_LAUNCH_DMA_SRC_INLINE_TRUE                                                              0x00000001
#define NV9039_LAUNCH_DMA_SRC_MEMORY_LAYOUT                                                                   4:4
#define NV9039_LAUNCH_DMA_SRC_MEMORY_LAYOUT_BLOCKLINEAR                                                0x00000000
#define NV9039_LAUNCH_DMA_SRC_MEMORY_LAYOUT_PITCH                                                      0x00000001
#define NV9039_LAUNCH_DMA_DST_MEMORY_LAYOUT                                                                   8:8
#define NV9039_LAUNCH_DMA_DST_MEMORY_LAYOUT_BLOCKLINEAR                                                0x00000000
#define NV9039_LAUNCH_DMA_DST_MEMORY_LAYOUT_PITCH                                                      0x00000001
#define NV9039_LAUNCH_DMA_COMPLETION_TYPE                                                                   13:12
#define NV9039_LAUNCH_DMA_COMPLETION_TYPE_FLUSH_DISABLE                                                0x00000000
#define NV9039_LAUNCH_DMA_COMPLETION_TYPE_FLUSH_ONLY                                                   0x00000001
#define NV9039_LAUNCH_DMA_COMPLETION_TYPE_RELEASE_SEMAPHORE                                            0x00000002
#define NV9039_LAUNCH_DMA_INTERRUPT_TYPE                                                                    17:16
#define NV9039_LAUNCH_DMA_INTERRUPT_TYPE_NONE                                                          0x00000000
#define NV9039_LAUNCH_DMA_INTERRUPT_TYPE_INTERRUPT                                                     0x00000001
#define NV9039_LAUNCH_DMA_SEMAPHORE_STRUCT_SIZE                                                             20:20
#define NV9039_LAUNCH_DMA_SEMAPHORE_STRUCT_SIZE_FOUR_WORDS                                             0x00000000
#define NV9039_LAUNCH_DMA_SEMAPHORE_STRUCT_SIZE_ONE_WORD                                               0x00000001

#define NV9039_LOAD_INLINE_DATA                                                                            0x0304
#define NV9039_LOAD_INLINE_DATA_V                                                                            31:0

#define NV9039_SET_RENDER_ENABLE_OVERRIDE                                                                  0x0308
#define NV9039_SET_RENDER_ENABLE_OVERRIDE_MODE                                                                1:0
#define NV9039_SET_RENDER_ENABLE_OVERRIDE_MODE_USE_RENDER_ENABLE                                       0x00000000
#define NV9039_SET_RENDER_ENABLE_OVERRIDE_MODE_ALWAYS_RENDER                                           0x00000001
#define NV9039_SET_RENDER_ENABLE_OVERRIDE_MODE_NEVER_RENDER                                            0x00000002

#define NV9039_OFFSET_IN_UPPER                                                                             0x030c
#define NV9039_OFFSET_IN_UPPER_VALUE                                                                          7:0

#define NV9039_OFFSET_IN                                                                                   0x0310
#define NV9039_OFFSET_IN_VALUE                                                                               31:0

#define NV9039_PITCH_IN                                                                                    0x0314
#define NV9039_PITCH_IN_VALUE                                                                                31:0

#define NV9039_PITCH_OUT                                                                                   0x0318
#define NV9039_PITCH_OUT_VALUE                                                                               31:0

#define NV9039_LINE_LENGTH_IN                                                                              0x031c
#define NV9039_LINE_LENGTH_IN_VALUE                                                                          31:0

#define NV9039_LINE_COUNT                                                                                  0x0320
#define NV9039_LINE_COUNT_VALUE                                                                              31:0

#define NV9039_SET_SEMAPHORE_A                                                                             0x032c
#define NV9039_SET_SEMAPHORE_A_OFFSET_UPPER                                                                   7:0

#define NV9039_SET_SEMAPHORE_B                                                                             0x0330
#define NV9039_SET_SEMAPHORE_B_OFFSET_LOWER                                                                  31:0

#define NV9039_SET_SEMAPHORE_C                                                                             0x0334
#define NV9039_SET_SEMAPHORE_C_PAYLOAD                                                                       31:0

#define NV9039_SET_RENDER_ENABLE_A                                                                         0x0338
#define NV9039_SET_RENDER_ENABLE_A_OFFSET_UPPER                                                               7:0

#define NV9039_SET_RENDER_ENABLE_B                                                                         0x033c
#define NV9039_SET_RENDER_ENABLE_B_OFFSET_LOWER                                                              31:0

#define NV9039_SET_RENDER_ENABLE_C                                                                         0x0340
#define NV9039_SET_RENDER_ENABLE_C_MODE                                                                       2:0
#define NV9039_SET_RENDER_ENABLE_C_MODE_FALSE                                                          0x00000000
#define NV9039_SET_RENDER_ENABLE_C_MODE_TRUE                                                           0x00000001
#define NV9039_SET_RENDER_ENABLE_C_MODE_CONDITIONAL                                                    0x00000002
#define NV9039_SET_RENDER_ENABLE_C_MODE_RENDER_IF_EQUAL                                                0x00000003
#define NV9039_SET_RENDER_ENABLE_C_MODE_RENDER_IF_NOT_EQUAL                                            0x00000004

#define NV9039_SET_SRC_ORIGIN_BYTES_X                                                                      0x0344
#define NV9039_SET_SRC_ORIGIN_BYTES_X_V                                                                      19:0

#define NV9039_SET_SRC_ORIGIN_SAMPLES_Y                                                                    0x0348
#define NV9039_SET_SRC_ORIGIN_SAMPLES_Y_V                                                                    15:0

#define NV9039_SET_DST_ORIGIN_BYTES_X                                                                      0x034c
#define NV9039_SET_DST_ORIGIN_BYTES_X_V                                                                      19:0

#define NV9039_SET_DST_ORIGIN_SAMPLES_Y                                                                    0x0350
#define NV9039_SET_DST_ORIGIN_SAMPLES_Y_V                                                                    15:0

#define NV9039_SET_MME_SHADOW_SCRATCH(i)                                                           (0x3400+(i)*4)
#define NV9039_SET_MME_SHADOW_SCRATCH_V                                                                      31:0

#define NV9039_CALL_MME_MACRO(j)                                                                   (0x3800+(j)*8)
#define NV9039_CALL_MME_MACRO_V                                                                              31:0

#define NV9039_CALL_MME_DATA(j)                                                                    (0x3804+(j)*8)
#define NV9039_CALL_MME_DATA_V                                                                               31:0

#endif /* _cl_fermi_memory_to_memory_format_a_h_ */
