
#define PERL_NO_GET_CONTEXT /* we want efficiency */

/* I guese no private function needs pTHX_ and aTHX_ */

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

/* This file is prepared by mkheader */
#include "ucatbl.h"

/* At present, char > 0x10ffff are unaffected without complaint, right? */
#define VALID_UTF_MAX    (0x10ffff)
#define OVER_UTF_MAX(uv) (VALID_UTF_MAX < (uv))

#define MAX_DIV_16 (UV_MAX / 16)

/* Supported Levels */
#define MinLevel	(1)
#define MaxLevel	(4)

/* Shifted weight at 4th level */
#define Shift4Wt	(0xFFFF)

#define VCE_Length	(9)

#define Hangul_SBase  (0xAC00)
#define Hangul_SIni   (0xAC00)
#define Hangul_SFin   (0xD7A3)
#define Hangul_NCount (588)
#define Hangul_TCount (28)
#define Hangul_LBase  (0x1100)
#define Hangul_LIni   (0x1100)
#define Hangul_LFin   (0x1159)
#define Hangul_LFill  (0x115F)
#define Hangul_LEnd   (0x115F) /* Unicode 5.2 */
#define Hangul_VBase  (0x1161)
#define Hangul_VIni   (0x1160) /* from Vowel Filler */
#define Hangul_VFin   (0x11A2)
#define Hangul_VEnd   (0x11A7) /* Unicode 5.2 */
#define Hangul_TBase  (0x11A7) /* from "no-final" codepoint */
#define Hangul_TIni   (0x11A8)
#define Hangul_TFin   (0x11F9)
#define Hangul_TEnd   (0x11FF) /* Unicode 5.2 */
#define HangulL2Ini   (0xA960) /* Unicode 5.2 */
#define HangulL2Fin   (0xA97C) /* Unicode 5.2 */
#define HangulV2Ini   (0xD7B0) /* Unicode 5.2 */
#define HangulV2Fin   (0xD7C6) /* Unicode 5.2 */
#define HangulT2Ini   (0xD7CB) /* Unicode 5.2 */
#define HangulT2Fin   (0xD7FB) /* Unicode 5.2 */

#define CJK_UidIni    (0x4E00)
#define CJK_UidFin    (0x9FA5)
#define CJK_UidF41    (0x9FBB) /* Unicode 4.1 */
#define CJK_UidF51    (0x9FC3) /* Unicode 5.1 */
#define CJK_UidF52    (0x9FCB) /* Unicode 5.2 */
#define CJK_UidF61    (0x9FCC) /* Unicode 6.1 */
#define CJK_UidF80    (0x9FD5) /* Unicode 8.0 */
#define CJK_UidF100   (0x9FEA) /* Unicode 10.0 */
#define CJK_UidF110   (0x9FEF) /* Unicode 11.0 */
#define CJK_UidF130   (0x9FFC) /* Unicode 13.0 */

#define CJK_ExtAIni   (0x3400) /* Unicode 3.0 */
#define CJK_ExtAFin   (0x4DB5) /* Unicode 3.0 */
#define CJK_ExtA130   (0x4DBF) /* Unicode 13.0 */
#define CJK_ExtBIni  (0x20000) /* Unicode 3.1 */
#define CJK_ExtBFin  (0x2A6D6) /* Unicode 3.1 */
#define CJK_ExtB130  (0x2A6DD) /* Unicode 13.0 */
#define CJK_ExtCIni  (0x2A700) /* Unicode 5.2 */
#define CJK_ExtCFin  (0x2B734) /* Unicode 5.2 */
#define CJK_ExtDIni  (0x2B740) /* Unicode 6.0 */
#define CJK_ExtDFin  (0x2B81D) /* Unicode 6.0 */
#define CJK_ExtEIni  (0x2B820) /* Unicode 8.0 */
#define CJK_ExtEFin  (0x2CEA1) /* Unicode 8.0 */
#define CJK_ExtFIni  (0x2CEB0) /* Unicode 10.0 */
#define CJK_ExtFFin  (0x2EBE0) /* Unicode 10.0 */
#define CJK_ExtGIni  (0x30000) /* Unicode 13.0 */
#define CJK_ExtGFin  (0x3134A) /* Unicode 13.0 */

#define CJK_CompIni  (0xFA0E)
#define CJK_CompFin  (0xFA29)
static const STDCHAR UnifiedCompat[] = {
      1,1,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,1,0,0,1,1,1
}; /* E F 0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 */

#define TangIdeoIni  (0x17000) /* Unicode 9.0 */
#define TangIdeoFin  (0x187EC) /* Unicode 9.0 */
#define TangIdeo110  (0x187F1) /* Unicode 11.0 */
#define TangIdeo120  (0x187F7) /* Unicode 12.0 */
#define TangCompIni  (0x18800) /* Unicode 9.0 */
#define TangCompFin  (0x18AF2) /* Unicode 9.0 */
#define TangComp130  (0x18AFF) /* Unicode 13.0 */
#define TangSuppIni  (0x18D00) /* Unicode 13.0 */
#define TangSuppFin  (0x18D08) /* Unicode 13.0 */
#define NushuIni     (0x1B170) /* Unicode 10.0 */
#define NushuFin     (0x1B2FB) /* Unicode 10.0 */
#define KhitanIni    (0x18B00) /* Unicode 13.0 */
#define KhitanFin    (0x18CD5) /* Unicode 13.0 */

#define codeRange(bcode, ecode)	((bcode) <= code && code <= (ecode))

MODULE = Unicode::Collate	PACKAGE = Unicode::Collate

PROTOTYPES: DISABLE

void
_fetch_rest ()
  PREINIT:
    char ** rest;
  PPCODE:
    for (rest = (char **)UCA_rest; *rest; ++rest) {
	XPUSHs(sv_2mortal(newSVpv((char *) *rest, 0)));
    }


void
_fetch_simple (uv)
    UV uv
  PREINIT:
    U8 ***plane, **row;
    U8* result = NULL;
  PPCODE:
    if (!OVER_UTF_MAX(uv)){
	plane = (U8***)UCA_simple[uv >> 16];
	if (plane) {
	    row = plane[(uv >> 8) & 0xff];
	    result = row ? row[uv & 0xff] : NULL;
	}
    }
    if (result) {
	int i;
	int num = (int)*result;
	++result;
	EXTEND(SP, num);
	for (i = 0; i < num; ++i) {
	    PUSHs(sv_2mortal(newSVpvn((char *) result, VCE_Length)));
	    result += VCE_Length;
	}
    } else {
	PUSHs(sv_2mortal(newSViv(0)));
    }

SV*
_ignorable_simple (uv)
    UV uv
  ALIAS:
    _exists_simple = 1
  PREINIT:
    U8 ***plane, **row;
    int num = -1;
    U8* result = NULL;
  CODE:
    if (!OVER_UTF_MAX(uv)){
	plane = (U8***)UCA_simple[uv >> 16];
	if (plane) {
	    row = plane[(uv >> 8) & 0xff];
	    result = row ? row[uv & 0xff] : NULL;
	}
	if (result)
	    num = (int)*result; /* assuming 0 <= num < 128 */
    }

    if (ix)
	RETVAL = boolSV(num >0);
    else
	RETVAL = boolSV(num==0);
  OUTPUT:
    RETVAL


void
_getHexArray (src)
    SV* src
  PREINIT:
    char *s, *e;
    STRLEN byte;
    UV value;
    bool overflowed = FALSE;
    const char *hexdigit;
  PPCODE:
    s = SvPV(src,byte);
    for (e = s + byte; s < e;) {
	hexdigit = strchr((char *) PL_hexdigit, *s++);
	if (! hexdigit)
	    continue;
	value = (hexdigit - PL_hexdigit) & 0xF;
	while (*s) {
	    hexdigit = strchr((char *) PL_hexdigit, *s++);
	    if (! hexdigit)
		break;
	    if (overflowed)
		continue;
	    if (value > MAX_DIV_16) {
		overflowed = TRUE;
		continue;
	    }
	    value = (value << 4) | ((hexdigit - PL_hexdigit) & 0xF);
	}
	XPUSHs(sv_2mortal(newSVuv(overflowed ? UV_MAX : value)));
    }


SV*
_isIllegal (sv)
    SV* sv
  PREINIT:
    UV uv;
  CODE:
    if (!sv || !SvIOK(sv))
	XSRETURN_YES;
    uv = SvUVX(sv);
    RETVAL = boolSV(
	   0x10FFFF < uv                   /* out of range */
	|| ((uv & 0xFFFE) == 0xFFFE)       /* ??FFF[EF] */
	|| (0xD800 <= uv && uv <= 0xDFFF)  /* unpaired surrogates */
	|| (0xFDD0 <= uv && uv <= 0xFDEF)  /* other non-characters */
    );
OUTPUT:
    RETVAL


void
_decompHangul (code)
    UV code
  PREINIT:
    UV sindex, lindex, vindex, tindex;
  PPCODE:
    /* code *must* be in Hangul syllable.
     * Check it before you enter here. */
    sindex =  code - Hangul_SBase;
    lindex =  sindex / Hangul_NCount;
    vindex = (sindex % Hangul_NCount) / Hangul_TCount;
    tindex =  sindex % Hangul_TCount;

    EXTEND(SP, tindex ? 3 : 2);
    PUSHs(sv_2mortal(newSVuv(lindex + Hangul_LBase)));
    PUSHs(sv_2mortal(newSVuv(vindex + Hangul_VBase)));
    if (tindex)
	PUSHs(sv_2mortal(newSVuv(tindex + Hangul_TBase)));


SV*
getHST (code, uca_vers = 0)
    UV code;
    IV uca_vers;
  PREINIT:
    const char * hangtype;
    STRLEN typelen;
  CODE:
    if (codeRange(Hangul_SIni, Hangul_SFin)) {
	if ((code - Hangul_SBase) % Hangul_TCount) {
	    hangtype = "LVT"; typelen = 3;
	} else {
	    hangtype = "LV"; typelen = 2;
	}
    } else if (uca_vers < 20) {
	if (codeRange(Hangul_LIni, Hangul_LFin) || code == Hangul_LFill) {
	    hangtype = "L"; typelen = 1;
	} else if (codeRange(Hangul_VIni, Hangul_VFin)) {
	    hangtype = "V"; typelen = 1;
	} else if (codeRange(Hangul_TIni, Hangul_TFin)) {
	    hangtype = "T"; typelen = 1;
	} else {
	    hangtype = ""; typelen = 0;
	}
    } else {
	if        (codeRange(Hangul_LIni, Hangul_LEnd) ||
		   codeRange(HangulL2Ini, HangulL2Fin)) {
	    hangtype = "L"; typelen = 1;
	} else if (codeRange(Hangul_VIni, Hangul_VEnd) ||
		   codeRange(HangulV2Ini, HangulV2Fin)) {
	    hangtype = "V"; typelen = 1;
	} else if (codeRange(Hangul_TIni, Hangul_TEnd) ||
		   codeRange(HangulT2Ini, HangulT2Fin)) {
	    hangtype = "T"; typelen = 1;
	} else {
	    hangtype = ""; typelen = 0;
	}
    }

    RETVAL = newSVpvn(hangtype, typelen);
OUTPUT:
    RETVAL


void
_derivCE_9 (code)
    UV code
  ALIAS:
    _derivCE_14 = 1
    _derivCE_18 = 2
    _derivCE_20 = 3
    _derivCE_22 = 4
    _derivCE_24 = 5
    _derivCE_32 = 6
    _derivCE_34 = 7
    _derivCE_36 = 8
    _derivCE_38 = 9
    _derivCE_40 = 10
    _derivCE_43 = 11
  PREINIT:
    UV base, aaaa, bbbb;
    U8 a[VCE_Length + 1] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    U8 b[VCE_Length + 1] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    bool basic_unified = 0, tangut = 0, nushu = 0, khitan = 0;
  PPCODE:
    if (codeRange(CJK_UidIni, CJK_CompFin)) {
	if (codeRange(CJK_CompIni, CJK_CompFin))
	    basic_unified = (bool)UnifiedCompat[code - CJK_CompIni];
	else
	    basic_unified = (ix >= 11 ? (code <= CJK_UidF130) :
			     ix >= 9  ? (code <= CJK_UidF110) :
			     ix == 8  ? (code <= CJK_UidF100) :
			     ix >= 6  ? (code <= CJK_UidF80) :
			     ix == 5  ? (code <= CJK_UidF61) :
			     ix >= 3  ? (code <= CJK_UidF52) :
			     ix == 2  ? (code <= CJK_UidF51) :
			     ix == 1  ? (code <= CJK_UidF41) :
				        (code <= CJK_UidFin));
    } else {
	if (ix >= 7) {
	    tangut = (ix >= 11) ? (codeRange(TangIdeoIni, TangIdeo120) ||
				   codeRange(TangCompIni, TangComp130) ||
				   codeRange(TangSuppIni, TangSuppFin)) :
		     (ix == 10) ? (codeRange(TangIdeoIni, TangIdeo120) ||
				   codeRange(TangCompIni, TangCompFin)) :
		     (ix == 9)  ? (codeRange(TangIdeoIni, TangIdeo110) ||
				   codeRange(TangCompIni, TangCompFin)) :
				  (codeRange(TangIdeoIni, TangIdeoFin) ||
				   codeRange(TangCompIni, TangCompFin));
	}
	if (ix >= 8)
	    nushu = (codeRange(NushuIni, NushuFin));
	if (ix >= 11)
	    khitan = (codeRange(KhitanIni, KhitanFin));
    }
    base = tangut
	    ? 0xFB00 :
	   nushu
	    ? 0xFB01 :
	   khitan
	    ? 0xFB02 :
	   basic_unified
	    ? 0xFB40 : /* CJK */
	   ((ix >= 11 ? codeRange(CJK_ExtAIni, CJK_ExtA130)
		      : codeRange(CJK_ExtAIni, CJK_ExtAFin))
		||
	    (ix >= 11 ? codeRange(CJK_ExtBIni, CJK_ExtB130)
		      : codeRange(CJK_ExtBIni, CJK_ExtBFin))
		||
	    (ix >= 3 && codeRange(CJK_ExtCIni, CJK_ExtCFin))
		||
	    (ix >= 4 && codeRange(CJK_ExtDIni, CJK_ExtDFin))
		||
	    (ix >= 6 && codeRange(CJK_ExtEIni, CJK_ExtEFin))
		||
	    (ix >= 8 && codeRange(CJK_ExtFIni, CJK_ExtFFin))
		||
	   (ix >= 11 && codeRange(CJK_ExtGIni, CJK_ExtGFin)))
	    ? 0xFB80   /* CJK ext. */
	    : 0xFBC0;  /* others */
    aaaa = tangut || nushu || khitan ? base : base + (code >> 15);
    bbbb = (tangut ? (code - TangIdeoIni) :
	    nushu  ? (code - NushuIni) :
	    khitan ? (code - KhitanIni) : (code & 0x7FFF)) | 0x8000;
    a[1] = (U8)(aaaa >> 8);
    a[2] = (U8)(aaaa & 0xFF);
    b[1] = (U8)(bbbb >> 8);
    b[2] = (U8)(bbbb & 0xFF);
    a[4] = (U8)(0x20); /* second octet of level 2 */
    a[6] = (U8)(0x02); /* second octet of level 3 */
    a[7] = b[7] = (U8)(code >> 8);
    a[8] = b[8] = (U8)(code & 0xFF);
    EXTEND(SP, 2);
    PUSHs(sv_2mortal(newSVpvn((char *) a, VCE_Length)));
    PUSHs(sv_2mortal(newSVpvn((char *) b, VCE_Length)));


void
_derivCE_8 (code)
    UV code
  PREINIT:
    UV aaaa, bbbb;
    U8 a[VCE_Length + 1] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    U8 b[VCE_Length + 1] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00";
  PPCODE:
    aaaa =  0xFF80 + (code >> 15);
    bbbb = (code & 0x7FFF) | 0x8000;
    a[1] = (U8)(aaaa >> 8);
    a[2] = (U8)(aaaa & 0xFF);
    b[1] = (U8)(bbbb >> 8);
    b[2] = (U8)(bbbb & 0xFF);
    a[4] = (U8)(0x02); /* second octet of level 2 */
    a[6] = (U8)(0x01); /* second octet of level 3 */
    a[7] = b[7] = (U8)(code >> 8);
    a[8] = b[8] = (U8)(code & 0xFF);
    EXTEND(SP, 2);
    PUSHs(sv_2mortal(newSVpvn((char *) a, VCE_Length)));
    PUSHs(sv_2mortal(newSVpvn((char *) b, VCE_Length)));


void
_uideoCE_8 (code)
    UV code
  PREINIT:
    U8 uice[VCE_Length + 1] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00";
  PPCODE:
    uice[1] = uice[7] = (U8)(code >> 8);
    uice[2] = uice[8] = (U8)(code & 0xFF);
    uice[4] = (U8)(0x20); /* second octet of level 2 */
    uice[6] = (U8)(0x02); /* second octet of level 3 */
    PUSHs(sv_2mortal(newSVpvn((char *) uice, VCE_Length)));


SV*
_isUIdeo (code, uca_vers)
    UV code;
    IV uca_vers;
    bool basic_unified = 0;
  CODE:
    /* uca_vers = 0 for _uideoCE_8() */
    if (CJK_UidIni <= code) {
	if (codeRange(CJK_CompIni, CJK_CompFin))
	    basic_unified = (bool)UnifiedCompat[code - CJK_CompIni];
	else
	    basic_unified = (uca_vers >= 43 ? (code <= CJK_UidF130) :
			     uca_vers >= 38 ? (code <= CJK_UidF110) :
			     uca_vers >= 36 ? (code <= CJK_UidF100) :
			     uca_vers >= 32 ? (code <= CJK_UidF80) :
			     uca_vers >= 24 ? (code <= CJK_UidF61) :
			     uca_vers >= 20 ? (code <= CJK_UidF52) :
			     uca_vers >= 18 ? (code <= CJK_UidF51) :
			     uca_vers >= 14 ? (code <= CJK_UidF41) :
					      (code <= CJK_UidFin));
    }
    RETVAL = boolSV(
	(basic_unified)
		||
	(codeRange(CJK_ExtAIni, CJK_ExtAFin))
		||
	(uca_vers >= 43 && codeRange(CJK_ExtAIni, CJK_ExtA130))
		||
	(uca_vers >=  8 && codeRange(CJK_ExtBIni, CJK_ExtBFin))
		||
	(uca_vers >= 43 && codeRange(CJK_ExtBIni, CJK_ExtB130))
		||
	(uca_vers >= 20 && codeRange(CJK_ExtCIni, CJK_ExtCFin))
		||
	(uca_vers >= 22 && codeRange(CJK_ExtDIni, CJK_ExtDFin))
		||
	(uca_vers >= 32 && codeRange(CJK_ExtEIni, CJK_ExtEFin))
		||
	(uca_vers >= 36 && codeRange(CJK_ExtFIni, CJK_ExtFFin))
		||
	(uca_vers >= 43 && codeRange(CJK_ExtGIni, CJK_ExtGFin))
    );
OUTPUT:
    RETVAL


SV*
mk_SortKey (self, buf)
    SV* self;
    SV* buf;
  PREINIT:
    SV *dst, **svp;
    STRLEN dlen, vlen;
    U8 *d, *p, *e, *v, *s[MaxLevel], *eachlevel[MaxLevel];
    AV *bufAV;
    HV *selfHV;
    UV back_flag;
    I32 i, buf_len;
    IV  lv, level, uca_vers;
    bool upper_lower, kata_hira, v2i, last_is_var;
  CODE:
    if (SvROK(self) && SvTYPE(SvRV(self)) == SVt_PVHV)
	selfHV = (HV*)SvRV(self);
    else
	croak("$self is not a HASHREF.");

    if (SvROK(buf) && SvTYPE(SvRV(buf)) == SVt_PVAV)
	bufAV = (AV*)SvRV(buf);
    else
	croak("XSUB, not an ARRAYREF.");

    buf_len = av_len(bufAV);

    if (buf_len < 0) { /* empty: -1 */
	dlen = 2 * (MaxLevel - 1);
	dst = newSV(dlen);
	(void)SvPOK_only(dst);
	d = (U8*)SvPVX(dst);
	while (dlen--)
	    *d++ = '\0';
    } else {
	svp = hv_fetch(selfHV, "level", 5, FALSE);
	level = svp ? SvIV(*svp) : MaxLevel;

	for (lv = 0; lv < level; lv++) {
	    New(0, eachlevel[lv], 2 * (1 + buf_len) + 1, U8);
	    s[lv] = eachlevel[lv];
	}

	svp = hv_fetch(selfHV, "upper_before_lower", 18, FALSE);
	upper_lower = svp ? SvTRUE(*svp) : FALSE;
	svp = hv_fetch(selfHV, "katakana_before_hiragana", 24, FALSE);
	kata_hira = svp ? SvTRUE(*svp) : FALSE;
	svp = hv_fetch(selfHV, "UCA_Version", 11, FALSE);
	uca_vers = SvIV(*svp);
	svp = hv_fetch(selfHV, "variable", 8, FALSE);
	v2i = uca_vers >= 9 && svp /* (vers >= 9) and not (non-ignorable) */
	    ? !(SvCUR(*svp) == 13 && memEQ(SvPVX(*svp), "non-ignorable", 13))
	    : FALSE;

	last_is_var = FALSE;
	for (i = 0; i <= buf_len; i++) {
	    svp = av_fetch(bufAV, i, FALSE);

	    if (svp && SvPOK(*svp))
		v = (U8*)SvPV(*svp, vlen);
	    else
		croak("not a vwt.");

	    if (vlen < VCE_Length) /* ignore short VCE (unexpected) */
		continue;

	    /* "Ignorable (L1, L2) after Variable" since track. v. 9 */
	    if (v2i) {
		if (*v)
		    last_is_var = TRUE;
		else if (v[1] || v[2]) /* non zero primary weight */
		    last_is_var = FALSE;
		else if (last_is_var) /* zero primary weight; skipped */
		    continue;
	    }

	    if (v[5] == 0) { /* tert wt < 256 */
		if (upper_lower) {
		    if (0x8 <= v[6] && v[6] <= 0xC) /* lower */
			v[6] -= 6;
		    else if (0x2 <= v[6] && v[6] <= 0x6) /* upper */
			v[6] += 6;
		    else if (v[6] == 0x1C) /* square upper */
			v[6]++;
		    else if (v[6] == 0x1D) /* square lower */
			v[6]--;
		}
		if (kata_hira) {
		    if (0x0F <= v[6] && v[6] <= 0x13) /* katakana */
			v[6] -= 2;
		    else if (0xD <= v[6] && v[6] <= 0xE) /* hiragana */
			v[6] += 5;
		}
	    }

	    for (lv = 0; lv < level; lv++) {
		if (v[2 * lv + 1] || v[2 * lv + 2]) {
		    *s[lv]++ = v[2 * lv + 1];
		    *s[lv]++ = v[2 * lv + 2];
		}
	    }
	}

	dlen = 2 * (MaxLevel - 1);
	for (lv = 0; lv < level; lv++)
	    dlen += s[lv] - eachlevel[lv];

	dst = newSV(dlen);
	(void)SvPOK_only(dst);
	d = (U8*)SvPVX(dst);

	svp = hv_fetch(selfHV, "backwardsFlag", 13, FALSE);
	back_flag = svp ? SvUV(*svp) : (UV)0;

	for (lv = 0; lv < level; lv++) {
	    if (back_flag & (1 << (lv + 1))) {
		p = s[lv];
		e = eachlevel[lv];
		for ( ; e < p; p -= 2) {
		    *d++ = p[-2];
		    *d++ = p[-1];
		}
	    }
	    else {
		p = eachlevel[lv];
		e = s[lv];
		while (p < e)
		    *d++ = *p++;
	    }
	    if (lv + 1 < MaxLevel) { /* lv + 1 == real level */
		*d++ = '\0';
		*d++ = '\0';
	    }
	}

	for (lv = level; lv < MaxLevel; lv++) {
	    if (lv + 1 < MaxLevel) { /* lv + 1 == real level */
		*d++ = '\0';
		*d++ = '\0';
	    }
	}

	for (lv = 0; lv < level; lv++) {
	    Safefree(eachlevel[lv]);
	}
    }
    *d = '\0';
    SvCUR_set(dst, d - (U8*)SvPVX(dst));
    RETVAL = dst;
OUTPUT:
    RETVAL


SV*
varCE (self, vce)
    SV* self;
    SV* vce;
  PREINIT:
    SV *dst, *vbl, **svp;
    HV *selfHV;
    U8 *a, *v, *d;
    STRLEN alen, vlen;
    bool ig_l2;
    IV uca_vers;
    UV totwt;
  CODE:
    if (SvROK(self) && SvTYPE(SvRV(self)) == SVt_PVHV)
	selfHV = (HV*)SvRV(self);
    else
	croak("$self is not a HASHREF.");

    svp = hv_fetch(selfHV, "ignore_level2", 13, FALSE);
    ig_l2 = svp ? SvTRUE(*svp) : FALSE;

    svp = hv_fetch(selfHV, "variable", 8, FALSE);
    vbl = svp ? *svp : &PL_sv_no;
    a = (U8*)SvPV(vbl, alen);
    v = (U8*)SvPV(vce, vlen);

    dst = newSV(vlen);
    d = (U8*)SvPVX(dst);
    (void)SvPOK_only(dst);
    Copy(v, d, vlen, U8);
    SvCUR_set(dst, vlen);
    d[vlen] = '\0';

    /* primary weight == 0 && secondary weight != 0 */
    if (ig_l2 && !d[1] && !d[2] && (d[3] || d[4])) {
	d[3] = d[4] = d[5] = d[6] = '\0';
    }

    /* variable: checked only the first char and the length,
       trusting checkCollator() and %VariableOK in Perl ... */

    if (vlen >= VCE_Length && *a != 'n') {
	if (*v) {
	    if (*a == 's') { /* shifted or shift-trimmed */
		d[7] = d[1]; /* wt level 1 to 4 */
		d[8] = d[2];
	    } /* else blanked */
	    d[1] = d[2] = d[3] = d[4] = d[5] = d[6] = '\0';
	} else if (*a == 's') { /* shifted or shift-trimmed */
	    totwt = d[1] + d[2] + d[3] + d[4] + d[5] + d[6];
	    if (alen == 7 && totwt != 0) { /* shifted */
		if (d[1] == 0 && d[2] == 1) { /* XXX: CollationAuxiliary-6.2.0 */
		    d[7] = d[1]; /* wt level 1 to 4 */
		    d[8] = d[2];
		} else {
		    svp = hv_fetch(selfHV, "UCA_Version", 11, FALSE);
		    if (!svp)
			croak("Panic: no $self->{UCA_Version} in varCE");
		    uca_vers = SvIV(*svp);

		    /* completely ignorable or the second derived CE */
		    if (uca_vers >= 36 && d[3] + d[4] + d[5] + d[6] == 0) {
			d[7] = d[8] = '\0';
		    } else {
			d[7] = (U8)(Shift4Wt >> 8);
			d[8] = (U8)(Shift4Wt & 0xFF);
		    }
		}
	    } else { /* shift-trimmed or completely ignorable */
		d[7] = d[8] = '\0';
	    }
	} /* else blanked */
    } /* else non-ignorable */
    RETVAL = dst;
OUTPUT:
    RETVAL



SV*
visualizeSortKey (self, key)
    SV * self
    SV * key
  PREINIT:
    HV *selfHV;
    SV **svp, *dst;
    U8 *s, *e, *d;
    STRLEN klen, dlen;
    UV uv;
    IV uca_vers, sep = 0;
    const char *upperhex = "0123456789ABCDEF";
  CODE:
    if (SvROK(self) && SvTYPE(SvRV(self)) == SVt_PVHV)
	selfHV = (HV*)SvRV(self);
    else
	croak("$self is not a HASHREF.");

    svp = hv_fetch(selfHV, "UCA_Version", 11, FALSE);
    if (!svp)
	croak("Panic: no $self->{UCA_Version} in visualizeSortKey");
    uca_vers = SvIV(*svp);

    s = (U8*)SvPV(key, klen);

   /* slightly *longer* than the need, but I'm afraid of miscounting;
      = (klen / 2) * 5 - 1
             # FFFF and ' ' for each 16bit units but ' ' is less by 1;
             # ' ' and '|' for level boundaries including the identical level
       + 2   # '[' and ']'
       + 1   # '\0'
       (a) if klen is odd (not expected), maybe more 5 bytes.
       (b) there is not always the identical level.
   */
    dlen = (klen / 2) * 5 + MaxLevel * 2 + 2;
    dst = newSV(dlen);
    (void)SvPOK_only(dst);
    d = (U8*)SvPVX(dst);

    *d++ = '[';
    for (e = s + klen; s < e; s += 2) {
	uv = (U16)(*s << 8 | s[1]);
	if (uv || sep >= MaxLevel) {
	    if ((d[-1] != '[') && ((9 <= uca_vers) || (d[-1] != '|')))
		*d++ = ' ';
	    *d++ = upperhex[ (s[0] >> 4) & 0xF ];
	    *d++ = upperhex[  s[0]       & 0xF ];
	    *d++ = upperhex[ (s[1] >> 4) & 0xF ];
	    *d++ = upperhex[  s[1]       & 0xF ];
	} else {
	    if ((9 <= uca_vers) && (d[-1] != '['))
		*d++ = ' ';
	    *d++ = '|';
	    ++sep;
	}
    }
    *d++ = ']';
    *d   = '\0';
    SvCUR_set(dst, d - (U8*)SvPVX(dst));
    RETVAL = dst;
OUTPUT:
    RETVAL
