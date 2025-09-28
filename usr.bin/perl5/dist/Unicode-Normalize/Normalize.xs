
#define PERL_NO_GET_CONTEXT /* we want efficiency */

/* private functions which need pTHX_ and aTHX_
    pv_cat_decompHangul
    sv_2pvunicode
    pv_utf8_decompose
    pv_utf8_reorder
    pv_utf8_compose
*/

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#define NEED_utf8_to_uvchr_buf
#include "ppport.h"

/* These 5 files are prepared by mkheader */
#include "unfcmb.h"
#include "unfcan.h"
#include "unfcpt.h"
#include "unfcmp.h"
#include "unfexc.h"

/* The generated normalization tables since v5.20 are in native character set
 * terms.  Prior to that, they were in Unicode terms.  So we use 'uvchr' for
 * later perls, and redefine that to be 'uvuni' for earlier ones */
#if PERL_VERSION_LT(5,20,0)
#   undef uvchr_to_utf8
#   ifdef uvuni_to_utf8
#       define uvchr_to_utf8   uvuni_to_utf8
#   else /* Perl 5.6.1 */
#       define uvchr_to_utf8   uv_to_utf8
#   endif
#endif

/* check if the string buffer is enough before uvchr_to_utf8(). */
/* dstart, d, and dlen should be defined outside before. */
#define Renew_d_if_not_enough_to(need)	STRLEN curlen = d - dstart;	\
		if (dlen < curlen + (need)) {	\
		    dlen += (need);		\
		    Renew(dstart, dlen+1, U8);	\
		    d = dstart + curlen;	\
		}

/* if utf8_to_uvchr_buf() sets retlen to 0 (if broken?) */
#define ErrRetlenIsZero "panic (Unicode::Normalize %s): zero-length character"

/* utf8_hop() hops back before start. Maybe broken UTF-8 */
#define ErrHopBeforeStart "panic (Unicode::Normalize): hopping before start"

/* At present, char > 0x10ffff are unaffected without complaint, right? */
#define VALID_UTF_MAX    (0x10ffff)
#define OVER_UTF_MAX(uv) (VALID_UTF_MAX < (uv))

/* size of array for combining characters */
/* enough as an initial value? */
#define CC_SEQ_SIZE (10)
#define CC_SEQ_STEP  (5)

/* HANGUL begin */
#define Hangul_SBase  0xAC00
#define Hangul_SFinal 0xD7A3
#define Hangul_SCount  11172

#define Hangul_NCount    588

#define Hangul_LBase  0x1100
#define Hangul_LFinal 0x1112
#define Hangul_LCount     19

#define Hangul_VBase  0x1161
#define Hangul_VFinal 0x1175
#define Hangul_VCount     21

#define Hangul_TBase  0x11A7
#define Hangul_TFinal 0x11C2
#define Hangul_TCount     28

#define Hangul_IsS(u)  ((Hangul_SBase <= (u)) && ((u) <= Hangul_SFinal))
#define Hangul_IsN(u)  (((u) - Hangul_SBase) % Hangul_TCount == 0)
#define Hangul_IsLV(u) (Hangul_IsS(u) && Hangul_IsN(u))
#define Hangul_IsL(u)  ((Hangul_LBase <= (u)) && ((u) <= Hangul_LFinal))
#define Hangul_IsV(u)  ((Hangul_VBase <= (u)) && ((u) <= Hangul_VFinal))
#define Hangul_IsT(u)  ((Hangul_TBase  < (u)) && ((u) <= Hangul_TFinal))
/* HANGUL end */

/* this is used for canonical ordering of combining characters (c.c.). */
typedef struct {
    U8 cc;	/* combining class */
    UV uv;	/* codepoint */
    STRLEN pos; /* position */
} UNF_cc;

static int compare_cc(const void *a, const void *b)
{
    int ret_cc;
    ret_cc = ((UNF_cc*) a)->cc - ((UNF_cc*) b)->cc;
    if (ret_cc)
	return ret_cc;

    return ( ((UNF_cc*) a)->pos > ((UNF_cc*) b)->pos )
	 - ( ((UNF_cc*) a)->pos < ((UNF_cc*) b)->pos );
}

static U8* dec_canonical(UV uv)
{
    U8 ***plane, **row;
    if (OVER_UTF_MAX(uv))
	return NULL;
    plane = (U8***)UNF_canon[uv >> 16];
    if (! plane)
	return NULL;
    row = plane[(U8) (uv >> 8)];
    return row ? row[(U8) uv] : NULL;
}

static U8* dec_compat(UV uv)
{
    U8 ***plane, **row;
    if (OVER_UTF_MAX(uv))
	return NULL;
    plane = (U8***)UNF_compat[uv >> 16];
    if (! plane)
	return NULL;
    row = plane[(U8) (uv >> 8)];
    return row ? row[(U8) uv] : NULL;
}

static UV composite_uv(UV uv, UV uv2)
{
    UNF_complist ***plane, **row, *cell, *i;

    if (!uv2 || OVER_UTF_MAX(uv) || OVER_UTF_MAX(uv2))
	return 0;

    if (Hangul_IsL(uv) && Hangul_IsV(uv2)) {
	UV lindex = uv  - Hangul_LBase;
	UV vindex = uv2 - Hangul_VBase;
	return(Hangul_SBase + (lindex * Hangul_VCount + vindex) *
	       Hangul_TCount);
    }
    if (Hangul_IsLV(uv) && Hangul_IsT(uv2)) {
	UV tindex = uv2 - Hangul_TBase;
	return(uv + tindex);
    }
    plane = UNF_compos[uv >> 16];
    if (! plane)
	return 0;
    row = plane[(U8) (uv >> 8)];
    if (! row)
	return 0;
    cell = row[(U8) uv];
    if (! cell)
	return 0;
    for (i = cell; i->nextchar; i++) {
	if (uv2 == i->nextchar)
	    return i->composite;
    }
    return 0;
}

static U8 getCombinClass(UV uv)
{
    U8 **plane, *row;
    if (OVER_UTF_MAX(uv))
	return 0;
    plane = (U8**)UNF_combin[uv >> 16];
    if (! plane)
	return 0;
    row = plane[(U8) (uv >> 8)];
    return row ? row[(U8) uv] : 0;
}

static U8* pv_cat_decompHangul(pTHX_ U8* d, UV uv)
{
    UV sindex =  uv - Hangul_SBase;
    UV lindex =  sindex / Hangul_NCount;
    UV vindex = (sindex % Hangul_NCount) / Hangul_TCount;
    UV tindex =  sindex % Hangul_TCount;

    if (! Hangul_IsS(uv))
	return d;

    d = uvchr_to_utf8(d, (lindex + Hangul_LBase));
    d = uvchr_to_utf8(d, (vindex + Hangul_VBase));
    if (tindex)
	d = uvchr_to_utf8(d, (tindex + Hangul_TBase));
    return d;
}

static char* sv_2pvunicode(pTHX_ SV *sv, STRLEN *lp)
{
    char *s;
    STRLEN len;
    s = SvPV(sv,len);
    if (!SvUTF8(sv)) {
	SV* tmpsv = sv_2mortal(newSVpvn(s, len));
	if (!SvPOK(tmpsv))
	    s = SvPV_force(tmpsv,len);
	sv_utf8_upgrade(tmpsv);
	s = SvPV(tmpsv,len);
    }
    if (lp)
	*lp = len;
    return s;
}

static
U8* pv_utf8_decompose(pTHX_ U8* s, STRLEN slen, U8** dp, STRLEN dlen, bool iscompat)
{
    U8* p = s;
    U8* e = s + slen;
    U8* dstart = *dp;
    U8* d = dstart;

    while (p < e) {
	STRLEN retlen;
	UV uv = utf8_to_uvchr_buf(p, e, &retlen);
	if (!retlen)
	    croak(ErrRetlenIsZero, "decompose");
	p += retlen;

	if (Hangul_IsS(uv)) {
	    Renew_d_if_not_enough_to(UTF8_MAXLEN * 3)
	    d = pv_cat_decompHangul(aTHX_ d, uv);
	}
	else {
	    U8* r = iscompat ? dec_compat(uv) : dec_canonical(uv);

	    if (r) {
		STRLEN len = (STRLEN)strlen((char *)r);
		Renew_d_if_not_enough_to(len)
		while (len--)
		    *d++ = *r++;
	    }
	    else {
		Renew_d_if_not_enough_to(UTF8_MAXLEN)
		d = uvchr_to_utf8(d, uv);
	    }
	}
    }
    *dp = dstart;
    return d;
}

static
U8* pv_utf8_reorder(pTHX_ U8* s, STRLEN slen, U8** dp, STRLEN dlen)
{
    U8* p = s;
    U8* e = s + slen;
    U8* dstart = *dp;
    U8* d = dstart;

    UNF_cc  seq_ary[CC_SEQ_SIZE];
    UNF_cc* seq_ptr = seq_ary; /* use array at the beginning */
    UNF_cc* seq_ext = NULL; /* extend if need */
    STRLEN seq_max = CC_SEQ_SIZE;
    STRLEN cc_pos = 0;

    while (p < e) {
	U8 curCC;
	STRLEN retlen;
	UV uv = utf8_to_uvchr_buf(p, e, &retlen);
	if (!retlen)
	    croak(ErrRetlenIsZero, "reorder");
	p += retlen;

	curCC = getCombinClass(uv);

	if (curCC != 0) {
	    if (seq_max < cc_pos + 1) { /* extend if need */
		seq_max = cc_pos + CC_SEQ_STEP; /* new size */
		if (CC_SEQ_SIZE == cc_pos) { /* seq_ary full */
		    STRLEN i;
		    New(0, seq_ext, seq_max, UNF_cc);
		    for (i = 0; i < cc_pos; i++)
			seq_ext[i] = seq_ary[i];
		}
		else {
		    Renew(seq_ext, seq_max, UNF_cc);
		}
		seq_ptr = seq_ext; /* use seq_ext from now */
	    }

	    seq_ptr[cc_pos].cc  = curCC;
	    seq_ptr[cc_pos].uv  = uv;
	    seq_ptr[cc_pos].pos = cc_pos;
	    ++cc_pos;

	    if (p < e)
		continue;
	}

	/* output */
	if (cc_pos) {
	    STRLEN i;

	    if (cc_pos > 1) /* reordered if there are two c.c.'s */
		qsort((void*)seq_ptr, cc_pos, sizeof(UNF_cc), compare_cc);

	    for (i = 0; i < cc_pos; i++) {
		Renew_d_if_not_enough_to(UTF8_MAXLEN)
		d = uvchr_to_utf8(d, seq_ptr[i].uv);
	    }
	    cc_pos = 0;
	}

	if (curCC == 0) {
	    Renew_d_if_not_enough_to(UTF8_MAXLEN)
	    d = uvchr_to_utf8(d, uv);
	}
    }
    if (seq_ext)
	Safefree(seq_ext);
    *dp = dstart;
    return d;
}

static
U8* pv_utf8_compose(pTHX_ U8* s, STRLEN slen, U8** dp, STRLEN dlen, bool iscontig)
{
    U8* p = s;
    U8* e = s + slen;
    U8* dstart = *dp;
    U8* d = dstart;

    UV uvS = 0; /* code point of the starter */
    bool valid_uvS = FALSE; /* if FALSE, uvS isn't initialized yet */
    U8 preCC = 0;

    UV  seq_ary[CC_SEQ_SIZE];
    UV* seq_ptr = seq_ary; /* use array at the beginning */
    UV* seq_ext = NULL; /* extend if need */
    STRLEN seq_max = CC_SEQ_SIZE;
    STRLEN cc_pos = 0;

    while (p < e) {
	U8 curCC;
	STRLEN retlen;
	UV uv = utf8_to_uvchr_buf(p, e, &retlen);
	if (!retlen)
	    croak(ErrRetlenIsZero, "compose");
	p += retlen;

	curCC = getCombinClass(uv);

	if (!valid_uvS) {
	    if (curCC == 0) {
		uvS = uv; /* the first Starter is found */
		valid_uvS = TRUE;
		if (p < e)
		    continue;
	    }
	    else {
		Renew_d_if_not_enough_to(UTF8_MAXLEN)
		d = uvchr_to_utf8(d, uv);
		continue;
	    }
	}
	else {
	    bool composed;

	    /* blocked */
	    if ((iscontig && cc_pos) || /* discontiguous combination */
		 (curCC != 0 && preCC == curCC) || /* blocked by same CC */
		 (preCC > curCC)) /* blocked by higher CC: revised D2 */
		composed = FALSE;

	    /* not blocked:
		 iscontig && cc_pos == 0      -- contiguous combination
		 curCC == 0 && preCC == 0     -- starter + starter
		 curCC != 0 && preCC < curCC  -- lower CC */
	    else {
		/* try composition */
		UV uvComp = composite_uv(uvS, uv);

		if (uvComp && !isExclusion(uvComp))  {
		    uvS = uvComp;
		    composed = TRUE;

		    /* preCC should not be changed to curCC */
		    /* e.g. 1E14 = 0045 0304 0300 where CC(0304) == CC(0300) */
		    if (p < e)
			continue;
		}
		else
		    composed = FALSE;
	    }

	    if (!composed) {
		preCC = curCC;
		if (curCC != 0 || !(p < e)) {
		    if (seq_max < cc_pos + 1) { /* extend if need */
			seq_max = cc_pos + CC_SEQ_STEP; /* new size */
			if (CC_SEQ_SIZE == cc_pos) { /* seq_ary full */
			    New(0, seq_ext, seq_max, UV);
			    Copy(seq_ary, seq_ext, cc_pos, UV);
			}
			else {
			    Renew(seq_ext, seq_max, UV);
			}
			seq_ptr = seq_ext; /* use seq_ext from now */
		    }
		    seq_ptr[cc_pos] = uv;
		    ++cc_pos;
		}
		if (curCC != 0 && p < e)
		    continue;
	    }
	}

	/* output */
	{
	    Renew_d_if_not_enough_to(UTF8_MAXLEN)
	    d = uvchr_to_utf8(d, uvS); /* starter (composed or not) */
	}

	if (cc_pos) {
	    STRLEN i;

	    for (i = 0; i < cc_pos; i++) {
		Renew_d_if_not_enough_to(UTF8_MAXLEN)
		d = uvchr_to_utf8(d, seq_ptr[i]);
	    }
	    cc_pos = 0;
	}

	uvS = uv;
    }
    if (seq_ext)
	Safefree(seq_ext);
    *dp = dstart;
    return d;
}

MODULE = Unicode::Normalize	PACKAGE = Unicode::Normalize

SV*
decompose(src, compat = &PL_sv_no)
    SV * src
    SV * compat
  PROTOTYPE: $;$
  PREINIT:
    SV* dst;
    U8 *s, *d, *dend;
    STRLEN slen, dlen;
  CODE:
    s = (U8*)sv_2pvunicode(aTHX_ src,&slen);
    dst = newSVpvn("", 0);
    dlen = slen;
    New(0, d, dlen+1, U8);
    dend = pv_utf8_decompose(aTHX_ s, slen, &d, dlen, (bool)SvTRUE(compat));
    sv_setpvn(dst, (char *)d, dend - d);
    SvUTF8_on(dst);
    Safefree(d);
    RETVAL = dst;
  OUTPUT:
    RETVAL


SV*
reorder(src)
    SV * src
  PROTOTYPE: $
  PREINIT:
    SV* dst;
    U8 *s, *d, *dend;
    STRLEN slen, dlen;
  CODE:
    s = (U8*)sv_2pvunicode(aTHX_ src,&slen);
    dst = newSVpvn("", 0);
    dlen = slen;
    New(0, d, dlen+1, U8);
    dend = pv_utf8_reorder(aTHX_ s, slen, &d, dlen);
    sv_setpvn(dst, (char *)d, dend - d);
    SvUTF8_on(dst);
    Safefree(d);
    RETVAL = dst;
  OUTPUT:
    RETVAL


SV*
compose(src)
    SV * src
  PROTOTYPE: $
  ALIAS:
    composeContiguous = 1
  PREINIT:
    SV* dst;
    U8 *s, *d, *dend;
    STRLEN slen, dlen;
  CODE:
    s = (U8*)sv_2pvunicode(aTHX_ src,&slen);
    dst = newSVpvn("", 0);
    dlen = slen;
    New(0, d, dlen+1, U8);
    dend = pv_utf8_compose(aTHX_ s, slen, &d, dlen, (bool)ix);
    sv_setpvn(dst, (char *)d, dend - d);
    SvUTF8_on(dst);
    Safefree(d);
    RETVAL = dst;
  OUTPUT:
    RETVAL


SV*
NFD(src)
    SV * src
  PROTOTYPE: $
  ALIAS:
    NFKD = 1
  PREINIT:
    SV *dst;
    U8 *s, *t, *tend, *d, *dend;
    STRLEN slen, tlen, dlen;
  CODE:
    s = (U8*)sv_2pvunicode(aTHX_ src,&slen);

    /* decompose */
    tlen = slen;
    New(0, t, tlen+1, U8);
    tend = pv_utf8_decompose(aTHX_ s, slen, &t, tlen, (bool)(ix==1));
    *tend = '\0';
    tlen = tend - t; /* no longer know real size of t */

    /* reorder */
    dlen = tlen;
    New(0, d, dlen+1, U8);
    dend = pv_utf8_reorder(aTHX_ t, tlen, &d, dlen);
    *dend = '\0';
    dlen = dend - d; /* no longer know real size of d */

    /* return */
    dst = newSVpvn("", 0);
    sv_setpvn(dst, (char *)d, dlen);
    SvUTF8_on(dst);

    Safefree(t);
    Safefree(d);
    RETVAL = dst;
  OUTPUT:
    RETVAL


SV*
NFC(src)
    SV * src
  PROTOTYPE: $
  ALIAS:
    NFKC = 1
    FCC  = 2
  PREINIT:
    SV *dst;
    U8 *s, *t, *tend, *u, *uend, *d, *dend;
    STRLEN slen, tlen, ulen, dlen;
  CODE:
    s = (U8*)sv_2pvunicode(aTHX_ src,&slen);

    /* decompose */
    tlen = slen;
    New(0, t, tlen+1, U8);
    tend = pv_utf8_decompose(aTHX_ s, slen, &t, tlen, (bool)(ix==1));
    *tend = '\0';
    tlen = tend - t; /* no longer know real size of t */

    /* reorder */
    ulen = tlen;
    New(0, u, ulen+1, U8);
    uend = pv_utf8_reorder(aTHX_ t, tlen, &u, ulen);
    *uend = '\0';
    ulen = uend - u; /* no longer know real size of u */

    /* compose */
    dlen = ulen;
    New(0, d, dlen+1, U8);
    dend = pv_utf8_compose(aTHX_ u, ulen, &d, dlen, (bool)(ix==2));
    *dend = '\0';
    dlen = dend - d; /* no longer know real size of d */

    /* return */
    dst = newSVpvn("", 0);
    sv_setpvn(dst, (char *)d, dlen);
    SvUTF8_on(dst);

    Safefree(t);
    Safefree(u);
    Safefree(d);
    RETVAL = dst;
  OUTPUT:
    RETVAL


SV*
checkNFD(src)
    SV * src
  PROTOTYPE: $
  ALIAS:
    checkNFKD = 1
  PREINIT:
    STRLEN srclen, retlen;
    U8 *s, *e, *p, curCC, preCC;
    bool result = TRUE;
  CODE:
    s = (U8*)sv_2pvunicode(aTHX_ src,&srclen);
    e = s + srclen;

    preCC = 0;
    for (p = s; p < e; p += retlen) {
	UV uv = utf8_to_uvchr_buf(p, e, &retlen);
	if (!retlen)
	    croak(ErrRetlenIsZero, "checkNFD or -NFKD");

	curCC = getCombinClass(uv);
	if (preCC > curCC && curCC != 0) { /* canonical ordering violated */
	    result = FALSE;
	    break;
	}
	if (Hangul_IsS(uv) || (ix ? dec_compat(uv) : dec_canonical(uv))) {
	    result = FALSE;
	    break;
	}
	preCC = curCC;
    }
    RETVAL = boolSV(result);
  OUTPUT:
    RETVAL


SV*
checkNFC(src)
    SV * src
  PROTOTYPE: $
  ALIAS:
    checkNFKC = 1
  PREINIT:
    STRLEN srclen, retlen;
    U8 *s, *e, *p, curCC, preCC;
    bool result = TRUE;
    bool isMAYBE = FALSE;
  CODE:
    s = (U8*)sv_2pvunicode(aTHX_ src,&srclen);
    e = s + srclen;

    preCC = 0;
    for (p = s; p < e; p += retlen) {
	UV uv = utf8_to_uvchr_buf(p, e, &retlen);
	if (!retlen)
	    croak(ErrRetlenIsZero, "checkNFC or -NFKC");

	curCC = getCombinClass(uv);
	if (preCC > curCC && curCC != 0) { /* canonical ordering violated */
	    result = FALSE;
	    break;
	}

	/* get NFC/NFKC property */
	if (Hangul_IsS(uv)) /* Hangul syllables are canonical composites */
	    ; /* YES */
	else if (isExclusion(uv) || isSingleton(uv) || isNonStDecomp(uv)) {
	    result = FALSE;
	    break;
	}
	else if (isComp2nd(uv))
	    isMAYBE = TRUE;
	else if (ix) {
	    char *canon, *compat;
	  /* NFKC_NO when having compatibility mapping. */
	    canon  = (char *) dec_canonical(uv);
	    compat = (char *) dec_compat(uv);
	    if (compat && !(canon && strEQ(canon, compat))) {
		result = FALSE;
		break;
	    }
	} /* end of get NFC/NFKC property */

	preCC = curCC;
    }
    if (isMAYBE && result) /* NO precedes MAYBE */
	XSRETURN_UNDEF;
    RETVAL = boolSV(result);
  OUTPUT:
    RETVAL


SV*
checkFCD(src)
    SV * src
  PROTOTYPE: $
  ALIAS:
    checkFCC = 1
  PREINIT:
    STRLEN srclen, retlen;
    U8 *s, *e, *p, curCC, preCC;
    bool result = TRUE;
    bool isMAYBE = FALSE;
  CODE:
    s = (U8*)sv_2pvunicode(aTHX_ src,&srclen);
    e = s + srclen;
    preCC = 0;
    for (p = s; p < e; p += retlen) {
	U8 *sCan;
	UV uvLead;
	STRLEN canlen = 0;
	UV uv = utf8_to_uvchr_buf(p, e, &retlen);
	if (!retlen)
	    croak(ErrRetlenIsZero, "checkFCD or -FCC");

	sCan = (U8*) dec_canonical(uv);

	if (sCan) {
	    STRLEN canret;
	    canlen = (STRLEN)strlen((char *) sCan);
	    uvLead = utf8_to_uvchr_buf(sCan, sCan + canlen, &canret);
	    if (!canret)
		croak(ErrRetlenIsZero, "checkFCD or -FCC");
	}
	else {
	    uvLead = uv;
	}

	curCC = getCombinClass(uvLead);

	if (curCC != 0 && curCC < preCC) { /* canonical ordering violated */
	    result = FALSE;
	    break;
	}

	if (ix) {
	    if (isExclusion(uv) || isSingleton(uv) || isNonStDecomp(uv)) {
		result = FALSE;
		break;
	    }
	    else if (isComp2nd(uv))
		isMAYBE = TRUE;
	}

	if (sCan) {
	    STRLEN canret;
	    UV uvTrail;
	    U8* eCan = sCan + canlen;
	    U8* pCan = utf8_hop(eCan, -1);
	    if (pCan < sCan)
		croak(ErrHopBeforeStart);
	    uvTrail = utf8_to_uvchr_buf(pCan, eCan, &canret);
	    if (!canret)
		croak(ErrRetlenIsZero, "checkFCD or -FCC");
	    preCC = getCombinClass(uvTrail);
	}
	else {
	    preCC = curCC;
	}
    }
    if (isMAYBE && result) /* NO precedes MAYBE */
	XSRETURN_UNDEF;
    RETVAL = boolSV(result);
  OUTPUT:
    RETVAL


U8
getCombinClass(uv)
    UV uv
  PROTOTYPE: $

bool
isExclusion(uv)
    UV uv
  PROTOTYPE: $

bool
isSingleton(uv)
    UV uv
  PROTOTYPE: $

bool
isNonStDecomp(uv)
    UV uv
  PROTOTYPE: $

bool
isComp2nd(uv)
    UV uv
  PROTOTYPE: $
  ALIAS:
    isNFC_MAYBE  = 1
    isNFKC_MAYBE = 2
  INIT:
    PERL_UNUSED_VAR(ix);

SV*
isNFD_NO(uv)
    UV uv
  PROTOTYPE: $
  ALIAS:
    isNFKD_NO = 1
  PREINIT:
    bool result = FALSE;
  CODE:
    if (Hangul_IsS(uv) || (ix ? dec_compat(uv) : dec_canonical(uv)))
	result = TRUE; /* NFD_NO or NFKD_NO */
    RETVAL = boolSV(result);
  OUTPUT:
    RETVAL


SV*
isComp_Ex(uv)
    UV uv
  PROTOTYPE: $
  ALIAS:
    isNFC_NO  = 0
    isNFKC_NO = 1
  PREINIT:
    bool result = FALSE;
  CODE:
    if (isExclusion(uv) || isSingleton(uv) || isNonStDecomp(uv))
	result = TRUE; /* NFC_NO or NFKC_NO */
    else if (ix) {
	char *canon, *compat;
	canon  = (char *) dec_canonical(uv);
	compat = (char *) dec_compat(uv);
	if (compat && (!canon || strNE(canon, compat)))
	    result = TRUE; /* NFC_NO or NFKC_NO */
    }
    RETVAL = boolSV(result);
  OUTPUT:
    RETVAL

SV*
getComposite(uv, uv2)
    UV uv
    UV uv2
  PROTOTYPE: $$
  PREINIT:
    UV composite;
  CODE:
    composite = composite_uv(uv, uv2);
    RETVAL = composite ? newSVuv(composite) : &PL_sv_undef;
  OUTPUT:
    RETVAL



SV*
getCanon(uv)
    UV uv
  PROTOTYPE: $
  ALIAS:
    getCompat = 1
  CODE:
    if (Hangul_IsS(uv)) {
	U8 tmp[3 * UTF8_MAXLEN + 1];
	U8 *t = tmp;
	U8 *e = pv_cat_decompHangul(aTHX_ t, uv);
	RETVAL = newSVpvn((char *)t, e - t);
    } else {
	U8* rstr = ix ? dec_compat(uv) : dec_canonical(uv);
	if (!rstr)
	    XSRETURN_UNDEF;
	RETVAL = newSVpvn((char *)rstr, strlen((char *)rstr));
    }
    SvUTF8_on(RETVAL);
  OUTPUT:
    RETVAL


void
splitOnLastStarter(src)
    SV * src
  PREINIT:
    SV *svp;
    STRLEN srclen;
    U8 *s, *e, *p;
  PPCODE:
    s = (U8*)sv_2pvunicode(aTHX_ src,&srclen);
    e = s + srclen;
    p = e;
    while (s < p) {
	UV uv;
	p = utf8_hop(p, -1);
	if (p < s)
	    croak(ErrHopBeforeStart);
	uv = utf8_to_uvchr_buf(p, e, NULL);
	if (getCombinClass(uv) == 0) /* Last Starter found */
	    break;
    }

    svp = sv_2mortal(newSVpvn((char*)s, p - s));
    SvUTF8_on(svp);
    XPUSHs(svp);

    svp = sv_2mortal(newSVpvn((char*)p, e - p));
    SvUTF8_on(svp);
    XPUSHs(svp);

