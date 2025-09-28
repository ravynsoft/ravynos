/*    dump.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 *  'You have talked long in your sleep, Frodo,' said Gandalf gently, 'and
 *   it has not been hard for me to read your mind and memory.'
 *
 *     [p.220 of _The Lord of the Rings_, II/i: "Many Meetings"]
 */

/* This file contains utility routines to dump the contents of SV and OP
 * structures, as used by command-line options like -Dt and -Dx, and
 * by Devel::Peek.
 *
 * It also holds the debugging version of the  runops function.

=for apidoc_section $display
 */

#include "EXTERN.h"
#define PERL_IN_DUMP_C
#include "perl.h"
#include "regcomp.h"

static const char* const svtypenames[SVt_LAST] = {
    "NULL",
    "IV",
    "NV",
    "PV",
    "INVLIST",
    "PVIV",
    "PVNV",
    "PVMG",
    "REGEXP",
    "PVGV",
    "PVLV",
    "PVAV",
    "PVHV",
    "PVCV",
    "PVFM",
    "PVIO",
    "PVOBJ",
};


static const char* const svshorttypenames[SVt_LAST] = {
    "UNDEF",
    "IV",
    "NV",
    "PV",
    "INVLST",
    "PVIV",
    "PVNV",
    "PVMG",
    "REGEXP",
    "GV",
    "PVLV",
    "AV",
    "HV",
    "CV",
    "FM",
    "IO",
    "OBJ",
};

struct flag_to_name {
    U32 flag;
    const char *name;
};

static void
S_append_flags(pTHX_ SV *sv, U32 flags, const struct flag_to_name *start,
               const struct flag_to_name *const end)
{
    do {
        if (flags & start->flag)
            sv_catpv(sv, start->name);
    } while (++start < end);
}

#define append_flags(sv, f, flags) \
    S_append_flags(aTHX_ (sv), (f), (flags), C_ARRAY_END(flags))

#define generic_pv_escape(sv,s,len,utf8) pv_escape( (sv), (s), (len), \
                              (len) * (4+UTF8_MAXBYTES) + 1, NULL, \
                              PERL_PV_ESCAPE_NONASCII | PERL_PV_ESCAPE_DWIM \
                              | ((utf8) ? PERL_PV_ESCAPE_UNI : 0) )

#define _pv_display_for_dump(dsv, pv, cur, len, pvlim) \
    _pv_display_flags(aTHX_ dsv, pv, cur, len, pvlim, PERL_PV_ESCAPE_DWIM_ALL_HEX)

/*
=for apidoc pv_escape

Escapes at most the first C<count> chars of C<pv> and puts the results into
C<dsv> such that the size of the escaped string will not exceed C<max> chars
and will not contain any incomplete escape sequences.  The number of bytes
escaped will be returned in the C<STRLEN *escaped> parameter if it is not null.
When the C<dsv> parameter is null no escaping actually occurs, but the number
of bytes that would be escaped were it not null will be calculated.

If flags contains C<PERL_PV_ESCAPE_QUOTE> then any double quotes in the string
will also be escaped.

Normally the SV will be cleared before the escaped string is prepared,
but when C<PERL_PV_ESCAPE_NOCLEAR> is set this will not occur.

If C<PERL_PV_ESCAPE_UNI> is set then the input string is treated as UTF-8.
If C<PERL_PV_ESCAPE_UNI_DETECT> is set then the input string is scanned
using C<is_utf8_string()> to determine if it is UTF-8.

If C<PERL_PV_ESCAPE_ALL> is set then all input chars will be output
using C<\x01F1> style escapes, otherwise if C<PERL_PV_ESCAPE_NONASCII>
is set, only non-ASCII chars will be escaped using this style;
otherwise, only chars above 255 will be so escaped; other non printable
chars will use octal or common escaped patterns like C<\n>. Otherwise,
if C<PERL_PV_ESCAPE_NOBACKSLASH> then all chars below 255 will be
treated as printable and will be output as literals. The
C<PERL_PV_ESCAPE_NON_WC> modifies the previous rules to cause word
chars, unicode or otherwise, to be output as literals, note this uses
the *unicode* rules for deciding on word characters.

If C<PERL_PV_ESCAPE_FIRSTCHAR> is set then only the first char of the
string will be escaped, regardless of max. If the output is to be in
hex, then it will be returned as a plain hex sequence. Thus the output
will either be a single char, an octal escape sequence, a special escape
like C<\n> or a hex value.

If C<PERL_PV_ESCAPE_RE> is set then the escape char used will be a
C<"%"> and not a C<"\\">. This is because regexes very often contain
backslashed sequences, whereas C<"%"> is not a particularly common
character in patterns.

Returns a pointer to the escaped text as held by C<dsv>.

=for apidoc Amnh||PERL_PV_ESCAPE_ALL
=for apidoc Amnh||PERL_PV_ESCAPE_FIRSTCHAR
=for apidoc Amnh||PERL_PV_ESCAPE_NOBACKSLASH
=for apidoc Amnh||PERL_PV_ESCAPE_NOCLEAR
=for apidoc Amnh||PERL_PV_ESCAPE_NONASCII
=for apidoc Amnh||PERL_PV_ESCAPE_QUOTE
=for apidoc Amnh||PERL_PV_ESCAPE_RE
=for apidoc Amnh||PERL_PV_ESCAPE_UNI
=for apidoc Amnh||PERL_PV_ESCAPE_UNI_DETECT
=for apidoc Amnh||PERL_PV_ESCAPE_NON_WC

=cut

Unused or not for public use
=for apidoc Cmnh||PERL_PV_PRETTY_REGPROP
=for apidoc Cmnh||PERL_PV_PRETTY_DUMP
=for apidoc Cmnh||PERL_PV_PRETTY_NOCLEAR

=cut
*/
#define PV_ESCAPE_OCTBUFSIZE 32

#define PV_BYTE_HEX_UC  "x%02" UVXf
#define PV_BYTE_HEX_LC  "x%02" UVxf

char *
Perl_pv_escape( pTHX_ SV *dsv, char const * const str, 
                const STRLEN count, STRLEN max,
                STRLEN * const escaped, U32 flags )
{

    bool use_uc_hex = false;
    if (flags & PERL_PV_ESCAPE_DWIM_ALL_HEX) {
        use_uc_hex = true;
        flags |= PERL_PV_ESCAPE_DWIM;
    }

    const char esc = (flags & PERL_PV_ESCAPE_RE) ? '%' : '\\';
    const char dq = (flags & PERL_PV_ESCAPE_QUOTE) ? '"' : esc;
    const char *qs;
    const char *qe;

    char octbuf[PV_ESCAPE_OCTBUFSIZE] = "%123456789ABCDF";
    STRLEN wrote = 0;    /* chars written so far */
    STRLEN chsize = 0;   /* size of data to be written */
    STRLEN readsize = 1; /* size of data just read */
    bool isuni= (flags & PERL_PV_ESCAPE_UNI)
                ? TRUE : FALSE; /* is this UTF-8 */
    const char *pv  = str;
    const char * const end = pv + count; /* end of string */
    const char *restart = NULL;
    STRLEN extra_len = 0;
    STRLEN tail = 0;
    if ((flags & PERL_PV_ESCAPE_TRUNC_MIDDLE) && max > 3) {
        if (flags & PERL_PV_ESCAPE_QUOTE) {
            qs = qe = "\"";
            extra_len = 5;
        } else if (flags & PERL_PV_PRETTY_LTGT) {
            qs = "<";
            qe = ">";
            extra_len = 5;
        } else {
            qs = qe = "";
            extra_len = 3;
        }
        tail = max / 2;
        restart = isuni ? (char *)utf8_hop_back((U8*)end,-tail,(U8*)pv) : end - tail;
        if (restart > pv) {
            max -= tail;
        } else {
            tail = 0;
            restart = NULL;
        }
    }
    else {
        qs = qe = "";
    }

    octbuf[0] = esc;

    PERL_ARGS_ASSERT_PV_ESCAPE;

    if (dsv && !(flags & PERL_PV_ESCAPE_NOCLEAR)) {
            /* This won't alter the UTF-8 flag */
            SvPVCLEAR(dsv);
    }
    
    if ((flags & PERL_PV_ESCAPE_UNI_DETECT) && is_utf8_string((U8*)pv, count))
        isuni = 1;
    
    for ( ; pv < end ; pv += readsize ) {
        const UV u= (isuni) ? utf8_to_uvchr_buf((U8*)pv, (U8*) end, &readsize) : (U8)*pv;
        const U8 c = (U8)u;
        const char *source_buf = octbuf;
        
        if ( ( u > 255 )
          || (flags & PERL_PV_ESCAPE_ALL)
          || (( ! isASCII(u) ) && (flags & (PERL_PV_ESCAPE_NONASCII|PERL_PV_ESCAPE_DWIM))))
        {
            if (flags & PERL_PV_ESCAPE_FIRSTCHAR) 
                chsize = my_snprintf( octbuf, PV_ESCAPE_OCTBUFSIZE, 
                                      "%" UVxf, u);
            else
            if ((flags & PERL_PV_ESCAPE_NON_WC) && isWORDCHAR_uvchr(u)) {
                chsize = readsize;
                source_buf = pv;
            }
            else
                chsize = my_snprintf( octbuf, PV_ESCAPE_OCTBUFSIZE, 
                                      ((flags & PERL_PV_ESCAPE_DWIM) && !isuni)
                                      ? ( use_uc_hex ? ("%c" PV_BYTE_HEX_UC) : ("%c" PV_BYTE_HEX_LC) )
                                      : "%cx{%02" UVxf "}", esc, u);

        } else if (flags & PERL_PV_ESCAPE_NOBACKSLASH) {
            chsize = 1;            
        } else {         
            if ( (c == dq) || (c == esc) || !isPRINT(c) ) {
                chsize = 2;
                switch (c) {
                
                case '\\' : /* FALLTHROUGH */
                case '%'  : if ( c == esc )  {
                                octbuf[1] = esc;  
                            } else {
                                chsize = 1;
                            }
                            break;
                case '\v' : octbuf[1] = 'v';  break;
                case '\t' : octbuf[1] = 't';  break;
                case '\r' : octbuf[1] = 'r';  break;
                case '\n' : octbuf[1] = 'n';  break;
                case '\f' : octbuf[1] = 'f';  break;
                case '"'  : 
                        if ( dq == '"' ) 
                                octbuf[1] = '"';
                        else 
                            chsize = 1;
                        break;
                default:
                    if ( (flags & PERL_PV_ESCAPE_DWIM_ALL_HEX) || ((flags & PERL_PV_ESCAPE_DWIM) && c != '\0') ) {
                        chsize = my_snprintf( octbuf, PV_ESCAPE_OCTBUFSIZE,
                                      isuni ? "%cx{%02" UVxf "}" : ( use_uc_hex ? ("%c" PV_BYTE_HEX_UC) : ("%c" PV_BYTE_HEX_LC) ),
                                      esc, u);
                    }
                    else if ((pv+readsize < end) && isDIGIT((U8)*(pv+readsize)))
                        chsize = my_snprintf( octbuf, PV_ESCAPE_OCTBUFSIZE,
                                                  "%c%03o", esc, c);
                    else
                        chsize = my_snprintf( octbuf, PV_ESCAPE_OCTBUFSIZE,
                                                  "%c%o", esc, c);
                }
            } else {
                chsize = 1;
            }
        }
        if (max && (wrote + chsize > max)) {
            if (restart) {
                /* this only happens with PERL_PV_ESCAPE_TRUNC_MIDDLE */
                if (dsv)
                    Perl_sv_catpvf( aTHX_ dsv,"%s...%s", qe, qs);
                wrote += extra_len;
                pv = restart;
                max = tail;
                wrote = tail = 0;
                restart = NULL;
            } else {
                break;
            }
        } else if (chsize > 1) {
            if (dsv)
                sv_catpvn(dsv, source_buf, chsize);
            wrote += chsize;
        } else {
            /* If PERL_PV_ESCAPE_NOBACKSLASH is set then non-ASCII bytes
               can be appended raw to the dsv. If dsv happens to be
               UTF-8 then we need catpvf to upgrade them for us.
               Or add a new API call sv_catpvc(). Think about that name, and
               how to keep it clear that it's unlike the s of catpvs, which is
               really an array of octets, not a string.  */
            if (dsv)
                Perl_sv_catpvf( aTHX_ dsv, "%c", c);
            wrote++;
        }
        if ( flags & PERL_PV_ESCAPE_FIRSTCHAR ) 
            break;
    }
    if (escaped != NULL)
        *escaped= pv - str;
    return dsv ? SvPVX(dsv) : NULL;
}
/*
=for apidoc pv_pretty

Converts a string into something presentable, handling escaping via
C<pv_escape()> and supporting quoting and ellipses.

If the C<PERL_PV_PRETTY_QUOTE> flag is set then the result will be
double quoted with any double quotes in the string escaped.  Otherwise
if the C<PERL_PV_PRETTY_LTGT> flag is set then the result be wrapped in
angle brackets. 

If the C<PERL_PV_PRETTY_ELLIPSES> flag is set and not all characters in
string were output then an ellipsis C<...> will be appended to the
string.  Note that this happens AFTER it has been quoted.

If C<start_color> is non-null then it will be inserted after the opening
quote (if there is one) but before the escaped text.  If C<end_color>
is non-null then it will be inserted after the escaped text but before
any quotes or ellipses.

Returns a pointer to the prettified text as held by C<dsv>.

=for apidoc Amnh||PERL_PV_PRETTY_QUOTE
=for apidoc Amnh||PERL_PV_PRETTY_LTGT
=for apidoc Amnh||PERL_PV_PRETTY_ELLIPSES

=cut           
*/

char *
Perl_pv_pretty( pTHX_ SV *dsv, char const * const str, const STRLEN count, 
  const STRLEN max, char const * const start_color, char const * const end_color, 
  const U32 flags ) 
{
    const U8 *quotes = (U8*)((flags & PERL_PV_PRETTY_QUOTE) ? "\"\"" :
                             (flags & PERL_PV_PRETTY_LTGT)  ? "<>" : NULL);
    STRLEN escaped;
    STRLEN max_adjust= 0;
    STRLEN orig_cur;
 
    PERL_ARGS_ASSERT_PV_PRETTY;
   
    if (!(flags & PERL_PV_PRETTY_NOCLEAR)) {
        /* This won't alter the UTF-8 flag */
        SvPVCLEAR(dsv);
    }
    orig_cur= SvCUR(dsv);

    if ( quotes )
        Perl_sv_catpvf(aTHX_ dsv, "%c", quotes[0]);
        
    if ( start_color != NULL ) 
        sv_catpv(dsv, start_color);

    if ((flags & PERL_PV_PRETTY_EXACTSIZE)) {
        if (quotes)
            max_adjust += 2;
        assert(max > max_adjust);
        pv_escape( NULL, str, count, max - max_adjust, &escaped, flags );
        if ( (flags & PERL_PV_PRETTY_ELLIPSES) && ( escaped < count ) )
            max_adjust += 3;
        assert(max > max_adjust);
    }

    pv_escape( dsv, str, count, max - max_adjust, &escaped, flags | PERL_PV_ESCAPE_NOCLEAR );

    if ( end_color != NULL ) 
        sv_catpv(dsv, end_color);

    if ( quotes )
        Perl_sv_catpvf(aTHX_ dsv, "%c", quotes[1]);
    
    if ( (flags & PERL_PV_PRETTY_ELLIPSES) && ( escaped < count ) )
            sv_catpvs(dsv, "...");

    if ((flags & PERL_PV_PRETTY_EXACTSIZE)) {
        while( SvCUR(dsv) - orig_cur < max )
            sv_catpvs(dsv," ");
    }
 
    return SvPVX(dsv);
}

STATIC char *
_pv_display_flags(pTHX_ SV *dsv, const char *pv, STRLEN cur, STRLEN len, STRLEN pvlim, I32 pretty_flags)
{
    PERL_ARGS_ASSERT_PV_DISPLAY;

    pv_pretty( dsv, pv, cur, pvlim, NULL, NULL, PERL_PV_PRETTY_DUMP | pretty_flags );
    if (len > cur && pv[cur] == '\0')
            sv_catpvs( dsv, "\\0");
    return SvPVX(dsv);
}

/*
=for apidoc pv_display

Similar to

  pv_escape(dsv,pv,cur,pvlim,PERL_PV_ESCAPE_QUOTE);

except that an additional "\0" will be appended to the string when
len > cur and pv[cur] is "\0".

Note that the final string may be up to 7 chars longer than pvlim.

=cut
*/

char *
Perl_pv_display(pTHX_ SV *dsv, const char *pv, STRLEN cur, STRLEN len, STRLEN pvlim)
{
    return _pv_display_flags(aTHX_ dsv, pv, cur, len, pvlim, 0);
}

/*
=for apidoc sv_peek

Implements C<SvPEEK>

=cut
*/

char *
Perl_sv_peek(pTHX_ SV *sv)
{
    SV * const t = sv_newmortal();
    int unref = 0;
    U32 type;

    SvPVCLEAR(t);
  retry:
    if (!sv) {
        sv_catpvs(t, "VOID");
        goto finish;
    }
    else if (sv == (const SV *)0x55555555 || ((char)SvTYPE(sv)) == 'U') {
        /* detect data corruption under memory poisoning */
        sv_catpvs(t, "WILD");
        goto finish;
    }
    else if (  sv == &PL_sv_undef || sv == &PL_sv_no || sv == &PL_sv_yes
            || sv == &PL_sv_zero || sv == &PL_sv_placeholder)
    {
        if (sv == &PL_sv_undef) {
            sv_catpvs(t, "SV_UNDEF");
            if (!(SvFLAGS(sv) & (SVf_OK|SVf_OOK|SVs_OBJECT|
                                 SVs_GMG|SVs_SMG|SVs_RMG)) &&
                SvREADONLY(sv))
                goto finish;
        }
        else if (sv == &PL_sv_no) {
            sv_catpvs(t, "SV_NO");
            if (!(SvFLAGS(sv) & (SVf_ROK|SVf_OOK|SVs_OBJECT|
                                 SVs_GMG|SVs_SMG|SVs_RMG)) &&
                !(~SvFLAGS(sv) & (SVf_POK|SVf_NOK|SVf_READONLY|
                                  SVp_POK|SVp_NOK)) &&
                SvCUR(sv) == 0 &&
                SvNVX(sv) == 0.0)
                goto finish;
        }
        else if (sv == &PL_sv_yes) {
            sv_catpvs(t, "SV_YES");
            if (!(SvFLAGS(sv) & (SVf_ROK|SVf_OOK|SVs_OBJECT|
                                 SVs_GMG|SVs_SMG|SVs_RMG)) &&
                !(~SvFLAGS(sv) & (SVf_POK|SVf_NOK|SVf_READONLY|
                                  SVp_POK|SVp_NOK)) &&
                SvCUR(sv) == 1 &&
                SvPVX_const(sv) && *SvPVX_const(sv) == '1' &&
                SvNVX(sv) == 1.0)
                goto finish;
        }
        else if (sv == &PL_sv_zero) {
            sv_catpvs(t, "SV_ZERO");
            if (!(SvFLAGS(sv) & (SVf_ROK|SVf_OOK|SVs_OBJECT|
                                 SVs_GMG|SVs_SMG|SVs_RMG)) &&
                !(~SvFLAGS(sv) & (SVf_POK|SVf_NOK|SVf_READONLY|
                                  SVp_POK|SVp_NOK)) &&
                SvCUR(sv) == 1 &&
                SvPVX_const(sv) && *SvPVX_const(sv) == '0' &&
                SvNVX(sv) == 0.0)
                goto finish;
        }
        else {
            sv_catpvs(t, "SV_PLACEHOLDER");
            if (!(SvFLAGS(sv) & (SVf_OK|SVf_OOK|SVs_OBJECT|
                                 SVs_GMG|SVs_SMG|SVs_RMG)) &&
                SvREADONLY(sv))
                goto finish;
        }
        sv_catpvs(t, ":");
    }
    else if (SvREFCNT(sv) == 0) {
        sv_catpvs(t, "(");
        unref++;
    }
    else if (DEBUG_R_TEST_) {
        int is_tmp = 0;
        SSize_t ix;
        /* is this SV on the tmps stack? */
        for (ix=PL_tmps_ix; ix>=0; ix--) {
            if (PL_tmps_stack[ix] == sv) {
                is_tmp = 1;
                break;
            }
        }
        if (is_tmp || SvREFCNT(sv) > 1 || SvPADTMP(sv)) {
            Perl_sv_catpvf(aTHX_ t, "<");
            if (SvREFCNT(sv) > 1)
                Perl_sv_catpvf(aTHX_ t, "%" UVuf, (UV)SvREFCNT(sv));
            if (SvPADTMP(sv))
                Perl_sv_catpvf(aTHX_ t, "%s",  "P");
            if (is_tmp)
                Perl_sv_catpvf(aTHX_ t, "%s", SvTEMP(t) ? "T" : "t");
            Perl_sv_catpvf(aTHX_ t, ">");
        }
    }

    if (SvROK(sv)) {
        sv_catpvs(t, "\\");
        if (SvCUR(t) + unref > 10) {
            SvCUR_set(t, unref + 3);
            *SvEND(t) = '\0';
            sv_catpvs(t, "...");
            goto finish;
        }
        sv = SvRV(sv);
        goto retry;
    }
    type = SvTYPE(sv);
    if (type == SVt_PVCV) {
        SV * const tmp = newSVpvs_flags("", SVs_TEMP);
        GV* gvcv = CvGV(sv);
        Perl_sv_catpvf(aTHX_ t, "CV(%s)", gvcv
                       ? generic_pv_escape( tmp, GvNAME(gvcv), GvNAMELEN(gvcv), GvNAMEUTF8(gvcv))
                       : "");
        goto finish;
    } else if (type < SVt_LAST) {
        sv_catpv(t, svshorttypenames[type]);

        if (type == SVt_NULL)
            goto finish;
    } else {
        sv_catpvs(t, "FREED");
        goto finish;
    }

    if (SvPOKp(sv)) {
        if (!SvPVX_const(sv))
            sv_catpvs(t, "(null)");
        else {
            SV * const tmp = newSVpvs("");
            sv_catpvs(t, "(");
            if (SvOOK(sv)) {
                STRLEN delta;
                SvOOK_offset(sv, delta);
                Perl_sv_catpvf(aTHX_ t, "[%s]", pv_display(tmp, SvPVX_const(sv)-delta, delta, 0, 127));
            }
            Perl_sv_catpvf(aTHX_ t, "%s)", pv_display(tmp, SvPVX_const(sv), SvCUR(sv), SvLEN(sv), 127));
            if (SvUTF8(sv))
                Perl_sv_catpvf(aTHX_ t, " [UTF8 \"%s\"]",
                               sv_uni_display(tmp, sv, 6 * SvCUR(sv),
                                              UNI_DISPLAY_QQ));
            SvREFCNT_dec_NN(tmp);
        }
    }
    else if (SvNOKp(sv)) {
        DECLARATION_FOR_LC_NUMERIC_MANIPULATION;
        STORE_LC_NUMERIC_SET_STANDARD();
        Perl_sv_catpvf(aTHX_ t, "(%" NVgf ")",SvNVX(sv));
        RESTORE_LC_NUMERIC();
    }
    else if (SvIOKp(sv)) {
        if (SvIsUV(sv))
            Perl_sv_catpvf(aTHX_ t, "(%" UVuf ")", (UV)SvUVX(sv));
        else
            Perl_sv_catpvf(aTHX_ t, "(%" IVdf ")", (IV)SvIVX(sv));
    }
    else
        sv_catpvs(t, "()");

  finish:
    while (unref--)
        sv_catpvs(t, ")");
    if (TAINTING_get && sv && SvTAINTED(sv))
        sv_catpvs(t, " [tainted]");
    return SvPV_nolen(t);
}

void
Perl_dump_indent(pTHX_ I32 level, PerlIO *file, const char* pat, ...)
{
    va_list args;
    PERL_ARGS_ASSERT_DUMP_INDENT;
    va_start(args, pat);
    dump_vindent(level, file, pat, &args);
    va_end(args);
}

void
Perl_dump_vindent(pTHX_ I32 level, PerlIO *file, const char* pat, va_list *args)
{
    PERL_ARGS_ASSERT_DUMP_VINDENT;
    PerlIO_printf(file, "%*s", (int)(level*PL_dumpindent), "");
    PerlIO_vprintf(file, pat, *args);
}


/* Like Perl_dump_indent(), but specifically for ops: adds a vertical bar
 * for each indent level as appropriate.
 *
 * bar contains bits indicating which indent columns should have a
 * vertical bar displayed. Bit 0 is the RH-most column. If there are more
 * levels than bits in bar, then the first few indents are displayed
 * without a bar.
 *
 * The start of a new op is signalled by passing a value for level which
 * has been negated and offset by 1 (so that level 0 is passed as -1 and
 * can thus be distinguished from -0); in this case, emit a suitably
 * indented blank line, then on the next line, display the op's sequence
 * number, and make the final indent an '+----'.
 *
 * e.g.
 *
 *      |   FOO       # level = 1,   bar = 0b1
 *      |   |         # level =-2-1, bar = 0b11
 * 1234 |   +---BAR
 *      |       BAZ   # level = 2,   bar = 0b10
 */

static void
S_opdump_indent(pTHX_ const OP *o, I32 level, UV bar, PerlIO *file,
                const char* pat, ...)
{
    va_list args;
    I32 i;
    bool newop = (level < 0);

    va_start(args, pat);

    /* start displaying a new op? */
    if (newop) {
        UV seq = sequence_num(o);

        level = -level - 1;

        /* output preceding blank line */
        PerlIO_puts(file, "     ");
        for (i = level-1; i >= 0; i--)
            PerlIO_puts(file,  (   i == 0
                                || (i < UVSIZE*8 && (bar & ((UV)1 << i)))
                               )
                                    ?  "|   " : "    ");
        PerlIO_puts(file, "\n");

        /* output sequence number */
        if (seq)
            PerlIO_printf(file, "%-4" UVuf " ", seq);
        else
            PerlIO_puts(file, "???? ");

    }
    else
        PerlIO_printf(file, "     ");

    for (i = level-1; i >= 0; i--)
            PerlIO_puts(file,
                  (i == 0 && newop) ? "+--"
                : (bar & (1 << i))  ? "|   "
                :                     "    ");
    PerlIO_vprintf(file, pat, args);
    va_end(args);
}


/* display a link field (e.g. op_next) in the format
 *     ====> sequence_number [opname 0x123456]
 */

static void
S_opdump_link(pTHX_ const OP *base, const OP *o, PerlIO *file)
{
    PerlIO_puts(file, " ===> ");
    if (o == base)
        PerlIO_puts(file, "[SELF]\n");
    else if (o)
        PerlIO_printf(file, "%" UVuf " [%s 0x%" UVxf "]\n",
            sequence_num(o), OP_NAME(o), PTR2UV(o));
    else
        PerlIO_puts(file, "[0x0]\n");
}

/*
=for apidoc_section $debugging
=for apidoc dump_all

Dumps the entire optree of the current program starting at C<PL_main_root> to 
C<STDERR>.  Also dumps the optrees for all visible subroutines in
C<PL_defstash>.

=cut
*/

void
Perl_dump_all(pTHX)
{
    dump_all_perl(FALSE);
}

void
Perl_dump_all_perl(pTHX_ bool justperl)
{
    PerlIO_setlinebuf(Perl_debug_log);
    if (PL_main_root)
        op_dump(PL_main_root);
    dump_packsubs_perl(PL_defstash, justperl);
}

/*
=for apidoc dump_packsubs

Dumps the optrees for all visible subroutines in C<stash>.

=cut
*/

void
Perl_dump_packsubs(pTHX_ const HV *stash)
{
    PERL_ARGS_ASSERT_DUMP_PACKSUBS;
    dump_packsubs_perl(stash, FALSE);
}

void
Perl_dump_packsubs_perl(pTHX_ const HV *stash, bool justperl)
{
    I32	i;

    PERL_ARGS_ASSERT_DUMP_PACKSUBS_PERL;

    if (!HvTOTALKEYS(stash))
        return;
    for (i = 0; i <= (I32) HvMAX(stash); i++) {
        const HE *entry;
        for (entry = HvARRAY(stash)[i]; entry; entry = HeNEXT(entry)) {
            GV * gv = (GV *)HeVAL(entry);
            if (SvROK(gv) && SvTYPE(SvRV(gv)) == SVt_PVCV)
                /* unfake a fake GV */
                (void)CvGV(SvRV(gv));
            if (SvTYPE(gv) != SVt_PVGV || !GvGP(gv))
                continue;
            if (GvCVu(gv))
                dump_sub_perl(gv, justperl);
            if (GvFORM(gv))
                dump_form(gv);
            if (HeKEY(entry)[HeKLEN(entry)-1] == ':') {
                const HV * const hv = GvHV(gv);
                if (hv && (hv != PL_defstash))
                    dump_packsubs_perl(hv, justperl); /* nested package */
            }
        }
    }
}

void
Perl_dump_sub(pTHX_ const GV *gv)
{
    PERL_ARGS_ASSERT_DUMP_SUB;
    dump_sub_perl(gv, FALSE);
}

void
Perl_dump_sub_perl(pTHX_ const GV *gv, bool justperl)
{
    CV *cv;

    PERL_ARGS_ASSERT_DUMP_SUB_PERL;

    cv = isGV_with_GP(gv) ? GvCV(gv) : CV_FROM_REF((SV*)gv);
    if (justperl && (CvISXSUB(cv) || !CvROOT(cv)))
        return;

    if (isGV_with_GP(gv)) {
        SV * const namesv = newSVpvs_flags("", SVs_TEMP);
        SV *escsv = newSVpvs_flags("", SVs_TEMP);
        const char *namepv;
        STRLEN namelen;
        gv_fullname3(namesv, gv, NULL);
        namepv = SvPV_const(namesv, namelen);
        Perl_dump_indent(aTHX_ 0, Perl_debug_log, "\nSUB %s = ",
                     generic_pv_escape(escsv, namepv, namelen, SvUTF8(namesv)));
    } else {
        Perl_dump_indent(aTHX_ 0, Perl_debug_log, "\nSUB = ");
    }
    if (CvISXSUB(cv))
        Perl_dump_indent(aTHX_ 0, Perl_debug_log, "(xsub 0x%" UVxf " %d)\n",
            PTR2UV(CvXSUB(cv)),
            (int)CvXSUBANY(cv).any_i32);
    else if (CvROOT(cv))
        op_dump(CvROOT(cv));
    else
        Perl_dump_indent(aTHX_ 0, Perl_debug_log, "<undef>\n");
}

/*
=for apidoc dump_form

Dumps the contents of the format contained in the GV C<gv> to C<STDERR>, or a
message that one doesn't exist.

=cut
*/

void
Perl_dump_form(pTHX_ const GV *gv)
{
    SV * const sv = sv_newmortal();

    PERL_ARGS_ASSERT_DUMP_FORM;

    gv_fullname3(sv, gv, NULL);
    Perl_dump_indent(aTHX_ 0, Perl_debug_log, "\nFORMAT %s = ", SvPVX_const(sv));
    if (CvROOT(GvFORM(gv)))
        op_dump(CvROOT(GvFORM(gv)));
    else
        Perl_dump_indent(aTHX_ 0, Perl_debug_log, "<undef>\n");
}

void
Perl_dump_eval(pTHX)
{
    op_dump(PL_eval_root);
}


/* returns a temp SV displaying the name of a GV. Handles the case where
 * a GV is in fact a ref to a CV */

static SV *
S_gv_display(pTHX_ GV *gv)
{
    SV * const name = newSVpvs_flags("", SVs_TEMP);
    if (gv) {
        SV * const raw = newSVpvs_flags("", SVs_TEMP);
        STRLEN len;
        const char * rawpv;

        if (isGV_with_GP(gv))
            gv_fullname3(raw, gv, NULL);
        else {
            Perl_sv_catpvf(aTHX_ raw, "cv ref: %s",
                    SvPV_nolen_const(cv_name(CV_FROM_REF((SV*)gv), name, 0)));
        }
        rawpv = SvPV_const(raw, len);
        generic_pv_escape(name, rawpv, len, SvUTF8(raw));
    }
    else
        sv_catpvs(name, "(NULL)");

    return name;
}



/* forward decl */
static void
S_do_op_dump_bar(pTHX_ I32 level, UV bar, PerlIO *file, const OP *o);


static void
S_do_pmop_dump_bar(pTHX_ I32 level, UV bar, PerlIO *file, const PMOP *pm)
{
    UV kidbar;

    if (!pm)
        return;

    kidbar = ((bar << 1) | cBOOL(pm->op_flags & OPf_KIDS)) << 1;

    if (PM_GETRE(pm)) {
        char ch = (pm->op_pmflags & PMf_ONCE) ? '?' : '/';
        S_opdump_indent(aTHX_ (OP*)pm, level, bar, file, "PMf_PRE %c%.*s%c\n",
             ch,(int)RX_PRELEN(PM_GETRE(pm)), RX_PRECOMP(PM_GETRE(pm)), ch);
    }
    else
        S_opdump_indent(aTHX_ (OP*)pm, level, bar, file, "PMf_PRE (RUNTIME)\n");

    if (pm->op_pmflags || PM_GETRE(pm)) {
        SV * const tmpsv = pm_description(pm);
        S_opdump_indent(aTHX_ (OP*)pm, level, bar, file, "PMFLAGS = (%s)\n",
                        SvCUR(tmpsv) ? SvPVX_const(tmpsv) + 1 : "");
        SvREFCNT_dec_NN(tmpsv);
    }

    if (pm->op_type == OP_SPLIT)
        S_opdump_indent(aTHX_ (OP*)pm, level, bar, file,
                    "TARGOFF/GV = 0x%" UVxf "\n",
                    PTR2UV(pm->op_pmreplrootu.op_pmtargetgv));
    else {
        if (pm->op_pmreplrootu.op_pmreplroot) {
            S_opdump_indent(aTHX_ (OP*)pm, level, bar, file, "PMf_REPL =\n");
            S_do_op_dump_bar(aTHX_ level + 2,
                (kidbar|cBOOL(OpHAS_SIBLING(pm->op_pmreplrootu.op_pmreplroot))),
                file, pm->op_pmreplrootu.op_pmreplroot);
        }
    }

    if (pm->op_code_list) {
        if (pm->op_pmflags & PMf_CODELIST_PRIVATE) {
            S_opdump_indent(aTHX_ (OP*)pm, level, bar, file, "CODE_LIST =\n");
            S_do_op_dump_bar(aTHX_ level + 2,
                            (kidbar | cBOOL(OpHAS_SIBLING(pm->op_code_list))),
                            file, pm->op_code_list);
        }
        else
            S_opdump_indent(aTHX_ (OP*)pm, level, bar, file,
                        "CODE_LIST = 0x%" UVxf "\n", PTR2UV(pm->op_code_list));
    }
}


void
Perl_do_pmop_dump(pTHX_ I32 level, PerlIO *file, const PMOP *pm)
{
    PERL_ARGS_ASSERT_DO_PMOP_DUMP;
    S_do_pmop_dump_bar(aTHX_ level, 0, file, pm);
}


const struct flag_to_name pmflags_flags_names[] = {
    {PMf_CONST, ",CONST"},
    {PMf_KEEP, ",KEEP"},
    {PMf_GLOBAL, ",GLOBAL"},
    {PMf_CONTINUE, ",CONTINUE"},
    {PMf_RETAINT, ",RETAINT"},
    {PMf_EVAL, ",EVAL"},
    {PMf_NONDESTRUCT, ",NONDESTRUCT"},
    {PMf_HAS_CV, ",HAS_CV"},
    {PMf_CODELIST_PRIVATE, ",CODELIST_PRIVATE"},
    {PMf_IS_QR, ",IS_QR"}
};

static SV *
S_pm_description(pTHX_ const PMOP *pm)
{
    SV * const desc = newSVpvs("");
    const REGEXP * const regex = PM_GETRE(pm);
    const U32 pmflags = pm->op_pmflags;

    PERL_ARGS_ASSERT_PM_DESCRIPTION;

    if (pmflags & PMf_ONCE)
        sv_catpvs(desc, ",ONCE");
#ifdef USE_ITHREADS
    if (SvREADONLY(PL_regex_pad[pm->op_pmoffset]))
        sv_catpvs(desc, ":USED");
#else
    if (pmflags & PMf_USED)
        sv_catpvs(desc, ":USED");
#endif

    if (regex) {
        if (RX_ISTAINTED(regex))
            sv_catpvs(desc, ",TAINTED");
        if (RX_CHECK_SUBSTR(regex)) {
            if (!(RX_INTFLAGS(regex) & PREGf_NOSCAN))
                sv_catpvs(desc, ",SCANFIRST");
            if (RX_EXTFLAGS(regex) & RXf_CHECK_ALL)
                sv_catpvs(desc, ",ALL");
        }
        if (RX_EXTFLAGS(regex) & RXf_START_ONLY)
            sv_catpvs(desc, ",START_ONLY");
        if (RX_EXTFLAGS(regex) & RXf_SKIPWHITE)
            sv_catpvs(desc, ",SKIPWHITE");
        if (RX_EXTFLAGS(regex) & RXf_WHITE)
            sv_catpvs(desc, ",WHITE");
        if (RX_EXTFLAGS(regex) & RXf_NULL)
            sv_catpvs(desc, ",NULL");
    }

    append_flags(desc, pmflags, pmflags_flags_names);
    return desc;
}

/*
=for apidoc pmop_dump

Dump an OP that is related to Pattern Matching, such as C<s/foo/bar/>; these require
special handling.

=cut
*/

void
Perl_pmop_dump(pTHX_ PMOP *pm)
{
    do_pmop_dump(0, Perl_debug_log, pm);
}

/* Return a unique integer to represent the address of op o.
 * If it already exists in PL_op_sequence, just return it;
 * otherwise add it.
 *  *** Note that this isn't thread-safe */

STATIC UV
S_sequence_num(pTHX_ const OP *o)
{
    SV     *op,
          **seq;
    const char *key;
    STRLEN  len;
    if (!o)
        return 0;
    op = newSVuv(PTR2UV(o));
    sv_2mortal(op);
    key = SvPV_const(op, len);
    if (!PL_op_sequence)
        PL_op_sequence = newHV();
    seq = hv_fetch(PL_op_sequence, key, len, TRUE);
    if (SvOK(*seq))
        return SvUV(*seq);
    sv_setuv(*seq, ++PL_op_seq);
    return PL_op_seq;
}





const struct flag_to_name op_flags_names[] = {
    {OPf_KIDS, ",KIDS"},
    {OPf_PARENS, ",PARENS"},
    {OPf_REF, ",REF"},
    {OPf_MOD, ",MOD"},
    {OPf_STACKED, ",STACKED"},
    {OPf_SPECIAL, ",SPECIAL"}
};


/* indexed by enum OPclass */
const char * const op_class_names[] = {
    "NULL",
    "OP",
    "UNOP",
    "BINOP",
    "LOGOP",
    "LISTOP",
    "PMOP",
    "SVOP",
    "PADOP",
    "PVOP",
    "LOOP",
    "COP",
    "METHOP",
    "UNOP_AUX",
};


/* dump an op and any children. level indicates the initial indent.
 * The bits of bar indicate which indents should receive a vertical bar.
 * For example if level == 5 and bar == 0b01101, then the indent prefix
 * emitted will be (not including the <>'s):
 *
 *   <    |   |       |   >
 *    55554444333322221111
 *
 * For heavily nested output, the level may exceed the number of bits
 * in bar; in this case the first few columns in the output will simply
 * not have a bar, which is harmless.
 */

static void
S_do_op_dump_bar(pTHX_ I32 level, UV bar, PerlIO *file, const OP *o)
{
    const OPCODE optype = o->op_type;

    PERL_ARGS_ASSERT_DO_OP_DUMP;

    /* print op header line */

    S_opdump_indent(aTHX_ o, -level-1, bar, file, "%s", OP_NAME(o));

    if (optype == OP_NULL && o->op_targ)
        PerlIO_printf(file, " (ex-%s)",PL_op_name[o->op_targ]);

    PerlIO_printf(file, " %s(0x%" UVxf ")",
                    op_class_names[op_class(o)], PTR2UV(o));
    S_opdump_link(aTHX_ o, o->op_next, file);

    /* print op common fields */

    if (level == 0) {
        S_opdump_indent(aTHX_ o, level, bar, file, "PARENT");
        S_opdump_link(aTHX_ o, op_parent((OP*)o), file);
    }
    else if (!OpHAS_SIBLING(o)) {
        bool ok = TRUE;
        OP *p = o->op_sibparent;
        if (!p || !(p->op_flags & OPf_KIDS))
            ok = FALSE;
        else {
            OP *kid = cUNOPx(p)->op_first;
            while (kid != o) {
                kid = OpSIBLING(kid);
                if (!kid) {
                    ok = FALSE;
                    break;
                }
            }
        }
        if (!ok) {
            S_opdump_indent(aTHX_ o, level, bar, file,
                            "*** WILD PARENT 0x%p\n", p);
        }
    }

    if (o->op_targ && optype != OP_NULL)
            S_opdump_indent(aTHX_ o, level, bar, file, "TARG = %ld\n",
                (long)o->op_targ);

    if (o->op_flags || o->op_slabbed || o->op_savefree || o->op_static) {
        SV * const tmpsv = newSVpvs("");
        switch (o->op_flags & OPf_WANT) {
        case OPf_WANT_VOID:
            sv_catpvs(tmpsv, ",VOID");
            break;
        case OPf_WANT_SCALAR:
            sv_catpvs(tmpsv, ",SCALAR");
            break;
        case OPf_WANT_LIST:
            sv_catpvs(tmpsv, ",LIST");
            break;
        default:
            sv_catpvs(tmpsv, ",UNKNOWN");
            break;
        }
        append_flags(tmpsv, o->op_flags, op_flags_names);
        if (o->op_slabbed)  sv_catpvs(tmpsv, ",SLABBED");
        if (o->op_savefree) sv_catpvs(tmpsv, ",SAVEFREE");
        if (o->op_static)   sv_catpvs(tmpsv, ",STATIC");
        if (o->op_folded)   sv_catpvs(tmpsv, ",FOLDED");
        if (o->op_moresib)  sv_catpvs(tmpsv, ",MORESIB");
        S_opdump_indent(aTHX_ o, level, bar, file, "FLAGS = (%s)\n",
                         SvCUR(tmpsv) ? SvPVX_const(tmpsv) + 1 : "");
    }

    if (o->op_private) {
        U16 oppriv = o->op_private;
        I16 op_ix = PL_op_private_bitdef_ix[o->op_type];
        SV * tmpsv = NULL;

        if (op_ix != -1) {
            U16 stop = 0;
            tmpsv = newSVpvs("");
            for (; !stop; op_ix++) {
                U16 entry = PL_op_private_bitdefs[op_ix];
                U16 bit = (entry >> 2) & 7;
                U16 ix = entry >> 5;

                stop = (entry & 1);

                if (entry & 2) {
                    /* bitfield */
                    I16 const *p = &PL_op_private_bitfields[ix];
                    U16 bitmin = (U16) *p++;
                    I16 label = *p++;
                    I16 enum_label;
                    U16 mask = 0;
                    U16 i;
                    U16 val;

                    for (i = bitmin; i<= bit; i++)
                        mask |= (1<<i);
                    bit = bitmin;
                    val = (oppriv & mask);

                    if (   label != -1
                        && PL_op_private_labels[label] == '-'
                        && PL_op_private_labels[label+1] == '\0'
                    )
                        /* display as raw number */
                        continue;

                    oppriv -= val;
                    val >>= bit;
                    enum_label = -1;
                    while (*p != -1) {
                        if (val == *p++) {
                            enum_label = *p;
                            break;
                        }
                        p++;
                    }
                    if (val == 0 && enum_label == -1)
                        /* don't display anonymous zero values */
                        continue;

                    sv_catpvs(tmpsv, ",");
                    if (label != -1) {
                        sv_catpv(tmpsv, &PL_op_private_labels[label]);
                        sv_catpvs(tmpsv, "=");
                    }
                    if (enum_label == -1)
                        Perl_sv_catpvf(aTHX_ tmpsv, "0x%" UVxf, (UV)val);
                    else
                        sv_catpv(tmpsv, &PL_op_private_labels[enum_label]);

                }
                else {
                    /* bit flag */
                    if (   oppriv & (1<<bit)
                        && !(PL_op_private_labels[ix] == '-'
                             && PL_op_private_labels[ix+1] == '\0'))
                    {
                        oppriv -= (1<<bit);
                        sv_catpvs(tmpsv, ",");
                        sv_catpv(tmpsv, &PL_op_private_labels[ix]);
                    }
                }
            }
            if (oppriv) {
                sv_catpvs(tmpsv, ",");
                Perl_sv_catpvf(aTHX_ tmpsv, "0x%" UVxf, (UV)oppriv);
            }
        }
        if (tmpsv && SvCUR(tmpsv)) {
            S_opdump_indent(aTHX_ o, level, bar, file, "PRIVATE = (%s)\n",
                            SvPVX_const(tmpsv) + 1);
        } else
            S_opdump_indent(aTHX_ o, level, bar, file,
                            "PRIVATE = (0x%" UVxf ")\n", (UV)oppriv);
    }

    switch (optype) {
    case OP_AELEMFAST:
    case OP_GVSV:
    case OP_GV:
#ifdef USE_ITHREADS
        S_opdump_indent(aTHX_ o, level, bar, file,
                        "PADIX = %" IVdf "\n", (IV)cPADOPo->op_padix);
#else
        S_opdump_indent(aTHX_ o, level, bar, file,
            "GV = %" SVf " (0x%" UVxf ")\n",
            SVfARG(S_gv_display(aTHX_ cGVOPo_gv)), PTR2UV(cGVOPo_gv));
#endif
        break;

    case OP_MULTIDEREF:
    {
        UNOP_AUX_item *items = cUNOP_AUXo->op_aux;
        UV i, count = items[-1].uv;

        S_opdump_indent(aTHX_ o, level, bar, file, "ARGS = \n");
        for (i=0; i < count;  i++)
            S_opdump_indent(aTHX_ o, level+1, (bar << 1), file,
                                    "%" UVuf " => 0x%" UVxf "\n",
                                    i, items[i].uv);
        break;
    }

    case OP_MULTICONCAT:
        S_opdump_indent(aTHX_ o, level, bar, file, "NARGS = %" IVdf "\n",
            (IV)cUNOP_AUXo->op_aux[PERL_MULTICONCAT_IX_NARGS].ssize);
        /* XXX really ought to dump each field individually,
         * but that's too much like hard work */
        S_opdump_indent(aTHX_ o, level, bar, file, "CONSTS = (%" SVf ")\n",
            SVfARG(multiconcat_stringify(o)));
        break;

    case OP_CONST:
    case OP_HINTSEVAL:
    case OP_METHOD_NAMED:
    case OP_METHOD_SUPER:
    case OP_METHOD_REDIR:
    case OP_METHOD_REDIR_SUPER:
#ifndef USE_ITHREADS
        /* with ITHREADS, consts are stored in the pad, and the right pad
         * may not be active here, so skip */
        S_opdump_indent(aTHX_ o, level, bar, file, "SV = %s\n",
                        SvPEEK(cMETHOPo_meth));
#endif
        break;
    case OP_NULL:
        if (o->op_targ != OP_NEXTSTATE && o->op_targ != OP_DBSTATE)
            break;
        /* FALLTHROUGH */
    case OP_NEXTSTATE:
    case OP_DBSTATE:
        if (CopLINE(cCOPo))
            S_opdump_indent(aTHX_ o, level, bar, file, "LINE = %" LINE_Tf "\n",
                            CopLINE(cCOPo));

        if (CopSTASHPV(cCOPo)) {
            SV* tmpsv = newSVpvs_flags("", SVs_TEMP);
            HV *stash = CopSTASH(cCOPo);
            const char * const hvname = HvNAME_get(stash);

            S_opdump_indent(aTHX_ o, level, bar, file, "PACKAGE = \"%s\"\n",
                               generic_pv_escape(tmpsv, hvname,
                                  HvNAMELEN(stash), HvNAMEUTF8(stash)));
        }

        if (CopLABEL(cCOPo)) {
            SV* tmpsv = newSVpvs_flags("", SVs_TEMP);
            STRLEN label_len;
            U32 label_flags;
            const char *label = CopLABEL_len_flags(cCOPo,
                                                     &label_len, &label_flags);
            S_opdump_indent(aTHX_ o, level, bar, file, "LABEL = \"%s\"\n",
                                generic_pv_escape( tmpsv, label, label_len,
                                           (label_flags & SVf_UTF8)));
        }
        /* add hints and features if set */
        if (cCOPo->cop_hints)
            S_opdump_indent(aTHX_ o, level, bar, file, "HINTS = %08x\n",cCOPo->cop_hints);
        if (cCOPo->cop_features)
            S_opdump_indent(aTHX_ o, level, bar, file, "FEATS = %08x\n",cCOPo->cop_features);

        S_opdump_indent(aTHX_ o, level, bar, file, "SEQ = %u\n",
                         (unsigned int)cCOPo->cop_seq);
        break;

    case OP_ENTERITER:
    case OP_ENTERLOOP:
        S_opdump_indent(aTHX_ o, level, bar, file, "REDO");
        S_opdump_link(aTHX_ o, cLOOPo->op_redoop, file);
        S_opdump_indent(aTHX_ o, level, bar, file, "NEXT");
        S_opdump_link(aTHX_ o, cLOOPo->op_nextop, file);
        S_opdump_indent(aTHX_ o, level, bar, file, "LAST");
        S_opdump_link(aTHX_ o, cLOOPo->op_lastop, file);
        break;

    case OP_REGCOMP:
    case OP_SUBSTCONT:
    case OP_COND_EXPR:
    case OP_RANGE:
    case OP_MAPWHILE:
    case OP_GREPWHILE:
    case OP_OR:
    case OP_DOR:
    case OP_AND:
    case OP_ORASSIGN:
    case OP_DORASSIGN:
    case OP_ANDASSIGN:
    case OP_ARGDEFELEM:
    case OP_ENTERGIVEN:
    case OP_ENTERWHEN:
    case OP_ENTERTRY:
    case OP_ONCE:
        S_opdump_indent(aTHX_ o, level, bar, file, "OTHER");
        S_opdump_link(aTHX_ o, cLOGOPo->op_other, file);
        break;
    case OP_SPLIT:
    case OP_MATCH:
    case OP_QR:
    case OP_SUBST:
        S_do_pmop_dump_bar(aTHX_ level, bar, file, cPMOPo);
        break;
    case OP_LEAVE:
    case OP_LEAVEEVAL:
    case OP_LEAVESUB:
    case OP_LEAVESUBLV:
    case OP_LEAVEWRITE:
    case OP_SCOPE:
        if (o->op_private & OPpREFCOUNTED)
            S_opdump_indent(aTHX_ o, level, bar, file,
                            "REFCNT = %" UVuf "\n", (UV)o->op_targ);
        break;

    case OP_DUMP:
    case OP_GOTO:
    case OP_NEXT:
    case OP_LAST:
    case OP_REDO:
        if (o->op_flags & (OPf_SPECIAL|OPf_STACKED|OPf_KIDS))
            break;
        {
            SV * const label = newSVpvs_flags("", SVs_TEMP);
            generic_pv_escape(label, cPVOPo->op_pv, strlen(cPVOPo->op_pv), 0);
            S_opdump_indent(aTHX_ o, level, bar, file,
                            "PV = \"%" SVf "\" (0x%" UVxf ")\n",
                            SVfARG(label), PTR2UV(cPVOPo->op_pv));
            break;
        }

    case OP_TRANS:
    case OP_TRANSR:
        if (o->op_private & OPpTRANS_USE_SVOP) {
            /* utf8: table stored as an inversion map */
#ifndef USE_ITHREADS
        /* with ITHREADS, it is stored in the pad, and the right pad
         * may not be active here, so skip */
            S_opdump_indent(aTHX_ o, level, bar, file,
                            "INVMAP = 0x%" UVxf "\n",
                            PTR2UV(MUTABLE_SV(cSVOPo->op_sv)));
#endif
        }
        else {
            const OPtrans_map * const tbl = (OPtrans_map*)cPVOPo->op_pv;
            SSize_t i, size = tbl->size;

            S_opdump_indent(aTHX_ o, level, bar, file,
                            "TABLE = 0x%" UVxf "\n",
                            PTR2UV(tbl));
            S_opdump_indent(aTHX_ o, level, bar, file,
                "  SIZE: 0x%" UVxf "\n", (UV)size);

            /* dump size+1 values, to include the extra slot at the end */
            for (i = 0; i <= size; i++) {
                short val = tbl->map[i];
                if ((i & 0xf) == 0)
                    S_opdump_indent(aTHX_ o, level, bar, file,
                        " %4" UVxf ":", (UV)i);
                if (val < 0)
                    PerlIO_printf(file, " %2"  IVdf, (IV)val);
                else
                    PerlIO_printf(file, " %02" UVxf, (UV)val);

                if ( i == size || (i & 0xf) == 0xf)
                    PerlIO_printf(file, "\n");
            }
        }
        break;


    default:
        break;
    }
    if (o->op_flags & OPf_KIDS) {
        OP *kid;
        level++;
        bar <<= 1;
        for (kid = cUNOPo->op_first; kid; kid = OpSIBLING(kid))
            S_do_op_dump_bar(aTHX_ level,
                            (bar | cBOOL(OpHAS_SIBLING(kid))),
                            file, kid);
    }
}


void
Perl_do_op_dump(pTHX_ I32 level, PerlIO *file, const OP *o)
{
    S_do_op_dump_bar(aTHX_ level, 0, file, o);
}


/*
=for apidoc op_dump

Dumps the optree starting at OP C<o> to C<STDERR>.

=cut
*/

void
Perl_op_dump(pTHX_ const OP *o)
{
    PERL_ARGS_ASSERT_OP_DUMP;
    do_op_dump(0, Perl_debug_log, o);
}

/*
=for apidoc gv_dump

Dump the name and, if they differ, the effective name of the GV C<gv> to
C<STDERR>.

=cut
*/

void
Perl_gv_dump(pTHX_ GV *gv)
{
    STRLEN len;
    const char* name;
    SV *sv, *tmp = newSVpvs_flags("", SVs_TEMP);

    if (!gv) {
        PerlIO_printf(Perl_debug_log, "{}\n");
        return;
    }
    sv = sv_newmortal();
    PerlIO_printf(Perl_debug_log, "{\n");
    gv_fullname3(sv, gv, NULL);
    name = SvPV_const(sv, len);
    Perl_dump_indent(aTHX_ 1, Perl_debug_log, "GV_NAME = %s",
                     generic_pv_escape( tmp, name, len, SvUTF8(sv) ));
    if (gv != GvEGV(gv)) {
        gv_efullname3(sv, GvEGV(gv), NULL);
        name = SvPV_const(sv, len);
        Perl_dump_indent(aTHX_ 1, Perl_debug_log, "-> %s",
                     generic_pv_escape( tmp, name, len, SvUTF8(sv) ));
    }
    (void)PerlIO_putc(Perl_debug_log, '\n');
    Perl_dump_indent(aTHX_ 0, Perl_debug_log, "}\n");
}


/* map magic types to the symbolic names
 * (with the PERL_MAGIC_ prefixed stripped)
 */

static const struct { const char type; const char *name; } magic_names[] = {
#include "mg_names.inc"
        /* this null string terminates the list */
        { 0,                         NULL },
};

void
Perl_do_magic_dump(pTHX_ I32 level, PerlIO *file, const MAGIC *mg, I32 nest, I32 maxnest, bool dumpops, STRLEN pvlim)
{
    PERL_ARGS_ASSERT_DO_MAGIC_DUMP;

    for (; mg; mg = mg->mg_moremagic) {
        Perl_dump_indent(aTHX_ level, file,
                         "  MAGIC = 0x%" UVxf "\n", PTR2UV(mg));
        if (mg->mg_virtual) {
            const MGVTBL * const v = mg->mg_virtual;
            if (v >= PL_magic_vtables
                && v < PL_magic_vtables + magic_vtable_max) {
                const U32 i = v - PL_magic_vtables;
                Perl_dump_indent(aTHX_ level, file, "    MG_VIRTUAL = &PL_vtbl_%s\n", PL_magic_vtable_names[i]);
            }
            else
                Perl_dump_indent(aTHX_ level, file, "    MG_VIRTUAL = 0x%"
                                       UVxf "\n", PTR2UV(v));
        }
        else
            Perl_dump_indent(aTHX_ level, file, "    MG_VIRTUAL = 0\n");

        if (mg->mg_private)
            Perl_dump_indent(aTHX_ level, file, "    MG_PRIVATE = %d\n", mg->mg_private);

        {
            int n;
            const char *name = NULL;
            for (n = 0; magic_names[n].name; n++) {
                if (mg->mg_type == magic_names[n].type) {
                    name = magic_names[n].name;
                    break;
                }
            }
            if (name)
                Perl_dump_indent(aTHX_ level, file,
                                "    MG_TYPE = PERL_MAGIC_%s\n", name);
            else
                Perl_dump_indent(aTHX_ level, file,
                                "    MG_TYPE = UNKNOWN(\\%o)\n", mg->mg_type);
        }

        if (mg->mg_flags) {
            Perl_dump_indent(aTHX_ level, file, "    MG_FLAGS = 0x%02X\n", mg->mg_flags);
            if (mg->mg_type == PERL_MAGIC_envelem &&
                mg->mg_flags & MGf_TAINTEDDIR)
                Perl_dump_indent(aTHX_ level, file, "      TAINTEDDIR\n");
            if (mg->mg_type == PERL_MAGIC_regex_global &&
                mg->mg_flags & MGf_MINMATCH)
                Perl_dump_indent(aTHX_ level, file, "      MINMATCH\n");
            if (mg->mg_flags & MGf_REFCOUNTED)
                Perl_dump_indent(aTHX_ level, file, "      REFCOUNTED\n");
            if (mg->mg_flags & MGf_GSKIP)
                Perl_dump_indent(aTHX_ level, file, "      GSKIP\n");
            if (mg->mg_flags & MGf_COPY)
                Perl_dump_indent(aTHX_ level, file, "      COPY\n");
            if (mg->mg_flags & MGf_DUP)
                Perl_dump_indent(aTHX_ level, file, "      DUP\n");
            if (mg->mg_flags & MGf_LOCAL)
                Perl_dump_indent(aTHX_ level, file, "      LOCAL\n");
            if (mg->mg_type == PERL_MAGIC_regex_global &&
                mg->mg_flags & MGf_BYTES)
                Perl_dump_indent(aTHX_ level, file, "      BYTES\n");
        }
        if (mg->mg_obj) {
            Perl_dump_indent(aTHX_ level, file, "    MG_OBJ = 0x%" UVxf "\n",
                PTR2UV(mg->mg_obj));
            if (mg->mg_type == PERL_MAGIC_qr) {
                REGEXP* const re = (REGEXP *)mg->mg_obj;
                SV * const dsv = sv_newmortal();
                const char * const s
                    = pv_pretty(dsv, RX_WRAPPED(re), RX_WRAPLEN(re),
                    60, NULL, NULL,
                    ( PERL_PV_PRETTY_QUOTE | PERL_PV_ESCAPE_RE | PERL_PV_PRETTY_ELLIPSES |
                    (RX_UTF8(re) ? PERL_PV_ESCAPE_UNI : 0))
                );
                Perl_dump_indent(aTHX_ level+1, file, "    PAT = %s\n", s);
                Perl_dump_indent(aTHX_ level+1, file, "    REFCNT = %" IVdf "\n",
                        (IV)RX_REFCNT(re));
            }
            if (mg->mg_flags & MGf_REFCOUNTED)
                do_sv_dump(level+2, file, mg->mg_obj, nest+1, maxnest, dumpops, pvlim); /* MG is already +1 */
        }
        if (mg->mg_len)
            Perl_dump_indent(aTHX_ level, file, "    MG_LEN = %ld\n", (long)mg->mg_len);
        if (mg->mg_ptr) {
            Perl_dump_indent(aTHX_ level, file, "    MG_PTR = 0x%" UVxf, PTR2UV(mg->mg_ptr));
            if (mg->mg_len >= 0) {
                if (mg->mg_type != PERL_MAGIC_utf8) {
                    SV * const sv = newSVpvs("");
                    PerlIO_printf(file, " %s", pv_display(sv, mg->mg_ptr, mg->mg_len, 0, pvlim));
                    SvREFCNT_dec_NN(sv);
                }
            }
            else if (mg->mg_len == HEf_SVKEY) {
                PerlIO_puts(file, " => HEf_SVKEY\n");
                do_sv_dump(level+2, file, MUTABLE_SV(((mg)->mg_ptr)), nest+1,
                           maxnest, dumpops, pvlim); /* MG is already +1 */
                continue;
            }
            else if (mg->mg_len == -1 && mg->mg_type == PERL_MAGIC_utf8);
            else
                PerlIO_puts(
                  file,
                 " ???? - " __FILE__
                 " does not know how to handle this MG_LEN"
                );
            (void)PerlIO_putc(file, '\n');
        }
        if (mg->mg_type == PERL_MAGIC_utf8) {
            const STRLEN * const cache = (STRLEN *) mg->mg_ptr;
            if (cache) {
                IV i;
                for (i = 0; i < PERL_MAGIC_UTF8_CACHESIZE; i++)
                    Perl_dump_indent(aTHX_ level, file,
                                     "      %2" IVdf ": %" UVuf " -> %" UVuf "\n",
                                     i,
                                     (UV)cache[i * 2],
                                     (UV)cache[i * 2 + 1]);
            }
        }
    }
}

/*
=for apidoc magic_dump

Dumps the contents of the MAGIC C<mg> to C<STDERR>.

=cut
*/

void
Perl_magic_dump(pTHX_ const MAGIC *mg)
{
    do_magic_dump(0, Perl_debug_log, mg, 0, 0, FALSE, 0);
}

void
Perl_do_hv_dump(pTHX_ I32 level, PerlIO *file, const char *name, HV *sv)
{
    const char *hvname;

    PERL_ARGS_ASSERT_DO_HV_DUMP;

    Perl_dump_indent(aTHX_ level, file, "%s = 0x%" UVxf, name, PTR2UV(sv));
    if (sv && (hvname = HvNAME_get(sv)))
    {
        /* we have to use pv_display and HvNAMELEN_get() so that we display the real package
           name which quite legally could contain insane things like tabs, newlines, nulls or
           other scary crap - this should produce sane results - except maybe for unicode package
           names - but we will wait for someone to file a bug on that - demerphq */
        SV * const tmpsv = newSVpvs_flags("", SVs_TEMP);
        PerlIO_printf(file, "\t\"%s\"\n",
                              generic_pv_escape( tmpsv, hvname,
                                   HvNAMELEN(sv), HvNAMEUTF8(sv)));
    }
    else
        (void)PerlIO_putc(file, '\n');
}

void
Perl_do_gv_dump(pTHX_ I32 level, PerlIO *file, const char *name, GV *sv)
{
    PERL_ARGS_ASSERT_DO_GV_DUMP;

    Perl_dump_indent(aTHX_ level, file, "%s = 0x%" UVxf, name, PTR2UV(sv));
    if (sv) {
        SV * const tmpsv = newSVpvs("");
        PerlIO_printf(file, "\t\"%s\"\n",
                              generic_pv_escape( tmpsv, GvNAME(sv), GvNAMELEN(sv), GvNAMEUTF8(sv) ));
    }
    else
        (void)PerlIO_putc(file, '\n');
}

void
Perl_do_gvgv_dump(pTHX_ I32 level, PerlIO *file, const char *name, GV *sv)
{
    PERL_ARGS_ASSERT_DO_GVGV_DUMP;

    Perl_dump_indent(aTHX_ level, file, "%s = 0x%" UVxf, name, PTR2UV(sv));
    if (sv) {
       SV *tmp = newSVpvs_flags("", SVs_TEMP);
        const char *hvname;
        HV * const stash = GvSTASH(sv);
        PerlIO_printf(file, "\t");
        /* TODO might have an extra \" here */
        if (stash && (hvname = HvNAME_get(stash))) {
            PerlIO_printf(file, "\"%s\" :: \"",
                                  generic_pv_escape(tmp, hvname,
                                      HvNAMELEN(stash), HvNAMEUTF8(stash)));
        }
        PerlIO_printf(file, "%s\"\n",
                              generic_pv_escape( tmp, GvNAME(sv), GvNAMELEN(sv), GvNAMEUTF8(sv)));
    }
    else
        (void)PerlIO_putc(file, '\n');
}

const struct flag_to_name first_sv_flags_names[] = {
    {SVs_TEMP, "TEMP,"},
    {SVs_OBJECT, "OBJECT,"},
    {SVs_GMG, "GMG,"},
    {SVs_SMG, "SMG,"},
    {SVs_RMG, "RMG,"},
    {SVf_IOK, "IOK,"},
    {SVf_NOK, "NOK,"},
    {SVf_POK, "POK,"}
};

const struct flag_to_name second_sv_flags_names[] = {
    {SVf_OOK, "OOK,"},
    {SVf_FAKE, "FAKE,"},
    {SVf_READONLY, "READONLY,"},
    {SVf_PROTECT, "PROTECT,"},
    {SVf_BREAK, "BREAK,"},
    {SVp_IOK, "pIOK,"},
    {SVp_NOK, "pNOK,"},
    {SVp_POK, "pPOK,"}
};

const struct flag_to_name cv_flags_names[] = {
    {CVf_ANON, "ANON,"},
    {CVf_UNIQUE, "UNIQUE,"},
    {CVf_CLONE, "CLONE,"},
    {CVf_CLONED, "CLONED,"},
    {CVf_CONST, "CONST,"},
    {CVf_NODEBUG, "NODEBUG,"},
    {CVf_LVALUE, "LVALUE,"},
    {CVf_NOWARN_AMBIGUOUS, "NOWARN_AMBIGUOUS,"},
    {CVf_WEAKOUTSIDE, "WEAKOUTSIDE,"},
    {CVf_CVGV_RC, "CVGV_RC,"},
    {CVf_DYNFILE, "DYNFILE,"},
    {CVf_AUTOLOAD, "AUTOLOAD,"},
    {CVf_HASEVAL, "HASEVAL,"},
    {CVf_SLABBED, "SLABBED,"},
    {CVf_NAMED, "NAMED,"},
    {CVf_LEXICAL, "LEXICAL,"},
    {CVf_ISXSUB, "ISXSUB,"},
    {CVf_ANONCONST,        "ANONCONST,"},
    {CVf_SIGNATURE,        "SIGNATURE,"},
    {CVf_REFCOUNTED_ANYSV, "REFCOUNTED_ANYSV,"},
    {CVf_IsMETHOD,         "IsMETHOD,"}

};

const struct flag_to_name hv_flags_names[] = {
    {SVphv_SHAREKEYS, "SHAREKEYS,"},
    {SVphv_LAZYDEL, "LAZYDEL,"},
    {SVphv_HASKFLAGS, "HASKFLAGS,"},
    {SVf_AMAGIC, "OVERLOAD,"},
    {SVphv_CLONEABLE, "CLONEABLE,"}
};

const struct flag_to_name gp_flags_names[] = {
    {GVf_INTRO, "INTRO,"},
    {GVf_MULTI, "MULTI,"},
    {GVf_ASSUMECV, "ASSUMECV,"},
};

const struct flag_to_name gp_flags_imported_names[] = {
    {GVf_IMPORTED_SV, " SV"},
    {GVf_IMPORTED_AV, " AV"},
    {GVf_IMPORTED_HV, " HV"},
    {GVf_IMPORTED_CV, " CV"},
};

/* NOTE: this structure is mostly duplicative of one generated by
 * 'make regen' in regnodes.h - perhaps we should somehow integrate
 * the two. - Yves */
const struct flag_to_name regexp_extflags_names[] = {
    {RXf_PMf_MULTILINE,   "PMf_MULTILINE,"},
    {RXf_PMf_SINGLELINE,  "PMf_SINGLELINE,"},
    {RXf_PMf_FOLD,        "PMf_FOLD,"},
    {RXf_PMf_EXTENDED,    "PMf_EXTENDED,"},
    {RXf_PMf_EXTENDED_MORE, "PMf_EXTENDED_MORE,"},
    {RXf_PMf_KEEPCOPY,    "PMf_KEEPCOPY,"},
    {RXf_PMf_NOCAPTURE,   "PMf_NOCAPURE,"},
    {RXf_IS_ANCHORED,     "IS_ANCHORED,"},
    {RXf_NO_INPLACE_SUBST, "NO_INPLACE_SUBST,"},
    {RXf_EVAL_SEEN,       "EVAL_SEEN,"},
    {RXf_CHECK_ALL,       "CHECK_ALL,"},
    {RXf_MATCH_UTF8,      "MATCH_UTF8,"},
    {RXf_USE_INTUIT_NOML, "USE_INTUIT_NOML,"},
    {RXf_USE_INTUIT_ML,   "USE_INTUIT_ML,"},
    {RXf_INTUIT_TAIL,     "INTUIT_TAIL,"},
    {RXf_SPLIT,           "SPLIT,"},
    {RXf_COPY_DONE,       "COPY_DONE,"},
    {RXf_TAINTED_SEEN,    "TAINTED_SEEN,"},
    {RXf_TAINTED,         "TAINTED,"},
    {RXf_START_ONLY,      "START_ONLY,"},
    {RXf_SKIPWHITE,       "SKIPWHITE,"},
    {RXf_WHITE,           "WHITE,"},
    {RXf_NULL,            "NULL,"},
};

/* NOTE: this structure is mostly duplicative of one generated by
 * 'make regen' in regnodes.h - perhaps we should somehow integrate
 * the two. - Yves */
const struct flag_to_name regexp_core_intflags_names[] = {
    {PREGf_SKIP,            "SKIP,"},
    {PREGf_IMPLICIT,        "IMPLICIT,"},
    {PREGf_NAUGHTY,         "NAUGHTY,"},
    {PREGf_VERBARG_SEEN,    "VERBARG_SEEN,"},
    {PREGf_CUTGROUP_SEEN,   "CUTGROUP_SEEN,"},
    {PREGf_USE_RE_EVAL,     "USE_RE_EVAL,"},
    {PREGf_NOSCAN,          "NOSCAN,"},
    {PREGf_GPOS_SEEN,       "GPOS_SEEN,"},
    {PREGf_GPOS_FLOAT,      "GPOS_FLOAT,"},
    {PREGf_ANCH_MBOL,       "ANCH_MBOL,"},
    {PREGf_ANCH_SBOL,       "ANCH_SBOL,"},
    {PREGf_ANCH_GPOS,       "ANCH_GPOS,"},
};

/* Minimum number of decimal digits to preserve the significand of NV.  */
#ifdef USE_LONG_DOUBLE
#  ifdef LDBL_DECIMAL_DIG
#    define NV_DECIMAL_DIG      LDBL_DECIMAL_DIG
#  endif
#elif defined(USE_QUADMATH) && defined(I_QUADMATH)
#  ifdef FLT128_DECIMAL_DIG
#    define NV_DECIMAL_DIG      FLT128_DECIMAL_DIG
#  endif
#else  /* NV is double */
#  ifdef DBL_DECIMAL_DIG
#    define NV_DECIMAL_DIG      DBL_DECIMAL_DIG
#  endif
#endif

#ifndef NV_DECIMAL_DIG
#  if defined(NV_MANT_DIG) && FLT_RADIX == 2
/* NV_DECIMAL_DIG = ceil(1 + NV_MANT_DIG * log10(2)), where log10(2) is
   approx. 146/485.  This is precise enough up to 2620 bits */
#    define NV_DECIMAL_DIG      (1 + (NV_MANT_DIG * 146 + 484) / 485)
#  endif
#endif

#ifndef NV_DECIMAL_DIG
#  define NV_DECIMAL_DIG        (NV_DIG + 3) /* last resort */
#endif

/* Perl_do_sv_dump():
 *
 * level:   amount to indent the output
 * sv:      the object to dump
 * nest:    the current level of recursion
 * maxnest: the maximum allowed level of recursion
 * dumpops: if true, also dump the ops associated with a CV
 * pvlim:   limit on the length of any strings that are output
 * */

void
Perl_do_sv_dump(pTHX_ I32 level, PerlIO *file, SV *sv, I32 nest, I32 maxnest, bool dumpops, STRLEN pvlim)
{
    SV *d;
    const char *s;
    U32 flags;
    U32 type;

    PERL_ARGS_ASSERT_DO_SV_DUMP;

    if (!sv) {
        Perl_dump_indent(aTHX_ level, file, "SV = 0\n");
        return;
    }

    flags = SvFLAGS(sv);
    type = SvTYPE(sv);

    /* process general SV flags */

    d = Perl_newSVpvf(aTHX_
                   "(0x%" UVxf ") at 0x%" UVxf "\n%*s  REFCNT = %" IVdf "\n%*s  FLAGS = (",
                   PTR2UV(SvANY(sv)), PTR2UV(sv),
                   (int)(PL_dumpindent*level), "", (IV)SvREFCNT(sv),
                   (int)(PL_dumpindent*level), "");

    if ((flags & SVs_PADSTALE))
            sv_catpvs(d, "PADSTALE,");
    if ((flags & SVs_PADTMP))
            sv_catpvs(d, "PADTMP,");
    append_flags(d, flags, first_sv_flags_names);
    if (flags & SVf_ROK)  {	
                                sv_catpvs(d, "ROK,");
        if (SvWEAKREF(sv))	sv_catpvs(d, "WEAKREF,");
    }
    if (flags & SVf_IsCOW && type != SVt_PVHV) sv_catpvs(d, "IsCOW,");
    append_flags(d, flags, second_sv_flags_names);
    if (flags & SVp_SCREAM && type != SVt_PVHV && !isGV_with_GP(sv)
                           && type != SVt_PVAV) {
        if (SvPCS_IMPORTED(sv))
                                sv_catpvs(d, "PCS_IMPORTED,");
        else
                                sv_catpvs(d, "SCREAM,");
    }

    /* process type-specific SV flags */

    switch (type) {
    case SVt_PVCV:
    case SVt_PVFM:
        append_flags(d, CvFLAGS(sv), cv_flags_names);
        break;
    case SVt_PVHV:
        append_flags(d, flags, hv_flags_names);
        break;
    case SVt_PVGV:
    case SVt_PVLV:
        if (isGV_with_GP(sv)) {
            append_flags(d, GvFLAGS(sv), gp_flags_names);
        }
        if (isGV_with_GP(sv) && GvIMPORTED(sv)) {
            sv_catpvs(d, "IMPORT");
            if (GvIMPORTED(sv) == GVf_IMPORTED)
                sv_catpvs(d, "ALL,");
            else {
                sv_catpvs(d, "(");
                append_flags(d, GvFLAGS(sv), gp_flags_imported_names);
                sv_catpvs(d, " ),");
            }
        }
        /* FALLTHROUGH */
    case SVt_PVMG:
    default:
        if (SvIsUV(sv) && !(flags & SVf_ROK))	sv_catpvs(d, "IsUV,");
        break;

    case SVt_PVAV:
        break;
    }
    /* SVphv_SHAREKEYS is also 0x20000000 */
    if ((type != SVt_PVHV) && SvUTF8(sv))
        sv_catpvs(d, "UTF8");

    if (*(SvEND(d) - 1) == ',') {
        SvCUR_set(d, SvCUR(d) - 1);
        SvPVX(d)[SvCUR(d)] = '\0';
    }
    sv_catpvs(d, ")");
    s = SvPVX_const(d);

    /* dump initial SV details */

#ifdef DEBUG_LEAKING_SCALARS
    Perl_dump_indent(aTHX_ level, file,
        "ALLOCATED at %s:%d %s %s (parent 0x%" UVxf "); serial %" UVuf "\n",
        sv->sv_debug_file ? sv->sv_debug_file : "(unknown)",
        sv->sv_debug_line,
        sv->sv_debug_inpad ? "for" : "by",
        sv->sv_debug_optype ? PL_op_name[sv->sv_debug_optype]: "(none)",
        PTR2UV(sv->sv_debug_parent),
        sv->sv_debug_serial
    );
#endif
    Perl_dump_indent(aTHX_ level, file, "SV = ");

    /* Dump SV type */

    if (type < SVt_LAST) {
        PerlIO_printf(file, "%s%s\n", svtypenames[type], s);

        if (type ==  SVt_NULL) {
            SvREFCNT_dec_NN(d);
            return;
        }
    } else {
        PerlIO_printf(file, "UNKNOWN(0x%" UVxf ") %s\n", (UV)type, s);
        SvREFCNT_dec_NN(d);
        return;
    }

    /* Dump general SV fields */

    if ((type >= SVt_PVIV && type <= SVt_PVLV
         && type != SVt_REGEXP && !isGV_with_GP(sv) && !SvVALID(sv))
        || (type == SVt_IV && !SvROK(sv))) {
        if (SvIsUV(sv)
                                     )
            Perl_dump_indent(aTHX_ level, file, "  UV = %" UVuf, (UV)SvUVX(sv));
        else
            Perl_dump_indent(aTHX_ level, file, "  IV = %" IVdf, (IV)SvIVX(sv));
        (void)PerlIO_putc(file, '\n');
    }

    if ((type >= SVt_PVNV && type <= SVt_PVLV
         && type != SVt_REGEXP && !isGV_with_GP(sv) && !SvVALID(sv))
               || type == SVt_NV) {
        DECLARATION_FOR_LC_NUMERIC_MANIPULATION;
        STORE_LC_NUMERIC_SET_STANDARD();
        Perl_dump_indent(aTHX_ level, file, "  NV = %.*" NVgf "\n", NV_DECIMAL_DIG, SvNVX(sv));
        RESTORE_LC_NUMERIC();
    }

    if (SvROK(sv)) {
        Perl_dump_indent(aTHX_ level, file, "  RV = 0x%" UVxf "\n",
                               PTR2UV(SvRV(sv)));
        if (nest < maxnest)
            do_sv_dump(level+1, file, SvRV(sv), nest+1, maxnest, dumpops, pvlim);
    }

    if (type < SVt_PV) {
        SvREFCNT_dec_NN(d);
        return;
    }

    if ((type <= SVt_PVLV && !isGV_with_GP(sv))
     || (type == SVt_PVIO && IoFLAGS(sv) & IOf_FAKE_DIRP)) {
        const bool re = isREGEXP(sv);
        const char * const ptr =
            re ? RX_WRAPPED((REGEXP*)sv) : SvPVX_const(sv);
        if (ptr) {
            STRLEN delta;
            if (SvOOK(sv)) {
                SvOOK_offset(sv, delta);
                Perl_dump_indent(aTHX_ level, file,"  OFFSET = %" UVuf "\n",
                                 (UV) delta);
            } else {
                delta = 0;
            }
            Perl_dump_indent(aTHX_ level, file,"  PV = 0x%" UVxf " ",
                                   PTR2UV(ptr));
            if (SvOOK(sv)) {
                PerlIO_printf(file, "( %s . ) ",
                              _pv_display_for_dump(d, ptr - delta, delta, 0,
                                         pvlim));
            }
            if (type == SVt_INVLIST) {
                PerlIO_printf(file, "\n");
                /* 4 blanks indents 2 beyond the PV, etc */
                _invlist_dump(file, level, "    ", sv);
            }
            else {
                PerlIO_printf(file, "%s", _pv_display_for_dump(d, ptr, SvCUR(sv),
                                                     re ? 0 : SvLEN(sv),
                                                     pvlim));
                if (SvUTF8(sv)) /* the 6?  \x{....} */
                    PerlIO_printf(file, " [UTF8 \"%s\"]",
                                         sv_uni_display(d, sv, 6 * SvCUR(sv),
                                                        UNI_DISPLAY_QQ));
                if (SvIsBOOL(sv))
                    PerlIO_printf(file, " [BOOL %s]", ptr == PL_Yes ? "PL_Yes" : "PL_No");
                PerlIO_printf(file, "\n");
            }
            Perl_dump_indent(aTHX_ level, file, "  CUR = %" IVdf "\n", (IV)SvCUR(sv));
            if (re && type == SVt_PVLV)
                /* LV-as-REGEXP usurps len field to store pointer to
                 * regexp struct */
                Perl_dump_indent(aTHX_ level, file, "  REGEXP = 0x%" UVxf "\n",
                   PTR2UV(((XPV*)SvANY(sv))->xpv_len_u.xpvlenu_rx));
            else
                Perl_dump_indent(aTHX_ level, file, "  LEN = %" IVdf "\n",
                                       (IV)SvLEN(sv));
#ifdef PERL_COPY_ON_WRITE
            if (SvIsCOW(sv) && SvLEN(sv))
                Perl_dump_indent(aTHX_ level, file, "  COW_REFCNT = %d\n",
                                       CowREFCNT(sv));
#endif
        }
        else
            Perl_dump_indent(aTHX_ level, file, "  PV = 0\n");
    }

    if (type >= SVt_PVMG) {
        if (SvMAGIC(sv))
                do_magic_dump(level, file, SvMAGIC(sv), nest+1, maxnest, dumpops, pvlim);
        if (SvSTASH(sv))
            do_hv_dump(level, file, "  STASH", SvSTASH(sv));

        if ((type == SVt_PVMG || type == SVt_PVLV) && SvVALID(sv)) {
            Perl_dump_indent(aTHX_ level, file, "  USEFUL = %" IVdf "\n",
                                   (IV)BmUSEFUL(sv));
        }
    }

    /* Dump type-specific SV fields */

    switch (type) {
    case SVt_PVAV:
        Perl_dump_indent(aTHX_ level, file, "  ARRAY = 0x%" UVxf,
                               PTR2UV(AvARRAY(sv)));
        if (AvARRAY(sv) != AvALLOC(sv)) {
            PerlIO_printf(file, " (offset=%" IVdf ")\n",
                                (IV)(AvARRAY(sv) - AvALLOC(sv)));
            Perl_dump_indent(aTHX_ level, file, "  ALLOC = 0x%" UVxf "\n",
                                   PTR2UV(AvALLOC(sv)));
        }
        else
            (void)PerlIO_putc(file, '\n');
        Perl_dump_indent(aTHX_ level, file, "  FILL = %" IVdf "\n",
                               (IV)AvFILLp(sv));
        Perl_dump_indent(aTHX_ level, file, "  MAX = %" IVdf "\n",
                               (IV)AvMAX(sv));
        SvPVCLEAR(d);
        if (AvREAL(sv))	sv_catpvs(d, ",REAL");
        if (AvREIFY(sv))	sv_catpvs(d, ",REIFY");
        Perl_dump_indent(aTHX_ level, file, "  FLAGS = (%s)\n",
                         SvCUR(d) ? SvPVX_const(d) + 1 : "");
        if (nest < maxnest && AvARRAY(MUTABLE_AV(sv))) {
            SSize_t count;
            SV **svp = AvARRAY(MUTABLE_AV(sv));
            for (count = 0;
                 count <= AvFILLp(MUTABLE_AV(sv)) && count < maxnest;
                 count++, svp++)
            {
                SV* const elt = *svp;
                Perl_dump_indent(aTHX_ level + 1, file, "Elt No. %" IVdf "\n",
                                       (IV)count);
                do_sv_dump(level+1, file, elt, nest+1, maxnest, dumpops, pvlim);
            }
        }
        break;
    case SVt_PVHV: {
        U32 totalkeys;
        if (HvHasAUX(sv)) {
            struct xpvhv_aux *const aux = HvAUX(sv);
            Perl_dump_indent(aTHX_ level, file, "  AUX_FLAGS = %" UVuf "\n",
                             (UV)aux->xhv_aux_flags);
        }
        Perl_dump_indent(aTHX_ level, file, "  ARRAY = 0x%" UVxf, PTR2UV(HvARRAY(sv)));
        totalkeys = HvTOTALKEYS(MUTABLE_HV(sv));
        if (totalkeys) {
            /* Show distribution of HEs in the ARRAY */
            int freq[200];
#define FREQ_MAX ((int)(C_ARRAY_LENGTH(freq) - 1))
            int i;
            int max = 0;
            U32 pow2 = 2;
            U32 keys = totalkeys;
            NV theoret, sum = 0;

            PerlIO_printf(file, "  (");
            Zero(freq, FREQ_MAX + 1, int);
            for (i = 0; (STRLEN)i <= HvMAX(sv); i++) {
                HE* h;
                int count = 0;
                for (h = HvARRAY(sv)[i]; h; h = HeNEXT(h))
                    count++;
                if (count > FREQ_MAX)
                    count = FREQ_MAX;
                freq[count]++;
                if (max < count)
                    max = count;
            }
            for (i = 0; i <= max; i++) {
                if (freq[i]) {
                    PerlIO_printf(file, "%d%s:%d", i,
                                  (i == FREQ_MAX) ? "+" : "",
                                  freq[i]);
                    if (i != max)
                        PerlIO_printf(file, ", ");
                }
            }
            (void)PerlIO_putc(file, ')');
            /* The "quality" of a hash is defined as the total number of
               comparisons needed to access every element once, relative
               to the expected number needed for a random hash.

               The total number of comparisons is equal to the sum of
               the squares of the number of entries in each bucket.
               For a random hash of n keys into k buckets, the expected
               value is
                                n + n(n-1)/2k
            */

            for (i = max; i > 0; i--) { /* Precision: count down. */
                sum += freq[i] * i * i;
            }
            while ((keys = keys >> 1))
                pow2 = pow2 << 1;
            theoret = totalkeys;
            theoret += theoret * (theoret-1)/pow2;
            (void)PerlIO_putc(file, '\n');
            Perl_dump_indent(aTHX_ level, file, "  hash quality = %.1"
                                   NVff "%%", theoret/sum*100);
        }
        (void)PerlIO_putc(file, '\n');
        Perl_dump_indent(aTHX_ level, file, "  KEYS = %" IVdf "\n",
                               (IV)totalkeys);
        {
            STRLEN count = 0;
            HE **ents = HvARRAY(sv);

            if (ents) {
                HE *const *const last = ents + HvMAX(sv);
                count = last + 1 - ents;
                
                do {
                    if (!*ents)
                        --count;
                } while (++ents <= last);
            }

            Perl_dump_indent(aTHX_ level, file, "  FILL = %" UVuf "\n",
                             (UV)count);
        }
        Perl_dump_indent(aTHX_ level, file, "  MAX = %" IVdf "\n",
                               (IV)HvMAX(sv));
        if (HvHasAUX(sv)) {
            Perl_dump_indent(aTHX_ level, file, "  RITER = %" IVdf "\n",
                                   (IV)HvRITER_get(sv));
            Perl_dump_indent(aTHX_ level, file, "  EITER = 0x%" UVxf "\n",
                                   PTR2UV(HvEITER_get(sv)));
#ifdef PERL_HASH_RANDOMIZE_KEYS
            Perl_dump_indent(aTHX_ level, file, "  RAND = 0x%" UVxf,
                                   (UV)HvRAND_get(sv));
            if (HvRAND_get(sv) != HvLASTRAND_get(sv) && HvRITER_get(sv) != -1 ) {
                PerlIO_printf(file, " (LAST = 0x%" UVxf ")",
                                    (UV)HvLASTRAND_get(sv));
            }
#endif
            (void)PerlIO_putc(file, '\n');
        }
        {
            MAGIC * const mg = mg_find(sv, PERL_MAGIC_symtab);
            if (mg && mg->mg_obj) {
                Perl_dump_indent(aTHX_ level, file, "  PMROOT = 0x%" UVxf "\n", PTR2UV(mg->mg_obj));
            }
        }
        {
            const char * const hvname = HvNAME_get(sv);
            if (hvname) {
                SV* tmpsv = newSVpvs_flags("", SVs_TEMP);
                Perl_dump_indent(aTHX_ level, file, "  NAME = \"%s\"\n",
                                       generic_pv_escape( tmpsv, hvname,
                                           HvNAMELEN(sv), HvNAMEUTF8(sv)));
        }
        }
        if (HvHasAUX(sv)) {
            AV * const backrefs
                = *Perl_hv_backreferences_p(aTHX_ MUTABLE_HV(sv));
            struct mro_meta * const meta = HvAUX(sv)->xhv_mro_meta;
            if (HvAUX(sv)->xhv_name_count)
                Perl_dump_indent(aTHX_
                 level, file, "  NAMECOUNT = %" IVdf "\n",
                 (IV)HvAUX(sv)->xhv_name_count
                );
            if (HvAUX(sv)->xhv_name_u.xhvnameu_name && HvENAME_HEK_NN(sv)) {
                const I32 count = HvAUX(sv)->xhv_name_count;
                if (count) {
                    SV * const names = newSVpvs_flags("", SVs_TEMP);
                    /* The starting point is the first element if count is
                       positive and the second element if count is negative. */
                    HEK *const *hekp = HvAUX(sv)->xhv_name_u.xhvnameu_names
                        + (count < 0 ? 1 : 0);
                    HEK *const *const endp = HvAUX(sv)->xhv_name_u.xhvnameu_names
                        + (count < 0 ? -count : count);
                    while (hekp < endp) {
                        if (*hekp) {
                            SV *tmp = newSVpvs_flags("", SVs_TEMP);
                            Perl_sv_catpvf(aTHX_ names, ", \"%s\"",
                              generic_pv_escape(tmp, HEK_KEY(*hekp), HEK_LEN(*hekp), HEK_UTF8(*hekp)));
                        } else {
                            /* This should never happen. */
                            sv_catpvs(names, ", (null)");
                        }
                        ++hekp;
                    }
                    Perl_dump_indent(aTHX_
                     level, file, "  ENAME = %s\n", SvPV_nolen(names)+2
                    );
                }
                else {
                    SV * const tmp = newSVpvs_flags("", SVs_TEMP);
                    const char *const hvename = HvENAME_get(sv);
                    Perl_dump_indent(aTHX_
                     level, file, "  ENAME = \"%s\"\n",
                     generic_pv_escape(tmp, hvename,
                                       HvENAMELEN_get(sv), HvENAMEUTF8(sv)));
                }
            }
            if (backrefs) {
                Perl_dump_indent(aTHX_ level, file, "  BACKREFS = 0x%" UVxf "\n",
                                 PTR2UV(backrefs));
                do_sv_dump(level+1, file, MUTABLE_SV(backrefs), nest+1, maxnest,
                           dumpops, pvlim);
            }
            if (meta) {
                SV* tmpsv = newSVpvs_flags("", SVs_TEMP);
                Perl_dump_indent(aTHX_ level, file, "  MRO_WHICH = \"%s\" (0x%"
                                 UVxf ")\n",
                                 generic_pv_escape( tmpsv, meta->mro_which->name,
                                meta->mro_which->length,
                                (meta->mro_which->kflags & HVhek_UTF8)),
                                 PTR2UV(meta->mro_which));
                Perl_dump_indent(aTHX_ level, file, "  CACHE_GEN = 0x%"
                                 UVxf "\n",
                                 (UV)meta->cache_gen);
                Perl_dump_indent(aTHX_ level, file, "  PKG_GEN = 0x%" UVxf "\n",
                                 (UV)meta->pkg_gen);
                if (meta->mro_linear_all) {
                    Perl_dump_indent(aTHX_ level, file, "  MRO_LINEAR_ALL = 0x%"
                                 UVxf "\n",
                                 PTR2UV(meta->mro_linear_all));
                do_sv_dump(level+1, file, MUTABLE_SV(meta->mro_linear_all), nest+1, maxnest,
                           dumpops, pvlim);
                }
                if (meta->mro_linear_current) {
                    Perl_dump_indent(aTHX_ level, file,
                                 "  MRO_LINEAR_CURRENT = 0x%" UVxf "\n",
                                 PTR2UV(meta->mro_linear_current));
                do_sv_dump(level+1, file, MUTABLE_SV(meta->mro_linear_current), nest+1, maxnest,
                           dumpops, pvlim);
                }
                if (meta->mro_nextmethod) {
                    Perl_dump_indent(aTHX_ level, file,
                                 "  MRO_NEXTMETHOD = 0x%" UVxf "\n",
                                 PTR2UV(meta->mro_nextmethod));
                do_sv_dump(level+1, file, MUTABLE_SV(meta->mro_nextmethod), nest+1, maxnest,
                           dumpops, pvlim);
                }
                if (meta->isa) {
                    Perl_dump_indent(aTHX_ level, file, "  ISA = 0x%" UVxf "\n",
                                 PTR2UV(meta->isa));
                do_sv_dump(level+1, file, MUTABLE_SV(meta->isa), nest+1, maxnest,
                           dumpops, pvlim);
                }
            }
        }
        if (nest < maxnest) {
            HV * const hv = MUTABLE_HV(sv);

            if (HvTOTALKEYS(hv)) {
                STRLEN i;
                int count = maxnest - nest;
                for (i=0; i <= HvMAX(hv); i++) {
                    HE *he;
                    for (he = HvARRAY(hv)[i]; he; he = HeNEXT(he)) {
                        U32 hash;
                        SV * keysv;
                        const char * keypv;
                        SV * elt;
                        STRLEN len;

                        if (count-- <= 0) goto DONEHV;

                        hash = HeHASH(he);
                        keysv = hv_iterkeysv(he);
                        keypv = SvPV_const(keysv, len);
                        elt = HeVAL(he);

                        Perl_dump_indent(aTHX_ level+1, file, "Elt %s ", _pv_display_for_dump(d, keypv, len, 0, pvlim));
                        if (SvUTF8(keysv))
                            PerlIO_printf(file, "[UTF8 \"%s\"] ", sv_uni_display(d, keysv, 6 * SvCUR(keysv), UNI_DISPLAY_QQ));
                        if (HvEITER_get(hv) == he)
                            PerlIO_printf(file, "[CURRENT] ");
                        PerlIO_printf(file, "HASH = 0x%" UVxf, (UV) hash);

                        if (sv == (SV*)PL_strtab)
                            PerlIO_printf(file, " REFCNT = 0x%" UVxf "\n",
                                (UV)he->he_valu.hent_refcount );
                        else {
                            (void)PerlIO_putc(file, '\n');
                            do_sv_dump(level+1, file, elt, nest+1, maxnest, dumpops, pvlim);
                        }
                    }
                }
              DONEHV:;
            }
        }
        break;
    } /* case SVt_PVHV */

    case SVt_PVCV:
        if (CvAUTOLOAD(sv)) {
            SV* tmpsv = newSVpvs_flags("", SVs_TEMP);
            STRLEN len;
            const char *const name =  SvPV_const(sv, len);
            Perl_dump_indent(aTHX_ level, file, "  AUTOLOAD = \"%s\"\n",
                             generic_pv_escape(tmpsv, name, len, SvUTF8(sv)));
        }
        if (SvPOK(sv)) {
            SV* tmpsv = newSVpvs_flags("", SVs_TEMP);
            const char *const proto = CvPROTO(sv);
            Perl_dump_indent(aTHX_ level, file, "  PROTOTYPE = \"%s\"\n",
                             generic_pv_escape(tmpsv, proto, CvPROTOLEN(sv),
                                SvUTF8(sv)));
        }
        /* FALLTHROUGH */
    case SVt_PVFM:
        do_hv_dump(level, file, "  COMP_STASH", CvSTASH(sv));
        if (!CvISXSUB(sv)) {
            if (CvSTART(sv)) {
                if (CvSLABBED(sv))
                    Perl_dump_indent(aTHX_ level, file,
                                 "  SLAB = 0x%" UVxf "\n",
                                 PTR2UV(CvSTART(sv)));
                else
                    Perl_dump_indent(aTHX_ level, file,
                                 "  START = 0x%" UVxf " ===> %" IVdf "\n",
                                 PTR2UV(CvSTART(sv)),
                                 (IV)sequence_num(CvSTART(sv)));
            }
            Perl_dump_indent(aTHX_ level, file, "  ROOT = 0x%" UVxf "\n",
                             PTR2UV(CvROOT(sv)));
            if (CvROOT(sv) && dumpops) {
                do_op_dump(level+1, file, CvROOT(sv));
            }
        } else {
            SV * const constant = cv_const_sv((const CV *)sv);

            Perl_dump_indent(aTHX_ level, file, "  XSUB = 0x%" UVxf "\n", PTR2UV(CvXSUB(sv)));

            if (constant) {
                Perl_dump_indent(aTHX_ level, file, "  XSUBANY = 0x%" UVxf
                                 " (CONST SV)\n",
                                 PTR2UV(CvXSUBANY(sv).any_ptr));
                do_sv_dump(level+1, file, constant, nest+1, maxnest, dumpops,
                           pvlim);
            } else {
                Perl_dump_indent(aTHX_ level, file, "  XSUBANY = %" IVdf "\n",
                                 (IV)CvXSUBANY(sv).any_i32);
            }
        }
        if (CvNAMED(sv))
            Perl_dump_indent(aTHX_ level, file, "  NAME = \"%s\"\n",
                                   HEK_KEY(CvNAME_HEK((CV *)sv)));
        else do_gvgv_dump(level, file, "  GVGV::GV", CvGV(sv));
        Perl_dump_indent(aTHX_ level, file, "  FILE = \"%s\"\n", CvFILE(sv));
        Perl_dump_indent(aTHX_ level, file, "  DEPTH = %"
                                      IVdf "\n", (IV)CvDEPTH(sv));
        Perl_dump_indent(aTHX_ level, file, "  FLAGS = 0x%" UVxf "\n",
                               (UV)CvFLAGS(sv));
        Perl_dump_indent(aTHX_ level, file, "  OUTSIDE_SEQ = %" UVuf "\n", (UV)CvOUTSIDE_SEQ(sv));
        if (!CvISXSUB(sv)) {
            Perl_dump_indent(aTHX_ level, file, "  PADLIST = 0x%" UVxf "\n", PTR2UV(CvPADLIST(sv)));
            if (nest < maxnest) {
                do_dump_pad(level+1, file, CvPADLIST(sv), 0);
            }
        }
        else
            Perl_dump_indent(aTHX_ level, file, "  HSCXT = 0x%p\n", CvHSCXT(sv));
        {
            const CV * const outside = CvOUTSIDE(sv);
            Perl_dump_indent(aTHX_ level, file, "  OUTSIDE = 0x%" UVxf " (%s)\n",
                        PTR2UV(outside),
                        (!outside ? "null"
                         : CvANON(outside) ? "ANON"
                         : (outside == PL_main_cv) ? "MAIN"
                         : CvUNIQUE(outside) ? "UNIQUE"
                         : CvGV(outside) ?
                             generic_pv_escape(
                                 newSVpvs_flags("", SVs_TEMP),
                                 GvNAME(CvGV(outside)),
                                 GvNAMELEN(CvGV(outside)),
                                 GvNAMEUTF8(CvGV(outside)))
                         : "UNDEFINED"));
        }
        if (CvOUTSIDE(sv)
         && (nest < maxnest && (CvCLONE(sv) || CvCLONED(sv))))
            do_sv_dump(level+1, file, MUTABLE_SV(CvOUTSIDE(sv)), nest+1, maxnest, dumpops, pvlim);
        break;

    case SVt_PVGV:
    case SVt_PVLV:
        if (type == SVt_PVLV) {
            Perl_dump_indent(aTHX_ level, file, "  TYPE = %c\n", LvTYPE(sv));
            Perl_dump_indent(aTHX_ level, file, "  TARGOFF = %" IVdf "\n", (IV)LvTARGOFF(sv));
            Perl_dump_indent(aTHX_ level, file, "  TARGLEN = %" IVdf "\n", (IV)LvTARGLEN(sv));
            Perl_dump_indent(aTHX_ level, file, "  TARG = 0x%" UVxf "\n", PTR2UV(LvTARG(sv)));
            Perl_dump_indent(aTHX_ level, file, "  FLAGS = %" IVdf "\n", (IV)LvFLAGS(sv));
            if (isALPHA_FOLD_NE(LvTYPE(sv), 't'))
                do_sv_dump(level+1, file, LvTARG(sv), nest+1, maxnest,
                    dumpops, pvlim);
        }
        if (isREGEXP(sv)) goto dumpregexp;
        if (!isGV_with_GP(sv))
            break;
        {
            SV* tmpsv = newSVpvs_flags("", SVs_TEMP);
            Perl_dump_indent(aTHX_ level, file, "  NAME = \"%s\"\n",
                     generic_pv_escape(tmpsv, GvNAME(sv),
                                       GvNAMELEN(sv),
                                       GvNAMEUTF8(sv)));
        }
        Perl_dump_indent(aTHX_ level, file, "  NAMELEN = %" IVdf "\n", (IV)GvNAMELEN(sv));
        do_hv_dump (level, file, "  GvSTASH", GvSTASH(sv));
        Perl_dump_indent(aTHX_ level, file, "  FLAGS = 0x%" UVxf "\n", (UV)GvFLAGS(sv));
        Perl_dump_indent(aTHX_ level, file, "  GP = 0x%" UVxf "\n", PTR2UV(GvGP(sv)));
        if (!GvGP(sv))
            break;
        Perl_dump_indent(aTHX_ level, file, "    SV = 0x%" UVxf "\n", PTR2UV(GvSV(sv)));
        Perl_dump_indent(aTHX_ level, file, "    REFCNT = %" IVdf "\n", (IV)GvREFCNT(sv));
        Perl_dump_indent(aTHX_ level, file, "    IO = 0x%" UVxf "\n", PTR2UV(GvIOp(sv)));
        Perl_dump_indent(aTHX_ level, file, "    FORM = 0x%" UVxf "  \n", PTR2UV(GvFORM(sv)));
        Perl_dump_indent(aTHX_ level, file, "    AV = 0x%" UVxf "\n", PTR2UV(GvAV(sv)));
        Perl_dump_indent(aTHX_ level, file, "    HV = 0x%" UVxf "\n", PTR2UV(GvHV(sv)));
        Perl_dump_indent(aTHX_ level, file, "    CV = 0x%" UVxf "\n", PTR2UV(GvCV(sv)));
        Perl_dump_indent(aTHX_ level, file, "    CVGEN = 0x%" UVxf "\n", (UV)GvCVGEN(sv));
        Perl_dump_indent(aTHX_ level, file, "    GPFLAGS = 0x%" UVxf
                                            " (%s)\n",
                               (UV)GvGPFLAGS(sv),
                               "");
        Perl_dump_indent(aTHX_ level, file, "    LINE = %" LINE_Tf "\n", (line_t)GvLINE(sv));
        Perl_dump_indent(aTHX_ level, file, "    FILE = \"%s\"\n", GvFILE(sv));
        do_gv_dump (level, file, "    EGV", GvEGV(sv));
        break;
    case SVt_PVIO:
        Perl_dump_indent(aTHX_ level, file, "  IFP = 0x%" UVxf "\n", PTR2UV(IoIFP(sv)));
        Perl_dump_indent(aTHX_ level, file, "  OFP = 0x%" UVxf "\n", PTR2UV(IoOFP(sv)));
        Perl_dump_indent(aTHX_ level, file, "  DIRP = 0x%" UVxf "\n", PTR2UV(IoDIRP(sv)));
        Perl_dump_indent(aTHX_ level, file, "  LINES = %" IVdf "\n", (IV)IoLINES(sv));
        Perl_dump_indent(aTHX_ level, file, "  PAGE = %" IVdf "\n", (IV)IoPAGE(sv));
        Perl_dump_indent(aTHX_ level, file, "  PAGE_LEN = %" IVdf "\n", (IV)IoPAGE_LEN(sv));
        Perl_dump_indent(aTHX_ level, file, "  LINES_LEFT = %" IVdf "\n", (IV)IoLINES_LEFT(sv));
        if (IoTOP_NAME(sv))
            Perl_dump_indent(aTHX_ level, file, "  TOP_NAME = \"%s\"\n", IoTOP_NAME(sv));
        if (!IoTOP_GV(sv) || SvTYPE(IoTOP_GV(sv)) == SVt_PVGV)
            do_gv_dump (level, file, "  TOP_GV", IoTOP_GV(sv));
        else {
            Perl_dump_indent(aTHX_ level, file, "  TOP_GV = 0x%" UVxf "\n",
                             PTR2UV(IoTOP_GV(sv)));
            do_sv_dump (level+1, file, MUTABLE_SV(IoTOP_GV(sv)), nest+1,
                        maxnest, dumpops, pvlim);
        }
        /* Source filters hide things that are not GVs in these three, so let's
           be careful out there.  */
        if (IoFMT_NAME(sv))
            Perl_dump_indent(aTHX_ level, file, "  FMT_NAME = \"%s\"\n", IoFMT_NAME(sv));
        if (!IoFMT_GV(sv) || SvTYPE(IoFMT_GV(sv)) == SVt_PVGV)
            do_gv_dump (level, file, "  FMT_GV", IoFMT_GV(sv));
        else {
            Perl_dump_indent(aTHX_ level, file, "  FMT_GV = 0x%" UVxf "\n",
                             PTR2UV(IoFMT_GV(sv)));
            do_sv_dump (level+1, file, MUTABLE_SV(IoFMT_GV(sv)), nest+1,
                        maxnest, dumpops, pvlim);
        }
        if (IoBOTTOM_NAME(sv))
            Perl_dump_indent(aTHX_ level, file, "  BOTTOM_NAME = \"%s\"\n", IoBOTTOM_NAME(sv));
        if (!IoBOTTOM_GV(sv) || SvTYPE(IoBOTTOM_GV(sv)) == SVt_PVGV)
            do_gv_dump (level, file, "  BOTTOM_GV", IoBOTTOM_GV(sv));
        else {
            Perl_dump_indent(aTHX_ level, file, "  BOTTOM_GV = 0x%" UVxf "\n",
                             PTR2UV(IoBOTTOM_GV(sv)));
            do_sv_dump (level+1, file, MUTABLE_SV(IoBOTTOM_GV(sv)), nest+1,
                        maxnest, dumpops, pvlim);
        }
        if (isPRINT(IoTYPE(sv)))
            Perl_dump_indent(aTHX_ level, file, "  TYPE = '%c'\n", IoTYPE(sv));
        else
            Perl_dump_indent(aTHX_ level, file, "  TYPE = '\\%o'\n", IoTYPE(sv));
        Perl_dump_indent(aTHX_ level, file, "  FLAGS = 0x%" UVxf "\n", (UV)IoFLAGS(sv));
        break;
    case SVt_REGEXP:
      dumpregexp:
        {
            struct regexp * const r = ReANY((REGEXP*)sv);

#define SV_SET_STRINGIFY_REGEXP_FLAGS(d,flags,names) STMT_START { \
            sv_setpv(d,"");                                 \
            append_flags(d, flags, names);     \
            if (SvCUR(d) > 0 && *(SvEND(d) - 1) == ',') {       \
                SvCUR_set(d, SvCUR(d) - 1);                 \
                SvPVX(d)[SvCUR(d)] = '\0';                  \
            }                                               \
} STMT_END
            SV_SET_STRINGIFY_REGEXP_FLAGS(d,r->compflags,regexp_extflags_names);
            Perl_dump_indent(aTHX_ level, file, "  COMPFLAGS = 0x%" UVxf " (%s)\n",
                                (UV)(r->compflags), SvPVX_const(d));

            SV_SET_STRINGIFY_REGEXP_FLAGS(d,r->extflags,regexp_extflags_names);
            Perl_dump_indent(aTHX_ level, file, "  EXTFLAGS = 0x%" UVxf " (%s)\n",
                                (UV)(r->extflags), SvPVX_const(d));

            Perl_dump_indent(aTHX_ level, file, "  ENGINE = 0x%" UVxf " (%s)\n",
                                PTR2UV(r->engine), (r->engine == &PL_core_reg_engine) ? "STANDARD" : "PLUG-IN" );
            if (r->engine == &PL_core_reg_engine) {
                SV_SET_STRINGIFY_REGEXP_FLAGS(d,r->intflags,regexp_core_intflags_names);
                Perl_dump_indent(aTHX_ level, file, "  INTFLAGS = 0x%" UVxf " (%s)\n",
                                (UV)(r->intflags), SvPVX_const(d));
            } else {
                Perl_dump_indent(aTHX_ level, file, "  INTFLAGS = 0x%" UVxf "(Plug in)\n",
                                (UV)(r->intflags));
            }
#undef SV_SET_STRINGIFY_REGEXP_FLAGS
            Perl_dump_indent(aTHX_ level, file, "  NPARENS = %" UVuf "\n",
                                (UV)(r->nparens));
            Perl_dump_indent(aTHX_ level, file, "  LOGICAL_NPARENS = %" UVuf "\n",
                                (UV)(r->logical_nparens));

#define SV_SET_STRINGIFY_I32_PAREN_ARRAY(d,count,ary)     \
    STMT_START {                                    \
        U32 n;                                      \
        sv_setpv(d,"{ ");                           \
        /* 0 element is irrelevant */               \
        for(n=0; n <= count; n++)                   \
            sv_catpvf(d,"%" IVdf "%s",              \
                (IV)ary[n],                         \
                n == count ? "" : ", ");            \
        sv_catpvs(d," }\n");                        \
    } STMT_END

            Perl_dump_indent(aTHX_ level, file, "  LOGICAL_TO_PARNO = 0x%" UVxf "\n",
                                PTR2UV(r->logical_to_parno));
            if (r->logical_to_parno) {
                SV_SET_STRINGIFY_I32_PAREN_ARRAY(d, r->logical_nparens, r->logical_to_parno);
                Perl_dump_indent(aTHX_ level, file, "    %" SVf, d);
            }
            Perl_dump_indent(aTHX_ level, file, "  PARNO_TO_LOGICAL = 0x%" UVxf "\n",
                                PTR2UV(r->parno_to_logical));
            if (r->parno_to_logical) {
                SV_SET_STRINGIFY_I32_PAREN_ARRAY(d, r->nparens, r->parno_to_logical);
                Perl_dump_indent(aTHX_ level, file, "    %" SVf, d);
            }

            Perl_dump_indent(aTHX_ level, file, "  PARNO_TO_LOGICAL_NEXT = 0x%" UVxf "\n",
                                PTR2UV(r->parno_to_logical_next));
            if (r->parno_to_logical_next) {
                SV_SET_STRINGIFY_I32_PAREN_ARRAY(d, r->nparens, r->parno_to_logical_next);
                Perl_dump_indent(aTHX_ level, file, "    %" SVf, d);
            }
#undef SV_SET_STRINGIFY_I32_ARRAY

            Perl_dump_indent(aTHX_ level, file, "  LASTPAREN = %" UVuf "\n",
                                (UV)(RXp_LASTPAREN(r)));
            Perl_dump_indent(aTHX_ level, file, "  LASTCLOSEPAREN = %" UVuf "\n",
                                (UV)(RXp_LASTCLOSEPAREN(r)));
            Perl_dump_indent(aTHX_ level, file, "  MINLEN = %" IVdf "\n",
                                (IV)(RXp_MINLEN(r)));
            Perl_dump_indent(aTHX_ level, file, "  MINLENRET = %" IVdf "\n",
                                (IV)(RXp_MINLENRET(r)));
            Perl_dump_indent(aTHX_ level, file, "  GOFS = %" UVuf "\n",
                                (UV)(RXp_GOFS(r)));
            Perl_dump_indent(aTHX_ level, file, "  PRE_PREFIX = %" UVuf "\n",
                                (UV)(RXp_PRE_PREFIX(r)));
            Perl_dump_indent(aTHX_ level, file, "  SUBLEN = %" IVdf "\n",
                                (IV)(RXp_SUBLEN(r)));
            Perl_dump_indent(aTHX_ level, file, "  SUBOFFSET = %" IVdf "\n",
                                (IV)(RXp_SUBOFFSET(r)));
            Perl_dump_indent(aTHX_ level, file, "  SUBCOFFSET = %" IVdf "\n",
                                (IV)(RXp_SUBCOFFSET(r)));
            if (RXp_SUBBEG(r))
                Perl_dump_indent(aTHX_ level, file, "  SUBBEG = 0x%" UVxf " %s\n",
                            PTR2UV(RXp_SUBBEG(r)),
                            pv_display(d, RXp_SUBBEG(r), RXp_SUBLEN(r), 50, pvlim));
            else
                Perl_dump_indent(aTHX_ level, file, "  SUBBEG = 0x0\n");
            Perl_dump_indent(aTHX_ level, file, "  PAREN_NAMES = 0x%" UVxf "\n",
                                PTR2UV(RXp_PAREN_NAMES(r)));
            Perl_dump_indent(aTHX_ level, file, "  SUBSTRS = 0x%" UVxf "\n",
                                PTR2UV(RXp_SUBSTRS(r)));
            Perl_dump_indent(aTHX_ level, file, "  PPRIVATE = 0x%" UVxf "\n",
                                PTR2UV(RXp_PPRIVATE(r)));
            Perl_dump_indent(aTHX_ level, file, "  OFFS = 0x%" UVxf "\n",
                                PTR2UV(RXp_OFFSp(r)));
            if (RXp_OFFSp(r)) {
                U32 n;
                sv_setpvs(d,"[ ");
                /* note offs[0] is for the whole match, and
                 * the data for $1 is in offs[1]. Thus we have to
                 * show one more than we have nparens. */
                for(n = 0; n <= r->nparens; n++) {
                    sv_catpvf(d,"%" IVdf ":%" IVdf "%s",
                        (IV)RXp_OFFSp(r)[n].start, (IV)RXp_OFFSp(r)[n].end,
                        n+1 > r->nparens ? " ]\n" : ", ");
                }
                Perl_dump_indent(aTHX_ level, file, "    %" SVf, d);
            }
            Perl_dump_indent(aTHX_ level, file, "  QR_ANONCV = 0x%" UVxf "\n",
                                PTR2UV(RXp_QR_ANONCV(r)));
#ifdef PERL_ANY_COW
            Perl_dump_indent(aTHX_ level, file, "  SAVED_COPY = 0x%" UVxf "\n",
                                PTR2UV(RXp_SAVED_COPY(r)));
#endif
            /* this should go LAST or the output gets really confusing */
            Perl_dump_indent(aTHX_ level, file, "  MOTHER_RE = 0x%" UVxf "\n",
                                PTR2UV(RXp_MOTHER_RE(r)));
            if (nest < maxnest && RXp_MOTHER_RE(r))
                do_sv_dump(level+1, file, (SV *)RXp_MOTHER_RE(r), nest+1,
                           maxnest, dumpops, pvlim);
        }
        break;
    case SVt_PVOBJ:
        Perl_dump_indent(aTHX_ level, file, "  MAXFIELD = %" IVdf "\n",
                (IV)ObjectMAXFIELD(sv));
        Perl_dump_indent(aTHX_ level, file, "  FIELDS = 0x%" UVxf "\n",
                PTR2UV(ObjectFIELDS(sv)));
        if (nest < maxnest && ObjectFIELDS(sv)) {
            SSize_t count;
            SV **svp = ObjectFIELDS(sv);
            PADNAME **pname = PadnamelistARRAY(HvAUX(SvSTASH(sv))->xhv_class_fields);
            for (count = 0;
                 count <= ObjectMAXFIELD(sv) && count < maxnest;
                 count++, svp++)
            {
                SV *const field = *svp;
                PADNAME *pn = pname[count];

                Perl_dump_indent(aTHX_ level + 1, file, "Field No. %" IVdf " (%s)\n",
                        (IV)count, PadnamePV(pn));

                do_sv_dump(level+1, file, field, nest+1, maxnest, dumpops, pvlim);
            }
        }
        break;
    }
    SvREFCNT_dec_NN(d);
}

/*
=for apidoc sv_dump

Dumps the contents of an SV to the C<STDERR> filehandle.

For an example of its output, see L<Devel::Peek>. If
the item is an SvROK it will dump items to a depth of 4,
otherwise it will dump only the top level item, which
means that it will not dump the contents of an AV * or
HV *. For that use C<av_dump()> or C<hv_dump()>.

=for apidoc sv_dump_depth

Dumps the contents of an SV to the C<STDERR> filehandle
to the depth requested. This function can be used on any
SV derived type (GV, HV, AV) with an appropriate cast.
This is a more flexible variant of sv_dump(). For example

    HV *hv = ...;
    sv_dump_depth((SV*)hv, 2);

would dump the hv, its keys and values, but would not recurse
into any RV values.

=for apidoc av_dump

Dumps the contents of an AV to the C<STDERR> filehandle,
Similar to using Devel::Peek on an arrayref but does not
expect an RV wrapper. Dumps contents to a depth of 3 levels
deep.

=for apidoc hv_dump

Dumps the contents of an HV to the C<STDERR> filehandle.
Similar to using Devel::Peek on an hashref but does not
expect an RV wrapper. Dumps contents to a depth of 3 levels
deep.

=cut
*/

void
Perl_sv_dump(pTHX_ SV *sv)
{
    if (sv && SvROK(sv))
        sv_dump_depth(sv, 4);
    else
        sv_dump_depth(sv, 0);
}

void
Perl_sv_dump_depth(pTHX_ SV *sv, I32 depth)
{
    do_sv_dump(0, Perl_debug_log, sv, 0, depth, 0, 0);
}

void
Perl_av_dump(pTHX_ AV *av)
{
    sv_dump_depth((SV*)av, 3);
}

void
Perl_hv_dump(pTHX_ HV *hv)
{
    sv_dump_depth((SV*)hv, 3);
}

int
Perl_runops_debug(pTHX)
{
#if defined DEBUGGING && !defined DEBUGGING_RE_ONLY
    SSize_t orig_stack_hwm = PL_curstackinfo->si_stack_hwm;

    PL_curstackinfo->si_stack_hwm = PL_stack_sp - PL_stack_base;
#endif

    if (!PL_op) {
        Perl_ck_warner_d(aTHX_ packWARN(WARN_DEBUGGING), "NULL OP IN RUN");
        return 0;
    }
    DEBUG_l(Perl_deb(aTHX_ "Entering new RUNOPS level\n"));
    do {
#ifdef PERL_TRACE_OPS
        ++PL_op_exec_cnt[PL_op->op_type];
#endif
#if defined DEBUGGING && !defined DEBUGGING_RE_ONLY
        if (PL_curstackinfo->si_stack_hwm < PL_stack_sp - PL_stack_base)
            Perl_croak_nocontext(
                "panic: previous op failed to extend arg stack: "
                "base=%p, sp=%p, hwm=%p\n",
                    PL_stack_base, PL_stack_sp,
                    PL_stack_base + PL_curstackinfo->si_stack_hwm);
        PL_curstackinfo->si_stack_hwm = PL_stack_sp - PL_stack_base;
#endif
        if (PL_debug) {
            ENTER;
            SAVETMPS;
            if (PL_watchaddr && (*PL_watchaddr != PL_watchok))
                PerlIO_printf(Perl_debug_log,
                              "WARNING: %" UVxf " changed from %" UVxf " to %" UVxf "\n",
                              PTR2UV(PL_watchaddr), PTR2UV(PL_watchok),
                              PTR2UV(*PL_watchaddr));
            if (DEBUG_s_TEST_) {
                if (DEBUG_v_TEST_) {
                    PerlIO_printf(Perl_debug_log, "\n");
                    deb_stack_all();
                }
                else
                    debstack();
            }


            if (DEBUG_t_TEST_) debop(PL_op);
            if (DEBUG_P_TEST_) debprof(PL_op);
            FREETMPS;
            LEAVE;
        }

        PERL_DTRACE_PROBE_OP(PL_op);
    } while ((PL_op = PL_op->op_ppaddr(aTHX)));
    DEBUG_l(Perl_deb(aTHX_ "leaving RUNOPS level\n"));
    PERL_ASYNC_CHECK();

#if defined DEBUGGING && !defined DEBUGGING_RE_ONLY
    if (PL_curstackinfo->si_stack_hwm < orig_stack_hwm)
        PL_curstackinfo->si_stack_hwm = orig_stack_hwm;
#endif
    TAINT_NOT;
    return 0;
}


/* print the names of the n lexical vars starting at pad offset off */

STATIC void
S_deb_padvar(pTHX_ PADOFFSET off, int n, bool paren)
{
    PADNAME *sv;
    CV * const cv = deb_curcv(cxstack_ix);
    PADNAMELIST *comppad = NULL;
    int i;

    if (cv) {
        PADLIST * const padlist = CvPADLIST(cv);
        comppad = PadlistNAMES(padlist);
    }
    if (paren)
        PerlIO_printf(Perl_debug_log, "(");
    for (i = 0; i < n; i++) {
        if (comppad && (sv = padnamelist_fetch(comppad, off + i)))
            PerlIO_printf(Perl_debug_log, "%" PNf, PNfARG(sv));
        else
            PerlIO_printf(Perl_debug_log, "[%" UVuf "]",
                    (UV)(off+i));
        if (i < n - 1)
            PerlIO_printf(Perl_debug_log, ",");
    }
    if (paren)
        PerlIO_printf(Perl_debug_log, ")");
}


/* append to the out SV, the name of the lexical at offset off in the CV
 * cv */

static void
S_append_padvar(pTHX_ PADOFFSET off, CV *cv, SV *out, int n,
        bool paren, bool is_scalar)
{
    PADNAME *sv;
    PADNAMELIST *namepad = NULL;
    int i;

    if (cv) {
        PADLIST * const padlist = CvPADLIST(cv);
        namepad = PadlistNAMES(padlist);
    }

    if (paren)
        sv_catpvs_nomg(out, "(");
    for (i = 0; i < n; i++) {
        if (namepad && (sv = padnamelist_fetch(namepad, off + i)))
        {
            STRLEN cur = SvCUR(out);
            Perl_sv_catpvf(aTHX_ out, "[%" UTF8f,
                                 UTF8fARG(1, PadnameLEN(sv) - 1,
                                          PadnamePV(sv) + 1));
            if (is_scalar)
                SvPVX(out)[cur] = '$';
        }
        else
            Perl_sv_catpvf(aTHX_ out, "[%" UVuf "]", (UV)(off+i));
        if (i < n - 1)
            sv_catpvs_nomg(out, ",");
    }
    if (paren)
        sv_catpvs_nomg(out, "(");
}


static void
S_append_gv_name(pTHX_ GV *gv, SV *out)
{
    SV *sv;
    if (!gv) {
        sv_catpvs_nomg(out, "<NULLGV>");
        return;
    }
    sv = newSV_type(SVt_NULL);
    gv_fullname4(sv, gv, NULL, FALSE);
    Perl_sv_catpvf(aTHX_ out, "$%" SVf, SVfARG(sv));
    SvREFCNT_dec_NN(sv);
}

#ifdef USE_ITHREADS
#  define ITEM_SV(item) (comppad ? \
    *av_fetch(comppad, (item)->pad_offset, FALSE) : NULL);
#else
#  define ITEM_SV(item) UNOP_AUX_item_sv(item)
#endif


/* return a temporary SV containing a stringified representation of
 * the op_aux field of a MULTIDEREF op, associated with CV cv
 */

SV*
Perl_multideref_stringify(pTHX_ const OP *o, CV *cv)
{
    UNOP_AUX_item *items = cUNOP_AUXo->op_aux;
    UV actions = items->uv;
    SV *sv;
    bool last = 0;
    bool is_hash = FALSE;
    int derefs = 0;
    SV *out = newSVpvn_flags("",0,SVs_TEMP);
#ifdef USE_ITHREADS
    PAD *comppad;

    if (cv) {
        PADLIST *padlist = CvPADLIST(cv);
        comppad = PadlistARRAY(padlist)[1];
    }
    else
        comppad = NULL;
#endif

    PERL_ARGS_ASSERT_MULTIDEREF_STRINGIFY;

    while (!last) {
        switch (actions & MDEREF_ACTION_MASK) {

        case MDEREF_reload:
            actions = (++items)->uv;
            continue;
            NOT_REACHED; /* NOTREACHED */

        case MDEREF_HV_padhv_helem:
            is_hash = TRUE;
            /* FALLTHROUGH */
        case MDEREF_AV_padav_aelem:
            derefs = 1;
            S_append_padvar(aTHX_ (++items)->pad_offset, cv, out, 1, 0, 1);
            goto do_elem;
            NOT_REACHED; /* NOTREACHED */

        case MDEREF_HV_gvhv_helem:
            is_hash = TRUE;
            /* FALLTHROUGH */
        case MDEREF_AV_gvav_aelem:
            derefs = 1;
            items++;
            sv = ITEM_SV(items);
            S_append_gv_name(aTHX_ (GV*)sv, out);
            goto do_elem;
            NOT_REACHED; /* NOTREACHED */

        case MDEREF_HV_gvsv_vivify_rv2hv_helem:
            is_hash = TRUE;
            /* FALLTHROUGH */
        case MDEREF_AV_gvsv_vivify_rv2av_aelem:
            items++;
            sv = ITEM_SV(items);
            S_append_gv_name(aTHX_ (GV*)sv, out);
            goto do_vivify_rv2xv_elem;
            NOT_REACHED; /* NOTREACHED */

        case MDEREF_HV_padsv_vivify_rv2hv_helem:
            is_hash = TRUE;
            /* FALLTHROUGH */
        case MDEREF_AV_padsv_vivify_rv2av_aelem:
            S_append_padvar(aTHX_ (++items)->pad_offset, cv, out, 1, 0, 1);
            goto do_vivify_rv2xv_elem;
            NOT_REACHED; /* NOTREACHED */

        case MDEREF_HV_pop_rv2hv_helem:
        case MDEREF_HV_vivify_rv2hv_helem:
            is_hash = TRUE;
            /* FALLTHROUGH */
        do_vivify_rv2xv_elem:
        case MDEREF_AV_pop_rv2av_aelem:
        case MDEREF_AV_vivify_rv2av_aelem:
            if (!derefs++)
                sv_catpvs_nomg(out, "->");
        do_elem:
            if ((actions & MDEREF_INDEX_MASK)== MDEREF_INDEX_none) {
                sv_catpvs_nomg(out, "->");
                last = 1;
                break;
            }

            sv_catpvn_nomg(out, (is_hash ? "{" : "["), 1);
            switch (actions & MDEREF_INDEX_MASK) {
            case MDEREF_INDEX_const:
                if (is_hash) {
                    items++;
                    sv = ITEM_SV(items);
                    if (!sv)
                        sv_catpvs_nomg(out, "???");
                    else {
                        STRLEN cur;
                        char *s;
                        s = SvPV(sv, cur);
                        pv_pretty(out, s, cur, 30,
                                    NULL, NULL,
                                    (PERL_PV_PRETTY_NOCLEAR
                                    |PERL_PV_PRETTY_QUOTE
                                    |PERL_PV_PRETTY_ELLIPSES));
                    }
                }
                else
                    Perl_sv_catpvf(aTHX_ out, "%" IVdf, (++items)->iv);
                break;
            case MDEREF_INDEX_padsv:
                S_append_padvar(aTHX_ (++items)->pad_offset, cv, out, 1, 0, 1);
                break;
            case MDEREF_INDEX_gvsv:
                items++;
                sv = ITEM_SV(items);
                S_append_gv_name(aTHX_ (GV*)sv, out);
                break;
            }
            sv_catpvn_nomg(out, (is_hash ? "}" : "]"), 1);

            if (actions & MDEREF_FLAG_last)
                last = 1;
            is_hash = FALSE;

            break;

        default:
            PerlIO_printf(Perl_debug_log, "UNKNOWN(%d)",
                (int)(actions & MDEREF_ACTION_MASK));
            last = 1;
            break;

        } /* switch */

        actions >>= MDEREF_SHIFT;
    } /* while */
    return out;
}


/* Return a temporary SV containing a stringified representation of
 * the op_aux field of a MULTICONCAT op. Note that if the aux contains
 * both plain and utf8 versions of the const string and indices, only
 * the first is displayed.
 */

SV*
Perl_multiconcat_stringify(pTHX_ const OP *o)
{
    UNOP_AUX_item *aux = cUNOP_AUXo->op_aux;
    UNOP_AUX_item *lens;
    STRLEN len;
    SSize_t nargs;
    char *s;
    SV *out = newSVpvn_flags("", 0, SVs_TEMP);

    PERL_ARGS_ASSERT_MULTICONCAT_STRINGIFY;

    nargs = aux[PERL_MULTICONCAT_IX_NARGS].ssize;
    s   = aux[PERL_MULTICONCAT_IX_PLAIN_PV].pv;
    len = aux[PERL_MULTICONCAT_IX_PLAIN_LEN].ssize;
    if (!s) {
        s   = aux[PERL_MULTICONCAT_IX_UTF8_PV].pv;
        len = aux[PERL_MULTICONCAT_IX_UTF8_LEN].ssize;
        sv_catpvs(out, "UTF8 ");
    }
    pv_pretty(out, s, len, 50,
                NULL, NULL,
                (PERL_PV_PRETTY_NOCLEAR
                |PERL_PV_PRETTY_QUOTE
                |PERL_PV_PRETTY_ELLIPSES));

    lens = aux + PERL_MULTICONCAT_IX_LENGTHS;
    while (nargs-- >= 0) {
        Perl_sv_catpvf(aTHX_ out, ",%" IVdf, (IV)lens->ssize);
        lens++;
    }
    return out;
}


/*
=for apidoc debop

Implements B<-Dt> perl command line option on OP C<o>.

=cut
*/

I32
Perl_debop(pTHX_ const OP *o)
{
    PERL_ARGS_ASSERT_DEBOP;

    if (CopSTASH_eq(PL_curcop, PL_debstash) && !DEBUG_J_TEST_)
        return 0;

    Perl_deb(aTHX_ "%s", OP_NAME(o));
    switch (o->op_type) {
    case OP_CONST:
    case OP_HINTSEVAL:
        /* With ITHREADS, consts are stored in the pad, and the right pad
         * may not be active here, so check.
         * Looks like only during compiling the pads are illegal.
         */
#ifdef USE_ITHREADS
        if ((((SVOP*)o)->op_sv) || !IN_PERL_COMPILETIME)
#endif
            PerlIO_printf(Perl_debug_log, "(%s)", SvPEEK(cSVOPo_sv));
        break;
    case OP_GVSV:
    case OP_GV:
        PerlIO_printf(Perl_debug_log, "(%" SVf ")",
                SVfARG(S_gv_display(aTHX_ cGVOPo_gv)));
        break;

    case OP_PADSV:
    case OP_PADAV:
    case OP_PADHV:
    case OP_ARGELEM:
        S_deb_padvar(aTHX_ o->op_targ, 1, 1);
        break;

    case OP_PADRANGE:
        S_deb_padvar(aTHX_ o->op_targ,
                        o->op_private & OPpPADRANGE_COUNTMASK, 1);
        break;

    case OP_MULTIDEREF:
        PerlIO_printf(Perl_debug_log, "(%" SVf ")",
            SVfARG(multideref_stringify(o, deb_curcv(cxstack_ix))));
        break;

    case OP_MULTICONCAT:
        PerlIO_printf(Perl_debug_log, "(%" SVf ")",
            SVfARG(multiconcat_stringify(o)));
        break;

    default:
        break;
    }
    PerlIO_printf(Perl_debug_log, "\n");
    return 0;
}


/*
=for apidoc op_class

Given an op, determine what type of struct it has been allocated as.
Returns one of the OPclass enums, such as OPclass_LISTOP.

=cut
*/


OPclass
Perl_op_class(pTHX_ const OP *o)
{
    bool custom = 0;

    if (!o)
        return OPclass_NULL;

    if (o->op_type == 0) {
        if (o->op_targ == OP_NEXTSTATE || o->op_targ == OP_DBSTATE)
            return OPclass_COP;
        return (o->op_flags & OPf_KIDS) ? OPclass_UNOP : OPclass_BASEOP;
    }

    if (o->op_type == OP_SASSIGN)
        return ((o->op_private & OPpASSIGN_BACKWARDS) ? OPclass_UNOP : OPclass_BINOP);

    if (o->op_type == OP_AELEMFAST) {
#ifdef USE_ITHREADS
            return OPclass_PADOP;
#else
            return OPclass_SVOP;
#endif
    }
    
#ifdef USE_ITHREADS
    if (o->op_type == OP_GV || o->op_type == OP_GVSV ||
        o->op_type == OP_RCATLINE)
        return OPclass_PADOP;
#endif

    if (o->op_type == OP_CUSTOM)
        custom = 1;

    switch (OP_CLASS(o)) {
    case OA_BASEOP:
        return OPclass_BASEOP;

    case OA_UNOP:
        return OPclass_UNOP;

    case OA_BINOP:
        return OPclass_BINOP;

    case OA_LOGOP:
        return OPclass_LOGOP;

    case OA_LISTOP:
        return OPclass_LISTOP;

    case OA_PMOP:
        return OPclass_PMOP;

    case OA_SVOP:
        return OPclass_SVOP;

    case OA_PADOP:
        return OPclass_PADOP;

    case OA_PVOP_OR_SVOP:
        /*
         * Character translations (tr///) are usually a PVOP, keeping a 
         * pointer to a table of shorts used to look up translations.
         * Under utf8, however, a simple table isn't practical; instead,
         * the OP is an SVOP (or, under threads, a PADOP),
         * and the SV is an AV.
         */
        return (!custom &&
                   (o->op_private & OPpTRANS_USE_SVOP)
               )
#if  defined(USE_ITHREADS)
                ? OPclass_PADOP : OPclass_PVOP;
#else
                ? OPclass_SVOP : OPclass_PVOP;
#endif

    case OA_LOOP:
        return OPclass_LOOP;

    case OA_COP:
        return OPclass_COP;

    case OA_BASEOP_OR_UNOP:
        /*
         * UNI(OP_foo) in toke.c returns token UNI or FUNC1 depending on
         * whether parens were seen. perly.y uses OPf_SPECIAL to
         * signal whether a BASEOP had empty parens or none.
         * Some other UNOPs are created later, though, so the best
         * test is OPf_KIDS, which is set in newUNOP.
         */
        return (o->op_flags & OPf_KIDS) ? OPclass_UNOP : OPclass_BASEOP;

    case OA_FILESTATOP:
        /*
         * The file stat OPs are created via UNI(OP_foo) in toke.c but use
         * the OPf_REF flag to distinguish between OP types instead of the
         * usual OPf_SPECIAL flag. As usual, if OPf_KIDS is set, then we
         * return OPclass_UNOP so that walkoptree can find our children. If
         * OPf_KIDS is not set then we check OPf_REF. Without OPf_REF set
         * (no argument to the operator) it's an OP; with OPf_REF set it's
         * an SVOP (and op_sv is the GV for the filehandle argument).
         */
        return ((o->op_flags & OPf_KIDS) ? OPclass_UNOP :
#ifdef USE_ITHREADS
                (o->op_flags & OPf_REF) ? OPclass_PADOP : OPclass_BASEOP);
#else
                (o->op_flags & OPf_REF) ? OPclass_SVOP : OPclass_BASEOP);
#endif
    case OA_LOOPEXOP:
        /*
         * next, last, redo, dump and goto use OPf_SPECIAL to indicate that a
         * label was omitted (in which case it's a BASEOP) or else a term was
         * seen. In this last case, all except goto are definitely PVOP but
         * goto is either a PVOP (with an ordinary constant label), an UNOP
         * with OPf_STACKED (with a non-constant non-sub) or an UNOP for
         * OP_REFGEN (with goto &sub) in which case OPf_STACKED also seems to
         * get set.
         */
        if (o->op_flags & OPf_STACKED)
            return OPclass_UNOP;
        else if (o->op_flags & OPf_SPECIAL)
            return OPclass_BASEOP;
        else
            return OPclass_PVOP;
    case OA_METHOP:
        return OPclass_METHOP;
    case OA_UNOP_AUX:
        return OPclass_UNOP_AUX;
    }
    Perl_warn(aTHX_ "Can't determine class of operator %s, assuming BASEOP\n",
         OP_NAME(o));
    return OPclass_BASEOP;
}



STATIC CV*
S_deb_curcv(pTHX_ I32 ix)
{
    PERL_SI *si = PL_curstackinfo;
    for (; ix >=0; ix--) {
        const PERL_CONTEXT * const cx = &(si->si_cxstack)[ix];

        if (CxTYPE(cx) == CXt_SUB || CxTYPE(cx) == CXt_FORMAT)
            return cx->blk_sub.cv;
        else if (CxTYPE(cx) == CXt_EVAL && !CxEVALBLOCK(cx))
            return cx->blk_eval.cv;
        else if (ix == 0 && si->si_type == PERLSI_MAIN)
            return PL_main_cv;
        else if (ix == 0 && CxTYPE(cx) == CXt_NULL
               && si->si_type == PERLSI_SORT)
        {
            /* fake sort sub; use CV of caller */
            si = si->si_prev;
            ix = si->si_cxix + 1;
        }
    }
    return NULL;
}

void
Perl_watch(pTHX_ char **addr)
{
    PERL_ARGS_ASSERT_WATCH;

    PL_watchaddr = addr;
    PL_watchok = *addr;
    PerlIO_printf(Perl_debug_log, "WATCHING, %" UVxf " is currently %" UVxf "\n",
        PTR2UV(PL_watchaddr), PTR2UV(PL_watchok));
}

/*
=for apidoc debprof

Called to indicate that C<o> was executed, for profiling purposes under the
C<-DP> command line option.

=cut
*/

STATIC void
S_debprof(pTHX_ const OP *o)
{
    PERL_ARGS_ASSERT_DEBPROF;

    if (!DEBUG_J_TEST_ && CopSTASH_eq(PL_curcop, PL_debstash))
        return;
    if (!PL_profiledata)
        Newxz(PL_profiledata, MAXO, U32);
    ++PL_profiledata[o->op_type];
}

/*
=for apidoc debprofdump

Dumps the contents of the data collected by the C<-DP> perl command line
option.

=cut
*/

void
Perl_debprofdump(pTHX)
{
    unsigned i;
    if (!PL_profiledata)
        return;
    for (i = 0; i < MAXO; i++) {
        if (PL_profiledata[i])
            PerlIO_printf(Perl_debug_log,
                          "%5lu %s\n", (unsigned long)PL_profiledata[i],
                                       PL_op_name[i]);
    }
}


/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
