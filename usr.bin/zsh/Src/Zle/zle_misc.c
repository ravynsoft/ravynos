/*
 * zle_misc.c - miscellaneous editor routines
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
#include "zle_misc.pro"

/* insert a zle string, with repetition and suffix removal */

/**/
void
doinsert(ZLE_STRING_T zstr, int len)
{
    ZLE_STRING_T s;
    ZLE_CHAR_T c1 = *zstr;	     /* first character */
    int neg = zmult < 0;             /* insert *after* the cursor? */
    int m = neg ? -zmult : zmult;    /* number of copies to insert */
    int count;

    UNMETACHECK();

    iremovesuffix(c1, 0);
    invalidatelist();

    /* In overwrite mode, don't replace newlines. */
    if (insmode || zleline[zlecs] == ZWC('\n'))
	spaceinline(m * len);
    else
    {
	int pos = zlecs, diff, i;
#ifdef MULTIBYTE_SUPPORT
	/*
	 * Calculate the number of character positions we are
	 * going to be using.  The algorithm is that
	 * anything that shows up as a logical single character
	 * (i.e. even if control, or double width, or with combining
	 * characters) is treated as 1 for the purpose of replacing
	 * what's there already.
	 *
	 * This can cause inserting of a combining character in
	 * places where it should overwrite, such as the start
	 * of a line.  However, combining characters aren't
	 * useful there anyway and this doesn't cause any
	 * particular harm.
	 */
	for (i = 0, count = 0; i < len * m; i++) {
	    if (!IS_COMBINING(zstr[i]))
		count++;
	}
#else
	count = len * m;
#endif
	/*
	 * Ensure we replace a complete combining characterfor each
	 * character we overwrite. Switch to inserting at first newline.
	 */
	for (i = count; pos < zlell && zleline[pos] != ZWC('\n') && i--; ) {
	    INCPOS(pos);
	}
	/*
	 * Calculate how many raw line places we need.
	 * pos - zlecs is the raw line distance we're replacing,
	 * m * len the number we're inserting.
	 */
	diff = pos - zlecs - m * len;
	if (diff < 0) {
	    spaceinline(-diff);
	} else if (diff > 0) {
	    /*
	     * We use shiftchars() here because we don't
	     * want combining char alignment fixed up: we
	     * are going to write over any that remain.
	     */
	    shiftchars(zlecs, diff);
	}
    }
    while (m--)
	for (s = zstr, count = len; count; s++, count--)
	    zleline[zlecs++] = *s;
    if (neg)
	zlecs += zmult * len;
    /* if we ended up on a combining character, skip over it */
    CCRIGHT();
}

/**/
mod_export int
selfinsert(UNUSED(char **args))
{
    ZLE_CHAR_T tmp;

#ifdef MULTIBYTE_SUPPORT
    /* may be redundant with getkeymapcmd(), but other widgets call here too */
    if (!lastchar_wide_valid)
	if (getrestchar(lastchar, NULL, NULL) == WEOF)
	    return 1;
#endif
    tmp = LASTFULLCHAR;
    doinsert(&tmp, 1);
    return 0;
}

/**/
mod_export void
fixunmeta(void)
{
    lastchar &= 0x7f;
    if (lastchar == '\r')
	lastchar = '\n';
#ifdef MULTIBYTE_SUPPORT
    /*
     * TODO: can we do this better?
     * We need a wide character to insert.
     * selfinsertunmeta is intrinsically problematic
     * with multibyte input.
     */
    lastchar_wide = (ZLE_INT_T)lastchar;
    lastchar_wide_valid = 1;
#endif
}

/**/
mod_export int
selfinsertunmeta(char **args)
{
    fixunmeta();
    return selfinsert(args);
}

/**/
int
deletechar(char **args)
{
    int n;
    if (zmult < 0) {
	int ret;
	zmult = -zmult;
	ret = backwarddeletechar(args);
	zmult = -zmult;
	return ret;
    }

    n = zmult;
    while (n--) {
	if (zlecs == zlell)
	    return 1;
	INCCS();
    }
    backdel(zmult, 0);
    return 0;
}

/**/
int
backwarddeletechar(char **args)
{
    if (zmult < 0) {
	int ret;
	zmult = -zmult;
	ret = deletechar(args);
	zmult = -zmult;
	return ret;
    }
    backdel(zmult > zlecs ? zlecs : zmult, 0);
    return 0;
}

/**/
int
killwholeline(UNUSED(char **args))
{
    int i, fg, n = zmult;

    if (n < 0)
	return 1;
    while (n--) {
	if ((fg = (zlecs && zlecs == zlell)))
	    zlecs--;
	while (zlecs && zleline[zlecs - 1] != '\n')
	    zlecs--;
	for (i = zlecs; i != zlell && zleline[i] != '\n'; i++);
	forekill(i - zlecs + (i != zlell), fg ? (CUT_FRONT|CUT_RAW) : CUT_RAW);
    }
    clearlist = 1;
    return 0;
}

/**/
int
killbuffer(UNUSED(char **args))
{
    zlecs = 0;
    forekill(zlell, CUT_RAW);
    clearlist = 1;
    return 0;
}

/**/
int
backwardkillline(char **args)
{
    int i = 0, n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = killline(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	if (zlecs && zleline[zlecs - 1] == '\n')
	    zlecs--, i++;
	else
	    while (zlecs && zleline[zlecs - 1] != '\n')
		zlecs--, i++;
    }
    forekill(i, CUT_FRONT|CUT_RAW);
    clearlist = 1;
    return 0;
}

#ifdef MULTIBYTE_SUPPORT
/*
 * Transpose the chunk of the line from start to middle with
 * that from middle to end.
 */

static void
transpose_swap(int start, int middle, int end)
{
    int len1, len2;
    ZLE_STRING_T first;

    len1 = middle - start;
    len2 = end - middle;

    first = (ZLE_STRING_T)zalloc(len1 * ZLE_CHAR_SIZE);
    ZS_memcpy(first, zleline + start, len1);
    /* Move may be overlapping... */
    ZS_memmove(zleline + start, zleline + middle, len2);
    ZS_memcpy(zleline + start + len2, first, len1);
    zfree(first, len1 * ZLE_CHAR_SIZE);
}
#endif

/**/
int
gosmacstransposechars(UNUSED(char **args))
{
    if (zlecs < 2 || zleline[zlecs - 1] == '\n' || zleline[zlecs - 2] == '\n') {
	int twice = (zlecs == 0 || zleline[zlecs - 1] == '\n');

	if (zlecs == zlell || zleline[zlecs] == '\n')
	    return 1;

	INCCS();
	if (twice) {
	    if (zlecs == zlell || zleline[zlecs] == '\n')
		return 1;
	    INCCS();
	}
    }
#ifdef MULTIBYTE_SUPPORT
    {
	int start, middle;

	middle = zlecs;
	DECPOS(middle);

	start = middle;
	DECPOS(start);

	transpose_swap(start, middle, zlecs);
    }
#else
    {
	ZLE_CHAR_T cc = zleline[zlecs - 2];
	zleline[zlecs - 2] = zleline[zlecs - 1];
	zleline[zlecs - 1] = cc;
    }
#endif
    return 0;
}

/**/
int
transposechars(UNUSED(char **args))
{
    int ct;
    int n = zmult;
    int neg = n < 0;

    if (neg)
	n = -n;
    while (n--) {
	if (!(ct = zlecs) || zleline[zlecs - 1] == '\n') {
	    if (zlell == zlecs || zleline[zlecs] == '\n')
		return 1;
	    if (!neg)
		INCCS();
	    INCPOS(ct);
	}
	if (neg) {
	    if (zlecs && zleline[zlecs - 1] != '\n') {
		DECCS();
		if (ct > 1 && zleline[ct - 2] != '\n') {
		    DECPOS(ct);
		}
	    }
	} else {
	    if (zlecs != zlell && zleline[zlecs] != '\n')
		INCCS();
	}
	if (ct == zlell || zleline[ct] == '\n') {
	    DECPOS(ct);
	}
	if (ct < 1 || zleline[ct - 1] == '\n')
	    return 1;
#ifdef MULTIBYTE_SUPPORT
	{
	    /*
	     * We should keep any accents etc. on their original characters.
	     */
	    int start = ct, end = ct;
	    DECPOS(start);
	    INCPOS(end);

	    transpose_swap(start, ct, end);
	}
#else
	{
	    ZLE_CHAR_T cc = zleline[ct - 1];
	    zleline[ct - 1] = zleline[ct];
	    zleline[ct] = cc;
	}
#endif
    }
    return 0;
}

/**/
int
poundinsert(UNUSED(char **args))
{
    zlecs = 0;
    vifirstnonblank(zlenoargs);
    if (zleline[zlecs] != '#') {
	spaceinline(1);
	zleline[zlecs] = '#';
	zlecs = findeol();
	while(zlecs != zlell) {
	    zlecs++;
	    vifirstnonblank(zlenoargs);
	    spaceinline(1);
	    zleline[zlecs] = '#';
	    zlecs = findeol();
	}
    } else {
	foredel(1, 0);
	zlecs = findeol();
	while(zlecs != zlell) {
	    zlecs++;
	    vifirstnonblank(zlenoargs);
	    if(zleline[zlecs] == '#')
		foredel(1, 0);
	    zlecs = findeol();
	}
    }
    done = 1;
    return 0;
}

/**/
int
acceptline(UNUSED(char **args))
{
    done = 1;
    return 0;
}

/**/
int
acceptandhold(UNUSED(char **args))
{
    zpushnode(bufstack, zlelineasstring(zleline, zlell, 0, NULL, NULL, 0));
    stackcs = zlecs;
    done = 1;
    return 0;
}

/**/
int
killline(char **args)
{
    int i = 0, n = zmult;

    if (n < 0) {
	int ret;
	zmult = -n;
	ret = backwardkillline(args);
	zmult = n;
	return ret;
    }
    while (n--) {
	if (zleline[zlecs] == ZWC('\n'))
	    zlecs++, i++;
	else
	    while (zlecs != zlell && zleline[zlecs] != ZWC('\n'))
		zlecs++, i++;
    }
    backkill(i, CUT_RAW);
    clearlist = 1;
    return 0;
}

/**/
void
regionlines(int *start, int *end)
{
    int origcs = zlecs;

    UNMETACHECK();
    if (zlecs < mark) {
	*start = findbol();
        zlecs = (mark > zlell) ? zlell : mark;
	*end = findeol();
    } else {
	*end = findeol();
        zlecs = mark;
	*start = findbol();
    }
    zlecs = origcs;
}

/**/
int
killregion(UNUSED(char **args))
{
    if (mark > zlell)
	mark = zlell;
    if (region_active == 2) {
	int a, b;
	regionlines(&a, &b);
	zlecs = a;
	region_active = 0;
	cut(zlecs, b - zlecs, CUT_RAW);
	shiftchars(zlecs, b - zlecs);
	if (zlell) {
	    if (zlecs == zlell)
		DECCS();
	    foredel(1, 0);
	    vifirstnonblank(zlenoargs);
	}
    } else if (mark > zlecs) {
	if (invicmdmode())
	    INCPOS(mark);
	forekill(mark - zlecs, CUT_RAW);
    } else {
	if (invicmdmode())
	    INCCS();
	backkill(zlecs - mark, CUT_FRONT|CUT_RAW);
    }
    return 0;
}

/**/
int
copyregionaskill(char **args)
{
    int start, end;
    if (*args) {
        int len;
        ZLE_STRING_T line = stringaszleline(*args, 0, &len, NULL, NULL);
	cuttext(line, len, CUT_REPLACE);
	free(line);
    } else {
	if (mark > zlell)
	    mark = zlell;
	if (mark > zlecs) {
	    start = zlecs;
	    end = mark;
	} else {
	    start = mark;
	    end = zlecs;
	}
	if (invicmdmode())
	    INCPOS(end);
	cut(start, end - start, mark > zlecs ? 0 : CUT_FRONT);
    }
    return 0;
}

/*
 * kct: index into kill ring, or -1 for original cutbuffer of yank.
 * yankcs marks the cursor position preceding the last yank
 */
static int kct, yankcs;

/**/
int yankb, yanke; /* mark the start and end of last yank in editing buffer. */

/* The original cutbuffer, either cutbuf or one of the vi buffers. */
static Cutbuffer kctbuf;

/**/
int
yank(UNUSED(char **args))
{
    int n = zmult;

    if (n < 0)
	return 1;
    if (zmod.flags & MOD_VIBUF)
	kctbuf = &vibuf[zmod.vibuf];
    else
	kctbuf = &cutbuf;
    if (!kctbuf->buf)
	return 1;
    yankb = yankcs = mark = zlecs;
    while (n--) {
	kct = -1;
	spaceinline(kctbuf->len);
	ZS_memcpy(zleline + zlecs, kctbuf->buf, kctbuf->len);
	zlecs += kctbuf->len;
	yanke = zlecs;
    }
    return 0;
}

/* position: 0 is before, 1 after, 2 split the line */
static void pastebuf(Cutbuffer buf, int mult, int position)
{
    int cc;
    if (buf->flags & CUTBUFFER_LINE) {
	if (position == 2) {
	    if (!zlecs)
		position = 0;
	    else if (zlecs == zlell)
		position = 1;
	}
	if (position == 2) {
	    yankb = zlecs;
	    spaceinline(buf->len + 2);
	    zleline[zlecs++] = ZWC('\n');
	    ZS_memcpy(zleline + zlecs, buf->buf, buf->len);
	    zlecs += buf->len;
	    zleline[zlecs] = ZWC('\n');
	    yanke = zlecs + 1;
	} else if (position != 0) {
	    yankb = zlecs = findeol();
	    spaceinline(buf->len + 1);
	    zleline[zlecs++] = ZWC('\n');
	    yanke = zlecs + buf->len;
	    ZS_memcpy(zleline + zlecs, buf->buf, buf->len);
	} else {
	    yankb = zlecs = findbol();
	    spaceinline(buf->len + 1);
	    ZS_memcpy(zleline + zlecs, buf->buf, buf->len);
	    yanke = zlecs + buf->len + 1;
	    zleline[zlecs + buf->len] = ZWC('\n');
	}
	vifirstnonblank(zlenoargs);
    } else {
	if (position == 1 && zlecs != findeol())
	    INCCS();
	yankb = zlecs;
	cc = buf->len;
	while (mult--) {
	    spaceinline(cc);
	    ZS_memcpy(zleline + zlecs, buf->buf, cc);
	    zlecs += cc;
	}
	yanke = zlecs;
	if (zlecs && invicmdmode())
	    DECCS();
    }
}

/**/
int
viputbefore(UNUSED(char **args))
{
    int n = zmult;

    startvichange(-1);
    if (n < 0)
	return 1;
    if (zmod.flags & MOD_NULL)
	return 0;
    if (zmod.flags & MOD_VIBUF)
	kctbuf = &vibuf[zmod.vibuf];
    else
	kctbuf = &cutbuf;
    if (!kctbuf->buf)
	return 1;
    kct = -1;
    yankcs = zlecs;
    pastebuf(kctbuf, n, 0);
    return 0;
}

/**/
int
viputafter(UNUSED(char **args))
{
    int n = zmult;

    startvichange(-1);
    if (n < 0)
	return 1;
    if (zmod.flags & MOD_NULL)
	return 0;
    if (zmod.flags & MOD_VIBUF)
	kctbuf = &vibuf[zmod.vibuf];
    else
	kctbuf = &cutbuf;
    if (!kctbuf->buf)
	return 1;
    kct = -1;
    yankcs = zlecs;
    pastebuf(kctbuf, n, 1);
    return 0;
}

/**/
int
putreplaceselection(UNUSED(char **args))
{
    int n = zmult;
    struct cutbuffer prevbuf;
    Cutbuffer putbuf;
    int clear = 0;
    int pos = 2;

    startvichange(-1);
    if (n < 0 || zmod.flags & MOD_NULL)
	return 1;
    putbuf = (zmod.flags & MOD_VIBUF) ? &vibuf[zmod.vibuf] : &cutbuf;
    if (!putbuf->buf)
	return 1;
    memcpy(&prevbuf, putbuf, sizeof(prevbuf));

    /* if "9 was specified, prevent killregion from freeing it */
    if (zmod.vibuf == 35) {
	putbuf->buf = 0;
	clear = 1;
    }

    zmod.flags = 0; /* flags apply to paste not kill */
    if (region_active == 2 && prevbuf.flags & CUTBUFFER_LINE) {
	int a, b;
	regionlines(&a, &b);
	pos = (b == zlell);
    }
    killregion(zlenoargs);

    pastebuf(&prevbuf, n, pos);
    if (clear)
	free(prevbuf.buf);
    return 0;
}

/**/
int
yankpop(UNUSED(char **args))
{
    int kctstart = kct;
    Cutbuffer buf;

    if (!(lastcmd & ZLE_YANK) || !kring || !kctbuf) {
	kctbuf = NULL;
	return 1;
    }
    do {
	/*
	 * This is supposed to make the yankpop loop
	 *   original buffer -> kill ring in order -> original buffer -> ...
	 * where the original buffer is -1 and the remainder are
	 * indices into the kill ring, remember that we need to start
	 * that at kringnum rather than zero.
	 */
	if (kct == -1)
	    kct = kringnum;
	else {
	    int kctnew = (kct + kringsize - 1) % kringsize;
	    if (kctnew == kringnum)
		kct = -1;
	    else
		kct = kctnew;
	}
	if (kct == -1)
	    buf = kctbuf;	/* Use original cutbuffer */
	else
	    buf = kring+kct;	/* Use somewhere in the kill ring */
	/* Careful to prevent infinite looping */
	if (kct == kctstart)
	    return 1;
	/*
	 * Skip unset buffers instead of stopping as we used to do.
	 * Also skip zero-length buffers.
	 * There are two reasons for this:
	 * 1. We now map the array $killring directly into the
	 *    killring, instead of via some extra size-checking logic.
	 *    When $killring has been set, a buffer will always have
	 *    at least a zero-length string in it.
	 * 2. The old logic was inconsistent; when the kill ring
	 *    was full, we could loop round and round it, otherwise
	 *    we just stopped when we hit the first empty buffer.
	 */
    } while (!buf->buf || *buf->buf == ZWC('\0'));

    zlecs = yankb;
    foredel(yanke - yankb, CUT_RAW);
    zlecs = yankcs;
    pastebuf(buf, 1, !!(lastcmd & ZLE_YANKAFTER));
    return 0;
}

/**/
mod_export char *
bracketedstring(void)
{
    static const char endesc[] = "\033[201~";
    int endpos = 0;
    size_t psize = 64;
    char *pbuf = zalloc(psize);
    size_t current = 0;
    int next, timeout;

    while (endesc[endpos]) {
	if (current + 1 >= psize)
	    pbuf = zrealloc(pbuf, psize *= 2);
	if ((next = getbyte(1L, &timeout, 1)) == EOF)
	    break;
	if (!endpos || next != endesc[endpos++])
	    endpos = (next == *endesc);
	if (imeta(next)) {
	    pbuf[current++] = Meta;
	    pbuf[current++] = next ^ 32;
	} else if (next == '\r')
	    pbuf[current++] = '\n';
	else
	    pbuf[current++] = next;
    }
    pbuf[current-endpos] = '\0';
    return pbuf;
}

/**/
int
bracketedpaste(char **args)
{
    char *pbuf = bracketedstring();

    if (*args) {
	setsparam(*args, pbuf);
    } else {
	int n;
	ZLE_STRING_T wpaste;
	wpaste = stringaszleline((zmult == 1) ? pbuf :
	    quotestring(pbuf, QT_SINGLE_OPTIONAL), 0, &n, NULL, NULL);
	cuttext(wpaste, n, CUT_REPLACE);
	if (!(zmod.flags & MOD_VIBUF)) {
	    kct = -1;
	    kctbuf = &cutbuf;
	    zmult = 1;
	    if (region_active)
		killregion(zlenoargs);
	    yankcs = yankb = zlecs;
	    doinsert(wpaste, n);
	    yanke = zlecs;
	}
	free(pbuf); free(wpaste);
    }
    return 0;
}

/**/
int
overwritemode(UNUSED(char **args))
{
    insmode ^= 1;
    return 0;
}

/**/
int
whatcursorposition(UNUSED(char **args))
{
    char msg[100];
    char *s = msg, *mbstr;
    int bol = findbol(), len;
    ZLE_CHAR_T c = zleline[zlecs];

    if (zlecs == zlell)
	strucpy(&s, "EOF");
    else {
	strucpy(&s, "Char: ");
	switch (c) {
	case ZWC(' '):
	    strucpy(&s, "SPC");
	    break;
	case ZWC('\t'):
	    strucpy(&s, "TAB");
	    break;
	case ZWC('\n'):
	    strucpy(&s, "LFD");
	    break;
	default:
	    /*
	     * convert a single character, remembering it may
	     * turn into a multibyte string or be metafied.
	     */
	    mbstr = zlelineasstring(zleline+zlecs, 1, 0, &len, NULL, 1);
	    strcpy(s, mbstr);
	    s += len;
	}
	sprintf(s, " (0%o, %u, 0x%x)", (unsigned int)c,
		(unsigned int)c, (unsigned int)c);
	s += strlen(s);
    }
    sprintf(s, "  point %d of %d(%d%%)  column %d", zlecs+1, zlell+1,
	    zlell ? 100 * zlecs / zlell : 0, zlecs - bol);
    showmsg(msg);
    return 0;
}

/**/
int
undefinedkey(UNUSED(char **args))
{
    return 1;
}

/**/
int
quotedinsert(char **args)
{
#ifndef HAS_TIO
    struct sgttyb sob;

    sob = shttyinfo.sgttyb;
    sob.sg_flags = (sob.sg_flags | RAW) & ~ECHO;
    ioctl(SHTTY, TIOCSETN, &sob);
#endif
    getfullchar(0);
#ifndef HAS_TIO
    zsetterm();
#endif
    if (LASTFULLCHAR == ZLEEOF)
	return 1;
    else
	return selfinsert(args);
}

static int
parsedigit(int inkey)
{
#ifdef MULTIBYTE_SUPPORT
    /*
     * It's too dangerous to allow metafied input.  See
     * universalargument for comments on (possibly suboptimal) handling
     * of digits.  We are assuming ASCII is a subset of the multibyte
     * encoding.
     */
#else
    /* allow metafied as well as ordinary digits */
    inkey &= 0x7f;
#endif

    /* remember inkey is not a wide character */
    if (zmod.base > 10) {
	if (inkey >= 'a' && inkey < 'a' + zmod.base - 10)
	    return inkey - 'a' + 10;
	if (inkey >= 'A' && inkey < 'A' + zmod.base - 10)
	    return inkey - 'A' + 10;
	if (idigit(inkey))
	    return inkey - '0';
	return -1;
    }
    if (inkey >= '0' && inkey < '0' + zmod.base)
	return inkey - '0';
    return -1;
}

/**/
int
digitargument(UNUSED(char **args))
{
    int sign = (zmult < 0) ? -1 : 1;
    int newdigit = parsedigit(lastchar);

    if (newdigit < 0)
	return 1;

    if (!(zmod.flags & MOD_TMULT))
	zmod.tmult = 0;
    if (zmod.flags & MOD_NEG) {
	/* If we just had a negative argument, this is the digit, *
	 * rather than the -1 assumed by negargument()            */
	zmod.tmult = sign * newdigit;
	zmod.flags &= ~MOD_NEG;
    } else
	zmod.tmult = zmod.tmult * zmod.base + sign * newdigit;
    zmod.flags |= MOD_TMULT;
    prefixflag = 1;
    return 0;
}

/**/
int
negargument(UNUSED(char **args))
{
    if (zmod.flags & MOD_TMULT)
	return 1;
    zmod.tmult = -1;
    zmod.flags |= MOD_TMULT|MOD_NEG;
    prefixflag = 1;
    return 0;
}

/**/
int
universalargument(char **args)
{
    int digcnt = 0, pref = 0, minus = 1, gotk;
    if (*args) {
	zmod.mult = atoi(*args);
	zmod.flags |= MOD_MULT;
	return 0;
    }
    /*
     * TODO: this is quite tricky to do when trying to maintain
     * compatibility between the old input system and Unicode.
     * We don't know what follows the digits, so if we try to
     * read wide characters we may fail (e.g. we may come across an old
     * \M-style binding).
     *
     * If we assume individual bytes are either explicitly ASCII or
     * not (a la UTF-8), we get away with it; we can back up individual
     * bytes and everything will work.  We may want to relax this
     * assumption later.  ("Much later" - (C) Steven Singer,
     * CSR BlueCore firmware, ca. 2000.)
     *
     * Hence for now this remains byte-by-byte.
     */
    while ((gotk = getbyte(0L, NULL, 1)) != EOF) {
	if (gotk == '-' && !digcnt) {
	    minus = -1;
	    digcnt++;
	} else {
	    int newdigit = parsedigit(gotk);

	    if (newdigit >= 0) {
		pref = pref * zmod.base + newdigit;
		digcnt++;
	    } else {
		ungetbyte(gotk);
		break;
	    }
	}
    }
    if (digcnt)
	zmod.tmult = minus * (pref ? pref : 1);
    else
	zmod.tmult *= 4;
    zmod.flags |= MOD_TMULT;
    prefixflag = 1;
    return 0;
}

/* Set the base for a digit argument. */

/**/
int
argumentbase(char **args)
{
    int multbase;

    if (*args)
	multbase = (int)zstrtol(*args, NULL, 0);
    else
	multbase = zmod.mult;

    if (multbase < 2 || multbase > ('9' - '0' + 1) + ('z' - 'a' + 1))
	return 1;

    zmod.base = multbase;

    /* reset modifier, apart from base... */
    zmod.flags = 0;
    zmod.mult = 1;
    zmod.tmult = 1;
    zmod.vibuf = 0;

    /* ...but indicate we are still operating on a prefix argument. */
    prefixflag = 1;

    return 0;
}

/**/
int
copyprevword(UNUSED(char **args))
{
    int len, t0 = zlecs, t1;

    if (zmult > 0) {
	int count = zmult;

	for (;;) {
	    t1 = t0;

	    while (t0) {
		int prev = t0;
		DECPOS(prev);
		if (ZC_iword(zleline[prev]))
		    break;
		t0 = prev;
	    }
	    while (t0) {
		int prev = t0;
		DECPOS(prev);
		if (!ZC_iword(zleline[prev]))
		    break;
		t0 = prev;
	    }

	    if (!--count)
		break;
	    if (t0 == 0)
		return 1;
	}
    }
    else
	return 1;
    len = t1 - t0;
    spaceinline(len);
    ZS_memcpy(zleline + zlecs, zleline + t0, len);
    zlecs += len;
    return 0;
}

/**/
int
copyprevshellword(UNUSED(char **args))
{
    LinkList l;
    LinkNode n;
    int i;
    char *p = NULL;

    if (zmult <= 0)
	return 1;

    if ((l = bufferwords(NULL, NULL, &i, LEXFLAGS_ZLE))) {
	i -= (zmult-1);
	if (i < 0)
	    return 1;
        for (n = firstnode(l); n; incnode(n))
            if (!i--) {
                p = getdata(n);
                break;
            }
    }

    if (p) {
	int len;
	ZLE_STRING_T lineadd = stringaszleline(p, 0, &len, NULL, NULL);

	spaceinline(len);
	ZS_memcpy(zleline + zlecs, lineadd, len);
	zlecs += len;

	free(lineadd);
    }
    return 0;
}

/**/
int
sendbreak(UNUSED(char **args))
{
    errflag |= ERRFLAG_ERROR|ERRFLAG_INT;
    return 1;
}

/**/
int
quoteregion(UNUSED(char **args))
{
    ZLE_STRING_T str;
    size_t len;
    int extra = invicmdmode();

    if (mark > zlell)
	mark = zlell;
    if (region_active == 2) {
	int a, b;
	regionlines(&a, &b);
	zlecs = a;
	mark = b;
	extra = 0;
    } else if (mark < zlecs) {
	int tmp = mark;
	mark = zlecs;
	zlecs = tmp;
    }
    if (extra)
	INCPOS(mark);
    str = (ZLE_STRING_T)hcalloc((len = mark - zlecs) *
	ZLE_CHAR_SIZE);
    ZS_memcpy(str, zleline + zlecs, len);
    foredel(len, CUT_RAW);
    str = makequote(str, &len);
    spaceinline(len);
    ZS_memcpy(zleline + zlecs, str, len);
    mark = zlecs;
    zlecs += len;
    return 0;
}

/**/
int
quoteline(UNUSED(char **args))
{
    ZLE_STRING_T str;
    size_t len = zlell;

    str = makequote(zleline, &len);
    sizeline(len);
    ZS_memcpy(zleline, str, len);
    zlecs = zlell = len;
    return 0;
}

/**/
static ZLE_STRING_T
makequote(ZLE_STRING_T str, size_t *len)
{
    int qtct = 0;
    ZLE_STRING_T l, ol;
    ZLE_STRING_T end = str + *len;

    for (l = str; l < end; l++)
	if (*l == ZWC('\''))
	    qtct++;
    *len += 2 + qtct*3;
    l = ol = (ZLE_STRING_T)zhalloc(*len * ZLE_CHAR_SIZE);
    *l++ = ZWC('\'');
    for (; str < end; str++)
	if (*str == ZWC('\'')) {
	    *l++ = ZWC('\'');
	    *l++ = ZWC('\\');
	    *l++ = ZWC('\'');
	    *l++ = ZWC('\'');
	} else
	    *l++ = *str;
    *l++ = ZWC('\'');
    return ol;
}

/*
 * cmdstr is the buffer used for execute-named-command converted
 * to a metafied multibyte string.
 */
static char *namedcmdstr;
static LinkList namedcmdll;
static int namedcmdambig;

/**/
static void
scancompcmd(HashNode hn, UNUSED(int flags))
{
    int l;
    Thingy t = (Thingy) hn;

    if(strpfx(namedcmdstr, t->nam)) {
	addlinknode(namedcmdll, t->nam);
	l = pfxlen(peekfirst(namedcmdll), t->nam);
	if (l < namedcmdambig)
	    namedcmdambig = l;
    }

}

#define NAMLEN 60

/*
 * Local keymap used when reading a command name for the
 * execute-named-command and where-is widgets.
 */

/**/
Keymap command_keymap;

/**/
Thingy
executenamedcommand(char *prmt)
{
    Thingy cmd, retval = NULL;
    int l, len, feep = 0, listed = 0, curlist = 0;
    int ols = (listshown && validlist), olll = lastlistlen;
    char *cmdbuf, *ptr;
    char *okeymap = ztrdup(curkeymapname);

    clearlist = 1;
    /* prmt may be constant */
    prmt = ztrdup(prmt);
    l = strlen(prmt);
    cmdbuf = (char *)zhalloc(l + NAMLEN + 2
#ifdef MULTIBYTE_SUPPORT
			     + 2 * MB_CUR_MAX
#endif
			     );
    strcpy(cmdbuf, prmt);
    zsfree(prmt);
    statusline = cmdbuf;
    selectlocalmap(command_keymap);
    selectkeymap("main", 1);
    ptr = cmdbuf += l;
    len = 0;
    for (;;) {
	*ptr = '_';
	ptr[1] = '\0';
	zrefresh();
	if (!(cmd = getkeycmd()) || cmd == Th(z_sendbreak)) {
	    statusline = NULL;
	    selectkeymap(okeymap, 1);
	    zsfree(okeymap);
	    if ((listshown = ols)) {
		showinglist = -2;
		lastlistlen = olll;
	    } else if (listed)
		clearlist = listshown = 1;

	    retval = NULL;
	    goto done;
	}
	if(cmd == Th(z_clearscreen)) {
	    clearscreen(zlenoargs);
	    if (curlist) {
		int zmultsav = zmult;

		zmult = 1;
		listlist(namedcmdll);
		showinglist = 0;
		zmult = zmultsav;
	    }
	} else if(cmd == Th(z_redisplay)) {
	    redisplay(zlenoargs);
	    if (curlist) {
		int zmultsav = zmult;

		zmult = 1;
		listlist(namedcmdll);
		showinglist = 0;
		zmult = zmultsav;
	    }
	} else if(cmd == Th(z_viquotedinsert)) {
	    *ptr = '^';
	    zrefresh();
	    getfullchar(0);
	    if(LASTFULLCHAR == ZLEEOF || !LASTFULLCHAR || len >= NAMLEN)
		feep = 1;
	    else {
		int ret = zlecharasstring(LASTFULLCHAR, ptr);
		len += ret;
		ptr += ret;
		curlist = 0;
	    }
	} else if(cmd == Th(z_quotedinsert)) {
	    if(getfullchar(0) == ZLEEOF ||
	       !LASTFULLCHAR || len == NAMLEN)
		feep = 1;
	    else {
		int ret = zlecharasstring(LASTFULLCHAR, ptr);
		len += ret;
		ptr += ret;
		curlist = 0;
	    }
	} else if(cmd == Th(z_backwarddeletechar) ||
		  cmd == Th(z_vibackwarddeletechar)) {
	    if (len) {
		ptr = backwardmetafiedchar(cmdbuf, ptr, NULL);
		len = ptr - cmdbuf;
		curlist = 0;
	    }
	} else if(cmd == Th(z_killregion) || cmd == Th(z_backwardkillword) ||
		  cmd == Th(z_vibackwardkillword)) {
	    if (len)
		curlist = 0;
	    while (len) {
		convchar_t cc;
		ptr = backwardmetafiedchar(cmdbuf, ptr, &cc);
		len = ptr - cmdbuf;
		if (cc == ZWC('-'))
		    break;
	    }
	} else if(cmd == Th(z_killwholeline) || cmd == Th(z_vikillline) ||
	    	cmd == Th(z_backwardkillline)) {
	    len = 0;
	    ptr = cmdbuf;
	    if (listed)
		clearlist = listshown = 1;
	    curlist = 0;
	} else if (cmd == Th(z_bracketedpaste)) {
	    char *insert = bracketedstring();
	    size_t inslen = strlen(insert);
	    if (len + inslen > NAMLEN)
		feep = 1;
	    else {
		strcpy(ptr, insert);
		len += inslen;
		ptr += inslen;
		if (listed) {
		    clearlist = listshown = 1;
		    listed = 0;
		} else
		    curlist = 0;
	    }
	    free(insert);
	} else {
	    if(cmd == Th(z_acceptline) || cmd == Th(z_vicmdmode)) {
		Thingy r;
		unambiguous:
		*ptr = 0;
		r = rthingy(cmdbuf);
		if (!(r->flags & DISABLED)) {
		    unrefthingy(r);
		    statusline = NULL;
		    selectkeymap(okeymap, 1);
		    zsfree(okeymap);
		    if ((listshown = ols)) {
			showinglist = -2;
			lastlistlen = olll;
		    } else if (listed)
			clearlist = listshown = 1;

		    retval = r;
		    goto done;
		}
		unrefthingy(r);
	    }
	    if(cmd == Th(z_selfinsertunmeta)) {
		fixunmeta();
		cmd = Th(z_selfinsert);
	    }
	    if (cmd == Th(z_listchoices) || cmd == Th(z_deletecharorlist) ||
		cmd == Th(z_expandorcomplete) || cmd == Th(z_completeword) ||
		cmd == Th(z_expandorcompleteprefix) || cmd == Th(z_vicmdmode) ||
		cmd == Th(z_acceptline) || lastchar == ' ' || lastchar == '\t') {
		namedcmdambig = 100;

		namedcmdll = newlinklist();

		*ptr = '\0';
		namedcmdstr = cmdbuf;
		scanhashtable(thingytab, 1, 0, DISABLED, scancompcmd, 0);
		namedcmdstr = NULL;

		if (empty(namedcmdll)) {
		    feep = 1;
		    if (listed)
			clearlist = listshown = 1;
		    curlist = 0;
		} else if (cmd == Th(z_listchoices) ||
		    cmd == Th(z_deletecharorlist)) {
		    int zmultsav = zmult;
		    *ptr = '_';
		    ptr[1] = '\0';
		    zmult = 1;
		    listlist(namedcmdll);
		    listed = curlist = 1;
		    showinglist = 0;
		    zmult = zmultsav;
		} else if (!nextnode(firstnode(namedcmdll))) {
		    strcpy(ptr = cmdbuf, peekfirst(namedcmdll));
		    len = strlen(ptr);
		    ptr += len;
		    if (cmd == Th(z_acceptline) || cmd == Th(z_vicmdmode))
			goto unambiguous;
		} else {
		    strcpy(cmdbuf, peekfirst(namedcmdll));
		    ptr = cmdbuf + namedcmdambig;
		    *ptr = '_';
		    ptr[1] = '\0';
		    if (isset(AUTOLIST) &&
			!(isset(LISTAMBIGUOUS) && namedcmdambig > len)) {
			int zmultsav = zmult;
			if (isset(LISTBEEP))
			    feep = 1;
			zmult = 1;
			listlist(namedcmdll);
			listed = curlist = 1;
			showinglist = 0;
			zmult = zmultsav;
		    }
		    len = namedcmdambig;
		}
	    } else {
		if (len == NAMLEN || cmd != Th(z_selfinsert))
		    feep = 1;
		else {
#ifdef MULTIBYTE_SUPPORT
		    if (!lastchar_wide_valid)
			getrestchar(lastchar, NULL, NULL);
		    if (lastchar_wide == WEOF)
			feep = 1;
		    else
#endif
		    if (ZC_icntrl(LASTFULLCHAR))
			feep = 1;
		    else {
			int ret = zlecharasstring(LASTFULLCHAR, ptr);
			len += ret;
			ptr += ret;
			if (listed) {
			    clearlist = listshown = 1;
			    listed = 0;
			} else
			    curlist = 0;
		    }
		}
	    }
	}
	if (feep)
	    handlefeep(zlenoargs);
	feep = 0;
    }

 done:
    selectlocalmap(NULL);
    return retval;
}

/*****************/
/* Suffix system */
/*****************/

/*
 * The completion system sometimes tentatively adds a suffix to a word,
 * which can be removed depending on what is inserted next.  These
 * functions provide the capability to handle a removable suffix.
 *
 * Any removable suffix consists of characters immediately before the
 * cursor.  Whether it is removed depends on the next editing action.
 * There can be more than one suffix simultaneously present, with
 * different actions deleting different numbers of characters.
 *
 * If the next editing action changes the buffer other than by inserting
 * characters, normally the suffix should be removed so as to leave a
 * meaningful complete word.  The behaviour should be the same if the
 * next character inserted is a word separator.  If the next character
 * reasonably belongs where it is typed, or if the next editing action
 * is a deletion, the suffix should not be removed.  Other reasons for
 * suffix removal may have other behaviour.
 *
 * In order to maintain a consistent state, after a suffix has been added
 * the table *must* be zeroed, one way or another, before the buffer is
 * changed.  If the suffix is not being removed, call fixsuffix() to
 * indicate that it is being permanently fixed.
 */

struct suffixset;

/* An element of a suffix specification */
struct suffixset {
    struct suffixset *next;	/* Next in the list */
    int tp;			/* The SUFTYP_* from enum suffixtype */
    int flags;			/* Some of SUFFLAGS_* */
    ZLE_STRING_T chars;		/* Set of characters to match (or not) */
    int lenstr;			/* Length of chars */
    int lensuf;			/* Length of suffix */
};

/* The list of suffix structures */
static struct suffixset *suffixlist;

/* Shell function to call to remove the suffix. */

/**/
static char *suffixfunc;

/* Whether to remove suffix on uninsertable characters */
/**/
int suffixnoinsrem;

/* Length of the currently active, auto-removable suffix. */
/**/
mod_export int
suffixlen;

/**/
mod_export void
addsuffix(int tp, int flags, ZLE_STRING_T chars, int lenstr, int lensuf)
{
    struct suffixset *newsuf = zalloc(sizeof(struct suffixset));
    newsuf->next = suffixlist;
    suffixlist = newsuf;

    newsuf->tp = tp;
    newsuf->flags = flags;
    if (lenstr) {
	newsuf->chars = zalloc(lenstr*sizeof(ZLE_CHAR_T));
	ZS_memcpy(newsuf->chars, chars, lenstr);
    } else
	newsuf->chars = NULL;
    newsuf->lenstr = lenstr;
    newsuf->lensuf = lensuf;
}


/* Same as addsuffix, but from metafied string */

/**/
mod_export void
addsuffixstring(int tp, int flags, char *chars, int lensuf)
{
    int slen, alloclen;
    ZLE_STRING_T suffixstr;

    /* string needs to be writable... I've been regretting this for years.. */
    chars = ztrdup(chars);
    suffixstr = stringaszleline(chars, 0, &slen, &alloclen, NULL);
    addsuffix(tp, flags, suffixstr, slen, lensuf);
    zfree(suffixstr, alloclen);
    zsfree(chars);
}

/* Set up suffix: the last n characters are a suffix that should be *
 * removed in the usual word end conditions.                        */

/**/
mod_export void
makesuffix(int n)
{
    char *suffixchars;

    if (!(suffixchars = getsparam_u("ZLE_REMOVE_SUFFIX_CHARS")))
	suffixchars = " \t\n;&|";

    addsuffixstring(SUFTYP_POSSTR, 0, suffixchars, n);

    /* Do this second so it takes precedence */
    if ((suffixchars = getsparam_u("ZLE_SPACE_SUFFIX_CHARS")) && *suffixchars)
	addsuffixstring(SUFTYP_POSSTR, SUFFLAGS_SPACE, suffixchars, n);

    suffixlen = n;
    suffixnoinsrem = 1;
}

/* Set up suffix for parameter names: the last n characters are a suffix *
 * that should be removed if the next character is one of the ones that  *
 * needs to go immediately after the parameter name.  br indicates that  *
 * the name is in braces (${PATH} instead of $PATH), so the extra        *
 * characters that can only be used in braces are included.              */

/**/
mod_export void
makeparamsuffix(int br, int n)
{
    ZLE_STRING_T charstr = ZWS(":[#%?-+=");
    int lenstr = 0;

    if (br || unset(KSHARRAYS)) {
	lenstr = 2;
	if (br)
	    lenstr += 6;
    }
    if (lenstr)
	addsuffix(SUFTYP_POSSTR, 0, charstr, lenstr, n);
}

/* Set up suffix given a string containing the characters on which to   *
 * remove the suffix. */

/**/
mod_export void
makesuffixstr(char *f, char *s, int n)
{
    if (f) {
	zsfree(suffixfunc);
	suffixfunc = ztrdup(f);
	suffixlen = n;
    } else if (s) {
	int inv, i, z = 0;
	ZLE_STRING_T ws, lasts, wptr;

	if (*s == '^' || *s == '!') {
	    inv = 1;
	    s++;
	} else
	    inv = 0;
	s = getkeystring(s, &i, GETKEYS_SUFFIX, &z);
	s = metafy(s, i, META_USEHEAP);
	ws = stringaszleline(s, 0, &i, NULL, NULL);

	/* Remove suffix on uninsertable characters if  \- was given *
	 * and the character class wasn't negated -- or vice versa.  */
	suffixnoinsrem = z ^ inv;
	suffixlen = n;

	lasts = wptr = ws;
	while (i) {
	    if (i >= 3 && wptr[1] == ZWC('-')) {
		ZLE_CHAR_T str[2];

		if (wptr > lasts)
		    addsuffix(inv ? SUFTYP_NEGSTR : SUFTYP_POSSTR, 0,
			      lasts, wptr - lasts, n);
		str[0] = *wptr;
		str[1] = wptr[2];
		addsuffix(inv ? SUFTYP_NEGRNG : SUFTYP_POSRNG, 0,
			  str, 2, n);

		wptr += 3;
		i -= 3;
		lasts = wptr;
	    } else {
		wptr++;
		i--;
	    }
	}
	if (wptr > lasts)
	    addsuffix(inv ? SUFTYP_NEGSTR : SUFTYP_POSSTR, 0,
		      lasts, wptr - lasts, n);
	free(ws);
    } else
	makesuffix(n);
}

/* Remove suffix, if there is one, when inserting character c. */

/**/
mod_export void
iremovesuffix(ZLE_INT_T c, int keep)
{
    if (suffixfunc) {
	Shfunc shfunc = getshfunc(suffixfunc);

	if (shfunc) {
	    LinkList args = newlinklist();
	    char buf[20];
	    int osc = sfcontext;
	    int wasmeta = (zlemetaline != 0);

	    if (wasmeta) {
		/*
		 * The suffix removal function runs as a normal
		 * ZLE function, not a completion function, so
		 * the line should be unmetafied if this was
		 * called from completion.  (It may not be since
		 * we may decide to remove the suffix later.)
		 */
		unmetafy_line();
	    }

	    sprintf(buf, "%d", suffixlen);
	    addlinknode(args, suffixfunc);
	    addlinknode(args, buf);

	    startparamscope();
	    makezleparams(0);
	    sfcontext = SFC_COMPLETE;
	    doshfunc(shfunc, args, 1);
	    sfcontext = osc;
	    endparamscope();

	    if (wasmeta)
		metafy_line();
	}
	zsfree(suffixfunc);
	suffixfunc = NULL;
    } else {
	int sl = 0, flags = 0;
	struct suffixset *ss;

	if (c == NO_INSERT_CHAR) {
	    sl = suffixnoinsrem ? suffixlen : 0;
	} else {
	    ZLE_CHAR_T ch = c;
	    /*
	     * Search for a match for ch in the suffix list.
	     * We stop if we encounter a match in a positive or negative
	     * list, using the suffix length specified or zero respectively.
	     * If we reached the end and passed through a negative
	     * list, we use the suffix length for that, else zero.
	     * This would break if it were possible to have negative
	     * sets with different suffix length:  that's not supposed
	     * to happen.
	     */
	    int negsuflen = 0, found = 0;

	    for (ss = suffixlist; ss; ss = ss->next) {
		switch (ss->tp) {
		case SUFTYP_POSSTR:
		    if (ZS_memchr(ss->chars, ch, ss->lenstr)) {
			sl = ss->lensuf;
			found = 1;
		    }
		    break;

		case SUFTYP_NEGSTR:
		    if (ZS_memchr(ss->chars, ch, ss->lenstr)) {
			sl = 0;
			found = 1;
		    } else {
			negsuflen = ss->lensuf;
		    }
		    break;

		case SUFTYP_POSRNG:
		    if (ss->chars[0] <= ch && ch <= ss->chars[1]) {
			sl = ss->lensuf;
			found = 1;
		    }
		    break;

		case SUFTYP_NEGRNG:
		    if (ss->chars[0] <= ch && ch <= ss->chars[1]) {
			sl = 0;
			found = 1;
		    } else {
			negsuflen = ss->lensuf;
		    }
		    break;
		}
		if (found) {
		    flags = ss->flags;
		    break;
		}
	    }

	    if (!found)
		sl = negsuflen;
	}
	if (sl) {
	    /* must be shifting wide character lengths */
	    backdel(sl, CUT_RAW);
	    if (flags & SUFFLAGS_SPACE)
	    {
		/* Add a space and advance over it */
		spaceinline(1);
		if (zlemetaline) {
		    zlemetaline[zlemetacs++] = ' ';
		} else {
		    zleline[zlecs++] = ZWC(' ');
		}
	    }
	    if (!keep)
		invalidatelist();
	}
    }
    fixsuffix();
}

/* Fix the suffix in place, if there is one, making it non-removable. */

/**/
mod_export void
fixsuffix(void)
{
    while (suffixlist) {
	struct suffixset *next = suffixlist->next;

	if (suffixlist->lenstr)
	    zfree(suffixlist->chars, suffixlist->lenstr * sizeof(ZLE_CHAR_T));
	zfree(suffixlist, sizeof(struct suffixset));

	suffixlist = next;
    }

    suffixnoinsrem = suffixlen = 0;
}
