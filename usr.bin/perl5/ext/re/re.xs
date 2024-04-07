#if defined(PERL_EXT_RE_DEBUG) && !defined(DEBUGGING)
#  define DEBUGGING
#  define DEBUGGING_RE_ONLY
#endif

#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "re_comp.h"

#undef dXSBOOTARGSXSAPIVERCHK
/* skip API version checking due to different interp struct size but,
   this hack is until GitHub issue #14169 is resolved */
#define dXSBOOTARGSXSAPIVERCHK dXSBOOTARGSNOVERCHK

START_EXTERN_C

extern REGEXP*	my_re_compile (pTHX_ SV * const pattern, const U32 pm_flags);
extern REGEXP*	my_re_op_compile (pTHX_ SV ** const patternp, int pat_count,
		    OP *expr, const regexp_engine* eng, REGEXP *volatile old_re,
		     bool *is_bare_re, U32 rx_flags, U32 pm_flags);

extern I32	my_regexec (pTHX_ REGEXP * const prog, char* stringarg, char* strend,
			    char* strbeg, SSize_t minend, SV* screamer,
			    void* data, U32 flags);

extern char*	my_re_intuit_start(pTHX_
                    REGEXP * const rx,
                    SV *sv,
                    const char * const strbeg,
                    char *strpos,
                    char *strend,
                    const U32 flags,
                    re_scream_pos_data *data);

extern SV*	my_re_intuit_string (pTHX_ REGEXP * const prog);

extern void	my_regfree (pTHX_ REGEXP * const r);

extern void	my_reg_numbered_buff_fetch(pTHX_ REGEXP * const rx, const I32 paren,
					   SV * const usesv);
extern void	my_reg_numbered_buff_store(pTHX_ REGEXP * const rx, const I32 paren,
					   SV const * const value);
extern I32	my_reg_numbered_buff_length(pTHX_ REGEXP * const rx,
					    const SV * const sv, const I32 paren);

extern SV*	my_reg_named_buff(pTHX_ REGEXP * const, SV * const, SV * const,
                              const U32);
extern SV*	my_reg_named_buff_iter(pTHX_ REGEXP * const rx,
                                   const SV * const lastkey, const U32 flags);

extern SV*      my_reg_qr_package(pTHX_ REGEXP * const rx);
#if defined(USE_ITHREADS)
extern void*	my_regdupe (pTHX_ REGEXP * const r, CLONE_PARAMS *param);
#endif
extern void     my_regprop(pTHX_
    const regexp *prog, SV* sv, const regnode* o,
    const regmatch_info *reginfo, const RExC_state_t *pRExC_state
);

EXTERN_C const struct regexp_engine my_reg_engine;
EXTERN_C const struct regexp_engine wild_reg_engine;

END_EXTERN_C

const struct regexp_engine my_reg_engine = { 
        my_re_compile, 
        my_regexec, 
        my_re_intuit_start, 
        my_re_intuit_string, 
        my_regfree, 
        my_reg_numbered_buff_fetch,
        my_reg_numbered_buff_store,
        my_reg_numbered_buff_length,
        my_reg_named_buff,
        my_reg_named_buff_iter,
        my_reg_qr_package,
#if defined(USE_ITHREADS)
        my_regdupe,
#endif
        my_re_op_compile,
};

/* For use with Unicode property wildcards, when we want to see the compilation
 * of the wildcard subpattern, but don't want to see the matching process.  All
 * but the compilation are the regcomp.c/regexec.c functions which aren't
 * subject to 'use re' */
const struct regexp_engine wild_reg_engine = {
        my_re_compile,
        Perl_regexec_flags,
        Perl_re_intuit_start,
        Perl_re_intuit_string,
        Perl_regfree_internal,
        Perl_reg_numbered_buff_fetch,
        Perl_reg_numbered_buff_store,
        Perl_reg_numbered_buff_length,
        Perl_reg_named_buff,
        Perl_reg_named_buff_iter,
        Perl_reg_qr_package,
#if defined(USE_ITHREADS)
        Perl_regdupe_internal,
#endif
        my_re_op_compile,
};

#define newSVbool_(x) newSViv((x) ? 1 : 0)

MODULE = re	PACKAGE = re

void
install()
    PPCODE:
        PL_colorset = 0;	/* Allow reinspection of ENV. */
        /* PL_debug |= DEBUG_r_FLAG; */
	XPUSHs(sv_2mortal(newSViv(PTR2IV(&my_reg_engine))));

void
regmust(sv)
    SV * sv
PROTOTYPE: $
PREINIT:
    REGEXP *re;
PPCODE:
{
    if ((re = SvRX(sv)) /* assign deliberate */
       /* only for re engines we know about */
       && (   RX_ENGINE(re) == &my_reg_engine
           || RX_ENGINE(re) == &wild_reg_engine
           || RX_ENGINE(re) == &PL_core_reg_engine))
    {
        SV *an = &PL_sv_no;
        SV *fl = &PL_sv_no;
        if (RX_ANCHORED_SUBSTR(re)) {
            an = sv_2mortal(newSVsv(RX_ANCHORED_SUBSTR(re)));
        } else if (RX_ANCHORED_UTF8(re)) {
            an = sv_2mortal(newSVsv(RX_ANCHORED_UTF8(re)));
        }
        if (RX_FLOAT_SUBSTR(re)) {
            fl = sv_2mortal(newSVsv(RX_FLOAT_SUBSTR(re)));
        } else if (RX_FLOAT_UTF8(re)) {
            fl = sv_2mortal(newSVsv(RX_FLOAT_UTF8(re)));
        }
        EXTEND(SP, 2);
        PUSHs(an);
        PUSHs(fl);
        XSRETURN(2);
    }
    XSRETURN_UNDEF;
}

SV *
optimization(sv)
    SV * sv
PROTOTYPE: $
PREINIT:
    REGEXP *re;
    regexp *r;
    struct reg_substr_datum * data;
    HV *hv;
CODE:
{
    re = SvRX(sv);
    if (!re) {
        XSRETURN_UNDEF;
    }

    /* only for re engines we know about */
    if (   RX_ENGINE(re) != &my_reg_engine
        && RX_ENGINE(re) != &wild_reg_engine
        && RX_ENGINE(re) != &PL_core_reg_engine)
    {
        XSRETURN_UNDEF;
    }

    if (!PL_colorset) {
        reginitcolors();
    }

    r = ReANY(re);
    hv = newHV();

    hv_stores(hv, "minlen", newSViv(r->minlen));
    hv_stores(hv, "minlenret", newSViv(r->minlenret));
    hv_stores(hv, "gofs", newSViv(r->gofs));

    data = &r->substrs->data[0];
    hv_stores(hv, "anchored", data->substr
            ? newSVsv(data->substr) : &PL_sv_undef);
    hv_stores(hv, "anchored utf8", data->utf8_substr
            ? newSVsv(data->utf8_substr) : &PL_sv_undef);
    hv_stores(hv, "anchored min offset", newSViv(data->min_offset));
    hv_stores(hv, "anchored max offset", newSViv(data->max_offset));
    hv_stores(hv, "anchored end shift", newSViv(data->end_shift));

    data = &r->substrs->data[1];
    hv_stores(hv, "floating", data->substr
            ? newSVsv(data->substr) : &PL_sv_undef);
    hv_stores(hv, "floating utf8", data->utf8_substr
            ? newSVsv(data->utf8_substr) : &PL_sv_undef);
    hv_stores(hv, "floating min offset", newSViv(data->min_offset));
    hv_stores(hv, "floating max offset", newSViv(data->max_offset));
    hv_stores(hv, "floating end shift", newSViv(data->end_shift));

    hv_stores(hv, "checking", newSVpv(
        (!r->check_substr && !r->check_utf8)
            ? "none"
        : (    r->check_substr == r->substrs->data[1].substr
            && r->check_utf8   == r->substrs->data[1].utf8_substr
        )
            ? "floating"
        : "anchored"
    , 0));

    hv_stores(hv, "noscan", newSVbool_(r->intflags & PREGf_NOSCAN));
    hv_stores(hv, "isall", newSVbool_(r->extflags & RXf_CHECK_ALL));
    hv_stores(hv, "anchor SBOL", newSVbool_(r->intflags & PREGf_ANCH_SBOL));
    hv_stores(hv, "anchor MBOL", newSVbool_(r->intflags & PREGf_ANCH_MBOL));
    hv_stores(hv, "anchor GPOS", newSVbool_(r->intflags & PREGf_ANCH_GPOS));
    hv_stores(hv, "skip", newSVbool_(r->intflags & PREGf_SKIP));
    hv_stores(hv, "implicit", newSVbool_(r->intflags & PREGf_IMPLICIT));

    {
        RXi_GET_DECL(r, ri);
        if (ri->regstclass) {
            SV* sv = newSV(0);
            /* not Perl_regprop, we must have the DEBUGGING version */
            my_regprop(aTHX_ r, sv, ri->regstclass, NULL, NULL);
            hv_stores(hv, "stclass", sv);
        } else {
            hv_stores(hv, "stclass", &PL_sv_undef);
        }
    }

    RETVAL = newRV_noinc((SV *)hv);
}
OUTPUT:
    RETVAL

#
# ex: set ts=8 sts=4 sw=4 et:
#
