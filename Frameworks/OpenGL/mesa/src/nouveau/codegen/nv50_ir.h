/*
 * Copyright 2011 Christoph Bumiller
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __NV50_IR_H__
#define __NV50_IR_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <deque>
#include <list>
#include <unordered_set>
#include <vector>

#include "nv50_ir_util.h"
#include "nv50_ir_graph.h"

#include "nv50_ir_driver.h"

namespace nv50_ir {

enum operation
{
   OP_NOP = 0,
   OP_PHI,
   OP_UNION, // unify a new definition and several source values
   OP_SPLIT, // $r0d -> { $r0, $r1 } ($r0d and $r0/$r1 will be coalesced)
   OP_MERGE, // opposite of split, e.g. combine 2 32 bit into a 64 bit value
   OP_MOV, // simple copy, no modifiers allowed
   OP_LOAD,
   OP_STORE,
   OP_ADD, // NOTE: add u64 + u32 is legal for targets w/o 64-bit integer adds
   OP_SUB,
   OP_MUL,
   OP_DIV,
   OP_MOD,
   OP_MAD,
   OP_FMA,
   OP_SAD, // abs(src0 - src1) + src2
   OP_SHLADD,
   // extended multiply-add (GM107+), does a lot of things.
   // see envytools for detailed documentation
   OP_XMAD,
   OP_ABS,
   OP_NEG,
   OP_NOT,
   OP_AND,
   OP_OR,
   OP_XOR,
   OP_LOP3_LUT,
   OP_SHL,
   OP_SHR,
   OP_SHF,
   OP_MAX,
   OP_MIN,
   OP_SAT, // CLAMP(f32, 0.0, 1.0)
   OP_CEIL,
   OP_FLOOR,
   OP_TRUNC,
   OP_CVT,
   OP_SET_AND, // dst = (src0 CMP src1) & src2
   OP_SET_OR,
   OP_SET_XOR,
   OP_SET,
   OP_SELP, // dst = src2 ? src0 : src1
   OP_SLCT, // dst = (src2 CMP 0) ? src0 : src1
   OP_RCP,
   OP_RSQ,
   OP_LG2,
   OP_SIN,
   OP_COS,
   OP_EX2,
   OP_PRESIN,
   OP_PREEX2,
   OP_SQRT,
   OP_BRA,
   OP_CALL,
   OP_RET,
   OP_CONT,
   OP_BREAK,
   OP_PRERET,
   OP_PRECONT,
   OP_PREBREAK,
   OP_BRKPT,     // breakpoint (not related to loops)
   OP_JOINAT,    // push control flow convergence point
   OP_JOIN,      // converge
   OP_DISCARD,
   OP_EXIT,
   OP_MEMBAR, // memory barrier (mfence, lfence, sfence)
   OP_VFETCH, // indirection 0 in attribute space, indirection 1 is vertex base
   OP_PFETCH, // fetch base address of vertex src0 (immediate) [+ src1]
   OP_AFETCH, // fetch base address of shader input (a[%r1+0x10])
   OP_EXPORT,
   OP_LINTERP,
   OP_PINTERP,
   OP_EMIT,    // emit vertex
   OP_RESTART, // restart primitive
   OP_FINAL, // finish emitting primitives
   OP_TEX,
   OP_TXB, // texture bias
   OP_TXL, // texture lod
   OP_TXF, // texel fetch
   OP_TXQ, // texture size query
   OP_TXD, // texture derivatives
   OP_TXG, // texture gather
   OP_TXLQ, // texture query lod
   OP_TEXCSAA, // texture op for coverage sampling
   OP_TEXPREP, // turn cube map array into 2d array coordinates
   OP_SULDB, // surface load (raw)
   OP_SULDP, // surface load (formatted)
   OP_SUSTB, // surface store (raw)
   OP_SUSTP, // surface store (formatted)
   OP_SUREDB,
   OP_SUREDP, // surface reduction (atomic op)
   OP_SULEA,   // surface load effective address
   OP_SUBFM,   // surface bitfield manipulation
   OP_SUCLAMP, // clamp surface coordinates
   OP_SUEAU,   // surface effective address
   OP_SUQ,     // surface query
   OP_MADSP,   // special integer multiply-add
   OP_TEXBAR, // texture dependency barrier
   OP_DFDX,
   OP_DFDY,
   OP_RDSV, // read system value
   OP_PIXLD, // get info about raster object or surfaces
   OP_QUADOP,
   OP_QUADON,
   OP_QUADPOP,
   OP_POPCNT, // bitcount(src0 & src1)
   OP_INSBF,  // insert first src1[8:15] bits of src0 into src2 at src1[0:7]
   OP_EXTBF,  // place bits [K,K+N) of src0 into dst, src1 = 0xNNKK
   OP_BFIND,  // find highest/lowest set bit
   OP_BREV,   // bitfield reverse
   OP_BMSK,   // bitfield mask
   OP_PERMT,  // dst = bytes from src2,src0 selected by src1 (nvc0's src order)
   OP_SGXT,
   OP_ATOM,
   OP_BAR,    // execution barrier, sources = { id, thread count, predicate }
   OP_VADD,   // byte/word vector operations
   OP_VAVG,
   OP_VMIN,
   OP_VMAX,
   OP_VSAD,
   OP_VSET,
   OP_VSHR,
   OP_VSHL,
   OP_VSEL,
   OP_CCTL, // cache control
   OP_SHFL, // warp shuffle
   OP_VOTE,
   OP_BUFQ, // buffer query
   OP_WARPSYNC,
   OP_LAST
};

// various instruction-specific modifier definitions Instruction::subOp
// MOV_FINAL marks a MOV originating from an EXPORT (used for placing TEXBARs)
#define NV50_IR_SUBOP_MUL_HIGH     1
#define NV50_IR_SUBOP_EMIT_RESTART 1
#define NV50_IR_SUBOP_LDC_IL       1
#define NV50_IR_SUBOP_LDC_IS       2
#define NV50_IR_SUBOP_LDC_ISL      3
#define NV50_IR_SUBOP_SHIFT_WRAP   1
#define NV50_IR_SUBOP_SHIFT_HIGH   2
#define NV50_IR_SUBOP_EMU_PRERET   1
#define NV50_IR_SUBOP_TEXBAR(n)    n
#define NV50_IR_SUBOP_MOV_FINAL    1
#define NV50_IR_SUBOP_EXTBF_REV    1
#define NV50_IR_SUBOP_BFIND_SAMT   1
#define NV50_IR_SUBOP_RCPRSQ_64H   1
#define NV50_IR_SUBOP_PERMT_F4E    1
#define NV50_IR_SUBOP_PERMT_B4E    2
#define NV50_IR_SUBOP_PERMT_RC8    3
#define NV50_IR_SUBOP_PERMT_ECL    4
#define NV50_IR_SUBOP_PERMT_ECR    5
#define NV50_IR_SUBOP_PERMT_RC16   6
#define NV50_IR_SUBOP_BAR_SYNC     0
#define NV50_IR_SUBOP_BAR_ARRIVE   1
#define NV50_IR_SUBOP_BAR_RED_AND  2
#define NV50_IR_SUBOP_BAR_RED_OR   3
#define NV50_IR_SUBOP_BAR_RED_POPC 4
#define NV50_IR_SUBOP_MEMBAR_L     1
#define NV50_IR_SUBOP_MEMBAR_S     2
#define NV50_IR_SUBOP_MEMBAR_M     3
#define NV50_IR_SUBOP_MEMBAR_CTA  (0 << 2)
#define NV50_IR_SUBOP_MEMBAR_GL   (1 << 2)
#define NV50_IR_SUBOP_MEMBAR_SYS  (2 << 2)
#define NV50_IR_SUBOP_MEMBAR_DIR(m)   ((m) & 0x3)
#define NV50_IR_SUBOP_MEMBAR_SCOPE(m) ((m) & ~0x3)
#define NV50_IR_SUBOP_MEMBAR(d,s) \
   (NV50_IR_SUBOP_MEMBAR_##d | NV50_IR_SUBOP_MEMBAR_##s)
#define NV50_IR_SUBOP_ATOM_ADD      0
#define NV50_IR_SUBOP_ATOM_MIN      1
#define NV50_IR_SUBOP_ATOM_MAX      2
#define NV50_IR_SUBOP_ATOM_INC      3
#define NV50_IR_SUBOP_ATOM_DEC      4
#define NV50_IR_SUBOP_ATOM_AND      5
#define NV50_IR_SUBOP_ATOM_OR       6
#define NV50_IR_SUBOP_ATOM_XOR      7
#define NV50_IR_SUBOP_ATOM_CAS      8
#define NV50_IR_SUBOP_ATOM_EXCH     9
#define NV50_IR_SUBOP_CCTL_IV      5
#define NV50_IR_SUBOP_CCTL_IVALL   6
#define NV50_IR_SUBOP_SUST_IGN     0
#define NV50_IR_SUBOP_SUST_TRAP    1
#define NV50_IR_SUBOP_SUST_SDCL    3
#define NV50_IR_SUBOP_SULD_ZERO    0
#define NV50_IR_SUBOP_SULD_TRAP    1
#define NV50_IR_SUBOP_SULD_SDCL    3
#define NV50_IR_SUBOP_SUBFM_3D     1
#define NV50_IR_SUBOP_SUCLAMP_2D   0x10
#define NV50_IR_SUBOP_SUCLAMP_SD(r, d) (( 0 + (r)) | ((d == 2) ? 0x10 : 0))
#define NV50_IR_SUBOP_SUCLAMP_PL(r, d) (( 5 + (r)) | ((d == 2) ? 0x10 : 0))
#define NV50_IR_SUBOP_SUCLAMP_BL(r, d) ((10 + (r)) | ((d == 2) ? 0x10 : 0))
#define NV50_IR_SUBOP_PIXLD_COUNT       0
#define NV50_IR_SUBOP_PIXLD_COVMASK     1
#define NV50_IR_SUBOP_PIXLD_COVERED     2
#define NV50_IR_SUBOP_PIXLD_OFFSET      3
#define NV50_IR_SUBOP_PIXLD_CENT_OFFSET 4
#define NV50_IR_SUBOP_PIXLD_SAMPLEID    5
#define NV50_IR_SUBOP_SHFL_IDX  0
#define NV50_IR_SUBOP_SHFL_UP   1
#define NV50_IR_SUBOP_SHFL_DOWN 2
#define NV50_IR_SUBOP_SHFL_BFLY 3
#define NV50_IR_SUBOP_LOAD_LOCKED    1
#define NV50_IR_SUBOP_STORE_UNLOCKED 2
#define NV50_IR_SUBOP_MADSP_SD     0xffff
// Yes, we could represent those with DataType.
// Or put the type into operation and have a couple 1000 values in that enum.
// This will have to do for now.
// The bitfields are supposed to correspond to nve4 ISA.
#define NV50_IR_SUBOP_MADSP(a,b,c) (((c) << 8) | ((b) << 4) | (a))
#define NV50_IR_SUBOP_V1(d,a,b)    (((d) << 10) | ((b) << 5) | (a) | 0x0000)
#define NV50_IR_SUBOP_V2(d,a,b)    (((d) << 10) | ((b) << 5) | (a) | 0x4000)
#define NV50_IR_SUBOP_V4(d,a,b)    (((d) << 10) | ((b) << 5) | (a) | 0x8000)
#define NV50_IR_SUBOP_Vn(n)        ((n) >> 14)
#define NV50_IR_SUBOP_VOTE_ALL 0
#define NV50_IR_SUBOP_VOTE_ANY 1
#define NV50_IR_SUBOP_VOTE_UNI 2
#define NV50_IR_SUBOP_LOP3_LUT_SRC0 0xf0
#define NV50_IR_SUBOP_LOP3_LUT_SRC1 0xcc
#define NV50_IR_SUBOP_LOP3_LUT_SRC2 0xaa
#define NV50_IR_SUBOP_LOP3_LUT(exp) ({         \
      uint8_t a = NV50_IR_SUBOP_LOP3_LUT_SRC0; \
      uint8_t b = NV50_IR_SUBOP_LOP3_LUT_SRC1; \
      uint8_t c = NV50_IR_SUBOP_LOP3_LUT_SRC2; \
      (uint8_t)(exp);                          \
})
#define NV50_IR_SUBOP_BMSK_C (0 << 0)
#define NV50_IR_SUBOP_BMSK_W (1 << 0)

#define NV50_IR_SUBOP_MINMAX_LOW  1
#define NV50_IR_SUBOP_MINMAX_MED  2
#define NV50_IR_SUBOP_MINMAX_HIGH 3

#define NV50_IR_SUBOP_SHF_L  (0 << 0)
#define NV50_IR_SUBOP_SHF_R  (1 << 0)
#define NV50_IR_SUBOP_SHF_LO (0 << 1)
#define NV50_IR_SUBOP_SHF_HI (1 << 1)
#define NV50_IR_SUBOP_SHF_C  (0 << 2)
#define NV50_IR_SUBOP_SHF_W  (1 << 2)

#define NV50_IR_SUBOP_VFETCH_PHYS 1

// xmad(src0, src1, 0) << 16 + src2
#define NV50_IR_SUBOP_XMAD_PSL (1 << 0)
// (xmad(src0, src1, src2) & 0xffff) | (src1 << 16)
#define NV50_IR_SUBOP_XMAD_MRG (1 << 1)
// xmad(src0, src1, src2.lo)
#define NV50_IR_SUBOP_XMAD_CLO (1 << 2)
// xmad(src0, src1, src2.hi)
#define NV50_IR_SUBOP_XMAD_CHI (2 << 2)
// if both operands to the multiplication are non-zero, subtract 65536 for each
// negative operand
#define NV50_IR_SUBOP_XMAD_CSFU (3 << 2)
// xmad(src0, src1, src2) + src1 << 16
#define NV50_IR_SUBOP_XMAD_CBCC (4 << 2)
#define NV50_IR_SUBOP_XMAD_CMODE_SHIFT 2
#define NV50_IR_SUBOP_XMAD_CMODE_MASK (0x7 << NV50_IR_SUBOP_XMAD_CMODE_SHIFT)

// use the high 16 bits instead of the low 16 bits for the multiplication.
// if the instruction's sType is signed, sign extend the operand from 16 bits
// to 32 before multiplication.
#define NV50_IR_SUBOP_XMAD_H1_SHIFT 5
#define NV50_IR_SUBOP_XMAD_H1(i) (1 << (NV50_IR_SUBOP_XMAD_H1_SHIFT + (i)))
#define NV50_IR_SUBOP_XMAD_H1_MASK (0x3 << NV50_IR_SUBOP_XMAD_H1_SHIFT)

enum DataType
{
   TYPE_NONE,
   TYPE_U8,
   TYPE_S8,
   TYPE_U16,
   TYPE_S16,
   TYPE_U32,
   TYPE_S32,
   TYPE_U64, // 64 bit operations are only lowered after register allocation
   TYPE_S64,
   TYPE_F16,
   TYPE_F32,
   TYPE_F64,
   TYPE_B96,
   TYPE_B128
};

enum CondCode
{
   CC_FL = 0,
   CC_NEVER = CC_FL, // when used with FILE_FLAGS
   CC_LT = 1,
   CC_EQ = 2,
   CC_NOT_P = CC_EQ, // when used with FILE_PREDICATE
   CC_LE = 3,
   CC_GT = 4,
   CC_NE = 5,
   CC_P  = CC_NE,
   CC_GE = 6,
   CC_TR = 7,
   CC_ALWAYS = CC_TR,
   CC_U  = 8,
   CC_LTU = 9,
   CC_EQU = 10,
   CC_LEU = 11,
   CC_GTU = 12,
   CC_NEU = 13,
   CC_GEU = 14,
   CC_NO = 0x10,
   CC_NC = 0x11,
   CC_NS = 0x12,
   CC_NA = 0x13,
   CC_A  = 0x14,
   CC_S  = 0x15,
   CC_C  = 0x16,
   CC_O  = 0x17
};

enum RoundMode
{
   ROUND_N, // nearest
   ROUND_M, // towards -inf
   ROUND_Z, // towards 0
   ROUND_P, // towards +inf
   ROUND_NI, // nearest integer
   ROUND_MI, // to integer towards -inf
   ROUND_ZI, // to integer towards 0
   ROUND_PI, // to integer towards +inf
};

enum CacheMode
{
   CACHE_CA,            // cache at all levels
   CACHE_WB = CACHE_CA, // cache write back
   CACHE_CG,            // cache at global level
   CACHE_CS,            // cache streaming
   CACHE_CV,            // cache as volatile
   CACHE_WT = CACHE_CV  // cache write-through
};

enum DataFile
{
   FILE_NULL = 0,
   FILE_GPR,
   FILE_PREDICATE,       // boolean predicate
   FILE_FLAGS,           // zero/sign/carry/overflow bits
   FILE_ADDRESS,
   FILE_BARRIER,
   LAST_REGISTER_FILE = FILE_BARRIER,
   FILE_IMMEDIATE,
   FILE_MEMORY_CONST,
   FILE_SHADER_INPUT,
   FILE_SHADER_OUTPUT,
   FILE_MEMORY_BUFFER,
   FILE_MEMORY_GLOBAL,
   FILE_MEMORY_SHARED,
   FILE_MEMORY_LOCAL,
   FILE_SYSTEM_VALUE,
   FILE_THREAD_STATE,           // "special" barrier registers
   DATA_FILE_COUNT
};

enum TexTarget
{
   TEX_TARGET_1D,
   TEX_TARGET_2D,
   TEX_TARGET_2D_MS,
   TEX_TARGET_3D,
   TEX_TARGET_CUBE,
   TEX_TARGET_1D_SHADOW,
   TEX_TARGET_2D_SHADOW,
   TEX_TARGET_CUBE_SHADOW,
   TEX_TARGET_1D_ARRAY,
   TEX_TARGET_2D_ARRAY,
   TEX_TARGET_2D_MS_ARRAY,
   TEX_TARGET_CUBE_ARRAY,
   TEX_TARGET_1D_ARRAY_SHADOW,
   TEX_TARGET_2D_ARRAY_SHADOW,
   TEX_TARGET_RECT,
   TEX_TARGET_RECT_SHADOW,
   TEX_TARGET_CUBE_ARRAY_SHADOW,
   TEX_TARGET_BUFFER,
   TEX_TARGET_COUNT
};

enum ImgFormat
{
   FMT_RGBA32F,
   FMT_RGBA16F,
   FMT_RG32F,
   FMT_RG16F,
   FMT_R11G11B10F,
   FMT_R32F,
   FMT_R16F,

   FMT_RGBA32UI,
   FMT_RGBA16UI,
   FMT_RGB10A2UI,
   FMT_RGBA8UI,
   FMT_RG32UI,
   FMT_RG16UI,
   FMT_RG8UI,
   FMT_R32UI,
   FMT_R16UI,
   FMT_R8UI,

   FMT_RGBA32I,
   FMT_RGBA16I,
   FMT_RGBA8I,
   FMT_RG32I,
   FMT_RG16I,
   FMT_RG8I,
   FMT_R32I,
   FMT_R16I,
   FMT_R8I,

   FMT_RGBA16,
   FMT_RGB10A2,
   FMT_RGBA8,
   FMT_RG16,
   FMT_RG8,
   FMT_R16,
   FMT_R8,

   FMT_RGBA16_SNORM,
   FMT_RGBA8_SNORM,
   FMT_RG16_SNORM,
   FMT_RG8_SNORM,
   FMT_R16_SNORM,
   FMT_R8_SNORM,

   FMT_BGRA8,

   IMG_FORMAT_COUNT,
};

enum ImgType {
   UINT,
   SINT,
   UNORM,
   SNORM,
   FLOAT,
};

enum SVSemantic
{
   SV_POSITION, // WPOS
   SV_VERTEX_ID,
   SV_INSTANCE_ID,
   SV_INVOCATION_ID,
   SV_PRIMITIVE_ID,
   SV_VERTEX_COUNT, // gl_PatchVerticesIn
   SV_LAYER,
   SV_VIEWPORT_INDEX,
   SV_VIEWPORT_MASK,
   SV_YDIR,
   SV_FACE,
   SV_POINT_SIZE,
   SV_POINT_COORD,
   SV_CLIP_DISTANCE,
   SV_SAMPLE_INDEX,
   SV_SAMPLE_POS,
   SV_SAMPLE_MASK,
   SV_TESS_OUTER,
   SV_TESS_INNER,
   SV_TESS_COORD,
   SV_TID,
   SV_COMBINED_TID,
   SV_CTAID,
   SV_NTID,
   SV_GRIDID,
   SV_NCTAID,
   SV_LANEID,
   SV_PHYSID,
   SV_NPHYSID,
   SV_CLOCK,
   SV_LBASE,
   SV_SBASE,
   SV_VERTEX_STRIDE,
   SV_INVOCATION_INFO,
   SV_THREAD_KILL,
   SV_BASEVERTEX,
   SV_BASEINSTANCE,
   SV_DRAWID,
   SV_WORK_DIM,
   SV_LANEMASK_EQ,
   SV_LANEMASK_LT,
   SV_LANEMASK_LE,
   SV_LANEMASK_GT,
   SV_LANEMASK_GE,
   SV_UNDEFINED,
   SV_LAST
};

enum TSSemantic
{
   // 0-15 are fixed ones on Volta/Turing
   TS_THREAD_STATE_ENUM0 = 0,
   TS_THREAD_STATE_ENUM1 = 1,
   TS_THREAD_STATE_ENUM2 = 2,
   TS_THREAD_STATE_ENUM3 = 3,
   TS_THREAD_STATE_ENUM4 = 4,
   TS_TRAP_RETURN_PC_LO  = 5,
   TS_TRAP_RETURN_PC_HI  = 6,
   TS_TRAP_RETURN_MASK   = 7,
   TS_MEXITED            = 8,
   TS_MKILL              = 9,
   TS_MACTIVE            = 10,
   TS_MATEXIT            = 11,
   TS_OPT_STACK          = 12,
   TS_API_CALL_DEPTH     = 13,
   TS_ATEXIT_PC_LO       = 14,
   TS_ATEXIT_PC_HI       = 15,
   // special ones to make our life easier
   TS_PQUAD_MACTIVE,
};

class Program;
class Function;
class BasicBlock;

class Target;

class Instruction;
class CmpInstruction;
class TexInstruction;
class FlowInstruction;

class Value;
class LValue;
class Symbol;
class ImmediateValue;

struct Storage
{
   DataFile file;
   int8_t fileIndex; // signed, may be indirect for CONST[]
   uint8_t size; // this should match the Instruction type's size
   DataType type; // mainly for pretty printing
   union {
      uint64_t u64;    // immediate values
      uint32_t u32;
      uint16_t u16;
      uint8_t u8;
      int64_t s64;
      int32_t s32;
      int16_t s16;
      int8_t s8;
      float f32;
      double f64;
      int32_t offset; // offset from 0 (base of address space)
      int32_t id;     // register id (< 0 if virtual/unassigned, in units <= 4)
      struct {
         SVSemantic sv;
         int index;
      } sv;
      TSSemantic ts;
   } data;
};

// precedence: NOT after SAT after NEG after ABS
#define NV50_IR_MOD_ABS (1 << 0)
#define NV50_IR_MOD_NEG (1 << 1)
#define NV50_IR_MOD_SAT (1 << 2)
#define NV50_IR_MOD_NOT (1 << 3)
#define NV50_IR_MOD_NEG_ABS (NV50_IR_MOD_NEG | NV50_IR_MOD_ABS)

#define NV50_IR_INTERP_MODE_MASK   0x3
#define NV50_IR_INTERP_LINEAR      (0 << 0)
#define NV50_IR_INTERP_PERSPECTIVE (1 << 0)
#define NV50_IR_INTERP_FLAT        (2 << 0)
#define NV50_IR_INTERP_SC          (3 << 0) // what exactly is that ?
#define NV50_IR_INTERP_SAMPLE_MASK 0xc
#define NV50_IR_INTERP_DEFAULT     (0 << 2)
#define NV50_IR_INTERP_CENTROID    (1 << 2)
#define NV50_IR_INTERP_OFFSET      (2 << 2)
#define NV50_IR_INTERP_SAMPLEID    (3 << 2)

// do we really want this to be a class ?
class Modifier
{
public:
   Modifier() : bits(0) { }
   Modifier(unsigned int m) : bits(m) { }
   Modifier(operation op);

   // @return new Modifier applying a after b (asserts if unrepresentable)
   Modifier operator*(const Modifier) const;
   Modifier operator*=(const Modifier m) { *this = *this * m; return *this; }
   Modifier operator==(const Modifier m) const { return m.bits == bits; }
   Modifier operator!=(const Modifier m) const { return m.bits != bits; }

   inline Modifier operator&(const Modifier m) const { return bits & m.bits; }
   inline Modifier operator|(const Modifier m) const { return bits | m.bits; }
   inline Modifier operator^(const Modifier m) const { return bits ^ m.bits; }

   operation getOp() const;

   inline int neg() const { return (bits & NV50_IR_MOD_NEG) ? 1 : 0; }
   inline int abs() const { return (bits & NV50_IR_MOD_ABS) ? 1 : 0; }

   inline operator bool() const { return bits ? true : false; }

   void applyTo(ImmediateValue &imm) const;

   int print(char *buf, size_t size) const;

private:
   uint8_t bits;
};

class ValueRef
{
public:
   ValueRef(Value * = NULL);
   ValueRef(const ValueRef&);
   ~ValueRef();

   ValueRef& operator=(const ValueRef&) = delete;

   inline bool exists() const { return value != NULL; }

   void set(Value *);
   void set(const ValueRef&);
   inline Value *get() const { return value; }
   inline Value *rep() const;

   inline Instruction *getInsn() const { return insn; }
   inline void setInsn(Instruction *inst) { insn = inst; }

   inline bool isIndirect(int dim) const { return indirect[dim] >= 0; }
   inline const ValueRef *getIndirect(int dim) const;

   inline DataFile getFile() const;
   inline unsigned getSize() const;

   // SSA: return eventual (traverse MOVs) literal value, if it exists
   bool getImmediate(ImmediateValue&) const;

public:
   Modifier mod;
   int8_t indirect[2]; // >= 0 if relative to lvalue in insn->src(indirect[i])

   bool usedAsPtr; // for printing

private:
   Value *value;
   Instruction *insn;
};

class ValueDef
{
public:
   ValueDef(Value * = NULL);
   ValueDef(const ValueDef&);
   ~ValueDef();

   ValueDef& operator=(const ValueDef&) = delete;

   inline bool exists() const { return value != NULL; }

   inline Value *get() const { return value; }
   inline Value *rep() const;
   void set(Value *);
   bool mayReplace(const ValueRef &);
   void replace(const ValueRef &, bool doSet); // replace all uses of the old value

   inline Instruction *getInsn() const { return insn; }
   inline void setInsn(Instruction *inst) { insn = inst; }

   inline DataFile getFile() const;
   inline unsigned getSize() const;

   inline void setSSA(LValue *);
   inline const LValue *preSSA() const;

private:
   Value *value;   // should make this LValue * ...
   LValue *origin; // pre SSA value
   Instruction *insn;
};

class Value
{
public:
   Value();
   virtual ~Value() { }

   Value(const Value&) = delete;
   Value& operator=(const Value&) = delete;

   virtual Value *clone(ClonePolicy<Function>&) const = 0;

   virtual int print(char *, size_t, DataType ty = TYPE_NONE) const = 0;

   virtual bool equals(const Value *, bool strict = false) const;
   virtual bool interfers(const Value *) const;
   virtual bool isUniform() const { return true; }

   inline Value *rep() const { return join; }

   inline Instruction *getUniqueInsn() const;
   inline Instruction *getInsn() const; // use when uniqueness is certain

   inline int refCount() { return uses.size(); }

   inline LValue *asLValue();
   inline Symbol *asSym();
   inline ImmediateValue *asImm();
   inline const Symbol *asSym() const;
   inline const ImmediateValue *asImm() const;

   inline bool inFile(DataFile f) const { return reg.file == f; }

   static inline Value *get(Iterator&);

   std::unordered_set<ValueRef *> uses;
   std::list<ValueDef *> defs;
   typedef std::unordered_set<ValueRef *>::iterator UseIterator;
   typedef std::unordered_set<ValueRef *>::const_iterator UseCIterator;
   typedef std::list<ValueDef *>::iterator DefIterator;
   typedef std::list<ValueDef *>::const_iterator DefCIterator;

   int id;
   Storage reg;

   // TODO: these should be in LValue:
   Interval livei;
   Value *join;
};

class LValue : public Value
{
public:
   LValue(Function *, DataFile file);
   LValue(Function *, LValue *);
   ~LValue() { }

   virtual bool isUniform() const;

   virtual LValue *clone(ClonePolicy<Function>&) const;

   virtual int print(char *, size_t, DataType ty = TYPE_NONE) const;

public:
   unsigned compMask : 8; // compound/component mask
   unsigned compound : 1; // used by RA, value involved in split/merge
   unsigned ssa      : 1;
   unsigned fixedReg : 1; // set & used by RA, earlier just use (id < 0)
   unsigned noSpill  : 1; // do not spill (e.g. if spill temporary already)
};

class Symbol : public Value
{
public:
   Symbol(Program *, DataFile file = FILE_MEMORY_CONST, uint8_t fileIdx = 0);
   ~Symbol() { }

   virtual Symbol *clone(ClonePolicy<Function>&) const;

   virtual bool equals(const Value *that, bool strict) const;

   virtual bool isUniform() const;

   virtual int print(char *, size_t, DataType ty = TYPE_NONE) const;

   // print with indirect values
   int print(char *, size_t, Value *, Value *, DataType ty = TYPE_NONE) const;

   inline void setFile(DataFile file, uint8_t fileIndex = 0)
   {
      reg.file = file;
      reg.fileIndex = fileIndex;
   }

   inline void setOffset(int32_t offset);
   inline void setAddress(Symbol *base, int32_t offset);
   inline void setSV(SVSemantic sv, uint32_t idx = 0);

   inline const Symbol *getBase() const { return baseSym; }

private:
   Symbol *baseSym; // array base for Symbols representing array elements
};

class ImmediateValue : public Value
{
public:
   ImmediateValue() { }
   ImmediateValue(Program *, uint32_t);
   ImmediateValue(Program *, float);
   ImmediateValue(Program *, double);
   // NOTE: not added to program with
   ImmediateValue(const ImmediateValue *, DataType ty);
   ~ImmediateValue() { };

   virtual ImmediateValue *clone(ClonePolicy<Function>&) const;

   virtual bool equals(const Value *that, bool strict) const;

   // these only work if 'type' is valid (we mostly use untyped literals):
   bool isInteger(const int ival) const; // ival is cast to this' type
   bool isNegative() const;
   bool isPow2() const;

   void applyLog2();

   // for constant folding:
   ImmediateValue operator+(const ImmediateValue&) const;
   ImmediateValue operator-(const ImmediateValue&) const;
   ImmediateValue operator*(const ImmediateValue&) const;
   ImmediateValue operator/(const ImmediateValue&) const;

   ImmediateValue& operator=(const ImmediateValue&); // only sets value !

   bool compare(CondCode cc, float fval) const;

   virtual int print(char *, size_t, DataType ty = TYPE_NONE) const;
};

class Instruction
{
public:
   Instruction();
   Instruction(Function *, operation, DataType);
   virtual ~Instruction();

   Instruction(const Instruction&) = delete;
   Instruction& operator=(const Instruction&) = delete;

   virtual Instruction *clone(ClonePolicy<Function>&,
                              Instruction * = NULL) const;

   void setDef(int i, Value *);
   void setSrc(int s, Value *);
   void setSrc(int s, const ValueRef&);
   void swapSources(int a, int b);
   void moveSources(int s, int delta);
   bool setIndirect(int s, int dim, Value *);

   inline ValueRef& src(int s) { return srcs[s]; }
   inline ValueDef& def(int s) { return defs[s]; }
   inline const ValueRef& src(int s) const { return srcs[s]; }
   inline const ValueDef& def(int s) const { return defs[s]; }

   inline Value *getDef(int d) const { return defs[d].get(); }
   inline Value *getSrc(int s) const { return srcs[s].get(); }
   inline Value *getIndirect(int s, int dim) const;

   inline bool defExists(unsigned d) const
   {
      return d < defs.size() && defs[d].exists();
   }
   inline bool srcExists(unsigned s) const
   {
      return s < srcs.size() && srcs[s].exists();
   }

   inline bool constrainedDefs() const;

   bool setPredicate(CondCode ccode, Value *);
   inline Value *getPredicate() const;
   bool writesPredicate() const;
   inline bool isPredicated() const { return predSrc >= 0; }

   inline void setFlagsSrc(int s, Value *);
   inline void setFlagsDef(int d, Value *);
   inline bool usesFlags() const { return flagsSrc >= 0; }

   unsigned int defCount() const { return defs.size(); };
   unsigned int defCount(unsigned int mask, bool singleFile = false) const;
   unsigned int srcCount() const { return srcs.size(); };
   unsigned int srcCount(unsigned int mask, bool singleFile = false) const;

   // save & remove / set indirect[0,1] and predicate source
   void takeExtraSources(int s, Value *[3]);
   void putExtraSources(int s, Value *[3]);

   inline void setType(DataType type) { dType = sType = type; }

   inline void setType(DataType dtype, DataType stype)
   {
      dType = dtype;
      sType = stype;
   }

   inline bool isPseudo() const { return op < OP_MOV; }
   bool isDead() const;
   bool isNop() const;
   bool isCommutationLegal(const Instruction *) const; // must be adjacent !
   bool isActionEqual(const Instruction *) const;
   bool isResultEqual(const Instruction *) const;

   // check whether the defs interfere with srcs and defs of another instruction
   bool canCommuteDefDef(const Instruction *) const;
   bool canCommuteDefSrc(const Instruction *) const;

   void print() const;

   inline CmpInstruction *asCmp();
   inline TexInstruction *asTex();
   inline FlowInstruction *asFlow();
   inline const TexInstruction *asTex() const;
   inline const CmpInstruction *asCmp() const;
   inline const FlowInstruction *asFlow() const;

public:
   Instruction *next;
   Instruction *prev;
   int id;
   int serial; // CFG order

   operation op;
   DataType dType; // destination or defining type
   DataType sType; // source or secondary type
   CondCode cc;
   RoundMode rnd;
   CacheMode cache;

   uint16_t subOp; // quadop, 1 for mul-high, etc.

   unsigned encSize    : 5; // encoding size in bytes
   unsigned saturate   : 1; // to [0.0f, 1.0f]
   unsigned join       : 1; // converge control flow (use OP_JOIN until end)
   unsigned fixed      : 1; // prevent dead code elimination
   unsigned terminator : 1; // end of basic block
   unsigned ftz        : 1; // flush denormal to zero
   unsigned dnz        : 1; // denormals, NaN are zero
   unsigned ipa        : 4; // interpolation mode
   unsigned lanes      : 4;
   unsigned perPatch   : 1;
   unsigned exit       : 1; // terminate program after insn
   unsigned mask       : 4; // for vector ops
   // prevent algebraic optimisations that aren't bit-for-bit identical
   unsigned precise    : 1;

   int8_t postFactor; // MUL/DIV(if < 0) by 1 << postFactor

   int8_t predSrc;
   int8_t flagsDef;
   int8_t flagsSrc;

   uint32_t sched; // scheduling data (NOTE: maybe move to separate storage)

   BasicBlock *bb;

protected:
   std::deque<ValueDef> defs; // no gaps !
   std::deque<ValueRef> srcs; // no gaps !

   // instruction specific methods:
   // (don't want to subclass, would need more constructors and memory pools)
public:
   inline void setInterpolate(unsigned int mode) { ipa = mode; }

   unsigned int getInterpMode() const { return ipa & 0x3; }
   unsigned int getSampleMode() const { return ipa & 0xc; }

private:
   void init();
};

enum TexQuery
{
   TXQ_DIMS, /* x, y, z, levels */
   TXQ_TYPE, /* ?, ?, samples, ? */
   TXQ_SAMPLE_POSITION,
   TXQ_FILTER,
   TXQ_LOD,
   TXQ_WRAP,
   TXQ_BORDER_COLOUR
};

class TexInstruction : public Instruction
{
public:
   class Target
   {
   public:
      Target(TexTarget targ = TEX_TARGET_1D) : target(targ) { }

      const char *getName() const { return descTable[target].name; }
      unsigned int getArgCount() const { return descTable[target].argc; }
      unsigned int getDim() const { return descTable[target].dim; }
      int isArray() const { return descTable[target].array ? 1 : 0; }
      int isCube() const { return descTable[target].cube ? 1 : 0; }
      int isShadow() const { return descTable[target].shadow ? 1 : 0; }
      int isMS() const {
        return target == TEX_TARGET_2D_MS || target == TEX_TARGET_2D_MS_ARRAY; }
      void clearMS() {
         if (isMS()) {
            if (isArray())
               target = TEX_TARGET_2D_ARRAY;
            else
               target = TEX_TARGET_2D;
         }
      }

      Target& operator=(TexTarget targ)
      {
         assert(targ < TEX_TARGET_COUNT);
         target = targ;
         return *this;
      }

      inline bool operator==(TexTarget targ) const { return target == targ; }
      inline bool operator!=(TexTarget targ) const { return target != targ; }

      enum TexTarget getEnum() const { return target; }

   private:
      struct Desc
      {
         char name[19];
         uint8_t dim;
         uint8_t argc;
         bool array;
         bool cube;
         bool shadow;
      };

      static const struct Desc descTable[TEX_TARGET_COUNT];

   private:
      enum TexTarget target;
   };

public:
   struct ImgFormatDesc
   {
      char name[19];
      uint8_t components;
      uint8_t bits[4];
      ImgType type;
      bool bgra;
   };

   static const struct ImgFormatDesc formatTable[IMG_FORMAT_COUNT];
   static const struct ImgFormatDesc *translateImgFormat(
         enum pipe_format format);

public:
   TexInstruction(Function *, operation);
   virtual ~TexInstruction();

   virtual TexInstruction *clone(ClonePolicy<Function>&,
                                 Instruction * = NULL) const;

   inline void setTexture(Target targ, uint8_t r, uint8_t s)
   {
      tex.r = r;
      tex.s = s;
      tex.target = targ;
   }

   void setIndirectR(Value *);
   void setIndirectS(Value *);
   inline Value *getIndirectR() const;
   inline Value *getIndirectS() const;

public:
   struct {
      Target target;

      uint16_t r;
      uint16_t s;
      int8_t rIndirectSrc;
      int8_t sIndirectSrc;

      uint8_t mask;
      uint8_t gatherComp;

      bool liveOnly; // only execute on live pixels of a quad (optimization)
      bool levelZero;
      bool derivAll;
      bool bindless;

      int8_t useOffsets; // 0, 1, or 4 for textureGatherOffsets
      int8_t offset[3]; // only used on nv50

      enum TexQuery query;
      const struct ImgFormatDesc *format;

      bool scalar; // for GM107s TEXS, TLDS, TLD4S
   } tex;

   ValueRef dPdx[3];
   ValueRef dPdy[3];
   ValueRef offset[4][3];
};

class CmpInstruction : public Instruction
{
public:
   CmpInstruction(Function *, operation);

   virtual CmpInstruction *clone(ClonePolicy<Function>&,
                                 Instruction * = NULL) const;

   void setCondition(CondCode cond) { setCond = cond; }
   CondCode getCondition() const { return setCond; }

public:
   CondCode setCond;
};

class FlowInstruction : public Instruction
{
public:
   FlowInstruction(Function *, operation, void *target);

   virtual FlowInstruction *clone(ClonePolicy<Function>&,
                                  Instruction * = NULL) const;

public:
   unsigned allWarp  : 1;
   unsigned absolute : 1;
   unsigned limit    : 1;
   unsigned builtin  : 1; // true for calls to emulation code
   unsigned indirect : 1; // target in src(0)

   union {
      BasicBlock *bb;
      int builtin;
      Function *fn;
   } target;
};

class BasicBlock
{
public:
   BasicBlock(Function *);
   ~BasicBlock();

   BasicBlock(const BasicBlock&) = delete;
   BasicBlock& operator=(const BasicBlock&) = delete;

   BasicBlock *clone(ClonePolicy<Function>&) const;

   inline int getId() const { return id; }
   inline unsigned int getInsnCount() const { return numInsns; }
   inline bool isTerminated() const { return exit && exit->terminator; }

   bool dominatedBy(BasicBlock *bb);
   inline bool reachableBy(const BasicBlock *by, const BasicBlock *term);

   // returns mask of conditional out blocks
   // e.g. 3 for IF { .. } ELSE { .. } ENDIF, 1 for IF { .. } ENDIF
   unsigned int initiatesSimpleConditional() const;

public:
   Function *getFunction() const { return func; }
   Program *getProgram() const { return program; }

   Instruction *getEntry() const { return entry; } // first non-phi instruction
   Instruction *getPhi() const { return phi; }
   Instruction *getFirst() const { return phi ? phi : entry; }
   Instruction *getExit() const { return exit; }

   void insertHead(Instruction *);
   void insertTail(Instruction *);
   void insertBefore(Instruction *, Instruction *);
   void insertAfter(Instruction *, Instruction *);
   void remove(Instruction *);
   void permuteAdjacent(Instruction *, Instruction *);

   BasicBlock *idom() const;

   // NOTE: currently does not rebuild the dominator tree
   BasicBlock *splitBefore(Instruction *, bool attach = true);
   BasicBlock *splitAfter(Instruction *, bool attach = true);

   DLList& getDF() { return df; }
   DLList::Iterator iterDF() { return df.iterator(); }

   static inline BasicBlock *get(Iterator&);
   static inline BasicBlock *get(Graph::Node *);

public:
   Graph::Node cfg; // first edge is branch *taken* (the ELSE branch)
   Graph::Node dom;

   BitSet liveSet;
   BitSet defSet;

   uint32_t binPos;
   uint32_t binSize;

   Instruction *joinAt; // for quick reference

   bool explicitCont; // loop headers: true if loop contains continue stmts

private:
   int id;
   DLList df;

   Instruction *phi;
   Instruction *entry;
   Instruction *exit;

   unsigned int numInsns;

private:
   Function *func;
   Program *program;

   void splitCommon(Instruction *, BasicBlock *, bool attach);
};

class Function
{
public:
   Function(Program *, const char *name, uint32_t label);
   ~Function();

   Function(const Function&) = delete;
   Function& operator=(const Function&) = delete;

   static inline Function *get(Graph::Node *node);

   inline Program *getProgram() const { return prog; }
   inline const char *getName() const { return name; }
   inline int getId() const { return id; }
   inline uint32_t getLabel() const { return label; }

   void print();
   void printLiveIntervals() const;
   void printCFGraph(const char *filePath);

   bool setEntry(BasicBlock *);
   bool setExit(BasicBlock *);

   unsigned int orderInstructions(ArrayList&);

   inline void add(BasicBlock *bb, int& id) { allBBlocks.insert(bb, id); }
   inline void add(Instruction *insn, int& id) { allInsns.insert(insn, id); }
   inline void add(LValue *lval, int& id) { allLValues.insert(lval, id); }

   inline LValue *getLValue(int id);

   void buildLiveSets();
   bool convertToSSA();

public:
   std::deque<ValueDef> ins;
   std::deque<ValueRef> outs;
   std::deque<Value *> clobbers;

   Graph cfg;
   Graph::Node *cfgExit;
   Graph *domTree;
   Graph::Node call; // node in the call graph

   BasicBlock **bbArray; // BBs in emission order
   int bbCount;

   unsigned int loopNestingBound;
   int regClobberMax;

   uint32_t binPos;
   uint32_t binSize;

   uint32_t tlsBase; // base address for l[] space (if no stack pointer is used)
   uint32_t tlsSize;

   ArrayList allBBlocks;
   ArrayList allInsns;
   ArrayList allLValues;

private:
   void buildLiveSetsPreSSA(BasicBlock *, const int sequence);
   void buildDefSetsPreSSA(BasicBlock *bb, const int seq);

private:
   uint32_t label;
   int id;
   const char *const name;
   Program *prog;
};

enum CGStage
{
   CG_STAGE_PRE_SSA,
   CG_STAGE_SSA, // expected directly before register allocation
   CG_STAGE_POST_RA
};

class Program
{
public:
   enum Type
   {
      TYPE_VERTEX,
      TYPE_TESSELLATION_CONTROL,
      TYPE_TESSELLATION_EVAL,
      TYPE_GEOMETRY,
      TYPE_FRAGMENT,
      TYPE_COMPUTE
   };

   Program(Type type, Target *targ);
   ~Program();

   Program(const Program&) = delete;
   Program& operator=(const Program&) = delete;

   void print();

   Type getType() const { return progType; }

   inline void add(Function *fn, int& id) { allFuncs.insert(fn, id); }
   inline void del(Function *fn, int& id) { allFuncs.remove(id); }
   inline void add(Value *rval, int& id) { allRValues.insert(rval, id); }

   bool makeFromNIR(struct nv50_ir_prog_info *,
                    struct nv50_ir_prog_info_out *);
   bool convertToSSA();
   bool optimizeSSA(int level);
   bool optimizePostRA(int level);
   bool registerAllocation();
   bool emitBinary(struct nv50_ir_prog_info_out *);

   const Target *getTarget() const { return target; }

private:
   Type progType;
   Target *target;

public:
   Function *main;
   Graph calls;

   ArrayList allFuncs;
   ArrayList allRValues;

   uint32_t *code;
   uint32_t binSize;
   uint32_t tlsSize; // size required for FILE_MEMORY_LOCAL

   int maxGPR;
   bool fp64;
   bool persampleInvocation;

   MemoryPool mem_Instruction;
   MemoryPool mem_CmpInstruction;
   MemoryPool mem_TexInstruction;
   MemoryPool mem_FlowInstruction;
   MemoryPool mem_LValue;
   MemoryPool mem_Symbol;
   MemoryPool mem_ImmediateValue;

   uint32_t dbgFlags;
   uint8_t  optLevel;

   void *targetPriv; // e.g. to carry information between passes

   const struct nv50_ir_prog_info *driver; // for driver configuration
   const struct nv50_ir_prog_info_out *driver_out; // for driver configuration

   void releaseInstruction(Instruction *);
   void releaseValue(Value *);
};

// TODO: add const version
class Pass
{
public:
   bool run(Program *, bool ordered = false, bool skipPhi = false);
   bool run(Function *, bool ordered = false, bool skipPhi = false);

private:
   bool doRun(Program *, bool ordered, bool skipPhi);
   bool doRun(Function *, bool ordered, bool skipPhi);

protected:
   // return false to continue with next entity on next higher level
   virtual bool visit(Function *) { return true; }
   virtual bool visit(BasicBlock *) { return true; }
   virtual bool visit(Instruction *) { return false; }

   bool err;
   Function *func;
   Program *prog;
};

// =============================================================================

#include "nv50_ir_inlines.h"

} // namespace nv50_ir

#endif // __NV50_IR_H__
