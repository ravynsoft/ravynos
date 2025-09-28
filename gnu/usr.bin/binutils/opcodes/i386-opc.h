/* Declarations for Intel 80386 opcode table
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

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
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "opcode/i386.h"
#include <limits.h>
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

/* Position of cpu flags bitfiled.  */

enum
{
  /* i186 or better required */
  Cpu186 = 0,
  /* i286 or better required */
  Cpu286,
  /* i386 or better required */
  Cpu386,
  /* i486 or better required */
  Cpu486,
  /* i585 or better required */
  Cpu586,
  /* i686 or better required */
  Cpu686,
  /* CMOV Instruction support required */
  CpuCMOV,
  /* FXSR Instruction support required */
  CpuFXSR,
  /* CLFLUSH Instruction support required */
  CpuClflush,
  /* NOP Instruction support required */
  CpuNop,
  /* SYSCALL Instructions support required */
  CpuSYSCALL,
  /* Floating point support required */
  Cpu8087,
  /* i287 support required */
  Cpu287,
  /* i387 support required */
  Cpu387,
  /* i686 and floating point support required */
  Cpu687,
  /* SSE3 and floating point support required */
  CpuFISTTP,
  /* MMX support required */
  CpuMMX,
  /* SSE support required */
  CpuSSE,
  /* SSE2 support required */
  CpuSSE2,
  /* 3dnow! support required */
  Cpu3dnow,
  /* 3dnow! Extensions support required */
  Cpu3dnowA,
  /* SSE3 support required */
  CpuSSE3,
  /* VIA PadLock required */
  CpuPadLock,
  /* AMD Secure Virtual Machine Ext-s required */
  CpuSVME,
  /* VMX Instructions required */
  CpuVMX,
  /* SMX Instructions required */
  CpuSMX,
  /* SSSE3 support required */
  CpuSSSE3,
  /* SSE4a support required */
  CpuSSE4a,
  /* LZCNT support required */
  CpuLZCNT,
  /* POPCNT support required */
  CpuPOPCNT,
  /* MONITOR support required */
  CpuMONITOR,
  /* SSE4.1 support required */
  CpuSSE4_1,
  /* SSE4.2 support required */
  CpuSSE4_2,
  /* AVX support required */
  CpuAVX,
  /* AVX2 support required */
  CpuAVX2,
  /* Intel AVX-512 Foundation Instructions support required */
  CpuAVX512F,
  /* Intel AVX-512 Conflict Detection Instructions support required */
  CpuAVX512CD,
  /* Intel AVX-512 Exponential and Reciprocal Instructions support
     required */
  CpuAVX512ER,
  /* Intel AVX-512 Prefetch Instructions support required */
  CpuAVX512PF,
  /* Intel AVX-512 VL Instructions support required.  */
  CpuAVX512VL,
  /* Intel AVX-512 DQ Instructions support required.  */
  CpuAVX512DQ,
  /* Intel AVX-512 BW Instructions support required.  */
  CpuAVX512BW,
  /* Intel IAMCU support required */
  CpuIAMCU,
  /* Xsave/xrstor New Instructions support required */
  CpuXsave,
  /* Xsaveopt New Instructions support required */
  CpuXsaveopt,
  /* AES support required */
  CpuAES,
  /* PCLMUL support required */
  CpuPCLMUL,
  /* FMA support required */
  CpuFMA,
  /* FMA4 support required */
  CpuFMA4,
  /* XOP support required */
  CpuXOP,
  /* LWP support required */
  CpuLWP,
  /* BMI support required */
  CpuBMI,
  /* TBM support required */
  CpuTBM,
  /* MOVBE Instruction support required */
  CpuMovbe,
  /* CMPXCHG16B instruction support required.  */
  CpuCX16,
  /* LAHF/SAHF instruction support required (in 64-bit mode).  */
  CpuLAHF_SAHF,
  /* EPT Instructions required */
  CpuEPT,
  /* RDTSCP Instruction support required */
  CpuRdtscp,
  /* FSGSBASE Instructions required */
  CpuFSGSBase,
  /* RDRND Instructions required */
  CpuRdRnd,
  /* F16C Instructions required */
  CpuF16C,
  /* Intel BMI2 support required */
  CpuBMI2,
  /* HLE support required */
  CpuHLE,
  /* RTM support required */
  CpuRTM,
  /* INVPCID Instructions required */
  CpuINVPCID,
  /* VMFUNC Instruction required */
  CpuVMFUNC,
  /* Intel MPX Instructions required  */
  CpuMPX,
  /* 64bit support available, used by -march= in assembler.  */
  CpuLM,
  /* RDRSEED instruction required.  */
  CpuRDSEED,
  /* Multi-presisionn add-carry instructions are required.  */
  CpuADX,
  /* Supports prefetchw and prefetch instructions.  */
  CpuPRFCHW,
  /* SMAP instructions required.  */
  CpuSMAP,
  /* SHA instructions required.  */
  CpuSHA,
  /* CLFLUSHOPT instruction required */
  CpuClflushOpt,
  /* XSAVES/XRSTORS instruction required */
  CpuXSAVES,
  /* XSAVEC instruction required */
  CpuXSAVEC,
  /* PREFETCHWT1 instruction required */
  CpuPREFETCHWT1,
  /* SE1 instruction required */
  CpuSE1,
  /* CLWB instruction required */
  CpuCLWB,
  /* Intel AVX-512 IFMA Instructions support required.  */
  CpuAVX512IFMA,
  /* Intel AVX-512 VBMI Instructions support required.  */
  CpuAVX512VBMI,
  /* Intel AVX-512 4FMAPS Instructions support required.  */
  CpuAVX512_4FMAPS,
  /* Intel AVX-512 4VNNIW Instructions support required.  */
  CpuAVX512_4VNNIW,
  /* Intel AVX-512 VPOPCNTDQ Instructions support required.  */
  CpuAVX512_VPOPCNTDQ,
  /* Intel AVX-512 VBMI2 Instructions support required.  */
  CpuAVX512_VBMI2,
  /* Intel AVX-512 VNNI Instructions support required.  */
  CpuAVX512_VNNI,
  /* Intel AVX-512 BITALG Instructions support required.  */
  CpuAVX512_BITALG,
  /* Intel AVX-512 BF16 Instructions support required.  */
  CpuAVX512_BF16,
  /* Intel AVX-512 VP2INTERSECT Instructions support required.  */
  CpuAVX512_VP2INTERSECT,
  /* TDX Instructions support required.  */
  CpuTDX,
  /* Intel AVX VNNI Instructions support required.  */
  CpuAVX_VNNI,
  /* Intel AVX-512 FP16 Instructions support required.  */
  CpuAVX512_FP16,
  /* PREFETCHI instruction required */
  CpuPREFETCHI,
  /* Intel AVX IFMA Instructions support required.  */
  CpuAVX_IFMA,
  /* Intel AVX VNNI-INT8 Instructions support required.  */
  CpuAVX_VNNI_INT8,
  /* Intel CMPccXADD instructions support required.  */
  CpuCMPCCXADD,
  /* Intel WRMSRNS Instructions support required */
  CpuWRMSRNS,
  /* Intel MSRLIST Instructions support required.  */
  CpuMSRLIST,
  /* Intel AVX NE CONVERT Instructions support required.  */
  CpuAVX_NE_CONVERT,
  /* Intel RAO INT Instructions support required.  */
  CpuRAO_INT,
  /* fred instruction required */
  CpuFRED,
  /* lkgs instruction required */
  CpuLKGS,
  /* mwaitx instruction required */
  CpuMWAITX,
  /* Clzero instruction required */
  CpuCLZERO,
  /* OSPKE instruction required */
  CpuOSPKE,
  /* RDPID instruction required */
  CpuRDPID,
  /* PTWRITE instruction required */
  CpuPTWRITE,
  /* CET instructions support required */
  CpuIBT,
  CpuSHSTK,
  /* AMX-INT8 instructions required */
  CpuAMX_INT8,
  /* AMX-BF16 instructions required */
  CpuAMX_BF16,
  /* AMX-FP16 instructions required */
  CpuAMX_FP16,
  /* AMX-COMPLEX instructions required.  */
  CpuAMX_COMPLEX,
  /* AMX-TILE instructions required */
  CpuAMX_TILE,
  /* GFNI instructions required */
  CpuGFNI,
  /* VAES instructions required */
  CpuVAES,
  /* VPCLMULQDQ instructions required */
  CpuVPCLMULQDQ,
  /* WBNOINVD instructions required */
  CpuWBNOINVD,
  /* PCONFIG instructions required */
  CpuPCONFIG,
  /* WAITPKG instructions required */
  CpuWAITPKG,
  /* UINTR instructions required */
  CpuUINTR,
  /* CLDEMOTE instruction required */
  CpuCLDEMOTE,
  /* MOVDIRI instruction support required */
  CpuMOVDIRI,
  /* MOVDIRR64B instruction required */
  CpuMOVDIR64B,
  /* ENQCMD instruction required */
  CpuENQCMD,
  /* SERIALIZE instruction required */
  CpuSERIALIZE,
  /* RDPRU instruction required */
  CpuRDPRU,
  /* MCOMMIT instruction required */
  CpuMCOMMIT,
  /* SEV-ES instruction(s) required */
  CpuSEV_ES,
  /* TSXLDTRK instruction required */
  CpuTSXLDTRK,
  /* KL instruction support required */
  CpuKL,
  /* WideKL instruction support required */
  CpuWideKL,
  /* HRESET instruction required */
  CpuHRESET,
  /* INVLPGB instructions required */
  CpuINVLPGB,
  /* TLBSYNC instructions required */
  CpuTLBSYNC,
  /* SNP instructions required */
  CpuSNP,
  /* RMPQUERY instruction required */
  CpuRMPQUERY,

  /* NOTE: These last three items need to remain last and in this order. */

  /* 64bit support required  */
  Cpu64,
  /* Not supported in the 64bit mode  */
  CpuNo64,
  /* The last bitfield in i386_cpu_flags.  */
  CpuMax = CpuNo64
};

#define CpuNumOfUints \
  (CpuMax / sizeof (unsigned int) / CHAR_BIT + 1)
#define CpuNumOfBits \
  (CpuNumOfUints * sizeof (unsigned int) * CHAR_BIT)

/* If you get a compiler error for zero width of the unused field,
   comment it out.  */
#define CpuUnused	(CpuMax + 1)

/* We can check if an instruction is available with array instead
   of bitfield. */
typedef union i386_cpu_flags
{
  struct
    {
      unsigned int cpui186:1;
      unsigned int cpui286:1;
      unsigned int cpui386:1;
      unsigned int cpui486:1;
      unsigned int cpui586:1;
      unsigned int cpui686:1;
      unsigned int cpucmov:1;
      unsigned int cpufxsr:1;
      unsigned int cpuclflush:1;
      unsigned int cpunop:1;
      unsigned int cpusyscall:1;
      unsigned int cpu8087:1;
      unsigned int cpu287:1;
      unsigned int cpu387:1;
      unsigned int cpu687:1;
      unsigned int cpufisttp:1;
      unsigned int cpummx:1;
      unsigned int cpusse:1;
      unsigned int cpusse2:1;
      unsigned int cpua3dnow:1;
      unsigned int cpua3dnowa:1;
      unsigned int cpusse3:1;
      unsigned int cpupadlock:1;
      unsigned int cpusvme:1;
      unsigned int cpuvmx:1;
      unsigned int cpusmx:1;
      unsigned int cpussse3:1;
      unsigned int cpusse4a:1;
      unsigned int cpulzcnt:1;
      unsigned int cpupopcnt:1;
      unsigned int cpumonitor:1;
      unsigned int cpusse4_1:1;
      unsigned int cpusse4_2:1;
      unsigned int cpuavx:1;
      unsigned int cpuavx2:1;
      unsigned int cpuavx512f:1;
      unsigned int cpuavx512cd:1;
      unsigned int cpuavx512er:1;
      unsigned int cpuavx512pf:1;
      unsigned int cpuavx512vl:1;
      unsigned int cpuavx512dq:1;
      unsigned int cpuavx512bw:1;
      unsigned int cpuiamcu:1;
      unsigned int cpuxsave:1;
      unsigned int cpuxsaveopt:1;
      unsigned int cpuaes:1;
      unsigned int cpupclmul:1;
      unsigned int cpufma:1;
      unsigned int cpufma4:1;
      unsigned int cpuxop:1;
      unsigned int cpulwp:1;
      unsigned int cpubmi:1;
      unsigned int cputbm:1;
      unsigned int cpumovbe:1;
      unsigned int cpucx16:1;
      unsigned int cpulahf_sahf:1;
      unsigned int cpuept:1;
      unsigned int cpurdtscp:1;
      unsigned int cpufsgsbase:1;
      unsigned int cpurdrnd:1;
      unsigned int cpuf16c:1;
      unsigned int cpubmi2:1;
      unsigned int cpuhle:1;
      unsigned int cpurtm:1;
      unsigned int cpuinvpcid:1;
      unsigned int cpuvmfunc:1;
      unsigned int cpumpx:1;
      unsigned int cpulm:1;
      unsigned int cpurdseed:1;
      unsigned int cpuadx:1;
      unsigned int cpuprfchw:1;
      unsigned int cpusmap:1;
      unsigned int cpusha:1;
      unsigned int cpuclflushopt:1;
      unsigned int cpuxsaves:1;
      unsigned int cpuxsavec:1;
      unsigned int cpuprefetchwt1:1;
      unsigned int cpuse1:1;
      unsigned int cpuclwb:1;
      unsigned int cpuavx512ifma:1;
      unsigned int cpuavx512vbmi:1;
      unsigned int cpuavx512_4fmaps:1;
      unsigned int cpuavx512_4vnniw:1;
      unsigned int cpuavx512_vpopcntdq:1;
      unsigned int cpuavx512_vbmi2:1;
      unsigned int cpuavx512_vnni:1;
      unsigned int cpuavx512_bitalg:1;
      unsigned int cpuavx512_bf16:1;
      unsigned int cpuavx512_vp2intersect:1;
      unsigned int cputdx:1;
      unsigned int cpuavx_vnni:1;
      unsigned int cpuavx512_fp16:1;
      unsigned int cpuprefetchi:1;
      unsigned int cpuavx_ifma:1;
      unsigned int cpuavx_vnni_int8:1;
      unsigned int cpucmpccxadd:1;
      unsigned int cpuwrmsrns:1;
      unsigned int cpumsrlist:1;
      unsigned int cpuavx_ne_convert:1;
      unsigned int cpurao_int:1;
      unsigned int cpufred:1;
      unsigned int cpulkgs:1;
      unsigned int cpumwaitx:1;
      unsigned int cpuclzero:1;
      unsigned int cpuospke:1;
      unsigned int cpurdpid:1;
      unsigned int cpuptwrite:1;
      unsigned int cpuibt:1;
      unsigned int cpushstk:1;
      unsigned int cpuamx_int8:1;
      unsigned int cpuamx_bf16:1;
      unsigned int cpuamx_fp16:1;
      unsigned int cpuamx_complex:1;
      unsigned int cpuamx_tile:1;
      unsigned int cpugfni:1;
      unsigned int cpuvaes:1;
      unsigned int cpuvpclmulqdq:1;
      unsigned int cpuwbnoinvd:1;
      unsigned int cpupconfig:1;
      unsigned int cpuwaitpkg:1;
      unsigned int cpuuintr:1;
      unsigned int cpucldemote:1;
      unsigned int cpumovdiri:1;
      unsigned int cpumovdir64b:1;
      unsigned int cpuenqcmd:1;
      unsigned int cpuserialize:1;
      unsigned int cpurdpru:1;
      unsigned int cpumcommit:1;
      unsigned int cpusev_es:1;
      unsigned int cputsxldtrk:1;
      unsigned int cpukl:1;
      unsigned int cpuwidekl:1;
      unsigned int cpuhreset:1;
      unsigned int cpuinvlpgb:1;
      unsigned int cputlbsync:1;
      unsigned int cpusnp:1;
      unsigned int cpurmpquery:1;
      /* NOTE: These last three fields need to remain last and in this order. */
      unsigned int cpu64:1;
      unsigned int cpuno64:1;
#ifdef CpuUnused
      unsigned int unused:(CpuNumOfBits - CpuUnused);
#endif
    } bitfield;
  unsigned int array[CpuNumOfUints];
} i386_cpu_flags;

/* Position of opcode_modifier bits.  */

enum
{
  /* has direction bit. */
  D = 0,
  /* set if operands can be both bytes and words/dwords/qwords, encoded the
     canonical way; the base_opcode field should hold the encoding for byte
     operands  */
  W,
  /* load form instruction. Must be placed before store form.  */
  Load,
  /* insn has a modrm byte. */
  Modrm,
  /* special case for jump insns; value has to be 1 */
#define JUMP 1
  /* call and jump */
#define JUMP_DWORD 2
  /* loop and jecxz */
#define JUMP_BYTE 3
  /* special case for intersegment leaps/calls */
#define JUMP_INTERSEGMENT 4
  /* absolute address for jump */
#define JUMP_ABSOLUTE 5
  Jump,
  /* FP insn memory format bit, sized by 0x4 */
  FloatMF,
  /* needs size prefix if in 32-bit mode */
#define SIZE16 1
  /* needs size prefix if in 16-bit mode */
#define SIZE32 2
  /* needs size prefix if in 64-bit mode */
#define SIZE64 3
  Size,
  /* Check that operand sizes match.  */
  CheckOperandSize,
  /* any memory size */
#define ANY_SIZE 1
  /* fake an extra reg operand for clr, imul and special register
     processing for some instructions.  */
#define REG_KLUDGE 2
  /* deprecated fp insn, gets a warning */
#define UGH 3
  /* An implicit xmm0 as the first operand */
#define IMPLICIT_1ST_XMM0 4
  /* The second operand must be a vector register, {x,y,z}mmN, where N is a multiple of 4.
     It implicitly denotes the register group of {x,y,z}mmN - {x,y,z}mm(N + 3).
   */
#define IMPLICIT_QUAD_GROUP 5
  /* Two source operands are swapped.  */
#define SWAP_SOURCES 6
  /* Default mask isn't allowed.  */
#define NO_DEFAULT_MASK 7
  /* Address prefix changes register operand */
#define ADDR_PREFIX_OP_REG 8
  /* Instrucion requires that destination must be distinct from source
     registers.  */
#define DISTINCT_DEST 9
  OperandConstraint,
  /* instruction ignores operand size prefix and in Intel mode ignores
     mnemonic size suffix check.  */
#define IGNORESIZE	1
  /* default insn size depends on mode */
#define DEFAULTSIZE	2
  MnemonicSize,
  /* b suffix on instruction illegal */
  No_bSuf,
  /* w suffix on instruction illegal */
  No_wSuf,
  /* l suffix on instruction illegal */
  No_lSuf,
  /* s suffix on instruction illegal */
  No_sSuf,
  /* q suffix on instruction illegal */
  No_qSuf,
  /* instruction needs FWAIT */
  FWait,
  /* IsString provides for a quick test for string instructions, and
     its actual value also indicates which of the operands (if any)
     requires use of the %es segment.  */
#define IS_STRING_ES_OP0 2
#define IS_STRING_ES_OP1 3
  IsString,
  /* RegMem is for instructions with a modrm byte where the register
     destination operand should be encoded in the mod and regmem fields.
     Normally, it will be encoded in the reg field. We add a RegMem
     flag to indicate that it should be encoded in the regmem field.  */
  RegMem,
  /* quick test if branch instruction is MPX supported */
  BNDPrefixOk,
#define PrefixNone		0
#define PrefixRep		1
#define PrefixHLERelease	2 /* Okay with an XRELEASE (0xf3) prefix. */
#define PrefixNoTrack		3
  /* Prefixes implying "LOCK okay" must come after Lock. All others have
     to come before.  */
#define PrefixLock		4
#define PrefixHLELock		5 /* Okay with a LOCK prefix.  */
#define PrefixHLEAny		6 /* Okay with or without a LOCK prefix.  */
  PrefixOk,
  /* opcode is a prefix */
  IsPrefix,
  /* instruction has extension in 8 bit imm */
  ImmExt,
  /* instruction don't need Rex64 prefix.  */
  NoRex64,
  /* insn has VEX prefix:
	1: 128bit VEX prefix (or operand dependent).
	2: 256bit VEX prefix.
	3: Scalar VEX prefix.
   */
#define VEX128		1
#define VEX256		2
#define VEXScalar	3
  Vex,
  /* How to encode VEX.vvvv:
     0: VEX.vvvv must be 1111b.
     1: VEX.vvvv encodes one of the register operands.
   */
  VexVVVV,
  /* How the VEX.W bit is used:
     0: Set by the REX.W bit.
     1: VEX.W0.  Should always be 0.
     2: VEX.W1.  Should always be 1.
     3: VEX.WIG. The VEX.W bit is ignored.
   */
#define VEXW0	1
#define VEXW1	2
#define VEXWIG	3
  VexW,
  /* Opcode prefix (values chosen to be usable directly in
     VEX/XOP/EVEX pp fields):
     0: None
     1: Add 0x66 opcode prefix.
     2: Add 0xf3 opcode prefix.
     3: Add 0xf2 opcode prefix.
   */
#define PREFIX_NONE	0
#define PREFIX_0X66	1
#define PREFIX_0XF3	2
#define PREFIX_0XF2	3
  OpcodePrefix,
  /* Instruction with a mandatory SIB byte:
	1: 128bit vector register.
	2: 256bit vector register.
	3: 512bit vector register.
   */
#define VECSIB128	1
#define VECSIB256	2
#define VECSIB512	3
#define SIBMEM		4
  SIB,

  /* SSE to AVX support required */
  SSE2AVX,

  /* insn has EVEX prefix:
	1: 512bit EVEX prefix.
	2: 128bit EVEX prefix.
	3: 256bit EVEX prefix.
	4: Length-ignored (LIG) EVEX prefix.
	5: Length determined from actual operands.
	6: L'L = 3 (reserved, .insn only)
   */
#define EVEX512                1
#define EVEX128                2
#define EVEX256                3
#define EVEXLIG                4
#define EVEXDYN                5
#define EVEX_L3                6
  EVex,

  /* AVX512 masking support */
  Masking,

  /* AVX512 broadcast support.  The number of bytes to broadcast is
     1 << (Broadcast - 1):
	1: Byte broadcast.
	2: Word broadcast.
	3: Dword broadcast.
	4: Qword broadcast.
   */
#define BYTE_BROADCAST	1
#define WORD_BROADCAST	2
#define DWORD_BROADCAST	3
#define QWORD_BROADCAST	4
  Broadcast,

  /* Static rounding control is supported.  */
  StaticRounding,

  /* Supress All Exceptions is supported.  */
  SAE,

  /* Compressed Disp8*N attribute.  */
#define DISP8_SHIFT_VL 7
  Disp8MemShift,

  /* Support encoding optimization.  */
  Optimize,

  /* AT&T mnemonic.  */
  ATTMnemonic,
  /* AT&T syntax.  */
  ATTSyntax,
  /* Intel syntax.  */
  IntelSyntax,
  /* ISA64: Don't change the order without other code adjustments.
	0: Common to AMD64 and Intel64.
	1: AMD64.
	2: Intel64.
	3: Only in Intel64.
   */
#define AMD64		1
#define INTEL64		2
#define INTEL64ONLY	3
  ISA64,
  /* The last bitfield in i386_opcode_modifier.  */
  Opcode_Modifier_Num
};

typedef struct i386_opcode_modifier
{
  unsigned int d:1;
  unsigned int w:1;
  unsigned int load:1;
  unsigned int modrm:1;
  unsigned int jump:3;
  unsigned int floatmf:1;
  unsigned int size:2;
  unsigned int checkoperandsize:1;
  unsigned int operandconstraint:4;
  unsigned int mnemonicsize:2;
  unsigned int no_bsuf:1;
  unsigned int no_wsuf:1;
  unsigned int no_lsuf:1;
  unsigned int no_ssuf:1;
  unsigned int no_qsuf:1;
  unsigned int fwait:1;
  unsigned int isstring:2;
  unsigned int regmem:1;
  unsigned int bndprefixok:1;
  unsigned int prefixok:3;
  unsigned int isprefix:1;
  unsigned int immext:1;
  unsigned int norex64:1;
  unsigned int vex:2;
  unsigned int vexvvvv:1;
  unsigned int vexw:2;
  unsigned int opcodeprefix:2;
  unsigned int sib:3;
  unsigned int sse2avx:1;
  unsigned int evex:3;
  unsigned int masking:1;
  unsigned int broadcast:3;
  unsigned int staticrounding:1;
  unsigned int sae:1;
  unsigned int disp8memshift:3;
  unsigned int optimize:1;
  unsigned int attmnemonic:1;
  unsigned int attsyntax:1;
  unsigned int intelsyntax:1;
  unsigned int isa64:2;
} i386_opcode_modifier;

/* Operand classes.  */

#define CLASS_WIDTH 4
enum operand_class
{
  ClassNone,
  Reg, /* GPRs and FP regs, distinguished by operand size */
  SReg, /* Segment register */
  RegCR, /* Control register */
  RegDR, /* Debug register */
  RegTR, /* Test register */
  RegMMX, /* MMX register */
  RegSIMD, /* XMM/YMM/ZMM registers, distinguished by operand size */
  RegMask, /* Vector Mask register */
  RegBND, /* Bound register */
};

/* Special operand instances.  */

#define INSTANCE_WIDTH 3
enum operand_instance
{
  InstanceNone,
  Accum, /* Accumulator %al/%ax/%eax/%rax/%st(0)/%xmm0 */
  RegC,  /* %cl / %cx / %ecx / %rcx, e.g. register to hold shift count */
  RegD,  /* %dl / %dx / %edx / %rdx, e.g. register to hold I/O port addr */
  RegB,  /* %bl / %bx / %ebx / %rbx */
};

/* Position of operand_type bits.  */

enum
{
  /* Class and Instance */
  ClassInstance = CLASS_WIDTH + INSTANCE_WIDTH - 1,
  /* 1 bit immediate */
  Imm1,
  /* 8 bit immediate */
  Imm8,
  /* 8 bit immediate sign extended */
  Imm8S,
  /* 16 bit immediate */
  Imm16,
  /* 32 bit immediate */
  Imm32,
  /* 32 bit immediate sign extended */
  Imm32S,
  /* 64 bit immediate */
  Imm64,
  /* 8bit/16bit/32bit displacements are used in different ways,
     depending on the instruction.  For jumps, they specify the
     size of the PC relative displacement, for instructions with
     memory operand, they specify the size of the offset relative
     to the base register, and for instructions with memory offset
     such as `mov 1234,%al' they specify the size of the offset
     relative to the segment base.  */
  /* 8 bit displacement */
  Disp8,
  /* 16 bit displacement */
  Disp16,
  /* 32 bit displacement (64-bit: sign-extended) */
  Disp32,
  /* 64 bit displacement */
  Disp64,
  /* Register which can be used for base or index in memory operand.  */
  BaseIndex,
  /* BYTE size. */
  Byte,
  /* WORD size. 2 byte */
  Word,
  /* DWORD size. 4 byte */
  Dword,
  /* FWORD size. 6 byte */
  Fword,
  /* QWORD size. 8 byte */
  Qword,
  /* TBYTE size. 10 byte */
  Tbyte,
  /* XMMWORD size. */
  Xmmword,
  /* YMMWORD size. */
  Ymmword,
  /* ZMMWORD size.  */
  Zmmword,
  /* TMMWORD size.  */
  Tmmword,
  /* Unspecified memory size.  */
  Unspecified,

  /* The number of bits in i386_operand_type.  */
  OTNum
};

#define OTNumOfUints \
  ((OTNum - 1) / sizeof (unsigned int) / CHAR_BIT + 1)
#define OTNumOfBits \
  (OTNumOfUints * sizeof (unsigned int) * CHAR_BIT)

/* If you get a compiler error for zero width of the unused field,
   comment it out.  */
#define OTUnused		OTNum

typedef union i386_operand_type
{
  struct
    {
      unsigned int class:CLASS_WIDTH;
      unsigned int instance:INSTANCE_WIDTH;
      unsigned int imm1:1;
      unsigned int imm8:1;
      unsigned int imm8s:1;
      unsigned int imm16:1;
      unsigned int imm32:1;
      unsigned int imm32s:1;
      unsigned int imm64:1;
      unsigned int disp8:1;
      unsigned int disp16:1;
      unsigned int disp32:1;
      unsigned int disp64:1;
      unsigned int baseindex:1;
      unsigned int byte:1;
      unsigned int word:1;
      unsigned int dword:1;
      unsigned int fword:1;
      unsigned int qword:1;
      unsigned int tbyte:1;
      unsigned int xmmword:1;
      unsigned int ymmword:1;
      unsigned int zmmword:1;
      unsigned int tmmword:1;
      unsigned int unspecified:1;
#ifdef OTUnused
      unsigned int unused:(OTNumOfBits - OTUnused);
#endif
    } bitfield;
  unsigned int array[OTNumOfUints];
} i386_operand_type;

typedef struct insn_template
{
  /* instruction name sans width suffix ("mov" for movl insns) */
  unsigned int mnem_off;

  /* Bitfield arrangement is such that individual fields can be easily
     extracted (in native builds at least) - either by at most a masking
     operation (base_opcode, operands), or by just a (signed) right shift
     (extension_opcode).  Please try to maintain this property.  */

  /* base_opcode is the fundamental opcode byte without optional
     prefix(es).  */
  unsigned int base_opcode:16;
#define Opcode_D	0x2 /* Direction bit:
			       set if Reg --> Regmem;
			       unset if Regmem --> Reg. */
#define Opcode_FloatR	0x8 /* ModR/M bit to swap src/dest for float insns. */
#define Opcode_FloatD   0x4 /* Direction bit for float insns. */
#define Opcode_ExtD	0x1 /* Direction bit for extended opcode space insns. */
#define Opcode_SIMD_IntD 0x10 /* Direction bit for SIMD int insns. */
/* The next value is arbitrary, as long as it's non-zero and distinct
   from all other values above.  */
#define Opcode_VexW	0xf /* Operand order controlled by VEX.W. */

  /* how many operands */
  unsigned int operands:3;

  /* opcode space */
  unsigned int opcode_space:4;
  /* Opcode encoding space (values chosen to be usable directly in
     VEX/XOP mmmmm and EVEX mm fields):
     0: Base opcode space.
     1: 0F opcode prefix / space.
     2: 0F38 opcode prefix / space.
     3: 0F3A opcode prefix / space.
     5: EVEXMAP5 opcode prefix / space.
     6: EVEXMAP6 opcode prefix / space.
     8: XOP 08 opcode space.
     9: XOP 09 opcode space.
     A: XOP 0A opcode space.
   */
#define SPACE_BASE	0
#define SPACE_0F	1
#define SPACE_0F38	2
#define SPACE_0F3A	3
#define SPACE_EVEXMAP5	5
#define SPACE_EVEXMAP6	6
#define SPACE_XOP08	8
#define SPACE_XOP09	9
#define SPACE_XOP0A	0xA

/* (Fake) base opcode value for pseudo prefixes.  */
#define PSEUDO_PREFIX 0

  /* extension_opcode is the 3 bit extension for group <n> insns.
     This field is also used to store the 8-bit opcode suffix for the
     AMD 3DNow! instructions.
     If this template has no extension opcode (the usual case) use None
     Instructions */
  signed int extension_opcode:9;
#define None (-1)		/* If no extension_opcode is possible.  */

/* Pseudo prefixes.  */
#define Prefix_Disp8		0	/* {disp8} */
#define Prefix_Disp16		1	/* {disp16} */
#define Prefix_Disp32		2	/* {disp32} */
#define Prefix_Load		3	/* {load} */
#define Prefix_Store		4	/* {store} */
#define Prefix_VEX		5	/* {vex} */
#define Prefix_VEX3		6	/* {vex3} */
#define Prefix_EVEX		7	/* {evex} */
#define Prefix_REX		8	/* {rex} */
#define Prefix_NoOptimize	9	/* {nooptimize} */

  /* the bits in opcode_modifier are used to generate the final opcode from
     the base_opcode.  These bits also are used to detect alternate forms of
     the same instruction */
  i386_opcode_modifier opcode_modifier;

  /* cpu feature flags */
  i386_cpu_flags cpu_flags;

  /* operand_types[i] describes the type of operand i.  This is made
     by OR'ing together all of the possible type masks.  (e.g.
     'operand_types[i] = Reg|Imm' specifies that operand i can be
     either a register or an immediate operand.  */
  i386_operand_type operand_types[MAX_OPERANDS];
}
insn_template;

/* these are for register name --> number & type hash lookup */
typedef struct
{
  char reg_name[8];
  i386_operand_type reg_type;
  unsigned char reg_flags;
#define RegRex	    0x1  /* Extended register.  */
#define RegRex64    0x2  /* Extended 8 bit register.  */
#define RegVRex	    0x4  /* Extended vector register.  */
  unsigned char reg_num;
#define RegIP	((unsigned char ) ~0)
/* EIZ and RIZ are fake index registers.  */
#define RegIZ	(RegIP - 1)
/* FLAT is a fake segment register (Intel mode).  */
#define RegFlat     ((unsigned char) ~0)
  signed char dw2_regnum[2];
#define Dw2Inval (-1)
}
reg_entry;
