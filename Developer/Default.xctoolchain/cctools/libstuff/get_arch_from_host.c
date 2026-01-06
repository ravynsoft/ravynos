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
#include "stdlib.h"
#include "string.h"
#include <mach/mach.h>
#include "stuff/openstep_mach.h"
#include "stuff/arch.h"
#include "stuff/allocate.h"

/*
 * get_arch_from_host() gets the architecture from the host this is running on
 * and returns zero if the architecture is not known and zero if the
 * architecture is known.  If the parameters family_arch_flag and
 * specific_arch_flag are not NULL they get fill in with the family
 * architecture and specific architecure for the host.  If the architecture
 * is unknown and the parameters are not NULL then all fields are set to zero.
 */
__private_extern__
int
get_arch_from_host(
struct arch_flag *family_arch_flag,
struct arch_flag *specific_arch_flag)
{
    struct host_basic_info host_basic_info;
    natural_t count; /* cctools-port: unsigned int -> natural_t */
    kern_return_t r;
    mach_port_t my_mach_host_self;

	if(family_arch_flag != NULL)
	    memset(family_arch_flag, '\0', sizeof(struct arch_flag));
	if(specific_arch_flag != NULL)
	    memset(specific_arch_flag, '\0', sizeof(struct arch_flag));

	count = HOST_BASIC_INFO_COUNT;
	my_mach_host_self = mach_host_self();
	if((r = host_info(my_mach_host_self, HOST_BASIC_INFO,
			  (host_info_t)(&host_basic_info),
			  &count)) != KERN_SUCCESS){
	    mach_port_deallocate(mach_task_self(), my_mach_host_self);
	    return(0);
	}
	mach_port_deallocate(mach_task_self(), my_mach_host_self);

	if(family_arch_flag != NULL){
	    family_arch_flag->cputype = host_basic_info.cpu_type;
	}
	if(specific_arch_flag != NULL){
	    specific_arch_flag->cputype = host_basic_info.cpu_type;
	    specific_arch_flag->cpusubtype = host_basic_info.cpu_subtype;
	}
	switch(host_basic_info.cpu_type){
	case CPU_TYPE_MC680x0:
	    switch(host_basic_info.cpu_subtype){
	    case CPU_SUBTYPE_MC680x0_ALL:
	    case CPU_SUBTYPE_MC68030_ONLY:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "m68k";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_MC680x0_ALL;
		}
		if(specific_arch_flag != NULL){
		    specific_arch_flag->name = "m68030";
		    /* 
		     * There is a "bug" in the kernel for compatiblity that on
		     * an 030 machine host_info() returns cpusubtype
		     * CPU_SUBTYPE_MC680x0_ALL and not CPU_SUBTYPE_MC68030_ONLY.
		     */
		    specific_arch_flag->cpusubtype = CPU_SUBTYPE_MC68030_ONLY;
		}
		return(1);
	    case CPU_SUBTYPE_MC68040:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "m68k";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_MC680x0_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "m68040";
		return(1);
	    }
	    break;
	case CPU_TYPE_POWERPC:
	    switch(host_basic_info.cpu_subtype){
	    case CPU_SUBTYPE_POWERPC_ALL:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "ppc";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_POWERPC_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "ppc";
		return(1);
	    case CPU_SUBTYPE_POWERPC_601:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "ppc";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_POWERPC_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "ppc601";
		return(1);
	    case CPU_SUBTYPE_POWERPC_603:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "ppc";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_POWERPC_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "ppc603";
		return(1);
	    case CPU_SUBTYPE_POWERPC_603e:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "ppc";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_POWERPC_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "ppc603e";
		return(1);
	    case CPU_SUBTYPE_POWERPC_603ev:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "ppc";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_POWERPC_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "ppc603ev";
		return(1);
	    case CPU_SUBTYPE_POWERPC_604:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "ppc";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_POWERPC_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "ppc604";
		return(1);
	    case CPU_SUBTYPE_POWERPC_604e:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "ppc";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_POWERPC_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "ppc604e";
		return(1);
	    case CPU_SUBTYPE_POWERPC_750:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "ppc";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_POWERPC_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "ppc750";
		return(1);
	    case CPU_SUBTYPE_POWERPC_7400:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "ppc";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_POWERPC_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "ppc7400";
		return(1);
	    case CPU_SUBTYPE_POWERPC_7450:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "ppc";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_POWERPC_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "ppc7450";
		return(1);
	    case CPU_SUBTYPE_POWERPC_970:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "ppc";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_POWERPC_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "ppc970";
		return(1);
	    default:
		if(family_arch_flag != NULL){
                    family_arch_flag->name = "ppc";
                    family_arch_flag->cpusubtype = CPU_SUBTYPE_POWERPC_ALL;
                }
                if(specific_arch_flag != NULL){
                    specific_arch_flag->name = 
			savestr("PowerPC cpusubtype 1234567890");
                    if(specific_arch_flag->name != NULL)
			sprintf(specific_arch_flag->name,
				"PowerPC cpusubtype %u", 
				host_basic_info.cpu_subtype);
		}
                return(1);
	    }
	    break;
	case CPU_TYPE_VEO:
	    switch(host_basic_info.cpu_subtype){
	    case CPU_SUBTYPE_VEO_1:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "veo";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_VEO_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "veo1";
		return(1);
	    case CPU_SUBTYPE_VEO_2:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "veo";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_VEO_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "veo2";
		return(1);
	    case CPU_SUBTYPE_VEO_3:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "veo";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_VEO_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "veo3";
		return(1);
	    case CPU_SUBTYPE_VEO_4:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "veo";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_VEO_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "veo4";
		return(1);
	    default:
		if(family_arch_flag != NULL){
                    family_arch_flag->name = "veo";
                    family_arch_flag->cpusubtype = CPU_SUBTYPE_VEO_ALL;
                }
                if(specific_arch_flag != NULL){
                    specific_arch_flag->name = 
			savestr("VEO cpusubtype 1234567890");
		    sprintf(specific_arch_flag->name,
				"VEO cpusubtype %u", 
				host_basic_info.cpu_subtype);
		}
                return(1);
	    }
	    break;
	case CPU_TYPE_MC88000:
	    switch(host_basic_info.cpu_subtype){
	    case CPU_SUBTYPE_MC88000_ALL:
	    case CPU_SUBTYPE_MC88110:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "m88k";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_MC88000_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "m88k";
		return(1);
	    }
	    break;
	case CPU_TYPE_I386:
	    switch(host_basic_info.cpu_subtype){
	    case CPU_SUBTYPE_I386_ALL:
	    /* case CPU_SUBTYPE_386: same value as above */
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "i386";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_I386_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "i386";
		return(1);
	    case CPU_SUBTYPE_486:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "i386";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_I386_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "i486";
		return(1);
	    case CPU_SUBTYPE_486SX:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "i386";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_I386_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "i486SX";
		return(1);
	    case CPU_SUBTYPE_PENT: /* same as CPU_SUBTYPE_586 */
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "i386";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_I386_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "pentium";
		return(1);
	    case CPU_SUBTYPE_PENTPRO:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "i386";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_I386_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "pentpro";
		return(1);
	    case CPU_SUBTYPE_PENTII_M3:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "i386";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_I386_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "pentIIm3";
		return(1);
	    case CPU_SUBTYPE_PENTII_M5:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "i386";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_I386_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "pentIIm5";
		return(1);
	    case CPU_SUBTYPE_PENTIUM_4:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "i386";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_I386_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "pentium4";
		return(1);
	    default:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "i386";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_I386_ALL;
                }
                if(specific_arch_flag != NULL){
                    specific_arch_flag->name =
			savestr("Intel family 12 model 12345678");
		    if(specific_arch_flag->name != NULL)
			sprintf(specific_arch_flag->name,
			    "Intel family %u model %u", 
			CPU_SUBTYPE_INTEL_FAMILY(host_basic_info.cpu_subtype),
			CPU_SUBTYPE_INTEL_MODEL(host_basic_info.cpu_subtype));
		}
                return(1);
	    }
	    break;
	case CPU_TYPE_I860:
	    switch(host_basic_info.cpu_subtype){
	    case CPU_SUBTYPE_I860_ALL:
	    case CPU_SUBTYPE_I860_860:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "i860";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_I860_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "i860";
		return(1);
	    }
	    break;
	case CPU_TYPE_HPPA:
	    switch(host_basic_info.cpu_subtype){
	    case CPU_SUBTYPE_HPPA_ALL:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "hppa";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_HPPA_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "hppa";
		return(1);
	    case CPU_SUBTYPE_HPPA_7100LC:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "hppa";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_HPPA_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "hppa7100LC";
		return(1);
	      
	    }
	    break;
	case CPU_TYPE_SPARC:
	    switch(host_basic_info.cpu_subtype){
	    case /*CPU_SUBTYPE_SPARC_ALL*/0:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "sparc";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_SPARC_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "sparc";
		return(1);
	    }
	    break;
	case CPU_TYPE_ARM:
	    switch(host_basic_info.cpu_subtype){
	    case CPU_SUBTYPE_ARM_ALL:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "arm";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_ARM_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "arm";
		return(1);
	    case CPU_SUBTYPE_ARM_V4T:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "arm";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_ARM_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "armv4t";
		return(1);
	    case CPU_SUBTYPE_ARM_V5TEJ:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "arm";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_ARM_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "armv5";
		return(1);
	    case CPU_SUBTYPE_ARM_XSCALE:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "arm";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_ARM_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "xscale";
		return(1);
	    case CPU_SUBTYPE_ARM_V6:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "arm";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_ARM_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "armv6";
		return(1);
	    case CPU_SUBTYPE_ARM_V6M:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "arm";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_ARM_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "armv6m";
		return(1);
	    case CPU_SUBTYPE_ARM_V7:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "arm";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_ARM_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "armv7";
		return(1);
	    case CPU_SUBTYPE_ARM_V7F:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "arm";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_ARM_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "armv7f";
		return(1);
	    case CPU_SUBTYPE_ARM_V7S:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "arm";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_ARM_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "armv7s";
		return(1);
	    case CPU_SUBTYPE_ARM_V7K:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "arm";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_ARM_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "armv7k";
		return(1);
	    case CPU_SUBTYPE_ARM_V7M:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "arm";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_ARM_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "armv7m";
		return(1);
	    case CPU_SUBTYPE_ARM_V7EM:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "arm";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_ARM_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "armv7em";
		return(1);
	    }
	    break;
	case CPU_TYPE_ARM64:
	    switch(host_basic_info.cpu_subtype){
	    case CPU_SUBTYPE_ARM64_ALL:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "arm64";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_ARM64_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "arm64";
		return(1);
	    case CPU_SUBTYPE_ARM64_V8:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "arm64";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_ARM64_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "arm64v8";
		return(1);
	    case CPU_SUBTYPE_ARM64E:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "arm64";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_ARM64_ALL;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "arm64e";
		return(1);
	    }
	    break;
	case CPU_TYPE_ARM64_32:
	    switch(host_basic_info.cpu_subtype){
	    case CPU_SUBTYPE_ARM64_32_V8:
		if(family_arch_flag != NULL){
		    family_arch_flag->name = "arm64_32";
		    family_arch_flag->cpusubtype = CPU_SUBTYPE_ARM64_32_V8;
		}
		if(specific_arch_flag != NULL)
		    specific_arch_flag->name = "arm64_32";
		return(1);
	    }
	    break;
	}
	return(0);
}
#endif /* !defined(RLD) */
