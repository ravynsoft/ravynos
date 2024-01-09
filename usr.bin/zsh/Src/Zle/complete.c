/*
 * complete.c - the complete module, interface part
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1999 Sven Wischnowsky
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

#include "complete.mdh"
#include "complete.pro"

/* global variables for shell parameters in new style completion */

/**/
mod_export
zlong compcurrent,
      complistmax;
/**/
zlong complistlines,
      compignored;

/**/
mod_export
char **compwords,
     **compredirs,
     *compprefix,
     *compsuffix,
     *complastprefix,
     *complastsuffix,
     *compisuffix,
     *compqiprefix,
     *compqisuffix,
     *compquote,
     *compqstack,      /* compstate[all_quotes] */
     *comppatmatch,
     *complastprompt;
/**/
char *compiprefix,
     *compcontext,
     *compparameter,
     *compredirect,
     *compquoting,
     *comprestore,
     *complist,
     *compinsert,
     *compexact,
     *compexactstr,
     *comppatinsert,
     *comptoend,      /* compstate[to_end]; populates 'movetoend' */
     *compoldlist,
     *compoldins,
     *compvared;

/*
 * An array of Param structures for compsys special parameters;
 * see 'comprparams' below.  An entry for $compstate is added
 * by makecompparams().
 *
 * See CP_REALPARAMS.
 */

/**/
Param *comprpms;

/* 
 * An array of Param structures for elements of $compstate; see
 * 'compkparams' below.
 *
 * See CP_KEYPARAMS.
 */

/**/
Param *compkpms;

/**/
mod_export void
freecmlist(Cmlist l)
{
    Cmlist n;

    while (l) {
	n = l->next;
	freecmatcher(l->matcher);
	zsfree(l->str);

	zfree(l, sizeof(struct cmlist));

	l = n;
    }
}

/**/
mod_export void
freecmatcher(Cmatcher m)
{
    Cmatcher n;

    if (!m || --(m->refc))
	return;

    while (m) {
	n = m->next;
	freecpattern(m->line);
	freecpattern(m->word);
	freecpattern(m->left);
	freecpattern(m->right);

	zfree(m, sizeof(struct cmatcher));

	m = n;
    }
}

/**/
void
freecpattern(Cpattern p)
{
    Cpattern n;

    while (p) {
	n = p->next;
	if (p->tp <= CPAT_EQUIV)
	    free(p->u.str);
	zfree(p, sizeof(struct cpattern));

	p = n;
    }
}

/* Copy a completion matcher list into permanent storage. */

/**/
mod_export Cmatcher
cpcmatcher(Cmatcher m)
{
    Cmatcher r = NULL, *p = &r, n;

    while (m) {
	*p = n = (Cmatcher) zalloc(sizeof(struct cmatcher));

	n->refc = 1;
	n->next = NULL;
	n->flags = m->flags;
	n->line = cpcpattern(m->line);
	n->llen = m->llen;
	n->word = cpcpattern(m->word);
	n->wlen = m->wlen;
	n->left = cpcpattern(m->left);
	n->lalen = m->lalen;
	n->right = cpcpattern(m->right);
	n->ralen = m->ralen;

	p = &(n->next);
	m = m->next;
    }
    return r;
}

/*
 * Copy a single entry in a matcher pattern.
 * If useheap is 1, it comes from the heap.
 */

/**/
mod_export Cpattern
cp_cpattern_element(Cpattern o)
{
    Cpattern n = zalloc(sizeof(struct cpattern));

    n->next = NULL;

    n->tp = o->tp;
    switch (o->tp)
    {
    case CPAT_CCLASS:
    case CPAT_NCLASS:
    case CPAT_EQUIV:
	n->u.str = ztrdup(o->u.str);
	break;

    case CPAT_CHAR:
	n->u.chr = o->u.chr;
	break;

    default:
	/* just to keep compiler quiet */
	break;
    }

    return n;
}

/* Copy a completion matcher pattern. */

/**/
static Cpattern
cpcpattern(Cpattern o)
{
    Cpattern r = NULL, *p = &r;

    while (o) {
	*p = cp_cpattern_element(o);
	p = &((*p)->next);
	o = o->next;
    }
    return r;
}

/* 
 * Parse a string for matcher control, containing multiple matchers.
 *
 * 's' is the string to be parsed.
 *
 * 'name' is the name of the builtin from which this is called, for errors.
 *
 * Return 'pcm_err' on error; a NULL return value means ...
 */

/**/
mod_export Cmatcher
parse_cmatcher(char *name, char *s)
{
    Cmatcher ret = NULL, r = NULL, n;
    Cpattern line, word, left, right;
    int fl, fl2, ll, wl, lal, ral, err, both;

    if (!*s)
	return NULL;

    while (*s) {
	lal = ral = both = fl2 = 0;
	left = right = NULL;

	while (*s && inblank(*s)) s++;

	if (!*s) break;

	switch (*s) {
	case 'b': fl2 = CMF_INTER; /* FALLTHROUGH */
	case 'l': fl = CMF_LEFT; break;
	case 'e': fl2 = CMF_INTER; /* FALLTHROUGH */
	case 'r': fl = CMF_RIGHT; break;
	case 'm': fl = 0; break;
	case 'B': fl2 = CMF_INTER; /* FALLTHROUGH */
	case 'L': fl = CMF_LEFT | CMF_LINE; break;
	case 'E': fl2 = CMF_INTER; /* FALLTHROUGH */
	case 'R': fl = CMF_RIGHT | CMF_LINE; break;
	case 'M': fl = CMF_LINE; break;
	case 'x': break;
	default:
	    if (name)
		zwarnnam(name, "unknown match specification character `%c'",
			 *s);
	    return pcm_err;
	}
	if (s[1] != ':') {
	    if (name)
		zwarnnam(name, "missing `:'");
	    return pcm_err;
	}
	if (*s == 'x') {
	    if (s[2] && !inblank(s[2])) {
		if (name)
		    zwarnnam(name,
			"unexpected pattern following x: specification");
		return pcm_err;
	    }
	    return ret;
	}
	s += 2;
	if (!*s) {
	    if (name)
		zwarnnam(name, "missing patterns");
	    return pcm_err;
	}
	if ((fl & CMF_LEFT) && !fl2) {
	    left = parse_pattern(name, &s, &lal, '|', &err);
	    if (err)
		return pcm_err;

	    if ((both = (*s && s[1] == '|')))
		s++;

	    if (!*s || !*++s) {
		if (name) {
                   if (both)
                       zwarnnam(name, "missing right anchor");
                   else
                       zwarnnam(name, "missing line pattern");
		}
		return pcm_err;
	    }
	} else
	    left = NULL;

	line = parse_pattern(name, &s, &ll,
			     (((fl & CMF_RIGHT) && !fl2) ? '|' : '='),
			     &err);
	if (err)
	    return pcm_err;
	if (both) {
	    right = line;
	    ral = ll;
	    line = NULL;
	    ll = 0;
	}
	if ((fl & CMF_RIGHT) && !fl2 && (!*s || !*++s)) {
	    if (name)
		zwarnnam(name, "missing right anchor");
           return pcm_err;
	} else if (!(fl & CMF_RIGHT) || fl2) {
	    if (!*s) {
		if (name)
		    zwarnnam(name, "missing word pattern");
		return pcm_err;
	    }
	    s++;
	}
	if ((fl & CMF_RIGHT) && !fl2) {
	    if (*s == '|') {
		left = line;
		lal = ll;
		line = NULL;
		ll = 0;
		s++;
	    }
	    right = parse_pattern(name, &s, &ral, '=', &err);
	    if (err)
		return pcm_err;
	    if (!*s) {
		if (name)
		    zwarnnam(name, "missing word pattern");
		return pcm_err;
	    }
	    s++;
       }

	if (*s == '*') {
	    if (!(fl & (CMF_LEFT | CMF_RIGHT))) {
		if (name)
		    zwarnnam(name, "need anchor for `*'");
		return pcm_err;
	    }
	    word = NULL;
	    if (*++s == '*') {
		s++;
		wl = -2;
	    } else
		wl = -1;
	} else {
	    word = parse_pattern(name, &s, &wl, 0, &err);

	    if (!word && !line) {
		if (name)
		    zwarnnam(name, "need non-empty word or line pattern");
		return pcm_err;
	    }
	}
	if (err)
	    return pcm_err;

	n = (Cmatcher) hcalloc(sizeof(*ret));
	n->next = NULL;
	n->flags = fl | fl2;
	n->line = line;
	n->llen = ll;
	n->word = word;
	n->wlen = wl;
	n->left = left;
	n->lalen = lal;
	n->right = right;
	n->ralen = ral;

	if (ret)
	    r->next = n;
	else
	    ret = n;

	r = n;
    }
    return ret;
}

/*
 * Parse a pattern for matcher control. 
 * name is the name of the builtin from which this is called, for errors.
 * *sp is the input string and will be updated to the end of the parsed
 *   pattern.
 * *lp will be set to the number of characters (possibly multibyte)
 *   that the pattern will match.  This must be deterministic, given
 *   the syntax allowed here.
 * e, if non-zero, is the ASCII end character to match; if zero,
 *   stop on a blank.
 * *err is set to 1 to indicate an error, else to 0.
 */

/**/
static Cpattern
parse_pattern(char *name, char **sp, int *lp, char e, int *err)
{
    Cpattern ret = NULL, r = NULL, n;
    char *s = *sp;
    convchar_t inchar;
    int l = 0, inlen;

    *err = 0;

    MB_METACHARINIT();
    while (*s && (e ? (*s != e) : !inblank(*s))) {
	n = (Cpattern) hcalloc(sizeof(*n));
	n->next = NULL;

	if (*s == '[' || *s == '{') {
	    s = parse_class(n, s);
	    if (!*s) {
		*err = 1;
		zwarnnam(name, "unterminated character class");
		return NULL;
	    }
	    s++;
	} else if (*s == '?') {
	    n->tp = CPAT_ANY;
	    s++;
	} else if (*s == '*' || *s == '(' || *s == ')' || *s == '=') {
	    *err = 1;
	    zwarnnam(name, "invalid pattern character `%c'", *s);
	    return NULL;
	} else {
	    if (*s == '\\' && s[1])
		s++;

	    inlen = MB_METACHARLENCONV(s, &inchar);
#ifdef MULTIBYTE_SUPPORT
	    if (inchar == WEOF)
		inchar = (convchar_t)(*s == Meta ? s[1] ^ 32 : *s);
#endif
	    s += inlen;
	    n->tp = CPAT_CHAR;
	    n->u.chr = inchar;
	}
	if (ret)
	    r->next = n;
	else
	    ret = n;

	r = n;

	l++;
    }
    *sp = (char *) s;
    *lp = l;
    return ret;
}

/* Parse a character class for matcher control. */

/**/
static char *
parse_class(Cpattern p, char *iptr)
{
    int endchar, firsttime = 1;
    char *optr, *nptr;

    if (*iptr++ == '[') {
	endchar = ']';
	/* TODO: surely [^]] is valid? */
	if ((*iptr == '!' || *iptr == '^') && iptr[1] != ']') {
	    p->tp = CPAT_NCLASS;
	    iptr++;
	} else
	    p->tp = CPAT_CCLASS;
    } else {
	endchar = '}';
	p->tp = CPAT_EQUIV;
    }

    /* find end of class.  End character can appear literally first. */
    for (optr = iptr; optr == iptr || *optr != endchar; optr++)
	if (!*optr)
	    return optr;
    /*
     * We can always fit the parsed class within the same length
     * because of the tokenization (including a null byte).
     *
     * As the input string is metafied, but shouldn't contain shell
     * tokens, we can just add our own tokens willy nilly.
     */
    optr = p->u.str = zhalloc((optr-iptr) + 1);

    while (firsttime || *iptr != endchar) {
	int ch;

	if (*iptr == '[' && iptr[1] == ':' &&
	    (nptr = strchr((char *)iptr + 2, ':')) && nptr[1] == ']') {
	    /* Range type */
	    iptr += 2;
	    ch = range_type((char *)iptr, nptr-iptr);
	    iptr = nptr + 2;
	    if (ch != PP_UNKWN)
		*optr++ = STOUC(Meta) + ch;
	} else {
	    /* characters stay metafied */
	    char *ptr1 = iptr;
	    if (*iptr == Meta)
		iptr++;
	    iptr++;
	    if (*iptr == '-' && iptr[1] && iptr[1] != endchar) {
		/* a run of characters */
		iptr++;
		/* range token */
		*optr++ = Meta + PP_RANGE;

		/* start of range character */
		if (*ptr1 == Meta) {
		    *optr++ = Meta;
		    *optr++ = ptr1[1] ^ 32;
		} else
		    *optr++ = *ptr1;

		if (*iptr == Meta) {
		    *optr++ = *iptr++;
		    *optr++ = *iptr++;
		} else
		    *optr++ = *iptr++;
	    } else {
		if (*ptr1 == Meta) {
		    *optr++ = Meta;
		    *optr++ = ptr1[1] ^ 32;
		} else
		    *optr++ = *ptr1;
	    }
	}
	firsttime = 0;
    }

    *optr = '\0';
    return iptr;
}

static struct { char *name; int abbrev; int oflag; } orderopts[] = {
    { "nosort", 2, CAF_NOSORT },
    { "match", 3, CAF_MATSORT },
    { "numeric", 3, CAF_NUMSORT },
    { "reverse", 3, CAF_REVSORT }
};

/* Parse the option to compadd -o, if flags is non-NULL set it
 * returns -1 if the argument isn't a valid ordering, 0 otherwise */

/**/
static int
parse_ordering(const char *arg, int *flags)
{
    int o, fl = 0;
    const char *next, *opt = arg;
    do {
	int found = 0;
	next = strchr(opt, ',');
	if (!next)
	    next = opt + strlen(opt);

	for (o = sizeof(orderopts)/sizeof(*orderopts) - 1; o >= 0 &&
		!found; --o)
	{
	    if ((found = next - opt >= orderopts[o].abbrev &&
	            !strncmp(orderopts[o].name, opt, next - opt)))
		fl |= orderopts[o].oflag;
	}
	if (!found) {
	    if (flags) /* default to "match" */
		*flags = CAF_MATSORT;
	    return -1;
	}
    } while (*next && ((opt = next + 1)));
    if (flags)
	*flags |= fl;
    return 0;
}

/**/
static int
bin_compadd(char *name, char **argv, UNUSED(Options ops), UNUSED(int func))
{
    struct cadata dat;
    char *mstr = NULL; /* argument of -M options, accumulated */
    char *oarg = NULL; /* argument of -o option */
    int added; /* return value */
    Cmatcher match = NULL;
    size_t dparlen = 0, dparsize = 0; /* no. of -D options and array size */

    if (incompfunc != 1) {
	zwarnnam(name, "can only be called from completion function");
	return 1;
    }
    dat.ipre = dat.isuf = dat.ppre = dat.psuf = dat.prpre = dat.mesg =
	dat.pre = dat.suf = dat.group = dat.rems = dat.remf = dat.disp =
	dat.ign = dat.exp = dat.apar = dat.opar = NULL;
    dat.dpar = NULL;
    dat.match = NULL;
    dat.flags = 0;
    dat.aflags = CAF_MATCH;
    dat.dummies = -1;

    for (; *argv && **argv ==  '-'; argv++) {
	char *p; /* loop variable, points into argv */
	if (!(*argv)[1]) {
	    argv++;
	    break;
	}
	for (p = *argv + 1; *p; p++) {
	    char *m = NULL; /* argument of -M option (this one only) */
	    int order = 0;  /* if -o found (argument to which is optional) */
	    char **sp = NULL; /* the argument to an option should be copied
				 to *sp. */
	    const char *e; /* error message */
	    switch (*p) {
	    case 'q':
		dat.flags |= CMF_REMOVE;
		break;
	    case 'Q':
		dat.aflags |= CAF_QUOTE;
		break;
	    case 'C':
		dat.aflags |= CAF_ALL;
		break;
	    case 'f':
		dat.flags |= CMF_FILE;
		break;
	    case 'e':
		dat.flags |= CMF_ISPAR;
		break;
	    case 'a':
		dat.aflags |= CAF_ARRAYS;
		break;
	    case 'k':
		dat.aflags |= CAF_ARRAYS|CAF_KEYS;
		break;
	    case 'F':
		sp = &(dat.ign);
		e = "string expected after -%c";
		break;
	    case 'n':
		dat.flags |= CMF_NOLIST;
		break;
	    case 'U':
		dat.aflags &= ~CAF_MATCH;
		break;
	    case 'P':
		sp = &(dat.pre);
		e = "string expected after -%c";
		break;
	    case 'S':
		sp = &(dat.suf);
		e = "string expected after -%c";
		break;
	    case 'J':
		sp = &(dat.group);
		e = "group name expected after -%c";
		break;
	    case 'V':
		if (!dat.group)
		    dat.aflags |= CAF_NOSORT;
		sp = &(dat.group);
		e = "group name expected after -%c";
		break;
	    case '1':
		if (!(dat.aflags & CAF_UNIQCON))
		    dat.aflags |= CAF_UNIQALL;
		break;
	    case '2':
		if (!(dat.aflags & CAF_UNIQALL))
		    dat.aflags |= CAF_UNIQCON;
		break;
	    case 'i':
		sp = &(dat.ipre);
		e = "string expected after -%c";
		break;
	    case 'I':
		sp = &(dat.isuf);
		e = "string expected after -%c";
		break;
	    case 'p':
		sp = &(dat.ppre);
		e = "string expected after -%c";
		break;
	    case 's':
		sp = &(dat.psuf);
		e = "string expected after -%c";
		break;
	    case 'W':
		sp = &(dat.prpre);
		e = "string expected after -%c";
		break;
	    case 'M':
		sp = &m;
		e = "matching specification expected after -%c";
		break;
	    case 'X':
		sp = &(dat.exp);
		e = "string expected after -%c";
		break;
	    case 'x':
		sp = &(dat.mesg);
		e = "string expected after -%c";
		break;
	    case 'r':
		dat.flags |= CMF_REMOVE;
		sp = &(dat.rems);
		e = "string expected after -%c";
		break;
	    case 'R':
		dat.flags |= CMF_REMOVE;
		sp = &(dat.remf);
		e = "function name expected after -%c";
		break;
	    case 'A':
		sp = &(dat.apar);
		e = "parameter name expected after -%c";
		break;
	    case 'O':
		sp = &(dat.opar);
		e = "parameter name expected after -%c";
		break;
	    case 'D':
		if (dparsize <= dparlen + 1) {
		    dparsize = (dparsize + 1) * 2;
		    dat.dpar = (char **)zrealloc(dat.dpar, sizeof(char *) * dparsize);
		}
		sp = dat.dpar + dparlen++;
		*sp = dat.dpar[dparlen] = NULL;
		e = "parameter name expected after -%c";
		break;
	    case 'd':
		sp = &(dat.disp);
		e = "parameter name expected after -%c";
		break;
	    case 'l':
		dat.flags |= CMF_DISPLINE;
		break;
	    case 'o':
		/* we honour just the first -o option but need to skip
		 * over a valid argument to subsequent -o options */
		order = oarg ? -1 : 1;
		sp = &oarg;
		/* no error string because argument is optional */
		break;
	    case 'E':
                if (p[1]) {
                    dat.dummies = atoi(p + 1);
		    p += strlen(p+1);
                } else if (argv[1]) {
                    argv++;
                    dat.dummies = atoi(*argv);
                } else {
                    zwarnnam(name, "number expected after -%c", *p);
		    zsfree(mstr);
		    zfree(dat.dpar, dparsize);
                    return 1;
                }
                if (dat.dummies < 0) {
                    zwarnnam(name, "invalid number: %d", dat.dummies);
		    zsfree(mstr);
		    zfree(dat.dpar, dparsize);
                    return 1;
                }
		break;
	    case '-':
		argv++;
		goto ca_args;
	    default:
		zwarnnam(name, "bad option: -%c", *p);
		zsfree(mstr);
		zfree(dat.dpar, dparsize);
		return 1;
	    }
	    if (sp) {
		if (p[1]) {
		    /* Pasted argument: -Xfoo. */
		    if (!*sp) /* take first option only */
			*sp = p + 1;
		    if (!order || !parse_ordering(oarg, order == 1 ? &dat.aflags : NULL))
			p += strlen(p+1);
		} else if (argv[1]) {
		    /* Argument in a separate word: -X foo. */
		    argv++;
		    if (!*sp)
			*sp = *argv;
		    if (order && parse_ordering(oarg, order == 1 ? &dat.aflags : NULL))
			--argv;
		} else if (!order) {
		    /* Missing argument: argv[N] == "-X", argv[N+1] == NULL. */
		    zwarnnam(name, e, *p);
		    zsfree(mstr);
		    zfree(dat.dpar, dparsize);
		    return 1;
		}
		if (m) {
		    if (mstr) {
			char *tmp = tricat(mstr, " ", m);
			zsfree(mstr);
			mstr = tmp;
		    } else
			mstr = ztrdup(m);
		}
	    }
	}
    }

 ca_args:

    if (mstr && (match = parse_cmatcher(name, mstr)) == pcm_err) {
	zsfree(mstr);
	zfree(dat.dpar, dparsize);
	return 1;
    }
    zsfree(mstr);

    if (!*argv && !dat.group && !dat.mesg &&
	!(dat.aflags & (CAF_NOSORT|CAF_UNIQALL|CAF_UNIQCON|CAF_ALL))) {
	zfree(dat.dpar, dparsize);
	return 1;
    }

    dat.match = match = cpcmatcher(match);
    added = addmatches(&dat, argv);
    freecmatcher(match);
    zfree(dat.dpar, dparsize);

    return added;
}

#define CVT_RANGENUM 0
#define CVT_RANGEPAT 1
#define CVT_PRENUM   2
#define CVT_PREPAT   3
#define CVT_SUFNUM   4
#define CVT_SUFPAT   5

/**/
mod_export void
ignore_prefix(int l)
{
    if (l) {
	char *tmp, sav;
	int pl = strlen(compprefix);

	if (l > pl)
	    l = pl;

	sav = compprefix[l];

	compprefix[l] = '\0';
	tmp = tricat(compiprefix, compprefix, "");
	zsfree(compiprefix);
	compiprefix = tmp;
	compprefix[l] = sav;
	tmp = ztrdup(compprefix + l);
	zsfree(compprefix);
	compprefix = tmp;
    }
}

/**/
mod_export void
ignore_suffix(int l)
{
    if (l) {
	char *tmp, sav;
	int sl = strlen(compsuffix);

	if ((l = sl - l) < 0)
	    l = 0;

	tmp = tricat(compsuffix + l, compisuffix, "");
	zsfree(compisuffix);
	compisuffix = tmp;
	sav = compsuffix[l];
	compsuffix[l] = '\0';
	tmp = ztrdup(compsuffix);
	compsuffix[l] = sav;
	zsfree(compsuffix);
	compsuffix = tmp;
    }
}

/**/
mod_export void
restrict_range(int b, int e)
{
    int wl = arrlen(compwords) - 1;

    if (wl && b >= 0 && e >= 0 && (b > 0 || e < wl)) {
	int i;
	char **p, **q, **pp;

	if (e > wl)
	    e = wl;

	i = e - b + 1;
	p = (char **) zshcalloc((i + 1) * sizeof(char *));

	for (q = p, pp = compwords + b; i; i--, q++, pp++)
	    *q = ztrdup(*pp);
	freearray(compwords);
	compwords = p;
	compcurrent -= b;
    }
}

/**/
static int
do_comp_vars(int test, int na, char *sa, int nb, char *sb, int mod)
{
    switch (test) {
    case CVT_RANGENUM:
	{
	    int l = arrlen(compwords);

	    if (na < 0)
		na += l;
	    else
		na--;
	    if (nb < 0)
		nb += l;
	    else
		nb--;

	    if (compcurrent - 1 < na || compcurrent - 1 > nb)
		return 0;
	    if (mod)
		restrict_range(na, nb);
	    return 1;
	}
    case CVT_RANGEPAT:
	{
	    char **p;
	    int i, l = arrlen(compwords), t = 0, b = 0, e = l - 1;
	    Patprog pp;

	    i = compcurrent - 1;
	    if (i < 0 || i >= l)
		return 0;

	    singsub(&sa);
	    pp = patcompile(sa, PAT_HEAPDUP, NULL);

	    for (i--, p = compwords + i; i >= 0; p--, i--) {
		if (pattry(pp, *p)) {
		    b = i + 1;
		    t = 1;
		    break;
		}
	    }
	    if (t && sb) {
		int tt = 0;

		singsub(&sb);
		pp = patcompile(sb, PAT_STATIC, NULL);

		for (i++, p = compwords + i; i < l; p++, i++) {
		    if (pattry(pp, *p)) {
			e = i - 1;
			tt = 1;
			break;
		    }
		}
		if (tt && i < compcurrent)
		    t = 0;
	    }
	    if (e < b)
		t = 0;
	    if (t && mod)
		restrict_range(b, e);
	    return t;
	}
    case CVT_PRENUM:
    case CVT_SUFNUM:
	if (na < 0)
	    return 0;
	if (na > 0 && mod) {
#ifdef MULTIBYTE_SUPPORT
	    if (isset(MULTIBYTE)) {
		if (test == CVT_PRENUM) {
		    const char *ptr = compprefix;
		    int len = 1;
		    int sum = 0;
		    while (*ptr && na && len) {
			wint_t wc;
			len = mb_metacharlenconv(ptr, &wc);
			ptr += len;
			sum += len;
			na--;
		    }
		    if (na)
			return 0;
		    na = sum;
		} else {
		    char *end = compsuffix + strlen(compsuffix);
		    char *ptr = end;
		    while (na-- && ptr > compsuffix)
			 ptr = backwardmetafiedchar(compsuffix, ptr, NULL);
		    if (na >= 0)
			return 0;
		    na = end - ptr;
		}
	    } else
#endif
	    if ((int)strlen(test == CVT_PRENUM ? compprefix : compsuffix) < na)
		return 0;
	    if (test == CVT_PRENUM)
		ignore_prefix(na);
	    else
		ignore_suffix(na);
	    return 1;
	}
	return 1;
    case CVT_PREPAT:
    case CVT_SUFPAT:
	{
	    Patprog pp;

	    if (!na)
		return 0;

	    if (!(pp = patcompile(sa, PAT_HEAPDUP, 0)))
		return 0;

	    if (test == CVT_PREPAT) {
		int l, add;
		char *p, sav;

		if (!(l = strlen(compprefix)))
		    return ((na == 1 || na == -1) && pattry(pp, compprefix));
		if (na < 0) {
		    p = compprefix + l;
		    na = -na;
		    add = -1;
		} else {
		    p = compprefix + 1 + (*compprefix == Meta);
		    if (p > compprefix + l)
			p = compprefix + l;
		    add = 1;
		}
		for (;;) {
		    sav = *p;
		    *p = '\0';
		    test = pattry(pp, compprefix);
		    *p = sav;
		    if (test && !--na)
			break;
		    if (add > 0) {
			if (p == compprefix + l)
			    return 0;
			p = p + 1 + (*p == Meta);
			if (p > compprefix + l)
			    p = compprefix + l;
		    } else {
			if (p == compprefix)
			    return 0;
			p--;
			if (p > compprefix && p[-1] == Meta)
			    p--;
		    }
		}
		if (mod)
		    ignore_prefix(p - compprefix);
	    } else {
		int l, ol, add;
		char *p;

		if (!(ol = l = strlen(compsuffix)))
		    return ((na == 1 || na == -1) && pattry(pp, compsuffix));
		if (na < 0) {
		    p = compsuffix;
		    na = -na;
		    add = 1;
		} else {
		    p = compsuffix + l - 1;
		    if (p > compsuffix && p[-1] == Meta)
			p--;
		    add = -1;
		}
		for (;;) {
		    if (pattry(pp, p) && !--na)
			break;

		    if (add > 0) {
			if (p == compsuffix + l)
			    return 0;
			if (*p == Meta)
			    p += 2;
			else
			    p++;
		    } else {
			if (p == compsuffix)
			    return 0;
			p--;
			if (p > compsuffix && p[-1] == Meta)
			    p--;
		    }
		}

		if (mod)
		    ignore_suffix(ol - (p - compsuffix));
	    }
	    return 1;
	}
    }
    return 0;
}

/**/
static int
bin_compset(char *name, char **argv, UNUSED(Options ops), UNUSED(int func))
{
    int test = 0, na = 0, nb = 0;
    char *sa = NULL, *sb = NULL;

    if (incompfunc != 1) {
	zwarnnam(name, "can only be called from completion function");
	return 1;
    }
    if (argv[0][0] != '-') {
	zwarnnam(name, "missing option");
	return 1;
    }
    switch (argv[0][1]) {
    case 'n': test = CVT_RANGENUM; break;
    case 'N': test = CVT_RANGEPAT; break;
    case 'p': test = CVT_PRENUM; break;
    case 'P': test = CVT_PREPAT; break;
    case 's': test = CVT_SUFNUM; break;
    case 'S': test = CVT_SUFPAT; break;
    case 'q': return set_comp_sep();
    default:
	zwarnnam(name, "bad option -%c", argv[0][1]);
	return 1;
    }
    if (argv[0][2]) {
	sa = argv[0] + 2;
	sb = argv[1];
	na = 2;
    } else {
	if (!(sa = argv[1])) {
	    zwarnnam(name, "missing string for option -%c", argv[0][1]);
	    return 1;
	}
	sb = argv[2];
	na = 3;
    }
    if (((test == CVT_PRENUM || test == CVT_SUFNUM) ? !!sb :
	 (sb && argv[na]))) {
	zwarnnam(name, "too many arguments");
	return 1;
    }
    switch (test) {
    case CVT_RANGENUM:
	na = atoi(sa);
	nb = (sb ? atoi(sb) : -1);
	break;
    case CVT_RANGEPAT:
	tokenize(sa);
	remnulargs(sa);
	if (sb) {
	    tokenize(sb);
	    remnulargs(sb);
	}
	break;
    case CVT_PRENUM:
    case CVT_SUFNUM:
	na = atoi(sa);
	break;
    case CVT_PREPAT:
    case CVT_SUFPAT:
	if (sb) {
	    na = atoi(sa);
	    sa = sb;
	} else
	    na = -1;
	tokenize(sa);
	remnulargs(sa);
	break;
    }
    return !do_comp_vars(test, na, sa, nb, sb, 1);
}

/* Definitions for the special parameters. Note that these have to match the
 * order of the CP_* bits in comp.h */

#define VAL(X) ((void *) (&(X)))
#define GSU(X) ((GsuScalar)(void *) (&(X)))
struct compparam {
    char *name;
    int type;
    void *var;
    GsuScalar gsu;
};

static const struct gsu_scalar compvarscalar_gsu =
{ strvargetfn, strvarsetfn, compunsetfn };
static const struct gsu_scalar complist_gsu =
{ get_complist, set_complist, compunsetfn };
static const struct gsu_scalar unambig_gsu =
{ get_unambig, nullstrsetfn, compunsetfn };
static const struct gsu_scalar unambig_pos_gsu =
{ get_unambig_pos, nullstrsetfn, compunsetfn };
static const struct gsu_scalar insert_pos_gsu =
{ get_insert_pos, nullstrsetfn, compunsetfn };
static const struct gsu_scalar compqstack_gsu =
{ get_compqstack, nullstrsetfn, compunsetfn };

static const struct gsu_integer compvarinteger_gsu =
{ intvargetfn, intvarsetfn, compunsetfn };
static const struct gsu_integer nmatches_gsu =
{ get_nmatches, NULL, compunsetfn };
static const struct gsu_integer unambig_curs_gsu =
{ get_unambig_curs, NULL, compunsetfn };
static const struct gsu_integer listlines_gsu =
{ get_listlines, NULL, compunsetfn };

static const struct gsu_array compvararray_gsu =
{ arrvargetfn, arrvarsetfn, compunsetfn };


static struct compparam comprparams[] = {
    { "words", PM_ARRAY, VAL(compwords), NULL },
    { "redirections", PM_ARRAY, VAL(compredirs), NULL },
    { "CURRENT", PM_INTEGER, VAL(compcurrent), NULL },
    { "PREFIX", PM_SCALAR, VAL(compprefix), NULL },
    { "SUFFIX", PM_SCALAR, VAL(compsuffix), NULL },
    { "IPREFIX", PM_SCALAR, VAL(compiprefix), NULL },
    { "ISUFFIX", PM_SCALAR, VAL(compisuffix), NULL },
    { "QIPREFIX", PM_SCALAR | PM_READONLY, VAL(compqiprefix), NULL },
    { "QISUFFIX", PM_SCALAR | PM_READONLY, VAL(compqisuffix), NULL },
    { NULL, 0, NULL, NULL }
};

static struct compparam compkparams[] = {
    { "nmatches", PM_INTEGER | PM_READONLY, NULL, GSU(nmatches_gsu) },
    { "context", PM_SCALAR, VAL(compcontext), NULL },
    { "parameter", PM_SCALAR, VAL(compparameter), NULL },
    { "redirect", PM_SCALAR, VAL(compredirect), NULL },
    { "quote", PM_SCALAR | PM_READONLY, VAL(compquote), NULL },
    { "quoting", PM_SCALAR | PM_READONLY, VAL(compquoting), NULL },
    { "restore", PM_SCALAR, VAL(comprestore), NULL },
    { "list", PM_SCALAR, NULL, GSU(complist_gsu) },
    { "insert", PM_SCALAR, VAL(compinsert), NULL },
    { "exact", PM_SCALAR, VAL(compexact), NULL },
    { "exact_string", PM_SCALAR, VAL(compexactstr), NULL },
    { "pattern_match", PM_SCALAR, VAL(comppatmatch), NULL },
    { "pattern_insert", PM_SCALAR, VAL(comppatinsert), NULL },
    { "unambiguous", PM_SCALAR | PM_READONLY, NULL, GSU(unambig_gsu) },
    { "unambiguous_cursor", PM_INTEGER | PM_READONLY, NULL,
      GSU(unambig_curs_gsu) },
    { "unambiguous_positions", PM_SCALAR | PM_READONLY, NULL,
      GSU(unambig_pos_gsu) },
    { "insert_positions", PM_SCALAR | PM_READONLY, NULL,
      GSU(insert_pos_gsu) },
    { "list_max", PM_INTEGER, VAL(complistmax), NULL },
    { "last_prompt", PM_SCALAR, VAL(complastprompt), NULL },
    { "to_end", PM_SCALAR, VAL(comptoend), NULL },
    { "old_list", PM_SCALAR, VAL(compoldlist), NULL },
    { "old_insert", PM_SCALAR, VAL(compoldins), NULL },
    { "vared", PM_SCALAR, VAL(compvared), NULL },
    { "list_lines", PM_INTEGER | PM_READONLY, NULL, GSU(listlines_gsu) },
    { "all_quotes", PM_SCALAR | PM_READONLY, NULL, GSU(compqstack_gsu) },
    { "ignored", PM_INTEGER | PM_READONLY, VAL(compignored), NULL },
    { NULL, 0, NULL, NULL }
};

#define COMPSTATENAME "compstate"

static void
addcompparams(struct compparam *cp, Param *pp)
{
    for (; cp->name; cp++, pp++) {
	Param pm = createparam(cp->name,
			       cp->type |PM_SPECIAL|PM_REMOVABLE|PM_LOCAL);
	if (!pm)
	    pm = (Param) paramtab->getnode(paramtab, cp->name);
	DPUTS(!pm, "param not set in addcompparams");

	*pp = pm;
	pm->level = locallevel + 1;
	if ((pm->u.data = cp->var)) {
	    switch(PM_TYPE(cp->type)) {
	    case PM_SCALAR:
		pm->gsu.s = &compvarscalar_gsu;
		break;
	    case PM_INTEGER:
		pm->gsu.i = &compvarinteger_gsu;
		pm->base = 10;
		break;
	    case PM_ARRAY:
		pm->gsu.a = &compvararray_gsu;
		break;
	    }
	} else {
	    pm->gsu.s = cp->gsu;
	}
    }
}

static const struct gsu_hash compstate_gsu =
{ get_compstate, set_compstate, compunsetfn };

/**/
void
makecompparams(void)
{
    Param cpm;
    HashTable tht;

    addcompparams(comprparams, comprpms);

    if (!(cpm = createparam(
	      COMPSTATENAME,
	      PM_SPECIAL|PM_REMOVABLE|PM_SINGLE|PM_LOCAL|PM_HASHED)))
	cpm = (Param) paramtab->getnode(paramtab, COMPSTATENAME);
    DPUTS(!cpm, "param not set in makecompparams");

    comprpms[CPN_COMPSTATE] = cpm;
    tht = paramtab;
    cpm->level = locallevel + 1;
    cpm->gsu.h = &compstate_gsu;
    cpm->u.hash = paramtab = newparamtable(31, COMPSTATENAME);
    addcompparams(compkparams, compkpms);
    paramtab = tht;
}

/**/
static HashTable
get_compstate(Param pm)
{
    return pm->u.hash;
}

/**/
static void
set_compstate(Param pm, HashTable ht)
{
    struct compparam *cp;
    Param *pp;
    HashNode hn;
    int i;
    struct value v;
    char *str;

    if (!ht)
        return;

    for (i = 0; i < ht->hsize; i++)
	for (hn = ht->nodes[i]; hn; hn = hn->next)
	    for (cp = compkparams,
		 pp = compkpms; cp->name; cp++, pp++)
		if (!strcmp(hn->nam, cp->name)) {
		    v.isarr = v.flags = v.start = 0;
		    v.end = -1;
		    v.arr = NULL;
		    v.pm = (Param) hn;
		    if (cp->type == PM_INTEGER)
			*((zlong *) cp->var) = getintvalue(&v);
		    else if ((str = getstrvalue(&v))) {
			zsfree(*((char **) cp->var));
			*((char **) cp->var) = ztrdup(str);
		    }
		    (*pp)->node.flags &= ~PM_UNSET;

		    break;
		}
    if (ht != pm->u.hash)
	deleteparamtable(ht);
}

/**/
static zlong
get_nmatches(UNUSED(Param pm))
{
    return (permmatches(0) ? 0 : nmatches);
}

/**/
static zlong
get_listlines(UNUSED(Param pm))
{
    return list_lines();
}

/**/
static void
set_complist(UNUSED(Param pm), char *v)
{
    comp_list(v);
}

/**/
static char *
get_complist(UNUSED(Param pm))
{
    return complist;
}

/**/
static char *
get_unambig(UNUSED(Param pm))
{
    return unambig_data(NULL, NULL, NULL);
}

/**/
static zlong
get_unambig_curs(UNUSED(Param pm))
{
    int c;

    unambig_data(&c, NULL, NULL);

    return c;
}

/**/
static char *
get_unambig_pos(UNUSED(Param pm))
{
    char *p;

    unambig_data(NULL, &p, NULL);

    return p;
}

/**/
static char *
get_insert_pos(UNUSED(Param pm))
{
    char *p;

    unambig_data(NULL, NULL, &p);

    return p;
}

/**/
static char *
get_compqstack(UNUSED(Param pm))
{
    char *p, *ptr, *cqp;

    if (!compqstack)		/* TODO: don't think this can happen... */
	return "";

    ptr = p = zhalloc(2*strlen(compqstack)+1);

    for (cqp = compqstack; *cqp; cqp++) {
	char *str = comp_quoting_string(*cqp);
	*ptr++ = *str;
    }
    *ptr = '\0';

    return p;
}

/**/
static void
compunsetfn(Param pm, int exp)
{
    if (exp) {
	if (pm->u.data) {
	    if (PM_TYPE(pm->node.flags) == PM_SCALAR) {
		zsfree(*((char **) pm->u.data));
		*((char **) pm->u.data) = ztrdup("");
	    } else if (PM_TYPE(pm->node.flags) == PM_ARRAY) {
		freearray(*((char ***) pm->u.data));
		*((char ***) pm->u.data) = zshcalloc(sizeof(char *));
	    } else if (PM_TYPE(pm->node.flags) == PM_HASHED) {
		deleteparamtable(pm->u.hash);
		pm->u.hash = NULL;
	    }
	}
    } else if (PM_TYPE(pm->node.flags) == PM_HASHED) {
	Param *p;
	int i;

	deletehashtable(pm->u.hash);
	pm->u.hash = NULL;

	for (p = compkpms, i = CP_KEYPARAMS; i--; p++)
	    *p = NULL;
    }
    if (!exp) {
	Param *p;
	int i;

	for (p = comprpms, i = CP_REALPARAMS; i; p++, i--)
	    if (*p == pm) {
		*p = NULL;
		break;
	    }
    }
}

/**/
void
comp_setunset(int rset, int runset, int kset, int kunset)
{
    Param *p;

    if (comprpms && (rset >= 0 || runset >= 0)) {
	for (p = comprpms; rset || runset; rset >>= 1, runset >>= 1, p++) {
	    if (*p) {
		if (rset & 1)
		    (*p)->node.flags &= ~PM_UNSET;
		if (runset & 1)
		    (*p)->node.flags |= PM_UNSET;
	    }
	}
    }
    if (compkpms && (kset >= 0 || kunset >= 0)) {
	for (p = compkpms; kset || kunset; kset >>= 1, kunset >>= 1, p++) {
	    if (*p) {
		if (kset & 1)
		    (*p)->node.flags &= ~PM_UNSET;
		if (kunset & 1)
		    (*p)->node.flags |= PM_UNSET;
	    }
	}
    }
}

/**/
static int
comp_wrapper(Eprog prog, FuncWrap w, char *name)
{
    if (incompfunc != 1)
	return 1;
    else {
	char *orest, *opre, *osuf, *oipre, *oisuf, **owords, **oredirs;
	char *oqipre, *oqisuf, *oq, *oqi, *oqs, *oaq;
	zlong ocur;
	unsigned int runset = 0, kunset = 0, m, sm;
	Param *pp;

	m = CP_WORDS | CP_REDIRS | CP_CURRENT | CP_PREFIX | CP_SUFFIX | 
	    CP_IPREFIX | CP_ISUFFIX | CP_QIPREFIX | CP_QISUFFIX;
	for (pp = comprpms, sm = 1; m; pp++, m >>= 1, sm <<= 1) {
	    if ((m & 1) && ((*pp)->node.flags & PM_UNSET))
		runset |= sm;
	}
	if (compkpms[CPN_RESTORE]->node.flags & PM_UNSET)
	    kunset = CP_RESTORE;
	orest = comprestore;
	comprestore = ztrdup("auto");
	ocur = compcurrent;
	opre = ztrdup(compprefix);
	osuf = ztrdup(compsuffix);
	oipre = ztrdup(compiprefix);
	oisuf = ztrdup(compisuffix);
	oqipre = ztrdup(compqiprefix);
	oqisuf = ztrdup(compqisuffix);
	oq = ztrdup(compquote);
	oqi = ztrdup(compquoting);
	oqs = ztrdup(compqstack);
	oaq = ztrdup(autoq);
	owords = zarrdup(compwords);
	oredirs = zarrdup(compredirs);

	runshfunc(prog, w, name);

	if (comprestore && !strcmp(comprestore, "auto")) {
	    compcurrent = ocur;
	    zsfree(compprefix);
	    compprefix = opre;
	    zsfree(compsuffix);
	    compsuffix = osuf;
	    zsfree(compiprefix);
	    compiprefix = oipre;
	    zsfree(compisuffix);
	    compisuffix = oisuf;
	    zsfree(compqiprefix);
	    compqiprefix = oqipre;
	    zsfree(compqisuffix);
	    compqisuffix = oqisuf;
	    zsfree(compquote);
	    compquote = oq;
	    zsfree(compquoting);
	    compquoting = oqi;
	    zsfree(compqstack);
	    compqstack = oqs;
	    zsfree(autoq);
	    autoq = oaq;
	    freearray(compwords);
	    freearray(compredirs);
	    compwords = owords;
            compredirs = oredirs;
	    comp_setunset(CP_COMPSTATE |
			  (~runset & (CP_WORDS | CP_REDIRS |
                                      CP_CURRENT | CP_PREFIX |
                                      CP_SUFFIX | CP_IPREFIX | CP_ISUFFIX |
                                      CP_QIPREFIX | CP_QISUFFIX)),
			  (runset & CP_ALLREALS),
			  (~kunset & CP_RESTORE), (kunset & CP_ALLKEYS));
	} else {
	    comp_setunset(CP_COMPSTATE, 0, (~kunset & CP_RESTORE),
			  (kunset & CP_RESTORE));
	    zsfree(opre);
	    zsfree(osuf);
	    zsfree(oipre);
	    zsfree(oisuf);
	    zsfree(oqipre);
	    zsfree(oqisuf);
	    zsfree(oq);
	    zsfree(oqi);
	    zsfree(oqs);
	    zsfree(oaq);
	    freearray(owords);
	    freearray(oredirs);
	}
	zsfree(comprestore);
	comprestore = orest;

	return 0;
    }
}

/**/
static int
comp_check(void)
{
    if (incompfunc != 1) {
	zerr("condition can only be used in completion function");
	return 0;
    }
    return 1;
}

/**/
static int
cond_psfix(char **a, int id)
{
    if (comp_check()) {
	if (a[1])
	    return do_comp_vars(id, cond_val(a, 0), cond_str(a, 1, 1),
				0, NULL, 0);
	else
	    return do_comp_vars(id, -1, cond_str(a, 0, 1), 0, NULL, 0);
    }
    return 0;
}

/**/
static int
cond_range(char **a, int id)
{
    return do_comp_vars(CVT_RANGEPAT, 0, cond_str(a, 0, 1), 0,
			(id ? cond_str(a, 1, 1) : NULL), 0);
}

static struct builtin bintab[] = {
    BUILTIN("compadd", BINF_HANDLES_OPTS, bin_compadd, 0, -1, 0, NULL, NULL),
    BUILTIN("compset", 0, bin_compset, 1, 3, 0, NULL, NULL),
};

static struct conddef cotab[] = {
    CONDDEF("after", 0, cond_range, 1, 1, 0),
    CONDDEF("between", 0, cond_range, 2, 2, 1),
    CONDDEF("prefix", 0, cond_psfix, 1, 2, CVT_PREPAT),
    CONDDEF("suffix", 0, cond_psfix, 1, 2, CVT_SUFPAT),
};

static struct funcwrap wrapper[] = {
    WRAPDEF(comp_wrapper),
};

/* The order of the entries in this table has to match the *HOOK
 * macros in comp.h */

/**/
struct hookdef comphooks[] = {
    HOOKDEF("insert_match", NULL, HOOKF_ALL),
    HOOKDEF("menu_start", NULL, HOOKF_ALL),
    HOOKDEF("compctl_make", NULL, 0),
    HOOKDEF("compctl_cleanup", NULL, 0),
    HOOKDEF("comp_list_matches", ilistmatches, 0),
};

static struct features module_features = {
    bintab, sizeof(bintab)/sizeof(*bintab),
    cotab, sizeof(cotab)/sizeof(*cotab),
    NULL, 0,
    NULL, 0,
    0
};

/**/
int
setup_(UNUSED(Module m))
{
    hasperm = 0;

    comprpms = compkpms = NULL;
    compwords = compredirs = NULL;
    compprefix = compsuffix = compiprefix = compisuffix = 
	compqiprefix = compqisuffix =
	compcontext = compparameter = compredirect = compquote =
	compquoting = comprestore = complist = compinsert =
	compexact = compexactstr = comppatmatch = comppatinsert =
	complastprompt = comptoend = compoldlist = compoldins =
	compvared = compqstack = NULL;
    complastprefix = ztrdup("");
    complastsuffix = ztrdup("");
    complistmax = 0;
    hascompmod = 1;

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
    addhookfunc("complete", (Hookfn) do_completion);
    addhookfunc("before_complete", (Hookfn) before_complete);
    addhookfunc("after_complete", (Hookfn) after_complete);
    addhookfunc("accept_completion", (Hookfn) accept_last);
    addhookfunc("list_matches", (Hookfn) list_matches);
    addhookfunc("invalidate_list", (Hookfn) invalidate_list);
    (void)addhookdefs(m, comphooks, sizeof(comphooks)/sizeof(*comphooks));
    return addwrapper(m, wrapper);
}

/**/
int
cleanup_(Module m)
{
    deletehookfunc("complete", (Hookfn) do_completion);
    deletehookfunc("before_complete", (Hookfn) before_complete);
    deletehookfunc("after_complete", (Hookfn) after_complete);
    deletehookfunc("accept_completion", (Hookfn) accept_last);
    deletehookfunc("list_matches", (Hookfn) list_matches);
    deletehookfunc("invalidate_list", (Hookfn) invalidate_list);
    (void)deletehookdefs(m, comphooks,
			 sizeof(comphooks)/sizeof(*comphooks));
    deletewrapper(m, wrapper);
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    if (compwords)
	freearray(compwords);
    if (compredirs)
	freearray(compredirs);
    zsfree(compprefix);
    zsfree(compsuffix);
    zsfree(complastprefix);
    zsfree(complastsuffix);
    zsfree(compiprefix);
    zsfree(compisuffix);
    zsfree(compqiprefix);
    zsfree(compqisuffix);
    zsfree(compcontext);
    zsfree(compparameter);
    zsfree(compredirect);
    zsfree(compquote);
    zsfree(compqstack);
    zsfree(compquoting);
    zsfree(comprestore);
    zsfree(complist);
    zsfree(compinsert);
    zsfree(compexact);
    zsfree(compexactstr);
    zsfree(comppatmatch);
    zsfree(comppatinsert);
    zsfree(complastprompt);
    zsfree(comptoend);
    zsfree(compoldlist);
    zsfree(compoldins);
    zsfree(compvared);

    hascompmod = 0;

    return 0;
}
