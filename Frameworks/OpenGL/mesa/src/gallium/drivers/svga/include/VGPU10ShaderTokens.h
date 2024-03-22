/* SPDX-License-Identifier: GPL-2.0 OR MIT */
/*
 * Copyright 2012-2022 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

/*
 * VGPU10ShaderTokens.h --
 *
 *    VGPU10 shader token definitions.
 */





#ifndef VGPU10SHADERTOKENS_H
#define VGPU10SHADERTOKENS_H


#define VGPU10_MAX_VS_INPUTS 16
#define VGPU10_MAX_VS_OUTPUTS 16
#define VGPU10_MAX_GS_INPUTS 16
#define VGPU10_MAX_GS_OUTPUTS 32
#define VGPU10_MAX_FS_INPUTS 32
#define VGPU10_MAX_FS_OUTPUTS 8
#define VGPU10_MAX_TEMPS 4096
#define VGPU10_MAX_CONSTANT_BUFFERS (14 + 1)
#define VGPU10_MAX_CONSTANT_BUFFER_ELEMENT_COUNT 4096
#define VGPU10_MAX_IMMEDIATE_CONSTANT_BUFFER_ELEMENT_COUNT 4096
#define VGPU10_MAX_SAMPLERS 16
#define VGPU10_MAX_RESOURCES 128
#define VGPU10_MIN_TEXEL_FETCH_OFFSET -8
#define VGPU10_MAX_TEXEL_FETCH_OFFSET 7


#define VGPU10_1_MAX_VS_INPUTS   32
#define VGPU10_1_MAX_VS_OUTPUTS  32
#define VGPU10_1_MAX_GS_INPUTS   32


#define VGPU11_MAX_HS_INPUT_CONTROL_POINTS      32
#define VGPU11_MAX_HS_INPUT_PATCH_CONSTANTS     32
#define VGPU11_MAX_HS_OUTPUT_CP_PHASE_ELEMENTS  32
#define VGPU11_MAX_HS_OUTPUT_CONTROL_POINTS     32
#define VGPU11_MAX_HS_OUTPUTS                   32
#define VGPU11_MAX_DS_INPUT_CONTROL_POINTS      32
#define VGPU11_MAX_DS_INPUT_PATCH_CONSTANTS     32
#define VGPU11_MAX_DS_OUTPUTS                   32
#define VGPU11_MAX_GS_STREAMS                   4
#define VGPU11_MAX_FUNCTION_BODIES              256
#define VGPU11_MAX_FUNCTION_TABLES              256
#define VGPU11_MAX_INTERFACES                   253


#define VGPU10_MAX_INPUTS                 32
#define VGPU10_MAX_OUTPUTS                32
#define VGPU10_MAX_INPUT_PATCH_CONSTANTS  32

typedef enum {
   VGPU10_PIXEL_SHADER     = 0,
   VGPU10_VERTEX_SHADER    = 1,
   VGPU10_GEOMETRY_SHADER  = 2,


   VGPU10_HULL_SHADER      = 3,
   VGPU10_DOMAIN_SHADER    = 4,
   VGPU10_COMPUTE_SHADER   = 5
} VGPU10_PROGRAM_TYPE;

typedef union {
   struct {
      unsigned int minorVersion  : 4;
      unsigned int majorVersion  : 4;
      unsigned int               : 8;
      unsigned int programType   : 16;
   };
   uint32 value;
} VGPU10ProgramToken;


typedef enum {
   VGPU10_OPCODE_ADD                               = 0,
   VGPU10_OPCODE_AND                               = 1,
   VGPU10_OPCODE_BREAK                             = 2,
   VGPU10_OPCODE_BREAKC                            = 3,
   VGPU10_OPCODE_CALL                              = 4,
   VGPU10_OPCODE_CALLC                             = 5,
   VGPU10_OPCODE_CASE                              = 6,
   VGPU10_OPCODE_CONTINUE                          = 7,
   VGPU10_OPCODE_CONTINUEC                         = 8,
   VGPU10_OPCODE_CUT                               = 9,
   VGPU10_OPCODE_DEFAULT                           = 10,
   VGPU10_OPCODE_DERIV_RTX                         = 11,
   VGPU10_OPCODE_DERIV_RTY                         = 12,
   VGPU10_OPCODE_DISCARD                           = 13,
   VGPU10_OPCODE_DIV                               = 14,
   VGPU10_OPCODE_DP2                               = 15,
   VGPU10_OPCODE_DP3                               = 16,
   VGPU10_OPCODE_DP4                               = 17,
   VGPU10_OPCODE_ELSE                              = 18,
   VGPU10_OPCODE_EMIT                              = 19,
   VGPU10_OPCODE_EMITTHENCUT                       = 20,
   VGPU10_OPCODE_ENDIF                             = 21,
   VGPU10_OPCODE_ENDLOOP                           = 22,
   VGPU10_OPCODE_ENDSWITCH                         = 23,
   VGPU10_OPCODE_EQ                                = 24,
   VGPU10_OPCODE_EXP                               = 25,
   VGPU10_OPCODE_FRC                               = 26,
   VGPU10_OPCODE_FTOI                              = 27,
   VGPU10_OPCODE_FTOU                              = 28,
   VGPU10_OPCODE_GE                                = 29,
   VGPU10_OPCODE_IADD                              = 30,
   VGPU10_OPCODE_IF                                = 31,
   VGPU10_OPCODE_IEQ                               = 32,
   VGPU10_OPCODE_IGE                               = 33,
   VGPU10_OPCODE_ILT                               = 34,
   VGPU10_OPCODE_IMAD                              = 35,
   VGPU10_OPCODE_IMAX                              = 36,
   VGPU10_OPCODE_IMIN                              = 37,
   VGPU10_OPCODE_IMUL                              = 38,
   VGPU10_OPCODE_INE                               = 39,
   VGPU10_OPCODE_INEG                              = 40,
   VGPU10_OPCODE_ISHL                              = 41,
   VGPU10_OPCODE_ISHR                              = 42,
   VGPU10_OPCODE_ITOF                              = 43,
   VGPU10_OPCODE_LABEL                             = 44,
   VGPU10_OPCODE_LD                                = 45,
   VGPU10_OPCODE_LD_MS                             = 46,
   VGPU10_OPCODE_LOG                               = 47,
   VGPU10_OPCODE_LOOP                              = 48,
   VGPU10_OPCODE_LT                                = 49,
   VGPU10_OPCODE_MAD                               = 50,
   VGPU10_OPCODE_MIN                               = 51,
   VGPU10_OPCODE_MAX                               = 52,
   VGPU10_OPCODE_CUSTOMDATA                        = 53,
   VGPU10_OPCODE_MOV                               = 54,
   VGPU10_OPCODE_MOVC                              = 55,
   VGPU10_OPCODE_MUL                               = 56,
   VGPU10_OPCODE_NE                                = 57,
   VGPU10_OPCODE_NOP                               = 58,
   VGPU10_OPCODE_NOT                               = 59,
   VGPU10_OPCODE_OR                                = 60,
   VGPU10_OPCODE_RESINFO                           = 61,
   VGPU10_OPCODE_RET                               = 62,
   VGPU10_OPCODE_RETC                              = 63,
   VGPU10_OPCODE_ROUND_NE                          = 64,
   VGPU10_OPCODE_ROUND_NI                          = 65,
   VGPU10_OPCODE_ROUND_PI                          = 66,
   VGPU10_OPCODE_ROUND_Z                           = 67,
   VGPU10_OPCODE_RSQ                               = 68,
   VGPU10_OPCODE_SAMPLE                            = 69,
   VGPU10_OPCODE_SAMPLE_C                          = 70,
   VGPU10_OPCODE_SAMPLE_C_LZ                       = 71,
   VGPU10_OPCODE_SAMPLE_L                          = 72,
   VGPU10_OPCODE_SAMPLE_D                          = 73,
   VGPU10_OPCODE_SAMPLE_B                          = 74,
   VGPU10_OPCODE_SQRT                              = 75,
   VGPU10_OPCODE_SWITCH                            = 76,
   VGPU10_OPCODE_SINCOS                            = 77,
   VGPU10_OPCODE_UDIV                              = 78,
   VGPU10_OPCODE_ULT                               = 79,
   VGPU10_OPCODE_UGE                               = 80,
   VGPU10_OPCODE_UMUL                              = 81,
   VGPU10_OPCODE_UMAD                              = 82,
   VGPU10_OPCODE_UMAX                              = 83,
   VGPU10_OPCODE_UMIN                              = 84,
   VGPU10_OPCODE_USHR                              = 85,
   VGPU10_OPCODE_UTOF                              = 86,
   VGPU10_OPCODE_XOR                               = 87,
   VGPU10_OPCODE_DCL_RESOURCE                      = 88,
   VGPU10_OPCODE_DCL_CONSTANT_BUFFER               = 89,
   VGPU10_OPCODE_DCL_SAMPLER                       = 90,
   VGPU10_OPCODE_DCL_INDEX_RANGE                   = 91,
   VGPU10_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY  = 92,
   VGPU10_OPCODE_DCL_GS_INPUT_PRIMITIVE            = 93,
   VGPU10_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT       = 94,
   VGPU10_OPCODE_DCL_INPUT                         = 95,
   VGPU10_OPCODE_DCL_INPUT_SGV                     = 96,
   VGPU10_OPCODE_DCL_INPUT_SIV                     = 97,
   VGPU10_OPCODE_DCL_INPUT_PS                      = 98,
   VGPU10_OPCODE_DCL_INPUT_PS_SGV                  = 99,
   VGPU10_OPCODE_DCL_INPUT_PS_SIV                  = 100,
   VGPU10_OPCODE_DCL_OUTPUT                        = 101,
   VGPU10_OPCODE_DCL_OUTPUT_SGV                    = 102,
   VGPU10_OPCODE_DCL_OUTPUT_SIV                    = 103,
   VGPU10_OPCODE_DCL_TEMPS                         = 104,
   VGPU10_OPCODE_DCL_INDEXABLE_TEMP                = 105,
   VGPU10_OPCODE_DCL_GLOBAL_FLAGS                  = 106,


   VGPU10_OPCODE_VMWARE                            = 107,


   VGPU10_OPCODE_LOD                               = 108,
   VGPU10_OPCODE_GATHER4                           = 109,
   VGPU10_OPCODE_SAMPLE_POS                        = 110,
   VGPU10_OPCODE_SAMPLE_INFO                       = 111,


   VGPU10_OPCODE_RESERVED1                         = 112,
   VGPU10_OPCODE_HS_DECLS                          = 113,
   VGPU10_OPCODE_HS_CONTROL_POINT_PHASE            = 114,
   VGPU10_OPCODE_HS_FORK_PHASE                     = 115,
   VGPU10_OPCODE_HS_JOIN_PHASE                     = 116,
   VGPU10_OPCODE_EMIT_STREAM                       = 117,
   VGPU10_OPCODE_CUT_STREAM                        = 118,
   VGPU10_OPCODE_EMITTHENCUT_STREAM                = 119,
   VGPU10_OPCODE_INTERFACE_CALL                    = 120,
   VGPU10_OPCODE_BUFINFO                           = 121,
   VGPU10_OPCODE_DERIV_RTX_COARSE                  = 122,
   VGPU10_OPCODE_DERIV_RTX_FINE                    = 123,
   VGPU10_OPCODE_DERIV_RTY_COARSE                  = 124,
   VGPU10_OPCODE_DERIV_RTY_FINE                    = 125,
   VGPU10_OPCODE_GATHER4_C                         = 126,
   VGPU10_OPCODE_GATHER4_PO                        = 127,
   VGPU10_OPCODE_GATHER4_PO_C                      = 128,
   VGPU10_OPCODE_RCP                               = 129,
   VGPU10_OPCODE_F32TOF16                          = 130,
   VGPU10_OPCODE_F16TOF32                          = 131,
   VGPU10_OPCODE_UADDC                             = 132,
   VGPU10_OPCODE_USUBB                             = 133,
   VGPU10_OPCODE_COUNTBITS                         = 134,
   VGPU10_OPCODE_FIRSTBIT_HI                       = 135,
   VGPU10_OPCODE_FIRSTBIT_LO                       = 136,
   VGPU10_OPCODE_FIRSTBIT_SHI                      = 137,
   VGPU10_OPCODE_UBFE                              = 138,
   VGPU10_OPCODE_IBFE                              = 139,
   VGPU10_OPCODE_BFI                               = 140,
   VGPU10_OPCODE_BFREV                             = 141,
   VGPU10_OPCODE_SWAPC                             = 142,
   VGPU10_OPCODE_DCL_STREAM                        = 143,
   VGPU10_OPCODE_DCL_FUNCTION_BODY                 = 144,
   VGPU10_OPCODE_DCL_FUNCTION_TABLE                = 145,
   VGPU10_OPCODE_DCL_INTERFACE                     = 146,
   VGPU10_OPCODE_DCL_INPUT_CONTROL_POINT_COUNT     = 147,
   VGPU10_OPCODE_DCL_OUTPUT_CONTROL_POINT_COUNT    = 148,
   VGPU10_OPCODE_DCL_TESS_DOMAIN                   = 149,
   VGPU10_OPCODE_DCL_TESS_PARTITIONING             = 150,
   VGPU10_OPCODE_DCL_TESS_OUTPUT_PRIMITIVE         = 151,
   VGPU10_OPCODE_DCL_HS_MAX_TESSFACTOR             = 152,
   VGPU10_OPCODE_DCL_HS_FORK_PHASE_INSTANCE_COUNT  = 153,
   VGPU10_OPCODE_DCL_HS_JOIN_PHASE_INSTANCE_COUNT  = 154,
   VGPU10_OPCODE_DCL_THREAD_GROUP                  = 155,
   VGPU10_OPCODE_DCL_UAV_TYPED                     = 156,
   VGPU10_OPCODE_DCL_UAV_RAW                       = 157,
   VGPU10_OPCODE_DCL_UAV_STRUCTURED                = 158,
   VGPU10_OPCODE_DCL_TGSM_RAW                      = 159,
   VGPU10_OPCODE_DCL_TGSM_STRUCTURED               = 160,
   VGPU10_OPCODE_DCL_RESOURCE_RAW                  = 161,
   VGPU10_OPCODE_DCL_RESOURCE_STRUCTURED           = 162,
   VGPU10_OPCODE_LD_UAV_TYPED                      = 163,
   VGPU10_OPCODE_STORE_UAV_TYPED                   = 164,
   VGPU10_OPCODE_LD_RAW                            = 165,
   VGPU10_OPCODE_STORE_RAW                         = 166,
   VGPU10_OPCODE_LD_STRUCTURED                     = 167,
   VGPU10_OPCODE_STORE_STRUCTURED                  = 168,
   VGPU10_OPCODE_ATOMIC_AND                        = 169,
   VGPU10_OPCODE_ATOMIC_OR                         = 170,
   VGPU10_OPCODE_ATOMIC_XOR                        = 171,
   VGPU10_OPCODE_ATOMIC_CMP_STORE                  = 172,
   VGPU10_OPCODE_ATOMIC_IADD                       = 173,
   VGPU10_OPCODE_ATOMIC_IMAX                       = 174,
   VGPU10_OPCODE_ATOMIC_IMIN                       = 175,
   VGPU10_OPCODE_ATOMIC_UMAX                       = 176,
   VGPU10_OPCODE_ATOMIC_UMIN                       = 177,
   VGPU10_OPCODE_IMM_ATOMIC_ALLOC                  = 178,
   VGPU10_OPCODE_IMM_ATOMIC_CONSUME                = 179,
   VGPU10_OPCODE_IMM_ATOMIC_IADD                   = 180,
   VGPU10_OPCODE_IMM_ATOMIC_AND                    = 181,
   VGPU10_OPCODE_IMM_ATOMIC_OR                     = 182,
   VGPU10_OPCODE_IMM_ATOMIC_XOR                    = 183,
   VGPU10_OPCODE_IMM_ATOMIC_EXCH                   = 184,
   VGPU10_OPCODE_IMM_ATOMIC_CMP_EXCH               = 185,
   VGPU10_OPCODE_IMM_ATOMIC_IMAX                   = 186,
   VGPU10_OPCODE_IMM_ATOMIC_IMIN                   = 187,
   VGPU10_OPCODE_IMM_ATOMIC_UMAX                   = 188,
   VGPU10_OPCODE_IMM_ATOMIC_UMIN                   = 189,
   VGPU10_OPCODE_SYNC                              = 190,
   VGPU10_OPCODE_DADD                              = 191,
   VGPU10_OPCODE_DMAX                              = 192,
   VGPU10_OPCODE_DMIN                              = 193,
   VGPU10_OPCODE_DMUL                              = 194,
   VGPU10_OPCODE_DEQ                               = 195,
   VGPU10_OPCODE_DGE                               = 196,
   VGPU10_OPCODE_DLT                               = 197,
   VGPU10_OPCODE_DNE                               = 198,
   VGPU10_OPCODE_DMOV                              = 199,
   VGPU10_OPCODE_DMOVC                             = 200,
   VGPU10_OPCODE_DTOF                              = 201,
   VGPU10_OPCODE_FTOD                              = 202,
   VGPU10_OPCODE_EVAL_SNAPPED                      = 203,
   VGPU10_OPCODE_EVAL_SAMPLE_INDEX                 = 204,
   VGPU10_OPCODE_EVAL_CENTROID                     = 205,
   VGPU10_OPCODE_DCL_GS_INSTANCE_COUNT             = 206,
   VGPU10_OPCODE_ABORT                             = 207,
   VGPU10_OPCODE_DEBUG_BREAK                       = 208,


   VGPU10_OPCODE_RESERVED0                         = 209,
   VGPU10_OPCODE_DDIV                              = 210,
   VGPU10_OPCODE_DFMA                              = 211,
   VGPU10_OPCODE_DRCP                              = 212,
   VGPU10_OPCODE_MSAD                              = 213,
   VGPU10_OPCODE_DTOI                              = 214,
   VGPU10_OPCODE_DTOU                              = 215,
   VGPU10_OPCODE_ITOD                              = 216,
   VGPU10_OPCODE_UTOD                              = 217,

   VGPU10_NUM_OPCODES
} VGPU10_OPCODE_TYPE;


typedef enum {
   VGPU10_VMWARE_OPCODE_IDIV                       = 0,
   VGPU10_VMWARE_OPCODE_DFRC                       = 1,
   VGPU10_VMWARE_OPCODE_DRSQ                       = 2,
   VGPU10_VMWARE_NUM_OPCODES
} VGPU10_VMWARE_OPCODE_TYPE;

typedef enum {
   VGPU10_INTERPOLATION_UNDEFINED = 0,
   VGPU10_INTERPOLATION_CONSTANT = 1,
   VGPU10_INTERPOLATION_LINEAR = 2,
   VGPU10_INTERPOLATION_LINEAR_CENTROID = 3,
   VGPU10_INTERPOLATION_LINEAR_NOPERSPECTIVE = 4,
   VGPU10_INTERPOLATION_LINEAR_NOPERSPECTIVE_CENTROID = 5,
   VGPU10_INTERPOLATION_LINEAR_SAMPLE = 6,
   VGPU10_INTERPOLATION_LINEAR_NOPERSPECTIVE_SAMPLE = 7
} VGPU10_INTERPOLATION_MODE;

typedef enum {
   VGPU10_RESOURCE_DIMENSION_UNKNOWN            = 0,
   VGPU10_RESOURCE_DIMENSION_BUFFER             = 1,
   VGPU10_RESOURCE_DIMENSION_TEXTURE1D          = 2,
   VGPU10_RESOURCE_DIMENSION_TEXTURE2D          = 3,
   VGPU10_RESOURCE_DIMENSION_TEXTURE2DMS        = 4,
   VGPU10_RESOURCE_DIMENSION_TEXTURE3D          = 5,
   VGPU10_RESOURCE_DIMENSION_TEXTURECUBE        = 6,
   VGPU10_RESOURCE_DIMENSION_TEXTURE1DARRAY     = 7,
   VGPU10_RESOURCE_DIMENSION_TEXTURE2DARRAY     = 8,
   VGPU10_RESOURCE_DIMENSION_TEXTURE2DMSARRAY   = 9,
   VGPU10_RESOURCE_DIMENSION_TEXTURECUBEARRAY   = 10,


   VGPU10_RESOURCE_DIMENSION_RAW_BUFFER         = 11,
   VGPU10_RESOURCE_DIMENSION_STRUCTURED_BUFFER  = 12,
   VGPU10_RESOURCE_DIMENSION_MAX                = 12
} VGPU10_RESOURCE_DIMENSION;

typedef enum {
   VGPU10_SAMPLER_MODE_DEFAULT = 0,
   VGPU10_SAMPLER_MODE_COMPARISON = 1,
   VGPU10_SAMPLER_MODE_MONO = 2
} VGPU10_SAMPLER_MODE;

typedef enum {
   VGPU10_INSTRUCTION_TEST_ZERO     = 0,
   VGPU10_INSTRUCTION_TEST_NONZERO  = 1
} VGPU10_INSTRUCTION_TEST_BOOLEAN;

typedef enum {
   VGPU10_CB_IMMEDIATE_INDEXED   = 0,
   VGPU10_CB_DYNAMIC_INDEXED     = 1
} VGPU10_CB_ACCESS_PATTERN;

typedef enum {
   VGPU10_PRIMITIVE_UNDEFINED    = 0,
   VGPU10_PRIMITIVE_POINT        = 1,
   VGPU10_PRIMITIVE_LINE         = 2,
   VGPU10_PRIMITIVE_TRIANGLE     = 3,
   VGPU10_PRIMITIVE_LINE_ADJ     = 6,
   VGPU10_PRIMITIVE_TRIANGLE_ADJ = 7,
   VGPU10_PRIMITIVE_SM40_MAX     = 7,


   VGPU10_PRIMITIVE_1_CONTROL_POINT_PATCH    = 8,
   VGPU10_PRIMITIVE_2_CONTROL_POINT_PATCH    = 9,
   VGPU10_PRIMITIVE_3_CONTROL_POINT_PATCH    = 10,
   VGPU10_PRIMITIVE_4_CONTROL_POINT_PATCH    = 11,
   VGPU10_PRIMITIVE_5_CONTROL_POINT_PATCH    = 12,
   VGPU10_PRIMITIVE_6_CONTROL_POINT_PATCH    = 13,
   VGPU10_PRIMITIVE_7_CONTROL_POINT_PATCH    = 14,
   VGPU10_PRIMITIVE_8_CONTROL_POINT_PATCH    = 15,
   VGPU10_PRIMITIVE_9_CONTROL_POINT_PATCH    = 16,
   VGPU10_PRIMITIVE_10_CONTROL_POINT_PATCH   = 17,
   VGPU10_PRIMITIVE_11_CONTROL_POINT_PATCH   = 18,
   VGPU10_PRIMITIVE_12_CONTROL_POINT_PATCH   = 19,
   VGPU10_PRIMITIVE_13_CONTROL_POINT_PATCH   = 20,
   VGPU10_PRIMITIVE_14_CONTROL_POINT_PATCH   = 21,
   VGPU10_PRIMITIVE_15_CONTROL_POINT_PATCH   = 22,
   VGPU10_PRIMITIVE_16_CONTROL_POINT_PATCH   = 23,
   VGPU10_PRIMITIVE_17_CONTROL_POINT_PATCH   = 24,
   VGPU10_PRIMITIVE_18_CONTROL_POINT_PATCH   = 25,
   VGPU10_PRIMITIVE_19_CONTROL_POINT_PATCH   = 26,
   VGPU10_PRIMITIVE_20_CONTROL_POINT_PATCH   = 27,
   VGPU10_PRIMITIVE_21_CONTROL_POINT_PATCH   = 28,
   VGPU10_PRIMITIVE_22_CONTROL_POINT_PATCH   = 29,
   VGPU10_PRIMITIVE_23_CONTROL_POINT_PATCH   = 30,
   VGPU10_PRIMITIVE_24_CONTROL_POINT_PATCH   = 31,
   VGPU10_PRIMITIVE_25_CONTROL_POINT_PATCH   = 32,
   VGPU10_PRIMITIVE_26_CONTROL_POINT_PATCH   = 33,
   VGPU10_PRIMITIVE_27_CONTROL_POINT_PATCH   = 34,
   VGPU10_PRIMITIVE_28_CONTROL_POINT_PATCH   = 35,
   VGPU10_PRIMITIVE_29_CONTROL_POINT_PATCH   = 36,
   VGPU10_PRIMITIVE_30_CONTROL_POINT_PATCH   = 37,
   VGPU10_PRIMITIVE_31_CONTROL_POINT_PATCH   = 38,
   VGPU10_PRIMITIVE_32_CONTROL_POINT_PATCH   = 39,
   VGPU10_PRIMITIVE_MAX                      = 39
} VGPU10_PRIMITIVE;

typedef enum {
   VGPU10_PRIMITIVE_TOPOLOGY_UNDEFINED          = 0,
   VGPU10_PRIMITIVE_TOPOLOGY_POINTLIST          = 1,
   VGPU10_PRIMITIVE_TOPOLOGY_LINELIST           = 2,
   VGPU10_PRIMITIVE_TOPOLOGY_LINESTRIP          = 3,
   VGPU10_PRIMITIVE_TOPOLOGY_TRIANGLELIST       = 4,
   VGPU10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP      = 5,
   VGPU10_PRIMITIVE_TOPOLOGY_LINELIST_ADJ       = 10,
   VGPU10_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ      = 11,
   VGPU10_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ   = 12,
   VGPU10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ  = 13
} VGPU10_PRIMITIVE_TOPOLOGY;

typedef enum {
   VGPU10_CUSTOMDATA_COMMENT                       = 0,
   VGPU10_CUSTOMDATA_DEBUGINFO                     = 1,
   VGPU10_CUSTOMDATA_OPAQUE                        = 2,
   VGPU10_CUSTOMDATA_DCL_IMMEDIATE_CONSTANT_BUFFER = 3
} VGPU10_CUSTOMDATA_CLASS;

typedef enum {
   VGPU10_RESINFO_RETURN_FLOAT      = 0,
   VGPU10_RESINFO_RETURN_RCPFLOAT   = 1,
   VGPU10_RESINFO_RETURN_UINT       = 2
} VGPU10_RESINFO_RETURN_TYPE;


typedef enum {
   VGPU10_INSTRUCTION_RETURN_FLOAT  = 0,
   VGPU10_INSTRUCTION_RETURN_UINT   = 1
} VGPU10_INSTRUCTION_RETURN_TYPE;


typedef enum {
    VGPU10_TESSELLATOR_DOMAIN_UNDEFINED   = 0,
    VGPU10_TESSELLATOR_DOMAIN_ISOLINE     = 1,
    VGPU10_TESSELLATOR_DOMAIN_TRI         = 2,
    VGPU10_TESSELLATOR_DOMAIN_QUAD        = 3,
    VGPU10_TESSELLATOR_DOMAIN_MAX         = 3
} VGPU10_TESSELLATOR_DOMAIN;


typedef enum {
    VGPU10_TESSELLATOR_PARTITIONING_UNDEFINED         = 0,
    VGPU10_TESSELLATOR_PARTITIONING_INTEGER           = 1,
    VGPU10_TESSELLATOR_PARTITIONING_POW2              = 2,
    VGPU10_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD    = 3,
    VGPU10_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN   = 4,
    VGPU10_TESSELLATOR_PARTITIONING_MAX               = 4
} VGPU10_TESSELLATOR_PARTITIONING;


typedef enum {
    VGPU10_TESSELLATOR_OUTPUT_UNDEFINED      = 0,
    VGPU10_TESSELLATOR_OUTPUT_POINT          = 1,
    VGPU10_TESSELLATOR_OUTPUT_LINE           = 2,
    VGPU10_TESSELLATOR_OUTPUT_TRIANGLE_CW    = 3,
    VGPU10_TESSELLATOR_OUTPUT_TRIANGLE_CCW   = 4,
    VGPU10_TESSELLATOR_OUTPUT_MAX            = 4
} VGPU10_TESSELLATOR_OUTPUT_PRIMITIVE;

typedef union {
   struct {
      unsigned int opcodeType          : 11;
      unsigned int interpolationMode   : 4;
      unsigned int                     : 3;
      unsigned int testBoolean         : 1;
      unsigned int preciseValues       : 4;
      unsigned int                     : 1;
      unsigned int instructionLength   : 7;
      unsigned int extended            : 1;
   };

   struct {
      unsigned int                     : 11;
      unsigned int vmwareOpcodeType    : 4;
   };
   struct {
      unsigned int                     : 11;
      unsigned int resourceDimension   : 5;
      unsigned int sampleCount         : 7;
   };
   struct {
      unsigned int                     : 11;
      unsigned int samplerMode         : 4;
   };
   struct {
      unsigned int                     : 11;
      unsigned int accessPattern       : 1;
   };
   struct {
      unsigned int                     : 11;
      unsigned int primitive           : 6;
   };
   struct {
      unsigned int                     : 11;
      unsigned int primitiveTopology   : 7;
   };
   struct {
      unsigned int                     : 11;
      unsigned int customDataClass     : 21;
   };
   struct {
      unsigned int                     : 11;
      unsigned int resinfoReturnType   : 2;
      unsigned int saturate            : 1;
   };
   struct {
      unsigned int                     : 11;
      unsigned int refactoringAllowed  : 1;


      unsigned int enableDoublePrecisionFloatOps   : 1;
      unsigned int forceEarlyDepthStencil          : 1;
      unsigned int enableRawAndStructuredBuffers   : 1;
   };
   struct {
      unsigned int                     : 11;
      unsigned int instReturnType      : 2;
   };


   struct {
      unsigned int                        : 11;
      unsigned int syncThreadsInGroup     : 1;
      unsigned int syncThreadGroupShared  : 1;
      unsigned int syncUAVMemoryGroup     : 1;
      unsigned int syncUAVMemoryGlobal    : 1;
   };
   struct {
      unsigned int                     : 11;
      unsigned int controlPointCount   : 6;
   };
   struct {
      unsigned int                     : 11;
      unsigned int tessDomain          : 2;
   };
   struct {
      unsigned int                     : 11;
      unsigned int tessPartitioning    : 3;
   };
   struct {
      unsigned int                     : 11;
      unsigned int tessOutputPrimitive : 3;
   };
   struct {
      unsigned int                              : 11;
      unsigned int interfaceIndexedDynamically  : 1;
   };
   struct {
      unsigned int                        : 11;
      unsigned int uavResourceDimension   : 5;
      unsigned int globallyCoherent       : 1;
      unsigned int                        : 6;
      unsigned int uavHasCounter          : 1;
   };
   uint32 value;
} VGPU10OpcodeToken0;


typedef enum {
   VGPU10_EXTENDED_OPCODE_EMPTY                 = 0,
   VGPU10_EXTENDED_OPCODE_SAMPLE_CONTROLS       = 1,


   VGPU10_EXTENDED_OPCODE_RESOURCE_DIM          = 2,
   VGPU10_EXTENDED_OPCODE_RESOURCE_RETURN_TYPE  = 3
} VGPU10_EXTENDED_OPCODE_TYPE;

typedef union {
   struct {
      unsigned int opcodeType : 6;
      unsigned int            : 3;
      unsigned int offsetU    : 4;
      unsigned int offsetV    : 4;
      unsigned int offsetW    : 4;
      unsigned int            : 10;
      unsigned int extended   : 1;
   };


   struct {
      unsigned int                     : 6;
      unsigned int resourceDimension   : 5;
   };
   struct {
      unsigned int                     : 6;
      unsigned int resourceReturnTypeX : 4;
      unsigned int resourceReturnTypeY : 4;
      unsigned int resourceReturnTypeZ : 4;
      unsigned int resourceReturnTypeW : 4;
   };
   uint32 value;
} VGPU10OpcodeToken1;


typedef enum {
   VGPU10_OPERAND_0_COMPONENT = 0,
   VGPU10_OPERAND_1_COMPONENT = 1,
   VGPU10_OPERAND_4_COMPONENT = 2,
   VGPU10_OPERAND_N_COMPONENT = 3
} VGPU10_OPERAND_NUM_COMPONENTS;

typedef enum {
   VGPU10_OPERAND_4_COMPONENT_MASK_MODE = 0,
   VGPU10_OPERAND_4_COMPONENT_SWIZZLE_MODE = 1,
   VGPU10_OPERAND_4_COMPONENT_SELECT_1_MODE = 2
} VGPU10_OPERAND_4_COMPONENT_SELECTION_MODE;

#define VGPU10_OPERAND_4_COMPONENT_MASK_X    0x1
#define VGPU10_OPERAND_4_COMPONENT_MASK_Y    0x2
#define VGPU10_OPERAND_4_COMPONENT_MASK_Z    0x4
#define VGPU10_OPERAND_4_COMPONENT_MASK_W    0x8

#define VGPU10_OPERAND_4_COMPONENT_MASK_XY   (VGPU10_OPERAND_4_COMPONENT_MASK_X   | VGPU10_OPERAND_4_COMPONENT_MASK_Y)
#define VGPU10_OPERAND_4_COMPONENT_MASK_XZ   (VGPU10_OPERAND_4_COMPONENT_MASK_X   | VGPU10_OPERAND_4_COMPONENT_MASK_Z)
#define VGPU10_OPERAND_4_COMPONENT_MASK_XW   (VGPU10_OPERAND_4_COMPONENT_MASK_X   | VGPU10_OPERAND_4_COMPONENT_MASK_W)
#define VGPU10_OPERAND_4_COMPONENT_MASK_YZ   (VGPU10_OPERAND_4_COMPONENT_MASK_Y   | VGPU10_OPERAND_4_COMPONENT_MASK_Z)
#define VGPU10_OPERAND_4_COMPONENT_MASK_YW   (VGPU10_OPERAND_4_COMPONENT_MASK_Y   | VGPU10_OPERAND_4_COMPONENT_MASK_W)
#define VGPU10_OPERAND_4_COMPONENT_MASK_ZW   (VGPU10_OPERAND_4_COMPONENT_MASK_Z   | VGPU10_OPERAND_4_COMPONENT_MASK_W)
#define VGPU10_OPERAND_4_COMPONENT_MASK_XYZ  (VGPU10_OPERAND_4_COMPONENT_MASK_XY  | VGPU10_OPERAND_4_COMPONENT_MASK_Z)
#define VGPU10_OPERAND_4_COMPONENT_MASK_XYW  (VGPU10_OPERAND_4_COMPONENT_MASK_XY  | VGPU10_OPERAND_4_COMPONENT_MASK_W)
#define VGPU10_OPERAND_4_COMPONENT_MASK_XZW  (VGPU10_OPERAND_4_COMPONENT_MASK_XZ  | VGPU10_OPERAND_4_COMPONENT_MASK_W)
#define VGPU10_OPERAND_4_COMPONENT_MASK_YZW  (VGPU10_OPERAND_4_COMPONENT_MASK_YZ  | VGPU10_OPERAND_4_COMPONENT_MASK_W)
#define VGPU10_OPERAND_4_COMPONENT_MASK_XYZW (VGPU10_OPERAND_4_COMPONENT_MASK_XYZ | VGPU10_OPERAND_4_COMPONENT_MASK_W)
#define VGPU10_OPERAND_4_COMPONENT_MASK_ALL  VGPU10_OPERAND_4_COMPONENT_MASK_XYZW

#define VGPU10_REGISTER_INDEX_FROM_SEMANTIC  0xffffffff

typedef enum {
   VGPU10_COMPONENT_X = 0,
   VGPU10_COMPONENT_Y = 1,
   VGPU10_COMPONENT_Z = 2,
   VGPU10_COMPONENT_W = 3
} VGPU10_COMPONENT_NAME;

typedef enum {
   VGPU10_OPERAND_TYPE_TEMP                                 = 0,
   VGPU10_OPERAND_TYPE_INPUT                                = 1,
   VGPU10_OPERAND_TYPE_OUTPUT                               = 2,
   VGPU10_OPERAND_TYPE_INDEXABLE_TEMP                       = 3,
   VGPU10_OPERAND_TYPE_IMMEDIATE32                          = 4,
   VGPU10_OPERAND_TYPE_IMMEDIATE64                          = 5,
   VGPU10_OPERAND_TYPE_SAMPLER                              = 6,
   VGPU10_OPERAND_TYPE_RESOURCE                             = 7,
   VGPU10_OPERAND_TYPE_CONSTANT_BUFFER                      = 8,
   VGPU10_OPERAND_TYPE_IMMEDIATE_CONSTANT_BUFFER            = 9,
   VGPU10_OPERAND_TYPE_LABEL                                = 10,
   VGPU10_OPERAND_TYPE_INPUT_PRIMITIVEID                    = 11,
   VGPU10_OPERAND_TYPE_OUTPUT_DEPTH                         = 12,
   VGPU10_OPERAND_TYPE_NULL                                 = 13,
   VGPU10_OPERAND_TYPE_SM40_MAX                             = 13,


   VGPU10_OPERAND_TYPE_RASTERIZER                           = 14,
   VGPU10_OPERAND_TYPE_OUTPUT_COVERAGE_MASK                 = 15,
   VGPU10_OPERAND_TYPE_SM41_MAX                             = 15,


   VGPU10_OPERAND_TYPE_STREAM                               = 16,
   VGPU10_OPERAND_TYPE_FUNCTION_BODY                        = 17,
   VGPU10_OPERAND_TYPE_FUNCTION_TABLE                       = 18,
   VGPU10_OPERAND_TYPE_INTERFACE                            = 19,
   VGPU10_OPERAND_TYPE_FUNCTION_INPUT                       = 20,
   VGPU10_OPERAND_TYPE_FUNCTION_OUTPUT                      = 21,
   VGPU10_OPERAND_TYPE_OUTPUT_CONTROL_POINT_ID              = 22,
   VGPU10_OPERAND_TYPE_INPUT_FORK_INSTANCE_ID               = 23,
   VGPU10_OPERAND_TYPE_INPUT_JOIN_INSTANCE_ID               = 24,
   VGPU10_OPERAND_TYPE_INPUT_CONTROL_POINT                  = 25,
   VGPU10_OPERAND_TYPE_OUTPUT_CONTROL_POINT                 = 26,
   VGPU10_OPERAND_TYPE_INPUT_PATCH_CONSTANT                 = 27,
   VGPU10_OPERAND_TYPE_INPUT_DOMAIN_POINT                   = 28,
   VGPU10_OPERAND_TYPE_THIS_POINTER                         = 29,
   VGPU10_OPERAND_TYPE_UAV                                  = 30,
   VGPU10_OPERAND_TYPE_THREAD_GROUP_SHARED_MEMORY           = 31,
   VGPU10_OPERAND_TYPE_INPUT_THREAD_ID                      = 32,
   VGPU10_OPERAND_TYPE_INPUT_THREAD_GROUP_ID                = 33,
   VGPU10_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP             = 34,
   VGPU10_OPERAND_TYPE_INPUT_COVERAGE_MASK                  = 35,
   VGPU10_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP_FLATTENED   = 36,
   VGPU10_OPERAND_TYPE_INPUT_GS_INSTANCE_ID                 = 37,
   VGPU10_OPERAND_TYPE_OUTPUT_DEPTH_GREATER_EQUAL           = 38,
   VGPU10_OPERAND_TYPE_OUTPUT_DEPTH_LESS_EQUAL              = 39,
   VGPU10_OPERAND_TYPE_CYCLE_COUNTER                        = 40,
   VGPU10_OPERAND_TYPE_SM50_MAX                             = 40,

   VGPU10_NUM_OPERANDS
} VGPU10_OPERAND_TYPE;

typedef enum {
   VGPU10_OPERAND_INDEX_0D = 0,
   VGPU10_OPERAND_INDEX_1D = 1,
   VGPU10_OPERAND_INDEX_2D = 2,
   VGPU10_OPERAND_INDEX_3D = 3
} VGPU10_OPERAND_INDEX_DIMENSION;

typedef enum {
   VGPU10_OPERAND_INDEX_IMMEDIATE32 = 0,
   VGPU10_OPERAND_INDEX_IMMEDIATE64 = 1,
   VGPU10_OPERAND_INDEX_RELATIVE = 2,
   VGPU10_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE = 3,
   VGPU10_OPERAND_INDEX_IMMEDIATE64_PLUS_RELATIVE = 4
} VGPU10_OPERAND_INDEX_REPRESENTATION;

typedef union {
   struct {
      unsigned int numComponents          : 2;
      unsigned int selectionMode          : 2;
      unsigned int mask                   : 4;
      unsigned int                        : 4;
      unsigned int operandType            : 8;
      unsigned int indexDimension         : 2;
      unsigned int index0Representation   : 3;
      unsigned int index1Representation   : 3;
      unsigned int                        : 3;
      unsigned int extended               : 1;
   };
   struct {
      unsigned int                        : 4;
      unsigned int swizzleX               : 2;
      unsigned int swizzleY               : 2;
      unsigned int swizzleZ               : 2;
      unsigned int swizzleW               : 2;
   };
   struct {
      unsigned int                        : 4;
      unsigned int selectMask             : 2;
   };
   uint32 value;
} VGPU10OperandToken0;


typedef enum {
   VGPU10_EXTENDED_OPERAND_EMPTY = 0,
   VGPU10_EXTENDED_OPERAND_MODIFIER = 1
} VGPU10_EXTENDED_OPERAND_TYPE;

typedef enum {
   VGPU10_OPERAND_MODIFIER_NONE = 0,
   VGPU10_OPERAND_MODIFIER_NEG = 1,
   VGPU10_OPERAND_MODIFIER_ABS = 2,
   VGPU10_OPERAND_MODIFIER_ABSNEG = 3
} VGPU10_OPERAND_MODIFIER;

typedef union {
   struct {
      unsigned int extendedOperandType : 6;
      unsigned int operandModifier     : 8;
      unsigned int                     : 17;
      unsigned int extended            : 1;
   };
   uint32 value;
} VGPU10OperandToken1;


typedef enum {
   VGPU10_RETURN_TYPE_MIN     = 1,

   VGPU10_RETURN_TYPE_UNORM   = 1,
   VGPU10_RETURN_TYPE_SNORM   = 2,
   VGPU10_RETURN_TYPE_SINT    = 3,
   VGPU10_RETURN_TYPE_UINT    = 4,
   VGPU10_RETURN_TYPE_FLOAT   = 5,
   VGPU10_RETURN_TYPE_MIXED   = 6,
   VGPU10_RETURN_TYPE_SM40_MAX = 6,


   VGPU10_RETURN_TYPE_DOUBLE     = 7,
   VGPU10_RETURN_TYPE_CONTINUED  = 8,
   VGPU10_RETURN_TYPE_UNUSED     = 9,

   VGPU10_RETURN_TYPE_MAX        = 9
} VGPU10_RESOURCE_RETURN_TYPE;

typedef union {
   struct {
      unsigned int component0 : 4;
      unsigned int component1 : 4;
      unsigned int component2 : 4;
      unsigned int component3 : 4;
   };
   uint32 value;
} VGPU10ResourceReturnTypeToken;


typedef enum {
   VGPU10_NAME_MIN                        = 0,

   VGPU10_NAME_UNDEFINED                  = 0,
   VGPU10_NAME_POSITION                   = 1,
   VGPU10_NAME_CLIP_DISTANCE              = 2,
   VGPU10_NAME_CULL_DISTANCE              = 3,
   VGPU10_NAME_RENDER_TARGET_ARRAY_INDEX  = 4,
   VGPU10_NAME_VIEWPORT_ARRAY_INDEX       = 5,
   VGPU10_NAME_VERTEX_ID                  = 6,
   VGPU10_NAME_PRIMITIVE_ID               = 7,
   VGPU10_NAME_INSTANCE_ID                = 8,
   VGPU10_NAME_IS_FRONT_FACE              = 9,
   VGPU10_NAME_SAMPLE_INDEX               = 10,
   VGPU10_NAME_SM40_MAX                   = 10,


   VGPU10_NAME_FINAL_QUAD_U_EQ_0_EDGE_TESSFACTOR   = 11,
   VGPU10_NAME_FINAL_QUAD_V_EQ_0_EDGE_TESSFACTOR   = 12,
   VGPU10_NAME_FINAL_QUAD_U_EQ_1_EDGE_TESSFACTOR   = 13,
   VGPU10_NAME_FINAL_QUAD_V_EQ_1_EDGE_TESSFACTOR   = 14,
   VGPU10_NAME_FINAL_QUAD_U_INSIDE_TESSFACTOR      = 15,
   VGPU10_NAME_FINAL_QUAD_V_INSIDE_TESSFACTOR      = 16,
   VGPU10_NAME_FINAL_TRI_U_EQ_0_EDGE_TESSFACTOR    = 17,
   VGPU10_NAME_FINAL_TRI_V_EQ_0_EDGE_TESSFACTOR    = 18,
   VGPU10_NAME_FINAL_TRI_W_EQ_0_EDGE_TESSFACTOR    = 19,
   VGPU10_NAME_FINAL_TRI_INSIDE_TESSFACTOR         = 20,
   VGPU10_NAME_FINAL_LINE_DETAIL_TESSFACTOR        = 21,
   VGPU10_NAME_FINAL_LINE_DENSITY_TESSFACTOR       = 22,

   VGPU10_NAME_MAX                                 = 22
} VGPU10_SYSTEM_NAME;

typedef union {
   struct {
      unsigned int name : 16;
   };
   uint32 value;
} VGPU10NameToken;

#endif
