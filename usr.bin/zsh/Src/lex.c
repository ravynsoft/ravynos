/*
 * lex.c - lexical analysis
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
#include "lex.pro"

#define LEX_HEAP_SIZE (32)

/* tokens */

/**/
mod_export char ztokens[] = "#$^*(())$=|{}[]`<>>?~`,-!'\"\\\\";

/* parts of the current token */

/**/
char *zshlextext;
/**/
mod_export char *tokstr;
/**/
mod_export enum lextok tok;
/**/
mod_export int tokfd;

/*
 * Line number at which the first character of a token was found.
 * We always set this in gettok(), which is always called from
 * zshlex() unless we have reached an error.  So it is always
 * valid when parsing.  It is not useful during execution
 * of the parsed structure.
 */

/**/
zlong toklineno;

/* lexical analyzer error flag */
 
/**/
mod_export int lexstop;

/* if != 0, this is the first line of the command */
 
/**/
mod_export int isfirstln;
 
/* if != 0, this is the first char of the command (not including white space) */
 
/**/
int isfirstch;

/* flag that an alias should be expanded after expansion ending in space */

/**/
int inalmore;

/*
 * Don't do spelling correction.
 * Bit 1 is only valid for the current word.  It's
 * set when we detect a lookahead that stops the word from
 * needing correction.
 */
 
/**/
int nocorrect;

/*
 * TBD: the following exported variables are part of the non-interface
 * with ZLE for completion.  They are poorly named and the whole
 * scheme is incredibly brittle.  One piece of robustness is applied:
 * the variables are only set if LEXFLAGS_ZLE is set.  Improvements
 * should therefore concentrate on areas with this flag set.
 *
 * Cursor position and line length in zle when the line is
 * metafied for access from the main shell.
 */

/**/
mod_export int zlemetacs, zlemetall;

/* inwhat says what exactly we are in     *
 * (its value is one of the IN_* things). */

/**/
mod_export int inwhat;

/* 1 if x added to complete in a blank between words */

/**/
mod_export int addedx;

/* wb and we hold the beginning/end position of the word we are completing. */

/**/
mod_export int wb, we;

/**/
mod_export int wordbeg;

/**/
mod_export int parbegin;

/**/
mod_export int parend;


/* 1 if aliases should not be expanded */

/**/
mod_export int noaliases;

/*
 * If non-zero, we are parsing a line sent to use by the editor, or some
 * other string that's not part of standard command input (e.g. eval is
 * part of normal command input).
 *
 * Set of bits from LEXFLAGS_*.
 *
 * Note that although it is passed into the lexer as an input, the
 * lexer can set it to zero after finding the word it's searching for.
 * This only happens if the line being parsed actually does come from
 * ZLE, and hence the bit LEXFLAGS_ZLE is set.
 */

/**/
mod_export int lexflags;

/* don't recognize comments */

/**/
mod_export int nocomments;

/* add raw input characters while parsing command substitution */

/**/
int lex_add_raw;

/* variables associated with the above */

static char *tokstr_raw;
static struct lexbufstate lexbuf_raw;

/* text of punctuation tokens */

/**/
mod_export char *tokstrings[WHILE + 1] = {
    NULL,	/* NULLTOK	  0  */
    ";",	/* SEPER	     */
    "\\n",	/* NEWLIN	     */
    ";",	/* SEMI		     */
    ";;",	/* DSEMI	     */
    "&",	/* AMPER	  5  */
    "(",	/* INPAR	     */
    ")",	/* OUTPAR	     */
    "||",	/* DBAR		     */
    "&&",	/* DAMPER	     */
    ">",	/* OUTANG	  10 */
    ">|",	/* OUTANGBANG	     */
    ">>",	/* DOUTANG	     */
    ">>|",	/* DOUTANGBANG	     */
    "<",	/* INANG	     */
    "<>",	/* INOUTANG	  15 */
    "<<",	/* DINANG	     */
    "<<-",	/* DINANGDASH	     */
    "<&",	/* INANGAMP	     */
    ">&",	/* OUTANGAMP	     */
    "&>",	/* AMPOUTANG	  20 */
    "&>|",	/* OUTANGAMPBANG     */
    ">>&",	/* DOUTANGAMP	     */
    ">>&|",	/* DOUTANGAMPBANG    */
    "<<<",	/* TRINANG	     */
    "|",	/* BAR		  25 */
    "|&",	/* BARAMP	     */
    "()",	/* INOUTPAR	     */
    "((",	/* DINPAR	     */
    "))",	/* DOUTPAR	     */
    "&|",	/* AMPERBANG	  30 */
    ";&",	/* SEMIAMP	     */
    ";|",	/* SEMIBAR	     */
};

/* lexical state */

static int dbparens;
static struct lexbufstate lexbuf = { NULL, 256, 0 };

/* save lexical context */

/**/
void
lex_context_save(struct lex_stack *ls, int toplevel)
{
    (void)toplevel;

    ls->dbparens = dbparens;
    ls->isfirstln = isfirstln;
    ls->isfirstch = isfirstch;
    ls->lexflags = lexflags;

    ls->tok = tok;
    ls->tokstr = tokstr;
    ls->zshlextext = zshlextext;
    ls->lexbuf = lexbuf;
    ls->lex_add_raw = lex_add_raw;
    ls->tokstr_raw = tokstr_raw;
    ls->lexbuf_raw = lexbuf_raw;
    ls->lexstop = lexstop;
    ls->toklineno = toklineno;

    tokstr = zshlextext = lexbuf.ptr = NULL;
    lexbuf.siz = 256;
    tokstr_raw = lexbuf_raw.ptr = NULL;
    lexbuf_raw.siz = lexbuf_raw.len = lex_add_raw = 0;
}

/* restore lexical context */

/**/
mod_export void
lex_context_restore(const struct lex_stack *ls, int toplevel)
{
    (void)toplevel;

    dbparens = ls->dbparens;
    isfirstln = ls->isfirstln;
    isfirstch = ls->isfirstch;
    lexflags = ls->lexflags;
    tok = ls->tok;
    tokstr = ls->tokstr;
    zshlextext = ls->zshlextext;
    lexbuf = ls->lexbuf;
    lex_add_raw = ls->lex_add_raw;
    tokstr_raw = ls->tokstr_raw;
    lexbuf_raw = ls->lexbuf_raw;
    lexstop = ls->lexstop;
    toklineno = ls->toklineno;
}

/**/
void
zshlex(void)
{
    if (tok == LEXERR)
	return;
    do {
	if (inrepeat_)
	    ++inrepeat_;
	if (inrepeat_ == 3 && (isset(SHORTLOOPS) || isset(SHORTREPEAT)))
	    incmdpos = 1;
	tok = gettok();
    } while (tok != ENDINPUT && exalias());
    nocorrect &= 1;
    if (tok == NEWLIN || tok == ENDINPUT) {
	while (hdocs) {
	    struct heredocs *next = hdocs->next;
	    char *doc, *munged_term;

	    hwbegin(0);
	    cmdpush(hdocs->type == REDIR_HEREDOC ? CS_HEREDOC : CS_HEREDOCD);
	    munged_term = dupstring(hdocs->str);
	    STOPHIST
	    doc = gethere(&munged_term, hdocs->type);
	    ALLOWHIST
	    cmdpop();
	    hwend();
	    if (!doc) {
		zerr("here document too large");
		while (hdocs) {
		    next = hdocs->next;
		    zfree(hdocs, sizeof(struct heredocs));
		    hdocs = next;
		}
		tok = LEXERR;
		break;
	    }
	    setheredoc(hdocs->pc, REDIR_HERESTR, doc, hdocs->str,
		       munged_term);
	    zfree(hdocs, sizeof(struct heredocs));
	    hdocs = next;
	}
    }
    if (tok != NEWLIN)
	isnewlin = 0;
    else
	isnewlin = (inbufct) ? -1 : 1;
    if (tok == SEMI || (tok == NEWLIN && !(lexflags & LEXFLAGS_NEWLINE)))
	tok = SEPER;
}

/**/
mod_export void
ctxtlex(void)
{
    static int oldpos;

    zshlex();
    switch (tok) {
    case SEPER:
    case NEWLIN:
    case SEMI:
    case DSEMI:
    case SEMIAMP:
    case SEMIBAR:
    case AMPER:
    case AMPERBANG:
    case INPAR:
    case INBRACE:
    case DBAR:
    case DAMPER:
    case BAR:
    case BARAMP:
    case INOUTPAR:
    case DOLOOP:
    case THEN:
    case ELIF:
    case ELSE:
    case DOUTBRACK:
	incmdpos = 1;
	break;
    case STRING:
    case TYPESET:
 /* case ENVSTRING: */
    case ENVARRAY:
    case OUTPAR:
    case CASE:
    case DINBRACK:
	incmdpos = 0;
	break;

    default:
	/* nothing to do, keep compiler happy */
	break;
    }
    if (tok != DINPAR)
	infor = tok == FOR ? 2 : 0;
    if (IS_REDIROP(tok) || tok == FOR || tok == FOREACH || tok == SELECT) {
	inredir = 1;
	oldpos = incmdpos;
	incmdpos = 0;
    } else if (inredir) {
	incmdpos = oldpos;
	inredir = 0;
    }
}

#define LX1_BKSLASH 0
#define LX1_COMMENT 1
#define LX1_NEWLIN 2
#define LX1_SEMI 3
#define LX1_AMPER 5
#define LX1_BAR 6
#define LX1_INPAR 7
#define LX1_OUTPAR 8
#define LX1_INANG 13
#define LX1_OUTANG 14
#define LX1_OTHER 15

#define LX2_BREAK 0
#define LX2_OUTPAR 1
#define LX2_BAR 2
#define LX2_STRING 3
#define LX2_INBRACK 4
#define LX2_OUTBRACK 5
#define LX2_TILDE 6
#define LX2_INPAR 7
#define LX2_INBRACE 8
#define LX2_OUTBRACE 9
#define LX2_OUTANG 10
#define LX2_INANG 11
#define LX2_EQUALS 12
#define LX2_BKSLASH 13
#define LX2_QUOTE 14
#define LX2_DQUOTE 15
#define LX2_BQUOTE 16
#define LX2_COMMA 17
#define LX2_DASH 18
#define LX2_BANG 19
#define LX2_OTHER 20
#define LX2_META 21

static unsigned char lexact1[256], lexact2[256], lextok2[256];

/**/
void
initlextabs(void)
{
    int t0;
    static char *lx1 = "\\q\n;!&|(){}[]<>";
    static char *lx2 = ";)|$[]~({}><=\\\'\"`,-!";

    for (t0 = 0; t0 != 256; t0++) {
       lexact1[t0] = LX1_OTHER;
	lexact2[t0] = LX2_OTHER;
	lextok2[t0] = t0;
    }
    for (t0 = 0; lx1[t0]; t0++)
	lexact1[(int)lx1[t0]] = t0;
    for (t0 = 0; lx2[t0]; t0++)
	lexact2[(int)lx2[t0]] = t0;
    lexact2['&'] = LX2_BREAK;
    lexact2[STOUC(Meta)] = LX2_META;
    lextok2['*'] = Star;
    lextok2['?'] = Quest;
    lextok2['{'] = Inbrace;
    lextok2['['] = Inbrack;
    lextok2['$'] = String;
    lextok2['~'] = Tilde;
    lextok2['#'] = Pound;
    lextok2['^'] = Hat;
}

/* initialize lexical state */

/**/
void
lexinit(void)
{
    nocorrect = dbparens = lexstop = 0;
    tok = ENDINPUT;
}

/* add a char to the string buffer */

/**/
void
add(int c)
{
    *lexbuf.ptr++ = c;
    if (lexbuf.siz == ++lexbuf.len) {
	int newbsiz = lexbuf.siz * 2;

	if (newbsiz > inbufct && inbufct > lexbuf.siz)
	    newbsiz = inbufct;

	tokstr = (char *)hrealloc(tokstr, lexbuf.siz, newbsiz);
	lexbuf.ptr = tokstr + lexbuf.len;
	/* len == bsiz, so bptr is at the start of newly allocated memory */
	memset(lexbuf.ptr, 0, newbsiz - lexbuf.siz);
	lexbuf.siz = newbsiz;
    }
}

#define SETPARBEGIN {							\
	if ((lexflags & LEXFLAGS_ZLE) && !(inbufflags & INP_ALIAS) &&	\
	    zlemetacs >= zlemetall+1-inbufct)				\
	    parbegin = inbufct;		      \
    }
#define SETPAREND {						      \
	if ((lexflags & LEXFLAGS_ZLE) && !(inbufflags & INP_ALIAS) && \
	    parbegin != -1 && parend == -1) {			      \
	    if (zlemetacs >= zlemetall + 1 - inbufct)		      \
		parbegin = -1;					      \
	    else						      \
		parend = inbufct;				      \
	}							      \
    }

enum {
    CMD_OR_MATH_CMD,
    CMD_OR_MATH_MATH,
    CMD_OR_MATH_ERR
};

/*
 * Return one of the above.  If it couldn't be
 * parsed as math, but there was no gross error, it's a command.
 */

static int
cmd_or_math(int cs_type)
{
    int oldlen = lexbuf.len;
    int c;
    int oinflags = inbufflags;

    cmdpush(cs_type);
    inbufflags |= INP_APPEND;
    c = dquote_parse(')', 0);
    if (!(oinflags & INP_APPEND))
	inbufflags &= ~INP_APPEND;
    cmdpop();
    *lexbuf.ptr = '\0';
    if (!c) {
	/* Successfully parsed, see if it was math */
	c = hgetc();
	if (c == ')')
	    return CMD_OR_MATH_MATH; /* yes */
	hungetc(c);
	lexstop = 0;
	c = ')';
    } else if (lexstop) {
	/* we haven't got anything to unget */
	return CMD_OR_MATH_ERR;
    }
    /* else unsuccessful: unget the whole thing */
    hungetc(c);
    lexstop = 0;
    while (lexbuf.len > oldlen && !(errflag & ERRFLAG_ERROR)) {
	lexbuf.len--;
	hungetc(itok(*--lexbuf.ptr) ?
		ztokens[*lexbuf.ptr - Pound] : *lexbuf.ptr);
    }
    if (errflag)
	return CMD_OR_MATH_ERR;
    hungetc('(');
    return errflag ? CMD_OR_MATH_ERR : CMD_OR_MATH_CMD;
}


/*
 * Parse either a $(( ... )) or a $(...)
 * Return the same as cmd_or_math().
 */
static int
cmd_or_math_sub(void)
{
    int c = hgetc(), ret;

    if (c == '\\') {
	c = hgetc();
	if (c != '\n') {
	    hungetc(c);
	    hungetc('\\');
	    lexstop = 0;
	    return skipcomm() ? CMD_OR_MATH_ERR : CMD_OR_MATH_CMD;
	}
	c = hgetc();
    }

    if (c == '(') {
	int lexpos = (int)(lexbuf.ptr - tokstr);
	add(Inpar);
	add('(');
	if ((ret = cmd_or_math(CS_MATHSUBST)) == CMD_OR_MATH_MATH) {
	    tokstr[lexpos] = Inparmath;
	    add(')');
	    return CMD_OR_MATH_MATH;
	}
	if (ret == CMD_OR_MATH_ERR)
	    return CMD_OR_MATH_ERR;
	lexbuf.ptr -= 2;
	lexbuf.len -= 2;
    } else {
	hungetc(c);
	lexstop = 0;
    }
    return skipcomm() ? CMD_OR_MATH_ERR : CMD_OR_MATH_CMD;
}

/* Check whether we're looking at valid numeric globbing syntax      *
 * (/\<[0-9]*-[0-9]*\>/).  Call pointing just after the opening "<". *
 * Leaves the input in the same place, returning 0 or 1.             */

/**/
static int
isnumglob(void)
{
    int c, ec = '-', ret = 0;
    int tbs = 256, n = 0;
    char *tbuf = (char *)zalloc(tbs);

    while(1) {
	c = hgetc();
	if(lexstop) {
	    lexstop = 0;
	    break;
	}
	tbuf[n++] = c;
	if(!idigit(c)) {
	    if(c != ec)
		break;
	    if(ec == '>') {
		ret = 1;
		break;
	    }
	    ec = '>';
	}
	if(n == tbs)
	    tbuf = (char *)realloc(tbuf, tbs *= 2);
    }
    while(n--)
	hungetc(tbuf[n]);
    zfree(tbuf, tbs);
    return ret;
}

/**/
static enum lextok
gettok(void)
{
    int c, d;
    int peekfd = -1;
    enum lextok peek;

  beginning:
    tokstr = NULL;
    while (iblank(c = hgetc()) && !lexstop);
    toklineno = lineno;
    if (lexstop)
	return (errflag) ? LEXERR : ENDINPUT;
    isfirstln = 0;
    if ((lexflags & LEXFLAGS_ZLE) && !(inbufflags & INP_ALIAS))
	wordbeg = inbufct - (qbang && c == bangchar);
    hwbegin(-1-(qbang && c == bangchar));
    /* word includes the last character read and possibly \ before ! */
    if (dbparens) {
	lexbuf.len = 0;
	lexbuf.ptr = tokstr = (char *) hcalloc(lexbuf.siz = LEX_HEAP_SIZE);
	hungetc(c);
	cmdpush(CS_MATH);
	c = dquote_parse(infor ? ';' : ')', 0);
	cmdpop();
	*lexbuf.ptr = '\0';
	if (!c && infor) {
	    infor--;
	    return DINPAR;
	}
	if (c || (c = hgetc()) != ')') {
	    hungetc(c);
	    return LEXERR;
	}
	dbparens = 0;
	return DOUTPAR;
    } else if (idigit(c)) {	/* handle 1< foo */
	d = hgetc();
	if(d == '&') {
	    d = hgetc();
	    if(d == '>') {
		peekfd = c - '0';
		hungetc('>');
		c = '&';
	    } else {
		hungetc(d);
		lexstop = 0;
		hungetc('&');
	    }
	} else if (d == '>' || d == '<') {
	    peekfd = c - '0';
	    c = d;
	} else {
	    hungetc(d);
	    lexstop = 0;
	}
    }

    /* chars in initial position in word */

    /*
     * Handle comments.  There are some special cases when this
     * is not normal command input: lexflags implies we are examining
     * a line lexically without it being used for normal command input.
     */
    if (c == hashchar && !nocomments &&
	(isset(INTERACTIVECOMMENTS) ||
	 ((!lexflags || (lexflags & LEXFLAGS_COMMENTS)) && !expanding &&
	  (!interact || unset(SHINSTDIN) || strin)))) {
	/* History is handled here to prevent extra  *
	 * newlines being inserted into the history. */

	if (lexflags & LEXFLAGS_COMMENTS_KEEP) {
	    lexbuf.len = 0;
	    lexbuf.ptr = tokstr =
		(char *)hcalloc(lexbuf.siz = LEX_HEAP_SIZE);
	    add(c);
	}
	hwabort();
	while ((c = ingetc()) != '\n' && !lexstop) {
	    hwaddc(c);
	    addtoline(c);
	    if (lexflags & LEXFLAGS_COMMENTS_KEEP)
		add(c);
	}

	if (errflag)
	    peek = LEXERR;
	else {
	    if (lexflags & LEXFLAGS_COMMENTS_KEEP) {
		*lexbuf.ptr = '\0';
		if (!lexstop)
		    hungetc(c);
		peek = STRING;
	    } else {
		hwend();
		hwbegin(0);
		hwaddc('\n');
		addtoline('\n');
		/*
		 * If splitting a line and removing comments,
		 * we don't want a newline token since it's
		 * treated specially.
		 */
		if ((lexflags & LEXFLAGS_COMMENTS_STRIP) && lexstop)
		    peek = ENDINPUT;
		else
		    peek = NEWLIN;
	    }
	}
	return peek;
    }
    switch (lexact1[STOUC(c)]) {
    case LX1_BKSLASH:
	d = hgetc();
	if (d == '\n')
	    goto beginning;
	hungetc(d);
	lexstop = 0;
	break;
    case LX1_NEWLIN:
	return NEWLIN;
    case LX1_SEMI:
	d = hgetc();
	if(d == ';')
	    return DSEMI;
	else if(d == '&')
	    return SEMIAMP;
	else if (d == '|')
	    return SEMIBAR;
	hungetc(d);
	lexstop = 0;
	return SEMI;
    case LX1_AMPER:
	d = hgetc();
	if (d == '&')
	    return DAMPER;
	else if (d == '!' || d == '|')
	    return AMPERBANG;
	else if (d == '>') {
	    tokfd = peekfd;
	    d = hgetc();
	    if (d == '!' || d == '|')
		return OUTANGAMPBANG;
	    else if (d == '>') {
		d = hgetc();
		if (d == '!' || d == '|')
		    return DOUTANGAMPBANG;
		hungetc(d);
		lexstop = 0;
		return DOUTANGAMP;
	    }
	    hungetc(d);
	    lexstop = 0;
	    return AMPOUTANG;
	}
	hungetc(d);
	lexstop = 0;
	return AMPER;
    case LX1_BAR:
	d = hgetc();
	if (d == '|' && !incasepat)
	    return DBAR;
	else if (d == '&')
	    return BARAMP;
	hungetc(d);
	lexstop = 0;
	return BAR;
    case LX1_INPAR:
	d = hgetc();
	if (d == '(') {
	    if (infor) {
		dbparens = 1;
		return DINPAR;
	    }
	    if (incmdpos || (isset(SHGLOB) && !isset(KSHGLOB))) {
		lexbuf.len = 0;
		lexbuf.ptr = tokstr = (char *)
		    hcalloc(lexbuf.siz = LEX_HEAP_SIZE);
		switch (cmd_or_math(CS_MATH)) {
		case CMD_OR_MATH_MATH:
		    return DINPAR;

		case CMD_OR_MATH_CMD:
		    /*
		     * Not math, so we don't return the contents
		     * as a string in this case.
		     */
		    tokstr = NULL;
		    return INPAR;
		    
		case CMD_OR_MATH_ERR:
		    /*
		     * LEXFLAGS_ACTIVE means we came from bufferwords(),
		     * so we treat as an incomplete math expression
		     */
		    if (lexflags & LEXFLAGS_ACTIVE)
			tokstr = dyncat("((", tokstr ? tokstr : "");
		    /* fall through */

		default:
		    return LEXERR;
		}
	    }
	} else if (d == ')')
	    return INOUTPAR;
	hungetc(d);
	lexstop = 0;
	if (!(isset(SHGLOB) || incond == 1 || incmdpos))
	    break;
	return INPAR;
    case LX1_OUTPAR:
	return OUTPAR;
    case LX1_INANG:
	d = hgetc();
	if (d == '(') {
	    hungetc(d);
	    lexstop = 0;
	    unpeekfd:
	    if(peekfd != -1) {
		hungetc(c);
		c = '0' + peekfd;
	    }
	    break;
	}
	if (d == '>') {
	    peek = INOUTANG;
	} else if (d == '<') {
	    int e = hgetc();

	    if (e == '(') {
		hungetc(e);
		hungetc(d);
		peek = INANG;
	    } else if (e == '<')
		peek = TRINANG;
	    else if (e == '-')
		peek = DINANGDASH;
	    else {
		hungetc(e);
		lexstop = 0;
		peek = DINANG;
	    }
	} else if (d == '&') {
	    peek = INANGAMP;
	} else {
	    hungetc(d);
	    if(isnumglob())
		goto unpeekfd;
	    peek = INANG;
	}
	tokfd = peekfd;
	return peek;
    case LX1_OUTANG:
	d = hgetc();
	if (d == '(') {
	    hungetc(d);
	    goto unpeekfd;
	} else if (d == '&') {
	    d = hgetc();
	    if (d == '!' || d == '|')
		peek = OUTANGAMPBANG;
	    else {
		hungetc(d);
		lexstop = 0;
		peek = OUTANGAMP;
	    }
	} else if (d == '!' || d == '|')
	    peek = OUTANGBANG;
	else if (d == '>') {
	    d = hgetc();
	    if (d == '&') {
		d = hgetc();
		if (d == '!' || d == '|')
		    peek = DOUTANGAMPBANG;
		else {
		    hungetc(d);
		    lexstop = 0;
		    peek = DOUTANGAMP;
		}
	    } else if (d == '!' || d == '|')
		peek = DOUTANGBANG;
	    else if (d == '(') {
		hungetc(d);
		hungetc('>');
		peek = OUTANG;
	    } else {
		hungetc(d);
		lexstop = 0;
		peek = DOUTANG;
		if (isset(HISTALLOWCLOBBER))
		    hwaddc('|');
	    }
	} else {
	    hungetc(d);
	    lexstop = 0;
	    peek = OUTANG;
	    if (!incond && isset(HISTALLOWCLOBBER))
		hwaddc('|');
	}
	tokfd = peekfd;
	return peek;
    }

    /* we've started a string, now get the *
     * rest of it, performing tokenization */
    return gettokstr(c, 0);
}

/*
 * Get the remains of a token string.  This has two uses.
 * When called from gettok(), with sub = 0, we have already identified
 * any interesting initial character and want to get the rest of
 * what we now know is a string.  However, the string may still include
 * metacharacters and potentially substitutions.
 *
 * When called from parse_subst_string() with sub = 1, we are not
 * fully parsing a command line, merely tokenizing a string.
 * In this case we always add characters to the parsed string
 * unless there is a parse error.
 */

/**/
static enum lextok
gettokstr(int c, int sub)
{
    int bct = 0, pct = 0, brct = 0, seen_brct = 0, fdpar = 0;
    int intpos = 1, in_brace_param = 0;
    int inquote, unmatched = 0;
    enum lextok peek;
#ifdef DEBUG
    int ocmdsp = cmdsp;
#endif

    peek = STRING;
    if (!sub) {
	lexbuf.len = 0;
	lexbuf.ptr = tokstr = (char *) hcalloc(lexbuf.siz = LEX_HEAP_SIZE);
    }
    for (;;) {
	int act;
	int e;
	int inbl = inblank(c);
	
	if (fdpar && !inbl && c != ')')
	    fdpar = 0;

	if (inbl && !in_brace_param && !pct)
	    act = LX2_BREAK;
	else {
	    act = lexact2[STOUC(c)];
	    c = lextok2[STOUC(c)];
	}
	switch (act) {
	case LX2_BREAK:
	    if (!in_brace_param && !sub)
		goto brk;
	    break;
	case LX2_META:
	    c = hgetc();
#ifdef DEBUG
	    if (lexstop) {
		fputs("BUG: input terminated by Meta\n", stderr);
		fflush(stderr);
		goto brk;
	    }
#endif
	    add(Meta);
	    break;
	case LX2_OUTPAR:
	    if (fdpar) {
		/* this is a single word `(   )', treat as INOUTPAR */
		add(c);
		*lexbuf.ptr = '\0';
		return INOUTPAR;
	    }
	    if ((sub || in_brace_param) && isset(SHGLOB))
		break;
	    if (!in_brace_param && !pct--) {
		if (sub) {
		    pct = 0;
		    break;
		} else
		    goto brk;
	    }
	    c = Outpar;
	    break;
	case LX2_BAR:
	    if (!pct && !in_brace_param) {
		if (sub)
		    break;
		else
		    goto brk;
	    }
	    if (unset(SHGLOB) || (!sub && !in_brace_param))
		c = Bar;
	    break;
	case LX2_STRING:
	    e = hgetc();
	    if (e == '\\') {
		e = hgetc();
		if (e != '\n') {
		    hungetc(e);
		    hungetc('\\');
		    lexstop = 0;
		    break;
		}
		e = hgetc();
	    }
	    if (e == '[') {
		cmdpush(CS_MATHSUBST);
		add(String);
		add(Inbrack);
		c = dquote_parse(']', sub);
		cmdpop();
		if (c) {
		    peek = LEXERR;
		    goto brk;
		}
		c = Outbrack;
	    } else if (e == '(') {
		add(String);
		switch (cmd_or_math_sub()) {
		case CMD_OR_MATH_CMD:
		    c = Outpar;
		    break;

		case CMD_OR_MATH_MATH:
		    c = Outparmath;
		    break;

		default:
		    peek = LEXERR;
		    goto brk;
		}
	    } else {
		if (e == '{') {
		    add(c);
		    c = Inbrace;
		    ++bct;
		    cmdpush(CS_BRACEPAR);
		    if (!in_brace_param) {
			if ((in_brace_param = bct))
			    seen_brct = 0;
		    }
		} else {
		    hungetc(e);
		    lexstop = 0;
		}
	    }
	    break;
	case LX2_INBRACK:
	    if (!in_brace_param) {
		brct++;
		seen_brct = 1;
	    }
	    c = Inbrack;
	    break;
	case LX2_OUTBRACK:
	    if (!in_brace_param)
		brct--;
	    if (brct < 0)
		brct = 0;
	    c = Outbrack;
	    break;
	case LX2_INPAR:
	    if (isset(SHGLOB)) {
		if (sub || in_brace_param)
		    break;
		if (incasepat > 0 && !lexbuf.len)
		    return INPAR;
		if (!isset(KSHGLOB) && lexbuf.len)
		    goto brk;
	    }
	    if (!in_brace_param) {
		if (!sub) {
		    e = hgetc();
		    hungetc(e);
		    lexstop = 0;
		    /* For command words, parentheses are only
		     * special at the start.  But now we're tokenising
		     * the remaining string.  So I don't see what
		     * the old incmdpos test here is for.
		     *   pws 1999/6/8
		     *
		     * Oh, no.
		     *  func1(   )
		     * is a valid function definition in [k]sh.  The best
		     * thing we can do, without really nasty lookahead tricks,
		     * is break if we find a blank after a parenthesis.  At
		     * least this can't happen inside braces or brackets.  We
		     * only allow this with SHGLOB (set for both sh and ksh).
		     *
		     * Things like `print @( |foo)' should still
		     * work, because [k]sh don't allow multiple words
		     * in a function definition, so we only do this
		     * in command position.
		     *   pws 1999/6/14
		     */
		    if (e == ')' || (isset(SHGLOB) && inblank(e) && !bct &&
				     !brct && !intpos && incmdpos)) {
			/*
			 * Either a () token, or a command word with
			 * something suspiciously like a ksh function
			 * definition.
			 * The current word isn't spellcheckable.
			 */
			nocorrect |= 2;
			goto brk;
		    }
		}
		/*
		 * This also handles the [k]sh `foo( )' function definition.
		 * Maintain a variable fdpar, set as long as a single set of
		 * parentheses contains only space.  Then if we get to the
		 * closing parenthesis and it is still set, we can assume we
		 * have a function definition.  Only do this at the start of
		 * the word, since the (...) must be a separate token.
		 */
		if (!pct++ && isset(SHGLOB) && intpos && !bct && !brct)
		    fdpar = 1;
	    }
	    c = Inpar;
	    break;
	case LX2_INBRACE:
	    if (isset(IGNOREBRACES) || sub)
		c = '{';
	    else {
		if (!lexbuf.len && incmdpos) {
		    add('{');
		    *lexbuf.ptr = '\0';
		    return STRING;
		}
		if (in_brace_param) {
		    cmdpush(CS_BRACE);
		}
		bct++;
	    }
	    break;
	case LX2_OUTBRACE:
	    if ((isset(IGNOREBRACES) || sub) && !in_brace_param)
		break;
	    if (!bct)
		break;
	    if (in_brace_param) {
		cmdpop();
	    }
	    if (bct-- == in_brace_param)
		in_brace_param = 0;
	    c = Outbrace;
	    break;
	case LX2_COMMA:
	    if (unset(IGNOREBRACES) && !sub && bct > in_brace_param)
		c = Comma;
	    break;
	case LX2_OUTANG:
	    if (in_brace_param || sub)
		break;
	    e = hgetc();
	    if (e != '(') {
		hungetc(e);
		lexstop = 0;
		goto brk;
	    }
	    add(OutangProc);
	    if (skipcomm()) {
		peek = LEXERR;
		goto brk;
	    }
	    c = Outpar;
	    break;
	case LX2_INANG:
	    if (isset(SHGLOB) && sub)
		break;
	    e = hgetc();
	    if (!(in_brace_param || sub) && e == '(') {
		add(Inang);
		if (skipcomm()) {
		    peek = LEXERR;
		    goto brk;
		}
		c = Outpar;
		break;
	    }
	    hungetc(e);
	    if(isnumglob()) {
		add(Inang);
		while ((c = hgetc()) != '>')
		    add(c);
		c = Outang;
		break;
	    }
	    lexstop = 0;
	    if (in_brace_param || sub)
		break;
	    goto brk;
	case LX2_EQUALS:
	    if (!sub) {
		if (intpos) {
		    e = hgetc();
		    if (e != '(') {
			hungetc(e);
			lexstop = 0;
			c = Equals;
		    } else {
			add(Equals);
			if (skipcomm()) {
			    peek = LEXERR;
			    goto brk;
			}
			c = Outpar;
		    }
		} else if (peek != ENVSTRING &&
			   (incmdpos || intypeset) && !bct && !brct) {
		    char *t = tokstr;
		    if (idigit(*t))
			while (++t < lexbuf.ptr && idigit(*t));
		    else {
			int sav = *lexbuf.ptr;
			*lexbuf.ptr = '\0';
			t = itype_end(t, IIDENT, 0);
			if (t < lexbuf.ptr) {
			    skipparens(Inbrack, Outbrack, &t);
			} else {
			    *lexbuf.ptr = sav;
			}
		    }
		    if (*t == '+')
			t++;
		    if (t == lexbuf.ptr) {
			e = hgetc();
			if (e == '(') {
			    *lexbuf.ptr = '\0';
			    return ENVARRAY;
			}
			hungetc(e);
			lexstop = 0;
			peek = ENVSTRING;
			intpos = 2;
		    } else
			c = Equals;
		} else
		    c = Equals;
	    }
	    break;
	case LX2_BKSLASH:
	    c = hgetc();
	    if (c == '\n') {
		c = hgetc();
		if (!lexstop)
		    continue;
	    } else {
		add(Bnull);
		if (c == STOUC(Meta)) {
		    c = hgetc();
#ifdef DEBUG
		    if (lexstop) {
			fputs("BUG: input terminated by Meta\n", stderr);
			fflush(stderr);
			goto brk;
		    }
#endif
		    add(Meta);
		}
	    }
	    if (lexstop)
		goto brk;
	    break;
	case LX2_QUOTE: {
	    int strquote = (lexbuf.len && lexbuf.ptr[-1] == String);

	    add(Snull);
	    cmdpush(CS_QUOTE);
	    for (;;) {
		STOPHIST
		while ((c = hgetc()) != '\'' && !lexstop) {
		    if (strquote && c == '\\') {
			c = hgetc();
			if (lexstop)
			    break;
			/*
			 * Mostly we don't need to do anything special
			 * with escape backslashes or closing quotes
			 * inside $'...'; however in completion we
			 * need to be able to strip multiple backslashes
			 * neatly.
			 */
			if (c == '\\' || c == '\'')
			    add(Bnull);
			else
			    add('\\');
		    } else if (!sub && isset(CSHJUNKIEQUOTES) && c == '\n') {
			if (lexbuf.ptr[-1] == '\\')
			    lexbuf.ptr--, lexbuf.len--;
			else
			    break;
		    }
		    add(c);
		}
		ALLOWHIST
		if (c != '\'') {
		    unmatched = '\'';
		    /* Not an error when called from bufferwords() */
		    if (!(lexflags & LEXFLAGS_ACTIVE))
			peek = LEXERR;
		    cmdpop();
		    goto brk;
		}
		e = hgetc();
		if (e != '\'' || unset(RCQUOTES) || strquote)
		    break;
		add(c);
	    }
	    cmdpop();
	    hungetc(e);
	    lexstop = 0;
	    c = Snull;
	    break;
	}
	case LX2_DQUOTE:
	    add(Dnull);
	    cmdpush(CS_DQUOTE);
	    c = dquote_parse('"', sub);
	    cmdpop();
	    if (c) {
		unmatched = '"';
		/* Not an error when called from bufferwords() */
		if (!(lexflags & LEXFLAGS_ACTIVE))
		    peek = LEXERR;
		goto brk;
	    }
	    c = Dnull;
	    break;
	case LX2_BQUOTE:
	    add(Tick);
	    cmdpush(CS_BQUOTE);
	    SETPARBEGIN
	    inquote = 0;
	    while ((c = hgetc()) != '`' && !lexstop) {
		if (c == '\\') {
		    c = hgetc();
		    if (c != '\n') {
			add(c == '`' || c == '\\' || c == '$' ? Bnull : '\\');
			add(c);
		    }
		    else if (!sub && isset(CSHJUNKIEQUOTES))
			add(c);
		} else {
		    if (!sub && isset(CSHJUNKIEQUOTES) && c == '\n') {
			break;
		    }
		    add(c);
		    if (c == '\'') {
			if ((inquote = !inquote))
			    STOPHIST
			else
			    ALLOWHIST
		    }
		}
	    }
	    if (inquote)
		ALLOWHIST
	    cmdpop();
	    if (c != '`') {
		unmatched = '`';
		/* Not an error when called from bufferwords() */
		if (!(lexflags & LEXFLAGS_ACTIVE))
		    peek = LEXERR;
		goto brk;
	    }
	    c = Tick;
	    SETPAREND
	    break;
	case LX2_DASH:
	    /*
	     * - shouldn't be treated as a special character unless
	     * we're in a pattern.  Unfortunately, working out for
	     * sure in complicated expressions whether we're in a
	     * pattern is tricky.  So we'll make it special and
	     * turn it back any time we don't need it special.
	     * This is not ideal as it's a lot of work.
	     */
	    c = Dash;
           break;
       case LX2_BANG:
           /*
            * Same logic as Dash, for ! to perform negation in range.
            */
           if (seen_brct)
               c = Bang;
           else
               c = '!';
       }
       add(c);
       c = hgetc();
	if (intpos)
	    intpos--;
	if (lexstop)
	    break;
    }
  brk:
    if (errflag) {
	if (in_brace_param) {
	    while(bct-- >= in_brace_param)
		cmdpop();
	}
	return LEXERR;
    }
    hungetc(c);
    if (unmatched && !(lexflags & LEXFLAGS_ACTIVE))
	zerr("unmatched %c", unmatched);
    if (in_brace_param) {
	while(bct-- >= in_brace_param)
	    cmdpop();
	zerr("closing brace expected");
    } else if (unset(IGNOREBRACES) && !sub && lexbuf.len > 1 &&
	       peek == STRING && lexbuf.ptr[-1] == '}' &&
	       lexbuf.ptr[-2] != Bnull) {
	/* hack to get {foo} command syntax work */
	lexbuf.ptr--;
	lexbuf.len--;
	lexstop = 0;
	hungetc('}');
    }
    *lexbuf.ptr = '\0';
    DPUTS(cmdsp != ocmdsp, "BUG: gettok: cmdstack changed.");
    return peek;
}


/*
 * Parse input as if in double quotes.
 * endchar is the end character to expect.
 * sub has got something to do with whether we are doing quoted substitution.
 * Return non-zero for error (character to unget), else zero
 */

/**/
static int
dquote_parse(char endchar, int sub)
{
    int pct = 0, brct = 0, bct = 0, intick = 0, err = 0;
    int c;
    int math = endchar == ')' || endchar == ']' || infor;
    int zlemath = math && zlemetacs > zlemetall + addedx - inbufct;

    while (((c = hgetc()) != endchar || bct ||
	    (math && ((pct > 0) || (brct > 0))) ||
	    intick) && !lexstop) {
      cont:
	switch (c) {
	case '\\':
	    c = hgetc();
	    if (c != '\n') {
		if (c == '$' || c == '\\' || (c == '}' && !intick && bct) ||
		    c == endchar || c == '`' ||
		    (endchar == ']' && (c == '[' || c == ']' ||
					c == '(' || c == ')' ||
					c == '{' || c == '}' ||
					(c == '"' && sub))))
		    add(Bnull);
		else {
		    /* lexstop is implicitly handled here */
		    add('\\');
		    goto cont;
		}
	    } else if (sub || unset(CSHJUNKIEQUOTES) || endchar != '"')
		continue;
	    break;
	case '\n':
	    err = !sub && isset(CSHJUNKIEQUOTES) && endchar == '"';
	    break;
	case '$':
	    if (intick)
		break;
	    c = hgetc();
	    if (c == '(') {
		add(Qstring);
		switch (cmd_or_math_sub()) {
		case CMD_OR_MATH_CMD:
		    c = Outpar;
		    break;

		case CMD_OR_MATH_MATH:
		    c = Outparmath;
		    break;

		default:
		    err = 1;
		    break;
		}
	    } else if (c == '[') {
		add(String);
		add(Inbrack);
		cmdpush(CS_MATHSUBST);
		err = dquote_parse(']', sub);
		cmdpop();
		c = Outbrack;
	    } else if (c == '{') {
		add(Qstring);
		c = Inbrace;
		cmdpush(CS_BRACEPAR);
		bct++;
	    } else if (c == '$')
		add(Qstring);
	    else {
		hungetc(c);
		lexstop = 0;
		c = Qstring;
	    }
	    break;
	case '}':
	    if (intick || !bct)
		break;
	    c = Outbrace;
	    bct--;
	    cmdpop();
	    break;
	case '`':
	    c = Qtick;
	    if (intick == 2)
		ALLOWHIST
	    if ((intick = !intick)) {
		SETPARBEGIN
		cmdpush(CS_BQUOTE);
	    } else {
		SETPAREND
	        cmdpop();
	    }
	    break;
	case '\'':
	    if (!intick)
		break;
	    if (intick == 1)
		intick = 2, STOPHIST
	    else
		intick = 1, ALLOWHIST
	    break;
	case '(':
	    if (!math || !bct)
		pct++;
	    break;
	case ')':
	    if (!math || !bct)
		err = (!pct-- && math);
	    break;
	case '[':
	    if (!math || !bct)
		brct++;
	    break;
	case ']':
	    if (!math || !bct)
		err = (!brct-- && math);
	    break;
	case '"':
	    if (intick || (endchar != '"' && !bct))
		break;
	    if (bct) {
		add(Dnull);
		cmdpush(CS_DQUOTE);
		err = dquote_parse('"', sub);
		cmdpop();
		c = Dnull;
	    } else
		err = 1;
	    break;
	}
	if (err || lexstop)
	    break;
	add(c);
    }
    if (intick == 2)
	ALLOWHIST
    if (intick) {
	cmdpop();
    }
    while (bct--)
	cmdpop();
    if (lexstop)
	err = intick || endchar || err;
    else if (err == 1) {
	/*
	 * TODO: as far as I can see, this hack is used in gettokstr()
	 * to hungetc() a character on an error.  However, I don't
	 * understand what that actually gets us, and we can't guarantee
	 * it's a character anyway, because of the previous test.
	 *
	 * We use the same feature in cmd_or_math where we actually do
	 * need to unget if we decide it's really a command substitution.
	 * We try to handle the other case by testing for lexstop.
	 */
	err = c;
    }
    if (zlemath && zlemetacs <= zlemetall + 1 - inbufct)
	inwhat = IN_MATH;
    return err;
}

/*
 * Tokenize a string given in s. Parsing is done as in double
 * quotes.  This is usually called before singsub().
 *
 * parsestr() is noisier, reporting an error if the parse failed.
 *
 * On entry, *s must point to a string allocated from the stack of
 * exactly the right length, i.e. strlen(*s) + 1, as the string
 * is used as the lexical token string whose memory management
 * demands this.  Usually the input string will therefore be
 * the result of an immediately preceding dupstring().
 */

/**/
mod_export int
parsestr(char **s)
{
    int err;

    if ((err = parsestrnoerr(s))) {
	untokenize(*s);
	if (!(errflag & ERRFLAG_INT)) {
	    if (err > 32 && err < 127)
		zerr("parse error near `%c'", err);
	    else
		zerr("parse error");
	    tok = LEXERR;
	}
    }
    return err;
}

/**/
mod_export int
parsestrnoerr(char **s)
{
    int l = strlen(*s), err;

    zcontext_save();
    untokenize(*s);
    inpush(dupstring_wlen(*s, l), 0, NULL);
    strinbeg(0);
    lexbuf.len = 0;
    lexbuf.ptr = tokstr = *s;
    lexbuf.siz = l + 1;
    err = dquote_parse('\0', 1);
    if (tokstr)
	*s = tokstr;
    *lexbuf.ptr = '\0';
    strinend();
    inpop();
    DPUTS(cmdsp, "BUG: parsestr: cmdstack not empty.");
    zcontext_restore();
    return err;
}

/*
 * Parse a subscript in string s.
 * sub is passed down to dquote_parse().
 * endchar is the final character.
 * Return the next character, or NULL.
 */
/**/
mod_export char *
parse_subscript(char *s, int sub, int endchar)
{
    int l = strlen(s), err, toklen;
    char *t;

    if (!*s || *s == endchar)
	return 0;
    zcontext_save();
    untokenize(t = dupstring_wlen(s, l));
    inpush(t, 0, NULL);
    strinbeg(0);
    /*
     * Warning to Future Generations:
     *
     * This way of passing the subscript through the lexer is brittle.
     * Code above this for several layers assumes that when we tokenise
     * the input it goes into the same place as the original string.
     * However, the lexer may overwrite later bits of the string or
     * reallocate it, in particular when expanding aliaes.  To get
     * around this, we copy the string and then copy it back.  This is a
     * bit more robust but still relies on the underlying assumption of
     * length preservation.
     */
    lexbuf.len = 0;
    lexbuf.ptr = tokstr = dupstring_wlen(s, l);
    lexbuf.siz = l + 1;
    err = dquote_parse(endchar, sub);
    toklen = (int)(lexbuf.ptr - tokstr);
    DPUTS(toklen > l, "Bad length for parsed subscript");
    memcpy(s, tokstr, toklen);
    if (err) {
	char *strend = s + toklen;
	err = *strend;
	*strend = '\0';
	untokenize(s);
	*strend = err;
	s = NULL;
    } else {
	s += toklen;
    }
    strinend();
    inpop();
    DPUTS(cmdsp, "BUG: parse_subscript: cmdstack not empty.");
    zcontext_restore();
    return s;
}

/* Tokenize a string given in s. Parsing is done as if s were a normal *
 * command-line argument but it may contain separators.  This is used  *
 * to parse the right-hand side of ${...%...} substitutions.           */

/**/
mod_export int
parse_subst_string(char *s)
{
    int c, l = strlen(s), err;
    char *ptr;
    enum lextok ctok;

    if (!*s || !strcmp(s, nulstring))
	return 0;
    zcontext_save();
    untokenize(s);
    inpush(dupstring_wlen(s, l), 0, NULL);
    strinbeg(0);
    lexbuf.len = 0;
    lexbuf.ptr = tokstr = s;
    lexbuf.siz = l + 1;
    c = hgetc();
    ctok = gettokstr(c, 1);
    err = errflag;
    strinend();
    inpop();
    DPUTS(cmdsp, "BUG: parse_subst_string: cmdstack not empty.");
    zcontext_restore();
    /* Keep any interrupt error status */
    errflag = err | (errflag & ERRFLAG_INT);
    if (ctok == LEXERR) {
	untokenize(s);
	return 1;
    }
#ifdef DEBUG
    /*
     * Historical note: we used to check here for olen (the value of lexbuf.len
     * before zcontext_restore()) == l, but that's not necessarily the case if
     * we stripped an RCQUOTE.
     */
    if (ctok != STRING || (errflag && !noerrs)) {
	fprintf(stderr, "Oops. Bug in parse_subst_string: %s\n",
		errflag ? "errflag" : "ctok != STRING");
	fflush(stderr);
	untokenize(s);
	return 1;
    }
#endif
    /* Check for $'...' quoting.  This needs special handling. */
    for (ptr = s; *ptr; )
    {
	if (*ptr == String && ptr[1] == Snull)
	{
	    char *t;
	    int len, tlen, diff;
	    t = getkeystring(ptr + 2, &len, GETKEYS_DOLLARS_QUOTE, NULL);
	    len += 2;
	    tlen = strlen(t);
	    diff = len - tlen;
	    /*
	     * Yuk.
	     * parse_subst_string() currently handles strings in-place.
	     * That's not so easy to fix without knowing whether
	     * additional memory should come off the heap or
	     * otherwise.  So we cheat by copying the unquoted string
	     * into place, unless it's too long.  That's not the
	     * normal case, but I'm worried there are pathological
	     * cases with converting metafied multibyte strings.
	     * If someone can prove there aren't I will be very happy.
	     */
	    if (diff < 0) {
		DPUTS(1, "$'...' subst too long: fix get_parse_string()");
		return 1;
	    }
	    memcpy(ptr, t, tlen);
	    ptr += tlen;
	    if (diff > 0) {
		char *dptr = ptr;
		char *sptr = ptr + diff;
		while ((*dptr++ = *sptr++))
		    ;
	    }
	} else
	    ptr++;
    }
    return 0;
}

/* Called below to report word positions. */

/**/
static void
gotword(void)
{
    int nwe = zlemetall + 1 - inbufct + (addedx == 2 ? 1 : 0);
    if (zlemetacs <= nwe) {
	int nwb = zlemetall - wordbeg + addedx;
	if (zlemetacs >= nwb) {
	    wb = nwb;
	    we = nwe;
	} else {
	    wb = zlemetacs + addedx;
	    if (we < wb)
		we = wb;
	}
	lexflags = 0;
    }
}

/* Check if current lex text matches an alias: 1 if so, else 0 */

static int
checkalias(void)
{
    Alias an;

    if (!zshlextext)
	return 0;

    if (!noaliases && isset(ALIASESOPT) &&
	(!isset(POSIXALIASES) ||
	 (tok == STRING && !reswdtab->getnode(reswdtab, zshlextext)))) {
	char *suf;

	an = (Alias) aliastab->getnode(aliastab, zshlextext);
	if (an && !an->inuse &&
	    ((an->node.flags & ALIAS_GLOBAL) ||
	     (incmdpos && tok == STRING) || inalmore)) {
	    if (!lexstop) {
		/*
		 * Tokens that don't require a space after, get one,
		 * because they are treated as if preceded by one.
		 */
		int c = hgetc();
		hungetc(c);
		if (!iblank(c))
		    inpush(" ", INP_ALIAS, 0);
	    }
	    inpush(an->text, INP_ALIAS, an);
	    if (an->text[0] == ' ' && !(an->node.flags & ALIAS_GLOBAL))
		aliasspaceflag = 1;
	    lexstop = 0;
	    return 1;
	}
	if ((suf = strrchr(zshlextext, '.')) && suf[1] &&
	    suf > zshlextext && suf[-1] != Meta &&
	    (an = (Alias)sufaliastab->getnode(sufaliastab, suf+1)) &&
	    !an->inuse && incmdpos) {
	    inpush(dupstring(zshlextext), INP_ALIAS, an);
	    inpush(" ", INP_ALIAS, NULL);
	    inpush(an->text, INP_ALIAS, NULL);
	    lexstop = 0;
	    return 1;
	}
    }

    return 0;
}

/* expand aliases and reserved words */

/**/
int
exalias(void)
{
    Reswd rw;

    hwend();
    if (interact && isset(SHINSTDIN) && !strin && incasepat <= 0 &&
	tok == STRING && !nocorrect && !(inbufflags & INP_ALIAS) &&
	!hist_is_in_word()  &&
	(isset(CORRECTALL) || (isset(CORRECT) && incmdpos)))
	spckword(&tokstr, 1, incmdpos, 1);

    if (!tokstr) {
	zshlextext = tokstrings[tok];

	if (tok == NEWLIN)
	    return 0;
	return checkalias();
    } else {
	VARARR(char, copy, (strlen(tokstr) + 1));

	if (has_token(tokstr)) {
	    char *p, *t;

	    zshlextext = p = copy;
	    for (t = tokstr;
		 (*p++ = itok(*t) ? ztokens[*t++ - Pound] : *t++););
	} else
	    zshlextext = tokstr;

	if ((lexflags & LEXFLAGS_ZLE) && !(inbufflags & INP_ALIAS)) {
	    int zp = lexflags;

	    gotword();
	    if ((zp & LEXFLAGS_ZLE) && !lexflags) {
		if (zshlextext == copy)
		    zshlextext = tokstr;
		return 0;
	    }
	}

	if (tok == STRING) {
	    /* Check for an alias */
	    if ((zshlextext != copy || !isset(POSIXALIASES)) && checkalias()) {
		if (zshlextext == copy)
		    zshlextext = tokstr;
		return 1;
	    }

	    /* Then check for a reserved word */
	    if ((incmdpos ||
		 (unset(IGNOREBRACES) && unset(IGNORECLOSEBRACES) &&
		  zshlextext[0] == '}' && !zshlextext[1])) &&
		(rw = (Reswd) reswdtab->getnode(reswdtab, zshlextext))) {
		tok = rw->token;
		inrepeat_ = (tok == REPEAT);
		if (tok == DINBRACK)
		    incond = 1;
	    } else if (incond && !strcmp(zshlextext, "]]")) {
		tok = DOUTBRACK;
		incond = 0;
	    } else if (incond == 1 && zshlextext[0] == '!' && !zshlextext[1])
		tok = BANG;
	}
	inalmore = 0;
	if (zshlextext == copy)
	    zshlextext = tokstr;
    }
    return 0;
}

/**/
void
zshlex_raw_add(int c)
{
    if (!lex_add_raw)
	return;

    *lexbuf_raw.ptr++ = c;
    if (lexbuf_raw.siz == ++lexbuf_raw.len) {
	int newbsiz = lexbuf_raw.siz * 2;

	tokstr_raw = (char *)hrealloc(tokstr_raw, lexbuf_raw.siz, newbsiz);
	lexbuf_raw.ptr = tokstr_raw + lexbuf_raw.len;
	memset(lexbuf_raw.ptr, 0, newbsiz - lexbuf_raw.siz);
	lexbuf_raw.siz = newbsiz;
    }
}

/**/
void
zshlex_raw_back(void)
{
    if (!lex_add_raw)
	return;
    lexbuf_raw.ptr--;
    lexbuf_raw.len--;
}

/**/
int
zshlex_raw_mark(int offset)
{
    if (!lex_add_raw)
	return 0;
    return lexbuf_raw.len + offset;
}

/**/
void
zshlex_raw_back_to_mark(int mark)
{
    if (!lex_add_raw)
	return;
    lexbuf_raw.ptr = tokstr_raw + mark;
    lexbuf_raw.len = mark;
}

/*
 * Skip (...) for command-style substitutions: $(...), <(...), >(...)
 *
 * In order to ensure we don't stop at closing parentheses with
 * some other syntactic significance, we'll parse the input until
 * we find an unmatched closing parenthesis.  However, we'll throw
 * away the result of the parsing and just keep the string we've built
 * up on the way.
 */

/**/
static int
skipcomm(void)
{
#ifdef ZSH_OLD_SKIPCOMM
    int pct = 1, c, start = 1;

    cmdpush(CS_CMDSUBST);
    SETPARBEGIN
    c = Inpar;
    do {
	int iswhite;
	add(c);
	c = hgetc();
	if (itok(c) || lexstop)
	    break;
	iswhite = inblank(c);
	switch (c) {
	case '(':
	    pct++;
	    break;
	case ')':
	    pct--;
	    break;
	case '\\':
	    add(c);
	    c = hgetc();
	    break;
	case '\'': {
	    int strquote = lexbuf.ptr[-1] == '$';
	    add(c);
	    STOPHIST
	    while ((c = hgetc()) != '\'' && !lexstop) {
		if (c == '\\' && strquote) {
		    add(c);
		    c = hgetc();
		}
		add(c);
	    }
	    ALLOWHIST
	    break;
	}
	case '\"':
	    add(c);
	    while ((c = hgetc()) != '\"' && !lexstop)
		if (c == '\\') {
		    add(c);
		    add(hgetc());
		} else
		    add(c);
	    break;
	case '`':
	    add(c);
	    while ((c = hgetc()) != '`' && !lexstop)
		if (c == '\\')
		    add(c), add(hgetc());
		else
		    add(c);
	    break;
	case '#':
	    if (start) {
		add(c);
		while ((c = hgetc()) != '\n' && !lexstop)
		    add(c);
		iswhite = 1;
	    }
	    break;
	}
	start = iswhite;
    }
    while (pct);
    if (!lexstop)
	SETPAREND
    cmdpop();
    return lexstop;
#else
    char *new_tokstr;
    int new_lexstop, new_lex_add_raw;
    int save_infor = infor;
    struct lexbufstate new_lexbuf;

    infor = 0;
    cmdpush(CS_CMDSUBST);
    SETPARBEGIN
    add(Inpar);

    new_lex_add_raw = lex_add_raw + 1;
    if (!lex_add_raw) {
	/*
	 * We'll combine the string so far with the input
	 * read in for the command substitution.  To do this
	 * we'll just propagate the current tokstr etc. as the
	 * variables used for adding raw input, and
	 * ensure we swap those for the real tokstr etc. at the end.
	 *
	 * However, we need to save and restore the rest of the
	 * lexical and parse state as we're effectively parsing
	 * an internal string.  Because we're still parsing it from
	 * the original input source (we have to --- we don't know
	 * when to stop inputting it otherwise and can't rely on
	 * the input being recoverable until we've read it) we need
	 * to keep the same history context.
	 */
	new_tokstr = tokstr;
	new_lexbuf = lexbuf;

	/*
	 * If we're expanding an alias at this point, we need the whole
	 * remaining text as part of the string for the command in
	 * parentheses, so don't backtrack.  This is different from the
	 * usual case where the alias is fully within the command, where
	 * we want the unexpanded text so that it will be expanded
	 * again when the command in the parentheses is executed.
	 *
	 * I never wanted to be a software engineer, you know.
	 */
	if (inbufflags & INP_ALIAS)
	    inbufflags |= INP_RAW_KEEP;
	zcontext_save_partial(ZCONTEXT_LEX|ZCONTEXT_PARSE);
	hist_in_word(1);
    } else {
	/*
	 * Set up for nested command substitution, however
	 * we don't actually need the string until we get
	 * back to the top level and recover the lot.
	 * The $() body just appears empty.
	 *
	 * We do need to propagate the raw variables which would
	 * otherwise by cleared, though.
	 */
	new_tokstr = tokstr_raw;
	new_lexbuf = lexbuf_raw;

	zcontext_save_partial(ZCONTEXT_LEX|ZCONTEXT_PARSE);
    }
    tokstr_raw = new_tokstr;
    lexbuf_raw = new_lexbuf;
    lex_add_raw = new_lex_add_raw;
    /*
     * Don't do any ZLE specials down here: they're only needed
     * when we return the string from the recursive parse.
     * (TBD: this probably means we should be initialising lexflags
     * more consistently.)
     *
     * Note that in that case we're still using the ZLE line reading
     * function at the history layer --- this is consistent with the
     * intention of maintaining the history and input layers across
     * the recursive parsing.
     *
     * Also turn off LEXFLAGS_NEWLINE because this is already skipping
     * across the entire construct, and parse_event() needs embedded
     * newlines to be "real" when looking for the OUTPAR token.
     */
    lexflags &= ~(LEXFLAGS_ZLE|LEXFLAGS_NEWLINE);
    dbparens = 0;	/* restored by zcontext_restore_partial() */

    if (!parse_event(OUTPAR) || tok != OUTPAR) {
	if (strin) {
	    /*
	     * Get the rest of the string raw since we don't
	     * know where this token ends.
	     */
	    while (!lexstop)
		(void)ingetc();
	} else
	    lexstop = 1;
    }
     /* Outpar lexical token gets added in caller if present */

    /*
     * We're going to keep the full raw input string
     * as the current token string after popping the stack.
     */
    new_tokstr = tokstr_raw;
    new_lexbuf = lexbuf_raw;
    /*
     * We're also going to propagate the lexical state:
     * if we couldn't parse the command substitution we
     * can't continue.
     */
    new_lexstop = lexstop;

    zcontext_restore_partial(ZCONTEXT_LEX|ZCONTEXT_PARSE);

    if (lex_add_raw) {
	/*
	 * Keep going, so retain the raw variables.
	 */
	tokstr_raw = new_tokstr;
	lexbuf_raw = new_lexbuf;
    } else {
	if (!new_lexstop) {
	    /* Ignore the ')' added on input */
	    new_lexbuf.len--;
	    *--new_lexbuf.ptr = '\0';
	}

	/*
	 * Convince the rest of lex.c we were examining a string
	 * all along.
	 */
	tokstr = new_tokstr;
	lexbuf = new_lexbuf;
	lexstop = new_lexstop;
	hist_in_word(0);
    }

    if (!lexstop)
	SETPAREND
    cmdpop();
    infor = save_infor;

    return lexstop;
#endif
}
