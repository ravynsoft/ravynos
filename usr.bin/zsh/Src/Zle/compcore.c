/*
 * compcore.c - the complete module, completion core code
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
#include "compcore.pro"

/* Flags saying what we have to do with the result. */

/**/
int useexact, useline, uselist, forcelist, startauto;

/**/
mod_export int iforcemenu;

/* Non-zero if we should go back to the last prompt. */

/**/
mod_export int dolastprompt;

/* Non-zero if we should keep an old list. */

/**/
mod_export int oldlist, oldins;

/* Original prefix/suffix lengths. Flag saying if they changed. */

/**/
int origlpre, origlsuf, lenchanged;

/* This is used to decide when the cursor should be moved to the end of    *
 * the inserted word: 0 - never, 1 - only when a single match is inserted, *
 * 2 - when a full match is inserted (single or menu), 3 - always.         */

/**/
int movetoend;

/* The match and group number to insert when starting menucompletion.   */

/**/
mod_export int insmnum, insspace;

#if 0
/* group-numbers in compstate[insert] */
int insgnum, insgroup; /* mod_export */
#endif

/* Information about menucompletion. */

/**/
mod_export struct menuinfo minfo;

/* Number of matches accepted with accept-and-menu-complete */

/**/
mod_export int menuacc;

/* Brace insertion stuff. */

/**/
int hasunqu, useqbr, brpcs, brscs;

/* Flags saying in what kind of string we are. */

/**/
mod_export int ispar, linwhat;

/* A parameter expansion prefix (like ${). */

/**/
char *parpre;

/* Flags for parameter expansions for new style completion. */

/**/
int parflags;

/* Match flags for all matches in this group. */

/**/
mod_export int mflags;

/* Flags saying how the parameter expression we are in is quoted. */

/**/
int parq, eparq;

/* We store the following prefixes/suffixes:                               *
 * ipre,ripre  -- the ignored prefix (quoted and unquoted)                 *
 * isuf        -- the ignored suffix                                       */

/**/
mod_export char *ipre, *ripre, *isuf;

/* The list of matches.  fmatches contains the matches we first ignore *
 * because of fignore.                                                 */

/**/
mod_export LinkList matches;
/**/
LinkList fmatches;

/* This holds the list of matches-groups. lastmatches holds the last list of 
 * permanently allocated matches, pmatches is the same for the list
 * currently built, amatches is the heap allocated stuff during completion
 * (after all matches have been generated it is an alias for pmatches), and
 * lmatches/lastlmatches is a pointer to the last element in the lists. */

/**/
mod_export Cmgroup lastmatches, pmatches, amatches, lmatches, lastlmatches;

/* Non-zero if we have permanently allocated matches (old and new). */

/**/
mod_export int hasoldlist, hasperm;

/* Non-zero if we have a match representing all other matches. */

/**/
int hasallmatch;

/* Non-zero if we have newly added matches. */

/**/
mod_export int newmatches;

/* Number of permanently allocated matches and groups. */

/**/
mod_export int permmnum, permgnum, lastpermmnum, lastpermgnum;

/* The total number of matches and the number of matches to be listed. */

/**/
mod_export int nmatches;
/**/
mod_export int smatches;

/* != 0 if more than one match and at least two different matches */

/**/
mod_export int diffmatches;

/* The number of messages. */

/**/
mod_export int nmessages;

/* != 0 if only explanation strings should be printed */

/**/
mod_export int onlyexpl;

/* Information about the matches for listing. */

/**/
mod_export struct cldata listdat;

/* This flag is non-zero if we are completing a pattern (with globcomplete) */

/**/
mod_export int ispattern, haspattern;

/* Non-zero if at least one match was added without/with -U. */

/**/
mod_export int hasmatched, hasunmatched;

/* The current group of matches. */

/**/
Cmgroup mgroup;

/* Match counter: all matches. */

/**/
mod_export int mnum;

/* The match counter when unambig_data() was called. */

/**/
mod_export int unambig_mnum;

/* Length of longest/shortest match. */

/**/
int maxmlen, minmlen;

/* This holds the explanation strings we have to print in this group and *
 * a pointer to the current cexpl structure. */

/**/
LinkList expls;

/**/
mod_export Cexpl curexpl;

/* A stack of completion matchers to be used. */

/**/
mod_export Cmlist mstack;

/* The completion matchers used when building new stuff for the line. */

/**/
mod_export Cmlist bmatchers;

/* A list with references to all matchers we used. */

/**/
mod_export LinkList matchers;

/* A heap of free Cline structures. */

/**/
mod_export Cline freecl;

/* Ambiguous information. */

/**/
mod_export Aminfo ainfo, fainfo;

/* The memory heap to use for new style completion generation. */

/**/
mod_export Heap compheap;

/* A list of some data.
 *
 * Well, actually, it's the list of all compctls used so far, but since
 * conceptually we don't know anything about compctls here... */

/**/
mod_export LinkList allccs;

/* This says what of the state the line is in when completion is started *
 * came from a previous completion. If the FC_LINE bit is set, the       *
 * string was inserted. If FC_INWORD is set, the last completion moved   *
 * the cursor into the word although it was at the end of it when the    *
 * last completion was invoked.                                          *
 * This is used to detect if the string should be taken as an exact      *
 * match (see do_ambiguous()) and if the cursor has to be moved to the   *
 * end of the word before generating the completions.                    */

/**/
int fromcomp;

/* This holds the end-position of the last string inserted into the line. */

/**/
mod_export int lastend;

#define inststr(X) inststrlen((X),1,-1)

/*
 * Main completion entry point, called from zle. 
 * At this point the line is already metafied.
 */

/**/
int
do_completion(UNUSED(Hookdef dummy), Compldat dat)
{
    int ret = 0, lst = dat->lst, incmd = dat->incmd, osl = showinglist;
    char *s = dat->s;
    char *opm;
    LinkNode n;

    METACHECK();

    pushheap();

    ainfo = fainfo = NULL;
    matchers = newlinklist();

    zsfree(compqstack);
    compqstack = zalloc(2);
    /*
     * It looks like we may need to do stuff with backslashes even
     * if instring is QT_NONE.
     */
    *compqstack = (instring == QT_NONE) ? QT_BACKSLASH : (char)instring;
    compqstack[1] = '\0';

    hasunqu = 0;
    useline = (wouldinstab ? -1 : (lst != COMP_LIST_COMPLETE));
    useexact = isset(RECEXACT);
    zsfree(compexactstr);
    compexactstr = ztrdup("");
    uselist = (useline ?
	       ((isset(AUTOLIST) && !isset(BASHAUTOLIST)) ? 
		(isset(LISTAMBIGUOUS) ? 3 : 2) : 0) : 1);
    zsfree(comppatmatch);
    opm = comppatmatch = ztrdup(useglob ? "*" : "");
    zsfree(comppatinsert);
    comppatinsert = ztrdup("menu");
    forcelist = 0;
    haspattern = 0;
    complistmax = getiparam("LISTMAX");
    zsfree(complastprompt);
    complastprompt = ztrdup(isset(ALWAYSLASTPROMPT) ? "yes" : "");
    dolastprompt = 1;
    zsfree(complist);
    complist = ztrdup(isset(LISTROWSFIRST) ?
		      (isset(LISTPACKED) ? "packed rows" : "rows") :
		      (isset(LISTPACKED) ? "packed" : ""));
    startauto = isset(AUTOMENU);
    movetoend = ((zlemetacs == we || isset(ALWAYSTOEND)) ? 2 : 1);
    showinglist = 0;
    hasmatched = hasunmatched = 0;
    minmlen = 1000000;
    maxmlen = -1;
    compignored = 0;
    nmessages = 0;
    hasallmatch = 0;

    /* Make sure we have the completion list and compctl. */
    if (makecomplist(s, incmd, lst)) {
	/* Error condition: feeeeeeeeeeeeep(). */
	zlemetacs = 0;
	foredel(zlemetall, CUT_RAW);
	inststr(origline);
	zlemetacs = origcs;
	clearlist = 1;
	ret = 1;
	minfo.cur = NULL;
	if (useline < 0) {
	    /* unmetafy line before calling ZLE */
	    unmetafy_line();
	    ret = selfinsert(zlenoargs);
	    metafy_line();
	}
	goto compend;
    }
    zsfree(lastprebr);
    zsfree(lastpostbr);
    lastprebr = lastpostbr = NULL;

    if (comppatmatch && *comppatmatch && comppatmatch != opm)
	haspattern = 1;
    if (iforcemenu) {
	if (nmatches)
            do_ambig_menu();
	ret = !nmatches;
    } else if (useline < 0) {
	/* unmetafy line before calling ZLE */
	unmetafy_line();
	ret = selfinsert(zlenoargs);
	metafy_line();
    } else if (!useline && uselist) {
	/* All this and the guy only wants to see the list, sigh. */
	zlemetacs = 0;
	foredel(zlemetall, CUT_RAW);
	inststr(origline);
	zlemetacs = origcs;
	showinglist = -2;
    } else if (useline == 2 && nmatches > 1) {
	do_allmatches(1);

	minfo.cur = NULL;

	if (forcelist)
	    showinglist = -2;
	else
	    invalidatelist();
    } else if (useline) {
	/* We have matches. */
	if (nmatches > 1 && diffmatches) {
	    /* There is more than one match. */
	    ret = do_ambiguous();

	    if (!showinglist && uselist && listshown && (usemenu == 2 || oldlist))
		showinglist = osl;
	} else if (nmatches == 1 || (nmatches > 1 && !diffmatches)) {
	    /* Only one match. */
	    Cmgroup m = amatches;
#ifdef ZSH_HEAP_DEBUG
	    if (memory_validate(m->heap_id)) {
		HEAP_ERROR(m->heap_id);
	    }
#endif

	    while (!m->mcount)
		m = m->next;
	    minfo.cur = NULL;
	    minfo.asked = 0;
	    do_single(m->matches[0]);
	    if (forcelist) {
		if (uselist)
		    showinglist = -2;
		else
		    clearlist = 1;
	    } else
		invalidatelist();
	} else if (nmessages && forcelist) {
	    if (uselist)
		showinglist = -2;
	    else
		clearlist = 1;
	}
    } else {
	invalidatelist();
	lastambig = isset(BASHAUTOLIST);
	if (forcelist)
	    clearlist = 1;
	zlemetacs = 0;
	foredel(zlemetall, CUT_RAW);
	inststr(origline);
	zlemetacs = origcs;
    }
    /* Print the explanation strings if needed. */
    if (!showinglist && validlist && usemenu != 2 && uselist &&
	(nmatches != 1 || diffmatches) &&
	useline >= 0 && useline != 2 && (!oldlist || !listshown)) {
	onlyexpl = 3;
	showinglist = -2;
    }
 compend:
    for (n = firstnode(matchers); n; incnode(n))
	freecmatcher((Cmatcher) getdata(n));

    zlemetall = strlen(zlemetaline);
    if (zlemetacs > zlemetall)
	zlemetacs = zlemetall;
    popheap();

    return ret;
}

/* Before and after hooks called by zle. */

static int oldmenucmp;

/**/
int
before_complete(UNUSED(Hookdef dummy), int *lst)
{
    oldmenucmp = menucmp;

    if (showagain && validlist)
	showinglist = -2;
    showagain = 0;

    /* If we are doing a menu-completion... */

    if (minfo.cur && menucmp && *lst != COMP_LIST_EXPAND) {
	do_menucmp(*lst);
	return 1;
    }
    if (minfo.cur && menucmp && validlist && *lst == COMP_LIST_COMPLETE) {
	showinglist = -2;
	onlyexpl = listdat.valid = 0;
	return 1;
    }

    /* We may have to reset the cursor to its position after the   *
     * string inserted by the last completion. */

    /*
     * Currently this hook runs before metafication.
     * This is the only hook of the three defined here of
     * which that is true.
     */
    if ((fromcomp & FC_INWORD) && (zlecs = lastend) > zlell)
	zlecs = zlell;

    /* Check if we have to start a menu-completion (via automenu). */

    if (startauto && lastambig &&
	(!isset(BASHAUTOLIST) || lastambig == 2))
	usemenu = 2;

    return 0;
}

/**/
int
after_complete(UNUSED(Hookdef dummy), int *dat)
{
    if (menucmp && !oldmenucmp) {
	struct chdata cdat;
	int ret;

	cdat.matches = amatches;
#ifdef ZSH_HEAP_DEBUG
	if (memory_validate(cdat.matches->heap_id)) {
	    HEAP_ERROR(cdat.matches->heap_id);
	}
#endif
	cdat.num = nmatches;
	cdat.nmesg = nmessages;
	cdat.cur = NULL;
	if ((ret = runhookdef(MENUSTARTHOOK, (void *) &cdat))) {
	    dat[1] = 0;
	    menucmp = menuacc = 0;
	    minfo.cur = NULL;
	    if (ret >= 2) {
		fixsuffix();
		zlemetacs = 0;
		foredel(zlemetall, CUT_RAW);
		inststr(origline);
		zlemetacs = origcs;
		if (ret == 2) {
		    clearlist = 1;
		    invalidatelist();
		}
	    }
	}
    }
    return 0;
}

/* This calls the given completion widget function. */

static int parwb, parwe, paroffs;

/**/
static void
callcompfunc(char *s, char *fn)
{
    Shfunc shfunc;
    int lv = lastval;
    char buf[20];

    METACHECK();

    if ((shfunc = getshfunc(fn))) {
	char **p, *tmp;
	int aadd = 0, usea = 1, icf = incompfunc, osc = sfcontext;
	unsigned int rset, kset;
	Param *ocrpms = comprpms, *ockpms = compkpms;

	comprpms = (Param *) zalloc(CP_REALPARAMS * sizeof(Param));
	compkpms = (Param *) zalloc(CP_KEYPARAMS * sizeof(Param));

	rset = CP_ALLREALS;
	kset = CP_ALLKEYS &
	    ~(CP_PARAMETER | CP_REDIRECT | CP_QUOTE | CP_QUOTING |
	      CP_EXACTSTR | CP_OLDLIST | CP_OLDINS |
	      (useglob ? 0 : CP_PATMATCH));
	zsfree(compvared);
	if (varedarg) {
	    compvared = ztrdup(varedarg);
	    kset |= CP_VARED;
	} else
	    compvared = ztrdup("");
	if (!*complastprompt)
	    kset &= ~CP_LASTPROMPT;
	zsfree(compcontext);
	zsfree(compparameter);
	zsfree(compredirect);
	compparameter = compredirect = "";
	if (ispar)
	    compcontext = (ispar == 2 ? "brace_parameter" : "parameter");
        else if (linwhat == IN_PAR)
            compcontext = "assign_parameter";
	else if (linwhat == IN_MATH) {
	    if (insubscr) {
		compcontext = "subscript";
		if (varname) {
		    compparameter = varname;
		    kset |= CP_PARAMETER;
		}
	    } else
		compcontext = "math";
	    usea = 0;
	} else if (lincmd) {
	    if (insubscr) {
		compcontext = "subscript";
		kset |= CP_PARAMETER;
	    } else
		compcontext = "command";
	} else if (linredir) {
	    compcontext = "redirect";
	    if (rdstr)
		compredirect = rdstr;
	    kset |= CP_REDIRECT;
	} else {
	    switch (linwhat) {
	    case IN_ENV:
		compcontext = (linarr ? "array_value" : "value");
		compparameter = varname;
		kset |= CP_PARAMETER;
		if (!clwpos) {
		    clwpos = 1;
		    clwnum = 2;
		    zsfree(clwords[1]);
		    clwords[1] = ztrdup(s);
		    zsfree(clwords[2]);
		    clwords[2] = NULL;
		}
		aadd = 1;
		break;
	    case IN_COND:
		compcontext = "condition";
		break;
	    default:
		if (cmdstr)
		    compcontext = "command";
		else {
		    compcontext = "value";
		    kset |= CP_PARAMETER;
		    if (clwords[0])
			compparameter = clwords[0];
		    aadd = 1;
		}
	    }
	}
	compcontext = ztrdup(compcontext);
	if (compwords)
	    freearray(compwords);
	if (usea && (!aadd || clwords[0])) {
	    char **q;

	    q = compwords = (char **)
		zalloc((clwnum + 1) * sizeof(char *));
	    for (p = clwords + aadd; *p; p++, q++)
		untokenize(*q = ztrdup(*p));
	    *q = NULL;
	} else
	    compwords = (char **) zshcalloc(sizeof(char *));

	if (compredirs)
	    freearray(compredirs);
        if (rdstrs)
            compredirs = zlinklist2array(rdstrs, 1);
        else
            compredirs = (char **) zshcalloc(sizeof(char *));

	/*
	 * We need to untokenize compparameter which is the
	 * raw internals of a parameter subscript.
	 *
	 * The double memory duplication is a bit ugly: the additional
	 * dupstring() is necessary because untokenize() might change
	 * the string length and so later zsfree() would get the wrong
	 * length of the string.
	 */
	compparameter = dupstring(compparameter);
	untokenize(compparameter);
	compparameter = ztrdup(compparameter);
	compredirect = ztrdup(compredirect);
	zsfree(compquote);
	zsfree(compquoting);
	if (instring > QT_BACKSLASH) {
	    switch (instring) {
	    case QT_SINGLE:
		compquote = ztrdup("\'");
		compquoting = ztrdup("single");
		break;

	    case QT_DOUBLE:
		compquote = ztrdup("\"");
		compquoting = ztrdup("double");
		break;

	    case QT_DOLLARS:
		compquote = ztrdup("$'");
		compquoting = ztrdup("dollars");
		break;
	    }
	    kset |= CP_QUOTE | CP_QUOTING;
	} else if (inbackt) {
	    compquote = ztrdup("`");
	    compquoting = ztrdup("backtick");
	    kset |= CP_QUOTE | CP_QUOTING;
	} else {
	    compquote = ztrdup("");
	    compquoting = ztrdup("");
	}
	zsfree(compprefix);
	zsfree(compsuffix);
	makebangspecial(0);
	if (unset(COMPLETEINWORD)) {
	    tmp = (linwhat == IN_MATH ? dupstring(s) : multiquote(s, 0));
	    untokenize(tmp);
	    compprefix = ztrdup(tmp);
	    compsuffix = ztrdup("");
	} else {
	    char *ss, sav;
	    
	    ss = s + offs;

	    sav = *ss;
	    *ss = '\0';
	    tmp = (linwhat == IN_MATH ? dupstring(s) : multiquote(s, 0));
	    untokenize(tmp);
	    compprefix = ztrdup(tmp);
	    *ss = sav;
	    ss = (linwhat == IN_MATH ? dupstring(ss) : multiquote(ss, 0));
	    untokenize(ss);
	    compsuffix = ztrdup(ss);
	}
	makebangspecial(1);
        zsfree(complastprefix);
        zsfree(complastsuffix);
        complastprefix = ztrdup(compprefix);
        complastsuffix = ztrdup(compsuffix);
	zsfree(compiprefix);
	zsfree(compisuffix);
	if (parwb < 0) {
	    compiprefix = ztrdup("");
	    compisuffix = ztrdup("");
	} else {
	    int l;

	    compiprefix = (char *) zalloc((l = wb - parwb) + 1);
	    memcpy(compiprefix, zlemetaline + parwb, l);
	    compiprefix[l] = '\0';
	    compisuffix = (char *) zalloc((l = parwe - we) + 1);
	    memcpy(compisuffix, zlemetaline + we, l);
	    compisuffix[l] = '\0';

	    wb = parwb;
	    we = parwe;
	    offs = paroffs;
	}
	zsfree(compqiprefix);
	compqiprefix = ztrdup(qipre ? qipre : "");
	zsfree(compqisuffix);
	compqisuffix = ztrdup(qisuf ? qisuf : "");
	origlpre = (strlen(compqiprefix) + strlen(compiprefix) +
		    strlen(compprefix));
	origlsuf = (strlen(compqisuffix) + strlen(compisuffix) +
		    strlen(compsuffix));
	lenchanged = 0;
	compcurrent = (usea ? (clwpos + 1 - aadd) : 0);

	zsfree(complist);
	switch (uselist) {
	case 0: complist = ""; kset &= ~CP_LIST; break;
	case 1: complist = "list"; break;
	case 2: complist = "autolist"; break;
	case 3: complist = "ambiguous"; break;
	}
	if (isset(LISTPACKED))
	    complist = dyncat(complist, " packed");
	if (isset(LISTROWSFIRST))
	    complist = dyncat(complist, " rows");

	complist = ztrdup(complist);
	zsfree(compinsert);
	if (useline) {
	    switch (usemenu) {
	    case 0:
		compinsert = (isset(AUTOMENU) ?
			      "automenu-unambiguous" :
			      "unambiguous");
		break;
	    case 1: compinsert = "menu"; break;
	    case 2: compinsert = "automenu"; break;
	    }
	} else {
	    compinsert = "";
	    kset &= ~CP_INSERT;
	}
	compinsert = (useline < 0 ? tricat("tab ", "", compinsert) :
		      ztrdup(compinsert));
	zsfree(compexact);
	if (useexact)
	    compexact = ztrdup("accept");
	else {
	    compexact = ztrdup("");
	    kset &= ~CP_EXACT;
	}
	zsfree(comptoend);
	if (movetoend == 1)
	    comptoend = ztrdup("single");
	else
	    comptoend = ztrdup("match");
	zsfree(compoldlist);
	zsfree(compoldins);
	if (hasoldlist && lastpermmnum) {
	    if (listshown)
		compoldlist = "shown";
	    else
		compoldlist = "yes";
	    kset |= CP_OLDLIST;
	    if (minfo.cur) {
		sprintf(buf, "%d", (*(minfo.cur))->gnum);
		compoldins = buf;
		kset |= CP_OLDINS;
	    } else
		compoldins = "";
	} else
	    compoldlist = compoldins = "";
	compoldlist = ztrdup(compoldlist);
	compoldins = ztrdup(compoldins);

	incompfunc = 1;
	startparamscope();
	makecompparams();
	comp_setunset(rset, (~rset & CP_ALLREALS),
		      kset, (~kset & CP_ALLKEYS));
	makezleparams(1);
	sfcontext = SFC_CWIDGET;
	NEWHEAPS(compheap) {
	    LinkList largs = NULL;
	    int oxt = isset(XTRACE);

	    if (*cfargs) {
		char **p = cfargs;

		largs = newlinklist();
		addlinknode(largs, dupstring(fn));
		while (*p)
		    addlinknode(largs, dupstring(*p++));
	    }
	    opts[XTRACE] = 0;
	    cfret = doshfunc(shfunc, largs, 1);
	    opts[XTRACE] = oxt;
	} OLDHEAPS;
	sfcontext = osc;
	endparamscope();
	lastcmd = 0;
	incompfunc = icf;
	startauto = 0;

	if (!complist)
	    uselist = 0;
	else if (!strncmp(complist, "list", 4))
	    uselist = 1;
	else if (!strncmp(complist, "auto", 4))
	    uselist = 2;
	else if (!strncmp(complist, "ambig", 5))
	    uselist = 3;
	else
	    uselist = 0;
	forcelist = (complist && strstr(complist, "force"));
	onlyexpl = (complist ? ((strstr(complist, "expl") ? 1 : 0) |
				(strstr(complist, "messages") ? 2 : 0)) : 0);

	if (!compinsert)
	    useline = 0;
	else if (strstr(compinsert, "tab"))
	    useline = -1;
	else if (!strcmp(compinsert, "unambig") ||
		 !strcmp(compinsert, "unambiguous") ||
		 !strcmp(compinsert, "automenu-unambiguous"))
	    useline = 1, usemenu = 0;
	else if (!strcmp(compinsert, "all"))
	    useline = 2, usemenu = 0;
	else if (idigit(*compinsert)) {
#if 0
	    /* group-numbers in compstate[insert] */
	    char *m;
#endif
	    useline = 1; usemenu = 3;
	    insmnum = atoi(compinsert);
#if 0
	    /* group-numbers in compstate[insert] */
	    if ((m = strchr(compinsert, ':'))) {
		insgroup = 1;
		insgnum = atoi(m + 1);
	    }
#endif
	    insspace = (compinsert[strlen(compinsert) - 1] == ' ');
	} else {
	    char *p;

	    if (strpfx("menu", compinsert))
		useline = 1, usemenu = 1;
	    else if (strpfx("auto", compinsert))
		useline = 1, usemenu = 2;
	    else {
		useline = usemenu = 0;
		/* if compstate[insert] was emptied, no unambiguous prefix
		 * ever gets inserted so allow the next tab to already start
		 * menu completion */
		startauto = lastambig = isset(AUTOMENU);
	    }

	    if (useline && (p = strchr(compinsert, ':'))) {
		insmnum = atoi(++p);
#if 0
		/* group-numbers in compstate[insert] */
		if ((p = strchr(p, ':'))) {
		    insgroup = 1;
		    insgnum = atoi(p + 1);
		}
#endif
	    }
	}
	startauto = startauto || ((compinsert &&
		      !strcmp(compinsert, "automenu-unambiguous")) ||
		     (bashlistfirst && isset(AUTOMENU) &&
                      (!compinsert || !*compinsert)));
	useexact = (compexact && !strcmp(compexact, "accept"));

	if (!comptoend || !*comptoend)
	    movetoend = 0;
	else if (!strcmp(comptoend, "single"))
	    movetoend = 1;
	else if (!strcmp(comptoend, "always"))
	    movetoend = 3;
	else
	    movetoend = 2;

	oldlist = (hasoldlist && compoldlist && !strcmp(compoldlist, "keep"));
	oldins = (hasoldlist && minfo.cur &&
		  compoldins && !strcmp(compoldins, "keep"));

	zfree(comprpms, CP_REALPARAMS * sizeof(Param));
	zfree(compkpms, CP_KEYPARAMS * sizeof(Param));
	comprpms = ocrpms;
	compkpms = ockpms;
    }
    lastval = lv;
}

/* Create the completion list.  This is called whenever some bit of   *
 * completion code needs the list.                                    *
 * Along with the list is maintained the prefixes/suffixes etc.  When *
 * any of this becomes invalid -- e.g. if some text is changed on the *
 * command line -- invalidatelist() should be called, to set          *
 * validlist to zero and free up the memory used.  This function      *
 * returns non-zero on error.                                         */

/**/
static int
makecomplist(char *s, int incmd, int lst)
{
    char *p;
    int owb = wb, owe = we, ooffs = offs;

    /* Inside $... ? */
    if (compfunc && (p = check_param(s, 0, 0))) {
	s = p;
	parwb = owb;
	parwe = owe;
	paroffs = ooffs;
    } else
	parwb = -1;

    linwhat = inwhat;

    if (compfunc) {
	char *os = s;
	int onm = nmatches, odm = diffmatches, osi = movefd(0);

	bmatchers = NULL;
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

	s = dupstring(os);
	callcompfunc(s, compfunc);
	endcmgroup(NULL);

	/* Needed for compcall. */
	runhookdef(COMPCTLCLEANUPHOOK, NULL);

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

	if ((nmatches || nmessages) && !errflag) {
	    validlist = 1;

	    redup(osi, 0);

	    return 0;
	}
	redup(osi, 0);
	return 1;
    } else {
	struct ccmakedat dat;

	dat.str = s;
	dat.incmd = incmd;
	dat.lst = lst;
	runhookdef(COMPCTLMAKEHOOK, (void *) &dat);

	/* Needed for compcall. */
	runhookdef(COMPCTLCLEANUPHOOK, NULL);

	return dat.lst;
    }
}

/*
 * Quote 's' according to compqstack, aka $compstate[all_quotes].
 *
 * If 'ign' is 1, skip the innermost quoting level.  Otherwise 'ign'
 * must be 0.
 */

/**/
mod_export char *
multiquote(char *s, int ign)
{
    if (s) {
	char *os = s, *p = compqstack;

	if (p && *p && (ign == 0 || p[1])) {
	    if (ign)
		p++;
	    while (*p) {
		s = quotestring(s, *p);
		p++;
	    }
	}
	return (s == os ? dupstring(s) : s);
    }
    DPUTS(1, "BUG: null pointer in multiquote()");
    return NULL;
}

/*
 * tildequote(s, ign): Equivalent to multiquote(s, ign), except that if
 * compqstack[0] == QT_BACKSLASH and s[0] == '~', then that tilde is not
 * quoted.
 */

/**/
mod_export char *
tildequote(char *s, int ign)
{
    if (s) {
	int tilde;

	if ((tilde = (*s == '~')))
	    *s = 'x';
	s = multiquote(s, ign);
	if (tilde)
	    *s = '~';

	return s;
    }
    DPUTS(1, "BUG: null pointer in tildequote()");
    return NULL;
}

/* Check if we have to complete a parameter name. */

/**/
mod_export char *
check_param(char *s, int set, int test)
{
    char *p;
    int found = 0, qstring = 0;

    zsfree(parpre);
    parpre = NULL;

    if (!test)
	ispar = parq = eparq = 0;
    /*
     * Try to find a `$'.
     *
     * TODO: passing s as a parameter while we get some mysterious
     * offset "offs" into it via a global sucks badly.
     *
     * From ../lex.c we know:
     * wb is the beginning position of the current word in the line
     * we is the position of the end of the current word in the line
     * From zle_tricky.c we know:
     * offs is position within the word where we are completing
     *
     * So wb + offs is the current cursor position if COMPLETE_IN_WORD
     * is set, otherwise it is the end of the word (same as we).
     * 
     * Note below we are thus stepping backward from our completion
     * position to find a '$' in the current word (if any).
     */ 
    for (p = s + offs; ; p--) {
	if (*p == String || *p == Qstring) {
	    /*
	     * String followed by Snull (unquoted) or
	     * QString followed by ' (quoted) indicate a nested
	     * $'...', not a substitution.
	     *
	     * TODO: the argument passing is obscure, no idea if
	     * it's safe to test for the "'" at the end.
	     */
	    if (p < s + offs &&
		!(*p == String && p[1] == Snull) &&
		!(*p == Qstring && p[1] == '\'')) {
		found = 1;
		qstring = (*p == Qstring);
		break;
	    }
	}
	if (p == s)
	    break;
    }
    if (found) {
	/*
	 * Handle $$'s
	 *
	 * TODO: this is already bad enough, so I haven't tried
	 * testing for $'...' here.  If we parsed this forwards
	 * it wouldn't be quite so bad.
	 */
	while (p > s && (p[-1] == String || p[-1] == Qstring))
	    p--;
	while ((p[1] == String || p[1] == Qstring) &&
	       (p[2] == String || p[2] == Qstring))
	    p += 2;
    }
    if (found &&
	p[1] != Inpar && p[1] != Inbrack && p[1] != Snull) {
	/* This is a parameter expression, not $(...), $[...], $'...'. */
	char *b = p + 1, *e = b, *ie;
	int br = 1, nest = 0;

	if (*b == Inbrace) {
	    char *tb = b;

	    /* If this is a ${...}, see if we are before the '}'. */
	    if (!skipparens(Inbrace, Outbrace, &tb) && tb - s <= offs)
		return NULL;

	    /* Ignore the possible (...) flags. */
	    b++, br++;
	    if ((qstring ? skipparens('(', ')', &b) :
		 skipparens(Inpar, Outpar, &b)) > 0 || b - s > offs) {
		/*
		 * We are still within the parameter flags.  There's no
		 * point trying to do anything clever here with
		 * parameter names.  Instead, just report that we are in
		 * a brace parameter but let the completion function
		 * decide what to do about it.
		 */
		ispar = 2;
		return NULL;
	    }

	    for (tb = p - 1; tb > s && *tb != Outbrace && *tb != Inbrace; tb--);
	    if (tb > s && *tb == Inbrace && (tb[-1] == String || *tb == Qstring))
		nest = 1;
	}

	/* Ignore the stuff before the parameter name. */
	for (; *b; b++)
	    if (*b != '^' && *b != Hat &&
		*b != '=' && *b != Equals &&
		*b != '~' && *b != Tilde)
		break;
	if (*b == '#' || *b == Pound || *b == '+')
	    b++;

	e = b;
	if (br) {
	    while (*e == (test ? Dnull : '"'))
		e++, parq++;
	    if (!test)
		b = e;
	}
	/* Find the end of the name. */
	if (*e == Quest || *e == Star || *e == String || *e == Qstring ||
	    *e == '?'   || *e == '*'  || *e == '$'    ||
	    *e == '-'   || *e == '!'  || *e == '@')
	    e++;
	else if (idigit(*e))
	    while (idigit(*e))
		e++;
	else if ((ie = itype_end(e, IIDENT, 0)) != e) {
	    do {
		e = ie;
		if (comppatmatch && *comppatmatch &&
		    (*e == Star || *e == Quest))
		    ie = e + 1;
		else
		    ie = itype_end(e, IIDENT, 0);
	    } while (ie != e);
	}

	/* Now make sure that the cursor is inside the name. */
	if (offs <= e - s && offs >= b - s) {
	    char sav;

	    if (br) {
		p = e;
		while (*p == (test ? Dnull : '"'))
		    p++, parq--, eparq++;
	    }
	    /* It is. */
	    if (test)
		return b;
	    /* If we were called from makecomplistflags(), we have to set the
	     * global variables. */

	    if (set) {
		if (br >= 2) {
		    mflags |= CMF_PARBR;
		    if (nest)
			mflags |= CMF_PARNEST;
		}
		/* Get the prefix (anything up to the character before the name). */
		isuf = dupstring(e);
		untokenize(isuf);
		sav = *b;
		*b = *e = '\0';
		ripre = dyncat((ripre ? ripre : ""), s);
		ipre = dyncat((ipre ? ipre : ""), s);
		*b = sav;

		untokenize(ipre);
	    }
	    /* Save the prefix. */
	    if (compfunc) {
		parflags = (br >= 2 ? CMF_PARBR | (nest ? CMF_PARNEST : 0) : 0);
		sav = *b;
		*b = '\0';
		untokenize(parpre = ztrdup(s));
		*b = sav;
	    }
	    /* And adjust wb, we, and offs again. */
	    offs -= b - s;
	    wb = zlemetacs - offs;
	    we = wb + e - b;
	    ispar = (br >= 2 ? 2 : 1);
	    b[we-wb] = '\0';
	    return b;
	} else if (offs > e - s && *e == ':') {
	    /*
	     * Guess whether we are in modifiers.
	     * If the name is followed by a : and the stuff after
	     * that is either colons or alphanumerics we probably are.
	     * This is a very rough guess.
	     */
	    char *offsptr = s + offs;
	    for (; e < offsptr; e++) {
		if (*e != ':' && !ialnum(*e))
		    break;
	    }
	    ispar = (br >= 2 ? 2 : 1);
	    return NULL;
	}
    }
    return NULL;
}

/* Copy the given string and remove backslashes from the copy and return it. */

/**/
mod_export char *
rembslash(char *s)
{
    char *t = s = dupstring(s);

    while (*s)
	if (*s == '\\') {
	    chuck(s);
	    if (*s)
		s++;
	} else
	    s++;

    return t;
}

/* Remove one of every pair of single quotes, without copying. Return
 * the number of removed quotes. */

/**/
mod_export int
remsquote(char *s)
{
    int ret = 0, qa = (isset(RCQUOTES) ? 1 : 3);
    char *t = s;

    while (*s)
	if (qa == 1 ?
            (s[0] == '\'' && s[1] == '\'') :
            (s[0] == '\'' && s[1] == '\\' && s[2] == '\'' && s[3] == '\'')) {
            ret += qa;
            *t++ = '\'';
            s += qa + 1;
	} else
	    *t++ = *s++;
    *t = '\0';

    return ret;
}

/* This should probably be moved into tokenize(). */

/**/
mod_export char *
ctokenize(char *p)
{
    char *r = p;
    int bslash = 0;

    tokenize(p);

    for (p = r; *p; p++) {
	if (*p == '\\')
	    bslash = 1;
	else {
	    if (*p == '$' || *p == '{' || *p == '}') {
		if (bslash)
		    p[-1] = Bnull;
		else
		    *p = (*p == '$' ? String :
			  (*p == '{' ? Inbrace : Outbrace));
	    }
	    bslash = 0;
	}
    }
    return r;
}


/*
 * This function reconstructs the full completion argument in
 * heap memory by concatenating and, if untok is non-zero, untokenizing
 * the ignored prefix and the active prefix and suffix.
 * (It appears from the function that the ignored prefix won't
 * be tokenized but I haven't checked this.)
 * ipl and/or pl may be passed and if so will be set to the ignored
 * prefix length and active prefix length respectively.
 */

/**/
mod_export char *
comp_str(int *ipl, int *pl, int untok)
{
    char *p = dupstring(compprefix);
    char *s = dupstring(compsuffix);
    char *ip = dupstring(compiprefix);
    char *str;
    int lp, ls, lip;

    if (!untok) {
	ctokenize(p);
	remnulargs(p);
	ctokenize(s);
	remnulargs(s);
    }
    lp = strlen(p);
    ls = strlen(s);
    lip = strlen(ip);
    str = zhalloc(lip + lp + ls + 1);
    strcpy(str, ip);
    strcat(str, p);
    strcat(str, s);

    if (ipl)
	*ipl = lip;
    if (pl)
	*pl = lp;

    return str;
}

/**/
mod_export char *
comp_quoting_string(int stype)
{
    switch (stype)
    {
    case QT_SINGLE:
	return "'";
    case QT_DOUBLE:
	return "\"";
    case QT_DOLLARS:
	return "$'";
    default:			/* shuts up compiler */
	return "\\";
    }
}

/*
 * This is the code behind compset -q, which splits the
 * the current word as if it were a command line.
 *
 * This is one of those completion functions that merits the
 * coveted title "not just ordinarily horrific".
 */

/**/
int
set_comp_sep(void)
{
    /*
     * s: full (reconstructed) completion argument
     * lip: ignored prefix length
     * lp: active prefix length
     * 1: the number "one" => untokenize
     */
    int lip, lp;
    char *s = comp_str(&lip, &lp, 1);
    LinkList foo = newlinklist();
    LinkNode n;
    /* Save word position */
    int owe = we, owb = wb;
    /*
     * Values of word beginning and end and cursor after subtractions
     * due to separators.   I think these are indexes into zlemetaline,
     * but with some subtractions; they don't see to be indexes into
     * s, which is the current argument before quote stripping.
     */
    int swb, swe, scs;
    /* Offset into current word after subtractions. */
    int soffs;
    /* Current state of error suppression. */
    int ne = noerrs;
    /* Length of tmp string */
    int tl;
    /* flag that we've got the current completion word, perhaps? */
    int got = 0;
    /*
     * i starts off as the number of the completion word we're looking at,
     * which is why it's initialised, but is then recycled as a
     * loop variable.  j is always a loop variable.
     */
    int i = 0, j;
    /*
     * cur: completion word currently being completed (0 offset).
     * sl: length of string s, the string we're manipulating.
     * css: modification of offset into current word beyond cursor
     * position due to the effects of backslashing, counted during our first
     * examination of compqstack for double quotes and dollar quotes.
     * However, for some reason, when the current quoting scheme is
     * backslashing we modify swb directly later rather than counting it at
     * the point we remove the backquotes.
     */
    int cur = -1, sl, css = 0;
    /*
     * Flag that we're doing the thing with backslashes mentioned
     * for css.
     */
    int remq = 0;
    /*
     * dq: backslash-removals for double quotes
     * odq: value of dq before modification for active (Bnull'ed)
     *      backslashes, or something.
     * sq: quote-removals for single quotes; either RCQUOTES or '\'' which
     *     are specially handled (but currently only if RCQUOTES is not
     *     set, which isn't necessarily correct if the quotes were typed by
     *     the user).
     * osq: c.f. odq, taking account of Snull's and embedded "'"'s.
     * qttype: type of quotes using standard QT_* definitions.
     * lsq: when quoting is single quotes (QT_SINGLE), counts the offset
     *      adjustment needed in the word being examined in the lexer loop.
     * sqq: the value of lsq for the current completion word.
     * qa:  not, unfortunately, a question and answer session with the
     *      original author, but the number of characters being removed
     *      when stripping single quotes: 1 for RCQUOTES, 3 otherwise
     *      (because we leave a "'" in the final string).
     */
    int dq = 0, odq, sq = 0, qttype, sqq = 0, lsq = 0, qa = 0;
    /* dolq: like sq and dq but for dollars quoting. */
    int dolq = 0;
    /* remember some global variable values (except lp is local) */
    int ois = instring, oib = inbackt, noffs = lp, ona = noaliases;
    /*
     * tmp: used for temporary processing of strings
     * p: loop pointer for tmp etc.
     * ns: holds yet another version of the current completion string,
     *     goodness knows how it differs from s, tmp, ts, ...
     * ts: untokenized ns
     * ol: saves old metafied editing line
     * sav: save character when NULLed; careful, there's a nested
     *      definition of sav just to keep you on your toes
     * qp, qs: prefix and suffix strings deduced from s.
     */
    char *tmp, *p, *ns, *ts, *ol, sav, *qp, *qs;

    METACHECK();

    s += lip;
    wb += lip;
    untokenize(s);

    swb = swe = soffs = 0;
    ns = NULL;

    /* Put the string in the lexer buffer and call the lexer to *
     * get the words we have to expand.                        */
    zle_save_positions();
    ol = zlemetaline;
    addedx = 1;
    noerrs = 1;
    zcontext_save();
    lexflags = LEXFLAGS_ZLE;
    /*
     * tl is the length of the temporary string including
     * the space at the start and the x at the cursor position,
     * but not the NULL byte.
     */
    tl = strlen(s) + 2;
    tmp = (char *) zhalloc(tl + 1);
    tmp[0] = ' ';
    memcpy(tmp + 1, s, noffs);
    tmp[(scs = zlemetacs = 1 + noffs)] = 'x';
    strcpy(tmp + 2 + noffs, s + noffs);

    switch ((qttype = *compqstack)) {
    case QT_BACKSLASH:
        remq = 1;
	tmp = rembslash(tmp);
        break;

    case QT_SINGLE:
        if (isset(RCQUOTES))
            qa = 1;
        else
            qa = 3;
        sq = remsquote(tmp);
        break;

    case QT_DOUBLE:
        for (j = 0, p = tmp; *p; p++, j++)
	    /*
	     * I added the handling for " here: before it just handled
	     * backslashes.  This meant that a \" inside a " wasn't
	     * handled properly.  I presume that was an oversight.
	     * I don't know if this is the right place to fix this
	     * particular problem because I'm utterly confused by
	     * the structure of the code in this function.
	     */
            if (*p == '\\' && (p[1] == '\\' || p[1] == '"')) {
		dq++;
                chuck(p);
		if (*p == '"')
		    zlemetacs--;
		else if (j > zlemetacs) {
                    zlemetacs++;
                    css++;
                }
                if (!*p)
                    break;
            }
	break;

    case QT_DOLLARS:
	j = zlemetacs;
	tmp = getkeystring(tmp, &sl,
			   GETKEY_DOLLAR_QUOTE|GETKEY_UPDATE_OFFSET,
			   &zlemetacs);
	/* The number of bytes we removed because of $' quoting */
	dolq = tl - sl;
	/* Offset into the word is modified, too... */
	css += zlemetacs - j;
	break;

    case QT_NONE:
    default: /* to silence compiler warnings */
#ifdef DEBUG
	dputs("BUG: head of compqstack is NULL");
#endif
	break;

    }
    odq = dq;
    inpush(dupstrspace(tmp), 0, NULL);
    zlemetaline = tmp;
    /*
     * tl is the length of temporary string, calculated above.
     * It seems zlemetall need not include the 'x' added at the cursor.
     * addedx is taken care of in function gotword() (lex.c).
     */
    zlemetall = tl - addedx;
    strinbeg(0);
    noaliases = 1;
    do {
	ctxtlex();
	if (tok == LEXERR) {
	    int j;

	    if (!tokstr)
		break;
	    /*
	     * If there was an error, it may be because we're in
	     * an unterminated string.  Count the active quote
	     * characters to see.  We need an odd number.
	     * This works for $', too, since the ' there is an Snull.
	     */
	    for (j = 0, p = tokstr; *p; p++) {
		if (*p == Snull || *p == Dnull)
		    j++;
            }
	    if (j & 1) {
		tok = STRING;
		if (p > tokstr && p[-1] == ' ')
		    p[-1] = '\0';
	    }
	}
	if (tok == ENDINPUT)
	    break;
	if (tokstr && *tokstr) {
            for (p = tokstr; dq && *p; p++) {
                if (*p == Bnull) {
                    dq--;
                    if (p[1] == '\\')
                        dq--;
                }
            }
            if (qttype == QT_SINGLE) {
                for (p = tokstr, lsq = 0; *p; p++) {
                    if (sq && *p == Snull)
                        sq -= qa;
                    if (*p == '\'') {
                        sq -= qa;
                        lsq += qa;
                    }
                }
            }
            else
                lsq = 0;
	    addlinknode(foo, (p = ztrdup(tokstr)));
        }
	else
	    p = NULL;
	if (!got && !lexflags) {
	    DPUTS(!p, "no current word in substr");
	    got = 1;
	    cur = countlinknodes(foo) - 1;  /* cur is 0 offset */
	    swb = wb - dq - sq - dolq;
	    swe = we - dq - sq - dolq;
            sqq = lsq;
	    soffs = zlemetacs - swb - css;
	    DPUTS2(p[soffs] != 'x', "expecting 'x' at offset %d of \"%s\"",
		   soffs, p);
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
    zlemetaline = ol;
    zle_restore_positions();
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
	    swb++;
	    break;
	}

	inbackt = 0;
	swb++;
	if (nsptr[strlen(nsptr) - 1] == *nsptr && nsptr[1])
	    swe--;
	zsfree(autoq);
	sav = *++tsptr;
	*tsptr = '\0';
	autoq = ztrdup(compqstack[1] ? "" : multiquote(ts, 1));
	*(ts = tsptr) = sav;
    } else {
	instring = QT_NONE;
	zsfree(autoq);
	autoq = NULL;
    }

    /*
     * In the following loop we look for parse quotes yet again.
     * I don't really have the faintest idea why, but given that
     * ns is immediately reassigned from ts afterwards (why? what's
     * wrong with it being in ts?) and scs isn't used again, I
     * presume it's in aid of getting the indexes for word beginning
     * (swb) and start offset (soffs) into s correct.
     *
     * I think soffs is an index into s, while swb and scs are indexes
     * into the full line but with some jiggery pokery for quote removal.
     */
    for (p = ns, i = swb; *p; p++, i++) {
	if (inull(*p)) {
	    if (i < scs) {
		if (*p == Bnull) {
                    if (p[1] && remq)
                        swb -= 2;
                    if (odq) {
                        swb--;
                        if (p[1] == '\\')
                            swb--;
                    }
                }
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

    if (instring && strchr(compqstack, QT_BACKSLASH)) {
	int rl = strlen(ns), ql = strlen(multiquote(ns, !!compqstack[1]));

	if (ql > rl)
	    swb -= ql - rl;
    }
    /*
     * Using the word beginning and end as an index into the reconstructed
     * string s, swb and swe, we can get the strings before and after
     * the word we're considering.
     *
     * Because it would be too easy otherwise, there are random
     * additional subtractions to be made.  The 1 might be something
     * to do with the space that appeared mysteriously at the start of the
     * line when we passed it through the lexer.  The sqq is to do with
     * the single quote quoting when we passed it through the lexer.
     *
     * TODO: I added the "+ dq" because it seemed to improve matters for
     * double quoting but the fact it's arrived at in a rather different way
     * from sqq may indicate this is wrong.  $'...' may need something, too.
     */
    sav = s[(i = swb - 1 - sqq + dq)];
    s[i] = '\0';
    qp = (qttype == QT_SINGLE) ? dupstring(s) : rembslash(s);
    s[i] = sav;
    if (swe < swb)
	swe = swb;
    swe--;
    sl = strlen(s);
    if (swe > sl) {
	swe = sl;
	if ((int)strlen(ns) > swe - swb + 1)
	    ns[swe - swb + 1] = '\0';
    }
    qs = (qttype == QT_SINGLE) ? dupstring(s + swe) : rembslash(s + swe);
    sl = strlen(ns);
    if (soffs > sl)
	soffs = sl;
    if (qttype == QT_SINGLE) {
        remsquote(qp);
        remsquote(qs);
    }
    {
	int set = CP_QUOTE | CP_QUOTING, unset = 0;

	tl = strlen(compqstack);
	p = zalloc(tl + 2);
	*p = (char)(instring == QT_NONE ? QT_BACKSLASH : instring);
	memcpy(p+1, compqstack, tl);
	p[tl+1] = '\0';
	zsfree(compqstack);
	compqstack = p;

	zsfree(compquote);
	zsfree(compquoting);
	switch (instring) {
	case QT_DOUBLE:
	    compquote = "\"";
	    compquoting = "double";
	    break;

	case QT_SINGLE:
	    compquote = "'";
	    compquoting = "single";
	    break;

	case QT_DOLLARS:
	    compquote = "$'";
	    compquoting = "dollars";
	    break;

	default:
	    compquote = compquoting = "";
	    unset = set;
	    set = 0;
	    break;
	}
	compquote = ztrdup(compquote);
	compquoting = ztrdup(compquoting);
	comp_setunset(0, 0, set, unset);

	zsfree(compprefix);
	zsfree(compsuffix);
	if (unset(COMPLETEINWORD)) {
	    untokenize(ns);
	    compprefix = ztrdup(ns);
	    compsuffix = ztrdup("");
	} else {
	    char *ss, sav;
	    
	    ss = ns + soffs;

	    sav = *ss;
	    *ss = '\0';
	    untokenize(ns);
	    compprefix = ztrdup(ns);
	    *ss = sav;
	    untokenize(ss);
	    compsuffix = ztrdup(ss);
	}
        if ((i = strlen(compprefix)) > 1 && compprefix[i - 1] == '\\' &&
	    compprefix[i - 2] != '\\' && compprefix[i - 2] != Meta)
            compprefix[i - 1] = '\0';
        
	tmp = tricat(compqiprefix, compiprefix, multiquote(qp, 1));
	zsfree(compqiprefix);
	compqiprefix = tmp;
	tmp = tricat(multiquote(qs, 1), compisuffix, compqisuffix);
	zsfree(compqisuffix);
	compqisuffix = tmp;
	zsfree(compiprefix);
	compiprefix = ztrdup("");
	zsfree(compisuffix);
	compisuffix = ztrdup("");
	freearray(compwords);
	i = countlinknodes(foo);
	compwords = (char **) zalloc((i + 1) * sizeof(char *));
	for (n = firstnode(foo), i = 0; n; incnode(n), i++) {
	    p = compwords[i] = (char *) getdata(n);
	    untokenize(p);
	}
	/* The current position shouldn't exceed the new word count */
	if ((compcurrent = cur + 1) > i) {
	    DPUTS2(1, "compcurrent=%d > number_of_words=%d", compcurrent, i);
	    compcurrent = i;
	}
	compwords[i] = NULL;
    }
    instring = ois;
    inbackt = oib;

    return 0;
}

/* This stores the strings from the list in an array. */

/**/
mod_export void
set_list_array(char *name, LinkList l)
{
    setaparam(name, zlinklist2array(l, 1));
}

/* Get the words from a variable or a (list of words). */

/**/
mod_export char **
get_user_var(char *nam)
{
    if (!nam)
	return NULL;
    else if (*nam == '(') {
	/* It's a (...) list, not a parameter name. */
	char *ptr, *s, **uarr, **aptr;
	int count = 0, notempty = 0, brk = 0;
	LinkList arrlist = newlinklist();

	ptr = dupstring(nam);
	s = ptr + 1;
	while (*++ptr) {
	    if (*ptr == '\\' && ptr[1])
		chuck(ptr), notempty = 1;
	    else if (*ptr == ',' || inblank(*ptr) || *ptr == ')') {
		if (*ptr == ')')
		    brk++;
		if (notempty) {
		    *ptr = '\0';
		    count++;
		    if (*s == '\n')
			s++;
		    addlinknode(arrlist, s);
		}
		s = ptr + 1;
		notempty = 0;
	    } else {
		notempty = 1;
		if (*ptr == Meta)
		    ptr++;
	    }
	    if (brk)
		break;
	}
	if (!brk || !count)
	    return NULL;
	*ptr = '\0';
	aptr = uarr = (char **) zhalloc(sizeof(char *) * (count + 1));

	while ((*aptr++ = (char *)ugetnode(arrlist)));
	uarr[count] = NULL;
	return uarr;
    } else {
	/* Otherwise it should be a parameter name. */
	char **arr = NULL, *val;

	queue_signals();
	if ((arr = getaparam(nam)) || (arr = gethparam(nam)))
	    arr = (incompfunc ? arrdup(arr) : arr);
	else if ((val = getsparam(nam))) {
	    arr = (char **) zhalloc(2*sizeof(char *));
	    arr[0] = (incompfunc ? dupstring(val) : val);
	    arr[1] = NULL;
	}
	unqueue_signals();
	return arr;
    }
}

/*
 * If KEYS, then NAME is an associative array; return its keys.
 * Else, NAME is a plain array; return its elements.
 */

static char **
get_data_arr(char *name, int keys)
{
    struct value vbuf;
    char **ret;
    Value v;

    queue_signals();
    if (!(v = fetchvalue(&vbuf, &name, 1,
			 (keys ? SCANPM_WANTKEYS : SCANPM_WANTVALS) |
			 SCANPM_MATCHMANY)))
	ret = NULL;
    else
	ret = getarrvalue(v);
    unqueue_signals();

    return ret;
}

static void
addmatch(char *str, int flags, char ***dispp, int line)
{
    Cmatch cm = (Cmatch) zhalloc(sizeof(struct cmatch));
    char **disp = *dispp;

    memset(cm, 0, sizeof(struct cmatch));
    cm->str = dupstring(str);
    cm->flags = (flags |
                 (complist ?
                  ((strstr(complist, "packed") ? CMF_PACKED : 0) |
                   (strstr(complist, "rows")   ? CMF_ROWS   : 0)) : 0));
    if (disp) {
        if (!*++disp)
            disp = NULL;
        if (disp)
            cm->disp = dupstring(*disp);
    } else if (line) {
        cm->disp = dupstring("");
        cm->flags |= CMF_DISPLINE;
    }
    mnum++;
    ainfo->count++;
    if (curexpl)
        curexpl->count++;

    addlinknode(matches, cm);

    newmatches = 1;
    mgroup->new = 1;

    *dispp = disp;
}

/* This is used by compadd to add a couple of matches. The arguments are
 * the strings given via options. The last argument is the array with
 * the matches. */

/**/
int
addmatches(Cadata dat, char **argv)
{
    /* ms: "match string" - string to use as completion.
     * Overloaded at one place as a temporary. */
    char *s, *ms, *lipre = NULL, *lisuf = NULL, *lpre = NULL, *lsuf = NULL;
    char **aign = NULL, ***dparr = NULL, *oaq = autoq, *oppre = dat->ppre;
    char *oqp = qipre, *oqs = qisuf, qc, **disp = NULL, *ibuf = NULL;
    char **arrays = NULL;
    int dind, lpl, lsl, bcp = 0, bcs = 0, bpadd = 0, bsadd = 0;
    int ppl = 0, psl = 0, ilen = 0;
    int llpl = 0, llsl = 0, nm = mnum, gflags = 0, ohp = haspattern;
    int isexact, doadd, ois = instring, oib = inbackt;
    Cline lc = NULL, pline = NULL, sline = NULL;
    struct cmlist mst;
    Cmlist oms = mstack;
    Patprog cp = NULL, *pign = NULL;
    LinkList aparl = NULL, oparl = NULL, *dparl = NULL;
    Brinfo bp, bpl = brbeg, obpl, bsl = brend, obsl;
    Heap oldheap;

    SWITCHHEAPS(oldheap, compheap) {
        if (dat->dummies >= 0)
            dat->aflags = ((dat->aflags | CAF_NOSORT | CAF_UNIQCON) &
                           ~CAF_UNIQALL);

        /* Select the group in which to store the matches. */
        gflags = (((dat->aflags & CAF_NOSORT ) ? CGF_NOSORT  : 0) |
                  ((dat->aflags & CAF_MATSORT) ? CGF_MATSORT : 0) |
                  ((dat->aflags & CAF_NUMSORT) ? CGF_NUMSORT : 0) |
                  ((dat->aflags & CAF_REVSORT) ? CGF_REVSORT : 0) |
                  ((dat->aflags & CAF_UNIQALL) ? CGF_UNIQALL : 0) |
                  ((dat->aflags & CAF_UNIQCON) ? CGF_UNIQCON : 0));
        if (dat->group) {
            endcmgroup(NULL);
            begcmgroup(dat->group, gflags);
        } else {
            endcmgroup(NULL);
            begcmgroup("default", 0);
        }
        if (dat->mesg || dat->exp) {
            curexpl = (Cexpl) zhalloc(sizeof(struct cexpl));
            curexpl->always = !!dat->mesg;
            curexpl->count = curexpl->fcount = 0;
            curexpl->str = dupstring(dat->mesg ? dat->mesg : dat->exp);
            if (dat->mesg && !dat->dpar && !dat->opar && !dat->apar)
                addexpl(1);
        } else
            curexpl = NULL;
    } SWITCHBACKHEAPS(oldheap);

    if (!*argv && !dat->dummies && !(dat->aflags & CAF_ALL))
	return 1;

    for (bp = brbeg; bp; bp = bp->next)
	bp->curpos = ((dat->aflags & CAF_QUOTE) ? bp->pos : bp->qpos);
    for (bp = brend; bp; bp = bp->next)
	bp->curpos = ((dat->aflags & CAF_QUOTE) ? bp->pos : bp->qpos);

    if (dat->flags & CMF_ISPAR)
	dat->flags |= parflags;
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
	    autoq = multiquote(*compquote == '$' ? compquote+1 : compquote, 1);
	}
    } else {
	instring = QT_NONE;
	inbackt = 0;
	autoq = NULL;
    }
    qipre = ztrdup(compqiprefix ? compqiprefix : "");
    qisuf = ztrdup(compqisuffix ? compqisuffix : "");

    useexact = (compexact && !strcmp(compexact, "accept"));

    /* Switch back to the heap that was used when the completion widget
     * was invoked. */
    SWITCHHEAPS(oldheap, compheap) {
	if ((doadd = (!dat->apar && !dat->opar && !dat->dpar))) {
	    if (dat->aflags & CAF_MATCH)
		hasmatched = 1;
	    else
		hasunmatched = 1;
	}
	if (dat->apar)
	    aparl = newlinklist();
	if (dat->opar)
	    oparl = newlinklist();
	if (dat->dpar) {
	    int darr = 0, dparlen = arrlen(dat->dpar);
	    char **tail = dat->dpar + dparlen;

	    dparr = (char ***)hcalloc((1 + dparlen) * sizeof(char **));
	    dparl = (LinkList *)hcalloc((1 + dparlen) * sizeof(LinkList));
	    queue_signals();
	    while (darr < dparlen) {
		if ((dparr[darr] = getaparam(dat->dpar[darr])) && *dparr[darr]) {
		    dparr[darr] = arrdup(dparr[darr]);
		    dparl[darr++] = newlinklist();
		} else {
		    /* swap in the last -D argument if we didn't get a non-empty array */
		    dat->dpar[darr] = *--tail;
		    *tail = NULL;
		    --dparlen;
	        }
	    }
	    unqueue_signals();
	}
	/* Store the matcher in our stack of matchers. */
	if (dat->match) {
	    mst.next = mstack;
	    mst.matcher = dat->match;
	    mstack = &mst;

	    add_bmatchers(dat->match);

	    addlinknode(matchers, dat->match);
	    dat->match->refc++;
	}
	if (mnum && (mstack || bmatchers))
	    update_bmatchers();

	/* Get the suffixes to ignore. */
	if (dat->ign && (aign = get_user_var(dat->ign))) {
	    char **ap, **sp, *tmp;
	    Patprog *pp, prog;

	    pign = (Patprog *) zhalloc((arrlen(aign) + 1) * sizeof(Patprog));

	    for (ap = sp = aign, pp = pign; (tmp = *ap); ap++) {
		tokenize(tmp);
		remnulargs(tmp);
		if (((tmp[0] == Quest && tmp[1] == Star) ||
		     (tmp[1] == Quest && tmp[0] == Star)) &&
		    tmp[2] && !haswilds(tmp + 2))
		    untokenize(*sp++ = tmp + 2);
		else if ((prog = patcompile(tmp, 0, NULL)))
		    *pp++ = prog;
	    }
	    *sp = NULL;
	    *pp = NULL;
	    if (!*aign)
		aign = NULL;
	    if (!*pign)
		pign = NULL;
	}
	/* Get the display strings. */
	if (dat->disp)
	    if ((disp = get_user_var(dat->disp)))
		disp--;
	/* Get the contents of the completion variables if we have
	 * to perform matching. */
	if (dat->aflags & CAF_MATCH) {
	    lipre = dupstring(compiprefix);
	    lisuf = dupstring(compisuffix);
	    lpre = dupstring(compprefix);
	    lsuf = dupstring(compsuffix);
	    llpl = strlen(lpre);
	    llsl = strlen(lsuf);

	    if (llpl + (int)strlen(compqiprefix) + (int)strlen(lipre) != origlpre
	     || llsl + (int)strlen(compqisuffix) + (int)strlen(lisuf) != origlsuf)
		lenchanged = 1;

	    /* Test if there is an existing -P prefix. */
	    if (dat->pre && *dat->pre) {
		int prefix_length = pfxlen(dat->pre, lpre);
		if (dat->pre[prefix_length] == '\0' ||
		    lpre[prefix_length] == '\0') {
		    /* $compadd_args[-P] is a prefix of ${PREFIX}, or
		     * vice-versa. */
		    llpl -= prefix_length;
		    lpre += prefix_length;
		}
	    }
	}
	/* Now duplicate the strings we have from the command line. */
	if (dat->ipre)
	    dat->ipre = (lipre ? dyncat(lipre, dat->ipre) :
			 dupstring(dat->ipre));
	else if (lipre)
	    dat->ipre = lipre;
	if (dat->isuf)
	    dat->isuf = (lisuf ? dyncat(lisuf, dat->isuf) :
			 dupstring(dat->isuf));
	else if (lisuf)
	    dat->isuf = lisuf;
	if (dat->ppre) {
	    dat->ppre = ((dat->flags & CMF_FILE) ?
			 tildequote(dat->ppre, !!(dat->aflags & CAF_QUOTE)) :
			 multiquote(dat->ppre, !!(dat->aflags & CAF_QUOTE)));
	    lpl = strlen(dat->ppre);
	} else
	    lpl = 0;
	if (dat->psuf) {
	    dat->psuf = multiquote(dat->psuf, !!(dat->aflags & CAF_QUOTE));
	    lsl = strlen(dat->psuf);
	} else
	    lsl = 0;
	if (dat->aflags & CAF_MATCH) {
	    int ml, gfl = 0;
	    char *globflag = NULL;

	    if (comppatmatch && *comppatmatch &&
		dat->ppre && lpre[0] == '(' && lpre[1] == '#') {
		char *p;

		for (p = lpre + 2; *p && *p != ')'; p++);

		if (*p == ')') {
		    char sav = p[1];

		    p[1] = '\0';
		    globflag = dupstring(lpre);
		    gfl = p - lpre + 1;
		    p[1] = sav;

		    lpre = p + 1;
		    llpl -= gfl;
		}
	    }
	    if ((s = dat->ppre)) {
		if ((ml = match_str(lpre, s, &bpl, 0, NULL, 0, 0, 1)) >= 0) {
		    if (matchsubs) {
			Cline tmp = get_cline(NULL, 0, NULL, 0, NULL, 0, 0);

			tmp->prefix = matchsubs;
			if (matchlastpart)
			    matchlastpart->next = tmp;
			else
			    matchparts = tmp;
		    }
		    pline = matchparts;
		    lpre += ml;
		    llpl -= ml;
		    bcp = ml;
		    bpadd = strlen(s) - ml;
		} else {
		    if (llpl <= lpl && strpfx(lpre, s))
			lpre = dupstring("");
		    else if (llpl > lpl && strpfx(s, lpre))
			lpre += lpl;
		    else
			*argv = NULL;
		    bcp = lpl;
		}
	    }
	    if ((s = dat->psuf)) {
		if ((ml = match_str(lsuf, s, &bsl, 0, NULL, 1, 0, 1)) >= 0) {
		    if (matchsubs) {
			Cline tmp = get_cline(NULL, 0, NULL, 0, NULL, 0, CLF_SUF);

			tmp->suffix = matchsubs;
			if (matchlastpart)
			    matchlastpart->next = tmp;
			else
			    matchparts = tmp;
		    }
		    sline = revert_cline(matchparts);
		    lsuf[llsl - ml] = '\0';
		    llsl -= ml;
		    bcs = ml;
		    bsadd = strlen(s) - ml;
		} else {
		    if (llsl <= lsl && strsfx(lsuf, s))
			lsuf = dupstring("");
		    else if (llsl > lsl && strsfx(s, lsuf))
			lsuf[llsl - lsl] = '\0';
		    else
			*argv = NULL;
		    bcs = lsl;
		}
	    }
	    if (comppatmatch && *comppatmatch) {
		int is = (*comppatmatch == '*');
		char *tmp = (char *) zhalloc(2 + llpl + llsl + gfl);

		if (gfl) {
		    strcpy(tmp, globflag);
		    strcat(tmp, lpre);
		} else
		    strcpy(tmp, lpre);
		tmp[llpl + gfl] = 'x';
		strcpy(tmp + llpl + gfl + is, lsuf);

		tokenize(tmp);
		if (haswilds(tmp)) {
		    if (is)
			tmp[llpl + gfl] = Star;
		    remnulargs(tmp);
		    if ((cp = patcompile(tmp, 0, NULL)))
			haspattern = 1;
		}
	    }
	} else {
	    /*
	     * (This is called a "comment".  Given you've been
	     * spending your time reading the completion code, you
	     * may have forgotten what one is.  It's used to deconfuse
	     * the poor so-and-so who's landed up having to maintain
	     * the code.)
	     *
	     * So what's going on here then?  I'm glad you asked.  To test
	     * whether we should start menu completion, we test whether
	     * compstate[insert] has been set to "menu", but only if we found
	     * patterns in the code.  It's not clear to me from the
	     * documentation why the second condition would apply, but sure
	     * enough if I remove it the test suite falls over.  (Testing
	     * comppatmatch at the later point doesn't work because compstate
	     * is likely to have been reset by the point we actually insert
	     * the completions, after all functions have exited; this is at
	     * least part of the problem.)  In the present case, we are not
	     * doing matching on the code because all the clever stuff has
	     * been done over our heads and we've simply between told to
	     * insert it.  However, we still need to take account of ambiguous
	     * completions properly.  To do this, we rely on the caller to
	     * pass down the same prefix/suffix with the patterns that we
	     * would get if we were doing matching, and test those for
	     * patterns.  This gets us out of the hole apparently without
	     * breaking anything.  The particular case where this is needed is
	     * approximate file completion: this does its own matching but
	     * _approximate still sets the prefix to include the pattern.
	     */
	    if (comppatmatch && *comppatmatch) {
		int pflen = strlen(compprefix);
		char *tmp = zhalloc(pflen + strlen(compsuffix) + 1);
		strcpy(tmp, compprefix);
		strcpy(tmp + pflen, compsuffix);
		tokenize(tmp);
		remnulargs(tmp);
		if (haswilds(tmp))
		    haspattern = 1;
	    }
	}
	if (*argv) {
	    if (dat->pre)
		dat->pre = dupstring(dat->pre);
	    if (dat->suf)
		dat->suf = dupstring(dat->suf);
	    if (!dat->prpre && (dat->prpre = dupstring(oppre))) {
		singsub(&(dat->prpre));
		untokenize(dat->prpre);
	    } else
		dat->prpre = dupstring(dat->prpre);
	    /* Select the set of matches. */

	    if (dat->remf) {
		dat->remf = dupstring(dat->remf);
		dat->rems = NULL;
	    } else if (dat->rems)
		dat->rems = dupstring(dat->rems);

	    if (lpre)
		lpre = ((!(dat->aflags & CAF_QUOTE) &&
			 (!dat->ppre && (dat->flags & CMF_FILE))) ?
			tildequote(lpre, 1) : multiquote(lpre, 1));
	    if (lsuf)
		lsuf = multiquote(lsuf, 1);
	}
	/* Walk through the matches given. */
	obpl = bpl;
	obsl = bsl;
	if (dat->aflags & CAF_ARRAYS) {
	    Heap oldheap2;

	    SWITCHHEAPS(oldheap2, oldheap) {
		arrays = argv;
		argv = NULL;
		while (*arrays &&
		       (!(argv = get_data_arr(*arrays,
					      (dat->aflags & CAF_KEYS))) ||
			!*argv))
		    arrays++;
		arrays++;
		if (!argv) {
		    ms = NULL;
		    argv = &ms;
		}
	    } SWITCHBACKHEAPS(oldheap2);
	}
	if (dat->ppre)
	    ppl = strlen(dat->ppre);
	if (dat->psuf)
	    psl = strlen(dat->psuf);
	for (; (s = *argv); argv++) {
	    int sl;
	    bpl = obpl;
	    bsl = obsl;
	    if (disp) {
		if (!*++disp)
		    disp = NULL;
	    }
	    sl = strlen(s);
	    if (aign || pign) {
		int il = ppl + sl + psl, addit = 1;

		if (il + 1> ilen)
		    ibuf = (char *) zhalloc((ilen = il) + 2);

		if (ppl)
		    memcpy(ibuf, dat->ppre, ppl);
		strcpy(ibuf + ppl, s);
		if (psl)
		    strcpy(ibuf + ppl + sl, dat->psuf);

		if (aign) {
		    /* Do the suffix-test. If the match has one of the
		     * suffixes from aign, we put it in the alternate set. */
		    char **pt = aign;
		    int filell;

		    for (; addit && *pt; pt++)
			addit = !((filell = strlen(*pt)) < il &&
				  !strcmp(*pt, ibuf + il - filell));
		}
		if (addit && pign) {
		    Patprog *pt = pign;

		    for (; addit && *pt; pt++)
			addit = !pattry(*pt, ibuf);
		}
		if (!addit) {
		    compignored++;
		    for (dind = 0; dparl && dparl[dind]; dind++) {
			if (dparr[dind] && !*++dparr[dind])
			    dparr[dind] = NULL;
		    }
		    goto next_array;
		}
	    }
	    if (!(dat->aflags & CAF_MATCH)) {
		if (dat->aflags & CAF_QUOTE)
		    ms = dupstring(s);
		else
		    sl = strlen(ms = multiquote(s, 0));
		lc = bld_parts(ms, sl, -1, NULL, NULL);
		isexact = 0;
	    } else if (!(ms = comp_match(lpre, lsuf, s, cp, &lc,
					 (!(dat->aflags & CAF_QUOTE) ?
					  (dat->ppre ||
					   !(dat->flags & CMF_FILE) ? 1 : 2) : 0),
					 &bpl, bcp, &bsl, bcs,
					 &isexact))) {
		for (dind = 0; dparl && dparl[dind]; dind++) {
		    if (dparr[dind] && !*++dparr[dind])
			dparr[dind] = NULL;
		}
		goto next_array;
	    }
	    if (doadd) {
		Cmatch cm;
		Brinfo bp;

		for (bp = obpl; bp; bp = bp->next)
		    bp->curpos += bpadd;
		for (bp = obsl; bp; bp = bp->next)
		    bp->curpos += bsadd;

		if ((cm = add_match_data(0, ms, s, lc, dat->ipre, NULL,
					 dat->isuf, dat->pre, dat->prpre,
					 dat->ppre, pline,
					 dat->psuf, sline,
					 dat->suf, dat->flags, isexact))) {
		    cm->rems = dat->rems;
		    cm->remf = dat->remf;
		    if (disp)
			cm->disp = dupstring(*disp);
		}
	    } else {
		if (dat->apar)
		    addlinknode(aparl, ms);
		if (dat->opar)
		    addlinknode(oparl, s);
		if (dat->dpar) {
		    for (dind = 0; dparl[dind]; dind++) {
			if (dparr[dind]) {
			    addlinknode(dparl[dind], *dparr[dind]);
			    if (!*++dparr[dind])
				dparr[dind] = NULL;
			}
		    }
		}
		free_cline(lc);
	    }
	next_array:
	    if ((dat->aflags & CAF_ARRAYS) && !argv[1]) {
		Heap oldheap2;

		SWITCHHEAPS(oldheap2, oldheap) {
		    argv = NULL;
		    while (*arrays &&
			   (!(argv = get_data_arr(*arrays,
						  (dat->aflags & CAF_KEYS))) ||
			    !*argv))
			arrays++;
		    arrays++;
		    if (!argv) {
			ms = NULL;
			argv = &ms;
		    }
		    argv--;
		} SWITCHBACKHEAPS(oldheap2);
	    }
	}
	if (dat->apar)
	    set_list_array(dat->apar, aparl);
	if (dat->opar)
	    set_list_array(dat->opar, oparl);
	if (dat->dpar) {
	    for (dind = 0; dparl[dind]; dind++)
		set_list_array(dat->dpar[dind], dparl[dind]);
	}
	if (dat->exp)
	    addexpl(0);
	if (!hasallmatch && (dat->aflags & CAF_ALL)) {
            addmatch("<all>", dat->flags | CMF_ALL, &disp, 1);
	    hasallmatch = 1;
	}
        while (dat->dummies-- > 0)
            addmatch("", dat->flags | CMF_DUMMY, &disp, 0);

    } SWITCHBACKHEAPS(oldheap);

    /* We switched back to the current heap, now restore the stack of
     * matchers. */
    mstack = oms;

    instring = ois;
    inbackt = oib;
    autoq = oaq;
    zsfree(qipre);
    zsfree(qisuf);
    qipre = oqp;
    qisuf = oqs;

    if (mnum == nm)
	haspattern = ohp;

    return (mnum == nm);
}

/* This adds all the data we have for a match. */

/**/
mod_export Cmatch
add_match_data(int alt, char *str, char *orig, Cline line,
	       char *ipre, char *ripre, char *isuf,
	       char *pre, char *prpre,
	       char *ppre, Cline pline,
	       char *psuf, Cline sline,
	       char *suf, int flags, int exact)
{
#ifdef MULTIBYTE_SUPPORT
    mbstate_t mbs;
    char curchar, *t, *f, *fs, *fe, *new_str = NULL;
    size_t cnt;
    wchar_t wc;
#endif
    Cmatch cm;
    Aminfo ai = (alt ? fainfo : ainfo);
    int palen, salen, qipl, ipl, pl, ppl, qisl, isl, psl;
    int stl, lpl, lsl, ml;

    palen = salen = qipl = ipl = pl = ppl = qisl = isl = psl = 0;

    DPUTS(!line, "BUG: add_match_data() without cline");

    cline_matched(line);
    if (pline)
	cline_matched(pline);
    if (sline)
	cline_matched(sline);

    /* If there is a path suffix, we build a cline list for it and
     * append it to the list for the match itself. */
    if (!sline && psuf)
	salen = (psl = strlen(psuf));
    if (isuf)
	salen += (isl = strlen(isuf));
    if (qisuf)
	salen += (qisl = strlen(qisuf));

    if (salen) {
	Cline pp, p, s, sl = NULL;

	for (pp = NULL, p = line; p->next; pp = p, p = p->next);

	if (psl) {
	    s = bld_parts(psuf, psl, psl, &sl, NULL);

	    if (sline) {
		Cline sp;

		sline = cp_cline(sline, 1);

		for (sp = sline; sp->next; sp = sp->next);
		sp->next = s;
		s = sline;
		sline = NULL;
	    }
	    if (!(p->flags & (CLF_SUF | CLF_MID)) &&
		!p->llen && !p->wlen && !p->olen) {
		if (p->prefix) {
		    Cline q;

		    for (q = p->prefix; q->next; q = q->next);
		    q->next = s->prefix;
		    s->prefix = p->prefix;
		    p->prefix = NULL;
		}
		s->flags |= (p->flags & CLF_MATCHED) | CLF_MID;
		free_cline(p);
		if (pp)
		    pp->next = s;
		else
		    line = s;
	    } else
		p->next = s;
	}
	if (isl) {
	    Cline tsl;

	    s = bld_parts(isuf, isl, isl, &tsl, NULL);

	    if (sl)
		sl->next = s;
	    else if (sline) {
		Cline sp;

		sline = cp_cline(sline, 1);

		for (sp = sline; sp->next; sp = sp->next);
		sp->next = s;
		p->next = sline;
		sline = NULL;
	    } else
		p->next = s;

	    sl = tsl;
	}
	if (qisl) {
	    Cline qsl = bld_parts(dupstring(qisuf), qisl, qisl, NULL, NULL);

	    qsl->flags |= CLF_SUF;
	    qsl->suffix = qsl->prefix;
	    qsl->prefix = NULL;
	    if (sl)
		sl->next = qsl;
	    else if (sline) {
		Cline sp;

		sline = cp_cline(sline, 1);

		for (sp = sline; sp->next; sp = sp->next);
		sp->next = qsl;
		p->next = sline;
	    } else
		p->next = qsl;
	}
    } else if (sline) {
	Cline p;

	for (p = line; p->next; p = p->next);
	p->next = cp_cline(sline, 1);
    }
    /* The prefix is handled differently because the completion code
     * is much more eager to insert the -P prefix than it is to insert
     * the -S suffix. */
    if (qipre)
	palen = (qipl = strlen(qipre));
    if (ipre)
	palen += (ipl = strlen(ipre));
    if (pre)
	palen += (pl = strlen(pre));
    if (!pline && ppre)
	palen += (ppl = strlen(ppre));

    if (pl) {
	if (ppl || pline) {
	    Cline lp, p;

	    if (pline)
		for (p = cp_cline(pline, 1), lp = p; lp->next; lp = lp->next);
	    else
		p = bld_parts(ppre, ppl, ppl, &lp, NULL);

	    if (lp->prefix && !(line->flags & (CLF_SUF | CLF_MID)) &&
		!lp->llen && !lp->wlen && !lp->olen) {
		Cline lpp;

		for (lpp = lp->prefix; lpp->next; lpp = lpp->next);

		lpp->next = line->prefix;
		line->prefix = lp->prefix;
		lp->prefix = NULL;

		free_cline(lp);

		if (p != lp) {
		    Cline q;

		    for (q = p; q->next != lp; q = q->next);

		    q->next = line;
		    line = p;
		}
	    } else {
		lp->next = line;
		line = p;
	    }
	}
	if (pl) {
	    Cline lp, p = bld_parts(pre, pl, pl, &lp, NULL);

	    lp->next = line;
	    line = p;
	}
	if (ipl) {
	    Cline lp, p = bld_parts(ipre, ipl, ipl, &lp, NULL);

	    lp->next = line;
	    line = p;
	}
	if (qipl) {
	    Cline lp, p = bld_parts(dupstring(qipre), qipl, qipl, &lp, NULL);

	    lp->next = line;
	    line = p;
	}
    } else if (palen || pline) {
	Cline p, lp;

	if (palen) {
	    char *apre = (char *) zhalloc(palen);

	    if (qipl)
		memcpy(apre, qipre, qipl);
	    if (ipl)
		memcpy(apre + qipl, ipre, ipl);
	    if (pl)
		memcpy(apre + qipl + ipl, pre, pl);
	    if (ppl)
		memcpy(apre + qipl + ipl + pl, ppre, ppl);

	    p = bld_parts(apre, palen, palen, &lp, NULL);

	    if (pline)
		for (lp->next = cp_cline(pline, 1); lp->next; lp = lp->next);
	} else
	    for (p = lp = cp_cline(pline, 1); lp->next; lp = lp->next);

	if (lp->prefix && !(line->flags & CLF_SUF) &&
	    !lp->llen && !lp->wlen && !lp->olen) {
	    Cline lpp;

	    for (lpp = lp->prefix; lpp->next; lpp = lpp->next);

	    lpp->next = line->prefix;
	    line->prefix = lp->prefix;
	    lp->prefix = NULL;

	    free_cline(lp);

	    if (p != lp) {
		Cline q;

		for (q = p; q->next != lp; q = q->next);

		q->next = line;
		line = p;
	    }
	} else {
	    lp->next = line;
	    line = p;
	}
    }

    stl = strlen(str);
#ifdef MULTIBYTE_SUPPORT
    /* If "str" contains a character that won't convert into a wide
     * character, change it into a $'\123' sequence. */
    memset(&mbs, '\0', sizeof mbs);
    for (t = f = fs = str, fe = f + stl; fs < fe; ) {
	if ((curchar = *f++) == Meta)
	    curchar = *f++ ^ 32;
	cnt = mbrtowc(&wc, &curchar, 1, &mbs);
	switch (cnt) {
	case MB_INCOMPLETE:
	    if (f < fe)
		continue;
	    /* FALL THROUGH */
	case MB_INVALID:
	    /* Get mbs out of its undefined state. */
	    memset(&mbs, '\0', sizeof mbs);
	    if (!new_str) {
		/* Be very pessimistic about how much space we'll need. */
		new_str = zhalloc((t - str) + (fe - fs)*7 + 1);
		memcpy(new_str, str, t - str);
		t = new_str + (t - str);
	    }
	    /* Output one byte from the start of this invalid multibyte
	     * sequence unless we got MB_INCOMPLETE at the end of the
	     * string, in which case we output all the incomplete bytes. */
	    do {
		if ((curchar = *fs++) == Meta)
		    curchar = *fs++ ^ 32;
		*t++ = '$';
		*t++ = '\'';
		*t++ = '\\';
		*t++ = '0' + ((STOUC(curchar) >> 6) & 7);
		*t++ = '0' + ((STOUC(curchar) >> 3) & 7);
		*t++ = '0' + (STOUC(curchar) & 7);
		*t++ = '\'';
	    } while (cnt == MB_INCOMPLETE && fs < fe);
	    /* Scanning restarts from the spot after the char we skipped. */
	    f = fs;
	    break;
	default:
	    while (fs < f)
		*t++ = *fs++;
	    break;
	}
    }
    if (new_str) {
	*t = '\0';
	str = new_str;
	stl = t - str;
    }
#endif

    /* Allocate and fill the match structure. */
    cm = (Cmatch) zhalloc(sizeof(struct cmatch));
    cm->str = str;
    cm->orig = dupstring(orig);
    cm->ppre = (ppre && *ppre ? ppre : NULL);
    cm->psuf = (psuf && *psuf ? psuf : NULL);
    cm->prpre = ((flags & CMF_FILE) && prpre && *prpre ? prpre : NULL);
    if (qipre && *qipre)
	cm->ipre = (ipre && *ipre ? dyncat(qipre, ipre) : dupstring(qipre));
    else
	cm->ipre = (ipre && *ipre ? ipre : NULL);
    cm->ripre = (ripre && *ripre ? ripre : NULL);
    if (qisuf && *qisuf)
	cm->isuf = (isuf && *isuf ? dyncat(isuf, qisuf) : dupstring(qisuf));
    else
	cm->isuf = (isuf && *isuf ? isuf : NULL);
    cm->pre = pre;
    cm->suf = suf;
    cm->flags = (flags |
		 (complist ?
		  ((strstr(complist, "packed") ? CMF_PACKED : 0) |
		   (strstr(complist, "rows")   ? CMF_ROWS   : 0)) : 0));
    cm->mode = cm->fmode = 0;
    cm->modec = cm->fmodec = '\0';
    if ((flags & CMF_FILE) && orig[0] && orig[strlen(orig) - 1] != '/') {
        struct stat buf;
	char *pb;

        pb = (char *) zhalloc((cm->prpre ? strlen(cm->prpre) : 0) +
                              3 + strlen(orig));
        sprintf(pb, "%s%s", (cm->prpre ? cm->prpre : "./"), orig);

        if (!ztat(pb, &buf, 1)) {
            cm->mode = buf.st_mode;
            if ((cm->modec = file_type(buf.st_mode)) == ' ')
                cm->modec = '\0';
        }
        if (!ztat(pb, &buf, 0)) {
            cm->fmode = buf.st_mode;
            if ((cm->fmodec = file_type(buf.st_mode)) == ' ')
                cm->fmodec = '\0';
        }
    }
    if ((*compqstack == QT_BACKSLASH && compqstack[1]) ||
	(autoq && *compqstack && compqstack[1] == QT_BACKSLASH))
	cm->flags |= CMF_NOSPACE;
    if (nbrbeg) {
	int *p;
	Brinfo bp;

	cm->brpl = (int *) zhalloc(nbrbeg * sizeof(int));

	for (p = cm->brpl, bp = brbeg; bp; p++, bp = bp->next)
	    *p = bp->curpos;
    } else
	cm->brpl = NULL;
    if (nbrend) {
	int *p;
	Brinfo bp;

	cm->brsl = (int *) zhalloc(nbrend * sizeof(int));

	for (p = cm->brsl, bp = brend; bp; p++, bp = bp->next)
	    *p = bp->curpos;
    } else
	cm->brsl = NULL;
    cm->qipl = qipl;
    cm->qisl = qisl;
    cm->autoq = dupstring(autoq ? autoq : (inbackt ? "`" : NULL));
    cm->rems = cm->remf = cm->disp = NULL;

    if ((lastprebr || lastpostbr) && !hasbrpsfx(cm, lastprebr, lastpostbr))
	return NULL;

    /* Then build the unambiguous cline list. */
    ai->line = join_clines(ai->line, line);

    mnum++;
    ai->count++;

    addlinknode((alt ? fmatches : matches), cm);

    newmatches = 1;
    mgroup->new = 1;
    if (alt)
	compignored++;

    if (!complastprompt || !*complastprompt)
	dolastprompt = 0;
    /* One more match for this explanation. */
    if (curexpl) {
	if (alt)
	    curexpl->fcount++;
	else
	    curexpl->count++;
    }
    if (!ai->firstm)
	ai->firstm = cm;

    lpl = (cm->ppre ? strlen(cm->ppre) : 0);
    lsl = (cm->psuf ? strlen(cm->psuf) : 0);
    ml = stl + lpl + lsl;

    if (ml < minmlen)
	minmlen = ml;
    if (ml > maxmlen)
	maxmlen = ml;

    /* Do we have an exact match? More than one? */
    if (exact) {
	if (!ai->exact) {
	    ai->exact = useexact;
	    if (incompfunc && (!compexactstr || !*compexactstr)) {
		/* If a completion widget is active, we make the exact
		 * string available in `compstate'. */

		char *e;

		zsfree(compexactstr);
		compexactstr = e = (char *) zalloc(ml + 1);
		if (cm->ppre) {
		    strcpy(e, cm->ppre);
		    e += lpl;
		}
		strcpy(e, str);
		e += stl;
		if (cm->psuf)
		    strcpy(e, cm->psuf);
		comp_setunset(0, 0, CP_EXACTSTR, 0);
	    }
	    ai->exactm = cm;
	} else if (useexact && (!ai->exactm || !matcheq(cm, ai->exactm))) {
	    ai->exact = 2;
	    ai->exactm = NULL;
	    if (incompfunc)
		comp_setunset(0, 0, 0, CP_EXACTSTR);
	}
    }
    return cm;
}

/* This begins a new group of matches. */

/**/
mod_export void
begcmgroup(char *n, int flags)
{
    if (n) {
	/* If a group named <n> already exists, reuse it. */
	Cmgroup p;
	for (p = amatches; p; p = p->next) {
#ifdef ZSH_HEAP_DEBUG
	    if (memory_validate(p->heap_id)) {
		HEAP_ERROR(p->heap_id);
	    }
#endif
	    if (p->name && flags ==
		(p->flags & (CGF_NOSORT|CGF_UNIQALL|CGF_UNIQCON|
			     CGF_MATSORT|CGF_NUMSORT|CGF_REVSORT)) &&
		!strcmp(n, p->name)) {
		mgroup = p;

		expls = p->lexpls;
		matches = p->lmatches;
		fmatches = p->lfmatches;
		allccs = p->lallccs;

		return;
	    }
	}
    }

    /* Create a new group. */
    mgroup = (Cmgroup) zhalloc(sizeof(struct cmgroup));
#ifdef ZSH_HEAP_DEBUG
    mgroup->heap_id = last_heap_id;
#endif
    mgroup->name = dupstring(n);
    mgroup->lcount = mgroup->llcount = mgroup->mcount = mgroup->ecount = 
	mgroup->ccount = 0;
    mgroup->flags = flags;
    mgroup->matches = NULL;
    mgroup->ylist = NULL;
    mgroup->expls = NULL;
    mgroup->perm = NULL;
    mgroup->new = mgroup->num = mgroup->nbrbeg = mgroup->nbrend = 0;

    mgroup->lexpls = expls = newlinklist();
    mgroup->lmatches = matches = newlinklist();
    mgroup->lfmatches = fmatches = newlinklist();

    mgroup->lallccs = allccs = ((flags & CGF_NOSORT) ? NULL : newlinklist());

    if ((mgroup->next = amatches))
	amatches->prev = mgroup;
    mgroup->prev = NULL;
    amatches = mgroup;
}

/* End the current group for now. */

/**/
mod_export void
endcmgroup(char **ylist)
{
    mgroup->ylist = ylist;
}

/* Add an explanation string to the current group, joining duplicates. */

/**/
mod_export void
addexpl(int always)
{
    LinkNode n;
    Cexpl e;

    for (n = firstnode(expls); n; incnode(n)) {
	e = (Cexpl) getdata(n);
	if (!strcmp(curexpl->str, e->str)) {
	    e->count += curexpl->count;
	    e->fcount += curexpl->fcount;
            if (always) {
                e->always = 1;
                nmessages++;
                newmatches = 1;
                mgroup->new = 1;
            }
	    return;
	}
    }
    addlinknode(expls, curexpl);
    newmatches = 1;
    if (always) {
        mgroup->new = 1;
        nmessages++;
    }
}

/* The comparison function for matches (used for sorting). */

static int matchorder;

/**/
static int
matchcmp(Cmatch *a, Cmatch *b)
{
    const char *as, *bs;
    int cmp = !!(*b)->disp - !!(*a)->disp;
    int sortdir = (matchorder & CGF_REVSORT) ? -1 : 1;

    /* if match sorting selected or we have no display strings */
    if ((matchorder & CGF_MATSORT) || (!cmp && !(*a)->disp)) {
	as = (*a)->str;
	bs = (*b)->str;
    } else {
        if (cmp) /* matches with display strings come first */
	    return cmp;

	cmp = ((*b)->flags & CMF_DISPLINE) - ((*a)->flags & CMF_DISPLINE);
        if (cmp) /* sort one-per-line display strings first */
	    return cmp;

	as = (*a)->disp;
	bs = (*b)->disp;
    }

    return sortdir * zstrcmp(as, bs, SORTIT_IGNORING_BACKSLASHES|
	    ((isset(NUMERICGLOBSORT) ||
	    matchorder & CGF_NUMSORT) ? SORTIT_NUMERICALLY : 0));
}

/* This tests whether two matches are equal (would produce the same
 * strings on the command line). */

#define matchstreq(a, b) ((!(a) && !(b)) || ((a) && (b) && !strcmp((a), (b))))

/**/
static int
matcheq(Cmatch a, Cmatch b)
{
    return matchstreq(a->ipre, b->ipre) &&
	matchstreq(a->pre, b->pre) &&
	matchstreq(a->ppre, b->ppre) &&
	matchstreq(a->psuf, b->psuf) &&
	matchstreq(a->suf, b->suf) &&
	  matchstreq(a->str, b->str);
}

/* Make an array from a linked list. The second argument says whether *
 * the array should be sorted. The third argument is used to return   *
 * the number of elements in the resulting array. The fourth argument *
 * is used to return the number of NOLIST elements. */

/**/
static Cmatch *
makearray(LinkList l, int type, int flags, int *np, int *nlp, int *llp)
{
    Cmatch *ap, *bp, *cp, *rp;
    LinkNode nod;
    int n, nl = 0, ll = 0;

    /* Build an array for the matches. */
    rp = ap = (Cmatch *) hcalloc(((n = countlinknodes(l)) + 1) *
				 sizeof(Cmatch));

    /* And copy them into it. */
    for (nod = firstnode(l); nod; incnode(nod))
	*ap++ = (Cmatch) getdata(nod);
    *ap = NULL;

    if (!type) {
	if (flags) {
	    char **ap, **bp, **cp;

	    /* Now sort the array (it contains strings). */
	    strmetasort((char **)rp, SORTIT_IGNORING_BACKSLASHES |
			(isset(NUMERICGLOBSORT) ? SORTIT_NUMERICALLY : 0),
			NULL);

	    /* And delete the ones that occur more than once. */
	    for (ap = cp = (char **) rp; *ap; ap++) {
		*cp++ = *ap;
		for (bp = ap; bp[1] && !strcmp(*ap, bp[1]); bp++, n--);
		ap = bp;
	    }
	    *cp = NULL;
	}
    } else if (n > 0) {
	if (!(flags & CGF_NOSORT)) {
	    /* Now sort the array (it contains matches). */
	    matchorder = flags;
	    qsort((void *) rp, n, sizeof(Cmatch),
		  (int (*) _((const void *, const void *)))matchcmp);

	    /* since the matches are sorted and the default is to remove
	     * all duplicates, -1 (remove only consecutive dupes) is a no-op,
	     * so this condition only checks for -2 */
	    if (!(flags & CGF_UNIQCON)) {
		int dup;

		/* we did not pass -2 so go ahead and remove those dupes */
		for (ap = cp = rp; *ap; ap++) {
		    *cp++ = *ap;
		    for (bp = ap; bp[1] && matcheq(*ap, bp[1]); bp++, n--);
		    ap = bp;
		    /* Mark those, that would show the same string in the list. */
		    for (dup = 0; bp[1] && !(*ap)->disp && !(bp[1])->disp &&
			     !strcmp((*ap)->str, (bp[1])->str); bp++) {
			(bp[1])->flags |= CMF_MULT;
			dup = 1;
		    }
		    if (dup)
			(*ap)->flags |= CMF_FMULT;
		}
		*cp = NULL;
	    }
	    for (ap = rp; *ap; ap++) {
		if ((*ap)->disp && ((*ap)->flags & CMF_DISPLINE))
		    ll++;
		if ((*ap)->flags & (CMF_NOLIST | CMF_MULT))
		    nl++;
	    }
	/* used -O nosort or -V, don't sort */
	} else {
	    /* didn't use -1 or -2, so remove all duplicates (efficient) */
	    if (!(flags & CGF_UNIQALL) && !(flags & CGF_UNIQCON)) {
                int dup, del = 0;

		/* To avoid O(n^2) here, sort a copy of the list, then remove marked elements */
		matchorder = flags;
		Cmatch *sp, *asp;
		sp = (Cmatch *) zhalloc((n + 1) * sizeof(Cmatch));
		memcpy(sp, rp, (n + 1) * sizeof(Cmatch));
		qsort((void *) sp, n, sizeof(Cmatch),
		      (int (*) _((const void *, const void *)))matchcmp);
		for (asp = sp + 1; *asp; asp++) {
		    Cmatch *ap = asp - 1, *bp = asp;
		    if (matcheq(*ap, *bp)) {
			bp[0]->flags = CMF_DELETE;
			del = 1;
		    } else if (!ap[0]->disp) {
			/* Mark those, that would show the same string in the list. */
			for (dup = 0; bp[0] && !(bp[0])->disp &&
				 !strcmp((*ap)->str, (bp[0])->str); bp = ++sp) {
			    (bp[0])->flags |= CMF_MULT;
			    dup = 1;
			}
			if (dup)
			    (*ap)->flags |= CMF_FMULT;
		    }
		}
		if (del) {
		    int n_orig = n;
		    for (bp = rp, ap = rp; bp < rp + n_orig; ap++, bp++) {
			while (bp < rp + n_orig && (bp[0]->flags & CMF_DELETE)) {
			    bp++;
			    n--;
			}
			*ap = *bp;
		    }
		    *ap = NULL;
		}
	    /* passed -1 but not -2, so remove consecutive duplicates (efficient) */
	    } else if (!(flags & CGF_UNIQCON)) {
		int dup;

		for (ap = cp = rp; *ap; ap++) {
		    *cp++ = *ap;
		    for (bp = ap; bp[1] && matcheq(*ap, bp[1]); bp++, n--);
		    ap = bp;
		    for (dup = 0; bp[1] && !(*ap)->disp && !(bp[1])->disp &&
			     !strcmp((*ap)->str, (bp[1])->str); bp++) {
			(bp[1])->flags |= CMF_MULT;
			dup = 1;
		    }
		    if (dup)
			(*ap)->flags |= CMF_FMULT;
		}
		*cp = NULL;
	    }
	    for (ap = rp; *ap; ap++) {
		if ((*ap)->disp && ((*ap)->flags & CMF_DISPLINE))
		    ll++;
		if ((*ap)->flags & (CMF_NOLIST | CMF_MULT))
		    nl++;
	    }
	}
    }
    if (np)
	*np = n;
    if (nlp)
	*nlp = nl;
    if (llp)
	*llp = ll;
    return rp;
}

/* This duplicates one match. */

/**/
static Cmatch
dupmatch(Cmatch m, int nbeg, int nend)
{
    Cmatch r;

    r = (Cmatch) zshcalloc(sizeof(struct cmatch));

    r->str = ztrdup(m->str);
    r->orig = ztrdup(m->orig);
    r->ipre = ztrdup(m->ipre);
    r->ripre = ztrdup(m->ripre);
    r->isuf = ztrdup(m->isuf);
    r->ppre = ztrdup(m->ppre);
    r->psuf = ztrdup(m->psuf);
    r->prpre = ztrdup(m->prpre);
    r->pre = ztrdup(m->pre);
    r->suf = ztrdup(m->suf);
    r->flags = m->flags;
    if (m->brpl) {
	int *p, *q, i;

	r->brpl = (int *) zalloc(nbeg * sizeof(int));

	for (p = r->brpl, q = m->brpl, i = nbeg; i--; p++, q++)
	    *p = *q;
    } else
	r->brpl = NULL;
    if (m->brsl) {
	int *p, *q, i;

	r->brsl = (int *) zalloc(nend * sizeof(int));

	for (p = r->brsl, q = m->brsl, i = nend; i--; p++, q++)
	    *p = *q;
    } else
	r->brsl = NULL;
    r->rems = ztrdup(m->rems);
    r->remf = ztrdup(m->remf);
    r->autoq = ztrdup(m->autoq);
    r->qipl = m->qipl;
    r->qisl = m->qisl;
    r->disp = ztrdup(m->disp);
    r->mode = m->mode;
    r->modec = m->modec;
    r->fmode = m->fmode;
    r->fmodec = m->fmodec;

    return r;
}

/* This duplicates all groups of matches. */

/**/
mod_export int
permmatches(int last)
{
    Cmgroup g = amatches, n;
    Cmatch *p, *q;
    Cexpl *ep, *eq, e, o;
    LinkList mlist;
    static int fi = 0;
    int nn, nl, ll, gn = 1, mn = 1, rn, ofi = fi;

    if (pmatches && !newmatches) {
	if (last && fi)
	    ainfo = fainfo;
	return fi;
    }
    newmatches = fi = 0;

    pmatches = lmatches = NULL;
    nmatches = smatches = diffmatches = 0;

    if (!ainfo->count) {
	if (last)
	    ainfo = fainfo;
	fi = 1;
    }
    while (g) {
#ifdef ZSH_HEAP_DEBUG
	if (memory_validate(g->heap_id)) {
	    HEAP_ERROR(g->heap_id);
	}
#endif
	if (fi != ofi || !g->perm || g->new) {
	    if (fi)
		/* We have no matches, try ignoring fignore. */
		mlist = g->lfmatches;
	    else
		mlist = g->lmatches;

	    g->matches = makearray(mlist, 1, g->flags, &nn, &nl, &ll);
	    g->mcount = nn;
	    if ((g->lcount = nn - nl) < 0)
		g->lcount = 0;
	    g->llcount = ll;
	    if (g->ylist) {
		g->lcount = arrlen(g->ylist);
		smatches = 2;
	    }
	    g->expls = (Cexpl *) makearray(g->lexpls, 0, 0, &(g->ecount),
					   NULL, NULL);

	    g->ccount = 0;

	    nmatches += g->mcount;
	    smatches += g->lcount;

	    if (g->mcount > 1)
		diffmatches = 1;

	    n = (Cmgroup) zshcalloc(sizeof(struct cmgroup));
#ifdef ZSH_HEAP_DEBUG
	    n->heap_id = HEAPID_PERMANENT;
#endif

	    if (g->perm) {
		g->perm->next = NULL;
		freematches(g->perm, 0);
	    }
	    g->perm = n;

	    if (!lmatches)
		lmatches = n;
	    if (pmatches)
		pmatches->prev = n;
	    n->next = pmatches;
	    pmatches = n;
	    n->prev = NULL;
	    n->num = gn++;
	    n->flags = g->flags;
	    n->mcount = g->mcount;
	    n->matches = p = (Cmatch *) zshcalloc((n->mcount + 1) * sizeof(Cmatch));
	    n->name = ztrdup(g->name);
	    for (q = g->matches; *q; q++, p++)
		*p = dupmatch(*q, nbrbeg, nbrend);
	    *p = NULL;

	    n->lcount = g->lcount;
	    n->llcount = g->llcount;
	    if (g->ylist)
		n->ylist = zarrdup(g->ylist);
	    else
		n->ylist = NULL;

	    if ((n->ecount = g->ecount)) {
		n->expls = ep = (Cexpl *) zshcalloc((n->ecount + 1) * sizeof(Cexpl));
		for (eq = g->expls; (o = *eq); eq++, ep++) {
		    *ep = e = (Cexpl) zshcalloc(sizeof(struct cexpl));
		    e->count = (fi ? o->fcount : o->count);
                    e->always = o->always;
		    e->fcount = 0;
		    e->str = ztrdup(o->str);
		}
		*ep = NULL;
	    } else
		n->expls = NULL;

	    n->widths = NULL;
	} else {
	    if (!lmatches)
		lmatches = g->perm;
	    if (pmatches)
		pmatches->prev = g->perm;
	    g->perm->next = pmatches;
	    pmatches = g->perm;
	    g->perm->prev = NULL;

	    nmatches += g->mcount;
	    smatches += g->lcount;

	    if (g->mcount > 1)
		diffmatches = 1;

	    g->num = gn++;
	}
	g->new = 0;
	g = g->next;
    }
    for (g = pmatches, p = NULL; g; g = g->next) {
	g->nbrbeg = nbrbeg;
	g->nbrend = nbrend;
	for (rn = 1, q = g->matches; *q; q++) {
	    (*q)->rnum = rn++;
	    (*q)->gnum = mn++;
	}
	if (!diffmatches && *g->matches) {
	    if (p) {
		if (!matcheq(*g->matches, *p))
		    diffmatches = 1;
	    } else
		p = g->matches;
	}
    }
    hasperm = 1;
    permmnum = mn - 1;
    permgnum = gn - 1;
    listdat.valid = 0;

    return fi;
}

/* This frees one match. */

/**/
static void
freematch(Cmatch m, int nbeg, int nend)
{
    if (!m) return;

    zsfree(m->str);
    zsfree(m->orig);
    zsfree(m->ipre);
    zsfree(m->ripre);
    zsfree(m->isuf);
    zsfree(m->ppre);
    zsfree(m->psuf);
    zsfree(m->pre);
    zsfree(m->suf);
    zsfree(m->prpre);
    zsfree(m->rems);
    zsfree(m->remf);
    zsfree(m->disp);
    zsfree(m->autoq);
    if (m->brpl)
	zfree(m->brpl, nbeg * sizeof(int));
    if (m->brsl)
	zfree(m->brsl, nend * sizeof(int));

    zfree(m, sizeof(*m));
}

/* This frees the groups of matches. */

/**/
mod_export void
freematches(Cmgroup g, int cm)
{
    Cmgroup n;
    Cmatch *m;
    Cexpl *e;

    while (g) {
	n = g->next;

	for (m = g->matches; *m; m++)
	    freematch(*m, g->nbrbeg, g->nbrend);
	free(g->matches);

	if (g->ylist)
	    freearray(g->ylist);

	if ((e = g->expls)) {
	    while (*e) {
		zsfree((*e)->str);
		free(*e);
		e++;
	    }
	    free(g->expls);
	}
	if (g->widths)
	    free(g->widths);
	zsfree(g->name);
	free(g);

	g = n;
    }
    if (cm)
	minfo.cur = NULL;
}
