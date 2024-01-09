/*
 * zle_word.c - word-related editor functions
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

#include "zle.mdh"
#include "zle_word.pro"

/*
 * In principle we shouldn't consider a zero-length punctuation
 * character (i.e. a modifier of some sort) part of the word unless
 * the base character has.  However, we only consider them part of
 * a word if we so consider all alphanumerics, so the distinction
 * only applies if the characters are modifying something they probably
 * ought not to be modifying.  It's not really clear we need to
 * be clever about this not very useful case.
 */

/**/
int
forwardword(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = backwardword(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	while (zlecs != zlell && ZC_iword(zleline[zlecs]))
	    INCCS();
	if (wordflag && !n)
	    return 0;
	while (zlecs != zlell && !ZC_iword(zleline[zlecs]))
	    INCCS();
    }
    return 0;
}

/*
 * class of character (for vi-mode word motion)
 * 0: blank,  1: alnum or _,  2: punctuation,  3: the others
 */

/**/
int
wordclass(ZLE_CHAR_T x)
{
    return (ZC_iblank(x) ? 0 : ((ZC_ialnum(x) || (ZWC('_') == x)) ? 1 :
		ZC_ipunct(x) ? 2 : 3));
}

/**/
int
viforwardword(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = vibackwardword(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	int nl;
	int cc = wordclass(zleline[zlecs]);
	while (zlecs != zlell && wordclass(zleline[zlecs]) == cc) {
	    INCCS();
	}
	if (wordflag && !n)
	    return 0;
	nl = (zleline[zlecs] == ZWC('\n'));
	while (zlecs != zlell && nl < 2 && ZC_inblank(zleline[zlecs])) {
	    INCCS();
	    nl += (zleline[zlecs] == ZWC('\n'));
	}
    }
    return 0;
}

/**/
int
viforwardblankword(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = vibackwardblankword(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	int nl;
	while (zlecs != zlell && !ZC_inblank(zleline[zlecs]))
	    INCCS();
	if (wordflag && !n)
	    return 0;
	nl = (zleline[zlecs] == ZWC('\n'));
	while (zlecs != zlell && nl < 2 && ZC_inblank(zleline[zlecs])) {
	    INCCS();
	    nl += (zleline[zlecs] == ZWC('\n'));
	}
    }
    return 0;
}

/**/
int
emacsforwardword(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = emacsbackwardword(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	while (zlecs != zlell && !ZC_iword(zleline[zlecs]))
	    INCCS();
	if (wordflag && !n)
	    return 0;
	while (zlecs != zlell && ZC_iword(zleline[zlecs]))
	    INCCS();
    }
    return 0;
}

/**/
int
viforwardblankwordend(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = vibackwardblankwordend(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	while (zlecs != zlell) {
	    int pos = zlecs;
	    INCPOS(pos);
	    if (!ZC_inblank(zleline[pos]))
		break;
	    zlecs = pos;
	}
	while (zlecs != zlell) {
	    int pos = zlecs;
	    INCPOS(pos);
	    if (ZC_inblank(zleline[pos]))
		break;
	    zlecs = pos;
	}
    }
    if (zlecs != zlell && virangeflag)
	INCCS();
    return 0;
}

/**/
int
viforwardwordend(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = vibackwardwordend(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	int pos;
	while (zlecs != zlell) {
	    pos = zlecs;
	    INCPOS(pos);
	    if (!ZC_inblank(zleline[pos]))
		break;
	    zlecs = pos;
	}
	if (zlecs != zlell) {
	    int cc;
	    pos = zlecs;
	    INCPOS(pos);
	    cc = wordclass(zleline[pos]);
	    for (;;) {
		zlecs = pos;
		if (zlecs == zlell)
		    break;
		INCPOS(pos);
		if (wordclass(zleline[pos]) != cc)
			break;
	    }
	}
    }
    if (zlecs != zlell && virangeflag)
	INCCS();
    return 0;
}

/**/
int
backwardword(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = forwardword(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	while (zlecs) {
	    int pos = zlecs;
	    DECPOS(pos);
	    if (ZC_iword(zleline[pos]))
		break;
	    zlecs = pos;
	}
	while (zlecs) {
	    int pos = zlecs;
	    DECPOS(pos);
	    if (!ZC_iword(zleline[pos]))
		break;
	    zlecs = pos;
	}
    }
    return 0;
}

/**/
int
vibackwardword(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = viforwardword(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	int nl = 0;
	while (zlecs) {
	    DECCS();
	    if (!ZC_inblank(zleline[zlecs]))
		break;
	    nl += (zleline[zlecs] == ZWC('\n'));
	    if (nl == 2) {
		INCCS();
		break;
	    }
	}
	if (zlecs) {
	    int pos = zlecs;
	    int cc = wordclass(zleline[pos]);
	    for (;;) {
		zlecs = pos;
		if (zlecs == 0)
		    break;
		DECPOS(pos);
		if (wordclass(zleline[pos]) != cc || ZC_inblank(zleline[pos]))
		    break;
	    }
	}
    }
    return 0;
}

/**/
int
vibackwardblankword(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = viforwardblankword(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	int nl = 0;
	while (zlecs) {
	    int pos = zlecs;
	    DECPOS(pos);
	    if (!ZC_inblank(zleline[pos]))
		break;
	    nl += (zleline[pos] == ZWC('\n'));
	    if (nl == 2) break;
	    zlecs = pos;
	}
	while (zlecs) {
	    int pos = zlecs;
	    DECPOS(pos);
	    if (ZC_inblank(zleline[pos]))
		break;
	    zlecs = pos;
	}
    }
    return 0;
}

/**/
int
vibackwardwordend(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = viforwardwordend(args);
	zmult = n;
	return ret;
    }
    while (n-- && zlecs > 1) {
	int cc = wordclass(zleline[zlecs]);
	DECCS();
	while (zlecs) {
	    if (wordclass(zleline[zlecs]) != cc || ZC_iblank(zleline[zlecs]))
		break;
	    DECCS();
	}
	while (zlecs && ZC_iblank(zleline[zlecs]))
	    DECCS();
    }
    return 0;
}

/**/
int
vibackwardblankwordend(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = viforwardblankwordend(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	while (zlecs && !ZC_inblank(zleline[zlecs]))
	    DECCS();
	while (zlecs && ZC_inblank(zleline[zlecs]))
	    DECCS();
    }
    return 0;
}

/**/
int
emacsbackwardword(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = emacsforwardword(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	while (zlecs) {
	    int pos = zlecs;
	    DECPOS(pos);
	    if (ZC_iword(zleline[pos]))
		break;
	    zlecs = pos;
	}
	while (zlecs) {
	    int pos = zlecs;
	    DECPOS(pos);
	    if (!ZC_iword(zleline[pos]))
		break;
	    zlecs = pos;
	}
    }
    return 0;
}

/**/
int
backwarddeleteword(char **args)
{
    int x = zlecs, n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = deleteword(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	while (x) {
	    int pos = x;
	    DECPOS(pos);
	    if (ZC_iword(zleline[pos]))
		break;
	    x = pos;
	}
	while (x) {
	    int pos = x;
	    DECPOS(pos);
	    if (!ZC_iword(zleline[pos]))
		break;
	    x = pos;
	}
    }
    backdel(zlecs - x, CUT_RAW);
    return 0;
}

/**/
int
vibackwardkillword(UNUSED(char **args))
{
    int x = zlecs, lim = (viinsbegin > findbol()) ? viinsbegin : findbol();
    int n = zmult;

    if (n < 0)
	return 1;
/* this taken from "vibackwardword" */
    while (n--) {
	while (x > lim) {
	    int pos = x;
	    DECPOS(pos);
	    if (!ZC_iblank(zleline[pos]))
		break;
	    x = pos;
	}
	if (x > lim) {
	    int cc;
	    int pos = x;
	    DECPOS(pos);
	    cc = wordclass(zleline[pos]);
	    for (;;) {
		x = pos;
		if (x <= lim)
		    break;
		DECPOS(pos);
		if (wordclass(zleline[pos]) != cc)
		    break;
	    }
	}
    }
    backkill(zlecs - x, CUT_FRONT|CUT_RAW);
    return 0;
}

/**/
int
backwardkillword(char **args)
{
    int x = zlecs;
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = killword(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	while (x) {
	    int pos = x;
	    DECPOS(pos);
	    if (ZC_iword(zleline[pos]))
		break;
	    x = pos;
	}
	while (x) {
	    int pos = x;
	    DECPOS(pos);
	    if (!ZC_iword(zleline[pos]))
		break;
	    x = pos;
	}
    }
    backkill(zlecs - x, CUT_FRONT|CUT_RAW);
    return 0;
}

/**/
int
upcaseword(UNUSED(char **args))
{
    int n = zmult;
    int neg = n < 0, ocs = zlecs;

    if (neg)
	n = -n;
    while (n--) {
	while (zlecs != zlell && !ZC_iword(zleline[zlecs]))
	    INCCS();
	while (zlecs != zlell && ZC_iword(zleline[zlecs])) {
	    zleline[zlecs] = ZC_toupper(zleline[zlecs]);
	    INCCS();
	}
    }
    if (neg)
	zlecs = ocs;
    return 0;
}

/**/
int
downcaseword(UNUSED(char **args))
{
    int n = zmult;
    int neg = n < 0, ocs = zlecs;

    if (neg)
	n = -n;
    while (n--) {
	while (zlecs != zlell && !ZC_iword(zleline[zlecs]))
	    INCCS();
	while (zlecs != zlell && ZC_iword(zleline[zlecs])) {
	    zleline[zlecs] = ZC_tolower(zleline[zlecs]);
	    INCCS();
	}
    }
    if (neg)
	zlecs = ocs;
    return 0;
}

/**/
int
capitalizeword(UNUSED(char **args))
{
    int first, n = zmult;
    int neg = n < 0, ocs = zlecs;

    if (neg)
	n = -n;
    while (n--) {
	first = 1;
	while (zlecs != zlell && !ZC_iword(zleline[zlecs]))
	    INCCS();
	while (zlecs != zlell && ZC_iword(zleline[zlecs]) && !ZC_ialpha(zleline[zlecs]))
	    INCCS();
	while (zlecs != zlell && ZC_iword(zleline[zlecs])) {
	    zleline[zlecs] = (first) ? ZC_toupper(zleline[zlecs]) :
		ZC_tolower(zleline[zlecs]);
	    first = 0;
	    INCCS();
	}
    }
    if (neg)
	zlecs = ocs;
    return 0;
}

/**/
int
deleteword(char **args)
{
    int x = zlecs;
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = backwarddeleteword(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	while (x != zlell && !ZC_iword(zleline[x]))
	    INCPOS(x);
	while (x != zlell && ZC_iword(zleline[x]))
	    INCPOS(x);
    }
    foredel(x - zlecs, CUT_RAW);
    return 0;
}

/**/
int
killword(char **args)
{
    int x = zlecs;
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = backwardkillword(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	while (x != zlell && !ZC_iword(zleline[x]))
	    INCPOS(x);
	while (x != zlell && ZC_iword(zleline[x]))
	    INCPOS(x);
    }
    forekill(x - zlecs, CUT_RAW);
    return 0;
}

/**/
int
transposewords(UNUSED(char **args))
{
    int p1, p2, p3, p4, pt, len, x = zlecs, pos;
    ZLE_STRING_T temp, pp;
    int n = zmult;
    int neg = n < 0, ocs = zlecs;

    if (neg)
	n = -n;

    while (x != zlell && zleline[x] != ZWC('\n') && !ZC_iword(zleline[x]))
	INCPOS(x);

    if (x == zlell || zleline[x] == ZWC('\n')) {
	x = zlecs;
	while (x) {
	    if (ZC_iword(zleline[x]))
		break;
	    pos = x;
	    DECPOS(pos);
	    if (zleline[pos] == ZWC('\n'))
		break;
	    x = pos;
	}
	if (!x)
	    return 1;
	pos = x;
	DECPOS(pos);
	if (zleline[pos] == ZWC('\n'))
	    return 1;
    }

    for (p4 = x; p4 != zlell && ZC_iword(zleline[p4]); INCPOS(p4))
	;

    for (p3 = p4; p3; ) {
	pos = p3;
	DECPOS(pos);
	if (!ZC_iword(zleline[pos]))
	    break;
	p3 = pos;
    }

    if (!p3)
	return 1;

    p1 = p2 = pt = p3;

    while (n--) {
	for (p2 = pt; p2; ) {
	    pos = p2;
	    DECPOS(pos);
	    if (ZC_iword(zleline[pos]))
		break;
	    p2 = pos;
	}
	if (!p2)
	    return 1;
	for (p1 = p2; p1; ) {
	    pos = p1;
	    DECPOS(pos);
	    if (!ZC_iword(zleline[pos]))
		break;
	    p1 = pos;
	}
	pt = p1;
    }

    pp = temp = (ZLE_STRING_T)zhalloc((p4 - p1)*ZLE_CHAR_SIZE);
    len = p4 - p3;
    ZS_memcpy(pp, zleline + p3, len);
    pp += len;
    len = p3 - p2;
    ZS_memcpy(pp, zleline + p2, len);
    pp += len;
    ZS_memcpy(pp, zleline + p1, p2 - p1);

    ZS_memcpy(zleline + p1, temp, p4 - p1);

    if (neg)
	zlecs = ocs;
    else
	zlecs = p4;

    return 0;
}
