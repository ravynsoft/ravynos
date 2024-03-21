/*
 * rlimits.c - resource limit builtins
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1992-1997 Paul Falstad
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Paul Falstad or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Paul Falstad and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Paul Falstad and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Paul Falstad and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "rlimits.mdh"
#include "rlimits.pro"

#if defined(HAVE_GETRLIMIT) && defined(RLIM_INFINITY)

enum zlimtype {
    ZLIMTYPE_MEMORY,
    ZLIMTYPE_NUMBER,
    ZLIMTYPE_TIME,
    ZLIMTYPE_MICROSECONDS,
    ZLIMTYPE_UNKNOWN
};

typedef struct resinfo_T {
    int	res;		/* RLIMIT_XXX */
    char* name;		/* used by limit builtin */
    enum zlimtype type;
    int unit;		/* 1, 512, or 1024 */
    char opt;		/* option character */
    char* descr;	/* used by ulimit builtin */
} resinfo_T;

/* table of known resources */
/*
 * How to add a new resource:
 * 1. Add zsh_LIMIT_PRESENT(RLIMIT_XXX) in configure.ac.
 * 2. Add an entry for RLIMIT_XXX to known_resources[].
 *    Make sure the option letter (resinto_T.opt) is unique.
 * 3. Build zsh and run the test B12rlimit.ztst.
 */
static const resinfo_T known_resources[] = {
    {RLIMIT_CPU, "cputime", ZLIMTYPE_TIME, 1,
		't', "cpu time (seconds)"},
    {RLIMIT_FSIZE, "filesize", ZLIMTYPE_MEMORY, 512,
		'f', "file size (blocks)"},
    {RLIMIT_DATA, "datasize", ZLIMTYPE_MEMORY, 1024,
		'd', "data seg size (kbytes)"},
    {RLIMIT_STACK, "stacksize", ZLIMTYPE_MEMORY, 1024,
		's', "stack size (kbytes)"},
    {RLIMIT_CORE, "coredumpsize", ZLIMTYPE_MEMORY, 512,
		'c', "core file size (blocks)"},
# ifdef HAVE_RLIMIT_NOFILE
    {RLIMIT_NOFILE, "descriptors", ZLIMTYPE_NUMBER, 1,
		'n', "file descriptors"},
# endif
# if defined(HAVE_RLIMIT_AS) && !defined(RLIMIT_VMEM_IS_AS)
    {RLIMIT_AS, "addressspace", ZLIMTYPE_MEMORY, 1024,
		'v', "address space (kbytes)"},
# endif
# if defined(HAVE_RLIMIT_RSS) && !defined(RLIMIT_VMEM_IS_RSS) && !defined(RLIMIT_RSS_IS_AS)
    {RLIMIT_RSS, "resident", ZLIMTYPE_MEMORY, 1024,
		'm', "resident set size (kbytes)"},
# endif
# if defined(HAVE_RLIMIT_VMEM)
    {RLIMIT_VMEM,
#  if defined(RLIMIT_VMEM_IS_RSS)
		 "resident", ZLIMTYPE_MEMORY, 1024,
		 'm', "memory size (kbytes)"
#  else
		 "vmemorysize", ZLIMTYPE_MEMORY, 1024,
		 'v', "virtual memory size (kbytes)"
#  endif
    },
# endif
# ifdef HAVE_RLIMIT_NPROC
    {RLIMIT_NPROC, "maxproc", ZLIMTYPE_NUMBER, 1,
		'u', "processes"},
# endif
# ifdef HAVE_RLIMIT_MEMLOCK
    {RLIMIT_MEMLOCK, "memorylocked", ZLIMTYPE_MEMORY, 1024,
		'l', "locked-in-memory size (kbytes)"},
# endif
    /* Linux */
# ifdef HAVE_RLIMIT_LOCKS
    {RLIMIT_LOCKS, "maxfilelocks", ZLIMTYPE_NUMBER, 1,
		'x', "file locks"},
# endif
# ifdef HAVE_RLIMIT_SIGPENDING
    {RLIMIT_SIGPENDING, "sigpending", ZLIMTYPE_NUMBER, 1,
		'i', "pending signals"},
# endif
# ifdef HAVE_RLIMIT_MSGQUEUE
    {RLIMIT_MSGQUEUE, "msgqueue", ZLIMTYPE_NUMBER, 1,
		'q', "bytes in POSIX msg queues"},
# endif
# ifdef HAVE_RLIMIT_NICE
    {RLIMIT_NICE, "nice", ZLIMTYPE_NUMBER, 1,
		'e', "max nice"},
# endif
# ifdef HAVE_RLIMIT_RTPRIO
    {RLIMIT_RTPRIO, "rt_priority", ZLIMTYPE_NUMBER, 1,
		'r', "max rt priority"},
# endif
# ifdef HAVE_RLIMIT_RTTIME
    {RLIMIT_RTTIME, "rt_time", ZLIMTYPE_MICROSECONDS, 1,
		'N', "rt cpu time (microseconds)"},
# endif
    /* BSD */
# ifdef HAVE_RLIMIT_SBSIZE
    {RLIMIT_SBSIZE, "sockbufsize", ZLIMTYPE_MEMORY, 1,
		'b', "socket buffer size (bytes)"},
# endif
# ifdef HAVE_RLIMIT_KQUEUES /* FreeBSD */
    {RLIMIT_KQUEUES, "kqueues", ZLIMTYPE_NUMBER, 1,
		'k', "kqueues"},
# endif
# ifdef HAVE_RLIMIT_NPTS    /* FreeBSD */
    {RLIMIT_NPTS, "pseudoterminals", ZLIMTYPE_NUMBER, 1,
		'p', "pseudo-terminals"},
# endif
# ifdef HAVE_RLIMIT_SWAP    /* FreeBSD */
    {RLIMIT_SWAP, "swapsize", ZLIMTYPE_MEMORY, 1024,
		'w', "swap size (kbytes)"},
# endif
# ifdef HAVE_RLIMIT_UMTXP   /* FreeBSD */
    {RLIMIT_UMTXP, "umtxp", ZLIMTYPE_NUMBER, 1,
		'o', "umtx shared locks"},
# endif

# ifdef HAVE_RLIMIT_POSIXLOCKS	/* DragonFly */
    {RLIMIT_POSIXLOCKS, "posixlocks", ZLIMTYPE_NUMBER, 1,
		'x', "number of POSIX locks"},
# endif
# if defined(HAVE_RLIMIT_NTHR) && !defined(HAVE_RLIMIT_RTPRIO) /* Net/OpenBSD */
    {RLIMIT_NTHR, "maxpthreads", ZLIMTYPE_NUMBER, 1,
		'r', "threads"},
# endif
    /* others */
# if defined(HAVE_RLIMIT_PTHREAD) && !defined(HAVE_RLIMIT_NTHR)	/* IRIX ? */
    {RLIMIT_PTHREAD, "maxpthreads", ZLIMTYPE_NUMBER, 1,
		'T', "threads per process"},
# endif
# ifdef HAVE_RLIMIT_AIO_MEM /* HP-UX ? */
    {RLIMIT_AIO_MEM, "aiomemorylocked", ZLIMTYPE_MEMORY, 1024,
		'N', "AIO locked-in-memory (kbytes)"},
# endif
# ifdef HAVE_RLIMIT_AIO_OPS /* HP-UX ? */
    {RLIMIT_AIO_OPS, "aiooperations", ZLIMTYPE_NUMBER, 1,
		'N', "AIO operations"},
# endif
# ifdef HAVE_RLIMIT_TCACHE  /* HP-UX ? */
    {RLIMIT_TCACHE, "cachedthreads", ZLIMTYPE_NUMBER, 1,
		'N', "cached threads"},
# endif
};

/* resinfo[RLIMIT_XXX] points to the corresponding entry
 * in known_resources[] */
static const resinfo_T **resinfo;

/**/
static void
set_resinfo(void)
{
    int i;

    resinfo = (const resinfo_T **)zshcalloc(RLIM_NLIMITS*sizeof(resinfo_T *));

    for (i=0; i<sizeof(known_resources)/sizeof(resinfo_T); ++i) {
	resinfo[known_resources[i].res] = &known_resources[i];
    }
    for (i=0; i<RLIM_NLIMITS; ++i) {
	if (!resinfo[i]) {
	    /* unknown resource */
	    resinfo_T *info = (resinfo_T *)zshcalloc(sizeof(resinfo_T));
	    char *buf = (char *)zalloc(12);
	    snprintf(buf, 12, "UNKNOWN-%d", i);
	    info->res = - 1;	/* negative value indicates "unknown" */
	    info->name = buf;
	    info->type = ZLIMTYPE_UNKNOWN;
	    info->unit = 1;
	    info->opt = 'N';
	    info->descr = buf;
	    resinfo[i] = info;
	}
    }
}

/**/
static void
free_resinfo(void)
{
    int i;
    for (i=0; i<RLIM_NLIMITS; ++i) {
	if (resinfo[i]->res < 0) {  /* unknown resource */
	    free(resinfo[i]->name);
	    free((void*)resinfo[i]);
	}
    }
    free(resinfo);
    resinfo = NULL;
}

/* Find resource by its option character */

/**/
static int
find_resource(char c)
{
    int i;
    for (i=0; i<RLIM_NLIMITS; ++i) {
	if (resinfo[i]->opt == c)
	    return i;
    }
    return -1;
}

/* Print a value of type rlim_t */

/**/
static void
printrlim(rlim_t val, const char *unit)
{
# ifdef RLIM_T_IS_QUAD_T
	printf("%qd%s", val, unit);
# else
#  ifdef RLIM_T_IS_LONG_LONG
	printf("%lld%s", val, unit);
#  else
#   ifdef RLIM_T_IS_UNSIGNED
	printf("%lu%s", (unsigned long)val, unit);
#   else
	printf("%ld%s", (long)val, unit);
#   endif /* RLIM_T_IS_UNSIGNED */
#  endif /* RLIM_T_IS_LONG_LONG */
# endif /* RLIM_T_IS_QUAD_T */
}

/**/
static rlim_t
zstrtorlimt(const char *s, char **t, int base)
{
    rlim_t ret = 0;

    if (strcmp(s, "unlimited") == 0) {
	if (t)
	    *t = (char *) s + 9;
	return RLIM_INFINITY;
    }
# if defined(RLIM_T_IS_QUAD_T) || defined(RLIM_T_IS_LONG_LONG) || defined(RLIM_T_IS_UNSIGNED)
    if (!base) {
	if (*s != '0')
	    base = 10;
	else if (*++s == 'x' || *s == 'X')
	    base = 16, s++;
	else
	    base = 8;
    } 
    if (base <= 10)
	for (; *s >= '0' && *s < ('0' + base); s++)
	    ret = ret * base + *s - '0';
    else
	for (; idigit(*s) || (*s >= 'a' && *s < ('a' + base - 10))
	     || (*s >= 'A' && *s < ('A' + base - 10)); s++)
	    ret = ret * base + (idigit(*s) ? (*s - '0') : (*s & 0x1f) + 9);
    if (t)
	*t = (char *)s;
# else /* !RLIM_T_IS_QUAD_T && !RLIM_T_IS_LONG_LONG && !RLIM_T_IS_UNSIGNED */
    ret = zstrtol(s, t, base);
# endif /* !RLIM_T_IS_QUAD_T && !RLIM_T_IS_LONG_LONG && !RLIM_T_IS_UNSIGNED */
    return ret;
}

/**/
static void
showlimitvalue(int lim, rlim_t val)
{
    /* display limit for resource number lim */
    if (lim < RLIM_NLIMITS)
	printf("%-16s", resinfo[lim]->name);
    else
    {
	/* Unknown limit, hence unknown units. */
	printf("%-16d", lim);
    }
    if (val == RLIM_INFINITY)
	printf("unlimited\n");
    else if (lim >= RLIM_NLIMITS)
	printrlim(val, "\n");
    else if (resinfo[lim]->type == ZLIMTYPE_TIME) {
	/* time-type resource -- display as hours, minutes and
	   seconds. */
	printf("%d:%02d:%02d\n", (int)(val / 3600),
	       (int)(val / 60) % 60, (int)(val % 60));
    } else if (resinfo[lim]->type == ZLIMTYPE_MICROSECONDS)
	printrlim(val, "us\n");	/* microseconds */
    else if (resinfo[lim]->type == ZLIMTYPE_NUMBER ||
	       resinfo[lim]->type == ZLIMTYPE_UNKNOWN)
	printrlim(val, "\n");	/* pure numeric resource */
    else {
	/* memory resource -- display with `k' or `M' modifier */
	if (val >= 1024L * 1024L)
	    printrlim(val/(1024L * 1024L), "MB\n");
	else
	    printrlim(val/1024L, "kB\n");
    }
}

/* Display resource limits.  hard indicates whether `hard' or `soft'  *
 * limits should be displayed.  lim specifies the limit, or may be -1 *
 * to show all.                                                       */

/**/
static int
showlimits(char *nam, int hard, int lim)
{
    int rt;

    if (lim >= RLIM_NLIMITS)
    {
	/*
	 * Not configured into the shell.  Ask the OS
	 * explicitly for this limit.
	 */
	struct rlimit vals;
	if (getrlimit(lim, &vals) < 0)
	{
	    zwarnnam(nam, "can't read limit: %e", errno);
	    return 1;
	}
	showlimitvalue(lim, hard ? vals.rlim_max : vals.rlim_cur);
    }
    else if (lim != -1)
    {
	showlimitvalue(lim, hard ? limits[lim].rlim_max :
		       limits[lim].rlim_cur);
    }
    else
    {
	/* main loop over resource types */
	for (rt = 0; rt != RLIM_NLIMITS; rt++)
	    showlimitvalue(rt, (hard) ? limits[rt].rlim_max :
			   limits[rt].rlim_cur);
    }

    return 0;
}

/* Display a resource limit, in ulimit style.  lim specifies which   *
 * limit should be displayed, and hard indicates whether the hard or *
 * soft limit should be displayed.                                   */

/**/
static int
printulimit(char *nam, int lim, int hard, int head)
{
    rlim_t limit;

    /* get the limit in question */
    if (lim >= RLIM_NLIMITS)
    {
	struct rlimit vals;

	if (getrlimit(lim, &vals) < 0)
	{
	    zwarnnam(nam, "can't read limit: %e", errno);
	    return 1;
	}
	limit = (hard) ? vals.rlim_max : vals.rlim_cur;
    }
    else
	limit = (hard) ? limits[lim].rlim_max : limits[lim].rlim_cur;
    /* display the appropriate heading */
    if (head) {
	if (lim < RLIM_NLIMITS) {
	    const resinfo_T *info = resinfo[lim];
	    if (info->opt == 'N')
		printf("-N %2d: %-29s", lim, info->descr);
	    else
		printf("-%c: %-32s", info->opt, info->descr);
	}
	else
	    printf("-N %2d: %-29s", lim, "");
    }
    /* display the limit */
    if (limit == RLIM_INFINITY)
	printf("unlimited\n");
    else {
	if (lim < RLIM_NLIMITS)
	    printrlim(limit/resinfo[lim]->unit, "\n");
	else
	    printrlim(limit, "\n");
    }

    return 0;
}

/**/
static int
do_limit(char *nam, int lim, rlim_t val, int hard, int soft, int set)
{
    if (lim >= RLIM_NLIMITS) {
	struct rlimit vals;
	if (getrlimit(lim, &vals) < 0)
	{
	    /* best guess about error */
	    zwarnnam(nam, "can't read limit: %e", errno);
	    return 1;
	}
	if (hard)
	{
	    if (val > vals.rlim_max && geteuid()) {
		zwarnnam(nam, "can't raise hard limits");
		return 1;
	    }
	    vals.rlim_max = val;
	    /*
	     * not show if all systems will do this silently, but
	     * best be safe...
	     */
	    if (val < vals.rlim_cur)
		vals.rlim_cur = val;
	}
	if (soft || !hard) {
	    if (val > vals.rlim_max) {
		zwarnnam(nam, "limit exceeds hard limit");
		return 1;
	    }
	    else
		vals.rlim_cur = val;
	}
	if (!set)
	{
	    zwarnnam(nam,
		     "warning: unrecognised limit %d, use -s to set",
		     lim);
	    return 1;
	}
	else if (setrlimit(lim, &vals) < 0)
	{
	    zwarnnam(nam, "setrlimit failed: %e", errno);
	    return 1;
	}
    } else {
	/* new limit is valid and has been interpreted; apply it to the
	specified resource */
	if (hard) {
	    /* can only raise hard limits if running as root */
	    if (val > current_limits[lim].rlim_max && geteuid()) {
		zwarnnam(nam, "can't raise hard limits");
		return 1;
	    } else {
		limits[lim].rlim_max = val;
		if (val < limits[lim].rlim_cur)
		    limits[lim].rlim_cur = val;
	    }
	}
	if (soft || !hard) {
	    if (val > limits[lim].rlim_max) {
		/* no idea about this difference, don't intend to worry */
		if (*nam == 'u')
		{
		    /* ulimit does this */
		    if (val > current_limits[lim].rlim_max && geteuid()) {
			zwarnnam(nam, "value exceeds hard limit");
			return 1;
		    }
		    limits[lim].rlim_max = limits[lim].rlim_cur = val;
		} else {
		    /* but limit does this */
		    zwarnnam(nam, "limit exceeds hard limit");
		    return 1;
		}
	    } else
		limits[lim].rlim_cur = val;
	    if (set && zsetlimit(lim, nam))
		return 1;
	}
    }
    return 0;
}

/* limit: set or show resource limits.  The variable hard indicates *
 * whether `hard' or `soft' resource limits are being set/shown.    */

/**/
static int
bin_limit(char *nam, char **argv, Options ops, UNUSED(int func))
{
    char *s;
    int hard, limnum, lim;
    rlim_t val;
    int ret = 0;

    hard = OPT_ISSET(ops,'h');
    if (OPT_ISSET(ops,'s') && !*argv)
	return setlimits(NULL);
    /* without arguments, display limits */
    if (!*argv)
	return showlimits(nam, hard, -1);
    while ((s = *argv++)) {
	/* Search for the appropriate resource name.  When a name matches (i.e. *
	 * starts with) the argument, the lim variable changes from -1 to the   *
	 * number of the resource.  If another match is found, lim goes to -2.  */
	if (idigit(*s))
	{
	    lim = (int)zstrtol(s, NULL, 10);
	}
	else
	    for (lim = -1, limnum = 0; limnum < RLIM_NLIMITS; limnum++)
		if (!strncmp(resinfo[limnum]->name, s, strlen(s))) {
		    if (lim != -1)
			lim = -2;
		    else
			lim = limnum;
		}
	/* lim==-1 indicates that no matches were found.       *
	 * lim==-2 indicates that multiple matches were found. */
	if (lim < 0) {
	    zwarnnam(nam,
		     (lim == -2) ? "ambiguous resource specification: %s"
		     : "no such resource: %s", s);
	    return 1;
	}
	/* without value for limit, display the current limit */
	if (!(s = *argv++))
	    return showlimits(nam, hard, lim);
	if (lim >= RLIM_NLIMITS)
	{
	    val = zstrtorlimt(s, &s, 10);
	    if (*s)
	    {
		/* unknown limit, no idea how to scale */
		zwarnnam(nam, "unknown scaling factor: %s", s);
		return 1;
	    }
	}
	else if (resinfo[lim]->type == ZLIMTYPE_TIME) {
	    /* time-type resource -- may be specified as seconds, or minutes or *
	     * hours with the `m' and `h' modifiers, and `:' may be used to add *
	     * together more than one of these.  It's easier to understand from *
	     * the code:                                                        */
	    val = zstrtorlimt(s, &s, 10);
	    if (*s) {
		if ((*s == 'h' || *s == 'H') && !s[1])
		    val *= 3600L;
		else if ((*s == 'm' || *s == 'M') && !s[1])
		    val *= 60L;
		else if (*s == ':')
		    val = val * 60 + zstrtorlimt(s + 1, &s, 10);
		else {
		    zwarnnam(nam, "unknown scaling factor: %s", s);
		    return 1;
		}
	    }
	} else if (resinfo[lim]->type == ZLIMTYPE_NUMBER ||
		   resinfo[lim]->type == ZLIMTYPE_UNKNOWN ||
		   resinfo[lim]->type == ZLIMTYPE_MICROSECONDS) {
	    /* pure numeric resource -- only a straight decimal number is
	    permitted. */
	    char *t = s;
	    val = zstrtorlimt(t, &s, 10);
	    if (s == t) {
		zwarnnam(nam, "limit must be a number");
		return 1;
	    }
	} else {
	    /* memory-type resource -- `k', `M' and `G' modifiers are *
	     * permitted, meaning (respectively) 2^10, 2^20 and 2^30. */
	    val = zstrtorlimt(s, &s, 10);
	    if (!*s || ((*s == 'k' || *s == 'K') && !s[1])) {
		if (val != RLIM_INFINITY)
		    val *= 1024L;
	    } else if ((*s == 'M' || *s == 'm') && !s[1])
		val *= 1024L * 1024;
	    else if ((*s == 'G' || *s == 'g') && !s[1])
		val *= 1024L * 1024 * 1024;
	    else {
		zwarnnam(nam, "unknown scaling factor: %s", s);
		return 1;
	    }
	}
	if (do_limit(nam, lim, val, hard, !hard, OPT_ISSET(ops, 's')))
	    ret++;
    }
    return ret;
}

/**/
static int
do_unlimit(char *nam, int lim, int hard, int soft, int set, int euid)
{
    /* remove specified limit */
    if (lim >= RLIM_NLIMITS) {
	struct rlimit vals;
	if (getrlimit(lim, &vals) < 0)
	{
	    zwarnnam(nam, "can't read limit: %e", errno);
	    return 1;
	}
	if (hard) {
	    if (euid && vals.rlim_max != RLIM_INFINITY) {
		zwarnnam(nam, "can't remove hard limits");
		return 1;
	    } else
		vals.rlim_max = RLIM_INFINITY;
	}
	if (!hard || soft)
	    vals.rlim_cur = vals.rlim_max;
	if (!set) {
	    zwarnnam(nam,
		     "warning: unrecognised limit %d, use -s to set", lim);
	    return 1;
	} else if (setrlimit(lim, &vals) < 0) {
	    zwarnnam(nam, "setrlimit failed: %e", errno);
	    return 1;
	}
    } else {
	if (hard) {
	    if (euid && current_limits[lim].rlim_max != RLIM_INFINITY) {
		zwarnnam(nam, "can't remove hard limits");
		return 1;
	    } else
		limits[lim].rlim_max = RLIM_INFINITY;
	}
	if (!hard || soft)
	    limits[lim].rlim_cur = limits[lim].rlim_max;
	if (set && zsetlimit(lim, nam))
	    return 1;
    }
    return 0;
}

/* unlimit: remove resource limits.  Much of this code is the same as *
 * that in bin_limit().                                               */

/**/
static int
bin_unlimit(char *nam, char **argv, Options ops, UNUSED(int func))
{
    int hard, limnum, lim;
    int ret = 0;
    uid_t euid = geteuid();

    hard = OPT_ISSET(ops,'h');
    /* Without arguments, remove all limits. */
    if (!*argv) {
	for (limnum = 0; limnum != RLIM_NLIMITS; limnum++) {
	    if (hard) {
		if (euid && current_limits[limnum].rlim_max != RLIM_INFINITY)
		    ret++;
		else
		    limits[limnum].rlim_max = RLIM_INFINITY;
	    } else
		limits[limnum].rlim_cur = limits[limnum].rlim_max;
	}
	if (OPT_ISSET(ops,'s'))
	    ret += setlimits(nam);
	if (ret)
	    zwarnnam(nam, "can't remove hard limits");
    } else {
	for (; *argv; argv++) {
	    /* Search for the appropriate resource name.  When a name     *
	     * matches (i.e. starts with) the argument, the lim variable  *
	     * changes from -1 to the number of the resource.  If another *
	     * match is found, lim goes to -2.                            */
	    if (idigit(**argv)) {
		lim = (int)zstrtol(*argv, NULL, 10);
	    } else {
		for (lim = -1, limnum = 0; limnum < RLIM_NLIMITS; limnum++)
		    if (!strncmp(resinfo[limnum]->name, *argv, strlen(*argv))) {
			if (lim != -1)
			    lim = -2;
			else
			    lim = limnum;
		    }
	    }
	    /* lim==-1 indicates that no matches were found.       *
	     * lim==-2 indicates that multiple matches were found. */
	    if (lim < 0) {
		zwarnnam(nam,
			 (lim == -2) ? "ambiguous resource specification: %s"
			 : "no such resource: %s", *argv);
		return 1;
	    }
	    else if (do_unlimit(nam, lim, hard, !hard, OPT_ISSET(ops, 's'),
				euid))
		ret++;
	}
    }
    return ret;
}

/* ulimit: set or display resource limits */

/**/
static int
bin_ulimit(char *name, char **argv, UNUSED(Options ops), UNUSED(int func))
{
    int res, resmask = 0, hard = 0, soft = 0, nres = 0, all = 0, ret = 0;
    char *options, *eptr, *number;

    do {
	options = *argv;
	if (options && *options == '-' && !options[1]) {
	    zwarnnam(name, "missing option letter");
	    return 1;
	}
	res = -1;
	if (options && *options == '-') {
	    argv++;
	    while (*++options) {
		if(*options == Meta)
		    *++options ^= 32;
		res = -1;
		switch (*options) {
		case 'H':
		    hard = 1;
		    continue;
		case 'S':
		    soft = 1;
		    continue;
		case 'N':
		    if (options[1]) {
			number = options + 1;
		    } else if (*argv) {
			number = *argv++;
		    } else {
			zwarnnam(name, "number required after -N");
			return 1;
		    }
		    res = (int)zstrtol(number, &eptr, 10);
		    if (*eptr) {
			zwarnnam(name, "invalid number: %s", number);
			return 1;
		    }
		    /*
		     * fake it so it looks like we just finished an option...
		     */
		    while (options[1])
			options++;
		    break;
		case 'a':
		    if (resmask) {
			zwarnnam(name, "no limits allowed with -a");
			return 1;
		    }
		    all = 1;
		    resmask = (1 << RLIM_NLIMITS) - 1;
		    nres = RLIM_NLIMITS;
		    continue;
		default:
		    res = find_resource(*options);
		    if (res < 0) {
			/* unrecognised limit */
			zwarnnam(name, "bad option: -%c", *options);
			return 1;
		    }
		    break;
		}
		if (options[1]) {
		    resmask |= 1 << res;
		    nres++;
		}
		if (all && res != -1) {
		    zwarnnam(name, "no limits allowed with -a");
		    return 1;
		}
	    }
	}
	if (!*argv || **argv == '-') {
	    if (res < 0) {
		if (*argv || nres)
		    continue;
		else
		    res = RLIMIT_FSIZE;
	    }
	    resmask |= 1 << res;
	    nres++;
	    continue;
	}
	if (all) {
	    zwarnnam(name, "no arguments allowed after -a");
	    return 1;
	}
	if (res < 0)
	    res = RLIMIT_FSIZE;
	if (strcmp(*argv, "unlimited")) {
	    /* set limit to specified value */
	    rlim_t limit;

	    if (!strcmp(*argv, "hard")) {
		struct rlimit vals;

		if (getrlimit(res, &vals) < 0)
		{
		    zwarnnam(name, "can't read limit: %e", errno);
		    return 1;
		}
		else
		{
		    limit = vals.rlim_max;
		}
	    } else {
		limit = zstrtorlimt(*argv, &eptr, 10);
		if (*eptr) {
		    zwarnnam(name, "invalid number: %s", *argv);
		    return 1;
		}
		/* scale appropriately */
		if (res < RLIM_NLIMITS)
		    limit *= resinfo[res]->unit;
	    }
	    if (do_limit(name, res, limit, hard, soft, 1))
		ret++;
	} else {
	    if (do_unlimit(name, res, hard, soft, 1, geteuid()))
		ret++;
	}
	argv++;
    } while (*argv);
    for (res = 0; resmask; res++, resmask >>= 1)
	if ((resmask & 1) && printulimit(name, res, hard, nres > 1))
	    ret++;
    return ret;
}

#else /* !HAVE_GETRLIMIT || !RLIM_INFINITY */

# define bin_limit   bin_notavail
# define bin_ulimit  bin_notavail
# define bin_unlimit bin_notavail

#endif /* !HAVE_GETRLIMIT || !RLIM_INFINITY */

static struct builtin bintab[] = {
    BUILTIN("limit",   0, bin_limit,   0, -1, 0, "sh", NULL),
    BUILTIN("ulimit",  0, bin_ulimit,  0, -1, 0, NULL, NULL),
    BUILTIN("unlimit", 0, bin_unlimit, 0, -1, 0, "hs", NULL),
};

static struct features module_features = {
    bintab, sizeof(bintab)/sizeof(*bintab),
    NULL, 0,
    NULL, 0,
    NULL, 0,
    0
};

/**/
int
setup_(UNUSED(Module m))
{
    return 0;
}

/**/
int
features_(Module m, char ***features)
{
    *features = featuresarray(m, &module_features);
    return 0;
}

/**/
int
enables_(Module m, int **enables)
{
    return handlefeatures(m, &module_features, enables);
}

/**/
int
boot_(UNUSED(Module m))
{
    set_resinfo();
    return 0;
}

/**/
int
cleanup_(Module m)
{
    free_resinfo();
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    return 0;
}
