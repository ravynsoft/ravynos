/*
 * Copyright 2015 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "ac_gpu_info.h"
#include "ac_perfcounter.h"
#include "ac_spm.h"

#include "util/u_memory.h"
#include "util/macros.h"

/* cik_CB */
static unsigned cik_CB_select0[] = {
   R_037004_CB_PERFCOUNTER0_SELECT,
   R_03700C_CB_PERFCOUNTER1_SELECT,
   R_037010_CB_PERFCOUNTER2_SELECT,
   R_037014_CB_PERFCOUNTER3_SELECT,
};
static unsigned cik_CB_select1[] = {
   R_037008_CB_PERFCOUNTER0_SELECT1,
};
static struct ac_pc_block_base cik_CB = {
   .gpu_block = CB,
   .name = "CB",
   .num_counters = 4,
   .flags = AC_PC_BLOCK_SE | AC_PC_BLOCK_INSTANCE_GROUPS,

   .select0 = cik_CB_select0,
   .select1 = cik_CB_select1,
   .counter0_lo = R_035018_CB_PERFCOUNTER0_LO,

   .num_spm_counters = 1,
   .num_spm_wires = 2,
   .spm_block_select = AC_SPM_SE_BLOCK_CB,
};

/* cik_CPC */
static unsigned cik_CPC_select0[] = {
   R_036024_CPC_PERFCOUNTER0_SELECT,
   R_03600C_CPC_PERFCOUNTER1_SELECT,
};
static unsigned cik_CPC_select1[] = {
   R_036010_CPC_PERFCOUNTER0_SELECT1,
};
static unsigned cik_CPC_counters[] = {
   R_034018_CPC_PERFCOUNTER0_LO,
   R_034010_CPC_PERFCOUNTER1_LO,
};
static struct ac_pc_block_base cik_CPC = {
   .gpu_block = CPC,
   .name = "CPC",
   .num_counters = 2,

   .select0 = cik_CPC_select0,
   .select1 = cik_CPC_select1,
   .counters = cik_CPC_counters,

   .num_spm_counters = 1,
   .num_spm_wires = 2,
   .spm_block_select = AC_SPM_GLOBAL_BLOCK_CPC,
};

/* cik_CPF */
static unsigned cik_CPF_select0[] = {
   R_03601C_CPF_PERFCOUNTER0_SELECT,
   R_036014_CPF_PERFCOUNTER1_SELECT,
};
static unsigned cik_CPF_select1[] = {
   R_036018_CPF_PERFCOUNTER0_SELECT1,
};
static unsigned cik_CPF_counters[] = {
   R_034028_CPF_PERFCOUNTER0_LO,
   R_034020_CPF_PERFCOUNTER1_LO,
};
static struct ac_pc_block_base cik_CPF = {
   .gpu_block = CPF,
   .name = "CPF",
   .num_counters = 2,

   .select0 = cik_CPF_select0,
   .select1 = cik_CPF_select1,
   .counters = cik_CPF_counters,

   .num_spm_counters = 1,
   .num_spm_wires = 2,
   .spm_block_select = AC_SPM_GLOBAL_BLOCK_CPF,
};

/* cik_CPG */
static unsigned cik_CPG_select0[] = {
   R_036008_CPG_PERFCOUNTER0_SELECT,
   R_036000_CPG_PERFCOUNTER1_SELECT,
};
static unsigned cik_CPG_select1[] = {
   R_036004_CPG_PERFCOUNTER0_SELECT1
};
static unsigned cik_CPG_counters[] = {
   R_034008_CPG_PERFCOUNTER0_LO,
   R_034000_CPG_PERFCOUNTER1_LO,
};
static struct ac_pc_block_base cik_CPG = {
   .gpu_block = CPG,
   .name = "CPG",
   .num_counters = 2,

   .select0 = cik_CPG_select0,
   .select1 = cik_CPG_select1,
   .counters = cik_CPG_counters,

   .num_spm_counters = 1,
   .num_spm_wires = 2,
   .spm_block_select = AC_SPM_GLOBAL_BLOCK_CPG,
};

/* cik_DB */
static unsigned cik_DB_select0[] = {
   R_037100_DB_PERFCOUNTER0_SELECT,
   R_037108_DB_PERFCOUNTER1_SELECT,
   R_037110_DB_PERFCOUNTER2_SELECT,
   R_037118_DB_PERFCOUNTER3_SELECT,
};
static unsigned cik_DB_select1[] = {
   R_037104_DB_PERFCOUNTER0_SELECT1,
   R_03710C_DB_PERFCOUNTER1_SELECT1,
};
static struct ac_pc_block_base cik_DB = {
   .gpu_block = DB,
   .name = "DB",
   .num_counters = 4,
   .flags = AC_PC_BLOCK_SE | AC_PC_BLOCK_INSTANCE_GROUPS,

   .select0 = cik_DB_select0,
   .select1 = cik_DB_select1,
   .counter0_lo = R_035100_DB_PERFCOUNTER0_LO,

   .num_spm_counters = 2,
   .num_spm_wires = 3,
};

/* cik_GDS */
static unsigned cik_GDS_select0[] = {
   R_036A00_GDS_PERFCOUNTER0_SELECT,
   R_036A04_GDS_PERFCOUNTER1_SELECT,
   R_036A08_GDS_PERFCOUNTER2_SELECT,
   R_036A0C_GDS_PERFCOUNTER3_SELECT,
};
static unsigned cik_GDS_select1[] = {
   R_036A10_GDS_PERFCOUNTER0_SELECT1,
};
static struct ac_pc_block_base cik_GDS = {
   .gpu_block = GDS,
   .name = "GDS",
   .num_counters = 4,

   .select0 = cik_GDS_select0,
   .select1 = cik_GDS_select1,
   .counter0_lo = R_034A00_GDS_PERFCOUNTER0_LO,

   .num_spm_counters = 1,
   .num_spm_wires = 2,
   .spm_block_select = AC_SPM_GLOBAL_BLOCK_GDS,
};

/* cik_GRBM */
static unsigned cik_GRBM_select0[] = {
   R_036100_GRBM_PERFCOUNTER0_SELECT,
   R_036104_GRBM_PERFCOUNTER1_SELECT,
};
static unsigned cik_GRBM_counters[] = {
   R_034100_GRBM_PERFCOUNTER0_LO,
   R_03410C_GRBM_PERFCOUNTER1_LO,
};
static struct ac_pc_block_base cik_GRBM = {
   .gpu_block = GRBM,
   .name = "GRBM",
   .num_counters = 2,

   .select0 = cik_GRBM_select0,
   .counters = cik_GRBM_counters,
};

/* cik_GRBMSE */
static unsigned cik_GRBMSE_select0[] = {
   R_036108_GRBM_SE0_PERFCOUNTER_SELECT,
   R_03610C_GRBM_SE1_PERFCOUNTER_SELECT,
   R_036110_GRBM_SE2_PERFCOUNTER_SELECT,
   R_036114_GRBM_SE3_PERFCOUNTER_SELECT,
};
static struct ac_pc_block_base cik_GRBMSE = {
   .gpu_block = GRBMSE,
   .name = "GRBMSE",
   .num_counters = 4,

   .select0 = cik_GRBMSE_select0,
   .counter0_lo = R_034114_GRBM_SE0_PERFCOUNTER_LO,
};

/* cik_IA */
static unsigned cik_IA_select0[] = {
   R_036210_IA_PERFCOUNTER0_SELECT,
   R_036214_IA_PERFCOUNTER1_SELECT,
   R_036218_IA_PERFCOUNTER2_SELECT,
   R_03621C_IA_PERFCOUNTER3_SELECT,
};
static unsigned cik_IA_select1[] = {
   R_036220_IA_PERFCOUNTER0_SELECT1,
};
static struct ac_pc_block_base cik_IA = {
   .gpu_block = IA,
   .name = "IA",
   .num_counters = 4,

   .select0 = cik_IA_select0,
   .select1 = cik_IA_select1,
   .counter0_lo = R_034220_IA_PERFCOUNTER0_LO,

   .num_spm_counters = 1,
   .num_spm_wires = 2,
};

/* cik_PA_SC */
static unsigned cik_PA_SC_select0[] = {
   R_036500_PA_SC_PERFCOUNTER0_SELECT,
   R_036508_PA_SC_PERFCOUNTER1_SELECT,
   R_03650C_PA_SC_PERFCOUNTER2_SELECT,
   R_036510_PA_SC_PERFCOUNTER3_SELECT,
   R_036514_PA_SC_PERFCOUNTER4_SELECT,
   R_036518_PA_SC_PERFCOUNTER5_SELECT,
   R_03651C_PA_SC_PERFCOUNTER6_SELECT,
   R_036520_PA_SC_PERFCOUNTER7_SELECT,
};
static unsigned cik_PA_SC_select1[] = {
   R_036504_PA_SC_PERFCOUNTER0_SELECT1,
};
static struct ac_pc_block_base cik_PA_SC = {
   .gpu_block = PA_SC,
   .name = "PA_SC",
   .num_counters = 8,
   .flags = AC_PC_BLOCK_SE,

   .select0 = cik_PA_SC_select0,
   .select1 = cik_PA_SC_select1,
   .counter0_lo = R_034500_PA_SC_PERFCOUNTER0_LO,

   .num_spm_counters = 1,
   .num_spm_wires = 2,
   .spm_block_select = AC_SPM_SE_BLOCK_SC,
};

/* cik_PA_SU */
static unsigned cik_PA_SU_select0[] = {
   R_036400_PA_SU_PERFCOUNTER0_SELECT,
   R_036408_PA_SU_PERFCOUNTER1_SELECT,
   R_036410_PA_SU_PERFCOUNTER2_SELECT,
   R_036414_PA_SU_PERFCOUNTER3_SELECT,
};
static unsigned cik_PA_SU_select1[] = {
   R_036404_PA_SU_PERFCOUNTER0_SELECT1,
   R_03640C_PA_SU_PERFCOUNTER1_SELECT1,
};
/* According to docs, PA_SU counters are only 48 bits wide. */
static struct ac_pc_block_base cik_PA_SU = {
   .gpu_block = PA_SU,
   .name = "PA_SU",
   .num_counters = 4,
   .flags = AC_PC_BLOCK_SE,

   .select0 = cik_PA_SU_select0,
   .select1 = cik_PA_SU_select1,
   .counter0_lo = R_034400_PA_SU_PERFCOUNTER0_LO,

   .num_spm_counters = 2,
   .num_spm_wires = 3,
};

/* cik_SPI */
static unsigned cik_SPI_select0[] = {
   R_036600_SPI_PERFCOUNTER0_SELECT,
   R_036604_SPI_PERFCOUNTER1_SELECT,
   R_036608_SPI_PERFCOUNTER2_SELECT,
   R_03660C_SPI_PERFCOUNTER3_SELECT,
   R_036620_SPI_PERFCOUNTER4_SELECT,
   R_036624_SPI_PERFCOUNTER5_SELECT,
};
static unsigned cik_SPI_select1[] = {
   R_036610_SPI_PERFCOUNTER0_SELECT1,
   R_036614_SPI_PERFCOUNTER1_SELECT1,
   R_036618_SPI_PERFCOUNTER2_SELECT1,
   R_03661C_SPI_PERFCOUNTER3_SELECT1
};
static struct ac_pc_block_base cik_SPI = {
   .gpu_block = SPI,
   .name = "SPI",
   .num_counters = 6,
   .flags = AC_PC_BLOCK_SE,

   .select0 = cik_SPI_select0,
   .select1 = cik_SPI_select1,
   .counter0_lo = R_034604_SPI_PERFCOUNTER0_LO,

   .num_spm_counters = 4,
   .num_spm_wires = 8,
   .spm_block_select = AC_SPM_SE_BLOCK_SPI,
};

/* cik_SQ */
static unsigned cik_SQ_select0[] = {
   R_036700_SQ_PERFCOUNTER0_SELECT,
   R_036704_SQ_PERFCOUNTER1_SELECT,
   R_036708_SQ_PERFCOUNTER2_SELECT,
   R_03670C_SQ_PERFCOUNTER3_SELECT,
   R_036710_SQ_PERFCOUNTER4_SELECT,
   R_036714_SQ_PERFCOUNTER5_SELECT,
   R_036718_SQ_PERFCOUNTER6_SELECT,
   R_03671C_SQ_PERFCOUNTER7_SELECT,
   R_036720_SQ_PERFCOUNTER8_SELECT,
   R_036724_SQ_PERFCOUNTER9_SELECT,
   R_036728_SQ_PERFCOUNTER10_SELECT,
   R_03672C_SQ_PERFCOUNTER11_SELECT,
   R_036730_SQ_PERFCOUNTER12_SELECT,
   R_036734_SQ_PERFCOUNTER13_SELECT,
   R_036738_SQ_PERFCOUNTER14_SELECT,
   R_03673C_SQ_PERFCOUNTER15_SELECT,
};
static struct ac_pc_block_base cik_SQ = {
   .gpu_block = SQ,
   .name = "SQ",
   .num_counters = 16,
   .flags = AC_PC_BLOCK_SE | AC_PC_BLOCK_SHADER,

   .select0 = cik_SQ_select0,
   .select_or = S_036700_SQC_BANK_MASK(15) | S_036700_SQC_CLIENT_MASK(15) | S_036700_SIMD_MASK(15),
   .counter0_lo = R_034700_SQ_PERFCOUNTER0_LO,

   .num_spm_wires = 8,
};

/* cik_SX */
static unsigned cik_SX_select0[] = {
   R_036900_SX_PERFCOUNTER0_SELECT,
   R_036904_SX_PERFCOUNTER1_SELECT,
   R_036908_SX_PERFCOUNTER2_SELECT,
   R_03690C_SX_PERFCOUNTER3_SELECT,
};
static unsigned cik_SX_select1[] = {
   R_036910_SX_PERFCOUNTER0_SELECT1,
   R_036914_SX_PERFCOUNTER1_SELECT1,
};
static struct ac_pc_block_base cik_SX = {
   .gpu_block = SX,
   .name = "SX",
   .num_counters = 4,
   .flags = AC_PC_BLOCK_SE,

   .select0 = cik_SX_select0,
   .select1 = cik_SX_select1,
   .counter0_lo = R_034900_SX_PERFCOUNTER0_LO,

   .num_spm_counters = 2,
   .num_spm_wires = 4,
   .spm_block_select = AC_SPM_SE_BLOCK_SX,
};

/* cik_TA */
static unsigned cik_TA_select0[] = {
   R_036B00_TA_PERFCOUNTER0_SELECT,
   R_036B08_TA_PERFCOUNTER1_SELECT,
};
static unsigned cik_TA_select1[] = {
   R_036B04_TA_PERFCOUNTER0_SELECT1,
};
static struct ac_pc_block_base cik_TA = {
   .gpu_block = TA,
   .name = "TA",
   .num_counters = 2,
   .flags = AC_PC_BLOCK_SE | AC_PC_BLOCK_INSTANCE_GROUPS | AC_PC_BLOCK_SHADER_WINDOWED,

   .select0 = cik_TA_select0,
   .select1 = cik_TA_select1,
   .counter0_lo = R_034B00_TA_PERFCOUNTER0_LO,

   .num_spm_counters = 1,
   .num_spm_wires = 2,
   .spm_block_select = AC_SPM_SE_BLOCK_TA,
};

/* cik_TD */
static unsigned cik_TD_select0[] = {
   R_036C00_TD_PERFCOUNTER0_SELECT,
   R_036C08_TD_PERFCOUNTER1_SELECT,
};
static unsigned cik_TD_select1[] = {
   R_036C04_TD_PERFCOUNTER0_SELECT1,
};
static struct ac_pc_block_base cik_TD = {
   .gpu_block = TD,
   .name = "TD",
   .num_counters = 2,
   .flags = AC_PC_BLOCK_SE | AC_PC_BLOCK_INSTANCE_GROUPS | AC_PC_BLOCK_SHADER_WINDOWED,

   .select0 = cik_TD_select0,
   .select1 = cik_TD_select1,
   .counter0_lo = R_034C00_TD_PERFCOUNTER0_LO,

   .num_spm_counters = 1,
   .num_spm_wires = 2,
   .spm_block_select = AC_SPM_SE_BLOCK_TD,
};

/* cik_TCA */
static unsigned cik_TCA_select0[] = {
   R_036E40_TCA_PERFCOUNTER0_SELECT,
   R_036E48_TCA_PERFCOUNTER1_SELECT,
   R_036E50_TCA_PERFCOUNTER2_SELECT,
   R_036E54_TCA_PERFCOUNTER3_SELECT,
};
static unsigned cik_TCA_select1[] = {
   R_036E44_TCA_PERFCOUNTER0_SELECT1,
   R_036E4C_TCA_PERFCOUNTER1_SELECT1,
};
static struct ac_pc_block_base cik_TCA = {
   .gpu_block = TCA,
   .name = "TCA",
   .num_counters = 4,
   .flags = AC_PC_BLOCK_INSTANCE_GROUPS,

   .select0 = cik_TCA_select0,
   .select1 = cik_TCA_select1,
   .counter0_lo = R_034E40_TCA_PERFCOUNTER0_LO,

   .num_spm_counters = 2,
   .num_spm_wires = 4,
};

/* cik_TCC */
static unsigned cik_TCC_select0[] = {
   R_036E00_TCC_PERFCOUNTER0_SELECT,
   R_036E08_TCC_PERFCOUNTER1_SELECT,
   R_036E10_TCC_PERFCOUNTER2_SELECT,
   R_036E14_TCC_PERFCOUNTER3_SELECT,
};
static unsigned cik_TCC_select1[] = {
   R_036E04_TCC_PERFCOUNTER0_SELECT1,
   R_036E0C_TCC_PERFCOUNTER1_SELECT1,
};
static struct ac_pc_block_base cik_TCC = {
   .gpu_block = TCC,
   .name = "TCC",
   .num_counters = 4,
   .flags = AC_PC_BLOCK_INSTANCE_GROUPS,

   .select0 = cik_TCC_select0,
   .select1 = cik_TCC_select1,
   .counter0_lo = R_034E00_TCC_PERFCOUNTER0_LO,

   .num_spm_counters = 2,
   .num_spm_wires = 4,
};

/* cik_TCP */
static unsigned cik_TCP_select0[] = {
   R_036D00_TCP_PERFCOUNTER0_SELECT,
   R_036D08_TCP_PERFCOUNTER1_SELECT,
   R_036D10_TCP_PERFCOUNTER2_SELECT,
   R_036D14_TCP_PERFCOUNTER3_SELECT,
};
static unsigned cik_TCP_select1[] = {
   R_036D04_TCP_PERFCOUNTER0_SELECT1,
   R_036D0C_TCP_PERFCOUNTER1_SELECT1,
};
static struct ac_pc_block_base cik_TCP = {
   .gpu_block = TCP,
   .name = "TCP",
   .num_counters = 4,
   .flags = AC_PC_BLOCK_SE | AC_PC_BLOCK_INSTANCE_GROUPS | AC_PC_BLOCK_SHADER_WINDOWED,

   .select0 = cik_TCP_select0,
   .select1 = cik_TCP_select1,
   .counter0_lo = R_034D00_TCP_PERFCOUNTER0_LO,

   .num_spm_counters = 2,
   .num_spm_wires = 3,
};

/* cik_VGT */
static unsigned cik_VGT_select0[] = {
   R_036230_VGT_PERFCOUNTER0_SELECT,
   R_036234_VGT_PERFCOUNTER1_SELECT,
   R_036238_VGT_PERFCOUNTER2_SELECT,
   R_03623C_VGT_PERFCOUNTER3_SELECT,
};
static unsigned cik_VGT_select1[] = {
   R_036240_VGT_PERFCOUNTER0_SELECT1,
   R_036244_VGT_PERFCOUNTER1_SELECT1,
};
static struct ac_pc_block_base cik_VGT = {
   .gpu_block = VGT,
   .name = "VGT",
   .num_counters = 4,
   .flags = AC_PC_BLOCK_SE,

   .select0 = cik_VGT_select0,
   .select1 = cik_VGT_select1,
   .counter0_lo = R_034240_VGT_PERFCOUNTER0_LO,

   .num_spm_counters = 2,
   .num_spm_wires = 3,
};

/* cik_WD */
static unsigned cik_WD_select0[] = {
   R_036200_WD_PERFCOUNTER0_SELECT,
   R_036204_WD_PERFCOUNTER1_SELECT,
   R_036208_WD_PERFCOUNTER2_SELECT,
   R_03620C_WD_PERFCOUNTER3_SELECT,
};
static struct ac_pc_block_base cik_WD = {
   .gpu_block = WD,
   .name = "WD",
   .num_counters = 4,

   .select0 = cik_WD_select0,
   .counter0_lo = R_034200_WD_PERFCOUNTER0_LO,
};

/* cik_MC */
static struct ac_pc_block_base cik_MC = {
   .gpu_block = MC,
   .name = "MC",
   .num_counters = 4,
};

/* cik_SRBM */
static struct ac_pc_block_base cik_SRBM = {
   .gpu_block = SRBM,
   .name = "SRBM",
   .num_counters = 2,
};

/* gfx10_CHA */
static unsigned gfx10_CHA_select0[] = {
   R_037780_CHA_PERFCOUNTER0_SELECT,
   R_037788_CHA_PERFCOUNTER1_SELECT,
   R_03778C_CHA_PERFCOUNTER2_SELECT,
   R_037790_CHA_PERFCOUNTER3_SELECT,
};
static unsigned gfx10_CHA_select1[] = {
   R_037784_CHA_PERFCOUNTER0_SELECT1,
};
static struct ac_pc_block_base gfx10_CHA = {
   .gpu_block = CHA,
   .name = "CHA",
   .num_counters = 4,

   .select0 = gfx10_CHA_select0,
   .select1 = gfx10_CHA_select1,
   .counter0_lo = R_035800_CHA_PERFCOUNTER0_LO,

   .num_spm_counters = 1,
   .num_spm_wires = 2,
   .spm_block_select = AC_SPM_GLOBAL_BLOCK_CHA,
};

/* gfx10_CHCG */
static unsigned gfx10_CHCG_select0[] = {
   R_036F18_CHCG_PERFCOUNTER0_SELECT,
   R_036F20_CHCG_PERFCOUNTER1_SELECT,
   R_036F24_CHCG_PERFCOUNTER2_SELECT,
   R_036F28_CHCG_PERFCOUNTER3_SELECT,
};
static unsigned gfx10_CHCG_select1[] = {
   R_036F1C_CHCG_PERFCOUNTER0_SELECT1,
};
static struct ac_pc_block_base gfx10_CHCG = {
   .gpu_block = CHCG,
   .name = "CHCG",
   .num_counters = 4,

   .select0 = gfx10_CHCG_select0,
   .select1 = gfx10_CHCG_select1,
   .counter0_lo = R_034F20_CHCG_PERFCOUNTER0_LO,

   .num_spm_counters = 1,
   .num_spm_wires = 2,
   .spm_block_select = AC_SPM_GLOBAL_BLOCK_CHCG,
};

/* gfx10_CHC */
static unsigned gfx10_CHC_select0[] = {
   R_036F00_CHC_PERFCOUNTER0_SELECT,
   R_036F08_CHC_PERFCOUNTER1_SELECT,
   R_036F0C_CHC_PERFCOUNTER2_SELECT,
   R_036F10_CHC_PERFCOUNTER3_SELECT,
};
static unsigned gfx10_CHC_select1[] = {
   R_036F04_CHC_PERFCOUNTER0_SELECT1,
};
static struct ac_pc_block_base gfx10_CHC = {
   .gpu_block = CHC,
   .name = "CHC",
   .num_counters = 4,

   .select0 = gfx10_CHC_select0,
   .select1 = gfx10_CHC_select1,
   .counter0_lo = R_034F00_CHC_PERFCOUNTER0_LO,

   .num_spm_counters = 1,
   .num_spm_wires = 2,
   .spm_block_select = AC_SPM_GLOBAL_BLOCK_CHC,
};

/* gfx10_DB */
static struct ac_pc_block_base gfx10_DB = {
   .gpu_block = DB,
   .name = "DB",
   .num_counters = 4,
   .flags = AC_PC_BLOCK_SE | AC_PC_BLOCK_INSTANCE_GROUPS,

   .select0 = cik_DB_select0,
   .select1 = cik_DB_select1,
   .counter0_lo = R_035100_DB_PERFCOUNTER0_LO,

   .num_spm_counters = 2,
   .num_spm_wires = 4,
   .spm_block_select = AC_SPM_SE_BLOCK_DB,
};

/* gfx10_GCR */
static unsigned gfx10_GCR_select0[] = {
   R_037580_GCR_PERFCOUNTER0_SELECT,
   R_037588_GCR_PERFCOUNTER1_SELECT,
};
static unsigned gfx10_GCR_select1[] = {
   R_037584_GCR_PERFCOUNTER0_SELECT1,
};
static struct ac_pc_block_base gfx10_GCR = {
   .gpu_block = GCR,
   .name = "GCR",
   .num_counters = 2,

   .select0 = gfx10_GCR_select0,
   .select1 = gfx10_GCR_select1,
   .counter0_lo = R_035480_GCR_PERFCOUNTER0_LO,

   .num_spm_counters = 1,
   .num_spm_wires = 2,
   .spm_block_select = AC_SPM_GLOBAL_BLOCK_GCR,
};

/* gfx10_GE */
static unsigned gfx10_GE_select0[] = {
   R_036200_GE_PERFCOUNTER0_SELECT,
   R_036208_GE_PERFCOUNTER1_SELECT,
   R_036210_GE_PERFCOUNTER2_SELECT,
   R_036218_GE_PERFCOUNTER3_SELECT,
   R_036220_GE_PERFCOUNTER4_SELECT,
   R_036228_GE_PERFCOUNTER5_SELECT,
   R_036230_GE_PERFCOUNTER6_SELECT,
   R_036238_GE_PERFCOUNTER7_SELECT,
   R_036240_GE_PERFCOUNTER8_SELECT,
   R_036248_GE_PERFCOUNTER9_SELECT,
   R_036250_GE_PERFCOUNTER10_SELECT,
   R_036258_GE_PERFCOUNTER11_SELECT,
};
static unsigned gfx10_GE_select1[] = {
   R_036204_GE_PERFCOUNTER0_SELECT1,
   R_03620C_GE_PERFCOUNTER1_SELECT1,
   R_036214_GE_PERFCOUNTER2_SELECT1,
   R_03621C_GE_PERFCOUNTER3_SELECT1,
};
static struct ac_pc_block_base gfx10_GE = {
   .gpu_block = GE,
   .name = "GE",
   .num_counters = 12,

   .select0 = gfx10_GE_select0,
   .select1 = gfx10_GE_select1,
   .counter0_lo = R_034200_GE_PERFCOUNTER0_LO,

   .num_spm_counters = 4,
   .num_spm_wires = 8,
   .spm_block_select = AC_SPM_GLOBAL_BLOCK_GE,
};

/* gfx10_GL1A */
static unsigned gfx10_GL1A_select0[] = {
   R_037700_GL1A_PERFCOUNTER0_SELECT,
   R_037708_GL1A_PERFCOUNTER1_SELECT,
   R_03770C_GL1A_PERFCOUNTER2_SELECT,
   R_037710_GL1A_PERFCOUNTER3_SELECT,
};
static unsigned gfx10_GL1A_select1[] = {
   R_037704_GL1A_PERFCOUNTER0_SELECT1,
};
static struct ac_pc_block_base gfx10_GL1A = {
   .gpu_block = GL1A,
   .name = "GL1A",
   .num_counters = 4,
   .flags = AC_PC_BLOCK_SE | AC_PC_BLOCK_SHADER_WINDOWED,

   .select0 = gfx10_GL1A_select0,
   .select1 = gfx10_GL1A_select1,
   .counter0_lo = R_035700_GL1A_PERFCOUNTER0_LO,

   .num_spm_counters = 1,
   .num_spm_wires = 2,
   .spm_block_select = AC_SPM_SE_BLOCK_GL1A,
};

/* gfx10_GL1C */
static unsigned gfx10_GL1C_select0[] = {
   R_036E80_GL1C_PERFCOUNTER0_SELECT,
   R_036E88_GL1C_PERFCOUNTER1_SELECT,
   R_036E8C_GL1C_PERFCOUNTER2_SELECT,
   R_036E90_GL1C_PERFCOUNTER3_SELECT,
};
static unsigned gfx10_GL1C_select1[] = {
   R_036E84_GL1C_PERFCOUNTER0_SELECT1,
};
static struct ac_pc_block_base gfx10_GL1C = {
   .gpu_block = GL1C,
   .name = "GL1C",
   .num_counters = 4,
   .flags = AC_PC_BLOCK_SE | AC_PC_BLOCK_SHADER_WINDOWED,

   .select0 = gfx10_GL1C_select0,
   .select1 = gfx10_GL1C_select1,
   .counter0_lo = R_034E80_GL1C_PERFCOUNTER0_LO,

   .num_spm_counters = 1,
   .num_spm_wires = 2,
   .spm_block_select = AC_SPM_SE_BLOCK_GL1C,
};

/* gfx10_GL2A */
static unsigned gfx10_GL2A_select0[] = {
   R_036E40_GL2A_PERFCOUNTER0_SELECT,
   R_036E48_GL2A_PERFCOUNTER1_SELECT,
   R_036E50_GL2A_PERFCOUNTER2_SELECT,
   R_036E54_GL2A_PERFCOUNTER3_SELECT,
};
static unsigned gfx10_GL2A_select1[] = {
   R_036E44_GL2A_PERFCOUNTER0_SELECT1,
   R_036E4C_GL2A_PERFCOUNTER1_SELECT1,
};
static struct ac_pc_block_base gfx10_GL2A = {
   .gpu_block = GL2A,
   .name = "GL2A",
   .num_counters = 4,

   .select0 = gfx10_GL2A_select0,
   .select1 = gfx10_GL2A_select1,
   .counter0_lo = R_034E40_GL2A_PERFCOUNTER0_LO,

   .num_spm_counters = 2,
   .num_spm_wires = 4,
   .spm_block_select = AC_SPM_GLOBAL_BLOCK_GL2A,
};

/* gfx10_GL2C */
static unsigned gfx10_GL2C_select0[] = {
   R_036E00_GL2C_PERFCOUNTER0_SELECT,
   R_036E08_GL2C_PERFCOUNTER1_SELECT,
   R_036E10_GL2C_PERFCOUNTER2_SELECT,
   R_036E14_GL2C_PERFCOUNTER3_SELECT,
};
static unsigned gfx10_GL2C_select1[] = {
   R_036E04_GL2C_PERFCOUNTER0_SELECT1,
   R_036E0C_GL2C_PERFCOUNTER1_SELECT1,
};
static struct ac_pc_block_base gfx10_GL2C = {
   .gpu_block = GL2C,
   .name = "GL2C",
   .num_counters = 4,

   .select0 = gfx10_GL2C_select0,
   .select1 = gfx10_GL2C_select1,
   .counter0_lo = R_034E00_GL2C_PERFCOUNTER0_LO,

   .num_spm_counters = 2,
   .num_spm_wires = 4,
   .spm_block_select = AC_SPM_GLOBAL_BLOCK_GL2C,
};

/* gfx10_PA_PH */
static unsigned gfx10_PA_PH_select0[] = {
   R_037600_PA_PH_PERFCOUNTER0_SELECT,
   R_037608_PA_PH_PERFCOUNTER1_SELECT,
   R_03760C_PA_PH_PERFCOUNTER2_SELECT,
   R_037610_PA_PH_PERFCOUNTER3_SELECT,
   R_037614_PA_PH_PERFCOUNTER4_SELECT,
   R_037618_PA_PH_PERFCOUNTER5_SELECT,
   R_03761C_PA_PH_PERFCOUNTER6_SELECT,
   R_037620_PA_PH_PERFCOUNTER7_SELECT,
};
static unsigned gfx10_PA_PH_select1[] = {
   R_037604_PA_PH_PERFCOUNTER0_SELECT1,
   R_037640_PA_PH_PERFCOUNTER1_SELECT1,
   R_037644_PA_PH_PERFCOUNTER2_SELECT1,
   R_037648_PA_PH_PERFCOUNTER3_SELECT1,
};
static struct ac_pc_block_base gfx10_PA_PH = {
   .gpu_block = PA_PH,
   .name = "PA_PH",
   .num_counters = 8,
   .flags = AC_PC_BLOCK_SE,

   .select0 = gfx10_PA_PH_select0,
   .select1 = gfx10_PA_PH_select1,
   .counter0_lo = R_035600_PA_PH_PERFCOUNTER0_LO,

   .num_spm_counters = 4,
   .num_spm_wires = 8,
   .spm_block_select = AC_SPM_GLOBAL_BLOCK_PH,
};

/* gfx10_PA_SU */
static unsigned gfx10_PA_SU_select0[] = {
   R_036400_PA_SU_PERFCOUNTER0_SELECT,
   R_036408_PA_SU_PERFCOUNTER1_SELECT,
   R_036410_PA_SU_PERFCOUNTER2_SELECT,
   R_036418_PA_SU_PERFCOUNTER3_SELECT,
};
static unsigned gfx10_PA_SU_select1[] = {
   R_036404_PA_SU_PERFCOUNTER0_SELECT1,
   R_03640C_PA_SU_PERFCOUNTER1_SELECT1,
   R_036414_PA_SU_PERFCOUNTER2_SELECT1,
   R_03641C_PA_SU_PERFCOUNTER3_SELECT1,
};
static struct ac_pc_block_base gfx10_PA_SU = {
   .gpu_block = PA_SU,
   .name = "PA_SU",
   .num_counters = 4,
   .flags = AC_PC_BLOCK_SE,

   .select0 = gfx10_PA_SU_select0,
   .select1 = gfx10_PA_SU_select1,
   .counter0_lo = R_034400_PA_SU_PERFCOUNTER0_LO,

   .num_spm_counters = 4,
   .num_spm_wires = 8,
   .spm_block_select = AC_SPM_SE_BLOCK_PA,
};

/* gfx10_RLC */
static unsigned gfx10_RLC_select0[] = {
   R_037304_RLC_PERFCOUNTER0_SELECT,
   R_037308_RLC_PERFCOUNTER1_SELECT,
};
static struct ac_pc_block_base gfx10_RLC = {
   .gpu_block = RLC,
   .name = "RLC",
   .num_counters = 2,

   .select0 = gfx10_RLC_select0,
   .counter0_lo = R_035200_RLC_PERFCOUNTER0_LO,
   .num_spm_counters = 0,
};

/* gfx10_RMI */
static unsigned gfx10_RMI_select0[] = {
   R_037400_RMI_PERFCOUNTER0_SELECT,
   R_037408_RMI_PERFCOUNTER1_SELECT,
   R_03740C_RMI_PERFCOUNTER2_SELECT,
   R_037414_RMI_PERFCOUNTER3_SELECT,
};
static unsigned gfx10_RMI_select1[] = {
   R_037404_RMI_PERFCOUNTER0_SELECT1,
   R_037410_RMI_PERFCOUNTER2_SELECT1,
};
static struct ac_pc_block_base gfx10_RMI = {
   .gpu_block = RMI,
   .name = "RMI",
   .num_counters = 4,
   .flags = AC_PC_BLOCK_SE | AC_PC_BLOCK_INSTANCE_GROUPS,

   .select0 = gfx10_RMI_select0,
   .select1 = gfx10_RMI_select1,
   .counter0_lo = R_035300_RMI_PERFCOUNTER0_LO,

   .num_spm_counters = 2,
   .num_spm_wires = 2,
   .spm_block_select = AC_SPM_SE_BLOCK_RMI,
};

/* gfx10_SQ */
static struct ac_pc_block_base gfx10_SQ = {
   .gpu_block = SQ,
   .name = "SQ",
   .num_counters = 16,
   .flags = AC_PC_BLOCK_SE | AC_PC_BLOCK_SHADER,

   .select0 = cik_SQ_select0,
   .select_or = S_036700_SQC_BANK_MASK(15),
   .counter0_lo = R_034700_SQ_PERFCOUNTER0_LO,

   .num_spm_wires = 16,
   .spm_block_select = AC_SPM_SE_BLOCK_SQG,
};

/* gfx10_TCP */
static struct ac_pc_block_base gfx10_TCP = {
   .gpu_block = TCP,
   .name = "TCP",
   .num_counters = 4,
   .flags = AC_PC_BLOCK_SE | AC_PC_BLOCK_INSTANCE_GROUPS | AC_PC_BLOCK_SHADER_WINDOWED,

   .select0 = cik_TCP_select0,
   .select1 = cik_TCP_select1,
   .counter0_lo = R_034D00_TCP_PERFCOUNTER0_LO,

   .num_spm_counters = 2,
   .num_spm_wires = 4,
   .spm_block_select = AC_SPM_SE_BLOCK_TCP,
};

/* gfx10_UTCL1 */
static unsigned gfx10_UTCL1_select0[] = {
   R_03758C_UTCL1_PERFCOUNTER0_SELECT,
   R_037590_UTCL1_PERFCOUNTER1_SELECT,
};
static struct ac_pc_block_base gfx10_UTCL1 = {
   .gpu_block = UTCL1,
   .name = "UTCL1",
   .num_counters = 2,
   .flags = AC_PC_BLOCK_SE | AC_PC_BLOCK_SHADER_WINDOWED,

   .select0 = gfx10_UTCL1_select0,
   .counter0_lo = R_035470_UTCL1_PERFCOUNTER0_LO,
   .num_spm_counters = 0,
};

/* gfx11_SQ_WQP */
static struct ac_pc_block_base gfx11_SQ_WGP = {
   .gpu_block = SQ_WGP,
   .name = "SQ_WGP",
   .num_counters = 16,
   .flags = AC_PC_BLOCK_SE | AC_PC_BLOCK_SHADER,

   .select0 = cik_SQ_select0,
   .counter0_lo = R_034700_SQ_PERFCOUNTER0_LO,

   .num_spm_counters = 8,
   .num_spm_wires = 8,
   .spm_block_select = AC_SPM_SE_BLOCK_SQC,
};

/* Both the number of instances and selectors varies between chips of the same
 * class. We only differentiate by class here and simply expose the maximum
 * number over all chips in a class.
 *
 * Unfortunately, GPUPerfStudio uses the order of performance counter groups
 * blindly once it believes it has identified the hardware, so the order of
 * blocks here matters.
 */
static struct ac_pc_block_gfxdescr groups_CIK[] = {
   {&cik_CB, 226},     {&cik_CPF, 17},    {&cik_DB, 257},  {&cik_GRBM, 34},   {&cik_GRBMSE, 15},
   {&cik_PA_SU, 153},  {&cik_PA_SC, 395}, {&cik_SPI, 186}, {&cik_SQ, 252},    {&cik_SX, 32},
   {&cik_TA, 111},     {&cik_TCA, 39, 2}, {&cik_TCC, 160}, {&cik_TD, 55},     {&cik_TCP, 154},
   {&cik_GDS, 121},    {&cik_VGT, 140},   {&cik_IA, 22},   {&cik_MC, 22},     {&cik_SRBM, 19},
   {&cik_WD, 22},      {&cik_CPG, 46},    {&cik_CPC, 22},

};

static struct ac_pc_block_gfxdescr groups_VI[] = {
   {&cik_CB, 405},     {&cik_CPF, 19},    {&cik_DB, 257},  {&cik_GRBM, 34},   {&cik_GRBMSE, 15},
   {&cik_PA_SU, 154},  {&cik_PA_SC, 397}, {&cik_SPI, 197}, {&cik_SQ, 273},    {&cik_SX, 34},
   {&cik_TA, 119},     {&cik_TCA, 35, 2}, {&cik_TCC, 192}, {&cik_TD, 55},     {&cik_TCP, 180},
   {&cik_GDS, 121},    {&cik_VGT, 147},   {&cik_IA, 24},   {&cik_MC, 22},     {&cik_SRBM, 27},
   {&cik_WD, 37},      {&cik_CPG, 48},    {&cik_CPC, 24},

};

static struct ac_pc_block_gfxdescr groups_gfx9[] = {
   {&cik_CB, 438},     {&cik_CPF, 32},    {&cik_DB, 328},  {&cik_GRBM, 38},   {&cik_GRBMSE, 16},
   {&cik_PA_SU, 292},  {&cik_PA_SC, 491}, {&cik_SPI, 196}, {&cik_SQ, 374},    {&cik_SX, 208},
   {&cik_TA, 119},     {&cik_TCA, 35, 2}, {&cik_TCC, 256}, {&cik_TD, 57},     {&cik_TCP, 85},
   {&cik_GDS, 121},    {&cik_VGT, 148},   {&cik_IA, 32},   {&cik_WD, 58},     {&cik_CPG, 59},
   {&cik_CPC, 35},
};

static struct ac_pc_block_gfxdescr groups_gfx10[] = {
   {&cik_CB, 461},
   {&gfx10_CHA, 45},
   {&gfx10_CHCG, 35},
   {&gfx10_CHC, 35},
   {&cik_CPC, 47},
   {&cik_CPF, 40},
   {&cik_CPG, 82},
   {&gfx10_DB, 370},
   {&gfx10_GCR, 94},
   {&cik_GDS, 123},
   {&gfx10_GE, 315},
   {&gfx10_GL1A, 36},
   {&gfx10_GL1C, 64, 4},
   {&gfx10_GL2A, 91},
   {&gfx10_GL2C, 235},
   {&cik_GRBM, 47},
   {&cik_GRBMSE, 19},
   {&gfx10_PA_PH, 960},
   {&cik_PA_SC, 552},
   {&gfx10_PA_SU, 266},
   {&gfx10_RLC, 7},
   {&gfx10_RMI, 258},
   {&cik_SPI, 329},
   {&gfx10_SQ, 509},
   {&cik_SX, 225},
   {&cik_TA, 226},
   {&gfx10_TCP, 77},
   {&cik_TD, 61},
   {&gfx10_UTCL1, 15},
};

static struct ac_pc_block_gfxdescr groups_gfx11[] = {
   {&cik_CB, 313},
   {&gfx10_CHA, 39},
   {&gfx10_CHCG, 43},
   {&gfx10_CHC, 43},
   {&cik_CPC, 55},
   {&cik_CPF, 43},
   {&cik_CPG, 91},
   {&gfx10_DB, 370},
   {&gfx10_GCR, 154},
   {&cik_GDS, 147},
   {&gfx10_GE, 39},
   {&gfx10_GL1A, 23},
   {&gfx10_GL1C, 83, 4},
   {&gfx10_GL2A, 107},
   {&gfx10_GL2C, 258},
   {&cik_GRBM, 49},
   {&cik_GRBMSE, 20},
   {&gfx10_PA_PH, 1023},
   {&cik_PA_SC, 664},
   {&gfx10_PA_SU, 310},
   {&gfx10_RLC, 6},
   {&gfx10_RMI, 138},
   {&cik_SPI, 283},
   {&gfx10_SQ, 36},
   {&cik_SX, 81},
   {&cik_TA, 235},
   {&gfx10_TCP, 77},
   {&cik_TD, 196},
   {&gfx10_UTCL1, 65},
   {&gfx11_SQ_WGP, 511, 4},
};

struct ac_pc_block *ac_lookup_counter(const struct ac_perfcounters *pc,
                                      unsigned index, unsigned *base_gid,
                                      unsigned *sub_index)
{
   struct ac_pc_block *block = pc->blocks;
   unsigned bid;

   *base_gid = 0;
   for (bid = 0; bid < pc->num_blocks; ++bid, ++block) {
      unsigned total = block->num_groups * block->b->selectors;

      if (index < total) {
         *sub_index = index;
         return block;
      }

      index -= total;
      *base_gid += block->num_groups;
   }

   return NULL;
}

struct ac_pc_block *ac_lookup_group(const struct ac_perfcounters *pc,
                                    unsigned *index)
{
   unsigned bid;
   struct ac_pc_block *block = pc->blocks;

   for (bid = 0; bid < pc->num_blocks; ++bid, ++block) {
      if (*index < block->num_groups)
         return block;
      *index -= block->num_groups;
   }

   return NULL;
}

bool ac_init_block_names(const struct radeon_info *info,
                         const struct ac_perfcounters *pc,
                         struct ac_pc_block *block)
{
   bool per_instance_groups = ac_pc_block_has_per_instance_groups(pc, block);
   bool per_se_groups = ac_pc_block_has_per_se_groups(pc, block);
   unsigned i, j, k;
   unsigned groups_shader = 1, groups_se = 1, groups_instance = 1;
   unsigned namelen;
   char *groupname;
   char *p;

   if (per_instance_groups)
      groups_instance = block->num_instances;
   if (per_se_groups)
      groups_se = info->max_se;
   if (block->b->b->flags & AC_PC_BLOCK_SHADER)
      groups_shader = ARRAY_SIZE(ac_pc_shader_type_bits);

   namelen = strlen(block->b->b->name);
   block->group_name_stride = namelen + 1;
   if (block->b->b->flags & AC_PC_BLOCK_SHADER)
      block->group_name_stride += 3;
   if (per_se_groups) {
      assert(groups_se <= 10);
      block->group_name_stride += 1;

      if (per_instance_groups)
         block->group_name_stride += 1;
   }
   if (per_instance_groups) {
      assert(groups_instance <= 100);
      block->group_name_stride += 2;
   }

   block->group_names = MALLOC(block->num_groups * block->group_name_stride);
   if (!block->group_names)
      return false;

   groupname = block->group_names;
   for (i = 0; i < groups_shader; ++i) {
      const char *shader_suffix = ac_pc_shader_type_suffixes[i];
      unsigned shaderlen = strlen(shader_suffix);
      for (j = 0; j < groups_se; ++j) {
         for (k = 0; k < groups_instance; ++k) {
            strcpy(groupname, block->b->b->name);
            p = groupname + namelen;

            if (block->b->b->flags & AC_PC_BLOCK_SHADER) {
               strcpy(p, shader_suffix);
               p += shaderlen;
            }

            if (per_se_groups) {
               p += sprintf(p, "%d", j);
               if (per_instance_groups)
                  *p++ = '_';
            }

            if (per_instance_groups)
               p += sprintf(p, "%d", k);

            groupname += block->group_name_stride;
         }
      }
   }

   block->selector_name_stride = block->group_name_stride + 4;
   block->selector_names =
      MALLOC(block->num_groups * block->b->selectors * block->selector_name_stride);
   if (!block->selector_names)
      return false;

   groupname = block->group_names;
   p = block->selector_names;
   for (i = 0; i < block->num_groups; ++i) {
      for (j = 0; j < block->b->selectors; ++j) {
         sprintf(p, "%s_%03d", groupname, j);
         p += block->selector_name_stride;
      }
      groupname += block->group_name_stride;
   }

   return true;
}

bool ac_init_perfcounters(const struct radeon_info *info,
                          bool separate_se,
                          bool separate_instance,
                          struct ac_perfcounters *pc)
{
   const struct ac_pc_block_gfxdescr *blocks;
   unsigned num_blocks;

   switch (info->gfx_level) {
   case GFX7:
      blocks = groups_CIK;
      num_blocks = ARRAY_SIZE(groups_CIK);
      break;
   case GFX8:
      blocks = groups_VI;
      num_blocks = ARRAY_SIZE(groups_VI);
      break;
   case GFX9:
      blocks = groups_gfx9;
      num_blocks = ARRAY_SIZE(groups_gfx9);
      break;
   case GFX10:
   case GFX10_3:
      blocks = groups_gfx10;
      num_blocks = ARRAY_SIZE(groups_gfx10);
      break;
   case GFX11:
      blocks = groups_gfx11;
      num_blocks = ARRAY_SIZE(groups_gfx11);
      break;
   case GFX6:
   default:
      return false; /* not implemented */
   }

   pc->separate_se = separate_se;
   pc->separate_instance = separate_instance;

   pc->blocks = CALLOC(num_blocks, sizeof(struct ac_pc_block));
   if (!pc->blocks)
      return false;
   pc->num_blocks = num_blocks;

   for (unsigned i = 0; i < num_blocks; i++) {
      struct ac_pc_block *block = &pc->blocks[i];

      block->b = &blocks[i];
      block->num_instances = MAX2(1, block->b->instances);

      if (!strcmp(block->b->b->name, "CB") ||
          !strcmp(block->b->b->name, "DB") ||
          !strcmp(block->b->b->name, "RMI"))
         block->num_instances = info->max_se;
      else if (!strcmp(block->b->b->name, "TCC"))
         block->num_instances = info->max_tcc_blocks;
      else if (!strcmp(block->b->b->name, "IA"))
         block->num_instances = MAX2(1, info->max_se / 2);
      else if (!strcmp(block->b->b->name, "TA") ||
               !strcmp(block->b->b->name, "TCP") ||
               !strcmp(block->b->b->name, "TD")) {
         block->num_instances = MAX2(1, info->max_good_cu_per_sa);
      }

      if (info->gfx_level >= GFX10) {
         if (!strcmp(block->b->b->name, "TCP")) {
            block->num_global_instances = MAX2(1, info->num_cu_per_sh) * info->num_se * info->max_sa_per_se;
         } else if (!strcmp(block->b->b->name, "SQ")) {
            block->num_global_instances = block->num_instances * info->num_se;
         } else if (!strcmp(block->b->b->name, "GL1C") ||
                    !strcmp(block->b->b->name, "SQ_WGP")) {
            block->num_global_instances = block->num_instances * info->num_se * info->max_sa_per_se;
         } else if (!strcmp(block->b->b->name, "GL2C")) {
            block->num_instances = block->num_global_instances = info->num_tcc_blocks;
         }
      }

      if (ac_pc_block_has_per_instance_groups(pc, block)) {
         block->num_groups = block->num_instances;
      } else {
         block->num_groups = 1;
      }

      if (ac_pc_block_has_per_se_groups(pc, block))
         block->num_groups *= info->max_se;
      if (block->b->b->flags & AC_PC_BLOCK_SHADER)
         block->num_groups *= ARRAY_SIZE(ac_pc_shader_type_bits);

      pc->num_groups += block->num_groups;
   }

   return true;
}

void ac_destroy_perfcounters(struct ac_perfcounters *pc)
{
   if (!pc)
      return;

   for (unsigned i = 0; i < pc->num_blocks; ++i) {
      FREE(pc->blocks[i].group_names);
      FREE(pc->blocks[i].selector_names);
   }
   FREE(pc->blocks);
}

struct ac_pc_block *ac_pc_get_block(const struct ac_perfcounters *pc,
                                    enum ac_pc_gpu_block gpu_block)
{
   for (unsigned i = 0; i < pc->num_blocks; i++) {
      struct ac_pc_block *block = &pc->blocks[i];
      if (block->b->b->gpu_block == gpu_block) {
         return block;
      }
   }
   return NULL;
}
