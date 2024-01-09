/*
 * pcre.c - interface to the PCRE library
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2007 Clint Adams
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Clint Adams or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Andrew Main and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Clint Adams and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Andrew Main and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */


#include "pcre.mdh"
#include "pcre.pro"

#define CPCRE_PLAIN 0

/**/
#if defined(HAVE_PCRE_COMPILE) && defined(HAVE_PCRE_EXEC)
#include <pcre.h>

static pcre *pcre_pattern;
static pcre_extra *pcre_hints;

/**/
static int
zpcre_utf8_enabled(void)
{
#if defined(MULTIBYTE_SUPPORT) && defined(HAVE_NL_LANGINFO) && defined(CODESET)
    static int have_utf8_pcre = -1;

    /* value can toggle based on MULTIBYTE, so don't
     * be too eager with caching */
    if (have_utf8_pcre < -1)
	return 0;

    if (!isset(MULTIBYTE))
	return 0;

    if ((have_utf8_pcre == -1) &&
        (!strcmp(nl_langinfo(CODESET), "UTF-8"))) {

	if (pcre_config(PCRE_CONFIG_UTF8, &have_utf8_pcre))
	    have_utf8_pcre = -2; /* erk, failed to ask */
    }

    if (have_utf8_pcre < 0)
	return 0;
    return have_utf8_pcre;

#else
    return 0;
#endif
}

/**/
static int
bin_pcre_compile(char *nam, char **args, Options ops, UNUSED(int func))
{
    int pcre_opts = 0, pcre_errptr, target_len;
    const char *pcre_error;
    char *target;
    
    if(OPT_ISSET(ops,'a')) pcre_opts |= PCRE_ANCHORED;
    if(OPT_ISSET(ops,'i')) pcre_opts |= PCRE_CASELESS;
    if(OPT_ISSET(ops,'m')) pcre_opts |= PCRE_MULTILINE;
    if(OPT_ISSET(ops,'x')) pcre_opts |= PCRE_EXTENDED;
    if(OPT_ISSET(ops,'s')) pcre_opts |= PCRE_DOTALL;
    
    if (zpcre_utf8_enabled())
	pcre_opts |= PCRE_UTF8;

#ifdef HAVE_PCRE_STUDY
    if (pcre_hints)
#ifdef PCRE_CONFIG_JIT
	pcre_free_study(pcre_hints);
#else
	pcre_free(pcre_hints);
#endif
    pcre_hints = NULL;
#endif

    if (pcre_pattern)
	pcre_free(pcre_pattern);
    pcre_pattern = NULL;

    target = ztrdup(*args);
    unmetafy(target, &target_len);

    if ((int)strlen(target) != target_len) {
	zwarnnam(nam, "embedded NULs in PCRE pattern terminate pattern");
    }

    pcre_pattern = pcre_compile(target, pcre_opts, &pcre_error, &pcre_errptr, NULL);

    free(target);

    if (pcre_pattern == NULL)
    {
	zwarnnam(nam, "error in regex: %s", pcre_error);
	return 1;
    }
    
    return 0;
}

/**/
#ifdef HAVE_PCRE_STUDY

/**/
static int
bin_pcre_study(char *nam, UNUSED(char **args), UNUSED(Options ops), UNUSED(int func))
{
    const char *pcre_error;

    if (pcre_pattern == NULL)
    {
	zwarnnam(nam, "no pattern has been compiled for study");
	return 1;
    }
    
    if (pcre_hints)
#ifdef PCRE_CONFIG_JIT
	pcre_free_study(pcre_hints);
#else
	pcre_free(pcre_hints);
#endif
    pcre_hints = NULL;

    pcre_hints = pcre_study(pcre_pattern, 0, &pcre_error);
    if (pcre_error != NULL)
    {
	zwarnnam(nam, "error while studying regex: %s", pcre_error);
	return 1;
    }
    
    return 0;
}

/**/
#else /* !HAVE_PCRE_STUDY */

# define bin_pcre_study bin_notavail

/**/
#endif /* !HAVE_PCRE_STUDY */

/**/
static int
zpcre_get_substrings(char *arg, int *ovec, int captured_count, char *matchvar,
		     char *substravar, int want_offset_pair, int matchedinarr,
		     int want_begin_end)
{
    char **captures, *match_all, **matches;
    char offset_all[50];
    int capture_start = 1;

    if (matchedinarr) {
	/* bash-style captures[0] entire-matched string in the array */
	capture_start = 0;
    }

    /* captures[0] will be entire matched string, [1] first substring */
    if (!pcre_get_substring_list(arg, ovec, captured_count, (const char ***)&captures)) {
	int nelem = arrlen(captures)-1;
	/* Set to the offsets of the complete match */
	if (want_offset_pair) {
	    sprintf(offset_all, "%d %d", ovec[0], ovec[1]);
	    setsparam("ZPCRE_OP", ztrdup(offset_all));
	}
	/*
	 * Result strings can contain embedded NULs; the length of each is the
	 * difference between the two values in each paired entry in ovec.
	 * ovec is length 2*(1+capture_list_length)
	 */
	if (matchvar) {
	    match_all = metafy(captures[0], ovec[1] - ovec[0], META_DUP);
	    setsparam(matchvar, match_all);
	}
	/*
	 * If we're setting match, mbegin, mend we only do
	 * so if there were parenthesised matches, for consistency
	 * (c.f. regex.c).  That's the next block after this one.
	 * Here we handle the simpler case where we don't worry about
	 * Unicode lengths, etc.
	 * Either !want_begin_end (ie, this is bash) or nelem; if bash
	 * then we're invoked always, even without nelem results, to
	 * set the array variable with one element in it, the complete match.
	 */
	if (substravar &&
	    (!want_begin_end || nelem)) {
	    char **x, **y;
	    int vec_off, i;
	    y = &captures[capture_start];
	    matches = x = (char **) zalloc(sizeof(char *) * (captured_count+1-capture_start));
	    for (i = capture_start; i < captured_count; i++, y++) {
		vec_off = 2*i;
		if (*y)
		    *x++ = metafy(*y, ovec[vec_off+1]-ovec[vec_off], META_DUP);
		else
		    *x++ = NULL;
	    }
	    *x = NULL;
	    setaparam(substravar, matches);
	}

	if (want_begin_end) {
	    /*
	     * cond-infix rather than builtin; also not bash; so we set a bunch
	     * of variables and arrays to values which require handling Unicode
	     * lengths
	     */
	    char *ptr = arg;
	    zlong offs = 0;
	    int clen, leftlen;

	    /* Count the characters before the match */
	    MB_CHARINIT();
	    leftlen = ovec[0];
	    while (leftlen) {
		offs++;
		clen = MB_CHARLEN(ptr, leftlen);
		ptr += clen;
		leftlen -= clen;
	    }
	    setiparam("MBEGIN", offs + !isset(KSHARRAYS));
	    /* Add on the characters in the match */
	    leftlen = ovec[1] - ovec[0];
	    while (leftlen) {
		offs++;
		clen = MB_CHARLEN(ptr, leftlen);
		ptr += clen;
		leftlen -= clen;
	    }
	    setiparam("MEND", offs + !isset(KSHARRAYS) - 1);
	    if (nelem) {
		char **mbegin, **mend, **bptr, **eptr;
		int i, *ipair;

		bptr = mbegin = zalloc(sizeof(char*)*(nelem+1));
		eptr = mend = zalloc(sizeof(char*)*(nelem+1));

		for (ipair = ovec + 2, i = 0;
		     i < nelem;
		     ipair += 2, i++, bptr++, eptr++)
		{
		    char buf[DIGBUFSIZE];
		    ptr = arg;
		    offs = 0;
		    /* Find the start offset */
		    MB_CHARINIT();
		    leftlen = ipair[0];
		    while (leftlen > 0) {
			offs++;
			clen = MB_CHARLEN(ptr, leftlen);
			ptr += clen;
			leftlen -= clen;
		    }
		    convbase(buf, offs + !isset(KSHARRAYS), 10);
		    *bptr = ztrdup(buf);
		    /* Continue to the end offset */
		    leftlen = ipair[1] - ipair[0];
		    while (leftlen) {
			offs++;
			clen = MB_CHARLEN(ptr, leftlen);
			ptr += clen;
			leftlen -= clen;
		    }
		    convbase(buf, offs + !isset(KSHARRAYS) - 1, 10);
		    *eptr = ztrdup(buf);
		}
		*bptr = *eptr = NULL;

		setaparam("mbegin", mbegin);
		setaparam("mend", mend);
	    }
	}

	pcre_free_substring_list((const char **)captures);
    }

    return 0;
}

/**/
static int
getposint(char *instr, char *nam)
{
    char *eptr;
    int ret;

    ret = (int)zstrtol(instr, &eptr, 10);
    if (*eptr || ret < 0) {
	zwarnnam(nam, "integer expected: %s", instr);
	return -1;
    }

    return ret;
}

/**/
static int
bin_pcre_match(char *nam, char **args, Options ops, UNUSED(int func))
{
    int ret, capcount, *ovec, ovecsize, c;
    char *matched_portion = NULL;
    char *plaintext = NULL;
    char *receptacle = NULL;
    int return_value = 1;
    /* The subject length and offset start are both int values in pcre_exec */
    int subject_len;
    int offset_start = 0;
    int want_offset_pair = 0;

    if (pcre_pattern == NULL) {
	zwarnnam(nam, "no pattern has been compiled");
	return 1;
    }

    matched_portion = "MATCH";
    receptacle = "match";
    if(OPT_HASARG(ops,c='a')) {
	receptacle = OPT_ARG(ops,c);
    }
    if(OPT_HASARG(ops,c='v')) {
	matched_portion = OPT_ARG(ops,c);
    }
    if(OPT_HASARG(ops,c='n')) { /* The offset position to start the search, in bytes. */
	if ((offset_start = getposint(OPT_ARG(ops,c), nam)) < 0)
	    return 1;
    }
    /* For the entire match, 'Return' the offset byte positions instead of the matched string */
    if(OPT_ISSET(ops,'b')) want_offset_pair = 1;

    if ((ret = pcre_fullinfo(pcre_pattern, pcre_hints, PCRE_INFO_CAPTURECOUNT, &capcount)))
    {
	zwarnnam(nam, "error %d in fullinfo", ret);
	return 1;
    }

    ovecsize = (capcount+1)*3;
    ovec = zalloc(ovecsize*sizeof(int));

    plaintext = ztrdup(*args);
    unmetafy(plaintext, &subject_len);

    if (offset_start > 0 && offset_start >= subject_len)
	ret = PCRE_ERROR_NOMATCH;
    else
	ret = pcre_exec(pcre_pattern, pcre_hints, plaintext, subject_len, offset_start, 0, ovec, ovecsize);

    if (ret==0) return_value = 0;
    else if (ret==PCRE_ERROR_NOMATCH) /* no match */;
    else if (ret>0) {
	zpcre_get_substrings(plaintext, ovec, ret, matched_portion, receptacle,
			     want_offset_pair, 0, 0);
	return_value = 0;
    }
    else {
	zwarnnam(nam, "error in pcre_exec [%d]", ret);
    }
    
    if (ovec)
	zfree(ovec, ovecsize*sizeof(int));
    zsfree(plaintext);

    return return_value;
}

/**/
static int
cond_pcre_match(char **a, int id)
{
    pcre *pcre_pat;
    const char *pcre_err;
    char *lhstr, *rhre, *lhstr_plain, *rhre_plain, *avar, *svar;
    int r = 0, pcre_opts = 0, pcre_errptr, capcnt, *ov, ovsize;
    int lhstr_plain_len, rhre_plain_len;
    int return_value = 0;

    if (zpcre_utf8_enabled())
	pcre_opts |= PCRE_UTF8;
    if (isset(REMATCHPCRE) && !isset(CASEMATCH))
	pcre_opts |= PCRE_CASELESS;

    lhstr = cond_str(a,0,0);
    rhre = cond_str(a,1,0);
    lhstr_plain = ztrdup(lhstr);
    rhre_plain = ztrdup(rhre);
    unmetafy(lhstr_plain, &lhstr_plain_len);
    unmetafy(rhre_plain, &rhre_plain_len);
    pcre_pat = NULL;
    ov = NULL;
    ovsize = 0;

    if (isset(BASHREMATCH)) {
	svar = NULL;
	avar = "BASH_REMATCH";
    } else {
	svar = "MATCH";
	avar = "match";
    }

    switch(id) {
	 case CPCRE_PLAIN:
		if ((int)strlen(rhre_plain) != rhre_plain_len) {
		    zwarn("embedded NULs in PCRE pattern terminate pattern");
		}
		pcre_pat = pcre_compile(rhre_plain, pcre_opts, &pcre_err, &pcre_errptr, NULL);
		if (pcre_pat == NULL) {
		    zwarn("failed to compile regexp /%s/: %s", rhre, pcre_err);
		    break;
		}
                pcre_fullinfo(pcre_pat, NULL, PCRE_INFO_CAPTURECOUNT, &capcnt);
    		ovsize = (capcnt+1)*3;
		ov = zalloc(ovsize*sizeof(int));
    		r = pcre_exec(pcre_pat, NULL, lhstr_plain, lhstr_plain_len, 0, 0, ov, ovsize);
		/* r < 0 => error; r==0 match but not enough size in ov
		 * r > 0 => (r-1) substrings found; r==1 => no substrings
		 */
    		if (r==0) {
		    zwarn("reportable zsh problem: pcre_exec() returned 0");
		    return_value = 1;
		    break;
		}
	        else if (r==PCRE_ERROR_NOMATCH) {
		    return_value = 0; /* no match */
		    break;
		}
		else if (r<0) {
		    zwarn("pcre_exec() error [%d]", r);
		    break;
		}
                else if (r>0) {
		    zpcre_get_substrings(lhstr_plain, ov, r, svar, avar, 0,
					 isset(BASHREMATCH),
					 !isset(BASHREMATCH));
		    return_value = 1;
		    break;
		}
		break;
    }

    if (lhstr_plain)
	free(lhstr_plain);
    if(rhre_plain)
	free(rhre_plain);
    if (pcre_pat)
	pcre_free(pcre_pat);
    if (ov)
	zfree(ov, ovsize*sizeof(int));

    return return_value;
}

static struct conddef cotab[] = {
    CONDDEF("pcre-match", CONDF_INFIX, cond_pcre_match, 0, 0, CPCRE_PLAIN)
    /* CONDDEF can register =~ but it won't be found */
};

/**/
#else /* !(HAVE_PCRE_COMPILE && HAVE_PCRE_EXEC) */

# define bin_pcre_compile bin_notavail
# define bin_pcre_study bin_notavail
# define bin_pcre_match bin_notavail

/**/
#endif /* !(HAVE_PCRE_COMPILE && HAVE_PCRE_EXEC) */

static struct builtin bintab[] = {
    BUILTIN("pcre_compile", 0, bin_pcre_compile, 1, 1, 0, "aimxs",  NULL),
    BUILTIN("pcre_match",   0, bin_pcre_match,   1, 1, 0, "a:v:n:b",    NULL),
    BUILTIN("pcre_study",   0, bin_pcre_study,   0, 0, 0, NULL,    NULL)
};


static struct features module_features = {
    bintab, sizeof(bintab)/sizeof(*bintab),
#if defined(HAVE_PCRE_COMPILE) && defined(HAVE_PCRE_EXEC)
    cotab, sizeof(cotab)/sizeof(*cotab),
#else /* !(HAVE_PCRE_COMPILE && HAVE_PCRE_EXEC) */
    NULL, 0,
#endif /* !(HAVE_PCRE_COMPILE && HAVE_PCRE_EXEC) */
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
#if defined(HAVE_PCRE_COMPILE) && defined(HAVE_PCRE_EXEC)
#ifdef HAVE_PCRE_STUDY
    if (pcre_hints)
#ifdef PCRE_CONFIG_JIT
	pcre_free_study(pcre_hints);
#else
	pcre_free(pcre_hints);
#endif
    pcre_hints = NULL;
#endif

    if (pcre_pattern)
	pcre_free(pcre_pattern);
    pcre_pattern = NULL;
#endif

    return 0;
}
