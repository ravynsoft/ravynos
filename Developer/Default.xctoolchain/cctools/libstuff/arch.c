/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#ifndef RLD
#include "stdio.h"
#endif /* !defined(RLD) */
#include "stdlib.h"
#include "string.h"
#include <mach/mach.h>
#include "stuff/openstep_mach.h"
#include "stuff/arch.h"
#include "stuff/allocate.h"

/*
 * The array of all currently know architecture flags (terminated with an entry
 * with all zeros).  Pointer to this returned with get_arch_flags().
 */
#ifdef __DYNAMIC__
static struct arch_flag arch_flags[] = {
#else
static const struct arch_flag arch_flags[] = {
#endif
    { "any",	CPU_TYPE_ANY,	  CPU_SUBTYPE_MULTIPLE },
    { "little",	CPU_TYPE_ANY,	  CPU_SUBTYPE_LITTLE_ENDIAN },
    { "big",	CPU_TYPE_ANY,	  CPU_SUBTYPE_BIG_ENDIAN },

/* 64-bit Mach-O architectures */

    /* architecture families */
    { "ppc64",     CPU_TYPE_POWERPC64, CPU_SUBTYPE_POWERPC_ALL },
    { "x86_64",    CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_ALL },
    { "x86_64h",   CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_H },
    { "arm64",     CPU_TYPE_ARM64,     CPU_SUBTYPE_ARM64_ALL },
    /* specific architecture implementations */
    { "ppc970-64", CPU_TYPE_POWERPC64, CPU_SUBTYPE_POWERPC_970 },
    { "arm64_32",  CPU_TYPE_ARM64_32,  CPU_SUBTYPE_ARM64_32_V8 },
    { "arm64e",    CPU_TYPE_ARM64,  CPU_SUBTYPE_ARM64E },

/* 32-bit Mach-O architectures */

    /* architecture families */
    { "ppc",    CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_ALL },
    { "i386",   CPU_TYPE_I386,    CPU_SUBTYPE_I386_ALL },
    { "m68k",   CPU_TYPE_MC680x0, CPU_SUBTYPE_MC680x0_ALL },
    { "hppa",   CPU_TYPE_HPPA,    CPU_SUBTYPE_HPPA_ALL },
    { "sparc",	CPU_TYPE_SPARC,   CPU_SUBTYPE_SPARC_ALL },
    { "m88k",   CPU_TYPE_MC88000, CPU_SUBTYPE_MC88000_ALL },
    { "i860",   CPU_TYPE_I860,    CPU_SUBTYPE_I860_ALL },
    { "veo",    CPU_TYPE_VEO,     CPU_SUBTYPE_VEO_ALL },
    { "arm",    CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_ALL },
    /* specific architecture implementations */
    { "ppc601", CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_601 },
    { "ppc603", CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_603 },
    { "ppc603e",CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_603e },
    { "ppc603ev",CPU_TYPE_POWERPC,CPU_SUBTYPE_POWERPC_603ev },
    { "ppc604", CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_604 },
    { "ppc604e",CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_604e },
    { "ppc750", CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_750 },
    { "ppc7400",CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_7400 },
    { "ppc7450",CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_7450 },
    { "ppc970", CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_970 },
    { "i486",   CPU_TYPE_I386,    CPU_SUBTYPE_486 },
    { "i486SX", CPU_TYPE_I386,    CPU_SUBTYPE_486SX },
    { "pentium",CPU_TYPE_I386,    CPU_SUBTYPE_PENT }, /* same as i586 */
    { "i586",   CPU_TYPE_I386,    CPU_SUBTYPE_586 },
    { "pentpro", CPU_TYPE_I386, CPU_SUBTYPE_PENTPRO }, /* same as i686 */
    { "i686",   CPU_TYPE_I386, CPU_SUBTYPE_PENTPRO },
    { "pentIIm3",CPU_TYPE_I386, CPU_SUBTYPE_PENTII_M3 },
    { "pentIIm5",CPU_TYPE_I386, CPU_SUBTYPE_PENTII_M5 },
    { "pentium4",CPU_TYPE_I386, CPU_SUBTYPE_PENTIUM_4 },
    { "m68030", CPU_TYPE_MC680x0, CPU_SUBTYPE_MC68030_ONLY },
    { "m68040", CPU_TYPE_MC680x0, CPU_SUBTYPE_MC68040 },
    { "hppa7100LC", CPU_TYPE_HPPA,  CPU_SUBTYPE_HPPA_7100LC },
    { "veo1",   CPU_TYPE_VEO,     CPU_SUBTYPE_VEO_1 },
    { "veo2",   CPU_TYPE_VEO,     CPU_SUBTYPE_VEO_2 },
    { "veo3",   CPU_TYPE_VEO,     CPU_SUBTYPE_VEO_3 },
    { "veo4",   CPU_TYPE_VEO,     CPU_SUBTYPE_VEO_4 },
    { "armv4t", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V4T},
    { "armv5",  CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V5TEJ},
    { "xscale", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_XSCALE},
    { "armv6",  CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V6 },
    { "armv6m", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V6M },
    { "armv7",  CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V7 },
    { "armv7f", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V7F },
    { "armv7s", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V7S },
    { "armv7k", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V7K },
    { "armv7m", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V7M },
    { "armv7em", CPU_TYPE_ARM,    CPU_SUBTYPE_ARM_V7EM },
    { "arm64v8",CPU_TYPE_ARM64,   CPU_SUBTYPE_ARM64_V8 },
    { NULL,	0,		  0 }
};

struct cpu_entry {
    cpu_type_t 		cputype;
    enum byte_sex	endian;
    uint64_t 		staddr;
    uint32_t		segalign;
};

/*
 * The cpu_entries table holds vital statistics for a number of entries in the
 * arch_info table. Functions such as get_byte_sex_from_flag() are driven off
 * the contents of this table, making it easier to add new cputypes. Rows in
 * the table are organized by frequency, with the most common / recent at the
 * top of the list.
 *
 * Note that some historical architectures, such as NeXTSTEP's i386 and
 * Rhapsody's PPC, are no longer addressible with this design.
 */
 
static const struct cpu_entry cpu_entries[] = {
    /* embedded */
    { CPU_TYPE_ARM64,	    LITTLE_ENDIAN_BYTE_SEX, 0,		       0x4000 },
    { CPU_TYPE_ARM64_32,    LITTLE_ENDIAN_BYTE_SEX, 0,		       0x4000 },
    { CPU_TYPE_ARM,	    LITTLE_ENDIAN_BYTE_SEX, 0,		       0x4000 },
    
    /* desktop */
    { CPU_TYPE_X86_64,	    LITTLE_ENDIAN_BYTE_SEX, 0x7fff5fc00000LL,  0x1000 },
    { CPU_TYPE_I386,	    LITTLE_ENDIAN_BYTE_SEX, 0xc0000000,        0x1000 },
    { CPU_TYPE_POWERPC,	    BIG_ENDIAN_BYTE_SEX,    0xc0000000,	       0x1000 },
    { CPU_TYPE_POWERPC64,   BIG_ENDIAN_BYTE_SEX,    0x7ffff00000000LL, 0x1000 },
    { CPU_TYPE_VEO,	    BIG_ENDIAN_BYTE_SEX,    0xc0000000,	       0x1000 },
    
    /* NeXTSTEP / Rhapsody */
    /*
    { CPU_TYPE_I386,	    LITTLE_ENDIAN_BYTE_SEX, 0xc0000000,        0x2000 },
    { CPU_TYPE_POWERPC,	    BIG_ENDIAN_BYTE_SEX,    0xc0000000,	       0x2000 },
     */
    { CPU_TYPE_MC680x0,     BIG_ENDIAN_BYTE_SEX,    0x04000000,	       0x2000 },
    { CPU_TYPE_MC88000,     BIG_ENDIAN_BYTE_SEX,    0xffffe000,	       0x2000 },
    { CPU_TYPE_SPARC,       BIG_ENDIAN_BYTE_SEX,    0xf0000000,	       0x2000 },
    { CPU_TYPE_I860,        BIG_ENDIAN_BYTE_SEX,    0,                 0x2000 },
    { CPU_TYPE_HPPA,        BIG_ENDIAN_BYTE_SEX,    0,                 0x2000 },
    { 0 },
};
    
#ifndef RLD
/*
 * get_arch_from_flag() is passed a name of an architecture flag and returns
 * zero if that flag is not known and non-zero if the flag is known.
 * If the pointer to the arch_flag is not NULL it is filled in with the
 * arch_flag struct that matches the name.
 */
__private_extern__
int
get_arch_from_flag(
char *name,
struct arch_flag *arch_flag)
{
    uint32_t i;

	for(i = 0; arch_flags[i].name != NULL; i++){
	    if(strcmp(arch_flags[i].name, name) == 0){
		if(arch_flag != NULL)
		    *arch_flag = arch_flags[i];
		return(1);
	    }
	}
	if(arch_flag != NULL)
	    memset(arch_flag, '\0', sizeof(struct arch_flag));
	return(0);
}

/*
 * get_arch_flags() returns a pointer to an array of all currently know
 * architecture flags (terminated with an entry with all zeros).
 */
__private_extern__
const struct arch_flag *
get_arch_flags(
void)
{
	return(arch_flags);
}
#endif /* !defined(RLD) */

/*
 * get_arch_name_from_types() returns the name of the architecture for the
 * specified cputype and cpusubtype if known.  If unknown it returns a pointer
 * to the an allocated string "cputype X cpusubtype Y" where X and Y are decimal
 * values.
 */
__private_extern__
const char *
get_arch_name_from_types(
cpu_type_t cputype,
cpu_subtype_t cpusubtype)
{
    const char *s;
    char *p;

	s = get_arch_name_if_known(cputype, cpusubtype);
	if (s == NULL) {
#ifndef RLD
	    p = savestr("cputype 1234567890 cpusubtype 1234567890");
	    if(p != NULL)
		sprintf(p, "cputype %u cpusubtype %u", cputype,
			cpusubtype & ~CPU_SUBTYPE_MASK);
#else
	    /* there is no sprintf() in the rld kernel API's */
	    p = savestr("cputype ?? cpusubtype ??");
#endif
	    s = p;
	}

	return(s);
}

/*
 * get_arch_name_if_known() returns the name of the architecture for the
 * specified cputype and cpusubtype if known.  If unknown it returns NULL.
 */
__private_extern__
const char *
get_arch_name_if_known(
cpu_type_t cputype,
cpu_subtype_t cpusubtype)
{
    uint32_t i;

	for(i = 0; arch_flags[i].name != NULL; i++){
	    if(arch_flags[i].cputype == cputype &&
	       (arch_flags[i].cpusubtype & ~CPU_SUBTYPE_MASK) ==
	       (cpusubtype & ~CPU_SUBTYPE_MASK))
		return(arch_flags[i].name);
	}

	return(NULL);
}

/*
 * get_arch_family_from_cputype() returns the family architecture for the
 * specified cputype if known.  If unknown it returns NULL.
 */
__private_extern__
const struct arch_flag *
get_arch_family_from_cputype(
cpu_type_t cputype)
{
    uint32_t i;

	/* arm64 is not to match a family but the specific arm64 arch */
	if(cputype == CPU_TYPE_ARM64)
	    return(NULL);
	for(i = 0; arch_flags[i].name != NULL; i++){
	    if(arch_flags[i].cputype == cputype)
		return(arch_flags + i);
	}
	return(NULL);
}

/*
 * abort_with_unknown_cputype() is a helper function to make calls to abort()
 * more descriptive in a symbolicated backtrace.
 */
static
void
abort_with_unknown_cputype(cpu_type_t cputype)
{
    abort();
}
    
/*
 * get_cpu_entry_from_cputype() for the specified cputype. If unknown it
 * calls abort_with_unknown_cputype().
 */
static
const struct cpu_entry *
get_cpu_entry_from_cputype(cpu_type_t cputype)
{
    for (uint32_t i = 0; cpu_entries[i].cputype != 0; ++i)
    {
	if (cpu_entries[i].cputype == cputype)
	    return cpu_entries + i;
    }
    
    abort_with_unknown_cputype(cputype);
    
    return NULL; /* unreachable */
}
    
/*
 * get_byte_sex_from_flag() returns the byte sex of the architecture for the
 * specified cputype and cpusubtype if known.  If unknown it returns
 * UNKNOWN_BYTE_SEX.  If the bytesex can be determined directly as in the case
 * of reading a magic number from a file that should be done and this routine
 * should not be used as it could be out of date.
 */
__private_extern__
enum byte_sex
get_byte_sex_from_flag(
const struct arch_flag *flag)
{
    const struct cpu_entry *entry = get_cpu_entry_from_cputype(flag->cputype);
    
    return entry->endian;
}

#ifndef RLD

/*
 * get_stack_addr_from_flag() returns the default starting address of the user
 * stack.  This should be in the header file <bsd/XXX/vmparam.h> as USRSTACK.
 * Since some architectures have come and gone and come back and because you
 * can't include all of these headers in one source the constants have been
 * copied here.
 */
__private_extern__
uint64_t
get_stack_addr_from_flag(
const struct arch_flag *flag)
{
    const struct cpu_entry *entry = get_cpu_entry_from_cputype(flag->cputype);
    
    return entry->staddr;
}

/*
 * get_segalign_from_flag() returns the default segment alignment (page size).
 */
__private_extern__
uint32_t
get_segalign_from_flag(
const struct arch_flag *flag)
{
    const struct cpu_entry *entry = get_cpu_entry_from_cputype(flag->cputype);
    
    return entry->segalign;
}

/*
 * get_shared_region_size_from_flag() returns the default shared
 * region size.
 */
__private_extern__
uint32_t
get_shared_region_size_from_flag(
const struct arch_flag *flag)
{
	if(flag->cputype == CPU_TYPE_ARM)
	   return (0x08000000);
	else
	   return (0x10000000);
}

/*
 * force_cpusubtype_ALL_for_cputype() takes a cputype and returns TRUE if for
 * that cputype the cpusubtype should always be forced to the ALL cpusubtype,
 * otherwise it returns FALSE.
 */
__private_extern__
enum bool
force_cpusubtype_ALL_for_cputype(
cpu_type_t cputype)
{
	if(cputype == CPU_TYPE_I386)
	    return(TRUE);
	else
	    return(FALSE);
}

#endif /* !defined(RLD) */
