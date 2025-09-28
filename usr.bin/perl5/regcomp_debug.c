#ifdef PERL_EXT_RE_BUILD
#include "re_top.h"
#endif

#include "EXTERN.h"
#define PERL_IN_REGEX_ENGINE
#define PERL_IN_REGCOMP_ANY
#define PERL_IN_REGCOMP_DEBUG_C
#include "perl.h"

#ifdef PERL_IN_XSUB_RE
#  include "re_comp.h"
#else
#  include "regcomp.h"
#endif

#include "invlist_inline.h"
#include "unicode_constants.h"
#include "regcomp_internal.h"

#ifdef DEBUGGING

int
Perl_re_printf(pTHX_ const char *fmt, ...)
{
    va_list ap;
    int result;
    PerlIO *f= Perl_debug_log;
    PERL_ARGS_ASSERT_RE_PRINTF;
    va_start(ap, fmt);
    result = PerlIO_vprintf(f, fmt, ap);
    va_end(ap);
    return result;
}

int
Perl_re_indentf(pTHX_ const char *fmt, U32 depth, ...)
{
    va_list ap;
    int result;
    PerlIO *f= Perl_debug_log;
    PERL_ARGS_ASSERT_RE_INDENTF;
    va_start(ap, depth);
    PerlIO_printf(f, "%*s", ( (int)depth % 20 ) * 2, "");
    result = PerlIO_vprintf(f, fmt, ap);
    va_end(ap);
    return result;
}

void
Perl_debug_show_study_flags(pTHX_ U32 flags, const char *open_str,
                                    const char *close_str)
{
    PERL_ARGS_ASSERT_DEBUG_SHOW_STUDY_FLAGS;
    if (!flags)
        return;

    Perl_re_printf( aTHX_  "%s", open_str);
    DEBUG_SHOW_STUDY_FLAG(flags, SF_BEFORE_SEOL);
    DEBUG_SHOW_STUDY_FLAG(flags, SF_BEFORE_MEOL);
    DEBUG_SHOW_STUDY_FLAG(flags, SF_IS_INF);
    DEBUG_SHOW_STUDY_FLAG(flags, SF_HAS_PAR);
    DEBUG_SHOW_STUDY_FLAG(flags, SF_IN_PAR);
    DEBUG_SHOW_STUDY_FLAG(flags, SF_HAS_EVAL);
    DEBUG_SHOW_STUDY_FLAG(flags, SCF_DO_SUBSTR);
    DEBUG_SHOW_STUDY_FLAG(flags, SCF_DO_STCLASS_AND);
    DEBUG_SHOW_STUDY_FLAG(flags, SCF_DO_STCLASS_OR);
    DEBUG_SHOW_STUDY_FLAG(flags, SCF_DO_STCLASS);
    DEBUG_SHOW_STUDY_FLAG(flags, SCF_WHILEM_VISITED_POS);
    DEBUG_SHOW_STUDY_FLAG(flags, SCF_TRIE_RESTUDY);
    DEBUG_SHOW_STUDY_FLAG(flags, SCF_SEEN_ACCEPT);
    DEBUG_SHOW_STUDY_FLAG(flags, SCF_TRIE_DOING_RESTUDY);
    DEBUG_SHOW_STUDY_FLAG(flags, SCF_IN_DEFINE);
    Perl_re_printf( aTHX_  "%s", close_str);
}

void
Perl_debug_studydata(pTHX_ const char *where, scan_data_t *data,
                    U32 depth, int is_inf,
                    SSize_t min, SSize_t stopmin, SSize_t delta)
{
    PERL_ARGS_ASSERT_DEBUG_STUDYDATA;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    DEBUG_OPTIMISE_MORE_r({
        if (!data) {
            Perl_re_indentf(aTHX_  "%s: NO DATA",
                depth,
                where);
            return;
        }
        Perl_re_indentf(aTHX_  "%s: M/S/D: %" IVdf "/%" IVdf "/%" IVdf " Pos:%" IVdf "/%" IVdf " Flags: 0x%" UVXf,
            depth,
            where,
            min, stopmin, delta,
            (IV)data->pos_min,
            (IV)data->pos_delta,
            (UV)data->flags
        );

        Perl_debug_show_study_flags(aTHX_ data->flags," [","]");

        Perl_re_printf( aTHX_
            " Whilem_c: %" IVdf " Lcp: %" IVdf " %s",
            (IV)data->whilem_c,
            (IV)(data->last_closep ? *((data)->last_closep) : -1),
            is_inf ? "INF " : ""
        );

        if (data->last_found) {
            int i;
            Perl_re_printf(aTHX_
                "Last:'%s' %" IVdf ":%" IVdf "/%" IVdf,
                    SvPVX_const(data->last_found),
                    (IV)data->last_end,
                    (IV)data->last_start_min,
                    (IV)data->last_start_max
            );

            for (i = 0; i < 2; i++) {
                Perl_re_printf(aTHX_
                    " %s%s: '%s' @ %" IVdf "/%" IVdf,
                    data->cur_is_floating == i ? "*" : "",
                    i ? "Float" : "Fixed",
                    SvPVX_const(data->substrs[i].str),
                    (IV)data->substrs[i].min_offset,
                    (IV)data->substrs[i].max_offset
                );
                Perl_debug_show_study_flags(aTHX_ data->substrs[i].flags," [","]");
            }
        }

        Perl_re_printf( aTHX_ "\n");
    });
}


void
Perl_debug_peep(pTHX_ const char *str, const RExC_state_t *pRExC_state,
                regnode *scan, U32 depth, U32 flags)
{
    PERL_ARGS_ASSERT_DEBUG_PEEP;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    DEBUG_OPTIMISE_r({
        regnode *Next;

        if (!scan)
            return;
        Next = regnext(scan);
        regprop(RExC_rx, RExC_mysv, scan, NULL, pRExC_state);
        Perl_re_indentf( aTHX_   "%s>%3d: %s (%d)",
            depth,
            str,
            REG_NODE_NUM(scan), SvPV_nolen_const(RExC_mysv),
            Next ? (REG_NODE_NUM(Next)) : 0 );
        Perl_debug_show_study_flags(aTHX_ flags," [ ","]");
        Perl_re_printf( aTHX_  "\n");
   });
}

#endif /* DEBUGGING */

/*
 - regdump - dump a regexp onto Perl_debug_log in vaguely comprehensible form
 */
#ifdef DEBUGGING

static void
S_regdump_intflags(pTHX_ const char *lead, const U32 flags)
{
    int bit;
    int set=0;

    ASSUME(REG_INTFLAGS_NAME_SIZE <= sizeof(flags)*8);

    for (bit=0; bit<=REG_INTFLAGS_NAME_SIZE; bit++) {
        if (flags & (1<<bit)) {
            if (!set++ && lead)
                Perl_re_printf( aTHX_  "%s", lead);
            Perl_re_printf( aTHX_  "%s ", PL_reg_intflags_name[bit]);
        }
    }
    if (lead)  {
        if (set)
            Perl_re_printf( aTHX_  "\n");
        else
            Perl_re_printf( aTHX_  "%s[none-set]\n", lead);
    }
}

static void
S_regdump_extflags(pTHX_ const char *lead, const U32 flags)
{
    int bit;
    int set=0;
    regex_charset cs;

    ASSUME(REG_EXTFLAGS_NAME_SIZE <= sizeof(flags)*8);

    for (bit=0; bit<REG_EXTFLAGS_NAME_SIZE; bit++) {
        if (flags & (1U<<bit)) {
            if ((1U<<bit) & RXf_PMf_CHARSET) {  /* Output separately, below */
                continue;
            }
            if (!set++ && lead)
                Perl_re_printf( aTHX_  "%s", lead);
            Perl_re_printf( aTHX_  "%s ", PL_reg_extflags_name[bit]);
        }
    }
    if ((cs = get_regex_charset(flags)) != REGEX_DEPENDS_CHARSET) {
            if (!set++ && lead) {
                Perl_re_printf( aTHX_  "%s", lead);
            }
            switch (cs) {
                case REGEX_UNICODE_CHARSET:
                    Perl_re_printf( aTHX_  "UNICODE");
                    break;
                case REGEX_LOCALE_CHARSET:
                    Perl_re_printf( aTHX_  "LOCALE");
                    break;
                case REGEX_ASCII_RESTRICTED_CHARSET:
                    Perl_re_printf( aTHX_  "ASCII-RESTRICTED");
                    break;
                case REGEX_ASCII_MORE_RESTRICTED_CHARSET:
                    Perl_re_printf( aTHX_  "ASCII-MORE_RESTRICTED");
                    break;
                default:
                    Perl_re_printf( aTHX_  "UNKNOWN CHARACTER SET");
                    break;
            }
    }
    if (lead)  {
        if (set)
            Perl_re_printf( aTHX_  "\n");
        else
            Perl_re_printf( aTHX_  "%s[none-set]\n", lead);
    }
}
#endif

void
Perl_regdump(pTHX_ const regexp *r)
{
#ifdef DEBUGGING
    int i;
    SV * const sv = sv_newmortal();
    SV *dsv= sv_newmortal();
    RXi_GET_DECL(r, ri);
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_REGDUMP;

    (void)dumpuntil(r, ri->program, ri->program + 1, NULL, NULL, sv, 0, 0);

    /* Header fields of interest. */
    for (i = 0; i < 2; i++) {
        if (r->substrs->data[i].substr) {
            RE_PV_QUOTED_DECL(s, 0, dsv,
                            SvPVX_const(r->substrs->data[i].substr),
                            RE_SV_DUMPLEN(r->substrs->data[i].substr),
                            PL_dump_re_max_len);
            Perl_re_printf( aTHX_
                          "%s %s%s at %" IVdf "..%" UVuf " ",
                          i ? "floating" : "anchored",
                          s,
                          RE_SV_TAIL(r->substrs->data[i].substr),
                          (IV)r->substrs->data[i].min_offset,
                          (UV)r->substrs->data[i].max_offset);
        }
        else if (r->substrs->data[i].utf8_substr) {
            RE_PV_QUOTED_DECL(s, 1, dsv,
                            SvPVX_const(r->substrs->data[i].utf8_substr),
                            RE_SV_DUMPLEN(r->substrs->data[i].utf8_substr),
                            30);
            Perl_re_printf( aTHX_
                          "%s utf8 %s%s at %" IVdf "..%" UVuf " ",
                          i ? "floating" : "anchored",
                          s,
                          RE_SV_TAIL(r->substrs->data[i].utf8_substr),
                          (IV)r->substrs->data[i].min_offset,
                          (UV)r->substrs->data[i].max_offset);
        }
    }

    if (r->check_substr || r->check_utf8)
        Perl_re_printf( aTHX_
                      (const char *)
                      (   r->check_substr == r->substrs->data[1].substr
                       && r->check_utf8   == r->substrs->data[1].utf8_substr
                       ? "(checking floating" : "(checking anchored"));
    if (r->intflags & PREGf_NOSCAN)
        Perl_re_printf( aTHX_  " noscan");
    if (r->extflags & RXf_CHECK_ALL)
        Perl_re_printf( aTHX_  " isall");
    if (r->check_substr || r->check_utf8)
        Perl_re_printf( aTHX_  ") ");

    if (ri->regstclass) {
        regprop(r, sv, ri->regstclass, NULL, NULL);
        Perl_re_printf( aTHX_  "stclass %s ", SvPVX_const(sv));
    }
    if (r->intflags & PREGf_ANCH) {
        Perl_re_printf( aTHX_  "anchored");
        if (r->intflags & PREGf_ANCH_MBOL)
            Perl_re_printf( aTHX_  "(MBOL)");
        if (r->intflags & PREGf_ANCH_SBOL)
            Perl_re_printf( aTHX_  "(SBOL)");
        if (r->intflags & PREGf_ANCH_GPOS)
            Perl_re_printf( aTHX_  "(GPOS)");
        Perl_re_printf( aTHX_ " ");
    }
    if (r->intflags & PREGf_GPOS_SEEN)
        Perl_re_printf( aTHX_  "GPOS:%" UVuf " ", (UV)r->gofs);
    if (r->intflags & PREGf_SKIP)
        Perl_re_printf( aTHX_  "plus ");
    if (r->intflags & PREGf_IMPLICIT)
        Perl_re_printf( aTHX_  "implicit ");
    Perl_re_printf( aTHX_  "minlen %" IVdf " ", (IV)r->minlen);
    if (r->extflags & RXf_EVAL_SEEN)
        Perl_re_printf( aTHX_  "with eval ");
    Perl_re_printf( aTHX_  "\n");
    DEBUG_FLAGS_r({
        regdump_extflags("r->extflags: ", r->extflags);
        regdump_intflags("r->intflags: ", r->intflags);
    });
#else
    PERL_ARGS_ASSERT_REGDUMP;
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(r);
#endif  /* DEBUGGING */
}

/* Should be synchronized with ANYOF_ #defines in regcomp.h */
#ifdef DEBUGGING

#  if   CC_WORDCHAR_ != 0 || CC_DIGIT_ != 1        || CC_ALPHA_ != 2    \
     || CC_LOWER_ != 3    || CC_UPPER_ != 4        || CC_PUNCT_ != 5    \
     || CC_PRINT_ != 6    || CC_ALPHANUMERIC_ != 7 || CC_GRAPH_ != 8    \
     || CC_CASED_ != 9    || CC_SPACE_ != 10       || CC_BLANK_ != 11   \
     || CC_XDIGIT_ != 12  || CC_CNTRL_ != 13       || CC_ASCII_ != 14   \
     || CC_VERTSPACE_ != 15
#   error Need to adjust order of anyofs[]
#  endif
static const char * const anyofs[] = {
    "\\w",
    "\\W",
    "\\d",
    "\\D",
    "[:alpha:]",
    "[:^alpha:]",
    "[:lower:]",
    "[:^lower:]",
    "[:upper:]",
    "[:^upper:]",
    "[:punct:]",
    "[:^punct:]",
    "[:print:]",
    "[:^print:]",
    "[:alnum:]",
    "[:^alnum:]",
    "[:graph:]",
    "[:^graph:]",
    "[:cased:]",
    "[:^cased:]",
    "\\s",
    "\\S",
    "[:blank:]",
    "[:^blank:]",
    "[:xdigit:]",
    "[:^xdigit:]",
    "[:cntrl:]",
    "[:^cntrl:]",
    "[:ascii:]",
    "[:^ascii:]",
    "\\v",
    "\\V"
};
#endif

/*
- regprop - printable representation of opcode, with run time support
*/

void
Perl_regprop(pTHX_ const regexp *prog, SV *sv, const regnode *o, const regmatch_info *reginfo, const RExC_state_t *pRExC_state)
{
#ifdef DEBUGGING
    U8 k;
    const U8 op = OP(o);
    RXi_GET_DECL(prog, progi);
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_REGPROP;

    SvPVCLEAR(sv);

    if (op > REGNODE_MAX) {          /* regnode.type is unsigned */
        if (pRExC_state) {  /* This gives more info, if we have it */
            FAIL3("panic: corrupted regexp opcode %d > %d",
                  (int)op, (int)REGNODE_MAX);
        }
        else {
            Perl_croak(aTHX_ "panic: corrupted regexp opcode %d > %d",
                             (int)op, (int)REGNODE_MAX);
        }
    }
    sv_catpv(sv, REGNODE_NAME(op)); /* Take off const! */

    k = REGNODE_TYPE(op);
    if (op == BRANCH) {
        Perl_sv_catpvf(aTHX_ sv, " (buf:%" IVdf "/%" IVdf ")", (IV)ARG1a(o),(IV)ARG1b(o));
    }
    else if (op == BRANCHJ) {
        Perl_sv_catpvf(aTHX_ sv, " (buf:%" IVdf "/%" IVdf ")", (IV)ARG2a(o),(IV)ARG2b(o));
    }
    else if (k == EXACT) {
        sv_catpvs(sv, " ");
        /* Using is_utf8_string() (via PERL_PV_UNI_DETECT)
         * is a crude hack but it may be the best for now since
         * we have no flag "this EXACTish node was UTF-8"
         * --jhi */
        pv_pretty(sv, STRING(o), STR_LEN(o), PL_dump_re_max_len,
                  PL_colors[0], PL_colors[1],
                  PERL_PV_ESCAPE_UNI_DETECT |
                  PERL_PV_ESCAPE_NONASCII   |
                  PERL_PV_PRETTY_ELLIPSES   |
                  PERL_PV_PRETTY_LTGT       |
                  PERL_PV_PRETTY_NOCLEAR
                  );
    } else if (k == TRIE) {
        /* print the details of the trie in dumpuntil instead, as
         * progi->data isn't available here */
        const U32 n = ARG1u(o);
        const reg_ac_data * const ac = IS_TRIE_AC(op) ?
               (reg_ac_data *)progi->data->data[n] :
               NULL;
        const reg_trie_data * const trie
            = (reg_trie_data*)progi->data->data[!IS_TRIE_AC(op) ? n : ac->trie];

        Perl_sv_catpvf(aTHX_ sv, "-%s", REGNODE_NAME(FLAGS(o)));
        DEBUG_TRIE_COMPILE_r({
          if (trie->jump)
            sv_catpvs(sv, "(JUMP)");
          Perl_sv_catpvf(aTHX_ sv,
            "<S:%" UVuf "/%" IVdf " W:%" UVuf " L:%" UVuf "/%" UVuf " C:%" UVuf "/%" UVuf ">",
            (UV)trie->startstate,
            (IV)trie->statecount-1, /* -1 because of the unused 0 element */
            (UV)trie->wordcount,
            (UV)trie->minlen,
            (UV)trie->maxlen,
            (UV)TRIE_CHARCOUNT(trie),
            (UV)trie->uniquecharcount
          );
        });
        if ( IS_ANYOF_TRIE(op) || trie->bitmap ) {
            sv_catpvs(sv, "[");
            (void) put_charclass_bitmap_innards(sv,
                                                ((IS_ANYOF_TRIE(op))
                                                 ? ANYOF_BITMAP(o)
                                                 : TRIE_BITMAP(trie)),
                                                NULL,
                                                NULL,
                                                NULL,
                                                0,
                                                FALSE
                                               );
            sv_catpvs(sv, "]");
        }
        if (trie->before_paren || trie->after_paren)
            Perl_sv_catpvf(aTHX_ sv, " (buf:%" IVdf "/%" IVdf ")",
                    (IV)trie->before_paren,(IV)trie->after_paren);
    } else if (k == CURLY) {
        U32 lo = ARG1i(o), hi = ARG2i(o);
        if (ARG3u(o)) /* check both ARG3a and ARG3b at the same time */
            Perl_sv_catpvf(aTHX_ sv, "<%d:%d>", ARG3a(o),ARG3b(o)); /* paren before, paren after */
        if (op == CURLYM || op == CURLYN || op == CURLYX)
            Perl_sv_catpvf(aTHX_ sv, "[%d]", FLAGS(o)); /* Parenth number */
        Perl_sv_catpvf(aTHX_ sv, "{%u,", (unsigned) lo);
        if (hi == REG_INFTY)
            sv_catpvs(sv, "INFTY");
        else
            Perl_sv_catpvf(aTHX_ sv, "%u", (unsigned) hi);
        sv_catpvs(sv, "}");
    }
    else if (k == WHILEM && FLAGS(o))                   /* Ordinal/of */
        Perl_sv_catpvf(aTHX_ sv, "[%d/%d]", FLAGS(o) & 0xf, FLAGS(o)>>4);
    else if (k == REF || k == OPEN || k == CLOSE
             || k == GROUPP || op == ACCEPT)
    {
        AV *name_list= NULL;
        U32 parno= (op == ACCEPT)              ? ARG2u(o) :
                   (op == OPEN || op == CLOSE) ? PARNO(o) :
                                                 ARG1u(o);
        if ( RXp_PAREN_NAMES(prog) ) {
            name_list= MUTABLE_AV(progi->data->data[progi->name_list_idx]);
        } else if ( pRExC_state ) {
            name_list= RExC_paren_name_list;
        }
        if ( name_list ) {
            if ( k != REF || (op < REFN)) {
                UV logical_parno = parno;
                if (prog->parno_to_logical)
                    logical_parno = prog->parno_to_logical[parno];

                Perl_sv_catpvf(aTHX_ sv, "%" UVuf, (UV)logical_parno);     /* Parenth number */
                if (parno != logical_parno)
                    Perl_sv_catpvf(aTHX_ sv, "/%" UVuf, (UV)parno);        /* Parenth number */

                SV **name= av_fetch_simple(name_list, parno, 0 );
                if (name)
                    Perl_sv_catpvf(aTHX_ sv, " '%" SVf "'", SVfARG(*name));
            }
            else
            if (parno > 0) {
                /* parno must always be larger than 0 for this block
                 * as it represents a slot into the data array, which
                 * has the 0 slot reserved for a placeholder so any valid
                 * index into it is always true, eg non-zero
                 * see the '%' "what" type and the implementation of
                 * S_reg_add_data()
                 */
                SV *sv_dat= MUTABLE_SV(progi->data->data[ parno ]);
                I32 *nums=(I32*)SvPVX(sv_dat);
                SV **name= av_fetch_simple(name_list, nums[0], 0 );
                I32 n;
                if (name) {
                    for ( n=0; n<SvIVX(sv_dat); n++ ) {
                        Perl_sv_catpvf(aTHX_ sv, "%s%" IVdf,
                                    (n ? "," : ""), (IV)nums[n]);
                    }
                    Perl_sv_catpvf(aTHX_ sv, " '%" SVf "'", SVfARG(*name));
                }
            }
        } else if (parno>0) {
            UV logical_parno = parno;
            if (prog->parno_to_logical)
                logical_parno = prog->parno_to_logical[parno];

            Perl_sv_catpvf(aTHX_ sv, "%" UVuf, (UV)logical_parno);     /* Parenth number */
            if (logical_parno != parno)
                Perl_sv_catpvf(aTHX_ sv, "/%" UVuf, (UV)parno);     /* Parenth number */

        }
        if ( k == REF ) {
            Perl_sv_catpvf(aTHX_ sv, " <%" IVdf ">", (IV)ARG2i(o));
        }
        if ( k == REF && reginfo) {
            U32 n = ARG1u(o);  /* which paren pair */
            I32 ln = RXp_OFFS_START(prog,n);
            if (RXp_LASTPAREN(prog) < n || ln == -1 || RXp_OFFS_END(prog,n) == -1)
                Perl_sv_catpvf(aTHX_ sv, ": FAIL");
            else if (ln == RXp_OFFS_END(prog,n))
                Perl_sv_catpvf(aTHX_ sv, ": ACCEPT - EMPTY STRING");
            else {
                const char *s = reginfo->strbeg + ln;
                Perl_sv_catpvf(aTHX_ sv, ": ");
                Perl_pv_pretty( aTHX_ sv, s, RXp_OFFS_END(prog,n) - RXp_OFFS_START(prog,n), 32, 0, 0,
                    PERL_PV_ESCAPE_UNI_DETECT|PERL_PV_PRETTY_NOCLEAR|PERL_PV_PRETTY_ELLIPSES|PERL_PV_PRETTY_QUOTE );
            }
        }
    } else if (k == GOSUB) {
        AV *name_list= NULL;
        IV parno = ARG1u(o);
        IV logical_parno = (parno && prog->parno_to_logical)
                         ? prog->parno_to_logical[parno]
                         : parno;
        if ( RXp_PAREN_NAMES(prog) ) {
            name_list= MUTABLE_AV(progi->data->data[progi->name_list_idx]);
        } else if ( pRExC_state ) {
            name_list= RExC_paren_name_list;
        }

        /* Paren and offset */
        Perl_sv_catpvf(aTHX_ sv, "%" IVdf, logical_parno);
        if (logical_parno != parno)
            Perl_sv_catpvf(aTHX_ sv, "/%" IVdf, parno);

        Perl_sv_catpvf(aTHX_ sv, "[%+d:%d]", (int)ARG2i(o),
                (int)((o + (int)ARG2i(o)) - progi->program) );
        if (name_list) {
            SV **name= av_fetch_simple(name_list, ARG1u(o), 0 );
            if (name)
                Perl_sv_catpvf(aTHX_ sv, " '%" SVf "'", SVfARG(*name));
        }
    }
    else if (k == LOGICAL)
        /* 2: embedded, otherwise 1 */
        Perl_sv_catpvf(aTHX_ sv, "[%d]", FLAGS(o));
    else if (k == ANYOF || k == ANYOFH || k == ANYOFR) {
        U8 flags;
        char * bitmap;
        U8 do_sep = 0;    /* Do we need to separate various components of the
                             output? */
        /* Set if there is still an unresolved user-defined property */
        SV *unresolved                = NULL;

        /* Things that are ignored except when the runtime locale is UTF-8 */
        SV *only_utf8_locale_invlist = NULL;

        /* Code points that don't fit in the bitmap */
        SV *nonbitmap_invlist = NULL;

        /* And things that aren't in the bitmap, but are small enough to be */
        SV* bitmap_range_not_in_bitmap = NULL;

        bool inverted;

        if (k != ANYOF) {
            flags = 0;
            bitmap = NULL;
        }
        else {
            flags = ANYOF_FLAGS(o);
            bitmap = ANYOF_BITMAP(o);
        }

        if (op == ANYOFL || op == ANYOFPOSIXL) {
            if ((flags & ANYOFL_UTF8_LOCALE_REQD)) {
                sv_catpvs(sv, "{utf8-locale-reqd}");
            }
            if (flags & ANYOFL_FOLD) {
                sv_catpvs(sv, "{i}");
            }
        }

        inverted = flags & ANYOF_INVERT;

        /* If there is stuff outside the bitmap, get it */
        if (k == ANYOFR) {

            /* For a single range, split into the parts inside vs outside the
             * bitmap. */
            UV start = ANYOFRbase(o);
            UV end   = ANYOFRbase(o) + ANYOFRdelta(o);

            if (start < NUM_ANYOF_CODE_POINTS) {
                if (end < NUM_ANYOF_CODE_POINTS) {
                    bitmap_range_not_in_bitmap
                          = _add_range_to_invlist(bitmap_range_not_in_bitmap,
                                                  start, end);
                }
                else {
                    bitmap_range_not_in_bitmap
                          = _add_range_to_invlist(bitmap_range_not_in_bitmap,
                                                  start, NUM_ANYOF_CODE_POINTS);
                    start = NUM_ANYOF_CODE_POINTS;
                }
            }

            if (start >= NUM_ANYOF_CODE_POINTS) {
                nonbitmap_invlist = _add_range_to_invlist(nonbitmap_invlist,
                                                ANYOFRbase(o),
                                                ANYOFRbase(o) + ANYOFRdelta(o));
            }
        }
        else if (ANYOF_MATCHES_ALL_OUTSIDE_BITMAP(o)) {
            nonbitmap_invlist = _add_range_to_invlist(nonbitmap_invlist,
                                                      NUM_ANYOF_CODE_POINTS,
                                                      UV_MAX);
        }
        else if (ANYOF_HAS_AUX(o)) {
                (void) GET_REGCLASS_AUX_DATA(prog, o, FALSE,
                                                &unresolved,
                                                &only_utf8_locale_invlist,
                                                &nonbitmap_invlist);

            /* The aux data may contain stuff that could fit in the bitmap.
             * This could come from a user-defined property being finally
             * resolved when this call was done; or much more likely because
             * there are matches that require UTF-8 to be valid, and so aren't
             * in the bitmap (or ANYOFR).  This is teased apart later */
            _invlist_intersection(nonbitmap_invlist,
                                  PL_InBitmap,
                                  &bitmap_range_not_in_bitmap);
            /* Leave just the things that don't fit into the bitmap */
            _invlist_subtract(nonbitmap_invlist,
                              PL_InBitmap,
                              &nonbitmap_invlist);
        }

        /* Ready to start outputting.  First, the initial left bracket */
        Perl_sv_catpvf(aTHX_ sv, "[%s", PL_colors[0]);

        if (   bitmap
            || bitmap_range_not_in_bitmap
            || only_utf8_locale_invlist
            || unresolved)
        {
            /* Then all the things that could fit in the bitmap */
            do_sep = put_charclass_bitmap_innards(
                                    sv,
                                    bitmap,
                                    bitmap_range_not_in_bitmap,
                                    only_utf8_locale_invlist,
                                    o,
                                    flags,

                                    /* Can't try inverting for a
                                                   * better display if there
                                                   * are things that haven't
                                                   * been resolved */
                                    (unresolved != NULL || k == ANYOFR));
            SvREFCNT_dec(bitmap_range_not_in_bitmap);

            /* If there are user-defined properties which haven't been defined
             * yet, output them.  If the result is not to be inverted, it is
             * clearest to output them in a separate [] from the bitmap range
             * stuff.  If the result is to be complemented, we have to show
             * everything in one [], as the inversion applies to the whole
             * thing.  Use {braces} to separate them from anything in the
             * bitmap and anything above the bitmap. */
            if (unresolved) {
                if (inverted) {
                    if (! do_sep) { /* If didn't output anything in the bitmap
                                     */
                        sv_catpvs(sv, "^");
                    }
                    sv_catpvs(sv, "{");
                }
                else if (do_sep) {
                    Perl_sv_catpvf(aTHX_ sv,"%s][%s", PL_colors[1],
                                                      PL_colors[0]);
                }
                sv_catsv(sv, unresolved);
                if (inverted) {
                    sv_catpvs(sv, "}");
                }
                do_sep = ! inverted;
            }
            else if (     do_sep == 2
                     && ! nonbitmap_invlist
                     &&   ANYOF_MATCHES_NONE_OUTSIDE_BITMAP(o))
            {
                /* Here, the display shows the class as inverted, and
                 * everything above the lower display should also match, but
                 * there is no indication of that.  Add this range so the code
                 * below will add it to the display */
                _invlist_union_complement_2nd(nonbitmap_invlist,
                                              PL_InBitmap,
                                              &nonbitmap_invlist);
            }
        }

        /* And, finally, add the above-the-bitmap stuff */
        if (nonbitmap_invlist && _invlist_len(nonbitmap_invlist)) {
            SV* contents;

            /* See if truncation size is overridden */
            const STRLEN dump_len = (PL_dump_re_max_len > 256)
                                    ? PL_dump_re_max_len
                                    : 256;

            /* This is output in a separate [] */
            if (do_sep) {
                Perl_sv_catpvf(aTHX_ sv,"%s][%s", PL_colors[1], PL_colors[0]);
            }

            /* And, for easy of understanding, it is shown in the
             * uncomplemented form if possible.  The one exception being if
             * there are unresolved items, where the inversion has to be
             * delayed until runtime */
            if (inverted && ! unresolved) {
                _invlist_invert(nonbitmap_invlist);
                _invlist_subtract(nonbitmap_invlist, PL_InBitmap, &nonbitmap_invlist);
            }

            contents = invlist_contents(nonbitmap_invlist,
                                        FALSE /* output suitable for catsv */
                                       );

            /* If the output is shorter than the permissible maximum, just do it. */
            if (SvCUR(contents) <= dump_len) {
                sv_catsv(sv, contents);
            }
            else {
                const char * contents_string = SvPVX(contents);
                STRLEN i = dump_len;

                /* Otherwise, start at the permissible max and work back to the
                 * first break possibility */
                while (i > 0 && contents_string[i] != ' ') {
                    i--;
                }
                if (i == 0) {       /* Fail-safe.  Use the max if we couldn't
                                       find a legal break */
                    i = dump_len;
                }

                sv_catpvn(sv, contents_string, i);
                sv_catpvs(sv, "...");
            }

            SvREFCNT_dec_NN(contents);
            SvREFCNT_dec_NN(nonbitmap_invlist);
        }

        /* And finally the matching, closing ']' */
        Perl_sv_catpvf(aTHX_ sv, "%s]", PL_colors[1]);

        if (op == ANYOFHs) {
            Perl_sv_catpvf(aTHX_ sv, " (Leading UTF-8 bytes=%s", _byte_dump_string((U8 *) ((struct regnode_anyofhs *) o)->string, FLAGS(o), 1));
        }
        else if (REGNODE_TYPE(op) != ANYOF) {
            U8 lowest = (op != ANYOFHr)
                         ? FLAGS(o)
                         : LOWEST_ANYOF_HRx_BYTE(FLAGS(o));
            U8 highest = (op == ANYOFHr)
                         ? HIGHEST_ANYOF_HRx_BYTE(FLAGS(o))
                         : (op == ANYOFH || op == ANYOFR)
                           ? 0xFF
                           : lowest;
#ifndef EBCDIC
            if (op != ANYOFR || ! isASCII(ANYOFRbase(o) + ANYOFRdelta(o)))
#endif
            {
                Perl_sv_catpvf(aTHX_ sv, " (First UTF-8 byte=%02X", lowest);
                if (lowest != highest) {
                    Perl_sv_catpvf(aTHX_ sv, "-%02X", highest);
                }
                Perl_sv_catpvf(aTHX_ sv, ")");
            }
        }

        SvREFCNT_dec(unresolved);
    }
    else if (k == ANYOFM) {
        SV * cp_list = get_ANYOFM_contents(o);

        Perl_sv_catpvf(aTHX_ sv, "[%s", PL_colors[0]);
        if (op == NANYOFM) {
            _invlist_invert(cp_list);
        }

        put_charclass_bitmap_innards(sv, NULL, cp_list, NULL, NULL, 0, TRUE);
        Perl_sv_catpvf(aTHX_ sv, "%s]", PL_colors[1]);

        SvREFCNT_dec(cp_list);
    }
    else if (k == ANYOFHbbm) {
        SV * cp_list = get_ANYOFHbbm_contents(o);
        Perl_sv_catpvf(aTHX_ sv, "[%s", PL_colors[0]);

        sv_catsv(sv, invlist_contents(cp_list,
                                      FALSE /* output suitable for catsv */
                                     ));
        Perl_sv_catpvf(aTHX_ sv, "%s]", PL_colors[1]);

        SvREFCNT_dec(cp_list);
    }
    else if (k == POSIXD || k == NPOSIXD) {
        U8 index = FLAGS(o) * 2;
        if (index < C_ARRAY_LENGTH(anyofs)) {
            if (*anyofs[index] != '[')  {
                sv_catpvs(sv, "[");
            }
            sv_catpv(sv, anyofs[index]);
            if (*anyofs[index] != '[')  {
                sv_catpvs(sv, "]");
            }
        }
        else {
            Perl_sv_catpvf(aTHX_ sv, "[illegal type=%d])", index);
        }
    }
    else if (k == BOUND || k == NBOUND) {
        /* Must be synced with order of 'bound_type' in regcomp.h */
        const char * const bounds[] = {
            "",      /* Traditional */
            "{gcb}",
            "{lb}",
            "{sb}",
            "{wb}"
        };
        assert(FLAGS(o) < C_ARRAY_LENGTH(bounds));
        sv_catpv(sv, bounds[FLAGS(o)]);
    }
    else if (k == BRANCHJ && (op == UNLESSM || op == IFMATCH)) {
        Perl_sv_catpvf(aTHX_ sv, "[%d", -(FLAGS(o)));
        if (NEXT_OFF(o)) {
            Perl_sv_catpvf(aTHX_ sv, "..-%d", FLAGS(o) - NEXT_OFF(o));
        }
        Perl_sv_catpvf(aTHX_ sv, "]");
    }
    else if (op == SBOL)
        Perl_sv_catpvf(aTHX_ sv, " /%s/", FLAGS(o) ? "\\A" : "^");
    else if (op == EVAL) {
        if (FLAGS(o) & EVAL_OPTIMISTIC_FLAG)
            Perl_sv_catpvf(aTHX_ sv, " optimistic");
    }

    /* add on the verb argument if there is one */
    if ( ( k == VERB || op == ACCEPT || op == OPFAIL ) && FLAGS(o)) {
        if ( ARG1u(o) )
            Perl_sv_catpvf(aTHX_ sv, ":%" SVf,
                       SVfARG((MUTABLE_SV(progi->data->data[ ARG1u( o ) ]))));
        else
            sv_catpvs(sv, ":NULL");
    }
#else
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(sv);
    PERL_UNUSED_ARG(o);
    PERL_UNUSED_ARG(prog);
    PERL_UNUSED_ARG(reginfo);
    PERL_UNUSED_ARG(pRExC_state);
#endif  /* DEBUGGING */
}

#ifdef DEBUGGING

STATIC void
S_put_code_point(pTHX_ SV *sv, UV c)
{
    PERL_ARGS_ASSERT_PUT_CODE_POINT;

    if (c > 255) {
        Perl_sv_catpvf(aTHX_ sv, "\\x{%04" UVXf "}", c);
    }
    else if (isPRINT(c)) {
        const char string = (char) c;

        /* We use {phrase} as metanotation in the class, so also escape literal
         * braces */
        if (isBACKSLASHED_PUNCT(c) || c == '{' || c == '}')
            sv_catpvs(sv, "\\");
        sv_catpvn(sv, &string, 1);
    }
    else if (isMNEMONIC_CNTRL(c)) {
        Perl_sv_catpvf(aTHX_ sv, "%s", cntrl_to_mnemonic((U8) c));
    }
    else {
        Perl_sv_catpvf(aTHX_ sv, "\\x%02X", (U8) c);
    }
}

STATIC void
S_put_range(pTHX_ SV *sv, UV start, const UV end, const bool allow_literals)
{
    /* Appends to 'sv' a displayable version of the range of code points from
     * 'start' to 'end'.  Mnemonics (like '\r') are used for the few controls
     * that have them, when they occur at the beginning or end of the range.
     * It uses hex to output the remaining code points, unless 'allow_literals'
     * is true, in which case the printable ASCII ones are output as-is (though
     * some of these will be escaped by put_code_point()).
     *
     * NOTE:  This is designed only for printing ranges of code points that fit
     *        inside an ANYOF bitmap.  Higher code points are simply suppressed
     */

    const unsigned int min_range_count = 3;

    assert(start <= end);

    PERL_ARGS_ASSERT_PUT_RANGE;

    while (start <= end) {
        UV this_end;
        const char * format;

        if (    end - start < min_range_count
            && (end - start <= 2 || (isPRINT_A(start) && isPRINT_A(end))))
        {
            /* Output a range of 1 or 2 chars individually, or longer ranges
             * when printable */
            for (; start <= end; start++) {
                put_code_point(sv, start);
            }
            break;
        }

        /* If permitted by the input options, and there is a possibility that
         * this range contains a printable literal, look to see if there is
         * one. */
        if (allow_literals && start <= MAX_PRINT_A) {

            /* If the character at the beginning of the range isn't an ASCII
             * printable, effectively split the range into two parts:
             *  1) the portion before the first such printable,
             *  2) the rest
             * and output them separately. */
            if (! isPRINT_A(start)) {
                UV temp_end = start + 1;

                /* There is no point looking beyond the final possible
                 * printable, in MAX_PRINT_A */
                UV max = MIN(end, MAX_PRINT_A);

                while (temp_end <= max && ! isPRINT_A(temp_end)) {
                    temp_end++;
                }

                /* Here, temp_end points to one beyond the first printable if
                 * found, or to one beyond 'max' if not.  If none found, make
                 * sure that we use the entire range */
                if (temp_end > MAX_PRINT_A) {
                    temp_end = end + 1;
                }

                /* Output the first part of the split range: the part that
                 * doesn't have printables, with the parameter set to not look
                 * for literals (otherwise we would infinitely recurse) */
                put_range(sv, start, temp_end - 1, FALSE);

                /* The 2nd part of the range (if any) starts here. */
                start = temp_end;

                /* We do a continue, instead of dropping down, because even if
                 * the 2nd part is non-empty, it could be so short that we want
                 * to output it as individual characters, as tested for at the
                 * top of this loop.  */
                continue;
            }

            /* Here, 'start' is a printable ASCII.  If it is an alphanumeric,
             * output a sub-range of just the digits or letters, then process
             * the remaining portion as usual. */
            if (isALPHANUMERIC_A(start)) {
                UV mask = (isDIGIT_A(start))
                           ? CC_DIGIT_
                             : isUPPER_A(start)
                               ? CC_UPPER_
                               : CC_LOWER_;
                UV temp_end = start + 1;

                /* Find the end of the sub-range that includes just the
                 * characters in the same class as the first character in it */
                while (temp_end <= end && generic_isCC_A_(temp_end, mask)) {
                    temp_end++;
                }
                temp_end--;

                /* For short ranges, don't duplicate the code above to output
                 * them; just call recursively */
                if (temp_end - start < min_range_count) {
                    put_range(sv, start, temp_end, FALSE);
                }
                else {  /* Output as a range */
                    put_code_point(sv, start);
                    sv_catpvs(sv, "-");
                    put_code_point(sv, temp_end);
                }
                start = temp_end + 1;
                continue;
            }

            /* We output any other printables as individual characters */
            if (isPUNCT_A(start) || isSPACE_A(start)) {
                while (start <= end && (isPUNCT_A(start)
                                        || isSPACE_A(start)))
                {
                    put_code_point(sv, start);
                    start++;
                }
                continue;
            }
        } /* End of looking for literals */

        /* Here is not to output as a literal.  Some control characters have
         * mnemonic names.  Split off any of those at the beginning and end of
         * the range to print mnemonically.  It isn't possible for many of
         * these to be in a row, so this won't overwhelm with output */
        if (   start <= end
            && (isMNEMONIC_CNTRL(start) || isMNEMONIC_CNTRL(end)))
        {
            while (isMNEMONIC_CNTRL(start) && start <= end) {
                put_code_point(sv, start);
                start++;
            }

            /* If this didn't take care of the whole range ... */
            if (start <= end) {

                /* Look backwards from the end to find the final non-mnemonic
                 * */
                UV temp_end = end;
                while (isMNEMONIC_CNTRL(temp_end)) {
                    temp_end--;
                }

                /* And separately output the interior range that doesn't start
                 * or end with mnemonics */
                put_range(sv, start, temp_end, FALSE);

                /* Then output the mnemonic trailing controls */
                start = temp_end + 1;
                while (start <= end) {
                    put_code_point(sv, start);
                    start++;
                }
                break;
            }
        }

        /* As a final resort, output the range or subrange as hex. */

        if (start >= NUM_ANYOF_CODE_POINTS) {
            this_end = end;
        }
        else {  /* Have to split range at the bitmap boundary */
            this_end = (end < NUM_ANYOF_CODE_POINTS)
                        ? end
                        : NUM_ANYOF_CODE_POINTS - 1;
        }
#if NUM_ANYOF_CODE_POINTS > 256
        format = (this_end < 256)
                 ? "\\x%02" UVXf "-\\x%02" UVXf
                 : "\\x{%04" UVXf "}-\\x{%04" UVXf "}";
#else
        format = "\\x%02" UVXf "-\\x%02" UVXf;
#endif
        GCC_DIAG_IGNORE_STMT(-Wformat-nonliteral);
        Perl_sv_catpvf(aTHX_ sv, format, start, this_end);
        GCC_DIAG_RESTORE_STMT;
        break;
    }
}

STATIC void
S_put_charclass_bitmap_innards_invlist(pTHX_ SV *sv, SV* invlist)
{
    /* Concatenate onto the PV in 'sv' a displayable form of the inversion list
     * 'invlist' */

    UV start, end;
    bool allow_literals = TRUE;

    PERL_ARGS_ASSERT_PUT_CHARCLASS_BITMAP_INNARDS_INVLIST;

    /* Generally, it is more readable if printable characters are output as
     * literals, but if a range (nearly) spans all of them, it's best to output
     * it as a single range.  This code will use a single range if all but 2
     * ASCII printables are in it */
    invlist_iterinit(invlist);
    while (invlist_iternext(invlist, &start, &end)) {

        /* If the range starts beyond the final printable, it doesn't have any
         * in it */
        if (start > MAX_PRINT_A) {
            break;
        }

        /* In both ASCII and EBCDIC, a SPACE is the lowest printable.  To span
         * all but two, the range must start and end no later than 2 from
         * either end */
        if (start < ' ' + 2 && end > MAX_PRINT_A - 2) {
            if (end > MAX_PRINT_A) {
                end = MAX_PRINT_A;
            }
            if (start < ' ') {
                start = ' ';
            }
            if (end - start >= MAX_PRINT_A - ' ' - 2) {
                allow_literals = FALSE;
            }
            break;
        }
    }
    invlist_iterfinish(invlist);

    /* Here we have figured things out.  Output each range */
    invlist_iterinit(invlist);
    while (invlist_iternext(invlist, &start, &end)) {
        if (start >= NUM_ANYOF_CODE_POINTS) {
            break;
        }
        put_range(sv, start, end, allow_literals);
    }
    invlist_iterfinish(invlist);

    return;
}

STATIC SV*
S_put_charclass_bitmap_innards_common(pTHX_
        SV* invlist,            /* The bitmap */
        SV* posixes,            /* Under /l, things like [:word:], \S */
        SV* only_utf8,          /* Under /d, matches iff the target is UTF-8 */
        SV* not_utf8,           /* /d, matches iff the target isn't UTF-8 */
        SV* only_utf8_locale,   /* Under /l, matches if the locale is UTF-8 */
        const bool invert       /* Is the result to be inverted? */
)
{
    /* Create and return an SV containing a displayable version of the bitmap
     * and associated information determined by the input parameters.  If the
     * output would have been only the inversion indicator '^', NULL is instead
     * returned. */

    SV * output;

    PERL_ARGS_ASSERT_PUT_CHARCLASS_BITMAP_INNARDS_COMMON;

    if (invert) {
        output = newSVpvs("^");
    }
    else {
        output = newSVpvs("");
    }

    /* First, the code points in the bitmap that are unconditionally there */
    put_charclass_bitmap_innards_invlist(output, invlist);

    /* Traditionally, these have been placed after the main code points */
    if (posixes) {
        sv_catsv(output, posixes);
    }

    if (only_utf8 && _invlist_len(only_utf8)) {
        Perl_sv_catpvf(aTHX_ output, "%s{utf8}%s", PL_colors[1], PL_colors[0]);
        put_charclass_bitmap_innards_invlist(output, only_utf8);
    }

    if (not_utf8 && _invlist_len(not_utf8)) {
        Perl_sv_catpvf(aTHX_ output, "%s{not utf8}%s", PL_colors[1], PL_colors[0]);
        put_charclass_bitmap_innards_invlist(output, not_utf8);
    }

    if (only_utf8_locale && _invlist_len(only_utf8_locale)) {
        Perl_sv_catpvf(aTHX_ output, "%s{utf8 locale}%s", PL_colors[1], PL_colors[0]);
        put_charclass_bitmap_innards_invlist(output, only_utf8_locale);

        /* This is the only list in this routine that can legally contain code
         * points outside the bitmap range.  The call just above to
         * 'put_charclass_bitmap_innards_invlist' will simply suppress them, so
         * output them here.  There's about a half-dozen possible, and none in
         * contiguous ranges longer than 2 */
        if (invlist_highest(only_utf8_locale) >= NUM_ANYOF_CODE_POINTS) {
            UV start, end;
            SV* above_bitmap = NULL;

            _invlist_subtract(only_utf8_locale, PL_InBitmap, &above_bitmap);

            invlist_iterinit(above_bitmap);
            while (invlist_iternext(above_bitmap, &start, &end)) {
                UV i;

                for (i = start; i <= end; i++) {
                    put_code_point(output, i);
                }
            }
            invlist_iterfinish(above_bitmap);
            SvREFCNT_dec_NN(above_bitmap);
        }
    }

    if (invert && SvCUR(output) == 1) {
        return NULL;
    }

    return output;
}

STATIC U8
S_put_charclass_bitmap_innards(pTHX_ SV *sv,
                                     char *bitmap,
                                     SV *nonbitmap_invlist,
                                     SV *only_utf8_locale_invlist,
                                     const regnode * const node,
                                     const U8 flags,
                                     const bool force_as_is_display)
{
    /* Appends to 'sv' a displayable version of the innards of the bracketed
     * character class defined by the other arguments:
     *  'bitmap' points to the bitmap, or NULL if to ignore that.
     *  'nonbitmap_invlist' is an inversion list of the code points that are in
     *      the bitmap range, but for some reason aren't in the bitmap; NULL if
     *      none.  The reasons for this could be that they require some
     *      condition such as the target string being or not being in UTF-8
     *      (under /d), or because they came from a user-defined property that
     *      was not resolved at the time of the regex compilation (under /u)
     *  'only_utf8_locale_invlist' is an inversion list of the code points that
     *      are valid only if the runtime locale is a UTF-8 one; NULL if none
     *  'node' is the regex pattern ANYOF node.  It is needed only when the
     *      above two parameters are not null, and is passed so that this
     *      routine can tease apart the various reasons for them.
     *  'flags' is the flags field of 'node'
     *  'force_as_is_display' is TRUE if this routine should definitely NOT try
     *      to invert things to see if that leads to a cleaner display.  If
     *      FALSE, this routine is free to use its judgment about doing this.
     *
     * It returns 0 if nothing was actually output.  (It may be that
     *              the bitmap, etc is empty.)
     *            1 if the output wasn't inverted (didn't begin with a '^')
     *            2 if the output was inverted (did begin with a '^')
     *
     * When called for outputting the bitmap of a non-ANYOF node, just pass the
     * bitmap, with the succeeding parameters set to NULL, and the final one to
     * FALSE.
     */

    /* In general, it tries to display the 'cleanest' representation of the
     * innards, choosing whether to display them inverted or not, regardless of
     * whether the class itself is to be inverted.  However,  there are some
     * cases where it can't try inverting, as what actually matches isn't known
     * until runtime, and hence the inversion isn't either. */

    bool inverting_allowed = ! force_as_is_display;

    int i;
    STRLEN orig_sv_cur = SvCUR(sv);

    SV* invlist;            /* Inversion list we accumulate of code points that
                               are unconditionally matched */
    SV* only_utf8 = NULL;   /* Under /d, list of matches iff the target is
                               UTF-8 */
    SV* not_utf8 =  NULL;   /* /d, list of matches iff the target isn't UTF-8
                             */
    SV* posixes = NULL;     /* Under /l, string of things like [:word:], \D */
    SV* only_utf8_locale = NULL;    /* Under /l, list of matches if the locale
                                       is UTF-8 */

    SV* as_is_display;      /* The output string when we take the inputs
                               literally */
    SV* inverted_display;   /* The output string when we invert the inputs */

    bool invert = cBOOL(flags & ANYOF_INVERT);  /* Is the input to be inverted
                                                   to match? */
    /* We are biased in favor of displaying things without them being inverted,
     * as that is generally easier to understand */
    const int bias = 5;

    PERL_ARGS_ASSERT_PUT_CHARCLASS_BITMAP_INNARDS;

    /* Start off with whatever code points are passed in.  (We clone, so we
     * don't change the caller's list) */
    if (nonbitmap_invlist) {
        assert(invlist_highest(nonbitmap_invlist) < NUM_ANYOF_CODE_POINTS);
        invlist = invlist_clone(nonbitmap_invlist, NULL);
    }
    else {  /* Worst case size is every other code point is matched */
        invlist = _new_invlist(NUM_ANYOF_CODE_POINTS / 2);
    }

    if (flags) {
        if (OP(node) == ANYOFD) {

            /* This flag indicates that the code points below 0x100 in the
             * nonbitmap list are precisely the ones that match only when the
             * target is UTF-8 (they should all be non-ASCII). */
            if (flags & ANYOF_HAS_EXTRA_RUNTIME_MATCHES) {
                _invlist_intersection(invlist, PL_UpperLatin1, &only_utf8);
                _invlist_subtract(invlist, only_utf8, &invlist);
            }

            /* And this flag for matching all non-ASCII 0xFF and below */
            if (flags & ANYOFD_NON_UTF8_MATCHES_ALL_NON_ASCII__shared) {
                not_utf8 = invlist_clone(PL_UpperLatin1, NULL);
            }
        }
        else if (OP(node) == ANYOFL || OP(node) == ANYOFPOSIXL) {

            /* If either of these flags are set, what matches isn't
             * determinable except during execution, so don't know enough here
             * to invert */
            if (flags & (ANYOFL_FOLD|ANYOF_MATCHES_POSIXL)) {
                inverting_allowed = FALSE;
            }

            /* What the posix classes match also varies at runtime, so these
             * will be output symbolically. */
            if (ANYOF_POSIXL_TEST_ANY_SET(node)) {
                int i;

                posixes = newSVpvs("");
                for (i = 0; i < ANYOF_POSIXL_MAX; i++) {
                    if (ANYOF_POSIXL_TEST(node, i)) {
                        sv_catpv(posixes, anyofs[i]);
                    }
                }
            }
        }
    }

    /* Accumulate the bit map into the unconditional match list */
    if (bitmap) {
        for (i = 0; i < NUM_ANYOF_CODE_POINTS; i++) {
            if (BITMAP_TEST(bitmap, i)) {
                int start = i++;
                for (;
                     i < NUM_ANYOF_CODE_POINTS && BITMAP_TEST(bitmap, i);
                     i++)
                { /* empty */ }
                invlist = _add_range_to_invlist(invlist, start, i-1);
            }
        }
    }

    /* Make sure that the conditional match lists don't have anything in them
     * that match unconditionally; otherwise the output is quite confusing.
     * This could happen if the code that populates these misses some
     * duplication. */
    if (only_utf8) {
        _invlist_subtract(only_utf8, invlist, &only_utf8);
    }
    if (not_utf8) {
        _invlist_subtract(not_utf8, invlist, &not_utf8);
    }

    if (only_utf8_locale_invlist) {

        /* Since this list is passed in, we have to make a copy before
         * modifying it */
        only_utf8_locale = invlist_clone(only_utf8_locale_invlist, NULL);

        _invlist_subtract(only_utf8_locale, invlist, &only_utf8_locale);

        /* And, it can get really weird for us to try outputting an inverted
         * form of this list when it has things above the bitmap, so don't even
         * try */
        if (invlist_highest(only_utf8_locale) >= NUM_ANYOF_CODE_POINTS) {
            inverting_allowed = FALSE;
        }
    }

    /* Calculate what the output would be if we take the input as-is */
    as_is_display = put_charclass_bitmap_innards_common(invlist,
                                                    posixes,
                                                    only_utf8,
                                                    not_utf8,
                                                    only_utf8_locale,
                                                    invert);

    /* If have to take the output as-is, just do that */
    if (! inverting_allowed) {
        if (as_is_display) {
            sv_catsv(sv, as_is_display);
            SvREFCNT_dec_NN(as_is_display);
        }
    }
    else { /* But otherwise, create the output again on the inverted input, and
              use whichever version is shorter */

        int inverted_bias, as_is_bias;

        /* We will apply our bias to whichever of the results doesn't have
         * the '^' */
        bool trial_invert;
        if (invert) {
            trial_invert = FALSE;
            as_is_bias = bias;
            inverted_bias = 0;
        }
        else {
            trial_invert = TRUE;
            as_is_bias = 0;
            inverted_bias = bias;
        }

        /* Now invert each of the lists that contribute to the output,
         * excluding from the result things outside the possible range */

        /* For the unconditional inversion list, we have to add in all the
         * conditional code points, so that when inverted, they will be gone
         * from it */
        _invlist_union(only_utf8, invlist, &invlist);
        _invlist_union(not_utf8, invlist, &invlist);
        _invlist_union(only_utf8_locale, invlist, &invlist);
        _invlist_invert(invlist);
        _invlist_intersection(invlist, PL_InBitmap, &invlist);

        if (only_utf8) {
            _invlist_invert(only_utf8);
            _invlist_intersection(only_utf8, PL_UpperLatin1, &only_utf8);
        }
        else if (not_utf8) {

            /* If a code point matches iff the target string is not in UTF-8,
             * then complementing the result has it not match iff not in UTF-8,
             * which is the same thing as matching iff it is UTF-8. */
            only_utf8 = not_utf8;
            not_utf8 = NULL;
        }

        if (only_utf8_locale) {
            _invlist_invert(only_utf8_locale);
            _invlist_intersection(only_utf8_locale,
                                  PL_InBitmap,
                                  &only_utf8_locale);
        }

        inverted_display = put_charclass_bitmap_innards_common(
                                            invlist,
                                            posixes,
                                            only_utf8,
                                            not_utf8,
                                            only_utf8_locale, trial_invert);

        /* Use the shortest representation, taking into account our bias
         * against showing it inverted */
        if (   inverted_display
            && (   ! as_is_display
                || (  SvCUR(inverted_display) + inverted_bias
                    < SvCUR(as_is_display)    + as_is_bias)))
        {
            sv_catsv(sv, inverted_display);
            invert = ! invert;
        }
        else if (as_is_display) {
            sv_catsv(sv, as_is_display);
        }

        SvREFCNT_dec(as_is_display);
        SvREFCNT_dec(inverted_display);
    }

    SvREFCNT_dec_NN(invlist);
    SvREFCNT_dec(only_utf8);
    SvREFCNT_dec(not_utf8);
    SvREFCNT_dec(posixes);
    SvREFCNT_dec(only_utf8_locale);

    U8 did_output_something = (bool) (SvCUR(sv) > orig_sv_cur);
    if (did_output_something) {
        /* Distinguish between non and inverted cases */
        did_output_something += invert;
    }

    return did_output_something;
}


const regnode *
Perl_dumpuntil(pTHX_ const regexp *r, const regnode *start, const regnode *node,
            const regnode *last, const regnode *plast,
            SV* sv, I32 indent, U32 depth)
{
    const regnode *next;
    const regnode *optstart= NULL;

    RXi_GET_DECL(r, ri);
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_DUMPUNTIL;

#ifdef DEBUG_DUMPUNTIL
    Perl_re_printf( aTHX_  "--- %d : %d - %d - %d\n", indent, node-start,
        last ? last-start : 0, plast ? plast-start : 0);
#endif

    if (plast && plast < last)
        last= plast;

    while (node && (!last || node < last)) {
        const U8 op = OP(node);

        if (op == CLOSE || op == SRCLOSE || op == WHILEM)
            indent--;
        next = regnext((regnode *)node);
        const regnode *after = regnode_after((regnode *)node,0);

        /* Where, what. */
        if (op == OPTIMIZED) {
            if (!optstart && RE_DEBUG_FLAG(RE_DEBUG_COMPILE_OPTIMISE))
                optstart = node;
            else
                goto after_print;
        } else
            CLEAR_OPTSTART;

        regprop(r, sv, node, NULL, NULL);
        Perl_re_printf( aTHX_  "%4" IVdf ":%*s%s", (IV)(node - start),
                      (int)(2*indent + 1), "", SvPVX_const(sv));

        if (op != OPTIMIZED) {
            if (next == NULL)           /* Next ptr. */
                Perl_re_printf( aTHX_  " (0)");
            else if (REGNODE_TYPE(op) == BRANCH
                     && REGNODE_TYPE(OP(next)) != BRANCH )
                Perl_re_printf( aTHX_  " (FAIL)");
            else
                Perl_re_printf( aTHX_  " (%" IVdf ")", (IV)(next - start));
            Perl_re_printf( aTHX_ "\n");
        }

      after_print:
        if (REGNODE_TYPE(op) == BRANCHJ) {
            assert(next);
            const regnode *nnode = (OP(next) == LONGJMP
                                   ? regnext((regnode *)next)
                                   : next);
            if (last && nnode > last)
                nnode = last;
            DUMPUNTIL(after, nnode);
        }
        else if (REGNODE_TYPE(op) == BRANCH) {
            assert(next);
            DUMPUNTIL(after, next);
        }
        else if ( REGNODE_TYPE(op)  == TRIE ) {
            const regnode *this_trie = node;
            const U32 n = ARG1u(node);
            const reg_ac_data * const ac = op>=AHOCORASICK ?
               (reg_ac_data *)ri->data->data[n] :
               NULL;
            const reg_trie_data * const trie =
                (reg_trie_data*)ri->data->data[op<AHOCORASICK ? n : ac->trie];
#ifdef DEBUGGING
            AV *const trie_words
                           = MUTABLE_AV(ri->data->data[n + TRIE_WORDS_OFFSET]);
#endif
            const regnode *nextbranch= NULL;
            I32 word_idx;
            SvPVCLEAR(sv);
            for (word_idx= 0; word_idx < (I32)trie->wordcount; word_idx++) {
                SV ** const elem_ptr = av_fetch_simple(trie_words, word_idx, 0);

                Perl_re_indentf( aTHX_  "%s ",
                    indent+3,
                    elem_ptr
                    ? pv_pretty(sv, SvPV_nolen_const(*elem_ptr),
                                SvCUR(*elem_ptr), PL_dump_re_max_len,
                                PL_colors[0], PL_colors[1],
                                (SvUTF8(*elem_ptr)
                                 ? PERL_PV_ESCAPE_UNI
                                 : 0)
                                | PERL_PV_PRETTY_ELLIPSES
                                | PERL_PV_PRETTY_LTGT
                            )
                    : "???"
                );
                if (trie->jump) {
                    U16 dist= trie->jump[word_idx+1];
                    Perl_re_printf( aTHX_  "(%" UVuf ")\n",
                               (UV)((dist ? this_trie + dist : next) - start));
                    if (dist) {
                        if (!nextbranch)
                            nextbranch= this_trie + trie->jump[0];
                        DUMPUNTIL(this_trie + dist, nextbranch);
                    }
                    if (nextbranch && REGNODE_TYPE(OP(nextbranch))==BRANCH)
                        nextbranch= regnext((regnode *)nextbranch);
                } else {
                    Perl_re_printf( aTHX_  "\n");
                }
            }
            if (last && next > last)
                node= last;
            else
                node= next;
        }
        else if ( op == CURLY ) {   /* "next" might be very big: optimizer */
            DUMPUNTIL(after, after + 1); /* +1 is NOT a REGNODE_AFTER */
        }
        else if (REGNODE_TYPE(op) == CURLY && op != CURLYX) {
            assert(next);
            DUMPUNTIL(after, next);
        }
        else if ( op == PLUS || op == STAR) {
            DUMPUNTIL(after, after + 1); /* +1 NOT a REGNODE_AFTER */
        }
        else if (REGNODE_TYPE(op) == EXACT || op == ANYOFHs) {
            /* Literal string, where present. */
            node = (const regnode *)REGNODE_AFTER_varies(node);
        }
        else {
            node = REGNODE_AFTER_opcode(node,op);
        }
        if (op == CURLYX || op == OPEN || op == SROPEN)
            indent++;
        if (REGNODE_TYPE(op) == END)
            break;
    }
    CLEAR_OPTSTART;
#ifdef DEBUG_DUMPUNTIL
    Perl_re_printf( aTHX_  "--- %d\n", (int)indent);
#endif
    return node;
}

#endif  /* DEBUGGING */
