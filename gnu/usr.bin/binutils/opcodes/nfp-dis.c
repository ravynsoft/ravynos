/* Print NFP instructions for objdump.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   Contributed by Francois H. Theron <francois.theron@netronome.com>

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/* There will be many magic numbers here that are based on hardware.
   Making #define macros for each encoded bit field will probably reduce
   readability far more than the simple numbers will, so we make sure that
   the context of the magic numbers make it clear what they are used for.  */

#include "sysdep.h"
#include <stdio.h>
#include "disassemble.h"
#include "libiberty.h"
#include "elf/nfp.h"
#include "opcode/nfp.h"
#include "opintl.h"
#include "elf-bfd.h"
#include "bfd.h"
#include <stdint.h>

#define _NFP_ERR_STOP -1
#define _NFP_ERR_CONT -8

#define _BTST(v, b)               (((v) >> b) & 1)
#define _BF(v, msb, lsb)          (((v) >> (lsb)) & \
				   ((1U << ((msb) - (lsb) + 1)) - 1))
#define _BFS(v, msb, lsb, lshift) (_BF(v, msb, lsb) << (lshift))

#define _NFP_ME27_28_CSR_CTX_ENABLES     0x18
#define _NFP_ME27_28_CSR_MISC_CONTROL    0x160

#define _NFP_ISLAND_MAX 64
#define _NFP_ME_MAX     12

typedef struct
{
  unsigned char ctx4_mode:1;
  unsigned char addr_3rdparty32:1;
  unsigned char scs_cnt:2;
  unsigned char _future:4;
}
nfp_priv_mecfg;

typedef struct
{
  unsigned char show_pc;
  unsigned char ctx_mode;
}
nfp_opts;

/* mecfgs[island][menum][is-text] */
typedef struct
{
  nfp_priv_mecfg mecfgs[_NFP_ISLAND_MAX][_NFP_ME_MAX][2];
}
nfp_priv_data;

static const char *nfp_mealu_shf_op[8] =
{
  /* 0b000 (0) */ "B",
  /* 0b001 (1) */ "~B",
  /* 0b010 (2) */ "AND",
  /* 0b011 (3) */ "~AND",
  /* 0b100 (4) */ "AND~",
  /* 0b101 (5) */ "OR",
  /* 0b110 (6) */ "asr",
  /* 0b111 (7) */ "byte_align"
};

static const char *nfp_me27_28_alu_op[32] =
{
  /* 0b00000 (0) */ "B",
  /* 0b00001 (1) */ "+",
  NULL,
  /* 0b00011 (3) */ "pop_count3",
  /* 0b00100 (4) */ "~B",
  /* 0b00101 (5) */ "+16",
  /* 0b00110 (6) */ "pop_count1",
  /* 0b00111 (7) */ "pop_count2",
  /* 0b01000 (8) */ "AND",
  /* 0b01001 (9) */ "+8",
  NULL,
  /* 0b01011 (11) */ "cam_clear",
  /* 0b01100 (12) */ "~AND",
  /* 0b01101 (13) */ "-carry",
  /* 0b01110 (14) */ "ffs",
  /* 0b01111 (15) */ "cam_read_tag",
  /* 0b10000 (16) */ "AND~",
  /* 0b10001 (17) */ "+carry",
  /* 0b10010 (18) */ "CRC",
  /* 0b10011 (19) */ "cam_write",
  /* 0b10100 (20) */ "OR",
  /* 0b10101 (21) */ "-",
  NULL,
  /* 0b10111 (23) */ "cam_lookup",
  /* 0b11000 (24) */ "XOR",
  /* 0b11001 (25) */ "B-A",
  NULL,
  /* 0b11011 (27) */ "cam_write_state",
  NULL,
  NULL,
  NULL,
  /* 0b11111 (31) */ "cam_read_state"
};

static const char *nfp_me27_28_crc_op[8] =
{
  /* 0b000 (0) */ "--",
  NULL,
  /* 0b010 (2) */ "crc_ccitt",
  NULL,
  /* 0b100 (4) */ "crc_32",
  /* 0b101 (5) */ "crc_iscsi",
  /* 0b110 (6) */ "crc_10",
  /* 0b111 (7) */ "crc_5"
};

static const char *nfp_me27_28_crc_bytes[8] =
{
  /* 0b000 (0) */ "bytes_0_3",
  /* 0b001 (1) */ "bytes_1_3",
  /* 0b010 (2) */ "bytes_2_3",
  /* 0b011 (3) */ "byte_3",
  /* 0b100 (4) */ "bytes_0_2",
  /* 0b101 (5) */ "bytes_0_1",
  /* 0b110 (6) */ "byte_0"
};

static const char *nfp_me27_28_mecsrs[] =
{
  /* 0x000 (0) */ "UstorAddr",
  /* 0x004 (1) */ "UstorDataLwr",
  /* 0x008 (2) */ "UstorDataUpr",
  /* 0x00c (3) */ "UstorErrStat",
  /* 0x010 (4) */ "ALUOut",
  /* 0x014 (5) */ "CtxArbCtrl",
  /* 0x018 (6) */ "CtxEnables",
  /* 0x01c (7) */ "CondCodeEn",
  /* 0x020 (8) */ "CSRCtxPtr",
  /* 0x024 (9) */ "PcBreakpoint0",
  /* 0x028 (10) */ "PcBreakpoint1",
  /* 0x02c (11) */ "PcBreakpointStatus",
  /* 0x030 (12) */ "RegErrStatus",
  /* 0x034 (13) */ "LMErrStatus",
  /* 0x038 (14) */ "LMeccErrorMask",
  NULL,
  /* 0x040 (16) */ "IndCtxStatus",
  /* 0x044 (17) */ "ActCtxStatus",
  /* 0x048 (18) */ "IndCtxSglEvt",
  /* 0x04c (19) */ "ActCtxSglEvt",
  /* 0x050 (20) */ "IndCtxWkpEvt",
  /* 0x054 (21) */ "ActCtxWkpEvt",
  /* 0x058 (22) */ "IndCtxFtrCnt",
  /* 0x05c (23) */ "ActCtxFtrCnt",
  /* 0x060 (24) */ "IndLMAddr0",
  /* 0x064 (25) */ "ActLMAddr0",
  /* 0x068 (26) */ "IndLMAddr1",
  /* 0x06c (27) */ "ActLMAddr1",
  /* 0x070 (28) */ "ByteIndex",
  /* 0x074 (29) */ "XferIndex",
  /* 0x078 (30) */ "IndFtrCntSgl",
  /* 0x07c (31) */ "ActFtrCntSgl",
  /* 0x080 (32) */ "NNPut",
  /* 0x084 (33) */ "NNGet",
  NULL,
  NULL,
  /* 0x090 (36) */ "IndLMAddr2",
  /* 0x094 (37) */ "ActLMAddr2",
  /* 0x098 (38) */ "IndLMAddr3",
  /* 0x09c (39) */ "ActLMAddr3",
  /* 0x0a0 (40) */ "IndLMAddr2BytIdx",
  /* 0x0a4 (41) */ "ActLMAddr2BytIdx",
  /* 0x0a8 (42) */ "IndLMAddr3BytIdx",
  /* 0x0ac (43) */ "ActLMAddr3BytIdx",
  /* 0x0b0 (44) */ "IndPredCC",
  NULL,
  NULL,
  NULL,
  /* 0x0c0 (48) */ "TimestampLow",
  /* 0x0c4 (49) */ "TimestampHgh",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  /* 0x0e0 (56) */ "IndLMAddr0BytIdx",
  /* 0x0e4 (57) */ "ActLMAddr0BytIdx",
  /* 0x0e8 (58) */ "IndLMAddr1BytIdx",
  /* 0x0ec (59) */ "ActLMAddr1BytIdx",
  NULL,
  /* 0x0f4 (61) */ "XfrAndBytIdx",
  NULL,
  NULL,
  /* 0x100 (64) */ "NxtNghbrSgl",
  /* 0x104 (65) */ "PrvNghbrSgl",
  /* 0x108 (66) */ "SameMESignal",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  /* 0x140 (80) */ "CRCRemainder",
  /* 0x144 (81) */ "ProfileCnt",
  /* 0x148 (82) */ "PseudoRndNum",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  /* 0x160 (88) */ "MiscControl",
  /* 0x164 (89) */ "PcBreakpoint0Mask",
  /* 0x168 (90) */ "PcBreakpoint1Mask",
  NULL,
  /* 0x170 (92) */ "Mailbox0",
  /* 0x174 (93) */ "Mailbox1",
  /* 0x178 (94) */ "Mailbox2",
  /* 0x17c (95) */ "Mailbox3",
  NULL,
  NULL,
  NULL,
  NULL,
  /* 0x190 (100) */ "CmdIndirectRef0"
};

const char *nfp_me27_28_br_ops[32] =
{
  /* 0b00000 (0) */ "beq",
  /* 0b00001 (1) */ "bne",
  /* 0b00010 (2) */ "bmi",
  /* 0b00011 (3) */ "bpl",
  /* 0b00100 (4) */ "bcs",
  /* 0b00101 (5) */ "bcc",
  /* 0b00110 (6) */ "bvs",
  /* 0b00111 (7) */ "bvc",
  /* 0b01000 (8) */ "bge",
  /* 0b01001 (9) */ "blt",
  /* 0b01010 (10) */ "ble",
  /* 0b01011 (11) */ "bgt",
  /* (12) */ NULL,
  /* (13) */ NULL,
  /* (14) */ NULL,
  /* (15) */ NULL,
  /* 0b10000 (16) */ "br=ctx",
  /* 0b10001 (17) */ "br!=ctx",
  /* 0b10010 (18) */ "br_signal",
  /* 0b10011 (19) */ "br_!signal",
  /* 0b10100 (20) */ "br_inp_state",
  /* 0b10101 (21) */ "br_!inp_state",
  /* 0b10110 (22) */ "br_cls_state",
  /* 0b10111 (23) */ "br_!cls_state",
  /* 0b11000 (24) */ "br",
  /* (25) */ NULL,
  /* (26) */ NULL,
  /* (27) */ NULL,
  /* (28) */ NULL,
  /* (29) */ NULL,
  /* (30) */ NULL,
  /* (31) */ NULL
};

static const char *nfp_me27_br_inpstates[16] =
{
  /* 0 */ "nn_empty",
  /* 1 */ "nn_full",
  /* 2 */ "scr_ring0_status",
  /* 3 */ "scr_ring1_status",
  /* 4 */ "scr_ring2_status",
  /* 5 */ "scr_ring3_status",
  /* 6 */ "scr_ring4_status",
  /* 7 */ "scr_ring5_status",
  /* 8 */ "scr_ring6_status",
  /* 9 */ "scr_ring7_status",
  /* 10 */ "scr_ring8_status",
  /* 11 */ "scr_ring9_status",
  /* 12 */ "scr_ring10_status",
  /* 13 */ "scr_ring11_status",
  /* 14 */ "fci_not_empty",
  /* 15 */ "fci_full"
};

static const char *nfp_me28_br_inpstates[16] =
{
  /* 0 */ "nn_empty",
  /* 1 */ "nn_full",
  /* 2 */ "ctm_ring0_status",
  /* 3 */ "ctm_ring1_status",
  /* 4 */ "ctm_ring2_status",
  /* 5 */ "ctm_ring3_status",
  /* 6 */ "ctm_ring4_status",
  /* 7 */ "ctm_ring5_status",
  /* 8 */ "ctm_ring6_status",
  /* 9 */ "ctm_ring7_status",
  /* 10 */ "ctm_ring8_status",
  /* 11 */ "ctm_ring9_status",
  /* 12 */ "ctm_ring10_status",
  /* 13 */ "ctm_ring11_status",
  /* 14 */ "ctm_ring12_status",
  /* 15 */ "ctm_ring13_status"
};

static const char *nfp_me27_28_mult_steps[8] =
{
  /* 0 */ "step1",
  /* 1 */ "step2",
  /* 2 */ "step3",
  /* 3 */ "step4",
  /* 4 */ "last",
  /* 5 */ "last2",
  NULL,
  NULL
};

static const char *nfp_me27_28_mult_types[4] =
{
  "start",
  "24x8",
  "16x16",
  "32x32"
};

/* The cmd_mnemonics arrays are sorted here in its definition so that we can
   use bsearch () on the first three fields.  There can be multiple matches
   and we assume that bsearch can return any of them, so we manually step
   back to the first one.  */

static const nfp_cmd_mnemonic nfp_me27_mnemonics[] =
{
  {NFP_3200_CPPTGT_MSF0, 0, 0, 0, 0, "read"},
  {NFP_3200_CPPTGT_MSF0, 0, 2, 0, 0, "read64"},
  {NFP_3200_CPPTGT_MSF0, 1, 0, 0, 0, "write"},
  {NFP_3200_CPPTGT_MSF0, 1, 1, 0, 0, "fast_wr"},
  {NFP_3200_CPPTGT_MSF0, 1, 2, 0, 0, "write64"},
  {NFP_3200_CPPTGT_QDR, 0, 0, 0, 0, "read"},
  {NFP_3200_CPPTGT_QDR, 1, 0, 0, 0, "write"},
  {NFP_3200_CPPTGT_QDR, 2, 0, 0, 0, "write_atomic"},
  {NFP_3200_CPPTGT_QDR, 2, 1, 0, 0, "swap"},
  {NFP_3200_CPPTGT_QDR, 3, 0, 0, 0, "set"},
  {NFP_3200_CPPTGT_QDR, 3, 1, 0, 0, "test_and_set"},
  {NFP_3200_CPPTGT_QDR, 4, 0, 0, 0, "clr"},
  {NFP_3200_CPPTGT_QDR, 4, 1, 0, 0, "test_and_clr"},
  {NFP_3200_CPPTGT_QDR, 5, 0, 0, 0, "add"},
  {NFP_3200_CPPTGT_QDR, 5, 1, 0, 0, "test_and_add"},
  {NFP_3200_CPPTGT_QDR, 6, 0, 0, 0, "read_queue"},
  {NFP_3200_CPPTGT_QDR, 6, 1, 0, 0, "read_queue_ring"},
  {NFP_3200_CPPTGT_QDR, 6, 2, 0, 0, "write_queue"},
  {NFP_3200_CPPTGT_QDR, 6, 3, 0, 0, "write_queue_ring"},
  {NFP_3200_CPPTGT_QDR, 7, 0, 0, 0, "incr"},
  {NFP_3200_CPPTGT_QDR, 7, 1, 0, 0, "test_and_incr"},
  {NFP_3200_CPPTGT_QDR, 8, 0, 0, 0, "decr"},
  {NFP_3200_CPPTGT_QDR, 8, 1, 0, 0, "test_and_decr"},
  {NFP_3200_CPPTGT_QDR, 9, 0, 0, 0, "put"},
  {NFP_3200_CPPTGT_QDR, 9, 1, 0, 0, "get"},
  {NFP_3200_CPPTGT_QDR, 9, 2, 0, 0, "put_imm"},
  {NFP_3200_CPPTGT_QDR, 9, 3, 0, 0, "pop"},
  {NFP_3200_CPPTGT_QDR, 10, 0, 0, 0, "journal"},
  {NFP_3200_CPPTGT_QDR, 10, 1, 0, 0, "fast_journal"},
  {NFP_3200_CPPTGT_QDR, 11, 0, 0, 0, "dequeue"},
  {NFP_3200_CPPTGT_QDR, 12, 0, 0, 0, "enqueue"},
  {NFP_3200_CPPTGT_QDR, 12, 1, 0, 0, "enueue_tail"},
  {NFP_3200_CPPTGT_QDR, 12, 2, 0, 0, "nfp_enqueue"},
  {NFP_3200_CPPTGT_QDR, 12, 3, 0, 0, "nfp_enueue_tail"},
  {NFP_3200_CPPTGT_QDR, 13, 0, 0, 0, "csr_wr"},
  {NFP_3200_CPPTGT_QDR, 13, 1, 0, 0, "csr_rd"},
  {NFP_3200_CPPTGT_QDR, 14, 0, 0, 0, "wr_qdesc"},
  {NFP_3200_CPPTGT_QDR, 14, 1, 0, 0, "nfp_wr_qdesc"},
  {NFP_3200_CPPTGT_QDR, 14, 2, 0, 0, "wr_qdesc_count"},
  {NFP_3200_CPPTGT_QDR, 14, 3, 0, 0, "push_qdesc"},
  {NFP_3200_CPPTGT_QDR, 15, 0, 0, 0, "rd_qdesc_other"},
  {NFP_3200_CPPTGT_QDR, 15, 1, 0, 0, "rd_qdesc_tail"},
  {NFP_3200_CPPTGT_QDR, 15, 2, 0, 0, "rd_qdesc_head"},
  {NFP_3200_CPPTGT_QDR, 15, 3, 0, 0, "nfp_rd_qdesc"},
  {NFP_3200_CPPTGT_MSF1, 0, 0, 0, 0, "read"},
  {NFP_3200_CPPTGT_MSF1, 0, 2, 0, 0, "read64"},
  {NFP_3200_CPPTGT_MSF1, 1, 0, 0, 0, "write"},
  {NFP_3200_CPPTGT_MSF1, 1, 1, 0, 0, "fast_wr"},
  {NFP_3200_CPPTGT_MSF1, 1, 2, 0, 0, "write64"},
  {NFP_3200_CPPTGT_HASH, 0, 0, 0, 0, "hash_48"},
  {NFP_3200_CPPTGT_HASH, 0, 1, 0, 0, "hash_64"},
  {NFP_3200_CPPTGT_HASH, 0, 2, 0, 0, "hash_128"},
  {NFP_3200_CPPTGT_MU, 0, 0, 0, 0, "read"},
  {NFP_3200_CPPTGT_MU, 0, 1, 0, 0, "read_le"},
  {NFP_3200_CPPTGT_MU, 0, 2, 0, 0, "read_swap"},
  {NFP_3200_CPPTGT_MU, 0, 3, 0, 0, "read_swap_le"},
  {NFP_3200_CPPTGT_MU, 1, 0, 0, 0, "write"},
  {NFP_3200_CPPTGT_MU, 1, 1, 0, 0, "write_le"},
  {NFP_3200_CPPTGT_MU, 1, 2, 0, 0, "write_swap"},
  {NFP_3200_CPPTGT_MU, 1, 3, 0, 0, "write_swap_le"},
  {NFP_3200_CPPTGT_MU, 2, 0, 0, 0, "write8"},
  {NFP_3200_CPPTGT_MU, 2, 1, 0, 0, "write8_le"},
  {NFP_3200_CPPTGT_MU, 2, 2, 0, 0, "write8_swap"},
  {NFP_3200_CPPTGT_MU, 2, 3, 0, 0, "write8_swap_le"},
  {NFP_3200_CPPTGT_MU, 3, 0, 0, 0, "read_atomic"},
  {NFP_3200_CPPTGT_MU, 3, 1, 0, 0, "read8"},
  {NFP_3200_CPPTGT_MU, 3, 2, 0, 0, "compare_write"},
  {NFP_3200_CPPTGT_MU, 3, 3, 0, 0, "test_and_compare_write"},
  {NFP_3200_CPPTGT_MU, 4, 0, 0, 0, "write_atomic"},
  {NFP_3200_CPPTGT_MU, 4, 1, 0, 0, "swap"},
  {NFP_3200_CPPTGT_MU, 4, 2, 0, 0, "write_atomic_imm"},
  {NFP_3200_CPPTGT_MU, 4, 3, 0, 0, "swap_imm"},
  {NFP_3200_CPPTGT_MU, 5, 0, 0, 0, "set"},
  {NFP_3200_CPPTGT_MU, 5, 1, 0, 0, "test_and_set"},
  {NFP_3200_CPPTGT_MU, 5, 2, 0, 0, "set_imm"},
  {NFP_3200_CPPTGT_MU, 5, 3, 0, 0, "test_and_set_imm"},
  {NFP_3200_CPPTGT_MU, 6, 0, 0, 0, "clr"},
  {NFP_3200_CPPTGT_MU, 6, 1, 0, 0, "test_and_clr"},
  {NFP_3200_CPPTGT_MU, 6, 2, 0, 0, "clr_imm"},
  {NFP_3200_CPPTGT_MU, 6, 3, 0, 0, "test_and_clr_imm"},
  {NFP_3200_CPPTGT_MU, 7, 0, 0, 4, "add"},
  {NFP_3200_CPPTGT_MU, 7, 0, 4, 4, "add64"},
  {NFP_3200_CPPTGT_MU, 7, 1, 0, 4, "test_and_add"},
  {NFP_3200_CPPTGT_MU, 7, 1, 4, 4, "test_and_add64"},
  {NFP_3200_CPPTGT_MU, 7, 2, 0, 4, "add_imm"},
  {NFP_3200_CPPTGT_MU, 7, 2, 4, 4, "add64_imm"},
  {NFP_3200_CPPTGT_MU, 7, 3, 0, 4, "test_and_add_imm"},
  {NFP_3200_CPPTGT_MU, 7, 3, 4, 4, "test_and_add64_imm"},
  {NFP_3200_CPPTGT_MU, 8, 0, 0, 4, "add_sat"},
  {NFP_3200_CPPTGT_MU, 8, 0, 4, 4, "add64_sat"},
  {NFP_3200_CPPTGT_MU, 8, 1, 0, 4, "test_and_add_sat"},
  {NFP_3200_CPPTGT_MU, 8, 1, 4, 4, "test_and_add64_sat"},
  {NFP_3200_CPPTGT_MU, 8, 2, 0, 4, "add_imm_sat"},
  {NFP_3200_CPPTGT_MU, 8, 2, 4, 4, "add_imm_sat"},
  {NFP_3200_CPPTGT_MU, 8, 3, 0, 0, "test_and_add_sat_imm"},
  {NFP_3200_CPPTGT_MU, 9, 0, 0, 4, "sub"},
  {NFP_3200_CPPTGT_MU, 9, 0, 4, 4, "sub64"},
  {NFP_3200_CPPTGT_MU, 9, 1, 0, 4, "test_and_sub"},
  {NFP_3200_CPPTGT_MU, 9, 1, 4, 4, "test_and_sub64"},
  {NFP_3200_CPPTGT_MU, 9, 2, 0, 4, "sub_imm"},
  {NFP_3200_CPPTGT_MU, 9, 2, 4, 4, "sub64_imm"},
  {NFP_3200_CPPTGT_MU, 9, 3, 0, 0, "tes_and_sub_imm"},
  {NFP_3200_CPPTGT_MU, 10, 0, 0, 4, "sub_sat"},
  {NFP_3200_CPPTGT_MU, 10, 0, 4, 4, "sub64_sat"},
  {NFP_3200_CPPTGT_MU, 10, 1, 0, 4, "test_and_sub_sat"},
  {NFP_3200_CPPTGT_MU, 10, 1, 4, 4, "test_and_sub64_sat"},
  {NFP_3200_CPPTGT_MU, 10, 2, 0, 4, "sub_imm_sat"},
  {NFP_3200_CPPTGT_MU, 10, 2, 4, 4, "sub64_imm_sat"},
  {NFP_3200_CPPTGT_MU, 10, 3, 0, 0, "test_and_sub_sat_imm"},
  {NFP_3200_CPPTGT_MU, 11, 0, 0, 0, "release_ticket"},
  {NFP_3200_CPPTGT_MU, 11, 1, 0, 0, "release_ticket_ind"},
  {NFP_3200_CPPTGT_MU, 12, 0, 0, 0, "cam_lookup"},
  {NFP_3200_CPPTGT_MU, 12, 1, 0, 0, "cam_lookup_add"},
  {NFP_3200_CPPTGT_MU, 12, 2, 0, 0, "tcam_lookup"},
  {NFP_3200_CPPTGT_MU, 12, 3, 0, 3, "lock"},
  {NFP_3200_CPPTGT_MU, 12, 3, 2, 3, "cam_lookup_add_inc"},
  {NFP_3200_CPPTGT_MU, 13, 0, 0, 4, "microq128_get"},
  {NFP_3200_CPPTGT_MU, 13, 0, 4, 4, "microq256_get"},
  {NFP_3200_CPPTGT_MU, 13, 1, 0, 4, "microq128_pop"},
  {NFP_3200_CPPTGT_MU, 13, 1, 4, 4, "microq256_pop"},
  {NFP_3200_CPPTGT_MU, 13, 2, 0, 4, "microq128_put"},
  {NFP_3200_CPPTGT_MU, 13, 2, 4, 4, "microq256_put"},
  {NFP_3200_CPPTGT_MU, 14, 0, 0, 4, "queue128_lock"},
  {NFP_3200_CPPTGT_MU, 14, 0, 4, 4, "queue256_lock"},
  {NFP_3200_CPPTGT_MU, 14, 1, 0, 4, "queue128_unlock"},
  {NFP_3200_CPPTGT_MU, 14, 1, 4, 4, "queue256_unlock"},
  {NFP_3200_CPPTGT_MU, 15, 0, 0, 0, "xor"},
  {NFP_3200_CPPTGT_MU, 15, 1, 0, 0, "test_and_xor"},
  {NFP_3200_CPPTGT_MU, 15, 2, 0, 0, "xor_imm"},
  {NFP_3200_CPPTGT_MU, 15, 3, 0, 0, "test_and_xor_imm"},
  {NFP_3200_CPPTGT_MU, 16, 0, 0, 0, "rd_qdesc"},
  {NFP_3200_CPPTGT_MU, 16, 1, 0, 0, "wr_qdesc"},
  {NFP_3200_CPPTGT_MU, 16, 2, 0, 0, "push_qdesc"},
  {NFP_3200_CPPTGT_MU, 16, 3, 0, 0, "tag_writeback"},
  {NFP_3200_CPPTGT_MU, 17, 0, 0, 0, "enqueue"},
  {NFP_3200_CPPTGT_MU, 17, 1, 0, 0, "enqueue_tail"},
  {NFP_3200_CPPTGT_MU, 17, 2, 0, 0, "dequeue"},
  {NFP_3200_CPPTGT_MU, 18, 0, 0, 0, "read_queue"},
  {NFP_3200_CPPTGT_MU, 18, 1, 0, 0, "read_queue_ring"},
  {NFP_3200_CPPTGT_MU, 18, 2, 0, 0, "write_queue"},
  {NFP_3200_CPPTGT_MU, 18, 3, 0, 0, "write_queue_ring"},
  {NFP_3200_CPPTGT_MU, 19, 0, 0, 0, "add_tail"},
  {NFP_3200_CPPTGT_MU, 19, 1, 0, 0, "qadd_thread"},
  {NFP_3200_CPPTGT_MU, 19, 2, 0, 0, "qadd_work"},
  {NFP_3200_CPPTGT_MU, 19, 3, 0, 0, "qadd_work_imm"},
  {NFP_3200_CPPTGT_MU, 20, 0, 0, 0, "put"},
  {NFP_3200_CPPTGT_MU, 20, 1, 0, 0, "put_tag"},
  {NFP_3200_CPPTGT_MU, 20, 2, 0, 0, "journal"},
  {NFP_3200_CPPTGT_MU, 20, 3, 0, 0, "journal_tag"},
  {NFP_3200_CPPTGT_MU, 21, 0, 0, 0, "get"},
  {NFP_3200_CPPTGT_MU, 21, 1, 0, 0, "get_eop"},
  {NFP_3200_CPPTGT_MU, 21, 2, 0, 0, "get_safe"},
  {NFP_3200_CPPTGT_MU, 21, 3, 0, 0, "get_tag_safe"},
  {NFP_3200_CPPTGT_MU, 22, 0, 0, 0, "pop"},
  {NFP_3200_CPPTGT_MU, 22, 1, 0, 0, "pop_eop"},
  {NFP_3200_CPPTGT_MU, 22, 2, 0, 0, "pop_safe"},
  {NFP_3200_CPPTGT_MU, 22, 3, 0, 0, "pop_tag_safe"},
  {NFP_3200_CPPTGT_MU, 23, 0, 0, 0, "fast_journal"},
  {NFP_3200_CPPTGT_MU, 23, 1, 0, 0, "fast_journal_sig"},
  {NFP_3200_CPPTGT_GS, 0, 0, 0, 0, "read"},
  {NFP_3200_CPPTGT_GS, 1, 0, 0, 0, "write"},
  {NFP_3200_CPPTGT_GS, 2, 0, 0, 0, "write_atomic"},
  {NFP_3200_CPPTGT_GS, 2, 1, 0, 0, "swap"},
  {NFP_3200_CPPTGT_GS, 3, 0, 0, 0, "set"},
  {NFP_3200_CPPTGT_GS, 3, 1, 0, 0, "test_and_set"},
  {NFP_3200_CPPTGT_GS, 4, 0, 0, 0, "clr"},
  {NFP_3200_CPPTGT_GS, 4, 1, 0, 0, "test_and_clr"},
  {NFP_3200_CPPTGT_GS, 5, 0, 0, 0, "add"},
  {NFP_3200_CPPTGT_GS, 5, 1, 0, 0, "test_and_add"},
  {NFP_3200_CPPTGT_GS, 6, 0, 0, 0, "sub"},
  {NFP_3200_CPPTGT_GS, 6, 1, 0, 0, "test_and_sub"},
  {NFP_3200_CPPTGT_GS, 7, 0, 0, 0, "inc"},
  {NFP_3200_CPPTGT_GS, 7, 1, 0, 0, "test_and_inc"},
  {NFP_3200_CPPTGT_GS, 8, 0, 0, 0, "dec"},
  {NFP_3200_CPPTGT_GS, 8, 1, 0, 0, "test_and_dec"},
  {NFP_3200_CPPTGT_GS, 9, 0, 0, 0, "get"},
  {NFP_3200_CPPTGT_GS, 10, 0, 0, 0, "put"},
  {NFP_3200_CPPTGT_PCIE, 0, 0, 0, 0, "read"},
  {NFP_3200_CPPTGT_PCIE, 1, 0, 0, 0, "write"},
  {NFP_3200_CPPTGT_PCIE, 2, 0, 0, 0, "read_internal"},
  {NFP_3200_CPPTGT_PCIE, 3, 0, 0, 0, "write_internal"},
  {NFP_3200_CPPTGT_ARM, 0, 0, 0, 0, "read"},
  {NFP_3200_CPPTGT_ARM, 1, 0, 0, 0, "write"},
  {NFP_3200_CPPTGT_CRYPTO, 0, 0, 0, 0, "read"},
  {NFP_3200_CPPTGT_CRYPTO, 1, 0, 0, 0, "write"},
  {NFP_3200_CPPTGT_CRYPTO, 2, 0, 0, 0, "write_fifo"},
  {NFP_3200_CPPTGT_CAP, 0, 0, 0, 0, "read_enum"},
  {NFP_3200_CPPTGT_CAP, 0, 1, 0, 0, "read"},
  {NFP_3200_CPPTGT_CAP, 0, 2, 0, 0, "read_reflect"},
  {NFP_3200_CPPTGT_CAP, 1, 0, 0, 0, "write_enum"},
  {NFP_3200_CPPTGT_CAP, 1, 1, 0, 0, "write"},
  {NFP_3200_CPPTGT_CAP, 1, 2, 0, 0, "write_reflect"},
  {NFP_3200_CPPTGT_CAP, 2, 0, 0, 0, "fast_wr_alu"},
  {NFP_3200_CPPTGT_CAP, 3, 0, 0, 0, "fast_wr"},
  {NFP_3200_CPPTGT_CT, 1, 0, 0, 0, "write"},
  {NFP_3200_CPPTGT_CLS, 0, 0, 0, 0, "read_be"},
  {NFP_3200_CPPTGT_CLS, 0, 1, 0, 0, "read_le"},
  {NFP_3200_CPPTGT_CLS, 0, 2, 0, 0, "test_and_compare_write"},
  {NFP_3200_CPPTGT_CLS, 0, 3, 0, 0, "xor"},
  {NFP_3200_CPPTGT_CLS, 1, 0, 0, 0, "write_be"},
  {NFP_3200_CPPTGT_CLS, 1, 1, 0, 0, "write_le"},
  {NFP_3200_CPPTGT_CLS, 1, 2, 0, 0, "write8_be"},
  {NFP_3200_CPPTGT_CLS, 1, 3, 0, 0, "write8_le"},
  {NFP_3200_CPPTGT_CLS, 2, 0, 0, 0, "set"},
  {NFP_3200_CPPTGT_CLS, 2, 1, 0, 0, "clr"},
  {NFP_3200_CPPTGT_CLS, 2, 2, 0, 0, "test_and_set"},
  {NFP_3200_CPPTGT_CLS, 2, 3, 0, 0, "test_and_clr"},
  {NFP_3200_CPPTGT_CLS, 3, 0, 0, 0, "set_imm"},
  {NFP_3200_CPPTGT_CLS, 3, 1, 0, 0, "clr_imm"},
  {NFP_3200_CPPTGT_CLS, 3, 2, 0, 0, "test_and_set_imm"},
  {NFP_3200_CPPTGT_CLS, 3, 3, 0, 0, "test_and_clr_imm"},
  {NFP_3200_CPPTGT_CLS, 4, 0, 0, 0, "add"},
  {NFP_3200_CPPTGT_CLS, 4, 1, 0, 0, "add64"},
  {NFP_3200_CPPTGT_CLS, 4, 2, 0, 0, "add_sat"},
  {NFP_3200_CPPTGT_CLS, 4, 3, 0, 0, "test_and_add_sat"},
  {NFP_3200_CPPTGT_CLS, 5, 0, 0, 0, "add_imm"},
  {NFP_3200_CPPTGT_CLS, 5, 1, 0, 0, "add64_imm"},
  {NFP_3200_CPPTGT_CLS, 5, 2, 0, 0, "add_imm_sat"},
  {NFP_3200_CPPTGT_CLS, 5, 3, 0, 0, "test_and_add_imm_sat"},
  {NFP_3200_CPPTGT_CLS, 6, 0, 0, 0, "sub"},
  {NFP_3200_CPPTGT_CLS, 6, 1, 0, 0, "sub64"},
  {NFP_3200_CPPTGT_CLS, 6, 2, 0, 0, "sub_sat"},
  {NFP_3200_CPPTGT_CLS, 6, 3, 0, 0, "test_and_sub_sat"},
  {NFP_3200_CPPTGT_CLS, 7, 0, 0, 0, "sub_imm"},
  {NFP_3200_CPPTGT_CLS, 7, 1, 0, 0, "sub64_imm"},
  {NFP_3200_CPPTGT_CLS, 7, 2, 0, 0, "sub_imm_sat"},
  {NFP_3200_CPPTGT_CLS, 7, 3, 0, 0, "test_and_sub_imm_sat"},
  {NFP_3200_CPPTGT_CLS, 8, 0, 0, 0, "queue_lock"},
  {NFP_3200_CPPTGT_CLS, 8, 1, 0, 0, "queue_unlock"},
  {NFP_3200_CPPTGT_CLS, 8, 2, 0, 0, "hash_mask"},
  {NFP_3200_CPPTGT_CLS, 8, 3, 0, 0, "hash_mask_clear"},
  {NFP_3200_CPPTGT_CLS, 9, 0, 0, 0, "get"},
  {NFP_3200_CPPTGT_CLS, 9, 1, 0, 0, "pop"},
  {NFP_3200_CPPTGT_CLS, 9, 2, 0, 0, "get_safe"},
  {NFP_3200_CPPTGT_CLS, 9, 3, 0, 0, "pop_safe"},
  {NFP_3200_CPPTGT_CLS, 10, 0, 0, 0, "put"},
  {NFP_3200_CPPTGT_CLS, 10, 1, 0, 0, "put_offset"},
  {NFP_3200_CPPTGT_CLS, 10, 2, 0, 0, "journal"},
  {NFP_3200_CPPTGT_CLS, 10, 3, 0, 0, "add_tail"},
  {NFP_3200_CPPTGT_CLS, 11, 0, 0, 0, "cam_lookup32"},
  {NFP_3200_CPPTGT_CLS, 11, 1, 0, 0, "cam_lookup32_add"},
  {NFP_3200_CPPTGT_CLS, 11, 2, 0, 0, "cam_lookup24"},
  {NFP_3200_CPPTGT_CLS, 11, 3, 0, 0, "cam_lookup24_add"},
  {NFP_3200_CPPTGT_CLS, 12, 0, 0, 0, "cam_lookup8"},
  {NFP_3200_CPPTGT_CLS, 12, 1, 0, 0, "cam_lookup8_add"},
  {NFP_3200_CPPTGT_CLS, 12, 2, 0, 0, "cam_lookup16"},
  {NFP_3200_CPPTGT_CLS, 12, 3, 0, 0, "cam_lookup16_add"},
  {NFP_3200_CPPTGT_CLS, 13, 0, 0, 0, "tcam_lookup32"},
  {NFP_3200_CPPTGT_CLS, 13, 1, 0, 0, "tcam_lookup24"},
  {NFP_3200_CPPTGT_CLS, 13, 2, 0, 0, "tcam_lookup16"},
  {NFP_3200_CPPTGT_CLS, 13, 3, 0, 0, "tcam_lookup8"},
  {NFP_3200_CPPTGT_CLS, 14, 0, 0, 0, "reflect_from_sig_src"},
  {NFP_3200_CPPTGT_CLS, 14, 1, 0, 0, "reflect_from_sig_dst"},
  {NFP_3200_CPPTGT_CLS, 14, 2, 0, 0, "reflect_from_sig_both"},
  {NFP_3200_CPPTGT_CLS, 15, 0, 0, 0, "reflect_to_sig_src"},
  {NFP_3200_CPPTGT_CLS, 15, 1, 0, 0, "reflect_to_sig_dst"},
  {NFP_3200_CPPTGT_CLS, 15, 2, 0, 0, "reflect_to_sig_both"}
};

static const nfp_cmd_mnemonic nfp_me28_mnemonics[] =
{
  {NFP_6000_CPPTGT_NBI, 0, 0, 0, 0, "read"},
  {NFP_6000_CPPTGT_NBI, 1, 0, 0, 0, "write"},
  {NFP_6000_CPPTGT_NBI, 3, 0, 0, 0, "packet_ready_drop"},
  {NFP_6000_CPPTGT_NBI, 3, 1, 0, 0, "packet_ready_unicast"},
  {NFP_6000_CPPTGT_NBI, 3, 2, 0, 0, "packet_ready_multicast_dont_free"},
  {NFP_6000_CPPTGT_NBI, 3, 3, 0, 0, "packet_ready_multicast_free_on_last"},
  {NFP_6000_CPPTGT_ILA, 0, 0, 0, 0, "read"},
  {NFP_6000_CPPTGT_ILA, 0, 1, 0, 0, "read_check_error"},
  {NFP_6000_CPPTGT_ILA, 1, 0, 0, 0, "write"},
  {NFP_6000_CPPTGT_ILA, 1, 1, 0, 0, "write_check_error"},
  {NFP_6000_CPPTGT_ILA, 2, 0, 0, 0, "read_int"},
  {NFP_6000_CPPTGT_ILA, 3, 0, 0, 7, "write_int"},
  {NFP_6000_CPPTGT_ILA, 3, 0, 3, 7, "write_dma"},
  {NFP_6000_CPPTGT_MU, 0, 0, 0, 0, "read"},
  {NFP_6000_CPPTGT_MU, 0, 1, 0, 0, "read_le"},
  {NFP_6000_CPPTGT_MU, 0, 2, 0, 0, "read_swap"},
  {NFP_6000_CPPTGT_MU, 0, 3, 0, 0, "read_swap_le"},
  {NFP_6000_CPPTGT_MU, 1, 0, 0, 0, "write"},
  {NFP_6000_CPPTGT_MU, 1, 1, 0, 0, "write_le"},
  {NFP_6000_CPPTGT_MU, 1, 2, 0, 0, "write_swap"},
  {NFP_6000_CPPTGT_MU, 1, 3, 0, 0, "write_swap_le"},
  {NFP_6000_CPPTGT_MU, 2, 0, 0, 0, "write8"},
  {NFP_6000_CPPTGT_MU, 2, 1, 0, 0, "write8_le"},
  {NFP_6000_CPPTGT_MU, 2, 2, 0, 0, "write8_swap"},
  {NFP_6000_CPPTGT_MU, 2, 3, 0, 0, "write8_swap_le"},
  {NFP_6000_CPPTGT_MU, 3, 0, 0, 0, "atomic_read"},
  {NFP_6000_CPPTGT_MU, 3, 1, 0, 0, "read8"},
  {NFP_6000_CPPTGT_MU, 3, 2, 0, 0,
   "compare_write_or_incr/mask_compare_write"},
  {NFP_6000_CPPTGT_MU, 3, 3, 0, 0,
   "test_compare_write_or_incr/test_mask_compare_write"},
  {NFP_6000_CPPTGT_MU, 4, 0, 0, 0, "atomic_write"},
  {NFP_6000_CPPTGT_MU, 4, 1, 0, 0, "swap"},
  {NFP_6000_CPPTGT_MU, 4, 2, 0, 0, "atomic_write_imm"},
  {NFP_6000_CPPTGT_MU, 4, 3, 0, 0, "swap_imm"},
  {NFP_6000_CPPTGT_MU, 5, 0, 0, 0, "set"},
  {NFP_6000_CPPTGT_MU, 5, 1, 0, 0, "test_set"},
  {NFP_6000_CPPTGT_MU, 5, 2, 0, 0, "set_imm"},
  {NFP_6000_CPPTGT_MU, 5, 3, 0, 0, "test_set_imm"},
  {NFP_6000_CPPTGT_MU, 6, 0, 0, 0, "clr"},
  {NFP_6000_CPPTGT_MU, 6, 1, 0, 0, "test_clr"},
  {NFP_6000_CPPTGT_MU, 6, 2, 0, 0, "clr_imm"},
  {NFP_6000_CPPTGT_MU, 6, 3, 0, 0, "test_clr_imm"},
  {NFP_6000_CPPTGT_MU, 7, 0, 0, 4, "add"},
  {NFP_6000_CPPTGT_MU, 7, 0, 4, 4, "add64"},
  {NFP_6000_CPPTGT_MU, 7, 1, 0, 4, "test_add"},
  {NFP_6000_CPPTGT_MU, 7, 1, 4, 4, "test_add64"},
  {NFP_6000_CPPTGT_MU, 7, 2, 0, 4, "add_imm"},
  {NFP_6000_CPPTGT_MU, 7, 2, 4, 4, "add64_imm"},
  {NFP_6000_CPPTGT_MU, 7, 3, 0, 4, "test_add_imm"},
  {NFP_6000_CPPTGT_MU, 7, 3, 4, 4, "test_add64_imm"},
  {NFP_6000_CPPTGT_MU, 8, 0, 0, 4, "addsat"},
  {NFP_6000_CPPTGT_MU, 8, 0, 4, 4, "addsat64"},
  {NFP_6000_CPPTGT_MU, 8, 1, 0, 4, "test_addsat"},
  {NFP_6000_CPPTGT_MU, 8, 1, 4, 4, "test_addsat64"},
  {NFP_6000_CPPTGT_MU, 8, 2, 0, 4, "addsat_imm"},
  {NFP_6000_CPPTGT_MU, 8, 2, 4, 4, "addsat64_imm"},
  {NFP_6000_CPPTGT_MU, 8, 3, 0, 4, "test_addsat_imm"},
  {NFP_6000_CPPTGT_MU, 8, 3, 4, 4, "test_addsat64_imm"},
  {NFP_6000_CPPTGT_MU, 9, 0, 0, 4, "sub"},
  {NFP_6000_CPPTGT_MU, 9, 0, 4, 4, "sub64"},
  {NFP_6000_CPPTGT_MU, 9, 1, 0, 4, "test_sub"},
  {NFP_6000_CPPTGT_MU, 9, 1, 4, 4, "test_sub64"},
  {NFP_6000_CPPTGT_MU, 9, 2, 0, 4, "sub_imm"},
  {NFP_6000_CPPTGT_MU, 9, 2, 4, 4, "sub64_imm"},
  {NFP_6000_CPPTGT_MU, 9, 3, 0, 4, "test_sub_imm"},
  {NFP_6000_CPPTGT_MU, 9, 3, 4, 4, "test_sub64_imm"},
  {NFP_6000_CPPTGT_MU, 10, 0, 0, 4, "subsat"},
  {NFP_6000_CPPTGT_MU, 10, 0, 4, 4, "subsat64"},
  {NFP_6000_CPPTGT_MU, 10, 1, 0, 4, "test_subsat"},
  {NFP_6000_CPPTGT_MU, 10, 1, 4, 4, "test_subsat64"},
  {NFP_6000_CPPTGT_MU, 10, 2, 0, 4, "subsat_imm"},
  {NFP_6000_CPPTGT_MU, 10, 2, 4, 4, "subsat64_imm"},
  {NFP_6000_CPPTGT_MU, 10, 3, 0, 4, "test_subsat_imm"},
  {NFP_6000_CPPTGT_MU, 10, 3, 4, 4, "test_subsat64_imm"},
  {NFP_6000_CPPTGT_MU, 11, 0, 0, 0, "ticket_release"},
  {NFP_6000_CPPTGT_MU, 11, 1, 0, 0, "ticket_release_ind"},
  {NFP_6000_CPPTGT_MU, 12, 0, 0, 7, "cam128_lookup8/cam384_lookup8"},
  {NFP_6000_CPPTGT_MU, 12, 0, 1, 7, "cam128_lookup16/cam384_lookup16"},
  {NFP_6000_CPPTGT_MU, 12, 0, 2, 7, "cam128_lookup24/cam384_lookup24"},
  {NFP_6000_CPPTGT_MU, 12, 0, 3, 7, "cam128_lookup32/cam384_lookup32"},
  {NFP_6000_CPPTGT_MU, 12, 0, 4, 7, "cam256_lookup8/cam512_lookup8"},
  {NFP_6000_CPPTGT_MU, 12, 0, 5, 7, "cam256_lookup16/cam512_lookup16"},
  {NFP_6000_CPPTGT_MU, 12, 0, 6, 7, "cam256_lookup24/cam512_lookup24"},
  {NFP_6000_CPPTGT_MU, 12, 0, 7, 7, "cam256_lookup32/cam512_lookup32"},
  {NFP_6000_CPPTGT_MU, 12, 1, 0, 7,
   "cam128_lookup8_add/cam384_lookup8_add"},
  {NFP_6000_CPPTGT_MU, 12, 1, 1, 7,
   "cam128_lookup16_add/cam384_lookup16_add"},
  {NFP_6000_CPPTGT_MU, 12, 1, 2, 7,
   "cam128_lookup24_add/cam384_lookup24_add"},
  {NFP_6000_CPPTGT_MU, 12, 1, 3, 7,
   "cam128_lookup32_add/cam384_lookup32_add"},
  {NFP_6000_CPPTGT_MU, 12, 1, 4, 7,
   "cam256_lookup8_add/cam512_lookup8_add"},
  {NFP_6000_CPPTGT_MU, 12, 1, 5, 7,
   "cam256_lookup16_add/cam512_lookup16_add"},
  {NFP_6000_CPPTGT_MU, 12, 1, 6, 7,
   "cam256_lookup24_add/cam512_lookup24_add"},
  {NFP_6000_CPPTGT_MU, 12, 1, 7, 7,
   "cam256_lookup32_add/cam512_lookup32_add"},
  {NFP_6000_CPPTGT_MU, 12, 2, 0, 7, "tcam128_lookup8/tcam384_lookup8"},
  {NFP_6000_CPPTGT_MU, 12, 2, 1, 7, "tcam128_lookup16/tcam384_lookup16"},
  {NFP_6000_CPPTGT_MU, 12, 2, 2, 7, "tcam128_lookup24/tcam384_lookup24"},
  {NFP_6000_CPPTGT_MU, 12, 2, 3, 7, "tcam128_lookup32/tcam384_lookup32"},
  {NFP_6000_CPPTGT_MU, 12, 2, 4, 7, "tcam256_lookup8/tcam512_lookup8"},
  {NFP_6000_CPPTGT_MU, 12, 2, 5, 7, "tcam256_lookup16/tcam512_lookup16"},
  {NFP_6000_CPPTGT_MU, 12, 2, 6, 7, "tcam256_lookup24/tcam512_lookup24"},
  {NFP_6000_CPPTGT_MU, 12, 2, 7, 7, "tcam256_lookup32/tcam512_lookup32"},
  {NFP_6000_CPPTGT_MU, 12, 3, 0, 7, "lock128/lock384"},
  {NFP_6000_CPPTGT_MU, 12, 3, 2, 7,
   "cam128_lookup24_add_inc/cam384_lookup24_add_inc"},
  {NFP_6000_CPPTGT_MU, 12, 3, 4, 7, "lock256/lock512"},
  {NFP_6000_CPPTGT_MU, 12, 3, 6, 7,
   "cam256_lookup24_add_inc/cam512_lookup24_add_inc"},
  {NFP_6000_CPPTGT_MU, 13, 0, 0, 7, "microq128_get"},
  {NFP_6000_CPPTGT_MU, 13, 0, 4, 7, "microq256_get"},
  {NFP_6000_CPPTGT_MU, 13, 1, 0, 7, "microq128_pop"},
  {NFP_6000_CPPTGT_MU, 13, 1, 4, 7, "microq256_pop"},
  {NFP_6000_CPPTGT_MU, 13, 2, 0, 7, "microq128_put"},
  {NFP_6000_CPPTGT_MU, 13, 2, 4, 7, "microq256_put"},
  {NFP_6000_CPPTGT_MU, 14, 0, 0, 7, "queue128_lock"},
  {NFP_6000_CPPTGT_MU, 14, 0, 4, 7, "queue256_lock"},
  {NFP_6000_CPPTGT_MU, 14, 1, 0, 7, "queue128_unlock"},
  {NFP_6000_CPPTGT_MU, 14, 1, 4, 7, "queue256_unlock"},
  {NFP_6000_CPPTGT_MU, 15, 0, 0, 0, "xor"},
  {NFP_6000_CPPTGT_MU, 15, 1, 0, 0, "test_xor"},
  {NFP_6000_CPPTGT_MU, 15, 2, 0, 0, "xor_imm"},
  {NFP_6000_CPPTGT_MU, 15, 3, 0, 0, "test_xor_imm"},
  {NFP_6000_CPPTGT_MU, 16, 0, 0, 0,
   "ctm.packet_wait_packet_status/emem.rd_qdesc/imem.stats_log"},
  {NFP_6000_CPPTGT_MU, 16, 1, 0, 0,
   "ctm.packet_read_packet_status/emem.wr_qdesc/imem.stats_log_sat"},
  {NFP_6000_CPPTGT_MU, 16, 2, 0, 0,
   "emem.push_qdesc/imem.stats_log_event"},
  {NFP_6000_CPPTGT_MU, 16, 3, 0, 0, "imem.stats_log_sat_event"},
  {NFP_6000_CPPTGT_MU, 17, 0, 0, 0,
   "ctm.packet_alloc/emem.enqueue/imem.stats_push"},
  {NFP_6000_CPPTGT_MU, 17, 1, 0, 0,
   "ctm.packet_credit_get/emem.enqueue_tail/imem.stats_push_clear"},
  {NFP_6000_CPPTGT_MU, 17, 2, 0, 0, "ctm.packet_alloc_poll/emem.dequeue"},
  {NFP_6000_CPPTGT_MU, 17, 3, 0, 0, "ctm.packet_add_thread"},
  {NFP_6000_CPPTGT_MU, 18, 0, 0, 0,
   "ctm.packet_free/emem.read_queue/imem.lb_write_desc"},
  {NFP_6000_CPPTGT_MU, 18, 1, 0, 0,
   "ctm.packet_free_and_signal/emem.read_queue_ring/imem.lb_read_desc"},
  {NFP_6000_CPPTGT_MU, 18, 2, 0, 0,
   "ctm.packet_free_and_return_pointer/emem.write_queue"},
  {NFP_6000_CPPTGT_MU, 18, 3, 0, 0,
   "ctm.packet_return_pointer/emem.write_queue_ring"},
  {NFP_6000_CPPTGT_MU, 19, 0, 0, 0,
   "ctm.packet_complete_drop/emem.add_tail/imem.lb_write_idtable"},
  {NFP_6000_CPPTGT_MU, 19, 1, 0, 0,
   "ctm.packet_complete_unicast/emem.qadd_thread/imem.lb_read_idtable"},
  {NFP_6000_CPPTGT_MU, 19, 2, 0, 0,
   "ctm.packet_complete_multicast/emem.qadd_work"},
  {NFP_6000_CPPTGT_MU, 19, 3, 0, 0,
   "ctm.packet_complete_multicast_free/emem.qadd_work_imm"},
  {NFP_6000_CPPTGT_MU, 20, 0, 0, 0,
   "ctm.pe_dma_to_memory_packet/emem.put/imem.lb_bucket_write_local"},
  {NFP_6000_CPPTGT_MU, 20, 1, 0, 0,
   "ctm.pe_dma_to_memory_packet_swap/imem.lb_bucket_write_dcache"},
  {NFP_6000_CPPTGT_MU, 20, 2, 0, 0,
   "ctm.pe_dma_to_memory_packet_free/emem.journal"},
  {NFP_6000_CPPTGT_MU, 20, 3, 0, 0,
   "ctm.pe_dma_to_memory_packet_free_swap"},
  {NFP_6000_CPPTGT_MU, 21, 0, 0, 0,
   "ctm.pe_dma_to_memory_indirect/emem.get/imem.lb_bucket_read_local"},
  {NFP_6000_CPPTGT_MU, 21, 1, 0, 0,
   "ctm.pe_dma_to_memory_indirect_swap/emem.get_eop/"
     "imem.lb_bucket_read_dcache"},
  {NFP_6000_CPPTGT_MU, 21, 2, 0, 0,
   "ctm.pe_dma_to_memory_indirect_free/emem.get_freely"},
  {NFP_6000_CPPTGT_MU, 21, 3, 0, 0,
   "ctm.pe_dma_to_memory_indirect_free_swap"},
  {NFP_6000_CPPTGT_MU, 22, 0, 0, 0,
   "ctm.pe_dma_to_memory_buffer/emem.pop/imem.lb_lookup_bundleid"},
  {NFP_6000_CPPTGT_MU, 22, 1, 0, 0,
   "ctm.pe_dma_to_memory_buffer_le/emem.pop_eop/imem.lb_lookup_dcache"},
  {NFP_6000_CPPTGT_MU, 22, 2, 0, 0,
   "ctm.pe_dma_to_memory_buffer_swap/emem.pop_freely/imem.lb_lookup_idtable"},
  {NFP_6000_CPPTGT_MU, 22, 3, 0, 0, "ctm.pe_dma_to_memory_buffer_le_swap"},
  {NFP_6000_CPPTGT_MU, 23, 0, 0, 0,
   "ctm.pe_dma_from_memory_buffer/emem.fast_journal/imem.lb_push_stats_local"},
  {NFP_6000_CPPTGT_MU, 23, 1, 0, 0,
   "ctm.pe_dma_from_memory_buffer_le/emem.fast_journal_sig/"
     "imem.lb_push_stats_dcache"},
  {NFP_6000_CPPTGT_MU, 23, 2, 0, 0,
   "ctm.pe_dma_from_memory_buffer_swap/imem.lb_push_stats_local_clr"},
  {NFP_6000_CPPTGT_MU, 23, 3, 0, 0,
   "ctm.pe_dma_from_memory_buffer_le_swap/imem.lb_push_stats_dcache_clr"},
  {NFP_6000_CPPTGT_MU, 26, 0, 0, 0, "emem.lookup/imem.lookup"},
  {NFP_6000_CPPTGT_MU, 28, 0, 0, 0, "read32"},
  {NFP_6000_CPPTGT_MU, 28, 1, 0, 0, "read32_le"},
  {NFP_6000_CPPTGT_MU, 28, 2, 0, 0, "read32_swap"},
  {NFP_6000_CPPTGT_MU, 28, 3, 0, 0, "read32_swap_le"},
  {NFP_6000_CPPTGT_MU, 29, 1, 0, 0, "cam_lookup_add_lock"},
  {NFP_6000_CPPTGT_MU, 29, 2, 0, 0, "cam_lookup_add_extend"},
  {NFP_6000_CPPTGT_MU, 29, 3, 0, 0, "cam_lookup_add_inc"},
  {NFP_6000_CPPTGT_MU, 30, 2, 0, 0, "meter"},
  {NFP_6000_CPPTGT_MU, 31, 0, 0, 0, "write32"},
  {NFP_6000_CPPTGT_MU, 31, 1, 0, 0, "write32_le"},
  {NFP_6000_CPPTGT_MU, 31, 2, 0, 0, "write32_swap"},
  {NFP_6000_CPPTGT_MU, 31, 3, 0, 0, "write32_swap_le"},
  {NFP_6000_CPPTGT_PCIE, 0, 0, 0, 0, "read"},
  {NFP_6000_CPPTGT_PCIE, 0, 1, 0, 0, "read_rid"},
  {NFP_6000_CPPTGT_PCIE, 1, 0, 0, 0, "write"},
  {NFP_6000_CPPTGT_PCIE, 1, 1, 0, 0, "write_rid"},
  {NFP_6000_CPPTGT_PCIE, 1, 2, 0, 0, "write_vdm"},
  {NFP_6000_CPPTGT_PCIE, 2, 0, 0, 0, "read_int"},
  {NFP_6000_CPPTGT_PCIE, 3, 0, 0, 0, "write_int"},
  {NFP_6000_CPPTGT_ARM, 0, 0, 0, 0, "read"},
  {NFP_6000_CPPTGT_ARM, 1, 0, 0, 0, "write"},
  {NFP_6000_CPPTGT_CRYPTO, 0, 0, 0, 0, "read"},
  {NFP_6000_CPPTGT_CRYPTO, 1, 0, 0, 0, "write"},
  {NFP_6000_CPPTGT_CRYPTO, 2, 0, 0, 0, "write_fifo"},
  {NFP_6000_CPPTGT_CTXPB, 0, 0, 0, 0, "xpb_read"},
  {NFP_6000_CPPTGT_CTXPB, 0, 1, 0, 0, "ring_get"},
  {NFP_6000_CPPTGT_CTXPB, 0, 2, 0, 0, "interthread_signal"},
  {NFP_6000_CPPTGT_CTXPB, 1, 0, 0, 0, "xpb_write"},
  {NFP_6000_CPPTGT_CTXPB, 1, 1, 0, 0, "ring_put"},
  {NFP_6000_CPPTGT_CTXPB, 1, 2, 0, 0, "ctnn_write"},
  {NFP_6000_CPPTGT_CTXPB, 2, 0, 0, 0, "reflect_read_none"},
  {NFP_6000_CPPTGT_CTXPB, 2, 1, 0, 0, "reflect_read_sig_init"},
  {NFP_6000_CPPTGT_CTXPB, 2, 2, 0, 0, "reflect_read_sig_remote"},
  {NFP_6000_CPPTGT_CTXPB, 2, 3, 0, 0, "reflect_read_sig_both"},
  {NFP_6000_CPPTGT_CTXPB, 3, 0, 0, 0, "reflect_write_none"},
  {NFP_6000_CPPTGT_CTXPB, 3, 1, 0, 0, "reflect_write_sig_init"},
  {NFP_6000_CPPTGT_CTXPB, 3, 2, 0, 0, "reflect_write_sig_remote"},
  {NFP_6000_CPPTGT_CTXPB, 3, 3, 0, 0, "reflect_write_sig_both"},
  {NFP_6000_CPPTGT_CLS, 0, 0, 0, 0, "read"},
  {NFP_6000_CPPTGT_CLS, 0, 1, 0, 0, "read_le"},
  {NFP_6000_CPPTGT_CLS, 0, 2, 0, 0, "swap/test_compare_write"},
  {NFP_6000_CPPTGT_CLS, 0, 3, 0, 0, "xor"},
  {NFP_6000_CPPTGT_CLS, 1, 0, 0, 0, "write"},
  {NFP_6000_CPPTGT_CLS, 1, 1, 0, 0, "write_le"},
  {NFP_6000_CPPTGT_CLS, 1, 2, 0, 0, "write8_be"},
  {NFP_6000_CPPTGT_CLS, 1, 3, 0, 0, "write8_le"},
  {NFP_6000_CPPTGT_CLS, 2, 0, 0, 0, "set"},
  {NFP_6000_CPPTGT_CLS, 2, 1, 0, 0, "clr"},
  {NFP_6000_CPPTGT_CLS, 2, 2, 0, 0, "test_set"},
  {NFP_6000_CPPTGT_CLS, 2, 3, 0, 0, "test_clr"},
  {NFP_6000_CPPTGT_CLS, 3, 0, 0, 0, "set_imm"},
  {NFP_6000_CPPTGT_CLS, 3, 1, 0, 0, "clr_imm"},
  {NFP_6000_CPPTGT_CLS, 3, 2, 0, 0, "test_set_imm"},
  {NFP_6000_CPPTGT_CLS, 3, 3, 0, 0, "test_clr_imm"},
  {NFP_6000_CPPTGT_CLS, 4, 0, 0, 0, "add"},
  {NFP_6000_CPPTGT_CLS, 4, 1, 0, 0, "add64"},
  {NFP_6000_CPPTGT_CLS, 4, 2, 0, 0, "addsat"},
  {NFP_6000_CPPTGT_CLS, 5, 0, 0, 0, "add_imm"},
  {NFP_6000_CPPTGT_CLS, 5, 1, 0, 0, "add64_imm"},
  {NFP_6000_CPPTGT_CLS, 5, 2, 0, 0, "addsat_imm"},
  {NFP_6000_CPPTGT_CLS, 6, 0, 0, 0, "sub"},
  {NFP_6000_CPPTGT_CLS, 6, 1, 0, 0, "sub64"},
  {NFP_6000_CPPTGT_CLS, 6, 2, 0, 0, "subsat"},
  {NFP_6000_CPPTGT_CLS, 7, 0, 0, 0, "sub_imm"},
  {NFP_6000_CPPTGT_CLS, 7, 1, 0, 0, "sub64_imm"},
  {NFP_6000_CPPTGT_CLS, 7, 2, 0, 0, "subsat_imm"},
  {NFP_6000_CPPTGT_CLS, 8, 0, 0, 0, "queue_lock"},
  {NFP_6000_CPPTGT_CLS, 8, 1, 0, 0, "queue_unlock"},
  {NFP_6000_CPPTGT_CLS, 8, 2, 0, 0, "hash_mask"},
  {NFP_6000_CPPTGT_CLS, 8, 3, 0, 0, "hash_mask_clear"},
  {NFP_6000_CPPTGT_CLS, 9, 0, 0, 0, "get"},
  {NFP_6000_CPPTGT_CLS, 9, 1, 0, 0, "pop"},
  {NFP_6000_CPPTGT_CLS, 9, 2, 0, 0, "get_safe"},
  {NFP_6000_CPPTGT_CLS, 9, 3, 0, 0, "pop_safe"},
  {NFP_6000_CPPTGT_CLS, 10, 0, 0, 0, "ring_put"},
  {NFP_6000_CPPTGT_CLS, 10, 2, 0, 0, "ring_journal"},
  {NFP_6000_CPPTGT_CLS, 11, 0, 0, 0, "cam_lookup32"},
  {NFP_6000_CPPTGT_CLS, 11, 1, 0, 0, "cam_lookup32_add"},
  {NFP_6000_CPPTGT_CLS, 11, 2, 0, 0, "cam_lookup24"},
  {NFP_6000_CPPTGT_CLS, 11, 3, 0, 0, "cam_lookup24_add"},
  {NFP_6000_CPPTGT_CLS, 12, 0, 0, 0, "cam_lookup8"},
  {NFP_6000_CPPTGT_CLS, 12, 1, 0, 0, "cam_lookup8_add"},
  {NFP_6000_CPPTGT_CLS, 12, 2, 0, 0, "cam_lookup16"},
  {NFP_6000_CPPTGT_CLS, 12, 3, 0, 0, "cam_lookup16_add"},
  {NFP_6000_CPPTGT_CLS, 13, 0, 0, 0, "tcam_lookup32"},
  {NFP_6000_CPPTGT_CLS, 13, 1, 0, 0, "tcam_lookup24"},
  {NFP_6000_CPPTGT_CLS, 13, 2, 0, 0, "tcam_lookup16"},
  {NFP_6000_CPPTGT_CLS, 13, 3, 0, 0, "tcam_lookup8"},
  {NFP_6000_CPPTGT_CLS, 14, 0, 0, 0, "reflect_write_sig_local"},
  {NFP_6000_CPPTGT_CLS, 14, 1, 0, 0, "reflect_write_sig_remote"},
  {NFP_6000_CPPTGT_CLS, 14, 2, 0, 0, "reflect_write_sig_both"},
  {NFP_6000_CPPTGT_CLS, 15, 0, 0, 0, "reflect_read_sig_remote"},
  {NFP_6000_CPPTGT_CLS, 15, 1, 0, 0, "reflect_read_sig_local"},
  {NFP_6000_CPPTGT_CLS, 15, 2, 0, 0, "reflect_read_sig_both"},
  {NFP_6000_CPPTGT_CLS, 16, 1, 0, 0, "cam_lookup32_add_lock"},
  {NFP_6000_CPPTGT_CLS, 16, 2, 0, 0, "cam_lookup24_add_inc"},
  {NFP_6000_CPPTGT_CLS, 16, 3, 0, 0, "cam_lookup32_add_extend"},
  {NFP_6000_CPPTGT_CLS, 17, 0, 0, 0, "meter"},
  {NFP_6000_CPPTGT_CLS, 17, 2, 0, 0, "statistic"},
  {NFP_6000_CPPTGT_CLS, 17, 3, 0, 0, "statistic_imm"},
  {NFP_6000_CPPTGT_CLS, 20, 0, 0, 0, "test_add"},
  {NFP_6000_CPPTGT_CLS, 20, 1, 0, 0, "test_add64"},
  {NFP_6000_CPPTGT_CLS, 20, 2, 0, 0, "test_addsat"},
  {NFP_6000_CPPTGT_CLS, 21, 0, 0, 0, "test_add_imm"},
  {NFP_6000_CPPTGT_CLS, 21, 1, 0, 0, "test_add64_imm"},
  {NFP_6000_CPPTGT_CLS, 21, 2, 0, 0, "test_addsat_imm"},
  {NFP_6000_CPPTGT_CLS, 22, 0, 0, 0, "test_sub"},
  {NFP_6000_CPPTGT_CLS, 22, 1, 0, 0, "test_sub64"},
  {NFP_6000_CPPTGT_CLS, 22, 2, 0, 0, "test_subsat"},
  {NFP_6000_CPPTGT_CLS, 23, 0, 0, 0, "test_sub_imm"},
  {NFP_6000_CPPTGT_CLS, 23, 1, 0, 0, "test_sub64_imm"},
  {NFP_6000_CPPTGT_CLS, 23, 2, 0, 0, "test_subsat_imm"},
  {NFP_6000_CPPTGT_CLS, 24, 0, 0, 0, "ring_read"},
  {NFP_6000_CPPTGT_CLS, 24, 1, 0, 0, "ring_write"},
  {NFP_6000_CPPTGT_CLS, 24, 2, 0, 0, "ring_ordered_lock"},
  {NFP_6000_CPPTGT_CLS, 24, 3, 0, 0, "ring_ordered_unlock"},
  {NFP_6000_CPPTGT_CLS, 25, 0, 0, 0, "ring_workq_add_thread"},
  {NFP_6000_CPPTGT_CLS, 25, 1, 0, 0, "ring_workq_add_work"}
};

static int
nfp_me_print_invalid (uint64_t instr, struct disassemble_info *dinfo)
{
  const char * err_msg = N_("<invalid_instruction>:");
  dinfo->fprintf_func (dinfo->stream, "%s 0x%" PRIx64, err_msg, instr);
  return _NFP_ERR_CONT;
}

static bool
nfp_me_is_imm_opnd10 (unsigned int opnd)
{
  return _BF (opnd, 9, 8) == 0x3;
}

static bool
nfp_me_is_imm_opnd8 (unsigned int opnd)
{
  return _BTST (opnd, 5);
}

static unsigned int
nfp_me_imm_opnd10 (unsigned int opnd)
{
  return nfp_me_is_imm_opnd10 (opnd) ? (opnd & 0xff) : ~0U;
}

static unsigned int
nfp_me_imm_opnd8 (unsigned int opnd, unsigned int imm8_msb)
{
  unsigned int v = (imm8_msb << 7) | _BFS (opnd, 7, 6, 5) | _BF (opnd, 4, 0);

  return nfp_me_is_imm_opnd8 (opnd) ? v : ~0U;
}

/* Print an unrestricted/10-bit operand.
   This can mostly be generic across NFP families at the moment.  */
static bool
nfp_me_print_opnd10 (unsigned int opnd, char bank, int num_ctx, int lmem_ext,
		     struct disassemble_info *dinfo)
{
  unsigned int n = _BF (opnd, (num_ctx == 8) ? 3 : 4, 0);

  /* Absolute GPR.  */
  if (_BF (opnd, 9, 7) == 0x1)
    dinfo->fprintf_func (dinfo->stream, "@gpr%c_%d", bank, _BF (opnd, 6, 0));

  /* Relative GPR.  */
  else if (_BF (opnd, 9, 6) == 0x0)
    dinfo->fprintf_func (dinfo->stream, "gpr%c_%d", bank, n);

  /* Indexed Xfer.  */
  else if (_BF (opnd, 9, 7) == 0x2)
    {
      dinfo->fprintf_func (dinfo->stream, "*$index");
      if (_BF (opnd, 2, 1) == 0x1)
	dinfo->fprintf_func (dinfo->stream, "++");
      else if (_BF (opnd, 2, 1) == 0x2)
	dinfo->fprintf_func (dinfo->stream, "--");
    }

  /* Relative Xfer.  */
  else if (_BF (opnd, 9, 7) == 0x3)
    {
      if (_BTST (opnd, 6))
	n += (num_ctx == 8 ? 16 : 32);
      dinfo->fprintf_func (dinfo->stream, "$xfer_%d", n);
    }

  /* Indexed Next Neighbour.  */
  else if (_BF (opnd, 9, 6) == 0x9)
    {
      dinfo->fprintf_func (dinfo->stream, "*n$index");
      if (_BTST (opnd, 1))
	dinfo->fprintf_func (dinfo->stream, "++");
    }

  /* Relative Next Neighbour.  */
  else if (_BF (opnd, 9, 6) == 0xa)
    {
      dinfo->fprintf_func (dinfo->stream, "n$reg_%d", n);
    }

  /* Indexed LMEM.  */
  else if (_BF (opnd, 9, 6) == 0x8)
    {
      n = _BF (opnd, 5, 5) + (lmem_ext * 2);
      dinfo->fprintf_func (dinfo->stream, "*l$index%d", n);
      if (_BTST (opnd, 4))
	dinfo->fprintf_func (dinfo->stream, _BTST (opnd, 0) ? "--" : "++");
      else if (_BF (opnd, 3, 0))
	dinfo->fprintf_func (dinfo->stream, "[%d]", _BF (opnd, 3, 0));
    }

  /* 8-bit Constant value.  */
  else if (_BF (opnd, 9, 8) == 0x3)
    dinfo->fprintf_func (dinfo->stream, "0x%x", _BF (opnd, 7, 0));

  else
    {
      dinfo->fprintf_func (dinfo->stream, "<opnd:0x%x>", opnd);
      return false;
    }

  return true;
}

/* Print a restricted/8-bit operand.
   This can mostly be generic across NFP families at the moment.  */

static bool
nfp_me_print_opnd8 (unsigned int opnd, char bank, int num_ctx, int lmem_ext,
		    unsigned int imm8_msb, struct disassemble_info *dinfo)
{
  unsigned int n = _BF (opnd, (num_ctx == 8) ? 3 : 4, 0);

  /* Relative GPR.  */
  if (_BF (opnd, 7, 5) == 0x0)
    dinfo->fprintf_func (dinfo->stream, "gpr%c_%d", bank, n);

  /* Relative Xfer.  */
  else if (_BF (opnd, 7, 5) == 0x4)
    dinfo->fprintf_func (dinfo->stream, "$xfer_%d", n);

  /* Relative Xfer.  */
  else if (_BF (opnd, 7, 5) == 0x6)
    {
      n += (num_ctx == 8 ? 16 : 32);
      dinfo->fprintf_func (dinfo->stream, "$xfer_%d", n);
    }

  /* Indexed Xfer.  */
  else if ((_BF (opnd, 7, 4) == 0x4) && (!_BTST (opnd, 0)))
    {
      dinfo->fprintf_func (dinfo->stream, "*$index");
      if (_BF (opnd, 2, 1) == 0x1)
	dinfo->fprintf_func (dinfo->stream, "++");
      else if (_BF (opnd, 2, 1) == 0x2)
	dinfo->fprintf_func (dinfo->stream, "--");
    }

  /* Indexed NN.  */
  else if ((_BF (opnd, 7, 4) == 0x4) && (_BTST (opnd, 0)))
    {
      dinfo->fprintf_func (dinfo->stream, "*n$index");
      if (_BTST (opnd, 1))
	dinfo->fprintf_func (dinfo->stream, "++");
    }

  /* Indexed LMEM.  */
  else if (_BF (opnd, 7, 4) == 0x5)
    {
      n = _BF (opnd, 3, 3) + (lmem_ext * 2);
      dinfo->fprintf_func (dinfo->stream, "*l$index%d", n);
      if (_BF (opnd, 2, 0))
	dinfo->fprintf_func (dinfo->stream, "[%d]", _BF (opnd, 2, 0));
    }

  /* 7+1-bit Constant value.  */
  else if (_BTST (opnd, 5))
    {
      n = (imm8_msb << 7) | _BFS (opnd, 7, 6, 5) | _BF (opnd, 4, 0);
      dinfo->fprintf_func (dinfo->stream, "0x%x", n);
    }

  else
    {
      dinfo->fprintf_func (dinfo->stream, "<opnd:0x%x>", opnd);
      return false;
    }

  return true;
}

static int
nfp_me27_28_print_alu_shf (uint64_t instr, unsigned int pred_cc,
			   unsigned int dst_lmext, unsigned int src_lmext,
			   unsigned int gpr_wrboth,
			   int num_ctx, struct disassemble_info *dinfo)
{
  unsigned int op = _BF (instr, 35, 33);
  unsigned int srcA = _BF (instr, 7, 0);
  unsigned int srcB = _BF (instr, 17, 10);
  unsigned int dst = _BF (instr, 27, 20);
  unsigned int sc = _BF (instr, 9, 8);
  unsigned int imm_msb = _BTST (instr, 18);
  unsigned int swap = _BTST (instr, 19);
  unsigned int shift = _BF (instr, 32, 28);
  char dst_bank = 'A' + _BTST (instr, 36);
  unsigned int nocc = _BTST (instr, 40);
  bool err = false;

  if (swap)
    {
      unsigned int tmp = srcA;
      srcA = srcB;
      srcB = tmp;
    }

  /* alu_shf, dbl_shf, asr.  */
  if (op < 7)
    {
      if (sc == 3)
	dinfo->fprintf_func (dinfo->stream, "dbl_shf[");
      else if (op == 6)
	dinfo->fprintf_func (dinfo->stream, "asr[");
      else
	dinfo->fprintf_func (dinfo->stream, "alu_shf[");

      /* dest operand */
      if (nfp_me_is_imm_opnd8 (dst))
	dinfo->fprintf_func (dinfo->stream, "--");
      else
	err = err || !nfp_me_print_opnd8 (dst, dst_bank, num_ctx,
					  dst_lmext, imm_msb, dinfo);

      dinfo->fprintf_func (dinfo->stream, ", ");

      /* A operand.  */
      if (op != 6)
	{
	  if ((op < 2) && (sc != 3))	/* Not dbl_shf.  */
	    dinfo->fprintf_func (dinfo->stream, "--");	/* B or ~B operator.  */
	  else
	    err = err || !nfp_me_print_opnd8 (srcA, (swap) ? 'B' : 'A',
					      num_ctx, src_lmext, imm_msb,
					      dinfo);

	  dinfo->fprintf_func (dinfo->stream, ", ");

	  /* Operator (not for dbl_shf).  */
	  if (sc != 3)
	    {
	      dinfo->fprintf_func (dinfo->stream, "%s, ",
				   nfp_mealu_shf_op[op]);
	    }
	}

      /* B operand.  */
      err = err || !nfp_me_print_opnd8 (srcB, (swap) ? 'A' : 'B',
					num_ctx, src_lmext, imm_msb, dinfo);

      dinfo->fprintf_func (dinfo->stream, ", ");

      /* Shift */
      if (sc == 0)
	dinfo->fprintf_func (dinfo->stream, ">>rot%d", shift);
      else if (sc == 2)
	{
	  if (shift)
	    dinfo->fprintf_func (dinfo->stream, "<<%d", (32 - shift));
	  else
	    dinfo->fprintf_func (dinfo->stream, "<<indirect");
	}
      else
	{
	  if (shift)
	    dinfo->fprintf_func (dinfo->stream, ">>%d", shift);
	  else
	    dinfo->fprintf_func (dinfo->stream, ">>indirect");
	}
    }
  /* Byte Align.  */
  else if (op == 7)
    {
      dinfo->fprintf_func (dinfo->stream, "byte_align_%s[",
			   ((sc == 2) ? "le" : "be"));

      /* Dest operand.  */
      if (nfp_me_is_imm_opnd8 (dst))
	dinfo->fprintf_func (dinfo->stream, "--");
      else
	err = err || !nfp_me_print_opnd8 (dst, dst_bank, num_ctx,
					  dst_lmext, imm_msb, dinfo);

      dinfo->fprintf_func (dinfo->stream, ", ");

      if (sc == 2)
	err = err || !nfp_me_print_opnd8 (srcA, (swap) ? 'B' : 'A', num_ctx,
					  0, imm_msb, dinfo);
      else
	err = err || !nfp_me_print_opnd8 (srcB, (swap) ? 'A' : 'B', num_ctx,
					  0, imm_msb, dinfo);
    }

  dinfo->fprintf_func (dinfo->stream, "]");
  if (nocc)
    dinfo->fprintf_func (dinfo->stream, ", no_cc");
  if (gpr_wrboth)
    dinfo->fprintf_func (dinfo->stream, ", gpr_wrboth");
  if (pred_cc)
    dinfo->fprintf_func (dinfo->stream, ", predicate_cc");

  if (err)
    return _NFP_ERR_CONT;
  return 0;
}

static int
nfp_me27_28_print_alu (uint64_t instr, unsigned int pred_cc,
		       unsigned int dst_lmext, unsigned int src_lmext,
		       unsigned int gpr_wrboth,
		       int num_ctx, struct disassemble_info *dinfo)
{
  unsigned int op = _BF (instr, 35, 31);
  unsigned int srcA = _BF (instr, 9, 0);
  unsigned int srcB = _BF (instr, 19, 10);
  unsigned int dst = _BF (instr, 29, 20);
  unsigned int swap = _BTST (instr, 30);
  char dst_bank = 'A' + _BTST (instr, 36);
  unsigned int nocc = _BTST (instr, 40);
  int do_close_bracket = 1;
  bool err = false;

  if (swap)
    {
      unsigned int tmp = srcA;
      srcA = srcB;
      srcB = tmp;
    }

  switch (op)
    {
    case 3:			/* pop_count3[dst, srcB] */
    case 6:			/* pop_count1[srcB] */
    case 7:			/* pop_count2[srcB] */
    case 14:			/* ffs[dst, srcB] */
    case 15:			/* cam_read_tag[dst, srcB] */
    case 31:			/* cam_read_state[dst, srcB] */
      dinfo->fprintf_func (dinfo->stream, "%s[", nfp_me27_28_alu_op[op]);

      /* No dest for pop_count1/2.  */
      if ((op != 6) && (op != 7))
	{
	  /* dest operand */
	  if (nfp_me_is_imm_opnd10 (dst))
	    dinfo->fprintf_func (dinfo->stream, "--");
	  else
	    err = err || !nfp_me_print_opnd10 (dst, dst_bank, num_ctx,
					       dst_lmext, dinfo);

	  dinfo->fprintf_func (dinfo->stream, ", ");
	}

      /* B operand.  */
      err = err || !nfp_me_print_opnd10 (srcB, (swap) ? 'A' : 'B',
					 num_ctx, src_lmext, dinfo);
      break;
 
      /* cam_clear.  */
    case 11:
      do_close_bracket = 0;
      dinfo->fprintf_func (dinfo->stream, "cam_clear");
      break;

      /* cam_lookup.  */
    case 23:
      do_close_bracket = 0;
      dinfo->fprintf_func (dinfo->stream, "%s[", nfp_me27_28_alu_op[op]);

      /* Dest operand.  */
      if (nfp_me_is_imm_opnd10 (dst))
	dinfo->fprintf_func (dinfo->stream, "--");
      else
	err = err || !nfp_me_print_opnd10 (dst, dst_bank, num_ctx,
					   dst_lmext, dinfo);

      dinfo->fprintf_func (dinfo->stream, ", ");

      /* A operand.  */
      err = err || !nfp_me_print_opnd10 (srcA, (swap) ? 'B' : 'A',
					 num_ctx, src_lmext, dinfo);

      dinfo->fprintf_func (dinfo->stream, "]");

      if (_BF (srcB, 1, 0))
	{
	  unsigned int n = _BTST (srcB, 1);
	  if (_BTST (srcB, 4))	/* Only for MEv28.  */
	    n += 2;
	  dinfo->fprintf_func (dinfo->stream, ", lm_addr%d[%d]", n,
			       _BF (srcB, 3, 2));
	}

      break;

    case 19:      /* cam_write.  */
    case 27:      /* cam_write_state.  */
      dinfo->fprintf_func (dinfo->stream, "%s[", nfp_me27_28_alu_op[op]);
      err = err || !nfp_me_print_opnd10 (srcB, (swap) ? 'A' : 'B',
					 num_ctx, src_lmext, dinfo);
      dinfo->fprintf_func (dinfo->stream, ", ");
      if (op == 19)
	{
	  err = err || !nfp_me_print_opnd10 (srcA, (swap) ? 'B' : 'A',
					     num_ctx, src_lmext, dinfo);
	  dinfo->fprintf_func (dinfo->stream, ", ");
	}
      dinfo->fprintf_func (dinfo->stream, "%d", (dst & 0xf));
      break;

      /* CRC.  */
    case 18:	
      do_close_bracket = 0;
      dinfo->fprintf_func (dinfo->stream, "crc_%s[",
			   _BTST (srcA, 3) ? "le" : "be");
      if (!nfp_me27_28_crc_op[_BF (srcA, 7, 5)])
	{
	  dinfo->fprintf_func (dinfo->stream, _(", <invalid CRC operator>, "));
	  err = true;
	}
      else
	{
	  dinfo->fprintf_func (dinfo->stream, "%s, ",
			       nfp_me27_28_crc_op[_BF (srcA, 7, 5)]);
	}

      /* Dest operand.  */
      if (nfp_me_is_imm_opnd10 (dst))
	dinfo->fprintf_func (dinfo->stream, "--");
      else
	err = err || !nfp_me_print_opnd10 (dst, dst_bank, num_ctx,
					   dst_lmext, dinfo);

      dinfo->fprintf_func (dinfo->stream, ", ");

      /* B operand.  */
      err = err || !nfp_me_print_opnd10 (srcB, (swap) ? 'A' : 'B',
					 num_ctx, src_lmext, dinfo);

      dinfo->fprintf_func (dinfo->stream, "]");
      if (_BF (srcA, 2, 0))
	dinfo->fprintf_func (dinfo->stream, ", %s",
			     nfp_me27_28_crc_bytes[_BF (srcA, 2, 0)]);
      if (_BTST (srcA, 4))
	dinfo->fprintf_func (dinfo->stream, ", bit_swap");
      break;

    default:
      /* s += 'alu[%s, %s, %s, %s]' % (dst, srcAs, op, srcBs).  */
      dinfo->fprintf_func (dinfo->stream, "alu[");

      /* Dest operand.  */
      if (nfp_me_is_imm_opnd10 (dst))
	dinfo->fprintf_func (dinfo->stream, "--");
      else
	err = err || !nfp_me_print_opnd10 (dst, dst_bank, num_ctx,
					   dst_lmext, dinfo);
      dinfo->fprintf_func (dinfo->stream, ", ");

      /* A operand.  */
      if ((op == 0) || (op == 4))	/* B only operators.  */
	dinfo->fprintf_func (dinfo->stream, "--");
      else
	err = err || !nfp_me_print_opnd10 (srcA, (swap) ? 'B' : 'A',
					   num_ctx, src_lmext, dinfo);

      if (!nfp_me27_28_alu_op[op])
	{
	  dinfo->fprintf_func (dinfo->stream, ", <operator:0x%x>, ", op);
	  err = true;
	}
      else
	{
	  dinfo->fprintf_func (dinfo->stream, ", %s, ",
			       nfp_me27_28_alu_op[op]);
	}

      /* B operand.  */
      err = err || !nfp_me_print_opnd10 (srcB, (swap) ? 'A' : 'B',
					 num_ctx, src_lmext, dinfo);
      break;
    }

  if (do_close_bracket)
    dinfo->fprintf_func (dinfo->stream, "]");

  if (nocc)
    dinfo->fprintf_func (dinfo->stream, ", no_cc");
  if (gpr_wrboth)
    dinfo->fprintf_func (dinfo->stream, ", gpr_wrboth");
  if (pred_cc)
    dinfo->fprintf_func (dinfo->stream, ", predicate_cc");

  if (err)
    return _NFP_ERR_CONT;
  return 0;
}

static int
nfp_me27_28_print_immed (uint64_t instr, unsigned int pred_cc,
			 unsigned int dst_lmext,
			 unsigned int gpr_wrboth,
			 int num_ctx, struct disassemble_info *dinfo)
{
  unsigned int srcA = _BF (instr, 9, 0);
  unsigned int srcB = _BF (instr, 19, 10);
  unsigned int imm = _BF (instr, 27, 20);
  unsigned int by = _BTST (instr, 29);
  unsigned int wd = _BTST (instr, 30);
  unsigned int inv = _BTST (instr, 31);
  unsigned int byte_shift = _BF (instr, 34, 33);
  bool err = false;

  if (nfp_me_is_imm_opnd10 (srcB))
    {
      imm = (imm << 8) | nfp_me_imm_opnd10 (srcB);
      if (nfp_me_is_imm_opnd10 (srcA) && (imm == 0))
	{
	  dinfo->fprintf_func (dinfo->stream, "nop");
	  return 0;
	}
    }
  else
    {
      imm = (imm << 8) | nfp_me_imm_opnd10 (srcA);
    }

  if (inv)
    imm = (imm ^ 0xffff) | 0xffff0000U;

  if (by)
    {
      dinfo->fprintf_func (dinfo->stream, "immed_b%d[", byte_shift);
      imm &= 0xff;
    }
  else if (wd)
    {
      dinfo->fprintf_func (dinfo->stream, "immed_w%d[", (byte_shift / 2));
      imm &= 0xffff;
    }
  else
    dinfo->fprintf_func (dinfo->stream, "immed[");

  /* Dest.  */
  if (nfp_me_is_imm_opnd10 (srcA) && nfp_me_is_imm_opnd10 (srcB))
    dinfo->fprintf_func (dinfo->stream, "--");	/* No Dest.  */
  else if (nfp_me_is_imm_opnd10 (srcA))
    err = err || !nfp_me_print_opnd10 (srcB, 'B', num_ctx, dst_lmext, dinfo);
  else
    err = err || !nfp_me_print_opnd10 (srcA, 'A', num_ctx, dst_lmext, dinfo);

  dinfo->fprintf_func (dinfo->stream, ", 0x%x", imm);

  if ((!by) && (!wd) && (byte_shift))
    dinfo->fprintf_func (dinfo->stream, ", <<%d", (byte_shift * 8));

  dinfo->fprintf_func (dinfo->stream, "]");

  if (gpr_wrboth)
    dinfo->fprintf_func (dinfo->stream, ", gpr_wrboth");
  if (pred_cc)
    dinfo->fprintf_func (dinfo->stream, ", predicate_cc");

  if (err)
    return _NFP_ERR_CONT;
  return 0;
}

static int
nfp_me27_28_print_ld_field (uint64_t instr, unsigned int pred_cc,
			    unsigned int dst_lmext, unsigned int src_lmext,
			    unsigned int gpr_wrboth,
			    int num_ctx, struct disassemble_info *dinfo)
{
  unsigned int load_cc = _BTST (instr, 34);
  unsigned int shift = _BF (instr, 32, 28);
  unsigned int byte_mask = _BF (instr, 27, 24);
  unsigned int zerof = _BTST (instr, 20);
  unsigned int swap = _BTST (instr, 19);
  unsigned int imm_msb = _BTST (instr, 18);
  unsigned int src = _BF (instr, 17, 10);
  unsigned int sc = _BF (instr, 9, 8);
  unsigned int dst = _BF (instr, 7, 0);
  bool err = false;

  if (swap)
    {
      unsigned int tmp = src;
      src = dst;
      dst = tmp;
    }

  if (zerof)
    dinfo->fprintf_func (dinfo->stream, "ld_field_w_clr[");
  else
    dinfo->fprintf_func (dinfo->stream, "ld_field[");

  err = err || !nfp_me_print_opnd8 (dst, (swap) ? 'B' : 'A', num_ctx,
				    dst_lmext, imm_msb, dinfo);
  dinfo->fprintf_func (dinfo->stream, ", %d%d%d%d, ",
		       _BTST (byte_mask, 3),
		       _BTST (byte_mask, 2),
		       _BTST (byte_mask, 1), _BTST (byte_mask, 0));
  err = err || !nfp_me_print_opnd8 (src, (swap) ? 'A' : 'B', num_ctx,
				    src_lmext, imm_msb, dinfo);

  if ((sc == 0) && (shift != 0))
    dinfo->fprintf_func (dinfo->stream, ", >>rot%d", shift);
  else if (sc == 1)
    {
      if (shift)
	dinfo->fprintf_func (dinfo->stream, ", >>%d", shift);
      else
	dinfo->fprintf_func (dinfo->stream, ", >>indirect");
    }
  else if (sc == 2)
    {
      if (shift)
	dinfo->fprintf_func (dinfo->stream, ", <<%d", (32 - shift));
      else
	dinfo->fprintf_func (dinfo->stream, ", <<indirect");
    }
  else if (sc == 3)
    dinfo->fprintf_func (dinfo->stream, ", >>dbl%d", shift);

  dinfo->fprintf_func (dinfo->stream, "]");

  if (load_cc)
    dinfo->fprintf_func (dinfo->stream, ", load_cc");
  if (gpr_wrboth)
    dinfo->fprintf_func (dinfo->stream, ", gpr_wrboth");
  if (pred_cc)
    dinfo->fprintf_func (dinfo->stream, ", predicate_cc");

  if (err)
    return _NFP_ERR_CONT;
  return 0;
}

static int
nfp_me27_28_print_ctx_arb (uint64_t instr, struct disassemble_info *dinfo)
{
  unsigned int resume_addr = _BFS (instr, 40, 40, 13) | _BF (instr, 34, 22);
  unsigned int defer = _BF (instr, 21, 20);
  unsigned int no_load = _BTST (instr, 19);
  unsigned int resume = _BTST (instr, 18);
  unsigned int bpt = _BTST (instr, 17);
  unsigned int sig_or = _BTST (instr, 16);
  unsigned int ev_mask = _BF (instr, 15, 0);

  dinfo->fprintf_func (dinfo->stream, "ctx_arb[");
  if (bpt)
    dinfo->fprintf_func (dinfo->stream, "bpt");
  else if (ev_mask == 1)
    dinfo->fprintf_func (dinfo->stream, "voluntary");
  else if ((!no_load) && (ev_mask == 0))
    {
      dinfo->fprintf_func (dinfo->stream, "kill");
      sig_or = 0;
    }
  else if (ev_mask == 0)
    dinfo->fprintf_func (dinfo->stream, "--");
  else
    {
      int first_print = 1;
      unsigned int n;

      for (n = 1; n < 16; n++)
	{
	  if (!_BTST (ev_mask, n))
	    continue;
	  dinfo->fprintf_func (dinfo->stream, "%ssig%d",
			       (first_print) ? "" : ", ", n);
	  first_print = 0;
	}
    }

  dinfo->fprintf_func (dinfo->stream, "]");

  if (sig_or)
    dinfo->fprintf_func (dinfo->stream, ", any");
  if (resume)
    dinfo->fprintf_func (dinfo->stream, ", br[.%d]", resume_addr);
  if (defer)
    dinfo->fprintf_func (dinfo->stream, ", defer[%d]", defer);

  return 0;
}

static int
nfp_me27_28_print_local_csr (uint64_t instr,
			     unsigned int src_lmext,
			     int num_ctx, struct disassemble_info *dinfo)
{
  unsigned int srcA = _BF (instr, 9, 0);
  unsigned int srcB = _BF (instr, 19, 10);
  unsigned int wr = _BTST (instr, 21);
  unsigned int csr_num = _BF (instr, 32, 22);
  unsigned int src = srcA;
  char src_bank = 'A';
  bool err = false;

  if (nfp_me_is_imm_opnd10 (srcA) && !nfp_me_is_imm_opnd10 (srcB))
    {
      src_bank = 'B';
      src = srcB;
    }

  /* MEv28 does not have urd/uwr.  */
  if (csr_num == 1)
    {
      if (wr)
	{
	  dinfo->fprintf_func (dinfo->stream, "uwr[*u$index%d++, ",
			       (int) _BTST (instr, 20));
	  err = err || !nfp_me_print_opnd10 (src, src_bank, num_ctx,
					     src_lmext, dinfo);
	}
      else
	{
	  dinfo->fprintf_func (dinfo->stream, "urd[");
	  err = err || !nfp_me_print_opnd10 (src, src_bank, num_ctx,
					     src_lmext, dinfo);
	  dinfo->fprintf_func (dinfo->stream, ", *u$index%d++",
			       (int) _BTST (instr, 20));
	}
      dinfo->fprintf_func (dinfo->stream, "]");
    }
  else
    {
      const char *nm = NULL;

      if (csr_num < ARRAY_SIZE (nfp_me27_28_mecsrs))
	nm = nfp_me27_28_mecsrs[csr_num];

      dinfo->fprintf_func (dinfo->stream, "local_csr_%s[",
			   (wr) ? "wr" : "rd");
      if (nm)
	dinfo->fprintf_func (dinfo->stream, "%s", nm);
      else
	dinfo->fprintf_func (dinfo->stream, "0x%x", (csr_num * 4));

      if (wr)
	{
	  dinfo->fprintf_func (dinfo->stream, ", ");
	  err = err || !nfp_me_print_opnd10 (src, src_bank, num_ctx,
					     src_lmext, dinfo);
	}
      dinfo->fprintf_func (dinfo->stream, "]");
    }

  if (err)
    return _NFP_ERR_CONT;
  return 0;
}

static int
nfp_me27_28_print_branch (uint64_t instr,
			  const char *br_inpstates[16],
			  struct disassemble_info *dinfo)
{
  unsigned int br_op = _BF (instr, 4, 0);
  unsigned int ctx_sig_state = _BF (instr, 17, 14);
  unsigned int defer = _BF (instr, 21, 20);
  unsigned int br_addr = _BFS (instr, 40, 40, 13) | _BF (instr, 34, 22);
  int ret = 0;

  if (!nfp_me27_28_br_ops[br_op])
    {
      dinfo->fprintf_func (dinfo->stream, _("<invalid branch>["));
      ret = _NFP_ERR_CONT;
    }
  else
    dinfo->fprintf_func (dinfo->stream, "%s[", nfp_me27_28_br_ops[br_op]);

  switch (br_op)
    {
    case 16:			/* br=ctx */
    case 17:			/* br!=ctx */
    case 18:			/* br_signal */
    case 19:			/* br_!signal */
      dinfo->fprintf_func (dinfo->stream, "%d, ", ctx_sig_state);
      break;
    case 20:			/* "br_inp_state" */
    case 21:			/* "br_!inp_state" */
      dinfo->fprintf_func (dinfo->stream, "%s, ",
			   br_inpstates[ctx_sig_state]);
      break;
    case 22:			/* "br_cls_state" */
    case 23:			/* "br_!cls_state" */
      dinfo->fprintf_func (dinfo->stream, "cls_ring%d_status, ",
			   ctx_sig_state);
      break;
    default:
      break;
    }

  dinfo->fprintf_func (dinfo->stream, ".%d]", br_addr);

  if (defer)
    dinfo->fprintf_func (dinfo->stream, ", defer[%d]", defer);

  return ret;
}

static int
nfp_me27_28_print_br_byte (uint64_t instr,
			   unsigned int src_lmext, int num_ctx,
			   struct disassemble_info *dinfo)
{
  unsigned int srcA = _BF (instr, 7, 0);
  unsigned int by = _BF (instr, 9, 8);
  unsigned int srcB = _BF (instr, 17, 10);
  unsigned int imm_msb = _BTST (instr, 18);
  unsigned int eq = _BTST (instr, 19);
  unsigned int defer = _BF (instr, 21, 20);
  unsigned int br_addr = _BFS (instr, 40, 40, 13) | _BF (instr, 34, 22);
  bool err = false;

  if (eq)
    dinfo->fprintf_func (dinfo->stream, "br=byte[");
  else
    dinfo->fprintf_func (dinfo->stream, "br!=byte[");

  if (nfp_me_is_imm_opnd8 (srcA))
    err = err || !nfp_me_print_opnd8 (srcB, 'B', num_ctx,
				      src_lmext, imm_msb, dinfo);
  else
    err = err || !nfp_me_print_opnd8 (srcA, 'A', num_ctx,
				      src_lmext, imm_msb, dinfo);

  dinfo->fprintf_func (dinfo->stream, ", %d, ", by);

  if (nfp_me_is_imm_opnd8 (srcA))
    err = err || !nfp_me_print_opnd8 (srcA, 'A', num_ctx,
				      src_lmext, imm_msb, dinfo);
  else
    err = err || !nfp_me_print_opnd8 (srcB, 'B', num_ctx,
				      src_lmext, imm_msb, dinfo);

  dinfo->fprintf_func (dinfo->stream, ", .%d]", br_addr);

  if (defer)
    dinfo->fprintf_func (dinfo->stream, ", defer[%d]", defer);

  if (err)
    return _NFP_ERR_CONT;
  return 0;
}

static int
nfp_me27_28_print_br_bit (uint64_t instr, unsigned int src_lmext,
			  int num_ctx, struct disassemble_info *dinfo)
{
  unsigned int srcA = _BF (instr, 7, 0);
  unsigned int srcB = _BF (instr, 17, 10);
  unsigned int b = _BTST (instr, 18);
  unsigned int defer = _BF (instr, 21, 20);
  unsigned int br_addr = _BFS (instr, 40, 40, 13) | _BF (instr, 34, 22);
  bool err = false;

  if (b)
    dinfo->fprintf_func (dinfo->stream, "br_bset[");
  else
    dinfo->fprintf_func (dinfo->stream, "br_bclr[");

  if (nfp_me_is_imm_opnd8 (srcA))
    {
      err = err
	|| !nfp_me_print_opnd8 (srcB, 'B', num_ctx, src_lmext, 0, dinfo);
      b = (nfp_me_imm_opnd8 (srcA, 0) - 1) & 0x1f;
    }
  else
    {
      err = err
	|| !nfp_me_print_opnd8 (srcA, 'A', num_ctx, src_lmext, 0, dinfo);
      b = (nfp_me_imm_opnd8 (srcB, 0) - 1) & 0x1f;
    }

  dinfo->fprintf_func (dinfo->stream, ", %d, .%d]", b, br_addr);

  if (defer)
    dinfo->fprintf_func (dinfo->stream, ", defer[%d]", defer);

  if (err)
    return _NFP_ERR_CONT;
  return 0;
}

static int
nfp_me27_28_print_br_alu (uint64_t instr, unsigned int src_lmext,
			  int num_ctx, struct disassemble_info *dinfo)
{
  unsigned int srcA = _BF (instr, 9, 0);
  unsigned int srcB = _BF (instr, 19, 10);
  unsigned int defer = _BF (instr, 21, 20);
  unsigned int imm = _BF (instr, 30, 22);
  bool err = false;

  if (nfp_me_is_imm_opnd10 (srcA))
    imm = (imm << 8) | nfp_me_imm_opnd10 (srcA);
  else
    imm = (imm << 8) | nfp_me_imm_opnd10 (srcB);

  if (!imm)
    dinfo->fprintf_func (dinfo->stream, "rtn[");
  else
    dinfo->fprintf_func (dinfo->stream, "jump[");

  if (nfp_me_is_imm_opnd10 (srcA))
    err = err || !nfp_me_print_opnd10 (srcB, 'B', num_ctx, src_lmext, dinfo);
  else
    err = err || !nfp_me_print_opnd10 (srcA, 'A', num_ctx, src_lmext, dinfo);

  if (imm)
    dinfo->fprintf_func (dinfo->stream, ", .%d", imm);

  dinfo->fprintf_func (dinfo->stream, "]");

  if (defer)
    dinfo->fprintf_func (dinfo->stream, ", defer[%d]", defer);

  if (err)
    return _NFP_ERR_CONT;
  return 0;
}

static int
nfp_me27_28_print_mult (uint64_t instr, unsigned int pred_cc,
			unsigned int dst_lmext, unsigned int src_lmext,
			unsigned int gpr_wrboth,
			int num_ctx, struct disassemble_info *dinfo)
{
  unsigned int srcA = _BF (instr, 9, 0);
  unsigned int srcB = _BF (instr, 19, 10);
  unsigned int mstep = _BF (instr, 22, 20);
  char dst_bank = 'A' + _BTST (instr, 23);
  unsigned int swap = _BTST (instr, 30);
  unsigned int mtype = _BF (instr, 32, 31);
  unsigned int nocc = _BTST (instr, 40);
  bool err = false;

  if (swap)
    {
      unsigned int tmp = srcA;
      srcA = srcB;
      srcB = tmp;
    }

  dinfo->fprintf_func (dinfo->stream, "mul_step[");

  if (mstep >= 4)
    err = err
      || !nfp_me_print_opnd10 (srcA, dst_bank, num_ctx, dst_lmext, dinfo);
  else
    err = err || !nfp_me_print_opnd10 (srcA, (swap) ? 'B' : 'A', num_ctx,
				       src_lmext, dinfo);

  dinfo->fprintf_func (dinfo->stream, ", ");

  if (mstep >= 4)
    dinfo->fprintf_func (dinfo->stream, "--");
  else
    err = err || !nfp_me_print_opnd10 (srcB, (swap) ? 'A' : 'B', num_ctx,
				       src_lmext, dinfo);

  dinfo->fprintf_func (dinfo->stream, "], %s", nfp_me27_28_mult_types[mtype]);
  if (mtype > 0)
    {
      const char *s = nfp_me27_28_mult_steps[mstep];
      if (!s)
	{
	  s = "<invalid mul_step>";
	  err = true;
	}
      dinfo->fprintf_func (dinfo->stream, "_%s", s);
    }

  if (nocc)
    dinfo->fprintf_func (dinfo->stream, ", no_cc");
  if (gpr_wrboth)
    dinfo->fprintf_func (dinfo->stream, ", gpr_wrboth");
  if (pred_cc)
    dinfo->fprintf_func (dinfo->stream, ", predicate_cc");

  if (err)
    return _NFP_ERR_CONT;
  return 0;
}

static int
_nfp_cmp_mnmnc (const void *arg_a, const void *arg_b)
{
  const nfp_cmd_mnemonic *a = arg_a;
  const nfp_cmd_mnemonic *b = arg_b;

  if (a->cpp_target != b->cpp_target)
    return (a->cpp_target > b->cpp_target) - (a->cpp_target < b->cpp_target);

  if (a->cpp_action != b->cpp_action)
    return (a->cpp_action > b->cpp_action) - (a->cpp_action < b->cpp_action);

  return (a->cpp_token > b->cpp_token) - (a->cpp_token < b->cpp_token);
}

static const char *
nfp_me_find_mnemonic (unsigned int cpp_tgt, unsigned int cpp_act,
		      unsigned int cpp_tok, unsigned int cpp_len,
		      const nfp_cmd_mnemonic * mnemonics,
		      size_t mnemonics_cnt)
{
  nfp_cmd_mnemonic search_key = { cpp_tgt, cpp_act, cpp_tok, 0, 0, NULL };
  const nfp_cmd_mnemonic *cmd = NULL;

  cmd = bsearch (&search_key, mnemonics, mnemonics_cnt,
		 sizeof (nfp_cmd_mnemonic), _nfp_cmp_mnmnc);

  if (!cmd)
    return NULL;

  /* Make sure we backtrack to the first entry that still matches the three
     bsearched fields - then we simply iterate and compare cpp_len.  */
  while ((cmd > mnemonics) && (_nfp_cmp_mnmnc (&cmd[-1], &search_key) == 0))
    --cmd;

  /* Now compare by cpp_len and make sure we stay in range.  */
  for (; (cmd < (mnemonics + mnemonics_cnt))
       && (_nfp_cmp_mnmnc (cmd, &search_key) == 0); ++cmd)
    {
      if ((cpp_len & cmd->len_mask) == cmd->len_fixed)
	return cmd->mnemonic;
    }

  return NULL;
}

/* NFP-32xx (ME Version 2.7).  */

static int
nfp_me27_print_cmd (uint64_t instr, int third_party_32bit,
		    int num_ctx, struct disassemble_info *dinfo)
{
  unsigned int srcA = _BF (instr, 7, 0);
  unsigned int ctxswap_defer = _BF (instr, 9, 8);
  unsigned int srcB = _BF (instr, 17, 10);
  unsigned int token = _BF (instr, 19, 18);
  unsigned int xfer = _BFS (instr, 40, 40, 5) | _BF (instr, 24, 20);
  unsigned int cpp_len = _BF (instr, 27, 25);
  unsigned int sig = _BF (instr, 31, 28);
  unsigned int tgtcmd = _BF (instr, 38, 32);
  unsigned int indref = _BTST (instr, 41);
  unsigned int mode = _BF (instr, 44, 42);

  bool err = false;
  int cpp_target = -1;
  int cpp_action = -1;
  const char *mnemonic = NULL;
  unsigned int imm;
  unsigned int valBA;
  int visswap = ((mode == 1) || (mode == 3));

  imm = (sig << 10) | (cpp_len << 7) | ((xfer & 0x1f) << 2) | token;
  valBA = (srcB << 8) | srcA;

  if (mode == 6)
    {
      token = 0;
      sig = 0;
      xfer = 0;
    }

  /* Convert tgtcmd to action/token tuple.  */
  if (_BF (tgtcmd, 6, 5) == 0x0)
    {
      switch (_BF (tgtcmd, 4, 2))
	{
	case 0:
	  cpp_target = NFP_3200_CPPTGT_CAP;
	  dinfo->fprintf_func (dinfo->stream, "cap[");
	  break;
	case 1:
	  cpp_target = NFP_3200_CPPTGT_MSF0;
	  dinfo->fprintf_func (dinfo->stream, "msf0[");
	  break;
	case 2:
	  cpp_target = NFP_3200_CPPTGT_MSF1;
	  dinfo->fprintf_func (dinfo->stream, "msf1[");
	  break;
	case 3:
	  cpp_target = NFP_3200_CPPTGT_PCIE;
	  dinfo->fprintf_func (dinfo->stream, "pcie[");
	  break;
	case 4:
	  cpp_target = NFP_3200_CPPTGT_HASH;
	  break;
	case 5:
	  cpp_target = NFP_3200_CPPTGT_CRYPTO;
	  dinfo->fprintf_func (dinfo->stream, "crypto[");
	  break;
	case 6:
	  cpp_target = NFP_3200_CPPTGT_ARM;
	  dinfo->fprintf_func (dinfo->stream, "arm[");
	  break;
	case 7:
	  cpp_target = NFP_3200_CPPTGT_CT;
	  dinfo->fprintf_func (dinfo->stream, "ct[");
	  break;
	}
      cpp_action = _BF (tgtcmd, 1, 0);
    }
  else
    {
      switch (_BF (tgtcmd, 6, 4))
	{
	case 2:
	  cpp_target = NFP_3200_CPPTGT_GS;
	  dinfo->fprintf_func (dinfo->stream, "scratch[");
	  break;
	case 3:
	  cpp_target = NFP_3200_CPPTGT_QDR;	/* A.k.a. SRAM.  */
	  dinfo->fprintf_func (dinfo->stream, "sram[");
	  break;
	case 4:
	case 5:
	  cpp_target = NFP_3200_CPPTGT_MU;
	  dinfo->fprintf_func (dinfo->stream, "mem[");
	  break;
	case 6:
	case 7:
	  cpp_target = NFP_3200_CPPTGT_CLS;
	  dinfo->fprintf_func (dinfo->stream, "cls[");
	  break;
	}
      cpp_action = _BF (tgtcmd, 3, 0);
    }

  if (cpp_target < 0)
    {
      dinfo->fprintf_func (dinfo->stream, _("<invalid cmd target %d:%d:%d>[]"),
			   cpp_target, cpp_action, token);
      return _NFP_ERR_CONT;
    }

  mnemonic = nfp_me_find_mnemonic (cpp_target, cpp_action, token, cpp_len,
				   nfp_me27_mnemonics,
				   ARRAY_SIZE (nfp_me27_mnemonics));

  if (!mnemonic)
    {
      dinfo->fprintf_func (dinfo->stream, _("<invalid cmd action %d:%d:%d>[]"),
			   cpp_target, cpp_action, token);
      return _NFP_ERR_CONT;
    }

  if (cpp_target == NFP_3200_CPPTGT_HASH)
    {
      dinfo->fprintf_func (dinfo->stream, "%s[$xfer_%d, %d",
			   mnemonic, xfer, cpp_len);
      goto print_opt_toks;
    }

  dinfo->fprintf_func (dinfo->stream, "%s, ", mnemonic);

  if (visswap)
    {
      unsigned int tmp = srcA;
      srcA = srcB;
      srcB = tmp;
    }

  switch (mode)
    {
    case 0:			/* (A << 8) + B.  */
    case 1:			/* (B << 8) + A.  */
      dinfo->fprintf_func (dinfo->stream, "$xfer_%d, ", xfer);
      err = err
	|| !nfp_me_print_opnd8 (srcA, 'A' + visswap, num_ctx, 0, 0, dinfo);
      dinfo->fprintf_func (dinfo->stream, ", <<8, ");
      err = err
	|| !nfp_me_print_opnd8 (srcB, 'B' - visswap, num_ctx, 0, 0, dinfo);
      dinfo->fprintf_func (dinfo->stream, ", %d", (cpp_len + 1));
      break;
    case 2:			/* Accelerated 3rd party (A[ << 8]) + B.  */
    case 3:			/* Accelerated 3rd party (B[ << 8]) + A.  */
      dinfo->fprintf_func (dinfo->stream, "0x%x, ", (indref << 6) | xfer);
      err = err
	|| !nfp_me_print_opnd8 (srcA, 'A' + visswap, num_ctx, 0, 0, dinfo);
      if (third_party_32bit)
	dinfo->fprintf_func (dinfo->stream, ", ");
      else
	dinfo->fprintf_func (dinfo->stream, ", <<8, ");
      err = err
	|| !nfp_me_print_opnd8 (srcB, 'B' - visswap, num_ctx, 0, 0, dinfo);
      dinfo->fprintf_func (dinfo->stream, ", %d", (cpp_len + 1));
      break;
    case 4:			/* A + B.  */
      dinfo->fprintf_func (dinfo->stream, "$xfer_%d, ", xfer);
      err = err || !nfp_me_print_opnd8 (srcA, 'A', num_ctx, 0, 0, dinfo);
      dinfo->fprintf_func (dinfo->stream, ", ");
      err = err || !nfp_me_print_opnd8 (srcB, 'B', num_ctx, 0, 0, dinfo);
      dinfo->fprintf_func (dinfo->stream, ", %d", (cpp_len + 1));
      break;
    case 5:			/* Immediate address.  */
      dinfo->fprintf_func (dinfo->stream, "$xfer_%d, 0x%x, %d", xfer, valBA,
			   (cpp_len + 1));
      break;
    case 6:			/* Immediate address and data.  */
      dinfo->fprintf_func (dinfo->stream, "0x%x, 0x%x", valBA, imm);
      break;
    case 7:			/* Immediate data.  */
      dinfo->fprintf_func (dinfo->stream, "0x%x, --, %d",
			   ((xfer << 16) | valBA), (cpp_len + 1));
      break;
    }

 print_opt_toks:
  dinfo->fprintf_func (dinfo->stream, "]");

  if (indref && (mode != 2) && (mode != 3))
    dinfo->fprintf_func (dinfo->stream, ", indirect_ref");

  if (ctxswap_defer != 3)
    {
      dinfo->fprintf_func (dinfo->stream, ", ctx_swap[");
      if (sig)
	dinfo->fprintf_func (dinfo->stream, "sig%d]", sig);
      else
	dinfo->fprintf_func (dinfo->stream, "--]");

      if (ctxswap_defer != 0)
	dinfo->fprintf_func (dinfo->stream, ", defer[%d]", ctxswap_defer);
    }
  else if (sig)
    dinfo->fprintf_func (dinfo->stream, ", sig_done[sig%d]", sig);

  if (err)
    return _NFP_ERR_CONT;
  return 0;
}

static int
nfp_me27_print_alu_shf (uint64_t instr, int num_ctx,
			struct disassemble_info *dinfo)
{
  return nfp_me27_28_print_alu_shf (instr, 0, 0, 0, 0, num_ctx, dinfo);
}

static int
nfp_me27_print_alu (uint64_t instr, int num_ctx,
		    struct disassemble_info *dinfo)
{
  return nfp_me27_28_print_alu_shf (instr, 0, 0, 0, 0, num_ctx, dinfo);
}

static int
nfp_me27_print_immed (uint64_t instr, int num_ctx,
		      struct disassemble_info *dinfo)
{
  return nfp_me27_28_print_immed (instr, 0, 0, 0, num_ctx, dinfo);
}

static int
nfp_me27_print_ld_field (uint64_t instr, int num_ctx,
			 struct disassemble_info *dinfo)
{
  return nfp_me27_28_print_ld_field (instr, 0, 0, 0, 0, num_ctx, dinfo);
}

static int
nfp_me27_print_ctx_arb (uint64_t instr, struct disassemble_info *dinfo)
{
  return nfp_me27_28_print_ctx_arb (instr, dinfo);
}

static int
nfp_me27_print_local_csr (uint64_t instr, int num_ctx,
			  struct disassemble_info *dinfo)
{
  return nfp_me27_28_print_local_csr (instr, 0, num_ctx, dinfo);
}

static int
nfp_me27_print_branch (uint64_t instr, struct disassemble_info *dinfo)
{
  return nfp_me27_28_print_branch (instr, nfp_me27_br_inpstates, dinfo);
}

static int
nfp_me27_print_br_byte (uint64_t instr, int num_ctx,
			struct disassemble_info *dinfo)
{
  return nfp_me27_28_print_br_byte (instr, 0, num_ctx, dinfo);
}

static int
nfp_me27_print_br_bit (uint64_t instr, int num_ctx,
		       struct disassemble_info *dinfo)
{
  return nfp_me27_28_print_br_bit (instr, 0, num_ctx, dinfo);
}

static int
nfp_me27_print_br_alu (uint64_t instr, int num_ctx,
		       struct disassemble_info *dinfo)
{
  return nfp_me27_28_print_br_alu (instr, 0, num_ctx, dinfo);
}

static int
nfp_me27_print_mult (uint64_t instr, int num_ctx,
		     struct disassemble_info *dinfo)
{
  return nfp_me27_28_print_mult (instr, 0, 0, 0, 0, num_ctx, dinfo);
}

/*NFP-6xxx/4xxx (ME Version 2.8).  */

static int
nfp_me28_print_cmd (uint64_t instr, int third_party_32bit,
		    int num_ctx, struct disassemble_info *dinfo)
{
  unsigned int srcA = _BF (instr, 7, 0);
  unsigned int ctxswap_defer = _BF (instr, 9, 8);
  unsigned int srcB = _BF (instr, 17, 10);
  unsigned int token = _BF (instr, 19, 18);
  unsigned int xfer = _BFS (instr, 40, 40, 5) | _BF (instr, 24, 20);
  unsigned int cpp_len = _BF (instr, 27, 25);
  unsigned int sig = _BF (instr, 31, 28);
  unsigned int tgtcmd = _BF (instr, 38, 32);
  unsigned int indref = _BTST (instr, 41);
  unsigned int mode = _BF (instr, 44, 42);

  bool err = false;
  int cpp_target = -1;
  int cpp_action = -1;
  const char *mnemonic = NULL;
  unsigned int imm;
  unsigned int valBA;
  int visswap = ((mode == 1) || (mode == 3));

  imm = (sig << 10) | (cpp_len << 7) | ((xfer & 0x1f) << 2) | token;
  valBA = (srcB << 8) | srcA;

  if (mode == 6)
    {
      token = 0;
      sig = 0;
      xfer = 0;
    }

  /* Convert tgtcmd to action/token tuple.  */
  if (_BF (tgtcmd, 6, 5) == 0x0)
    {
      switch (_BF (tgtcmd, 4, 2))
	{
	case 0:
	  cpp_target = NFP_6000_CPPTGT_ILA;
	  dinfo->fprintf_func (dinfo->stream, "ila[");
	  break;
	case 1:
	  cpp_target = NFP_6000_CPPTGT_NBI;
	  dinfo->fprintf_func (dinfo->stream, "nbi[");
	  break;
	case 3:
	  cpp_target = NFP_6000_CPPTGT_PCIE;
	  dinfo->fprintf_func (dinfo->stream, "pcie[");
	  break;
	case 5:
	  cpp_target = NFP_6000_CPPTGT_CRYPTO;
	  dinfo->fprintf_func (dinfo->stream, "crypto[");
	  break;
	case 6:
	  cpp_target = NFP_6000_CPPTGT_ARM;
	  dinfo->fprintf_func (dinfo->stream, "arm[");
	  break;
	case 7:
	  cpp_target = NFP_6000_CPPTGT_CTXPB;
	  dinfo->fprintf_func (dinfo->stream, "ct[");
	  break;
	}
      cpp_action = _BF (tgtcmd, 1, 0);
    }
  else
    {
      /* One bit overlap between "t" and "a" fields, for sram it's "t" and
	 for mem/cls it's "a".  */
      cpp_action = _BF (tgtcmd, 4, 0);
      switch (_BF (tgtcmd, 6, 4))
	{
	case 3:
	  cpp_target = NFP_6000_CPPTGT_VQDR;
	  cpp_action = _BF (tgtcmd, 3, 0);
	  dinfo->fprintf_func (dinfo->stream, "sram[");
	  break;
	case 4:
	case 5:
	  cpp_target = NFP_6000_CPPTGT_MU;
	  dinfo->fprintf_func (dinfo->stream, "mem[");
	  break;
	case 6:
	case 7:
	  cpp_target = NFP_6000_CPPTGT_CLS;
	  dinfo->fprintf_func (dinfo->stream, "cls[");
	  break;
	}
    }

  if (cpp_target < 0)
    {
      dinfo->fprintf_func (dinfo->stream, _("<invalid cmd target %d:%d:%d>[]"),
			   cpp_target, cpp_action, token);
      return _NFP_ERR_CONT;
    }

  mnemonic = nfp_me_find_mnemonic (cpp_target, cpp_action, token, cpp_len,
				   nfp_me28_mnemonics,
				   ARRAY_SIZE (nfp_me28_mnemonics));

  if (!mnemonic)
    {
      dinfo->fprintf_func (dinfo->stream, _("<invalid cmd action %d:%d:%d>[]"),
			   cpp_target, cpp_action, token);
      return _NFP_ERR_CONT;
    }

  dinfo->fprintf_func (dinfo->stream, "%s, ", mnemonic);

  if (visswap)
    {
      unsigned int tmp = srcA;
      srcA = srcB;
      srcB = tmp;
    }

  switch (mode)
    {
    case 0:			/* (A << 8) + B.  */
    case 1:			/* (B << 8) + A.  */
      dinfo->fprintf_func (dinfo->stream, "$xfer_%d, ", xfer);
      err = err
	|| !nfp_me_print_opnd8 (srcA, 'A' + visswap, num_ctx, 0, 0, dinfo);
      dinfo->fprintf_func (dinfo->stream, ", <<8, ");
      err = err
	|| !nfp_me_print_opnd8 (srcB, 'B' - visswap, num_ctx, 0, 0, dinfo);
      dinfo->fprintf_func (dinfo->stream, ", %d", (cpp_len + 1));
      break;
    case 2:			/* Accelerated 3rd party (A[ << 8]) + B.  */
    case 3:			/* Accelerated 3rd party (B[ << 8]) + A.  */
      dinfo->fprintf_func (dinfo->stream, "0x%x, ", (indref << 6) | xfer);
      err = err
	|| !nfp_me_print_opnd8 (srcA, 'A' + visswap, num_ctx, 0, 0, dinfo);
      if (third_party_32bit)
	dinfo->fprintf_func (dinfo->stream, ", ");
      else
	dinfo->fprintf_func (dinfo->stream, ", <<8, ");
      err = err
	|| !nfp_me_print_opnd8 (srcB, 'B' - visswap, num_ctx, 0, 0, dinfo);
      dinfo->fprintf_func (dinfo->stream, ", %d", (cpp_len + 1));
      break;
    case 4:			/* A + B.  */
      dinfo->fprintf_func (dinfo->stream, "$xfer_%d, ", xfer);
      err = err || !nfp_me_print_opnd8 (srcA, 'A', num_ctx, 0, 0, dinfo);
      dinfo->fprintf_func (dinfo->stream, ", ");
      err = err || !nfp_me_print_opnd8 (srcB, 'B', num_ctx, 0, 0, dinfo);
      dinfo->fprintf_func (dinfo->stream, ", %d", (cpp_len + 1));
      break;
    case 5:			/* Immediate address.  */
      dinfo->fprintf_func (dinfo->stream, "$xfer_%d, 0x%x, %d", xfer, valBA,
			   (cpp_len + 1));
      break;
    case 6:			/* Immediate address and data.  */
      dinfo->fprintf_func (dinfo->stream, "0x%x, 0x%x", valBA, imm);
      break;
    case 7:			/* Immediate data.  */
      dinfo->fprintf_func (dinfo->stream, "0x%x, --, %d",
			   ((xfer << 16) | valBA), (cpp_len + 1));
      break;
    }

  dinfo->fprintf_func (dinfo->stream, "]");

  if (indref && (mode != 2) && (mode != 3))
    dinfo->fprintf_func (dinfo->stream, ", indirect_ref");

  if (ctxswap_defer != 3)
    {
      dinfo->fprintf_func (dinfo->stream, ", ctx_swap[");
      if (sig)
	dinfo->fprintf_func (dinfo->stream, "sig%d]", sig);
      else
	dinfo->fprintf_func (dinfo->stream, "--]");

      if (ctxswap_defer != 0)
	dinfo->fprintf_func (dinfo->stream, ", defer[%d]", ctxswap_defer);
    }
  else if (sig)
    dinfo->fprintf_func (dinfo->stream, ", sig_done[sig%d]", sig);

  if (err)
    return _NFP_ERR_CONT;
  return 0;
}

static int
nfp_me28_print_alu_shf (uint64_t instr, int num_ctx,
			struct disassemble_info *dinfo)
{
  unsigned int gpr_wrboth = _BTST (instr, 41);
  unsigned int src_lmext = _BTST (instr, 42);
  unsigned int dst_lmext = _BTST (instr, 43);
  unsigned int pred_cc = _BTST (instr, 44);

  return nfp_me27_28_print_alu_shf (instr, pred_cc, dst_lmext,
				    src_lmext, gpr_wrboth, num_ctx, dinfo);
}

static int
nfp_me28_print_alu (uint64_t instr, int num_ctx,
		    struct disassemble_info *dinfo)
{
  unsigned int gpr_wrboth = _BTST (instr, 41);
  unsigned int src_lmext = _BTST (instr, 42);
  unsigned int dst_lmext = _BTST (instr, 43);
  unsigned int pred_cc = _BTST (instr, 44);

  return nfp_me27_28_print_alu (instr, pred_cc, dst_lmext, src_lmext,
				gpr_wrboth, num_ctx, dinfo);
}

static int
nfp_me28_print_immed (uint64_t instr, int num_ctx,
		      struct disassemble_info *dinfo)
{
  unsigned int gpr_wrboth = _BTST (instr, 41);
  unsigned int dst_lmext = _BTST (instr, 43);
  unsigned int pred_cc = _BTST (instr, 44);

  return nfp_me27_28_print_immed (instr, pred_cc, dst_lmext, gpr_wrboth,
				  num_ctx, dinfo);
}

static int
nfp_me28_print_ld_field (uint64_t instr, int num_ctx,
			 struct disassemble_info *dinfo)
{
  unsigned int gpr_wrboth = _BTST (instr, 41);
  unsigned int src_lmext = _BTST (instr, 42);
  unsigned int dst_lmext = _BTST (instr, 43);
  unsigned int pred_cc = _BTST (instr, 44);

  return nfp_me27_28_print_ld_field (instr, pred_cc, dst_lmext,
				     src_lmext, gpr_wrboth, num_ctx, dinfo);
}

static int
nfp_me28_print_ctx_arb (uint64_t instr, struct disassemble_info *dinfo)
{
  return nfp_me27_28_print_ctx_arb (instr, dinfo);
}

static int
nfp_me28_print_local_csr (uint64_t instr, int num_ctx,
			  struct disassemble_info *dinfo)
{
  unsigned int src_lmext = _BTST (instr, 42);

  return nfp_me27_28_print_local_csr (instr, src_lmext, num_ctx, dinfo);
}

static int
nfp_me28_print_branch (uint64_t instr, struct disassemble_info *dinfo)
{
  return nfp_me27_28_print_branch (instr, nfp_me28_br_inpstates, dinfo);
}

static int
nfp_me28_print_br_byte (uint64_t instr, int num_ctx,
			struct disassemble_info *dinfo)
{
  unsigned int src_lmext = _BTST (instr, 42);
  return nfp_me27_28_print_br_byte (instr, src_lmext, num_ctx, dinfo);
}

static int
nfp_me28_print_br_bit (uint64_t instr, int num_ctx,
		       struct disassemble_info *dinfo)
{
  unsigned int src_lmext = _BTST (instr, 42);
  return nfp_me27_28_print_br_bit (instr, src_lmext, num_ctx, dinfo);
}

static int
nfp_me28_print_br_alu (uint64_t instr, int num_ctx,
		       struct disassemble_info *dinfo)
{
  unsigned int src_lmext = _BTST (instr, 42);
  return nfp_me27_28_print_br_alu (instr, src_lmext, num_ctx, dinfo);
}

static int
nfp_me28_print_mult (uint64_t instr, int num_ctx,
		     struct disassemble_info *dinfo)
{
  unsigned int gpr_wrboth = _BTST (instr, 41);
  unsigned int src_lmext = _BTST (instr, 42);
  unsigned int dst_lmext = _BTST (instr, 43);
  unsigned int pred_cc = _BTST (instr, 44);

  return nfp_me27_28_print_mult (instr, pred_cc, dst_lmext, src_lmext,
				 gpr_wrboth, num_ctx, dinfo);
}

static bool
init_nfp3200_priv (nfp_priv_data * priv, struct disassemble_info *dinfo)
{
  Elf_Internal_Shdr *sec = NULL;
  Elf_Nfp_MeConfig mecfg_ent;
  unsigned char buffer[sizeof (Elf_Nfp_MeConfig)];
  file_ptr roff = 0;
  unsigned int sec_cnt = 0;
  unsigned int sec_idx;
  size_t menum_linear = 0;

  if (!dinfo->section)
    /* No section info, will use default values.  */
    return true;

  sec_cnt = elf_numsections (dinfo->section->owner);

  /* Find the MECONFIG section.  It's index is also in e_flags, but it has
     a unique SHT and we'll use that.  */
  for (sec_idx = 0; sec_idx < sec_cnt; sec_idx++)
    {
      sec = elf_elfsections (dinfo->section->owner)[sec_idx];

      if (sec->sh_type == SHT_NFP_MECONFIG)
	break;
    }

  if (sec_idx == sec_cnt)
    {
      dinfo->fprintf_func (dinfo->stream, _("File has no ME-Config section."));
      return false;
    }

  for (roff = 0; (bfd_size_type) roff < sec->sh_size;
       roff += sec->sh_entsize, menum_linear++)
    {
      nfp_priv_mecfg *mecfg;
      int isl = menum_linear >> 3;
      int menum = menum_linear & 7;

      if (menum_linear >= 40)
	{
	  dinfo->fprintf_func (dinfo->stream,
			       _("File has invalid ME-Config section."));
	  return false;
	}

      mecfg = &priv->mecfgs[isl][menum][1];

      if (!bfd_get_section_contents (dinfo->section->owner, sec->bfd_section,
				     buffer, roff, sizeof (buffer)))
	return false;

      mecfg_ent.ctx_enables = bfd_getl32 (buffer + offsetof (Elf_Nfp_MeConfig,
							     ctx_enables));
      mecfg_ent.misc_control = bfd_getl32 (buffer
	+ offsetof (Elf_Nfp_MeConfig, misc_control));

      mecfg->ctx4_mode = _BTST (mecfg_ent.ctx_enables, 31);
      mecfg->addr_3rdparty32 = _BTST (mecfg_ent.misc_control, 4);
      mecfg->scs_cnt = _BTST (mecfg_ent.misc_control, 2);
    }

  return true;
}

static bool
init_nfp6000_mecsr_sec (nfp_priv_data * priv, Elf_Internal_Shdr * sec,
			bool is_for_text, struct disassemble_info *dinfo)
{
  Elf_Nfp_InitRegEntry ireg;
  unsigned char buffer[sizeof (Elf_Nfp_InitRegEntry)];
  file_ptr ireg_off = 0;
  size_t isl, menum;

  if (sec->sh_entsize != sizeof (ireg))
    return false;

  isl = SHI_NFP_IREG_ISLAND (sec->sh_info);

  /* For these sections we know that the address will only be 32 bits
     so we only need cpp_offset_lo.
     Address is encoded as follows:
     <31:30> 0
     <29:24> island (already got this from sh_info)
     <23:17> 0
     <16:16> XferCsrRegSel (1 for these sections)
     <15:14> 0
     <13:10> DataMasterID (MEnum = this - 4)
     <9:2> register (index)
     <1:0> 0b0 (register byte address if appened to the previous field).  */
  for (ireg_off = 0; (bfd_size_type) ireg_off < sec->sh_size;
       ireg_off += sec->sh_entsize)
    {
      uint32_t csr_off;
      nfp_priv_mecfg *mecfg;

      if (!bfd_get_section_contents (dinfo->section->owner, sec->bfd_section,
				     buffer, ireg_off, sizeof (buffer)))
	return false;

      ireg.cpp_offset_lo = bfd_getl32 (buffer
	+ offsetof (Elf_Nfp_InitRegEntry, cpp_offset_lo));
      ireg.mask = bfd_getl32 (buffer + offsetof (Elf_Nfp_InitRegEntry, mask));
      ireg.val = bfd_getl32 (buffer + offsetof (Elf_Nfp_InitRegEntry, val));
      ireg.w0 = bfd_getl32 (buffer + offsetof (Elf_Nfp_InitRegEntry, w0));

      if (NFP_IREG_ENTRY_WO_NLW (ireg.w0))
	continue;

      /* Only consider entries that are permanent for runtime.  */
      if ((NFP_IREG_ENTRY_WO_VTP (ireg.w0) != NFP_IREG_VTP_CONST)
	  && (NFP_IREG_ENTRY_WO_VTP (ireg.w0) != NFP_IREG_VTP_FORCE))
	continue;

      menum = _BF (ireg.cpp_offset_lo, 13, 10) - 4;
      csr_off = _BF (ireg.cpp_offset_lo, 9, 0);

      if (isl >= _NFP_ISLAND_MAX || menum >= _NFP_ME_MAX)
	return false;
	
      mecfg = &priv->mecfgs[isl][menum][is_for_text];
      switch (csr_off)
	{
	case _NFP_ME27_28_CSR_CTX_ENABLES:
	  mecfg->ctx4_mode = _BTST (ireg.val, 31);
	  break;
	case _NFP_ME27_28_CSR_MISC_CONTROL:
	  mecfg->addr_3rdparty32 = _BTST (ireg.val, 4);
	  mecfg->scs_cnt = _BTST (ireg.val, 2);
	  break;
	default:
	  break;
	}
    }

  return true;
}

static bool
init_nfp6000_priv (nfp_priv_data * priv, struct disassemble_info *dinfo)
{
  int mecfg_orders[64][2];
  size_t isl;
  unsigned int sec_cnt = 0;
  unsigned int sec_idx;
  bool is_for_text;

  memset (mecfg_orders, -1, sizeof (mecfg_orders));

  if (dinfo->section == NULL
      || dinfo->section->owner == NULL
      || elf_elfsections (dinfo->section->owner) == NULL)
    /* No section info, will use default values.  */
    return true;

  sec_cnt = elf_numsections (dinfo->section->owner);

  /* Go through all MECSR init sections to find ME configs.  */
  for (sec_idx = 0; sec_idx < sec_cnt; sec_idx++)
    {
      Elf_Internal_Shdr *sec;
      int sec_order;

      sec = elf_elfsections (dinfo->section->owner)[sec_idx];
      sec_order = (int) SHI_NFP_IREG_ORDER (sec->sh_info);

      is_for_text = (sec->sh_flags & (SHF_NFP_INIT | SHF_NFP_INIT2)) == 0;

      /* If we have an init2 section, that is the one that applies to the
	 ME when executing init code.  So we make it's order higher than
	 any plain init section.  */
      if (sec->sh_flags & SHF_NFP_INIT2)
	sec_order += SHI_NFP_IREG_ORDER (~0U) + 1;

      if (sec->sh_type != SHT_NFP_INITREG)
	continue;
      if (!SHI_NFP_6000_IS_IREG_MECSR (sec->sh_info))
	continue;

      isl = SHI_NFP_IREG_ISLAND (sec->sh_info);
      if ((sec_order < mecfg_orders[isl][is_for_text]))
	/* Lower order or transient, skip it.  */
	continue;

      mecfg_orders[isl][is_for_text] = sec_order;

      if (!init_nfp6000_mecsr_sec (priv, sec, is_for_text, dinfo))
	{
	  dinfo->fprintf_func (dinfo->stream,
			       _("Error processing section %u "), sec_idx);
	  return false;
	}
    }

  return true;
}

static int
parse_disassembler_options (nfp_opts * opts, struct disassemble_info *dinfo)
{
  const char *option;

  if (dinfo->disassembler_options == NULL)
    return 0;

  FOR_EACH_DISASSEMBLER_OPTION (option, dinfo->disassembler_options)
  {
    if (disassembler_options_cmp (option, "no-pc") == 0)
      opts->show_pc = 0;
    else if (disassembler_options_cmp (option, "ctx4") == 0)
      {
	if (!opts->ctx_mode)
	  opts->ctx_mode = 4;
      }
    else if (disassembler_options_cmp (option, "ctx8") == 0)
      opts->ctx_mode = 8;
    else
      {
	dinfo->fprintf_func (dinfo->stream, _("Invalid NFP option: %s"), option);
	return _NFP_ERR_STOP;
      }
  }

  return 0;
}

/* Called on first disassembly attempt so that dinfo->section is valid
   so that we can get the bfd owner to find ME configs.  */

static nfp_priv_data *
init_nfp_priv (struct disassemble_info *dinfo)
{
  nfp_priv_data *priv;
  int ret = false;

  if (dinfo->private_data)
    return (nfp_priv_data *) dinfo->private_data;

#if 0  /* Right now only section-related info is kept in priv.
	  So don't even calloc it if we don't need it.  */
  if (!dinfo->section)
     return NULL;
#endif

  /* Alloc with no free, seems to be either this or a static global variable
     and this at least keeps a large struct unallocated until really needed.  */
  priv = calloc (1, sizeof (*priv));
  if (!priv)
    return NULL;

  switch (dinfo->mach)
    {
    case E_NFP_MACH_3200:
      ret = init_nfp3200_priv (priv, dinfo);
      break;
    case E_NFP_MACH_6000:
      ret = init_nfp6000_priv (priv, dinfo);
      break;
    }

  if (!ret)
    {
      free (priv);
      return NULL;
    }

  dinfo->private_data = priv;
  return priv;
}

static int
_print_instrs (bfd_vma addr, struct disassemble_info *dinfo, nfp_opts * opts)
{
  nfp_priv_data *priv = init_nfp_priv (dinfo);
  bfd_byte buffer[8];
  int err;
  uint64_t instr = 0;
  size_t island, menum;
  int num_ctx, scs_cnt, addr_3rdparty32, pc, tmpi, tmpj;
  int is_text = 1;

  err = dinfo->read_memory_func (addr, buffer, 8, dinfo);
  if (err)
    return _NFP_ERR_STOP;

  if (!dinfo->section)
    {
      num_ctx = 8;
      scs_cnt = 0;
      addr_3rdparty32 = 0;
    }
  else
    {
      unsigned int sh_info = 0;
      nfp_priv_mecfg *mecfg;

      /* We have a section, presumably all ELF sections.  Try to find
	 proper ME configs to produce better disassembly.  */
      if (!priv)
	return _NFP_ERR_STOP;	/* Sanity check */

      is_text = (elf_section_flags (dinfo->section)
		 & (SHF_NFP_INIT | SHF_NFP_INIT2)) == 0;

      sh_info = elf_section_info (dinfo->section);

      switch (dinfo->mach)
	{
	case E_NFP_MACH_3200:
	  island = SHI_NFP_3200_ISLAND (sh_info);
	  menum = SHI_NFP_3200_MENUM (sh_info);
	  break;
	default:
	  island = SHI_NFP_ISLAND (sh_info);
	  menum = SHI_NFP_MENUM (sh_info);
	  break;
	}

      if (island >= _NFP_ISLAND_MAX || menum >= _NFP_ME_MAX)
	{
	  dinfo->fprintf_func (dinfo->stream, "Invalid island or me.");
	  return _NFP_ERR_STOP;
	}

      mecfg = &priv->mecfgs[island][menum][is_text];
      num_ctx = (mecfg->ctx4_mode) ? 4 : 8;
      addr_3rdparty32 = mecfg->addr_3rdparty32;
      scs_cnt = mecfg->scs_cnt;
    }

  if (opts->ctx_mode)
    num_ctx = opts->ctx_mode;

  dinfo->bytes_per_line = 8;
  dinfo->bytes_per_chunk = 8;

  instr = bfd_getl64 (buffer);

  if (opts->show_pc)
    {
      pc = (int) (addr >> 3);

      /* Guess max PC for formatting */
      tmpj = (int) (dinfo->buffer_length >> 3);
      if (scs_cnt == 1)
	{
	  pc *= 2;
	  tmpj *= 2;
	  if (! !(menum & 1))
	    {
	      pc++;
	      tmpj++;
	    }
	}

      for (tmpi = 1; tmpj > 9; tmpj /= 10)
	tmpi++;

      tmpj = pc;
      for (; tmpj > 9; tmpj /= 10)
	tmpi--;

      dinfo->fprintf_func (dinfo->stream, "%*c%d  ", tmpi, '.', pc);
    }

  switch (dinfo->mach)
    {
    case E_NFP_MACH_3200:
      if (NFP_ME27_INSTR_IS_CMD (instr))
	err = nfp_me27_print_cmd (instr, addr_3rdparty32, num_ctx, dinfo);
      else if (NFP_ME27_INSTR_IS_ALU_SHF (instr))
	err = nfp_me27_print_alu_shf (instr, num_ctx, dinfo);
      else if (NFP_ME27_INSTR_IS_ALU (instr))
	err = nfp_me27_print_alu (instr, num_ctx, dinfo);
      else if (NFP_ME27_INSTR_IS_IMMED (instr))
	err = nfp_me27_print_immed (instr, num_ctx, dinfo);
      else if (NFP_ME27_INSTR_IS_LD_FIELD (instr))
	err = nfp_me27_print_ld_field (instr, num_ctx, dinfo);
      else if (NFP_ME27_INSTR_IS_CTX_ARB (instr))
	err = nfp_me27_print_ctx_arb (instr, dinfo);
      else if (NFP_ME27_INSTR_IS_LOCAL_CSR (instr))
	err = nfp_me27_print_local_csr (instr, num_ctx, dinfo);
      else if (NFP_ME27_INSTR_IS_BRANCH (instr))
	err = nfp_me27_print_branch (instr, dinfo);
      else if (NFP_ME27_INSTR_IS_BR_BYTE (instr))
	err = nfp_me27_print_br_byte (instr, num_ctx, dinfo);
      else if (NFP_ME27_INSTR_IS_BR_BIT (instr))
	err = nfp_me27_print_br_bit (instr, num_ctx, dinfo);
      else if (NFP_ME27_INSTR_IS_BR_ALU (instr))
	err = nfp_me27_print_br_alu (instr, num_ctx, dinfo);
      else if (NFP_ME27_INSTR_IS_MULT (instr))
	err = nfp_me27_print_mult (instr, num_ctx, dinfo);
      else
	err = nfp_me_print_invalid (instr, dinfo);
      break;

    case E_NFP_MACH_6000:
      if (NFP_ME28_INSTR_IS_CMD (instr))
	err = nfp_me28_print_cmd (instr, addr_3rdparty32, num_ctx, dinfo);
      else if (NFP_ME28_INSTR_IS_ALU_SHF (instr))
	err = nfp_me28_print_alu_shf (instr, num_ctx, dinfo);
      else if (NFP_ME28_INSTR_IS_ALU (instr))
	err = nfp_me28_print_alu (instr, num_ctx, dinfo);
      else if (NFP_ME28_INSTR_IS_IMMED (instr))
	err = nfp_me28_print_immed (instr, num_ctx, dinfo);
      else if (NFP_ME28_INSTR_IS_LD_FIELD (instr))
	err = nfp_me28_print_ld_field (instr, num_ctx, dinfo);
      else if (NFP_ME28_INSTR_IS_CTX_ARB (instr))
	err = nfp_me28_print_ctx_arb (instr, dinfo);
      else if (NFP_ME28_INSTR_IS_LOCAL_CSR (instr))
	err = nfp_me28_print_local_csr (instr, num_ctx, dinfo);
      else if (NFP_ME28_INSTR_IS_BRANCH (instr))
	err = nfp_me28_print_branch (instr, dinfo);
      else if (NFP_ME28_INSTR_IS_BR_BYTE (instr))
	err = nfp_me28_print_br_byte (instr, num_ctx, dinfo);
      else if (NFP_ME28_INSTR_IS_BR_BIT (instr))
	err = nfp_me28_print_br_bit (instr, num_ctx, dinfo);
      else if (NFP_ME28_INSTR_IS_BR_ALU (instr))
	err = nfp_me28_print_br_alu (instr, num_ctx, dinfo);
      else if (NFP_ME28_INSTR_IS_MULT (instr))
	err = nfp_me28_print_mult (instr, num_ctx, dinfo);
      else
	err = nfp_me_print_invalid (instr, dinfo);
      break;
    }

  if (err < 0)
    return err;
  return 8;
}

int
print_insn_nfp (bfd_vma addr, struct disassemble_info *dinfo)
{
  nfp_opts opts;
  int err;

  opts.show_pc = 1;
  opts.ctx_mode = 0;
  err = parse_disassembler_options (&opts, dinfo);
  if (err < 0)
    goto end;

  err = _print_instrs (addr, dinfo, &opts);

 end:
  if (err != 8)
    dinfo->fprintf_func (dinfo->stream, "\t # ERROR");
  if (err == _NFP_ERR_CONT)
    return 8;
  return err;
}

void
print_nfp_disassembler_options (FILE * stream)
{
  fprintf (stream, _("\n\
The following NFP specific disassembler options are supported for use\n\
with the -M switch (multiple options should be separated by commas):\n"));

  fprintf (stream, _("\n\
  no-pc		    Don't print program counter prefix.\n\
  ctx4		    Force disassembly using 4-context mode.\n\
  ctx8		    Force 8-context mode, takes precedence."));

  fprintf (stream, _("\n"));
}
