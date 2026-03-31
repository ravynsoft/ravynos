/*
 * zutil.c - misc utilities
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

#include "zutil.mdh"
#include "zutil.pro"

typedef struct {
    char **match;
    char **mbegin;
    char **mend;
} MatchData;

static void
savematch(MatchData *m)
{
    char **a;

    queue_signals();
    a = getaparam("match");
    m->match = a ? zarrdup(a) : NULL;
    a = getaparam("mbegin");
    m->mbegin = a ? zarrdup(a) : NULL;
    a = getaparam("mend");
    m->mend = a ? zarrdup(a) : NULL;
    unqueue_signals();
}

static void
restorematch(MatchData *m)
{
    if (m->match)
	setaparam("match", m->match);
    else
	unsetparam("match");
    if (m->mbegin)
	setaparam("mbegin", m->mbegin);
    else
	unsetparam("mbegin");
    if (m->mend)
	setaparam("mend", m->mend);
    else
	unsetparam("mend");
}

static void
freematch(MatchData *m)
{
    if (m->match)
	freearray(m->match);
    if (m->mbegin)
	freearray(m->mbegin);
    if (m->mend)
	freearray(m->mend);
}

/* Style stuff. */

typedef struct stypat *Stypat;
typedef struct style *Style;

/* A pattern and the styles for it. */

struct style {
    struct hashnode node;
    Stypat pats;		/* patterns, sorted by weight descending, then
                                   by order of definition, newest first. */
};

struct stypat {
    Stypat next;
    char *pat;			/* pattern string */
    Patprog prog;		/* compiled pattern */
    zulong weight;		/* how specific is the pattern? */
    Eprog eval;			/* eval-on-retrieve? */
    char **vals;
};

/* Hash table of styles and associated functions. */

static HashTable zstyletab;

/* Memory stuff. */

static void
freestylepatnode(Stypat p)
{
    zsfree(p->pat);
    freepatprog(p->prog);
    if (p->vals)
	freearray(p->vals);
    if (p->eval)
	freeeprog(p->eval);
    zfree(p, sizeof(*p));
}

static void
freestylenode(HashNode hn)
{
    Style s = (Style) hn;
    Stypat p, pn;

    p = s->pats;
    while (p) {
	pn = p->next;
	freestylepatnode(p);
	p = pn;
    }

    zsfree(s->node.nam);
    zfree(s, sizeof(struct style));
}

/*
 * Free the information for one of the patterns associated with
 * a style.
 *
 * If the style s is passed, prev is the previous pattern in the list,
 * found when scanning.  We use this to update the list of patterns.
 * If this results in their being no remaining patterns, the style
 * itself is removed from the list of styles.  This isn't optimised,
 * since it's not a very frequent operation; we simply scan down the list
 * to find the previous entry.
 */
static void
freestypat(Stypat p, Style s, Stypat prev)
{
    if (s) {
	if (prev)
	    prev->next = p->next;
	else
	    s->pats = p->next;
    }

    freestylepatnode(p);

    if (s && !s->pats) {
	/* No patterns left, free style */
	zstyletab->removenode(zstyletab, s->node.nam);
	zsfree(s->node.nam);
	zfree(s, sizeof(*s));
    }
}

/* Pattern to match context when printing nodes */

static Patprog zstyle_contprog;

/*
 * Print a node.  Print flags as shown.
 */
enum {
    ZSLIST_NONE,
    ZSLIST_BASIC,
    ZSLIST_SYNTAX,
};

static void
printstylenode(HashNode hn, int printflags)
{
    Style s = (Style)hn;
    Stypat p;
    char **v;

    if (printflags == ZSLIST_BASIC) {
	quotedzputs(s->node.nam, stdout);
	putchar('\n');
    }

    for (p = s->pats; p; p = p->next) {
	if (zstyle_contprog && !pattry(zstyle_contprog, p->pat))
	    continue;
	if (printflags == ZSLIST_BASIC)
	    printf("%s  %s", (p->eval ? "(eval)" : "      "), p->pat);
	else {
	    printf("zstyle %s", (p->eval ? "-e " : ""));
	    quotedzputs(p->pat, stdout);
	    putchar(' ');
	    quotedzputs(s->node.nam, stdout);
	}
	for (v = p->vals; *v; v++) {
	    putchar(' ');
	    quotedzputs(*v, stdout);
	}
	putchar('\n');
    }
}

/*
 * Scan the list for a particular pattern, maybe adding matches to
 * the link list (heap memory).  Value to be added as
 * shown in enum
 */
static LinkList zstyle_list;
static char *zstyle_patname;

enum {
    ZSPAT_NAME,		/* Add style names for matched pattern to list */
    ZSPAT_PAT,		/* Add all patterns to list, doesn't use patname */
    ZSPAT_REMOVE,	/* Remove matched pattern, doesn't use list */
};

static void
scanpatstyles(HashNode hn, int spatflags)
{
    Style s = (Style)hn;
    Stypat p, q;
    LinkNode n;

    for (q = NULL, p = s->pats; p; q = p, p = p->next) {
	switch (spatflags) {
	case ZSPAT_NAME:
	    if (!strcmp(p->pat, zstyle_patname)) {
		addlinknode(zstyle_list, s->node.nam);
		return;
	    }
	    break;

	case ZSPAT_PAT:
	    /* Check pattern isn't already there */
	    for (n = firstnode(zstyle_list); n; incnode(n))
		if (!strcmp(p->pat, (char *) getdata(n)))
		    break;
	    if (!n)
		addlinknode(zstyle_list, p->pat);
	    break;

	case ZSPAT_REMOVE:
	    if (!strcmp(p->pat, zstyle_patname)) {
		freestypat(p, s, q);
		/*
		 * May remove link node itself; that's OK
		 * when scanning but we need to make sure
		 * we don't look at it any more.
		 */
		return;
	    }
	    break;
	}
    }
}


static HashTable
newzstyletable(int size, char const *name)
{
    HashTable ht;
    ht = newhashtable(size, name, NULL);

    ht->hash        = hasher;
    ht->emptytable  = emptyhashtable;
    ht->filltable   = NULL;
    ht->cmpnodes    = strcmp;
    ht->addnode     = addhashnode;
    /* DISABLED is not supported */
    ht->getnode     = gethashnode2;
    ht->getnode2    = gethashnode2;
    ht->removenode  = removehashnode;
    ht->disablenode = NULL;
    ht->enablenode  = NULL;
    ht->freenode    = freestylenode;
    ht->printnode   = printstylenode;

    return ht;
}

/* Store a value for a style. */

static int
setstypat(Style s, char *pat, Patprog prog, char **vals, int eval)
{
    zulong weight;
    int tmp;
    int first;
    char *str;
    Stypat p, q, qq;
    Eprog eprog = NULL;

    if (eval) {
	int ef = errflag;

	eprog = parse_string(zjoin(vals, ' ', 1), 0);
	/* Keep any user interrupt error status */
	errflag = ef | (errflag & ERRFLAG_INT);

	if (!eprog)
	{
	    freepatprog(prog);
	    return 1;
	}

	eprog = dupeprog(eprog, 0);
    }
    for (p = s->pats; p; p = p->next)
	if (!strcmp(pat, p->pat)) {

	    /* Exists -> replace. */

	    if (p->vals)
		freearray(p->vals);
	    if (p->eval)
		freeeprog(p->eval);
	    p->vals = zarrdup(vals);
	    p->eval = eprog;
	    freepatprog(prog);

	    return 0;
	}

    /* New pattern. */

    p = (Stypat) zalloc(sizeof(*p));
    p->pat = ztrdup(pat);
    p->prog = prog;
    p->vals = zarrdup(vals);
    p->eval = eprog;
    p->next = NULL;

    /* Calculate the weight.
     *
     * The weight of a pattern is scored as follows:
     *
     * - The pattern is split to colon-separated components.
     * - A component equal to '*' (with nothing else) scores 0 points.
     * - A component that's a pattern, otherwise, scores 1 point.
     * - A component that's a literal string scores 2 points.
     * - The score of a pattern is the sum of the score of its components.
     *
     * The result of this calculation is stored in the low bits of 'weight'.
     * The high bits of 'weight' are used to store the number of ':'-separated
     * components.  This provides a lexicographic comparison: first compare
     * the number of components, and if that's equal, compare the specificity
     * of the components.
     *
     * This corresponds to the notion of 'more specific' in the zshmodules(1)
     * documentation of zstyle.
     */

    for (weight = 0, tmp = 2, first = 1, str = pat; *str; str++) {
	if (first && *str == '*' && (!str[1] || str[1] == ':')) {
	    /* Only `*' in this component. */
	    tmp = 0;
	    continue;
	}
	first = 0;

	if (*str == '(' || *str == '|' || *str == '*' || *str == '[' ||
	    *str == '<' ||  *str == '?' || *str == '#' || *str == '^')
	    /* Is pattern. */
	    tmp = 1;

	if (*str == ':') {
	    /* Yet another component. */
	    weight += ZLONG_CONST(1) << (CHAR_BIT * sizeof(weight) / 2);

	    first = 1;
	    weight += tmp;
	    tmp = 2;
	}
    }
    p->weight = (weight += tmp);

    /* Insert 'q' to 's->pats', using 'qq' as a temporary. */
    for (qq = NULL, q = s->pats; q && q->weight >= weight;
	 qq = q, q = q->next);
    p->next = q;
    if (qq)
	qq->next = p;
    else
	s->pats = p;

    return 0;
}

/* Add a new style. */

static Style
addstyle(char *name)
{
    Style s = (Style) zshcalloc(sizeof(*s));

    zstyletab->addnode(zstyletab, ztrdup(name), s);

    return s;
}

static char **
evalstyle(Stypat p)
{
    int ef = errflag;
    char **ret, *str;

    unsetparam("reply");
    execode(p->eval, 1, 0, "style");
    if (errflag) {
	/* Keep any user interrupt error status */
	errflag = ef | (errflag & ERRFLAG_INT);
	return NULL;
    }
    errflag = ef | (errflag & ERRFLAG_INT);

    queue_signals();
    if ((ret = getaparam("reply")))
	ret = arrdup(ret);
    else if ((str = getsparam("reply"))) {
	ret = (char **) hcalloc(2 * sizeof(char *));
	ret[0] = dupstring(str);
    }
    unqueue_signals();
    unsetparam("reply");

    return ret;
}

/* Look up a style for a context pattern. This does the matching. */

static char **
lookupstyle(char *ctxt, char *style)
{
    Style s;
    Stypat p;
    char **found = NULL;

    s = (Style)zstyletab->getnode2(zstyletab, style);
    if (s) {
	MatchData match;
	savematch(&match);
	for (p = s->pats; p; p = p->next)
	    if (pattry(p->prog, ctxt)) {
		found = (p->eval ? evalstyle(p) : p->vals);
		break;
	    }
	restorematch(&match);
    }

    return found;
}

static int
bin_zstyle(char *nam, char **args, UNUSED(Options ops), UNUSED(int func))
{
    int min, max, n, add = 0, list = ZSLIST_NONE, eval = 0;

    if (!args[0])
	list = ZSLIST_BASIC;
    else if (args[0][0] == '-') {
	char oc;

	if ((oc = args[0][1]) && oc != '-') {
	    if (args[0][2]) {
		zwarnnam(nam, "invalid argument: %s", args[0]);
		return 1;
	    }
	    if (oc == 'L') {
		list = ZSLIST_SYNTAX;
		args++;
	    } else if (oc == 'e') {
		eval = add = 1;
		args++;
	    }
	} else {
	    add = 1;
	    args++;
	}
    } else
	add = 1;

    if (add) {
	Style s;
	Patprog prog;
	char *pat;

	if (arrlen_lt(args, 2)) {
	    zwarnnam(nam, "not enough arguments");
	    return 1;
	}
	pat = dupstring(args[0]);
	tokenize(pat);

	if (!(prog = patcompile(pat, PAT_ZDUP, NULL))) {
	    zwarnnam(nam, "invalid pattern: %s", args[0]);
	    return 1;
	}
	if (!(s = (Style)zstyletab->getnode2(zstyletab, args[1])))
	    s = addstyle(args[1]);
	return setstypat(s, args[0], prog, args + 2, eval);
    }
    if (list) {
	Style s;
	char *context, *stylename;

	switch (arrlen_ge(args, 3) ? 3 : arrlen(args)) {
	case 2:
	    context = args[0];
	    stylename = args[1];
	    break;

	case 1:
	    context = args[0];
	    stylename = NULL;
	    break;

	case 0:
	    context = stylename = NULL;
	    break;

	default:
	    zwarnnam(nam, "too many arguments");
	    return 1;
	}

	queue_signals();	/* Protect PAT_STATIC */

	if (context) {
	    tokenize(context);
	    zstyle_contprog = patcompile(context, PAT_STATIC, NULL);

	    if (!zstyle_contprog) {
		unqueue_signals();
		return 1;
	    }
	} else
	    zstyle_contprog = NULL;

	if (stylename) {
	    s = (Style)zstyletab->getnode2(zstyletab, stylename);
	    if (!s) {
		unqueue_signals();
		return 1;
	    }
	    zstyletab->printnode(&s->node, list);
	} else {
	    scanhashtable(zstyletab, 1, 0, 0,
			  zstyletab->printnode, list);
	}

	unqueue_signals();
	return 0;
    }
    switch (args[0][1]) {
    case 'd': min = 0; max = -1; break;
    case 's': min = 3; max =  4; break;
    case 'b': min = 3; max =  3; break;
    case 'a': min = 3; max =  3; break;
    case 't': min = 2; max = -1; break;
    case 'T': min = 2; max = -1; break;
    case 'm': min = 3; max =  3; break;
    case 'g': min = 1; max =  3; break;
    default:
	zwarnnam(nam, "invalid option: %s", args[0]);
	return 1;
    }
    n = arrlen(args) - 1;
    if (n < min) {
	zwarnnam(nam, "not enough arguments");
	return 1;
    } else if (max >= 0 && n > max) {
	zwarnnam(nam, "too many arguments");
	return 1;
    }
    switch (args[0][1]) {
    case 'd':
	{
	    Style s;

	    if (args[1]) {
		if (args[2]) {
		    char *pat = args[1];

		    for (args += 2; *args; args++) {
			if ((s = (Style)zstyletab->getnode2(zstyletab,
							    *args))) {
			    Stypat p, q;

			    for (q = NULL, p = s->pats; p;
				 q = p, p = p->next) {
				if (!strcmp(p->pat, pat)) {
				    freestypat(p, s, q);
				    break;
				}
			    }
			}
		    }
		} else {
		    zstyle_patname = args[1];

		    /* sorting not needed for deletion */
		    scanhashtable(zstyletab, 0, 0, 0, scanpatstyles,
				  ZSPAT_REMOVE);
		}
	    } else
		zstyletab->emptytable(zstyletab);
	}
	break;
    case 's':
	{
	    char **vals, *ret;
	    int val;

	    if ((vals = lookupstyle(args[1], args[2])) && vals[0]) {
		ret = sepjoin(vals, (args[4] ? args[4] : " "), 0);
		val = 0;
	    } else {
		ret = ztrdup("");
		val = 1;
	    }
	    setsparam(args[3], ret);

	    return val;
	}
	break;
    case 'b':
	{
	    char **vals, *ret;
	    int val;

	    if ((vals = lookupstyle(args[1], args[2])) &&
		vals[0] && !vals[1] &&
		(!strcmp(vals[0], "yes") ||
		 !strcmp(vals[0], "true") ||
		 !strcmp(vals[0], "on") ||
		 !strcmp(vals[0], "1"))) {
		ret = "yes";
		val = 0;
	    } else {
		ret = "no";
		val = 1;
	    }
	    setsparam(args[3], ztrdup(ret));

	    return val;
	}
	break;
    case 'a':
	{
	    char **vals, **ret;
	    int val;

	    if ((vals = lookupstyle(args[1], args[2]))) {
		ret = zarrdup(vals);
		val = 0;
	    } else {
		char *dummy = NULL;

		ret = zarrdup(&dummy);
		val = 1;
	    }
	    setaparam(args[3], ret);

	    return val;
	}
	break;
    case 't':
    case 'T':
	{
	    char **vals;

	    if ((vals = lookupstyle(args[1], args[2])) && vals[0]) {
		if (args[3]) {
		    char **ap = args + 3, **p;

		    while (*ap) {
			p = vals;
			while (*p)
			    if (!strcmp(*ap, *p++))
				return 0;
			ap++;
		    }
		    return 1;
		} else
		    return !(!strcmp(vals[0], "true") ||
			     !strcmp(vals[0], "yes") ||
			     !strcmp(vals[0], "on") ||
			     !strcmp(vals[0], "1"));
	    }
	    return (args[0][1] == 't' ? (vals ? 1 : 2) : 0);
	}
	break;
    case 'm':
	{
	    char **vals;
	    Patprog prog;

	    queue_signals();	/* Protect PAT_STATIC */

	    tokenize(args[3]);

	    if ((vals = lookupstyle(args[1], args[2])) &&
		(prog = patcompile(args[3], PAT_STATIC, NULL))) {
		while (*vals)
		    if (pattry(prog, *vals++)) {
			unqueue_signals();
			return 0;
		    }
	    }

	    unqueue_signals();
	    return 1;
	}
	break;
    case 'g':
	{
	    int ret = 1;
	    Style s;
	    Stypat p;

	    zstyle_list = newlinklist();

	    if (args[2]) {
		if (args[3]) {
		    if ((s = (Style)zstyletab->getnode2(zstyletab, args[3]))) {
			for (p = s->pats; p; p = p->next) {
			    if (!strcmp(args[2], p->pat)) {
				char **v = p->vals;

				while (*v)
				    addlinknode(zstyle_list, *v++);

				ret = 0;
				break;
			    }
			}
		    }
		} else {
		    zstyle_patname = args[2];
		    scanhashtable(zstyletab, 1, 0, 0, scanpatstyles,
				  ZSPAT_NAME);
		    ret = 0;
		}
	    } else {
		scanhashtable(zstyletab, 1, 0, 0, scanpatstyles,
			      ZSPAT_PAT);
		ret = 0;
	    }
	    set_list_array(args[1], zstyle_list);

	    return ret;
	}
    }
    return 0;
}

/* Format stuff. */

/*
 * One chunk of text, to allow recursive handling of ternary
 * expressions in zformat -f output.
 *   instr	The input string.
 *   specs	The format specifiers, specs[c] is the string from c:string
 *   outp	*outp is the start of the output string
 *   ousedp	(*outp)[*ousedp] is where to write next
 *   olenp	*olenp is the size allocated for *outp
 *   endchar    Terminator character in addition to `\0' (may be '\0')
 *   presence   -F: Ternary expressions test emptyness instead
 *   skip	If 1, don't output, just parse.
 */
static char *zformat_substring(char* instr, char **specs, char **outp,
			       int *ousedp, int *olenp, int endchar,
			       int presence, int skip)
{
    char *s;

    for (s = instr; *s && *s != endchar; s++) {
	if (*s == '%') {
	    int right, min = -1, max = -1, outl, testit;
	    char *spec, *start = s;

	    if ((right = (*++s == '-')))
		s++;

	    if (idigit(*s)) {
		for (min = 0; idigit(*s); s++)
		    min = (min * 10) + (int) STOUC(*s) - '0';
	    }

	    /* Ternary expressions */
	    testit = (STOUC(*s) == '(');
	    if (testit && s[1] == '-')
	    {
		/* Allow %(-1... etc. */
		right = 1;
		s++;
	    }
	    if ((*s == '.' || testit) && idigit(s[1])) {
		for (max = 0, s++; idigit(*s); s++)
		    max = (max * 10) + (int) STOUC(*s) - '0';
	    } else if (*s == '.' || testit)
		s++;

	    if (testit && STOUC(*s)) {
		int actval, testval, endcharl;

		/* Only one number is useful for ternary expressions. */
		testval = (min >= 0) ? min : (max >= 0) ? max : 0;

		if (specs[STOUC(*s)] && *specs[STOUC(*s)]) {
		    if (presence) {
			if (testval)
#ifdef MULTIBYTE_SUPPORT
			    if (isset(MULTIBYTE))
				actval = MB_METASTRWIDTH(specs[STOUC(*s)]);
			    else
#endif
				actval = strlen(specs[STOUC(*s)]);
		        else
			    actval = 1;
			actval = right ? (testval < actval) : (testval >= actval);
		    } else {
			if (right) /* put the sign back */
			    testval *= -1;
			/* zero means values are equal, i.e. true */
			actval = (int)mathevali(specs[STOUC(*s)]) - testval;
		    }
		} else
		    actval = presence ? !right : testval;

		/* careful about premature end of string */
		if (!(endcharl = *++s))
		    return NULL;

		/*
		 * Either skip true text and output false text, or
		 * vice versa... unless we are already skipping.
		 */
		if (!(s = zformat_substring(s+1, specs, outp, ousedp,
			    olenp, endcharl, presence, skip || actval)))
		    return NULL;
		if (!(s = zformat_substring(s+1, specs, outp, ousedp,
			    olenp, ')', presence, skip || !actval)))
		    return NULL;
	    } else if (skip) {
		continue;
	    } else if ((spec = specs[STOUC(*s)])) {
		int len;

		if ((len = strlen(spec)) > max && max >= 0)
		    len = max;
		outl = (min >= 0 ? (min > len ? min : len) : len);

		if (*ousedp + outl >= *olenp) {
		    int nlen = *olenp + outl + 128;
		    char *tmp = (char *) zhalloc(nlen);

		    memcpy(tmp, *outp, *olenp);
		    *olenp = nlen;
		    *outp = tmp;
		}
		if (len >= outl) {
		    memcpy(*outp + *ousedp, spec, outl);
		    *ousedp += outl;
		} else {
		    int diff = outl - len;

		    if (right) {
			while (diff--)
			    (*outp)[(*ousedp)++] = ' ';
			memcpy(*outp + *ousedp, spec, len);
			*ousedp += len;
		    } else {
			memcpy(*outp + *ousedp, spec, len);
			*ousedp += len;
			while (diff--)
			    (*outp)[(*ousedp)++] = ' ';
		    }
		}
	    } else {
		int len = s - start + 1;

		if (*ousedp + len >= *olenp) {
		    int nlen = *olenp + len + 128;
		    char *tmp = (char *) zhalloc(nlen);

		    memcpy(tmp, *outp, *olenp);
		    *olenp = nlen;
		    *outp = tmp;
		}
		memcpy(*outp + *ousedp, start, len);
		*ousedp += len;
	    }
	} else {
	    if (skip)
		continue;
	    if (*ousedp + 1 >= *olenp) {
		char *tmp = (char *) zhalloc((*olenp) << 1);

		memcpy(tmp, *outp, *olenp);
		*olenp <<= 1;
		*outp = tmp;
	    }
	    (*outp)[(*ousedp)++] = *s;
	}
    }

    return s;
}

static int
bin_zformat(char *nam, char **args, UNUSED(Options ops), UNUSED(int func))
{
    char opt;
    int presence = 0;

    if (args[0][0] != '-' || !(opt = args[0][1]) || args[0][2]) {
	zwarnnam(nam, "invalid argument: %s", args[0]);
	return 1;
    }
    args++;

    switch (opt) {
    case 'F':
	presence = 1;
	/* fall-through */
    case 'f':
	{
	    char **ap, *specs[256] = {0}, *out;
	    int olen, oused = 0;

	    specs['%'] = "%";
	    specs[')'] = ")";

	    /* Parse the specs in argv. */
	    for (ap = args + 2; *ap; ap++) {
		if (!ap[0][0] || ap[0][0] == '-' || ap[0][0] == '.' ||
		    idigit(ap[0][0]) || ap[0][1] != ':') {
		    zwarnnam(nam, "invalid argument: %s", *ap);
		    return 1;
		}
		specs[STOUC(ap[0][0])] = ap[0] + 2;
	    }
	    out = (char *) zhalloc(olen = 128);

	    zformat_substring(args[1], specs, &out, &oused, &olen, '\0',
		    presence, 0);
	    out[oused] = '\0';

	    setsparam(args[0], ztrdup(out));
	    return 0;
	}
	break;
    case 'a':
	{
	    char **ap, *cp;
	    int nbc = 0, colon = 0, pre = 0, suf = 0;
#ifdef MULTIBYTE_SUPPORT
	    int prechars = 0;
#endif /* MULTIBYTE_SUPPORT */

	    for (ap = args + 2; *ap; ap++) {
		for (nbc = 0, cp = *ap; *cp && *cp != ':'; cp++)
		    if (*cp == '\\' && cp[1])
			cp++, nbc++;
		if (*cp == ':' && cp[1]) {
		    int d;
#ifdef MULTIBYTE_SUPPORT
		    int dchars = 0;
#endif /* MULTIBYTE_SUPPORT */

		    colon++;
		    if ((d = cp - *ap - nbc) > pre)
			pre = d;
#ifdef MULTIBYTE_SUPPORT
		    if (isset(MULTIBYTE)) {
			*cp = '\0';
			dchars = MB_METASTRWIDTH(*ap) - nbc;
			*cp = ':';
		    } else
			dchars = d;
		    if (dchars > prechars)
			prechars = dchars;
#endif /* MULTIBYTE_SUPPORT */
		    if ((d = strlen(cp + 1)) > suf)
			suf = d;
		}
	    }
	    {
		int sl = strlen(args[1]);
		VARARR(char, buf, pre + suf + sl + 1);
		char **ret, **rp, *copy, *cpp, oldc;

		ret = (char **) zalloc((arrlen(args + 2) + 1) *
				       sizeof(char *));

#ifndef MULTIBYTE_SUPPORT
		memcpy(buf + pre, args[1], sl);
		suf = pre + sl;
#endif /* MULTIBYTE_SUPPORT */

		for (rp = ret, ap = args + 2; *ap; ap++) {
		    copy = dupstring(*ap);
		    for (cp = cpp = copy; *cp && *cp != ':'; cp++) {
			if (*cp == '\\' && cp[1])
			    cp++;
			*cpp++ = *cp;
		    }
		    oldc = *cpp;
		    *cpp = '\0';
		    if (((cpp == cp && oldc == ':') || *cp == ':') && cp[1]) {
#ifdef MULTIBYTE_SUPPORT
			int rempad;
			char *ptr;
			memcpy(buf, copy, (cpp - copy));
			*cp = '\0';
			if (isset(MULTIBYTE))
			    rempad = prechars - MB_METASTRWIDTH(copy);
			else
			    rempad = prechars - strlen(copy);
			ptr = buf + (cpp - copy);
			if (rempad)
			    memset(ptr, ' ', rempad);
			ptr += rempad;
			memcpy(ptr, args[1], sl);
			ptr += sl;
			strcpy(ptr, cp + 1);
#else /* MULTIBYTE_SUPPORT */
			memset(buf, ' ', pre);
			memcpy(buf, copy, (cpp - copy));
			strcpy(buf + suf, cp + 1);
#endif /* MULTIBYTE_SUPPORT */
			*rp++ = ztrdup(buf);
		    } else
			*rp++ = ztrdup(copy);
		}
		*rp = NULL;

		setaparam(args[0], ret);
		return 0;
	    }
	}
	break;
    }
    zwarnnam(nam, "invalid option: -%c", opt);
    return 1;
}

/* Zregexparse stuff. */

typedef struct {
    int cutoff;
    char *pattern;
    Patprog patprog;
    char *guard;
    char *action;
    LinkList branches;
} RParseState;

typedef struct {
    RParseState *state;
    LinkList actions;
} RParseBranch;

typedef struct {
    LinkList nullacts;
    LinkList in;
    LinkList out;
} RParseResult;

static char **rparseargs;
static LinkList rparsestates;

static int rparsealt(RParseResult *result, jmp_buf *perr);

static void
connectstates(LinkList out, LinkList in)
{
    LinkNode outnode, innode, ln;

    for (outnode = firstnode(out); outnode; outnode = nextnode(outnode)) {
	RParseBranch *outbranch = getdata(outnode);

	for (innode = firstnode(in); innode; innode = nextnode(innode)) {
	    RParseBranch *inbranch = getdata(innode);
	    RParseBranch *br = hcalloc(sizeof(*br));

	    br->state = inbranch->state;
	    br->actions = newlinklist();
	    for (ln = firstnode(outbranch->actions); ln; ln = nextnode(ln))
		addlinknode(br->actions, getdata(ln));
	    for (ln = firstnode(inbranch->actions); ln; ln = nextnode(ln))
		addlinknode(br->actions, getdata(ln));
	    addlinknode(outbranch->state->branches, br);
	}
    }
}

static int
rparseelt(RParseResult *result, jmp_buf *perr)
{
    int l;
    char *s = *rparseargs;

    if (!s)
        return 1;

    switch (s[0]) {
    case '/': {
	RParseState *st;
	RParseBranch *br;
	char *pattern, *lookahead;
	int patternlen, lookaheadlen = 0;

	l = strlen(s);
	if (!((2 <= l && s[l - 1] == '/') ||
	      (3 <= l && s[l - 2] == '/' && (s[l - 1] == '+' ||
					     s[l - 1] == '-'))))
	    return 1;
	st = hcalloc(sizeof(*st));
	st->branches = newlinklist();
	st->cutoff = s[l - 1];
	if (s[l - 1] == '/') {
	    pattern = s + 1;
	    patternlen = l - 2;
	} else {
	    pattern = s + 1;
	    patternlen = l - 3;
	}
	rparseargs++;
	if ((s = *rparseargs) && s[0] == '%' &&
	   2 <= (l = strlen(s)) && s[l - 1] == '%') {
	    rparseargs++;
	    lookahead = s + 1;
	    lookaheadlen = l - 2;
	} else {
	    lookahead = NULL;
	}
	if (patternlen == 2 && !strncmp(pattern, "[]", 2))
	    st->pattern = NULL;
	else {
	    char *cp;
	    int l = patternlen + 12; /* (#b)((#B)...)...* */
	    if(lookahead)
	        l += lookaheadlen + 4; /* (#B)... */
	    cp = st->pattern = hcalloc(l);
	    strcpy(cp, "(#b)((#B)");
	    cp += 9;
	    strcpy(cp, pattern);
	    cp += patternlen;
	    strcpy(cp, ")");
	    cp += 1;
	    if (lookahead) {
		strcpy(cp, "(#B)");
		cp += 4;
		strcpy(cp, lookahead);
		cp += lookaheadlen;
	    }
	    strcpy(cp, "*");
	}
	st->patprog = NULL;
	if ((s = *rparseargs) && *s == '-') {
	    rparseargs++;
	    l = strlen(s);
	    st->guard = hcalloc(l);
	    memcpy(st->guard, s + 1, l - 1);
	    st->guard[l - 1] = '\0';
	} else
	    st->guard = NULL;
	if ((s = *rparseargs) && *s == ':') {
	    rparseargs++;
	    l = strlen(s);
	    st->action = hcalloc(l);
	    memcpy(st->action, s + 1, l - 1);
	    st->action[l - 1] = '\0';
	} else
	    st->action = NULL;
	result->nullacts = NULL;
	result->in = newlinklist();
	br = hcalloc(sizeof(*br));
	br->state = st;
	br->actions = newlinklist();
	addlinknode(result->in, br);
	result->out = newlinklist();
	br = hcalloc(sizeof(*br));
	br->state = st;
	br->actions = newlinklist();
	addlinknode(result->out, br);
	break;
    }
    case '(':
	if (s[1])
	    return 1;
	rparseargs++;
	if (rparsealt(result, perr))
	    longjmp(*perr, 2);
	s = *rparseargs;
	if (!s || s[0] != ')' || s[1] != '\0')
	    longjmp(*perr, 2);
	rparseargs++;
        break;
    default:
        return 1;
    }

    return 0;
}

static int
rparseclo(RParseResult *result, jmp_buf *perr)
{
    if (rparseelt(result, perr))
	return 1;

    if (*rparseargs && !strcmp(*rparseargs, "#")) {
	rparseargs++;
	while (*rparseargs && !strcmp(*rparseargs, "#"))
	    rparseargs++;

	connectstates(result->out, result->in);
	result->nullacts = newlinklist();
    }
    return 0;
}

static void
prependactions(LinkList acts, LinkList branches)
{
    LinkNode aln, bln;

    for (bln = firstnode(branches); bln; bln = nextnode(bln)) {
	RParseBranch *br = getdata(bln);

	for (aln = lastnode(acts); aln != (LinkNode)acts; aln = prevnode(aln))
	    pushnode(br->actions, getdata(aln));
    }
}

static void
appendactions(LinkList acts, LinkList branches)
{
    LinkNode aln, bln;
    for (bln = firstnode(branches); bln; bln = nextnode(bln)) {
	RParseBranch *br = getdata(bln);

	for (aln = firstnode(acts); aln; aln = nextnode(aln))
	    addlinknode(br->actions, getdata(aln));
    }
}

static int
rparseseq(RParseResult *result, jmp_buf *perr)
{
    int l;
    char *s;
    RParseResult sub;

    result->nullacts = newlinklist();
    result->in = newlinklist();
    result->out = newlinklist();

    while (1) {
	if ((s = *rparseargs) && s[0] == '{' && s[(l = strlen(s)) - 1] == '}') {
	    char *action = hcalloc(l - 1);
	    LinkNode ln;

	    rparseargs++;
	    memcpy(action, s + 1, l - 2);
	    action[l - 2] = '\0';
	    if (result->nullacts)
		addlinknode(result->nullacts, action);
	    for (ln = firstnode(result->out); ln; ln = nextnode(ln)) {
		RParseBranch *br = getdata(ln);
		addlinknode(br->actions, action);
	    }
	}
        else if (!rparseclo(&sub, perr)) {
	    connectstates(result->out, sub.in);

	    if (result->nullacts) {
		prependactions(result->nullacts, sub.in);
		insertlinklist(sub.in, lastnode(result->in), result->in);
	    }
	    if (sub.nullacts) {
		appendactions(sub.nullacts, result->out);
		insertlinklist(sub.out, lastnode(result->out), result->out);
	    } else
		result->out = sub.out;

	    if (result->nullacts && sub.nullacts)
		insertlinklist(sub.nullacts, lastnode(result->nullacts),
			       result->nullacts);
	    else
		result->nullacts = NULL;
	}
	else
	    break;
    }
    return 0;
}

static int
rparsealt(RParseResult *result, jmp_buf *perr)
{
    RParseResult sub;

    if (rparseseq(result, perr))
	return 1;

    while (*rparseargs && !strcmp(*rparseargs, "|")) {
	rparseargs++;
	if (rparseseq(&sub, perr))
	    longjmp(*perr, 2);
	if (!result->nullacts && sub.nullacts)
	    result->nullacts = sub.nullacts;

	insertlinklist(sub.in, lastnode(result->in), result->in);
	insertlinklist(sub.out, lastnode(result->out), result->out);
    }
    return 0;
}

static int
rmatch(RParseResult *sm, char *subj, char *var1, char *var2, int comp)
{
    LinkNode ln, lnn;
    LinkList nexts;
    LinkList nextslist;
    RParseBranch *br;
    RParseState *st = NULL;
    int point1 = 0, point2 = 0;

    setiparam(var1, point1);
    setiparam(var2, point2);

    if (!comp && !*subj && sm->nullacts) {
	for (ln = firstnode(sm->nullacts); ln; ln = nextnode(ln)) {
	    char *action = getdata(ln);

	    if (action)
		execstring(action, 1, 0, "zregexparse-action");
	}
	return 0;
    }

    nextslist = newlinklist();
    nexts = sm->in;
    addlinknode(nextslist, nexts);
    do {
	MatchData match1, match2;

	savematch(&match1);

	for (ln = firstnode(nexts); ln; ln = nextnode(ln)) {
	    int i;
	    RParseState *next;

	    br = getdata(ln);
	    next = br->state;
	    if (next->pattern && !next->patprog) {
	        tokenize(next->pattern);
		if (!(next->patprog = patcompile(next->pattern, 0, NULL)))
		    return 3;
	    }
	    if (next->pattern && pattry(next->patprog, subj) &&
		(!next->guard || (execstring(next->guard, 1, 0,
					     "zregexparse-guard"), !lastval))) {
		LinkNode aln;
		char **mend;
		int len;

		queue_signals();
		mend = getaparam("mend");
		len = atoi(mend[0]);
		unqueue_signals();

		for (i = len; i; i--)
		  if (*subj++ == Meta)
		    subj++;

		savematch(&match2);
		restorematch(&match1);

		for (aln = firstnode(br->actions); aln; aln = nextnode(aln)) {
		    char *action = getdata(aln);

		    if (action)
			execstring(action, 1, 0, "zregexparse-action");
		}
		restorematch(&match2);

		point2 += len;
		setiparam(var2, point2);
		st = br->state;
		nexts = st->branches;
		if (next->cutoff == '-' || (next->cutoff == '/' && len)) {
		    nextslist = newlinklist();
		    point1 = point2;
		    setiparam(var1, point1);
		}
		addlinknode(nextslist, nexts);
		break;
	    }
	}
	if (!ln)
	    freematch(&match1);
    } while (ln);

    if (!comp && !*subj)
	for (ln = firstnode(sm->out); ln; ln = nextnode(ln)) {
	    br = getdata(ln);
	    if (br->state == st) {
		for (ln = firstnode(br->actions); ln; ln = nextnode(ln)) {
		    char *action = getdata(ln);

		    if (action)
			execstring(action, 1, 0, "zregexparse-action");
		}
		return 0;
	    }
	}

    for (lnn = firstnode(nextslist); lnn; lnn = nextnode(lnn)) {
	nexts = getdata(lnn);
	for (ln = firstnode(nexts); ln; ln = nextnode(ln)) {
	    br = getdata(ln);
	    if (br->state->action)
		execstring(br->state->action, 1, 0, "zregexparse-action");
	}
    }
    return empty(nexts) ? 2 : 1;
}

/*
  usage: zregexparse [-c] var1 var2 string regex...
  status:
    0: matched
    1: unmatched (all next state candidates are failed)
    2: unmatched (there is no next state candidates)
    3: regex parse error
*/

static int
bin_zregexparse(char *nam, char **args, Options ops, UNUSED(int func))
{
    int oldextendedglob = opts[EXTENDEDGLOB];
    char *var1 = args[0];
    char *var2 = args[1];
    char *subj = args[2];
    int ret;
    jmp_buf rparseerr;
    RParseResult result;

    opts[EXTENDEDGLOB] = 1;

    rparseargs = args + 3;

    pushheap();
    rparsestates = newlinklist();
    if (setjmp(rparseerr) || rparsealt(&result, &rparseerr) || *rparseargs) {
	if (*rparseargs)
	    zwarnnam(nam, "invalid regex : %s", *rparseargs);
	else
	    zwarnnam(nam, "not enough regex arguments");
	ret = 3;
    } else
	ret = 0;

    if (!ret)
	ret = rmatch(&result, subj, var1, var2, OPT_ISSET(ops,'c'));
    popheap();

    opts[EXTENDEDGLOB] = oldextendedglob;
    return ret;
}

typedef struct zoptdesc *Zoptdesc;
typedef struct zoptarr *Zoptarr;
typedef struct zoptval *Zoptval;

struct zoptdesc {
    Zoptdesc next;
    char *name;
    int flags;
    Zoptarr arr;
    Zoptval vals, last;
};

#define ZOF_ARG  1
#define ZOF_OPT  2
#define ZOF_MULT 4
#define ZOF_SAME 8
#define ZOF_MAP 16
#define ZOF_CYC 32

struct zoptarr {
    Zoptarr next;
    char *name;
    Zoptval vals, last;
    int num;
};

struct zoptval {
    Zoptval next, onext;
    char *name;
    char *arg;
    char *str;
};

static Zoptdesc opt_descs;
static Zoptarr opt_arrs;

static Zoptdesc
get_opt_desc(char *name)
{
    Zoptdesc p;

    for (p = opt_descs; p; p = p->next)
	if (!strcmp(name, p->name))
	    return p;

    return NULL;
}

static Zoptdesc
lookup_opt(char *str)
{
    Zoptdesc p;

    for (p = opt_descs; p; p = p->next) {
	if ((p->flags & ZOF_ARG) ? strpfx(p->name, str) : !strcmp(p->name, str))
	    return p;
    }
    return NULL;
}

static Zoptarr
get_opt_arr(char *name)
{
    Zoptarr p;

    for (p = opt_arrs; p; p = p->next)
	if (!strcmp(name, p->name))
	    return p;

    return NULL;
}

static Zoptdesc
map_opt_desc(Zoptdesc start)
{
    Zoptdesc map = NULL;

    if (!start || !(start->flags & ZOF_MAP))
	return start;

    map = get_opt_desc(start->arr->name);

    if (!map)
	return start;

    if (map == start) {
	start->flags &= ~ZOF_MAP;	/* optimize */
	return start;
    }

    if (map->flags & ZOF_CYC)
	return NULL;

    start->flags |= ZOF_CYC;
    map = map_opt_desc(map);
    start->flags &= ~ZOF_CYC;

    return map;
}

static void
add_opt_val(Zoptdesc d, char *arg)
{
    Zoptval v = NULL;
    char *n = dyncat("-", d->name);
    int new = 0;

    Zoptdesc map = map_opt_desc(d);
    if (map)
	d = map;

    if (!(d->flags & ZOF_MULT))
	v = d->vals;
    if (!v) {
	v = (Zoptval) zhalloc(sizeof(*v));
	v->next = v->onext = NULL;
	v->name = n;
	new = 1;
    }
    v->arg = arg;
    if ((d->flags & ZOF_ARG) && !(d->flags & (ZOF_OPT | ZOF_SAME))) {
	v->str = NULL;
	if (d->arr)
	    d->arr->num += (arg ? 2 : 1);
    } else if (arg) {
	char *s = (char *) zhalloc(strlen(d->name) + strlen(arg) + 2);

	*s = '-';
	strcpy(s + 1, d->name);
	strcat(s, arg);
	v->str = s;
	if (d->arr)
	    d->arr->num += 1;
    } else {
	v->str = NULL;
	if (d->arr)
	    d->arr->num += 1;
    }
    if (new) {
	if (d->arr) {
	    if (d->arr->last)
		d->arr->last->next = v;
	    else
		d->arr->vals = v;
	    d->arr->last = v;
	}
	if (d->last)
	    d->last->onext = v;
	else
	    d->vals = v;
	d->last = v;
    }
}

/*
 * For "zparseopts -K -A assoc ..." this function copies the keys and
 * values from the default and allocates the extra space for any parsed
 * values having the same keys.  If there are no new values, creates an
 * empty array.  Returns a pointer to the NULL element marking the end.
 *
 *  aval = pointer to the newly allocated array
 *  assoc = name of the default hash param to copy
 *  keep = whether we need to make the copy at all
 *  num = count of new values to add space for
 */
static char **
zalloc_default_array(char ***aval, char *assoc, int keep, int num)
{
    char **ap = 0;

    *aval = 0;
    if (keep && num) {
	struct value vbuf;
	Value v = fetchvalue(&vbuf, &assoc, 0,
			     SCANPM_WANTKEYS|SCANPM_WANTVALS|SCANPM_MATCHMANY);
	if (v && v->isarr) {
	    char **dp, **dval = getarrvalue(v);
	    int dnum = (dval ? arrlen(dval) : 0) + 1;
	    *aval = (char **) zalloc(((num * 2) + dnum) * sizeof(char *));
	    for (ap = *aval, dp = dval; dp && *dp; dp++) {
		*ap = (char *) zalloc(strlen(*dp) + 1);
		strcpy(*ap++, *dp);
	    }
	    *ap = NULL;
	}
    }
    if (!ap) {
	ap = *aval = (char **) zalloc(((num * 2) + 1) * sizeof(char *));
	*ap = NULL;
    }
    return ap;
}

static int
bin_zparseopts(char *nam, char **args, UNUSED(Options ops), UNUSED(int func))
{
    char *o, *p, *n, **pp, **aval, **ap, *assoc = NULL, **cp, **np;
    int del = 0, flags = 0, extract = 0, fail = 0, keep = 0;
    Zoptdesc sopts[256], d;
    Zoptarr a, defarr = NULL;
    Zoptval v;

    opt_descs = NULL;
    opt_arrs = NULL;
    memset(sopts, 0, 256 * sizeof(Zoptdesc));

    while ((o = *args++)) {
	if (*o == '-') {
	    switch (o[1]) {
	    case '\0':
		o = NULL;
		break;
	    case '-':
		if (o[2])
		    args--;
		/* else unreachable, default parsing removes "--" */
		o = NULL;
		break;
	    case 'D':
		if (o[2]) {
		    args--;
		    o = NULL;
		    break;
		}
		del = 1;
		break;
	    case 'E':
		if (o[2]) {
		    args--;
		    o = NULL;
		    break;
		}
		extract = 1;
		break;
	    case 'F':
		if (o[2]) {
		    args--;
		    o = NULL;
		    break;
		}
		fail = 1;
		break;
	    case 'K':
		if (o[2]) {
		    args--;
		    o = NULL;
		    break;
		}
		keep = 1;
		break;
	    case 'M':
		if (o[2]) {
		    args--;
		    o = NULL;
		    break;
		}
		flags |= ZOF_MAP;
		break;
	    case 'a':
		if (defarr) {
		    zwarnnam(nam, "default array given more than once");
		    return 1;
		}
		if (o[2])
		    n = o + 2;
		else if (*args)
		    n = *args++;
		else {
		    zwarnnam(nam, "missing array name");
		    return 1;
		}
		defarr = (Zoptarr) zhalloc(sizeof(*defarr));
		defarr->name = n;
		defarr->num = 0;
		defarr->vals = defarr->last = NULL;
		defarr->next = NULL;
		opt_arrs = defarr;
		break;
	    case 'A':
		if (assoc) {
		    zwarnnam(nam, "associative array given more than once");
		    return 1;
		}
		if (o[2]) 
		    assoc = o + 2;
		else if (*args)
		    assoc = *args++;
		else {
		    zwarnnam(nam, "missing array name");
		    return 1;
		}
		break;
	    default:
		/* Anything else is an option description */
		args--;
		o = NULL;
		break;
	    }
	    if (!o) {
		o = "";
		break;
	    }
	} else {
	    args--;
	    break;
	}
    }
    if (!o) {
	zwarnnam(nam, "missing option descriptions");
	return 1;
    }
    while ((o = dupstring(*args++))) {
	int f = 0;
	if (!*o) {
	    zwarnnam(nam, "invalid option description: %s", o);
	    return 1;
	}
	for (p = o; *p; p++) {
	    if (*p == '\\' && p[1])
		p++;
	    else if (p > o) {	/* At least one character of option name */
		if (*p == '+') {
		    f |= ZOF_MULT;
		    *p = '\0';
		    p++;
		    break;
		} else if (*p == ':' || *p == '=')
		    break;
	    }
	}
	if (*p == ':') {
	    f |= ZOF_ARG;
	    *p = '\0';
	    if (*++p == ':') {
		p++;
		f |= ZOF_OPT;
	    }
	    if (*p == '-') {
		p++;
		f |= ZOF_SAME;
	    }
	}
	a = NULL;
	if (*p == '=') {
	    *p++ = '\0';
	    f |= flags;
	    if (!(a = get_opt_arr(p))) {
		a = (Zoptarr) zhalloc(sizeof(*a));
		a->name = p;
		a->num = 0;
		a->vals = a->last = NULL;
		a->next = opt_arrs;
		opt_arrs = a;
	    }
	} else if (*p) {
	    zwarnnam(nam, "invalid option description: %s", args[-1]);
	    return 1;
	} else if (!(a = defarr) && !assoc) {
	    zwarnnam(nam, "no default array defined: %s", args[-1]);
	    return 1;
	}
	for (p = n = o; *p; p++) {
	    if (*p == '\\' && p[1])
		p++;
	    *n++ = *p;
	}
	*n = '\0';
	if (get_opt_desc(o)) {
	    zwarnnam(nam, "option defined more than once: %s", o);
	    return 1;
	}
	d = (Zoptdesc) zhalloc(sizeof(*d));
	d->name = o;
	d->flags = f;
	d->arr = a;
	d->next = opt_descs;
	d->vals = d->last = NULL;
	opt_descs = d;
	if (!o[1])
	    sopts[STOUC(*o)] = d;
	if ((flags & ZOF_MAP) && !map_opt_desc(d)) {
	    zwarnnam(nam, "cyclic option mapping: %s", args[-1]);
	    return 1;
	}
    }
    np = cp = pp = ((extract && del) ? arrdup(pparams) : pparams);
    for (; (o = *pp); pp++) {
	if (*o != '-') {
	    if (extract) {
		if (del)
		    *cp++ = o;
		continue;
	    } else
		break;
	}
	if (!o[1] || (o[1] == '-' && !o[2])) {
	    if (del && extract)
		*cp++ = o;
	    pp++;
	    break;
	}
	if (!(d = lookup_opt(o + 1))) {
	    while (*++o) {
		if (!(d = sopts[STOUC(*o)])) {
		    if (fail) {
			if (*o != '-')
			    zwarnnam(nam, "bad option: -%c", *o);
			else
			    zwarnnam(nam, "bad option: -%s", o);
			return 1;
		    }
		    o = NULL;
		    break;
		}
		if (d->flags & ZOF_ARG) {
		    if (o[1]) {
			add_opt_val(d, o + 1);
			break;
		    } else if (!(d->flags & ZOF_OPT) ||
			       (pp[1] && pp[1][0] != '-')) {
			if (!pp[1]) {
			    zwarnnam(nam, "missing argument for option: -%s",
				    d->name);
			    return 1;
			}
			add_opt_val(d, *++pp);
		    } else
			add_opt_val(d, NULL);
		} else
		    add_opt_val(d, NULL);
	    }
	    if (!o) {
		if (extract) {
		    if (del)
			*cp++ = *pp;
		    continue;
		} else
		    break;
	    }
	} else {
	    if (d->flags & ZOF_ARG) {
		char *e = o + strlen(d->name) + 1;

		if (*e)
		    add_opt_val(d, e);
		else if (!(d->flags & ZOF_OPT) ||
			 (pp[1] && pp[1][0] != '-')) {
		    if (!pp[1]) {
			zwarnnam(nam, "missing argument for option: -%s",
				d->name);
			return 1;
		    }
		    add_opt_val(d, *++pp);
		} else
		    add_opt_val(d, NULL);
	    } else
		add_opt_val(d, NULL);
	}
    }

    if (flags & ZOF_MAP) {
	for (d = opt_descs; d; d = d->next)
	    if (d->arr && !d->vals && (d->flags & ZOF_MAP)) {
		if (d->arr->num == 0 && get_opt_desc(d->arr->name))
		    d->arr->num = -1;	/* this is not a real array */
	    }
    }
    if (extract && del)
	while (*pp)
	    *cp++ = *pp++;

    for (a = opt_arrs; a; a = a->next) {
	if (a->num >= 0 && (!keep || a->num)) {
	    aval = (char **) zalloc((a->num + 1) * sizeof(char *));
	    for (ap = aval, v = a->vals; v; ap++, v = v->next) {
		if (v->str)
		    *ap = ztrdup(v->str);
		else {
		    *ap = ztrdup(v->name);
		    if (v->arg)
			*++ap = ztrdup(v->arg);
		}
	    }
	    *ap = NULL;
	    setaparam(a->name, aval);
	}
    }
    if (assoc) {
	int num;

	for (num = 0, d = opt_descs; d; d = d->next)
	    if (d->vals)
		num++;

	if (!keep || num) {
	    ap = zalloc_default_array(&aval, assoc, keep, num);
	    for (d = opt_descs; d; d = d->next) {
		if (d->vals) {
		    *ap++ = n = (char *) zalloc(strlen(d->name) + 2);
		    *n = '-';
		    strcpy(n + 1, d->name);

		    for (num = 1, v = d->vals; v; v = v->onext) {
			num += (v->arg ? strlen(v->arg) : 0);
			if (v->next)
			    num++;
		    }
		    *ap++ = n = (char *) zalloc(num);
		    for (v = d->vals; v; v = v->onext) {
			if (v->arg) {
			    strcpy(n, v->arg);
			    n += strlen(v->arg);
			}
			*n = ' ';
		    }
		    *n = '\0';
		}
	    }
	    *ap = NULL;
	    sethparam(assoc, aval);
	}
    }
    if (del) {
	if (extract) {
	    *cp = NULL;
	    freearray(pparams);
	    pparams = zarrdup(np);
	} else {
	    pp = zarrdup(pp);
	    freearray(pparams);
	    pparams = pp;
	}
    }
    return 0;
}

static struct builtin bintab[] = {
    BUILTIN("zformat", 0, bin_zformat, 3, -1, 0, NULL, NULL),
    BUILTIN("zparseopts", 0, bin_zparseopts, 1, -1, 0, NULL, NULL),
    BUILTIN("zregexparse", 0, bin_zregexparse, 3, -1, 0, "c", NULL),
    BUILTIN("zstyle", 0, bin_zstyle, 0, -1, 0, NULL, NULL),
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
    zstyletab = newzstyletable(17, "zstyletab");

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
    return 0;
}

/**/
int
cleanup_(Module m)
{
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    deletehashtable(zstyletab);

    return 0;
}
