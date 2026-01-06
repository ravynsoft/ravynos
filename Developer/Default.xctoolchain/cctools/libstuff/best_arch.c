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
#include <stdio.h>
#include <limits.h>
#endif /* RLD */
#include <mach-o/fat.h>
#include <stuff/best_arch.h>

#ifndef RLD
/*
 * internal_cpusubtype_findbestarch() is passed a cputype and cpusubtype and a
 * either set of fat_arch structs or fat_arch_64 structs and selects the best
 * one that matches (if any) and returns an index to the array of structs or
 * -1 if none works for the cputype and cpusubtype.  The fat_arch structs or
 * fat_arch_64 structs must be in the host byte sex and correct such that the
 * fat_archs really points to enough memory for nfat_arch structs.  It is
 * possible that this routine could fail if new cputypes or cpusubtypes are
 * added and an old version of this routine is used.  But if there is an exact
 * match between the cputype and cpusubtype and one of the structs this routine
 * will always succeed.
 */
static
int32_t
internal_cpusubtype_findbestarch(
cpu_type_t cputype,
cpu_subtype_t cpusubtype,
struct fat_arch *fat_archs,
struct fat_arch_64 *fat_archs64,
uint32_t nfat_archs)
{
    uint32_t i, lowest_index;
    long lowest_family, lowest_model;
    cpu_type_t fat_cputype;
    cpu_subtype_t fat_cpusubtype;

	/*
	 * Look for the first exact match.
	 */
	for(i = 0; i < nfat_archs; i++){
	    if(fat_archs64 != NULL){
		fat_cputype = fat_archs64[i].cputype;
		fat_cpusubtype = fat_archs64[i].cpusubtype;
	    }
	    else{
		fat_cputype = fat_archs[i].cputype;
		fat_cpusubtype = fat_archs[i].cpusubtype;
	    }
	    if(fat_cputype == cputype &&
	       (fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
	       (cpusubtype & ~CPU_SUBTYPE_MASK))
		return(i);
	}

	/*
	 * An exact match was not found so find the next best match which is
	 * cputype dependent.
	 */
	switch(cputype){

	/* 64-bit architectures */

	case CPU_TYPE_POWERPC64:
	    /*
	     * An exact match was not found.  So for all the PowerPC64 subtypes
	     * pick the subtype from the following order starting from a subtype
	     * that will work (contains 64-bit instructions or altivec if
	     * needed):
	     *	970 (currently only the one 64-bit subtype)
	     * For an unknown subtype pick only the ALL type if it exists.
	     */
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_POWERPC_ALL:
		/*
		 * The CPU_SUBTYPE_POWERPC_ALL is only used by the development
		 * environment tools when building a generic ALL type binary.
		 * In the case of a non-exact match we pick the most current
		 * processor.
		 */
	    case CPU_SUBTYPE_POWERPC_970:
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_POWERPC_970)
			return(i);
		}
	    default:
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_POWERPC_ALL)
			return(i);
		}
	    }
	    break;

	case CPU_TYPE_X86_64:
	    /*
	     * We have no subtypes for x86-64, so treat all cases the same here.
	     */
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    default:
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_I386_ALL)
			return(i);
		}
	    }
	    break;

	/* 32-bit architectures */

	case CPU_TYPE_I386:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    default:
		/*
		 * Intel cpusubtypes after the pentium (same as 586) are handled
		 * such that they require an exact match or they can use the
		 * pentium.  If that is not found call into the loop for the
		 * earilier subtypes.
		 */
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_PENT)
			return(i);
		}
	    case CPU_SUBTYPE_PENT:
	    case CPU_SUBTYPE_486SX:
		/*
		 * Since an exact match as not found look for the i486 else
		 * break into the loop to look for the i386_ALL.
		 */
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_486)
			return(i);
		}
		break;
	    case CPU_SUBTYPE_I386_ALL:
	    /* case CPU_SUBTYPE_I386: same as above */
	    case CPU_SUBTYPE_486:
		break;
	    }
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		   CPU_SUBTYPE_I386_ALL)
		    return(i);
	    }

	    /*
	     * A match failed, promote as little as possible.
	     */
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		   CPU_SUBTYPE_486)
		    return(i);
	    }
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		   CPU_SUBTYPE_486SX)
		    return(i);
	    }
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		   CPU_SUBTYPE_586)
		    return(i);
	    }
	    /*
	     * Now look for the lowest family and in that the lowest model.
	     */
	    lowest_family = CPU_SUBTYPE_INTEL_FAMILY_MAX + 1;
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if(CPU_SUBTYPE_INTEL_FAMILY(fat_cpusubtype &
					    ~CPU_SUBTYPE_MASK) <
		   lowest_family)
		    lowest_family = CPU_SUBTYPE_INTEL_FAMILY(
				fat_cpusubtype & ~CPU_SUBTYPE_MASK);
	    }
	    /* if no intel cputypes found return NULL */
	    if(lowest_family == CPU_SUBTYPE_INTEL_FAMILY_MAX + 1)
		return(-1);
	    lowest_model = LONG_MAX;
	    lowest_index = -1;
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if(CPU_SUBTYPE_INTEL_FAMILY(fat_cpusubtype &
					    ~CPU_SUBTYPE_MASK) ==
		   lowest_family){
		    if(CPU_SUBTYPE_INTEL_MODEL(fat_cpusubtype &
					       ~CPU_SUBTYPE_MASK) <
		       lowest_model){
		        lowest_model = CPU_SUBTYPE_INTEL_MODEL(
				fat_cpusubtype & ~CPU_SUBTYPE_MASK);
			lowest_index = i;
		    }
		}
	    }
	    return(lowest_index);
	case CPU_TYPE_MC680x0:
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		   CPU_SUBTYPE_MC680x0_ALL)
		    return(i);
	    }
	    /*
	     * Try to promote if starting from CPU_SUBTYPE_MC680x0_ALL and
	     * favor the CPU_SUBTYPE_MC68040 over the CPU_SUBTYPE_MC68030_ONLY.
	     */
	    if((cpusubtype & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC680x0_ALL){
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_MC68040)
			return(i);
		}
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_MC68030_ONLY)
			return(i);
		}
	    }
	    break;
	case CPU_TYPE_POWERPC:
	    /*
	     * An exact match as not found.  So for all the PowerPC subtypes
	     * pick the subtype from the following order starting from a subtype
	     * that will work (contains 64-bit instructions or altivec if
	     * needed):
	     *	970, 7450, 7400, 750, 604e, 604, 603ev, 603e, 603, ALL
	     * Note the 601 is NOT in the list above.  It is only picked via
	     * an exact match.  For an unknown subtype pick only the ALL type if
	     * it exists.
	     */
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_POWERPC_ALL:
		/*
		 * The CPU_SUBTYPE_POWERPC_ALL is only used by the development
		 * environment tools when building a generic ALL type binary.
		 * In the case of a non-exact match we pick the most current
		 * processor.
		 */
	    case CPU_SUBTYPE_POWERPC_970:
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_POWERPC_970)
			return(i);
		}
	    case CPU_SUBTYPE_POWERPC_7450:
	    case CPU_SUBTYPE_POWERPC_7400:
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_POWERPC_7450)
			return(i);
		}
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_POWERPC_7400)
			return(i);
		}
	    case CPU_SUBTYPE_POWERPC_750:
	    case CPU_SUBTYPE_POWERPC_604e:
	    case CPU_SUBTYPE_POWERPC_604:
	    case CPU_SUBTYPE_POWERPC_603ev:
	    case CPU_SUBTYPE_POWERPC_603e:
	    case CPU_SUBTYPE_POWERPC_603:
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_POWERPC_750)
			return(i);
		}
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_POWERPC_604e)
			return(i);
		}
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK)
		        == CPU_SUBTYPE_POWERPC_604)
			return(i);
		}
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_POWERPC_603ev)
			return(i);
		}
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_POWERPC_603e)
			return(i);
		}
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_POWERPC_603)
			return(i);
		}
	    default:
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_POWERPC_ALL)
			return(i);
		}
	    }
	    break;
	case CPU_TYPE_VEO:
	    /*
	     * An exact match was not found.  So for the VEO subtypes if VEO1
	     * or VEO3 is wanted then VEO2 can be used.  If VEO4 is wanted then
	     * either VEO2 or (preferably) VEO3 can be used.  But if VEO2 is
	     * wanted only VEO2 can be used.  Any unknown values don't match.
	     */
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_VEO_1:
	    case CPU_SUBTYPE_VEO_3:
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_VEO_2)
			return(i);
		}
	    case CPU_SUBTYPE_VEO_4:
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_VEO_3)
			return(i);
		}
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       CPU_SUBTYPE_VEO_2)
			return(i);
		}
	    }
	    break;
	case CPU_TYPE_MC88000:
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		   CPU_SUBTYPE_MC88000_ALL)
		    return(i);
	    }
	    break;
	case CPU_TYPE_I860:
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		   CPU_SUBTYPE_I860_ALL)
		    return(i);
	    }
	    break;
	case CPU_TYPE_HPPA:
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		   CPU_SUBTYPE_HPPA_ALL)
		    return(i);
	    }
	    break;
	case CPU_TYPE_SPARC:
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		   CPU_SUBTYPE_SPARC_ALL)
		    return(i);
	    }
	    break;
	case CPU_TYPE_ARM:
	    /*
	     * If it weren't for xscale, we could have a simple
	     * hierarchy like ppc.  However, xscale has instructions
	     * which aren't present on v5 or v6.  Here's the acceptable
	     * fat slices for each ARM subtype, for most to least
	     * preferred:
	     *   v4t: v4t, ALL
	     *   v5: v5, v4t, ALL
	     *   xscale: xscale, v4t, ALL
	     *   v6: v7, v6, v5, v4t, ALL
	     *   ALL: v6, v5, xscale, v4t, ALL
	     */
	    if(cpusubtype == CPU_SUBTYPE_ARM_ALL ||
	       cpusubtype == CPU_SUBTYPE_ARM_V7K){
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype == cputype &&
		       fat_cpusubtype == CPU_SUBTYPE_ARM_V7S)
			return(i);
		}
	    }
	    if(cpusubtype == CPU_SUBTYPE_ARM_ALL ||
	       cpusubtype == CPU_SUBTYPE_ARM_V7S){
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype == cputype &&
		       fat_cpusubtype == CPU_SUBTYPE_ARM_V7F)
			return(i);
		}
	    }
	    if(cpusubtype == CPU_SUBTYPE_ARM_ALL ||
	       cpusubtype == CPU_SUBTYPE_ARM_V7F){
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype == cputype &&
		       fat_cpusubtype == CPU_SUBTYPE_ARM_V7)
			return(i);
		}
	    }
	    if(cpusubtype == CPU_SUBTYPE_ARM_ALL ||
	       cpusubtype == CPU_SUBTYPE_ARM_V7 ||
	       cpusubtype == CPU_SUBTYPE_ARM_V6){
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype == cputype &&
		       fat_cpusubtype == CPU_SUBTYPE_ARM_V6)
			return(i);
		}
	    }
	    if(cpusubtype == CPU_SUBTYPE_ARM_ALL ||
	       cpusubtype == CPU_SUBTYPE_ARM_V6 ||
	       cpusubtype == CPU_SUBTYPE_ARM_V5TEJ){
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype == cputype &&
		       fat_cpusubtype == CPU_SUBTYPE_ARM_V5TEJ)
			return(i);
		}
	    }
	    if(cpusubtype == CPU_SUBTYPE_ARM_ALL ||
	       cpusubtype == CPU_SUBTYPE_ARM_XSCALE){
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype == cputype &&
		       fat_cpusubtype == CPU_SUBTYPE_ARM_XSCALE)
			return(i);
		}
	    }
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype == cputype &&
		   fat_cpusubtype == CPU_SUBTYPE_ARM_V4T)
		    return(i);
	    }
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype == cputype &&
		   fat_cpusubtype == CPU_SUBTYPE_ARM_V6M)
		    return(i);
	    }
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype == cputype &&
		   fat_cpusubtype == CPU_SUBTYPE_ARM_V7M)
		    return(i);
	    }
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype == cputype &&
		   fat_cpusubtype == CPU_SUBTYPE_ARM_V7EM)
		    return(i);
	    }
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype == cputype &&
		   fat_cpusubtype == CPU_SUBTYPE_ARM_ALL)
		    return(i);
	    }

	case CPU_TYPE_ARM64:
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		   CPU_SUBTYPE_ARM64_ALL)
		    return(i);
	    }
	    break;

	/* For CPU_TYPE_ARM64_32 only an exact match is allowed. */

	default:
	    return(-1);
	}
	return(-1);
}

/*
 * cpusubtype_findbestarch_64() is passed a cputype and cpusubtype and a set of
 * fat_arch_64 structs and selects the best one that matches (if any) and
 * returns a pointer to that fat_arch_64 struct (or NULL).  The fat_arch_64
 * structs must be in the host byte sex and correct such that the fat_archs64
 * really points to enough memory for nfat_arch_64 structs.  It is possible
 * that this routine could fail if new cputypes or cpusubtypes are added and an
 * old version of this routine is used.  But if there is an exact match between
 * the cputype and cpusubtype and one of the fat_arch structs this routine will
 * always succeed.
 */
__private_extern__
struct fat_arch_64 *
cpusubtype_findbestarch_64(
cpu_type_t cputype,
cpu_subtype_t cpusubtype,
struct fat_arch_64 *fat_archs64,
uint32_t nfat_archs)
{
    int32_t i;

	i = internal_cpusubtype_findbestarch(cputype, cpusubtype, NULL,
					     fat_archs64, nfat_archs);
	if(i == -1)
	    return(NULL);
	return(fat_archs64 + i);
}

/*
 * cpusubtype_findbestarch() is passed a cputype and cpusubtype and a set of
 * fat_arch structs and selects the best one that matches (if any) and returns
 * a pointer to that fat_arch struct (or NULL).  The fat_arch structs must be
 * in the host byte sex and correct such that the fat_archs really points to
 * enough memory for nfat_arch structs.  It is possible that this routine could
 * fail if new cputypes or cpusubtypes are added and an old version of this
 * routine is used.  But if there is an exact match between the cputype and
 * cpusubtype and one of the fat_arch structs this routine will always succeed.
 */
__private_extern__
struct fat_arch *
cpusubtype_findbestarch(
cpu_type_t cputype,
cpu_subtype_t cpusubtype,
struct fat_arch *fat_archs,
uint32_t nfat_archs)
{
    int32_t i;

	i = internal_cpusubtype_findbestarch(cputype, cpusubtype, fat_archs,
					     NULL, nfat_archs);
	if(i == -1)
	    return(NULL);
	return(fat_archs + i);
}
#endif /* RLD */

/*
 * cpusubtype_combine() returns the resulting cpusubtype when combining two
 * differnet cpusubtypes for the specified cputype.  If the two cpusubtypes
 * can't be combined (the specific subtypes are mutually exclusive) -1 is
 * returned indicating it is an error to combine them.  This can also fail and
 * return -1 if new cputypes or cpusubtypes are added and an old version of
 * this routine is used.  But if the cpusubtypes are the same they can always
 * be combined and this routine will return the cpusubtype pass in.
 */
__private_extern__
cpu_subtype_t
cpusubtype_combine(
cpu_type_t cputype,
cpu_subtype_t cpusubtype1,
cpu_subtype_t cpusubtype2)
{
	/*
	 * If this is an x86_64 cputype and either subtype is the
	 * "Haswell and compatible" it does not combine with anything else.
	 */
	if(cputype == CPU_TYPE_X86_64 &&
	   (cpusubtype1 == CPU_SUBTYPE_X86_64_H ||
	    cpusubtype2 == CPU_SUBTYPE_X86_64_H))
	    return((cpu_subtype_t)-1);

	/*
	 * We now combine any i386 or x86_64 subtype to the ALL subtype.
	 */
	if(cputype == CPU_TYPE_I386 || cputype == CPU_TYPE_X86_64)
	    return(CPU_SUBTYPE_I386_ALL);

	/*
         * The same cpusubtypes for any cputype returns that cpusubtype. For
	 * some cputypes like CPU_TYPE_ARM64_32 there is no combining of
	 * cpusubtypes so there is no code for those cputypes below.
	 */
	if((cpusubtype1 & ~CPU_SUBTYPE_MASK) ==
	   (cpusubtype2 & ~CPU_SUBTYPE_MASK))
	    return(cpusubtype1);

	switch(cputype){
	case CPU_TYPE_MC680x0:
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC680x0_ALL &&
	       (cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC68030_ONLY &&
	       (cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC68040)
		return((cpu_subtype_t)-1);
	    if((cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC680x0_ALL &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC68030_ONLY &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC68040)
		return((cpu_subtype_t)-1);

	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC68030_ONLY &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC68040)
		return((cpu_subtype_t)-1);
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK)  == CPU_SUBTYPE_MC68040 &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK)  == CPU_SUBTYPE_MC68030_ONLY)
		return((cpu_subtype_t)-1);

	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC68030_ONLY ||
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC68030_ONLY)
		return(CPU_SUBTYPE_MC68030_ONLY);

	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC68040 ||
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC68040)
		return(CPU_SUBTYPE_MC68040);
	    break; /* logically can't get here */

	case CPU_TYPE_POWERPC:
	    /*
	     * Combining with the ALL type becomes the other type. Combining
	     * anything with the 601 becomes 601.  All other non exact matches
	     * combine to the higher value subtype.
	     */
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_POWERPC_ALL)
		return(cpusubtype2);
	    if((cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_POWERPC_ALL)
		return(cpusubtype1);

	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_POWERPC_601 ||
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_POWERPC_601)
		return(CPU_SUBTYPE_POWERPC_601);

	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) >
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK))
		return(cpusubtype1);
	    else
		return(cpusubtype2);
	    break; /* logically can't get here */

	case CPU_TYPE_POWERPC64:
	    /*
	     * Combining with the ALL type becomes the other type.  All other
	     * non exact matches combine to the higher value subtype.
	     */
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_POWERPC_ALL)
		return(cpusubtype2);
	    if((cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_POWERPC_ALL)
		return(cpusubtype1);

	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) >
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK))
		return(cpusubtype1);
	    else
		return(cpusubtype2);
	    break; /* logically can't get here */

	case CPU_TYPE_VEO:
	    /*
	     * Combining VEO1 with VEO2 returns VEO1.  Any unknown values don't
	     * combine.
	     */
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_VEO_1 &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_VEO_2)
		return(CPU_SUBTYPE_VEO_1);
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_VEO_2 &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_VEO_1)
		return(CPU_SUBTYPE_VEO_1);
	    /*
	     * Combining VEO3 with VEO2 returns VEO3.  Any unknown values don't
	     * combine.
	     */
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_VEO_3 &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_VEO_2)
		return(CPU_SUBTYPE_VEO_3);
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_VEO_2 &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_VEO_3)
		return(CPU_SUBTYPE_VEO_3);
	    /*
	     * Combining VEO4 with VEO2 or VEO3 returns VEO4.  Any unknown
	     * values don't combine.
	     */
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_VEO_4 &&
	       ((cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_VEO_2
	        || (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_VEO_3))
		return(CPU_SUBTYPE_VEO_4);
	    if(((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_VEO_2
		|| (cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_VEO_3) &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_VEO_4)
		return(CPU_SUBTYPE_VEO_4);
	    return((cpu_subtype_t)-1);
	    break; /* logically can't get here */

	case CPU_TYPE_MC88000:
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC88000_ALL &&
	       (cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC88110)
		return((cpu_subtype_t)-1);
	    if((cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC88000_ALL &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC88110)
		return((cpu_subtype_t)-1);

	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC88110 ||
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC88110)
		return(CPU_SUBTYPE_MC88110);

	    break; /* logically can't get here */

	case CPU_TYPE_I860:
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_I860_ALL &&
	       (cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_I860_860)
		return((cpu_subtype_t)-1);
	    if((cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_I860_ALL &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_I860_860)
		return((cpu_subtype_t)-1);

	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_I860_860 ||
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_I860_860)
		return(CPU_SUBTYPE_I860_860);
	    break; /* logically can't get here */

	case CPU_TYPE_HPPA:
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_HPPA_ALL &&
	       (cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_HPPA_7100LC)
		return((cpu_subtype_t)-1);
	    if((cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_HPPA_ALL &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_HPPA_7100LC)
		return((cpu_subtype_t)-1);

	    return(CPU_SUBTYPE_HPPA_7100LC);
	    break; /* logically can't get here */

	case CPU_TYPE_SPARC:
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_SPARC_ALL)
			return((cpu_subtype_t)-1);
	    if((cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_SPARC_ALL)
			return((cpu_subtype_t)-1);
	    break; /* logically can't get here */

	case CPU_TYPE_ARM:
	    /*
	     * Combinability matrix for ARM:
	     *            V4T      V5  XSCALE      V6     V7   ALL
	     *            ~~~      ~~  ~~~~~~      ~~     ~~   ~~~
	     * V4T        V4T      V5  XSCALE      V6     V7   ALL
	     * V5          V5      V5      --      V6     V7   ALL
	     * XSCALE  XSCALE      --  XSCALE      --     --   ALL
	     * V6          V6      V6      --      V6     V7   ALL
	     * V7          V7      V7      --      V7     V7   ALL
	     * ALL        ALL     ALL     ALL     ALL     ALL  ALL
	     */
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_ARM_ALL)
		return(cpusubtype2);
	    if((cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_ARM_ALL)
		return(cpusubtype1);
	    switch((cpusubtype1 & ~CPU_SUBTYPE_MASK)){
		case CPU_SUBTYPE_ARM_V7:
		    switch((cpusubtype2 & ~CPU_SUBTYPE_MASK)){
			case CPU_SUBTYPE_ARM_XSCALE:
			    return((cpu_subtype_t)-1);
			default:
			    return(CPU_SUBTYPE_ARM_V7);
		    }
		case CPU_SUBTYPE_ARM_V6:
		    switch((cpusubtype2 & ~CPU_SUBTYPE_MASK)){
			case CPU_SUBTYPE_ARM_XSCALE:
			    return((cpu_subtype_t)-1);
			default:
			    return(CPU_SUBTYPE_ARM_V6);
		    }
		case CPU_SUBTYPE_ARM_XSCALE:
		    switch((cpusubtype2 & ~CPU_SUBTYPE_MASK)){
			case CPU_SUBTYPE_ARM_V7:
			case CPU_SUBTYPE_ARM_V6:
			case CPU_SUBTYPE_ARM_V5TEJ:
			    return((cpu_subtype_t)-1);
			default:
			    return(CPU_SUBTYPE_ARM_XSCALE);
		    }
		case CPU_SUBTYPE_ARM_V5TEJ:
		    switch((cpusubtype2 & ~CPU_SUBTYPE_MASK)){
			case CPU_SUBTYPE_ARM_XSCALE:
			    return((cpu_subtype_t)-1);
			case CPU_SUBTYPE_ARM_V7:
			    return(CPU_SUBTYPE_ARM_V7);
			case CPU_SUBTYPE_ARM_V6:
			    return(CPU_SUBTYPE_ARM_V6);
			default:
			    return(CPU_SUBTYPE_ARM_V5TEJ);
		    }
		case CPU_SUBTYPE_ARM_V4T:
		    return((cpusubtype2 & ~CPU_SUBTYPE_MASK));
		default:
		    return((cpu_subtype_t)-1);
	    }

	case CPU_TYPE_ARM64:
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_ARM64_ALL)
			return((cpu_subtype_t)-1);
	    if((cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_ARM64_ALL)
			return((cpu_subtype_t)-1);
	    break; /* logically can't get here */

	default:
	    return((cpu_subtype_t)-1);
	}
	return((cpu_subtype_t)-1); /* logically can't get here */
}

#ifndef RLD
/*
 * cpusubtype_execute() returns TRUE if the exec_cpusubtype can be used for
 * execution on the host_cpusubtype for the specified cputype (this routine is
 * used by the dynamic linker and should match the kernel's exec(2) code).  If
 * the exec_cpusubtype can't be run on the host_cpusubtype FALSE is returned
 * indicating it can't be run on that cpu.  This can also return FALSE and
 * if new cputypes or cpusubtypes are added and an old version of this routine
 * is used.  But if the cpusubtypes are the same they can always be executed
 * and this routine will return TRUE.  And ALL subtypes are always allowed to be
 * executed on unknown host_cpusubtype's.
 */
__private_extern__
enum bool
cpusubtype_execute(
cpu_type_t host_cputype,
cpu_subtype_t host_cpusubtype, /* can NOT be the ALL type */
cpu_subtype_t exec_cpusubtype) /* can be the ALL type */
{
	if((host_cpusubtype & ~CPU_SUBTYPE_MASK) ==
	   (exec_cpusubtype & ~CPU_SUBTYPE_MASK))
	    return(TRUE);

	switch(host_cputype){
	case CPU_TYPE_POWERPC:
	    /*
	     * The 970 has 64-bit and altivec instructions
	     * The 7450 and 7400 have altivec instructions
	     * The 601 has Power instructions (can only execute on a 601)
	     * other known subtypes can execute anywhere
	     * unknown hosts will only be allowed to execute the ALL subtype
	     */
	    switch(host_cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_POWERPC_970:
		switch(exec_cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_POWERPC_970:
		case CPU_SUBTYPE_POWERPC_7450:
		case CPU_SUBTYPE_POWERPC_7400:
		case CPU_SUBTYPE_POWERPC_750:
		case CPU_SUBTYPE_POWERPC_620:
		case CPU_SUBTYPE_POWERPC_604e:
		case CPU_SUBTYPE_POWERPC_604:
		case CPU_SUBTYPE_POWERPC_603ev:
		case CPU_SUBTYPE_POWERPC_603e:
		case CPU_SUBTYPE_POWERPC_603:
		case CPU_SUBTYPE_POWERPC_602:
		case CPU_SUBTYPE_POWERPC_ALL:
		    return(TRUE);
		case CPU_SUBTYPE_POWERPC_601:
		default:
		    return(FALSE);
		}
		break; /* logically can't get here */

	    case CPU_SUBTYPE_POWERPC_7450:
	    case CPU_SUBTYPE_POWERPC_7400:
		switch(exec_cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_POWERPC_7450:
		case CPU_SUBTYPE_POWERPC_7400:
		case CPU_SUBTYPE_POWERPC_750:
		case CPU_SUBTYPE_POWERPC_620:
		case CPU_SUBTYPE_POWERPC_604e:
		case CPU_SUBTYPE_POWERPC_604:
		case CPU_SUBTYPE_POWERPC_603ev:
		case CPU_SUBTYPE_POWERPC_603e:
		case CPU_SUBTYPE_POWERPC_603:
		case CPU_SUBTYPE_POWERPC_602:
		case CPU_SUBTYPE_POWERPC_ALL:
		    return(TRUE);
		case CPU_SUBTYPE_POWERPC_970:
		case CPU_SUBTYPE_POWERPC_601:
		default:
		    return(FALSE);
		}
		break; /* logically can't get here */

	    case CPU_SUBTYPE_POWERPC_750:
	    case CPU_SUBTYPE_POWERPC_620:
	    case CPU_SUBTYPE_POWERPC_604e:
	    case CPU_SUBTYPE_POWERPC_604:
	    case CPU_SUBTYPE_POWERPC_603ev:
	    case CPU_SUBTYPE_POWERPC_603e:
	    case CPU_SUBTYPE_POWERPC_603:
	    case CPU_SUBTYPE_POWERPC_602:
		switch(exec_cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_POWERPC_750:
		case CPU_SUBTYPE_POWERPC_620:
		case CPU_SUBTYPE_POWERPC_604e:
		case CPU_SUBTYPE_POWERPC_604:
		case CPU_SUBTYPE_POWERPC_603ev:
		case CPU_SUBTYPE_POWERPC_603e:
		case CPU_SUBTYPE_POWERPC_603:
		case CPU_SUBTYPE_POWERPC_602:
		case CPU_SUBTYPE_POWERPC_ALL:
		    return(TRUE);
		case CPU_SUBTYPE_POWERPC_970:
		case CPU_SUBTYPE_POWERPC_7450:
		case CPU_SUBTYPE_POWERPC_7400:
		case CPU_SUBTYPE_POWERPC_601:
		default:
		    return(FALSE);
		}
		break; /* logically can't get here */

	    case CPU_SUBTYPE_POWERPC_601:
		switch(exec_cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_POWERPC_750:
		case CPU_SUBTYPE_POWERPC_620:
		case CPU_SUBTYPE_POWERPC_604e:
		case CPU_SUBTYPE_POWERPC_604:
		case CPU_SUBTYPE_POWERPC_603ev:
		case CPU_SUBTYPE_POWERPC_603e:
		case CPU_SUBTYPE_POWERPC_603:
		case CPU_SUBTYPE_POWERPC_602:
		case CPU_SUBTYPE_POWERPC_601:
		case CPU_SUBTYPE_POWERPC_ALL:
		    return(TRUE);
		case CPU_SUBTYPE_POWERPC_970:
		case CPU_SUBTYPE_POWERPC_7450:
		case CPU_SUBTYPE_POWERPC_7400:
		default:
		    return(FALSE);
		}
		break; /* logically can't get here */

	    default: /* unknown host */
		switch(exec_cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_POWERPC_ALL:
		    return(TRUE);
		default:
		    return(FALSE);
		}
		break; /* logically can't get here */
	    }
	    break; /* logically can't get here */

	case CPU_TYPE_I386:
	    /*
	     * On i386 if it is any known subtype it is allowed to execute on
	     * any host (even unknown hosts).  And the binary is expected to
	     * have code to avoid instuctions that will not execute on the
	     * host cpu.
	     */
	    switch(exec_cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_I386_ALL: /* same as CPU_SUBTYPE_386 */
	    case CPU_SUBTYPE_486:
	    case CPU_SUBTYPE_486SX:
	    case CPU_SUBTYPE_586: /* same as CPU_SUBTYPE_PENT */
	    case CPU_SUBTYPE_PENTPRO:
	    case CPU_SUBTYPE_PENTII_M3:
	    case CPU_SUBTYPE_PENTII_M5:
	    case CPU_SUBTYPE_PENTIUM_4:
		return(TRUE);
	    default:
		return(FALSE);
	    }
	    break; /* logically can't get here */

	case CPU_TYPE_MC680x0:
	    switch(host_cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_MC68040:
		switch(exec_cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_MC68040:
		case CPU_SUBTYPE_MC680x0_ALL: /* same as CPU_SUBTYPE_MC68030 */
		    return(TRUE);
		case CPU_SUBTYPE_MC68030_ONLY:
		default:
		    return(FALSE);
		}
		break; /* logically can't get here */

	    case CPU_SUBTYPE_MC68030:
		switch(exec_cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_MC680x0_ALL: /* same as CPU_SUBTYPE_MC68030 */
		case CPU_SUBTYPE_MC68030_ONLY:
		    return(TRUE);
		case CPU_SUBTYPE_MC68040:
		default:
		    return(FALSE);
		}
		break; /* logically can't get here */
		
	    default: /* unknown host */
		switch(exec_cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_MC680x0_ALL: /* same as CPU_SUBTYPE_MC68030 */
		    return(TRUE);
		default:
		    return(FALSE);
		}
		break; /* logically can't get here */
	    }
	    break; /* logically can't get here */

	case CPU_TYPE_MC88000:
	    switch(host_cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_MC88110:
		switch(exec_cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_MC88110:
		case CPU_SUBTYPE_MC88000_ALL:
		    return(TRUE);
		default:
		    return(FALSE);
		}
		break; /* logically can't get here */

	    default: /* unknown host */
		switch(exec_cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_MC88000_ALL:
		    return(TRUE);
		default:
		    return(FALSE);
		}
		break; /* logically can't get here */
	    }
	    break; /* logically can't get here */
	    
	case CPU_TYPE_HPPA:
	    switch(host_cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_HPPA_7100LC:
		switch(exec_cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_HPPA_ALL: /* same as CPU_SUBTYPE_HPPA_7100 */
		case CPU_SUBTYPE_HPPA_7100LC:
		    return(TRUE);
		default:
		    return(FALSE);
		}
		break; /* logically can't get here */

	    case CPU_SUBTYPE_HPPA_7100:
		switch(exec_cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_HPPA_ALL: /* same as CPU_SUBTYPE_HPPA_7100 */
		    return(TRUE);
		case CPU_SUBTYPE_HPPA_7100LC:
		default:
		    return(FALSE);
		}
		break; /* logically can't get here */

	    default: /* unknown host */
		switch(exec_cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_HPPA_ALL: /* same as CPU_SUBTYPE_HPPA_7100 */
		    return(TRUE);
		default:
		    return(FALSE);
		}
		break; /* logically can't get here */
	    }
	    break; /* logically can't get here */

	case CPU_TYPE_SPARC:
	    /*
	     * For Sparc we only have the ALL subtype defined.
	     */
	    switch(exec_cpusubtype & ~CPU_SUBTYPE_MASK){
	    case CPU_SUBTYPE_SPARC_ALL:
		return(TRUE);
	    default:
		return(FALSE);
	    }
	    break; /* logically can't get here */

	case CPU_TYPE_ARM64_32:
	    /*
	     * For CPU_TYPE_ARM64_32 we only have CPU_SUBTYPE_ARM64_32_V8
	     * defined.
	     */
	    switch (host_cpusubtype){
	    case CPU_SUBTYPE_ARM64_32_V8:
		switch(exec_cpusubtype){
		case CPU_SUBTYPE_ARM64_32_V8:
		    return(TRUE);
		default:
		    return(FALSE);
		}
		break;
	    }
	    break;

	case CPU_TYPE_ARM64:
	    switch (host_cpusubtype){
	    case CPU_SUBTYPE_ARM64_V8:
		switch(exec_cpusubtype){
		case CPU_SUBTYPE_ARM64_ALL:
		case CPU_SUBTYPE_ARM64_V8:
		case CPU_SUBTYPE_ARM64E:
		    return(TRUE);
		default:
		    break; /* fall through to arm 32-bit types below */
		}
		break;

	    default:
	        switch (exec_cpusubtype){
	        case CPU_SUBTYPE_ARM64_ALL:
		    return(TRUE);
		default:
		    break; /* fall through to arm 32-bit types below */
		}
		break;
	    }
	    /* fall through to arm 32-bit types below */

	case CPU_TYPE_ARM:
	    switch (host_cpusubtype){
	    case CPU_SUBTYPE_ARM_V6:
		switch(exec_cpusubtype){
		case CPU_SUBTYPE_ARM_ALL:
		case CPU_SUBTYPE_ARM_V4T:
		case CPU_SUBTYPE_ARM_V5TEJ:
		case CPU_SUBTYPE_ARM_V6:
		    return(TRUE);
		default:
		    return(FALSE);
		}
		break; /* logically can't get here */

	    case CPU_SUBTYPE_ARM_V5TEJ:
		switch(exec_cpusubtype){
		case CPU_SUBTYPE_ARM_ALL:
		case CPU_SUBTYPE_ARM_V5TEJ:
		case CPU_SUBTYPE_ARM_V4T:
		    return(TRUE);
		default:
		    return(FALSE);
		}
		break; /* logically can't get here */

	    case CPU_SUBTYPE_ARM_XSCALE:
		switch(exec_cpusubtype){
		case CPU_SUBTYPE_ARM_ALL:
		case CPU_SUBTYPE_ARM_XSCALE:
		case CPU_SUBTYPE_ARM_V4T:
		    return(TRUE);
		default:
		    return(FALSE);
		}
		break; /* logically can't get here */

	    case CPU_SUBTYPE_ARM_V4T:
		switch(exec_cpusubtype){
		case CPU_SUBTYPE_ARM_ALL:
		case CPU_SUBTYPE_ARM_V4T:
		    return(TRUE);
		default:
		    return(FALSE);
		}
		break; /* logically can't get here */

	    default:
	      switch (exec_cpusubtype){
	      case CPU_SUBTYPE_ARM_ALL:
		return(TRUE);
	      default:
		return(FALSE);
	      }
	      break; /* logically can't get here */
	    }
	    break; /* logically can't get here */

	case CPU_TYPE_VEO:  /* not used with the dynamic linker */
	case CPU_TYPE_I860: /* not used with the dynamic linker */
	default:
	    return(FALSE);
	}
	return(FALSE); /* logically can't get here */
}
#endif /* RLD */
