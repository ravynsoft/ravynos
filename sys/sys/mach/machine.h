/*
 * Copyright 1991-1998 by Open Software Foundation, Inc. 
 * Copyright 2021 Zoe Knox <zoe@ravynsoft.com>
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/* 2021-12-16 Z. Knox  Updated CPU definitions to match LLVM */
/*
 * MkLinux
 */
/* CMU_HIST */
/*
 * Revision 2.13.5.4  92/09/15  17:23:42  jeffreyh
 * 	New 860 names from Intel
 * 	[92/09/10            jeffreyh]
 * 
 * Revision 2.13.5.3  92/05/27  00:46:46  jeffreyh
 * 	Initial Paragon support.
 * 	[andyp@ssd.intel.com]
 * 
 * Revision 2.13.5.2  92/03/28  10:11:11  jeffreyh
 * 	Pick up changes from MK71
 * 	[92/03/20  13:21:20  jeffreyh]
 * 
 * Revision 2.14  92/02/19  15:08:38  elf
 * 	Added more Sparc subtypes.
 * 	[92/02/19            rpd]
 * 
 * Revision 2.13.5.1  92/02/18  19:13:35  jeffreyh
 * 	Added sub cpu type for Corollary MP
 * 	[91/06/25            bernadat]
 *
 * Revision 2.13  91/12/10  16:32:50  jsb
 * 	Fixes from Intel
 * 	[91/12/10  15:51:57  jsb]
 * 
 * Revision 2.12  91/07/31  17:53:56  dbg
 * 	Remove declaration of interrupt_stack - it is machine-dependent.
 * 	Changed CPU_SUBTYPE_LUNA88K - there's already a (potential)
 * 	mc88000 port.
 * 	[91/07/26            dbg]
 * 
 * Revision 2.11  91/07/09  23:22:06  danner
 * 	   Added CPU_SUBTYPE_LUNA88K
 * 	[91/05/06            danner]
 * 
 * Revision 2.10  91/05/14  16:55:24  mrt
 * 	Correcting copyright
 * 
 * Revision 2.9  91/05/08  12:49:54  dbg
 * 	Add CPU_SUBTYPE_SYMMETRY.
 * 	[91/04/26  14:44:44  dbg]
 * 
 * Revision 2.8  91/02/05  17:33:48  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:18:51  mrt]
 * 
 * Revision 2.7  90/12/04  14:51:27  jsb
 * 	Renamed CPU_SUBTYPE_iPSC2 as CPU_SUBTYPE_iPSC386.
 * 	[90/12/03  22:22:25  jsb]
 * 
 * Revision 2.6  90/11/25  17:48:28  jsb
 * 	Added CPU_TYPE_I860 and CPU_SUBTYPE_iPSC860.
 * 	[90/11/25  16:51:44  jsb]
 * 
 * Revision 2.5  90/09/23  17:45:42  jsb
 * 	Added CPU_SUBTYPE_iPSC2.
 * 	[90/09/21  16:43:01  jsb]
 * 
 * Revision 2.4  90/08/07  22:23:05  rpd
 * 	Added new MIPS subtypes for R3000, fix old defs.
 * 	[90/08/07  15:54:40  af]
 * 
 * Revision 2.3  90/06/02  14:58:47  rpd
 * 	Picked up new cpu types.
 * 	[90/03/26  22:34:38  rpd]
 * 
 * Revision 2.2  89/11/29  14:09:32  af
 * 	Changes for MACH_KERNEL:
 * 	. Removed should_exit.
 * 
 * 	17-May-1989 Randall W. Dean (rwd)
 * 	changed machine/vm_types.h to mach/vm_param.h
 * 	[89/05/23            dbg]
 * 
 * Revision 2.1  89/08/03  16:02:33  rwd
 * Created.
 * 
 * Revision 2.11  89/04/18  16:43:32  mwyoung
 * 	Use <machine/vm_types.h> rather than <vm/vm_param.h> to get
 * 	VM types.  Remove old history... none of it was insightful.
 * 
 * 	The variable declarations should be moved elsewhere.
 * 	[89/01/24            mwyoung]
 * 
 * Revision 2.2.4.1  90/06/11  10:59:52  af
 * 	Added new MIPS subtypes for R3000, fix old defs.
 * 
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 */
/*	File:	machine.h
 *	Author:	Avadis Tevanian, Jr.
 *	Date:	1986
 *
 *	Machine independent machine abstraction.
 */

#ifndef	_MACH_MACHINE_H_
#define _MACH_MACHINE_H_

#ifdef	MACH_KERNEL
#include <cpus.h>
#endif	/* MACH_KERNEL */

#include <sys/types.h>
#ifndef _KERNEL
#include <vm/vm.h> // for boolean_t
#endif
#include <sys/mach/vm_types.h>

/*
 *	For each host, there is a maximum possible number of
 *	cpus that may be available in the system.  This is the
 *	compile-time constant NCPUS, which is defined in cpus.h.
 *
 *	In addition, there is a machine_slot specifier for each
 *	possible cpu in the system.
 */

struct machine_info {
	integer_t	major_version;	/* kernel major version id */
	integer_t	minor_version;	/* kernel minor version id */
	int		max_cpus;	/* max number of cpus compiled */
	int		avail_cpus;	/* number actually available */
	vm_size_t	memory_size;	/* size of memory in bytes */
};

typedef struct machine_info	*machine_info_t;
typedef struct machine_info	machine_info_data_t;	/* bogus */

typedef uint32_t cpu_type_t;
typedef uint32_t cpu_subtype_t;

#define CPU_STATE_MAX		4

#define CPU_STATE_USER		0
#define CPU_STATE_NICE		1
#define CPU_STATE_SYSTEM	2
#define CPU_STATE_IDLE		3

struct machine_slot {
	boolean_t	is_cpu;		/* is there a cpu in this slot? */
	cpu_type_t	cpu_type;	/* type of cpu */
	cpu_subtype_t	cpu_subtype;	/* subtype of cpu */
	integer_t	running;	/* is cpu running */
	integer_t	cpu_ticks[CPU_STATE_MAX];
	integer_t	clock_freq;	/* clock interrupt frequency */
};

typedef struct machine_slot	*machine_slot_t;
typedef struct machine_slot	machine_slot_data_t;	/* bogus */

#ifdef	MACH_KERNEL
extern struct machine_info	machine_info;
extern struct machine_slot	machine_slot[NCPUS];
#endif	/* MACH_KERNEL */

/*
 *	Machine types known by all.
 *
 *	When adding new types & subtypes, please also update slot_name.c
 *	in the libmach sources.
 */

enum : uint32_t {
    CPU_ARCH_MASK = 0xFF000000,
    CPU_ARCH_ABI64 = 0x01000000,
    CPU_ARCH_ABI64_32 = 0x02000000
};

enum : cpu_type_t {
    CPU_TYPE_ANY = 0xffffffff,  // -1 as uint32_t
    CPU_TYPE_X86 = 7,
    CPU_TYPE_I386 = CPU_TYPE_X86,
    CPU_TYPE_X86_64 = CPU_TYPE_X86|CPU_ARCH_ABI64,
    CPU_TYPE_MIPS = 8,
    CPU_TYPE_MC98000 = 10,
    CPU_TYPE_ARM = 12,
    CPU_TYPE_ARM64 = CPU_TYPE_ARM|CPU_ARCH_ABI64,
    CPU_TYPE_ARM64_32 = CPU_TYPE_ARM|CPU_ARCH_ABI64_32,
    CPU_TYPE_SPARC = 14,
    CPU_TYPE_POWERPC = 18,
    CPU_TYPE_PPC = CPU_TYPE_POWERPC,
    CPU_TYPE_POWERPC64 = CPU_TYPE_POWERPC|CPU_ARCH_ABI64,
    CPU_TYPE_PPC64 = CPU_TYPE_POWERPC64
};

enum : uint32_t {
    CPU_SUBTYPE_MASK = 0xFF000000,
    CPU_SUBTYPE_LIB64 = 0x80000000,
    CPU_SUBTYPE_MULTIPLE = ~0u
};

/*
 *	Machine subtypes (these are defined here, instead of in a machine
 *	dependent directory, so that any program can get all definitions
 *	regardless of where is it compiled).
 */

/*
 *	VAX subtypes (these do *not* necessarily conform to the actual cpu
 *	ID assigned by DEC available via the SID register).
 */

#define CPU_SUBTYPE_VAX780	((cpu_subtype_t) 1)
#define CPU_SUBTYPE_VAX785	((cpu_subtype_t) 2)
#define CPU_SUBTYPE_VAX750	((cpu_subtype_t) 3)
#define CPU_SUBTYPE_VAX730	((cpu_subtype_t) 4)
#define CPU_SUBTYPE_UVAXI	((cpu_subtype_t) 5)
#define CPU_SUBTYPE_UVAXII	((cpu_subtype_t) 6)
#define CPU_SUBTYPE_VAX8200	((cpu_subtype_t) 7)
#define CPU_SUBTYPE_VAX8500	((cpu_subtype_t) 8)
#define CPU_SUBTYPE_VAX8600	((cpu_subtype_t) 9)
#define CPU_SUBTYPE_VAX8650	((cpu_subtype_t) 10)
#define CPU_SUBTYPE_VAX8800	((cpu_subtype_t) 11)
#define CPU_SUBTYPE_UVAXIII	((cpu_subtype_t) 12)

/*
 *	Alpha subtypes (these do *not* necessary conform to the actual cpu
 *	ID assigned by DEC available via the SID register).
 */

#define CPU_SUBTYPE_ALPHA_ADU		((cpu_subtype_t) 1)
#define CPU_SUBTYPE_DEC_4000		((cpu_subtype_t) 2)
#define CPU_SUBTYPE_DEC_7000		((cpu_subtype_t) 3)
#define CPU_SUBTYPE_DEC_3000_500	((cpu_subtype_t) 4)
#define CPU_SUBTYPE_DEC_3000_400	((cpu_subtype_t) 5)
#define CPU_SUBTYPE_DEC_10000		((cpu_subtype_t) 6)
#define CPU_SUBTYPE_DEC_3000_300        ((cpu_subtype_t) 7)
#define CPU_SUBTYPE_DEC_2000_300        ((cpu_subtype_t) 8)
#define CPU_SUBTYPE_DEC_2100_A500	((cpu_subtype_t) 9)
#define CPU_SUBTYPE_DEC_MORGAN		((cpu_subtype_t) 11)
#define CPU_SUBTYPE_DEC_AVANTI          ((cpu_subtype_t) 13)
#define CPU_SUBTYPE_DEC_MUSTANG         ((cpu_subtype_t) 14)

/*
 *	ROMP subtypes.
 */

#define CPU_SUBTYPE_RT_PC	((cpu_subtype_t) 1)
#define CPU_SUBTYPE_RT_APC	((cpu_subtype_t) 2)
#define CPU_SUBTYPE_RT_135	((cpu_subtype_t) 3)

/*
 *	68020 subtypes.
 */

#define CPU_SUBTYPE_SUN3_50	((cpu_subtype_t) 1)
#define CPU_SUBTYPE_SUN3_160	((cpu_subtype_t) 2)
#define CPU_SUBTYPE_SUN3_260	((cpu_subtype_t) 3)
#define CPU_SUBTYPE_SUN3_110	((cpu_subtype_t) 4)
#define CPU_SUBTYPE_SUN3_60	((cpu_subtype_t) 5)

#define CPU_SUBTYPE_HP_320	((cpu_subtype_t) 6)
	/* 16.67 Mhz HP 300 series, custom MMU [HP 320] */
#define CPU_SUBTYPE_HP_330	((cpu_subtype_t) 7)
	/* 16.67 Mhz HP 300 series, MC68851 MMU [HP 318,319,330,349] */
#define CPU_SUBTYPE_HP_350	((cpu_subtype_t) 8)
	/* 25.00 Mhz HP 300 series, custom MMU [HP 350] */

/*
 *	32032/32332/32532 subtypes.
 */

#define CPU_SUBTYPE_MMAX_DPC	    ((cpu_subtype_t) 1)	/* 032 CPU */
#define CPU_SUBTYPE_SQT		    ((cpu_subtype_t) 2) /* Symmetry */
#define CPU_SUBTYPE_MMAX_APC_FPU    ((cpu_subtype_t) 3)	/* 32081 FPU */
#define CPU_SUBTYPE_MMAX_APC_FPA    ((cpu_subtype_t) 4)	/* Weitek FPA */
#define CPU_SUBTYPE_MMAX_XPC	    ((cpu_subtype_t) 5)	/* 532 CPU */
#define CPU_SUBTYPE_SQT86	    ((cpu_subtype_t) 6) /* ?? */

/*
 *	X86 & X86_64 subtypes.
 */

#define CPU_SUBTYPE_I386_ALL		((cpu_subtype_t) 3)
#define CPU_SUBTYPE_386			CPU_SUBTYPE_I386_ALL
#define CPU_SUBTYPE_486			((cpu_subtype_t) 4)
#define CPU_SUBTYPE_486SX		((cpu_subtype_t) 0x84)
#define CPU_SUBTYPE_586			((cpu_subtype_t) 5)
#define CPU_SUBTYPE_PENT		CPU_SUBTYPE_586
#define CPU_SUBTYPE_PENTPRO		((cpu_subtype_t) 0x16)
#define CPU_SUBTYPE_PENTII_M3		((cpu_subtype_t) 0x36)
#define CPU_SUBTYPE_PENTII_M5		((cpu_subtype_t) 0x56)
#define CPU_SUBTYPE_CELERON		((cpu_subtype_t) 0x67)
#define CPU_SUBTYPE_CELERON_MOBILE	((cpu_subtype_t) 0x77)
#define CPU_SUBTYPE_PENTIUM_3		((cpu_subtype_t) 0x08)
#define CPU_SUBTYPE_PENTIUM_3_M		((cpu_subtype_t) 0x18)
#define CPU_SUBTYPE_PENTIUM_3_XEON	((cpu_subtype_t) 0x28)
#define CPU_SUBTYPE_PENTIUM_M		((cpu_subtype_t) 0x09)
#define CPU_SUBTYPE_PENTIUM_4		((cpu_subtype_t) 0x0a)
#define CPU_SUBTYPE_PENTIUM_4_M		((cpu_subtype_t) 0x1a)
#define CPU_SUBTYPE_ITANIUM		((cpu_subtype_t) 0x0b)
#define CPU_SUBTYPE_ITANIUM_2		((cpu_subtype_t) 0x1b)
#define CPU_SUBTYPE_XEON		((cpu_subtype_t) 0x0c)
#define CPU_SUBTYPE_XEON_MP		((cpu_subtype_t) 0x1c)

#define CPU_SUBTYPE_X86_ALL		((cpu_subtype_t) 3)
#define CPU_SUBTYPE_X86_64_ALL		((cpu_subtype_t) 3)
#define CPU_SUBTYPE_X86_ARCH1		((cpu_subtype_t) 4)
#define CPU_SUBTYPE_X86_64_H		((cpu_subtype_t) 8)

#define CPU_SUBTYPE_INTEL_FAMILY_MAX	15
#define CPU_SUBTYPE_INTEL_MODEL_ALL	0

inline int CPU_SUBTYPE_INTEL(int family, int model) {
    return family | (model << 4);
}

inline int CPU_SUBTYPE_INTEL_FAMILY(cpu_subtype_t subtype) {
    return ((int)subtype & 0x0f);
}

inline int CPU_SUBTYPE_INTEL_MODEL(cpu_subtype_t subtype) {
    return ((int)subtype >> 4);
}

/*
 *	Mips subtypes.
 */

#define CPU_SUBTYPE_MIPS_R2300	((cpu_subtype_t) 1)
#define CPU_SUBTYPE_MIPS_R2600	((cpu_subtype_t) 2)
#define CPU_SUBTYPE_MIPS_R2800	((cpu_subtype_t) 3)
#define CPU_SUBTYPE_MIPS_R2000a	((cpu_subtype_t) 4)	/* pmax */
#define CPU_SUBTYPE_MIPS_R2000	((cpu_subtype_t) 5)
#define CPU_SUBTYPE_MIPS_R3000a	((cpu_subtype_t) 6)	/* 3max */
#define CPU_SUBTYPE_MIPS_R3000	((cpu_subtype_t) 7)

/*
 * 	MC68030 subtypes.
 */

#define CPU_SUBTYPE_NeXT	((cpu_subtype_t) 1) 
	/* NeXt thinks MC68030 is 6 rather than 9 */
#define CPU_SUBTYPE_HP_340	((cpu_subtype_t) 2) 
	/* 16.67 Mhz HP 300 series [HP 332,340] */
#define CPU_SUBTYPE_HP_360	((cpu_subtype_t) 3) 
	/* 25.00 Mhz HP 300 series [HP 360] */
#define CPU_SUBTYPE_HP_370	((cpu_subtype_t) 4) 
	/* 33.33 Mhz HP 300 series [HP 370] */

/*
 *	HPPA subtypes.
 */

#define CPU_SUBTYPE_HPPA_825		((cpu_subtype_t) 1)
#define CPU_SUBTYPE_HPPA_835		((cpu_subtype_t) 2)
#define CPU_SUBTYPE_HPPA_840		((cpu_subtype_t) 3)
#define CPU_SUBTYPE_HPPA_850		((cpu_subtype_t) 4)
#define CPU_SUBTYPE_HPPA_855		((cpu_subtype_t) 5)
#define	CPU_SUBTYPE_HPPA_705		((cpu_subtype_t) 6)
#define	CPU_SUBTYPE_HPPA_710		((cpu_subtype_t) 7)
#define	CPU_SUBTYPE_HPPA_720		((cpu_subtype_t) 8)
#define	CPU_SUBTYPE_HPPA_725		((cpu_subtype_t) 9)
#define	CPU_SUBTYPE_HPPA_730       	((cpu_subtype_t)10)
#define	CPU_SUBTYPE_HPPA_750		((cpu_subtype_t)11)
#define	CPU_SUBTYPE_HPPA_770		((cpu_subtype_t)12)
#define	CPU_SUBTYPE_HPPA_777		((cpu_subtype_t)13)
#define	CPU_SUBTYPE_HPPA_712		((cpu_subtype_t)14)
#define	CPU_SUBTYPE_HPPA_715		((cpu_subtype_t)15)

/*
 * 	ARM subtypes.
 */

#define CPU_SUBTYPE_ARM_ALL		((cpu_subtype_t) 0)
#define CPU_SUBTYPE_ARM_A500_ARCH	((cpu_subtype_t) 1)
#define CPU_SUBTYPE_ARM_A500		((cpu_subtype_t) 2)
#define CPU_SUBTYPE_ARM_A440		((cpu_subtype_t) 3)
#define CPU_SUBTYPE_ARM_M4		((cpu_subtype_t) 4)
#define CPU_SUBTYPE_ARM_V4T		((cpu_subtype_t) 5)
#define CPU_SUBTYPE_ARM_V6		((cpu_subtype_t) 6)
#define CPU_SUBTYPE_ARM_V5		((cpu_subtype_t) 7)
#define CPU_SUBTYPE_ARM_V5TEJ		CPU_SUBTYPE_ARM_V5
#define CPU_SUBTYPE_ARM_XSCALE		((cpu_subtype_t) 8)
#define CPU_SUBTYPE_ARM_V7		((cpu_subtype_t) 9)
#define CPU_SUBTYPE_ARM_V7F		((cpu_subtype_t) 10) /* unused */
#define CPU_SUBTYPE_ARM_V7S		((cpu_subtype_t) 11)
#define CPU_SUBTYPE_ARM_V7K		((cpu_subtype_t) 12)
#define CPU_SUBTYPE_ARM_V6M		((cpu_subtype_t) 14)
#define CPU_SUBTYPE_ARM_V7M		((cpu_subtype_t) 15)
#define CPU_SUBTYPE_ARM_V7EM		((cpu_subtype_t) 16)

#define CPU_SUBTYPE_ARM64_ALL		((cpu_subtype_t) 0)
#define CPU_SUBTYPE_ARM64_V8		((cpu_subtype_t) 1)
#define CPU_SUBTYPE_ARM64E		((cpu_subtype_t) 2)

/*
 *	MC88000 subtypes.
 */

#define CPU_SUBTYPE_MMAX_JPC		((cpu_subtype_t) 1)
#define CPU_SUBTYPE_LUNA88K             ((cpu_subtype_t) 2)

/*
 *	Sparc subtypes.
 */

#define CPU_SUBTYPE_SUN4_260		((cpu_subtype_t) 1)
#define CPU_SUBTYPE_SUN4_110		((cpu_subtype_t) 2)
#define CPU_SUBTYPE_SUN4_330		((cpu_subtype_t) 3)
#define CPU_SUBTYPE_SUN4C_60		((cpu_subtype_t) 4)
#define CPU_SUBTYPE_SUN4C_65		((cpu_subtype_t) 5)
#define CPU_SUBTYPE_SUN4C_20		((cpu_subtype_t) 6)
#define CPU_SUBTYPE_SUN4C_30		((cpu_subtype_t) 7)
#define CPU_SUBTYPE_SUN4C_40		((cpu_subtype_t) 8)
#define CPU_SUBTYPE_SUN4C_50		((cpu_subtype_t) 9)
#define CPU_SUBTYPE_SUN4C_75		((cpu_subtype_t) 10)

/*
 *	i860 subtypes.
 */

#define CPU_SUBTYPE_iPSC860		((cpu_subtype_t) 1)
#define CPU_SUBTYPE_PARAGON860		((cpu_subtype_t) 2)
#define CPU_SUBTYPE_PARAGON860_MP	((cpu_subtype_t) 3)
#define CPU_SUBTYPE_OKI860		((cpu_subtype_t) 4)

/*
 *	Alpha subtypes.
 */

#define CPU_SUBTYPE_ALPHA_EV3		((cpu_subtype_t) 1)
#define CPU_SUBTYPE_ALPHA_EV4		((cpu_subtype_t) 2)
#define CPU_SUBTYPE_ALPHA_ISP		((cpu_subtype_t) 3)
#define CPU_SUBTYPE_ALPHA_21064		((cpu_subtype_t) 4)

/*
 *	PowerPc subtypes. These are defined as the values in the upper
 *	16 bits of the PVR register of the CPU in question.
 */

#define CPU_SUBTYPE_PPC601		((cpu_subtype_t) 1)
#define CPU_SUBTYPE_PPC603		((cpu_subtype_t) 3)
#define CPU_SUBTYPE_PPC604		((cpu_subtype_t) 4)
#define CPU_SUBTYPE_PPC602		((cpu_subtype_t) 5)
#define CPU_SUBTYPE_PPC603e		((cpu_subtype_t) 6)
#define CPU_SUBTYPE_PPC603ev		((cpu_subtype_t) 7)
#define CPU_SUBTYPE_PPC604e		((cpu_subtype_t) 9)
#define CPU_SUBTYPE_PPC620		((cpu_subtype_t) 20)

#endif	/* _MACH_MACHINE_H_ */
