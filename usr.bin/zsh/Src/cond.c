/*
 * cond.c - evaluate conditional expressions
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

#include "zsh.mdh"
#include "cond.pro"

/**/
int tracingcond;    /* updated by execcond() in exec.c */

static char *condstr[COND_MOD] = {
    "!", "&&", "||", "=", "==", "!=", "<", ">", "-nt", "-ot", "-ef", "-eq",
    "-ne", "-lt", "-gt", "-le", "-ge", "=~"
};

static void cond_subst(char **strp, int glob_ok)
{
    if (glob_ok &&
	checkglobqual(*strp, strlen(*strp), 1, NULL)) {
	LinkList args = newlinklist();
	addlinknode(args, *strp);
	prefork(args, 0, NULL);
	while (!errflag && args && nonempty(args) &&
	       has_token((char *)peekfirst(args)))
	    zglob(args, firstnode(args), 0);
	*strp = sepjoin(hlinklist2array(args, 0), NULL, 1);
    } else
	singsub(strp);
}

/*
 * Evaluate a conditional expression given the arguments.
 * If fromtest is set, the caller is the test or [ builtin;
 * with the pointer giving the name of the command.
 * for POSIX conformance this supports a more limited range
 * of functionality.
 *
 * Return status is the final shell status, i.e. 0 for true,
 * 1 for false, 2 for syntax error, 3 for "option in tested in
 * -o does not exist".
 */

/**/
int
evalcond(Estate state, char *fromtest)
{
    struct stat *st;
    char *left, *right, *overridename, overridebuf[13];
    Wordcode pcode;
    wordcode code;
    int ctype, htok = 0, ret;

 rec:

    left = right = overridename = NULL;
    pcode = state->pc++;
    code = *pcode;
    ctype = WC_COND_TYPE(code);

    switch (ctype) {
    case COND_NOT:
	if (tracingcond)
	    fprintf(xtrerr, " %s", condstr[ctype]);
	ret = evalcond(state, fromtest);
	if (ret == 0 || ret == 1)
	    return !ret;
	else
	    return ret;
    case COND_AND:
	if (!(ret = evalcond(state, fromtest))) {
	    if (tracingcond)
		fprintf(xtrerr, " %s", condstr[ctype]);
	    goto rec;
	} else {
	    state->pc = pcode + (WC_COND_SKIP(code) + 1);
	    return ret;
	}
    case COND_OR:
	ret = evalcond(state, fromtest);
	if (ret == 1 || ret == 3) {
	    if (tracingcond)
		fprintf(xtrerr, " %s", condstr[ctype]);
	    goto rec;
	} else {
	    state->pc = pcode + (WC_COND_SKIP(code) + 1);
	    return ret;
	}
    case COND_REGEX:
	{
	    char *modname = isset(REMATCHPCRE) ? "zsh/pcre" : "zsh/regex";
	    sprintf(overridename = overridebuf, "-%s-match", modname+4);
	    (void)ensurefeature(modname, "C:", overridename+1);
	    ctype = COND_MODI;
	}
	/*FALLTHROUGH*/
    case COND_MOD:
    case COND_MODI:
	{
	    Conddef cd;
	    char *name = overridename, *errname;
	    char **strs;
	    int l = WC_COND_SKIP(code);

	    if (name == NULL)
		name = ecgetstr(state, EC_NODUP, NULL);
	    if (ctype == COND_MOD)
		strs = ecgetarr(state, l, EC_DUP, NULL);
	    else {
		char *sbuf[3];

		sbuf[0] = ecgetstr(state, EC_NODUP, NULL);
		sbuf[1] = ecgetstr(state, EC_NODUP, NULL);
		sbuf[2] = NULL;

		strs = arrdup(sbuf);
		l = 2;
	    }
	    if (name && IS_DASH(name[0]))
		untokenize(errname = dupstring(name));
	    else if (strs[0] && IS_DASH(*strs[0]))
		untokenize(errname = strs[0]);
	    else
		errname = "<null>";
	    if (name && IS_DASH(name[0]) &&
		(cd = getconddef((ctype == COND_MODI), name + 1, 1))) {
		if (ctype == COND_MOD &&
		    (l < cd->min || (cd->max >= 0 && l > cd->max))) {
		    zwarnnam(fromtest, "unknown condition: %s", name);
		    return 2;
		}
		if (tracingcond)
		    tracemodcond(name, strs, ctype == COND_MODI);
		return !cd->handler(strs, cd->condid);
	    }
	    else {
		char *s = strs[0];

		if (overridename) {
		    /*
		     * Standard regex function not available: this
		     * is a hard error.
		     */
		    zerrnam(fromtest, "%s not available for regex",
			     overridename);
		    return 2;
		}

		strs[0] = dupstring(name);
		name = s;

		if (name && IS_DASH(name[0]) &&
		    (cd = getconddef(0, name + 1, 1))) {
		    if (l < cd->min || (cd->max >= 0 && l > cd->max)) {
			zwarnnam(fromtest, "unknown condition: %s",
				 errname);
			return 2;
		    }
		    if (tracingcond)
			tracemodcond(name, strs, ctype == COND_MODI);
		    return !cd->handler(strs, cd->condid);
		} else {
		    zwarnnam(fromtest,
			     "unknown condition: %s",
			     errname);
		}
	    }
	    /* module not found, error */
	    return 2;
	}
    }
    left = ecgetstr(state, EC_DUPTOK, &htok);
    if (htok) {
	cond_subst(&left, !fromtest);
	untokenize(left);
    }
    if (ctype <= COND_GE && ctype != COND_STREQ && ctype != COND_STRDEQ &&
	ctype != COND_STRNEQ) {
	right = ecgetstr(state, EC_DUPTOK, &htok);
	if (htok) {
	    cond_subst(&right, !fromtest);
	    untokenize(right);
	}
    }
    if (tracingcond) {
	if (ctype < COND_MOD) {
	    fputc(' ',xtrerr);
	    quotedzputs(left, xtrerr);
	    fprintf(xtrerr, " %s ", condstr[ctype]);
	    if (ctype == COND_STREQ || ctype == COND_STRDEQ ||
		ctype == COND_STRNEQ) {
		char *rt = dupstring(ecrawstr(state->prog, state->pc, NULL));
		cond_subst(&rt, !fromtest);
		quote_tokenized_output(rt, xtrerr);
	    }
	    else
		quotedzputs((char *)right, xtrerr);
	} else {
	    fprintf(xtrerr, " -%c ", ctype);
	    quotedzputs(left, xtrerr);
	}
    }

    if (ctype >= COND_EQ && ctype <= COND_GE) {
	mnumber mn1, mn2;
	if (fromtest) {
	    /*
	     * For test and [, the expressions must be base 10 integers,
	     * not integer expressions.
	     */
	    char *eptr, *err;

	    mn1.u.l = zstrtol(left, &eptr, 10);
	    if (!*eptr)
	    {
		mn2.u.l = zstrtol(right, &eptr, 10);
		err = right;
	    }
	    else
		err = left;

	    if (*eptr)
	    {
		zwarnnam(fromtest, "integer expression expected: %s", err);
		return 2;
	    }

	    mn1.type = mn2.type = MN_INTEGER;
	} else {
	    mn1 = matheval(left);
	    mn2 = matheval(right);
	}

	if (((mn1.type|mn2.type) & (MN_INTEGER|MN_FLOAT)) ==
	    (MN_INTEGER|MN_FLOAT)) {
	    /* promote to float */
	    if (mn1.type & MN_INTEGER) {
		mn1.type = MN_FLOAT;
		mn1.u.d = (double)mn1.u.l;
	    }
	    if (mn2.type & MN_INTEGER) {
		mn2.type = MN_FLOAT;
		mn2.u.d = (double)mn2.u.l;
	    }
	}
	switch(ctype) {
	case COND_EQ:
	    return !((mn1.type & MN_FLOAT) ? (mn1.u.d == mn2.u.d) :
		     (mn1.u.l == mn2.u.l));
	case COND_NE:
	    return !((mn1.type & MN_FLOAT) ? (mn1.u.d != mn2.u.d) :
		     (mn1.u.l != mn2.u.l));
	case COND_LT:
	    return !((mn1.type & MN_FLOAT) ? (mn1.u.d < mn2.u.d) :
		     (mn1.u.l < mn2.u.l));
	case COND_GT:
	    return !((mn1.type & MN_FLOAT) ? (mn1.u.d > mn2.u.d) :
		     (mn1.u.l > mn2.u.l));
	case COND_LE:
	    return !((mn1.type & MN_FLOAT) ? (mn1.u.d <= mn2.u.d) :
		     (mn1.u.l <= mn2.u.l));
	case COND_GE:
	    return !((mn1.type & MN_FLOAT) ? (mn1.u.d >= mn2.u.d) :
		     (mn1.u.l >= mn2.u.l));
	}
    }

    switch (ctype) {
    case COND_STREQ:
    case COND_STRDEQ:
    case COND_STRNEQ:
	{
	    int test, npat = state->pc[1];
	    Patprog pprog = state->prog->pats[npat];

	    queue_signals();

	    if (pprog == dummy_patprog1 || pprog == dummy_patprog2) {
		char *opat;
		int save;

		right = dupstring(opat = ecrawstr(state->prog, state->pc,
						  &htok));
		singsub(&right);
		save = (!(state->prog->flags & EF_HEAP) &&
			!strcmp(opat, right) && pprog != dummy_patprog2);

		if (!(pprog = patcompile(right, (save ? PAT_ZDUP : PAT_STATIC),
					 NULL))) {
		    zwarnnam(fromtest, "bad pattern: %s", right);
		    unqueue_signals();
		    return 2;
		}
		else if (save)
		    state->prog->pats[npat] = pprog;
	    }
	    state->pc += 2;
	    test = (pprog && pattry(pprog, left));

	    unqueue_signals();

	    return !(ctype == COND_STRNEQ ? !test : test);
	}
    case COND_STRLT:
	return !(strcmp(left, right) < 0);
    case COND_STRGTR:
	return !(strcmp(left, right) > 0);
    case 'e':
    case 'a':
	return (!doaccess(left, F_OK));
    case 'b':
	return (!S_ISBLK(dostat(left)));
    case 'c':
	return (!S_ISCHR(dostat(left)));
    case 'd':
	return (!S_ISDIR(dostat(left)));
    case 'f':
	return (!S_ISREG(dostat(left)));
    case 'g':
	return (!(dostat(left) & S_ISGID));
    case 'k':
	return (!(dostat(left) & S_ISVTX));
    case 'n':
	return (!strlen(left));
    case 'o':
	return (optison(fromtest, left));
    case 'p':
	return (!S_ISFIFO(dostat(left)));
    case 'r':
	return (!doaccess(left, R_OK));
    case 's':
	return !((st = getstat(left)) && !!(st->st_size));
    case 'S':
	return (!S_ISSOCK(dostat(left)));
    case 'u':
	return (!(dostat(left) & S_ISUID));
    case 'v':
	return (!issetvar(left));
    case 'w':
	return (!doaccess(left, W_OK));
    case 'x':
	if (privasserted()) {
	    mode_t mode = dostat(left);
	    return !((mode & S_IXUGO) || S_ISDIR(mode));
	}
	return !doaccess(left, X_OK);
    case 'z':
	return !!(strlen(left));
    case 'h':
    case 'L':
	return (!S_ISLNK(dolstat(left)));
    case 'O':
	return !((st = getstat(left)) && st->st_uid == geteuid());
    case 'G':
	return !((st = getstat(left)) && st->st_gid == getegid());
    case 'N':
#if defined(GET_ST_MTIME_NSEC) && defined(GET_ST_ATIME_NSEC)
	if (!(st = getstat(left)))
	    return 1;
        return (st->st_atime == st->st_mtime) ?
        	GET_ST_ATIME_NSEC(*st) > GET_ST_MTIME_NSEC(*st) :
        	st->st_atime > st->st_mtime;
#else
	return !((st = getstat(left)) && st->st_atime <= st->st_mtime);
#endif
    case 't':
	return !isatty(mathevali(left));
    case COND_NT:
    case COND_OT:
	{
	    time_t a;
#ifdef GET_ST_MTIME_NSEC
	    long nsecs;
#endif

	    if (!(st = getstat(left)))
		return 1;
	    a = st->st_mtime;
#ifdef GET_ST_MTIME_NSEC
	    nsecs = GET_ST_MTIME_NSEC(*st);
#endif
	    if (!(st = getstat(right)))
		return 1;
#ifdef GET_ST_MTIME_NSEC
	    if (a == st->st_mtime) {
                return !((ctype == COND_NT) ? nsecs > GET_ST_MTIME_NSEC(*st) :
                        nsecs < GET_ST_MTIME_NSEC(*st));
	    }
#endif
	    return !((ctype == COND_NT) ? a > st->st_mtime : a < st->st_mtime);
	}
    case COND_EF:
	{
	    dev_t d;
	    ino_t i;

	    if (!(st = getstat(left)))
		return 1;
	    d = st->st_dev;
	    i = st->st_ino;
	    if (!(st = getstat(right)))
		return 1;
	    return !(d == st->st_dev && i == st->st_ino);
	}
    default:
	zwarnnam(fromtest, "bad cond code");
	return 2;
    }
}


/**/
static int
doaccess(char *s, int c)
{
#ifdef HAVE_FACCESSX
    if (!strncmp(s, "/dev/fd/", 8))
	return !faccessx(atoi(s + 8), c, ACC_SELF);
#endif
    return !access(unmeta(s), c);
}


static struct stat st;

/**/
static struct stat *
getstat(char *s)
{
    char *us;

/* /dev/fd/n refers to the open file descriptor n.  We always use fstat *
 * in this case since on Solaris /dev/fd/n is a device special file     */
    if (!strncmp(s, "/dev/fd/", 8)) {
	if (fstat(atoi(s + 8), &st))
	    return NULL;
        return &st;
    }

    if (!(us = unmeta(s)))
        return NULL;
    if (stat(us, &st))
	return NULL;
    return &st;
}


/**/
static mode_t
dostat(char *s)
{
    struct stat *statp;

    if (!(statp = getstat(s)))
	return 0;
    return statp->st_mode;
}


/* pem@aaii.oz; needed since dostat now uses "stat" */

/**/
static mode_t
dolstat(char *s)
{
    if (lstat(unmeta(s), &st) < 0)
	return 0;
    return st.st_mode;
}


/*
 * optison returns evalcond-friendly statuses (true, false, error).
 */

/**/
static int
optison(char *name, char *s)
{
    int i;

    if (strlen(s) == 1)
	i = optlookupc(*s);
    else
	i = optlookup(s);
    if (!i) {
	if (isset(POSIXBUILTINS))
	    return 1;
	else {
	    zwarnnam(name, "no such option: %s", s);
	    return 3;
	}
    } else if(i < 0)
	return !unset(-i);
    else
	return !isset(i);
}

/**/
mod_export char *
cond_str(char **args, int num, int raw)
{
    char *s = args[num];

    if (has_token(s)) {
	singsub(&s);
	if (!raw)
	    untokenize(s);
    }
    return s;
}

/**/
mod_export zlong
cond_val(char **args, int num)
{
    char *s = args[num];

    if (has_token(s)) {
	singsub(&s);
	untokenize(s);
    }
    return mathevali(s);
}

/**/
mod_export int
cond_match(char **args, int num, char *str)
{
    char *s = args[num];

    singsub(&s);

    return matchpat(str, s);
}

/**/
static void
tracemodcond(char *name, char **args, int inf)
{
    char **aptr;

    args = arrdup(args);
    for (aptr = args; *aptr; aptr++)
	untokenize(*aptr);
    if (inf) {
	fprintf(xtrerr, " %s %s %s", args[0], name, args[1]);
    } else {
	fprintf(xtrerr, " %s", name);
	while (*args)
	    fprintf(xtrerr, " %s", *args++);
    }
}
