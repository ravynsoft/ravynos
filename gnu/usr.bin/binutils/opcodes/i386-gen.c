/* Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

#include "sysdep.h"
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include "getopt.h"
#include "libiberty.h"
#include "hashtab.h"
#include "safe-ctype.h"

#include "i386-opc.h"

/* Build-time checks are preferrable over runtime ones.  Use this construct
   in preference where possible.  */
#define static_assert(e) ((void)sizeof (struct { int _:1 - 2 * !(e); }))

static const char *program_name = NULL;
static int debug = 0;

typedef struct dependency
{
  const char *name;
  /* Note: Only direct dependencies should be enumerated.  */
  const char *deps;
} dependency;

static const dependency isa_dependencies[] =
{
  { "UNKNOWN",
    "~IAMCU" },
  { "GENERIC32",
    "386" },
  { "GENERIC64",
    "PENTIUMPRO|Clflush|SYSCALL|MMX|SSE2|LM" },
  { "NONE",
    "0" },
  { "PENTIUMPRO",
    "686|Nop" },
  { "P2",
    "PENTIUMPRO|MMX" },
  { "P3",
    "P2|SSE" },
  { "P4",
    "P3|Clflush|SSE2" },
  { "NOCONA",
    "GENERIC64|FISTTP|SSE3|MONITOR|CX16" },
  { "CORE",
    "P4|FISTTP|SSE3|MONITOR|CX16" },
  { "CORE2",
    "NOCONA|SSSE3" },
  { "COREI7",
    "CORE2|SSE4_2|Rdtscp|LAHF_SAHF" },
  { "K6",
    "186|286|386|486|586|SYSCALL|387|MMX" },
  { "K6_2",
    "K6|3dnow" },
  { "ATHLON",
    "K6_2|686:min|687|Nop|3dnowA" },
  { "K8",
    "ATHLON|Rdtscp|SSE2|LM" },
  { "AMDFAM10",
    "K8|FISTTP|SSE4A|ABM|MONITOR" },
  { "BDVER1",
    "GENERIC64|FISTTP|Rdtscp|MONITOR|CX16|LAHF_SAHF|XOP|ABM|LWP|SVME|AES|PCLMUL|PRFCHW" },
  { "BDVER2",
    "BDVER1|FMA|BMI|TBM|F16C" },
  { "BDVER3",
    "BDVER2|Xsaveopt|FSGSBase" },
  { "BDVER4",
    "BDVER3|AVX2|Movbe|BMI2|RdRnd|MWAITX" },
  { "ZNVER1",
    "GENERIC64|FISTTP|Rdtscp|MONITOR|CX16|LAHF_SAHF|AVX2|SSE4A|ABM|SVME|AES|PCLMUL|PRFCHW|FMA|BMI|F16C|Xsaveopt|FSGSBase|Movbe|BMI2|RdRnd|ADX|RdSeed|SMAP|SHA|XSAVEC|XSAVES|ClflushOpt|CLZERO|MWAITX" },
  { "ZNVER2",
    "ZNVER1|CLWB|RDPID|RDPRU|MCOMMIT|WBNOINVD" },
  { "ZNVER3",
    "ZNVER2|INVLPGB|TLBSYNC|VAES|VPCLMULQDQ|INVPCID|SNP|OSPKE" },
  { "ZNVER4",
    "ZNVER3|AVX512F|AVX512DQ|AVX512IFMA|AVX512CD|AVX512BW|AVX512VL|AVX512_BF16|AVX512VBMI|AVX512_VBMI2|AVX512_VNNI|AVX512_BITALG|AVX512_VPOPCNTDQ|GFNI|RMPQUERY" },
  { "BTVER1",
    "GENERIC64|FISTTP|MONITOR|CX16|LAHF_SAHF|Rdtscp|SSSE3|SSE4A|ABM|PRFCHW|Clflush|FISTTP|SVME" },
  { "BTVER2",
    "BTVER1|AVX|BMI|F16C|AES|PCLMUL|Movbe|Xsaveopt|PRFCHW" },
  { "286",
    "186" },
  { "386",
    "286" },
  { "486",
    "386" },
  { "586",
    "486|387" },
  { "586:nofpu",
    "486" },
  { "686",
    "586|687|CMOV|FXSR" },
  { "686:min",
    "586|687" },
  { "687",
    "387" },
  { "FISTTP",
    "687" },
  { "SSE",
    "FXSR" },
  { "SSE2",
    "SSE" },
  { "SSE3",
    "SSE2" },
  { "SSSE3",
    "SSE3" },
  { "SSE4_1",
    "SSSE3" },
  { "SSE4_2",
    "SSE4_1|POPCNT" },
  { "Xsaveopt",
    "XSAVE" },
  { "AES",
    "SSE2" },
  { "PCLMUL",
    "SSE2" },
  { "FMA",
    "AVX" },
  { "FMA4",
    "AVX" },
  { "XOP",
    "SSE4A|FMA4" },
  { "LWP",
    "XSAVE" },
  { "F16C",
    "AVX" },
  { "3dnow",
    "MMX" },
  { "3dnowA",
    "3dnow" },
  { "SSE4a",
    "SSE3" },
  { "ABM",
    "LZCNT|POPCNT" },
  { "AVX",
    "SSE4_2|XSAVE" },
  { "AVX2",
    "AVX" },
  { "AVX_VNNI",
    "AVX2" },
  { "AVX_IFMA",
    "AVX2" },
  { "AVX_VNNI_INT8",
    "AVX2" },
  { "AVX_NE_CONVERT",
    "AVX2" },
  { "FRED",
    "LKGS" },
  { "AVX512F",
    "AVX2" },
  { "AVX512CD",
    "AVX512F" },
  { "AVX512ER",
    "AVX512F" },
  { "AVX512PF",
    "AVX512F" },
  { "AVX512DQ",
    "AVX512F" },
  { "AVX512BW",
    "AVX512F" },
  { "AVX512VL",
    "AVX512F" },
  { "AVX512IFMA",
    "AVX512F" },
  { "AVX512VBMI",
    "AVX512BW" },
  { "AVX512_4FMAPS",
    "AVX512F" },
  { "AVX512_4VNNIW",
    "AVX512F" },
  { "AVX512_VPOPCNTDQ",
    "AVX512F" },
  { "AVX512_VBMI2",
    "AVX512BW" },
  { "AVX512_VNNI",
    "AVX512F" },
  { "AVX512_BITALG",
    "AVX512BW" },
  { "AVX512_VP2INTERSECT",
    "AVX512F" },
  { "AVX512_BF16",
    "AVX512BW" },
  { "AVX512_FP16",
    "AVX512BW" },
  { "IAMCU",
    "586:nofpu" },
  { "EPT",
    "VMX" },
  { "VMFUNC",
    "VMX" },
  { "MPX",
    "XSAVE" },
  { "SHA",
    "SSE2" },
  { "XSAVES",
    "XSAVEC" },
  { "XSAVEC",
    "XSAVE" },
  { "OSPKE",
    "XSAVE" },
  { "GFNI",
    "SSE2" },
  { "VAES",
    "AVX2" },
  { "VPCLMULQDQ",
    "AVX2" },
  { "SEV_ES",
    "SVME" },
  { "SNP",
    "SEV_ES" },
  { "RMPQUERY",
    "SNP" },
  { "TSX",
    "RTM|HLE" },
  { "TSXLDTRK",
    "RTM" },
  { "AMX_TILE",
    "XSAVE" },
  { "AMX_INT8",
    "AMX_TILE" },
  { "AMX_BF16",
    "AMX_TILE" },
  { "AMX_FP16",
    "AMX_TILE" },
  { "AMX_COMPLEX",
    "AMX_TILE" },
  { "KL",
    "SSE2" },
  { "WIDEKL",
    "KL" },
};

/* This array is populated as process_i386_initializers() walks cpu_flags[].  */
static unsigned char isa_reverse_deps[Cpu64][Cpu64];

typedef struct bitfield
{
  int position;
  int value;
  const char *name;
} bitfield;

#define BITFIELD(n) { Cpu##n, 0, #n }

static bitfield cpu_flags[] =
{
  BITFIELD (186),
  BITFIELD (286),
  BITFIELD (386),
  BITFIELD (486),
  BITFIELD (586),
  BITFIELD (686),
  BITFIELD (CMOV),
  BITFIELD (FXSR),
  BITFIELD (Clflush),
  BITFIELD (Nop),
  BITFIELD (SYSCALL),
  BITFIELD (8087),
  BITFIELD (287),
  BITFIELD (387),
  BITFIELD (687),
  BITFIELD (FISTTP),
  BITFIELD (MMX),
  BITFIELD (SSE),
  BITFIELD (SSE2),
  BITFIELD (SSE3),
  BITFIELD (SSSE3),
  BITFIELD (SSE4_1),
  BITFIELD (SSE4_2),
  BITFIELD (AVX),
  BITFIELD (AVX2),
  BITFIELD (AVX512F),
  BITFIELD (AVX512CD),
  BITFIELD (AVX512ER),
  BITFIELD (AVX512PF),
  BITFIELD (AVX512VL),
  BITFIELD (AVX512DQ),
  BITFIELD (AVX512BW),
  BITFIELD (IAMCU),
  BITFIELD (SSE4a),
  BITFIELD (3dnow),
  BITFIELD (3dnowA),
  BITFIELD (PadLock),
  BITFIELD (SVME),
  BITFIELD (VMX),
  BITFIELD (SMX),
  BITFIELD (Xsave),
  BITFIELD (Xsaveopt),
  BITFIELD (AES),
  BITFIELD (PCLMUL),
  BITFIELD (FMA),
  BITFIELD (FMA4),
  BITFIELD (XOP),
  BITFIELD (LWP),
  BITFIELD (BMI),
  BITFIELD (TBM),
  BITFIELD (LM),
  BITFIELD (Movbe),
  BITFIELD (CX16),
  BITFIELD (LAHF_SAHF),
  BITFIELD (EPT),
  BITFIELD (Rdtscp),
  BITFIELD (FSGSBase),
  BITFIELD (RdRnd),
  BITFIELD (F16C),
  BITFIELD (BMI2),
  BITFIELD (LZCNT),
  BITFIELD (POPCNT),
  BITFIELD (MONITOR),
  BITFIELD (HLE),
  BITFIELD (RTM),
  BITFIELD (INVPCID),
  BITFIELD (VMFUNC),
  BITFIELD (RDSEED),
  BITFIELD (ADX),
  BITFIELD (PRFCHW),
  BITFIELD (SMAP),
  BITFIELD (SHA),
  BITFIELD (ClflushOpt),
  BITFIELD (XSAVES),
  BITFIELD (XSAVEC),
  BITFIELD (PREFETCHWT1),
  BITFIELD (SE1),
  BITFIELD (CLWB),
  BITFIELD (MPX),
  BITFIELD (AVX512IFMA),
  BITFIELD (AVX512VBMI),
  BITFIELD (AVX512_4FMAPS),
  BITFIELD (AVX512_4VNNIW),
  BITFIELD (AVX512_VPOPCNTDQ),
  BITFIELD (AVX512_VBMI2),
  BITFIELD (AVX512_VNNI),
  BITFIELD (AVX512_BITALG),
  BITFIELD (AVX512_BF16),
  BITFIELD (AVX512_VP2INTERSECT),
  BITFIELD (TDX),
  BITFIELD (AVX_VNNI),
  BITFIELD (AVX512_FP16),
  BITFIELD (PREFETCHI),
  BITFIELD (AVX_IFMA),
  BITFIELD (AVX_VNNI_INT8),
  BITFIELD (CMPCCXADD),
  BITFIELD (WRMSRNS),
  BITFIELD (MSRLIST),
  BITFIELD (AVX_NE_CONVERT),
  BITFIELD (RAO_INT),
  BITFIELD (FRED),
  BITFIELD (LKGS),
  BITFIELD (MWAITX),
  BITFIELD (CLZERO),
  BITFIELD (OSPKE),
  BITFIELD (RDPID),
  BITFIELD (PTWRITE),
  BITFIELD (IBT),
  BITFIELD (SHSTK),
  BITFIELD (GFNI),
  BITFIELD (VAES),
  BITFIELD (VPCLMULQDQ),
  BITFIELD (WBNOINVD),
  BITFIELD (PCONFIG),
  BITFIELD (WAITPKG),
  BITFIELD (UINTR),
  BITFIELD (CLDEMOTE),
  BITFIELD (AMX_INT8),
  BITFIELD (AMX_BF16),
  BITFIELD (AMX_FP16),
  BITFIELD (AMX_COMPLEX),
  BITFIELD (AMX_TILE),
  BITFIELD (MOVDIRI),
  BITFIELD (MOVDIR64B),
  BITFIELD (ENQCMD),
  BITFIELD (SERIALIZE),
  BITFIELD (RDPRU),
  BITFIELD (MCOMMIT),
  BITFIELD (SEV_ES),
  BITFIELD (TSXLDTRK),
  BITFIELD (KL),
  BITFIELD (WideKL),
  BITFIELD (HRESET),
  BITFIELD (INVLPGB),
  BITFIELD (TLBSYNC),
  BITFIELD (SNP),
  BITFIELD (RMPQUERY),
  BITFIELD (64),
  BITFIELD (No64),
#ifdef CpuUnused
  BITFIELD (Unused),
#endif
};

#undef BITFIELD
#define BITFIELD(n) { n, 0, #n }

static bitfield opcode_modifiers[] =
{
  BITFIELD (D),
  BITFIELD (W),
  BITFIELD (Load),
  BITFIELD (Modrm),
  BITFIELD (Jump),
  BITFIELD (FloatMF),
  BITFIELD (Size),
  BITFIELD (CheckOperandSize),
  BITFIELD (OperandConstraint),
  BITFIELD (MnemonicSize),
  BITFIELD (No_bSuf),
  BITFIELD (No_wSuf),
  BITFIELD (No_lSuf),
  BITFIELD (No_sSuf),
  BITFIELD (No_qSuf),
  BITFIELD (FWait),
  BITFIELD (IsString),
  BITFIELD (RegMem),
  BITFIELD (BNDPrefixOk),
  BITFIELD (PrefixOk),
  BITFIELD (IsPrefix),
  BITFIELD (ImmExt),
  BITFIELD (NoRex64),
  BITFIELD (Vex),
  BITFIELD (VexVVVV),
  BITFIELD (VexW),
  BITFIELD (OpcodePrefix),
  BITFIELD (SIB),
  BITFIELD (SSE2AVX),
  BITFIELD (EVex),
  BITFIELD (Masking),
  BITFIELD (Broadcast),
  BITFIELD (StaticRounding),
  BITFIELD (SAE),
  BITFIELD (Disp8MemShift),
  BITFIELD (Optimize),
  BITFIELD (ATTMnemonic),
  BITFIELD (ATTSyntax),
  BITFIELD (IntelSyntax),
  BITFIELD (ISA64),
};

#define CLASS(n) #n, n

static const struct {
  const char *name;
  enum operand_class value;
} operand_classes[] = {
  CLASS (Reg),
  CLASS (SReg),
  CLASS (RegCR),
  CLASS (RegDR),
  CLASS (RegTR),
  CLASS (RegMMX),
  CLASS (RegSIMD),
  CLASS (RegMask),
  CLASS (RegBND),
};

#undef CLASS

#define INSTANCE(n) #n, n

static const struct {
  const char *name;
  enum operand_instance value;
} operand_instances[] = {
    INSTANCE (Accum),
    INSTANCE (RegC),
    INSTANCE (RegD),
    INSTANCE (RegB),
};

#undef INSTANCE

static bitfield operand_types[] =
{
  BITFIELD (Imm1),
  BITFIELD (Imm8),
  BITFIELD (Imm8S),
  BITFIELD (Imm16),
  BITFIELD (Imm32),
  BITFIELD (Imm32S),
  BITFIELD (Imm64),
  BITFIELD (BaseIndex),
  BITFIELD (Disp8),
  BITFIELD (Disp16),
  BITFIELD (Disp32),
  BITFIELD (Disp64),
  BITFIELD (Byte),
  BITFIELD (Word),
  BITFIELD (Dword),
  BITFIELD (Fword),
  BITFIELD (Qword),
  BITFIELD (Tbyte),
  BITFIELD (Xmmword),
  BITFIELD (Ymmword),
  BITFIELD (Zmmword),
  BITFIELD (Tmmword),
  BITFIELD (Unspecified),
#ifdef OTUnused
  BITFIELD (OTUnused),
#endif
};

static const char *filename;
static i386_cpu_flags active_cpu_flags;
static int active_isstring;

struct template_arg {
  const struct template_arg *next;
  const char *val;
};

struct template_instance {
  const struct template_instance *next;
  const char *name;
  const struct template_arg *args;
};

struct template_param {
  const struct template_param *next;
  const char *name;
};

struct template {
  struct template *next;
  const char *name;
  const struct template_instance *instances;
  const struct template_param *params;
};

static struct template *templates;

static int
compare (const void *x, const void *y)
{
  const bitfield *xp = (const bitfield *) x;
  const bitfield *yp = (const bitfield *) y;
  return xp->position - yp->position;
}

static void
fail (const char *message, ...)
{
  va_list args;

  va_start (args, message);
  fprintf (stderr, "%s: error: ", program_name);
  vfprintf (stderr, message, args);
  va_end (args);
  xexit (1);
}

static void
process_copyright (FILE *fp)
{
  fprintf (fp, "/* This file is automatically generated by i386-gen.  Do not edit!  */\n\
/* Copyright (C) 2007-2023 Free Software Foundation, Inc.\n\
\n\
   This file is part of the GNU opcodes library.\n\
\n\
   This library is free software; you can redistribute it and/or modify\n\
   it under the terms of the GNU General Public License as published by\n\
   the Free Software Foundation; either version 3, or (at your option)\n\
   any later version.\n\
\n\
   It is distributed in the hope that it will be useful, but WITHOUT\n\
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY\n\
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public\n\
   License for more details.\n\
\n\
   You should have received a copy of the GNU General Public License\n\
   along with this program; if not, write to the Free Software\n\
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,\n\
   MA 02110-1301, USA.  */\n");
}

/* Remove leading white spaces.  */

static char *
remove_leading_whitespaces (char *str)
{
  while (ISSPACE (*str))
    str++;
  return str;
}

/* Remove trailing white spaces.  */

static void
remove_trailing_whitespaces (char *str)
{
  size_t last = strlen (str);

  if (last == 0)
    return;

  do
    {
      last--;
      if (ISSPACE (str [last]))
	str[last] = '\0';
      else
	break;
    }
  while (last != 0);
}

/* Find next field separated by SEP and terminate it. Return a
   pointer to the one after it.  */

static char *
next_field (char *str, char sep, char **next, char *last)
{
  char *p;

  p = remove_leading_whitespaces (str);
  for (str = p; *str != sep && *str != '\0'; str++);

  *str = '\0';
  remove_trailing_whitespaces (p);

  *next = str + 1;

  if (p >= last)
    abort ();

  return p;
}

static void set_bitfield (char *, bitfield *, int, unsigned int, int);

static void
set_bitfield (char *f, bitfield *array, int value,
	      unsigned int size, int lineno)
{
  unsigned int i;

  /* Ignore empty fields; they may result from template expansions.  */
  if (*f == '\0')
    return;

  for (i = 0; i < size; i++)
    if (strcasecmp (array[i].name, f) == 0)
      {
	array[i].value = value;
	return;
      }

  if (value)
    {
      const char *v = strchr (f, '=');

      if (v)
	{
	  size_t n = v - f;
	  char *end;

	  for (i = 0; i < size; i++)
	    if (strncasecmp (array[i].name, f, n) == 0)
	      {
		value = strtol (v + 1, &end, 0);
		if (*end == '\0')
		  {
		    array[i].value = value;
		    return;
		  }
		break;
	      }
	}
    }

  if (lineno != -1)
    fail ("%s: %d: unknown bitfield: %s\n", filename, lineno, f);
  else
    fail ("unknown bitfield: %s\n", f);
}

static void
add_isa_dependencies (bitfield *flags, const char *f, int value,
		      unsigned int reverse)
{
  unsigned int i;
  char *str = NULL;
  const char *isa = f;
  bool is_isa = false, is_avx = false;

  /* Need to find base entry for references to auxiliary ones.  */
  if (strchr (f, ':'))
    {
      str = xstrdup (f);
      *strchr (str, ':') = '\0';
      isa = str;
    }
  for (i = 0; i < Cpu64; ++i)
    if (strcasecmp (flags[i].name, isa) == 0)
      {
	flags[i].value = value;
	if (reverse < ARRAY_SIZE (isa_reverse_deps[0])
	    /* Don't record the feature itself here.  */
	    && reverse != i
	    /* Don't record base architectures.  */
	    && reverse > Cpu686)
	  isa_reverse_deps[i][reverse] = 1;
	is_isa = true;
	if (i == CpuAVX || i == CpuXOP)
	  is_avx = true;
	break;
      }
  free (str);

  /* Do not turn off dependencies.  */
  if (is_isa && !value)
    return;

  for (i = 0; i < ARRAY_SIZE (isa_dependencies); ++i)
    if (strcasecmp (isa_dependencies[i].name, f) == 0)
      {
	char *deps = xstrdup (isa_dependencies[i].deps);
	char *next = deps;
	char *last = deps + strlen (deps);

	for (; next && next < last; )
	  {
	    char *str = next_field (next, '|', &next, last);

	    /* No AVX/XOP -> SSE reverse dependencies.  */
	    if (is_avx && strncmp (str, "SSE", 3) == 0)
	      add_isa_dependencies (flags, str, value, CpuMax);
	    else
	      add_isa_dependencies (flags, str, value, reverse);
	  }
	free (deps);

	/* ISA extensions with dependencies need CPU_ANY_*_FLAGS emitted.  */
	if (reverse < ARRAY_SIZE (isa_reverse_deps[0]))
	  isa_reverse_deps[reverse][reverse] = 1;

	return;
      }

  if (!is_isa)
    fail ("unknown bitfield: %s\n", f);
}

static void
output_cpu_flags (FILE *table, bitfield *flags, unsigned int size,
		  int macro, const char *comma, const char *indent)
{
  unsigned int i;

  memset (&active_cpu_flags, 0, sizeof(active_cpu_flags));

  fprintf (table, "%s{ { ", indent);

  for (i = 0; i < size - 1; i++)
    {
      if (((i + 1) % 20) != 0)
	fprintf (table, "%d, ", flags[i].value);
      else
	fprintf (table, "%d,", flags[i].value);
      if (((i + 1) % 20) == 0)
	{
	  /* We need \\ for macro.  */
	  if (macro)
	    fprintf (table, " \\\n    %s", indent);
	  else
	    fprintf (table, "\n    %s", indent);
	}
      if (flags[i].value)
	active_cpu_flags.array[i / 32] |= 1U << (i % 32);
    }

  fprintf (table, "%d } }%s\n", flags[i].value, comma);
}

static void
process_i386_cpu_flag (FILE *table, char *flag,
		       const char *name,
		       const char *comma, const char *indent,
		       int lineno, unsigned int reverse)
{
  char *str, *next = flag, *last;
  unsigned int i;
  int value = 1;
  bool is_isa = false;
  bitfield flags [ARRAY_SIZE (cpu_flags)];

  /* Copy the default cpu flags.  */
  memcpy (flags, cpu_flags, sizeof (cpu_flags));

  if (flag == NULL)
    {
      for (i = 0; i < ARRAY_SIZE (isa_reverse_deps[0]); ++i)
	flags[i].value = isa_reverse_deps[reverse][i];
      goto output;
    }

  if (flag[0] == '~')
    {
      last = flag + strlen (flag);

      if (flag[1] == '(')
	{
	  last -= 1;
	  next = flag + 2;
	  if (*last != ')')
	    fail ("%s: %d: missing `)' in bitfield: %s\n", filename,
		  lineno, flag);
	  *last = '\0';
	}
      else
	next = flag + 1;

      /* First we turn on everything except for cpu64, cpuno64, and - if
         present - the padding field.  */
      for (i = 0; i < ARRAY_SIZE (flags); i++)
	if (flags[i].position < Cpu64)
	  flags[i].value = 1;

      /* Turn off selective bits.  */
      value = 0;
    }

  if (name != NULL && value != 0)
    {
      for (i = 0; i < ARRAY_SIZE (flags); i++)
	if (strcasecmp (flags[i].name, name) == 0)
	  {
	    add_isa_dependencies (flags, name, 1, reverse);
	    is_isa = true;
	    break;
	  }
    }

  if (strcmp (flag, "0"))
    {
      if (is_isa)
	return;

      /* Turn on/off selective bits.  */
      last = flag + strlen (flag);
      for (; next && next < last; )
	{
	  str = next_field (next, '|', &next, last);
	  if (name == NULL)
	    set_bitfield (str, flags, value, ARRAY_SIZE (flags), lineno);
	  else
	    add_isa_dependencies (flags, str, value, reverse);
	}
    }

 output:
  if (name != NULL)
    {
      size_t len = strlen (name);
      char *upper = xmalloc (len + 1);

      for (i = 0; i < len; ++i)
	{
	  /* Don't emit #define-s for auxiliary entries.  */
	  if (name[i] == ':')
	    return;
	  upper[i] = TOUPPER (name[i]);
	}
      upper[i] = '\0';
      fprintf (table, "\n#define CPU_%s%s_FLAGS \\\n",
	       flag != NULL ? "": "ANY_", upper);
      free (upper);
    }

  output_cpu_flags (table, flags, ARRAY_SIZE (flags), name != NULL,
		    comma, indent);
}

static void
output_opcode_modifier (FILE *table, bitfield *modifier, unsigned int size)
{
  unsigned int i;

  fprintf (table, "    { ");

  for (i = 0; i < size - 1; i++)
    {
      if (((i + 1) % 20) != 0)
        fprintf (table, "%d, ", modifier[i].value);
      else
        fprintf (table, "%d,", modifier[i].value);
      if (((i + 1) % 20) == 0)
	fprintf (table, "\n      ");
    }

  fprintf (table, "%d },\n", modifier[i].value);
}

/* Returns LOG2 of element size.  */
static int
get_element_size (char **opnd, int lineno)
{
  char *str, *next, *last, *op;
  const char *full = opnd[0];
  int elem_size = INT_MAX;

  /* Find the memory operand.  */
  while (full != NULL && strstr(full, "BaseIndex") == NULL)
    full = *++opnd;
  if (full == NULL)
    fail ("%s: %d: no memory operand\n", filename, lineno);

  op = xstrdup (full);
  last = op + strlen (op);
  for (next = op; next && next < last; )
    {
      str = next_field (next, '|', &next, last);
      if (str)
	{
	  if (strcasecmp(str, "Byte") == 0)
	    {
	      /* The smallest element size, no need to check
		 further.  */
	      elem_size = 0;
	      break;
	    }
	  else if (strcasecmp(str, "Word") == 0)
	    {
	      if (elem_size > 1)
		elem_size = 1;
	    }
	  else if (strcasecmp(str, "Dword") == 0)
	    {
	      if (elem_size > 2)
		elem_size = 2;
	    }
	  else if (strcasecmp(str, "Qword") == 0)
	    {
	      if (elem_size > 3)
		elem_size = 3;
	    }
	}
    }
  free (op);

  if (elem_size == INT_MAX)
    fail ("%s: %d: unknown element size: %s\n", filename, lineno, full);

  return elem_size;
}

static void
process_i386_opcode_modifier (FILE *table, char *mod, unsigned int space,
			      unsigned int prefix, const char *extension_opcode,
			      char **opnd, int lineno)
{
  char *str, *next, *last;
  bitfield modifiers [ARRAY_SIZE (opcode_modifiers)];
  static const char *const spaces[] = {
#define SPACE(n) [SPACE_##n] = #n
    SPACE(BASE),
    SPACE(0F),
    SPACE(0F38),
    SPACE(0F3A),
    SPACE(EVEXMAP5),
    SPACE(EVEXMAP6),
    SPACE(XOP08),
    SPACE(XOP09),
    SPACE(XOP0A),
#undef SPACE
  };

  active_isstring = 0;

  /* Copy the default opcode modifier.  */
  memcpy (modifiers, opcode_modifiers, sizeof (modifiers));

  if (strcmp (mod, "0"))
    {
      unsigned int have_w = 0, bwlq_suf = 0xf;

      last = mod + strlen (mod);
      for (next = mod; next && next < last; )
	{
	  str = next_field (next, '|', &next, last);
	  if (str)
	    {
	      int val = 1;

	      if (strncmp(str, "OpcodeSpace", 11) == 0)
		{
		  char *end;

		  if (str[11] != '=')
		    fail ("%s:%d: Missing value for `OpcodeSpace'\n",
			  filename, lineno);

		  val = strtol (str + 12, &end, 0);
		  if (*end)
		    fail ("%s:%d: Bogus value `%s' for `OpcodeSpace'\n",
			  filename, lineno, end);

		  if (space)
		    {
		      if (val != space)
			fail ("%s:%d: Conflicting opcode space specifications\n",
			      filename, lineno);
		      fprintf (stderr,
			       "%s:%d: Warning: redundant opcode space specification\n",
			       filename, lineno);
		    }

		  space = val;
		  continue;
		}

	      if (strcasecmp(str, "Broadcast") == 0)
		val = get_element_size (opnd, lineno) + BYTE_BROADCAST;
	      else if (strcasecmp(str, "Disp8MemShift") == 0)
		val = get_element_size (opnd, lineno);

	      set_bitfield (str, modifiers, val, ARRAY_SIZE (modifiers),
			    lineno);
	      if (strcasecmp(str, "IsString") == 0)
		active_isstring = 1;

	      if (strcasecmp(str, "W") == 0)
		have_w = 1;

	      if (strcasecmp(str, "No_bSuf") == 0)
		bwlq_suf &= ~1;
	      if (strcasecmp(str, "No_wSuf") == 0)
		bwlq_suf &= ~2;
	      if (strcasecmp(str, "No_lSuf") == 0)
		bwlq_suf &= ~4;
	      if (strcasecmp(str, "No_qSuf") == 0)
		bwlq_suf &= ~8;
	    }
	}

      if (prefix)
	{
	  if (!modifiers[OpcodePrefix].value)
	    modifiers[OpcodePrefix].value = prefix;
	  else if (modifiers[OpcodePrefix].value != prefix)
	    fail ("%s:%d: Conflicting prefix specifications\n",
		  filename, lineno);
	  else
	    fprintf (stderr,
		     "%s:%d: Warning: redundant prefix specification\n",
		     filename, lineno);
	}

      if (have_w && !bwlq_suf)
	fail ("%s: %d: stray W modifier\n", filename, lineno);
      if (have_w && !(bwlq_suf & 1))
	fprintf (stderr, "%s: %d: W modifier without Byte operand(s)\n",
		 filename, lineno);
      if (have_w && !(bwlq_suf & ~1))
	fprintf (stderr,
		 "%s: %d: W modifier without Word/Dword/Qword operand(s)\n",
		 filename, lineno);
    }

  if (space >= ARRAY_SIZE (spaces) || !spaces[space])
    fail ("%s:%d: Unknown opcode space %u\n", filename, lineno, space);

  fprintf (table, " SPACE_%s, %s,\n",
	   spaces[space], extension_opcode ? extension_opcode : "None");

  output_opcode_modifier (table, modifiers, ARRAY_SIZE (modifiers));
}

enum stage {
  stage_macros,
  stage_opcodes,
  stage_registers,
};

static void
output_operand_type (FILE *table, enum operand_class class,
		     enum operand_instance instance,
		     const bitfield *types, unsigned int size,
		     enum stage stage, const char *indent)
{
  unsigned int i;

  fprintf (table, "{ { %d, %d, ", class, instance);

  for (i = 0; i < size - 1; i++)
    {
      if (((i + 3) % 20) != 0)
	fprintf (table, "%d, ", types[i].value);
      else
	fprintf (table, "%d,", types[i].value);
      if (((i + 3) % 20) == 0)
	{
	  /* We need \\ for macro.  */
	  if (stage == stage_macros)
	    fprintf (table, " \\\n%s", indent);
	  else
	    fprintf (table, "\n%s", indent);
	}
    }

  fprintf (table, "%d } }", types[i].value);
}

static void
process_i386_operand_type (FILE *table, char *op, enum stage stage,
			   const char *indent, int lineno)
{
  char *str, *next, *last;
  enum operand_class class = ClassNone;
  enum operand_instance instance = InstanceNone;
  bitfield types [ARRAY_SIZE (operand_types)];

  /* Copy the default operand type.  */
  memcpy (types, operand_types, sizeof (types));

  if (strcmp (op, "0"))
    {
      int baseindex = 0;

      last = op + strlen (op);
      for (next = op; next && next < last; )
	{
	  str = next_field (next, '|', &next, last);
	  if (str)
	    {
	      unsigned int i;

	      if (!strncmp(str, "Class=", 6))
		{
		  for (i = 0; i < ARRAY_SIZE(operand_classes); ++i)
		    if (!strcmp(str + 6, operand_classes[i].name))
		      {
			class = operand_classes[i].value;
			str = NULL;
			break;
		      }
		}

	      if (str && !strncmp(str, "Instance=", 9))
		{
		  for (i = 0; i < ARRAY_SIZE(operand_instances); ++i)
		    if (!strcmp(str + 9, operand_instances[i].name))
		      {
			instance = operand_instances[i].value;
			str = NULL;
			break;
		      }
		}
	    }
	  if (str)
	    {
	      set_bitfield (str, types, 1, ARRAY_SIZE (types), lineno);
	      if (strcasecmp(str, "BaseIndex") == 0)
		baseindex = 1;
	    }
	}

      if (stage == stage_opcodes && baseindex && !active_isstring)
	{
	  set_bitfield("Disp8", types, 1, ARRAY_SIZE (types), lineno);
	  if (!active_cpu_flags.bitfield.cpu64
	      && !active_cpu_flags.bitfield.cpumpx)
	    set_bitfield("Disp16", types, 1, ARRAY_SIZE (types), lineno);
	  set_bitfield("Disp32", types, 1, ARRAY_SIZE (types), lineno);
	}
    }
  output_operand_type (table, class, instance, types, ARRAY_SIZE (types),
		       stage, indent);
}

static char *mkident (const char *mnem)
{
  char *ident = xstrdup (mnem), *p = ident;

  do
    {
      if (!ISALNUM (*p))
	*p = '_';
    }
  while (*++p);

  return ident;
}

static void
output_i386_opcode (FILE *table, const char *name, char *str,
		    char *last, int lineno)
{
  unsigned int i, length, prefix = 0, space = 0;
  char *base_opcode, *extension_opcode, *end, *ident;
  char *cpu_flags, *opcode_modifier, *operand_types [MAX_OPERANDS];
  unsigned long long opcode;

  /* Find base_opcode.  */
  base_opcode = next_field (str, ',', &str, last);

  /* Find extension_opcode, if any.  */
  extension_opcode = strchr (base_opcode, '/');
  if (extension_opcode)
    *extension_opcode++ = '\0';

  /* Find cpu_flags.  */
  cpu_flags = next_field (str, ',', &str, last);

  /* Find opcode_modifier.  */
  opcode_modifier = next_field (str, ',', &str, last);

  /* Remove the first {.  */
  str = remove_leading_whitespaces (str);
  if (*str != '{')
    abort ();
  str = remove_leading_whitespaces (str + 1);
  remove_trailing_whitespaces (str);

  /* Remove } and trailing white space. */
  i = strlen (str);
  if (!i || str[i - 1] != '}')
    abort ();
  str[--i] = '\0';
  remove_trailing_whitespaces (str);

  if (!*str)
    operand_types [i = 0] = NULL;
  else
    {
      last = str + strlen (str);

      /* Find operand_types.  */
      for (i = 0; i < ARRAY_SIZE (operand_types); i++)
	{
	  if (str >= last)
	    {
	      operand_types [i] = NULL;
	      break;
	    }

	  operand_types [i] = next_field (str, ',', &str, last);
	}
    }

  opcode = strtoull (base_opcode, &end, 0);

  /* Determine opcode length.  */
  for (length = 1; length < 8; ++length)
    if (!(opcode >> (8 * length)))
       break;

  /* Transform prefixes encoded in the opcode into opcode modifier
     representation.  */
  if (length > 1)
    {
      switch (opcode >> (8 * length - 8))
	{
	case 0x66: prefix = PREFIX_0X66; break;
	case 0xF3: prefix = PREFIX_0XF3; break;
	case 0xF2: prefix = PREFIX_0XF2; break;
	}

      if (prefix)
	opcode &= (1ULL << (8 * --length)) - 1;
    }

  /* Transform opcode space encoded in the opcode into opcode modifier
     representation.  */
  if (length > 1 && (opcode >> (8 * length - 8)) == 0xf)
    {
      switch ((opcode >> (8 * length - 16)) & 0xff)
	{
	default:   space = SPACE_0F;   break;
	case 0x38: space = SPACE_0F38; break;
	case 0x3A: space = SPACE_0F3A; break;
	}

      if (space != SPACE_0F && --length == 1)
	fail ("%s:%d: %s: unrecognized opcode encoding space\n",
	      filename, lineno, name);
      opcode &= (1ULL << (8 * --length)) - 1;
    }

  if (length > 2)
    fail ("%s:%d: %s: residual opcode (0x%0*llx) too large\n",
	  filename, lineno, name, 2 * length, opcode);

  ident = mkident (name);
  fprintf (table, "  { MN_%s, 0x%0*llx%s, %u,",
	   ident, 2 * (int)length, opcode, end, i);
  free (ident);

  process_i386_opcode_modifier (table, opcode_modifier, space, prefix,
				extension_opcode, operand_types, lineno);

  process_i386_cpu_flag (table, cpu_flags, NULL, ",", "    ", lineno, CpuMax);

  fprintf (table, "    { ");

  for (i = 0; i < ARRAY_SIZE (operand_types); i++)
    {
      if (!operand_types[i])
	{
	  if (i == 0)
	    process_i386_operand_type (table, "0", stage_opcodes, "\t  ",
				       lineno);
	  break;
	}

      if (i != 0)
	fprintf (table, ",\n      ");

      process_i386_operand_type (table, operand_types[i], stage_opcodes,
				 "\t  ", lineno);
    }
  fprintf (table, " } },\n");
}

struct opcode_hash_entry
{
  const char *name;
  struct opcode_entry
  {
    struct opcode_entry *next;
    char *opcode;
    int lineno;
  } entry;
};

/* Calculate the hash value of an opcode hash entry P.  */

static hashval_t
opcode_hash_hash (const void *p)
{
  struct opcode_hash_entry *entry = (struct opcode_hash_entry *) p;
  return htab_hash_string (entry->name);
}

/* Compare a string Q against an opcode hash entry P.  */

static int
opcode_hash_eq (const void *p, const void *q)
{
  struct opcode_hash_entry *entry = (struct opcode_hash_entry *) p;
  const char *name = (const char *) q;
  return strcmp (name, entry->name) == 0;
}

static void
parse_template (char *buf, int lineno)
{
  char sep, *end, *name;
  struct template *tmpl;
  struct template_instance *last_inst = NULL;

  buf = remove_leading_whitespaces (buf + 1);
  end = strchr (buf, ':');
  if (end == NULL)
    {
      struct template *prev = NULL;

      end = strchr (buf, '>');
      if (end == NULL)
	fail ("%s: %d: missing ':' or '>'\n", filename, lineno);
      if (*remove_leading_whitespaces (end + 1))
	fail ("%s: %d: malformed template purge\n", filename, lineno);
      *end = '\0';
      remove_trailing_whitespaces (buf);
      /* Don't bother freeing the various structures.  */
      for (tmpl = templates; tmpl != NULL; tmpl = (prev = tmpl)->next)
	if (!strcmp (buf, tmpl->name))
	  break;
      if (tmpl == NULL)
	fail ("%s: %d: no template '%s'\n", filename, lineno, buf);
      if (prev)
	prev->next = tmpl->next;
      else
	templates = tmpl->next;
      return;
    }
  *end++ = '\0';
  remove_trailing_whitespaces (buf);

  if (*buf == '\0')
    fail ("%s: %d: missing template identifier\n", filename, lineno);
  tmpl = xmalloc (sizeof (*tmpl));
  tmpl->name = xstrdup (buf);

  tmpl->params = NULL;
  do {
      struct template_param *param;

      buf = remove_leading_whitespaces (end);
      end = strpbrk (buf, ":,");
      if (end == NULL)
        fail ("%s: %d: missing ':' or ','\n", filename, lineno);

      sep = *end;
      *end++ = '\0';
      remove_trailing_whitespaces (buf);

      param = xmalloc (sizeof (*param));
      param->name = xstrdup (buf);
      param->next = tmpl->params;
      tmpl->params = param;
  } while (sep == ':');

  tmpl->instances = NULL;
  do {
      struct template_instance *inst;
      char *cur, *next;
      const struct template_param *param;

      buf = remove_leading_whitespaces (end);
      end = strpbrk (buf, ",>");
      if (end == NULL)
        fail ("%s: %d: missing ',' or '>'\n", filename, lineno);

      sep = *end;
      *end++ = '\0';

      inst = xmalloc (sizeof (*inst));
      inst->next = NULL;
      inst->args = NULL;

      cur = next_field (buf, ':', &next, end);
      inst->name = *cur != '$' ? xstrdup (cur) : "";

      for (param = tmpl->params; param; param = param->next)
	{
	  struct template_arg *arg = xmalloc (sizeof (*arg));

	  cur = next_field (next, ':', &next, end);
	  if (next > end)
	    fail ("%s: %d: missing argument for '%s'\n", filename, lineno, param->name);
	  arg->val = xstrdup (cur);
	  arg->next = inst->args;
	  inst->args = arg;
	}

      if (tmpl->instances)
	last_inst->next = inst;
      else
	tmpl->instances = inst;
      last_inst = inst;
  } while (sep == ',');

  buf = remove_leading_whitespaces (end);
  if (*buf)
    fprintf(stderr, "%s: %d: excess characters '%s'\n",
	    filename, lineno, buf);

  tmpl->next = templates;
  templates = tmpl;
}

static unsigned int
expand_templates (char *name, const char *str, htab_t opcode_hash_table,
		  struct opcode_hash_entry ***opcode_array_p, int lineno)
{
  static unsigned int idx, opcode_array_size;
  struct opcode_hash_entry **opcode_array = *opcode_array_p;
  struct opcode_hash_entry **hash_slot;
  struct opcode_entry *entry;
  char *ptr1 = strchr(name, '<'), *ptr2;

  if (ptr1 == NULL)
    {
      /* Get the slot in hash table.  */
      hash_slot = (struct opcode_hash_entry **)
	htab_find_slot_with_hash (opcode_hash_table, name,
				  htab_hash_string (name),
				  INSERT);

      if (*hash_slot == NULL)
	{
	  /* It is the new one.  Put it on opcode array.  */
	  if (idx >= opcode_array_size)
	    {
	      /* Grow the opcode array when needed.  */
	      opcode_array_size += 1024;
	      opcode_array = (struct opcode_hash_entry **)
		xrealloc (opcode_array,
			  sizeof (*opcode_array) * opcode_array_size);
		*opcode_array_p = opcode_array;
	    }

	  opcode_array[idx] = (struct opcode_hash_entry *)
	    xmalloc (sizeof (struct opcode_hash_entry));
	  opcode_array[idx]->name = xstrdup (name);
	  *hash_slot = opcode_array[idx];
	  entry = &opcode_array[idx]->entry;
	  idx++;
	}
      else
	{
	  /* Append it to the existing one.  */
	  struct opcode_entry **entryp = &(*hash_slot)->entry.next;

	  while (*entryp != NULL)
	    entryp = &(*entryp)->next;
	  entry = (struct opcode_entry *)xmalloc (sizeof (struct opcode_entry));
	  *entryp = entry;
	}

      entry->next = NULL;
      entry->opcode = xstrdup (str);
      entry->lineno = lineno;
    }
  else if ((ptr2 = strchr(ptr1 + 1, '>')) == NULL)
    fail ("%s: %d: missing '>'\n", filename, lineno);
  else
    {
      const struct template *tmpl;
      const struct template_instance *inst;

      *ptr1 = '\0';
      ptr1 = remove_leading_whitespaces (ptr1 + 1);
      remove_trailing_whitespaces (ptr1);

      *ptr2++ = '\0';

      for ( tmpl = templates; tmpl; tmpl = tmpl->next )
	if (!strcmp(ptr1, tmpl->name))
	  break;
      if (!tmpl)
	fail ("reference to unknown template '%s'\n", ptr1);

      for (inst = tmpl->instances; inst; inst = inst->next)
	{
	  char *name2 = xmalloc(strlen(name) + strlen(inst->name) + strlen(ptr2) + 1);
	  char *str2 = xmalloc(2 * strlen(str));
	  const char *src;

	  strcpy (name2, name);
	  strcat (name2, inst->name);
	  strcat (name2, ptr2);

	  for (ptr1 = str2, src = str; *src; )
	    {
	      const char *ident = tmpl->name, *end;
	      const struct template_param *param;
	      const struct template_arg *arg;

	      if ((*ptr1 = *src++) != '<')
		{
		  ++ptr1;
		  continue;
		}
	      while (ISSPACE(*src))
		++src;
	      while (*ident && *src == *ident)
		++src, ++ident;
	      while (ISSPACE(*src))
		++src;
	      if (*src != ':' || *ident != '\0')
		{
		  memcpy (++ptr1, tmpl->name, ident - tmpl->name);
		  ptr1 += ident - tmpl->name;
		  continue;
		}
	      while (ISSPACE(*++src))
		;

	      end = src;
	      while (*end != '\0' && !ISSPACE(*end) && *end != '>')
		++end;

	      for (param = tmpl->params, arg = inst->args; param;
		   param = param->next, arg = arg->next)
		{
		  if (end - src == strlen (param->name)
		      && !memcmp (src, param->name, end - src))
		    {
		      src = end;
		      break;
		    }
		}

	      if (param == NULL)
		fail ("template '%s' has no parameter '%.*s'\n",
		      tmpl->name, (int)(end - src), src);

	      while (ISSPACE(*src))
		++src;
	      if (*src != '>')
		fail ("%s: %d: missing '>'\n", filename, lineno);

	      memcpy(ptr1, arg->val, strlen(arg->val));
	      ptr1 += strlen(arg->val);
	      ++src;
	    }

	  *ptr1 = '\0';

	  expand_templates (name2, str2, opcode_hash_table, opcode_array_p,
			    lineno);

	  free (str2);
	  free (name2);
	}
    }

  return idx;
}

static int mnemonic_cmp(const void *p1, const void *p2)
{
  const struct opcode_hash_entry *const *e1 = p1, *const *e2 = p2;
  const char *s1 = (*e1)->name, *s2 = (*e2)->name;
  unsigned int i;
  size_t l1 = strlen (s1), l2 = strlen (s2);

  for (i = 1; i <= l1 && i <= l2; ++i)
    {
      if (s1[l1 - i] != s2[l2 - i])
	return (unsigned char)s1[l1 - i] - (unsigned char)s2[l2 - i];
    }

  return (int)(l1 - l2);
}

static void
process_i386_opcodes (FILE *table)
{
  FILE *fp;
  char buf[2048];
  unsigned int i, j, nr, offs;
  size_t l;
  char *str, *p, *last;
  htab_t opcode_hash_table;
  struct opcode_hash_entry **opcode_array = NULL;
  int lineno = 0, marker = 0;

  filename = "i386-opc.tbl";
  fp = stdin;

  i = 0;
  opcode_hash_table = htab_create_alloc (16, opcode_hash_hash,
					 opcode_hash_eq, NULL,
					 xcalloc, free);

  fprintf (table, "\n#include \"i386-mnem.h\"\n");
  fprintf (table, "\n/* i386 opcode table.  */\n\n");
  fprintf (table, "static const insn_template i386_optab[] =\n{\n");

  /* Put everything on opcode array.  */
  while (!feof (fp))
    {
      char *name;

      if (fgets (buf, sizeof (buf), fp) == NULL)
	break;

      p = remove_leading_whitespaces (buf);

      for ( ; ; )
	{
	  lineno++;

	  /* Skip comments.  */
	  str = strstr (p, "//");
	  if (str != NULL)
	    {
	      str[0] = '\0';
	      remove_trailing_whitespaces (p);
	      break;
	    }

	  /* Look for line continuation character.  */
	  remove_trailing_whitespaces (p);
	  j = strlen (buf);
	  if (!j || buf[j - 1] != '+')
	    break;
	  if (j >= sizeof (buf) - 1)
	    fail ("%s: %d: (continued) line too long\n", filename, lineno);

	  if (fgets (buf + j - 1, sizeof (buf) - j + 1, fp) == NULL)
	    {
	      fprintf (stderr, "%s: Line continuation on last line?\n",
		       filename);
	      break;
	    }
	}

      switch (p[0])
	{
	case '#':
	  if (!strcmp("### MARKER ###", buf))
	    marker = 1;
	  else
	    {
	      /* Since we ignore all included files (we only care about their
		 #define-s here), we don't need to monitor filenames.  The final
		 line number directive is going to refer to the main source file
		 again.  */
	      char *end;
	      unsigned long ln;

	      p = remove_leading_whitespaces (p + 1);
	      if (!strncmp(p, "line", 4))
		p += 4;
	      ln = strtoul (p, &end, 10);
	      if (ln > 1 && ln < INT_MAX
		  && *remove_leading_whitespaces (end) == '"')
		lineno = ln - 1;
	    }
	  /* Ignore comments.  */
	case '\0':
	  continue;
	  break;
	case '<':
	  parse_template (p, lineno);
	  continue;
	default:
	  if (!marker)
	    continue;
	  break;
	}

      last = p + strlen (p);

      /* Find name.  */
      name = next_field (p, ',', &str, last);

      i = expand_templates (name, str, opcode_hash_table, &opcode_array,
			    lineno);
    }

  /* Process opcode array.  */
  for (j = 0; j < i; j++)
    {
      const char *name = opcode_array[j]->name;
      struct opcode_entry *next;

      for (next = &opcode_array[j]->entry; next; next = next->next)
	{
	  str = next->opcode;
	  lineno = next->lineno;
	  last = str + strlen (str);
	  output_i386_opcode (table, name, str, last, lineno);
	}
    }

  fclose (fp);

  fprintf (table, "};\n");

  /* Generate opcode sets array.  */
  fprintf (table, "\n/* i386 opcode sets table.  */\n\n");
  fprintf (table, "static const insn_template *const i386_op_sets[] =\n{\n");
  fprintf (table, "  i386_optab,\n");

  for (nr = j = 0; j < i; j++)
    {
      struct opcode_entry *next = &opcode_array[j]->entry;

      do
	{
	  ++nr;
	  next = next->next;
	}
      while (next);
      fprintf (table, "  i386_optab + %u,\n", nr);
    }

  fprintf (table, "};\n");

  /* Emit mnemonics and associated #define-s.  */
  qsort (opcode_array, i, sizeof (*opcode_array), mnemonic_cmp);

  fp = fopen ("i386-mnem.h", "w");
  if (fp == NULL)
    fail ("can't create i386-mnem.h, errno = %s\n",
	  xstrerror (errno));

  process_copyright (fp);

  fprintf (table, "\n/* i386 mnemonics table.  */\n\n");
  fprintf (table, "const char i386_mnemonics[] =\n");
  fprintf (fp, "\nextern const char i386_mnemonics[];\n\n");

  str = NULL;
  for (l = strlen (opcode_array[offs = j = 0]->name); j < i; j++)
    {
      const char *name = opcode_array[j]->name;
      const char *next = NULL;
      size_t l1 = j + 1 < i ? strlen(next = opcode_array[j + 1]->name) : 0;

      if (str == NULL)
	str = mkident (name);
      if (l < l1 && !strcmp(name, next + l1 - l))
	{
	  fprintf (fp, "#define MN_%s ", str);
	  free (str);
	  str = mkident (next);
	  fprintf (fp, "(MN_%s + %u)\n", str, l1 - l);
	}
      else
	{
	  fprintf (table, "  \"\\0\"\"%s\"\n", name);
	  fprintf (fp, "#define MN_%s %#x\n", str, offs + 1);
	  offs += strlen (name) + 1;
	  free (str);
	  str = NULL;
	}
      l = l1;
    }

  fprintf (table, "  \"\\0\"\".insn\"\n");
  fprintf (fp, "#define MN__insn %#x\n", offs + 1);

  fprintf (table, ";\n");

  fclose (fp);
}

static void
process_i386_registers (FILE *table)
{
  FILE *fp;
  char buf[2048];
  char *str, *p, *last;
  char *reg_name, *reg_type, *reg_flags, *reg_num;
  char *dw2_32_num, *dw2_64_num;
  int lineno = 0;

  filename = "i386-reg.tbl";
  fp = fopen (filename, "r");
  if (fp == NULL)
    fail ("can't find i386-reg.tbl for reading, errno = %s\n",
	  xstrerror (errno));

  fprintf (table, "\n/* i386 register table.  */\n\n");
  fprintf (table, "static const reg_entry i386_regtab[] =\n{\n");

  while (!feof (fp))
    {
      if (fgets (buf, sizeof (buf), fp) == NULL)
	break;

      lineno++;

      p = remove_leading_whitespaces (buf);

      /* Skip comments.  */
      str = strstr (p, "//");
      if (str != NULL)
	str[0] = '\0';

      /* Remove trailing white spaces.  */
      remove_trailing_whitespaces (p);

      switch (p[0])
	{
	case '#':
	  fprintf (table, "%s\n", p);
	case '\0':
	  continue;
	  break;
	default:
	  break;
	}

      last = p + strlen (p);

      /* Find reg_name.  */
      reg_name = next_field (p, ',', &str, last);

      /* Find reg_type.  */
      reg_type = next_field (str, ',', &str, last);

      /* Find reg_flags.  */
      reg_flags = next_field (str, ',', &str, last);

      /* Find reg_num.  */
      reg_num = next_field (str, ',', &str, last);

      fprintf (table, "  { \"%s\",\n    ", reg_name);

      process_i386_operand_type (table, reg_type, stage_registers, "\t",
				 lineno);

      /* Find 32-bit Dwarf2 register number.  */
      dw2_32_num = next_field (str, ',', &str, last);

      /* Find 64-bit Dwarf2 register number.  */
      dw2_64_num = next_field (str, ',', &str, last);

      fprintf (table, ",\n    %s, %s, { %s, %s } },\n",
	       reg_flags, reg_num, dw2_32_num, dw2_64_num);
    }

  fclose (fp);

  fprintf (table, "};\n");

  fprintf (table, "\nstatic const unsigned int i386_regtab_size = ARRAY_SIZE (i386_regtab);\n");
}

static void
process_i386_initializers (void)
{
  unsigned int i;
  FILE *fp = fopen ("i386-init.h", "w");

  if (fp == NULL)
    fail ("can't create i386-init.h, errno = %s\n",
	  xstrerror (errno));

  process_copyright (fp);

  for (i = 0; i < Cpu64; i++)
    process_i386_cpu_flag (fp, "0", cpu_flags[i].name, "", "  ", -1, i);

  for (i = 0; i < ARRAY_SIZE (isa_dependencies); i++)
    {
      char *deps = xstrdup (isa_dependencies[i].deps);

      process_i386_cpu_flag (fp, deps, isa_dependencies[i].name,
			     "", "  ", -1, CpuMax);
      free (deps);
    }

  /* Early x87 is somewhat special: Both 287 and 387 not only add new insns
     but also remove some.  Hence 8087 isn't a prereq to 287, and 287 isn't
     one to 387.  We want the reverse to be true though: Disabling 8087 also
     is to disable 287+ and later; disabling 287 also means disabling 387+.  */
  memcpy (isa_reverse_deps[Cpu287], isa_reverse_deps[Cpu387],
          sizeof (isa_reverse_deps[0]));
  isa_reverse_deps[Cpu287][Cpu387] = 1;
  memcpy (isa_reverse_deps[Cpu8087], isa_reverse_deps[Cpu287],
          sizeof (isa_reverse_deps[0]));
  isa_reverse_deps[Cpu8087][Cpu287] = 1;

  /* While we treat POPCNT as a prereq to SSE4.2, its disabling should not
     lead to disabling of anything else.  */
  memset (isa_reverse_deps[CpuPOPCNT], 0, sizeof (isa_reverse_deps[0]));

  for (i = Cpu686 + 1; i < ARRAY_SIZE (isa_reverse_deps); i++)
    {
      size_t len;
      char *upper;

      if (memchr(isa_reverse_deps[i], 1,
	  ARRAY_SIZE (isa_reverse_deps[0])) == NULL)
	continue;

      isa_reverse_deps[i][i] = 1;
      process_i386_cpu_flag (fp, NULL, cpu_flags[i].name, "", "  ", -1, i);
    }

  fprintf (fp, "\n");

  fclose (fp);
}

/* Program options.  */
#define OPTION_SRCDIR	200

struct option long_options[] =
{
  {"srcdir",  required_argument, NULL, OPTION_SRCDIR},
  {"debug",   no_argument,       NULL, 'd'},
  {"version", no_argument,       NULL, 'V'},
  {"help",    no_argument,       NULL, 'h'},
  {0,         no_argument,       NULL, 0}
};

static void
print_version (void)
{
  printf ("%s: version 1.0\n", program_name);
  xexit (0);
}

static void
usage (FILE * stream, int status)
{
  fprintf (stream, "Usage: %s [-V | --version] [-d | --debug] [--srcdir=dirname] [--help]\n",
	   program_name);
  xexit (status);
}

int
main (int argc, char **argv)
{
  extern int chdir (char *);
  char *srcdir = NULL;
  int c;
  unsigned int i, cpumax;
  FILE *table;

  program_name = *argv;
  xmalloc_set_program_name (program_name);

  while ((c = getopt_long (argc, argv, "vVdh", long_options, 0)) != EOF)
    switch (c)
      {
      case OPTION_SRCDIR:
	srcdir = optarg;
	break;
      case 'V':
      case 'v':
	print_version ();
	break;
      case 'd':
	debug = 1;
	break;
      case 'h':
      case '?':
	usage (stderr, 0);
      default:
      case 0:
	break;
      }

  if (optind != argc)
    usage (stdout, 1);

  if (srcdir != NULL)
    if (chdir (srcdir) != 0)
      fail ("unable to change directory to \"%s\", errno = %s\n",
	    srcdir, xstrerror (errno));

  /* cpu_flags isn't sorted by position.  */
  cpumax = 0;
  for (i = 0; i < ARRAY_SIZE (cpu_flags); i++)
    if (cpu_flags[i].position > cpumax)
      cpumax = cpu_flags[i].position;

  /* Check the unused bitfield in i386_cpu_flags.  */
#ifdef CpuUnused
  static_assert (ARRAY_SIZE (cpu_flags) == CpuMax + 2);

  if ((cpumax - 1) != CpuMax)
    fail ("CpuMax != %d!\n", cpumax);
#else
  static_assert (ARRAY_SIZE (cpu_flags) == CpuMax + 1);

  if (cpumax != CpuMax)
    fail ("CpuMax != %d!\n", cpumax);

  c = CpuNumOfBits - CpuMax - 1;
  if (c)
    fail ("%d unused bits in i386_cpu_flags.\n", c);
#endif

  static_assert (ARRAY_SIZE (opcode_modifiers) == Opcode_Modifier_Num);

  /* Check the unused bitfield in i386_operand_type.  */
#ifdef OTUnused
  static_assert (ARRAY_SIZE (operand_types) + CLASS_WIDTH + INSTANCE_WIDTH
		 == OTNum + 1);
#else
  static_assert (ARRAY_SIZE (operand_types) + CLASS_WIDTH + INSTANCE_WIDTH
		 == OTNum);

  c = OTNumOfBits - OTNum;
  if (c)
    fail ("%d unused bits in i386_operand_type.\n", c);
#endif

  qsort (cpu_flags, ARRAY_SIZE (cpu_flags), sizeof (cpu_flags [0]),
	 compare);

  qsort (opcode_modifiers, ARRAY_SIZE (opcode_modifiers),
	 sizeof (opcode_modifiers [0]), compare);

  qsort (operand_types, ARRAY_SIZE (operand_types),
	 sizeof (operand_types [0]), compare);

  table = fopen ("i386-tbl.h", "w");
  if (table == NULL)
    fail ("can't create i386-tbl.h, errno = %s\n",
	  xstrerror (errno));

  process_copyright (table);

  process_i386_opcodes (table);
  process_i386_registers (table);
  process_i386_initializers ();

  fclose (table);

  exit (0);
}
