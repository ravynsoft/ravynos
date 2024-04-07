/*
 $Id: Encode.xs,v 2.51 2021/10/08 15:29:23 dankogai Exp $
 */

#define PERL_NO_GET_CONTEXT
#define IN_ENCODE_XS
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "encode.h"
#include "def_t.h"

# define PERLIO_MODNAME  "PerlIO::encoding"
# define PERLIO_FILENAME "PerlIO/encoding.pm"

/* set 1 or more to profile.  t/encoding.t dumps core because of
   Perl_warner and PerlIO don't work well */
#define ENCODE_XS_PROFILE 0

/* set 0 to disable floating point to calculate buffer size for
   encode_method().  1 is recommended. 2 restores NI-S original */
#define ENCODE_XS_USEFP   1

#ifndef SvIV_nomg
#define SvIV_nomg SvIV
#endif

#ifndef SvTRUE_nomg
#define SvTRUE_nomg SvTRUE
#endif

#ifndef SVfARG
#define SVfARG(p) ((void*)(p))
#endif

static void
Encode_XSEncoding(pTHX_ encode_t * enc)
{
    dSP;
    HV *stash = gv_stashpv("Encode::XS", TRUE);
    SV *iv    = newSViv(PTR2IV(enc));
    SV *sv    = sv_bless(newRV_noinc(iv),stash);
    int i = 0;
    /* with the SvLEN() == 0 hack, PVX won't be freed. We cast away name's
    constness, in the hope that perl won't mess with it. */
    assert(SvTYPE(iv) >= SVt_PV); assert(SvLEN(iv) == 0);
    SvFLAGS(iv) |= SVp_POK;
    SvPVX(iv) = (char*) enc->name[0];
    PUSHMARK(sp);
    XPUSHs(sv);
    while (enc->name[i]) {
    const char *name = enc->name[i++];
    XPUSHs(sv_2mortal(newSVpvn(name, strlen(name))));
    }
    PUTBACK;
    call_pv("Encode::define_encoding", G_DISCARD);
    SvREFCNT_dec(sv);
}

static void
utf8_safe_downgrade(pTHX_ SV ** src, U8 ** s, STRLEN * slen, bool modify)
{
    if (!modify) {
        SV *tmp = sv_2mortal(newSVpvn((char *)*s, *slen));
        SvUTF8_on(tmp);
        if (SvTAINTED(*src))
            SvTAINTED_on(tmp);
        *src = tmp;
        *s = (U8 *)SvPVX(*src);
    }
    if (*slen) {
        if (!utf8_to_bytes(*s, slen))
            croak("Wide character");
        SvCUR_set(*src, *slen);
    }
    SvUTF8_off(*src);
}

static void
utf8_safe_upgrade(pTHX_ SV ** src, U8 ** s, STRLEN * slen, bool modify)
{
    if (!modify) {
        SV *tmp = sv_2mortal(newSVpvn((char *)*s, *slen));
        if (SvTAINTED(*src))
            SvTAINTED_on(tmp);
        *src = tmp;
    }
    sv_utf8_upgrade_nomg(*src);
    *s = (U8 *)SvPV_nomg(*src, *slen);
}

#define ERR_ENCODE_NOMAP "\"\\x{%04" UVxf "}\" does not map to %s"
#define ERR_DECODE_NOMAP "%s \"\\x%02" UVXf "\" does not map to Unicode"
#define ERR_DECODE_STR_NOMAP "%s \"%s\" does not map to Unicode"

static SV *
do_fallback_cb(pTHX_ UV ch, SV *fallback_cb)
{
    dSP;
    int argc;
    SV *retval;
    ENTER;
    SAVETMPS;
    PUSHMARK(sp);
    XPUSHs(sv_2mortal(newSVuv(ch)));
    PUTBACK;
    argc = call_sv(fallback_cb, G_SCALAR);
    SPAGAIN;
    if (argc != 1){
	croak("fallback sub must return scalar!");
    }
    retval = POPs;
    SvREFCNT_inc(retval);
    PUTBACK;
    FREETMPS;
    LEAVE;
    return retval;
}

static SV *
do_bytes_fallback_cb(pTHX_ U8 *s, STRLEN slen, SV *fallback_cb)
{
    dSP;
    int argc;
    STRLEN i;
    SV *retval;
    ENTER;
    SAVETMPS;
    PUSHMARK(sp);
    for (i=0; i<slen; ++i)
        XPUSHs(sv_2mortal(newSVuv(s[i])));
    PUTBACK;
    argc = call_sv(fallback_cb, G_SCALAR);
    SPAGAIN;
    if (argc != 1){
        croak("fallback sub must return scalar!");
    }
    retval = POPs;
    SvREFCNT_inc(retval);
    PUTBACK;
    FREETMPS;
    LEAVE;
    return retval;
}

static SV *
encode_method(pTHX_ const encode_t * enc, const encpage_t * dir, SV * src, U8 * s, STRLEN slen,
	      IV check, STRLEN * offset, SV * term, int * retcode, 
	      SV *fallback_cb)
{
    U8 *sorig    = s;
    STRLEN tlen  = slen;
    STRLEN ddone = 0;
    STRLEN sdone = 0;
    /* We allocate slen+1.
       PerlIO dumps core if this value is smaller than this. */
    SV *dst = sv_2mortal(newSV(slen+1));
    U8 *d = (U8 *)SvPVX(dst);
    STRLEN dlen = SvLEN(dst)-1;
    int code = 0;
    STRLEN trmlen = 0;
    U8 *trm = term ? (U8*) SvPV(term, trmlen) : NULL;

    if (SvTAINTED(src)) SvTAINTED_on(dst); /* propagate taintedness */

    if (offset) {
      s += *offset;
      if (slen > *offset){ /* safeguard against slen overflow */
          slen -= *offset;
      }else{
          slen = 0;
      }
      tlen = slen;
    }

    if (slen == 0){
        SvCUR_set(dst, 0);
        SvPOK_only(dst);
        goto ENCODE_END;
    }

    while( (code = do_encode(dir, s, &slen, d, dlen, &dlen, !check,
                 trm, trmlen)) )
    {
        SvCUR_set(dst, dlen+ddone);
        SvPOK_only(dst);

        if (code == ENCODE_FALLBACK || code == ENCODE_PARTIAL ||
            code == ENCODE_FOUND_TERM) {
            break;
        }
        switch (code) {
        case ENCODE_NOSPACE:
        {
            STRLEN more = 0; /* make sure you initialize! */
            STRLEN sleft;
            sdone += slen;
            ddone += dlen;
            sleft = tlen - sdone;
#if ENCODE_XS_PROFILE >= 2
            Perl_warn(aTHX_
                  "more=%d, sdone=%d, sleft=%d, SvLEN(dst)=%d\n",
                  more, sdone, sleft, SvLEN(dst));
#endif
            if (sdone != 0) { /* has src ever been processed ? */
#if   ENCODE_XS_USEFP == 2
                more = (1.0*tlen*SvLEN(dst)+sdone-1)/sdone
                    - SvLEN(dst);
#elif ENCODE_XS_USEFP
                more = (STRLEN)((1.0*SvLEN(dst)+1)/sdone * sleft);
#else
            /* safe until SvLEN(dst) == MAX_INT/16 */
                more = (16*SvLEN(dst)+1)/sdone/16 * sleft;
#endif
            }
            more += UTF8_MAXLEN; /* insurance policy */
            d = (U8 *) SvGROW(dst, SvLEN(dst) + more);
            /* dst need to grow need MORE bytes! */
            if (ddone >= SvLEN(dst)) {
                Perl_croak(aTHX_ "Destination couldn't be grown.");
            }
            dlen = SvLEN(dst)-ddone-1;
            d   += ddone;
            s   += slen;
            slen = tlen-sdone;
            continue;
        }

    case ENCODE_NOREP:
        /* encoding */	
        if (dir == enc->f_utf8) {
        STRLEN clen;
        UV ch =
            utf8n_to_uvchr(s+slen, (tlen-sdone-slen),
                   &clen, UTF8_ALLOW_ANY|UTF8_CHECK_ONLY);
        /* if non-representable multibyte prefix at end of current buffer - break*/
        if (clen > tlen - sdone - slen) break;
        if (check & ENCODE_DIE_ON_ERR) {
            Perl_croak(aTHX_ ERR_ENCODE_NOMAP,
                   (UV)ch, enc->name[0]);
            return &PL_sv_undef; /* never reaches but be safe */
        }
        if (encode_ckWARN(check, WARN_UTF8)) {
            Perl_warner(aTHX_ packWARN(WARN_UTF8),
                ERR_ENCODE_NOMAP, (UV)ch, enc->name[0]);
        }
        if (check & ENCODE_RETURN_ON_ERR){
            goto ENCODE_SET_SRC;
        }
        if (check & (ENCODE_PERLQQ|ENCODE_HTMLCREF|ENCODE_XMLCREF)){
            STRLEN sublen;
            char *substr;
            SV* subchar = 
            (fallback_cb != &PL_sv_undef)
		? do_fallback_cb(aTHX_ ch, fallback_cb)
		: newSVpvf(check & ENCODE_PERLQQ ? "\\x{%04" UVxf "}" :
                 check & ENCODE_HTMLCREF ? "&#%" UVuf ";" :
                 "&#x%" UVxf ";", (UV)ch);
            substr = SvPV(subchar, sublen);
            if (SvUTF8(subchar) && sublen && !utf8_to_bytes((U8 *)substr, &sublen)) { /* make sure no decoded string gets in */
                SvREFCNT_dec(subchar);
                croak("Wide character");
            }
            sdone += slen + clen;
            ddone += dlen + sublen;
            sv_catpvn(dst, substr, sublen);
            SvREFCNT_dec(subchar);
        } else {
            /* fallback char */
            sdone += slen + clen;
            ddone += dlen + enc->replen;
            sv_catpvn(dst, (char*)enc->rep, enc->replen);
        }
        }
        /* decoding */
        else {
        if (check & ENCODE_DIE_ON_ERR){
            Perl_croak(aTHX_ ERR_DECODE_NOMAP,
                              enc->name[0], (UV)s[slen]);
            return &PL_sv_undef; /* never reaches but be safe */
        }
        if (encode_ckWARN(check, WARN_UTF8)) {
            Perl_warner(
            aTHX_ packWARN(WARN_UTF8),
            ERR_DECODE_NOMAP,
               	        enc->name[0], (UV)s[slen]);
        }
        if (check & ENCODE_RETURN_ON_ERR){
            goto ENCODE_SET_SRC;
        }
        if (check &
            (ENCODE_PERLQQ|ENCODE_HTMLCREF|ENCODE_XMLCREF)){
            STRLEN sublen;
            char *substr;
            SV* subchar = 
            (fallback_cb != &PL_sv_undef)
		? do_fallback_cb(aTHX_ (UV)s[slen], fallback_cb) 
		: newSVpvf("\\x%02" UVXf, (UV)s[slen]);
            substr = SvPVutf8(subchar, sublen);
            sdone += slen + 1;
            ddone += dlen + sublen;
            sv_catpvn(dst, substr, sublen);
            SvREFCNT_dec(subchar);
        } else {
            sdone += slen + 1;
            ddone += dlen + strlen(FBCHAR_UTF8);
            sv_catpvn(dst, FBCHAR_UTF8, strlen(FBCHAR_UTF8));
        }
        }
        /* settle variables when fallback */
        d    = (U8 *)SvEND(dst);
        dlen = SvLEN(dst) - ddone - 1;
        s    = sorig + sdone;
        slen = tlen - sdone;
        break;

        default:
            Perl_croak(aTHX_ "Unexpected code %d converting %s %s",
                   code, (dir == enc->f_utf8) ? "to" : "from",
                   enc->name[0]);
            return &PL_sv_undef;
        }
    }   /* End of looping through the string */
 ENCODE_SET_SRC:
    if (check && !(check & ENCODE_LEAVE_SRC)){
        sdone = tlen - (slen+sdone);
        sv_setpvn(src, (char*)s+slen, sdone);
        SvSETMAGIC(src);
    }
    /* warn("check = 0x%X, code = 0x%d\n", check, code); */

    SvCUR_set(dst, dlen+ddone);
    SvPOK_only(dst);

#if ENCODE_XS_PROFILE
    if (SvCUR(dst) > tlen){
    Perl_warn(aTHX_
          "SvLEN(dst)=%d, SvCUR(dst)=%d. %d bytes unused(%f %%)\n",
          SvLEN(dst), SvCUR(dst), SvLEN(dst) - SvCUR(dst),
          (SvLEN(dst) - SvCUR(dst))*1.0/SvLEN(dst)*100.0);
    }
#endif

    if (offset)
      *offset += sdone + slen;

 ENCODE_END:
    *SvEND(dst) = '\0';
    if (retcode) *retcode = code;
    return dst;
}

static bool
strict_utf8(pTHX_ SV* sv)
{
    HV* hv;
    SV** svp;
    sv = SvRV(sv);
    if (!sv || SvTYPE(sv) != SVt_PVHV)
        return 0;
    hv = (HV*)sv;
    svp = hv_fetch(hv, "strict_utf8", 11, 0);
    if (!svp)
        return 0;
    return SvTRUE(*svp);
}

static U8*
process_utf8(pTHX_ SV* dst, U8* s, U8* e, SV *check_sv,
             bool encode, bool strict, bool stop_at_partial)
{
    /* Copies the purportedly UTF-8 encoded string starting at 's' and ending
     * at 'e' - 1 to 'dst', checking as it goes along that the string actually
     * is valid UTF-8.  There are two levels of strictness checking.  If
     * 'strict' is FALSE, the string is checked for being well-formed UTF-8, as
     * extended by Perl.  Additionally, if 'strict' is TRUE, above-Unicode code
     * points, surrogates, and non-character code points are checked for.  When
     * invalid input is encountered, some action is taken, exactly what depends
     * on the flags in 'check_sv'.  'encode' gives if this is from an encode
     * operation (if TRUE), or a decode one.  This function returns the
     * position in 's' of the start of the next character beyond where it got
     * to.  If there were no problems, that will be 'e'.  If 'stop_at_partial'
     * is TRUE, if the final character before 'e' is incomplete, but valid as
     * far as is available, no action will be taken on that partial character,
     * and the return value will point to its first byte */

    UV uv;
    STRLEN ulen;
    SV *fallback_cb;
    IV check;
    U8 *d;
    STRLEN dlen;
    char esc[UTF8_MAXLEN * 6 + 1];
    STRLEN i;
    const U32 flags = (strict)
                    ? UTF8_DISALLOW_ILLEGAL_INTERCHANGE
                    : 0;

    if (!SvOK(check_sv)) {
	fallback_cb = &PL_sv_undef;
	check = 0;
    }
    else if (SvROK(check_sv)) {
	/* croak("UTF-8 decoder doesn't support callback CHECK"); */
	fallback_cb = check_sv;
	check = ENCODE_PERLQQ|ENCODE_LEAVE_SRC; /* same as perlqq */
    }
    else {
	fallback_cb = &PL_sv_undef;
	check = SvIV_nomg(check_sv);
    }

    SvPOK_only(dst);
    SvCUR_set(dst,0);

    dlen = (s && e && s < e) ? e-s+1 : 1;
    d = (U8 *) SvGROW(dst, dlen);

    stop_at_partial = stop_at_partial || (check & ENCODE_STOP_AT_PARTIAL);

    while (s < e) {

        /* If there were no errors, this will be 'e'; otherwise it will point
         * to the first byte of the erroneous input */
        const U8* e_or_where_failed;
        bool valid = is_utf8_string_loc_flags(s, e - s, &e_or_where_failed, flags);
        STRLEN len = e_or_where_failed - s;

        /* Copy as far as was successful */
        Move(s, d, len, U8);
        d += len;
        s = (U8 *) e_or_where_failed;

        /* Are done if it was valid, or we are accepting partial characters and
         * the only error is that the final bytes form a partial character */
        if (    LIKELY(valid)
            || (   stop_at_partial
                && is_utf8_valid_partial_char_flags(s, e, flags)))
        {
            break;
        }

        /* Here, was not valid.  If is 'strict', and is legal extended UTF-8,
         * we know it is a code point whose value we can calculate, just not
         * one accepted under strict.  Otherwise, it is malformed in some way.
         * In either case, the system function can calculate either the code
         * point, or the best substitution for it */
        uv = utf8n_to_uvchr(s, e - s, &ulen, UTF8_ALLOW_ANY);

        /*
         * Here, we are looping through the input and found an error.
         * 'uv' is the code point in error if calculable, or the REPLACEMENT
         *      CHARACTER if not.
         * 'ulen' is how many bytes of input this iteration of the loop
         *        consumes */

        if (!encode && (check & (ENCODE_DIE_ON_ERR|ENCODE_WARN_ON_ERR|ENCODE_PERLQQ)))
            for (i=0; i<ulen; ++i) sprintf(esc+4*i, "\\x%02X", s[i]);
        if (check & ENCODE_DIE_ON_ERR){
            if (encode)
                Perl_croak(aTHX_ ERR_ENCODE_NOMAP, uv, (strict ? "UTF-8" : "utf8"));
            else
                Perl_croak(aTHX_ ERR_DECODE_STR_NOMAP, (strict ? "UTF-8" : "utf8"), esc);
        }
        if (encode_ckWARN(check, WARN_UTF8)) {
            if (encode)
                Perl_warner(aTHX_ packWARN(WARN_UTF8),
                            ERR_ENCODE_NOMAP, uv, (strict ? "UTF-8" : "utf8"));
            else
                Perl_warner(aTHX_ packWARN(WARN_UTF8),
                            ERR_DECODE_STR_NOMAP, (strict ? "UTF-8" : "utf8"), esc);
        }
        if (check & ENCODE_RETURN_ON_ERR) {
                break;
        }
        if (check & (ENCODE_PERLQQ|ENCODE_HTMLCREF|ENCODE_XMLCREF)){
            STRLEN sublen;
            char *substr;
            SV* subchar;
            if (encode) {
                subchar =
                    (fallback_cb != &PL_sv_undef)
                    ? do_fallback_cb(aTHX_ uv, fallback_cb)
                    : newSVpvf(check & ENCODE_PERLQQ
                        ? (ulen == 1 ? "\\x%02" UVXf : "\\x{%04" UVXf "}")
                        :  check & ENCODE_HTMLCREF ? "&#%" UVuf ";"
                        : "&#x%" UVxf ";", uv);
                substr = SvPV(subchar, sublen);
                if (SvUTF8(subchar) && sublen && !utf8_to_bytes((U8 *)substr, &sublen)) { /* make sure no decoded string gets in */
                    SvREFCNT_dec(subchar);
                    croak("Wide character");
                }
            } else {
                if (fallback_cb != &PL_sv_undef) {
                    /* in decode mode we have sequence of wrong bytes */
                    subchar = do_bytes_fallback_cb(aTHX_ s, ulen, fallback_cb);
                } else {
                    char *ptr = esc;
                    /* ENCODE_PERLQQ is already stored in esc */
                    if (check & (ENCODE_HTMLCREF|ENCODE_XMLCREF))
                        for (i=0; i<ulen; ++i) ptr += sprintf(ptr, ((check & ENCODE_HTMLCREF) ? "&#%u;" : "&#x%02X;"), s[i]);
                    subchar = newSVpvn(esc, strlen(esc));
                }
                substr = SvPVutf8(subchar, sublen);
            }
            dlen += sublen - ulen;
            SvCUR_set(dst, d-(U8 *)SvPVX(dst));
            *SvEND(dst) = '\0';
            sv_catpvn(dst, substr, sublen);
            SvREFCNT_dec(subchar);
            d = (U8 *) SvGROW(dst, dlen) + SvCUR(dst);
        } else {
            STRLEN fbcharlen = strlen(FBCHAR_UTF8);
            dlen += fbcharlen - ulen;
            if (SvLEN(dst) < dlen) {
                SvCUR_set(dst, d-(U8 *)SvPVX(dst));
                d = (U8 *) sv_grow(dst, dlen) + SvCUR(dst);
            }
            memcpy(d, FBCHAR_UTF8, fbcharlen);
            d += fbcharlen;
        }
        s += ulen;
    }
    SvCUR_set(dst, d-(U8 *)SvPVX(dst));
    *SvEND(dst) = '\0';

    return s;
}

static SV *
find_encoding(pTHX_ SV *enc)
{
    dSP;
    I32 count;
    SV *m_enc;
    SV *obj = &PL_sv_undef;
#ifndef SV_NOSTEAL
    U32 tmp;
#endif

    ENTER;
    SAVETMPS;
    PUSHMARK(sp);

    m_enc = sv_newmortal();
#ifndef SV_NOSTEAL
    tmp = SvFLAGS(enc) & SVs_TEMP;
    SvTEMP_off(enc);
    sv_setsv_flags(m_enc, enc, 0);
    SvFLAGS(enc) |= tmp;
#else
#if SV_NOSTEAL == 0
    #error You have broken SV_NOSTEAL which cause memory corruption in sv_setsv_flags()
    #error Most probably broken SV_NOSTEAL was defined by buggy version of ppport.h
#else
    sv_setsv_flags(m_enc, enc, SV_NOSTEAL);
#endif
#endif
    XPUSHs(m_enc);

    PUTBACK;

    count = call_pv("Encode::find_encoding", G_SCALAR);

    SPAGAIN;

    if (count > 0) {
        obj = POPs;
        SvREFCNT_inc(obj);
    }

    PUTBACK;
    FREETMPS;
    LEAVE;
    return sv_2mortal(obj);
}

static SV *
call_encoding(pTHX_ const char *method, SV *obj, SV *src, SV *check)
{
    dSP;
    I32 count;
    SV *dst = &PL_sv_undef;

    PUSHMARK(sp);

    if (check)
        check = sv_2mortal(newSVsv(check));

    if (!check || SvROK(check) || !SvTRUE_nomg(check) || (SvIV_nomg(check) & ENCODE_LEAVE_SRC))
        src = sv_2mortal(newSVsv(src));

    XPUSHs(obj);
    XPUSHs(src);
    XPUSHs(check ? check : &PL_sv_no);

    PUTBACK;

    count = call_method(method, G_SCALAR);

    SPAGAIN;

    if (count > 0) {
        dst = POPs;
        SvREFCNT_inc(dst);
    }

    PUTBACK;
    return dst;
}


MODULE = Encode		PACKAGE = Encode::utf8	PREFIX = Method_

PROTOTYPES: DISABLE

void
Method_decode(obj,src,check_sv = &PL_sv_no)
SV *	obj
SV *	src
SV *	check_sv
PREINIT:
    STRLEN slen;
    U8 *s;
    U8 *e;
    SV *dst;
    bool renewed = 0;
    IV check;
    bool modify;
    dSP;
INIT:
    SvGETMAGIC(src);
    SvGETMAGIC(check_sv);
    check = SvROK(check_sv) ? ENCODE_PERLQQ|ENCODE_LEAVE_SRC : SvOK(check_sv) ? SvIV_nomg(check_sv) : 0;
    modify = (check && !(check & ENCODE_LEAVE_SRC));
PPCODE:
    if (!SvOK(src))
        XSRETURN_UNDEF;
    s = modify ? (U8 *)SvPV_force_nomg(src, slen) : (U8 *)SvPV_nomg(src, slen);
    if (SvUTF8(src))
        utf8_safe_downgrade(aTHX_ &src, &s, &slen, modify);
    e = s+slen;

    /*
     * PerlIO check -- we assume the object is of PerlIO if renewed
     */
    ENTER; SAVETMPS;
    PUSHMARK(sp);
    XPUSHs(obj);
    PUTBACK;
    if (call_method("renewed",G_SCALAR) == 1) {
    SPAGAIN;
    renewed = (bool)POPi;
    PUTBACK;
#if 0
    fprintf(stderr, "renewed == %d\n", renewed);
#endif
    }
    FREETMPS; LEAVE;
    /* end PerlIO check */

    dst = sv_2mortal(newSV(slen>0?slen:1)); /* newSV() abhors 0 -- inaba */
    s = process_utf8(aTHX_ dst, s, e, check_sv, 0, strict_utf8(aTHX_ obj), renewed);

    /* Clear out translated part of source unless asked not to */
    if (modify) {
        slen = e-s;
        sv_setpvn(src, (char*)s, slen);
        SvSETMAGIC(src);
    }
    SvUTF8_on(dst);
    if (SvTAINTED(src)) SvTAINTED_on(dst); /* propagate taintedness */
    ST(0) = dst;
    XSRETURN(1);

void
Method_encode(obj,src,check_sv = &PL_sv_no)
SV *	obj
SV *	src
SV *	check_sv
PREINIT:
    STRLEN slen;
    U8 *s;
    U8 *e;
    SV *dst;
    IV check;
    bool modify;
INIT:
    SvGETMAGIC(src);
    SvGETMAGIC(check_sv);
    check = SvROK(check_sv) ? ENCODE_PERLQQ|ENCODE_LEAVE_SRC : SvOK(check_sv) ? SvIV_nomg(check_sv) : 0;
    modify = (check && !(check & ENCODE_LEAVE_SRC));
PPCODE:
    if (!SvOK(src))
        XSRETURN_UNDEF;
    s = modify ? (U8 *)SvPV_force_nomg(src, slen) : (U8 *)SvPV_nomg(src, slen);
    e = s+slen;
    dst = sv_2mortal(newSV(slen>0?slen:1)); /* newSV() abhors 0 -- inaba */
    if (SvUTF8(src)) {
    /* Already encoded */
    if (strict_utf8(aTHX_ obj)) {
        s = process_utf8(aTHX_ dst, s, e, check_sv, 1, 1, 0);
    }
        else {
            /* trust it and just copy the octets */
    	    sv_setpvn(dst,(char *)s,(e-s));
        s = e;
        }
    }
    else {
        /* Native bytes - can always encode */
        U8 *d = (U8 *) SvGROW(dst, 2*slen+1); /* +1 or assertion will botch */
        while (s < e) {
#ifdef append_utf8_from_native_byte
            append_utf8_from_native_byte(*s, &d);
            s++;
#else
            UV uv = NATIVE_TO_UNI((UV) *s);
            s++; /* Above expansion of NATIVE_TO_UNI() is safer this way. */
            if (UNI_IS_INVARIANT(uv))
                *d++ = (U8)UTF_TO_NATIVE(uv);
            else {
                *d++ = (U8)UTF8_EIGHT_BIT_HI(uv);
                *d++ = (U8)UTF8_EIGHT_BIT_LO(uv);
            }
#endif
        }
        SvCUR_set(dst, d- (U8 *)SvPVX(dst));
        *SvEND(dst) = '\0';
    }

    /* Clear out translated part of source unless asked not to */
    if (modify) {
        slen = e-s;
        sv_setpvn(src, (char*)s, slen);
        SvSETMAGIC(src);
    }
    SvPOK_only(dst);
    SvUTF8_off(dst);
    if (SvTAINTED(src)) SvTAINTED_on(dst); /* propagate taintedness */
    ST(0) = dst;
    XSRETURN(1);

MODULE = Encode		PACKAGE = Encode::XS	PREFIX = Method_

PROTOTYPES: DISABLE

SV *
Method_renew(obj)
SV *	obj
CODE:
    PERL_UNUSED_VAR(obj);
    RETVAL = newSVsv(obj);
OUTPUT:
    RETVAL

int
Method_renewed(obj)
SV *    obj
CODE:
    RETVAL = 0;
    PERL_UNUSED_VAR(obj);
OUTPUT:
    RETVAL

SV *
Method_name(obj)
SV *	obj
PREINIT:
    encode_t *enc;
INIT:
    enc = INT2PTR(encode_t *, SvIV(SvRV(obj)));
CODE:
    RETVAL = newSVpvn(enc->name[0], strlen(enc->name[0]));
OUTPUT:
    RETVAL

bool
Method_cat_decode(obj, dst, src, off, term, check_sv = &PL_sv_no)
SV *	obj
SV *	dst
SV *	src
SV *	off
SV *	term
SV *    check_sv
PREINIT:
    IV check;
    SV *fallback_cb;
    bool modify;
    encode_t *enc;
    STRLEN offset;
    int code = 0;
    U8 *s;
    STRLEN slen;
    SV *tmp;
INIT:
    SvGETMAGIC(src);
    SvGETMAGIC(check_sv);
    check = SvROK(check_sv) ? ENCODE_PERLQQ|ENCODE_LEAVE_SRC : SvOK(check_sv) ? SvIV_nomg(check_sv) : 0;
    fallback_cb = SvROK(check_sv) ? check_sv : &PL_sv_undef;
    modify = (check && !(check & ENCODE_LEAVE_SRC));
    enc = INT2PTR(encode_t *, SvIV(SvRV(obj)));
    offset = (STRLEN)SvIV(off);
CODE:
    if (!SvOK(src))
        XSRETURN_NO;
    s = modify ? (U8 *)SvPV_force_nomg(src, slen) : (U8 *)SvPV_nomg(src, slen);
    if (SvUTF8(src))
        utf8_safe_downgrade(aTHX_ &src, &s, &slen, modify);
    tmp = encode_method(aTHX_ enc, enc->t_utf8, src, s, slen, check,
                &offset, term, &code, fallback_cb);
    sv_catsv(dst, tmp);
    SvIV_set(off, (IV)offset);
    RETVAL = (code == ENCODE_FOUND_TERM);
OUTPUT:
    RETVAL

void
Method_decode(obj,src,check_sv = &PL_sv_no)
SV *	obj
SV *	src
SV *	check_sv
PREINIT:
    IV check;
    SV *fallback_cb;
    bool modify;
    encode_t *enc;
    U8 *s;
    STRLEN slen;
    SV *ret;
INIT:
    SvGETMAGIC(src);
    SvGETMAGIC(check_sv);
    check = SvROK(check_sv) ? ENCODE_PERLQQ|ENCODE_LEAVE_SRC : SvOK(check_sv) ? SvIV_nomg(check_sv) : 0;
    fallback_cb = SvROK(check_sv) ? check_sv : &PL_sv_undef;
    modify = (check && !(check & ENCODE_LEAVE_SRC));
    enc = INT2PTR(encode_t *, SvIV(SvRV(obj)));
CODE:
    if (!SvOK(src))
        XSRETURN_UNDEF;
    s = modify ? (U8 *)SvPV_force_nomg(src, slen) : (U8 *)SvPV_nomg(src, slen);
    if (SvUTF8(src))
        utf8_safe_downgrade(aTHX_ &src, &s, &slen, modify);
    ret = encode_method(aTHX_ enc, enc->t_utf8, src, s, slen, check,
              NULL, Nullsv, NULL, fallback_cb);
    SvUTF8_on(ret);
    ST(0) = ret;
    XSRETURN(1);

void
Method_encode(obj,src,check_sv = &PL_sv_no)
SV *	obj
SV *	src
SV *	check_sv
PREINIT:
    IV check;
    SV *fallback_cb;
    bool modify;
    encode_t *enc;
    U8 *s;
    STRLEN slen;
    SV *ret;
INIT:
    SvGETMAGIC(src);
    SvGETMAGIC(check_sv);
    check = SvROK(check_sv) ? ENCODE_PERLQQ|ENCODE_LEAVE_SRC : SvOK(check_sv) ? SvIV_nomg(check_sv) : 0;
    fallback_cb = SvROK(check_sv) ? check_sv : &PL_sv_undef;
    modify = (check && !(check & ENCODE_LEAVE_SRC));
    enc = INT2PTR(encode_t *, SvIV(SvRV(obj)));
CODE:
    if (!SvOK(src))
        XSRETURN_UNDEF;
    s = modify ? (U8 *)SvPV_force_nomg(src, slen) : (U8 *)SvPV_nomg(src, slen);
    if (!SvUTF8(src))
        utf8_safe_upgrade(aTHX_ &src, &s, &slen, modify);
    ret = encode_method(aTHX_ enc, enc->f_utf8, src, s, slen, check,
              NULL, Nullsv, NULL, fallback_cb);
    ST(0) = ret;
    XSRETURN(1);

bool
Method_needs_lines(obj)
SV *	obj
CODE:
    PERL_UNUSED_VAR(obj);
    RETVAL = FALSE;
OUTPUT:
    RETVAL

bool
Method_perlio_ok(obj)
SV *	obj
PREINIT:
    SV *sv;
CODE:
    PERL_UNUSED_VAR(obj);
    sv = eval_pv("require PerlIO::encoding", 0);
    RETVAL = SvTRUE(sv);
OUTPUT:
    RETVAL

SV *
Method_mime_name(obj)
SV *	obj
PREINIT:
    encode_t *enc;
INIT:
    enc = INT2PTR(encode_t *, SvIV(SvRV(obj)));
CODE:
    ENTER;
    SAVETMPS;
    PUSHMARK(sp);
    XPUSHs(sv_2mortal(newSVpvn(enc->name[0], strlen(enc->name[0]))));
    PUTBACK;
    call_pv("Encode::MIME::Name::get_mime_name", G_SCALAR);
    SPAGAIN;
    RETVAL = newSVsv(POPs);
    PUTBACK;
    FREETMPS;
    LEAVE;
OUTPUT:
    RETVAL

MODULE = Encode         PACKAGE = Encode

PROTOTYPES: ENABLE

bool
is_utf8(sv, check = 0)
SV *	sv
int	check
PREINIT:
    char *str;
    STRLEN len;
CODE:
    SvGETMAGIC(sv); /* SvGETMAGIC() can modify SvOK flag */
    str = SvOK(sv) ? SvPV_nomg(sv, len) : NULL; /* SvPV() can modify SvUTF8 flag */
    RETVAL = SvUTF8(sv) ? TRUE : FALSE;
    if (RETVAL && check && (!str || !is_utf8_string((U8 *)str, len)))
        RETVAL = FALSE;
OUTPUT:
    RETVAL

SV *
_utf8_on(sv)
SV *	sv
CODE:
    SvGETMAGIC(sv);
    if (!SvTAINTED(sv) && SvPOKp(sv)) {
        if (SvTHINKFIRST(sv)) sv_force_normal(sv);
        RETVAL = boolSV(SvUTF8(sv));
        SvUTF8_on(sv);
        SvSETMAGIC(sv);
    } else {
        RETVAL = &PL_sv_undef;
    }
OUTPUT:
    RETVAL

SV *
_utf8_off(sv)
SV *	sv
CODE:
    SvGETMAGIC(sv);
    if (!SvTAINTED(sv) && SvPOKp(sv)) {
        if (SvTHINKFIRST(sv)) sv_force_normal(sv);
        RETVAL = boolSV(SvUTF8(sv));
        SvUTF8_off(sv);
        SvSETMAGIC(sv);
    } else {
        RETVAL = &PL_sv_undef;
    }
OUTPUT:
    RETVAL

SV *
decode(encoding, octets, check = NULL)
SV *	encoding
SV *	octets
SV *	check
ALIAS:
    bytes2str = 0
PREINIT:
    SV *obj;
INIT:
    PERL_UNUSED_VAR(ix);
    SvGETMAGIC(encoding);
CODE:
    if (!SvOK(encoding))
        croak("Encoding name should not be undef");
    obj = find_encoding(aTHX_ encoding);
    if (!SvOK(obj))
        croak("Unknown encoding '%" SVf "'", SVfARG(encoding));
    RETVAL = call_encoding(aTHX_ "decode", obj, octets, check);
OUTPUT:
    RETVAL

SV *
encode(encoding, string, check = NULL)
SV *	encoding
SV *	string
SV *	check
ALIAS:
    str2bytes = 0
PREINIT:
    SV *obj;
INIT:
    PERL_UNUSED_VAR(ix);
    SvGETMAGIC(encoding);
CODE:
    if (!SvOK(encoding))
        croak("Encoding name should not be undef");
    obj = find_encoding(aTHX_ encoding);
    if (!SvOK(obj))
        croak("Unknown encoding '%" SVf "'", SVfARG(encoding));
    RETVAL = call_encoding(aTHX_ "encode", obj, string, check);
OUTPUT:
    RETVAL

SV *
decode_utf8(octets, check = NULL)
SV *	octets
SV *	check
PREINIT:
    HV *hv;
    SV **sv;
CODE:
    hv = get_hv("Encode::Encoding", 0);
    if (!hv)
        croak("utf8 encoding was not found");
    sv = hv_fetch(hv, "utf8", 4, 0);
    if (!sv || !*sv || !SvOK(*sv))
        croak("utf8 encoding was not found");
    RETVAL = call_encoding(aTHX_ "decode", *sv, octets, check);
OUTPUT:
    RETVAL

SV *
encode_utf8(string)
SV *	string
CODE:
    RETVAL = newSVsv(string);
    if (SvOK(RETVAL))
        sv_utf8_encode(RETVAL);
OUTPUT:
    RETVAL

SV *
from_to(octets, from, to, check = NULL)
SV *	octets
SV *	from
SV *	to
SV *	check
PREINIT:
    SV *from_obj;
    SV *to_obj;
    SV *string;
    SV *new_octets;
    U8 *ptr;
    STRLEN len;
INIT:
    SvGETMAGIC(from);
    SvGETMAGIC(to);
CODE:
    if (!SvOK(from) || !SvOK(to))
        croak("Encoding name should not be undef");
    from_obj = find_encoding(aTHX_ from);
    if (!SvOK(from_obj))
        croak("Unknown encoding '%" SVf "'", SVfARG(from));
    to_obj = find_encoding(aTHX_ to);
    if (!SvOK(to_obj))
        croak("Unknown encoding '%" SVf "'", SVfARG(to));
    string = sv_2mortal(call_encoding(aTHX_ "decode", from_obj, octets, NULL));
    new_octets = sv_2mortal(call_encoding(aTHX_ "encode", to_obj, string, check));
    SvGETMAGIC(new_octets);
    if (SvOK(new_octets) && (!check || SvROK(check) || !SvTRUE_nomg(check) || sv_len(string) == 0)) {
        ptr = (U8 *)SvPV_nomg(new_octets, len);
        if (SvUTF8(new_octets))
            len = utf8_length(ptr, ptr+len);
        RETVAL = newSVuv(len);
    } else {
        RETVAL = &PL_sv_undef;
    }
    sv_setsv_nomg(octets, new_octets);
    SvSETMAGIC(octets);
OUTPUT:
    RETVAL

void
onBOOT()
CODE:
{
#include "def_t.exh"
}

BOOT:
{
    HV *stash = gv_stashpvn("Encode", (U32)strlen("Encode"), GV_ADD);
    newCONSTSUB(stash, "DIE_ON_ERR", newSViv(ENCODE_DIE_ON_ERR));
    newCONSTSUB(stash, "WARN_ON_ERR", newSViv(ENCODE_WARN_ON_ERR));
    newCONSTSUB(stash, "RETURN_ON_ERR", newSViv(ENCODE_RETURN_ON_ERR));
    newCONSTSUB(stash, "LEAVE_SRC", newSViv(ENCODE_LEAVE_SRC));
    newCONSTSUB(stash, "ONLY_PRAGMA_WARNINGS", newSViv(ENCODE_ONLY_PRAGMA_WARNINGS));
    newCONSTSUB(stash, "PERLQQ", newSViv(ENCODE_PERLQQ));
    newCONSTSUB(stash, "HTMLCREF", newSViv(ENCODE_HTMLCREF));
    newCONSTSUB(stash, "XMLCREF", newSViv(ENCODE_XMLCREF));
    newCONSTSUB(stash, "STOP_AT_PARTIAL", newSViv(ENCODE_STOP_AT_PARTIAL));
    newCONSTSUB(stash, "FB_DEFAULT", newSViv(ENCODE_FB_DEFAULT));
    newCONSTSUB(stash, "FB_CROAK", newSViv(ENCODE_FB_CROAK));
    newCONSTSUB(stash, "FB_QUIET", newSViv(ENCODE_FB_QUIET));
    newCONSTSUB(stash, "FB_WARN", newSViv(ENCODE_FB_WARN));
    newCONSTSUB(stash, "FB_PERLQQ", newSViv(ENCODE_FB_PERLQQ));
    newCONSTSUB(stash, "FB_HTMLCREF", newSViv(ENCODE_FB_HTMLCREF));
    newCONSTSUB(stash, "FB_XMLCREF", newSViv(ENCODE_FB_XMLCREF));
}
