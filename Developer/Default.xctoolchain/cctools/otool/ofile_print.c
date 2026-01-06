/*
 * Copyright © 2009 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1.  Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * This file intention is to beable to print the structures in an object file
 * and handle problems with reguard to alignment and bytesex.  The goal is to
 * print as much as possible even when things are truncated or trashed.  Both
 * a verbose (symbolic) and non-verbose modes are supported to aid in seeing
 * the values even if they are not correct.  As much as possible strict checks
 * on values of fields for correctness should be done (such as proper alignment)
 * and notations on errors should be printed.
 */

#define __cr cr
#define __ctr ctr
#define __dar dar
#define __dsisr dsisr
#define __exception exception
#define __fpregs fpregs
#define __fpscr fpscr
#define __fpscr_pad fpscr_pad
#define __lr lr
#define __mq mq
#define __pad0 pad0
#define __pad1 pad1
#define __r0 r0
#define __r1 r1
#define __r10 r10
#define __r11 r11
#define __r12 r12
#define __r13 r13
#define __r14 r14
#define __r15 r15
#define __r16 r16
#define __r17 r17
#define __r18 r18
#define __r19 r19
#define __r2 r2
#define __r20 r20
#define __r21 r21
#define __r22 r22
#define __r23 r23
#define __r24 r24
#define __r25 r25
#define __r26 r26
#define __r27 r27
#define __r28 r28
#define __r29 r29
#define __r3 r3
#define __r30 r30
#define __r31 r31
#define __r4 r4
#define __r5 r5
#define __r6 r6
#define __r7 r7
#define __r8 r8
#define __r9 r9
#define __srr0 srr0
#define __srr1 srr1
#define __vrsave vrsave
#define __xer xer

#define __darwin_i386_exception_state i386_exception_state
#define __darwin_i386_float_state i386_float_state
#define __darwin_i386_thread_state i386_thread_state
#define __busy busy
#define __c0 c0
#define __c1 c1
#define __c2 c2
#define __c3 c3
#define __cs cs
#define __darwin_fp_control fp_control
#define __darwin_fp_status fp_status
#define __darwin_mmst_reg mmst_reg
#define __darwin_xmm_reg xmm_reg
#define __denorm denorm
#define __ds ds
#define __eax eax
#define __ebp ebp
#define __ebx ebx
#define __ecx ecx
#define __edi edi
#define __edx edx
#define __eflags eflags
#define __eip eip
#define __err err
#define __errsumm errsumm
#define __es es
#define __esi esi
#define __esp esp
#define __faultvaddr faultvaddr
#define __fpu_cs fpu_cs
#define __fpu_dp fpu_dp
#define __fpu_ds fpu_ds
#define __fpu_fcw fpu_fcw
#define __fpu_fop fpu_fop
#define __fpu_fsw fpu_fsw
#define __fpu_ftw fpu_ftw
#define __fpu_ip fpu_ip
#define __fpu_mxcsr fpu_mxcsr
#define __fpu_mxcsrmask fpu_mxcsrmask
#define __fpu_reserved fpu_reserved
#define __fpu_reserved1 fpu_reserved1
#define __fpu_rsrv1 fpu_rsrv1
#define __fpu_rsrv2 fpu_rsrv2
#define __fpu_rsrv3 fpu_rsrv3
#define __fpu_rsrv4 fpu_rsrv4
#define __fpu_stmm0 fpu_stmm0
#define __fpu_stmm1 fpu_stmm1
#define __fpu_stmm2 fpu_stmm2
#define __fpu_stmm3 fpu_stmm3
#define __fpu_stmm4 fpu_stmm4
#define __fpu_stmm5 fpu_stmm5
#define __fpu_stmm6 fpu_stmm6
#define __fpu_stmm7 fpu_stmm7
#define __fpu_xmm0 fpu_xmm0
#define __fpu_xmm1 fpu_xmm1
#define __fpu_xmm2 fpu_xmm2
#define __fpu_xmm3 fpu_xmm3
#define __fpu_xmm4 fpu_xmm4
#define __fpu_xmm5 fpu_xmm5
#define __fpu_xmm6 fpu_xmm6
#define __fpu_xmm7 fpu_xmm7
#define __fpu_xmm8 fpu_xmm8
#define __fpu_xmm9 fpu_xmm9
#define __fpu_xmm10 fpu_xmm10
#define __fpu_xmm11 fpu_xmm11
#define __fpu_xmm12 fpu_xmm12
#define __fpu_xmm13 fpu_xmm13
#define __fpu_xmm14 fpu_xmm14
#define __fpu_xmm15 fpu_xmm15
#define __fs fs
#define __gs gs
#define __invalid invalid
#define __mmst_reg mmst_reg
#define __mmst_rsrv mmst_rsrv
#define __ovrfl ovrfl
#define __pc pc
#define __precis precis
#define __rc rc
#define __ss ss
#define __stkflt stkflt
#define __tos tos
#define __trapno trapno
#define __undfl undfl
#define __xmm_reg xmm_reg
#define __zdiv zdiv

#define __rax rax
#define __rbx rbx
#define __rcx rcx
#define __rdx rdx
#define __rdi rdi
#define __rsi rsi
#define __rbp rbp
#define __rsp rsp
#define __r8 r8
#define __r9 r9
#define __r10 r10
#define __r11 r11
#define __r12 r12
#define __r13 r13
#define __r14 r14
#define __r15 r15
#define __rip rip
#define __rflags rflags

#define __dr0 dr0
#define __dr1 dr1
#define __dr2 dr2
#define __dr3 dr3
#define __dr4 dr4
#define __dr5 dr5
#define __dr6 dr6
#define __dr7 dr7

/*
 * Otherwise string.h may hide strnlen().
 * https://github.com/tpoechtrager/cctools-port/pull/8
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <ar.h>
#include <libc.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach-o/reloc.h>
#include <mach-o/i860/reloc.h>
#include <mach-o/m88k/reloc.h>
#include <mach-o/ppc/reloc.h>
#include <mach-o/hppa/reloc.h>
#include <mach-o/sparc/reloc.h>
#include <mach-o/arm/reloc.h>
#include <mach-o/arm64/reloc.h>
#include "stuff/symbol.h"
#include "stuff/ofile.h"
#include "stuff/allocate.h"
#include "stuff/errors.h"
#include "stuff/guess_short_name.h"
#include "dyld_bind_info.h"
#include "ofile_print.h"

/* <mach/loader.h> */
/* The maximum section alignment allowed to be specified, as a power of two */
#define MAXSECTALIGN		15 /* 2**15 or 0x8000 */

static void print_arch(
    cpu_type_t cputype,
    cpu_subtype_t cpusubtype);
static void print_cputype(
    cpu_type_t cputype,
    cpu_subtype_t cpusubtype);
static void print_version_xyz(
    const char* label,
    uint32_t version);

#if i386_THREAD_STATE == 1
#ifdef i386_EXCEPTION_STATE_COUNT
static void print_mmst_reg(
    struct mmst_reg *r);
static void print_xmm_reg(
    struct xmm_reg *r);
#endif /* defined(i386_EXCEPTION_STATE_COUNT) */
#endif /* i386_THREAD_STATE == 1 */

static void print_unknown_state(
    char *begin,
    char *end,
    unsigned int count,
    enum bool swapped);

struct reloc_section_info {
    char segname[16];
    char sectname[16];
    uint32_t nreloc;
    uint32_t reloff;
};

static void print_relocs(
    unsigned reloff,
    unsigned nreloc,
    struct reloc_section_info *sect_rel,
    uint32_t nsects,
    enum bool swapped,
    cpu_type_t cputype,
    char *object_addr,
    uint64_t object_size,
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    char *strings,
    uint32_t strings_size,
    enum bool verbose);
static void print_r_type(
    cpu_type_t cputype,
    uint32_t r_type,
    enum bool predicted);
static void print_cstring_char(
    char c);
static void print_literal4(
    uint32_t l,
    float f);
static void print_literal8(
    uint32_t l0,
    uint32_t l1,
    double d,
    enum byte_sex literal_byte_sex);
static void print_literal16(
    uint32_t l0,
    uint32_t l1,
    uint32_t l2,
    uint32_t l3);
static int rel_bsearch(
    uint32_t *address,
    struct relocation_info *rel);

/*
 * Print the fat header and the fat_archs or fat_archs64.  The caller is
 * responsible for making sure the structures are properly aligned and that the
 * fat_archs or fat_archs64 is of the size fat_header->nfat_arch *
 * sizeof(struct fat_arch) or sizeof(struct fat_arch_64).
 */
void
print_fat_headers(
struct fat_header *fat_header,
struct fat_arch *fat_archs,
struct fat_arch_64 *fat_archs64,
uint64_t filesize,
enum bool verbose)
{
    uint32_t i, j, sizeof_fat_arch;
    uint64_t big_size;
    cpu_type_t cputype;
    cpu_subtype_t cpusubtype;
    uint64_t offset;
    uint64_t size;
    uint32_t align;

	if(verbose){
	    if(fat_header->magic == FAT_MAGIC_64)
		printf("fat_magic FAT_MAGIC_64\n");
	    else if(fat_header->magic == FAT_MAGIC)
		printf("fat_magic FAT_MAGIC\n");
	    else
		printf("fat_magic 0x%x\n", (unsigned int)(fat_header->magic));
	}
	else
	    printf("fat_magic 0x%x\n", (unsigned int)(fat_header->magic));
	if(fat_header->magic == FAT_MAGIC_64)
	    sizeof_fat_arch = sizeof(struct fat_arch_64);
	else
	    sizeof_fat_arch = sizeof(struct fat_arch);


	printf("nfat_arch %u", fat_header->nfat_arch);
	big_size = fat_header->nfat_arch;
	big_size *= sizeof_fat_arch;
	big_size += sizeof(struct fat_header);
	if(fat_header->nfat_arch == 0)
	    printf(" (malformed, contains zero architecture types)\n");
	else if(big_size > filesize)
	    printf(" (malformed, architectures past end of file)\n");
	else
	    printf("\n");

	for(i = 0; i < fat_header->nfat_arch; i++){
	    big_size = i;
	    big_size *= sizeof_fat_arch;
	    big_size += sizeof(struct fat_header);
	    if(big_size > filesize)
		break;
	    printf("architecture ");
	    for(j = 0; i != 0 && j <= i - 1; j++){
		if(fat_header->magic == FAT_MAGIC_64){
		    if(fat_archs64[i].cputype != 0 &&
		       fat_archs64[i].cpusubtype != 0 &&
		       fat_archs64[i].cputype == fat_archs64[j].cputype &&
		       (fat_archs64[i].cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       (fat_archs64[j].cpusubtype & ~CPU_SUBTYPE_MASK)){
			printf("(illegal duplicate architecture) ");
			break;
		    }
		}
		else{
		    if(fat_archs[i].cputype != 0 &&
		       fat_archs[i].cpusubtype != 0 &&
		       fat_archs[i].cputype == fat_archs[j].cputype &&
		       (fat_archs[i].cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       (fat_archs[j].cpusubtype & ~CPU_SUBTYPE_MASK)){
			printf("(illegal duplicate architecture) ");
			break;
		    }
		}
	    }
	    if(fat_header->magic == FAT_MAGIC_64){
		cputype = fat_archs64[i].cputype;
		cpusubtype = fat_archs64[i].cpusubtype;
		offset = fat_archs64[i].offset;
		size = fat_archs64[i].size;
		align = fat_archs64[i].align;
	    }
	    else{
		cputype = fat_archs[i].cputype;
		cpusubtype = fat_archs[i].cpusubtype;
		offset = fat_archs[i].offset;
		size = fat_archs[i].size;
		align = fat_archs[i].align;
	    }
	    if(verbose){
		print_arch(cputype, cpusubtype);
		print_cputype(cputype, cpusubtype & ~CPU_SUBTYPE_MASK);
	    }
	    else{
		printf("%u\n", i);
		printf("    cputype %d\n", cputype);
		printf("    cpusubtype %d\n", cpusubtype & ~CPU_SUBTYPE_MASK);
	    }
	    if(verbose && (cpusubtype & CPU_SUBTYPE_MASK) ==
	       CPU_SUBTYPE_LIB64)
		printf("    capabilities CPU_SUBTYPE_LIB64\n");
	    else
		printf("    capabilities 0x%x\n", (unsigned int)
		       ((cpusubtype & CPU_SUBTYPE_MASK) >>24));
	    printf("    offset %llu", offset);
	    if(offset > filesize)
		printf(" (past end of file)");
	    if(offset % (1 << align) != 0)
		printf(" (not aligned on it's alignment (2^%u))\n", align);
	    else
		printf("\n");

	    printf("    size %llu", size);
	    big_size = offset;
	    big_size += size;
	    if(big_size > filesize)
		printf(" (past end of file)\n");
	    else
		printf("\n");

	    printf("    align 2^%u (%d)", align, 1 << align);
	    if(align > MAXSECTALIGN)
		printf("( too large, maximum 2^%d)\n", MAXSECTALIGN);
	    else
		printf("\n");
	}
}

/*
 * print_arch() helps print_fat_headers by printing the
 * architecture name for the cputype and cpusubtype.
 */
static
void
print_arch(
cpu_type_t cputype,
cpu_subtype_t cpusubtype)
{
	switch(cputype){
	case CPU_TYPE_MC680x0:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_MC680x0_ALL:
		printf("m68k\n");
		break;
	    case CPU_SUBTYPE_MC68030_ONLY:
		printf("m68030\n");
		break;
	    case CPU_SUBTYPE_MC68040:
		printf("m68040\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_MC88000:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_MC88000_ALL:
	    case CPU_SUBTYPE_MC88110:
		printf("m88k\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_I386:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_I386_ALL:
	    /* case CPU_SUBTYPE_386: same as above */
		printf("i386\n");
		break;
	    case CPU_SUBTYPE_486:
		printf("i486\n");
		break;
	    case CPU_SUBTYPE_486SX:
		printf("i486SX\n");
		break;
	    case CPU_SUBTYPE_PENT: /* same as 586 */
		printf("pentium\n");
		break;
	    case CPU_SUBTYPE_PENTPRO:
		printf("pentpro\n");
		break;
	    case CPU_SUBTYPE_PENTII_M3:
		printf("pentIIm3\n");
		break;
	    case CPU_SUBTYPE_PENTII_M5:
		printf("pentIIm5\n");
		break;
	    default:
		printf("intel x86 family %d model %d\n",
		       CPU_SUBTYPE_INTEL_FAMILY(cpusubtype &
						~CPU_SUBTYPE_MASK),
		       CPU_SUBTYPE_INTEL_MODEL(cpusubtype &
					       ~CPU_SUBTYPE_MASK));
		break;
	    }
	    break;
	case CPU_TYPE_X86_64:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_X86_64_ALL:
		printf("x86_64\n");
		break;
	    case CPU_SUBTYPE_X86_64_H:
		printf("x86_64h\n");
		break;
	    default:
		goto print_arch_unknown;
	    }		
	    break;
	case CPU_TYPE_I860:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_I860_ALL:
	    case CPU_SUBTYPE_I860_860:
		printf("i860\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_POWERPC:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_POWERPC_ALL:
		printf("ppc\n");
		break;
	    case CPU_SUBTYPE_POWERPC_601:
		printf("ppc601\n");
		break;
	    case CPU_SUBTYPE_POWERPC_602:
		printf("ppc602\n");
		break;
	    case CPU_SUBTYPE_POWERPC_603:
		printf("ppc603\n");
		break;
	    case CPU_SUBTYPE_POWERPC_603e:
		printf("ppc603e\n");
		break;
	    case CPU_SUBTYPE_POWERPC_603ev:
		printf("ppc603ev\n");
		break;
	    case CPU_SUBTYPE_POWERPC_604:
		printf("ppc604\n");
		break;
	    case CPU_SUBTYPE_POWERPC_604e:
		printf("ppc604e\n");
		break;
	    case CPU_SUBTYPE_POWERPC_620:
		printf("ppc620\n");
		break;
	    case CPU_SUBTYPE_POWERPC_750:
		printf("ppc750\n");
		break;
	    case CPU_SUBTYPE_POWERPC_7400:
		printf("ppc7400\n");
		break;
	    case CPU_SUBTYPE_POWERPC_7450:
		printf("ppc7450\n");
		break;
	    case CPU_SUBTYPE_POWERPC_970:
		printf("ppc970\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_POWERPC64:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_POWERPC_ALL:
		printf("ppc64\n");
		break;
	    case CPU_SUBTYPE_POWERPC_970:
		printf("ppc970-64\n");
		break;
	    default:
		goto print_arch_unknown;
	    }		
	    break;
	case CPU_TYPE_VEO:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_VEO_1:
		printf("veo1\n");
		break;
	    case CPU_SUBTYPE_VEO_2:
		printf("veo2\n");
		break;
	    case CPU_SUBTYPE_VEO_3:
		printf("veo3\n");
		break;
	    case CPU_SUBTYPE_VEO_4:
		printf("veo4\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_HPPA:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_HPPA_ALL:
	    case CPU_SUBTYPE_HPPA_7100LC:
		printf("hppa\n");
	    break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_SPARC:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_SPARC_ALL:
		printf("sparc\n");
	    break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_ARM:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_ARM_ALL:
		printf("arm\n");
		break;
	    case CPU_SUBTYPE_ARM_V4T:
		printf("armv4t\n");
		break;
	    case CPU_SUBTYPE_ARM_V5TEJ:
		printf("armv5\n");
		break;
	    case CPU_SUBTYPE_ARM_XSCALE:
		printf("xscale\n");
		break;
	    case CPU_SUBTYPE_ARM_V6:
		printf("armv6\n");
		break;
	    case CPU_SUBTYPE_ARM_V6M:
		printf("armv6m\n");
		break;
	    case CPU_SUBTYPE_ARM_V7:
		printf("armv7\n");
		break;
	    case CPU_SUBTYPE_ARM_V7F:
		printf("armv7f\n");
		break;
	    case CPU_SUBTYPE_ARM_V7S:
		printf("armv7s\n");
		break;
	    case CPU_SUBTYPE_ARM_V7K:
		printf("armv7k\n");
		break;
	    case CPU_SUBTYPE_ARM_V7M:
		printf("armv7m\n");
		break;
	    case CPU_SUBTYPE_ARM_V7EM:
		printf("armv7em\n");
		break;
	    default:
		goto print_arch_unknown;
		break;
	    }
	    break;
	case CPU_TYPE_ARM64:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_ARM64_ALL:
		printf("arm64\n");
	    break;
	    case CPU_SUBTYPE_ARM64_V8:
		printf("arm64v8\n");
	    case CPU_SUBTYPE_ARM64E:
		printf("arm64e\n");
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_ARM64_32:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_ARM64_32_V8:
		printf("arm64_32\n");
	    break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_ANY:
	    switch((int)(cpusubtype & ~CPU_SUBTYPE_MASK)){
	    case CPU_SUBTYPE_MULTIPLE:
		printf("any\n");
		break;
	    case CPU_SUBTYPE_LITTLE_ENDIAN:
		printf("little\n");
		break;
	    case CPU_SUBTYPE_BIG_ENDIAN:
		printf("big\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
print_arch_unknown:
	default:
	    printf("cputype (%d) cpusubtype (%d)\n", cputype,
		   cpusubtype & ~CPU_SUBTYPE_MASK);
	    break;
	}
}

/*
 * print_cputype() helps print_fat_headers by printing the cputype and
 * cpusubtype (symbolically for the one's it knows about).
 */
static
void
print_cputype(
cpu_type_t cputype,
cpu_subtype_t cpusubtype)
{
	switch(cputype){
	case CPU_TYPE_MC680x0:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_MC680x0_ALL:
		printf("    cputype CPU_TYPE_MC680x0\n"
		       "    cpusubtype CPU_SUBTYPE_MC680x0_ALL\n");
		break;
	    case CPU_SUBTYPE_MC68030_ONLY:
		printf("    cputype CPU_TYPE_MC680x0\n"
		       "    cpusubtype CPU_SUBTYPE_MC68030_ONLY\n");
		break;
	    case CPU_SUBTYPE_MC68040:
		printf("    cputype CPU_TYPE_MC680x0\n"
		       "    cpusubtype CPU_SUBTYPE_MC68040\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_MC88000:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_MC88000_ALL:
		printf("    cputype CPU_TYPE_MC88000\n"
		       "    cpusubtype CPU_SUBTYPE_MC88000_ALL\n");
		break;
	    case CPU_SUBTYPE_MC88110:
		printf("    cputype CPU_TYPE_MC88000\n"
		       "    cpusubtype CPU_SUBTYPE_MC88110\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_I386:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_I386_ALL:
	    /* case CPU_SUBTYPE_386: same as above */
		printf("    cputype CPU_TYPE_I386\n"
		       "    cpusubtype CPU_SUBTYPE_I386_ALL\n");
		break;
	    case CPU_SUBTYPE_486:
		printf("    cputype CPU_TYPE_I386\n"
		       "    cpusubtype CPU_SUBTYPE_486\n");
		break;
	    case CPU_SUBTYPE_486SX:
		printf("    cputype CPU_TYPE_I386\n"
		       "    cpusubtype CPU_SUBTYPE_486SX\n");
		break;
	    case CPU_SUBTYPE_PENT: /* same as 586 */
		printf("    cputype CPU_TYPE_I386\n"
		       "    cpusubtype CPU_SUBTYPE_PENT\n");
		break;
	    case CPU_SUBTYPE_PENTPRO:
		printf("    cputype CPU_TYPE_I386\n"
		       "    cpusubtype CPU_SUBTYPE_PENTPRO\n");
		break;
	    case CPU_SUBTYPE_PENTII_M3:
		printf("    cputype CPU_TYPE_I386\n"
		       "    cpusubtype CPU_SUBTYPE_PENTII_M3\n");
		break;
	    case CPU_SUBTYPE_PENTII_M5:
		printf("    cputype CPU_TYPE_I386\n"
		       "    cpusubtype CPU_SUBTYPE_PENTII_M5\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_X86_64:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_X86_64_ALL:
		printf("    cputype CPU_TYPE_X86_64\n"
		       "    cpusubtype CPU_SUBTYPE_X86_64_ALL\n");
		break;
	    case CPU_SUBTYPE_X86_64_H:
		printf("    cputype CPU_TYPE_X86_64\n"
		       "    cpusubtype CPU_SUBTYPE_X86_64_H\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_I860:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_I860_ALL:
		printf("    cputype CPU_TYPE_I860\n"
		       "    cpusubtype CPU_SUBTYPE_I860_ALL\n");
		break;
	    case CPU_SUBTYPE_I860_860:
		printf("    cputype CPU_TYPE_I860\n"
		       "    cpusubtype CPU_SUBTYPE_I860_860\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_POWERPC:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_POWERPC_ALL:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_ALL\n");
		break;
	    case CPU_SUBTYPE_POWERPC_601:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_601\n");
		break;
	    case CPU_SUBTYPE_POWERPC_602:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_602\n");
		break;
	    case CPU_SUBTYPE_POWERPC_603:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_603\n");
		break;
	    case CPU_SUBTYPE_POWERPC_603e:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_603e\n");
		break;
	    case CPU_SUBTYPE_POWERPC_603ev:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_603ev\n");
		break;
	    case CPU_SUBTYPE_POWERPC_604:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_604\n");
		break;
	    case CPU_SUBTYPE_POWERPC_604e:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_604e\n");
		break;
	    case CPU_SUBTYPE_POWERPC_620:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_620\n");
		break;
	    case CPU_SUBTYPE_POWERPC_750:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_750\n");
		break;
	    case CPU_SUBTYPE_POWERPC_7400:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_7400\n");
		break;
	    case CPU_SUBTYPE_POWERPC_7450:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_7450\n");
		break;
	    case CPU_SUBTYPE_POWERPC_970:
		printf("    cputype CPU_TYPE_POWERPC\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_970\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_POWERPC64:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_POWERPC_ALL:
		printf("    cputype CPU_TYPE_POWERPC64\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC64_ALL\n");
		break;
	    case CPU_SUBTYPE_POWERPC_970:
		printf("    cputype CPU_TYPE_POWERPC64\n"
		       "    cpusubtype CPU_SUBTYPE_POWERPC_970\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_VEO:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_VEO_1:
		printf("    cputype CPU_TYPE_VEO\n"
		       "    cpusubtype CPU_SUBTYPE_VEO_1\n");
		break;
	    case CPU_SUBTYPE_VEO_2:
		printf("    cputype CPU_TYPE_VEO\n"
		       "    cpusubtype CPU_SUBTYPE_VEO_2\n");
		break;
	    case CPU_SUBTYPE_VEO_3:
		printf("    cputype CPU_TYPE_VEO\n"
		       "    cpusubtype CPU_SUBTYPE_VEO_3\n");
		break;
	    case CPU_SUBTYPE_VEO_4:
		printf("    cputype CPU_TYPE_VEO\n"
		       "    cpusubtype CPU_SUBTYPE_VEO_4\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_HPPA:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_HPPA_ALL:
		printf("    cputype CPU_TYPE_HPPA\n"
		       "    cpusubtype CPU_SUBTYPE_HPPA_ALL\n");
	    	break;
	    case CPU_SUBTYPE_HPPA_7100LC:
		printf("    cputype CPU_TYPE_HPPA\n"
		       "    cpusubtype CPU_SUBTYPE_HPPA_7100LC\n");
	    	break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_SPARC:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_SPARC_ALL:
		printf("    cputype CPU_TYPE_SPARC\n"
		       "    cpusubtype CPU_SUBTYPE_SPARC_ALL\n");
	    	break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_ARM:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_ARM_ALL:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_ALL\n");
		break;
	    case CPU_SUBTYPE_ARM_V4T:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V4T\n");
		break;
	    case CPU_SUBTYPE_ARM_V5TEJ:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V5TEJ\n");
		break;
	    case CPU_SUBTYPE_ARM_XSCALE:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_XSCALE\n");
		break;
	    case CPU_SUBTYPE_ARM_V6:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V6\n");
		break;
	    case CPU_SUBTYPE_ARM_V6M:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V6M\n");
		break;
	    case CPU_SUBTYPE_ARM_V7:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V7\n");
		break;
	    case CPU_SUBTYPE_ARM_V7F:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V7F\n");
		break;
	    case CPU_SUBTYPE_ARM_V7S:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V7S\n");
		break;
	    case CPU_SUBTYPE_ARM_V7K:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V7K\n");
		break;
	    case CPU_SUBTYPE_ARM_V7M:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V7M\n");
		break;
	    case CPU_SUBTYPE_ARM_V7EM:
		printf("    cputype CPU_TYPE_ARM\n"
		       "    cpusubtype CPU_SUBTYPE_ARM_V7EM\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_ARM64:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_ARM64_ALL:
		printf("    cputype CPU_TYPE_ARM64\n"
		       "    cpusubtype CPU_SUBTYPE_ARM64_ALL\n");
	    	break;
	    case CPU_SUBTYPE_ARM64_V8:
		printf("    cputype CPU_TYPE_ARM64\n"
		       "    cpusubtype CPU_SUBTYPE_ARM64_V8\n");
	    	break;
	    case CPU_SUBTYPE_ARM64E:
		printf("    cputype CPU_TYPE_ARM64\n"
		       "    cpusubtype CPU_SUBTYPE_ARM64E\n");
	    	break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_ARM64_32:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_ARM64_32_V8:
		printf("    cputype CPU_TYPE_ARM64_32\n"
		       "    cpusubtype CPU_SUBTYPE_ARM64_32_V8\n");
	    	break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
	case CPU_TYPE_ANY:
	    switch((int)(cpusubtype & ~CPU_SUBTYPE_MASK)){
	    case CPU_SUBTYPE_MULTIPLE:
		printf("    cputype CPU_TYPE_ANY\n"
		       "    cpusubtype CPU_SUBTYPE_MULTIPLE\n");
		break;
	    case CPU_SUBTYPE_LITTLE_ENDIAN:
		printf("    cputype CPU_TYPE_ANY\n"
		       "    cpusubtype CPU_SUBTYPE_LITTLE_ENDIAN\n");
		break;
	    case CPU_SUBTYPE_BIG_ENDIAN:
		printf("    cputype CPU_TYPE_ANY\n"
		       "    cpusubtype CPU_SUBTYPE_BIG_ENDIAN\n");
		break;
	    default:
		goto print_arch_unknown;
	    }
	    break;
print_arch_unknown:
	default:
	    printf("    cputype (%d)\n"
		   "    cpusubtype (%d)\n", cputype,
			cpusubtype & ~CPU_SUBTYPE_MASK);
	    break;
	}
}

/*
 * Print a formatted version number where components are encoded into a
 * uint32_t in nibbles: X.Y.Z => 0xXXXXYYZZ. If a 'label' is supplied, it will
 * precede the version number and a newline will follow.
 */
void print_version_xyz(const char* label, uint32_t version)
{
    const char* space = " ";
    const char* nl = "\n";
    if (label == NULL)
	label = space = nl = "";
    if((version & 0xff) == 0)
	printf("%s%s%u.%u%s",
	       label,
	       space,
	       version >> 16,
	       (version >> 8) & 0xff,
	       nl);
    else
	printf("%s%s%u.%u.%u%s",
	       label,
	       space,
	       version >> 16,
	       (version >> 8) & 0xff,
	       version & 0xff,
	       nl);
}

/*
 * Print the archive header.  The format is constant width character fields
 * blank padded.  So the trailing blanks are stripped and full field widths
 * are handled correctly.
 */
void
print_ar_hdr(
struct ar_hdr *ar_hdr,
char *member_name,
uint32_t member_name_size,
uint64_t member_offset,
enum bool verbose,
enum bool print_offset)
{
    int32_t i;
    uint32_t j, mode;
    time_t date;
    char *p, *endp;

    char date_buf[sizeof(ar_hdr->ar_date) + 1];
    char  uid_buf[sizeof(ar_hdr->ar_uid)  + 1];
    char  gid_buf[sizeof(ar_hdr->ar_gid)  + 1];
    char mode_buf[sizeof(ar_hdr->ar_mode) + 1];
    char size_buf[sizeof(ar_hdr->ar_size) + 1];

	memcpy(date_buf, ar_hdr->ar_date, sizeof(ar_hdr->ar_date));
	for(i = sizeof(ar_hdr->ar_date) - 1; i >= 0 && date_buf[i] == ' '; i--)
	    date_buf[i] = '\0';
	date_buf[sizeof(ar_hdr->ar_date)] = '\0';

	memcpy(uid_buf, ar_hdr->ar_uid, sizeof(ar_hdr->ar_uid));
	for(i = sizeof(ar_hdr->ar_uid) - 1; i >= 0 && uid_buf[i] == ' '; i--)
	    uid_buf[i] = '\0';
	uid_buf[sizeof(ar_hdr->ar_uid)] = '\0';

	memcpy(gid_buf, ar_hdr->ar_gid, sizeof(ar_hdr->ar_gid));
	for(i = sizeof(ar_hdr->ar_gid) - 1; i >= 0 && gid_buf[i] == ' '; i--)
	    gid_buf[i] = '\0';
	gid_buf[sizeof(ar_hdr->ar_gid)] = '\0';

	memcpy(mode_buf, ar_hdr->ar_mode, sizeof(ar_hdr->ar_mode));
	for(i = sizeof(ar_hdr->ar_mode) - 1; i >= 0 && mode_buf[i] == ' '; i--)
	    mode_buf[i] = '\0';
	mode_buf[sizeof(ar_hdr->ar_mode)] = '\0';

	memcpy(size_buf, ar_hdr->ar_size, sizeof(ar_hdr->ar_size));
	for(i = sizeof(ar_hdr->ar_size) - 1; i >= 0 && size_buf[i] == ' '; i--)
	    size_buf[i] = '\0';
	size_buf[sizeof(ar_hdr->ar_size)] = '\0';

	if(print_offset == TRUE)
	    printf("%llu\t", member_offset);

	if(verbose == TRUE){
	    mode = (uint32_t)strtoul(mode_buf, &endp, 8);
	    if(*endp != '\0')
		printf("(mode: \"%s\" contains non-octal chars) ", mode_buf);
	    print_mode_verbose(mode);
	}
	else
	    /* printf("0%03o ", mode & 0777); */
	    printf("0%s ", mode_buf);

	printf("%3s/%-3s %5s ", uid_buf, gid_buf, size_buf);

	/*
	 * Since cime(3) returns a 26 character string of the form:
	 * "Sun Sep 16 01:03:52 1973\n\0"
	 * and the new line is not wanted a '\0' is placed there.
	 */
	if(verbose){
	    date = (uint32_t)strtoul(date_buf, &endp, 10);
	    if(*endp != '\0')
		printf("(date: \"%s\" contains non-decimal chars) ", date_buf);
	    p = ctime(&date);
	    p[24] = '\0';
	    printf("%s ", p);
	}
	else
	    printf("%s ", date_buf);

	if(verbose){
	    printf("%.*s", (int)member_name_size, member_name);
	}
	else{
	    j = size_ar_name(ar_hdr);
	    printf("%.*s", (int)j, ar_hdr->ar_name);
	}

	if(memcmp(ar_hdr->ar_fmag, ARFMAG, sizeof(ARFMAG) - 1) == 0)
	    printf("\n");
	else
	    printf(" (ar_fmag not ARFMAG)\n");
}

void
print_mode_verbose(
uint32_t mode)
{
	switch(mode & S_IFMT){
	case S_IFDIR:
	    printf("d");
	    break;
	case S_IFCHR:
	    printf("c");
	    break;
	case S_IFBLK:
	    printf("b");
	    break;
	case S_IFREG:
	    printf("-");
	    break;
	case S_IFLNK:
	    printf("l");
	    break;
	case S_IFSOCK:
	    printf("s");
	    break;
	default:
	    printf("?");
	    break;
	}

	/* owner permissions */
	if(mode & S_IREAD)
	    printf("r");
	else
	    printf("-");
	if(mode & S_IWRITE)
	    printf("w");
	else
	    printf("-");
	if(mode & S_ISUID)
	    printf("s");
	else if(mode & S_IEXEC)
	    printf("x");
	else
	    printf("-");

	/* group permissions */
	if(mode & (S_IREAD >> 3))
	    printf("r");
	else
	    printf("-");
	if(mode & (S_IWRITE >> 3))
	    printf("w");
	else
	    printf("-");
	if(mode & S_ISGID)
	    printf("s");
	else if(mode & (S_IEXEC >> 3))
	    printf("x");
	else
	    printf("-");

	/* other permissions */
	if(mode & (S_IREAD >> 6))
	    printf("r");
	else
	    printf("-");
	if(mode & (S_IWRITE >> 6))
	    printf("w");
	else
	    printf("-");
	if(mode & S_ISVTX)
	    printf("t");
	else if(mode & (S_IEXEC >> 6))
	    printf("x");
	else
	    printf("-");
}

/*
 * print_library_toc prints the table of contents of the a library.  It is
 * converted to the host byte sex if toc_byte_sex is not the host byte sex.
 * The problem with determing the byte sex of the table of contents is left
 * to the caller.  The determination is based on the byte sex of the object
 * files contained in the library (this can still present a problem since the
 * object files could be of differing byte sex in an erroneous library).  There
 * is no problem of a library containing no objects with respect to the byte
 * sex of the table of contents since the table of contents would be made up
 * of two binary uint32_t or two binary uint64_t zeros which are the same in
 * either byte sex.
 */
void
print_library_toc(
struct ar_hdr *toc_ar_hdr,
char *toc_name,
uint32_t toc_name_size,
char *toc_addr,
uint32_t toc_size,
enum byte_sex toc_byte_sex,
char *library_name,
char *library_addr,
uint64_t library_size,
char *arch_name,
enum bool verbose)
{
    enum bool using_64toc;
    enum byte_sex host_byte_sex;
    uint32_t ran_size, str_size, i, member_name_size;
    uint64_t ran_size64, nranlibs, str_size64, sizeof_rans, string_size;
    uint64_t toc_offset, ran_off, ran_strx;
    struct ranlib *ranlibs;
    struct ranlib_64 *ranlibs64;
    char *strings, *member_name;
    struct ar_hdr *ar_hdr;
    int n;
    char buf[20];
    uint64_t big_size;

	/* cctools-port start */
	str_size = 0;
	string_size = 0;
	/* cctools-port end */

	host_byte_sex = get_host_byte_sex();
	toc_offset = 0;
	strings = NULL;
	ran_size = 0;
	ran_size64 = 0;
	string_size = 0;

	if(strncmp(toc_name, SYMDEF_64, sizeof(SYMDEF_64)-1) == 0 ||
	   strncmp(toc_name, SYMDEF_64_SORTED, sizeof(SYMDEF_64_SORTED)-1) == 0)
	    using_64toc = TRUE;
	else
	    using_64toc = FALSE;

	if(using_64toc == FALSE){
	    if(toc_offset + sizeof(uint32_t) > toc_size){
		error_with_arch(arch_name, "truncated table of contents in: "
		    "%s(%.*s) (size of ranlib structs extends past the end "
		    "of the table of contents member)", library_name,
		    (int)toc_name_size, toc_name);
		return;
	    }
	    memcpy((char *)&ran_size, toc_addr + toc_offset, sizeof(uint32_t));
	}
	else{
	    if(toc_offset + sizeof(uint64_t) > toc_size){
		error_with_arch(arch_name, "truncated table of contents in: "
		    "%s(%.*s) (size of ranlib_64 structs extends past the end "
		    "of the table of contents member)", library_name,
		    (int)toc_name_size, toc_name);
		return;
	    }
	    memcpy((char *)&ran_size64, toc_addr +toc_offset, sizeof(uint64_t));
	}
	/*
	 * With the advent of things like LTO object files we may end up getting
	 * handed UNKNOWN_BYTE_SEX for the table of contents byte sex.  So at
	 * this point we are guessing.  A better guess is to go with the host
	 * bytesex as that is more likely.  Otherwise we will always think it is
	 * swapped.
	 */
	if(toc_byte_sex == UNKNOWN_BYTE_SEX)
	    toc_byte_sex = host_byte_sex;
	if(toc_byte_sex != host_byte_sex){
	    if(using_64toc == FALSE)
		ran_size = SWAP_INT(ran_size);
	    else
		ran_size64 = SWAP_LONG_LONG(ran_size64);
	}
	if(using_64toc == FALSE)
	    toc_offset += sizeof(uint32_t);
	else
	    toc_offset += sizeof(uint64_t);

	big_size = toc_offset;
	if(using_64toc == FALSE)
	    big_size += ran_size;
	else
	    big_size += ran_size64;
	if(big_size > toc_size){
	    if(using_64toc == FALSE)
		error_with_arch(arch_name, "truncated table of contents in: "
		    "%s(%.*s) (ranlib structures extends past the end of "
		    "the table of contents member)", library_name,
		    (int)toc_name_size, toc_name);
	    else
		error_with_arch(arch_name, "truncated table of contents in: "
		    "%s(%.*s) (ranlib_64 structures extends past the end of "
		    "the table of contents member)", library_name,
		    (int)toc_name_size, toc_name);
	    return;
	}
	if(using_64toc == FALSE){
	    ranlibs = allocate(ran_size);
	    ranlibs64 = NULL;
	    memcpy((char *)ranlibs, toc_addr + toc_offset, ran_size);
	    nranlibs = ran_size / sizeof(struct ranlib);
	    if(toc_byte_sex != host_byte_sex)
		swap_ranlib(ranlibs, (uint32_t)nranlibs, host_byte_sex);
	    sizeof_rans = ran_size;
	    toc_offset += ran_size;
	}
	else{
	    ranlibs64 = allocate(ran_size64);
	    ranlibs = NULL;
	    memcpy((char *)ranlibs64, toc_addr + toc_offset, ran_size64);
	    nranlibs = ran_size64 / sizeof(struct ranlib_64);
	    if(toc_byte_sex != host_byte_sex)
		swap_ranlib_64(ranlibs64, nranlibs, host_byte_sex);
	    sizeof_rans = ran_size64;
	    toc_offset += ran_size64;
	}

	if(verbose){
	    if(using_64toc == FALSE){
		if(toc_offset + sizeof(uint32_t) > toc_size){
		    error_with_arch(arch_name, "truncated table of contents "
			"in: %s(%.*s) (size of ranlib strings extends past "
			"the end of the table of contents member)",
			library_name, (int)toc_name_size, toc_name);
		    free(ranlibs);
		    return;
		}
		memcpy((char *)&str_size, toc_addr + toc_offset,
		       sizeof(uint32_t));
		if(toc_byte_sex != host_byte_sex)
		    str_size = SWAP_INT(str_size);
		string_size = str_size;
		toc_offset += sizeof(uint32_t);
	    }
	    else{
		if(toc_offset + sizeof(uint64_t) > toc_size){
		    error_with_arch(arch_name, "truncated table of contents "
			"in: %s(%.*s) (size of ranlib strings extends past "
			"the end of the table of contents member)",
			library_name, (int)toc_name_size, toc_name);
		    free(ranlibs64);
		    return;
		}
		memcpy((char *)&str_size64, toc_addr + toc_offset,
		       sizeof(uint64_t));
		if(toc_byte_sex != host_byte_sex)
		    str_size64 = SWAP_LONG_LONG(str_size64);
		string_size = str_size64;
		toc_offset += sizeof(uint64_t);
	    }

	    big_size = toc_offset;
	    big_size += string_size;
	    if(big_size > toc_size){
		error_with_arch(arch_name, "truncated table of contents in: "
		    "%s(%.*s) (ranlib strings extends past the end of the "
		    "table of contents member)", library_name,
		    (int)toc_name_size, toc_name);
		if(ranlibs != NULL)
		    free(ranlibs);
		if(ranlibs64 != NULL)
		    free(ranlibs64);
		return;
	    }
	    strings = toc_addr + toc_offset;
	}

	printf("Table of contents from: %s(%.*s)", library_name,
	       (int)toc_name_size, toc_name);
	if(arch_name != NULL)
	    printf(" (for architecture %s)\n", arch_name);
	else
	    printf("\n");
	printf("size of ranlib structures: %llu (number %llu)\n", sizeof_rans,
	       nranlibs);
	if(verbose){
	    printf("size of strings: %llu", string_size);
	    if(using_64toc == FALSE){
		if(string_size % sizeof(int32_t) != 0)
		    printf(" (not multiple of sizeof(int32_t))");
	    }
	    else{
		if(string_size % sizeof(int64_t) != 0)
		    printf(" (not multiple of sizeof(int64_t))");
	    }
	    printf("\n");
	}
	if(verbose)
	    printf("object           symbol name\n");
	else
	    printf("object offset  string index\n");

	for(i = 0; i < nranlibs; i++){
	    if(using_64toc == FALSE){
		ran_off = ranlibs[i].ran_off;
		ran_strx = ranlibs[i].ran_un.ran_strx;
	    }
	    else{
		ran_off = ranlibs64[i].ran_off;
		ran_strx = ranlibs64[i].ran_un.ran_strx;
	    }
	    if(verbose){
		if(ran_off + sizeof(struct ar_hdr) <= library_size){
		    ar_hdr = (struct ar_hdr *)
			     (library_addr + ran_off);
		    if(strncmp(ar_hdr->ar_name, AR_EFMT1,
			       sizeof(AR_EFMT1) - 1) == 0){
			member_name = ar_hdr->ar_name + sizeof(struct ar_hdr);
			member_name_size = (uint32_t)strtoul(ar_hdr->ar_name +
				sizeof(AR_EFMT1) - 1, NULL, 10);
			while(member_name_size > 0 &&
			      member_name[member_name_size - 1] == '\0')
			    member_name_size--;
			printf("%-.*s ", (int)member_name_size, member_name);
			if(member_name_size < 16)
			    printf("%-.*s", (int)(16 - member_name_size),
				   "                ");
		    }
		    else{
			printf("%-.16s ", ar_hdr->ar_name);
		    }
		}
		else{
		    n = sprintf(buf, "?(%llu) ", ran_off);
		    printf("%s%.*s", buf, 17 - n, "              ");
		}
		if(ran_strx < string_size)
		    printf("%s\n", strings + ran_strx);
		else
		    printf("?(%llu)\n", ran_strx);
	    }
	    else{
		printf("%-14llu %llu\n", ran_off, ran_strx);
	    }
	}

	if(ranlibs != NULL)
	    free(ranlibs);
	if(ranlibs64 != NULL)
	    free(ranlibs64);
}

/*
 * print_sysv_library_toc prints the table of contents of a System V format 
 * archive library.
 */
void
print_sysv_library_toc(
struct ar_hdr *toc_ar_hdr,
char *toc_name,
uint32_t toc_name_size,
char *toc_addr,
uint32_t toc_size,
enum byte_sex toc_byte_sex,
char *library_name,
char *library_addr,
uint64_t library_size,
char *arch_name,
enum bool verbose)
{
    enum byte_sex host_byte_sex;
    uint32_t i, j, num_entries, *memoffrefs, sym_names_size, max_sym_name_len,
	     member_name_offset;
    uint64_t toc_offset, big_size;
    char *sym_names, *sym_name;
    struct ar_hdr *ar_hdr;
    int n;
    char buf[20];

    struct ar_hdr *strtab_ar_hdr;
    char *ar_strtab;
    uint32_t ar_strtab_size, library_size_after_toc;

	host_byte_sex = get_host_byte_sex();
	toc_offset = 0;

	/*
	 * The first thing in the toc is a 32-bit big endian value of the
	 * count of the number of symbol table entries that follow it.
	 */
	if(toc_offset + sizeof(uint32_t) > toc_size){
	    error_with_arch(arch_name, "truncated table of contents in: "
		"%s(%.*s) (number of symbol table entries extends past the end "
		"of the table of contents member)", library_name,
		(int)toc_name_size, toc_name);
	    return;
	}
	memcpy((char *)&num_entries, toc_addr + toc_offset, sizeof(uint32_t));
	if(host_byte_sex != BIG_ENDIAN_BYTE_SEX)
	    num_entries = SWAP_INT(num_entries);
	toc_offset += sizeof(uint32_t);

	/*
	 * Next in the toc is num_entries of 32-bit big endian values which
	 * are offsets to the members that define a symbol.
	 */
	big_size = num_entries * sizeof(uint32_t);
	big_size += toc_offset;
	if(big_size > toc_size){
	    error_with_arch(arch_name, "truncated table of contents in: "
		"%s(%.*s) (member offset referernces extends past the end of "
		"the table of contents member)", library_name,
		(int)toc_name_size, toc_name);
	    return;
	}
	memoffrefs = allocate(num_entries * sizeof(uint32_t));
	memcpy((char *)memoffrefs, toc_addr + toc_offset,
	       num_entries * sizeof(uint32_t));
	if(host_byte_sex != BIG_ENDIAN_BYTE_SEX){
	    for(i = 0; i < num_entries; i++)
		memoffrefs[i] = SWAP_INT(memoffrefs[i]);
	}
	toc_offset += num_entries * sizeof(uint32_t);

	/*
	 * Lastly in the toc are a list of null terminated strings in the same
	 * order as the member offset references above.
	 */
	sym_names = toc_addr + toc_offset;
	sym_names_size = (uint32_t)(toc_size - toc_offset);

	/*
	 * Long archive member names are stored in the archive member contents
	 * of the archive member after the table of contents in an archive
	 * member with the name "//".  And referred to by the ar_name with
	 * the format "/offset" where the "offset" is a decimal offset into the
	 * archive member string names.
	 */
	library_size_after_toc = (uint32_t)(library_size -
	    (((char *)toc_ar_hdr + sizeof(struct ar_hdr) + toc_size) -
	    library_addr));
	ar_strtab = NULL;
	ar_strtab_size = 0;
	if(library_size_after_toc >= sizeof(struct ar_hdr)){
	    strtab_ar_hdr = (struct ar_hdr *)
		((char *)toc_ar_hdr + sizeof(struct ar_hdr) + toc_size);
	    if(strncmp(strtab_ar_hdr->ar_name, "// ", sizeof("// ")-1) == 0){
		ar_strtab_size = (uint32_t)strtoul(strtab_ar_hdr->ar_size,
						   NULL, 10);
		if(ar_strtab_size >
		   library_size_after_toc - sizeof(struct ar_hdr))
		    ar_strtab_size = library_size_after_toc -
				     sizeof(struct ar_hdr);
		ar_strtab = (char *)strtab_ar_hdr + sizeof(struct ar_hdr);
	    }
	}

	printf("Table of contents from: %s(%.*s)", library_name,
	       (int)toc_name_size, toc_name);
	if(arch_name != NULL)
	    printf(" (for architecture %s)\n", arch_name);
	else
	    printf("\n");
	printf("number of entries: %u\n", num_entries);
	if(verbose){
	    printf("size of strings: %u\n", sym_names_size);
	}
	if(verbose)
	    printf("object           symbol name\n");
	else
	    printf("object offset  string index\n");

	/*
	 * Loop through the table of contents entries.  Starting with the first
	 * symbol name through a count of num_entries.
	 */
	sym_name = sym_names;
	max_sym_name_len = sym_names_size;
	for(i = 0; i < num_entries; i++){
	    if(max_sym_name_len == 0){
		error_with_arch(arch_name, "truncated table of contents in: "
		    "%s(%.*s) (string table extends past the end of the table "
		    "of contents member)", library_name, (int)toc_name_size,
		    toc_name);
		return;
	    }
	    if(verbose){
		/*
		 * Print the archive member name for the member offset that
		 * is an offset from the start of the library to the archive
		 * header for that member.  Member names end in with a '/'
		 * character which we don't print.
		 */
		if(memoffrefs[i] + sizeof(struct ar_hdr) <= library_size){
		    ar_hdr = (struct ar_hdr *)(library_addr + memoffrefs[i]);
		    /*
		     * If the name starts with a '/' a decimal number follows
		     * it that is the offset into the member name string table.
		     */
		    if(ar_hdr->ar_name[0] == '/'){
			member_name_offset =
			    (uint32_t)strtoul(ar_hdr->ar_name + 1, NULL, 10);
			if(member_name_offset < ar_strtab_size){
			    for(n = member_name_offset;
				n < ar_strtab_size; n++){
				if(ar_strtab[n] != '/')
				    printf("%c", ar_strtab[n]);
				else
				    break;
			    }
			    if(n < ar_strtab_size){
				if((n - member_name_offset) <= 16)
				    printf("%.*s", 17 - (n -member_name_offset),
					   "                ");
				else
				    printf(" ");
			    }
			    else
				printf(" ");
			}
			else{
			    printf("bad member name offset %u ",
				   member_name_offset);
			}
		    }
		    else{
			for(n = 0; n < sizeof(ar_hdr->ar_name); n++){
			    if(ar_hdr->ar_name[n] != '/')
				printf("%c", ar_hdr->ar_name[n]);
			    else
				break;
			}
			printf("%.*s", 17 - n, "                ");
		    }
		}
		else{
		    n = sprintf(buf, "?(%u) ", memoffrefs[i]);
		    printf("%s%.*s", buf, 17 - n, "              ");
		}
	    }
	    else{
		printf("%-14u ", memoffrefs[i]);
	    }

	    if(verbose){
		printf("%.*s\n", max_sym_name_len, sym_name);
	    }
	    else{
		printf("%ld\n", sym_name - sym_names);
	    }

	    /*
	     * Adjust the maximum symbol name length left after this symbol
	     * name.
	     */
	    for(j = 0; sym_name[j] != '\0' && max_sym_name_len != 0; j++)
	       max_sym_name_len--;
	    if(max_sym_name_len != 0 && sym_name[j] == '\0'){
		max_sym_name_len--;
		sym_name = sym_name + j + 1;
	    }
	}
}

/*
 * Print the mach header.  It is assumed that the parameters are in the host
 * byte sex.  In this way it is up to the caller to determine he has a
 * mach_header and what byte sex it is and get it aligned in the host byte sex
 * for the parameters to this routine.
 */
void
print_mach_header(
uint32_t magic,
cpu_type_t cputype,
cpu_subtype_t cpusubtype,
uint32_t filetype,
uint32_t ncmds,
uint32_t sizeofcmds,
uint32_t flags,
enum bool verbose)
{
    uint32_t f;

	printf("Mach header\n");
	printf("      magic cputype cpusubtype  caps    filetype ncmds "
	       "sizeofcmds      flags\n");
	if(verbose){
	    if(magic == MH_MAGIC)
		printf("%11s", "MH_MAGIC");
	    else if(magic == MH_MAGIC_64)
		printf("%11s", "MH_MAGIC_64");
	    else
		printf(" 0x%08x", (unsigned int)magic);
	    switch(cputype){
	    case CPU_TYPE_POWERPC64:
		printf("   PPC64");
		switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_POWERPC_ALL:
		    printf("        ALL");
		    break;
		case CPU_SUBTYPE_POWERPC_970:
		    printf("     ppc970");
		    break;
		default:
		    printf(" %10d", cpusubtype & ~CPU_SUBTYPE_MASK);
		    break;
		}
		break;
	    case CPU_TYPE_X86_64:
		printf("  X86_64");
		switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_X86_64_ALL:
		    printf("        ALL");
		    break;
		case CPU_SUBTYPE_X86_64_H:
		    printf("    Haswell");
		    break;
		default:
		    printf(" %10d", cpusubtype & ~CPU_SUBTYPE_MASK);
		    break;
		}
		break;
	    case CPU_TYPE_VAX:
		printf("     VAX");
		switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_VAX780:
		    printf("     VAX780");
		    break;
		case CPU_SUBTYPE_VAX785:
		    printf("     VAX785");
		    break;
		case CPU_SUBTYPE_VAX750:
		    printf("     VAX750");
		    break;
		case CPU_SUBTYPE_VAX730:
		    printf("     VAX730");
		    break;
		case CPU_SUBTYPE_UVAXI:
		    printf("     UVAXI");
		    break;
		case CPU_SUBTYPE_UVAXII:
		    printf("     UVAXII");
		    break;
		case CPU_SUBTYPE_VAX8200:
		    printf("    VAX8200");
		    break;
		case CPU_SUBTYPE_VAX8500:
		    printf("    VAX8500");
		    break;
		case CPU_SUBTYPE_VAX8600:
		    printf("    VAX8600");
		    break;
		case CPU_SUBTYPE_VAX8650:
		    printf("    VAX8650");
		    break;
		case CPU_SUBTYPE_VAX8800:
		    printf("    VAX8800");
		    break;
		case CPU_SUBTYPE_UVAXIII:
		    printf("    UVAXIII");
		    break;
		default:
		    printf(" %10d", cpusubtype & ~CPU_SUBTYPE_MASK);
		    break;
		}
		break;
	    case CPU_TYPE_ROMP:
		printf("    ROMP");
		switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_RT_PC:
		    printf("      RT_PC");
		    break;
		case CPU_SUBTYPE_RT_APC:
		    printf("     RT_APC");
		    break;
		case CPU_SUBTYPE_RT_135:
		    printf("     RT_135");
		    break;

		default:
		    printf(" %10d", cpusubtype & ~CPU_SUBTYPE_MASK);
		    break;
		}
		break;
	    case CPU_TYPE_NS32032:
		printf(" NS32032");
		goto NS32;
	    case CPU_TYPE_NS32332:
		printf(" NS32332");
NS32:
		switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_MMAX_DPC:
		    printf("   MMAX_DPC");
		    break;
		case CPU_SUBTYPE_SQT:
		    printf("        SQT");
		    break;
		case CPU_SUBTYPE_MMAX_APC_FPU:
		    printf(" MMAX_APC_FPC");
		    break;
		case CPU_SUBTYPE_MMAX_APC_FPA:
		    printf(" MMAX_APC_FPA");
		    break;
		case CPU_SUBTYPE_MMAX_XPC:
		    printf("   MMAX_XPC");
		    break;
		default:
		    printf(" %10d", cpusubtype & ~CPU_SUBTYPE_MASK);
		    break;
		}
		break;
	    case CPU_TYPE_MC680x0:
		printf(" MC680x0");
		switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_MC680x0_ALL:
		    printf("        ALL");
		    break;
		case CPU_SUBTYPE_MC68030_ONLY:
		    printf("    MC68030");
		    break;
		case CPU_SUBTYPE_MC68040:
		    printf("    MC68040");
		    break;
		default:
		    printf(" %10d", cpusubtype & ~CPU_SUBTYPE_MASK);
		    break;
		}
		break;
	    case CPU_TYPE_MC88000:
		printf(" MC88000");
		switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_MC88000_ALL:
		    printf("        ALL");
		    break;
		case CPU_SUBTYPE_MC88100:
		    printf("    MC88100");
		    break;
		case CPU_SUBTYPE_MC88110:
		    printf("    MC88110");
		    break;
		default:
		    printf(" %10d", cpusubtype & ~CPU_SUBTYPE_MASK);
		    break;
		}
		break;
	    case CPU_TYPE_I860:
		printf("    I860");
		switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_I860_ALL:
		    printf("        ALL");
		    break;
		case CPU_SUBTYPE_I860_860:
		    printf("        860");
		    break;
		default:
		    printf(" %10d", cpusubtype & ~CPU_SUBTYPE_MASK);
		    break;
		}
		break;
	    case CPU_TYPE_I386:
		printf("    I386");
		switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_I386_ALL:
		/* case CPU_SUBTYPE_386: same as above */
		    printf("        ALL");
		    break;
		case CPU_SUBTYPE_486:
		    printf("        486");
		    break;
		case CPU_SUBTYPE_486SX:
		    printf("      486SX");
		    break;
		case CPU_SUBTYPE_PENT: /* same as 586 */
		    printf("       PENT");
		    break;
		case CPU_SUBTYPE_PENTPRO:
		    printf("    PENTPRO");
		    break;
		case CPU_SUBTYPE_PENTII_M3:
		    printf("  PENTII_M3");
		    break;
		case CPU_SUBTYPE_PENTII_M5:
		    printf("  PENTII_M5");
		    break;
		default:
		    printf(" %10d", cpusubtype & ~CPU_SUBTYPE_MASK);
		    break;
		}
		break;
	    case CPU_TYPE_POWERPC:
		printf("     PPC");
		switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_POWERPC_ALL:
		    printf("        ALL");
		    break;
		case CPU_SUBTYPE_POWERPC_601:
		    printf("     ppc601");
		    break;
		case CPU_SUBTYPE_POWERPC_602:
		    printf("     ppc602");
		    break;
		case CPU_SUBTYPE_POWERPC_603:
		    printf("     ppc603");
		    break;
		case CPU_SUBTYPE_POWERPC_603e:
		    printf("    ppc603e");
		    break;
		case CPU_SUBTYPE_POWERPC_603ev:
		    printf("   ppc603ev");
		    break;
		case CPU_SUBTYPE_POWERPC_604:
		    printf("     ppc604");
		    break;
		case CPU_SUBTYPE_POWERPC_604e:
		    printf("    ppc604e");
		    break;
		case CPU_SUBTYPE_POWERPC_620:
		    printf("     ppc620");
		    break;
		case CPU_SUBTYPE_POWERPC_750:
		    printf("     ppc750");
		    break;
		case CPU_SUBTYPE_POWERPC_7400:
		    printf("    ppc7400");
		    break;
		case CPU_SUBTYPE_POWERPC_7450:
		    printf("    ppc7450");
		    break;
		case CPU_SUBTYPE_POWERPC_970:
		    printf("     ppc970");
		    break;
		default:
		    printf(" %10d", cpusubtype & ~CPU_SUBTYPE_MASK);
		    break;
		}
		break;
	    case CPU_TYPE_VEO:
		printf("     VEO");
		switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_VEO_1:
		    printf("       veo1");
		    break;
		case CPU_SUBTYPE_VEO_2:
		    printf("       veo2");
		    break;
		case CPU_SUBTYPE_VEO_3:
		    printf("       veo3");
		    break;
		case CPU_SUBTYPE_VEO_4:
		    printf("       veo4");
		    break;
		default:
		    printf(" %10d", cpusubtype & ~CPU_SUBTYPE_MASK);
		    break;
		}
		break;
	    case CPU_TYPE_HPPA:
		printf("    HPPA");
		switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_HPPA_ALL:
		    printf("        ALL");
		    break;
		case CPU_SUBTYPE_HPPA_7100LC:
		    printf("  HPPA_7100LC");
		    break;
		default:
		    printf(" %10d", cpusubtype & ~CPU_SUBTYPE_MASK);
		    break;
		}
		break;
	    case CPU_TYPE_SPARC:
		printf("   SPARC");
		switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_SPARC_ALL:
		    printf("        ALL");
		    break;
		default:
		    printf(" %10d", cpusubtype & ~CPU_SUBTYPE_MASK);
		    break;
		}
		break;
	    case CPU_TYPE_ARM:
		printf("     ARM");
		switch(cpusubtype){
		case CPU_SUBTYPE_ARM_ALL:
		    printf("        ALL");
		    break;
		case CPU_SUBTYPE_ARM_V4T:
		    printf("        V4T");
		    break;
		case CPU_SUBTYPE_ARM_V5TEJ:
		    printf("      V5TEJ");
		    break;
		case CPU_SUBTYPE_ARM_XSCALE:
		    printf("     XSCALE");
		    break;
		case CPU_SUBTYPE_ARM_V6:
		    printf("         V6");
		    break;
		case CPU_SUBTYPE_ARM_V6M:
		    printf("        V6M");
		    break;
		case CPU_SUBTYPE_ARM_V7:
		    printf("         V7");
		    break;
		case CPU_SUBTYPE_ARM_V7F:
		    printf("        V7F");
		    break;
		case CPU_SUBTYPE_ARM_V7S:
		    printf("        V7S");
		    break;
		case CPU_SUBTYPE_ARM_V7K:
		    printf("        V7K");
		    break;
		case CPU_SUBTYPE_ARM_V7M:
		    printf("        V7M");
		    break;
		case CPU_SUBTYPE_ARM_V7EM:
		    printf("       V7EM");
		    break;
		default:
		    printf(" %10d", cpusubtype & ~CPU_SUBTYPE_MASK);
		    break;
		}
		break;
	    case CPU_TYPE_ARM64:
		printf("   ARM64");
		switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_ARM64_ALL:
		    printf("        ALL");
		    break;
		case CPU_SUBTYPE_ARM64_V8:
		    printf("         V8");
		    break;
		case CPU_SUBTYPE_ARM64E:
		    printf("          E");
		    break;
		default:
		    printf(" %10d", cpusubtype & ~CPU_SUBTYPE_MASK);
		    break;
		}
		break;
	    case CPU_TYPE_ARM64_32:
		printf(" ARM64_32");
		switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_ARM64_32_V8:
		    printf("        V8");
		    break;
		default:
		    printf(" %10d", cpusubtype & ~CPU_SUBTYPE_MASK);
		    break;
		}
		break;
	    default:
		printf(" %7d %10d", cputype, cpusubtype & ~CPU_SUBTYPE_MASK);
		break;
	    }
	    if((cpusubtype & CPU_SUBTYPE_MASK) == CPU_SUBTYPE_LIB64){
		printf(" LIB64 ");
	    }
	    else{
		printf("  0x%02x ", (unsigned int)
		       ((cpusubtype & ~CPU_SUBTYPE_MASK) >> 24));
	    }
	    switch(filetype){
	    case MH_OBJECT:
		printf("     OBJECT");
		break;
	    case MH_EXECUTE:
		printf("    EXECUTE");
		break;
	    case MH_FVMLIB:
		printf("     FVMLIB");
		break;
	    case MH_CORE:
		printf("       CORE");
		break;
	    case MH_PRELOAD:
		printf("    PRELOAD");
		break;
	    case MH_DYLIB:
		printf("      DYLIB");
		break;
	    case MH_DYLIB_STUB:
		printf(" DYLIB_STUB");
		break;
	    case MH_DYLINKER:
		printf("   DYLINKER");
		break;
	    case MH_BUNDLE:
		printf("     BUNDLE");
		break;
	    case MH_DSYM:
		printf("       DSYM");
		break;
	    case MH_KEXT_BUNDLE:
		printf(" KEXTBUNDLE");
		break;
	    default:
		printf(" %10u", filetype);
		break;
	    }
	    printf(" %5u %10u", ncmds, sizeofcmds);
	    f = flags;
	    if(f & MH_NOUNDEFS){
		printf("   NOUNDEFS");
		f &= ~MH_NOUNDEFS;
	    }
	    if(f & MH_INCRLINK){
		printf(" INCRLINK");
		f &= ~MH_INCRLINK;
	    }
	    if(f & MH_DYLDLINK){
		printf(" DYLDLINK");
		f &= ~MH_DYLDLINK;
	    }
	    if(f & MH_BINDATLOAD){
		printf(" BINDATLOAD");
		f &= ~MH_BINDATLOAD;
	    }
	    if(f & MH_PREBOUND){
		printf(" PREBOUND");
		f &= ~MH_PREBOUND;
	    }
	    if(f & MH_SPLIT_SEGS){
		printf(" SPLIT_SEGS");
		f &= ~MH_SPLIT_SEGS;
	    }
	    if(f & MH_LAZY_INIT){
		printf(" LAZY_INIT");
		f &= ~MH_LAZY_INIT;
	    }
	    if(f & MH_TWOLEVEL){
		printf(" TWOLEVEL");
		f &= ~MH_TWOLEVEL;
	    }
	    if(f & MH_FORCE_FLAT){
		printf(" FORCE_FLAT");
		f &= ~MH_FORCE_FLAT;
	    }
	    if(f & MH_NOMULTIDEFS){
		printf(" NOMULTIDEFS");
		f &= ~MH_NOMULTIDEFS;
	    }
	    if(f & MH_NOFIXPREBINDING){
		printf(" NOFIXPREBINDING");
		f &= ~MH_NOFIXPREBINDING;
	    }
	    if(f & MH_PREBINDABLE){
		printf(" PREBINDABLE");
		f &= ~MH_PREBINDABLE;
	    }
	    if(f & MH_ALLMODSBOUND){
		printf(" ALLMODSBOUND");
		f &= ~MH_ALLMODSBOUND;
	    }
	    if(f & MH_SUBSECTIONS_VIA_SYMBOLS){
		printf(" SUBSECTIONS_VIA_SYMBOLS");
		f &= ~MH_SUBSECTIONS_VIA_SYMBOLS;
	    }
	    if(f & MH_CANONICAL){
		printf(" CANONICAL");
		f &= ~MH_CANONICAL;
	    }
	    if(f & MH_WEAK_DEFINES){
		printf(" WEAK_DEFINES");
		f &= ~MH_WEAK_DEFINES;
	    }
	    if(f & MH_BINDS_TO_WEAK){
		printf(" BINDS_TO_WEAK");
		f &= ~MH_BINDS_TO_WEAK;
	    }
	    if(f & MH_ALLOW_STACK_EXECUTION){
		printf(" ALLOW_STACK_EXECUTION");
		f &= ~MH_ALLOW_STACK_EXECUTION;
	    }
	    if(f & MH_ROOT_SAFE){
		printf(" ROOT_SAFE");
		f &= ~MH_ROOT_SAFE;
	    }
	    if(f & MH_SETUID_SAFE){
		printf(" SETUID_SAFE");
		f &= ~MH_SETUID_SAFE;
	    }
	    if(f & MH_NO_REEXPORTED_DYLIBS){
		printf(" NO_REEXPORTED_DYLIBS");
		f &= ~MH_NO_REEXPORTED_DYLIBS;
	    }
	    if(f & MH_PIE){
		printf(" PIE");
		f &= ~MH_PIE;
	    }
	    if(f & MH_DEAD_STRIPPABLE_DYLIB){
		printf(" DEAD_STRIPPABLE_DYLIB");
		f &= ~MH_DEAD_STRIPPABLE_DYLIB;
	    }
	    if(f & MH_HAS_TLV_DESCRIPTORS){
		printf(" MH_HAS_TLV_DESCRIPTORS");
		f &= ~MH_HAS_TLV_DESCRIPTORS;
	    }
	    if(f & MH_NO_HEAP_EXECUTION){
		printf(" MH_NO_HEAP_EXECUTION");
		f &= ~MH_NO_HEAP_EXECUTION;
	    }
	    if(f & MH_APP_EXTENSION_SAFE){
		printf(" APP_EXTENSION_SAFE");
		f &= ~MH_APP_EXTENSION_SAFE;
	    }
	    if(f & MH_NLIST_OUTOFSYNC_WITH_DYLDINFO){
		printf(" NLIST_OUTOFSYNC_WITH_DYLDINFO");
		f &= ~MH_NLIST_OUTOFSYNC_WITH_DYLDINFO;
	    }
	    if(f & MH_SIM_SUPPORT){
		printf(" SIM_SUPPORT");
		f &= ~MH_SIM_SUPPORT;
	    }
	    if(f != 0 || flags == 0)
		printf(" 0x%08x", (unsigned int)f);
	    printf("\n");
	}
	else{
	    printf(" 0x%08x %7d %10d  0x%02x  %10u %5u %10u 0x%08x\n",
		   (unsigned int)magic, cputype, cpusubtype & ~CPU_SUBTYPE_MASK,
		   (unsigned int)((cpusubtype & CPU_SUBTYPE_MASK) >> 24),
		   filetype, ncmds, sizeofcmds,
		   (unsigned int)flags);
	}
}

/*
 * Print the load commands. The load commands pointed to by load_commands can
 * have any alignment, are in the specified byte_sex, and must be at least
 * sizeofcmds in length.
 */
void
print_loadcmds(
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
cpu_type_t cputype,
uint32_t filetype,
enum byte_sex load_commands_byte_sex,
uint64_t object_size,
enum bool verbose,
enum bool very_verbose)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    uint32_t i, j, k, left, size, *unknown, nsyms;
    char *p, *begin, *end;
    struct load_command *lc, l;
    struct segment_command sg;
    struct section s;
    struct segment_command_64 sg64;
    struct section_64 s64;
    struct symtab_command st;
    struct dysymtab_command dyst;
    struct symseg_command ss;
    struct fvmlib_command fl;
    struct dylib_command dl;
    struct prebound_dylib_command pbdylib;
    struct sub_framework_command sub;
    struct sub_umbrella_command usub;
    struct sub_library_command lsub;
    struct sub_client_command csub;
    struct fvmfile_command ff;
    struct dylinker_command dyld;
    struct routines_command rc;
    struct routines_command_64 rc64;
    struct twolevel_hints_command hints;
    struct prebind_cksum_command cs;
    struct uuid_command uuid;
    struct linkedit_data_command ld;
    struct rpath_command rpath;
    struct encryption_info_command encrypt;
    struct encryption_info_command_64 encrypt64;
    struct linker_option_command lo;
    struct dyld_info_command dyld_info;
    struct version_min_command vd;
    struct build_version_command bv;
    struct build_tool_version btv;
    struct entry_point_command ep;
    struct source_version_command sv;
    struct note_command nc;
    uint64_t big_load_end;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != load_commands_byte_sex;

	nsyms = UINT_MAX;
	lc = load_commands;
	big_load_end = 0;
	for(i = 0 ; i < ncmds; i++){
	    printf("Load command %u\n", i);

	    memcpy((char *)&l, (char *)lc, sizeof(struct load_command));
	    if(swapped)
		swap_load_command(&l, host_byte_sex);
	    if(l.cmdsize % sizeof(int32_t) != 0)
		printf("load command %u size not a multiple of "
		       "sizeof(int32_t)\n", i);
	    big_load_end += l.cmdsize;
	    if(big_load_end > sizeofcmds)
		printf("load command %u extends past end of load commands\n",
		       i);
	    left = sizeofcmds - (uint32_t)((char *)lc - (char *)load_commands);

	    switch(l.cmd){
	    case LC_SEGMENT:
		memset((char *)&sg, '\0', sizeof(struct segment_command));
		size = left < sizeof(struct segment_command) ?
		       left : sizeof(struct segment_command);
		memcpy((char *)&sg, (char *)lc, size);
		if(swapped)
		    swap_segment_command(&sg, host_byte_sex);
		print_segment_command(sg.cmd, sg.cmdsize, sg.segname,
		    sg.vmaddr, sg.vmsize, sg.fileoff, sg.filesize,
		    sg.maxprot, sg.initprot, sg.nsects, sg.flags,
		    object_size, verbose);
		p = (char *)lc + sizeof(struct segment_command);
		for(j = 0 ; j < sg.nsects ; j++){
		    if(p + sizeof(struct section) >
		       (char *)load_commands + sizeofcmds){
			printf("section structure command extends past end of "
			       "load commands\n");
		    }
		    left = sizeofcmds - (uint32_t)(p - (char *)load_commands);
		    memset((char *)&s, '\0', sizeof(struct section));
		    size = left < sizeof(struct section) ?
			   left : sizeof(struct section);
		    memcpy((char *)&s, p, size);
		    if(swapped)
			swap_section(&s, 1, host_byte_sex);
		    print_section(s.sectname, s.segname, s.addr, s.size,
			s.offset, s.align, s.reloff, s.nreloc, s.flags,
			s.reserved1, s.reserved2, sg.cmd, sg.segname,
			filetype, object_size, verbose);
		    if(p + sizeof(struct section) >
		       (char *)load_commands + sizeofcmds)
			return;
		    p += size;
		}
		break;

	    case LC_SEGMENT_64:
		memset((char *)&sg64, '\0', sizeof(struct segment_command_64));
		size = left < sizeof(struct segment_command_64) ?
		       left : sizeof(struct segment_command_64);
		memcpy((char *)&sg64, (char *)lc, size);
		if(swapped)
		    swap_segment_command_64(&sg64, host_byte_sex);
		print_segment_command(sg64.cmd, sg64.cmdsize, sg64.segname,
		    sg64.vmaddr, sg64.vmsize, sg64.fileoff, sg64.filesize,
		    sg64.maxprot, sg64.initprot, sg64.nsects, sg64.flags,
		    object_size, verbose);
		p = (char *)lc + sizeof(struct segment_command_64);
		for(j = 0 ; j < sg64.nsects ; j++){
		    if(p + sizeof(struct section_64) >
		       (char *)load_commands + sizeofcmds){
			printf("section structure command extends past end of "
			       "load commands\n");
		    }
		    left = sizeofcmds - (uint32_t)(p - (char *)load_commands);
		    memset((char *)&s64, '\0', sizeof(struct section_64));
		    size = left < sizeof(struct section_64) ?
			   left : sizeof(struct section_64);
		    memcpy((char *)&s64, p, size);
		    if(swapped)
			swap_section_64(&s64, 1, host_byte_sex);
		    print_section(s64.sectname, s64.segname, s64.addr,
			s64.size, s64.offset, s64.align, s64.reloff,
			s64.nreloc, s64.flags, s64.reserved1, s64.reserved2,
			sg64.cmd, sg64.segname, filetype, object_size,
			verbose);
		    if(p + sizeof(struct section_64) >
		       (char *)load_commands + sizeofcmds)
			return;
		    p += size;
		}
		break;

	    case LC_SYMTAB:
		memset((char *)&st, '\0', sizeof(struct symtab_command));
		size = left < sizeof(struct symtab_command) ?
		       left : sizeof(struct symtab_command);
		memcpy((char *)&st, (char *)lc, size);
		if(swapped)
		    swap_symtab_command(&st, host_byte_sex);
		nsyms = st.nsyms;
		print_symtab_command(&st, cputype, object_size);
		break;

	    case LC_DYSYMTAB:
		memset((char *)&dyst, '\0', sizeof(struct dysymtab_command));
		size = left < sizeof(struct dysymtab_command) ?
		       left : sizeof(struct dysymtab_command);
		memcpy((char *)&dyst, (char *)lc, size);
		if(swapped)
		    swap_dysymtab_command(&dyst, host_byte_sex);
		print_dysymtab_command(&dyst, nsyms, object_size, cputype);
		break;

	    case LC_SYMSEG:
		memset((char *)&ss, '\0', sizeof(struct symseg_command));
		size = left < sizeof(struct symseg_command) ?
		       left : sizeof(struct symseg_command);
		memcpy((char *)&ss, (char *)lc, size);
		if(swapped)
		    swap_symseg_command(&ss, host_byte_sex);
		print_symseg_command(&ss, object_size);
		break;

	    case LC_IDFVMLIB:
	    case LC_LOADFVMLIB:
		memset((char *)&fl, '\0', sizeof(struct fvmlib_command));
		size = left < sizeof(struct fvmlib_command) ?
		       left : sizeof(struct fvmlib_command);
		memcpy((char *)&fl, (char *)lc, size);
		if(swapped)
		    swap_fvmlib_command(&fl, host_byte_sex);
		print_fvmlib_command(&fl, lc, left);
		break;

	    case LC_ID_DYLIB:
	    case LC_LOAD_DYLIB:
	    case LC_LOAD_WEAK_DYLIB:
	    case LC_REEXPORT_DYLIB:
	    case LC_LOAD_UPWARD_DYLIB:
	    case LC_LAZY_LOAD_DYLIB:
		memset((char *)&dl, '\0', sizeof(struct dylib_command));
		size = left < sizeof(struct dylib_command) ?
		       left : sizeof(struct dylib_command);
		memcpy((char *)&dl, (char *)lc, size);
		if(swapped)
		    swap_dylib_command(&dl, host_byte_sex);
		print_dylib_command(&dl, lc, left);
		break;

	    case LC_SUB_FRAMEWORK:
		memset((char *)&sub, '\0',sizeof(struct sub_framework_command));
		size = left < sizeof(struct sub_framework_command) ?
		       left : sizeof(struct sub_framework_command);
		memcpy((char *)&sub, (char *)lc, size);
		if(swapped)
		    swap_sub_framework_command(&sub, host_byte_sex);
		print_sub_framework_command(&sub, lc, left);
		break;

	    case LC_SUB_UMBRELLA:
		memset((char *)&usub, '\0',sizeof(struct sub_umbrella_command));
		size = left < sizeof(struct sub_umbrella_command) ?
		       left : sizeof(struct sub_umbrella_command);
		memcpy((char *)&usub, (char *)lc, size);
		if(swapped)
		    swap_sub_umbrella_command(&usub, host_byte_sex);
		print_sub_umbrella_command(&usub, lc, left);
		break;

	    case LC_SUB_LIBRARY:
		memset((char *)&lsub, '\0',sizeof(struct sub_library_command));
		size = left < sizeof(struct sub_library_command) ?
		       left : sizeof(struct sub_library_command);
		memcpy((char *)&lsub, (char *)lc, size);
		if(swapped)
		    swap_sub_library_command(&lsub, host_byte_sex);
		print_sub_library_command(&lsub, lc, left);
		break;

	    case LC_SUB_CLIENT:
		memset((char *)&csub, '\0',sizeof(struct sub_client_command));
		size = left < sizeof(struct sub_client_command) ?
		       left : sizeof(struct sub_client_command);
		memcpy((char *)&csub, (char *)lc, size);
		if(swapped)
		    swap_sub_client_command(&csub, host_byte_sex);
		print_sub_client_command(&csub, lc, left);
		break;

	    case LC_PREBOUND_DYLIB:
		memset((char *)&pbdylib, '\0',
			sizeof(struct prebound_dylib_command));
		size = left < sizeof(struct prebound_dylib_command) ?
		       left : sizeof(struct prebound_dylib_command);
		memcpy((char *)&pbdylib, (char *)lc, size);
		if(swapped)
		    swap_prebound_dylib_command(&pbdylib, host_byte_sex);
		print_prebound_dylib_command(&pbdylib, lc, left, very_verbose);
		break;

	    case LC_ID_DYLINKER:
	    case LC_LOAD_DYLINKER:
	    case LC_DYLD_ENVIRONMENT:
		memset((char *)&dyld, '\0', sizeof(struct dylinker_command));
		size = left < sizeof(struct dylinker_command) ?
		       left : sizeof(struct dylinker_command);
		memcpy((char *)&dyld, (char *)lc, size);
		if(swapped)
		    swap_dylinker_command(&dyld, host_byte_sex);
		print_dylinker_command(&dyld, lc, left);
		break;

	    case LC_FVMFILE:
		memset((char *)&ff, '\0', sizeof(struct fvmfile_command));
		size = left < sizeof(struct fvmfile_command) ?
		       left : sizeof(struct fvmfile_command);
		memcpy((char *)&ff, (char *)lc, size);
		if(swapped)
		    swap_fvmfile_command(&ff, host_byte_sex);
		print_fvmfile_command(&ff, lc, left);
		break;

	    case LC_UNIXTHREAD:
	    case LC_THREAD:
	        if(l.cmd == LC_UNIXTHREAD)
		    printf("        cmd LC_UNIXTHREAD\n");
		else
		    printf("        cmd LC_THREAD\n");
		printf("    cmdsize %u\n", l.cmdsize);

		if(left <= sizeof(struct thread_command))
		    break;
		begin = (char *)lc + sizeof(struct thread_command);
		if(left >= l.cmdsize)
		    end = (char *)lc + l.cmdsize;
		else
		    end = (char *)lc + left;
		print_thread_states(begin, end, cputype,
				    load_commands_byte_sex);
		break;

	    case LC_IDENT:
		printf("          cmd LC_IDENT\n");
		printf("      cmdsize %u", l.cmdsize);
		if(l.cmdsize < sizeof(struct ident_command))
		    printf(" Incorrect size\n");
		else
		    printf("\n");
		begin = (char *)lc + sizeof(struct ident_command);
		left -= sizeof(struct ident_command);
		if(left >= l.cmdsize)
		    end = (char *)lc + l.cmdsize;
		else
		    end = (char *)lc + left;
		if((end - (char *)load_commands) > sizeofcmds)
		    end = (char *)load_commands + sizeofcmds;

		p = ((char *)lc) + sizeof(struct ident_command);
		while(begin < end){
		    if(*begin == '\0'){
			begin++;
			continue;
		    }
		    for(j = 0; begin + j < end && begin[j] != '\0'; j++)
			;
		    printf(" ident string %.*s\n", (int)j, begin);
		    begin += j;
		}
		break;

	    case LC_ROUTINES:
		memset((char *)&rc, '\0', sizeof(struct routines_command));
		size = left < sizeof(struct routines_command) ?
		       left : sizeof(struct routines_command);
		memcpy((char *)&rc, (char *)lc, size);
		if(swapped)
		    swap_routines_command(&rc, host_byte_sex);
		print_routines_command(&rc);
		break;

	    case LC_ROUTINES_64:
		memset((char *)&rc64, '\0', sizeof(struct routines_command_64));
		size = left < sizeof(struct routines_command_64) ?
		       left : sizeof(struct routines_command_64);
		memcpy((char *)&rc64, (char *)lc, size);
		if(swapped)
		    swap_routines_command_64(&rc64, host_byte_sex);
		print_routines_command_64(&rc64);
		break;

	    case LC_TWOLEVEL_HINTS:
		memset((char *)&hints, '\0',
		       sizeof(struct twolevel_hints_command));
		size = left < sizeof(struct twolevel_hints_command) ?
		       left : sizeof(struct twolevel_hints_command);
		memcpy((char *)&hints, (char *)lc, size);
		if(swapped)
		    swap_twolevel_hints_command(&hints, host_byte_sex);
		print_twolevel_hints_command(&hints, object_size);
		break;

	    case LC_PREBIND_CKSUM:
		memset((char *)&cs, '\0', sizeof(struct prebind_cksum_command));
		size = left < sizeof(struct prebind_cksum_command) ?
		       left : sizeof(struct prebind_cksum_command);
		memcpy((char *)&cs, (char *)lc, size);
		if(swapped)
		    swap_prebind_cksum_command(&cs, host_byte_sex);
		print_prebind_cksum_command(&cs);
		break;

	    case LC_UUID:
		memset((char *)&uuid, '\0', sizeof(struct uuid_command));
		size = left < sizeof(struct uuid_command) ?
		       left : sizeof(struct uuid_command);
		memcpy((char *)&uuid, (char *)lc, size);
		if(swapped)
		    swap_uuid_command(&uuid, host_byte_sex);
		print_uuid_command(&uuid);
		break;

	    case LC_CODE_SIGNATURE:
	    case LC_SEGMENT_SPLIT_INFO:
	    case LC_FUNCTION_STARTS:
	    case LC_DATA_IN_CODE:
	    case LC_DYLIB_CODE_SIGN_DRS:
	    case LC_LINKER_OPTIMIZATION_HINT:
	    case LC_DYLD_EXPORTS_TRIE:
	    case LC_DYLD_CHAINED_FIXUPS:
		memset((char *)&ld, '\0', sizeof(struct linkedit_data_command));
		size = left < sizeof(struct linkedit_data_command) ?
		       left : sizeof(struct linkedit_data_command);
		memcpy((char *)&ld, (char *)lc, size);
		if(swapped)
		    swap_linkedit_data_command(&ld, host_byte_sex);
		print_linkedit_data_command(&ld, object_size);
		break;

	    case LC_RPATH:
		memset((char *)&rpath, '\0', sizeof(struct rpath_command));
		size = left < sizeof(struct rpath_command) ?
		       left : sizeof(struct rpath_command);
		memcpy((char *)&rpath, (char *)lc, size);
		if(swapped)
		    swap_rpath_command(&rpath, host_byte_sex);
		print_rpath_command(&rpath, lc);
		break;

	    case LC_ENCRYPTION_INFO:
		memset((char *)&encrypt, '\0',
		       sizeof(struct encryption_info_command));
		size = left < sizeof(struct encryption_info_command) ?
		       left : sizeof(struct encryption_info_command);
		memcpy((char *)&encrypt, (char *)lc, size);
		if(swapped)
		    swap_encryption_command(&encrypt, host_byte_sex);
		print_encryption_info_command(&encrypt, object_size);
		break;

	    case LC_ENCRYPTION_INFO_64:
		memset((char *)&encrypt64, '\0',
		       sizeof(struct encryption_info_command_64));
		size = left < sizeof(struct encryption_info_command_64) ?
		       left : sizeof(struct encryption_info_command_64);
		memcpy((char *)&encrypt64, (char *)lc, size);
		if(swapped)
		    swap_encryption_command_64(&encrypt64, host_byte_sex);
		print_encryption_info_command_64(&encrypt64, object_size);
		break;

	    case LC_LINKER_OPTION:
		memset((char *)&lo, '\0',
		       sizeof(struct linker_option_command));
		size = left < sizeof(struct linker_option_command) ?
		       left : sizeof(struct linker_option_command);
		memcpy((char *)&lo, (char *)lc, size);
		if(swapped)
		    swap_linker_option_command(&lo, host_byte_sex);
		print_linker_option_command(&lo, lc, left);
		break;

	    case LC_DYLD_INFO:
	    case LC_DYLD_INFO_ONLY:
		memset((char *)&dyld_info, '\0',
		       sizeof(struct dyld_info_command));
		size = left < sizeof(struct dyld_info_command) ?
		       left : sizeof(struct dyld_info_command);
		memcpy((char *)&dyld_info, (char *)lc, size);
		if(swapped)
		    swap_dyld_info_command(&dyld_info, host_byte_sex);
		print_dyld_info_info_command(&dyld_info, object_size);
		break;

	    case LC_VERSION_MIN_MACOSX:
	    case LC_VERSION_MIN_IPHONEOS:
	    case LC_VERSION_MIN_WATCHOS:
	    case LC_VERSION_MIN_TVOS:
		memset((char *)&vd, '\0', sizeof(struct version_min_command));
		size = left < sizeof(struct version_min_command) ?
		       left : sizeof(struct version_min_command);
		memcpy((char *)&vd, (char *)lc, size);
		if(swapped)
		    swap_version_min_command(&vd, host_byte_sex);
		print_version_min_command(&vd);
		break;

	    case LC_BUILD_VERSION:
		memset((char *)&bv, '\0', sizeof(struct build_version_command));
		size = left < sizeof(struct build_version_command) ?
		       left : sizeof(struct build_version_command);
		memcpy((char *)&bv, (char *)lc, size);
		if(swapped)
		    swap_build_version_command(&bv, host_byte_sex);
		print_build_version_command(&bv, verbose);
		p = (char *)lc + sizeof(struct build_version_command);
		for(j = 0 ; j < bv.ntools ; j++){
		    if(p + sizeof(struct build_tool_version) >
		       (char *)load_commands + sizeofcmds){
			printf("build_tool_version structure command extends "
			       "past end of load commands\n");
		    }
		    left = sizeofcmds - (uint32_t)(p - (char *)load_commands);
		    memset((char *)&s, '\0', sizeof(struct build_tool_version));
		    size = left < sizeof(struct build_tool_version) ?
			   left : sizeof(struct build_tool_version);
		    memcpy((char *)&btv, p, size);
		    if(swapped)
			swap_build_tool_version(&btv, 1, host_byte_sex);
		    print_build_tool_version(btv.tool, btv.version, verbose);
		    if(p + sizeof(struct build_tool_version) >
		       (char *)load_commands + sizeofcmds)
			return;
		    p += size;
		}
		break;

	    case LC_SOURCE_VERSION:
		memset((char *)&sv, '\0',sizeof(struct source_version_command));
		size = left < sizeof(struct source_version_command) ?
		       left : sizeof(struct source_version_command);
		memcpy((char *)&sv, (char *)lc, size);
		if(swapped)
		    swap_source_version_command(&sv, host_byte_sex);
		print_source_version_command(&sv);
		break;

	    case LC_NOTE:
		memset((char *)&nc, '\0',sizeof(struct note_command));
		size = left < sizeof(struct note_command) ?
		       left : sizeof(struct note_command);
		memcpy((char *)&nc, (char *)lc, size);
		if(swapped)
		    swap_note_command(&nc, host_byte_sex);
		print_note_command(&nc, object_size);
		break;

	    case LC_MAIN:
		memset((char *)&ep, '\0', sizeof(struct entry_point_command));
		size = left < sizeof(struct entry_point_command) ?
		       left : sizeof(struct entry_point_command);
		memcpy((char *)&ep, (char *)lc, size);
		if(swapped)
		    swap_entry_point_command(&ep, host_byte_sex);
		print_entry_point_command(&ep);
		break;

	    default:
		printf("      cmd ?(0x%08x) Unknown load command\n",
		       (unsigned int)l.cmd);
		printf("  cmdsize %u\n", l.cmdsize);
		if(left < sizeof(struct load_command))
		    return;
		left -= sizeof(struct load_command);
		size = left < l.cmdsize - sizeof(struct load_command) ?
		       left : l.cmdsize - sizeof(struct load_command);
		unknown = allocate(size);
		memcpy((char *)unknown,
		       ((char *)lc) + sizeof(struct load_command), size);
		if(swapped)
		    for(j = 0; j < size / sizeof(uint32_t); j++)
			unknown[j] = SWAP_INT(unknown[j]);
		for(j = 0; j < size / sizeof(uint32_t); j += k){
		    for(k = 0;
			k < 8 && j + k < size / sizeof(uint32_t);
			k++)
			printf("%08x ", (unsigned int)unknown[j + k]);
		    printf("\n");
		}
		break;
	    }
	    if(l.cmdsize == 0){
		printf("load command %u size zero (can't advance to other "
		       "load commands)\n", i);
		return;
	    }
	    lc = (struct load_command *)((char *)lc + l.cmdsize);
	    if((char *)lc > (char *)load_commands + sizeofcmds)
		return;
	}
	if((char *)load_commands + sizeofcmds != (char *)lc)
	    printf("Inconsistent sizeofcmds\n");
}

void
print_libraries(
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex load_commands_byte_sex,
enum bool just_id,
enum bool verbose)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    uint32_t i, left, size;
    struct load_command *lc, l;
    struct fvmlib_command fl;
    struct dylib_command dl;
    char *p;
    time_t timestamp;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != load_commands_byte_sex;

	lc = load_commands;
	for(i = 0 ; i < ncmds; i++){
	    memcpy((char *)&l, (char *)lc, sizeof(struct load_command));
	    if(swapped)
		swap_load_command(&l, host_byte_sex);
	    if(l.cmdsize % sizeof(int32_t) != 0)
		printf("load command %u size not a multiple of "
		       "sizeof(int32_t)\n", i);
	    if((char *)lc + l.cmdsize > (char *)load_commands + sizeofcmds)
		printf("load command %u extends past end of load commands\n",
		       i);
	    left = sizeofcmds - (uint32_t)((char *)lc - (char *)load_commands);

	    switch(l.cmd){
	    case LC_IDFVMLIB:
	    case LC_LOADFVMLIB:
		if(just_id == TRUE)
		    break;
		memset((char *)&fl, '\0', sizeof(struct fvmlib_command));
		size = left < sizeof(struct fvmlib_command) ?
		       left : sizeof(struct fvmlib_command);
		memcpy((char *)&fl, (char *)lc, size);
		if(swapped)
		    swap_fvmlib_command(&fl, host_byte_sex);
		if(fl.fvmlib.name.offset < fl.cmdsize &&
		   fl.fvmlib.name.offset < left){
		    p = (char *)lc + fl.fvmlib.name.offset;
		    printf("\t%s (minor version %u)\n", p,
			   fl.fvmlib.minor_version);
		}
		else{
		    printf("\tBad offset (%u) for name of %s command %u\n",
			   fl.fvmlib.name.offset, l.cmd == LC_IDFVMLIB ?
			   "LC_IDFVMLIB" : "LC_LOADFVMLIB" , i);
		}
		break;

	    case LC_LOAD_DYLIB:
	    case LC_LOAD_WEAK_DYLIB:
	    case LC_REEXPORT_DYLIB:
	    case LC_LOAD_UPWARD_DYLIB:
	    case LC_LAZY_LOAD_DYLIB:
		if(just_id == TRUE)
		    break;
	    case LC_ID_DYLIB:
		memset((char *)&dl, '\0', sizeof(struct dylib_command));
		size = left < sizeof(struct dylib_command) ?
		       left : sizeof(struct dylib_command);
		memcpy((char *)&dl, (char *)lc, size);
		if(swapped)
		    swap_dylib_command(&dl, host_byte_sex);
		if(dl.dylib.name.offset < dl.cmdsize &&
		   dl.dylib.name.offset < left){
		    p = (char *)lc + dl.dylib.name.offset;
		    if(just_id == TRUE)
			printf("%s\n", p);
		    else
			printf("\t%s (compatibility version %u.%u.%u, "
			   "current version %u.%u.%u)\n", p,
			   dl.dylib.compatibility_version >> 16,
			   (dl.dylib.compatibility_version >> 8) & 0xff,
			   dl.dylib.compatibility_version & 0xff,
			   dl.dylib.current_version >> 16,
			   (dl.dylib.current_version >> 8) & 0xff,
			   dl.dylib.current_version & 0xff);
		    if(verbose){
			printf("\ttime stamp %u ", dl.dylib.timestamp);
			timestamp = (time_t)dl.dylib.timestamp;
			printf("%s", ctime(&timestamp));
		    }
		}
		else{
		    printf("\tBad offset (%u) for name of ",
			   dl.dylib.name.offset);
		    if(l.cmd == LC_ID_DYLIB)
			printf("LC_ID_DYLIB ");
		    else if(l.cmd == LC_LOAD_DYLIB)
			printf("LC_LOAD_DYLIB ");
		    else if(l.cmd == LC_LOAD_WEAK_DYLIB)
			printf("LC_LOAD_WEAK_DYLIB ");
		    else if(l.cmd == LC_LAZY_LOAD_DYLIB)
			printf("LC_LAZY_LOAD_DYLIB ");
		    else if(l.cmd == LC_REEXPORT_DYLIB)
			printf("LC_REEXPORT_DYLIB ");
		    else if(l.cmd == LC_LOAD_UPWARD_DYLIB)
			printf("LC_LOAD_UPWARD_DYLIB ");
		    else
			printf("LC_??? ");
		    printf("command %u\n", i);
		}
		break;
	    }
	    if(l.cmdsize == 0){
		printf("load command %u size zero (can't advance to other "
		       "load commands)\n", i);
		return;
	    }
	    lc = (struct load_command *)((char *)lc + l.cmdsize);
	    if((char *)lc > (char *)load_commands + sizeofcmds)
		return;
	}
	if((char *)load_commands + sizeofcmds != (char *)lc)
	    printf("Inconsistent sizeofcmds\n");
}

/*
 * print an LC_SEGMENT command.  The fields of the segment_command must
 * be in the host byte sex.
 */
void
print_segment_command(
uint32_t cmd,
uint32_t cmdsize,
char *segname,
uint64_t vmaddr,
uint64_t vmsize,
uint64_t fileoff,
uint64_t filesize,
vm_prot_t maxprot,
vm_prot_t initprot,
uint32_t nsects,
uint32_t flags,
uint64_t object_size,
enum bool verbose)
{
    uint64_t expected_cmdsize;

	if(cmd == LC_SEGMENT){
	    printf("      cmd LC_SEGMENT\n");
	    expected_cmdsize = nsects;
	    expected_cmdsize *= sizeof(struct section);
	    expected_cmdsize += sizeof(struct segment_command);
	}
	else{
	    printf("      cmd LC_SEGMENT_64\n");
	    expected_cmdsize = nsects;
	    expected_cmdsize *= sizeof(struct section_64);
	    expected_cmdsize += sizeof(struct segment_command_64);
	}
	printf("  cmdsize %u", cmdsize);
	if(cmdsize != expected_cmdsize)
	    printf(" Inconsistent size\n");
	else
	    printf("\n");
	printf("  segname %.16s\n", segname);
	if(cmd == LC_SEGMENT_64){
	    printf("   vmaddr 0x%016llx\n", vmaddr);
	    printf("   vmsize 0x%016llx\n", vmsize);
	}
	else{
	    printf("   vmaddr 0x%08x\n", (uint32_t)vmaddr);
	    printf("   vmsize 0x%08x\n", (uint32_t)vmsize);
	}
	printf("  fileoff %llu", fileoff);
	if(fileoff > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf(" filesize %llu", filesize);
	if(fileoff + filesize > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	if(verbose){
	    if((maxprot &
	      ~(VM_PROT_READ  | VM_PROT_WRITE  | VM_PROT_EXECUTE)) != 0)
		printf("  maxprot ?(0x%08x)\n", (unsigned int)maxprot);
	    else{
		if(maxprot & VM_PROT_READ)
		    printf("  maxprot r");
		else
		    printf("  maxprot -");
		if(maxprot & VM_PROT_WRITE)
		    printf("w");
		else
		    printf("-");
		if(maxprot & VM_PROT_EXECUTE)
		    printf("x\n");
		else
		    printf("-\n");
	    }
	    if((initprot &
	      ~(VM_PROT_READ  | VM_PROT_WRITE  | VM_PROT_EXECUTE)) != 0)
		printf(" initprot ?(0x%08x)\n", (unsigned int)initprot);
	    else{
		if(initprot & VM_PROT_READ)
		    printf(" initprot r");
		else
		    printf(" initprot -");
		if(initprot & VM_PROT_WRITE)
		    printf("w");
		else
		    printf("-");
		if(initprot & VM_PROT_EXECUTE)
		    printf("x\n");
		else
		    printf("-\n");
	    }
	}
	else{
	    printf("  maxprot 0x%08x\n", (unsigned int)maxprot);
	    printf(" initprot 0x%08x\n", (unsigned int)initprot);
	}
	printf("   nsects %u\n", nsects);
	if(verbose){
	    printf("    flags");
	    if(flags == 0)
		printf(" (none)\n");
	    else{
		if(flags & SG_HIGHVM){
		    printf(" HIGHVM");
		    flags &= ~SG_HIGHVM;
		}
		if(flags & SG_FVMLIB){
		    printf(" FVMLIB");
		    flags &= ~SG_FVMLIB;
		}
		if(flags & SG_NORELOC){
		    printf(" NORELOC");
		    flags &= ~SG_NORELOC;
		}
		if(flags & SG_PROTECTED_VERSION_1){
		    printf(" PROTECTED_VERSION_1");
		    flags &= ~SG_PROTECTED_VERSION_1;
		}
        if(flags & SG_READ_ONLY){
            printf(" SG_READ_ONLY");
            flags &= ~SG_READ_ONLY;
        }
		if(flags)
		    printf(" 0x%x (unknown flags)\n", (unsigned int)flags);
		else
		    printf("\n");
	    }
	}
	else{
	    printf("    flags 0x%x\n", (unsigned int)flags);
	}
}

/*
 * print a section structure.  All parameters must be in the host byte sex.
 */
void
print_section(
char *sectname,
char *segname,
uint64_t addr,
uint64_t size,
uint32_t offset,
uint32_t align,
uint32_t reloff,
uint32_t nreloc,
uint32_t flags,
uint32_t reserved1,
uint32_t reserved2,
uint32_t cmd,
char *sg_segname,
uint32_t filetype,
uint64_t object_size,
enum bool verbose)
{
    uint32_t section_type, section_attributes;

	printf("Section\n");
	printf("  sectname %.16s\n", sectname);
	printf("   segname %.16s", segname);
	if(filetype != MH_OBJECT &&
	   strcmp(sg_segname, segname) != 0)
	    printf(" (does not match segment)\n");
	else
	    printf("\n");
	if(cmd == LC_SEGMENT_64){
	    printf("      addr 0x%016llx\n", addr);
	    printf("      size 0x%016llx", size);
	}
	else{
	    printf("      addr 0x%08x\n", (uint32_t)addr);
	    printf("      size 0x%08x", (uint32_t)size);
	}
	if((flags & S_ZEROFILL) != S_ZEROFILL && offset + size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("    offset %u", offset);
	if(offset > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("     align 2^%u (%d)\n", align, 1 << align);
	printf("    reloff %u", reloff);
	if(reloff > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("    nreloc %u", nreloc);
	if(reloff + nreloc * sizeof(struct relocation_info) > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	section_type = flags & SECTION_TYPE;
	if(verbose){
	    printf("      type");
	    if(section_type == S_REGULAR)
		printf(" S_REGULAR\n");
	    else if(section_type == S_ZEROFILL)
		printf(" S_ZEROFILL\n");
	    else if(section_type == S_CSTRING_LITERALS)
		printf(" S_CSTRING_LITERALS\n");
	    else if(section_type == S_4BYTE_LITERALS)
		printf(" S_4BYTE_LITERALS\n");
	    else if(section_type == S_8BYTE_LITERALS)
		printf(" S_8BYTE_LITERALS\n");
	    else if(section_type == S_16BYTE_LITERALS)
		printf(" S_16BYTE_LITERALS\n");
	    else if(section_type == S_LITERAL_POINTERS)
		printf(" S_LITERAL_POINTERS\n");
	    else if(section_type == S_NON_LAZY_SYMBOL_POINTERS)
		printf(" S_NON_LAZY_SYMBOL_POINTERS\n");
	    else if(section_type == S_LAZY_SYMBOL_POINTERS)
		printf(" S_LAZY_SYMBOL_POINTERS\n");
	    else if(section_type == S_SYMBOL_STUBS)
		printf(" S_SYMBOL_STUBS\n");
	    else if(section_type == S_MOD_INIT_FUNC_POINTERS)
		printf(" S_MOD_INIT_FUNC_POINTERS\n");
	    else if(section_type == S_MOD_TERM_FUNC_POINTERS)
		printf(" S_MOD_TERM_FUNC_POINTERS\n");
	    else if(section_type == S_COALESCED)
		printf(" S_COALESCED\n");
	    else if(section_type == S_INTERPOSING)
		printf(" S_INTERPOSING\n");
	    else if(section_type == S_DTRACE_DOF)
		printf(" S_DTRACE_DOF\n");
	    else if(section_type == S_LAZY_DYLIB_SYMBOL_POINTERS)
		printf(" S_LAZY_DYLIB_SYMBOL_POINTERS\n");
	    else if(section_type == S_THREAD_LOCAL_REGULAR)
		printf(" S_THREAD_LOCAL_REGULAR\n");
	    else if(section_type == S_THREAD_LOCAL_ZEROFILL)
		printf(" S_THREAD_LOCAL_ZEROFILL\n");
	    else if(section_type == S_THREAD_LOCAL_VARIABLES)
		printf(" S_THREAD_LOCAL_VARIABLES\n");
	    else if(section_type == S_THREAD_LOCAL_VARIABLE_POINTERS)
		printf(" S_THREAD_LOCAL_VARIABLE_POINTERS\n");
	    else if(section_type == S_THREAD_LOCAL_INIT_FUNCTION_POINTERS)
		printf(" S_THREAD_LOCAL_INIT_FUNCTION_POINTERS\n");
	    else
		printf(" 0x%08x\n", (unsigned int)section_type);

	    printf("attributes");
	    section_attributes = flags & SECTION_ATTRIBUTES;
	    if(section_attributes & S_ATTR_PURE_INSTRUCTIONS)
		printf(" PURE_INSTRUCTIONS");
	    if(section_attributes & S_ATTR_NO_TOC)
		printf(" NO_TOC");
	    if(section_attributes & S_ATTR_STRIP_STATIC_SYMS)
		printf(" STRIP_STATIC_SYMS");
	    if(section_attributes & S_ATTR_NO_DEAD_STRIP)
		printf(" NO_DEAD_STRIP");
	    if(section_attributes & S_ATTR_LIVE_SUPPORT)
		printf(" LIVE_SUPPORT");
	    if(section_attributes & S_ATTR_SELF_MODIFYING_CODE)
		printf(" SELF_MODIFYING_CODE");
	    if(section_attributes & S_ATTR_DEBUG)
		printf(" DEBUG");
	    if(section_attributes & S_ATTR_SOME_INSTRUCTIONS)
		printf(" SOME_INSTRUCTIONS");
	    if(section_attributes & S_ATTR_EXT_RELOC)
		printf(" EXT_RELOC");
	    if(section_attributes & S_ATTR_LOC_RELOC)
		printf(" LOC_RELOC");
	    if(section_attributes == 0)
		printf(" (none)");
	    printf("\n");
	}
	else
	    printf("     flags 0x%08x\n", (unsigned int)flags);
	printf(" reserved1 %u", reserved1);
	if(section_type == S_SYMBOL_STUBS ||
	   section_type == S_LAZY_SYMBOL_POINTERS ||
	   section_type == S_LAZY_DYLIB_SYMBOL_POINTERS ||
	   section_type == S_NON_LAZY_SYMBOL_POINTERS ||
	   section_type == S_THREAD_LOCAL_VARIABLE_POINTERS)
	    printf(" (index into indirect symbol table)\n");
	else
	    printf("\n");
	printf(" reserved2 %u", reserved2);
	if(section_type == S_SYMBOL_STUBS)
	    printf(" (size of stubs)\n");
	else
	    printf("\n");
}

/*
 * print an LC_SYMTAB command.  The symtab_command structure specified must
 * be aligned correctly and in the host byte sex.
 */
void
print_symtab_command(
struct symtab_command *st,
cpu_type_t cputype,
uint64_t object_size)
{
    uint64_t big_size;

	printf("     cmd LC_SYMTAB\n");
	printf(" cmdsize %u", st->cmdsize);
	if(st->cmdsize != sizeof(struct symtab_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	printf("  symoff %u", st->symoff);
	if(st->symoff > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("   nsyms %u", st->nsyms);
	if(cputype & CPU_ARCH_ABI64){
	    big_size = st->nsyms;
	    big_size *= sizeof(struct nlist_64);
	    big_size += st->symoff;
	    if(big_size > object_size)
		printf(" (past end of file)\n");
	    else
		printf("\n");
	}
	else{
	    big_size = st->nsyms;
	    big_size *= sizeof(struct nlist);
	    big_size += st->symoff;
	    if(big_size > object_size)
		printf(" (past end of file)\n");
	    else
		printf("\n");
	}
	printf("  stroff %u", st->stroff);
	if(st->stroff > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf(" strsize %u", st->strsize);
	big_size = st->stroff;
	big_size += st->strsize;
	if(big_size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
}

/*
 * print an LC_DYSYMTAB command.  The dysymtab_command structure specified must
 * be aligned correctly and in the host byte sex.
 */
void
print_dysymtab_command(
struct dysymtab_command *dyst,
uint32_t nsyms,
uint64_t object_size,
cpu_type_t cputype)
{
    uint64_t modtabend, big_size;

	printf("            cmd LC_DYSYMTAB\n");
	printf("        cmdsize %u", dyst->cmdsize);
	if(dyst->cmdsize != sizeof(struct dysymtab_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");

	printf("      ilocalsym %u", dyst->ilocalsym);
	if(dyst->ilocalsym > nsyms)
	    printf(" (greater than the number of symbols)\n");
	else
	    printf("\n");
	printf("      nlocalsym %u", dyst->nlocalsym);
	big_size = dyst->ilocalsym;
	big_size += dyst->nlocalsym;
	if(big_size > nsyms)
	    printf(" (past the end of the symbol table)\n");
	else
	    printf("\n");
	printf("     iextdefsym %u", dyst->iextdefsym);
	if(dyst->iextdefsym > nsyms)
	    printf(" (greater than the number of symbols)\n");
	else
	    printf("\n");
	printf("     nextdefsym %u", dyst->nextdefsym);
	big_size = dyst->iextdefsym;
	big_size += dyst->nextdefsym;
	if(big_size > nsyms)
	    printf(" (past the end of the symbol table)\n");
	else
	    printf("\n");
	printf("      iundefsym %u", dyst->iundefsym);
	if(dyst->iundefsym > nsyms)
	    printf(" (greater than the number of symbols)\n");
	else
	    printf("\n");
	printf("      nundefsym %u", dyst->nundefsym);
	big_size = dyst->iundefsym;
	big_size += dyst->nundefsym;
	if(big_size > nsyms)
	    printf(" (past the end of the symbol table)\n");
	else
	    printf("\n");
	printf("         tocoff %u", dyst->tocoff);
	if(dyst->tocoff > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("           ntoc %u", dyst->ntoc);
	big_size = dyst->ntoc;
	big_size *= sizeof(struct dylib_table_of_contents);
	big_size += dyst->tocoff;
	if(big_size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("      modtaboff %u", dyst->modtaboff);
	if(dyst->modtaboff > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("        nmodtab %u", dyst->nmodtab);
	if(cputype & CPU_ARCH_ABI64){
	    modtabend = dyst->nmodtab;
	    modtabend *= sizeof(struct dylib_module_64);
	    modtabend += dyst->modtaboff;
	}
	else{
	    modtabend = dyst->nmodtab;
	    modtabend *= sizeof(struct dylib_module);
	    modtabend += dyst->modtaboff;
	}
	if(modtabend > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("   extrefsymoff %u", dyst->extrefsymoff);
	if(dyst->extrefsymoff > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("    nextrefsyms %u", dyst->nextrefsyms);
	big_size = dyst->nextrefsyms;
	big_size *= sizeof(struct dylib_reference);
	big_size += dyst->extrefsymoff;
	if(big_size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf(" indirectsymoff %u", dyst->indirectsymoff);
	if(dyst->indirectsymoff > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("  nindirectsyms %u", dyst->nindirectsyms);
	big_size = dyst->nindirectsyms;
	big_size *= sizeof(uint32_t);
	big_size += dyst->indirectsymoff;
	if(big_size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("      extreloff %u", dyst->extreloff);
	if(dyst->extreloff > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("        nextrel %u", dyst->nextrel);
	big_size = dyst->nextrel;
	big_size *= sizeof(struct relocation_info);
	big_size += dyst->extreloff;
	if(big_size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("      locreloff %u", dyst->locreloff);
	if(dyst->locreloff > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("        nlocrel %u", dyst->nlocrel);
	big_size = dyst->nlocrel;
	big_size *= sizeof(struct relocation_info);
	big_size += dyst->locreloff;
	if(big_size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
}

/*
 * print an LC_SYMSEG command.  The symseg_command structure specified must
 * be aligned correctly and in the host byte sex.
 */
void
print_symseg_command(
struct symseg_command *ss,
uint64_t object_size)
{
    uint64_t big_size;

	printf("     cmd LC_SYMSEG (obsolete)\n");
	printf(" cmdsize %u", ss->cmdsize);
	if(ss->cmdsize != sizeof(struct symseg_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	printf("  offset %u", ss->offset);
	if(ss->offset > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("    size %u", ss->size);
	big_size = ss->offset;
	big_size += ss->size;
	if(big_size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
}

/*
 * print an LC_IDFVMLIB or LC_LOADFVMLIB command.  The fvmlib_command structure
 * specified must be aligned correctly and in the host byte sex.
 */
void
print_fvmlib_command(
struct fvmlib_command *fl,
struct load_command *lc,
uint32_t left)
{
    char *p;

	if(fl->cmd == LC_IDFVMLIB)
	    printf("           cmd LC_IDFVMLIB\n");
	else
	    printf("           cmd LC_LOADFVMLIB\n");
	printf("       cmdsize %u", fl->cmdsize);
	if(fl->cmdsize < sizeof(struct fvmlib_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	if(fl->fvmlib.name.offset < fl->cmdsize &&
	   fl->fvmlib.name.offset < left){
	    p = (char *)lc + fl->fvmlib.name.offset;
	    printf("          name %s (offset %u)\n",
		   p, fl->fvmlib.name.offset);
	}
	else{
	    printf("          name ?(bad offset %u)\n",
		   fl->fvmlib.name.offset);
	}
	printf(" minor version %u\n", fl->fvmlib.minor_version);
	printf("   header addr 0x%08x\n", (unsigned int)fl->fvmlib.header_addr);
}

/*
 * print an LC_ID_DYLIB, LC_LOAD_DYLIB, LC_LOAD_WEAK_DYLIB, LC_REEXPORT_DYLIB,
 * LC_LAZY_LOAD_DYLIB, or LC_LOAD_UPWARD_DYLIB command.  The dylib_command
 * structure specified must be aligned correctly and in the host byte sex.
 */
void
print_dylib_command(
struct dylib_command *dl,
struct load_command *lc,
uint32_t left)
{
    char *p;
    time_t t;

	if(dl->cmd == LC_ID_DYLIB)
	    printf("          cmd LC_ID_DYLIB\n");
	else if(dl->cmd == LC_LOAD_DYLIB)
	    printf("          cmd LC_LOAD_DYLIB\n");
	else if(dl->cmd == LC_LOAD_WEAK_DYLIB)
	    printf("          cmd LC_LOAD_WEAK_DYLIB\n");
	else if(dl->cmd == LC_REEXPORT_DYLIB)
	    printf("          cmd LC_REEXPORT_DYLIB\n");
	else if(dl->cmd == LC_LAZY_LOAD_DYLIB)
	    printf("          cmd LC_LAZY_LOAD_DYLIB\n");
	else if(dl->cmd == LC_LOAD_UPWARD_DYLIB)
	    printf("          cmd LC_LOAD_UPWARD_DYLIB\n");
	else
	    printf("          cmd %u (unknown)\n", dl->cmd);
	printf("      cmdsize %u", dl->cmdsize);
	if(dl->cmdsize < sizeof(struct dylib_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	if(dl->dylib.name.offset < dl->cmdsize &&
           dl->dylib.name.offset < left){
	    p = (char *)lc + dl->dylib.name.offset;
	    printf("         name %.*s (offset %u)\n",
		   left, p, dl->dylib.name.offset);
	}
	else{
	    printf("         name ?(bad offset %u)\n",
		   dl->dylib.name.offset);
	}
	printf("   time stamp %u ", dl->dylib.timestamp);
	t = dl->dylib.timestamp;
	printf("%s", ctime(&t));
	printf("      current version ");
	if(dl->dylib.current_version == 0xffffffff)
	    printf("n/a\n");
	else
	    printf("%u.%u.%u\n",
		   dl->dylib.current_version >> 16,
		   (dl->dylib.current_version >> 8) & 0xff,
		   dl->dylib.current_version & 0xff);
	printf("compatibility version ");
	if(dl->dylib.compatibility_version == 0xffffffff)
	    printf("n/a\n");
	else
	    printf("%u.%u.%u\n",
		   dl->dylib.compatibility_version >> 16,
		   (dl->dylib.compatibility_version >> 8) & 0xff,
		   dl->dylib.compatibility_version & 0xff);
}

/*
 * print an LC_SUB_FRAMEWORK command.  The sub_framework_command
 * structure specified must be aligned correctly and in the host byte sex.
 */
void
print_sub_framework_command(
struct sub_framework_command *sub,
struct load_command *lc,
uint32_t left)
{
    char *p;

	printf("          cmd LC_SUB_FRAMEWORK\n");
	printf("      cmdsize %u", sub->cmdsize);
	if(sub->cmdsize < sizeof(struct sub_framework_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	if(sub->umbrella.offset < sub->cmdsize &&
	   sub->umbrella.offset < left){
	    p = (char *)lc + sub->umbrella.offset;
	    printf("     umbrella %s (offset %u)\n",
		   p, sub->umbrella.offset);
	}
	else{
	    printf("     umbrella ?(bad offset %u)\n",
		   sub->umbrella.offset);
	}
}

/*
 * print an LC_SUB_UMBRELLA command.  The sub_umbrella_command
 * structure specified must be aligned correctly and in the host byte sex.
 */
void
print_sub_umbrella_command(
struct sub_umbrella_command *usub,
struct load_command *lc,
uint32_t left)
{
    char *p;

	printf("          cmd LC_SUB_UMBRELLA\n");
	printf("      cmdsize %u", usub->cmdsize);
	if(usub->cmdsize < sizeof(struct sub_umbrella_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	if(usub->sub_umbrella.offset < usub->cmdsize &&
	   usub->sub_umbrella.offset < left){
	    p = (char *)lc + usub->sub_umbrella.offset;
	    printf(" sub_umbrella %s (offset %u)\n",
		   p, usub->sub_umbrella.offset);
	}
	else{
	    printf(" sub_umbrella ?(bad offset %u)\n",
		   usub->sub_umbrella.offset);
	}
}

/*
 * print an LC_SUB_LIBRARY command.  The sub_library_command
 * structure specified must be aligned correctly and in the host byte sex.
 */
void
print_sub_library_command(
struct sub_library_command *lsub,
struct load_command *lc,
uint32_t left)
{
    char *p;

	printf("          cmd LC_SUB_LIBRARY\n");
	printf("      cmdsize %u", lsub->cmdsize);
	if(lsub->cmdsize < sizeof(struct sub_library_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	if(lsub->sub_library.offset < lsub->cmdsize &&
	   lsub->sub_library.offset < left){
	    p = (char *)lc + lsub->sub_library.offset;
	    printf("  sub_library %s (offset %u)\n",
		   p, lsub->sub_library.offset);
	}
	else{
	    printf("  sub_library ?(bad offset %u)\n",
		   lsub->sub_library.offset);
	}
}

/*
 * print an LC_SUB_CLIENT command.  The sub_client_command
 * structure specified must be aligned correctly and in the host byte sex.
 */
void
print_sub_client_command(
struct sub_client_command *csub,
struct load_command *lc,
uint32_t left)
{
    char *p;

	printf("          cmd LC_SUB_CLIENT\n");
	printf("      cmdsize %u", csub->cmdsize);
	if(csub->cmdsize < sizeof(struct sub_client_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	if(csub->client.offset < csub->cmdsize &&
	   csub->client.offset < left){
	    p = (char *)lc + csub->client.offset;
	    printf("       client %s (offset %u)\n",
		   p, csub->client.offset);
	}
	else{
	    printf("       client ?(bad offset %u)\n",
		   csub->client.offset);
	}
}

/*
 * print an LC_PREBOUND_DYLIB command.  The prebound_dylib_command structure
 * specified must be aligned correctly and in the host byte sex.
 */
void
print_prebound_dylib_command(
struct prebound_dylib_command *pbdylib,
struct load_command *lc,
uint32_t left,
enum bool verbose)
{
    char *p;
    uint32_t i;

	printf("            cmd LC_PREBOUND_DYLIB\n");
	printf("        cmdsize %u", pbdylib->cmdsize);
	if(pbdylib->cmdsize < sizeof(struct prebound_dylib_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	if(pbdylib->name.offset < pbdylib->cmdsize &&
           pbdylib->name.offset < left){
	    p = (char *)lc + pbdylib->name.offset;
	    printf("           name %s (offset %u)\n",
		   p, pbdylib->name.offset);
	}
	else{
	    printf("           name ?(bad offset %u)\n",
		   pbdylib->name.offset);
	}
	printf("       nmodules %u\n", pbdylib->nmodules);

	if(pbdylib->linked_modules.offset < pbdylib->cmdsize &&
	   pbdylib->linked_modules.offset < left){
	    p = (char *)lc + pbdylib->linked_modules.offset;
	    if(verbose == TRUE){
		printf(" linked_modules (offset %u)\n",
			pbdylib->linked_modules.offset);
		for(i = 0; i < pbdylib->nmodules &&
			   pbdylib->linked_modules.offset + i/8 < left; i++){
		    if(((p[i/8] >> (i%8)) & 1) == 1)
			printf("%u\n", i);
		}
	    }
	    else{
		printf(" linked_modules ");
		for(i = 0; i < pbdylib->nmodules && i < 8; i++){
		    if(((*p >> i) & 1) == 0)
			printf("0");
		    else
			printf("1");
		}
		if(i <= pbdylib->nmodules)
		    printf("...");
		printf(" (offset %u)\n", pbdylib->linked_modules.offset);
	    }
	}
	else{
	    printf(" linked_modules ?(bad offset %u)\n",
		   pbdylib->linked_modules.offset);
	}
}

/*
 * print an LC_ID_DYLINKER, LC_LOAD_DYLINKER or LC_DYLD_ENVIRONMENT command.
 * The dylinker_command structure specified must be aligned correctly and in the
 * host byte sex.
 */
void
print_dylinker_command(
struct dylinker_command *dyld,
struct load_command *lc,
uint32_t left)
{
    char *p;

	if(dyld->cmd == LC_ID_DYLINKER)
	    printf("          cmd LC_ID_DYLINKER\n");
	else if(dyld->cmd == LC_LOAD_DYLINKER)
	    printf("          cmd LC_LOAD_DYLINKER\n");
	else if(dyld->cmd == LC_DYLD_ENVIRONMENT)
	    printf("          cmd LC_DYLD_ENVIRONMENT\n");
	else
	    printf("          cmd ?(%u)\n", dyld->cmd);
	printf("      cmdsize %u", dyld->cmdsize);
	if(dyld->cmdsize < sizeof(struct dylinker_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	if(dyld->name.offset < dyld->cmdsize &&
	   dyld->name.offset < left){
	    p = (char *)lc + dyld->name.offset;
	    printf("         name %s (offset %u)\n", p, dyld->name.offset);
	}
	else{
	    printf("         name ?(bad offset %u)\n", dyld->name.offset);
	}
}

/*
 * print an LC_FVMFILE command.  The fvmfile_command structure specified must
 * be aligned correctly and in the host byte sex.
 */
void
print_fvmfile_command(
struct fvmfile_command *ff,
struct load_command *lc,
uint32_t left)
{
    char *p;

	printf("           cmd LC_FVMFILE\n");
	printf("       cmdsize %u", ff->cmdsize);
	if(ff->cmdsize < sizeof(struct fvmfile_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	if(ff->name.offset < ff->cmdsize &&
	   ff->name.offset < left){
	    p = (char *)lc + ff->name.offset;
	    printf("          name %s (offset %u)\n", p, ff->name.offset);
	}
	else{
	    printf("          name ?(bad offset %u)\n", ff->name.offset);
	}
	printf("   header addr 0x%08x\n", (unsigned int)ff->header_addr);
}

/*
 * print an LC_ROUTINES command.  The routines_command structure specified must
 * be aligned correctly and in the host byte sex.
 */
void
print_routines_command(
struct routines_command *rc)
{
	printf("          cmd LC_ROUTINES\n");
	printf("      cmdsize %u", rc->cmdsize);
	if(rc->cmdsize != sizeof(struct routines_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	printf(" init_address 0x%08x\n", rc->init_address);
	printf("  init_module %u\n", rc->init_module);
	printf("    reserved1 %u\n", rc->reserved1);
	printf("    reserved2 %u\n", rc->reserved2);
	printf("    reserved3 %u\n", rc->reserved3);
	printf("    reserved4 %u\n", rc->reserved4);
	printf("    reserved5 %u\n", rc->reserved5);
	printf("    reserved6 %u\n", rc->reserved6);
}

/*
 * print an LC_ROUTINES_64 command.  The routines_command_64 structure specified
 * must be aligned correctly and in the host byte sex.
 */
void
print_routines_command_64(
struct routines_command_64 *rc64)
{
	printf("          cmd LC_ROUTINES_64\n");
	printf("      cmdsize %u", rc64->cmdsize);
	if(rc64->cmdsize != sizeof(struct routines_command_64))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	printf(" init_address 0x%016llx\n", rc64->init_address);
	printf("  init_module %llu\n", rc64->init_module);
	printf("    reserved1 %llu\n", rc64->reserved1);
	printf("    reserved2 %llu\n", rc64->reserved2);
	printf("    reserved3 %llu\n", rc64->reserved3);
	printf("    reserved4 %llu\n", rc64->reserved4);
	printf("    reserved5 %llu\n", rc64->reserved5);
	printf("    reserved6 %llu\n", rc64->reserved6);
}

/*
 * print an LC_TWOLEVEL_HINTS command.  The twolevel_hints_command structure
 * specified must be aligned correctly and in the host byte sex.
 */
void
print_twolevel_hints_command(
struct twolevel_hints_command *hints,
uint64_t object_size)
{
    uint64_t big_size;

	printf("     cmd LC_TWOLEVEL_HINTS\n");
	printf(" cmdsize %u", hints->cmdsize);
	if(hints->cmdsize != sizeof(struct twolevel_hints_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	printf("  offset %u", hints->offset);
	if(hints->offset > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("  nhints %u", hints->nhints);
	big_size = hints->nhints;
	big_size *= sizeof(struct twolevel_hint);
	big_size += hints->offset;
	if(big_size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
}

/*
 * print an LC_PREBIND_CKSUM command.  The prebind_cksum_command structure
 * specified must be aligned correctly and in the host byte sex.
 */
void
print_prebind_cksum_command(
struct prebind_cksum_command *cksum)
{
	printf("     cmd LC_PREBIND_CKSUM\n");
	printf(" cmdsize %u", cksum->cmdsize);
	if(cksum->cmdsize != sizeof(struct prebind_cksum_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	printf("   cksum 0x%08x\n", (unsigned int)cksum->cksum);
}

/*
 * print an LC_UUID command.  The uuid_command structure
 * specified must be aligned correctly and in the host byte sex.
 */
void
print_uuid_command(
struct uuid_command *uuid)
{
	printf("     cmd LC_UUID\n");
	printf(" cmdsize %u", uuid->cmdsize);
	if(uuid->cmdsize != sizeof(struct uuid_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	printf("    uuid %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-"
	       "%02X%02X%02X%02X%02X%02X\n",
	       (unsigned int)uuid->uuid[0], (unsigned int)uuid->uuid[1],
	       (unsigned int)uuid->uuid[2],  (unsigned int)uuid->uuid[3],
	       (unsigned int)uuid->uuid[4],  (unsigned int)uuid->uuid[5],
	       (unsigned int)uuid->uuid[6],  (unsigned int)uuid->uuid[7],
	       (unsigned int)uuid->uuid[8],  (unsigned int)uuid->uuid[9],
	       (unsigned int)uuid->uuid[10], (unsigned int)uuid->uuid[11],
	       (unsigned int)uuid->uuid[12], (unsigned int)uuid->uuid[13],
	       (unsigned int)uuid->uuid[14], (unsigned int)uuid->uuid[15]);
}

/*
 * print a linkedit_data_command.  The linkedit_data_command structure
 * specified must be aligned correctly and in the host byte sex.
 */
void
print_linkedit_data_command(
struct linkedit_data_command *ld,
uint64_t object_size)
{
    uint64_t big_size;

	if(ld->cmd == LC_CODE_SIGNATURE)
	    printf("      cmd LC_CODE_SIGNATURE\n");
	else if(ld->cmd == LC_SEGMENT_SPLIT_INFO)
	    printf("      cmd LC_SEGMENT_SPLIT_INFO\n");
        else if(ld->cmd == LC_FUNCTION_STARTS)
	    printf("      cmd LC_FUNCTION_STARTS\n");
        else if(ld->cmd == LC_DATA_IN_CODE)
	    printf("      cmd LC_DATA_IN_CODE\n");
        else if(ld->cmd == LC_DYLIB_CODE_SIGN_DRS)
	    printf("      cmd LC_DYLIB_CODE_SIGN_DRS\n");
        else if(ld->cmd == LC_LINKER_OPTIMIZATION_HINT)
	    printf("      cmd LC_LINKER_OPTIMIZATION_HINT\n");
        else if(ld->cmd == LC_DYLD_EXPORTS_TRIE)
	    printf("      cmd LC_DYLD_EXPORTS_TRIE\n");
        else if(ld->cmd == LC_DYLD_CHAINED_FIXUPS)
	    printf("      cmd LC_DYLD_CHAINED_FIXUPS\n");
	else
	    printf("      cmd %u (?)\n", ld->cmd);
	printf("  cmdsize %u", ld->cmdsize);
	if(ld->cmdsize != sizeof(struct linkedit_data_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	printf("  dataoff %u", ld->dataoff);
	if(ld->dataoff > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf(" datasize %u", ld->datasize);
	big_size = ld->dataoff;
	big_size += ld->datasize;
	if(big_size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
}

/*
 * print a version_min_command.  The version_min_command structure
 * specified must be aligned correctly and in the host byte sex.
 */
void
print_version_min_command(
struct version_min_command *vd)
{
	if(vd->cmd == LC_VERSION_MIN_MACOSX)
	    printf("      cmd LC_VERSION_MIN_MACOSX\n");
	else if(vd->cmd == LC_VERSION_MIN_IPHONEOS)
	    printf("      cmd LC_VERSION_MIN_IPHONEOS\n");
	else if(vd->cmd == LC_VERSION_MIN_WATCHOS)
	    printf("      cmd LC_VERSION_MIN_WATCHOS\n");
	else if(vd->cmd == LC_VERSION_MIN_TVOS)
	    printf("      cmd LC_VERSION_MIN_TVOS\n");
	else
	    printf("      cmd %u (?)\n", vd->cmd);
	printf("  cmdsize %u", vd->cmdsize);
	if(vd->cmdsize != sizeof(struct version_min_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	print_version_xyz("  version", vd->version);
	if(vd->sdk == 0)
	    printf("      sdk n/a\n");
	else
	    print_version_xyz("      sdk", vd->sdk);
}

/*
 * print a build_version_command.  The build_version_command structure
 * specified must be aligned correctly and in the host byte sex.
 */
void
print_build_version_command(
struct build_version_command *bv,
enum bool verbose)
{
	if(bv->cmd == LC_BUILD_VERSION)
	    printf("      cmd LC_BUILD_VERSION\n");
	else
	    printf("      cmd %u (?)\n", bv->cmd);
	printf("  cmdsize %u", bv->cmdsize);
	if(bv->cmdsize != sizeof(struct build_version_command) +
			  bv->ntools * sizeof(struct build_tool_version))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	if(verbose){
	    printf(" platform ");
	    switch(bv->platform){
	    case PLATFORM_MACOS:
		printf("MACOS\n");
		break;
	    case PLATFORM_IOS:
		printf("IOS\n");
		break;
	    case PLATFORM_TVOS:
		printf("TVOS\n");
		break;
	    case PLATFORM_WATCHOS:
		printf("WATCHOS\n");
		break;
	    case PLATFORM_BRIDGEOS:
		printf("BRIDGEOS\n");
		break;
	    case PLATFORM_MACCATALYST:
		printf("MACCATALYST\n");
		break;
	    case PLATFORM_IOSSIMULATOR:
		printf("IOSSIMULATOR\n");
		break;
	    case PLATFORM_TVOSSIMULATOR:
		printf("TVOSSIMULATOR\n");
		break;
	    case PLATFORM_WATCHOSSIMULATOR:
		printf("WATCHOSSIMULATOR\n");
		break;
	    case PLATFORM_DRIVERKIT:
		printf("DRIVERKIT\n");
		break;
	    default:
	        printf("%u\n", bv->platform);
		break;
	    }
	}
	else{
	    printf(" platform %u\n", bv->platform);
	}
	print_version_xyz("    minos", bv->minos);
	if(bv->sdk == 0)
	    printf("      sdk n/a\n");
	else{
	    print_version_xyz("      sdk", bv->sdk);
	}
	printf("   ntools %u\n", bv->ntools);
}

void
print_build_tool_version(
uint32_t tool,
uint32_t version,
enum bool verbose)
{
    if(verbose){
        printf("     tool ");
	switch(tool){
	case TOOL_CLANG:
	    printf("CLANG\n");
	    break;
	case TOOL_SWIFT:
	    printf("SWIFT\n");
	    break;
	case TOOL_LD:
	    printf("LD\n");
	    break;
	default:
	    printf("%u\n", tool);
	    break;
	}
    }
    else
        printf("     tool %u\n", tool);
    print_version_xyz("  version", version);
}

/*
 * print a source_version_command.  The source_version_command structure
 * specified must be aligned correctly and in the host byte sex.
 */
void
print_source_version_command(
struct source_version_command *sv)
{
    uint64_t a, b, c, d, e;

	printf("      cmd LC_SOURCE_VERSION\n");
	printf("  cmdsize %u", sv->cmdsize);
	if(sv->cmdsize != sizeof(struct source_version_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	a = (sv->version >> 40) & 0xffffff;
	b = (sv->version >> 30) & 0x3ff;
	c = (sv->version >> 20) & 0x3ff;
	d = (sv->version >> 10) & 0x3ff;
	e = sv->version & 0x3ff;
	if(e != 0)
	    printf("  version %llu.%llu.%llu.%llu.%llu\n", a, b, c, d, e);
	else if(d != 0)
	    printf("  version %llu.%llu.%llu.%llu\n", a, b, c, d);
	else if(c != 0)
	    printf("  version %llu.%llu.%llu\n", a, b, c);
	else
	    printf("  version %llu.%llu\n", a, b);
}

/*
 * print a note_command. The note_command structure specified must aligned and
 * in the host byte sex.
 */
void
print_note_command(
struct note_command *nc,
uint64_t object_size)
{
    uint64_t big_size;

	printf("       cmd LC_NOTE\n");
	printf("   cmdsize %u", nc->cmdsize);
	if(nc->cmdsize != sizeof(struct note_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	printf("data_owner %.16s\n", nc->data_owner);
	printf("    offset %llu", nc->offset);
	if(nc->offset > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("      size %llu", nc->size);
	big_size = nc->offset;
	big_size += nc->size;
	if(big_size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
}

/*
 * print a entry_point_command.  The entry_point_command structure
 * specified must be aligned correctly and in the host byte sex.
 */
void
print_entry_point_command(
struct entry_point_command *ep)
{
	printf("       cmd LC_MAIN\n");
	printf("   cmdsize %u", ep->cmdsize);
	if(ep->cmdsize != sizeof(struct entry_point_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	printf("  entryoff %llu\n", ep->entryoff);
	printf(" stacksize %llu\n", ep->stacksize);
}

/*
 * print an LC_RPATH command.  The rpath_command structure specified must be
 * aligned correctly and in the host byte sex.
 */
void
print_rpath_command(
struct rpath_command *rpath,
struct load_command *lc)
{
    char *p;

	printf("          cmd LC_RPATH\n");
	printf("      cmdsize %u", rpath->cmdsize);
	if(rpath->cmdsize < sizeof(struct rpath_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	if(rpath->path.offset < rpath->cmdsize){
	    p = (char *)lc + rpath->path.offset;
	    printf("         path %s (offset %u)\n", p, rpath->path.offset);
	}
	else{
	    printf("         path ?(bad offset %u)\n", rpath->path.offset);
	}
}

/*
 * print an LC_ENCRYPTION_INFO command.  The encryption_info_command structure
 * specified must be aligned correctly and in the host byte sex.
 */
void
print_encryption_info_command(
struct encryption_info_command *ec,
uint64_t object_size)
{
    uint64_t big_size;

	printf("          cmd LC_ENCRYPTION_INFO\n");
	printf("      cmdsize %u", ec->cmdsize);
	if(ec->cmdsize != sizeof(struct encryption_info_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	printf("     cryptoff %u", ec->cryptoff);
	if(ec->cryptoff > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("    cryptsize %u", ec->cryptsize);
	big_size = ec->cryptsize;
	big_size += ec->cryptoff;
	if(big_size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("      cryptid %u\n", ec->cryptid);
}

/*
 * print an LC_ENCRYPTION_INFO_64 command.  The encryption_info_command_64
 * structure specified must be aligned correctly and in the host byte sex.
 */
void
print_encryption_info_command_64(
struct encryption_info_command_64 *ec,
uint64_t object_size)
{
    uint64_t big_size;

	printf("          cmd LC_ENCRYPTION_INFO_64\n");
	printf("      cmdsize %u", ec->cmdsize);
	if(ec->cmdsize != sizeof(struct encryption_info_command_64))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	printf("     cryptoff %u", ec->cryptoff);
	if(ec->cryptoff > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("    cryptsize %u", ec->cryptsize);
	big_size = ec->cryptsize;
	big_size += ec->cryptoff;
	if(big_size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("      cryptid %u\n", ec->cryptid);
	printf("          pad %u\n", ec->pad);
}

/*
 * print an LC_LINKER_OPTION command.  The linker_option_command structure
 * specified must be aligned correctly and in the host byte sex.  The lc is
 * the actual load command with the strings that follow it and must have been
 * previously checked so that the cmdsize does not extend past the size of the
 * load commands.
 */
void
print_linker_option_command(
struct linker_option_command *lo,
struct load_command *lc,
uint32_t cmdleft)
{
    int left, i;
    size_t len;
    char *string;

	printf("     cmd LC_LINKER_OPTION\n");
	printf(" cmdsize %u", lo->cmdsize);
	if(lo->cmdsize < sizeof(struct linker_option_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");
	printf("   count %u\n", lo->count);
	string = (char *)lc + sizeof(struct linker_option_command);
	left = lo->cmdsize - sizeof(struct linker_option_command);
	if(left > cmdleft)
	    left = cmdleft;
	i = 0;
	while(left > 0){
	    while(*string == '\0' && left > 0){
		string++;
		left--;
	    }
	    if(left > 0){
		i++;
		printf("  string #%d %.*s\n", i, left, string);
		len = strnlen(string, left) + 1;
		string += len;
		left -= len;
	    }
	}
	if(lo->count != i)
	  printf("   count %u does not match number of strings %u\n",
		 lo->count, i);
}

/*
 * print an LC_DYLD_INFO command.  The dyld_info_command structure
 * specified must be aligned correctly and in the host byte sex.
 */
void
print_dyld_info_info_command(
struct dyld_info_command *dc,
uint64_t object_size)
{
    uint64_t big_size;

	if(dc->cmd == LC_DYLD_INFO)
	    printf("            cmd LC_DYLD_INFO\n");
	else
	    printf("            cmd LC_DYLD_INFO_ONLY\n");
	printf("        cmdsize %u", dc->cmdsize);
	if(dc->cmdsize != sizeof(struct dyld_info_command))
	    printf(" Incorrect size\n");
	else
	    printf("\n");

	printf("     rebase_off %u", dc->rebase_off);
	if(dc->rebase_off > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("    rebase_size %u", dc->rebase_size);
	big_size = dc->rebase_off;
	big_size += dc->rebase_size;
	if(big_size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");

	printf("       bind_off %u", dc->bind_off);
	if(dc->bind_off > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("      bind_size %u", dc->bind_size);
	big_size = dc->bind_off;
	big_size += dc->bind_size;
	if(big_size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	    
	printf("  weak_bind_off %u", dc->weak_bind_off);
	if(dc->weak_bind_off > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf(" weak_bind_size %u", dc->weak_bind_size);
	big_size = dc->weak_bind_off;
	big_size += dc->weak_bind_size;
	if(big_size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");

	printf("  lazy_bind_off %u", dc->lazy_bind_off);
	if(dc->lazy_bind_off > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf(" lazy_bind_size %u", dc->lazy_bind_size);
	big_size = dc->lazy_bind_off;
	big_size += dc->lazy_bind_size;
	if(big_size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	    
	printf("     export_off %u", dc->export_off);
	if(dc->export_off > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
	printf("    export_size %u", dc->export_size);
	big_size = dc->export_off;
	big_size += dc->export_size;
	if(big_size > object_size)
	    printf(" (past end of file)\n");
	else
	    printf("\n");
}

/*
 * print the thread states from an LC_THREAD or LC_UNIXTHREAD command.  The
 * thread state triples (flavor, count, state) are in memory between begin and
 * and end values specified, and in the specified byte sex.  The mach_header
 * structure specified must be aligned correctly and in the host byte sex.
 */
void
print_thread_states(
char *begin, 
char *end,
cpu_type_t cputype,
enum byte_sex thread_states_byte_sex)
{
    uint32_t i, j, k, flavor, count, left;
    enum byte_sex host_byte_sex;
    enum bool swapped;

	i = 0;
	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != thread_states_byte_sex;

	if(cputype == CPU_TYPE_MC680x0){
	    struct m68k_thread_state_regs cpu;
	    struct m68k_thread_state_68882 fpu;
	    struct m68k_thread_state_user_reg user_reg;

	    while(begin < end){
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&flavor, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    flavor = 0;
		    begin = end;
		}
		if(swapped)
		    flavor = SWAP_INT(flavor);
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&count, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    count = 0;
		    begin = end;
		}
		if(swapped)
		    count = SWAP_INT(count);

		switch(flavor){
		case M68K_THREAD_STATE_REGS:
		    printf("     flavor M68K_THREAD_STATE_REGS\n");
		    if(count == M68K_THREAD_STATE_REGS_COUNT)
			printf("      count M68K_THREAD_STATE_"
			       "REGS_COUNT\n");
		    else
			printf("      count %u (not M68K_THREAD_STATE_"
			       "REGS_COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(struct m68k_thread_state_regs)){
		        memcpy((char *)&cpu, begin,
			       sizeof(struct m68k_thread_state_regs));
		        begin += sizeof(struct m68k_thread_state_regs);
		    }
		    else{
		        memset((char *)&cpu, '\0',
			       sizeof(struct m68k_thread_state_regs));
		        memcpy((char *)&cpu, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_m68k_thread_state_regs(&cpu, host_byte_sex);
		    printf(" dregs ");
		    for(j = 0 ; j < 8 ; j++)
			printf(" %08x", (unsigned int)cpu.dreg[j]);
		    printf("\n");
		    printf(" aregs ");
		    for(j = 0 ; j < 8 ; j++)
			printf(" %08x", (unsigned int)cpu.areg[j]);
		    printf("\n");
		    printf(" pad 0x%04x sr 0x%04x pc 0x%08x\n", 
			    (unsigned int)(cpu.pad0 & 0x0000ffff),
			    (unsigned int)(cpu.sr & 0x0000ffff),
			    (unsigned int)cpu.pc);
		    break;

		case M68K_THREAD_STATE_68882:
		    printf("     flavor M68K_THREAD_STATE_68882\n");
		    if(count == M68K_THREAD_STATE_68882_COUNT)
			printf("      count M68K_THREAD_STATE_"
			       "68882_COUNT\n");
		    else
			printf("      count %u (not M68K_THREAD_STATE_"
			       "68882_COUNT\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(struct m68k_thread_state_68882)){
		        memcpy((char *)&fpu, begin,
			       sizeof(struct m68k_thread_state_68882));
		        begin += sizeof(struct m68k_thread_state_68882);
		    }
		    else{
		        memset((char *)&fpu, '\0',
			       sizeof(struct m68k_thread_state_68882));
		        memcpy((char *)&fpu, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_m68k_thread_state_68882(&fpu, host_byte_sex);
		    for(j = 0 ; j < 8 ; j++)
			printf(" fp reg %u %08x %08x %08x\n", j,
			       (unsigned int)fpu.regs[j].fp[0],
			       (unsigned int)fpu.regs[j].fp[1],
			       (unsigned int)fpu.regs[j].fp[2]);
		    printf(" cr 0x%08x sr 0x%08x state 0x%08x\n", 
			   (unsigned int)fpu.cr, (unsigned int)fpu.sr,
			   (unsigned int)fpu.state);
		    break;

		case M68K_THREAD_STATE_USER_REG:
		    printf("     flavor M68K_THREAD_STATE_USER_REG\n");
		    if(count == M68K_THREAD_STATE_USER_REG_COUNT)
			printf("      count M68K_THREAD_STATE_"
			       "USER_REG_COUNT\n");
		    else
			printf("      count %u (not M68K_THREAD_STATE_"
			       "USER_REG_COUNT", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(struct m68k_thread_state_user_reg)){
		        memcpy((char *)&user_reg, begin,
			       sizeof(struct m68k_thread_state_user_reg));
		        begin += sizeof(struct m68k_thread_state_user_reg);
		    }
		    else{
		        memset((char *)&user_reg, '\0',
			       sizeof(struct m68k_thread_state_user_reg));
		        memcpy((char *)&user_reg, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_m68k_thread_state_user_reg(&user_reg,
							host_byte_sex);
		    printf(" user_reg 0x%08x\n",
			   (unsigned int)user_reg.user_reg);
		    break;

		default:
		    printf("     flavor %u (unknown)\n", flavor);
		    printf("      count %u\n", count);
		    printf("      state:\n");
		    print_unknown_state(begin, end, count, swapped);
		    begin += count * sizeof(uint32_t);
		    break;
		}
	    }
	}
	else if(cputype == CPU_TYPE_HPPA){
	    while(begin < end){
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&flavor, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    flavor = 0;
		    begin = end;
		}
		if(swapped)
		    flavor = SWAP_INT(flavor);
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&count, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    count = 0;
		    begin = end;
		}
		if(swapped)
		    count = SWAP_INT(count);

		switch(flavor){
		case HPPA_INTEGER_THREAD_STATE:
			{ struct hp_pa_integer_thread_state frame;
			
		    printf("      flavor HPPA_INTEGER_THREAD_STATE\n");
		    if(count == HPPA_INTEGER_THREAD_STATE_COUNT)
			printf("      count HPPA_INTEGER_THREAD_STATE_COUNT\n");
		    else
			printf("      count %u (not HPPA_INTEGER_THREAD_STATE_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(struct hp_pa_integer_thread_state)){
		        memcpy((char *)&frame, begin,
			       sizeof(struct hp_pa_integer_thread_state));
		        begin += sizeof(struct hp_pa_integer_thread_state);
		    }
		    else{
		        memset((char *)&frame, '\0',
			       sizeof(struct hp_pa_integer_thread_state));
		        memcpy((char *)&frame, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_hppa_integer_thread_state(&frame, host_byte_sex);
			printf(
		         "r1  0x%08x  r2  0x%08x  r3  0x%08x  r4  0x%08x\n"
		         "r5  0x%08x  r6  0x%08x  r7  0x%08x  r8  0x%08x\n"
		         "r9  0x%08x  r10 0x%08x  r11 0x%08x  r12 0x%08x\n"
		         "r13 0x%08x  r14 0x%08x  r15 0x%08x  r16 0x%08x\n"
		         "r17 0x%08x  r18 0x%08x  r19 0x%08x  r20 0x%08x\n"
		         "r21 0x%08x  r22 0x%08x  r23 0x%08x  r24 0x%08x\n"
		         "r25 0x%08x  r26 0x%08x  r27 0x%08x  r28 0x%08x\n"
		         "r29 0x%08x  r30 0x%08x  r31 0x%08x\n"
			 "sr0 0x%08x  sr1 0x%08x  sr2 0x%08x  sar 0x%08x\n",
		   frame.ts_gr1,  frame.ts_gr2,  frame.ts_gr3,  frame.ts_gr4,
		   frame.ts_gr5,  frame.ts_gr6,  frame.ts_gr7,  frame.ts_gr8,
		   frame.ts_gr9,  frame.ts_gr10, frame.ts_gr11, frame.ts_gr12,
		   frame.ts_gr13, frame.ts_gr14, frame.ts_gr15, frame.ts_gr16,
		   frame.ts_gr17, frame.ts_gr18, frame.ts_gr19, frame.ts_gr20,
		   frame.ts_gr21, frame.ts_gr22, frame.ts_gr23, frame.ts_gr24,
		   frame.ts_gr25, frame.ts_gr26, frame.ts_gr27, frame.ts_gr28,
		   frame.ts_gr29, frame.ts_gr30, frame.ts_gr31,
		   frame.ts_sr0,  frame.ts_sr1,  frame.ts_sr2,  frame.ts_sar);
			}
		    break;
		case HPPA_FRAME_THREAD_STATE: {
			struct hp_pa_frame_thread_state frame;
		    printf("      flavor HPPA_FRAME_THREAD_STATE\n");
		    if(count == HPPA_FRAME_THREAD_STATE_COUNT)
			printf("      count HPPA_FRAME_THREAD_STATE_COUNT\n");
		    else
			printf("      count %u (not HPPA_FRAME_THREAD_STATE_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(struct hp_pa_frame_thread_state)){
		        memcpy((char *)&frame, begin,
			       sizeof(struct hp_pa_frame_thread_state));
		        begin += sizeof(struct hp_pa_frame_thread_state);
		    }
		    else{
		        memset((char *)&frame, '\0',
			       sizeof(struct hp_pa_frame_thread_state));
		        memcpy((char *)&frame, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_hppa_frame_thread_state(&frame, host_byte_sex);
		    printf("pcsq_front  0x%08x pcsq_back  0x%08x\n"
		           "pcoq_front  0x%08x pcoq_back  0x%08x\n"
			   "       psw  0x%08x\n",
			   frame.ts_pcsq_front, frame.ts_pcsq_back,
			   frame.ts_pcoq_front, frame.ts_pcoq_back,
			   frame.ts_psw);
		    break;
		}
		case HPPA_FP_THREAD_STATE: {
			struct hp_pa_fp_thread_state frame;
		    printf("      flavor HPPA_FP_THREAD_STATE\n");
		    if(count == HPPA_FP_THREAD_STATE_COUNT)
			printf("      count HPPA_FP_THREAD_STATE_COUNT\n");
		    else
			printf("      count %u (not HPPA_FP_THREAD_STATE_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(struct hp_pa_fp_thread_state)){
		        memcpy((char *)&frame, begin,
			       sizeof(struct hp_pa_fp_thread_state));
		        begin += sizeof(struct hp_pa_fp_thread_state);
		    }
		    else{
		        memset((char *)&frame, '\0',
			       sizeof(struct hp_pa_fp_thread_state));
		        memcpy((char *)&frame, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_hppa_fp_thread_state(&frame, host_byte_sex);
		    printf("fp0  %f    fp1  %f\nfp2  %f    fp3  %f\n"
			   "fp4  %f    fp5  %f\nfp6  %f    fp7  %f\n"
			   "fp8  %f    fp9  %f\nfp10 %f    fp11 %f\n"
			   "fp12 %f    fp13 %f\nfp14 %f    fp15 %f\n"
			   "fp16 %f    fp17 %f\nfp18 %f    fp19 %f\n"
			   "fp20 %f    fp21 %f\nfp22 %f    fp23 %f\n"
			   "fp24 %f    fp25 %f\nfp26 %f    fp27 %f\n"
			   "fp28 %f    fp29 %f\nfp30 %f    fp31 %f\n",
		    frame.ts_fp0,  frame.ts_fp1,  frame.ts_fp2,  frame.ts_fp3,
		    frame.ts_fp4,  frame.ts_fp5,  frame.ts_fp6,  frame.ts_fp7,
		    frame.ts_fp8,  frame.ts_fp9,  frame.ts_fp10, frame.ts_fp11,
		    frame.ts_fp12, frame.ts_fp13, frame.ts_fp14, frame.ts_fp15,
		    frame.ts_fp16, frame.ts_fp17, frame.ts_fp18, frame.ts_fp19,
		    frame.ts_fp20, frame.ts_fp21, frame.ts_fp22, frame.ts_fp23,
		    frame.ts_fp24, frame.ts_fp25, frame.ts_fp26, frame.ts_fp27,
		    frame.ts_fp28, frame.ts_fp29, frame.ts_fp30, frame.ts_fp31);
		    break;
		}
		default:
		    printf("     flavor %u (unknown)\n", flavor);
		    printf("      count %u\n", count);
		    printf("      state:\n");
		    print_unknown_state(begin, end, count, swapped);
		    begin += count * sizeof(uint32_t);
		    break;
		}
	    }
	}
	else if(cputype == CPU_TYPE_SPARC){
	    while(begin < end){
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&flavor, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    flavor = 0;
		    begin = end;
		}
		if(swapped)
		    flavor = SWAP_INT(flavor);
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&count, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    count = 0;
		    begin = end;
		}
		if(swapped)
		    count = SWAP_INT(count);

		switch(flavor){
		case SPARC_THREAD_STATE_REGS:
		  { struct sparc_thread_state_regs cpu;
		    printf("     flavor SPARC_THREAD_STATE_REGS\n");
		    if (count == SPARC_THREAD_STATE_REGS_COUNT)
		      printf("      count SPARC_THREAD_STATE_REGS_COUNT\n");
		    else
		      printf("      count %u (not SPARC_THREAD_STATE_REGS_COUNT)\n",
			     count);
		    left = (uint32_t)(end - begin);
		    if (left >= sizeof(struct sparc_thread_state_regs)) {
		      memcpy((char *) &cpu, begin,
			     sizeof(struct sparc_thread_state_regs));
		      begin += sizeof(struct sparc_thread_state_regs);
		    } else {
		      memset((char *) &cpu, '\0',
			     sizeof(struct sparc_thread_state_regs));
		      begin += left;
		    }
		    if (swapped)
		      swap_sparc_thread_state_regs(&cpu, host_byte_sex);
		    printf(
			   "psr 0x%08x  pc  0x%08x  npc 0x%08x  y   0x%08x\n"
			   "g0  0x%08x  g1  0x%08x  g2  0x%08x  g3  0x%08x\n"
			   "g4  0x%08x  g5  0x%08x  g6  0x%08x  g7  0x%08x\n"
			   "o0  0x%08x  o1  0x%08x  o2  0x%08x  o3  0x%08x\n"
			   "o4  0x%08x  o5  0x%08x  o6  0x%08x  o7  0x%08x\n",
			   cpu.regs.r_psr, cpu.regs.r_pc, cpu.regs.r_npc, 
			   cpu.regs.r_y, 0, cpu.regs.r_g1, 
			   cpu.regs.r_g2, cpu.regs.r_g3,
			   cpu.regs.r_g4, cpu.regs.r_g5, 
			   cpu.regs.r_g6, cpu.regs.r_g7,
			   cpu.regs.r_o0, cpu.regs.r_o1, 
			   cpu.regs.r_o2, cpu.regs.r_o3,
			   cpu.regs.r_o4, cpu.regs.r_o5, 
			   cpu.regs.r_o6, cpu.regs.r_o7);
		    break;
		  }
		case SPARC_THREAD_STATE_FPU:
		  { struct sparc_thread_state_fpu fpu;

		    printf("     flavor SPARC_THREAD_STATE_FPU\n");
		    if (count == SPARC_THREAD_STATE_FPU_COUNT)
		      printf("      count SPARC_THREAD_STATE_FPU_COUNT\n");
		    else
		      printf("      count %u (not SPARC_THREAD_STATE_FPU_COUNT)\n",
			     count);
		    left = (uint32_t)(end - begin);
		    if (left >= sizeof(struct sparc_thread_state_fpu)) {
		      memcpy((char *) &fpu, begin,
			     sizeof(struct sparc_thread_state_fpu));
		      begin += sizeof(struct sparc_thread_state_fpu);
		    } else {
		      memset((char *) &fpu, '\0',
			     sizeof(struct sparc_thread_state_fpu));
		      begin += left;
		    }
		    if (swapped)
		      swap_sparc_thread_state_fpu(&fpu, host_byte_sex);
		    printf(
			   "f0  0x%08x  f1  0x%08x  f2  0x%08x  f3  0x%08x\n"
			   "f4  0x%08x  f5  0x%08x  f6  0x%08x  f7  0x%08x\n"
			   "f8  0x%08x  f9  0x%08x  f10 0x%08x  f11 0x%08x\n"
			   "f12 0x%08x  f13 0x%08x  f14 0x%08x  f15 0x%08x\n"
			   "f16 0x%08x  f17 0x%08x  f18 0x%08x  f19 0x%08x\n"
			   "f20 0x%08x  f21 0x%08x  f22 0x%08x  f23 0x%08x\n"
			   "f24 0x%08x  f25 0x%08x  f26 0x%08x  f27 0x%08x\n"
			   "f28 0x%08x  f29 0x%08x  f30 0x%08x  f31 0x%08x\n"
			   "fsr 0x%08x\n",
			   fpu.fpu.fpu_fr.Fpu_regs[0], fpu.fpu.fpu_fr.Fpu_regs[1],
			   fpu.fpu.fpu_fr.Fpu_regs[2], fpu.fpu.fpu_fr.Fpu_regs[3],
			   fpu.fpu.fpu_fr.Fpu_regs[4], fpu.fpu.fpu_fr.Fpu_regs[5],
			   fpu.fpu.fpu_fr.Fpu_regs[6], fpu.fpu.fpu_fr.Fpu_regs[7],
			   fpu.fpu.fpu_fr.Fpu_regs[8], fpu.fpu.fpu_fr.Fpu_regs[9],
			   fpu.fpu.fpu_fr.Fpu_regs[10], fpu.fpu.fpu_fr.Fpu_regs[11],
			   fpu.fpu.fpu_fr.Fpu_regs[12], fpu.fpu.fpu_fr.Fpu_regs[13],
			   fpu.fpu.fpu_fr.Fpu_regs[14], fpu.fpu.fpu_fr.Fpu_regs[15],
			   fpu.fpu.fpu_fr.Fpu_regs[16], fpu.fpu.fpu_fr.Fpu_regs[17],
			   fpu.fpu.fpu_fr.Fpu_regs[18], fpu.fpu.fpu_fr.Fpu_regs[19],
			   fpu.fpu.fpu_fr.Fpu_regs[20], fpu.fpu.fpu_fr.Fpu_regs[21],
			   fpu.fpu.fpu_fr.Fpu_regs[22], fpu.fpu.fpu_fr.Fpu_regs[23],
			   fpu.fpu.fpu_fr.Fpu_regs[24], fpu.fpu.fpu_fr.Fpu_regs[25],
			   fpu.fpu.fpu_fr.Fpu_regs[26], fpu.fpu.fpu_fr.Fpu_regs[27],
			   fpu.fpu.fpu_fr.Fpu_regs[28], fpu.fpu.fpu_fr.Fpu_regs[29],
			   fpu.fpu.fpu_fr.Fpu_regs[30], fpu.fpu.fpu_fr.Fpu_regs[31],
			   fpu.fpu.Fpu_fsr);
		    break;
		  }
		default:
		    printf("     flavor %u (unknown)\n", flavor);
		    printf("      count %u\n", count);
		    printf("      state:\n");
		    print_unknown_state(begin, end, count, swapped);
		    begin += count * sizeof(uint32_t);
		    break;
		}
	    }
	}
	else if(cputype == CPU_TYPE_POWERPC ||
	   cputype == CPU_TYPE_POWERPC64 ||
	   cputype == CPU_TYPE_VEO){
	    ppc_thread_state_t cpu;
	    ppc_thread_state64_t cpu64;
	    ppc_float_state_t fpu;
	    ppc_exception_state_t except;

	    while(begin < end){
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&flavor, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    flavor = 0;
		    begin = end;
		}
		if(swapped)
		    flavor = SWAP_INT(flavor);
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&count, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    count = 0;
		    begin = end;
		}
		if(swapped)
		    count = SWAP_INT(count);

		switch(flavor){
		case PPC_THREAD_STATE:
		    printf("     flavor PPC_THREAD_STATE\n");
		    if(count == PPC_THREAD_STATE_COUNT)
			printf("      count PPC_THREAD_STATE_COUNT\n");
		    else
			printf("      count %u (not PPC_THREAD_STATE_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(ppc_thread_state_t)){
		        memcpy((char *)&cpu, begin,
			       sizeof(ppc_thread_state_t));
		        begin += sizeof(ppc_thread_state_t);
		    }
		    else{
		        memset((char *)&cpu, '\0',
			       sizeof(ppc_thread_state_t));
		        memcpy((char *)&cpu, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_ppc_thread_state_t(&cpu, host_byte_sex);
		    printf("    r0  0x%08x r1  0x%08x r2  0x%08x r3   0x%08x "
			   "r4   0x%08x\n"
			   "    r5  0x%08x r6  0x%08x r7  0x%08x r8   0x%08x "
			   "r9   0x%08x\n"
			   "    r10 0x%08x r11 0x%08x r12 0x%08x r13  0x%08x "
			   "r14  0x%08x\n"
			   "    r15 0x%08x r16 0x%08x r17 0x%08x r18  0x%08x "
			   "r19  0x%08x\n"
			   "    r20 0x%08x r21 0x%08x r22 0x%08x r23  0x%08x "
			   "r24  0x%08x\n"
			   "    r25 0x%08x r26 0x%08x r27 0x%08x r28  0x%08x "
			   "r29  0x%08x\n"
			   "    r30 0x%08x r31 0x%08x cr  0x%08x xer  0x%08x "
			   "lr   0x%08x\n"
			   "    ctr 0x%08x mq  0x%08x vrsave 0x%08x srr0 0x%08x"
			   " srr1 0x%08x\n",
			   cpu.r0, cpu.r1, cpu.r2, cpu.r3, cpu.r4, cpu.r5,
			   cpu.r6, cpu.r7, cpu.r8, cpu.r9, cpu.r10, cpu.r11,
			   cpu.r12, cpu.r13, cpu.r14, cpu.r15, cpu.r16, cpu.r17,
			   cpu.r18, cpu.r19, cpu.r20, cpu.r21, cpu.r22, cpu.r23,
			   cpu.r24, cpu.r25, cpu.r26, cpu.r27, cpu.r28, cpu.r29,
			   cpu.r30, cpu.r31, cpu.cr,  cpu.xer, cpu.lr, cpu.ctr,
			   cpu.mq, cpu.vrsave, cpu.srr0, cpu.srr1);
		    break;
		case PPC_FLOAT_STATE:
		    printf("      flavor PPC_FLOAT_STATE\n");
		    if(count == PPC_FLOAT_STATE_COUNT)
			printf("      count PPC_FLOAT_STATE_COUNT\n");
		    else
			printf("      count %u (not PPC_FLOAT_STATE_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(ppc_float_state_t)){
		        memcpy((char *)&fpu, begin,
			       sizeof(ppc_float_state_t));
		        begin += sizeof(ppc_float_state_t);
		    }
		    else{
		        memset((char *)&fpu, '\0',
			       sizeof(ppc_float_state_t));
		        memcpy((char *)&fpu, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_ppc_float_state_t(&fpu, host_byte_sex);
		    printf("       f0  %f    f1  %f\n       f2  %f    f3  %f\n"
			   "       f4  %f    f5  %f\n       f6  %f    f7  %f\n"
			   "       f8  %f    f9  %f\n       f10 %f    f11 %f\n"
			   "       f12 %f    f13 %f\n       f14 %f    f15 %f\n"
			   "       f16 %f    f17 %f\n       f18 %f    f19 %f\n"
			   "       f20 %f    f21 %f\n       f22 %f    f23 %f\n"
			   "       f24 %f    f25 %f\n       f26 %f    f27 %f\n"
			   "       f28 %f    f29 %f\n       f30 %f    f31 %f\n",
			   fpu.fpregs[0],  fpu.fpregs[1],  fpu.fpregs[2],
			   fpu.fpregs[3],  fpu.fpregs[4],  fpu.fpregs[5],
			   fpu.fpregs[6],  fpu.fpregs[7],  fpu.fpregs[8],
			   fpu.fpregs[9],  fpu.fpregs[10], fpu.fpregs[11],
			   fpu.fpregs[12], fpu.fpregs[13], fpu.fpregs[14],
			   fpu.fpregs[15], fpu.fpregs[16], fpu.fpregs[17],
			   fpu.fpregs[18], fpu.fpregs[19], fpu.fpregs[20],
			   fpu.fpregs[21], fpu.fpregs[22], fpu.fpregs[23],
			   fpu.fpregs[24], fpu.fpregs[25], fpu.fpregs[26],
			   fpu.fpregs[27], fpu.fpregs[28], fpu.fpregs[29],
			   fpu.fpregs[30], fpu.fpregs[31]);
		    printf("       fpscr_pad 0x%x fpscr 0x%x\n", fpu.fpscr_pad,
			   fpu.fpscr);
		    break;
		case PPC_EXCEPTION_STATE:
		    printf("      flavor PPC_EXCEPTION_STATE\n");
		    if(count == PPC_EXCEPTION_STATE_COUNT)
			printf("      count PPC_EXCEPTION_STATE_COUNT\n");
		    else
			printf("      count %u (not PPC_EXCEPTION_STATE_COUNT"
			       ")\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(ppc_exception_state_t)){
		        memcpy((char *)&except, begin,
			       sizeof(ppc_exception_state_t));
		        begin += sizeof(ppc_exception_state_t);
		    }
		    else{
		        memset((char *)&except, '\0',
			       sizeof(ppc_exception_state_t));
		        memcpy((char *)&except, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_ppc_exception_state_t(&except, host_byte_sex);
		    printf("      dar 0x%x dsisr 0x%x exception 0x%x pad0 "
			   "0x%x\n", (unsigned int)except.dar,
			   (unsigned int)except.dsisr,
			   (unsigned int)except.exception,
			   (unsigned int)except.pad0);
		    printf("      pad1[0] 0x%x pad1[1] 0x%x pad1[2] "
			   "0x%x pad1[3] 0x%x\n", (unsigned int)except.pad1[0],
			   (unsigned int)except.pad1[0],
			   (unsigned int)except.pad1[0],
			   (unsigned int)except.pad1[0]);
		    break;
		case PPC_THREAD_STATE64:
		    printf("     flavor PPC_THREAD_STATE64\n");
		    if(count == PPC_THREAD_STATE64_COUNT)
			printf("      count PPC_THREAD_STATE64_COUNT\n");
		    else
			printf("      count %u (not PPC_THREAD_STATE64_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(ppc_thread_state64_t)){
		        memcpy((char *)&cpu64, begin,
			       sizeof(ppc_thread_state64_t));
		        begin += sizeof(ppc_thread_state64_t);
		    }
		    else{
		        memset((char *)&cpu64, '\0',
			       sizeof(ppc_thread_state64_t));
		        memcpy((char *)&cpu64, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_ppc_thread_state64_t(&cpu64, host_byte_sex);
		    printf("    r0  0x%016llx r1  0x%016llx r2   0x%016llx\n"
			   "    r3  0x%016llx r4  0x%016llx r5   0x%016llx\n"
			   "    r6  0x%016llx r7  0x%016llx r8   0x%016llx\n"
			   "    r9  0x%016llx r10 0x%016llx r11  0x%016llx\n"
			   "   r12  0x%016llx r13 0x%016llx r14  0x%016llx\n"
			   "   r15  0x%016llx r16 0x%016llx r17  0x%016llx\n"
			   "   r18  0x%016llx r19 0x%016llx r20  0x%016llx\n"
			   "   r21  0x%016llx r22 0x%016llx r23  0x%016llx\n"
			   "   r24  0x%016llx r25 0x%016llx r26  0x%016llx\n"
			   "   r27  0x%016llx r28 0x%016llx r29  0x%016llx\n"
			   "   r30  0x%016llx r31 0x%016llx cr   0x%08x\n"
			   "   xer  0x%016llx lr  0x%016llx ctr  0x%016llx\n"
			   "vrsave  0x%08x        srr0 0x%016llx srr1 "
			   "0x%016llx\n",
			   cpu64.r0, cpu64.r1, cpu64.r2, cpu64.r3, cpu64.r4,
			   cpu64.r5, cpu64.r6, cpu64.r7, cpu64.r8, cpu64.r9,
			   cpu64.r10, cpu64.r11, cpu64.r12, cpu64.r13,cpu64.r14,
			   cpu64.r15, cpu64.r16, cpu64.r17, cpu64.r18,cpu64.r19,
			   cpu64.r20, cpu64.r21, cpu64.r22, cpu64.r23,cpu64.r24,
			   cpu64.r25, cpu64.r26, cpu64.r27, cpu64.r28,cpu64.r29,
			   cpu64.r30, cpu64.r31, cpu64.cr,  cpu64.xer, cpu64.lr,
			   cpu64.ctr, cpu64.vrsave, cpu64.srr0, cpu64.srr1);
		    break;
		default:
		    printf("     flavor %u (unknown)\n", flavor);
		    printf("      count %u\n", count);
		    printf("      state:\n");
		    print_unknown_state(begin, end, count, swapped);
		    begin += count * sizeof(uint32_t);
		    break;
		}
	    }
	}
	else if(cputype == CPU_TYPE_MC88000){
	    m88k_thread_state_grf_t cpu;
	    m88k_thread_state_xrf_t fpu;
	    m88k_thread_state_user_t user;
	    m88110_thread_state_impl_t spu;

	    while(begin < end){
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&flavor, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    flavor = 0;
		    begin = end;
		}
		if(swapped)
		    flavor = SWAP_INT(flavor);
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&count, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    count = 0;
		    begin = end;
		}
		if(swapped)
		    count = SWAP_INT(count);

		switch(flavor){
		case M88K_THREAD_STATE_GRF:
		    printf("      flavor M88K_THREAD_STATE_GRF\n");
		    if(count == M88K_THREAD_STATE_GRF_COUNT)
			printf("      count M88K_THREAD_STATE_GRF_COUNT\n");
		    else
			printf("      count %u (not M88K_THREAD_STATE_GRF_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(m88k_thread_state_grf_t)){
		        memcpy((char *)&cpu, begin,
			       sizeof(m88k_thread_state_grf_t));
		        begin += sizeof(m88k_thread_state_grf_t);
		    }
		    else{
		        memset((char *)&cpu, '\0',
			       sizeof(m88k_thread_state_grf_t));
		        memcpy((char *)&cpu, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_m88k_thread_state_grf_t(&cpu, host_byte_sex);
		    printf("      r1  0x%08x r2  0x%08x r3  0x%08x r4  0x%08x "
			   "r5  0x%08x\n"
			   "      r6  0x%08x r7  0x%08x r8  0x%08x r9  0x%08x "
			   "r10 0x%08x\n"
			   "      r11 0x%08x r12 0x%08x r13 0x%08x r14 0x%08x "
			   "r15 0x%08x\n"
			   "      r16 0x%08x r17 0x%08x r18 0x%08x r19 0x%08x "
			   "r20 0x%08x\n"
			   "      r21 0x%08x r22 0x%08x r23 0x%08x r24 0x%08x "
			   "r25 0x%08x\n"
			   "      r26 0x%08x r27 0x%08x r28 0x%08x r29 0x%08x "
			   "r30 0x%08x\n"
			   "      r31 0x%08x xip 0x%08x xip_in_bd 0x%08x nip "
			   "0x%08x\n",
			   cpu.r1,  cpu.r2,  cpu.r3,  cpu.r4,  cpu.r5,
			   cpu.r6,  cpu.r7,  cpu.r8,  cpu.r9,  cpu.r10,
			   cpu.r11, cpu.r12, cpu.r13, cpu.r14, cpu.r15,
			   cpu.r16, cpu.r17, cpu.r18, cpu.r19, cpu.r20,
			   cpu.r21, cpu.r22, cpu.r23, cpu.r24, cpu.r25,
			   cpu.r26, cpu.r27, cpu.r28, cpu.r29, cpu.r30,
			   cpu.r31, cpu.xip, cpu.xip_in_bd, cpu.nip);
		    break;
		case M88K_THREAD_STATE_XRF:
		    printf("      flavor M88K_THREAD_STATE_XRF\n");
		    if(count == M88K_THREAD_STATE_XRF_COUNT)
			printf("      count M88K_THREAD_STATE_XRF_COUNT\n");
		    else
			printf("      count %u (not M88K_THREAD_STATE_XRF_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(m88k_thread_state_xrf_t)){
		        memcpy((char *)&fpu, begin,
			       sizeof(m88k_thread_state_xrf_t));
		        begin += sizeof(m88k_thread_state_xrf_t);
		    }
		    else{
		        memset((char *)&fpu, '\0',
			       sizeof(m88k_thread_state_xrf_t));
		        memcpy((char *)&fpu, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_m88k_thread_state_xrf_t(&fpu, host_byte_sex);
		    printf("      x1  0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x2  0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x3  0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x4  0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x5  0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x6  0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x7  0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x8  0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x9  0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x10 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x11 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x12 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x13 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x14 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x15 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x16 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x17 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x18 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x19 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x20 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x21 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x22 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x23 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x24 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x25 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x26 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x27 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x28 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x29 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x30 0x%08x 0x%08x 0x%08x 0x%08x\n"
			   "      x31 0x%08x 0x%08x 0x%08x 0x%08x\n",
			   fpu.x1.x[0],fpu.x1.x[1],fpu.x1.x[2],fpu.x1.x[3],
			   fpu.x2.x[0],fpu.x2.x[1],fpu.x2.x[2],fpu.x2.x[3],
			   fpu.x3.x[0],fpu.x3.x[1],fpu.x3.x[2],fpu.x3.x[3],
			   fpu.x4.x[0],fpu.x4.x[1],fpu.x4.x[2],fpu.x4.x[3],
			   fpu.x5.x[0],fpu.x5.x[1],fpu.x5.x[2],fpu.x5.x[3],
			   fpu.x6.x[0],fpu.x6.x[1],fpu.x6.x[2],fpu.x6.x[3],
			   fpu.x7.x[0],fpu.x7.x[1],fpu.x7.x[2],fpu.x7.x[3],
			   fpu.x8.x[0],fpu.x8.x[1],fpu.x8.x[2],fpu.x8.x[3],
			   fpu.x9.x[0],fpu.x9.x[1],fpu.x9.x[2],fpu.x9.x[3],
			   fpu.x10.x[0],fpu.x10.x[1],fpu.x10.x[2],fpu.x10.x[3],
			   fpu.x11.x[0],fpu.x11.x[1],fpu.x11.x[2],fpu.x11.x[3],
			   fpu.x12.x[0],fpu.x12.x[1],fpu.x12.x[2],fpu.x12.x[3],
			   fpu.x13.x[0],fpu.x13.x[1],fpu.x13.x[2],fpu.x13.x[3],
			   fpu.x14.x[0],fpu.x14.x[1],fpu.x14.x[2],fpu.x14.x[3],
			   fpu.x15.x[0],fpu.x15.x[1],fpu.x15.x[2],fpu.x15.x[3],
			   fpu.x16.x[0],fpu.x16.x[1],fpu.x16.x[2],fpu.x16.x[3],
			   fpu.x17.x[0],fpu.x17.x[1],fpu.x17.x[2],fpu.x17.x[3],
			   fpu.x18.x[0],fpu.x18.x[1],fpu.x18.x[2],fpu.x18.x[3],
			   fpu.x19.x[0],fpu.x19.x[1],fpu.x19.x[2],fpu.x19.x[3],
			   fpu.x20.x[0],fpu.x20.x[1],fpu.x20.x[2],fpu.x20.x[3],
			   fpu.x21.x[0],fpu.x21.x[1],fpu.x21.x[2],fpu.x21.x[3],
			   fpu.x22.x[0],fpu.x22.x[1],fpu.x22.x[2],fpu.x22.x[3],
			   fpu.x23.x[0],fpu.x23.x[1],fpu.x23.x[2],fpu.x23.x[3],
			   fpu.x24.x[0],fpu.x24.x[1],fpu.x24.x[2],fpu.x24.x[3],
			   fpu.x25.x[0],fpu.x25.x[1],fpu.x25.x[2],fpu.x25.x[3],
			   fpu.x26.x[0],fpu.x26.x[1],fpu.x26.x[2],fpu.x26.x[3],
			   fpu.x27.x[0],fpu.x27.x[1],fpu.x27.x[2],fpu.x27.x[3],
			   fpu.x28.x[0],fpu.x28.x[1],fpu.x28.x[2],fpu.x28.x[3],
			   fpu.x29.x[0],fpu.x29.x[1],fpu.x29.x[2],fpu.x29.x[3],
			   fpu.x30.x[0],fpu.x30.x[1],fpu.x30.x[2],fpu.x30.x[3],
			   fpu.x31.x[0],fpu.x31.x[1],fpu.x31.x[2],fpu.x31.x[3]);
		    printf("      fpsr xmod %d afinv %d afdvz %d afunf %d "
			   "afovf %d afinx %d\n", fpu.fpsr.xmod, fpu.fpsr.afinv,
			   fpu.fpsr.afdvz, fpu.fpsr.afunf,
			   fpu.fpsr.afovf, fpu.fpsr.afinx);
		    printf("      fpcr rm ");
			switch(fpu.fpcr.rm){
			case M88K_RM_NEAREST:
			    printf("RM_NEAREST ");
			    break;
			case M88K_RM_ZERO:
			    printf("RM_ZERO ");
			    break;
			case M88K_RM_NEGINF:
			    printf("RM_NEGINF ");
			    break;
			case M88K_RM_POSINF:
			    printf("RM_POSINF ");
			    break;
			}
		    printf("efinv %d efdvz %d efunf %d efovf %d efinx %d\n",
			   fpu.fpcr.efinv, fpu.fpcr.efdvz, fpu.fpcr.efunf,
			   fpu.fpcr.efovf, fpu.fpcr.efinx);
		    break;
		case M88K_THREAD_STATE_USER:
		    printf("      flavor M88K_THREAD_STATE_USER\n");
		    if(count == M88K_THREAD_STATE_USER_COUNT)
			printf("      count M88K_THREAD_STATE_USER_COUNT\n");
		    else
			printf("      count %u (not M88K_THREAD_STATE_USER_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(m88k_thread_state_user_t)){
		        memcpy((char *)&user, begin,
			       sizeof(m88k_thread_state_user_t));
		        begin += sizeof(m88k_thread_state_user_t);
		    }
		    else{
		        memset((char *)&user, '\0',
			       sizeof(m88k_thread_state_user_t));
		        memcpy((char *)&user, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_m88k_thread_state_user_t(&user, host_byte_sex);
		    printf("      user 0x%08x\n", (unsigned int)user.user);
		    break;

		case M88110_THREAD_STATE_IMPL:
		    printf("      flavor M88110_THREAD_STATE_IMPL\n");
		    if(count == M88110_THREAD_STATE_IMPL_COUNT)
			printf("      count M88110_THREAD_STATE_IMPL_COUNT\n");
		    else
			printf("      count %u (not M88110_THREAD_STATE_IMPL_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(m88110_thread_state_impl_t)){
		        memcpy((char *)&spu, begin,
			       sizeof(m88110_thread_state_impl_t));
		        begin += sizeof(m88110_thread_state_impl_t);
		    }
		    else{
		        memset((char *)&spu, '\0',
			       sizeof(m88110_thread_state_impl_t));
		        memcpy((char *)&spu, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_m88110_thread_state_impl_t(&spu, host_byte_sex);
		    for(j = 0; j < M88110_N_DATA_BP; j++){
			printf("      data_bp[%u] addr 0x%08x\n",
			       j, (unsigned int)spu.data_bp[i].addr);
			printf("                  cntl rw %d rwm %d "
			       "addr_match ", spu.data_bp[j].ctrl.rw,
			       spu.data_bp[j].ctrl.rwm);
			switch(spu.data_bp[j].ctrl.addr_match){
			case M88110_MATCH_BYTE:
			    printf("MATCH_BYTE ");
			    break;
			case M88110_MATCH_SHORT:
			    printf("MATCH_SHORT ");
			    break;
			case M88110_MATCH_WORD:
			    printf("MATCH_WORD ");
			    break;
			case M88110_MATCH_DOUBLE:
			    printf("MATCH_DOUBLE ");
			    break;
			case M88110_MATCH_QUAD:
			    printf("MATCH_QUAD ");
			    break;
			case M88110_MATCH_32:
			    printf("MATCH_32 ");
			    break;
			case M88110_MATCH_64:
			    printf("MATCH_64 ");
			    break;
			case M88110_MATCH_128:
			    printf("MATCH_128 ");
			    break;
			case M88110_MATCH_256:
			    printf("MATCH_256 ");
			    break;
			case M88110_MATCH_512:
			    printf("MATCH_512 ");
			    break;
			case M88110_MATCH_1024:
			    printf("MATCH_1024 ");
			    break;
			case M88110_MATCH_2048:
			    printf("MATCH_2048 ");
			    break;
			case M88110_MATCH_4096:
			    printf("MATCH_4096 ");
			    break;
			default:
			    printf("%d (?)", spu.data_bp[j].ctrl.addr_match);
			    break;
			}
			printf("v %d\n", spu.data_bp[j].ctrl.v);
		    }
		    printf("      psr le %d se %d c %d sgn_imd %d sm %d "
			   "sfu1dis %d mxm_dis %d\n" , spu.psr.le, spu.psr.se,
			   spu.psr.c, spu.psr.sgn_imd, spu.psr.sm,
			   spu.psr.sfu1dis, spu.psr.mxm_dis);
		    break;

		default:
		    printf("     flavor %u (unknown)\n", flavor);
		    printf("      count %u\n", count);
		    printf("      state:\n");
		    print_unknown_state(begin, end, count, swapped);
		    begin += count * sizeof(uint32_t);
		    break;
		}
	    }
	}
	else if(cputype == CPU_TYPE_I860){
	    struct i860_thread_state_regs cpu;

	    while(begin < end){
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&flavor, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    flavor = 0;
		    begin = end;
		}
		if(swapped)
		    flavor = SWAP_INT(flavor);
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&count, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    count = 0;
		    begin = end;
		}
		if(swapped)
		    count = SWAP_INT(count);

		switch(flavor){
		case I860_THREAD_STATE_REGS:
		    printf("      flavor I860_THREAD_STATE_REGS\n");
		    if(count == I860_THREAD_STATE_REGS_COUNT)
			printf("      count I860_THREAD_STATE_REGS_COUNT\n");
		    else
			printf("      count %u (not I860_THREAD_STATE_REGS_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(struct i860_thread_state_regs)){
		        memcpy((char *)&cpu, begin,
			       sizeof(struct i860_thread_state_regs));
		        begin += sizeof(struct i860_thread_state_regs);
		    }
		    else{
		        memset((char *)&cpu, '\0',
			       sizeof(struct i860_thread_state_regs));
		        memcpy((char *)&cpu, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_i860_thread_state_regs(&cpu, host_byte_sex);
		    printf(" iregs\n");
		    for(j = 0 ; j < 31 ; j += k){
			for(k = 0 ; k < 5 && j + k < 31 ; k++)
			    printf(" i%-2u 0x%08x", j + k,
				   (unsigned int)cpu.ireg[j + k]);
			printf("\n");
		    }
		    printf(" fregs\n");
		    for(j = 0 ; j < 30 ; j += k){
			for(k = 0 ; k < 5 && j + k < 30 ; k++)
			    printf(" f%-2u 0x%08x", j + k,
				   (unsigned int)cpu.freg[j + k]);
			printf("\n");
		    }
		    printf(" psr 0x%08x epsr 0x%08x db 0x%08x pc 0x%08x\n",
			   (unsigned int)cpu.psr, (unsigned int)cpu.epsr,
			   (unsigned int)cpu.db, (unsigned int)cpu.pc);
		    printf(" Mres3 %e Ares3 %e\n", cpu.Mres3, cpu.Ares3);
		    printf(" Mres2 %e Ares2 %e\n", cpu.Mres2, cpu.Ares2);
		    printf(" Mres1 %e Ares1 %e\n", cpu.Mres1, cpu.Ares1);
		    printf(" Ires1 %e\n", cpu.Ires1);
		    printf(" Lres3m %e Lres2m %e Lres1m %e\n", cpu.Lres3m,
			   cpu.Lres2m, cpu.Lres1m);
		    printf(" KR %e KI %e T %e\n", cpu.KR, cpu.KI, cpu.T);
		    printf(" Fsr3 0x%08x Fsr2 0x%08x Fsr1 0x%08x\n",
			   (unsigned int)cpu.Fsr3, (unsigned int)cpu.Fsr2,
			   (unsigned int)cpu.Fsr1);
		    printf(" Mergelo32 0x%08x Mergehi32 0x%08x\n",
			   (unsigned int)cpu.Mergelo32,
			   (unsigned int)cpu.Mergehi32);
		    break;

		default:
		    printf("     flavor %u (unknown)\n", flavor);
		    printf("      count %u\n", count);
		    printf("      state:\n");
		    print_unknown_state(begin, end, count, swapped);
		    begin += count * sizeof(uint32_t);
		    break;
		}
	    }
	}
	else if(cputype == CPU_TYPE_I386 ||
	        cputype == CPU_TYPE_X86_64){
	    i386_thread_state_t cpu;
/* current i386 thread states */
#if i386_THREAD_STATE == 1
#ifndef i386_EXCEPTION_STATE_COUNT
	    char *fpu;
	    uint32_t fpu_size;
#else /* defined(i386_EXCEPTION_STATE_COUNT) */
	    i386_float_state_t fpu;
#endif /* defined(i386_EXCEPTION_STATE_COUNT) */
	    i386_exception_state_t exc;
	    uint32_t f, g;

#ifdef x86_THREAD_STATE64
	    x86_thread_state64_t cpu64;
	    x86_float_state64_t fpu64;
	    x86_exception_state64_t exc64;
	    x86_debug_state64_t debug64;
	    x86_debug_state32_t debug;
	    struct x86_thread_state ts;
	    struct x86_float_state fs;
	    struct x86_exception_state es;
	    struct x86_debug_state ds;
#endif /* x86_THREAD_STATE64 */

#endif /* i386_THREAD_STATE == 1 */

/* i386 thread states on older releases */
#if i386_THREAD_STATE == -1
	    i386_thread_fpstate_t fpu;
	    i386_thread_exceptstate_t exc;
	    i386_thread_cthreadstate_t user;
	    const char *tags[] = { "VALID", "ZERO", "SPEC", "EMPTY" };
#endif /* i386_THREAD_STATE == -1 */

	    while(begin < end){
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&flavor, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    flavor = 0;
		    begin = end;
		}
		if(swapped)
		    flavor = SWAP_INT(flavor);
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&count, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    count = 0;
		    begin = end;
		}
		if(swapped)
		    count = SWAP_INT(count);

		switch(flavor){
		case i386_THREAD_STATE:
		    printf("     flavor i386_THREAD_STATE\n");
		    if(count == i386_THREAD_STATE_COUNT)
			printf("      count i386_THREAD_STATE_COUNT\n");
		    else
			printf("      count %u (not i386_THREAD_STATE_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(i386_thread_state_t)){
		        memcpy((char *)&cpu, begin,
			       sizeof(i386_thread_state_t));
		        begin += sizeof(i386_thread_state_t);
		    }
		    else{
		        memset((char *)&cpu, '\0',
			       sizeof(i386_thread_state_t));
		        memcpy((char *)&cpu, begin, left);
		        begin += left;
		    }
#if defined(x86_THREAD_STATE64) && i386_THREAD_STATE != -1
print_x86_thread_state32:
#endif
		    if(swapped)
			swap_i386_thread_state(&cpu, host_byte_sex);
		    printf(
		       "\t    eax 0x%08x ebx    0x%08x ecx 0x%08x edx 0x%08x\n"
		       "\t    edi 0x%08x esi    0x%08x ebp 0x%08x esp 0x%08x\n"
		       "\t    ss  0x%08x eflags 0x%08x eip 0x%08x cs  0x%08x\n"
		       "\t    ds  0x%08x es     0x%08x fs  0x%08x gs  0x%08x\n",
			cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.edi, cpu.esi,
			cpu.ebp, cpu.esp, cpu.ss, cpu.eflags, cpu.eip, cpu.cs,
			cpu.ds, cpu.es, cpu.fs, cpu.gs);
		    break;

/* current i386 thread states */
#if i386_THREAD_STATE == 1
		case i386_FLOAT_STATE:
		    printf("     flavor i386_FLOAT_STATE\n");
		    if(count == i386_FLOAT_STATE_COUNT)
			printf("      count i386_FLOAT_STATE_COUNT\n");
		    else
			printf("      count %u (not i386_FLOAT_STATE_COUNT)\n",
			       count);
		    left = (uint32_t)(end - begin);
#ifndef i386_EXCEPTION_STATE_COUNT
		    fpu = begin;
		    if(left >= sizeof(struct i386_float_state)){
			fpu_size = sizeof(struct i386_float_state);
		        begin += sizeof(struct i386_float_state);
		    }
		    else{
			fpu_size = left;
		        begin += left;
		    }
		    printf("\t    i386_float_state:\n");
		    for(f = 0; f < fpu_size; /* no increment expr */){
			printf("\t    ");
			for(g = 0; g < 16 && f < fpu_size; g++){
			    printf("%02x ", (unsigned int)(fpu[f] & 0xff));
			    f++;
			}
			printf("\n");
		    }
#else /* defined(i386_EXCEPTION_STATE_COUNT) */
		    if(left >= sizeof(i386_float_state_t)){
		        memcpy((char *)&fpu, begin,
			       sizeof(i386_float_state_t));
		        begin += sizeof(i386_float_state_t);
		    }
		    else{
		        memset((char *)&fpu, '\0',
			       sizeof(i386_float_state_t));
		        memcpy((char *)&fpu, begin, left);
		        begin += left;
		    }
#ifdef x86_THREAD_STATE64
print_x86_float_state32:
#endif /* x86_THREAD_STATE64 */
		    if(swapped)
			swap_i386_float_state(&fpu, host_byte_sex);
		    printf("\t    fpu_reserved[0] %d fpu_reserved[1] %d\n",
			   fpu.fpu_reserved[0], fpu.fpu_reserved[1]);
		    printf("\t    control: invalid %d denorm %d zdiv %d ovrfl "
			   "%d undfl %d precis %d\n",
			   fpu.fpu_fcw.invalid,
			   fpu.fpu_fcw.denorm,
			   fpu.fpu_fcw.zdiv,
			   fpu.fpu_fcw.ovrfl,
			   fpu.fpu_fcw.undfl,
			   fpu.fpu_fcw.precis);
		    printf("\t\t     pc ");
		    switch(fpu.fpu_fcw.pc){
		    case FP_PREC_24B:
			printf("FP_PREC_24B ");
			break;
		    case FP_PREC_53B:
			printf("FP_PREC_53B ");
			break;
		    case FP_PREC_64B:
			printf("FP_PREC_64B ");
			break;
		    default:
			printf("%d ", fpu.fpu_fcw.pc);
			break;
		    }
		    printf("rc ");
		    switch(fpu.fpu_fcw.rc){
		    case FP_RND_NEAR:
			printf("FP_RND_NEAR ");
			break;
		    case FP_RND_DOWN:
			printf("FP_RND_DOWN ");
			break;
		    case FP_RND_UP:
			printf("FP_RND_UP ");
			break;
		    case FP_CHOP:
			printf("FP_CHOP ");
			break;
		    }
		    printf("\n");
		    printf("\t    status: invalid %d denorm %d zdiv %d ovrfl "
			   "%d undfl %d precis %d stkflt %d\n",
			   fpu.fpu_fsw.invalid,
			   fpu.fpu_fsw.denorm,
			   fpu.fpu_fsw.zdiv,
			   fpu.fpu_fsw.ovrfl,
			   fpu.fpu_fsw.undfl,
			   fpu.fpu_fsw.precis,
			   fpu.fpu_fsw.stkflt);
		    printf("\t            errsumm %d c0 %d c1 %d c2 %d tos %d "
			   "c3 %d busy %d\n",
			   fpu.fpu_fsw.errsumm,
			   fpu.fpu_fsw.c0,
			   fpu.fpu_fsw.c1,
			   fpu.fpu_fsw.c2,
			   fpu.fpu_fsw.tos,
			   fpu.fpu_fsw.c3,
			   fpu.fpu_fsw.busy);
		    printf("\t    fpu_ftw 0x%02x fpu_rsrv1 0x%02x fpu_fop "
			   "0x%04x fpu_ip 0x%08x\n",
			   (unsigned int)fpu.fpu_ftw,
			   (unsigned int)fpu.fpu_rsrv1,
			   (unsigned int)fpu.fpu_fop,
			   (unsigned int)fpu.fpu_ip);
		    printf("\t    fpu_cs 0x%04x fpu_rsrv2 0x%04x fpu_dp 0x%08x "
			   "fpu_ds 0x%04x\n",
			   (unsigned int)fpu.fpu_cs,
			   (unsigned int)fpu.fpu_rsrv2,
			   (unsigned int)fpu.fpu_dp,
			   (unsigned int)fpu.fpu_ds);
		    printf("\t    fpu_rsrv3 0x%04x fpu_mxcsr 0x%08x "
			   "fpu_mxcsrmask 0x%08x\n",
			   (unsigned int)fpu.fpu_rsrv3,
			   (unsigned int)fpu.fpu_mxcsr,
			   (unsigned int)fpu.fpu_mxcsrmask);
		    printf("\t    fpu_stmm0:\n");
		    print_mmst_reg(&fpu.fpu_stmm0);
		    printf("\t    fpu_stmm1:\n");
		    print_mmst_reg(&fpu.fpu_stmm1);
		    printf("\t    fpu_stmm2:\n");
		    print_mmst_reg(&fpu.fpu_stmm2);
		    printf("\t    fpu_stmm3:\n");
		    print_mmst_reg(&fpu.fpu_stmm3);
		    printf("\t    fpu_stmm4:\n");
		    print_mmst_reg(&fpu.fpu_stmm4);
		    printf("\t    fpu_stmm5:\n");
		    print_mmst_reg(&fpu.fpu_stmm5);
		    printf("\t    fpu_stmm6:\n");
		    print_mmst_reg(&fpu.fpu_stmm6);
		    printf("\t    fpu_stmm7:\n");
		    print_mmst_reg(&fpu.fpu_stmm7);
		    printf("\t    fpu_xmm0:\n");
		    print_xmm_reg(&fpu.fpu_xmm0);
		    printf("\t    fpu_xmm1:\n");
		    print_xmm_reg(&fpu.fpu_xmm1);
		    printf("\t    fpu_xmm2:\n");
		    print_xmm_reg(&fpu.fpu_xmm2);
		    printf("\t    fpu_xmm3:\n");
		    print_xmm_reg(&fpu.fpu_xmm3);
		    printf("\t    fpu_xmm4:\n");
		    print_xmm_reg(&fpu.fpu_xmm4);
		    printf("\t    fpu_xmm5:\n");
		    print_xmm_reg(&fpu.fpu_xmm5);
		    printf("\t    fpu_xmm6:\n");
		    print_xmm_reg(&fpu.fpu_xmm6);
		    printf("\t    fpu_xmm7:\n");
		    print_xmm_reg(&fpu.fpu_xmm7);
		    printf("\t    fpu_rsrv4:\n");
		    for(f = 0; f < 14; f++){
			printf("\t            ");
			for(g = 0; g < 16; g++){
			    printf("%02x ",
				   (unsigned int)(fpu.fpu_rsrv4[f*g] & 0xff));
			}
			printf("\n");
		    }
		    printf("\t    fpu_reserved1 0x%08x\n",
			   (unsigned int)fpu.fpu_reserved1);
#endif /* defined(i386_EXCEPTION_STATE_COUNT) */
		    break;

		case i386_EXCEPTION_STATE:
		    printf("     flavor i386_EXCEPTION_STATE\n");
		    if(count == I386_EXCEPTION_STATE_COUNT)
			printf("      count I386_EXCEPTION_STATE_COUNT\n");
		    else
			printf("      count %u (not I386_EXCEPTION_STATE_COUNT"
			       ")\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(i386_exception_state_t)){
		        memcpy((char *)&exc, begin,
			       sizeof(i386_exception_state_t));
		        begin += sizeof(i386_exception_state_t);
		    }
		    else{
		        memset((char *)&exc, '\0',
			       sizeof(i386_exception_state_t));
		        memcpy((char *)&exc, begin, left);
		        begin += left;
		    }
#ifdef x86_THREAD_STATE64
print_x86_exception_state32:
#endif /* x86_THREAD_STATE64 */
		    if(swapped)
			swap_i386_exception_state(&exc, host_byte_sex);
		    printf("\t    trapno 0x%08x err 0x%08x faultvaddr 0x%08x\n",
			   exc.trapno, exc.err, exc.faultvaddr);
		    break;

#ifdef x86_THREAD_STATE64
		case x86_DEBUG_STATE32:
		    printf("     flavor x86_DEBUG_STATE32\n");
		    if(count == x86_DEBUG_STATE32_COUNT)
			printf("      count x86_DEBUG_STATE32_COUNT\n");
		    else
			printf("      count %u (not x86_DEBUG_STATE32_COUNT"
			       ")\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(x86_debug_state32_t)){
		        memcpy((char *)&debug, begin,
			       sizeof(x86_debug_state32_t));
		        begin += sizeof(x86_debug_state32_t);
		    }
		    else{
		        memset((char *)&debug, '\0',
			       sizeof(x86_debug_state32_t));
		        memcpy((char *)&debug, begin, left);
		        begin += left;
		    }
print_x86_debug_state32:
		    if(swapped)
			swap_x86_debug_state32(&debug, host_byte_sex);
		    printf("\t    dr0 0x%08x dr1 0x%08x dr2 0x%08x dr3 "
			   "0x%08x\n", debug.dr0, debug.dr1, debug.dr2,
			   debug.dr3);
		    printf("\t    dr4 0x%08x dr5 0x%08x dr6 0x%08x dr7 "
			   "0x%08x\n", debug.dr4, debug.dr5, debug.dr6,
			   debug.dr7);
		    break;

		case x86_THREAD_STATE64:
		    printf("     flavor x86_THREAD_STATE64\n");
		    if(count == x86_THREAD_STATE64_COUNT)
			printf("      count x86_THREAD_STATE64_COUNT\n");
		    else
			printf("      count %u (not x86_THREAD_STATE64_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(x86_thread_state64_t)){
		        memcpy((char *)&cpu64, begin,
			       sizeof(x86_thread_state64_t));
		        begin += sizeof(x86_thread_state64_t);
		    }
		    else{
		        memset((char *)&cpu64, '\0',
			       sizeof(x86_thread_state64_t));
		        memcpy((char *)&cpu64, begin, left);
		        begin += left;
		    }
print_x86_thread_state64:
		    if(swapped)
			swap_x86_thread_state64(&cpu64, host_byte_sex);

		    printf("   rax  0x%016llx rbx 0x%016llx rcx  0x%016llx\n"
			   "   rdx  0x%016llx rdi 0x%016llx rsi  0x%016llx\n"
			   "   rbp  0x%016llx rsp 0x%016llx r8   0x%016llx\n"
			   "    r9  0x%016llx r10 0x%016llx r11  0x%016llx\n"
			   "   r12  0x%016llx r13 0x%016llx r14  0x%016llx\n"
			   "   r15  0x%016llx rip 0x%016llx\n"
			   "rflags  0x%016llx cs  0x%016llx fs   0x%016llx\n"
			   "    gs  0x%016llx\n",
                        cpu64.rax, cpu64.rbx, cpu64.rcx, cpu64.rdx, cpu64.rdi,
			cpu64.rsi, cpu64.rbp, cpu64.rsp, cpu64.r8, cpu64.r9,
			cpu64.r10, cpu64.r11, cpu64.r12, cpu64.r13, cpu64.r14,
			cpu64.r15, cpu64.rip, cpu64.rflags, cpu64.cs, cpu64.fs,
			cpu64.gs);
		    break;

		case x86_FLOAT_STATE64:
		    printf("     flavor x86_FLOAT_STATE64\n");
		    if(count == x86_FLOAT_STATE64_COUNT)
			printf("      count x86_FLOAT_STATE64_COUNT\n");
		    else
			printf("      count %u (not x86_FLOAT_STATE64_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(x86_float_state64_t)){
		        memcpy((char *)&fpu64, begin,
			       sizeof(x86_float_state64_t));
		        begin += sizeof(x86_float_state64_t);
		    }
		    else{
		        memset((char *)&fpu64, '\0',
			       sizeof(x86_float_state64_t));
		        memcpy((char *)&fpu64, begin, left);
		        begin += left;
		    }
print_x86_float_state64:
		    if(swapped)
			swap_x86_float_state64(&fpu64, host_byte_sex);
		    printf("\t    fpu_reserved[0] %d fpu_reserved[1] %d\n",
			   fpu64.fpu_reserved[0], fpu64.fpu_reserved[1]);
		    printf("\t    control: invalid %d denorm %d zdiv %d ovrfl "
			   "%d undfl %d precis %d\n",
			   fpu64.fpu_fcw.invalid,
			   fpu64.fpu_fcw.denorm,
			   fpu64.fpu_fcw.zdiv,
			   fpu64.fpu_fcw.ovrfl,
			   fpu64.fpu_fcw.undfl,
			   fpu64.fpu_fcw.precis);
		    printf("\t\t     pc ");
		    switch(fpu64.fpu_fcw.pc){
		    case FP_PREC_24B:
			printf("FP_PREC_24B ");
			break;
		    case FP_PREC_53B:
			printf("FP_PREC_53B ");
			break;
		    case FP_PREC_64B:
			printf("FP_PREC_64B ");
			break;
		    default:
			printf("%d ", fpu64.fpu_fcw.pc);
			break;
		    }
		    printf("rc ");
		    switch(fpu64.fpu_fcw.rc){
		    case FP_RND_NEAR:
			printf("FP_RND_NEAR ");
			break;
		    case FP_RND_DOWN:
			printf("FP_RND_DOWN ");
			break;
		    case FP_RND_UP:
			printf("FP_RND_UP ");
			break;
		    case FP_CHOP:
			printf("FP_CHOP ");
			break;
		    }
		    printf("\n");
		    printf("\t    status: invalid %d denorm %d zdiv %d ovrfl "
			   "%d undfl %d precis %d stkflt %d\n",
			   fpu64.fpu_fsw.invalid,
			   fpu64.fpu_fsw.denorm,
			   fpu64.fpu_fsw.zdiv,
			   fpu64.fpu_fsw.ovrfl,
			   fpu64.fpu_fsw.undfl,
			   fpu64.fpu_fsw.precis,
			   fpu64.fpu_fsw.stkflt);
		    printf("\t            errsumm %d c0 %d c1 %d c2 %d tos %d "
			   "c3 %d busy %d\n",
			   fpu64.fpu_fsw.errsumm,
			   fpu64.fpu_fsw.c0,
			   fpu64.fpu_fsw.c1,
			   fpu64.fpu_fsw.c2,
			   fpu64.fpu_fsw.tos,
			   fpu64.fpu_fsw.c3,
			   fpu64.fpu_fsw.busy);
		    printf("\t    fpu_ftw 0x%02x fpu_rsrv1 0x%02x fpu_fop "
			   "0x%04x fpu_ip 0x%08x\n",
			   (unsigned int)fpu64.fpu_ftw,
			   (unsigned int)fpu64.fpu_rsrv1,
			   (unsigned int)fpu64.fpu_fop,
			   (unsigned int)fpu64.fpu_ip);
		    printf("\t    fpu_cs 0x%04x fpu_rsrv2 0x%04x fpu_dp 0x%08x "
			   "fpu_ds 0x%04x\n",
			   (unsigned int)fpu64.fpu_cs,
			   (unsigned int)fpu64.fpu_rsrv2,
			   (unsigned int)fpu64.fpu_dp,
			   (unsigned int)fpu64.fpu_ds);
		    printf("\t    fpu_rsrv3 0x%04x fpu_mxcsr 0x%08x "
			   "fpu_mxcsrmask 0x%08x\n",
			   (unsigned int)fpu64.fpu_rsrv3,
			   (unsigned int)fpu64.fpu_mxcsr,
			   (unsigned int)fpu64.fpu_mxcsrmask);
		    printf("\t    fpu_stmm0:\n");
		    print_mmst_reg(&fpu64.fpu_stmm0);
		    printf("\t    fpu_stmm1:\n");
		    print_mmst_reg(&fpu64.fpu_stmm1);
		    printf("\t    fpu_stmm2:\n");
		    print_mmst_reg(&fpu64.fpu_stmm2);
		    printf("\t    fpu_stmm3:\n");
		    print_mmst_reg(&fpu64.fpu_stmm3);
		    printf("\t    fpu_stmm4:\n");
		    print_mmst_reg(&fpu64.fpu_stmm4);
		    printf("\t    fpu_stmm5:\n");
		    print_mmst_reg(&fpu64.fpu_stmm5);
		    printf("\t    fpu_stmm6:\n");
		    print_mmst_reg(&fpu64.fpu_stmm6);
		    printf("\t    fpu_stmm7:\n");
		    print_mmst_reg(&fpu64.fpu_stmm7);
		    printf("\t    fpu_xmm0:\n");
		    print_xmm_reg(&fpu64.fpu_xmm0);
		    printf("\t    fpu_xmm1:\n");
		    print_xmm_reg(&fpu64.fpu_xmm1);
		    printf("\t    fpu_xmm2:\n");
		    print_xmm_reg(&fpu64.fpu_xmm2);
		    printf("\t    fpu_xmm3:\n");
		    print_xmm_reg(&fpu64.fpu_xmm3);
		    printf("\t    fpu_xmm4:\n");
		    print_xmm_reg(&fpu64.fpu_xmm4);
		    printf("\t    fpu_xmm5:\n");
		    print_xmm_reg(&fpu64.fpu_xmm5);
		    printf("\t    fpu_xmm6:\n");
		    print_xmm_reg(&fpu64.fpu_xmm6);
		    printf("\t    fpu_xmm7:\n");
		    print_xmm_reg(&fpu64.fpu_xmm7);
		    printf("\t    fpu_xmm8:\n");
		    print_xmm_reg(&fpu64.fpu_xmm8);
		    printf("\t    fpu_xmm9:\n");
		    print_xmm_reg(&fpu64.fpu_xmm9);
		    printf("\t    fpu_xmm10:\n");
		    print_xmm_reg(&fpu64.fpu_xmm10);
		    printf("\t    fpu_xmm11:\n");
		    print_xmm_reg(&fpu64.fpu_xmm11);
		    printf("\t    fpu_xmm12:\n");
		    print_xmm_reg(&fpu64.fpu_xmm12);
		    printf("\t    fpu_xmm13:\n");
		    print_xmm_reg(&fpu64.fpu_xmm13);
		    printf("\t    fpu_xmm14:\n");
		    print_xmm_reg(&fpu64.fpu_xmm14);
		    printf("\t    fpu_xmm15:\n");
		    print_xmm_reg(&fpu64.fpu_xmm15);
		    printf("\t    fpu_rsrv4:\n");
		    for(f = 0; f < 6; f++){
			printf("\t            ");
			for(g = 0; g < 16; g++){
			    printf("%02x ",
				   (unsigned int)(fpu64.fpu_rsrv4[f*g] & 0xff));
			}
			printf("\n");
		    }
		    printf("\t    fpu_reserved1 0x%08x\n",
			   (unsigned int)fpu64.fpu_reserved1);
		    break;

		case x86_EXCEPTION_STATE64:
		    printf("     flavor x86_EXCEPTION_STATE64\n");
		    if(count == x86_EXCEPTION_STATE64_COUNT)
			printf("      count x86_EXCEPTION_STATE64_COUNT\n");
		    else
			printf("      count %u (not x86_EXCEPTION_STATE64_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(x86_exception_state64_t)){
		        memcpy((char *)&exc64, begin,
			       sizeof(x86_exception_state64_t));
		        begin += sizeof(x86_exception_state64_t);
		    }
		    else{
		        memset((char *)&exc64, '\0',
			       sizeof(x86_exception_state64_t));
		        memcpy((char *)&exc64, begin, left);
		        begin += left;
		    }
print_x86_exception_state64:
		    if(swapped)
			swap_x86_exception_state64(&exc64, host_byte_sex);
		    printf("\t    trapno 0x%08x err 0x%08x faultvaddr "
			   "0x%016llx\n", exc64.trapno, exc64.err,
			   exc64.faultvaddr);
		    break;

		case x86_DEBUG_STATE64:
		    printf("     flavor x86_DEBUG_STATE64\n");
		    if(count == x86_DEBUG_STATE64_COUNT)
			printf("      count x86_DEBUG_STATE64_COUNT\n");
		    else
			printf("      count %u (not x86_DEBUG_STATE64_COUNT"
			       ")\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(x86_debug_state64_t)){
		        memcpy((char *)&debug64, begin,
			       sizeof(x86_debug_state32_t));
		        begin += sizeof(x86_debug_state32_t);
		    }
		    else{
		        memset((char *)&debug64, '\0',
			       sizeof(x86_debug_state64_t));
		        memcpy((char *)&debug64, begin, left);
		        begin += left;
		    }
print_x86_debug_state64:
		    if(swapped)
			swap_x86_debug_state64(&debug64, host_byte_sex);
		    printf("\t    dr0 0x%016llx dr1 0x%016llx dr2 0x%016llx "
			   "dr3 0x%016llx\n", debug64.dr0, debug64.dr1,
			   debug64.dr2, debug64.dr3);
		    printf("\t    dr4 0x%016llx dr5 0x%016llx dr6 0x%016llx "
			   "dr7 0x%016llx\n", debug64.dr4, debug64.dr5,
			   debug64.dr6, debug64.dr7);
		    break;

		case x86_THREAD_STATE:
		    printf("     flavor x86_THREAD_STATE\n");
		    if(count == x86_THREAD_STATE_COUNT)
			printf("      count x86_THREAD_STATE_COUNT\n");
		    else
			printf("      count %u (not x86_THREAD_STATE_COUNT)\n",
			       count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(x86_thread_state_t)){
		        memcpy((char *)&ts, begin,
			       sizeof(x86_thread_state_t));
		        begin += sizeof(x86_thread_state_t);
		    }
		    else{
		        memset((char *)&ts, '\0',
			       sizeof(x86_thread_state_t));
		        memcpy((char *)&ts, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_x86_state_hdr(&ts.tsh, host_byte_sex);
		    if(ts.tsh.flavor == x86_THREAD_STATE32){
			printf("\t    tsh.flavor x86_THREAD_STATE32 ");
			if(ts.tsh.count == x86_THREAD_STATE32_COUNT)
			    printf("tsh.count x86_THREAD_STATE32_COUNT\n");
			else
			    printf("tsh.count %d (not x86_THREAD_STATE32_"
				   "COUNT\n", ts.tsh.count);
			cpu = ts.uts.ts32;
			goto print_x86_thread_state32;
		    }
		    else if(ts.tsh.flavor == x86_THREAD_STATE64){
			printf("\t    tsh.flavor x86_THREAD_STATE64 ");
			if(ts.tsh.count == x86_THREAD_STATE64_COUNT)
			    printf("tsh.count x86_THREAD_STATE64_COUNT\n");
			else
			    printf("tsh.count %d (not x86_THREAD_STATE64_"
				   "COUNT\n", ts.tsh.count);
			cpu64 = ts.uts.ts64;
			goto print_x86_thread_state64;
		    }
		    else{
			printf("\t    tsh.flavor %d tsh.count %d\n",
			       ts.tsh.flavor, ts.tsh.count);
		    }
		    break;

		case x86_FLOAT_STATE:
		    printf("     flavor x86_FLOAT_STATE\n");
		    if(count == x86_FLOAT_STATE_COUNT)
			printf("      count x86_FLOAT_STATE_COUNT\n");
		    else
			printf("      count %u (not x86_FLOAT_STATE_COUNT)\n",
			       count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(x86_float_state_t)){
		        memcpy((char *)&fs, begin,
			       sizeof(x86_float_state_t));
		        begin += sizeof(x86_float_state_t);
		    }
		    else{
		        memset((char *)&fs, '\0',
			       sizeof(x86_float_state_t));
		        memcpy((char *)&fs, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_x86_state_hdr(&fs.fsh, host_byte_sex);
		    if(fs.fsh.flavor == x86_FLOAT_STATE32){
			printf("\t    fsh.flavor x86_FLOAT_STATE32 ");
			if(fs.fsh.count == x86_FLOAT_STATE32_COUNT)
			    printf("tsh.count x86_FLOAT_STATE32_COUNT\n");
			else
			    printf("tsh.count %d (not x86_FLOAT_STATE32_COUNT"
				   "\n", fs.fsh.count);
			fpu = fs.ufs.fs32;
			goto print_x86_float_state32;
		    }
		    else if(fs.fsh.flavor == x86_FLOAT_STATE64){
			printf("\t    fsh.flavor x86_FLOAT_STATE64 ");
			if(fs.fsh.count == x86_FLOAT_STATE64_COUNT)
			    printf("fsh.count x86_FLOAT_STATE64_COUNT\n");
			else
			    printf("fsh.count %d (not x86_FLOAT_STATE64_COUNT"
				   "\n", fs.fsh.count);
			fpu64 = fs.ufs.fs64;
			goto print_x86_float_state64;
		    }
		    else{
			printf("\t    fsh.flavor %d fsh.count %d\n",
			       fs.fsh.flavor, fs.fsh.count);
		    }
		    break;

		case x86_EXCEPTION_STATE:
		    printf("     flavor x86_EXCEPTION_STATE\n");
		    if(count == x86_EXCEPTION_STATE_COUNT)
			printf("      count x86_EXCEPTION_STATE_COUNT\n");
		    else
			printf("      count %u (not x86_EXCEPTION_STATE_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(x86_exception_state_t)){
		        memcpy((char *)&es, begin,
			       sizeof(x86_exception_state_t));
		        begin += sizeof(x86_exception_state_t);
		    }
		    else{
		        memset((char *)&es, '\0',
			       sizeof(x86_exception_state_t));
		        memcpy((char *)&es, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_x86_state_hdr(&es.esh, host_byte_sex);
		    if(es.esh.flavor == x86_EXCEPTION_STATE32){
			printf("\t    esh.flavor x86_EXCEPTION_STATE32\n");
			if(es.esh.count == x86_EXCEPTION_STATE32_COUNT)
			    printf("\t    esh.count x86_EXCEPTION_STATE32_"
				   "COUNT\n");
			else
			    printf("\t    esh.count %d (not x86_EXCEPTION_"
				   "STATE32_COUNT\n", es.esh.count);
			exc = es.ues.es32;
			goto print_x86_exception_state32;
		    }
		    else if(es.esh.flavor == x86_EXCEPTION_STATE64){
			printf("\t    esh.flavor x86_EXCEPTION_STATE64\n");
			if(es.esh.count == x86_EXCEPTION_STATE64_COUNT)
			    printf("\t    esh.count x86_EXCEPTION_STATE64_"
				   "COUNT\n");
			else
			    printf("\t    esh.count %d (not x86_EXCEPTION_"
				   "STATE64_COUNT\n", es.esh.count);
			exc64 = es.ues.es64;
			goto print_x86_exception_state64;
		    }
		    else{
			printf("\t    esh.flavor %d esh.count %d\n",
			       es.esh.flavor, es.esh.count);
		    }
		    break;

		case x86_DEBUG_STATE:
		    printf("     flavor x86_DEBUG_STATE\n");
		    if(count == x86_DEBUG_STATE_COUNT)
			printf("      count x86_DEBUG_STATE_COUNT\n");
		    else
			printf("      count %u (not x86_DEBUG_STATE_COUNT"
			       "\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(x86_debug_state_t)){
		        memcpy((char *)&ds, begin,
			       sizeof(x86_debug_state_t));
		        begin += sizeof(x86_debug_state_t);
		    }
		    else{
		        memset((char *)&ds, '\0',
			       sizeof(x86_debug_state_t));
		        memcpy((char *)&ds, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_x86_state_hdr(&ds.dsh, host_byte_sex);
		    if(ds.dsh.flavor == x86_DEBUG_STATE32){
			printf("\t    dsh.flavor x86_DEBUG_STATE32\n");
			if(ds.dsh.count == x86_DEBUG_STATE32_COUNT)
			    printf("\t    dsh.count x86_DEBUG_STATE32_COUNT\n");
			else
			    printf("\t    esh.count %d (not x86_DEBUG_STATE32_"
				   "_COUNT\n", ds.dsh.count);
			debug = ds.uds.ds32;
			goto print_x86_debug_state32;
		    }
		    if(ds.dsh.flavor == x86_DEBUG_STATE64){
			printf("\t    dsh.flavor x86_DEBUG_STATE64\n");
			if(ds.dsh.count == x86_DEBUG_STATE64_COUNT)
			    printf("\t    dsh.count x86_DEBUG_STATE64_COUNT\n");
			else
			    printf("\t    esh.count %d (not x86_DEBUG_STATE64_"
				   "_COUNT\n", ds.dsh.count);
			debug64 = ds.uds.ds64;
			goto print_x86_debug_state64;
		    }
		    else{
			printf("\t    dsh.flavor %d dsh.count %d\n",
			       ds.dsh.flavor, ds.dsh.count);
		    }
		    break;
#endif /* x86_THREAD_STATE64 */
#endif /* i386_THREAD_STATE == 1 */

/* i386 thread states on older releases */
#if i386_THREAD_STATE == -1
		case i386_THREAD_FPSTATE:
		    printf("     flavor i386_THREAD_FPSTATE\n");
		    if(count == i386_THREAD_FPSTATE_COUNT)
			printf("      count i386_THREAD_FPSTATE_COUNT\n");
		    else
			printf("      count %u (not i386_THREAD_FPSTATE_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(i386_thread_fpstate_t)){
		        memcpy((char *)&fpu, begin,
			       sizeof(i386_thread_fpstate_t));
		        begin += sizeof(i386_thread_fpstate_t);
		    }
		    else{
		        memset((char *)&fpu, '\0',
			       sizeof(i386_thread_fpstate_t));
		        memcpy((char *)&fpu, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_i386_thread_fpstate(&fpu, host_byte_sex);
		    printf("\t    control: invalid %d denorm %d zdiv %d ovrfl "
			   "%d undfl %d precis %d\n",
			   fpu.environ.control.invalid,
			   fpu.environ.control.denorm,
			   fpu.environ.control.zdiv,
			   fpu.environ.control.ovrfl,
			   fpu.environ.control.undfl,
			   fpu.environ.control.precis);
		    printf("\t\t     pc ");
		    switch(fpu.environ.control.pc){
		    case FP_PREC_24B:
			printf("FP_PREC_24B ");
			break;
		    case FP_PREC_53B:
			printf("FP_PREC_53B ");
			break;
		    case FP_PREC_64B:
			printf("FP_PREC_64B ");
			break;
		    default:
			printf("%d ", fpu.environ.control.pc);
			break;
		    }
		    printf("rc ");
		    switch(fpu.environ.control.rc){
		    case FP_RND_NEAR:
			printf("FP_RND_NEAR ");
			break;
		    case FP_RND_DOWN:
			printf("FP_RND_DOWN ");
			break;
		    case FP_RND_UP:
			printf("FP_RND_UP ");
			break;
		    case FP_CHOP:
			printf("FP_CHOP ");
			break;
		    }
		    printf("\n");

		    printf("\t    status: invalid %d denorm %d zdiv %d ovrfl "
			   "%d undfl %d precis %d stkflt %d\n",
			   fpu.environ.status.invalid,
			   fpu.environ.status.denorm,
			   fpu.environ.status.zdiv,
			   fpu.environ.status.ovrfl,
			   fpu.environ.status.undfl,
			   fpu.environ.status.precis,
			   fpu.environ.status.stkflt);
		    printf("\t\t    errsumm %d c0 %d c1 %d c2 %d tos %d c3 %d "
			   "busy %d\n", fpu.environ.status.errsumm,
			   fpu.environ.status.c0, fpu.environ.status.c1,
			   fpu.environ.status.c2, fpu.environ.status.tos,
			   fpu.environ.status.c3, fpu.environ.status.busy);
		    printf("\t    tags: tag0 %s tag1 %s tag2 %s tag3 %s\n"
			   "\t          tag4 %s tag5 %s tag6 %s tag7 %s\n",
			   tags[fpu.environ.tag.tag0],
			   tags[fpu.environ.tag.tag1],
			   tags[fpu.environ.tag.tag2],
			   tags[fpu.environ.tag.tag3],
			   tags[fpu.environ.tag.tag4],
			   tags[fpu.environ.tag.tag5],
			   tags[fpu.environ.tag.tag6],
			   tags[fpu.environ.tag.tag7]);
		    printf("\t    ip 0x%08x\n", fpu.environ.ip);
		    printf("\t    cs: rpl ");
		    switch(fpu.environ.cs.rpl){
		    case KERN_PRIV:
			printf("KERN_PRIV ");
			break;
		    case USER_PRIV:
			printf("USER_PRIV ");
			break;
		    default:
			printf("%d ", fpu.environ.cs.rpl);
			break;
		    }
		    printf("ti ");
		    switch(fpu.environ.cs.ti){
		    case SEL_GDT:
			printf("SEL_GDT ");
			break;
		    case SEL_LDT:
			printf("SEL_LDT ");
			break;
		    }
		    printf("index %d\n", fpu.environ.cs.index);
		    printf("\t    opcode 0x%04x\n",
			   (unsigned int)fpu.environ.opcode);
		    printf("\t    dp 0x%08x\n", fpu.environ.dp);
		    printf("\t    ds: rpl ");
		    switch(fpu.environ.ds.rpl){
		    case KERN_PRIV:
			printf("KERN_PRIV ");
			break;
		    case USER_PRIV:
			printf("USER_PRIV ");
			break;
		    default:
			printf("%d ", fpu.environ.ds.rpl);
			break;
		    }
		    printf("ti ");
		    switch(fpu.environ.ds.ti){
		    case SEL_GDT:
			printf("SEL_GDT ");
			break;
		    case SEL_LDT:
			printf("SEL_LDT ");
			break;
		    }
		    printf("index %d\n", fpu.environ.ds.index);
		    printf("\t    stack:\n");
		    for(i = 0; i < 8; i++){
			printf("\t\tST[%u] mant 0x%04x 0x%04x 0x%04x 0x%04x "
			       "exp 0x%04x sign %d\n", i,
			       (unsigned int)fpu.stack.ST[i].mant,
			       (unsigned int)fpu.stack.ST[i].mant1,
			       (unsigned int)fpu.stack.ST[i].mant2,
			       (unsigned int)fpu.stack.ST[i].mant3,
			       (unsigned int)fpu.stack.ST[i].exp,
			       fpu.stack.ST[i].sign);
		    }
		    break;
		case i386_THREAD_EXCEPTSTATE:
		    printf("     flavor i386_THREAD_EXCEPTSTATE\n");
		    if(count == i386_THREAD_EXCEPTSTATE_COUNT)
			printf("      count i386_THREAD_EXCEPTSTATE_COUNT\n");
		    else
			printf("      count %u (not i386_THREAD_EXCEPTSTATE_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(i386_thread_exceptstate_t)){
		        memcpy((char *)&exc, begin,
			       sizeof(i386_thread_exceptstate_t));
		        begin += sizeof(i386_thread_exceptstate_t);
		    }
		    else{
		        memset((char *)&exc, '\0',
			       sizeof(i386_thread_exceptstate_t));
		        memcpy((char *)&exc, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_i386_thread_exceptstate(&exc, host_byte_sex);
		    printf("\t    trapno 0x%08x\n", exc.trapno);
		    if(exc.trapno == 14){
			printf("\t    err.pgfault: prot %d wrtflt %d user %d\n",
			       exc.err.pgfault.prot, exc.err.pgfault.wrtflt,
			       exc.err.pgfault.user);
		    }
		    else{
			printf("\t    err.normal: ext %d ", exc.err.normal.ext);
			printf("tbl ");
			switch(exc.err.normal.tbl){
		        case ERR_GDT:
			    printf("ERR_GDT ");
			    break;
		        case ERR_IDT:
			    printf("ERR_IDT ");
			    break;
		        case ERR_LDT:
			    printf("ERR_LDT ");
			    break;
			default:
			    printf("%d ", exc.err.normal.tbl);
			    break;
			}
			printf("index %d\n", exc.err.normal.index);
		    }
		    break;

		case i386_THREAD_CTHREADSTATE:
		    printf("     flavor i386_THREAD_CTHREADSTATE\n");
		    if(count == i386_THREAD_CTHREADSTATE_COUNT)
			printf("      count i386_THREAD_CTHREADSTATE_COUNT\n");
		    else
			printf("      count %u (not i386_THREAD_CTHREADSTATE_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(i386_thread_cthreadstate_t)){
		        memcpy((char *)&user, begin,
			       sizeof(i386_thread_cthreadstate_t));
		        begin += sizeof(i386_thread_cthreadstate_t);
		    }
		    else{
		        memset((char *)&user, '\0',
			       sizeof(i386_thread_cthreadstate_t));
		        memcpy((char *)&user, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_i386_thread_cthreadstate(&user, host_byte_sex);
		    printf("\t    self 0x%08x\n", user.self);
		    break;
#endif /* i386_THREAD_STATE == -1 */
		default:
		    printf("     flavor %u (unknown)\n", flavor);
		    printf("      count %u\n", count);
		    printf("      state:\n");
		    print_unknown_state(begin, end, count, swapped);
		    begin += count * sizeof(uint32_t);
		    break;
		}
	    }
	}
	else if(cputype == CPU_TYPE_ARM){
	    arm_thread_state_t cpu;
	    while(begin < end){
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&flavor, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    flavor = 0;
		    begin = end;
		}
		if(swapped)
		    flavor = SWAP_INT(flavor);
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&count, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    count = 0;
		    begin = end;
		}
		if(swapped)
		    count = SWAP_INT(count);

		switch(flavor){
		case ARM_THREAD_STATE:
		    printf("     flavor ARM_THREAD_STATE\n");
		    if(count == ARM_THREAD_STATE_COUNT)
			printf("      count ARM_THREAD_STATE_COUNT\n");
		    else
			printf("      count %u (not ARM_THREAD_STATE_"
			       "COUNT)\n", count);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(arm_thread_state_t)){
		        memcpy((char *)&cpu, begin,
			       sizeof(arm_thread_state_t));
		        begin += sizeof(arm_thread_state_t);
		    }
		    else{
		        memset((char *)&cpu, '\0',
			       sizeof(arm_thread_state_t));
		        memcpy((char *)&cpu, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_arm_thread_state_t(&cpu, host_byte_sex);
		    printf(
		       "\t    r0  0x%08x r1     0x%08x r2  0x%08x r3  0x%08x\n"
		       "\t    r4  0x%08x r5     0x%08x r6  0x%08x r7  0x%08x\n"
		       "\t    r8  0x%08x r9     0x%08x r10 0x%08x r11 0x%08x\n"
		       "\t    r12 0x%08x sp     0x%08x lr  0x%08x pc  0x%08x\n"
		       "\t   cpsr 0x%08x\n",
			cpu.__r[0], cpu.__r[1], cpu.__r[2], cpu.__r[3],
			cpu.__r[4], cpu.__r[5], cpu.__r[6], cpu.__r[7],
			cpu.__r[8], cpu.__r[9], cpu.__r[10], cpu.__r[11],
			cpu.__r[12], cpu.__sp, cpu.__lr, cpu.__pc, cpu.__cpsr);
		    break;
		default:
		    printf("     flavor %u (unknown)\n", flavor);
		    printf("      count %u\n", count);
		    printf("      state:\n");
		    print_unknown_state(begin, end, count, swapped);
		    begin += count * sizeof(uint32_t);
		    break;
		}
	    }
	}
	else if(cputype == CPU_TYPE_ARM64 || cputype == CPU_TYPE_ARM64_32){
	    arm_thread_state64_t cpu;
	    arm_exception_state64_t except;
	    while(begin < end){
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&flavor, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    flavor = 0;
		    begin = end;
		}
		if(swapped)
		    flavor = SWAP_INT(flavor);
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&count, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    count = 0;
		    begin = end;
		}
		if(swapped)
		    count = SWAP_INT(count);

		switch(flavor){
		case 1:
		case ARM_THREAD_STATE64:
		    if(flavor == 1)
		        printf("     flavor 1 (not ARM_THREAD_STATE64 %u)\n",
			       ARM_THREAD_STATE64);
		    else
		        printf("     flavor ARM_THREAD_STATE64\n");
		    if(count == ARM_THREAD_STATE64_COUNT)
			printf("      count ARM_THREAD_STATE64_COUNT\n");
		    else
			printf("      count %u (not ARM_THREAD_STATE64_"
			       "COUNT %u)\n", count, ARM_THREAD_STATE64_COUNT);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(arm_thread_state64_t)){
		        memcpy((char *)&cpu, begin,
			       sizeof(arm_thread_state64_t));
		        begin += sizeof(arm_thread_state64_t);
		    }
		    else{
		        memset((char *)&cpu, '\0',
			       sizeof(arm_thread_state64_t));
		        memcpy((char *)&cpu, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_arm_thread_state64_t(&cpu, host_byte_sex);
		    printf(
		       "\t    x0  0x%016llx x1  0x%016llx x2  0x%016llx\n"
		       "\t    x3  0x%016llx x4  0x%016llx x5  0x%016llx\n"
		       "\t    x6  0x%016llx x7  0x%016llx x8  0x%016llx\n"
		       "\t    x9  0x%016llx x10 0x%016llx x11 0x%016llx\n"
		       "\t    x12 0x%016llx x13 0x%016llx x14 0x%016llx\n"
		       "\t    x15 0x%016llx x16 0x%016llx x17 0x%016llx\n"
		       "\t    x18 0x%016llx x19 0x%016llx x20 0x%016llx\n"
		       "\t    x21 0x%016llx x22 0x%016llx x23 0x%016llx\n"
		       "\t    x24 0x%016llx x25 0x%016llx x26 0x%016llx\n"
		       "\t    x27 0x%016llx x28 0x%016llx  fp 0x%016llx\n"
		       "\t     lr 0x%016llx sp  0x%016llx  pc 0x%016llx\n"
		       "\t   cpsr 0x%08x\n",
			cpu.__x[0], cpu.__x[1], cpu.__x[2], cpu.__x[3],
			cpu.__x[4], cpu.__x[5], cpu.__x[6], cpu.__x[7],
			cpu.__x[8], cpu.__x[9], cpu.__x[10], cpu.__x[11],
			cpu.__x[12], cpu.__x[13], cpu.__x[14], cpu.__x[15],
			cpu.__x[16], cpu.__x[17], cpu.__x[18], cpu.__x[19],
			cpu.__x[20], cpu.__x[21], cpu.__x[22], cpu.__x[23],
			cpu.__x[24], cpu.__x[25], cpu.__x[26], cpu.__x[27],
			cpu.__x[28], cpu.__fp, cpu.__lr, cpu.__sp, cpu.__pc,
			cpu.__cpsr);
		    break;
		case ARM_EXCEPTION_STATE64:
		    printf("     flavor ARM_EXCEPTION_STATE64\n");
		    if(count == ARM_EXCEPTION_STATE64_COUNT)
			printf("      count ARM_EXCEPTION_STATE64_COUNT\n");
		    else
			printf("      count %u (not ARM_EXCEPTION_STATE64_COUNT"
			       " %u)\n", count, ARM_EXCEPTION_STATE64_COUNT);
		    left = (uint32_t)(end - begin);
		    if(left >= sizeof(arm_exception_state64_t)){
		        memcpy((char *)&except, begin,
			       sizeof(arm_exception_state64_t));
		        begin += sizeof(arm_exception_state64_t);
		    }
		    else{
		        memset((char *)&except, '\0',
			       sizeof(arm_exception_state64_t));
		        memcpy((char *)&except, begin, left);
		        begin += left;
		    }
		    if(swapped)
			swap_arm_exception_state64_t(&except, host_byte_sex);
		    printf(
		       "\t   far  0x%016llx esr 0x%08x exception 0x%08x\n",
			except.__far, except.__esr, except.__exception);
		    break;
		default:
		    printf("     flavor %u (unknown)\n", flavor);
		    printf("      count %u\n", count);
		    printf("      state:\n");
		    print_unknown_state(begin, end, count, swapped);
		    begin += count * sizeof(uint32_t);
		    break;
		}
	    }
	}
	else{
	    while(begin < end){
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&flavor, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    flavor = 0;
		    begin = end;
		}
		if(swapped)
		    flavor = SWAP_INT(flavor);
		if(end - begin > (ptrdiff_t)sizeof(uint32_t)){
		    memcpy((char *)&count, begin, sizeof(uint32_t));
		    begin += sizeof(uint32_t);
		}
		else{
		    count = 0;
		    begin = end;
		}
		if(swapped)
		    count = SWAP_INT(count);
		printf("     flavor %u\n", flavor);
		printf("      count %u\n", count);
		printf("      state (Unknown cputype/cpusubtype):\n");
		print_unknown_state(begin, end, count, swapped);
		begin += count * sizeof(uint32_t);
	    }
	}
}

/* current i386 thread states */
#if i386_THREAD_STATE == 1
#ifdef i386_EXCEPTION_STATE_COUNT

static
void
print_mmst_reg(
struct mmst_reg *r)
{
    uint32_t f;

	printf("\t      mmst_reg  ");
	for(f = 0; f < 10; f++){
	    printf("%02x ",
		   (unsigned int)(r->mmst_reg[f] & 0xff));
	}
	printf("\n");
	printf("\t      mmst_rsrv ");
	for(f = 0; f < 6; f++){
	    printf("%02x ",
		   (unsigned int)(r->mmst_rsrv[f] & 0xff));
	}
	printf("\n");
}

static
void
print_xmm_reg(
struct xmm_reg *r)
{
    uint32_t f;

	printf("\t      xmm_reg ");
	for(f = 0; f < 16; f++){
	    printf("%02x ",
		   (unsigned int)(r->xmm_reg[f] & 0xff));
	}
	printf("\n");
}
#endif /* defined(i386_EXCEPTION_STATE_COUNT) */
#endif /* i386_THREAD_STATE == 1 */

static
void
print_unknown_state(
char *begin,
char *end,
unsigned int count,
enum bool swapped)
{
    uint32_t left, *state, i, j;

	left = (uint32_t)(end - begin);
	if(left / sizeof(uint32_t) >= count){
	    state = allocate(count * sizeof(uint32_t));
	    memcpy((char *)state, begin, count * sizeof(uint32_t));
	    begin += count * sizeof(uint32_t);
	}
	else{
	    state = allocate(left);
	    memset((char *)state, '\0', left);
	    memcpy((char *)state, begin, left);
	    count = left / sizeof(uint32_t);
	    begin += left;
	}
	if(swapped)
	    for(i = 0 ; i < count; i++)
		state[i] = SWAP_INT(state[i]);
	for(i = 0 ; i < count; i += j){
	    for(j = 0 ; j < 8 && i + j < count; j++)
		printf("%08x ", (unsigned int)state[i + j]);
	    printf("\n");
	}
	free(state);
}

/*
 * Print the relocation information.
 */
void
print_reloc(
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
cpu_type_t cputype,
enum byte_sex load_commands_byte_sex,
char *object_addr,
uint64_t object_size,
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
uint32_t strings_size,
enum bool verbose)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    uint32_t i, j, k, left, size, nsects;
    char *p;
    struct load_command *lc, l;
    struct segment_command sg;
    struct section s;
    struct segment_command_64 sg64;
    struct section_64 s64;
    struct reloc_section_info *sect_rel;
    struct dysymtab_command dyst;
    uint64_t big_size;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != load_commands_byte_sex;

	/*
	 * Create an array of section structures in the host byte sex so it
	 * can be processed and indexed into directly.
	 */
	k = 0;
	nsects = 0;
	sect_rel = NULL;
	lc = load_commands;
	dyst.cmd = 0;
	for(i = 0 ; i < ncmds; i++){
	    memcpy((char *)&l, (char *)lc, sizeof(struct load_command));
	    if(swapped)
		swap_load_command(&l, host_byte_sex);
	    if(l.cmdsize % sizeof(int32_t) != 0)
		printf("load command %u size not a multiple of "
		       "sizeof(int32_t)\n", i);
	    if((char *)lc + l.cmdsize >
	       (char *)load_commands + sizeofcmds)
		printf("load command %u extends past end of load "
		       "commands\n", i);
	    left = sizeofcmds - (uint32_t)((char *)lc - (char *)load_commands);

	    switch(l.cmd){
	    case LC_SEGMENT:
		memset((char *)&sg, '\0', sizeof(struct segment_command));
		size = left < sizeof(struct segment_command) ?
		       left : sizeof(struct segment_command);
		memcpy((char *)&sg, (char *)lc, size);
		if(swapped)
		    swap_segment_command(&sg, host_byte_sex);

		big_size = sg.nsects;
		big_size *= sizeof(struct segment_command);
		if(big_size > sg.cmdsize){
		    printf("number of sections in load command %u extends past "
			   "end of load command\n", i);
		    if(sg.cmdsize > sizeof(struct segment_command))
			sg.nsects = (sg.cmdsize -
				     sizeof(struct segment_command)) /
				    sizeof(struct section);
		    else
			sg.nsects = 0;
		}
		nsects += sg.nsects;
		sect_rel = reallocate(sect_rel,
			      nsects * sizeof(struct reloc_section_info));
		memset((char *)(sect_rel + (nsects - sg.nsects)), '\0',
		       sizeof(struct reloc_section_info) * sg.nsects);
		p = (char *)lc + sizeof(struct segment_command);
		for(j = 0 ; j < sg.nsects ; j++){
		    left = sizeofcmds - (uint32_t)(p - (char *)load_commands);
		    size = left < sizeof(struct section) ?
			   left : sizeof(struct section);
		    memcpy((char *)&s, p, size);
		    if(swapped)
			swap_section(&s, 1, host_byte_sex);

		    if(p + sizeof(struct section) >
		       (char *)load_commands + sizeofcmds)
			break;
		    p += size;
		    memcpy(sect_rel[k].segname, s.segname, 16);
		    memcpy(sect_rel[k].sectname, s.sectname, 16);
		    sect_rel[k].nreloc = s.nreloc;
		    sect_rel[k].reloff = s.reloff;
		    k++;
		}
		break;
	    case LC_SEGMENT_64:
		memset((char *)&sg64, '\0', sizeof(struct segment_command_64));
		size = left < sizeof(struct segment_command_64) ?
		       left : sizeof(struct segment_command_64);
		memcpy((char *)&sg64, (char *)lc, size);
		if(swapped)
		    swap_segment_command_64(&sg64, host_byte_sex);

		big_size = sg64.nsects;
		big_size *= sizeof(struct segment_command_64);
		if(big_size > sg64.cmdsize){
		    printf("number of sections in load command %u extends past "
			   "end of load command\n", i);
		    if(sg64.cmdsize > sizeof(struct segment_command_64))
			sg64.nsects = (sg64.cmdsize -
				       sizeof(struct segment_command_64)) /
				      sizeof(struct section_64);
		    else
			sg64.nsects = 0;
		}
		nsects += sg64.nsects;
		sect_rel = reallocate(sect_rel,
			      nsects * sizeof(struct reloc_section_info));
		memset((char *)(sect_rel + (nsects - sg64.nsects)), '\0',
		       sizeof(struct reloc_section_info) * sg64.nsects);
		p = (char *)lc + sizeof(struct segment_command_64);
		for(j = 0 ; j < sg64.nsects ; j++){
		    left = sizeofcmds - (uint32_t)(p - (char *)load_commands);
		    size = left < sizeof(struct section_64) ?
			   left : sizeof(struct section_64);
		    memcpy((char *)&s64, p, size);
		    if(swapped)
			swap_section_64(&s64, 1, host_byte_sex);

		    if(p + sizeof(struct section_64) >
		       (char *)load_commands + sizeofcmds)
			break;
		    p += size;
		    memcpy(sect_rel[k].segname, s64.segname, 16);
		    memcpy(sect_rel[k].sectname, s64.sectname, 16);
		    sect_rel[k].nreloc = s64.nreloc;
		    sect_rel[k].reloff = s64.reloff;
		    k++;
		}
		break;
	    case LC_DYSYMTAB:
		memset((char *)&dyst, '\0', sizeof(struct dysymtab_command));
		size = left < sizeof(struct dysymtab_command) ?
		       left : sizeof(struct dysymtab_command);
		memcpy((char *)&dyst, (char *)lc, size);
		if(swapped)
		    swap_dysymtab_command(&dyst, host_byte_sex);
		break;
	    }
	    if(l.cmdsize == 0){
		printf("load command %u size zero (can't advance to other "
		       "load commands)\n", i);
		break;
	    }
	    lc = (struct load_command *)((char *)lc + l.cmdsize);
	    if((char *)lc > (char *)load_commands + sizeofcmds)
		break;
	}
	if((char *)load_commands + sizeofcmds != (char *)lc)
	    printf("Inconsistent sizeofcmds\n");

	if(dyst.cmd != 0){
	    if(dyst.nextrel != 0){
		printf("External relocation information %u entries",
		       dyst.nextrel);
		if(dyst.extreloff > object_size){
		    printf(" (offset to relocation entries extends past the "
			   "end of the file)\n");
		}
		else{
		    printf("\naddress  pcrel length extern type    scattered "
			   "symbolnum/value\n");

		    print_relocs(dyst.extreloff, dyst.nextrel, sect_rel, nsects,
				 swapped, cputype, object_addr, object_size,
				 symbols, symbols64, nsymbols, strings,
				 strings_size, verbose);
		}
	    }
	    if(dyst.nlocrel != 0){
		printf("Local relocation information %u entries", dyst.nlocrel);
		if(dyst.locreloff > object_size){
		    printf(" (offset to relocation entries extends past the "
			   "end of the file)\n");
		}
		else{
		    printf("\naddress  pcrel length extern type    scattered "
			   "symbolnum/value\n");

		    print_relocs(dyst.locreloff, dyst.nlocrel, sect_rel, nsects,
				 swapped, cputype, object_addr, object_size,
				 symbols, symbols64, nsymbols, strings,
				 strings_size, verbose);
		}
	    }
	}

	for(i = 0 ; i < nsects ; i++){
	    if(sect_rel[i].nreloc == 0)
		continue;
	    printf("Relocation information (%.16s,%.16s) %u entries",
		   sect_rel[i].segname, sect_rel[i].sectname,
		   sect_rel[i].nreloc);
	    if(sect_rel[i].reloff > object_size){
		printf(" (offset to relocation entries extends past the end of "
		       " the file)\n");
		continue;
	    }
	    printf("\naddress  pcrel length extern type    scattered "
		   "symbolnum/value\n");

	    print_relocs(sect_rel[i].reloff, sect_rel[i].nreloc, sect_rel,
			 nsects, swapped, cputype, object_addr, object_size,
			 symbols, symbols64, nsymbols, strings, strings_size,
			 verbose);
	}
}

static
void
print_relocs(
unsigned reloff,
unsigned nreloc,
struct reloc_section_info *sect_rel,
uint32_t nsects,
enum bool swapped,
cpu_type_t cputype,
char *object_addr,
uint64_t object_size,
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
uint32_t strings_size,
enum bool verbose)
{
    enum byte_sex host_byte_sex;
    uint32_t j;
    struct relocation_info *r, reloc;
    struct scattered_relocation_info *sr;
    enum bool previous_sectdiff, previous_ppc_jbsr, previous_arm_half,predicted;
    uint32_t sectdiff_r_type;
    uint32_t n_strx;

	host_byte_sex = get_host_byte_sex();

	previous_sectdiff = FALSE;
	previous_ppc_jbsr = FALSE;
	previous_arm_half = FALSE;
	sectdiff_r_type = 0;
	for(j = 0 ;
	    j < nreloc &&
	    reloff + (j + 1) * sizeof(struct relocation_info) <= object_size;
	    j++){
	    predicted = FALSE;
	    r = (struct relocation_info *)
		 (object_addr + reloff +
		  j * sizeof(struct relocation_info));
	    memcpy((char *)&reloc, (char *)r,
		   sizeof(struct relocation_info));
	    if(swapped)
		swap_relocation_info(&reloc, 1, host_byte_sex);
	    
	    if((reloc.r_address & R_SCATTERED) != 0 &&
	       cputype != CPU_TYPE_X86_64){
		sr = (struct scattered_relocation_info *)&reloc; 
		if(verbose){
		    if((cputype == CPU_TYPE_MC680x0 &&
			sr->r_type == GENERIC_RELOC_PAIR) ||
		       (cputype == CPU_TYPE_I386 &&
			sr->r_type == GENERIC_RELOC_PAIR) ||
		       (cputype == CPU_TYPE_MC88000 &&
			sr->r_type == M88K_RELOC_PAIR) ||
		       ((cputype == CPU_TYPE_POWERPC ||
		         cputype == CPU_TYPE_POWERPC64 ||
		         cputype == CPU_TYPE_VEO) &&
			sr->r_type == PPC_RELOC_PAIR) ||
		       (cputype == CPU_TYPE_HPPA &&
			sr->r_type == HPPA_RELOC_PAIR) ||
		       (cputype == CPU_TYPE_SPARC &&
			sr->r_type == SPARC_RELOC_PAIR) ||
		       (cputype == CPU_TYPE_ARM &&
			sr->r_type == ARM_RELOC_PAIR) ||
		       (cputype == CPU_TYPE_I860 &&
			sr->r_type == I860_RELOC_PAIR))
			    printf("         ");
		    else
			printf("%08x ", (unsigned int)sr->r_address);
		    if(sr->r_pcrel)
			printf("True  ");
		    else
			printf("False ");
		    if(cputype == CPU_TYPE_ARM && 
		       (sr->r_type == ARM_RELOC_HALF ||
			sr->r_type == ARM_RELOC_HALF_SECTDIFF ||
			previous_arm_half == TRUE)){
			if((sr->r_length & 0x1) == 0)
			    printf("lo/");
			else
			    printf("hi/");
			if((sr->r_length & 0x2) == 0)
			    printf("arm ");
			else
			    printf("thm ");
		    }
		    else{
			switch(sr->r_length){
			case 0:
			    printf("byte   ");
			    break;
			case 1:
			    printf("word   ");
			    break;
			case 2:
			    printf("long   ");
			    break;
			case 3:
			    /*
			     * The value of 3 for r_length for PowerPC is to
			     * encode that a conditional branch using the Y-bit
			     * for static branch prediction was predicted in
			     * the assembly source.
			     */
			    if((cputype == CPU_TYPE_POWERPC64 && 
				reloc.r_type == PPC_RELOC_VANILLA) ||
			       cputype == CPU_TYPE_X86_64) {
				    printf("quad   ");
			    }
			    else if(cputype == CPU_TYPE_POWERPC ||
				    cputype == CPU_TYPE_POWERPC64 || 
				    cputype == CPU_TYPE_VEO){
				printf("long   ");
				predicted = TRUE;
			    }
			    else
				printf("?(%2d)  ", sr->r_length);
			    break;
			default:
			    printf("?(%2d)  ", sr->r_length);
			    break;
			}
		    }
		    printf("n/a    ");
		    print_r_type(cputype, sr->r_type, predicted);
		    printf("True      0x%08x", (unsigned int)sr->r_value);
		    if(previous_sectdiff == FALSE){
			if((cputype == CPU_TYPE_MC88000 &&
			    sr->r_type == M88K_RELOC_PAIR) ||
			   (cputype == CPU_TYPE_SPARC &&
			    sr->r_type == SPARC_RELOC_PAIR) ||
			   (cputype == CPU_TYPE_ARM &&
			    sr->r_type == ARM_RELOC_PAIR) ||
			   (cputype == CPU_TYPE_I860 &&
			    sr->r_type == I860_RELOC_PAIR))
			    printf(" half = 0x%04x ",
				   (unsigned int)sr->r_address);
			else if(cputype == CPU_TYPE_HPPA &&
				 sr->r_type == HPPA_RELOC_PAIR)
			    printf(" other_part = 0x%06x ",
				   (unsigned int)sr->r_address);
			else if(((cputype == CPU_TYPE_POWERPC ||
				  cputype == CPU_TYPE_POWERPC64 ||
				  cputype == CPU_TYPE_VEO) &&
				 sr->r_type == PPC_RELOC_PAIR)){
			    if(previous_ppc_jbsr == FALSE)
				printf(" half = 0x%04x ",
				       (unsigned int)reloc.r_address);
			    else{
				printf(" <- other_part ");
			    }
			}
		    }
		    else if(cputype == CPU_TYPE_HPPA &&
			    (sectdiff_r_type == HPPA_RELOC_HI21_SECTDIFF ||
			     sectdiff_r_type == HPPA_RELOC_LO14_SECTDIFF)){
			    printf(" other_part = 0x%06x ",
				   (unsigned int)sr->r_address);
		    }
		    else if(cputype == CPU_TYPE_SPARC &&
			    (sectdiff_r_type == SPARC_RELOC_HI22_SECTDIFF ||
			     sectdiff_r_type == SPARC_RELOC_LO10_SECTDIFF)){
			    printf(" other_part = 0x%06x ",
				   (unsigned int)sr->r_address);
		    }
		    else if((cputype == CPU_TYPE_POWERPC ||
			     cputype == CPU_TYPE_POWERPC64 ||
			     cputype == CPU_TYPE_VEO) &&
			    (sectdiff_r_type == PPC_RELOC_HI16_SECTDIFF ||
			     sectdiff_r_type == PPC_RELOC_LO16_SECTDIFF ||
			     sectdiff_r_type == PPC_RELOC_LO14_SECTDIFF ||
			     sectdiff_r_type == PPC_RELOC_HA16_SECTDIFF)){
			    printf(" other_half = 0x%04x ",
				   (unsigned int)sr->r_address);
		    }
		    else if(cputype == CPU_TYPE_ARM &&
			    sectdiff_r_type == ARM_RELOC_HALF_SECTDIFF){
			    printf(" other_half = 0x%04x ",
				   (unsigned int)sr->r_address);
		    }
		    if((cputype == CPU_TYPE_MC680x0 &&
			(sr->r_type == GENERIC_RELOC_SECTDIFF ||
			 sr->r_type == GENERIC_RELOC_LOCAL_SECTDIFF)) ||
		       (cputype == CPU_TYPE_I386 &&
			(sr->r_type == GENERIC_RELOC_SECTDIFF ||
			 sr->r_type == GENERIC_RELOC_LOCAL_SECTDIFF)) ||
		       (cputype == CPU_TYPE_MC88000 &&
			sr->r_type == M88K_RELOC_SECTDIFF) ||
		       ((cputype == CPU_TYPE_POWERPC ||
		         cputype == CPU_TYPE_POWERPC64 ||
		         cputype == CPU_TYPE_VEO) &&
			(sr->r_type == PPC_RELOC_SECTDIFF ||
			 sr->r_type == PPC_RELOC_LOCAL_SECTDIFF ||
			 sr->r_type == PPC_RELOC_HI16_SECTDIFF ||
			 sr->r_type == PPC_RELOC_LO16_SECTDIFF ||
			 sr->r_type == PPC_RELOC_LO14_SECTDIFF ||
			 sr->r_type == PPC_RELOC_HA16_SECTDIFF)) ||
		       (cputype == CPU_TYPE_I860 &&
			sr->r_type == I860_RELOC_SECTDIFF) ||
		       (cputype == CPU_TYPE_HPPA &&
			(sr->r_type == HPPA_RELOC_SECTDIFF ||
			 sr->r_type == HPPA_RELOC_HI21_SECTDIFF ||
			 sr->r_type == HPPA_RELOC_LO14_SECTDIFF)) ||
		       (cputype == CPU_TYPE_ARM &&
			(sr->r_type == ARM_RELOC_SECTDIFF ||
			 sr->r_type == ARM_RELOC_LOCAL_SECTDIFF ||
			 sr->r_type == ARM_RELOC_HALF_SECTDIFF)) ||
		       (cputype == CPU_TYPE_SPARC &&
			(sr->r_type == SPARC_RELOC_SECTDIFF ||
			 sr->r_type == SPARC_RELOC_HI22_SECTDIFF ||
			 sr->r_type == SPARC_RELOC_LO10_SECTDIFF))){
			previous_sectdiff = TRUE;
			sectdiff_r_type = sr->r_type;
		    }
		    else
			previous_sectdiff = FALSE;
		    if(((cputype == CPU_TYPE_POWERPC ||
		         cputype == CPU_TYPE_POWERPC64 ||
		         cputype == CPU_TYPE_VEO) &&
			 sr->r_type == PPC_RELOC_JBSR))
			previous_ppc_jbsr = TRUE;
		    else
			previous_ppc_jbsr = FALSE;
		    if(cputype == CPU_TYPE_ARM &&
		       (sr->r_type == ARM_RELOC_HALF ||
		        sr->r_type == ARM_RELOC_HALF_SECTDIFF))
			previous_arm_half = TRUE;
		    else
			previous_arm_half = FALSE;
		    printf("\n");
		}
		else{
		    printf("%08x %1d     %-2d     n/a    %-7d 1         "
			   "0x%08x\n", (unsigned int)sr->r_address,
			   sr->r_pcrel, sr->r_length, sr->r_type,
			   (unsigned int)sr->r_value);
		}
	    }
	    else{
		if(verbose){
		    if((cputype == CPU_TYPE_MC88000 &&
			reloc.r_type == M88K_RELOC_PAIR) ||
		       ((cputype == CPU_TYPE_POWERPC ||
		         cputype == CPU_TYPE_POWERPC64 ||
		         cputype == CPU_TYPE_VEO) &&
			reloc.r_type == PPC_RELOC_PAIR) ||
		       (cputype == CPU_TYPE_HPPA &&
			reloc.r_type == HPPA_RELOC_PAIR) ||
		       (cputype == CPU_TYPE_SPARC &&
			reloc.r_type == SPARC_RELOC_PAIR) ||
		       (cputype == CPU_TYPE_ARM &&
			reloc.r_type == ARM_RELOC_PAIR) ||
		       (cputype == CPU_TYPE_I860 &&
			reloc.r_type == I860_RELOC_PAIR))
			    printf("         ");
		    else
			printf("%08x ", (unsigned int)reloc.r_address);
		    if(reloc.r_pcrel)
			printf("True  ");
		    else
			printf("False ");
		    if(cputype == CPU_TYPE_ARM && 
		       (reloc.r_type == ARM_RELOC_HALF ||
			reloc.r_type == ARM_RELOC_HALF_SECTDIFF ||
			previous_arm_half == TRUE)){
			if((reloc.r_length & 0x1) == 0)
			    printf("lo/");
			else
			    printf("hi/");
			if((reloc.r_length & 0x2) == 0)
			    printf("arm ");
			else
			    printf("thm ");
		    }
		    else{
			switch(reloc.r_length){
			case 0:
			    printf("byte   ");
			    break;
			case 1:
			    printf("word   ");
			    break;
			case 2:
			    printf("long   ");
			    break;
			case 3:
			    /*
			     * The value of 3 for r_length for PowerPC is to
			     * encode that a conditional branch using the Y-bit
			     * for static branch prediction was predicted in
			     * the assembly source.
			     */
			    if((cputype == CPU_TYPE_POWERPC64 && 
				reloc.r_type == PPC_RELOC_VANILLA) ||
			       cputype == CPU_TYPE_ARM64 ||
			       cputype == CPU_TYPE_X86_64) {
				    printf("quad   ");
			    }
			    else if(cputype == CPU_TYPE_POWERPC ||
				    cputype == CPU_TYPE_POWERPC64 ||
				    cputype == CPU_TYPE_VEO){
				printf("long   ");
				predicted = TRUE;
			    }
			    else
				printf("?(%2d)  ", reloc.r_length);
			    break;
			default:
			    printf("?(%2d)  ", reloc.r_length);
			    break;
			}
		    }
		    if(reloc.r_extern){
			printf("True   ");
			print_r_type(cputype, reloc.r_type, predicted);
			printf("False     ");
			if((symbols == NULL && symbols64 == NULL) ||
			   strings == NULL ||
			   reloc.r_symbolnum > nsymbols)
			    printf("?(%d)\n", reloc.r_symbolnum);
			else{
			    if(symbols != NULL)
				n_strx = symbols[reloc.r_symbolnum].n_un.n_strx;
			    else
				n_strx = symbols64[reloc.r_symbolnum].
					 n_un.n_strx;
			    if(n_strx > strings_size)
				printf("?(%d)\n", reloc.r_symbolnum);
			    else
				printf("%s\n", strings + n_strx);
			}
		    }
		    else{
			printf("False  ");
			print_r_type(cputype, reloc.r_type, predicted);
			printf("False     ");
			if((cputype == CPU_TYPE_I860 &&
			    reloc.r_type == I860_RELOC_PAIR) ||
			   (cputype == CPU_TYPE_MC88000 &&
			    reloc.r_type == M88K_RELOC_PAIR) ){
			    printf("half = 0x%04x\n",
				   (unsigned int)reloc.r_address);
			}
			else if((cputype == CPU_TYPE_HPPA &&
				 reloc.r_type == HPPA_RELOC_PAIR) ||
				(cputype == CPU_TYPE_SPARC &&
				 reloc.r_type == SPARC_RELOC_PAIR)){
			    printf(" other_part = 0x%06x\n",
				   (unsigned int)reloc.r_address);
			}
			else if(((cputype == CPU_TYPE_POWERPC ||
				  cputype == CPU_TYPE_POWERPC64 ||
				  cputype == CPU_TYPE_VEO) &&
				 reloc.r_type == PPC_RELOC_PAIR)){
			    if(previous_ppc_jbsr == FALSE)
				printf("half = 0x%04x\n",
				       (unsigned int)reloc.r_address);
			    else
				printf("other_part = 0x%08x\n",
				       (unsigned int)reloc.r_address);
			}
			else if(cputype == CPU_TYPE_ARM &&
				reloc.r_type == ARM_RELOC_PAIR)
			    printf("other_half = 0x%04x\n",
				   (unsigned int)reloc.r_address);
			else if((cputype == CPU_TYPE_ARM64 ||
				 cputype == CPU_TYPE_ARM64_32) &&
				reloc.r_type == ARM64_RELOC_ADDEND)
			    printf("addend = 0x%06x\n",
				   (unsigned int)reloc.r_symbolnum);
			else{
			    printf("%d ", reloc.r_symbolnum);
			    if(reloc.r_symbolnum > nsects ||
			       sect_rel == NULL)
				printf("(?,?)\n");
			    else{
				if(reloc.r_symbolnum == R_ABS)
				    printf("R_ABS\n");
			        else if(*sect_rel[reloc.r_symbolnum-1].
						segname == '\0' ||
			                *sect_rel[reloc.r_symbolnum-1].
						sectname == '\0')
				    printf("(?,?)\n");
				else
				    printf("(%.16s,%.16s)\n",
				    sect_rel[reloc.r_symbolnum-1].segname,
				    sect_rel[reloc.r_symbolnum-1].sectname);
			    }
			}
		    }
		    if(((cputype == CPU_TYPE_POWERPC ||
		         cputype == CPU_TYPE_POWERPC64 ||
		         cputype == CPU_TYPE_VEO) &&
			 reloc.r_type == PPC_RELOC_JBSR))
			previous_ppc_jbsr = TRUE;
		    else
			previous_ppc_jbsr = FALSE;
		    if(cputype == CPU_TYPE_ARM &&
		       (reloc.r_type == ARM_RELOC_HALF ||
		        reloc.r_type == ARM_RELOC_HALF_SECTDIFF))
			previous_arm_half = TRUE;
		    else
			previous_arm_half = FALSE;
		}
		else{
		    printf("%08x %1d     %-2d     %1d      %-7d 0"
			   "         %d\n", (unsigned int)reloc.r_address,
			   reloc.r_pcrel, reloc.r_length, reloc.r_extern,
			   reloc.r_type, reloc.r_symbolnum);
		}
	    }
	}
}


static char *generic_r_types[] = {
    "VANILLA ", "PAIR    ", "SECTDIF ", "PBLAPTR ", "LOCSDIF ", "TLV     ",
    "  6 (?) ", "  7 (?) ", "  8 (?) ", "  9 (?) ", " 10 (?) ", " 11 (?) ",
    " 12 (?) ", " 13 (?) ", " 14 (?) ", " 15 (?) "
};
static char *m88k_r_types[] = {
    "VANILLA ", "PAIR    ", "PC16    ", "PC26    ", "HI16    ", "LO16    ",
    "SECTDIF ", "PBLAPTR ", "  8 (?) ", "  9 (?) ", " 10 (?) ", " 11 (?) ",
    " 12 (?) ", " 13 (?) ", " 14 (?) ", " 15 (?) "
};
static char *i860_r_types[] = {
    "VANILLA ", "PAIR    ", "HIGH    ", "LOW0    ", "LOW1    ", "LOW2    ",
    "LOW3    ", "LOW4    ", "SPLIT0  ", "SPLIT1  ", "SPLIT2  ", "HIGHADJ ",
    "BRADDR  ", "SECTDIF ", " 14 (?) ", " 15 (?) "
};
static char *ppc_r_types[] = {
    "VANILLA ", "PAIR    ", "BR14",     "BR24    ", "HI16    ", "LO16    ",
    "HA16    ", "LO14    ", "SECTDIF ", "PBLAPTR ", "HI16DIF ", "LO16DIF ",
    "HA16DIF ", "JBSR    ", "LO14DIF ", "LOCSDIF "
};
static char *x86_64_r_types[] = {
    "UNSIGND ", "SIGNED  ", "BRANCH  ", "GOT_LD  ", "GOT     ", "SUB     ",
    "SIGNED1 ", "SIGNED2 ", "SIGNED4 ", "TLV     ", " 10 (?) ", " 11 (?) ",
    " 12 (?) ", " 13 (?) ", " 14 (?) ", " 15 (?) "
};
static char *hppa_r_types[] = {
	"VANILLA ", "PAIR    ", "HI21    ", "LO14    ", "BR17    ",
	"BL17    ", "JBSR    ", "SECTDIF ", "HI21DIF ", "LO14DIF ",
	"PBLAPTR ", " 11 (?) ", " 12 (?) ", " 13 (?) ", " 14 (?) ", " 15 (?) "
};

static char *sparc_r_types[] = {
	"VANILLA ", "PAIR    ", "HI22    ", "LO10    ", "DISP22  ",
	"DISP30  ", "SECTDIFF", "HI22DIFF", "LO10DIFF", "PBLAPTR ", 
	" 10 (?) ", " 11 (?) ", " 12 (?) ", " 13 (?) ", " 14 (?) ", " 15 (?) "
};

static char *arm_r_types[] = {
	"VANILLA ", "PAIR    ", "SECTDIFF", "LOCSDIF ", "PBLAPTR ",
	"BR24    ", "T_BR22  ", "T_BR32  ", "HALF    ", "HALFDIF ", 
	" 10 (?) ", " 11 (?) ", " 12 (?) ", " 13 (?) ", " 14 (?) ", " 15 (?) "
};

static char *arm64_r_types[] = {
	"UNSIGND ", "SUB     ", "BR26    ", "PAGE21  ", "PAGOF12 ",
	"GOTLDP  ", "GOTLDPOF", "PTRTGOT ", "TLVLDP  ", "TLVLDPOF", 
	"ADDEND  ",
        "AUTH    ",
	" 12 (?) ", " 13 (?) ", " 14 (?) ", " 15 (?) "
};

static
void
print_r_type(
cpu_type_t cputype,
uint32_t r_type,
enum bool predicted)
{
	if(r_type > 0xf){
	    printf("%-7u ", r_type);
	    return;
	}
	switch(cputype){
	case CPU_TYPE_MC680x0:
	case CPU_TYPE_I386:
	    printf("%s", generic_r_types[r_type]);
	    break;
	case CPU_TYPE_X86_64:
		printf("%s", x86_64_r_types[r_type]);
		break;
	case CPU_TYPE_MC88000:
	    printf("%s", m88k_r_types[r_type]);
	    break;
	case CPU_TYPE_I860:
	    printf("%s", i860_r_types[r_type]);
	    break;
	case CPU_TYPE_POWERPC:
	case CPU_TYPE_POWERPC64:
	case CPU_TYPE_VEO:
	    printf("%s", ppc_r_types[r_type]);
	    if(r_type == PPC_RELOC_BR14){
		if(predicted == TRUE)
		    printf("+/- ");
		else
		    printf("    ");
	    }
	    break;
	case CPU_TYPE_HPPA:
	    printf("%s", hppa_r_types[r_type]);
	    break;
	case CPU_TYPE_SPARC:
	    printf("%s", sparc_r_types[r_type]);
	    break;
	case CPU_TYPE_ARM:
	    printf("%s", arm_r_types[r_type]);
	    break;
	case CPU_TYPE_ARM64:
	case CPU_TYPE_ARM64_32:
	    printf("%s", arm64_r_types[r_type]);
	    break;
	default:
	    printf("%-7u ", r_type);
	}
}

/*
 * Print the table of contents.
 */
void
print_toc(
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex load_commands_byte_sex,
char *object_addr,
uint64_t object_size,
struct dylib_table_of_contents *tocs,
uint32_t ntocs,
struct dylib_module *mods,
struct dylib_module_64 *mods64,
uint32_t nmods,
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
uint32_t strings_size,
enum bool verbose)
{
   uint32_t i;
   uint32_t n_strx;
   uint8_t n_type;

	printf("Table of contents (%u entries)\n", ntocs);
	if(verbose)
	    printf("module name      symbol name\n");
	else
	    printf("module index symbol index\n");
	for(i = 0; i < ntocs; i++){
	    if(verbose){
		if(tocs[i].module_index > nmods)
		    printf("%-16u (past the end of the module table) ",
			   tocs[i].module_index);
		else if(mods != NULL){
		    if(mods[tocs[i].module_index].module_name > strings_size)
			printf("%-16u (string index past the end of string "
			       "table) ", tocs[i].module_index);
		    else
			printf("%-16s ", strings +
			       mods[tocs[i].module_index].module_name);
		}
		else if(mods64 != NULL){
		    if(mods64[tocs[i].module_index].module_name > strings_size)
			printf("%-16u (string index past the end of string "
			       "table) ", tocs[i].module_index);
		    else
			printf("%-16s ", strings +
			       mods64[tocs[i].module_index].module_name);
		}

		if(tocs[i].symbol_index > nsymbols)
		    printf("%u (past the end of the symbol table)\n",
			   tocs[i].symbol_index);
		else if(symbols != NULL || symbols64 != NULL){
		    if(symbols != NULL){
			n_strx = symbols[tocs[i].symbol_index].n_un.n_strx;
			n_type = symbols[tocs[i].symbol_index].n_type;
		    }
		    else{
			n_strx = symbols64[tocs[i].symbol_index].n_un.n_strx;
			n_type = symbols64[tocs[i].symbol_index].n_type;
		    }
		    if(n_strx > strings_size){
			printf("%u (string index past the end of the string "
			       "table)\n", tocs[i].symbol_index);
		    }
		    else{
			printf("%s", strings + n_strx);
			if(n_type & N_EXT)
			    printf("\n");
			else
			    printf(" [private]\n");
		    }
		}
	    }
	    else{
		printf("%-12u %u\n", tocs[i].module_index,
		       tocs[i].symbol_index);
	    }
	}
}

/*
 * Print the module table (32-bit).
 */
void
print_module_table(
struct dylib_module *mods,
uint32_t nmods,
char *strings,
uint32_t strings_size,
enum bool verbose)
{
   uint32_t i;

	printf("Module table (%u entries)\n", nmods);
	for(i = 0; i < nmods; i++){
	    printf("module %u\n", i);
	    if(verbose){
		if(mods[i].module_name > strings_size)
		    printf("    module_name = %u (past end of string table)\n",
			   mods[i].module_name);
		else
		    printf("    module_name = %s\n",
			   strings + mods[i].module_name);
	    }
	    else{
		if(mods[i].module_name > strings_size)
		    printf("    module_name = %u (past end of string table)\n",
			   mods[i].module_name);
		else
		    printf("    module_name = %u\n", mods[i].module_name);
	    }
	    printf("     iextdefsym = %u\n", mods[i].iextdefsym);
	    printf("     nextdefsym = %u\n", mods[i].nextdefsym);
	    printf("        irefsym = %u\n", mods[i].irefsym);
	    printf("        nrefsym = %u\n", mods[i].nrefsym);
	    printf("      ilocalsym = %u\n", mods[i].ilocalsym);
	    printf("      nlocalsym = %u\n", mods[i].nlocalsym);
	    printf("        iextrel = %u\n", mods[i].iextrel);
	    printf("        nextrel = %u\n", mods[i].nextrel);
	    printf("    iinit_iterm = %u %u\n",
		mods[i].iinit_iterm & 0xffff,
		(mods[i].iinit_iterm >> 16) & 0xffff);
	    printf("    ninit_nterm = %u %u\n",
		mods[i].ninit_nterm & 0xffff,
		(mods[i].ninit_nterm >> 16) & 0xffff);
	    printf("      objc_addr = 0x%x\n",
		(unsigned int)mods[i].objc_module_info_addr);
	    printf("      objc_size = %u\n", mods[i].objc_module_info_size);
	}
}

/*
 * Print the module table (64-bit).
 */
void
print_module_table_64(
struct dylib_module_64 *mods64,
uint32_t nmods,
char *strings,
uint32_t strings_size,
enum bool verbose)
{
   uint32_t i;

	printf("Module table (%u entries)\n", nmods);
	for(i = 0; i < nmods; i++){
	    printf("module %u\n", i);
	    if(verbose){
		if(mods64[i].module_name > strings_size)
		    printf("    module_name = %u (past end of string table)\n",
			   mods64[i].module_name);
		else
		    printf("    module_name = %s\n",
			   strings + mods64[i].module_name);
	    }
	    else{
		if(mods64[i].module_name > strings_size)
		    printf("    module_name = %u (past end of string table)\n",
			   mods64[i].module_name);
		else
		    printf("    module_name = %u\n", mods64[i].module_name);
	    }
	    printf("     iextdefsym = %u\n", mods64[i].iextdefsym);
	    printf("     nextdefsym = %u\n", mods64[i].nextdefsym);
	    printf("        irefsym = %u\n", mods64[i].irefsym);
	    printf("        nrefsym = %u\n", mods64[i].nrefsym);
	    printf("      ilocalsym = %u\n", mods64[i].ilocalsym);
	    printf("      nlocalsym = %u\n", mods64[i].nlocalsym);
	    printf("        iextrel = %u\n", mods64[i].iextrel);
	    printf("        nextrel = %u\n", mods64[i].nextrel);
	    printf("    iinit_iterm = %u %u\n",
		mods64[i].iinit_iterm & 0xffff,
		(mods64[i].iinit_iterm >> 16) & 0xffff);
	    printf("    ninit_nterm = %u %u\n",
		mods64[i].ninit_nterm & 0xffff,
		(mods64[i].ninit_nterm >> 16) & 0xffff);
	    printf("      objc_addr = 0x%016llx\n",
		mods64[i].objc_module_info_addr);
	    printf("      objc_size = %u\n", mods64[i].objc_module_info_size);
	}
}

/*
 * Print the reference table.
 */
void
print_refs(
struct dylib_reference *refs,
uint32_t nrefs,
struct dylib_module *mods,
struct dylib_module_64 *mods64,
uint32_t nmods,
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
uint32_t strings_size,
enum bool verbose)
{
   uint32_t i, j;
   uint32_t module_name, irefsym, nrefsym, n_strx;
    uint64_t big_size;

	printf("Reference table (%u entries)\n", nrefs);
	for(i = 0; i < nmods; i++){
	    if(mods != NULL){
		module_name = mods[i].module_name;
		irefsym = mods[i].irefsym;
		nrefsym = mods[i].nrefsym;
	    }
	    else{
		module_name = mods64[i].module_name;
		irefsym = mods64[i].irefsym;
		nrefsym = mods64[i].nrefsym;
	    }
	    if(verbose){
		if(module_name > strings_size)
		    printf("    module %u (past end of string table)",
			   module_name);
		else
		    printf("    module %s", strings + module_name);
	    }
	    else{
		printf("    module %u", module_name);
	    }
	    if(irefsym > nrefs){
		printf(" %u entries, at index %u (past end of reference "
		       "table)\n", nrefsym, irefsym);
		continue;
	    }
	    big_size = irefsym;
	    big_size += nrefsym;
	    if(big_size > nrefs)
		printf(" %u entries (extends past the end of the reference "
		       "table), at index %u\n", nrefsym, irefsym);
	    else
		printf(" %u entries, at index %u\n", nrefsym, irefsym);
	    for(j = irefsym;
		j - irefsym < nrefsym && j < nrefs;
		j++){
		if(refs[j].isym > nsymbols)
		    printf("\t%u (past the end of the symbol table) ",
			   refs[j].isym);
		else{
		    if(verbose){
			if(refs[j].isym > nsymbols)
			    printf("\t%u (past the end of the symbol table) ",
				   refs[j].isym);
			else if(symbols != NULL || symbols64 != NULL){
			    if(symbols != NULL)
				n_strx = symbols[refs[j].isym].n_un.n_strx;
			    else
				n_strx = symbols64[refs[j].isym].n_un.n_strx;
			    if(n_strx > strings_size)
				printf("\t%u (string index past the end of the "
				       "string table) ", refs[j].isym);
			    else
				printf("\t%s ", strings + n_strx);
			}
		    }
		    else
			printf("\tisym %u ", refs[j].isym);
		}
		if(verbose){
		    switch(refs[j].flags){
		    case REFERENCE_FLAG_UNDEFINED_NON_LAZY:
			printf("undefined [non-lazy]\n");
			break;
		    case REFERENCE_FLAG_UNDEFINED_LAZY:
			printf("undefined [lazy]\n");
			break;
		    case REFERENCE_FLAG_DEFINED:
			printf("defined\n");
			break;
		    case REFERENCE_FLAG_PRIVATE_DEFINED:
			printf("private defined\n");
			break;
		    case REFERENCE_FLAG_PRIVATE_UNDEFINED_NON_LAZY:
			printf("private undefined [non-lazy]\n");
			break;
		    case REFERENCE_FLAG_PRIVATE_UNDEFINED_LAZY:
			printf("private undefined [lazy]\n");
			break;
		    default:
			printf("%u\n", (unsigned int)refs[j].flags);
			break;
		    }
		}
		else
		    printf("flags %u\n", (unsigned int)refs[j].flags);
	    }
	}
}

/*
 * Print the indirect symbol table.
 */
void
print_indirect_symbols(
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
cpu_type_t cputype,
enum byte_sex load_commands_byte_sex,
uint32_t *indirect_symbols,
uint32_t nindirect_symbols,
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
uint32_t strings_size,
enum bool verbose)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    uint32_t i, j, k, left, size, nsects, n, count, stride, section_type;
    uint64_t bigsize, big_load_end;
    char *p;
    struct load_command *lc, l;
    struct segment_command sg;
    struct section s;
    struct segment_command_64 sg64;
    struct section_64 s64;
    struct section_indirect_info {
	char segname[16];
	char sectname[16];
	uint64_t size;
	uint64_t addr;
	uint32_t reserved1;
	uint32_t reserved2;
	uint32_t flags;
    } *sect_ind;
    uint32_t n_strx;

	sect_ind = NULL;
	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != load_commands_byte_sex;

	/*
	 * Create an array of section structures in the host byte sex so it
	 * can be processed and indexed into directly.
	 */
	k = 0;
	nsects = 0;
	lc = load_commands;
	big_load_end = 0;
	for(i = 0 ; i < ncmds; i++){
	    memcpy((char *)&l, (char *)lc, sizeof(struct load_command));
	    if(swapped)
		swap_load_command(&l, host_byte_sex);
	    if(l.cmdsize % sizeof(int32_t) != 0)
		printf("load command %u size not a multiple of "
		       "sizeof(int32_t)\n", i);
	    big_load_end += l.cmdsize;
	    if(big_load_end > sizeofcmds)
		printf("load command %u extends past end of load "
		       "commands\n", i);
	    left = sizeofcmds - (uint32_t)((char *)lc - (char *)load_commands);

	    switch(l.cmd){
	    case LC_SEGMENT:
		memset((char *)&sg, '\0', sizeof(struct segment_command));
		size = left < sizeof(struct segment_command) ?
		       left : sizeof(struct segment_command);
		left -= size;
		memcpy((char *)&sg, (char *)lc, size);
		if(swapped)
		    swap_segment_command(&sg, host_byte_sex);
		bigsize = sg.nsects;
		bigsize *= sizeof(struct section);
		bigsize += size;
		if(bigsize > sg.cmdsize){
		    printf("number of sections in load command %u extends "
			   "past end of load commands\n", i);
		    sg.nsects = (sg.cmdsize-size) / sizeof(struct section);
		}
		nsects += sg.nsects;
		sect_ind = reallocate(sect_ind,
			      nsects * sizeof(struct section_indirect_info));
		memset((char *)(sect_ind + (nsects - sg.nsects)), '\0',
		       sizeof(struct section_indirect_info) * sg.nsects);
		p = (char *)lc + sizeof(struct segment_command);
		for(j = 0 ; j < sg.nsects ; j++){
		    left = sizeofcmds - (uint32_t)(p - (char *)load_commands);
		    size = left < sizeof(struct section) ?
			   left : sizeof(struct section);
		    memcpy((char *)&s, p, size);
		    if(swapped)
			swap_section(&s, 1, host_byte_sex);

		    if(p + sizeof(struct section) >
		       (char *)load_commands + sizeofcmds)
			break;
		    p += size;
		    memcpy(sect_ind[k].segname, s.segname, 16);
		    memcpy(sect_ind[k].sectname, s.sectname, 16);
		    sect_ind[k].size = s.size;
		    sect_ind[k].addr = s.addr;
		    sect_ind[k].reserved1 = s.reserved1;
		    sect_ind[k].reserved2 = s.reserved2;
		    sect_ind[k].flags = s.flags;
		    k++;
		}
		break;
	    case LC_SEGMENT_64:
		memset((char *)&sg64, '\0', sizeof(struct segment_command_64));
		size = left < sizeof(struct segment_command_64) ?
		       left : sizeof(struct segment_command_64);
		memcpy((char *)&sg64, (char *)lc, size);
		if(swapped)
		    swap_segment_command_64(&sg64, host_byte_sex);
		bigsize = sg64.nsects;
		bigsize *= sizeof(struct section_64);
		bigsize += size;
		if(bigsize > sg64.cmdsize){
		    printf("number of sections in load command %u extends "
			   "past end of load commands\n", i);
		    sg64.nsects = (sg64.cmdsize-size) /
				  sizeof(struct section_64);
		}
		nsects += sg64.nsects;
		sect_ind = reallocate(sect_ind,
			      nsects * sizeof(struct section_indirect_info));
		memset((char *)(sect_ind + (nsects - sg64.nsects)), '\0',
		       sizeof(struct section_indirect_info) * sg64.nsects);
		p = (char *)lc + sizeof(struct segment_command_64);
		for(j = 0 ; j < sg64.nsects ; j++){
		    left = sizeofcmds - (uint32_t)(p - (char *)load_commands);
		    size = left < sizeof(struct section_64) ?
			   left : sizeof(struct section_64);
		    memcpy((char *)&s64, p, size);
		    if(swapped)
			swap_section_64(&s64, 1, host_byte_sex);

		    if(p + sizeof(struct section) >
		       (char *)load_commands + sizeofcmds)
			break;
		    p += size;
		    memcpy(sect_ind[k].segname, s64.segname, 16);
		    memcpy(sect_ind[k].sectname, s64.sectname, 16);
		    sect_ind[k].size = s64.size;
		    sect_ind[k].addr = s64.addr;
		    sect_ind[k].reserved1 = s64.reserved1;
		    sect_ind[k].reserved2 = s64.reserved2;
		    sect_ind[k].flags = s64.flags;
		    k++;
		}
		break;
	    }
	    if(l.cmdsize == 0){
		printf("load command %u size zero (can't advance to other "
		       "load commands)\n", i);
		break;
	    }
	    lc = (struct load_command *)((char *)lc + l.cmdsize);
	    if((char *)lc > (char *)load_commands + sizeofcmds)
		break;
	}
	if((char *)load_commands + sizeofcmds != (char *)lc)
	    printf("Inconsistent sizeofcmds\n");

	for(i = 0 ; i < nsects ; i++){
	    section_type = sect_ind[i].flags & SECTION_TYPE;
	    if(section_type == S_SYMBOL_STUBS){
		stride = sect_ind[i].reserved2;
		if(stride == 0){
		    printf("Can't print indirect symbols for (%.16s,%.16s) "
			   "(size of stubs in reserved2 field is zero)\n",
			   sect_ind[i].segname, sect_ind[i].sectname);
		    continue;
		}
	    }
	    else if(section_type == S_LAZY_SYMBOL_POINTERS ||
	            section_type == S_NON_LAZY_SYMBOL_POINTERS ||
	            section_type == S_LAZY_DYLIB_SYMBOL_POINTERS ||
	   	    section_type == S_THREAD_LOCAL_VARIABLE_POINTERS){
		if(cputype & CPU_ARCH_ABI64)
		    stride = 8;
		else
		    stride = 4;
	    }
	    else
		continue;
	
	    count = (uint32_t)(sect_ind[i].size / stride);
	    printf("Indirect symbols for (%.16s,%.16s) %u entries",
		   sect_ind[i].segname, sect_ind[i].sectname,
		   count);

	    n = sect_ind[i].reserved1;
	    if(n > nindirect_symbols)
		printf(" (entries start past the end of the indirect symbol "
		       "table) (reserved1 field greater than the table size)");
	    else if(n + count > nindirect_symbols)
		printf(" (entries extends past the end of the indirect symbol "
		       "table)");
	    if(cputype & CPU_ARCH_ABI64)
		printf("\naddress            index");
	    else
		printf("\naddress    index");
	    if(verbose)
		printf(" name\n");
	    else
		printf("\n");

	    for(j = 0 ; j < count && n + j < nindirect_symbols; j++){
		if(cputype & CPU_ARCH_ABI64)
		    printf("0x%016llx ", sect_ind[i].addr + j * stride);
		else
		    printf("0x%08x ",(uint32_t)
				     (sect_ind[i].addr + j * stride));
		if(indirect_symbols[j + n] == INDIRECT_SYMBOL_LOCAL){
		    printf("LOCAL\n");
		    continue;
		}
		if(indirect_symbols[j + n] ==
		   (INDIRECT_SYMBOL_LOCAL | INDIRECT_SYMBOL_ABS)){
		    printf("LOCAL ABSOLUTE\n");
		    continue;
		}
		if(indirect_symbols[j + n] == INDIRECT_SYMBOL_ABS){
		    /* 
		     * Used for unused slots in the i386 __jump_table 
		     * and for image-loader-cache slot for new lazy
		     * symbol binding in Mac OS X 10.6 and later
		     */ 
		    printf("ABSOLUTE\n");
		    continue;
		}
		printf("%5u ", indirect_symbols[j + n]);
		if(verbose){
		    if(indirect_symbols[j + n] >= nsymbols ||
		       (symbols == NULL && symbols64 == NULL) ||
		       strings == NULL)
			printf("?\n");
		    else{
			if(symbols != NULL)
			    n_strx = symbols[indirect_symbols[j+n]].n_un.n_strx;
			else
			    n_strx = symbols64[indirect_symbols[j+n]].
					n_un.n_strx;
			if(n_strx >= strings_size)
			    printf("?\n");
			else
			    printf("%s\n", strings + n_strx);
		    }
		}
		else
		    printf("\n");
	    }
	    n += count;
	}
}

void
print_hints(
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex load_commands_byte_sex,
struct twolevel_hint *hints,
uint32_t nhints,
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
uint32_t strings_size,
enum bool verbose)
{
    enum byte_sex host_byte_sex;
    enum bool swapped, is_framework;
    uint32_t i, left, size, nlibs, dyst_cmd, lib_ord;
    char *p, **libs, *short_name, *has_suffix;
    struct load_command *lc, l;
    struct dysymtab_command dyst;
    struct dylib_command dl;
    uint32_t n_strx;
    uint16_t n_desc;
    uint64_t big_load_end;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != load_commands_byte_sex;

	/*
	 * If verbose is TRUE create an array of load dylibs names so it
	 * indexed into directly.
	 */
	nlibs = 0;
	libs = NULL;
	dyst_cmd = UINT_MAX;
	lc = load_commands;
	big_load_end = 0;
	memset((char *)&dyst, '\0', sizeof(struct dysymtab_command));
	for(i = 0 ; verbose == TRUE && i < ncmds; i++){
	    memcpy((char *)&l, (char *)lc, sizeof(struct load_command));
	    if(swapped)
		swap_load_command(&l, host_byte_sex);
	    if(l.cmdsize % sizeof(int32_t) != 0)
		printf("load command %u size not a multiple of "
		       "sizeof(int32_t)\n", i);
	    big_load_end += l.cmdsize;
	    if(big_load_end > sizeofcmds)
		printf("load command %u extends past end of load "
		       "commands\n", i);
	    left = sizeofcmds - (uint32_t)((char *)lc - (char *)load_commands);

	    switch(l.cmd){
	    case LC_DYSYMTAB:
		if(dyst_cmd != UINT_MAX){
		    printf("more than one LC_DYSYMTAB command (using command "
			   "%u)\n", dyst_cmd);
		    break;
		}
		size = left < sizeof(struct dysymtab_command) ?
		       left : sizeof(struct dysymtab_command);
		memcpy((char *)&dyst, (char *)lc, size);
		if(swapped)
		    swap_dysymtab_command(&dyst, host_byte_sex);
		dyst_cmd = i;
		break;

	    case LC_LOAD_DYLIB:
	    case LC_LOAD_WEAK_DYLIB:
	    case LC_REEXPORT_DYLIB:
	    case LC_LOAD_UPWARD_DYLIB:
	    case LC_LAZY_LOAD_DYLIB:
		memset((char *)&dl, '\0', sizeof(struct dylib_command));
		size = left < sizeof(struct dylib_command) ?
		       left : sizeof(struct dylib_command);
		memcpy((char *)&dl, (char *)lc, size);
		if(swapped)
		    swap_dylib_command(&dl, host_byte_sex);
		if(dl.dylib.name.offset < dl.cmdsize &&
                   dl.dylib.name.offset < left){
		    p = (char *)lc + dl.dylib.name.offset;
		    short_name = guess_short_name(p, &is_framework,
						  &has_suffix);
		    if(short_name != NULL)
			p = short_name;
		}
		else{
		    p = "bad dylib.name.offset";
		}
		libs = reallocate(libs, (nlibs+1) * sizeof(char *));
		libs[nlibs] = p;
		nlibs++;
		break;
	    }
	    if(l.cmdsize == 0){
		printf("load command %u size zero (can't advance to other "
		       "load commands)\n", i);
		break;
	    }
	    lc = (struct load_command *)((char *)lc + l.cmdsize);
	    if((char *)lc > (char *)load_commands + sizeofcmds)
		break;
	}
	if(verbose == TRUE &&
	   (char *)load_commands + sizeofcmds != (char *)lc)
	    printf("Inconsistent sizeofcmds\n");

	printf("Two-level namespace hints table (%u hints)\n", nhints);
	printf("index  isub  itoc\n");
	for(i = 0; i < nhints; i++){
	    printf("%5u %5d %5u", i, (int)hints[i].isub_image, hints[i].itoc);
	    if(verbose){
		if(dyst_cmd != UINT_MAX &&
		   dyst.iundefsym + i < nsymbols){
		    if(symbols != NULL){
			n_strx = symbols[dyst.iundefsym + i].n_un.n_strx;
			n_desc = symbols[dyst.iundefsym + i].n_desc;
		    }
		    else{
			n_strx = symbols64[dyst.iundefsym + i].n_un.n_strx;
			n_desc = symbols64[dyst.iundefsym + i].n_desc;
		    }
		    if(n_strx > strings_size)
			printf(" (bad string index in symbol %u)\n",
			       dyst.iundefsym + i);
		    else{
			printf(" %s", strings + n_strx);
			lib_ord = GET_LIBRARY_ORDINAL(n_desc);
			if(lib_ord != SELF_LIBRARY_ORDINAL &&
			   lib_ord - 1 < nlibs)
			    printf(" (from %s)\n", libs[lib_ord - 1]);
			else
			    printf("\n");
		    }
		}
		else
		    printf("\n");
	    }
	    else
		printf("\n");
	}
}

static
uint64_t
decodeULEB128(
const uint8_t *p,
unsigned *n)
{
  const uint8_t *orig_p = p;
  uint64_t Value = 0;
  unsigned Shift = 0;
  do {
    Value += (*p & 0x7f) << Shift;
    Shift += 7;
  } while (*p++ >= 128);
  if (n)
    *n = (unsigned)(p - orig_p);
  return Value;
}

void
print_link_opt_hints(
char *loh,
uint32_t nloh)
{
    uint32_t i, j;
    unsigned n;
    uint64_t identifier, narguments, value;

	printf("Linker optimiztion hints (%u total bytes)\n", nloh);
	for(i = 0; i < nloh;){
	    identifier = decodeULEB128((const uint8_t *)(loh + i), &n);
	    i += n;
	    printf("    identifier %llu ", identifier);
	    if(i >= nloh)
		return;
	    switch(identifier){
	    case 1:
		printf("AdrpAdrp\n");
		break;
	    case 2:
		printf("AdrpLdr\n");
		break;
	    case 3:
		printf("AdrpAddLdr\n");
		break;
	    case 4:
		printf("AdrpLdrGotLdr\n");
		break;
	    case 5:
		printf("AdrpAddStr\n");
		break;
	    case 6:
		printf("AdrpLdrGotStr\n");
		break;
	    case 7:
		printf("AdrpAdd\n");
		break;
	    case 8:
		printf("AdrpLdrGot\n");
		break;
	    default:
		printf("Unknown identifier value\n");
		break;
	    }

	    narguments = decodeULEB128((const uint8_t *)(loh + i), &n);
	    i += n;
	    printf("    narguments %llu\n", narguments);
	    if(i >= nloh)
		return;

	    for(j = 0; j < narguments; j++){
		value = decodeULEB128((const uint8_t *)(loh + i), &n);
		i += n;
		printf("\tvalue 0x%llx\n", value);
		if(i >= nloh)
		    return;
	    }
	}
}

void
print_dices(
struct data_in_code_entry *dices,
uint32_t ndices,
enum bool verbose)
{
    uint32_t i;

	printf("Data in code table (%u entries)\n", ndices);
	printf("offset     length kind\n");
	for(i = 0; i < ndices; i++){
	    printf("0x%08x %6u ", (unsigned)dices[i].offset, dices[i].length);
	    if(verbose){
		switch(dices[i].kind){
		case DICE_KIND_DATA:
		    printf("DATA");
		    break;
		case DICE_KIND_JUMP_TABLE8:
		    printf("JUMP_TABLE8");
		    break;
		case DICE_KIND_JUMP_TABLE16:
		    printf("JUMP_TABLE16");
		    break;
		case DICE_KIND_JUMP_TABLE32:
		    printf("JUMP_TABLE32");
		    break;
		case DICE_KIND_ABS_JUMP_TABLE32:
		    printf("ABS_JUMP_TABLE32");
		    break;
		default:
		    printf("0x%04x", (unsigned)dices[i].kind);
		    break;
		}
	    }
	    else
		printf("0x%04x", (unsigned)dices[i].kind);
	    printf("\n");
	}
}

void
print_cstring_section(
cpu_type_t cputype,
char *sect,
uint64_t sect_size,
uint64_t sect_addr,
enum bool print_addresses)
{
    uint64_t i;

	for(i = 0; i < sect_size ; i++){
	    if(print_addresses == TRUE){
	        if(cputype & CPU_ARCH_ABI64)
		    printf("%016llx  ", sect_addr + i);
		else
		    printf("%08x  ", (unsigned int)(sect_addr + i));
	    }

	    for( ; i < sect_size && sect[i] != '\0'; i++)
		print_cstring_char(sect[i]);
	    if(i < sect_size && sect[i] == '\0')
		printf("\n");
	}
}

static
void
print_cstring_char(
char c)
{
	if(isprint(c)){
	    if(c == '\\')	/* backslash */
		printf("\\\\");
	    else		/* all other printable characters */
		printf("%c", c);
	}
	else{
	    switch(c){
	    case '\n':		/* newline */
		printf("\\n");
		break;
	    case '\t':		/* tab */
		printf("\\t");
		break;
	    case '\v':		/* vertical tab */
		printf("\\v");
		break;
	    case '\b':		/* backspace */
		printf("\\b");
		break;
	    case '\r':		/* carriage return */
		printf("\\r");
		break;
	    case '\f':		/* formfeed */
		printf("\\f");
		break;
	    case '\a':		/* audiable alert */
		printf("\\a");
		break;
	    default:
		printf("\\%03o", (unsigned int)c);
	    }
	}
}

void
print_literal4_section(
cpu_type_t cputype,
char *sect,
uint64_t sect_size,
uint64_t sect_addr,
enum byte_sex literal_byte_sex,
enum bool print_addresses)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    uint64_t i, l;
    float f;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != literal_byte_sex;

	for(i = 0; i < sect_size ; i += sizeof(float)){
	    if(print_addresses == TRUE){
	        if(cputype & CPU_ARCH_ABI64)
		    printf("%016llx  ", sect_addr + i);
		else
		    printf("%08x  ", (unsigned int)(sect_addr + i));
	    }
	    memcpy((char *)&f, sect + i, sizeof(float));
	    memcpy((char *)&l, sect + i, sizeof(uint32_t));
	    if(swapped){
		f = SWAP_FLOAT(f);
		l = SWAP_INT(l);
	    }
	    print_literal4((uint32_t)l, f);
	}
}

static
void
print_literal4(
uint32_t l,
float f)
{
	printf("0x%08x", (unsigned int)l);
	if((l & 0x7f800000) != 0x7f800000){
	    printf(" (%.16e)\n", f);
	}
	else{
	    if(l == 0x7f800000)
		printf(" (+Infinity)\n");
	    else if(l == 0xff800000)
		printf(" (-Infinity)\n");
	    else if((l & 0x00400000) == 0x00400000)
		printf(" (non-signaling Not-a-Number)\n");
	    else
		printf(" (signaling Not-a-Number)\n");
	}
}

void
print_literal8_section(
cpu_type_t cputype,
char *sect,
uint64_t sect_size,
uint64_t sect_addr,
enum byte_sex literal_byte_sex,
enum bool print_addresses)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    uint64_t i;
    uint32_t l0, l1;
    double d;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != literal_byte_sex;

	for(i = 0; i < sect_size ; i += sizeof(double)){
	    if(print_addresses == TRUE){
	        if(cputype & CPU_ARCH_ABI64)
		    printf("%016llx  ", sect_addr + i);
		else
		    printf("%08x  ", (unsigned int)(sect_addr + i));
	    }
	    memcpy((char *)&d, sect + i, sizeof(double));
	    memcpy((char *)&l0, sect + i, sizeof(uint32_t));
	    memcpy((char *)&l1, sect + i + sizeof(uint32_t),
		   sizeof(uint32_t));
	    if(swapped){
		d = SWAP_DOUBLE(d);
		l0 = SWAP_INT(l0);
		l1 = SWAP_INT(l1);
	    }
	    print_literal8(l0, l1, d, literal_byte_sex);
	}
}

static
void
print_literal8(
uint32_t l0,
uint32_t l1,
double d,
enum byte_sex literal_byte_sex)
{
    uint32_t hi, lo;

	printf("0x%08x 0x%08x", (unsigned int)l0, (unsigned int)l1);
	if(literal_byte_sex == LITTLE_ENDIAN_BYTE_SEX){
	    hi = l1;
	    lo = l0;
	} else {
	    hi = l0;
	    lo = l1;
	}
	/* hi is the high word, so this is equivalent to if(isfinite(d)) */
	if((hi & 0x7ff00000) != 0x7ff00000)
	    printf(" (%.16e)\n", d);
	else{
	    if(hi == 0x7ff00000 && lo == 0)
		printf(" (+Infinity)\n");
	    else if(hi == 0xfff00000 && lo == 0)
		printf(" (-Infinity)\n");
	    else if((hi & 0x00080000) == 0x00080000)
		printf(" (non-signaling Not-a-Number)\n");
	    else
		printf(" (signaling Not-a-Number)\n");
	}
}

void
print_literal16_section(
cpu_type_t cputype,
char *sect,
uint64_t sect_size,
uint64_t sect_addr,
enum byte_sex literal_byte_sex,
enum bool print_addresses)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    uint64_t i;
    uint32_t l0, l1, l2, l3;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != literal_byte_sex;

	for(i = 0; i < sect_size ; i += 4 * sizeof(uint32_t)){
	    if(print_addresses == TRUE){
	        if(cputype & CPU_ARCH_ABI64)
		    printf("%016llx  ", sect_addr + i);
		else
		    printf("%08x  ", (unsigned int)(sect_addr + i));
	    }
	    memcpy((char *)&l0, sect + i, sizeof(uint32_t));
	    memcpy((char *)&l1, sect + i + sizeof(uint32_t),
		   sizeof(uint32_t));
	    memcpy((char *)&l2, sect + i + 2 * sizeof(uint32_t),
		   sizeof(uint32_t));
	    memcpy((char *)&l3, sect + i + 3 * sizeof(uint32_t),
		   sizeof(uint32_t));
	    if(swapped){
		l0 = SWAP_INT(l0);
		l1 = SWAP_INT(l1);
		l2 = SWAP_INT(l2);
		l3 = SWAP_INT(l3);
	    }
	    print_literal16(l0, l1, l2, l3);
	}
}

static
void
print_literal16(
uint32_t l0,
uint32_t l1,
uint32_t l2,
uint32_t l3)
{
	printf("0x%08x 0x%08x 0x%08x 0x%08x\n", (unsigned int)l0,\
	       (unsigned int)l1, (unsigned int)l2, (unsigned int)l3);
}

void
print_literal_pointer_section(
cpu_type_t cputype,
cpu_subtype_t cpusubtype,
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
uint32_t filetype,
enum byte_sex object_byte_sex,
char *object_addr,
uint64_t object_size,
char *sect,
uint64_t sect_size,
uint64_t sect_addr,
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
uint32_t strings_size,
struct relocation_info *relocs,
uint32_t nrelocs,
enum bool print_addresses)
{
    enum byte_sex host_byte_sex;
    enum bool swapped, found;
    uint32_t i, j, li, l0, l1, l2, l3, left, size, lp_size;
    uint64_t k, lp;
    struct load_command lcmd, *lc;
    struct segment_command sg;
    struct section s;
    struct segment_command_64 sg64;
    struct section_64 s64;
    struct literal_section {
	char segname[16];
	char sectname[16];
        uint64_t addr;
        uint32_t flags;
	char *contents;
	uint32_t size;
    } *literal_sections;
    char *p;
    uint32_t nliteral_sections;
    float f;
    double d = 0.;
    struct relocation_info *reloc;
    uint32_t n_strx;
    uint64_t big_load_end, big_size;

    d = 0.0; /* cctools-port */

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != object_byte_sex;

	literal_sections = NULL;
	nliteral_sections = 0;

	lc = load_commands;
	big_load_end = 0;
	for(i = 0 ; i < ncmds; i++){
	    memcpy((char *)&lcmd, (char *)lc, sizeof(struct load_command));
	    if(swapped)
		swap_load_command(&lcmd, host_byte_sex);
	    if(lcmd.cmdsize % sizeof(int32_t) != 0)
		printf("load command %u size not a multiple of "
		       "sizeof(int32_t)\n", i);
	    big_load_end += lcmd.cmdsize;
	    if(big_load_end > sizeofcmds)
		printf("load command %u extends past end of load "
		       "commands\n", i);
	    left = sizeofcmds - (uint32_t)((char *)lc - (char *)load_commands);

	    switch(lcmd.cmd){
	    case LC_SEGMENT:
		memset((char *)&sg, '\0', sizeof(struct segment_command));
		size = left < sizeof(struct segment_command) ?
		       left : sizeof(struct segment_command);
		memcpy((char *)&sg, (char *)lc, size);
		if(swapped)
		    swap_segment_command(&sg, host_byte_sex);

		p = (char *)lc + sizeof(struct segment_command);
		for(j = 0 ; j < sg.nsects ; j++){
		    if(p + sizeof(struct section) >
		       (char *)load_commands + sizeofcmds){
			printf("section structure command extends past "
			       "end of load commands\n");
		    }
		    left = sizeofcmds - (uint32_t)(p - (char *)load_commands);
		    memset((char *)&s, '\0', sizeof(struct section));
		    size = left < sizeof(struct section) ?
			   left : sizeof(struct section);
		    memcpy((char *)&s, p, size);
		    if(swapped)
			swap_section(&s, 1, host_byte_sex);

		    if(s.flags == S_CSTRING_LITERALS ||
		       s.flags == S_4BYTE_LITERALS ||
		       s.flags == S_8BYTE_LITERALS ||
		       s.flags == S_16BYTE_LITERALS){
			literal_sections = reallocate(literal_sections,
						sizeof(struct literal_section) *
						(nliteral_sections + 1));
			memcpy(literal_sections[nliteral_sections].segname,
			       s.segname, 16);
			memcpy(literal_sections[nliteral_sections].sectname,
			       s.sectname, 16);
        		literal_sections[nliteral_sections].addr = s.addr;
			literal_sections[nliteral_sections].flags = s.flags;
			literal_sections[nliteral_sections].contents = 
							object_addr + s.offset;
			big_size = s.offset;
			big_size += s.size;
			if(s.offset > object_size){
			    printf("section contents of: (%.16s,%.16s) is past "
				   "end of file\n", s.segname, s.sectname);
			    literal_sections[nliteral_sections].size =  0;
			}
			else if(big_size > object_size){
			    printf("part of section contents of: (%.16s,%.16s) "
				   "is past end of file\n",
				   s.segname, s.sectname);
			    literal_sections[nliteral_sections].size =
				(uint32_t)(object_size - s.offset);
			}
			else
			    literal_sections[nliteral_sections].size = s.size;
			nliteral_sections++;
		    }

		    if(p + sizeof(struct section) >
		       (char *)load_commands + sizeofcmds)
			break;
		    p += size;
		}
		break;
	    case LC_SEGMENT_64:
		memset((char *)&sg64, '\0', sizeof(struct segment_command_64));
		size = left < sizeof(struct segment_command_64) ?
		       left : sizeof(struct segment_command_64);
		memcpy((char *)&sg64, (char *)lc, size);
		if(swapped)
		    swap_segment_command_64(&sg64, host_byte_sex);

		p = (char *)lc + sizeof(struct segment_command_64);
		for(j = 0 ; j < sg64.nsects ; j++){
		    if(p + sizeof(struct section_64) >
		       (char *)load_commands + sizeofcmds){
			printf("section structure command extends past "
			       "end of load commands\n");
		    }
		    left = sizeofcmds - (uint32_t)(p - (char *)load_commands);
		    memset((char *)&s64, '\0', sizeof(struct section_64));
		    size = left < sizeof(struct section_64) ?
			   left : sizeof(struct section_64);
		    memcpy((char *)&s64, p, size);
		    if(swapped)
			swap_section_64(&s64, 1, host_byte_sex);

		    if(s64.flags == S_CSTRING_LITERALS ||
		       s64.flags == S_4BYTE_LITERALS ||
		       s64.flags == S_8BYTE_LITERALS ||
		       s64.flags == S_16BYTE_LITERALS){
			literal_sections = reallocate(literal_sections,
						sizeof(struct literal_section) *
						(nliteral_sections + 1));
			memcpy(literal_sections[nliteral_sections].segname,
			       s64.segname, 16);
			memcpy(literal_sections[nliteral_sections].sectname,
			       s64.sectname, 16);
        		literal_sections[nliteral_sections].addr = s64.addr;
			literal_sections[nliteral_sections].flags = s64.flags;
			literal_sections[nliteral_sections].contents = 
						    object_addr + s64.offset;
			big_size = s64.offset;
			big_size += s64.size;
			if(s64.offset > object_size){
			    printf("section contents of: (%.16s,%.16s) is past "
				   "end of file\n", s64.segname, s64.sectname);
			    literal_sections[nliteral_sections].size =  0;
			}
			else if(big_size > object_size){
			    printf("part of section contents of: (%.16s,%.16s) "
				   "is past end of file\n",
				   s64.segname, s64.sectname);
			    literal_sections[nliteral_sections].size =
				(uint32_t)(object_size - s64.offset);
			}
			else
			    literal_sections[nliteral_sections].size =
				(uint32_t)s64.size;
			nliteral_sections++;
		    }

		    if(p + sizeof(struct section) >
		       (char *)load_commands + sizeofcmds)
			break;
		    p += size;
		}
		break;
	    }
	    if(lcmd.cmdsize == 0){
		printf("load command %u size zero (can't advance to other "
		       "load commands)\n", i);
		break;
	    }
	    lc = (struct load_command *)((char *)lc + lcmd.cmdsize);
	    if((char *)lc > (char *)load_commands + sizeofcmds)
		break;
	}

	/* loop through the literal pointer section and print the pointers */
	if(cputype & CPU_ARCH_ABI64)
	    lp_size = 8;
	else
	    lp_size = 4;
	for(i = 0; i < sect_size ; i += lp_size){
	    if(print_addresses == TRUE){
	        if(cputype & CPU_ARCH_ABI64)
		    printf("%016llx  ", sect_addr + i);
		else
		    printf("%08x  ", (unsigned int)(sect_addr + i));
	    }
	    if(cputype & CPU_ARCH_ABI64){
		lp = (uint64_t)*((uint64_t *)(sect + i));
		memcpy((char *)&lp, sect + i, sizeof(uint64_t));
		if(swapped)
		    lp = SWAP_LONG_LONG(lp);
		/* Clear out the bits for threaded rebase/bind */
		if(cputype == CPU_TYPE_ARM64 &&
		   cpusubtype == CPU_SUBTYPE_ARM64E){
		    if(filetype == MH_OBJECT){
			if(lp & 0x8000000000000000ULL){
			    lp = 0xffffffffULL & lp;
			    if((lp & 0x80000000ULL) != 0)
				lp |= 0xffffffff00000000ULL;
			}
		    }
		    else{
			if(lp & 0x8000000000000000ULL)
			    lp = 0xffffffffULL & lp;
			else
			    lp = 0x0007ffffffffffffULL & lp;
		    }
		}
	    }
	    else{
		li = (int32_t)*((int32_t *)(sect + i));
		memcpy((char *)&li, sect + i, sizeof(uint32_t));
		if(swapped)
		    li = SWAP_INT(li);
		lp = li;
	    }
	    /*
	     * If there is an external relocation entry for this pointer then
	     * print the symbol and any offset.
	     */
	    reloc = bsearch(&i, relocs, nrelocs, sizeof(struct relocation_info),
			    (int (*)(const void *, const void *))rel_bsearch);
	    if(reloc != NULL && (reloc->r_address & R_SCATTERED) == 0 &&
	       reloc->r_extern == 1){
		printf("external relocation entry for symbol:");
		if(reloc->r_symbolnum < nsymbols){
		    if(symbols != NULL)
			n_strx = symbols[reloc->r_symbolnum].n_un.n_strx;
		    else
			n_strx = symbols64[reloc->r_symbolnum].n_un.n_strx;
		    if(n_strx < strings_size){
			if(lp != 0)
			    printf("%s+0x%llx\n", strings + n_strx, lp);
			else
			    printf("%s\n", strings + n_strx);
		    }
		    else{
			printf("bad string index for symbol: %u\n",
			       reloc->r_symbolnum);
		    }
		}
		else{
		    printf("bad relocation entry\n");
		}
		continue;
	    }
	    found = FALSE;
	    for(j = 0; j < nliteral_sections; j++){
		if(lp >= literal_sections[j].addr &&
		   lp < literal_sections[j].addr +
		        literal_sections[j].size){
		    printf("%.16s:%.16s:", literal_sections[j].segname,
			   literal_sections[j].sectname);
		    switch(literal_sections[j].flags){
		    case S_CSTRING_LITERALS:
			for(k = lp - literal_sections[j].addr;
			    k < literal_sections[j].size &&
					literal_sections[j].contents[k] != '\0';
			    k++)
			    print_cstring_char(literal_sections[j].contents[k]);
			printf("\n");
			break;
		    case S_4BYTE_LITERALS:
			memcpy((char *)&f,
			       (char *)(literal_sections[j].contents +
					lp - literal_sections[j].addr),
				sizeof(float));
			memcpy((char *)&l0,
			       (char *)(literal_sections[j].contents +
					lp - literal_sections[j].addr),
				sizeof(uint32_t));
			if(swapped){
			    d = SWAP_DOUBLE(d);
			    l0 = SWAP_INT(l0);
			}
			print_literal4(l0, f);
			break;
		    case S_8BYTE_LITERALS:
			memcpy((char *)&d,
			       (char *)(literal_sections[j].contents +
					lp - literal_sections[j].addr),
				sizeof(double));
			memcpy((char *)&l0,
			       (char *)(literal_sections[j].contents +
					lp - literal_sections[j].addr),
				sizeof(uint32_t));
			memcpy((char *)&l1,
			       (char *)(literal_sections[j].contents +
					lp - literal_sections[j].addr +
					sizeof(uint32_t)),
			       sizeof(uint32_t));
			if(swapped){
			    d = SWAP_DOUBLE(d);
			    l0 = SWAP_INT(l0);
			    l1 = SWAP_INT(l1);
			}
			print_literal8(l0, l1, d, object_byte_sex);
			break;
		    case S_16BYTE_LITERALS:
			memcpy((char *)&l0,
			       (char *)(literal_sections[j].contents +
					lp - literal_sections[j].addr),
				sizeof(uint32_t));
			memcpy((char *)&l1,
			       (char *)(literal_sections[j].contents +
					lp - literal_sections[j].addr +
					sizeof(uint32_t)),
			       sizeof(uint32_t));
			memcpy((char *)&l2,
			       (char *)(literal_sections[j].contents +
					lp - literal_sections[j].addr +
					2 * sizeof(uint32_t)),
			       sizeof(uint32_t));
			memcpy((char *)&l3,
			       (char *)(literal_sections[j].contents +
					lp - literal_sections[j].addr +
					3 * sizeof(uint32_t)),
			       sizeof(uint32_t));
			if(swapped){
			    l0 = SWAP_INT(l0);
			    l1 = SWAP_INT(l1);
			    l2 = SWAP_INT(l2);
			    l3 = SWAP_INT(l3);
			}
			print_literal16(l0, l1, l2, l3);
			break;
		    }
		    found = TRUE;
		    break;
		}
	    }
	    if(found == FALSE)
		printf("0x%llx (not in a literal section)\n", lp);
	}

	if(literal_sections != NULL)
	    free(literal_sections);
}

void
print_init_term_pointer_section(
cpu_type_t cputype,
char *sect,
uint64_t sect_size,
uint64_t sect_addr,
enum byte_sex object_byte_sex,
struct symbol *sorted_symbols,
uint32_t nsorted_symbols,
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
uint32_t strings_size,
struct relocation_info *relocs,
uint32_t nrelocs,
enum bool verbose)
{
    uint64_t i, stride;
    uint32_t p;
    uint64_t q, lp;
    enum byte_sex host_byte_sex;
    enum bool swapped;
    const char *name;
    struct relocation_info *reloc;
    uint32_t n_strx;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != object_byte_sex;
	p = 0;
	q = 0;
    
	if(cputype & CPU_ARCH_ABI64)
	    stride = sizeof(uint64_t);
	else
	    stride = sizeof(uint32_t);

	for(i = 0 ; i < sect_size; i += stride){
	    if(cputype & CPU_ARCH_ABI64)
		printf("0x%016llx ", sect_addr + i * stride);
	    else
		printf("0x%08x ",(uint32_t)(sect_addr + i * stride));

	    if(cputype & CPU_ARCH_ABI64)
		memcpy(&q, sect + i, stride);
	    else
		memcpy(&p, sect + i, stride);

	    if(swapped == TRUE){
		if(cputype & CPU_ARCH_ABI64)
		     q = SWAP_LONG_LONG(q);
		else
		     p = SWAP_INT(p);
	    }
	    if(cputype & CPU_ARCH_ABI64){
		printf("0x%016llx", q);
		lp = q;
	    } else {
		printf("0x%08x", p);
		lp = p;
	    }

	    if(verbose == TRUE){
		/*
		 * If there is an external relocation entry for this pointer then
		 * print the symbol and any offset.
		 */
		reloc = bsearch(&i, relocs, nrelocs,
				sizeof(struct relocation_info),
				(int (*)(const void *, const void *))
				rel_bsearch);
		if(reloc != NULL && (reloc->r_address & R_SCATTERED) == 0 &&
		   reloc->r_extern == 1){
		    if(reloc->r_symbolnum < nsymbols){
			if(symbols != NULL)
			    n_strx = symbols[reloc->r_symbolnum].n_un.n_strx;
			else
			    n_strx = symbols64[reloc->r_symbolnum].n_un.n_strx;
			if(n_strx < strings_size){
			    if(lp != 0)
				printf(" %s+0x%llx\n", strings + n_strx, lp);
			    else
				printf(" %s\n", strings + n_strx);
			}
			else{
			    printf("bad string index for symbol: %u\n",
				   reloc->r_symbolnum);
			}
		    }
		    else{
			printf("bad relocation entry\n");
		    }
		} else {
		    if(cputype & CPU_ARCH_ABI64)
			name = guess_symbol(q, sorted_symbols, nsorted_symbols,
					    verbose);
		    else
			name = guess_symbol(p, sorted_symbols, nsorted_symbols,
					    verbose);
		    if(name != NULL)
			printf(" %s\n", name);
		    else
			printf("\n");
		}
	    }
	    else{
		printf("\n");
	    }
	}
}

/*
 * Function for bsearch for searching relocation entries.
 */
static
int
rel_bsearch(
uint32_t *address,
struct relocation_info *rel)
{
    struct scattered_relocation_info *srel;
    uint32_t r_address;

	if((rel->r_address & R_SCATTERED) != 0){
	    srel = (struct scattered_relocation_info *)rel;
	    r_address = srel->r_address;
	}
	else
	    r_address = rel->r_address;

	if(*address == r_address)
	    return(0);
	if(*address < r_address)
	    return(-1);
	else
	    return(1);
}

/*
 * Print the shared library initialization table.
 */
void
print_shlib_init(
enum byte_sex object_byte_sex,
char *sect,
uint64_t sect_size,
uint64_t sect_addr,
struct symbol *sorted_symbols,
uint32_t nsorted_symbols,
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
uint32_t strings_size,
struct relocation_info *relocs,
uint32_t nrelocs,
enum bool verbose)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    uint64_t i;
    struct shlib_init {
	int32_t value;		/* the value to be stored at the address */
	int32_t address;	/* the address to store the value */
    } shlib_init;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != object_byte_sex;

	for(i = 0; i < sect_size; i += sizeof(struct shlib_init)){
	    memcpy((char *)&shlib_init, sect + i, sizeof(struct shlib_init));
	    if(swapped){
		shlib_init.value = SWAP_INT(shlib_init.value);
		shlib_init.address = SWAP_INT(shlib_init.address);
	    }
	    printf("\tvalue   0x%08x ", (unsigned int)shlib_init.value);
	    (void)print_symbol(shlib_init.value, (uint32_t)(sect_addr + i), 0,
			       relocs, nrelocs, symbols, symbols64, nsymbols,
			       sorted_symbols, nsorted_symbols, strings,
			       strings_size, verbose);
	    printf("\n");
	    printf("\taddress 0x%08x ", (unsigned int)shlib_init.address);
	    (void)print_symbol(shlib_init.address,
			       (uint32_t)(sect_addr+i+sizeof(int32_t)), 0,
			       relocs, nrelocs, symbols, symbols64, nsymbols,
			       sorted_symbols, nsorted_symbols, strings,
			       strings_size, verbose);
	    printf("\n");
	}
}

/*
 * Print_symbol prints a symbol name for the addr if a symbol exist with the
 * same address.  Nothing else is printed, no whitespace, no newline.  If it
 * prints something then it returns TRUE, else it returns FALSE.
 */
enum bool
print_symbol(
uint64_t value,
uint32_t r_address,
uint32_t dot_value,
struct relocation_info *relocs,
uint32_t nrelocs,
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsymbols,
struct symbol *sorted_symbols,
uint32_t nsorted_symbols,
char *strings,
uint32_t strings_size,
enum bool verbose)
{
    uint32_t i, offset;
    struct scattered_relocation_info *sreloc, *pair;
    unsigned int r_symbolnum;
    uint32_t n_strx;
    const char *name, *add, *sub;

	if(verbose == FALSE)
	    return(FALSE);

	for(i = 0; i < nrelocs; i++){
	    if(((relocs[i].r_address) & R_SCATTERED) != 0){
		sreloc = (struct scattered_relocation_info *)(relocs + i);
		if(sreloc->r_type == GENERIC_RELOC_PAIR){
		    fprintf(stderr, "Stray GENERIC_RELOC_PAIR relocation entry "
			    "%u\n", i);
		    continue;
		}
		if(sreloc->r_type == GENERIC_RELOC_VANILLA){
		    if(sreloc->r_address == r_address){
			name = guess_symbol(sreloc->r_value, sorted_symbols,
					    nsorted_symbols, verbose);
			offset = (uint32_t)(value - sreloc->r_value);
			if(name != NULL){
			    printf("%s+0x%x", name, (unsigned int)offset);
			    return(TRUE);
			}
		    }
		    continue;
		}
		if(sreloc->r_type != GENERIC_RELOC_SECTDIFF &&
		   sreloc->r_type != GENERIC_RELOC_LOCAL_SECTDIFF){
		    fprintf(stderr, "Unknown relocation r_type for entry "
			    "%u\n", i);
		    continue;
		}
		if(i + 1 < nrelocs){
		    pair = (struct scattered_relocation_info *)(relocs + i + 1);
		    if(pair->r_scattered == 0 ||
		       pair->r_type != GENERIC_RELOC_PAIR){
			fprintf(stderr, "No GENERIC_RELOC_PAIR relocation "
				"entry after entry %u\n", i);
			continue;
		    }
		}
		else{
		    fprintf(stderr, "No GENERIC_RELOC_PAIR relocation entry "
			    "after entry %u\n", i);
		    continue;
		}
		i++; /* skip the pair reloc */

		if(sreloc->r_address == r_address){
		    add = guess_symbol(sreloc->r_value, sorted_symbols,
				       nsorted_symbols, verbose);
		    sub = guess_symbol(pair->r_value, sorted_symbols,
				       nsorted_symbols, verbose);
		    offset = (uint32_t)(value -
					(sreloc->r_value - pair->r_value));
		    if(add != NULL)
			printf("%s", add);
		    else
			printf("0x%x", (unsigned int)sreloc->r_value);
		    if(sub != NULL)
			printf("-%s", sub);
		    else{
			if((uint32_t)pair->r_value == dot_value)
			    printf("-.");
			else
			    printf("-0x%x", (unsigned int)pair->r_value);
		    }
		    if(offset != 0)
			printf("+0x%x", (unsigned int)offset);
		    return(TRUE);
		}
	    }
	    else{
		if((uint32_t)relocs[i].r_address == r_address){
		    r_symbolnum = relocs[i].r_symbolnum;
		    if(relocs[i].r_extern){
		        if(r_symbolnum >= nsymbols)
			    return(FALSE);
			if(symbols != NULL)
			    n_strx = symbols[r_symbolnum].n_un.n_strx;
			else
			    n_strx = symbols64[r_symbolnum].n_un.n_strx;
			if(n_strx <= 0 || n_strx >= strings_size)
			    return(FALSE);
			if(value != 0)
			    printf("%s+0x%x", strings + n_strx,
				   (unsigned int)value);
			else
			    printf("%s", strings + n_strx);
			return(TRUE);
		    }
		    break;
		}
	    }
	}

	name = guess_symbol(value, sorted_symbols, nsorted_symbols, verbose);
	if(name != NULL){
	    printf("%s", name);
	    return(TRUE);
	}
	return(FALSE);
}

/*
 * guess_symbol() guesses the name for a symbol based on the specified value.
 * It returns the name of symbol or NULL.  It only returns a symbol name if
 *  a symbol with that exact value exists.
 */
const char *
guess_symbol(
const uint64_t value,	/* the value of this symbol (in) */
const struct symbol *sorted_symbols,
const uint32_t nsorted_symbols,
const enum bool verbose)
{
    int32_t high, low, mid;

	if(verbose == FALSE)
	    return(NULL);

	low = 0;
	high = nsorted_symbols - 1;
	mid = (high - low) / 2;
	while(high >= low){
	    if(sorted_symbols[mid].n_value == value){
		return(sorted_symbols[mid].name);
	    }
	    if(sorted_symbols[mid].n_value > value){
		high = mid - 1;
		mid = (high + low) / 2;
	    }
	    else{
		low = mid + 1;
		mid = (high + low) / 2;
	    }
	}
	return(NULL);
}

/*
 * guess_indirect_symbol() returns the name of the indirect symbol for the
 * value passed in or NULL.
 */
const char *
guess_indirect_symbol(
const uint64_t value,	/* the value of this symbol (in) */
const uint32_t ncmds,
const uint32_t sizeofcmds,
const struct load_command *load_commands,
const enum byte_sex load_commands_byte_sex,
const uint32_t *indirect_symbols,
const uint32_t nindirect_symbols,
const struct nlist *symbols,
const struct nlist_64 *symbols64,
const uint32_t nsymbols,
const char *strings,
const uint32_t strings_size)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    uint32_t i, j, section_type, index, stride;
    const struct load_command *lc;
    struct load_command l;
    struct segment_command sg;
    struct section s;
    struct segment_command_64 sg64;
    struct section_64 s64;
    char *p;
    uint64_t big_load_end;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != load_commands_byte_sex;

	lc = load_commands;
	big_load_end = 0;
	for(i = 0 ; i < ncmds; i++){
	    memcpy((char *)&l, (char *)lc, sizeof(struct load_command));
	    if(swapped)
		swap_load_command(&l, host_byte_sex);
	    if(l.cmdsize % sizeof(int32_t) != 0)
		return(NULL);
	    big_load_end += l.cmdsize;
	    if(big_load_end > sizeofcmds)
		return(NULL);
	    switch(l.cmd){
	    case LC_SEGMENT:
		memcpy((char *)&sg, (char *)lc, sizeof(struct segment_command));
		if(swapped)
		    swap_segment_command(&sg, host_byte_sex);
		p = (char *)lc + sizeof(struct segment_command);
		for(j = 0 ; j < sg.nsects &&
			    j * sizeof(struct section) +
			    sizeof(struct segment_command) < sizeofcmds ;
                    j++){
		    memcpy((char *)&s, p, sizeof(struct section));
		    p += sizeof(struct section);
		    if(swapped)
			swap_section(&s, 1, host_byte_sex);
		    section_type = s.flags & SECTION_TYPE;
		    if((section_type == S_NON_LAZY_SYMBOL_POINTERS ||
		        section_type == S_LAZY_SYMBOL_POINTERS ||
		        section_type == S_LAZY_DYLIB_SYMBOL_POINTERS ||
		        section_type == S_THREAD_LOCAL_VARIABLE_POINTERS ||
		        section_type == S_SYMBOL_STUBS) &&
		        value >= s.addr && value < s.addr + s.size){
			if(section_type == S_SYMBOL_STUBS)
			    stride = s.reserved2;
			else
			    stride = 4;
			if(stride == 0)
			    return(NULL);
			index = (uint32_t)(s.reserved1 + (value - s.addr) /
					   stride);
			if(index < nindirect_symbols &&
		    	   symbols != NULL && strings != NULL &&
		           indirect_symbols[index] < nsymbols &&
		           (uint32_t)symbols[indirect_symbols[index]].
				n_un.n_strx < strings_size)
			    return(strings +
				symbols[indirect_symbols[index]].n_un.n_strx);
			else
			    return(NULL);
		    }
		}
		break;
	    case LC_SEGMENT_64:
		memcpy((char *)&sg64, (char *)lc,
		       sizeof(struct segment_command_64));
		if(swapped)
		    swap_segment_command_64(&sg64, host_byte_sex);
		p = (char *)lc + sizeof(struct segment_command_64);
		for(j = 0 ; j < sg64.nsects &&
			    j * sizeof(struct section_64) +
			    sizeof(struct segment_command_64) < sizeofcmds ;
                    j++){
		    memcpy((char *)&s64, p, sizeof(struct section_64));
		    p += sizeof(struct section_64);
		    if(swapped)
			swap_section_64(&s64, 1, host_byte_sex);
		    section_type = s64.flags & SECTION_TYPE;
		    if((section_type == S_NON_LAZY_SYMBOL_POINTERS ||
		        section_type == S_LAZY_SYMBOL_POINTERS ||
		        section_type == S_LAZY_DYLIB_SYMBOL_POINTERS ||
	   		section_type == S_THREAD_LOCAL_VARIABLE_POINTERS ||
		        section_type == S_SYMBOL_STUBS) &&
		        value >= s64.addr && value < s64.addr + s64.size){
			if(section_type == S_SYMBOL_STUBS)
			    stride = s64.reserved2;
			else
			    stride = 8;
			if(stride == 0)
			    return(NULL);
			index = (uint32_t)(s64.reserved1 + (value - s64.addr) /
					   stride);
			if(index < nindirect_symbols &&
		    	   symbols64 != NULL && strings != NULL &&
		           indirect_symbols[index] < nsymbols &&
		           (uint32_t)symbols64[indirect_symbols[index]].
				n_un.n_strx < strings_size)
			    return(strings +
				symbols64[indirect_symbols[index]].n_un.n_strx);
			else
			    return(NULL);
		    }
		}
		break;
	    }
	    if(l.cmdsize == 0){
		return(NULL);
	    }
	    lc = (struct load_command *)((char *)lc + l.cmdsize);
	    if((char *)lc > (char *)load_commands + sizeofcmds)
		return(NULL);
	}
	return(NULL);
}

void
print_sect(
cpu_type_t cputype,
enum byte_sex object_byte_sex,
char *sect,
uint64_t size,
uint64_t addr)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    uint64_t i, j, k;
    uint32_t long_word;
    unsigned short short_word;
    unsigned char byte_word;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != object_byte_sex;

	if(cputype == CPU_TYPE_I386 ||
	   cputype == CPU_TYPE_X86_64){
	    for(i = 0 ; i < size ; i += j , addr += j){
		if(cputype & CPU_ARCH_ABI64)
		    printf("%016llx\t", addr);
		else
		    printf("%08x\t", (uint32_t)addr);
		for(j = 0;
		    j < 16 * sizeof(char) && i + j < size;
		    j += sizeof(char)){
		    byte_word = *(sect + i + j);
		    printf("%02x ", (unsigned int)byte_word);
		}
		printf("\n");
	    }
	}
	else if(cputype == CPU_TYPE_MC680x0){
	    for(i = 0 ; i < size ; i += j , addr += j){
		printf("%08x ", (unsigned int)addr);
		for(j = 0;
		    j < 8 * sizeof(short) && i + j < size;
		    j += sizeof(short)){
		    memcpy(&short_word, sect + i + j, sizeof(short));
		    if(swapped)
			short_word = SWAP_SHORT(short_word);
		    printf("%04x ", (unsigned int)short_word);
		}
		printf("\n");
	    }
	}
	else{
	    for(i = 0 ; i < size ; i += j , addr += j){
		if(cputype & CPU_ARCH_ABI64)
		    printf("%016llx\t", addr);
		else
		    printf("%08x\t", (uint32_t)addr);
		for(j = 0;
		    j < 4 * sizeof(int32_t) && i + j < size;
		    j += sizeof(int32_t)){
		    if(i + j + sizeof(int32_t) <= size){
			memcpy(&long_word, sect + i + j, sizeof(int32_t));
			if(swapped)
			    long_word = SWAP_INT(long_word);
			printf("%08x ", (unsigned int)long_word);
		    }
		    else{
			for(k = 0; i + j + k < size; k++){
			    byte_word = *(sect + i + j + k);
			    printf("%02x ", (unsigned int)byte_word);
			}
		    }
		}
		printf("\n");
	    }
	}
}

/*
 * get_label returns a symbol name for the addr if a symbol exist with the
 * same address else it returns NULL.
 */
char *
get_label(
uint64_t addr,
struct symbol *sorted_symbols,
uint32_t nsorted_symbols)
{
    int32_t high, low, mid;

	low = 0;
	high = nsorted_symbols - 1;
	mid = (high - low) / 2;
	while(high >= low){
	    if(sorted_symbols[mid].n_value == addr)
		return(sorted_symbols[mid].name);
	    if(sorted_symbols[mid].n_value > addr){
		high = mid - 1;
		mid = (high + low) / 2;
	    }
	    else{
		low = mid + 1;
		mid = (high + low) / 2;
	    }
	}
	return(NULL);
}

/*
 * Print_label prints a symbol name for the addr if a symbol exist with the
 * same address in label form, namely:.
 *
 * <symbol name>:\n
 *
 * The colon and the newline are printed if colon_and_newline is TRUE.
 * If it prints a label it returns TRUE else it returns FALSE.
 */
enum bool
print_label(
uint64_t addr,
enum bool colon_and_newline,
struct symbol *sorted_symbols,
uint32_t nsorted_symbols)
{
    int32_t high, low, mid;

	low = 0;
	high = nsorted_symbols - 1;
	mid = (high - low) / 2;
	while(high >= low){
	    if(sorted_symbols[mid].n_value == addr){
		printf("%s", sorted_symbols[mid].name);
		if(colon_and_newline == TRUE)
		    printf(":\n");
		return(TRUE);
	    }
	    if(sorted_symbols[mid].n_value > addr){
		high = mid - 1;
		mid = (high + low) / 2;
	    }
	    else{
		low = mid + 1;
		mid = (high + low) / 2;
	    }
	}
	return(FALSE);
}
