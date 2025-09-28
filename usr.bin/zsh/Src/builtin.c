/*
 * builtin.c - builtin commands
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

/* this is defined so we get the prototype for open_memstream */
#define _GNU_SOURCE 1

#include "zsh.mdh"
#include "builtin.pro"

#include <math.h>

/* Builtins in the main executable */

static struct builtin builtins[] =
{
    BIN_PREFIX("-", BINF_DASH),
    BIN_PREFIX("builtin", BINF_BUILTIN),
    BIN_PREFIX("command", BINF_COMMAND),
    BIN_PREFIX("exec", BINF_EXEC),
    BIN_PREFIX("noglob", BINF_NOGLOB),
    BUILTIN("[", BINF_HANDLES_OPTS, bin_test, 0, -1, BIN_BRACKET, NULL, NULL),
    BUILTIN(".", BINF_PSPECIAL, bin_dot, 1, -1, 0, NULL, NULL),
    BUILTIN(":", BINF_PSPECIAL, bin_true, 0, -1, 0, NULL, NULL),
    BUILTIN("alias", BINF_MAGICEQUALS | BINF_PLUSOPTS, bin_alias, 0, -1, 0, "Lgmrs", NULL),
    BUILTIN("autoload", BINF_PLUSOPTS, bin_functions, 0, -1, 0, "dmktrRTUwWXz", "u"),
    BUILTIN("bg", 0, bin_fg, 0, -1, BIN_BG, NULL, NULL),
    BUILTIN("break", BINF_PSPECIAL, bin_break, 0, 1, BIN_BREAK, NULL, NULL),
    BUILTIN("bye", 0, bin_break, 0, 1, BIN_EXIT, NULL, NULL),
    BUILTIN("cd", BINF_SKIPINVALID | BINF_SKIPDASH | BINF_DASHDASHVALID, bin_cd, 0, 2, BIN_CD, "qsPL", NULL),
    BUILTIN("chdir", BINF_SKIPINVALID | BINF_SKIPDASH | BINF_DASHDASHVALID, bin_cd, 0, 2, BIN_CD, "qsPL", NULL),
    BUILTIN("continue", BINF_PSPECIAL, bin_break, 0, 1, BIN_CONTINUE, NULL, NULL),
    BUILTIN("declare", BINF_PLUSOPTS | BINF_MAGICEQUALS | BINF_PSPECIAL | BINF_ASSIGN, (HandlerFunc)bin_typeset, 0, -1, 0, "AE:%F:%HL:%R:%TUZ:%afghi:%klmp:%rtuxz", NULL),
    BUILTIN("dirs", 0, bin_dirs, 0, -1, 0, "clpv", NULL),
    BUILTIN("disable", 0, bin_enable, 0, -1, BIN_DISABLE, "afmprs", NULL),
    BUILTIN("disown", 0, bin_fg, 0, -1, BIN_DISOWN, NULL, NULL),
    BUILTIN("echo", BINF_SKIPINVALID, bin_print, 0, -1, BIN_ECHO, "neE", "-"),
    BUILTIN("emulate", 0, bin_emulate, 0, -1, 0, "lLR", NULL),
    BUILTIN("enable", 0, bin_enable, 0, -1, BIN_ENABLE, "afmprs", NULL),
    BUILTIN("eval", BINF_PSPECIAL, bin_eval, 0, -1, BIN_EVAL, NULL, NULL),
    BUILTIN("exit", BINF_PSPECIAL, bin_break, 0, 1, BIN_EXIT, NULL, NULL),
    BUILTIN("export", BINF_PLUSOPTS | BINF_MAGICEQUALS | BINF_PSPECIAL | BINF_ASSIGN, (HandlerFunc)bin_typeset, 0, -1, BIN_EXPORT, "E:%F:%HL:%R:%TUZ:%afhi:%lp:%rtu", "xg"),
    BUILTIN("false", 0, bin_false, 0, -1, 0, NULL, NULL),
    /*
     * We used to behave as if the argument to -e was optional.
     * But that's actually not useful, so it's more consistent to
     * cause an error.
     */
    BUILTIN("fc", 0, bin_fc, 0, -1, BIN_FC, "aAdDe:EfiIlLmnpPrRst:W", NULL),
    BUILTIN("fg", 0, bin_fg, 0, -1, BIN_FG, NULL, NULL),
    BUILTIN("float", BINF_PLUSOPTS | BINF_MAGICEQUALS | BINF_PSPECIAL | BINF_ASSIGN, (HandlerFunc)bin_typeset, 0, -1, 0, "E:%F:%HL:%R:%Z:%ghlp:%rtux", "E"),
    BUILTIN("functions", BINF_PLUSOPTS, bin_functions, 0, -1, 0, "ckmMstTuUWx:z", NULL),
    BUILTIN("getln", 0, bin_read, 0, -1, 0, "ecnAlE", "zr"),
    BUILTIN("getopts", 0, bin_getopts, 2, -1, 0, NULL, NULL),
    BUILTIN("hash", BINF_MAGICEQUALS, bin_hash, 0, -1, 0, "Ldfmrv", NULL),

#ifdef ZSH_HASH_DEBUG
    BUILTIN("hashinfo", 0, bin_hashinfo, 0, 0, 0, NULL, NULL),
#endif

    BUILTIN("history", 0, bin_fc, 0, -1, BIN_FC, "adDEfiLmnpPrt:", "l"),
    BUILTIN("integer", BINF_PLUSOPTS | BINF_MAGICEQUALS | BINF_PSPECIAL | BINF_ASSIGN, (HandlerFunc)bin_typeset, 0, -1, 0, "HL:%R:%Z:%ghi:%lp:%rtux", "i"),
    BUILTIN("jobs", 0, bin_fg, 0, -1, BIN_JOBS, "dlpZrs", NULL),
    BUILTIN("kill", BINF_HANDLES_OPTS, bin_kill, 0, -1, 0, NULL, NULL),
    BUILTIN("let", 0, bin_let, 1, -1, 0, NULL, NULL),
    BUILTIN("local", BINF_PLUSOPTS | BINF_MAGICEQUALS | BINF_PSPECIAL | BINF_ASSIGN, (HandlerFunc)bin_typeset, 0, -1, 0, "AE:%F:%HL:%R:%TUZ:%ahi:%lp:%rtux", NULL),
    BUILTIN("logout", 0, bin_break, 0, 1, BIN_LOGOUT, NULL, NULL),

#if defined(ZSH_MEM) & defined(ZSH_MEM_DEBUG)
    BUILTIN("mem", 0, bin_mem, 0, 0, 0, "v", NULL),
#endif

#if defined(ZSH_PAT_DEBUG)
    BUILTIN("patdebug", 0, bin_patdebug, 1, -1, 0, "p", NULL),
#endif

    BUILTIN("popd", BINF_SKIPINVALID | BINF_SKIPDASH | BINF_DASHDASHVALID, bin_cd, 0, 1, BIN_POPD, "q", NULL),
    BUILTIN("print", BINF_PRINTOPTS, bin_print, 0, -1, BIN_PRINT, "abcC:Df:ilmnNoOpPrRsSu:v:x:X:z-", NULL),
    BUILTIN("printf", BINF_SKIPINVALID | BINF_SKIPDASH, bin_print, 1, -1, BIN_PRINTF, "v:", NULL),
    BUILTIN("pushd", BINF_SKIPINVALID | BINF_SKIPDASH | BINF_DASHDASHVALID, bin_cd, 0, 2, BIN_PUSHD, "qsPL", NULL),
    BUILTIN("pushln", 0, bin_print, 0, -1, BIN_PRINT, NULL, "-nz"),
    BUILTIN("pwd", 0, bin_pwd, 0, 0, 0, "rLP", NULL),
    BUILTIN("r", 0, bin_fc, 0, -1, BIN_R, "IlLnr", NULL),
    BUILTIN("read", 0, bin_read, 0, -1, 0, "cd:ek:%lnpqrst:%zu:AE", NULL),
    BUILTIN("readonly", BINF_PLUSOPTS | BINF_MAGICEQUALS | BINF_PSPECIAL | BINF_ASSIGN, (HandlerFunc)bin_typeset, 0, -1, BIN_READONLY, "AE:%F:%HL:%R:%TUZ:%afghi:%lptux", "r"),
    BUILTIN("rehash", 0, bin_hash, 0, 0, 0, "df", "r"),
    BUILTIN("return", BINF_PSPECIAL, bin_break, 0, 1, BIN_RETURN, NULL, NULL),
    BUILTIN("set", BINF_PSPECIAL | BINF_HANDLES_OPTS, bin_set, 0, -1, 0, NULL, NULL),
    BUILTIN("setopt", 0, bin_setopt, 0, -1, BIN_SETOPT, NULL, NULL),
    BUILTIN("shift", BINF_PSPECIAL, bin_shift, 0, -1, 0, "p", NULL),
    BUILTIN("source", BINF_PSPECIAL, bin_dot, 1, -1, 0, NULL, NULL),
    BUILTIN("suspend", 0, bin_suspend, 0, 0, 0, "f", NULL),
    BUILTIN("test", BINF_HANDLES_OPTS, bin_test, 0, -1, BIN_TEST, NULL, NULL),
    BUILTIN("ttyctl", 0, bin_ttyctl, 0, 0, 0, "fu", NULL),
    BUILTIN("times", BINF_PSPECIAL, bin_times, 0, 0, 0, NULL, NULL),
    BUILTIN("trap", BINF_PSPECIAL | BINF_HANDLES_OPTS, bin_trap, 0, -1, 0, NULL, NULL),
    BUILTIN("true", 0, bin_true, 0, -1, 0, NULL, NULL),
    BUILTIN("type", 0, bin_whence, 0, -1, 0, "ampfsSw", "v"),
    BUILTIN("typeset", BINF_PLUSOPTS | BINF_MAGICEQUALS | BINF_PSPECIAL | BINF_ASSIGN, (HandlerFunc)bin_typeset, 0, -1, 0, "AE:%F:%HL:%R:%TUZ:%afghi:%klp:%rtuxmz", NULL),
    BUILTIN("umask", 0, bin_umask, 0, 1, 0, "S", NULL),
    BUILTIN("unalias", 0, bin_unhash, 0, -1, BIN_UNALIAS, "ams", NULL),
    BUILTIN("unfunction", 0, bin_unhash, 1, -1, BIN_UNFUNCTION, "m", "f"),
    BUILTIN("unhash", 0, bin_unhash, 1, -1, BIN_UNHASH, "adfms", NULL),
    BUILTIN("unset", BINF_PSPECIAL, bin_unset, 1, -1, BIN_UNSET, "fmv", NULL),
    BUILTIN("unsetopt", 0, bin_setopt, 0, -1, BIN_UNSETOPT, NULL, NULL),
    BUILTIN("wait", 0, bin_fg, 0, -1, BIN_WAIT, NULL, NULL),
    BUILTIN("whence", 0, bin_whence, 0, -1, 0, "acmpvfsSwx:", NULL),
    BUILTIN("where", 0, bin_whence, 0, -1, 0, "pmsSwx:", "ca"),
    BUILTIN("which", 0, bin_whence, 0, -1, 0, "ampsSwx:", "c"),
    BUILTIN("zmodload", 0, bin_zmodload, 0, -1, 0, "AFRILP:abcfdilmpsue", NULL),
    BUILTIN("zcompile", 0, bin_zcompile, 0, -1, 0, "tUMRcmzka", NULL),
};

/****************************************/
/* Builtin Command Hash Table Functions */
/****************************************/

/* hash table containing builtin commands */

/**/
mod_export HashTable builtintab;

/**/
void
createbuiltintable(void)
{
    builtintab = newhashtable(85, "builtintab", NULL);

    builtintab->hash        = hasher;
    builtintab->emptytable  = NULL;
    builtintab->filltable   = NULL;
    builtintab->cmpnodes    = strcmp;
    builtintab->addnode     = addhashnode;
    builtintab->getnode     = gethashnode;
    builtintab->getnode2    = gethashnode2;
    builtintab->removenode  = removehashnode;
    builtintab->disablenode = disablehashnode;
    builtintab->enablenode  = enablehashnode;
    builtintab->freenode    = freebuiltinnode;
    builtintab->printnode   = printbuiltinnode;

    (void)addbuiltins("zsh", builtins, sizeof(builtins)/sizeof(*builtins));
}

/* Print a builtin */

/**/
static void
printbuiltinnode(HashNode hn, int printflags)
{
    Builtin bn = (Builtin) hn;

    if (printflags & PRINT_WHENCE_WORD) {
	printf("%s: builtin\n", bn->node.nam);
	return;
    }

    if (printflags & PRINT_WHENCE_CSH) {
	printf("%s: shell built-in command\n", bn->node.nam);
	return;
    }

    if (printflags & PRINT_WHENCE_VERBOSE) {
	printf("%s is a shell builtin\n", bn->node.nam);
	return;
    }

    /* default is name only */
    printf("%s\n", bn->node.nam);
}

/**/
static void
freebuiltinnode(HashNode hn)
{
    Builtin bn = (Builtin) hn;

    if(!(bn->node.flags & BINF_ADDED)) {
	zsfree(bn->node.nam);
	zsfree(bn->optstr);
	zfree(bn, sizeof(struct builtin));
    }
}

/**/
void
init_builtins(void)
{
    if (!EMULATION(EMULATE_ZSH)) {
	HashNode hn = reswdtab->getnode2(reswdtab, "repeat");
	if (hn)
	    reswdtab->disablenode(hn, 0);
    }
}

/* Make sure we have space for a new option and increment. */

#define OPT_ALLOC_CHUNK 16

/**/
static int
new_optarg(Options ops)
{
    /* Argument index must be a non-zero 6-bit number. */
    if (ops->argscount == 63)
	return 1;
    if (ops->argsalloc == ops->argscount) {
	char **newptr =
	    (char **)zhalloc((ops->argsalloc + OPT_ALLOC_CHUNK) *
			     sizeof(char *));
	if (ops->argsalloc)
	    memcpy(newptr, ops->args, ops->argsalloc * sizeof(char *));
	ops->args = newptr;
	ops->argsalloc += OPT_ALLOC_CHUNK;
    }
    ops->argscount++;
    return 0;
}


/* execute a builtin handler function after parsing the arguments */

/**/
int
execbuiltin(LinkList args, LinkList assigns, Builtin bn)
{
    char *pp, *name, *optstr;
    int flags, argc, execop, xtr = isset(XTRACE);
    struct options ops;

    /* initialise options structure */
    memset(ops.ind, 0, MAX_OPS*sizeof(unsigned char));
    ops.args = NULL;
    ops.argscount = ops.argsalloc = 0;

    /* initialize some local variables */
    name = (char *) ugetnode(args);

    if (!bn->handlerfunc) {
	DPUTS(1, "Missing builtin detected too late");
	deletebuiltin(bn->node.nam);
	return 1;
    }
    /* get some information about the command */
    flags = bn->node.flags;
    optstr = bn->optstr;

    /* Set up the argument list. */
    /* count the arguments */
    argc = countlinknodes(args);

    {
	/*
	 * Keep all arguments, including options, in an array.
	 * We don't actually need the option part of the argument
	 * after option processing, but it makes XTRACE output
	 * much simpler.
	 */
	VARARR(char *, argarr, argc + 1);
	char **argv;

	/*
	 * Get the actual arguments, into argv.  Remember argarr
	 * may be an array declaration, depending on the compiler.
	 */
	argv = argarr;
	while ((*argv++ = (char *)ugetnode(args)));
	argv = argarr;

	/* Sort out the options. */
	if (optstr) {
	    char *arg = *argv;
	    int sense; /* 1 for -x, 0 for +x */
	    /* while arguments look like options ... */
	    while (arg &&
		   /* Must begin with - or maybe + */
		   ((sense = (*arg == '-')) ||
		    ((flags & BINF_PLUSOPTS) && *arg == '+'))) {
		/* Digits aren't arguments unless the command says they are. */
		if (!(flags & BINF_KEEPNUM) && idigit(arg[1]))
		    break;
		/* For cd and friends, a single dash is not an option. */
		if ((flags & BINF_SKIPDASH) && !arg[1])
		    break;
		if ((flags & BINF_DASHDASHVALID) && !strcmp(arg, "--")) {
		    /*
		     * Need to skip this before checking whether this is
		     * really an option.
		     */
		    argv++;
		    break;
		}
		/*
		 * Unrecognised options to echo etc. are not really
		 * options.
		 *
		 * Note this flag is not smart enough to handle option
		 * arguments.  In fact, ideally it shouldn't be added
		 * to any new builtins, to preserve standard option
		 * handling as much as possible.
		*/
		if (flags & BINF_SKIPINVALID) {
		    char *p = arg;
		    while (*++p && strchr(optstr, (int) *p));
		    if (*p)
			break;
		}
		/* handle -- or - (ops.ind['-']), and +
		 * (ops.ind['-'] and ops.ind['+']) */
		if (arg[1] == '-')
		    arg++;
		if (!arg[1]) {
		    ops.ind['-'] = 1;
		    if (!sense)
			ops.ind['+'] = 1;
		}
		/* save options in ops, as long as they are in bn->optstr */
		while (*++arg) {
		    char *optptr;
		    if ((optptr = strchr(optstr, execop = (int)*arg))) {
			ops.ind[(int)*arg] = (sense) ? 1 : 2;
			if (optptr[1] == ':') {
			    char *argptr = NULL;
			    if (optptr[2] == ':') {
				if (arg[1])
				    argptr = arg+1;
				/* Optional argument in same word*/
			    } else if (optptr[2] == '%') {
				/* Optional numeric argument in same
				 * or next word. */
				if (arg[1] && idigit(arg[1]))
				    argptr = arg+1;
				else if (argv[1] && idigit(*argv[1]))
				    argptr = arg = *++argv;
			    } else {
				/* Mandatory argument */
				if (arg[1])
				    argptr = arg+1;
				else if ((arg = *++argv))
				    argptr = arg;
				else {
				    zwarnnam(name, "argument expected: -%c",
					     execop);
				    return 1;
				}
			    }
			    if (argptr) {
				if (new_optarg(&ops)) {
				    zwarnnam(name,
					     "too many option arguments");
				    return 1;
				}
				ops.ind[execop] |= ops.argscount << 2;
				ops.args[ops.argscount-1] = argptr;
				while (arg[1])
				    arg++;
			    }
			}
		    } else
			break;
		}
		/* The above loop may have exited on an invalid option.  (We  *
		 * assume that any option requiring metafication is invalid.) */
		if (*arg) {
		    if(*arg == Meta)
			*++arg ^= 32;
		    zwarnnam(name, "bad option: %c%c", "+-"[sense], *arg);
		    return 1;
		}
		arg = *++argv;
		/* for the "print" builtin, the options after -R are treated as
		   options to "echo" */
		if ((flags & BINF_PRINTOPTS) && ops.ind['R'] &&
		    !ops.ind['f']) {
		    optstr = "ne";
		    flags |= BINF_SKIPINVALID;
		}
		/* the option -- indicates the end of the options */
		if (ops.ind['-'])
		    break;
	    }
	} else if (!(flags & BINF_HANDLES_OPTS) && *argv &&
		   !strcmp(*argv, "--")) {
	    ops.ind['-'] = 1;
	    argv++;
	}

	/* handle built-in options, for overloaded handler functions */
	if ((pp = bn->defopts)) {
	    while (*pp) {
		/* only if not already set */
		if (!ops.ind[(int)*pp])
		    ops.ind[(int)*pp] = 1;
		pp++;
	    }
	}

	/* Fix the argument count by subtracting option arguments */
	argc -= argv - argarr;

	if (errflag) {
	    errflag &= ~ERRFLAG_ERROR;
	    return 1;
	}

	/* check that the argument count lies within the specified bounds */
	if (argc < bn->minargs || (argc > bn->maxargs && bn->maxargs != -1)) {
	    zwarnnam(name, (argc < bn->minargs)
		     ? "not enough arguments" : "too many arguments");
	    return 1;
	}

	/* display execution trace information, if required */
	if (xtr) {
	    /* Use full argument list including options for trace output */
	    char **fullargv = argarr;
	    printprompt4();
	    fprintf(xtrerr, "%s", name);
	    while (*fullargv) {
	        fputc(' ', xtrerr);
	        quotedzputs(*fullargv++, xtrerr);
	    }
	    if (assigns) {
		LinkNode node;
		for (node = firstnode(assigns); node; incnode(node)) {
		    Asgment asg = (Asgment)node;
		    fputc(' ', xtrerr);
		    quotedzputs(asg->name, xtrerr);
		    if (asg->flags & ASG_ARRAY) {
			fprintf(xtrerr, "=(");
			if (asg->value.array) {
			    if (asg->flags & ASG_KEY_VALUE) {
				LinkNode keynode, valnode;
				keynode = firstnode(asg->value.array);
				for (;;) {
				    if (!keynode)
					break;
				    valnode = nextnode(keynode);
				    if (!valnode)
					break;
				    fputc('[', xtrerr);
				    quotedzputs((char *)getdata(keynode),
						xtrerr);
				    fprintf(stderr, "]=");
				    quotedzputs((char *)getdata(valnode),
						xtrerr);
				    keynode = nextnode(valnode);
				}
			    } else {
				LinkNode arrnode;
				for (arrnode = firstnode(asg->value.array);
				     arrnode;
				     incnode(arrnode)) {
				    fputc(' ', xtrerr);
				    quotedzputs((char *)getdata(arrnode),
						xtrerr);
				}
			    }
			}
			fprintf(xtrerr, " )");
		    } else if (asg->value.scalar) {
			fputc('=', xtrerr);
			quotedzputs(asg->value.scalar, xtrerr);
		    }
		}
	    }
	    fputc('\n', xtrerr);
	    fflush(xtrerr);
	}
	/* call the handler function, and return its return value */
	if (flags & BINF_ASSIGN)
	{
	    /*
	     * Takes two sets of arguments.
	     */
	    HandlerFuncAssign assignfunc = (HandlerFuncAssign)bn->handlerfunc;
	    return (*(assignfunc)) (name, argv, assigns, &ops, bn->funcid);
	}
	else
	{
	    return (*(bn->handlerfunc)) (name, argv, &ops, bn->funcid);
	}
    }
}

/* Enable/disable an element in one of the internal hash tables.  *
 * With no arguments, it lists all the currently enabled/disabled *
 * elements in that particular hash table.                        */

/**/
int
bin_enable(char *name, char **argv, Options ops, int func)
{
    HashTable ht;
    HashNode hn;
    ScanFunc scanfunc;
    Patprog pprog;
    int flags1 = 0, flags2 = 0;
    int match = 0, returnval = 0;

    /* Find out which hash table we are working with. */
    if (OPT_ISSET(ops,'p')) {
	return pat_enables(name, argv, func == BIN_ENABLE);
    } else if (OPT_ISSET(ops,'f'))
	ht = shfunctab;
    else if (OPT_ISSET(ops,'r'))
	ht = reswdtab;
    else if (OPT_ISSET(ops,'s'))
	ht = sufaliastab;
    else if (OPT_ISSET(ops,'a'))
	ht = aliastab;
    else
	ht = builtintab;

    /* Do we want to enable or disable? */
    if (func == BIN_ENABLE) {
	flags2 = DISABLED;
	scanfunc = ht->enablenode;
    } else {
	flags1 = DISABLED;
	scanfunc = ht->disablenode;
    }

    /* Given no arguments, print the names of the enabled/disabled elements  *
     * in this hash table.  If func == BIN_ENABLE, then scanhashtable will   *
     * print nodes NOT containing the DISABLED flag, else scanhashtable will *
     * print nodes containing the DISABLED flag.                             */
    if (!*argv) {
	queue_signals();
	scanhashtable(ht, 1, flags1, flags2, ht->printnode, 0);
	unqueue_signals();
	return 0;
    }

    /* With -m option, treat arguments as glob patterns. */
    if (OPT_ISSET(ops,'m')) {
	for (; *argv; argv++) {
	    queue_signals();

	    /* parse pattern */
	    tokenize(*argv);
	    if ((pprog = patcompile(*argv, PAT_STATIC, 0)))
		match += scanmatchtable(ht, pprog, 0, 0, 0, scanfunc, 0);
	    else {
		untokenize(*argv);
		zwarnnam(name, "bad pattern : %s", *argv);
		returnval = 1;
	    }
	    unqueue_signals();
	}
	/* If we didn't match anything, we return 1. */
	if (!match)
	    returnval = 1;
	return returnval;
    }

    /* Take arguments literally -- do not glob */
    queue_signals();
    for (; *argv; argv++) {
	if ((hn = ht->getnode2(ht, *argv))) {
	    scanfunc(hn, 0);
	} else {
	    zwarnnam(name, "no such hash table element: %s", *argv);
	    returnval = 1;
	}
    }
    unqueue_signals();
    return returnval;
}

/* set: either set the shell options, or set the shell arguments, *
 * or declare an array, or show various things                    */

/**/
int
bin_set(char *nam, char **args, UNUSED(Options ops), UNUSED(int func))
{
    int action, optno, array = 0, hadopt = 0,
	hadplus = 0, hadend = 0, sort = 0;
    char **x, *arrayname = NULL;

    /* Obsolescent sh compatibility: set - is the same as set +xv *
     * and set - args is the same as set +xv -- args              */
    if (!EMULATION(EMULATE_ZSH) && *args && **args == '-' && !args[0][1]) {
	dosetopt(VERBOSE, 0, 0, opts);
	dosetopt(XTRACE, 0, 0, opts);
	if (!args[1])
	    return 0;
    }

    /* loop through command line options (begins with "-" or "+") */
    while (*args && (**args == '-' || **args == '+')) {
	action = (**args == '-');
	hadplus |= !action;
	if(!args[0][1])
	    *args = "--";
	while (*++*args) {
	    if(**args == Meta)
		*++*args ^= 32;
	    if(**args != '-' || action)
		hadopt = 1;
	    /* The pseudo-option `--' signifies the end of options. */
	    if (**args == '-') {
		hadend = 1;
		args++;
		goto doneoptions;
	    } else if (**args == 'o') {
		if (!*++*args)
		    args++;
		if (!*args) {
		    printoptionstates(hadplus);
		    inittyptab();
		    return 0;
		}
		if(!(optno = optlookup(*args)))
		    zerrnam(nam, "no such option: %s", *args);
		else if(dosetopt(optno, action, 0, opts))
		    zerrnam(nam, "can't change option: %s", *args);
		break;
	    } else if(**args == 'A') {
		if(!*++*args)
		    args++;
		array = action ? 1 : -1;
		arrayname = *args;
		if (!arrayname)
		    goto doneoptions;
		else if  (!isset(KSHARRAYS))
		{
		    args++;
		    goto doneoptions;
		}
		break;
	    } else if (**args == 's')
		sort = action ? 1 : -1;
	    else {
	    	if (!(optno = optlookupc(**args)))
		    zerrnam(nam, "bad option: -%c", **args);
		else if(dosetopt(optno, action, 0, opts))
		    zerrnam(nam, "can't change option: -%c", **args);
	    }
	}
	args++;
    }
    if (errflag)
	return 1;
 doneoptions:
    inittyptab();

    /* Show the parameters, possibly with values */
    queue_signals();
    if (!arrayname)
    {
	if (!hadopt && !*args)
	    scanhashtable(paramtab, 1, 0, 0, paramtab->printnode,
			  hadplus ? PRINT_NAMEONLY : 0);

	if (array) {
	    /* display arrays */
	    scanhashtable(paramtab, 1, PM_ARRAY, 0, paramtab->printnode,
			  hadplus ? PRINT_NAMEONLY : 0);
	}
	if (!*args && !hadend) {
	    unqueue_signals();
	    return 0;
	}
    }
    if (sort)
	strmetasort(args, sort < 0 ? SORTIT_BACKWARDS : 0, NULL);
    if (array) {
	/* create an array with the specified elements */
	char **a = NULL, **y;
	int len = arrlen(args);

	if (array < 0 && (a = getaparam(arrayname)) && arrlen_gt(a, len)) {
	    a += len;
	    len += arrlen(a);
	}
	for (x = y = zalloc((len + 1) * sizeof(char *)); len--;) {
	    if (!*args)
		args = a;
	    *y++ = ztrdup(*args++);
	}
	*y++ = NULL;
	setaparam(arrayname, x);
    } else {
	/* set shell arguments */
	freearray(pparams);
	pparams = zarrdup(args);
    }
    unqueue_signals();
    return 0;
}

/**** directory-handling builtins ****/

/**/
int doprintdir = 0;		/* set in exec.c (for autocd, cdpath, etc.) */

/* pwd: display the name of the current directory */

/**/
int
bin_pwd(UNUSED(char *name), UNUSED(char **argv), Options ops, UNUSED(int func))
{
    if (OPT_ISSET(ops,'r') || OPT_ISSET(ops,'P') ||
	(isset(CHASELINKS) && !OPT_ISSET(ops,'L')))
	printf("%s\n", zgetcwd());
    else {
	zputs(pwd, stdout);
	putchar('\n');
    }
    return 0;
}

/* the directory stack */

/**/
mod_export LinkList dirstack;

/* dirs: list the directory stack, or replace it with a provided list */

/**/
int
bin_dirs(UNUSED(char *name), char **argv, Options ops, UNUSED(int func))
{
    LinkList l;

    queue_signals();
    /* with -v, -p or no arguments display the directory stack */
    if (!(*argv || OPT_ISSET(ops,'c')) || OPT_ISSET(ops,'v') ||
	OPT_ISSET(ops,'p')) {
	LinkNode node;
	char *fmt;
	int pos = 1;

	/* with the -v option, display a numbered list, starting at zero */
	if (OPT_ISSET(ops,'v')) {
	    printf("0\t");
	    fmt = "\n%d\t";
	/* with the -p option, display entries one per line */
	} else if (OPT_ISSET(ops,'p'))
	    fmt = "\n";
	else
	    fmt = " ";
	if (OPT_ISSET(ops,'l'))
	    zputs(pwd, stdout);
	else
	    fprintdir(pwd, stdout);
	for (node = firstnode(dirstack); node; incnode(node)) {
	    printf(fmt, pos++);
	    if (OPT_ISSET(ops,'l'))
		zputs(getdata(node), stdout);
	    else
		fprintdir(getdata(node), stdout);

	}
	unqueue_signals();
	putchar('\n');
	return 0;
    }
    /* replace the stack with the specified directories */
    l = znewlinklist();
    while (*argv)
	zaddlinknode(l, ztrdup(*argv++));
    freelinklist(dirstack, freestr);
    dirstack = l;
    unqueue_signals();
    return 0;
}

/* cd, chdir, pushd, popd */

/**/
void
set_pwd_env(void)
{
    Param pm;

    /* update the PWD and OLDPWD shell parameters */

    pm = (Param) paramtab->getnode(paramtab, "PWD");
    if (pm && PM_TYPE(pm->node.flags) != PM_SCALAR) {
	pm->node.flags &= ~PM_READONLY;
	unsetparam_pm(pm, 0, 1);
    }

    pm = (Param) paramtab->getnode(paramtab, "OLDPWD");
    if (pm && PM_TYPE(pm->node.flags) != PM_SCALAR) {
	pm->node.flags &= ~PM_READONLY;
	unsetparam_pm(pm, 0, 1);
    }

    assignsparam("PWD", ztrdup(pwd), 0);
    assignsparam("OLDPWD", ztrdup(oldpwd), 0);

    pm = (Param) paramtab->getnode(paramtab, "PWD");
    if (!(pm->node.flags & PM_EXPORTED))
	addenv(pm, pwd);
    pm = (Param) paramtab->getnode(paramtab, "OLDPWD");
    if (!(pm->node.flags & PM_EXPORTED))
	addenv(pm, oldpwd);
}

/* set if we are resolving links to their true paths */
static int chasinglinks;

/* The main pwd changing function.  The real work is done by other     *
 * functions.  cd_get_dest() does the initial argument processing;     *
 * cd_do_chdir() actually changes directory, if possible; cd_new_pwd() *
 * does the ancillary processing associated with actually changing    *
 * directory.                                                          */

/**/
int
bin_cd(char *nam, char **argv, Options ops, int func)
{
    LinkNode dir;

    if (isset(RESTRICTED)) {
	zwarnnam(nam, "restricted");
	return 1;
    }
    doprintdir = (doprintdir == -1);

    chasinglinks = OPT_ISSET(ops,'P') ||
	(isset(CHASELINKS) && !OPT_ISSET(ops,'L'));
    queue_signals();
    zpushnode(dirstack, ztrdup(pwd));
    if (!(dir = cd_get_dest(nam, argv, OPT_ISSET(ops,'s'), func))) {
	zsfree(getlinknode(dirstack));
	unqueue_signals();
	return 1;
    }
    cd_new_pwd(func, dir, OPT_ISSET(ops, 'q'));

    unqueue_signals();
    return 0;
}

/* Get directory to chdir to */

/**/
static LinkNode
cd_get_dest(char *nam, char **argv, int hard, int func)
{
    LinkNode dir = NULL;
    LinkNode target;
    char *dest;

    if (!argv[0]) {
	if (func == BIN_POPD && !nextnode(firstnode(dirstack))) {
	    zwarnnam(nam, "directory stack empty");
	    return NULL;
	}
	if (func == BIN_PUSHD && unset(PUSHDTOHOME))
	    dir = nextnode(firstnode(dirstack));
	if (dir)
	    zinsertlinknode(dirstack, dir, getlinknode(dirstack));
	else if (func != BIN_POPD) {
	    if (!home) {
		zwarnnam(nam, "HOME not set");
		return NULL;
	    }
	    zpushnode(dirstack, ztrdup(home));
	}
    } else if (!argv[1]) {
	int dd;
	char *end;

	doprintdir++;
	if (!isset(POSIXCD) && argv[0][1] && (argv[0][0] == '+' || argv[0][0] == '-')
	    && strspn(argv[0]+1, "0123456789") == strlen(argv[0]+1)) {
	    dd = zstrtol(argv[0] + 1, &end, 10);
	    if (*end == '\0') {
		if ((argv[0][0] == '+') ^ isset(PUSHDMINUS))
		    for (dir = firstnode(dirstack); dir && dd; dd--, incnode(dir));
		else
		    for (dir = lastnode(dirstack); dir != (LinkNode) dirstack && dd;
			 dd--, dir = prevnode(dir));
		if (!dir || dir == (LinkNode) dirstack) {
		    zwarnnam(nam, "no such entry in dir stack");
		    return NULL;
		}
	    }
	}
	if (!dir)
	    zpushnode(dirstack, ztrdup(strcmp(argv[0], "-")
				       ? (doprintdir--, argv[0]) : oldpwd));
    } else {
	char *u, *d;
	int len1, len2, len3;

	if (!(u = strstr(pwd, argv[0]))) {
	    zwarnnam(nam, "string not in pwd: %s", argv[0]);
	    return NULL;
	}
	len1 = strlen(argv[0]);
	len2 = strlen(argv[1]);
	len3 = u - pwd;
	d = (char *)zalloc(len3 + len2 + strlen(u + len1) + 1);
	strncpy(d, pwd, len3);
	strcpy(d + len3, argv[1]);
	strcat(d, u + len1);
	zpushnode(dirstack, d);
	doprintdir++;
    }

    target = dir;
    if (func == BIN_POPD) {
	if (!dir) {
	    target = dir = firstnode(dirstack);
	} else if (dir != firstnode(dirstack)) {
	    return dir;
	}
	dir = nextnode(dir);
    }
    if (!dir) {
	dir = firstnode(dirstack);
    }
    if (!dir || !getdata(dir)) {
	DPUTS(1, "Directory not set, not detected early enough");
	return NULL;
    }
    if (!(dest = cd_do_chdir(nam, getdata(dir), hard))) {
	if (!target)
	    zsfree(getlinknode(dirstack));
	if (func == BIN_POPD)
	    zsfree(remnode(dirstack, dir));
	return NULL;
    }
    if (dest != (char *)getdata(dir)) {
	zsfree(getdata(dir));
	setdata(dir, dest);
    }
    return target ? target : dir;
}

/* Change to given directory, if possible.  This function works out  *
 * exactly how the directory should be interpreted, including cdpath *
 * and CDABLEVARS.  For each possible interpretation of the given    *
 * path, this calls cd_try_chdir(), which attempts to chdir to that  *
 * particular path.                                                  */

/**/
static char *
cd_do_chdir(char *cnam, char *dest, int hard)
{
    char **pp, *ret;
    int hasdot = 0, eno = ENOENT;
    /*
     * nocdpath indicates that cdpath should not be used.
     * This is the case iff dest is a relative path
     * whose first segment is . or .., but if the path is
     * absolute then cdpath won't be used anyway.
     */
    int nocdpath;
#ifdef __CYGWIN__
    /*
     * Normalize path under Cygwin to avoid messing with
     * DOS style names with drives in them
     */
    static char buf[PATH_MAX+1];
#ifdef HAVE_CYGWIN_CONV_PATH
    cygwin_conv_path(CCP_WIN_A_TO_POSIX | CCP_RELATIVE, dest, buf,
		     PATH_MAX);
#else
#ifndef _SYS_CYGWIN_H
    void cygwin_conv_to_posix_path(const char *, char *);
#endif

    cygwin_conv_to_posix_path(dest, buf);
#endif
    dest = buf;
#endif
    nocdpath = dest[0] == '.' &&
	(dest[1] == '/' || !dest[1] || (dest[1] == '.' &&
					(dest[2] == '/' || !dest[2])));

    /*
     * If we have an absolute path, use it as-is only
     */
    if (*dest == '/') {
	if ((ret = cd_try_chdir(NULL, dest, hard)))
	    return ret;
	zwarnnam(cnam, "%e: %s", errno, dest);
	return NULL;
    }

    /*
     * If cdpath is being used, check it for ".".
     * Don't bother doing this if POSIXCD is set, we don't
     * need to know (though it doesn't actually matter).
     */
    if (!nocdpath && !isset(POSIXCD))
	for (pp = cdpath; *pp; pp++)
	    if (!(*pp)[0] || ((*pp)[0] == '.' && (*pp)[1] == '\0'))
		hasdot = 1;
    /*
     * If 
     * (- there is no . in cdpath
     *  - or cdpath is not being used)
     *  - and the POSIXCD option is not set
     * try the directory as-is (i.e. from .)
     */
    if (!hasdot && !isset(POSIXCD)) {
	if ((ret = cd_try_chdir(NULL, dest, hard)))
	    return ret;
	if (errno != ENOENT)
	    eno = errno;
    }
    /* if cdpath is being used, try given directory relative to each element in
       cdpath in turn */
    if (!nocdpath)
	for (pp = cdpath; *pp; pp++) {
	    if ((ret = cd_try_chdir(*pp, dest, hard))) {
		if (isset(POSIXCD)) {
		    /*
		     * For POSIX we need to print the directory
		     * any time CDPATH was used, except in the
		     * special case of an empty segment being
		     * treated as a ".".
		     */
		    if (**pp)
			doprintdir++;
		}  else {
		    if (strcmp(*pp, ".")) {
			doprintdir++;
		    }
		}
		return ret;
	    }
	    if (errno != ENOENT)
		eno = errno;
	}
    /*
     * POSIX requires us to check "." after CDPATH rather than before.
     */
    if (isset(POSIXCD)) {
	if ((ret = cd_try_chdir(NULL, dest, hard)))
	    return ret;
	if (errno != ENOENT)
	    eno = errno;
    }

    /* handle the CDABLEVARS option */
    if ((ret = cd_able_vars(dest))) {
	if ((ret = cd_try_chdir(NULL, ret,hard))) {
	    doprintdir++;
	    return ret;
	}
	if (errno != ENOENT)
	    eno = errno;
    }

    /* If we got here, it means that we couldn't chdir to any of the
       multitudinous possible paths allowed by zsh.  We've run out of options!
       Add more here! */
    zwarnnam(cnam, "%e: %s", eno, dest);
    return NULL;
}

/* If the CDABLEVARS option is set, return the new *
 * interpretation of the given path.               */

/**/
char *
cd_able_vars(char *s)
{
    char *rest, save;

    if (isset(CDABLEVARS)) {
	for (rest = s; *rest && *rest != '/'; rest++);
	save = *rest;
	*rest = 0;
	s = getnameddir(s);
	*rest = save;

	if (s && *rest)
	    s = dyncat(s, rest);

	return s;
    }
    return NULL;
}

/* Attempt to change to a single given directory.  The directory,    *
 * for the convenience of the calling function, may be provided in   *
 * two parts, which must be concatenated before attempting to chdir. *
 * Returns NULL if the chdir fails.  If the directory change is      *
 * possible, it is performed, and a pointer to the new full pathname *
 * is returned.                                                      */

/**/
static char *
cd_try_chdir(char *pfix, char *dest, int hard)
{
    char *buf;
    int dlen, dochaselinks = 0;

    /* handle directory prefix */
    if (pfix && *pfix) {
	if (*pfix == '/') {
#ifdef __CYGWIN__
/* NB: Don't turn "/"+"bin" into "//"+"bin" by mistake!  "//bin" may *
 * not be what user really wants (probably wants "/bin"), but        *
 * "//bin" could be valid too (see fixdir())!  This is primarily for *
 * handling CDPATH correctly.  Likewise for "//"+"bin" not becoming  *
 * "///bin" (aka "/bin").                                            */
	    int root = pfix[1] == '\0' || (pfix[1] == '/' && pfix[2] == '\0');
	    buf = tricat(pfix, ( root ? "" : "/" ), dest);
#else
	    buf = tricat(pfix, "/", dest);
#endif
	} else {
	    int pfl = strlen(pfix);
	    dlen = strlen(pwd);
	    if (dlen == 1 && *pwd == '/')
		dlen = 0;
	    buf = zalloc(dlen + pfl + strlen(dest) + 3);
	    if (dlen)
		strcpy(buf, pwd);
	    buf[dlen] = '/';
	    strcpy(buf + dlen + 1, pfix);
	    buf[dlen + 1 + pfl] = '/';
	    strcpy(buf + dlen + pfl + 2, dest);
	}
    } else if (*dest == '/')
	buf = ztrdup(dest);
    else {
	dlen = strlen(pwd);
	if (pwd[dlen-1] == '/')
	    --dlen;
	buf = zalloc(dlen + strlen(dest) + 2);
	strcpy(buf, pwd);
	buf[dlen] = '/';
	strcpy(buf + dlen + 1, dest);
    }

    /* Normalise path.  See the definition of fixdir() for what this means.
     * We do not do this if we are chasing links.
     */
    if (!chasinglinks)
	dochaselinks = fixdir(buf);
    else
	unmetafy(buf, &dlen);

    /* We try the full path first.  If that fails, try the
     * argument to cd relatively.  This is useful if the cwd
     * or a parent directory is renamed in the interim.
     */
    if (lchdir(buf, NULL, hard) &&
	(pfix || *dest == '/' || lchdir(unmeta(dest), NULL, hard))) {
	free(buf);
	return NULL;
    }
    /* the chdir succeeded, so decide if we should force links to be chased */
    if (dochaselinks)
	chasinglinks = 1;
    return metafy(buf, -1, META_NOALLOC);
}

/* do the extra processing associated with changing directory */

/**/
static void
cd_new_pwd(int func, LinkNode dir, int quiet)
{
    char *new_pwd, *s;
    struct stat st1, st2;
    int dirstacksize;

    if (func == BIN_PUSHD)
	rolllist(dirstack, dir);
    new_pwd = remnode(dirstack, dir);

    if (func == BIN_POPD && firstnode(dirstack)) {
	zsfree(new_pwd);
	new_pwd = getlinknode(dirstack);
    } else if (func == BIN_CD && unset(AUTOPUSHD))
	zsfree(getlinknode(dirstack));

    if (chasinglinks) {
	s = findpwd(new_pwd);
	if (s) {
	    zsfree(new_pwd);
	    new_pwd = s;
	}
    }
    if (isset(PUSHDIGNOREDUPS)) {
	LinkNode n;
	for (n = firstnode(dirstack); n; incnode(n)) {
	    if (!strcmp(new_pwd, getdata(n))) {
		zsfree(remnode(dirstack, n));
		break;
	    }
	}
    }

    if (stat(unmeta(new_pwd), &st1) < 0) {
	zsfree(new_pwd);
	new_pwd = NULL;
	new_pwd = metafy(zgetcwd(), -1, META_DUP);
    } else if (stat(".", &st2) < 0) {
	if (chdir(unmeta(new_pwd)) < 0)
	    zwarn("unable to chdir(%s): %e", new_pwd, errno);
    } else if (st1.st_ino != st2.st_ino || st1.st_dev != st2.st_dev) {
	if (chasinglinks) {
	    zsfree(new_pwd);
	    new_pwd = NULL;
	    new_pwd = metafy(zgetcwd(), -1, META_DUP);
	} else if (chdir(unmeta(new_pwd)) < 0)
	    zwarn("unable to chdir(%s): %e", new_pwd, errno);
    }

    /* shift around the pwd variables, to make oldpwd and pwd relate to the
       current (i.e. new) pwd */
    zsfree(oldpwd);
    oldpwd = pwd;
    setjobpwd();
    pwd = new_pwd;
    set_pwd_env();

    if (isset(INTERACTIVE) || isset(POSIXCD)) {
	if (func != BIN_CD && isset(INTERACTIVE)) {
            if (unset(PUSHDSILENT) && !quiet)
	        printdirstack();
	} else if (unset(CDSILENT) && doprintdir) {
	    fprintdir(pwd, stdout);
	    putchar('\n');
	}
    }

    /* execute the chpwd function */
    fflush(stdout);
    fflush(stderr);
    if (!quiet)
	callhookfunc("chpwd", NULL, 1, NULL);

    dirstacksize = getiparam("DIRSTACKSIZE");
    /* handle directory stack sizes out of range */
    if (dirstacksize > 0) {
	int remove = countlinknodes(dirstack) -
	    (dirstacksize < 2 ? 2 : dirstacksize);
	while (remove-- >= 0)
	    zsfree(remnode(dirstack, lastnode(dirstack)));
    }
}

/* Print the directory stack */

/**/
static void
printdirstack(void)
{
    LinkNode node;

    fprintdir(pwd, stdout);
    for (node = firstnode(dirstack); node; incnode(node)) {
	putchar(' ');
	fprintdir(getdata(node), stdout);
    }
    putchar('\n');
}

/* Normalise a path.  Segments consisting of ., and foo/.. *
 * combinations, are removed and the path is unmetafied.
 * Returns 1 if we found a ../ path which should force links to
 * be chased, 0 otherwise.
 */

/**/
int
fixdir(char *src)
{
    char *dest = src, *d0 = dest;
#ifdef __CYGWIN__
    char *s0 = src;
#endif
    /* This function is always called with n path containing at
     * least one slash, either because one was input by the user or
     * because the caller has prepended either pwd or a cdpath dir.
     * If asked to make a relative change and pwd is set to ".",
     * the current directory has been removed out from under us,
     * so force links to be chased.
     *
     * Ordinarily we can't get here with "../" as the first component
     * but handle the silly special case of ".." in cdpath.
     *
     * Order of comparisons here looks funny, but it short-circuits
     * most rapidly in the event of a false condition.  Set to 2
     * here so we still obey the (lack of) CHASEDOTS option after
     * the first "../" is preserved (test chasedots > 1 below).
     */
    int chasedots = (src[0] == '.' && pwd[0] == '.' && pwd[1] == '\0' &&
		     (src[1] == '/' || (src[1] == '.' && src[2] == '/'))) * 2;

/*** if have RFS superroot directory ***/
#ifdef HAVE_SUPERROOT
    /* allow /.. segments to remain */
    while (*src == '/' && src[1] == '.' && src[2] == '.' &&
	   (!src[3] || src[3] == '/')) {
	*dest++ = '/';
	*dest++ = '.';
	*dest++ = '.';
	src += 3;
    }
#endif

    for (;;) {
	/* compress multiple /es into single */
	if (*src == '/') {
#ifdef __CYGWIN__
	    /* allow leading // under cygwin, but /// still becomes / */
	    if (src == s0 && src[1] == '/' && src[2] != '/')
		*dest++ = *src++;
#endif
	    *dest++ = *src++;
	    while (*src == '/')
		src++;
	}
	/* if we are at the end of the input path, remove a trailing / (if it
	   exists), and return ct */
	if (!*src) {
	    while (dest > d0 + 1 && dest[-1] == '/')
		dest--;
	    *dest = '\0';
	    return chasedots;
	}
	if (src[0] == '.' && src[1] == '.' &&
	    (src[2] == '\0' || src[2] == '/')) {
	    if (isset(CHASEDOTS) || chasedots > 1) {
		chasedots = 1;
		/* and treat as normal path segment */
	    } else {
		if (dest > d0 + 1) {
		    /*
		     * remove a foo/.. combination:
		     * first check foo exists, else return.
		     */
		    struct stat st;
		    *dest = '\0';
		    if (stat(d0, &st) < 0 || !S_ISDIR(st.st_mode)) {
			char *ptrd, *ptrs;
			if (dest == src)
			    *dest = '.';
			for (ptrs = src, ptrd = dest; *ptrs; ptrs++, ptrd++)
			    *ptrd = (*ptrs == Meta) ? (*++ptrs ^ 32) : *ptrs;
			*ptrd = '\0';
			return 1;
		    }
		    for (dest--; dest > d0 + 1 && dest[-1] != '/'; dest--);
		    if (dest[-1] != '/')
			dest--;
		}
		src++;
		while (*++src == '/');
		continue;
	    }
	}
	if (src[0] == '.' && (src[1] == '/' || src[1] == '\0')) {
	    /* skip a . section */
	    while (*++src == '/');
	} else {
	    /* copy a normal segment into the output */
	    while (*src != '/' && *src != '\0')
		if ((*dest++ = *src++) == Meta)
		    dest[-1] = *src++ ^ 32;
	}
    }
    /* unreached */
}

/**/
mod_export void
printqt(char *str)
{
    /* Print str, but turn any single quote into '\'' or ''. */
    for (; *str; str++)
	if (*str == '\'')
	    printf(isset(RCQUOTES) ? "''" : "'\\''");
	else
	    putchar(*str);
}

/**/
mod_export void
printif(char *str, int c)
{
    /* If flag c has an argument, print that */
    if (str) {
	printf(" -%c ", c);
	quotedzputs(str, stdout);
    }
}

/**** history list functions ****/

/* fc, history, r */

/**/
int
bin_fc(char *nam, char **argv, Options ops, int func)
{
    zlong first = -1, last = -1;
    int retval;
    char *s;
    struct asgment *asgf = NULL, *asgl = NULL;
    Patprog pprog = NULL;

    /* fc is only permitted in interactive shells */
#ifdef FACIST_INTERACTIVE
    if (!interact) {
	zwarnnam(nam, "not interactive shell");
	return 1;
    }
#endif
    if (OPT_ISSET(ops,'p')) {
	char *hf = "";
	zlong hs = DEFAULT_HISTSIZE;
	zlong shs = 0;
	int level = OPT_ISSET(ops,'a') ? locallevel : -1;
	if (*argv) {
	    hf = *argv++;
	    if (*argv) {
		char *check;
		hs = zstrtol(*argv++, &check, 10);
		if (*check) {
		    zwarnnam("fc", "HISTSIZE must be an integer");
		    return 1;
		}
		if (*argv) {
		    shs = zstrtol(*argv++, &check, 10);
		    if (*check) {
			zwarnnam("fc", "SAVEHIST must be an integer");
			return 1;
		    }
		} else
		    shs = hs;
		if (*argv) {
		    zwarnnam("fc", "too many arguments");
		    return 1;
		}
	    } else {
		hs = histsiz;
		shs = savehistsiz;
	    }
	}
	if (!pushhiststack(hf, hs, shs, level))
	    return 1;
	if (*hf) {
	    struct stat st;
	    if (stat(hf, &st) >= 0 || errno != ENOENT)
		readhistfile(hf, 1, HFILE_USE_OPTIONS);
	}
	return 0;
    }
    if (OPT_ISSET(ops,'P')) {
	if (*argv) {
	    zwarnnam("fc", "too many arguments");
	    return 1;
	}
	return !saveandpophiststack(-1, HFILE_USE_OPTIONS);
    }
    /* with the -m option, the first argument is taken *
     * as a pattern that history lines have to match   */
    if (*argv && OPT_ISSET(ops,'m')) {
	tokenize(*argv);
	if (!(pprog = patcompile(*argv++, 0, NULL))) {
	    zwarnnam(nam, "invalid match pattern");
	    return 1;
	}
    }
    queue_signals();
    if (OPT_ISSET(ops,'R')) {
	/* read history from a file */
	readhistfile(*argv, 1, OPT_ISSET(ops,'I') ? HFILE_SKIPOLD : 0);
	unqueue_signals();
	return 0;
    }
    if (OPT_ISSET(ops,'W')) {
	/* write history to a file */
	savehistfile(*argv, 1, OPT_ISSET(ops,'I') ? HFILE_SKIPOLD : 0);
	unqueue_signals();
	return 0;
    }
    if (OPT_ISSET(ops,'A')) {
	/* append history to a file */
	savehistfile(*argv, 1, HFILE_APPEND |
		     (OPT_ISSET(ops,'I') ? HFILE_SKIPOLD : 0));
	unqueue_signals();
	return 0;
    }

    if (zleactive) {
	unqueue_signals();
	zwarnnam(nam, "no interactive history within ZLE");
	return 1;
    }

    /* put foo=bar type arguments into the substitution list */
    while (*argv && equalsplit(*argv, &s)) {
	Asgment a = (Asgment) zhalloc(sizeof *a);

	if (!**argv) {
	    zwarnnam(nam, "invalid replacement pattern: =%s", s);
	    return 1;
	}
	if (!asgf)
	    asgf = asgl = a;
	else {
	    asgl->node.next = &a->node;
	    asgl = a;
	}
	a->name = *argv;
	a->flags = 0;
	a->value.scalar = s;
	a->node.next = a->node.prev = NULL;
	argv++;
    }
    /* interpret and check first history line specifier */
    if (*argv) {
	first = fcgetcomm(*argv);
	if (first == -1) {
	    unqueue_signals();
	    return 1;
	}
	argv++;
    }
    /* interpret and check second history line specifier */
    if (*argv) {
	last = fcgetcomm(*argv);
	if (last == -1) {
	    unqueue_signals();
	    return 1;
	}
	argv++;
    }
    /* There is a maximum of two history specifiers.  At least, there *
     * will be as long as the history list is one-dimensional.        */
    if (*argv) {
	unqueue_signals();
	zwarnnam("fc", "too many arguments");
	return 1;
    }
    /* default values of first and last, and range checking */
    if (last == -1) {
	if (OPT_ISSET(ops,'l') && first < curhist) {
	    /*
	     * When listing base our calculations on curhist,
	     * to show anything added since the edited history line.
	     * Also, in that case curhist will have been modified
	     * past the current history line; then we want to
	     * show everything, because the user expects to
	     * see the result of "print -s".  Otherwise, we subtract
	     * -1 from the line, because the user doesn't usually expect
	     * to see the command line that caused history to be
	     * listed.
	     */
	    last = (curline.histnum == curhist) ? addhistnum(curhist,-1,0)
		: curhist;
	    if (last < firsthist())
		last = firsthist();
	}
	else
	    last = first;
    }
    if (first == -1) {
	/*
	 * When listing, we want to see everything that's been
	 * added to the history, including by print -s, so use
	 * curhist.
	 * When reexecuting, we want to restrict to the last edited
	 * command line to avoid giving the user a nasty turn
	 * if some helpful soul ran "print -s 'rm -rf /'".
	 */
	int xflags = OPT_ISSET(ops,'L') ? HIST_FOREIGN : 0;
	first = OPT_ISSET(ops,'l')? addhistnum(curhist,-16,xflags)
			: addhistnum(curline.histnum,-1,xflags);
	if (first < 1)
	    first = 1;
	if (last < first)
	    last = first;
    }
    if (OPT_ISSET(ops,'l')) {
	/* list the required part of the history */
	retval = fclist(stdout, ops, first, last, asgf, pprog, 0);
	unqueue_signals();
    }
    else {
	/* edit history file, and (if successful) use the result as a new command */
	int tempfd;
	FILE *out;
	char *fil;

	retval = 1;
	if ((tempfd = gettempfile(NULL, 1, &fil)) < 0
	 || ((out = fdopen(tempfd, "w")) == NULL)) {
	    unqueue_signals();
	    zwarnnam("fc", "can't open temp file: %e", errno);
	} else {
	    /*
	     * Nasty behaviour results if we use the current history
	     * line here.  Treat it as if it doesn't exist, unless
	     * that gives us an empty range.
	     */
	    if (last >= curhist) {
		last = curhist - 1;
		if (first > last) {
		    unqueue_signals();
		    zwarnnam("fc",
		      "current history line would recurse endlessly, aborted");
		    fclose(out);
		    unlink(fil);
		    return 1;
		}
	    }
	    ops->ind['n'] = 1;	/* No line numbers here. */
	    if (!fclist(out, ops, first, last, asgf, pprog, 1)) {
		char *editor;

		if (func == BIN_R || OPT_ISSET(ops, 's'))
		    editor = "-";
		else if (OPT_HASARG(ops, 'e'))
		    editor = OPT_ARG(ops, 'e');
		else
		    editor = getsparam("FCEDIT");
		if (!editor)
		    editor = getsparam("EDITOR");
		if (!editor)
		    editor = DEFAULT_FCEDIT;

		unqueue_signals();
		if (fcedit(editor, fil)) {
		    if (stuff(fil))
			zwarnnam("fc", "%e: %s", errno, fil);
		    else {
			loop(0,1);
			retval = lastval;
		    }
		}
	    } else
		unqueue_signals();
	}
	unlink(fil);
    }
    return retval;
}

/* History handling functions: these are called by ZLE, as well as  *
 * the actual builtins.  fcgetcomm() gets a history line, specified *
 * either by number or leading string.  fcsubs() performs a given   *
 * set of simple old=new substitutions on a given command line.     *
 * fclist() outputs a given range of history lines to a text file.  */

/* get the history event associated with s */

/**/
static zlong
fcgetcomm(char *s)
{
    zlong cmd;

    /* First try to match a history number.  Negative *
     * numbers indicate reversed numbering.           */
    if ((cmd = atoi(s)) != 0 || *s == '0') {
	if (cmd < 0)
	    cmd = addhistnum(curline.histnum,cmd,HIST_FOREIGN);
	if (cmd < 0)
	    cmd = 0;
	return cmd;
    }
    /* not a number, so search by string */
    cmd = hcomsearch(s);
    if (cmd == -1)
	zwarnnam("fc", "event not found: %s", s);
    return cmd;
}

/* Perform old=new substitutions.  Uses the asgment structure from zsh.h, *
 * which is essentially a linked list of string,replacement pairs.       */

/**/
static int
fcsubs(char **sp, struct asgment *sub)
{
    char *oldstr, *newstr, *oldpos, *newpos, *newmem, *s = *sp;
    int subbed = 0;

    /* loop through the linked list */
    while (sub) {
	oldstr = sub->name;
	newstr = sub->value.scalar;
	sub = (Asgment)sub->node.next;
	oldpos = s;
	/* loop over occurrences of oldstr in s, replacing them with newstr */
	while ((newpos = (char *)strstr(oldpos, oldstr))) {
	    newmem = (char *) zhalloc(1 + (newpos - s)
				      + strlen(newstr) + strlen(newpos + strlen(oldstr)));
	    ztrncpy(newmem, s, newpos - s);
	    strcat(newmem, newstr);
	    oldpos = newmem + strlen(newmem);
	    strcat(newmem, newpos + strlen(oldstr));
	    s = newmem;
	    subbed = 1;
	}
    }
    *sp = s;
    return subbed;
}

/* Print a series of history events to a file.  The file pointer is     *
 * given by f, and the required range of events by first and last.      *
 * subs is an optional list of foo=bar substitutions to perform on the  *
 * history lines before output.  com is an optional comp structure      *
 * that the history lines are required to match.  n, r, D and d are     *
 * options: n indicates that each line should be numbered.  r indicates *
 * that the lines should be output in reverse order (newest first).     *
 * D indicates that the real time taken by each command should be       *
 * output.  d indicates that the time of execution of each command      *
 * should be output; d>1 means that the date should be output too; d>3  *
 * means that mm/dd/yyyy form should be used for the dates, as opposed  *
 * to dd.mm.yyyy form; d>7 means that yyyy-mm-dd form should be used.   */

/**/
static int
fclist(FILE *f, Options ops, zlong first, zlong last,
       struct asgment *subs, Patprog pprog, int is_command)
{
    int fclistdone = 0, xflags = 0;
    zlong tmp;
    char *s, *tdfmt, *timebuf;
    Histent ent;

    /* reverse range if required */
    if (OPT_ISSET(ops,'r')) {
	tmp = last;
	last = first;
	first = tmp;
    }
    if (is_command && first > last) {
	zwarnnam("fc", "history events can't be executed backwards, aborted");
	if (f != stdout)
	    fclose(f);
	return 1;
    }

    ent = gethistent(first, first < last? GETHIST_DOWNWARD : GETHIST_UPWARD);
    if (!ent || (first < last? ent->histnum > last : ent->histnum < last)) {
	if (first == last) {
	    char buf[DIGBUFSIZE];
	    convbase(buf, first, 10);
	    zwarnnam("fc", "no such event: %s", buf);
	} else
	    zwarnnam("fc", "no events in that range");
	if (f != stdout)
	    fclose(f);
	return 1;
    }

    if (OPT_ISSET(ops,'d') || OPT_ISSET(ops,'f') ||
	OPT_ISSET(ops,'E') || OPT_ISSET(ops,'i') ||
	OPT_ISSET(ops,'t')) {
	if (OPT_ISSET(ops,'t')) {
	    tdfmt = OPT_ARG(ops,'t');
	} else if (OPT_ISSET(ops,'i')) {
	    tdfmt = "%Y-%m-%d %H:%M";
	} else if (OPT_ISSET(ops,'E')) {
	    tdfmt = "%f.%-m.%Y %H:%M";
	} else if (OPT_ISSET(ops,'f')) {
	    tdfmt = "%-m/%f/%Y %H:%M";
	} else {
	    tdfmt = "%H:%M";
	}
	timebuf = zhalloc(256);
    } else {
	tdfmt = timebuf = NULL;
    }

    /* xflags exclude events */
    if (OPT_ISSET(ops,'L')) {
	xflags |= HIST_FOREIGN;
    }
    if (OPT_ISSET(ops,'I')) {
	xflags |= HIST_READ;
    }

    for (;;) {
	if (ent->node.flags & xflags)
	    s = NULL;
	else
	    s = dupstring(ent->node.nam);
	/* this if does the pattern matching, if required */
	if (s && (!pprog || pattry(pprog, s))) {
	    /* perform substitution */
	    fclistdone |= (subs ? fcsubs(&s, subs) : 1);

	    /* do numbering */
	    if (!OPT_ISSET(ops,'n')) {
		char buf[DIGBUFSIZE];
		convbase(buf, ent->histnum, 10);
		fprintf(f, "%5s%c ", buf,
			ent->node.flags & HIST_FOREIGN ? '*' : ' ');
	    }
	    /* output actual time (and possibly date) of execution of the
	       command, if required */
	    if (tdfmt != NULL) {
		struct tm *ltm;
		int len;
		ltm = localtime(&ent->stim);
		if ((len = ztrftime(timebuf, 256, tdfmt, ltm, 0L)) >= 0) {
		    fwrite(timebuf, 1, len, f);
		    fprintf(f, "  ");
		}
	    }
	    /* display the time taken by the command, if required */
	    if (OPT_ISSET(ops,'D')) {
		long diff;
		diff = (ent->ftim) ? ent->ftim - ent->stim : 0;
		fprintf(f, "%ld:%02ld  ", diff / 60, diff % 60);
	    }

	    /* output the command */
	    if (f == stdout) {
		nicezputs(s, f);
		putc('\n', f);
	    } else {
		int len;
		unmetafy(s, &len);
		fwrite(s, 1, len, f);
		putc('\n', f);
	    }
	}
	/* move on to the next history line, or quit the loop */
	if (first < last) {
	    if (!(ent = down_histent(ent)) || ent->histnum > last)
		break;
	}
	else {
	    if (!(ent = up_histent(ent)) || ent->histnum < last)
		break;
	}
    }

    /* final processing */
    if (f != stdout)
	fclose(f);
    if (!fclistdone) {
	if (subs)
	    zwarnnam("fc", "no substitutions performed");
	else if (xflags || pprog)
	    zwarnnam("fc", "no matching events found");
	return 1;
    }
    return 0;
}

/* edit a history file */

/**/
static int
fcedit(char *ename, char *fn)
{
    char *s;

    if (!strcmp(ename, "-"))
	return 1;

    s = tricat(ename, " ", fn);
    execstring(s, 1, 0, "fc");
    zsfree(s);

    return !lastval;
}

/**** parameter builtins ****/

/* Separate an argument into name=value parts, returning them in an     *
 * asgment structure.  Because the asgment structure used is global,    *
 * only one of these can be active at a time.  The string s gets placed *
 * in this global structure, so it needs to be in permanent memory.     */

/**/
static Asgment
getasg(char ***argvp, LinkList assigns)
{
    char *s = **argvp;
    static struct asgment asg;

    /* sanity check for valid argument */
    if (!s) {
	if (assigns) {
	    Asgment asgp = (Asgment)firstnode(assigns);
	    if (!asgp)
		return NULL;
	    (void)uremnode(assigns, &asgp->node);
	    return asgp;
	}
	return NULL;
    }

    /* check if name is empty */
    if (*s == '=') {
	zerr("bad assignment");
	return NULL;
    }
    asg.name = s;
    asg.flags = 0;

    /* search for `=' */
    for (; *s && *s != '='; s++);

    /* found `=', so return with a value */
    if (*s) {
	*s = '\0';
	asg.value.scalar = s + 1;
    } else {
	/* didn't find `=', so we only have a name */
	asg.value.scalar = NULL;
    }
    (*argvp)++;
    return &asg;
}

/* for new special parameters */
enum {
    NS_NONE,
    NS_NORMAL,
    NS_SECONDS
};

static const struct gsu_scalar tiedarr_gsu =
{ tiedarrgetfn, tiedarrsetfn, tiedarrunsetfn };

/* Install a base if we are turning on a numeric option with an argument */

static int
typeset_setbase(const char *name, Param pm, Options ops, int on, int always)
{
    char *arg = NULL;

    if ((on & PM_INTEGER) && OPT_HASARG(ops,'i'))
	arg = OPT_ARG(ops,'i');
    else if ((on & PM_EFLOAT) && OPT_HASARG(ops,'E'))
	arg = OPT_ARG(ops,'E');
    else if ((on & PM_FFLOAT) && OPT_HASARG(ops,'F'))
	arg = OPT_ARG(ops,'F');

    if (arg) {
	char *eptr;
	int base = (int)zstrtol(arg, &eptr, 10);
	if (*eptr) {
	    if (on & PM_INTEGER)
		zwarnnam(name, "bad base value: %s", arg);
	    else
		zwarnnam(name, "bad precision value: %s", arg);
	    return 1;
	}
	if ((on & PM_INTEGER) && (base < 2 || base > 36)) {
	    zwarnnam(name, "invalid base (must be 2 to 36 inclusive): %d",
		     base);
	    return 1;
	}
	pm->base = base;
    } else if (always)
	pm->base = 0;

    return 0;
}

/* Install a width if we are turning on a padding option with an argument */

static int
typeset_setwidth(const char * name, Param pm, Options ops, int on, int always)
{
    char *arg = NULL;

    if ((on & PM_LEFT) && OPT_HASARG(ops,'L'))
	arg = OPT_ARG(ops,'L');
    else if ((on & PM_RIGHT_B) && OPT_HASARG(ops,'R'))
	arg = OPT_ARG(ops,'R');
    else if ((on & PM_RIGHT_Z) && OPT_HASARG(ops,'Z'))
	arg = OPT_ARG(ops,'Z');

    if (arg) {
	char *eptr;
	pm->width = (int)zstrtol(arg, &eptr, 10);
	if (*eptr) {
	    zwarnnam(name, "bad width value: %s", arg);
	    return 1;
	}
    } else if (always)
	pm->width = 0;

    return 0;
}

/* function to set a single parameter */

/**/
static Param
typeset_single(char *cname, char *pname, Param pm, int func,
	       int on, int off, int roff, Asgment asg, Param altpm,
	       Options ops, int joinchar)
{
    int usepm, tc, keeplocal = 0, newspecial = NS_NONE, readonly, dont_set = 0;
    char *subscript;

    /*
     * Do we use the existing pm?  Note that this isn't the end of the
     * story, because if we try and create a new pm at the same
     * locallevel as an unset one we use the pm struct anyway: that's
     * handled in createparam().  Here we just avoid using it for the
     * present tests if it's unset.
     *
     * POSIXBUILTINS horror: we need to retain the 'readonly' or 'export'
     * flags of an unset parameter.
     */
    usepm = pm && (!(pm->node.flags & PM_UNSET) ||
		   (isset(POSIXBUILTINS) &&
		    (pm->node.flags & (PM_READONLY|PM_EXPORTED))));

    /*
     * We need to compare types with an existing pm if special,
     * even if that's unset
     */
    if (!usepm && pm && (pm->node.flags & PM_SPECIAL))
	usepm = 2;	/* indicate that we preserve the PM_UNSET flag */

    /*
     * Don't use an existing param if
     *   - the local level has changed, and
     *   - we are really locallizing the parameter
     */
    if (usepm && locallevel != pm->level && (on & PM_LOCAL)) {
	/*
	 * If the original parameter was special and we're creating
	 * a new one, we need to keep it special.
	 *
	 * The -h (hide) flag prevents an existing special being made
	 * local.  It can be applied either to the special or in the
	 * typeset/local statement for the local variable.
	 */
	if ((pm->node.flags & PM_SPECIAL)
	    && !(on & PM_HIDE) && !(pm->node.flags & PM_HIDE & ~off))
	    newspecial = NS_NORMAL;
	usepm = 0;
    }

    /* attempting a type conversion, or making a tied colonarray? */
    tc = 0;
    if (ASG_ARRAYP(asg) && PM_TYPE(on) == PM_SCALAR &&
	!(usepm && (PM_TYPE(pm->node.flags) & (PM_ARRAY|PM_HASHED))))
	on |= PM_ARRAY;
    if (usepm && ASG_ARRAYP(asg) && newspecial == NS_NONE &&
	PM_TYPE(pm->node.flags) != PM_ARRAY &&
	PM_TYPE(pm->node.flags) != PM_HASHED) {
	if (on & (PM_EFLOAT|PM_FFLOAT|PM_INTEGER)) {
	    zerrnam(cname, "%s: can't assign array value to non-array", pname);
	    return NULL;
	}
	if (pm->node.flags & PM_SPECIAL) {
	    zerrnam(cname, "%s: can't assign array value to non-array special", pname);
	    return NULL;
	}
	tc = 1;
	usepm = 0;
    }
    else if (usepm || newspecial != NS_NONE) {
	int chflags = ((off & pm->node.flags) | (on & ~pm->node.flags)) &
	    (PM_INTEGER|PM_EFLOAT|PM_FFLOAT|PM_HASHED|
	     PM_ARRAY|PM_TIED|PM_AUTOLOAD);
	/* keep the parameter if just switching between floating types */
	if ((tc = chflags && chflags != (PM_EFLOAT|PM_FFLOAT)))
	    usepm = 0;
    }

    /*
     * Extra checks if converting the type of a parameter, or if
     * trying to remove readonlyness.  It's dangerous doing either
     * with a special or a parameter which isn't loaded yet (which
     * may be special when it is loaded; we can't tell yet).
     */
    if ((readonly =
	 ((usepm || newspecial != NS_NONE) &&
	  (off & pm->node.flags & PM_READONLY))) ||
	tc) {
	if (pm->node.flags & PM_SPECIAL) {
	    int err = 1;
	    if (!readonly && !strcmp(pname, "SECONDS"))
	    {
		/*
		 * We allow SECONDS to change type between integer
		 * and floating point.  If we are creating a new
		 * local copy we check the type here and allow
		 * a new special to be created with that type.
		 * We then need to make sure the correct type
		 * for the special is restored at the end of the scope.
		 * If we are changing the type of an existing
		 * parameter, we do the whole thing here.
		 */
		if (newspecial != NS_NONE)
		{
		    /*
		     * The first test allows `typeset' to copy the
		     * existing type.  This is the usual behaviour
		     * for making special parameters local.
		     */
		    if (PM_TYPE(on) == 0 || PM_TYPE(on) == PM_INTEGER ||
			PM_TYPE(on) == PM_FFLOAT || PM_TYPE(on) == PM_EFLOAT)
		    {
			newspecial = NS_SECONDS;
			err = 0;	/* and continue */
			tc = 0;	/* but don't do a normal conversion */
		    }
		} else if (!setsecondstype(pm, on, off)) {
		    if (asg->value.scalar &&
			!(pm = assignsparam(
			      pname, ztrdup(asg->value.scalar), 0)))
			return NULL;
		    usepm = 1;
		    err = 0;
		}
	    }
	    if (err)
	    {
		zerrnam(cname, "%s: can't change type of a special parameter",
			pname);
		return NULL;
	    }
	} else if (pm->node.flags & PM_AUTOLOAD) {
	    zerrnam(cname, "%s: can't change type of autoloaded parameter",
		    pname);
	    return NULL;
	}
    }
    else if (newspecial != NS_NONE && strcmp(pname, "SECONDS") == 0)
	newspecial = NS_SECONDS;

    if (isset(POSIXBUILTINS)) {
	/*
	 * Stricter rules about retaining readonly attribute in this case.
	 */
	if ((on & (PM_READONLY|PM_EXPORTED)) &&
	    (!usepm || (pm->node.flags & PM_UNSET)) &&
	    !ASG_VALUEP(asg))
	    on |= PM_UNSET;
	else if (usepm && (pm->node.flags & PM_READONLY) &&
		 !(on & PM_READONLY) && func != BIN_EXPORT) {
	    zerr("read-only variable: %s", pm->node.nam);
	    return NULL;
	}
	/* This is handled by createparam():
	if (usepm && (pm->node.flags & PM_EXPORTED) && !(off & PM_EXPORTED))
	    on |= PM_EXPORTED;
	*/
    }

    /*
     * A parameter will be local if
     * 1. we are re-using an existing local parameter
     *    or
     * 2. we are not using an existing parameter, but
     *   i. there is already a parameter, which will be hidden
     *     or
     *   ii. we are creating a new local parameter
     */
    if (usepm) {
	if ((asg->flags & ASG_ARRAY) ?
	    !(PM_TYPE(pm->node.flags) & (PM_ARRAY|PM_HASHED)) :
	    (asg->value.scalar && (PM_TYPE(pm->node.flags &
					   (PM_ARRAY|PM_HASHED))))) {
	    zerrnam(cname, "%s: inconsistent type for assignment", pname);
	    return NULL;
	}
	on &= ~PM_LOCAL;
	if (!on && !roff && !ASG_VALUEP(asg)) {
	    if (OPT_ISSET(ops,'p'))
		paramtab->printnode(&pm->node, PRINT_TYPESET);
	    else if (!OPT_ISSET(ops,'g') &&
		     (unset(TYPESETSILENT) || OPT_ISSET(ops,'m')))
		paramtab->printnode(&pm->node, PRINT_INCLUDEVALUE);
	    return pm;
	}
	if ((pm->node.flags & PM_RESTRICTED) && isset(RESTRICTED)) {
	    zerrnam(cname, "%s: restricted", pname);
	    return pm;
	}
	if ((on & PM_UNIQUE) && !(pm->node.flags & PM_READONLY & ~off)) {
	    Param apm;
	    char **x;
	    if (PM_TYPE(pm->node.flags) == PM_ARRAY) {
		x = (*pm->gsu.a->getfn)(pm);
		uniqarray(x);
		if (pm->node.flags & PM_SPECIAL) {
		    if (zheapptr(x))
			x = zarrdup(x);
		    (*pm->gsu.a->setfn)(pm, x);
		} else if (pm->ename && x)
		    arrfixenv(pm->ename, x);
	    } else if (PM_TYPE(pm->node.flags) == PM_SCALAR && pm->ename &&
		       (apm =
			(Param) paramtab->getnode(paramtab, pm->ename))) {
		x = (*apm->gsu.a->getfn)(apm);
		uniqarray(x);
		if (x)
		    arrfixenv(pm->node.nam, x);
	    }
	}
	if (usepm == 2)		/* do not change the PM_UNSET flag */
	    pm->node.flags = (pm->node.flags | (on & ~PM_READONLY)) & ~off;
	else {
	    /*
	     * Keep unset if using readonly in POSIX mode.
	     */
	    if (!(on & PM_READONLY) || !isset(POSIXBUILTINS))
		off |= PM_UNSET;
	    pm->node.flags = (pm->node.flags |
			      (on & ~PM_READONLY)) & ~off;
	}
	if (on & (PM_LEFT | PM_RIGHT_B | PM_RIGHT_Z)) {
	    if (typeset_setwidth(cname, pm, ops, on, 0))
		return NULL;
	}
	if (on & (PM_INTEGER | PM_EFLOAT | PM_FFLOAT)) {
	    if (typeset_setbase(cname, pm, ops, on, 0))
		return NULL;
	}
	if (!(pm->node.flags & (PM_ARRAY|PM_HASHED))) {
	    if (pm->node.flags & PM_EXPORTED) {
		if (!(pm->node.flags & PM_UNSET) && !pm->env && !ASG_VALUEP(asg))
		    addenv(pm, getsparam(pname));
	    } else if (pm->env && !(pm->node.flags & PM_HASHELEM))
		delenv(pm);
	    DPUTS(ASG_ARRAYP(asg), "BUG: typeset got array value where scalar expected");
	    if (altpm && !(pm->node.flags & PM_SPECIAL)) {
		struct tieddata* tdp = (struct tieddata *) pm->u.data;
		if (tdp) {
		    if (tdp->joinchar != joinchar && !asg->value.scalar) {
			/*
			 * Reassign the scalar to itself to do the splitting with
			 * the new joinchar
			 */
			tdp->joinchar = joinchar;
			if (!(pm = assignsparam(pname, ztrdup(getsparam(pname)), 0)))
			    return NULL;
		    }
		}
		else
		    DPUTS(!tdp, "BUG: no join character to update");
	    }
	    if (asg->value.scalar &&
		!(pm = assignsparam(pname, ztrdup(asg->value.scalar), 0)))
		return NULL;
	} else if (asg->flags & ASG_ARRAY) {
	    int flags = (asg->flags & ASG_KEY_VALUE) ? ASSPM_KEY_VALUE : 0;
	    if (!(pm = assignaparam(pname, asg->value.array ?
				 zlinklist2array(asg->value.array, 1) :
				 mkarray(NULL), flags)))
		return NULL;
	}
	if (errflag)
	    return NULL;
	pm->node.flags |= (on & PM_READONLY);
	if (OPT_ISSET(ops,'p'))
	    paramtab->printnode(&pm->node, PRINT_TYPESET);
	return pm;
    }

    if ((asg->flags & ASG_ARRAY) ?
	!(on & (PM_ARRAY|PM_HASHED)) :
	(asg->value.scalar && (on & (PM_ARRAY|PM_HASHED)))) {
	zerrnam(cname, "%s: inconsistent type for assignment", pname);
	return NULL;
    }

    /*
     * We're here either because we're creating a new parameter,
     * or we're adding a parameter at a different local level,
     * or we're converting the type of a parameter.  In the
     * last case only, we need to delete the old parameter.
     */
    if (tc) {
	/* Maintain existing readonly/exported status... */
	on |= ~off & (PM_READONLY|PM_EXPORTED) & pm->node.flags;
	/* ...but turn off existing readonly so we can delete it */
	pm->node.flags &= ~PM_READONLY;
	/*
	 * If we're just changing the type, we should keep the
	 * variable at the current level of localness.
	 */
	keeplocal = pm->level;
	/*
	 * Try to carry over a value, but not when changing from,
	 * to, or between non-scalar types.
	 *
	 * (We can do better now, but it does have user-visible
	 * implications.)
	 */
	if (!ASG_VALUEP(asg) && !((pm->node.flags|on) & (PM_ARRAY|PM_HASHED))) {
	    asg->value.scalar = dupstring(getsparam(pname));
	    asg->flags = 0;
	}
	/* pname may point to pm->nam which is about to disappear */
	pname = dupstring(pname);
	unsetparam_pm(pm, 0, 1);
    }

    if (newspecial != NS_NONE) {
	Param tpm, pm2;
	if ((pm->node.flags & PM_RESTRICTED) && isset(RESTRICTED)) {
	    zerrnam(cname, "%s: restricted", pname);
	    return pm;
	}
	if (pm->node.flags & PM_SINGLE) {
	    zerrnam(cname, "%s: can only have a single instance", pname);
	    return pm;
	}

	on |= pm->node.flags & PM_TIED;

	/*
	 * For specials, we keep the same struct but zero everything.
	 * Maybe it would be easier to create a new struct but copy
	 * the get/set methods.
	 */
	tpm = (Param) zshcalloc(sizeof *tpm);

	tpm->node.nam = pm->node.nam;
	if (pm->ename &&
	    (pm2 = (Param) paramtab->getnode(paramtab, pm->ename)) &&
	    pm2->level == locallevel) {
	    /* This is getting silly, but anyway:  if one of a path/PATH
	     * pair has already been made local at the current level, we
	     * have to make sure that the other one does not have its value
	     * saved:  since that comes from an internal variable it will
	     * already reflect the local value, so restoring it on exit
	     * would be wrong.
	     *
	     * This problem is also why we make sure we have a copy
	     * of the environment entry in tpm->env, rather than relying
	     * on the restored value to provide it.
	     */
	    tpm->node.flags = pm->node.flags | PM_NORESTORE;
	} else {
	    copyparam(tpm, pm, 1);
	}
	tpm->old = pm->old;
	tpm->level = pm->level;
	tpm->base = pm->base;
	tpm->width = pm->width;
	if (pm->env)
	    delenv(pm);
	tpm->env = NULL;

	pm->old = tpm;
	/*
	 * The remaining on/off flags should be harmless to use,
	 * because we've checked for unpleasant surprises above.
	 */
	pm->node.flags = (PM_TYPE(pm->node.flags) | on | PM_SPECIAL) & ~off;
	/*
	 * Readonlyness of special parameters must be preserved.
	 */
	pm->node.flags |= tpm->node.flags & PM_READONLY;
	if (newspecial == NS_SECONDS) {
	    /* We save off the raw internal value of the SECONDS var */
	    tpm->u.dval = getrawseconds();
	    setsecondstype(pm, on, off);
	}

	/*
	 * Final tweak: if we've turned on one of the flags with
	 * numbers, we should use the appropriate integer.
	 */
	if (on & (PM_LEFT|PM_RIGHT_B|PM_RIGHT_Z)) {
	    if (typeset_setwidth(cname, pm, ops, on, 1))
		return NULL;
	}
	if (on & (PM_INTEGER|PM_EFLOAT|PM_FFLOAT)) {
	    if (typeset_setbase(cname, pm, ops, on, 1))
		return NULL;
	}
    } else if ((subscript = strchr(pname, '['))) {
	if (on & PM_READONLY) {
	    zerrnam(cname,
		    "%s: can't create readonly array elements", pname);
	    return NULL;
	} else if ((on & PM_LOCAL) && locallevel) {
	    *subscript = 0;
	    pm = (Param) (paramtab == realparamtab ?
			  /* getnode2() to avoid autoloading */
			  paramtab->getnode2(paramtab, pname) :
			  paramtab->getnode(paramtab, pname));
	    *subscript = '[';
	    if (!pm || pm->level != locallevel) {
		zerrnam(cname,
			"%s: can't create local array elements", pname);
		return NULL;
	    }
	}
	if (PM_TYPE(on) == PM_SCALAR && !ASG_ARRAYP(asg)) {
	    /*
	     * This will either complain about bad identifiers, or will set
	     * a hash element or array slice.  This once worked by accident,
	     * creating a stray parameter along the way via createparam(),
	     * now called below in the isident() branch.
	     */
	    if (!(pm = assignsparam(
		      pname,
		      ztrdup(asg->value.scalar ? asg->value.scalar : ""), 0)))
		return NULL;
	    dont_set = 1;
	    asg->flags = 0;
	    keeplocal = 0;
	    on = pm->node.flags;
	} else if (PM_TYPE(on) == PM_ARRAY && ASG_ARRAYP(asg)) {
	    int flags = (asg->flags & ASG_KEY_VALUE) ? ASSPM_KEY_VALUE : 0;
	    if (!(pm = assignaparam(pname, asg->value.array ?
				    zlinklist2array(asg->value.array, 1) :
				    mkarray(NULL), flags)))
		return NULL;
	    dont_set = 1;
	    keeplocal = 0;
	    on = pm->node.flags;
	} else {
	    zerrnam(cname,
		    "%s: inconsistent array element or slice assignment", pname);
	    return NULL;
	}
    }
    /*
     * As we can hide existing parameters, we allow a name if
     * it's not a normal identifier but is one of the special
     * set found in the parameter table.  The second test is
     * because we can set individual positional parameters;
     * however "0" is not a positional parameter and is OK.
     *
     * It would be neater to extend isident() and be clearer
     * about where we allow various parameter types.  It's
     * not entirely clear to me isident() should reject
     * specially named parameters given that it accepts digits.
     */
    else if ((isident(pname) || paramtab->getnode(paramtab, pname))
	     && (!idigit(*pname) || !strcmp(pname, "0"))) {
	/*
	 * Create a new node for a parameter with the flags in `on' minus the
	 * readonly flag
	 */
	pm = createparam(pname, on & ~PM_READONLY);
	if (!pm) {
	    if (on & (PM_LEFT | PM_RIGHT_B | PM_RIGHT_Z |
		      PM_INTEGER | PM_EFLOAT | PM_FFLOAT))
		zerrnam(cname, "can't change variable attribute: %s", pname);
	    return NULL;
	}
	if (on & (PM_LEFT | PM_RIGHT_B | PM_RIGHT_Z)) {
	    if (typeset_setwidth(cname, pm, ops, on, 0)) {
		unsetparam_pm(pm, 0, 1);
		return NULL;
	    }
	}
	if (on & (PM_INTEGER | PM_EFLOAT | PM_FFLOAT)) {
	    if (typeset_setbase(cname, pm, ops, on, 0)) {
		unsetparam_pm(pm, 0, 1);
		return NULL;
	    }
	}
	if (isset(TYPESETTOUNSET))
	    pm->node.flags |= PM_DEFAULTED;
    } else {
	if (idigit(*pname))
	    zerrnam(cname, "not an identifier: %s", pname);
	else
	    zerrnam(cname, "not valid in this context: %s", pname);
	return NULL;
    }

    if (altpm && PM_TYPE(pm->node.flags) == PM_SCALAR && !(pm->node.flags & PM_SPECIAL)) {
	/*
	 * It seems safer to set this here than in createparam(),
	 * to make sure we only ever use the colonarr functions
	 * when u.data is correctly set.
	 */
	struct tieddata *tdp = (struct tieddata *)
	    zalloc(sizeof(struct tieddata));
	if (!tdp) {
	    unsetparam_pm(pm, 0, 1);
	    return NULL;
	}
	tdp->joinchar = joinchar;
	tdp->arrptr = &altpm->u.arr;

	pm->gsu.s = &tiedarr_gsu;
	pm->u.data = tdp;
    }

    if (keeplocal)
	pm->level = keeplocal;
    else if (on & PM_LOCAL)
	pm->level = locallevel;
    if (ASG_VALUEP(asg) && !dont_set) {
	Param ipm = pm;
	if (pm->node.flags & (PM_ARRAY|PM_HASHED)) {
	    char **arrayval;
	    int flags = (asg->flags & ASG_KEY_VALUE) ? ASSPM_KEY_VALUE : 0;
	    if (!ASG_ARRAYP(asg)) {
		/*
		 * Attempt to assign a scalar value to an array.
		 * This can happen if the array is special.
		 * We'll be lenient and guess what the user meant.
		 * This is how normal assignment works.
		 */
		if (*asg->value.scalar) {
		    /* Array with one value */
		    arrayval = mkarray(ztrdup(asg->value.scalar));
		} else {
		    /* Empty array */
		    arrayval = mkarray(NULL);
		}
	    } else if (asg->value.array)
		arrayval = zlinklist2array(asg->value.array, 1);
	    else
		arrayval = mkarray(NULL);
	    if (!(pm=assignaparam(pname, arrayval, flags)))
		return NULL;
	} else {
	    DPUTS(ASG_ARRAYP(asg), "BUG: inconsistent array value for scalar");
	    if (!(pm = assignsparam(pname, ztrdup(asg->value.scalar), 0)))
		return NULL;
	}
	if (pm != ipm) {
	    DPUTS(ipm->node.flags != pm->node.flags,
		  "BUG: parameter recreated with wrong flags");
	    unsetparam_pm(ipm, 0, 1);
	}
    } else if (newspecial != NS_NONE &&
	       !(pm->old->node.flags & (PM_NORESTORE|PM_READONLY))) {
	/*
	 * We need to use the special setting function to re-initialise
	 * the special parameter to empty.
	 */
	switch (PM_TYPE(pm->node.flags)) {
	case PM_SCALAR:
	    pm->gsu.s->setfn(pm, ztrdup(""));
	    break;
	case PM_INTEGER:
	    /*
	     * Restricted integers are dangerous to initialize to 0,
	     * so don't do that.
	     */
	    if (!(pm->old->node.flags & PM_RESTRICTED))
		pm->gsu.i->setfn(pm, 0);
	    break;
	case PM_EFLOAT:
	case PM_FFLOAT:
	    pm->gsu.f->setfn(pm, 0.0);
	    break;
	case PM_ARRAY:
	    pm->gsu.a->setfn(pm, mkarray(NULL));
	    break;
	case PM_HASHED:
	    pm->gsu.h->setfn(pm, newparamtable(17, pm->node.nam));
	    break;
	}
    }
    pm->node.flags |= (on & PM_READONLY);
    DPUTS(OPT_ISSET(ops,'p'), "BUG: -p not handled");

    return pm;
}

/*
 * declare, export, float, integer, local, readonly, typeset
 *
 * Note the difference in interface from most builtins, covered by the
 * BINF_ASSIGN builtin flag.  This is only made use of by builtins
 * called by reserved word, which only covers declare, local, readonly
 * and typeset.  Otherwise assigns is NULL.
 */

/**/
mod_export int
bin_typeset(char *name, char **argv, LinkList assigns, Options ops, int func)
{
    Param pm;
    Asgment asg;
    Patprog pprog;
    char *optstr = TYPESET_OPTSTR;
    int on = 0, off = 0, roff, bit = PM_ARRAY;
    int i;
    int returnval = 0, printflags = 0;
    int hasargs = *argv != NULL || (assigns && firstnode(assigns));

    /* POSIXBUILTINS is set for bash/ksh and both ignore -p with args */
    if ((func == BIN_READONLY || func == BIN_EXPORT) &&
	isset(POSIXBUILTINS) && hasargs)
	ops->ind['p'] = 0;

    /* hash -f is really the builtin `functions' */
    if (OPT_ISSET(ops,'f'))
	return bin_functions(name, argv, ops, func);

    /* POSIX handles "readonly" specially */
    if (func == BIN_READONLY && isset(POSIXBUILTINS) && !OPT_PLUS(ops, 'g'))
	ops->ind['g'] = 1;

    /* Translate the options into PM_* flags.   *
     * Unfortunately, this depends on the order *
     * these flags are defined in zsh.h         */
    for (; *optstr; optstr++, bit <<= 1)
    {
	int optval = STOUC(*optstr);
	if (OPT_MINUS(ops,optval))
	    on |= bit;
	else if (OPT_PLUS(ops,optval))
	    off |= bit;
    }
    roff = off;

    /* Sanity checks on the options.  Remove conflicting options. */
    if (on & PM_FFLOAT) {
	off |= PM_UPPER | PM_ARRAY | PM_HASHED | PM_INTEGER | PM_EFLOAT;
	/* Allow `float -F' to work even though float sets -E by default */
	on &= ~PM_EFLOAT;
    }
    if (on & PM_EFLOAT)
	off |= PM_UPPER | PM_ARRAY | PM_HASHED | PM_INTEGER | PM_FFLOAT;
    if (on & PM_INTEGER)
	off |= PM_UPPER | PM_ARRAY | PM_HASHED | PM_EFLOAT | PM_FFLOAT;
    /*
     * Allowing -Z with -L is a feature: left justify, suppressing
     * leading zeroes.
     */
    if (on & (PM_LEFT|PM_RIGHT_Z))
	off |= PM_RIGHT_B;
    if (on & PM_RIGHT_B)
	off |= PM_LEFT | PM_RIGHT_Z;
    if (on & PM_UPPER)
	off |= PM_LOWER;
    if (on & PM_LOWER)
	off |= PM_UPPER;
    if (on & PM_HASHED)
	off |= PM_ARRAY;
    if (on & PM_TIED)
	off |= PM_INTEGER | PM_EFLOAT | PM_FFLOAT | PM_ARRAY | PM_HASHED;

    on &= ~off;

    queue_signals();

    /* Given no arguments, list whatever the options specify. */
    if (OPT_ISSET(ops,'p')) {

	if (isset(POSIXBUILTINS) && SHELL_EMULATION() != EMULATE_KSH) {
	  if (func == BIN_EXPORT)
	    printflags |= PRINT_POSIX_EXPORT;
	  else if (func == BIN_READONLY)
	    printflags |= PRINT_POSIX_READONLY;
	  else
	    printflags |= PRINT_TYPESET;
	} else
	    printflags |= PRINT_TYPESET;

	if (OPT_HASARG(ops,'p')) {
	    char *eptr;
	    int pflag = (int)zstrtol(OPT_ARG(ops,'p'), &eptr, 10);
	    if (pflag == 1 && !*eptr)
		printflags |= PRINT_LINE;
	    else if (pflag || *eptr) {
		zwarnnam(name, "bad argument to -p: %s", OPT_ARG(ops,'p'));
		unqueue_signals();
		return 1;
	    }
	    /* -p0 treated as -p for consistency */
	}
    }
    if (!hasargs) {
	int exclude = 0;
	if (!OPT_ISSET(ops,'p')) {
	    if (!(on|roff))
		printflags |= PRINT_TYPE;
	    if (roff || OPT_ISSET(ops,'+'))
		printflags |= PRINT_NAMEONLY;
	} else if (printflags & (PRINT_POSIX_EXPORT|PRINT_POSIX_READONLY)) {
	    /*
	     * For POSIX export/readonly, exclude non-scalars unless
	     * explicitly requested.
	     */
	    exclude = (PM_ARRAY|PM_HASHED) & ~(on|roff);
	}
	scanhashtable(paramtab, 1, on|roff, exclude, paramtab->printnode, printflags);
	unqueue_signals();
	return 0;
    }

    if (!(OPT_ISSET(ops,'g') || OPT_ISSET(ops,'x') || OPT_ISSET(ops,'m')) ||
	OPT_PLUS(ops,'g') || *name == 'l' ||
	(!isset(GLOBALEXPORT) && !OPT_ISSET(ops,'g')))
	on |= PM_LOCAL;

    if ((on & PM_TIED) && !OPT_ISSET(ops, 'p')) {
	Param apm;
	struct asgment asg0, asg2;
	char *oldval = NULL, *joinstr;
	int joinchar, nargs;
	int already_tied = 0;

	if (OPT_ISSET(ops,'m')) {
	    zwarnnam(name, "incompatible options for -T");
	    unqueue_signals();
	    return 1;
	}
	on &= ~off;
	nargs = arrlen(argv) + (assigns ? countlinknodes(assigns) : 0);
	if (nargs < 2) {
	    zwarnnam(name, "-T requires names of scalar and array");
	    unqueue_signals();
	    return 1;
	}
	if (nargs > 3) {
	    zwarnnam(name, "too many arguments for -T");
	    unqueue_signals();
	    return 1;
	}

	if (!(asg = getasg(&argv, assigns))) {
	    unqueue_signals();
	    return 1;
	}
	asg0 = *asg;
	if (ASG_ARRAYP(&asg0)) {
	    unqueue_signals();
	    zwarnnam(name, "first argument of tie must be scalar: %s",
		     asg0.name);
	    return 1;
	}

	if (!(asg = getasg(&argv, assigns))) {
	    unqueue_signals();
	    return 1;
	}
	if (!ASG_ARRAYP(asg) && asg->value.scalar) {
	    unqueue_signals();
	    zwarnnam(name, "second argument of tie must be array: %s",
		     asg->name);
	    return 1;
	}

	if (!strcmp(asg0.name, asg->name)) {
	    unqueue_signals();
	    zerrnam(name, "can't tie a variable to itself: %s", asg0.name);
	    return 1;
	}
	if (strchr(asg0.name, '[') || strchr(asg->name, '[')) {
	    unqueue_signals();
	    zerrnam(name, "can't tie array elements: %s", asg0.name);
	    return 1;
	}
	if (ASG_VALUEP(asg) && ASG_VALUEP(&asg0)) {
	    unqueue_signals();
	    zerrnam(name, "only one tied parameter can have value: %s", asg0.name);
	    return 1;
	}

	/*
	 * Third argument, if given, is character used to join
	 * the elements of the array in the scalar.
	 */
	if (*argv)
	    joinstr = *argv;
	else if (assigns && firstnode(assigns)) {
	    Asgment nextasg = (Asgment)firstnode(assigns);
	    if (ASG_ARRAYP(nextasg) || ASG_VALUEP(nextasg)) {
		zwarnnam(name, "third argument of tie must be join character");
		unqueue_signals();
		return 1;
	    }
	    joinstr = nextasg->name;
	} else
	    joinstr = NULL;
	if (!joinstr)
	    joinchar = ':';
	else if (!*joinstr)
	    joinchar = 0;
	else if (*joinstr == Meta)
	    joinchar = joinstr[1] ^ 32;
	else
	    joinchar = *joinstr;

	pm = (Param) paramtab->getnode(paramtab, asg0.name);
	apm = (Param) paramtab->getnode(paramtab, asg->name);

	if (pm && (pm->node.flags & (PM_SPECIAL|PM_TIED)) == (PM_SPECIAL|PM_TIED)) {
	    /*
	     * Only allow typeset -T on special tied parameters if the tied
	     * parameter and join char are the same
	     */
	    if (strcmp(pm->ename, asg->name) || !(apm->node.flags & PM_SPECIAL)) {
		zwarnnam(name, "%s special parameter can only be tied to special parameter %s", asg0.name, pm->ename);
		unqueue_signals();
		return 1;
	    }
	    if (joinchar != ':') {
		zwarnnam(name, "cannot change the join character of special tied parameters");
		unqueue_signals();
		return 1;
	    }
	    already_tied = 1;
	} else if (apm && (apm->node.flags & (PM_SPECIAL|PM_TIED)) == (PM_SPECIAL|PM_TIED)) {
	    /*
	     * For the array variable, this covers attempts to tie the
	     * array to a different scalar or to the scalar after it has
	     * been made non-special
	     */
	    zwarnnam(name, "%s special parameter can only be tied to special parameter %s", asg->name, apm->ename);
	    unqueue_signals();
	    return 1;
	} else if (pm) {
	    if ((!(pm->node.flags & PM_UNSET) || pm->node.flags & PM_DECLARED)
		&& (locallevel == pm->level || !(on & PM_LOCAL))) {
		if (pm->node.flags & PM_TIED) {
		    if (PM_TYPE(pm->node.flags) != PM_SCALAR) {
			zwarnnam(name, "already tied as non-scalar: %s", asg0.name);
			unqueue_signals();
			return 1;
		    } else if (!strcmp(asg->name, pm->ename)) {
			already_tied = 1;
		    } else {
			zwarnnam(name, "can't tie already tied scalar: %s",
				 asg0.name);
			unqueue_signals();
			return 1;
		    }
		} else {
		    /*
		     * Variable already exists in the current scope but is not tied.
		     * We're preserving its value and export attribute but no other
		     * attributes upon converting to "tied".
		     */
		    if (!asg0.value.scalar && !asg->value.array &&
			!(PM_TYPE(pm->node.flags) & (PM_ARRAY|PM_HASHED)))
			oldval = ztrdup(getsparam(asg0.name));
		    on |= (pm->node.flags & ~roff) & PM_EXPORTED;
		}
	    }
	}
	if (already_tied) {
	    int ret;
	    /*
	     * If already tied, we still need to call typeset_single on
	     * both the array and colonarray, if only to update the attributes
	     * of both, and of course to set the new value if one is provided
	     * for either of them.
	     */
	    ret = !(typeset_single(name, asg0.name, pm,
				   func, on, off, roff, &asg0, apm,
				   ops, joinchar) &&
		    typeset_single(name, asg->name, apm,
				   func, (on | PM_ARRAY) & ~PM_EXPORTED,
				   off & ~PM_ARRAY, roff, asg, NULL, ops, 0)
		   );
	    unqueue_signals();
	    return ret;
	}
	/*
	 * Create the tied array; this is normal except that
	 * it has the PM_TIED flag set.  Do it first because
	 * we need the address.
	 *
	 * Don't attempt to set it yet, it's too early
	 * to be exported properly.
	 *
	 * This may create the array with PM_DEFAULTED.
	 */
	asg2.name = asg->name;
	asg2.flags = 0;
	asg2.value.array = (LinkList)0;
	if (!(apm=typeset_single(name, asg->name,
				 (Param)paramtab->getnode(paramtab,
							  asg->name),
				 func, (on | PM_ARRAY) & ~PM_EXPORTED,
				 off, roff, &asg2, NULL, ops, 0))) {
	    if (oldval)
		zsfree(oldval);
	    unqueue_signals();
	    return 1;
	}
	/*
	 * Create the tied colonarray.  We make it as a normal scalar
	 * and fix up the oddities later.
	 */
	if (!(pm=typeset_single(name, asg0.name, pm,
				func, on, off, roff, &asg0, apm,
				ops, joinchar))) {
	    if (oldval)
		zsfree(oldval);
	    unsetparam_pm(apm, 1, 1);
	    unqueue_signals();
	    return 1;
	}

	/*
	 * pm->ename is only deleted when the struct is, so
	 * we need to free it here if it already exists.
	 */
	if (pm->ename)
	    zsfree(pm->ename);
	pm->ename = ztrdup(asg->name);
	if (apm->ename)
	    zsfree(apm->ename);
	apm->ename = ztrdup(asg0.name);
	if (asg->value.array) {
	    int flags = (asg->flags & ASG_KEY_VALUE) ? ASSPM_KEY_VALUE : 0;
	    assignaparam(asg->name, zlinklist2array(asg->value.array, 1), flags);
	} else if (asg0.value.scalar || oldval) {
	    /* We have to undo what we did wrong with asg2 */
	    apm->node.flags &= ~PM_DEFAULTED;
	    if (oldval)
		assignsparam(asg0.name, oldval, 0);
	}
	unqueue_signals();

	return 0;
    }
    if (off & PM_TIED) {
	unqueue_signals();
	zerrnam(name, "use unset to remove tied variables");
	return 1;
    }

    /* With the -m option, treat arguments as glob patterns */
    if (OPT_ISSET(ops,'m')) {
	if (!OPT_ISSET(ops,'p')) {
	    if (!(on|roff))
		printflags |= PRINT_TYPE;
	    if (!on)
		printflags |= PRINT_NAMEONLY;
	}

	while ((asg = getasg(&argv, assigns))) {
	    LinkList pmlist = newlinklist();
	    LinkNode pmnode;

	    tokenize(asg->name);   /* expand argument */
	    if (!(pprog = patcompile(asg->name, 0, NULL))) {
		untokenize(asg->name);
		zwarnnam(name, "bad pattern : %s", asg->name);
		returnval = 1;
		continue;
	    }
	    if (OPT_PLUS(ops,'m') && !ASG_VALUEP(asg)) {
		scanmatchtable(paramtab, pprog, 1, on|roff, 0,
			       paramtab->printnode, printflags);
		continue;
	    }
	    /*
	     * Search through the parameter table and change all parameters
	     * matching the glob pattern to have these flags and/or value.
	     * Bad news:  if the parameter gets altered, e.g. by
	     * a type conversion, then paramtab can be shifted around,
	     * so we need to store the parameters to alter on a separate
	     * list for later use.
	     */
	    for (i = 0; i < paramtab->hsize; i++) {
		for (pm = (Param) paramtab->nodes[i]; pm;
		     pm = (Param) pm->node.next) {
		    if (((pm->node.flags & PM_RESTRICTED) && isset(RESTRICTED)) ||
			(pm->node.flags & PM_UNSET))
			continue;
		    if (pattry(pprog, pm->node.nam))
			addlinknode(pmlist, pm);
		}
	    }
	    for (pmnode = firstnode(pmlist); pmnode; incnode(pmnode)) {
		pm = (Param) getdata(pmnode);
		if (!typeset_single(name, pm->node.nam, pm, func, on, off, roff,
				    asg, NULL, ops, 0))
		    returnval = 1;
	    }
	}
	unqueue_signals();
	return returnval;
    }

    /* Take arguments literally.  Don't glob */
    while ((asg = getasg(&argv, assigns))) {
	HashNode hn = (paramtab == realparamtab ?
		       /* getnode2() to avoid autoloading */
		       paramtab->getnode2(paramtab, asg->name) :
		       paramtab->getnode(paramtab, asg->name));
	if (OPT_ISSET(ops,'p')) {
	    if (hn)
		paramtab->printnode(hn, printflags);
	    else {
		zwarnnam(name, "no such variable: %s", asg->name);
		returnval = 1;
	    }
	    continue;
	}
	if (!typeset_single(name, asg->name, (Param)hn,
			    func, on, off, roff, asg, NULL,
			    ops, 0))
	    returnval = 1;
    }
    unqueue_signals();
    return returnval;
}

/* Helper for bin_functions() when run as "autoload -X" */

/**/
int
eval_autoload(Shfunc shf, char *name, Options ops, int func)
{
    if (!(shf->node.flags & PM_UNDEFINED))
	return 1;

    if (shf->funcdef) {
	freeeprog(shf->funcdef);
	shf->funcdef = &dummy_eprog;
    }
    if (OPT_MINUS(ops,'X')) {
	char *fargv[3];
	fargv[0] = quotestring(name, QT_SINGLE_OPTIONAL);
	fargv[1] = "\"$@\"";
	fargv[2] = 0;
	shf->funcdef = mkautofn(shf);
	return bin_eval(name, fargv, ops, func);
    }

    return !loadautofn(shf, (OPT_ISSET(ops,'k') ? 2 :
			     (OPT_ISSET(ops,'z') ? 0 : 1)), 1,
		       OPT_ISSET(ops,'d'));
}

/* Helper for bin_functions() for -X and -r options */

/**/
static int
check_autoload(Shfunc shf, char *name, Options ops, int func)
{
    if (OPT_ISSET(ops,'X'))
    {
	return eval_autoload(shf, name, ops, func);
    }
    if ((OPT_ISSET(ops,'r') || OPT_ISSET(ops,'R')) &&
	(shf->node.flags & PM_UNDEFINED))
    {
	char *dir_path;
	if (shf->filename && (shf->node.flags & PM_LOADDIR)) {
	    char *spec_path[2];
	    spec_path[0] = shf->filename;
	    spec_path[1] = NULL;
	    if (getfpfunc(shf->node.nam, NULL, &dir_path, spec_path, 1)) {
		/* shf->filename is already correct. */
		return 0;
	    }
	    if (!OPT_ISSET(ops,'d')) {
		if (OPT_ISSET(ops,'R')) {
		    zerr("%s: function definition file not found",
			 shf->node.nam);
		    return 1;
		}
		return 0;
	    }
	}
	if (getfpfunc(shf->node.nam, NULL, &dir_path, NULL, 1)) {
	    dircache_set(&shf->filename, NULL);
	    if (*dir_path != '/') {
		dir_path = zhtricat(metafy(zgetcwd(), -1, META_HEAPDUP),
				    "/", dir_path);
		dir_path = xsymlink(dir_path, 1);
	    }
	    dircache_set(&shf->filename, dir_path);
	    shf->node.flags |= PM_LOADDIR;
	    return 0;
	}
	if (OPT_ISSET(ops,'R')) {
	    zerr("%s: function definition file not found",
		 shf->node.nam);
	    return 1;
	}
	/* with -r, we don't flag an error, just let it be found later. */
    }
    return 0;
}

/* List a user-defined math function. */
static void
listusermathfunc(MathFunc p)
{
    int showargs;

    if (p->module)
	showargs = 3;
    else if (p->maxargs != (p->minargs ? p->minargs : -1))
	showargs = 2;
    else if (p->minargs)
	showargs = 1;
    else
	showargs = 0;

    printf("functions -M%s %s", (p->flags & MFF_STR) ? "s" : "", p->name);
    if (showargs) {
	printf(" %d", p->minargs);
	showargs--;
    }
    if (showargs) {
	printf(" %d", p->maxargs);
	showargs--;
    }
    if (showargs) {
	/*
	 * function names are not required to consist of ident characters
	 */
	putchar(' ');
	quotedzputs(p->module, stdout);
	showargs--;
    }
    putchar('\n');
}


static void
add_autoload_function(Shfunc shf, char *funcname)
{
    char *nam;
    if (*funcname == '/' && funcname[1] &&
	(nam = strrchr(funcname, '/')) && nam[1] &&
	(shf->node.flags & PM_UNDEFINED)) {
	char *dir;
	nam = strrchr(funcname, '/');
	if (nam == funcname) {
	    dir = "/";
	} else {
	    *nam++ = '\0';
	    dir = funcname;
	}
	dircache_set(&shf->filename, NULL);
	dircache_set(&shf->filename, dir);
	shf->node.flags |= PM_LOADDIR;
	shf->node.flags |= PM_ABSPATH_USED;
	shfunctab->addnode(shfunctab, ztrdup(nam), shf);
    } else {
        Shfunc shf2;
        Funcstack fs;
        const char *calling_f = NULL;
        char buf[PATH_MAX+1];

        /* Find calling function */
        for (fs = funcstack; fs; fs = fs->prev) {
            if (fs->tp == FS_FUNC && fs->name && (!shf->node.nam || 0 != strcmp(fs->name,shf->node.nam))) {
                calling_f = fs->name;
                break;
            }
        }

        /* Get its directory */
        if (calling_f) {
            /* Should contain load directory, and be loaded via absolute path */
            if ((shf2 = (Shfunc) shfunctab->getnode2(shfunctab, calling_f))
                    && (shf2->node.flags & PM_LOADDIR) && (shf2->node.flags & PM_ABSPATH_USED)
                    && shf2->filename)
            {
                if (strlen(shf2->filename) + strlen(funcname) + 1 < PATH_MAX)
                {
                    sprintf(buf, "%s/%s", shf2->filename, funcname);
                    /* Set containing directory if the function file
                     * exists (do normal FPATH processing otherwise) */
                    if (!access(buf, R_OK)) {
                        dircache_set(&shf->filename, NULL);
                        dircache_set(&shf->filename, shf2->filename);
                        shf->node.flags |= PM_LOADDIR;
                        shf->node.flags |= PM_ABSPATH_USED;
                    }
                }
            }
        }

	shfunctab->addnode(shfunctab, ztrdup(funcname), shf);
    }
}

/* Display or change the attributes of shell functions.   *
 * If called as autoload, it will define a new autoloaded *
 * (undefined) shell function.                            */

/**/
int
bin_functions(char *name, char **argv, Options ops, int func)
{
    Patprog pprog;
    Shfunc shf;
    int i, returnval = 0;
    int on = 0, off = 0, pflags = 0, roff, expand = 0;

    /* Do we have any flags defined? */
    if (OPT_PLUS(ops,'u'))
	off |= PM_UNDEFINED;
    else if (OPT_MINUS(ops,'u') || OPT_ISSET(ops,'X'))
	on |= PM_UNDEFINED;
    if (OPT_MINUS(ops,'U'))
	on |= PM_UNALIASED|PM_UNDEFINED;
    else if (OPT_PLUS(ops,'U'))
	off |= PM_UNALIASED;
    if (OPT_MINUS(ops,'t'))
	on |= PM_TAGGED;
    else if (OPT_PLUS(ops,'t'))
	off |= PM_TAGGED;
    if (OPT_MINUS(ops,'T'))
	on |= PM_TAGGED_LOCAL;
    else if (OPT_PLUS(ops,'T'))
	off |= PM_TAGGED_LOCAL;
    if (OPT_MINUS(ops,'W'))
	on |= PM_WARNNESTED;
    else if (OPT_PLUS(ops,'W'))
	off |= PM_WARNNESTED;
    roff = off;
    if (OPT_MINUS(ops,'z')) {
	on |= PM_ZSHSTORED;
	off |= PM_KSHSTORED;
    } else if (OPT_PLUS(ops,'z')) {
	off |= PM_ZSHSTORED;
	roff |= PM_ZSHSTORED;
    }
    if (OPT_MINUS(ops,'k')) {
	on |= PM_KSHSTORED;
	off |= PM_ZSHSTORED;
    } else if (OPT_PLUS(ops,'k')) {
	off |= PM_KSHSTORED;
	roff |= PM_KSHSTORED;
    }
    if (OPT_MINUS(ops,'d')) {
	on |= PM_CUR_FPATH;
	off |= PM_CUR_FPATH;
    } else if (OPT_PLUS(ops,'d')) {
	off |= PM_CUR_FPATH;
	roff |= PM_CUR_FPATH;
    }

    if ((off & PM_UNDEFINED) || (OPT_ISSET(ops,'k') && OPT_ISSET(ops,'z')) ||
	(OPT_ISSET(ops,'x') && !OPT_HASARG(ops,'x')) ||
	(OPT_MINUS(ops,'X') && (OPT_ISSET(ops,'m') || !scriptname)) ||
	(OPT_ISSET(ops,'c') && (OPT_ISSET(ops,'x') || OPT_ISSET(ops,'X') ||
				OPT_ISSET(ops,'m')))) {
	zwarnnam(name, "invalid option(s)");
	return 1;
    }

    if (OPT_ISSET(ops,'c')) {
	Shfunc newsh;
	if (!*argv || !argv[1] || argv[2]) {
	    zwarnnam(name, "-c: requires two arguments");
	    return 1;
	}
	shf = (Shfunc) shfunctab->getnode(shfunctab, *argv);
	if (!shf) {
	    zwarnnam(name, "no such function: %s", *argv);
	    return 1;
	}
	if (shf->node.flags & PM_UNDEFINED) {
	    if (shf->funcdef) {
		freeeprog(shf->funcdef);
		shf->funcdef = &dummy_eprog;
	    }
	    shf = loadautofn(shf, 1, 0, 0);
	    if (!shf)
		return 1;
	}
	newsh = zalloc(sizeof(*newsh));
	memcpy(newsh, shf, sizeof(*newsh));
	if (newsh->node.flags & PM_LOADDIR) {
	    /* Expand original location of autoloaded file */
	    newsh->node.flags &= ~PM_LOADDIR;
	    newsh->filename = tricat(shf->filename, "/", shf->node.nam);
	} else
	    newsh->filename = ztrdup(shf->filename);
	newsh->funcdef->nref++;
	if (newsh->redir)
	    newsh->redir->nref++;
	if (shf->sticky)
	    newsh->sticky = sticky_emulation_dup(sticky, 0);
	shfunctab->addnode(shfunctab, ztrdup(argv[1]), &newsh->node);
	return 0;
    }

    if (OPT_ISSET(ops,'x')) {
	char *eptr;
	expand = (int)zstrtol(OPT_ARG(ops,'x'), &eptr, 10);
	if (*eptr) {
	    zwarnnam(name, "number expected after -x");
	    return 1;
	}
	if (expand == 0)	/* no indentation at all */
	    expand = -1;
    }

    if (OPT_PLUS(ops,'f') || roff || OPT_ISSET(ops,'+'))
	pflags |= PRINT_NAMEONLY;

    if (OPT_MINUS(ops,'M') || OPT_PLUS(ops,'M')) {
	MathFunc p, q, prev;
	/*
	 * Add/remove/list function as mathematical.
	 */
	if (on || off || pflags || OPT_ISSET(ops,'X') || OPT_ISSET(ops,'u')
	    || OPT_ISSET(ops,'U') || OPT_ISSET(ops,'w')) {
	    zwarnnam(name, "invalid option(s)");
	    return 1;
	}
	if (!*argv) {
	    /* List functions. */
	    queue_signals();
	    for (p = mathfuncs; p; p = p->next)
		if (p->flags & MFF_USERFUNC)
		    listusermathfunc(p);
	    unqueue_signals();
	} else if (OPT_ISSET(ops,'m')) {
	    /* List matching functions. */
	    for (; *argv; argv++) {
		queue_signals();
		tokenize(*argv);
		if ((pprog = patcompile(*argv, PAT_STATIC, 0))) {
		    for (p = mathfuncs, q = NULL; p; q = p) {
			MathFunc next;
			do {
			    next = NULL;
			    if ((p->flags & MFF_USERFUNC) &&
				pattry(pprog, p->name)) {
				if (OPT_PLUS(ops,'M')) {
				    next = p->next;
				    removemathfunc(q, p);
				    p = next;
				} else
				    listusermathfunc(p);
			    }
			    /* if we deleted one, retry with the new p */
			} while (next);
			if (p)
			    p = p->next;
		    }
		} else {
		    untokenize(*argv);
		    zwarnnam(name, "bad pattern : %s", *argv);
		    returnval = 1;
		}
		unqueue_signals();
	    }
	} else if (OPT_PLUS(ops,'M')) {
	    /* Delete functions. -m is allowed but is handled above. */
	    for (; *argv; argv++) {
		queue_signals();
		for (p = mathfuncs, q = NULL; p; q = p, p = p->next) {
		    if (!strcmp(p->name, *argv)) {
			if (!(p->flags & MFF_USERFUNC)) {
			    zwarnnam(name, "+M %s: is a library function",
				     *argv);
			    returnval = 1;
			    break;
			}
			removemathfunc(q, p);
			break;
		    }
		}
		unqueue_signals();
	    }
	} else {
	    /* Add a function */
	    int minargs, maxargs;
	    char *funcname = *argv++;
	    char *modname = NULL;
	    char *ptr;

	    if (OPT_ISSET(ops,'s')) {
		minargs = maxargs = 1;
	    } else {
		minargs = 0;
		maxargs = -1;
	    }

	    ptr = itype_end(funcname, IIDENT, 0);
	    if (idigit(*funcname) || funcname == ptr || *ptr) {
		zwarnnam(name, "-M %s: bad math function name", funcname);
		return 1;
	    }

	    if (*argv) {
		minargs = (int)zstrtol(*argv, &ptr, 0);
		if (minargs < 0 || *ptr) {
		    zwarnnam(name, "-M: invalid min number of arguments: %s",
			     *argv);
		    return 1;
		}
		if (OPT_ISSET(ops,'s') && minargs != 1) {
		    zwarnnam(name, "-Ms: must take a single string argument");
		    return 1;
		}
		maxargs = minargs;
		argv++;
	    }
	    if (*argv) {
		maxargs = (int)zstrtol(*argv, &ptr, 0);
		if (maxargs < -1 ||
		    (maxargs != -1 && maxargs < minargs) ||
		    *ptr) {
		    zwarnnam(name,
			     "-M: invalid max number of arguments: %s",
			     *argv);
		    return 1;
		}
		if (OPT_ISSET(ops,'s') && maxargs != 1) {
		    zwarnnam(name, "-Ms: must take a single string argument");
		    return 1;
		}
		argv++;
	    }
	    if (*argv)
		modname = *argv++;
	    if (*argv) {
		zwarnnam(name, "-M: too many arguments");
		return 1;
	    }

	    p = (MathFunc)zshcalloc(sizeof(struct mathfunc));
	    p->name = ztrdup(funcname);
	    p->flags = MFF_USERFUNC;
	    if (OPT_ISSET(ops,'s'))
		p->flags |= MFF_STR;
	    p->module = modname ? ztrdup(modname) : NULL;
	    p->minargs = minargs;
	    p->maxargs = maxargs;

	    queue_signals();
	    for (q = mathfuncs, prev = NULL; q; prev = q, q = q->next) {
		if (!strcmp(q->name, funcname)) {
		    removemathfunc(prev, q);
		    break;
		}
	    }

	    p->next = mathfuncs;
	    mathfuncs = p;
	    unqueue_signals();
	}

	return returnval;
    }

    if (OPT_MINUS(ops,'X')) {
	Funcstack fs;
	char *funcname = NULL;
	int ret;
	if (*argv && argv[1]) {
	    zwarnnam(name, "-X: too many arguments");
	    return 1;
	}
	queue_signals();
	for (fs = funcstack; fs; fs = fs->prev) {
	    if (fs->tp == FS_FUNC) {
		/*
		 * dupstring here is paranoia but unlikely to be
		 * problematic
		 */
		funcname = dupstring(fs->name);
		break;
	    }
	}
	if (!funcname)
	{
	    zerrnam(name, "bad autoload");
	    ret = 1;
	} else {
	    if ((shf = (Shfunc) shfunctab->getnode(shfunctab, funcname))) {
		DPUTS(!shf->funcdef,
		      "BUG: Calling autoload from empty function");
	    } else {
		shf = (Shfunc) zshcalloc(sizeof *shf);
		shfunctab->addnode(shfunctab, ztrdup(funcname), shf);
	    }
	    if (*argv) {
		dircache_set(&shf->filename, NULL);
		dircache_set(&shf->filename, *argv);
		on |= PM_LOADDIR;
	    }
	    shf->node.flags = on;
	    ret = eval_autoload(shf, funcname, ops, func);
	}
	unqueue_signals();
	return ret;
    } else if (!*argv) {
	/* If no arguments given, we will print functions.  If flags *
	 * are given, we will print only functions containing these  *
	 * flags, else we'll print them all.                         */
	int ret = 0;

	queue_signals();
	if (OPT_ISSET(ops,'U') && !OPT_ISSET(ops,'u'))
		on &= ~PM_UNDEFINED;
	    scanshfunc(1, on|off, DISABLED, shfunctab->printnode,
		       pflags, expand);
	unqueue_signals();
	return ret;
    }

    /* With the -m option, treat arguments as glob patterns */
    if (OPT_ISSET(ops,'m')) {
	on &= ~PM_UNDEFINED;
	for (; *argv; argv++) {
	    queue_signals();
	    /* expand argument */
	    tokenize(*argv);
	    if ((pprog = patcompile(*argv, PAT_STATIC, 0))) {
		/* with no options, just print all functions matching the glob pattern */
		if (!(on|off) && !OPT_ISSET(ops,'X')) {
		    scanmatchshfunc(pprog, 1, 0, DISABLED,
				   shfunctab->printnode, pflags, expand);
		} else {
		    /* apply the options to all functions matching the glob pattern */
		    for (i = 0; i < shfunctab->hsize; i++) {
			for (shf = (Shfunc) shfunctab->nodes[i]; shf;
			     shf = (Shfunc) shf->node.next)
			    if (pattry(pprog, shf->node.nam) &&
				!(shf->node.flags & DISABLED)) {
				shf->node.flags = (shf->node.flags |
					      (on & ~PM_UNDEFINED)) & ~off;
				if (check_autoload(shf, shf->node.nam,
						   ops, func)) {
				    returnval = 1;
				}
			    }
		    }
		}
	    } else {
		untokenize(*argv);
		zwarnnam(name, "bad pattern : %s", *argv);
		returnval = 1;
	    }
	    unqueue_signals();
	}
	return returnval;
    }

    /* Take the arguments literally -- do not glob */
    queue_signals();
    for (; *argv; argv++) {
	if (OPT_ISSET(ops,'w'))
	    returnval = dump_autoload(name, *argv, on, ops, func);
	else if ((shf = (Shfunc) shfunctab->getnode(shfunctab, *argv))) {
	    /* if any flag was given */
	    if (on|off) {
		/* turn on/off the given flags */
		shf->node.flags = (shf->node.flags | (on & ~PM_UNDEFINED)) & ~off;
		if (check_autoload(shf, shf->node.nam, ops, func))
		    returnval = 1;
	    } else
		/* no flags, so just print */
		printshfuncexpand(&shf->node, pflags, expand);
	} else if (on & PM_UNDEFINED) {
	    int signum = -1, ok = 1;

	    if (!strncmp(*argv, "TRAP", 4) &&
		(signum = getsignum(*argv + 4)) != -1) {
		/*
		 * Because of the possibility of alternative names,
		 * we must remove the trap explicitly.
		 */
		removetrapnode(signum);
	    }

	    if (**argv == '/') {
		char *base = strrchr(*argv, '/') + 1;
		if (*base &&
		    (shf = (Shfunc) shfunctab->getnode(shfunctab, base))) {
		    char *dir;
		    /* turn on/off the given flags */
		    shf->node.flags =
			(shf->node.flags | (on & ~PM_UNDEFINED)) & ~off;
		    if (shf->node.flags & PM_UNDEFINED) {
			/* update path if not yet loaded */
			if (base == *argv + 1)
			    dir = "/";
			else {
			    dir = *argv;
			    base[-1] = '\0';
			}
			dircache_set(&shf->filename, NULL);
			dircache_set(&shf->filename, dir);
		    }
		    if (check_autoload(shf, shf->node.nam, ops, func))
			returnval = 1;
		    continue;
		}
	    }

	    /* Add a new undefined (autoloaded) function to the *
	     * hash table with the corresponding flags set.     */
	    shf = (Shfunc) zshcalloc(sizeof *shf);
	    shf->node.flags = on;
	    shf->funcdef = mkautofn(shf);
	    shfunc_set_sticky(shf);
	    add_autoload_function(shf, *argv);

	    if (signum != -1) {
		if (settrap(signum, NULL, ZSIG_FUNC)) {
		    shfunctab->removenode(shfunctab, *argv);
		    shfunctab->freenode(&shf->node);
		    returnval = 1;
		    ok = 0;
		}
	    }

	    if (ok && check_autoload(shf, shf->node.nam, ops, func))
		returnval = 1;
	} else
	    returnval = 1;
    }
    unqueue_signals();
    return returnval;
}

/**/
Eprog
mkautofn(Shfunc shf)
{
    Eprog p;

    p = (Eprog) zalloc(sizeof(*p));
    p->len = 5 * sizeof(wordcode);
    p->prog = (Wordcode) zalloc(p->len);
    p->strs = NULL;
    p->shf = shf;
    p->npats = 0;
    p->nref = 1; /* allocated from permanent storage */
    p->pats = (Patprog *) p->prog;
    p->flags = EF_REAL;
    p->dump = NULL;

    p->prog[0] = WCB_LIST((Z_SYNC | Z_END), 0);
    p->prog[1] = WCB_SUBLIST(WC_SUBLIST_END, 0, 3);
    p->prog[2] = WCB_PIPE(WC_PIPE_END, 0);
    p->prog[3] = WCB_AUTOFN();
    p->prog[4] = WCB_END();

    return p;
}

/* unset: unset parameters */

/**/
int
bin_unset(char *name, char **argv, Options ops, int func)
{
    Param pm, next;
    Patprog pprog;
    char *s;
    int match = 0, returnval = 0;
    int i;

    /* unset -f is the same as unfunction */
    if (OPT_ISSET(ops,'f'))
	return bin_unhash(name, argv, ops, func);

    /* with -m option, treat arguments as glob patterns */
    if (OPT_ISSET(ops,'m')) {
	while ((s = *argv++)) {
	    queue_signals();
	    /* expand */
	    tokenize(s);
	    if ((pprog = patcompile(s, PAT_STATIC, NULL))) {
		/* Go through the parameter table, and unset any matches */
		for (i = 0; i < paramtab->hsize; i++) {
		    for (pm = (Param) paramtab->nodes[i]; pm; pm = next) {
			/* record pointer to next, since we may free this one */
			next = (Param) pm->node.next;
			if ((!(pm->node.flags & PM_RESTRICTED) ||
			     unset(RESTRICTED)) &&
			    pattry(pprog, pm->node.nam)) {
			    unsetparam_pm(pm, 0, 1);
			    match++;
			}
		    }
		}
	    } else {
		untokenize(s);
		zwarnnam(name, "bad pattern : %s", s);
		returnval = 1;
	    }
	    unqueue_signals();
	}
	/* If we didn't match anything, we return 1. */
	if (!match)
	    returnval = 1;
	return returnval;
    }

    /* do not glob -- unset the given parameter */
    queue_signals();
    while ((s = *argv++)) {
	char *ss = strchr(s, '['), *subscript = 0;
	if (ss) {
	    char *sse = ss + strlen(ss)-1;
	    *ss = 0;
	    if (*sse == ']') {
		*sse = 0;
		subscript = dupstring(ss+1);
		*sse = ']';
	    }
	}
	if ((ss && !subscript) || !isident(s)) {
	    if (ss)
		*ss = '[';
	    zerrnam(name, "%s: invalid parameter name", s);
	    returnval = 1;
	    continue;
	}
	pm = (Param) (paramtab == realparamtab ?
		      /* getnode2() to avoid autoloading */
		      paramtab->getnode2(paramtab, s) :
		      paramtab->getnode(paramtab, s));
	/*
	 * Unsetting an unset variable is not an error.
	 * This appears to be reasonably standard behaviour.
	 */
	if (!pm)
	    continue;
	else if ((pm->node.flags & PM_RESTRICTED) && isset(RESTRICTED)) {
	    zerrnam(name, "%s: restricted", pm->node.nam);
	    returnval = 1;
	} else if (ss) {
	    if (PM_TYPE(pm->node.flags) == PM_HASHED) {
		HashTable tht = paramtab;
		if ((paramtab = pm->gsu.h->getfn(pm)))
		    unsetparam(subscript);
		paramtab = tht;
	    } else if (PM_TYPE(pm->node.flags) == PM_SCALAR ||
		       PM_TYPE(pm->node.flags) == PM_ARRAY) {
		struct value vbuf;
		vbuf.isarr = (PM_TYPE(pm->node.flags) == PM_ARRAY ?
			      SCANPM_ARRONLY : 0);
		vbuf.pm = pm;
		vbuf.flags = 0;
		vbuf.start = 0;
		vbuf.end = -1;
		vbuf.arr = 0;
		*ss = '[';
		if (getindex(&ss, &vbuf, SCANPM_ASSIGNING) == 0 &&
		    vbuf.pm && !(vbuf.pm->node.flags & PM_UNSET)) {
		    if (PM_TYPE(pm->node.flags) == PM_SCALAR) {
			setstrvalue(&vbuf, ztrdup(""));
		    } else {
			/* start is after the element for reverse index */
			int start = vbuf.start - !!(vbuf.flags & VALFLAG_INV);
			if (arrlen_gt(vbuf.pm->u.arr, start)) {
			    char *arr[2];
			    arr[0] = "";
			    arr[1] = 0;
			    setarrvalue(&vbuf, zarrdup(arr));
			}
		    }
		}
		returnval = errflag;
		errflag &= ~ERRFLAG_ERROR;
	    } else {
		zerrnam(name, "%s: invalid element for unset", s);
		returnval = 1;
	    }
	} else {
	    if (unsetparam_pm(pm, 0, 1))
		returnval = 1;
	}
	if (ss)
	    *ss = '[';
    }
    unqueue_signals();
    return returnval;
}

/* type, whence, which, command */

static LinkList matchednodes;

static void
fetchcmdnamnode(HashNode hn, UNUSED(int printflags))
{
    Cmdnam cn = (Cmdnam) hn;
    addlinknode(matchednodes, cn->node.nam);
}

/**/
int
bin_whence(char *nam, char **argv, Options ops, int func)
{
    HashNode hn;
    Patprog pprog;
    int returnval = 0;
    int printflags = 0;
    int aliasflags;
    int csh, all, v, wd;
    int informed = 0;
    int expand = 0;
    char *cnam, **allmatched = 0;

    /* Check some option information */
    csh = OPT_ISSET(ops,'c');
    v   = OPT_ISSET(ops,'v');
    all = OPT_ISSET(ops,'a');
    wd  = OPT_ISSET(ops,'w');

    if (OPT_ISSET(ops,'x')) {
	char *eptr;
	expand = (int)zstrtol(OPT_ARG(ops,'x'), &eptr, 10);
	if (*eptr) {
	    zwarnnam(nam, "number expected after -x");
	    return 1;
	}
	if (expand == 0)	/* no indentation at all */
	    expand = -1;
    }

    if (OPT_ISSET(ops,'w'))
	printflags |= PRINT_WHENCE_WORD;
    else if (OPT_ISSET(ops,'c'))
	printflags |= PRINT_WHENCE_CSH;
    else if (OPT_ISSET(ops,'v'))
	printflags |= PRINT_WHENCE_VERBOSE;
    else
	printflags |= PRINT_WHENCE_SIMPLE;
    if (OPT_ISSET(ops,'f'))
	printflags |= PRINT_WHENCE_FUNCDEF;

    if (func == BIN_COMMAND)
	if (OPT_ISSET(ops,'V')) {
	    printflags = aliasflags = PRINT_WHENCE_VERBOSE;
	    v = 1;
	} else {
	    aliasflags = PRINT_LIST;
	    printflags = PRINT_WHENCE_SIMPLE;
	    v = 0;
	}
    else
	aliasflags = printflags;

    /* With -m option -- treat arguments as a glob patterns */
    if (OPT_ISSET(ops,'m')) {
	cmdnamtab->filltable(cmdnamtab);
	if (all) {
	    pushheap();
	    matchednodes = newlinklist();
	}
	queue_signals();
	for (; *argv; argv++) {
	    /* parse the pattern */
	    tokenize(*argv);
	    if (!(pprog = patcompile(*argv, PAT_STATIC, NULL))) {
		untokenize(*argv);
		zwarnnam(nam, "bad pattern : %s", *argv);
		returnval = 1;
		continue;
	    }
	    if (!OPT_ISSET(ops,'p')) {
		/* -p option is for path search only.    *
		 * We're not using it, so search for ... */

		/* aliases ... */
		informed +=
		scanmatchtable(aliastab, pprog, 1, 0, DISABLED,
			       aliastab->printnode, printflags);

		/* and reserved words ... */
		informed +=
		scanmatchtable(reswdtab, pprog, 1, 0, DISABLED,
			       reswdtab->printnode, printflags);

		/* and shell functions... */
		informed +=
		scanmatchshfunc(pprog, 1, 0, DISABLED,
			       shfunctab->printnode, printflags, expand);

		/* and builtins. */
		informed +=
		scanmatchtable(builtintab, pprog, 1, 0, DISABLED,
			       builtintab->printnode, printflags);
	    }
	    /* Done search for `internal' commands, if the -p option *
	     * was not used.  Now search the path.                   */
	    informed +=
	    scanmatchtable(cmdnamtab, pprog, 1, 0, 0,
			   (all ? fetchcmdnamnode : cmdnamtab->printnode),
			   printflags);
	    run_queued_signals();
	}
	unqueue_signals();
	if (all) {
	    allmatched = argv = zlinklist2array(matchednodes, 1);
	    matchednodes = NULL;
	    popheap();
	} else
	    return returnval || !informed;
    }

    /* Take arguments literally -- do not glob */
    queue_signals();
    for (; *argv; argv++) {
	if (!OPT_ISSET(ops,'p') && !allmatched) {
	    char *suf;

	    /* Look for alias */
	    if ((hn = aliastab->getnode(aliastab, *argv))) {
		aliastab->printnode(hn, aliasflags);
		informed = 1;
		if (!all)
		    continue;
	    }
	    /* Look for suffix alias */
	    if ((suf = strrchr(*argv, '.')) && suf[1] &&
		suf > *argv && suf[-1] != Meta &&
		(hn = sufaliastab->getnode(sufaliastab, suf+1))) {
		sufaliastab->printnode(hn, printflags);
		informed = 1;
		if (!all)
		    continue;
	    }
	    /* Look for reserved word */
	    if ((hn = reswdtab->getnode(reswdtab, *argv))) {
		reswdtab->printnode(hn, printflags);
		informed = 1;
		if (!all)
		    continue;
	    }
	    /* Look for shell function */
	    if ((hn = shfunctab->getnode(shfunctab, *argv))) {
		printshfuncexpand(hn, printflags, expand);
		informed = 1;
		if (!all)
		    continue;
	    }
	    /* Look for builtin command */
	    if ((hn = builtintab->getnode(builtintab, *argv))) {
		builtintab->printnode(hn, printflags);
		informed = 1;
		if (!all)
		    continue;
	    }
	    /* Look for commands that have been added to the *
	     * cmdnamtab with the builtin `hash foo=bar'.    */
	    if ((hn = cmdnamtab->getnode(cmdnamtab, *argv)) && (hn->flags & HASHED)) {
		cmdnamtab->printnode(hn, printflags);
		informed = 1;
		if (!all)
		    continue;
	    }
	}

	/* Option -a is to search the entire path, *
	 * rather than just looking for one match. */
	if (all && **argv != '/') {
	    char **pp, *buf;

	    pushheap();
	    for (pp = path; *pp; pp++) {
		if (**pp) {
		    buf = zhtricat(*pp, "/", *argv);
		} else buf = dupstring(*argv);

		if (iscom(buf)) {
		    if (wd) {
			printf("%s: command\n", *argv);
		    } else {
			if (v && !csh) {
			    zputs(*argv, stdout), fputs(" is ", stdout);
			    quotedzputs(buf, stdout);
			} else
			    zputs(buf, stdout);
			if (OPT_ISSET(ops,'s') || OPT_ISSET(ops, 'S'))
			    print_if_link(buf, OPT_ISSET(ops, 'S'));
			fputc('\n', stdout);
		    }
		    informed = 1;
		}
	    }
	    if (!informed && (wd || v || csh)) {
		/* this is information and not an error so, as in csh, use stdout */
		zputs(*argv, stdout);
		puts(wd ? ": none" : " not found");
		returnval = 1;
	    }
	    popheap();
	} else if (func == BIN_COMMAND && OPT_ISSET(ops,'p') &&
		   (hn = builtintab->getnode(builtintab, *argv))) {
	    /*
	     * Special case for "command -p[vV]" which needs to
	     * show a builtin in preference to an external command.
	     */
	    builtintab->printnode(hn, printflags);
	    informed = 1;
	} else if ((cnam = findcmd(*argv, 1,
				   func == BIN_COMMAND &&
				   OPT_ISSET(ops,'p')))) {
	    /* Found external command. */
	    if (wd) {
		printf("%s: command\n", *argv);
	    } else {
		if (v && !csh) {
		    zputs(*argv, stdout), fputs(" is ", stdout);
		    quotedzputs(cnam, stdout);
		} else
		    zputs(cnam, stdout);
		if (OPT_ISSET(ops,'s') || OPT_ISSET(ops,'S'))
		    print_if_link(cnam, OPT_ISSET(ops,'S'));
		fputc('\n', stdout);
	    }
	    informed = 1;
	} else {
	    /* Not found at all. That's not an error as such so this goes to stdout */
	    if (v || csh || wd)
		zputs(*argv, stdout), puts(wd ? ": none" : " not found");
	    returnval = 1;
	}
    }
    if (allmatched)
	freearray(allmatched);

    unqueue_signals();
    return returnval || !informed;
}

/**** command & named directory hash table builtins ****/

/*****************************************************************
 * hash -- explicitly hash a command.                            *
 * 1) Given no arguments, list the hash table.                   *
 * 2) The -m option prints out commands in the hash table that   *
 *    match a given glob pattern.                                *
 * 3) The -f option causes the entire path to be added to the    *
 *    hash table (cannot be combined with any arguments).        *
 * 4) The -r option causes the entire hash table to be discarded *
 *    (cannot be combined with any arguments).                   *
 * 5) Given argument of the form foo=bar, add element to command *
 *    hash table, so that when `foo' is entered, then `bar' is   *
 *    executed.                                                  *
 * 6) Given arguments not of the previous form, add it to the    *
 *    command hash table as if it were being executed.           *
 * 7) The -d option causes analogous things to be done using     *
 *    the named directory hash table.                            *
 *****************************************************************/

/**/
int
bin_hash(char *name, char **argv, Options ops, UNUSED(int func))
{
    HashTable ht;
    Patprog pprog;
    Asgment asg;
    int returnval = 0;
    int printflags = 0;

    if (OPT_ISSET(ops,'d'))
	ht = nameddirtab;
    else
	ht = cmdnamtab;

    if (OPT_ISSET(ops,'r') || OPT_ISSET(ops,'f')) {
	/* -f and -r can't be used with any arguments */
	if (*argv) {
	    zwarnnam("hash", "too many arguments");
	    return 1;
	}

	/* empty the hash table */
	if (OPT_ISSET(ops,'r'))
	    ht->emptytable(ht);

	/* fill the hash table in a standard way */
	if (OPT_ISSET(ops,'f'))
	    ht->filltable(ht);

	return 0;
    }

    if (OPT_ISSET(ops,'L')) printflags |= PRINT_LIST;

    /* Given no arguments, display current hash table. */
    if (!*argv) {
	queue_signals();
	scanhashtable(ht, 1, 0, 0, ht->printnode, printflags);
	unqueue_signals();
	return 0;
    }

    queue_signals();
    while (*argv) {
	void *hn;
	if (OPT_ISSET(ops,'m')) {
	    /* with the -m option, treat the argument as a glob pattern */
	    tokenize(*argv);  /* expand */
	    if ((pprog = patcompile(*argv, PAT_STATIC, NULL))) {
		/* display matching hash table elements */
		scanmatchtable(ht, pprog, 1, 0, 0, ht->printnode, printflags);
	    } else {
		untokenize(*argv);
		zwarnnam(name, "bad pattern : %s", *argv);
		returnval = 1;
	    }
	    argv++;
            continue;
	}
        if (!(asg = getasg(&argv, NULL))) {
	    zwarnnam(name, "bad assignment");
	    returnval = 1;
	    break;
        } else if (ASG_VALUEP(asg)) {
	    if(isset(RESTRICTED)) {
		zwarnnam(name, "restricted: %s", asg->value.scalar);
		returnval = 1;
	    } else {
		/* The argument is of the form foo=bar, *
		 * so define an entry for the table.    */
		if(OPT_ISSET(ops,'d')) {
		    /* shouldn't return NULL if asg->name is not NULL */
		    if (*itype_end(asg->name, IUSER, 0)) {
			zwarnnam(name,
				 "invalid character in directory name: %s",
				 asg->name);
			returnval = 1;
			continue;
		    } else {
			Nameddir nd = hn = zshcalloc(sizeof *nd);
			nd->node.flags = 0;
			nd->dir = ztrdup(asg->value.scalar);
		    }
		} else {
		    Cmdnam cn = hn = zshcalloc(sizeof *cn);
		    cn->node.flags = HASHED;
		    cn->u.cmd = ztrdup(asg->value.scalar);
		}
		ht->addnode(ht, ztrdup(asg->name), hn);
		if(OPT_ISSET(ops,'v'))
		    ht->printnode(hn, 0);
	    }
	} else if (!(hn = ht->getnode2(ht, asg->name))) {
	    /* With no `=value' part to the argument, *
	     * work out what it ought to be.          */
	    if(OPT_ISSET(ops,'d')) {
		if(!getnameddir(asg->name)) {
		    zwarnnam(name, "no such directory name: %s", asg->name);
		    returnval = 1;
		}
	    } else {
		if (!hashcmd(asg->name, path)) {
		    zwarnnam(name, "no such command: %s", asg->name);
		    returnval = 1;
		}
	    }
	    if(OPT_ISSET(ops,'v') && (hn = ht->getnode2(ht, asg->name)))
		ht->printnode(hn, 0);
	} else if(OPT_ISSET(ops,'v'))
	    ht->printnode(hn, 0);
    }
    unqueue_signals();
    return returnval;
}

/* unhash: remove specified elements from a hash table */

/**/
int
bin_unhash(char *name, char **argv, Options ops, int func)
{
    HashTable ht;
    HashNode hn, nhn;
    Patprog pprog;
    int match = 0, returnval = 0, all = 0;
    int i;

    /* Check which hash table we are working with. */
    if (func == BIN_UNALIAS) {
	if (OPT_ISSET(ops,'s'))
	    ht = sufaliastab;	/* suffix aliases */
	else
	    ht = aliastab;	/* aliases           */
	if (OPT_ISSET(ops, 'a')) {
	    if (*argv) {
		zwarnnam(name, "-a: too many arguments");
		return 1;
	    }
	    all = 1;
	} else if (!*argv) {
	    zwarnnam(name, "not enough arguments");
	    return 1;
	}
    } else if (OPT_ISSET(ops,'d'))
	ht = nameddirtab;	/* named directories */
    else if (OPT_ISSET(ops,'f'))
	ht = shfunctab;		/* shell functions   */
    else if (OPT_ISSET(ops,'s'))
	ht = sufaliastab;	/* suffix aliases, must precede aliases */
    else if (func == BIN_UNHASH && (OPT_ISSET(ops,'a')))
	ht = aliastab;		/* aliases           */
    else
	ht = cmdnamtab;		/* external commands */

    if (all) {
	queue_signals();
	for (i = 0; i < ht->hsize; i++) {
	    for (hn = ht->nodes[i]; hn; hn = nhn) {
		/* record pointer to next, since we may free this one */
		nhn = hn->next;
		ht->freenode(ht->removenode(ht, hn->nam));
	    }
	}
	unqueue_signals();
	return 0;
    }

    /* With -m option, treat arguments as glob patterns. *
     * "unhash -m '*'" is legal, but not recommended.    */
    if (OPT_ISSET(ops,'m')) {
	for (; *argv; argv++) {
	    queue_signals();
	    /* expand argument */
	    tokenize(*argv);
	    if ((pprog = patcompile(*argv, PAT_STATIC, NULL))) {
		/* remove all nodes matching glob pattern */
		for (i = 0; i < ht->hsize; i++) {
		    for (hn = ht->nodes[i]; hn; hn = nhn) {
			/* record pointer to next, since we may free this one */
			nhn = hn->next;
			if (pattry(pprog, hn->nam)) {
			    ht->freenode(ht->removenode(ht, hn->nam));
			    match++;
			}
		    }
		}
	    } else {
		untokenize(*argv);
		zwarnnam(name, "bad pattern : %s", *argv);
		returnval = 1;
	    }
	    unqueue_signals();
	}
	/* If we didn't match anything, we return 1. */
	if (!match)
	    returnval = 1;
	return returnval;
    }

    /* Take arguments literally -- do not glob */
    queue_signals();
    for (; *argv; argv++) {
	if ((hn = ht->removenode(ht, *argv))) {
	    ht->freenode(hn);
	} else if (func == BIN_UNSET && isset(POSIXBUILTINS)) {
	    /* POSIX: unset: "Unsetting a variable or function that was *
	     * not previously set shall not be considered an error."    */
	    returnval = 0;
	} else {
	    zwarnnam(name, "no such hash table element: %s", *argv);
	    returnval = 1;
	}
    }
    unqueue_signals();
    return returnval;
}

/**** alias builtins ****/

/* alias: display or create aliases. */

/**/
int
bin_alias(char *name, char **argv, Options ops, UNUSED(int func))
{
    Alias a;
    Patprog pprog;
    Asgment asg;
    int returnval = 0;
    int flags1 = 0, flags2 = DISABLED;
    int printflags = 0;
    int type_opts;
    HashTable ht = aliastab;

    /* Did we specify the type of alias? */
    type_opts = OPT_ISSET(ops, 'r') + OPT_ISSET(ops, 'g') +
	OPT_ISSET(ops, 's');
    if (type_opts) {
	if (type_opts > 1) {
	    zwarnnam(name, "illegal combination of options");
	    return 1;
	}
	if (OPT_ISSET(ops,'g'))
	    flags1 |= ALIAS_GLOBAL;
	else
	    flags2 |= ALIAS_GLOBAL;
	if (OPT_ISSET(ops, 's')) {
	    /*
	     * Although we keep suffix aliases in a different table,
	     * it is useful to be able to distinguish Alias structures
	     * without reference to the table, so we have a separate
	     * flag, too.
	     */
	    flags1 |= ALIAS_SUFFIX;
	    ht = sufaliastab;
	} else
	    flags2 |= ALIAS_SUFFIX;
    }

    if (OPT_ISSET(ops,'L'))
	printflags |= PRINT_LIST;
    else if (OPT_PLUS(ops,'g') || OPT_PLUS(ops,'r') || OPT_PLUS(ops,'s') ||
	     OPT_PLUS(ops,'m') || OPT_ISSET(ops,'+'))
	printflags |= PRINT_NAMEONLY;

    /* In the absence of arguments, list all aliases.  If a command *
     * line flag is specified, list only those of that type.        */
    if (!*argv) {
	queue_signals();
	scanhashtable(ht, 1, flags1, flags2, ht->printnode, printflags);
	unqueue_signals();
	return 0;
    }

    /* With the -m option, treat the arguments as *
     * glob patterns of aliases to display.       */
    if (OPT_ISSET(ops,'m')) {
	for (; *argv; argv++) {
	    queue_signals();
	    tokenize(*argv);  /* expand argument */
	    if ((pprog = patcompile(*argv, PAT_STATIC, NULL))) {
		/* display the matching aliases */
		scanmatchtable(ht, pprog, 1, flags1, flags2,
			       ht->printnode, printflags);
	    } else {
		untokenize(*argv);
		zwarnnam(name, "bad pattern : %s", *argv);
		returnval = 1;
	    }
	    unqueue_signals();
	}
	return returnval;
    }

    /* Take arguments literally.  Don't glob */
    queue_signals();
    while ((asg = getasg(&argv, NULL))) {
	if (asg->value.scalar && !OPT_ISSET(ops,'L')) {
	    /* The argument is of the form foo=bar and we are not *
	     * forcing a listing with -L, so define an alias      */
	    ht->addnode(ht, ztrdup(asg->name),
			createaliasnode(ztrdup(asg->value.scalar), flags1));
	} else if ((a = (Alias) ht->getnode(ht, asg->name))) {
	    /* display alias if appropriate */
	    if (!type_opts || ht == sufaliastab ||
		(OPT_ISSET(ops,'r') &&
		 !(a->node.flags & (ALIAS_GLOBAL|ALIAS_SUFFIX))) ||
		(OPT_ISSET(ops,'g') && (a->node.flags & ALIAS_GLOBAL)))
		ht->printnode(&a->node, printflags);
	} else
	    returnval = 1;
    }
    unqueue_signals();
    return returnval;
}


/**** miscellaneous builtins ****/

/* true, : (colon) */

/**/
int
bin_true(UNUSED(char *name), UNUSED(char **argv), UNUSED(Options ops), UNUSED(int func))
{
    return 0;
}

/* false builtin */

/**/
int
bin_false(UNUSED(char *name), UNUSED(char **argv), UNUSED(Options ops), UNUSED(int func))
{
    return 1;
}

/* the zle buffer stack */

/**/
mod_export LinkList bufstack;

/* echo, print, printf, pushln */

#define print_val(VAL) \
    if (prec >= 0) \
	count += fprintf(fout, spec, width, prec, VAL); \
    else \
	count += fprintf(fout, spec, width, VAL);

/*
 * Because of the use of getkeystring() to interpret the arguments,
 * the elements of args spend a large part of the function unmetafied
 * with the lengths in len.  This may have seemed a good idea once.
 * As we are stuck with this for now, we need to be very careful
 * deciding what state args is in.
 */

/**/
int
bin_print(char *name, char **args, Options ops, int func)
{
    int flen, width, prec, type, argc, n, narg, curlen = 0;
    int nnl = 0, fmttrunc = 0, ret = 0, maxarg = 0, nc = 0;
    int flags[6], *len, visarr = 0;
    char *start, *endptr, *c, *d, *flag, *buf = NULL, spec[14], *fmt = NULL;
    char **first, **argp, *curarg, *flagch = "'0+- #", save = '\0', nullstr = '\0';
    size_t rcount = 0, count = 0;
    size_t *cursplit = 0, *splits = 0;
    FILE *fout = stdout;
#ifdef HAVE_OPEN_MEMSTREAM
    size_t mcount;
#define ASSIGN_MSTREAM(BUF,FOUT) \
    do { \
        if ((FOUT = open_memstream(&BUF, &mcount)) == NULL) { \
            zwarnnam(name, "open_memstream failed"); \
            return 1; \
        } \
    } while (0)
    /*
     * Some implementations of open_memstream() have a bug such that,
     * if fflush() is followed by fclose(), another NUL byte is written
     * to the buffer at the wrong position.  Therefore we must fclose()
     * before reading.
     */
#define READ_MSTREAM(BUF,FOUT) \
    ((fclose(FOUT) == 0) ? mcount : (size_t)-1)
#define CLOSE_MSTREAM(FOUT) 0

#else /* simulate HAVE_OPEN_MEMSTREAM */

#define ASSIGN_MSTREAM(BUF,FOUT) \
    do { \
        int tempfd; \
        char *tmpf; \
        if ((tempfd = gettempfile(NULL, 1, &tmpf)) < 0) { \
            zwarnnam(name, "can't open temp file: %e", errno); \
            return 1; \
        } \
        unlink(tmpf); \
        if ((FOUT = fdopen(tempfd, "w+")) == NULL) { \
            close(tempfd); \
            zwarnnam(name, "can't open temp file: %e", errno); \
            return 1; \
        } \
    } while (0)
#define READ_MSTREAM(BUF,FOUT) \
    ((((count = ftell(FOUT)), (BUF = (char *)zalloc(count + 1))) && \
      ((fseek(FOUT, 0L, SEEK_SET) == 0) && !(BUF[count] = '\0')) && \
      (fread(BUF, 1, count, FOUT) == count)) ? count : (size_t)-1)
#define CLOSE_MSTREAM(FOUT) fclose(FOUT)

#endif

#define IS_MSTREAM(FOUT) \
    (FOUT != stdout && \
     (OPT_ISSET(ops,'z') || OPT_ISSET(ops,'s') || OPT_ISSET(ops,'v')))

    /* Testing EBADF special-cases >&- redirections */
#define CLOSE_CLEANLY(FOUT) \
    (IS_MSTREAM(FOUT) ? CLOSE_MSTREAM(FOUT) == 0 : \
     ((FOUT == stdout) ? (fflush(FOUT) == 0 || errno == EBADF) : \
      (fclose(FOUT) == 0)))	/* implies error for -u on a closed fd */

    Histent ent;
    mnumber mnumval;
    double doubleval;
    int intval;
    zlong zlongval;
    zulong zulongval;
    char *stringval;

    /* Error check option combinations and option arguments */

    if (OPT_ISSET(ops, 'z') +
	OPT_ISSET(ops, 's') + OPT_ISSET(ops, 'S') +
	OPT_ISSET(ops, 'v') > 1) {
	zwarnnam(name, "only one of -s, -S, -v, or -z allowed");
	return 1;
    }
    if ((OPT_ISSET(ops, 'z') | OPT_ISSET(ops, 's') | OPT_ISSET(ops, 'S')) +
	(OPT_ISSET(ops, 'c') | OPT_ISSET(ops, 'C')) > 1) {
	zwarnnam(name, "-c or -C not allowed with -s, -S, or -z");
	return 1;
    }
    if ((OPT_ISSET(ops, 'z') | OPT_ISSET(ops, 'v') |
         OPT_ISSET(ops, 's') | OPT_ISSET(ops, 'S')) +
	(OPT_ISSET(ops, 'p') | OPT_ISSET(ops, 'u')) > 1) {
	zwarnnam(name, "-p or -u not allowed with -s, -S, -v, or -z");
	return 1;
    }
    /*
    if (OPT_ISSET(ops, 'f') &&
	(OPT_ISSET(ops, 'S') || OPT_ISSET(ops, 'c') || OPT_ISSET(ops, 'C'))) {
	zwarnnam(name, "-f not allowed with -c, -C, or -S");
	return 1;
    }
    */

    /* -C -- number of columns */
    if (!fmt && OPT_ISSET(ops,'C')) {
	char *eptr, *argptr = OPT_ARG(ops,'C');
	nc = (int)zstrtol(argptr, &eptr, 10);
	if (*eptr) {
	    zwarnnam(name, "number expected after -%c: %s", 'C', argptr);
	    return 1;
	}
	if (nc <= 0) {
	    zwarnnam(name, "invalid number of columns: %s", argptr);
	    return 1;
	}
    }

    if (func == BIN_PRINTF) {
        if (!strcmp(*args, "--") && !*++args) {
            zwarnnam(name, "not enough arguments");
	    return 1;
        }
  	fmt = *args++;
    } else if (func == BIN_ECHO && isset(BSDECHO))
	ops->ind['E'] = 1;
    else if (OPT_HASARG(ops,'f'))
	fmt = OPT_ARG(ops,'f');
    if (fmt)
	fmt = getkeystring(fmt, &flen, OPT_ISSET(ops,'b') ? GETKEYS_BINDKEY :
			   GETKEYS_PRINTF_FMT, &fmttrunc);

    first = args;

    /* -m option -- treat the first argument as a pattern and remove
     * arguments not matching */
    if (OPT_ISSET(ops,'m')) {
	Patprog pprog;
	char **t, **p;

	if (!*args) {
	    zwarnnam(name, "no pattern specified");
	    return 1;
	}
	queue_signals();
	tokenize(*args);
	if (!(pprog = patcompile(*args, PAT_STATIC, NULL))) {
	    untokenize(*args);
	    zwarnnam(name, "bad pattern: %s", *args);
	    unqueue_signals();
	    return 1;
	}
	for (t = p = ++args; *p; p++)
	    if (pattry(pprog, *p))
		*t++ = *p;
	*t = NULL;
	first = args;
	unqueue_signals();
	if (fmt && !*args) return 0;
    }
    /* compute lengths, and interpret according to -P, -D, -e, etc. */
    argc = arrlen(args);
    len = (int *) hcalloc(argc * sizeof(int));
    for (n = 0; n < argc; n++) {
	/* first \ sequences */
	if (fmt ||
	    (!OPT_ISSET(ops,'e') &&
	     (OPT_ISSET(ops,'R') || OPT_ISSET(ops,'r') || OPT_ISSET(ops,'E'))))
	    unmetafy(args[n], &len[n]);
	else {
	    int escape_how;
	    if (OPT_ISSET(ops,'b'))
		escape_how = GETKEYS_BINDKEY;
	    else if (func != BIN_ECHO && !OPT_ISSET(ops,'e'))
		escape_how = GETKEYS_PRINT;
	    else
		escape_how = GETKEYS_ECHO;
	    args[n] = getkeystring(args[n], &len[n], escape_how, &nnl);
	    if (nnl) {
		/* If there was a \c escape, make this the last arg. */
		argc = n + 1;
		args[argc] = NULL;
	    }
	}
	/* -P option -- interpret as a prompt sequence */
	if (OPT_ISSET(ops,'P')) {
	    /*
	     * promptexpand uses permanent storage: to avoid
	     * messy memory management, stick it on the heap
	     * instead.
	     */
	    char *str = unmetafy(
		promptexpand(metafy(args[n], len[n], META_NOALLOC),
			     0, NULL, NULL, NULL),
		&len[n]);
	    args[n] = dupstrpfx(str, len[n]);
	    free(str);
	}
	/* -D option -- interpret as a directory, and use ~ */
	if (OPT_ISSET(ops,'D')) {
	    Nameddir d;

	    queue_signals();
	    /* TODO: finddir takes a metafied file */
	    d = finddir(args[n]);
	    if (d) {
		int dirlen = strlen(d->dir);
		char *arg = zhalloc(len[n] - dirlen + strlen(d->node.nam) + 2);
		sprintf(arg, "~%s%s", d->node.nam, args[n] + dirlen);
		args[n] = arg;
		len[n] = strlen(args[n]);
	    }
	    unqueue_signals();
	}
    }

    /* -o and -O -- sort the arguments */
    if (OPT_ISSET(ops,'o') || OPT_ISSET(ops,'O')) {
	int flags;

	if (fmt && !*args)
	    return 0;
	flags = OPT_ISSET(ops,'i') ? SORTIT_IGNORING_CASE : 0;
	if (OPT_ISSET(ops,'O'))
	    flags |= SORTIT_BACKWARDS;
	strmetasort(args, flags, len);
    }

    /* -u and -p -- output to other than standard output */
    if ((OPT_HASARG(ops,'u') || OPT_ISSET(ops,'p')) &&
	/* rule out conflicting options -- historical precedence */
	((!fmt && (OPT_ISSET(ops,'c') || OPT_ISSET(ops,'C'))) ||
	 !(OPT_ISSET(ops, 'z') || OPT_ISSET(ops, 'v') ||
	   OPT_ISSET(ops, 's') || OPT_ISSET(ops, 'S')))) {
	int fdarg, fd;

	if (OPT_ISSET(ops, 'p')) {
	    fdarg = coprocout;
	    if (fdarg < 0) {
		zwarnnam(name, "-p: no coprocess");
		return 1;
	    }
	} else {
	    char *argptr = OPT_ARG(ops,'u'), *eptr;
	    /* Handle undocumented feature that -up worked */
	    if (!strcmp(argptr, "p")) {
		fdarg = coprocout;
		if (fdarg < 0) {
		    zwarnnam(name, "-p: no coprocess");
		    return 1;
		}
	    } else {
		fdarg = (int)zstrtol(argptr, &eptr, 10);
		if (*eptr) {
		    zwarnnam(name, "number expected after -u: %s", argptr);
		    return 1;
		}
	    }
	}

	if ((fd = dup(fdarg)) < 0) {
	    zwarnnam(name, "bad file number: %d", fdarg);
	    return 1;
	}
	if ((fout = fdopen(fd, "w")) == 0) {
	    close(fd);
	    zwarnnam(name, "bad mode on fd %d", fd);
	    return 1;
	}
    }

    if (OPT_ISSET(ops, 'v') ||
	(fmt && (OPT_ISSET(ops,'z') || OPT_ISSET(ops,'s'))))
	ASSIGN_MSTREAM(buf,fout);

    /* -c -- output in columns */
    if (!fmt && (OPT_ISSET(ops,'c') || OPT_ISSET(ops,'C'))) {
	int l, nr, sc, n, t, i;
#ifdef MULTIBYTE_SUPPORT
	int *widths;

	if (isset(MULTIBYTE)) {
	    int *wptr;

	    /*
	     * We need the character widths to align output in
	     * columns.
	     */
	    wptr = widths = (int *) zhalloc(argc * sizeof(int));
	    for (i = 0; i < argc && args[i]; i++, wptr++) {
		int l = len[i], width = 0;
		char *aptr = args[i];
		mbstate_t mbs;

		memset(&mbs, 0, sizeof(mbstate_t));
		while (l > 0) {
		    wchar_t wc;
		    size_t cnt;
		    int wcw;

		    /*
		     * Prevent misaligned columns due to escape sequences by
		     * skipping over them. Octals \033 and \233 are the
		     * possible escape characters recognized by ANSI.
		     *
		     * It ought to be possible to do this in the case
		     * of prompt expansion by propagating the information
		     * about escape sequences (currently we strip this
		     * out).
		     */
		    if (*aptr == '\033' || *aptr == '\233') {
			for (aptr++, l--;
			     l && !isalpha(STOUC(*aptr));
			     aptr++, l--)
			    ;
			aptr++;
			l--;
			continue;
		    }

		    cnt = mbrtowc(&wc, aptr, l, &mbs);

		    if (cnt == MB_INCOMPLETE || cnt == MB_INVALID)
		    {
			/* treat as ordinary string */
			width += l;
			break;
		    }
		    wcw = WCWIDTH(wc);
		    /* treat unprintable as 0 */
		    if (wcw > 0)
			width += wcw;
		    /* skip over NUL normally */
		    if (cnt == 0)
			cnt = 1;
		    aptr += cnt;
		    l -= cnt;
		}
		widths[i] = width;
	    }
	}
	else
	    widths = len;
#else
	int *widths = len;
#endif

	if (OPT_ISSET(ops,'C')) {
	    /*
	     * n: number of elements
	     * nc: number of columns (above)
	     * nr: number of rows
	     */
	    n = arrlen(args);
	    nr = (n + nc - 1) / nc;

	    /*
	     * i: loop counter
	     * l: maximum length seen
	     *
	     * Ignore lengths in last column since they don't affect
	     * the separation.
	     */
	    for (i = l = 0; i < argc; i++) {
		if (OPT_ISSET(ops, 'a')) {
		    if ((i % nc) == nc - 1)
			continue;
		} else {
		    if (i >= nr * (nc - 1))
			break;
		}
		if (l < widths[i])
		    l = widths[i];
	    }
	    sc = l + 2;
	}
	else
	{
	    /*
	     * n: loop counter
	     * l: maximum length seen
	     */
	    for (n = l = 0; n < argc; n++)
		if (l < widths[n])
		    l = widths[n];

	    /*
	     * sc: column width
	     * nc: number of columns (at least one)
	     */
	    sc = l + 2;
	    nc = (zterm_columns + 1) / sc;
	    if (!nc)
		nc = 1;
	    nr = (n + nc - 1) / nc;
	}

	if (OPT_ISSET(ops,'a'))	/* print across, i.e. columns first */
	    n = 0;
	for (i = 0; i < nr; i++) {
	    if (OPT_ISSET(ops,'a'))
	    {
		int ic;
		for (ic = 0; ic < nc && n < argc; ic++, n++)
		{
		    fwrite(args[n], len[n], 1, fout);
		    l = widths[n];
		    if (n < argc && ic < nc - 1)
			for (; l < sc; l++)
			    fputc(' ', fout);
		}
	    }
	    else
	    {
		n = i;
		do {
		    fwrite(args[n], len[n], 1, fout);
		    l = widths[n];
		    for (t = nr; t && n < argc; t--, n++);
		    if (n < argc)
			for (; l < sc; l++)
			    fputc(' ', fout);
		} while (n < argc);
	    }
	    fputc(OPT_ISSET(ops,'N') ? '\0' : '\n', fout);
	}
	if (IS_MSTREAM(fout) && (rcount = READ_MSTREAM(buf,fout)) == -1)
	    ret = 1;
	if (!CLOSE_CLEANLY(fout) || ret) {
            zwarnnam(name, "write error: %e", errno);
            ret = 1;
	}
	if (buf) {
	    /* assert: we must be doing -v at this point */
	    queue_signals();
	    if (ret)
		free(buf);
	    else
		setsparam(OPT_ARG(ops, 'v'),
			  metafy(buf, rcount, META_REALLOC));
	    unqueue_signals();
	}
	return ret;
    }

    /* normal output */
    if (!fmt) {
	if (OPT_ISSET(ops, 'z') ||
	    OPT_ISSET(ops, 's') || OPT_ISSET(ops, 'S')) {
	    /*
	     * We don't want the arguments unmetafied after all.
	     */
	    for (n = 0; n < argc; n++)
		metafy(args[n], len[n], META_NOALLOC);
	}

	/* -z option -- push the arguments onto the editing buffer stack */
	if (OPT_ISSET(ops,'z')) {
	    queue_signals();
	    zpushnode(bufstack, sepjoin(args, NULL, 0));
	    unqueue_signals();
	    return 0;
	}
	/* -s option -- add the arguments to the history list */
	if (OPT_ISSET(ops,'s') || OPT_ISSET(ops,'S')) {
	    int nwords = 0, nlen, iwords;
	    char **pargs = args;

	    queue_signals();
	    while (*pargs++)
		nwords++;
	    if (nwords) {
		if (OPT_ISSET(ops,'S')) {
		    int wordsize;
		    short *words;
		    if (nwords > 1) {
			zwarnnam(name, "option -S takes a single argument");
			unqueue_signals();
			return 1;
		    }
		    words = NULL;
		    wordsize = 0;
		    histsplitwords(*args, &words, &wordsize, &nwords, 1);
		    ent = prepnexthistent();
		    ent->words = (short *)zalloc(nwords*sizeof(short));
		    memcpy(ent->words, words, nwords*sizeof(short));
		    free(words);
		    ent->nwords = nwords/2;
		} else {
		    ent = prepnexthistent();
		    ent->words = (short *)zalloc(nwords*2*sizeof(short));
		    ent->nwords = nwords;
		    nlen = iwords = 0;
		    for (pargs = args; *pargs; pargs++) {
			ent->words[iwords++] = nlen;
			nlen += strlen(*pargs);
			ent->words[iwords++] = nlen;
			nlen++;
		    }
		}
	    } else {
		ent = prepnexthistent();
		ent->words = (short *)NULL;
	    }
	    ent->node.nam = zjoin(args, ' ', 0);
	    ent->stim = ent->ftim = time(NULL);
	    ent->node.flags = 0;
	    addhistnode(histtab, ent->node.nam, ent);
	    unqueue_signals();
	    return 0;
	}

	if (OPT_HASARG(ops, 'x') || OPT_HASARG(ops, 'X')) {
	    char *eptr;
	    int expand, startpos = 0;
	    int all = OPT_HASARG(ops, 'X');
	    char *xarg = all ? OPT_ARG(ops, 'X') : OPT_ARG(ops, 'x');

	    expand = (int)zstrtol(xarg, &eptr, 10);
	    if (*eptr || expand <= 0) {
		zwarnnam(name, "positive integer expected after -%c: %s", 'x',
			 xarg);
		return 1;
	    }
	    for (; *args; args++, len++) {
		startpos = zexpandtabs(*args, *len, expand, startpos, fout,
				       all);
		if (args[1]) {
		    if (OPT_ISSET(ops, 'l')) {
			fputc('\n', fout);
			startpos = 0;
		    } else if (OPT_ISSET(ops,'N')) {
			fputc('\0', fout);
		    } else {
			fputc(' ', fout);
			startpos++;
		    }
		}
	    }
	} else {
	    for (; *args; args++, len++) {
		fwrite(*args, *len, 1, fout);
		if (args[1])
		    fputc(OPT_ISSET(ops,'l') ? '\n' :
			  OPT_ISSET(ops,'N') ? '\0' : ' ', fout);
	    }
	}
	if (!(OPT_ISSET(ops,'n') || nnl ||
	    (OPT_ISSET(ops, 'v') && !OPT_ISSET(ops, 'l'))))
	    fputc(OPT_ISSET(ops,'N') ? '\0' : '\n', fout);
	if (IS_MSTREAM(fout) && (rcount = READ_MSTREAM(buf,fout)) == -1)
	    ret = 1;
	if (!CLOSE_CLEANLY(fout) || ret) {
            zwarnnam(name, "write error: %e", errno);
            ret = 1;
	}
	if (buf) {
	    /* assert: we must be doing -v at this point */
	    queue_signals();
	    if (ret)
		free(buf);
	    else
		setsparam(OPT_ARG(ops, 'v'),
			  metafy(buf, rcount, META_REALLOC));
	    unqueue_signals();
	}
	return ret;
    }

    /*
     * All the remaining code in this function is for printf-style
     * output (printf itself, or print -f).  We still have to handle
     * special cases of printing to a ZLE buffer or the history, however.
     */

    if (OPT_ISSET(ops,'v')) {
	struct value vbuf;
	char* s = OPT_ARG(ops,'v');
	Value v = getvalue(&vbuf, &s, 0);
	visarr = v && PM_TYPE(v->pm->node.flags) == PM_ARRAY;
    }
    /* printf style output */
    *spec = '%';
    argp = args;
    do {
    	rcount = count;
	if (argp > args && visarr) { /* reusing format string */
	    if (!splits)
		cursplit = splits = (size_t *)zhalloc(sizeof(size_t) *
			(arrlen(args) / (argp - args) + 1));
	    *cursplit++ = count;
	}
    	if (maxarg) {
	    first += maxarg;
	    argc -= maxarg;
    	    maxarg = 0;
	}
	for (c = fmt; c-fmt < flen; c++) {
	    if (*c != '%') {
		putc(*c, fout);
		++count;
		continue;
	    }

	    start = c++;
	    if (*c == '%') {
		putc('%', fout);
		++count;
		continue;
	    }

	    type = prec = -1;
	    width = 0;
	    curarg = NULL;
	    d = spec + 1;

	    if (*c >= '1' && *c <= '9') {
	    	narg = strtoul(c, &endptr, 0);
		if (*endptr == '$') {
		    c = endptr + 1;
		    if (narg <= 0 || narg > argc) {
		    	zwarnnam(name, "%d: argument specifier out of range",
				 narg);
			if (fout != stdout)
			    fclose(fout);
#ifdef HAVE_OPEN_MEMSTREAM
			if (buf)
			    free(buf);
#endif
			return 1;
		    } else {
		    	if (narg > maxarg) maxarg = narg;
		    	curarg = *(first + narg - 1);
			curlen = len[first - args + narg - 1];
		    }
		}
	    }

	    /* copy only one of each flag as spec has finite size */
	    memset(flags, 0, sizeof(flags));
	    while (*c && (flag = strchr(flagch, *c))) {
	    	if (!flags[flag - flagch]) {
	    	    flags[flag - flagch] = 1;
		    *d++ = *c;
		}
	    	c++;
	    }

	    if (idigit(*c)) {
		width = strtoul(c, &endptr, 0);
		c = endptr;
	    } else if (*c == '*') {
		if (idigit(*++c)) {
		    narg = strtoul(c, &endptr, 0);
		    if (*endptr == '$') {
		    	c = endptr + 1;
			if (narg > argc || narg <= 0) {
		    	    zwarnnam(name,
				     "%d: argument specifier out of range",
				     narg);
			    if (fout != stdout)
				fclose(fout);
#ifdef HAVE_OPEN_MEMSTREAM
			    if (buf)
				free(buf);
#endif
			    return 1;
			} else {
		    	    if (narg > maxarg) maxarg = narg;
		    	    argp = first + narg - 1;
			}
		    }
		}
		if (*argp) {
		    width = (int)mathevali(*argp++);
		    if (errflag) {
			errflag &= ~ERRFLAG_ERROR;
			ret = 1;
		    }
		}
	    }
	    *d++ = '*';

	    if (*c == '.') {
		if (*++c == '*') {
		    if (idigit(*++c)) {
			narg = strtoul(c, &endptr, 0);
			if (*endptr == '$') {
			    c = endptr + 1;
			    if (narg > argc || narg <= 0) {
		    		zwarnnam(name,
					 "%d: argument specifier out of range",
					 narg);
				if (fout != stdout)
				    fclose(fout);
#ifdef HAVE_OPEN_MEMSTREAM
				if (buf)
				    free(buf);
#endif
				return 1;
			    } else {
		    		if (narg > maxarg) maxarg = narg;
		    		argp = first + narg - 1;
			    }
			}
		    }

		    if (*argp) {
			prec = (int)mathevali(*argp++);
			if (errflag) {
			    errflag &= ~ERRFLAG_ERROR;
			    ret = 1;
			}
		    }
		} else if (idigit(*c)) {
		    prec = strtoul(c, &endptr, 0);
		    c = endptr;
		} else
		    prec = 0;
		if (prec >= 0) *d++ = '.', *d++ = '*';
	    }

	    /* ignore any size modifier */
	    if (*c == 'l' || *c == 'L' || *c == 'h') c++;

	    if (!curarg && *argp) {
		curarg = *argp;
		curlen = len[argp++ - args];
	    }
	    d[1] = '\0';
	    switch (*d = *c) {
	    case 'c':
		if (curarg)
		    intval = *curarg;
		else
		    intval = 0;
		print_val(intval);
		break;
	    case 's':
	    case 'b':
		if (curarg) {
		    char *b, *ptr;
		    int lbytes, lchars, lleft;
#ifdef MULTIBYTE_SUPPORT
		    mbstate_t mbs;
#endif

		    if (*c == 'b') {
			b = getkeystring(metafy(curarg, curlen, META_USEHEAP),
					 &lbytes,
					 OPT_ISSET(ops,'b') ? GETKEYS_BINDKEY :
					 GETKEYS_PRINTF_ARG, &nnl);
		    } else {
			b = curarg;
			lbytes = curlen;
		    }
		    /*
		     * Handle width/precision here and use fwrite so that
		     * nul characters can be output.
		     *
		     * First, examine width of string given that it
		     * may contain multibyte characters.  The output
		     * widths are for characters, so we need to count
		     * (in lchars).  However, if we need to truncate
		     * the string we need the width in bytes (in lbytes).
		     */
		    ptr = b;
#ifdef MULTIBYTE_SUPPORT
		    memset(&mbs, 0, sizeof(mbs));
#endif

		    for (lchars = 0, lleft = lbytes; lleft > 0; lchars++) {
			int chars;

			if (lchars == prec) {
			    /* Truncate at this point. */
			    lbytes = ptr - b;
			    break;
			}
#ifdef MULTIBYTE_SUPPORT
			if (isset(MULTIBYTE)) {
			    chars = mbrlen(ptr, lleft, &mbs);
			    if (chars < 0) {
				/*
				 * Invalid/incomplete character at this
				 * point.  Assume all the rest are a
				 * single byte.  That's about the best we
				 * can do.
				 */
				lchars += lleft;
				lbytes = (ptr - b) + lleft;
				break;
			    } else if (chars == 0) {
				/* NUL, handle as real character */
				chars = 1;
			    }
			}
			else	/* use the non-multibyte code below */
#endif
			    chars = 1; /* compiler can optimise this...*/
			lleft -= chars;
			ptr += chars;
		    }
		    if (width > 0 && flags[3]) width = -width;
		    if (width > 0 && lchars < width)
		    	count += fprintf(fout, "%*c", width - lchars, ' ');
		    count += fwrite(b, 1, lbytes, fout);
		    if (width < 0 && lchars < -width)
		    	count += fprintf(fout, "%*c", -width - lchars, ' ');
		    if (nnl) {
			/* If the %b arg had a \c escape, truncate the fmt. */
			flen = c - fmt + 1;
			fmttrunc = 1;
		    }
		} else if (width)
		    count += fprintf(fout, "%*c", width, ' ');
		break;
	    case 'q':
		stringval = curarg ?
		    quotestring(metafy(curarg, curlen, META_USEHEAP),
				QT_BACKSLASH_SHOWNULL) : &nullstr;
		*d = 's';
		print_val(unmetafy(stringval, &curlen));
		break;
	    case 'd':
	    case 'i':
		type=1;
		break;
	    case 'e':
	    case 'E':
	    case 'f':
	    case 'g':
	    case 'G':
		type=2;
		break;
	    case 'o':
	    case 'u':
	    case 'x':
	    case 'X':
		type=3;
		break;
	    case 'n':
		if (curarg) setiparam(curarg, count - rcount);
		break;
	    default:
	        if (*c) {
		    save = c[1];
	            c[1] = '\0';
		}
		zwarnnam(name, "%s: invalid directive", start);
		if (*c) c[1] = save;
		/* Why do we care about a clean close here? */
		if (!CLOSE_CLEANLY(fout))
		    zwarnnam(name, "write error: %e", errno);
#ifdef HAVE_OPEN_MEMSTREAM
		if (buf)
		    free(buf);
#endif
		return 1;
	    }

	    if (type > 0) {
		if (curarg && (*curarg == '\'' || *curarg == '"' )) {
		    convchar_t cc;
#ifdef MULTIBYTE_SUPPORT
		    if (isset(MULTIBYTE)) {
			mb_charinit();
			(void)mb_metacharlenconv(metafy(curarg+1, curlen-1,
							META_USEHEAP), &cc);
		    }
		    else
			cc = WEOF;
		    if (cc == WEOF)
			cc = (curlen > 1) ? STOUC(curarg[1]) : 0;
#else
		    cc = (curlen > 1) ? STOUC(curarg[1]) : 0;
#endif
		    if (type == 2) {
			doubleval = cc;
			print_val(doubleval);
		    } else {
			intval = cc;
			print_val(intval);
		    }
		} else {
		    switch (type) {
		    case 1:
#ifdef ZSH_64_BIT_TYPE
 		    	*d++ = 'l';
#endif
		    	*d++ = 'l', *d++ = *c, *d = '\0';
			zlongval = (curarg) ? mathevali(curarg) : 0;
			if (errflag) {
			    zlongval = 0;
			    errflag &= ~ERRFLAG_ERROR;
			    ret = 1;
			}
			print_val(zlongval)
			    break;
		    case 2:
			if (curarg) {
			    char *eptr;
			    /*
			     * First attempt to parse as a floating
			     * point constant.  If we go through
			     * a math evaluation, we can lose
			     * mostly unimportant information
			     * that people in standards organizations
			     * worry about.
			     */
			    doubleval = strtod(curarg, &eptr);
			    /*
			     * If it didn't parse as a constant,
			     * parse it as an expression.
			     */
			    if (*eptr != '\0') {
				mnumval = matheval(curarg);
				doubleval = (mnumval.type & MN_FLOAT) ?
				    mnumval.u.d : (double)mnumval.u.l;
			    }
			} else doubleval = 0;
			if (errflag) {
			    doubleval = 0;
			    errflag &= ~ERRFLAG_ERROR;
			    ret = 1;
			}
			/* force consistent form for Inf/NaN output */
			if (isnan(doubleval))
			    count += fputs("nan", fout);
			else if (isinf(doubleval))
			    count += fputs((doubleval < 0.0) ? "-inf" : "inf", fout);
		        else
			    print_val(doubleval)
			break;
		    case 3:
#ifdef ZSH_64_BIT_UTYPE
 		    	*d++ = 'l';
#endif
		    	*d++ = 'l', *d++ = *c, *d = '\0';
			if (!curarg)
			    zulongval = (zulong)0;
			else if (!zstrtoul_underscore(curarg, &zulongval))
			    zulongval = mathevali(curarg);
			if (errflag) {
			    zulongval = 0;
			    errflag &= ~ERRFLAG_ERROR;
			    ret = 1;
			}
			print_val(zulongval)
		    }
		}
	    }
	    if (maxarg && (argp - first > maxarg))
	    	maxarg = argp - first;
	}

    	if (maxarg) argp = first + maxarg;
	/* if there are remaining args, reuse format string */
    } while (*argp && argp != first && !fmttrunc && !OPT_ISSET(ops,'r'));

    if (IS_MSTREAM(fout)) {
	queue_signals();
	if ((rcount = READ_MSTREAM(buf,fout)) == -1) {
	    zwarnnam(name, "i/o error: %e", errno);
	    if (buf)
		free(buf);
	} else {
	    if (visarr && splits) {
		char **arrayval = zshcalloc((cursplit - splits + 2) * sizeof(char *));
		for (;cursplit >= splits; cursplit--) {
		    int start = cursplit == splits ? 0 : cursplit[-1];
		    arrayval[cursplit - splits] =
			    metafy(buf + start, count - start, META_DUP);
		    count = start;
		}
		setaparam(OPT_ARG(ops, 'v'), arrayval);
		free(buf);
	    } else {
		stringval = metafy(buf, rcount, META_REALLOC);
		if (OPT_ISSET(ops,'z')) {
		    zpushnode(bufstack, stringval);
		} else if (OPT_ISSET(ops,'v')) {
		    setsparam(OPT_ARG(ops, 'v'), stringval);
		} else {
		    ent = prepnexthistent();
		    ent->node.nam = stringval;
		    ent->stim = ent->ftim = time(NULL);
		    ent->node.flags = 0;
		    ent->words = (short *)NULL;
		    addhistnode(histtab, ent->node.nam, ent);
		}
	    }
	}
	unqueue_signals();
    }

    if (!CLOSE_CLEANLY(fout))
    {
	zwarnnam(name, "write error: %e", errno);
	ret = 1;
    }
    return ret;
}

/* shift builtin */

/**/
int
bin_shift(char *name, char **argv, Options ops, UNUSED(int func))
{
    int num = 1, l, ret = 0;
    char **s;

    /* optional argument can be either numeric or an array */
    queue_signals();
    if (*argv && !getaparam(*argv)) {
        num = mathevali(*argv++);
	if (errflag) {
	    unqueue_signals();
	    return 1;
	}
    }

    if (num < 0) {
	unqueue_signals();
        zwarnnam(name, "argument to shift must be non-negative");
        return 1;
    }

    if (*argv) {
        for (; *argv; argv++)
            if ((s = getaparam(*argv))) {
                if (arrlen_lt(s, num)) {
		    zwarnnam(name, "shift count must be <= $#");
		    ret++;
		    continue;
		}
		if (OPT_ISSET(ops,'p')) {
		    char **s2, **src, **dst;
		    int count;
		    l = arrlen(s);
		    src = s;
		    dst = s2 = (char **)zalloc((l - num + 1) * sizeof(char *));
		    for (count = l - num; count; count--)
			*dst++ = ztrdup(*src++);
		    *dst = NULL;
		    s = s2;
		} else {
		    s = zarrdup(s + num);
		}
                setaparam(*argv, s);
            }
    } else {
        if (num > (l = arrlen(pparams))) {
	    zwarnnam(name, "shift count must be <= $#");
	    ret = 1;
	} else {
	    s = zalloc((l - num + 1) * sizeof(char *));
	    if (OPT_ISSET(ops,'p')) {
		memcpy(s, pparams, (l - num) * sizeof(char *));
		s[l-num] = NULL;
		while (num--)
		    zsfree(pparams[l-1-num]);
	    } else {
		memcpy(s, pparams + num, (l - num + 1) * sizeof(char *));
		while (num--)
		    zsfree(pparams[num]);
	    }
	    zfree(pparams, (l + 1) * sizeof(char *));
	    pparams = s;
	}
    }
    unqueue_signals();
    return ret;
}

/*
 * Position of getopts option within OPTIND argument with multiple options.
 */

/**/
int optcind;

/* getopts: automagical option handling for shell scripts */

/**/
int
bin_getopts(UNUSED(char *name), char **argv, UNUSED(Options ops), UNUSED(int func))
{
    int lenstr, lenoptstr, quiet, lenoptbuf;
    char *optstr = unmetafy(*argv++, &lenoptstr), *var = *argv++;
    char **args = (*argv) ? argv : pparams;
    char *str, optbuf[2] = " ", *p, opch;

    /* zoptind keeps count of the current argument number.  The *
     * user can set it to zero to start a new option parse.     */
    if (zoptind < 1) {
	/* first call */
	zoptind = 1;
	optcind = 0;
    }
    if (arrlen_lt(args, zoptind))
	/* no more options */
	return 1;

    /* leading ':' in optstr means don't print an error message */
    quiet = *optstr == ':';
    optstr += quiet;
    lenoptstr -= quiet;

    /* find place in relevant argument */
    str = unmetafy(dupstring(args[zoptind - 1]), &lenstr);
    if (!lenstr)		/* Definitely not an option. */
	return 1;
    if(optcind >= lenstr) {
	optcind = 0;
	if(!args[zoptind++])
	    return 1;
	str = unmetafy(dupstring(args[zoptind - 1]), &lenstr);
    }
    if(!optcind) {
	if(lenstr < 2 || (*str != '-' && *str != '+'))
	    return 1;
	if(lenstr == 2 && str[0] == '-' && str[1] == '-') {
	    zoptind++;
	    return 1;
	}
	optcind++;
    }
    opch = str[optcind++];
    if(str[0] == '+') {
	optbuf[0] = '+';
	lenoptbuf = 2;
    } else
	lenoptbuf = 1;
    optbuf[lenoptbuf - 1] = opch;

    /* check for legality */
    if(opch == ':' || !(p = memchr(optstr, opch, lenoptstr))) {
	p = "?";
	/* Keep OPTIND correct if the user doesn't return after the error */
	if (isset(POSIXBUILTINS)) {
	    optcind = 0;
	    zoptind++;
	}
	zsfree(zoptarg);
	setsparam(var, ztrdup(p));
	if(quiet) {
	    zoptarg = metafy(optbuf, lenoptbuf, META_DUP);
	} else {
	    zwarn("bad option: %c%c",
		  "?-+"[lenoptbuf], opch);
	    zoptarg=ztrdup("");
	}
	return 0;
    }

    /* check for required argument */
    if(p[1] == ':') {
	if(optcind == lenstr) {
	    if(!args[zoptind]) {
		/* Fix OPTIND as above */
		if (isset(POSIXBUILTINS)) {
		    optcind = 0;
		    zoptind++;
		}
		zsfree(zoptarg);
		if(quiet) {
		    setsparam(var, ztrdup(":"));
		    zoptarg = metafy(optbuf, lenoptbuf, META_DUP);
		} else {
		    setsparam(var, ztrdup("?"));
		    zoptarg = ztrdup("");
		    zwarn("argument expected after %c%c option",
			  "?-+"[lenoptbuf], opch);
		}
		return 0;
	    }
	    p = ztrdup(args[zoptind++]);
	} else
	    p = metafy(str+optcind, lenstr-optcind, META_DUP);
	/*
	 * Careful:  I've just changed the following two lines from
	 *   optcind = ztrlen(args[zoptind - 1]);
	 * and it's a rigorous theorem that every change in getopts breaks
	 * something.  See zsh-workers/9095 for the bug fixed here.
	 *   PWS 2000/05/02
	 */
	optcind = 0;
	zoptind++;
	zsfree(zoptarg);
	zoptarg = p;
    } else {
	zsfree(zoptarg);
	zoptarg = ztrdup("");
    }

    setsparam(var, metafy(optbuf, lenoptbuf, META_DUP));
    return 0;
}

/* Boolean flag that we should exit the shell as soon as all functions return.
 *
 * Set by the 'exit' builtin.
 */

/**/
mod_export volatile int exit_pending;

/* Shell level at which we exit if exit_pending */
/**/
mod_export volatile int exit_level;

/* we have printed a 'you have stopped (running) jobs.' message */

/**/
mod_export volatile int stopmsg;

/* break, bye, continue, exit, logout, return -- most of these take   *
 * one numeric argument, and the other (logout) is related to return. *
 * (return is treated as a logout when in a login shell.)             */

/**/
int
bin_break(char *name, char **argv, UNUSED(Options ops), int func)
{
    int num = lastval, nump = 0, implicit;

    /* handle one optional numeric argument */
    implicit = !*argv;
    if (*argv) {
	num = mathevali(*argv++);
	nump = 1;
    }

    if (nump > 0 && (func == BIN_CONTINUE || func == BIN_BREAK) && num <= 0) {
	zerrnam(name, "argument is not positive: %d", num);
	return 1;
    }

    switch (func) {
    case BIN_CONTINUE:
	if (!loops) {   /* continue is only permitted in loops */
	    zerrnam(name, "not in while, until, select, or repeat loop");
	    return 1;
	}
	contflag = 1; /* FALLTHROUGH */
    case BIN_BREAK:
	if (!loops) {   /* break is only permitted in loops */
	    zerrnam(name, "not in while, until, select, or repeat loop");
	    return 1;
	}
	breaks = nump ? minimum(num,loops) : 1;
	break;
    case BIN_RETURN:
	if ((isset(INTERACTIVE) && isset(SHINSTDIN))
	    || locallevel || sourcelevel) {
	    retflag = 1;
	    breaks = loops;
	    lastval = num;
	    if (trap_state == TRAP_STATE_PRIMED && trap_return == -2
		/*
		 * With POSIX, "return" on its own in a trap doesn't
		 * update $? --- we keep the status from before the
		 * trap.
		 */
		&& !(isset(POSIXTRAPS) && implicit)) {
		trap_state = TRAP_STATE_FORCE_RETURN;
		trap_return = lastval;
	    }
	    return lastval;
	}
	zexit(num, ZEXIT_NORMAL);	/* else treat return as logout/exit */
	break;
    case BIN_LOGOUT:
	if (unset(LOGINSHELL)) {
	    zerrnam(name, "not login shell");
	    return 1;
	}
	/*FALLTHROUGH*/
    case BIN_EXIT:
	if (locallevel > forklevel && shell_exiting != -1) {
	    /*
	     * We don't exit directly from functions to allow tidying
	     * up, in particular EXIT traps.  We still need to perform
	     * the usual interactive tests to see if we can exit at
	     * all, however.
	     *
	     * If we are forked, we exit the shell at the function depth
	     * at which we became a subshell, hence the comparison.
	     *
	     * If we are already exiting... give this all up as
	     * a bad job.
	     */
	    if (stopmsg || (zexit(0, ZEXIT_DEFERRED), !stopmsg)) {
		if (trap_state) 
		    trap_state = TRAP_STATE_FORCE_RETURN;
		retflag = 1;
		breaks = loops;
		exit_pending = 1;
		exit_level = locallevel;
		exit_val = num;
	    }
	} else
	    zexit(num, ZEXIT_NORMAL);
	break;
    }
    return 0;
}

/* check to see if user has jobs running/stopped */

/**/
static void
checkjobs(void)
{
    int i;

    for (i = 1; i <= maxjob; i++)
	if (i != thisjob && (jobtab[i].stat & STAT_LOCKED) &&
	    !(jobtab[i].stat & STAT_NOPRINT) &&
	    (isset(CHECKRUNNINGJOBS) || jobtab[i].stat & STAT_STOPPED))
	    break;
    if (i <= maxjob) {
	if (jobtab[i].stat & STAT_STOPPED) {

#ifdef USE_SUSPENDED
	    zerr("you have suspended jobs.");
#else
	    zerr("you have stopped jobs.");
#endif

	} else
	    zerr("you have running jobs.");
	stopmsg = 1;
    }
}

/*
 * -1 if the shell is already committed to exit.
 * positive if zexit() was already called.
 */

/**/
int shell_exiting;

/*
 * Exit status if explicitly set by an exit command.
 * This is complicated by the fact the exit command may be within
 * a function whose state we need to unwind (exit_pending set
 * and the exit will happen up the stack), or we may need to execute
 * additional code such as a trap after we are committed to exiting
 * (shell_exiting and the exit will happen down the stack).
 *
 * It's lucky this is all so obvious there is no possibility of any
 * bugs.  (C.f. the entire rest of the shell.)
 */
/**/
int exit_val;

/*
 * Actually exit the shell, working out the status locally.
 * This is exit_val if "exit" has explicitly been called in the shell,
 * else lastval.
 */

/**/
void
realexit(void)
{
    exit((shell_exiting || exit_pending) ? exit_val : lastval);
}

/* As realexit(), but call _exit instead */

/**/
void
_realexit(void)
{
    _exit((shell_exiting || exit_pending) ? exit_val : lastval);
}

/* exit the shell.  val is the return value of the shell.  *
 * from_where is
 *   ZEXIT_SIGNAL   if zexit is called because of a signal
 *   ZEXIT_DEFERRED if we can't actually exit yet (e.g., functions need
 *                  terminating) but should perform the usual interactive
 *                  tests.
 */

/**/
mod_export void
zexit(int val, enum zexit_t from_where)
{
    /*
     * Don't do anything recursively:  see below.
     * Do, however, update exit status --- there's no nesting,
     * a later value always overrides an earlier.
     */
    exit_val = val;
    if (shell_exiting == -1) {
	retflag = 1;
	breaks = loops;
	return;
    }

    if (isset(MONITOR) && !stopmsg && from_where != ZEXIT_SIGNAL) {
	scanjobs();    /* check if jobs need printing           */
	if (isset(CHECKJOBS))
	    checkjobs();   /* check if any jobs are running/stopped */
	if (stopmsg) {
	    stopmsg = 2;
	    return;
	}
    }
    /* Positive shell_exiting means we have been here before */
    if (from_where == ZEXIT_DEFERRED ||
	(shell_exiting++ && from_where != ZEXIT_NORMAL))
	return;

    /*
     * We're now committed to exiting.  Set shell_exiting to -1 to
     * indicate we shouldn't do any recursive processing.
     */
    shell_exiting = -1;
    /*
     * We want to do all remaining processing regardless of preceding
     * errors, even user interrupts.
     */
    errflag = 0;

    if (isset(MONITOR)) {
	/* send SIGHUP to any jobs left running  */
	killrunjobs(from_where == ZEXIT_SIGNAL);
    }
    cleanfilelists();
    if (isset(RCS) && interact) {
	if (!nohistsave) {
	    int writeflags = HFILE_USE_OPTIONS;
	    if (from_where == ZEXIT_SIGNAL)
		writeflags |= HFILE_NO_REWRITE;
	    saveandpophiststack(1, writeflags);
	    savehistfile(NULL, 1, writeflags);
	}
	if (islogin && !subsh) {
	    sourcehome(".zlogout");
#ifdef GLOBAL_ZLOGOUT
	    if (isset(RCS) && isset(GLOBALRCS))
		source(GLOBAL_ZLOGOUT);
#endif
	}
    }
    lastval = exit_val;
    /*
     * Now we are committed to exiting any previous state
     * is irrelevant.  Ensure trap can run.
     */
    errflag = intrap = 0;
    if (sigtrapped[SIGEXIT])
	dotrap(SIGEXIT);
    callhookfunc("zshexit", NULL, 1, NULL);
    runhookdef(EXITHOOK, NULL);
    if (opts[MONITOR] && interact && (SHTTY != -1)) {
       release_pgrp();
    }
    if (mypid != getpid())
	_exit(exit_val);
    else
	exit(exit_val);
}

/* . (dot), source */

/**/
int
bin_dot(char *name, char **argv, UNUSED(Options ops), UNUSED(int func))
{
    char **old, *old0 = NULL;
    int diddot = 0, dotdot = 0;
    char *s, **t, *enam, *arg0, *buf;
    struct stat st;
    enum source_return ret;

    if (!*argv)
	return 0;
    old = pparams;
    /* get arguments for the script */
    if (argv[1])
	pparams = zarrdup(argv + 1);

    enam = arg0 = ztrdup(*argv);
    if (isset(FUNCTIONARGZERO)) {
	old0 = argzero;
	argzero = ztrdup(arg0);
    }
    s = unmeta(enam);
    errno = ENOENT;
    ret = SOURCE_NOT_FOUND;
    /* for source only, check in current directory first */
    if (*name != '.' && access(s, F_OK) == 0
	&& stat(s, &st) >= 0 && !S_ISDIR(st.st_mode)) {
	diddot = 1;
	ret = source(enam);
    }
    if (ret == SOURCE_NOT_FOUND) {
	/* use a path with / in it */
	for (s = arg0; *s; s++)
	    if (*s == '/') {
		if (*arg0 == '.') {
		    if (arg0 + 1 == s)
			++diddot;
		    else if (arg0[1] == '.' && arg0 + 2 == s)
			++dotdot;
		}
		ret = source(arg0);
		break;
	    }
	if (!*s || (ret == SOURCE_NOT_FOUND &&
		    isset(PATHDIRS) && diddot < 2 && dotdot == 0)) {
	    pushheap();
	    /* search path for script */
	    for (t = path; *t; t++) {
		if (!(*t)[0] || ((*t)[0] == '.' && !(*t)[1])) {
		    if (diddot)
			continue;
		    diddot = 1;
		    buf = dupstring(arg0);
		} else
		    buf = zhtricat(*t, "/", arg0);

		s = unmeta(buf);
		if (access(s, F_OK) == 0 && stat(s, &st) >= 0
		    && !S_ISDIR(st.st_mode)) {
		    ret = source(enam = buf);
		    break;
		}
	    }
	    popheap();
	}
    }
    /* clean up and return */
    if (argv[1]) {
	freearray(pparams);
	pparams = old;
    }
    if (ret == SOURCE_NOT_FOUND) {
	if (isset(POSIXBUILTINS)) {
	    /* hard error in POSIX (we'll exit later) */
	    zerrnam(name, "%e: %s", errno, enam);
	} else {
	    zwarnnam(name, "%e: %s", errno, enam);
	}
    }
    zsfree(arg0);
    if (old0) {
	zsfree(argzero);
	argzero = old0;
    }
    return ret == SOURCE_OK ? lastval : 128 - ret;
}

/*
 * common for bin_emulate and bin_eval
 */

static int
eval(char **argv)
{
    Eprog prog;
    char *oscriptname = scriptname;
    int oineval = ineval, fpushed;
    struct funcstack fstack;

    /*
     * If EVALLINENO is not set, we use the line number of the
     * environment and must flag this up to exec.c.  Otherwise,
     * we use a special script name to indicate the special line number.
     */
    ineval = !isset(EVALLINENO);
    if (!ineval) {
	scriptname = "(eval)";
	fstack.prev = funcstack;
	fstack.name = scriptname;
	fstack.caller = funcstack ? funcstack->name : dupstring(argzero);
	fstack.lineno = lineno;
	fstack.tp = FS_EVAL;

	/*
	 * To get file line numbers, we need to know if parent is
	 * the original script/shell or a sourced file, in which
	 * case we use the line number raw, or a function or eval,
	 * in which case we need to deduce where that came from.
	 *
	 * This replicates the logic for working out the information
	 * for $funcfiletrace---eval is similar to an inlined function
	 * call from a tracing perspective.
	 */
	if (!funcstack || funcstack->tp == FS_SOURCE) {
	    fstack.flineno = fstack.lineno;
	    fstack.filename = fstack.caller;
	} else {
	    fstack.flineno = funcstack->flineno + lineno;
	    /*
	     * Line numbers in eval start from 1, not zero,
	     * so offset by one to get line in file.
	     */
	    if (funcstack->tp == FS_EVAL)
		fstack.flineno--;
	    fstack.filename = funcstack->filename;
	    if (!fstack.filename)
		fstack.filename = "";
	}
	funcstack = &fstack;

	fpushed = 1;
    } else
	fpushed = 0;

    prog = parse_string(zjoin(argv, ' ', 1), 1);
    if (prog) {
	if (wc_code(*prog->prog) != WC_LIST) {
	    /* No code to execute */
	    lastval = 0;
	} else {
	    execode(prog, 1, 0, "eval");

	    if (errflag && !lastval)
		lastval = errflag;
	}
    } else {
	lastval = 1;
    }

    if (fpushed)
	funcstack = funcstack->prev;

    errflag &= ~ERRFLAG_ERROR;
    scriptname = oscriptname;
    ineval = oineval;

    return lastval;
}

/* emulate: set emulation mode and optionally evaluate shell code */

/**/
int
bin_emulate(char *nam, char **argv, Options ops, UNUSED(int func))
{
    int opt_L = OPT_ISSET(ops, 'L');
    int opt_R = OPT_ISSET(ops, 'R');
    int opt_l = OPT_ISSET(ops, 'l');
    int saveemulation, savehackchar;
    int ret = 1, new_emulation;
    unsigned int savepatterns;
    char saveopts[OPT_SIZE], new_opts[OPT_SIZE];
    char *cmd = 0;
    const char *shname = *argv;
    LinkList optlist;
    LinkNode optnode;
    Emulation_options save_sticky;
    OptIndex *on_ptr, *off_ptr;

    /* without arguments just print current emulation */
    if (!shname) {
	if (opt_L || opt_R) {
	    zwarnnam(nam, "not enough arguments");
	    return 1;
	}

	switch(SHELL_EMULATION()) {
	case EMULATE_CSH:
	    shname = "csh";
	    break;

	case EMULATE_KSH:
	    shname = "ksh";
	    break;

	case EMULATE_SH:
	    shname = "sh";
	    break;

	default:
	    shname = "zsh";
	    break;
	}

	printf("%s\n", shname);
	return 0;
    }

    /* with single argument set current emulation */
    if (!argv[1]) {
	char *cmdopts;
	if (opt_l) {
	    cmdopts = (char *)zhalloc(OPT_SIZE);
	    memcpy(cmdopts, opts, OPT_SIZE);
	} else
	    cmdopts = opts;
	emulate(shname, opt_R, &emulation, cmdopts);
	if (opt_L)
	    cmdopts[LOCALOPTIONS] = cmdopts[LOCALTRAPS] =
		cmdopts[LOCALPATTERNS] = 1;
	if (opt_l) {
	    list_emulate_options(cmdopts, opt_R);
	    return 0;
	}
	clearpatterndisables();
	return 0;
    }

    if (opt_l) {
	zwarnnam(nam, "too many arguments for -l");
	return 1;
    }

    argv++;
    memcpy(saveopts, opts, sizeof(opts));
    memcpy(new_opts, opts, sizeof(opts));
    savehackchar = keyboardhackchar;
    emulate(shname, opt_R, &new_emulation, new_opts);
    optlist = newlinklist();
    if (parseopts(nam, &argv, new_opts, &cmd, optlist, 0, NULL)) {
	ret = 1;
	goto restore;
    }

    /* parseopts() has consumed anything that looks like an option */
    if (*argv) {
	zwarnnam(nam, "unknown argument %s", *argv);
	goto restore;
    }

    savepatterns = savepatterndisables();
    /*
     * All emulations start with an empty set of pattern disables,
     * hence no special "sticky" behaviour is required.
     */
    clearpatterndisables();

    saveemulation = emulation;
    emulation = new_emulation;
    memcpy(opts, new_opts, sizeof(opts));
    /* If "-c command" is given, evaluate command using specified
     * emulation mode.
     */
    if (cmd) {
	if (opt_L) {
	    zwarnnam(nam, "option -L incompatible with -c");
	    goto restore2;
	}
	*--argv = cmd;	/* on stack, never free()d, see execbuiltin() */
    } else {
	if (opt_L)
	    opts[LOCALOPTIONS] = opts[LOCALTRAPS] = opts[LOCALPATTERNS] = 1;
	return 0;
    }

    save_sticky = sticky;
    sticky = hcalloc(sizeof(*sticky));
    sticky->emulation = emulation;
    for (optnode = firstnode(optlist); optnode; incnode(optnode)) {
	/* Data is index into new_opts */
	char *optptr = (char *)getdata(optnode);
	if (*optptr)
	    sticky->n_on_opts++;
	else
	    sticky->n_off_opts++;
    }
    if (sticky->n_on_opts)
	on_ptr = sticky->on_opts =
	    zhalloc(sticky->n_on_opts * sizeof(*sticky->on_opts));
    else
	on_ptr = NULL;
    if (sticky->n_off_opts)
	off_ptr = sticky->off_opts = zhalloc(sticky->n_off_opts *
					     sizeof(*sticky->off_opts));
    else
	off_ptr = NULL;
    for (optnode = firstnode(optlist); optnode; incnode(optnode)) {
	/* Data is index into new_opts */
	char *optptr = (char *)getdata(optnode);
	int optno = optptr - new_opts;
	if (*optptr)
	    *on_ptr++ = optno;
	else
	    *off_ptr++ = optno;
    }
    ret = eval(argv);
    sticky = save_sticky;
restore2:
    emulation = saveemulation;
    memcpy(opts, saveopts, sizeof(opts));
    restorepatterndisables(savepatterns);
restore:
    keyboardhackchar = savehackchar;
    inittyptab();	/* restore banghist */
    return ret;
}

/* eval: simple evaluation */

/**/
mod_export int ineval;

/**/
int
bin_eval(UNUSED(char *nam), char **argv, UNUSED(Options ops), UNUSED(int func))
{
    return eval(argv);
}

static char *zbuf;
static int readfd;

/* Read a character from readfd, or from the buffer zbuf.  Return EOF on end of
file/buffer. */

/* read: get a line of input, or (for compctl functions) return some *
 * useful data about the state of the editing line.  The -E and -e   *
 * options mean that the result should be sent to stdout.  -e means, *
 * in addition, that the result should not actually be assigned to   *
 * the specified parameters.                                         */

/**/
int
bin_read(char *name, char **args, Options ops, UNUSED(int func))
{
    char *reply, *readpmpt;
    int bsiz, c = 0, gotnl = 0, al = 0, first, nchars = 1, bslash, keys = 0;
    int haso = 0;	/* true if /dev/tty has been opened specially */
    int isem = !strcmp(term, "emacs"), izle = zleactive;
    char *buf, *bptr, *firstarg, *zbuforig;
    LinkList readll = newlinklist();
    FILE *oshout = NULL;
    int readchar = -1, val, resettty = 0;
    struct ttyinfo saveti;
    char d;
    long izle_timeout = 0;
#ifdef MULTIBYTE_SUPPORT
    wchar_t delim = L'\n', wc;
    mbstate_t mbs;
    char *laststart;
    size_t ret;
#else
    char delim = '\n';
#endif

    if (OPT_HASARG(ops,c='k')) {
	char *eptr, *optarg = OPT_ARG(ops,c);
	nchars = (int)zstrtol(optarg, &eptr, 10);
	if (*eptr) {
	    zwarnnam(name, "number expected after -%c: %s", c, optarg);
	    return 1;
	}
    }
    /* This `*args++ : *args' looks a bit weird, but it works around a bug
     * in gcc-2.8.1 under DU 4.0. */
    firstarg = (*args && **args == '?' ? *args++ : *args);
    reply = *args ? *args++ : OPT_ISSET(ops,'A') ? "reply" : "REPLY";

    if (OPT_ISSET(ops,'A') && *args) {
	zwarnnam(name, "only one array argument allowed");
	return 1;
    }

    /* handle compctl case */
    if(OPT_ISSET(ops,'l') || OPT_ISSET(ops,'c'))
	return compctlreadptr(name, args, ops, reply);

    if ((OPT_ISSET(ops,'k') || OPT_ISSET(ops,'q')) &&
	!OPT_ISSET(ops,'u') && !OPT_ISSET(ops,'p')) {
	if (!zleactive) {
	    if (SHTTY == -1) {
		/* need to open /dev/tty specially */
		if ((SHTTY = open("/dev/tty", O_RDWR|O_NOCTTY)) != -1) {
		    haso = 1;
		    oshout = shout;
		    init_shout();
		}
	    } else if (!shout) {
		/* We need an output FILE* on the tty */
		init_shout();
	    }
	    /* We should have a SHTTY opened by now. */
	    if (SHTTY == -1) {
		/* Unfortunately, we didn't. */
		fprintf(stderr, "not interactive and can't open terminal\n");
		fflush(stderr);
		return 1;
	    }
	    if (unset(INTERACTIVE))
		gettyinfo(&shttyinfo);
	    /* attach to the tty */
	    attachtty(mypgrp);
	    if (!isem)
		setcbreak();
	    readfd = SHTTY;
	}
	keys = 1;
    } else if (OPT_HASARG(ops,'u') && !OPT_ISSET(ops,'p')) {
	/* -u means take input from the specified file descriptor. */
	char *eptr, *argptr = OPT_ARG(ops,'u');
	/* The old code handled -up, but that was never documented. Still...*/
	if (!strcmp(argptr, "p")) {
	    readfd = coprocin;
	    if (readfd < 0) {
		zwarnnam(name, "-p: no coprocess");
		return 1;
	    }
	} else {
	    readfd = (int)zstrtol(argptr, &eptr, 10);
	    if (*eptr) {
		zwarnnam(name, "number expected after -%c: %s", 'u', argptr);
		return 1;
	    }
	}
#if 0
	/* This code is left as a warning to future generations --- pws. */
	for (readfd = 9; readfd && !OPT_ISSET(ops,readfd + '0'); --readfd);
#endif
	izle = 0;
    } else if (OPT_ISSET(ops,'p')) {
	readfd = coprocin;
	if (readfd < 0) {
	    zwarnnam(name, "-p: no coprocess");
	    return 1;
	}
	izle = 0;
    } else
	readfd = izle = 0;

    if (OPT_ISSET(ops,'s') && SHTTY != -1) {
	struct ttyinfo ti;
	gettyinfo(&ti);
	saveti = ti;
	resettty = 1;
#ifdef HAS_TIO
	ti.tio.c_lflag &= ~ECHO;
#else
	ti.sgttyb.sg_flags &= ~ECHO;
#endif
	settyinfo(&ti);
    }

    /* handle prompt */
    if (firstarg) {
	for (readpmpt = firstarg;
	     *readpmpt && *readpmpt != '?'; readpmpt++);
	if (*readpmpt++) {
	    if (keys || isatty(0)) {
		zputs(readpmpt, (shout ? shout : stderr));
		fflush(shout ? shout : stderr);
	    }
	    readpmpt[-1] = '\0';
	}
    }

    if (OPT_ISSET(ops,'d')) {
	char *delimstr = OPT_ARG(ops,'d');
#ifdef MULTIBYTE_SUPPORT
	wint_t wi;

	if (isset(MULTIBYTE)) {
	    mb_charinit();
	    (void)mb_metacharlenconv(delimstr, &wi);
	}
	else
	    wi = WEOF;
	if (wi != WEOF)
	    delim = (wchar_t)wi;
	else
	    delim = (wchar_t)((delimstr[0] == Meta) ?
			      delimstr[1] ^ 32 : delimstr[0]);
#else
        delim = (delimstr[0] == Meta) ? delimstr[1] ^ 32 : delimstr[0];
#endif
	if (SHTTY != -1) {
	    struct ttyinfo ti;
	    gettyinfo(&ti);
	    if (! resettty) {
	      saveti = ti;
	      resettty = 1;
	    }
#ifdef HAS_TIO
	    ti.tio.c_lflag &= ~ICANON;
	    ti.tio.c_cc[VMIN] = 1;
	    ti.tio.c_cc[VTIME] = 0;
#else
	    ti.sgttyb.sg_flags |= CBREAK;
#endif
	    settyinfo(&ti);
	}
    }
    if (OPT_ISSET(ops,'t')) {
	zlong timeout = 0;
	if (OPT_HASARG(ops,'t')) {
	    mnumber mn = zero_mnumber;
	    mn = matheval(OPT_ARG(ops,'t'));
	    if (errflag)
		return 1;
	    if (mn.type == MN_FLOAT) {
		mn.u.d *= 1e6;
		timeout = (zlong)mn.u.d;
	    } else {
		timeout = (zlong)mn.u.l * (zlong)1000000;
	    }
	}
	if (izle) {
	    /*
	     * Timeout is in 100ths of a second rather than us.
	     * See calc_timeout() in zle_main for format of this.
	     */
	    timeout = -(timeout/(zlong)10000 + 1L);
	    izle_timeout = (long)timeout;
#ifdef LONG_MAX
	    /* saturate if range exceeded */
	    if ((zlong)izle_timeout != timeout)
		izle_timeout = LONG_MAX;
#endif
	} else {
	    if (readfd == -1 ||
		!read_poll(readfd, &readchar, keys && !zleactive,
			   timeout)) {
		if (keys && !zleactive && !isem)
		    settyinfo(&shttyinfo);
		else if (resettty && SHTTY != -1)
		    settyinfo(&saveti);
		if (haso) {
		    fclose(shout);
		    shout = oshout;
		    SHTTY = -1;
		}
		return OPT_ISSET(ops,'q') ? 2 : 1;
	    }
	}
    }

#ifdef MULTIBYTE_SUPPORT
    memset(&mbs, 0, sizeof(mbs));
#endif

    /*
     * option -k means read only a given number of characters (default 1)
     * option -q means get one character, and interpret it as a Y or N
     */
    if (OPT_ISSET(ops,'k') || OPT_ISSET(ops,'q')) {
	int eof = 0;
	/* allocate buffer space for result */
#ifdef MULTIBYTE_SUPPORT
	bptr = buf = (char *)zalloc(nchars*MB_CUR_MAX+1);
#else
	bptr = buf = (char *)zalloc(nchars+1);
#endif

	do {
	    if (izle) {
		zleentry(ZLE_CMD_GET_KEY, izle_timeout, NULL, &val);
		if (val < 0) {
		    eof = 1;
		    break;
		}
		*bptr = (char) val;
#ifdef MULTIBYTE_SUPPORT
		if (isset(MULTIBYTE)) {
		    ret = mbrlen(bptr++, 1, &mbs);
		    if (ret == MB_INVALID)
			memset(&mbs, 0, sizeof(mbs));
		    /* treat invalid as single character */
		    if (ret != MB_INCOMPLETE)
			nchars--;
		    continue;
		} else {
		    bptr++;
		    nchars--;
		}
#else
		bptr++;
		nchars--;
#endif
	    } else {
		/* If read returns 0, is end of file */
		if (readchar >= 0) {
		    *bptr = readchar;
		    val = 1;
		    readchar = -1;
		} else {
		    while ((val = read(readfd, bptr, nchars)) < 0) {
			if (errno != EINTR ||
			    errflag || retflag || breaks || contflag)
			    break;
		    }
		    if (val <= 0) {
			eof = 1;
			break;
		    }
		}

#ifdef MULTIBYTE_SUPPORT
		if (isset(MULTIBYTE)) {
		    while (val > 0) {
			ret = mbrlen(bptr, val, &mbs);
			if (ret == MB_INCOMPLETE) {
			    bptr += val;
			    break;
			} else {
			    if (ret == MB_INVALID) {
				memset(&mbs, 0, sizeof(mbs));
				/* treat as single byte */
				ret = 1;
			    }
			    else if (ret == 0) /* handle null as normal char */
				ret = 1;
			    else if (ret > (size_t)val) {
				/* Some mbrlen()s return the full char len */
				ret = val;
			    }
			    nchars--;
			    val -= ret;
			    bptr += ret;
			}
		    }
		    continue;
		}
#endif
		/* decrement number of characters read from number required */
		nchars -= val;

		/* increment pointer past read characters */
		bptr += val;
	    }
	} while (nchars > 0);

	if (!izle && !OPT_ISSET(ops,'u') && !OPT_ISSET(ops,'p')) {
	    /* dispose of result appropriately, etc. */
	    if (isem)
		while (val > 0 && read(SHTTY, &d, 1) == 1 && d != '\n');
	    else {
		settyinfo(&shttyinfo);
		resettty = 0;
	    }
	    if (haso) {
		fclose(shout);	/* close(SHTTY) */
		shout = oshout;
		SHTTY = -1;
	    }
	}

	if (OPT_ISSET(ops,'q'))
	{
	    /*
	     * Keep eof as status but status is now whether we read
	     * 'y' or 'Y'.  If we timed out, status is 2.
	     */
	    if (eof)
		eof = 2;
	    else
		eof = (bptr - buf != 1 || (buf[0] != 'y' && buf[0] != 'Y'));
	    buf[0] = eof ? 'n' : 'y';
	    bptr = buf + 1;
	}
	if (OPT_ISSET(ops,'e') || OPT_ISSET(ops,'E'))
	    fwrite(buf, bptr - buf, 1, stdout);
	if (!OPT_ISSET(ops,'e'))
	    setsparam(reply, metafy(buf, bptr - buf, META_REALLOC));
	else
	    zfree(buf, bptr - buf + 1);
	if (resettty && SHTTY != -1)
	    settyinfo(&saveti);
	return eof;
    }

    /* All possible special types of input have been exhausted.  Take one line,
       and assign words to the parameters until they run out.  Leftover words go
       onto the last parameter.  If an array is specified, all the words become
       separate elements of the array. */

    zbuforig = zbuf = (!OPT_ISSET(ops,'z')) ? NULL :
	(nonempty(bufstack)) ? (char *) getlinknode(bufstack) : ztrdup("");
    first = 1;
    bslash = 0;
    while (*args || (OPT_ISSET(ops,'A') && !gotnl)) {
	sigset_t s = child_unblock();
	buf = bptr = (char *)zalloc(bsiz = 64);
#ifdef MULTIBYTE_SUPPORT
	laststart = buf;
	ret = MB_INCOMPLETE;
#endif
	/* get input, a character at a time */
	while (!gotnl) {
	    c = zread(izle, &readchar, izle_timeout);
	    /* \ at the end of a line indicates a continuation *
	     * line, except in raw mode (-r option)            */
#ifdef MULTIBYTE_SUPPORT
	    if (c == EOF) {
		/* not waiting to be completed any more */
		ret = 0;
		break;
	    }
	    *bptr = (char)c;
	    if (isset(MULTIBYTE)) {
		ret = mbrtowc(&wc, bptr, 1, &mbs);
		if (!ret)	/* NULL */
		    ret = 1;
	    } else {
		ret = 1;
		wc = (wchar_t)c;
	    }
	    if (ret != MB_INCOMPLETE) {
		if (ret == MB_INVALID) {
		    memset(&mbs, 0, sizeof(mbs));
		    /* Treat this as a single character */
		    wc = (wchar_t)c;
		    laststart = bptr;
		}
		if (bslash && wc == delim) {
		    bslash = 0;
		    continue;
		}
		if (wc == delim)
		    break;
		/*
		 * `first' is non-zero if any separator we encounter is a
		 * non-whitespace separator, which means that anything
		 * (even an empty string) between, before or after separators
		 * is significant.  If it is zero, we have a whitespace
		 * separator, which shouldn't cause extra empty strings to
		 * be emitted.  Hence the test for (*buf || first) when
		 * we assign the result of reading a word.
		 */
		if (!bslash && wcsitype(wc, ISEP)) {
		    if (bptr != buf ||
			(!(c < 128 && iwsep(c)) && first)) {
			first |= !(c < 128 && iwsep(c));
			break;
		    }
		    first |= !(c < 128 && iwsep(c));
		    continue;
		}
		bslash = (wc == L'\\' && !bslash && !OPT_ISSET(ops,'r'));
		if (bslash)
		    continue;
		first = 0;
	    }
	    if (imeta(STOUC(*bptr))) {
		bptr[1] = bptr[0] ^ 32;
		bptr[0] = Meta;
		bptr += 2;
	    }
	    else
		bptr++;
	    if (ret != MB_INCOMPLETE)
		laststart = bptr;
#else
	    if (c == EOF)
		break;
	    if (bslash && c == delim) {
		bslash = 0;
		continue;
	    }
	    if (c == delim)
		break;
	    /*
	     * `first' is non-zero if any separator we encounter is a
	     * non-whitespace separator, which means that anything
	     * (even an empty string) between, before or after separators
	     * is significant.  If it is zero, we have a whitespace
	     * separator, which shouldn't cause extra empty strings to
	     * be emitted.  Hence the test for (*buf || first) when
	     * we assign the result of reading a word.
	     */
	    if (!bslash && isep(c)) {
		if (bptr != buf || (!iwsep(c) && first)) {
		    first |= !iwsep(c);
		    break;
		}
		first |= !iwsep(c);
		continue;
	    }
	    bslash = c == '\\' && !bslash && !OPT_ISSET(ops,'r');
	    if (bslash)
		continue;
	    first = 0;
	    if (imeta(c)) {
		*bptr++ = Meta;
		*bptr++ = c ^ 32;
	    } else
		*bptr++ = c;
#endif
	    /* increase the buffer size, if necessary */
	    if (bptr >= buf + bsiz - 1) {
		int blen = bptr - buf;
#ifdef MULTIBYTE_SUPPORT
		int llen = laststart - buf;
#endif

		buf = realloc(buf, bsiz *= 2);
		bptr = buf + blen;
#ifdef MULTIBYTE_SUPPORT
		laststart = buf + llen;
#endif
	    }
	}
	signal_setmask(s);
#ifdef MULTIBYTE_SUPPORT
	if (c == EOF) {
	    gotnl = 1;
	    *bptr = '\0';	/* see below */
	} else if (ret == MB_INCOMPLETE) {
	    /*
	     * We can only get here if there is an EOF in the
	     * middle of a character... safest to keep the debris,
	     * I suppose.
	     */
	    *bptr = '\0';
	} else {
	    if (wc == delim)
		gotnl = 1;
	    *laststart = '\0';
	}
#else
	if (c == delim || c == EOF)
	    gotnl = 1;
	*bptr = '\0';
#endif
	/* dispose of word appropriately */
	if (OPT_ISSET(ops,'e') ||
	    /*
	     * When we're doing an array assignment, we'll
	     * handle echoing at that point.  In all other
	     * cases (including -A with no assignment)
	     * we'll do it here.
	     */
	    (OPT_ISSET(ops,'E') && !OPT_ISSET(ops,'A'))) {
	    zputs(buf, stdout);
	    putchar('\n');
	}
	if (!OPT_ISSET(ops,'e') && (*buf || first || gotnl)) {
	    if (OPT_ISSET(ops,'A')) {
		addlinknode(readll, buf);
		al++;
	    } else
		setsparam(reply, buf);
	} else
	    free(buf);
	if (!OPT_ISSET(ops,'A'))
	    reply = *args++;
    }
    /* handle EOF */
    if (c == EOF) {
	if (readfd == coprocin) {
	    close(coprocin);
	    close(coprocout);
	    coprocin = coprocout = -1;
	}
    }
    /* final assignment (and display) of array parameter */
    if (OPT_ISSET(ops,'A')) {
	char **pp, **p = NULL;
	LinkNode n;

	p = (OPT_ISSET(ops,'e') ? (char **)NULL
	     : (char **)zalloc((al + 1) * sizeof(char *)));

	for (pp = p, n = firstnode(readll); n; incnode(n)) {
	    if (OPT_ISSET(ops,'E')) {
		zputs((char *) getdata(n), stdout);
		putchar('\n');
	    }
	    if (p)
		*pp++ = (char *)getdata(n);
	    else
		zsfree(getdata(n));
	}
	if (p) {
	    *pp++ = NULL;
	    setaparam(reply, p);
	}
	if (resettty && SHTTY != -1)
	    settyinfo(&saveti);
	return c == EOF;
    }
    buf = bptr = (char *)zalloc(bsiz = 64);
#ifdef MULTIBYTE_SUPPORT
    laststart = buf;
    ret = MB_INCOMPLETE;
#endif
    /* any remaining part of the line goes into one parameter */
    bslash = 0;
    if (!gotnl) {
	sigset_t s = child_unblock();
	for (;;) {
	    c = zread(izle, &readchar, izle_timeout);
#ifdef MULTIBYTE_SUPPORT
	    if (c == EOF) {
		/* not waiting to be completed any more */
		ret = 0;
		break;
	    }
	    *bptr = (char)c;
	    if (isset(MULTIBYTE)) {
		ret = mbrtowc(&wc, bptr, 1, &mbs);
		if (!ret)	/* NULL */
		    ret = 1;
	    } else {
		ret = 1;
		wc = (wchar_t)c;
	    }
	    if (ret != MB_INCOMPLETE) {
		if (ret == MB_INVALID) {
		    memset(&mbs, 0, sizeof(mbs));
		    /* Treat this as a single character */
		    wc = (wchar_t)c;
		    laststart = bptr;
		}
		/*
		 * \ at the end of a line introduces a continuation line,
		 * except in raw mode (-r option)
		 */
		if (bslash && wc == delim) {
		    bslash = 0;
		    continue;
		}
		if (wc == delim && !zbuf)
		    break;
		if (!bslash && bptr == buf && wcsitype(wc, ISEP)) {
		    if (c < 128 && iwsep(c))
			continue;
		    else if (!first) {
			first = 1;
			continue;
		    }
		}
		bslash = (wc == L'\\' && !bslash && !OPT_ISSET(ops,'r'));
		if (bslash)
		    continue;
	    }
	    if (imeta(STOUC(*bptr))) {
		bptr[1] = bptr[0] ^ 32;
		bptr[0] = Meta;
		bptr += 2;
	    }
	    else
		bptr++;
	    if (ret != MB_INCOMPLETE)
		laststart = bptr;
#else
	    /* \ at the end of a line introduces a continuation line, except in
	       raw mode (-r option) */
	    if (bslash && c == delim) {
		bslash = 0;
		continue;
	    }
	    if (c == EOF || (c == delim && !zbuf))
		break;
	    if (!bslash && isep(c) && bptr == buf) {
		if (iwsep(c))
		    continue;
		else if (!first) {
		    first = 1;
		    continue;
		}
	    }
	    bslash = c == '\\' && !bslash && !OPT_ISSET(ops,'r');
	    if (bslash)
		continue;
	    if (imeta(c)) {
		*bptr++ = Meta;
		*bptr++ = c ^ 32;
	    } else
		*bptr++ = c;
#endif
	    /* increase the buffer size, if necessary */
	    if (bptr >= buf + bsiz - 1) {
		int blen = bptr - buf;
#ifdef MULTIBYTE_SUPPORT
		int llen = laststart - buf;
#endif

		buf = realloc(buf, bsiz *= 2);
		bptr = buf + blen;
#ifdef MULTIBYTE_SUPPORT
		laststart = buf + llen;
#endif
	    }
	}
	signal_setmask(s);
    }
#ifdef MULTIBYTE_SUPPORT
    if (ret != MB_INCOMPLETE)
	bptr = laststart;
#endif
    /*
     * Strip trailing IFS whitespace.
     * iwsep can only be certain single-byte ASCII bytes, but we
     * must check the byte isn't metafied.
     */
    while (bptr > buf) {
	if (bptr > buf + 1 && bptr[-2] == Meta) {
	    /* non-ASCII, can't be IWSEP */
	    break;
	} else if (iwsep(bptr[-1]))
	    bptr--;
	else
	    break;
    }
    *bptr = '\0';
    if (resettty && SHTTY != -1)
	settyinfo(&saveti);
    /* final assignment of reply, etc. */
    if (OPT_ISSET(ops,'e') || OPT_ISSET(ops,'E')) {
	zputs(buf, stdout);
	putchar('\n');
    }
    if (!OPT_ISSET(ops,'e'))
	setsparam(reply, buf);
    else
	zsfree(buf);
    if (zbuforig) {
	char first = *zbuforig;

	zsfree(zbuforig);
	if (!first)
	    return 1;
    } else if (c == EOF) {
	if (readfd == coprocin) {
	    close(coprocin);
	    close(coprocout);
	    coprocin = coprocout = -1;
	}
	return 1;
    }
    /*
     * The following is to ensure a failure to set the parameter
     * causes a non-zero status return.  There are arguments for
     * turning a non-zero status into errflag more widely.
     */
    return errflag;
}

/**/
static int
zread(int izle, int *readchar, long izle_timeout)
{
    char cc, retry = 0;
    int ret;

    if (izle) {
	int c;
	zleentry(ZLE_CMD_GET_KEY, izle_timeout, NULL, &c);

	return (c < 0 ? EOF : c);
    }
    /* use zbuf if possible */
    if (zbuf) {
	/* If zbuf points to anything, it points to the next character in the
	   buffer.  This may be a null byte to indicate EOF.  If reading from the
	   buffer, move on the buffer pointer. */
	if (*zbuf == Meta)
	    return zbuf++, STOUC(*zbuf++ ^ 32);
	else
	    return (*zbuf) ? STOUC(*zbuf++) : EOF;
    }
    if (*readchar >= 0) {
	cc = *readchar;
	*readchar = -1;
	return STOUC(cc);
    }
    for (;;) {
	/* read a character from readfd */
	ret = read(readfd, &cc, 1);
	switch (ret) {
	case 1:
	    /* return the character read */
	    return STOUC(cc);
	case -1:
#if defined(EAGAIN) || defined(EWOULDBLOCK)
	    if (!retry && readfd == 0 && (
# ifdef EAGAIN
		errno == EAGAIN
#  ifdef EWOULDBLOCK
		||
#  endif /* EWOULDBLOCK */
# endif /* EAGAIN */
# ifdef EWOULDBLOCK
		errno == EWOULDBLOCK
# endif /* EWOULDBLOCK */
		) && setblock_stdin()) {
		retry = 1;
		continue;
	    } else
#endif /* EAGAIN || EWOULDBLOCK */
		if (errno == EINTR && !(errflag || retflag || breaks || contflag))
		    continue;
	    break;
	}
	return EOF;
    }
}

/* holds arguments for testlex() */
/**/
char **testargs, **curtestarg;

/* test, [: the old-style general purpose logical expression builtin */

/**/
void
testlex(void)
{
    if (tok == LEXERR)
	return;

    tokstr = *(curtestarg = testargs);
    if (!*testargs) {
	/* if tok is already zero, reading past the end:  error */
	tok = tok ? NULLTOK : LEXERR;
	return;
    } else if (!strcmp(*testargs, "-o"))
	tok = DBAR;
    else if (!strcmp(*testargs, "-a"))
	tok = DAMPER;
    else if (!strcmp(*testargs, "!"))
	tok = BANG;
    else if (!strcmp(*testargs, "("))
	tok = INPAR;
    else if (!strcmp(*testargs, ")"))
	tok = OUTPAR;
    else
	tok = STRING;
    testargs++;
}

/**/
int
bin_test(char *name, char **argv, UNUSED(Options ops), int func)
{
    char **s;
    Eprog prog;
    struct estate state;
    int nargs, sense = 0, ret;

    /* if "test" was invoked as "[", it needs a matching "]" *
     * which is subsequently ignored                         */
    if (func == BIN_BRACKET) {
	for (s = argv; *s; s++);
	if (s == argv || strcmp(s[-1], "]")) {
	    zwarnnam(name, "']' expected");
	    return 2;
	}
	s[-1] = NULL;
    }
    /* an empty argument list evaluates to false (1) */
    if (!*argv)
	return 1;

    /*
     * Implement some XSI extensions to POSIX here.
     * See
     * http://pubs.opengroup.org/onlinepubs/9699919799/utilities/test.html
     */
    nargs = arrlen(argv);
    if (nargs == 3 || nargs == 4)
    {
	/*
	 * As parentheses are an extension, we need to be careful ---
	 * if this is a three-argument expression that could
	 * be a binary operator, prefer that.
	 */
	if (!strcmp(argv[0], "(") && !strcmp(argv[nargs-1],")") &&
	    (nargs != 3 || !is_cond_binary_op(argv[1]))) {
	    argv[nargs-1] = NULL;
	    argv++;
	}
	if (nargs == 4 && !strcmp("!", argv[0])) {
	    sense = 1;
	    argv++;
	}
    }

    zcontext_save();
    testargs = argv;
    tok = NULLTOK;
    condlex = testlex;
    testlex();
    prog = parse_cond();
    condlex = zshlex;

    if (errflag) {
	errflag &= ~ERRFLAG_ERROR;
	zcontext_restore();
	return 2;
    }

    if (!prog || tok == LEXERR) {
	zwarnnam(name, tokstr ? "parse error" : "argument expected");
	zcontext_restore();
	return 2;
    }
    zcontext_restore();

    if (*curtestarg) {
	zwarnnam(name, "too many arguments");
	return 2;
    }

    /* syntax is OK, so evaluate */

    state.prog = prog;
    state.pc = prog->prog;
    state.strs = prog->strs;

    ret = evalcond(&state, name);
    if (ret < 2 && sense)
	ret = ! ret;

    return ret;
}

/* display a time, provided in units of 1/60s, as minutes and seconds */
#define pttime(X) printf("%ldm%ld.%02lds",((long) (X))/(60 * clktck),\
			 ((long) (X))/clktck%clktck,\
			 ((long) (X))*100/clktck%100)

/* times: display, in a two-line format, the times provided by times(3) */

/**/
int
bin_times(UNUSED(char *name), UNUSED(char **argv), UNUSED(Options ops), UNUSED(int func))
{
    struct tms buf;
    long clktck = get_clktck();

    /* get time accounting information */
    if (times(&buf) == -1)
	return 1;
    pttime(buf.tms_utime);	/* user time */
    putchar(' ');
    pttime(buf.tms_stime);	/* system time */
    putchar('\n');
    pttime(buf.tms_cutime);	/* user time, children */
    putchar(' ');
    pttime(buf.tms_cstime);	/* system time, children */
    putchar('\n');
    return 0;
}

/* trap: set/unset signal traps */

/**/
int
bin_trap(char *name, char **argv, UNUSED(Options ops), UNUSED(int func))
{
    Eprog prog;
    char *arg, *s;
    int sig;

    if (*argv && !strcmp(*argv, "--"))
	argv++;

    /* If given no arguments, list all currently-set traps */
    if (!*argv) {
	queue_signals();
	for (sig = 0; sig < VSIGCOUNT; sig++) {
	    if (sigtrapped[sig] & ZSIG_FUNC) {
		HashNode hn;

		if ((hn = gettrapnode(sig, 0)))
		    shfunctab->printnode(hn, 0);
		DPUTS(!hn, "BUG: I did not find any trap functions!");
	    } else if (sigtrapped[sig]) {
		const char *name = getsigname(sig);
		if (!siglists[sig])
		    printf("trap -- '' %s\n", name);
		else {
		    s = getpermtext(siglists[sig], NULL, 0);
		    printf("trap -- ");
		    quotedzputs(s, stdout);
		    printf(" %s\n", name);
		    zsfree(s);
		}
	    }
	}
	unqueue_signals();
	return 0;
    }

    /* If we have a signal number, unset the specified *
     * signals.  With only -, remove all traps.        */
    if ((getsignum(*argv) != -1) || (!strcmp(*argv, "-") && argv++)) {
	if (!*argv) {
	    for (sig = 0; sig < VSIGCOUNT; sig++)
		unsettrap(sig);
	} else {
	    for (; *argv; argv++) {
		sig = getsignum(*argv);
		if (sig == -1) {
		    zwarnnam(name, "undefined signal: %s", *argv);
		    break;
		}
		unsettrap(sig);
	    }
	}
	return *argv != NULL;
    }

    /* Sort out the command to execute on trap */
    arg = *argv++;
    if (!*arg)
	prog = &dummy_eprog;
    else if (!(prog = parse_string(arg, 1))) {
	zwarnnam(name, "couldn't parse trap command");
	return 1;
    }

    /* set traps */
    for (; *argv; argv++) {
	Eprog t;
	int flags;

	sig = getsignum(*argv);
	if (sig == -1) {
	    zwarnnam(name, "undefined signal: %s", *argv);
	    break;
	}
	if (idigit(**argv) ||
	    !strcmp(sigs[sig], *argv) ||
	    (!strncmp("SIG", *argv, 3) && !strcmp(sigs[sig], *argv+3))) {
	    /* The signal was specified by number or by canonical name (with
	     * or without SIG prefix).
	     */
	    flags = 0;
	}
	else {
	    /*
	     * Record that the signal is used under an assumed name.
	     * If we ever have more than one alias per signal this
	     * will need improving.
	     */
	    flags = ZSIG_ALIAS;
	}
	t = dupeprog(prog, 0);
	if (settrap(sig, t, flags))
	    freeeprog(t);
    }
    return *argv != NULL;
}

/**/
int
bin_ttyctl(UNUSED(char *name), UNUSED(char **argv), Options ops, UNUSED(int func))
{
    if (OPT_ISSET(ops,'f'))
	ttyfrozen = 1;
    else if (OPT_ISSET(ops,'u'))
	ttyfrozen = 0;
    else
	printf("tty is %sfrozen\n", ttyfrozen ? "" : "not ");
    return 0;
}

/* let -- mathematical evaluation */

/**/
int
bin_let(UNUSED(char *name), char **argv, UNUSED(Options ops), UNUSED(int func))
{
    mnumber val = zero_mnumber;

    while (*argv)
	val = matheval(*argv++);
    /* Errors in math evaluation in let are non-fatal. */
    errflag &= ~ERRFLAG_ERROR;
    /* should test for fabs(val.u.d) < epsilon? */
    return (val.type == MN_INTEGER) ? val.u.l == 0 : val.u.d == 0.0;
}

/* umask command.  umask may be specified as octal digits, or in the  *
 * symbolic form that chmod(1) uses.  Well, a subset of it.  Remember *
 * that only the bottom nine bits of umask are used, so there's no    *
 * point allowing the set{u,g}id and sticky bits to be specified.     */

/**/
int
bin_umask(char *nam, char **args, Options ops, UNUSED(int func))
{
    mode_t um;
    char *s = *args;

    /* Get the current umask. */
    queue_signals();
    um = umask(0777);
    umask(um);
    unqueue_signals();

    /* No arguments means to display the current setting. */
    if (!s) {
	if (OPT_ISSET(ops,'S')) {
	    char *who = "ugo";

	    while (*who) {
		char *what = "rwx";
		printf("%c=", *who++);
		while (*what) {
		    if (!(um & 0400))
			putchar(*what);
		    um <<= 1;
		    what++;
		}
		putchar(*who ? ',' : '\n');
	    }
	} else {
	    if (um & 0700)
		putchar('0');
	    printf("%03o\n", (unsigned)um);
	}
	return 0;
    }

    if (idigit(*s)) {
	/* Simple digital umask. */
	um = zstrtol(s, &s, 8);
	if (*s) {
	    zwarnnam(nam, "bad umask");
	    return 1;
	}
    } else {
	/* Symbolic notation -- slightly complicated. */
	int whomask, umaskop, mask;

	/* More than one symbolic argument may be used at once, each separated
	   by commas. */
	for (;;) {
	    /* First part of the argument -- who does this apply to?
	       u=owner, g=group, o=other. */
	    whomask = 0;
	    while (*s == 'u' || *s == 'g' || *s == 'o' || *s == 'a')
		if (*s == 'u')
		    s++, whomask |= 0700;
		else if (*s == 'g')
		    s++, whomask |= 0070;
		else if (*s == 'o')
		    s++, whomask |= 0007;
		else if (*s == 'a')
		    s++, whomask |= 0777;
	    /* Default whomask is everyone. */
	    if (!whomask)
		whomask = 0777;
	    /* Operation may be +, - or =. */
	    umaskop = (int)*s;
	    if (!(umaskop == '+' || umaskop == '-' || umaskop == '=')) {
		if (umaskop)
		    zwarnnam(nam, "bad symbolic mode operator: %c", umaskop);
		else
		    zwarnnam(nam, "bad umask");
		return 1;
	    }
	    /* Permissions mask -- r=read, w=write, x=execute. */
	    mask = 0;
	    while (*++s && *s != ',')
		if (*s == 'r')
		    mask |= 0444 & whomask;
		else if (*s == 'w')
		    mask |= 0222 & whomask;
		else if (*s == 'x')
		    mask |= 0111 & whomask;
		else {
		    zwarnnam(nam, "bad symbolic mode permission: %c", *s);
		    return 1;
		}
	    /* Apply parsed argument to um. */
	    if (umaskop == '+')
		um &= ~mask;
	    else if (umaskop == '-')
		um |= mask;
	    else		/* umaskop == '=' */
		um = (um | (whomask)) & ~mask;
	    if (*s == ',')
		s++;
	    else
		break;
	}
	if (*s) {
	    zwarnnam(nam, "bad character in symbolic mode: %c", *s);
	    return 1;
	}
    }

    /* Finally, set the new umask. */
    umask(um);
    return 0;
}

/* Generic builtin for facilities not available on this OS */

/**/
mod_export int
bin_notavail(char *nam, UNUSED(char **argv), UNUSED(Options ops), UNUSED(int func))
{
    zwarnnam(nam, "not available on this system");
    return 1;
}
