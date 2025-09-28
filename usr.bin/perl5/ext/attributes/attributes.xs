/*    xsutils.c
 *
 *    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008
 *    by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * 'Perilous to us all are the devices of an art deeper than we possess
 *  ourselves.'                                            --Gandalf
 *
 *     [p.597 of _The Lord of the Rings_, III/xi: "The Palantír"]
 */

#define PERL_EXT

#define PERL_NO_GET_CONTEXT

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

/*
 * Contributed by Spider Boardman (spider.boardman@orb.nashua.nh.us).
 */

static int
modify_SV_attributes(pTHX_ SV *sv, SV **retlist, SV **attrlist, int numattrs)
{
    SV *attr;
    int nret;

    for (nret = 0 ; numattrs && (attr = *attrlist++); numattrs--) {
	STRLEN len;
	const char *name = SvPV_const(attr, len);
	const bool negated = (*name == '-');

	if (negated) {
	    name++;
	    len--;
	}
	switch (SvTYPE(sv)) {
	case SVt_PVCV:
	    switch ((int)len) {
	    case 5:
		if (memEQs(name, 5, "const")) {
		    if (negated)
			CvANONCONST_off(sv);
		    else {
			const bool warn = (!CvANON(sv) || CvCLONED(sv))
				       && !CvANONCONST(sv);
			CvANONCONST_on(sv);
			if (warn)
			    break;
		    }
		    continue;
		}
		break;
	    case 6:
		switch (name[3]) {
		case 'l':
		    if (memEQs(name, 6, "lvalue")) {
			bool warn =
			    !CvISXSUB(MUTABLE_CV(sv))
			 && CvROOT(MUTABLE_CV(sv))
			 && cBOOL(CvLVALUE(MUTABLE_CV(sv))) == negated;
			if (negated)
			    CvFLAGS(MUTABLE_CV(sv)) &= ~CVf_LVALUE;
			else
			    CvFLAGS(MUTABLE_CV(sv)) |= CVf_LVALUE;
			if (warn) break;
			continue;
		    }
		    break;
		case 'h':
		    if (memEQs(name, 6, "method")) {
			if (negated)
			    CvFLAGS(MUTABLE_CV(sv)) &= ~CVf_NOWARN_AMBIGUOUS;
			else
			    CvFLAGS(MUTABLE_CV(sv)) |= CVf_NOWARN_AMBIGUOUS;
			continue;
		    }
		    break;
		}
		break;
	    default:
		if (memBEGINPs(name, len, "prototype(")) {
                    const STRLEN proto_len = sizeof("prototype(") - 1;
		    SV * proto = newSVpvn(name + proto_len, len - proto_len - 1);
		    HEK *const hek = CvNAME_HEK((CV *)sv);
		    SV *subname;
		    if (name[len-1] != ')')
			Perl_croak(aTHX_ "Unterminated attribute parameter in attribute list");
		    if (hek)
			subname = sv_2mortal(newSVhek(hek));
		    else
			subname=(SV *)CvGV((const CV *)sv);
		    if (ckWARN(WARN_ILLEGALPROTO))
			Perl_validate_proto(aTHX_ subname, proto, TRUE, 0);
		    Perl_cv_ckproto_len_flags(aTHX_ (const CV *)sv,
		                                    (const GV *)subname,
		                                    name+10,
		                                    len-11,
		                                    SvUTF8(attr));
		    sv_setpvn(MUTABLE_SV(sv), name+10, len-11);
		    if (SvUTF8(attr)) SvUTF8_on(MUTABLE_SV(sv));
		    continue;
		}
		break;
	    }
	    break;
	default:
	    if (memEQs(name, len, "shared")) {
			if (negated)
			    Perl_croak(aTHX_ "A variable may not be unshared");
			SvSHARE(sv);
                        continue;
	    }
	    break;
	}
	/* anything recognized had a 'continue' above */
	*retlist++ = attr;
	nret++;
    }

    return nret;
}

MODULE = attributes		PACKAGE = attributes

void
_modify_attrs(...)
  PREINIT:
    SV *rv, *sv;
  PPCODE:

    if (items < 1) {
usage:
	croak_xs_usage(cv, "@attributes");
    }

    rv = ST(0);
    if (!(SvOK(rv) && SvROK(rv)))
	goto usage;
    sv = SvRV(rv);
    if (items > 1)
	XSRETURN(modify_SV_attributes(aTHX_ sv, &ST(0), &ST(1), items-1));

    XSRETURN(0);

void
_fetch_attrs(...)
  PROTOTYPE: $
  PREINIT:
    SV *rv, *sv;
    cv_flags_t cvflags;
  PPCODE:
    if (items != 1) {
usage:
	croak_xs_usage(cv, "$reference");
    }

    rv = ST(0);
    if (!(SvOK(rv) && SvROK(rv)))
	goto usage;
    sv = SvRV(rv);

    switch (SvTYPE(sv)) {
    case SVt_PVCV:
	cvflags = CvFLAGS((const CV *)sv);
	if (cvflags & CVf_LVALUE)
	    XPUSHs(newSVpvs_flags("lvalue", SVs_TEMP));
	if (cvflags & CVf_NOWARN_AMBIGUOUS)
	    XPUSHs(newSVpvs_flags("method", SVs_TEMP));
	break;
    default:
	break;
    }

    PUTBACK;

void
_guess_stash(...)
  PROTOTYPE: $
  PREINIT:
    SV *rv, *sv;
    dXSTARG;
  PPCODE:
    if (items != 1) {
usage:
	croak_xs_usage(cv, "$reference");
    }

    rv = ST(0);
    ST(0) = TARG;
    if (!(SvOK(rv) && SvROK(rv)))
	goto usage;
    sv = SvRV(rv);

    if (SvOBJECT(sv))
	Perl_sv_sethek(aTHX_ TARG, HvNAME_HEK(SvSTASH(sv)));
#if 0	/* this was probably a bad idea */
    else if (SvPADMY(sv))
	sv_setbool(TARG, FALSE);	/* unblessed lexical */
#endif
    else {
	const HV *stash = NULL;
	switch (SvTYPE(sv)) {
	case SVt_PVCV:
	    if (CvGV(sv) && isGV(CvGV(sv)) && GvSTASH(CvGV(sv)))
		stash = GvSTASH(CvGV(sv));
	    else if (/* !CvANON(sv) && */ CvSTASH(sv))
		stash = CvSTASH(sv);
	    break;
	case SVt_PVGV:
	    if (isGV_with_GP(sv) && GvGP(sv) && GvESTASH(MUTABLE_GV(sv)))
		stash = GvESTASH(MUTABLE_GV(sv));
	    break;
	default:
	    break;
	}
	if (stash)
	    Perl_sv_sethek(aTHX_ TARG, HvNAME_HEK(stash));
    }

    SvSETMAGIC(TARG);
    XSRETURN(1);

void
reftype(...)
  PROTOTYPE: $
  PREINIT:
    SV *rv, *sv;
    dXSTARG;
  PPCODE:
    if (items != 1) {
usage:
	croak_xs_usage(cv, "$reference");
    }

    rv = ST(0);
    ST(0) = TARG;
    SvGETMAGIC(rv);
    if (!(SvOK(rv) && SvROK(rv)))
	goto usage;
    sv = SvRV(rv);
    sv_setpv(TARG, sv_reftype(sv, 0));
    SvSETMAGIC(TARG);

    XSRETURN(1);
/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
