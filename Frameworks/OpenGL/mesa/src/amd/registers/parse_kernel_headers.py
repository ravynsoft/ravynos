#!/usr/bin/env python3

import sys, io, re, json
from canonicalize import json_canonicalize

######### BEGIN HARDCODED CONFIGURATION

gfx_levels = {
    'gfx6': [
        [],
        'asic_reg/gca/gfx_6_0_d.h',
        'asic_reg/gca/gfx_6_0_sh_mask.h',
        'asic_reg/gca/gfx_7_2_enum.h' # the file for gfx6 doesn't exist
    ],
    'gfx7': [
        [],
        'asic_reg/gca/gfx_7_2_d.h',
        'asic_reg/gca/gfx_7_2_sh_mask.h',
        'asic_reg/gca/gfx_7_2_enum.h'
    ],
    'gfx8': [
        [],
        'asic_reg/gca/gfx_8_0_d.h',
        'asic_reg/gca/gfx_8_0_sh_mask.h',
        'asic_reg/gca/gfx_8_0_enum.h',
    ],
    'gfx81': [
        [],
        'asic_reg/gca/gfx_8_1_d.h',
        'asic_reg/gca/gfx_8_1_sh_mask.h',
        'asic_reg/gca/gfx_8_1_enum.h',
    ],
    'gfx9': [
        [0x00002000, 0x0000A000, 0, 0, 0], # IP_BASE GC_BASE
        'asic_reg/gc/gc_9_2_1_offset.h',
        'asic_reg/gc/gc_9_2_1_sh_mask.h',
        'vega10_enum.h',
    ],
    'gfx940': [
        [0x00002000, 0x0000A000, 0, 0, 0], # IP_BASE GC_BASE
        'asic_reg/gc/gc_9_4_3_offset.h',
        'asic_reg/gc/gc_9_4_3_sh_mask.h',
        'vega10_enum.h',
    ],
    'gfx10': [
        [0x00001260, 0x0000A000, 0x02402C00, 0, 0], # IP_BASE GC_BASE
        'asic_reg/gc/gc_10_1_0_offset.h',
        'asic_reg/gc/gc_10_1_0_sh_mask.h',
        'navi10_enum.h',
    ],
    'gfx103': [
        [0x00001260, 0x0000A000, 0x0001C000, 0x02402C00, 0], # IP_BASE GC_BASE
        'asic_reg/gc/gc_10_3_0_offset.h',
        'asic_reg/gc/gc_10_3_0_sh_mask.h',
        'navi10_enum.h', # the file for gfx10.3 doesn't exist
    ],
    'gfx11': [
        [0x00001260, 0x0000A000, 0x0001C000, 0x02402C00, 0, 0], # IP_BASE GC_BASE
        'asic_reg/gc/gc_11_0_0_offset.h',
        'asic_reg/gc/gc_11_0_0_sh_mask.h',
        'soc21_enum.h',
    ],
    'gfx115': [
        [0x00001260, 0x0000A000, 0x0001C000, 0x02402C00, 0, 0], # IP_BASE GC_BASE
        'asic_reg/gc/gc_11_5_0_offset.h',
        'asic_reg/gc/gc_11_5_0_sh_mask.h',
        'soc21_enum.h',
    ],
}

# match: #define mmSDMA0_DEC_START                              0x0000
# match: #define ixSDMA0_DEC_START                              0x0000
# match: #define regSDMA0_DEC_START                              0x0000
re_offset = re.compile(r'^#define (?P<mm>(mm|ix|reg))(?P<name>\w+)\s+(?P<value>\w+)\n')

# match: #define SDMA0_DEC_START__START__SHIFT                  0x0
re_shift = re.compile(r'^#define (?P<name>\w+)__(?P<field>\w+)__SHIFT\s+(?P<value>\w+)\n')

# match: #define SDMA0_DEC_START__START_MASK                    0xFFFFFFFFL
# match: #define SDMA0_DEC_START__START_MASK                    0xFFFFFFFF
re_mask = re.compile(r'^#define (?P<name>\w+)__(?P<field>\w+)_MASK\s+(?P<value>[0-9a-fA-Fx]+)L?\n')

def register_filter(gfx_level, name, offset, already_added):
    group = offset // 0x1000
    is_cdna = gfx_level in ['gfx940']

    # Shader and uconfig registers
    umd_ranges = [0xB, 0x30]

    # Gfx context, other uconfig, and perf counter registers
    if not is_cdna:
        umd_ranges += [0x28, 0x31, 0x34, 0x35, 0x36, 0x37]

    # Add all registers in the 0x8000 range for gfx6
    if gfx_level == 'gfx6':
        umd_ranges += [0x8]

    # Only accept writeable registers and debug registers
    return ((group in umd_ranges or
             # Add SQ_WAVE registers for trap handlers
             name.startswith('SQ_WAVE_') or
             # Add registers in the 0x8000 range used by all generations
             (group == 0x8 and
              (name.startswith('SQ_IMG_') or
               name.startswith('SQ_BUF_') or
               name.startswith('SQ_THREAD') or
               name.startswith('GRBM_STATUS') or
               name.startswith('CP_CP'))) or
             name.startswith('GCVM_L2_PROTECTION_FAULT_STATUS') or
             # Add registers in the 0x9000 range
             (group == 0x9 and
              (name in ['TA_CS_BC_BASE_ADDR', 'GB_ADDR_CONFIG', 'SPI_CONFIG_CNTL'] or
               (name.startswith('GB') and 'TILE_MODE' in name)))) and
            # Remove SQ compiler definitions
            offset // 4 not in (0x23B0, 0x23B1, 0x237F) and
            # Remove conflicts (multiple definitions for the same offset)
            not already_added and
            'PREF_PRI_ACCUM' not in name and
            # only define SPI and COMPUTE registers in the 0xB000 range.
            (group != 0xB or name.startswith('SPI') or name.startswith('COMPUTE')) and
            # only define CP_COHER uconfig registers on CDNA
            (not is_cdna or group != 0x30 or name.startswith('CP_COHER')))

# Mapping from field names to enum types
enum_map = {
    # Format:
    #    field: [type1]                          - all registers use the same enum
    # OR:
    #    field: [type1, reg1, type2, reg2, ...]  - apply different enums to different registers
    "ALPHA_COMB_FCN": ["CombFunc", "CB_BLEND0_CONTROL", "SX_OPT_COMB_FCN", "SX_MRT0_BLEND_OPT"],
    "ALPHA_DESTBLEND": ["BlendOp"],
    "ALPHA_DST_OPT": ["SX_BLEND_OPT"],
    "ALPHA_SRCBLEND": ["BlendOp"],
    "ALPHA_SRC_OPT": ["SX_BLEND_OPT"],
    "ARRAY_MODE": ["ArrayMode"],
    "BANK_HEIGHT": ["BankHeight"],
    "BANK_WIDTH": ["BankWidth"],
    "BC_SWIZZLE": ["SQ_IMG_RSRC_WORD4__BC_SWIZZLE"],
    "BIN_MAPPING_MODE": ["BinMapMode"],
    "BINNING_MODE": ["BinningMode"],
    "BIN_SIZE_X_EXTEND": ["BinSizeExtend"],
    "BIN_SIZE_Y_EXTEND": ["BinSizeExtend"],
    "BLEND_OPT_DISCARD_PIXEL": ["BlendOpt"],
    "BLEND_OPT_DONT_RD_DST": ["BlendOpt"],
    "BORDER_COLOR_TYPE": ["SQ_TEX_BORDER_COLOR"],
    "BUF_TYPE": ["VGT_DMA_BUF_TYPE"],
    "CLAMP_X": ["SQ_TEX_CLAMP"],
    "CLAMP_Y": ["SQ_TEX_CLAMP"],
    "CLAMP_Z": ["SQ_TEX_CLAMP"],
    "CLEAR_FILTER_SEL": ["CBPerfClearFilterSel"],
    "CLIP_RULE": ["CLIP_RULE"],
    "CMASK_ADDR_TYPE": ["CmaskAddr"],
    "CMASK_RD_POLICY": ["ReadPolicy"],
    "CMASK_WR_POLICY": ["WritePolicy"],
    "COL0_EXPORT_FORMAT": ["SPI_SHADER_EX_FORMAT"],
    "COL1_EXPORT_FORMAT": ["SPI_SHADER_EX_FORMAT"],
    "COL2_EXPORT_FORMAT": ["SPI_SHADER_EX_FORMAT"],
    "COL3_EXPORT_FORMAT": ["SPI_SHADER_EX_FORMAT"],
    "COL4_EXPORT_FORMAT": ["SPI_SHADER_EX_FORMAT"],
    "COL5_EXPORT_FORMAT": ["SPI_SHADER_EX_FORMAT"],
    "COL6_EXPORT_FORMAT": ["SPI_SHADER_EX_FORMAT"],
    "COL7_EXPORT_FORMAT": ["SPI_SHADER_EX_FORMAT"],
    "COLOR_COMB_FCN": ["CombFunc", "CB_BLEND0_CONTROL", "SX_OPT_COMB_FCN", "SX_MRT0_BLEND_OPT"],
    "COLOR_DESTBLEND": ["BlendOp"],
    "COLOR_DST_OPT": ["SX_BLEND_OPT"],
    "COLOR_RD_POLICY": ["ReadPolicy"],
    "COLOR_SRCBLEND": ["BlendOp"],
    "COLOR_SRC_OPT": ["SX_BLEND_OPT"],
    "COLOR_WR_POLICY": ["WritePolicy"],
    "COMPAREFUNC0": ["CompareFrag"],
    "COMPAREFUNC1": ["CompareFrag"],
    "COMP_SWAP": ["SurfaceSwap"],
    "CONSERVATIVE_Z_EXPORT": ["ConservativeZExport"],
    "COVERAGE_TO_SHADER_SELECT": ["CovToShaderSel"],
    "CUT_MODE": ["VGT_GS_CUT_MODE"],
    "DATA_FORMAT": ["BUF_DATA_FORMAT", "SQ_BUF_RSRC_WORD3", "IMG_DATA_FORMAT", "SQ_IMG_RSRC_WORD1"],
    "DCC_RD_POLICY": ["ReadPolicy"],
    "DCC_WR_POLICY": ["WritePolicy"],
    "DEPTH_COMPARE_FUNC": ["SQ_TEX_DEPTH_COMPARE"],
    "DETECT_ONE": ["VGT_DETECT_ONE"],
    "DETECT_ZERO": ["VGT_DETECT_ZERO"],
    "DISTRIBUTION_MODE": ["VGT_DIST_MODE"],
    "DST_SEL_W": ["SQ_SEL_XYZW01"],
    "DST_SEL_X": ["SQ_SEL_XYZW01"],
    "DST_SEL_Y": ["SQ_SEL_XYZW01"],
    "DST_SEL_Z": ["SQ_SEL_XYZW01"],
    "ENDIAN": ["SurfaceEndian"],
    "ES_EN": ["VGT_STAGES_ES_EN"],
    "EVENT_TYPE": ["VGT_EVENT_TYPE"],
    "EXCP": ["EXCP_EN"],
    "EXCP_EN": ["EXCP_EN"],
    "FAULT_BEHAVIOR": ["DbPRTFaultBehavior"],
    "FILTER_MODE": ["SQ_IMG_FILTER_TYPE"],
    "FLOAT_MODE": ["FLOAT_MODE"],
    "FMASK_RD_POLICY": ["ReadPolicy"],
    "FMASK_WR_POLICY": ["WritePolicy"],
    "FORCE_FULL_Z_RANGE": ["ForceControl"],
    "FORCE_HIS_ENABLE0": ["ForceControl"],
    "FORCE_HIS_ENABLE1": ["ForceControl"],
    "FORCE_HIZ_ENABLE": ["ForceControl"],
    "FORCE_Z_LIMIT_SUMM": ["ZLimitSumm"],
    "FORMAT": ["ColorFormat", "CB_COLOR0_INFO", "StencilFormat", "DB_STENCIL_INFO", "ZFormat", "DB_Z_INFO"],
    "GS_EN": ["VGT_STAGES_GS_EN"],
    "HIZ_ZFUNC": ["CompareFrag"],
    "HS_EN": ["VGT_STAGES_HS_EN"],
    "HTILE_RD_POLICY": ["ReadPolicy"],
    "HTILE_WR_POLICY": ["WritePolicy"],
    "IDX0_EXPORT_FORMAT": ["SPI_SHADER_FORMAT"],
    "INDEX_TYPE": ["VGT_INDEX_TYPE_MODE"],
    "LS_EN": ["VGT_STAGES_LS_EN"],
    "MACRO_TILE_ASPECT": ["MacroTileAspect"],
    "MAJOR_MODE": ["VGT_DI_MAJOR_MODE_SELECT"],
    "MAX_UNCOMPRESSED_BLOCK_SIZE": ["CB_COLOR_DCC_CONTROL__MAX_UNCOMPRESSED_BLOCK_SIZE"],
    "MICRO_TILE_MODE": ["GB_TILE_MODE0__MICRO_TILE_MODE"],
    "MICRO_TILE_MODE_NEW": ["MicroTileMode"],
    "MIN_COMPRESSED_BLOCK_SIZE": ["CB_COLOR_DCC_CONTROL__MIN_COMPRESSED_BLOCK_SIZE"],
    "MIP_FILTER": ["SQ_TEX_MIP_FILTER"],
    "MODE": ["CBMode", "CB_COLOR_CONTROL", "VGT_GS_MODE_TYPE", "VGT_GS_MODE"],
    "MRT0_EPSILON": ["SX_BLEND_OPT_EPSILON__MRT0_EPSILON"],
    "MRT0": ["SX_DOWNCONVERT_FORMAT"],
    "MRT1": ["SX_DOWNCONVERT_FORMAT"],
    "MRT2": ["SX_DOWNCONVERT_FORMAT"],
    "MRT3": ["SX_DOWNCONVERT_FORMAT"],
    "MRT4": ["SX_DOWNCONVERT_FORMAT"],
    "MRT5": ["SX_DOWNCONVERT_FORMAT"],
    "MRT6": ["SX_DOWNCONVERT_FORMAT"],
    "MRT7": ["SX_DOWNCONVERT_FORMAT"],
    "NUM_BANKS": ["NumBanks"],
    "NUM_FORMAT": ["BUF_NUM_FORMAT", "SQ_BUF_RSRC_WORD3", "IMG_NUM_FORMAT", "SQ_IMG_RSRC_WORD1"],
    "NUMBER_TYPE": ["SurfaceNumber"],
    "OFFCHIP_GRANULARITY": ["VGT_HS_OFFCHIP_PARAM__OFFCHIP_GRANULARITY"],
    "OP_FILTER_SEL": ["CBPerfOpFilterSel"],
    "OREO_MODE": ["OreoMode"],
    "OUTPRIM_TYPE_1": ["VGT_GS_OUTPRIM_TYPE"],
    "OUTPRIM_TYPE_2": ["VGT_GS_OUTPRIM_TYPE"],
    "OUTPRIM_TYPE_3": ["VGT_GS_OUTPRIM_TYPE"],
    "OUTPRIM_TYPE": ["VGT_GS_OUTPRIM_TYPE"],
    "PARTIAL_SQUAD_LAUNCH_CONTROL": ["DbPSLControl"],
    "PARTITIONING": ["VGT_TESS_PARTITION"],
    "PERFMON_ENABLE_MODE": ["CP_PERFMON_ENABLE_MODE"],
    "PERFMON_STATE": ["CP_PERFMON_STATE"],
    "PIPE_CONFIG": ["PipeConfig"],
    "PKR_MAP": ["PkrMap"],
    "PKR_XSEL2": ["PkrXsel2"],
    "PKR_XSEL": ["PkrXsel"],
    "PKR_YSEL": ["PkrYsel"],
    "PNT_SPRITE_OVRD_W": ["SPI_PNT_SPRITE_OVERRIDE"],
    "PNT_SPRITE_OVRD_X": ["SPI_PNT_SPRITE_OVERRIDE"],
    "PNT_SPRITE_OVRD_Y": ["SPI_PNT_SPRITE_OVERRIDE"],
    "PNT_SPRITE_OVRD_Z": ["SPI_PNT_SPRITE_OVERRIDE"],
    "POLYMODE_BACK_PTYPE": ["PA_SU_SC_MODE_CNTL__POLYMODE_FRONT_PTYPE"],
    "POLYMODE_FRONT_PTYPE": ["PA_SU_SC_MODE_CNTL__POLYMODE_FRONT_PTYPE"],
    "POLY_MODE": ["PA_SU_SC_MODE_CNTL__POLY_MODE"],
    "POS0_EXPORT_FORMAT": ["SPI_SHADER_FORMAT"],
    "POS1_EXPORT_FORMAT": ["SPI_SHADER_FORMAT"],
    "POS2_EXPORT_FORMAT": ["SPI_SHADER_FORMAT"],
    "POS3_EXPORT_FORMAT": ["SPI_SHADER_FORMAT"],
    "POS4_EXPORT_FORMAT": ["SPI_SHADER_FORMAT"],
    "PRIM_TYPE": ["VGT_DI_PRIM_TYPE"],
    "PUNCHOUT_MODE": ["DB_DFSM_CONTROL__PUNCHOUT_MODE"],
    "QUANT_MODE": ["QUANT_MODE"],
    "RB_MAP_PKR0": ["RbMap"],
    "RB_MAP_PKR1": ["RbMap"],
    "RB_XSEL2": ["RbXsel2"],
    "RB_XSEL": ["RbXsel"],
    "RB_YSEL": ["RbYsel"],
    "ROP3": ["ROP3"],
    "RDREQ_POLICY": ["VGT_RDREQ_POLICY"],
    "REG_INCLUDE": ["ThreadTraceRegInclude"],
    "ROUND_MODE": ["PA_SU_VTX_CNTL__ROUND_MODE", "PA_SU_VTX_CNTL"],
    "SC_MAP": ["ScMap"],
    "SC_XSEL": ["ScXsel"],
    "SC_YSEL": ["ScYsel"],
    "SE_MAP": ["SeMap"],
    "SE_PAIR_MAP": ["SePairMap"],
    "SE_PAIR_XSEL": ["SePairXsel"],
    "SE_PAIR_YSEL": ["SePairYsel"],
    "SE_XSEL": ["SeXsel"],
    "SE_YSEL": ["SeYsel"],
    "SOURCE_SELECT": ["VGT_DI_SOURCE_SELECT"],
    "SPM_PERFMON_STATE": ["SPM_PERFMON_STATE"],
    "S_RD_POLICY": ["ReadPolicy"],
    "STENCILFAIL_BF": ["StencilOp"],
    "STENCILFAIL": ["StencilOp"],
    "STENCILFUNC_BF": ["CompareFrag"],
    "STENCILFUNC": ["CompareFrag"],
    "STENCILZFAIL_BF": ["StencilOp"],
    "STENCILZFAIL": ["StencilOp"],
    "STENCILZPASS_BF": ["StencilOp"],
    "STENCILZPASS": ["StencilOp"],
    "SWAP_MODE": ["VGT_DMA_SWAP_MODE"],
    "S_WR_POLICY": ["WritePolicy"],
    "TILE_SPLIT": ["TileSplit"],
    "TOKEN_EXCLUDE": ["ThreadTraceTokenExclude"],
    "TOPOLOGY": ["VGT_TESS_TOPOLOGY"],
    "TYPE": ["SQ_RSRC_BUF_TYPE", "SQ_BUF_RSRC_WORD3", "SQ_RSRC_IMG_TYPE", "SQ_IMG_RSRC_WORD3", "VGT_TESS_TYPE", "VGT_TF_PARAM"],
    "UNCERTAINTY_REGION_MODE": ["ScUncertaintyRegionMode"],
    "VRS_HTILE_ENCODING": ["VRSHtileEncoding"],
    "VRS_RATE": ["VRSrate"],
    "VS_EN": ["VGT_STAGES_VS_EN"],
    "XY_MAG_FILTER": ["SQ_TEX_XY_FILTER"],
    "XY_MIN_FILTER": ["SQ_TEX_XY_FILTER"],
    "Z_EXPORT_FORMAT": ["SPI_SHADER_EX_FORMAT"],
    "Z_FILTER": ["SQ_TEX_Z_FILTER"],
    "ZFUNC": ["CompareFrag"],
    "Z_ORDER": ["ZOrder"],
    "ZPCPSD_WR_POLICY": ["WritePolicy"],
    "Z_RD_POLICY": ["ReadPolicy"],
    "Z_WR_POLICY": ["WritePolicy"],

    "VERTEX_RATE_COMBINER_MODE": ["VRSCombinerModeSC"],
    "PRIMITIVE_RATE_COMBINER_MODE": ["VRSCombinerModeSC"],
    "HTILE_RATE_COMBINER_MODE": ["VRSCombinerModeSC"],
    "SAMPLE_ITER_COMBINER_MODE": ["VRSCombinerModeSC"],
    "VRS_OVERRIDE_RATE_COMBINER_MODE": ["VRSCombinerModeSC"],
}

# Enum definitions that are incomplete or missing in kernel headers
DB_DFSM_CONTROL__PUNCHOUT_MODE = {
 "entries": [
  {"name": "AUTO", "value": 0},
  {"name": "FORCE_ON", "value": 1},
  {"name": "FORCE_OFF", "value": 2},
  {"name": "RESERVED", "value": 3}
 ]
}

ColorFormat = {
 "entries": [
  {"name": "COLOR_INVALID", "value": 0},
  {"name": "COLOR_8", "value": 1},
  {"name": "COLOR_16", "value": 2},
  {"name": "COLOR_8_8", "value": 3},
  {"name": "COLOR_32", "value": 4},
  {"name": "COLOR_16_16", "value": 5},
  {"name": "COLOR_10_11_11", "value": 6},
  {"name": "COLOR_11_11_10", "value": 7},
  {"name": "COLOR_10_10_10_2", "value": 8},
  {"name": "COLOR_2_10_10_10", "value": 9},
  {"name": "COLOR_8_8_8_8", "value": 10},
  {"name": "COLOR_32_32", "value": 11},
  {"name": "COLOR_16_16_16_16", "value": 12},
  {"name": "COLOR_32_32_32_32", "value": 14},
  {"name": "COLOR_5_6_5", "value": 16},
  {"name": "COLOR_1_5_5_5", "value": 17},
  {"name": "COLOR_5_5_5_1", "value": 18},
  {"name": "COLOR_4_4_4_4", "value": 19},
  {"name": "COLOR_8_24", "value": 20},
  {"name": "COLOR_24_8", "value": 21},
  {"name": "COLOR_X24_8_32_FLOAT", "value": 22},
  {"name": "COLOR_5_9_9_9", "value": 24}
 ]
}

SQ_IMG_RSRC_WORD4__BC_SWIZZLE = {
 "entries": [
  {"name": "BC_SWIZZLE_XYZW", "value": 0},
  {"name": "BC_SWIZZLE_XWYZ", "value": 1},
  {"name": "BC_SWIZZLE_WZYX", "value": 2},
  {"name": "BC_SWIZZLE_WXYZ", "value": 3},
  {"name": "BC_SWIZZLE_ZYXW", "value": 4},
  {"name": "BC_SWIZZLE_YXWZ", "value": 5}
 ]
}

SX_DOWNCONVERT_FORMAT = {
 "entries": [
  {"name": "SX_RT_EXPORT_NO_CONVERSION", "value": 0},
  {"name": "SX_RT_EXPORT_32_R", "value": 1},
  {"name": "SX_RT_EXPORT_32_A", "value": 2},
  {"name": "SX_RT_EXPORT_10_11_11", "value": 3},
  {"name": "SX_RT_EXPORT_2_10_10_10", "value": 4},
  {"name": "SX_RT_EXPORT_8_8_8_8", "value": 5},
  {"name": "SX_RT_EXPORT_5_6_5", "value": 6},
  {"name": "SX_RT_EXPORT_1_5_5_5", "value": 7},
  {"name": "SX_RT_EXPORT_4_4_4_4", "value": 8},
  {"name": "SX_RT_EXPORT_16_16_GR", "value": 9},
  {"name": "SX_RT_EXPORT_16_16_AR", "value": 10},
  {"name": "SX_RT_EXPORT_9_9_9_E5", "value": 11}
 ]
}

ThreadTraceRegInclude = {
  "entries": [
   {"name": "REG_INCLUDE_SQDEC", "value": 1},
   {"name": "REG_INCLUDE_SHDEC", "value": 2},
   {"name": "REG_INCLUDE_GFXUDEC", "value": 4},
   {"name": "REG_INCLUDE_COMP", "value": 8},
   {"name": "REG_INCLUDE_CONTEXT", "value": 16},
   {"name": "REG_INCLUDE_CONFIG", "value": 32},
   {"name": "REG_INCLUDE_OTHER", "value": 64},
   {"name": "REG_INCLUDE_READS", "value": 128}
  ]
}

ThreadTraceTokenExclude = {
  "entries": [
   {"name": "TOKEN_EXCLUDE_VMEMEXEC", "value": 1},
   {"name": "TOKEN_EXCLUDE_ALUEXEC", "value": 2},
   {"name": "TOKEN_EXCLUDE_VALUINST", "value": 4},
   {"name": "TOKEN_EXCLUDE_WAVERDY", "value": 8},
   {"name": "TOKEN_EXCLUDE_IMMED1", "value": 16},
   {"name": "TOKEN_EXCLUDE_IMMEDIATE", "value": 32},
   {"name": "TOKEN_EXCLUDE_REG", "value": 64},
   {"name": "TOKEN_EXCLUDE_EVENT", "value": 128},
   {"name": "TOKEN_EXCLUDE_INST", "value": 256},
   {"name": "TOKEN_EXCLUDE_UTILCTR", "value": 512},
   {"name": "TOKEN_EXCLUDE_WAVEALLOC", "value": 1024},
   {"name": "TOKEN_EXCLUDE_PERF", "value": 2048}
  ]
}

GB_TILE_MODE0__MICRO_TILE_MODE = {
 "entries": [
  {"name": "ADDR_SURF_DISPLAY_MICRO_TILING", "value": 0},
  {"name": "ADDR_SURF_THIN_MICRO_TILING", "value": 1},
  {"name": "ADDR_SURF_DEPTH_MICRO_TILING", "value": 2},
  {"name": "ADDR_SURF_THICK_MICRO_TILING_GFX6", "value": 3}
 ]
}

IMG_DATA_FORMAT_STENCIL = {
 "entries": [
  {"name": "IMG_DATA_FORMAT_S8_16", "value": 59},
  {"name": "IMG_DATA_FORMAT_S8_32", "value": 60},
 ]
}

VRSCombinerModeSC = {
 "entries": [
  {"name": "SC_VRS_COMB_MODE_PASSTHRU", "value": 0},
  {"name": "SC_VRS_COMB_MODE_OVERRIDE", "value": 1},
  {"name": "SC_VRS_COMB_MODE_MIN", "value": 2},
  {"name": "SC_VRS_COMB_MODE_MAX", "value": 3},
  {"name": "SC_VRS_COMB_MODE_SATURATE", "value": 4},
 ]
}

VRSHtileEncoding = {
 "entries": [
  {"name": "VRS_HTILE_DISABLE", "value": 0},
  {"name": "VRS_HTILE_2BIT_ENCODING", "value": 1},
  {"name": "VRS_HTILE_4BIT_ENCODING", "value": 2},
 ]
}

missing_enums_all = {
  'FLOAT_MODE': {
    "entries": [
      {"name": "FP_32_ROUND_TOWARDS_ZERO", "value": 3},
      {"name": "FP_16_64_ROUND_TOWARDS_ZERO", "value": 12},
      {"name": "FP_32_DENORMS", "value": 48},
      {"name": "FP_16_64_DENORMS", "value": 192},
    ]
  },
  'QUANT_MODE': {
    "entries": [
      {"name": "X_16_8_FIXED_POINT_1_16TH", "value": 0},
      {"name": "X_16_8_FIXED_POINT_1_8TH", "value": 1},
      {"name": "X_16_8_FIXED_POINT_1_4TH", "value": 2},
      {"name": "X_16_8_FIXED_POINT_1_2", "value": 3},
      {"name": "X_16_8_FIXED_POINT_1", "value": 4},
      {"name": "X_16_8_FIXED_POINT_1_256TH", "value": 5},
      {"name": "X_14_10_FIXED_POINT_1_1024TH", "value": 6},
      {"name": "X_12_12_FIXED_POINT_1_4096TH", "value": 7}
    ]
  },
  "CLIP_RULE": {
   "entries": [
    {"name": "OUT", "value": 1},
    {"name": "IN_0", "value": 2},
    {"name": "IN_1", "value": 4},
    {"name": "IN_10", "value": 8},
    {"name": "IN_2", "value": 16},
    {"name": "IN_20", "value": 32},
    {"name": "IN_21", "value": 64},
    {"name": "IN_210", "value": 128},
    {"name": "IN_3", "value": 256},
    {"name": "IN_30", "value": 512},
    {"name": "IN_31", "value": 1024},
    {"name": "IN_310", "value": 2048},
    {"name": "IN_32", "value": 4096},
    {"name": "IN_320", "value": 8192},
    {"name": "IN_321", "value": 16384},
    {"name": "IN_3210", "value": 32768}
   ]
  },
  'PA_SU_VTX_CNTL__ROUND_MODE': {
    "entries": [
      {"name": "X_TRUNCATE", "value": 0},
      {"name": "X_ROUND", "value": 1},
      {"name": "X_ROUND_TO_EVEN", "value": 2},
      {"name": "X_ROUND_TO_ODD", "value": 3}
    ]
  },
  "PA_SU_SC_MODE_CNTL__POLYMODE_FRONT_PTYPE": {
   "entries": [
    {"name": "X_DRAW_POINTS", "value": 0},
    {"name": "X_DRAW_LINES", "value": 1},
    {"name": "X_DRAW_TRIANGLES", "value": 2}
   ]
  },
  "PA_SU_SC_MODE_CNTL__POLY_MODE": {
   "entries": [
    {"name": "X_DISABLE_POLY_MODE", "value": 0},
    {"name": "X_DUAL_MODE", "value": 1}
   ]
  },
  'VGT_HS_OFFCHIP_PARAM__OFFCHIP_GRANULARITY': {
    "entries": [
      {"name": "X_8K_DWORDS", "value": 0},
      {"name": "X_4K_DWORDS", "value": 1},
      {"name": "X_2K_DWORDS", "value": 2},
      {"name": "X_1K_DWORDS", "value": 3}
    ]
  },
  "ROP3": {
   "entries": [
    {"name": "ROP3_CLEAR", "value": 0},
    {"name": "X_0X05", "value": 5},
    {"name": "X_0X0A", "value": 10},
    {"name": "X_0X0F", "value": 15},
    {"name": "ROP3_NOR", "value": 17},
    {"name": "ROP3_AND_INVERTED", "value": 34},
    {"name": "ROP3_COPY_INVERTED", "value": 51},
    {"name": "ROP3_AND_REVERSE", "value": 68},
    {"name": "X_0X50", "value": 80},
    {"name": "ROP3_INVERT", "value": 85},
    {"name": "X_0X5A", "value": 90},
    {"name": "X_0X5F", "value": 95},
    {"name": "ROP3_XOR", "value": 102},
    {"name": "ROP3_NAND", "value": 119},
    {"name": "ROP3_AND", "value": 136},
    {"name": "ROP3_EQUIVALENT", "value": 153},
    {"name": "X_0XA0", "value": 160},
    {"name": "X_0XA5", "value": 165},
    {"name": "ROP3_NO_OP", "value": 170},
    {"name": "X_0XAF", "value": 175},
    {"name": "ROP3_OR_INVERTED", "value": 187},
    {"name": "ROP3_COPY", "value": 204},
    {"name": "ROP3_OR_REVERSE", "value": 221},
    {"name": "ROP3_OR", "value": 238},
    {"name": "X_0XF0", "value": 240},
    {"name": "X_0XF5", "value": 245},
    {"name": "X_0XFA", "value": 250},
    {"name": "ROP3_SET", "value": 255}
   ]
  },
  "EXCP_EN": {
   "entries": [
    {"name": "INVALID", "value": 1},
    {"name": "INPUT_DENORMAL", "value": 2},
    {"name": "DIVIDE_BY_ZERO", "value": 4},
    {"name": "OVERFLOW", "value": 8},
    {"name": "UNDERFLOW", "value": 16},
    {"name": "INEXACT", "value": 32},
    {"name": "INT_DIVIDE_BY_ZERO", "value": 64},
    {"name": "ADDRESS_WATCH", "value": 128},
    {"name": "MEMORY_VIOLATION", "value": 256}
   ]
  }
}

missing_enums_gfx8plus = {
  **missing_enums_all,
  'CB_COLOR_DCC_CONTROL__MAX_UNCOMPRESSED_BLOCK_SIZE': {
    "entries": [
      {"name": "MAX_BLOCK_SIZE_64B", "value": 0},
      {"name": "MAX_BLOCK_SIZE_128B", "value": 1},
      {"name": "MAX_BLOCK_SIZE_256B", "value": 2}
    ]
  },
  'CB_COLOR_DCC_CONTROL__MIN_COMPRESSED_BLOCK_SIZE': {
    "entries": [
      {"name": "MIN_BLOCK_SIZE_32B", "value": 0},
      {"name": "MIN_BLOCK_SIZE_64B", "value": 1}
    ]
  },
}

missing_enums_gfx81plus = {
  **missing_enums_gfx8plus,
  "SX_BLEND_OPT_EPSILON__MRT0_EPSILON": {
    "entries": [
      {"name": "EXACT", "value": 0},
      # This determines whether epsilon is 0.5 or 0.75 in the unnormalized format
      # that is used to determine whether a channel is equal to 0 for blending.
      # 0.5 is exactly between 0 and the next representable value. 0.75 can be
      # used for less precise blending.
      {"name": "10BIT_FORMAT_0_5", "value": 2},  # (1.0 * 2^−11) * 1024 = 0.5
      {"name": "10BIT_FORMAT_0_75", "value": 3}, # (1.5 * 2^−11) * 1024 = 0.75
      {"name": "8BIT_FORMAT_0_5", "value": 6},   # (1.0 * 2^−9) * 256 = 0.5
      {"name": "8BIT_FORMAT_0_75", "value": 7},  # (1.5 * 2^−9) * 256 = 0.75
      {"name": "6BIT_FORMAT_0_5", "value": 10},  # (1.0 * 2^-7) * 64 = 0.5
      {"name": "6BIT_FORMAT_0_75", "value": 11}, # (1.5 * 2^-7) * 64 = 0.75
      {"name": "5BIT_FORMAT_0_5", "value": 12},  # (1.0 * 2^-6) * 32 = 0.5
      {"name": "5BIT_FORMAT_0_75", "value": 13}, # (1.5 * 2^-6) * 32 = 0.75
      {"name": "4BIT_FORMAT_0_5", "value": 14},  # (1.0 * 2^-5) * 16 = 0.5
      {"name": "4BIT_FORMAT_0_75", "value": 15}, # (1.5 * 2^-5) * 16 = 0.75
    ]
  },
}

missing_enums_gfx9 = {
  **missing_enums_gfx81plus,
  "DB_DFSM_CONTROL__PUNCHOUT_MODE": DB_DFSM_CONTROL__PUNCHOUT_MODE,
  "IMG_DATA_FORMAT_STENCIL": IMG_DATA_FORMAT_STENCIL,
  "SQ_IMG_RSRC_WORD4__BC_SWIZZLE": SQ_IMG_RSRC_WORD4__BC_SWIZZLE,
  "BinSizeExtend": {
    "entries": [
      {"name": "BIN_SIZE_32_PIXELS", "value": 0},
      {"name": "BIN_SIZE_64_PIXELS", "value": 1},
      {"name": "BIN_SIZE_128_PIXELS", "value": 2},
      {"name": "BIN_SIZE_256_PIXELS", "value": 3},
      {"name": "BIN_SIZE_512_PIXELS", "value": 4}
    ]
  },
  "ScUncertaintyRegionMode": {
    "entries": [
      {"name": "SC_HALF_LSB", "value": 0},
      {"name": "SC_LSB_ONE_SIDED", "value": 1},
      {"name": "SC_LSB_TWO_SIDED", "value": 2}
    ]
  },
}

missing_enums_gfx103plus = {
  **missing_enums_gfx81plus,
  "ColorFormat": ColorFormat,
  "ThreadTraceRegInclude": ThreadTraceRegInclude,
  "ThreadTraceTokenExclude": ThreadTraceTokenExclude,
}

missing_enums_gfx11plus = {
  **missing_enums_gfx103plus,
  "ZFormat": {
   "entries": [
    {"name": "Z_INVALID", "value": 0},
    {"name": "Z_16", "value": 1},
    {"name": "Z_24", "value": 2},
    {"name": "Z_32_FLOAT", "value": 3}
   ]
  },
  "StencilFormat": {
   "entries": [
    {"name": "STENCIL_INVALID", "value": 0},
    {"name": "STENCIL_8", "value": 1}
   ]
  },
  "SurfaceNumber": {
   "entries": [
    {"name": "NUMBER_UNORM", "value": 0},
    {"name": "NUMBER_SNORM", "value": 1},
    {"name": "NUMBER_USCALED", "value": 2},
    {"name": "NUMBER_SSCALED", "value": 3},
    {"name": "NUMBER_UINT", "value": 4},
    {"name": "NUMBER_SINT", "value": 5},
    {"name": "NUMBER_SRGB", "value": 6},
    {"name": "NUMBER_FLOAT", "value": 7}
   ]
  },
  "SurfaceSwap": {
   "entries": [
    {"name": "SWAP_STD", "value": 0},
    {"name": "SWAP_ALT", "value": 1},
    {"name": "SWAP_STD_REV", "value": 2},
    {"name": "SWAP_ALT_REV", "value": 3}
   ]
  },
}

enums_missing = {
  'gfx6': {
    **missing_enums_all,
   "GB_TILE_MODE0__MICRO_TILE_MODE": GB_TILE_MODE0__MICRO_TILE_MODE,
  },
  'gfx7': {
    **missing_enums_all,
  },
  'gfx8': {
    **missing_enums_gfx8plus,
  },
  'gfx81': {
    **missing_enums_gfx81plus,
  },
  'gfx9': {
    **missing_enums_gfx9,
  },
  'gfx940': {
    **missing_enums_gfx9,
  },
  'gfx10': {
    **missing_enums_gfx81plus,
    "DB_DFSM_CONTROL__PUNCHOUT_MODE": DB_DFSM_CONTROL__PUNCHOUT_MODE,
    "ThreadTraceRegInclude": ThreadTraceRegInclude,
    "ThreadTraceTokenExclude": ThreadTraceTokenExclude,
  },
  'gfx103': {
    **missing_enums_gfx103plus,
    "SX_DOWNCONVERT_FORMAT": SX_DOWNCONVERT_FORMAT,
    "DB_DFSM_CONTROL__PUNCHOUT_MODE": DB_DFSM_CONTROL__PUNCHOUT_MODE,
    "VRSHtileEncoding": VRSHtileEncoding,
    "VRSCombinerModeSC": VRSCombinerModeSC,
  },
  'gfx11': {
    **missing_enums_gfx11plus,
  },
  'gfx115': {
    **missing_enums_gfx11plus,
  },
}

# Register field definitions that are missing in kernel headers
fields_missing = {
  # Format:
  #   Register: [[Field, StartBit, EndBit, EnumType(optional), ReplaceField=True/False(optional)], ...]
  'gfx6': {
    "COMPUTE_RESOURCE_LIMITS": [["WAVES_PER_SH_GFX6", 0, 5]],
    "GB_TILE_MODE0": [["MICRO_TILE_MODE", 0, 1, "GB_TILE_MODE0__MICRO_TILE_MODE"]],
    "SQ_IMG_SAMP_WORD3": [["UPGRADED_DEPTH", 29, 29]],
    "SQ_THREAD_TRACE_MASK": [["RANDOM_SEED", 16, 31]],
  },
  'gfx7': {
    "SQ_IMG_SAMP_WORD3": [["UPGRADED_DEPTH", 29, 29]],
    "SQ_THREAD_TRACE_MASK": [["RANDOM_SEED", 16, 31]],
  },
  'gfx8': {
    "SQ_IMG_SAMP_WORD3": [["UPGRADED_DEPTH", 29, 29]],
    "SQ_THREAD_TRACE_MASK": [["RANDOM_SEED", 16, 31]],
  },
  'gfx81': {
    "SQ_IMG_SAMP_WORD3": [["UPGRADED_DEPTH", 29, 29]],
    "SQ_THREAD_TRACE_MASK": [["RANDOM_SEED", 16, 31]],
  },
  'gfx9': {
    "SQ_IMG_RSRC_WORD1": [
      ["DATA_FORMAT_STENCIL", 20, 25, "IMG_DATA_FORMAT_STENCIL"],
      ["NUM_FORMAT_FMASK", 26, 29, "IMG_NUM_FORMAT_FMASK"]
    ],
  },
  'gfx10': {
    "DB_RESERVED_REG_2": [["RESOURCE_LEVEL", 28, 31, None, True]],
  },
  'gfx103': {
    "DB_RESERVED_REG_2": [["RESOURCE_LEVEL", 28, 31, None, True]],
    "VGT_DRAW_PAYLOAD_CNTL": [["EN_VRS_RATE", 6, 6]],
    "VGT_SHADER_STAGES_EN": [["PRIMGEN_PASSTHRU_NO_MSG", 26, 26]],
  },
  'gfx11': {
    "VGT_DRAW_PAYLOAD_CNTL": [["EN_VRS_RATE", 6, 6]],
    # Only GFX1103_R2:
    "CB_COLOR0_FDCC_CONTROL": [["DISABLE_OVERRIDE_INCONSISTENT_KEYS", 25, 25],
                               ["ENABLE_MAX_COMP_FRAG_OVERRIDE", 26, 26],
                               ["MAX_COMP_FRAGS", 27, 29]],
    "CB_COLOR1_FDCC_CONTROL": [["DISABLE_OVERRIDE_INCONSISTENT_KEYS", 25, 25],
                               ["ENABLE_MAX_COMP_FRAG_OVERRIDE", 26, 26],
                               ["MAX_COMP_FRAGS", 27, 29]],
    "CB_COLOR2_FDCC_CONTROL": [["DISABLE_OVERRIDE_INCONSISTENT_KEYS", 25, 25],
                               ["ENABLE_MAX_COMP_FRAG_OVERRIDE", 26, 26],
                               ["MAX_COMP_FRAGS", 27, 29]],
    "CB_COLOR3_FDCC_CONTROL": [["DISABLE_OVERRIDE_INCONSISTENT_KEYS", 25, 25],
                               ["ENABLE_MAX_COMP_FRAG_OVERRIDE", 26, 26],
                               ["MAX_COMP_FRAGS", 27, 29]],
    "CB_COLOR4_FDCC_CONTROL": [["DISABLE_OVERRIDE_INCONSISTENT_KEYS", 25, 25],
                               ["ENABLE_MAX_COMP_FRAG_OVERRIDE", 26, 26],
                               ["MAX_COMP_FRAGS", 27, 29]],
    "CB_COLOR5_FDCC_CONTROL": [["DISABLE_OVERRIDE_INCONSISTENT_KEYS", 25, 25],
                               ["ENABLE_MAX_COMP_FRAG_OVERRIDE", 26, 26],
                               ["MAX_COMP_FRAGS", 27, 29]],
    "CB_COLOR6_FDCC_CONTROL": [["DISABLE_OVERRIDE_INCONSISTENT_KEYS", 25, 25],
                               ["ENABLE_MAX_COMP_FRAG_OVERRIDE", 26, 26],
                               ["MAX_COMP_FRAGS", 27, 29]],
    "CB_COLOR7_FDCC_CONTROL": [["DISABLE_OVERRIDE_INCONSISTENT_KEYS", 25, 25],
                               ["ENABLE_MAX_COMP_FRAG_OVERRIDE", 26, 26],
                               ["MAX_COMP_FRAGS", 27, 29]],
  },
}

######### END HARDCODED CONFIGURATION

def bitcount(n):
    return bin(n).count('1')

def generate_json(gfx_level, amd_headers_path):
    gc_base_offsets = gfx_levels[gfx_level][0]

    # Add the path to the filenames
    filenames = [amd_headers_path + '/' + a for a in gfx_levels[gfx_level][1:]]

    # Open the files
    files = [open(a, 'r').readlines() if a is not None else None for a in filenames]

    # Parse the offset.h file
    name = None
    offset = None
    added_offsets = set()
    regs = {}
    for line in files[0]:
        r = re_offset.match(line)
        if r is None:
            continue

        if '_BASE_IDX' not in r.group('name'):
            name = r.group('name')
            offset = int(r.group('value'), 0) * 4
            if len(gc_base_offsets) > 0 and r.group('mm') == 'mm':
                continue
        else:
            if name != r.group('name')[:-9]:
                print('Warning: "{0}" not preceded by {1} but by {2}'.format(r.group('name'), r.group('name')[:-9], name))
                continue
            idx = int(r.group('value'))
            assert idx < len(gc_base_offsets)
            offset += gc_base_offsets[idx] * 4

        # Remove the _UMD suffix because it was mistakenly added to indicate it's for a User-Mode Driver
        if name[-4:] == '_UMD':
            name = name[:-4]

        # Only accept writeable registers and debug registers
        if register_filter(gfx_level, name, offset, offset in added_offsets):
            regs[name] = {
                'chips': [gfx_level],
                'map': {'at': offset, 'to': 'mm'},
                'name': name,
            }
            added_offsets.add(offset)


    # Parse the sh_mask.h file
    shifts = {}
    masks = {}
    for line in files[1]:
        r = re_shift.match(line)
        is_shift = r is not None
        r = re_mask.match(line) if r is None else r
        if r is None:
            continue

        name = r.group('name')
        if name not in regs.keys():
            continue

        field = r.group('field')
        value = int(r.group('value'), 0)
        assert not is_shift or value < 32

        d = shifts if is_shift else masks
        if name not in d:
            d[name] = {}
        d[name][field] = value


    # Parse the enum.h file
    re_enum_begin = re.compile(r'^typedef enum (?P<name>\w+) {\n')
    re_enum_entry = re.compile(r'\s*(?P<name>\w+)\s*=\s*(?P<value>\w+),?\n')
    re_enum_end = re.compile(r'^} \w+;\n')
    inside_enum = False
    name = None
    enums = enums_missing[gfx_level] if gfx_level in enums_missing else {}

    for line in files[2]:
        r = re_enum_begin.match(line)
        if r is not None:
            name = r.group('name')
            if name in enums:
                continue
            enums[name] = {'entries': []}
            inside_enum = True
            continue

        r = re_enum_end.match(line)
        if r is not None:
            inside_enum = False
            name = None
            continue

        if inside_enum:
            r = re_enum_entry.match(line)
            assert r
            enums[name]['entries'].append({
                'name': r.group('name'),
                'value': int(r.group('value'), 0),
            })


    # Assemble everything
    reg_types = {}
    reg_mappings = []
    missing_fields = fields_missing[gfx_level] if gfx_level in fields_missing else {}

    for (name, reg) in regs.items():
        type = {'fields': []}

        if name in shifts and name in masks:
            for (field, shift) in shifts[name].items():
                if field not in masks[name]:
                    continue

                new = {
                    'bits': [shift, shift + bitcount(masks[name][field]) - 1],
                    'name': field,
                }
                if field in enum_map:
                    type_map = enum_map[field]
                    type_name = None

                    if len(type_map) == 1:
                        type_name = type_map[0];
                    else:
                        reg_index = type_map.index(name) if name in type_map else -1
                        if reg_index >= 1 and reg_index % 2 == 1:
                            type_name = type_map[reg_index - 1]

                    if type_name is not None:
                        if type_name not in enums:
                            print('{0}: {1} type not found for {2}.{3}'
                                  .format(gfx_level, type_name, name, field), file=sys.stderr)
                        else:
                            new['enum_ref'] = type_name

                type['fields'].append(new)

        if name in missing_fields:
            fields = missing_fields[name]
            for f in fields:
                field = {
                    'bits': [f[1], f[2]],
                    'name': f[0],
                }
                if len(f) >= 4 and f[3] is not None and f[3] in enums:
                    field['enum_ref'] = f[3]
                # missing_fields should replace overlapping fields if requested
                if len(f) >= 5 and f[4]:
                    for f2 in type['fields']:
                        if f2['bits'] == field['bits']:
                            type['fields'].remove(f2)

                type['fields'].append(field)

        if len(type['fields']) > 0:
            reg_types[name] = type

            # Don't define types that have only one field covering all bits
            field0_bits = type['fields'][0]['bits'];
            if len(type['fields']) > 1 or field0_bits[0] != 0 or field0_bits[1] != 31:
                reg['type_ref'] = name

            reg_mappings.append(reg)


    # Generate and canonicalize json
    all = {
        'enums': enums,
        'register_mappings': reg_mappings,
        'register_types': reg_types,
    }

    return json_canonicalize(io.StringIO(json.dumps(all, indent=1)))


if __name__ == '__main__':
    if len(sys.argv) <= 1 or (sys.argv[1] not in gfx_levels and sys.argv[1] != 'all'):
        print('First parameter should be one of: all, ' + ', '.join(gfx_levels.keys()), file=sys.stderr)
        sys.exit(1)

    if len(sys.argv) <= 2:
        print('Second parameter should be the path to the amd/include directory.', file=sys.stderr)
        sys.exit(1)

    if sys.argv[1] == 'all':
        for gfx_level in gfx_levels.keys():
            print(generate_json(gfx_level, sys.argv[2]), file=open(gfx_level + '.json', 'w'))
        sys.exit(0)

    print(generate_json(sys.argv[1], sys.argv[2]))
