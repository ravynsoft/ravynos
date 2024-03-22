/*
 * Copyright © 2020 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

/* These tables define the set of ranges of registers we shadow when
 * mid command buffer preemption is enabled.
 */

#include "ac_shadowed_regs.h"

#include "ac_debug.h"
#include "sid.h"
#include "util/macros.h"
#include "util/u_debug.h"

#include <stdio.h>

static const struct ac_reg_range Gfx9UserConfigShadowRange[] = {
   {
      R_0300FC_CP_STRMOUT_CNTL,
      4,
   },
   {
      R_0301EC_CP_COHER_START_DELAY,
      4,
   },
   {
      R_030904_VGT_GSVS_RING_SIZE,
      R_030908_VGT_PRIMITIVE_TYPE - R_030904_VGT_GSVS_RING_SIZE + 4,
   },
   {
      R_030920_VGT_MAX_VTX_INDX,
      R_03092C_VGT_MULTI_PRIM_IB_RESET_EN - R_030920_VGT_MAX_VTX_INDX + 4,
   },
   {
      R_030934_VGT_NUM_INSTANCES,
      R_030944_VGT_TF_MEMORY_BASE_HI - R_030934_VGT_NUM_INSTANCES + 4,
   },
   {
      R_030960_IA_MULTI_VGT_PARAM,
      4,
   },
   {
      R_030968_VGT_INSTANCE_BASE_ID,
      4,
   },
   {
      R_030E00_TA_CS_BC_BASE_ADDR,
      R_030E04_TA_CS_BC_BASE_ADDR_HI - R_030E00_TA_CS_BC_BASE_ADDR + 4,
   },
   {
      R_030AD4_PA_STATE_STEREO_X,
      4,
   },
};

static const struct ac_reg_range Gfx9ContextShadowRange[] = {
   {
      R_028000_DB_RENDER_CONTROL,
      R_028084_TA_BC_BASE_ADDR_HI - R_028000_DB_RENDER_CONTROL + 4,
   },
   {
      R_0281E8_COHER_DEST_BASE_HI_0,
      R_02835C_PA_SC_TILE_STEERING_OVERRIDE - R_0281E8_COHER_DEST_BASE_HI_0 + 4,
   },
   {
      R_02840C_VGT_MULTI_PRIM_IB_RESET_INDX,
      4,
   },
   {
      R_028414_CB_BLEND_RED,
      R_028618_PA_CL_UCP_5_W - R_028414_CB_BLEND_RED + 4,
   },
   {
      R_028644_SPI_PS_INPUT_CNTL_0,
      R_028714_SPI_SHADER_COL_FORMAT - R_028644_SPI_PS_INPUT_CNTL_0 + 4,
   },
   {
      R_028754_SX_PS_DOWNCONVERT,
      R_0287BC_CB_MRT7_EPITCH - R_028754_SX_PS_DOWNCONVERT + 4,
   },
   {
      R_028800_DB_DEPTH_CONTROL,
      R_028840_PA_STEREO_CNTL - R_028800_DB_DEPTH_CONTROL + 4,
   },
   {
      R_028A00_PA_SU_POINT_SIZE,
      R_028A0C_PA_SC_LINE_STIPPLE - R_028A00_PA_SU_POINT_SIZE + 4,
   },
   {
      R_028A18_VGT_HOS_MAX_TESS_LEVEL,
      R_028A1C_VGT_HOS_MIN_TESS_LEVEL - R_028A18_VGT_HOS_MAX_TESS_LEVEL + 4,
   },
   {
      R_028A40_VGT_GS_MODE,
      R_028A6C_VGT_GS_OUT_PRIM_TYPE - R_028A40_VGT_GS_MODE + 4,
   },
   {
      R_028A84_VGT_PRIMITIVEID_EN,
      4,
   },
   {
      R_028A8C_VGT_PRIMITIVEID_RESET,
      4,
   },
   {
      R_028A94_VGT_GS_MAX_PRIMS_PER_SUBGROUP,
      R_028AD4_VGT_STRMOUT_VTX_STRIDE_0 - R_028A94_VGT_GS_MAX_PRIMS_PER_SUBGROUP + 4,
   },
   {
      R_028AE0_VGT_STRMOUT_BUFFER_SIZE_1,
      R_028AE4_VGT_STRMOUT_VTX_STRIDE_1 - R_028AE0_VGT_STRMOUT_BUFFER_SIZE_1 + 4,
   },
   {
      R_028AF0_VGT_STRMOUT_BUFFER_SIZE_2,
      R_028AF4_VGT_STRMOUT_VTX_STRIDE_2 - R_028AF0_VGT_STRMOUT_BUFFER_SIZE_2 + 4,
   },
   {
      R_028B00_VGT_STRMOUT_BUFFER_SIZE_3,
      R_028B04_VGT_STRMOUT_VTX_STRIDE_3 - R_028B00_VGT_STRMOUT_BUFFER_SIZE_3 + 4,
   },
   {
      R_028B28_VGT_STRMOUT_DRAW_OPAQUE_OFFSET,
      R_028B30_VGT_STRMOUT_DRAW_OPAQUE_VERTEX_STRIDE - R_028B28_VGT_STRMOUT_DRAW_OPAQUE_OFFSET + 4,
   },
   {
      R_028B38_VGT_GS_MAX_VERT_OUT,
      R_028B98_VGT_STRMOUT_BUFFER_CONFIG - R_028B38_VGT_GS_MAX_VERT_OUT + 4,
   },
   {
      R_028BD4_PA_SC_CENTROID_PRIORITY_0,
      R_028E3C_CB_COLOR7_DCC_BASE_EXT - R_028BD4_PA_SC_CENTROID_PRIORITY_0 + 4,
   },
};

static const struct ac_reg_range Gfx9ShShadowRange[] = {
   {
      R_00B020_SPI_SHADER_PGM_LO_PS,
      R_00B0AC_SPI_SHADER_USER_DATA_PS_31 - R_00B020_SPI_SHADER_PGM_LO_PS + 4,
   },
   {
      R_00B11C_SPI_SHADER_LATE_ALLOC_VS,
      R_00B1AC_SPI_SHADER_USER_DATA_VS_31 - R_00B11C_SPI_SHADER_LATE_ALLOC_VS + 4,
   },
   {
      R_00B204_SPI_SHADER_PGM_RSRC4_GS,
      R_00B214_SPI_SHADER_PGM_HI_ES - R_00B204_SPI_SHADER_PGM_RSRC4_GS + 4,
   },
   {
      R_00B220_SPI_SHADER_PGM_LO_GS,
      R_00B22C_SPI_SHADER_PGM_RSRC2_GS - R_00B220_SPI_SHADER_PGM_LO_GS + 4,
   },
   {
      R_00B330_SPI_SHADER_USER_DATA_ES_0,
      R_00B3AC_SPI_SHADER_USER_DATA_ES_31 - R_00B330_SPI_SHADER_USER_DATA_ES_0 + 4,
   },
   {
      R_00B404_SPI_SHADER_PGM_RSRC4_HS,
      R_00B414_SPI_SHADER_PGM_HI_LS - R_00B404_SPI_SHADER_PGM_RSRC4_HS + 4,
   },
   {
      R_00B420_SPI_SHADER_PGM_LO_HS,
      R_00B4AC_SPI_SHADER_USER_DATA_LS_31 - R_00B420_SPI_SHADER_PGM_LO_HS + 4,
   },
};

static const struct ac_reg_range Gfx9CsShShadowRange[] = {
   {
      R_00B810_COMPUTE_START_X,
      R_00B824_COMPUTE_NUM_THREAD_Z - R_00B810_COMPUTE_START_X + 4,
   },
   {
      R_00B82C_COMPUTE_PERFCOUNT_ENABLE,
      R_00B834_COMPUTE_PGM_HI - R_00B82C_COMPUTE_PERFCOUNT_ENABLE + 4,
   },
   {
      R_00B848_COMPUTE_PGM_RSRC1,
      R_00B84C_COMPUTE_PGM_RSRC2 - R_00B848_COMPUTE_PGM_RSRC1 + 4,
   },
   {
      R_00B854_COMPUTE_RESOURCE_LIMITS,
      4,
   },
   {
      R_00B860_COMPUTE_TMPRING_SIZE,
      4,
   },
   {
      R_00B878_COMPUTE_THREAD_TRACE_ENABLE,
      4,
   },
   {
      R_00B900_COMPUTE_USER_DATA_0,
      R_00B93C_COMPUTE_USER_DATA_15 - R_00B900_COMPUTE_USER_DATA_0 + 4,
   },
};

static const struct ac_reg_range Gfx9ShShadowRangeRaven2[] = {
   {
      R_00B018_SPI_SHADER_PGM_CHKSUM_PS,
      4,
   },
   {
      R_00B020_SPI_SHADER_PGM_LO_PS,
      R_00B0AC_SPI_SHADER_USER_DATA_PS_31 - R_00B020_SPI_SHADER_PGM_LO_PS + 4,
   },
   {
      R_00B114_SPI_SHADER_PGM_CHKSUM_VS,
      4,
   },
   {
      R_00B11C_SPI_SHADER_LATE_ALLOC_VS,
      R_00B1AC_SPI_SHADER_USER_DATA_VS_31 - R_00B11C_SPI_SHADER_LATE_ALLOC_VS + 4,
   },
   {
      R_00B200_SPI_SHADER_PGM_CHKSUM_GS,
      R_00B214_SPI_SHADER_PGM_HI_ES - R_00B200_SPI_SHADER_PGM_CHKSUM_GS + 4,
   },
   {
      R_00B220_SPI_SHADER_PGM_LO_GS,
      R_00B22C_SPI_SHADER_PGM_RSRC2_GS - R_00B220_SPI_SHADER_PGM_LO_GS + 4,
   },
   {
      R_00B330_SPI_SHADER_USER_DATA_ES_0,
      R_00B3AC_SPI_SHADER_USER_DATA_ES_31 - R_00B330_SPI_SHADER_USER_DATA_ES_0 + 4,
   },
   {
      R_00B400_SPI_SHADER_PGM_CHKSUM_HS,
      R_00B414_SPI_SHADER_PGM_HI_LS - R_00B400_SPI_SHADER_PGM_CHKSUM_HS + 4,
   },
   {
      R_00B420_SPI_SHADER_PGM_LO_HS,
      R_00B4AC_SPI_SHADER_USER_DATA_LS_31 - R_00B420_SPI_SHADER_PGM_LO_HS + 4,
   },
};

static const struct ac_reg_range Gfx9CsShShadowRangeRaven2[] = {
   {
      R_00B810_COMPUTE_START_X,
      R_00B824_COMPUTE_NUM_THREAD_Z - R_00B810_COMPUTE_START_X + 4,
   },
   {
      R_00B82C_COMPUTE_PERFCOUNT_ENABLE,
      R_00B834_COMPUTE_PGM_HI - R_00B82C_COMPUTE_PERFCOUNT_ENABLE + 4,
   },
   {
      R_00B848_COMPUTE_PGM_RSRC1,
      R_00B84C_COMPUTE_PGM_RSRC2 - R_00B848_COMPUTE_PGM_RSRC1 + 4,
   },
   {
      R_00B854_COMPUTE_RESOURCE_LIMITS,
      4,
   },
   {
      R_00B860_COMPUTE_TMPRING_SIZE,
      4,
   },
   {
      R_00B878_COMPUTE_THREAD_TRACE_ENABLE,
      4,
   },
   {
      R_00B894_COMPUTE_SHADER_CHKSUM,
      4,
   },
   {
      R_00B900_COMPUTE_USER_DATA_0,
      R_00B93C_COMPUTE_USER_DATA_15 - R_00B900_COMPUTE_USER_DATA_0 + 4,
   },
};

static const struct ac_reg_range Nv10ContextShadowRange[] = {
   {
      R_028000_DB_RENDER_CONTROL,
      R_028084_TA_BC_BASE_ADDR_HI - R_028000_DB_RENDER_CONTROL + 4,
   },
   {
      R_0281E8_COHER_DEST_BASE_HI_0,
      R_02835C_PA_SC_TILE_STEERING_OVERRIDE - R_0281E8_COHER_DEST_BASE_HI_0 + 4,
   },
   {
      R_02840C_VGT_MULTI_PRIM_IB_RESET_INDX,
      R_028618_PA_CL_UCP_5_W - R_02840C_VGT_MULTI_PRIM_IB_RESET_INDX + 4,
   },
   {
      R_028644_SPI_PS_INPUT_CNTL_0,
      R_028714_SPI_SHADER_COL_FORMAT - R_028644_SPI_PS_INPUT_CNTL_0 + 4,
   },
   {
      R_028754_SX_PS_DOWNCONVERT,
      R_02879C_CB_BLEND7_CONTROL - R_028754_SX_PS_DOWNCONVERT + 4,
   },
   {
      R_0287D4_PA_CL_POINT_X_RAD,
      R_0287E0_PA_CL_POINT_CULL_RAD - R_0287D4_PA_CL_POINT_X_RAD + 4,
   },
   {
      R_0287FC_GE_MAX_OUTPUT_PER_SUBGROUP,
      R_028844_PA_STATE_STEREO_X - R_0287FC_GE_MAX_OUTPUT_PER_SUBGROUP + 4,
   },
   {
      R_028A00_PA_SU_POINT_SIZE,
      R_028A0C_PA_SC_LINE_STIPPLE - R_028A00_PA_SU_POINT_SIZE + 4,
   },
   {
      R_028A18_VGT_HOS_MAX_TESS_LEVEL,
      R_028A1C_VGT_HOS_MIN_TESS_LEVEL - R_028A18_VGT_HOS_MAX_TESS_LEVEL + 4,
   },
   {
      R_028A40_VGT_GS_MODE,
      R_028A6C_VGT_GS_OUT_PRIM_TYPE - R_028A40_VGT_GS_MODE + 4,
   },
   {
      R_028A84_VGT_PRIMITIVEID_EN,
      4,
   },
   {
      R_028A8C_VGT_PRIMITIVEID_RESET,
      4,
   },
   {
      R_028A98_VGT_DRAW_PAYLOAD_CNTL,
      R_028B98_VGT_STRMOUT_BUFFER_CONFIG - R_028A98_VGT_DRAW_PAYLOAD_CNTL + 4,
   },
   {
      R_028BD4_PA_SC_CENTROID_PRIORITY_0,
      R_028EFC_CB_COLOR7_ATTRIB3 - R_028BD4_PA_SC_CENTROID_PRIORITY_0 + 4,
   },
};

static const struct ac_reg_range Nv10UserConfigShadowRange[] = {
   {
      R_0300FC_CP_STRMOUT_CNTL,
      4,
   },
   {
      R_0301EC_CP_COHER_START_DELAY,
      4,
   },
   {
      R_030904_VGT_GSVS_RING_SIZE,
      R_030908_VGT_PRIMITIVE_TYPE - R_030904_VGT_GSVS_RING_SIZE + 4,
   },
   {
      R_030964_GE_MAX_VTX_INDX,
      4,
   },
   {
      R_030924_GE_MIN_VTX_INDX,
      R_03092C_GE_MULTI_PRIM_IB_RESET_EN - R_030924_GE_MIN_VTX_INDX + 4,
   },
   {
      R_030934_VGT_NUM_INSTANCES,
      R_030940_VGT_TF_MEMORY_BASE - R_030934_VGT_NUM_INSTANCES + 4,
   },
   {
      R_03097C_GE_STEREO_CNTL,
      R_030984_VGT_TF_MEMORY_BASE_HI - R_03097C_GE_STEREO_CNTL + 4,
   },
   {
      R_03096C_GE_CNTL,
      4,
   },
   {
      R_030968_VGT_INSTANCE_BASE_ID,
      4,
   },
   {
      R_030988_GE_USER_VGPR_EN,
      4,
   },
   {
      R_030E00_TA_CS_BC_BASE_ADDR,
      R_030E04_TA_CS_BC_BASE_ADDR_HI - R_030E00_TA_CS_BC_BASE_ADDR + 4,
   },
};

static const struct ac_reg_range Gfx10ShShadowRange[] = {
   {
      R_00B018_SPI_SHADER_PGM_CHKSUM_PS,
      4,
   },
   {
      R_00B020_SPI_SHADER_PGM_LO_PS,
      R_00B0AC_SPI_SHADER_USER_DATA_PS_31 - R_00B020_SPI_SHADER_PGM_LO_PS + 4,
   },
   {
      R_00B0C8_SPI_SHADER_USER_ACCUM_PS_0,
      R_00B0D4_SPI_SHADER_USER_ACCUM_PS_3 - R_00B0C8_SPI_SHADER_USER_ACCUM_PS_0 + 4,
   },
   {
      R_00B114_SPI_SHADER_PGM_CHKSUM_VS,
      4,
   },
   {
      R_00B11C_SPI_SHADER_LATE_ALLOC_VS,
      R_00B1AC_SPI_SHADER_USER_DATA_VS_31 - R_00B11C_SPI_SHADER_LATE_ALLOC_VS + 4,
   },
   {
      R_00B1C8_SPI_SHADER_USER_ACCUM_VS_0,
      R_00B1D4_SPI_SHADER_USER_ACCUM_VS_3 - R_00B1C8_SPI_SHADER_USER_ACCUM_VS_0 + 4,
   },
   {
      R_00B320_SPI_SHADER_PGM_LO_ES,
      R_00B324_SPI_SHADER_PGM_HI_ES - R_00B320_SPI_SHADER_PGM_LO_ES + 4,
   },
   {
      R_00B520_SPI_SHADER_PGM_LO_LS,
      R_00B524_SPI_SHADER_PGM_HI_LS - R_00B520_SPI_SHADER_PGM_LO_LS + 4,
   },
   {
      R_00B200_SPI_SHADER_PGM_CHKSUM_GS,
      4,
   },
   {
      R_00B220_SPI_SHADER_PGM_LO_GS,
      R_00B2AC_SPI_SHADER_USER_DATA_GS_31 - R_00B220_SPI_SHADER_PGM_LO_GS + 4,
   },
   {
      R_00B208_SPI_SHADER_USER_DATA_ADDR_LO_GS,
      R_00B20C_SPI_SHADER_USER_DATA_ADDR_HI_GS - R_00B208_SPI_SHADER_USER_DATA_ADDR_LO_GS + 4,
   },
   {
      R_00B408_SPI_SHADER_USER_DATA_ADDR_LO_HS,
      R_00B40C_SPI_SHADER_USER_DATA_ADDR_HI_HS - R_00B408_SPI_SHADER_USER_DATA_ADDR_LO_HS + 4,
   },
   {
      R_00B2C8_SPI_SHADER_USER_ACCUM_ESGS_0,
      R_00B2D4_SPI_SHADER_USER_ACCUM_ESGS_3 - R_00B2C8_SPI_SHADER_USER_ACCUM_ESGS_0 + 4,
   },
   {
      R_00B400_SPI_SHADER_PGM_CHKSUM_HS,
      4,
   },
   {
      R_00B420_SPI_SHADER_PGM_LO_HS,
      R_00B4AC_SPI_SHADER_USER_DATA_HS_31 - R_00B420_SPI_SHADER_PGM_LO_HS + 4,
   },
   {
      R_00B4C8_SPI_SHADER_USER_ACCUM_LSHS_0,
      R_00B4D4_SPI_SHADER_USER_ACCUM_LSHS_3 - R_00B4C8_SPI_SHADER_USER_ACCUM_LSHS_0 + 4,
   },
   {
      R_00B0C0_SPI_SHADER_REQ_CTRL_PS,
      4,
   },
   {
      R_00B1C0_SPI_SHADER_REQ_CTRL_VS,
      4,
   },
};

static const struct ac_reg_range Gfx10CsShShadowRange[] = {
   {
      R_00B810_COMPUTE_START_X,
      R_00B824_COMPUTE_NUM_THREAD_Z - R_00B810_COMPUTE_START_X + 4,
   },
   {
      R_00B82C_COMPUTE_PERFCOUNT_ENABLE,
      R_00B834_COMPUTE_PGM_HI - R_00B82C_COMPUTE_PERFCOUNT_ENABLE + 4,
   },
   {
      R_00B848_COMPUTE_PGM_RSRC1,
      R_00B84C_COMPUTE_PGM_RSRC2 - R_00B848_COMPUTE_PGM_RSRC1 + 4,
   },
   {
      R_00B854_COMPUTE_RESOURCE_LIMITS,
      4,
   },
   {
      R_00B860_COMPUTE_TMPRING_SIZE,
      4,
   },
   {
      R_00B878_COMPUTE_THREAD_TRACE_ENABLE,
      4,
   },
   {
      R_00B890_COMPUTE_USER_ACCUM_0,
      R_00B8A0_COMPUTE_PGM_RSRC3 - R_00B890_COMPUTE_USER_ACCUM_0 + 4,
   },
   {
      R_00B8A8_COMPUTE_SHADER_CHKSUM,
      4,
   },
   {
      R_00B900_COMPUTE_USER_DATA_0,
      R_00B93C_COMPUTE_USER_DATA_15 - R_00B900_COMPUTE_USER_DATA_0 + 4,
   },
   {
      R_00B9F4_COMPUTE_DISPATCH_TUNNEL,
      4,
   },
};

static const struct ac_reg_range Gfx103ContextShadowRange[] = {
   {
      R_028000_DB_RENDER_CONTROL,
      R_028084_TA_BC_BASE_ADDR_HI - R_028000_DB_RENDER_CONTROL + 4,
   },
   {
      R_0281E8_COHER_DEST_BASE_HI_0,
      R_02835C_PA_SC_TILE_STEERING_OVERRIDE - R_0281E8_COHER_DEST_BASE_HI_0 + 4,
   },
   {
      R_02840C_VGT_MULTI_PRIM_IB_RESET_INDX,
      R_028618_PA_CL_UCP_5_W - R_02840C_VGT_MULTI_PRIM_IB_RESET_INDX + 4,
   },
   {
      R_028644_SPI_PS_INPUT_CNTL_0,
      R_028714_SPI_SHADER_COL_FORMAT - R_028644_SPI_PS_INPUT_CNTL_0 + 4,
   },
   {
      R_028750_SX_PS_DOWNCONVERT_CONTROL,
      R_02879C_CB_BLEND7_CONTROL - R_028750_SX_PS_DOWNCONVERT_CONTROL + 4,
   },
   {
      R_0287D4_PA_CL_POINT_X_RAD,
      R_0287E0_PA_CL_POINT_CULL_RAD - R_0287D4_PA_CL_POINT_X_RAD + 4,
   },
   {
      R_0287FC_GE_MAX_OUTPUT_PER_SUBGROUP,
      R_028848_PA_CL_VRS_CNTL - R_0287FC_GE_MAX_OUTPUT_PER_SUBGROUP + 4,
   },
   {
      R_028A00_PA_SU_POINT_SIZE,
      R_028A0C_PA_SC_LINE_STIPPLE - R_028A00_PA_SU_POINT_SIZE + 4,
   },
   {
      R_028A18_VGT_HOS_MAX_TESS_LEVEL,
      R_028A1C_VGT_HOS_MIN_TESS_LEVEL - R_028A18_VGT_HOS_MAX_TESS_LEVEL + 4,
   },
   {
      R_028A40_VGT_GS_MODE,
      R_028A6C_VGT_GS_OUT_PRIM_TYPE - R_028A40_VGT_GS_MODE + 4,
   },
   {
      R_028A84_VGT_PRIMITIVEID_EN,
      4,
   },
   {
      R_028A8C_VGT_PRIMITIVEID_RESET,
      4,
   },
   {
      R_028A98_VGT_DRAW_PAYLOAD_CNTL,
      R_028B98_VGT_STRMOUT_BUFFER_CONFIG - R_028A98_VGT_DRAW_PAYLOAD_CNTL + 4,
   },
   {
      R_028BD4_PA_SC_CENTROID_PRIORITY_0,
      R_028EFC_CB_COLOR7_ATTRIB3 - R_028BD4_PA_SC_CENTROID_PRIORITY_0 + 4,
   },
};

static const struct ac_reg_range Gfx103UserConfigShadowRange[] = {
   {
      R_0300FC_CP_STRMOUT_CNTL,
      4,
   },
   {
      R_0301EC_CP_COHER_START_DELAY,
      4,
   },
   {
      R_030904_VGT_GSVS_RING_SIZE,
      R_030908_VGT_PRIMITIVE_TYPE - R_030904_VGT_GSVS_RING_SIZE + 4,
   },
   {
      R_030964_GE_MAX_VTX_INDX,
      4,
   },
   {
      R_030924_GE_MIN_VTX_INDX,
      R_03092C_GE_MULTI_PRIM_IB_RESET_EN - R_030924_GE_MIN_VTX_INDX + 4,
   },
   {
      R_030934_VGT_NUM_INSTANCES,
      R_030940_VGT_TF_MEMORY_BASE - R_030934_VGT_NUM_INSTANCES + 4,
   },
   {
      R_03097C_GE_STEREO_CNTL,
      R_030984_VGT_TF_MEMORY_BASE_HI - R_03097C_GE_STEREO_CNTL + 4,
   },
   {
      R_03096C_GE_CNTL,
      4,
   },
   {
      R_030968_VGT_INSTANCE_BASE_ID,
      4,
   },
   {
      R_030E00_TA_CS_BC_BASE_ADDR,
      R_030E04_TA_CS_BC_BASE_ADDR_HI - R_030E00_TA_CS_BC_BASE_ADDR + 4,
   },
   {
      R_030988_GE_USER_VGPR_EN,
      0x03098C - R_030988_GE_USER_VGPR_EN + 4,
   },
};

static const struct ac_reg_range Gfx11ShShadowRange[] =
{
   /* First register                            Count * 4      Last register */
   { R_00B004_SPI_SHADER_PGM_RSRC4_PS,             1  * 4}, // SPI_SHADER_PGM_RSRC4_PS
   { R_00B018_SPI_SHADER_PGM_CHKSUM_PS,            38 * 4}, // SPI_SHADER_USER_DATA_PS_31
   { R_00B0C0_SPI_SHADER_REQ_CTRL_PS,              1  * 4}, // SPI_SHADER_REQ_CTRL_PS
   { R_00B0C8_SPI_SHADER_USER_ACCUM_PS_0,          4  * 4}, // SPI_SHADER_USER_ACCUM_PS_3
   { R_00B200_SPI_SHADER_PGM_CHKSUM_GS,            2  * 4}, // SPI_SHADER_PGM_RSRC4_GS
   { R_00B21C_SPI_SHADER_PGM_RSRC3_GS,             39 * 4}, // SPI_SHADER_GS_MESHLET_EXP_ALLOC
   { R_00B2C8_SPI_SHADER_USER_ACCUM_ESGS_0,        4  * 4}, // SPI_SHADER_USER_ACCUM_ESGS_3
   { R_00B320_SPI_SHADER_PGM_LO_ES,                2  * 4}, // SPI_SHADER_PGM_HI_ES
   { R_00B400_SPI_SHADER_PGM_CHKSUM_HS,            2  * 4}, // SPI_SHADER_PGM_RSRC4_HS
   { R_00B41C_SPI_SHADER_PGM_RSRC3_HS,             37 * 4}, // SPI_SHADER_USER_DATA_HS_31
   { R_00B4C8_SPI_SHADER_USER_ACCUM_LSHS_0,        4  * 4}, // SPI_SHADER_USER_ACCUM_LSHS_3
   { R_00B520_SPI_SHADER_PGM_LO_LS,                2  * 4}  // SPI_SHADER_PGM_HI_LS
};

static const struct ac_reg_range Gfx11CsShShadowRange[] =
{
   /* First register                            Count * 4      Last register */
   { R_00B810_COMPUTE_START_X,                     6  * 4}, // COMPUTE_NUM_THREAD_Z
   { R_00B82C_COMPUTE_PERFCOUNT_ENABLE,            5  * 4}, // COMPUTE_DISPATCH_PKT_ADDR_HI
   { R_00B840_COMPUTE_DISPATCH_SCRATCH_BASE_LO,    4  * 4}, // COMPUTE_PGM_RSRC2
   { R_00B854_COMPUTE_RESOURCE_LIMITS,             6  * 4}, // COMPUTE_STATIC_THREAD_MGMT_SE3
   { R_00B878_COMPUTE_THREAD_TRACE_ENABLE,         1  * 4}, // COMPUTE_THREAD_TRACE_ENABLE
   { R_00B890_COMPUTE_USER_ACCUM_0,                5  * 4}, // COMPUTE_PGM_RSRC3
   { R_00B8A8_COMPUTE_SHADER_CHKSUM,               6  * 4}, // COMPUTE_DISPATCH_INTERLEAVE
   { R_00B900_COMPUTE_USER_DATA_0,                 16 * 4}, // COMPUTE_USER_DATA_15
   { R_00B9F4_COMPUTE_DISPATCH_TUNNEL,             1  * 4}  // COMPUTE_DISPATCH_TUNNEL
};

/* Defines the set of ranges of context registers we shadow when mid command buffer preemption
 * is enabled.
 */
static const struct ac_reg_range Gfx11ContextShadowRange[] =
{
   /* First register                            Count * 4      Last register */
   { R_028000_DB_RENDER_CONTROL,                  6   * 4}, // DB_HTILE_DATA_BASE
   { R_02801C_DB_DEPTH_SIZE_XY,                   7   * 4}, // PA_SC_SCREEN_SCISSOR_BR
   { R_02803C_DB_RESERVED_REG_2,                  10  * 4}, // DB_SPI_VRS_CENTER_LOCATION
   { R_028068_DB_Z_READ_BASE_HI,                  8   * 4}, // TA_BC_BASE_ADDR_HI
   { R_0281E8_COHER_DEST_BASE_HI_0,               94  * 4}, // PA_SC_TILE_STEERING_OVERRIDE
   { R_0283D0_PA_SC_VRS_OVERRIDE_CNTL,            4   * 4}, // PA_SC_VRS_RATE_FEEDBACK_SIZE_XY
   { R_0283E4_PA_SC_VRS_RATE_CACHE_CNTL,          1   * 4}, // PA_SC_VRS_RATE_CACHE_CNTL
   { R_0283F0_PA_SC_VRS_RATE_BASE,                3   * 4}, // PA_SC_VRS_RATE_SIZE_XY
   { R_02840C_VGT_MULTI_PRIM_IB_RESET_INDX,       11  * 4}, // DB_STENCILREFMASK_BF
   { R_02843C_PA_CL_VPORT_XSCALE,                 120 * 4}, // PA_CL_UCP_5_W
   { R_02861C_PA_CL_PROG_NEAR_CLIP_Z,             2   * 4}, // PA_RATE_CNTL - not shadowed by RS64 !!!
   { R_028644_SPI_PS_INPUT_CNTL_0,                33  * 4}, // SPI_VS_OUT_CONFIG
   { R_0286CC_SPI_PS_INPUT_ENA,                   6   * 4}, // SPI_BARYC_CNTL
   { R_0286E8_SPI_TMPRING_SIZE,                   3   * 4}, // SPI_GFX_SCRATCH_BASE_HI
   { R_028708_SPI_SHADER_IDX_FORMAT,              4   * 4}, // SPI_SHADER_COL_FORMAT
   { R_028750_SX_PS_DOWNCONVERT_CONTROL,          20  * 4}, // CB_BLEND7_CONTROL
   { R_0287D4_PA_CL_POINT_X_RAD,                  4   * 4}, // PA_CL_POINT_CULL_RAD
   { R_0287FC_GE_MAX_OUTPUT_PER_SUBGROUP,         14  * 4}, // PA_SU_SMALL_PRIM_FILTER_CNTL
   { R_028838_PA_CL_NGG_CNTL,                     5   * 4}, // PA_CL_VRS_CNTL
   { R_028A00_PA_SU_POINT_SIZE,                   4   * 4}, // PA_SC_LINE_STIPPLE
   { R_028A18_VGT_HOS_MAX_TESS_LEVEL,             2   * 4}, // VGT_HOS_MIN_TESS_LEVEL
   { R_028A48_PA_SC_MODE_CNTL_0,                  3   * 4}, // VGT_ENHANCE
   { R_028A84_VGT_PRIMITIVEID_EN,                 1   * 4}, // VGT_PRIMITIVEID_EN
   { R_028A8C_VGT_PRIMITIVEID_RESET,              1   * 4}, // VGT_PRIMITIVEID_RESET
   { R_028A98_VGT_DRAW_PAYLOAD_CNTL,              1   * 4}, // VGT_DRAW_PAYLOAD_CNTL
   { R_028AAC_VGT_ESGS_RING_ITEMSIZE,             1   * 4}, // VGT_ESGS_RING_ITEMSIZE
   { R_028AB4_VGT_REUSE_OFF,                      1   * 4}, // VGT_REUSE_OFF
   { R_028ABC_DB_HTILE_SURFACE,                   4   * 4}, // DB_PRELOAD_CONTROL
   { R_028B28_VGT_STRMOUT_DRAW_OPAQUE_OFFSET,     3   * 4}, // VGT_STRMOUT_DRAW_OPAQUE_VERTEX_STRIDE
   { R_028B38_VGT_GS_MAX_VERT_OUT,                1   * 4}, // VGT_GS_MAX_VERT_OUT
   { R_028B4C_GE_NGG_SUBGRP_CNTL,                 4   * 4}, // VGT_LS_HS_CONFIG
   { R_028B6C_VGT_TF_PARAM,                       2   * 4}, // DB_ALPHA_TO_MASK
   { R_028B78_PA_SU_POLY_OFFSET_DB_FMT_CNTL,      7   * 4}, // VGT_GS_INSTANCE_CNT
   { R_028BD4_PA_SC_CENTROID_PRIORITY_0,          33  * 4}, // PA_SC_BINNER_CNTL_2
   { R_028C60_CB_COLOR0_BASE,                     1   * 4}, // CB_COLOR0_BASE
   { R_028C6C_CB_COLOR0_VIEW,                     4   * 4}, // CB_COLOR0_FDCC_CONTROL
   { R_028C94_CB_COLOR0_DCC_BASE,                 1   * 4}, // CB_COLOR0_DCC_BASE
   { R_028C9C_CB_COLOR1_BASE,                     1   * 4}, // CB_COLOR1_BASE
   { R_028CA8_CB_COLOR1_VIEW,                     4   * 4}, // CB_COLOR1_FDCC_CONTROL
   { R_028CD0_CB_COLOR1_DCC_BASE,                 1   * 4}, // CB_COLOR1_DCC_BASE
   { R_028CD8_CB_COLOR2_BASE,                     1   * 4}, // CB_COLOR2_BASE
   { R_028CE4_CB_COLOR2_VIEW,                     4   * 4}, // CB_COLOR2_FDCC_CONTROL
   { R_028D0C_CB_COLOR2_DCC_BASE,                 1   * 4}, // CB_COLOR2_DCC_BASE
   { R_028D14_CB_COLOR3_BASE,                     1   * 4}, // CB_COLOR3_BASE
   { R_028D20_CB_COLOR3_VIEW,                     4   * 4}, // CB_COLOR3_FDCC_CONTROL
   { R_028D48_CB_COLOR3_DCC_BASE,                 1   * 4}, // CB_COLOR3_DCC_BASE
   { R_028D50_CB_COLOR4_BASE,                     1   * 4}, // CB_COLOR4_BASE
   { R_028D5C_CB_COLOR4_VIEW,                     4   * 4}, // CB_COLOR4_FDCC_CONTROL
   { R_028D84_CB_COLOR4_DCC_BASE,                 1   * 4}, // CB_COLOR4_DCC_BASE
   { R_028D8C_CB_COLOR5_BASE,                     1   * 4}, // CB_COLOR5_BASE
   { R_028D98_CB_COLOR5_VIEW,                     4   * 4}, // CB_COLOR5_FDCC_CONTROL
   { R_028DC0_CB_COLOR5_DCC_BASE,                 1   * 4}, // CB_COLOR5_DCC_BASE
   { R_028DC8_CB_COLOR6_BASE,                     1   * 4}, // CB_COLOR6_BASE
   { R_028DD4_CB_COLOR6_VIEW,                     4   * 4}, // CB_COLOR6_FDCC_CONTROL
   { R_028DFC_CB_COLOR6_DCC_BASE,                 1   * 4}, // CB_COLOR6_DCC_BASE
   { R_028E04_CB_COLOR7_BASE,                     1   * 4}, // CB_COLOR7_BASE
   { R_028E10_CB_COLOR7_VIEW,                     4   * 4}, // CB_COLOR7_FDCC_CONTROL
   { R_028E38_CB_COLOR7_DCC_BASE,                 1   * 4}, // CB_COLOR7_DCC_BASE
   { R_028E40_CB_COLOR0_BASE_EXT,                 8   * 4}, // CB_COLOR7_BASE_EXT
   { R_028EA0_CB_COLOR0_DCC_BASE_EXT,             24  * 4}  // CB_COLOR7_ATTRIB3
};

static const struct ac_reg_range Gfx11UserConfigShadowRange[] =
{
   /* First register                            Count * 4      Last register */
   { R_030908_VGT_PRIMITIVE_TYPE,                   1 * 4}, // VGT_PRIMITIVE_TYPE
   { R_030924_GE_MIN_VTX_INDX,                      3 * 4}, // GE_MULTI_PRIM_IB_RESET_EN
   { R_030934_VGT_NUM_INSTANCES,                    4 * 4}, // VGT_TF_MEMORY_BASE
   { R_030964_GE_MAX_VTX_INDX,                      3 * 4}, // GE_CNTL
   { R_03097C_GE_STEREO_CNTL,                       5 * 4}, // GE_VRS_RATE
   { R_030998_VGT_GS_OUT_PRIM_TYPE,                 1 * 4}, // VGT_GS_OUT_PRIM_TYPE
   { R_030A00_PA_SU_LINE_STIPPLE_VALUE,             2 * 4}, // PA_SC_LINE_STIPPLE_STATE - not shadowed by RS64
   { R_030E00_TA_CS_BC_BASE_ADDR,                   2 * 4}, // TA_CS_BC_BASE_ADDR_HI
   { R_031110_SPI_GS_THROTTLE_CNTL1,                4 * 4}, // SPI_ATTRIBUTE_RING_SIZE
   /* GDS_STRMOUT_* registers are not listed because they are modified outside of the command buffer,
    * so they have to be shadowed differently by firmware.
  . */
};

void ac_get_reg_ranges(enum amd_gfx_level gfx_level, enum radeon_family family,
                       enum ac_reg_range_type type, unsigned *num_ranges,
                       const struct ac_reg_range **ranges)
{
#define RETURN(array)                                                                              \
   do {                                                                                            \
      *ranges = array;                                                                             \
      *num_ranges = ARRAY_SIZE(array);                                                             \
   } while (0)

   *num_ranges = 0;
   *ranges = NULL;

   switch (type) {
   case SI_REG_RANGE_UCONFIG:
      if (gfx_level == GFX11)
         RETURN(Gfx11UserConfigShadowRange);
      else if (gfx_level == GFX10_3)
         RETURN(Gfx103UserConfigShadowRange);
      else if (gfx_level == GFX10)
         RETURN(Nv10UserConfigShadowRange);
      else if (gfx_level == GFX9)
         RETURN(Gfx9UserConfigShadowRange);
      break;
   case SI_REG_RANGE_CONTEXT:
      if (gfx_level == GFX11)
         RETURN(Gfx11ContextShadowRange);
      else if (gfx_level == GFX10_3)
         RETURN(Gfx103ContextShadowRange);
      else if (gfx_level == GFX10)
         RETURN(Nv10ContextShadowRange);
      else if (gfx_level == GFX9)
         RETURN(Gfx9ContextShadowRange);
      break;
   case SI_REG_RANGE_SH:
      if (gfx_level == GFX11)
         RETURN(Gfx11ShShadowRange);
      else if (gfx_level == GFX10_3 || gfx_level == GFX10)
         RETURN(Gfx10ShShadowRange);
      else if (family == CHIP_RAVEN2 || family == CHIP_RENOIR)
         RETURN(Gfx9ShShadowRangeRaven2);
      else if (gfx_level == GFX9)
         RETURN(Gfx9ShShadowRange);
      break;
   case SI_REG_RANGE_CS_SH:
      if (gfx_level == GFX11)
         RETURN(Gfx11CsShShadowRange);
      else if (gfx_level == GFX10_3 || gfx_level == GFX10)
         RETURN(Gfx10CsShShadowRange);
      else if (family == CHIP_RAVEN2 || family == CHIP_RENOIR)
         RETURN(Gfx9CsShShadowRangeRaven2);
      else if (gfx_level == GFX9)
         RETURN(Gfx9CsShShadowRange);
      break;
   default:
      break;
   }
}

/**
 * Emulate CLEAR_STATE.
 */
static void gfx9_emulate_clear_state(struct radeon_cmdbuf *cs,
                                     set_context_reg_seq_array_fn set_context_reg_seq_array)
{
   static const uint32_t DbRenderControlGfx9[] = {
      0x0,        // DB_RENDER_CONTROL
      0x0,        // DB_COUNT_CONTROL
      0x0,        // DB_DEPTH_VIEW
      0x0,        // DB_RENDER_OVERRIDE
      0x0,        // DB_RENDER_OVERRIDE2
      0x0,        // DB_HTILE_DATA_BASE
      0x0,        // DB_HTILE_DATA_BASE_HI
      0x0,        // DB_DEPTH_SIZE
      0x0,        // DB_DEPTH_BOUNDS_MIN
      0x0,        // DB_DEPTH_BOUNDS_MAX
      0x0,        // DB_STENCIL_CLEAR
      0x0,        // DB_DEPTH_CLEAR
      0x0,        // PA_SC_SCREEN_SCISSOR_TL
      0x40004000, // PA_SC_SCREEN_SCISSOR_BR
      0x0,        // DB_Z_INFO
      0x0,        // DB_STENCIL_INFO
      0x0,        // DB_Z_READ_BASE
      0x0,        // DB_Z_READ_BASE_HI
      0x0,        // DB_STENCIL_READ_BASE
      0x0,        // DB_STENCIL_READ_BASE_HI
      0x0,        // DB_Z_WRITE_BASE
      0x0,        // DB_Z_WRITE_BASE_HI
      0x0,        // DB_STENCIL_WRITE_BASE
      0x0,        // DB_STENCIL_WRITE_BASE_HI
      0x0,        // DB_DFSM_CONTROL
      0x0,        //
      0x0,        // DB_Z_INFO2
      0x0,        // DB_STENCIL_INFO2
      0x0,        //
      0x0,        //
      0x0,        //
      0x0,        //
      0x0,        // TA_BC_BASE_ADDR
      0x0         // TA_BC_BASE_ADDR_HI
   };
   static const uint32_t CoherDestBaseHi0Gfx9[] = {
      0x0,        // COHER_DEST_BASE_HI_0
      0x0,        // COHER_DEST_BASE_HI_1
      0x0,        // COHER_DEST_BASE_HI_2
      0x0,        // COHER_DEST_BASE_HI_3
      0x0,        // COHER_DEST_BASE_2
      0x0,        // COHER_DEST_BASE_3
      0x0,        // PA_SC_WINDOW_OFFSET
      0x80000000, // PA_SC_WINDOW_SCISSOR_TL
      0x40004000, // PA_SC_WINDOW_SCISSOR_BR
      0xffff,     // PA_SC_CLIPRECT_RULE
      0x0,        // PA_SC_CLIPRECT_0_TL
      0x40004000, // PA_SC_CLIPRECT_0_BR
      0x0,        // PA_SC_CLIPRECT_1_TL
      0x40004000, // PA_SC_CLIPRECT_1_BR
      0x0,        // PA_SC_CLIPRECT_2_TL
      0x40004000, // PA_SC_CLIPRECT_2_BR
      0x0,        // PA_SC_CLIPRECT_3_TL
      0x40004000, // PA_SC_CLIPRECT_3_BR
      0xaa99aaaa, // PA_SC_EDGERULE
      0x0,        // PA_SU_HARDWARE_SCREEN_OFFSET
      0xffffffff, // CB_TARGET_MASK
      0xffffffff, // CB_SHADER_MASK
      0x80000000, // PA_SC_GENERIC_SCISSOR_TL
      0x40004000, // PA_SC_GENERIC_SCISSOR_BR
      0x0,        // COHER_DEST_BASE_0
      0x0,        // COHER_DEST_BASE_1
      0x80000000, // PA_SC_VPORT_SCISSOR_0_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_0_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_1_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_1_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_2_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_2_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_3_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_3_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_4_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_4_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_5_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_5_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_6_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_6_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_7_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_7_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_8_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_8_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_9_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_9_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_10_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_10_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_11_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_11_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_12_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_12_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_13_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_13_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_14_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_14_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_15_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_15_BR
      0x0,        // PA_SC_VPORT_ZMIN_0
      0x3f800000, // PA_SC_VPORT_ZMAX_0
      0x0,        // PA_SC_VPORT_ZMIN_1
      0x3f800000, // PA_SC_VPORT_ZMAX_1
      0x0,        // PA_SC_VPORT_ZMIN_2
      0x3f800000, // PA_SC_VPORT_ZMAX_2
      0x0,        // PA_SC_VPORT_ZMIN_3
      0x3f800000, // PA_SC_VPORT_ZMAX_3
      0x0,        // PA_SC_VPORT_ZMIN_4
      0x3f800000, // PA_SC_VPORT_ZMAX_4
      0x0,        // PA_SC_VPORT_ZMIN_5
      0x3f800000, // PA_SC_VPORT_ZMAX_5
      0x0,        // PA_SC_VPORT_ZMIN_6
      0x3f800000, // PA_SC_VPORT_ZMAX_6
      0x0,        // PA_SC_VPORT_ZMIN_7
      0x3f800000, // PA_SC_VPORT_ZMAX_7
      0x0,        // PA_SC_VPORT_ZMIN_8
      0x3f800000, // PA_SC_VPORT_ZMAX_8
      0x0,        // PA_SC_VPORT_ZMIN_9
      0x3f800000, // PA_SC_VPORT_ZMAX_9
      0x0,        // PA_SC_VPORT_ZMIN_10
      0x3f800000, // PA_SC_VPORT_ZMAX_10
      0x0,        // PA_SC_VPORT_ZMIN_11
      0x3f800000, // PA_SC_VPORT_ZMAX_11
      0x0,        // PA_SC_VPORT_ZMIN_12
      0x3f800000, // PA_SC_VPORT_ZMAX_12
      0x0,        // PA_SC_VPORT_ZMIN_13
      0x3f800000, // PA_SC_VPORT_ZMAX_13
      0x0,        // PA_SC_VPORT_ZMIN_14
      0x3f800000, // PA_SC_VPORT_ZMAX_14
      0x0,        // PA_SC_VPORT_ZMIN_15
      0x3f800000, // PA_SC_VPORT_ZMAX_15
      0x0,        // PA_SC_RASTER_CONFIG
      0x0,        // PA_SC_RASTER_CONFIG_1
      0x0,        //
      0x0         // PA_SC_TILE_STEERING_OVERRIDE
   };
   static const uint32_t VgtMultiPrimIbResetIndxGfx9[] = {
      0x0 // VGT_MULTI_PRIM_IB_RESET_INDX
   };
   static const uint32_t CbBlendRedGfx9[] = {
      0x0,       // CB_BLEND_RED
      0x0,       // CB_BLEND_GREEN
      0x0,       // CB_BLEND_BLUE
      0x0,       // CB_BLEND_ALPHA
      0x0,       // CB_DCC_CONTROL
      0x0,       //
      0x0,       // DB_STENCIL_CONTROL
      0x1000000, // DB_STENCILREFMASK
      0x1000000, // DB_STENCILREFMASK_BF
      0x0,       //
      0x0,       // PA_CL_VPORT_XSCALE
      0x0,       // PA_CL_VPORT_XOFFSET
      0x0,       // PA_CL_VPORT_YSCALE
      0x0,       // PA_CL_VPORT_YOFFSET
      0x0,       // PA_CL_VPORT_ZSCALE
      0x0,       // PA_CL_VPORT_ZOFFSET
      0x0,       // PA_CL_VPORT_XSCALE_1
      0x0,       // PA_CL_VPORT_XOFFSET_1
      0x0,       // PA_CL_VPORT_YSCALE_1
      0x0,       // PA_CL_VPORT_YOFFSET_1
      0x0,       // PA_CL_VPORT_ZSCALE_1
      0x0,       // PA_CL_VPORT_ZOFFSET_1
      0x0,       // PA_CL_VPORT_XSCALE_2
      0x0,       // PA_CL_VPORT_XOFFSET_2
      0x0,       // PA_CL_VPORT_YSCALE_2
      0x0,       // PA_CL_VPORT_YOFFSET_2
      0x0,       // PA_CL_VPORT_ZSCALE_2
      0x0,       // PA_CL_VPORT_ZOFFSET_2
      0x0,       // PA_CL_VPORT_XSCALE_3
      0x0,       // PA_CL_VPORT_XOFFSET_3
      0x0,       // PA_CL_VPORT_YSCALE_3
      0x0,       // PA_CL_VPORT_YOFFSET_3
      0x0,       // PA_CL_VPORT_ZSCALE_3
      0x0,       // PA_CL_VPORT_ZOFFSET_3
      0x0,       // PA_CL_VPORT_XSCALE_4
      0x0,       // PA_CL_VPORT_XOFFSET_4
      0x0,       // PA_CL_VPORT_YSCALE_4
      0x0,       // PA_CL_VPORT_YOFFSET_4
      0x0,       // PA_CL_VPORT_ZSCALE_4
      0x0,       // PA_CL_VPORT_ZOFFSET_4
      0x0,       // PA_CL_VPORT_XSCALE_5
      0x0,       // PA_CL_VPORT_XOFFSET_5
      0x0,       // PA_CL_VPORT_YSCALE_5
      0x0,       // PA_CL_VPORT_YOFFSET_5
      0x0,       // PA_CL_VPORT_ZSCALE_5
      0x0,       // PA_CL_VPORT_ZOFFSET_5
      0x0,       // PA_CL_VPORT_XSCALE_6
      0x0,       // PA_CL_VPORT_XOFFSET_6
      0x0,       // PA_CL_VPORT_YSCALE_6
      0x0,       // PA_CL_VPORT_YOFFSET_6
      0x0,       // PA_CL_VPORT_ZSCALE_6
      0x0,       // PA_CL_VPORT_ZOFFSET_6
      0x0,       // PA_CL_VPORT_XSCALE_7
      0x0,       // PA_CL_VPORT_XOFFSET_7
      0x0,       // PA_CL_VPORT_YSCALE_7
      0x0,       // PA_CL_VPORT_YOFFSET_7
      0x0,       // PA_CL_VPORT_ZSCALE_7
      0x0,       // PA_CL_VPORT_ZOFFSET_7
      0x0,       // PA_CL_VPORT_XSCALE_8
      0x0,       // PA_CL_VPORT_XOFFSET_8
      0x0,       // PA_CL_VPORT_YSCALE_8
      0x0,       // PA_CL_VPORT_YOFFSET_8
      0x0,       // PA_CL_VPORT_ZSCALE_8
      0x0,       // PA_CL_VPORT_ZOFFSET_8
      0x0,       // PA_CL_VPORT_XSCALE_9
      0x0,       // PA_CL_VPORT_XOFFSET_9
      0x0,       // PA_CL_VPORT_YSCALE_9
      0x0,       // PA_CL_VPORT_YOFFSET_9
      0x0,       // PA_CL_VPORT_ZSCALE_9
      0x0,       // PA_CL_VPORT_ZOFFSET_9
      0x0,       // PA_CL_VPORT_XSCALE_10
      0x0,       // PA_CL_VPORT_XOFFSET_10
      0x0,       // PA_CL_VPORT_YSCALE_10
      0x0,       // PA_CL_VPORT_YOFFSET_10
      0x0,       // PA_CL_VPORT_ZSCALE_10
      0x0,       // PA_CL_VPORT_ZOFFSET_10
      0x0,       // PA_CL_VPORT_XSCALE_11
      0x0,       // PA_CL_VPORT_XOFFSET_11
      0x0,       // PA_CL_VPORT_YSCALE_11
      0x0,       // PA_CL_VPORT_YOFFSET_11
      0x0,       // PA_CL_VPORT_ZSCALE_11
      0x0,       // PA_CL_VPORT_ZOFFSET_11
      0x0,       // PA_CL_VPORT_XSCALE_12
      0x0,       // PA_CL_VPORT_XOFFSET_12
      0x0,       // PA_CL_VPORT_YSCALE_12
      0x0,       // PA_CL_VPORT_YOFFSET_12
      0x0,       // PA_CL_VPORT_ZSCALE_12
      0x0,       // PA_CL_VPORT_ZOFFSET_12
      0x0,       // PA_CL_VPORT_XSCALE_13
      0x0,       // PA_CL_VPORT_XOFFSET_13
      0x0,       // PA_CL_VPORT_YSCALE_13
      0x0,       // PA_CL_VPORT_YOFFSET_13
      0x0,       // PA_CL_VPORT_ZSCALE_13
      0x0,       // PA_CL_VPORT_ZOFFSET_13
      0x0,       // PA_CL_VPORT_XSCALE_14
      0x0,       // PA_CL_VPORT_XOFFSET_14
      0x0,       // PA_CL_VPORT_YSCALE_14
      0x0,       // PA_CL_VPORT_YOFFSET_14
      0x0,       // PA_CL_VPORT_ZSCALE_14
      0x0,       // PA_CL_VPORT_ZOFFSET_14
      0x0,       // PA_CL_VPORT_XSCALE_15
      0x0,       // PA_CL_VPORT_XOFFSET_15
      0x0,       // PA_CL_VPORT_YSCALE_15
      0x0,       // PA_CL_VPORT_YOFFSET_15
      0x0,       // PA_CL_VPORT_ZSCALE_15
      0x0,       // PA_CL_VPORT_ZOFFSET_15
      0x0,       // PA_CL_UCP_0_X
      0x0,       // PA_CL_UCP_0_Y
      0x0,       // PA_CL_UCP_0_Z
      0x0,       // PA_CL_UCP_0_W
      0x0,       // PA_CL_UCP_1_X
      0x0,       // PA_CL_UCP_1_Y
      0x0,       // PA_CL_UCP_1_Z
      0x0,       // PA_CL_UCP_1_W
      0x0,       // PA_CL_UCP_2_X
      0x0,       // PA_CL_UCP_2_Y
      0x0,       // PA_CL_UCP_2_Z
      0x0,       // PA_CL_UCP_2_W
      0x0,       // PA_CL_UCP_3_X
      0x0,       // PA_CL_UCP_3_Y
      0x0,       // PA_CL_UCP_3_Z
      0x0,       // PA_CL_UCP_3_W
      0x0,       // PA_CL_UCP_4_X
      0x0,       // PA_CL_UCP_4_Y
      0x0,       // PA_CL_UCP_4_Z
      0x0,       // PA_CL_UCP_4_W
      0x0,       // PA_CL_UCP_5_X
      0x0,       // PA_CL_UCP_5_Y
      0x0,       // PA_CL_UCP_5_Z
      0x0        // PA_CL_UCP_5_W
   };
   static const uint32_t SpiPsInputCntl0Gfx9[] = {
      0x0, // SPI_PS_INPUT_CNTL_0
      0x0, // SPI_PS_INPUT_CNTL_1
      0x0, // SPI_PS_INPUT_CNTL_2
      0x0, // SPI_PS_INPUT_CNTL_3
      0x0, // SPI_PS_INPUT_CNTL_4
      0x0, // SPI_PS_INPUT_CNTL_5
      0x0, // SPI_PS_INPUT_CNTL_6
      0x0, // SPI_PS_INPUT_CNTL_7
      0x0, // SPI_PS_INPUT_CNTL_8
      0x0, // SPI_PS_INPUT_CNTL_9
      0x0, // SPI_PS_INPUT_CNTL_10
      0x0, // SPI_PS_INPUT_CNTL_11
      0x0, // SPI_PS_INPUT_CNTL_12
      0x0, // SPI_PS_INPUT_CNTL_13
      0x0, // SPI_PS_INPUT_CNTL_14
      0x0, // SPI_PS_INPUT_CNTL_15
      0x0, // SPI_PS_INPUT_CNTL_16
      0x0, // SPI_PS_INPUT_CNTL_17
      0x0, // SPI_PS_INPUT_CNTL_18
      0x0, // SPI_PS_INPUT_CNTL_19
      0x0, // SPI_PS_INPUT_CNTL_20
      0x0, // SPI_PS_INPUT_CNTL_21
      0x0, // SPI_PS_INPUT_CNTL_22
      0x0, // SPI_PS_INPUT_CNTL_23
      0x0, // SPI_PS_INPUT_CNTL_24
      0x0, // SPI_PS_INPUT_CNTL_25
      0x0, // SPI_PS_INPUT_CNTL_26
      0x0, // SPI_PS_INPUT_CNTL_27
      0x0, // SPI_PS_INPUT_CNTL_28
      0x0, // SPI_PS_INPUT_CNTL_29
      0x0, // SPI_PS_INPUT_CNTL_30
      0x0, // SPI_PS_INPUT_CNTL_31
      0x0, // SPI_VS_OUT_CONFIG
      0x0, //
      0x0, // SPI_PS_INPUT_ENA
      0x0, // SPI_PS_INPUT_ADDR
      0x0, // SPI_INTERP_CONTROL_0
      0x2, // SPI_PS_IN_CONTROL
      0x0, //
      0x0, // SPI_BARYC_CNTL
      0x0, //
      0x0, // SPI_TMPRING_SIZE
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, // SPI_SHADER_POS_FORMAT
      0x0, // SPI_SHADER_Z_FORMAT
      0x0  // SPI_SHADER_COL_FORMAT
   };
   static const uint32_t SxPsDownconvertGfx9[] = {
      0x0, // SX_PS_DOWNCONVERT
      0x0, // SX_BLEND_OPT_EPSILON
      0x0, // SX_BLEND_OPT_CONTROL
      0x0, // SX_MRT0_BLEND_OPT
      0x0, // SX_MRT1_BLEND_OPT
      0x0, // SX_MRT2_BLEND_OPT
      0x0, // SX_MRT3_BLEND_OPT
      0x0, // SX_MRT4_BLEND_OPT
      0x0, // SX_MRT5_BLEND_OPT
      0x0, // SX_MRT6_BLEND_OPT
      0x0, // SX_MRT7_BLEND_OPT
      0x0, // CB_BLEND0_CONTROL
      0x0, // CB_BLEND1_CONTROL
      0x0, // CB_BLEND2_CONTROL
      0x0, // CB_BLEND3_CONTROL
      0x0, // CB_BLEND4_CONTROL
      0x0, // CB_BLEND5_CONTROL
      0x0, // CB_BLEND6_CONTROL
      0x0, // CB_BLEND7_CONTROL
      0x0, // CB_MRT0_EPITCH
      0x0, // CB_MRT1_EPITCH
      0x0, // CB_MRT2_EPITCH
      0x0, // CB_MRT3_EPITCH
      0x0, // CB_MRT4_EPITCH
      0x0, // CB_MRT5_EPITCH
      0x0, // CB_MRT6_EPITCH
      0x0  // CB_MRT7_EPITCH
   };
   static const uint32_t DbDepthControlGfx9[] = {
      0x0,     // DB_DEPTH_CONTROL
      0x0,     // DB_EQAA
      0x0,     // CB_COLOR_CONTROL
      0x0,     // DB_SHADER_CONTROL
      0x90000, // PA_CL_CLIP_CNTL
      0x4,     // PA_SU_SC_MODE_CNTL
      0x0,     // PA_CL_VTE_CNTL
      0x0,     // PA_CL_VS_OUT_CNTL
      0x0,     // PA_CL_NANINF_CNTL
      0x0,     // PA_SU_LINE_STIPPLE_CNTL
      0x0,     // PA_SU_LINE_STIPPLE_SCALE
      0x0,     // PA_SU_PRIM_FILTER_CNTL
      0x0,     // PA_SU_SMALL_PRIM_FILTER_CNTL
      0x0,     // PA_CL_OBJPRIM_ID_CNTL
      0x0,     // PA_CL_NGG_CNTL
      0x0,     // PA_SU_OVER_RASTERIZATION_CNTL
      0x0      // PA_STEREO_CNTL
   };
   static const uint32_t PaSuPointSizeGfx9[] = {
      0x0, // PA_SU_POINT_SIZE
      0x0, // PA_SU_POINT_MINMAX
      0x0, // PA_SU_LINE_CNTL
      0x0  // PA_SC_LINE_STIPPLE
   };
   static const uint32_t VgtHosMaxTessLevelGfx9[] = {
      0x0, // VGT_HOS_MAX_TESS_LEVEL
      0x0  // VGT_HOS_MIN_TESS_LEVEL
   };
   static const uint32_t VgtGsModeGfx9[] = {
      0x0,   // VGT_GS_MODE
      0x0,   // VGT_GS_ONCHIP_CNTL
      0x0,   // PA_SC_MODE_CNTL_0
      0x0,   // PA_SC_MODE_CNTL_1
      0x0,   // VGT_ENHANCE
      0x100, // VGT_GS_PER_ES
      0x80,  // VGT_ES_PER_GS
      0x2,   // VGT_GS_PER_VS
      0x0,   // VGT_GSVS_RING_OFFSET_1
      0x0,   // VGT_GSVS_RING_OFFSET_2
      0x0,   // VGT_GSVS_RING_OFFSET_3
      0x0    // VGT_GS_OUT_PRIM_TYPE
   };
   static const uint32_t VgtPrimitiveidEnGfx9[] = {
      0x0 // VGT_PRIMITIVEID_EN
   };
   static const uint32_t VgtPrimitiveidResetGfx9[] = {
      0x0 // VGT_PRIMITIVEID_RESET
   };
   static const uint32_t VgtGsMaxPrimsPerSubgroupGfx9[] = {
      0x0, // VGT_GS_MAX_PRIMS_PER_SUBGROUP
      0x0, // VGT_DRAW_PAYLOAD_CNTL
      0x0, //
      0x0, // VGT_INSTANCE_STEP_RATE_0
      0x0, // VGT_INSTANCE_STEP_RATE_1
      0x0, //
      0x0, // VGT_ESGS_RING_ITEMSIZE
      0x0, // VGT_GSVS_RING_ITEMSIZE
      0x0, // VGT_REUSE_OFF
      0x0, // VGT_VTX_CNT_EN
      0x0, // DB_HTILE_SURFACE
      0x0, // DB_SRESULTS_COMPARE_STATE0
      0x0, // DB_SRESULTS_COMPARE_STATE1
      0x0, // DB_PRELOAD_CONTROL
      0x0, //
      0x0, // VGT_STRMOUT_BUFFER_SIZE_0
      0x0  // VGT_STRMOUT_VTX_STRIDE_0
   };
   static const uint32_t VgtStrmoutBufferSize1Gfx9[] = {
      0x0, // VGT_STRMOUT_BUFFER_SIZE_1
      0x0  // VGT_STRMOUT_VTX_STRIDE_1
   };
   static const uint32_t VgtStrmoutBufferSize2Gfx9[] = {
      0x0, // VGT_STRMOUT_BUFFER_SIZE_2
      0x0  // VGT_STRMOUT_VTX_STRIDE_2
   };
   static const uint32_t VgtStrmoutBufferSize3Gfx9[] = {
      0x0, // VGT_STRMOUT_BUFFER_SIZE_3
      0x0  // VGT_STRMOUT_VTX_STRIDE_3
   };
   static const uint32_t VgtStrmoutDrawOpaqueOffsetGfx9[] = {
      0x0, // VGT_STRMOUT_DRAW_OPAQUE_OFFSET
      0x0, // VGT_STRMOUT_DRAW_OPAQUE_BUFFER_FILLED_SIZE
      0x0  // VGT_STRMOUT_DRAW_OPAQUE_VERTEX_STRIDE
   };
   static const uint32_t VgtGsMaxVertOutGfx9[] = {
      0x0, // VGT_GS_MAX_VERT_OUT
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, // VGT_TESS_DISTRIBUTION
      0x0, // VGT_SHADER_STAGES_EN
      0x0, // VGT_LS_HS_CONFIG
      0x0, // VGT_GS_VERT_ITEMSIZE
      0x0, // VGT_GS_VERT_ITEMSIZE_1
      0x0, // VGT_GS_VERT_ITEMSIZE_2
      0x0, // VGT_GS_VERT_ITEMSIZE_3
      0x0, // VGT_TF_PARAM
      0x0, // DB_ALPHA_TO_MASK
      0x0, // VGT_DISPATCH_DRAW_INDEX
      0x0, // PA_SU_POLY_OFFSET_DB_FMT_CNTL
      0x0, // PA_SU_POLY_OFFSET_CLAMP
      0x0, // PA_SU_POLY_OFFSET_FRONT_SCALE
      0x0, // PA_SU_POLY_OFFSET_FRONT_OFFSET
      0x0, // PA_SU_POLY_OFFSET_BACK_SCALE
      0x0, // PA_SU_POLY_OFFSET_BACK_OFFSET
      0x0, // VGT_GS_INSTANCE_CNT
      0x0, // VGT_STRMOUT_CONFIG
      0x0  // VGT_STRMOUT_BUFFER_CONFIG
   };
   static const uint32_t PaScCentroidPriority0Gfx9[] = {
      0x0,        // PA_SC_CENTROID_PRIORITY_0
      0x0,        // PA_SC_CENTROID_PRIORITY_1
      0x1000,     // PA_SC_LINE_CNTL
      0x0,        // PA_SC_AA_CONFIG
      0x5,        // PA_SU_VTX_CNTL
      0x3f800000, // PA_CL_GB_VERT_CLIP_ADJ
      0x3f800000, // PA_CL_GB_VERT_DISC_ADJ
      0x3f800000, // PA_CL_GB_HORZ_CLIP_ADJ
      0x3f800000, // PA_CL_GB_HORZ_DISC_ADJ
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_1
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_2
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_3
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_0
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_1
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_2
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_3
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_0
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_1
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_2
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_3
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_0
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_1
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_2
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_3
      0xffffffff, // PA_SC_AA_MASK_X0Y0_X1Y0
      0xffffffff, // PA_SC_AA_MASK_X0Y1_X1Y1
      0x0,        // PA_SC_SHADER_CONTROL
      0x3,        // PA_SC_BINNER_CNTL_0
      0x0,        // PA_SC_BINNER_CNTL_1
      0x100000,   // PA_SC_CONSERVATIVE_RASTERIZATION_CNTL
      0x0,        // PA_SC_NGG_MODE_CNTL
      0x0,        //
      0x1e,       // VGT_VERTEX_REUSE_BLOCK_CNTL
      0x20,       // VGT_OUT_DEALLOC_CNTL
      0x0,        // CB_COLOR0_BASE
      0x0,        // CB_COLOR0_BASE_EXT
      0x0,        // CB_COLOR0_ATTRIB2
      0x0,        // CB_COLOR0_VIEW
      0x0,        // CB_COLOR0_INFO
      0x0,        // CB_COLOR0_ATTRIB
      0x0,        // CB_COLOR0_DCC_CONTROL
      0x0,        // CB_COLOR0_CMASK
      0x0,        // CB_COLOR0_CMASK_BASE_EXT
      0x0,        // CB_COLOR0_FMASK
      0x0,        // CB_COLOR0_FMASK_BASE_EXT
      0x0,        // CB_COLOR0_CLEAR_WORD0
      0x0,        // CB_COLOR0_CLEAR_WORD1
      0x0,        // CB_COLOR0_DCC_BASE
      0x0,        // CB_COLOR0_DCC_BASE_EXT
      0x0,        // CB_COLOR1_BASE
      0x0,        // CB_COLOR1_BASE_EXT
      0x0,        // CB_COLOR1_ATTRIB2
      0x0,        // CB_COLOR1_VIEW
      0x0,        // CB_COLOR1_INFO
      0x0,        // CB_COLOR1_ATTRIB
      0x0,        // CB_COLOR1_DCC_CONTROL
      0x0,        // CB_COLOR1_CMASK
      0x0,        // CB_COLOR1_CMASK_BASE_EXT
      0x0,        // CB_COLOR1_FMASK
      0x0,        // CB_COLOR1_FMASK_BASE_EXT
      0x0,        // CB_COLOR1_CLEAR_WORD0
      0x0,        // CB_COLOR1_CLEAR_WORD1
      0x0,        // CB_COLOR1_DCC_BASE
      0x0,        // CB_COLOR1_DCC_BASE_EXT
      0x0,        // CB_COLOR2_BASE
      0x0,        // CB_COLOR2_BASE_EXT
      0x0,        // CB_COLOR2_ATTRIB2
      0x0,        // CB_COLOR2_VIEW
      0x0,        // CB_COLOR2_INFO
      0x0,        // CB_COLOR2_ATTRIB
      0x0,        // CB_COLOR2_DCC_CONTROL
      0x0,        // CB_COLOR2_CMASK
      0x0,        // CB_COLOR2_CMASK_BASE_EXT
      0x0,        // CB_COLOR2_FMASK
      0x0,        // CB_COLOR2_FMASK_BASE_EXT
      0x0,        // CB_COLOR2_CLEAR_WORD0
      0x0,        // CB_COLOR2_CLEAR_WORD1
      0x0,        // CB_COLOR2_DCC_BASE
      0x0,        // CB_COLOR2_DCC_BASE_EXT
      0x0,        // CB_COLOR3_BASE
      0x0,        // CB_COLOR3_BASE_EXT
      0x0,        // CB_COLOR3_ATTRIB2
      0x0,        // CB_COLOR3_VIEW
      0x0,        // CB_COLOR3_INFO
      0x0,        // CB_COLOR3_ATTRIB
      0x0,        // CB_COLOR3_DCC_CONTROL
      0x0,        // CB_COLOR3_CMASK
      0x0,        // CB_COLOR3_CMASK_BASE_EXT
      0x0,        // CB_COLOR3_FMASK
      0x0,        // CB_COLOR3_FMASK_BASE_EXT
      0x0,        // CB_COLOR3_CLEAR_WORD0
      0x0,        // CB_COLOR3_CLEAR_WORD1
      0x0,        // CB_COLOR3_DCC_BASE
      0x0,        // CB_COLOR3_DCC_BASE_EXT
      0x0,        // CB_COLOR4_BASE
      0x0,        // CB_COLOR4_BASE_EXT
      0x0,        // CB_COLOR4_ATTRIB2
      0x0,        // CB_COLOR4_VIEW
      0x0,        // CB_COLOR4_INFO
      0x0,        // CB_COLOR4_ATTRIB
      0x0,        // CB_COLOR4_DCC_CONTROL
      0x0,        // CB_COLOR4_CMASK
      0x0,        // CB_COLOR4_CMASK_BASE_EXT
      0x0,        // CB_COLOR4_FMASK
      0x0,        // CB_COLOR4_FMASK_BASE_EXT
      0x0,        // CB_COLOR4_CLEAR_WORD0
      0x0,        // CB_COLOR4_CLEAR_WORD1
      0x0,        // CB_COLOR4_DCC_BASE
      0x0,        // CB_COLOR4_DCC_BASE_EXT
      0x0,        // CB_COLOR5_BASE
      0x0,        // CB_COLOR5_BASE_EXT
      0x0,        // CB_COLOR5_ATTRIB2
      0x0,        // CB_COLOR5_VIEW
      0x0,        // CB_COLOR5_INFO
      0x0,        // CB_COLOR5_ATTRIB
      0x0,        // CB_COLOR5_DCC_CONTROL
      0x0,        // CB_COLOR5_CMASK
      0x0,        // CB_COLOR5_CMASK_BASE_EXT
      0x0,        // CB_COLOR5_FMASK
      0x0,        // CB_COLOR5_FMASK_BASE_EXT
      0x0,        // CB_COLOR5_CLEAR_WORD0
      0x0,        // CB_COLOR5_CLEAR_WORD1
      0x0,        // CB_COLOR5_DCC_BASE
      0x0,        // CB_COLOR5_DCC_BASE_EXT
      0x0,        // CB_COLOR6_BASE
      0x0,        // CB_COLOR6_BASE_EXT
      0x0,        // CB_COLOR6_ATTRIB2
      0x0,        // CB_COLOR6_VIEW
      0x0,        // CB_COLOR6_INFO
      0x0,        // CB_COLOR6_ATTRIB
      0x0,        // CB_COLOR6_DCC_CONTROL
      0x0,        // CB_COLOR6_CMASK
      0x0,        // CB_COLOR6_CMASK_BASE_EXT
      0x0,        // CB_COLOR6_FMASK
      0x0,        // CB_COLOR6_FMASK_BASE_EXT
      0x0,        // CB_COLOR6_CLEAR_WORD0
      0x0,        // CB_COLOR6_CLEAR_WORD1
      0x0,        // CB_COLOR6_DCC_BASE
      0x0,        // CB_COLOR6_DCC_BASE_EXT
      0x0,        // CB_COLOR7_BASE
      0x0,        // CB_COLOR7_BASE_EXT
      0x0,        // CB_COLOR7_ATTRIB2
      0x0,        // CB_COLOR7_VIEW
      0x0,        // CB_COLOR7_INFO
      0x0,        // CB_COLOR7_ATTRIB
      0x0,        // CB_COLOR7_DCC_CONTROL
      0x0,        // CB_COLOR7_CMASK
      0x0,        // CB_COLOR7_CMASK_BASE_EXT
      0x0,        // CB_COLOR7_FMASK
      0x0,        // CB_COLOR7_FMASK_BASE_EXT
      0x0,        // CB_COLOR7_CLEAR_WORD0
      0x0,        // CB_COLOR7_CLEAR_WORD1
      0x0,        // CB_COLOR7_DCC_BASE
      0x0         // CB_COLOR7_DCC_BASE_EXT
   };

#define SET(array) ARRAY_SIZE(array), array

   set_context_reg_seq_array(cs, R_028000_DB_RENDER_CONTROL, SET(DbRenderControlGfx9));
   set_context_reg_seq_array(cs, R_0281E8_COHER_DEST_BASE_HI_0, SET(CoherDestBaseHi0Gfx9));
   set_context_reg_seq_array(cs, R_02840C_VGT_MULTI_PRIM_IB_RESET_INDX,
                             SET(VgtMultiPrimIbResetIndxGfx9));
   set_context_reg_seq_array(cs, R_028414_CB_BLEND_RED, SET(CbBlendRedGfx9));
   set_context_reg_seq_array(cs, R_028644_SPI_PS_INPUT_CNTL_0, SET(SpiPsInputCntl0Gfx9));
   set_context_reg_seq_array(cs, R_028754_SX_PS_DOWNCONVERT, SET(SxPsDownconvertGfx9));
   set_context_reg_seq_array(cs, R_028800_DB_DEPTH_CONTROL, SET(DbDepthControlGfx9));
   set_context_reg_seq_array(cs, R_028A00_PA_SU_POINT_SIZE, SET(PaSuPointSizeGfx9));
   set_context_reg_seq_array(cs, R_028A18_VGT_HOS_MAX_TESS_LEVEL, SET(VgtHosMaxTessLevelGfx9));
   set_context_reg_seq_array(cs, R_028A40_VGT_GS_MODE, SET(VgtGsModeGfx9));
   set_context_reg_seq_array(cs, R_028A84_VGT_PRIMITIVEID_EN, SET(VgtPrimitiveidEnGfx9));
   set_context_reg_seq_array(cs, R_028A8C_VGT_PRIMITIVEID_RESET, SET(VgtPrimitiveidResetGfx9));
   set_context_reg_seq_array(cs, R_028A94_VGT_GS_MAX_PRIMS_PER_SUBGROUP,
                             SET(VgtGsMaxPrimsPerSubgroupGfx9));
   set_context_reg_seq_array(cs, R_028AE0_VGT_STRMOUT_BUFFER_SIZE_1,
                             SET(VgtStrmoutBufferSize1Gfx9));
   set_context_reg_seq_array(cs, R_028AF0_VGT_STRMOUT_BUFFER_SIZE_2,
                             SET(VgtStrmoutBufferSize2Gfx9));
   set_context_reg_seq_array(cs, R_028B00_VGT_STRMOUT_BUFFER_SIZE_3,
                             SET(VgtStrmoutBufferSize3Gfx9));
   set_context_reg_seq_array(cs, R_028B28_VGT_STRMOUT_DRAW_OPAQUE_OFFSET,
                             SET(VgtStrmoutDrawOpaqueOffsetGfx9));
   set_context_reg_seq_array(cs, R_028B38_VGT_GS_MAX_VERT_OUT, SET(VgtGsMaxVertOutGfx9));
   set_context_reg_seq_array(cs, R_028BD4_PA_SC_CENTROID_PRIORITY_0,
                             SET(PaScCentroidPriority0Gfx9));
}

/**
 * Emulate CLEAR_STATE. Additionally, initialize num_reg_pairs registers specified
 * via reg_offsets and reg_values.
 */
static void gfx10_emulate_clear_state(struct radeon_cmdbuf *cs, unsigned num_reg_pairs,
                                      unsigned *reg_offsets, uint32_t *reg_values,
                                      set_context_reg_seq_array_fn set_context_reg_seq_array)
{
   static const uint32_t DbRenderControlNv10[] = {
      0x0,        // DB_RENDER_CONTROL
      0x0,        // DB_COUNT_CONTROL
      0x0,        // DB_DEPTH_VIEW
      0x0,        // DB_RENDER_OVERRIDE
      0x0,        // DB_RENDER_OVERRIDE2
      0x0,        // DB_HTILE_DATA_BASE
      0x0,        //
      0x0,        // DB_DEPTH_SIZE_XY
      0x0,        // DB_DEPTH_BOUNDS_MIN
      0x0,        // DB_DEPTH_BOUNDS_MAX
      0x0,        // DB_STENCIL_CLEAR
      0x0,        // DB_DEPTH_CLEAR
      0x0,        // PA_SC_SCREEN_SCISSOR_TL
      0x40004000, // PA_SC_SCREEN_SCISSOR_BR
      0x0,        // DB_DFSM_CONTROL
      0x0,        // DB_RESERVED_REG_2
      0x0,        // DB_Z_INFO
      0x0,        // DB_STENCIL_INFO
      0x0,        // DB_Z_READ_BASE
      0x0,        // DB_STENCIL_READ_BASE
      0x0,        // DB_Z_WRITE_BASE
      0x0,        // DB_STENCIL_WRITE_BASE
      0x0,        //
      0x0,        //
      0x0,        //
      0x0,        //
      0x0,        // DB_Z_READ_BASE_HI
      0x0,        // DB_STENCIL_READ_BASE_HI
      0x0,        // DB_Z_WRITE_BASE_HI
      0x0,        // DB_STENCIL_WRITE_BASE_HI
      0x0,        // DB_HTILE_DATA_BASE_HI
      0x0,        // DB_RMI_L2_CACHE_CONTROL
      0x0,        // TA_BC_BASE_ADDR
      0x0         // TA_BC_BASE_ADDR_HI
   };
   static const uint32_t CoherDestBaseHi0Nv10[] = {
      0x0,        // COHER_DEST_BASE_HI_0
      0x0,        // COHER_DEST_BASE_HI_1
      0x0,        // COHER_DEST_BASE_HI_2
      0x0,        // COHER_DEST_BASE_HI_3
      0x0,        // COHER_DEST_BASE_2
      0x0,        // COHER_DEST_BASE_3
      0x0,        // PA_SC_WINDOW_OFFSET
      0x80000000, // PA_SC_WINDOW_SCISSOR_TL
      0x40004000, // PA_SC_WINDOW_SCISSOR_BR
      0xffff,     // PA_SC_CLIPRECT_RULE
      0x0,        // PA_SC_CLIPRECT_0_TL
      0x40004000, // PA_SC_CLIPRECT_0_BR
      0x0,        // PA_SC_CLIPRECT_1_TL
      0x40004000, // PA_SC_CLIPRECT_1_BR
      0x0,        // PA_SC_CLIPRECT_2_TL
      0x40004000, // PA_SC_CLIPRECT_2_BR
      0x0,        // PA_SC_CLIPRECT_3_TL
      0x40004000, // PA_SC_CLIPRECT_3_BR
      0xaa99aaaa, // PA_SC_EDGERULE
      0x0,        // PA_SU_HARDWARE_SCREEN_OFFSET
      0xffffffff, // CB_TARGET_MASK
      0xffffffff, // CB_SHADER_MASK
      0x80000000, // PA_SC_GENERIC_SCISSOR_TL
      0x40004000, // PA_SC_GENERIC_SCISSOR_BR
      0x0,        // COHER_DEST_BASE_0
      0x0,        // COHER_DEST_BASE_1
      0x80000000, // PA_SC_VPORT_SCISSOR_0_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_0_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_1_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_1_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_2_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_2_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_3_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_3_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_4_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_4_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_5_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_5_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_6_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_6_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_7_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_7_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_8_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_8_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_9_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_9_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_10_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_10_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_11_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_11_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_12_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_12_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_13_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_13_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_14_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_14_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_15_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_15_BR
      0x0,        // PA_SC_VPORT_ZMIN_0
      0x3f800000, // PA_SC_VPORT_ZMAX_0
      0x0,        // PA_SC_VPORT_ZMIN_1
      0x3f800000, // PA_SC_VPORT_ZMAX_1
      0x0,        // PA_SC_VPORT_ZMIN_2
      0x3f800000, // PA_SC_VPORT_ZMAX_2
      0x0,        // PA_SC_VPORT_ZMIN_3
      0x3f800000, // PA_SC_VPORT_ZMAX_3
      0x0,        // PA_SC_VPORT_ZMIN_4
      0x3f800000, // PA_SC_VPORT_ZMAX_4
      0x0,        // PA_SC_VPORT_ZMIN_5
      0x3f800000, // PA_SC_VPORT_ZMAX_5
      0x0,        // PA_SC_VPORT_ZMIN_6
      0x3f800000, // PA_SC_VPORT_ZMAX_6
      0x0,        // PA_SC_VPORT_ZMIN_7
      0x3f800000, // PA_SC_VPORT_ZMAX_7
      0x0,        // PA_SC_VPORT_ZMIN_8
      0x3f800000, // PA_SC_VPORT_ZMAX_8
      0x0,        // PA_SC_VPORT_ZMIN_9
      0x3f800000, // PA_SC_VPORT_ZMAX_9
      0x0,        // PA_SC_VPORT_ZMIN_10
      0x3f800000, // PA_SC_VPORT_ZMAX_10
      0x0,        // PA_SC_VPORT_ZMIN_11
      0x3f800000, // PA_SC_VPORT_ZMAX_11
      0x0,        // PA_SC_VPORT_ZMIN_12
      0x3f800000, // PA_SC_VPORT_ZMAX_12
      0x0,        // PA_SC_VPORT_ZMIN_13
      0x3f800000, // PA_SC_VPORT_ZMAX_13
      0x0,        // PA_SC_VPORT_ZMIN_14
      0x3f800000, // PA_SC_VPORT_ZMAX_14
      0x0,        // PA_SC_VPORT_ZMIN_15
      0x3f800000, // PA_SC_VPORT_ZMAX_15
      0x0,        // PA_SC_RASTER_CONFIG
      0x0,        // PA_SC_RASTER_CONFIG_1
      0x0,        //
      0x0         // PA_SC_TILE_STEERING_OVERRIDE
   };
   static const uint32_t VgtMultiPrimIbResetIndxNv10[] = {
      0x0,       // VGT_MULTI_PRIM_IB_RESET_INDX
      0x0,       // CB_RMI_GL2_CACHE_CONTROL
      0x0,       // CB_BLEND_RED
      0x0,       // CB_BLEND_GREEN
      0x0,       // CB_BLEND_BLUE
      0x0,       // CB_BLEND_ALPHA
      0x0,       // CB_DCC_CONTROL
      0x0,       // CB_COVERAGE_OUT_CONTROL
      0x0,       // DB_STENCIL_CONTROL
      0x1000000, // DB_STENCILREFMASK
      0x1000000, // DB_STENCILREFMASK_BF
      0x0,       //
      0x0,       // PA_CL_VPORT_XSCALE
      0x0,       // PA_CL_VPORT_XOFFSET
      0x0,       // PA_CL_VPORT_YSCALE
      0x0,       // PA_CL_VPORT_YOFFSET
      0x0,       // PA_CL_VPORT_ZSCALE
      0x0,       // PA_CL_VPORT_ZOFFSET
      0x0,       // PA_CL_VPORT_XSCALE_1
      0x0,       // PA_CL_VPORT_XOFFSET_1
      0x0,       // PA_CL_VPORT_YSCALE_1
      0x0,       // PA_CL_VPORT_YOFFSET_1
      0x0,       // PA_CL_VPORT_ZSCALE_1
      0x0,       // PA_CL_VPORT_ZOFFSET_1
      0x0,       // PA_CL_VPORT_XSCALE_2
      0x0,       // PA_CL_VPORT_XOFFSET_2
      0x0,       // PA_CL_VPORT_YSCALE_2
      0x0,       // PA_CL_VPORT_YOFFSET_2
      0x0,       // PA_CL_VPORT_ZSCALE_2
      0x0,       // PA_CL_VPORT_ZOFFSET_2
      0x0,       // PA_CL_VPORT_XSCALE_3
      0x0,       // PA_CL_VPORT_XOFFSET_3
      0x0,       // PA_CL_VPORT_YSCALE_3
      0x0,       // PA_CL_VPORT_YOFFSET_3
      0x0,       // PA_CL_VPORT_ZSCALE_3
      0x0,       // PA_CL_VPORT_ZOFFSET_3
      0x0,       // PA_CL_VPORT_XSCALE_4
      0x0,       // PA_CL_VPORT_XOFFSET_4
      0x0,       // PA_CL_VPORT_YSCALE_4
      0x0,       // PA_CL_VPORT_YOFFSET_4
      0x0,       // PA_CL_VPORT_ZSCALE_4
      0x0,       // PA_CL_VPORT_ZOFFSET_4
      0x0,       // PA_CL_VPORT_XSCALE_5
      0x0,       // PA_CL_VPORT_XOFFSET_5
      0x0,       // PA_CL_VPORT_YSCALE_5
      0x0,       // PA_CL_VPORT_YOFFSET_5
      0x0,       // PA_CL_VPORT_ZSCALE_5
      0x0,       // PA_CL_VPORT_ZOFFSET_5
      0x0,       // PA_CL_VPORT_XSCALE_6
      0x0,       // PA_CL_VPORT_XOFFSET_6
      0x0,       // PA_CL_VPORT_YSCALE_6
      0x0,       // PA_CL_VPORT_YOFFSET_6
      0x0,       // PA_CL_VPORT_ZSCALE_6
      0x0,       // PA_CL_VPORT_ZOFFSET_6
      0x0,       // PA_CL_VPORT_XSCALE_7
      0x0,       // PA_CL_VPORT_XOFFSET_7
      0x0,       // PA_CL_VPORT_YSCALE_7
      0x0,       // PA_CL_VPORT_YOFFSET_7
      0x0,       // PA_CL_VPORT_ZSCALE_7
      0x0,       // PA_CL_VPORT_ZOFFSET_7
      0x0,       // PA_CL_VPORT_XSCALE_8
      0x0,       // PA_CL_VPORT_XOFFSET_8
      0x0,       // PA_CL_VPORT_YSCALE_8
      0x0,       // PA_CL_VPORT_YOFFSET_8
      0x0,       // PA_CL_VPORT_ZSCALE_8
      0x0,       // PA_CL_VPORT_ZOFFSET_8
      0x0,       // PA_CL_VPORT_XSCALE_9
      0x0,       // PA_CL_VPORT_XOFFSET_9
      0x0,       // PA_CL_VPORT_YSCALE_9
      0x0,       // PA_CL_VPORT_YOFFSET_9
      0x0,       // PA_CL_VPORT_ZSCALE_9
      0x0,       // PA_CL_VPORT_ZOFFSET_9
      0x0,       // PA_CL_VPORT_XSCALE_10
      0x0,       // PA_CL_VPORT_XOFFSET_10
      0x0,       // PA_CL_VPORT_YSCALE_10
      0x0,       // PA_CL_VPORT_YOFFSET_10
      0x0,       // PA_CL_VPORT_ZSCALE_10
      0x0,       // PA_CL_VPORT_ZOFFSET_10
      0x0,       // PA_CL_VPORT_XSCALE_11
      0x0,       // PA_CL_VPORT_XOFFSET_11
      0x0,       // PA_CL_VPORT_YSCALE_11
      0x0,       // PA_CL_VPORT_YOFFSET_11
      0x0,       // PA_CL_VPORT_ZSCALE_11
      0x0,       // PA_CL_VPORT_ZOFFSET_11
      0x0,       // PA_CL_VPORT_XSCALE_12
      0x0,       // PA_CL_VPORT_XOFFSET_12
      0x0,       // PA_CL_VPORT_YSCALE_12
      0x0,       // PA_CL_VPORT_YOFFSET_12
      0x0,       // PA_CL_VPORT_ZSCALE_12
      0x0,       // PA_CL_VPORT_ZOFFSET_12
      0x0,       // PA_CL_VPORT_XSCALE_13
      0x0,       // PA_CL_VPORT_XOFFSET_13
      0x0,       // PA_CL_VPORT_YSCALE_13
      0x0,       // PA_CL_VPORT_YOFFSET_13
      0x0,       // PA_CL_VPORT_ZSCALE_13
      0x0,       // PA_CL_VPORT_ZOFFSET_13
      0x0,       // PA_CL_VPORT_XSCALE_14
      0x0,       // PA_CL_VPORT_XOFFSET_14
      0x0,       // PA_CL_VPORT_YSCALE_14
      0x0,       // PA_CL_VPORT_YOFFSET_14
      0x0,       // PA_CL_VPORT_ZSCALE_14
      0x0,       // PA_CL_VPORT_ZOFFSET_14
      0x0,       // PA_CL_VPORT_XSCALE_15
      0x0,       // PA_CL_VPORT_XOFFSET_15
      0x0,       // PA_CL_VPORT_YSCALE_15
      0x0,       // PA_CL_VPORT_YOFFSET_15
      0x0,       // PA_CL_VPORT_ZSCALE_15
      0x0,       // PA_CL_VPORT_ZOFFSET_15
      0x0,       // PA_CL_UCP_0_X
      0x0,       // PA_CL_UCP_0_Y
      0x0,       // PA_CL_UCP_0_Z
      0x0,       // PA_CL_UCP_0_W
      0x0,       // PA_CL_UCP_1_X
      0x0,       // PA_CL_UCP_1_Y
      0x0,       // PA_CL_UCP_1_Z
      0x0,       // PA_CL_UCP_1_W
      0x0,       // PA_CL_UCP_2_X
      0x0,       // PA_CL_UCP_2_Y
      0x0,       // PA_CL_UCP_2_Z
      0x0,       // PA_CL_UCP_2_W
      0x0,       // PA_CL_UCP_3_X
      0x0,       // PA_CL_UCP_3_Y
      0x0,       // PA_CL_UCP_3_Z
      0x0,       // PA_CL_UCP_3_W
      0x0,       // PA_CL_UCP_4_X
      0x0,       // PA_CL_UCP_4_Y
      0x0,       // PA_CL_UCP_4_Z
      0x0,       // PA_CL_UCP_4_W
      0x0,       // PA_CL_UCP_5_X
      0x0,       // PA_CL_UCP_5_Y
      0x0,       // PA_CL_UCP_5_Z
      0x0        // PA_CL_UCP_5_W
   };
   static const uint32_t SpiPsInputCntl0Nv10[] = {
      0x0, // SPI_PS_INPUT_CNTL_0
      0x0, // SPI_PS_INPUT_CNTL_1
      0x0, // SPI_PS_INPUT_CNTL_2
      0x0, // SPI_PS_INPUT_CNTL_3
      0x0, // SPI_PS_INPUT_CNTL_4
      0x0, // SPI_PS_INPUT_CNTL_5
      0x0, // SPI_PS_INPUT_CNTL_6
      0x0, // SPI_PS_INPUT_CNTL_7
      0x0, // SPI_PS_INPUT_CNTL_8
      0x0, // SPI_PS_INPUT_CNTL_9
      0x0, // SPI_PS_INPUT_CNTL_10
      0x0, // SPI_PS_INPUT_CNTL_11
      0x0, // SPI_PS_INPUT_CNTL_12
      0x0, // SPI_PS_INPUT_CNTL_13
      0x0, // SPI_PS_INPUT_CNTL_14
      0x0, // SPI_PS_INPUT_CNTL_15
      0x0, // SPI_PS_INPUT_CNTL_16
      0x0, // SPI_PS_INPUT_CNTL_17
      0x0, // SPI_PS_INPUT_CNTL_18
      0x0, // SPI_PS_INPUT_CNTL_19
      0x0, // SPI_PS_INPUT_CNTL_20
      0x0, // SPI_PS_INPUT_CNTL_21
      0x0, // SPI_PS_INPUT_CNTL_22
      0x0, // SPI_PS_INPUT_CNTL_23
      0x0, // SPI_PS_INPUT_CNTL_24
      0x0, // SPI_PS_INPUT_CNTL_25
      0x0, // SPI_PS_INPUT_CNTL_26
      0x0, // SPI_PS_INPUT_CNTL_27
      0x0, // SPI_PS_INPUT_CNTL_28
      0x0, // SPI_PS_INPUT_CNTL_29
      0x0, // SPI_PS_INPUT_CNTL_30
      0x0, // SPI_PS_INPUT_CNTL_31
      0x0, // SPI_VS_OUT_CONFIG
      0x0, //
      0x0, // SPI_PS_INPUT_ENA
      0x0, // SPI_PS_INPUT_ADDR
      0x0, // SPI_INTERP_CONTROL_0
      0x2, // SPI_PS_IN_CONTROL
      0x0, //
      0x0, // SPI_BARYC_CNTL
      0x0, //
      0x0, // SPI_TMPRING_SIZE
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, // SPI_SHADER_IDX_FORMAT
      0x0, // SPI_SHADER_POS_FORMAT
      0x0, // SPI_SHADER_Z_FORMAT
      0x0  // SPI_SHADER_COL_FORMAT
   };
   static const uint32_t SxPsDownconvertNv10[] = {
      0x0, // SX_PS_DOWNCONVERT
      0x0, // SX_BLEND_OPT_EPSILON
      0x0, // SX_BLEND_OPT_CONTROL
      0x0, // SX_MRT0_BLEND_OPT
      0x0, // SX_MRT1_BLEND_OPT
      0x0, // SX_MRT2_BLEND_OPT
      0x0, // SX_MRT3_BLEND_OPT
      0x0, // SX_MRT4_BLEND_OPT
      0x0, // SX_MRT5_BLEND_OPT
      0x0, // SX_MRT6_BLEND_OPT
      0x0, // SX_MRT7_BLEND_OPT
      0x0, // CB_BLEND0_CONTROL
      0x0, // CB_BLEND1_CONTROL
      0x0, // CB_BLEND2_CONTROL
      0x0, // CB_BLEND3_CONTROL
      0x0, // CB_BLEND4_CONTROL
      0x0, // CB_BLEND5_CONTROL
      0x0, // CB_BLEND6_CONTROL
      0x0  // CB_BLEND7_CONTROL
   };
   static const uint32_t PaClPointXRadNv10[] = {
      0x0, // PA_CL_POINT_X_RAD
      0x0, // PA_CL_POINT_Y_RAD
      0x0, // PA_CL_POINT_SIZE
      0x0  // PA_CL_POINT_CULL_RAD
   };
   static const uint32_t GeMaxOutputPerSubgroupNv10[] = {
      0x0,     // GE_MAX_OUTPUT_PER_SUBGROUP
      0x0,     // DB_DEPTH_CONTROL
      0x0,     // DB_EQAA
      0x0,     // CB_COLOR_CONTROL
      0x0,     // DB_SHADER_CONTROL
      0x90000, // PA_CL_CLIP_CNTL
      0x4,     // PA_SU_SC_MODE_CNTL
      0x0,     // PA_CL_VTE_CNTL
      0x0,     // PA_CL_VS_OUT_CNTL
      0x0,     // PA_CL_NANINF_CNTL
      0x0,     // PA_SU_LINE_STIPPLE_CNTL
      0x0,     // PA_SU_LINE_STIPPLE_SCALE
      0x0,     // PA_SU_PRIM_FILTER_CNTL
      0x0,     // PA_SU_SMALL_PRIM_FILTER_CNTL
      0x0,     // PA_CL_OBJPRIM_ID_CNTL
      0x0,     // PA_CL_NGG_CNTL
      0x0,     // PA_SU_OVER_RASTERIZATION_CNTL
      0x0,     // PA_STEREO_CNTL
      0x0      // PA_STATE_STEREO_X
   };
   static const uint32_t PaSuPointSizeNv10[] = {
      0x0, // PA_SU_POINT_SIZE
      0x0, // PA_SU_POINT_MINMAX
      0x0, // PA_SU_LINE_CNTL
      0x0  // PA_SC_LINE_STIPPLE
   };
   static const uint32_t VgtHosMaxTessLevelNv10[] = {
      0x0, // VGT_HOS_MAX_TESS_LEVEL
      0x0  // VGT_HOS_MIN_TESS_LEVEL
   };
   static const uint32_t VgtGsModeNv10[] = {
      0x0,   // VGT_GS_MODE
      0x0,   // VGT_GS_ONCHIP_CNTL
      0x0,   // PA_SC_MODE_CNTL_0
      0x0,   // PA_SC_MODE_CNTL_1
      0x0,   // VGT_ENHANCE
      0x100, // VGT_GS_PER_ES
      0x80,  // VGT_ES_PER_GS
      0x2,   // VGT_GS_PER_VS
      0x0,   // VGT_GSVS_RING_OFFSET_1
      0x0,   // VGT_GSVS_RING_OFFSET_2
      0x0,   // VGT_GSVS_RING_OFFSET_3
      0x0    // VGT_GS_OUT_PRIM_TYPE
   };
   static const uint32_t VgtPrimitiveidEnNv10[] = {
      0x0 // VGT_PRIMITIVEID_EN
   };
   static const uint32_t VgtPrimitiveidResetNv10[] = {
      0x0 // VGT_PRIMITIVEID_RESET
   };
   static const uint32_t VgtDrawPayloadCntlNv10[] = {
      0x0, // VGT_DRAW_PAYLOAD_CNTL
      0x0, //
      0x0, // VGT_INSTANCE_STEP_RATE_0
      0x0, // VGT_INSTANCE_STEP_RATE_1
      0x0, // IA_MULTI_VGT_PARAM
      0x0, // VGT_ESGS_RING_ITEMSIZE
      0x0, // VGT_GSVS_RING_ITEMSIZE
      0x0, // VGT_REUSE_OFF
      0x0, // VGT_VTX_CNT_EN
      0x0, // DB_HTILE_SURFACE
      0x0, // DB_SRESULTS_COMPARE_STATE0
      0x0, // DB_SRESULTS_COMPARE_STATE1
      0x0, // DB_PRELOAD_CONTROL
      0x0, //
      0x0, // VGT_STRMOUT_BUFFER_SIZE_0
      0x0, // VGT_STRMOUT_VTX_STRIDE_0
      0x0, //
      0x0, // VGT_STRMOUT_BUFFER_OFFSET_0
      0x0, // VGT_STRMOUT_BUFFER_SIZE_1
      0x0, // VGT_STRMOUT_VTX_STRIDE_1
      0x0, //
      0x0, // VGT_STRMOUT_BUFFER_OFFSET_1
      0x0, // VGT_STRMOUT_BUFFER_SIZE_2
      0x0, // VGT_STRMOUT_VTX_STRIDE_2
      0x0, //
      0x0, // VGT_STRMOUT_BUFFER_OFFSET_2
      0x0, // VGT_STRMOUT_BUFFER_SIZE_3
      0x0, // VGT_STRMOUT_VTX_STRIDE_3
      0x0, //
      0x0, // VGT_STRMOUT_BUFFER_OFFSET_3
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, // VGT_STRMOUT_DRAW_OPAQUE_OFFSET
      0x0, // VGT_STRMOUT_DRAW_OPAQUE_BUFFER_FILLED_SIZE
      0x0, // VGT_STRMOUT_DRAW_OPAQUE_VERTEX_STRIDE
      0x0, //
      0x0, // VGT_GS_MAX_VERT_OUT
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, // GE_NGG_SUBGRP_CNTL
      0x0, // VGT_TESS_DISTRIBUTION
      0x0, // VGT_SHADER_STAGES_EN
      0x0, // VGT_LS_HS_CONFIG
      0x0, // VGT_GS_VERT_ITEMSIZE
      0x0, // VGT_GS_VERT_ITEMSIZE_1
      0x0, // VGT_GS_VERT_ITEMSIZE_2
      0x0, // VGT_GS_VERT_ITEMSIZE_3
      0x0, // VGT_TF_PARAM
      0x0, // DB_ALPHA_TO_MASK
      0x0, // VGT_DISPATCH_DRAW_INDEX
      0x0, // PA_SU_POLY_OFFSET_DB_FMT_CNTL
      0x0, // PA_SU_POLY_OFFSET_CLAMP
      0x0, // PA_SU_POLY_OFFSET_FRONT_SCALE
      0x0, // PA_SU_POLY_OFFSET_FRONT_OFFSET
      0x0, // PA_SU_POLY_OFFSET_BACK_SCALE
      0x0, // PA_SU_POLY_OFFSET_BACK_OFFSET
      0x0, // VGT_GS_INSTANCE_CNT
      0x0, // VGT_STRMOUT_CONFIG
      0x0  // VGT_STRMOUT_BUFFER_CONFIG
   };
   static const uint32_t PaScCentroidPriority0Nv10[] = {
      0x0,        // PA_SC_CENTROID_PRIORITY_0
      0x0,        // PA_SC_CENTROID_PRIORITY_1
      0x1000,     // PA_SC_LINE_CNTL
      0x0,        // PA_SC_AA_CONFIG
      0x5,        // PA_SU_VTX_CNTL
      0x3f800000, // PA_CL_GB_VERT_CLIP_ADJ
      0x3f800000, // PA_CL_GB_VERT_DISC_ADJ
      0x3f800000, // PA_CL_GB_HORZ_CLIP_ADJ
      0x3f800000, // PA_CL_GB_HORZ_DISC_ADJ
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_1
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_2
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_3
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_0
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_1
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_2
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_3
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_0
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_1
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_2
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_3
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_0
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_1
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_2
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_3
      0xffffffff, // PA_SC_AA_MASK_X0Y0_X1Y0
      0xffffffff, // PA_SC_AA_MASK_X0Y1_X1Y1
      0x0,        // PA_SC_SHADER_CONTROL
      0x3,        // PA_SC_BINNER_CNTL_0
      0x0,        // PA_SC_BINNER_CNTL_1
      0x100000,   // PA_SC_CONSERVATIVE_RASTERIZATION_CNTL
      0x0,        // PA_SC_NGG_MODE_CNTL
      0x0,        //
      0x1e,       // VGT_VERTEX_REUSE_BLOCK_CNTL
      0x20,       // VGT_OUT_DEALLOC_CNTL
      0x0,        // CB_COLOR0_BASE
      0x0,        //
      0x0,        //
      0x0,        // CB_COLOR0_VIEW
      0x0,        // CB_COLOR0_INFO
      0x0,        // CB_COLOR0_ATTRIB
      0x0,        // CB_COLOR0_DCC_CONTROL
      0x0,        // CB_COLOR0_CMASK
      0x0,        //
      0x0,        // CB_COLOR0_FMASK
      0x0,        //
      0x0,        // CB_COLOR0_CLEAR_WORD0
      0x0,        // CB_COLOR0_CLEAR_WORD1
      0x0,        // CB_COLOR0_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR1_BASE
      0x0,        //
      0x0,        //
      0x0,        // CB_COLOR1_VIEW
      0x0,        // CB_COLOR1_INFO
      0x0,        // CB_COLOR1_ATTRIB
      0x0,        // CB_COLOR1_DCC_CONTROL
      0x0,        // CB_COLOR1_CMASK
      0x0,        //
      0x0,        // CB_COLOR1_FMASK
      0x0,        //
      0x0,        // CB_COLOR1_CLEAR_WORD0
      0x0,        // CB_COLOR1_CLEAR_WORD1
      0x0,        // CB_COLOR1_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR2_BASE
      0x0,        //
      0x0,        //
      0x0,        // CB_COLOR2_VIEW
      0x0,        // CB_COLOR2_INFO
      0x0,        // CB_COLOR2_ATTRIB
      0x0,        // CB_COLOR2_DCC_CONTROL
      0x0,        // CB_COLOR2_CMASK
      0x0,        //
      0x0,        // CB_COLOR2_FMASK
      0x0,        //
      0x0,        // CB_COLOR2_CLEAR_WORD0
      0x0,        // CB_COLOR2_CLEAR_WORD1
      0x0,        // CB_COLOR2_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR3_BASE
      0x0,        //
      0x0,        //
      0x0,        // CB_COLOR3_VIEW
      0x0,        // CB_COLOR3_INFO
      0x0,        // CB_COLOR3_ATTRIB
      0x0,        // CB_COLOR3_DCC_CONTROL
      0x0,        // CB_COLOR3_CMASK
      0x0,        //
      0x0,        // CB_COLOR3_FMASK
      0x0,        //
      0x0,        // CB_COLOR3_CLEAR_WORD0
      0x0,        // CB_COLOR3_CLEAR_WORD1
      0x0,        // CB_COLOR3_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR4_BASE
      0x0,        //
      0x0,        //
      0x0,        // CB_COLOR4_VIEW
      0x0,        // CB_COLOR4_INFO
      0x0,        // CB_COLOR4_ATTRIB
      0x0,        // CB_COLOR4_DCC_CONTROL
      0x0,        // CB_COLOR4_CMASK
      0x0,        //
      0x0,        // CB_COLOR4_FMASK
      0x0,        //
      0x0,        // CB_COLOR4_CLEAR_WORD0
      0x0,        // CB_COLOR4_CLEAR_WORD1
      0x0,        // CB_COLOR4_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR5_BASE
      0x0,        //
      0x0,        //
      0x0,        // CB_COLOR5_VIEW
      0x0,        // CB_COLOR5_INFO
      0x0,        // CB_COLOR5_ATTRIB
      0x0,        // CB_COLOR5_DCC_CONTROL
      0x0,        // CB_COLOR5_CMASK
      0x0,        //
      0x0,        // CB_COLOR5_FMASK
      0x0,        //
      0x0,        // CB_COLOR5_CLEAR_WORD0
      0x0,        // CB_COLOR5_CLEAR_WORD1
      0x0,        // CB_COLOR5_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR6_BASE
      0x0,        //
      0x0,        //
      0x0,        // CB_COLOR6_VIEW
      0x0,        // CB_COLOR6_INFO
      0x0,        // CB_COLOR6_ATTRIB
      0x0,        // CB_COLOR6_DCC_CONTROL
      0x0,        // CB_COLOR6_CMASK
      0x0,        //
      0x0,        // CB_COLOR6_FMASK
      0x0,        //
      0x0,        // CB_COLOR6_CLEAR_WORD0
      0x0,        // CB_COLOR6_CLEAR_WORD1
      0x0,        // CB_COLOR6_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR7_BASE
      0x0,        //
      0x0,        //
      0x0,        // CB_COLOR7_VIEW
      0x0,        // CB_COLOR7_INFO
      0x0,        // CB_COLOR7_ATTRIB
      0x0,        // CB_COLOR7_DCC_CONTROL
      0x0,        // CB_COLOR7_CMASK
      0x0,        //
      0x0,        // CB_COLOR7_FMASK
      0x0,        //
      0x0,        // CB_COLOR7_CLEAR_WORD0
      0x0,        // CB_COLOR7_CLEAR_WORD1
      0x0,        // CB_COLOR7_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR0_BASE_EXT
      0x0,        // CB_COLOR1_BASE_EXT
      0x0,        // CB_COLOR2_BASE_EXT
      0x0,        // CB_COLOR3_BASE_EXT
      0x0,        // CB_COLOR4_BASE_EXT
      0x0,        // CB_COLOR5_BASE_EXT
      0x0,        // CB_COLOR6_BASE_EXT
      0x0,        // CB_COLOR7_BASE_EXT
      0x0,        // CB_COLOR0_CMASK_BASE_EXT
      0x0,        // CB_COLOR1_CMASK_BASE_EXT
      0x0,        // CB_COLOR2_CMASK_BASE_EXT
      0x0,        // CB_COLOR3_CMASK_BASE_EXT
      0x0,        // CB_COLOR4_CMASK_BASE_EXT
      0x0,        // CB_COLOR5_CMASK_BASE_EXT
      0x0,        // CB_COLOR6_CMASK_BASE_EXT
      0x0,        // CB_COLOR7_CMASK_BASE_EXT
      0x0,        // CB_COLOR0_FMASK_BASE_EXT
      0x0,        // CB_COLOR1_FMASK_BASE_EXT
      0x0,        // CB_COLOR2_FMASK_BASE_EXT
      0x0,        // CB_COLOR3_FMASK_BASE_EXT
      0x0,        // CB_COLOR4_FMASK_BASE_EXT
      0x0,        // CB_COLOR5_FMASK_BASE_EXT
      0x0,        // CB_COLOR6_FMASK_BASE_EXT
      0x0,        // CB_COLOR7_FMASK_BASE_EXT
      0x0,        // CB_COLOR0_DCC_BASE_EXT
      0x0,        // CB_COLOR1_DCC_BASE_EXT
      0x0,        // CB_COLOR2_DCC_BASE_EXT
      0x0,        // CB_COLOR3_DCC_BASE_EXT
      0x0,        // CB_COLOR4_DCC_BASE_EXT
      0x0,        // CB_COLOR5_DCC_BASE_EXT
      0x0,        // CB_COLOR6_DCC_BASE_EXT
      0x0,        // CB_COLOR7_DCC_BASE_EXT
      0x0,        // CB_COLOR0_ATTRIB2
      0x0,        // CB_COLOR1_ATTRIB2
      0x0,        // CB_COLOR2_ATTRIB2
      0x0,        // CB_COLOR3_ATTRIB2
      0x0,        // CB_COLOR4_ATTRIB2
      0x0,        // CB_COLOR5_ATTRIB2
      0x0,        // CB_COLOR6_ATTRIB2
      0x0,        // CB_COLOR7_ATTRIB2
      0x0,        // CB_COLOR0_ATTRIB3
      0x0,        // CB_COLOR1_ATTRIB3
      0x0,        // CB_COLOR2_ATTRIB3
      0x0,        // CB_COLOR3_ATTRIB3
      0x0,        // CB_COLOR4_ATTRIB3
      0x0,        // CB_COLOR5_ATTRIB3
      0x0,        // CB_COLOR6_ATTRIB3
      0x0         // CB_COLOR7_ATTRIB3
   };

   set_context_reg_seq_array(cs, R_028000_DB_RENDER_CONTROL, SET(DbRenderControlNv10));
   set_context_reg_seq_array(cs, R_0281E8_COHER_DEST_BASE_HI_0, SET(CoherDestBaseHi0Nv10));
   set_context_reg_seq_array(cs, R_02840C_VGT_MULTI_PRIM_IB_RESET_INDX,
                             SET(VgtMultiPrimIbResetIndxNv10));
   set_context_reg_seq_array(cs, R_028644_SPI_PS_INPUT_CNTL_0, SET(SpiPsInputCntl0Nv10));
   set_context_reg_seq_array(cs, R_028754_SX_PS_DOWNCONVERT, SET(SxPsDownconvertNv10));
   set_context_reg_seq_array(cs, R_0287D4_PA_CL_POINT_X_RAD, SET(PaClPointXRadNv10));
   set_context_reg_seq_array(cs, R_0287FC_GE_MAX_OUTPUT_PER_SUBGROUP,
                             SET(GeMaxOutputPerSubgroupNv10));
   set_context_reg_seq_array(cs, R_028A00_PA_SU_POINT_SIZE, SET(PaSuPointSizeNv10));
   set_context_reg_seq_array(cs, R_028A18_VGT_HOS_MAX_TESS_LEVEL, SET(VgtHosMaxTessLevelNv10));
   set_context_reg_seq_array(cs, R_028A40_VGT_GS_MODE, SET(VgtGsModeNv10));
   set_context_reg_seq_array(cs, R_028A84_VGT_PRIMITIVEID_EN, SET(VgtPrimitiveidEnNv10));
   set_context_reg_seq_array(cs, R_028A8C_VGT_PRIMITIVEID_RESET, SET(VgtPrimitiveidResetNv10));
   set_context_reg_seq_array(cs, R_028A98_VGT_DRAW_PAYLOAD_CNTL, SET(VgtDrawPayloadCntlNv10));
   set_context_reg_seq_array(cs, R_028BD4_PA_SC_CENTROID_PRIORITY_0,
                             SET(PaScCentroidPriority0Nv10));

   for (unsigned i = 0; i < num_reg_pairs; i++)
      set_context_reg_seq_array(cs, reg_offsets[i], 1, &reg_values[i]);
}

/**
 * Emulate CLEAR_STATE. Additionally, initialize num_reg_pairs registers specified
 * via reg_offsets and reg_values.
 */
static void gfx103_emulate_clear_state(struct radeon_cmdbuf *cs, unsigned num_reg_pairs,
                                       unsigned *reg_offsets, uint32_t *reg_values,
                                       set_context_reg_seq_array_fn set_context_reg_seq_array)
{
   static const uint32_t DbRenderControlGfx103[] = {
      0x0,        // DB_RENDER_CONTROL
      0x0,        // DB_COUNT_CONTROL
      0x0,        // DB_DEPTH_VIEW
      0x0,        // DB_RENDER_OVERRIDE
      0x0,        // DB_RENDER_OVERRIDE2
      0x0,        // DB_HTILE_DATA_BASE
      0x0,        //
      0x0,        // DB_DEPTH_SIZE_XY
      0x0,        // DB_DEPTH_BOUNDS_MIN
      0x0,        // DB_DEPTH_BOUNDS_MAX
      0x0,        // DB_STENCIL_CLEAR
      0x0,        // DB_DEPTH_CLEAR
      0x0,        // PA_SC_SCREEN_SCISSOR_TL
      0x40004000, // PA_SC_SCREEN_SCISSOR_BR
      0x0,        // DB_DFSM_CONTROL
      0x0,        // DB_RESERVED_REG_2
      0x0,        // DB_Z_INFO
      0x0,        // DB_STENCIL_INFO
      0x0,        // DB_Z_READ_BASE
      0x0,        // DB_STENCIL_READ_BASE
      0x0,        // DB_Z_WRITE_BASE
      0x0,        // DB_STENCIL_WRITE_BASE
      0x0,        //
      0x0,        //
      0x0,        //
      0x0,        //
      0x0,        // DB_Z_READ_BASE_HI
      0x0,        // DB_STENCIL_READ_BASE_HI
      0x0,        // DB_Z_WRITE_BASE_HI
      0x0,        // DB_STENCIL_WRITE_BASE_HI
      0x0,        // DB_HTILE_DATA_BASE_HI
      0x0,        // DB_RMI_L2_CACHE_CONTROL
      0x0,        // TA_BC_BASE_ADDR
      0x0         // TA_BC_BASE_ADDR_HI
   };
   static const uint32_t CoherDestBaseHi0Gfx103[] = {
      0x0,        // COHER_DEST_BASE_HI_0
      0x0,        // COHER_DEST_BASE_HI_1
      0x0,        // COHER_DEST_BASE_HI_2
      0x0,        // COHER_DEST_BASE_HI_3
      0x0,        // COHER_DEST_BASE_2
      0x0,        // COHER_DEST_BASE_3
      0x0,        // PA_SC_WINDOW_OFFSET
      0x80000000, // PA_SC_WINDOW_SCISSOR_TL
      0x40004000, // PA_SC_WINDOW_SCISSOR_BR
      0xffff,     // PA_SC_CLIPRECT_RULE
      0x0,        // PA_SC_CLIPRECT_0_TL
      0x40004000, // PA_SC_CLIPRECT_0_BR
      0x0,        // PA_SC_CLIPRECT_1_TL
      0x40004000, // PA_SC_CLIPRECT_1_BR
      0x0,        // PA_SC_CLIPRECT_2_TL
      0x40004000, // PA_SC_CLIPRECT_2_BR
      0x0,        // PA_SC_CLIPRECT_3_TL
      0x40004000, // PA_SC_CLIPRECT_3_BR
      0xaa99aaaa, // PA_SC_EDGERULE
      0x0,        // PA_SU_HARDWARE_SCREEN_OFFSET
      0xffffffff, // CB_TARGET_MASK
      0xffffffff, // CB_SHADER_MASK
      0x80000000, // PA_SC_GENERIC_SCISSOR_TL
      0x40004000, // PA_SC_GENERIC_SCISSOR_BR
      0x0,        // COHER_DEST_BASE_0
      0x0,        // COHER_DEST_BASE_1
      0x80000000, // PA_SC_VPORT_SCISSOR_0_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_0_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_1_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_1_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_2_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_2_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_3_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_3_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_4_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_4_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_5_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_5_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_6_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_6_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_7_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_7_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_8_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_8_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_9_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_9_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_10_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_10_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_11_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_11_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_12_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_12_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_13_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_13_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_14_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_14_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_15_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_15_BR
      0x0,        // PA_SC_VPORT_ZMIN_0
      0x3f800000, // PA_SC_VPORT_ZMAX_0
      0x0,        // PA_SC_VPORT_ZMIN_1
      0x3f800000, // PA_SC_VPORT_ZMAX_1
      0x0,        // PA_SC_VPORT_ZMIN_2
      0x3f800000, // PA_SC_VPORT_ZMAX_2
      0x0,        // PA_SC_VPORT_ZMIN_3
      0x3f800000, // PA_SC_VPORT_ZMAX_3
      0x0,        // PA_SC_VPORT_ZMIN_4
      0x3f800000, // PA_SC_VPORT_ZMAX_4
      0x0,        // PA_SC_VPORT_ZMIN_5
      0x3f800000, // PA_SC_VPORT_ZMAX_5
      0x0,        // PA_SC_VPORT_ZMIN_6
      0x3f800000, // PA_SC_VPORT_ZMAX_6
      0x0,        // PA_SC_VPORT_ZMIN_7
      0x3f800000, // PA_SC_VPORT_ZMAX_7
      0x0,        // PA_SC_VPORT_ZMIN_8
      0x3f800000, // PA_SC_VPORT_ZMAX_8
      0x0,        // PA_SC_VPORT_ZMIN_9
      0x3f800000, // PA_SC_VPORT_ZMAX_9
      0x0,        // PA_SC_VPORT_ZMIN_10
      0x3f800000, // PA_SC_VPORT_ZMAX_10
      0x0,        // PA_SC_VPORT_ZMIN_11
      0x3f800000, // PA_SC_VPORT_ZMAX_11
      0x0,        // PA_SC_VPORT_ZMIN_12
      0x3f800000, // PA_SC_VPORT_ZMAX_12
      0x0,        // PA_SC_VPORT_ZMIN_13
      0x3f800000, // PA_SC_VPORT_ZMAX_13
      0x0,        // PA_SC_VPORT_ZMIN_14
      0x3f800000, // PA_SC_VPORT_ZMAX_14
      0x0,        // PA_SC_VPORT_ZMIN_15
      0x3f800000, // PA_SC_VPORT_ZMAX_15
      0x0,        // PA_SC_RASTER_CONFIG
      0x0,        // PA_SC_RASTER_CONFIG_1
      0x0,        //
      0x0         // PA_SC_TILE_STEERING_OVERRIDE
   };
   static const uint32_t VgtMultiPrimIbResetIndxGfx103[] = {
      0x0,       // VGT_MULTI_PRIM_IB_RESET_INDX
      0x0,       // CB_RMI_GL2_CACHE_CONTROL
      0x0,       // CB_BLEND_RED
      0x0,       // CB_BLEND_GREEN
      0x0,       // CB_BLEND_BLUE
      0x0,       // CB_BLEND_ALPHA
      0x0,       // CB_DCC_CONTROL
      0x0,       // CB_COVERAGE_OUT_CONTROL
      0x0,       // DB_STENCIL_CONTROL
      0x1000000, // DB_STENCILREFMASK
      0x1000000, // DB_STENCILREFMASK_BF
      0x0,       //
      0x0,       // PA_CL_VPORT_XSCALE
      0x0,       // PA_CL_VPORT_XOFFSET
      0x0,       // PA_CL_VPORT_YSCALE
      0x0,       // PA_CL_VPORT_YOFFSET
      0x0,       // PA_CL_VPORT_ZSCALE
      0x0,       // PA_CL_VPORT_ZOFFSET
      0x0,       // PA_CL_VPORT_XSCALE_1
      0x0,       // PA_CL_VPORT_XOFFSET_1
      0x0,       // PA_CL_VPORT_YSCALE_1
      0x0,       // PA_CL_VPORT_YOFFSET_1
      0x0,       // PA_CL_VPORT_ZSCALE_1
      0x0,       // PA_CL_VPORT_ZOFFSET_1
      0x0,       // PA_CL_VPORT_XSCALE_2
      0x0,       // PA_CL_VPORT_XOFFSET_2
      0x0,       // PA_CL_VPORT_YSCALE_2
      0x0,       // PA_CL_VPORT_YOFFSET_2
      0x0,       // PA_CL_VPORT_ZSCALE_2
      0x0,       // PA_CL_VPORT_ZOFFSET_2
      0x0,       // PA_CL_VPORT_XSCALE_3
      0x0,       // PA_CL_VPORT_XOFFSET_3
      0x0,       // PA_CL_VPORT_YSCALE_3
      0x0,       // PA_CL_VPORT_YOFFSET_3
      0x0,       // PA_CL_VPORT_ZSCALE_3
      0x0,       // PA_CL_VPORT_ZOFFSET_3
      0x0,       // PA_CL_VPORT_XSCALE_4
      0x0,       // PA_CL_VPORT_XOFFSET_4
      0x0,       // PA_CL_VPORT_YSCALE_4
      0x0,       // PA_CL_VPORT_YOFFSET_4
      0x0,       // PA_CL_VPORT_ZSCALE_4
      0x0,       // PA_CL_VPORT_ZOFFSET_4
      0x0,       // PA_CL_VPORT_XSCALE_5
      0x0,       // PA_CL_VPORT_XOFFSET_5
      0x0,       // PA_CL_VPORT_YSCALE_5
      0x0,       // PA_CL_VPORT_YOFFSET_5
      0x0,       // PA_CL_VPORT_ZSCALE_5
      0x0,       // PA_CL_VPORT_ZOFFSET_5
      0x0,       // PA_CL_VPORT_XSCALE_6
      0x0,       // PA_CL_VPORT_XOFFSET_6
      0x0,       // PA_CL_VPORT_YSCALE_6
      0x0,       // PA_CL_VPORT_YOFFSET_6
      0x0,       // PA_CL_VPORT_ZSCALE_6
      0x0,       // PA_CL_VPORT_ZOFFSET_6
      0x0,       // PA_CL_VPORT_XSCALE_7
      0x0,       // PA_CL_VPORT_XOFFSET_7
      0x0,       // PA_CL_VPORT_YSCALE_7
      0x0,       // PA_CL_VPORT_YOFFSET_7
      0x0,       // PA_CL_VPORT_ZSCALE_7
      0x0,       // PA_CL_VPORT_ZOFFSET_7
      0x0,       // PA_CL_VPORT_XSCALE_8
      0x0,       // PA_CL_VPORT_XOFFSET_8
      0x0,       // PA_CL_VPORT_YSCALE_8
      0x0,       // PA_CL_VPORT_YOFFSET_8
      0x0,       // PA_CL_VPORT_ZSCALE_8
      0x0,       // PA_CL_VPORT_ZOFFSET_8
      0x0,       // PA_CL_VPORT_XSCALE_9
      0x0,       // PA_CL_VPORT_XOFFSET_9
      0x0,       // PA_CL_VPORT_YSCALE_9
      0x0,       // PA_CL_VPORT_YOFFSET_9
      0x0,       // PA_CL_VPORT_ZSCALE_9
      0x0,       // PA_CL_VPORT_ZOFFSET_9
      0x0,       // PA_CL_VPORT_XSCALE_10
      0x0,       // PA_CL_VPORT_XOFFSET_10
      0x0,       // PA_CL_VPORT_YSCALE_10
      0x0,       // PA_CL_VPORT_YOFFSET_10
      0x0,       // PA_CL_VPORT_ZSCALE_10
      0x0,       // PA_CL_VPORT_ZOFFSET_10
      0x0,       // PA_CL_VPORT_XSCALE_11
      0x0,       // PA_CL_VPORT_XOFFSET_11
      0x0,       // PA_CL_VPORT_YSCALE_11
      0x0,       // PA_CL_VPORT_YOFFSET_11
      0x0,       // PA_CL_VPORT_ZSCALE_11
      0x0,       // PA_CL_VPORT_ZOFFSET_11
      0x0,       // PA_CL_VPORT_XSCALE_12
      0x0,       // PA_CL_VPORT_XOFFSET_12
      0x0,       // PA_CL_VPORT_YSCALE_12
      0x0,       // PA_CL_VPORT_YOFFSET_12
      0x0,       // PA_CL_VPORT_ZSCALE_12
      0x0,       // PA_CL_VPORT_ZOFFSET_12
      0x0,       // PA_CL_VPORT_XSCALE_13
      0x0,       // PA_CL_VPORT_XOFFSET_13
      0x0,       // PA_CL_VPORT_YSCALE_13
      0x0,       // PA_CL_VPORT_YOFFSET_13
      0x0,       // PA_CL_VPORT_ZSCALE_13
      0x0,       // PA_CL_VPORT_ZOFFSET_13
      0x0,       // PA_CL_VPORT_XSCALE_14
      0x0,       // PA_CL_VPORT_XOFFSET_14
      0x0,       // PA_CL_VPORT_YSCALE_14
      0x0,       // PA_CL_VPORT_YOFFSET_14
      0x0,       // PA_CL_VPORT_ZSCALE_14
      0x0,       // PA_CL_VPORT_ZOFFSET_14
      0x0,       // PA_CL_VPORT_XSCALE_15
      0x0,       // PA_CL_VPORT_XOFFSET_15
      0x0,       // PA_CL_VPORT_YSCALE_15
      0x0,       // PA_CL_VPORT_YOFFSET_15
      0x0,       // PA_CL_VPORT_ZSCALE_15
      0x0,       // PA_CL_VPORT_ZOFFSET_15
      0x0,       // PA_CL_UCP_0_X
      0x0,       // PA_CL_UCP_0_Y
      0x0,       // PA_CL_UCP_0_Z
      0x0,       // PA_CL_UCP_0_W
      0x0,       // PA_CL_UCP_1_X
      0x0,       // PA_CL_UCP_1_Y
      0x0,       // PA_CL_UCP_1_Z
      0x0,       // PA_CL_UCP_1_W
      0x0,       // PA_CL_UCP_2_X
      0x0,       // PA_CL_UCP_2_Y
      0x0,       // PA_CL_UCP_2_Z
      0x0,       // PA_CL_UCP_2_W
      0x0,       // PA_CL_UCP_3_X
      0x0,       // PA_CL_UCP_3_Y
      0x0,       // PA_CL_UCP_3_Z
      0x0,       // PA_CL_UCP_3_W
      0x0,       // PA_CL_UCP_4_X
      0x0,       // PA_CL_UCP_4_Y
      0x0,       // PA_CL_UCP_4_Z
      0x0,       // PA_CL_UCP_4_W
      0x0,       // PA_CL_UCP_5_X
      0x0,       // PA_CL_UCP_5_Y
      0x0,       // PA_CL_UCP_5_Z
      0x0        // PA_CL_UCP_5_W
   };
   static const uint32_t SpiPsInputCntl0Gfx103[] = {
      0x0, // SPI_PS_INPUT_CNTL_0
      0x0, // SPI_PS_INPUT_CNTL_1
      0x0, // SPI_PS_INPUT_CNTL_2
      0x0, // SPI_PS_INPUT_CNTL_3
      0x0, // SPI_PS_INPUT_CNTL_4
      0x0, // SPI_PS_INPUT_CNTL_5
      0x0, // SPI_PS_INPUT_CNTL_6
      0x0, // SPI_PS_INPUT_CNTL_7
      0x0, // SPI_PS_INPUT_CNTL_8
      0x0, // SPI_PS_INPUT_CNTL_9
      0x0, // SPI_PS_INPUT_CNTL_10
      0x0, // SPI_PS_INPUT_CNTL_11
      0x0, // SPI_PS_INPUT_CNTL_12
      0x0, // SPI_PS_INPUT_CNTL_13
      0x0, // SPI_PS_INPUT_CNTL_14
      0x0, // SPI_PS_INPUT_CNTL_15
      0x0, // SPI_PS_INPUT_CNTL_16
      0x0, // SPI_PS_INPUT_CNTL_17
      0x0, // SPI_PS_INPUT_CNTL_18
      0x0, // SPI_PS_INPUT_CNTL_19
      0x0, // SPI_PS_INPUT_CNTL_20
      0x0, // SPI_PS_INPUT_CNTL_21
      0x0, // SPI_PS_INPUT_CNTL_22
      0x0, // SPI_PS_INPUT_CNTL_23
      0x0, // SPI_PS_INPUT_CNTL_24
      0x0, // SPI_PS_INPUT_CNTL_25
      0x0, // SPI_PS_INPUT_CNTL_26
      0x0, // SPI_PS_INPUT_CNTL_27
      0x0, // SPI_PS_INPUT_CNTL_28
      0x0, // SPI_PS_INPUT_CNTL_29
      0x0, // SPI_PS_INPUT_CNTL_30
      0x0, // SPI_PS_INPUT_CNTL_31
      0x0, // SPI_VS_OUT_CONFIG
      0x0, //
      0x0, // SPI_PS_INPUT_ENA
      0x0, // SPI_PS_INPUT_ADDR
      0x0, // SPI_INTERP_CONTROL_0
      0x2, // SPI_PS_IN_CONTROL
      0x0, //
      0x0, // SPI_BARYC_CNTL
      0x0, //
      0x0, // SPI_TMPRING_SIZE
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, // SPI_SHADER_IDX_FORMAT
      0x0, // SPI_SHADER_POS_FORMAT
      0x0, // SPI_SHADER_Z_FORMAT
      0x0  // SPI_SHADER_COL_FORMAT
   };
   static const uint32_t SxPsDownconvertControlGfx103[] = {
      0x0, // SX_PS_DOWNCONVERT_CONTROL
      0x0, // SX_PS_DOWNCONVERT
      0x0, // SX_BLEND_OPT_EPSILON
      0x0, // SX_BLEND_OPT_CONTROL
      0x0, // SX_MRT0_BLEND_OPT
      0x0, // SX_MRT1_BLEND_OPT
      0x0, // SX_MRT2_BLEND_OPT
      0x0, // SX_MRT3_BLEND_OPT
      0x0, // SX_MRT4_BLEND_OPT
      0x0, // SX_MRT5_BLEND_OPT
      0x0, // SX_MRT6_BLEND_OPT
      0x0, // SX_MRT7_BLEND_OPT
      0x0, // CB_BLEND0_CONTROL
      0x0, // CB_BLEND1_CONTROL
      0x0, // CB_BLEND2_CONTROL
      0x0, // CB_BLEND3_CONTROL
      0x0, // CB_BLEND4_CONTROL
      0x0, // CB_BLEND5_CONTROL
      0x0, // CB_BLEND6_CONTROL
      0x0  // CB_BLEND7_CONTROL
   };
   static const uint32_t PaClPointXRadGfx103[] = {
      0x0, // PA_CL_POINT_X_RAD
      0x0, // PA_CL_POINT_Y_RAD
      0x0, // PA_CL_POINT_SIZE
      0x0  // PA_CL_POINT_CULL_RAD
   };
   static const uint32_t GeMaxOutputPerSubgroupGfx103[] = {
      0x0,     // GE_MAX_OUTPUT_PER_SUBGROUP
      0x0,     // DB_DEPTH_CONTROL
      0x0,     // DB_EQAA
      0x0,     // CB_COLOR_CONTROL
      0x0,     // DB_SHADER_CONTROL
      0x90000, // PA_CL_CLIP_CNTL
      0x4,     // PA_SU_SC_MODE_CNTL
      0x0,     // PA_CL_VTE_CNTL
      0x0,     // PA_CL_VS_OUT_CNTL
      0x0,     // PA_CL_NANINF_CNTL
      0x0,     // PA_SU_LINE_STIPPLE_CNTL
      0x0,     // PA_SU_LINE_STIPPLE_SCALE
      0x0,     // PA_SU_PRIM_FILTER_CNTL
      0x0,     // PA_SU_SMALL_PRIM_FILTER_CNTL
      0x0,     // PA_CL_OBJPRIM_ID_CNTL
      0x0,     // PA_CL_NGG_CNTL
      0x0,     // PA_SU_OVER_RASTERIZATION_CNTL
      0x0,     // PA_STEREO_CNTL
      0x0,     // PA_STATE_STEREO_X
      0x0      // PA_CL_VRS_CNTL
   };
   static const uint32_t PaSuPointSizeGfx103[] = {
      0x0, // PA_SU_POINT_SIZE
      0x0, // PA_SU_POINT_MINMAX
      0x0, // PA_SU_LINE_CNTL
      0x0  // PA_SC_LINE_STIPPLE
   };
   static const uint32_t VgtHosMaxTessLevelGfx103[] = {
      0x0, // VGT_HOS_MAX_TESS_LEVEL
      0x0  // VGT_HOS_MIN_TESS_LEVEL
   };
   static const uint32_t VgtGsModeGfx103[] = {
      0x0,   // VGT_GS_MODE
      0x0,   // VGT_GS_ONCHIP_CNTL
      0x0,   // PA_SC_MODE_CNTL_0
      0x0,   // PA_SC_MODE_CNTL_1
      0x0,   // VGT_ENHANCE
      0x100, // VGT_GS_PER_ES
      0x80,  // VGT_ES_PER_GS
      0x2,   // VGT_GS_PER_VS
      0x0,   // VGT_GSVS_RING_OFFSET_1
      0x0,   // VGT_GSVS_RING_OFFSET_2
      0x0,   // VGT_GSVS_RING_OFFSET_3
      0x0    // VGT_GS_OUT_PRIM_TYPE
   };
   static const uint32_t VgtPrimitiveidEnGfx103[] = {
      0x0 // VGT_PRIMITIVEID_EN
   };
   static const uint32_t VgtPrimitiveidResetGfx103[] = {
      0x0 // VGT_PRIMITIVEID_RESET
   };
   static const uint32_t VgtDrawPayloadCntlGfx103[] = {
      0x0, // VGT_DRAW_PAYLOAD_CNTL
      0x0, //
      0x0, // VGT_INSTANCE_STEP_RATE_0
      0x0, // VGT_INSTANCE_STEP_RATE_1
      0x0, // IA_MULTI_VGT_PARAM
      0x0, // VGT_ESGS_RING_ITEMSIZE
      0x0, // VGT_GSVS_RING_ITEMSIZE
      0x0, // VGT_REUSE_OFF
      0x0, // VGT_VTX_CNT_EN
      0x0, // DB_HTILE_SURFACE
      0x0, // DB_SRESULTS_COMPARE_STATE0
      0x0, // DB_SRESULTS_COMPARE_STATE1
      0x0, // DB_PRELOAD_CONTROL
      0x0, //
      0x0, // VGT_STRMOUT_BUFFER_SIZE_0
      0x0, // VGT_STRMOUT_VTX_STRIDE_0
      0x0, //
      0x0, // VGT_STRMOUT_BUFFER_OFFSET_0
      0x0, // VGT_STRMOUT_BUFFER_SIZE_1
      0x0, // VGT_STRMOUT_VTX_STRIDE_1
      0x0, //
      0x0, // VGT_STRMOUT_BUFFER_OFFSET_1
      0x0, // VGT_STRMOUT_BUFFER_SIZE_2
      0x0, // VGT_STRMOUT_VTX_STRIDE_2
      0x0, //
      0x0, // VGT_STRMOUT_BUFFER_OFFSET_2
      0x0, // VGT_STRMOUT_BUFFER_SIZE_3
      0x0, // VGT_STRMOUT_VTX_STRIDE_3
      0x0, //
      0x0, // VGT_STRMOUT_BUFFER_OFFSET_3
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, // VGT_STRMOUT_DRAW_OPAQUE_OFFSET
      0x0, // VGT_STRMOUT_DRAW_OPAQUE_BUFFER_FILLED_SIZE
      0x0, // VGT_STRMOUT_DRAW_OPAQUE_VERTEX_STRIDE
      0x0, //
      0x0, // VGT_GS_MAX_VERT_OUT
      0x0, //
      0x0, //
      0x0, //
      0x0, //
      0x0, // GE_NGG_SUBGRP_CNTL
      0x0, // VGT_TESS_DISTRIBUTION
      0x0, // VGT_SHADER_STAGES_EN
      0x0, // VGT_LS_HS_CONFIG
      0x0, // VGT_GS_VERT_ITEMSIZE
      0x0, // VGT_GS_VERT_ITEMSIZE_1
      0x0, // VGT_GS_VERT_ITEMSIZE_2
      0x0, // VGT_GS_VERT_ITEMSIZE_3
      0x0, // VGT_TF_PARAM
      0x0, // DB_ALPHA_TO_MASK
      0x0, //
      0x0, // PA_SU_POLY_OFFSET_DB_FMT_CNTL
      0x0, // PA_SU_POLY_OFFSET_CLAMP
      0x0, // PA_SU_POLY_OFFSET_FRONT_SCALE
      0x0, // PA_SU_POLY_OFFSET_FRONT_OFFSET
      0x0, // PA_SU_POLY_OFFSET_BACK_SCALE
      0x0, // PA_SU_POLY_OFFSET_BACK_OFFSET
      0x0, // VGT_GS_INSTANCE_CNT
      0x0, // VGT_STRMOUT_CONFIG
      0x0  // VGT_STRMOUT_BUFFER_CONFIG
   };
   static const uint32_t PaScCentroidPriority0Gfx103[] = {
      0x0,        // PA_SC_CENTROID_PRIORITY_0
      0x0,        // PA_SC_CENTROID_PRIORITY_1
      0x1000,     // PA_SC_LINE_CNTL
      0x0,        // PA_SC_AA_CONFIG
      0x5,        // PA_SU_VTX_CNTL
      0x3f800000, // PA_CL_GB_VERT_CLIP_ADJ
      0x3f800000, // PA_CL_GB_VERT_DISC_ADJ
      0x3f800000, // PA_CL_GB_HORZ_CLIP_ADJ
      0x3f800000, // PA_CL_GB_HORZ_DISC_ADJ
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_1
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_2
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_3
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_0
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_1
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_2
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_3
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_0
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_1
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_2
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_3
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_0
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_1
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_2
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_3
      0xffffffff, // PA_SC_AA_MASK_X0Y0_X1Y0
      0xffffffff, // PA_SC_AA_MASK_X0Y1_X1Y1
      0x0,        // PA_SC_SHADER_CONTROL
      0x3,        // PA_SC_BINNER_CNTL_0
      0x0,        // PA_SC_BINNER_CNTL_1
      0x100000,   // PA_SC_CONSERVATIVE_RASTERIZATION_CNTL
      0x0,        // PA_SC_NGG_MODE_CNTL
      0x0,        //
      0x1e,       // VGT_VERTEX_REUSE_BLOCK_CNTL
      0x20,       // VGT_OUT_DEALLOC_CNTL
      0x0,        // CB_COLOR0_BASE
      0x0,        //
      0x0,        //
      0x0,        // CB_COLOR0_VIEW
      0x0,        // CB_COLOR0_INFO
      0x0,        // CB_COLOR0_ATTRIB
      0x0,        // CB_COLOR0_DCC_CONTROL
      0x0,        // CB_COLOR0_CMASK
      0x0,        //
      0x0,        // CB_COLOR0_FMASK
      0x0,        //
      0x0,        // CB_COLOR0_CLEAR_WORD0
      0x0,        // CB_COLOR0_CLEAR_WORD1
      0x0,        // CB_COLOR0_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR1_BASE
      0x0,        //
      0x0,        //
      0x0,        // CB_COLOR1_VIEW
      0x0,        // CB_COLOR1_INFO
      0x0,        // CB_COLOR1_ATTRIB
      0x0,        // CB_COLOR1_DCC_CONTROL
      0x0,        // CB_COLOR1_CMASK
      0x0,        //
      0x0,        // CB_COLOR1_FMASK
      0x0,        //
      0x0,        // CB_COLOR1_CLEAR_WORD0
      0x0,        // CB_COLOR1_CLEAR_WORD1
      0x0,        // CB_COLOR1_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR2_BASE
      0x0,        //
      0x0,        //
      0x0,        // CB_COLOR2_VIEW
      0x0,        // CB_COLOR2_INFO
      0x0,        // CB_COLOR2_ATTRIB
      0x0,        // CB_COLOR2_DCC_CONTROL
      0x0,        // CB_COLOR2_CMASK
      0x0,        //
      0x0,        // CB_COLOR2_FMASK
      0x0,        //
      0x0,        // CB_COLOR2_CLEAR_WORD0
      0x0,        // CB_COLOR2_CLEAR_WORD1
      0x0,        // CB_COLOR2_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR3_BASE
      0x0,        //
      0x0,        //
      0x0,        // CB_COLOR3_VIEW
      0x0,        // CB_COLOR3_INFO
      0x0,        // CB_COLOR3_ATTRIB
      0x0,        // CB_COLOR3_DCC_CONTROL
      0x0,        // CB_COLOR3_CMASK
      0x0,        //
      0x0,        // CB_COLOR3_FMASK
      0x0,        //
      0x0,        // CB_COLOR3_CLEAR_WORD0
      0x0,        // CB_COLOR3_CLEAR_WORD1
      0x0,        // CB_COLOR3_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR4_BASE
      0x0,        //
      0x0,        //
      0x0,        // CB_COLOR4_VIEW
      0x0,        // CB_COLOR4_INFO
      0x0,        // CB_COLOR4_ATTRIB
      0x0,        // CB_COLOR4_DCC_CONTROL
      0x0,        // CB_COLOR4_CMASK
      0x0,        //
      0x0,        // CB_COLOR4_FMASK
      0x0,        //
      0x0,        // CB_COLOR4_CLEAR_WORD0
      0x0,        // CB_COLOR4_CLEAR_WORD1
      0x0,        // CB_COLOR4_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR5_BASE
      0x0,        //
      0x0,        //
      0x0,        // CB_COLOR5_VIEW
      0x0,        // CB_COLOR5_INFO
      0x0,        // CB_COLOR5_ATTRIB
      0x0,        // CB_COLOR5_DCC_CONTROL
      0x0,        // CB_COLOR5_CMASK
      0x0,        //
      0x0,        // CB_COLOR5_FMASK
      0x0,        //
      0x0,        // CB_COLOR5_CLEAR_WORD0
      0x0,        // CB_COLOR5_CLEAR_WORD1
      0x0,        // CB_COLOR5_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR6_BASE
      0x0,        //
      0x0,        //
      0x0,        // CB_COLOR6_VIEW
      0x0,        // CB_COLOR6_INFO
      0x0,        // CB_COLOR6_ATTRIB
      0x0,        // CB_COLOR6_DCC_CONTROL
      0x0,        // CB_COLOR6_CMASK
      0x0,        //
      0x0,        // CB_COLOR6_FMASK
      0x0,        //
      0x0,        // CB_COLOR6_CLEAR_WORD0
      0x0,        // CB_COLOR6_CLEAR_WORD1
      0x0,        // CB_COLOR6_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR7_BASE
      0x0,        //
      0x0,        //
      0x0,        // CB_COLOR7_VIEW
      0x0,        // CB_COLOR7_INFO
      0x0,        // CB_COLOR7_ATTRIB
      0x0,        // CB_COLOR7_DCC_CONTROL
      0x0,        // CB_COLOR7_CMASK
      0x0,        //
      0x0,        // CB_COLOR7_FMASK
      0x0,        //
      0x0,        // CB_COLOR7_CLEAR_WORD0
      0x0,        // CB_COLOR7_CLEAR_WORD1
      0x0,        // CB_COLOR7_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR0_BASE_EXT
      0x0,        // CB_COLOR1_BASE_EXT
      0x0,        // CB_COLOR2_BASE_EXT
      0x0,        // CB_COLOR3_BASE_EXT
      0x0,        // CB_COLOR4_BASE_EXT
      0x0,        // CB_COLOR5_BASE_EXT
      0x0,        // CB_COLOR6_BASE_EXT
      0x0,        // CB_COLOR7_BASE_EXT
      0x0,        // CB_COLOR0_CMASK_BASE_EXT
      0x0,        // CB_COLOR1_CMASK_BASE_EXT
      0x0,        // CB_COLOR2_CMASK_BASE_EXT
      0x0,        // CB_COLOR3_CMASK_BASE_EXT
      0x0,        // CB_COLOR4_CMASK_BASE_EXT
      0x0,        // CB_COLOR5_CMASK_BASE_EXT
      0x0,        // CB_COLOR6_CMASK_BASE_EXT
      0x0,        // CB_COLOR7_CMASK_BASE_EXT
      0x0,        // CB_COLOR0_FMASK_BASE_EXT
      0x0,        // CB_COLOR1_FMASK_BASE_EXT
      0x0,        // CB_COLOR2_FMASK_BASE_EXT
      0x0,        // CB_COLOR3_FMASK_BASE_EXT
      0x0,        // CB_COLOR4_FMASK_BASE_EXT
      0x0,        // CB_COLOR5_FMASK_BASE_EXT
      0x0,        // CB_COLOR6_FMASK_BASE_EXT
      0x0,        // CB_COLOR7_FMASK_BASE_EXT
      0x0,        // CB_COLOR0_DCC_BASE_EXT
      0x0,        // CB_COLOR1_DCC_BASE_EXT
      0x0,        // CB_COLOR2_DCC_BASE_EXT
      0x0,        // CB_COLOR3_DCC_BASE_EXT
      0x0,        // CB_COLOR4_DCC_BASE_EXT
      0x0,        // CB_COLOR5_DCC_BASE_EXT
      0x0,        // CB_COLOR6_DCC_BASE_EXT
      0x0,        // CB_COLOR7_DCC_BASE_EXT
      0x0,        // CB_COLOR0_ATTRIB2
      0x0,        // CB_COLOR1_ATTRIB2
      0x0,        // CB_COLOR2_ATTRIB2
      0x0,        // CB_COLOR3_ATTRIB2
      0x0,        // CB_COLOR4_ATTRIB2
      0x0,        // CB_COLOR5_ATTRIB2
      0x0,        // CB_COLOR6_ATTRIB2
      0x0,        // CB_COLOR7_ATTRIB2
      0x0,        // CB_COLOR0_ATTRIB3
      0x0,        // CB_COLOR1_ATTRIB3
      0x0,        // CB_COLOR2_ATTRIB3
      0x0,        // CB_COLOR3_ATTRIB3
      0x0,        // CB_COLOR4_ATTRIB3
      0x0,        // CB_COLOR5_ATTRIB3
      0x0,        // CB_COLOR6_ATTRIB3
      0x0         // CB_COLOR7_ATTRIB3
   };

   set_context_reg_seq_array(cs, R_028000_DB_RENDER_CONTROL, SET(DbRenderControlGfx103));
   set_context_reg_seq_array(cs, R_0281E8_COHER_DEST_BASE_HI_0, SET(CoherDestBaseHi0Gfx103));
   set_context_reg_seq_array(cs, R_02840C_VGT_MULTI_PRIM_IB_RESET_INDX,
                             SET(VgtMultiPrimIbResetIndxGfx103));
   set_context_reg_seq_array(cs, R_028644_SPI_PS_INPUT_CNTL_0, SET(SpiPsInputCntl0Gfx103));
   set_context_reg_seq_array(cs, R_028750_SX_PS_DOWNCONVERT_CONTROL,
                             SET(SxPsDownconvertControlGfx103));
   set_context_reg_seq_array(cs, R_0287D4_PA_CL_POINT_X_RAD, SET(PaClPointXRadGfx103));
   set_context_reg_seq_array(cs, R_0287FC_GE_MAX_OUTPUT_PER_SUBGROUP,
                             SET(GeMaxOutputPerSubgroupGfx103));
   set_context_reg_seq_array(cs, R_028A00_PA_SU_POINT_SIZE, SET(PaSuPointSizeGfx103));
   set_context_reg_seq_array(cs, R_028A18_VGT_HOS_MAX_TESS_LEVEL, SET(VgtHosMaxTessLevelGfx103));
   set_context_reg_seq_array(cs, R_028A40_VGT_GS_MODE, SET(VgtGsModeGfx103));
   set_context_reg_seq_array(cs, R_028A84_VGT_PRIMITIVEID_EN, SET(VgtPrimitiveidEnGfx103));
   set_context_reg_seq_array(cs, R_028A8C_VGT_PRIMITIVEID_RESET, SET(VgtPrimitiveidResetGfx103));
   set_context_reg_seq_array(cs, R_028A98_VGT_DRAW_PAYLOAD_CNTL, SET(VgtDrawPayloadCntlGfx103));
   set_context_reg_seq_array(cs, R_028BD4_PA_SC_CENTROID_PRIORITY_0,
                             SET(PaScCentroidPriority0Gfx103));

   for (unsigned i = 0; i < num_reg_pairs; i++)
      set_context_reg_seq_array(cs, reg_offsets[i], 1, &reg_values[i]);
}

/**
 * Emulate CLEAR_STATE. Additionally, initialize num_reg_pairs registers specified
 * via reg_offsets and reg_values.
 */
static void gfx11_emulate_clear_state(struct radeon_cmdbuf *cs, unsigned num_reg_pairs,
                                      unsigned *reg_offsets, uint32_t *reg_values,
                                      set_context_reg_seq_array_fn set_context_reg_seq_array)
{
   static const uint32_t DbRenderControlGfx11[] = {
      0x0,        // DB_RENDER_CONTROL
      0x0,        // DB_COUNT_CONTROL
      0x0,        // DB_DEPTH_VIEW
      0x0,        // DB_RENDER_OVERRIDE
      0x0,        // DB_RENDER_OVERRIDE2
      0x0,        // DB_HTILE_DATA_BASE
      0x0,        //
      0x0,        // DB_DEPTH_SIZE_XY
      0x0,        // DB_DEPTH_BOUNDS_MIN
      0x0,        // DB_DEPTH_BOUNDS_MAX
      0x0,        // DB_STENCIL_CLEAR
      0x0,        // DB_DEPTH_CLEAR
      0x0,        // PA_SC_SCREEN_SCISSOR_TL
      0x40004000, // PA_SC_SCREEN_SCISSOR_BR
      0x0,        //
      0x0,        // DB_RESERVED_REG_2
      0x0,        // DB_Z_INFO
      0x0,        // DB_STENCIL_INFO
      0x0,        // DB_Z_READ_BASE
      0x0,        // DB_STENCIL_READ_BASE
      0x0,        // DB_Z_WRITE_BASE
      0x0,        // DB_STENCIL_WRITE_BASE
      0x0,        // DB_RESERVED_REG_1
      0x0,        // DB_RESERVED_REG_3
      0x0,        // DB_SPI_VRS_CENTER_LOCATION
      0x0,        //
      0x0,        // DB_Z_READ_BASE_HI
      0x0,        // DB_STENCIL_READ_BASE_HI
      0x0,        // DB_Z_WRITE_BASE_HI
      0x0,        // DB_STENCIL_WRITE_BASE_HI
      0x0,        // DB_HTILE_DATA_BASE_HI
      0x0,        // DB_RMI_L2_CACHE_CONTROL
      0x0,        // TA_BC_BASE_ADDR
      0x0,        // TA_BC_BASE_ADDR_HI
   };
   static const uint32_t CoherDestBaseHi0Gfx11[] = {
      0x0,        // COHER_DEST_BASE_HI_0
      0x0,        // COHER_DEST_BASE_HI_1
      0x0,        // COHER_DEST_BASE_HI_2
      0x0,        // COHER_DEST_BASE_HI_3
      0x0,        // COHER_DEST_BASE_2
      0x0,        // COHER_DEST_BASE_3
      0x0,        // PA_SC_WINDOW_OFFSET
      0x80000000, // PA_SC_WINDOW_SCISSOR_TL
      0x40004000, // PA_SC_WINDOW_SCISSOR_BR
      0xffff,     // PA_SC_CLIPRECT_RULE
      0x0,        // PA_SC_CLIPRECT_0_TL
      0x40004000, // PA_SC_CLIPRECT_0_BR
      0x0,        // PA_SC_CLIPRECT_1_TL
      0x40004000, // PA_SC_CLIPRECT_1_BR
      0x0,        // PA_SC_CLIPRECT_2_TL
      0x40004000, // PA_SC_CLIPRECT_2_BR
      0x0,        // PA_SC_CLIPRECT_3_TL
      0x40004000, // PA_SC_CLIPRECT_3_BR
      0xaa99aaaa, // PA_SC_EDGERULE
      0x0,        // PA_SU_HARDWARE_SCREEN_OFFSET
      0xffffffff, // CB_TARGET_MASK
      0xffffffff, // CB_SHADER_MASK
      0x80000000, // PA_SC_GENERIC_SCISSOR_TL
      0x40004000, // PA_SC_GENERIC_SCISSOR_BR
      0x0,        // COHER_DEST_BASE_0
      0x0,        // COHER_DEST_BASE_1
      0x80000000, // PA_SC_VPORT_SCISSOR_0_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_0_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_1_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_1_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_2_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_2_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_3_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_3_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_4_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_4_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_5_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_5_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_6_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_6_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_7_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_7_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_8_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_8_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_9_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_9_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_10_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_10_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_11_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_11_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_12_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_12_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_13_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_13_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_14_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_14_BR
      0x80000000, // PA_SC_VPORT_SCISSOR_15_TL
      0x40004000, // PA_SC_VPORT_SCISSOR_15_BR
      0x0,        // PA_SC_VPORT_ZMIN_0
      0x3f800000, // PA_SC_VPORT_ZMAX_0
      0x0,        // PA_SC_VPORT_ZMIN_1
      0x3f800000, // PA_SC_VPORT_ZMAX_1
      0x0,        // PA_SC_VPORT_ZMIN_2
      0x3f800000, // PA_SC_VPORT_ZMAX_2
      0x0,        // PA_SC_VPORT_ZMIN_3
      0x3f800000, // PA_SC_VPORT_ZMAX_3
      0x0,        // PA_SC_VPORT_ZMIN_4
      0x3f800000, // PA_SC_VPORT_ZMAX_4
      0x0,        // PA_SC_VPORT_ZMIN_5
      0x3f800000, // PA_SC_VPORT_ZMAX_5
      0x0,        // PA_SC_VPORT_ZMIN_6
      0x3f800000, // PA_SC_VPORT_ZMAX_6
      0x0,        // PA_SC_VPORT_ZMIN_7
      0x3f800000, // PA_SC_VPORT_ZMAX_7
      0x0,        // PA_SC_VPORT_ZMIN_8
      0x3f800000, // PA_SC_VPORT_ZMAX_8
      0x0,        // PA_SC_VPORT_ZMIN_9
      0x3f800000, // PA_SC_VPORT_ZMAX_9
      0x0,        // PA_SC_VPORT_ZMIN_10
      0x3f800000, // PA_SC_VPORT_ZMAX_10
      0x0,        // PA_SC_VPORT_ZMIN_11
      0x3f800000, // PA_SC_VPORT_ZMAX_11
      0x0,        // PA_SC_VPORT_ZMIN_12
      0x3f800000, // PA_SC_VPORT_ZMAX_12
      0x0,        // PA_SC_VPORT_ZMIN_13
      0x3f800000, // PA_SC_VPORT_ZMAX_13
      0x0,        // PA_SC_VPORT_ZMIN_14
      0x3f800000, // PA_SC_VPORT_ZMAX_14
      0x0,        // PA_SC_VPORT_ZMIN_15
      0x3f800000, // PA_SC_VPORT_ZMAX_15
      0x0,        // PA_SC_RASTER_CONFIG
      0x0,        // PA_SC_RASTER_CONFIG_1
      0x0,        //
      0x0,        // PA_SC_TILE_STEERING_OVERRIDE
   };
   static const uint32_t PaScVrsOverrideCntlGfx11[] = {
      0x0,        // PA_SC_VRS_OVERRIDE_CNTL
      0x0,        // PA_SC_VRS_RATE_FEEDBACK_BASE
      0x0,        // PA_SC_VRS_RATE_FEEDBACK_BASE_EXT
      0x0,        // PA_SC_VRS_RATE_FEEDBACK_SIZE_XY
      0x0,        //
      0x0,        // PA_SC_VRS_RATE_CACHE_CNTL
   };
   static const uint32_t PaScVrsRateBaseGfx11[] = {
      0x0,        // PA_SC_VRS_RATE_BASE
      0x0,        // PA_SC_VRS_RATE_BASE_EXT
      0x0,        // PA_SC_VRS_RATE_SIZE_XY
   };
   static const uint32_t VgtMultiPrimIbResetIndxGfx11[] = {
      0x0,        // VGT_MULTI_PRIM_IB_RESET_INDX
      0x0,        // CB_RMI_GL2_CACHE_CONTROL
      0x0,        // CB_BLEND_RED
      0x0,        // CB_BLEND_GREEN
      0x0,        // CB_BLEND_BLUE
      0x0,        // CB_BLEND_ALPHA
      0x0,        // CB_FDCC_CONTROL
      0x0,        // CB_COVERAGE_OUT_CONTROL
      0x0,        // DB_STENCIL_CONTROL
      0x1000000,  // DB_STENCILREFMASK
      0x1000000,  // DB_STENCILREFMASK_BF
      0x0,        //
      0x0,        // PA_CL_VPORT_XSCALE
      0x0,        // PA_CL_VPORT_XOFFSET
      0x0,        // PA_CL_VPORT_YSCALE
      0x0,        // PA_CL_VPORT_YOFFSET
      0x0,        // PA_CL_VPORT_ZSCALE
      0x0,        // PA_CL_VPORT_ZOFFSET
      0x0,        // PA_CL_VPORT_XSCALE_1
      0x0,        // PA_CL_VPORT_XOFFSET_1
      0x0,        // PA_CL_VPORT_YSCALE_1
      0x0,        // PA_CL_VPORT_YOFFSET_1
      0x0,        // PA_CL_VPORT_ZSCALE_1
      0x0,        // PA_CL_VPORT_ZOFFSET_1
      0x0,        // PA_CL_VPORT_XSCALE_2
      0x0,        // PA_CL_VPORT_XOFFSET_2
      0x0,        // PA_CL_VPORT_YSCALE_2
      0x0,        // PA_CL_VPORT_YOFFSET_2
      0x0,        // PA_CL_VPORT_ZSCALE_2
      0x0,        // PA_CL_VPORT_ZOFFSET_2
      0x0,        // PA_CL_VPORT_XSCALE_3
      0x0,        // PA_CL_VPORT_XOFFSET_3
      0x0,        // PA_CL_VPORT_YSCALE_3
      0x0,        // PA_CL_VPORT_YOFFSET_3
      0x0,        // PA_CL_VPORT_ZSCALE_3
      0x0,        // PA_CL_VPORT_ZOFFSET_3
      0x0,        // PA_CL_VPORT_XSCALE_4
      0x0,        // PA_CL_VPORT_XOFFSET_4
      0x0,        // PA_CL_VPORT_YSCALE_4
      0x0,        // PA_CL_VPORT_YOFFSET_4
      0x0,        // PA_CL_VPORT_ZSCALE_4
      0x0,        // PA_CL_VPORT_ZOFFSET_4
      0x0,        // PA_CL_VPORT_XSCALE_5
      0x0,        // PA_CL_VPORT_XOFFSET_5
      0x0,        // PA_CL_VPORT_YSCALE_5
      0x0,        // PA_CL_VPORT_YOFFSET_5
      0x0,        // PA_CL_VPORT_ZSCALE_5
      0x0,        // PA_CL_VPORT_ZOFFSET_5
      0x0,        // PA_CL_VPORT_XSCALE_6
      0x0,        // PA_CL_VPORT_XOFFSET_6
      0x0,        // PA_CL_VPORT_YSCALE_6
      0x0,        // PA_CL_VPORT_YOFFSET_6
      0x0,        // PA_CL_VPORT_ZSCALE_6
      0x0,        // PA_CL_VPORT_ZOFFSET_6
      0x0,        // PA_CL_VPORT_XSCALE_7
      0x0,        // PA_CL_VPORT_XOFFSET_7
      0x0,        // PA_CL_VPORT_YSCALE_7
      0x0,        // PA_CL_VPORT_YOFFSET_7
      0x0,        // PA_CL_VPORT_ZSCALE_7
      0x0,        // PA_CL_VPORT_ZOFFSET_7
      0x0,        // PA_CL_VPORT_XSCALE_8
      0x0,        // PA_CL_VPORT_XOFFSET_8
      0x0,        // PA_CL_VPORT_YSCALE_8
      0x0,        // PA_CL_VPORT_YOFFSET_8
      0x0,        // PA_CL_VPORT_ZSCALE_8
      0x0,        // PA_CL_VPORT_ZOFFSET_8
      0x0,        // PA_CL_VPORT_XSCALE_9
      0x0,        // PA_CL_VPORT_XOFFSET_9
      0x0,        // PA_CL_VPORT_YSCALE_9
      0x0,        // PA_CL_VPORT_YOFFSET_9
      0x0,        // PA_CL_VPORT_ZSCALE_9
      0x0,        // PA_CL_VPORT_ZOFFSET_9
      0x0,        // PA_CL_VPORT_XSCALE_10
      0x0,        // PA_CL_VPORT_XOFFSET_10
      0x0,        // PA_CL_VPORT_YSCALE_10
      0x0,        // PA_CL_VPORT_YOFFSET_10
      0x0,        // PA_CL_VPORT_ZSCALE_10
      0x0,        // PA_CL_VPORT_ZOFFSET_10
      0x0,        // PA_CL_VPORT_XSCALE_11
      0x0,        // PA_CL_VPORT_XOFFSET_11
      0x0,        // PA_CL_VPORT_YSCALE_11
      0x0,        // PA_CL_VPORT_YOFFSET_11
      0x0,        // PA_CL_VPORT_ZSCALE_11
      0x0,        // PA_CL_VPORT_ZOFFSET_11
      0x0,        // PA_CL_VPORT_XSCALE_12
      0x0,        // PA_CL_VPORT_XOFFSET_12
      0x0,        // PA_CL_VPORT_YSCALE_12
      0x0,        // PA_CL_VPORT_YOFFSET_12
      0x0,        // PA_CL_VPORT_ZSCALE_12
      0x0,        // PA_CL_VPORT_ZOFFSET_12
      0x0,        // PA_CL_VPORT_XSCALE_13
      0x0,        // PA_CL_VPORT_XOFFSET_13
      0x0,        // PA_CL_VPORT_YSCALE_13
      0x0,        // PA_CL_VPORT_YOFFSET_13
      0x0,        // PA_CL_VPORT_ZSCALE_13
      0x0,        // PA_CL_VPORT_ZOFFSET_13
      0x0,        // PA_CL_VPORT_XSCALE_14
      0x0,        // PA_CL_VPORT_XOFFSET_14
      0x0,        // PA_CL_VPORT_YSCALE_14
      0x0,        // PA_CL_VPORT_YOFFSET_14
      0x0,        // PA_CL_VPORT_ZSCALE_14
      0x0,        // PA_CL_VPORT_ZOFFSET_14
      0x0,        // PA_CL_VPORT_XSCALE_15
      0x0,        // PA_CL_VPORT_XOFFSET_15
      0x0,        // PA_CL_VPORT_YSCALE_15
      0x0,        // PA_CL_VPORT_YOFFSET_15
      0x0,        // PA_CL_VPORT_ZSCALE_15
      0x0,        // PA_CL_VPORT_ZOFFSET_15
      0x0,        // PA_CL_UCP_0_X
      0x0,        // PA_CL_UCP_0_Y
      0x0,        // PA_CL_UCP_0_Z
      0x0,        // PA_CL_UCP_0_W
      0x0,        // PA_CL_UCP_1_X
      0x0,        // PA_CL_UCP_1_Y
      0x0,        // PA_CL_UCP_1_Z
      0x0,        // PA_CL_UCP_1_W
      0x0,        // PA_CL_UCP_2_X
      0x0,        // PA_CL_UCP_2_Y
      0x0,        // PA_CL_UCP_2_Z
      0x0,        // PA_CL_UCP_2_W
      0x0,        // PA_CL_UCP_3_X
      0x0,        // PA_CL_UCP_3_Y
      0x0,        // PA_CL_UCP_3_Z
      0x0,        // PA_CL_UCP_3_W
      0x0,        // PA_CL_UCP_4_X
      0x0,        // PA_CL_UCP_4_Y
      0x0,        // PA_CL_UCP_4_Z
      0x0,        // PA_CL_UCP_4_W
      0x0,        // PA_CL_UCP_5_X
      0x0,        // PA_CL_UCP_5_Y
      0x0,        // PA_CL_UCP_5_Z
      0x0,        // PA_CL_UCP_5_W
   };
   static const uint32_t SpiPsInputCntl0Gfx11[] = {
      0x0,        // SPI_PS_INPUT_CNTL_0
      0x0,        // SPI_PS_INPUT_CNTL_1
      0x0,        // SPI_PS_INPUT_CNTL_2
      0x0,        // SPI_PS_INPUT_CNTL_3
      0x0,        // SPI_PS_INPUT_CNTL_4
      0x0,        // SPI_PS_INPUT_CNTL_5
      0x0,        // SPI_PS_INPUT_CNTL_6
      0x0,        // SPI_PS_INPUT_CNTL_7
      0x0,        // SPI_PS_INPUT_CNTL_8
      0x0,        // SPI_PS_INPUT_CNTL_9
      0x0,        // SPI_PS_INPUT_CNTL_10
      0x0,        // SPI_PS_INPUT_CNTL_11
      0x0,        // SPI_PS_INPUT_CNTL_12
      0x0,        // SPI_PS_INPUT_CNTL_13
      0x0,        // SPI_PS_INPUT_CNTL_14
      0x0,        // SPI_PS_INPUT_CNTL_15
      0x0,        // SPI_PS_INPUT_CNTL_16
      0x0,        // SPI_PS_INPUT_CNTL_17
      0x0,        // SPI_PS_INPUT_CNTL_18
      0x0,        // SPI_PS_INPUT_CNTL_19
      0x0,        // SPI_PS_INPUT_CNTL_20
      0x0,        // SPI_PS_INPUT_CNTL_21
      0x0,        // SPI_PS_INPUT_CNTL_22
      0x0,        // SPI_PS_INPUT_CNTL_23
      0x0,        // SPI_PS_INPUT_CNTL_24
      0x0,        // SPI_PS_INPUT_CNTL_25
      0x0,        // SPI_PS_INPUT_CNTL_26
      0x0,        // SPI_PS_INPUT_CNTL_27
      0x0,        // SPI_PS_INPUT_CNTL_28
      0x0,        // SPI_PS_INPUT_CNTL_29
      0x0,        // SPI_PS_INPUT_CNTL_30
      0x0,        // SPI_PS_INPUT_CNTL_31
      0x0,        // SPI_VS_OUT_CONFIG
      0x0,        //
      0x0,        // SPI_PS_INPUT_ENA
      0x0,        // SPI_PS_INPUT_ADDR
      0x0,        // SPI_INTERP_CONTROL_0
      0x2,        // SPI_PS_IN_CONTROL
      0x0,        // SPI_BARYC_SSAA_CNTL
      0x0,        // SPI_BARYC_CNTL
      0x0,        //
      0x0,        // SPI_TMPRING_SIZE
      0x0,        // SPI_GFX_SCRATCH_BASE_LO
      0x0,        // SPI_GFX_SCRATCH_BASE_HI
   };
   static const uint32_t SpiShaderIdxFormatGfx11[] = {
      0x0,        // SPI_SHADER_IDX_FORMAT
      0x0,        // SPI_SHADER_POS_FORMAT
      0x0,        // SPI_SHADER_Z_FORMAT
      0x0,        // SPI_SHADER_COL_FORMAT
   };
   static const uint32_t SxPsDownconvertControlGfx11[] = {
      0x0,        // SX_PS_DOWNCONVERT_CONTROL
      0x0,        // SX_PS_DOWNCONVERT
      0x0,        // SX_BLEND_OPT_EPSILON
      0x0,        // SX_BLEND_OPT_CONTROL
      0x0,        // SX_MRT0_BLEND_OPT
      0x0,        // SX_MRT1_BLEND_OPT
      0x0,        // SX_MRT2_BLEND_OPT
      0x0,        // SX_MRT3_BLEND_OPT
      0x0,        // SX_MRT4_BLEND_OPT
      0x0,        // SX_MRT5_BLEND_OPT
      0x0,        // SX_MRT6_BLEND_OPT
      0x0,        // SX_MRT7_BLEND_OPT
      0x0,        // CB_BLEND0_CONTROL
      0x0,        // CB_BLEND1_CONTROL
      0x0,        // CB_BLEND2_CONTROL
      0x0,        // CB_BLEND3_CONTROL
      0x0,        // CB_BLEND4_CONTROL
      0x0,        // CB_BLEND5_CONTROL
      0x0,        // CB_BLEND6_CONTROL
      0x0,        // CB_BLEND7_CONTROL
   };
   static const uint32_t PaClPointXRadGfx11[] = {
      0x0,        // PA_CL_POINT_X_RAD
      0x0,        // PA_CL_POINT_Y_RAD
      0x0,        // PA_CL_POINT_SIZE
      0x0,        // PA_CL_POINT_CULL_RAD
   };
   static const uint32_t GeMaxOutputPerSubgroupGfx11[] = {
      0x0,        // GE_MAX_OUTPUT_PER_SUBGROUP
      0x0,        // DB_DEPTH_CONTROL
      0x0,        // DB_EQAA
      0x0,        // CB_COLOR_CONTROL
      0x0,        // DB_SHADER_CONTROL
      0x90000,    // PA_CL_CLIP_CNTL
      0x4,        // PA_SU_SC_MODE_CNTL
      0x0,        // PA_CL_VTE_CNTL
      0x0,        // PA_CL_VS_OUT_CNTL
      0x0,        // PA_CL_NANINF_CNTL
      0x0,        // PA_SU_LINE_STIPPLE_CNTL
      0x0,        // PA_SU_LINE_STIPPLE_SCALE
      0x0,        // PA_SU_PRIM_FILTER_CNTL
      0x0,        // PA_SU_SMALL_PRIM_FILTER_CNTL
      0x0,        //
      0x0,        // PA_CL_NGG_CNTL
      0x0,        // PA_SU_OVER_RASTERIZATION_CNTL
      0x0,        // PA_STEREO_CNTL
      0x0,        // PA_STATE_STEREO_X
      0x0,        // PA_CL_VRS_CNTL
   };
   static const uint32_t PaSuPointSizeGfx11[] = {
      0x0,        // PA_SU_POINT_SIZE
      0x0,        // PA_SU_POINT_MINMAX
      0x0,        // PA_SU_LINE_CNTL
      0x0,        // PA_SC_LINE_STIPPLE
   };
   static const uint32_t VgtHosMaxTessLevelGfx11[] = {
      0x0,        // VGT_HOS_MAX_TESS_LEVEL
      0x0,        // VGT_HOS_MIN_TESS_LEVEL
   };
   static const uint32_t PaScModeCntl0Gfx11[] = {
      0x0,        // PA_SC_MODE_CNTL_0
      0x0,        // PA_SC_MODE_CNTL_1
      0x0,        // VGT_ENHANCE
   };
   static const uint32_t VgtPrimitiveidEnGfx11[] = {
      0x0,        // VGT_PRIMITIVEID_EN
   };
   static const uint32_t VgtPrimitiveidResetGfx11[] = {
      0x0,        // VGT_PRIMITIVEID_RESET
   };
   static const uint32_t VgtDrawPayloadCntlGfx11[] = {
      0x0,        // VGT_DRAW_PAYLOAD_CNTL
   };
   static const uint32_t VgtEsgsRingItemsizeGfx11[] = {
      0x0,        // VGT_ESGS_RING_ITEMSIZE
      0x0,        //
      0x0,        // VGT_REUSE_OFF
      0x0,        //
      0x0,        // DB_HTILE_SURFACE
      0x0,        // DB_SRESULTS_COMPARE_STATE0
      0x0,        // DB_SRESULTS_COMPARE_STATE1
   };
   static const uint32_t VgtStrmoutDrawOpaqueOffsetGfx11[] = {
      0x0,        // VGT_STRMOUT_DRAW_OPAQUE_OFFSET
      0x0,        // VGT_STRMOUT_DRAW_OPAQUE_BUFFER_FILLED_SIZE
      0x0,        // VGT_STRMOUT_DRAW_OPAQUE_VERTEX_STRIDE
      0x0,        //
      0x0,        // VGT_GS_MAX_VERT_OUT
   };
   static const uint32_t GeNggSubgrpCntlGfx11[] = {
      0x0,        // GE_NGG_SUBGRP_CNTL
      0x0,        // VGT_TESS_DISTRIBUTION
      0x0,        // VGT_SHADER_STAGES_EN
      0x0,        // VGT_LS_HS_CONFIG
   };
   static const uint32_t VgtTfParamGfx11[] = {
      0x0,        // VGT_TF_PARAM
      0x0,        // DB_ALPHA_TO_MASK
      0x0,        //
      0x0,        // PA_SU_POLY_OFFSET_DB_FMT_CNTL
      0x0,        // PA_SU_POLY_OFFSET_CLAMP
      0x0,        // PA_SU_POLY_OFFSET_FRONT_SCALE
      0x0,        // PA_SU_POLY_OFFSET_FRONT_OFFSET
      0x0,        // PA_SU_POLY_OFFSET_BACK_SCALE
      0x0,        // PA_SU_POLY_OFFSET_BACK_OFFSET
      0x0,        // VGT_GS_INSTANCE_CNT
   };
   static const uint32_t PaScCentroidPriority0Gfx11[] = {
      0x0,        // PA_SC_CENTROID_PRIORITY_0
      0x0,        // PA_SC_CENTROID_PRIORITY_1
      0x1000,     // PA_SC_LINE_CNTL
      0x0,        // PA_SC_AA_CONFIG
      0x5,        // PA_SU_VTX_CNTL
      0x3f800000, // PA_CL_GB_VERT_CLIP_ADJ
      0x3f800000, // PA_CL_GB_VERT_DISC_ADJ
      0x3f800000, // PA_CL_GB_HORZ_CLIP_ADJ
      0x3f800000, // PA_CL_GB_HORZ_DISC_ADJ
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_1
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_2
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_3
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_0
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_1
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_2
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_3
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_0
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_1
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_2
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_3
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_0
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_1
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_2
      0x0,        // PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_3
      0xffffffff, // PA_SC_AA_MASK_X0Y0_X1Y0
      0xffffffff, // PA_SC_AA_MASK_X0Y1_X1Y1
      0x0,        // PA_SC_SHADER_CONTROL
      0x3,        // PA_SC_BINNER_CNTL_0
      0x0,        // PA_SC_BINNER_CNTL_1
      0x100000,   // PA_SC_CONSERVATIVE_RASTERIZATION_CNTL
      0x0,        // PA_SC_NGG_MODE_CNTL
      0x0,        // PA_SC_BINNER_CNTL_2
   };
   static const uint32_t CbColor0BaseGfx11[] = {
      0x0,        // CB_COLOR0_BASE
   };
   static const uint32_t CbColor0ViewGfx11[] = {
      0x0,        // CB_COLOR0_VIEW
      0x0,        // CB_COLOR0_INFO
      0x0,        // CB_COLOR0_ATTRIB
      0x0,        // CB_COLOR0_DCC_CONTROL
   };
   static const uint32_t CbColor0DccBaseGfx11[] = {
      0x0,        // CB_COLOR0_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR1_BASE
   };
   static const uint32_t CbColor1ViewGfx11[] = {
      0x0,        // CB_COLOR1_VIEW
      0x0,        // CB_COLOR1_INFO
      0x0,        // CB_COLOR1_ATTRIB
      0x0,        // CB_COLOR1_DCC_CONTROL
   };
   static const uint32_t CbColor1DccBaseGfx11[] = {
      0x0,        // CB_COLOR1_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR2_BASE
   };
   static const uint32_t CbColor2ViewGfx11[] = {
      0x0,        // CB_COLOR2_VIEW
      0x0,        // CB_COLOR2_INFO
      0x0,        // CB_COLOR2_ATTRIB
      0x0,        // CB_COLOR2_DCC_CONTROL
   };
   static const uint32_t CbColor2DccBaseGfx11[] = {
      0x0,        // CB_COLOR2_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR3_BASE
   };
   static const uint32_t CbColor3ViewGfx11[] = {
      0x0,        // CB_COLOR3_VIEW
      0x0,        // CB_COLOR3_INFO
      0x0,        // CB_COLOR3_ATTRIB
      0x0,        // CB_COLOR3_DCC_CONTROL
   };
   static const uint32_t CbColor3DccBaseGfx11[] = {
      0x0,        // CB_COLOR3_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR4_BASE
   };
   static const uint32_t CbColor4ViewGfx11[] = {
      0x0,        // CB_COLOR4_VIEW
      0x0,        // CB_COLOR4_INFO
      0x0,        // CB_COLOR4_ATTRIB
      0x0,        // CB_COLOR4_DCC_CONTROL
   };
   static const uint32_t CbColor4DccBaseGfx11[] = {
      0x0,        // CB_COLOR4_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR5_BASE
   };
   static const uint32_t CbColor5ViewGfx11[] = {
      0x0,        // CB_COLOR5_VIEW
      0x0,        // CB_COLOR5_INFO
      0x0,        // CB_COLOR5_ATTRIB
      0x0,        // CB_COLOR5_DCC_CONTROL
   };
   static const uint32_t CbColor5DccBaseGfx11[] = {
      0x0,        // CB_COLOR5_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR6_BASE
   };
   static const uint32_t CbColor6ViewGfx11[] = {
      0x0,        // CB_COLOR6_VIEW
      0x0,        // CB_COLOR6_INFO
      0x0,        // CB_COLOR6_ATTRIB
      0x0,        // CB_COLOR6_DCC_CONTROL
   };
   static const uint32_t CbColor6DccBaseGfx11[] = {
      0x0,        // CB_COLOR6_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR7_BASE
   };
   static const uint32_t CbColor7ViewGfx11[] = {
      0x0,        // CB_COLOR7_VIEW
      0x0,        // CB_COLOR7_INFO
      0x0,        // CB_COLOR7_ATTRIB
      0x0,        // CB_COLOR7_DCC_CONTROL
   };
   static const uint32_t CbColor7DccBaseGfx11[] = {
      0x0,        // CB_COLOR7_DCC_BASE
      0x0,        //
      0x0,        // CB_COLOR0_BASE_EXT
      0x0,        // CB_COLOR1_BASE_EXT
      0x0,        // CB_COLOR2_BASE_EXT
      0x0,        // CB_COLOR3_BASE_EXT
      0x0,        // CB_COLOR4_BASE_EXT
      0x0,        // CB_COLOR5_BASE_EXT
      0x0,        // CB_COLOR6_BASE_EXT
      0x0,        // CB_COLOR7_BASE_EXT
   };
   static const uint32_t CbColor0DccBaseExtGfx11[] = {
      0x0,        // CB_COLOR0_DCC_BASE_EXT
      0x0,        // CB_COLOR1_DCC_BASE_EXT
      0x0,        // CB_COLOR2_DCC_BASE_EXT
      0x0,        // CB_COLOR3_DCC_BASE_EXT
      0x0,        // CB_COLOR4_DCC_BASE_EXT
      0x0,        // CB_COLOR5_DCC_BASE_EXT
      0x0,        // CB_COLOR6_DCC_BASE_EXT
      0x0,        // CB_COLOR7_DCC_BASE_EXT
      0x0,        // CB_COLOR0_ATTRIB2
      0x0,        // CB_COLOR1_ATTRIB2
      0x0,        // CB_COLOR2_ATTRIB2
      0x0,        // CB_COLOR3_ATTRIB2
      0x0,        // CB_COLOR4_ATTRIB2
      0x0,        // CB_COLOR5_ATTRIB2
      0x0,        // CB_COLOR6_ATTRIB2
      0x0,        // CB_COLOR7_ATTRIB2
      0x0,        // CB_COLOR0_ATTRIB3
      0x0,        // CB_COLOR1_ATTRIB3
      0x0,        // CB_COLOR2_ATTRIB3
      0x0,        // CB_COLOR3_ATTRIB3
      0x0,        // CB_COLOR4_ATTRIB3
      0x0,        // CB_COLOR5_ATTRIB3
      0x0,        // CB_COLOR6_ATTRIB3
      0x0,        // CB_COLOR7_ATTRIB3
   };

   set_context_reg_seq_array(cs, R_028000_DB_RENDER_CONTROL, SET(DbRenderControlGfx11));
   set_context_reg_seq_array(cs, R_0281E8_COHER_DEST_BASE_HI_0, SET(CoherDestBaseHi0Gfx11));
   set_context_reg_seq_array(cs, R_0283D0_PA_SC_VRS_OVERRIDE_CNTL, SET(PaScVrsOverrideCntlGfx11));
   set_context_reg_seq_array(cs, R_0283F0_PA_SC_VRS_RATE_BASE, SET(PaScVrsRateBaseGfx11));
   set_context_reg_seq_array(cs, R_02840C_VGT_MULTI_PRIM_IB_RESET_INDX, SET(VgtMultiPrimIbResetIndxGfx11));
   set_context_reg_seq_array(cs, R_028644_SPI_PS_INPUT_CNTL_0, SET(SpiPsInputCntl0Gfx11));
   set_context_reg_seq_array(cs, R_028708_SPI_SHADER_IDX_FORMAT, SET(SpiShaderIdxFormatGfx11));
   set_context_reg_seq_array(cs, R_028750_SX_PS_DOWNCONVERT_CONTROL, SET(SxPsDownconvertControlGfx11));
   set_context_reg_seq_array(cs, R_0287D4_PA_CL_POINT_X_RAD, SET(PaClPointXRadGfx11));
   set_context_reg_seq_array(cs, R_0287FC_GE_MAX_OUTPUT_PER_SUBGROUP, SET(GeMaxOutputPerSubgroupGfx11));
   set_context_reg_seq_array(cs, R_028A00_PA_SU_POINT_SIZE, SET(PaSuPointSizeGfx11));
   set_context_reg_seq_array(cs, R_028A18_VGT_HOS_MAX_TESS_LEVEL, SET(VgtHosMaxTessLevelGfx11));
   set_context_reg_seq_array(cs, R_028A48_PA_SC_MODE_CNTL_0, SET(PaScModeCntl0Gfx11));
   set_context_reg_seq_array(cs, R_028A84_VGT_PRIMITIVEID_EN, SET(VgtPrimitiveidEnGfx11));
   set_context_reg_seq_array(cs, R_028A8C_VGT_PRIMITIVEID_RESET, SET(VgtPrimitiveidResetGfx11));
   set_context_reg_seq_array(cs, R_028A98_VGT_DRAW_PAYLOAD_CNTL, SET(VgtDrawPayloadCntlGfx11));
   set_context_reg_seq_array(cs, R_028AAC_VGT_ESGS_RING_ITEMSIZE, SET(VgtEsgsRingItemsizeGfx11));
   set_context_reg_seq_array(cs, R_028B28_VGT_STRMOUT_DRAW_OPAQUE_OFFSET, SET(VgtStrmoutDrawOpaqueOffsetGfx11));
   set_context_reg_seq_array(cs, R_028B4C_GE_NGG_SUBGRP_CNTL, SET(GeNggSubgrpCntlGfx11));
   set_context_reg_seq_array(cs, R_028B6C_VGT_TF_PARAM, SET(VgtTfParamGfx11));
   set_context_reg_seq_array(cs, R_028BD4_PA_SC_CENTROID_PRIORITY_0, SET(PaScCentroidPriority0Gfx11));
   set_context_reg_seq_array(cs, R_028C60_CB_COLOR0_BASE, SET(CbColor0BaseGfx11));
   set_context_reg_seq_array(cs, R_028C6C_CB_COLOR0_VIEW, SET(CbColor0ViewGfx11));
   set_context_reg_seq_array(cs, R_028C94_CB_COLOR0_DCC_BASE, SET(CbColor0DccBaseGfx11));
   set_context_reg_seq_array(cs, R_028CA8_CB_COLOR1_VIEW, SET(CbColor1ViewGfx11));
   set_context_reg_seq_array(cs, R_028CD0_CB_COLOR1_DCC_BASE, SET(CbColor1DccBaseGfx11));
   set_context_reg_seq_array(cs, R_028CE4_CB_COLOR2_VIEW, SET(CbColor2ViewGfx11));
   set_context_reg_seq_array(cs, R_028D0C_CB_COLOR2_DCC_BASE, SET(CbColor2DccBaseGfx11));
   set_context_reg_seq_array(cs, R_028D20_CB_COLOR3_VIEW, SET(CbColor3ViewGfx11));
   set_context_reg_seq_array(cs, R_028D48_CB_COLOR3_DCC_BASE, SET(CbColor3DccBaseGfx11));
   set_context_reg_seq_array(cs, R_028D5C_CB_COLOR4_VIEW, SET(CbColor4ViewGfx11));
   set_context_reg_seq_array(cs, R_028D84_CB_COLOR4_DCC_BASE, SET(CbColor4DccBaseGfx11));
   set_context_reg_seq_array(cs, R_028D98_CB_COLOR5_VIEW, SET(CbColor5ViewGfx11));
   set_context_reg_seq_array(cs, R_028DC0_CB_COLOR5_DCC_BASE, SET(CbColor5DccBaseGfx11));
   set_context_reg_seq_array(cs, R_028DD4_CB_COLOR6_VIEW, SET(CbColor6ViewGfx11));
   set_context_reg_seq_array(cs, R_028DFC_CB_COLOR6_DCC_BASE, SET(CbColor6DccBaseGfx11));
   set_context_reg_seq_array(cs, R_028E10_CB_COLOR7_VIEW, SET(CbColor7ViewGfx11));
   set_context_reg_seq_array(cs, R_028E38_CB_COLOR7_DCC_BASE, SET(CbColor7DccBaseGfx11));
   set_context_reg_seq_array(cs, R_028C98_CB_COLOR0_DCC_BASE_EXT, SET(CbColor0DccBaseExtGfx11));

   for (unsigned i = 0; i < num_reg_pairs; i++)
      set_context_reg_seq_array(cs, reg_offsets[i], 1, &reg_values[i]);
}

void ac_emulate_clear_state(const struct radeon_info *info, struct radeon_cmdbuf *cs,
                            set_context_reg_seq_array_fn set_context_reg_seq_array)
{
   /* Set context registers same as CLEAR_STATE to initialize shadow memory. */
   unsigned reg_offset = R_02835C_PA_SC_TILE_STEERING_OVERRIDE;
   uint32_t reg_value = info->pa_sc_tile_steering_override;

   if (info->gfx_level >= GFX11) {
      gfx11_emulate_clear_state(cs, 1, &reg_offset, &reg_value, set_context_reg_seq_array);
   } else if (info->gfx_level == GFX10_3) {
      gfx103_emulate_clear_state(cs, 1, &reg_offset, &reg_value, set_context_reg_seq_array);
   } else if (info->gfx_level == GFX10) {
      gfx10_emulate_clear_state(cs, 1, &reg_offset, &reg_value, set_context_reg_seq_array);
   } else if (info->gfx_level == GFX9) {
      gfx9_emulate_clear_state(cs, set_context_reg_seq_array);
   } else {
      unreachable("unimplemented");
   }
}

static void ac_print_nonshadowed_reg(enum amd_gfx_level gfx_level, enum radeon_family family,
                                     unsigned reg_offset)
{
   bool found = false;

   for (unsigned type = 0; type < SI_NUM_REG_RANGES && !found; type++) {
      const struct ac_reg_range *ranges;
      unsigned num_ranges;

      ac_get_reg_ranges(gfx_level, family, type, &num_ranges, &ranges);

      for (unsigned i = 0; i < num_ranges; i++) {
         if (reg_offset >= ranges[i].offset && reg_offset < ranges[i].offset + ranges[i].size) {
            /* Assertion: A register can be listed only once in the shadowed tables. */
            if (found) {
               printf("warning: register R_%06X_%s found multiple times in tables\n",
                      reg_offset, ac_get_register_name(gfx_level, family, reg_offset));
            }
            found = true;
         }
      }
   }

   if (!found) {
      printf("register R_%06X_%s not found in any tables\n", reg_offset,
             ac_get_register_name(gfx_level, family, reg_offset));
   }
}

void ac_print_nonshadowed_regs(enum amd_gfx_level gfx_level, enum radeon_family family)
{
   if (!debug_get_bool_option("AMD_PRINT_SHADOW_REGS", false))
      return;

   for (unsigned i = 0xB000; i < 0xBFFF; i += 4) {
      if (ac_register_exists(gfx_level, family, i))
         ac_print_nonshadowed_reg(gfx_level, family, i);
   }

   for (unsigned i = 0x28000; i < 0x28FFF; i += 4) {
      if (ac_register_exists(gfx_level, family, i))
         ac_print_nonshadowed_reg(gfx_level, family, i);
   }

   for (unsigned i = 0x30000; i < 0x31FFF; i += 4) {
      if (ac_register_exists(gfx_level, family, i))
         ac_print_nonshadowed_reg(gfx_level, family, i);
   }
}

static void ac_build_load_reg(const struct radeon_info *info,
                              pm4_cmd_add_fn pm4_cmd_add, void *pm4_cmdbuf,
                              enum ac_reg_range_type type,
                              uint64_t gpu_address)
{
   unsigned packet, num_ranges, offset;
   const struct ac_reg_range *ranges;

   ac_get_reg_ranges(info->gfx_level, info->family,
                     type, &num_ranges, &ranges);

   switch (type) {
   case SI_REG_RANGE_UCONFIG:
      gpu_address += SI_SHADOWED_UCONFIG_REG_OFFSET;
      offset = CIK_UCONFIG_REG_OFFSET;
      packet = PKT3_LOAD_UCONFIG_REG;
      break;
   case SI_REG_RANGE_CONTEXT:
      gpu_address += SI_SHADOWED_CONTEXT_REG_OFFSET;
      offset = SI_CONTEXT_REG_OFFSET;
      packet = PKT3_LOAD_CONTEXT_REG;
      break;
   default:
      gpu_address += SI_SHADOWED_SH_REG_OFFSET;
      offset = SI_SH_REG_OFFSET;
      packet = PKT3_LOAD_SH_REG;
      break;
   }

   pm4_cmd_add(pm4_cmdbuf, PKT3(packet, 1 + num_ranges * 2, 0));
   pm4_cmd_add(pm4_cmdbuf, gpu_address);
   pm4_cmd_add(pm4_cmdbuf, gpu_address >> 32);
   for (unsigned i = 0; i < num_ranges; i++) {
      pm4_cmd_add(pm4_cmdbuf, (ranges[i].offset - offset) / 4);
      pm4_cmd_add(pm4_cmdbuf, ranges[i].size / 4);
   }
}

void ac_create_shadowing_ib_preamble(const struct radeon_info *info,
                                     pm4_cmd_add_fn pm4_cmd_add, void *pm4_cmdbuf,
                                     uint64_t gpu_address,
                                     bool dpbb_allowed)
{
   if (dpbb_allowed) {
      pm4_cmd_add(pm4_cmdbuf, PKT3(PKT3_EVENT_WRITE, 0, 0));
      pm4_cmd_add(pm4_cmdbuf, EVENT_TYPE(V_028A90_BREAK_BATCH) | EVENT_INDEX(0));
   }

   /* Wait for idle, because we'll update VGT ring pointers. */
   pm4_cmd_add(pm4_cmdbuf, PKT3(PKT3_EVENT_WRITE, 0, 0));
   pm4_cmd_add(pm4_cmdbuf, EVENT_TYPE(V_028A90_VS_PARTIAL_FLUSH) | EVENT_INDEX(4));

   /* VGT_FLUSH is required even if VGT is idle. It resets VGT pointers. */
   pm4_cmd_add(pm4_cmdbuf, PKT3(PKT3_EVENT_WRITE, 0, 0));
   pm4_cmd_add(pm4_cmdbuf, EVENT_TYPE(V_028A90_VGT_FLUSH) | EVENT_INDEX(0));

   if (info->gfx_level >= GFX11) {
      uint64_t rb_mask = BITFIELD64_MASK(info->max_render_backends);

      pm4_cmd_add(pm4_cmdbuf, PKT3(PKT3_EVENT_WRITE, 2, 0));
      pm4_cmd_add(pm4_cmdbuf, EVENT_TYPE(V_028A90_PIXEL_PIPE_STAT_CONTROL) | EVENT_INDEX(1));
      pm4_cmd_add(pm4_cmdbuf, PIXEL_PIPE_STATE_CNTL_COUNTER_ID(0) |
                              PIXEL_PIPE_STATE_CNTL_STRIDE(2) |
                              PIXEL_PIPE_STATE_CNTL_INSTANCE_EN_LO(rb_mask));
      pm4_cmd_add(pm4_cmdbuf, PIXEL_PIPE_STATE_CNTL_INSTANCE_EN_HI(rb_mask));

      /* We must wait for idle using an EOP event before changing the attribute ring registers.
       * Use the bottom-of-pipe EOP event, but increment the PWS counter instead of writing memory.
       */
      pm4_cmd_add(pm4_cmdbuf, PKT3(PKT3_RELEASE_MEM, 6, 0));
      pm4_cmd_add(pm4_cmdbuf, S_490_EVENT_TYPE(V_028A90_BOTTOM_OF_PIPE_TS) |
                              S_490_EVENT_INDEX(5) |
                              S_490_PWS_ENABLE(1));
      pm4_cmd_add(pm4_cmdbuf, 0); /* DST_SEL, INT_SEL, DATA_SEL */
      pm4_cmd_add(pm4_cmdbuf, 0); /* ADDRESS_LO */
      pm4_cmd_add(pm4_cmdbuf, 0); /* ADDRESS_HI */
      pm4_cmd_add(pm4_cmdbuf, 0); /* DATA_LO */
      pm4_cmd_add(pm4_cmdbuf, 0); /* DATA_HI */
      pm4_cmd_add(pm4_cmdbuf, 0); /* INT_CTXID */

      unsigned gcr_cntl = S_586_GL2_INV(1) | S_586_GL2_WB(1) |
                          S_586_GLM_INV(1) | S_586_GLM_WB(1) |
                          S_586_GL1_INV(1) | S_586_GLV_INV(1) |
                          S_586_GLK_INV(1) | S_586_GLI_INV(V_586_GLI_ALL);

      /* Wait for the PWS counter. */
      pm4_cmd_add(pm4_cmdbuf, PKT3(PKT3_ACQUIRE_MEM, 6, 0));
      pm4_cmd_add(pm4_cmdbuf, S_580_PWS_STAGE_SEL(V_580_CP_PFP) |
                              S_580_PWS_COUNTER_SEL(V_580_TS_SELECT) |
                              S_580_PWS_ENA2(1) |
                              S_580_PWS_COUNT(0));
      pm4_cmd_add(pm4_cmdbuf, 0xffffffff); /* GCR_SIZE */
      pm4_cmd_add(pm4_cmdbuf, 0x01ffffff); /* GCR_SIZE_HI */
      pm4_cmd_add(pm4_cmdbuf, 0); /* GCR_BASE_LO */
      pm4_cmd_add(pm4_cmdbuf, 0); /* GCR_BASE_HI */
      pm4_cmd_add(pm4_cmdbuf, S_585_PWS_ENA(1));
      pm4_cmd_add(pm4_cmdbuf, gcr_cntl); /* GCR_CNTL */
   } else if (info->gfx_level >= GFX10) {
      unsigned gcr_cntl = S_586_GL2_INV(1) | S_586_GL2_WB(1) |
                          S_586_GLM_INV(1) | S_586_GLM_WB(1) |
                          S_586_GL1_INV(1) | S_586_GLV_INV(1) |
                          S_586_GLK_INV(1) | S_586_GLI_INV(V_586_GLI_ALL);

      pm4_cmd_add(pm4_cmdbuf, PKT3(PKT3_ACQUIRE_MEM, 6, 0));
      pm4_cmd_add(pm4_cmdbuf, 0);           /* CP_COHER_CNTL */
      pm4_cmd_add(pm4_cmdbuf, 0xffffffff);  /* CP_COHER_SIZE */
      pm4_cmd_add(pm4_cmdbuf, 0xffffff);    /* CP_COHER_SIZE_HI */
      pm4_cmd_add(pm4_cmdbuf, 0);           /* CP_COHER_BASE */
      pm4_cmd_add(pm4_cmdbuf, 0);           /* CP_COHER_BASE_HI */
      pm4_cmd_add(pm4_cmdbuf, 0x0000000A);  /* POLL_INTERVAL */
      pm4_cmd_add(pm4_cmdbuf, gcr_cntl);    /* GCR_CNTL */

      pm4_cmd_add(pm4_cmdbuf, PKT3(PKT3_PFP_SYNC_ME, 0, 0));
      pm4_cmd_add(pm4_cmdbuf, 0);
   } else if (info->gfx_level == GFX9) {
      unsigned cp_coher_cntl = S_0301F0_SH_ICACHE_ACTION_ENA(1) |
                               S_0301F0_SH_KCACHE_ACTION_ENA(1) |
                               S_0301F0_TC_ACTION_ENA(1) |
                               S_0301F0_TCL1_ACTION_ENA(1) |
                               S_0301F0_TC_WB_ACTION_ENA(1);

      pm4_cmd_add(pm4_cmdbuf, PKT3(PKT3_ACQUIRE_MEM, 5, 0));
      pm4_cmd_add(pm4_cmdbuf, cp_coher_cntl); /* CP_COHER_CNTL */
      pm4_cmd_add(pm4_cmdbuf, 0xffffffff);    /* CP_COHER_SIZE */
      pm4_cmd_add(pm4_cmdbuf, 0xffffff);      /* CP_COHER_SIZE_HI */
      pm4_cmd_add(pm4_cmdbuf, 0);             /* CP_COHER_BASE */
      pm4_cmd_add(pm4_cmdbuf, 0);             /* CP_COHER_BASE_HI */
      pm4_cmd_add(pm4_cmdbuf, 0x0000000A);    /* POLL_INTERVAL */

      pm4_cmd_add(pm4_cmdbuf, PKT3(PKT3_PFP_SYNC_ME, 0, 0));
      pm4_cmd_add(pm4_cmdbuf, 0);
   } else {
      unreachable("invalid chip");
   }

   pm4_cmd_add(pm4_cmdbuf, PKT3(PKT3_CONTEXT_CONTROL, 1, 0));
   pm4_cmd_add(pm4_cmdbuf,
               CC0_UPDATE_LOAD_ENABLES(1) |
               CC0_LOAD_PER_CONTEXT_STATE(1) |
               CC0_LOAD_CS_SH_REGS(1) |
               CC0_LOAD_GFX_SH_REGS(1) |
               CC0_LOAD_GLOBAL_UCONFIG(1));
   pm4_cmd_add(pm4_cmdbuf,
               CC1_UPDATE_SHADOW_ENABLES(1) |
               CC1_SHADOW_PER_CONTEXT_STATE(1) |
               CC1_SHADOW_CS_SH_REGS(1) |
               CC1_SHADOW_GFX_SH_REGS(1) |
               CC1_SHADOW_GLOBAL_UCONFIG(1) |
               CC1_SHADOW_GLOBAL_CONFIG(1));

   if (!info->has_fw_based_shadowing) {
      for (unsigned i = 0; i < SI_NUM_REG_RANGES; i++)
         ac_build_load_reg(info, pm4_cmd_add, pm4_cmdbuf, i, gpu_address);
   }
}
