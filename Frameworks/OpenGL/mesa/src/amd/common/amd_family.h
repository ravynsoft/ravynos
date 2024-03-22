/*
 * Copyright 2008 Corbin Simpson <MostAwesomeDude@gmail.com>
 * Copyright 2010 Marek Olšák <maraeo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef AMD_FAMILY_H
#define AMD_FAMILY_H

#ifdef __cplusplus
extern "C" {
#endif

enum radeon_family
{
   CHIP_UNKNOWN = 0,
   /* R3xx-based cores. (GFX2) */
   CHIP_R300,
   CHIP_R350,
   CHIP_RV350,
   CHIP_RV370,
   CHIP_RV380,
   CHIP_RS400,
   CHIP_RC410,
   CHIP_RS480,
   /* R4xx-based cores. (GFX2) */
   CHIP_R420,
   CHIP_R423,
   CHIP_R430,
   CHIP_R480,
   CHIP_R481,
   CHIP_RV410,
   CHIP_RS600,
   CHIP_RS690,
   CHIP_RS740,
   /* R5xx-based cores. (GFX2) */
   CHIP_RV515,
   CHIP_R520,
   CHIP_RV530,
   CHIP_R580,
   CHIP_RV560,
   CHIP_RV570,
   /* GFX3 (R6xx) */
   CHIP_R600,
   CHIP_RV610,
   CHIP_RV630,
   CHIP_RV670,
   CHIP_RV620,
   CHIP_RV635,
   CHIP_RS780,
   CHIP_RS880,
   /* GFX3 (R7xx) */
   CHIP_RV770,
   CHIP_RV730,
   CHIP_RV710,
   CHIP_RV740,
   /* GFX4 (Evergreen) */
   CHIP_CEDAR,
   CHIP_REDWOOD,
   CHIP_JUNIPER,
   CHIP_CYPRESS,
   CHIP_HEMLOCK,
   CHIP_PALM,
   CHIP_SUMO,
   CHIP_SUMO2,
   CHIP_BARTS,
   CHIP_TURKS,
   CHIP_CAICOS,
   /* GFX5 (Northern Islands) */
   CHIP_CAYMAN,
   CHIP_ARUBA,
   /* GFX6 (Southern Islands) */
   CHIP_TAHITI,
   CHIP_PITCAIRN,
   CHIP_VERDE,
   CHIP_OLAND,
   CHIP_HAINAN,
   /* GFX7 (Sea Islands) */
   CHIP_BONAIRE,
   CHIP_KAVERI,
   CHIP_KABINI,
   CHIP_HAWAII,         /* Radeon 290, 390 */
   /* GFX8 (Volcanic Islands & Polaris) */
   CHIP_TONGA,          /* Radeon 285, 380 */
   CHIP_ICELAND,
   CHIP_CARRIZO,
   CHIP_FIJI,           /* Radeon Fury */
   CHIP_STONEY,
   CHIP_POLARIS10,      /* Radeon 470, 480, 570, 580, 590 */
   CHIP_POLARIS11,      /* Radeon 460, 560 */
   CHIP_POLARIS12,      /* Radeon 540, 550 */
   CHIP_VEGAM,
   /* GFX9 (Vega) */
   CHIP_VEGA10,         /* Vega 56, 64 */
   CHIP_VEGA12,
   CHIP_VEGA20,         /* Radeon VII, MI50 */
   CHIP_RAVEN,          /* Ryzen 2000, 3000 */
   CHIP_RAVEN2,         /* Ryzen 2200U, 3200U */
   CHIP_RENOIR,         /* Ryzen 4000, 5000 */
   CHIP_MI100,
   CHIP_MI200,
   CHIP_GFX940,
   /* GFX10.1 (RDNA 1) */
   CHIP_NAVI10,         /* Radeon 5600, 5700 */
   CHIP_NAVI12,         /* Radeon Pro 5600M */
   CHIP_NAVI14,         /* Radeon 5300, 5500 */
   /* GFX10.3 (RDNA 2) */
   CHIP_NAVI21,         /* Radeon 6800, 6900 (formerly "Sienna Cichlid") */
   CHIP_NAVI22,         /* Radeon 6700 (formerly "Navy Flounder") */
   CHIP_VANGOGH,        /* Steam Deck */
   CHIP_NAVI23,         /* Radeon 6600 (formerly "Dimgrey Cavefish") */
   CHIP_NAVI24,         /* Radeon 6400, 6500 (formerly "Beige Goby") */
   CHIP_REMBRANDT,      /* Ryzen 6000 (formerly "Yellow Carp") */
   CHIP_RAPHAEL_MENDOCINO, /* Ryzen 7000(X), Ryzen 7045, Ryzen 7020 */
   /* GFX11 (RDNA 3) */
   CHIP_NAVI31,         /* Radeon 7900 */
   CHIP_NAVI32,         /* Radeon 7800, 7700 */
   CHIP_NAVI33,         /* Radeon 7600, 7700S (mobile) */
   CHIP_GFX1103_R1,
   CHIP_GFX1103_R2,
   CHIP_GFX1150,
   CHIP_LAST,
};

enum amd_gfx_level
{
   CLASS_UNKNOWN = 0,
   R300,
   R400,
   R500,
   R600,
   R700,
   EVERGREEN,
   CAYMAN,
   GFX6,
   GFX7,
   GFX8,
   GFX9,
   GFX10,
   GFX10_3,
   GFX11,
   GFX11_5,

   NUM_GFX_VERSIONS,
};

enum amd_ip_type
{
   AMD_IP_GFX = 0,
   AMD_IP_COMPUTE,
   AMD_IP_SDMA,
   AMD_IP_UVD,
   AMD_IP_VCE,
   AMD_IP_UVD_ENC,
   AMD_IP_VCN_DEC,
   AMD_IP_VCN_ENC,
   AMD_IP_VCN_UNIFIED = AMD_IP_VCN_ENC,
   AMD_IP_VCN_JPEG,
   AMD_IP_VPE,
   AMD_NUM_IP_TYPES,
};

enum amd_vram_type {
   AMD_VRAM_TYPE_UNKNOWN = 0,
   AMD_VRAM_TYPE_GDDR1,
   AMD_VRAM_TYPE_DDR2,
   AMD_VRAM_TYPE_GDDR3,
   AMD_VRAM_TYPE_GDDR4,
   AMD_VRAM_TYPE_GDDR5,
   AMD_VRAM_TYPE_HBM,
   AMD_VRAM_TYPE_DDR3,
   AMD_VRAM_TYPE_DDR4,
   AMD_VRAM_TYPE_GDDR6,
   AMD_VRAM_TYPE_DDR5,
   AMD_VRAM_TYPE_LPDDR4,
   AMD_VRAM_TYPE_LPDDR5,
};

enum vcn_version{
   VCN_UNKNOWN,
   VCN_1_0_0,
   VCN_1_0_1,

   VCN_2_0_0,
   VCN_2_0_2,
   VCN_2_0_3,
   VCN_2_2_0,
   VCN_2_5_0,
   VCN_2_6_0,

   VCN_3_0_0,
   VCN_3_0_2,
   VCN_3_0_16,
   VCN_3_0_33,
   VCN_3_1_1,
   VCN_3_1_2,

   VCN_4_0_0,
   VCN_4_0_2,
   VCN_4_0_3,
   VCN_4_0_4,
   VCN_4_0_5,
};

#define SDMA_VERSION_VALUE(major, minor) (((major) << 8) | (minor))

enum sdma_version {
   SDMA_UNKNOWN = 0,
   /* GFX6 */
   SDMA_1_0 = SDMA_VERSION_VALUE(1, 0),

   /* GFX7 */
   SDMA_2_0 = SDMA_VERSION_VALUE(2, 0),

   /* GFX8 */
   SDMA_2_4 = SDMA_VERSION_VALUE(2, 4),
   SDMA_3_0 = SDMA_VERSION_VALUE(3, 0),
   SDMA_3_1 = SDMA_VERSION_VALUE(3, 1),

   /* GFX9 */
   SDMA_4_0 = SDMA_VERSION_VALUE(4, 0),
   SDMA_4_1 = SDMA_VERSION_VALUE(4, 1),
   SDMA_4_2 = SDMA_VERSION_VALUE(4, 2),
   SDMA_4_4 = SDMA_VERSION_VALUE(4, 4),

   /* GFX10 */
   SDMA_5_0 = SDMA_VERSION_VALUE(5, 0),

   /* GFX10.3 */
   SDMA_5_2 = SDMA_VERSION_VALUE(5, 2),

   /* GFX11 */
   SDMA_6_0 = SDMA_VERSION_VALUE(6, 0),

   /* GFX11.5 */
   SDMA_6_1 = SDMA_VERSION_VALUE(6, 1),
};

const char *ac_get_family_name(enum radeon_family family);
enum amd_gfx_level ac_get_gfx_level(enum radeon_family family);
unsigned ac_get_family_id(enum radeon_family family);
const char *ac_get_llvm_processor_name(enum radeon_family family);

#ifdef __cplusplus
}
#endif

#endif
