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

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pvr_device_info.h"
#include "pvr_pds.h"
#include "pvr_rogue_pds_defs.h"
#include "pvr_rogue_pds_disasm.h"
#include "pvr_rogue_pds_encode.h"
#include "util/log.h"
#include "util/macros.h"

#define H32(X) (uint32_t)((((X) >> 32U) & 0xFFFFFFFFUL))
#define L32(X) (uint32_t)(((X)&0xFFFFFFFFUL))

/*****************************************************************************
 Macro definitions
*****************************************************************************/

#define PVR_PDS_DWORD_SHIFT 2

#define PVR_PDS_CONSTANTS_BLOCK_BASE 0
#define PVR_PDS_CONSTANTS_BLOCK_SIZE 128
#define PVR_PDS_TEMPS_BLOCK_BASE 128
#define PVR_PDS_TEMPS_BLOCK_SIZE 32

#define PVR_ROGUE_PDSINST_ST_COUNT4_MAX_SIZE PVR_ROGUE_PDSINST_ST_COUNT4_MASK
#define PVR_ROGUE_PDSINST_LD_COUNT8_MAX_SIZE PVR_ROGUE_PDSINST_LD_COUNT8_MASK

/* Map PDS temp registers to the CDM values they contain Work-group IDs are only
 * available in the coefficient sync task.
 */
#define PVR_PDS_CDM_WORK_GROUP_ID_X 0
#define PVR_PDS_CDM_WORK_GROUP_ID_Y 1
#define PVR_PDS_CDM_WORK_GROUP_ID_Z 2
/* Local IDs are available in every task. */
#define PVR_PDS_CDM_LOCAL_ID_X 0
#define PVR_PDS_CDM_LOCAL_ID_YZ 1

#define PVR_PDS_DOUTW_LOWER32 0x0
#define PVR_PDS_DOUTW_UPPER32 0x1
#define PVR_PDS_DOUTW_LOWER64 0x2
#define PVR_PDS_DOUTW_LOWER128 0x3
#define PVR_PDS_DOUTW_MAXMASK 0x4

#define ROGUE_PDS_FIXED_PIXEL_SHADER_DATA_SIZE 8U
#define PDS_ROGUE_TA_STATE_PDS_ADDR_ALIGNSIZE (16U)

/*****************************************************************************
 Static variables
*****************************************************************************/

static const uint32_t dword_mask_const[PVR_PDS_DOUTW_MAXMASK] = {
   PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_BSIZE_LOWER,
   PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_BSIZE_UPPER,
   PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_BSIZE_ALL64,
   PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_BSIZE_ALL64
};

/* If has_slc_mcu_cache_control is enabled use cache_control_const[0], else use
 * cache_control_const[1].
 */
static const uint32_t cache_control_const[2][2] = {
   { PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_CMODE_BYPASS,
     PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_CMODE_CACHED },
   { 0, 0 }
};

/*****************************************************************************
 Function definitions
*****************************************************************************/

uint64_t pvr_pds_encode_ld_src0(uint64_t dest,
                                uint64_t count8,
                                uint64_t src_add,
                                bool cached,
                                const struct pvr_device_info *dev_info)
{
   uint64_t encoded = 0;

   if (PVR_HAS_FEATURE(dev_info, slc_mcu_cache_controls)) {
      encoded |= (cached ? PVR_ROGUE_PDSINST_LD_LD_SRC0_SLCMODE_CACHED
                         : PVR_ROGUE_PDSINST_LD_LD_SRC0_SLCMODE_BYPASS);
   }

   encoded |= ((src_add & PVR_ROGUE_PDSINST_LD_SRCADD_MASK)
               << PVR_ROGUE_PDSINST_LD_LD_SRC0_SRCADD_SHIFT);
   encoded |= ((count8 & PVR_ROGUE_PDSINST_LD_COUNT8_MASK)
               << PVR_ROGUE_PDSINST_LD_LD_SRC0_COUNT8_SHIFT);
   encoded |= (cached ? PVR_ROGUE_PDSINST_LD_LD_SRC0_CMODE_CACHED
                      : PVR_ROGUE_PDSINST_LD_LD_SRC0_CMODE_BYPASS);
   encoded |= ((dest & PVR_ROGUE_PDSINST_REGS64TP_MASK)
               << PVR_ROGUE_PDSINST_LD_LD_SRC0_DEST_SHIFT);

   return encoded;
}

uint64_t pvr_pds_encode_st_src0(uint64_t src,
                                uint64_t count4,
                                uint64_t dst_add,
                                bool write_through,
                                const struct pvr_device_info *device_info)
{
   uint64_t encoded = 0;

   if (device_info->features.has_slc_mcu_cache_controls) {
      encoded |= (write_through
                     ? PVR_ROGUE_PDSINST_ST_ST_SRC0_SLCMODE_WRITE_THROUGH
                     : PVR_ROGUE_PDSINST_ST_ST_SRC0_SLCMODE_WRITE_BACK);
   }

   encoded |= ((dst_add & PVR_ROGUE_PDSINST_ST_SRCADD_MASK)
               << PVR_ROGUE_PDSINST_ST_ST_SRC0_DSTADD_SHIFT);
   encoded |= ((count4 & PVR_ROGUE_PDSINST_ST_COUNT4_MASK)
               << PVR_ROGUE_PDSINST_ST_ST_SRC0_COUNT4_SHIFT);
   encoded |= (write_through ? PVR_ROGUE_PDSINST_ST_ST_SRC0_CMODE_WRITE_THROUGH
                             : PVR_ROGUE_PDSINST_ST_ST_SRC0_CMODE_WRITE_BACK);
   encoded |= ((src & PVR_ROGUE_PDSINST_REGS32TP_MASK)
               << PVR_ROGUE_PDSINST_ST_ST_SRC0_SRC_SHIFT);

   return encoded;
}

static ALWAYS_INLINE uint32_t
pvr_pds_encode_doutw_src1(uint32_t dest,
                          uint32_t dword_mask,
                          uint32_t flags,
                          bool cached,
                          const struct pvr_device_info *dev_info)
{
   assert(((dword_mask > PVR_PDS_DOUTW_LOWER64) && ((dest & 3) == 0)) ||
          ((dword_mask == PVR_PDS_DOUTW_LOWER64) && ((dest & 1) == 0)) ||
          (dword_mask < PVR_PDS_DOUTW_LOWER64));

   uint32_t encoded =
      (dest << PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_AO_SHIFT);

   encoded |= dword_mask_const[dword_mask];

   encoded |= flags;

   encoded |=
      cache_control_const[PVR_HAS_FEATURE(dev_info, slc_mcu_cache_controls) ? 0
                                                                            : 1]
                         [cached ? 1 : 0];
   return encoded;
}

static ALWAYS_INLINE uint32_t pvr_pds_encode_doutw64(uint32_t cc,
                                                     uint32_t end,
                                                     uint32_t src1,
                                                     uint32_t src0)
{
   return pvr_pds_inst_encode_dout(cc,
                                   end,
                                   src1,
                                   src0,
                                   PVR_ROGUE_PDSINST_DSTDOUT_DOUTW);
}

static ALWAYS_INLINE uint32_t pvr_pds_encode_doutu(uint32_t cc,
                                                   uint32_t end,
                                                   uint32_t src0)
{
   return pvr_pds_inst_encode_dout(cc,
                                   end,
                                   0,
                                   src0,
                                   PVR_ROGUE_PDSINST_DSTDOUT_DOUTU);
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_doutc(uint32_t cc,
                                                        uint32_t end)
{
   return pvr_pds_inst_encode_dout(cc,
                                   end,
                                   0,
                                   0,
                                   PVR_ROGUE_PDSINST_DSTDOUT_DOUTC);
}

static ALWAYS_INLINE uint32_t pvr_pds_encode_doutd(uint32_t cc,
                                                   uint32_t end,
                                                   uint32_t src1,
                                                   uint32_t src0)
{
   return pvr_pds_inst_encode_dout(cc,
                                   end,
                                   src1,
                                   src0,
                                   PVR_ROGUE_PDSINST_DSTDOUT_DOUTD);
}

static ALWAYS_INLINE uint32_t pvr_pds_encode_douti(uint32_t cc,
                                                   uint32_t end,
                                                   uint32_t src0)
{
   return pvr_pds_inst_encode_dout(cc,
                                   end,
                                   0,
                                   src0,
                                   PVR_ROGUE_PDSINST_DSTDOUT_DOUTI);
}

static ALWAYS_INLINE uint32_t pvr_pds_encode_bra(uint32_t srcc,
                                                 uint32_t neg,
                                                 uint32_t setc,
                                                 int32_t relative_address)
{
   /* Address should be signed but API only allows unsigned value. */
   return pvr_pds_inst_encode_bra(srcc, neg, setc, (uint32_t)relative_address);
}

/**
 * Gets the next constant address and moves the next constant pointer along.
 *
 * \param next_constant Pointer to the next constant address.
 * \param num_constants The number of constants required.
 * \param count The number of constants allocated.
 * \return The address of the next constant.
 */
static uint32_t pvr_pds_get_constants(uint32_t *next_constant,
                                      uint32_t num_constants,
                                      uint32_t *count)
{
   uint32_t constant;

   /* Work out starting constant number. For even number of constants, start on
    * a 64-bit boundary.
    */
   if (num_constants & 1)
      constant = *next_constant;
   else
      constant = (*next_constant + 1) & ~1;

   /* Update the count with the number of constants actually allocated. */
   *count += constant + num_constants - *next_constant;

   /* Move the next constant pointer. */
   *next_constant = constant + num_constants;

   assert((constant + num_constants) <= PVR_PDS_CONSTANTS_BLOCK_SIZE);

   return constant;
}

/**
 * Gets the next temp address and moves the next temp pointer along.
 *
 * \param next_temp Pointer to the next temp address.
 * \param num_temps The number of temps required.
 * \param count The number of temps allocated.
 * \return The address of the next temp.
 */
static uint32_t
pvr_pds_get_temps(uint32_t *next_temp, uint32_t num_temps, uint32_t *count)
{
   uint32_t temp;

   /* Work out starting temp number. For even number of temps, start on a
    * 64-bit boundary.
    */
   if (num_temps & 1)
      temp = *next_temp;
   else
      temp = (*next_temp + 1) & ~1;

   /* Update the count with the number of temps actually allocated. */
   *count += temp + num_temps - *next_temp;

   /* Move the next temp pointer. */
   *next_temp = temp + num_temps;

   assert((temp + num_temps) <=
          (PVR_PDS_TEMPS_BLOCK_SIZE + PVR_PDS_TEMPS_BLOCK_BASE));

   return temp;
}

/**
 * Write a 32-bit constant indexed by the long range.
 *
 * \param data_block Pointer to data block to write to.
 * \param index Index within the data to write to.
 * \param dword The 32-bit constant to write.
 */
static void
pvr_pds_write_constant32(uint32_t *data_block, uint32_t index, uint32_t dword0)
{
   /* Check range. */
   assert(index <= (PVR_ROGUE_PDSINST_REGS32_CONST32_UPPER -
                    PVR_ROGUE_PDSINST_REGS32_CONST32_LOWER));

   data_block[index + 0] = dword0;

   PVR_PDS_PRINT_DATA("WriteConstant32", (uint64_t)dword0, index);
}

/**
 * Write a 64-bit constant indexed by the long range.
 *
 * \param data_block Pointer to data block to write to.
 * \param index Index within the data to write to.
 * \param dword0 Lower half of the 64 bit constant.
 * \param dword1 Upper half of the 64 bit constant.
 */
static void pvr_pds_write_constant64(uint32_t *data_block,
                                     uint32_t index,
                                     uint32_t dword0,
                                     uint32_t dword1)
{
   /* Has to be on 64 bit boundary. */
   assert((index & 1) == 0);

   /* Check range. */
   assert((index >> 1) <= (PVR_ROGUE_PDSINST_REGS64_CONST64_UPPER -
                           PVR_ROGUE_PDSINST_REGS64_CONST64_LOWER));

   data_block[index + 0] = dword0;
   data_block[index + 1] = dword1;

   PVR_PDS_PRINT_DATA("WriteConstant64",
                      ((uint64_t)dword0 << 32) | (uint64_t)dword1,
                      index);
}

/**
 * Write a 64-bit constant from a single wide word indexed by the long-range
 * number.
 *
 * \param data_block Pointer to data block to write to.
 * \param index Index within the data to write to.
 * \param word The 64-bit constant to write.
 */

static void
pvr_pds_write_wide_constant(uint32_t *data_block, uint32_t index, uint64_t word)
{
   /* Has to be on 64 bit boundary. */
   assert((index & 1) == 0);

   /* Check range. */
   assert((index >> 1) <= (PVR_ROGUE_PDSINST_REGS64_CONST64_UPPER -
                           PVR_ROGUE_PDSINST_REGS64_CONST64_LOWER));

   data_block[index + 0] = L32(word);
   data_block[index + 1] = H32(word);

   PVR_PDS_PRINT_DATA("WriteWideConstant", word, index);
}

static void pvr_pds_write_dma_address(uint32_t *data_block,
                                      uint32_t index,
                                      uint64_t address,
                                      bool coherent,
                                      const struct pvr_device_info *dev_info)
{
   /* Has to be on 64 bit boundary. */
   assert((index & 1) == 0);

   if (PVR_HAS_FEATURE(dev_info, slc_mcu_cache_controls))
      address |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTD_SRC0_SLCMODE_CACHED;

   /* Check range. */
   assert((index >> 1) <= (PVR_ROGUE_PDSINST_REGS64_CONST64_UPPER -
                           PVR_ROGUE_PDSINST_REGS64_CONST64_LOWER));

   data_block[index + 0] = L32(address);
   data_block[index + 1] = H32(address);

   PVR_PDS_PRINT_DATA("WriteDMAAddress", address, index);
}

/**
 * External API to append a 64-bit constant to an existing data segment
 * allocation.
 *
 * \param constants Pointer to start of data segment.
 * \param constant_value Value to write to constant.
 * \param data_size The number of constants allocated.
 * \returns The address of the next constant.
 */
uint32_t pvr_pds_append_constant64(uint32_t *constants,
                                   uint64_t constant_value,
                                   uint32_t *data_size)
{
   /* Calculate next constant from current data size. */
   uint32_t next_constant = *data_size;
   uint32_t constant = pvr_pds_get_constants(&next_constant, 2, data_size);

   /* Set the value. */
   pvr_pds_write_wide_constant(constants, constant, constant_value);

   return constant;
}

void pvr_pds_pixel_shader_sa_initialize(
   struct pvr_pds_pixel_shader_sa_program *program)
{
   memset(program, 0, sizeof(*program));
}

/**
 * Encode a DMA burst.
 *
 * \param dma_control DMA control words.
 * \param dma_address DMA address.
 * \param dest_offset Destination offset in the attribute.
 * \param dma_size The size of the DMA in words.
 * \param src_address Source address for the burst.
 * \param last Last DMA in program.
 * \param dev_info PVR device info structure.
 * \returns The number of DMA transfers required.
 */
uint32_t pvr_pds_encode_dma_burst(uint32_t *dma_control,
                                  uint64_t *dma_address,
                                  uint32_t dest_offset,
                                  uint32_t dma_size,
                                  uint64_t src_address,
                                  bool last,
                                  const struct pvr_device_info *dev_info)
{
   dma_control[0] = dma_size
                    << PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTD_SRC1_BSIZE_SHIFT;
   dma_control[0] |= dest_offset
                     << PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTD_SRC1_AO_SHIFT;

   dma_control[0] |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTD_SRC1_CMODE_CACHED |
                     PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTD_SRC1_DEST_COMMON_STORE;

   if (last)
      dma_control[0] |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTD_SRC1_LAST_EN;

   dma_address[0] = src_address;
   if (PVR_HAS_FEATURE(dev_info, slc_mcu_cache_controls))
      dma_address[0] |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTD_SRC0_SLCMODE_CACHED;

   /* Force to 1 DMA. */
   return 1;
}

/* FIXME: use the csbgen interface and pvr_csb_pack.
 * FIXME: use bool for phase_rate_change.
 */
/**
 * Sets up the USC control words for a DOUTU.
 *
 * \param usc_task_control USC task control structure to be setup.
 * \param execution_address USC execution virtual address.
 * \param usc_temps Number of USC temps.
 * \param sample_rate Sample rate for the DOUTU.
 * \param phase_rate_change Phase rate change for the DOUTU.
 */
void pvr_pds_setup_doutu(struct pvr_pds_usc_task_control *usc_task_control,
                         uint64_t execution_address,
                         uint32_t usc_temps,
                         uint32_t sample_rate,
                         bool phase_rate_change)
{
   usc_task_control->src0 = UINT64_C(0);

   /* Set the execution address. */
   pvr_set_usc_execution_address64(&(usc_task_control->src0),
                                   execution_address);

   if (usc_temps > 0) {
      /* Temps are allocated in blocks of 4 dwords. */
      usc_temps =
         DIV_ROUND_UP(usc_temps,
                      PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTU_SRC0_TEMPS_ALIGNSIZE);

      /* Check for losing temps due to too many requested. */
      assert((usc_temps & PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTU_TEMPS_MASK) ==
             usc_temps);

      usc_task_control->src0 |=
         ((uint64_t)(usc_temps &
                     PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTU_TEMPS_MASK))
         << PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTU_SRC0_TEMPS_SHIFT;
   }

   if (sample_rate > 0) {
      usc_task_control->src0 |=
         ((uint64_t)sample_rate)
         << PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTU_SRC0_SAMPLE_RATE_SHIFT;
   }

   if (phase_rate_change) {
      usc_task_control->src0 |=
         PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTU_SRC0_DUAL_PHASE_EN;
   }
}

/**
 * Generates the PDS pixel event program.
 *
 * \param program Pointer to the PDS pixel event program.
 * \param buffer Pointer to the buffer for the program.
 * \param gen_mode Generate either a data segment or code segment.
 * \param dev_info PVR device info structure.
 * \returns Pointer to just beyond the buffer for the program.
 */
uint32_t *
pvr_pds_generate_pixel_event(struct pvr_pds_event_program *restrict program,
                             uint32_t *restrict buffer,
                             enum pvr_pds_generate_mode gen_mode,
                             const struct pvr_device_info *dev_info)
{
   uint32_t next_constant = PVR_PDS_CONSTANTS_BLOCK_BASE;
   uint32_t *constants = buffer;

   uint32_t data_size = 0;

   /* Copy the DMA control words and USC task control words to constants, then
    * arrange them so that the 64-bit words are together followed by the 32-bit
    * words.
    */
   uint32_t control_constant =
      pvr_pds_get_constants(&next_constant, 2, &data_size);
   uint32_t emit_constant =
      pvr_pds_get_constants(&next_constant,
                            (2 * program->num_emit_word_pairs),
                            &data_size);

   uint32_t control_word_constant =
      pvr_pds_get_constants(&next_constant,
                            program->num_emit_word_pairs,
                            &data_size);

   if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
      /* Src0 for DOUTU. */
      pvr_pds_write_wide_constant(buffer,
                                  control_constant,
                                  program->task_control.src0); /* DOUTU */
      /* 64-bit Src0. */

      /* Emit words for end of tile program. */
      for (uint32_t i = 0; i < program->num_emit_word_pairs; i++) {
         pvr_pds_write_constant64(constants,
                                  emit_constant + (2 * i),
                                  program->emit_words[(2 * i) + 0],
                                  program->emit_words[(2 * i) + 1]);
      }

      /* Control words. */
      for (uint32_t i = 0; i < program->num_emit_word_pairs; i++) {
         uint32_t doutw = pvr_pds_encode_doutw_src1(
            (2 * i),
            PVR_PDS_DOUTW_LOWER64,
            PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_COMMON_STORE,
            false,
            dev_info);

         if (i == (program->num_emit_word_pairs - 1))
            doutw |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN;

         pvr_pds_write_constant32(constants, control_word_constant + i, doutw);
      }
   }

   else if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
      /* DOUTW the state into the shared register. */
      for (uint32_t i = 0; i < program->num_emit_word_pairs; i++) {
         *buffer++ = pvr_pds_encode_doutw64(
            /* cc */ 0,
            /* END */ 0,
            /* SRC1 */ (control_word_constant + i), /* DOUTW 32-bit Src1 */
            /* SRC0 */ (emit_constant + (2 * i)) >> 1); /* DOUTW 64-bit Src0
                                                         */
      }

      /* Kick the USC. */
      *buffer++ = pvr_pds_encode_doutu(
         /* cc */ 0,
         /* END */ 1,
         /* SRC0 */ control_constant >> 1);
   }

   uint32_t code_size = 1 + program->num_emit_word_pairs;

   /* Save the data segment Pointer and size. */
   program->data_segment = constants;
   program->data_size = data_size;
   program->code_size = code_size;

   if (gen_mode == PDS_GENERATE_DATA_SEGMENT)
      return (constants + next_constant);

   if (gen_mode == PDS_GENERATE_CODE_SEGMENT)
      return buffer;

   return NULL;
}

/**
 * Checks if any of the vertex streams contains instance data.
 *
 * \param streams Streams contained in the vertex shader.
 * \param num_streams Number of vertex streams.
 * \returns true if one or more of the given vertex streams contains
 *          instance data, otherwise false.
 */
static bool pvr_pds_vertex_streams_contains_instance_data(
   const struct pvr_pds_vertex_stream *streams,
   uint32_t num_streams)
{
   for (uint32_t i = 0; i < num_streams; i++) {
      const struct pvr_pds_vertex_stream *vertex_stream = &streams[i];
      if (vertex_stream->instance_data)
         return true;
   }

   return false;
}

static uint32_t pvr_pds_get_bank_based_constants(uint32_t num_backs,
                                                 uint32_t *next_constant,
                                                 uint32_t num_constants,
                                                 uint32_t *count)
{
   /* Allocate constant for PDS vertex shader where constant is divided into
    * banks.
    */
   uint32_t constant;

   assert(num_constants == 1 || num_constants == 2);

   if (*next_constant >= (num_backs << 3))
      return pvr_pds_get_constants(next_constant, num_constants, count);

   if ((*next_constant % 8) == 0) {
      constant = *next_constant;

      if (num_constants == 1)
         *next_constant += 1;
      else
         *next_constant += 8;
   } else if (num_constants == 1) {
      constant = *next_constant;
      *next_constant += 7;
   } else {
      *next_constant += 7;
      constant = *next_constant;

      if (*next_constant >= (num_backs << 3)) {
         *next_constant += 2;
         *count += 2;
      } else {
         *next_constant += 8;
      }
   }
   return constant;
}

/**
 * Generates a PDS program to load USC vertex inputs based from one or more
 * vertex buffers, each containing potentially multiple elements, and then a
 * DOUTU to execute the USC.
 *
 * \param program Pointer to the description of the program which should be
 *                generated.
 * \param buffer Pointer to buffer that receives the output of this function.
 *               Will either be the data segment or code segment depending on
 *               gen_mode.
 * \param gen_mode Which part to generate, either data segment or
 *                 code segment. If PDS_GENERATE_SIZES is specified, nothing is
 *                 written, but size information in program is updated.
 * \param dev_info PVR device info structure.
 * \returns Pointer to just beyond the buffer for the data - i.e the value
 *          of the buffer after writing its contents.
 */
/* FIXME: Implement PDS_GENERATE_CODEDATA_SEGMENTS? */
uint32_t *
pvr_pds_vertex_shader(struct pvr_pds_vertex_shader_program *restrict program,
                      uint32_t *restrict buffer,
                      enum pvr_pds_generate_mode gen_mode,
                      const struct pvr_device_info *dev_info)
{
   uint32_t next_constant = PVR_PDS_CONSTANTS_BLOCK_BASE;
   uint32_t next_stream_constant;
   uint32_t next_temp;
   uint32_t usc_control_constant64;
   uint32_t stride_constant32 = 0;
   uint32_t dma_address_constant64 = 0;
   uint32_t dma_control_constant64;
   uint32_t multiplier_constant32 = 0;
   uint32_t base_instance_const32 = 0;

   uint32_t temp = 0;
   uint32_t index_temp64 = 0;
   uint32_t num_vertices_temp64 = 0;
   uint32_t pre_index_temp = (uint32_t)(-1);
   bool first_ddmadt = true;
   uint32_t input_register0;
   uint32_t input_register1;
   uint32_t input_register2;

   struct pvr_pds_vertex_stream *vertex_stream;
   struct pvr_pds_vertex_element *vertex_element;
   uint32_t shift_2s_comp;

   uint32_t data_size = 0;
   uint32_t code_size = 0;
   uint32_t temps_used = 0;

   bool direct_writes_needed = false;

   uint32_t consts_size = 0;
   uint32_t vertex_id_control_word_const32 = 0;
   uint32_t instance_id_control_word_const32 = 0;
   uint32_t instance_id_modifier_word_const32 = 0;
   uint32_t geometry_id_control_word_const64 = 0;
   uint32_t empty_dma_control_constant64 = 0;

   bool any_instanced_stream =
      pvr_pds_vertex_streams_contains_instance_data(program->streams,
                                                    program->num_streams);

   uint32_t base_instance_register = 0;
   uint32_t ddmadt_enables = 0;

   bool issue_empty_ddmad = false;
   uint32_t last_stream_index = program->num_streams - 1;
   bool current_p0 = false;
   uint32_t skip_stream_flag = 0;

   /* Generate the PDS vertex shader data. */

#if defined(DEBUG)
   if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
      for (uint32_t i = 0; i < program->data_size; i++)
         buffer[i] = 0xDEADBEEF;
   }
#endif

   /* Generate the PDS vertex shader program */
   next_temp = PVR_PDS_TEMPS_BLOCK_BASE;
   /* IR0 is in first 32-bit temp, temp[0].32, vertex_Index. */
   input_register0 = pvr_pds_get_temps(&next_temp, 1, &temps_used);
   /* IR1 is in second 32-bit temp, temp[1].32, instance_ID. */
   input_register1 = pvr_pds_get_temps(&next_temp, 1, &temps_used);

   if (program->iterate_remap_id)
      input_register2 = pvr_pds_get_temps(&next_temp, 1, &temps_used);
   else
      input_register2 = 0; /* Not used, but need to silence the compiler. */

   /* Generate the PDS vertex shader code. The constants in the data block are
    * arranged as follows:
    *
    * 64 bit bank 0        64 bit bank 1          64 bit bank 2    64 bit bank
    * 3 Not used (tmps)    Stride | Multiplier    Address          Control
    */

   /* Find out how many constants are needed by streams. */
   for (uint32_t stream = 0; stream < program->num_streams; stream++) {
      pvr_pds_get_constants(&next_constant,
                            8 * program->streams[stream].num_elements,
                            &consts_size);
   }

   /* If there are no vertex streams allocate the first bank for USC Code
    * Address.
    */
   if (consts_size == 0)
      pvr_pds_get_constants(&next_constant, 2, &consts_size);
   else
      next_constant = 8;

   direct_writes_needed = program->iterate_instance_id ||
                          program->iterate_vtx_id || program->iterate_remap_id;

   if (!PVR_HAS_FEATURE(dev_info, pds_ddmadt)) {
      /* Evaluate what config of DDMAD should be used for each stream. */
      for (uint32_t stream = 0; stream < program->num_streams; stream++) {
         vertex_stream = &program->streams[stream];

         if (vertex_stream->use_ddmadt) {
            ddmadt_enables |= (1 << stream);

            /* The condition for index value is:
             * index * stride + size <= bufferSize (all in unit of byte)
             */
            if (vertex_stream->stride == 0) {
               if (vertex_stream->elements[0].size <=
                   vertex_stream->buffer_size_in_bytes) {
                  /* index can be any value -> no need to use DDMADT. */
                  ddmadt_enables &= (~(1 << stream));
               } else {
                  /* No index works -> no need to issue DDMAD instruction.
                   */
                  skip_stream_flag |= (1 << stream);
               }
            } else {
               /* index * stride + size <= bufferSize
                *
                * can be converted to:
                * index <= (bufferSize - size) / stride
                *
                * where maximum index is:
                * integer((bufferSize - size) / stride).
                */
               if (vertex_stream->buffer_size_in_bytes <
                   vertex_stream->elements[0].size) {
                  /* No index works -> no need to issue DDMAD instruction.
                   */
                  skip_stream_flag |= (1 << stream);
               } else {
                  uint32_t max_index = (vertex_stream->buffer_size_in_bytes -
                                        vertex_stream->elements[0].size) /
                                       vertex_stream->stride;
                  if (max_index == 0xFFFFFFFFu) {
                     /* No need to use DDMADT as all possible indices can
                      * pass the test.
                      */
                     ddmadt_enables &= (~(1 << stream));
                  } else {
                     /* In this case, test condition can be changed to
                      * index < max_index + 1.
                      */
                     program->streams[stream].num_vertices =
                        pvr_pds_get_bank_based_constants(program->num_streams,
                                                         &next_constant,
                                                         1,
                                                         &consts_size);

                     if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
                        pvr_pds_write_constant32(
                           buffer,
                           program->streams[stream].num_vertices,
                           max_index + 1);
                     }
                  }
               }
            }
         }

         if ((skip_stream_flag & (1 << stream)) == 0) {
            issue_empty_ddmad = (ddmadt_enables & (1 << stream)) != 0;
            last_stream_index = stream;
         }
      }
   } else {
      if (program->num_streams > 0 &&
          program->streams[program->num_streams - 1].use_ddmadt) {
         issue_empty_ddmad = true;
      }
   }

   if (direct_writes_needed)
      issue_empty_ddmad = false;

   if (issue_empty_ddmad) {
      /* An empty DMA control const (DMA size = 0) is required in case the
       * last DDMADD is predicated out and last flag does not have any usage.
       */
      empty_dma_control_constant64 =
         pvr_pds_get_bank_based_constants(program->num_streams,
                                          &next_constant,
                                          2,
                                          &consts_size);
   }

   /* Assign constants for non stream or base instance if there is any
    * instanced stream.
    */
   if (direct_writes_needed || any_instanced_stream ||
       program->instance_id_modifier) {
      if (program->iterate_vtx_id) {
         vertex_id_control_word_const32 =
            pvr_pds_get_bank_based_constants(program->num_streams,
                                             &next_constant,
                                             1,
                                             &consts_size);
      }

      if (program->iterate_instance_id || program->instance_id_modifier) {
         if (program->instance_id_modifier == 0) {
            instance_id_control_word_const32 =
               pvr_pds_get_bank_based_constants(program->num_streams,
                                                &next_constant,
                                                1,
                                                &consts_size);
         } else {
            instance_id_modifier_word_const32 =
               pvr_pds_get_bank_based_constants(program->num_streams,
                                                &next_constant,
                                                1,
                                                &consts_size);
            if ((instance_id_modifier_word_const32 % 2) == 0) {
               instance_id_control_word_const32 =
                  pvr_pds_get_bank_based_constants(program->num_streams,
                                                   &next_constant,
                                                   1,
                                                   &consts_size);
            } else {
               instance_id_control_word_const32 =
                  instance_id_modifier_word_const32;
               instance_id_modifier_word_const32 =
                  pvr_pds_get_bank_based_constants(program->num_streams,
                                                   &next_constant,
                                                   1,
                                                   &consts_size);
            }
         }
      }

      if (program->base_instance != 0) {
         base_instance_const32 =
            pvr_pds_get_bank_based_constants(program->num_streams,
                                             &next_constant,
                                             1,
                                             &consts_size);
      }

      if (program->iterate_remap_id) {
         geometry_id_control_word_const64 =
            pvr_pds_get_bank_based_constants(program->num_streams,
                                             &next_constant,
                                             2,
                                             &consts_size);
      }
   }

   if (program->instance_id_modifier != 0) {
      /* This instanceID modifier is used when a draw array instanced call
       * sourcing from client data cannot fit into vertex buffer and needs to
       * be broken down into several draw calls.
       */

      code_size += 1;

      if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
         pvr_pds_write_constant32(buffer,
                                  instance_id_modifier_word_const32,
                                  program->instance_id_modifier);
      } else if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
         *buffer++ = pvr_pds_inst_encode_add32(
            /* cc */ 0x0,
            /* ALUM */ 0, /* Unsigned */
            /* SNA */ 0, /* Add */
            /* SRC0 32b */ instance_id_modifier_word_const32,
            /* SRC1 32b */ input_register1,
            /* DST 32b */ input_register1);
      }
   }

   /* Adjust instanceID if necessary. */
   if (any_instanced_stream || program->iterate_instance_id) {
      if (program->base_instance != 0) {
         assert(!program->draw_indirect);

         if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
            pvr_pds_write_constant32(buffer,
                                     base_instance_const32,
                                     program->base_instance);
         }

         base_instance_register = base_instance_const32;
      }

      if (program->draw_indirect) {
         assert((program->instance_id_modifier == 0) &&
                (program->base_instance == 0));

         base_instance_register = PVR_ROGUE_PDSINST_REGS32_PTEMP32_LOWER + 1;
      }
   }

   next_constant = next_stream_constant = PVR_PDS_CONSTANTS_BLOCK_BASE;
   usc_control_constant64 =
      pvr_pds_get_constants(&next_stream_constant, 2, &data_size);

   for (uint32_t stream = 0; stream < program->num_streams; stream++) {
      bool instance_data_with_base_instance;

      if ((!PVR_HAS_FEATURE(dev_info, pds_ddmadt)) &&
          ((skip_stream_flag & (1 << stream)) != 0)) {
         continue;
      }

      vertex_stream = &program->streams[stream];

      instance_data_with_base_instance =
         ((vertex_stream->instance_data) &&
          ((program->base_instance > 0) || (program->draw_indirect)));

      /* Get all 8 32-bit constants at once, only 6 for first stream due to
       * USC constants.
       */
      if (stream == 0) {
         stride_constant32 =
            pvr_pds_get_constants(&next_stream_constant, 6, &data_size);
      } else {
         next_constant =
            pvr_pds_get_constants(&next_stream_constant, 8, &data_size);

         /* Skip bank 0. */
         stride_constant32 = next_constant + 2;
      }

      multiplier_constant32 = stride_constant32 + 1;

      if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
         pvr_pds_write_constant32(buffer,
                                  stride_constant32,
                                  vertex_stream->stride);

         /* Vertex stream frequency multiplier. */
         if (vertex_stream->multiplier)
            pvr_pds_write_constant32(buffer,
                                     multiplier_constant32,
                                     vertex_stream->multiplier);
      }

      /* Update the code size count and temps count for the above code
       * segment.
       */
      if (vertex_stream->current_state) {
         code_size += 1;
         temp = pvr_pds_get_temps(&next_temp, 1, &temps_used); /* 32-bit */
      } else {
         unsigned int num_temps_required = 0;

         if (vertex_stream->multiplier) {
            num_temps_required += 2;
            code_size += 3;

            if (vertex_stream->shift) {
               code_size += 1;

               if ((int32_t)vertex_stream->shift > 0)
                  code_size += 1;
            }
         } else if (vertex_stream->shift) {
            code_size += 1;
            num_temps_required += 1;
         } else if (instance_data_with_base_instance) {
            num_temps_required += 1;
         }

         if (num_temps_required != 0) {
            temp = pvr_pds_get_temps(&next_temp,
                                     num_temps_required,
                                     &temps_used); /* 64-bit */
         } else {
            temp = vertex_stream->instance_data ? input_register1
                                                : input_register0;
         }

         if (instance_data_with_base_instance)
            code_size += 1;
      }

      /* The real code segment. */
      if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
         /* If it's current state stream, then index = 0 always. */
         if (vertex_stream->current_state) {
            /* Put zero in temp. */
            *buffer++ = pvr_pds_inst_encode_limm(0, temp, 0, 0);
         } else if (vertex_stream->multiplier) {
            /* old: Iout = (Iin * (Multiplier+2^24)) >> (Shift+24)
             * new: Iout = (Iin * Multiplier) >> (shift+31)
             */

            /* Put zero in temp. Need zero for add part of the following
             * MAD. MAD source is 64 bit, so need two LIMMs.
             */
            *buffer++ = pvr_pds_inst_encode_limm(0, temp, 0, 0);
            /* Put zero in temp. Need zero for add part of the following
             * MAD.
             */
            *buffer++ = pvr_pds_inst_encode_limm(0, temp + 1, 0, 0);

            /* old: (Iin * (Multiplier+2^24))
             * new: (Iin * Multiplier)
             */
            *buffer++ = pvr_rogue_inst_encode_mad(
               0, /* Sign of add is positive. */
               0, /* Unsigned ALU mode */
               0, /* Unconditional */
               multiplier_constant32,
               vertex_stream->instance_data ? input_register1 : input_register0,
               temp / 2,
               temp / 2);

            if (vertex_stream->shift) {
               int32_t shift = (int32_t)vertex_stream->shift;

               /* new: >> (shift + 31) */
               shift += 31;
               shift *= -1;

               if (shift < -31) {
                  /* >> (31) */
                  shift_2s_comp = 0xFFFE1;
                  *buffer++ = pvr_pds_inst_encode_sftlp64(
                     /* cc */ 0,
                     /* LOP */ PVR_ROGUE_PDSINST_LOP_NONE,
                     /* IM */ 1, /*  enable immediate */
                     /* SRC0 */ temp / 2,
                     /* SRC1 */ input_register0, /* This won't be used in
                                                  * a shift operation.
                                                  */
                     /* SRC2 (Shift) */ shift_2s_comp,
                     /* DST */ temp / 2);
                  shift += 31;
               }

               /* old: >> (Shift+24)
                * new: >> (shift + 31)
                */
               shift_2s_comp = *((uint32_t *)&shift);
               *buffer++ = pvr_pds_inst_encode_sftlp64(
                  /* cc */ 0,
                  /* LOP */ PVR_ROGUE_PDSINST_LOP_NONE,
                  /* IM */ 1, /*enable immediate */
                  /* SRC0 */ temp / 2,
                  /* SRC1 */ input_register0, /* This won't be used in
                                               * a shift operation.
                                               */
                  /* SRC2 (Shift) */ shift_2s_comp,
                  /* DST */ temp / 2);
            }

            if (instance_data_with_base_instance) {
               *buffer++ =
                  pvr_pds_inst_encode_add32(0, /* cc */
                                            0, /* ALNUM */
                                            0, /* SNA */
                                            base_instance_register, /* src0
                                                                     */
                                            temp, /* src1 */
                                            temp /* dst */
                  );
            }
         } else { /* NOT vertex_stream->multiplier */
            if (vertex_stream->shift) {
               /* Shift Index/InstanceNum Right by shift bits. Put result
                * in a Temp.
                */

               /* 2's complement of shift as this will be a right shift. */
               shift_2s_comp = ~(vertex_stream->shift) + 1;

               *buffer++ = pvr_pds_inst_encode_sftlp32(
                  /* IM */ 1, /*  enable immediate. */
                  /* cc */ 0,
                  /* LOP */ PVR_ROGUE_PDSINST_LOP_NONE,
                  /* SRC0 */ vertex_stream->instance_data ? input_register1
                                                          : input_register0,
                  /* SRC1 */ input_register0, /* This won't be used in
                                               * a shift operation.
                                               */
                  /* SRC2 (Shift) */ shift_2s_comp,
                  /* DST */ temp);

               if (instance_data_with_base_instance) {
                  *buffer++ =
                     pvr_pds_inst_encode_add32(0, /* cc */
                                               0, /* ALNUM */
                                               0, /* SNA */
                                               base_instance_register, /* src0
                                                                        */
                                               temp, /* src1 */
                                               temp /* dst */
                     );
               }
            } else {
               if (instance_data_with_base_instance) {
                  *buffer++ =
                     pvr_pds_inst_encode_add32(0, /* cc */
                                               0, /* ALNUM */
                                               0, /* SNA */
                                               base_instance_register, /* src0
                                                                        */
                                               input_register1, /* src1 */
                                               temp /* dst */
                     );
               } else {
                  /* If the shift instruction doesn't happen, use the IR
                   * directly into the following MAD.
                   */
                  temp = vertex_stream->instance_data ? input_register1
                                                      : input_register0;
               }
            }
         }
      }

      if (PVR_HAS_FEATURE(dev_info, pds_ddmadt)) {
         if (vertex_stream->use_ddmadt)
            ddmadt_enables |= (1 << stream);
      } else {
         if ((ddmadt_enables & (1 << stream)) != 0) {
            /* Emulate what DDMADT does for range checking. */
            if (first_ddmadt) {
               /* Get an 64 bits temp such that cmp current index with
                * allowed vertex number can work.
                */
               index_temp64 =
                  pvr_pds_get_temps(&next_temp, 2, &temps_used); /* 64-bit
                                                                  */
               num_vertices_temp64 =
                  pvr_pds_get_temps(&next_temp, 2, &temps_used); /* 64-bit
                                                                  */

               index_temp64 -= PVR_ROGUE_PDSINST_REGS32_TEMP32_LOWER;
               num_vertices_temp64 -= PVR_ROGUE_PDSINST_REGS32_TEMP32_LOWER;

               code_size += 3;
               current_p0 = true;
            }

            code_size += (temp == pre_index_temp ? 1 : 2);

            if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
               if (first_ddmadt) {
                  /* Set predicate to be P0. */
                  *buffer++ = pvr_pds_encode_bra(
                     PVR_ROGUE_PDSINST_PREDICATE_KEEP, /* SRCCC
                                                        */
                     0, /* Neg */
                     PVR_ROGUE_PDSINST_PREDICATE_P0, /* SETCC
                                                      */
                     1); /* Addr */

                  *buffer++ =
                     pvr_pds_inst_encode_limm(0, index_temp64 + 1, 0, 0);
                  *buffer++ =
                     pvr_pds_inst_encode_limm(0, num_vertices_temp64 + 1, 0, 0);
               }

               if (temp != pre_index_temp) {
                  *buffer++ = pvr_pds_inst_encode_sftlp32(
                     /* IM */ 1, /*  enable immediate. */
                     /* cc */ 0,
                     /* LOP */ PVR_ROGUE_PDSINST_LOP_NONE,
                     /* SRC0 */ temp - PVR_ROGUE_PDSINST_REGS32_TEMP32_LOWER,
                     /* SRC1 */ 0,
                     /* SRC2 (Shift) */ 0,
                     /* DST */ index_temp64);
               }

               *buffer++ = pvr_pds_inst_encode_sftlp32(
                  /* IM */ 1, /*  enable immediate. */
                  /* cc */ 0,
                  /* LOP */ PVR_ROGUE_PDSINST_LOP_OR,
                  /* SRC0 */ num_vertices_temp64 + 1,
                  /* SRC1 */ vertex_stream->num_vertices,
                  /* SRC2 (Shift) */ 0,
                  /* DST */ num_vertices_temp64);
            }

            first_ddmadt = false;

            pre_index_temp = temp;
         }
      }

      /* Process the elements in the stream. */
      for (uint32_t element = 0; element < vertex_stream->num_elements;
           element++) {
         bool terminate = false;

         vertex_element = &vertex_stream->elements[element];
         /* Check if last DDMAD needs terminate or not. */
         if ((element == (vertex_stream->num_elements - 1)) &&
             (stream == last_stream_index)) {
            terminate = !issue_empty_ddmad && !direct_writes_needed;
         }

         /* Get a new set of constants for this element. */
         if (element) {
            /* Get all 8 32 bit constants at once. */
            next_constant =
               pvr_pds_get_constants(&next_stream_constant, 8, &data_size);
         }

         dma_address_constant64 = next_constant + 4;
         dma_control_constant64 = dma_address_constant64 + 2;

         if (vertex_element->component_size == 0) {
            /* Standard DMA.
             *
             * Write the DMA transfer control words into the PDS data
             * section.
             *
             * DMA Address is 40-bit.
             */

            if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
               uint32_t dma_control_word;
               uint64_t dma_control_word64 = 0;
               uint32_t dma_size;

               /* Write the address to the constant. */
               pvr_pds_write_dma_address(buffer,
                                         dma_address_constant64,
                                         vertex_stream->address +
                                            (uint64_t)vertex_element->offset,
                                         false,
                                         dev_info);
               {
                  if (program->stream_patch_offsets) {
                     program
                        ->stream_patch_offsets[program->num_stream_patches++] =
                        (stream << 16) | (dma_address_constant64 >> 1);
                  }
               }

               /* Size is in bytes - round up to nearest 32 bit word. */
               dma_size =
                  (vertex_element->size + (1 << PVR_PDS_DWORD_SHIFT) - 1) >>
                  PVR_PDS_DWORD_SHIFT;

               assert(dma_size <= PVR_ROGUE_PDSINST_DDMAD_FIELDS_BSIZE_UPPER);

               /* Set up the dma transfer control word. */
               dma_control_word =
                  dma_size << PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_BSIZE_SHIFT;

               dma_control_word |=
                  vertex_element->reg
                  << PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_AO_SHIFT;

               dma_control_word |=
                  PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_DEST_UNIFIED_STORE |
                  PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_CMODE_CACHED;

               if (PVR_HAS_FEATURE(dev_info, pds_ddmadt)) {
                  if ((ddmadt_enables & (1 << stream)) != 0) {
                     assert(
                        ((((uint64_t)vertex_stream->buffer_size_in_bytes
                           << PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_MSIZE_SHIFT) &
                          ~PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_MSIZE_CLRMSK) >>
                         PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_MSIZE_SHIFT) ==
                        (uint64_t)vertex_stream->buffer_size_in_bytes);
                     dma_control_word64 =
                        (PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_TEST_EN |
                         (((uint64_t)vertex_stream->buffer_size_in_bytes
                           << PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_MSIZE_SHIFT) &
                          ~PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_MSIZE_CLRMSK));
                  }
               }
               /* If this is the last dma then also set the last flag. */
               if (terminate) {
                  dma_control_word |=
                     PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_LAST_EN;
               }

               /* Write the 32-Bit SRC3 word to a 64-bit constant as per
                * spec.
                */
               pvr_pds_write_wide_constant(buffer,
                                           dma_control_constant64,
                                           dma_control_word64 |
                                              (uint64_t)dma_control_word);
            }

            if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
               if (!PVR_HAS_FEATURE(dev_info, pds_ddmadt)) {
                  if ((ddmadt_enables & (1 << stream)) != 0) {
                     *buffer++ = pvr_pds_inst_encode_cmp(
                        0, /* cc enable */
                        PVR_ROGUE_PDSINST_COP_LT, /* Operation */
                        index_temp64 >> 1, /* SRC0 (REGS64TP) */
                        (num_vertices_temp64 >> 1) +
                           PVR_ROGUE_PDSINST_REGS64_TEMP64_LOWER); /* SRC1
                                                                      (REGS64)
                                                                    */
                  }
               }
               /* Multiply by the vertex stream stride and add the base
                * followed by a DOUTD.
                *
                * dmad32 (C0 * T0) + C1, C2
                * src0 = stride  src1 = index  src2 = baseaddr src3 =
                * doutd part
                */

               uint32_t cc;
               if (PVR_HAS_FEATURE(dev_info, pds_ddmadt))
                  cc = 0;
               else
                  cc = (ddmadt_enables & (1 << stream)) != 0 ? 1 : 0;

               *buffer++ = pvr_pds_inst_encode_ddmad(
                  /* cc */ cc,
                  /* END */ 0,
                  /* SRC0 */ stride_constant32, /* Stride 32-bit*/
                  /* SRC1 */ temp, /* Index 32-bit*/
                  /* SRC2 64-bit */ dma_address_constant64 >> 1, /* Stream
                                                                  * Address
                                                                  * +
                                                                  * Offset
                                                                  */
                  /* SRC3 64-bit */ dma_control_constant64 >> 1 /* DMA
                                                                 * Transfer
                                                                 * Control
                                                                 * Word.
                                                                 */
               );
            }

            if ((!PVR_HAS_FEATURE(dev_info, pds_ddmadt)) &&
                ((ddmadt_enables & (1 << stream)) != 0)) {
               code_size += 1;
            }
            code_size += 1;
         } else {
            /* Repeat DMA.
             *
             * Write the DMA transfer control words into the PDS data
             * section.
             *
             * DMA address is 40-bit.
             */

            if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
               uint32_t dma_control_word;

               /* Write the address to the constant. */
               pvr_pds_write_dma_address(buffer,
                                         dma_address_constant64,
                                         vertex_stream->address +
                                            (uint64_t)vertex_element->offset,
                                         false,
                                         dev_info);

               /* Set up the DMA transfer control word. */
               dma_control_word =
                  vertex_element->size
                  << PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_BSIZE_SHIFT;

               dma_control_word |=
                  vertex_element->reg
                  << PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_AO_SHIFT;

               switch (vertex_element->component_size) {
               case 4: {
                  dma_control_word |=
                     PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_WORDSIZE_FOUR;
                  break;
               }
               case 3: {
                  dma_control_word |=
                     PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_WORDSIZE_THREE;
                  break;
               }
               case 2: {
                  dma_control_word |=
                     PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_WORDSIZE_TWO;
                  break;
               }
               default: {
                  dma_control_word |=
                     PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_WORDSIZE_ONE;
                  break;
               }
               }

               dma_control_word |=
                  PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_REPEAT_REPEAT;

               dma_control_word |=
                  PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_DEST_UNIFIED_STORE |
                  PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_CMODE_CACHED;

               /* If this is the last dma then also set the last flag. */
               if (terminate) {
                  dma_control_word |=
                     PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_LAST_EN;
               }

               /* Write the 32-Bit SRC3 word to a 64-bit constant as per
                * spec.
                */
               pvr_pds_write_wide_constant(buffer,
                                           dma_control_constant64,
                                           (uint64_t)dma_control_word);
            }

            if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
               /* Multiply by the vertex stream stride and add the base
                * followed by a DOUTD.
                *
                * dmad32 (C0 * T0) + C1, C2
                * src0 = stride  src1 = index  src2 = baseaddr src3 =
                * doutd part
                */
               *buffer++ = pvr_pds_inst_encode_ddmad(
                  /* cc */ 0,
                  /* END */ 0,
                  /* SRC0 */ stride_constant32, /* Stride 32-bit*/
                  /* SRC1 */ temp, /* Index 32-bit*/
                  /* SRC2 64-bit */ dma_address_constant64 >> 1, /* Stream
                                                                  * Address
                                                                  * +
                                                                  * Offset.
                                                                  */
                  /* SRC3 64-bit */ dma_control_constant64 >> 1 /* DMA
                                                                 * Transfer
                                                                 * Control
                                                                 * Word.
                                                                 */
               );
            }

            code_size += 1;
         } /* End of repeat DMA. */
      } /* Element loop */
   } /* Stream loop */

   if (issue_empty_ddmad) {
      /* Issue an empty last DDMAD, always executed. */
      if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
         pvr_pds_write_wide_constant(
            buffer,
            empty_dma_control_constant64,
            PVR_ROGUE_PDSINST_DDMAD_FIELDS_SRC3_LAST_EN);
      }

      code_size += 1;

      if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
         *buffer++ = pvr_pds_inst_encode_ddmad(
            /* cc */ 0,
            /* END */ 0,
            /* SRC0 */ stride_constant32, /* Stride 32-bit*/
            /* SRC1 */ temp, /* Index 32-bit*/
            /* SRC2 64-bit */ dma_address_constant64 >> 1, /* Stream
                                                            *Address +
                                                            *Offset.
                                                            */
            /* SRC3 64-bit */ empty_dma_control_constant64 >> 1 /* DMA
                                                                 * Transfer
                                                                 * Control
                                                                 * Word.
                                                                 */
         );
      }
   }

   if (!PVR_HAS_FEATURE(dev_info, pds_ddmadt)) {
      if (current_p0) {
         code_size += 1;

         if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
            /* Revert predicate back to IF0 which is required by DOUTU. */
            *buffer++ =
               pvr_pds_encode_bra(PVR_ROGUE_PDSINST_PREDICATE_KEEP, /* SRCCC
                                                                     */
                                  0, /* Neg */
                                  PVR_ROGUE_PDSINST_PREDICATE_IF0, /* SETCC
                                                                    */
                                  1); /* Addr */
         }
      }
   }
   /* Send VertexID if requested. */
   if (program->iterate_vtx_id) {
      if (program->draw_indirect) {
         if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
            *buffer++ = pvr_pds_inst_encode_add32(
               /* cc */ 0x0,
               /* ALUM */ 0, /* Unsigned */
               /* SNA */ 1, /* Minus */
               /* SRC0 32b */ input_register0, /* vertexID */
               /* SRC1 32b */ PVR_ROGUE_PDSINST_REGS32_PTEMP32_LOWER, /* base
                                                                       * vertexID.
                                                                       */
               /* DST 32b */ input_register0);
         }

         code_size += 1;
      }

      if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
         uint32_t doutw = pvr_pds_encode_doutw_src1(
            program->vtx_id_register,
            PVR_PDS_DOUTW_LOWER32,
            PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_UNIFIED_STORE,
            false,
            dev_info);

         if (!program->iterate_instance_id && !program->iterate_remap_id)
            doutw |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN;

         pvr_pds_write_constant32(buffer,
                                  vertex_id_control_word_const32,
                                  doutw);
      } else if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
         *buffer++ = pvr_pds_encode_doutw64(
            /* cc */ 0,
            /* END */ 0,
            /* SRC1 */ vertex_id_control_word_const32, /* DOUTW 32-bit Src1
                                                        */
            /* SRC0 */ input_register0 >> 1); /* DOUTW 64-bit Src0 */
      }

      code_size += 1;
   }

   /* Send InstanceID if requested. */
   if (program->iterate_instance_id) {
      if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
         uint32_t doutw = pvr_pds_encode_doutw_src1(
            program->instance_id_register,
            PVR_PDS_DOUTW_UPPER32,
            PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_UNIFIED_STORE,
            true,
            dev_info);

         if (!program->iterate_remap_id)
            doutw |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN;

         pvr_pds_write_constant32(buffer,
                                  instance_id_control_word_const32,
                                  doutw);
      } else if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
         *buffer++ = pvr_pds_encode_doutw64(
            /* cc */ 0,
            /* END */ 0,
            /* SRC1 */ instance_id_control_word_const32, /* DOUTW 32-bit Src1 */
            /* SRC0 */ input_register1 >> 1); /* DOUTW 64-bit Src0 */
      }

      code_size += 1;
   }

   /* Send remapped index number to vi0. */
   if (program->iterate_remap_id) {
      if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
         uint32_t doutw = pvr_pds_encode_doutw_src1(
            0 /* vi0 */,
            PVR_PDS_DOUTW_LOWER32,
            PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_UNIFIED_STORE |
               PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN,
            false,
            dev_info);

         pvr_pds_write_constant64(buffer,
                                  geometry_id_control_word_const64,
                                  doutw,
                                  0);
      } else if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
         *buffer++ = pvr_pds_encode_doutw64(
            /* cc */ 0,
            /* END */ 0,
            /* SRC1 */ geometry_id_control_word_const64, /* DOUTW 32-bit
                                                          * Src1
                                                          */
            /* SRC0 */ input_register2 >> 1); /* DOUTW 64-bit Src0 */
      }

      code_size += 1;
   }

   /* Copy the USC task control words to constants. */
   if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
      pvr_pds_write_wide_constant(buffer,
                                  usc_control_constant64,
                                  program->usc_task_control.src0); /* 64-bit
                                                                    * Src0
                                                                    */
      if (program->stream_patch_offsets) {
         /* USC TaskControl is always the first patch. */
         program->stream_patch_offsets[0] = usc_control_constant64 >> 1;
      }
   }

   if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
      /* Conditionally (if last in task) issue the task to the USC
       * (if0) DOUTU src1=USC Code Base address, src2=DOUTU word 2.
       */

      *buffer++ = pvr_pds_encode_doutu(
         /* cc */ 1,
         /* END */ 1,
         /* SRC0 */ usc_control_constant64 >> 1); /* DOUTU 64-bit Src0 */

      /* End the program if the Dout did not already end it. */
      *buffer++ = pvr_pds_inst_encode_halt(0);
   }

   code_size += 2;

   if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
      /* Set the data segment pointer and ensure we return 1 past the buffer
       * ptr.
       */
      program->data_segment = buffer;

      buffer += consts_size;
   }

   program->temps_used = temps_used;
   program->data_size = consts_size;
   program->code_size = code_size;
   program->ddmadt_enables = ddmadt_enables;
   if (!PVR_HAS_FEATURE(dev_info, pds_ddmadt))
      program->skip_stream_flag = skip_stream_flag;

   return buffer;
}

/**
 * Generates a PDS program to load USC compute shader global/local/workgroup
 * sizes/ids and then a DOUTU to execute the USC.
 *
 * \param program Pointer to description of the program that should be
 *                generated.
 * \param buffer Pointer to buffer that receives the output of this function.
 *               This will be either the data segment, or the code depending on
 *               gen_mode.
 * \param gen_mode Which part to generate, either data segment or code segment.
 *                 If PDS_GENERATE_SIZES is specified, nothing is written, but
 *                 size information in program is updated.
 * \param dev_info PVR device info struct.
 * \returns Pointer to just beyond the buffer for the data - i.e. the value of
 *          the buffer after writing its contents.
 */
uint32_t *
pvr_pds_compute_shader(struct pvr_pds_compute_shader_program *restrict program,
                       uint32_t *restrict buffer,
                       enum pvr_pds_generate_mode gen_mode,
                       const struct pvr_device_info *dev_info)
{
   uint32_t usc_control_constant64;
   uint32_t usc_control_constant64_coeff_update = 0;
   uint32_t zero_constant64 = 0;

   uint32_t data_size = 0;
   uint32_t code_size = 0;
   uint32_t temps_used = 0;
   uint32_t doutw = 0;

   uint32_t barrier_ctrl_word = 0;
   uint32_t barrier_ctrl_word2 = 0;

   /* Even though there are 3 IDs for local and global we only need max one
    * DOUTW for local, and two for global.
    */
   uint32_t work_group_id_ctrl_words[2] = { 0 };
   uint32_t local_id_ctrl_word = 0;
   uint32_t local_input_register;

   /* For the constant value to load into ptemp (SW fence). */
   uint64_t predicate_ld_src0_constant = 0;
   uint32_t cond_render_negate_constant = 0;

   uint32_t cond_render_pred_temp;
   uint32_t cond_render_negate_temp;

   /* 2x 64 bit registers that will mask out the Predicate load. */
   uint32_t cond_render_pred_mask_constant = 0;

#if defined(DEBUG)
   if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
      for (uint32_t j = 0; j < program->data_size; j++)
         buffer[j] = 0xDEADBEEF;
   }
#endif

   /* All the compute input registers are in temps. */
   temps_used += PVR_PDS_NUM_COMPUTE_INPUT_REGS;

   uint32_t next_temp = PVR_PDS_TEMPS_BLOCK_BASE + temps_used;

   uint32_t next_constant = PVR_PDS_CONSTANTS_BLOCK_BASE;

   if (program->kick_usc) {
      /* Copy the USC task control words to constants. */
      usc_control_constant64 =
         pvr_pds_get_constants(&next_constant, 2, &data_size);
   }

   if (program->has_coefficient_update_task) {
      usc_control_constant64_coeff_update =
         pvr_pds_get_constants(&next_constant, 2, &data_size);
   }

   if (program->conditional_render) {
      predicate_ld_src0_constant =
         pvr_pds_get_constants(&next_constant, 2, &data_size);
      cond_render_negate_constant =
         pvr_pds_get_constants(&next_constant, 2, &data_size);
      cond_render_pred_mask_constant =
         pvr_pds_get_constants(&next_constant, 4, &data_size);

      /* LD will load a 64 bit value. */
      cond_render_pred_temp = pvr_pds_get_temps(&next_temp, 4, &temps_used);
      cond_render_negate_temp = pvr_pds_get_temps(&next_temp, 2, &temps_used);

      program->cond_render_const_offset_in_dwords = predicate_ld_src0_constant;
      program->cond_render_pred_temp = cond_render_pred_temp;
   }

   if ((program->barrier_coefficient != PVR_PDS_COMPUTE_INPUT_REG_UNUSED) ||
       (program->clear_pds_barrier) ||
       (program->kick_usc && program->conditional_render)) {
      zero_constant64 = pvr_pds_get_constants(&next_constant, 2, &data_size);
   }

   if (program->barrier_coefficient != PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
      barrier_ctrl_word = pvr_pds_get_constants(&next_constant, 1, &data_size);
      if (PVR_HAS_QUIRK(dev_info, 51210)) {
         barrier_ctrl_word2 =
            pvr_pds_get_constants(&next_constant, 1, &data_size);
      }
   }

   if (program->work_group_input_regs[0] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED ||
       program->work_group_input_regs[1] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
      work_group_id_ctrl_words[0] =
         pvr_pds_get_constants(&next_constant, 1, &data_size);
   }

   if (program->work_group_input_regs[2] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
      work_group_id_ctrl_words[1] =
         pvr_pds_get_constants(&next_constant, 1, &data_size);
   }

   if ((program->local_input_regs[0] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED) ||
       (program->local_input_regs[1] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED) ||
       (program->local_input_regs[2] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED)) {
      local_id_ctrl_word = pvr_pds_get_constants(&next_constant, 1, &data_size);
   }

   if (program->add_base_workgroup) {
      for (uint32_t workgroup_component = 0; workgroup_component < 3;
           workgroup_component++) {
         if (program->work_group_input_regs[workgroup_component] !=
             PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
            program
               ->base_workgroup_constant_offset_in_dwords[workgroup_component] =
               pvr_pds_get_constants(&next_constant, 1, &data_size);
         }
      }
   }

   if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
      if (program->kick_usc) {
         /* Src0 for DOUTU */
         pvr_pds_write_wide_constant(buffer,
                                     usc_control_constant64,
                                     program->usc_task_control.src0); /* 64-bit
                                                                       * Src0.
                                                                       */
      }

      if (program->has_coefficient_update_task) {
         /* Src0 for DOUTU. */
         pvr_pds_write_wide_constant(
            buffer,
            usc_control_constant64_coeff_update,
            program->usc_task_control_coeff_update.src0); /* 64-bit Src0 */
      }

      if ((program->barrier_coefficient != PVR_PDS_COMPUTE_INPUT_REG_UNUSED) ||
          (program->clear_pds_barrier) ||
          (program->kick_usc && program->conditional_render)) {
         pvr_pds_write_wide_constant(buffer, zero_constant64, 0); /* 64-bit
                                                                   * Src0
                                                                   */
      }

      if (program->barrier_coefficient != PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
         if (PVR_HAS_QUIRK(dev_info, 51210)) {
            /* Write the constant for the coefficient register write. */
            doutw = pvr_pds_encode_doutw_src1(
               program->barrier_coefficient + 4,
               PVR_PDS_DOUTW_LOWER64,
               PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_COMMON_STORE,
               true,
               dev_info);
            pvr_pds_write_constant32(buffer, barrier_ctrl_word2, doutw);
         }
         /* Write the constant for the coefficient register write. */
         doutw = pvr_pds_encode_doutw_src1(
            program->barrier_coefficient,
            PVR_PDS_DOUTW_LOWER64,
            PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_COMMON_STORE,
            true,
            dev_info);

         /* Check whether the barrier is going to be the last DOUTW done by
          * the coefficient sync task.
          */
         if ((program->work_group_input_regs[0] ==
              PVR_PDS_COMPUTE_INPUT_REG_UNUSED) &&
             (program->work_group_input_regs[1] ==
              PVR_PDS_COMPUTE_INPUT_REG_UNUSED) &&
             (program->work_group_input_regs[2] ==
              PVR_PDS_COMPUTE_INPUT_REG_UNUSED)) {
            doutw |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN;
         }

         pvr_pds_write_constant32(buffer, barrier_ctrl_word, doutw);
      }

      /* If we want work-group id X, see if we also want work-group id Y. */
      if (program->work_group_input_regs[0] !=
             PVR_PDS_COMPUTE_INPUT_REG_UNUSED &&
          program->work_group_input_regs[1] !=
             PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
         /* Make sure we are going to DOUTW them into adjacent registers
          * otherwise we can't do it in one.
          */
         assert(program->work_group_input_regs[1] ==
                (program->work_group_input_regs[0] + 1));

         doutw = pvr_pds_encode_doutw_src1(
            program->work_group_input_regs[0],
            PVR_PDS_DOUTW_LOWER64,
            PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_COMMON_STORE,
            true,
            dev_info);

         /* If we don't want the Z work-group id then this is the last one.
          */
         if (program->work_group_input_regs[2] ==
             PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
            doutw |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN;
         }

         pvr_pds_write_constant32(buffer, work_group_id_ctrl_words[0], doutw);
      }
      /* If we only want one of X or Y then handle them separately. */
      else {
         if (program->work_group_input_regs[0] !=
             PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
            doutw = pvr_pds_encode_doutw_src1(
               program->work_group_input_regs[0],
               PVR_PDS_DOUTW_LOWER32,
               PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_COMMON_STORE,
               true,
               dev_info);

            /* If we don't want the Z work-group id then this is the last
             * one.
             */
            if (program->work_group_input_regs[2] ==
                PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
               doutw |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN;
            }

            pvr_pds_write_constant32(buffer,
                                     work_group_id_ctrl_words[0],
                                     doutw);
         } else if (program->work_group_input_regs[1] !=
                    PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
            doutw = pvr_pds_encode_doutw_src1(
               program->work_group_input_regs[1],
               PVR_PDS_DOUTW_UPPER32,
               PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_COMMON_STORE,
               true,
               dev_info);

            /* If we don't want the Z work-group id then this is the last
             * one.
             */
            if (program->work_group_input_regs[2] ==
                PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
               doutw |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN;
            }

            pvr_pds_write_constant32(buffer,
                                     work_group_id_ctrl_words[0],
                                     doutw);
         }
      }

      /* Handle work-group id Z. */
      if (program->work_group_input_regs[2] !=
          PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
         doutw = pvr_pds_encode_doutw_src1(
            program->work_group_input_regs[2],
            PVR_PDS_DOUTW_UPPER32,
            PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_COMMON_STORE |
               PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN,
            true,
            dev_info);

         pvr_pds_write_constant32(buffer, work_group_id_ctrl_words[1], doutw);
      }

      /* Handle the local IDs. */
      if ((program->local_input_regs[1] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED) ||
          (program->local_input_regs[2] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED)) {
         uint32_t dest_reg;

         /* If we want local id Y and Z make sure the compiler wants them in
          * the same register.
          */
         if (!program->flattened_work_groups) {
            if ((program->local_input_regs[1] !=
                 PVR_PDS_COMPUTE_INPUT_REG_UNUSED) &&
                (program->local_input_regs[2] !=
                 PVR_PDS_COMPUTE_INPUT_REG_UNUSED)) {
               assert(program->local_input_regs[1] ==
                      program->local_input_regs[2]);
            }
         }

         if (program->local_input_regs[1] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED)
            dest_reg = program->local_input_regs[1];
         else
            dest_reg = program->local_input_regs[2];

         /* If we want local id X and (Y or Z) then we can do that in a
          * single 64-bit DOUTW.
          */
         if (program->local_input_regs[0] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
            assert(dest_reg == (program->local_input_regs[0] + 1));

            doutw = pvr_pds_encode_doutw_src1(
               program->local_input_regs[0],
               PVR_PDS_DOUTW_LOWER64,
               PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_UNIFIED_STORE,
               true,
               dev_info);

            doutw |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN;

            pvr_pds_write_constant32(buffer, local_id_ctrl_word, doutw);
         }
         /* Otherwise just DMA in Y and Z together in a single 32-bit DOUTW.
          */
         else {
            doutw = pvr_pds_encode_doutw_src1(
               dest_reg,
               PVR_PDS_DOUTW_UPPER32,
               PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_UNIFIED_STORE,
               true,
               dev_info);

            doutw |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN;

            pvr_pds_write_constant32(buffer, local_id_ctrl_word, doutw);
         }
      }
      /* If we don't want Y or Z then just DMA in X in a single 32-bit DOUTW.
       */
      else if (program->local_input_regs[0] !=
               PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
         doutw = pvr_pds_encode_doutw_src1(
            program->local_input_regs[0],
            PVR_PDS_DOUTW_LOWER32,
            PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_UNIFIED_STORE |
               PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN,
            true,
            dev_info);

         pvr_pds_write_constant32(buffer, local_id_ctrl_word, doutw);
      }
   }

   if (gen_mode == PDS_GENERATE_CODE_SEGMENT ||
       gen_mode == PDS_GENERATE_SIZES) {
      const bool encode = (gen_mode == PDS_GENERATE_CODE_SEGMENT);
#define APPEND(X)                    \
   if (encode) {                     \
      *buffer = X;                   \
      buffer++;                      \
   } else {                          \
      code_size += sizeof(uint32_t); \
   }

      /* Assert that coeff_update_task_branch_size is > 0 because if it is 0
       * then we will be doing an infinite loop.
       */
      if (gen_mode == PDS_GENERATE_CODE_SEGMENT)
         assert(program->coeff_update_task_branch_size > 0);

      /* Test whether this is the coefficient update task or not. */
      APPEND(
         pvr_pds_encode_bra(PVR_ROGUE_PDSINST_PREDICATE_IF1, /* SRCC */
                            PVR_ROGUE_PDSINST_NEG_ENABLE, /* NEG */
                            PVR_ROGUE_PDSINST_PREDICATE_KEEP, /* SETC */
                            program->coeff_update_task_branch_size /* ADDR */));

      /* Do we need to initialize the barrier coefficient? */
      if (program->barrier_coefficient != PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
         if (PVR_HAS_QUIRK(dev_info, 51210)) {
            /* Initialize the second barrier coefficient registers to zero.
             */
            APPEND(pvr_pds_encode_doutw64(0, /* cc */
                                          0, /* END */
                                          barrier_ctrl_word2, /* SRC1 */
                                          zero_constant64 >> 1)); /* SRC0 */
         }
         /* Initialize the coefficient register to zero. */
         APPEND(pvr_pds_encode_doutw64(0, /* cc */
                                       0, /* END */
                                       barrier_ctrl_word, /* SRC1 */
                                       zero_constant64 >> 1)); /* SRC0 */
      }

      if (program->add_base_workgroup) {
         const uint32_t temp_values[3] = { 0, 1, 3 };
         for (uint32_t workgroup_component = 0; workgroup_component < 3;
              workgroup_component++) {
            if (program->work_group_input_regs[workgroup_component] ==
                PVR_PDS_COMPUTE_INPUT_REG_UNUSED)
               continue;

            APPEND(pvr_pds_inst_encode_add32(
               /* cc */ 0x0,
               /* ALUM */ 0,
               /* SNA */ 0,
               /* SRC0 (R32)*/ PVR_ROGUE_PDSINST_REGS32_CONST32_LOWER +
                  program->base_workgroup_constant_offset_in_dwords
                     [workgroup_component],
               /* SRC1 (R32)*/ PVR_ROGUE_PDSINST_REGS32_TEMP32_LOWER +
                  PVR_PDS_CDM_WORK_GROUP_ID_X +
                  temp_values[workgroup_component],
               /* DST  (R32TP)*/ PVR_ROGUE_PDSINST_REGS32TP_TEMP32_LOWER +
                  PVR_PDS_CDM_WORK_GROUP_ID_X +
                  temp_values[workgroup_component]));
         }
      }

      /* If we are going to put the work-group IDs in coefficients then we
       * just need to do the DOUTWs.
       */
      if ((program->work_group_input_regs[0] !=
           PVR_PDS_COMPUTE_INPUT_REG_UNUSED) ||
          (program->work_group_input_regs[1] !=
           PVR_PDS_COMPUTE_INPUT_REG_UNUSED)) {
         uint32_t dest_reg;

         if (program->work_group_input_regs[0] !=
             PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
            dest_reg = PVR_PDS_TEMPS_BLOCK_BASE + PVR_PDS_CDM_WORK_GROUP_ID_X;
         } else {
            dest_reg = PVR_PDS_TEMPS_BLOCK_BASE + PVR_PDS_CDM_WORK_GROUP_ID_Y;
         }

         APPEND(pvr_pds_encode_doutw64(0, /* cc */
                                       0, /* END */
                                       work_group_id_ctrl_words[0], /* SRC1
                                                                     */
                                       dest_reg >> 1)); /* SRC0 */
      }

      if (program->work_group_input_regs[2] !=
          PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
         APPEND(pvr_pds_encode_doutw64(
            0, /* cc */
            0, /* END */
            work_group_id_ctrl_words[1], /* SRC1 */
            (PVR_PDS_TEMPS_BLOCK_BASE + PVR_PDS_CDM_WORK_GROUP_ID_Z) >>
               1)); /* SRC0 */
      }

      /* Issue the task to the USC. */
      if (program->kick_usc && program->has_coefficient_update_task) {
         APPEND(pvr_pds_encode_doutu(0, /* cc */
                                     1, /* END */
                                     usc_control_constant64_coeff_update >>
                                        1)); /* SRC0; DOUTU 64-bit Src0 */
      }

      /* Encode a HALT */
      APPEND(pvr_pds_inst_encode_halt(0));

      /* Set the branch size used to skip the coefficient sync task. */
      program->coeff_update_task_branch_size = code_size / sizeof(uint32_t);

      /* DOUTW in the local IDs. */

      /* If we want X and Y or Z, we only need one DOUTW. */
      if ((program->local_input_regs[0] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED) &&
          ((program->local_input_regs[1] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED) ||
           (program->local_input_regs[2] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED))) {
         local_input_register =
            PVR_PDS_TEMPS_BLOCK_BASE + PVR_PDS_CDM_LOCAL_ID_X;
      } else {
         /* If we just want X. */
         if (program->local_input_regs[0] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
            local_input_register =
               PVR_PDS_TEMPS_BLOCK_BASE + PVR_PDS_CDM_LOCAL_ID_X;
         }
         /* If we just want Y or Z. */
         else if (program->local_input_regs[1] !=
                     PVR_PDS_COMPUTE_INPUT_REG_UNUSED ||
                  program->local_input_regs[2] !=
                     PVR_PDS_COMPUTE_INPUT_REG_UNUSED) {
            local_input_register =
               PVR_PDS_TEMPS_BLOCK_BASE + PVR_PDS_CDM_LOCAL_ID_YZ;
         }
      }

      if ((program->local_input_regs[0] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED) ||
          (program->local_input_regs[1] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED) ||
          (program->local_input_regs[2] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED)) {
         APPEND(pvr_pds_encode_doutw64(0, /* cc */
                                       0, /* END */
                                       local_id_ctrl_word, /* SRC1 */
                                       local_input_register >> 1)); /* SRC0
                                                                     */
      }

      if (program->clear_pds_barrier) {
         /* Zero the persistent temp (SW fence for context switch). */
         APPEND(pvr_pds_inst_encode_add64(
            0, /* cc */
            PVR_ROGUE_PDSINST_ALUM_UNSIGNED,
            PVR_ROGUE_PDSINST_MAD_SNA_ADD,
            PVR_ROGUE_PDSINST_REGS64_CONST64_LOWER +
               (zero_constant64 >> 1), /* src0 = 0 */
            PVR_ROGUE_PDSINST_REGS64_CONST64_LOWER +
               (zero_constant64 >> 1), /* src1 = 0 */
            PVR_ROGUE_PDSINST_REGS64TP_PTEMP64_LOWER + 0)); /* dest =
                                                             * ptemp64[0]
                                                             */
      }

      /* If this is a fence, issue the DOUTC. */
      if (program->fence) {
         APPEND(pvr_pds_inst_encode_doutc(0, /* cc */
                                          0 /* END */));
      }

      if (program->kick_usc) {
         if (program->conditional_render) {
            /* Skip if coefficient update task. */
            APPEND(pvr_pds_inst_encode_bra(PVR_ROGUE_PDSINST_PREDICATE_IF1,
                                           0,
                                           PVR_ROGUE_PDSINST_PREDICATE_KEEP,
                                           16));

            /* Load the predicate. */
            APPEND(pvr_pds_inst_encode_ld(0, predicate_ld_src0_constant >> 1));

            /* Load negate constant into temp for CMP. */
            APPEND(pvr_pds_inst_encode_add64(
               0, /* cc */
               PVR_ROGUE_PDSINST_ALUM_UNSIGNED,
               PVR_ROGUE_PDSINST_MAD_SNA_ADD,
               PVR_ROGUE_PDSINST_REGS64_CONST64_LOWER +
                  (cond_render_negate_constant >> 1), /* src0 = 0 */
               PVR_ROGUE_PDSINST_REGS64_CONST64_LOWER +
                  (zero_constant64 >> 1), /* src1 = 0 */
               PVR_ROGUE_PDSINST_REGS64TP_TEMP64_LOWER +
                  (cond_render_negate_temp >> 1))); /* dest = ptemp64[0]
                                                     */

            APPEND(pvr_pds_inst_encode_wdf(0));

            for (uint32_t i = 0; i < 4; i++) {
               APPEND(pvr_pds_inst_encode_sftlp32(
                  1, /* enable immediate */
                  0, /* cc */
                  PVR_ROGUE_PDSINST_LOP_AND, /* LOP */
                  cond_render_pred_temp + i, /* SRC0 */
                  cond_render_pred_mask_constant + i, /* SRC1 */
                  0, /* SRC2 (Shift) */
                  cond_render_pred_temp + i)); /* DST */

               APPEND(
                  pvr_pds_inst_encode_sftlp32(1, /* enable immediate */
                                              0, /* cc */
                                              PVR_ROGUE_PDSINST_LOP_OR, /* LOP
                                                                         */
                                              cond_render_pred_temp + i, /* SRC0
                                                                          */
                                              cond_render_pred_temp, /* SRC1 */
                                              0, /* SRC2 (Shift) */
                                              cond_render_pred_temp)); /* DST */
            }

            APPEND(pvr_pds_inst_encode_limm(0, /* cc */
                                            cond_render_pred_temp + 1, /* SRC1
                                                                        */
                                            0, /* SRC0 */
                                            0)); /* GLOBALREG */

            APPEND(pvr_pds_inst_encode_sftlp32(1, /* enable immediate */
                                               0, /* cc */
                                               PVR_ROGUE_PDSINST_LOP_XOR, /* LOP
                                                                           */
                                               cond_render_pred_temp, /* SRC0 */
                                               cond_render_negate_temp, /* SRC1
                                                                         */
                                               0, /* SRC2 (Shift) */
                                               cond_render_pred_temp)); /* DST
                                                                         */

            /* Check that the predicate is 0. */
            APPEND(pvr_pds_inst_encode_cmpi(
               0, /* cc */
               PVR_ROGUE_PDSINST_COP_EQ, /* LOP */
               (cond_render_pred_temp >> 1) +
                  PVR_ROGUE_PDSINST_REGS64TP_TEMP64_LOWER, /* SRC0 */
               0)); /* SRC1 */

            /* If predicate is 0, skip DOUTU. */
            APPEND(pvr_pds_inst_encode_bra(
               PVR_ROGUE_PDSINST_PREDICATE_P0, /* SRCC:
                                                  P0 */
               0, /* NEG */
               PVR_ROGUE_PDSINST_PREDICATE_KEEP, /* SETC:
                                                    keep
                                                  */
               2));
         }

         /* Issue the task to the USC.
          * DoutU src1=USC Code Base address, src2=doutu word 2.
          */
         APPEND(pvr_pds_encode_doutu(1, /* cc */
                                     1, /* END */
                                     usc_control_constant64 >> 1)); /* SRC0;
                                                                     * DOUTU
                                                                     * 64-bit
                                                                     * Src0.
                                                                     */
      }

      /* End the program if the Dout did not already end it. */
      APPEND(pvr_pds_inst_encode_halt(0));
#undef APPEND
   }

   if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
      /* Set the data segment pointer and ensure we return 1 past the buffer
       * ptr.
       */
      program->data_segment = buffer;

      buffer += next_constant;
   }

   /* Require at least one DWORD of PDS data so the program runs. */
   data_size = MAX2(1, data_size);

   program->temps_used = temps_used;
   program->highest_temp = temps_used;
   program->data_size = data_size;
   if (gen_mode == PDS_GENERATE_SIZES)
      program->code_size = code_size;

   return buffer;
}

/**
 * Generates the PDS vertex shader data or code block. This program will do a
 * DMA into USC Constants followed by a DOUTU.
 *
 * \param program Pointer to the PDS vertex shader program.
 * \param buffer Pointer to the buffer for the program.
 * \param gen_mode Generate code or data.
 * \param dev_info PVR device information struct.
 * \returns Pointer to just beyond the code/data.
 */
uint32_t *pvr_pds_vertex_shader_sa(
   struct pvr_pds_vertex_shader_sa_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info)
{
   uint32_t next_constant;
   uint32_t data_size = 0;
   uint32_t code_size = 0;

   uint32_t usc_control_constant64 = 0;
   uint32_t dma_address_constant64 = 0;
   uint32_t dma_control_constant32 = 0;
   uint32_t doutw_value_constant64 = 0;
   uint32_t doutw_control_constant32 = 0;
   uint32_t fence_constant_word = 0;
   uint32_t *buffer_base;
   uint32_t kick_index;

   uint32_t total_num_doutw =
      program->num_dword_doutw + program->num_q_word_doutw;
   uint32_t total_size_dma =
      program->num_dword_doutw + 2 * program->num_q_word_doutw;

   next_constant = PVR_PDS_CONSTANTS_BLOCK_BASE;

   /* Copy the DMA control words and USC task control words to constants.
    *
    * Arrange them so that the 64-bit words are together followed by the 32-bit
    * words.
    */
   if (program->kick_usc) {
      usc_control_constant64 =
         pvr_pds_get_constants(&next_constant, 2, &data_size);
   }

   if (program->clear_pds_barrier) {
      fence_constant_word =
         pvr_pds_get_constants(&next_constant, 2, &data_size);
   }
   dma_address_constant64 = pvr_pds_get_constants(&next_constant,
                                                  2 * program->num_dma_kicks,
                                                  &data_size);

   /* Assign all unaligned constants together to avoid alignment issues caused
    * by pvr_pds_get_constants with even allocation sizes.
    */
   doutw_value_constant64 = pvr_pds_get_constants(
      &next_constant,
      total_size_dma + total_num_doutw + program->num_dma_kicks,
      &data_size);
   doutw_control_constant32 = doutw_value_constant64 + total_size_dma;
   dma_control_constant32 = doutw_control_constant32 + total_num_doutw;

   if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
      buffer_base = buffer;

      if (program->kick_usc) {
         /* Src0 for DOUTU. */
         pvr_pds_write_wide_constant(buffer_base,
                                     usc_control_constant64,
                                     program->usc_task_control.src0); /* DOUTU
                                                                       * 64-bit
                                                                       * Src0.
                                                                       */
         buffer += 2;
      }

      if (program->clear_pds_barrier) {
         /* Encode the fence constant src0. Fence barrier is initialized to
          * zero.
          */
         pvr_pds_write_wide_constant(buffer_base, fence_constant_word, 0);
         buffer += 2;
      }

      if (total_num_doutw > 0) {
         for (uint32_t i = 0; i < program->num_q_word_doutw; i++) {
            /* Write the constant for the coefficient register write. */
            pvr_pds_write_constant64(buffer_base,
                                     doutw_value_constant64,
                                     program->q_word_doutw_value[2 * i],
                                     program->q_word_doutw_value[2 * i + 1]);
            pvr_pds_write_constant32(
               buffer_base,
               doutw_control_constant32,
               program->q_word_doutw_control[i] |
                  ((!program->num_dma_kicks && i == total_num_doutw - 1)
                      ? PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN
                      : 0));

            doutw_value_constant64 += 2;
            doutw_control_constant32 += 1;
         }

         for (uint32_t i = 0; i < program->num_dword_doutw; i++) {
            /* Write the constant for the coefficient register write. */
            pvr_pds_write_constant32(buffer_base,
                                     doutw_value_constant64,
                                     program->dword_doutw_value[i]);
            pvr_pds_write_constant32(
               buffer_base,
               doutw_control_constant32,
               program->dword_doutw_control[i] |
                  ((!program->num_dma_kicks && i == program->num_dword_doutw - 1)
                      ? PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN
                      : 0));

            doutw_value_constant64 += 1;
            doutw_control_constant32 += 1;
         }

         buffer += total_size_dma + total_num_doutw;
      }

      if (program->num_dma_kicks == 1) /* Most-common case. */
      {
         /* Src0 for DOUTD - Address. */
         pvr_pds_write_dma_address(buffer_base,
                                   dma_address_constant64,
                                   program->dma_address[0],
                                   false,
                                   dev_info);

         /* Src1 for DOUTD - Control Word. */
         pvr_pds_write_constant32(
            buffer_base,
            dma_control_constant32,
            program->dma_control[0] |
               PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTD_SRC1_LAST_EN);

         /* Move the buffer ptr along as we will return 1 past the buffer. */
         buffer += 3;
      } else if (program->num_dma_kicks > 1) {
         for (kick_index = 0; kick_index < program->num_dma_kicks - 1;
              kick_index++) {
            /* Src0 for DOUTD - Address. */
            pvr_pds_write_dma_address(buffer_base,
                                      dma_address_constant64,
                                      program->dma_address[kick_index],
                                      false,
                                      dev_info);

            /* Src1 for DOUTD - Control Word. */
            pvr_pds_write_constant32(buffer_base,
                                     dma_control_constant32,
                                     program->dma_control[kick_index]);
            dma_address_constant64 += 2;
            dma_control_constant32 += 1;
         }

         /* Src0 for DOUTD - Address. */
         pvr_pds_write_dma_address(buffer_base,
                                   dma_address_constant64,
                                   program->dma_address[kick_index],
                                   false,
                                   dev_info);

         /* Src1 for DOUTD - Control Word. */
         pvr_pds_write_constant32(
            buffer_base,
            dma_control_constant32,
            program->dma_control[kick_index] |
               PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTD_SRC1_LAST_EN);

         buffer += 3 * program->num_dma_kicks;
      }
   } else if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
      if (program->clear_pds_barrier) {
         /* Zero the persistent temp (SW fence for context switch). */
         *buffer++ = pvr_pds_inst_encode_add64(
            0, /* cc */
            PVR_ROGUE_PDSINST_ALUM_UNSIGNED,
            PVR_ROGUE_PDSINST_MAD_SNA_ADD,
            PVR_ROGUE_PDSINST_REGS64_CONST64_LOWER +
               (fence_constant_word >> 1), /* src0 = 0 */
            PVR_ROGUE_PDSINST_REGS64_CONST64_LOWER +
               (fence_constant_word >> 1), /* src1 = 0 */
            PVR_ROGUE_PDSINST_REGS64TP_PTEMP64_LOWER + 0); /* dest =
                                                            * ptemp[0]
                                                            */
      }

      if (total_num_doutw > 0) {
         for (uint32_t i = 0; i < program->num_q_word_doutw; i++) {
            /* Set the coefficient register to data value. */
            *buffer++ = pvr_pds_encode_doutw64(
               /* cc */ 0,
               /* END */ !program->num_dma_kicks && !program->kick_usc &&
                  (i == total_num_doutw - 1),
               /* SRC1 */ doutw_control_constant32,
               /* SRC0 */ doutw_value_constant64 >> 1);

            doutw_value_constant64 += 2;
            doutw_control_constant32 += 1;
         }

         for (uint32_t i = 0; i < program->num_dword_doutw; i++) {
            /* Set the coefficient register to data value. */
            *buffer++ = pvr_pds_encode_doutw64(
               /* cc */ 0,
               /* END */ !program->num_dma_kicks && !program->kick_usc &&
                  (i == program->num_dword_doutw - 1),
               /* SRC1 */ doutw_control_constant32,
               /* SRC0 */ doutw_value_constant64 >> 1);

            doutw_value_constant64 += 1;
            doutw_control_constant32 += 1;
         }
      }

      if (program->num_dma_kicks != 0) {
         /* DMA the state into the secondary attributes. */

         if (program->num_dma_kicks == 1) /* Most-common case. */
         {
            *buffer++ = pvr_pds_encode_doutd(
               /* cc */ 0,
               /* END */ !program->kick_usc,
               /* SRC1 */ dma_control_constant32, /* DOUTD 32-bit Src1 */
               /* SRC0 */ dma_address_constant64 >> 1); /* DOUTD 64-bit
                                                         * Src0.
                                                         */
         } else {
            for (kick_index = 0; kick_index < program->num_dma_kicks;
                 kick_index++) {
               *buffer++ = pvr_pds_encode_doutd(
                  /* cc */ 0,
                  /* END */ (!program->kick_usc) &&
                     (kick_index + 1 == program->num_dma_kicks),
                  /* SRC1 */ dma_control_constant32, /* DOUTD 32-bit
                                                      * Src1.
                                                      */
                  /* SRC0 */ dma_address_constant64 >> 1); /* DOUTD
                                                            * 64-bit
                                                            * Src0.
                                                            */
               dma_address_constant64 += 2;
               dma_control_constant32 += 1;
            }
         }
      }

      if (program->kick_usc) {
         /* Kick the USC. */
         *buffer++ = pvr_pds_encode_doutu(
            /* cc */ 0,
            /* END */ 1,
            /* SRC0 */ usc_control_constant64 >> 1); /* DOUTU 64-bit Src0.
                                                      */
      }

      if (!program->kick_usc && program->num_dma_kicks == 0 &&
          total_num_doutw == 0) {
         *buffer++ = pvr_pds_inst_encode_halt(0);
      }
   }

   code_size = program->num_dma_kicks + total_num_doutw;
   if (program->clear_pds_barrier)
      code_size++; /* ADD64 instruction. */

   if (program->kick_usc)
      code_size++;

   /* If there are no DMAs and no USC kick then code is HALT only. */
   if (code_size == 0)
      code_size = 1;

   program->data_size = data_size;
   program->code_size = code_size;

   return buffer;
}

/**
 * Writes the Uniform Data block for the PDS pixel shader secondary attributes
 * program.
 *
 * \param program Pointer to the PDS pixel shader secondary attributes program.
 * \param buffer Pointer to the buffer for the code/data.
 * \param gen_mode Either code or data can be generated or sizes only updated.
 * \returns Pointer to just beyond the buffer for the program/data.
 */
uint32_t *pvr_pds_pixel_shader_uniform_texture_code(
   struct pvr_pds_pixel_shader_sa_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode)
{
   uint32_t *instruction;
   uint32_t code_size = 0;
   uint32_t data_size = 0;
   uint32_t temps_used = 0;
   uint32_t next_constant;

   assert((((uintptr_t)buffer) & (PDS_ROGUE_TA_STATE_PDS_ADDR_ALIGNSIZE - 1)) ==
          0);

   assert((gen_mode == PDS_GENERATE_CODE_SEGMENT && buffer) ||
          gen_mode == PDS_GENERATE_SIZES);

   /* clang-format off */
   /* Shape of code segment (note: clear is different)
    *
    *      Code
    *    +------------+
    *    | BRA if0    |
    *    | DOUTD      |
    *    |  ...       |
    *    | DOUTD.halt |
    *    | uniform    |
    *    | DOUTD      |
    *    |  ...       |
    *    |  ...       |
    *    | DOUTW      |
    *    |  ...       |
    *    |  ...       |
    *    | DOUTU.halt |
    *    | HALT       |
    *    +------------+
    */
   /* clang-format on */
   instruction = buffer;

   next_constant = PVR_PDS_CONSTANTS_BLOCK_BASE;

   /* The clear color can arrive packed in the right form in the first (or
    * first 2) dwords of the shared registers and the program will issue a
    * single doutw for this.
    */
   if (program->clear && program->packed_clear) {
      uint32_t color_constant1 =
         pvr_pds_get_constants(&next_constant, 2, &data_size);

      uint32_t control_word_constant1 =
         pvr_pds_get_constants(&next_constant, 2, &data_size);

      if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
         /* DOUTW the clear color to the USC constants. Predicate with
          * uniform loading flag (IF0).
          */
         *instruction++ = pvr_pds_encode_doutw64(
            /* cc */ 1, /* Only for uniform loading program. */
            /* END */ program->kick_usc ? 0 : 1, /* Last
                                                  * instruction
                                                  * for a clear.
                                                  */
            /* SRC1 */ control_word_constant1, /* DOUTW 32-bit Src1 */
            /* SRC0 */ color_constant1 >> 1); /* DOUTW 64-bit Src0 */

         code_size += 1;
      }
   } else if (program->clear) {
      uint32_t color_constant1, color_constant2;

      if (program->clear_color_dest_reg & 0x1) {
         uint32_t color_constant3, control_word_constant1,
            control_word_constant2, color_constant4;

         color_constant1 = pvr_pds_get_constants(&next_constant, 1, &data_size);
         color_constant2 = pvr_pds_get_constants(&next_constant, 2, &data_size);
         color_constant3 = pvr_pds_get_constants(&next_constant, 1, &data_size);

         control_word_constant1 =
            pvr_pds_get_constants(&next_constant, 2, &data_size);
         control_word_constant2 =
            pvr_pds_get_constants(&next_constant, 2, &data_size);
         color_constant4 = pvr_pds_get_constants(&next_constant, 2, &data_size);

         if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
            /* DOUTW the clear color to the USSE constants. Predicate with
             * uniform loading flag (IF0).
             */
            *instruction++ = pvr_pds_encode_doutw64(
               /* cc */ 1, /* Only for Uniform Loading program */
               /* END */ 0,
               /* SRC1 */ control_word_constant1, /* DOUTW 32-bit Src1 */
               /* SRC0 */ color_constant1 >> 1); /* DOUTW 64-bit Src0 */

            *instruction++ = pvr_pds_encode_doutw64(
               /* cc */ 1, /* Only for Uniform Loading program */
               /* END */ 0,
               /* SRC1 */ control_word_constant2, /* DOUTW 32-bit Src1 */
               /* SRC0 */ color_constant2 >> 1); /* DOUTW 64-bit Src0 */

            *instruction++ = pvr_pds_encode_doutw64(
               /* cc */ 1, /* Only for uniform loading program */
               /* END */ program->kick_usc ? 0 : 1, /* Last
                                                     * instruction
                                                     * for a clear.
                                                     */
               /* SRC1 */ color_constant4, /* DOUTW 32-bit Src1 */
               /* SRC0 */ color_constant3 >> 1); /* DOUTW 64-bit Src0 */
         }

         code_size += 3;
      } else {
         uint32_t control_word_constant, control_word_last_constant;

         /* Put the clear color and control words into the first 8
          * constants.
          */
         color_constant1 = pvr_pds_get_constants(&next_constant, 2, &data_size);
         color_constant2 = pvr_pds_get_constants(&next_constant, 2, &data_size);
         control_word_constant =
            pvr_pds_get_constants(&next_constant, 2, &data_size);
         control_word_last_constant =
            pvr_pds_get_constants(&next_constant, 2, &data_size);

         if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
            /* DOUTW the clear color to the USSE constants. Predicate with
             * uniform loading flag (IF0).
             */
            *instruction++ = pvr_pds_encode_doutw64(
               /* cc */ 1, /* Only for Uniform Loading program */
               /* END */ 0,
               /* SRC1 */ control_word_constant, /* DOUTW 32-bit Src1 */
               /* SRC0 */ color_constant1 >> 1); /* DOUTW 64-bit Src0 */

            *instruction++ = pvr_pds_encode_doutw64(
               /* cc */ 1, /* Only for uniform loading program */
               /* END */ program->kick_usc ? 0 : 1, /* Last
                                                     * instruction
                                                     * for a clear.
                                                     */
               /* SRC1 */ control_word_last_constant, /* DOUTW 32-bit Src1
                                                       */
               /* SRC0 */ color_constant2 >> 1); /* DOUTW 64-bit Src0 */
         }

         code_size += 2;
      }

      if (program->kick_usc) {
         uint32_t doutu_constant64;

         doutu_constant64 =
            pvr_pds_get_constants(&next_constant, 2, &data_size);

         if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
            /* Issue the task to the USC.
             *
             * dout ds1[constant_use], ds0[constant_use],
             * ds1[constant_use], emit
             */
            *instruction++ = pvr_pds_encode_doutu(
               /* cc */ 0,
               /* END */ 1,
               /* SRC0 */ doutu_constant64 >> 1); /* DOUTU 64-bit Src0
                                                   */
         }

         code_size += 1;
      }

      if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
         /* End the program. */
         *instruction++ = pvr_pds_inst_encode_halt(0);
      }
      code_size += 1;
   } else {
      uint32_t total_num_doutw =
         program->num_dword_doutw + program->num_q_word_doutw;
      bool both_textures_and_uniforms =
         ((program->num_texture_dma_kicks > 0) &&
          ((program->num_uniform_dma_kicks > 0 || total_num_doutw > 0) ||
           program->kick_usc));
      uint32_t doutu_constant64 = 0;

      if (both_textures_and_uniforms) {
         /* If the size of a PDS data section is 0, the hardware won't run
          * it. We therefore don't need to branch when there is only a
          * texture OR a uniform update program.
          */
         if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
            uint32_t branch_address =
               MAX2(1 + program->num_texture_dma_kicks, 2);

            /* Use If0 to BRAnch to uniform code. */
            *instruction++ = pvr_pds_encode_bra(
               /* SRCC */ PVR_ROGUE_PDSINST_PREDICATE_IF0,
               /* NEG */ PVR_ROGUE_PDSINST_NEG_DISABLE,
               /* SETC */ PVR_ROGUE_PDSINST_PREDICATE_KEEP,
               /* ADDR */ branch_address);
         }

         code_size += 1;
      }

      if (program->num_texture_dma_kicks > 0) {
         uint32_t dma_address_constant64;
         uint32_t dma_control_constant32;
         /* Allocate 3 constant spaces for each kick. The 64-bit constants
          * come first followed by the 32-bit constants.
          */
         dma_address_constant64 = PVR_PDS_CONSTANTS_BLOCK_BASE;
         dma_control_constant32 =
            dma_address_constant64 + (program->num_texture_dma_kicks * 2);

         for (uint32_t dma = 0; dma < program->num_texture_dma_kicks; dma++) {
            code_size += 1;
            if (gen_mode != PDS_GENERATE_CODE_SEGMENT)
               continue;

            /* DMA the state into the secondary attributes. */
            *instruction++ = pvr_pds_encode_doutd(
               /* cc */ 0,
               /* END */ dma == (program->num_texture_dma_kicks - 1),
               /* SRC1 */ dma_control_constant32, /* DOUT 32-bit Src1 */
               /* SRC0 */ dma_address_constant64 >> 1); /* DOUT
                                                         * 64-bit
                                                         * Src0
                                                         */
            dma_address_constant64 += 2;
            dma_control_constant32 += 1;
         }
      } else if (both_textures_and_uniforms) {
         if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
            /* End the program. */
            *instruction++ = pvr_pds_inst_encode_halt(0);
         }

         code_size += 1;
      }

      /* Reserve space at the beginning of the data segment for the DOUTU Task
       * Control if one is needed.
       */
      if (program->kick_usc) {
         doutu_constant64 =
            pvr_pds_get_constants(&next_constant, 2, &data_size);
      }

      /* Allocate 3 constant spaces for each DMA and 2 for a USC kick. The
       * 64-bit constants come first followed by the 32-bit constants.
       */
      uint32_t total_size_dma =
         program->num_dword_doutw + 2 * program->num_q_word_doutw;

      uint32_t dma_address_constant64 = pvr_pds_get_constants(
         &next_constant,
         program->num_uniform_dma_kicks * 3 + total_size_dma + total_num_doutw,
         &data_size);
      uint32_t doutw_value_constant64 =
         dma_address_constant64 + program->num_uniform_dma_kicks * 2;
      uint32_t dma_control_constant32 = doutw_value_constant64 + total_size_dma;
      uint32_t doutw_control_constant32 =
         dma_control_constant32 + program->num_uniform_dma_kicks;

      if (total_num_doutw > 0) {
         pvr_pds_get_constants(&next_constant, 0, &data_size);

         if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
            for (uint32_t i = 0; i < program->num_q_word_doutw; i++) {
               /* Set the coefficient register to data value. */
               *instruction++ = pvr_pds_encode_doutw64(
                  /* cc */ 0,
                  /* END */ !program->num_uniform_dma_kicks &&
                     !program->kick_usc && (i == total_num_doutw - 1),
                  /* SRC1 */ doutw_control_constant32,
                  /* SRC0 */ doutw_value_constant64 >> 1);

               doutw_value_constant64 += 2;
               doutw_control_constant32 += 1;
            }

            for (uint32_t i = 0; i < program->num_dword_doutw; i++) {
               /* Set the coefficient register to data value. */
               *instruction++ = pvr_pds_encode_doutw64(
                  /* cc */ 0,
                  /* END */ !program->num_uniform_dma_kicks &&
                     !program->kick_usc && (i == program->num_dword_doutw - 1),
                  /* SRC1 */ doutw_control_constant32,
                  /* SRC0 */ doutw_value_constant64 >> 1);

               doutw_value_constant64 += 1;
               doutw_control_constant32 += 1;
            }
         }
         code_size += total_num_doutw;
      }

      if (program->num_uniform_dma_kicks > 0) {
         for (uint32_t dma = 0; dma < program->num_uniform_dma_kicks; dma++) {
            code_size += 1;

            if (gen_mode != PDS_GENERATE_CODE_SEGMENT)
               continue;

            bool last_instruction = false;
            if (!program->kick_usc &&
                (dma == program->num_uniform_dma_kicks - 1)) {
               last_instruction = true;
            }
            /* DMA the state into the secondary attributes. */
            *instruction++ = pvr_pds_encode_doutd(
               /* cc */ 0,
               /* END */ last_instruction,
               /* SRC1 */ dma_control_constant32, /* DOUT 32-bit Src1
                                                   */
               /* SRC0 */ dma_address_constant64 >> 1); /* DOUT
                                                         * 64-bit
                                                         * Src0
                                                         */
            dma_address_constant64 += 2;
            dma_control_constant32 += 1;
         }
      }

      if (program->kick_usc) {
         if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
            /* Issue the task to the USC.
             *
             * dout ds1[constant_use], ds0[constant_use],
             * ds1[constant_use], emit
             */

            *instruction++ = pvr_pds_encode_doutu(
               /* cc */ 0,
               /* END */ 1,
               /* SRC0 */ doutu_constant64 >> 1); /* DOUTU 64-bit Src0 */
         }

         code_size += 1;
      } else if (program->num_uniform_dma_kicks == 0 && total_num_doutw == 0) {
         if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
            /* End the program. */
            *instruction++ = pvr_pds_inst_encode_halt(0);
         }

         code_size += 1;
      }
   }

   /* Minimum temp count is 1. */
   program->temps_used = MAX2(temps_used, 1);
   program->code_size = code_size;

   if (gen_mode == PDS_GENERATE_CODE_SEGMENT)
      return instruction;
   else
      return NULL;
}

/**
 * Writes the Uniform Data block for the PDS pixel shader secondary attributes
 * program.
 *
 * \param program Pointer to the PDS pixel shader secondary attributes program.
 * \param buffer Pointer to the buffer for the code/data.
 * \param gen_mode Either code or data can be generated or sizes only updated.
 * \param dev_info PVR device information struct.
 * \returns Pointer to just beyond the buffer for the program/data.
 */
uint32_t *pvr_pds_pixel_shader_uniform_texture_data(
   struct pvr_pds_pixel_shader_sa_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode,
   bool uniform,
   const struct pvr_device_info *dev_info)
{
   uint32_t *constants = buffer;
   uint32_t next_constant = PVR_PDS_CONSTANTS_BLOCK_BASE;
   uint32_t temps_used = 0;
   uint32_t data_size = 0;

   assert((((uintptr_t)buffer) & (PDS_ROGUE_TA_STATE_PDS_ADDR_ALIGNSIZE - 1)) ==
          0);

   assert(gen_mode != PDS_GENERATE_CODE_SEGMENT);

   /* Shape of data segment (note: clear is different).
    *
    *        Uniform            Texture
    *    +--------------+   +-------------+
    *    | USC Task   L |   | USC Task  L |
    *    |            H |   |           H |
    *    | DMA1 Src0  L |   | DMA1 Src0 L |
    *    |            H |   |           H |
    *    | DMA2 Src0  L |   |             |
    *    |            H |   |             |
    *    | DMA1 Src1    |   | DMA1 Src1   |
    *    | DMA2 Src1    |   |             |
    *    | DOUTW0 Src1  |   |             |
    *    | DOUTW1 Src1  |   |             |
    *    |   ...        |   |             |
    *    | DOUTWn Srcn  |   |             |
    *    | other data   |   |             |
    *    +--------------+   +-------------+
    */

   /* Generate the PDS pixel shader secondary attributes data.
    *
    * Packed Clear
    * The clear color can arrive packed in the right form in the first (or
    * first 2) dwords of the shared registers and the program will issue a
    * single DOUTW for this.
    */
   if (program->clear && uniform && program->packed_clear) {
      uint32_t color_constant1 =
         pvr_pds_get_constants(&next_constant, 2, &data_size);

      uint32_t control_word_constant1 =
         pvr_pds_get_constants(&next_constant, 2, &data_size);

      if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
         uint32_t doutw;

         pvr_pds_write_constant64(constants,
                                  color_constant1,
                                  program->clear_color[0],
                                  program->clear_color[1]);

         /* Load into first constant in common store. */
         doutw = pvr_pds_encode_doutw_src1(
            program->clear_color_dest_reg,
            PVR_PDS_DOUTW_LOWER64,
            PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_COMMON_STORE,
            false,
            dev_info);

         /* Set the last flag. */
         doutw |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN;
         pvr_pds_write_constant64(constants, control_word_constant1, doutw, 0);
      }
   } else if (program->clear && uniform) {
      uint32_t color_constant1, color_constant2;

      if (program->clear_color_dest_reg & 0x1) {
         uint32_t color_constant3, control_word_constant1,
            control_word_constant2, color_constant4;

         color_constant1 = pvr_pds_get_constants(&next_constant, 1, &data_size);
         color_constant2 = pvr_pds_get_constants(&next_constant, 2, &data_size);
         color_constant3 = pvr_pds_get_constants(&next_constant, 1, &data_size);

         control_word_constant1 =
            pvr_pds_get_constants(&next_constant, 2, &data_size);
         control_word_constant2 =
            pvr_pds_get_constants(&next_constant, 2, &data_size);
         color_constant4 = pvr_pds_get_constants(&next_constant, 2, &data_size);

         if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
            uint32_t doutw;

            pvr_pds_write_constant32(constants,
                                     color_constant1,
                                     program->clear_color[0]);

            pvr_pds_write_constant64(constants,
                                     color_constant2,
                                     program->clear_color[1],
                                     program->clear_color[2]);

            pvr_pds_write_constant32(constants,
                                     color_constant3,
                                     program->clear_color[3]);

            /* Load into first constant in common store. */
            doutw = pvr_pds_encode_doutw_src1(
               program->clear_color_dest_reg,
               PVR_PDS_DOUTW_LOWER32,
               PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_COMMON_STORE,
               false,
               dev_info);

            pvr_pds_write_constant64(constants,
                                     control_word_constant1,
                                     doutw,
                                     0);

            /* Move the destination register along. */
            doutw = pvr_pds_encode_doutw_src1(
               program->clear_color_dest_reg + 1,
               PVR_PDS_DOUTW_LOWER64,
               PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_COMMON_STORE,
               false,
               dev_info);

            pvr_pds_write_constant64(constants,
                                     control_word_constant2,
                                     doutw,
                                     0);

            /* Move the destination register along. */
            doutw = pvr_pds_encode_doutw_src1(
               program->clear_color_dest_reg + 3,
               PVR_PDS_DOUTW_LOWER32,
               PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_COMMON_STORE,
               false,
               dev_info);

            /* Set the last flag. */
            doutw |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN;
            pvr_pds_write_constant64(constants, color_constant4, doutw, 0);
         }
      } else {
         uint32_t control_word_constant, control_word_last_constant;

         /* Put the clear color and control words into the first 8
          * constants.
          */
         color_constant1 = pvr_pds_get_constants(&next_constant, 2, &data_size);
         color_constant2 = pvr_pds_get_constants(&next_constant, 2, &data_size);
         control_word_constant =
            pvr_pds_get_constants(&next_constant, 2, &data_size);
         control_word_last_constant =
            pvr_pds_get_constants(&next_constant, 2, &data_size);

         if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
            uint32_t doutw;
            pvr_pds_write_constant64(constants,
                                     color_constant1,
                                     program->clear_color[0],
                                     program->clear_color[1]);

            pvr_pds_write_constant64(constants,
                                     color_constant2,
                                     program->clear_color[2],
                                     program->clear_color[3]);

            /* Load into first constant in common store. */
            doutw = pvr_pds_encode_doutw_src1(
               program->clear_color_dest_reg,
               PVR_PDS_DOUTW_LOWER64,
               PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_COMMON_STORE,
               false,
               dev_info);

            pvr_pds_write_constant64(constants, control_word_constant, doutw, 0);

            /* Move the destination register along. */
            doutw &= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_AO_CLRMSK;
            doutw |= (program->clear_color_dest_reg + 2)
                     << PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_AO_SHIFT;

            /* Set the last flag. */
            doutw |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN;
            pvr_pds_write_constant64(constants,
                                     control_word_last_constant,
                                     doutw,
                                     0);
         }
      }

      /* Constants for the DOUTU Task Control, if needed. */
      if (program->kick_usc) {
         uint32_t doutu_constant64 =
            pvr_pds_get_constants(&next_constant, 2, &data_size);

         if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
            pvr_pds_write_wide_constant(
               constants,
               doutu_constant64,
               program->usc_task_control.src0); /* 64-bit
                                                 */
            /* Src0 */
         }
      }
   } else {
      if (uniform) {
         /* Reserve space at the beginning of the data segment for the DOUTU
          * Task Control if one is needed.
          */
         if (program->kick_usc) {
            uint32_t doutu_constant64 =
               pvr_pds_get_constants(&next_constant, 2, &data_size);

            if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
               pvr_pds_write_wide_constant(
                  constants,
                  doutu_constant64,
                  program->usc_task_control.src0); /* 64-bit Src0 */
            }
         }

         uint32_t total_num_doutw =
            program->num_dword_doutw + program->num_q_word_doutw;
         uint32_t total_size_dma =
            program->num_dword_doutw + 2 * program->num_q_word_doutw;

         /* Allocate 3 constant spaces for each kick. The 64-bit constants
          * come first followed by the 32-bit constants.
          */
         uint32_t dma_address_constant64 =
            pvr_pds_get_constants(&next_constant,
                                  program->num_uniform_dma_kicks * 3 +
                                     total_size_dma + total_num_doutw,
                                  &data_size);
         uint32_t doutw_value_constant64 =
            dma_address_constant64 + program->num_uniform_dma_kicks * 2;
         uint32_t dma_control_constant32 =
            doutw_value_constant64 + total_size_dma;
         uint32_t doutw_control_constant32 =
            dma_control_constant32 + program->num_uniform_dma_kicks;

         if (total_num_doutw > 0) {
            if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
               for (uint32_t i = 0; i < program->num_q_word_doutw; i++) {
                  pvr_pds_write_constant64(
                     constants,
                     doutw_value_constant64,
                     program->q_word_doutw_value[2 * i],
                     program->q_word_doutw_value[2 * i + 1]);
                  pvr_pds_write_constant32(
                     constants,
                     doutw_control_constant32,
                     program->q_word_doutw_control[i] |
                        ((!program->num_uniform_dma_kicks &&
                          i == total_num_doutw - 1)
                            ? PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN
                            : 0));

                  doutw_value_constant64 += 2;
                  doutw_control_constant32 += 1;
               }

               for (uint32_t i = 0; i < program->num_dword_doutw; i++) {
                  pvr_pds_write_constant32(constants,
                                           doutw_value_constant64,
                                           program->dword_doutw_value[i]);
                  pvr_pds_write_constant32(
                     constants,
                     doutw_control_constant32,
                     program->dword_doutw_control[i] |
                        ((!program->num_uniform_dma_kicks &&
                          i == program->num_dword_doutw - 1)
                            ? PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN
                            : 0));

                  doutw_value_constant64 += 1;
                  doutw_control_constant32 += 1;
               }
            }
         }

         if (program->num_uniform_dma_kicks > 0) {
            uint32_t kick;

            if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
               for (kick = 0; kick < program->num_uniform_dma_kicks - 1;
                    kick++) {
                  /* Copy the dma control words to constants. */
                  pvr_pds_write_dma_address(constants,
                                            dma_address_constant64,
                                            program->uniform_dma_address[kick],
                                            false,
                                            dev_info);
                  pvr_pds_write_constant32(constants,
                                           dma_control_constant32,
                                           program->uniform_dma_control[kick]);

                  dma_address_constant64 += 2;
                  dma_control_constant32 += 1;
               }

               pvr_pds_write_dma_address(constants,
                                         dma_address_constant64,
                                         program->uniform_dma_address[kick],
                                         false,
                                         dev_info);
               pvr_pds_write_constant32(
                  constants,
                  dma_control_constant32,
                  program->uniform_dma_control[kick] |
                     PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTD_SRC1_LAST_EN);
            }
         }

      } else if (program->num_texture_dma_kicks > 0) {
         /* Allocate 3 constant spaces for each kick. The 64-bit constants
          * come first followed by the 32-bit constants.
          */
         uint32_t dma_address_constant64 =
            pvr_pds_get_constants(&next_constant,
                                  program->num_texture_dma_kicks * 3,
                                  &data_size);
         uint32_t dma_control_constant32 =
            dma_address_constant64 + (program->num_texture_dma_kicks * 2);

         if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
            uint32_t kick;
            for (kick = 0; kick < program->num_texture_dma_kicks - 1; kick++) {
               /* Copy the DMA control words to constants. */
               pvr_pds_write_dma_address(constants,
                                         dma_address_constant64,
                                         program->texture_dma_address[kick],
                                         false,
                                         dev_info);

               pvr_pds_write_constant32(constants,
                                        dma_control_constant32,
                                        program->texture_dma_control[kick]);

               dma_address_constant64 += 2;
               dma_control_constant32 += 1;
            }

            pvr_pds_write_dma_address(constants,
                                      dma_address_constant64,
                                      program->texture_dma_address[kick],
                                      false,
                                      dev_info);

            pvr_pds_write_constant32(
               constants,
               dma_control_constant32,
               program->texture_dma_control[kick] |
                  PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTD_SRC1_LAST_EN);
         }
      }
   }

   /* Save the data segment pointer and size. */
   program->data_segment = constants;

   /* Minimum temp count is 1. */
   program->temps_used = MAX2(temps_used, 1);
   program->data_size = data_size;

   if (gen_mode == PDS_GENERATE_DATA_SEGMENT)
      return (constants + next_constant);
   else
      return NULL;
}

/**
 * Generates generic DOUTC PDS program.
 *
 * \param program Pointer to the PDS kick USC.
 * \param buffer Pointer to the buffer for the program.
 * \param gen_mode Either code and data can be generated, or sizes only updated.
 * \returns Pointer to just beyond the buffer for the code or program segment.
 */
uint32_t *pvr_pds_generate_doutc(struct pvr_pds_fence_program *restrict program,
                                 uint32_t *restrict buffer,
                                 enum pvr_pds_generate_mode gen_mode)
{
   uint32_t constant = 0;

   /* Automatically get a data size of 1x 128bit chunks. */
   uint32_t data_size = 0, code_size = 0;

   /* Setup the data part. */
   uint32_t *constants = buffer; /* Constants placed at front of buffer. */
   uint32_t *instruction = buffer;
   uint32_t next_constant = PVR_PDS_CONSTANTS_BLOCK_BASE; /* Constants count in
                                                           * dwords.
                                                           */

   /* Update the program sizes. */
   program->data_size = data_size;
   program->code_size = code_size;
   program->data_segment = constants;

   if (gen_mode == PDS_GENERATE_SIZES)
      return NULL;

   if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
      /* Copy the USC task control words to constants. */

      constant = pvr_pds_get_constants(&next_constant, 2, &data_size);
      pvr_pds_write_wide_constant(constants, constant + 0, 0); /* 64-bit
                                                                * Src0
                                                                */

      uint32_t control_word_constant =
         pvr_pds_get_constants(&next_constant, 2, &data_size);
      pvr_pds_write_constant64(constants, control_word_constant, 0, 0); /* 32-bit
                                                                         * Src1
                                                                         */

      program->data_size = data_size;
      buffer += data_size;

      return buffer;
   } else if (gen_mode == PDS_GENERATE_CODE_SEGMENT && instruction) {
      *instruction++ = pvr_pds_inst_encode_doutc(
         /* cc */ 0,
         /* END */ 0);

      code_size++;

      /* End the program. */
      *instruction++ = pvr_pds_inst_encode_halt(0);
      code_size++;

      program->code_size = code_size;
   }

   return instruction;
}

/**
 * Generates generic kick DOUTU PDS program in a single data+code block.
 *
 * \param control Pointer to the PDS kick USC.
 * \param buffer Pointer to the buffer for the program.
 * \param gen_mode Either code and data can be generated or sizes only updated.
 * \param dev_info PVR device information structure.
 * \returns Pointer to just beyond the buffer for the code or program segment.
 */
uint32_t *pvr_pds_generate_doutw(struct pvr_pds_doutw_control *restrict control,
                                 uint32_t *restrict buffer,
                                 enum pvr_pds_generate_mode gen_mode,
                                 const struct pvr_device_info *dev_info)
{
   uint32_t next_constant = PVR_PDS_CONSTANTS_BLOCK_BASE;
   uint32_t doutw;
   uint32_t data_size = 0, code_size = 0;
   uint32_t constant[PVR_PDS_MAX_NUM_DOUTW_CONSTANTS];
   uint32_t control_word_constant[PVR_PDS_MAX_NUM_DOUTW_CONSTANTS];

   /* Assert if buffer is exceeded. */
   assert(control->num_const64 <= PVR_PDS_MAX_NUM_DOUTW_CONSTANTS);

   uint32_t *constants = buffer;
   uint32_t *instruction = buffer;

   /* Put the constants and control words interleaved in the data region. */
   for (uint32_t const_pair = 0; const_pair < control->num_const64;
        const_pair++) {
      constant[const_pair] =
         pvr_pds_get_constants(&next_constant, 2, &data_size);
      control_word_constant[const_pair] =
         pvr_pds_get_constants(&next_constant, 2, &data_size);
   }

   if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
      /* Data segment points to start of constants. */
      control->data_segment = constants;

      for (uint32_t const_pair = 0; const_pair < control->num_const64;
           const_pair++) {
         pvr_pds_write_constant64(constants,
                                  constant[const_pair],
                                  H32(control->doutw_data[const_pair]),
                                  L32(control->doutw_data[const_pair]));

         /* Start loading at offset 0. */
         if (control->dest_store == PDS_COMMON_STORE) {
            doutw = pvr_pds_encode_doutw_src1(
               (2 * const_pair),
               PVR_PDS_DOUTW_LOWER64,
               PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_COMMON_STORE,
               false,
               dev_info);
         } else {
            doutw = pvr_pds_encode_doutw_src1(
               (2 * const_pair),
               PVR_PDS_DOUTW_LOWER64,
               PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_DEST_UNIFIED_STORE,
               false,
               dev_info);
         }

         if (const_pair + 1 == control->num_const64) {
            /* Set the last flag for the MCU (assume there are no following
             * DOUTD's).
             */
            doutw |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTW_SRC1_LAST_EN;
         }
         pvr_pds_write_constant64(constants,
                                  control_word_constant[const_pair],
                                  doutw,
                                  0);
      }

      control->data_size = data_size;
   } else if (gen_mode == PDS_GENERATE_CODE_SEGMENT && instruction) {
      /* Code section. */

      for (uint32_t const_pair = 0; const_pair < control->num_const64;
           const_pair++) {
         /* DOUTW the PDS data to the USC constants. */
         *instruction++ = pvr_pds_encode_doutw64(
            /* cc */ 0,
            /* END */ control->last_instruction &&
               (const_pair + 1 == control->num_const64),
            /* SRC1 */ control_word_constant[const_pair], /* DOUTW 32-bit
                                                           * Src1.
                                                           */
            /* SRC0 */ constant[const_pair] >> 1); /* DOUTW 64-bit Src0. */

         code_size++;
      }

      if (control->last_instruction) {
         /* End the program. */
         *instruction++ = pvr_pds_inst_encode_halt(0);
         code_size++;
      }

      control->code_size = code_size;
   }

   if (gen_mode == PDS_GENERATE_DATA_SEGMENT)
      return (constants + next_constant);
   else
      return instruction;
}

/**
 * Generates generic kick DOUTU PDS program in a single data+code block.
 *
 * \param program Pointer to the PDS kick USC.
 * \param buffer Pointer to the buffer for the program.
 * \param start_next_constant Next constant in data segment. Non-zero if another
 *                            instruction precedes the DOUTU.
 * \param cc_enabled If true then the DOUTU is predicated (cc set).
 * \param gen_mode Either code and data can be generated or sizes only updated.
 * \returns Pointer to just beyond the buffer for the code or program segment.
 */
uint32_t *pvr_pds_kick_usc(struct pvr_pds_kickusc_program *restrict program,
                           uint32_t *restrict buffer,
                           uint32_t start_next_constant,
                           bool cc_enabled,
                           enum pvr_pds_generate_mode gen_mode)
{
   uint32_t constant = 0;

   /* Automatically get a data size of 2 128bit chunks. */
   uint32_t data_size = ROGUE_PDS_FIXED_PIXEL_SHADER_DATA_SIZE;
   uint32_t code_size = 1; /* Single doutu */
   uint32_t dummy_count = 0;

   /* Setup the data part. */
   uint32_t *constants = buffer; /* Constants placed at front of buffer. */
   uint32_t next_constant = PVR_PDS_CONSTANTS_BLOCK_BASE; /* Constants count in
                                                           * dwords.
                                                           */

   /* Update the program sizes. */
   program->data_size = data_size;
   program->code_size = code_size;
   program->data_segment = constants;

   if (gen_mode == PDS_GENERATE_SIZES)
      return NULL;

   if (gen_mode == PDS_GENERATE_DATA_SEGMENT ||
       gen_mode == PDS_GENERATE_CODEDATA_SEGMENTS) {
      /* Copy the USC task control words to constants. */

      constant = pvr_pds_get_constants(&next_constant, 2, &dummy_count);

      pvr_pds_write_wide_constant(constants,
                                  constant + 0,
                                  program->usc_task_control.src0); /* 64-bit
                                                                    * Src0.
                                                                    */
      buffer += data_size;

      if (gen_mode == PDS_GENERATE_DATA_SEGMENT)
         return buffer;
   }

   if (gen_mode == PDS_GENERATE_CODE_SEGMENT ||
       gen_mode == PDS_GENERATE_CODEDATA_SEGMENTS) {
      /* Generate the PDS pixel shader code. */

      /* Setup the instruction pointer. */
      uint32_t *instruction = buffer;

      /* Issue the task to the USC.
       *
       * dout ds1[constant_use], ds0[constant_use], ds1[constant_use], emit ;
       * halt halt
       */

      *instruction++ = pvr_pds_encode_doutu(
         /* cc */ cc_enabled,
         /* END */ 1,
         /* SRC0 */ (constant + start_next_constant) >> 1); /* DOUTU
                                                             * 64-bit Src0
                                                             */

      /* Return pointer to just after last instruction. */
      return instruction;
   }

   /* Execution should never reach here; keep compiler happy. */
   return NULL;
}

uint32_t *pvr_pds_generate_compute_barrier_conditional(
   uint32_t *buffer,
   enum pvr_pds_generate_mode gen_mode)
{
   /* Compute barriers supported. Need to test for coeff sync task. */

   if (gen_mode == PDS_GENERATE_DATA_SEGMENT)
      return buffer; /* No data segment. */

   if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
      /* Test whether this is the coefficient update task or not. */
      *buffer++ = pvr_pds_encode_bra(PVR_ROGUE_PDSINST_PREDICATE_IF0, /* SRCC
                                                                       */
                                     PVR_ROGUE_PDSINST_BRA_NEG_DISABLE, /* NEG
                                                                         */
                                     PVR_ROGUE_PDSINST_PREDICATE_IF1, /* SETC
                                                                       */
                                     1 /* ADDR */);

      /* Encode a HALT. */
      *buffer++ = pvr_pds_inst_encode_halt(1);

      /* Reset the default predicate to IF0. */
      *buffer++ = pvr_pds_encode_bra(PVR_ROGUE_PDSINST_PREDICATE_IF0, /* SRCC
                                                                       */
                                     PVR_ROGUE_PDSINST_BRA_NEG_DISABLE, /* NEG
                                                                         */
                                     PVR_ROGUE_PDSINST_PREDICATE_IF0, /* SETC
                                                                       */
                                     1 /* ADDR */);
   }

   return buffer;
}

/**
 * Generates program to kick the USC task to store shared.
 *
 * \param program Pointer to the PDS shared register.
 * \param buffer Pointer to the buffer for the program.
 * \param gen_mode Either code and data can be generated or sizes only updated.
 * \param dev_info PVR device information structure.
 * \returns Pointer to just beyond the buffer for the program.
 */
uint32_t *pvr_pds_generate_shared_storing_program(
   struct pvr_pds_shared_storing_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info)
{
   struct pvr_pds_kickusc_program *kick_usc_program = &program->usc_task;
   struct pvr_pds_doutw_control *doutw_control = &program->doutw_control;

   if (gen_mode == PDS_GENERATE_SIZES)
      return NULL;

   if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
      uint32_t *constants = buffer;

      constants =
         pvr_pds_generate_doutw(doutw_control, constants, gen_mode, dev_info);
      program->data_size = doutw_control->data_size;

      constants = pvr_pds_kick_usc(kick_usc_program,
                                   constants,
                                   0,
                                   program->cc_enable,
                                   gen_mode);
      program->data_size += kick_usc_program->data_size;

      return constants;
   }

   if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
      /* Generate PDS code segment. */
      uint32_t *instruction = buffer;

      /* doutw	vi1, vi0
       * doutu	ds1[constant_use], ds0[constant_use], ds1[constant_use],
       * emit
       */
      instruction =
         pvr_pds_generate_doutw(doutw_control, buffer, gen_mode, dev_info);
      program->code_size = doutw_control->code_size;

      /* Offset into data segment follows on from doutw data segment. */
      instruction = pvr_pds_kick_usc(kick_usc_program,
                                     instruction,
                                     doutw_control->data_size,
                                     program->cc_enable,
                                     gen_mode);
      program->code_size += kick_usc_program->code_size;

      return instruction;
   }

   /* Execution should never reach here. */
   return NULL;
}

uint32_t *pvr_pds_generate_fence_terminate_program(
   struct pvr_pds_fence_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info)
{
   uint32_t data_size = 0;
   uint32_t code_size = 0;

   if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
      /* Data segment. */
      uint32_t *constants, *constants_base;

      constants = constants_base = (uint32_t *)buffer;

      /* DOUTC sources are not used, but they must be valid. */
      pvr_pds_generate_doutc(program, constants, PDS_GENERATE_DATA_SEGMENT);
      data_size += program->data_size;

      if (PVR_NEED_SW_COMPUTE_PDS_BARRIER(dev_info)) {
         /* Append a 64-bit constant with value 1. Used to increment ptemp.
          * Return the offset into the data segment.
          */
         program->fence_constant_word =
            pvr_pds_append_constant64(constants_base, 1, &data_size);
      }

      program->data_size = data_size;
      return constants;
   }

   if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
      /* Code segment. */
      uint32_t *instruction = (uint32_t *)buffer;

      instruction = pvr_pds_generate_compute_barrier_conditional(
         instruction,
         PDS_GENERATE_CODE_SEGMENT);
      code_size += 3;

      if (PVR_NEED_SW_COMPUTE_PDS_BARRIER(dev_info)) {
         /* lock */
         *instruction++ = pvr_pds_inst_encode_lock(0); /* cc */

         /* add64	pt[0], pt[0], #1 */
         *instruction++ = pvr_pds_inst_encode_add64(
            0, /* cc */
            PVR_ROGUE_PDSINST_ALUM_UNSIGNED,
            PVR_ROGUE_PDSINST_MAD_SNA_ADD,
            PVR_ROGUE_PDSINST_REGS64_PTEMP64_LOWER + 0, /* src0 = ptemp[0]
                                                         */
            PVR_ROGUE_PDSINST_REGS64_CONST64_LOWER +
               (program->fence_constant_word >> 1), /* src1 = 1 */
            PVR_ROGUE_PDSINST_REGS64TP_PTEMP64_LOWER + 0); /* dest =
                                                            * ptemp[0]
                                                            */

         /* release */
         *instruction++ = pvr_pds_inst_encode_release(0); /* cc */

         /* cmp		pt[0] EQ 0x4 == Number of USC clusters per phantom */
         *instruction++ = pvr_pds_inst_encode_cmpi(
            0, /* cc */
            PVR_ROGUE_PDSINST_COP_EQ,
            PVR_ROGUE_PDSINST_REGS64TP_PTEMP64_LOWER + 0, /* src0
                                                           * = ptemp[0]
                                                           */
            PVR_GET_FEATURE_VALUE(dev_info, num_clusters, 0));

         /* bra		-1 */
         *instruction++ =
            pvr_pds_encode_bra(0, /* cc */
                               1, /* PVR_ROGUE_PDSINST_BRA_NEG_ENABLE
                                   */
                               0, /* PVR_ROGUE_PDSINST_BRA_SETC_P0
                                   */
                               -1); /* bra PC */
         code_size += 5;
      }

      /* DOUTC */
      instruction = pvr_pds_generate_doutc(program,
                                           instruction,
                                           PDS_GENERATE_CODE_SEGMENT);
      code_size += program->code_size;

      program->code_size = code_size;
      return instruction;
   }

   /* Execution should never reach here. */
   return NULL;
}

/**
 * Generates program to kick the USC task to load shared registers from memory.
 *
 * \param program Pointer to the PDS shared register.
 * \param buffer Pointer to the buffer for the program.
 * \param gen_mode Either code and data can be generated or sizes only updated.
 * \param dev_info PVR device information struct.
 * \returns Pointer to just beyond the buffer for the program.
 */
uint32_t *pvr_pds_generate_compute_shared_loading_program(
   struct pvr_pds_shared_storing_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info)
{
   struct pvr_pds_kickusc_program *kick_usc_program = &program->usc_task;
   struct pvr_pds_doutw_control *doutw_control = &program->doutw_control;

   uint32_t next_constant;
   uint32_t data_size = 0;
   uint32_t code_size = 0;

   /* This needs to persist to the CODE_SEGMENT call. */
   static uint32_t fence_constant_word = 0;
   uint64_t zero_constant64 = 0;

   if (gen_mode == PDS_GENERATE_SIZES)
      return NULL;

   if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
      uint32_t *constants = buffer;

      constants = pvr_pds_generate_doutw(doutw_control,
                                         constants,
                                         PDS_GENERATE_DATA_SEGMENT,
                                         dev_info);
      data_size += doutw_control->data_size;

      constants = pvr_pds_kick_usc(kick_usc_program,
                                   constants,
                                   0,
                                   program->cc_enable,
                                   gen_mode);
      data_size += kick_usc_program->data_size;

      /* Copy the fence constant value (64-bit). */
      next_constant = data_size; /* Assumes data words fully packed. */
      fence_constant_word =
         pvr_pds_get_constants(&next_constant, 2, &data_size);

      /* Encode the fence constant src0 (offset measured from start of data
       * buffer). Fence barrier is initialized to zero.
       */
      pvr_pds_write_wide_constant(buffer, fence_constant_word, zero_constant64);
      /* Update the const size. */
      data_size += 2;
      constants += 2;

      program->data_size = data_size;
      return constants;
   }

   if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
      /* Generate PDS code segment. */
      uint32_t *instruction = buffer;

      /* add64	pt0, c0, c0
       * IF [2x Phantoms]
       * add64	pt1, c0, c0
       * st		[constant_mem_addr], pt0, 4
       * ENDIF
       * doutw	vi1, vi0
       * doutu	ds1[constant_use], ds0[constant_use], ds1[constant_use],
       * emit
       *
       * Zero the persistent temp (SW fence for context switch).
       */
      *instruction++ = pvr_pds_inst_encode_add64(
         0, /* cc */
         PVR_ROGUE_PDSINST_ALUM_UNSIGNED,
         PVR_ROGUE_PDSINST_MAD_SNA_ADD,
         PVR_ROGUE_PDSINST_REGS64_CONST64_LOWER +
            (fence_constant_word >> 1), /* src0
                                         *  = 0
                                         */
         PVR_ROGUE_PDSINST_REGS64_CONST64_LOWER +
            (fence_constant_word >> 1), /* src1
                                         * = 0
                                         */
         PVR_ROGUE_PDSINST_REGS64TP_PTEMP64_LOWER + 0); /* dest = ptemp64[0]
                                                         */
      code_size++;

      instruction = pvr_pds_generate_doutw(doutw_control,
                                           instruction,
                                           PDS_GENERATE_CODE_SEGMENT,
                                           dev_info);
      code_size += doutw_control->code_size;

      /* Offset into data segment follows on from doutw data segment. */
      instruction = pvr_pds_kick_usc(kick_usc_program,
                                     instruction,
                                     doutw_control->data_size,
                                     program->cc_enable,
                                     gen_mode);
      code_size += kick_usc_program->code_size;

      program->code_size = code_size;
      return instruction;
   }

   /* Execution should never reach here. */
   return NULL;
}

/**
 * Generates both code and data when gen_mode is not PDS_GENERATE_SIZES.
 * Relies on num_fpu_iterators being initialized for size calculation.
 * Relies on num_fpu_iterators, destination[], and FPU_iterators[] being
 * initialized for program generation.
 *
 * \param program Pointer to the PDS pixel shader program.
 * \param buffer Pointer to the buffer for the program.
 * \param gen_mode Either code and data can be generated or sizes only updated.
 * \returns Pointer to just beyond the buffer for the program.
 */
uint32_t *pvr_pds_coefficient_loading(
   struct pvr_pds_coeff_loading_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode)
{
   uint32_t constant;
   uint32_t *instruction;
   uint32_t total_data_size, code_size;

   /* Place constants at the front of the buffer. */
   uint32_t *constants = buffer;
   /* Start counting constants from 0. */
   uint32_t next_constant = PVR_PDS_CONSTANTS_BLOCK_BASE;

   /* Save the data segment pointer and size. */
   program->data_segment = constants;

   total_data_size = 0;
   code_size = 0;

   total_data_size += 2 * program->num_fpu_iterators;
   code_size += program->num_fpu_iterators;

   /* Instructions start where constants finished, but we must take note of
    * alignment.
    *
    * 128-bit boundary = 4 dwords.
    */
   total_data_size = ALIGN_POT(total_data_size, 4);
   if (gen_mode != PDS_GENERATE_SIZES) {
      uint32_t data_size = 0;
      uint32_t iterator = 0;

      instruction = buffer + total_data_size;

      while (iterator < program->num_fpu_iterators) {
         uint64_t iterator_word;

         /* Copy the USC task control words to constants. */
         constant = pvr_pds_get_constants(&next_constant, 2, &data_size);

         /* Write the first iterator. */
         iterator_word =
            (uint64_t)program->FPU_iterators[iterator]
            << PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTI_SRC0_ISSUE0_SHIFT;

         /* Write the destination. */
         iterator_word |=
            (uint64_t)program->destination[iterator++]
            << PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTI_SRC0_DEST_SHIFT;

         /* If this is the last DOUTI word the "Last Issue" bit should be
          * set.
          */
         if (iterator >= program->num_fpu_iterators) {
            iterator_word |= PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTI_SRC0_ISSUE_EN;
         }

         /* Write the word to the buffer. */
         pvr_pds_write_wide_constant(constants,
                                     constant,
                                     iterator_word); /* 64-bit
                                                        Src0
                                                      */

         /* Write the DOUT instruction. */
         *instruction++ = pvr_pds_encode_douti(
            /* cc */ 0,
            /* END */ 0,
            /* SRC0 */ constant >> 1); /* DOUT Issue word 0 64-bit */
      }

      /* Update the last DOUTI instruction to have the END flag set. */
      *(instruction - 1) |= 1 << PVR_ROGUE_PDSINST_DOUT_END_SHIFT;
   } else {
      instruction = NULL;
   }

   /* Update the data size and code size. Minimum temp count is 1. */
   program->temps_used = 1;
   program->data_size = total_data_size;
   program->code_size = code_size;

   return instruction;
}

/**
 * Generate a single ld/st instruction. This can correspond to one or more
 * real ld/st instructions based on the value of count.
 *
 * \param ld true to generate load, false to generate store.
 * \param control Cache mode control.
 * \param temp_index Dest temp for load/source temp for store, in 32bits
 *                   register index.
 * \param address Source for load/dest for store in bytes.
 * \param count Number of dwords for load/store.
 * \param next_constant
 * \param total_data_size
 * \param total_code_size
 * \param buffer Pointer to the buffer for the program.
 * \param data_fence Issue data fence.
 * \param gen_mode Either code and data can be generated or sizes only updated.
 * \param dev_info PVR device information structure.
 * \returns Pointer to just beyond the buffer for the program.
 */
uint32_t *pvr_pds_generate_single_ldst_instruction(
   bool ld,
   const struct pvr_pds_ldst_control *control,
   uint32_t temp_index,
   uint64_t address,
   uint32_t count,
   uint32_t *next_constant,
   uint32_t *total_data_size,
   uint32_t *total_code_size,
   uint32_t *restrict buffer,
   bool data_fence,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info)
{
   /* A single ld/ST here does NOT actually correspond to a single ld/ST
    * instruction, but may needs multiple ld/ST instructions because each ld/ST
    * instruction can only ld/ST a restricted max number of dwords which may
    * less than count passed here.
    */

   uint32_t num_inst;
   uint32_t constant;

   if (ld) {
      /* ld must operate on 64bits unit, and it needs to load from and to 128
       * bits aligned. Apart from the last ld, all the other need to ld 2x(x =
       * 1, 2, ...) times 64bits unit.
       */
      uint32_t per_inst_count = 0;
      uint32_t last_inst_count;

      assert((gen_mode == PDS_GENERATE_SIZES) ||
             (((count % 2) == 0) && ((address % 16) == 0) &&
              (temp_index % 2) == 0));

      count >>= 1;
      temp_index >>= 1;

      /* Found out how many ld instructions are needed and ld size for the all
       * possible ld instructions.
       */
      if (count <= PVR_ROGUE_PDSINST_LD_COUNT8_MAX_SIZE) {
         num_inst = 1;
         last_inst_count = count;
      } else {
         per_inst_count = PVR_ROGUE_PDSINST_LD_COUNT8_MAX_SIZE;
         if ((per_inst_count % 2) != 0)
            per_inst_count -= 1;

         num_inst = count / per_inst_count;
         last_inst_count = count - per_inst_count * num_inst;
         num_inst += 1;
      }

      /* Generate all the instructions. */
      for (uint32_t i = 0; i < num_inst; i++) {
         if ((i == (num_inst - 1)) && (last_inst_count == 0))
            break;

         /* A single load instruction. */
         constant = pvr_pds_get_constants(next_constant, 2, total_data_size);

         if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
            uint64_t ld_src0 = 0;

            ld_src0 |= (((address >> 2) & PVR_ROGUE_PDSINST_LD_SRCADD_MASK)
                        << PVR_ROGUE_PDSINST_LD_LD_SRC0_SRCADD_SHIFT);
            ld_src0 |= (((uint64_t)((i == num_inst - 1) ? last_inst_count
                                                        : per_inst_count) &
                         PVR_ROGUE_PDSINST_LD_COUNT8_MASK)
                        << PVR_ROGUE_PDSINST_LD_LD_SRC0_COUNT8_SHIFT);
            ld_src0 |= (((uint64_t)temp_index & PVR_ROGUE_PDSINST_REGS64TP_MASK)
                        << PVR_ROGUE_PDSINST_LD_LD_SRC0_DEST_SHIFT);

            if (!control) {
               ld_src0 |= PVR_ROGUE_PDSINST_LD_LD_SRC0_CMODE_CACHED;

               if (PVR_HAS_FEATURE(dev_info, slc_mcu_cache_controls))
                  ld_src0 |= PVR_ROGUE_PDSINST_LD_LD_SRC0_SLCMODE_CACHED;

            } else {
               ld_src0 |= control->cache_control_const;
            }

            /* Write it to the constant. */
            pvr_pds_write_constant64(buffer,
                                     constant,
                                     (uint32_t)(ld_src0),
                                     (uint32_t)(ld_src0 >> 32));

            /* Adjust value for next ld instruction. */
            temp_index += per_inst_count;
            address += (((uint64_t)(per_inst_count)) << 3);
         }

         if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
            *buffer++ = pvr_pds_inst_encode_ld(0, constant >> 1);

            if (data_fence)
               *buffer++ = pvr_pds_inst_encode_wdf(0);
         }
      }
   } else {
      /* ST needs source memory address to be 32bits aligned. */
      assert((gen_mode == PDS_GENERATE_SIZES) || ((address % 4) == 0));

      /* Found out how many ST instructions are needed, each ST can only store
       * PVR_ROGUE_PDSINST_ST_COUNT4_MASK number of 32bits.
       */
      num_inst = count / PVR_ROGUE_PDSINST_ST_COUNT4_MAX_SIZE;
      num_inst += ((count % PVR_ROGUE_PDSINST_ST_COUNT4_MAX_SIZE) == 0 ? 0 : 1);

      /* Generate all the instructions. */
      for (uint32_t i = 0; i < num_inst; i++) {
         /* A single store instruction. */
         constant = pvr_pds_get_constants(next_constant, 2, total_data_size);

         if (gen_mode == PDS_GENERATE_DATA_SEGMENT) {
            uint32_t per_inst_count =
               (count <= PVR_ROGUE_PDSINST_ST_COUNT4_MAX_SIZE
                   ? count
                   : PVR_ROGUE_PDSINST_ST_COUNT4_MAX_SIZE);
            uint64_t st_src0 = 0;

            st_src0 |= (((address >> 2) & PVR_ROGUE_PDSINST_ST_SRCADD_MASK)
                        << PVR_ROGUE_PDSINST_ST_ST_SRC0_DSTADD_SHIFT);
            st_src0 |=
               (((uint64_t)per_inst_count & PVR_ROGUE_PDSINST_ST_COUNT4_MASK)
                << PVR_ROGUE_PDSINST_ST_ST_SRC0_COUNT4_SHIFT);
            st_src0 |= (((uint64_t)temp_index & PVR_ROGUE_PDSINST_REGS32TP_MASK)
                        << PVR_ROGUE_PDSINST_ST_ST_SRC0_SRC_SHIFT);

            if (!control) {
               st_src0 |= PVR_ROGUE_PDSINST_ST_ST_SRC0_CMODE_WRITE_THROUGH;

               if (PVR_HAS_FEATURE(dev_info, slc_mcu_cache_controls)) {
                  st_src0 |= PVR_ROGUE_PDSINST_ST_ST_SRC0_SLCMODE_WRITE_THROUGH;
               }

            } else {
               st_src0 |= control->cache_control_const;
            }

            /* Write it to the constant. */
            pvr_pds_write_constant64(buffer,
                                     constant,
                                     (uint32_t)(st_src0),
                                     (uint32_t)(st_src0 >> 32));

            /* Adjust value for next ST instruction. */
            temp_index += per_inst_count;
            count -= per_inst_count;
            address += (((uint64_t)(per_inst_count)) << 2);
         }

         if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
            *buffer++ = pvr_pds_inst_encode_st(0, constant >> 1);

            if (data_fence)
               *buffer++ = pvr_pds_inst_encode_wdf(0);
         }
      }
   }

   (*total_code_size) += num_inst;
   if (data_fence)
      (*total_code_size) += num_inst;

   if (gen_mode != PDS_GENERATE_SIZES)
      return buffer;
   return NULL;
}

/**
 * Generate programs used to prepare stream out, i.e., clear stream out buffer
 * overflow flags and update Persistent temps by a ld instruction.
 *
 * This must be used in PPP state update.
 *
 * \param program Pointer to the stream out program.
 * \param buffer Pointer to the buffer for the program.
 * \param store_mode If true then the data is stored to memory. If false then
 *                   the data is loaded from memory.
 * \param gen_mode Either code and data can be generated or sizes only updated.
 * \param dev_info PVR device information structure.
 * \returns Pointer to just beyond the buffer for the program.
 */
uint32_t *pvr_pds_generate_stream_out_init_program(
   struct pvr_pds_stream_out_init_program *restrict program,
   uint32_t *restrict buffer,
   bool store_mode,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info)
{
   uint32_t total_data_size = 0;
   uint32_t PTDst = PVR_ROGUE_PDSINST_REGS32TP_PTEMP32_LOWER;

   /* Start counting constants from 0. */
   uint32_t next_constant = PVR_PDS_CONSTANTS_BLOCK_BASE;

   uint32_t total_code_size = 1;

   if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
      /* We only need to clear global stream out predicate, other predicates
       * are not used during the stream out buffer overflow test.
       */
      *buffer++ = pvr_pds_inst_encode_stmc(0, 0x10);
   }

   for (uint32_t index = 0; index < program->num_buffers; index++) {
      if (program->dev_address_for_buffer_data[index] != 0) {
         /* Generate load/store program to load/store persistent temps. */

         /* NOTE: store_mode == true case should be handled by
          * StreamOutTerminate.
          */
         buffer = pvr_pds_generate_single_ldst_instruction(
            !store_mode,
            NULL,
            PTDst,
            program->dev_address_for_buffer_data[index],
            program->pds_buffer_data_size[index],
            &next_constant,
            &total_data_size,
            &total_code_size,
            buffer,
            false,
            gen_mode,
            dev_info);
      }

      PTDst += program->pds_buffer_data_size[index];
   }

   total_code_size += 2;

   if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
      /* We need to fence the loading. */
      *buffer++ = pvr_pds_inst_encode_wdf(0);
      *buffer++ = pvr_pds_inst_encode_halt(0);
   }

   /* Save size information to program */
   program->stream_out_init_pds_data_size =
      ALIGN_POT(total_data_size, 4); /* 128-bit boundary = 4 dwords; */
   /* PDS program code size. */
   program->stream_out_init_pds_code_size = total_code_size;

   if (gen_mode == PDS_GENERATE_DATA_SEGMENT)
      return buffer + program->stream_out_init_pds_data_size;
   else if (gen_mode == PDS_GENERATE_CODE_SEGMENT)
      return buffer;

   return NULL;
}

/**
 * Generate stream out terminate program for stream out.
 *
 * If pds_persistent_temp_size_to_store is 0, the final primitive written value
 * will be stored.
 *
 * If pds_persistent_temp_size_to_store is non 0, the value of persistent temps
 * will be stored into memory.
 *
 * The stream out terminate program is used to update the PPP state and the data
 * and code section cannot be separate.
 *
 * \param program Pointer to the stream out program.
 * \param buffer Pointer to the buffer for the program.
 * \param gen_mode Either code and data can be generated or sizes only updated.
 * \param dev_info PVR device info structure.
 * \returns Pointer to just beyond the buffer for the program.
 */
uint32_t *pvr_pds_generate_stream_out_terminate_program(
   struct pvr_pds_stream_out_terminate_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info)
{
   uint32_t next_constant;
   uint32_t total_data_size = 0, total_code_size = 0;

   /* Start counting constants from 0. */
   next_constant = PVR_PDS_CONSTANTS_BLOCK_BASE;

   /* Generate store program to store persistent temps. */
   buffer = pvr_pds_generate_single_ldst_instruction(
      false,
      NULL,
      PVR_ROGUE_PDSINST_REGS32TP_PTEMP32_LOWER,
      program->dev_address_for_storing_persistent_temp,
      program->pds_persistent_temp_size_to_store,
      &next_constant,
      &total_data_size,
      &total_code_size,
      buffer,
      false,
      gen_mode,
      dev_info);

   total_code_size += 2;
   if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
      *buffer++ = pvr_pds_inst_encode_wdf(0);
      *buffer++ = pvr_pds_inst_encode_halt(0);
   }

   /* Save size information to program. */
   program->stream_out_terminate_pds_data_size =
      ALIGN_POT(total_data_size, 4); /* 128-bit boundary = 4 dwords; */
   /* PDS program code size. */
   program->stream_out_terminate_pds_code_size = total_code_size;

   if (gen_mode == PDS_GENERATE_DATA_SEGMENT)
      return buffer + program->stream_out_terminate_pds_data_size;
   else if (gen_mode == PDS_GENERATE_CODE_SEGMENT)
      return buffer;

   return NULL;
}

/* DrawArrays works in several steps:
 *
 * 1) load data from draw_indirect buffer
 * 2) tweak data to match hardware formats
 * 3) write data to indexblock
 * 4) signal the VDM to continue
 *
 * This is complicated by HW limitations on alignment, as well as a HWBRN.
 *
 * 1) Load data.
 * Loads _must_ be 128-bit aligned. Because there is no such limitation in the
 * spec we must deal with this by choosing an appropriate earlier address and
 * loading enough dwords that we load the entirety of the buffer.
 *
 * if addr & 0xf:
 *   load [addr & ~0xf] 6 dwords -> tmp[0, 1, 2, 3, 4, 5]
 *   data = tmp[0 + (uiAddr & 0xf) >> 2]...
 * else
 *   load [addr] 4 dwords -> tmp[0, 1, 2, 3]
 *   data = tmp[0]...
 *
 *
 * 2) Tweak data.
 * primCount in the spec does not match the encoding of INDEX_INSTANCE_COUNT in
 * the VDM control stream. We must subtract 1 from the loaded primCount.
 *
 * However, there is a HWBRN that disallows the ADD32 instruction from sourcing
 * a tmp that is non-64-bit-aligned. To work around this, we must move primCount
 * into another tmp that has the correct alignment. Note: this is only required
 * when data = tmp[even], as primCount is data+1:
 *
 * if data = tmp[even]:
 *   primCount = data + 1 = tmp[odd] -- not 64-bit aligned!
 * else:
 *   primCount = data + 1 = tmp[even] -- already aligned, don't need workaround.
 *
 * This boils down to:
 *
 * primCount = data[1]
 * primCountSrc = data[1]
 * if brn_present && (data is even):
 *   mov scratch, primCount
 *   primCountSrc = scratch
 * endif
 * sub primCount, primCountSrc, 1
 *
 * 3) Store Data.
 * Write the now-tweaked data over the top of the indexblock.
 * To ensure the write completes before the VDM re-reads the data, we must cause
 * a data hazard by doing a dummy (dummy meaning we don't care about the
 * returned data) load from the same addresses. Again, because the ld must
 * always be 128-bit aligned (note: the ST is dword-aligned), we must ensure the
 * index block is 128-bit aligned. This is the client driver's responsibility.
 *
 * st data[0, 1, 2] -> (idxblock + 4)
 * load [idxblock] 4 dwords
 *
 * 4) Signal the VDM
 * This is simply a DOUTV with a src1 of 0, indicating the VDM should continue
 * where it is currently fenced on a dummy idxblock that has been inserted by
 * the driver.
 */

#include "pvr_draw_indirect_arrays0.h"
#include "pvr_draw_indirect_arrays1.h"
#include "pvr_draw_indirect_arrays2.h"
#include "pvr_draw_indirect_arrays3.h"

#include "pvr_draw_indirect_arrays_base_instance0.h"
#include "pvr_draw_indirect_arrays_base_instance1.h"
#include "pvr_draw_indirect_arrays_base_instance2.h"
#include "pvr_draw_indirect_arrays_base_instance3.h"

#include "pvr_draw_indirect_arrays_base_instance_drawid0.h"
#include "pvr_draw_indirect_arrays_base_instance_drawid1.h"
#include "pvr_draw_indirect_arrays_base_instance_drawid2.h"
#include "pvr_draw_indirect_arrays_base_instance_drawid3.h"

#define ENABLE_SLC_MCU_CACHE_CONTROLS(device)        \
   ((device)->features.has_slc_mcu_cache_controls    \
       ? PVR_ROGUE_PDSINST_LD_LD_SRC0_SLCMODE_CACHED \
       : PVR_ROGUE_PDSINST_LD_LD_SRC0_SLCMODE_BYPASS)

void pvr_pds_generate_draw_arrays_indirect(
   struct pvr_pds_drawindirect_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info)
{
   if ((gen_mode == PDS_GENERATE_CODE_SEGMENT) ||
       (gen_mode == PDS_GENERATE_SIZES)) {
      const struct pvr_psc_program_output *psc_program = NULL;
      switch ((program->arg_buffer >> 2) % 4) {
      case 0:
         if (program->support_base_instance) {
            if (program->increment_draw_id) {
               psc_program =
                  &pvr_draw_indirect_arrays_base_instance_drawid0_program;
            } else {
               psc_program = &pvr_draw_indirect_arrays_base_instance0_program;
            }
         } else {
            psc_program = &pvr_draw_indirect_arrays0_program;
         }
         break;
      case 1:
         if (program->support_base_instance) {
            if (program->increment_draw_id) {
               psc_program =
                  &pvr_draw_indirect_arrays_base_instance_drawid1_program;
            } else {
               psc_program = &pvr_draw_indirect_arrays_base_instance1_program;
            }
         } else {
            psc_program = &pvr_draw_indirect_arrays1_program;
         }
         break;
      case 2:
         if (program->support_base_instance) {
            if (program->increment_draw_id) {
               psc_program =
                  &pvr_draw_indirect_arrays_base_instance_drawid2_program;
            } else {
               psc_program = &pvr_draw_indirect_arrays_base_instance2_program;
            }
         } else {
            psc_program = &pvr_draw_indirect_arrays2_program;
         }
         break;
      case 3:
         if (program->support_base_instance) {
            if (program->increment_draw_id) {
               psc_program =
                  &pvr_draw_indirect_arrays_base_instance_drawid3_program;
            } else {
               psc_program = &pvr_draw_indirect_arrays_base_instance3_program;
            }
         } else {
            psc_program = &pvr_draw_indirect_arrays3_program;
         }
         break;
      }

      if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
         memcpy(buffer,
                psc_program->code,
                psc_program->code_size * sizeof(uint32_t));
#if defined(DUMP_PDS)
         for (uint32_t i = 0; i < psc_program->code_size; i++)
            PVR_PDS_PRINT_INST(buffer[i]);
#endif
      }

      program->program = *psc_program;
   } else {
      switch ((program->arg_buffer >> 2) % 4) {
      case 0:
         if (program->support_base_instance) {
            if (program->increment_draw_id) {
               pvr_write_draw_indirect_arrays_base_instance_drawid0_di_data(
                  buffer,
                  program->arg_buffer & ~0xfull,
                  dev_info);
               pvr_write_draw_indirect_arrays_base_instance_drawid0_write_vdm(
                  buffer,
                  program->index_list_addr_buffer + 4);
               pvr_write_draw_indirect_arrays_base_instance_drawid0_flush_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_arrays_base_instance_drawid0_num_views(
                  buffer,
                  program->num_views);
               pvr_write_draw_indirect_arrays_base_instance_drawid0_immediates(
                  buffer);
            } else {
               pvr_write_draw_indirect_arrays_base_instance0_di_data(
                  buffer,
                  program->arg_buffer & ~0xfull,
                  dev_info);
               pvr_write_draw_indirect_arrays_base_instance0_write_vdm(
                  buffer,
                  program->index_list_addr_buffer + 4);
               pvr_write_draw_indirect_arrays_base_instance0_flush_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_arrays_base_instance0_num_views(
                  buffer,
                  program->num_views);
               pvr_write_draw_indirect_arrays_base_instance0_immediates(buffer);
            }
         } else {
            pvr_write_draw_indirect_arrays0_di_data(buffer,
                                                    program->arg_buffer &
                                                       ~0xfull,
                                                    dev_info);
            pvr_write_draw_indirect_arrays0_write_vdm(
               buffer,
               program->index_list_addr_buffer + 4);
            pvr_write_draw_indirect_arrays0_flush_vdm(
               buffer,
               program->index_list_addr_buffer);
            pvr_write_draw_indirect_arrays0_num_views(buffer,
                                                      program->num_views);
            pvr_write_draw_indirect_arrays0_immediates(buffer);
         }
         break;
      case 1:
         if (program->support_base_instance) {
            if (program->increment_draw_id) {
               pvr_write_draw_indirect_arrays_base_instance_drawid1_di_data(
                  buffer,
                  program->arg_buffer & ~0xfull,
                  dev_info);
               pvr_write_draw_indirect_arrays_base_instance_drawid1_write_vdm(
                  buffer,
                  program->index_list_addr_buffer + 4);
               pvr_write_draw_indirect_arrays_base_instance_drawid1_flush_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_arrays_base_instance_drawid1_num_views(
                  buffer,
                  program->num_views);
               pvr_write_draw_indirect_arrays_base_instance_drawid1_immediates(
                  buffer);
            } else {
               pvr_write_draw_indirect_arrays_base_instance1_di_data(
                  buffer,
                  program->arg_buffer & ~0xfull,
                  dev_info);
               pvr_write_draw_indirect_arrays_base_instance1_write_vdm(
                  buffer,
                  program->index_list_addr_buffer + 4);
               pvr_write_draw_indirect_arrays_base_instance1_flush_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_arrays_base_instance1_num_views(
                  buffer,
                  program->num_views);
               pvr_write_draw_indirect_arrays_base_instance1_immediates(buffer);
            }
         } else {
            pvr_write_draw_indirect_arrays1_di_data(buffer,
                                                    program->arg_buffer &
                                                       ~0xfull,
                                                    dev_info);
            pvr_write_draw_indirect_arrays1_write_vdm(
               buffer,
               program->index_list_addr_buffer + 4);
            pvr_write_draw_indirect_arrays1_flush_vdm(
               buffer,
               program->index_list_addr_buffer);
            pvr_write_draw_indirect_arrays1_num_views(buffer,
                                                      program->num_views);
            pvr_write_draw_indirect_arrays1_immediates(buffer);
         }
         break;
      case 2:
         if (program->support_base_instance) {
            if (program->increment_draw_id) {
               pvr_write_draw_indirect_arrays_base_instance_drawid2_di_data(
                  buffer,
                  program->arg_buffer & ~0xfull,
                  dev_info);
               pvr_write_draw_indirect_arrays_base_instance_drawid2_write_vdm(
                  buffer,
                  program->index_list_addr_buffer + 4);
               pvr_write_draw_indirect_arrays_base_instance_drawid2_flush_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_arrays_base_instance_drawid2_num_views(
                  buffer,
                  program->num_views);
               pvr_write_draw_indirect_arrays_base_instance_drawid2_immediates(
                  buffer);
            } else {
               pvr_write_draw_indirect_arrays_base_instance2_di_data(
                  buffer,
                  program->arg_buffer & ~0xfull,
                  dev_info);
               pvr_write_draw_indirect_arrays_base_instance2_write_vdm(
                  buffer,
                  program->index_list_addr_buffer + 4);
               pvr_write_draw_indirect_arrays_base_instance2_flush_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_arrays_base_instance2_num_views(
                  buffer,
                  program->num_views);
               pvr_write_draw_indirect_arrays_base_instance2_immediates(buffer);
            }
         } else {
            pvr_write_draw_indirect_arrays2_di_data(buffer,
                                                    program->arg_buffer &
                                                       ~0xfull,
                                                    dev_info);
            pvr_write_draw_indirect_arrays2_write_vdm(
               buffer,
               program->index_list_addr_buffer + 4);
            pvr_write_draw_indirect_arrays2_flush_vdm(
               buffer,
               program->index_list_addr_buffer);
            pvr_write_draw_indirect_arrays2_num_views(buffer,
                                                      program->num_views);
            pvr_write_draw_indirect_arrays2_immediates(buffer);
         }
         break;
      case 3:
         if (program->support_base_instance) {
            if (program->increment_draw_id) {
               pvr_write_draw_indirect_arrays_base_instance_drawid3_di_data(
                  buffer,
                  program->arg_buffer & ~0xfull,
                  dev_info);
               pvr_write_draw_indirect_arrays_base_instance_drawid3_write_vdm(
                  buffer,
                  program->index_list_addr_buffer + 4);
               pvr_write_draw_indirect_arrays_base_instance_drawid3_flush_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_arrays_base_instance_drawid3_num_views(
                  buffer,
                  program->num_views);
               pvr_write_draw_indirect_arrays_base_instance_drawid3_immediates(
                  buffer);
            } else {
               pvr_write_draw_indirect_arrays_base_instance3_di_data(
                  buffer,
                  program->arg_buffer & ~0xfull,
                  dev_info);
               pvr_write_draw_indirect_arrays_base_instance3_write_vdm(
                  buffer,
                  program->index_list_addr_buffer + 4);
               pvr_write_draw_indirect_arrays_base_instance3_flush_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_arrays_base_instance3_num_views(
                  buffer,
                  program->num_views);
               pvr_write_draw_indirect_arrays_base_instance3_immediates(buffer);
            }
         } else {
            pvr_write_draw_indirect_arrays3_di_data(buffer,
                                                    program->arg_buffer &
                                                       ~0xfull,
                                                    dev_info);
            pvr_write_draw_indirect_arrays3_write_vdm(
               buffer,
               program->index_list_addr_buffer + 4);
            pvr_write_draw_indirect_arrays3_flush_vdm(
               buffer,
               program->index_list_addr_buffer);
            pvr_write_draw_indirect_arrays3_num_views(buffer,
                                                      program->num_views);
            pvr_write_draw_indirect_arrays3_immediates(buffer);
         }
         break;
      }
   }
}

#include "pvr_draw_indirect_elements0.h"
#include "pvr_draw_indirect_elements1.h"
#include "pvr_draw_indirect_elements2.h"
#include "pvr_draw_indirect_elements3.h"
#include "pvr_draw_indirect_elements_base_instance0.h"
#include "pvr_draw_indirect_elements_base_instance1.h"
#include "pvr_draw_indirect_elements_base_instance2.h"
#include "pvr_draw_indirect_elements_base_instance3.h"
#include "pvr_draw_indirect_elements_base_instance_drawid0.h"
#include "pvr_draw_indirect_elements_base_instance_drawid1.h"
#include "pvr_draw_indirect_elements_base_instance_drawid2.h"
#include "pvr_draw_indirect_elements_base_instance_drawid3.h"

void pvr_pds_generate_draw_elements_indirect(
   struct pvr_pds_drawindirect_program *restrict program,
   uint32_t *restrict buffer,
   enum pvr_pds_generate_mode gen_mode,
   const struct pvr_device_info *dev_info)
{
   if ((gen_mode == PDS_GENERATE_CODE_SEGMENT) ||
       (gen_mode == PDS_GENERATE_SIZES)) {
      const struct pvr_psc_program_output *psc_program = NULL;
      switch ((program->arg_buffer >> 2) % 4) {
      case 0:
         if (program->support_base_instance) {
            if (program->increment_draw_id) {
               psc_program =
                  &pvr_draw_indirect_elements_base_instance_drawid0_program;
            } else {
               psc_program = &pvr_draw_indirect_elements_base_instance0_program;
            }
         } else {
            psc_program = &pvr_draw_indirect_elements0_program;
         }
         break;
      case 1:
         if (program->support_base_instance) {
            if (program->increment_draw_id) {
               psc_program =
                  &pvr_draw_indirect_elements_base_instance_drawid1_program;
            } else {
               psc_program = &pvr_draw_indirect_elements_base_instance1_program;
            }
         } else {
            psc_program = &pvr_draw_indirect_elements1_program;
         }
         break;
      case 2:
         if (program->support_base_instance) {
            if (program->increment_draw_id) {
               psc_program =
                  &pvr_draw_indirect_elements_base_instance_drawid2_program;
            } else {
               psc_program = &pvr_draw_indirect_elements_base_instance2_program;
            }
         } else {
            psc_program = &pvr_draw_indirect_elements2_program;
         }
         break;
      case 3:
         if (program->support_base_instance) {
            if (program->increment_draw_id) {
               psc_program =
                  &pvr_draw_indirect_elements_base_instance_drawid3_program;
            } else {
               psc_program = &pvr_draw_indirect_elements_base_instance3_program;
            }
         } else {
            psc_program = &pvr_draw_indirect_elements3_program;
         }
         break;
      }

      if (gen_mode == PDS_GENERATE_CODE_SEGMENT) {
         memcpy(buffer,
                psc_program->code,
                psc_program->code_size * sizeof(uint32_t));

#if defined(DUMP_PDS)
         for (uint32_t i = 0; i < psc_program->code_size; i++)
            PVR_PDS_PRINT_INST(buffer[i]);
#endif
      }

      program->program = *psc_program;
   } else {
      switch ((program->arg_buffer >> 2) % 4) {
      case 0:
         if (program->support_base_instance) {
            if (program->increment_draw_id) {
               pvr_write_draw_indirect_elements_base_instance_drawid0_di_data(
                  buffer,
                  program->arg_buffer & ~0xfull,
                  dev_info);
               pvr_write_draw_indirect_elements_base_instance_drawid0_write_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_elements_base_instance_drawid0_flush_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_elements_base_instance_drawid0_num_views(
                  buffer,
                  program->num_views);
               pvr_write_draw_indirect_elements_base_instance_drawid0_idx_stride(
                  buffer,
                  program->index_stride);
               pvr_write_draw_indirect_elements_base_instance_drawid0_idx_base(
                  buffer,
                  program->index_buffer);
               pvr_write_draw_indirect_elements_base_instance_drawid0_idx_header(
                  buffer,
                  program->index_block_header);
               pvr_write_draw_indirect_elements_base_instance_drawid0_immediates(
                  buffer);
            } else {
               pvr_write_draw_indirect_elements_base_instance0_di_data(
                  buffer,
                  program->arg_buffer & ~0xfull,
                  dev_info);
               pvr_write_draw_indirect_elements_base_instance0_write_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_elements_base_instance0_flush_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_elements_base_instance0_num_views(
                  buffer,
                  program->num_views);
               pvr_write_draw_indirect_elements_base_instance0_idx_stride(
                  buffer,
                  program->index_stride);
               pvr_write_draw_indirect_elements_base_instance0_idx_base(
                  buffer,
                  program->index_buffer);
               pvr_write_draw_indirect_elements_base_instance0_idx_header(
                  buffer,
                  program->index_block_header);
               pvr_write_draw_indirect_elements_base_instance0_immediates(
                  buffer);
            }
         } else {
            pvr_write_draw_indirect_elements0_di_data(buffer,
                                                      program->arg_buffer &
                                                         ~0xfull,
                                                      dev_info);
            pvr_write_draw_indirect_elements0_write_vdm(
               buffer,
               program->index_list_addr_buffer);
            pvr_write_draw_indirect_elements0_flush_vdm(
               buffer,
               program->index_list_addr_buffer);
            pvr_write_draw_indirect_elements0_num_views(buffer,
                                                        program->num_views);
            pvr_write_draw_indirect_elements0_idx_stride(buffer,
                                                         program->index_stride);
            pvr_write_draw_indirect_elements0_idx_base(buffer,
                                                       program->index_buffer);
            pvr_write_draw_indirect_elements0_idx_header(
               buffer,
               program->index_block_header);
            pvr_write_draw_indirect_elements0_immediates(buffer);
         }
         break;
      case 1:
         if (program->support_base_instance) {
            if (program->increment_draw_id) {
               pvr_write_draw_indirect_elements_base_instance_drawid1_di_data(
                  buffer,
                  program->arg_buffer & ~0xfull,
                  dev_info);
               pvr_write_draw_indirect_elements_base_instance_drawid1_write_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_elements_base_instance_drawid1_flush_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_elements_base_instance_drawid1_num_views(
                  buffer,
                  program->num_views);
               pvr_write_draw_indirect_elements_base_instance_drawid1_idx_stride(
                  buffer,
                  program->index_stride);
               pvr_write_draw_indirect_elements_base_instance_drawid1_idx_base(
                  buffer,
                  program->index_buffer);
               pvr_write_draw_indirect_elements_base_instance_drawid1_idx_header(
                  buffer,
                  program->index_block_header);
               pvr_write_draw_indirect_elements_base_instance_drawid1_immediates(
                  buffer);
            } else {
               pvr_write_draw_indirect_elements_base_instance1_di_data(
                  buffer,
                  program->arg_buffer & ~0xfull,
                  dev_info);
               pvr_write_draw_indirect_elements_base_instance1_write_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_elements_base_instance1_flush_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_elements_base_instance1_num_views(
                  buffer,
                  program->num_views);
               pvr_write_draw_indirect_elements_base_instance1_idx_stride(
                  buffer,
                  program->index_stride);
               pvr_write_draw_indirect_elements_base_instance1_idx_base(
                  buffer,
                  program->index_buffer);
               pvr_write_draw_indirect_elements_base_instance1_idx_header(
                  buffer,
                  program->index_block_header);
               pvr_write_draw_indirect_elements_base_instance1_immediates(
                  buffer);
            }
         } else {
            pvr_write_draw_indirect_elements1_di_data(buffer,
                                                      program->arg_buffer &
                                                         ~0xfull,
                                                      dev_info);
            pvr_write_draw_indirect_elements1_write_vdm(
               buffer,
               program->index_list_addr_buffer);
            pvr_write_draw_indirect_elements1_flush_vdm(
               buffer,
               program->index_list_addr_buffer);
            pvr_write_draw_indirect_elements1_num_views(buffer,
                                                        program->num_views);
            pvr_write_draw_indirect_elements1_idx_stride(buffer,
                                                         program->index_stride);
            pvr_write_draw_indirect_elements1_idx_base(buffer,
                                                       program->index_buffer);
            pvr_write_draw_indirect_elements1_idx_header(
               buffer,
               program->index_block_header);
            pvr_write_draw_indirect_elements1_immediates(buffer);
         }
         break;
      case 2:
         if (program->support_base_instance) {
            if (program->increment_draw_id) {
               pvr_write_draw_indirect_elements_base_instance_drawid2_di_data(
                  buffer,
                  program->arg_buffer & ~0xfull,
                  dev_info);
               pvr_write_draw_indirect_elements_base_instance_drawid2_write_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_elements_base_instance_drawid2_flush_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_elements_base_instance_drawid2_num_views(
                  buffer,
                  program->num_views);
               pvr_write_draw_indirect_elements_base_instance_drawid2_idx_stride(
                  buffer,
                  program->index_stride);
               pvr_write_draw_indirect_elements_base_instance_drawid2_idx_base(
                  buffer,
                  program->index_buffer);
               pvr_write_draw_indirect_elements_base_instance_drawid2_idx_header(
                  buffer,
                  program->index_block_header);
               pvr_write_draw_indirect_elements_base_instance_drawid2_immediates(
                  buffer);
            } else {
               pvr_write_draw_indirect_elements_base_instance2_di_data(
                  buffer,
                  program->arg_buffer & ~0xfull,
                  dev_info);
               pvr_write_draw_indirect_elements_base_instance2_write_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_elements_base_instance2_flush_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_elements_base_instance2_num_views(
                  buffer,
                  program->num_views);
               pvr_write_draw_indirect_elements_base_instance2_idx_stride(
                  buffer,
                  program->index_stride);
               pvr_write_draw_indirect_elements_base_instance2_idx_base(
                  buffer,
                  program->index_buffer);
               pvr_write_draw_indirect_elements_base_instance2_idx_header(
                  buffer,
                  program->index_block_header);
               pvr_write_draw_indirect_elements_base_instance2_immediates(
                  buffer);
            }
         } else {
            pvr_write_draw_indirect_elements2_di_data(buffer,
                                                      program->arg_buffer &
                                                         ~0xfull,
                                                      dev_info);
            pvr_write_draw_indirect_elements2_write_vdm(
               buffer,
               program->index_list_addr_buffer);
            pvr_write_draw_indirect_elements2_flush_vdm(
               buffer,
               program->index_list_addr_buffer);
            pvr_write_draw_indirect_elements2_num_views(buffer,
                                                        program->num_views);
            pvr_write_draw_indirect_elements2_idx_stride(buffer,
                                                         program->index_stride);
            pvr_write_draw_indirect_elements2_idx_base(buffer,
                                                       program->index_buffer);
            pvr_write_draw_indirect_elements2_idx_header(
               buffer,
               program->index_block_header);
            pvr_write_draw_indirect_elements2_immediates(buffer);
         }
         break;
      case 3:
         if (program->support_base_instance) {
            if (program->increment_draw_id) {
               pvr_write_draw_indirect_elements_base_instance_drawid3_di_data(
                  buffer,
                  program->arg_buffer & ~0xfull,
                  dev_info);
               pvr_write_draw_indirect_elements_base_instance_drawid3_write_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_elements_base_instance_drawid3_flush_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_elements_base_instance_drawid3_num_views(
                  buffer,
                  program->num_views);
               pvr_write_draw_indirect_elements_base_instance_drawid3_idx_stride(
                  buffer,
                  program->index_stride);
               pvr_write_draw_indirect_elements_base_instance_drawid3_idx_base(
                  buffer,
                  program->index_buffer);
               pvr_write_draw_indirect_elements_base_instance_drawid3_idx_header(
                  buffer,
                  program->index_block_header);
               pvr_write_draw_indirect_elements_base_instance_drawid3_immediates(
                  buffer);
            } else {
               pvr_write_draw_indirect_elements_base_instance3_di_data(
                  buffer,
                  program->arg_buffer & ~0xfull,
                  dev_info);
               pvr_write_draw_indirect_elements_base_instance3_write_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_elements_base_instance3_flush_vdm(
                  buffer,
                  program->index_list_addr_buffer);
               pvr_write_draw_indirect_elements_base_instance3_num_views(
                  buffer,
                  program->num_views);
               pvr_write_draw_indirect_elements_base_instance3_idx_stride(
                  buffer,
                  program->index_stride);
               pvr_write_draw_indirect_elements_base_instance3_idx_base(
                  buffer,
                  program->index_buffer);
               pvr_write_draw_indirect_elements_base_instance3_idx_header(
                  buffer,
                  program->index_block_header);
               pvr_write_draw_indirect_elements_base_instance3_immediates(
                  buffer);
            }
         } else {
            pvr_write_draw_indirect_elements3_di_data(buffer,
                                                      program->arg_buffer &
                                                         ~0xfull,
                                                      dev_info);
            pvr_write_draw_indirect_elements3_write_vdm(
               buffer,
               program->index_list_addr_buffer);
            pvr_write_draw_indirect_elements3_flush_vdm(
               buffer,
               program->index_list_addr_buffer);
            pvr_write_draw_indirect_elements3_num_views(buffer,
                                                        program->num_views);
            pvr_write_draw_indirect_elements3_idx_stride(buffer,
                                                         program->index_stride);
            pvr_write_draw_indirect_elements3_idx_base(buffer,
                                                       program->index_buffer);
            pvr_write_draw_indirect_elements3_idx_header(
               buffer,
               program->index_block_header);
            pvr_write_draw_indirect_elements3_immediates(buffer);
         }
         break;
      }
   }
}
