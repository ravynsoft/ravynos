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
 * Revision 2.4  91/05/14  16:58:46  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:35:31  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:20:39  mrt]
 * 
 * Revision 2.2  90/06/02  14:59:49  rpd
 * 	Created for new host/processor technology.
 * 	[90/03/26  23:51:38  rpd]
 * 
 * 	Merge to X96
 * 	[89/08/02  23:12:21  dlb]
 * 
 * 	Add scheduling flavor of information.
 * 	[89/07/25  18:52:18  dlb]
 * 
 * 	Add load average and mach factor to processor set basic info.
 * 	[89/02/09            dlb]
 * 
 * Revision 2.3  89/10/15  02:05:54  rpd
 * 	Minor cleanups.
 * 
 * Revision 2.2  89/10/11  14:41:03  dlb
 * 	Add scheduling flavor of information.
 * 
 * 	Add load average and mach factor to processor set basic info.
 * 	[89/02/09            dlb]
 * 
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 *	File:	mach/processor_info.h
 *	Author:	David L. Black
 *	Date:	1988
 *
 *	Data structure definitions for processor_info, processor_set_info
 */

#ifndef	_MACH_PROCESSOR_INFO_H_
#define _MACH_PROCESSOR_INFO_H_

#include <sys/mach/machine.h>

/*
 *	Generic information structure to allow for expansion.
 */
typedef integer_t	*mach_processor_info_t;	/* varying array of int. */
typedef mach_processor_info_t *processor_info_array_t;

#define PROCESSOR_INFO_MAX	(1024)	/* max array size */
typedef integer_t	processor_info_data_t[PROCESSOR_INFO_MAX];


typedef integer_t	*processor_set_info_t;	/* varying array of int. */

#define PROCESSOR_SET_INFO_MAX	(1024)	/* max array size */
typedef integer_t	processor_set_info_data_t[PROCESSOR_SET_INFO_MAX];


typedef int	*processor_slot_t;	/* varying array of int. */

/*
 *	Currently defined information.
 */
typedef int	processor_flavor_t;
#define	PROCESSOR_BASIC_INFO	1		/* basic information */

struct processor_basic_info {
	cpu_type_t	cpu_type;	/* type of cpu */
	cpu_subtype_t	cpu_subtype;	/* subtype of cpu */
	boolean_t	running;	/* is processor running */
	int		slot_num;	/* slot number */
	boolean_t	is_master;	/* is this the master processor */
};

typedef	struct processor_basic_info	processor_basic_info_data_t;
typedef struct processor_basic_info	*processor_basic_info_t;
#define PROCESSOR_BASIC_INFO_COUNT \
		(sizeof(processor_basic_info_data_t)/sizeof(natural_t))

/*
 *	Scaling factor for load_average, mach_factor.
 */
#define	LOAD_SCALE	1000		

typedef	int	processor_set_flavor_t;
#define	PROCESSOR_SET_BASIC_INFO	5	/* basic information */

struct processor_set_basic_info {
	int		processor_count;	/* How many processors */
	int		default_policy;		/* When others not enabled */
};

typedef	struct processor_set_basic_info	processor_set_basic_info_data_t;
typedef struct processor_set_basic_info	*processor_set_basic_info_t;
#define PROCESSOR_SET_BASIC_INFO_COUNT \
		(sizeof(processor_set_basic_info_data_t)/sizeof(natural_t))

#define PROCESSOR_SET_LOAD_INFO		4	/* scheduling statistics */

struct processor_set_load_info {
        int             task_count;             /* How many tasks */
        int             thread_count;           /* How many threads */
        integer_t       load_average;           /* Scaled */
        integer_t       mach_factor;            /* Scaled */
};

typedef struct processor_set_load_info processor_set_load_info_data_t;
typedef struct processor_set_load_info *processor_set_load_info_t;
#define PROCESSOR_SET_LOAD_INFO_COUNT	\
                (sizeof(processor_set_load_info_data_t)/sizeof(natural_t))


/*
 *      New scheduling control interface
 */
#define PROCESSOR_SET_ENABLED_POLICIES                   3
#define PROCESSOR_SET_ENABLED_POLICIES_COUNT	\
		(sizeof(policy_t)/sizeof(natural_t))

#define PROCESSOR_SET_TIMESHARE_DEFAULT                 10
#define PROCESSOR_SET_TIMESHARE_LIMITS                  11

#define PROCESSOR_SET_RR_DEFAULT                        20
#define PROCESSOR_SET_RR_LIMITS                         21

#define PROCESSOR_SET_FIFO_DEFAULT                      30
#define PROCESSOR_SET_FIFO_LIMITS                       31

#endif	/* _MACH_PROCESSOR_INFO_H_ */
