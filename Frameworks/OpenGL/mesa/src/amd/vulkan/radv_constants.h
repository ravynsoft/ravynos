/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
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

#ifndef RADV_CONSTANTS_H
#define RADV_CONSTANTS_H

#define ATI_VENDOR_ID 0x1002

#define MAX_VBS                        32
#define MAX_VERTEX_ATTRIBS             32
#define MAX_RTS                        8
#define MAX_VIEWPORTS                  16
#define MAX_SCISSORS                   16
#define MAX_DISCARD_RECTANGLES         4
#define MAX_SAMPLE_LOCATIONS           32
#define MAX_PUSH_CONSTANTS_SIZE        256
#define MAX_PUSH_DESCRIPTORS           32
#define MAX_DYNAMIC_UNIFORM_BUFFERS    16
#define MAX_DYNAMIC_STORAGE_BUFFERS    8
#define MAX_DYNAMIC_BUFFERS            (MAX_DYNAMIC_UNIFORM_BUFFERS + MAX_DYNAMIC_STORAGE_BUFFERS)
#define MAX_SAMPLES_LOG2               4
#define NUM_META_FS_KEYS               12
#define RADV_MAX_DRM_DEVICES           8
#define MAX_VIEWS                      8
#define MAX_SO_STREAMS                 4
#define MAX_SO_BUFFERS                 4
#define MAX_SO_OUTPUTS                 128
#define MAX_INLINE_UNIFORM_BLOCK_SIZE  (4ull * 1024 * 1024)
#define MAX_INLINE_UNIFORM_BLOCK_COUNT 64
#define MAX_BIND_POINTS                3 /* compute + graphics + raytracing */

#define NUM_DEPTH_CLEAR_PIPELINES      2
#define NUM_DEPTH_DECOMPRESS_PIPELINES 3
#define MAX_FRAMEBUFFER_WIDTH          (1u << 14)
#define MAX_FRAMEBUFFER_HEIGHT         (1u << 14)

/*
 * This is the point we switch from using CP to compute shader
 * for certain buffer operations.
 */
#define RADV_BUFFER_OPS_CS_THRESHOLD 4096

#define RADV_BUFFER_UPDATE_THRESHOLD 1024

/* descriptor index into scratch ring offsets */
#define RING_SCRATCH             0
#define RING_ESGS_VS             1
#define RING_ESGS_GS             2
#define RING_GSVS_VS             3
#define RING_GSVS_GS             4
#define RING_HS_TESS_FACTOR      5
#define RING_HS_TESS_OFFCHIP     6
#define RING_TS_DRAW             7
#define RING_TS_PAYLOAD          8
#define RING_MS_SCRATCH          9
#define RING_PS_ATTR             10
#define RING_PS_SAMPLE_POSITIONS 11

#define SI_GS_PER_ES 128

/* max number of descriptor sets */
#define MAX_SETS 32

/* Make sure everything is addressable by a signed 32-bit int, and
 * our largest descriptors are 96 bytes.
 */
#define RADV_MAX_PER_SET_DESCRIPTORS ((1ull << 31) / 96)

/* Our buffer size fields allow only 2**32 - 1. We round that down to a multiple
 * of 4 bytes so we can align buffer sizes up.
 */
#define RADV_MAX_MEMORY_ALLOCATION_SIZE 0xFFFFFFFCull

/* Number of entries in the mesh shader scratch ring.
 * This depends on VGT_GS_MAX_WAVE_ID which is set by the kernel
 * and is impossible to query. We leave it on its maximum value
 * because real applications are unlikely to use it.
 *
 * The maximum ID on GFX10.3 is 2047 (0x7ff), so we need 2048 entries.
 */
#define RADV_MESH_SCRATCH_NUM_ENTRIES 2048

/* Size of each entry in the mesh shader scratch ring.
 * We must ensure that the absolute maximum mesh shader output fits here.
 *
 * Mesh shaders can create up to 256 vertices/primitives per workgroup,
 * and up to the following amount of outputs:
 * - 32 parameters
 * - 4 positions (clip/cull distance, etc.)
 * - 4 per-primitive built-in outputs (layer, view index, prim id, VRS rate)
 * - primitive indices which are always kept in LDS
 * That is a total of 32+4+4=40 output slots x 16 bytes per slot x 256 = 160K bytes.
 */
#define RADV_MESH_SCRATCH_ENTRY_BYTES (160 * 1024)

/* Number of invocations in each subgroup. */
#define RADV_SUBGROUP_SIZE 64

/* The spec requires this to be 32. */
#define RADV_RT_HANDLE_SIZE 32

#define RADV_MAX_HIT_ATTRIB_SIZE   32
#define RADV_MAX_HIT_ATTRIB_DWORDS (RADV_MAX_HIT_ATTRIB_SIZE / 4)

#define RADV_SHADER_ALLOC_ALIGNMENT      256
#define RADV_SHADER_ALLOC_MIN_ARENA_SIZE (256 * 1024)
/* 256 KiB << 5 = 8 MiB */
#define RADV_SHADER_ALLOC_MAX_ARENA_SIZE_SHIFT 5u
#define RADV_SHADER_ALLOC_MIN_SIZE_CLASS       8
#define RADV_SHADER_ALLOC_MAX_SIZE_CLASS       15
#define RADV_SHADER_ALLOC_NUM_FREE_LISTS       (RADV_SHADER_ALLOC_MAX_SIZE_CLASS - RADV_SHADER_ALLOC_MIN_SIZE_CLASS + 1)

#define PERF_CTR_MAX_PASSES      512
#define PERF_CTR_BO_PASS_OFFSET  16
#define PERF_CTR_BO_LOCK_OFFSET  0
#define PERF_CTR_BO_FENCE_OFFSET 8

/* The maximum number of in-flight uploads (radv_shader_dma_submission) when asynchronous shader
 * upload is used.
 */
#define RADV_SHADER_UPLOAD_CS_COUNT 32

/* Shader GDS counters:
 *   offset  0| 4| 8|12  - reserved for NGG streamout counters
 *   offset 16           - number of primitives generated by geometry shader invocations
 *   offset 20           - number of geometry shader invocations
 *   offset 24|28|32|36  - generated primitive counter for stream 0|1|2|3
 *   offset 40|44|48|52  - written primitive counter for stream 0|1|2|3
 *
 * Mesh shader GDS counters:
 *   offset 56 - number of mesh shader invocations
 *   offset 60 - number of mesh shader generated primitives
 *
 * Task shader GDS counter:
 *   offset 64 - number of task shader invocations
 */
#define RADV_SHADER_QUERY_GS_PRIM_EMIT_OFFSET     16
#define RADV_SHADER_QUERY_GS_INVOCATION_OFFSET    20
#define RADV_SHADER_QUERY_PRIM_GEN_OFFSET(stream) (24 + stream * 4)
#define RADV_SHADER_QUERY_PRIM_XFB_OFFSET(stream) (40 + stream * 4)
#define RADV_SHADER_QUERY_MS_INVOCATION_OFFSET    56
#define RADV_SHADER_QUERY_MS_PRIM_GEN_OFFSET      60
#define RADV_SHADER_QUERY_TS_INVOCATION_OFFSET    64

/* Number of samples for line smooth lowering (hw requirement). */
#define RADV_NUM_SMOOTH_AA_SAMPLES 4

/* Size of the temporary buffer allocated for transfer queue copy command workarounds.
 * The size is chosen so that it can fit two lines of (1 << 14) blocks at 16 bpp.
 */
#define RADV_SDMA_TRANSFER_TEMP_BYTES (2 * (1 << 14) * 16)

#endif /* RADV_CONSTANTS_H */
