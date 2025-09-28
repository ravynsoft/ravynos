/*
 * sched.c - execute commands at scheduled times
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

#include "sched.mdh"
#include "sched.pro"

/* node in sched list */

typedef struct schedcmd  *Schedcmd;

/* Flags for each scheduled event */
enum schedflags {
    /* Trash zle if necessary when event is activated */
    SCHEDFLAG_TRASH_ZLE = 1
};

struct schedcmd {
    struct schedcmd *next;
    char *cmd;			/* command to run */
    time_t time;		/* when to run it */
    int flags;			/* flags as above */
};

/* the list of sched jobs pending */

static struct schedcmd *schedcmds;

/* flag that timed event is running (via addtimedfn())*/
static int schedcmdtimed;

/* Use addtimedfn() to add a timed event for sched's use */

/**/
static void
schedaddtimed(void)
{
    /*
     * The following code shouldn't be necessary and indicates
     * a bug.  However, the DPUTS() in the caller should pick
     * this up so we can detect and fix it, and the following
     * Makes The World Safe For Timed Events in non-debugging shells.
     */
    if (schedcmdtimed)
	scheddeltimed();
    schedcmdtimed = 1;
    addtimedfn(checksched, schedcmds->time);
}

/* Use deltimedfn() to remove the sched timed event */

/**/
static void
scheddeltimed(void)
{
    if (schedcmdtimed)
    {
	deltimedfn(checksched);
	schedcmdtimed = 0;
    }
}


/* Check scheduled commands; call this function from time to time. */

/**/
static void
checksched(void)
{
    time_t t;
    struct schedcmd *sch;

    if(!schedcmds)
	return;
    t = time(NULL);
    /*
     * List is ordered, so we only need to consider the
     * head element.
     */
    while (schedcmds && schedcmds->time <= t) {
	/*
	 * Remove the entry to be executed from the list
	 * before execution:  this makes quite sure that
	 * the entry hasn't been monkeyed with when we
	 * free it.
	 */
	sch = schedcmds;
	schedcmds = sch->next;
	/*
	 * Delete from the timed function list now in case
	 * the called code reschedules.
	 */
	scheddeltimed();

	if ((sch->flags & SCHEDFLAG_TRASH_ZLE) && zleactive)
	    zleentry(ZLE_CMD_TRASH);
	execstring(sch->cmd, 0, 0, "sched");
	zsfree(sch->cmd);
	zfree(sch, sizeof(struct schedcmd));

	/*
	 * Fix time for future events.
	 * I had this outside the loop, for a little extra efficiency.
	 * However, it then occurred to me that having the list of
	 * forthcoming entries up to date could be regarded as
	 * a feature, and the inefficiency is negligible.
	 *
	 * Careful in case the code we called has already set
	 * up a timed event; if it has, that'll be up to date since
	 * we haven't changed the list here.
	 */
	if (schedcmds && !schedcmdtimed) {
	    /*
	     * We've already delete the function from the list.
	     */
	    DPUTS(timedfns && firstnode(timedfns),
		  "BUG: already timed fn (1)");
	    schedaddtimed();
	}
    }
}

/**/
static int
bin_sched(char *nam, char **argv, UNUSED(Options ops), UNUSED(int func))
{
    char *s, **argptr;
    time_t t;
    long h, m, sec;
    struct tm *tm;
    struct schedcmd *sch, *sch2, *schl;
    int sn, flags = 0;

    /* If the argument begins with a -, remove the specified item from the
    schedule. */
    for (argptr = argv; *argptr && **argptr == '-'; argptr++) {
	char *arg = *argptr + 1;
	if (idigit(*arg)) {
	    sn = atoi(arg);

	    if (!sn) {
		zwarnnam("sched", "usage for delete: sched -<item#>.");
		return 1;
	    }
	    for (schl = NULL, sch = schedcmds, sn--;
		 sch && sn; sch = (schl = sch)->next, sn--);
	    if (!sch) {
		zwarnnam("sched", "not that many entries");
		return 1;
	    }
	    if (schl)
		schl->next = sch->next;
	    else {
		scheddeltimed();
		schedcmds = sch->next;
		if (schedcmds) {
		    DPUTS(timedfns && firstnode(timedfns), "BUG: already timed fn (2)");
		    schedaddtimed();
		}
	    }
	    zsfree(sch->cmd);
	    zfree(sch, sizeof(struct schedcmd));

	    return 0;
	} else if (*arg == '-') {
	    /* end of options */
	    argptr++;
	    break;
	} else if (!strcmp(arg, "o")) {
	    flags |= SCHEDFLAG_TRASH_ZLE;
	} else {
	    if (*arg)
		zwarnnam(nam, "bad option: -%c", *arg);
	    else
		zwarnnam(nam, "option expected");
	    return 1;
	}
    }

    /* given no arguments, display the schedule list */
    if (!*argptr) {
	for (sn = 1, sch = schedcmds; sch; sch = sch->next, sn++) {
	    char tbuf[60], *flagstr, *endstr;
	    time_t t;
	    struct tm *tmp;

	    t = sch->time;
	    tmp = localtime(&t);
	    ztrftime(tbuf, 40, "%a %b %e %k:%M:%S", tmp, 0L);
	    if (sch->flags & SCHEDFLAG_TRASH_ZLE)
		flagstr = "-o ";
	    else
		flagstr = "";
	    if (*sch->cmd == '-')
		endstr = "-- ";
	    else
		endstr = "";
	    printf("%3d %s %s%s%s\n", sn, tbuf, flagstr, endstr,
		   unmeta(sch->cmd));
	}
	return 0;
    } else if (!argptr[1]) {
	/* other than the two cases above, sched *
	 *requires at least two arguments        */
	zwarnnam("sched", "not enough arguments");
	return 1;
    }

    /* The first argument specifies the time to schedule the command for.  The
    remaining arguments form the command. */
    s = *argptr++;
    if (*s == '+') {
	/*
	 * + introduces a relative time.  The rest of the argument may be an
	 * hour:minute offset from the current time.  Once the hour and minute
	 * numbers have been extracted, and the format verified, the resulting
	 * offset is simply added to the current time.
	 */
	zlong zl = zstrtol(s + 1, &s, 10);
	if (*s == ':') {
	    m = (long)zstrtol(s + 1, &s, 10);
	    if (*s == ':')
		sec = (long)zstrtol(s + 1, &s, 10);
	    else
		sec = 0;
	    if (*s) {
		zwarnnam("sched", "bad time specifier");
		return 1;
	    }
	    t = time(NULL) + (long)zl * 3600 + m * 60 + sec;
	} else if (!*s) {
	    /*
	     * Alternatively, it may simply be a number of seconds.
	     * This is here for consistency with absolute times.
	     */
	    t = time(NULL) + (time_t)zl;
	} else {
	    zwarnnam("sched", "bad time specifier");
	    return 1;
	}
    } else {
	/*
	 * If there is no +, an absolute time must have been given.
	 * This may be in hour:minute format, optionally followed by a string
	 * starting with `a' or `p' (for a.m. or p.m.).  Characters after the
	 * `a' or `p' are ignored.
	 */
	zlong zl = zstrtol(s, &s, 10);
	if (*s == ':') {
	    h = (long)zl;
	    m = (long)zstrtol(s + 1, &s, 10);
	    if (*s == ':')
		sec = (long)zstrtol(s + 1, &s, 10);
	    else
		sec = 0;
	    if (*s && *s != 'a' && *s != 'A' && *s != 'p' && *s != 'P') {
		zwarnnam("sched", "bad time specifier");
		return 1;
	    }
	    t = time(NULL);
	    tm = localtime(&t);
	    t -= tm->tm_sec + tm->tm_min * 60 + tm->tm_hour * 3600;
	    if (*s == 'p' || *s == 'P')
		h += 12;
	    t += h * 3600 + m * 60 + sec;
	    /*
	     * If the specified time is before the current time, it must refer
	     * to tomorrow.
	     */
	    if (t < time(NULL))
		t += 3600 * 24;
	} else if (!*s) {
	    /*
	     * Otherwise, it must be a raw time specifier.
	     */
	    t = (long)zl;
	} else {
	    zwarnnam("sched", "bad time specifier");
	    return 1;
	}
    }
    /* The time has been calculated; now add the new entry to the linked list
    of scheduled commands. */
    sch = (struct schedcmd *) zalloc(sizeof *sch);
    sch->time = t;
    sch->cmd = zjoin(argptr, ' ', 0);
    sch->flags = flags;
    /* Insert into list in time order */
    if (schedcmds) {
	if (sch->time < schedcmds->time) {
	    scheddeltimed();
	    sch->next = schedcmds;
	    schedcmds = sch;
	    DPUTS(timedfns && firstnode(timedfns), "BUG: already timed fn (3)");
	    schedaddtimed();
	} else {
	    for (sch2 = schedcmds;
		 sch2->next && sch2->next->time < sch->time;
		 sch2 = sch2->next)
		;
	    sch->next = sch2->next;
	    sch2->next = sch;
	}
    } else {
	sch->next = NULL;
	schedcmds = sch;
	DPUTS(timedfns && firstnode(timedfns), "BUG: already timed fn (4)");
	schedaddtimed();
    }
    return 0;
}


/**/
static char **
schedgetfn(UNUSED(Param pm))
{
    int i;
    struct schedcmd *sch;
    char **ret, **aptr;

    for (i = 0, sch = schedcmds; sch; sch = sch->next, i++)
	;

    aptr = ret = zhalloc(sizeof(char *) * (i+1));
    for (sch = schedcmds; sch; sch = sch->next, aptr++) {
	char tbuf[40], *flagstr;
	time_t t;

	t = sch->time;
#if defined(PRINTF_HAS_LLD)
	sprintf(tbuf, "%lld", (long long)t);
#else
	sprintf(tbuf, "%ld", (long)t);
#endif
	if (sch->flags & SCHEDFLAG_TRASH_ZLE)
	    flagstr = "-o";
	else
	    flagstr = "";
	*aptr = (char *)zhalloc(5 + strlen(tbuf) + strlen(sch->cmd));
	sprintf(*aptr, "%s:%s:%s", tbuf, flagstr, sch->cmd);
    }
    *aptr = NULL;

    return ret;
}


static struct builtin bintab[] = {
    BUILTIN("sched", 0, bin_sched, 0, -1, 0, NULL, NULL),
};

static const struct gsu_array sched_gsu =
{ schedgetfn, arrsetfn, stdunsetfn };

static struct paramdef partab[] = {
    SPECIALPMDEF("zsh_scheduled_events", PM_ARRAY|PM_READONLY,
		 &sched_gsu, NULL, NULL)
};

static struct features module_features = {
    bintab, sizeof(bintab)/sizeof(*bintab),
    NULL, 0,
    NULL, 0,
    partab, sizeof(partab)/sizeof(*partab),
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
    addprepromptfn(&checksched);
    return 0;
}

/**/
int
cleanup_(Module m)
{
    struct schedcmd *sch, *schn;

    if (schedcmds)
	scheddeltimed();
    for (sch = schedcmds; sch; sch = schn) {
	schn = sch->next;
	zsfree(sch->cmd);
	zfree(sch, sizeof(*sch));
    }
    delprepromptfn(&checksched);
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    return 0;
}
