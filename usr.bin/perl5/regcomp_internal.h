#ifndef REGCOMP_INTERNAL_H
#define REGCOMP_INTERNAL_H
#ifndef STATIC
#define STATIC  static
#endif
#ifndef RE_OPTIMIZE_CURLYX_TO_CURLYM
#define RE_OPTIMIZE_CURLYX_TO_CURLYM 1
#endif
#ifndef RE_OPTIMIZE_CURLYX_TO_CURLYN
#define RE_OPTIMIZE_CURLYX_TO_CURLYN 1
#endif

/* this is a chain of data about sub patterns we are processing that
   need to be handled separately/specially in study_chunk. Its so
   we can simulate recursion without losing state.  */
struct scan_frame;
typedef struct scan_frame {
    regnode *last_regnode;      /* last node to process in this frame */
    regnode *next_regnode;      /* next node to process when last is reached */
    U32 prev_recursed_depth;
    I32 stopparen;              /* what stopparen do we use */
    bool in_gosub;              /* this or an outer frame is for GOSUB */

    struct scan_frame *this_prev_frame; /* this previous frame */
    struct scan_frame *prev_frame;      /* previous frame */
    struct scan_frame *next_frame;      /* next frame */
} scan_frame;

/* Certain characters are output as a sequence with the first being a
 * backslash. */
#define isBACKSLASHED_PUNCT(c)  memCHRs("-[]\\^", c)


struct RExC_state_t {
    U32         flags;                  /* RXf_* are we folding, multilining? */
    U32         pm_flags;               /* PMf_* stuff from the calling PMOP */
    char        *precomp;               /* uncompiled string. */
    char        *precomp_end;           /* pointer to end of uncompiled string. */
    REGEXP      *rx_sv;                 /* The SV that is the regexp. */
    regexp      *rx;                    /* perl core regexp structure */
    regexp_internal     *rxi;           /* internal data for regexp object
                                           pprivate field */
    char        *start;                 /* Start of input for compile */
    char        *end;                   /* End of input for compile */
    char        *parse;                 /* Input-scan pointer. */
    char        *copy_start;            /* start of copy of input within
                                           constructed parse string */
    char        *save_copy_start;       /* Provides one level of saving
                                           and restoring 'copy_start' */
    char        *copy_start_in_input;   /* Position in input string
                                           corresponding to copy_start */
    SSize_t     whilem_seen;            /* number of WHILEM in this expr */
    regnode     *emit_start;            /* Start of emitted-code area */
    regnode_offset emit;                /* Code-emit pointer */
    I32         naughty;                /* How bad is this pattern? */
    I32         sawback;                /* Did we see \1, ...? */
    SSize_t     size;                   /* Number of regnode equivalents in
                                           pattern */
    Size_t      sets_depth;              /* Counts recursion depth of already-
                                           compiled regex set patterns */
    U32         seen;

    I32      parens_buf_size;           /* #slots malloced open/close_parens */
    regnode_offset *open_parens;        /* offsets to open parens */
    regnode_offset *close_parens;       /* offsets to close parens */
    HV          *paren_names;           /* Paren names */

    /* position beyond 'precomp' of the warning message furthest away from
     * 'precomp'.  During the parse, no warnings are raised for any problems
     * earlier in the parse than this position.  This works if warnings are
     * raised the first time a given spot is parsed, and if only one
     * independent warning is raised for any given spot */
    Size_t      latest_warn_offset;

    /* Branch reset /(?|...|...)/ gives us two concepts of capture buffer id.
     * "Logical Parno" is the user visible view with branch reset taken into
     * account. "Parno" (or physical parno) is the actual capture buffers in
     * the pattern *NOT* taking into account branch reset. We also maintain
     * a map of "next" pointers which allow us to skip to the next physical
     * capture buffer with the same logical id, with 0 representing "none".
     *
     * As we compile we keep track of the two different counts using the
     * 'logical_npar' and 'npar' members, and we keep track of the upper bound
     * of both in 'total_par' and 'logical_total_par', we also populate
     * the 'logical_to_parno' map, which gives us the first physical parno
     * for a given logical parno, and the `parno_to_logical` array which gives
     * us the logical id for each physical parno. When compilation is
     * completed we construct the 'parno_to_logical_next' array from the
     * 'parno_to_logical' array. (We do not bother constructing it during
     * compilation as we do not need it, and we can construct it in O(N) time
     * once we are done, but would need more complicated logic during the
     * compile, because we want the next pointers to go from smallest to
     * largest, eg, left to right.)
     *
     * Logical: $1      $2  $3  $4    $2  $3    $2    $5
     * Physical: 1       2   3   4     5   6     7     8
     * Next:     0       5   6   0     7   0     0     0
     * Pattern /(a) (?| (b) (c) (d) | (e) (f) | (g) ) (h)/
     *
     * As much as possible the internals use and store the physical id of
     * of capture buffers. We decode the physical to the logical only when
     * we need to, for instance when someone use $2.
     *
     * Note that when branch reset is not used logical and physical are the
     * same and the next data would be all zero. So when branch reset is not
     * used we do not need to populate this data into the final regexp.
     *
     */
    I32         *logical_to_parno;        /* logical_parno to parno */
    I32         *parno_to_logical;        /* parno to logical_parno */
    I32         *parno_to_logical_next;   /* parno to next (greater value)
                                             parno with the same
                                             logical_parno as parno.*/

    I32         npar;                   /* Capture buffer count so far in the
                                           parse, (OPEN) plus one. ("par" 0 is
                                           the whole pattern)*/
    I32         logical_npar;           /* Logical version of npar */
    I32         total_par;              /* During initial parse, is either 0,
                                           or -1; the latter indicating a
                                           reparse is needed.  After that pass,
                                           it is what 'npar' became after the
                                           pass.  Hence, it being > 0 indicates
                                           we are in a reparse situation */
    I32         logical_total_par;      /* Logical version to total par */
    I32         nestroot;               /* root parens we are in - used by
                                           accept */
    I32         seen_zerolen;
    regnode     *end_op;                /* END node in program */
    I32         utf8;           /* whether the pattern is utf8 or not */
    I32         orig_utf8;      /* whether the pattern was originally in utf8 */
                                /* XXX use this for future optimisation of case
                                 * where pattern must be upgraded to utf8. */
    I32         uni_semantics;  /* If a d charset modifier should use unicode
                                   rules, even if the pattern is not in
                                   utf8 */

    I32         recurse_count;          /* Number of recurse regops we have generated */
    regnode     **recurse;              /* Recurse regops */
    U8          *study_chunk_recursed;  /* bitmap of which subs we have moved
                                           through */
    U32         study_chunk_recursed_bytes;  /* bytes in bitmap */
    I32         in_lookaround;
    I32         contains_locale;
    I32         override_recoding;
    I32         recode_x_to_native;
    I32         in_multi_char_class;
    int         code_index;             /* next code_blocks[] slot */
    struct reg_code_blocks *code_blocks;/* positions of literal (?{})
                                            within pattern */
    SSize_t     maxlen;                        /* mininum possible number of chars in string to match */
    scan_frame *frame_head;
    scan_frame *frame_last;
    U32         frame_count;
    AV         *warn_text;
    HV         *unlexed_names;
    SV          *runtime_code_qr;       /* qr with the runtime code blocks */
#ifdef DEBUGGING
    const char  *lastparse;
    I32         lastnum;
    U32         study_chunk_recursed_count;
    AV          *paren_name_list;       /* idx -> name */
    SV          *mysv1;
    SV          *mysv2;
#endif
    bool        seen_d_op;
    bool        strict;
    bool        study_started;
    bool        in_script_run;
    bool        use_BRANCHJ;
    bool        sWARN_EXPERIMENTAL__VLB;
    bool        sWARN_EXPERIMENTAL__REGEX_SETS;
};

#ifdef DEBUGGING
#define RExC_lastparse  (pRExC_state->lastparse)
#define RExC_lastnum    (pRExC_state->lastnum)
#define RExC_paren_name_list    (pRExC_state->paren_name_list)
#define RExC_study_chunk_recursed_count    (pRExC_state->study_chunk_recursed_count)
#define RExC_mysv       (pRExC_state->mysv1)
#define RExC_mysv1      (pRExC_state->mysv1)
#define RExC_mysv2      (pRExC_state->mysv2)
#endif

#define RExC_flags      (pRExC_state->flags)
#define RExC_pm_flags   (pRExC_state->pm_flags)
#define RExC_precomp    (pRExC_state->precomp)
#define RExC_copy_start_in_input (pRExC_state->copy_start_in_input)
#define RExC_copy_start_in_constructed  (pRExC_state->copy_start)
#define RExC_save_copy_start_in_constructed  (pRExC_state->save_copy_start)
#define RExC_precomp_end (pRExC_state->precomp_end)
#define RExC_rx_sv      (pRExC_state->rx_sv)
#define RExC_rx         (pRExC_state->rx)
#define RExC_rxi        (pRExC_state->rxi)
#define RExC_start      (pRExC_state->start)
#define RExC_end        (pRExC_state->end)
#define RExC_parse      (pRExC_state->parse)
#define RExC_latest_warn_offset (pRExC_state->latest_warn_offset )
#define RExC_whilem_seen        (pRExC_state->whilem_seen)
#define RExC_seen_d_op (pRExC_state->seen_d_op) /* Seen something that differs
                                                   under /d from /u ? */

#define RExC_emit       (pRExC_state->emit)
#define RExC_emit_start (pRExC_state->emit_start)
#define RExC_sawback    (pRExC_state->sawback)
#define RExC_seen       (pRExC_state->seen)
#define RExC_size       (pRExC_state->size)
#define RExC_maxlen        (pRExC_state->maxlen)
#define RExC_logical_npar           (pRExC_state->logical_npar)
#define RExC_logical_total_parens   (pRExC_state->logical_total_par)
#define RExC_logical_to_parno       (pRExC_state->logical_to_parno)
#define RExC_parno_to_logical       (pRExC_state->parno_to_logical)
#define RExC_parno_to_logical_next  (pRExC_state->parno_to_logical_next)
#define RExC_npar       (pRExC_state->npar)
#define RExC_total_parens       (pRExC_state->total_par)
#define RExC_parens_buf_size    (pRExC_state->parens_buf_size)
#define RExC_nestroot   (pRExC_state->nestroot)
#define RExC_seen_zerolen       (pRExC_state->seen_zerolen)
#define RExC_utf8       (pRExC_state->utf8)
#define RExC_uni_semantics      (pRExC_state->uni_semantics)
#define RExC_orig_utf8  (pRExC_state->orig_utf8)
#define RExC_open_parens        (pRExC_state->open_parens)
#define RExC_close_parens       (pRExC_state->close_parens)
#define RExC_end_op     (pRExC_state->end_op)
#define RExC_paren_names        (pRExC_state->paren_names)
#define RExC_recurse    (pRExC_state->recurse)
#define RExC_recurse_count      (pRExC_state->recurse_count)
#define RExC_sets_depth         (pRExC_state->sets_depth)
#define RExC_study_chunk_recursed        (pRExC_state->study_chunk_recursed)
#define RExC_study_chunk_recursed_bytes  \
                                   (pRExC_state->study_chunk_recursed_bytes)
#define RExC_in_lookaround      (pRExC_state->in_lookaround)
#define RExC_contains_locale    (pRExC_state->contains_locale)
#define RExC_recode_x_to_native (pRExC_state->recode_x_to_native)

#ifdef EBCDIC
#  define SET_recode_x_to_native(x)                                         \
                    STMT_START { RExC_recode_x_to_native = (x); } STMT_END
#else
#  define SET_recode_x_to_native(x) NOOP
#endif

#define RExC_in_multi_char_class (pRExC_state->in_multi_char_class)
#define RExC_frame_head (pRExC_state->frame_head)
#define RExC_frame_last (pRExC_state->frame_last)
#define RExC_frame_count (pRExC_state->frame_count)
#define RExC_strict (pRExC_state->strict)
#define RExC_study_started      (pRExC_state->study_started)
#define RExC_warn_text (pRExC_state->warn_text)
#define RExC_in_script_run      (pRExC_state->in_script_run)
#define RExC_use_BRANCHJ        (pRExC_state->use_BRANCHJ)
#define RExC_warned_WARN_EXPERIMENTAL__VLB (pRExC_state->sWARN_EXPERIMENTAL__VLB)
#define RExC_warned_WARN_EXPERIMENTAL__REGEX_SETS (pRExC_state->sWARN_EXPERIMENTAL__REGEX_SETS)
#define RExC_unlexed_names (pRExC_state->unlexed_names)


/***********************************************************************/
/* UTILITY MACROS FOR ADVANCING OR SETTING THE PARSE "CURSOR" RExC_parse
 *
 * All of these macros depend on the above RExC_ accessor macros, which
 * in turns depend on a variable pRExC_state being in scope where they
 * are used. This is the standard regexp parser context variable which is
 * passed into every non-trivial parse function in this file.
 *
 * Note that the UTF macro is itself a wrapper around RExC_utf8, so all
 * of the macros which do not take an argument will operate on the
 * pRExC_state structure *only*.
 *
 * Please do NOT modify RExC_parse without using these macros. In the
 * future these macros will be extended for enhanced debugging and trace
 * output during the parse process.
 */

/* RExC_parse_incf(flag)
 *
 * Increment RExC_parse to point at the next codepoint, while doing
 * the right thing depending on whether we are parsing UTF-8 strings
 * or not. The 'flag' argument determines if content is UTF-8 or not,
 * intended for cases where this is NOT governed by the UTF macro.
 *
 * Use RExC_parse_inc() if UTF-8ness is controlled by the UTF macro.
 *
 * WARNING: Does NOT take into account RExC_end; it is the callers
 * responsibility to make sure there are enough octets left in
 * RExC_parse to ensure that when processing UTF-8 we would not read
 * past the end of the string.
 */
#define RExC_parse_incf(flag) STMT_START {              \
    RExC_parse += (flag) ? UTF8SKIP(RExC_parse) : 1;    \
} STMT_END

/* RExC_parse_inc_safef(flag)
 *
 * Safely increment RExC_parse to point at the next codepoint,
 * doing the right thing depending on whether we are parsing
 * UTF-8 strings or not and NOT reading past the end of the buffer.
 * The 'flag' argument determines if content is UTF-8 or not,
 * intended for cases where this is NOT governed by the UTF macro.
 *
 * Use RExC_parse_safe() if UTF-8ness is controlled by the UTF macro.
 *
 * NOTE: Will NOT read past RExC_end when content is UTF-8.
 */
#define RExC_parse_inc_safef(flag) STMT_START {                     \
    RExC_parse += (flag) ? UTF8_SAFE_SKIP(RExC_parse,RExC_end) : 1; \
} STMT_END

/* RExC_parse_inc()
 *
 * Increment RExC_parse to point at the next codepoint,
 * doing the right thing depending on whether we are parsing
 * UTF-8 strings or not.
 *
 * WARNING: Does NOT take into account RExC_end, it is the callers
 * responsibility to make sure there are enough octets left in
 * RExC_parse to ensure that when processing UTF-8 we would not read
 * past the end of the string.
 *
 * NOTE: whether we are parsing UTF-8 or not is determined by the
 * UTF macro which is defined as cBOOL(RExC_parse_utf8), thus this
 * macro operates on the pRExC_state structure only.
 */
#define RExC_parse_inc() RExC_parse_incf(UTF)

/* RExC_parse_inc_safe()
 *
 * Safely increment RExC_parse to point at the next codepoint,
 * doing the right thing depending on whether we are parsing
 * UTF-8 strings or not and NOT reading past the end of the buffer.
 *
 * NOTE: whether we are parsing UTF-8 or not is determined by the
 * UTF macro which is defined as cBOOL(RExC_parse_utf8), thus this
 * macro operates on the pRExC_state structure only.
 */
#define RExC_parse_inc_safe() RExC_parse_inc_safef(UTF)

/* RExC_parse_inc_utf8()
 *
 * Increment RExC_parse to point at the next utf8 codepoint,
 * assumes content is UTF-8.
 *
 * WARNING: Does NOT take into account RExC_end; it is the callers
 * responsibility to make sure there are enough octets left in RExC_parse
 * to ensure that when processing UTF-8 we would not read past the end
 * of the string.
 */
#define RExC_parse_inc_utf8() STMT_START {  \
    RExC_parse += UTF8SKIP(RExC_parse);     \
} STMT_END

/* RExC_parse_inc_if_char()
 *
 * Increment RExC_parse to point at the next codepoint, if and only
 * if the current parse point is NOT a NULL, while doing the right thing
 * depending on whether we are parsing UTF-8 strings or not.
 *
 * WARNING: Does NOT take into account RExC_end, it is the callers
 * responsibility to make sure there are enough octets left in RExC_parse
 * to ensure that when processing UTF-8 we would not read past the end
 * of the string.
 *
 * NOTE: whether we are parsing UTF-8 or not is determined by the
 * UTF macro which is defined as cBOOL(RExC_parse_utf8), thus this
 * macro operates on the pRExC_state structure only.
 */
#define RExC_parse_inc_if_char() STMT_START {         \
    RExC_parse += SKIP_IF_CHAR(RExC_parse,RExC_end);  \
} STMT_END

/* RExC_parse_inc_by(n_octets)
 *
 * Increment the parse cursor by the number of octets specified by
 * the 'n_octets' argument.
 *
 * NOTE: Does NOT check ANY constraints. It is the callers responsibility
 * that this will not move past the end of the string, or leave the
 * pointer in the middle of a UTF-8 sequence.
 *
 * Typically used to advanced past previously analyzed content.
 */
#define RExC_parse_inc_by(n_octets) STMT_START {  \
    RExC_parse += (n_octets);                     \
} STMT_END

/* RExC_parse_set(to_ptr)
 *
 * Sets the RExC_parse pointer to the pointer specified by the 'to'
 * argument. No validation whatsoever is performed on the to pointer.
 */
#define RExC_parse_set(to_ptr) STMT_START { \
    RExC_parse = (to_ptr);                  \
} STMT_END

/**********************************************************************/

/* Heuristic check on the complexity of the pattern: if TOO_NAUGHTY, we set
 * a flag to disable back-off on the fixed/floating substrings - if it's
 * a high complexity pattern we assume the benefit of avoiding a full match
 * is worth the cost of checking for the substrings even if they rarely help.
 */
#define RExC_naughty    (pRExC_state->naughty)
#define TOO_NAUGHTY (10)
#define MARK_NAUGHTY(add) \
    if (RExC_naughty < TOO_NAUGHTY) \
        RExC_naughty += (add)
#define MARK_NAUGHTY_EXP(exp, add) \
    if (RExC_naughty < TOO_NAUGHTY) \
        RExC_naughty += RExC_naughty / (exp) + (add)

#define isNON_BRACE_QUANTIFIER(c)   ((c) == '*' || (c) == '+' || (c) == '?')
#define isQUANTIFIER(s,e)  (   isNON_BRACE_QUANTIFIER(*s)                      \
                            || ((*s) == '{' && regcurly(s, e, NULL)))

/*
 * Flags to be passed up.
 */
#define HASWIDTH        0x01    /* Known to not match null strings, could match
                                   non-null ones. */
#define SIMPLE          0x02    /* Exactly one character wide */
                                /* (or LNBREAK as a special case) */
#define POSTPONED       0x08    /* (?1),(?&name), (??{...}) or similar */
#define TRYAGAIN        0x10    /* Weeded out a declaration. */
#define RESTART_PARSE   0x20    /* Need to redo the parse */
#define NEED_UTF8       0x40    /* In conjunction with RESTART_PARSE, need to
                                   calcuate sizes as UTF-8 */

#define REG_NODE_NUM(x) ((x) ? (int)((x)-RExC_emit_start) : -1)

/* whether trie related optimizations are enabled */
#if PERL_ENABLE_EXTENDED_TRIE_OPTIMISATION
#define TRIE_STUDY_OPT
#define FULL_TRIE_STUDY
#define TRIE_STCLASS
#endif

/* About the term "restudy" and the var "restudied" and the defines
 * "SCF_TRIE_RESTUDY" and "SCF_TRIE_DOING_RESTUDY": All of these relate to
 * doing multiple study_chunk() calls over the same set of opcodes for* the
 * purpose of enhanced TRIE optimizations.
 *
 * Specifically, when TRIE_STUDY_OPT is defined, and it is defined in normal
 * builds, (see above), during compilation SCF_TRIE_RESTUDY may be enabled
 * which then causes the Perl_re_op_compile() to then call the optimizer
 * S_study_chunk() a second time to perform additional optimizations,
 * including the aho_corasick startclass optimization.
 * This additional pass will only happen once, which is managed by the
 * 'restudied' variable in Perl_re_op_compile().
 *
 * When this second pass is under way the flags passed into study_chunk() will
 * include SCF_TRIE_DOING_RESTUDY and this flag is and must be cascaded down
 * to any recursive calls to S_study_chunk().
 *
 * IMPORTANT: Any logic in study_chunk() that emits warnings should check that
 * the SCF_TRIE_DOING_RESTUDY flag is NOT set in 'flags', or the warning may
 * be produced twice.
 *
 * See commit 07be1b83a6b2d24b492356181ddf70e1c7917ae3 and
 * 688e03912e3bff2d2419c457d8b0e1bab3eb7112 for more details.
 */


#define PBYTE(u8str,paren) ((U8*)(u8str))[(paren) >> 3]
#define PBITVAL(paren) (1 << ((paren) & 7))
#define PAREN_OFFSET(depth) \
    (RExC_study_chunk_recursed + (depth) * RExC_study_chunk_recursed_bytes)
#define PAREN_TEST(depth, paren) \
    (PBYTE(PAREN_OFFSET(depth), paren) & PBITVAL(paren))
#define PAREN_SET(depth, paren) \
    (PBYTE(PAREN_OFFSET(depth), paren) |= PBITVAL(paren))
#define PAREN_UNSET(depth, paren) \
    (PBYTE(PAREN_OFFSET(depth), paren) &= ~PBITVAL(paren))

#define REQUIRE_UTF8(flagp) STMT_START {                                   \
                                     if (!UTF) {                           \
                                         *flagp = RESTART_PARSE|NEED_UTF8; \
                                         return 0;                         \
                                     }                                     \
                             } STMT_END

/* /u is to be chosen if we are supposed to use Unicode rules, or if the
 * pattern is in UTF-8.  This latter condition is in case the outermost rules
 * are locale.  See GH #17278 */
#define toUSE_UNI_CHARSET_NOT_DEPENDS (RExC_uni_semantics || UTF)

/* Change from /d into /u rules, and restart the parse.  RExC_uni_semantics is
 * a flag that indicates we need to override /d with /u as a result of
 * something in the pattern.  It should only be used in regards to calling
 * set_regex_charset() or get_regex_charset() */
#define REQUIRE_UNI_RULES(flagp, restart_retval)                            \
    STMT_START {                                                            \
            if (DEPENDS_SEMANTICS) {                                        \
                set_regex_charset(&RExC_flags, REGEX_UNICODE_CHARSET);      \
                RExC_uni_semantics = 1;                                     \
                if (RExC_seen_d_op && LIKELY(! IN_PARENS_PASS)) {           \
                    /* No need to restart the parse if we haven't seen      \
                     * anything that differs between /u and /d, and no need \
                     * to restart immediately if we're going to reparse     \
                     * anyway to count parens */                            \
                    *flagp |= RESTART_PARSE;                                \
                    return restart_retval;                                  \
                }                                                           \
            }                                                               \
    } STMT_END

#define REQUIRE_BRANCHJ(flagp, restart_retval)                              \
    STMT_START {                                                            \
                RExC_use_BRANCHJ = 1;                                       \
                *flagp |= RESTART_PARSE;                                    \
                return restart_retval;                                      \
    } STMT_END

/* Until we have completed the parse, we leave RExC_total_parens at 0 or
 * less.  After that, it must always be positive, because the whole re is
 * considered to be surrounded by virtual parens.  Setting it to negative
 * indicates there is some construct that needs to know the actual number of
 * parens to be properly handled.  And that means an extra pass will be
 * required after we've counted them all */
#define ALL_PARENS_COUNTED (RExC_total_parens > 0)
#define REQUIRE_PARENS_PASS                                                 \
    STMT_START {  /* No-op if have completed a pass */                      \
                    if (! ALL_PARENS_COUNTED) RExC_total_parens = -1;       \
    } STMT_END
#define IN_PARENS_PASS (RExC_total_parens < 0)


/* This is used to return failure (zero) early from the calling function if
 * various flags in 'flags' are set.  Two flags always cause a return:
 * 'RESTART_PARSE' and 'NEED_UTF8'.   'extra' can be used to specify any
 * additional flags that should cause a return; 0 if none.  If the return will
 * be done, '*flagp' is first set to be all of the flags that caused the
 * return. */
#define RETURN_FAIL_ON_RESTART_OR_FLAGS(flags,flagp,extra)                  \
    STMT_START {                                                            \
            if ((flags) & (RESTART_PARSE|NEED_UTF8|(extra))) {              \
                *(flagp) = (flags) & (RESTART_PARSE|NEED_UTF8|(extra));     \
                return 0;                                                   \
            }                                                               \
    } STMT_END

#define MUST_RESTART(flags) ((flags) & (RESTART_PARSE))

#define RETURN_FAIL_ON_RESTART(flags,flagp)                                 \
                        RETURN_FAIL_ON_RESTART_OR_FLAGS( flags, flagp, 0)
#define RETURN_FAIL_ON_RESTART_FLAGP(flagp)                                 \
                                    if (MUST_RESTART(*(flagp))) return 0

/* This converts the named class defined in regcomp.h to its equivalent class
 * number defined in handy.h. */
#define namedclass_to_classnum(class)  ((int) ((class) / 2))
#define classnum_to_namedclass(classnum)  ((classnum) * 2)

#define _invlist_union_complement_2nd(a, b, output) \
                        _invlist_union_maybe_complement_2nd(a, b, TRUE, output)
#define _invlist_intersection_complement_2nd(a, b, output) \
                 _invlist_intersection_maybe_complement_2nd(a, b, TRUE, output)

/* We add a marker if we are deferring expansion of a property that is both
 * 1) potentiallly user-defined; and
 * 2) could also be an official Unicode property.
 *
 * Without this marker, any deferred expansion can only be for a user-defined
 * one.  This marker shouldn't conflict with any that could be in a legal name,
 * and is appended to its name to indicate this.  There is a string and
 * character form */
#define DEFERRED_COULD_BE_OFFICIAL_MARKERs  "~"
#define DEFERRED_COULD_BE_OFFICIAL_MARKERc  '~'

/* What is infinity for optimization purposes */
#define OPTIMIZE_INFTY  SSize_t_MAX

/* About scan_data_t.

  During optimisation we recurse through the regexp program performing
  various inplace (keyhole style) optimisations. In addition study_chunk
  and scan_commit populate this data structure with information about
  what strings MUST appear in the pattern. We look for the longest
  string that must appear at a fixed location, and we look for the
  longest string that may appear at a floating location. So for instance
  in the pattern:

    /FOO[xX]A.*B[xX]BAR/

  Both 'FOO' and 'A' are fixed strings. Both 'B' and 'BAR' are floating
  strings (because they follow a .* construct). study_chunk will identify
  both FOO and BAR as being the longest fixed and floating strings respectively.

  The strings can be composites, for instance

     /(f)(o)(o)/

  will result in a composite fixed substring 'foo'.

  For each string some basic information is maintained:

  - min_offset
    This is the position the string must appear at, or not before.
    It also implicitly (when combined with minlenp) tells us how many
    characters must match before the string we are searching for.
    Likewise when combined with minlenp and the length of the string it
    tells us how many characters must appear after the string we have
    found.

  - max_offset
    Only used for floating strings. This is the rightmost point that
    the string can appear at. If set to OPTIMIZE_INFTY it indicates that the
    string can occur infinitely far to the right.
    For fixed strings, it is equal to min_offset.

  - minlenp
    A pointer to the minimum number of characters of the pattern that the
    string was found inside. This is important as in the case of positive
    lookahead or positive lookbehind we can have multiple patterns
    involved. Consider

    /(?=FOO).*F/

    The minimum length of the pattern overall is 3, the minimum length
    of the lookahead part is 3, but the minimum length of the part that
    will actually match is 1. So 'FOO's minimum length is 3, but the
    minimum length for the F is 1. This is important as the minimum length
    is used to determine offsets in front of and behind the string being
    looked for.  Since strings can be composites this is the length of the
    pattern at the time it was committed with a scan_commit. Note that
    the length is calculated by study_chunk, so that the minimum lengths
    are not known until the full pattern has been compiled, thus the
    pointer to the value.

  - lookbehind

    In the case of lookbehind the string being searched for can be
    offset past the start point of the final matching string.
    If this value was just blithely removed from the min_offset it would
    invalidate some of the calculations for how many chars must match
    before or after (as they are derived from min_offset and minlen and
    the length of the string being searched for).
    When the final pattern is compiled and the data is moved from the
    scan_data_t structure into the regexp structure the information
    about lookbehind is factored in, with the information that would
    have been lost precalculated in the end_shift field for the
    associated string.

  The fields pos_min and pos_delta are used to store the minimum offset
  and the delta to the maximum offset at the current point in the pattern.

*/

struct scan_data_substrs {
    SV      *str;       /* longest substring found in pattern */
    SSize_t min_offset; /* earliest point in string it can appear */
    SSize_t max_offset; /* latest point in string it can appear */
    SSize_t *minlenp;   /* pointer to the minlen relevant to the string */
    SSize_t lookbehind; /* is the pos of the string modified by LB */
    I32 flags;          /* per substring SF_* and SCF_* flags */
};

/* this is typedef'ed in perl.h */
struct scan_data_t {
    /*I32 len_min;      unused */
    /*I32 len_delta;    unused */
    SSize_t pos_min;
    SSize_t pos_delta;
    SV *last_found;
    SSize_t last_end;       /* min value, <0 unless valid. */
    SSize_t last_start_min;
    SSize_t last_start_max;
    U8      cur_is_floating; /* whether the last_* values should be set as
                              * the next fixed (0) or floating (1)
                              * substring */

    /* [0] is longest fixed substring so far, [1] is longest float so far */
    struct scan_data_substrs  substrs[2];

    I32 flags;             /* common SF_* and SCF_* flags */
    I32 whilem_c;
    SSize_t *last_closep;
    regnode **last_close_opp; /* pointer to pointer to last CLOSE regop
                                 seen. DO NOT DEREFERENCE the regnode
                                 pointer - the op may have been optimized
                                 away */
    regnode_ssc *start_class;
};

/*
 * Forward declarations for pregcomp()'s friends.
 */

static const scan_data_t zero_scan_data = {
    0, 0, NULL, 0, 0, 0, 0,
    {
        { NULL, 0, 0, 0, 0, 0 },
        { NULL, 0, 0, 0, 0, 0 },
    },
    0, 0, NULL, NULL, NULL
};

/* study flags */

#define SF_BEFORE_SEOL          0x0001
#define SF_BEFORE_MEOL          0x0002
#define SF_BEFORE_EOL           (SF_BEFORE_SEOL|SF_BEFORE_MEOL)

#define SF_IS_INF               0x0040
#define SF_HAS_PAR              0x0080
#define SF_IN_PAR               0x0100
#define SF_HAS_EVAL             0x0200


/* SCF_DO_SUBSTR is the flag that tells the regexp analyzer to track the
 * longest substring in the pattern. When it is not set the optimiser keeps
 * track of position, but does not keep track of the actual strings seen,
 *
 * So for instance /foo/ will be parsed with SCF_DO_SUBSTR being true, but
 * /foo/i will not.
 *
 * Similarly, /foo.*(blah|erm|huh).*fnorble/ will have "foo" and "fnorble"
 * parsed with SCF_DO_SUBSTR on, but while processing the (...) it will be
 * turned off because of the alternation (BRANCH). */
#define SCF_DO_SUBSTR           0x0400

#define SCF_DO_STCLASS_AND      0x0800
#define SCF_DO_STCLASS_OR       0x1000
#define SCF_DO_STCLASS          (SCF_DO_STCLASS_AND|SCF_DO_STCLASS_OR)
#define SCF_WHILEM_VISITED_POS  0x2000

#define SCF_TRIE_RESTUDY        0x4000 /* Need to do restudy in study_chunk()?
                                          Search for "restudy" in this file
                                          to find a detailed explanation.*/
#define SCF_SEEN_ACCEPT         0x8000
#define SCF_TRIE_DOING_RESTUDY 0x10000 /* Are we in restudy right now?
                                          Search for "restudy" in this file
                                          to find a detailed explanation. */
#define SCF_IN_DEFINE          0x20000



#define UTF cBOOL(RExC_utf8)

/* The enums for all these are ordered so things work out correctly */
#define LOC (get_regex_charset(RExC_flags) == REGEX_LOCALE_CHARSET)
#define DEPENDS_SEMANTICS (get_regex_charset(RExC_flags)                    \
                                                     == REGEX_DEPENDS_CHARSET)
#define UNI_SEMANTICS (get_regex_charset(RExC_flags) == REGEX_UNICODE_CHARSET)
#define AT_LEAST_UNI_SEMANTICS (get_regex_charset(RExC_flags)                \
                                                     >= REGEX_UNICODE_CHARSET)
#define ASCII_RESTRICTED (get_regex_charset(RExC_flags)                      \
                                            == REGEX_ASCII_RESTRICTED_CHARSET)
#define AT_LEAST_ASCII_RESTRICTED (get_regex_charset(RExC_flags)             \
                                            >= REGEX_ASCII_RESTRICTED_CHARSET)
#define ASCII_FOLD_RESTRICTED (get_regex_charset(RExC_flags)                 \
                                        == REGEX_ASCII_MORE_RESTRICTED_CHARSET)

#define FOLD cBOOL(RExC_flags & RXf_PMf_FOLD)

/* For programs that want to be strictly Unicode compatible by dying if any
 * attempt is made to match a non-Unicode code point against a Unicode
 * property.  */
#define ALWAYS_WARN_SUPER  ckDEAD(packWARN(WARN_NON_UNICODE))

#define OOB_NAMEDCLASS          -1

/* There is no code point that is out-of-bounds, so this is problematic.  But
 * its only current use is to initialize a variable that is always set before
 * looked at. */
#define OOB_UNICODE             0xDEADBEEF

#define CHR_SVLEN(sv) (UTF ? sv_len_utf8(sv) : SvCUR(sv))


/* length of regex to show in messages that don't mark a position within */
#define RegexLengthToShowInErrorMessages 127

/*
 * If MARKER[12] are adjusted, be sure to adjust the constants at the top
 * of t/op/regmesg.t, the tests in t/op/re_tests, and those in
 * op/pragma/warn/regcomp.
 */
#define MARKER1 "<-- HERE"    /* marker as it appears in the description */
#define MARKER2 " <-- HERE "  /* marker as it appears within the regex */

#define REPORT_LOCATION " in regex; marked by " MARKER1    \
                        " in m/%" UTF8f MARKER2 "%" UTF8f "/"

/* The code in this file in places uses one level of recursion with parsing
 * rebased to an alternate string constructed by us in memory.  This can take
 * the form of something that is completely different from the input, or
 * something that uses the input as part of the alternate.  In the first case,
 * there should be no possibility of an error, as we are in complete control of
 * the alternate string.  But in the second case we don't completely control
 * the input portion, so there may be errors in that.  Here's an example:
 *      /[abc\x{DF}def]/ui
 * is handled specially because \x{df} folds to a sequence of more than one
 * character: 'ss'.  What is done is to create and parse an alternate string,
 * which looks like this:
 *      /(?:\x{DF}|[abc\x{DF}def])/ui
 * where it uses the input unchanged in the middle of something it constructs,
 * which is a branch for the DF outside the character class, and clustering
 * parens around the whole thing. (It knows enough to skip the DF inside the
 * class while in this substitute parse.) 'abc' and 'def' may have errors that
 * need to be reported.  The general situation looks like this:
 *
 *                                       |<------- identical ------>|
 *              sI                       tI               xI       eI
 * Input:       ---------------------------------------------------------------
 * Constructed:         ---------------------------------------------------
 *                      sC               tC               xC       eC     EC
 *                                       |<------- identical ------>|
 *
 * sI..eI   is the portion of the input pattern we are concerned with here.
 * sC..EC   is the constructed substitute parse string.
 *  sC..tC  is constructed by us
 *  tC..eC  is an exact duplicate of the portion of the input pattern tI..eI.
 *          In the diagram, these are vertically aligned.
 *  eC..EC  is also constructed by us.
 * xC       is the position in the substitute parse string where we found a
 *          problem.
 * xI       is the position in the original pattern corresponding to xC.
 *
 * We want to display a message showing the real input string.  Thus we need to
 * translate from xC to xI.  We know that xC >= tC, since the portion of the
 * string sC..tC has been constructed by us, and so shouldn't have errors.  We
 * get:
 *      xI = tI + (xC - tC)
 *
 * When the substitute parse is constructed, the code needs to set:
 *      RExC_start (sC)
 *      RExC_end (eC)
 *      RExC_copy_start_in_input  (tI)
 *      RExC_copy_start_in_constructed (tC)
 * and restore them when done.
 *
 * During normal processing of the input pattern, both
 * 'RExC_copy_start_in_input' and 'RExC_copy_start_in_constructed' are set to
 * sI, so that xC equals xI.
 */

#define sI              RExC_precomp
#define eI              RExC_precomp_end
#define sC              RExC_start
#define eC              RExC_end
#define tI              RExC_copy_start_in_input
#define tC              RExC_copy_start_in_constructed
#define xI(xC)          (tI + (xC - tC))
#define xI_offset(xC)   (xI(xC) - sI)

#define REPORT_LOCATION_ARGS(xC)                                            \
    UTF8fARG(UTF,                                                           \
             (xI(xC) > eI) /* Don't run off end */                          \
              ? eI - sI   /* Length before the <--HERE */                   \
              : ((xI_offset(xC) >= 0)                                       \
                 ? xI_offset(xC)                                            \
                 : (Perl_croak(aTHX_ "panic: %s: %d: negative offset: %"    \
                                    IVdf " trying to output message for "   \
                                    " pattern %.*s",                        \
                                    __FILE__, __LINE__, (IV) xI_offset(xC), \
                                    ((int) (eC - sC)), sC), 0)),            \
             sI),         /* The input pattern printed up to the <--HERE */ \
    UTF8fARG(UTF,                                                           \
             (xI(xC) > eI) ? 0 : eI - xI(xC), /* Length after <--HERE */    \
             (xI(xC) > eI) ? eI : xI(xC))     /* pattern after <--HERE */

/* Used to point after bad bytes for an error message, but avoid skipping
 * past a nul byte. */
#define SKIP_IF_CHAR(s, e) (!*(s) ? 0 : UTF ? UTF8_SAFE_SKIP(s, e) : 1)

/* Set up to clean up after our imminent demise */
#define PREPARE_TO_DIE                                                      \
    STMT_START {                                                            \
        if (RExC_rx_sv)                                                     \
            SAVEFREESV(RExC_rx_sv);                                         \
        if (RExC_open_parens)                                               \
            SAVEFREEPV(RExC_open_parens);                                   \
        if (RExC_close_parens)                                              \
            SAVEFREEPV(RExC_close_parens);                                  \
        if (RExC_logical_to_parno)                                          \
            SAVEFREEPV(RExC_logical_to_parno);                              \
        if (RExC_parno_to_logical)                                          \
            SAVEFREEPV(RExC_parno_to_logical);                              \
    } STMT_END

/*
 * Calls SAVEDESTRUCTOR_X if needed, then calls Perl_croak with the given
 * arg. Show regex, up to a maximum length. If it's too long, chop and add
 * "...".
 */
#define _FAIL(code) STMT_START {                                        \
    const char *ellipses = "";                                          \
    IV len = RExC_precomp_end - RExC_precomp;                           \
                                                                        \
    PREPARE_TO_DIE;                                                     \
    if (len > RegexLengthToShowInErrorMessages) {                       \
        /* chop 10 shorter than the max, to ensure meaning of "..." */  \
        len = RegexLengthToShowInErrorMessages - 10;                    \
        ellipses = "...";                                               \
    }                                                                   \
    code;                                                               \
} STMT_END

#define FAIL(msg) _FAIL(                            \
    Perl_croak(aTHX_ "%s in regex m/%" UTF8f "%s/",         \
            msg, UTF8fARG(UTF, len, RExC_precomp), ellipses))

#define FAIL2(msg,arg) _FAIL(                       \
    Perl_croak(aTHX_ msg " in regex m/%" UTF8f "%s/",       \
            arg, UTF8fARG(UTF, len, RExC_precomp), ellipses))

#define FAIL3(msg,arg1,arg2) _FAIL(                         \
    Perl_croak(aTHX_ msg " in regex m/%" UTF8f "%s/",       \
     arg1, arg2, UTF8fARG(UTF, len, RExC_precomp), ellipses))

/*
 * Simple_vFAIL -- like FAIL, but marks the current location in the scan
 */
#define Simple_vFAIL(m) STMT_START {                                    \
    Perl_croak(aTHX_ "%s" REPORT_LOCATION,                              \
            m, REPORT_LOCATION_ARGS(RExC_parse));                       \
} STMT_END

/*
 * Calls SAVEDESTRUCTOR_X if needed, then Simple_vFAIL()
 */
#define vFAIL(m) STMT_START {                           \
    PREPARE_TO_DIE;                                     \
    Simple_vFAIL(m);                                    \
} STMT_END

/*
 * Like Simple_vFAIL(), but accepts two arguments.
 */
#define Simple_vFAIL2(m,a1) STMT_START {                        \
    S_re_croak(aTHX_ UTF, m REPORT_LOCATION, a1,                \
                      REPORT_LOCATION_ARGS(RExC_parse));        \
} STMT_END

/*
 * Calls SAVEDESTRUCTOR_X if needed, then Simple_vFAIL2().
 */
#define vFAIL2(m,a1) STMT_START {                       \
    PREPARE_TO_DIE;                                     \
    Simple_vFAIL2(m, a1);                               \
} STMT_END


/*
 * Like Simple_vFAIL(), but accepts three arguments.
 */
#define Simple_vFAIL3(m, a1, a2) STMT_START {                   \
    S_re_croak(aTHX_ UTF, m REPORT_LOCATION, a1, a2,            \
            REPORT_LOCATION_ARGS(RExC_parse));                  \
} STMT_END

/*
 * Calls SAVEDESTRUCTOR_X if needed, then Simple_vFAIL3().
 */
#define vFAIL3(m,a1,a2) STMT_START {                    \
    PREPARE_TO_DIE;                                     \
    Simple_vFAIL3(m, a1, a2);                           \
} STMT_END

/*
 * Like Simple_vFAIL(), but accepts four arguments.
 */
#define Simple_vFAIL4(m, a1, a2, a3) STMT_START {               \
    S_re_croak(aTHX_ UTF, m REPORT_LOCATION, a1, a2, a3,        \
            REPORT_LOCATION_ARGS(RExC_parse));                  \
} STMT_END

#define vFAIL4(m,a1,a2,a3) STMT_START {                 \
    PREPARE_TO_DIE;                                     \
    Simple_vFAIL4(m, a1, a2, a3);                       \
} STMT_END

/* A specialized version of vFAIL2 that works with UTF8f */
#define vFAIL2utf8f(m, a1) STMT_START {             \
    PREPARE_TO_DIE;                                 \
    S_re_croak(aTHX_ UTF, m REPORT_LOCATION, a1,  \
            REPORT_LOCATION_ARGS(RExC_parse));      \
} STMT_END

#define vFAIL3utf8f(m, a1, a2) STMT_START {             \
    PREPARE_TO_DIE;                                     \
    S_re_croak(aTHX_ UTF, m REPORT_LOCATION, a1, a2,  \
            REPORT_LOCATION_ARGS(RExC_parse));          \
} STMT_END

/* Setting this to NULL is a signal to not output warnings */
#define TURN_OFF_WARNINGS_IN_SUBSTITUTE_PARSE                               \
    STMT_START {                                                            \
      RExC_save_copy_start_in_constructed  = RExC_copy_start_in_constructed;\
      RExC_copy_start_in_constructed = NULL;                                \
    } STMT_END
#define RESTORE_WARNINGS                                                    \
    RExC_copy_start_in_constructed = RExC_save_copy_start_in_constructed

/* Since a warning can be generated multiple times as the input is reparsed, we
 * output it the first time we come to that point in the parse, but suppress it
 * otherwise.  'RExC_copy_start_in_constructed' being NULL is a flag to not
 * generate any warnings */
#define TO_OUTPUT_WARNINGS(loc)                                         \
  (   RExC_copy_start_in_constructed                                    \
   && ((xI(loc)) - RExC_precomp) > (Ptrdiff_t) RExC_latest_warn_offset)

/* After we've emitted a warning, we save the position in the input so we don't
 * output it again */
#define UPDATE_WARNINGS_LOC(loc)                                        \
    STMT_START {                                                        \
        if (TO_OUTPUT_WARNINGS(loc)) {                                  \
            RExC_latest_warn_offset = MAX(sI, MIN(eI, xI(loc)))         \
                                                       - RExC_precomp;  \
        }                                                               \
    } STMT_END

/* 'warns' is the output of the packWARNx macro used in 'code' */
#define _WARN_HELPER(loc, warns, code)                                  \
    STMT_START {                                                        \
        if (! RExC_copy_start_in_constructed) {                         \
            Perl_croak( aTHX_ "panic! %s: %d: Tried to warn when none"  \
                              " expected at '%s'",                      \
                              __FILE__, __LINE__, loc);                 \
        }                                                               \
        if (TO_OUTPUT_WARNINGS(loc)) {                                  \
            if (ckDEAD(warns))                                          \
                PREPARE_TO_DIE;                                         \
            code;                                                       \
            UPDATE_WARNINGS_LOC(loc);                                   \
        }                                                               \
    } STMT_END

/* m is not necessarily a "literal string", in this macro */
#define warn_non_literal_string(loc, packed_warn, m)                    \
    _WARN_HELPER(loc, packed_warn,                                      \
                      Perl_warner(aTHX_ packed_warn,                    \
                                       "%s" REPORT_LOCATION,            \
                                  m, REPORT_LOCATION_ARGS(loc)))
#define reg_warn_non_literal_string(loc, m)                             \
                warn_non_literal_string(loc, packWARN(WARN_REGEXP), m)

#define ckWARN2_non_literal_string(loc, packwarn, m, a1)                    \
    STMT_START {                                                            \
                char * format;                                              \
                Size_t format_size = strlen(m) + strlen(REPORT_LOCATION)+ 1;\
                Newx(format, format_size, char);                            \
                my_strlcpy(format, m, format_size);                         \
                my_strlcat(format, REPORT_LOCATION, format_size);           \
                SAVEFREEPV(format);                                         \
                _WARN_HELPER(loc, packwarn,                                 \
                      Perl_ck_warner(aTHX_ packwarn,                        \
                                        format,                             \
                                        a1, REPORT_LOCATION_ARGS(loc)));    \
    } STMT_END

#define ckWARNreg(loc,m)                                                \
    _WARN_HELPER(loc, packWARN(WARN_REGEXP),                            \
                      Perl_ck_warner(aTHX_ packWARN(WARN_REGEXP),       \
                                          m REPORT_LOCATION,            \
                                          REPORT_LOCATION_ARGS(loc)))

#define vWARN(loc, m)                                                   \
    _WARN_HELPER(loc, packWARN(WARN_REGEXP),                            \
                      Perl_warner(aTHX_ packWARN(WARN_REGEXP),          \
                                       m REPORT_LOCATION,               \
                                       REPORT_LOCATION_ARGS(loc)))      \

#define vWARN_dep(loc,category,m)                                           \
    _WARN_HELPER(loc, packWARN(category),                                   \
                      Perl_warner(aTHX_ packWARN(category),                 \
                                       m REPORT_LOCATION,                   \
                                       REPORT_LOCATION_ARGS(loc)))

#define ckWARNdep(loc,category,m)                                           \
    _WARN_HELPER(loc, packWARN(category),                                   \
                      Perl_ck_warner_d(aTHX_ packWARN(category),            \
                                            m REPORT_LOCATION,              \
                                            REPORT_LOCATION_ARGS(loc)))

#define ckWARNregdep(loc,category,m)                                        \
    _WARN_HELPER(loc, packWARN2(category, WARN_REGEXP),                     \
                      Perl_ck_warner_d(aTHX_ packWARN2(category,            \
                                                      WARN_REGEXP),         \
                                             m REPORT_LOCATION,             \
                                             REPORT_LOCATION_ARGS(loc)))

#define ckWARN2reg_d(loc,m, a1)                                             \
    _WARN_HELPER(loc, packWARN(WARN_REGEXP),                                \
                      Perl_ck_warner_d(aTHX_ packWARN(WARN_REGEXP),         \
                                            m REPORT_LOCATION,              \
                                            a1, REPORT_LOCATION_ARGS(loc)))

#define ckWARN2reg(loc, m, a1)                                              \
    _WARN_HELPER(loc, packWARN(WARN_REGEXP),                                \
                      Perl_ck_warner(aTHX_ packWARN(WARN_REGEXP),           \
                                          m REPORT_LOCATION,                \
                                          a1, REPORT_LOCATION_ARGS(loc)))

#define vWARN3(loc, m, a1, a2)                                              \
    _WARN_HELPER(loc, packWARN(WARN_REGEXP),                                \
                      Perl_warner(aTHX_ packWARN(WARN_REGEXP),              \
                                       m REPORT_LOCATION,                   \
                                       a1, a2, REPORT_LOCATION_ARGS(loc)))

#define ckWARN3reg(loc, m, a1, a2)                                          \
    _WARN_HELPER(loc, packWARN(WARN_REGEXP),                                \
                      Perl_ck_warner(aTHX_ packWARN(WARN_REGEXP),           \
                                          m REPORT_LOCATION,                \
                                          a1, a2,                           \
                                          REPORT_LOCATION_ARGS(loc)))

#define vWARN4(loc, m, a1, a2, a3)                                      \
    _WARN_HELPER(loc, packWARN(WARN_REGEXP),                            \
                      Perl_warner(aTHX_ packWARN(WARN_REGEXP),          \
                                       m REPORT_LOCATION,               \
                                       a1, a2, a3,                      \
                                       REPORT_LOCATION_ARGS(loc)))

#define ckWARN4reg(loc, m, a1, a2, a3)                                  \
    _WARN_HELPER(loc, packWARN(WARN_REGEXP),                            \
                      Perl_ck_warner(aTHX_ packWARN(WARN_REGEXP),       \
                                          m REPORT_LOCATION,            \
                                          a1, a2, a3,                   \
                                          REPORT_LOCATION_ARGS(loc)))

#define vWARN5(loc, m, a1, a2, a3, a4)                                  \
    _WARN_HELPER(loc, packWARN(WARN_REGEXP),                            \
                      Perl_warner(aTHX_ packWARN(WARN_REGEXP),          \
                                       m REPORT_LOCATION,               \
                                       a1, a2, a3, a4,                  \
                                       REPORT_LOCATION_ARGS(loc)))

#define ckWARNexperimental(loc, class, m)                               \
    STMT_START {                                                        \
        if (! RExC_warned_ ## class) { /* warn once per compilation */  \
            RExC_warned_ ## class = 1;                                  \
            _WARN_HELPER(loc, packWARN(class),                          \
                      Perl_ck_warner_d(aTHX_ packWARN(class),           \
                                            m REPORT_LOCATION,          \
                                            REPORT_LOCATION_ARGS(loc)));\
        }                                                               \
    } STMT_END

#define ckWARNexperimental_with_arg(loc, class, m, arg)                 \
    STMT_START {                                                        \
        if (! RExC_warned_ ## class) { /* warn once per compilation */  \
            RExC_warned_ ## class = 1;                                  \
            _WARN_HELPER(loc, packWARN(class),                          \
                      Perl_ck_warner_d(aTHX_ packWARN(class),           \
                                       m REPORT_LOCATION,               \
                                       arg, REPORT_LOCATION_ARGS(loc)));\
        }                                                               \
    } STMT_END

/* Convert between a pointer to a node and its offset from the beginning of the
 * program */
#define REGNODE_p(offset)    (RExC_emit_start + (offset))
#define REGNODE_OFFSET(node) (__ASSERT_((node) >= RExC_emit_start)      \
                              (SSize_t) ((node) - RExC_emit_start))

#define ProgLen(ri) ri->proglen
#define SetProgLen(ri,x) ri->proglen = x

#if PERL_ENABLE_EXPERIMENTAL_REGEX_OPTIMISATIONS
#define EXPERIMENTAL_INPLACESCAN
#endif /*PERL_ENABLE_EXPERIMENTAL_REGEX_OPTIMISATIONS*/

#define DEBUG_RExC_seen()                                                   \
        DEBUG_OPTIMISE_MORE_r({                                             \
            Perl_re_printf( aTHX_ "RExC_seen: ");                           \
                                                                            \
            if (RExC_seen & REG_ZERO_LEN_SEEN)                              \
                Perl_re_printf( aTHX_ "REG_ZERO_LEN_SEEN ");                \
                                                                            \
            if (RExC_seen & REG_LOOKBEHIND_SEEN)                            \
                Perl_re_printf( aTHX_ "REG_LOOKBEHIND_SEEN ");              \
                                                                            \
            if (RExC_seen & REG_GPOS_SEEN)                                  \
                Perl_re_printf( aTHX_ "REG_GPOS_SEEN ");                    \
                                                                            \
            if (RExC_seen & REG_RECURSE_SEEN)                               \
                Perl_re_printf( aTHX_ "REG_RECURSE_SEEN ");                 \
                                                                            \
            if (RExC_seen & REG_TOP_LEVEL_BRANCHES_SEEN)                    \
                Perl_re_printf( aTHX_ "REG_TOP_LEVEL_BRANCHES_SEEN ");      \
                                                                            \
            if (RExC_seen & REG_VERBARG_SEEN)                               \
                Perl_re_printf( aTHX_ "REG_VERBARG_SEEN ");                 \
                                                                            \
            if (RExC_seen & REG_CUTGROUP_SEEN)                              \
                Perl_re_printf( aTHX_ "REG_CUTGROUP_SEEN ");                \
                                                                            \
            if (RExC_seen & REG_RUN_ON_COMMENT_SEEN)                        \
                Perl_re_printf( aTHX_ "REG_RUN_ON_COMMENT_SEEN ");          \
                                                                            \
            if (RExC_seen & REG_UNFOLDED_MULTI_SEEN)                        \
                Perl_re_printf( aTHX_ "REG_UNFOLDED_MULTI_SEEN ");          \
                                                                            \
            if (RExC_seen & REG_UNBOUNDED_QUANTIFIER_SEEN)                  \
                Perl_re_printf( aTHX_ "REG_UNBOUNDED_QUANTIFIER_SEEN ");    \
                                                                            \
            if (RExC_seen & REG_PESSIMIZE_SEEN)                             \
                Perl_re_printf( aTHX_ "REG_PESSIMIZE_SEEN ");               \
                                                                            \
            Perl_re_printf( aTHX_ "\n");                                    \
        });

#define DEBUG_SHOW_STUDY_FLAG(flags,flag) \
  if ((flags) & flag) Perl_re_printf( aTHX_  "%s ", #flag)


#ifdef DEBUGGING
#  define DEBUG_STUDYDATA(where, data, depth, is_inf, min, stopmin, delta) \
                    debug_studydata(where, data, depth, is_inf, min, stopmin, delta)

#  define DEBUG_PEEP(str, scan, depth, flags)   \
                    debug_peep(str, pRExC_state, scan, depth, flags)
#else
#  define DEBUG_STUDYDATA(where, data, depth, is_inf, min, stopmin, delta) NOOP
#  define DEBUG_PEEP(str, scan, depth, flags)         NOOP
#endif

#define REGTAIL(x,y,z) regtail((x),(y),(z),depth+1)
#ifdef DEBUGGING
#define REGTAIL_STUDY(x,y,z) regtail_study((x),(y),(z),depth+1)
#else
#define REGTAIL_STUDY(x,y,z) regtail((x),(y),(z),depth+1)
#endif

#define MADE_TRIE       1
#define MADE_JUMP_TRIE  2
#define MADE_EXACT_TRIE 4

#define INVLIST_INDEX                   0
#define ONLY_LOCALE_MATCHES_INDEX       1
#define DEFERRED_USER_DEFINED_INDEX     2

/* These two functions currently do the exact same thing */
#define ssc_init_zero           ssc_init

#define ssc_add_cp(ssc, cp)   ssc_add_range((ssc), (cp), (cp))
#define ssc_match_all_cp(ssc) ssc_add_range(ssc, 0, UV_MAX)

#ifdef DEBUGGING
#define REGNODE_GUTS(state,op,extra_size) \
    regnode_guts_debug(state,op,extra_size)
#else
#define REGNODE_GUTS(state,op,extra_size) \
    regnode_guts(state,extra_size)
#endif

#define CLEAR_OPTSTART                                                          \
    if (optstart) STMT_START {                                                  \
        DEBUG_OPTIMISE_r(Perl_re_printf( aTHX_                                  \
                              " (%" IVdf " nodes)\n", (IV)(node - optstart)));  \
        optstart=NULL;                                                          \
    } STMT_END

#define DUMPUNTIL(b,e)                                          \
    CLEAR_OPTSTART;                                             \
    node = dumpuntil(r,start,(b),(e),last,sv,indent+1,depth+1);

#define REGNODE_STEP_OVER(ret,t1,t2) \
    NEXT_OFF(REGNODE_p(ret)) = ((sizeof(t1)+sizeof(t2))/sizeof(regnode))

#endif /* REGCOMP_INTERNAL_H */
