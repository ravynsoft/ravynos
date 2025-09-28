#define PERL_NO_GET_CONTEXT

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "bsd_glob.h"

#define MY_CXT_KEY "File::Glob::_guts" XS_VERSION

typedef struct {
#ifdef USE_ITHREADS
    tTHX interp;
#endif
    int		x_GLOB_ERROR;
    HV *	x_GLOB_ENTRIES;
    Perl_ophook_t	x_GLOB_OLD_OPHOOK;
} my_cxt_t;

START_MY_CXT

#define GLOB_ERROR	(MY_CXT.x_GLOB_ERROR)

#include "const-c.inc"

#ifdef WIN32
#define errfunc		NULL
#else
static int
errfunc(const char *foo, int bar) {
  PERL_UNUSED_ARG(foo);
  return !(bar == EACCES || bar == ENOENT || bar == ENOTDIR);
}
#endif

static void
doglob(pTHX_ const char *pattern, int flags)
{
    dSP;
    glob_t pglob;
    int i;
    int retval;
    SV *tmp;
    {
	dMY_CXT;

	/* call glob */
	memset(&pglob, 0, sizeof(glob_t));
	retval = bsd_glob(pattern, flags, errfunc, &pglob);
	GLOB_ERROR = retval;

	/* return any matches found */
	EXTEND(sp, pglob.gl_pathc);
	for (i = 0; i < pglob.gl_pathc; i++) {
	    /* printf("# bsd_glob: %s\n", pglob.gl_pathv[i]); */
	    tmp = newSVpvn_flags(pglob.gl_pathv[i], strlen(pglob.gl_pathv[i]),
				 SVs_TEMP);
	    TAINT;
	    SvTAINT(tmp);
	    PUSHs(tmp);
	}
	PUTBACK;

	bsd_globfree(&pglob);
    }
}

static void
iterate(pTHX_ bool(*globber)(pTHX_ AV *entries, const char *pat, STRLEN len, bool is_utf8))
{
    dSP;
    dMY_CXT;

    const char * const cxixpv = (char *)&PL_op;
    STRLEN const cxixlen = sizeof(OP *);
    AV *entries;
    U32 const gimme = GIMME_V;
    SV *patsv = POPs;
    bool on_stack = FALSE;

    if (!MY_CXT.x_GLOB_ENTRIES) MY_CXT.x_GLOB_ENTRIES = newHV();
    entries = (AV *)*(hv_fetch(MY_CXT.x_GLOB_ENTRIES, cxixpv, cxixlen, 1));

    /* if we're just beginning, do it all first */
    if (SvTYPE(entries) != SVt_PVAV) {
        const char *pat;
        STRLEN len;
        bool is_utf8;

        /* glob without args defaults to $_ */
        SvGETMAGIC(patsv);
        if (
            !SvOK(patsv)
              && (patsv = DEFSV, SvGETMAGIC(patsv), !SvOK(patsv))
            ) {
            pat = "";
            len = 0;
            is_utf8 = 0;
        }
        else {
            pat = SvPV_nomg(patsv,len);
            is_utf8 = cBOOL(SvUTF8(patsv));
            /* the lower-level code expects a null-terminated string */
            if (!SvPOK(patsv) || pat != SvPVX(patsv) || pat[len] != '\0') {
                SV *newpatsv = newSVpvn_flags(pat, len, SVs_TEMP);
                pat = SvPV_nomg(newpatsv,len);
            }
        }

        if (!IS_SAFE_SYSCALL(pat, len, "pattern", "glob")) {
            if (gimme != G_LIST)
                PUSHs(&PL_sv_undef);
            PUTBACK;
            return;
        }

	PUTBACK;
	on_stack = globber(aTHX_ entries, pat, len, is_utf8);
	SPAGAIN;
    }

    /* chuck it all out, quick or slow */
    if (gimme == G_LIST) {
	if (!on_stack && AvFILLp(entries) + 1) {
	    EXTEND(SP, AvFILLp(entries)+1);
	    Copy(AvARRAY(entries), SP+1, AvFILLp(entries)+1, SV *);
	    SP += AvFILLp(entries)+1;
	}
	/* No G_DISCARD here!  It will free the stack items. */
	(void)hv_delete(MY_CXT.x_GLOB_ENTRIES, cxixpv, cxixlen, 0);
    }
    else {
	if (AvFILLp(entries) + 1) {
	    mPUSHs(av_shift(entries));
	}
	else {
	    /* return undef for EOL */
	    (void)hv_delete(MY_CXT.x_GLOB_ENTRIES, cxixpv, cxixlen, G_DISCARD);
	    PUSHs(&PL_sv_undef);
	}
    }
    PUTBACK;
}

/* returns true if the items are on the stack already, but only in
   list context */
static bool
csh_glob(pTHX_ AV *entries, const char *pat, STRLEN len, bool is_utf8)
{
	dSP;
	AV *patav = NULL;
	const char *patend;
	const char *s = NULL;
	const char *piece = NULL;
	SV *word = NULL;
	SV *flags_sv = get_sv("File::Glob::DEFAULT_FLAGS", GV_ADD);
	int const flags = (int)SvIV(flags_sv);
	U32 const gimme = GIMME_V;

	patend = pat + len;

	assert(SvTYPE(entries) != SVt_PVAV);
	sv_upgrade((SV *)entries, SVt_PVAV);

	/* extract patterns */
	s = pat-1;
	while (++s < patend) {
	    switch (*s) {
	    case '\'':
	    case '"' :
	      {
		bool found = FALSE;
		const char quote = *s;
		if (!word) {
		    word = newSVpvs("");
		    if (is_utf8) SvUTF8_on(word);
		}
		if (piece) sv_catpvn(word, piece, s-piece);
		piece = s+1;
		while (++s < patend)
		    if (*s == '\\') {
			s++;
			/* If the backslash is here to escape a quote,
			   obliterate it. */
			if (s < patend && *s == quote)
			    sv_catpvn(word, piece, s-piece-1), piece = s;
		    }
		    else if (*s == quote) {
			sv_catpvn(word, piece, s-piece);
			piece = NULL;
			found = TRUE;
			break;
		    }
		if (!found) { /* unmatched quote */
		    /* Give up on tokenisation and treat the whole string
		       as a single token, but with whitespace stripped. */
		    piece = pat;
		    while (isSPACE(*pat)) pat++;
		    while (isSPACE(*(patend-1))) patend--;
		    /* bsd_glob expects a trailing null, but we cannot mod-
		       ify the original */
		    if (patend < pat + len) {
			if (word) sv_setpvn(word, pat, patend-pat);
			else
			    word = newSVpvn_flags(
				pat, patend-pat, SVf_UTF8*is_utf8
			    );
			piece = NULL;
		    }
		    else {
			if (word) SvREFCNT_dec(word), word=NULL;
			piece = pat;
			s = patend;
		    }
		    goto end_of_parsing;
		}
		break;
	      }
	    case '\\':
		if (!piece) piece = s;
		s++;
		/* If the backslash is here to escape a quote,
		   obliterate it. */
		if (s < patend && (*s == '"' || *s == '\'')) {
		    if (!word) {
			word = newSVpvn(piece,s-piece-1);
			if (is_utf8) SvUTF8_on(word);
		    }
		    else sv_catpvn(word, piece, s-piece-1);
		    piece = s;
		}
		break;
	    default:
		if (isSPACE(*s)) {
		    if (piece) {
			if (!word) {
			    word = newSVpvn(piece,s-piece);
			    if (is_utf8) SvUTF8_on(word);
			}
			else sv_catpvn(word, piece, s-piece);
		    }
		    if (!word) break;
		    if (!patav) patav = (AV *)sv_2mortal((SV *)newAV());
		    av_push(patav, word);
		    word = NULL;
		    piece = NULL;
		}
		else if (!piece) piece = s;
		break;
	    }
	}
      end_of_parsing:

	if (patav) {
	    I32 items = AvFILLp(patav) + 1;
	    SV **svp = AvARRAY(patav);
	    while (items--) {
		PUSHMARK(SP);
		PUTBACK;
		doglob(aTHX_ SvPVXx(*svp++), flags);
		SPAGAIN;
		{
		    dMARK;
		    dORIGMARK;
		    while (++MARK <= SP)
			av_push(entries, SvREFCNT_inc_simple_NN(*MARK));
		    SP = ORIGMARK;
		}
	    }
	}
	/* piece is set at this point if there is no trailing whitespace.
	   It is the beginning of the last token or quote-delimited
	   piece thereof.  word is set at this point if the last token has
	   multiple quoted pieces. */
	if (piece || word) {
	    if (word) {
		if (piece) sv_catpvn(word, piece, s-piece);
		piece = SvPVX(word);
	    }
	    PUSHMARK(SP);
	    PUTBACK;
	    doglob(aTHX_ piece, flags);
	    if (word) SvREFCNT_dec(word);
	    SPAGAIN;
	    {
		dMARK;
		dORIGMARK;
		/* short-circuit here for a fairly common case */
		if (!patav && gimme == G_LIST) { PUTBACK; return TRUE; }
		while (++MARK <= SP)
		    av_push(entries, SvREFCNT_inc_simple_NN(*MARK));

		SP = ORIGMARK;
	    }
	}
	PUTBACK;
	return FALSE;
}

static void
csh_glob_iter(pTHX)
{
    iterate(aTHX_ csh_glob);
}

/* wrapper around doglob that can be passed to the iterator */
static bool
doglob_iter_wrapper(pTHX_ AV *entries, const char *pattern, STRLEN len, bool is_utf8)
{
    dSP;
    SV * flags_sv = get_sv("File::Glob::DEFAULT_FLAGS", GV_ADD);
    int const flags = (int)SvIV(flags_sv);

    PERL_UNUSED_VAR(len); /* we use \0 termination instead */
    /* XXX we currently just use the underlying bytes of the passed SV.
     * Some day someone needs to make glob utf8 aware */
    PERL_UNUSED_VAR(is_utf8);

    PUSHMARK(SP);
    PUTBACK;
    doglob(aTHX_ pattern, flags);
    SPAGAIN;
    {
	dMARK;
	dORIGMARK;
	if (GIMME_V == G_LIST) { PUTBACK; return TRUE; }
	sv_upgrade((SV *)entries, SVt_PVAV);
	while (++MARK <= SP)
	    av_push(entries, SvREFCNT_inc_simple_NN(*MARK));
	SP = ORIGMARK;
    }
    return FALSE;
}

static void
glob_ophook(pTHX_ OP *o)
{
  if (PL_dirty) return;
  {
    dMY_CXT;
    if (MY_CXT.x_GLOB_ENTRIES
     && (o->op_type == OP_GLOB || o->op_type == OP_ENTERSUB))
	(void)hv_delete(MY_CXT.x_GLOB_ENTRIES, (char *)&o, sizeof(OP *),
		  G_DISCARD);
    if (MY_CXT.x_GLOB_OLD_OPHOOK) MY_CXT.x_GLOB_OLD_OPHOOK(aTHX_ o);
  }
}

MODULE = File::Glob		PACKAGE = File::Glob

int
GLOB_ERROR()
    PREINIT:
	dMY_CXT;
    CODE:
	RETVAL = GLOB_ERROR;
    OUTPUT:
	RETVAL

void
bsd_glob(pattern_sv,...)
    SV *pattern_sv
PREINIT:
    int flags = 0;
    char *pattern;
    STRLEN len;
PPCODE:
    {
        pattern = SvPV(pattern_sv, len);
        if (!IS_SAFE_SYSCALL(pattern, len, "pattern", "bsd_glob"))
            XSRETURN(0);
	/* allow for optional flags argument */
	if (items > 1) {
	    flags = (int) SvIV(ST(1));
	    /* remove unsupported flags */
	    flags &= ~(GLOB_APPEND | GLOB_DOOFFS | GLOB_ALTDIRFUNC | GLOB_MAGCHAR);
	} else {
	    SV * flags_sv = get_sv("File::Glob::DEFAULT_FLAGS", GV_ADD);
	    flags = (int)SvIV(flags_sv);
	}
	
	PUTBACK;
	doglob(aTHX_ pattern, flags);
	SPAGAIN;
    }

PROTOTYPES: DISABLE
void
csh_glob(...)
PPCODE:
    /* For backward-compatibility with the original Perl function, we sim-
     * ply take the first argument, regardless of how many there are.
     */
    if (items) SP ++;
    else {
	XPUSHs(&PL_sv_undef);
    }
    PUTBACK;
    csh_glob_iter(aTHX);
    SPAGAIN;

void
bsd_glob_override(...)
PPCODE:
    if (items) SP ++;
    else {
	XPUSHs(&PL_sv_undef);
    }
    PUTBACK;
    iterate(aTHX_ doglob_iter_wrapper);
    SPAGAIN;

#ifdef USE_ITHREADS

void
CLONE(...)
INIT:
    HV *glob_entries_clone = NULL;
CODE:
    PERL_UNUSED_ARG(items);
    {
        dMY_CXT;
        if ( MY_CXT.x_GLOB_ENTRIES ) {
            CLONE_PARAMS param;
            param.stashes    = NULL;
            param.flags      = 0;
            param.proto_perl = MY_CXT.interp;
            
            glob_entries_clone = MUTABLE_HV(sv_dup_inc((SV*)MY_CXT.x_GLOB_ENTRIES, &param));
        }
    }
    {
        MY_CXT_CLONE;
        MY_CXT.x_GLOB_ENTRIES = glob_entries_clone;
        MY_CXT.interp = aTHX;
    }

#endif

BOOT:
{
#ifndef PERL_EXTERNAL_GLOB
    /* Don't do this at home! The globhook interface is highly volatile. */
    PL_globhook = csh_glob_iter;
#endif
}

BOOT:
{
    MY_CXT_INIT;
    {
	dMY_CXT;
	MY_CXT.x_GLOB_ENTRIES = NULL;
#ifdef USE_ITHREADS
        MY_CXT.interp = aTHX;
#endif
	if(!MY_CXT.x_GLOB_OLD_OPHOOK) {
	    MY_CXT.x_GLOB_OLD_OPHOOK = PL_opfreehook;
	    PL_opfreehook = glob_ophook;
	}
    }  
}

INCLUDE: const-xs.inc
