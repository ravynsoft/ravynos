/*
 $Id: Unicode.xs,v 2.20 2021/07/23 02:26:54 dankogai Exp $
 */

#define IN_UNICODE_XS

#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "../Encode/encode.h"

#define FBCHAR			0xFFFd
#define BOM_BE			0xFeFF
#define BOM16LE			0xFFFe
#define BOM32LE			0xFFFe0000
#define issurrogate(x)		(0xD800 <= (x)  && (x) <= 0xDFFF )
#define isHiSurrogate(x)	(0xD800 <= (x)  && (x) <  0xDC00 )
#define isLoSurrogate(x)	(0xDC00 <= (x)  && (x) <= 0xDFFF )
#define invalid_ucs2(x)         ( issurrogate(x) || 0xFFFF < (x) )

#ifndef SVfARG
#define SVfARG(p) ((void*)(p))
#endif

#define PERLIO_BUFSIZ 1024 /* XXX value comes from PerlIOEncode_get_base */

/* Avoid wasting too much space in the result buffer */
/* static void */
/* shrink_buffer(SV *result) */
/* { */
/*     if (SvLEN(result) > 42 + SvCUR(result)) { */
/* 	char *buf; */
/* 	STRLEN len = 1 + SvCUR(result); /\* include the NUL byte *\/ */
/* 	New(0, buf, len, char); */
/* 	Copy(SvPVX(result), buf, len, char); */
/* 	Safefree(SvPVX(result)); */
/* 	SvPV_set(result, buf); */
/* 	SvLEN_set(result, len); */
/*     } */
/* } */

#define shrink_buffer(result) { \
    if (SvLEN(result) > 42 + SvCUR(result)) { \
	char *newpv; \
	STRLEN newlen = 1 + SvCUR(result); /* include the NUL byte */ \
	New(0, newpv, newlen, char); \
	Copy(SvPVX(result), newpv, newlen, char); \
	Safefree(SvPVX(result)); \
	SvPV_set(result, newpv); \
	SvLEN_set(result, newlen); \
    } \
}

static UV
enc_unpack(pTHX_ U8 **sp, U8 *e, STRLEN size, U8 endian)
{
    U8 *s = *sp;
    UV v = 0;
    if (s+size > e) {
	croak("Partial character %c",(char) endian);
    }
    switch(endian) {
    case 'N':
	v = *s++;
	v = (v << 8) | *s++;
        /* FALLTHROUGH */
    case 'n':
	v = (v << 8) | *s++;
	v = (v << 8) | *s++;
	break;
    case 'V':
    case 'v':
	v |= *s++;
	v |= (*s++ << 8);
	if (endian == 'v')
	    break;
	v |= (*s++ << 16);
	v |= ((UV)*s++ << 24);
	break;
    default:
	croak("Unknown endian %c",(char) endian);
	break;
    }
    *sp = s;
    return v;
}

static void
enc_pack(pTHX_ SV *result, STRLEN size, U8 endian, UV value)
{
    U8 *d = (U8 *) SvPV_nolen(result);

    switch(endian) {
    case 'v':
    case 'V':
	d += SvCUR(result);
	SvCUR_set(result,SvCUR(result)+size);
	while (size--) {
	    *d++ = (U8)(value & 0xFF);
	    value >>= 8;
	}
	break;
    case 'n':
    case 'N':
	SvCUR_set(result,SvCUR(result)+size);
	d += SvCUR(result);
	while (size--) {
	    *--d = (U8)(value & 0xFF);
	    value >>= 8;
	}
	break;
    default:
	croak("Unknown endian %c",(char) endian);
	break;
    }
}

MODULE = Encode::Unicode PACKAGE = Encode::Unicode

PROTOTYPES: DISABLE

#define attr(k)  (hv_exists((HV *)SvRV(obj),"" k "",sizeof(k)-1) ? \
    *hv_fetch((HV *)SvRV(obj),"" k "",sizeof(k)-1,0) : &PL_sv_undef)

void
decode(obj, str, check = 0)
SV *	obj
SV *	str
IV	check
CODE:
{
    SV *name     = attr("Name");
    SV *sve      = attr("endian");
    U8 endian    = *((U8 *)SvPV_nolen(sve));
    SV *svs      = attr("size");
    int size     = SvIV(svs);
    int ucs2     = -1; /* only needed in the event of surrogate pairs */
    SV *result   = newSVpvn("",0);
    STRLEN usize = (size > 0 ? size : 1); /* protect against rogue size<=0 */
    STRLEN ulen;
    STRLEN resultbuflen;
    U8 *resultbuf;
    U8 *s;
    U8 *e;
    bool modify = (check && !(check & ENCODE_LEAVE_SRC));
    bool temp_result;

    SvGETMAGIC(str);
    if (!SvOK(str))
        XSRETURN_UNDEF;
    s = modify ? (U8 *)SvPV_force_nomg(str, ulen) : (U8 *)SvPV_nomg(str, ulen);
    if (SvUTF8(str)) {
        if (!modify) {
            SV *tmp = sv_2mortal(newSVpvn((char *)s, ulen));
            SvUTF8_on(tmp);
            if (SvTAINTED(str))
                SvTAINTED_on(tmp);
            str = tmp;
            s = (U8 *)SvPVX(str);
        }
        if (ulen) {
            if (!utf8_to_bytes(s, &ulen))
                croak("Wide character");
            SvCUR_set(str, ulen);
        }
        SvUTF8_off(str);
    }
    e = s+ulen;

    /* Optimise for the common case of being called from PerlIOEncode_fill()
       with a standard length buffer. In this case the result SV's buffer is
       only used temporarily, so we can afford to allocate the maximum needed
       and not care about unused space. */
    temp_result = (ulen == PERLIO_BUFSIZ);

    ST(0) = sv_2mortal(result);
    SvUTF8_on(result);

    if (!endian && s+size <= e) {
	SV *sv;
	UV bom;
	endian = (size == 4) ? 'N' : 'n';
	bom = enc_unpack(aTHX_ &s,e,size,endian);
	if (bom != BOM_BE) {
	    if (bom == BOM16LE) {
		endian = 'v';
	    }
	    else if (bom == BOM32LE) {
		endian = 'V';
	    }
	    else {
               /* No BOM found, use big-endian fallback as specified in
                * RFC2781 and the Unicode Standard version 8.0:
                *
                *  The UTF-16 encoding scheme may or may not begin with
                *  a BOM. However, when there is no BOM, and in the
                *  absence of a higher-level protocol, the byte order
                *  of the UTF-16 encoding scheme is big-endian.
                *
                *  If the first two octets of the text is not 0xFE
                *  followed by 0xFF, and is not 0xFF followed by 0xFE,
                *  then the text SHOULD be interpreted as big-endian.
                */
                s -= size;
	    }
	}
#if 1
	/* Update endian for next sequence */
	sv = attr("renewed");
	if (SvTRUE(sv)) {
	    (void)hv_store((HV *)SvRV(obj),"endian",6,newSVpv((char *)&endian,1),0);
	}
#endif
    }

    if (temp_result) {
	resultbuflen = 1 + ulen/usize * UTF8_MAXLEN;
    } else {
	/* Preallocate the buffer to the minimum possible space required. */
	resultbuflen = ulen/usize + UTF8_MAXLEN + 1;
    }
    resultbuf = (U8 *) SvGROW(result, resultbuflen);

    while (s < e && s+size <= e) {
	UV ord = enc_unpack(aTHX_ &s,e,size,endian);
	U8 *d;
	HV *hv = NULL;
	if (issurrogate(ord)) {
	    if (ucs2 == -1) {
		SV *sv = attr("ucs2");
		ucs2 = SvTRUE(sv);
	    }
	    if (ucs2 || size == 4) {
		if (check & ENCODE_DIE_ON_ERR) {
		    croak("%" SVf ":no surrogates allowed %" UVxf,
			  SVfARG(name), ord);
		}
		if (encode_ckWARN(check, WARN_SURROGATE)) {
		    warner(packWARN(WARN_SURROGATE),
			  "%" SVf ":no surrogates allowed %" UVxf,
			  SVfARG(name), ord);
		}
		ord = FBCHAR;
	    }
	    else {
		UV lo;
		if (!isHiSurrogate(ord)) {
		    if (check & ENCODE_DIE_ON_ERR) {
			croak("%" SVf ":Malformed HI surrogate %" UVxf,
			      SVfARG(name), ord);
		    }
		    if (encode_ckWARN(check, WARN_SURROGATE)) {
			warner(packWARN(WARN_SURROGATE),
			      "%" SVf ":Malformed HI surrogate %" UVxf,
			      SVfARG(name), ord);
		    }
		    ord = FBCHAR;
		}
		else if (s+size > e) {
		    if (check & ENCODE_STOP_AT_PARTIAL) {
		        s -= size;
		        break;
		    }
		    if (check & ENCODE_DIE_ON_ERR) {
			croak("%" SVf ":Malformed HI surrogate %" UVxf,
			      SVfARG(name), ord);
		    }
		    if (encode_ckWARN(check, WARN_SURROGATE)) {
			warner(packWARN(WARN_SURROGATE),
			      "%" SVf ":Malformed HI surrogate %" UVxf,
			      SVfARG(name), ord);
		    }
		    ord = FBCHAR;
		}
		else {
		    lo = enc_unpack(aTHX_ &s,e,size,endian);
		    if (!isLoSurrogate(lo)) {
			if (check & ENCODE_DIE_ON_ERR) {
			    croak("%" SVf ":Malformed LO surrogate %" UVxf,
				  SVfARG(name), ord);
			}
			if (encode_ckWARN(check, WARN_SURROGATE)) {
			    warner(packWARN(WARN_SURROGATE),
				  "%" SVf ":Malformed LO surrogate %" UVxf,
				  SVfARG(name), ord);
			}
			s -= size;
			ord = FBCHAR;
		    }
		    else {
			ord = 0x10000 + ((ord - 0xD800) << 10) + (lo - 0xDC00);
		    }
		}
	    }
	}

	if ((ord & 0xFFFE) == 0xFFFE || (ord >= 0xFDD0 && ord <= 0xFDEF)) {
	    if (check & ENCODE_DIE_ON_ERR) {
		croak("%" SVf ":Unicode character %" UVxf " is illegal",
		      SVfARG(name), ord);
	    }
	    if (encode_ckWARN(check, WARN_NONCHAR)) {
	        warner(packWARN(WARN_NONCHAR),
		      "%" SVf ":Unicode character %" UVxf " is illegal",
		      SVfARG(name), ord);
	    }
	    ord = FBCHAR;
	}

	if (resultbuflen < SvCUR(result) + UTF8_MAXLEN + 1) {
	    /* Do not allocate >8Mb more than the minimum needed.
	       This prevents allocating too much in the rogue case of a large
	       input consisting initially of long sequence uft8-byte unicode
	       chars followed by single utf8-byte chars. */
            /* +1 
               fixes  Unicode.xs!decode_xs n-byte heap-overflow
              */
	    STRLEN remaining = (e - s)/usize + 1; /* +1 to avoid the leak */
	    STRLEN max_alloc = remaining + (8*1024*1024);
	    STRLEN est_alloc = remaining * UTF8_MAXLEN;
	    STRLEN newlen = SvLEN(result) + /* min(max_alloc, est_alloc) */
		(est_alloc > max_alloc ? max_alloc : est_alloc);
	    resultbuf = (U8 *) SvGROW(result, newlen);
	    resultbuflen = SvLEN(result);
	}

        d = uvchr_to_utf8_flags_msgs(resultbuf+SvCUR(result), ord, UNICODE_DISALLOW_ILLEGAL_INTERCHANGE | UNICODE_WARN_ILLEGAL_INTERCHANGE, &hv);
        if (hv) {
            SV *message = *hv_fetch(hv, "text", 4, 0);
            U32 categories = SvUVx(*hv_fetch(hv, "warn_categories", 15, 0));
            sv_2mortal((SV *)hv);
            if (check & ENCODE_DIE_ON_ERR)
                croak("%" SVf, SVfARG(message));
            if (encode_ckWARN_packed(check, categories))
                warner(categories, "%" SVf, SVfARG(message));
            d = uvchr_to_utf8_flags(resultbuf+SvCUR(result), FBCHAR, 0);
        }

	SvCUR_set(result, d - (U8 *)SvPVX(result));
    }

    if (s < e) {
	/* unlikely to happen because it's fixed-length -- dankogai */
        if (check & ENCODE_DIE_ON_ERR)
            croak("%" SVf ":Partial character", SVfARG(name));
        if (encode_ckWARN(check, WARN_UTF8)) {
            warner(packWARN(WARN_UTF8),"%" SVf ":Partial character", SVfARG(name));
	}
    }
    if (check && !(check & ENCODE_LEAVE_SRC)) {
	if (s < e) {
	    Move(s,SvPVX(str),e-s,U8);
	    SvCUR_set(str,(e-s));
	}
	else {
	    SvCUR_set(str,0);
	}
	*SvEND(str) = '\0';
	SvSETMAGIC(str);
    }

    if (!temp_result) shrink_buffer(result);

    /* Make sure we have a trailing NUL: */
    *SvEND(result) = '\0';

    if (SvTAINTED(str)) SvTAINTED_on(result); /* propagate taintedness */
    XSRETURN(1);
}

void
encode(obj, utf8, check = 0)
SV *	obj
SV *	utf8
IV	check
CODE:
{
    SV *name = attr("Name");
    SV *sve = attr("endian");
    U8 endian = *((U8 *)SvPV_nolen(sve));
    SV *svs = attr("size");
    const int size = SvIV(svs);
    int ucs2 = -1; /* only needed if there is invalid_ucs2 input */
    const STRLEN usize = (size > 0 ? size : 1);
    SV *result = newSVpvn("", 0);
    STRLEN ulen;
    U8 *s;
    U8 *e;
    bool modify = (check && !(check & ENCODE_LEAVE_SRC));
    bool temp_result;

    SvGETMAGIC(utf8);
    if (!SvOK(utf8))
        XSRETURN_UNDEF;
    s = modify ? (U8 *)SvPV_force_nomg(utf8, ulen) : (U8 *)SvPV_nomg(utf8, ulen);
    if (!SvUTF8(utf8)) {
        if (!modify) {
            SV *tmp = sv_2mortal(newSVpvn((char *)s, ulen));
            if (SvTAINTED(utf8))
                SvTAINTED_on(tmp);
            utf8 = tmp;
        }
        sv_utf8_upgrade_nomg(utf8);
        s = (U8 *)SvPV_nomg(utf8, ulen);
    }
    e = s+ulen;

    /* Optimise for the common case of being called from PerlIOEncode_flush()
       with a standard length buffer. In this case the result SV's buffer is
       only used temporarily, so we can afford to allocate the maximum needed
       and not care about unused space. */
    temp_result = (ulen == PERLIO_BUFSIZ);

    ST(0) = sv_2mortal(result);

    /* Preallocate the result buffer to the maximum possible size.
       ie. assume each UTF8 byte is 1 character.
       Then shrink the result's buffer if necesary at the end. */
    SvGROW(result, ((ulen+1) * usize));

    if (!endian) {
	SV *sv;
	endian = (size == 4) ? 'N' : 'n';
	enc_pack(aTHX_ result,size,endian,BOM_BE);
#if 1
	/* Update endian for next sequence */
	sv = attr("renewed");
	if (SvTRUE(sv)) {
	    (void)hv_store((HV *)SvRV(obj),"endian",6,newSVpv((char *)&endian,1),0);
	}
#endif
    }
    while (s < e && s+UTF8SKIP(s) <= e) {
        STRLEN len;
        AV *msgs = NULL;
        UV ord = utf8n_to_uvchr_msgs(s, e-s, &len, UTF8_DISALLOW_ILLEGAL_INTERCHANGE | UTF8_WARN_ILLEGAL_INTERCHANGE, NULL, &msgs);
        if (msgs) {
            SSize_t i;
            SSize_t len = av_len(msgs)+1;
            sv_2mortal((SV *)msgs);
            for (i = 0; i < len; ++i) {
                SV *sv = *av_fetch(msgs, i, 0);
                HV *hv = (HV *)SvRV(sv);
                SV *message = *hv_fetch(hv, "text", 4, 0);
                U32 categories = SvUVx(*hv_fetch(hv, "warn_categories", 15, 0));
                if (check & ENCODE_DIE_ON_ERR)
                    croak("%" SVf, SVfARG(message));
                if (encode_ckWARN_packed(check, categories))
                    warner(categories, "%" SVf, SVfARG(message));
            }
        }
	if ((size != 4 && invalid_ucs2(ord)) || (ord == 0 && *s != 0)) {
	    if (!issurrogate(ord)) {
		if (ucs2 == -1) {
		    SV *sv = attr("ucs2");
		    ucs2 = SvTRUE(sv);
		}
		if (ucs2 || ord > 0x10FFFF) {
		    if (check & ENCODE_DIE_ON_ERR) {
			croak("%" SVf ":code point \"\\x{%" UVxf "}\" too high",
				  SVfARG(name),ord);
		    }
		    if (encode_ckWARN(check, WARN_NON_UNICODE)) {
			warner(packWARN(WARN_NON_UNICODE),
				  "%" SVf ":code point \"\\x{%" UVxf "}\" too high",
				  SVfARG(name),ord);
		    }
		    enc_pack(aTHX_ result,size,endian,FBCHAR);
		} else if (ord == 0) {
		    enc_pack(aTHX_ result,size,endian,FBCHAR);
		} else {
		    UV hi = ((ord - 0x10000) >> 10)   + 0xD800;
		    UV lo = ((ord - 0x10000) & 0x3FF) + 0xDC00;
		    enc_pack(aTHX_ result,size,endian,hi);
		    enc_pack(aTHX_ result,size,endian,lo);
		}
	    }
	    else {
		/* not supposed to happen */
		enc_pack(aTHX_ result,size,endian,FBCHAR);
	    }
	}
	else {
	    enc_pack(aTHX_ result,size,endian,ord);
	}
	s += len;
    }
    if (s < e) {
	/* UTF-8 partial char happens often on PerlIO.
	   Since this is okay and normal, we do not warn.
	   But this is critical when you choose to LEAVE_SRC
	   in which case we die */
	if (check & (ENCODE_DIE_ON_ERR|ENCODE_LEAVE_SRC)) {
	    Perl_croak(aTHX_ "%" SVf ":partial character is not allowed "
		       "when CHECK = 0x%" UVuf,
		       SVfARG(name), check);
	}
    }
    if (check && !(check & ENCODE_LEAVE_SRC)) {
	if (s < e) {
	    Move(s,SvPVX(utf8),e-s,U8);
	    SvCUR_set(utf8,(e-s));
	}
	else {
	    SvCUR_set(utf8,0);
	}
	*SvEND(utf8) = '\0';
	SvSETMAGIC(utf8);
    }

    if (!temp_result) shrink_buffer(result);
    if (SvTAINTED(utf8)) SvTAINTED_on(result); /* propagate taintedness */

    XSRETURN(1);
}
