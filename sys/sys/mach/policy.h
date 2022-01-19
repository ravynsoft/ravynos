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
 * Revision 2.4  91/05/14  16:58:29  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:35:22  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:20:11  mrt]
 * 
 * Revision 2.2  90/06/02  14:59:37  rpd
 * 	Created for new host/processor technology.
 * 	[90/03/26  23:51:22  rpd]
 * 
 * 	Cleanup changes.
 * 	[89/08/02            dlb]
 * 	Created.
 * 	[89/07/25  18:47:00  dlb]
 * 
 * Revision 2.3  89/10/15  02:05:50  rpd
 * 	Minor cleanups.
 * 
 * Revision 2.2  89/10/11  14:40:53  dlb
 * 	Cleanup changes.
 * 	[89/08/02            dlb]
 * 
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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

#ifndef	_MACH_POLICY_H_
#define _MACH_POLICY_H_

/*
 *	mach/policy.h
 *
 *	Definitions for scheduing policy.
 */

#include <sys/mach/vm_types.h>

/*
 *	Policy definitions.  Policies should be powers of 2,
 *	but cannot be or'd together other than to test for a
 *	policy 'class'.
 */
#define	POLICY_TIMESHARE	1	/* timesharing		*/
#define	POLICY_RR		2	/* fixed round robin	*/
#define POLICY_FIFO		4	/* fixed fifo		*/

/*
 *	Check if policy is of 'class' fixed-priority.
 */
#define	POLICYCLASS_FIXEDPRI	(POLICY_RR | POLICY_FIFO)

/*
 *	Check if policy is valid.
 */
#define invalid_policy(policy)			\
	((policy) != POLICY_TIMESHARE &&	\
	 (policy) != POLICY_RR &&		\
	 (policy) != POLICY_FIFO)


/*
 *	New scheduling control interface
 */
typedef int 				policy_t;
typedef integer_t			*policy_info_t;
typedef integer_t			*policy_base_t;
typedef integer_t			*policy_limit_t;


/*
 * 	Types for TIMESHARE policy
 */
struct policy_timeshare_base {			
	integer_t		base_priority;
};
struct policy_timeshare_limit {
	integer_t		max_priority;
};
struct policy_timeshare_info {
	integer_t		max_priority;
	integer_t		base_priority;
	integer_t		cur_priority;
	boolean_t		depressed;
	integer_t		depress_priority;
};

typedef struct policy_timeshare_base	*policy_timeshare_base_t; 
typedef struct policy_timeshare_limit	*policy_timeshare_limit_t;
typedef struct policy_timeshare_info	*policy_timeshare_info_t;

typedef struct policy_timeshare_base	policy_timeshare_base_data_t; 
typedef struct policy_timeshare_limit	policy_timeshare_limit_data_t;
typedef struct policy_timeshare_info	policy_timeshare_info_data_t;


#define POLICY_TIMESHARE_BASE_COUNT		\
	(sizeof(struct policy_timeshare_base)/sizeof(integer_t))
#define POLICY_TIMESHARE_LIMIT_COUNT		\
	(sizeof(struct policy_timeshare_limit)/sizeof(integer_t))
#define POLICY_TIMESHARE_INFO_COUNT		\
	(sizeof(struct policy_timeshare_info)/sizeof(integer_t))


/*
 *	Types for the ROUND ROBIN (RR) policy
 */
struct policy_rr_base {				
	integer_t		base_priority;
	integer_t		quantum;
};
struct policy_rr_limit {
	integer_t		max_priority;
};
struct policy_rr_info {
	integer_t		max_priority;
	integer_t		base_priority;
	integer_t		quantum;
	boolean_t		depressed;
	integer_t		depress_priority;
};

typedef struct policy_rr_base		*policy_rr_base_t;
typedef struct policy_rr_limit		*policy_rr_limit_t;
typedef struct policy_rr_info		*policy_rr_info_t;

typedef struct policy_rr_base		policy_rr_base_data_t;
typedef struct policy_rr_limit		policy_rr_limit_data_t;
typedef struct policy_rr_info		policy_rr_info_data_t;

#define POLICY_RR_BASE_COUNT		\
	(sizeof(struct policy_rr_base)/sizeof(integer_t))
#define POLICY_RR_LIMIT_COUNT		\
	(sizeof(struct policy_rr_limit)/sizeof(integer_t))
#define POLICY_RR_INFO_COUNT		\
	(sizeof(struct policy_rr_info)/sizeof(integer_t))


/*
 * 	Types for the FIRST-IN-FIRST-OUT (FIFO) policy
 */
struct policy_fifo_base {		
	integer_t		base_priority;
};
struct policy_fifo_limit {
	integer_t		max_priority;
};
struct policy_fifo_info {
	integer_t		max_priority;
	integer_t		base_priority;
	boolean_t		depressed;
	integer_t		depress_priority;
};

typedef struct policy_fifo_base		*policy_fifo_base_t;
typedef struct policy_fifo_limit	*policy_fifo_limit_t;
typedef struct policy_fifo_info		*policy_fifo_info_t;

typedef struct policy_fifo_base		policy_fifo_base_data_t;
typedef struct policy_fifo_limit	policy_fifo_limit_data_t;
typedef struct policy_fifo_info		policy_fifo_info_data_t;

#define POLICY_FIFO_BASE_COUNT		\
	(sizeof(struct policy_fifo_base)/sizeof(integer_t))
#define POLICY_FIFO_LIMIT_COUNT		\
	(sizeof(struct policy_fifo_limit)/sizeof(integer_t))
#define POLICY_FIFO_INFO_COUNT		\
	(sizeof(struct policy_fifo_info)/sizeof(integer_t))


/*
 * 	Aggregate policy types
 */

struct policy_bases {
	policy_timeshare_base_data_t	ts;
	policy_rr_base_data_t		rr;
	policy_fifo_base_data_t		fifo;
};

struct policy_limits {
	policy_timeshare_limit_data_t	ts;
	policy_rr_limit_data_t		rr;
	policy_fifo_limit_data_t	fifo;
};

struct policy_infos {
	policy_timeshare_info_data_t	ts;
	policy_rr_info_data_t		rr;
	policy_fifo_info_data_t		fifo;
};

typedef struct policy_bases		policy_base_data_t;
typedef struct policy_limits		policy_limit_data_t;
typedef struct policy_infos		policy_info_data_t;


#endif /* _MACH_POLICY_H_ */
