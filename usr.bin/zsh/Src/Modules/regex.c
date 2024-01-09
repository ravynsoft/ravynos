/*
 * regex.c
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 2007,2012 Phil Pennock
 * All Rights Reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Phil Pennock or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Phil Pennock and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Phil Pennock and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Phil Pennock and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "regex.mdh"
#include "regex.pro"

#include <regex.h>

/* we default to a vaguely modern syntax and set of capabilities */
#define ZREGEX_EXTENDED 0
/* if you want Basic syntax, make it an alternative options */

static void
zregex_regerrwarn(int r, regex_t *re, char *msg)
{
    char *errbuf;
    size_t errbufsz;

    errbufsz = regerror(r, re, NULL, 0);
    errbuf = zalloc(errbufsz*sizeof(char));
    regerror(r, re, errbuf, errbufsz);
    zwarn("%s: %s", msg, errbuf);
    zfree(errbuf, errbufsz);
}

/**/
static int
zcond_regex_match(char **a, int id)
{
    regex_t re;
    regmatch_t *m, *matches = NULL;
    size_t matchessz = 0;
    char *lhstr, *lhstr_zshmeta, *rhre, *rhre_zshmeta, *s, **arr, **x;
    int r, n, return_value, rcflags, reflags, nelem, start;

    lhstr_zshmeta = cond_str(a,0,0);
    rhre_zshmeta = cond_str(a,1,0);
    rcflags = reflags = 0;
    return_value = 0; /* 1 => matched successfully */

    lhstr = ztrdup(lhstr_zshmeta);
    unmetafy(lhstr, NULL);
    rhre = ztrdup(rhre_zshmeta);
    unmetafy(rhre, NULL);

    switch(id) {
    case ZREGEX_EXTENDED:
	rcflags |= REG_EXTENDED;
	if (!isset(CASEMATCH))
	    rcflags |= REG_ICASE;
	r = regcomp(&re, rhre, rcflags);
	if (r) {
	    zregex_regerrwarn(r, &re, "failed to compile regex");
	    break;
	}
	/* re.re_nsub is number of parenthesized groups, we also need
	 * 1 for the 0 offset, which is the entire matched portion
	 */
	if ((int)re.re_nsub < 0) {
	    zwarn("INTERNAL ERROR: regcomp() returned "
		    "negative subpattern count %d", (int)re.re_nsub);
	    break;
	}
	matchessz = (re.re_nsub + 1) * sizeof(regmatch_t);
	matches = zalloc(matchessz);
	r = regexec(&re, lhstr, re.re_nsub+1, matches, reflags);
	if (r == REG_NOMATCH)
	    ; /* We do nothing when we fail to match. */
	else if (r == 0) {
	    return_value = 1;
	    if (isset(BASHREMATCH)) {
		start = 0;
		nelem = re.re_nsub + 1;
	    } else {
		start = 1;
		nelem = re.re_nsub;
	    }
	    arr = NULL; /* bogus gcc warning of used uninitialised */
	    /* entire matched portion + re_nsub substrings + NULL */
	    if (nelem) {
		arr = x = (char **) zalloc(sizeof(char *) * (nelem + 1));
		for (m = matches + start, n = start; n <= (int)re.re_nsub; ++n, ++m, ++x) {
		    *x = metafy(lhstr + m->rm_so, m->rm_eo - m->rm_so, META_DUP);
		}
		*x = NULL;
	    }
	    if (isset(BASHREMATCH)) {
		assignaparam("BASH_REMATCH", arr, 0);
	    } else {
		zlong offs;
		char *ptr;
		int clen, leftlen;

		m = matches;
		s = metafy(lhstr + m->rm_so, m->rm_eo - m->rm_so, META_DUP);
		assignsparam("MATCH", s, 0);
		/*
		 * Count the characters before the match.
		 */
		ptr = lhstr;
		leftlen = m->rm_so;
		offs = 0;
		MB_CHARINIT();
		while (leftlen) {
		    offs++;
		    clen = MB_CHARLEN(ptr, leftlen);
		    ptr += clen;
		    leftlen -= clen;
		}
		assigniparam("MBEGIN", offs + !isset(KSHARRAYS), 0);
		/*
		 * Add on the characters in the match.
		 */
		leftlen = m->rm_eo - m->rm_so;
		while (leftlen) {
		    offs++;
		    clen = MB_CHARLEN(ptr, leftlen);
		    ptr += clen;
		    leftlen -= clen;
		}
		assigniparam("MEND", offs + !isset(KSHARRAYS) - 1, 0);
		if (nelem) {
		    char **mbegin, **mend, **bptr, **eptr;
		    bptr = mbegin = (char **)zalloc(sizeof(char *)*(nelem+1));
		    eptr = mend = (char **)zalloc(sizeof(char *)*(nelem+1));

		    for (m = matches + start, n = 0;
			 n < nelem;
			 ++n, ++m, ++bptr, ++eptr)
		    {
			char buf[DIGBUFSIZE];
			if (m->rm_so < 0 || m->rm_eo < 0) {
			    *bptr = ztrdup("-1");
			    *eptr = ztrdup("-1");
			    continue;
			}
			ptr = lhstr;
			leftlen = m->rm_so;
			offs = 0;
			/* Find the start offset */
			MB_CHARINIT();
			while (leftlen) {
			    offs++;
			    clen = MB_CHARLEN(ptr, leftlen);
			    ptr += clen;
			    leftlen -= clen;
			}
			convbase(buf, offs + !isset(KSHARRAYS), 10);
			*bptr = ztrdup(buf);
			/* Continue to the end offset */
			leftlen = m->rm_eo - m->rm_so;
			while (leftlen ) {
			    offs++;
			    clen = MB_CHARLEN(ptr, leftlen);
			    ptr += clen;
			    leftlen -= clen;
			}
			convbase(buf, offs + !isset(KSHARRAYS) - 1, 10);
			*eptr = ztrdup(buf);
		    }
		    *bptr = *eptr = NULL;

		    setaparam("match", arr);
		    setaparam("mbegin", mbegin);
		    setaparam("mend", mend);
		}
	    }
	}
	else
	    zregex_regerrwarn(r, &re, "regex matching error");
	break;
    default:
	DPUTS(1, "bad regex option");
	return_value = 0;
	goto CLEAN_BASEMETA;
    }

    if (matches)
	zfree(matches, matchessz);
    regfree(&re);
CLEAN_BASEMETA:
    free(lhstr);
    free(rhre);
    return return_value;
}

static struct conddef cotab[] = {
    CONDDEF("regex-match", CONDF_INFIX, zcond_regex_match, 0, 0, ZREGEX_EXTENDED)
};


static struct features module_features = {
    NULL, 0,
    cotab, sizeof(cotab)/sizeof(*cotab),
    NULL, 0,
    NULL, 0,
    0
};


/**/
int
setup_(UNUSED(Module m))
{
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
    return 0;
}
