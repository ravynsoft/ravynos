/*
 * deltochar.c - ZLE module implementing Emacs' zap-to-char
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1996-1997 Peter Stephenson
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Peter Stephenson or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Peter Stephenson and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Peter Stephenson and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Peter Stephenson and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "deltochar.mdh"
#include "deltochar.pro"

static Widget w_deletetochar;
static Widget w_zaptochar;

/**/
static int
deltochar(UNUSED(char **args))
{
    ZLE_INT_T c = getfullchar(0);
    int dest = zlecs, ok = 0, n = zmult;
    int zap = (bindk->widget == w_zaptochar);

    if (n > 0) {
	while (n-- && dest != zlell) {
	    while (dest != zlell && (ZLE_INT_T)zleline[dest] != c)
		INCPOS(dest);
	    if (dest != zlell) {
		if (!zap || n > 0)
		    INCPOS(dest);
		if (!n) {
		    forekill(dest - zlecs, CUT_RAW);
		    ok++;
		}
	    }
	}
    } else {
	/* ignore character cursor is on when scanning backwards */
	if (dest)
	    DECPOS(dest);
	while (n++ && dest != 0) {
	    while (dest != 0 && (ZLE_INT_T)zleline[dest] != c)
		DECPOS(dest);
	    if ((ZLE_INT_T)zleline[dest] == c) {
		if (!n) {
		    /* HERE adjust zap for trailing combining chars */
		    backkill(zlecs - dest - zap, CUT_RAW|CUT_FRONT);
		    ok++;
		}
		if (dest)
		    DECPOS(dest);
	    }
	}
    }
    return !ok;
}


static struct features module_features = {
    NULL, 0,
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
boot_(Module m)
{
    w_deletetochar = addzlefunction("delete-to-char", deltochar,
                                    ZLE_KILL | ZLE_KEEPSUFFIX);
    if (w_deletetochar) {
	w_zaptochar = addzlefunction("zap-to-char", deltochar,
				     ZLE_KILL | ZLE_KEEPSUFFIX);
	if (w_zaptochar)
	    return 0;
	deletezlefunction(w_deletetochar);
    }
    zwarnnam(m->node.nam, "deltochar: name clash when adding ZLE functions");
    return -1;
}

/**/
int
cleanup_(Module m)
{
    deletezlefunction(w_deletetochar);
    deletezlefunction(w_zaptochar);
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    return 0;
}
