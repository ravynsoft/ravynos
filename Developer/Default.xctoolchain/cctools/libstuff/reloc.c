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
#include <mach/machine.h>
#include <mach-o/reloc.h>
#include <mach-o/m88k/reloc.h>
#include <mach-o/ppc/reloc.h>
#include <mach-o/i860/reloc.h>
#include <mach-o/hppa/reloc.h>
#include <mach-o/sparc/reloc.h>
#include <mach-o/x86_64/reloc.h>
#include <mach-o/arm/reloc.h>
#include "stuff/bool.h"
#include "stuff/errors.h"
#include "stuff/reloc.h"

/*
 * reloc_pair_r_type() returns the PAIR constant for the specific cputype for
 * a paired relocation entry.
 */
__private_extern__
uint32_t
reloc_pair_r_type(
cpu_type_t cputype)
{
	switch(cputype){
	case CPU_TYPE_MC680x0:
	case CPU_TYPE_I386:
	    return(GENERIC_RELOC_PAIR);
	    break;
	case CPU_TYPE_X86_64:
		/*
		 * We should never hit this case for x86-64, so drop down to the
		 * fatal error below.
		 */
		break;
	case CPU_TYPE_MC88000:
	    return(M88K_RELOC_PAIR);
	    break;
	case CPU_TYPE_I860:
	    return(I860_RELOC_PAIR);
	    break;
	case CPU_TYPE_POWERPC:
	case CPU_TYPE_POWERPC64:
	case CPU_TYPE_VEO:
	    return(PPC_RELOC_PAIR);
	    break;
	case CPU_TYPE_HPPA:
	    return(HPPA_RELOC_PAIR);
	    break;
	case CPU_TYPE_SPARC:
	    return(SPARC_RELOC_PAIR);
	    break;
	case CPU_TYPE_ARM:
	    return(ARM_RELOC_PAIR);
	    break;
	case CPU_TYPE_ARM64:
	case CPU_TYPE_ARM64_32:
	    /*
	     * We should never hit this case for arm64 or arm64_32, so drop down
	     * to the fatal error below.
	     */
	    break;
	}
	fatal("internal error: reloc_pair_r_type() called with unknown "
	      "cputype (%u)", cputype);
	/* can't get here but to shut up the compiler warning ... */
	return(0);
}

/*
 * reloc_has_pair() returns TRUE if the specified r_type for the specified
 * cputype for has a paired relocation entry.
 */
__private_extern__
enum bool
reloc_has_pair(
cpu_type_t cputype,
uint32_t r_type)
{
	switch(cputype){
	case CPU_TYPE_MC680x0:
	case CPU_TYPE_I386:
	    if(r_type == GENERIC_RELOC_SECTDIFF ||
	       r_type == GENERIC_RELOC_LOCAL_SECTDIFF)
		return(TRUE);
	    break;
	case CPU_TYPE_X86_64:
		return(FALSE);
		break;
	case CPU_TYPE_MC88000:
	    if(r_type == M88K_RELOC_HI16 ||
	       r_type == M88K_RELOC_LO16 ||
	       r_type == M88K_RELOC_SECTDIFF)
		return(TRUE);
	    break;
	case CPU_TYPE_I860:
	    if(r_type == I860_RELOC_HIGH ||
	       r_type == I860_RELOC_HIGHADJ ||
	       r_type == I860_RELOC_SECTDIFF)
		return(TRUE);
	    break;
	case CPU_TYPE_POWERPC:
	case CPU_TYPE_POWERPC64:
	case CPU_TYPE_VEO:
	    if(r_type == PPC_RELOC_HI16 ||
	       r_type == PPC_RELOC_LO16 ||
	       r_type == PPC_RELOC_HA16 ||
	       r_type == PPC_RELOC_LO14 ||
	       r_type == PPC_RELOC_SECTDIFF ||
	       r_type == PPC_RELOC_LOCAL_SECTDIFF ||
	       r_type == PPC_RELOC_HI16_SECTDIFF ||
	       r_type == PPC_RELOC_LO16_SECTDIFF ||
	       r_type == PPC_RELOC_LO14_SECTDIFF ||
	       r_type == PPC_RELOC_HA16_SECTDIFF ||
	       r_type == PPC_RELOC_JBSR)
		return(TRUE);
	    break;
	case CPU_TYPE_HPPA:
	    if(r_type == HPPA_RELOC_HI21 ||
	       r_type == HPPA_RELOC_LO14 ||
	       r_type == HPPA_RELOC_BR17 ||
	       r_type == HPPA_RELOC_JBSR ||
	       r_type == HPPA_RELOC_SECTDIFF ||
	       r_type == HPPA_RELOC_HI21_SECTDIFF ||
	       r_type == HPPA_RELOC_LO14_SECTDIFF)
		return(TRUE);
	    break;
	case CPU_TYPE_SPARC:
	    if (r_type == SPARC_RELOC_HI22 ||
		r_type == SPARC_RELOC_LO10 ||
		r_type == SPARC_RELOC_HI22_SECTDIFF ||
		r_type == SPARC_RELOC_LO10_SECTDIFF ||
		r_type == SPARC_RELOC_SECTDIFF)
	      return(TRUE);
	    break;
	case CPU_TYPE_ARM:
	    if(r_type == ARM_RELOC_SECTDIFF ||
	       r_type == ARM_RELOC_LOCAL_SECTDIFF ||
	       r_type == ARM_RELOC_HALF ||
	       r_type == ARM_RELOC_HALF_SECTDIFF)
		return(TRUE);
	    break;
	case CPU_TYPE_ARM64:
	case CPU_TYPE_ARM64_32:
	    return(FALSE);
	default:
	    fatal("internal error: reloc_has_pair() called with unknown "
		  "cputype (%u)", cputype);
	}
	return(FALSE);
}

/*
 * reloc_is_sectdiff() returns TRUE if the specified r_type for the specified
 * cputype is a section difference relocation type.
 */
__private_extern__
enum bool
reloc_is_sectdiff(
cpu_type_t cputype,
uint32_t r_type)
{
	switch(cputype){
	case CPU_TYPE_MC680x0:
	case CPU_TYPE_I386:
	    if(r_type == GENERIC_RELOC_SECTDIFF ||
	       r_type == GENERIC_RELOC_LOCAL_SECTDIFF)
		return(TRUE);
	    break;
	case CPU_TYPE_X86_64:
		/* No sectdiff relocs for x86-64. */
		return(FALSE);
		break;
	case CPU_TYPE_MC88000:
	    if(r_type == M88K_RELOC_SECTDIFF)
		return(TRUE);
	    break;
	case CPU_TYPE_I860:
	    if(r_type == I860_RELOC_SECTDIFF)
		return(TRUE);
	    break;
	case CPU_TYPE_POWERPC:
	case CPU_TYPE_VEO:
	    if(r_type == PPC_RELOC_SECTDIFF ||
	       r_type == PPC_RELOC_LOCAL_SECTDIFF ||
	       r_type == PPC_RELOC_HI16_SECTDIFF ||
	       r_type == PPC_RELOC_LO16_SECTDIFF ||
	       r_type == PPC_RELOC_LO14_SECTDIFF ||
	       r_type == PPC_RELOC_HA16_SECTDIFF)
		return(TRUE);
	    break;
	case CPU_TYPE_HPPA:
	    if(r_type == HPPA_RELOC_SECTDIFF ||
	       r_type == HPPA_RELOC_HI21_SECTDIFF ||
	       r_type == HPPA_RELOC_LO14_SECTDIFF)
		return(TRUE);
	    break;
	case CPU_TYPE_SPARC:
	    if(r_type == SPARC_RELOC_SECTDIFF ||
	       r_type == SPARC_RELOC_HI22_SECTDIFF ||
	       r_type == SPARC_RELOC_LO10_SECTDIFF)
		return(TRUE);
	    break;
	case CPU_TYPE_ARM:
	    if(r_type == ARM_RELOC_SECTDIFF ||
	       r_type == ARM_RELOC_LOCAL_SECTDIFF ||
	       r_type == ARM_RELOC_HALF_SECTDIFF)
		return(TRUE);
	    break;
	case CPU_TYPE_ARM64:
	case CPU_TYPE_ARM64_32:
		/* No sectdiff relocs for arm64 or arm64_32. */
		return(FALSE);
		break;
	default:
	    fatal("internal error: reloc_is_sectdiff() called with unknown "
		  "cputype (%u)", cputype);
	}
	return(FALSE);
}
