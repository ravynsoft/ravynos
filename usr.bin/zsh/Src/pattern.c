/*
 * pattern.c - pattern matching
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1999 Peter Stephenson
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Peter Stephenson or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Peter Stephenson and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Peter Stephenson and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Peter Stephenson and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 * Pattern matching code derived from the regexp library by Henry
 * Spencer, which has the following copyright.
 *
 *	Copyright (c) 1986 by University of Toronto.
 *	Written by Henry Spencer.  Not derived from licensed software.
 *
 *	Permission is granted to anyone to use this software for any
 *	purpose on any computer system, and to redistribute it freely,
 *	subject to the following restrictions:
 *
 *	1. The author is not responsible for the consequences of use of
 *		this software, no matter how awful, even if they arise
 *		from defects in it.
 *
 *	2. The origin of this software must not be misrepresented, either
 *		by explicit claim or by omission.
 *
 *	3. Altered versions must be plainly marked as such, and must not
 *		be misrepresented as being the original software.
 *
 * Eagle-eyed readers will notice this is an altered version.  Incredibly
 * sharp-eyed readers might even find bits that weren't altered.
 *
 *
 *      And I experienced a sense that, like certain regular
 *      expressions, seemed to match the day from beginning to end, so
 *      that I did not need to identify the parenthesised subexpression
 *      that told of dawn, nor the group of characters that indicated
 *      the moment when my grandfather returned home with news of
 *      Swann's departure for Paris; and the whole length of the month
 *      of May, as if matched by a closure, fitted into the buffer of my
 *      life with no sign of overflowing, turning the days, like a
 *      procession of insects that could consist of this or that
 *      species, into a random and unstructured repetition of different
 *      sequences, anchored from the first day of the month to the last
 *      in the same fashion as the weeks when I knew I would not see
 *      Gilberte and would search in vain for any occurrences of the
 *      string in the avenue of hawthorns by Tansonville, without my
 *      having to delimit explicitly the start or finish of the pattern.
 *
 *                                 M. Proust, "In Search of Lost Files",
 *                                 bk I, "The Walk by Bourne's Place".
 */

#include "zsh.mdh"

/*
 * The following union is used mostly for alignment purposes.
 * Normal nodes are longs, while certain nodes take a char * as an argument;
 * here we make sure that they both work out to the same length.
 * The compiled regexp we construct consists of upats stuck together;
 * anything else to be added (strings, numbers) is stuck after and
 * then aligned to a whole number of upat units.
 *
 * Note also that offsets are in terms of the sizes of these things.
 */
union upat {
    long l;
    unsigned char *p;
};

typedef union upat *Upat;

#include "pattern.pro"

/* Number of active parenthesized expressions allowed in backreferencing */
#define NSUBEXP  9

/* definition	number	opnd?	meaning */
#define	P_END	  0x00	/* no	End of program. */
#define P_EXCSYNC 0x01	/* no   Test if following exclude already failed */
#define P_EXCEND  0x02	/* no   Test if exclude matched orig branch */
#define	P_BACK	  0x03	/* no	Match "", "next" ptr points backward. */
#define	P_EXACTLY 0x04	/* lstr	Match this string. */
#define	P_NOTHING 0x05	/* no	Match empty string. */
#define	P_ONEHASH 0x06	/* node	Match this (simple) thing 0 or more times. */
#define	P_TWOHASH 0x07	/* node	Match this (simple) thing 1 or more times. */
#define P_GFLAGS  0x08	/* long Match nothing and set globbing flags */
#define P_ISSTART 0x09  /* no   Match start of string. */
#define P_ISEND   0x0a  /* no   Match end of string. */
#define P_COUNTSTART 0x0b /* no Initialise P_COUNT */
#define P_COUNT   0x0c  /* 3*long uc* node Match a number of repetitions */
/* numbered so we can test bit 5 for a branch */
#define	P_BRANCH  0x20	/* node	Match this alternative, or the next... */
#define	P_WBRANCH 0x21	/* uc* node P_BRANCH, but match at least 1 char */
/* excludes are also branches, but have bit 4 set, too */
#define P_EXCLUDE 0x30	/* uc* node Exclude this from previous branch */
#define P_EXCLUDP 0x31	/* uc* node Exclude, using full file path so far */
/* numbered so we can test bit 6 so as not to match initial '.' */
#define	P_ANY	  0x40	/* no	Match any one character. */
#define	P_ANYOF	  0x41	/* str  Match any character in this string. */
#define	P_ANYBUT  0x42	/* str  Match any character not in this string. */
#define P_STAR    0x43	/* no   Match any set of characters. */
#define P_NUMRNG  0x44	/* zr, zr Match a numeric range. */
#define P_NUMFROM 0x45	/* zr   Match a number >= X */
#define P_NUMTO   0x46	/* zr   Match a number <= X */
#define P_NUMANY  0x47	/* no   Match any set of decimal digits */
/* spaces left for P_OPEN+n,... for backreferences */
#define	P_OPEN	  0x80	/* no	Mark this point in input as start of n. */
#define	P_CLOSE	  0x90	/* no	Analogous to OPEN. */
/*
 * no    no argument
 * zr    the range type zrange_t:  may be zlong or unsigned long
 * char  a single char
 * uc*   a pointer to unsigned char, used at run time and initialised
 *       to NULL.
 * str   null-terminated, metafied string
 * lstr  length as long then string, not null-terminated, unmetafied.
 */

/*
 * Notes on usage:
 * P_WBRANCH:  This works like a branch and is used in complex closures,
 *    to ensure we don't succeed on a zero-length match of the pattern,
 *    since that would cause an infinite loop.  We do this by recording
 *    the positions where we have already tried to match.   See the
 *    P_WBRANCH test in patmatch().
 *
 *  P_ANY, P_ANYOF:  the operand is a null terminated
 *    string.  Normal characters match as expected.  Characters
 *    in the range Meta+PP_ALPHA..Meta+PP_UNKWN do the appropriate
 *    Posix range tests.  This relies on imeta returning true for these
 *    characters.  We treat unknown POSIX ranges as never matching.
 *    PP_RANGE means the next two (possibly metafied) characters form
 *    the limits of a range to test; it's too much like hard work to
 *    expand the range.
 *
 *  P_EXCLUDE, P_EXCSYNC, PEXCEND:  P_EXCLUDE appears in the pattern like
 *    P_BRANCH, but applies to the immediately preceding branch.  The code in
 *    the corresponding branch is followed by a P_EXCSYNC, which simply
 *    acts as a marker that a P_EXCLUDE comes next.  The P_EXCLUDE
 *    has a pointer to char embedded in it, which works
 *    like P_WBRANCH:  if we get to the P_EXCSYNC, and we already matched
 *    up to the same position, fail.  Thus we are forced to backtrack
 *    on closures in the P_BRANCH if the first attempt was excluded.
 *    Corresponding to P_EXCSYNC in the original branch, there is a
 *    P_EXCEND in the exclusion.  If we get to this point, and we did
 *    *not* match in the original branch, the exclusion itself fails,
 *    otherwise it succeeds since we know the tail already matches,
 *    so P_EXCEND is the end of the exclusion test.
 *    The whole sorry mess looks like this, where the upper lines
 *    show the linkage of the branches, and the lower shows the linkage
 *    of their pattern arguments.
 *
 *     	        ---------------------      ----------------------
 *              ^      	       	     v    ^      	         v
 *      ( <BRANCH>:apat-><EXCSYNC> <EXCLUDE>:excpat-><EXCEND> ) tail
 *                               	                         ^
 *		       	  |                                      |
 *			   --------------------------------------
 *
 * P_EXCLUDP: this behaves exactly like P_EXCLUDE, with the sole exception
 *   that we prepend the path so far to the exclude pattern.   This is
 *   for top level file globs, e.g. ** / *.c~*foo.c
 *                                    ^ I had to leave this space
 * P_NUM*: zl is a zlong if that is 64-bit, else an unsigned long.
 *
 * P_COUNTSTART, P_COUNT: a P_COUNTSTART flags the start of a quantified
 * closure (#cN,M) and is used to initialise the count.  Executing
 * the pattern leads back to the P_COUNT, while the next links of the
 * P_COUNTSTART and P_COUNT lead to the tail of the pattern:
 *
 *	       	        ----------------
 *     	       	       v       	        ^
 *        <COUNTSTART><COUNT>pattern<BACK> tail
 *	     	    v      v  	  	    ^
 *	            ------------------------
 */

#define	P_OP(p)		((p)->l & 0xff)
#define	P_NEXT(p)	((p)->l >> 8)
#define	P_OPERAND(p)	((p) + 1)
#define P_ISBRANCH(p)   ((p)->l & 0x20)
#define P_ISEXCLUDE(p)	(((p)->l & 0x30) == 0x30)
#define P_NOTDOT(p)	((p)->l & 0x40)

/* Specific to lstr type, i.e. P_EXACTLY. */
#define P_LS_LEN(p)	((p)[1].l) /* can be used as lvalue */
#define P_LS_STR(p)	((char *)((p) + 2))

/* Specific to P_COUNT: arguments as offset in nodes from operator */
#define P_CT_CURRENT	(1)	/* Current count */
#define P_CT_MIN	(2)     /* Minimum count */
#define P_CT_MAX	(3)	/* Maximum count, -1 for none */
#define P_CT_PTR	(4)	/* Pointer to last match start */
#define P_CT_OPERAND	(5)	/* Operand of P_COUNT */

/* Flags needed when pattern is executed */
#define P_SIMPLE        0x01	/* Simple enough to be #/## operand. */
#define P_HSTART        0x02	/* Starts with # or ##'d pattern. */
#define P_PURESTR	0x04	/* Can be matched with a strcmp */

#if defined(ZSH_64_BIT_TYPE) || defined(LONG_IS_64_BIT)
typedef zlong zrange_t;
#define ZRANGE_T_IS_SIGNED	(1)
#define ZRANGE_MAX ZLONG_MAX
#else
typedef unsigned long zrange_t;
#define ZRANGE_MAX ULONG_MAX
#endif

#ifdef MULTIBYTE_SUPPORT
/*
 * Handle a byte that's not part of a valid character.
 *
 * This range in Unicode is recommended for purposes of this
 * kind as it corresponds to invalid characters.
 *
 * Note that this strictly only works if wchar_t represents
 * Unicode code points, which isn't necessarily true; however,
 * converting an invalid character into an unknown format is
 * a bit tricky...
 */
#define WCHAR_INVALID(ch)			\
    ((wchar_t) (0xDC00 + STOUC(ch)))
#endif /* MULTIBYTE_SUPPORT */

/*
 * Array of characters corresponding to zpc_chars enum, which it must match.
 */
static const char zpc_chars[ZPC_COUNT] = {
    '/', '\0', Bar, Outpar, Tilde, Inpar, Quest, Star, Inbrack, Inang,
    Hat, Pound, Bnullkeep, Quest, Star, '+', Bang, '!', '@'
};

/*
 * Corresponding strings used in enable/disable -p.
 * NULL means no way of turning this on or off.
 */
/**/
mod_export const char *zpc_strings[ZPC_COUNT] = {
   NULL, NULL, "|", NULL, "~", "(", "?", "*", "[", "<",
   "^", "#", NULL, "?(", "*(", "+(", "!(", "\\!(", "@("
};

/*
 * Corresponding array of pattern disables as set by the user
 * using "disable -p".
 */
/**/
mod_export char zpc_disables[ZPC_COUNT];

/*
 * Stack of saved (compressed) zpc_disables for function scope.
 */

static struct zpc_disables_save *zpc_disables_stack;

/*
 * Characters which terminate a simple string (ZPC_COUNT) or
 * an entire pattern segment (the first ZPC_SEG_COUNT).
 * Each entry is either the corresponding character in zpc_chars
 * or Marker which is guaranteed not to match a character in a
 * pattern we are compiling.
 *
 * The complete list indicates characters that are special, so e.g.
 * (testchar == special[ZPC_TILDE]) succeeds only if testchar is a Tilde
 * *and* Tilde is currently special.
 */

/**/
char zpc_special[ZPC_COUNT];

/* Default size for pattern buffer */
#define P_DEF_ALLOC 256

/* Flags used in compilation */
static char *patstart, *patparse;	/* input pointers */
static int patnpar;		/* () count */
static char *patcode;		/* point of code emission */
static long patsize;		/* size of code */
static char *patout;		/* start of code emission string */
static long patalloc;		/* size allocated for same */

/* Flags used in both compilation and execution */
static int patflags;		    /* flags passed down to patcompile */
static int patglobflags;  /* globbing flags & approx */

/*
 * Increment pointer to metafied multibyte string.
 */
#ifdef MULTIBYTE_SUPPORT
typedef wint_t patint_t;

#define PEOF WEOF

#define METACHARINC(x) ((void)metacharinc(&x))

/*
 * TODO: the shiftstate isn't well handled; we don't guarantee
 * to maintain it properly between characters.  If we don't
 * need it we should use mbtowc() instead.
 */
static mbstate_t shiftstate;

/* See clear_mbstate() in params.c for the use of clear_shiftstate() */

/**/
mod_export void
clear_shiftstate(void) {
    memset(&shiftstate, 0, sizeof(shiftstate));
}

/*
 * Multibyte version: it's (almost) as easy to return the
 * value as not, so do so since we sometimes need it..
 */
static wchar_t
metacharinc(char **x)
{
    char *inptr = *x;
    char inchar;
    size_t ret = MB_INVALID;
    wchar_t wc;

    /*
     * Cheat if the top bit isn't set.  This is second-guessing
     * the library, but we know for sure that if the character
     * set doesn't have the property that all bytes with the 8th
     * bit clear are single characters then we are stuffed.
     */
    if (!(patglobflags & GF_MULTIBYTE) || !(STOUC(*inptr) & 0x80))
    {
	if (itok(*inptr))
	    inchar = ztokens[*inptr++ - Pound];
	else if (*inptr == Meta) {
	    inptr++;
	    inchar = *inptr++ ^ 32;
	} else {
	    inchar = *inptr++;
	}
	*x = inptr;
	return (wchar_t)STOUC(inchar);
    }

    while (*inptr) {
	if (itok(*inptr))
	    inchar = ztokens[*inptr++ - Pound];
	else if (*inptr == Meta) {
	    inptr++;
	    inchar = *inptr++ ^ 32;
	} else {
	    inchar = *inptr++;
	}
	ret = mbrtowc(&wc, &inchar, 1, &shiftstate);

	if (ret == MB_INVALID)
	    break;
	if (ret == MB_INCOMPLETE)
	    continue;
	*x = inptr;
	return wc;
    }

    /* Error. */
    /* Reset the shift state for next time. */
    memset(&shiftstate, 0, sizeof(shiftstate));
    return WCHAR_INVALID(*(*x)++);
}

#else
typedef int patint_t;

#define PEOF EOF

#define METACHARINC(x)	((void)((x) += (*(x) == Meta) ? 2 : 1))
#endif

/*
 * Return unmetafied char from string (x is any char *).
 * Used with MULTIBYTE_SUPPORT if the GF_MULTIBYTE is not
 * in effect.
 */
#define UNMETA(x)	(*(x) == Meta ? (x)[1] ^ 32 : *(x))

/* Add n more characters, ensuring there is enough space. */

enum {
    PA_NOALIGN = 1,
    PA_UNMETA  = 2
};

/**/
static void
patadd(char *add, int ch, long n, int paflags)
{
    /* Make sure everything gets aligned unless we get PA_NOALIGN. */
    long newpatsize = patsize + n;
    if (!(paflags & PA_NOALIGN))
	newpatsize = (newpatsize + sizeof(union upat) - 1) &
		      ~(sizeof(union upat) - 1);
    if (patalloc < newpatsize) {
	long newpatalloc =
	    2*(newpatsize > patalloc ? newpatsize : patalloc);
	patout = (char *)zrealloc((char *)patout, newpatalloc);
	patcode = patout + patsize;
	patalloc = newpatalloc;
    }
    patsize = newpatsize;
    if (add) {
	if (paflags & PA_UNMETA) {
	    /*
	     * Unmetafy and untokenize the string as we go.
	     * The Meta characters in add aren't counted in n.
	     */
	    while (n--) {
		if (itok(*add))
		    *patcode++ = ztokens[*add++ - Pound];
		else if (*add == Meta) {
		    add++;
		    *patcode++ = *add++ ^ 32;
		} else {
		    *patcode++ = *add++;
		}
	    }
	} else {
	    while (n--)
		*patcode++ = *add++;
	}
    } else
	*patcode++ = ch;
    patcode = patout + patsize;
}

static long rn_offs;
/* operates on pointers to union upat, returns a pointer */
#define PATNEXT(p) ((rn_offs = P_NEXT(p)) ? \
		    (P_OP(p) == P_BACK) ? \
		    ((p)-rn_offs) : ((p)+rn_offs) : NULL)

/*
 * Set up zpc_special with characters that end a string segment.
 * "Marker" cannot occur in the pattern we are compiling so
 * is used to mark "invalid".
 */
static void
patcompcharsset(void)
{
    char *spp, *disp;
    int i;

    /* Initialise enabled special characters */
    memcpy(zpc_special, zpc_chars, ZPC_COUNT);
    /* Apply user disables from disable -p */
    for (i = 0, spp = zpc_special, disp = zpc_disables;
	 i < ZPC_COUNT;
	 i++, spp++, disp++) {
	if (*disp)
	    *spp = Marker;
    }

    if (!isset(EXTENDEDGLOB)) {
	/* Extended glob characters are not active */
	zpc_special[ZPC_TILDE] = zpc_special[ZPC_HAT] =
	    zpc_special[ZPC_HASH] = Marker;
    }
    if (!isset(KSHGLOB)) {
	/*
	 * Ksh glob characters are not active.
	 * * and ? are shared with normal globbing, but for their
	 * use here we are looking for a following Inpar.
	 */
	zpc_special[ZPC_KSH_QUEST] = zpc_special[ZPC_KSH_STAR] =
	    zpc_special[ZPC_KSH_PLUS] = zpc_special[ZPC_KSH_BANG] =
	    zpc_special[ZPC_KSH_BANG2] = zpc_special[ZPC_KSH_AT] = Marker;
    }
    /*
     * Note that if we are using KSHGLOB, then we test for a following
     * Inpar, not zpc_special[ZPC_INPAR]:  the latter makes an Inpar on
     * its own active.  The zpc_special[ZPC_KSH_*] followed by any old Inpar
     * discriminate ksh globbing.
     */
    if (isset(SHGLOB)) {
	/*
	 * Grouping and numeric ranges are not valid.
	 * We do allow alternation, however; it's needed for
	 * "case".  This may not be entirely consistent.
	 *
	 * Don't disable Outpar: we may need to match the end of KSHGLOB
	 * parentheses and it would be difficult to tell them apart.
	 */
	zpc_special[ZPC_INPAR] = zpc_special[ZPC_INANG] = Marker;
    }
}

/* Called before parsing a set of file matches to initialize flags */

/**/
void
patcompstart(void)
{
    patcompcharsset();
    if (isset(CASEGLOB) || isset(CASEPATHS))
	patglobflags = 0;
    else
	patglobflags = GF_IGNCASE;
    if (isset(MULTIBYTE))
	patglobflags |= GF_MULTIBYTE;
}

/*
 * Top level pattern compilation subroutine
 * exp is a null-terminated, metafied string.
 * inflags is an or of some PAT_* flags.
 * endexp, if non-null, is set to a pointer to the end of the
 *   part of exp which was compiled.  This is used when
 *   compiling patterns for directories which must be
 *   matched recursively.
 */

/**/
mod_export Patprog
patcompile(char *exp, int inflags, char **endexp)
{
    int flags = 0;
    long len = 0;
    long startoff;
    Upat pscan;
    char *lng, *strp = NULL;
    Patprog p;

    queue_signals();

    startoff = sizeof(struct patprog);
    /* Ensure alignment of start of program string */
    startoff = (startoff + sizeof(union upat) - 1) & ~(sizeof(union upat) - 1);

    /* Allocate reasonable sized chunk if none, reduce size if too big */
    if (patalloc != P_DEF_ALLOC)
	patout = (char *)zrealloc(patout, patalloc = P_DEF_ALLOC);
    patcode = patout + startoff;
    patsize = patcode - patout;
    patstart = patparse = exp;
    /*
     * Note global patnpar numbers parentheses 1..9, while patnpar
     * in struct is actual count of parentheses.
     */
    patnpar = 1;
    patflags = inflags & ~(PAT_PURES|PAT_HAS_EXCLUDP);

    if (!(patflags & PAT_FILE)) {
	patcompcharsset();
	zpc_special[ZPC_SLASH] = Marker;
	remnulargs(patparse);
	if (isset(MULTIBYTE))
	    patglobflags = GF_MULTIBYTE;
	else
	    patglobflags = 0;
    }
    if (patflags & PAT_LCMATCHUC)
	patglobflags |= GF_LCMATCHUC;
    /*
     * Have to be set now, since they get updated during compilation.
     */
    ((Patprog)patout)->globflags = patglobflags;

    if (!(patflags & PAT_ANY)) {
	/* Look for a really pure string, with no tokens at all. */
	if (!(patglobflags & ~GF_MULTIBYTE)
#ifdef __CYGWIN__
	    /*
	     * If the OS treats files case-insensitively and we
	     * are looking at files, we don't need to use pattern
	     * matching to find the file.
	     */
	    || (!(patglobflags & ~GF_IGNCASE) && (patflags & PAT_FILE))
#endif
	    )
	{
	    /*
	     * Waah!  I wish I understood this.
	     * Empty metafied strings have an initial Nularg.
	     * This never corresponds to a real character in
	     * a glob pattern or string, so skip it.
	     */
	    if (*exp == Nularg)
		exp++;
	    for (strp = exp; *strp &&
		     (!(patflags & PAT_FILE) || *strp != '/') && !itok(*strp);
		 strp++)
		;
	}
	if (!strp || (*strp && *strp != '/')) {
	    /* No, do normal compilation. */
	    strp = NULL;
	    if (patcompswitch(0, &flags) == 0) {
		unqueue_signals();
		return NULL;
	    }
	} else {
	    /*
	     * Yes, copy the string, and skip compilation altogether.
	     * Null terminate for the benefit of globbing.
	     * Leave metafied both for globbing and for our own
	     * efficiency.
	     */
	    patparse = strp;
	    len = strp - exp;
	    patadd(exp, 0, len + 1, 0);
	    patout[startoff + len] = '\0';
	    patflags |= PAT_PURES;
	}
    }

    /* end of compilation: safe to use pointers */
    p = (Patprog)patout;
    p->startoff = startoff;
    p->patstartch = '\0';
    p->globend = patglobflags;
    p->flags = patflags;
    p->mustoff = 0;
    p->size = patsize;
    p->patmlen = len;
    p->patnpar = patnpar-1;

#ifndef __CYGWIN__  /* The filesystem itself is case-insensitive on Cygwin */
    if ((patflags & PAT_FILE) && !isset(CASEGLOB) && !(patflags & PAT_PURES)) {
	p->globflags |= GF_IGNCASE;
	p->globend |= GF_IGNCASE;
    }
#endif

    if (!strp) {
	pscan = (Upat)(patout + startoff);

	if (!(patflags & PAT_ANY) && P_OP(PATNEXT(pscan)) == P_END) {
	    /* only one top level choice */
	    pscan = P_OPERAND(pscan);

	    if (flags & P_PURESTR) {
		/*
		 * The pattern can be matched with a simple strncmp/strcmp.
		 * Careful in case we've overwritten the node for the next ptr.
		 */
		char *dst = patout + startoff;
		Upat next;
		p->flags |= PAT_PURES;
		for (; pscan; pscan = next) {
		    next = PATNEXT(pscan);
		    if (P_OP(pscan) == P_EXACTLY) {
			char *opnd = P_LS_STR(pscan), *mtest;
			long oplen = P_LS_LEN(pscan), ilen;
			int nmeta = 0;
			/*
			 * Unfortunately we unmetafied the string
			 * and we need to put any metacharacters
			 * back now we know it's a pure string.
			 * This shouldn't happen too often, it's
			 * just that there are some cases such
			 * as . and .. in files where we really
			 * need a pure string even if there are
			 * pattern characters flying around.
			 */
			for (mtest = opnd, ilen = oplen; ilen;
			     mtest++, ilen--)
			    if (imeta(*mtest))
				nmeta++;
			if (nmeta) {
			    patadd(NULL, 0, nmeta, 0);
			    p = (Patprog)patout;
			    opnd = dupstring_wlen(opnd, oplen);
			    dst = patout + startoff;
			}

			while (oplen--) {
			    if (imeta(*opnd)) {
				*dst++ = Meta;
				*dst++ = *opnd++ ^ 32;
			    } else {
				*dst++ = *opnd++;
			    }
			}
			/* Only one string in a PAT_PURES, so now done. */
			break;
		    }
		}
		p->size = dst - patout;
		/* patmlen is really strlen.  We don't need a null. */
		p->patmlen = p->size - startoff;
	    } else {
		/* starting point info */
		if (P_OP(pscan) == P_EXACTLY && !p->globflags &&
		    P_LS_LEN(pscan))
		    p->patstartch = *P_LS_STR(pscan);
		/*
		 * Find the longest literal string in something expensive.
		 * This is itself not all that cheap if we have
		 * case-insensitive matching or approximation, so don't.
		 */
		if ((flags & P_HSTART) && !p->globflags) {
		    lng = NULL;
		    len = 0;
		    for (; pscan; pscan = PATNEXT(pscan))
			if (P_OP(pscan) == P_EXACTLY &&
			    P_LS_LEN(pscan) >= len) {
			    lng = P_LS_STR(pscan);
			    len = P_LS_LEN(pscan);
			}
		    if (lng) {
			p->mustoff = lng - patout;
			p->patmlen = len;
		    }
		}
	    }
	}
    }

    /*
     * The pattern was compiled in a fixed buffer:  unless told otherwise,
     * we stick the compiled pattern on the heap.  This is necessary
     * for files where we will often be compiling multiple segments at once.
     * But if we get the ZDUP flag we always put it in zalloc()ed memory.
     */
    if (patflags & PAT_ZDUP) {
	Patprog newp = (Patprog)zalloc(patsize);
	memcpy((char *)newp, (char *)p, patsize);
	p = newp;
    } else if (!(patflags & PAT_STATIC)) {
	Patprog newp = (Patprog)zhalloc(patsize);
	memcpy((char *)newp, (char *)p, patsize);
	p = newp;
    }

    if (endexp)
	*endexp = patparse;

    unqueue_signals();
    return p;
}

/*
 * Main body or parenthesized subexpression in pattern
 * Parenthesis (and any ksh_glob gubbins) will have been removed.
 */

/**/
static long
patcompswitch(int paren, int *flagp)
{
    long starter, br, ender, excsync = 0;
    int parno = 0;
    int flags, gfchanged = 0;
    long savglobflags = (long)patglobflags;
    Upat ptr;

    *flagp = 0;

    if (paren && (patglobflags & GF_BACKREF) && patnpar <= NSUBEXP) {
	/*
	 * parenthesized:  make an open node.
	 * We can only refer to the first nine parentheses.
	 * For any others, we just use P_OPEN on its own; there's
	 * no gain in arbitrarily limiting the number of parentheses.
	 */
	parno = patnpar++;
	starter = patnode(P_OPEN + parno);
    } else
	starter = 0;

    br = patnode(P_BRANCH);
    if (!patcompbranch(&flags, paren))
	return 0;
    if (patglobflags != (int)savglobflags)
	gfchanged++;
    if (starter)
	pattail(starter, br);
    else
	starter = br;

    *flagp |= flags & (P_HSTART|P_PURESTR);

    while (*patparse == zpc_chars[ZPC_BAR] ||
	   (*patparse == zpc_special[ZPC_TILDE] &&
	    (patparse[1] == '/' ||
	     !memchr(zpc_special, patparse[1], ZPC_SEG_COUNT)))) {
	int tilde = *patparse++ == zpc_special[ZPC_TILDE];
	long gfnode = 0, newbr;

	*flagp &= ~P_PURESTR;

	if (tilde) {
	    union upat up;
	    /* excsync remembers the P_EXCSYNC node before a chain of
	     * exclusions:  all point back to this.  only the
	     * original (non-excluded) branch gets a trailing P_EXCSYNC.
	     */
	    if (!excsync) {
		excsync = patnode(P_EXCSYNC);
		patoptail(br, excsync);
	    }
	    /*
	     * By default, approximations are turned off in exclusions:
	     * we need to do this here as otherwise the code compiling
	     * the exclusion doesn't know if the flags have really
	     * changed if the error count gets restored.
	     */
	    patglobflags &= ~0xff;
	    if (!(patflags & PAT_FILET) || paren) {
		br = patnode(P_EXCLUDE);
	    } else {
		/*
		 * At top level (paren == 0) in a file glob !(patflags
		 * &PAT_FILET) do the exclusion prepending the file path
		 * so far.  We need to flag this to avoid unnecessarily
		 * copying the path.
		 */
		br = patnode(P_EXCLUDP);
		patflags |= PAT_HAS_EXCLUDP;
	    }
	    up.p = NULL;
	    patadd((char *)&up, 0, sizeof(up), 0);
	    /* / is not treated as special if we are at top level */
	    if (!paren && zpc_special[ZPC_SLASH] == '/') {
		tilde++;
		zpc_special[ZPC_SLASH] = Marker;
	    }
	} else {
	    excsync = 0;
	    br = patnode(P_BRANCH);
	    /*
	     * The position of the following statements means globflags
	     * set in the main branch carry over to the exclusion.
	     */
	    if (!paren) {
		patglobflags = 0;
		if (((Patprog)patout)->globflags) {
		    /*
		     * If at top level, we need to reinitialize flags to zero,
		     * since (#i)foo|bar only applies to foo and we stuck
		     * the #i into the global flags.
		     * We could have done it so that they only got set in the
		     * first branch, but it's quite convenient having any
		     * global flags set in the header and not buried in the
		     * pattern.  (Or maybe it isn't and we should
		     * forget this bit and always stick in an explicit GFLAGS
		     * statement instead of using the header.)
		     * Also, this can't happen for file globs where there are
		     * no top-level |'s.
		     *
		     * No gfchanged, as nothing to follow branch at top
		     * level.
		     */
		    union upat up;
		    gfnode = patnode(P_GFLAGS);
		    up.l = patglobflags;
		    patadd((char *)&up, 0, sizeof(union upat), 0);
		}
	    } else {
		patglobflags = (int)savglobflags;
	    }
	}
	newbr = patcompbranch(&flags, paren);
	if (tilde == 2) {
	    /* restore special treatment of / */
	    zpc_special[ZPC_SLASH] = '/';
	}
	if (!newbr)
	    return 0;
	if (gfnode)
	    pattail(gfnode, newbr);
	if (!tilde && patglobflags != (int)savglobflags)
	    gfchanged++;
	pattail(starter, br);
	if (excsync)
	    patoptail(br, patnode(P_EXCEND));
	*flagp |= flags & P_HSTART;
    }

    /*
     * Make a closing node, hooking it to the end.
     * Note that we can't optimize P_NOTHING out here, since another
     * branch at that point would indicate the current choices continue,
     * which they don't.
     */
    ender = patnode(paren ? parno ? P_CLOSE+parno : P_NOTHING : P_END);
    pattail(starter, ender);

    /*
     * Hook the tails of the branches to the closing node,
     * except for exclusions which terminate where they are.
     */
    for (ptr = (Upat)patout + starter; ptr; ptr = PATNEXT(ptr))
	if (!P_ISEXCLUDE(ptr))
	    patoptail(ptr-(Upat)patout, ender);

    /* check for proper termination */
    if ((paren && *patparse++ != Outpar) ||
	(!paren && *patparse &&
	 !((patflags & PAT_FILE) && *patparse == '/')))
	return 0;

    if (paren && gfchanged) {
	/*
	 * Restore old values of flags when leaving parentheses.
	 * gfchanged detects a change in any branch (except exclusions
	 * which are separate), since we need to emit this even if
	 * a later branch happened to put the flags back.
	 */
	pattail(ender, patnode(P_GFLAGS));
	patglobflags = (int)savglobflags;
	patadd((char *)&savglobflags, 0, sizeof(long), 0);
    }

    return starter;
}

/*
 * Compile something ended by Bar, Outpar, Tilde, or end of string.
 * Note the BRANCH or EXCLUDE tag must already have been omitted:
 * this returns the position of the operand of that.
 */

/**/
static long
patcompbranch(int *flagp, int paren)
{
    long chain, latest = 0, starter;
    int flags = 0;

    *flagp = P_PURESTR;

    starter = chain = 0;
    while (!memchr(zpc_special, *patparse, ZPC_SEG_COUNT) ||
	   (*patparse == zpc_special[ZPC_TILDE] && patparse[1] != '/' &&
	    memchr(zpc_special, patparse[1], ZPC_SEG_COUNT))) {
	if ((*patparse == zpc_special[ZPC_INPAR] &&
	     patparse[1] == zpc_special[ZPC_HASH]) ||
	    (*patparse == zpc_special[ZPC_KSH_AT] && patparse[1] == Inpar &&
	     patparse[2] == zpc_special[ZPC_HASH])) {
	    /* Globbing flags. */
	    char *pp1 = patparse;
	    int oldglobflags = patglobflags, ignore;
	    long assert;
	    patparse += (*patparse == '@') ? 3 : 2;
	    if (!patgetglobflags(&patparse, &assert, &ignore))
		return 0;
	    if (!ignore) {
		if (assert) {
		    /*
		     * Start/end assertion looking like flags, but
		     * actually handled as a normal node
		     */
		    latest = patnode(assert);
		    flags = 0;
		} else {
		    if (pp1 == patstart) {
			/* Right at start of pattern, the simplest case.
			 * Put them into the flags and don't emit anything.
			 */
			((Patprog)patout)->globflags = patglobflags;
			continue;
		    } else if (!*patparse) {
			/* Right at the end, so just leave the flags for
			 * the next Patprog in the chain to pick up.
			 */
			break;
		    }
		    /*
		     * Otherwise, we have to stick them in as a pattern
		     * matching nothing.
		     */
		    if (oldglobflags != patglobflags) {
			/* Flags changed */
			union upat up;
			latest = patnode(P_GFLAGS);
			up.l = patglobflags;
			patadd((char *)&up, 0, sizeof(union upat), 0);
		    } else {
			/* No effect. */
			continue;
		    }
		}
	    } else if (!*patparse)
		break;
	    else
		continue;
	} else if (*patparse == zpc_special[ZPC_HAT]) {
	    /*
	     * ^pat:  anything but pat.  For proper backtracking,
	     * etc., we turn this into (*~pat), except without the
	     * parentheses.
	     */
	    patparse++;
	    latest = patcompnot(0, &flags);
	} else
	    latest = patcomppiece(&flags, paren);
	if (!latest)
	    return 0;
	if (!starter)
	    starter = latest;
	if (!(flags & P_PURESTR))
	    *flagp &= ~P_PURESTR;
	if (!chain)
	    *flagp |= flags & P_HSTART;
	else
	    pattail(chain, latest);
	chain = latest;
    }
    /* check if there was nothing in the loop, i.e. () */
    if (!chain)
	starter = patnode(P_NOTHING);

    return starter;
}

/* get glob flags, return 1 for success, 0 for failure */

/**/
int
patgetglobflags(char **strp, long *assertp, int *ignore)
{
    char *nptr, *ptr = *strp;
    zlong ret;

    *assertp = 0;
    *ignore = 1;
    /* (#X): assumes we are still positioned on the first X */
    for (; *ptr && *ptr != Outpar; ptr++) {
	if (*ptr == 'q') {
	    /* Glob qualifiers, ignored in pattern code */
	    while (*ptr && *ptr != Outpar)
		ptr++;
	    break;
	} else {
	    *ignore = 0;
	    switch (*ptr) {
	    case 'a':
		/* Approximate matching, max no. of errors follows */
		ret = zstrtol(++ptr, &nptr, 10);
		/*
		 * We can't have more than 254, because we need 255 to
		 * mark 254 errors in wbranch and exclude sync strings
		 * (hypothetically --- hope no-one tries it).
		 */
		if (ret < 0 || ret > 254 || ptr == nptr)
		    return 0;
		patglobflags = (patglobflags & ~0xff) | (ret & 0xff);
		ptr = nptr-1;
		break;

	    case 'l':
		/* Lowercase in pattern matches lower or upper in target */
		patglobflags = (patglobflags & ~GF_IGNCASE) | GF_LCMATCHUC;
		break;

	    case 'i':
		/* Fully case insensitive */
		patglobflags = (patglobflags & ~GF_LCMATCHUC) | GF_IGNCASE;
		break;

	    case 'I':
		/* Restore case sensitivity */
		patglobflags &= ~(GF_LCMATCHUC|GF_IGNCASE);
		break;

	    case 'b':
		/* Make backreferences */
		patglobflags |= GF_BACKREF;
		break;

	    case 'B':
		/* Don't make backreferences */
		patglobflags &= ~GF_BACKREF;
		break;

	    case 'm':
		/* Make references to complete match */
		patglobflags |= GF_MATCHREF;
		break;

	    case 'M':
		/* Don't */
		patglobflags &= ~GF_MATCHREF;
		break;

	    case 's':
		*assertp = P_ISSTART;
		break;

	    case 'e':
		*assertp = P_ISEND;
		break;

	    case 'u':
		patglobflags |= GF_MULTIBYTE;
		break;

	    case 'U':
		patglobflags &= ~GF_MULTIBYTE;
		break;

	    default:
		return 0;
	    }
	}
    }
    if (*ptr != Outpar)
	return 0;
    /* Start/end assertions must appear on their own. */
    if (*assertp && (*strp)[1] != Outpar)
	return 0;
    *strp = ptr + 1;
    return 1;
}


static const char *colon_stuffs[]  = {
    "alpha", "alnum", "ascii", "blank", "cntrl", "digit", "graph", 
    "lower", "print", "punct", "space", "upper", "xdigit", "IDENT",
    "IFS", "IFSSPACE", "WORD", "INCOMPLETE", "INVALID", NULL
};

/*
 * Handle the guts of a [:stuff:] character class element.
 * start is the beginning of "stuff" and len is its length.
 * This code is exported for the benefit of completion matching.
 */

/**/
mod_export int
range_type(char *start, int len)
{
    const char **csp;

    for (csp = colon_stuffs; *csp; csp++) {
	if (strlen(*csp) == len && !strncmp(start, *csp, len))
		return (csp - colon_stuffs) + PP_FIRST;
    }

    return PP_UNKWN;
}


/*
 * Convert the contents of a [...] or [^...] expression (just the
 * ... part) back into a string.  This is used by compfiles -p/-P
 * for some reason.  The compiled form (a metafied string) is
 * passed in rangestr.
 *
 * If outstr is non-NULL the compiled form is placed there.  It
 * must be sufficiently long.  A terminating NULL is appended.
 *
 * Return the length required, not including the terminating NULL.
 *
 * TODO: this is non-multibyte for now.  It will need to be defined
 * appropriately with MULTIBYTE_SUPPORT when the completion matching
 * code catches up.
 */

/**/
mod_export int
pattern_range_to_string(char *rangestr, char *outstr)
{
    int len = 0;

    while (*rangestr) {
	if (imeta(STOUC(*rangestr))) {
	    int swtype = STOUC(*rangestr) - STOUC(Meta);

	    if (swtype == 0) {
		/* Ordindary metafied character */
		if (outstr)
		{
		    *outstr++ = Meta;
		    *outstr++ = rangestr[1] ^ 32;
		}
		len += 2;
		rangestr += 2;
	    } else if (swtype == PP_RANGE) {
		/* X-Y range */
		int i;

		for (i = 0; i < 2; i++) {
		    if (*rangestr == Meta) {
			if (outstr) {
			    *outstr++ = Meta;
			    *outstr++ = rangestr[1];
			}
			len += 2;
			rangestr += 2;
		    } else {
			if (outstr)
			    *outstr++ = *rangestr;
			len++;
			rangestr++;
		    }

		    if (i == 0) {
			if (outstr)
			    *outstr++ = '-';
			len++;
		    }
		}
	    } else if (swtype >= PP_FIRST && swtype <= PP_LAST) {
		/* [:stuff:]; we need to output [: and :] */
		const char *found = colon_stuffs[swtype - PP_FIRST];
		int newlen = strlen(found);
		if (outstr) {
		    strcpy(outstr, "[:");
		    outstr += 2;
		    memcpy(outstr, found, newlen);
		    outstr += newlen;
		    strcpy(outstr, ":]");
		    outstr += 2;
		}
		len += newlen + 4;
		rangestr++;
	    } else {
		/* shouldn't happen */
		DPUTS(1, "BUG: unknown PP_ code in pattern range");
		rangestr++;
	    }
	} else {
	    /* ordinary character, guaranteed no Meta handling needed */
	    if (outstr)
		*outstr++ = *rangestr;
	    len++;
	    rangestr++;
	}
    }

    if (outstr)
	*outstr = '\0';
    return len;
}

/*
 * compile a chunk such as a literal string or a [...] followed
 * by a possible hash operator
 */

/**/
static long
patcomppiece(int *flagp, int paren)
{
    long starter = 0, next, op, opnd;
    int flags, flags2, kshchar, len, ch, patch, nmeta;
    int hash, count;
    union upat up;
    char *nptr, *str0, *ptr, *patprev;
    zrange_t from = 0, to;
    char *charstart;

    flags = 0;
    str0 = patprev = patparse;
    for (;;) {
	/*
	 * Check if we have a string. First, we need to make sure
	 * the string doesn't introduce a ksh-like parenthesized expression.
	 */
	kshchar = '\0';
	if (*patparse && patparse[1] == Inpar) {
	    if (*patparse == zpc_special[ZPC_KSH_PLUS])
		kshchar = STOUC('+');
	    else if (*patparse == zpc_special[ZPC_KSH_BANG])
		kshchar = STOUC('!');
	    else if (*patparse == zpc_special[ZPC_KSH_BANG2])
		kshchar = STOUC('!');
	    else if (*patparse == zpc_special[ZPC_KSH_AT])
		kshchar = STOUC('@');
	    else if (*patparse == zpc_special[ZPC_KSH_STAR])
		kshchar = STOUC('*');
	    else if (*patparse == zpc_special[ZPC_KSH_QUEST])
		kshchar = STOUC('?');
	}

	/*
	 * If '(' is disabled as a pattern char, allow ')' as
	 * an ordinary string character if there are no parentheses to
	 * close.  Don't allow it otherwise, it changes the syntax.
	 */
	if (zpc_special[ZPC_INPAR] != Marker || *patparse != Outpar ||
	    paren) {
	    /*
	     * End of string (or no string at all) if ksh-type parentheses,
	     * or special character, unless that character is a tilde and
	     * the character following is an end-of-segment character.  Thus
	     * tildes are not special if there is nothing following to
	     * be excluded.
	     *
	     * Don't look for X()-style kshglobs at this point; we've
	     * checked above for the case with parentheses and we don't
	     * want to match without parentheses.
	     */
	    if (kshchar ||
		(memchr(zpc_special, *patparse, ZPC_NO_KSH_GLOB) &&
		 (*patparse != zpc_special[ZPC_TILDE] ||
		  patparse[1] == '/' ||
		  !memchr(zpc_special, patparse[1], ZPC_SEG_COUNT)))) {
		break;
	    }
    	}

	/* Remember the previous character for backtracking */
	patprev = patparse;
	METACHARINC(patparse);
    }

    if (patparse > str0) {
	long slen = patparse - str0;
	int morelen;

	/* Ordinary string: cancel kshchar lookahead */
	kshchar = '\0';
	/*
	 * Assume it matches a simple string until we find otherwise.
	 */
	flags |= P_PURESTR;
	DPUTS(patparse == str0, "BUG: matched nothing in patcomppiece.");
	/* more than one character matched? */
	morelen = (patprev > str0);
	/*
	 * If we have more than one character, a following hash
	 * or (#c...) only applies to the last, so backtrack one character.
	 */
	if ((*patparse == zpc_special[ZPC_HASH] ||
	     (*patparse == zpc_special[ZPC_INPAR] &&
	      patparse[1] == zpc_special[ZPC_HASH] &&
	      patparse[2] == 'c') ||
	     (*patparse == zpc_special[ZPC_KSH_AT] &&
	      patparse[1] == Inpar &&
	      patparse[2] == zpc_special[ZPC_HASH] &&
	      patparse[3] == 'c')) && morelen)
	    patparse = patprev;
	/*
	 * If len is 1, we can't have an active # following, so doesn't
	 * matter that we don't make X in `XX#' simple.
	 */
	if (!morelen)
	    flags |= P_SIMPLE;
	starter = patnode(P_EXACTLY);

	/* Get length of string without metafication. */
	nmeta = 0;
	/* inherited from domatch, but why, exactly? */
	if (*str0 == Nularg)
	    str0++;
	for (ptr = str0; ptr < patparse; ptr++) {
	    if (*ptr == Meta) {
		nmeta++;
		ptr++;
	    }
	}
	slen = (patparse - str0) - nmeta;
	/* First add length, which is a long */
	patadd((char *)&slen, 0, sizeof(long), 0);
	/*
	 * Then the string, not null terminated.
	 * Unmetafy and untokenize; pass the final length,
	 * which is what we need to allocate, i.e. not including
	 * a count for each Meta in the string.
	 */
	patadd(str0, 0, slen, PA_UNMETA);
	nptr = P_LS_STR((Upat)patout + starter);
	/*
	 * It's much simpler to turn off pure string mode for
	 * any case-insensitive or approximate matching; usually,
	 * that is correct, or they wouldn't have been turned on.
	 * However, we need to make sure we match a "." or ".."
	 * in a file name as a pure string.  There's a minor bug
	 * that this will also apply to something like
	 * ..(#a1).. (i.e. the (#a1) has no effect), but if you're
	 * going to write funny patterns, you get no sympathy from me.
	 */
	if (patglobflags &
#ifdef __CYGWIN__
	    /*
	     * As above: don't use pattern matching for files
	     * just because of case insensitivity if file system
	     * is known to be case insensitive.
	     *
	     * This is known to be necessary in at least one case:
	     * if "mount -c /" is in effect, so that drives appear
	     * directly under / instead of the usual /cygdrive, they
	     * aren't shown by readdir().  So it's vital we don't use
	     * globbing to find "/c", since that'll fail.
	     */
	    ((patflags & PAT_FILE) ?
	    (0xFF|GF_LCMATCHUC) :
	    (0xFF|GF_LCMATCHUC|GF_IGNCASE))
#else
	    (0xFF|GF_LCMATCHUC|GF_IGNCASE)
#endif
	    ) {
	    if (!(patflags & PAT_FILE))
		flags &= ~P_PURESTR;
	    else if (!(nptr[0] == '.' &&
		       (slen == 1 || (nptr[1] == '.' && slen == 2))))
		flags &= ~P_PURESTR;
	}
    } else {
	if (kshchar)
	    patparse++;

	patch = *patparse;
	METACHARINC(patparse);
	switch(patch) {
	case Quest:
	    DPUTS(zpc_special[ZPC_QUEST] == Marker,
		  "Treating '?' as pattern character although disabled");
	    flags |= P_SIMPLE;
	    starter = patnode(P_ANY);
	    break;
	case Star:
	    DPUTS(zpc_special[ZPC_STAR] == Marker,
		  "Treating '*' as pattern character although disabled");
	    /* kshchar is used as a sign that we can't have #'s. */
	    kshchar = -1;
	    starter = patnode(P_STAR);
	    break;
	case Inbrack:
	    DPUTS(zpc_special[ZPC_INBRACK] == Marker,
		  "Treating '[' as pattern character although disabled");
	    flags |= P_SIMPLE;
	    if (*patparse == Hat || *patparse == Bang) {
		patparse++;
		starter = patnode(P_ANYBUT);
	    } else
		starter = patnode(P_ANYOF);
	    /*
	     * []...] means match a "]" or other included characters.
	     * However, to be a bit helpful and for compatibility
	     * with other shells, don't take in that sense if
	     * there's no further "]".  That's still imperfect,
	     * but it's all we can do --- we're required to
	     * treat [$var]*[$var]with empty var as [ ... ]
	     * containing "]*[".
	     */
	    if (*patparse == Outbrack && strchr(patparse+1, Outbrack)) {
		patparse++;
		patadd(NULL, ']', 1, PA_NOALIGN);
	    }
	    while (*patparse && *patparse != Outbrack) {
		/* Meta is not a token */
		if (*patparse == Inbrack && patparse[1] == ':' &&
			(nptr = strchr(patparse+2, ':')) &&
			nptr[1] == Outbrack) {
			/* Posix range. */
			patparse += 2;
			len = nptr - patparse;
			ch = range_type(patparse, len);
			patparse = nptr + 2;
			if (ch != PP_UNKWN)
			    patadd(NULL, STOUC(Meta) + ch, 1, PA_NOALIGN);
			continue;
		}
		charstart = patparse;
		METACHARINC(patparse);

		if (*patparse == Dash && patparse[1] &&
		    patparse[1] != Outbrack) {
		    patadd(NULL, STOUC(Meta)+PP_RANGE, 1, PA_NOALIGN);
		    if (itok(*charstart)) {
			patadd(0, STOUC(ztokens[*charstart - Pound]), 1,
			       PA_NOALIGN);
		    } else {
			patadd(charstart, 0, patparse-charstart, PA_NOALIGN);
		    }
		    charstart = ++patparse;	/* skip Dash token */
		    METACHARINC(patparse);
		}
		if (itok(*charstart)) {
		    patadd(0, STOUC(ztokens[*charstart - Pound]), 1,
			   PA_NOALIGN);
		} else {
		    patadd(charstart, 0, patparse-charstart, PA_NOALIGN);
		}
	    }
	    if (*patparse != Outbrack)
		return 0;
	    patparse++;
	    /* terminate null string and fix alignment */
	    patadd(NULL, 0, 1, 0);
	    break;
	case Inpar:
	    DPUTS(!kshchar && zpc_special[ZPC_INPAR] == Marker,
		  "Treating '(' as pattern character although disabled");
	    DPUTS(isset(SHGLOB) && !kshchar,
		  "Treating bare '(' as pattern character with SHGLOB");
	    if (kshchar == '!') {
		/* This is nasty, we should really either handle all
		 * kshglobbing below or here.  But most of the
		 * others look like non-ksh patterns, while this one
		 * doesn't, so we handle it here and leave the rest.
		 * We treat it like an extendedglob ^, except that
		 * it goes into parentheses.
		 *
		 * If we did do kshglob here, we could support
		 * the old behaviour that things like !(foo)##
		 * work, but it makes the code more complicated at
		 * the expense of allowing the user to do things
		 * they shouldn't.
		 */
		if (!(starter = patcompnot(1, &flags2)))
		    return 0;
	    } else if (!(starter = patcompswitch(1, &flags2)))
		return 0;
	    flags |= flags2 & P_HSTART;
	    break;
	case Inang:
	    /* Numeric glob */
	    DPUTS(zpc_special[ZPC_INANG] == Marker,
		  "Treating '<' as pattern character although disabled");
	    DPUTS(isset(SHGLOB), "Treating <..> as numeric range with SHGLOB");
	    len = 0;		/* beginning present 1, end present 2 */
	    if (idigit(*patparse)) {
		from = (zrange_t) zstrtol((char *)patparse,
					 (char **)&nptr, 10);
		patparse = nptr;
		len |= 1;
	    }
	    DPUTS(!IS_DASH(*patparse), "BUG: - missing from numeric glob");
	    patparse++;
	    if (idigit(*patparse)) {
		to = (zrange_t) zstrtol((char *)patparse,
					  (char **)&nptr, 10);
		patparse = nptr;
		len |= 2;
	    }
	    if (*patparse != Outang)
		return 0;
	    patparse++;
	    switch(len) {
	    case 3:
		starter = patnode(P_NUMRNG);
		patadd((char *)&from, 0, sizeof(from), 0);
		patadd((char *)&to, 0, sizeof(to), 0);
		break;
	    case 2:
		starter = patnode(P_NUMTO);
		patadd((char *)&to, 0, sizeof(to), 0);
		break;
	    case 1:
		starter = patnode(P_NUMFROM);
		patadd((char *)&from, 0, sizeof(from), 0);
		break;
	    case 0:
		starter = patnode(P_NUMANY);
		break;
	    }
	    /* This can't be simple, because it isn't.
	     * Mention in manual that matching digits with [...]
	     * is more efficient.
	     */
	    break;
	case Pound:
	    DPUTS(zpc_special[ZPC_HASH] == Marker,
		  "Treating '#' as pattern character although disabled");
	    DPUTS(!isset(EXTENDEDGLOB), "BUG: # not treated as string");
	    /*
	     * A hash here is an error; it should follow something
	     * repeatable.
	     */
	    return 0;
	    break;
	case Bnullkeep:
	    /*
	     * Marker for restoring a backslash in output:
	     * does not match a character.
	     */
	    next = patcomppiece(flagp, paren);
	    /*
	     * Can't match a pure string since we need to do this
	     * as multiple chunks.
	     */
	    *flagp &= ~P_PURESTR;
	    return next;
	    break;
#ifdef DEBUG
	default:
	    dputs("BUG: character not handled in patcomppiece");
	    return 0;
	    break;
#endif
	}
    }

    count = 0;
    if (!(hash = (*patparse == zpc_special[ZPC_HASH])) &&
	!(count = ((*patparse == zpc_special[ZPC_INPAR] &&
		    patparse[1] == zpc_special[ZPC_HASH] &&
		    patparse[2] == 'c') ||
		   (*patparse == zpc_special[ZPC_KSH_AT] &&
		    patparse[1] == Inpar &&
		    patparse[2] == zpc_special[ZPC_HASH] &&
		    patparse[3] == 'c'))) &&
	(kshchar <= 0 || kshchar == '@' || kshchar == '!')) {
	*flagp = flags;
	return starter;
    }

    /* too much at once doesn't currently work */
    if (kshchar && (hash || count))
	return 0;

    if (kshchar == '*') {
	op = P_ONEHASH;
	*flagp = P_HSTART;
    } else if (kshchar == '+') {
	op = P_TWOHASH;
	*flagp = P_HSTART;
    } else if (kshchar == '?') {
	op = 0;
	*flagp = 0;
    } else if (count) {
	op = P_COUNT;
	patparse += 3;
	*flagp = P_HSTART;
    } else if (*++patparse == zpc_special[ZPC_HASH]) {
	op = P_TWOHASH;
	patparse++;
	*flagp = P_HSTART;
    } else {
	op = P_ONEHASH;
	*flagp = P_HSTART;
    }

    /*
     * Note optimizations with pointers into P_NOTHING branches:  some
     * should logically point to next node after current piece.
     *
     * Backtracking is also encoded in a slightly obscure way:  the
     * code emitted ensures we test the non-empty branch of complex
     * patterns before the empty branch on each repetition.  Hence
     * each time we fail on a non-empty branch, we try the empty branch,
     * which is equivalent to backtracking.
     */
    if (op == P_COUNT) {
	/* (#cN,M) */
	union upat countargs[P_CT_OPERAND];
	char *opp = patparse;

	countargs[0].l = P_COUNT;
	countargs[P_CT_CURRENT].l = 0L;
	countargs[P_CT_MIN].l = (long)zstrtol(patparse, &patparse, 10);
	if (patparse == opp) {
	    /* missing number treated as zero */
	    countargs[P_CT_MIN].l = 0L;
	}
	if (*patparse != ',' && *patparse != Comma) {
	    /* either max = min or error */
	    if (*patparse != Outpar)
		return 0;
	    countargs[P_CT_MAX].l = countargs[P_CT_MIN].l;
	} else {
	    opp = ++patparse;
	    countargs[P_CT_MAX].l = (long)zstrtol(patparse, &patparse, 10);
	    if (*patparse != Outpar)
		return 0;
	    if (patparse == opp) {
		/* missing number treated as infinity: record as -1 */
		countargs[P_CT_MAX].l = -1L;
	    }
	}
	patparse++;
	countargs[P_CT_PTR].p = NULL;
	/* Mark this chain as a min/max count... */
	patinsert(P_COUNTSTART, starter, (char *)countargs, sizeof(countargs));
	/*
	 * The next of the operand is a loop back to the P_COUNT.  This is
	 * how we get recursion for the count.  We don't loop back to
	 * the P_COUNTSTART; that's used for initialising the count
	 * and saving and restoring the count for any enclosing use
	 * of the match.
	 */
	opnd = P_OPERAND(starter) + P_CT_OPERAND;
	pattail(opnd, patnode(P_BACK));
	pattail(opnd, P_OPERAND(starter));
	/*
	 * The next of the counter operators is what follows the
	 * closure.
	 * This handles matching of the tail.
	 */
	next = patnode(P_NOTHING);
	pattail(starter, next);
	pattail(P_OPERAND(starter), next);
    } else if ((flags & P_SIMPLE) && (op == P_ONEHASH || op == P_TWOHASH) &&
	P_OP((Upat)patout+starter) == P_ANY) {
	/* Optimize ?# to *.  Silly thing to do, since who would use
	 * use ?# ? But it makes the later code shorter.
	 */
	Upat uptr = (Upat)patout + starter;
	if (op == P_TWOHASH) {
	    /* ?## becomes ?* */
	    uptr->l = (uptr->l & ~0xff) | P_ANY;
	    pattail(starter, patnode(P_STAR));
	} else {
	    uptr->l = (uptr->l & ~0xff) | P_STAR;
	}
    } else if ((flags & P_SIMPLE) && op && !(patglobflags & 0xff)) {
	/* Simplify, but not if we need to look for approximations. */
	patinsert(op, starter, NULL, 0);
    } else if (op == P_ONEHASH) {
	/* Emit x# as (x&|), where & means "self". */
	up.p = NULL;
	patinsert(P_WBRANCH, starter, (char *)&up, sizeof(up));
	                                      /* Either x */
	patoptail(starter, patnode(P_BACK));  /* and loop */
	patoptail(starter, starter);	      /* back */
	pattail(starter, patnode(P_BRANCH));  /* or */
	pattail(starter, patnode(P_NOTHING)); /* null. */
    } else if (op == P_TWOHASH) {
	/* Emit x## as x(&|) where & means "self". */
	next = patnode(P_WBRANCH);	      /* Either */
	up.p = NULL;
	patadd((char *)&up, 0, sizeof(up), 0);
	pattail(starter, next);
	pattail(patnode(P_BACK), starter);    /* loop back */
	pattail(next, patnode(P_BRANCH));     /* or */
	pattail(starter, patnode(P_NOTHING)); /* null. */
    } else if (kshchar == '?') {
	/* Emit ?(x) as (x|) */
	patinsert(P_BRANCH, starter, NULL, 0); /* Either x */
	pattail(starter, patnode(P_BRANCH));   /* or */
	next = patnode(P_NOTHING);	       /* null */
	pattail(starter, next);
	patoptail(starter, next);
    }
    if (*patparse == zpc_special[ZPC_HASH])
	return 0;

    return starter;
}

/*
 * Turn a ^foo (paren = 0) or !(foo) (paren = 1) into *~foo with
 * parentheses if necessary.   As you see, that's really quite easy.
 */

/**/
static long
patcompnot(int paren, int *flagsp)
{
    union upat up;
    long excsync, br, excl, n, starter;
    int dummy;

    /* Here, we're matching a star at the start. */
    *flagsp = P_HSTART;

    starter = patnode(P_BRANCH);
    br = patnode(P_STAR);
    excsync = patnode(P_EXCSYNC);
    pattail(br, excsync);
    pattail(starter, excl = patnode(P_EXCLUDE));
    up.p = NULL;
    patadd((char *)&up, 0, sizeof(up), 0);
    if (!(br = (paren ? patcompswitch(1, &dummy) : patcompbranch(&dummy, 0))))
	return 0;
    pattail(br, patnode(P_EXCEND));
    n = patnode(P_NOTHING); /* just so much easier */
    pattail(excsync, n);
    pattail(excl, n);

    return starter;
}

/* Emit a node */

/**/
static long
patnode(long op)
{
    long starter = (Upat)patcode - (Upat)patout;
    union upat up;

    up.l = op;
    patadd((char *)&up, 0, sizeof(union upat), 0);
    return starter;
}

/*
 * insert an operator in front of an already emitted operand:
 * we relocate the operand.  there had better be nothing else after.
 */

/**/
static void
patinsert(long op, int opnd, char *xtra, int sz)
{
    char *src, *dst, *opdst;
    union upat buf, *lptr;

    buf.l = 0;
    patadd((char *)&buf, 0, sizeof(buf), 0);
    if (sz)
	patadd(xtra, 0, sz, 0);
    src = patcode - sizeof(union upat) - sz;
    dst = patcode;
    opdst = patout + opnd * sizeof(union upat);
    while (src > opdst)
	*--dst = *--src;

    /* A cast can't be an lvalue */
    lptr = (Upat)opdst;
    lptr->l = op;
    opdst += sizeof(union upat);
    while (sz--)
	*opdst++ = *xtra++;
}

/* set the 'next' pointer at the end of a node chain */

/**/
static void
pattail(long p, long val)
{
    Upat scan, temp;
    long offset;

    scan = (Upat)patout + p;
    for (;;) {
	if (!(temp = PATNEXT(scan)))
	    break;
	scan = temp;
    }

    offset = (P_OP(scan) == P_BACK)
	? (scan - (Upat)patout) - val : val - (scan - (Upat)patout);

    scan->l |= offset << 8;
}

/* do pattail, but on operand of first argument; nop if operandless */

/**/
static void
patoptail(long p, long val)
{
    Upat ptr = (Upat)patout + p;
    int op = P_OP(ptr);
    if (!p || !P_ISBRANCH(ptr))
	return;
    if (op == P_BRANCH)
	pattail(P_OPERAND(p), val);
    else
	pattail(P_OPERAND(p) + 1, val);
}


/*
 * Run a pattern.
 */
struct rpat {
    char *patinstart;		/* Start of input string */
    char *patinend;		/* End of input string */
    char *patinput;		/* String input pointer */
    char *patinpath;		/* Full path for use with ~ exclusions */
    int   patinlen;		/* Length of last successful match.
				 * Includes count of Meta characters.
				 */

    char *patbeginp[NSUBEXP];	/* Pointer to backref beginnings */
    char *patendp[NSUBEXP];	/* Pointer to backref ends */
    int parsfound;		/* parentheses (with backrefs) found */

    int globdots;		/* Glob initial dots? */
};

static struct rpat pattrystate;

#define patinstart	(pattrystate.patinstart)
#define patinend	(pattrystate.patinend)
#define patinput	(pattrystate.patinput)
#define patinpath	(pattrystate.patinpath)
#define patinlen	(pattrystate.patinlen)
#define patbeginp	(pattrystate.patbeginp)
#define patendp		(pattrystate.patendp)
#define parsfound	(pattrystate.parsfound)
#define globdots	(pattrystate.globdots)


/*
 * Character functions operating on unmetafied strings.
 */
#ifdef MULTIBYTE_SUPPORT

/* Get a character from the start point in a string */
#define CHARREF(x, y)	charref((x), (y), (int *)NULL)
static wchar_t
charref(char *x, char *y, int *zmb_ind)
{
    wchar_t wc;
    size_t ret;

    if (!(patglobflags & GF_MULTIBYTE) || !(STOUC(*x) & 0x80))
	return (wchar_t) STOUC(*x);

    ret = mbrtowc(&wc, x, y-x, &shiftstate);

    if (ret == MB_INVALID || ret == MB_INCOMPLETE) {
	/* Error. */
	/* Reset the shift state for next time. */
	memset(&shiftstate, 0, sizeof(shiftstate));
	if (zmb_ind)
	    *zmb_ind = (ret == MB_INVALID) ? ZMB_INVALID : ZMB_INCOMPLETE;
	return WCHAR_INVALID(*x);
    }

    if (zmb_ind)
	*zmb_ind = ZMB_VALID;
    return wc;
}

/* Get  a pointer to the next character */
#define CHARNEXT(x, y)	charnext((x), (y))
static char *
charnext(char *x, char *y)
{
    wchar_t wc;
    size_t ret;

    if (!(patglobflags & GF_MULTIBYTE) || !(STOUC(*x) & 0x80))
	return x + 1;

    ret = mbrtowc(&wc, x, y-x, &shiftstate);

    if (ret == MB_INVALID || ret == MB_INCOMPLETE) {
	/* Error.  Treat as single byte. */
	/* Reset the shift state for next time. */
	memset(&shiftstate, 0, sizeof(shiftstate));
	return x + 1;
    }

    /* Nulls here are normal characters */
    return x + (ret ? ret : 1);
}

/* Increment a pointer past the current character. */
#define CHARINC(x, y)	((x) = charnext((x), (y)))


/* Get a character and increment */
#define CHARREFINC(x, y, z)	charrefinc(&(x), (y), (z))
static wchar_t
charrefinc(char **x, char *y, int *z)
{
    wchar_t wc;
    size_t ret;

    if (!(patglobflags & GF_MULTIBYTE) || !(STOUC(**x) & 0x80))
	return (wchar_t) STOUC(*(*x)++);

    ret = mbrtowc(&wc, *x, y-*x, &shiftstate);

    if (ret == MB_INVALID || ret == MB_INCOMPLETE) {
	/* Error.  Treat as single byte, but flag. */
	*z = 1;
	/* Reset the shift state for next time. */
	memset(&shiftstate, 0, sizeof(shiftstate));
	return WCHAR_INVALID(*(*x)++);
    }

    /* Nulls here are normal characters */
    *x += ret ? ret : 1;

    return wc;
}


/*
 * Counter the number of characters between two pointers, smaller first
 *
 * This is used when setting values in parameters, so we obey
 * the MULTIBYTE option (even if it's been overridden locally).
 */
#define CHARSUB(x,y)	charsub(x, y)
static ptrdiff_t
charsub(char *x, char *y)
{
    ptrdiff_t res = 0;
    size_t ret;
    wchar_t wc;

    if (!isset(MULTIBYTE))
	return y - x;

    while (x < y) {
	ret = mbrtowc(&wc, x, y-x, &shiftstate);

	if (ret == MB_INVALID || ret == MB_INCOMPLETE) {
	    /* Error.  Treat remainder as single characters */
	    /* Reset the shift state for next time. */
	    memset(&shiftstate, 0, sizeof(shiftstate));
	    return res + (y - x);
	}

	/* Treat nulls as normal characters */
	if (!ret)
	    ret = 1;
	res++;
	x += ret;
    }

    return res;
}

#else /* no MULTIBYTE_SUPPORT */

/* Get a character from the start point in a string */
#define CHARREF(x, y)	(STOUC(*(x)))
/* Get  a pointer to the next character */
#define CHARNEXT(x, y)	((x)+1)
/* Increment a pointer past the current character. */
#define CHARINC(x, y)	((x)++)
/* Get a character and increment */
#define CHARREFINC(x, y, z)	(STOUC(*(x)++))
/* Counter the number of characters between two pointers, smaller first */
#define CHARSUB(x,y)	((y) - (x))

#endif /* MULTIBYTE_SUPPORT */

/*
 * The following need to be accessed in the globbing scanner for
 * a multi-component file path.  See horror story in glob.c.
 */
/**/
int errsfound;				/* Total error count so far */

/**/
int forceerrs;				/* Forced maximum error count */

/*
 * exactpos is used to remember how far down an exact string we have
 * matched, if we are doing approximation and can therefore redo from
 * the same point; we never need to otherwise.
 *
 * exactend is a pointer to the end of the string, which isn't
 * null-terminated.
 */
static char *exactpos, *exactend;

/**/
void
pattrystart(void)
{
    forceerrs = -1;
    errsfound = 0;
}

/*
 * Fix up string length stuff.
 *
 * If we call patallocstr() with "force" to set things up early, it's
 * done there, else it's done in pattryrefs().  The reason for the
 * difference is in the latter case we may not be relying on
 * patallocstr() having an effect.
 */

/**/
static void
patmungestring(char **string, int *stringlen, int *unmetalenin)
{
    /*
     * Special signalling of empty tokenised string.
     */
    if (*stringlen > 0 && **string == Nularg) {
	(*string)++;
	/*
	 * If we don't have an unmetafied length
	 * and need it (we may not) we'll get it later.
	 */
	if (*unmetalenin > 0)
	    (*unmetalenin)--;
	if (*stringlen > 0)
	    (*stringlen)--;
    }

    /* Ensure we have a metafied length */
    if (*stringlen < 0)
	*stringlen = strlen(*string);
}

/*
 * Allocate memory for pattern match.  Note this is specific to use
 * of pattern *and* trial string.
 *
 * Unmetafy a trial string for use in pattern matching, if needed.
 *
 * If it is needed, returns a heap allocated string; if not needed,
 * returns NULL.
 *
 * prog is the pattern to be executed.
 * string is the metafied trial string.
 * stringlen is it's length; it will be calculated if it's negative
 *   (this is a simple strlen()).
 * unmetalen is the unmetafied length of the string, may be -1.
 * force is 1 if we always unmetafy: this is useful if we are going
 *   to try again with different versions of the string.  If this is
 *   called from pattryrefs() we don't force unmetafication as it won't
 *   be optimal.  This option should be used if the resulting
 *   patstralloc is going to be passed to pattrylen() / pattryrefs().
 * In patstralloc (supplied by caller, must last until last pattry is done)
 *  unmetalen is the unmetafied length of the string; it will be
 *    calculated if the input value is negative.
 *  unmetalenp is the umetafied length of a path segment preceding
 *    the trial string needed for file mananagement; it is calculated as
 *    needed so does not need to be initialised.
 *  alloced is the memory allocated on the heap --- same as return value from
 *    function.
 */
/**/
mod_export
char *patallocstr(Patprog prog, char *string, int stringlen, int unmetalen,
		  int force, Patstralloc patstralloc)
{
    int needfullpath;

    if (force)
	patmungestring(&string, &stringlen, &unmetalen);

    /*
     * For a top-level ~-exclusion, we will need the full
     * path to exclude, so copy the path so far and append the
     * current test string.
     */
    needfullpath = (prog->flags & PAT_HAS_EXCLUDP) && pathpos;

    /* Get the length of the full string when unmetafied. */
    if (unmetalen < 0)
	patstralloc->unmetalen = ztrsub(string + stringlen, string);
    else
	patstralloc->unmetalen = unmetalen;
    if (needfullpath) {
	patstralloc->unmetalenp = ztrsub(pathbuf + pathpos, pathbuf);
	if (!patstralloc->unmetalenp)
	    needfullpath = 0;
    } else
	patstralloc->unmetalenp = 0;
    /* Initialise cache area */
    patstralloc->progstrunmeta = NULL;
    patstralloc->progstrunmetalen = 0;

    DPUTS(needfullpath && (prog->flags & (PAT_PURES|PAT_ANY)),
	  "rum sort of file exclusion");
    /*
     * Partly for efficiency, and partly for the convenience of
     * globbing, we don't unmetafy pure string patterns, and
     * there's no reason to if the pattern is just a *.
     */
    if (force ||
	(!(prog->flags & (PAT_PURES|PAT_ANY))
	 && (needfullpath || patstralloc->unmetalen != stringlen))) {
	/*
	 * We need to copy if we need to prepend the path so far
	 * (in which case we copy both chunks), or if we have
	 * Meta characters.
	 */
	char *dst, *ptr;
	int i, icopy, ncopy;

	dst = patstralloc->alloced =
	    zhalloc(patstralloc->unmetalen + patstralloc->unmetalenp);

	if (needfullpath) {
	    /* loop twice, copy path buffer first time */
	    ptr = pathbuf;
	    ncopy = patstralloc->unmetalenp;
	} else {
	    /* just loop once, copy string with unmetafication */
	    ptr = string;
	    ncopy = patstralloc->unmetalen;
	}
	for (icopy = 0; icopy < 2; icopy++) {
	    for (i = 0; i < ncopy; i++) {
		if (*ptr == Meta) {
		    ptr++;
		    *dst++ = *ptr++ ^ 32;
		} else {
		    *dst++ = *ptr++;
		}
	    }
	    if (!needfullpath)
		break;
	    /* next time append test string to path so far */
	    ptr = string;
	    ncopy = patstralloc->unmetalen;
	}
    }
    else
    {
	patstralloc->alloced = NULL;
    }

    return patstralloc->alloced;
}


/*
 * Test prog against null-terminated, metafied string.
 */

/**/
mod_export int
pattry(Patprog prog, char *string)
{
    return pattryrefs(prog, string, -1, -1, NULL, 0, NULL, NULL, NULL);
}

/*
 * Test prog against string of given length, no null termination
 * but still metafied at this point.  offset gives an offset
 * to include in reported match indices
 */

/**/
mod_export int
pattrylen(Patprog prog, char *string, int len, int unmetalen,
	  Patstralloc patstralloc, int offset)
{
    return pattryrefs(prog, string, len, unmetalen, patstralloc, offset,
		      NULL, NULL, NULL);
}

/*
 * Test prog against string with given lengths.  The input
 * string is metafied; stringlen is the raw string length, and
 * unmetalen the number of characters in the original string (some
 * of which may now be metafied).  Either value may be -1
 * to indicate a null-terminated string which will be counted.  Note
 * there may be a severe penalty for this if a lot of matching is done
 * on one string.
 *
 * If patstralloc is not NULL it is used to optimise unmetafication
 * of a trial string that may be passed (or any substring may be passed) to
 * pattryrefs multiple times or the same pattern (N.B. so patstralloc
 * depends on both prog *and* the trial string).  This should only be
 * done if there is no path prefix (pathpos == 0) as otherwise the path
 * buffer and unmetafied string may not match.  To do this,
 * patallocstr() is called (use force = 1 to ensure it is always
 * unmetafied); paststralloc points to existing storage. Memory is
 * on the heap.
 *
 * patstralloc->alloced and patstralloc->unmetalen contain the
 * unmetafied string and its length.  In that case, the rules for the
 * earlier arguments change:
 * - string is an unmetafied string
 * - stringlen is its unmetafied (i.e. actual) length
 * - unmetalenin is not used.
 * string and stringlen may refer to arbitrary substrings of
 * patstralloc->alloced without any internal modification to patstralloc.
 *
 * patoffset is the position in the original string (not seen by
 * the pattern module) at which we are trying to match.
 * This is added in to the positions recorded in patbeginp and patendp
 * when we are looking for substrings.  Currently this only happens
 * in the parameter substitution code.  It refers to a real character
 * offset, i.e. is already in the form ready for presentation to the
 * general public --- this is necessary as we don't have the
 * information to convert it down here.
 *
 * Note this is a character offset, i.e. a single possibly metafied and
 * possibly multibyte character counts as 1.
 *
 * The last three arguments are used to report the positions for the
 * backreferences. On entry, *nump should contain the maximum number
 * of positions to report.  In this case the match, mbegin, mend
 * arrays are not altered.
 *
 * If nump is NULL but endp is not NULL, then *endp is set to the
 * end position of the match, taking into account patinstart.
 */

/**/
mod_export int
pattryrefs(Patprog prog, char *string, int stringlen, int unmetalenin,
	   Patstralloc patstralloc, int patoffset,
	   int *nump, int *begp, int *endp)
{
    int i, maxnpos = 0, ret;
    int origlen;
    char **sp, **ep, *ptr;
    char *progstr = (char *)prog + prog->startoff;
    struct patstralloc patstralloc_struct;

    if (nump) {
	maxnpos = *nump;
	*nump = 0;
    }

    if (!patstralloc)
	patmungestring(&string, &stringlen, &unmetalenin);
    origlen = stringlen;

    if (patstralloc) {
	DPUTS(!patstralloc->alloced,
	      "External unmetafy didn't actually unmetafy.");
	DPUTS(patstralloc->unmetalenp,
	      "Ooh-err: pathpos with external unmetafy. I have bad vibes.");
	patinpath = NULL;
	patinstart = string;
	/* stringlen is unmetafied length; unmetalenin is ignored */
    } else {
	patstralloc = &patstralloc_struct;
	if (patallocstr(prog, string, stringlen, unmetalenin, 0, patstralloc)) {
	    patinstart = patstralloc->alloced + patstralloc->unmetalenp;
	    stringlen = patstralloc->unmetalen;
	} else
	    patinstart = string;
	if (patstralloc->unmetalenp)
	    patinpath = patstralloc->alloced;
	else
	    patinpath = NULL;
    }

    patflags = prog->flags;
    patinend = patinstart + stringlen;
    /*
     * From now on we do not require NULL termination of
     * the test string.  There should also be no more references
     * to the variable string.
     */

    if (prog->flags & (PAT_PURES|PAT_ANY)) {
	/*
	 * Either we are testing against a pure string,
	 * or we can match anything at all.
	 */
	int pstrlen;
	char *pstr;
	if (patstralloc->alloced)
	{
	    /*
	     * Unmetafied; we need pattern string that's also unmetafied.
	     * We'll cache it in the patstralloc structure.
	     * Note it's on the heap.
	     */
	    if (!patstralloc->progstrunmeta)
	    {
		patstralloc->progstrunmeta =
		    dupstrpfx(progstr, (int)prog->patmlen);
		unmetafy(patstralloc->progstrunmeta,
			 &patstralloc->progstrunmetalen);
	    }
	    pstr = patstralloc->progstrunmeta;
	    pstrlen = patstralloc->progstrunmetalen;
	}
	else
	{
	    /* Metafied. */
	    pstr = progstr;
	    pstrlen = (int)prog->patmlen;
	}
	if (prog->flags & PAT_ANY) {
	    /*
	     * Optimisation for a single "*": always matches
	     * (except for no_glob_dots, see below).
	     */
	    ret = 1;
	} else {
	    /*
	     * Testing a pure string.  See if initial
	     * components match.
	     */
	    int lendiff = stringlen - pstrlen;
	    if (lendiff < 0) {
		/* No, the pattern string is too long. */
		ret = 0;
	    } else if (!memcmp(pstr, patinstart, pstrlen)) {
		/*
		 * Initial component matches.  Matches either
		 * if lengths are the same or we are not anchored
		 * to the end of the string.
		 */
		ret = !lendiff || (prog->flags & PAT_NOANCH);
	    } else {
		/* No match. */
		ret = 0;
	    }
	}
	if (ret) {
	    /*
	     * For files, we won't match initial "."s unless
	     * glob_dots is set.
	     */
	    if ((prog->flags & PAT_NOGLD) && *patinstart == '.') {
		ret = 0;
	    } else {
		/*
		 * Remember the length in case used for ${..#..} etc.
		 * In this case, we didn't unmetafy the pattern string
		 * in the original structure, but it might be unmetafied
		 * for use with an unmetafied test string.
		 */
		patinlen = pstrlen;
		/* if matching files, must update globbing flags */
		patglobflags = prog->globend;

		if ((patglobflags & GF_MATCHREF) &&
		    !(patflags & PAT_FILE)) {
		    char *str;
		    int mlen;

		    if (patstralloc->alloced) {
			/*
			 * Unmetafied: pstrlen contains unmetafied
			 * length in bytes.
			 */
			str = metafy(patinstart, pstrlen, META_DUP);
			mlen = CHARSUB(patinstart, patinstart + pstrlen);
		    } else {
			str = ztrduppfx(patinstart, patinlen);
			/*
			 * Count the characters.  We're not using CHARSUB()
			 * because the string is still metafied.
			 */
			MB_METACHARINIT();
			mlen = MB_METASTRLEN2END(patinstart, 0,
						 patinstart + patinlen);
		    }

		    setsparam("MATCH", str);
		    setiparam("MBEGIN",
			      (zlong)(patoffset + !isset(KSHARRAYS)));
		    setiparam("MEND",
			      (zlong)(mlen + patoffset +
				      !isset(KSHARRAYS) - 1));
		}
	    }
	}
    } else {
	/*
	 * Test for a `must match' string, unless we're scanning for a match
	 * in which case we don't need to do this each time.
	 */
	ret = 1;
	if (!(prog->flags & PAT_SCAN) && prog->mustoff)
	{
	    char *testptr;	/* start pointer into test string */
	    char *teststop;	/* last point from which we can match */
	    char *patptr = (char *)prog + prog->mustoff;
	    int patlen = prog->patmlen;
	    int found = 0;

	    if (patlen > stringlen) {
		/* Too long, can't match. */
		ret = 0;
	    } else {
		teststop = patinend - patlen;

		for (testptr = patinstart; testptr <= teststop; testptr++)
		{
		    if (!memcmp(testptr, patptr, patlen)) {
			found = 1;
			break;
		    }
		}

		if (!found)
		    ret = 0;
	    }
	}
	if (!ret)
	    return 0;

	patglobflags = prog->globflags;
	if (!(patflags & PAT_FILE)) {
	    forceerrs = -1;
	    errsfound = 0;
	}
	globdots = !(patflags & PAT_NOGLD);
	parsfound = 0;

	patinput = patinstart;

	exactpos = exactend = NULL;
	/* The only external call to patmatch --- all others are recursive */
	if (patmatch((Upat)progstr)) {
	    /*
	     * we were lazy and didn't save the globflags if an exclusion
	     * failed, so set it now
	     */
	    patglobflags = prog->globend;

	    /*
	     * Record length of successful match, including Meta
	     * characters.  Do it here so that patmatchlen() can return
	     * it even if we delete the pattern strings.
	     */
	    patinlen = patinput - patinstart;
	    /*
	     * Optimization: if we didn't find any Meta characters
	     * to begin with, we don't need to look for them now.
	     *
	     * For patstralloc passed in, we want the unmetafied length.
	     */
	    if (patstralloc == &patstralloc_struct &&
		patstralloc->unmetalen != origlen) {
		for (ptr = patinstart; ptr < patinput; ptr++)
		    if (imeta(*ptr))
			patinlen++;
	    }

	    /*
	     * Should we clear backreferences and matches on a failed
	     * match?
	     */
	    if ((patglobflags & GF_MATCHREF) && !(patflags & PAT_FILE)) {
		/*
		 * m flag: for global match.  This carries no overhead
		 * in the pattern matching part.
		 *
		 * Remember the test pattern is already unmetafied.
		 */
		char *str;
		int mlen = CHARSUB(patinstart, patinput);

		str = metafy(patinstart, patinput - patinstart, META_DUP);
		setsparam("MATCH", str);
		setiparam("MBEGIN", (zlong)(patoffset + !isset(KSHARRAYS)));
		setiparam("MEND",
			  (zlong)(mlen + patoffset +
				  !isset(KSHARRAYS) - 1));
	    }
	    if (prog->patnpar && nump) {
		/*
		 * b flag: for backreferences using parentheses. Reported
		 * directly.
		 */
		*nump = prog->patnpar;

		sp = patbeginp;
		ep = patendp;

		for (i = 0; i < prog->patnpar && i < maxnpos; i++) {
		    if (parsfound & (1 << i)) {
			if (begp)
			    *begp++ = CHARSUB(patinstart, *sp) + patoffset;
			if (endp)
			    *endp++ = CHARSUB(patinstart, *ep) + patoffset
				- 1;
		    } else {
			if (begp)
			    *begp++ = -1;
			if (endp)
			    *endp++ = -1;
		    }

		    sp++;
		    ep++;
		}
	    } else if (prog->patnpar && !(patflags & PAT_FILE)) {
		/*
		 * b flag: for backreferences using parentheses.
		 */
		int palen = prog->patnpar+1;
		char **matcharr, **mbeginarr, **mendarr;
		char numbuf[DIGBUFSIZE];

		matcharr = zshcalloc(palen*sizeof(char *));
		mbeginarr = zshcalloc(palen*sizeof(char *));
		mendarr = zshcalloc(palen*sizeof(char *));

		sp = patbeginp;
		ep = patendp;

		for (i = 0; i < prog->patnpar; i++) {
		    if (parsfound & (1 << i)) {
			matcharr[i] = metafy(*sp, *ep - *sp, META_DUP);
			/*
			 * mbegin and mend give indexes into the string
			 * in the standard notation, i.e. respecting
			 * KSHARRAYS, and with the end index giving
			 * the last character, not one beyond.
			 * For example, foo=foo; [[ $foo = (f)oo ]] gives
			 * (without KSHARRAYS) indexes 1 and 1, which
			 * corresponds to indexing as ${foo[1,1]}.
			 */
			sprintf(numbuf, "%ld",
				(long)(CHARSUB(patinstart, *sp) +
				       patoffset +
				       !isset(KSHARRAYS)));
			mbeginarr[i] = ztrdup(numbuf);
			sprintf(numbuf, "%ld",
				(long)(CHARSUB(patinstart, *ep) +
				       patoffset +
				       !isset(KSHARRAYS) - 1));
			mendarr[i] = ztrdup(numbuf);
		    } else {
			/* Pattern wasn't set: either it was in an
			 * unmatched branch, or a hashed parenthesis
			 * that didn't match at all.
			 */
			matcharr[i] = ztrdup("");
			mbeginarr[i] = ztrdup("-1");
			mendarr[i] = ztrdup("-1");
		    }
		    sp++;
		    ep++;
		}
		setaparam("match", matcharr);
		setaparam("mbegin", mbeginarr);
		setaparam("mend", mendarr);
	    }

	    if (!nump && endp) {
		/*
		 * We just need the overall end position.
		 */
		*endp = CHARSUB(patinstart, patinput) + patoffset;
	    }

	    ret = 1;
	} else
	    ret = 0;
    }

    return ret;
}

/*
 * Return length of previous successful match.  This is
 * in metafied bytes, i.e. includes a count of Meta characters,
 * unless the match was done on an unmetafied string using
 * a patstralloc struct, in which case it too is unmetafied.
 * Unusual and futile attempt at modular encapsulation.
 */

/**/
int
patmatchlen(void)
{
    return patinlen;
}

/*
 * Match literal characters with case insensitivity test:  the first
 * comes from the input string, the second the current pattern.
 */
#ifdef MULTIBYTE_SUPPORT
#define ISUPPER(x)	iswupper(x)
#define ISLOWER(x)	iswlower(x)
#define TOUPPER(x)	towupper(x)
#define TOLOWER(x)	towlower(x)
#define ISDIGIT(x)	iswdigit(x)
#else
#define ISUPPER(x)	isupper(x)
#define ISLOWER(x)	islower(x)
#define TOUPPER(x)	toupper(x)
#define TOLOWER(x)	tolower(x)
#define ISDIGIT(x)	idigit(x)
#endif
#define CHARMATCH(chin, chpa) (chin == chpa || \
        ((patglobflags & GF_IGNCASE) ? \
	 ((ISUPPER(chin) ? TOLOWER(chin) : chin) == \
	  (ISUPPER(chpa) ? TOLOWER(chpa) : chpa)) : \
	 (patglobflags & GF_LCMATCHUC) ? \
	 (ISLOWER(chpa) && TOUPPER(chpa) == chin) : 0))

/*
 * The same but caching an expression from the first argument,
 * Requires local charmatch_cache definition.
 */
#define CHARMATCH_EXPR(expr, chpa) \
	(charmatch_cache = (expr), CHARMATCH(charmatch_cache, chpa))

/*
 * Main matching routine.
 *
 * Testing the tail end of a match is usually done by recursion, but
 * we try to eliminate that in favour of looping for simple cases.
 */

/**/
static int
patmatch(Upat prog)
{
    /* Current and next nodes */
    Upat scan = prog, next, opnd;
    char *start, *save, *chrop, *chrend, *compend;
    int savglobflags, op, no, min, fail = 0, saverrsfound;
    zrange_t from, to, comp;
    patint_t nextch;
    int q = queue_signal_level();

    /*
     * To avoid overhead of saving state if there are no queued signals
     * waiting, we pierce the signals.h veil and examine queue state.
     */
#define check_for_signals() do if (queue_front != queue_rear) { \
	    int savpatflags = patflags, savpatglobflags = patglobflags; \
            char *savexactpos = exactpos, *savexactend = exactend; \
	    struct rpat savpattrystate = pattrystate; \
	    dont_queue_signals(); \
	    restore_queue_signals(q); \
	    exactpos = savexactpos; \
	    exactend = savexactend; \
	    patflags = savpatflags; \
	    patglobflags = savpatglobflags; \
	    pattrystate = savpattrystate; \
	} while (0)

    check_for_signals();

    while  (scan && !errflag) {
	next = PATNEXT(scan);

	if (!globdots && P_NOTDOT(scan) && patinput == patinstart &&
	    patinput < patinend && *patinput == '.')
	    return 0;

	switch (P_OP(scan)) {
	case P_ANY:
	    if (patinput == patinend)
		fail = 1;
	    else
		CHARINC(patinput, patinend);
	    break;
	case P_EXACTLY:
	    /*
	     * acts as nothing if *chrop is null:  this is used by
	     * approx code.
	     */
	    if (exactpos) {
		chrop = exactpos;
		chrend = exactend;
	    } else {
		chrop = P_LS_STR(scan);
		chrend = chrop + P_LS_LEN(scan);
	    }
	    exactpos = NULL;
	    while (chrop < chrend && patinput < patinend) {
		char *savpatinput = patinput;
		char *savchrop = chrop;
		int badin = 0, badpa = 0;
		/*
		 * Care with character matching:
		 * We do need to convert the character to wide
		 * representation if possible, because we may need
		 * to do case transformation.  However, we should
		 * be careful in case one, but not the other, wasn't
		 * representable in the current locale---in that
		 * case they don't match even if the returned
		 * values (one properly converted, one raw) are
		 * the same.
		 */
		patint_t chin = CHARREFINC(patinput, patinend, &badin);
		patint_t chpa = CHARREFINC(chrop, chrend, &badpa);
		if (!CHARMATCH(chin, chpa) || badin != badpa) {
		    fail = 1;
		    patinput = savpatinput;
		    chrop = savchrop;
		    break;
		}
	    }
	    if (chrop < chrend) {
		exactpos = chrop;
		exactend = chrend;
		fail = 1;
	    }
	    break;
	case P_ANYOF:
	case P_ANYBUT:
	    if (patinput == patinend)
		fail = 1;
	    else {
#ifdef MULTIBYTE_SUPPORT
		int zmb_ind;
		wchar_t cr = charref(patinput, patinend, &zmb_ind);
		char *scanop = (char *)P_OPERAND(scan);
		if (patglobflags & GF_MULTIBYTE) {
		    if (mb_patmatchrange(scanop, cr, zmb_ind, NULL, NULL) ^
			(P_OP(scan) == P_ANYOF))
			fail = 1;
		    else
			CHARINC(patinput, patinend);
		} else if (patmatchrange(scanop, (int)cr, NULL, NULL) ^
			   (P_OP(scan) == P_ANYOF))
		    fail = 1;
		else
		    CHARINC(patinput, patinend);
#else
		if (patmatchrange((char *)P_OPERAND(scan),
				  CHARREF(patinput, patinend), NULL, NULL) ^
		    (P_OP(scan) == P_ANYOF))
		    fail = 1;
		else
		    CHARINC(patinput, patinend);
#endif
	    }
	    break;
	case P_NUMRNG:
	case P_NUMFROM:
	case P_NUMTO:
	    /*
	     * To do this properly, we really have to treat numbers as
	     * closures:  that's so things like <1-1000>33 will
	     * match 633 (they didn't up to 3.1.6).  To avoid making this
	     * too inefficient, we see if there's an exact match next:
	     * if there is, and it's not a digit, we return 1 after
	     * the first attempt.
	     */
	    op = P_OP(scan);
	    start = (char *)P_OPERAND(scan);
	    from = to = 0;
	    if (op != P_NUMTO) {
#ifdef ZSH_64_BIT_TYPE
		/* We can't rely on pointer alignment being good enough. */
		memcpy((char *)&from, start, sizeof(zrange_t));
#else
		from = *((zrange_t *) start);
#endif
		start += sizeof(zrange_t);
	    }
	    if (op != P_NUMFROM) {
#ifdef ZSH_64_BIT_TYPE
		memcpy((char *)&to, start, sizeof(zrange_t));
#else
		to = *((zrange_t *) start);
#endif
	    }
	    start = compend = patinput;
	    comp = 0;
	    while (patinput < patinend && idigit(*patinput)) {
		int out_of_range = 0;
		int digit = *patinput - '0';
		if (comp > ZRANGE_MAX / (zlong)10) {
		    out_of_range = 1;
		} else {
		    zrange_t c10 = comp ? comp * 10 : 0;
		    if (ZRANGE_MAX - c10 < digit) {
			out_of_range = 1;
		    } else {
			comp = c10;
			comp += digit;
		    }
		}
		patinput++;
		compend++;

		if (out_of_range ||
		    (comp & ((zrange_t)1 << (sizeof(comp)*8 -
#ifdef ZRANGE_T_IS_SIGNED
					    2
#else
					    1
#endif
				)))) {
		    /*
		     * Out of range (allowing for signedness, which
		     * we need if we are using zlongs).
		     * This is as far as we can go.
		     * If we're doing a range "from", skip all the
		     * remaining numbers.  Otherwise, we can't
		     * match beyond the previous point anyway.
		     * Leave the pointer to the last calculated
		     * position (compend) where it was before.
		     */
		    if (op == P_NUMFROM) {
			while (patinput < patinend && idigit(*patinput))
			    patinput++;
		    }
		}
	    }
	    save = patinput;
	    no = 0;
	    while (patinput > start) {
		/* if already too small, no power on earth can save it */
		if (comp < from && patinput <= compend)
		    break;
		if ((op == P_NUMFROM || comp <= to) && patmatch(next)) {
		    return 1;
		}
		if (!no && P_OP(next) == P_EXACTLY &&
		    (!P_LS_LEN(next) ||
		     !idigit(STOUC(*P_LS_STR(next)))) &&
		    !(patglobflags & 0xff))
		    return 0;
		patinput = --save;
		no++;
		/*
		 * With a range start and an unrepresentable test
		 * number, we just back down the test string without
		 * changing the number until we get to a representable
		 * one.
		 */
		if (patinput < compend)
		    comp /= 10;
	    }
	    patinput = start;
	    fail = 1;
	    break;
	case P_NUMANY:
	    /* This is <->: any old set of digits, don't bother comparing */
	    start = patinput;
	    while (patinput < patinend && idigit(*patinput))
		patinput++;
	    save = patinput;
	    no = 0;
	    while (patinput > start) {
		if (patmatch(next))
		    return 1;
		if (!no && P_OP(next) == P_EXACTLY &&
		    (!P_LS_LEN(next) ||
		     !idigit(*P_LS_STR(next))) &&
		    !(patglobflags & 0xff))
		    return 0;
		patinput = --save;
		no++;
	    }
	    patinput = start;
	    fail = 1;
	    break;
	case P_NOTHING:
	    break;
	case P_BACK:
	    break;
	case P_GFLAGS:
	    patglobflags = P_OPERAND(scan)->l;
	    break;
	case P_OPEN:
	case P_OPEN+1:
	case P_OPEN+2:
	case P_OPEN+3:
	case P_OPEN+4:
	case P_OPEN+5:
	case P_OPEN+6:
	case P_OPEN+7:
	case P_OPEN+8:
	case P_OPEN+9:
	    no = P_OP(scan) - P_OPEN;
	    save = patinput;

	    if (patmatch(next)) {
		/*
		 * Don't set patbeginp if some later invocation of
		 * the same parentheses already has.
		 */
		if (no && !(parsfound & (1 << (no - 1)))) {
		    patbeginp[no-1] = save;
		    parsfound |= 1 << (no - 1);
		}
		return 1;
	    } else
		return 0;
	    break;
	case P_CLOSE:
	case P_CLOSE+1:
	case P_CLOSE+2:
	case P_CLOSE+3:
	case P_CLOSE+4:
	case P_CLOSE+5:
	case P_CLOSE+6:
	case P_CLOSE+7:
	case P_CLOSE+8:
	case P_CLOSE+9:
	    no = P_OP(scan) - P_CLOSE;
	    save = patinput;

	    if (patmatch(next)) {
		if (no && !(parsfound & (1 << (no + 15)))) {
		    patendp[no-1] = save;
		    parsfound |= 1 << (no + 15);
		}
		return 1;
	    } else
		return 0;
	    break;
	case P_EXCSYNC:
	    /* See the P_EXCLUDE code below for where syncptr comes from */
	    {
		unsigned char *syncptr;
		Upat after;
		after = P_OPERAND(scan);
		DPUTS(!P_ISEXCLUDE(after),
		      "BUG: EXCSYNC not followed by EXCLUDE.");
		DPUTS(!P_OPERAND(after)->p,
		      "BUG: EXCSYNC not handled by EXCLUDE");
		syncptr = P_OPERAND(after)->p + (patinput - patinstart);
		/*
		 * If we already matched from here, this time we fail.
		 * See WBRANCH code for story about error count.
		 */
		if (*syncptr && errsfound + 1 >= *syncptr)
		    return 0;
		/*
		 * Else record that we (possibly) matched this time.
		 * No harm if we don't:  then the previous test will just
		 * short cut the attempted match that is bound to fail.
		 * We never try to exclude something that has already
		 * failed anyway.
		 */
		*syncptr = errsfound + 1;
	    }
	    break;
	case P_EXCEND:
	    /*
	     * This is followed by a P_EXCSYNC, but only in the P_EXCLUDE
	     * branch.  Actually, we don't bother following it:  all we
	     * need to know is that we successfully matched so far up
	     * to the end of the asserted pattern; the endpoint
	     * in the target string is nulled out.
	     */
	    if (!(fail = (patinput < patinend)))
		return 1;
	    break;
	case P_BRANCH:
	case P_WBRANCH:
	    /* P_EXCLUDE shouldn't occur without a P_BRANCH */
	    if (!P_ISBRANCH(next)) {
		/* no choice, avoid recursion */
		DPUTS(P_OP(scan) == P_WBRANCH,
		      "BUG: WBRANCH with no alternative.");
		next = P_OPERAND(scan);
	    } else {
		do {
		    save = patinput;
		    savglobflags = patglobflags;
		    saverrsfound = errsfound;
		    if (P_ISEXCLUDE(next)) {
			/*
			 * The strategy is to test the asserted pattern,
			 * recording via P_EXCSYNC how far the part to
			 * be excluded matched.  We then set the
			 * length of the test string to that
			 * point and see if the exclusion as far as
			 * P_EXCEND also matches that string.
			 * We need to keep testing the asserted pattern
			 * by backtracking, since the first attempt
			 * may be excluded while a later attempt may not.
			 * For this we keep a pointer just after
			 * the P_EXCLUDE which is tested by the P_EXCSYNC
			 * to see if we matched there last time, in which
			 * case we fail.  If there is nothing to backtrack
			 * over, that doesn't matter:  we should fail anyway.
			 * The pointer also tells us where the asserted
			 * pattern matched for use by the exclusion.
			 *
			 * It's hard to allocate space for this
			 * beforehand since we may need to do it
			 * recursively.
			 *
			 * P.S. in case you were wondering, this code
			 * is horrible.
			 */
			Upat syncstrp;
			char *origpatinend;
			unsigned char *oldsyncstr;
			char *matchpt = NULL;
			int ret, savglobdots, matchederrs = 0;
			int savparsfound = parsfound;
			DPUTS(P_OP(scan) == P_WBRANCH,
			      "BUG: excluded WBRANCH");
			syncstrp = P_OPERAND(next);
			/*
			 * Unlike WBRANCH, each test at the same exclude
			 * sync point (due to an external loop) is separate,
			 * i.e testing (foo~bar)# is no different from
			 * (foo~bar)(foo~bar)... from the exclusion point
			 * of view, so we use a different sync string.
			 */
			oldsyncstr = syncstrp->p;
			syncstrp->p = (unsigned char *)
			    zshcalloc((patinend - patinstart) + 1);
			origpatinend = patinend;
			while ((ret = patmatch(P_OPERAND(scan)))) {
			    unsigned char *syncpt;
			    char *savpatinstart;
			    int savforce = forceerrs;
			    int savpatflags = patflags, synclen;
			    forceerrs = -1;
			    savglobdots = globdots;
			    matchederrs = errsfound;
			    matchpt = patinput;    /* may not be end */
			    globdots = 1;	   /* OK to match . first */
			    /* Find the point where the scan
			     * matched the part to be excluded: because
			     * of backtracking, the one
			     * most recently matched will be the first.
			     * (Luckily, backtracking is done after all
			     * possibilities for approximation have been
			     * checked.)
			     */
			    for (syncpt = syncstrp->p; !*syncpt; syncpt++)
				;
			    synclen = syncpt - syncstrp->p;
			    if (patinstart + synclen != patinend) {
				/*
				 * Temporarily mark the string as
				 * ending at this point.
				 */
				DPUTS(patinstart + synclen > matchpt,
				      "BUG: EXCSYNC failed");

				patinend = patinstart + synclen;
				/*
				 * If this isn't really the end of the string,
				 * remember this for the (#e) assertion.
				 */
				patflags |= PAT_NOTEND;
			    }
			    savpatinstart = patinstart;
			    next = PATNEXT(scan);
			    while (next && P_ISEXCLUDE(next)) {
				patinput = save;
				/*
				 * turn off approximations in exclusions:
				 * note we keep remaining patglobflags
				 * set by asserted branch (or previous
				 * excluded branches, for consistency).
				 */
				patglobflags &= ~0xff;
				errsfound = 0;
				opnd = P_OPERAND(next) + 1;
				if (P_OP(next) == P_EXCLUDP && patinpath) {
				    /*
				     * Top level exclusion with a file,
				     * applies to whole path so add the
				     * segments already matched.
				     * We copied these in front of the
				     * test pattern, so patinend doesn't
				     * need moving.
				     */
				    DPUTS(patinput != patinstart,
					  "BUG: not at start excluding path");
				    patinput = patinstart = patinpath;
				}
				if (patmatch(opnd)) {
				    ret = 0;
				    /*
				     * Another subtlety: if we exclude the
				     * match, any parentheses just found
				     * become invalidated.
				     */
				    parsfound = savparsfound;
				}
				if (patinpath) {
				    patinput = savpatinstart +
					(patinput - patinstart);
				    patinstart = savpatinstart;
				}
				if (!ret)
				    break;
				next = PATNEXT(next);
			    }
			    /*
			     * Restore original end position.
			     */
			    patinend = origpatinend;
			    patflags = savpatflags;
			    globdots = savglobdots;
			    forceerrs = savforce;
			    if (ret)
				break;
			    patinput = save;
			    patglobflags = savglobflags;
			    errsfound = saverrsfound;
			}
			zfree((char *)syncstrp->p,
			      (patinend - patinstart) + 1);
			syncstrp->p = oldsyncstr;
			if (ret) {
			    patinput = matchpt;
			    errsfound = matchederrs;
			    return 1;
			}
			while ((scan = PATNEXT(scan)) &&
			       P_ISEXCLUDE(scan))
			    ;
		    } else {
			int ret = 1, pfree = 0;
			Upat ptrp = NULL;
			unsigned char *ptr;
			if (P_OP(scan) == P_WBRANCH) {
			    /*
			     * This is where we make sure that we are not
			     * repeatedly matching zero-length strings in
			     * a closure, which would cause an infinite loop,
			     * and also remove exponential behaviour in
			     * backtracking nested closures.
			     * The P_WBRANCH operator leaves a space for a
			     * uchar *, initialized to NULL, which is
			     * turned into a string the same length as the
			     * target string.  Every time we match from a
			     * particular point in the target string, we
			     * stick a 1 at the corresponding point here.
			     * If we come round to the same branch again, and
			     * there is already a 1, then the test fails.
			     */
			    opnd = P_OPERAND(scan);
			    ptrp = opnd++;
			    if (!ptrp->p) {
				ptrp->p = (unsigned char *)
				    zshcalloc((patinend - patinstart) + 1);
				pfree = 1;
			    }
			    ptr = ptrp->p + (patinput - patinstart);

			    /*
			     * Without approximation, this is just a
			     * single bit test.  With approximation, we
			     * need to know how many errors there were
			     * last time we made the test.  If errsfound
			     * is now smaller than it was, hence we can
			     * make more approximations in the remaining
			     * code, we continue with the test.
			     * (This is why the max number of errors is
			     * 254, not 255.)
			     */
			    if (*ptr && errsfound + 1 >= *ptr)
				ret = 0;
			    *ptr = errsfound + 1;
			} else
			    opnd = P_OPERAND(scan);
			if (ret)
			    ret = patmatch(opnd);
			if (pfree) {
			    zfree((char *)ptrp->p,
				  (patinend - patinstart) + 1);
			    ptrp->p = NULL;
			}
			if (ret)
			    return 1;
			scan = PATNEXT(scan);
		    }
		    patinput = save;
		    patglobflags = savglobflags;
		    errsfound = saverrsfound;
		    DPUTS(P_OP(scan) == P_WBRANCH,
			  "BUG: WBRANCH not first choice.");
		    next = PATNEXT(scan);
		} while (scan && P_ISBRANCH(scan));
		return 0;
	    }
	    break;
	case P_STAR:
	    /* Handle specially for speed, although really P_ONEHASH+P_ANY */
	    while (P_OP(next) == P_STAR) {
		/*
		 * If there's another * following we can optimise it
		 * out.  Chains of *'s can give pathologically bad
		 * performance.
		 */
		scan = next;
		next = PATNEXT(scan);
	    }
	    /*FALLTHROUGH*/
	case P_ONEHASH:
	case P_TWOHASH:
	    /*
	     * This is just simple cases, matching one character.
	     * With approximations, we still handle * this way, since
	     * no approximation is ever necessary, but other closures
	     * are handled by the more complicated branching method
	     */
	    op = P_OP(scan);
	    /* Note that no counts possibly metafied characters */
	    start = patinput;
	    {
		char *lastcharstart;
		/*
		 * Array to record the start of characters for
		 * backtracking.
		 */
		VARARR(char, charstart, patinend-patinput);
		memset(charstart, 0, patinend-patinput);

		if (op == P_STAR) {
		    for (no = 0; patinput < patinend;
			 CHARINC(patinput, patinend))
		    {
			charstart[patinput-start] = 1;
			no++;
		    }
		    /* simple optimization for reasonably common case */
		    if (P_OP(next) == P_END)
			return 1;
		} else {
		    DPUTS(patglobflags & 0xff,
			  "BUG: wrong backtracking with approximation.");
		    if (!globdots && P_NOTDOT(P_OPERAND(scan)) &&
			patinput == patinstart && patinput < patinend &&
			CHARREF(patinput, patinend) == ZWC('.'))
			return 0;
		    no = patrepeat(P_OPERAND(scan), charstart);
		}
		min = (op == P_TWOHASH) ? 1 : 0;
		/*
		 * Lookahead to avoid useless matches. This is not possible
		 * with approximation.
		 */
		if (P_OP(next) == P_EXACTLY && P_LS_LEN(next) &&
		    !(patglobflags & 0xff)) {
		    char *nextop = P_LS_STR(next);
#ifdef MULTIBYTE_SUPPORT
		    /* else second argument of CHARREF isn't used */
		    int nextlen = P_LS_LEN(next);
#endif
		    /*
		     * If that P_EXACTLY is last (common in simple patterns,
		     * such as *.c), then it can be only be matched at one
		     * point in the test string, so record that.
		     */
		    if (P_OP(PATNEXT(next)) == P_END &&
			!(patflags & PAT_NOANCH)) {
			int ptlen = patinend - patinput;
			int lenmatch = patinend -
			    (min ? CHARNEXT(start, patinend) : start);
			/* Are we in the right range? */
			if (P_LS_LEN(next) > lenmatch ||
			    P_LS_LEN(next) < ptlen)
			    return 0;
			/* Yes, just position appropriately and test. */
			patinput += ptlen - P_LS_LEN(next);
			/*
			 * Here we will need to be careful that patinput is not
			 * in the middle of a multibyte character.
			 */
			/* Continue loop with P_EXACTLY test. */
			break;
		    }
		    nextch = CHARREF(nextop, nextop + nextlen);
		} else
		    nextch = PEOF;
		savglobflags = patglobflags;
		saverrsfound = errsfound;
		lastcharstart = charstart + (patinput - start);
		if (no >= min) {
		    for (;;) {
			patint_t charmatch_cache;
			if (nextch == PEOF ||
			    (patinput < patinend &&
			     CHARMATCH_EXPR(CHARREF(patinput, patinend),
					    nextch))) {
			    if (patmatch(next))
				return 1;
			}
			if (--no < min)
			    break;
			/* find start of previous full character */
			while (!*--lastcharstart)
			    DPUTS(lastcharstart < charstart,
				  "lastcharstart invalid");
			patinput = start + (lastcharstart-charstart);
			patglobflags = savglobflags;
			errsfound = saverrsfound;
		    }
		}
	    }
	    /*
	     * As with branches, the patmatch(next) stuff for *
	     * handles approximation, so we don't need to try
	     * anything here.
	     */
	    return 0;
	case P_ISSTART:
	    if (patinput != patinstart || (patflags & PAT_NOTSTART))
		fail = 1;
	    break;
	case P_ISEND:
	    if (patinput < patinend || (patflags & PAT_NOTEND))
		fail = 1;
	    break;
	case P_COUNTSTART:
	    {
		/*
		 * Save and restore the current count and the
		 * start pointer in case the pattern has been
		 * executed by a previous repetition of a
		 * closure.
		 */
		long *curptr = &P_OPERAND(scan)[P_CT_CURRENT].l;
		long savecount = *curptr;
		unsigned char *saveptr = scan[P_CT_PTR].p;
		int ret;

		*curptr = 0L;
		ret = patmatch(P_OPERAND(scan));
		*curptr = savecount;
		scan[P_CT_PTR].p = saveptr;
		return ret;
	    }
	case P_COUNT:
	    {
		/* (#cN,M): execution is relatively straightforward */
		long cur = scan[P_CT_CURRENT].l;
		long min = scan[P_CT_MIN].l;
		long max = scan[P_CT_MAX].l;

		if (cur && cur >= min &&
		    (unsigned char *)patinput == scan[P_CT_PTR].p) {
		    /*
		     * Not at the first attempt to match so
		     * the previous attempt managed zero length.
		     * We can do this indefinitely so there's
		     * no point in going on.  Simply try to
		     * match the remainder of the pattern.
		     */
		    return patmatch(next);
		}
		scan[P_CT_PTR].p = (unsigned char *)patinput;

		if (max < 0 || cur < max) {
		    char *patinput_thistime = patinput;
		    scan[P_CT_CURRENT].l = cur + 1;
		    if (patmatch(scan + P_CT_OPERAND))
			return 1;
		    scan[P_CT_CURRENT].l = cur;
		    patinput = patinput_thistime;
		}
		if (cur < min)
		    return 0;
		return patmatch(next);
	    }
	case P_END:
	    if (!(fail = (patinput < patinend && !(patflags & PAT_NOANCH))))
		return 1;
	    break;
#ifdef DEBUG
	default:
	    dputs("BUG: bad operand in patmatch.");
	    return 0;
	    break;
#endif
	}

	if (fail) {
	    if (errsfound < (patglobflags & 0xff) &&
		(forceerrs == -1 || errsfound < forceerrs)) {
		/*
		 * Approximation code.  There are four possibilities
		 *
		 * 1. omit character from input string
		 * 2. transpose characters in input and pattern strings
		 * 3. omit character in both input and pattern strings
		 * 4. omit character from pattern string.
		 *
		 * which we try in that order.
		 *
		 * Of these, 2, 3 and 4 require an exact match string
		 * (P_EXACTLY) while 1, 2 and 3 require that we not
		 * have reached the end of the input string.
		 *
		 * Note in each case after making the approximation we
		 * need to retry the *same* pattern; this is what
		 * requires exactpos, a slightly doleful way of
		 * communicating with the exact character matcher.
		 */
		char *savexact = exactpos;
		save = patinput;
		savglobflags = patglobflags;
		saverrsfound = ++errsfound;
		fail = 0;

		DPUTS(P_OP(scan) != P_EXACTLY && exactpos,
		      "BUG: non-exact match has set exactpos");

		/* Try omitting a character from the input string */
		if (patinput < patinend) {
		    CHARINC(patinput, patinend);
		    /* If we are not on an exact match, then this is
		     * our last gasp effort, so we can optimize out
		     * the recursive call.
		     */
		    if (P_OP(scan) != P_EXACTLY)
			continue;
		    if (patmatch(scan))
			return 1;
		}

		if (P_OP(scan) == P_EXACTLY) {
		    char *nextexact = savexact;
		    DPUTS(!savexact,
			  "BUG: exact match has not set exactpos");
		    CHARINC(nextexact, exactend);

		    if (save < patinend) {
			char *nextin = save;
			CHARINC(nextin, patinend);
			patglobflags = savglobflags;
			errsfound = saverrsfound;
			exactpos = savexact;

			/*
			 * Try swapping two characters in patinput and
			 * exactpos
			 */
			if (save < patinend && nextin < patinend &&
			    nextexact < exactend) {
			    patint_t cin0 = CHARREF(save, patinend);
			    patint_t cpa0 = CHARREF(exactpos, exactend);
			    patint_t cin1 = CHARREF(nextin, patinend);
			    patint_t cpa1 = CHARREF(nextexact, exactend);

			    if (CHARMATCH(cin0, cpa1) &&
				CHARMATCH(cin1, cpa0)) {
				patinput = nextin;
				CHARINC(patinput, patinend);
				exactpos = nextexact;
				CHARINC(exactpos, exactend);
				if (patmatch(scan))
				    return 1;

				patglobflags = savglobflags;
				errsfound = saverrsfound;
			    }
			}

			/*
			 * Try moving up both strings.
			 */
			patinput = nextin;
			exactpos = nextexact;
			if (patmatch(scan))
			    return 1;

			patinput = save;
			patglobflags = savglobflags;
			errsfound = saverrsfound;
			exactpos = savexact;
		    }

		    DPUTS(exactpos == exactend, "approximating too far");
		    /*
		     * Try moving up the exact match pattern.
		     * This must be the last attempt, so just loop
		     * instead of calling recursively.
		     */
		    CHARINC(exactpos, exactend);
		    continue;
		}
	    }
	    exactpos = NULL;
	    return 0;
	}

	scan = next;

	/* Allow handlers to run once per loop */
	check_for_signals();
    }

    return 0;
}


/**/
#ifdef MULTIBYTE_SUPPORT

/*
 * See if character ch matches a pattern range specification.
 * The null-terminated specification is in range; the test
 * character is in ch.
 *
 * zmb is one of the enum defined above charref(), for indicating
 * incomplete or invalid multibyte characters.
 *
 * indptr is used by completion matching, which is why this
 * function is exported.  If indptr is not NULL we set *indptr
 * to the index of the character in the range string, adjusted
 * in the case of "A-B" ranges such that A would count as its
 * normal index (say IA), B would count as IA + (B-A), and any
 * character within the range as appropriate.  We're not strictly
 * guaranteed this fits within a wint_t, but if this is Unicode
 * in 32 bits we have a fair amount of distance left over.
 *
 * mtp is used in the same circumstances.  *mtp returns the match type:
 * 0 for a standard character, else the PP_ index.  It's not
 * useful if the match failed.
 */

/**/
mod_export int
mb_patmatchrange(char *range, wchar_t ch, int zmb_ind, wint_t *indptr, int *mtp)
{
    wchar_t r1, r2;

    if (indptr)
	*indptr = 0;
    /*
     * Careful here: unlike other strings, range is a NULL-terminated,
     * metafied string, because we need to treat the Posix and hyphenated
     * ranges specially.
     */
    while (*range) {
	if (imeta(STOUC(*range))) {
	    int swtype = STOUC(*range++) - STOUC(Meta);
	    if (mtp)
		*mtp = swtype;
	    switch (swtype) {
	    case 0:
		/* ordinary metafied character */
		range--;
		if (metacharinc(&range) == ch)
		    return 1;
		break;
	    case PP_ALPHA:
		if (iswalpha(ch))
		    return 1;
		break;
	    case PP_ALNUM:
		if (iswalnum(ch))
		    return 1;
		break;
	    case PP_ASCII:
		if ((ch & ~0x7f) == 0)
		    return 1;
		break;
	    case PP_BLANK:
#if !defined(HAVE_ISWBLANK) && !defined(iswblank)
/*
 * iswblank() is GNU and C99. There's a remote chance that some
 * systems still don't support it (but would support the other ones
 * if MULTIBYTE_SUPPORT is enabled).
 */
#define iswblank(c) (c == L' ' || c == L'\t')
#endif
		if (iswblank(ch))
		    return 1;
		break;
	    case PP_CNTRL:
		if (iswcntrl(ch))
		    return 1;
		break;
	    case PP_DIGIT:
		if (iswdigit(ch))
		    return 1;
		break;
	    case PP_GRAPH:
		if (iswgraph(ch))
		    return 1;
		break;
	    case PP_LOWER:
		if (iswlower(ch))
		    return 1;
		break;
	    case PP_PRINT:
		if (WC_ISPRINT(ch))
		    return 1;
		break;
	    case PP_PUNCT:
		if (iswpunct(ch))
		    return 1;
		break;
	    case PP_SPACE:
		if (iswspace(ch))
		    return 1;
		break;
	    case PP_UPPER:
		if (iswupper(ch))
		    return 1;
		break;
	    case PP_XDIGIT:
		if (iswxdigit(ch))
		    return 1;
		break;
	    case PP_IDENT:
		if (wcsitype(ch, IIDENT))
		    return 1;
		break;
	    case PP_IFS:
		if (wcsitype(ch, ISEP))
		    return 1;
		break;
	    case PP_IFSSPACE:
		/* must be ASCII space character */
		if (ch < 128 && iwsep((int)ch))
		    return 1;
		break;
	    case PP_WORD:
		if (wcsitype(ch, IWORD))
		    return 1;
		break;
	    case PP_RANGE:
		r1 = metacharinc(&range);
		r2 = metacharinc(&range);
		if (r1 <= ch && ch <= r2) {
		    if (indptr)
			*indptr += ch - r1;
		    return 1;
		}
		/* Careful not to screw up counting with bogus range */
		if (indptr && r1 < r2) {
		    /*
		     * This gets incremented again below to get
		     * us past the range end.  This is correct.
		     */
		    *indptr += r2 - r1;
		}
		break;
	    case PP_INCOMPLETE:
		if (zmb_ind == ZMB_INCOMPLETE)
		    return 1;
		break;
	    case PP_INVALID:
		if (zmb_ind == ZMB_INVALID)
		    return 1;
		break;
	    case PP_UNKWN:
		DPUTS(1, "BUG: unknown posix range passed through.\n");
		break;
	    default:
		DPUTS(1, "BUG: unknown metacharacter in range.");
		break;
	    }
	} else if (metacharinc(&range) == ch) {
	    if (mtp)
		*mtp = 0;
	    return 1;
	}
	if (indptr)
	    (*indptr)++;
    }
    return 0;
}


/*
 * This is effectively the reverse of mb_patmatchrange().
 * Given a range descriptor of the same form, and an index into it,
 * try to determine the character that is matched.  If the index
 * points to a [:...:] generic style match, set chr to WEOF and
 * return the type in mtp instead.  Return 1 if successful, 0 if
 * there was no corresponding index.  Note all pointer arguments
 * must be non-null.
 */

/**/
mod_export int
mb_patmatchindex(char *range, wint_t ind, wint_t *chr, int *mtp)
{
    wchar_t r1, r2, rchr;
    wint_t rdiff;

    *chr = WEOF;
    *mtp = 0;

    while (*range) {
	if (imeta(STOUC(*range))) {
	    int swtype = STOUC(*range++) - STOUC(Meta);
	    switch (swtype) {
	    case 0:
		range--;
		rchr = metacharinc(&range);
		if (!ind) {
		    *chr = (wint_t) rchr;
		    return 1;
		}
		break;

	    case PP_ALPHA:
	    case PP_ALNUM:
	    case PP_ASCII:
	    case PP_BLANK:
	    case PP_CNTRL:
	    case PP_DIGIT:
	    case PP_GRAPH:
	    case PP_LOWER:
	    case PP_PRINT:
	    case PP_PUNCT:
	    case PP_SPACE:
	    case PP_UPPER:
	    case PP_XDIGIT:
	    case PP_IDENT:
	    case PP_IFS:
	    case PP_IFSSPACE:
	    case PP_WORD:
	    case PP_INCOMPLETE:
	    case PP_INVALID:
		if (!ind) {
		    *mtp = swtype;
		    return 1;
		}
		break;

	    case PP_RANGE:
		r1 = metacharinc(&range);
		r2 = metacharinc(&range);
		rdiff = (wint_t)r2 - (wint_t)r1; 
		if (rdiff >= ind) {
		    *chr = (wint_t)r1 + ind;
		    return 1;
		}
		/* note the extra decrement to ind below */
		ind -= rdiff;
		break;
	    case PP_UNKWN:
		DPUTS(1, "BUG: unknown posix range passed through.\n");
		break;
	    default:
		DPUTS(1, "BUG: unknown metacharacter in range.");
		break;
	    }
	} else {
	    rchr = metacharinc(&range);
	    if (!ind) {
		*chr = (wint_t)rchr;
		return 1;
	    }
	}
	if (!ind--)
	    break;
    }

    /* No corresponding index. */
    return 0;
}

/**/
#endif /* MULTIBYTE_SUPPORT */

/*
 * Identical function to mb_patmatchrange() above for single-byte
 * characters.
 */

/**/
mod_export int
patmatchrange(char *range, int ch, int *indptr, int *mtp)
{
    int r1, r2;

    if (indptr)
	*indptr = 0;
    /*
     * Careful here: unlike other strings, range is a NULL-terminated,
     * metafied string, because we need to treat the Posix and hyphenated
     * ranges specially.
     */
    for (; *range; range++) {
	if (imeta(STOUC(*range))) {
	    int swtype = STOUC(*range) - STOUC(Meta);
	    if (mtp)
		*mtp = swtype;
	    switch (swtype) {
	    case 0:
		if (STOUC(*++range ^ 32) == ch)
		    return 1;
		break;
	    case PP_ALPHA:
		if (isalpha(ch))
		    return 1;
		break;
	    case PP_ALNUM:
		if (isalnum(ch))
		    return 1;
		break;
	    case PP_ASCII:
		if ((ch & ~0x7f) == 0)
		    return 1;
		break;
	    case PP_BLANK:
#if !defined(HAVE_ISBLANK) && !defined(isblank)
/*
 * isblank() is GNU and C99. There's a remote chance that some
 * systems still don't support it.
 */
#define isblank(c) (c == ' ' || c == '\t')
#endif
		if (isblank(ch))
		    return 1;
		break;
	    case PP_CNTRL:
		if (iscntrl(ch))
		    return 1;
		break;
	    case PP_DIGIT:
		if (isdigit(ch))
		    return 1;
		break;
	    case PP_GRAPH:
		if (isgraph(ch))
		    return 1;
		break;
	    case PP_LOWER:
		if (islower(ch))
		    return 1;
		break;
	    case PP_PRINT:
		if (ZISPRINT(ch))
		    return 1;
		break;
	    case PP_PUNCT:
		if (ispunct(ch))
		    return 1;
		break;
	    case PP_SPACE:
		if (isspace(ch))
		    return 1;
		break;
	    case PP_UPPER:
		if (isupper(ch))
		    return 1;
		break;
	    case PP_XDIGIT:
		if (isxdigit(ch))
		    return 1;
		break;
	    case PP_IDENT:
		if (iident(ch))
		    return 1;
		break;
	    case PP_IFS:
		if (isep(ch))
		    return 1;
		break;
	    case PP_IFSSPACE:
		if (iwsep(ch))
		    return 1;
		break;
	    case PP_WORD:
		if (iword(ch))
		    return 1;
		break;
	    case PP_RANGE:
		range++;
		r1 = STOUC(UNMETA(range));
		METACHARINC(range);
		r2 = STOUC(UNMETA(range));
		if (*range == Meta)
		    range++;
		if (r1 <= ch && ch <= r2) {
		    if (indptr)
			*indptr += ch - r1;
		    return 1;
		}
		if (indptr && r1 < r2)
		    *indptr += r2 - r1;
		break;
	    case PP_INCOMPLETE:
	    case PP_INVALID:
		/* Never true if not in multibyte mode */
		break;
	    case PP_UNKWN:
		DPUTS(1, "BUG: unknown posix range passed through.\n");
		break;
	    default:
		DPUTS(1, "BUG: unknown metacharacter in range.");
		break;
	    }
	} else if (STOUC(*range) == ch) {
	    if (mtp)
		*mtp = 0;
	    return 1;
	}
	if (indptr)
	    (*indptr)++;
    }
    return 0;
}


/**/
#ifndef MULTIBYTE_SUPPORT

/*
 * Identical function to mb_patmatchindex() above for single-byte
 * characters.  Here -1 represents a character that needs a special type.
 *
 * Unlike patmatchrange, we only need this in ZLE, which always
 * uses MULTIBYTE_SUPPORT if compiled in; hence we don't use
 * this function in that case.
 */

/**/
mod_export int
patmatchindex(char *range, int ind, int *chr, int *mtp)
{
    int r1, r2, rdiff, rchr;

    *chr = -1;
    *mtp = 0;

    for (; *range; range++) {
	if (imeta(STOUC(*range))) {
	    int swtype = STOUC(*range) - STOUC(Meta);
	    switch (swtype) {
	    case 0:
		/* ordinary metafied character */
		rchr = STOUC(*++range) ^ 32;
		if (!ind) {
		    *chr = rchr;
		    return 1;
		}
		break;

	    case PP_ALPHA:
	    case PP_ALNUM:
	    case PP_ASCII:
	    case PP_BLANK:
	    case PP_CNTRL:
	    case PP_DIGIT:
	    case PP_GRAPH:
	    case PP_LOWER:
	    case PP_PRINT:
	    case PP_PUNCT:
	    case PP_SPACE:
	    case PP_UPPER:
	    case PP_XDIGIT:
	    case PP_IDENT:
	    case PP_IFS:
	    case PP_IFSSPACE:
	    case PP_WORD:
	    case PP_INCOMPLETE:
	    case PP_INVALID:
		if (!ind) {
		    *mtp = swtype;
		    return 1;
		}
		break;

	    case PP_RANGE:
		range++;
		r1 = STOUC(UNMETA(range));
		METACHARINC(range);
		r2 = STOUC(UNMETA(range));
		if (*range == Meta)
		    range++;
		rdiff = r2 - r1; 
		if (rdiff >= ind) {
		    *chr = r1 + ind;
		    return 1;
		}
		/* note the extra decrement to ind below */
		ind -= rdiff;
		break;
	    case PP_UNKWN:
		DPUTS(1, "BUG: unknown posix range passed through.\n");
		break;
	    default:
		DPUTS(1, "BUG: unknown metacharacter in range.");
		break;
	    }
	} else {
	    if (!ind) {
		*chr = STOUC(*range);
		return 1;
	    }
	}
	if (!ind--)
	    break;
    }

    /* No corresponding index. */
    return 0;
}

/**/
#endif /* MULTIBYTE_SUPPORT */

/*
 * Repeatedly match something simple and say how many times.
 * charstart is an array parallel to that starting at patinput
 * and records the start of (possibly multibyte) characters
 * to aid in later backtracking.
 */

/**/
static int patrepeat(Upat p, char *charstart)
{
    int count = 0;
    patint_t tch, charmatch_cache;
    char *scan, *opnd;

    scan = patinput;
    opnd = (char *)P_OPERAND(p);

    switch(P_OP(p)) {
#ifdef DEBUG
    case P_ANY:
	dputs("BUG: ?# did not get optimized to *");
	return 0;
	break;
#endif
    case P_EXACTLY:
	DPUTS(P_LS_LEN(p) != 1, "closure following more than one character");
	tch = CHARREF(P_LS_STR(p), P_LS_STR(p) + P_LS_LEN(p));
	while (scan < patinend &&
	       CHARMATCH_EXPR(CHARREF(scan, patinend), tch)) {
	    charstart[scan-patinput] = 1;
	    count++;
	    CHARINC(scan, patinend);
	}
	break;
    case P_ANYOF:
    case P_ANYBUT:
	while (scan < patinend) {
#ifdef MULTIBYTE_SUPPORT
	    int zmb_ind;
	    wchar_t cr = charref(scan, patinend, &zmb_ind);
	    if (patglobflags & GF_MULTIBYTE) {
		if (mb_patmatchrange(opnd, cr, zmb_ind, NULL, NULL) ^
		    (P_OP(p) == P_ANYOF))
		    break;
	    } else if (patmatchrange(opnd, (int)cr, NULL, NULL) ^
		       (P_OP(p) == P_ANYOF))
		break;
#else
	    if (patmatchrange(opnd, CHARREF(scan, patinend), NULL, NULL) ^
		(P_OP(p) == P_ANYOF))
		break;
#endif
	    charstart[scan-patinput] = 1;
	    count++;
	    CHARINC(scan, patinend);
	}
	break;
#ifdef DEBUG
    default:
	dputs("BUG: something very strange is happening in patrepeat");
	return 0;
	break;
#endif
    }

    patinput = scan;
    return count;
}

/* Free a patprog. */

/**/
mod_export void
freepatprog(Patprog prog)
{
    if (prog && prog != dummy_patprog1 && prog != dummy_patprog2)
	zfree(prog, prog->size);
}

/* Disable or reenable a pattern character */

/**/
int
pat_enables(const char *cmd, char **patp, int enable)
{
    int ret = 0;
    const char **stringp;
    char *disp;

    if (!*patp) {
	int done = 0;
	for (stringp = zpc_strings, disp = zpc_disables;
	     stringp < zpc_strings + ZPC_COUNT;
	     stringp++, disp++) {
	    if (!*stringp)
		continue;
	    if (enable ? *disp : !*disp)
		continue;
	    if (done)
		putc(' ', stdout);
	    printf("'%s'", *stringp);
	    done = 1;
	}
	if (done)
	    putc('\n', stdout);
	return 0;
    }

    for (; *patp; patp++) {
	for (stringp = zpc_strings, disp = zpc_disables;
	     stringp < zpc_strings + ZPC_COUNT;
	     stringp++, disp++) {
	    if (*stringp && !strcmp(*stringp, *patp)) {
		*disp = (char)!enable;
		break;
	    }
	}
	if (stringp == zpc_strings + ZPC_COUNT) {
	    zerrnam(cmd, "invalid pattern: %s", *patp);
	    ret = 1;
	}
    }

    return ret;
}

/*
 * Save the current state of pattern disables, returning the saved value.
 */

/**/
unsigned int
savepatterndisables(void)
{
    unsigned int disables, bit;
    char *disp;

    disables = 0;
    for (bit = 1, disp = zpc_disables;
	 disp < zpc_disables + ZPC_COUNT;
	 bit <<= 1, disp++) {
	if (*disp)
	    disables |= bit;
    }
    return disables;
}

/*
 * Function scope saving pattern enables.
 */

/**/
void
startpatternscope(void)
{
    Zpc_disables_save newdis;

    newdis = (Zpc_disables_save)zalloc(sizeof(*newdis));
    newdis->next = zpc_disables_stack;
    newdis->disables = savepatterndisables();

    zpc_disables_stack = newdis;
}

/*
 * Restore completely the state of pattern disables.
 */

/**/
void
restorepatterndisables(unsigned int disables)
{
    char *disp;
    unsigned int bit;

    for (bit = 1, disp = zpc_disables;
	 disp < zpc_disables + ZPC_COUNT;
	 bit <<= 1, disp++) {
	if (disables & bit)
	    *disp = 1;
	else
	    *disp = 0;
    }
}

/*
 * Function scope to restore pattern enables if localpatterns is turned on.
 */

/**/
void
endpatternscope(void)
{
    Zpc_disables_save olddis;

    olddis = zpc_disables_stack;
    zpc_disables_stack = olddis->next;

    if (isset(LOCALPATTERNS))
	restorepatterndisables(olddis->disables);

    zfree(olddis, sizeof(*olddis));
}

/* Reinitialise pattern disables */

/**/
void
clearpatterndisables(void)
{
    memset(zpc_disables, 0, ZPC_COUNT);
}


/* Check to see if str is eligible for filename generation. */

/**/
mod_export int
haswilds(char *str)
{
    char *start;

    /* `[' and `]' are legal even if bad patterns are usually not. */
    if ((*str == Inbrack || *str == Outbrack) && !str[1])
	return 0;

    /* If % is immediately followed by ?, then that ? is     *
     * not treated as a wildcard.  This is so you don't have *
     * to escape job references such as %?foo.               */
    if (str[0] == '%' && str[1] == Quest)
	str[1] = '?';

    /*
     * Note that at this point zpc_special has not been set up.
     */
    start = str;
    for (; *str; str++) {
	switch (*str) {
	    case Inpar:
		if ((!isset(SHGLOB) && !zpc_disables[ZPC_INPAR]) ||
		    (str > start && isset(KSHGLOB) &&
		     ((str[-1] == Quest && !zpc_disables[ZPC_KSH_QUEST]) ||
		      (str[-1] == Star && !zpc_disables[ZPC_KSH_STAR]) ||
		      (str[-1] == '+' && !zpc_disables[ZPC_KSH_PLUS]) ||
		      (str[-1] == Bang && !zpc_disables[ZPC_KSH_BANG]) ||
		      (str[-1] == '!' && !zpc_disables[ZPC_KSH_BANG2]) ||
		      (str[-1] == '@' && !zpc_disables[ZPC_KSH_AT]))))
		    return 1;
		break;

	    case Bar:
		if (!zpc_disables[ZPC_BAR])
		    return 1;
		break;

	    case Star:
		if (!zpc_disables[ZPC_STAR])
		    return 1;
		break;

	    case Inbrack:
		if (!zpc_disables[ZPC_INBRACK])
		    return 1;
		break;

	    case Inang:
		if (!zpc_disables[ZPC_INANG])
		    return 1;
		break;

	    case Quest:
		if (!zpc_disables[ZPC_QUEST])
		    return 1;
		break;

	    case Pound:
		if (isset(EXTENDEDGLOB) && !zpc_disables[ZPC_HASH])
		    return 1;
		break;

	    case Hat:
		if (isset(EXTENDEDGLOB) && !zpc_disables[ZPC_HAT])
		    return 1;
		break;
	}
    }
    return 0;
}
