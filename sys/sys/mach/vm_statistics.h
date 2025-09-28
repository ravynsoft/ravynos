/*
 * Copyright 1991-1998 by Open Software Foundation, Inc. 
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
/*
 * MkLinux
 */
/* CMU_HIST */
/*
 * Revision 2.3  91/05/14  17:03:07  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:37:41  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:22:49  mrt]
 * 
 * Revision 2.1  89/08/03  16:06:55  rwd
 * Created.
 * 
 * Revision 2.4  89/02/25  18:42:35  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.3  89/02/07  00:54:39  mwyoung
 * Relocated from sys/vm_statistics.h
 * 
 * Revision 2.2  89/01/30  22:08:54  rpd
 * 	Made variable declarations use "extern".
 * 	[89/01/25  15:26:30  rpd]
 * 
 * 30-Sep-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Changed "reclaim" to "inactive."
 *
 * 22-Aug-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Made vm_stat structure kernel-only.
 *
 * 22-May-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Defined vm_statistics_data_t as a real typedef so that
 *	MatchMaker can deal with it.
 *
 * 14-Feb-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Created.
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
/*
 *	File:	mach/vm_statistics.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young, David Golub
 *
 *	Virtual memory statistics structure.
 *
 */

#ifndef	VM_STATISTICS_H_
#define	VM_STATISTICS_H_

#include <sys/mach/vm_types.h>

struct vm_statistics {
	integer_t	free_count;		/* # of pages free */
	integer_t	active_count;		/* # of pages active */
	integer_t	inactive_count;		/* # of pages inactive */
	integer_t	wire_count;		/* # of pages wired down */
	integer_t	zero_fill_count;	/* # of zero fill pages */
	integer_t	reactivations;		/* # of pages reactivated */
	integer_t	pageins;		/* # of pageins */
	integer_t	pageouts;		/* # of pageouts */
	integer_t	faults;			/* # of faults */
	integer_t	cow_faults;		/* # of copy-on-writes */
	integer_t	lookups;		/* object cache lookups */
	integer_t	hits;			/* object cache hits */

	integer_t       purgeable_count;        /* # of pages purgeable */
	integer_t       purges;                 /* # of pages purged */

};

typedef struct vm_statistics	*vm_statistics_t;
typedef struct vm_statistics	vm_statistics_data_t;

struct vm_extmod_statistics {
        int64_t task_for_pid_count;                     /* # of times task port was looked up */
        int64_t task_for_pid_caller_count;      /* # of times this task called task_for_pid */
        int64_t thread_creation_count;          /* # of threads created in task */
        int64_t thread_creation_caller_count;   /* # of threads created by task */
        int64_t thread_set_state_count;         /* # of register state sets in task */
        int64_t thread_set_state_caller_count;  /* # of register state sets by task */
} __attribute__((aligned(8)));

typedef struct vm_extmod_statistics *vm_extmod_statistics_t;
typedef struct vm_extmod_statistics vm_extmod_statistics_data_t;

typedef struct vm_purgeable_stat {
        uint64_t        count;
        uint64_t        size;
}vm_purgeable_stat_t;

struct vm_purgeable_info {
        vm_purgeable_stat_t fifo_data[8];
        vm_purgeable_stat_t obsolete_data;
        vm_purgeable_stat_t lifo_data[8];
};

typedef struct vm_purgeable_info        *vm_purgeable_info_t;


/*
 * VM allocation flags:
 * 
 * VM_FLAGS_FIXED
 *      (really the absence of VM_FLAGS_ANYWHERE)
 *      Allocate new VM region at the specified virtual address, if possible.
 * 
 * VM_FLAGS_ANYWHERE
 *      Allocate new VM region anywhere it would fit in the address space.
 *
 * VM_FLAGS_PURGABLE
 *      Create a purgable VM object for that new VM region.
 *
 * VM_FLAGS_NO_PMAP_CHECK
 *      (for DEBUG kernel config only, ignored for other configs)
 *      Do not check that there is no stale pmap mapping for the new VM region.
 *      This is useful for kernel memory allocations at bootstrap when building
 *      the initial kernel address space while some memory is already in use.
 *
 * VM_FLAGS_OVERWRITE
 *      The new VM region can replace existing VM regions if necessary
 *      (to be used in combination with VM_FLAGS_FIXED).
 *
 * VM_FLAGS_NO_CACHE
 *      Pages brought in to this VM region are placed on the speculative
 *      queue instead of the active queue.  In other words, they are not
 *      cached so that they will be stolen first if memory runs low.
 */
#define VM_FLAGS_FIXED          0x0000
#define VM_FLAGS_ANYWHERE       0x0001
#define VM_FLAGS_PURGABLE       0x0002
#define VM_FLAGS_NO_CACHE       0x0010
#define VM_FLAGS_OVERWRITE      0x4000  /* delete any existing mappings first */
/*
 * VM_FLAGS_SUPERPAGE_MASK
 *      3 bits that specify whether large pages should be used instead of
 *      base pages (!=0), as well as the requested page size.
 */
#define VM_FLAGS_SUPERPAGE_MASK 0x70000 /* bits 0x10000, 0x20000, 0x40000 */
#define VM_FLAGS_RETURN_DATA_ADDR       0x100000 /* Return address of target data, rather than base of page */
#define VM_FLAGS_ALIAS_MASK     0xFF000000
#define VM_GET_FLAGS_ALIAS(flags, alias)                        \
                (alias) = ((flags) & VM_FLAGS_ALIAS_MASK) >> 24 
#define VM_SET_FLAGS_ALIAS(flags, alias)                        \
                (flags) = (((flags) & ~VM_FLAGS_ALIAS_MASK) |   \
                (((alias) & ~VM_FLAGS_ALIAS_MASK) << 24))

/* These are the flags that we accept from user-space */
#define VM_FLAGS_USER_ALLOCATE  (VM_FLAGS_FIXED |               \
                                 VM_FLAGS_ANYWHERE |            \
                                 VM_FLAGS_PURGABLE |            \
                                 VM_FLAGS_NO_CACHE |            \
                                 VM_FLAGS_OVERWRITE |           \
                                 VM_FLAGS_SUPERPAGE_MASK |      \
                                 VM_FLAGS_ALIAS_MASK)
#define VM_FLAGS_USER_MAP       (VM_FLAGS_USER_ALLOCATE |       \
                                 VM_FLAGS_RETURN_DATA_ADDR)
#define VM_FLAGS_USER_REMAP     (VM_FLAGS_FIXED |    \
                                 VM_FLAGS_ANYWHERE | \
                                 VM_FLAGS_OVERWRITE| \
                                 VM_FLAGS_RETURN_DATA_ADDR)

/*
 *	Each machine dependent implementation is expected to
 *	keep certain statistics.  They may do this anyway they
 *	so choose, but are expected to return the statistics
 *	in the following structure.
 */
#if 0
struct pmap_statistics {
	integer_t	resident_count;	/* # of pages mapped (total)*/
	integer_t	wired_count;	/* # of pages wired */
};

typedef struct pmap_statistics	*pmap_statistics_t;
#endif
#endif	/* VM_STATISTICS_H_ */
