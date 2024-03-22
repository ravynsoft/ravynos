/**********************************************************
 * Copyright 2007-2014 VMware, Inc.  All rights reserved.
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
 **********************************************************/

/*
 * svga3d_shaderdefs.h --
 *
 * SVGA3D byte code format and limit definitions.
 *
 * The format of the byte code directly corresponds to that defined
 * by Microsoft DirectX SDK 9.0c (file d3d9types.h). The format can
 * also be extended so that different shader formats can be supported
 * for example GLSL, ARB vp/fp, NV/ATI shader formats, etc.
 *
 */

#ifndef __SVGA3D_SHADER_DEFS__
#define __SVGA3D_SHADER_DEFS__

/* SVGA3D shader hardware limits. */

#define SVGA3D_INPUTREG_MAX            16
#define SVGA3D_OUTPUTREG_MAX           12
#define SVGA3D_VERTEX_SAMPLERREG_MAX   4
#define SVGA3D_PIXEL_SAMPLERREG_MAX    16
#define SVGA3D_SAMPLERREG_MAX          (SVGA3D_PIXEL_SAMPLERREG_MAX+\
                                        SVGA3D_VERTEX_SAMPLERREG_MAX)
#define SVGA3D_TEMPREG_MAX             32
#define SVGA3D_CONSTREG_MAX            256
#define SVGA3D_CONSTINTREG_MAX         16
#define SVGA3D_CONSTBOOLREG_MAX        16
#define SVGA3D_ADDRREG_MAX             1
#define SVGA3D_PREDREG_MAX             1

/* SVGA3D byte code specific limits */

#define SVGA3D_MAX_SRC_REGS      4
#define SVGA3D_MAX_NESTING_LEVEL 32

/* SVGA3D version information. */

#define SVGA3D_VS_TYPE  0xFFFE
#define SVGA3D_PS_TYPE  0xFFFF

typedef struct {
   union {
      struct {
         uint32 minor : 8;
         uint32 major : 8;
         uint32 type : 16;
      };

      uint32 value;
   };
} SVGA3dShaderVersion;

#define SVGA3D_VS_10 ((SVGA3D_VS_TYPE << 16) | 1 << 8)
#define SVGA3D_VS_11 (SVGA3D_VS_10 | 1)
#define SVGA3D_VS_20 ((SVGA3D_VS_TYPE << 16) | 2 << 8)
#define SVGA3D_VS_21 (SVGA3D_VS_20 | 1)
#define SVGA3D_VS_30 ((SVGA3D_VS_TYPE << 16) | 3 << 8)

#define SVGA3D_PS_10 ((SVGA3D_PS_TYPE << 16) | 1 << 8)
#define SVGA3D_PS_11 (SVGA3D_PS_10 | 1)
#define SVGA3D_PS_12 (SVGA3D_PS_10 | 2)
#define SVGA3D_PS_13 (SVGA3D_PS_10 | 3)
#define SVGA3D_PS_14 (SVGA3D_PS_10 | 4)
#define SVGA3D_PS_20 ((SVGA3D_PS_TYPE << 16) | 2 << 8)
#define SVGA3D_PS_21 (SVGA3D_PS_20 | 1)
#define SVGA3D_PS_30 ((SVGA3D_PS_TYPE << 16) | 3 << 8)

/* The *_ENABLED are for backwards compatibility with old drivers */
typedef enum {
   SVGA3DPSVERSION_NONE = 0,
   SVGA3DPSVERSION_ENABLED = 1,
   SVGA3DPSVERSION_11 = 3,
   SVGA3DPSVERSION_12 = 5,
   SVGA3DPSVERSION_13 = 7,
   SVGA3DPSVERSION_14 = 9,
   SVGA3DPSVERSION_20 = 11,
   SVGA3DPSVERSION_30 = 13,
   SVGA3DPSVERSION_40 = 15,
   SVGA3DPSVERSION_MAX
} SVGA3dPixelShaderVersion;

typedef enum {
   SVGA3DVSVERSION_NONE = 0,
   SVGA3DVSVERSION_ENABLED = 1,
   SVGA3DVSVERSION_11 = 3,
   SVGA3DVSVERSION_20 = 5,
   SVGA3DVSVERSION_30 = 7,
   SVGA3DVSVERSION_40 = 9,
   SVGA3DVSVERSION_MAX
} SVGA3dVertexShaderVersion;

/* SVGA3D instruction op codes. */

typedef enum {
   SVGA3DOP_NOP = 0,
   SVGA3DOP_MOV,
   SVGA3DOP_ADD,
   SVGA3DOP_SUB,
   SVGA3DOP_MAD,
   SVGA3DOP_MUL,
   SVGA3DOP_RCP,
   SVGA3DOP_RSQ,
   SVGA3DOP_DP3,
   SVGA3DOP_DP4,
   SVGA3DOP_MIN,
   SVGA3DOP_MAX,
   SVGA3DOP_SLT,
   SVGA3DOP_SGE,
   SVGA3DOP_EXP,
   SVGA3DOP_LOG,
   SVGA3DOP_LIT,
   SVGA3DOP_DST,
   SVGA3DOP_LRP,
   SVGA3DOP_FRC,
   SVGA3DOP_M4x4,
   SVGA3DOP_M4x3,
   SVGA3DOP_M3x4,
   SVGA3DOP_M3x3,
   SVGA3DOP_M3x2,
   SVGA3DOP_CALL,
   SVGA3DOP_CALLNZ,
   SVGA3DOP_LOOP,
   SVGA3DOP_RET,
   SVGA3DOP_ENDLOOP,
   SVGA3DOP_LABEL,
   SVGA3DOP_DCL,
   SVGA3DOP_POW,
   SVGA3DOP_CRS,
   SVGA3DOP_SGN,
   SVGA3DOP_ABS,
   SVGA3DOP_NRM,
   SVGA3DOP_SINCOS,
   SVGA3DOP_REP,
   SVGA3DOP_ENDREP,
   SVGA3DOP_IF,
   SVGA3DOP_IFC,
   SVGA3DOP_ELSE,
   SVGA3DOP_ENDIF,
   SVGA3DOP_BREAK,
   SVGA3DOP_BREAKC,
   SVGA3DOP_MOVA,
   SVGA3DOP_DEFB,
   SVGA3DOP_DEFI,
   SVGA3DOP_TEXCOORD = 64,
   SVGA3DOP_TEXKILL,
   SVGA3DOP_TEX,
   SVGA3DOP_TEXBEM,
   SVGA3DOP_TEXBEML,
   SVGA3DOP_TEXREG2AR,
   SVGA3DOP_TEXREG2GB = 70,
   SVGA3DOP_TEXM3x2PAD,
   SVGA3DOP_TEXM3x2TEX,
   SVGA3DOP_TEXM3x3PAD,
   SVGA3DOP_TEXM3x3TEX,
   SVGA3DOP_RESERVED0,
   SVGA3DOP_TEXM3x3SPEC,
   SVGA3DOP_TEXM3x3VSPEC,
   SVGA3DOP_EXPP,
   SVGA3DOP_LOGP,
   SVGA3DOP_CND = 80,
   SVGA3DOP_DEF,
   SVGA3DOP_TEXREG2RGB,
   SVGA3DOP_TEXDP3TEX,
   SVGA3DOP_TEXM3x2DEPTH,
   SVGA3DOP_TEXDP3,
   SVGA3DOP_TEXM3x3,
   SVGA3DOP_TEXDEPTH,
   SVGA3DOP_CMP,
   SVGA3DOP_BEM,
   SVGA3DOP_DP2ADD = 90,
   SVGA3DOP_DSX,
   SVGA3DOP_DSY,
   SVGA3DOP_TEXLDD,
   SVGA3DOP_SETP,
   SVGA3DOP_TEXLDL,
   SVGA3DOP_BREAKP = 96,
   SVGA3DOP_LAST_INST,
   SVGA3DOP_PHASE = 0xFFFD,
   SVGA3DOP_COMMENT = 0xFFFE,
   SVGA3DOP_END = 0xFFFF,
} SVGA3dShaderOpCodeType;

/* SVGA3D operation control/comparison function types */

typedef enum {
   SVGA3DOPCONT_NONE,
   SVGA3DOPCONT_PROJECT,   /* Projective texturing */
   SVGA3DOPCONT_BIAS,      /* Texturing with a LOD bias */
} SVGA3dShaderOpCodeControlFnType;

typedef enum {
   SVGA3DOPCOMP_RESERVED0 = 0,
   SVGA3DOPCOMP_GT,
   SVGA3DOPCOMP_EQ,
   SVGA3DOPCOMP_GE,
   SVGA3DOPCOMP_LT,
   SVGA3DOPCOMPC_NE,
   SVGA3DOPCOMP_LE,
   SVGA3DOPCOMP_RESERVED1
} SVGA3dShaderOpCodeCompFnType;

/* SVGA3D register types */

typedef enum {
    SVGA3DREG_TEMP = 0,       /* Temporary register file */
    SVGA3DREG_INPUT,          /* Input register file */
    SVGA3DREG_CONST,          /* Constant register file */
    SVGA3DREG_ADDR,           /* Address register for VS */
    SVGA3DREG_TEXTURE = 3,    /* Texture register file for PS */
    SVGA3DREG_RASTOUT,        /* Rasterizer register file */
    SVGA3DREG_ATTROUT,        /* Attribute output register file */
    SVGA3DREG_TEXCRDOUT,      /* Texture coordinate output register file */
    SVGA3DREG_OUTPUT = 6,     /* Output register file for VS 3.0+ */
    SVGA3DREG_CONSTINT,       /* Constant integer vector register file */
    SVGA3DREG_COLOROUT,       /* Color output register file */
    SVGA3DREG_DEPTHOUT,       /* Depth output register file */
    SVGA3DREG_SAMPLER,        /* Sampler state register file */
    SVGA3DREG_CONST2,         /* Constant register file 2048 - 4095 */
    SVGA3DREG_CONST3,         /* Constant register file 4096 - 6143 */
    SVGA3DREG_CONST4,         /* Constant register file 6144 - 8191 */
    SVGA3DREG_CONSTBOOL,      /* Constant boolean register file */
    SVGA3DREG_LOOP,           /* Loop counter register file */
    SVGA3DREG_TEMPFLOAT16,    /* 16-bit float temp register file */
    SVGA3DREG_MISCTYPE,       /* Miscellaneous (single) registers */
    SVGA3DREG_LABEL,          /* Label */
    SVGA3DREG_PREDICATE,      /* Predicate register */
} SVGA3dShaderRegType;

/* SVGA3D rasterizer output register types */

typedef enum {
   SVGA3DRASTOUT_POSITION = 0,
   SVGA3DRASTOUT_FOG,
   SVGA3DRASTOUT_PSIZE
} SVGA3dShaderRastOutRegType;

/* SVGA3D miscellaneous register types */

typedef enum {
   SVGA3DMISCREG_POSITION = 0,   /* Input position x,y,z,rhw (PS) */
   SVGA3DMISCREG_FACE            /* Floating point primitive area (PS) */
} SVGA3DShaderMiscRegType;

/* SVGA3D sampler types */

typedef enum {
   SVGA3DSAMP_UNKNOWN = 0, /* Uninitialized value */
   SVGA3DSAMP_2D = 2,      /* dcl_2d s# (for declaring a 2D texture) */
   SVGA3DSAMP_CUBE,        /* dcl_cube s# (for declaring a cube texture) */
   SVGA3DSAMP_VOLUME,      /* dcl_volume s# (for declaring a volume texture) */
   SVGA3DSAMP_2D_SHADOW,   /* dcl_2d s# (for declaring a 2D shadow texture) */
   SVGA3DSAMP_MAX,
} SVGA3dShaderSamplerType;

/* SVGA3D write mask */

#define SVGA3DWRITEMASK_0    1 /* Component 0 (X;Red) */
#define SVGA3DWRITEMASK_1    2 /* Component 1 (Y;Green) */
#define SVGA3DWRITEMASK_2    4 /* Component 2 (Z;Blue) */
#define SVGA3DWRITEMASK_3    8 /* Component 3 (W;Alpha) */
#define SVGA3DWRITEMASK_ALL 15 /* All components */

/* SVGA3D destination modifiers */

#define SVGA3DDSTMOD_NONE              0 /* nop */
#define SVGA3DDSTMOD_SATURATE          1 /* clamp to [0, 1] */
#define SVGA3DDSTMOD_PARTIALPRECISION  2 /* Partial precision hint */

/*
 * Relevant to multisampling only:
 * When the pixel center is not covered, sample
 * attribute or compute gradients/LOD
 * using multisample "centroid" location.
 * "Centroid" is some location within the covered
 * region of the pixel.
 */

#define SVGA3DDSTMOD_MSAMPCENTROID     4

/* SVGA3D destination shift scale */

typedef enum {
   SVGA3DDSTSHFSCALE_X1 = 0,  /* 1.0 */
   SVGA3DDSTSHFSCALE_X2 = 1,  /* 2.0 */
   SVGA3DDSTSHFSCALE_X4 = 2,  /* 4.0 */
   SVGA3DDSTSHFSCALE_X8 = 3,  /* 8.0 */
   SVGA3DDSTSHFSCALE_D8 = 13, /* 0.125 */
   SVGA3DDSTSHFSCALE_D4 = 14, /* 0.25 */
   SVGA3DDSTSHFSCALE_D2 = 15  /* 0.5 */
} SVGA3dShaderDstShfScaleType;

/* SVGA3D source swizzle */

#define SVGA3DSWIZZLE_REPLICATEX 0x00
#define SVGA3DSWIZZLE_REPLICATEY 0x55
#define SVGA3DSWIZZLE_REPLICATEZ 0xAA
#define SVGA3DSWIZZLE_REPLICATEW 0xFF
#define SVGA3DSWIZZLE_NONE       0xE4
#define SVGA3DSWIZZLE_YZXW       0xC9
#define SVGA3DSWIZZLE_ZXYW       0xD2
#define SVGA3DSWIZZLE_WXYZ       0x1B

/* SVGA3D source modifiers */

typedef enum {
    SVGA3DSRCMOD_NONE = 0, /* nop */
    SVGA3DSRCMOD_NEG,      /* negate */
    SVGA3DSRCMOD_BIAS,     /* bias */
    SVGA3DSRCMOD_BIASNEG,  /* bias and negate */
    SVGA3DSRCMOD_SIGN,     /* sign */
    SVGA3DSRCMOD_SIGNNEG,  /* sign and negate */
    SVGA3DSRCMOD_COMP,     /* complement */
    SVGA3DSRCMOD_X2,       /* x2 */
    SVGA3DSRCMOD_X2NEG,    /* x2 and negate */
    SVGA3DSRCMOD_DZ,       /* divide through by z component */
    SVGA3DSRCMOD_DW,       /* divide through by w component */
    SVGA3DSRCMOD_ABS,      /* abs() */
    SVGA3DSRCMOD_ABSNEG,   /* -abs() */
    SVGA3DSRCMOD_NOT,      /* ! (for predicate register) */
} SVGA3dShaderSrcModType;

/* SVGA3D instruction token */

typedef struct {
   union {
      struct {
         uint32 comment_op : 16;
         uint32 comment_size : 16;
      };

      struct {
         uint32 op : 16;
         uint32 control : 3;
         uint32 reserved2 : 5;
         uint32 size : 4;
         uint32 predicated : 1;
         uint32 reserved1 : 1;
         uint32 coissue : 1;
         uint32 reserved0 : 1;
      };

      uint32 value;
   };
} SVGA3dShaderInstToken;

/* SVGA3D destination parameter token */

typedef struct {
   union {
      struct {
         uint32 num : 11;
         uint32 type_upper : 2;
         uint32 relAddr : 1;
         uint32 reserved1 : 2;
         uint32 mask : 4;
         uint32 dstMod : 4;
         uint32 shfScale : 4;
         uint32 type_lower : 3;
         uint32 reserved0 : 1;
      };

      uint32 value;
   };
} SVGA3dShaderDestToken;

/* SVGA3D source parameter token */

typedef struct {
   union {
      struct {
         uint32 num : 11;
         uint32 type_upper : 2;
         uint32 relAddr : 1;
         uint32 reserved1 : 2;
         uint32 swizzle : 8;
         uint32 srcMod : 4;
         uint32 type_lower : 3;
         uint32 reserved0 : 1;
      };

      uint32 value;
   };
} SVGA3dShaderSrcToken;

/* SVGA3DOP_DCL parameter tokens */

typedef struct {
   union {
      struct {
         union {
            struct {
               uint32 usage : 5;
               uint32 reserved1 : 11;
               uint32 index : 4;
               uint32 reserved0 : 12;
            }; /* input / output declaration */

            struct {
               uint32 reserved3 : 27;
               uint32 type : 4;
               uint32 reserved2 : 1;
            }; /* sampler declaration */
         };

         SVGA3dShaderDestToken dst;
      };

      uint32 values[2];
   };
} SVGA3DOpDclArgs;

/* SVGA3DOP_DEF parameter tokens */

typedef struct {
   union {
      struct {
         SVGA3dShaderDestToken dst;

         union {
            float constValues[4];
            int constIValues[4];
            Bool constBValue;
         };
      };

      uint32 values[5];
   };
} SVGA3DOpDefArgs;

/* SVGA3D shader token */

typedef union {
   uint32 value;
   SVGA3dShaderInstToken inst;
   SVGA3dShaderDestToken dest;
   SVGA3dShaderSrcToken src;
} SVGA3dShaderToken;

/* SVGA3D shader program */

typedef struct {
   SVGA3dShaderVersion version;
   /* SVGA3dShaderToken stream */
} SVGA3dShaderProgram;

/* SVGA3D version specific register assignments */

static const uint32 SVGA3D_INPUT_REG_POSITION_VS11 = 0;
static const uint32 SVGA3D_INPUT_REG_PSIZE_VS11 = 1;
static const uint32 SVGA3D_INPUT_REG_FOG_VS11 = 3;
static const uint32 SVGA3D_INPUT_REG_FOG_MASK_VS11 = SVGA3DWRITEMASK_3;
static const uint32 SVGA3D_INPUT_REG_COLOR_BASE_VS11 = 2;
static const uint32 SVGA3D_INPUT_REG_TEXCOORD_BASE_VS11 = 4;

static const uint32 SVGA3D_INPUT_REG_COLOR_BASE_PS11 = 0;
static const uint32 SVGA3D_INPUT_REG_TEXCOORD_BASE_PS11 = 2;
static const uint32 SVGA3D_OUTPUT_REG_DEPTH_PS11 = 0;
static const uint32 SVGA3D_OUTPUT_REG_COLOR_PS11 = 1;

static const uint32 SVGA3D_INPUT_REG_COLOR_BASE_PS20 = 0;
static const uint32 SVGA3D_INPUT_REG_COLOR_NUM_PS20 = 2;
static const uint32 SVGA3D_INPUT_REG_TEXCOORD_BASE_PS20 = 2;
static const uint32 SVGA3D_INPUT_REG_TEXCOORD_NUM_PS20 = 8;
static const uint32 SVGA3D_OUTPUT_REG_COLOR_BASE_PS20 = 1;
static const uint32 SVGA3D_OUTPUT_REG_COLOR_NUM_PS20 = 4;
static const uint32 SVGA3D_OUTPUT_REG_DEPTH_BASE_PS20 = 0;
static const uint32 SVGA3D_OUTPUT_REG_DEPTH_NUM_PS20 = 1;

/*
 *----------------------------------------------------------------------
 *
 * SVGA3dShaderGetRegType --
 *
 *      As the register type is split into two non sequential fields,
 *      this function provides an useful way of accessing the actual
 *      register type without having to manually concatenate the
 *      type_upper and type_lower fields.
 *
 * Results:
 *      Returns the register type.
 *
 *----------------------------------------------------------------------
 */

static inline SVGA3dShaderRegType
SVGA3dShaderGetRegType(uint32 token)
{
   SVGA3dShaderSrcToken src;
   src.value = token;
   return (SVGA3dShaderRegType)(src.type_upper << 3 | src.type_lower);
}

#endif /* __SVGA3D_SHADER_DEFS__ */
