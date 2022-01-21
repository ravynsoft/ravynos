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

/*
 *	File:		clock_types.h
 *	Purpose:	Clock facility header definitions. These
 *			definitons are needed by both kernel and
 *			user-level software.
 */

#ifndef	_MACH_CLOCK_TYPES_H_
#define	_MACH_CLOCK_TYPES_H_

/*
 * Reserved clock id values for default clocks.
 */
#define	REALTIME_CLOCK	0		/* required for all systems */
#define BATTERY_CLOCK	1		/* optional */
#define HIGHRES_CLOCK	2		/* optional */

/* Any number of distributed clocks may be defined. */
#define DIST_CLOCK_0	3		/* optional */
#define DIST_CLOCK_1	4		/* optional */

/*
 * Type definitions.
 */
typedef	int	alarm_type_t;		/* alarm time type */
typedef int	sleep_type_t;		/* sleep time type */
typedef	int	clock_id_t;		/* clock identification type */
typedef int	clock_flavor_t;		/* clock flavor type */
typedef int	*clock_attr_t;		/* clock attribute type */
typedef int	clock_res_t;		/* clock resolution type */


/*
 * Attribute names.
 */
#define	CLOCK_GET_TIME_RES	1	/* get_time call resolution */
#define	CLOCK_MAP_TIME_RES	2	/* map_time call resolution */
#define CLOCK_ALARM_CURRES	3	/* current alarm resolution */
#define CLOCK_ALARM_MINRES	4	/* minimum alarm resolution */
#define CLOCK_ALARM_MAXRES	5	/* maximum alarm resolution */
#define CLOCK_BASE_FREQ		6	/* base counter frequency */
/* Attribute names specific to DISTRIBUTED_CLOCK */
#define DCLOCK_STATE			7
#define DCLOCK_PARAM_BOUND		8
#define DCLOCK_PARAM_DRIFT_CONST 	9
#define DCLOCK_OPARAM_REFCLOCK		10	

/* DISTRIBUTED_CLOCK states (accessible via attribute DCLOCK_STATE) */
#define DCLOCK_STATE_UNINITIALIZED		0
#define DCLOCK_STATE_MANAGED			1
#define DCLOCK_STATE_INITIALIZED		2
#define	DCLOCK_STATE_RUNNING			3
#define	DCLOCK_STATE_FAILED			4

/*
 * Normal time specification used by the kernel clock facility.
 */
struct tvalspec {
	unsigned int	tv_sec;			/* seconds */
	clock_res_t	tv_nsec;		/* nanoseconds */
};
typedef struct tvalspec	tvalspec_t;
typedef struct tvalspec mach_timespec_t;


#ifdef NSEC_PER_SEC
#undef NSEC_PER_SEC
#endif
#ifdef USEC_PER_SEC
#undef USEC_PER_SEC
#endif
#ifdef NSEC_PER_USEC
#undef NSEC_PER_USEC
#endif
#define NSEC_PER_USEC	1000ll		/* nanoseconds per microsecond */
#define USEC_PER_SEC	1000000ll	/* microseconds per second */
#define NSEC_PER_SEC	1000000000ll /* nanoseconds per second */
#define BAD_TVALSPEC(t)							\
	((t)->tv_nsec < 0 || (t)->tv_nsec >= NSEC_PER_SEC)

/* t1 <=> t2 */
#define CMP_TVALSPEC(t1, t2)						\
	((t1)->tv_sec > (t2)->tv_sec ? +1 :				\
	((t1)->tv_sec < (t2)->tv_sec ? -1 : (t1)->tv_nsec - (t2)->tv_nsec))

/* t1  += t2 */
#define ADD_TVALSPEC(t1, t2)						\
{									\
	if (((t1)->tv_nsec += (t2)->tv_nsec) >= NSEC_PER_SEC) {		\
		(t1)->tv_nsec -= NSEC_PER_SEC;				\
		(t1)->tv_sec  += 1;					\
	}								\
	(t1)->tv_sec += (t2)->tv_sec;					\
}

/* t1  -= t2 */
#define SUB_TVALSPEC(t1, t2)						\
{									\
	if (((t1)->tv_nsec -= (t2)->tv_nsec) < 0) {			\
		(t1)->tv_nsec += NSEC_PER_SEC;				\
		(t1)->tv_sec  -= 1;					\
	}								\
	(t1)->tv_sec -= (t2)->tv_sec;					\
}

/*
 * Mapped time specification used by the kernel clock facility.
 */
struct	mapped_tvalspec {
	tvalspec_t	mtv_time;
#define	mtv_sec		mtv_time.tv_sec		/* seconds */
#define mtv_nsec	mtv_time.tv_nsec	/* nanoseconds */
	unsigned int	mtv_csec;		/* check seconds */
};
typedef struct mapped_tvalspec	mapped_tvalspec_t;

/*
 * Macro for reading a consistant tvalspec_t value "ts"
 * from a mapped time specification "mts". (On a multi
 * processor, it is assumed that processors see writes
 * in the "correct" order since the kernel updates the
 * mapped time in the inverse order it is read here.)
 */
#define MTS_TO_TS(mts, ts)				\
	do {						\
		(ts)->tv_sec  = (mts)->mtv_sec;		\
		(ts)->tv_nsec = (mts)->mtv_nsec;	\
	} while ((ts)->tv_sec != (mts)->mtv_csec);

/*
 * Alarm parameter defines.
 */
#define ALRMTYPE	0xff		/* type (8-bit field)	*/
#define   TIME_ABSOLUTE	0x0		/* absolute time */
#define	  TIME_RELATIVE	0x1		/* relative time */
#define BAD_ALRMTYPE(t)	\
	(((t) & 0xfe) != 0)

#endif /* _MACH_CLOCK_TYPES_H_ */
