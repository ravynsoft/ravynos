/*
 * termcap.c - termcap manipulation through curses
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

/*
 * We need to include the zsh headers later to avoid clashes with
 * the definitions on some systems, however we need the configuration
 * file to decide whether we should avoid curses.h, which clashes
 * with several zsh constants on some systems (e.g. SunOS 4).
 */
#include "../../config.h"

#include "termcap.mdh"
#include "termcap.pro"

/**/
#ifdef HAVE_TGETENT

#ifndef HAVE_BOOLCODES
static char *boolcodes[] = {
    "bw", "am", "ut", "cc", "xs", "YA", "YF", "YB", "xt", "xn", "eo",
    "gn", "hc", "HC", "km", "YC", "hs", "hl", "in", "YG", "da", "db",
    "mi", "ms", "nx", "xb", "NP", "ND", "NR", "os", "5i", "YD", "YE",
    "es", "hz", "ul", "xo", NULL};
#endif

/**/
static int
ztgetflag(char *s)
{
    char **b;

    /* ncurses can tell if an existing boolean capability is *
     * off, but other curses variants can't, so we fudge it. *
     * This feature of ncurses appears to have gone away as  *
     * of NCURSES_MAJOR_VERSION == 5, so don't rely on it.   */
    switch (tgetflag(s)) {
    case -1:
	break;
    case 0:
	for (b = (char **)boolcodes; *b; ++b)
	    if (s[0] == (*b)[0] && s[1] == (*b)[1])
		return 0;
	break;
    default:
	return 1;
    }
    return -1;
}

/* echotc: output a termcap */

/**/
static int
bin_echotc(char *name, char **argv, UNUSED(Options ops), UNUSED(int func))
{
    char *s, buf[2048], *t, *u;
    int num, argct;

    s = *argv++;
    if (termflags & TERM_BAD)
	return 1;
    if ((termflags & TERM_UNKNOWN) && (isset(INTERACTIVE) || !init_term()))
	return 1;
    /* if the specified termcap has a numeric value, display it */
    if ((num = tgetnum(s)) != -1) {
	printf("%d\n", num);
	return 0;
    }
    /* if the specified termcap is boolean, and set, say so  */
    switch (ztgetflag(s)) {
    case -1:
	break;
    case 0:
	puts("no");
	return 0;
    default:
	puts("yes");
	return 0;
    }
    /* get a string-type capability */
    u = buf;
    t = tgetstr(s, &u);
    if (t == (char *)-1 || !t || !*t) {
	/* capability doesn't exist, or (if boolean) is off */
	zwarnnam(name, "no such capability: %s", s);
	return 1;
    }
    /* count the number of arguments required */
    for (argct = 0, u = t; *u; u++)
	if (*u == '%') {
	    if (u++, (*u == 'd' || *u == '2' || *u == '3' || *u == '.' ||
		      *u == '+'))
		argct++;
	}
    /* check that the number of arguments provided is correct */
    if (arrlen(argv) != argct) {
	zwarnnam(name, (arrlen(argv) < argct) ? "not enough arguments" :
		 "too many arguments");
	return 1;
    }
    /* output string, through the proper termcap functions */
    if (!argct)
	tputs(t, 1, putraw);
    else {
	/* This assumes arguments of <lines> <columns> for cap 'cm' */
	num = (argv[1]) ? atoi(argv[1]) : atoi(*argv);
	tputs(tgoto(t, num, atoi(*argv)), 1, putraw);
    }
    return 0;
}

static struct builtin bintab[] = {
    BUILTIN("echotc", 0, bin_echotc, 1, -1, 0, NULL, NULL),
};

/**/
static HashNode
gettermcap(UNUSED(HashTable ht), const char *name)
{
    int len, num;
    char *tcstr, buf[2048], *u, *nameu;
    Param pm = NULL;

    /* This depends on the termcap stuff in init.c */
    if (termflags & TERM_BAD)
	return NULL;
    if ((termflags & TERM_UNKNOWN) && (isset(INTERACTIVE) || !init_term()))
	return NULL;

    
    nameu = dupstring(name);
    unmetafy(nameu, &len);

    pm = (Param) hcalloc(sizeof(struct param));
    pm->node.nam = nameu;
    pm->node.flags = PM_READONLY;
    u = buf;

    /* logic in the following cascade copied from echotc, above */

    if ((num = tgetnum(nameu)) != -1) {
	pm->gsu.i = &nullsetinteger_gsu;
	pm->u.val = num;
	pm->node.flags |= PM_INTEGER;
	return &pm->node;
    }

    pm->gsu.s = &nullsetscalar_gsu;
    switch (ztgetflag(nameu)) {
    case -1:
	break;
    case 0:
	pm->u.str = dupstring("no");
	pm->node.flags |= PM_SCALAR;
	return &pm->node;
    default:
	pm->u.str = dupstring("yes");
	pm->node.flags |= PM_SCALAR;
	return &pm->node;
    }
    if ((tcstr = tgetstr(nameu, &u)) != NULL && tcstr != (char *)-1) {
	pm->u.str = dupstring(tcstr);
	pm->node.flags |= PM_SCALAR;
    } else {
	/* zwarn("no such capability: %s", name); */
	pm->u.str = dupstring("");
	pm->node.flags |= PM_UNSET;
    }
    return &pm->node;
}

/**/
static void
scantermcap(UNUSED(HashTable ht), ScanFunc func, int flags)
{
    Param pm = NULL;
    int num;
    char **capcode, *tcstr, buf[2048], *u;

#ifndef HAVE_NUMCODES
    static char *numcodes[] = {
	"co", "it", "lh", "lw", "li", "lm", "sg", "ma", "Co", "pa", "MW",
	"NC", "Nl", "pb", "vt", "ws", "Yo", "Yp", "Ya", "BT", "Yc", "Yb",
	"Yd", "Ye", "Yf", "Yg", "Yh", "Yi", "Yk", "Yj", "Yl", "Ym", "Yn",
	NULL};
#endif

#ifndef HAVE_STRCODES
    static char *zstrcodes[] = {
	"ac", "bt", "bl", "cr", "ZA", "ZB", "ZC", "ZD", "cs", "rP", "ct",
	"MC", "cl", "cb", "ce", "cd", "ch", "CC", "CW", "cm", "do", "ho",
	"vi", "le", "CM", "ve", "nd", "ll", "up", "vs", "ZE", "dc", "dl",
	"DI", "ds", "DK", "hd", "eA", "as", "SA", "mb", "md", "ti", "dm",
	"mh", "ZF", "ZG", "im", "ZH", "ZI", "ZJ", "ZK", "ZL", "mp", "mr",
	"mk", "ZM", "so", "ZN", "ZO", "us", "ZP", "SX", "ec", "ae", "RA",
	"me", "te", "ed", "ZQ", "ei", "ZR", "ZS", "ZT", "ZU", "se", "ZV",
	"ZW", "ue", "ZX", "RX", "PA", "fh", "vb", "ff", "fs", "WG", "HU",
	"i1", "is", "i3", "if", "iP", "Ic", "Ip", "ic", "al", "ip", "K1",
	"K3", "K2", "kb", "@1", "kB", "K4", "K5", "@2", "ka", "kC", "@3",
	"@4", "@5", "@6", "kt", "kD", "kL", "kd", "kM", "@7", "@8", "kE",
	"kS", "@9", "k0", "k1", "k;", "F1", "F2", "F3", "F4", "F5", "F6",
	"F7", "F8", "F9", "k2", "FA", "FB", "FC", "FD", "FE", "FF", "FG",
	"FH", "FI", "FJ", "k3", "FK", "FL", "FM", "FN", "FO", "FP", "FQ",
	"FR", "FS", "FT", "k4", "FU", "FV", "FW", "FX", "FY", "FZ", "Fa",
	"Fb", "Fc", "Fd", "k5", "Fe", "Ff", "Fg", "Fh", "Fi", "Fj", "Fk",
	"Fl", "Fm", "Fn", "k6", "Fo", "Fp", "Fq", "Fr", "k7", "k8", "k9",
	"@0", "%1", "kh", "kI", "kA", "kl", "kH", "%2", "%3", "%4", "%5",
	"kN", "%6", "%7", "kP", "%8", "%9", "%0", "&1", "&2", "&3", "&4",
	"&5", "kr", "&6", "&9", "&0", "*1", "*2", "*3", "*4", "*5", "*6",
	"*7", "*8", "*9", "kF", "*0", "#1", "#2", "#3", "#4", "%a", "%b",
	"%c", "%d", "%e", "%f", "kR", "%g", "%h", "%i", "%j", "!1", "!2",
	"kT", "!3", "&7", "&8", "ku", "ke", "ks", "l0", "l1", "la", "l2",
	"l3", "l4", "l5", "l6", "l7", "l8", "l9", "Lf", "LF", "LO", "mo",
	"mm", "ZY", "ZZ", "Za", "Zb", "Zc", "Zd", "nw", "Ze", "oc", "op",
	"pc", "DC", "DL", "DO", "Zf", "IC", "SF", "AL", "LE", "Zg", "RI",
	"Zh", "SR", "UP", "Zi", "pk", "pl", "px", "pn", "ps", "pO", "pf",
	"po", "PU", "QD", "RC", "rp", "RF", "r1", "r2", "r3", "rf", "rc",
	"cv", "sc", "sf", "sr", "Zj", "sa", "Sb", "Zk", "Zl", "SC", "sp",
	"Sf", "ML", "Zm", "MR", "Zn", "st", "Zo", "Zp", "wi", "Zq", "Zr",
	"Zs", "Zt", "Zu", "Zv", "ta", "Zw", "ts", "TO", "uc", "hu", "u0",
	"u1", "u2", "u3", "u4", "u5", "u6", "u7", "u8", "u9", "WA", "XF",
	"XN", "Zx", "S8", "Yv", "Zz", "Xy", "Zy", "ci", "Yw", "Yx", "dv",
	"S1", "Yy", "S2", "S4", "S3", "S5", "Gm", "Km", "Mi", "S6", "xl",
	"RQ", "S7", "s0", "s1", "s2", "s3", "AB", "AF", "Yz", "ML", "YZ",
	"MT", "Xh", "Xl", "Xo", "Xr", "Xt", "Xv", "sA", "sL", NULL};
#endif

    pm = (Param) hcalloc(sizeof(struct param));
    u = buf;

    pm->node.flags = PM_READONLY | PM_SCALAR;
    pm->gsu.s = &nullsetscalar_gsu;

    for (capcode = (char **)boolcodes; *capcode; capcode++) {
	if ((num = ztgetflag(*capcode)) != -1) {
	    pm->u.str = num ? dupstring("yes") : dupstring("no");
	    pm->node.nam = dupstring(*capcode);
	    func(&pm->node, flags);
	}
    }

    pm->node.flags = PM_READONLY | PM_INTEGER;
    pm->gsu.i = &nullsetinteger_gsu;

    for (capcode = (char **)numcodes; *capcode; capcode++) {
	if ((num = tgetnum(*capcode)) != -1) {
	    pm->u.val = num;
	    pm->node.nam = dupstring(*capcode);
	    func(&pm->node, flags);
	}
    }

    pm->node.flags = PM_READONLY | PM_SCALAR;
    pm->gsu.s = &nullsetscalar_gsu;

    for (capcode = (char **)
#ifdef HAVE_STRCODES
	     strcodes
#else
	     zstrcodes
#endif
	     ; *capcode; capcode++) {
	if ((tcstr = (char *)tgetstr(*capcode,&u)) != NULL &&
	    tcstr != (char *)-1) {
	    pm->u.str = dupstring(tcstr);
	    pm->node.nam = dupstring(*capcode);
	    func(&pm->node, flags);
	}
    }
}

static struct paramdef partab[] = {
    SPECIALPMDEF("termcap", PM_READONLY, NULL, gettermcap, scantermcap)
};

/**/
#endif /* HAVE_TGETENT */

static struct features module_features = {
#ifdef HAVE_TGETENT
    bintab, sizeof(bintab)/sizeof(*bintab),
#else
    NULL, 0,
#endif
    NULL, 0,
    NULL, 0,
#ifdef HAVE_TGETENT
    partab, sizeof(partab)/sizeof(*partab),
#else
    NULL, 0,
#endif
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
#ifdef HAVE_TGETENT
    zsetupterm();
#endif
    return  0;
}

/**/
int
cleanup_(Module m)
{
#ifdef HAVE_TGETENT
    zdeleteterm();
#endif
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    return 0;
}
