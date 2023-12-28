/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#if defined(__i386__) || defined(__x86_64)
#include <cpuid.h>  /* GCC-provided */
#elif defined(__aarch64__)
#define ATTRIBUTE_UNUSED __attribute__((unused))

static inline uint_t __attribute_const__
__get_cpuid (unsigned int op ATTRIBUTE_UNUSED, unsigned int *eax,
	     unsigned int *ebx ATTRIBUTE_UNUSED,
	     unsigned int *ecx ATTRIBUTE_UNUSED, unsigned int *edx ATTRIBUTE_UNUSED)
{
  // CPUID bit assignments:
  // [31:24] IMPLEMENTER (0x50 - ARM_CPU_IMP_APM)
  // [23:20] VARIANT indicates processor revision (0x2 = Revision 2)
  // [19:16] Constant (Reads as 0xF)
  // [15:04] PARTNO indicates part number (0xC23 = Cortex-M3)
  // [03:00] REVISION indicates patch release (0x0 = Patch 0)
  //    unsigned long v = 0;
  //    __asm volatile ("MRS %[result], MPIDR_EL1" : [result] "=r" (v));
  //    Tprintf(DBG_LT0, "cpuid.c:%d read_cpuid_id() MPIDR_EL1=0x%016lx\n", __LINE__, v);
  uint_t res = 0;
  __asm volatile ("MRS %[result], MIDR_EL1" : [result] "=r" (*eax));
  Tprintf (DBG_LT0, "cpuid.c:%d read_cpuid_id() MIDR_EL1=0x%016x\n", __LINE__, *eax);
  return res;
}
#endif

/*
 * Various routines to handle identification
 * and classification of x86 processors.
 */

#define IS_GLOBAL /* externally visible */
#define	X86_VENDOR_Intel	0
#define	X86_VENDORSTR_Intel	"GenuineIntel"
#define	X86_VENDOR_IntelClone	1
#define	X86_VENDOR_AMD		2
#define	X86_VENDORSTR_AMD	"AuthenticAMD"

#define BITX(u, h, l)       (((u) >> (l)) & ((1LU << ((h) - (l) + 1LU)) - 1LU))
#define CPI_FAMILY_XTD(reg) BITX(reg, 27, 20)
#define CPI_MODEL_XTD(reg)  BITX(reg, 19, 16)
#define CPI_TYPE(reg)       BITX(reg, 13, 12)
#define CPI_FAMILY(reg)     BITX(reg, 11, 8)
#define CPI_STEP(reg)       BITX(reg, 3, 0)
#define CPI_MODEL(reg)      BITX(reg, 7, 4)
#define IS_EXTENDED_MODEL_INTEL(model)  ((model) == 0x6 || (model) >= 0xf)


typedef struct
{
  unsigned int eax;
  unsigned int ebx;
  unsigned int ecx;
  unsigned int edx;
} cpuid_regs_t;

typedef struct
{
  unsigned int cpi_model;
  unsigned int cpi_family;
  unsigned int cpi_vendor;        /* enum of cpi_vendorstr */
  unsigned int cpi_maxeax;        /* fn 0: %eax */
  char cpi_vendorstr[13];         /* fn 0: %ebx:%ecx:%edx */
} cpuid_info_t;


#if defined(__i386__) || defined(__x86_64)
static uint_t
cpuid_vendorstr_to_vendorcode (char *vendorstr)
{
  if (strcmp (vendorstr, X86_VENDORSTR_Intel) == 0)
    return X86_VENDOR_Intel;
  else if (strcmp (vendorstr, X86_VENDORSTR_AMD) == 0)
    return X86_VENDOR_AMD;
  else
    return X86_VENDOR_IntelClone;
}

static int
my_cpuid (unsigned int op, cpuid_regs_t *regs)
{
  regs->eax = regs->ebx = regs->ecx = regs->edx = 0;
  int ret = __get_cpuid (op, &regs->eax, &regs->ebx, &regs->ecx, &regs->edx);
  TprintfT (DBG_LT1, "my_cpuid: __get_cpuid(0x%x, 0x%x, 0x%x, 0x%x, 0x%x) returns %d\n",
	    op, regs->eax, regs->ebx, regs->ecx, regs->edx, ret);
  return ret;
}
#endif

static cpuid_info_t *
get_cpuid_info ()
{
  static int cpuid_inited = 0;
  static cpuid_info_t cpuid_info;
  cpuid_info_t *cpi = &cpuid_info;
  if (cpuid_inited)
    return cpi;
  cpuid_inited = 1;

#if defined(__aarch64__)
  // CPUID bit assignments:
  // [31:24] IMPLEMENTER (0x50 - ARM_CPU_IMP_APM)
  // [23:20] VARIANT indicates processor revision (0x2 = Revision 2)
  // [19:16] Constant (Reads as 0xF)
  // [15:04] PARTNO indicates part number (0xC23 = Cortex-M3)
  // [03:00] REVISION indicates patch release (0x0 = Patch 0)
  uint_t reg = 0;
  __asm volatile ("MRS %[result], MIDR_EL1" : [result] "=r" (reg));
  cpi->cpi_vendor = reg >> 24;
  cpi->cpi_model = (reg >> 4) & 0xfff;
  switch (cpi->cpi_vendor)
    {
    case ARM_CPU_IMP_APM:
    case ARM_CPU_IMP_ARM:
    case ARM_CPU_IMP_CAVIUM:
    case ARM_CPU_IMP_BRCM:
    case ARM_CPU_IMP_QCOM:
      strncpy (cpi->cpi_vendorstr, AARCH64_VENDORSTR_ARM, sizeof (cpi->cpi_vendorstr));
      break;
    default:
      strncpy (cpi->cpi_vendorstr, "UNKNOWN ARM", sizeof (cpi->cpi_vendorstr));
      break;
    }
  Tprintf (DBG_LT0, "cpuid.c:%d read_cpuid_id() MIDR_EL1==0x%016x cpi_vendor=%d cpi_model=%d\n",
	   __LINE__, (unsigned int) reg, cpi->cpi_vendor, cpi->cpi_model);

#elif defined(__i386__) || defined(__x86_64)
  cpuid_regs_t regs;
  my_cpuid (0, &regs);
  cpi->cpi_maxeax = regs.eax;
  ((uint32_t *) cpi->cpi_vendorstr)[0] = regs.ebx;
  ((uint32_t *) cpi->cpi_vendorstr)[1] = regs.edx;
  ((uint32_t *) cpi->cpi_vendorstr)[2] = regs.ecx;
  cpi->cpi_vendorstr[12] = 0;
  cpi->cpi_vendor = cpuid_vendorstr_to_vendorcode (cpi->cpi_vendorstr);

  my_cpuid (1, &regs);
  cpi->cpi_model = CPI_MODEL (regs.eax);
  cpi->cpi_family = CPI_FAMILY (regs.eax);
  if (cpi->cpi_family == 0xf)
    cpi->cpi_family += CPI_FAMILY_XTD (regs.eax);

  /*
   * Beware: AMD uses "extended model" iff base *FAMILY* == 0xf.
   * Intel, and presumably everyone else, uses model == 0xf, as
   * one would expect (max value means possible overflow).  Sigh.
   */
  switch (cpi->cpi_vendor)
    {
    case X86_VENDOR_Intel:
      if (IS_EXTENDED_MODEL_INTEL (cpi->cpi_family))
	cpi->cpi_model += CPI_MODEL_XTD (regs.eax) << 4;
      break;
    case X86_VENDOR_AMD:
      if (CPI_FAMILY (cpi->cpi_family) == 0xf)
	cpi->cpi_model += CPI_MODEL_XTD (regs.eax) << 4;
      break;
    default:
      if (cpi->cpi_model == 0xf)
	cpi->cpi_model += CPI_MODEL_XTD (regs.eax) << 4;
      break;
    }
#endif
  return cpi;
}

static inline uint_t
cpuid_getvendor ()
{
  return get_cpuid_info ()->cpi_vendor;
}

static inline uint_t
cpuid_getfamily ()
{
  return get_cpuid_info ()->cpi_family;
}

static inline uint_t
cpuid_getmodel ()
{
  return get_cpuid_info ()->cpi_model;
}
