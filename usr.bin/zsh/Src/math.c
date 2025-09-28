/*
 * math.c - mathematical expression evaluation
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

struct mathvalue;

#include "zsh.mdh"
#include "math.pro"

#include <math.h>

/* nonzero means we are not evaluating, just parsing */
 
/**/
int noeval;
 
/* integer zero */

/**/
mod_export mnumber zero_mnumber;

/*
 * The last value we computed:  note this isn't cleared
 * until the next computation, unlike yyval.
 * Everything else is saved and returned to allow recursive calls.
 */
/**/
mnumber lastmathval;

/* last input base we used */

/**/
int lastbase;
 
static char *ptr;

static mnumber yyval;
static char *yylval;

#define MAX_MLEVEL 256

static int mlevel = 0;

/* != 0 means recognize unary plus, minus, etc. */

static int unary = 1;

/* LR = left-to-right associativity *
 * RL = right-to-left associativity *
 * BOOL = short-circuiting boolean   */

#define LR   0x0000
#define RL   0x0001
#define BOOL 0x0002

#define MTYPE(x)  ((x) & 3)

/*
 * OP_A2    2 arguments
 * OP_A2IR  2 arguments, return integer
 * OP_A2IO  2 arguments, must be integer, return integer
 * OP_E2    2 arguments with assignment
 * OP_E2IO  2 arguments with assignment, must be integer, return integer
 * OP_OP    None of the above, but occurs where we are expecting an operator
 *          rather than an operand.
 * OP_OPF   Followed by an operator, not an operand.
 *
 * OP_A2*, OP_E2*, OP_OP*:
 *   Occur when we need an operator; the next object must be an operand,
 *   unless OP_OPF is also supplied.
 *
 * Others:
 *   Occur when we need an operand; the next object must also be an operand,
 *   unless OP_OPF is also supplied.
 */
#define OP_A2   0x0004
#define OP_A2IR 0x0008
#define OP_A2IO 0x0010
#define OP_E2   0x0020
#define OP_E2IO 0x0040
#define OP_OP   0x0080
#define OP_OPF  0x0100

#define M_INPAR 0
#define M_OUTPAR 1
#define NOT 2
#define COMP 3
#define POSTPLUS 4
#define POSTMINUS 5
#define UPLUS 6
#define UMINUS 7
#define AND 8
#define XOR 9
#define OR 10
#define MUL 11
#define DIV 12
#define MOD 13
#define PLUS 14
#define MINUS 15
#define SHLEFT 16
#define SHRIGHT 17
#define LES 18
#define LEQ 19
#define GRE 20
#define GEQ 21
#define DEQ 22
#define NEQ 23
#define DAND 24
#define DOR 25
#define DXOR 26
#define QUEST 27
#define COLON 28
#define EQ 29
#define PLUSEQ 30
#define MINUSEQ 31
#define MULEQ 32
#define DIVEQ 33
#define MODEQ 34
#define ANDEQ 35
#define XOREQ 36
#define OREQ 37
#define SHLEFTEQ 38
#define SHRIGHTEQ 39
#define DANDEQ 40
#define DOREQ 41
#define DXOREQ 42
#define COMMA 43
#define EOI 44
#define PREPLUS 45
#define PREMINUS 46
#define NUM 47
#define ID 48
#define POWER 49
#define CID 50
#define POWEREQ 51
#define FUNC 52
#define TOKCOUNT 53

/*
 * Operator precedences: in reverse order, i.e. lower number, high precedence.
 * These are the C precedences.
 *
 * 0   Non-operators: NUM (numeric constant), ID (identifier),
 *                    CID (identifier with '#'), FUNC (math function)
 * 1   Opening parenthesis: M_INPAR '('  (for convenience, not an operator)
 * 2   Unary operators: PREPLUS/POSTPLUS '++', PREMINUS/POSTMINUS '--',
 *                      NOT '!', COMP '~', UPLUS '+', UMINUS '-'
 * 3   POWER '**' (not in C but at high precedence in Perl)
 * 4   MUL '*', DIV '/', MOD '%'
 * 5   PLUS '+', MINUS '-'
 * 6   SHLEFT '<<', SHRIGHT '>>'
 * 7   GRE '>', 'GEQ' '>=', LES '<', LEQ '<='
 * 8   DEQ '==', NEQ '!='
 * 9   AND '&'
 * 10  XOR '^'
 * 11  OR  '|'
 * 12  DAND '&&'
 * 13  DXOR '^^' (not in C)
 * 14  DOR '||'
 * 15  QUEST '?'
 * 16  COLON ':'
 * 17  EQ '=', PLUSEQ '+=', MINUSEQ '-=', MULEQ '*=', DIVEQ '/=',
 *     MODEQ '%=', ANDEQ '&=', XOREQ '^=', OREQ '|=',
 *     SHFLEFTEQ '<<=', SHRIGHTEQ '>>=', DANDEQ '&&=', DOREQ '||=',
 *     DXOREQ '^^='
 * 18 COMMA ','
 * 137 M_OUTPAR ')' (for convenience, not an operator)
 * 200 EOI (end of input:  for convenience, not an operator)
 */
static int c_prec[TOKCOUNT] =
{
/*        M_INPAR   M_OUTPAR     NOT       COMP     POSTPLUS */
/*  0 */     1,       137,        2,        2,         2,
/*        POSTMINUS   UPLUS     UMINUS     AND        XOR    */
/*  5 */     2,         2,        2,        9,        10,
/*          OR         MUL       DIV       MOD       PLUS    */
/* 10 */    11,         4,        4,        4,         5,
/*         MINUS      SHLEFT   SHRIGHT     LES        LEQ    */
/* 15 */     5,         6,        6,        7,         7,
/*          GRE        GEQ       DEQ       NEQ       DAND    */
/* 20 */     7,         7,        8,        8,        12,
/*          DOR        DXOR     QUEST     COLON       EQ     */
/* 25 */    14,        13,       15,       16,        17,
/*         PLUSEQ    MINUSEQ    MULEQ     DIVEQ      MODEQ   */
/* 30 */    17,        17,       17,       17,        17,
/*         ANDEQ      XOREQ     OREQ    SHLEFTEQ   SHRIGHTEQ */
/* 35 */    17,        17,       17,       17,        17,
/*        DANDEQ      DOREQ    DXOREQ    COMMA       EOI     */
/* 40 */    17,        17,       17,       18,       200,
/*       PREPLUS    PREMINUS     NUM        ID       POWER   */
/* 45 */     2,         2,        0,        0,         3,
/*          CID      POWEREQ     FUNC  */
/* 50 */     0,        17,        0
};

/*
 * Operator precedences: in reverse order, i.e. lower number, high precedence.
 * These are the default zsh precedences.
 *
 * 0   Non-operators: NUM (numeric constant), ID (identifier),
 *                    CID (identifier with '#'), FUNC (math function)
 * 1   Opening parenthesis: M_INPAR '('  (for convenience, not an operator)
 * 2   Unary operators: PREPLUS/POSTPLUS '++', PREMINUS/POSTMINUS '--',
 *                      NOT '!', COMP '~', UPLUS '+', UMINUS '-' 
 * 3   SHLEFT '<<', SHRIGHT '>>'
 * 4   AND '&'
 * 5   XOR '^'
 * 6   OR  '|'
 * 7   POWER '**' (not in C but at high precedence in Perl)
 * 8   MUL '*', DIV '/', MOD '%'
 * 9   PLUS '+', MINUS '-'
 * 10  GRE '>', 'GEQ' '>=', LES '<', LEQ '<='
 * 11  DEQ '==', NEQ '!='
 * 12  DAND '&&'
 * 13  DOR '||', DXOR '^^' (not in C)
 * 14  QUEST '?'
 * 15  COLON ':'
 * 16  EQ '=', PLUSEQ '+=', MINUSEQ '-=', MULEQ '*=', DIVEQ '/=',
 *     MODEQ '%=', ANDEQ '&=', XOREQ '^=', OREQ '|=',
 *     SHFLEFTEQ '<<=', SHRIGHTEQ '>>=', DANDEQ '&&=', DOREQ '||=',
 *     DXOREQ '^^='
 * 17 COMMA ','
 * 137 M_OUTPAR ')' (for convenience, not an operator)
 * 200 EOI (end of input:  for convenience, not an operator)
 */
static int z_prec[TOKCOUNT] =
{
/*        M_INPAR   M_OUTPAR     NOT       COMP     POSTPLUS */
/*  0 */     1,       137,        2,        2,         2,
/*        POSTMINUS   UPLUS     UMINUS     AND        XOR    */
/*  5 */     2,         2,        2,        4,         5,
/*          OR         MUL       DIV       MOD       PLUS    */
/* 10 */     6,         8,        8,        8,         9,
/*         MINUS      SHLEFT   SHRIGHT     LES        LEQ    */
/* 15 */     9,         3,        3,       10,        10,
/*          GRE        GEQ       DEQ       NEQ       DAND    */
/* 20 */    10,        10,       11,       11,        12,
/*          DOR        DXOR     QUEST     COLON       EQ     */
/* 25 */    13,        13,       14,       15,        16,
/*         PLUSEQ    MINUSEQ    MULEQ     DIVEQ      MODEQ   */
/* 30 */    16,        16,       16,       16,        16,
/*         ANDEQ      XOREQ     OREQ    SHLEFTEQ   SHRIGHTEQ */
/* 35 */    16,        16,       16,       16,        16,
/*        DANDEQ      DOREQ    DXOREQ    COMMA       EOI     */
/* 40 */    16,        16,       16,       17,       200,
/*       PREPLUS    PREMINUS     NUM        ID       POWER   */
/* 45 */     2,         2,        0,        0,         7,
/*          CID      POWEREQ     FUNC  */
/* 50 */     0,        16,        0
};

/* Option-selectable preference table */
static int *prec;

/*
 * Precedences for top and argument evaluation.  Careful:
 * prec needs to be set before we use these.
 */
#define TOPPREC (prec[COMMA]+1)
#define ARGPREC (prec[COMMA]-1)

static int type[TOKCOUNT] =
{
/*  0 */  LR, LR|OP_OP|OP_OPF, RL, RL, RL|OP_OP|OP_OPF,
/*  5 */  RL|OP_OP|OP_OPF, RL, RL, LR|OP_A2IO, LR|OP_A2IO,
/* 10 */  LR|OP_A2IO, LR|OP_A2, LR|OP_A2, LR|OP_A2, LR|OP_A2,
/* 15 */  LR|OP_A2, LR|OP_A2IO, LR|OP_A2IO, LR|OP_A2IR, LR|OP_A2IR,
/* 20 */  LR|OP_A2IR, LR|OP_A2IR, LR|OP_A2IR, LR|OP_A2IR, BOOL|OP_A2IO,
/* 25 */  BOOL|OP_A2IO, LR|OP_A2IO, RL|OP_OP, RL|OP_OP, RL|OP_E2,
/* 30 */  RL|OP_E2, RL|OP_E2, RL|OP_E2, RL|OP_E2, RL|OP_E2,
/* 35 */  RL|OP_E2IO, RL|OP_E2IO, RL|OP_E2IO, RL|OP_E2IO, RL|OP_E2IO,
/* 40 */  BOOL|OP_E2IO, BOOL|OP_E2IO, RL|OP_A2IO, RL|OP_A2, RL|OP_OP,
/* 45 */  RL, RL, LR|OP_OPF, LR|OP_OPF, RL|OP_A2,
/* 50 */  LR|OP_OPF, RL|OP_E2, LR|OP_OPF
};

/* the value stack */

#define STACKSZ 100
static int mtok;			/* last token */
static int sp = -1;			/* stack pointer */

struct mathvalue {
    /*
     * If we need to get a variable, this is the string to be passed
     * to the parameter code.  It may include a subscript.
     */
    char *lval;
    /*
     * If this is not zero, we've retrieved a variable and this
     * stores a reference to it.
     */
    Value pval;
    mnumber val;
};

static struct mathvalue *stack;

enum prec_type {
    /* Evaluating a top-level expression */
    MPREC_TOP,
    /* Evaluating a function argument */
    MPREC_ARG
};


/*
 * Get a number from a variable.
 * Try to be clever about reusing subscripts by caching the Value structure.
 */
static mnumber
getmathparam(struct mathvalue *mptr)
{
    mnumber result;
    if (!mptr->pval) {
	char *s = mptr->lval;
	mptr->pval = (Value)zhalloc(sizeof(struct value));
	if (!getvalue(mptr->pval, &s, 1))
	{
	    if (unset(UNSET))
		zerr("%s: parameter not set", mptr->lval);
	    mptr->pval = NULL;
	    if (isset(FORCEFLOAT)) {
		result.type = MN_FLOAT;
		result.u.d = 0.0;
		return result;
	    }
	    return zero_mnumber;
	}
    }
    result = getnumvalue(mptr->pval);
    if (isset(FORCEFLOAT) && result.type == MN_INTEGER) {
	result.type = MN_FLOAT;
	result.u.d = (double)result.u.l;
    }
    return result;
}

static mnumber
mathevall(char *s, enum prec_type prec_tp, char **ep)
{
    int xlastbase, xnoeval, xunary, *xprec;
    char *xptr;
    mnumber xyyval;
    char *xyylval;
    int xsp;
    struct mathvalue *xstack = 0, nstack[STACKSZ];
    mnumber ret;

    if (mlevel >= MAX_MLEVEL) {
	xyyval.type = MN_INTEGER;
	xyyval.u.l = 0;
	*ep = s;

	zerr("math recursion limit exceeded: %s", *ep);

	return xyyval;
    }
    if (mlevel++) {
	xlastbase = lastbase;
	xnoeval = noeval;
	xunary = unary;
	xptr = ptr;
	xyyval = yyval;
	xyylval = yylval;

	xsp = sp;
	xstack = stack;
	xprec = prec;
    } else {
	xlastbase = xnoeval = xunary = xsp = 0;
	xyyval.type = MN_INTEGER;
	xyyval.u.l = 0;
	xyylval = NULL;
	xptr = NULL;
	xprec = NULL;
    }
    prec = isset(CPRECEDENCES) ? c_prec : z_prec;
    stack = nstack;
    lastbase = -1;
    ptr = s;
    sp = -1;
    unary = 1;
    stack[0].val.type = MN_INTEGER;
    stack[0].val.u.l = 0;
    mathparse(prec_tp == MPREC_TOP ? TOPPREC : ARGPREC);
    /*
     * Internally, we parse the contents of parentheses at top
     * precedence... so we can return a parenthesis here if
     * there are too many at the end.
     */
    if (mtok == M_OUTPAR && !errflag)
	zerr("bad math expression: unexpected ')'");
    *ep = ptr;
    DPUTS(!errflag && sp > 0,
	  "BUG: math: wallabies roaming too freely in outback");

    if (errflag) {
	/*
	 * This used to set the return value to errflag.
	 * I don't understand how that could be useful; the
	 * caller doesn't know that's what's happened and
	 * may not get a value at all.
	 * Worse, we reset errflag in execarith() and setting
	 * this explicitly non-zero means a (( ... )) returns
	 * status 0 if there's an error.  That surely can't
	 * be right.  execarith() now detects an error and returns
	 * status 2.
	 */
	ret.type = MN_INTEGER;
	ret.u.l = 0;
    } else {
	if (stack[0].val.type == MN_UNSET)
	    ret = getmathparam(stack);
	else
	    ret = stack[0].val;
    }

    if (--mlevel) {
	lastbase = xlastbase;
	noeval = xnoeval;
	unary = xunary;
	ptr = xptr;
	yyval = xyyval;
	yylval = xyylval;

	sp = xsp;
	stack = xstack;
	prec = xprec;
    }
    return lastmathval = ret;
}

static int
lexconstant(void)
{
#ifdef USE_LOCALE
    char *prev_locale;
#endif
    char *nptr;

    nptr = ptr;
    if (IS_DASH(*nptr))
	nptr++;

    if (*nptr == '0') {
	int lowchar;
	nptr++;
	lowchar = tolower(*nptr);
	if (lowchar == 'x' || lowchar == 'b') {
	    /* Let zstrtol parse number with base */
	    yyval.u.l = zstrtol_underscore(ptr, &ptr, 0, 1);
	    /* Should we set lastbase here? */
	    lastbase = (lowchar == 'b') ? 2 : 16;
	    if (isset(FORCEFLOAT))
	    {
		yyval.type = MN_FLOAT;
		yyval.u.d = (double)yyval.u.l;
	    }
	    return NUM;
	}
	else if (isset(OCTALZEROES))
	{
	    char *ptr2;

	    /*
	     * Make sure this is a real octal constant;
	     * it can't be a base indication (always decimal)
	     * or a floating point number.
	     */
	    for (ptr2 = nptr; idigit(*ptr2) || *ptr2 == '_'; ptr2++)
		;

	    if (ptr2 > nptr && *ptr2 != '.' && *ptr2 != 'e' &&
		*ptr2 != 'E' && *ptr2 != '#')
	    {
		yyval.u.l = zstrtol_underscore(ptr, &ptr, 0, 1);
		lastbase = 8;
		if (isset(FORCEFLOAT))
		{
		    yyval.type = MN_FLOAT;
		    yyval.u.d = (double)yyval.u.l;
		}
		return NUM;
	    }
	    nptr = ptr2;
	}
    }
    while (idigit(*nptr) || *nptr == '_')
	nptr++;

    if (*nptr == '.' || *nptr == 'e' || *nptr == 'E') {
	char *ptr2;
	/* it's a float */
	yyval.type = MN_FLOAT;
#ifdef USE_LOCALE
	prev_locale = dupstring(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "POSIX");
#endif
	if (*nptr == '.') {
	    nptr++;
	    while (idigit(*nptr) || *nptr == '_')
		nptr++;
	}
	if (*nptr == 'e' || *nptr == 'E') {
	    nptr++;
	    if (*nptr == '+' || IS_DASH(*nptr))
		nptr++;
	    while (idigit(*nptr) || *nptr == '_')
		nptr++;
	}
	for (ptr2 = ptr; ptr2 < nptr; ptr2++) {
	    if (*ptr2 == '_') {
		int len = nptr - ptr;
		ptr = dupstring(ptr);
		for (ptr2 = ptr; len; len--) {
		    if (*ptr2 == '_')
			chuck(ptr2);
		    else
			ptr2++;
		}
		break;
	    }
	}
	yyval.u.d = strtod(ptr, &nptr);
#ifdef USE_LOCALE
	if (prev_locale) setlocale(LC_NUMERIC, prev_locale);
#endif
	if (ptr == nptr || *nptr == '.') {
	    zerr("bad floating point constant");
	    return EOI;
	}
	ptr = nptr;
    } else {
	/* it's an integer */
	yyval.u.l = zstrtol_underscore(ptr, &ptr, 10, 1);

	if (*ptr == '#') {
	    ptr++;
	    lastbase = yyval.u.l;
	    yyval.u.l = zstrtol_underscore(ptr, &ptr, lastbase, 1);
	}
	if (isset(FORCEFLOAT))
	{
	    yyval.type = MN_FLOAT;
	    yyval.u.d = (double)yyval.u.l;
	}
    }
    return NUM;
}

/**/
int outputradix;

/**/
int outputunderscore;

#ifndef HAVE_ISINF
/**/
int
isinf(double x)
{
    if ((-1.0 < x) && (x < 1.0))       /* x is small, and thus finite */
       return (0);
    else if ((x + x) == x)             /* only true if x == Infinity */
       return (1);
    else                               /* must be finite (normal or subnormal), or NaN */
       return (0);
}
#endif

#if !defined(HAVE_ISNAN)
static double
store(double *x)
{
    return (*x);
}

/**/
int
isnan(double x)
{
    /* (x != x) should be sufficient, but some compilers incorrectly optimize it away */
    return (store(&x) != store(&x));
}
#endif

/**/
static int
zzlex(void)
{
    int cct = 0;
    char *ie;
    yyval.type = MN_INTEGER;

    for (;; cct = 0)
	switch (*ptr++) {
	case '+':
	    if (*ptr == '+') {
		ptr++;
		return (unary) ? PREPLUS : POSTPLUS;
	    }
	    if (*ptr == '=') {
		ptr++;
		return PLUSEQ;
	    }
	    return (unary) ? UPLUS : PLUS;
	case '-':
	case Dash:
	    if (IS_DASH(*ptr)) {
		ptr++;
		return (unary) ? PREMINUS : POSTMINUS;
	    }
	    if (*ptr == '=') {
		ptr++;
		return MINUSEQ;
	    }
	    if (unary) {
		if (idigit(*ptr) || *ptr == '.') {
		    int ctype = lexconstant();
		    if (ctype == NUM)
		    {
			if (yyval.type == MN_FLOAT)
			{
			    yyval.u.d = -yyval.u.d;
			}
			else
			{
			    yyval.u.l = -yyval.u.l;
			}
		    }
		    return ctype;
		} else
		    return UMINUS;
	    } else
		return MINUS;
	case '(':
	    return M_INPAR;
	case ')':
	    return M_OUTPAR;
	case '!':
	    if (*ptr == '=') {
		ptr++;
		return NEQ;
	    }
	    return NOT;
	case '~':
	    return COMP;
	case '&':
	    if (*ptr == '&') {
		if (*++ptr == '=') {
		    ptr++;
		    return DANDEQ;
		}
		return DAND;
	    } else if (*ptr == '=') {
		ptr++;
		return ANDEQ;
	    }
	    return AND;
	case '|':
	    if (*ptr == '|') {
		if (*++ptr == '=') {
		    ptr++;
		    return DOREQ;
		}
		return DOR;
	    } else if (*ptr == '=') {
		ptr++;
		return OREQ;
	    }
	    return OR;
	case '^':
	    if (*ptr == '^') {
		if (*++ptr == '=') {
		    ptr++;
		    return DXOREQ;
		}
		return DXOR;
	    } else if (*ptr == '=') {
		ptr++;
		return XOREQ;
	    }
	    return XOR;
	case '*':
	    if (*ptr == '*') {
		if (*++ptr == '=') {
		    ptr++;
		    return POWEREQ;
		}
		return POWER;
	    }
	    if (*ptr == '=') {
		ptr++;
		return MULEQ;
	    }
	    return MUL;
	case '/':
	    if (*ptr == '=') {
		ptr++;
		return DIVEQ;
	    }
	    return DIV;
	case '%':
	    if (*ptr == '=') {
		ptr++;
		return MODEQ;
	    }
	    return MOD;
	case '<':
	    if (*ptr == '<') {
		if (*++ptr == '=') {
		    ptr++;
		    return SHLEFTEQ;
		}
		return SHLEFT;
	    } else if (*ptr == '=') {
		ptr++;
		return LEQ;
	    }
	    return LES;
	case '>':
	    if (*ptr == '>') {
		if (*++ptr == '=') {
		    ptr++;
		    return SHRIGHTEQ;
		}
		return SHRIGHT;
	    } else if (*ptr == '=') {
		ptr++;
		return GEQ;
	    }
	    return GRE;
	case '=':
	    if (*ptr == '=') {
		ptr++;
		return DEQ;
	    }
	    return EQ;
	case '$':
	    yyval.u.l = mypid;
	    return NUM;
	case '?':
	    if (unary) {
		yyval.u.l = lastval;
		return NUM;
	    }
	    return QUEST;
	case ':':
	    return COLON;
	case ',':
	    return COMMA;
	case '\0':
	    ptr--;
	    return EOI;
	case '[':
	    {
		int n, checkradix = 0;

		if (idigit(*ptr)) {
		    n = zstrtol(ptr, &ptr, 10);
		    if (*ptr != ']' || !idigit(*++ptr)) {
			zerr("bad base syntax");
			return EOI;
		    }
		    yyval.u.l = zstrtol(ptr, &ptr, lastbase = n);
		    return NUM;
		}
		if (*ptr == '#') {
		    n = 1;
		    if (*++ptr == '#') {
			n = -1;
			ptr++;
		    }
		    if (!idigit(*ptr) && *ptr != '_')
			goto bofs;
		    if (idigit(*ptr)) {
			outputradix = n * zstrtol(ptr, &ptr, 10);
			checkradix = 1;
		    }
		    if (*ptr == '_') {
			ptr++;
			if (idigit(*ptr))
			    outputunderscore = zstrtol(ptr, &ptr, 10);
			else
			    outputunderscore = 3;
		    }
		} else {
		    bofs:
		    zerr("bad output format specification");
		    return EOI;
		}
		if(*ptr != ']')
			goto bofs;
		if (checkradix) {
		    n = (outputradix < 0) ? -outputradix : outputradix;
		    if (n < 2 || n > 36) {
			zerr("invalid base (must be 2 to 36 inclusive): %d",
			     outputradix);
			return EOI;
		    }
		}
		ptr++;
		break;
	    }
	case ' ': /* Fall through! */
	case '\t':
	case '\n':
	case '"': /* POSIX says ignore these */
	case Dnull:
	    break;
	default:
	    if (idigit(*--ptr) || *ptr == '.')
		return lexconstant();
	    if (*ptr == '#') {
		if (*++ptr == '\\' || *ptr == '#') {
		    int v;
		    char *optr = ptr;

		    ptr++;
		    if (!*ptr) {
			zerr("bad math expression: character missing after ##");
			return EOI;
		    }
		    if(!(ptr = getkeystring(ptr, NULL, GETKEYS_MATH, &v))) {
			zerr("bad math expression: bad character after ##");
			ptr = optr;
			return EOI;
		    }
		    yyval.u.l = v;
		    return NUM;
		}
		cct = 1;
	    }
	    if ((ie = itype_end(ptr, IIDENT, 0)) != ptr) {
		int func = 0;
		char *p;

		p = ptr;
		ptr = ie;
		if (ie - p == 3 && !EMULATION(EMULATE_SH)) {
		    if ((p[0] == 'N' || p[0] == 'n') &&
			(p[1] == 'A' || p[1] == 'a') &&
			(p[2] == 'N' || p[2] == 'n')) {
			yyval.type = MN_FLOAT;
			yyval.u.d = 0.0;
			yyval.u.d /= yyval.u.d;
			return NUM;
		    }
		    else if ((p[0] == 'I' || p[0] == 'i') &&
			     (p[1] == 'N' || p[1] == 'n') &&
			     (p[2] == 'F' || p[2] == 'f')) {
			yyval.type = MN_FLOAT;
			yyval.u.d = 0.0;
			yyval.u.d = 1.0 / yyval.u.d;
			return NUM;
		    }
		}
		if (*ptr == '[' || (!cct && *ptr == '(')) {
		    char op = *ptr, cp = ((*ptr == '[') ? ']' : ')');
		    int l;
		    func = (op == '(');
		    for (ptr++, l = 1; *ptr && l; ptr++) {
			if (*ptr == op)
			    l++;
			if (*ptr == cp)
			    l--;
			if (*ptr == '\\' && ptr[1])
			    ptr++;
		    }
		}
		yylval = dupstrpfx(p, ptr - p);
		return (func ? FUNC : (cct ? CID : ID));
	    }
	    else if (cct) {
		yyval.u.l = poundgetfn(NULL);
		return NUM;
	    }
	    return EOI;
	}
}

/**/
static void
push(mnumber val, char *lval, int getme)
{
    if (sp == STACKSZ - 1)
	zerr("stack overflow");
    else
	sp++;
    stack[sp].val = val;
    stack[sp].lval = lval;
    stack[sp].pval = NULL;
    if (getme)
	stack[sp].val.type = MN_UNSET;
}

/**/
static mnumber
pop(int noget)
{
    struct mathvalue *mv = stack+sp;

    if (mv->val.type == MN_UNSET && !noget)
	mv->val = getmathparam(mv);
    sp--;
    return errflag ? zero_mnumber : mv->val;
}

/**/
static mnumber
getcvar(char *s)
{
    char *t;
    mnumber mn;
    mn.type = MN_INTEGER;

    queue_signals();
    if (!(t = getsparam(s)))
	mn.u.l = 0;
    else {
#ifdef MULTIBYTE_SUPPORT
	if (isset(MULTIBYTE)) {
	    wint_t wc;
	    (void)mb_metacharlenconv(t, &wc);
	    if (wc != WEOF) {
		mn.u.l = (zlong)wc;
		unqueue_signals();
		return mn;
	    }
	}
#endif
	mn.u.l = STOUC(*t == Meta ? t[1] ^ 32 : *t);
    }
    unqueue_signals();
    return mn;
}

/**/
static mnumber
setmathvar(struct mathvalue *mvp, mnumber v)
{
    Param pm;

    if (mvp->pval) {
	/*
	 * This value may have been hanging around for a while.
	 * Be ultra-paranoid in checking the variable is still valid.
	 */
	char *s = mvp->lval, *ptr;
	Param pm;
	DPUTS(!mvp->lval, "no variable name but variable value in math");
	if ((ptr = strchr(s, '[')))
	    s = dupstrpfx(s, ptr - s);
	pm = (Param) paramtab->getnode(paramtab, s);
	if (pm == mvp->pval->pm) {
	    if (noeval)
		return v;
	    setnumvalue(mvp->pval, v);
	    return v;
	}
	/* Different parameter, start again from scratch */
	mvp->pval = NULL;
    }
    if (!mvp->lval) {
	zerr("bad math expression: lvalue required");
	v.type = MN_INTEGER;
	v.u.l = 0;
	return v;
    }
    if (noeval)
	return v;
    untokenize(mvp->lval);
    pm = setnparam(mvp->lval, v);
    if (pm) {
	/*
	 * If we are performing an assignment, we return the
	 * number with the same type as the parameter we are
	 * assigning to, in the spirit of the way assignments
	 * in C work.  Note this was a change to long-standing
	 * zsh behaviour.
	 */
	switch (PM_TYPE(pm->node.flags)) {
	case PM_INTEGER:
	    if (v.type != MN_INTEGER) {
		v.u.l = (zlong)v.u.d;
		v.type = MN_INTEGER;
	    }
	    break;

	case PM_EFLOAT:
	case PM_FFLOAT:
	    if (v.type != MN_FLOAT) {
		v.u.d = (double)v.u.l;
		v.type = MN_FLOAT;
	    }
	    break;
	}
    }
    return v;
}


/**/
static mnumber
callmathfunc(char *o)
{
    MathFunc f;
    char *a, *n;
    static mnumber dummy;

    n = a = dupstring(o);

    while (*a != '(')
	a++;
    *a++ = '\0';
    a[strlen(a) - 1] = '\0';

    if ((f = getmathfunc(n, 1))) {
	if ((f->flags & (MFF_STR|MFF_USERFUNC)) == MFF_STR) {
	    return f->sfunc(n, a, f->funcid);
	} else {
	    int argc = 0;
	    mnumber *argv = NULL, *q, marg;
	    LinkList l = newlinklist();
	    LinkNode node;

	    if (f->flags & MFF_USERFUNC) {
		/* first argument is function name: always use mathfunc */
		addlinknode(l, n);
	    }

	    if (f->flags & MFF_STR) {
		if (!*a) {
		    addlinknode(l, dupstring(""));
		    argc++;
		}
	    } else {
		while (iblank(*a))
		    a++;
	    }
	    while (*a) {
		if (*a) {
		    argc++;
		    if (f->flags & MFF_USERFUNC) {
			/* need to pass strings */
			char *str;
			if (f->flags & MFF_STR) {
			    str = dupstring(a);
			    a = "";
			} else {
			    marg = mathevall(a, MPREC_ARG, &a);
			    if (marg.type & MN_FLOAT) {
				/* convfloat is off the heap */
				str = convfloat(marg.u.d, 0, 0, NULL);
			    } else {
				char buf[BDIGBUFSIZE];
				convbase(buf, marg.u.l, 10);
				str = dupstring(buf);
			    }
			}
			addlinknode(l, str);
		    } else {
			q = (mnumber *) zhalloc(sizeof(mnumber));
			*q = mathevall(a, MPREC_ARG, &a);
			addlinknode(l, q);
		    }
		    if (errflag || mtok != COMMA)
			break;
		}
	    }
	    if (*a && !errflag)
		zerr("bad math expression: illegal character: %c", *a);
	    if (!errflag) {
		if (argc >= f->minargs && (f->maxargs < 0 ||
					   argc <= f->maxargs)) {
		    if (f->flags & MFF_USERFUNC) {
			char *shfnam = f->module ? f->module : n;
			Shfunc shfunc = getshfunc(shfnam);
			if (!shfunc)
			    zerr("no such function: %s", shfnam);
			else {
			    doshfunc(shfunc, l, 1);
			    return lastmathval;
			}
		    } else {
			if (argc) {
			    q = argv =
				(mnumber *)zhalloc(argc * sizeof(mnumber));
			    for (node = firstnode(l); node; incnode(node))
				*q++ = *(mnumber *)getdata(node);
			}
			return f->nfunc(n, argc, argv, f->funcid);
		    }
		} else
		    zerr("wrong number of arguments: %s", o);
	    }
	}
    } else {
	zerr("unknown function: %s", n);
    }

    dummy.type = MN_INTEGER;
    dummy.u.l = 0;

    return dummy;
}

/**/
static int
notzero(mnumber a)
{
    if ((a.type & MN_INTEGER) && a.u.l == 0) {
        zerr("division by zero");
        return 0;
    }
    return 1;
}

/* macro to pop three values off the value stack */

static void
op(int what)
{
    mnumber a, b, c, *spval;
    int tp = type[what];

    if (errflag)
	return;
    if (sp < 0) {
	zerr("bad math expression: stack empty");
	return;
    }

    if (tp & (OP_A2|OP_A2IR|OP_A2IO|OP_E2|OP_E2IO)) {
	/* Make sure anyone seeing this message reports it. */
	DPUTS(sp < 1, "BUG: math: not enough wallabies in outback.");
	b = pop(0);
	a = pop(what == EQ);
	if (errflag)
	    return;

	if (tp & (OP_A2IO|OP_E2IO)) {
	    /* coerce to integers */
	    if (a.type & MN_FLOAT) {
		a.type = MN_INTEGER;
		a.u.l = (zlong)a.u.d;
	    }
	    if (b.type & MN_FLOAT) {
		b.type = MN_INTEGER;
		b.u.l = (zlong)b.u.d;
	    }
	} else if (a.type != b.type && what != COMMA &&
		   (a.type != MN_UNSET || what != EQ)) {
	    /*
	     * Different types, so coerce to float.
	     * It may happen during an assignment that the LHS
	     * variable is actually an integer, but there's still
	     * no harm in doing the arithmetic in floating point;
	     * the assignment will do the correct conversion.
	     * This way, if the parameter is actually a scalar, but
	     * used to contain an integer, we can write a float into it.
	     */
	    if (a.type & MN_INTEGER) {
		a.type = MN_FLOAT;
		a.u.d = (double)a.u.l;
	    }
	    if (b.type & MN_INTEGER) {
		b.type = MN_FLOAT;
		b.u.d = (double)b.u.l;
	    }
	}

	if (noeval) {
	    c.type = MN_INTEGER;
	    c.u.l = 0;
	} else {
	    /*
	     * type for operation: usually same as operands, but e.g.
	     * (a == b) returns int.
	     */
	    c.type = (tp & OP_A2IR) ? MN_INTEGER : a.type;

	    switch(what) {
	    case AND:
	    case ANDEQ:
		c.u.l = a.u.l & b.u.l;
		break;
	    case XOR:
	    case XOREQ:
		c.u.l = a.u.l ^ b.u.l;
		break;
	    case OR:
	    case OREQ:
		c.u.l = a.u.l | b.u.l;
		break;
	    case MUL:
	    case MULEQ:
		if (c.type == MN_FLOAT)
		    c.u.d = a.u.d * b.u.d;
		else
		    c.u.l = a.u.l * b.u.l;
		break;
	    case DIV:
	    case DIVEQ:
		if (!notzero(b))
		    return;
		if (c.type == MN_FLOAT)
		    c.u.d = a.u.d / b.u.d;
		else {
		    /*
		     * Avoid exception when dividing the smallest
		     * negative integer by -1.  Always treat it the
		     * same as multiplication.  This still doesn't give
		     * numerically the right answer in two's complement,
		     * but treating both these in the same way seems
		     * reasonable.
		     */
		    if (b.u.l == -1)
			c.u.l = - a.u.l;
		    else
			c.u.l = a.u.l / b.u.l;
		}
		break;
	    case MOD:
	    case MODEQ:
		if (!notzero(b))
		    return;
		/*
		 * Avoid exception as above.
		 * Any integer mod -1 is the same as any integer mod 1
		 * i.e. zero.
		 */
		if (c.type == MN_FLOAT)
		    c.u.d = fmod(a.u.d, b.u.d);
		else if (b.u.l == -1)
		    c.u.l = 0;
		else
		    c.u.l = a.u.l % b.u.l;
		break;
	    case PLUS:
	    case PLUSEQ:
		if (c.type == MN_FLOAT)
		    c.u.d = a.u.d + b.u.d;
		else
		    c.u.l = a.u.l + b.u.l;
		break;
	    case MINUS:
	    case MINUSEQ:
		if (c.type == MN_FLOAT)
		    c.u.d = a.u.d - b.u.d;
		else
		    c.u.l = a.u.l - b.u.l;
		break;
	    case SHLEFT:
	    case SHLEFTEQ:
		c.u.l = a.u.l << b.u.l;
		break;
	    case SHRIGHT:
	    case SHRIGHTEQ:
		c.u.l = a.u.l >> b.u.l;
		break;
	    case LES:
		c.u.l = (zlong)
		    (a.type == MN_FLOAT ? (a.u.d < b.u.d) : (a.u.l < b.u.l));
		break;
	    case LEQ:
		c.u.l = (zlong)
		    (a.type == MN_FLOAT ? (a.u.d <= b.u.d) : (a.u.l <= b.u.l));
		break;
	    case GRE:
		c.u.l = (zlong)
		    (a.type == MN_FLOAT ? (a.u.d > b.u.d) : (a.u.l > b.u.l));
		break;
	    case GEQ:
		c.u.l = (zlong)
		    (a.type == MN_FLOAT ? (a.u.d >= b.u.d) : (a.u.l >= b.u.l));
		break;
	    case DEQ:
		c.u.l = (zlong)
		    (a.type == MN_FLOAT ? (a.u.d == b.u.d) : (a.u.l == b.u.l));
		break;
	    case NEQ:
		c.u.l = (zlong)
		    (a.type == MN_FLOAT ? (a.u.d != b.u.d) : (a.u.l != b.u.l));
		break;
	    case DAND:
	    case DANDEQ:
		c.u.l = (zlong)(a.u.l && b.u.l);
		break;
	    case DOR:
	    case DOREQ:
		c.u.l = (zlong)(a.u.l || b.u.l);
		break;
	    case DXOR:
	    case DXOREQ:
		c.u.l = (zlong)((a.u.l && !b.u.l) || (!a.u.l && b.u.l));
		break;
	    case COMMA:
		c = b;
		break;
	    case POWER:
	    case POWEREQ:
		if (c.type == MN_INTEGER && b.u.l < 0) {
		    /* produces a real result, so cast to real. */
		    a.type = b.type = c.type = MN_FLOAT;
		    a.u.d = (double) a.u.l;
		    b.u.d = (double) b.u.l;
		}
		if (c.type == MN_INTEGER) {
		    for (c.u.l = 1; b.u.l--; c.u.l *= a.u.l);
		} else {
		    if (b.u.d <= 0 && !notzero(a))
			return;
		    if (a.u.d < 0) {
			/* Error if (-num ** b) and b is not an integer */
			double tst = (double)(zlong)b.u.d;
			if (tst != b.u.d) {
			    zerr("bad math expression: imaginary power");
			    return;
			}
		    }
		    c.u.d = pow(a.u.d, b.u.d);
		}
		break;
	    case EQ:
		c = b;
		break;
	    }
	}
	if (tp & (OP_E2|OP_E2IO)) {
	    struct mathvalue *mvp = stack + sp + 1;
	    c = setmathvar(mvp, c);
	    push(c, mvp->lval, 0);
	} else
	    push(c,NULL, 0);
	return;
    }

    spval = &stack[sp].val;
    if (stack[sp].val.type == MN_UNSET)
	*spval = getmathparam(stack + sp);
    switch (what) {
    case NOT:
	if (spval->type & MN_FLOAT) {
	    spval->u.l = !spval->u.d;
	    spval->type = MN_INTEGER;
	} else
	    spval->u.l = !spval->u.l;
	break;
    case COMP:
	if (spval->type & MN_FLOAT) {
	    spval->u.l = ~((zlong)spval->u.d);
	    spval->type = MN_INTEGER;
	} else
	    spval->u.l = ~spval->u.l;
	break;
    case POSTPLUS:
	a = *spval;
	if (spval->type & MN_FLOAT)
	    a.u.d++;
	else
	    a.u.l++;
	(void)setmathvar(stack + sp, a);
	break;
    case POSTMINUS:
	a = *spval;
	if (spval->type & MN_FLOAT)
	    a.u.d--;
	else
	    a.u.l--;
	(void)setmathvar(stack + sp, a);
	break;
    case UPLUS:
	break;
    case UMINUS:
	if (spval->type & MN_FLOAT)
	    spval->u.d = -spval->u.d;
	else
	    spval->u.l = -spval->u.l;
	break;
    case QUEST:
	DPUTS(sp < 2, "BUG: math: three shall be the number of the counting.");
	c = pop(0);
	b = pop(0);
	a = pop(0);
	if (errflag)
	    return;
	/* b and c can stay different types in this case. */
	push(((a.type & MN_FLOAT) ? a.u.d : a.u.l) ? b : c, NULL, 0);
	break;
    case COLON:
	zerr("bad math expression: ':' without '?'");
	break;
    case PREPLUS:
	if (spval->type & MN_FLOAT)
	    spval->u.d++;
	else
	    spval->u.l++;
	setmathvar(stack + sp, *spval);
	break;
    case PREMINUS:
	if (spval->type & MN_FLOAT)
	    spval->u.d--;
	else
	    spval->u.l--;
	setmathvar(stack + sp, *spval);
	break;
    default:
	zerr("bad math expression: out of integers");
	return;
    }
    stack[sp].lval = NULL;
    stack[sp].pval = NULL;
}


/**/
static void
bop(int tk)
{
    mnumber *spval = &stack[sp].val;
    int tst;

    if (stack[sp].val.type == MN_UNSET)
	*spval = getmathparam(stack + sp);
    tst = (spval->type & MN_FLOAT) ? (zlong)spval->u.d : spval->u.l; 

    switch (tk) {
    case DAND:
    case DANDEQ:
	if (!tst)
	    noeval++;
	break;
    case DOR:
    case DOREQ:
	if (tst)
	    noeval++;
	break;
    };
}


/**/
mod_export mnumber
matheval(char *s)
{
    char *junk;
    mnumber x;
    int xmtok = mtok;
    /* maintain outputradix and outputunderscore across levels of evaluation */
    if (!mlevel)
	outputradix = outputunderscore = 0;

    if (*s == Nularg)
	s++;
    if (!*s) {
	x.type = MN_INTEGER;
	x.u.l = 0;
	return x;
    }
    x = mathevall(s, MPREC_TOP, &junk);
    mtok = xmtok;
    if (*junk)
	zerr("bad math expression: illegal character: %c", *junk);
    return x;
}

/**/
mod_export zlong
mathevali(char *s)
{
    mnumber x = matheval(s);
    return (x.type & MN_FLOAT) ? (zlong)x.u.d : x.u.l;
}


/**/
zlong
mathevalarg(char *s, char **ss)
{
    mnumber x;
    int xmtok = mtok;

    /*
     * At this entry point we don't allow an empty expression,
     * whereas we do with matheval().  I'm not sure if this
     * difference is deliberate, but it does mean that e.g.
     * $array[$ind] where ind hasn't been set produces an error,
     * which is probably safe.
     *
     * To avoid a more opaque error further in, bail out here.
     */
    if (*s == Nularg)
	s++;
    if (!*s) {
	zerr("bad math expression: empty string");
	return (zlong)0;
    }
    x = mathevall(s, MPREC_ARG, ss);
    if (mtok == COMMA)
	(*ss)--;
    mtok = xmtok;
    return (x.type & MN_FLOAT) ? (zlong)x.u.d : x.u.l;
}

/*
 * Make sure we have an operator or an operand, whatever is expected.
 * For this purpose, unary operators constitute part of an operand.
 */

/**/
static void
checkunary(int mtokc, char *mptr)
{
    int errmsg = 0;
    int tp = type[mtokc];
    if (tp & (OP_A2|OP_A2IR|OP_A2IO|OP_E2|OP_E2IO|OP_OP)) {
	if (unary)
	    errmsg = 1;
    } else {
	if (!unary)
	    errmsg = 2;
    }
    if (errmsg) {
	int len, over = 0;
	char *errtype = errmsg == 2 ? "operator" : "operand";
	while (inblank(*mptr))
	    mptr++;
	len = ztrlen(mptr);
	if (len > 10) {
	    len = 10;
	    over = 1;
	}
	if (!*mptr)
	    zerr("bad math expression: %s expected at end of string",
		errtype);
	else
	    zerr("bad math expression: %s expected at `%l%s'",
		 errtype, mptr, len, over ? "..." : "");
    }
    unary = !(tp & OP_OPF);
}

/* operator-precedence parse the string and execute */

/**/
static void
mathparse(int pc)
{
    zlong q;
    int otok, onoeval;
    char *optr = ptr;

    if (errflag)
	return;
    queue_signals();
    mtok = zzlex();
    /* Handle empty input */
    if (pc == TOPPREC && mtok == EOI) {
	unqueue_signals();
	return;
    }
    checkunary(mtok, optr);
    while (prec[mtok] <= pc) {
	if (errflag) {
	    unqueue_signals();
	    return;
	}
	switch (mtok) {
	case NUM:
	    push(yyval, NULL, 0);
	    break;
	case ID:
	    push(zero_mnumber, yylval, !noeval);
	    break;
	case CID:
	    push((noeval ? zero_mnumber : getcvar(yylval)), yylval, 0);
	    break;
	case FUNC:
	    push((noeval ? zero_mnumber : callmathfunc(yylval)), yylval, 0);
	    break;
	case M_INPAR:
	    mathparse(TOPPREC);
	    if (mtok != M_OUTPAR) {
		if (!errflag)
		    zerr("bad math expression: ')' expected");
		unqueue_signals();
		return;
	    }
	    break;
	case QUEST:
	    if (stack[sp].val.type == MN_UNSET)
		stack[sp].val = getmathparam(stack + sp);
	    q = (stack[sp].val.type == MN_FLOAT) ?
		(stack[sp].val.u.d == 0 ? 0 : 1) :
		stack[sp].val.u.l;

	    if (!q)
		noeval++;
	    mathparse(prec[COLON] - 1);
	    if (!q)
		noeval--;
	    if (mtok != COLON) {
		if (!errflag)
		    zerr("bad math expression: ':' expected");
		unqueue_signals();
		return;
	    }
	    if (q)
		noeval++;
	    mathparse(prec[QUEST]);
	    if (q)
		noeval--;
	    op(QUEST);
	    continue;
	default:
	    otok = mtok;
	    onoeval = noeval;
	    if (MTYPE(type[otok]) == BOOL)
		bop(otok);
	    mathparse(prec[otok] - (MTYPE(type[otok]) != RL));
	    noeval = onoeval;
	    op(otok);
	    continue;
	}
	optr = ptr;
	mtok = zzlex();
	checkunary(mtok, optr);
    }
    unqueue_signals();
}
