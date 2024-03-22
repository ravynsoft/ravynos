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

#ifndef PVR_SHADER_FACTORY_H
#define PVR_SHADER_FACTORY_H

#include <stdint.h>
#include <stdbool.h>

#include "util/bitpack_helpers.h"
#include "util/bitscan.h"
#include "util/u_math.h"

/* Occlusion query availability writes. */
enum pvr_query_availability_write_pool_const {
   PVR_QUERY_AVAILABILITY_WRITE_INDEX_COUNT,
   PVR_QUERY_AVAILABILITY_WRITE_COUNT,
};

/* Copy query pool results. */
enum pvr_copy_query_pool_const {
   PVR_COPY_QUERY_POOL_RESULTS_INDEX_COUNT,
   PVR_COPY_QUERY_POOL_RESULTS_BASE_ADDRESS_LOW,
   PVR_COPY_QUERY_POOL_RESULTS_BASE_ADDRESS_HIGH,
   PVR_COPY_QUERY_POOL_RESULTS_DEST_STRIDE,
   PVR_COPY_QUERY_POOL_RESULTS_PARTIAL_RESULT_FLAG,
   PVR_COPY_QUERY_POOL_RESULTS_64_BIT_FLAG,
   PVR_COPY_QUERY_POOL_RESULTS_WITH_AVAILABILITY_FLAG,
   PVR_COPY_QUERY_POOL_RESULTS_COUNT,
};

/* Reset query pool. */
enum pvr_reset_query_pool_pool_const {
   PVR_RESET_QUERY_POOL_INDEX_COUNT,
   PVR_RESET_QUERY_POOL_COUNT,
};

/* ClearAttachments. */
enum pvr_clear_attachment_const {
   PVR_CLEAR_ATTACHMENT_CONST_COMPONENT_0 = 0, /* Don't change. Indexes array.
                                                */
   PVR_CLEAR_ATTACHMENT_CONST_COMPONENT_1 = 1, /* Don't change. Indexes array.
                                                */
   PVR_CLEAR_ATTACHMENT_CONST_COMPONENT_2 = 2, /* Don't change. Indexes array.
                                                */
   PVR_CLEAR_ATTACHMENT_CONST_COMPONENT_3 = 3, /* Don't change. Indexes array.
                                                */
   PVR_CLEAR_ATTACHMENT_CONST_TILE_BUFFER_UPPER,
   PVR_CLEAR_ATTACHMENT_CONST_TILE_BUFFER_LOWER,
   PVR_CLEAR_ATTACHMENT_CONST_COUNT,
};

#define PVR_CLEAR_ATTACHMENT_DEST_ID_UNUSED (~0U)

/* 8 + 8 = 16 <- 1 Dword, 8 offsets, to registers/tile buffers
 * 7 + 7 = 14 <- 2 Dwords, 7 offsets, to registers/tile buffers
 * 6 + 6 = 12 <- 3 Dwords, 6 offsets, to registers/tile buffers
 * 5 + 5 = 10 <- 4 Dwords, 5 offsets, to registers/tile buffers
 */
#define PVR_CLEAR_ATTACHMENT_PROGRAM_COUNT 52

/* This defines the max theoretic number of clear attachment programs. In cases
 * where the dword count goes past the number of on-chip on-tile-buffer targets
 * there are unused elements. There are 4 versions for clearing 1..4 dwords, 8
 * versions for clearing offsets 0..7 and 2 versions for clearing either on
 * chip or in memory calculated as 4 * 8 * 2 = 64.
 */
#define PVR_CLEAR_ATTACHMENT_PROGRAM_COUNT_WITH_HOLES 64

/**
 * \brief Returns the index of the clear attachment USC program.
 *
 * For shaders which use output registers "dword_count" is essentially the
 * count of output registers to use, and "offset" is the first output reg to
 * use. E.g. dword_count 3, offset 1, will use o1, o2, o3.
 *
 * For shaders which use tile buffers as the destination "dword_count" is the
 * the amount of dwords to write to the tile buffer and "offset" is the offset
 * at which to start writing at.
 */
static inline uint32_t
pvr_get_clear_attachment_program_index(uint32_t dword_count,
                                       uint32_t offset,
                                       bool uses_tile_buffer)
{
   /* dest        - Clear on chip or in memory.
    * offset      - Clear offset 0..7 .
    * dword_count - Clear from 1..4 dwords.
    */
   const uint32_t dest_start = 0;
   const uint32_t dest_end = 0;

   const uint32_t offset_start = 1;
   const uint32_t offset_end = 3;

   const uint32_t dword_count_start = 4;
   const uint32_t dword_count_end = 5;

   uint32_t idx = 0;

   dword_count -= 1;

   idx |= util_bitpack_uint(uses_tile_buffer, dest_start, dest_end);
   idx |= util_bitpack_uint(offset, offset_start, offset_end);
   idx |= util_bitpack_uint(dword_count, dword_count_start, dword_count_end);

   return idx;
}

enum pvr_spm_load_const {
   SPM_LOAD_CONST_TILE_BUFFER_1_UPPER,
   SPM_LOAD_CONST_TILE_BUFFER_1_LOWER,
   SPM_LOAD_CONST_TILE_BUFFER_2_UPPER,
   SPM_LOAD_CONST_TILE_BUFFER_2_LOWER,
   SPM_LOAD_CONST_TILE_BUFFER_3_UPPER,
   SPM_LOAD_CONST_TILE_BUFFER_3_LOWER,
   /* The following are only available if the core does not have the
    * has_eight_output_registers feature. I.e. only available if the device has
    * 4 output regs.
    */
   SPM_LOAD_CONST_TILE_BUFFER_4_UPPER,
   SPM_LOAD_CONST_TILE_BUFFER_4_LOWER,
   SPM_LOAD_CONST_TILE_BUFFER_5_UPPER,
   SPM_LOAD_CONST_TILE_BUFFER_5_LOWER,
   SPM_LOAD_CONST_TILE_BUFFER_6_UPPER,
   SPM_LOAD_CONST_TILE_BUFFER_6_LOWER,
   SPM_LOAD_CONST_TILE_BUFFER_7_UPPER,
   SPM_LOAD_CONST_TILE_BUFFER_7_LOWER,
};
#define PVR_SPM_LOAD_CONST_COUNT (SPM_LOAD_CONST_TILE_BUFFER_7_LOWER + 1)
#define PVR_SPM_LOAD_DEST_UNUSED ~0

#define PVR_SPM_LOAD_SAMPLES_COUNT 4U

#define PVR_SPM_LOAD_IN_REGS_COUNT 3 /* 1, 2, 4 */
#define PVR_SPM_LOAD_IN_TILE_BUFFERS_COUNT 7 /* 1, 2, 3, 4, 5, 6, 7 */

/* If output_regs == 8
 *    reg_load_programs = 4            # 1, 2, 4, 8
 *    tile_buffer_load_programs = 3    # 1, 2, 3
 * else                                #output_regs == 4
 *    reg_load_programs = 3            # 1, 2, 4
 *    tile_buffer_load_programs = 7    # 1, 2, 3, 4, 5, 6, 7
 *
 * See PVR_SPM_LOAD_IN_BUFFERS_COUNT for where the amount of
 * tile_buffer_load_programs comes from.
 *
 * Tot = sample_count * (reg_load_programs + tile_buffer_load_programs)
 */
/* FIXME: This is currently hard coded for the am62. The Chromebook has 8
 * output regs so the count is different.
 */
#define PVR_SPM_LOAD_PROGRAM_COUNT \
   (PVR_SPM_LOAD_SAMPLES_COUNT *   \
    (PVR_SPM_LOAD_IN_REGS_COUNT + PVR_SPM_LOAD_IN_TILE_BUFFERS_COUNT))

static inline uint32_t pvr_get_spm_load_program_index(uint32_t sample_count,
                                                      uint32_t num_tile_buffers,
                                                      uint32_t num_output_regs)
{
   uint32_t idx;

   assert(util_is_power_of_two_nonzero(sample_count));
   idx = util_logbase2(sample_count) *
         (PVR_SPM_LOAD_IN_REGS_COUNT + PVR_SPM_LOAD_IN_TILE_BUFFERS_COUNT);

   assert((num_tile_buffers > 0) ^ (num_output_regs > 0));

   if (num_output_regs > 0) {
      assert(util_is_power_of_two_nonzero(num_output_regs));
      assert(util_logbase2(num_output_regs) < PVR_SPM_LOAD_IN_REGS_COUNT);
      idx += util_logbase2(num_output_regs);
   } else {
      assert(num_tile_buffers <= PVR_SPM_LOAD_IN_TILE_BUFFERS_COUNT);
      idx += PVR_SPM_LOAD_IN_REGS_COUNT + num_tile_buffers - 1;
   }

   assert(idx < PVR_SPM_LOAD_PROGRAM_COUNT);
   return idx;
}

#endif /* PVR_SHADER_FACTORY_H */
