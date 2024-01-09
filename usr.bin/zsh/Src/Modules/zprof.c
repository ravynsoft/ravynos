/*
 * zprof.c - a shell function profiling module for zsh
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1996-1997 Sven Wischnowsky
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Sven Wischnowsky or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Sven Wischnowsky and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Sven Wischnowsky and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Sven Wischnowsky and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "zprof.mdh"
#include "zprof.pro"

#include <sys/time.h>
#include <unistd.h>

typedef struct pfunc *Pfunc;

struct pfunc {
    Pfunc next;
    char *name;
    long calls;
    double time;
    double self;
    long num;
};

typedef struct sfunc *Sfunc;

struct sfunc {
    Pfunc p;
    Sfunc prev;
    double beg;
};

typedef struct parc *Parc;

struct parc {
    Parc next;
    Pfunc from;
    Pfunc to;
    long calls;
    double time;
    double self;
};

static Pfunc calls;
static int ncalls;
static Parc arcs;
static int narcs;
static Sfunc stack;
static Module zprof_module;

static void
freepfuncs(Pfunc f)
{
    Pfunc n;

    for (; f; f = n) {
	n = f->next;
	zsfree(f->name);
	zfree(f, sizeof(*f));
    }
}

static void
freeparcs(Parc a)
{
    Parc n;

    for (; a; a = n) {
	n = a->next;
	zfree(a, sizeof(*a));
    }
}

static Pfunc
findpfunc(char *name)
{
    Pfunc f;

    for (f = calls; f; f = f->next)
	if (!strcmp(name, f->name))
	    return f;

    return NULL;
}

static Parc
findparc(Pfunc f, Pfunc t)
{
    Parc a;

    for (a = arcs; a; a = a->next)
	if (a->from == f && a->to == t)
	    return a;

    return NULL;
}

static int
cmpsfuncs(Pfunc *a, Pfunc *b)
{
    return ((*a)->self > (*b)->self ? -1 : ((*a)->self != (*b)->self));
}

static int
cmptfuncs(Pfunc *a, Pfunc *b)
{
    return ((*a)->time > (*b)->time ? -1 : ((*a)->time != (*b)->time));
}

static int
cmpparcs(Parc *a, Parc *b)
{
    return ((*a)->time > (*b)->time ? -1 : ((*a)->time != (*b)->time));
}

static int
bin_zprof(UNUSED(char *nam), UNUSED(char **args), Options ops, UNUSED(int func))
{
    if (OPT_ISSET(ops,'c')) {
	freepfuncs(calls);
	calls = NULL;
	ncalls = 0;
	freeparcs(arcs);
	arcs = NULL;
	narcs = 0;
    } else {
	VARARR(Pfunc, fs, (ncalls + 1));
	Pfunc f, *fp;
	VARARR(Parc, as, (narcs + 1));
	Parc a, *ap;
	long i;
	double total;

	for (total = 0.0, f = calls, fp = fs; f; f = f->next, fp++) {
	    *fp = f;
	    total += f->self;
	}
	*fp = NULL;
	for (a = arcs, ap = as; a; a = a->next, ap++)
	    *ap = a;
	*ap = NULL;

	qsort(fs, ncalls, sizeof(f),
	      (int (*) _((const void *, const void *))) cmpsfuncs);
	qsort(as, narcs, sizeof(a),
	      (int (*) _((const void *, const void *))) cmpparcs);

	printf("num  calls                time                       self            name\n-----------------------------------------------------------------------------------\n");
	for (fp = fs, i = 1; *fp; fp++, i++) {
	    printf("%2ld) %4ld       %8.2f %8.2f  %6.2f%%  %8.2f %8.2f  %6.2f%%  %s\n",
		   ((*fp)->num = i),
		   (*fp)->calls,
		   (*fp)->time, (*fp)->time / ((double) (*fp)->calls),
		   ((*fp)->time / total) * 100.0,
		   (*fp)->self, (*fp)->self / ((double) (*fp)->calls),
		   ((*fp)->self / total) * 100.0,
		   (*fp)->name);
	}
	qsort(fs, ncalls, sizeof(f),
	      (int (*) _((const void *, const void *))) cmptfuncs);

	for (fp = fs; *fp; fp++) {
	    printf("\n-----------------------------------------------------------------------------------\n\n");
	    for (ap = as; *ap; ap++)
		if ((*ap)->to == *fp) {
		    printf("    %4ld/%-4ld  %8.2f %8.2f  %6.2f%%  %8.2f %8.2f             %s [%ld]\n",
			   (*ap)->calls, (*fp)->calls,
			   (*ap)->time, (*ap)->time / ((double) (*ap)->calls),
			   ((*ap)->time / total) * 100.0,
			   (*ap)->self, (*ap)->self / ((double) (*ap)->calls),
			   (*ap)->from->name, (*ap)->from->num);
		}
	    printf("%2ld) %4ld       %8.2f %8.2f  %6.2f%%  %8.2f %8.2f  %6.2f%%  %s\n",
		   (*fp)->num, (*fp)->calls,
		   (*fp)->time, (*fp)->time / ((double) (*fp)->calls),
		   ((*fp)->time / total) * 100.0,
		   (*fp)->self, (*fp)->self / ((double) (*fp)->calls),
		   ((*fp)->self / total) * 100.0,
		   (*fp)->name);
	    for (ap = as + narcs - 1; ap >= as; ap--)
		if ((*ap)->from == *fp) {
		    printf("    %4ld/%-4ld  %8.2f %8.2f  %6.2f%%  %8.2f %8.2f             %s [%ld]\n",
			   (*ap)->calls, (*ap)->to->calls,
			   (*ap)->time, (*ap)->time / ((double) (*ap)->calls),
			   ((*ap)->time / total) * 100.0,
			   (*ap)->self, (*ap)->self / ((double) (*ap)->calls),
			   (*ap)->to->name, (*ap)->to->num);
		}
	}
    }
    return 0;
}

static char *
name_for_anonymous_function(char *name)
{
    char lineno[DIGBUFSIZE];
    char *parts[7];

    convbase(lineno, funcstack[0].flineno, 10);

    parts[0] = name;
    parts[1] = " [";
    parts[2] = funcstack[0].filename ? funcstack[0].filename : "";
    parts[3] = ":";
    parts[4] = lineno;
    parts[5] = "]";
    parts[6] = NULL;

    return sepjoin(parts, "", 1);
}

static int
zprof_wrapper(Eprog prog, FuncWrap w, char *name)
{
    int active = 0;
    struct sfunc sf, *sp;
    Pfunc f = NULL;
    Parc a = NULL;
    struct timeval tv;
    struct timezone dummy;
    double prev = 0, now;
    char *name_for_lookups;

    if (is_anonymous_function_name(name)) {
        name_for_lookups = name_for_anonymous_function(name);
    } else {
        name_for_lookups = name;
    }

    if (zprof_module && !(zprof_module->node.flags & MOD_UNLOAD)) {
        active = 1;
        if (!(f = findpfunc(name_for_lookups))) {
            f = (Pfunc) zalloc(sizeof(*f));
            f->name = ztrdup(name_for_lookups);
            f->calls = 0;
            f->time = f->self = 0.0;
            f->next = calls;
            calls = f;
            ncalls++;
        }
        if (stack) {
            if (!(a = findparc(stack->p, f))) {
                a = (Parc) zalloc(sizeof(*a));
                a->from = stack->p;
                a->to = f;
                a->calls = 0;
                a->time = a->self = 0.0;
                a->next = arcs;
                arcs = a;
                narcs++;
            }
        }
        sf.prev = stack;
        sf.p = f;
        stack = &sf;

        f->calls++;
        tv.tv_sec = tv.tv_usec = 0;
        gettimeofday(&tv, &dummy);
        sf.beg = prev = ((((double) tv.tv_sec) * 1000.0) +
                         (((double) tv.tv_usec) / 1000.0));
    }
    runshfunc(prog, w, name);
    if (active) {
        if (zprof_module && !(zprof_module->node.flags & MOD_UNLOAD)) {
            tv.tv_sec = tv.tv_usec = 0;
            gettimeofday(&tv, &dummy);

            now = ((((double) tv.tv_sec) * 1000.0) +
                   (((double) tv.tv_usec) / 1000.0));
            f->self += now - sf.beg;
            for (sp = sf.prev; sp && sp->p != f; sp = sp->prev);
            if (!sp)
                f->time += now - prev;
            if (a) {
                a->calls++;
                a->self += now - sf.beg;
            }
            stack = sf.prev;

            if (stack) {
                stack->beg += now - prev;
                if (a)
                    a->time += now - prev;
            }
        } else
            stack = sf.prev;
    }
    return 0;
}

static struct builtin bintab[] = {
    BUILTIN("zprof", 0, bin_zprof, 0, 0, 0, "c", NULL),
};

static struct funcwrap wrapper[] = {
    WRAPDEF(zprof_wrapper),
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
setup_(Module m)
{
    zprof_module = m;
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
boot_(Module m)
{
    calls = NULL;
    ncalls = 0;
    arcs = NULL;
    narcs = 0;
    stack = NULL;
    return addwrapper(m, wrapper);
}

/**/
int
cleanup_(Module m)
{
    freepfuncs(calls);
    freeparcs(arcs);
    deletewrapper(m, wrapper);
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    return 0;
}
