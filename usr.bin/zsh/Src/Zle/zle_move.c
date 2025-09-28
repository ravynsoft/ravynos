/*
 * zle_move.c - editor movement
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
#include "zle_move.pro"

static int vimarkcs[27], vimarkline[27];

#ifdef MULTIBYTE_SUPPORT
/*
 * Take account of combining characters when moving left.  If
 * we are on a zero-width printable wide character and are
 * treating these as part of the base character for display purposes,
 * move left until we reach a non-zero-width printable character
 * (the base character).  If we reach something else first, stay where we
 * were.
 *
 * If setpos is non-zero, update zlecs on success.
 * Return 1 if we were on a combining char and could move, else 0.
 */
/**/
int
alignmultiwordleft(int *pos, int setpos)
{
    int loccs = *pos;

    /* generic nothing to do test */
    if (!isset(COMBININGCHARS) || loccs == zlell || loccs == 0)
	return 0;

    /* need to be on combining character */
    if (!IS_COMBINING(zleline[loccs]))
	 return 0;

    /* yes, go left */
    loccs--;

    for (;;) {
	if (IS_BASECHAR(zleline[loccs])) {
	    /* found start position */
	    if (setpos)
		*pos = loccs;
	    return 1;
	} else if (!IS_COMBINING(zleline[loccs])) {
	    /* no go */
	    return 0;
	}
	/* combining char, keep going */
	if (loccs-- == 0)
	    return 0;
    }
}


/*
 * Same principle when moving right.  We need to check if
 * alignmultiwordleft() would be successful in order to decide
 * if we're on a combining character, and if so we move right to
 * anything that isn't one.
 */
/**/
int
alignmultiwordright(int *pos, int setpos)
{
    int loccs;

    /*
     * Are we on a suitable character?
     */
    if (!alignmultiwordleft(pos, 0))
	return 0;

    /* yes, go right */
    loccs = *pos + 1;

    while (loccs < zlell) {
	/* Anything other than a combining char will do here */
	if (!IS_COMBINING(zleline[loccs])) {
	    if (setpos)
		*pos = loccs;
	    return 1;
	}
	loccs++;
    }

    if (setpos)
	*pos = loccs;
    return 1;
}


/* Move cursor right, checking for combining characters */

/**/
mod_export void
inccs(void)
{
    zlecs++;
    alignmultiwordright(&zlecs, 1);
}


/* Move cursor left, checking for combining characters */

/**/
mod_export void
deccs(void)
{
    zlecs--;
    alignmultiwordleft(&zlecs, 1);
}

/* Same utilities for general position */

/**/
mod_export void
incpos(int *pos)
{
    (*pos)++;
    alignmultiwordright(pos, 1);
}


/**/
mod_export void
decpos(int *pos)
{
    (*pos)--;
    alignmultiwordleft(pos, 1);
}
#endif


/* Size of buffer in the following function */
#define BMC_BUFSIZE MB_CUR_MAX
/*
 * For a metafied string that starts at "start" and where the
 * current position is "ptr", go back one full character,
 * taking account of combining characters if necessary.
 */

/**/
mod_export char *
backwardmetafiedchar(char *start, char *endptr, convchar_t *retchr)
{
#ifdef MULTIBYTE_SUPPORT
    int charlen = 0;
    char *last = NULL, *bufptr, *ptr = endptr;
    convchar_t lastc = (convchar_t)0; /* not used, silence compiler */
    mbstate_t mbs;
    size_t ret;
    wchar_t wc;
    VARARR(char, buf, BMC_BUFSIZE);

    bufptr = buf + BMC_BUFSIZE;
    while (ptr > start) {
	ptr--;
	/*
	 * Scanning backwards we're not guaranteed ever to find a
	 * valid character.  If we've looked as far as we should
	 * need to, give up.
	 */
	if (bufptr-- == buf)
	    break;
	charlen++;
	if (ptr > start && ptr[-1] == Meta)
	    *bufptr = *ptr-- ^ 32;
	else
	    *bufptr = *ptr;

	/* we always need to restart the character from scratch */
	memset(&mbs, 0, sizeof(mbs));
	ret = mbrtowc(&wc, bufptr, charlen, &mbs);
	if (ret == 0) {
	    /* NULL: unlikely, but handle anyway. */
	    if (last) {
		if (retchr)
		    *retchr = lastc;
		return last;
	    } else {
		if (retchr)
		    *retchr = wc;
		return ptr;
	    }
	}
	if (ret != (size_t)-1) {
	    if (ret < (size_t)charlen) {
		/* The last character didn't convert, so use it raw. */
		break;
	    }
	    if (!isset(COMBININGCHARS)) {
		if (retchr)
		    *retchr = wc;
		return ptr;
	    }
	    if (!IS_COMBINING(wc)) {
		/* not a combining character... */
		if (last) {
		    /*
		     * ... but we were looking for a suitable base character,
		     * test it.
		     */
		    if (IS_BASECHAR(wc)) {
			/*
			 * Yes, this will do.
			 */
			if (retchr)
			    *retchr = wc;
			return ptr;
		    } else {
			/* No, just return the first character we found */
			if (retchr)
			    *retchr = lastc;
			return last;
		    }
		}
		/* This is the first character, so just return it. */
		if (retchr)
		    *retchr = wc;
		return ptr;
	    }
	    if (!last) {
		/* still looking for the character immediately before ptr */
		last = ptr;
		lastc = wc;
	    }
	    /* searching for base character of combining character */
	    charlen = 0;
	    bufptr = buf + BMC_BUFSIZE;
	}
	/*
	 * Else keep scanning this character even if MB_INVALID:  we can't
	 * expect MB_INCOMPLETE to work when moving backwards.
	 */
    }
    /*
     * Found something we didn't like, was there a good character
     * immediately before ptr?
     */
    if (last) {
	if (retchr)
	    *retchr = lastc;
	return last;
    }
    /*
     * No, we couldn't find any good character, so just treat
     * the last unmetafied byte we found as a character.
     */
#endif
    if (endptr > start) {
	if (endptr > start - 1 && endptr[-2] == Meta)
	{
	    if (retchr)
		*retchr = (convchar_t)(endptr[-1] ^ 32);
	    return endptr - 2;
	}
	else
	{
	    if (retchr)
		*retchr = (convchar_t)endptr[-1];
	    return endptr - 1;
	}
    }
    if (retchr)
	*retchr = (convchar_t)0;
    return endptr;
}


/**/
int
beginningofline(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = endofline(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	int pos;

	if (zlecs == 0)
	    return 0;
	pos = zlecs;
	DECPOS(pos);
	if (zleline[pos] == '\n') {
	    zlecs = pos;
	    if (!zlecs)
		return 0;
	}

	/* works OK with combining chars since '\n' must be on its own */
	while (zlecs && zleline[zlecs - 1] != '\n')
	    zlecs--;
    }
    return 0;
}

/**/
int
endofline(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = beginningofline(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	if (zlecs >= zlell) {
	    zlecs = zlell;
	    return 0;
	}
	if ((zlecs += invicmdmode()) == zlell)
	    break;
	if (zleline[zlecs] == '\n')
	    if (++zlecs == zlell)
		return 0;
	while (zlecs != zlell && zleline[zlecs] != '\n')
	    zlecs++;
    }
    return 0;
}

/**/
int
beginningoflinehist(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = endoflinehist(args);
	zmult = n;
	return ret;
    }
    while (n) {
	int pos;

	if (zlecs == 0)
	    break;
	pos = zlecs;
	DECPOS(pos);
	if (zleline[pos] == '\n') {
	    zlecs = pos;
	    if (!pos)
		break;
	}

	/* works OK with combining chars since '\n' must be on its own */
	while (zlecs && zleline[zlecs - 1] != '\n')
	    zlecs--;
	n--;
    }
    if (n) {
	int m = zmult, ret;

	zmult = n;
	ret = uphistory(args);
	zmult = m;
	zlecs = 0;
	return ret;
    }
    return 0;
}

/**/
int
endoflinehist(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = beginningoflinehist(args);
	zmult = n;
	return ret;
    }
    while (n) {
	if (zlecs >= zlell) {
	    zlecs = zlell;
	    break;
	}
	if ((zlecs += invicmdmode()) == zlell)
	    break;
	if (zleline[zlecs] == '\n')
	    if (++zlecs == zlell)
		break;
	while (zlecs != zlell && zleline[zlecs] != '\n')
	    zlecs++;
	n--;
    }
    if (n) {
	int m = zmult, ret;

	zmult = n;
	ret = downhistory(args);
	zmult = m;
	return ret;
    }
    return 0;
}

/**/
int
forwardchar(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = backwardchar(args);
	zmult = n;
	return ret;
    }

    /*
     * If handling combining characters with the base character,
     * we skip over the whole set in one go, so need to check.
     */
    while (zlecs < zlell && n--)
	INCCS();
    return 0;
}

/**/
int
backwardchar(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = forwardchar(args);
	zmult = n;
	return ret;
    }

    while (zlecs > 0 && n--)
	DECCS();
    return 0;
}

/**/
int
setmarkcommand(UNUSED(char **args))
{
    if (zmult < 0) {
	region_active = 0;
	return 0;
    }
    mark = zlecs;
    region_active = 1;
    return 0;
}

/**/
int
exchangepointandmark(UNUSED(char **args))
{
    int x;

    if (zmult == 0) {
	region_active = 1;
	return 0;
    }
    x = mark;
    mark = zlecs;
    zlecs = x;
    if (zlecs > zlell)
	zlecs = zlell;
    if (zmult > 0)
	region_active = 1;
    return 0;
}

/**/
int
visualmode(UNUSED(char **args))
{
    if (virangeflag) {
	prefixflag = 1;
	zmod.flags &= ~MOD_LINE;
	zmod.flags |= MOD_CHAR;
	return 0;
    }
    switch (region_active) {
    case 1:
	region_active = 0;
	break;
    case 0:
	mark = zlecs;
	/* fall through */
    case 2:
	region_active = 1;
	break;
    }
    return 0;
}

/**/
int
visuallinemode(UNUSED(char **args))
{
    if (virangeflag) {
	prefixflag = 1;
	zmod.flags &= ~MOD_CHAR;
	zmod.flags |= MOD_LINE;
	return 0;
    }
    switch (region_active) {
    case 2:
	region_active = 0;
	break;
    case 0:
	mark = zlecs;
	/* fall through */
    case 1:
	region_active = 2;
	break;
    }
    return 0;
}

/**/
int
deactivateregion(UNUSED(char **args))
{
    region_active = 0;
    return 0;
}

/**/
int
vigotocolumn(UNUSED(char **args))
{
    int x, y, n = zmult;

    findline(&x, &y);
    if (n >= 0) {
	if (n)
	    n--;
	zlecs = x;
	while (zlecs < y && n--)
	    INCCS();
    } else {
	zlecs = y;
	n = -n;
	while (zlecs > x && n--)
	    DECCS();
    }
    return 0;
}

/**/
int
vimatchbracket(UNUSED(char **args))
{
    int ocs = zlecs, dir, ct;
    unsigned char oth, me;

    if ((zlecs == zlell || zleline[zlecs] == '\n') && zlecs > 0)
	DECCS();
    if (virangeflag)
	mark = zlecs;
  otog:
    if (zlecs == zlell || zleline[zlecs] == '\n') {
	zlecs = ocs;
	return 1;
    }
    switch (me = zleline[zlecs]) {
    case '{':
	dir = 1;
	oth = '}';
	break;
    case /*{*/ '}':
	dir = -1;
	oth = '{'; /*}*/
	break;
    case '(':
	dir = 1;
	oth = ')';
	break;
    case ')':
	dir = -1;
	oth = '(';
	break;
    case '[':
	dir = 1;
	oth = ']';
	break;
    case ']':
	dir = -1;
	oth = '[';
	break;
    default:
	INCCS();
	goto otog;
    }
    if (virangeflag && dir < 0)
	INCPOS(mark); /* include starting position when going backwards */
    ct = 1;
    while (zlecs >= 0 && zlecs < zlell && ct) {
	if (dir < 0)
	    DECCS();
	else
	    INCCS();
	if (zleline[zlecs] == oth)
	    ct--;
	else if (zleline[zlecs] == me)
	    ct++;
    }
    if (zlecs < 0 || zlecs >= zlell) {
	zlecs = ocs;
	return 1;
    } else if(dir > 0 && virangeflag)
	INCCS();
    return 0;
}

/**/
int
viforwardchar(char **args)
{
    int lim = findeol();
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = vibackwardchar(args);
	zmult = n;
	return ret;
    }
    if (invicmdmode() && !virangeflag)
	DECPOS(lim);
    if (zlecs >= lim)
	return 1;
    while (n-- && zlecs < lim)
	INCCS();
    return 0;
}

/**/
int
vibackwardchar(char **args)
{
    int n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = viforwardchar(args);
	zmult = n;
	return ret;
    }
    if (zlecs == findbol())
	return 1;
    while (n-- && zlecs > 0) {
	DECCS();
	if (zleline[zlecs] == '\n') {
	    zlecs++;
	    break;
	}
    }
    return 0;
}

/**/
int
viendofline(UNUSED(char **args))
{
    int oldcs = zlecs, n = zmult;

    if (n < 1)
	return 1;
    while(n--) {
	if (zlecs > zlell) {
	    zlecs = oldcs;
	    return 1;
	}
	zlecs = findeol() + 1;
    }
    DECCS();
    lastcol = 1<<30;
    return 0;
}

/**/
int
vibeginningofline(UNUSED(char **args))
{
    zlecs = findbol();
    return 0;
}

static ZLE_INT_T vfindchar;
static int vfinddir, tailadd;

/**/
int
vifindnextchar(char **args)
{
    if ((vfindchar = vigetkey()) != ZLEEOF) {
	vfinddir = 1;
	tailadd = 0;
	return vifindchar(0, args);
    }
    return 1;
}

/**/
int
vifindprevchar(char **args)
{
    if ((vfindchar = vigetkey()) != ZLEEOF) {
	vfinddir = -1;
	tailadd = 0;
	return vifindchar(0, args);
    }
    return 1;
}

/**/
int
vifindnextcharskip(char **args)
{
    if ((vfindchar = vigetkey()) != ZLEEOF) {
	vfinddir = 1;
	tailadd = -1;
	return vifindchar(0, args);
    }
    return 1;
}

/**/
int
vifindprevcharskip(char **args)
{
    if ((vfindchar = vigetkey()) != ZLEEOF) {
	vfinddir = -1;
	tailadd = 1;
	return vifindchar(0, args);
    }
    return 1;
}

/**/
int
vifindchar(int repeat, char **args)
{
    int ocs = zlecs, n = zmult;

    if (!vfinddir)
	return 1;
    if (n < 0) {
	int ret;
	zmult = -n;
	ret = virevrepeatfind(args);
	zmult = n;
	return ret;
    }
    if (repeat && tailadd != 0) {
	if (vfinddir > 0) {
	    if(zlecs < zlell && (ZLE_INT_T)zleline[zlecs+1] == vfindchar)
		INCCS();
	}
	else {
	    if(zlecs > 0 && (ZLE_INT_T)zleline[zlecs-1] == vfindchar)
		DECCS();
	}
    }
    while (n--) {
	do {
	    if (vfinddir > 0)
		INCCS();
	    else
		DECCS();
	} while (zlecs >= 0 && zlecs < zlell
	    && (ZLE_INT_T)zleline[zlecs] != vfindchar
	    && zleline[zlecs] != ZWC('\n'));
	if (zlecs < 0 || zlecs >= zlell || zleline[zlecs] == ZWC('\n')) {
	    zlecs = ocs;
	    return 1;
	}
    }
    if (tailadd > 0)
	INCCS();
    else if (tailadd < 0)
	DECCS();
    if (vfinddir == 1 && virangeflag)
	INCCS();
    return 0;
}

/**/
int
virepeatfind(char **args)
{
    return vifindchar(1, args);
}

/**/
int
virevrepeatfind(char **args)
{
    int ret;

    if (zmult < 0) {
	zmult = -zmult;
	ret = vifindchar(1, args);
	zmult = -zmult;
	return ret;
    }
    tailadd = -tailadd;
    vfinddir = -vfinddir;
    ret = vifindchar(1, args);
    vfinddir = -vfinddir;
    tailadd = -tailadd;
    return ret;
}

/**/
int
vifirstnonblank(UNUSED(char **args))
{
    zlecs = findbol();
    while (zlecs != zlell && ZC_iblank(zleline[zlecs]))
	INCCS();
    return 0;
}

/**/
int
visetmark(UNUSED(char **args))
{
    ZLE_INT_T ch;

    ch = getfullchar(0);
    if (ch < ZWC('a') || ch > ZWC('z'))
	return 1;
    ch -= ZWC('a');
    vimarkcs[ch] = zlecs;
    vimarkline[ch] = histline;
    return 0;
}

/**/
int
vigotomark(UNUSED(char **args))
{
    ZLE_INT_T ch;
    int *markcs, *markhist = 0;
    int oldcs = zlecs;
    int oldline = histline;
    int tmpcs, tmphist;

    ch = getfullchar(0);
    if (ch == ZWC('\'') || ch == ZWC('`')) {
	markhist = vimarkline + 26;
	markcs = vimarkcs + 26;
    } else if (ch == ZWC('.') && curchange->prev) {
	/* position cursor where it was after the last change. not exactly
	 * what vim does but close enough */
	tmpcs = curchange->prev->new_cs;
	tmphist = curchange->prev->hist;
	markcs = &tmpcs;
	markhist = &tmphist;
    } else if (ch >= ZWC('a') && ch <= ZWC('z')) {
	markhist = vimarkline + (ch - ZWC('a'));
	markcs = vimarkcs + (ch - ZWC('a'));
    } else
	return 1;
    if (markhist) {
	if (!*markhist)
	    return 1;
	if (histline != *markhist && !zle_goto_hist(*markhist, 0, 0)) {
	    *markhist = 0;
	    return 1;
	}
    }
    zlecs = *markcs;
    vimarkcs[26] = oldcs;
    vimarkline[26] = oldline;
    if (zlecs > zlell)
	zlecs = zlell;
    return 0;
}

/**/
int
vigotomarkline(char **args)
{
    vigotomark(args);
    return vifirstnonblank(zlenoargs);
}
