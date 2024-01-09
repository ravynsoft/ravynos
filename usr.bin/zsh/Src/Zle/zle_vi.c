/*
 * zle_vi.c - vi-specific functions
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
#include "zle_vi.pro"

/* != 0 if we're getting a vi range */

/**/
int virangeflag;

/* kludge to get cw and dw to work right */

/**/
int wordflag;

/* != 0 if we're killing lines into a buffer, vi-style */

/**/
int vilinerange;

/*
 * lastvichg: last vi change buffer, for vi change repetition
 * curvichg: current incomplete vi change
 */

/**/
struct vichange lastvichg, curvichg;

/*
 * true whilst a vi change is active causing keys to be
 * accumulated in curvichg.buf
 * first set to 2 and when the initial widget finishes, reduced to 1 if
 * in insert mode implying that the change continues until returning to
 * normal mode
 */

/**/
int vichgflag;

/*
 * analogous to vichgflag for a repeated change with the value following
 * a similar pattern (is 3 until first repeated widget starts)
 */

/**/
int viinrepeat;

/* point where vi insert mode was last entered */

/**/
int viinsbegin;

/**
 * im: >= 0: is an insertmode
 *    -1: skip setting insert/overwrite mode
 *    -2: entering viins at start of editing from clean --- don't use
 *        inrepeat or keybuf, synthesise an entry to insert mode.
 * Note that zmult is updated so this should be called before zmult is used.
 */

/**/
void
startvichange(int im)
{
    if (im > -1)
	insmode = im;
    if (viinrepeat && im != -2) {
	zmod = lastvichg.mod;
	vichgflag = 0;
    } else if (!vichgflag) {
	curvichg.mod = zmod;
	if (curvichg.buf)
	    free(curvichg.buf);
	curvichg.buf = (char *)zalloc(curvichg.bufsz = 16 + keybuflen);
	if (im == -2) {
	    vichgflag = 1;
	    curvichg.buf[0] =
		zlell ? (insmode ? (zlecs < zlell ? 'i' : 'a') : 'R') : 'o';
	    curvichg.buf[1] = '\0';
	    curvichg.bufptr = 1;
	} else {
	    vichgflag = 2;
	    strcpy(curvichg.buf, keybuf);
	    unmetafy(curvichg.buf, &curvichg.bufptr);
	}
    }
}

/**/
static void
startvitext(int im)
{
    startvichange(im);
    selectkeymap("main", 1);
    vistartchange = undo_changeno;
    viinsbegin = zlecs;
}

/**/
ZLE_INT_T
vigetkey(void)
{
    Keymap mn = openkeymap("main");
    char m[3], *str;
    Thingy cmd;

    if (getbyte(0L, NULL, 1) == EOF)
	return ZLEEOF;

    m[0] = lastchar;
    metafy(m, 1, META_NOALLOC);
    if(mn)
	cmd = keybind(mn, m, &str);
    else
	cmd = t_undefinedkey;

    if (!cmd || cmd == Th(z_sendbreak)) {
	return ZLEEOF;
    } else if (cmd == Th(z_quotedinsert)) {
	if (getfullchar(0) == ZLEEOF)
	    return ZLEEOF;
    } else if(cmd == Th(z_viquotedinsert)) {
	ZLE_CHAR_T sav = zleline[zlecs];

	zleline[zlecs] = '^';
	zrefresh();
	getfullchar(0);
	zleline[zlecs] = sav;
	if(LASTFULLCHAR == ZLEEOF)
	    return ZLEEOF;
    } else if (cmd == Th(z_vicmdmode)) {
	return ZLEEOF;
    }
#ifdef MULTIBYTE_SUPPORT
    if (!lastchar_wide_valid)
    {
	getrestchar(lastchar, NULL, NULL);
    }
#endif
    return LASTFULLCHAR;
}

/**/
static int
getvirange(int wf)
{
    int pos = zlecs, mpos = mark, ret = 0;
    int visual = region_active; /* movement command might set it */
    int mult1 = zmult, hist1 = histline;
    Thingy k2;

    if (visual) {
	if (!zlell)
	    return -1;
	pos = mark;
	vilinerange = (visual == 2);
	region_active = 0;
    } else {
	virangeflag = 1;
	wordflag = wf;
	mark = -1;
	/* use operator-pending keymap if one exists */
	Keymap km = openkeymap("viopp");
	if (km)
	    selectlocalmap(km);
	/* Now we need to execute the movement command, to see where it *
	 * actually goes.  virangeflag here indicates to the movement   *
	 * function that it should place the cursor at the end of the   *
	 * range, rather than where the cursor would actually go if it  *
	 * were executed normally.  This makes a difference to some     *
	 * commands, but not all.  For example, if searching forward    *
	 * for a character, under normal circumstances the cursor lands *
	 * on the character.  For a range, the range must include the   *
	 * character, so the cursor gets placed after the character if  *
	 * virangeflag is set.                                          */
	zmod.flags &= ~MOD_TMULT;
	do {
	    vilinerange = 0;
	    prefixflag = 0;
	    if (!(k2 = getkeycmd()) || (k2->flags & DISABLED) ||
		    k2 == Th(z_sendbreak)) {
		wordflag = 0;
		virangeflag = 0;
		mark = mpos;
		return -1;
	    }
	    /*
	     * With k2 == bindk, the command key is repeated:
	     * a number of lines is used.  If the function used
	     * returns 1, we fail.
	     */
	    if ((k2 == bindk) ? dovilinerange() : execzlefunc(k2, zlenoargs, 1, 0))
		ret = -1;
	    if (viinrepeat)
		zmult = mult1;
	    else {
		zmult = mult1 * zmod.tmult;
		if (vichgflag == 2)
		    curvichg.mod.mult = zmult;
            }
	} while(prefixflag && !ret);
	wordflag = 0;
	selectlocalmap(NULL);

	/* It is an error to use a non-movement command to delimit the *
	 * range.  We here reject the case where the command modified  *
	 * the line, or selected a different history line.             */
	if (histline != hist1 || zlell != lastll || memcmp(zleline, lastline, zlell)) {
	    histline = hist1;
	    ZS_memcpy(zleline, lastline, zlell = lastll);
	    zlecs = pos;
	    mark = mpos;
	    virangeflag = 0;
	    return -1;
	}

	/* Can't handle an empty file.  Also, if the movement command *
	 * failed, or didn't move, it is an error.                    */
	if (!zlell || (zlecs == pos && (mark == -1 || mark == zlecs) &&
		    virangeflag != 2) || ret == -1) {
	    mark = mpos;
	    virangeflag = 0;
	    return -1;
	}
	virangeflag = 0;

	/* if the mark has moved, ignore the original cursor position *
	 * and use the mark.                                          */
	if (mark != -1)
	    pos = mark;
    }
    mark = mpos;

    /* Get the range the right way round.  zlecs is placed at the *
     * start of the range, and pos (the return value of this   *
     * function) is the end.                                   */
    if (zlecs > pos) {
	int tmp = zlecs;
	zlecs = pos;
	pos = tmp;
    }

    /* visual selection mode needs to include additional position */
    if (visual == 1 && pos < zlell && invicmdmode())
	INCPOS(pos);

    /* Was it a line-oriented move?  If so, the command will have set *
     * the vilinerange flag.  In this case, entire lines are taken,   *
     * rather than just the sequence of characters delimited by pos   *
     * and zlecs.  The terminating newline is left out of the range,  *
     * which the real command must deal with appropriately.  At this  *
     * point we just need to make the range encompass entire lines.   */
    vilinerange = (zmod.flags & MOD_LINE) ||
	    (vilinerange && !(zmod.flags & MOD_CHAR));
    if (vilinerange) {
	int newcs = findbol();
	lastcol = zlecs - newcs;
	zlecs = pos;
	pos = findeol();
	zlecs = newcs;
    } else if (!visual) {
	/* for a character-wise move don't include a newline at the *
	 * end of the range                                         */
	int prev = pos;
	DECPOS(prev);
	if (zleline[prev] == ZWC('\n'))
	    pos = prev;
    }
    return pos;
}

/**/
static int
dovilinerange(void)
{
    int pos = zlecs, n = zmult;

    /* A number of lines is taken as the range.  The current line *
     * is included.  If the repeat count is positive the lines go *
     * downward, otherwise upward.  The repeat count gives the    *
     * number of lines.                                           */
    vilinerange = 1;
    if (!n)
	return 1;
    if (n > 0) {
	while(n-- && zlecs <= zlell)
	    zlecs = findeol() + 1;
	if (n != -1) {
	    zlecs = pos;
	    return 1;
	}
	DECCS();
    } else {
	while(n++ && zlecs >= 0)
	    zlecs = findbol() - 1;
	if (n != 1) {
	    zlecs = pos;
	    return 1;
	}
	INCCS();
    }
    virangeflag = 2;
    return 0;
}

/**/
int
viaddnext(UNUSED(char **args))
{
    if (zlecs != findeol())
	INCCS();
    startvitext(1);
    return 0;
}

/**/
int
viaddeol(UNUSED(char **args))
{
    zlecs = findeol();
    startvitext(1);
    return 0;
}

/**/
int
viinsert(UNUSED(char **args))
{
    startvitext(1);
    return 0;
}

/*
 * Go to vi insert mode when we first start the line editor.
 * Iniialises some other stuff.
 */

/**/
void
viinsert_init(void)
{
    startvitext(-2);
}

/**/
int
viinsertbol(UNUSED(char **args))
{
    vifirstnonblank(zlenoargs);
    startvitext(1);
    return 0;
}

/**/
int
videlete(UNUSED(char **args))
{
    int c2, ret = 1;

    startvichange(1);
    if ((c2 = getvirange(0)) != -1) {
	forekill(c2 - zlecs, CUT_RAW);
	ret = 0;
	if (vilinerange && zlell) {
	    lastcol = -1;
	    if (zlecs == zlell)
		DECCS();
	    foredel(1, 0);
	    vifirstnonblank(zlenoargs);
	}
    }
    return ret;
}

/**/
int
videletechar(char **args)
{
    int n;

    startvichange(-1);
    n = zmult;

    /* handle negative argument */
    if (n < 0) {
	int ret;
	zmult = -n;
	ret = vibackwarddeletechar(args);
	zmult = n;
	return ret;
    }
    /* it is an error to be on the end of line */
    if (zlecs == zlell || zleline[zlecs] == '\n')
	return 1;
    /* Put argument into the acceptable range -- it is not an error to  *
     * specify a greater count than the number of available characters. */
    /* HERE: we should do the test properly with INCPOS(). */
    if (n > findeol() - zlecs) {
	n = findeol() - zlecs;
	/* do the deletion */
	forekill(n, CUT_RAW);
    } else {
	forekill(n, 0);
    }
    return 0;
}

/**/
int
vichange(UNUSED(char **args))
{
    int c2, ret = 1;

    startvichange(1);
    if ((c2 = getvirange(1)) != -1) {
	ret = 0;
	forekill(c2 - zlecs, CUT_RAW);
	selectkeymap("main", 1);
	viinsbegin = zlecs;
	vistartchange = undo_changeno;
    }
    return ret;
}

/**/
int
visubstitute(UNUSED(char **args))
{
    int n;

    startvichange(1);
    n = zmult;
    if (n < 0)
	return 1;
    /* it is an error to be on the end of line */
    if (zlecs == zlell || zleline[zlecs] == '\n')
	return 1;
    if (region_active) {
	killregion(zlenoargs);
    } else {
	/* Put argument into the acceptable range -- it is not an error to  *
	* specify a greater count than the number of available characters. */
	if (n > findeol() - zlecs)
	    n = findeol() - zlecs;
	/* do the substitution */
	forekill(n, CUT_RAW);
    }
    startvitext(1);
    return 0;
}

/**/
int
vichangeeol(UNUSED(char **args))
{
    int a, b;
    if (region_active) {
	regionlines(&a, &b);
	zlecs = a;
	region_active = 0;
	cut(zlecs, b - zlecs, CUT_RAW);
	shiftchars(zlecs, b - zlecs);
    } else
	forekill(findeol() - zlecs, CUT_RAW);
    startvitext(1);
    return 0;
}

/**/
int
vichangewholeline(char **args)
{
    vifirstnonblank(args);
    return vichangeeol(zlenoargs);
}

/**/
int
viyank(UNUSED(char **args))
{
    int c2, ret = 1;

    startvichange(1);
    if ((c2 = getvirange(0)) != -1) {
	cut(zlecs, c2 - zlecs, CUT_YANK);
	ret = 0;
    }
    /* cursor now at the start of the range yanked. For line mode
     * restore the column position */
    if (vilinerange && lastcol != -1) {
	int x = findeol();

	if ((zlecs += lastcol) >= x) {
	    zlecs = x;
	    if (zlecs > findbol() && invicmdmode())
		DECCS();
	}
#ifdef MULTIBYTE_SUPPORT
	else
	    CCRIGHT();
#endif
	lastcol = -1;
    }
    return ret;
}

/**/
int
viyankeol(UNUSED(char **args))
{
    int x = findeol();

    startvichange(-1);
    if (x == zlecs)
	return 1;
    cut(zlecs, x - zlecs, CUT_YANK);
    return 0;
}

/**/
int
viyankwholeline(UNUSED(char **args))
{
    int bol = findbol(), oldcs = zlecs;
    int n;

    startvichange(-1);
    n = zmult;
    if (n < 1)
	return 1;
    while(n--) {
     if (zlecs > zlell) {
	zlecs = oldcs;
	return 1;
     }
     zlecs = findeol() + 1;
    }
    vilinerange = 1;
    cut(bol, zlecs - bol - 1, CUT_YANK);
    zlecs = oldcs;
    return 0;
}

/**/
int
vireplace(UNUSED(char **args))
{
    startvitext(0);
    return 0;
}

/* vi-replace-chars has some oddities relating to vi-repeat-change.  In *
 * the real vi, if one does 3r at the end of a line, it feeps without   *
 * reading the argument, and won't repeat the action.  A successful rx  *
 * followed by 3. at the end of a line (or 3rx followed by . at the end *
 * of a line) will obviously feep after the ., even though it has the   *
 * argument available.  Here repeating is tied very closely to argument *
 * reading, so some trickery is needed to emulate this.  When repeating *
 * a change, we always read the argument normally, even if the count    *
 * was bad.  When recording a change for repeating, and a bad count is  *
 * given, we squash the repeat buffer to avoid repeating the partial    *
 * command.                                                             */

/**/
int
vireplacechars(UNUSED(char **args))
{
    ZLE_INT_T ch;
    int n, fail = 0, newchars = 0;

    startvichange(1);
    n = zmult;
    if (n > 0) {
	if (region_active) {
	    int a, b;
	    if (region_active == 1) {
		if (mark > zlecs) {
		    a = zlecs;
		    b = mark;
		} else {
		    a = mark;
		    b = zlecs;
		}
		INCPOS(b);
	    } else
		regionlines(&a, &b);
	    zlecs = a;
	    if (b > zlell)
		b = zlell;
	    n = b - a;
	    while (a < b) {
		newchars++;
		INCPOS(a);
            }
	    region_active = 0;
	} else {
	    int pos = zlecs;
	    while (n-- > 0) {
		if (pos == zlell || zleline[pos] == ZWC('\n')) {
		    fail = 1;
		    break;
		}
		newchars++;
		INCPOS(pos);
	    }
	    n = pos - zlecs;
	}
    }

    /* check argument range */
    if (n < 1 || fail) {
	if (viinrepeat)
	    vigetkey();
	return 1;
    }
    /* get key */
    if((ch = vigetkey()) == ZLEEOF) {
	return 1;
    }
    /* do change */
    if (ch == ZWC('\r') || ch == ZWC('\n')) {
	/* <return> handled specially */
	zlecs += n - 1;
	backkill(n - 1, CUT_RAW);
	zleline[zlecs++] = '\n';
    } else {
	/*
	 * Make sure we delete displayed characters, including
	 * attach combining characters. n includes this as a raw
	 * buffer offset.
	 * Use shiftchars so as not to adjust the cursor position;
	 * we are overwriting anything that remains directly.
	 * With a selection this will replace newlines which vim
	 * doesn't do but this simplifies things a lot.
	 */
	if (n > newchars)
	    shiftchars(zlecs, n - newchars);
	else if (n < newchars)
	    spaceinline(newchars - n);
	while (newchars--)
	    zleline[zlecs++] = ch;
	zlecs--;
    }
    return 0;
}

/**/
int
vicmdmode(UNUSED(char **args))
{
    if (invicmdmode() || selectkeymap("vicmd", 0))
	return 1;
    mergeundo();
    insmode = unset(OVERSTRIKE);
    if (vichgflag == 1) {
	vichgflag = 0;
	if (lastvichg.buf)
	    free(lastvichg.buf);
	lastvichg = curvichg;
	curvichg.buf = NULL;
    }
    if (viinrepeat == 1)
        viinrepeat = 0;
    if (zlecs != findbol())
	DECCS();
    return 0;
}

/**/
int
viopenlinebelow(UNUSED(char **args))
{
    zlecs = findeol();
    spaceinline(1);
    zleline[zlecs++] = '\n';
    startvitext(1);
    clearlist = 1;
    return 0;
}

/**/
int
viopenlineabove(UNUSED(char **args))
{
    zlecs = findbol();
    spaceinline(1);
    zleline[zlecs] = '\n';
    startvitext(1);
    clearlist = 1;
    return 0;
}

/**/
int
vioperswapcase(UNUSED(char **args))
{
    int oldcs, c2, ret = 1;

    /* get the range */
    startvichange(1);
    if ((c2 = getvirange(0)) != -1) {
	oldcs = zlecs;
	/* swap the case of all letters within range */
	while (zlecs < c2) {
	    if (ZC_ilower(zleline[zlecs]))
		zleline[zlecs] = ZC_toupper(zleline[zlecs]);
	    else if (ZC_iupper(zleline[zlecs]))
		zleline[zlecs] = ZC_tolower(zleline[zlecs]);
	    INCCS();
	}
	/* go back to the first line of the range */
	zlecs = oldcs;
	ret = 0;
#if 0
	vifirstnonblank();
#endif
    }
    return ret;
}

/**/
int
viupcase(UNUSED(char **args))
{
    int oldcs, c2, ret = 1;

    /* get the range */
    startvichange(1);
    if ((c2 = getvirange(0)) != -1) {
	oldcs = zlecs;
	/* covert the case of all letters within range */
	while (zlecs < c2) {
	    zleline[zlecs] = ZC_toupper(zleline[zlecs]);
	    INCCS();
	}
	/* go back to the first line of the range */
	zlecs = oldcs;
	ret = 0;
    }
    return ret;
}

/**/
int
vidowncase(UNUSED(char **args))
{
    int oldcs, c2, ret = 1;

    /* get the range */
    startvichange(1);
    if ((c2 = getvirange(0)) != -1) {
	oldcs = zlecs;
	/* convert the case of all letters within range */
	while (zlecs < c2) {
	    zleline[zlecs] = ZC_tolower(zleline[zlecs]);
	    INCCS();
	}
	/* go back to the first line of the range */
	zlecs = oldcs;
	ret = 0;
    }
    return ret;
}

/**/
int
virepeatchange(UNUSED(char **args))
{
    /* make sure we have a change to repeat */
    if (!lastvichg.buf || vichgflag || virangeflag)
	return 1;
    /* restore or update the saved count and buffer */
    if (zmod.flags & MOD_MULT) {
	lastvichg.mod.mult = zmod.mult;
	lastvichg.mod.flags |= MOD_MULT;
    }
    if (zmod.flags & MOD_VIBUF) {
	lastvichg.mod.vibuf = zmod.vibuf;
	lastvichg.mod.flags = (lastvichg.mod.flags & ~MOD_VIAPP) |
	    MOD_VIBUF | (zmod.flags & MOD_VIAPP);
    } else if (lastvichg.mod.flags & MOD_VIBUF &&
	    lastvichg.mod.vibuf >= 27 && lastvichg.mod.vibuf <= 34)
	lastvichg.mod.vibuf++; /* for "1 to "8 advance to next buffer */
    /* repeat the command */
    viinrepeat = 3;
    ungetbytes(lastvichg.buf, lastvichg.bufptr);
    return 0;
}

/**/
int
viindent(UNUSED(char **args))
{
    int oldcs = zlecs, c2;

    startvichange(1);
    /* force line range */
    if (region_active == 1)
	region_active = 2;
    /* get the range */
    if ((c2 = getvirange(0)) == -1) {
	return 1;
    }
    /* must be a line range */
    if (!vilinerange) {
	zlecs = oldcs;
	return 1;
    }
    oldcs = zlecs;
    /* add a tab to the beginning of each line within range */
    while (zlecs <= c2 + 1) {
	if (zleline[zlecs] == '\n') { /* leave blank lines alone */
	    ++zlecs;
	} else {
	    spaceinline(1);
	    zleline[zlecs] = '\t';
	    zlecs = findeol() + 1;
	}
    }
    /* go back to the first line of the range */
    zlecs = oldcs;
    vifirstnonblank(zlenoargs);
    return 0;
}

/**/
int
viunindent(UNUSED(char **args))
{
    int oldcs = zlecs, c2;

    startvichange(1);
    /* force line range */
    if (region_active == 1)
	region_active = 2;
    /* get the range */
    if ((c2 = getvirange(0)) == -1) {
	return 1;
    }
    /* must be a line range */
    if (!vilinerange) {
	zlecs = oldcs;
	return 1;
    }
    oldcs = zlecs;
    /* remove a tab from the beginning of each line within range */
    while (zlecs < c2) {
	if (zleline[zlecs] == '\t')
	    foredel(1, 0);
	zlecs = findeol() + 1;
    }
    /* go back to the first line of the range */
    zlecs = oldcs;
    vifirstnonblank(zlenoargs);
    return 0;
}

/**/
int
vibackwarddeletechar(char **args)
{
    int n;

    if (invicmdmode())
	startvichange(-1);

    /* handle negative argument */
    n = zmult;
    if (n < 0) {
	int ret;
	zmult = -n;
	ret = videletechar(args);
	zmult = n;
	return ret;
    }
    /* It is an error to be at the beginning of the line, or (in *
     * insert mode) to delete past the beginning of insertion.   */
    if ((!invicmdmode() && zlecs - n < viinsbegin) || zlecs == findbol()) {
	return 1;
    }
    /* Put argument into the acceptable range -- it is not an error to  *
     * specify a greater count than the number of available characters. */
    /* HERE: we should do the test properly with DECPOS(). */
    if (n > zlecs - findbol()) {
	n = zlecs - findbol();
	/* do the deletion */
	backkill(n, CUT_FRONT|CUT_RAW);
    } else
	backkill(n, CUT_FRONT);
    return 0;
}

/**/
int
vikillline(UNUSED(char **args))
{
    if (viinsbegin > zlecs)
	return 1;
    backdel(zlecs - viinsbegin, CUT_RAW);
    return 0;
}

/**/
int
vijoin(UNUSED(char **args))
{
    int x, pos;
    int n;
    int visual = region_active;

    startvichange(-1);
    n = zmult;
    if (n < 1)
	return 1;
    if (visual && zlecs > mark) {
	exchangepointandmark(zlenoargs);
	x = findeol();
	if (x >= mark) {
	    exchangepointandmark(zlenoargs);
	    return 1;
	}
    } else if ((x = findeol()) == zlell || (visual && x >= mark))
	return 1;

    do {
	zlecs = x + 1;
	pos = zlecs;
	for (; zlecs != zlell && ZC_iblank(zleline[zlecs]); INCPOS(zlecs))
	    ;
	x = 1 + (zlecs - pos);
	backdel(x, CUT_RAW);
	if (zlecs) {
	    int pos = zlecs;
	    DECPOS(pos);
	    if (ZC_iblank(zleline[pos])) {
		zlecs = pos;
		continue;
	    }
	}
	spaceinline(1);
	zleline[zlecs] = ZWC(' ');
    } while (!((!visual && --n < 2) || (x = findeol()) == zlell || (visual && x >= mark)));

    return 0;
}

/**/
int
viswapcase(UNUSED(char **args))
{
    int eol, n;

    startvichange(-1);
    n = zmult;
    if (n < 1)
	return 1;
    eol = findeol();
    if (zlecs == eol)
	return 1;
    while (zlecs < eol && n--) {
	if (ZC_ilower(zleline[zlecs]))
	    zleline[zlecs] = ZC_toupper(zleline[zlecs]);
	else if (ZC_iupper(zleline[zlecs]))
	    zleline[zlecs] = ZC_tolower(zleline[zlecs]);
	INCCS();
    }
    if (zlecs && zlecs == eol)
	DECCS();
    return 0;
}

/**/
int
vicapslockpanic(UNUSED(char **args))
{
    clearlist = 1;
    zbeep();
    statusline = "press a lowercase key to continue";
    zrefresh();
    while (!ZC_ilower(getfullchar(0)));
    statusline = NULL;
    return 0;
}

/**/
int
visetbuffer(char **args)
{
    ZLE_INT_T ch;

    if (*args) {
	ch = **args;
	if (args[1] || (ch && (*args)[1]))
	    return 1;
    } else {
	ch = getfullchar(0);
    }
    if (ch == ZWC('_')) {
	zmod.flags |= MOD_NULL;
	prefixflag = 1;
	return 0;
    } else
	zmod.flags &= ~MOD_NULL;
    if ((ch < ZWC('0') || ch > ZWC('9')) &&
	 (ch < ZWC('a') || ch > ZWC('z')) &&
	 (ch < ZWC('A') || ch > ZWC('Z')))
	return 1;
    if (ch >= ZWC('A') && ch <= ZWC('Z'))	/* needed in cut() */
	zmod.flags |= MOD_VIAPP;
    else
	zmod.flags &= ~MOD_VIAPP;
    /* FIXME how portable is it for multibyte encoding? */
    zmod.vibuf = ZC_tolower(ch);
    if (ch >= ZWC('0') && ch <= ZWC('9'))
	zmod.vibuf += - (int)ZWC('0') + 26;
    else
	zmod.vibuf += - (int)ZWC('a');
    zmod.flags |= MOD_VIBUF;
    prefixflag = 1;
    return 0;
}

/**/
int
vikilleol(UNUSED(char **args))
{
    int n = findeol() - zlecs;

    startvichange(-1);
    if (!n) {
	/* error -- line already empty */
	return 1;
    }
    /* delete to end of line */
    forekill(findeol() - zlecs, CUT_RAW);
    return 0;
}

/**/
int
vipoundinsert(UNUSED(char **args))
{
    int oldcs = zlecs;

    startvichange(-1);
    vifirstnonblank(zlenoargs);
    if(zleline[zlecs] != '#') {
	spaceinline(1);
	zleline[zlecs] = '#';
	if(zlecs <= viinsbegin)
	    INCPOS(viinsbegin);
	if (zlecs <= oldcs)
	    INCPOS(oldcs);
	zlecs = oldcs;
    } else {
	foredel(1, 0);
	if (zlecs < viinsbegin)
	    DECPOS(viinsbegin);
	if (zlecs < oldcs)
	    DECPOS(oldcs);
	zlecs = oldcs;
    }
    return 0;
}

/**/
int
viquotedinsert(char **args)
{
#ifndef HAS_TIO
    struct sgttyb sob;
#endif

    spaceinline(1);
    zleline[zlecs] = '^';
    zrefresh();
#ifndef HAS_TIO
    sob = shttyinfo.sgttyb;
    sob.sg_flags = (sob.sg_flags | RAW) & ~ECHO;
    ioctl(SHTTY, TIOCSETN, &sob);
#endif
    getfullchar(0);
#ifndef HAS_TIO
    zsetterm();
#endif
    foredel(1, 0);
    if(LASTFULLCHAR == ZLEEOF)
	return 1;
    else
	return selfinsert(args);
}

/* the 0 key in vi: continue a repeat count in the manner of      *
 * digit-argument if possible, otherwise do vi-beginning-of-line. */

/**/
int
vidigitorbeginningofline(char **args)
{
    if(zmod.flags & MOD_TMULT)
	return digitargument(args);
    else {
	removesuffix();
	invalidatelist();
	return vibeginningofline(args);
    }
}
