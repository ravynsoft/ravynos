/*
 * compctl.c - the compctl builtin
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

#include "compctl.mdh"
#include "compctl.pro"

/* Global matcher. */

/**/
static Cmlist cmatcher;

/* Default completion infos */
 
/**/
struct compctl cc_compos, cc_default, cc_first, cc_dummy;
 
/* Hash table for completion info for commands */
 
/**/
HashTable compctltab;

/* List of pattern compctls */

/**/
Patcomp patcomps;

#define COMP_LIST	(1<<0)	/* -L */
#define COMP_COMMAND	(1<<1)	/* -C */
#define COMP_DEFAULT	(1<<2)	/* -D */
#define COMP_FIRST	(1<<3)	/* -T */
#define COMP_REMOVE	(1<<4)
#define COMP_LISTMATCH	(1<<5)	/* -L and -M */

#define COMP_SPECIAL (COMP_COMMAND|COMP_DEFAULT|COMP_FIRST)

/* Flag for listing, command, default, or first completion */
static int cclist;

/* Mask for determining what to print */
static unsigned long showmask = 0;

/**/
static void
createcompctltable(void)
{
    compctltab = newhashtable(23, "compctltab", NULL);

    compctltab->hash        = hasher;
    compctltab->emptytable  = emptyhashtable;
    compctltab->filltable   = NULL;
    compctltab->cmpnodes    = strcmp;
    compctltab->addnode     = addhashnode;
    compctltab->getnode     = gethashnode2;
    compctltab->getnode2    = gethashnode2;
    compctltab->removenode  = removehashnode;
    compctltab->disablenode = NULL;
    compctltab->enablenode  = NULL;
    compctltab->freenode    = freecompctlp;
    compctltab->printnode   = printcompctlp;

    patcomps = NULL;
}

/**/
static void
freecompctlp(HashNode hn)
{
    Compctlp ccp = (Compctlp) hn;

    zsfree(ccp->node.nam);
    freecompctl(ccp->cc);
    zfree(ccp, sizeof(struct compctlp));
}

/**/
static void
freecompctl(Compctl cc)
{
    if (cc == &cc_default ||
 	cc == &cc_first ||
	cc == &cc_compos ||
	--cc->refc > 0)
	return;

    zsfree(cc->keyvar);
    zsfree(cc->glob);
    zsfree(cc->str);
    zsfree(cc->func);
    zsfree(cc->explain);
    zsfree(cc->ylist);
    zsfree(cc->prefix);
    zsfree(cc->suffix);
    zsfree(cc->hpat);
    zsfree(cc->gname);
    zsfree(cc->subcmd);
    zsfree(cc->substr);
    if (cc->cond)
	freecompcond(cc->cond);
    if (cc->ext) {
	Compctl n, m;

	n = cc->ext;
	do {
	    m = (Compctl) (n->next);
	    freecompctl(n);
	    n = m;
	}
	while (n);
    }
    if (cc->xor && cc->xor != &cc_default)
	freecompctl(cc->xor);
    if (cc->matcher)
	freecmatcher(cc->matcher);
    zsfree(cc->mstr);
    zfree(cc, sizeof(struct compctl));
}

/**/
static void
freecompcond(void *a)
{
    Compcond cc = (Compcond) a;
    Compcond and, or, c;
    int n;

    for (c = cc; c; c = or) {
	or = c->or;
	for (; c; c = and) {
	    and = c->and;
	    if (c->type == CCT_POS ||
		c->type == CCT_NUMWORDS) {
		free(c->u.r.a);
		free(c->u.r.b);
	    } else if (c->type == CCT_CURSUF ||
		       c->type == CCT_CURPRE) {
		for (n = 0; n < c->n; n++)
		    if (c->u.s.s[n])
			zsfree(c->u.s.s[n]);
		free(c->u.s.s);
	    } else if (c->type == CCT_RANGESTR ||
		       c->type == CCT_RANGEPAT) {
		for (n = 0; n < c->n; n++)
		    if (c->u.l.a[n])
			zsfree(c->u.l.a[n]);
		free(c->u.l.a);
		for (n = 0; n < c->n; n++)
		    if (c->u.l.b[n])
			zsfree(c->u.l.b[n]);
		free(c->u.l.b);
	    } else {
		for (n = 0; n < c->n; n++)
		    if (c->u.s.s[n])
			zsfree(c->u.s.s[n]);
		free(c->u.s.p);
		free(c->u.s.s);
	    }
	    zfree(c, sizeof(struct compcond));
	}
    }
}

/**/
static int
compctlread(char *name, char **args, Options ops, char *reply)
{
    char *buf, *bptr;

    /* only allowed to be called for completion */
    if (!incompctlfunc) {
	zwarnnam(name, "option valid only in functions called via compctl");
	return 1;
    }

    METACHECK();

    if (OPT_ISSET(ops,'l')) {
	/*
	 * -ln gives the index of the word the cursor is currently on, which
	 * is available in zlemetacs (but remember that Zsh counts from one,
	 * not zero!)
	 */
	if (OPT_ISSET(ops,'n')) {
	    char nbuf[14];

	    if (OPT_ISSET(ops,'e') || OPT_ISSET(ops,'E'))
		printf("%d\n", zlemetacs + 1);
	    if (!OPT_ISSET(ops,'e')) {
		sprintf(nbuf, "%d", zlemetacs + 1);
		setsparam(reply, ztrdup(nbuf));
	    }
	    return 0;
	}
	/* without -n, the current line is assigned to the given parameter as a
	scalar */
	if (OPT_ISSET(ops,'e') || OPT_ISSET(ops,'E')) {
	    zputs(zlemetaline, stdout);
	    putchar('\n');
	}
	if (!OPT_ISSET(ops,'e'))
	    setsparam(reply, ztrdup(zlemetaline));
    } else {
	int i;

	/* -cn gives the current cursor position within the current word, which
	is available in clwpos (but remember that Zsh counts from one, not
	zero!) */
	if (OPT_ISSET(ops,'n')) {
	    char nbuf[14];

	    if (OPT_ISSET(ops,'e') || OPT_ISSET(ops,'E'))
		printf("%d\n", clwpos + 1);
	    if (!OPT_ISSET(ops,'e')) {
		sprintf(nbuf, "%d", clwpos + 1);
		setsparam(reply, ztrdup(nbuf));
	    }
	    return 0;
	}
	/* without -n, the words of the current line are assigned to the given
	parameters separately */
	if (OPT_ISSET(ops,'A') && !OPT_ISSET(ops,'e')) {
	    /* the -A option means that one array is specified, instead of
	    many parameters */
	    char **p, **b = (char **)zshcalloc((clwnum + 1) * sizeof(char *));

	    for (i = 0, p = b; i < clwnum; p++, i++)
		*p = ztrdup(clwords[i]);

	    setaparam(reply, b);
	    return 0;
	}
	if (OPT_ISSET(ops,'e') || OPT_ISSET(ops,'E')) {
	    for (i = 0; i < clwnum; i++) {
		zputs(clwords[i], stdout);
		putchar('\n');
	    }

	    if (OPT_ISSET(ops,'e'))
		return 0;
	}

	for (i = 0; i < clwnum && *args; reply = *args++, i++)
	    setsparam(reply, ztrdup(clwords[i]));

	if (i < clwnum) {
	    int j, len;

	    for (j = i, len = 0; j < clwnum; len += strlen(clwords[j++]));
	    bptr = buf = zalloc(len + j - i);
	    while (i < clwnum) {
		strucpy(&bptr, clwords[i++]);
		*bptr++ = ' ';
	    }
	    bptr[-1] = '\0';
	} else
	    buf = ztrdup("");
	setsparam(reply, buf);
    }
    return 0;
}

/* Copy a list of completion matchers. */

/**/
static Cmlist
cpcmlist(Cmlist l)
{
    Cmlist r = NULL, *p = &r, n;

    while (l) {
	*p = n = (Cmlist) zalloc(sizeof(struct cmlist));
	n->next = NULL;
	n->matcher = cpcmatcher(l->matcher);
	n->str = ztrdup(l->str);

	p = &(n->next);
	l = l->next;
    }
    return r;
}

/* Set the global match specs. */

/**/
static int
set_gmatcher(char *name, char **argv)
{
    Cmlist l = NULL, *q = &l, n;
    Cmatcher m;

    while (*argv) {
	if ((m = parse_cmatcher(name, *argv)) == pcm_err)
	    return 1;
	*q = n = (Cmlist) zhalloc(sizeof(struct cmlist));
	n->next = NULL;
	n->matcher = m;
	n->str = *argv++;

	q = &(n->next);
    }
    freecmlist(cmatcher);
    cmatcher = cpcmlist(l);

    return 1;
}

/* Try to get the global matcher from the given compctl. */

/**/
static int
get_gmatcher(char *name, char **argv)
{
    if (!strcmp(*argv, "-M")) {
	char **p = ++argv;

	while (*p) {
	    if (**p++ == '-')
		return 0;
	}
	if (set_gmatcher(name, argv))
	    return 2;

	return 1;
    }
    return 0;
}

/* This prints the global matcher definitions. */

/**/
static void
print_gmatcher(int ac)
{
    Cmlist p;

    if ((p = cmatcher)) {
	printf((ac ? "compctl -M" : "MATCH"));

	while (p) {
	    printf(" \'%s\'", p->str);

	    p = p->next;
	}
	putchar('\n');
    }
}

/* Parse the basic flags for `compctl' */

/**/
static int
get_compctl(char *name, char ***av, Compctl cc, int first, int isdef, int cl)
{
    /* Parse the basic flags for completion:
     * first is a flag that we are not in extended completion,
     * while hx indicates or (+) completion (need to know for
     * default and command completion as the initial compctl is special). 
     * cct is a temporary just to hold flags; it never needs freeing.
     */
    struct compctl cct;
    char **argv = *av, argv_end[2] = "x";
    int ready = 0, hx = 0;

    /* Handle `compctl + foo ...' specially:  turn it into
     * a default compctl by removing it from the hash table.
     */
    if (first && argv[0][0] == '+' && !argv[0][1] &&
	!(argv[1] && argv[1][0] == '-' && argv[1][1])) {
	argv++;
	if(argv[0] && argv[0][0] == '-')
	    argv++;
	*av = argv;
	if (cl)
	    return 1;
	else {
	    cclist = COMP_REMOVE;
	    return 0;
	}
    }

    memset((void *)&cct, 0, sizeof(cct));
    cct.mask2 = CC_CCCONT;

    /* Loop through the flags until we have no more:        *
     * those with arguments are not properly allocated yet, *
     * we just hang on to the argument that was passed.     */
    for (; !ready && argv[0] && argv[0][0] == '-' && (argv[0][1] || !first);) {
	if (!argv[0][1])
	    *argv = "-+";
	while (!ready && *++(*argv)) {
	    if(**argv == Meta)
		*++*argv ^= 32;
	    switch (**argv) {
	    case 'f':
		cct.mask |= CC_FILES;
		break;
	    case 'c':
		cct.mask |= CC_COMMPATH;
		break;
	    case 'm':
		cct.mask |= CC_EXTCMDS;
		break;
	    case 'w':
		cct.mask |= CC_RESWDS;
		break;
	    case 'o':
		cct.mask |= CC_OPTIONS;
		break;
	    case 'v':
		cct.mask |= CC_VARS;
		break;
	    case 'b':
		cct.mask |= CC_BINDINGS;
		break;
	    case 'A':
		cct.mask |= CC_ARRAYS;
		break;
	    case 'I':
		cct.mask |= CC_INTVARS;
		break;
	    case 'F':
		cct.mask |= CC_SHFUNCS;
		break;
	    case 'p':
		cct.mask |= CC_PARAMS;
		break;
	    case 'E':
		cct.mask |= CC_ENVVARS;
		break;
	    case 'j':
		cct.mask |= CC_JOBS;
		break;
	    case 'r':
		cct.mask |= CC_RUNNING;
		break;
	    case 'z':
		cct.mask |= CC_STOPPED;
		break;
	    case 'B':
		cct.mask |= CC_BUILTINS;
		break;
	    case 'a':
		cct.mask |= CC_ALREG | CC_ALGLOB;
		break;
	    case 'R':
		cct.mask |= CC_ALREG;
		break;
	    case 'G':
		cct.mask |= CC_ALGLOB;
		break;
	    case 'u':
		cct.mask |= CC_USERS;
		break;
	    case 'd':
		cct.mask |= CC_DISCMDS;
		break;
	    case 'e':
		cct.mask |= CC_EXCMDS;
		break;
	    case 'N':
		cct.mask |= CC_SCALARS;
		break;
	    case 'O':
		cct.mask |= CC_READONLYS;
		break;
	    case 'Z':
		cct.mask |= CC_SPECIALS;
		break;
	    case 'q':
		cct.mask |= CC_REMOVE;
		break;
	    case 'U':
		cct.mask |= CC_DELETE;
		break;
	    case 'n':
		cct.mask |= CC_NAMED;
		break;
	    case 'Q':
		cct.mask |= CC_QUOTEFLAG;
		break;
	    case '/':
		cct.mask |= CC_DIRS;
		break;
	    case 't':
		{
		    char *p;

		    if (cl) {
			zwarnnam(name, "illegal option -%c", **argv);
			return 1;
		    }
		    if ((*argv)[1]) {
			p = (*argv) + 1;
			*argv = argv_end;
		    } else if (!argv[1]) {
			zwarnnam(name, "retry specification expected after -%c",
				 **argv);
			return 1;
		    } else {
			p = *++argv;
			*argv = argv_end;
		    }
		    switch (*p) {
		    case '+':
			cct.mask2 = CC_XORCONT;
			break;
		    case 'n':
			cct.mask2 = 0;
			break;
		    case '-':
			cct.mask2 = CC_PATCONT;
			break;
		    case 'x':
			cct.mask2 = CC_DEFCONT;
			break;
		    default:
			zwarnnam(name, "invalid retry specification character `%c'",
				 *p);
			return 1;
		    }
		    if (p[1]) {
			zwarnnam(name, "too many retry specification characters: `%s'",
				 p + 1);
			return 1;
		    }
		}
		break;
	    case 'k':
		if ((*argv)[1]) {
		    cct.keyvar = (*argv) + 1;
		    *argv = argv_end;
		} else if (!argv[1]) {
		    zwarnnam(name, "variable name expected after -%c", **argv);
		    return 1;
		} else {
		    cct.keyvar = *++argv;
		    *argv = argv_end;
		}
		break;
	    case 'K':
		if ((*argv)[1]) {
		    cct.func = (*argv) + 1;
		    *argv = argv_end;
		} else if (!argv[1]) {
		    zwarnnam(name, "function name expected after -%c", **argv);
		    return 1;
		} else {
		    cct.func = *++argv;
		    *argv = argv_end;
		}
		break;
	    case 'Y':
		cct.mask |= CC_EXPANDEXPL;
		goto expl;
	    case 'X':
		cct.mask &= ~CC_EXPANDEXPL;
	    expl:
		if ((*argv)[1]) {
		    cct.explain = (*argv) + 1;
		    *argv = argv_end;
		} else if (!argv[1]) {
		    zwarnnam(name, "string expected after -%c", **argv);
		    return 1;
		} else {
		    cct.explain = *++argv;
		    *argv = argv_end;
		}
		break;
	    case 'y':
		if ((*argv)[1]) {
		    cct.ylist = (*argv) + 1;
		    *argv = argv_end;
		} else if (!argv[1]) {
		    zwarnnam(name, "function/variable expected after -%c",
			     **argv);
		} else {
		    cct.ylist = *++argv;
		    *argv = argv_end;
		}
		break;
	    case 'P':
		if ((*argv)[1]) {
		    cct.prefix = (*argv) + 1;
		    *argv = argv_end;
		} else if (!argv[1]) {
		    zwarnnam(name, "string expected after -%c", **argv);
		    return 1;
		} else {
		    cct.prefix = *++argv;
		    *argv = argv_end;
		}
		break;
	    case 'S':
		if ((*argv)[1]) {
		    cct.suffix = (*argv) + 1;
		    *argv = argv_end;
		} else if (!argv[1]) {
		    zwarnnam(name, "string expected after -%c", **argv);
		    return 1;
		} else {
		    cct.suffix = *++argv;
		    *argv = argv_end;
		}
		break;
	    case 'g':
		if ((*argv)[1]) {
		    cct.glob = (*argv) + 1;
		    *argv = argv_end;
		} else if (!argv[1]) {
		    zwarnnam(name, "glob pattern expected after -%c", **argv);
		    return 1;
		} else {
		    cct.glob = *++argv;
		    *argv = argv_end;
		}
		break;
	    case 's':
		if ((*argv)[1]) {
		    cct.str = (*argv) + 1;
		    *argv = argv_end;
		} else if (!argv[1]) {
		    zwarnnam(name, "command string expected after -%c",
			     **argv);
		    return 1;
		} else {
		    cct.str = *++argv;
		    *argv = argv_end;
		}
		break;
	    case 'l':
		if (cl) {
		    zwarnnam(name, "illegal option -%c", **argv);
		    return 1;
		} else if ((*argv)[1]) {
		    cct.subcmd = (*argv) + 1;
		    *argv = argv_end;
		} else if (!argv[1]) {
		    zwarnnam(name, "command name expected after -%c", **argv);
		    return 1;
		} else {
		    cct.subcmd = *++argv;
		    *argv = argv_end;
		}
		break;
	    case 'h':
		if (cl) {
		    zwarnnam(name, "illegal option -%c", **argv);
		    return 1;
		} else if ((*argv)[1]) {
		    cct.substr = (*argv) + 1;
		    *argv = argv_end;
		} else if (!argv[1]) {
		    zwarnnam(name, "command name expected after -%c", **argv);
		    return 1;
		} else {
		    cct.substr = *++argv;
		    *argv = argv_end;
		}
		break;
	    case 'W':
		if ((*argv)[1]) {
		    cct.withd = (*argv) + 1;
		    *argv = argv_end;
		} else if (!argv[1]) {
		    zwarnnam(name, "path expected after -%c", **argv);
		    return 1;
		} else {
		    cct.withd = *++argv;
		    *argv = argv_end;
		}
		break;
	    case 'J':
		if ((*argv)[1]) {
		    cct.gname = (*argv) + 1;
		    *argv = argv_end;
		} else if (!argv[1]) {
		    zwarnnam(name, "group name expected after -%c", **argv);
		    return 1;
		} else {
		    cct.gname = *++argv;
		    *argv = argv_end;
		}
		break;
	    case 'V':
		if ((*argv)[1]) {
		    cct.gname = (*argv) + 1;
		    *argv = argv_end;
		} else if (!argv[1]) {
		    zwarnnam(name, "group name expected after -%c", **argv);
		    return 1;
		} else {
		    cct.gname = *++argv;
		    *argv = argv_end;
		}
		cct.mask2 |= CC_NOSORT;
		break;
	    case '1':
		cct.mask2 |= CC_UNIQALL;
		cct.mask2 &= ~CC_UNIQCON;
		break;
	    case '2':
		cct.mask2 |= CC_UNIQCON;
		cct.mask2 &= ~CC_UNIQALL;
		break;
	    case 'M':
		if (cclist & COMP_LIST) {
		    cclist |= COMP_LISTMATCH;
		} else if ((*argv)[1]) {
		    if ((cct.matcher =
			 parse_cmatcher(name, (cct.mstr = (*argv) + 1))) ==
			pcm_err) {
			cct.matcher = NULL;
			cct.mstr = NULL;
			return 1;
		    }
		    *argv = argv_end;
		} else if (!argv[1]) {
		    zwarnnam(name, "matching specification expected after -%c",
			     **argv);
		    return 1;
		} else {
		    if ((cct.matcher =
			 parse_cmatcher(name, (cct.mstr = *++argv))) ==
			pcm_err) {
			cct.matcher = NULL;
			cct.mstr = NULL;
			return 1;
		    }
		    *argv = argv_end;
		}
		break;
	    case 'H':
		if ((*argv)[1])
		    cct.hnum = atoi((*argv) + 1);
		else if (argv[1])
		    cct.hnum = atoi(*++argv);
		else {
		    zwarnnam(name, "number expected after -%c", **argv);
		    return 1;
		}
		if (!argv[1]) {
		    zwarnnam(name, "missing pattern after -%c", **argv);
		    return 1;
		}
		cct.hpat = *++argv;
		if (cct.hnum < 1)
		    cct.hnum = 0;
		if (*cct.hpat == '*' && !cct.hpat[1])
		    cct.hpat = "";
		*argv = argv_end;
		break;
	    case 'C':
		if (cl) {
		    zwarnnam(name, "illegal option -%c", **argv);
		    return 1;
		}
		if (first && !hx) {
		    cclist |= COMP_COMMAND;
		} else {
		    zwarnnam(name, "misplaced command completion (-C) flag");
		    return 1;
		}
		break;
	    case 'D':
		if (cl) {
		    zwarnnam(name, "illegal option -%c", **argv);
		    return 1;
		}
		if (first && !hx) {
		    isdef = 1;
		    cclist |= COMP_DEFAULT;
		} else {
		    zwarnnam(name, "misplaced default completion (-D) flag");
		    return 1;
		}
		break;
 	    case 'T':
		if (cl) {
		    zwarnnam(name, "illegal option -%c", **argv);
		    return 1;
		}
		if (first && !hx) {
 		    cclist |= COMP_FIRST;
 		} else {
 		    zwarnnam(name, "misplaced first completion (-T) flag");
 		    return 1;
 		}
 		break;
	    case 'L':
		if (cl) {
		    zwarnnam(name, "illegal option -%c", **argv);
		    return 1;
		}
		if (!first || hx) {
		    zwarnnam(name, "illegal use of -L flag");
		    return 1;
		}
		cclist |= COMP_LIST;
		break;
	    case 'x':
		if (cl) {
		    zwarnnam(name, "extended completion not allowed");
		    return 1;
		}
		if (!argv[1]) {
		    zwarnnam(name, "condition expected after -%c", **argv);
		    return 1;
		}
		if (first) {
		    argv++;
		    if (get_xcompctl(name, &argv, &cct, isdef)) {
			if (cct.ext)
			    freecompctl(cct.ext);
			return 1;
		    }
		    ready = 2;
		} else {
		    zwarnnam(name, "recursive extended completion not allowed");
		    return 1;
		}
		break;
	    default:
		if (!first && (**argv == '-' || **argv == '+') && !argv[0][1])
		    (*argv)--, argv--, ready = 1;
		else {
		    zwarnnam(name, "bad option: -%c", **argv);
		    return 1;
		}
	    }
	}

	if (*++argv && (!ready || ready == 2) &&
	    **argv == '+' && !argv[0][1]) {
	    if (cl) {
		zwarnnam(name, "xor'ed completion illegal");
		return 1;
	    }
	    /* There's an alternative (+) completion:  assign
	     * what we have so far before moving on to that.
	     */
	    if (cc_assign(name, &cc, &cct, first && !hx))
		return 1;

	    hx = 1;
	    ready = 0;

	    if (!*++argv || **argv != '-' ||
		(**argv == '-' && (!argv[0][1] ||
				   (argv[0][1] == '-' && !argv[0][2])))) {
		/* No argument to +, which means do default completion */
		if (isdef)
		    zwarnnam(name,
			    "recursive xor'd default completions not allowed");
		else
		    cc->xor = &cc_default;
	    } else {
		/* more flags follow:  prepare to loop again */
		cc->xor = (Compctl) zshcalloc(sizeof(*cc));
		cc = cc->xor;
		memset((void *)&cct, 0, sizeof(cct));
		cct.mask2 = CC_CCCONT;
	    }
	}
    }
    if (!ready && *argv && **argv == '-')
	argv++;

    if (! (cct.mask & (CC_EXCMDS | CC_DISCMDS)))
	cct.mask |= CC_EXCMDS;

    /* assign the last set of flags we parsed */
    if (cc_assign(name, &cc, &cct, first && !hx))
	return 1;

    *av = argv;

    return 0;
}

/* Handle the -x ... -- part of compctl. */

/**/
static int
get_xcompctl(char *name, char ***av, Compctl cc, int isdef)
{
    char **argv = *av, *t, *tt, sav;
    int n, l = 0, ready = 0;
    Compcond m, c, o;
    Compctl *next = &(cc->ext);

    while (!ready) {
	/* o keeps track of or's, m remembers the starting condition,
	 * c is the current condition being parsed
	 */
	o = m = c = (Compcond) zshcalloc(sizeof(*c));
	/* Loop over each condition:  something like 's[...][...], p[...]' */
	for (t = *argv; *t;) {
	    while (*t == ' ')
		t++;
	    /* First get the condition code */
	    switch (*t) {
	    case 'q':
		c->type = CCT_QUOTE;
		break;
	    case 's':
		c->type = CCT_CURSUF;
		break;
	    case 'S':
		c->type = CCT_CURPRE;
		break;
	    case 'p':
		c->type = CCT_POS;
		break;
	    case 'c':
		c->type = CCT_CURSTR;
		break;
	    case 'C':
		c->type = CCT_CURPAT;
		break;
	    case 'w':
		c->type = CCT_WORDSTR;
		break;
	    case 'W':
		c->type = CCT_WORDPAT;
		break;
	    case 'n':
		c->type = CCT_CURSUB;
		break;
	    case 'N':
		c->type = CCT_CURSUBC;
		break;
	    case 'm':
		c->type = CCT_NUMWORDS;
		break;
	    case 'r':
		c->type = CCT_RANGESTR;
		break;
	    case 'R':
		c->type = CCT_RANGEPAT;
		break;
	    default:
		t[1] = '\0';
		zwarnnam(name, "unknown condition code: %s", t);
		zfree(m, sizeof(struct compcond));

		return 1;
	    }
	    /* Now get the arguments in square brackets */
	    if (t[1] != '[') {
		t[1] = '\0';
		zwarnnam(name, "expected condition after condition code: %s", t);
		zfree(m, sizeof(struct compcond));

		return 1;
	    }
	    t++;
	    /* First count how many or'd arguments there are,
	     * marking the active ]'s and ,'s with unprintable characters.
	     */
	    for (n = 0, tt = t; *tt == '['; n++) {
		for (l = 1, tt++; *tt && l; tt++)
		    if (*tt == '\\' && tt[1])
			tt++;
		    else if (*tt == '[')
			l++;
		    else if (*tt == ']')
			l--;
		    else if (l == 1 && *tt == ',')
			*tt = '\201';
		if (tt[-1] == ']')
		    tt[-1] = '\200';
	    }

	    if (l) {
		t[1] = '\0';
		zwarnnam(name, "error after condition code: %s", t);
		zfree(m, sizeof(struct compcond));

		return 1;
	    }
	    c->n = n;

	    /* Allocate space for all the arguments of the conditions */
	    if (c->type == CCT_POS ||
		c->type == CCT_NUMWORDS) {
		c->u.r.a = (int *)zshcalloc(n * sizeof(int));
		c->u.r.b = (int *)zshcalloc(n * sizeof(int));
	    } else if (c->type == CCT_CURSUF ||
		       c->type == CCT_CURPRE ||
		       c->type == CCT_QUOTE)
		c->u.s.s = (char **)zshcalloc(n * sizeof(char *));

	    else if (c->type == CCT_RANGESTR ||
		     c->type == CCT_RANGEPAT) {
		c->u.l.a = (char **)zshcalloc(n * sizeof(char *));
		c->u.l.b = (char **)zshcalloc(n * sizeof(char *));
	    } else {
		c->u.s.p = (int *)zshcalloc(n * sizeof(int));
		c->u.s.s = (char **)zshcalloc(n * sizeof(char *));
	    }
	    /* Now loop over the actual arguments */
	    for (l = 0; *t == '['; l++, t++) {
		for (t++; *t && *t == ' '; t++);
		tt = t;
		if (c->type == CCT_POS ||
		    c->type == CCT_NUMWORDS) {
		    /* p[...] or m[...]:  one or two numbers expected */
		    for (; *t && *t != '\201' && *t != '\200'; t++);
		    if (!(sav = *t)) {
			zwarnnam(name, "error in condition");
			freecompcond(m);
			return 1;
		    }
		    *t = '\0';
		    c->u.r.a[l] = atoi(tt);
		    /* Second argument is optional:  see if it's there */
		    if (sav == '\200')
			/* no:  copy first argument */
			c->u.r.b[l] = c->u.r.a[l];
		    else {
			tt = ++t;
			for (; *t && *t != '\200'; t++);
			if (!*t) {
			    zwarnnam(name, "error in condition");
			    freecompcond(m);
			    return 1;
			}
			*t = '\0';
			c->u.r.b[l] = atoi(tt);
		    }
		} else if (c->type == CCT_CURSUF ||
			   c->type == CCT_CURPRE ||
			   c->type == CCT_QUOTE) {
		    /* -s[..] or -S[..]:  single string expected */
		    for (; *t && *t != '\200'; t++)
			if (*t == '\201')
			    *t = ',';
		    if (!*t) {
			zwarnnam(name, "error in condition");
			freecompcond(m);
			return 1;
		    }
		    *t = '\0';
		    c->u.s.s[l] = ztrdup(tt);
		} else if (c->type == CCT_RANGESTR ||
			   c->type == CCT_RANGEPAT) {
		    int hc;

		    /* -r[..,..] or -R[..,..]:  two strings expected */
		    for (; *t && *t != '\201' && *t != '\200'; t++);
		    if (!*t) {
			zwarnnam(name, "error in condition");
			freecompcond(m);
			return 1;
		    }
		    hc = (*t == '\201');
		    *t = '\0';
		    c->u.l.a[l] = ztrdup(tt);
		    if (hc) {
			tt = ++t;
			/* any more commas are text, not active */
			for (; *t && *t != '\200'; t++)
			    if (*t == '\201')
				*t = ',';
			if (!*t) {
			    zwarnnam(name, "error in condition");
			    freecompcond(m);
			    return 1;
			}
			*t = '\0';
			c->u.l.b[l] = ztrdup(tt);
		    }
		    else
			c->u.l.b[l] = NULL;
		} else {
		    /* remaining patterns are number followed by string */
		    for (; *t && *t != '\200' && *t != '\201'; t++);
		    if (!*t || *t == '\200') {
			zwarnnam(name, "error in condition");
			freecompcond(m);
			return 1;
		    }
		    *t = '\0';
		    c->u.s.p[l] = atoi(tt);
		    tt = ++t;
		    for (; *t && *t != '\200'; t++)
			if (*t == '\201')
			    *t = ',';
		    if (!*t) {
			zwarnnam(name, "error in condition");
			freecompcond(m);
			return 1;
		    }
		    *t = '\0';
		    c->u.s.s[l] = ztrdup(tt);
		}
	    }
	    while (*t == ' ')
		t++;
	    if (*t == ',') {
		/* Another condition to `or' */
		o->or = c = (Compcond) zshcalloc(sizeof(*c));
		o = c;
		t++;
	    } else if (*t) {
		/* Another condition to `and' */
		c->and = (Compcond) zshcalloc(sizeof(*c));
		c = c->and;
	    }
	}
	/* Assign condition to current compctl */
	*next = (Compctl) zshcalloc(sizeof(*cc));
	(*next)->cond = m;
	argv++;
	/* End of the condition; get the flags that go with it. */
	if (get_compctl(name, &argv, *next, 0, isdef, 0))
	    return 1;
 	if ((!argv || !*argv) && (cclist & COMP_SPECIAL))
 	    /* default, first, or command completion finished */
	    ready = 1;
	else {
	    /* see if we are looking for more conditions or are
	     * ready to return (ready = 1)
	     */
	    if (!argv || !*argv || **argv != '-' ||
		((!argv[0][1] || argv[0][1] == '+') && !argv[1])) {
		zwarnnam(name, "missing command names");
		return 1;
	    }
	    if (!strcmp(*argv, "--"))
		ready = 1;
	    else if (!strcmp(*argv, "-+") && argv[1] &&
		     !strcmp(argv[1], "--")) {
		ready = 1;
		argv++;
	    }
	    argv++;
	    /* prepare to put the next lot of conditions on the end */
	    next = &((*next)->next);
	}
    }
    /* save position at end of parsing */
    *av = argv - 1;
    return 0;
}

/**/
static int
cc_assign(char *name, Compctl *ccptr, Compctl cct, int reass)
{
    /* Copy over the details from the values in cct to those in *ccptr */
    Compctl cc;

    /* Handle assignment of new default or command completion */
    if (reass && !(cclist & COMP_LIST)) {
	/* if not listing */
	if (cclist == (COMP_COMMAND|COMP_DEFAULT)
	    || cclist == (COMP_COMMAND|COMP_FIRST)
	    || cclist == (COMP_DEFAULT|COMP_FIRST)
	    || cclist == COMP_SPECIAL) {
 	    zwarnnam(name, "can't set -D, -T, and -C simultaneously");
	    /* ... because the following code wouldn't work. */
	    return 1;
	}
	if (cclist & COMP_COMMAND) {
	    /* command */
	    *ccptr = &cc_compos;
	    cc_reassign(*ccptr);
	} else if (cclist & COMP_DEFAULT) {
	    /* default */
	    *ccptr = &cc_default;
	    cc_reassign(*ccptr);
 	} else if (cclist & COMP_FIRST) {
 	    /* first */
 	    *ccptr = &cc_first;
 	    cc_reassign(*ccptr);
	}
    }

    /* Free the old compctl */
    cc = *ccptr;
    zsfree(cc->keyvar);
    zsfree(cc->glob);
    zsfree(cc->str);
    zsfree(cc->func);
    zsfree(cc->explain);
    zsfree(cc->ylist);
    zsfree(cc->prefix);
    zsfree(cc->suffix);
    zsfree(cc->subcmd);
    zsfree(cc->substr);
    zsfree(cc->withd);
    zsfree(cc->hpat);
    zsfree(cc->gname);
    zsfree(cc->mstr);
    freecmatcher(cc->matcher);

    /* and copy over the new stuff, (permanently) allocating
     * space for strings.
     */
    cc->mask = cct->mask;
    cc->mask2 = cct->mask2;
    cc->keyvar = ztrdup(cct->keyvar);
    cc->glob = ztrdup(cct->glob);
    cc->str = ztrdup(cct->str);
    cc->func = ztrdup(cct->func);
    cc->explain = ztrdup(cct->explain);
    cc->ylist = ztrdup(cct->ylist);
    cc->prefix = ztrdup(cct->prefix);
    cc->suffix = ztrdup(cct->suffix);
    cc->subcmd = ztrdup(cct->subcmd);
    cc->substr = ztrdup(cct->substr);
    cc->withd = ztrdup(cct->withd);
    cc->gname = ztrdup(cct->gname);
    cc->hpat = ztrdup(cct->hpat);
    cc->hnum = cct->hnum;
    cc->matcher = cpcmatcher(cct->matcher);
    cc->mstr = ztrdup(cct->mstr);

    /* careful with extended completion:  it's already allocated */
    cc->ext = cct->ext;

    return 0;
}

/**/
static void
cc_reassign(Compctl cc)
{
    /* Free up a new default or command completion:
     * this is a hack to free up the parts which should be deleted,
     * without removing the basic variable which is statically allocated
     */
    Compctl c2;

    c2 = (Compctl) zshcalloc(sizeof *cc);
    c2->xor = cc->xor;
    c2->ext = cc->ext;
    c2->refc = 1;

    freecompctl(c2);

    cc->ext = cc->xor = NULL;
}

/* Check if the given string is a pattern. If so, return one and tokenize *
 * it. If not, we just remove the backslashes. */

static int
compctl_name_pat(char **p)
{
    char *s = *p;

    tokenize(s = dupstring(s));
    remnulargs(s);

    if (haswilds(s)) {
	*p = s;
	return 1;
    } else
	*p = rembslash(*p);

    return 0;
}

/* Delete the pattern compctl with the given name. */

static void
delpatcomp(char *n)
{
    Patcomp p, q;

    for (q = 0, p = patcomps; p; q = p, p = p->next) {
	if (!strcmp(n, p->pat)) {
	    if (q)
		q->next = p->next;
	    else
		patcomps = p->next;
	    zsfree(p->pat);
	    freecompctl(p->cc);
	    free(p);

	    break;
	}
    }
}

/**/
static void
compctl_process_cc(char **s, Compctl cc)
{
    Compctlp ccp;
    char *n;

    if (cclist & COMP_REMOVE) {
	/* Delete entries for the commands listed */
	for (; *s; s++) {
	    n = *s;
	    if (compctl_name_pat(&n))
		delpatcomp(n);
	    else if ((ccp = (Compctlp) compctltab->removenode(compctltab, n)))
		compctltab->freenode(&ccp->node);
	}
    } else {
	/* Add the compctl just read to the hash table */

	cc->refc = 0;
	for (; *s; s++) {
	    n = *s;

	    cc->refc++;
	    if (compctl_name_pat(&n)) {
		Patcomp pc;

		delpatcomp(n);
		pc = zalloc(sizeof *pc);
		pc->pat = ztrdup(n);
		pc->cc = cc;
		pc->next = patcomps;
		patcomps = pc;
	    } else {
		ccp = (Compctlp) zalloc(sizeof *ccp);
		ccp->cc = cc;
		compctltab->addnode(compctltab, ztrdup(n), ccp);
	    }
	}
    }
}

/* Print a `compctl' */

/**/
static void
printcompctl(char *s, Compctl cc, int printflags, int ispat)
{
    Compctl cc2;
    char *css = "fcqovbAIFpEjrzBRGudeNOZUnQmw/";
    char *mss = " pcCwWsSnNmrRq";
    unsigned long t = 0x7fffffff;
    unsigned long flags = cc->mask, flags2 = cc->mask2;
    unsigned long oldshowmask;

    /* Printflags is used outside the standard compctl commands*/
    if (printflags & PRINT_LIST)
	cclist |= COMP_LIST;
    else if (printflags & PRINT_TYPE)
	cclist &= ~COMP_LIST;

    if ((flags & CC_EXCMDS) && !(flags & CC_DISCMDS))
	flags &= ~CC_EXCMDS;

    /* If showmask is non-zero, then print only those *
     * commands with that flag set.                   */
    if (showmask && !(flags & showmask))
	return;

    /* Temporarily clear showmask in case we make *
     * recursive calls to printcompctl.           */
    oldshowmask = showmask;
    showmask = 0;

    /* print either command name or start of compctl command itself */
    if (s) {
	if (cclist & COMP_LIST) {
	    printf("compctl");
	    if (cc == &cc_compos)
		printf(" -C");
	    if (cc == &cc_default)
		printf(" -D");
	    if (cc == &cc_first)
		printf(" -T");
	} else if (ispat) {
	    char *p = dupstring(s);

	    untokenize(p);
	    quotedzputs(p, stdout);
	} else
	    quotedzputs(quotestring(s, QT_BACKSLASH), stdout);
    }

    /* loop through flags w/o args that are set, printing them if so */
    if ((flags & t) || (flags2 & (CC_UNIQALL | CC_UNIQCON))) {
	printf(" -");
	if ((flags & (CC_ALREG | CC_ALGLOB)) == (CC_ALREG | CC_ALGLOB))
	    putchar('a'), flags &= ~(CC_ALREG | CC_ALGLOB);
	while (*css) {
	    if (flags & t & 1)
		putchar(*css);
	    css++;
	    flags >>= 1;
	    t >>= 1;
	}
	if (flags2 & CC_UNIQALL)
	    putchar('1');
	else if (flags2 & CC_UNIQCON)
	    putchar('2');
    }
    if (flags2 & (CC_XORCONT | CC_PATCONT | CC_DEFCONT)) {
	printf(" -t");
	if (flags2 & CC_XORCONT)
	    putchar('+');
	if (flags2 & CC_PATCONT)
	    putchar('-');
	if (flags2 & CC_DEFCONT)
	    putchar('x');
    } else if (!(flags2 & CC_CCCONT))
	printf(" -tn");
    /* now flags with arguments */
    printif(cc->mstr, 'M');
    if (flags2 & CC_NOSORT)
	printif(cc->gname, 'V');
    else
	printif(cc->gname, 'J');
    printif(cc->keyvar, 'k');
    printif(cc->func, 'K');
    printif(cc->explain, (cc->mask & CC_EXPANDEXPL) ? 'Y' : 'X');
    printif(cc->ylist, 'y');
    printif(cc->prefix, 'P');
    printif(cc->suffix, 'S');
    printif(cc->glob, 'g');
    printif(cc->str, 's');
    printif(cc->subcmd, 'l');
    printif(cc->substr, 'h');
    printif(cc->withd, 'W');
    if (cc->hpat) {
	printf(" -H %d ", cc->hnum);
	quotedzputs(cc->hpat, stdout);
    }

    /* now the -x ... -- extended completion part */
    if (cc->ext) {
	Compcond c, o;
	int i;

	cc2 = cc->ext;
	printf(" -x");

	while (cc2) {
	    /* loop over conditions */
	    c = cc2->cond;

	    printf(" '");
	    for (c = cc2->cond; c;) {
		/* loop over or's */
		o = c->or;
		while (c) {
		    /* loop over and's */
		    putchar(mss[c->type]);

		    for (i = 0; i < c->n; i++) {
			/* for all [...]'s of a given condition */
			putchar('[');
			switch (c->type) {
			case CCT_POS:
			case CCT_NUMWORDS:
			    printf("%d,%d", c->u.r.a[i], c->u.r.b[i]);
			    break;
			case CCT_CURSUF:
			case CCT_CURPRE:
			case CCT_QUOTE:
			    printqt(c->u.s.s[i]);
			    break;
			case CCT_RANGESTR:
			case CCT_RANGEPAT:
			    printqt(c->u.l.a[i]);
			    putchar(',');
			    printqt(c->u.l.b[i]);
			    break;
			default:
			    printf("%d,", c->u.s.p[i]);
			    printqt(c->u.s.s[i]);
			}
			putchar(']');
		    }
		    if ((c = c->and))
			putchar(' ');
		}
		if ((c = o))
		    printf(" , ");
	    }
	    putchar('\'');
	    c = cc2->cond;
	    cc2->cond = NULL;
	    /* now print the flags for the current condition */
	    printcompctl(NULL, cc2, 0, 0);
	    cc2->cond = c;
	    if ((cc2 = (Compctl) (cc2->next)))
		printf(" -");
	}
	if (cclist & COMP_LIST)
	    printf(" --");
    }
    if (cc->xor) {
	/* print xor'd (+) completions */
	printf(" +");
	if (cc->xor != &cc_default)
	    printcompctl(NULL, cc->xor, 0, 0);
    }
    if (s) {
	if ((cclist & COMP_LIST) && (cc != &cc_compos)
	    && (cc != &cc_default) && (cc != &cc_first)) {
	    if(s[0] == '-' || s[0] == '+')
		printf(" -");
	    putchar(' ');
	    if (ispat) {
		char *p = dupstring(s);

		untokenize(p);
		quotedzputs(p, stdout);
	    } else {
		char *p = dupstring(s);

		untokenize(p);
		quotedzputs(quotestring(p, QT_BACKSLASH), stdout);
	    }
	}
	putchar('\n');
    }

    showmask = oldshowmask;
}

/**/
static void
printcompctlp(HashNode hn, int printflags)
{
    Compctlp ccp = (Compctlp) hn;

    /* Function needed for use by scanhashtable() */
    printcompctl(ccp->node.nam, ccp->cc, printflags, 0);
}

/* Main entry point for the `compctl' builtin */

/**/
static int
bin_compctl(char *name, char **argv, UNUSED(Options ops), UNUSED(int func))
{
    Compctl cc = NULL;
    int ret = 0;

    queue_signals();

    /* clear static flags */
    cclist = 0;
    showmask = 0;

    /* Parse all the arguments */
    if (*argv) {
	/* Let's see if this is a global matcher definition. */
	if ((ret = get_gmatcher(name, argv))) {
	    unqueue_signals();
	    return ret - 1;
	}

	cc = (Compctl) zshcalloc(sizeof(*cc));
	if (get_compctl(name, &argv, cc, 1, 0, 0)) {
	    freecompctl(cc);
	    unqueue_signals();
	    return 1;
	}

	/* remember flags for printing */
	showmask = cc->mask;
	if ((showmask & CC_EXCMDS) && !(showmask & CC_DISCMDS))
	    showmask &= ~CC_EXCMDS;

	/* if no command arguments or just listing, we don't want cc */
	if (!*argv || (cclist & COMP_LIST))
	    freecompctl(cc);
    }

    /* If no commands and no -C, -T, or -D, print all the compctl's *
     * If some flags (other than -C, -T, or -D) were given, then    *
     * only print compctl containing those flags.                   */
    if (!*argv && !(cclist & (COMP_SPECIAL|COMP_LISTMATCH))) {
	Patcomp pc;

	for (pc = patcomps; pc; pc = pc->next)
	    printcompctl(pc->pat, pc->cc, 0, 1);

	scanhashtable(compctltab, 1, 0, 0, compctltab->printnode, 0);
	printcompctl((cclist & COMP_LIST) ? "" : "COMMAND", &cc_compos, 0, 0);
	printcompctl((cclist & COMP_LIST) ? "" : "DEFAULT", &cc_default, 0, 0);
 	printcompctl((cclist & COMP_LIST) ? "" : "FIRST", &cc_first, 0, 0);
	print_gmatcher((cclist & COMP_LIST));
	unqueue_signals();
	return ret;
    }

    /* If we're listing and we've made it to here, then there are arguments *
     * or a COMP_SPECIAL flag (-D, -C, -T), so print only those.            */
    if (cclist & COMP_LIST) {
	HashNode hn;
	char **ptr, *n;

	showmask = 0;
	for (ptr = argv; *ptr; ptr++) {
	    n = *ptr;
	    if (compctl_name_pat(&n)) {
		Patcomp pc;

		for (pc = patcomps; pc; pc = pc->next)
		    if (!strcmp(n, pc->pat)) {
			printcompctl(pc->pat, pc->cc, 0, 1);
			n = NULL;
			break;
		    }
	    } else if ((hn = compctltab->getnode(compctltab, n))) {
		compctltab->printnode(hn, 0);
		n = NULL;
	    }
	    if (n) {
		zwarnnam(name, "no compctl defined for %s", n);
		ret = 1;
	    }
	}
	if (cclist & COMP_COMMAND)
	    printcompctl("", &cc_compos, 0, 0);
	if (cclist & COMP_DEFAULT)
	    printcompctl("", &cc_default, 0, 0);
	if (cclist & COMP_FIRST)
	    printcompctl("", &cc_first, 0, 0);
	if (cclist & COMP_LISTMATCH)
	    print_gmatcher(COMP_LIST);
	unqueue_signals();
	return ret;
    }

    /* Assign the compctl to the commands given */
    if (*argv) {
	if(cclist & COMP_SPECIAL)
	    /* Ideally we'd handle this properly, setting both the *
	     * special and normal completions.  For the moment,    *
	     * this is better than silently failing.               */
	    zwarnnam(name, "extraneous commands ignored");
	else
	    compctl_process_cc(argv, cc);
    }

    unqueue_signals();
    return ret;
}

/* Flags for makecomplist*(). Things not to do. */

#define CFN_FIRST   1
#define CFN_DEFAULT 2

static int
bin_compcall(char *name, UNUSED(char **argv), Options ops, UNUSED(int func))
{
    int ret;

    if (incompfunc != 1) {
	zwarnnam(name, "can only be called from completion function");
	return 1;
    }

    queue_signals();
    ret = makecomplistctl((OPT_ISSET(ops,'T') ? 0 : CFN_FIRST) |
			  (OPT_ISSET(ops,'D') ? 0 : CFN_DEFAULT));
    unqueue_signals();
    return ret;
}

/*
 * Functions to generate matches.
 */

/* A pointer to the compctl we are using. */

static Compctl curcc;

/* A list of all compctls we have already used. */

static LinkList ccused, lastccused;

/* A stack of currently used compctls. */

static LinkList ccstack;

/* The beginning and end of a word range to be used by -l. */

static int brange, erange;

/* This is used to detect when and what to continue. */

static unsigned long ccont;

/* Two patterns used when doing glob-completion.  The first one is built *
 * from the whole word we are completing and the second one from that    *
 * part of the word that was identified as a possible filename.          */

static Patprog patcomp, filecomp;

/* We store the following prefixes/suffixes:                               *
 * lpre/lsuf -- what's on the line                                         *
 * rpre/rsuf -- same as lpre/lsuf, but expanded                            *
 * ppre/psuf   -- the path prefix/suffix                                   *
 * lppre/lpsuf -- the path prefix/suffix, unexpanded                       *
 * fpre/fsuf   -- prefix/suffix of the pathname component the cursor is in *
 * prpre       -- ppre in expanded form usable for opendir                 *
 * qipre, qisuf-- ignored quoted string                                   *
 *                                                                         *
 * The integer variables hold the lengths of lpre, lsuf, rpre, rsuf,       *
 * fpre, fsuf, lppre, and lpsuf.  noreal is non-zero if we have rpre/rsuf. */

static char *lpre, *lsuf;
static char *rpre, *rsuf;
static char *ppre, *psuf, *lppre, *lpsuf, *prpre;
static char *fpre, *fsuf;
static char *qfpre, *qfsuf, *qrpre, *qrsuf, *qlpre, *qlsuf;
static int lpl, lsl, rpl, rsl, fpl, fsl, lppl, lpsl;
static int noreal;

/* This is either zero or equal to the special character the word we are *
 * trying to complete starts with (e.g. Tilde or Equals).                */

static char ic;

/* This variable says what we are currently adding to the list of matches. */

static int addwhat;

/*
 * Convenience macro for calling quotestring (formerly bslashquote()
 * (formerly quotename())).
 * This uses the instring variable exported from zle_tricky.c.
 */

#define quotename(s) \
quotestring(s, instring == QT_NONE ? QT_BACKSLASH : instring)

/* Hook functions */

static int
ccmakehookfn(UNUSED(Hookdef dummy), struct ccmakedat *dat)
{
    char *s = dat->str;
    int incmd = dat->incmd, lst = dat->lst;
    struct cmlist ms;
    Cmlist m;
    char *os = s;
    int onm = nmatches, odm = diffmatches, osi = movefd(0);
    LinkNode n;

    queue_signals();

    /* We build a copy of the list of matchers to use to make sure that this
     * works even if a shell function called from the completion code changes
     * the global matchers. */

    if ((m = cmatcher)) {
	Cmlist mm, *mp = &mm;
	int n;

	for (n = 0; m; m = m->next, n++) {
	    *mp = (Cmlist) zhalloc(sizeof(struct cmlist));
	    (*mp)->matcher = m->matcher;
	    (*mp)->next = NULL;
	    (*mp)->str = dupstring(m->str);
	    mp = &((*mp)->next);
	    addlinknode(matchers, m->matcher);
	    if (m->matcher)
		m->matcher->refc++;
	}
	m = mm;
    }

    /* Walk through the global matchers. */
    for (;;) {
	bmatchers = NULL;
	if (m) {
	    ms.next = NULL;
	    ms.matcher = m->matcher;
	    mstack = &ms;

	    /* Store the matchers used in the bmatchers list which is used
	     * when building new parts for the string to insert into the 
	     * line. */
	    add_bmatchers(m->matcher);
	} else
	    mstack = NULL;

	ainfo = (Aminfo) hcalloc(sizeof(struct aminfo));
	fainfo = (Aminfo) hcalloc(sizeof(struct aminfo));

	freecl = NULL;

	if (!validlist)
	    lastambig = 0;
	amatches = NULL;
	mnum = 0;
	unambig_mnum = -1;
	isuf = NULL;
	insmnum = zmult;
#if 0
	/* group-numbers in compstate[insert] */
	insgnum = 1;
	insgroup = 0;
#endif
	oldlist = oldins = 0;
	begcmgroup("default", 0);
	menucmp = menuacc = newmatches = onlyexpl = 0;

	ccused = newlinklist();
	ccstack = newlinklist();

	s = dupstring(os);
	makecomplistglobal(s, incmd, lst, 0);
	endcmgroup(NULL);

	if (amatches && !oldlist) {
	    if (lastccused)
		freelinklist(lastccused, (FreeFunc) freecompctl);

	    lastccused = znewlinklist();
	    for (n = firstnode(ccused); n; incnode(n))
		zaddlinknode(lastccused, getdata(n));
	} else if (ccused)
	    for (n = firstnode(ccused); n; incnode(n))
		if (((Compctl) getdata(n)) != &cc_dummy)
		    freecompctl((Compctl) getdata(n));

	if (oldlist) {
	    nmatches = onm;
	    diffmatches = odm;
	    validlist = 1;
	    amatches = lastmatches;
#ifdef ZSH_HEAP_DEBUG
	    if (memory_validate(amatches->heap_id)) {
		HEAP_ERROR(amatches->heap_id);
	    }
#endif
	    lmatches = lastlmatches;
	    if (pmatches) {
		freematches(pmatches, 1);
		pmatches = NULL;
		hasperm = 0;
	    }
	    redup(osi, 0);

	    dat->lst = 0;
	    unqueue_signals();
	    return 0;
	}
	if (lastmatches) {
	    freematches(lastmatches, 1);
	    lastmatches = NULL;
	}
	permmatches(1);
	amatches = pmatches;
	lastpermmnum = permmnum;
	lastpermgnum = permgnum;

	lastmatches = pmatches;
	lastlmatches = lmatches;
	pmatches = NULL;
	hasperm = 0;
	hasoldlist = 1;

	if (nmatches && !errflag) {
	    validlist = 1;

	    redup(osi, 0);

	    dat->lst = 0;
	    unqueue_signals();
	    return 0;
	}
	if (!m || !(m = m->next))
	    break;

	errflag &= ~ERRFLAG_ERROR;
    }
    redup(osi, 0);
    dat->lst = 1;

    unqueue_signals();
    return 0;
}

static int
cccleanuphookfn(UNUSED(Hookdef dummy), UNUSED(void *dat))
{
    ccused = ccstack = NULL;
    return 0;
}

/* This adds a match to the list of matches.  The string to add is given   *
 * in s, the type of match is given in the global variable addwhat and     *
 * the parameter t (if not NULL) is a pointer to a hash node which         *
 * may be used to give other information to this function.                 *
 *                                                                         *
 * addwhat contains either one of the special values (negative, see below) *
 * or the inclusive OR of some of the CC_* flags used for compctls.        */

/**/
static void
addmatch(char *s, char *t)
{
    int isfile = 0, isalt = 0, isexact;
    char *ms = NULL, *tt;
    HashNode hn;
    Param pm;
    Cline lc = NULL;
    Brinfo bp, bpl = brbeg, bsl = brend, bpt, bst;

    for (bp = brbeg; bp; bp = bp->next)
	bp->curpos = ((addwhat == CC_QUOTEFLAG) ? bp->qpos : bp->pos);
    for (bp = brend; bp; bp = bp->next)
	bp->curpos = ((addwhat == CC_QUOTEFLAG) ? bp->qpos : bp->pos);

    /*
     * addwhat: -5 is for files,
     *          -6 is for glob expansions,
     *          -8 is for executable files (e.g. command paths),
     *          -9 is for parameters
     *          -7 is for command names (from cmdnamtab)
     *          -4 is for a cdable parameter
     *          -3 is for executable command names.
     *          -2 is for anything unquoted
     *          -1 is for other file specifications
     *          (things with `~' or `=' at the beginning, ...).
     */

    /* Just to make the code cleaner */
    hn = (HashNode) t;
    pm = (Param) t;

    if (addwhat == -1 || addwhat == -5 || addwhat == -6 ||
	addwhat == CC_FILES || addwhat == -7 || addwhat == -8) {
	int ppl = (ppre ? strlen(ppre) : 0), psl = (psuf ? strlen(psuf) : 0);

	while (bpl && bpl->curpos < ppl)
	    bpl = bpl->next;
	while (bsl && bsl->curpos < psl)
	    bsl = bsl->next;

	if ((addwhat == CC_FILES ||
	     addwhat == -5) && !*psuf) {
	    /* If this is a filename, do the fignore check. */
	    char **pt = fignore;
	    int filell, sl = strlen(s);

	    for (isalt = 0; !isalt && *pt; pt++)
		if ((filell = strlen(*pt)) < sl &&
		    !strcmp(*pt, s + sl - filell))
		    isalt = 1;
	}
	ms = ((addwhat == CC_FILES || addwhat == -6 ||
	       addwhat == -5 || addwhat == -8) ?
	      comp_match(tildequote(qfpre, 1), multiquote(qfsuf, 1),
			 s, filecomp, &lc, (ppre && *ppre ? 1 : 2),
			 &bpl, ppl ,&bsl, psl, &isexact) :
	      comp_match(multiquote(fpre, 1), multiquote(fsuf, 1),
			 s, filecomp, &lc, 0,
			 &bpl, ppl, &bsl, psl, &isexact));
	if (!ms)
	    return;

	if (addwhat == -7 && !findcmd(s, 0, 0))
	    return;
	isfile = CMF_FILE;
    } else if (addwhat == CC_QUOTEFLAG || addwhat == -2  ||
	      (addwhat == -3 && !(hn->flags & DISABLED)) ||
	      (addwhat == -4 && (PM_TYPE(pm->node.flags) == PM_SCALAR) &&
	       !pm->level && (tt = pm->gsu.s->getfn(pm)) && *tt == '/') ||
	      (addwhat == -9 && !(hn->flags & PM_UNSET) && !pm->level) ||
	      (addwhat > 0 &&
	       ((!(hn->flags & PM_UNSET) &&
		 (((addwhat & CC_ARRAYS)    &&  (hn->flags & PM_ARRAY))    ||
		  ((addwhat & CC_INTVARS)   &&  (hn->flags & PM_INTEGER))  ||
		  ((addwhat & CC_ENVVARS)   &&  (hn->flags & PM_EXPORTED)) ||
		  ((addwhat & CC_SCALARS)   &&  (hn->flags & PM_SCALAR))   ||
		  ((addwhat & CC_READONLYS) &&  (hn->flags & PM_READONLY)) ||
		  ((addwhat & CC_SPECIALS)  &&  (hn->flags & PM_SPECIAL))  ||
		  ((addwhat & CC_PARAMS)    && !(hn->flags & PM_EXPORTED))) &&
		 !pm->level) ||
		((( addwhat & CC_SHFUNCS)				  ||
		  ( addwhat & CC_BUILTINS)				  ||
		  ( addwhat & CC_EXTCMDS)				  ||
		  ( addwhat & CC_RESWDS)				  ||
		  ((addwhat & CC_ALREG)   && !(hn->flags & ALIAS_GLOBAL)) ||
		  ((addwhat & CC_ALGLOB)  &&  (hn->flags & ALIAS_GLOBAL))) &&
		 (((addwhat & CC_DISCMDS) && (hn->flags & DISABLED)) ||
		  ((addwhat & CC_EXCMDS)  && !(hn->flags & DISABLED)))) ||
		((addwhat & CC_BINDINGS) && !(hn->flags & DISABLED))))) {
	char *p1, *s1, *p2, *s2;

	if (addwhat == CC_QUOTEFLAG) {
	    p1 = qrpre; s1 = qrsuf;
	    p2 = rpre;  s2 = rsuf;
	} else {
	    p1 = qlpre; s1 = qlsuf;
	    p2 = lpre;  s2 = lsuf;
	}
	p1 = multiquote(p1, 1); s1 = multiquote(s1, 1);
	p2 = multiquote(p2, 1); s2 = multiquote(s2, 1);
	bpt = bpl;
	bst = bsl;

	if (!(ms = comp_match(p1, s1, s, patcomp, &lc,
			      (addwhat == CC_QUOTEFLAG),
			      &bpl, strlen(p1), &bsl, strlen(s1),
			      &isexact))) {
	    bpl = bpt;
	    bsl = bst;
	    if (!(ms = comp_match(p2, s2, s, NULL, &lc,
				  (addwhat == CC_QUOTEFLAG),
				  &bpl, strlen(p2), &bsl, strlen(s2),
				  &isexact)))
		return;
	}
    }
    if (!ms)
	return;
    add_match_data(isalt, ms, s, lc, ipre, ripre, isuf, 
		   (incompfunc ? dupstring(curcc->prefix) : curcc->prefix),
		   prpre, 
		   (isfile ? lppre : NULL), NULL,
		   (isfile ? lpsuf : NULL), NULL,
		   (incompfunc ? dupstring(curcc->suffix) : curcc->suffix),
		   (mflags | isfile), isexact);
}

/**/
static void
maketildelist(void)
{
    /* add all the usernames to the named directory table */
    nameddirtab->filltable(nameddirtab);

    scanhashtable(nameddirtab, 0, (addwhat==-1) ? 0 : ND_USERNAME, 0,
		  addhnmatch, 0);
}

/* This does the check for compctl -x `n' and `N' patterns. */

/**/
static int
getcpat(char *str, int cpatindex, char *cpat, int class)
{
    char *s, *t, *p;
    int d = 0;

    if (!str || !*str)
	return -1;

    cpat = rembslash(cpat);

    if (!cpatindex)
	cpatindex++, d = 0;
    else if ((d = (cpatindex < 0)))
	cpatindex = -cpatindex;

    for (s = d ? str + strlen(str) - 1 : str;
	 d ? (s >= str) : *s;
	 d ? s-- : s++) {
	for (t = s, p = cpat; *t && *p; p++) {
	    if (class) {
		if (*p == *s && !--cpatindex)
		    return (int)(s - str + 1);
	    } else if (*t++ != *p)
		break;
	}
	if (!class && !*p && !--cpatindex)
	    return t - str;
    }
    return -1;
}

/* Dump a hash table (without sorting).  For each element the addmatch  *
 * function is called and at the beginning the addwhat variable is set. *
 * This could be done using scanhashtable(), but this is easy and much  *
 * more efficient.                                                      */

/**/
static void
dumphashtable(HashTable ht, int what)
{
    HashNode hn;
    int i;

    addwhat = what;

    for (i = 0; i < ht->hsize; i++)
	for (hn = ht->nodes[i]; hn; hn = hn->next)
	    addmatch(dupstring(hn->nam), (char *) hn);
}

/* ScanFunc used by maketildelist() et al. */

/**/
static void
addhnmatch(HashNode hn, UNUSED(int flags))
{
    addmatch(hn->nam, NULL);
}

/* Perform expansion on the given string and return the result. *
 * During this errors are not reported.                         */

/**/
static char *
getreal(char *str)
{
    LinkList l = newlinklist();
    int ne = noerrs;

    noerrs = 1;
    addlinknode(l, dupstring(str));
    prefork(l, 0, NULL);
    noerrs = ne;
    if (!errflag && nonempty(l) &&
	((char *) peekfirst(l)) && ((char *) peekfirst(l))[0])
	return dupstring(peekfirst(l));
    errflag &= ~ERRFLAG_ERROR;

    return dupstring(str);
}

/* This reads a directory and adds the files to the list of  *
 * matches.  The parameters say which files should be added. */

/**/
static void
gen_matches_files(int dirs, int execs, int all)
{
    DIR *d;
    struct stat buf;
    char *n, p[PATH_MAX+1], *q = NULL, *e, *pathpref;
    LinkList l = NULL;
    int ns = 0, ng = opts[NULLGLOB], test, aw = addwhat, pathpreflen;

    opts[NULLGLOB] = 1;

    if (*psuf) {
	/* If there is a path suffix, check if it doesn't have a `*' or *
	 * `)' at the end (this is used to determine if we should use   *
	 * globbing).                                                   */
	q = psuf + strlen(psuf) - 1;
	ns = !(*q == Star || *q == Outpar);
	l = newlinklist();
	/* And generate only directory names. */
	dirs = 1;
	all = execs = 0;
    }
    /* Open directory. */
    if (prpre && *prpre) {
	pathpref = dupstring(prpre);
	unmetafy(pathpref, &pathpreflen);
	if (pathpreflen > PATH_MAX)
	    return;
	/* system needs NULL termination, not provided by unmetafy */
	pathpref[pathpreflen] = '\0';
    } else {
	pathpref = NULL;
	pathpreflen = 0;
    }
    if ((d = opendir(pathpref ? pathpref : "."))) {
	/* If we search only special files, prepare a path buffer for stat. */
	if (!all && pathpreflen) {
	    /* include null byte we carefully added */
	    memcpy(p, pathpref, pathpreflen+1);
	}
	q = p + pathpreflen;
	/* Fine, now read the directory. */
	while ((n = zreaddir(d, 1)) && !errflag) {
	    /* Ignore files beginning with `.' unless the thing we found on *
	     * the command line also starts with a dot or GLOBDOTS is set.  */
	    if (*n != '.' || *fpre == '.' || isset(GLOBDOTS)) {
		addwhat = execs ? -8 : -5;
		if (filecomp)
		    /* If we have a pattern for the filename check, use it. */
		    test = pattry(filecomp, n);
		else {
		    /* Otherwise use the prefix and suffix strings directly. */
		    e = n + strlen(n) - fsl;
		    if ((test = !strncmp(n, fpre, fpl)))
			test = !strcmp(e, fsuf);
		    if (!test && mstack) {
			test = 1;
			addwhat = CC_FILES;
		    }
		}
		/* Filename didn't match? */
		if (!test)
		    continue;
		if (!all) {
		    char *ums;
		    int umlen;
		    /* We still have to check the file type, so prepare *
		     * the path buffer by appending the filename.       */
		    ums = dupstring(n);
		    unmetafy(ums, &umlen);
		    if (umlen + pathpreflen + 1 > PATH_MAX)
			continue;
		    memcpy(q, ums, umlen);
		    q[umlen] = '\0';
		    /* And do the stat. */
		    if (stat(p, &buf) < 0)
			continue;
		}
		if (all ||
		    (dirs && S_ISDIR(buf.st_mode)) ||
		    (execs && S_ISREG(buf.st_mode) && (buf.st_mode&S_IXUGO))) {
		    /* If we want all files or the file has the right type... */
		    if (*psuf) {
			/* We have to test for a path suffix. */
			int o = strlen(p), tt;

			if (o + strlen(psuf) > PATH_MAX)
			    continue;
			/* Append it to the path buffer. */
			strcpy(p + o, psuf);

			/* Do we have to use globbing? */
			if (ispattern ||
			    (ns && comppatmatch && *comppatmatch)) {
			    /* Yes, so append a `*' if needed. */
			    if (ns && comppatmatch && *comppatmatch == '*') {
				int tl = strlen(p);

				p[tl] = Star;
				p[tl + 1] = '\0';
			    }
			    /* Do the globbing... */
			    remnulargs(p);
			    addlinknode(l, p);
			    globlist(l, 0);
			    /* And see if that produced a filename. */
			    tt = nonempty(l);
			    while (ugetnode(l));
			} else
			    /* Otherwise just check, if we have access *
			     * to the file.                            */
			    tt = !access(p, F_OK);

			p[o] = '\0';
			if (tt)
			    /* Ok, we can add the filename to the *
			     * list of matches.                   */
			    addmatch(dupstring(n), NULL);
		    } else
			/* We want all files, so just add the name *
			 * to the matches.                         */
			addmatch(dupstring(n), NULL);
		}
	    }
	}
	closedir(d);
    }
    opts[NULLGLOB] = ng;
    addwhat = aw;
}

/* This returns the node with the given data. */
/* ...should probably be moved to linklist.c. */

static LinkNode
findnode(LinkList list, void *dat)
{
    LinkNode tmp = firstnode(list);

    while (tmp && getdata(tmp) != dat) incnode(tmp);

    return tmp;
}

/* A simple counter to avoid endless recursion between old and new style *
 * completion. */

static int cdepth = 0;

#define MAX_CDEPTH 16

/**/
static int
makecomplistctl(int flags)
{
    Heap oldheap;
    int ret;

    if (cdepth == MAX_CDEPTH)
	return 0;

    cdepth++;
    SWITCHHEAPS(oldheap, compheap) {
	int ooffs = offs, lip, lp;
	char *str = comp_str(&lip, &lp, 0), *t;
	char *os = cmdstr, **ow = clwords, **p, **q, qc;
	int on = clwnum, op = clwpos, ois =  instring, oib = inbackt;
	char *oisuf = isuf, *oqp = qipre, *oqs = qisuf, *oaq = autoq;
	char buf[3];

	if (compquote && (qc = *compquote)) {
	    if (qc == '`') {
		instring = QT_NONE;
		/*
		 * Yes, inbackt has always been set to zero here.  I'm
		 * sure there's a simple explanation.
		 */
		inbackt = 0;
		autoq = "";
	    } else {
		switch (qc) {
		case '\'':
		    instring = QT_SINGLE;
		    break;

		case '"':
		    instring = QT_DOUBLE;
		    break;

		case '$':
		    instring = QT_DOLLARS;
		    break;
		}
		inbackt = 0;
		strcpy(buf, *compquote == '$' ? compquote+1 : compquote);
		autoq = buf;
	    }
	} else {
	    instring = QT_NONE;
	    inbackt = 0;
	    autoq = "";
	}
	qipre = ztrdup(compqiprefix ? compqiprefix : "");
	qisuf = ztrdup(compqisuffix ? compqisuffix : "");
	isuf = dupstring(compisuffix);
	ctokenize(isuf);
	remnulargs(isuf);
	clwnum = arrlen(compwords);
	clwpos = compcurrent - 1;
	cmdstr = ztrdup(compwords[0]);
	clwords = (char **) zalloc((clwnum + 1) * sizeof(char *));
	for (p = compwords, q = clwords; *p; p++, q++) {
	    t = dupstring(*p);
	    tokenize(t);
	    remnulargs(t);
	    *q = ztrdup(t);
	}
	*q = NULL;
	offs = lip + lp;
	incompfunc = 2;
	ret = makecomplistglobal(str, !clwpos, COMP_COMPLETE, flags);
	incompfunc = 1;
	isuf = oisuf;
	zsfree(qipre);
	zsfree(qisuf);
	qipre = oqp;
	qisuf = oqs;
	instring = ois;
	inbackt = oib;
	autoq = oaq;
	offs = ooffs;
	zsfree(cmdstr);
	freearray(clwords);
	cmdstr = os;
	clwords = ow;
	clwnum = on;
	clwpos = op;
    } SWITCHBACKHEAPS(oldheap);
    cdepth--;

    return ret;
}

/* This function gets the compctls for the given command line and *
 * adds all completions for them. */

/**/
static int
makecomplistglobal(char *os, int incmd, UNUSED(int lst), int flags)
{
    Compctl cc = NULL;
    char *s;

    ccont = CC_CCCONT;
    cc_dummy.suffix = NULL;

    if (linwhat == IN_ENV) {
        /* Default completion for parameter values. */
	if (!(flags & CFN_DEFAULT)) {
	    cc = &cc_default;
	    keypm = NULL;
	}
    } else if (linwhat == IN_MATH) {
	if (!(flags & CFN_DEFAULT)) {
	    if (insubscr >= 2) {
		/* Inside subscript of assoc array, complete keys. */
		cc_dummy.mask = 0;
		cc_dummy.suffix = (insubscr == 2 ? "]" : "");
	    } else {
		/* Other math environment, complete paramete names. */
		keypm = NULL;
		cc_dummy.mask = CC_PARAMS;
	    }
	    cc = &cc_dummy;
	    cc_dummy.refc = 10000;
	}
    } else if (linwhat == IN_COND) {
	/* We try to be clever here: in conditions we complete option   *
	 * names after a `-o', file names after `-nt', `-ot', and `-ef' *
	 * and file names and parameter names elsewhere.                */
	if (!(flags & CFN_DEFAULT)) {
	    s = clwpos ? clwords[clwpos - 1] : "";
	    cc_dummy.mask = !strcmp("-o", s) ? CC_OPTIONS :
		((*s == '-' && s[1] && !s[2]) ||
		 !strcmp("-nt", s) ||
		 !strcmp("-ot", s) ||
		 !strcmp("-ef", s)) ? CC_FILES :
		(CC_FILES | CC_PARAMS);
	    cc = &cc_dummy;
	    cc_dummy.refc = 10000;
	    keypm = NULL;
	}
    } else if (linredir) {
	if (!(flags & CFN_DEFAULT)) {
	    /* In redirections use default completion. */
	    cc = &cc_default;
	    keypm = NULL;
	}
    } else {
	/* Otherwise get the matches for the command. */
	keypm = NULL;
	return makecomplistcmd(os, incmd, flags);
    }
    if (cc) {
	/* First, use the -T compctl. */
	if (!(flags & CFN_FIRST)) {
	    makecomplistcc(&cc_first, os, incmd);

	    if (!(ccont & CC_CCCONT))
		return 0;
	}
	makecomplistcc(cc, os, incmd);
	return 1;
    }
    return 0;
}

/* This produces the matches for a command. */

/**/
static int
makecomplistcmd(char *os, int incmd, int flags)
{
    Compctl cc;
    Compctlp ccp;
    char *s;
    int ret = 0;

    /* First, use the -T compctl. */
    if (!(flags & CFN_FIRST)) {
	makecomplistcc(&cc_first, os, incmd);

	if (!(ccont & CC_CCCONT))
	    return 0;
    }
    /* Then search the pattern compctls, with the command name and the *
     * full pathname of the command. */
    if (cmdstr) {
	ret |= makecomplistpc(os, incmd);
	if (!(ccont & CC_CCCONT))
	    return ret;
    }
    /* If the command string starts with `=', try the path name of the *
     * command. */
    if (cmdstr && cmdstr[0] == Equals) {
	char *c = findcmd(cmdstr + 1, 1, 0);

	if (c) {
	    zsfree(cmdstr);
	    cmdstr = ztrdup(c);
	}
    }

    /* Find the compctl for this command, trying the full name and then *
     * the trailing pathname component. If that doesn't yield anything, *
     * use default completion. */
    if (incmd)
	cc = &cc_compos;
    else if (!(cmdstr &&
	  (((ccp = (Compctlp) compctltab->getnode(compctltab, cmdstr)) &&
	    (cc = ccp->cc)) ||
	   ((s = dupstring(cmdstr)) && remlpaths(&s, 1) &&
	    (ccp = (Compctlp) compctltab->getnode(compctltab, s)) &&
	    (cc = ccp->cc))))) {
	if (flags & CFN_DEFAULT)
	    return ret;
	cc = &cc_default;
    } else
	ret|= 1;
    makecomplistcc(cc, os, incmd);
    return ret;
}

/* This adds the matches for the pattern compctls. */

/**/
static int
makecomplistpc(char *os, int incmd)
{
    Patcomp pc;
    Patprog pat;
    char *s;
    int ret = 0;

    s = ((shfunctab->getnode(shfunctab, cmdstr) ||
	  builtintab->getnode(builtintab, cmdstr)) ? NULL :
	 findcmd(cmdstr, 1, 0));

    for (pc = patcomps; pc; pc = pc->next) {
	if ((pat = patcompile(pc->pat, PAT_STATIC, NULL)) &&
	    (pattry(pat, cmdstr) ||
	     (s && pattry(pat, s)))) {
	    makecomplistcc(pc->cc, os, incmd);
	    ret |= 2;
	    if (!(ccont & CC_CCCONT))
		return ret;
	}
    }
    return ret;
}

/* This produces the matches for one compctl. */

/**/
static void
makecomplistcc(Compctl cc, char *s, int incmd)
{
    cc->refc++;
    if (!ccused)
	ccused = newlinklist();
    addlinknode(ccused, cc);

    ccont = 0;

    makecomplistor(cc, s, incmd, 0, 0);
}

/* This adds the completions for one run of [x]or'ed completions. */

/**/
static void
makecomplistor(Compctl cc, char *s, int incmd, int compadd, int sub)
{
    int mn, ct, um = usemenu;

    /* Loop over xors. */
    do {
	mn = mnum;

	/* Loop over ors. */
	do {
	    /* Reset the range information if we are not in a sub-list. */
	    if (!sub) {
		brange = 0;
		erange = clwnum - 1;
	    }
	    usemenu = 0;
	    makecomplistlist(cc, s, incmd, compadd);
	    um |= usemenu;

	    ct = cc->mask2 & CC_XORCONT;

	    cc = cc->xor;
	} while (cc && ct);

	/* Stop if we got some matches. */
	if (mn != mnum)
	    break;
	if (cc) {
	    ccont &= ~(CC_DEFCONT | CC_PATCONT);
	    if (!sub)
		ccont &= ~CC_CCCONT;
	}
    } while (cc);

    usemenu = um;
}

/* This dispatches for simple and extended completion. */

/**/
static void
makecomplistlist(Compctl cc, char *s, int incmd, int compadd)
{
    int oloffs = offs, owe = we, owb = wb, ocs = zlemetacs;

    METACHECK();

    if (cc->ext)
	/* Handle extended completion. */
	makecomplistext(cc, s, incmd);
    else
	/* Only normal flags. */
	makecomplistflags(cc, s, incmd, compadd);

    /* Reset some information variables for the next try. */
    errflag &= ~ERRFLAG_ERROR;
    offs = oloffs;
    wb = owb;
    we = owe;
    zlemetacs = ocs;
}

/* This add matches for extended completion patterns */

/**/
static void
makecomplistext(Compctl occ, char *os, int incmd)
{
    Compctl compc;
    Compcond or, cc;
    Patprog pprog;
    int compadd, m = 0, d = 0, t, tt, i, j, a, b, ins;
    char *sc = NULL, *s, *ss;

    ins = (instring != QT_NONE ? instring : (inbackt ? QT_BACKTICK : 0));

    /* This loops over the patterns separated by `-'s. */
    for (compc = occ->ext; compc; compc = compc->next) {
	compadd = t = brange = 0;
	erange = clwnum - 1;
	/* This loops over OR'ed patterns. */
	for (cc = compc->cond; cc && !t; cc = or) {
	    or = cc->or;
	    /* This loops over AND'ed patterns. */
	    for (t = 1; cc && t; cc = cc->and) {
		/* And this loops over [...] pairs. */
		for (t = i = 0; i < cc->n && !t; i++) {
		    s = NULL;
		    brange = 0;
		    erange = clwnum - 1;
		    switch (cc->type) {
		    case CCT_QUOTE:
			t = ((cc->u.s.s[i][0] == 's' && ins == QT_SINGLE) ||
			     (cc->u.s.s[i][0] == 'd' && ins == QT_DOUBLE) ||
			     (cc->u.s.s[i][0] == 'b' && ins == QT_BACKTICK));
			break;
		    case CCT_POS:
			tt = clwpos;
			goto cct_num;
		    case CCT_NUMWORDS:
			tt = clwnum;
		    cct_num:
			if ((a = cc->u.r.a[i]) < 0)
			    a += clwnum;
			if ((b = cc->u.r.b[i]) < 0)
			    b += clwnum;
			if (cc->type == CCT_POS)
			    brange = a, erange = b;
			t = (tt >= a && tt <= b);
			break;
		    case CCT_CURSUF:
		    case CCT_CURPRE:
			s = ztrdup(clwpos < clwnum ? os : "");
			untokenize(s);
			if (isset(COMPLETEINWORD)) s[offs] = '\0';
			sc = rembslash(cc->u.s.s[i]);
			a = strlen(sc);
			if (!strncmp(s, sc, a)) {
			    compadd = (cc->type == CCT_CURSUF ? a : 0);
			    t = 1;
			}
			break;
		    case CCT_CURSUB:
		    case CCT_CURSUBC:
			if (clwpos < 0 || clwpos >= clwnum)
			    t = 0;
			else {
			    s = ztrdup(os);
			    untokenize(s);
			    if (isset(COMPLETEINWORD)) s[offs] = '\0';
			    a = getcpat(s,
					cc->u.s.p[i],
					cc->u.s.s[i],
					cc->type == CCT_CURSUBC);
			    if (a != -1)
				compadd = a, t = 1;
			}
			break;
			
		    case CCT_CURPAT:
		    case CCT_CURSTR:
			tt = clwpos;
			goto cct_str;
		    case CCT_WORDPAT:
		    case CCT_WORDSTR:
			tt = 0;
		    cct_str:
			if ((a = tt + cc->u.s.p[i]) < 0)
			    a += clwnum;
			s = ztrdup((a < 0 || a >= clwnum) ? "" :
				   clwords[a]);
			untokenize(s);
			
			if (cc->type == CCT_CURPAT ||
			    cc->type == CCT_WORDPAT) {
			    tokenize(ss = dupstring(cc->u.s.s[i]));
			    t = ((pprog = patcompile(ss, PAT_STATIC, NULL)) &&
				 pattry(pprog, s));
			} else
			    t = (!strcmp(s, rembslash(cc->u.s.s[i])));
			break;
		    case CCT_RANGESTR:
		    case CCT_RANGEPAT:
			if (cc->type == CCT_RANGEPAT)
			    tokenize(sc = dupstring(cc->u.l.a[i]));
			for (j = clwpos - 1; j > 0; j--) {
			    untokenize(s = ztrdup(clwords[j]));
			    if (cc->type == CCT_RANGESTR)
				sc = rembslash(cc->u.l.a[i]);
			    if (cc->type == CCT_RANGESTR ?
				!strncmp(s, sc, strlen(sc)) :
				((pprog = patcompile(sc, PAT_STATIC, 0)) &&
				 pattry(pprog, s))) {
				zsfree(s);
				brange = j + 1;
				t = 1;
				break;
			    }
			    zsfree(s);
			}
			if (t && cc->u.l.b[i]) {
			    if (cc->type == CCT_RANGEPAT)
				tokenize(sc = dupstring(cc->u.l.b[i]));
			    for (j++; j < clwnum; j++) {
				untokenize(s = ztrdup(clwords[j]));
				if (cc->type == CCT_RANGESTR)
				    sc = rembslash(cc->u.l.b[i]);
				if (cc->type == CCT_RANGESTR ?
				    !strncmp(s, sc, strlen(sc)) :
				    ((pprog = patcompile(sc, PAT_STATIC, 0)) &&
				     pattry(pprog, s))) {
				    zsfree(s);
				    erange = j - 1;
				    t = clwpos <= erange;
				    break;
				}
				zsfree(s);
			    }
			}
			s = NULL;
		    }
		    zsfree(s);
		}
	    }
	}
	if (t) {
	    /* The patterns matched, use the flags. */
	    m = 1;
	    ccont &= ~(CC_PATCONT | CC_DEFCONT);
	    makecomplistor(compc, os, incmd, compadd, 1);
	    if (!d && (ccont & CC_DEFCONT)) {
		d = 1;
		compadd = 0;
		brange = 0;
		erange = clwnum - 1;
		makecomplistflags(occ, os, incmd, 0);
	    }
	    if (!(ccont & CC_PATCONT))
		break;
	}
    }
    /* If no pattern matched, use the standard flags. */
    if (!m) {
	compadd = 0;
	brange = 0;
	erange = clwnum - 1;
	makecomplistflags(occ, os, incmd, 0);
    }
}

/**/
static int
sep_comp_string(char *ss, char *s, int noffs)
{
    LinkList foo = newlinklist();
    LinkNode n;
    int owe = we, owb = wb, ocs = zlemetacs, swb, swe, scs, soffs, ne = noerrs;
    int sl = strlen(ss), tl, got = 0, i = 0, cur = -1, oll = zlemetall, remq;
    int ois = instring, oib = inbackt, ona = noaliases;
    char *tmp, *p, *ns, *ol = zlemetaline, sav, *oaq = autoq;
    char *qp, *qs, *ts;

    swb = swe = soffs = 0;
    ns = NULL;

    METACHECK();

    /* Put the string in the lexer buffer and call the lexer to *
     * get the words we have to expand.                        */
    addedx = 1;
    noerrs = 1;
    zcontext_save();
    lexflags = LEXFLAGS_ZLE;
    tmp = (char *) zhalloc(tl = sl + 3 + strlen(s));
    strcpy(tmp, ss);
    tmp[sl] = ' ';
    memcpy(tmp + sl + 1, s, noffs);
    tmp[(scs = zlemetacs = sl + 1 + noffs)] = 'x';
    strcpy(tmp + sl + 2 + noffs, s + noffs);
    if ((remq = (*compqstack == QT_BACKSLASH)))
	tmp = rembslash(tmp);
    inpush(dupstrspace(tmp), 0, NULL);
    zlemetaline = tmp;
    zlemetall = tl - 1;
    strinbeg(0);
    noaliases = 1;
    do {
	ctxtlex();
	if (tok == LEXERR) {
	    int j;

	    if (!tokstr)
		break;
	    for (j = 0, p = tokstr; *p; p++)
		if (*p == Snull || *p == Dnull)
		    j++;
	    if (j & 1) {
		tok = STRING;
		if (p > tokstr && p[-1] == ' ')
		    p[-1] = '\0';
	    }
	}
	if (tok == ENDINPUT || tok == LEXERR)
	    break;
	if (tokstr && *tokstr)
	    addlinknode(foo, (p = ztrdup(tokstr)));
	else
	    p = NULL;
	if (!got && !lexflags) {
	    DPUTS(!p, "no current word in substr");
	    got = 1;
	    cur = i;
	    swb = wb - 1;
	    swe = we - 1;
	    soffs = zlemetacs - swb;
	    chuck(p + soffs);
	    ns = dupstring(p);
	}
	i++;
    } while (tok != ENDINPUT && tok != LEXERR);
    noaliases = ona;
    strinend();
    inpop();
    errflag &= ~ERRFLAG_ERROR;
    noerrs = ne;
    zcontext_restore();
    wb = owb;
    we = owe;
    zlemetacs = ocs;
    zlemetaline = ol;
    zlemetall = oll;
    if (cur < 0 || i < 1)
	return 1;
    owb = offs;
    offs = soffs;
    if ((p = check_param(ns, 0, 1))) {
	for (p = ns; *p; p++)
	    if (*p == Dnull)
		*p = '"';
	    else if (*p == Snull)
		*p = '\'';
    }
    offs = owb;

    untokenize(ts = dupstring(ns));

    if (*ns == Snull || *ns == Dnull ||
	((*ns == String || *ns == Qstring) && ns[1] == Snull)) {
	char *tsptr = ts, *nsptr = ns, sav;
	switch (*ns) {
	case Snull:
	    instring = QT_SINGLE;
	    break;

	case Dnull:
	    instring = QT_DOUBLE;
	    break;

	default:
	    instring = QT_DOLLARS;
	    nsptr++;
	    tsptr++;
	    break;
	}

	inbackt = 0;
	swb++;
	if (nsptr[strlen(nsptr) - 1] == *nsptr && nsptr[1])
	    swe--;
	sav = *++tsptr;
	*tsptr = '\0';
	autoq = compqstack[1] ? "" : multiquote(ts, 1);
	*(ts = tsptr) = sav;
    } else {
	instring = QT_NONE;
	autoq = "";
    }
    for (p = ns, i = swb; *p; p++, i++) {
	if (inull(*p)) {
	    if (i < scs) {
		soffs--;
		if (remq && *p == Bnull && p[1])
		    swb -= 2;
	    }
	    if (p[1] || *p != Bnull) {
		if (*p == Bnull) {
		    if (scs == i + 1)
			scs++, soffs++;
		} else {
		    if (scs > i--)
			scs--;
		}
	    } else {
		if (scs == swe)
		    scs--;
	    }
	    chuck(p--);
	}
    }
    ns = ts;

    if (instring != QT_NONE && strchr(compqstack, QT_BACKSLASH)) {
	int rl = strlen(ns), ql = strlen(multiquote(ns, !!compqstack[1]));

	if (ql > rl)
	    swb -= ql - rl;
    }
    sav = s[(i = swb - sl - 1)];
    s[i] = '\0';
    qp = tricat(qipre, multiquote(s, 0), "");
    s[i] = sav;
    if (swe < swb)
	swe = swb;
    swe -= sl + 1;
    sl = strlen(s);
    if (swe > sl) {
	swe = sl;
	if ((int)strlen(ns) > swe - swb + 1)
	    ns[swe - swb + 1] = '\0';
    }
    qs = tricat(multiquote(s + swe, 0), qisuf, "");
    sl = strlen(ns);
    if (soffs > sl)
	soffs = sl;

    {
	char **ow = clwords, *os = cmdstr, *oqp = qipre, *oqs = qisuf;
	char *oqst = compqstack, compnewchar[2];
	int olws = clwsize, olwn = clwnum, olwp = clwpos;
	int obr = brange, oer = erange, oof = offs;
	unsigned long occ = ccont;

	compnewchar[0] = (char)(instring != QT_NONE ? (char)instring :
				QT_BACKSLASH);
	compnewchar[1] = '\0';
	compqstack = tricat(compnewchar, compqstack, "");

	clwsize = clwnum = countlinknodes(foo);
	clwords = (char **) zalloc((clwnum + 1) * sizeof(char *));
	for (n = firstnode(foo), i = 0; n; incnode(n), i++) {
	    p = clwords[i] = (char *) getdata(n);
	    untokenize(p);
	}
	clwords[i] = NULL;
	clwpos = cur;
	cmdstr = ztrdup(clwords[0]);
	brange = 0;
	erange = clwnum - 1;
	qipre = qp;
	qisuf = qs;
	offs = soffs;
	ccont = CC_CCCONT;
	makecomplistcmd(ns, !clwpos, CFN_FIRST);
	ccont = occ;
	offs = oof;
	zsfree(cmdstr);
	cmdstr = os;
	freearray(clwords);
	clwords = ow;
	clwsize = olws;
	clwnum = olwn;
	clwpos = olwp;
	brange = obr;
	erange = oer;
	zsfree(qipre);
	qipre = oqp;
	zsfree(qisuf);
	qisuf = oqs;
	zsfree(compqstack);
	compqstack = oqst;
    }
    autoq = oaq;
    instring = ois;
    inbackt = oib;

    return 0;
}

/* This adds the completions for the flags in the given compctl. */

/**/
static void
makecomplistflags(Compctl cc, char *s, int incmd, int compadd)
{
    int t, sf1, sf2, ooffs, um = usemenu, delit, oaw, gflags;
    int mn = mnum, ohp = haspattern;
    char *p, *sd = NULL, *tt, *s1, *s2, *os =  dupstring(s);
    struct cmlist ms;

    ccont |= (cc->mask2 & (CC_CCCONT | CC_DEFCONT | CC_PATCONT));

    if (incompfunc != 1 && ccstack && findnode(ccstack, cc))
	return;

    if (!ccstack)
	ccstack = newlinklist();
    addlinknode(ccstack, cc);

    if (incompfunc != 1 && allccs) {
	if (findnode(allccs, cc)) {
	    uremnode(ccstack, firstnode(ccstack));
	    return;
	}
	addlinknode(allccs, cc);
    }
    /* Go to the end of the word if complete_in_word is not set. */
    if (unset(COMPLETEINWORD) && zlemetacs != we)
	zlemetacs = we, offs = strlen(s);

    s = dupstring(s);
    delit = ispattern = 0;
    usemenu = um;
    patcomp = filecomp = NULL;
    rpre = rsuf = lpre = lsuf = ppre = psuf = lppre = lpsuf =
	fpre = fsuf = ipre = ripre = prpre = 
	qfpre = qfsuf = qrpre = qrsuf = qlpre = qlsuf = NULL;

    curcc = cc;

    mflags = 0;
    gflags = (((cc->mask2 & CC_NOSORT ) ? CGF_NOSORT  : 0) |
	      ((cc->mask2 & CC_UNIQALL) ? CGF_UNIQALL : 0) |
	      ((cc->mask2 & CC_UNIQCON) ? CGF_UNIQCON : 0));
    if (cc->gname) {
	endcmgroup(NULL);
	begcmgroup(cc->gname, gflags);
    }
    if (cc->ylist) {
	endcmgroup(NULL);
	begcmgroup(NULL, gflags);
    }
    if (cc->mask & CC_REMOVE)
	mflags |= CMF_REMOVE;
    if (cc->explain) {
	curexpl = (Cexpl) zhalloc(sizeof(struct cexpl));
	curexpl->count = curexpl->fcount = 0;
    } else
	curexpl = NULL;
    /* compadd is the number of characters we have to ignore at the  *
     * beginning of the word.                                        */
    if (compadd) {
	ipre = dupstring(s);
	ipre[compadd] = '\0';
	untokenize(ipre);
	wb += compadd;
	s += compadd;
	if ((offs -= compadd) < 0) {
	    /* It's bigger than our word prefix, so we can't help here... */
	    uremnode(ccstack, firstnode(ccstack));
	    return;
	}
    } else
	ipre = NULL;

    if (cc->matcher) {
	ms.next = mstack;
	ms.matcher = cc->matcher;
	mstack = &ms;

	if (!mnum)
	    add_bmatchers(cc->matcher);

	addlinknode(matchers, cc->matcher);
	cc->matcher->refc++;
    }
    if (mnum && (mstack || bmatchers))
	update_bmatchers();

    /* Insert the prefix (compctl -P), if any. */
    if (cc->prefix) {
	int pl = 0;

	if (*s) {
	    char *dp = rembslash(cc->prefix);
	    /* First find out how much of the prefix is already on the line. */
	    sd = dupstring(s);
	    untokenize(sd);
	    pl = pfxlen(dp, sd);
	    s += pl;
	    sd += pl;
	    offs -= pl;
	}
    }
    /* Does this compctl have a suffix (compctl -S)? */
    if (cc->suffix) {
	char *sdup = rembslash(cc->suffix);
	int sl = strlen(sdup), suffixll;

	/* Ignore trailing spaces. */
	for (p = sdup + sl - 1; p >= sdup && *p == ' '; p--, sl--);
	p[1] = '\0';

	if (!sd) {
	    sd = dupstring(s);
	    untokenize(sd);
	}
	/* If the suffix is already there, ignore it (and don't add *
	 * it again).                                               */
	if (*sd && (suffixll = strlen(sd)) >= sl &&
	    offs <= suffixll - sl && !strcmp(sdup, sd + suffixll - sl))
	    s[suffixll - sl] = '\0';
    }
    /* Do we have one of the special characters `~' and `=' at the beginning? */
    if (incompfunc || ((ic = *s) != Tilde && ic != Equals))
	ic = 0;

    /* Check if we have to complete a parameter name... */
    if (!incompfunc && (p = check_param(s, 1, 0))) {
	s = p;
	/* And now make sure that we complete parameter names. */
	cc = &cc_dummy;
	cc_dummy.refc = 10000;
	cc_dummy.mask = CC_PARAMS | CC_ENVVARS;
    }
    ooffs = offs;
    /* If we have to ignore the word, do that. */
    if (cc->mask & CC_DELETE) {
	delit = 1;
	*s = '\0';
	offs = 0;
	if (isset(AUTOMENU))
	    usemenu = 1;
    }

    /* Compute line prefix/suffix. */
    lpl = offs;
    lpre = zhalloc(lpl + 1);
    if (comppatmatch)
    {
	int ccount;
	char *psrc, *pdst;
	for (ccount = 0, psrc = s, pdst = lpre;
	     ccount < lpl;
	     ++ccount, ++psrc, ++pdst)
	{
	    if (*psrc == Meta)
	    {
		ccount++;
		*pdst++ = *psrc++;
		*pdst = *psrc;
	    } else if (*psrc == Dash)
		*pdst = '-';
	    else
		*pdst = *psrc;
	}
    }
    else
	memcpy(lpre, s, lpl);
    lpre[lpl] = '\0';
    qlpre = quotename(lpre);
    lsuf = dupstring(s + offs);
    lsl = strlen(lsuf);
    qlsuf = quotename(lsuf);

    /* First check for ~.../... */
    if (ic == Tilde) {
	for (p = lpre + lpl; p > lpre; p--)
	    if (*p == '/')
		break;

	if (*p == '/')
	    ic = 0;
    }
    /* Compute real prefix/suffix. */

    noreal = !delit;
    for (p = lpre; *p && *p != String && *p != Tick; p++);
    tt = ic && !ispar ? lpre + 1 : lpre;
    rpre = (*p || *lpre == Tilde || *lpre == Equals) ?
	(noreal = 0, getreal(tt)) :
	dupstring(tt);
    qrpre = quotename(rpre);

    for (p = lsuf; *p && *p != String && *p != Tick; p++);
    rsuf = *p ? (noreal = 0, getreal(lsuf)) : dupstring(lsuf);
    qrsuf = quotename(rsuf);

    /* Check if word is a pattern. */

    for (s1 = NULL, sf1 = 0, p = rpre + (rpl = strlen(rpre)) - 1;
	 p >= rpre && (ispattern != 3 || !sf1);
	 p--)
	if (itok(*p) && (p > rpre || (*p != Equals && *p != Tilde)))
	    ispattern |= sf1 ? 1 : 2;
	else if (*p == '/') {
	    sf1++;
	    if (!s1)
		s1 = p;
	}
    rsl = strlen(rsuf);
    for (s2 = NULL, sf2 = t = 0, p = rsuf; *p && (!t || !sf2); p++)
	if (itok(*p))
	    t |= sf2 ? 4 : 2;
	else if (*p == '/') {
	    sf2++;
	    if (!s2)
		s2 = p;
	}
    ispattern = ispattern | t;

    /* But if we were asked not to do glob completion, we never treat the *
     * thing as a pattern.                                                */
    if (!comppatmatch || !*comppatmatch)
	ispattern = 0;

    if (ispattern) {
	/* The word should be treated as a pattern, so compute the matcher. */
	p = (char *) zhalloc(rpl + rsl + 2);
	strcpy(p, rpre);
	if (rpl && p[rpl - 1] != Star &&
	    (!comppatmatch || *comppatmatch == '*')) {
	    p[rpl] = Star;
	    strcpy(p + rpl + 1, rsuf);
	} else
	    strcpy(p + rpl, rsuf);
	patcomp = patcompile(p, 0, NULL);
	haspattern = 1;
    }
    if (!patcomp) {
	untokenize(rpre);
	untokenize(rsuf);

	rpl = strlen(rpre);
	rsl = strlen(rsuf);
    }
    else
    {
	for (p = rpre; *p; ++p)
	    if (*p == Dash)
		*p = '-';
	for (p = rsuf; *p; ++p)
	    if (*p == Dash)
		*p = '-';
    }
    untokenize(lpre);
    untokenize(lsuf);

    if (!(cc->mask & CC_DELETE))
	hasmatched = 1;

    /* Handle completion of files specially (of course). */

    if ((cc->mask & (CC_FILES | CC_DIRS | CC_COMMPATH)) || cc->glob) {
	/* s1 and s2 point to the last/first slash in the prefix/suffix. */
	if (!s1)
	    s1 = rpre;
	if (!s2)
	    s2 = rsuf + rsl;

	/* Compute the path prefix/suffix. */
	if (*s1 != '/')
	    ppre = "";
	else
	    ppre = dupstrpfx(rpre, s1 - rpre + 1);
	psuf = dupstring(s2);

	if (zlemetacs != wb) {
	    char save = zlemetaline[zlemetacs];

	    zlemetaline[zlemetacs] = 0;
	    lppre = dupstring(zlemetaline + wb +
			      (qipre && *qipre ?
			       (strlen(qipre) -
				(*qipre == '\'' || *qipre == '\"')) : 0));
	    zlemetaline[zlemetacs] = save;
	    if (brbeg) {
		Brinfo bp;

		for (bp = brbeg; bp; bp = bp->next)
		    strcpy(lppre + bp->qpos,
			   lppre + bp->qpos + strlen(bp->str));
	    }
	    if ((p = strrchr(lppre, '/'))) {
		p[1] = '\0';
		lppl = strlen(lppre);
	    } else if (!sf1) {
		lppre = NULL;
		lppl = 0;
	    } else {
		lppre = ppre;
		lppl = strlen(lppre);
	    }
	} else {
	    lppre = NULL;
	    lppl = 0;
	}
	if (zlemetacs != we) {
	    int end = we;
	    char save = zlemetaline[end];

	    if (qisuf && *qisuf) {
		int ql = strlen(qisuf);

		end -= ql - (qisuf[ql-1] == '\'' || qisuf[ql-1] == '"');
	    }
	    zlemetaline[end] = 0;
	    lpsuf = dupstring(zlemetaline + zlemetacs);
	    zlemetaline[end] = save;
	    if (brend) {
		Brinfo bp;

		for (bp = brend; bp; bp = bp->next) {
		    char *p2 = lpsuf + (we - zlemetacs) - bp->qpos;
		    char *p1 = p2 - strlen(bp->str);
		    memmove(p1, p2, strlen(p2) + 1);
		}
	    }
	    if (!(lpsuf = strchr(lpsuf, '/')) && sf2)
		lpsuf = psuf;
	    lpsl = (lpsuf ? strlen(lpsuf) : 0);
	} else {
	    lpsuf = NULL;
	    lpsl = 0;
	}

	/* And get the file prefix. */
	fpre = dupstring(((s1 == s || s1 == rpre || ic) &&
			  (*s != '/' || zlemetacs == wb)) ? s1 : s1 + 1);
	qfpre = quotename(fpre);
	/* And the suffix. */
	fsuf = dupstrpfx(rsuf, s2 - rsuf);
	qfsuf = quotename(fsuf);

	if (comppatmatch && *comppatmatch && (ispattern & 2)) {
	    int t2;

	    /* We have to use globbing, so compute the pattern from *
	     * the file prefix and suffix with a `*' between them.  */
	    p = (char *) zhalloc((t2 = strlen(fpre)) + strlen(fsuf) + 2);
	    strcpy(p, fpre);
	    if ((!t2 || p[t2 - 1] != Star) && *fsuf != Star &&
		(!comppatmatch || *comppatmatch == '*'))
		p[t2++] = Star;
	    strcpy(p + t2, fsuf);
	    filecomp = patcompile(p, 0, NULL);
	}
	if (!filecomp) {
	    untokenize(fpre);
	    untokenize(fsuf);

	    fpl = strlen(fpre);
	    fsl = strlen(fsuf);
	}
	addwhat = -1;

	/* Completion after `~', maketildelist adds the usernames *
	 * and named directories.                                 */
	if (ic == Tilde) {
	    char *oi = ipre;

	    ipre = (ipre ? dyncat("~", ipre) : "~");
	    maketildelist();
	    ipre = oi;
	} else if (ic == Equals) {
	    /* Completion after `=', get the command names from *
	     * the cmdnamtab and aliases from aliastab.         */
	    char *oi = ipre;

	    ipre = (ipre ? dyncat("=", ipre) : "=");
	    if (isset(HASHLISTALL))
		cmdnamtab->filltable(cmdnamtab);
	    dumphashtable(cmdnamtab, -7);
	    dumphashtable(aliastab, -2);
	    ipre = oi;
	} else {
	    /* Normal file completion... */
	    if (ispattern & 1) {
		/* But with pattern matching. */
		LinkList l = newlinklist();
		LinkNode n;
		int ng = opts[NULLGLOB];

		opts[NULLGLOB] = 1;

		addwhat = 0;
		p = (char *) zhalloc(lpl + lsl + 3);
		strcpy(p, lpre);
		if (*lsuf != '*' && *lpre && lpre[lpl - 1] != '*')
		    strcat(p, "*");
		strcat(p, lsuf);
		if (*lsuf && lsuf[lsl - 1] != '*' && lsuf[lsl - 1] != ')')
		    strcat(p, "*");

		/* Do the globbing. */
		tokenize(p);
		remnulargs(p);
		addlinknode(l, p);
		globlist(l, 0);

		if (nonempty(l)) {
		    /* And add the resulting words. */
		    mflags |= CMF_FILE;
		    for (n = firstnode(l); n; incnode(n))
			addmatch(getdata(n), NULL);
		    mflags &= ~CMF_FILE;
		}
		opts[NULLGLOB] = ng;
	    } else {
		char **dirs = 0, *ta[2];

		/* No pattern matching. */
		addwhat = CC_FILES;

		if (cc->withd) {
		    char **pp, **npp, *tp;
		    int tl = strlen(ppre) + 2, pl;

		    if ((pp = get_user_var(cc->withd))) {
			dirs = npp =
			    (char**) zhalloc(sizeof(char *)*(arrlen(pp)+1));
			while (*pp) {
			    pl = strlen(*pp);
			    tp = (char *) zhalloc(strlen(*pp) + tl);
			    strcpy(tp, *pp);
			    tp[pl] = '/';
			    strcpy(tp + pl + 1, ppre);
			    *npp++ = tp;
			    pp++;
			}
			*npp = NULL;
		    }
		}
		if (!dirs) {
		    dirs = ta;
		    if (cc->withd) {
			char *tp;
			int pl = strlen(cc->withd);

			ta[0] = tp = (char *) zhalloc(strlen(ppre) + pl + 2);
			strcpy(tp, cc->withd);
			tp[pl] = '/';
			strcpy(tp + pl + 1, ppre);
		    } else
			ta[0] = ppre;
		    ta[1] = NULL;
		}
		while (*dirs) {
		    prpre = *dirs;

		    if (sf2)
			/* We are in the path, so add only directories. */
			gen_matches_files(1, 0, 0);
		    else {
			if (cc->mask & CC_FILES)
			    /* Add all files. */
			    gen_matches_files(0, 0, 1);
			else if (cc->mask & CC_COMMPATH) {
			    /* Completion of command paths. */
			    if (sf1 || cc->withd)
				/* There is a path prefix, so add *
				 * directories and executables.   */
				gen_matches_files(1, 1, 0);
			    else {
				/* No path prefix, so add the things *
				 * reachable via the PATH variable.  */
				char **pc = path, *pp = prpre;

				for (; *pc; pc++)
				    if (!**pc || (pc[0][0] == '.' && !pc[0][1]))
					break;
				if (*pc) {
				    prpre = "./";
				    gen_matches_files(1, 1, 0);
				    prpre = pp;
				}
			    }
			} else if (cc->mask & CC_DIRS)
			    gen_matches_files(1, 0, 0);
			/* The compctl has a glob pattern (compctl -g). */
			if (cc->glob) {
			    int ns, pl = strlen(prpre), o, paalloc;
			    char *g = dupstring(cc->glob), *pa;
			    char *p2, *p3;
			    int ne = noerrs, md = opts[MARKDIRS];

			    /* These are used in the globbing code to make *
			     * things a bit faster.                        */
			    if (ispattern || mstack)
				glob_pre = glob_suf = NULL;
			    else {
				glob_pre = fpre;
				glob_suf = fsuf;
			    }
			    noerrs = 1;
			    addwhat = -6;
			    o = strlen(prpre);
			    pa = (char *)zalloc(paalloc = o + PATH_MAX);
			    strcpy(pa, prpre);
			    opts[MARKDIRS] = 0;

			    /* The compctl -g string may contain more than *
			     * one pattern, so we need a loop.             */
			    while (*g) {
				LinkList l = newlinklist();
				int ng;

				/* Find the blank terminating the pattern. */
				while (*g && inblank(*g))
				    g++;
				/* Oops, we already reached the end of the
				   string. */
				if (!*g)
				    break;
				for (p = g + 1; *p && !inblank(*p); p++)
				    if (*p == '\\' && p[1])
					p++;
				/* Get the pattern string. */
				tokenize(g = dupstrpfx(g, p - g));
				if (*g == '=' && isset(EQUALS))
				    *g = Equals;
				if (*g == '~')
				    *g = Tilde;
				remnulargs(g);
				if ((*g == Equals || *g == Tilde) && !cc->withd) {
				/* The pattern has a `~' or `=' at the  *
				 * beginning, so we expand this and use *
				 * the result.                          */
				    filesub(&g, 0);
				    addlinknode(l, dupstring(g));
				} else if (*g == '/' && !cc->withd)
				/* The pattern is a full path (starting *
				 * with '/'), so add it unchanged.      */
				    addlinknode(l, dupstring(g));
				else {
				/* It's a simple pattern, so append it to *
				 * the path we have on the command line.  */
				    int minlen = o + strlen(g);
				    if (minlen >= paalloc)
					pa = (char *)
					    zrealloc(pa, paalloc = minlen+1);
				    strcpy(pa + o, g);
				    addlinknode(l, dupstring(pa));
				}
				/* Do the globbing. */
				ng = opts[NULLGLOB];
				opts[NULLGLOB] = 1;
				globlist(l, 0);
				opts[NULLGLOB] = ng;
				/* Get the results. */
				if (nonempty(l) && peekfirst(l)) {
				    for (p2 = (char *)peekfirst(l); *p2; p2++)
					if (itok(*p2))
					    break;
				    if (!*p2) {
					if ((*g == Equals || *g == Tilde ||
					     *g == '/') || cc->withd) {
					    /* IF the pattern started with `~',  *
					     * `=', or `/', add the result only, *
					     * if it really matches what we have *
					     * on the line.                      *
					     * Do this if an initial directory   *
					     * was specified, too.               */
					    while ((p2 = (char *)ugetnode(l)))
						if (strpfx(prpre, p2))
						    addmatch(p2 + pl, NULL);
					} else {
					    /* Otherwise ignore the path we *
					     * prepended to the pattern.    */
					    while ((p2 = p3 =
						    (char *)ugetnode(l))) {
						for (ns = sf1; *p3 && ns; p3++)
						    if (*p3 == '/')
							ns--;

						addmatch(p3, NULL);
					    }
					}
				    }
				}
				pa[o] = '\0';
				g = p;
			    }
			    glob_pre = glob_suf = NULL;
			    noerrs = ne;
			    opts[MARKDIRS] = md;

			    zfree(pa, paalloc);
			}
		    }
		    dirs++;
		}
		prpre = NULL;
	    }
	}
	lppre = lpsuf = NULL;
	lppl = lpsl = 0;
    }
    if (ic) {
	/* Now change the `~' and `=' tokens to the real characters so *
	 * that things starting with these characters will be added.   */
	rpre = dyncat((ic == Tilde) ? "~" : "=", rpre);
	rpl++;
	qrpre = dyncat((ic == Tilde) ? "~" : "=", qrpre);
    }
    if (!ic && (cc->mask & CC_COMMPATH) && !*ppre && !*psuf) {
	/* If we have to complete commands, add alias names, *
	 * shell functions and builtins too.                 */
	dumphashtable(aliastab, -3);
	dumphashtable(reswdtab, -3);
	dumphashtable(shfunctab, -3);
	dumphashtable(builtintab, -3);
	if (isset(HASHLISTALL))
	    cmdnamtab->filltable(cmdnamtab);
	dumphashtable(cmdnamtab, -3);
	/* And parameter names if autocd and cdablevars are set. */
	if (isset(AUTOCD) && isset(CDABLEVARS))
	    dumphashtable(paramtab, -4);
    }
    oaw = addwhat = (cc->mask & CC_QUOTEFLAG) ? -2 : CC_QUOTEFLAG;

    if (cc->mask & CC_NAMED)
	/* Add named directories. */
	dumphashtable(nameddirtab, addwhat);
    if (cc->mask & CC_OPTIONS)
	/* Add option names. */
	dumphashtable(optiontab, addwhat);
    if (cc->mask & CC_VARS) {
	/* And parameter names. */
	dumphashtable(paramtab, -9);
	addwhat = oaw;
    }
    if (cc->mask & CC_BINDINGS) {
	/* And zle function names... */
	dumphashtable(thingytab, CC_BINDINGS);
	addwhat = oaw;
    }
    if (cc->keyvar) {
	/* This adds things given to the compctl -k flag *
	 * (from a parameter or a list of words).        */
	char **usr = get_user_var(cc->keyvar);

	if (usr)
	    while (*usr)
		addmatch(*usr++, NULL);
    }
    if (cc->mask & CC_USERS) {
	/* Add user names. */
	maketildelist();
	addwhat = oaw;
    }
    if (cc->func) {
	/* This handles the compctl -K flag. */
	Shfunc shfunc;
	char **r;
	int lv = lastval;
	    
	/* Get the function. */
	if ((shfunc = getshfunc(cc->func))) {
	    /* We have it, so build a argument list. */
	    LinkList args = newlinklist();
	    int osc = sfcontext;
		
	    addlinknode(args, cc->func);
		
	    if (delit) {
		p = dupstrpfx(os, ooffs);
		untokenize(p);
		addlinknode(args, p);
		p = dupstring(os + ooffs);
		untokenize(p);
		addlinknode(args, p);
	    } else {
		addlinknode(args, lpre);
		addlinknode(args, lsuf);
	    }
		
	    /* This flag allows us to use read -l and -c. */
	    if (incompfunc != 1)
		incompctlfunc = 1;
	    sfcontext = SFC_COMPLETE;
	    /* Call the function. */
	    doshfunc(shfunc, args, 1);
	    sfcontext = osc;
	    incompctlfunc = 0;
	    /* And get the result from the reply parameter. */
	    if ((r = get_user_var("reply")))
		while (*r)
		    addmatch(*r++, NULL);
	}
	lastval = lv;
    }
    if (cc->mask & (CC_JOBS | CC_RUNNING | CC_STOPPED)) {
	/* Get job names. */
	int i;
	char *j;

	for (i = 0; i <= maxjob; i++)
	    if ((jobtab[i].stat & STAT_INUSE) &&
		jobtab[i].procs && jobtab[i].procs->text[0]) {
		int stopped = jobtab[i].stat & STAT_STOPPED;

		j = dupstring(jobtab[i].procs->text);
		if ((cc->mask & CC_JOBS) ||
		    (stopped && (cc->mask & CC_STOPPED)) ||
		    (!stopped && (cc->mask & CC_RUNNING)))
		    addmatch(j, NULL);
	    }
    }
    if (cc->str) {
	/* Get the stuff from a compctl -s. */
	LinkList foo = newlinklist();
	LinkNode n;
	int first = 1, ng = opts[NULLGLOB], oowe = we, oowb = wb;
	int ona = noaliases;
	char *tmpbuf;

	opts[NULLGLOB] = 1;

	/* Put the string in the lexer buffer and call the lexer to *
	 * get the words we have to expand.                        */
	zcontext_save();
	lexflags = LEXFLAGS_ZLE;
	tmpbuf = (char *)zhalloc(strlen(cc->str) + 5);
	sprintf(tmpbuf, "foo %s", cc->str); /* KLUDGE! */
	inpush(tmpbuf, 0, NULL);
	strinbeg(0);
	noaliases = 1;
	do {
	    ctxtlex();
	    if (tok == ENDINPUT || tok == LEXERR)
		break;
	    if (!first && tokstr && *tokstr)
		addlinknode(foo, ztrdup(tokstr));
	    first = 0;
	} while (tok != ENDINPUT && tok != LEXERR);
	noaliases = ona;
	strinend();
	inpop();
	errflag &= ~ERRFLAG_ERROR;
	zcontext_restore();
	/* Fine, now do full expansion. */
	prefork(foo, 0, NULL);
	if (!errflag) {
	    globlist(foo, 0);
	    if (!errflag)
		/* And add the resulting words as matches. */
		for (n = firstnode(foo); n; incnode(n))
		    addmatch(getdata(n), NULL);
	}
	opts[NULLGLOB] = ng;
	we = oowe;
	wb = oowb;
    }
    if (cc->hpat) {
	/* We have a pattern to take things from the history. */
	Patprog pprogc = NULL;
	char *e, *h, hpatsav;
	zlong i = addhistnum(curhist,-1,HIST_FOREIGN), n = cc->hnum;
	Histent he = gethistent(i, GETHIST_UPWARD);

	/* Parse the pattern, if it isn't the null string. */
	if (*(cc->hpat)) {
	    char *thpat = dupstring(cc->hpat);

	    tokenize(thpat);
	    pprogc = patcompile(thpat, 0, NULL);
	}
	/* n holds the number of history line we have to search. */
	if (!n)
	    n = -1;

	/* Now search the history. */
	while (n-- && he) {
	    int iwords;
	    for (iwords = he->nwords - 1; iwords >= 0; iwords--) {
		h = he->node.nam + he->words[iwords*2];
		e = he->node.nam + he->words[iwords*2+1];
		hpatsav = *e;
		*e = '\0';
		/* We now have a word from the history, ignore it *
		 * if it begins with a quote or `$'.              */
		if (*h != '\'' && *h != '"' && *h != '`' && *h != '$' &&
		    (!pprogc || pattry(pprogc, h)))
		    /* Otherwise add it if it was matched. */
		    addmatch(dupstring(h), NULL);
		if (hpatsav)
		    *e = hpatsav;
	    }
	    he = up_histent(he);
	}
	freepatprog(pprogc);
    }
    if ((t = cc->mask & (CC_ARRAYS | CC_INTVARS | CC_ENVVARS | CC_SCALARS |
			 CC_READONLYS | CC_SPECIALS | CC_PARAMS)))
	/* Add various flavours of parameters. */
	dumphashtable(paramtab, t);
    if ((t = cc->mask & CC_SHFUNCS))
	/* Add shell functions. */
	dumphashtable(shfunctab, t | (cc->mask & (CC_DISCMDS|CC_EXCMDS)));
    if ((t = cc->mask & CC_BUILTINS))
	/* Add builtins. */
	dumphashtable(builtintab, t | (cc->mask & (CC_DISCMDS|CC_EXCMDS)));
    if ((t = cc->mask & CC_EXTCMDS)) {
	/* Add external commands */
	if (isset(HASHLISTALL))
	    cmdnamtab->filltable(cmdnamtab);
	dumphashtable(cmdnamtab, t | (cc->mask & (CC_DISCMDS|CC_EXCMDS)));
    }
    if ((t = cc->mask & CC_RESWDS))
	/* Add reserved words */
	dumphashtable(reswdtab, t | (cc->mask & (CC_DISCMDS|CC_EXCMDS)));
    if ((t = cc->mask & (CC_ALREG | CC_ALGLOB)))
	/* Add the two types of aliases. */
	dumphashtable(aliastab, t | (cc->mask & (CC_DISCMDS|CC_EXCMDS)));
    if (keypm && cc == &cc_dummy) {
	/* Add the keys of the parameter in keypm. */
	HashTable t = keypm->gsu.h->getfn(keypm);

	if (t)
	    scanhashtable(t, 0, 0, PM_UNSET, addhnmatch, 0);
	keypm = NULL;
	cc_dummy.suffix = NULL;
    }
    if (!errflag && cc->ylist) {
	/* generate the user-defined display list: if anything fails, *
	 * we silently allow the normal completion list to be used.   */
	char **yaptr = NULL, *uv = NULL;
	Shfunc shfunc;

	if (cc->ylist[0] == '$' || cc->ylist[0] == '(') {
	    /* from variable */
	    uv = cc->ylist + (cc->ylist[0] == '$');
	} else if ((shfunc = getshfunc(cc->ylist))) {
	    /* from function:  pass completions as arg list */
	    LinkList args = newlinklist();
	    LinkNode ln;
	    Cmatch m;
	    int osc = sfcontext;

	    addlinknode(args, cc->ylist);
	    for (ln = firstnode(matches); ln; ln = nextnode(ln)) {
		m = (Cmatch) getdata(ln);
		if (m->ppre) {
		    char *s = (m->psuf ? m->psuf : "");
		    char *p = (char *) zhalloc(strlen(m->ppre) + strlen(m->str) +
					      strlen(s) + 1);

		    sprintf(p, "%s%s%s", m->ppre, m->str, s);
		    addlinknode(args, dupstring(p));
		} else
		    addlinknode(args, dupstring(m->str));
	    }

	    /* No harm in allowing read -l and -c here, too */
	    if (incompfunc != 1)
		incompctlfunc = 1;
	    sfcontext = SFC_COMPLETE;
	    doshfunc(shfunc, args, 1);
	    sfcontext = osc;
	    incompctlfunc = 0;
	    uv = "reply";
	}
	if (uv)
	    yaptr = get_user_var(uv);
	if ((tt = cc->explain)) {
	    tt = dupstring(tt);
	    if ((cc->mask & CC_EXPANDEXPL) && !parsestr(&tt)) {
		singsub(&tt);
		untokenize(tt);
	    }
	    curexpl->str = tt;
	    if (cc->gname) {
		endcmgroup(yaptr);
		begcmgroup(cc->gname, gflags);
		addexpl(0);
	    } else {
		addexpl(0);
		endcmgroup(yaptr);
		begcmgroup("default", 0);
	    }
	} else {
	    endcmgroup(yaptr);
	    begcmgroup("default", 0);
	}
    } else if ((tt = cc->explain)) {
	tt = dupstring(tt);
	if ((cc->mask & CC_EXPANDEXPL) && !parsestr(&tt)) {
	    singsub(&tt);
	    untokenize(tt);
	}
	curexpl->str = tt;
	addexpl(0);
    }
    if (cc->subcmd) {
	/* Handle -l sub-completion. */
	char **ow = clwords, *os = cmdstr, *ops = NULL;
	int oldn = clwnum, oldp = clwpos, br;
	unsigned long occ = ccont;
	
	ccont = CC_CCCONT;
	
	/* So we restrict the words-array. */
	if (brange >= clwnum)
	    brange = clwnum - 1;
	if (brange < 1)
	    brange = 1;
	if (erange >= clwnum)
	    erange = clwnum - 1;
	if (erange < 1)
	    erange = 1;
	clwnum = erange - brange + 1;
	clwpos = clwpos - brange;
	br = brange;

	if (cc->subcmd[0]) {
	    /* And probably put the command name given to the flag *
	     * in the array.                                       */
	    clwpos++;
	    clwnum++;
	    incmd = 0;
	    ops = clwords[br - 1];
	    clwords[br - 1] = ztrdup(cc->subcmd);
	    cmdstr = ztrdup(cc->subcmd);
	    clwords += br - 1;
	} else {
	    cmdstr = ztrdup(clwords[br]);
	    incmd = !clwpos;
	    clwords += br;
	}
	/* Produce the matches. */
	makecomplistcmd(s, incmd, CFN_FIRST);

	/* And restore the things we changed. */
	clwords = ow;
	zsfree(cmdstr);
	cmdstr = os;
	clwnum = oldn;
	clwpos = oldp;
	if (ops) {
	    zsfree(clwords[br - 1]);
	    clwords[br - 1] = ops;
	}
	ccont = occ;
    }
    if (cc->substr)
	sep_comp_string(cc->substr, s, offs);
    uremnode(ccstack, firstnode(ccstack));
    if (cc->matcher)
	mstack = mstack->next;

    if (mn == mnum)
	haspattern = ohp;
}


static struct builtin bintab[] = {
    BUILTIN("compcall", 0, bin_compcall, 0, 0, 0, "TD", NULL),
    BUILTIN("compctl", 0, bin_compctl, 0, -1, 0, NULL, NULL),
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
    compctlreadptr = compctlread;
    createcompctltable();
    cc_compos.mask = CC_COMMPATH;
    cc_compos.mask2 = 0;
    cc_default.refc = 10000;
    cc_default.mask = CC_FILES;
    cc_default.mask2 = 0;
    cc_first.refc = 10000;
    cc_first.mask = 0;
    cc_first.mask2 = CC_CCCONT;

    lastccused = NULL;

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
    addhookfunc("compctl_make", (Hookfn) ccmakehookfn);
    addhookfunc("compctl_cleanup", (Hookfn) cccleanuphookfn);
    return 0;
}

/**/
int
cleanup_(Module m)
{
    deletehookfunc("compctl_make", (Hookfn) ccmakehookfn);
    deletehookfunc("compctl_cleanup", (Hookfn) cccleanuphookfn);
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    deletehashtable(compctltab);

    if (lastccused)
	freelinklist(lastccused, (FreeFunc) freecompctl);

    compctlreadptr = fallback_compctlread;
    return 0;
}
