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
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * Revision 1.1.1.1  1997/09/03 20:53:37  roland
 * Initial checkin of SGS release 244
 *
 *  2 July 1992	Mac Gillon at NeXT
 *	Changed HPPA subtypes to follow our practice. 
 *
 * 11 September 1992 David E. Bohman at NeXT
 *	Added CPU_SUBTYPE_486SX to the i386 family.
 *
 * 16 July 1992 David E. Bohman at NeXT
 *	Added CPU_SUBTYPE_586 to the i386 family.
 *
 * 17-Dec-91  Peter King (king) at NeXT
 *	Added support for the XXX_ALL subtypes.  These are used to
 *	tag object files that can run on any implementation of a
 *	particular family.
 *
 *  1-Mar-90  John Seamons (jks) at NeXT
 *	Redefined cpu_type and cpu_subtype definitions to indicate processor
 *	architecture instead of product types for the MC680x0.
 *
 * Revision 2.15  89/10/11  14:39:56  dlb
 * 	Removed should_exit - replaced by action thread.
 * 	[89/01/25            dlb]
 * 
 * Revision 2.14  89/07/14  17:21:39  rvb
 * 	Added CPU types and subtypes for MC68030, MC68040, MC88000,
 * 	HPPA, ARM and Sun4-SPARC.
 * 	[89/07/13            mrt]
 * 
 * Revision 2.12  89/05/30  10:38:58  rvb
 * 	Add R2000 machine types.
 * 	[89/05/30  08:28:53  rvb]
 * 
 * Revision 2.11  89/04/18  16:43:32  mwyoung
 * 	Use <machine/vm_types.h> rather than <vm/vm_param.h> to get
 * 	VM types.  Remove old history... none of it was insightful.
 * 
 * 	The variable declarations should be moved elsewhere.
 * 	[89/01/24            mwyoung]
 * 
 */
/*
 *	Machine independent machine abstraction.
 *	Copyright (C) 1986, Avadis Tevanian, Jr.
 */

#ifndef	_MACH_MACHINE_H_
#define _MACH_MACHINE_H_

#import <mach/machine/vm_types.h>
#import <mach/boolean.h>

/*
 *	For each host, there is a maximum possible number of
 *	cpus that may be available in the system.  This is the
 *	compile-time constant NCPUS, which is defined in cpus.h.
 *
 *	In addition, there is a machine_slot specifier for each
 *	possible cpu in the system.
 */

struct machine_info {
	int		major_version;	/* kernel major version id */
	int		minor_version;	/* kernel minor version id */
	int		max_cpus;	/* max number of cpus compiled */
	int		avail_cpus;	/* number actually available */
	vm_size_t	memory_size;	/* size of memory in bytes */
};

typedef struct machine_info	*machine_info_t;
typedef struct machine_info	machine_info_data_t;	/* bogus */

typedef int	cpu_type_t;
typedef int	cpu_subtype_t;
typedef integer_t	cpu_threadtype_t;

#define CPU_STATE_MAX		3

#define CPU_STATE_USER		0
#define CPU_STATE_SYSTEM	1
#define CPU_STATE_IDLE		2

struct machine_slot {
	boolean_t	is_cpu;		/* is there a cpu in this slot? */
	cpu_type_t	cpu_type;	/* type of cpu */
	cpu_subtype_t	cpu_subtype;	/* subtype of cpu */
	volatile boolean_t running;	/* is cpu running */
	long		cpu_ticks[CPU_STATE_MAX];
	int		clock_freq;	/* clock interrupt frequency */
};

typedef struct machine_slot	*machine_slot_t;
typedef struct machine_slot	machine_slot_data_t;	/* bogus */

#ifdef	KERNEL
extern struct machine_info	machine_info;
extern struct machine_slot	machine_slot[];

extern vm_offset_t		interrupt_stack[];
#endif	/* KERNEL */

/*
 *	Machine types known by all.
 */
 
#define CPU_TYPE_ANY		((cpu_type_t) -1)

#define CPU_TYPE_VAX		((cpu_type_t) 1)
#define CPU_TYPE_ROMP		((cpu_type_t) 2)
#define CPU_TYPE_NS32032	((cpu_type_t) 4)
#define CPU_TYPE_NS32332        ((cpu_type_t) 5)
#define	CPU_TYPE_MC680x0	((cpu_type_t) 6)
#define CPU_TYPE_I386		((cpu_type_t) 7)
#define CPU_TYPE_X86		((cpu_type_t) 7)
#define CPU_TYPE_X86_64		((cpu_type_t) (CPU_TYPE_I386 | CPU_ARCH_ABI64))
#define CPU_TYPE_MIPS		((cpu_type_t) 8)
#define CPU_TYPE_NS32532        ((cpu_type_t) 9)
#define CPU_TYPE_HPPA           ((cpu_type_t) 11)
#define CPU_TYPE_ARM		((cpu_type_t) 12)
#define CPU_TYPE_MC88000	((cpu_type_t) 13)
#define CPU_TYPE_SPARC		((cpu_type_t) 14)
#define CPU_TYPE_I860		((cpu_type_t) 15) // big-endian
#define	CPU_TYPE_I860_LITTLE	((cpu_type_t) 16) // little-endian
#define CPU_TYPE_RS6000		((cpu_type_t) 17)
#define CPU_TYPE_MC98000	((cpu_type_t) 18)
#define CPU_TYPE_POWERPC	((cpu_type_t) 18)
#define CPU_ARCH_ABI64		 0x1000000
#define CPU_ARCH_ABI64_32	 0x2000000
#define CPU_ARCH_MASK        0xff000000      /* cctools-port */
#define CPU_TYPE_POWERPC64	((cpu_type_t)(CPU_TYPE_POWERPC | CPU_ARCH_ABI64))
#define CPU_TYPE_VEO		((cpu_type_t) 255)
#define CPU_TYPE_ARM64		((cpu_type_t)(CPU_TYPE_ARM | CPU_ARCH_ABI64))
#define CPU_TYPE_ARM64_32	((cpu_type_t)(CPU_TYPE_ARM | CPU_ARCH_ABI64_32))
		

/*
 *	Machine subtypes (these are defined here, instead of in a machine
 *	dependent directory, so that any program can get all definitions
 *	regardless of where is it compiled).
 */

/*
 * Capability bits used in the definition of cpu_subtype.
 */
#define CPU_SUBTYPE_MASK	0xff000000	/* mask for feature flags */
#define CPU_SUBTYPE_LIB64	0x80000000	/* 64 bit libraries */


/*
 *	Object files that are hand-crafted to run on any
 *	implementation of an architecture are tagged with
 *	CPU_SUBTYPE_MULTIPLE.  This functions essentially the same as
 *	the "ALL" subtype of an architecture except that it allows us
 *	to easily find object files that may need to be modified
 *	whenever a new implementation of an architecture comes out.
 *
 *	It is the responsibility of the implementor to make sure the
 *	software handles unsupported implementations elegantly.
 */
#define	CPU_SUBTYPE_MULTIPLE	((cpu_subtype_t) -1)


/*
 *	VAX subtypes (these do *not* necessary conform to the actual cpu
 *	ID assigned by DEC available via the SID register).
 */

#define	CPU_SUBTYPE_VAX_ALL	((cpu_subtype_t) 0) 
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
 *	ROMP subtypes.
 */

#define	CPU_SUBTYPE_RT_ALL	((cpu_subtype_t) 0)
#define CPU_SUBTYPE_RT_PC	((cpu_subtype_t) 1)
#define CPU_SUBTYPE_RT_APC	((cpu_subtype_t) 2)
#define CPU_SUBTYPE_RT_135	((cpu_subtype_t) 3)

/*
 *	32032/32332/32532 subtypes.
 */

#define	CPU_SUBTYPE_MMAX_ALL	    ((cpu_subtype_t) 0)
#define CPU_SUBTYPE_MMAX_DPC	    ((cpu_subtype_t) 1)	/* 032 CPU */
#define CPU_SUBTYPE_SQT		    ((cpu_subtype_t) 2)
#define CPU_SUBTYPE_MMAX_APC_FPU    ((cpu_subtype_t) 3)	/* 32081 FPU */
#define CPU_SUBTYPE_MMAX_APC_FPA    ((cpu_subtype_t) 4)	/* Weitek FPA */
#define CPU_SUBTYPE_MMAX_XPC	    ((cpu_subtype_t) 5)	/* 532 CPU */

/*
 *	I386 subtypes.
 */

#define	CPU_SUBTYPE_I386_ALL	((cpu_subtype_t) 3)
#define	CPU_SUBTYPE_X86_64_ALL	CPU_SUBTYPE_I386_ALL
#define CPU_SUBTYPE_386		((cpu_subtype_t) 3)
#define CPU_SUBTYPE_486		((cpu_subtype_t) 4)
#define CPU_SUBTYPE_486SX	((cpu_subtype_t) 4 + 128)
#define CPU_SUBTYPE_586		((cpu_subtype_t) 5)
#define CPU_SUBTYPE_INTEL(f, m)	((cpu_subtype_t) (f) + ((m) << 4))
#define CPU_SUBTYPE_PENT	CPU_SUBTYPE_INTEL(5, 0)
#define CPU_SUBTYPE_PENTPRO	CPU_SUBTYPE_INTEL(6, 1)
#define CPU_SUBTYPE_PENTII_M3	CPU_SUBTYPE_INTEL(6, 3)
#define CPU_SUBTYPE_PENTII_M5	CPU_SUBTYPE_INTEL(6, 5)
#define CPU_SUBTYPE_PENTIUM_4	CPU_SUBTYPE_INTEL(10, 0)

#define CPU_SUBTYPE_INTEL_FAMILY(x)	((x) & 15)
#define CPU_SUBTYPE_INTEL_FAMILY_MAX	15

#define CPU_SUBTYPE_INTEL_MODEL(x)	((x) >> 4)
#define CPU_SUBTYPE_INTEL_MODEL_ALL	0

#define CPU_SUBTYPE_X86_64_H	((cpu_subtype_t)8) /* Haswell and compatible */

/*
 *	Mips subtypes.
 */

#define	CPU_SUBTYPE_MIPS_ALL	((cpu_subtype_t) 0)
#define CPU_SUBTYPE_MIPS_R2300	((cpu_subtype_t) 1)
#define CPU_SUBTYPE_MIPS_R2600	((cpu_subtype_t) 2)
#define CPU_SUBTYPE_MIPS_R2800	((cpu_subtype_t) 3)
#define CPU_SUBTYPE_MIPS_R2000a	((cpu_subtype_t) 4)

/*
 * 	680x0 subtypes
 *
 * The subtype definitions here are unusual for historical reasons.
 * NeXT used to consider 68030 code as generic 68000 code.  For
 * backwards compatability:
 * 
 *	CPU_SUBTYPE_MC68030 symbol has been preserved for source code
 *	compatability.
 *
 *	CPU_SUBTYPE_MC680x0_ALL has been defined to be the same
 *	subtype as CPU_SUBTYPE_MC68030 for binary comatability.
 *
 *	CPU_SUBTYPE_MC68030_ONLY has been added to allow new object
 *	files to be tagged as containing 68030-specific instructions.
 */

#define	CPU_SUBTYPE_MC680x0_ALL		((cpu_subtype_t) 1)
#define CPU_SUBTYPE_MC68030		((cpu_subtype_t) 1) /* compat */
#define CPU_SUBTYPE_MC68040		((cpu_subtype_t) 2) 
#define	CPU_SUBTYPE_MC68030_ONLY	((cpu_subtype_t) 3)

/*
 *	HPPA subtypes for Hewlett-Packard HP-PA family of
 *	risc processors. Port by NeXT to 700 series. 
 */

#define	CPU_SUBTYPE_HPPA_ALL		((cpu_subtype_t) 0)
#define CPU_SUBTYPE_HPPA_7100		((cpu_subtype_t) 0) /* compat */
#define CPU_SUBTYPE_HPPA_7100LC		((cpu_subtype_t) 1)

/* 
 * 	Acorn subtypes - Acorn Risc Machine port done by
 *		Olivetti System Software Laboratory
 */

#define	CPU_SUBTYPE_ARM_ALL		((cpu_subtype_t) 0)
#define CPU_SUBTYPE_ARM_A500_ARCH	((cpu_subtype_t) 1)
#define CPU_SUBTYPE_ARM_A500		((cpu_subtype_t) 2)
#define CPU_SUBTYPE_ARM_A440		((cpu_subtype_t) 3)
#define CPU_SUBTYPE_ARM_M4		((cpu_subtype_t) 4)
#define CPU_SUBTYPE_ARM_V4T		((cpu_subtype_t) 5)
#define CPU_SUBTYPE_ARM_V6		((cpu_subtype_t) 6)
#define CPU_SUBTYPE_ARM_V5TEJ		((cpu_subtype_t) 7)
#define CPU_SUBTYPE_ARM_XSCALE		((cpu_subtype_t) 8)
#define CPU_SUBTYPE_ARM_V7		((cpu_subtype_t) 9)
#define CPU_SUBTYPE_ARM_V7F		((cpu_subtype_t) 10) /* Cortex A9 */
#define CPU_SUBTYPE_ARM_V7S		((cpu_subtype_t) 11) /* Swift */
#define CPU_SUBTYPE_ARM_V7K		((cpu_subtype_t) 12) /* Kirkwood40 */
#define CPU_SUBTYPE_ARM_V6M		((cpu_subtype_t) 14) /* Not meant to be run under xnu */
#define CPU_SUBTYPE_ARM_V7M		((cpu_subtype_t) 15) /* Not meant to be run under xnu */
#define CPU_SUBTYPE_ARM_V7EM		((cpu_subtype_t) 16) /* Not meant to be run under xnu */
#define CPU_SUBTYPE_ARM_V8		((cpu_subtype_t) 13)

#define	CPU_SUBTYPE_ARM64_ALL		((cpu_subtype_t) 0)
#define	CPU_SUBTYPE_ARM64_V8		((cpu_subtype_t) 1)

#define	CPU_SUBTYPE_ARM64_32_V8		((cpu_subtype_t) 1)
#define	CPU_SUBTYPE_ARM64E		((cpu_subtype_t) 2)
#define	CPU_SUBTYPE_ARM64_E		CPU_SUBTYPE_ARM64E /* cctools-port */

/*
 *	MC88000 subtypes
 */
#define	CPU_SUBTYPE_MC88000_ALL	((cpu_subtype_t) 0)
#define CPU_SUBTYPE_MMAX_JPC	((cpu_subtype_t) 1)
#define CPU_SUBTYPE_MC88100	((cpu_subtype_t) 1)
#define CPU_SUBTYPE_MC88110	((cpu_subtype_t) 2)

/*
 *	MC98000 (PowerPC) subtypes
 */
#define	CPU_SUBTYPE_MC98000_ALL	((cpu_subtype_t) 0)
#define CPU_SUBTYPE_MC98601	((cpu_subtype_t) 1)

/*
 *	I860 subtypes
 */
#define CPU_SUBTYPE_I860_ALL	((cpu_subtype_t) 0)
#define CPU_SUBTYPE_I860_860	((cpu_subtype_t) 1)

/*
 * 	I860 subtypes for NeXT-internal backwards compatability.
 *	These constants will be going away.  DO NOT USE THEM!!!
 */
#define CPU_SUBTYPE_LITTLE_ENDIAN	((cpu_subtype_t) 0)
#define CPU_SUBTYPE_BIG_ENDIAN		((cpu_subtype_t) 1)

/*
 *	I860_LITTLE subtypes
 */
#define	CPU_SUBTYPE_I860_LITTLE_ALL	((cpu_subtype_t) 0)
#define	CPU_SUBTYPE_I860_LITTLE	((cpu_subtype_t) 1)

/*
 *	RS6000 subtypes
 */
#define	CPU_SUBTYPE_RS6000_ALL	((cpu_subtype_t) 0)
#define CPU_SUBTYPE_RS6000	((cpu_subtype_t) 1)

/*
 *	Sun4 subtypes - port done at CMU
 */
#define	CPU_SUBTYPE_SUN4_ALL		((cpu_subtype_t) 0)
#define CPU_SUBTYPE_SUN4_260		((cpu_subtype_t) 1)
#define CPU_SUBTYPE_SUN4_110		((cpu_subtype_t) 2)

#define	CPU_SUBTYPE_SPARC_ALL		((cpu_subtype_t) 0)

/*
 *      PowerPC subtypes
 */
#define CPU_SUBTYPE_POWERPC_ALL		((cpu_subtype_t) 0)
#define CPU_SUBTYPE_POWERPC_601		((cpu_subtype_t) 1)
#define CPU_SUBTYPE_POWERPC_602		((cpu_subtype_t) 2)
#define CPU_SUBTYPE_POWERPC_603		((cpu_subtype_t) 3)
#define CPU_SUBTYPE_POWERPC_603e	((cpu_subtype_t) 4)
#define CPU_SUBTYPE_POWERPC_603ev	((cpu_subtype_t) 5)
#define CPU_SUBTYPE_POWERPC_604		((cpu_subtype_t) 6)
#define CPU_SUBTYPE_POWERPC_604e	((cpu_subtype_t) 7)
#define CPU_SUBTYPE_POWERPC_620		((cpu_subtype_t) 8)
#define CPU_SUBTYPE_POWERPC_750		((cpu_subtype_t) 9)
#define CPU_SUBTYPE_POWERPC_7400	((cpu_subtype_t) 10)
#define CPU_SUBTYPE_POWERPC_7450	((cpu_subtype_t) 11)
#define CPU_SUBTYPE_POWERPC_970		((cpu_subtype_t) 100)

/*
 * VEO subtypes
 * Note: the CPU_SUBTYPE_VEO_ALL will likely change over time to be defined as
 * one of the specific subtypes.
 */
#define CPU_SUBTYPE_VEO_1	((cpu_subtype_t) 1)
#define CPU_SUBTYPE_VEO_2	((cpu_subtype_t) 2)
#define CPU_SUBTYPE_VEO_3	((cpu_subtype_t) 3)
#define CPU_SUBTYPE_VEO_4	((cpu_subtype_t) 4)
#define CPU_SUBTYPE_VEO_ALL	CPU_SUBTYPE_VEO_2


#endif	/* _MACH_MACHINE_H_ */
