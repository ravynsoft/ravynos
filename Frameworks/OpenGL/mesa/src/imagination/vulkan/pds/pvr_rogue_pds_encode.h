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

#ifndef PVR_ROGUE_PDS_ENCODE_H
#define PVR_ROGUE_PDS_ENCODE_H

#include <stdint.h>

#include "pvr_rogue_pds_defs.h"
#include "pvr_rogue_pds_disasm.h"
#include "util/macros.h"

static ALWAYS_INLINE uint32_t
pvr_pds_inst_decode_field_range_regs64tp(uint32_t value)
{
   if (value <= PVR_ROGUE_PDSINST_REGS64TP_TEMP64_UPPER)
      return PVR_ROGUE_PDSINST_REGS64TP_TEMP64;

   if ((value >= PVR_ROGUE_PDSINST_REGS64TP_PTEMP64_LOWER) &&
       (value <= PVR_ROGUE_PDSINST_REGS64TP_PTEMP64_UPPER)) {
      return PVR_ROGUE_PDSINST_REGS64TP_PTEMP64;
   }
   return 2;
}

static ALWAYS_INLINE uint32_t
pvr_pds_inst_decode_field_range_regs32(uint32_t value)
{
   if (value <= PVR_ROGUE_PDSINST_REGS32_CONST32_UPPER)
      return PVR_ROGUE_PDSINST_REGS32_CONST32;

   if ((value >= PVR_ROGUE_PDSINST_REGS32_TEMP32_LOWER) &&
       (value <= PVR_ROGUE_PDSINST_REGS32_TEMP32_UPPER)) {
      return PVR_ROGUE_PDSINST_REGS32_TEMP32;
   }
   if ((value >= PVR_ROGUE_PDSINST_REGS32_PTEMP32_LOWER) &&
       (value <= PVR_ROGUE_PDSINST_REGS32_PTEMP32_UPPER)) {
      return PVR_ROGUE_PDSINST_REGS32_PTEMP32;
   }
   return 3;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_sftlp64(uint32_t cc,
                                                          uint32_t lop,
                                                          uint32_t im,
                                                          uint32_t src0,
                                                          uint32_t src1,
                                                          uint32_t src2,
                                                          uint32_t dst)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEC_SFTLP64
              << PVR_ROGUE_PDSINST_SFTLP64_OPCODE_SHIFT;
   encoded |= ((dst & PVR_ROGUE_PDSINST_REGS64TP_MASK)
               << PVR_ROGUE_PDSINST_SFTLP64_DST_SHIFT);
   encoded |= ((src2 & PVR_ROGUE_PDSINST_REGS32_MASK)
               << PVR_ROGUE_PDSINST_SFTLP64_SRC2_SHIFT);
   encoded |= ((src1 & PVR_ROGUE_PDSINST_REGS64TP_MASK)
               << PVR_ROGUE_PDSINST_SFTLP64_SRC1_SHIFT);
   encoded |= ((src0 & PVR_ROGUE_PDSINST_REGS64TP_MASK)
               << PVR_ROGUE_PDSINST_SFTLP64_SRC0_SHIFT);
   encoded |= ((im & 1U) << PVR_ROGUE_PDSINST_SFTLP64_IM_SHIFT);
   encoded |= ((lop & PVR_ROGUE_PDSINST_LOP_MASK)
               << PVR_ROGUE_PDSINST_SFTLP64_LOP_SHIFT);
   encoded |= ((cc & 1U) << PVR_ROGUE_PDSINST_SFTLP64_CC_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t
pvr_pds_inst_decode_field_range_regs32t(uint32_t value)
{
   if (value <= PVR_ROGUE_PDSINST_REGS32T_TEMP32_UPPER)
      return PVR_ROGUE_PDSINST_REGS32T_TEMP32;

   return 1;
}

static ALWAYS_INLINE uint32_t
pvr_pds_inst_decode_field_range_regs32tp(uint32_t value)
{
   if (value <= PVR_ROGUE_PDSINST_REGS32TP_TEMP32_UPPER)
      return PVR_ROGUE_PDSINST_REGS32TP_TEMP32;

   if ((value >= PVR_ROGUE_PDSINST_REGS32TP_PTEMP32_LOWER) &&
       (value <= PVR_ROGUE_PDSINST_REGS32TP_PTEMP32_UPPER)) {
      return PVR_ROGUE_PDSINST_REGS32TP_PTEMP32;
   }
   return 2;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_sftlp32(uint32_t im,
                                                          uint32_t cc,
                                                          uint32_t lop,
                                                          uint32_t src0,
                                                          uint32_t src1,
                                                          uint32_t src2,
                                                          uint32_t dst)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEB_SFTLP32
              << PVR_ROGUE_PDSINST_SFTLP32_OPCODE_SHIFT;
   encoded |= ((dst & PVR_ROGUE_PDSINST_REGS32T_MASK)
               << PVR_ROGUE_PDSINST_SFTLP32_DST_SHIFT);
   encoded |= ((src2 & PVR_ROGUE_PDSINST_REGS32TP_MASK)
               << PVR_ROGUE_PDSINST_SFTLP32_SRC2_SHIFT);
   encoded |= ((src1 & PVR_ROGUE_PDSINST_REGS32_MASK)
               << PVR_ROGUE_PDSINST_SFTLP32_SRC1_SHIFT);
   encoded |= ((src0 & PVR_ROGUE_PDSINST_REGS32T_MASK)
               << PVR_ROGUE_PDSINST_SFTLP32_SRC0_SHIFT);
   encoded |= ((lop & PVR_ROGUE_PDSINST_LOP_MASK)
               << PVR_ROGUE_PDSINST_SFTLP32_LOP_SHIFT);
   encoded |= ((cc & 1U) << PVR_ROGUE_PDSINST_SFTLP32_CC_SHIFT);
   encoded |= ((im & 1U) << PVR_ROGUE_PDSINST_SFTLP32_IM_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_stm(uint32_t CCS_CCS_GLOBAL,
                                                      uint32_t CCS_CCS_SO,
                                                      uint32_t CCS_CCS_CC,
                                                      uint32_t SO_TST,
                                                      uint32_t SO,
                                                      uint32_t SO_SRC0,
                                                      uint32_t SO_SRC1,
                                                      uint32_t SO_SRC2,
                                                      uint32_t SO_SRC3)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEB_STM
              << PVR_ROGUE_PDSINST_STM_OPCODE_SHIFT;
   encoded |= ((SO_SRC3 & PVR_ROGUE_PDSINST_REGS64TP_MASK)
               << PVR_ROGUE_PDSINST_STM_SO_SRC3_SHIFT);
   encoded |= ((SO_SRC2 & PVR_ROGUE_PDSINST_REGS32_MASK)
               << PVR_ROGUE_PDSINST_STM_SO_SRC2_SHIFT);
   encoded |= ((SO_SRC1 & PVR_ROGUE_PDSINST_REGS64TP_MASK)
               << PVR_ROGUE_PDSINST_STM_SO_SRC1_SHIFT);
   encoded |= ((SO_SRC0 & PVR_ROGUE_PDSINST_REGS64TP_MASK)
               << PVR_ROGUE_PDSINST_STM_SO_SRC0_SHIFT);
   encoded |=
      ((SO & PVR_ROGUE_PDSINST_SO_MASK) << PVR_ROGUE_PDSINST_STM_SO_SHIFT);
   encoded |= ((SO_TST & 1U) << PVR_ROGUE_PDSINST_STM_SO_TST_SHIFT);
   encoded |= ((CCS_CCS_CC & 1U) << PVR_ROGUE_PDSINST_STM_CCS_CCS_CC_SHIFT);
   encoded |= ((CCS_CCS_SO & 1U) << PVR_ROGUE_PDSINST_STM_CCS_CCS_SO_SHIFT);
   encoded |=
      ((CCS_CCS_GLOBAL & 1U) << PVR_ROGUE_PDSINST_STM_CCS_CCS_GLOBAL_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t
pvr_pds_inst_decode_field_range_regs64(uint32_t value)
{
   if (value <= PVR_ROGUE_PDSINST_REGS64_CONST64_UPPER)
      return PVR_ROGUE_PDSINST_REGS64_CONST64;

   if ((value >= PVR_ROGUE_PDSINST_REGS64_TEMP64_LOWER) &&
       (value <= PVR_ROGUE_PDSINST_REGS64_TEMP64_UPPER)) {
      return PVR_ROGUE_PDSINST_REGS64_TEMP64;
   }
   if ((value >= PVR_ROGUE_PDSINST_REGS64_PTEMP64_LOWER) &&
       (value <= PVR_ROGUE_PDSINST_REGS64_PTEMP64_UPPER)) {
      return PVR_ROGUE_PDSINST_REGS64_PTEMP64;
   }
   return 3;
}

static ALWAYS_INLINE uint32_t pvr_rogue_inst_encode_mad(uint32_t sna,
                                                        uint32_t alum,
                                                        uint32_t cc,
                                                        uint32_t src0,
                                                        uint32_t src1,
                                                        uint32_t src2,
                                                        uint32_t dst)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEA_MAD
              << PVR_ROGUE_PDSINST_MAD_OPCODE_SHIFT;
   encoded |= ((dst & PVR_ROGUE_PDSINST_REGS64T_MASK)
               << PVR_ROGUE_PDSINST_MAD_DST_SHIFT);
   encoded |= ((src2 & PVR_ROGUE_PDSINST_REGS64_MASK)
               << PVR_ROGUE_PDSINST_MAD_SRC2_SHIFT);
   encoded |= ((src1 & PVR_ROGUE_PDSINST_REGS32_MASK)
               << PVR_ROGUE_PDSINST_MAD_SRC1_SHIFT);
   encoded |= ((src0 & PVR_ROGUE_PDSINST_REGS32_MASK)
               << PVR_ROGUE_PDSINST_MAD_SRC0_SHIFT);
   encoded |= ((cc & 1U) << PVR_ROGUE_PDSINST_MAD_CC_SHIFT);
   encoded |= ((alum & 1U) << PVR_ROGUE_PDSINST_MAD_ALUM_SHIFT);
   encoded |= ((sna & 1U) << PVR_ROGUE_PDSINST_MAD_SNA_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_add64(uint32_t cc,
                                                        uint32_t alum,
                                                        uint32_t sna,
                                                        uint32_t src0,
                                                        uint32_t src1,
                                                        uint32_t dst)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEC_ADD64
              << PVR_ROGUE_PDSINST_ADD64_OPCODE_SHIFT;
   encoded |= ((dst & PVR_ROGUE_PDSINST_REGS64TP_MASK)
               << PVR_ROGUE_PDSINST_ADD64_DST_SHIFT);
   encoded |= ((src1 & PVR_ROGUE_PDSINST_REGS64_MASK)
               << PVR_ROGUE_PDSINST_ADD64_SRC1_SHIFT);
   encoded |= ((src0 & PVR_ROGUE_PDSINST_REGS64_MASK)
               << PVR_ROGUE_PDSINST_ADD64_SRC0_SHIFT);
   encoded |= ((sna & 1U) << PVR_ROGUE_PDSINST_ADD64_SNA_SHIFT);
   encoded |= ((alum & 1U) << PVR_ROGUE_PDSINST_ADD64_ALUM_SHIFT);
   encoded |= ((cc & 1U) << PVR_ROGUE_PDSINST_ADD64_CC_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_add32(uint32_t cc,
                                                        uint32_t alum,
                                                        uint32_t sna,
                                                        uint32_t src0,
                                                        uint32_t src1,
                                                        uint32_t dst)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEC_ADD32
              << PVR_ROGUE_PDSINST_ADD32_OPCODE_SHIFT;
   encoded |= ((dst & PVR_ROGUE_PDSINST_REGS32TP_MASK)
               << PVR_ROGUE_PDSINST_ADD32_DST_SHIFT);
   encoded |= ((src1 & PVR_ROGUE_PDSINST_REGS32_MASK)
               << PVR_ROGUE_PDSINST_ADD32_SRC1_SHIFT);
   encoded |= ((src0 & PVR_ROGUE_PDSINST_REGS32_MASK)
               << PVR_ROGUE_PDSINST_ADD32_SRC0_SHIFT);
   encoded |= ((sna & 1U) << PVR_ROGUE_PDSINST_ADD32_SNA_SHIFT);
   encoded |= ((alum & 1U) << PVR_ROGUE_PDSINST_ADD32_ALUM_SHIFT);
   encoded |= ((cc & 1U) << PVR_ROGUE_PDSINST_ADD32_CC_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_cmp(uint32_t cc,
                                                      uint32_t cop,
                                                      uint32_t src0,
                                                      uint32_t src1)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEC_CMP
              << PVR_ROGUE_PDSINST_CMP_OPCODE_SHIFT;
   encoded |= ((src1 & PVR_ROGUE_PDSINST_REGS64_MASK)
               << PVR_ROGUE_PDSINST_CMP_SRC1_SHIFT);
   encoded |= ((src0 & PVR_ROGUE_PDSINST_REGS64TP_MASK)
               << PVR_ROGUE_PDSINST_CMP_SRC0_SHIFT);
   encoded |= UINT32_C(0x0) << PVR_ROGUE_PDSINST_CMP_IM_SHIFT;
   encoded |= UINT32_C(0x1) << PVR_ROGUE_PDSINST_CMP_SETCP_SHIFT;
   encoded |=
      ((cop & PVR_ROGUE_PDSINST_COP_MASK) << PVR_ROGUE_PDSINST_CMP_COP_SHIFT);
   encoded |= ((cc & 1U) << PVR_ROGUE_PDSINST_CMP_CC_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_cmpi(uint32_t cc,
                                                       uint32_t cop,
                                                       uint32_t src0,
                                                       uint32_t im16)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEC_CMP
              << PVR_ROGUE_PDSINST_CMPI_OPCODE_SHIFT;
   encoded |= ((im16 & PVR_ROGUE_PDSINST_IMM16_MASK)
               << PVR_ROGUE_PDSINST_CMPI_IM16_SHIFT);
   encoded |= ((src0 & PVR_ROGUE_PDSINST_REGS64TP_MASK)
               << PVR_ROGUE_PDSINST_CMPI_SRC0_SHIFT);
   encoded |= UINT32_C(0x1) << PVR_ROGUE_PDSINST_CMPI_IM_SHIFT;
   encoded |= UINT32_C(0x1) << PVR_ROGUE_PDSINST_CMPI_SETCP_SHIFT;
   encoded |=
      ((cop & PVR_ROGUE_PDSINST_COP_MASK) << PVR_ROGUE_PDSINST_CMPI_COP_SHIFT);
   encoded |= ((cc & 1U) << PVR_ROGUE_PDSINST_CMPI_CC_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_bra(uint32_t srcc,
                                                      uint32_t neg,
                                                      uint32_t setc,
                                                      uint32_t addr)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEC_BRA
              << PVR_ROGUE_PDSINST_BRA_OPCODE_SHIFT;
   encoded |= ((addr & PVR_ROGUE_PDSINST_BRAADDR_MASK)
               << PVR_ROGUE_PDSINST_BRA_ADDR_SHIFT);
   encoded |= ((setc & PVR_ROGUE_PDSINST_PREDICATE_MASK)
               << PVR_ROGUE_PDSINST_BRA_SETC_SHIFT);
   encoded |= ((neg & 1U) << PVR_ROGUE_PDSINST_BRA_NEG_SHIFT);
   encoded |= ((srcc & PVR_ROGUE_PDSINST_PREDICATE_MASK)
               << PVR_ROGUE_PDSINST_BRA_SRCC_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_ld(uint32_t cc, uint32_t src0)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEC_SP << PVR_ROGUE_PDSINST_LD_OPCODE_SHIFT;
   encoded |= ((src0 & PVR_ROGUE_PDSINST_REGS64_MASK)
               << PVR_ROGUE_PDSINST_LD_SRC0_SHIFT);
   encoded |= PVR_ROGUE_PDSINST_OPCODESP_LD << PVR_ROGUE_PDSINST_LD_OP_SHIFT;
   encoded |= ((cc & 1U) << PVR_ROGUE_PDSINST_LD_CC_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_st(uint32_t cc, uint32_t src0)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEC_SP << PVR_ROGUE_PDSINST_ST_OPCODE_SHIFT;
   encoded |= ((src0 & PVR_ROGUE_PDSINST_REGS64_MASK)
               << PVR_ROGUE_PDSINST_ST_SRC0_SHIFT);
   encoded |= PVR_ROGUE_PDSINST_OPCODESP_ST << PVR_ROGUE_PDSINST_ST_OP_SHIFT;
   encoded |= ((cc & 1U) << PVR_ROGUE_PDSINST_ST_CC_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_wdf(uint32_t cc)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEC_SP
              << PVR_ROGUE_PDSINST_WDF_OPCODE_SHIFT;
   encoded |= PVR_ROGUE_PDSINST_OPCODESP_WDF << PVR_ROGUE_PDSINST_WDF_OP_SHIFT;
   encoded |= ((cc & 1U) << PVR_ROGUE_PDSINST_WDF_CC_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_limm(uint32_t cc,
                                                       uint32_t src1,
                                                       uint32_t src0,
                                                       uint32_t gr)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEC_SP
              << PVR_ROGUE_PDSINST_LIMM_OPCODE_SHIFT;
   encoded |= ((gr & 1U) << PVR_ROGUE_PDSINST_LIMM_GR_SHIFT);
   encoded |= ((src0 & PVR_ROGUE_PDSINST_IMM16_MASK)
               << PVR_ROGUE_PDSINST_LIMM_SRC0_SHIFT);
   encoded |= ((src1 & PVR_ROGUE_PDSINST_REGS32T_MASK)
               << PVR_ROGUE_PDSINST_LIMM_SRC1_SHIFT);
   encoded |= PVR_ROGUE_PDSINST_OPCODESP_LIMM
              << PVR_ROGUE_PDSINST_LIMM_OP_SHIFT;
   encoded |= ((cc & 1U) << PVR_ROGUE_PDSINST_LIMM_CC_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_lock(uint32_t cc)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEC_SP
              << PVR_ROGUE_PDSINST_LOCK_OPCODE_SHIFT;
   encoded |= PVR_ROGUE_PDSINST_OPCODESP_LOCK
              << PVR_ROGUE_PDSINST_LOCK_OP_SHIFT;
   encoded |= ((cc & 1U) << PVR_ROGUE_PDSINST_LOCK_CC_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_release(uint32_t cc)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEC_SP
              << PVR_ROGUE_PDSINST_RELEASE_OPCODE_SHIFT;
   encoded |= PVR_ROGUE_PDSINST_OPCODESP_RELEASE
              << PVR_ROGUE_PDSINST_RELEASE_OP_SHIFT;
   encoded |= ((cc & 1U) << PVR_ROGUE_PDSINST_RELEASE_CC_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_halt(uint32_t cc)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEC_SP
              << PVR_ROGUE_PDSINST_HALT_OPCODE_SHIFT;
   encoded |= PVR_ROGUE_PDSINST_OPCODESP_HALT
              << PVR_ROGUE_PDSINST_HALT_OP_SHIFT;
   encoded |= ((cc & 1U) << PVR_ROGUE_PDSINST_HALT_CC_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_stmc(uint32_t cc,
                                                       uint32_t so_mask)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEC_SP
              << PVR_ROGUE_PDSINST_STMC_OPCODE_SHIFT;
   encoded |= ((so_mask & PVR_ROGUE_PDSINST_SOMASK_MASK)
               << PVR_ROGUE_PDSINST_STMC_SOMASK_SHIFT);
   encoded |= PVR_ROGUE_PDSINST_OPCODESP_STMC
              << PVR_ROGUE_PDSINST_STMC_OP_SHIFT;
   encoded |= ((cc & 1U) << PVR_ROGUE_PDSINST_STMC_CC_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t
pvr_rogue_pds_inst_decode_field_range_regs64c(uint32_t value)
{
   if (value <= PVR_ROGUE_PDSINST_REGS64C_CONST64_UPPER)
      return PVR_ROGUE_PDSINST_REGS64C_CONST64;

   return 1;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_ddmad(uint32_t cc,
                                                        uint32_t end,
                                                        uint32_t src0,
                                                        uint32_t src1,
                                                        uint32_t src2,
                                                        uint32_t src3)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEC_DDMAD
              << PVR_ROGUE_PDSINST_DDMAD_OPCODE_SHIFT;
   encoded |= ((src3 & PVR_ROGUE_PDSINST_REGS64C_MASK)
               << PVR_ROGUE_PDSINST_DDMAD_SRC3_SHIFT);
   encoded |= ((src2 & PVR_ROGUE_PDSINST_REGS64_MASK)
               << PVR_ROGUE_PDSINST_DDMAD_SRC2_SHIFT);
   encoded |= ((src1 & PVR_ROGUE_PDSINST_REGS32T_MASK)
               << PVR_ROGUE_PDSINST_DDMAD_SRC1_SHIFT);
   encoded |= ((src0 & PVR_ROGUE_PDSINST_REGS32_MASK)
               << PVR_ROGUE_PDSINST_DDMAD_SRC0_SHIFT);
   encoded |= ((end & 1U) << PVR_ROGUE_PDSINST_DDMAD_END_SHIFT);
   encoded |= ((cc & 1U) << PVR_ROGUE_PDSINST_DDMAD_CC_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

static ALWAYS_INLINE uint32_t pvr_pds_inst_encode_dout(uint32_t cc,
                                                       uint32_t end,
                                                       uint32_t src1,
                                                       uint32_t src0,
                                                       uint32_t dst)
{
   uint32_t encoded = 0;

   encoded |= PVR_ROGUE_PDSINST_OPCODEC_DOUT
              << PVR_ROGUE_PDSINST_DOUT_OPCODE_SHIFT;
   encoded |= ((dst & PVR_ROGUE_PDSINST_DSTDOUT_MASK)
               << PVR_ROGUE_PDSINST_DOUT_DST_SHIFT);
   encoded |= ((src0 & PVR_ROGUE_PDSINST_REGS64_MASK)
               << PVR_ROGUE_PDSINST_DOUT_SRC0_SHIFT);
   encoded |= ((src1 & PVR_ROGUE_PDSINST_REGS32_MASK)
               << PVR_ROGUE_PDSINST_DOUT_SRC1_SHIFT);
   encoded |= ((end & 1U) << PVR_ROGUE_PDSINST_DOUT_END_SHIFT);
   encoded |= ((cc & 1U) << PVR_ROGUE_PDSINST_DOUT_CC_SHIFT);

   PVR_PDS_PRINT_INST(encoded);

   return encoded;
}

#endif /* PVR_ROGUE_PDS_ENCODE_H */
