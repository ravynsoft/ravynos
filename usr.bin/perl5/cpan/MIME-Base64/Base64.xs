/*

Copyright 1997-2004 Gisle Aas

This library is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.


The tables and some of the code that used to be here was borrowed from
metamail, which comes with this message:

  Copyright (c) 1991 Bell Communications Research, Inc. (Bellcore)

  Permission to use, copy, modify, and distribute this material
  for any purpose and without fee is hereby granted, provided
  that the above copyright notice and this permission notice
  appear in all copies, and that the name of Bellcore not be
  used in advertising or publicity pertaining to this
  material without the specific, prior written permission
  of an authorized representative of Bellcore.	BELLCORE
  MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY
  OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS",
  WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.

*/


#define PERL_NO_GET_CONTEXT     /* we want efficiency */
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#define MAX_LINE  76 /* size of encoded lines */

static const char basis_64[] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define XX      255	/* illegal base64 char */
#define EQ      254	/* padding */
#define INVALID XX

static const unsigned char index_64[256] = {
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,62, XX,XX,XX,63,
    52,53,54,55, 56,57,58,59, 60,61,XX,XX, XX,EQ,XX,XX,
    XX, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
    15,16,17,18, 19,20,21,22, 23,24,25,XX, XX,XX,XX,XX,
    XX,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
    41,42,43,44, 45,46,47,48, 49,50,51,XX, XX,XX,XX,XX,

    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
};

#ifdef SvPVbyte
#   if PERL_REVISION == 5 && PERL_VERSION < 7
       /* SvPVbyte does not work in perl-5.6.1, borrowed version for 5.7.3 */
#       undef SvPVbyte
#       define SvPVbyte(sv, lp) \
          ((SvFLAGS(sv) & (SVf_POK|SVf_UTF8)) == (SVf_POK) \
           ? ((lp = SvCUR(sv)), SvPVX(sv)) : my_sv_2pvbyte(aTHX_ sv, &lp))
       static char *
       my_sv_2pvbyte(pTHX_ SV *sv, STRLEN *lp)
       {   
           sv_utf8_downgrade(sv,0);
           return SvPV(sv,*lp);
       }
#   endif
#else
#   define SvPVbyte SvPV
#endif

#ifndef isXDIGIT
#   define isXDIGIT isxdigit
#endif

#ifndef NATIVE_TO_ASCII
#   define NATIVE_TO_ASCII(ch) (ch)
#endif

MODULE = MIME::Base64		PACKAGE = MIME::Base64

SV*
encode_base64(sv,...)
	SV* sv
	PROTOTYPE: $;$

	PREINIT:
	char *str;     /* string to encode */
	SSize_t len;   /* length of the string */
	const char*eol;/* the end-of-line sequence to use */
	STRLEN eollen; /* length of the EOL sequence */
	char *r;       /* result string */
	STRLEN rlen;   /* length of result string */
	unsigned char c1, c2, c3;
	int chunk;
	U32 had_utf8;

	CODE:
#if PERL_REVISION == 5 && PERL_VERSION >= 6
	had_utf8 = SvUTF8(sv);
	sv_utf8_downgrade(sv, FALSE);
#endif
	str = SvPV(sv, rlen); /* SvPV(sv, len) gives warning for signed len */
	len = (SSize_t)rlen;

	/* set up EOL from the second argument if present, default to "\n" */
	if (items > 1 && SvOK(ST(1))) {
	    eol = SvPV(ST(1), eollen);
	} else {
	    eol = "\n";
	    eollen = 1;
	}

	/* calculate the length of the result */
	rlen = (len+2) / 3 * 4;	 /* encoded bytes */
	if (rlen) {
	    /* add space for EOL */
	    rlen += ((rlen-1) / MAX_LINE + 1) * eollen;
	}

	/* allocate a result buffer */
	RETVAL = newSV(rlen ? rlen : 1);
	SvPOK_on(RETVAL);	
	SvCUR_set(RETVAL, rlen);
	r = SvPVX(RETVAL);

	/* encode */
	for (chunk=0; len > 0; len -= 3, chunk++) {
	    if (chunk == (MAX_LINE/4)) {
		const char *c = eol;
		const char *e = eol + eollen;
		while (c < e)
		    *r++ = *c++;
		chunk = 0;
	    }
	    c1 = *str++;
	    c2 = len > 1 ? *str++ : '\0';
	    *r++ = basis_64[c1>>2];
	    *r++ = basis_64[((c1 & 0x3)<< 4) | ((c2 & 0xF0) >> 4)];
	    if (len > 2) {
		c3 = *str++;
		*r++ = basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)];
		*r++ = basis_64[c3 & 0x3F];
	    } else if (len == 2) {
		*r++ = basis_64[(c2 & 0xF) << 2];
		*r++ = '=';
	    } else { /* len == 1 */
		*r++ = '=';
		*r++ = '=';
	    }
	}
	if (rlen) {
	    /* append eol to the result string */
	    const char *c = eol;
	    const char *e = eol + eollen;
	    while (c < e)
		*r++ = *c++;
	}
	*r = '\0';  /* every SV in perl should be NUL-terminated */
#if PERL_REVISION == 5 && PERL_VERSION >= 6
	if (had_utf8)
	    sv_utf8_upgrade(sv);
#endif

	OUTPUT:
	RETVAL

SV*
decode_base64(sv)
	SV* sv
	PROTOTYPE: $

	PREINIT:
	STRLEN len;
	unsigned char *str = (unsigned char*)SvPV(sv, len);
	unsigned char const* end = str + len;
	char *r;
	unsigned char c[4];

	CODE:
	{
	    /* always enough, but might be too much */
	    STRLEN rlen = len * 3 / 4;
	    RETVAL = newSV(rlen ? rlen : 1);
	}
        SvPOK_on(RETVAL);
        r = SvPVX(RETVAL);

	while (str < end) {
	    int i = 0;
            do {
		unsigned char uc = index_64[NATIVE_TO_ASCII(*str++)];
		if (uc != INVALID)
		    c[i++] = uc;

		if (str == end) {
		    if (i < 4) {
			if (i < 2) goto thats_it;
			if (i == 2) c[2] = EQ;
			c[3] = EQ;
		    }
		    break;
		}
            } while (i < 4);
	
	    if (c[0] == EQ || c[1] == EQ) {
		break;
            }
	    /* printf("c0=%d,c1=%d,c2=%d,c3=%d\n", c[0],c[1],c[2],c[3]);*/

	    *r++ = (c[0] << 2) | ((c[1] & 0x30) >> 4);

	    if (c[2] == EQ)
		break;
	    *r++ = ((c[1] & 0x0F) << 4) | ((c[2] & 0x3C) >> 2);

	    if (c[3] == EQ)
		break;
	    *r++ = ((c[2] & 0x03) << 6) | c[3];
	}

      thats_it:
	SvCUR_set(RETVAL, r - SvPVX(RETVAL));
	*r = '\0';

	OUTPUT:
	RETVAL

int
encoded_base64_length(sv,...)
	SV* sv
	PROTOTYPE: $;$

	PREINIT:
	SSize_t len;   /* length of the string */
	STRLEN eollen; /* length of the EOL sequence */
	U32 had_utf8;

	CODE:
#if PERL_REVISION == 5 && PERL_VERSION >= 6
	had_utf8 = SvUTF8(sv);
	sv_utf8_downgrade(sv, FALSE);
#endif
	len = SvCUR(sv);
#if PERL_REVISION == 5 && PERL_VERSION >= 6
	if (had_utf8)
	    sv_utf8_upgrade(sv);
#endif

	if (items > 1 && SvOK(ST(1))) {
	    eollen = SvCUR(ST(1));
	} else {
	    eollen = 1;
	}

	RETVAL = (len+2) / 3 * 4;	 /* encoded bytes */
	if (RETVAL) {
	    RETVAL += ((RETVAL-1) / MAX_LINE + 1) * eollen;
	}

	OUTPUT:
	RETVAL

int
decoded_base64_length(sv)
	SV* sv
	PROTOTYPE: $

	PREINIT:
	STRLEN len;
	unsigned char *str = (unsigned char*)SvPV(sv, len);
	unsigned char const* end = str + len;
	int i = 0;

	CODE:
	RETVAL = 0;
	while (str < end) {
	    unsigned char uc = index_64[NATIVE_TO_ASCII(*str++)];
	    if (uc == INVALID)
		continue;
	    if (uc == EQ)
	        break;
	    if (i++) {
		RETVAL++;
		if (i == 4)
		    i = 0;
	    }
	}

	OUTPUT:
	RETVAL


MODULE = MIME::Base64		PACKAGE = MIME::QuotedPrint

#ifdef EBCDIC
#define qp_isplain(c) ((c) == '\t' || ((!isprint(c) && (c) != '=')))
#else
#define qp_isplain(c) ((c) == '\t' || (((c) >= ' ' && (c) <= '~') && (c) != '='))
#endif

SV*
encode_qp(sv,...)
	SV* sv
	PROTOTYPE: $;$$

	PREINIT:
	const char *eol;
	STRLEN eol_len;
	int binary;
	STRLEN sv_len;
	STRLEN linelen;
	char *beg;
	char *end;
	char *p;
	char *p_beg;
	STRLEN p_len;
	U32 had_utf8;

	CODE:
#if PERL_REVISION == 5 && PERL_VERSION >= 6
        had_utf8 = SvUTF8(sv);
	sv_utf8_downgrade(sv, FALSE);
#endif
	/* set up EOL from the second argument if present, default to "\n" */
	if (items > 1 && SvOK(ST(1))) {
	    eol = SvPV(ST(1), eol_len);
	} else {
	    eol = "\n";
	    eol_len = 1;
	}

	binary = (items > 2 && SvTRUE(ST(2)));

	beg = SvPV(sv, sv_len);
	end = beg + sv_len;

	RETVAL = newSV(sv_len + 1);
	sv_setpv(RETVAL, "");
	linelen = 0;

	p = beg;
	while (1) {
	    p_beg = p;

	    /* skip past as much plain text as possible */
	    while (p < end && qp_isplain(*p)) {
	        p++;
	    }
	    if (p == end || *p == '\n') {
		/* whitespace at end of line must be encoded */
		while (p > p_beg && (*(p - 1) == '\t' || *(p - 1) == ' '))
		    p--;
	    }

	    p_len = p - p_beg;
	    if (p_len) {
	        /* output plain text (with line breaks) */
	        if (eol_len) {
		    while (p_len > MAX_LINE - 1 - linelen) {
			STRLEN len = MAX_LINE - 1 - linelen;
			sv_catpvn(RETVAL, p_beg, len);
			p_beg += len;
			p_len -= len;
			sv_catpvn(RETVAL, "=", 1);
			sv_catpvn(RETVAL, eol, eol_len);
		        linelen = 0;
		    }
                }
		if (p_len) {
	            sv_catpvn(RETVAL, p_beg, p_len);
	            linelen += p_len;
		}
	    }

	    if (p == end) {
		break;
            }
	    else if (*p == '\n' && eol_len && !binary) {
		if (linelen == 1 && SvCUR(RETVAL) > eol_len + 1 && (SvEND(RETVAL)-eol_len)[-2] == '=') {
		    /* fixup useless soft linebreak */
		    (SvEND(RETVAL)-eol_len)[-2] = SvEND(RETVAL)[-1];
		    SvCUR_set(RETVAL, SvCUR(RETVAL) - 1);
		}
		else {
		    sv_catpvn(RETVAL, eol, eol_len);
		}
		p++;
		linelen = 0;
	    }
	    else {
		/* output escaped char (with line breaks) */
	        assert(p < end);
		if (eol_len && linelen > MAX_LINE - 4 && !(linelen == MAX_LINE - 3 && p + 1 < end && p[1] == '\n' && !binary)) {
		    sv_catpvn(RETVAL, "=", 1);
		    sv_catpvn(RETVAL, eol, eol_len);
		    linelen = 0;
		}
	        sv_catpvf(RETVAL, "=%02X", (unsigned char)*p);
	        p++;
	        linelen += 3;
	    }

	    /* optimize reallocs a bit */
	    if (SvLEN(RETVAL) > 80 && SvLEN(RETVAL) - SvCUR(RETVAL) < 3) {
		STRLEN expected_len = (SvCUR(RETVAL) * sv_len) / (p - beg);
     		SvGROW(RETVAL, expected_len);
	    }
        }

	if (SvCUR(RETVAL) && eol_len && linelen) {
	    sv_catpvn(RETVAL, "=", 1);
	    sv_catpvn(RETVAL, eol, eol_len);
	}
#if PERL_REVISION == 5 && PERL_VERSION >= 6
	if (had_utf8)
	    sv_utf8_upgrade(sv);
#endif

	OUTPUT:
	RETVAL

SV*
decode_qp(sv)
	SV* sv
	PROTOTYPE: $

        PREINIT:
	STRLEN len;
	char *str = SvPVbyte(sv, len);
	char const* end = str + len;
	char *r;
	char *whitespace = 0;

        CODE:
	RETVAL = newSV(len ? len : 1);
        SvPOK_on(RETVAL);
        r = SvPVX(RETVAL);
	while (str < end) {
	    if (*str == ' ' || *str == '\t') {
		if (!whitespace)
		    whitespace = str;
		str++;
	    }
	    else if (*str == '\r' && (str + 1) < end && str[1] == '\n') {
		str++;
	    }
	    else if (*str == '\n') {
		whitespace = 0;
		*r++ = *str++;
	    }
	    else {
		if (whitespace) {
		    while (whitespace < str) {
			*r++ = *whitespace++;
		    }
		    whitespace = 0;
                }
            	if (*str == '=') {
		    if ((str + 2) < end && isXDIGIT(str[1]) && isXDIGIT(str[2])) {
	                char buf[3];
                        str++;
	                buf[0] = *str++;
		        buf[1] = *str++;
	                buf[2] = '\0';
		        *r++ = (char)strtol(buf, 0, 16);
	            }
		    else {
		        /* look for soft line break */
		        char *p = str + 1;
		        while (p < end && (*p == ' ' || *p == '\t'))
		            p++;
		        if (p < end && *p == '\n')
		     	    str = p + 1;
		        else if ((p + 1) < end && *p == '\r' && *(p + 1) == '\n')
		            str = p + 2;
		        else
		            *r++ = *str++; /* give up */
		    }
		}
		else {
		    *r++ = *str++;
		}
	    }
	}
	if (whitespace) {
	    while (whitespace < str) {
		*r++ = *whitespace++;
	    }
        }
	*r = '\0';
	SvCUR_set(RETVAL, r - SvPVX(RETVAL));

        OUTPUT:
	RETVAL


MODULE = MIME::Base64		PACKAGE = MIME::Base64
