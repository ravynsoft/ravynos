/*
 * loop.c - loop execution
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
#include "loop.pro"

/* # of nested loops we are in */
 
/**/
int loops;
 
/* # of continue levels */
 
/**/
mod_export int contflag;
 
/* # of break levels */
 
/**/
mod_export volatile int breaks;

/**/
int
execfor(Estate state, int do_exec)
{
    Wordcode end, loop;
    wordcode code = state->pc[-1];
    int iscond = (WC_FOR_TYPE(code) == WC_FOR_COND), ctok = 0, atok = 0;
    int last = 0;
    char *name, *str, *cond = NULL, *advance = NULL;
    zlong val = 0;
    LinkList vars = NULL, args = NULL;
    int old_simple_pline = simple_pline;

    /* See comments in execwhile() */
    simple_pline = 1;

    end = state->pc + WC_FOR_SKIP(code);

    if (iscond) {
	str = dupstring(ecgetstr(state, EC_NODUP, NULL));
	singsub(&str);
	if (isset(XTRACE)) {
	    char *str2 = dupstring(str);
	    untokenize(str2);
	    printprompt4();
	    fprintf(xtrerr, "%s\n", str2);
	    fflush(xtrerr);
	}
	if (!errflag) {
	    matheval(str);
	}
	if (errflag) {
	    state->pc = end;
	    simple_pline = old_simple_pline;
	    return 1;
	}
	cond = ecgetstr(state, EC_NODUP, &ctok);
	advance = ecgetstr(state, EC_NODUP, &atok);
    } else {
	vars = ecgetlist(state, *state->pc++, EC_NODUP, NULL);

	if (WC_FOR_TYPE(code) == WC_FOR_LIST) {
	    int htok = 0;

	    if (!(args = ecgetlist(state, *state->pc++, EC_DUPTOK, &htok))) {
		state->pc = end;
		simple_pline = old_simple_pline;
		return 0;
	    }
	    if (htok) {
		execsubst(args);
		if (errflag) {
		    state->pc = end;
		    simple_pline = old_simple_pline;
		    return 1;
		}
	    }
	} else {
	    char **x;

	    args = newlinklist();
	    for (x = pparams; *x; x++)
		addlinknode(args, dupstring(*x));
	}
    }

    if (!args || empty(args))
	lastval = 0;

    loops++;
    pushheap();
    cmdpush(CS_FOR);
    loop = state->pc;
    while (!last) {
	if (iscond) {
	    if (ctok) {
		str = dupstring(cond);
		singsub(&str);
	    } else
		str = cond;
	    if (!errflag) {
		while (iblank(*str))
		    str++;
		if (*str) {
		    if (isset(XTRACE)) {
			printprompt4();
			fprintf(xtrerr, "%s\n", str);
			fflush(xtrerr);
		    }
		    val = mathevali(str);
		} else
		    val = 1;
	    }
	    if (errflag) {
		if (breaks)
		    breaks--;
		lastval = 1;
		break;
	    }
	    if (!val)
		break;
	} else {
	    LinkNode node;
	    int count = 0;
	    for (node = firstnode(vars); node; incnode(node))
	    {
		name = (char *)getdata(node);
		if (!args || !(str = (char *) ugetnode(args)))
		{
		    if (count) { 
			str = "";
			last = 1;
		    } else
			break;
		}
		if (isset(XTRACE)) {
		    printprompt4();
		    fprintf(xtrerr, "%s=%s\n", name, str);
		    fflush(xtrerr);
		}
		setsparam(name, ztrdup(str));
		count++;
	    }
	    if (!count)
		break;
	}
	state->pc = loop;
	execlist(state, 1, do_exec && args && empty(args));
	if (breaks) {
	    breaks--;
	    if (breaks || !contflag)
		break;
	    contflag = 0;
	}
	if (retflag)
	    break;
	if (iscond && !errflag) {
	    if (atok) {
		str = dupstring(advance);
		singsub(&str);
	    } else
		str = advance;
	    if (isset(XTRACE)) {
		printprompt4();
		fprintf(xtrerr, "%s\n", str);
		fflush(xtrerr);
	    }
	    if (!errflag)
		matheval(str);
	}
	if (errflag) {
	    if (breaks)
		breaks--;
	    lastval = 1;
	    break;
	}
	freeheap();
    }
    popheap();
    cmdpop();
    loops--;
    simple_pline = old_simple_pline;
    state->pc = end;
    this_noerrexit = 1;
    return lastval;
}

/**/
int
execselect(Estate state, UNUSED(int do_exec))
{
    Wordcode end, loop;
    wordcode code = state->pc[-1];
    char *str, *s, *name;
    LinkNode n;
    int i, usezle;
    FILE *inp;
    size_t more;
    LinkList args;
    int old_simple_pline = simple_pline;

    /* See comments in execwhile() */
    simple_pline = 1;

    end = state->pc + WC_FOR_SKIP(code);
    name = ecgetstr(state, EC_NODUP, NULL);

    if (WC_SELECT_TYPE(code) == WC_SELECT_PPARAM) {
	char **x;

	args = newlinklist();
	for (x = pparams; *x; x++)
	    addlinknode(args, dupstring(*x));
    } else {
	int htok = 0;

	if (!(args = ecgetlist(state, *state->pc++, EC_DUPTOK, &htok))) {
	    state->pc = end;
	    simple_pline = old_simple_pline;
	    return 0;
	}
	if (htok) {
	    execsubst(args);
	    if (errflag) {
		state->pc = end;
		simple_pline = old_simple_pline;
		return 1;
	    }
	}
    }
    if (!args || empty(args)) {
	state->pc = end;
	simple_pline = old_simple_pline;
	return 0;
    }
    loops++;

    pushheap();
    cmdpush(CS_SELECT);
    usezle = interact && SHTTY != -1 && isset(USEZLE);
    inp = fdopen(dup(usezle ? SHTTY : 0), "r");
    more = selectlist(args, 0);
    loop = state->pc;
    for (;;) {
	for (;;) {
	    if (empty(bufstack)) {
	    	if (usezle) {
		    int oef = errflag;

		    isfirstln = 1;
		    str = zleentry(ZLE_CMD_READ, &prompt3, NULL,
				   0, ZLCON_SELECT);
		    if (errflag)
			str = NULL;
		    /* Keep any user interrupt error status */
		    errflag = oef | (errflag & ERRFLAG_INT);
	    	} else {
		    str = promptexpand(prompt3, 0, NULL, NULL, NULL);
		    zputs(str, stderr);
		    free(str);
		    fflush(stderr);
		    str = fgets(zhalloc(256), 256, inp);
	    	}
	    } else
		str = (char *)getlinknode(bufstack);
            if (!str && !errflag)
                setsparam("REPLY", ztrdup("")); /* EOF (user pressed Ctrl+D) */
	    if (!str || errflag) {
		if (breaks)
		    breaks--;
		fprintf(stderr, "\n");
		fflush(stderr);
		goto done;
	    }
	    if ((s = strchr(str, '\n')))
		*s = '\0';
	    if (*str)
	      break;
	    more = selectlist(args, more);
	}
	setsparam("REPLY", ztrdup(str));
	i = atoi(str);
	if (!i)
	    str = "";
	else {
	    for (i--, n = firstnode(args); n && i; incnode(n), i--);
	    if (n)
		str = (char *) getdata(n);
	    else
		str = "";
	}
	setsparam(name, ztrdup(str));
	state->pc = loop;
	execlist(state, 1, 0);
	freeheap();
	if (breaks) {
	    breaks--;
	    if (breaks || !contflag)
		break;
	    contflag = 0;
	}
	if (retflag || errflag)
	    break;
    }
  done:
    cmdpop();
    popheap();
    fclose(inp);
    loops--;
    simple_pline = old_simple_pline;
    state->pc = end;
    this_noerrexit = 1;
    return lastval;
}

/* And this is used to print select lists. */

/**/
size_t
selectlist(LinkList l, size_t start)
{
    size_t longest = 1, fct, fw = 0, colsz, t0, t1, ct;
    char **arr, **ap;

    zleentry(ZLE_CMD_TRASH);
    arr = hlinklist2array(l, 0);
    for (ap = arr; *ap; ap++)
	if (strlen(*ap) > longest)
	    longest = strlen(*ap);
    t0 = ct = ap - arr;
    longest++;
    while (t0)
	t0 /= 10, longest++;
    /* to compensate for added ')' */
    fct = (zterm_columns - 1) / (longest + 3);
    if (fct == 0)
	fct = 1;
    else
	fw = (zterm_columns - 1) / fct;
    colsz = (ct + fct - 1) / fct;
    for (t1 = start; t1 != colsz && t1 - start < zterm_lines - 2; t1++) {
	ap = arr + t1;
	do {
	    size_t t2 = strlen(*ap) + 2;
	    int t3;

	    fprintf(stderr, "%d) %s", t3 = ap - arr + 1, *ap);
	    while (t3)
		t2++, t3 /= 10;
	    for (; t2 < fw; t2++)
		fputc(' ', stderr);
	    for (t0 = colsz; t0 && *ap; t0--, ap++);
	}
	while (*ap);
	fputc('\n', stderr);
    }

 /* Below is a simple attempt at doing it the Korn Way..
       ap = arr;
       t0 = 0;
       do {
           t0++;
           fprintf(stderr,"%d) %s\n",t0,*ap);
           ap++;
       }
       while (*ap);*/
    fflush(stderr);

    return t1 < colsz ? t1 : 0;
}

/**/
int
execwhile(Estate state, UNUSED(int do_exec))
{
    Wordcode end, loop;
    wordcode code = state->pc[-1];
    int olderrexit, oldval, isuntil = (WC_WHILE_TYPE(code) == WC_WHILE_UNTIL);
    int old_simple_pline = simple_pline;

    end = state->pc + WC_WHILE_SKIP(code);
    olderrexit = noerrexit;
    oldval = 0;
    pushheap();
    cmdpush(isuntil ? CS_UNTIL : CS_WHILE);
    loops++;
    loop = state->pc;

    if (loop[0] == WC_END && loop[1] == WC_END) {

        /* This is an empty loop.  Make sure the signal handler sets the
        * flags and then just wait for someone hitting ^C. */

        simple_pline = 1;

        while (!breaks)
            ;
        breaks--;

        simple_pline = old_simple_pline;
    } else {
        for (;;) {
            state->pc = loop;
            noerrexit = NOERREXIT_EXIT | NOERREXIT_RETURN;

	    /* In case the test condition is a functional no-op,
	     * make sure signal handlers recognize ^C to end the loop. */
	    simple_pline = 1;

            execlist(state, 1, 0);

	    simple_pline = old_simple_pline;
            noerrexit = olderrexit;
            if (!((lastval == 0) ^ isuntil)) {
                if (breaks)
                    breaks--;
		if (!retflag)
		    lastval = oldval;
                break;
            }
            if (retflag) {
		if (breaks)
		    breaks--;
                break;
	    }

	    /* In case the loop body is also a functional no-op,
	     * make sure signal handlers recognize ^C as above. */
	    simple_pline = 1;

            execlist(state, 1, 0);

	    simple_pline = old_simple_pline;
            if (breaks) {
                breaks--;
                if (breaks || !contflag)
                    break;
                contflag = 0;
            }
            if (errflag) {
                lastval = 1;
                break;
            }
            if (retflag)
                break;
            freeheap();
            oldval = lastval;
        }
    }
    cmdpop();
    popheap();
    loops--;
    state->pc = end;
    this_noerrexit = 1;
    return lastval;
}

/**/
int
execrepeat(Estate state, UNUSED(int do_exec))
{
    Wordcode end, loop;
    wordcode code = state->pc[-1];
    int count, htok = 0;
    char *tmp;
    int old_simple_pline = simple_pline;

    /* See comments in execwhile() */
    simple_pline = 1;

    end = state->pc + WC_REPEAT_SKIP(code);

    tmp = ecgetstr(state, EC_DUPTOK, &htok);
    if (htok) {
	singsub(&tmp);
	untokenize(tmp);
    }
    count = mathevali(tmp);
    if (errflag)
	return 1;
    lastval = 0; /* used when the repeat count is zero */
    pushheap();
    cmdpush(CS_REPEAT);
    loops++;
    loop = state->pc;
    while (count-- > 0) {
	state->pc = loop;
	execlist(state, 1, 0);
	freeheap();
	if (breaks) {
	    breaks--;
	    if (breaks || !contflag)
		break;
	    contflag = 0;
	}
	if (errflag) {
	    lastval = 1;
	    break;
	}
	if (retflag)
	    break;
    }
    cmdpop();
    popheap();
    loops--;
    simple_pline = old_simple_pline;
    state->pc = end;
    this_noerrexit = 1;
    return lastval;
}

/**/
int
execif(Estate state, int do_exec)
{
    Wordcode end, next;
    wordcode code = state->pc[-1];
    int olderrexit, s = 0, run = 0;

    olderrexit = noerrexit;
    end = state->pc + WC_IF_SKIP(code);

    noerrexit |= NOERREXIT_EXIT | NOERREXIT_RETURN;
    while (state->pc < end) {
	code = *state->pc++;
	if (wc_code(code) != WC_IF ||
	    (run = (WC_IF_TYPE(code) == WC_IF_ELSE))) {
	    if (run)
		run = 2;
	    break;
	}
	next = state->pc + WC_IF_SKIP(code);
	cmdpush(s ? CS_ELIF : CS_IF);
	execlist(state, 1, 0);
	cmdpop();
	if (!lastval) {
	    run = 1;
	    break;
	}
	if (retflag)
	    break;
	s = 1;
	state->pc = next;
    }

    if (run) {
	/* we need to ignore lastval until we reach execcmd() */
	if (olderrexit || run == 2)
	    noerrexit = olderrexit;
	else if (lastval)
	    noerrexit |= NOERREXIT_EXIT | NOERREXIT_RETURN | NOERREXIT_UNTIL_EXEC;
	else
	    noerrexit &= ~ (NOERREXIT_EXIT | NOERREXIT_RETURN);
	cmdpush(run == 2 ? CS_ELSE : (s ? CS_ELIFTHEN : CS_IFTHEN));
	execlist(state, 1, do_exec);
	cmdpop();
    } else {
	noerrexit = olderrexit;
	if (!retflag && !errflag)
	    lastval = 0;
    }
    state->pc = end;
    this_noerrexit = 1;

    return lastval;
}

/**/
int
execcase(Estate state, int do_exec)
{
    Wordcode end, next;
    wordcode code = state->pc[-1];
    char *word, *pat;
    int npat, save, nalts, ialt, patok, anypatok;
    Patprog *spprog, pprog;

    end = state->pc + WC_CASE_SKIP(code);

    word = ecgetstr(state, EC_DUP, NULL);
    singsub(&word);
    untokenize(word);
    anypatok = 0;

    cmdpush(CS_CASE);
    while (state->pc < end) {
	code = *state->pc++;
	if (wc_code(code) != WC_CASE)
	    break;

	save = 0;
	next = state->pc + WC_CASE_SKIP(code);
	nalts = *state->pc++;
	ialt = patok = 0;

	if (isset(XTRACE)) {
	    printprompt4();
	    fprintf(xtrerr, "case %s (", word);
	}

	while (!patok && nalts) {
	    npat = state->pc[1];
	    spprog = state->prog->pats + npat;
	    pprog = NULL;
	    pat = NULL;

	    queue_signals();

	    if (isset(XTRACE)) {
		int htok = 0;
		pat = dupstring(ecrawstr(state->prog, state->pc, &htok));
		if (htok)
		    singsub(&pat);

		if (ialt++)
		    fprintf(stderr, " | ");
		quote_tokenized_output(pat, xtrerr);
	    }

	    if (*spprog != dummy_patprog1 && *spprog != dummy_patprog2)
		pprog = *spprog;

	    if (!pprog) {
		if (!pat) {
		    char *opat;
		    int htok = 0;

		    pat = dupstring(opat = ecrawstr(state->prog,
						    state->pc, &htok));
		    if (htok)
			singsub(&pat);
		    save = (!(state->prog->flags & EF_HEAP) &&
			    !strcmp(pat, opat) && *spprog != dummy_patprog2);
		}
		if (!(pprog = patcompile(pat, (save ? PAT_ZDUP : PAT_STATIC),
					 NULL)))
		    zerr("bad pattern: %s", pat);
		else if (save)
		    *spprog = pprog;
	    }
	    if (pprog && pattry(pprog, word))
		patok = anypatok = 1;
	    state->pc += 2;
	    nalts--;

	    unqueue_signals();
	}
	state->pc += 2 * nalts;
	if (isset(XTRACE)) {
	    fprintf(xtrerr, ")\n");
	    fflush(xtrerr);
	}
	if (patok) {
	    execlist(state, 1, ((WC_CASE_TYPE(code) == WC_CASE_OR) &&
				do_exec));
	    while (!retflag && wc_code(code) == WC_CASE &&
		   WC_CASE_TYPE(code) == WC_CASE_AND && state->pc < end) {
		state->pc = next;
		code = *state->pc++;
		next = state->pc + WC_CASE_SKIP(code);
		nalts = *state->pc++;
		state->pc += 2 * nalts;
		execlist(state, 1, ((WC_CASE_TYPE(code) == WC_CASE_OR) &&
				    do_exec));
	    }
	    if (WC_CASE_TYPE(code) != WC_CASE_TESTAND)
		break;
	}
	state->pc = next;
    }
    cmdpop();

    state->pc = end;

    if (!anypatok)
	lastval = 0;
    this_noerrexit = 1;

    return lastval;
}

/*
 * Errflag from `try' block, may be reset in `always' block.
 * Accessible from an integer parameter, so needs to be a zlong.
 */

/**/
zlong
try_errflag = -1;

/**
 * Corresponding interrupt error status form `try' block.
 */

/**/
zlong
try_interrupt = -1;

/**/
zlong
try_tryflag = 0;

/**/
int
exectry(Estate state, int do_exec)
{
    Wordcode end, always;
    int endval;
    int save_retflag, save_breaks, save_contflag;
    zlong save_try_errflag, save_try_interrupt;

    end = state->pc + WC_TRY_SKIP(state->pc[-1]);
    always = state->pc + 1 + WC_TRY_SKIP(*state->pc);
    state->pc++;
    pushheap();
    cmdpush(CS_CURSH);

    /* The :try clause */
    ++try_tryflag;
    execlist(state, 1, 0);
    --try_tryflag;

    /* Don't record errflag here, may be reset.  However, */
    /* endval should show failure when there is an error. */
    endval = lastval ? lastval : errflag;

    freeheap();

    cmdpop();
    cmdpush(CS_ALWAYS);

    /* The always clause. */
    save_try_errflag = try_errflag;
    save_try_interrupt = try_interrupt;
    try_errflag = (zlong)(errflag & ERRFLAG_ERROR);
    try_interrupt = (zlong)((errflag & ERRFLAG_INT) ? 1 : 0);
    /* We need to reset all errors to allow the block to execute */
    errflag = 0;
    save_retflag = retflag;
    retflag = 0;
    save_breaks = breaks;
    breaks = 0;
    save_contflag = contflag;
    contflag = 0;

    state->pc = always;
    execlist(state, 1, do_exec);

    if (try_errflag)
	errflag |= ERRFLAG_ERROR;
    else
	errflag &= ~ERRFLAG_ERROR;
    if (try_interrupt)
	errflag |= ERRFLAG_INT;
    else
	errflag &= ~ERRFLAG_INT;
    try_errflag = save_try_errflag;
    try_interrupt = save_try_interrupt;
    if (!retflag)
	retflag = save_retflag;
    if (!breaks)
	breaks = save_breaks;
    if (!contflag)
	contflag = save_contflag;

    cmdpop();
    popheap();
    state->pc = end;

    return endval;
}
