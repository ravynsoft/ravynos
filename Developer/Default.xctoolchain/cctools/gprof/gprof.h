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
/*	$OpenBSD: gprof.h,v 1.4 1996/12/22 20:24:24 rahnds Exp $	*/
/*	$NetBSD: gprof.h,v 1.13 1996/04/01 21:54:06 mark Exp $	*/

/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)gprof.h	8.1 (Berkeley) 6/6/93
 */
#import <stdio.h>
#import <sys/types.h>
#import <sys/stat.h>
#import <gnu/a.out.h>
#ifdef __OPENSTEP__
#import <mach-o/rld_state.h>
#import <mach-o/gmon.h>
#else
#import <sys/gmon.h>
#endif
#import "stuff/bool.h"

/*
 * Used for comparison routine return values.
 */
#define	LESSTHAN	-1
#define	EQUALTO		0
#define	GREATERTHAN	1

#ifdef	NeXT_MOD
#define	UNIT short		/* unit of profiling */
#else
typedef	short UNIT;		/* unit of profiling */
#endif

/*
 * Progname for error messages.
 */
extern char *progname;

/*
 * Ticks per second.
 */
extern uint32_t hz;

extern char *a_outname;
#define	A_OUTNAME	"a.out"

extern char *gmonname;
#define	GMONNAME	"gmon.out"
#define	GMONSUM		"gmon.sum"
	
/*
 * Blurbs on the flat and graph profiles.
 */
#ifdef __OPENSTEP__
#define	FLAT_BLURB	"/usr/lib/gprof.flat"
#define	CALLG_BLURB	"/usr/lib/gprof.callg"
#else
#define	FLAT_BLURB	"/usr/share/gprof.flat"
#define	CALLG_BLURB	"/usr/share/gprof.callg"
#endif

/*
 * a constructed arc,
 *     with pointers to the namelist entry of the parent and the child,
 *     a count of how many times this arc was traversed,
 *     and pointers to the next parent of this child and
 * 	the next child of this parent.
 */
struct arcstruct {
    struct nl		*arc_parentp;	/* pointer to parent's nl entry */
    struct nl		*arc_childp;	/* pointer to child's nl entry */
    uint32_t		arc_count;	/* how calls from parent to child */
    uint32_t		arc_order;	/* order called */
    double		arc_time;	/* time inherited along arc */
    double		arc_childtime;	/* childtime inherited along arc */
    struct arcstruct	*arc_parentlist; /* parents-of-this-child list */
    struct arcstruct	*arc_childlist;	/* children-of-this-parent list */
};
typedef struct arcstruct	arctype;

/*
 * The symbol table;
 * for each external in the specified file we gather
 * its address, the number of calls and compute its share of cpu time.
 */
struct nl {
    char		*name;		/* the name */
    uint64_t		value;		/* the pc entry point */
    uint64_t		svalue;		/* entry point aligned to histograms */
    double		time;		/* ticks in this routine */
    double		childtime;	/* cumulative ticks in children */
    uint32_t		order;		/* order called */
    int32_t		ncall;		/* how many times called */
    int32_t		selfcalls;	/* how many calls to self */
    double		propfraction;	/* what % of time propagates */
    double		propself;	/* how much self time propagates */
    double		propchild;	/* how much child time propagates */
    enum bool		printflag;	/* should this be printed? */
    int			index;		/* index in the graph list */
    int			toporder;	/* graph call chain top-sort order */
    int			cycleno;	/* internal number of cycle on */
    struct nl		*cyclehead;	/* pointer to head of cycle */
    struct nl		*cnext;		/* pointer to next member of cycle */
    arctype		*parents;	/* list of caller arcs */
    arctype		*children;	/* list of callee arcs */
};
typedef struct nl	nltype;

extern nltype	*nl;			/* the whole namelist */
extern nltype	*npe;			/* the virtual end of the namelist */
extern uint32_t nname;		/* the number of function names */

/*
 * The list of file names and the ranges their pc's cover used for building
 * order files with the -S option.
 */
struct file { 
    uint64_t firstpc;
    uint64_t lastpc; 
    char *name;
    char *what_name;
}; 
extern struct file *files;
extern uint32_t n_files;

/*
 * flag which marks a nl entry as topologically ``busy''
 * flag which marks a nl entry as topologically ``not_numbered''
 */
#define	DFN_BUSY	-1
#define	DFN_NAN		0

/* 
 * namelist entries for cycle headers.
 * the number of discovered cycles.
 */
extern nltype	*cyclenl;	/* cycle header namelist */
extern int	ncycle;		/* number of cycles discovered */

/*
 * The information for the pc sample sets from the gmon.out file.
 */
struct sample_set {
    uint64_t s_lowpc;		/* lowpc from the profile file */
    uint64_t s_highpc;		/* highpc from the profile file */
    uint64_t lowpc;		/* range profiled, in UNIT's */
    uint64_t highpc;
    uint64_t sampbytes;		/* number of bytes of samples */
    uint64_t nsamples;		/* number of samples */
    unsigned UNIT *samples;	/* in core accumulated samples */
    double scale;		/* scale factor converting samples to
				   pc values: each sample covers scale
				   bytes */
#ifndef __OPENSTEP__
    int32_t version;
    int32_t profrate;
    int32_t spare[3];
#endif
};
extern struct sample_set *sample_sets;
extern uint32_t nsample_sets;
    
#ifdef __OPENSTEP__
/*
 * The rld loaded state from the gmon.out file.
 */
struct rld_loaded_state *grld_loaded_state;
extern uint32_t grld_nloaded_states;
extern void get_rld_state_symbols(void);
#endif

/*
 * The dyld images from the gmon.out file.
 */
struct dyld_image {
    char *name;
    uint64_t vmaddr_slide;
    uint64_t image_header;
};
extern uint32_t image_count;
extern struct dyld_image *dyld_images;
extern void get_dyld_state_symbols(void);

extern unsigned char	*textspace;	/* text space of a.out in core */

extern double	totime;			/* total time for all routines */
extern double	printtime;		/* total of time being printed */
extern double	actime;			/* accumulated time thus far for
					   putprofline */

/*
 * Option flags, from a to z.
 */
extern enum bool aflag;		/* suppress static functions */
extern enum bool bflag;		/* blurbs, too */
extern enum bool cflag;		/* discovered call graph, too */
extern enum bool dflag;		/* debugging options */
extern enum bool eflag;		/* specific functions excluded */
extern enum bool Eflag;		/* functions excluded with time */
extern enum bool fflag;		/* specific functions requested */
extern enum bool Fflag;		/* functions requested with time */
extern enum bool sflag;		/* sum multiple gmon.out files */
extern enum bool Sflag;		/* produce order file for scatter loading */
extern enum bool xflag;		/* don't produce gmon.order file */
extern enum bool zflag;		/* zero time/called functions, too */

/*
 * Structure for various string lists.
 */
struct stringlist {
    struct stringlist	*next;
    char		*string;
};
extern struct stringlist *elist;
extern struct stringlist *Elist;
extern struct stringlist *flist;
extern struct stringlist *Flist;

/*
 * The debug value for debugging gprof.
 */
extern uint32_t debug;

#define	DFNDEBUG	1
#define	CYCLEDEBUG	2
#define	ARCDEBUG	4
#define	TALLYDEBUG	8
#define	TIMEDEBUG	16
#define	SAMPLEDEBUG	32
#define	AOUTDEBUG	64
#define	CALLSDEBUG	128
#define	LOOKUPDEBUG	256
#define	PROPDEBUG	512
#define	ANYDEBUG	1024
#define	RLDDEBUG	2048
#define	DYLDDEBUG	4096

struct shlib_text_range {
    uint32_t lowpc;
    uint32_t highpc;
};
extern struct shlib_text_range *shlib_text_ranges;
extern uint32_t nshlib_text_ranges;

/*
 * External function declarations for the functions of the gprof source.
 */

/* arcs.c */
    extern void addarc(
	nltype *parentp,
	nltype *childp,
	uint32_t count,
	uint32_t order);

    extern nltype **doarcs(
	void);

/* calls.c */
    extern void findcalls(
	nltype *parentp,
	uint32_t p_lowpc,
	uint32_t p_highpc);

    /* dfn.c */
    extern void dfn(
	nltype *parentp);

/* getnfile.c */
    extern void getnfile(
	    void);

    extern void get_text_min_max(
	uint64_t *text_min,
	uint64_t *text_max);

/* hertz.c */
    extern uint32_t hertz(
	void);

/* lookup.c */
    extern nltype *nllookup(
	uint64_t address);

    extern arctype *arclookup(
	nltype *parentp,
	nltype *childp);

/* printgprof.c */
    extern void printgprof(
	nltype **timesortnlp);

    extern void printprof(
	void);

    extern void printindex(
	void);

    extern void printname(
	nltype *selfp);

    extern int totalcmp(
	nltype **npp1,
	nltype **npp2);

/* printlist.c */
    extern void addlist(
	struct stringlist *listp,
	char *funcname);

    extern enum bool onlist(
	struct stringlist *listp,
	char *funcname);

/* scatter.c */
    extern void printscatter(
	void);
