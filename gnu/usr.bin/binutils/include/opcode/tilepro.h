/* TILEPro opcode information.
 *
 *    Copyright (C) 2011-2023 Free Software Foundation, Inc.
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
 *    MA 02110-1301, USA.
 */

#ifndef opcode_tilepro_h
#define opcode_tilepro_h

typedef unsigned long long tilepro_bundle_bits;


enum
{
  TILEPRO_MAX_OPERANDS = 5 /* mm */
};

typedef enum
{
  TILEPRO_OPC_BPT,
  TILEPRO_OPC_INFO,
  TILEPRO_OPC_INFOL,
  TILEPRO_OPC_J,
  TILEPRO_OPC_JAL,
  TILEPRO_OPC_LW_TLS,
  TILEPRO_OPC_LW_TLS_SN,
  TILEPRO_OPC_MOVE,
  TILEPRO_OPC_MOVE_SN,
  TILEPRO_OPC_MOVEI,
  TILEPRO_OPC_MOVEI_SN,
  TILEPRO_OPC_MOVELI,
  TILEPRO_OPC_MOVELI_SN,
  TILEPRO_OPC_MOVELIS,
  TILEPRO_OPC_PREFETCH,
  TILEPRO_OPC_RAISE,
  TILEPRO_OPC_ADD,
  TILEPRO_OPC_ADD_SN,
  TILEPRO_OPC_ADDB,
  TILEPRO_OPC_ADDB_SN,
  TILEPRO_OPC_ADDBS_U,
  TILEPRO_OPC_ADDBS_U_SN,
  TILEPRO_OPC_ADDH,
  TILEPRO_OPC_ADDH_SN,
  TILEPRO_OPC_ADDHS,
  TILEPRO_OPC_ADDHS_SN,
  TILEPRO_OPC_ADDI,
  TILEPRO_OPC_ADDI_SN,
  TILEPRO_OPC_ADDIB,
  TILEPRO_OPC_ADDIB_SN,
  TILEPRO_OPC_ADDIH,
  TILEPRO_OPC_ADDIH_SN,
  TILEPRO_OPC_ADDLI,
  TILEPRO_OPC_ADDLI_SN,
  TILEPRO_OPC_ADDLIS,
  TILEPRO_OPC_ADDS,
  TILEPRO_OPC_ADDS_SN,
  TILEPRO_OPC_ADIFFB_U,
  TILEPRO_OPC_ADIFFB_U_SN,
  TILEPRO_OPC_ADIFFH,
  TILEPRO_OPC_ADIFFH_SN,
  TILEPRO_OPC_AND,
  TILEPRO_OPC_AND_SN,
  TILEPRO_OPC_ANDI,
  TILEPRO_OPC_ANDI_SN,
  TILEPRO_OPC_AULI,
  TILEPRO_OPC_AVGB_U,
  TILEPRO_OPC_AVGB_U_SN,
  TILEPRO_OPC_AVGH,
  TILEPRO_OPC_AVGH_SN,
  TILEPRO_OPC_BBNS,
  TILEPRO_OPC_BBNS_SN,
  TILEPRO_OPC_BBNST,
  TILEPRO_OPC_BBNST_SN,
  TILEPRO_OPC_BBS,
  TILEPRO_OPC_BBS_SN,
  TILEPRO_OPC_BBST,
  TILEPRO_OPC_BBST_SN,
  TILEPRO_OPC_BGEZ,
  TILEPRO_OPC_BGEZ_SN,
  TILEPRO_OPC_BGEZT,
  TILEPRO_OPC_BGEZT_SN,
  TILEPRO_OPC_BGZ,
  TILEPRO_OPC_BGZ_SN,
  TILEPRO_OPC_BGZT,
  TILEPRO_OPC_BGZT_SN,
  TILEPRO_OPC_BITX,
  TILEPRO_OPC_BITX_SN,
  TILEPRO_OPC_BLEZ,
  TILEPRO_OPC_BLEZ_SN,
  TILEPRO_OPC_BLEZT,
  TILEPRO_OPC_BLEZT_SN,
  TILEPRO_OPC_BLZ,
  TILEPRO_OPC_BLZ_SN,
  TILEPRO_OPC_BLZT,
  TILEPRO_OPC_BLZT_SN,
  TILEPRO_OPC_BNZ,
  TILEPRO_OPC_BNZ_SN,
  TILEPRO_OPC_BNZT,
  TILEPRO_OPC_BNZT_SN,
  TILEPRO_OPC_BYTEX,
  TILEPRO_OPC_BYTEX_SN,
  TILEPRO_OPC_BZ,
  TILEPRO_OPC_BZ_SN,
  TILEPRO_OPC_BZT,
  TILEPRO_OPC_BZT_SN,
  TILEPRO_OPC_CLZ,
  TILEPRO_OPC_CLZ_SN,
  TILEPRO_OPC_CRC32_32,
  TILEPRO_OPC_CRC32_32_SN,
  TILEPRO_OPC_CRC32_8,
  TILEPRO_OPC_CRC32_8_SN,
  TILEPRO_OPC_CTZ,
  TILEPRO_OPC_CTZ_SN,
  TILEPRO_OPC_DRAIN,
  TILEPRO_OPC_DTLBPR,
  TILEPRO_OPC_DWORD_ALIGN,
  TILEPRO_OPC_DWORD_ALIGN_SN,
  TILEPRO_OPC_FINV,
  TILEPRO_OPC_FLUSH,
  TILEPRO_OPC_FNOP,
  TILEPRO_OPC_ICOH,
  TILEPRO_OPC_ILL,
  TILEPRO_OPC_INTHB,
  TILEPRO_OPC_INTHB_SN,
  TILEPRO_OPC_INTHH,
  TILEPRO_OPC_INTHH_SN,
  TILEPRO_OPC_INTLB,
  TILEPRO_OPC_INTLB_SN,
  TILEPRO_OPC_INTLH,
  TILEPRO_OPC_INTLH_SN,
  TILEPRO_OPC_INV,
  TILEPRO_OPC_IRET,
  TILEPRO_OPC_JALB,
  TILEPRO_OPC_JALF,
  TILEPRO_OPC_JALR,
  TILEPRO_OPC_JALRP,
  TILEPRO_OPC_JB,
  TILEPRO_OPC_JF,
  TILEPRO_OPC_JR,
  TILEPRO_OPC_JRP,
  TILEPRO_OPC_LB,
  TILEPRO_OPC_LB_SN,
  TILEPRO_OPC_LB_U,
  TILEPRO_OPC_LB_U_SN,
  TILEPRO_OPC_LBADD,
  TILEPRO_OPC_LBADD_SN,
  TILEPRO_OPC_LBADD_U,
  TILEPRO_OPC_LBADD_U_SN,
  TILEPRO_OPC_LH,
  TILEPRO_OPC_LH_SN,
  TILEPRO_OPC_LH_U,
  TILEPRO_OPC_LH_U_SN,
  TILEPRO_OPC_LHADD,
  TILEPRO_OPC_LHADD_SN,
  TILEPRO_OPC_LHADD_U,
  TILEPRO_OPC_LHADD_U_SN,
  TILEPRO_OPC_LNK,
  TILEPRO_OPC_LNK_SN,
  TILEPRO_OPC_LW,
  TILEPRO_OPC_LW_SN,
  TILEPRO_OPC_LW_NA,
  TILEPRO_OPC_LW_NA_SN,
  TILEPRO_OPC_LWADD,
  TILEPRO_OPC_LWADD_SN,
  TILEPRO_OPC_LWADD_NA,
  TILEPRO_OPC_LWADD_NA_SN,
  TILEPRO_OPC_MAXB_U,
  TILEPRO_OPC_MAXB_U_SN,
  TILEPRO_OPC_MAXH,
  TILEPRO_OPC_MAXH_SN,
  TILEPRO_OPC_MAXIB_U,
  TILEPRO_OPC_MAXIB_U_SN,
  TILEPRO_OPC_MAXIH,
  TILEPRO_OPC_MAXIH_SN,
  TILEPRO_OPC_MF,
  TILEPRO_OPC_MFSPR,
  TILEPRO_OPC_MINB_U,
  TILEPRO_OPC_MINB_U_SN,
  TILEPRO_OPC_MINH,
  TILEPRO_OPC_MINH_SN,
  TILEPRO_OPC_MINIB_U,
  TILEPRO_OPC_MINIB_U_SN,
  TILEPRO_OPC_MINIH,
  TILEPRO_OPC_MINIH_SN,
  TILEPRO_OPC_MM,
  TILEPRO_OPC_MNZ,
  TILEPRO_OPC_MNZ_SN,
  TILEPRO_OPC_MNZB,
  TILEPRO_OPC_MNZB_SN,
  TILEPRO_OPC_MNZH,
  TILEPRO_OPC_MNZH_SN,
  TILEPRO_OPC_MTSPR,
  TILEPRO_OPC_MULHH_SS,
  TILEPRO_OPC_MULHH_SS_SN,
  TILEPRO_OPC_MULHH_SU,
  TILEPRO_OPC_MULHH_SU_SN,
  TILEPRO_OPC_MULHH_UU,
  TILEPRO_OPC_MULHH_UU_SN,
  TILEPRO_OPC_MULHHA_SS,
  TILEPRO_OPC_MULHHA_SS_SN,
  TILEPRO_OPC_MULHHA_SU,
  TILEPRO_OPC_MULHHA_SU_SN,
  TILEPRO_OPC_MULHHA_UU,
  TILEPRO_OPC_MULHHA_UU_SN,
  TILEPRO_OPC_MULHHSA_UU,
  TILEPRO_OPC_MULHHSA_UU_SN,
  TILEPRO_OPC_MULHL_SS,
  TILEPRO_OPC_MULHL_SS_SN,
  TILEPRO_OPC_MULHL_SU,
  TILEPRO_OPC_MULHL_SU_SN,
  TILEPRO_OPC_MULHL_US,
  TILEPRO_OPC_MULHL_US_SN,
  TILEPRO_OPC_MULHL_UU,
  TILEPRO_OPC_MULHL_UU_SN,
  TILEPRO_OPC_MULHLA_SS,
  TILEPRO_OPC_MULHLA_SS_SN,
  TILEPRO_OPC_MULHLA_SU,
  TILEPRO_OPC_MULHLA_SU_SN,
  TILEPRO_OPC_MULHLA_US,
  TILEPRO_OPC_MULHLA_US_SN,
  TILEPRO_OPC_MULHLA_UU,
  TILEPRO_OPC_MULHLA_UU_SN,
  TILEPRO_OPC_MULHLSA_UU,
  TILEPRO_OPC_MULHLSA_UU_SN,
  TILEPRO_OPC_MULLL_SS,
  TILEPRO_OPC_MULLL_SS_SN,
  TILEPRO_OPC_MULLL_SU,
  TILEPRO_OPC_MULLL_SU_SN,
  TILEPRO_OPC_MULLL_UU,
  TILEPRO_OPC_MULLL_UU_SN,
  TILEPRO_OPC_MULLLA_SS,
  TILEPRO_OPC_MULLLA_SS_SN,
  TILEPRO_OPC_MULLLA_SU,
  TILEPRO_OPC_MULLLA_SU_SN,
  TILEPRO_OPC_MULLLA_UU,
  TILEPRO_OPC_MULLLA_UU_SN,
  TILEPRO_OPC_MULLLSA_UU,
  TILEPRO_OPC_MULLLSA_UU_SN,
  TILEPRO_OPC_MVNZ,
  TILEPRO_OPC_MVNZ_SN,
  TILEPRO_OPC_MVZ,
  TILEPRO_OPC_MVZ_SN,
  TILEPRO_OPC_MZ,
  TILEPRO_OPC_MZ_SN,
  TILEPRO_OPC_MZB,
  TILEPRO_OPC_MZB_SN,
  TILEPRO_OPC_MZH,
  TILEPRO_OPC_MZH_SN,
  TILEPRO_OPC_NAP,
  TILEPRO_OPC_NOP,
  TILEPRO_OPC_NOR,
  TILEPRO_OPC_NOR_SN,
  TILEPRO_OPC_OR,
  TILEPRO_OPC_OR_SN,
  TILEPRO_OPC_ORI,
  TILEPRO_OPC_ORI_SN,
  TILEPRO_OPC_PACKBS_U,
  TILEPRO_OPC_PACKBS_U_SN,
  TILEPRO_OPC_PACKHB,
  TILEPRO_OPC_PACKHB_SN,
  TILEPRO_OPC_PACKHS,
  TILEPRO_OPC_PACKHS_SN,
  TILEPRO_OPC_PACKLB,
  TILEPRO_OPC_PACKLB_SN,
  TILEPRO_OPC_PCNT,
  TILEPRO_OPC_PCNT_SN,
  TILEPRO_OPC_RL,
  TILEPRO_OPC_RL_SN,
  TILEPRO_OPC_RLI,
  TILEPRO_OPC_RLI_SN,
  TILEPRO_OPC_S1A,
  TILEPRO_OPC_S1A_SN,
  TILEPRO_OPC_S2A,
  TILEPRO_OPC_S2A_SN,
  TILEPRO_OPC_S3A,
  TILEPRO_OPC_S3A_SN,
  TILEPRO_OPC_SADAB_U,
  TILEPRO_OPC_SADAB_U_SN,
  TILEPRO_OPC_SADAH,
  TILEPRO_OPC_SADAH_SN,
  TILEPRO_OPC_SADAH_U,
  TILEPRO_OPC_SADAH_U_SN,
  TILEPRO_OPC_SADB_U,
  TILEPRO_OPC_SADB_U_SN,
  TILEPRO_OPC_SADH,
  TILEPRO_OPC_SADH_SN,
  TILEPRO_OPC_SADH_U,
  TILEPRO_OPC_SADH_U_SN,
  TILEPRO_OPC_SB,
  TILEPRO_OPC_SBADD,
  TILEPRO_OPC_SEQ,
  TILEPRO_OPC_SEQ_SN,
  TILEPRO_OPC_SEQB,
  TILEPRO_OPC_SEQB_SN,
  TILEPRO_OPC_SEQH,
  TILEPRO_OPC_SEQH_SN,
  TILEPRO_OPC_SEQI,
  TILEPRO_OPC_SEQI_SN,
  TILEPRO_OPC_SEQIB,
  TILEPRO_OPC_SEQIB_SN,
  TILEPRO_OPC_SEQIH,
  TILEPRO_OPC_SEQIH_SN,
  TILEPRO_OPC_SH,
  TILEPRO_OPC_SHADD,
  TILEPRO_OPC_SHL,
  TILEPRO_OPC_SHL_SN,
  TILEPRO_OPC_SHLB,
  TILEPRO_OPC_SHLB_SN,
  TILEPRO_OPC_SHLH,
  TILEPRO_OPC_SHLH_SN,
  TILEPRO_OPC_SHLI,
  TILEPRO_OPC_SHLI_SN,
  TILEPRO_OPC_SHLIB,
  TILEPRO_OPC_SHLIB_SN,
  TILEPRO_OPC_SHLIH,
  TILEPRO_OPC_SHLIH_SN,
  TILEPRO_OPC_SHR,
  TILEPRO_OPC_SHR_SN,
  TILEPRO_OPC_SHRB,
  TILEPRO_OPC_SHRB_SN,
  TILEPRO_OPC_SHRH,
  TILEPRO_OPC_SHRH_SN,
  TILEPRO_OPC_SHRI,
  TILEPRO_OPC_SHRI_SN,
  TILEPRO_OPC_SHRIB,
  TILEPRO_OPC_SHRIB_SN,
  TILEPRO_OPC_SHRIH,
  TILEPRO_OPC_SHRIH_SN,
  TILEPRO_OPC_SLT,
  TILEPRO_OPC_SLT_SN,
  TILEPRO_OPC_SLT_U,
  TILEPRO_OPC_SLT_U_SN,
  TILEPRO_OPC_SLTB,
  TILEPRO_OPC_SLTB_SN,
  TILEPRO_OPC_SLTB_U,
  TILEPRO_OPC_SLTB_U_SN,
  TILEPRO_OPC_SLTE,
  TILEPRO_OPC_SLTE_SN,
  TILEPRO_OPC_SLTE_U,
  TILEPRO_OPC_SLTE_U_SN,
  TILEPRO_OPC_SLTEB,
  TILEPRO_OPC_SLTEB_SN,
  TILEPRO_OPC_SLTEB_U,
  TILEPRO_OPC_SLTEB_U_SN,
  TILEPRO_OPC_SLTEH,
  TILEPRO_OPC_SLTEH_SN,
  TILEPRO_OPC_SLTEH_U,
  TILEPRO_OPC_SLTEH_U_SN,
  TILEPRO_OPC_SLTH,
  TILEPRO_OPC_SLTH_SN,
  TILEPRO_OPC_SLTH_U,
  TILEPRO_OPC_SLTH_U_SN,
  TILEPRO_OPC_SLTI,
  TILEPRO_OPC_SLTI_SN,
  TILEPRO_OPC_SLTI_U,
  TILEPRO_OPC_SLTI_U_SN,
  TILEPRO_OPC_SLTIB,
  TILEPRO_OPC_SLTIB_SN,
  TILEPRO_OPC_SLTIB_U,
  TILEPRO_OPC_SLTIB_U_SN,
  TILEPRO_OPC_SLTIH,
  TILEPRO_OPC_SLTIH_SN,
  TILEPRO_OPC_SLTIH_U,
  TILEPRO_OPC_SLTIH_U_SN,
  TILEPRO_OPC_SNE,
  TILEPRO_OPC_SNE_SN,
  TILEPRO_OPC_SNEB,
  TILEPRO_OPC_SNEB_SN,
  TILEPRO_OPC_SNEH,
  TILEPRO_OPC_SNEH_SN,
  TILEPRO_OPC_SRA,
  TILEPRO_OPC_SRA_SN,
  TILEPRO_OPC_SRAB,
  TILEPRO_OPC_SRAB_SN,
  TILEPRO_OPC_SRAH,
  TILEPRO_OPC_SRAH_SN,
  TILEPRO_OPC_SRAI,
  TILEPRO_OPC_SRAI_SN,
  TILEPRO_OPC_SRAIB,
  TILEPRO_OPC_SRAIB_SN,
  TILEPRO_OPC_SRAIH,
  TILEPRO_OPC_SRAIH_SN,
  TILEPRO_OPC_SUB,
  TILEPRO_OPC_SUB_SN,
  TILEPRO_OPC_SUBB,
  TILEPRO_OPC_SUBB_SN,
  TILEPRO_OPC_SUBBS_U,
  TILEPRO_OPC_SUBBS_U_SN,
  TILEPRO_OPC_SUBH,
  TILEPRO_OPC_SUBH_SN,
  TILEPRO_OPC_SUBHS,
  TILEPRO_OPC_SUBHS_SN,
  TILEPRO_OPC_SUBS,
  TILEPRO_OPC_SUBS_SN,
  TILEPRO_OPC_SW,
  TILEPRO_OPC_SWADD,
  TILEPRO_OPC_SWINT0,
  TILEPRO_OPC_SWINT1,
  TILEPRO_OPC_SWINT2,
  TILEPRO_OPC_SWINT3,
  TILEPRO_OPC_TBLIDXB0,
  TILEPRO_OPC_TBLIDXB0_SN,
  TILEPRO_OPC_TBLIDXB1,
  TILEPRO_OPC_TBLIDXB1_SN,
  TILEPRO_OPC_TBLIDXB2,
  TILEPRO_OPC_TBLIDXB2_SN,
  TILEPRO_OPC_TBLIDXB3,
  TILEPRO_OPC_TBLIDXB3_SN,
  TILEPRO_OPC_TNS,
  TILEPRO_OPC_TNS_SN,
  TILEPRO_OPC_WH64,
  TILEPRO_OPC_XOR,
  TILEPRO_OPC_XOR_SN,
  TILEPRO_OPC_XORI,
  TILEPRO_OPC_XORI_SN,
  TILEPRO_OPC_NONE
} tilepro_mnemonic;

/* 64-bit pattern for a { bpt ; nop } bundle. */
#define TILEPRO_BPT_BUNDLE 0x400b3cae70166000ULL

#ifndef DISASM_ONLY

enum
{
  TILEPRO_SN_MAX_OPERANDS = 6 /* route */
};

typedef enum
{
  TILEPRO_SN_OPC_BZ,
  TILEPRO_SN_OPC_BNZ,
  TILEPRO_SN_OPC_JRR,
  TILEPRO_SN_OPC_FNOP,
  TILEPRO_SN_OPC_BLZ,
  TILEPRO_SN_OPC_NOP,
  TILEPRO_SN_OPC_MOVEI,
  TILEPRO_SN_OPC_MOVE,
  TILEPRO_SN_OPC_BGEZ,
  TILEPRO_SN_OPC_JR,
  TILEPRO_SN_OPC_BLEZ,
  TILEPRO_SN_OPC_BBNS,
  TILEPRO_SN_OPC_JALRR,
  TILEPRO_SN_OPC_BPT,
  TILEPRO_SN_OPC_JALR,
  TILEPRO_SN_OPC_SHR1,
  TILEPRO_SN_OPC_BGZ,
  TILEPRO_SN_OPC_BBS,
  TILEPRO_SN_OPC_SHL8II,
  TILEPRO_SN_OPC_ADDI,
  TILEPRO_SN_OPC_HALT,
  TILEPRO_SN_OPC_ROUTE,
  TILEPRO_SN_OPC_NONE
} tilepro_sn_mnemonic;

extern const unsigned char tilepro_sn_route_encode[6 * 6 * 6];
extern const signed char tilepro_sn_route_decode[256][3];
extern const char tilepro_sn_direction_names[6][5];
extern const signed char tilepro_sn_dest_map[6][6];
#endif /* DISASM_ONLY */


static __inline unsigned int
get_BrOff_SN(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 0)) & 0x3ff);
}

static __inline unsigned int
get_BrOff_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 43)) & 0x00007fff) |
         (((unsigned int)(n >> 20)) & 0x00018000);
}

static __inline unsigned int
get_BrType_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 31)) & 0xf);
}

static __inline unsigned int
get_Dest_Imm8_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 31)) & 0x0000003f) |
         (((unsigned int)(n >> 43)) & 0x000000c0);
}

static __inline unsigned int
get_Dest_SN(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 2)) & 0x3);
}

static __inline unsigned int
get_Dest_X0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 0)) & 0x3f);
}

static __inline unsigned int
get_Dest_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 31)) & 0x3f);
}

static __inline unsigned int
get_Dest_Y0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 0)) & 0x3f);
}

static __inline unsigned int
get_Dest_Y1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 31)) & 0x3f);
}

static __inline unsigned int
get_Imm16_X0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 12)) & 0xffff);
}

static __inline unsigned int
get_Imm16_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 43)) & 0xffff);
}

static __inline unsigned int
get_Imm8_SN(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 0)) & 0xff);
}

static __inline unsigned int
get_Imm8_X0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 12)) & 0xff);
}

static __inline unsigned int
get_Imm8_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 43)) & 0xff);
}

static __inline unsigned int
get_Imm8_Y0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 12)) & 0xff);
}

static __inline unsigned int
get_Imm8_Y1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 43)) & 0xff);
}

static __inline unsigned int
get_ImmOpcodeExtension_X0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 20)) & 0x7f);
}

static __inline unsigned int
get_ImmOpcodeExtension_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 51)) & 0x7f);
}

static __inline unsigned int
get_ImmRROpcodeExtension_SN(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 8)) & 0x3);
}

static __inline unsigned int
get_JOffLong_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 43)) & 0x00007fff) |
         (((unsigned int)(n >> 20)) & 0x00018000) |
         (((unsigned int)(n >> 14)) & 0x001e0000) |
         (((unsigned int)(n >> 16)) & 0x07e00000) |
         (((unsigned int)(n >> 31)) & 0x18000000);
}

static __inline unsigned int
get_JOff_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 43)) & 0x00007fff) |
         (((unsigned int)(n >> 20)) & 0x00018000) |
         (((unsigned int)(n >> 14)) & 0x001e0000) |
         (((unsigned int)(n >> 16)) & 0x07e00000) |
         (((unsigned int)(n >> 31)) & 0x08000000);
}

static __inline unsigned int
get_MF_Imm15_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 37)) & 0x00003fff) |
         (((unsigned int)(n >> 44)) & 0x00004000);
}

static __inline unsigned int
get_MMEnd_X0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 18)) & 0x1f);
}

static __inline unsigned int
get_MMEnd_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 49)) & 0x1f);
}

static __inline unsigned int
get_MMStart_X0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 23)) & 0x1f);
}

static __inline unsigned int
get_MMStart_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 54)) & 0x1f);
}

static __inline unsigned int
get_MT_Imm15_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 31)) & 0x0000003f) |
         (((unsigned int)(n >> 37)) & 0x00003fc0) |
         (((unsigned int)(n >> 44)) & 0x00004000);
}

static __inline unsigned int
get_Mode(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 63)) & 0x1);
}

static __inline unsigned int
get_NoRegOpcodeExtension_SN(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 0)) & 0xf);
}

static __inline unsigned int
get_Opcode_SN(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 10)) & 0x3f);
}

static __inline unsigned int
get_Opcode_X0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 28)) & 0x7);
}

static __inline unsigned int
get_Opcode_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 59)) & 0xf);
}

static __inline unsigned int
get_Opcode_Y0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 27)) & 0xf);
}

static __inline unsigned int
get_Opcode_Y1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 59)) & 0xf);
}

static __inline unsigned int
get_Opcode_Y2(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 56)) & 0x7);
}

static __inline unsigned int
get_RROpcodeExtension_SN(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 4)) & 0xf);
}

static __inline unsigned int
get_RRROpcodeExtension_X0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 18)) & 0x1ff);
}

static __inline unsigned int
get_RRROpcodeExtension_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 49)) & 0x1ff);
}

static __inline unsigned int
get_RRROpcodeExtension_Y0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 18)) & 0x3);
}

static __inline unsigned int
get_RRROpcodeExtension_Y1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 49)) & 0x3);
}

static __inline unsigned int
get_RouteOpcodeExtension_SN(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 0)) & 0x3ff);
}

static __inline unsigned int
get_S_X0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 27)) & 0x1);
}

static __inline unsigned int
get_S_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 58)) & 0x1);
}

static __inline unsigned int
get_ShAmt_X0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 12)) & 0x1f);
}

static __inline unsigned int
get_ShAmt_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 43)) & 0x1f);
}

static __inline unsigned int
get_ShAmt_Y0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 12)) & 0x1f);
}

static __inline unsigned int
get_ShAmt_Y1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 43)) & 0x1f);
}

static __inline unsigned int
get_SrcA_X0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 6)) & 0x3f);
}

static __inline unsigned int
get_SrcA_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 37)) & 0x3f);
}

static __inline unsigned int
get_SrcA_Y0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 6)) & 0x3f);
}

static __inline unsigned int
get_SrcA_Y1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 37)) & 0x3f);
}

static __inline unsigned int
get_SrcA_Y2(tilepro_bundle_bits n)
{
  return (((n >> 26)) & 0x00000001) |
         (((unsigned int)(n >> 50)) & 0x0000003e);
}

static __inline unsigned int
get_SrcBDest_Y2(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 20)) & 0x3f);
}

static __inline unsigned int
get_SrcB_X0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 12)) & 0x3f);
}

static __inline unsigned int
get_SrcB_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 43)) & 0x3f);
}

static __inline unsigned int
get_SrcB_Y0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 12)) & 0x3f);
}

static __inline unsigned int
get_SrcB_Y1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 43)) & 0x3f);
}

static __inline unsigned int
get_Src_SN(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 0)) & 0x3);
}

static __inline unsigned int
get_UnOpcodeExtension_X0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 12)) & 0x1f);
}

static __inline unsigned int
get_UnOpcodeExtension_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 43)) & 0x1f);
}

static __inline unsigned int
get_UnOpcodeExtension_Y0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 12)) & 0x1f);
}

static __inline unsigned int
get_UnOpcodeExtension_Y1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 43)) & 0x1f);
}

static __inline unsigned int
get_UnShOpcodeExtension_X0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 17)) & 0x3ff);
}

static __inline unsigned int
get_UnShOpcodeExtension_X1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 48)) & 0x3ff);
}

static __inline unsigned int
get_UnShOpcodeExtension_Y0(tilepro_bundle_bits num)
{
  const unsigned int n = (unsigned int)num;
  return (((n >> 17)) & 0x7);
}

static __inline unsigned int
get_UnShOpcodeExtension_Y1(tilepro_bundle_bits n)
{
  return (((unsigned int)(n >> 48)) & 0x7);
}


static __inline int
sign_extend(int n, int num_bits)
{
  int shift = (int)(sizeof(int) * 8 - num_bits);
  return (n << shift) >> shift;
}



static __inline tilepro_bundle_bits
create_BrOff_SN(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x3ff) << 0);
}

static __inline tilepro_bundle_bits
create_BrOff_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x00007fff)) << 43) |
         (((tilepro_bundle_bits)(n & 0x00018000)) << 20);
}

static __inline tilepro_bundle_bits
create_BrType_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0xf)) << 31);
}

static __inline tilepro_bundle_bits
create_Dest_Imm8_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x0000003f)) << 31) |
         (((tilepro_bundle_bits)(n & 0x000000c0)) << 43);
}

static __inline tilepro_bundle_bits
create_Dest_SN(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x3) << 2);
}

static __inline tilepro_bundle_bits
create_Dest_X0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x3f) << 0);
}

static __inline tilepro_bundle_bits
create_Dest_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x3f)) << 31);
}

static __inline tilepro_bundle_bits
create_Dest_Y0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x3f) << 0);
}

static __inline tilepro_bundle_bits
create_Dest_Y1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x3f)) << 31);
}

static __inline tilepro_bundle_bits
create_Imm16_X0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0xffff) << 12);
}

static __inline tilepro_bundle_bits
create_Imm16_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0xffff)) << 43);
}

static __inline tilepro_bundle_bits
create_Imm8_SN(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0xff) << 0);
}

static __inline tilepro_bundle_bits
create_Imm8_X0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0xff) << 12);
}

static __inline tilepro_bundle_bits
create_Imm8_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0xff)) << 43);
}

static __inline tilepro_bundle_bits
create_Imm8_Y0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0xff) << 12);
}

static __inline tilepro_bundle_bits
create_Imm8_Y1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0xff)) << 43);
}

static __inline tilepro_bundle_bits
create_ImmOpcodeExtension_X0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x7f) << 20);
}

static __inline tilepro_bundle_bits
create_ImmOpcodeExtension_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x7f)) << 51);
}

static __inline tilepro_bundle_bits
create_ImmRROpcodeExtension_SN(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x3) << 8);
}

static __inline tilepro_bundle_bits
create_JOffLong_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x00007fff)) << 43) |
         (((tilepro_bundle_bits)(n & 0x00018000)) << 20) |
         (((tilepro_bundle_bits)(n & 0x001e0000)) << 14) |
         (((tilepro_bundle_bits)(n & 0x07e00000)) << 16) |
         (((tilepro_bundle_bits)(n & 0x18000000)) << 31);
}

static __inline tilepro_bundle_bits
create_JOff_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x00007fff)) << 43) |
         (((tilepro_bundle_bits)(n & 0x00018000)) << 20) |
         (((tilepro_bundle_bits)(n & 0x001e0000)) << 14) |
         (((tilepro_bundle_bits)(n & 0x07e00000)) << 16) |
         (((tilepro_bundle_bits)(n & 0x08000000)) << 31);
}

static __inline tilepro_bundle_bits
create_MF_Imm15_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x00003fff)) << 37) |
         (((tilepro_bundle_bits)(n & 0x00004000)) << 44);
}

static __inline tilepro_bundle_bits
create_MMEnd_X0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x1f) << 18);
}

static __inline tilepro_bundle_bits
create_MMEnd_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x1f)) << 49);
}

static __inline tilepro_bundle_bits
create_MMStart_X0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x1f) << 23);
}

static __inline tilepro_bundle_bits
create_MMStart_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x1f)) << 54);
}

static __inline tilepro_bundle_bits
create_MT_Imm15_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x0000003f)) << 31) |
         (((tilepro_bundle_bits)(n & 0x00003fc0)) << 37) |
         (((tilepro_bundle_bits)(n & 0x00004000)) << 44);
}

static __inline tilepro_bundle_bits
create_Mode(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x1)) << 63);
}

static __inline tilepro_bundle_bits
create_NoRegOpcodeExtension_SN(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0xf) << 0);
}

static __inline tilepro_bundle_bits
create_Opcode_SN(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x3f) << 10);
}

static __inline tilepro_bundle_bits
create_Opcode_X0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x7) << 28);
}

static __inline tilepro_bundle_bits
create_Opcode_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0xf)) << 59);
}

static __inline tilepro_bundle_bits
create_Opcode_Y0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0xf) << 27);
}

static __inline tilepro_bundle_bits
create_Opcode_Y1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0xf)) << 59);
}

static __inline tilepro_bundle_bits
create_Opcode_Y2(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x7)) << 56);
}

static __inline tilepro_bundle_bits
create_RROpcodeExtension_SN(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0xf) << 4);
}

static __inline tilepro_bundle_bits
create_RRROpcodeExtension_X0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x1ff) << 18);
}

static __inline tilepro_bundle_bits
create_RRROpcodeExtension_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x1ff)) << 49);
}

static __inline tilepro_bundle_bits
create_RRROpcodeExtension_Y0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x3) << 18);
}

static __inline tilepro_bundle_bits
create_RRROpcodeExtension_Y1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x3)) << 49);
}

static __inline tilepro_bundle_bits
create_RouteOpcodeExtension_SN(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x3ff) << 0);
}

static __inline tilepro_bundle_bits
create_S_X0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x1) << 27);
}

static __inline tilepro_bundle_bits
create_S_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x1)) << 58);
}

static __inline tilepro_bundle_bits
create_ShAmt_X0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x1f) << 12);
}

static __inline tilepro_bundle_bits
create_ShAmt_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x1f)) << 43);
}

static __inline tilepro_bundle_bits
create_ShAmt_Y0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x1f) << 12);
}

static __inline tilepro_bundle_bits
create_ShAmt_Y1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x1f)) << 43);
}

static __inline tilepro_bundle_bits
create_SrcA_X0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x3f) << 6);
}

static __inline tilepro_bundle_bits
create_SrcA_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x3f)) << 37);
}

static __inline tilepro_bundle_bits
create_SrcA_Y0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x3f) << 6);
}

static __inline tilepro_bundle_bits
create_SrcA_Y1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x3f)) << 37);
}

static __inline tilepro_bundle_bits
create_SrcA_Y2(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x00000001) << 26) |
         (((tilepro_bundle_bits)(n & 0x0000003e)) << 50);
}

static __inline tilepro_bundle_bits
create_SrcBDest_Y2(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x3f) << 20);
}

static __inline tilepro_bundle_bits
create_SrcB_X0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x3f) << 12);
}

static __inline tilepro_bundle_bits
create_SrcB_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x3f)) << 43);
}

static __inline tilepro_bundle_bits
create_SrcB_Y0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x3f) << 12);
}

static __inline tilepro_bundle_bits
create_SrcB_Y1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x3f)) << 43);
}

static __inline tilepro_bundle_bits
create_Src_SN(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x3) << 0);
}

static __inline tilepro_bundle_bits
create_UnOpcodeExtension_X0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x1f) << 12);
}

static __inline tilepro_bundle_bits
create_UnOpcodeExtension_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x1f)) << 43);
}

static __inline tilepro_bundle_bits
create_UnOpcodeExtension_Y0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x1f) << 12);
}

static __inline tilepro_bundle_bits
create_UnOpcodeExtension_Y1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x1f)) << 43);
}

static __inline tilepro_bundle_bits
create_UnShOpcodeExtension_X0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x3ff) << 17);
}

static __inline tilepro_bundle_bits
create_UnShOpcodeExtension_X1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x3ff)) << 48);
}

static __inline tilepro_bundle_bits
create_UnShOpcodeExtension_Y0(int num)
{
  const unsigned int n = (unsigned int)num;
  return ((n & 0x7) << 17);
}

static __inline tilepro_bundle_bits
create_UnShOpcodeExtension_Y1(int num)
{
  const unsigned int n = (unsigned int)num;
  return (((tilepro_bundle_bits)(n & 0x7)) << 48);
}



typedef enum
{
  TILEPRO_PIPELINE_X0,
  TILEPRO_PIPELINE_X1,
  TILEPRO_PIPELINE_Y0,
  TILEPRO_PIPELINE_Y1,
  TILEPRO_PIPELINE_Y2,
  TILEPRO_NUM_PIPELINE_ENCODINGS
} tilepro_pipeline;

#define tilepro_is_x_pipeline(p) ((int)(p) <= (int)TILEPRO_PIPELINE_X1)

typedef enum
{
  TILEPRO_OP_TYPE_REGISTER,
  TILEPRO_OP_TYPE_IMMEDIATE,
  TILEPRO_OP_TYPE_ADDRESS,
  TILEPRO_OP_TYPE_SPR
} tilepro_operand_type;

/* This is the bit that determines if a bundle is in the Y encoding. */
#define TILEPRO_BUNDLE_Y_ENCODING_MASK ((tilepro_bundle_bits)1 << 63)

enum
{
  /* Maximum number of instructions in a bundle (2 for X, 3 for Y). */
  TILEPRO_MAX_INSTRUCTIONS_PER_BUNDLE = 3,

  /* Log base 2 of TILEPRO_BUNDLE_SIZE_IN_BYTES. */
  TILEPRO_LOG2_BUNDLE_SIZE_IN_BYTES = 3,

  /* Instructions take this many bytes. */
  TILEPRO_BUNDLE_SIZE_IN_BYTES = 1 << TILEPRO_LOG2_BUNDLE_SIZE_IN_BYTES,

  /* Log base 2 of TILEPRO_BUNDLE_ALIGNMENT_IN_BYTES. */
  TILEPRO_LOG2_BUNDLE_ALIGNMENT_IN_BYTES = 3,

  /* Bundles should be aligned modulo this number of bytes. */
  TILEPRO_BUNDLE_ALIGNMENT_IN_BYTES =
    (1 << TILEPRO_LOG2_BUNDLE_ALIGNMENT_IN_BYTES),

  /* Log base 2 of TILEPRO_SN_INSTRUCTION_SIZE_IN_BYTES. */
  TILEPRO_LOG2_SN_INSTRUCTION_SIZE_IN_BYTES = 1,

  /* Static network instructions take this many bytes. */
  TILEPRO_SN_INSTRUCTION_SIZE_IN_BYTES =
    (1 << TILEPRO_LOG2_SN_INSTRUCTION_SIZE_IN_BYTES),

  /* Number of registers (some are magic, such as network I/O). */
  TILEPRO_NUM_REGISTERS = 64,

  /* Number of static network registers. */
  TILEPRO_NUM_SN_REGISTERS = 4
};


struct tilepro_operand
{
  /* Is this operand a register, immediate or address? */
  tilepro_operand_type type;

  /* The default relocation type for this operand.  */
  signed int default_reloc : 16;

  /* How many bits is this value? (used for range checking) */
  unsigned int num_bits : 5;

  /* Is the value signed? (used for range checking) */
  unsigned int is_signed : 1;

  /* Is this operand a source register? */
  unsigned int is_src_reg : 1;

  /* Is this operand written? (i.e. is it a destination register) */
  unsigned int is_dest_reg : 1;

  /* Is this operand PC-relative? */
  unsigned int is_pc_relative : 1;

  /* By how many bits do we right shift the value before inserting? */
  unsigned int rightshift : 2;

  /* Return the bits for this operand to be ORed into an existing bundle. */
  tilepro_bundle_bits (*insert) (int op);

  /* Extract this operand and return it. */
  unsigned int (*extract) (tilepro_bundle_bits bundle);
};


extern const struct tilepro_operand tilepro_operands[];

/* One finite-state machine per pipe for rapid instruction decoding. */
extern const unsigned short * const
tilepro_bundle_decoder_fsms[TILEPRO_NUM_PIPELINE_ENCODINGS];


struct tilepro_opcode
{
  /* The opcode mnemonic, e.g. "add" */
  const char *name;

  /* The enum value for this mnemonic. */
  tilepro_mnemonic mnemonic;

  /* A bit mask of which of the five pipes this instruction
     is compatible with:
     X0  0x01
     X1  0x02
     Y0  0x04
     Y1  0x08
     Y2  0x10 */
  unsigned char pipes;

  /* How many operands are there? */
  unsigned char num_operands;

  /* Which register does this write implicitly, or TREG_ZERO if none? */
  unsigned char implicitly_written_register;

  /* Can this be bundled with other instructions (almost always true). */
  unsigned char can_bundle;

  /* The description of the operands. Each of these is an
   * index into the tilepro_operands[] table. */
  unsigned char operands[TILEPRO_NUM_PIPELINE_ENCODINGS][TILEPRO_MAX_OPERANDS];

#if !defined(__KERNEL__) && !defined(_LIBC)
  /* A mask of which bits have predefined values for each pipeline.
   * This is useful for disassembly. */
  tilepro_bundle_bits fixed_bit_masks[TILEPRO_NUM_PIPELINE_ENCODINGS];

  /* For each bit set in fixed_bit_masks, what the value is for this
   * instruction. */
  tilepro_bundle_bits fixed_bit_values[TILEPRO_NUM_PIPELINE_ENCODINGS];
#endif
};

extern const struct tilepro_opcode tilepro_opcodes[];

#if !defined(__KERNEL__) && !defined(_LIBC)

typedef unsigned short tilepro_sn_instruction_bits;

struct tilepro_sn_opcode
{
  /* The opcode mnemonic, e.g. "add" */
  const char *name;

  /* The enum value for this mnemonic. */
  tilepro_sn_mnemonic mnemonic;

  /* How many operands are there? */
  unsigned char num_operands;

  /* The description of the operands. Each of these is an
   * index into the tilepro_operands[] table. */
  unsigned char operands[TILEPRO_SN_MAX_OPERANDS];

  /* A mask of which bits have predefined values.
   * This is useful for disassembly. */
  tilepro_sn_instruction_bits fixed_bit_mask;

  /* For each bit set in fixed_bit_masks, what its value is. */
  tilepro_sn_instruction_bits fixed_bit_values;
};

extern const struct tilepro_sn_opcode tilepro_sn_opcodes[];

#endif /* !__KERNEL__ && !_LIBC */

/* Used for non-textual disassembly into structs. */
struct tilepro_decoded_instruction
{
  const struct tilepro_opcode *opcode;
  const struct tilepro_operand *operands[TILEPRO_MAX_OPERANDS];
  int operand_values[TILEPRO_MAX_OPERANDS];
};


/* Disassemble a bundle into a struct for machine processing. */
extern int parse_insn_tilepro(tilepro_bundle_bits bits,
                              unsigned int pc,
                              struct tilepro_decoded_instruction
                              decoded[TILEPRO_MAX_INSTRUCTIONS_PER_BUNDLE]);


/* Given a set of bundle bits and a specific pipe, returns which
 * instruction the bundle contains in that pipe.
 */
extern const struct tilepro_opcode *
find_opcode(tilepro_bundle_bits bits, tilepro_pipeline pipe);


#if !defined(__KERNEL__) && !defined(_LIBC)
/* Canonical names of all the registers. */
/* ISSUE: This table lives in "tilepro-dis.c" */
extern const char * const tilepro_register_names[];

/* Descriptor for a special-purpose register. */
struct tilepro_spr
{
  /* The number */
  int number;

  /* The name */
  const char *name;
};

/* List of all the SPRs; ordered by increasing number. */
extern const struct tilepro_spr tilepro_sprs[];

/* Number of special-purpose registers. */
extern const int tilepro_num_sprs;

extern const char *
get_tilepro_spr_name (int num);
#endif /* !__KERNEL__ && !_LIBC */

/* Make a few "tile_" variables to simply common code between
   architectures.  */

typedef tilepro_bundle_bits tile_bundle_bits;
#define TILE_BUNDLE_SIZE_IN_BYTES TILEPRO_BUNDLE_SIZE_IN_BYTES
#define TILE_BUNDLE_ALIGNMENT_IN_BYTES TILEPRO_BUNDLE_ALIGNMENT_IN_BYTES
#define TILE_LOG2_BUNDLE_ALIGNMENT_IN_BYTES \
  TILEPRO_LOG2_BUNDLE_ALIGNMENT_IN_BYTES

#endif /* opcode_tilepro_h */
