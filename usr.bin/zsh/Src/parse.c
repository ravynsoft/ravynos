/*
 * parse.c - parser
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
#include "parse.pro"

/* != 0 if we are about to read a command word */

/**/
mod_export int incmdpos;

/**/
int aliasspaceflag;

/* != 0 if we are in the middle of a [[ ... ]] */
 
/**/
mod_export int incond;
 
/* != 0 if we are after a redirection (for ctxtlex only) */
 
/**/
mod_export int inredir;
 
/*
 * 1 if we are about to read a case pattern
 * -1 if we are not quite sure
 * 0 otherwise
 */
 
/**/
int incasepat;
 
/* != 0 if we just read a newline */
 
/**/
int isnewlin;

/* != 0 if we are after a for keyword */

/**/
int infor;

/* != 0 if we are after a repeat keyword; if it's nonzero it's a 1-based index
 * of the current token from the last-seen command position */

/**/
int inrepeat_; /* trailing underscore because of name clash with Zle/zle_vi.c */

/* != 0 if parsing arguments of typeset etc. */

/**/
mod_export int intypeset;

/* list of here-documents */

/**/
struct heredocs *hdocs;
 

#define YYERROR(O)  { tok = LEXERR; ecused = (O); return 0; }
#define YYERRORV(O) { tok = LEXERR; ecused = (O); return; }
#define COND_ERROR(X,Y) \
    do {					\
	zwarn(X,Y);				\
	herrflush();				\
	if (noerrs != 2)			\
	    errflag |= ERRFLAG_ERROR;		\
	YYERROR(ecused)				\
	    } while(0)


/* 
 * Word code.
 *
 * The parser now produces word code, reducing memory consumption compared
 * to the nested structs we had before.
 *
 * Word codes are represented by the "wordcode" type.
 *
 * Each wordcode variable consists of a "code", in the least-significant bits
 * of the value, and "data" in the other bits.  The macros wc_code() and wc_data()
 * access the "code" and "data" parts of a wordcode.  The macros wc_bdata() and
 * wc_bld() build wordcodes from code and data.
 *
 * Word code layout:
 *
 *   WC_END
 *     - end of program code
 *
 *   WC_LIST
 *     - data contains type (sync, ...)
 *     - followed by code for this list
 *     - if not (type & Z_END), followed by next WC_LIST
 *
 *   WC_SUBLIST
 *     - data contains type (&&, ||, END) and flags (coprog, not)
 *     - followed by code for sublist
 *     - if not (type == END), followed by next WC_SUBLIST
 *
 *   WC_PIPE
 *     - data contains type (end, mid) and LINENO
 *     - if not (type == END), followed by offset to next WC_PIPE
 *     - followed by command
 *     - if not (type == END), followed by next WC_PIPE
 *
 *   WC_REDIR
 *     - must precede command-code (or WC_ASSIGN)
 *     - data contains type (<, >, ...)
 *     - followed by fd1 and name from struct redir
 *     - for the extended form {var}>... where the fd is assigned
 *       to var, there is an extra item to contain var
 *
 *   WC_ASSIGN
 *     - data contains type (scalar, array) and number of array-elements
 *     - followed by name and value
 *     Note variant for WC_TYPESET assignments: WC_ASSIGN_INC indicates
 *     a name with no equals, not an =+ which isn't valid here.
 *
 *   WC_SIMPLE
 *     - data contains the number of arguments (plus command)
 *     - followed by strings
 *
 *   WC_TYPESET
 *     Variant of WC_SIMPLE used when TYPESET reserved word found.
 *     - data contains the number of string arguments (plus command)
 *     - followed by strings
 *     - followed by number of assignments
 *     - followed by assignments if non-zero number.
 *
 *   WC_SUBSH
 *     - data unused
 *     - followed by list
 *
 *   WC_CURSH
 *     - data unused
 *     - followed by list
 *
 *   WC_TIMED
 *     - data contains type (followed by pipe or not)
 *     - if (type == PIPE), followed by pipe
 *
 *   WC_FUNCDEF
 *     - data contains offset to after body
 *     - followed by number of names
 *     - followed by names
 *     - followed by offset to first string
 *     - followed by length of string table
 *     - followed by number of patterns for body
 *     - followed by an integer indicating tracing status
 *     - followed by codes for body
 *     - followed by strings for body
 *     - if number of names is 0, followed by:
 *       - the offset to the end of the funcdef
 *       - the number of arguments to the function
 *       - the arguments to the function
 *
 *   WC_FOR
 *     - data contains type (list, ...) and offset to after body
 *     - if (type == COND), followed by init, cond, advance expressions
 *     - else if (type == PPARAM), followed by param name
 *     - else if (type == LIST), followed by param name, num strings, strings
 *     - followed by body
 *
 *   WC_SELECT
 *     - data contains type (list, ...) and offset to after body
 *     - if (type == PPARAM), followed by param name
 *     - else if (type == LIST), followed by param name, num strings, strings
 *     - followed by body
 *
 *   WC_WHILE
 *     - data contains type (while, until) and offset to after body
 *     - followed by condition
 *     - followed by body
 *
 *   WC_REPEAT
 *     - data contains offset to after body
 *     - followed by number-string
 *     - followed by body
 *
 *   WC_CASE
 *     - first CASE is always of type HEAD, data contains offset to esac
 *     - after that CASEs of type OR (;;), AND (;&) and TESTAND (;|),
 *       data is offset to next case
 *     - each OR/AND/TESTAND case is followed by pattern, pattern-number, list
 *
 *   WC_IF
 *     - first IF is of type HEAD, data contains offset to fi
 *     - after that IFs of type IF, ELIF, ELSE, data is offset to next
 *     - each non-HEAD is followed by condition (only IF, ELIF) and body
 *
 *   WC_COND
 *     - data contains type
 *     - if (type == AND/OR), data contains offset to after this one,
 *       followed by two CONDs
 *     - else if (type == NOT), followed by COND
 *     - else if (type == MOD), followed by name and strings
 *     - else if (type == MODI), followed by name, left, right
 *     - else if (type == STR[N]EQ), followed by left, right, pattern-number
 *     - else if (has two args) followed by left, right
 *     - else followed by string
 *
 *   WC_ARITH
 *     - followed by string (there's only one)
 *
 *   WC_AUTOFN
 *     - only used by the autoload builtin
 *
 * Lists and sublists may also be simplified, indicated by the presence
 * of the Z_SIMPLE or WC_SUBLIST_SIMPLE flags. In this case they are only
 * followed by a slot containing the line number, not by a WC_SUBLIST or
 * WC_PIPE, respectively. The real advantage of simplified lists and
 * sublists is that they can be executed faster, see exec.c. In the
 * parser, the test if a list can be simplified is done quite simply
 * by passing a int* around which gets set to non-zero if the thing
 * just parsed is `cmplx', i.e. may need to be run by forking or 
 * some such.
 *
 * In each of the above, strings are encoded as one word code. For empty
 * strings this is the bit pattern 11x, the lowest bit is non-zero if the
 * string contains tokens and zero otherwise (this is true for the other
 * ways to encode strings, too). For short strings (one to three
 * characters), this is the marker 01x with the 24 bits above that
 * containing the characters. Longer strings are encoded as the offset
 * into the strs character array stored in the eprog struct shifted by
 * two and ored with the bit pattern 0x.
 * The ecstrcode() function that adds the code for a string uses a simple
 * binary tree of strings already added so that long strings are encoded
 * only once.
 *
 * Note also that in the eprog struct the pattern, code, and string
 * arrays all point to the same memory block.
 *
 *
 * To make things even faster in future versions, we could not only 
 * test if the strings contain tokens, but instead what kind of
 * expansions need to be done on strings. In the execution code we
 * could then use these flags for a specialized version of prefork()
 * to avoid a lot of string parsing and some more string duplication.
 */

/* Number of wordcodes allocated. */
static int eclen;
/* Number of wordcodes populated. */
static int ecused;
/* Number of patterns... */
static int ecnpats;

static Wordcode ecbuf;

static Eccstr ecstrs;

static int ecsoffs, ecssub;

/*
 * ### The number of starts and ends of function definitions up to this point.
 * Never decremented.
 */
static int ecnfunc;

#define EC_INIT_SIZE         256
#define EC_DOUBLE_THRESHOLD  32768
#define EC_INCREMENT         1024

/* save parse context */

/**/
void
parse_context_save(struct parse_stack *ps, int toplevel)
{
    (void)toplevel;

    ps->incmdpos = incmdpos;
    ps->aliasspaceflag = aliasspaceflag;
    ps->incond = incond;
    ps->inredir = inredir;
    ps->incasepat = incasepat;
    ps->isnewlin = isnewlin;
    ps->infor = infor;
    ps->inrepeat_ = inrepeat_;
    ps->intypeset = intypeset;

    ps->hdocs = hdocs;
    ps->eclen = eclen;
    ps->ecused = ecused;
    ps->ecnpats = ecnpats;
    ps->ecbuf = ecbuf;
    ps->ecstrs = ecstrs;
    ps->ecsoffs = ecsoffs;
    ps->ecssub = ecssub;
    ps->ecnfunc = ecnfunc;
    ecbuf = NULL;
    hdocs = NULL;
}

/* restore parse context */

/**/
void
parse_context_restore(const struct parse_stack *ps, int toplevel)
{
    (void)toplevel;

    if (ecbuf)
	zfree(ecbuf, eclen);

    incmdpos = ps->incmdpos;
    aliasspaceflag = ps->aliasspaceflag;
    incond = ps->incond;
    inredir = ps->inredir;
    incasepat = ps->incasepat;
    isnewlin = ps->isnewlin;
    infor = ps->infor;
    inrepeat_ = ps->inrepeat_;
    intypeset = ps->intypeset;

    hdocs = ps->hdocs;
    eclen = ps->eclen;
    ecused = ps->ecused;
    ecnpats = ps->ecnpats;
    ecbuf = ps->ecbuf;
    ecstrs = ps->ecstrs;
    ecsoffs = ps->ecsoffs;
    ecssub = ps->ecssub;
    ecnfunc = ps->ecnfunc;

    errflag &= ~ERRFLAG_ERROR;
}

/* Adjust pointers in here-doc structs. */

static void
ecadjusthere(int p, int d)
{
    struct heredocs *h;

    for (h = hdocs; h; h = h->next)
	if (h->pc >= p)
	    h->pc += d;
}

/* Insert n free code-slots at position p. */

static void
ecispace(int p, int n)
{
    int m;

    if ((eclen - ecused) < n) {
	int a = (eclen < EC_DOUBLE_THRESHOLD ? eclen : EC_INCREMENT);

	if (n > a) a = n;

	ecbuf = (Wordcode) zrealloc((char *) ecbuf, (eclen + a) * sizeof(wordcode));
	eclen += a;
    }
    if ((m = ecused - p) > 0)
	memmove(ecbuf + p + n, ecbuf + p, m * sizeof(wordcode));
    ecused += n;
    ecadjusthere(p, n);
}

/* 
 * Add one wordcode.
 *
 * Return the index of the added wordcode.
 */

static int
ecadd(wordcode c)
{
    if ((eclen - ecused) < 1) {
	int a = (eclen < EC_DOUBLE_THRESHOLD ? eclen : EC_INCREMENT);

	ecbuf = (Wordcode) zrealloc((char *) ecbuf, (eclen + a) * sizeof(wordcode));
	eclen += a;
    }
    ecbuf[ecused] = c;

    return ecused++;
}

/* Delete a wordcode. */

static void
ecdel(int p)
{
    int n = ecused - p - 1;

    if (n > 0)
	memmove(ecbuf + p, ecbuf + p + 1, n * sizeof(wordcode));
    ecused--;
    ecadjusthere(p, -1);
}

/* Build the wordcode for a string. */

static wordcode
ecstrcode(char *s)
{
    int l, t;

    unsigned val = hasher(s);

    if ((l = strlen(s) + 1) && l <= 4) {
	/* Short string. */
	t = has_token(s);
	wordcode c = (t ? 3 : 2);
	switch (l) {
	case 4: c |= ((wordcode) STOUC(s[2])) << 19;
	case 3: c |= ((wordcode) STOUC(s[1])) << 11;
	case 2: c |= ((wordcode) STOUC(s[0])) <<  3; break;
	case 1: c = (t ? 7 : 6); break;
	}
	return c;
    } else {
	/* Long string. */
	Eccstr p, *pp;
	long cmp;

	for (pp = &ecstrs; (p = *pp); ) {
	    if (!(cmp = p->nfunc - ecnfunc) && !(cmp = (((long)p->hashval) - ((long)val))) && !(cmp = strcmp(p->str, s))) {
		/* Re-use the existing string. */
		return p->offs;
            }
	    pp = (cmp < 0 ? &(p->left) : &(p->right));
	}

        t = has_token(s);

	p = *pp = (Eccstr) zhalloc(sizeof(*p));
	p->left = p->right = 0;
	p->offs = ((ecsoffs - ecssub) << 2) | (t ? 1 : 0);
	p->aoffs = ecsoffs;
	p->str = s;
	p->nfunc = ecnfunc;
        p->hashval = val;
	ecsoffs += l;

	return p->offs;
    }
}

#define ecstr(S) ecadd(ecstrcode(S))

#define par_save_list(C) \
    do { \
        int eu = ecused; \
        par_list(C); \
        if (eu == ecused) ecadd(WCB_END()); \
    } while (0)
#define par_save_list1(C) \
    do { \
        int eu = ecused; \
        par_list1(C); \
        if (eu == ecused) ecadd(WCB_END()); \
    } while (0)


/**/
mod_export void
init_parse_status(void)
{
    /*
     * These variables are currently declared by the parser, so we
     * initialise them here.  Possibly they are more naturally declared
     * by the lexical anaylser; however, as they are used for signalling
     * between the two it's a bit ambiguous.  We clear them when
     * using the lexical analyser for strings as well as here.
     */
    incasepat = incond = inredir = infor = intypeset = 0;
    inrepeat_ = 0;
    incmdpos = 1;
}

/* Initialise wordcode buffer. */

/**/
void
init_parse(void)
{
    queue_signals();

    if (ecbuf) zfree(ecbuf, eclen);

    ecbuf = (Wordcode) zalloc((eclen = EC_INIT_SIZE) * sizeof(wordcode));
    ecused = 0;
    ecstrs = NULL;
    ecsoffs = ecnpats = 0;
    ecssub = 0;
    ecnfunc = 0;

    init_parse_status();

    unqueue_signals();
}

/* Build eprog. */

/*
 * Copy the strings of s and all its descendants in the binary tree to the
 * memory block p.
 *
 * careful: copy_ecstr is from arg1 to arg2, unlike memcpy
 */

static void
copy_ecstr(Eccstr s, char *p)
{
    while (s) {
	memcpy(p + s->aoffs, s->str, strlen(s->str) + 1);
	copy_ecstr(s->left, p);
	s = s->right;
    }
}

static Eprog
bld_eprog(int heap)
{
    Eprog ret;
    int l;

    queue_signals();

    ecadd(WCB_END());

    ret = heap ? (Eprog) zhalloc(sizeof(*ret)) : (Eprog) zalloc(sizeof(*ret));
    ret->len = ((ecnpats * sizeof(Patprog)) +
		(ecused * sizeof(wordcode)) +
		ecsoffs);
    ret->npats = ecnpats;
    ret->nref = heap ? -1 : 1;
    ret->pats = heap ? (Patprog *) zhalloc(ret->len) :
	(Patprog *) zshcalloc(ret->len);
    ret->prog = (Wordcode) (ret->pats + ecnpats);
    ret->strs = (char *) (ret->prog + ecused);
    ret->shf = NULL;
    ret->flags = heap ? EF_HEAP : EF_REAL;
    ret->dump = NULL;
    for (l = 0; l < ecnpats; l++)
	ret->pats[l] = dummy_patprog1;
    memcpy(ret->prog, ecbuf, ecused * sizeof(wordcode));
    copy_ecstr(ecstrs, ret->strs);

    zfree(ecbuf, eclen);
    ecbuf = NULL;

    unqueue_signals();

    return ret;
}

/**/
mod_export int
empty_eprog(Eprog p)
{
    return (!p || !p->prog || *p->prog == WCB_END());
}

static void
clear_hdocs(void)
{
    struct heredocs *p, *n;

    for (p = hdocs; p; p = n) {
        n = p->next;
        zfree(p, sizeof(struct heredocs));
    }
    hdocs = NULL;
}

/*
 * event	: ENDINPUT
 *			| SEPER
 *			| sublist [ SEPER | AMPER | AMPERBANG ]
 *
 * cmdsubst indicates our event is part of a command-style
 * substitution terminated by the token indicationg, usual closing
 * parenthesis.  In other cases endtok is ENDINPUT.
 */

/**/
Eprog
parse_event(int endtok)
{
    tok = ENDINPUT;
    incmdpos = 1;
    aliasspaceflag = 0;
    zshlex();
    init_parse();

    if (!par_event(endtok)) {
        clear_hdocs();
        return NULL;
    }
    if (endtok != ENDINPUT) {
	/* don't need to build an eprog for this */
	return &dummy_eprog;
    }
    return bld_eprog(1);
}

/**/
int
par_event(int endtok)
{
    int r = 0, p, c = 0;

    while (tok == SEPER) {
	if (isnewlin > 0 && endtok == ENDINPUT)
	    return 0;
	zshlex();
    }
    if (tok == ENDINPUT)
	return 0;
    if (tok == endtok)
	return 1;

    p = ecadd(0);

    if (par_sublist(&c)) {
	if (tok == ENDINPUT || tok == endtok) {
	    set_list_code(p, Z_SYNC, c);
	    r = 1;
	} else if (tok == SEPER) {
	    set_list_code(p, Z_SYNC, c);
	    if (isnewlin <= 0 || endtok != ENDINPUT)
		zshlex();
	    r = 1;
	} else if (tok == AMPER) {
	    set_list_code(p, Z_ASYNC, c);
	    zshlex();
	    r = 1;
	} else if (tok == AMPERBANG) {
	    set_list_code(p, (Z_ASYNC | Z_DISOWN), c);
	    zshlex();
	    r = 1;
	}
    }
    if (!r) {
	tok = LEXERR;
	if (errflag) {
	    yyerror(0);
	    ecused--;
	    return 0;
	}
	yyerror(1);
	herrflush();
	if (noerrs != 2)
	    errflag |= ERRFLAG_ERROR;
	ecused--;
	return 0;
    } else {
	int oec = ecused;

	if (!par_event(endtok)) {
	    ecused = oec;
	    ecbuf[p] |= wc_bdata(Z_END);
	    return errflag ? 0 : 1;
	}
    }
    return 1;
}

/**/
mod_export Eprog
parse_list(void)
{
    int c = 0;

    tok = ENDINPUT;
    init_parse();
    zshlex();
    par_list(&c);
    if (tok != ENDINPUT) {
        clear_hdocs();
	tok = LEXERR;
	yyerror(0);
	return NULL;
    }
    return bld_eprog(1);
}

/*
 * This entry point is only used for bin_test, our attempt to
 * provide compatibility with /bin/[ and /bin/test.  Hence
 * at this point condlex should always be set to testlex.
 */

/**/
mod_export Eprog
parse_cond(void)
{
    init_parse();

    if (!par_cond()) {
        clear_hdocs();
	return NULL;
    }
    return bld_eprog(1);
}

/* This adds a list wordcode. The important bit about this is that it also
 * tries to optimise this to a Z_SIMPLE list code. */

/**/
static void
set_list_code(int p, int type, int cmplx)
{
    if (!cmplx && (type == Z_SYNC || type == (Z_SYNC | Z_END)) &&
	WC_SUBLIST_TYPE(ecbuf[p + 1]) == WC_SUBLIST_END) {
	int ispipe = !(WC_SUBLIST_FLAGS(ecbuf[p + 1]) & WC_SUBLIST_SIMPLE);
	ecbuf[p] = WCB_LIST((type | Z_SIMPLE), ecused - 2 - p);
	ecdel(p + 1);
	if (ispipe)
	    ecbuf[p + 1] = WC_PIPE_LINENO(ecbuf[p + 1]);
    } else
	ecbuf[p] = WCB_LIST(type, 0);
}

/* The same for sublists. */

/**/
static void
set_sublist_code(int p, int type, int flags, int skip, int cmplx)
{
    if (cmplx)
	ecbuf[p] = WCB_SUBLIST(type, flags, skip);
    else {
	ecbuf[p] = WCB_SUBLIST(type, (flags | WC_SUBLIST_SIMPLE), skip);
	ecbuf[p + 1] = WC_PIPE_LINENO(ecbuf[p + 1]);
    }
}

/*
 * list	: { SEPER } [ sublist [ { SEPER | AMPER | AMPERBANG } list ] ]
 */

/**/
static void
par_list(int *cmplx)
{
    int p, lp = -1, c;

 rec:

    while (tok == SEPER)
	zshlex();

    p = ecadd(0);
    c = 0;

    if (par_sublist(&c)) {
	*cmplx |= c;
	if (tok == SEPER || tok == AMPER || tok == AMPERBANG) {
	    if (tok != SEPER)
		*cmplx = 1;
	    set_list_code(p, ((tok == SEPER) ? Z_SYNC :
			      (tok == AMPER) ? Z_ASYNC :
			      (Z_ASYNC | Z_DISOWN)), c);
	    incmdpos = 1;
	    do {
		zshlex();
	    } while (tok == SEPER);
	    lp = p;
	    goto rec;
	} else
	    set_list_code(p, (Z_SYNC | Z_END), c);
    } else {
	ecused--;
	if (lp >= 0)
	    ecbuf[lp] |= wc_bdata(Z_END);
    }
}

/**/
static void
par_list1(int *cmplx)
{
    int p = ecadd(0), c = 0;

    if (par_sublist(&c)) {
	set_list_code(p, (Z_SYNC | Z_END), c);
	*cmplx |= c;
    } else
	ecused--;
}

/*
 * sublist	: sublist2 [ ( DBAR | DAMPER ) { SEPER } sublist ]
 */

/**/
static int
par_sublist(int *cmplx)
{
    int f, p, c = 0;

    p = ecadd(0);

    if ((f = par_sublist2(&c)) != -1) {
	int e = ecused;

	*cmplx |= c;
	if (tok == DBAR || tok == DAMPER) {
	    enum lextok qtok = tok;
	    int sl;

	    cmdpush(tok == DBAR ? CS_CMDOR : CS_CMDAND);
	    zshlex();
	    while (tok == SEPER)
		zshlex();
	    sl = par_sublist(cmplx);
	    set_sublist_code(p, (sl ? (qtok == DBAR ?
				       WC_SUBLIST_OR : WC_SUBLIST_AND) :
				 WC_SUBLIST_END),
			     f, (e - 1 - p), c);
	    cmdpop();
	} else {
	    if (tok == AMPER || tok == AMPERBANG) {
		c = 1;
		*cmplx |= c;
	    }		
	    set_sublist_code(p, WC_SUBLIST_END, f, (e - 1 - p), c);
	}
	return 1;
    } else {
	ecused--;
	return 0;
    }
}

/*
 * sublist2	: [ COPROC | BANG ] pline
 */

/**/
static int
par_sublist2(int *cmplx)
{
    int f = 0;

    if (tok == COPROC) {
	*cmplx = 1;
	f |= WC_SUBLIST_COPROC;
	zshlex();
    } else if (tok == BANG) {
	*cmplx = 1;
	f |= WC_SUBLIST_NOT;
	zshlex();
    }
    if (!par_pline(cmplx) && !f)
	return -1;

    return f;
}

/*
 * pline	: cmd [ ( BAR | BARAMP ) { SEPER } pline ]
 */

/**/
static int
par_pline(int *cmplx)
{
    int p;
    zlong line = toklineno;

    p = ecadd(0);

    if (!par_cmd(cmplx, 0)) {
	ecused--;
	return 0;
    }
    if (tok == BAR) {
	*cmplx = 1;
	cmdpush(CS_PIPE);
	zshlex();
	while (tok == SEPER)
	    zshlex();
	ecbuf[p] = WCB_PIPE(WC_PIPE_MID, (line >= 0 ? line + 1 : 0));
	ecispace(p + 1, 1);
	ecbuf[p + 1] = ecused - 1 - p;
	if (!par_pline(cmplx)) {
	    tok = LEXERR;
	}
	cmdpop();
	return 1;
    } else if (tok == BARAMP) {
	int r;

	for (r = p + 1; wc_code(ecbuf[r]) == WC_REDIR;
	     r += WC_REDIR_WORDS(ecbuf[r]));

	ecispace(r, 3);
	ecbuf[r] = WCB_REDIR(REDIR_MERGEOUT);
	ecbuf[r + 1] = 2;
	ecbuf[r + 2] = ecstrcode("1");

	*cmplx = 1;
	cmdpush(CS_ERRPIPE);
	zshlex();
	while (tok == SEPER)
	    zshlex();
	ecbuf[p] = WCB_PIPE(WC_PIPE_MID, (line >= 0 ? line + 1 : 0));
	ecispace(p + 1, 1);
	ecbuf[p + 1] = ecused - 1 - p;
	if (!par_pline(cmplx)) {
	    tok = LEXERR;
	}
	cmdpop();
	return 1;
    } else {
	ecbuf[p] = WCB_PIPE(WC_PIPE_END, (line >= 0 ? line + 1 : 0));
	return 1;
    }
}

/*
 * cmd	: { redir } ( for | case | if | while | repeat |
 *				subsh | funcdef | time | dinbrack | dinpar | simple ) { redir }
 *
 * zsh_construct is passed through to par_subsh(), q.v.
 */

/**/
static int
par_cmd(int *cmplx, int zsh_construct)
{
    int r, nr = 0;

    r = ecused;

    if (IS_REDIROP(tok)) {
	*cmplx = 1;
	while (IS_REDIROP(tok)) {
	    nr += par_redir(&r, NULL);
	}
    }
    switch (tok) {
    case FOR:
	cmdpush(CS_FOR);
	par_for(cmplx);
	cmdpop();
	break;
    case FOREACH:
	cmdpush(CS_FOREACH);
	par_for(cmplx);
	cmdpop();
	break;
    case SELECT:
	*cmplx = 1;
	cmdpush(CS_SELECT);
	par_for(cmplx);
	cmdpop();
	break;
    case CASE:
	cmdpush(CS_CASE);
	par_case(cmplx);
	cmdpop();
	break;
    case IF:
	par_if(cmplx);
	break;
    case WHILE:
	cmdpush(CS_WHILE);
	par_while(cmplx);
	cmdpop();
	break;
    case UNTIL:
	cmdpush(CS_UNTIL);
	par_while(cmplx);
	cmdpop();
	break;
    case REPEAT:
	cmdpush(CS_REPEAT);
	par_repeat(cmplx);
	cmdpop();
	break;
    case INPAR:
	*cmplx = 1;
	cmdpush(CS_SUBSH);
	par_subsh(cmplx, zsh_construct);
	cmdpop();
	break;
    case INBRACE:
	cmdpush(CS_CURSH);
	par_subsh(cmplx, zsh_construct);
	cmdpop();
	break;
    case FUNC:
	cmdpush(CS_FUNCDEF);
	par_funcdef(cmplx);
	cmdpop();
	break;
    case DINBRACK:
	cmdpush(CS_COND);
	par_dinbrack();
	cmdpop();
	break;
    case DINPAR:
	ecadd(WCB_ARITH());
	ecstr(tokstr);
	zshlex();
	break;
    case TIME:
	{
	    static int inpartime = 0;

	    if (!inpartime) {
		*cmplx = 1;
		inpartime = 1;
		par_time();
		inpartime = 0;
		break;
	    }
	}
	tok = STRING;
	/* fall through */
    default:
	{
	    int sr;

	    if (!(sr = par_simple(cmplx, nr))) {
		if (!nr)
		    return 0;
	    } else {
		/* Take account of redirections */
		if (sr > 1) {
		    *cmplx = 1;
		    r += sr - 1;
		}
	    }
	}
	break;
    }
    if (IS_REDIROP(tok)) {
	*cmplx = 1;
	while (IS_REDIROP(tok))
	    (void)par_redir(&r, NULL);
    }
    incmdpos = 1;
    incasepat = 0;
    incond = 0;
    intypeset = 0;
    return 1;
}

/*
 * for  : ( FOR DINPAR expr SEMI expr SEMI expr DOUTPAR |
 *    ( FOR[EACH] | SELECT ) name ( "in" wordlist | INPAR wordlist OUTPAR ) )
 *	{ SEPER } ( DO list DONE | INBRACE list OUTBRACE | list ZEND | list1 )
 */

/**/
static void
par_for(int *cmplx)
{
    int oecused = ecused, csh = (tok == FOREACH), p, sel = (tok == SELECT);
    int type;

    p = ecadd(0);

    incmdpos = 0;
    infor = tok == FOR ? 2 : 0;
    zshlex();
    if (tok == DINPAR) {
	zshlex();
	if (tok != DINPAR)
	    YYERRORV(oecused);
	ecstr(tokstr);
	zshlex();
	if (tok != DINPAR)
	    YYERRORV(oecused);
	ecstr(tokstr);
	zshlex();
	if (tok != DOUTPAR)
	    YYERRORV(oecused);
	ecstr(tokstr);
	infor = 0;
	incmdpos = 1;
	zshlex();
	type = WC_FOR_COND;
    } else {
	int np = 0, n, posix_in, ona = noaliases, onc = nocorrect;
	infor = 0;
	if (tok != STRING || !isident(tokstr))
	    YYERRORV(oecused);
	if (!sel)
	    np = ecadd(0);
	n = 0;
	incmdpos = 1;
	noaliases = nocorrect = 1;
	for (;;) {
	    n++;
	    ecstr(tokstr);
	    zshlex();
	    if (tok != STRING || !strcmp(tokstr, "in") || sel)
		break;
	    if (!isident(tokstr) || errflag)
	    {
		noaliases = ona;
		nocorrect = onc;
		YYERRORV(oecused);
	    }
	}
	noaliases = ona;
	nocorrect = onc;
	if (!sel)
	    ecbuf[np] = n;
	posix_in = isnewlin;
	while (isnewlin)
	    zshlex();
        if (tok == STRING && !strcmp(tokstr, "in")) {
	    incmdpos = 0;
	    zshlex();
	    np = ecadd(0);
	    n = par_wordlist();
	    if (tok != SEPER)
		YYERRORV(oecused);
	    ecbuf[np] = n;
	    type = (sel ? WC_SELECT_LIST : WC_FOR_LIST);
	} else if (!posix_in && tok == INPAR) {
	    incmdpos = 0;
	    zshlex();
	    np = ecadd(0);
	    n = par_nl_wordlist();
	    if (tok != OUTPAR)
		YYERRORV(oecused);
	    ecbuf[np] = n;
	    incmdpos = 1;
	    zshlex();
	    type = (sel ? WC_SELECT_LIST : WC_FOR_LIST);
	} else
	    type = (sel ? WC_SELECT_PPARAM : WC_FOR_PPARAM);
    }
    incmdpos = 1;
    while (tok == SEPER)
	zshlex();
    if (tok == DOLOOP) {
	zshlex();
	par_save_list(cmplx);
	if (tok != DONE)
	    YYERRORV(oecused);
	incmdpos = 0;
	zshlex();
    } else if (tok == INBRACE) {
	zshlex();
	par_save_list(cmplx);
	if (tok != OUTBRACE)
	    YYERRORV(oecused);
	incmdpos = 0;
	zshlex();
    } else if (csh || isset(CSHJUNKIELOOPS)) {
	par_save_list(cmplx);
	if (tok != ZEND)
	    YYERRORV(oecused);
	incmdpos = 0;
	zshlex();
    } else if (unset(SHORTLOOPS)) {
	YYERRORV(oecused);
    } else
	par_save_list1(cmplx);

    ecbuf[p] = (sel ?
		WCB_SELECT(type, ecused - 1 - p) :
		WCB_FOR(type, ecused - 1 - p));
}

/*
 * case	: CASE STRING { SEPER } ( "in" | INBRACE )
				{ { SEPER } STRING { BAR STRING } OUTPAR
					list [ DSEMI | SEMIAMP | SEMIBAR ] }
				{ SEPER } ( "esac" | OUTBRACE )
 */

/**/
static void
par_case(int *cmplx)
{
    int oecused = ecused, brflag, p, pp, palts, type, nalts;
    int ona, onc;

    p = ecadd(0);

    incmdpos = 0;
    zshlex();
    if (tok != STRING)
	YYERRORV(oecused);
    ecstr(tokstr);

    incmdpos = 1;
    ona = noaliases;
    onc = nocorrect;
    noaliases = nocorrect = 1;
    zshlex();
    while (tok == SEPER)
	zshlex();
    if (!(tok == STRING && !strcmp(tokstr, "in")) && tok != INBRACE)
    {
	noaliases = ona;
	nocorrect = onc;
	YYERRORV(oecused);
    }
    brflag = (tok == INBRACE);
    incasepat = 1;
    incmdpos = 0;
    noaliases = ona;
    nocorrect = onc;
    zshlex();

    for (;;) {
	char *str;
	int skip_zshlex;

	while (tok == SEPER)
	    zshlex();
	if (tok == OUTBRACE)
	    break;
	if (tok == INPAR)
	    zshlex();
	if (tok == BAR) {
	    str = dupstring("");
	    skip_zshlex = 1;
	} else {
	    if (tok != STRING)
		YYERRORV(oecused);
	    if (!strcmp(tokstr, "esac"))
		break;
	    str = dupstring(tokstr);
	    skip_zshlex = 0;
	}
	type = WC_CASE_OR;
	pp = ecadd(0);
	palts = ecadd(0);
	nalts = 0;
	/*
	 * Hack here.
	 *
	 * [Pause for astonished hubbub to subside.]
	 *
	 * The next token we get may be
	 * - ")" or "|" if we're looking at an honest-to-god
	 *   "case" pattern, either because there's no opening
	 *   parenthesis, or because SH_GLOB is set and we
	 *   managed to grab an initial "(" to mark the start
	 *   of the case pattern.
	 * - Something else --- we don't care what --- because
	 *   we're parsing a complete "(...)" as a complete
	 *   zsh pattern.  In that case, we treat this as a
	 *   single instance of a case pattern but we pretend
	 *   we're doing proper case parsing --- in which the
	 *   parentheses and bar are in different words from
	 *   the string, so may be separated by whitespace.
	 *   So we quietly massage the whitespace and hope
	 *   no one noticed.  This is horrible, but it's
	 *   unfortunately too difficult to combine traditional
	 *   zsh patterns with a properly parsed case pattern
	 *   without generating incompatibilities which aren't
	 *   all that popular (I've discovered).
	 * - We can also end up with something other than ")" or "|"
	 *   just because we're looking at garbage.
	 *
	 * Because of the second case, what happens next might
	 * be the start of the command after the pattern, so we
	 * need to treat it as in command position.  Luckily
	 * this doesn't affect our ability to match a | or ) as
	 * these are valid on command lines.
	 */
	incasepat = -1;
	incmdpos = 1;
	if (!skip_zshlex)
	    zshlex();
	for (;;) {
	    if (tok == OUTPAR) {
		ecstr(str);
		ecadd(ecnpats++);
		nalts++;

		incasepat = 0;
		incmdpos = 1;
		zshlex();
		break;
	    } else if (tok == BAR) {
		ecstr(str);
		ecadd(ecnpats++);
		nalts++;

		incasepat = 1;
		incmdpos = 0;
	    } else {
		if (!nalts && str[0] == Inpar) {
		    int pct = 0, sl;
		    char *s;

		    for (s = str; *s; s++) {
			if (*s == Inpar)
			    pct++;
			if (!pct)
			    break;
			if (pct == 1) {
			    if (*s == Bar || *s == Inpar)
				while (iblank(s[1]))
				    chuck(s+1);
			    if (*s == Bar || *s == Outpar)
				while (iblank(s[-1]) &&
				       (s < str + 1 || s[-2] != Meta))
				    chuck(--s);
			}
			if (*s == Outpar)
			    pct--;
		    }
		    if (*s || pct || s == str)
			YYERRORV(oecused);
		    /* Simplify pattern by removing surrounding (...) */
		    sl = strlen(str);
		    DPUTS(*str != Inpar || str[sl - 1] != Outpar,
			  "BUG: strange case pattern");
		    str[sl - 1] = '\0';
		    chuck(str);
		    ecstr(str);
		    ecadd(ecnpats++);
		    nalts++;
		    break;
		}
		YYERRORV(oecused);
	    }

	    zshlex();
	    switch (tok) {
	    case STRING:
		/* Normal case */
		str = dupstring(tokstr);
		zshlex();
		break;

	    case OUTPAR:
	    case BAR:
		/* Empty string */
		str = dupstring("");
		break;

	    default:
		/* Oops. */
		YYERRORV(oecused);
		break;
	    }
	}
	incasepat = 0;
	par_save_list(cmplx);
	if (tok == SEMIAMP)
	    type = WC_CASE_AND;
	else if (tok == SEMIBAR)
	    type = WC_CASE_TESTAND;
	ecbuf[pp] = WCB_CASE(type, ecused - 1 - pp);
	ecbuf[palts] = nalts;
	if ((tok == ESAC && !brflag) || (tok == OUTBRACE && brflag))
	    break;
	if (tok != DSEMI && tok != SEMIAMP && tok != SEMIBAR)
	    YYERRORV(oecused);
	incasepat = 1;
	incmdpos = 0;
	zshlex();
    }
    incmdpos = 1;
    incasepat = 0;
    zshlex();

    ecbuf[p] = WCB_CASE(WC_CASE_HEAD, ecused - 1 - p);
}

/*
 * if	: { ( IF | ELIF ) { SEPER } ( INPAR list OUTPAR | list )
			{ SEPER } ( THEN list | INBRACE list OUTBRACE | list1 ) }
			[ FI | ELSE list FI | ELSE { SEPER } INBRACE list OUTBRACE ]
			(you get the idea...?)
 */

/**/
static void
par_if(int *cmplx)
{
    int oecused = ecused, p, pp, type, usebrace = 0;
    enum lextok xtok;
    unsigned char nc;

    p = ecadd(0);

    for (;;) {
	xtok = tok;
	cmdpush(xtok == IF ? CS_IF : CS_ELIF);
	if (xtok == FI) {
	    incmdpos = 0;
	    zshlex();
	    break;
	}
	zshlex();
	if (xtok == ELSE)
	    break;
	while (tok == SEPER)
	    zshlex();
	if (!(xtok == IF || xtok == ELIF)) {
	    cmdpop();
	    YYERRORV(oecused);
	}
	pp = ecadd(0);
	type = (xtok == IF ? WC_IF_IF : WC_IF_ELIF);
	par_save_list(cmplx);
	incmdpos = 1;
	if (tok == ENDINPUT) {
	    cmdpop();
	    YYERRORV(oecused);
	}
	while (tok == SEPER)
	    zshlex();
	xtok = FI;
	nc = cmdstack[cmdsp - 1] == CS_IF ? CS_IFTHEN : CS_ELIFTHEN;
	if (tok == THEN) {
	    usebrace = 0;
	    cmdpop();
	    cmdpush(nc);
	    zshlex();
	    par_save_list(cmplx);
	    ecbuf[pp] = WCB_IF(type, ecused - 1 - pp);
	    incmdpos = 1;
	    cmdpop();
	} else if (tok == INBRACE) {
	    usebrace = 1;
	    cmdpop();
	    cmdpush(nc);
	    zshlex();
	    par_save_list(cmplx);
	    if (tok != OUTBRACE) {
		cmdpop();
		YYERRORV(oecused);
	    }
	    ecbuf[pp] = WCB_IF(type, ecused - 1 - pp);
	    /* command word (else) allowed to follow immediately */
	    zshlex();
	    incmdpos = 1;
	    if (tok == SEPER)
		break;
	    cmdpop();
	} else if (unset(SHORTLOOPS)) {
	    cmdpop();
	    YYERRORV(oecused);
	} else {
	    cmdpop();
	    cmdpush(nc);
	    par_save_list1(cmplx);
	    ecbuf[pp] = WCB_IF(type, ecused - 1 - pp);
	    incmdpos = 1;
	    break;
	}
    }
    cmdpop();
    if (xtok == ELSE || tok == ELSE) {
	pp = ecadd(0);
	cmdpush(CS_ELSE);
	while (tok == SEPER)
	    zshlex();
	if (tok == INBRACE && usebrace) {
	    zshlex();
	    par_save_list(cmplx);
	    if (tok != OUTBRACE) {
		cmdpop();
		YYERRORV(oecused);
	    }
	} else {
	    par_save_list(cmplx);
	    if (tok != FI) {
		cmdpop();
		YYERRORV(oecused);
	    }
	}
	incmdpos = 0;
	ecbuf[pp] = WCB_IF(WC_IF_ELSE, ecused - 1 - pp);
	zshlex();
	cmdpop();
    }
    ecbuf[p] = WCB_IF(WC_IF_HEAD, ecused - 1 - p);
}

/*
 * while	: ( WHILE | UNTIL ) ( INPAR list OUTPAR | list ) { SEPER }
				( DO list DONE | INBRACE list OUTBRACE | list ZEND )
 */

/**/
static void
par_while(int *cmplx)
{
    int oecused = ecused, p;
    int type = (tok == UNTIL ? WC_WHILE_UNTIL : WC_WHILE_WHILE);

    p = ecadd(0);
    zshlex();
    par_save_list(cmplx);
    incmdpos = 1;
    while (tok == SEPER)
	zshlex();
    if (tok == DOLOOP) {
	zshlex();
	par_save_list(cmplx);
	if (tok != DONE)
	    YYERRORV(oecused);
	incmdpos = 0;
	zshlex();
    } else if (tok == INBRACE) {
	zshlex();
	par_save_list(cmplx);
	if (tok != OUTBRACE)
	    YYERRORV(oecused);
	incmdpos = 0;
	zshlex();
    } else if (isset(CSHJUNKIELOOPS)) {
	par_save_list(cmplx);
	if (tok != ZEND)
	    YYERRORV(oecused);
	zshlex();
    } else if (unset(SHORTLOOPS)) {
	YYERRORV(oecused);
    } else
	par_save_list1(cmplx);

    ecbuf[p] = WCB_WHILE(type, ecused - 1 - p);
}

/*
 * repeat	: REPEAT STRING { SEPER } ( DO list DONE | list1 )
 */

/**/
static void
par_repeat(int *cmplx)
{
    /* ### what to do about inrepeat_ here? */
    int oecused = ecused, p;

    p = ecadd(0);

    incmdpos = 0;
    zshlex();
    if (tok != STRING)
	YYERRORV(oecused);
    ecstr(tokstr);
    incmdpos = 1;
    zshlex();
    while (tok == SEPER)
	zshlex();
    if (tok == DOLOOP) {
	zshlex();
	par_save_list(cmplx);
	if (tok != DONE)
	    YYERRORV(oecused);
	incmdpos = 0;
	zshlex();
    } else if (tok == INBRACE) {
	zshlex();
	par_save_list(cmplx);
	if (tok != OUTBRACE)
	    YYERRORV(oecused);
	incmdpos = 0;
	zshlex();
    } else if (isset(CSHJUNKIELOOPS)) {
	par_save_list(cmplx);
	if (tok != ZEND)
	    YYERRORV(oecused);
	zshlex();
    } else if (unset(SHORTLOOPS) && unset(SHORTREPEAT)) {
	YYERRORV(oecused);
    } else
	par_save_list1(cmplx);

    ecbuf[p] = WCB_REPEAT(ecused - 1 - p);
}

/*
 * subsh	: INPAR list OUTPAR |
 *                INBRACE list OUTBRACE [ "always" INBRACE list OUTBRACE ]
 *
 * With zsh_construct non-zero, we're doing a zsh special in which
 * the following token is not considered in command position.  This
 * is used for arguments of anonymous functions.
 */

/**/
static void
par_subsh(int *cmplx, int zsh_construct)
{
    enum lextok otok = tok;
    int oecused = ecused, p, pp;

    p = ecadd(0);
    /* Extra word only needed for always block */
    pp = ecadd(0);
    zshlex();
    par_list(cmplx);
    ecadd(WCB_END());
    if (tok != ((otok == INPAR) ? OUTPAR : OUTBRACE))
	YYERRORV(oecused);
    incmdpos = !zsh_construct;
    zshlex();

    /* Optional always block.  No intervening SEPERs allowed. */
    if (otok == INBRACE && tok == STRING && !strcmp(tokstr, "always")) {
	ecbuf[pp] = WCB_TRY(ecused - 1 - pp);
	incmdpos = 1;
	do {
	    zshlex();
	} while (tok == SEPER);

	if (tok != INBRACE)
	    YYERRORV(oecused);
	cmdpop();
	cmdpush(CS_ALWAYS);

	zshlex();
	par_save_list(cmplx);
	while (tok == SEPER)
	    zshlex();

	incmdpos = 1;

	if (tok != OUTBRACE)
	    YYERRORV(oecused);
	zshlex();
	ecbuf[p] = WCB_TRY(ecused - 1 - p);
    } else {
	ecbuf[p] = (otok == INPAR ? WCB_SUBSH(ecused - 1 - p) :
		    WCB_CURSH(ecused - 1 - p));
    }
}

/*
 * funcdef	: FUNCTION wordlist [ INOUTPAR ] { SEPER }
 *					( list1 | INBRACE list OUTBRACE )
 */

/**/
static void
par_funcdef(int *cmplx)
{
    int oecused = ecused, num = 0, onp, p, c = 0;
    int so, oecssub = ecssub;
    zlong oldlineno = lineno;
    int do_tracing = 0;

    lineno = 0;
    nocorrect = 1;
    incmdpos = 0;
    zshlex();

    p = ecadd(0);
    ecadd(0); /* p + 1 */

    /* Consume an initial (-T), (--), or (-T --).
     * Anything else is a literal function name.
     */
    if (tok == STRING && tokstr[0] == Dash) {
	if (tokstr[1] == 'T' && !tokstr[2]) {
	    ++do_tracing;
	    zshlex();
	}
	if (tok == STRING && tokstr[0] == Dash &&
	    tokstr[1] == Dash && !tokstr[2]) {
	    zshlex();
	}
    }

    while (tok == STRING) {
	if ((*tokstr == Inbrace || *tokstr == '{') &&
	    !tokstr[1]) {
	    tok = INBRACE;
	    break;
	}
	ecstr(tokstr);
	num++;
	zshlex();
    }
    ecadd(0); /* p + num + 2 */
    ecadd(0); /* p + num + 3 */
    ecadd(0); /* p + num + 4 */
    ecadd(0); /* p + num + 5 */

    nocorrect = 0;
    incmdpos = 1;
    if (tok == INOUTPAR)
	zshlex();
    while (tok == SEPER)
	zshlex();

    ecnfunc++;
    ecssub = so = ecsoffs;
    onp = ecnpats;
    ecnpats = 0;

    if (tok == INBRACE) {
	zshlex();
	par_list(&c);
	if (tok != OUTBRACE) {
	    lineno += oldlineno;
	    ecnpats = onp;
	    ecssub = oecssub;
	    YYERRORV(oecused);
	}
	if (num == 0) {
	    /* Anonymous function, possibly with arguments */
	    incmdpos = 0;
	}
	zshlex();
    } else if (unset(SHORTLOOPS)) {
	lineno += oldlineno;
	ecnpats = onp;
	ecssub = oecssub;
	YYERRORV(oecused);
    } else
	par_list1(&c);

    ecadd(WCB_END());
    ecbuf[p + num + 2] = so - oecssub;
    ecbuf[p + num + 3] = ecsoffs - so; /* "length of string table" */
    ecbuf[p + num + 4] = ecnpats; /* "number of patterns for body" */
    ecbuf[p + num + 5] = do_tracing;
    ecbuf[p + 1] = num; /* "number of names" */

    ecnpats = onp;
    ecssub = oecssub;
    ecnfunc++;

    ecbuf[p] = WCB_FUNCDEF(ecused - 1 - p); /* "offset to after body" */

    /* If it's an anonymous function... */
    if (num == 0) {
	/* ... look for arguments to it. */
	int parg = ecadd(0);
	ecadd(0);
	while (tok == STRING) {
	    ecstr(tokstr);
	    num++;
	    zshlex();
	}
	if (num > 0)
	    *cmplx = 1;
	ecbuf[parg] = ecused - parg; /*?*/
	ecbuf[parg+1] = num;
    }
    lineno += oldlineno;
}

/*
 * time	: TIME sublist2
 */

/**/
static void
par_time(void)
{
    int p, f, c = 0;

    zshlex();

    p = ecadd(0);
    ecadd(0);
    if ((f = par_sublist2(&c)) < 0) {
	ecused--;
	ecbuf[p] = WCB_TIMED(WC_TIMED_EMPTY);
    } else {
	ecbuf[p] = WCB_TIMED(WC_TIMED_PIPE);
	set_sublist_code(p + 1, WC_SUBLIST_END, f, ecused - 2 - p, c);
    }
}

/*
 * dinbrack	: DINBRACK cond DOUTBRACK
 */

/**/
static void
par_dinbrack(void)
{
    int oecused = ecused;

    incond = 1;
    incmdpos = 0;
    zshlex();
    par_cond();
    if (tok != DOUTBRACK)
	YYERRORV(oecused);
    incond = 0;
    incmdpos = 1;
    zshlex();
}

/*
 * simple	: { COMMAND | EXEC | NOGLOB | NOCORRECT | DASH }
					{ STRING | ENVSTRING | ENVARRAY wordlist OUTPAR | redir }
					[ INOUTPAR { SEPER } ( list1 | INBRACE list OUTBRACE ) ]
 *
 * Returns 0 if no code, else 1 plus the number of code words
 * used up by redirections.
 */

/**/
static int
par_simple(int *cmplx, int nr)
{
    int oecused = ecused, isnull = 1, r, argc = 0, p, isfunc = 0, sr = 0;
    int c = *cmplx, nrediradd, assignments = 0, ppost = 0, is_typeset = 0;
    char *hasalias = input_hasalias();
    wordcode postassigns = 0;

    r = ecused;
    for (;;) {
	if (tok == NOCORRECT) {
	    *cmplx = c = 1;
	    nocorrect = 1;
	} else if (tok == ENVSTRING) {
	    char *ptr, *name, *str;

	    name = tokstr;
	    for (ptr = tokstr;
		 *ptr && *ptr != Inbrack && *ptr != '=' && *ptr != '+';
	         ptr++);
	    if (*ptr == Inbrack) skipparens(Inbrack, Outbrack, &ptr);
	    if (*ptr == '+') {
	    	*ptr++ = '\0';
	    	ecadd(WCB_ASSIGN(WC_ASSIGN_SCALAR, WC_ASSIGN_INC, 0));
	    } else
		ecadd(WCB_ASSIGN(WC_ASSIGN_SCALAR, WC_ASSIGN_NEW, 0));

	    if (*ptr == '=') {
		*ptr = '\0';
		str = ptr + 1;
	    } else
		equalsplit(tokstr, &str);
	    for (ptr = str; *ptr; ptr++) {
		/*
		 * We can't treat this as "simple" if it contains
		 * expansions that require process substitution, since then
		 * we need process handling.
		 */
		if (ptr[1] == Inpar &&
		    (*ptr == Equals || *ptr == Inang || *ptr == OutangProc)) {
		    *cmplx = 1;
		    break;
		}
	    }
	    ecstr(name);
	    ecstr(str);
	    isnull = 0;
	    assignments = 1;
	} else if (tok == ENVARRAY) {
	    int oldcmdpos = incmdpos, n, type2;

	    /*
	     * We consider array setting cmplx because it can
	     * contain process substitutions, which need a valid job.
	     */
	    *cmplx = c = 1;
	    p = ecadd(0);
	    incmdpos = 0;
	    if ((type2 = strlen(tokstr) - 1) && tokstr[type2] == '+') {
	    	tokstr[type2] = '\0';
		type2 = WC_ASSIGN_INC;
    	    } else
		type2 = WC_ASSIGN_NEW;
	    ecstr(tokstr);
	    cmdpush(CS_ARRAY);
	    zshlex();
	    n = par_nl_wordlist();
	    ecbuf[p] = WCB_ASSIGN(WC_ASSIGN_ARRAY, type2, n);
	    cmdpop();
	    if (tok != OUTPAR)
		YYERROR(oecused);
	    incmdpos = oldcmdpos;
	    isnull = 0;
	    assignments = 1;
	} else if (IS_REDIROP(tok)) {
	    *cmplx = c = 1;
	    nr += par_redir(&r, NULL);
	    continue;
	} else
	    break;
	zshlex();
	if (!hasalias)
	    hasalias = input_hasalias();
    }
    if (tok == AMPER || tok == AMPERBANG)
	YYERROR(oecused);

    p = ecadd(WCB_SIMPLE(0));

    for (;;) {
	if (tok == STRING || tok == TYPESET) {
	    int redir_var = 0;

	    *cmplx = 1;
	    incmdpos = 0;

	    if (tok == TYPESET)
		intypeset = is_typeset = 1;

	    if (!isset(IGNOREBRACES) && *tokstr == Inbrace)
	    {
		/* Look for redirs of the form {var}>file etc. */
		char *eptr = tokstr + strlen(tokstr) - 1;
		char *ptr = eptr;

		if (*ptr == Outbrace && ptr > tokstr + 1)
		{
		    if (itype_end(tokstr+1, IIDENT, 0) >= ptr)
		    {
			char *toksave = tokstr;
			char *idstring = dupstrpfx(tokstr+1, eptr-tokstr-1);
			redir_var = 1;
			zshlex();
			if (!hasalias)
			    hasalias = input_hasalias();

			if (IS_REDIROP(tok) && tokfd == -1)
			{
			    *cmplx = c = 1;
			    nrediradd = par_redir(&r, idstring);
			    p += nrediradd;
			    sr += nrediradd;
			}
			else if (postassigns)
			{
			    /* C.f. normal case below */
			    postassigns++;
			    ecadd(WCB_ASSIGN(WC_ASSIGN_SCALAR, WC_ASSIGN_INC, 0));
			    ecstr(toksave);
			    ecstr("");	/* TBD can possibly optimise out */
			}
			else
			{
			    ecstr(toksave);
			    argc++;
			}
		    }
		}
	    }

	    if (!redir_var)
	    {
		if (postassigns) {
		    /*
		     * We're in the variable part of a typeset,
		     * but this doesn't have an assignment.
		     * We'll parse it as if it does, but mark
		     * it specially with WC_ASSIGN_INC.
		     */
		    postassigns++;
		    ecadd(WCB_ASSIGN(WC_ASSIGN_SCALAR, WC_ASSIGN_INC, 0));
		    ecstr(tokstr);
		    ecstr("");	/* TBD can possibly optimise out */
		} else {
		    ecstr(tokstr);
		    argc++;
		}
		zshlex();
		if (!hasalias)
		    hasalias = input_hasalias();
	    }
	} else if (IS_REDIROP(tok)) {
	    *cmplx = c = 1;
	    nrediradd = par_redir(&r, NULL);
	    p += nrediradd;
	    if (ppost)
		ppost += nrediradd;
	    sr += nrediradd;
	} else if (tok == ENVSTRING) {
	    char *ptr, *name, *str;

	    if (!postassigns++)
		ppost = ecadd(0);

	    name = tokstr;
	    for (ptr = tokstr; *ptr && *ptr != Inbrack && *ptr != '=' && *ptr != '+';
	         ptr++);
	    if (*ptr == Inbrack) skipparens(Inbrack, Outbrack, &ptr);
	    ecadd(WCB_ASSIGN(WC_ASSIGN_SCALAR, WC_ASSIGN_NEW, 0));

	    if (*ptr == '=') {
		*ptr = '\0';
		str = ptr + 1;
	    } else
		equalsplit(tokstr, &str);
	    ecstr(name);
	    ecstr(str);
	    zshlex();
	    if (!hasalias)
		hasalias = input_hasalias();
	} else if (tok == ENVARRAY) {
	    int n, parr;

	    if (!postassigns++)
		ppost = ecadd(0);

	    parr = ecadd(0);
	    ecstr(tokstr);
	    cmdpush(CS_ARRAY);
	    /*
	     * Careful here: this must be the typeset case,
	     * but we need to tell the lexer not to look
	     * for assignments until we've finished the
	     * present one.
	     */
	    intypeset = 0;
	    zshlex();
	    n = par_nl_wordlist();
	    ecbuf[parr] = WCB_ASSIGN(WC_ASSIGN_ARRAY, WC_ASSIGN_NEW, n);
	    cmdpop();
	    intypeset = 1;
	    if (tok != OUTPAR)
		YYERROR(oecused);
	    zshlex();
	} else if (tok == INOUTPAR) {
	    zlong oldlineno = lineno;
	    int onp, so, oecssub = ecssub;

	    /* Error if too many function definitions at once */
	    if (!isset(MULTIFUNCDEF) && argc > 1)
		YYERROR(oecused);
	    /* Error if preceding assignments */
	    if (assignments || postassigns)
		YYERROR(oecused);
	    if (isset(EXECOPT) && hasalias && !isset(ALIASFUNCDEF) && argc &&
		hasalias != input_hasalias()) {
		zwarn("defining function based on alias `%s'", hasalias);
		YYERROR(oecused);
	    }

	    *cmplx = c;
	    lineno = 0;
	    incmdpos = 1;
	    cmdpush(CS_FUNCDEF);
	    zshlex();
	    while (tok == SEPER)
		zshlex();

	    ecispace(p + 1, 1);
	    ecbuf[p + 1] = argc;
	    ecadd(0);
	    ecadd(0);
	    ecadd(0);
	    ecadd(0);

	    ecnfunc++;
	    ecssub = so = ecsoffs;
	    onp = ecnpats;
	    ecnpats = 0;

	    if (tok == INBRACE) {
		int c = 0;

		zshlex();
		par_list(&c);
		if (tok != OUTBRACE) {
		    cmdpop();
		    lineno += oldlineno;
		    ecnpats = onp;
		    ecssub = oecssub;
		    YYERROR(oecused);
		}
		if (argc == 0) {
		    /* Anonymous function, possibly with arguments */
		    incmdpos = 0;
		}
		zshlex();
	    } else {
		int ll, sl, c = 0;

		ll = ecadd(0);
		sl = ecadd(0);
		(void)ecadd(WCB_PIPE(WC_PIPE_END, 0));

		if (!par_cmd(&c, argc == 0)) {
		    cmdpop();
		    YYERROR(oecused);
		}
		if (argc == 0) {
		    /*
		     * Anonymous function, possibly with arguments.
		     * N.B. for cmplx structures in particular
		     * ( ... ) we rely on lower level code doing this
		     * to get the immediately following word (the
		     * first token after the ")" has already been
		     * read).
		     */
		    incmdpos = 0;
		}

		set_sublist_code(sl, WC_SUBLIST_END, 0, ecused - 1 - sl, c);
		set_list_code(ll, (Z_SYNC | Z_END), c);
	    }
	    cmdpop();

	    ecadd(WCB_END());
	    ecbuf[p + argc + 2] = so - oecssub;
	    ecbuf[p + argc + 3] = ecsoffs - so;
	    ecbuf[p + argc + 4] = ecnpats;
	    ecbuf[p + argc + 5] = 0;

	    ecnpats = onp;
	    ecssub = oecssub;
	    ecnfunc++;

	    ecbuf[p] = WCB_FUNCDEF(ecused - 1 - p);

	    /* If it's an anonymous function... */
	    if (argc == 0) {
		/* ... look for arguments to it. */
		int parg = ecadd(0);
		ecadd(0);
		while (tok == STRING || IS_REDIROP(tok)) {
		    if (tok == STRING)
		    {
			ecstr(tokstr);
			argc++;
			zshlex();
		    } else {
			*cmplx = c = 1;
			nrediradd = par_redir(&r, NULL);
			p += nrediradd;
			if (ppost)
			    ppost += nrediradd;
			sr += nrediradd;
			parg += nrediradd;
		    }
		}
		if (argc > 0)
		    *cmplx = 1;
		ecbuf[parg] = ecused - parg; /*?*/
		ecbuf[parg+1] = argc;
	    }
	    lineno += oldlineno;

	    isfunc = 1;
	    isnull = 0;
	    break;
	} else
	    break;
	isnull = 0;
    }
    if (isnull && !(sr + nr)) {
	ecused = p;
	return 0;
    }
    incmdpos = 1;
    intypeset = 0;

    if (!isfunc) {
	if (is_typeset) {
	    ecbuf[p] = WCB_TYPESET(argc);
	    if (postassigns)
		ecbuf[ppost] = postassigns;
	    else
		ecadd(0);
	} else
	    ecbuf[p] = WCB_SIMPLE(argc);
    }

    return sr + 1;
}

/*
 * redir	: ( OUTANG | ... | TRINANG ) STRING
 *
 * Return number of code words required for redirection
 */

static int redirtab[TRINANG - OUTANG + 1] = {
    REDIR_WRITE,
    REDIR_WRITENOW,
    REDIR_APP,
    REDIR_APPNOW,
    REDIR_READ,
    REDIR_READWRITE,
    REDIR_HEREDOC,
    REDIR_HEREDOCDASH,
    REDIR_MERGEIN,
    REDIR_MERGEOUT,
    REDIR_ERRWRITE,
    REDIR_ERRWRITENOW,
    REDIR_ERRAPP,
    REDIR_ERRAPPNOW,
    REDIR_HERESTR,
};

/**/
static int
par_redir(int *rp, char *idstring)
{
    int r = *rp, type, fd1, oldcmdpos, oldnc, ncodes;
    char *name;

    oldcmdpos = incmdpos;
    incmdpos = 0;
    oldnc = nocorrect;
    if (tok != INANG && tok != INOUTANG)
	nocorrect = 1;
    type = redirtab[tok - OUTANG];
    fd1 = tokfd;
    zshlex();
    if (tok != STRING && tok != ENVSTRING)
	YYERROR(ecused);
    incmdpos = oldcmdpos;
    nocorrect = oldnc;

    /* assign default fd */
    if (fd1 == -1)
	fd1 = IS_READFD(type) ? 0 : 1;

    name = tokstr;

    switch (type) {
    case REDIR_HEREDOC:
    case REDIR_HEREDOCDASH: {
	/* <<[-] name */
	struct heredocs **hd;
	int htype = type;

	/*
	 * Add two here for the string to remember the HERE
	 * terminator in raw and munged form.
	 */
	if (idstring)
	{
	    type |= REDIR_VARID_MASK;
	    ncodes = 6;
	}
	else
	    ncodes = 5;

	/* If we ever to change the number of codes, we have to change
	 * the definition of WC_REDIR_WORDS. */
	ecispace(r, ncodes);
	*rp = r + ncodes;
	ecbuf[r] = WCB_REDIR(type | REDIR_FROM_HEREDOC_MASK);
	ecbuf[r + 1] = fd1;

	/*
	 * r + 2: the HERE string we recover
	 * r + 3: the HERE document terminator, raw
	 * r + 4: the HERE document terminator, munged
	 */
	if (idstring)
	    ecbuf[r + 5] = ecstrcode(idstring);

	for (hd = &hdocs; *hd; hd = &(*hd)->next)
	    ;
	*hd = zalloc(sizeof(struct heredocs));
	(*hd)->next = NULL;
	(*hd)->type = htype;
	(*hd)->pc = r;
	(*hd)->str = tokstr;

	zshlex();
	return ncodes;
    }
    case REDIR_WRITE:
    case REDIR_WRITENOW:
	if (tokstr[0] == OutangProc && tokstr[1] == Inpar)
	    /* > >(...) */
	    type = REDIR_OUTPIPE;
	else if (tokstr[0] == Inang && tokstr[1] == Inpar)
	    YYERROR(ecused);
	break;
    case REDIR_READ:
	if (tokstr[0] == Inang && tokstr[1] == Inpar)
	    /* < <(...) */
	    type = REDIR_INPIPE;
	else if (tokstr[0] == OutangProc && tokstr[1] == Inpar)
	    YYERROR(ecused);
	break;
    case REDIR_READWRITE:
	if ((tokstr[0] == Inang || tokstr[0] == OutangProc) &&
	    tokstr[1] == Inpar)
	    type = tokstr[0] == Inang ? REDIR_INPIPE : REDIR_OUTPIPE;
	break;
    }
    zshlex();

    /* If we ever to change the number of codes, we have to change
     * the definition of WC_REDIR_WORDS. */
    if (idstring)
    {
	type |= REDIR_VARID_MASK;
	ncodes = 4;
    }
    else
	ncodes = 3;

    ecispace(r, ncodes);
    *rp = r + ncodes;
    ecbuf[r] = WCB_REDIR(type);
    ecbuf[r + 1] = fd1;
    ecbuf[r + 2] = ecstrcode(name);
    if (idstring)
	ecbuf[r + 3] = ecstrcode(idstring);

    return ncodes;
}

/**/
void
setheredoc(int pc, int type, char *str, char *termstr, char *munged_termstr)
{
    int varid = WC_REDIR_VARID(ecbuf[pc]) ? REDIR_VARID_MASK : 0;
    ecbuf[pc] = WCB_REDIR(type | REDIR_FROM_HEREDOC_MASK | varid);
    ecbuf[pc + 2] = ecstrcode(str);
    ecbuf[pc + 3] = ecstrcode(termstr);
    ecbuf[pc + 4] = ecstrcode(munged_termstr);
}

/*
 * wordlist	: { STRING }
 */

/**/
static int
par_wordlist(void)
{
    int num = 0;
    while (tok == STRING) {
	ecstr(tokstr);
	num++;
	zshlex();
    }
    return num;
}

/*
 * nl_wordlist	: { STRING | SEPER }
 */

/**/
static int
par_nl_wordlist(void)
{
    int num = 0;

    while (tok == STRING || tok == SEPER) {
	if (tok != SEPER) {
	    ecstr(tokstr);
	    num++;
	}
	zshlex();
    }
    return num;
}

/*
 * condlex is zshlex for normal parsing, but is altered to allow
 * the test builtin to use par_cond.
 */

/**/
void (*condlex) _((void)) = zshlex;

/*
 * cond	: cond_1 { SEPER } [ DBAR { SEPER } cond ]
 */

#define COND_SEP() (tok == SEPER && condlex != testlex && *zshlextext != ';')

/**/
static int
par_cond(void)
{
    int p = ecused, r;

    r = par_cond_1();
    while (COND_SEP())
	condlex();
    if (tok == DBAR) {
	condlex();
	while (COND_SEP())
	    condlex();
	ecispace(p, 1);
	par_cond();
	ecbuf[p] = WCB_COND(COND_OR, ecused - 1 - p);
	return 1;
    }
    return r;
}

/*
 * cond_1 : cond_2 { SEPER } [ DAMPER { SEPER } cond_1 ]
 */

/**/
static int
par_cond_1(void)
{
    int r, p = ecused;

    r = par_cond_2();
    while (COND_SEP())
	condlex();
    if (tok == DAMPER) {
	condlex();
	while (COND_SEP())
	    condlex();
	ecispace(p, 1);
	par_cond_1();
	ecbuf[p] = WCB_COND(COND_AND, ecused - 1 - p);
	return 1;
    }
    return r;
}

/*
 * Return 1 if condition matches.  This also works for non-elided options.
 *
 * input is test string, may begin - or Dash.
 * cond is condition following the -.
 */
static int check_cond(const char *input, const char *cond)
{
    if (!IS_DASH(input[0]))
	return 0;
    return !strcmp(input + 1, cond);
}

/*
 * cond_2	: BANG cond_2
				| INPAR { SEPER } cond_2 { SEPER } OUTPAR
				| STRING STRING STRING
				| STRING STRING
				| STRING ( INANG | OUTANG ) STRING
 */

/**/
static int
par_cond_2(void)
{
    char *s1, *s2, *s3;
    int dble = 0;
    int n_testargs = (condlex == testlex) ? arrlen(testargs) + 1 : 0;

    if (n_testargs) {
	/* See the description of test in POSIX 1003.2 */
	if (tok == NULLTOK)
	    /* no arguments: false */
	    return par_cond_double(dupstring("-n"), dupstring(""));
	if (n_testargs == 1) {
	    /* one argument: [ foo ] is equivalent to [ -n foo ] */
	    s1 = tokstr;
	    condlex();
	    /* ksh behavior: [ -t ] means [ -t 1 ]; bash disagrees */
	    if (unset(POSIXBUILTINS) && check_cond(s1, "t"))
		return par_cond_double(s1, dupstring("1"));
	    return par_cond_double(dupstring("-n"), s1);
	}
	if (n_testargs > 2) {
	    /* three arguments: if the second argument is a binary operator, *
	     * perform that binary test on the first and the third argument  */
	    if (!strcmp(*testargs, "=")  ||
		!strcmp(*testargs, "==") ||
		!strcmp(*testargs, "!=") ||
		(IS_DASH(**testargs) && get_cond_num(*testargs + 1) >= 0)) {
		s1 = tokstr;
		condlex();
		s2 = tokstr;
		condlex();
		s3 = tokstr;
		condlex();
		return par_cond_triple(s1, s2, s3);
	    }
	}
	/*
	 * We fall through here on any non-numeric infix operator
	 * or any other time there are at least two arguments.
	 */
    } else
	while (COND_SEP())
	    condlex();
    if (tok == BANG) {
	/*
	 * In "test" compatibility mode, "! -a ..." and "! -o ..."
	 * are treated as "[string] [and] ..." and "[string] [or] ...".
	 */
	if (!(n_testargs > 2 && (check_cond(*testargs, "a") ||
				 check_cond(*testargs, "o"))))
	{
	    condlex();
	    ecadd(WCB_COND(COND_NOT, 0));
	    return par_cond_2();
	}
    }
    if (tok == INPAR) {
	int r;

	condlex();
	while (COND_SEP())
	    condlex();
	r = par_cond();
	while (COND_SEP())
	    condlex();
	if (tok != OUTPAR)
	    YYERROR(ecused);
	condlex();
	return r;
    }
    s1 = tokstr;
    dble = (s1 && IS_DASH(*s1)
	    && (!n_testargs
		|| strspn(s1+1, "abcdefghknoprstuvwxzLONGS") == 1)
	    && !s1[2]);
    if (tok != STRING) {
	/* Check first argument for [[ STRING ]] re-interpretation */
	if (s1 /* tok != DOUTBRACK && tok != DAMPER && tok != DBAR */
	    && tok != LEXERR && (!dble || n_testargs)) {
	    do condlex(); while (COND_SEP());
	    return par_cond_double(dupstring("-n"), s1);
	} else
	    YYERROR(ecused);
    }
    condlex();
    if (n_testargs == 2 && tok != STRING && tokstr && IS_DASH(s1[0])) {
	/*
	 * Something like "test -z" followed by a token.
	 * We'll turn the token into a string (we've also
	 * checked it does have a string representation).
	 */
	tok = STRING;
    } else
	while (COND_SEP())
	    condlex();
    if (tok == INANG || tok == OUTANG) {
	enum lextok xtok = tok;
	do condlex(); while (COND_SEP());
	if (tok != STRING)
	    YYERROR(ecused);
	s3 = tokstr;
	do condlex(); while (COND_SEP());
	ecadd(WCB_COND((xtok == INANG ? COND_STRLT : COND_STRGTR), 0));
	ecstr(s1);
	ecstr(s3);
	return 1;
    }
    if (tok != STRING) {
	/*
	 * Check second argument in case semantics e.g. [ = -a = ]
	 * mean we have to go back and fix up the first one
	 */
	if (tok != LEXERR) {
	    if (!dble || n_testargs)
		return par_cond_double(dupstring("-n"), s1);
	    else
		return par_cond_multi(s1, newlinklist());
	} else
	    YYERROR(ecused);
    }
    s2 = tokstr;
    if (!n_testargs)
	dble = (s2 && IS_DASH(*s2) && !s2[2]);
    incond++;			/* parentheses do globbing */
    do condlex(); while (COND_SEP());
    incond--;			/* parentheses do grouping */
    if (tok == STRING && !dble) {
	s3 = tokstr;
	do condlex(); while (COND_SEP());
	if (tok == STRING) {
	    LinkList l = newlinklist();

	    addlinknode(l, s2);
	    addlinknode(l, s3);

	    while (tok == STRING) {
		addlinknode(l, tokstr);
		do condlex(); while (COND_SEP());
	    }
	    return par_cond_multi(s1, l);
	} else
	    return par_cond_triple(s1, s2, s3);
    } else
	return par_cond_double(s1, s2);
}

/**/
static int
par_cond_double(char *a, char *b)
{
    if (!IS_DASH(a[0]) || !a[1])
	COND_ERROR("parse error: condition expected: %s", a);
    else if (!a[2] && strspn(a+1, "abcdefgknoprstuvwxzhLONGS") == 1) {
	ecadd(WCB_COND(a[1], 0));
	ecstr(b);
    } else {
	ecadd(WCB_COND(COND_MOD, 1));
	ecstr(a);
	ecstr(b);
    }
    return 1;
}

/**/
static int
get_cond_num(char *tst)
{
    static char *condstrs[] =
    {
	"nt", "ot", "ef", "eq", "ne", "lt", "gt", "le", "ge", NULL
    };
    int t0;

    for (t0 = 0; condstrs[t0]; t0++)
	if (!strcmp(condstrs[t0], tst))
	    return t0;
    return -1;
}

/**/
static int
par_cond_triple(char *a, char *b, char *c)
{
    int t0;

    if ((b[0] == Equals || b[0] == '=') && !b[1]) {
	ecadd(WCB_COND(COND_STREQ, 0));
	ecstr(a);
	ecstr(c);
	ecadd(ecnpats++);
    } else if ((b[0] == Equals || b[0] == '=') &&
	       (b[1] == Equals || b[1] == '=') && !b[2]) {
	ecadd(WCB_COND(COND_STRDEQ, 0));
	ecstr(a);
	ecstr(c);
	ecadd(ecnpats++);
    } else if (b[0] == '!' && (b[1] == Equals || b[1] == '=') && !b[2]) {
	ecadd(WCB_COND(COND_STRNEQ, 0));
	ecstr(a);
	ecstr(c);
	ecadd(ecnpats++);
    } else if ((b[0] == Equals || b[0] == '=') &&
               (b[1] == '~' || b[1] == Tilde) && !b[2]) {
        /* We become an implicit COND_MODI but do not provide the first
	 * item, it's skipped */
	ecadd(WCB_COND(COND_REGEX, 0));
	ecstr(a);
	ecstr(c);
    } else if (IS_DASH(b[0])) {
	if ((t0 = get_cond_num(b + 1)) > -1) {
	    ecadd(WCB_COND(t0 + COND_NT, 0));
	    ecstr(a);
	    ecstr(c);
	} else {
	    ecadd(WCB_COND(COND_MODI, 0));
	    ecstr(b);
	    ecstr(a);
	    ecstr(c);
	}
    } else if (IS_DASH(a[0]) && a[1]) {
	ecadd(WCB_COND(COND_MOD, 2));
	ecstr(a);
	ecstr(b);
	ecstr(c);
    } else
	COND_ERROR("condition expected: %s", b);

    return 1;
}

/**/
static int
par_cond_multi(char *a, LinkList l)
{
    if (!IS_DASH(a[0]) || !a[1])
	COND_ERROR("condition expected: %s", a);
    else {
	LinkNode n;

	ecadd(WCB_COND(COND_MOD, countlinknodes(l)));
	ecstr(a);
	for (n = firstnode(l); n; incnode(n))
	    ecstr((char *) getdata(n));
    }
    return 1;
}

/**/
static void
yyerror(int noerr)
{
    int t0;
    char *t;

    if ((t = dupstring(zshlextext)))
	untokenize(t);

    for (t0 = 0; t0 != 20; t0++)
	if (!t || !t[t0] || t[t0] == '\n')
	    break;
    if (!(histdone & HISTFLAG_NOEXEC) && !(errflag & ERRFLAG_INT)) {
	if (t0 == 20)
	    zwarn("parse error near `%l...'", t, 20);
	else if (t0)
	    zwarn("parse error near `%l'", t, t0);
	else
	    zwarn("parse error");
    }
    if (!noerr && noerrs != 2)
	errflag |= ERRFLAG_ERROR;
}

/*
 * Duplicate a programme list, on the heap if heap is 1, else
 * in permanent storage.
 *
 * Be careful in case p is the Eprog for a function which will
 * later be autoloaded.  The shf element of the returned Eprog
 * must be set appropriately by the caller.  (Normally we create
 * the Eprog in this case by using mkautofn.)
 */

/**/
mod_export Eprog
dupeprog(Eprog p, int heap)
{
    Eprog r;
    int i;
    Patprog *pp;

    if (p == &dummy_eprog)
	return p;

    r = (heap ? (Eprog) zhalloc(sizeof(*r)) : (Eprog) zalloc(sizeof(*r)));
    r->flags = (heap ? EF_HEAP : EF_REAL) | (p->flags & EF_RUN);
    r->dump = NULL;
    r->len = p->len;
    r->npats = p->npats;
    /*
     * If Eprog is on the heap, reference count is not valid.
     * Otherwise, initialise reference count to 1 so that a freeeprog()
     * will delete it if it is not in use.
     */
    r->nref = heap ? -1 : 1;
    pp = r->pats = (heap ? (Patprog *) hcalloc(r->len) :
		    (Patprog *) zshcalloc(r->len));
    r->prog = (Wordcode) (r->pats + r->npats);
    r->strs = ((char *) r->prog) + (p->strs - ((char *) p->prog));
    memcpy(r->prog, p->prog, r->len - (p->npats * sizeof(Patprog)));
    r->shf = NULL;

    for (i = r->npats; i--; pp++)
	*pp = dummy_patprog1;

    return r;
}


/*
 * Pair of functions to mark an Eprog as in use, and to delete it
 * when it is no longer in use, by means of the reference count in
 * then nref element.
 *
 * If nref is negative, the Eprog is on the heap and is never freed.
 */

/* Increase the reference count of an Eprog so it won't be deleted. */

/**/
mod_export void
useeprog(Eprog p)
{
    if (p && p != &dummy_eprog && p->nref >= 0)
	p->nref++;
}

/* Free an Eprog if we have finished with it */

/**/
mod_export void
freeeprog(Eprog p)
{
    int i;
    Patprog *pp;

    if (p && p != &dummy_eprog) {
	/* paranoia */
	DPUTS(p->nref > 0 && (p->flags & EF_HEAP), "Heap EPROG has nref > 0");
	DPUTS(p->nref < 0 && !(p->flags & EF_HEAP), "Real EPROG has nref < 0");
	DPUTS(p->nref < -1, "Uninitialised EPROG nref");
	if (p->nref > 0 && !--p->nref) {
	    for (i = p->npats, pp = p->pats; i--; pp++)
		freepatprog(*pp);
	    if (p->dump) {
		decrdumpcount(p->dump);
		zfree(p->pats, p->npats * sizeof(Patprog));
	    } else
		zfree(p->pats, p->len);
	    zfree(p, sizeof(*p));
	}
    }
}

/*
 * dup is of type 'enum ec_dup_t'.
 *
 * If tokflag is not NULL, *tokflag will be set to 1 if the string contains
 * tokens and to 0 otherwise.
 */

/**/
char *
ecgetstr(Estate s, int dup, int *tokflag)
{
    static char buf[4];
    wordcode c = *s->pc++;
    char *r;

    if (c == 6 || c == 7)
	r = "";
    else if (c & 2) {
	buf[0] = (char) ((c >>  3) & 0xff);
	buf[1] = (char) ((c >> 11) & 0xff);
	buf[2] = (char) ((c >> 19) & 0xff);
	buf[3] = '\0';
	r = dupstring(buf);
	dup = EC_NODUP;
    } else {
	r = s->strs + (c >> 2);
    }
    if (tokflag)
	*tokflag = (c & 1);

    /*** Since function dump files are mapped read-only, avoiding to
     *   to duplicate strings when they don't contain tokens may fail
     *   when one of the many utility functions happens to write to
     *   one of the strings (without really modifying it).
     *   If that happens to you and you don't feel like debugging it,
     *   just change the line below to:
     *
     *     return (dup ? dupstring(r) : r);
     */

    return ((dup == EC_DUP || (dup && (c & 1)))  ? dupstring(r) : r);
}

/**/
char *
ecrawstr(Eprog p, Wordcode pc, int *tokflag)
{
    static char buf[4];
    wordcode c = *pc;

    if (c == 6 || c == 7) {
	if (tokflag)
	    *tokflag = (c & 1);
	return "";
    } else if (c & 2) {
	buf[0] = (char) ((c >>  3) & 0xff);
	buf[1] = (char) ((c >> 11) & 0xff);
	buf[2] = (char) ((c >> 19) & 0xff);
	buf[3] = '\0';
	if (tokflag)
	    *tokflag = (c & 1);
	return buf;
    } else {
	if (tokflag)
	    *tokflag = (c & 1);
	return p->strs + (c >> 2);
    }
}

/**/
char **
ecgetarr(Estate s, int num, int dup, int *tokflag)
{
    char **ret, **rp;
    int tf = 0, tmp = 0;

    ret = rp = (char **) zhalloc((num + 1) * sizeof(char *));

    while (num--) {
	*rp++ = ecgetstr(s, dup, &tmp);
	tf |=  tmp;
    }
    *rp = NULL;
    if (tokflag)
	*tokflag = tf;

    return ret;
}

/**/
LinkList
ecgetlist(Estate s, int num, int dup, int *tokflag)
{
    if (num) {
	LinkList ret;
	int i, tf = 0, tmp = 0;

	ret = newsizedlist(num);
	for (i = 0; i < num; i++) {
	    setsizednode(ret, i, ecgetstr(s, dup, &tmp));
	    tf |= tmp;
	}
	if (tokflag)
	    *tokflag = tf;
	return ret;
    }
    if (tokflag)
	*tokflag = 0;
    return NULL;
}

/**/
LinkList
ecgetredirs(Estate s)
{
    LinkList ret = newlinklist();
    wordcode code = *s->pc++;

    while (wc_code(code) == WC_REDIR) {
	Redir r = (Redir) zhalloc(sizeof(*r));

	r->type = WC_REDIR_TYPE(code);
	r->fd1 = *s->pc++;
	r->name = ecgetstr(s, EC_DUP, NULL);
	if (WC_REDIR_FROM_HEREDOC(code)) {
	    r->flags = REDIRF_FROM_HEREDOC;
	    r->here_terminator = ecgetstr(s, EC_DUP, NULL);
	    r->munged_here_terminator = ecgetstr(s, EC_DUP, NULL);
	} else {
	    r->flags = 0;
	    r->here_terminator = NULL;
	    r->munged_here_terminator = NULL;
	}
	if (WC_REDIR_VARID(code))
	    r->varid = ecgetstr(s, EC_DUP, NULL);
	else
	    r->varid = NULL;

	addlinknode(ret, r);

	code = *s->pc++;
    }
    s->pc--;

    return ret;
}

/*
 * Copy the consecutive set of redirections in the state at s.
 * Return NULL if none, else an Eprog consisting only of the
 * redirections from permanently allocated memory.
 *
 * s is left in the state ready for whatever follows the redirections.
 */

/**/
Eprog
eccopyredirs(Estate s)
{
    Wordcode pc = s->pc;
    wordcode code = *pc;
    int ncode, ncodes = 0, r;

    if (wc_code(code) != WC_REDIR)
	return NULL;

    init_parse();

    while (wc_code(code) == WC_REDIR) {
#ifdef DEBUG
	int type = WC_REDIR_TYPE(code);
#endif

	DPUTS(type == REDIR_HEREDOC || type == REDIR_HEREDOCDASH,
	      "unexpanded here document");

	if (WC_REDIR_FROM_HEREDOC(code))
	    ncode = 5;
	else
	    ncode = 3;
	if (WC_REDIR_VARID(code))
	    ncode++;
	pc += ncode;
	ncodes += ncode;
	code = *pc;
    }
    r = ecused;
    ecispace(r, ncodes);

    code = *s->pc;
    while (wc_code(code) == WC_REDIR) {
	s->pc++;

	ecbuf[r++] = code;
	/* fd1 */
	ecbuf[r++] = *s->pc++;
	/* name or HERE string */
	/* No DUP needed as we'll copy into Eprog immediately below */
	ecbuf[r++] = ecstrcode(ecgetstr(s, EC_NODUP, NULL));
	if (WC_REDIR_FROM_HEREDOC(code))
	{
	    /* terminator, raw */
	    ecbuf[r++] = ecstrcode(ecgetstr(s, EC_NODUP, NULL));
	    /* terminator, munged */
	    ecbuf[r++] = ecstrcode(ecgetstr(s, EC_NODUP, NULL));
	}
	if (WC_REDIR_VARID(code))
	    ecbuf[r++] = ecstrcode(ecgetstr(s, EC_NODUP, NULL));

	code = *s->pc;
    }

    /* bld_eprog() appends a useful WC_END marker */
    return bld_eprog(0);
}

/**/
mod_export struct eprog dummy_eprog;

static wordcode dummy_eprog_code;

/**/
void
init_eprog(void)
{
    dummy_eprog_code = WCB_END();
    dummy_eprog.len = sizeof(wordcode);
    dummy_eprog.prog = &dummy_eprog_code;
    dummy_eprog.strs = NULL;
}

/* Code for function dump files.
 *
 * Dump files consist of a header and the function bodies (the wordcode
 * plus the string table) and that twice: once for the byte-order of the
 * host the file was created on and once for the other byte-order. The
 * header describes where the beginning of the `other' version is and it
 * is up to the shell reading the file to decide which version it needs.
 * This is done by checking if the first word is FD_MAGIC (then the 
 * shell reading the file has the same byte order as the one that created
 * the file) or if it is FD_OMAGIC, then the `other' version has to be
 * read.
 * The header is the magic number, a word containing the flags (if the
 * file should be mapped or read and if this header is the `other' one),
 * the version string in a field of 40 characters and the descriptions
 * for the functions in the dump file.
 *
 * NOTES:
 *  - This layout has to be kept; everything after it may be changed.
 *  - When incompatible changes are made, the FD_MAGIC and FD_OMAGIC
 *    numbers have to be changed.
 *
 * Each description consists of a struct fdhead followed by the name,
 * aligned to sizeof(wordcode) (i.e. 4 bytes).
 */

#include "version.h"

#define FD_EXT ".zwc"
#define FD_MINMAP 4096

#define FD_PRELEN 12
#define FD_MAGIC  0x04050607
#define FD_OMAGIC 0x07060504

#define FDF_MAP   1
#define FDF_OTHER 2

typedef struct fdhead *FDHead;

struct fdhead {
    wordcode start;		/* offset to function definition */
    wordcode len;		/* length of wordcode/strings */
    wordcode npats;		/* number of patterns needed */
    wordcode strs;		/* offset to strings */
    wordcode hlen;		/* header length (incl. name) */
    wordcode flags;		/* flags and offset to name tail */
};

#define fdheaderlen(f) (((Wordcode) (f))[FD_PRELEN])

#define fdmagic(f)       (((Wordcode) (f))[0])
#define fdsetbyte(f,i,v) \
    ((((unsigned char *) (((Wordcode) (f)) + 1))[i]) = ((unsigned char) (v)))
#define fdbyte(f,i)      ((wordcode) (((unsigned char *) (((Wordcode) (f)) + 1))[i]))
#define fdflags(f)       fdbyte(f, 0)
#define fdsetflags(f,v)  fdsetbyte(f, 0, v)
#define fdother(f)       (fdbyte(f, 1) + (fdbyte(f, 2) << 8) + (fdbyte(f, 3) << 16))
#define fdsetother(f, o) \
    do { \
        fdsetbyte(f, 1, ((o) & 0xff)); \
        fdsetbyte(f, 2, (((o) >> 8) & 0xff)); \
        fdsetbyte(f, 3, (((o) >> 16) & 0xff)); \
    } while (0)
#define fdversion(f)     ((char *) ((f) + 2))

#define firstfdhead(f) ((FDHead) (((Wordcode) (f)) + FD_PRELEN))
#define nextfdhead(f)  ((FDHead) (((Wordcode) (f)) + (f)->hlen))

#define fdhflags(f)      (((FDHead) (f))->flags)
#define fdhtail(f)       (((FDHead) (f))->flags >> 2)
#define fdhbldflags(f,t) ((f) | ((t) << 2))

#define FDHF_KSHLOAD 1
#define FDHF_ZSHLOAD 2

#define fdname(f)      ((char *) (((FDHead) (f)) + 1))

/* This is used when building wordcode files. */

typedef struct wcfunc *WCFunc;

struct wcfunc {
    char *name;
    Eprog prog;
    int flags;
};

/* Try to find the description for the given function name. */

static FDHead
dump_find_func(Wordcode h, char *name)
{
    FDHead n, e = (FDHead) (h + fdheaderlen(h));

    for (n = firstfdhead(h); n < e; n = nextfdhead(n))
	if (!strcmp(name, fdname(n) + fdhtail(n)))
	    return n;

    return NULL;
}

/**/
int
bin_zcompile(char *nam, char **args, Options ops, UNUSED(int func))
{
    int map, flags, ret;
    char *dump;

    if ((OPT_ISSET(ops,'k') && OPT_ISSET(ops,'z')) ||
	(OPT_ISSET(ops,'R') && OPT_ISSET(ops,'M')) ||
	(OPT_ISSET(ops,'c') &&
	 (OPT_ISSET(ops,'U') || OPT_ISSET(ops,'k') || OPT_ISSET(ops,'z'))) ||
	(!(OPT_ISSET(ops,'c') || OPT_ISSET(ops,'a')) && OPT_ISSET(ops,'m'))) {
	zwarnnam(nam, "illegal combination of options");
	return 1;
    }
    if ((OPT_ISSET(ops,'c') || OPT_ISSET(ops,'a')) && isset(KSHAUTOLOAD))
	zwarnnam(nam, "functions will use zsh style autoloading");

    flags = (OPT_ISSET(ops,'k') ? FDHF_KSHLOAD :
	     (OPT_ISSET(ops,'z') ? FDHF_ZSHLOAD : 0));

    if (OPT_ISSET(ops,'t')) {
	Wordcode f;

	if (!*args) {
	    zwarnnam(nam, "too few arguments");
	    return 1;
	}
	if (!(f = load_dump_header(nam, (strsfx(FD_EXT, *args) ? *args :
					 dyncat(*args, FD_EXT)), 1)))
		return 1;

	if (args[1]) {
	    for (args++; *args; args++)
		if (!dump_find_func(f, *args))
		    return 1;
	    return 0;
	} else {
	    FDHead h, e = (FDHead) (f + fdheaderlen(f));

	    printf("zwc file (%s) for zsh-%s\n",
		   ((fdflags(f) & FDF_MAP) ? "mapped" : "read"), fdversion(f));
	    for (h = firstfdhead(f); h < e; h = nextfdhead(h))
		printf("%s\n", fdname(h));
	    return 0;
	}
    }
    if (!*args) {
	zwarnnam(nam, "too few arguments");
	return 1;
    }
    map = (OPT_ISSET(ops,'M') ? 2 : (OPT_ISSET(ops,'R') ? 0 : 1));

    if (!args[1] && !(OPT_ISSET(ops,'c') || OPT_ISSET(ops,'a'))) {
	queue_signals();
	ret = build_dump(nam, dyncat(*args, FD_EXT), args, OPT_ISSET(ops,'U'),
			 map, flags);
	unqueue_signals();
	return ret;
    }
    dump = (strsfx(FD_EXT, *args) ? *args : dyncat(*args, FD_EXT));

    queue_signals();
    ret = ((OPT_ISSET(ops,'c') || OPT_ISSET(ops,'a')) ?
	   build_cur_dump(nam, dump, args + 1, OPT_ISSET(ops,'m'), map,
			  (OPT_ISSET(ops,'c') ? 1 : 0) | 
			  (OPT_ISSET(ops,'a') ? 2 : 0)) :
	   build_dump(nam, dump, args + 1, OPT_ISSET(ops,'U'), map, flags));
    unqueue_signals();

    return ret;
}

/* Load the header of a dump file. Returns NULL if the file isn't a
 * valid dump file. */

/**/
static Wordcode
load_dump_header(char *nam, char *name, int err)
{
    int fd, v = 1;
    wordcode buf[FD_PRELEN + 1];

    if ((fd = open(name, O_RDONLY)) < 0) {
	if (err)
	    zwarnnam(nam, "can't open zwc file: %s", name);
	return NULL;
    }
    if (read(fd, buf, (FD_PRELEN + 1) * sizeof(wordcode)) !=
	((FD_PRELEN + 1) * sizeof(wordcode)) ||
	(v = (fdmagic(buf) != FD_MAGIC && fdmagic(buf) != FD_OMAGIC)) ||
	strcmp(fdversion(buf), ZSH_VERSION)) {
	if (err) {
	    if (!v) {
		zwarnnam(nam, "zwc file has wrong version (zsh-%s): %s",
			 fdversion(buf), name);
	    } else
		zwarnnam(nam, "invalid zwc file: %s" , name);
	}
	close(fd);
	return NULL;
    } else {
	int len;
	Wordcode head;

	if (fdmagic(buf) == FD_MAGIC) {
	    len = fdheaderlen(buf) * sizeof(wordcode);
	    head = (Wordcode) zhalloc(len);
	}
	else {
	    int o = fdother(buf);

	    if (lseek(fd, o, 0) == -1 ||
		read(fd, buf, (FD_PRELEN + 1) * sizeof(wordcode)) !=
		((FD_PRELEN + 1) * sizeof(wordcode))) {
		zwarnnam(nam, "invalid zwc file: %s" , name);
		close(fd);
		return NULL;
	    }
	    len = fdheaderlen(buf) * sizeof(wordcode);
	    head = (Wordcode) zhalloc(len);
	}
	memcpy(head, buf, (FD_PRELEN + 1) * sizeof(wordcode));

	len -= (FD_PRELEN + 1) * sizeof(wordcode);
	if (read(fd, head + (FD_PRELEN + 1), len) != len) {
	    close(fd);
	    zwarnnam(nam, "invalid zwc file: %s" , name);
	    return NULL;
	}
	close(fd);
	return head;
    }
}

/* Swap the bytes in a wordcode. */

static void
fdswap(Wordcode p, int n)
{
    wordcode c;

    for (; n--; p++) {
	c = *p;
	*p = (((c & 0xff) << 24) |
	      ((c & 0xff00) << 8) |
	      ((c & 0xff0000) >> 8) |
	      ((c & 0xff000000) >> 24));
    }
}

/* Write a dump file. */

static void
write_dump(int dfd, LinkList progs, int map, int hlen, int tlen)
{
    LinkNode node;
    WCFunc wcf;
    int other = 0, ohlen, tmp;
    wordcode pre[FD_PRELEN];
    char *tail, *n;
    struct fdhead head;
    Eprog prog;

    if (map == 1)
	map = (tlen >= FD_MINMAP);

    memset(pre, 0, sizeof(wordcode) * FD_PRELEN);

    for (ohlen = hlen; ; hlen = ohlen) {
	fdmagic(pre) = (other ? FD_OMAGIC : FD_MAGIC);
	fdsetflags(pre, ((map ? FDF_MAP : 0) | other));
	fdsetother(pre, tlen);
	strcpy(fdversion(pre), ZSH_VERSION);
	write_loop(dfd, (char *)pre, FD_PRELEN * sizeof(wordcode));

	for (node = firstnode(progs); node; incnode(node)) {
	    wcf = (WCFunc) getdata(node);
	    n = wcf->name;
	    prog = wcf->prog;
	    head.start = hlen;
	    hlen += (prog->len - (prog->npats * sizeof(Patprog)) +
		     sizeof(wordcode) - 1) / sizeof(wordcode);
	    head.len = prog->len - (prog->npats * sizeof(Patprog));
	    head.npats = prog->npats;
	    head.strs = prog->strs - ((char *) prog->prog);
	    head.hlen = (sizeof(struct fdhead) / sizeof(wordcode)) +
		(strlen(n) + sizeof(wordcode)) / sizeof(wordcode);
	    if ((tail = strrchr(n, '/')))
		tail++;
	    else
		tail = n;
	    head.flags = fdhbldflags(wcf->flags, (tail - n));
	    if (other)
		fdswap((Wordcode) &head, sizeof(head) / sizeof(wordcode));
	    write_loop(dfd, (char *)&head, sizeof(head));
	    tmp = strlen(n) + 1;
	    write_loop(dfd, n, tmp);
	    if ((tmp &= (sizeof(wordcode) - 1)))
		write_loop(dfd, (char *)&head, sizeof(wordcode) - tmp);
	}
	for (node = firstnode(progs); node; incnode(node)) {
	    prog = ((WCFunc) getdata(node))->prog;
	    tmp = (prog->len - (prog->npats * sizeof(Patprog)) +
		   sizeof(wordcode) - 1) / sizeof(wordcode);
	    if (other)
		fdswap(prog->prog, (((Wordcode) prog->strs) - prog->prog));
	    write_loop(dfd, (char *)prog->prog, tmp * sizeof(wordcode));
	}
	if (other)
	    break;
	other = FDF_OTHER;
    }
}

/**/
static int
build_dump(char *nam, char *dump, char **files, int ali, int map, int flags)
{
    int dfd, fd, hlen, tlen, flen, ona = noaliases;
    LinkList progs;
    char *file;
    Eprog prog;
    WCFunc wcf;

    if (!strsfx(FD_EXT, dump))
	dump = dyncat(dump, FD_EXT);

    unlink(dump);
    if ((dfd = open(dump, O_WRONLY|O_CREAT, 0444)) < 0) {
	zwarnnam(nam, "can't write zwc file: %s", dump);
	return 1;
    }
    progs = newlinklist();
    noaliases = ali;

    for (hlen = FD_PRELEN, tlen = 0; *files; files++) {
	struct stat st;

	if (check_cond(*files, "k")) {
	    flags = (flags & ~(FDHF_KSHLOAD | FDHF_ZSHLOAD)) | FDHF_KSHLOAD;
	    continue;
	} else if (check_cond(*files, "z")) {
	    flags = (flags & ~(FDHF_KSHLOAD | FDHF_ZSHLOAD)) | FDHF_ZSHLOAD;
	    continue;
	}
	if ((fd = open(*files, O_RDONLY)) < 0 ||
	    fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) ||
	    (flen = lseek(fd, 0, 2)) == -1) {
	    if (fd >= 0)
		close(fd);
	    close(dfd);
	    zwarnnam(nam, "can't open file: %s", *files);
	    noaliases = ona;
	    unlink(dump);
	    return 1;
	}
	file = (char *) zalloc(flen + 1);
	file[flen] = '\0';
	lseek(fd, 0, 0);
	if (read(fd, file, flen) != flen) {
	    close(fd);
	    close(dfd);
	    zfree(file, flen);
	    zwarnnam(nam, "can't read file: %s", *files);
	    noaliases = ona;
	    unlink(dump);
	    return 1;
	}
	close(fd);
	file = metafy(file, flen, META_REALLOC);

	if (!(prog = parse_string(file, 1)) || errflag) {
	    errflag &= ~ERRFLAG_ERROR;
	    close(dfd);
	    zfree(file, flen);
	    zwarnnam(nam, "can't read file: %s", *files);
	    noaliases = ona;
	    unlink(dump);
	    return 1;
	}
	zfree(file, flen);

	wcf = (WCFunc) zhalloc(sizeof(*wcf));
	wcf->name = *files;
	wcf->prog = prog;
	wcf->flags = ((prog->flags & EF_RUN) ? FDHF_KSHLOAD : flags);
	addlinknode(progs, wcf);

	flen = (strlen(*files) + sizeof(wordcode)) / sizeof(wordcode);
	hlen += (sizeof(struct fdhead) / sizeof(wordcode)) + flen;

	tlen += (prog->len - (prog->npats * sizeof(Patprog)) +
		 sizeof(wordcode) - 1) / sizeof(wordcode);
    }
    noaliases = ona;

    tlen = (tlen + hlen) * sizeof(wordcode);

    write_dump(dfd, progs, map, hlen, tlen);

    close(dfd);

    return 0;
}

static int
cur_add_func(char *nam, Shfunc shf, LinkList names, LinkList progs,
	     int *hlen, int *tlen, int what)
{
    Eprog prog;
    WCFunc wcf;

    if (shf->node.flags & PM_UNDEFINED) {
	int ona = noaliases;

	if (!(what & 2)) {
	    zwarnnam(nam, "function is not loaded: %s", shf->node.nam);
	    return 1;
	}
	noaliases = (shf->node.flags & PM_UNALIASED);
	if (!(prog = getfpfunc(shf->node.nam, NULL, NULL, NULL, 0)) ||
	    prog == &dummy_eprog) {
	    noaliases = ona;
	    zwarnnam(nam, "can't load function: %s", shf->node.nam);
	    return 1;
	}
	if (prog->dump)
	    prog = dupeprog(prog, 1);
	noaliases = ona;
    } else {
	if (!(what & 1)) {
	    zwarnnam(nam, "function is already loaded: %s", shf->node.nam);
	    return 1;
	}
	prog = dupeprog(shf->funcdef, 1);
    }
    wcf = (WCFunc) zhalloc(sizeof(*wcf));
    wcf->name = shf->node.nam;
    wcf->prog = prog;
    wcf->flags = ((prog->flags & EF_RUN) ? FDHF_KSHLOAD : FDHF_ZSHLOAD);
    addlinknode(progs, wcf);
    addlinknode(names, shf->node.nam);

    *hlen += ((sizeof(struct fdhead) / sizeof(wordcode)) +
	      ((strlen(shf->node.nam) + sizeof(wordcode)) / sizeof(wordcode)));
    *tlen += (prog->len - (prog->npats * sizeof(Patprog)) +
	      sizeof(wordcode) - 1) / sizeof(wordcode);

    return 0;
}

/**/
static int
build_cur_dump(char *nam, char *dump, char **names, int match, int map,
	       int what)
{
    int dfd, hlen, tlen;
    LinkList progs, lnames;
    Shfunc shf = NULL;

    if (!strsfx(FD_EXT, dump))
	dump = dyncat(dump, FD_EXT);

    unlink(dump);
    if ((dfd = open(dump, O_WRONLY|O_CREAT, 0444)) < 0) {
	zwarnnam(nam, "can't write zwc file: %s", dump);
	return 1;
    }
    progs = newlinklist();
    lnames = newlinklist();

    hlen = FD_PRELEN;
    tlen = 0;

    if (!*names) {
	int i;
	HashNode hn;

	for (i = 0; i < shfunctab->hsize; i++)
	    for (hn = shfunctab->nodes[i]; hn; hn = hn->next)
		if (cur_add_func(nam, (Shfunc) hn, lnames, progs,
				 &hlen, &tlen, what)) {
		    errflag &= ~ERRFLAG_ERROR;
		    close(dfd);
		    unlink(dump);
		    return 1;
		}
    } else if (match) {
	char *pat;
	Patprog pprog;
	int i;
	HashNode hn;

	for (; *names; names++) {
	    tokenize(pat = dupstring(*names));
	    /* Signal-safe here, caller queues signals */
	    if (!(pprog = patcompile(pat, PAT_STATIC, NULL))) {
		zwarnnam(nam, "bad pattern: %s", *names);
		close(dfd);
		unlink(dump);
		return 1;
	    }
	    for (i = 0; i < shfunctab->hsize; i++)
		for (hn = shfunctab->nodes[i]; hn; hn = hn->next)
		    if (!linknodebydatum(lnames, hn->nam) &&
			pattry(pprog, hn->nam) &&
			cur_add_func(nam, (Shfunc) hn, lnames, progs,
				     &hlen, &tlen, what)) {
			errflag &= ~ERRFLAG_ERROR;
			close(dfd);
			unlink(dump);
			return 1;
		    }
	}
    } else {
	for (; *names; names++) {
	    if (errflag ||
		!(shf = (Shfunc) shfunctab->getnode(shfunctab, *names))) {
		zwarnnam(nam, "unknown function: %s", *names);
		errflag &= ~ERRFLAG_ERROR;
		close(dfd);
		unlink(dump);
		return 1;
	    }
	    if (cur_add_func(nam, shf, lnames, progs, &hlen, &tlen, what)) {
		errflag &= ~ERRFLAG_ERROR;
		close(dfd);
		unlink(dump);
		return 1;
	    }
	}
    }
    if (empty(progs)) {
	zwarnnam(nam, "no functions");
	errflag &= ~ERRFLAG_ERROR;
	close(dfd);
	unlink(dump);
	return 1;
    }
    tlen = (tlen + hlen) * sizeof(wordcode);

    write_dump(dfd, progs, map, hlen, tlen);

    close(dfd);

    return 0;
}

/**/
#if defined(HAVE_SYS_MMAN_H) && defined(HAVE_MMAP) && defined(HAVE_MUNMAP)

#include <sys/mman.h>

/**/
#if defined(MAP_SHARED) && defined(PROT_READ)

/**/
#define USE_MMAP 1

/**/
#endif
/**/
#endif

/**/
#ifdef USE_MMAP

/* List of dump files mapped. */

static FuncDump dumps;

/**/
static int
zwcstat(char *filename, struct stat *buf)
{
    if (stat(filename, buf)) {
#ifdef HAVE_FSTAT
        FuncDump f;
    
	for (f = dumps; f; f = f->next) {
	    if (!strncmp(filename, f->filename, strlen(f->filename)) &&
		!fstat(f->fd, buf))
		return 0;
	}
#endif
	return 1;
    } else return 0;
}

/* Load a dump file (i.e. map it). */

static void
load_dump_file(char *dump, struct stat *sbuf, int other, int len)
{
    FuncDump d;
    Wordcode addr;
    int fd, off, mlen;

    if (other) {
	static size_t pgsz = 0;

	if (!pgsz) {

#ifdef _SC_PAGESIZE
	    pgsz = sysconf(_SC_PAGESIZE);     /* SVR4 */
#else
# ifdef _SC_PAGE_SIZE
	    pgsz = sysconf(_SC_PAGE_SIZE);    /* HPUX */
# else
	    pgsz = getpagesize();
# endif
#endif

	    pgsz--;
	}
	off = len & ~pgsz;
        mlen = len + (len - off);
    } else {
	off = 0;
        mlen = len;
    }
    if ((fd = open(dump, O_RDONLY)) < 0)
	return;

    fd = movefd(fd);
    if (fd == -1)
	return;

    if ((addr = (Wordcode) mmap(NULL, mlen, PROT_READ, MAP_SHARED, fd, off)) ==
	((Wordcode) -1)) {
	close(fd);
	return;
    }
    d = (FuncDump) zalloc(sizeof(*d));
    d->next = dumps;
    dumps = d;
    d->dev = sbuf->st_dev;
    d->ino = sbuf->st_ino;
    d->fd = fd;
#ifdef FD_CLOEXEC
    fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif
    d->map = addr + (other ? (len - off) / sizeof(wordcode) : 0);
    d->addr = addr;
    d->len = len;
    d->count = 0;
    d->filename = ztrdup(dump);
}

#else

#define zwcstat(f, b) (!!stat(f, b))

/**/
#endif

/* Try to load a function from one of the possible wordcode files for it.
 * The first argument is a element of $fpath, the second one is the name
 * of the function searched and the last one is the possible name for the
 * uncompiled function file (<path>/<func>). */

/**/
Eprog
try_dump_file(char *path, char *name, char *file, int *ksh, int test_only)
{
    Eprog prog;
    struct stat std, stc, stn;
    int rd, rc, rn;
    char *dig, *wc;

    if (strsfx(FD_EXT, path)) {
	queue_signals();
	prog = check_dump_file(path, NULL, name, ksh, test_only);
	unqueue_signals();
	return prog;
    }
    dig = dyncat(path, FD_EXT);
    wc = dyncat(file, FD_EXT);

    rd = zwcstat(dig, &std);
    rc = stat(wc, &stc);
    rn = stat(file, &stn);

    /* See if there is a digest file for the directory, it is younger than
     * both the uncompiled function file and its compiled version (or they
     * don't exist) and the digest file contains the definition for the
     * function. */
    queue_signals();
    if (!rd &&
	(rc || std.st_mtime >= stc.st_mtime) &&
	(rn || std.st_mtime >= stn.st_mtime) &&
	(prog = check_dump_file(dig, &std, name, ksh, test_only))) {
	unqueue_signals();
	return prog;
    }
    /* No digest file. Now look for the per-function compiled file. */
    if (!rc &&
	(rn || stc.st_mtime >= stn.st_mtime) &&
	(prog = check_dump_file(wc, &stc, name, ksh, test_only))) {
	unqueue_signals();
	return prog;
    }
    /* No compiled file for the function. The caller (getfpfunc() will
     * check if the directory contains the uncompiled file for it. */
    unqueue_signals();
    return NULL;
}

/* Almost the same, but for sourced files. */

/**/
Eprog
try_source_file(char *file)
{
    Eprog prog;
    struct stat stc, stn;
    int rc, rn;
    char *wc, *tail;

    if ((tail = strrchr(file, '/')))
	tail++;
    else
	tail = file;

    if (strsfx(FD_EXT, file)) {
	queue_signals();
	prog = check_dump_file(file, NULL, tail, NULL, 0);
	unqueue_signals();
	return prog;
    }
    wc = dyncat(file, FD_EXT);

    rc = stat(wc, &stc);
    rn = stat(file, &stn);

    queue_signals();
    if (!rc && (rn || stc.st_mtime >= stn.st_mtime) &&
	(prog = check_dump_file(wc, &stc, tail, NULL, 0))) {
	unqueue_signals();
	return prog;
    }
    unqueue_signals();
    return NULL;
}

/* See if `file' names a wordcode dump file and that contains the
 * definition for the function `name'. If so, return an eprog for it. */

/**/
static Eprog
check_dump_file(char *file, struct stat *sbuf, char *name, int *ksh,
		int test_only)
{
    int isrec = 0;
    Wordcode d;
    FDHead h;
    FuncDump f;
    struct stat lsbuf;

    if (!sbuf) {
	if (zwcstat(file, &lsbuf))
	    return NULL;
	sbuf = &lsbuf;
    }

#ifdef USE_MMAP

 rec:

#endif

    d = NULL;

#ifdef USE_MMAP

    for (f = dumps; f; f = f->next)
	if (f->dev == sbuf->st_dev && f->ino == sbuf->st_ino) {
	    d = f->map;
	    break;
	}

#else

    f = NULL;

#endif

    if (!f && (isrec || !(d = load_dump_header(NULL, file, 0))))
	return NULL;

    if ((h = dump_find_func(d, name))) {
	/* Found the name. If the file is already mapped, return the eprog,
	 * otherwise map it and just go up. */
	if (test_only)
	{
	    /* This is all we need.  Just return dummy. */
	    return &dummy_eprog;
	}

#ifdef USE_MMAP

	if (f) {
	    Eprog prog = (Eprog) zalloc(sizeof(*prog));
	    Patprog *pp;
	    int np;

	    prog->flags = EF_MAP;
	    prog->len = h->len;
	    prog->npats = np = h->npats;
	    prog->nref = 1;	/* allocated from permanent storage */
	    prog->pats = pp = (Patprog *) zalloc(np * sizeof(Patprog));
	    prog->prog = f->map + h->start;
	    prog->strs = ((char *) prog->prog) + h->strs;
	    prog->shf = NULL;
	    prog->dump = f;

	    incrdumpcount(f);

	    while (np--)
		*pp++ = dummy_patprog1;

	    if (ksh)
		*ksh = ((fdhflags(h) & FDHF_KSHLOAD) ? 2 :
			((fdhflags(h) & FDHF_ZSHLOAD) ? 0 : 1));

	    return prog;
	} else if (fdflags(d) & FDF_MAP) {
	    load_dump_file(file, sbuf, (fdflags(d) & FDF_OTHER), fdother(d));
	    isrec = 1;
	    goto rec;
	} else

#endif

	{
	    Eprog prog;
	    Patprog *pp;
	    int np, fd, po = h->npats * sizeof(Patprog);

	    if ((fd = open(file, O_RDONLY)) < 0 ||
		lseek(fd, ((h->start * sizeof(wordcode)) +
			   ((fdflags(d) & FDF_OTHER) ? fdother(d) : 0)), 0) < 0) {
		if (fd >= 0)
		    close(fd);
		return NULL;
	    }
	    d = (Wordcode) zalloc(h->len + po);

	    if (read(fd, ((char *) d) + po, h->len) != (int)h->len) {
		close(fd);
		zfree(d, h->len);

		return NULL;
	    }
	    close(fd);

	    prog = (Eprog) zalloc(sizeof(*prog));

	    prog->flags = EF_REAL;
	    prog->len = h->len + po;
	    prog->npats = np = h->npats;
	    prog->nref = 1; /* allocated from permanent storage */
	    prog->pats = pp = (Patprog *) d;
	    prog->prog = (Wordcode) (((char *) d) + po);
	    prog->strs = ((char *) prog->prog) + h->strs;
	    prog->shf = NULL;
	    prog->dump = f;

	    while (np--)
		*pp++ = dummy_patprog1;

	    if (ksh)
		*ksh = ((fdhflags(h) & FDHF_KSHLOAD) ? 2 :
			((fdhflags(h) & FDHF_ZSHLOAD) ? 0 : 1));

	    return prog;
	}
    }
    return NULL;
}

#ifdef USE_MMAP

/* Increment the reference counter for a dump file. */

/**/
void
incrdumpcount(FuncDump f)
{
    f->count++;
}

/**/
static void
freedump(FuncDump f)
{
    munmap((void *) f->addr, f->len);
    zclose(f->fd);
    zsfree(f->filename);
    zfree(f, sizeof(*f));
}

/* Decrement the reference counter for a dump file. If zero, unmap the file. */

/**/
void
decrdumpcount(FuncDump f)
{
    f->count--;
    if (!f->count) {
	FuncDump p, q;

	for (q = NULL, p = dumps; p && p != f; q = p, p = p->next);
	if (p) {
	    if (q)
		q->next = p->next;
	    else
		dumps = p->next;
	    freedump(f);
	}
    }
}

#ifndef FD_CLOEXEC
/**/
mod_export void
closedumps(void)
{
    while (dumps) {
	FuncDump p = dumps->next;
	freedump(dumps);
	dumps = p;
    }
}
#endif

#else

void
incrdumpcount(FuncDump f)
{
}

void
decrdumpcount(FuncDump f)
{
}

#ifndef FD_CLOEXEC
/**/
mod_export void
closedumps(void)
{
}
#endif

#endif

/**/
int
dump_autoload(char *nam, char *file, int on, Options ops, int func)
{
    Wordcode h;
    FDHead n, e;
    Shfunc shf;
    int ret = 0;

    if (!strsfx(FD_EXT, file))
	file = dyncat(file, FD_EXT);

    if (!(h = load_dump_header(nam, file, 1)))
	return 1;

    for (n = firstfdhead(h), e = (FDHead) (h + fdheaderlen(h)); n < e;
	 n = nextfdhead(n)) {
	shf = (Shfunc) zshcalloc(sizeof *shf);
	shf->node.flags = on;
	shf->funcdef = mkautofn(shf);
	shf->sticky = NULL;
	shfunctab->addnode(shfunctab, ztrdup(fdname(n) + fdhtail(n)), shf);
	if (OPT_ISSET(ops,'X') && eval_autoload(shf, shf->node.nam, ops, func))
	    ret = 1;
    }
    return ret;
}
