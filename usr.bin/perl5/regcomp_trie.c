#ifdef PERL_EXT_RE_BUILD
#include "re_top.h"
#endif

#include "EXTERN.h"
#define PERL_IN_REGEX_ENGINE
#define PERL_IN_REGCOMP_ANY
#define PERL_IN_REGCOMP_TRIE_C
#include "perl.h"

#ifdef PERL_IN_XSUB_RE
#  include "re_comp.h"
#else
#  include "regcomp.h"
#endif

#include "invlist_inline.h"
#include "unicode_constants.h"
#include "regcomp_internal.h"

#define TRIE_LIST_ITEM(state,idx) (trie->states[state].trans.list)[ idx ]
#define TRIE_LIST_CUR(state)  ( TRIE_LIST_ITEM( state, 0 ).forid )
#define TRIE_LIST_LEN(state) ( TRIE_LIST_ITEM( state, 0 ).newstate )
#define TRIE_LIST_USED(idx)  ( trie->states[state].trans.list         \
                               ? (TRIE_LIST_CUR( idx ) - 1)           \
                               : 0 )


#ifdef DEBUGGING
/*
   dump_trie(trie,widecharmap,revcharmap)
   dump_trie_interim_list(trie,widecharmap,revcharmap,next_alloc)
   dump_trie_interim_table(trie,widecharmap,revcharmap,next_alloc)

   These routines dump out a trie in a somewhat readable format.
   The _interim_ variants are used for debugging the interim
   tables that are used to generate the final compressed
   representation which is what dump_trie expects.

   Part of the reason for their existence is to provide a form
   of documentation as to how the different representations function.

*/

/*
  Dumps the final compressed table form of the trie to Perl_debug_log.
  Used for debugging make_trie().
*/

STATIC void
S_dump_trie(pTHX_ const struct _reg_trie_data *trie, HV *widecharmap,
            AV *revcharmap, U32 depth)
{
    U32 state;
    SV *sv=sv_newmortal();
    int colwidth= widecharmap ? 6 : 4;
    U16 word;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_DUMP_TRIE;

    Perl_re_indentf( aTHX_  "Char : %-6s%-6s%-4s ",
        depth+1, "Match","Base","Ofs" );

    for( state = 0 ; state < trie->uniquecharcount ; state++ ) {
        SV ** const tmp = av_fetch_simple( revcharmap, state, 0);
        if ( tmp ) {
            Perl_re_printf( aTHX_  "%*s",
                colwidth,
                pv_pretty(sv, SvPV_nolen_const(*tmp), SvCUR(*tmp), colwidth,
                            PL_colors[0], PL_colors[1],
                            (SvUTF8(*tmp) ? PERL_PV_ESCAPE_UNI : 0) |
                            PERL_PV_ESCAPE_FIRSTCHAR
                )
            );
        }
    }
    Perl_re_printf( aTHX_  "\n");
    Perl_re_indentf( aTHX_ "State|-----------------------", depth+1);

    for( state = 0 ; state < trie->uniquecharcount ; state++ )
        Perl_re_printf( aTHX_  "%.*s", colwidth, "--------");
    Perl_re_printf( aTHX_  "\n");

    for( state = 1 ; state < trie->statecount ; state++ ) {
        const U32 base = trie->states[ state ].trans.base;

        Perl_re_indentf( aTHX_  "#%4" UVXf "|", depth+1, (UV)state);

        if ( trie->states[ state ].wordnum ) {
            Perl_re_printf( aTHX_  " W%4X", trie->states[ state ].wordnum );
        } else {
            Perl_re_printf( aTHX_  "%6s", "" );
        }

        Perl_re_printf( aTHX_  " @%4" UVXf " ", (UV)base );

        if ( base ) {
            U32 ofs = 0;

            while( ( base + ofs  < trie->uniquecharcount ) ||
                   ( base + ofs - trie->uniquecharcount < trie->lasttrans
                     && trie->trans[ base + ofs - trie->uniquecharcount ].check
                                                                    != state))
                    ofs++;

            Perl_re_printf( aTHX_  "+%2" UVXf "[ ", (UV)ofs);

            for ( ofs = 0 ; ofs < trie->uniquecharcount ; ofs++ ) {
                if ( ( base + ofs >= trie->uniquecharcount )
                        && ( base + ofs - trie->uniquecharcount
                                                        < trie->lasttrans )
                        && trie->trans[ base + ofs
                                    - trie->uniquecharcount ].check == state )
                {
                   Perl_re_printf( aTHX_  "%*" UVXf, colwidth,
                    (UV)trie->trans[ base + ofs - trie->uniquecharcount ].next
                   );
                } else {
                    Perl_re_printf( aTHX_  "%*s", colwidth,"   ." );
                }
            }

            Perl_re_printf( aTHX_  "]");

        }
        Perl_re_printf( aTHX_  "\n" );
    }
    Perl_re_indentf( aTHX_  "word_info N:(prev,len)=",
                                depth);
    for (word=1; word <= trie->wordcount; word++) {
        Perl_re_printf( aTHX_  " %d:(%d,%d)",
            (int)word, (int)(trie->wordinfo[word].prev),
            (int)(trie->wordinfo[word].len));
    }
    Perl_re_printf( aTHX_  "\n" );
}
/*
  Dumps a fully constructed but uncompressed trie in list form.
  List tries normally only are used for construction when the number of
  possible chars (trie->uniquecharcount) is very high.
  Used for debugging make_trie().
*/
STATIC void
S_dump_trie_interim_list(pTHX_ const struct _reg_trie_data *trie,
                         HV *widecharmap, AV *revcharmap, U32 next_alloc,
                         U32 depth)
{
    U32 state;
    SV *sv=sv_newmortal();
    int colwidth= widecharmap ? 6 : 4;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_DUMP_TRIE_INTERIM_LIST;

    /* print out the table precompression.  */
    Perl_re_indentf( aTHX_  "State :Word | Transition Data\n",
            depth+1 );
    Perl_re_indentf( aTHX_  "%s",
            depth+1, "------:-----+-----------------\n" );

    for( state=1 ; state < next_alloc ; state ++ ) {
        U16 charid;

        Perl_re_indentf( aTHX_  " %4" UVXf " :",
            depth+1, (UV)state  );
        if ( ! trie->states[ state ].wordnum ) {
            Perl_re_printf( aTHX_  "%5s| ","");
        } else {
            Perl_re_printf( aTHX_  "W%4x| ",
                trie->states[ state ].wordnum
            );
        }
        for( charid = 1 ; charid <= TRIE_LIST_USED( state ) ; charid++ ) {
            SV ** const tmp = av_fetch_simple( revcharmap,
                                        TRIE_LIST_ITEM(state, charid).forid, 0);
            if ( tmp ) {
                Perl_re_printf( aTHX_  "%*s:%3X=%4" UVXf " | ",
                    colwidth,
                    pv_pretty(sv, SvPV_nolen_const(*tmp), SvCUR(*tmp),
                              colwidth,
                              PL_colors[0], PL_colors[1],
                              (SvUTF8(*tmp) ? PERL_PV_ESCAPE_UNI : 0)
                              | PERL_PV_ESCAPE_FIRSTCHAR
                    ) ,
                    TRIE_LIST_ITEM(state, charid).forid,
                    (UV)TRIE_LIST_ITEM(state, charid).newstate
                );
                if (!(charid % 10))
                    Perl_re_printf( aTHX_  "\n%*s| ",
                        (int)((depth * 2) + 14), "");
            }
        }
        Perl_re_printf( aTHX_  "\n");
    }
}

/*
  Dumps a fully constructed but uncompressed trie in table form.
  This is the normal DFA style state transition table, with a few
  twists to facilitate compression later.
  Used for debugging make_trie().
*/
STATIC void
S_dump_trie_interim_table(pTHX_ const struct _reg_trie_data *trie,
                          HV *widecharmap, AV *revcharmap, U32 next_alloc,
                          U32 depth)
{
    U32 state;
    U16 charid;
    SV *sv=sv_newmortal();
    int colwidth= widecharmap ? 6 : 4;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_DUMP_TRIE_INTERIM_TABLE;

    /*
       print out the table precompression so that we can do a visual check
       that they are identical.
     */

    Perl_re_indentf( aTHX_  "Char : ", depth+1 );

    for( charid = 0 ; charid < trie->uniquecharcount ; charid++ ) {
        SV ** const tmp = av_fetch_simple( revcharmap, charid, 0);
        if ( tmp ) {
            Perl_re_printf( aTHX_  "%*s",
                colwidth,
                pv_pretty(sv, SvPV_nolen_const(*tmp), SvCUR(*tmp), colwidth,
                            PL_colors[0], PL_colors[1],
                            (SvUTF8(*tmp) ? PERL_PV_ESCAPE_UNI : 0) |
                            PERL_PV_ESCAPE_FIRSTCHAR
                )
            );
        }
    }

    Perl_re_printf( aTHX_ "\n");
    Perl_re_indentf( aTHX_  "State+-", depth+1 );

    for( charid=0 ; charid < trie->uniquecharcount ; charid++ ) {
        Perl_re_printf( aTHX_  "%.*s", colwidth,"--------");
    }

    Perl_re_printf( aTHX_  "\n" );

    for( state=1 ; state < next_alloc ; state += trie->uniquecharcount ) {

        Perl_re_indentf( aTHX_  "%4" UVXf " : ",
            depth+1,
            (UV)TRIE_NODENUM( state ) );

        for( charid = 0 ; charid < trie->uniquecharcount ; charid++ ) {
            UV v=(UV)SAFE_TRIE_NODENUM( trie->trans[ state + charid ].next );
            if (v)
                Perl_re_printf( aTHX_  "%*" UVXf, colwidth, v );
            else
                Perl_re_printf( aTHX_  "%*s", colwidth, "." );
        }
        if ( ! trie->states[ TRIE_NODENUM( state ) ].wordnum ) {
            Perl_re_printf( aTHX_  " (%4" UVXf ")\n",
                                            (UV)trie->trans[ state ].check );
        } else {
            Perl_re_printf( aTHX_  " (%4" UVXf ") W%4X\n",
                                            (UV)trie->trans[ state ].check,
            trie->states[ TRIE_NODENUM( state ) ].wordnum );
        }
    }
}

#endif


/* make_trie(startbranch,first,last,tail,word_count,flags,depth)
  startbranch: the first branch in the whole branch sequence
  first      : start branch of sequence of branch-exact nodes.
               May be the same as startbranch
  last       : Thing following the last branch.
               May be the same as tail.
  tail       : item following the branch sequence
  count      : words in the sequence
  flags      : currently the OP() type we will be building one of /EXACT(|F|FA|FU|FU_SS|L|FLU8)/
  depth      : indent depth

Inplace optimizes a sequence of 2 or more Branch-Exact nodes into a TRIE node.

A trie is an N'ary tree where the branches are determined by digital
decomposition of the key. IE, at the root node you look up the 1st character and
follow that branch repeat until you find the end of the branches. Nodes can be
marked as "accepting" meaning they represent a complete word. Eg:

  /he|she|his|hers/

would convert into the following structure. Numbers represent states, letters
following numbers represent valid transitions on the letter from that state, if
the number is in square brackets it represents an accepting state, otherwise it
will be in parenthesis.

      +-h->+-e->[3]-+-r->(8)-+-s->[9]
      |    |
      |   (2)
      |    |
     (1)   +-i->(6)-+-s->[7]
      |
      +-s->(3)-+-h->(4)-+-e->[5]

      Accept Word Mapping: 3=>1 (he),5=>2 (she), 7=>3 (his), 9=>4 (hers)

This shows that when matching against the string 'hers' we will begin at state 1
read 'h' and move to state 2, read 'e' and move to state 3 which is accepting,
then read 'r' and go to state 8 followed by 's' which takes us to state 9 which
is also accepting. Thus we know that we can match both 'he' and 'hers' with a
single traverse. We store a mapping from accepting to state to which word was
matched, and then when we have multiple possibilities we try to complete the
rest of the regex in the order in which they occurred in the alternation.

The only prior NFA like behaviour that would be changed by the TRIE support is
the silent ignoring of duplicate alternations which are of the form:

 / (DUPE|DUPE) X? (?{ ... }) Y /x

Thus EVAL blocks following a trie may be called a different number of times with
and without the optimisation. With the optimisations dupes will be silently
ignored. This inconsistent behaviour of EVAL type nodes is well established as
the following demonstrates:

 'words'=~/(word|word|word)(?{ print $1 })[xyz]/

which prints out 'word' three times, but

 'words'=~/(word|word|word)(?{ print $1 })S/

which doesnt print it out at all. This is due to other optimisations kicking in.

Example of what happens on a structural level:

The regexp /(ac|ad|ab)+/ will produce the following debug output:

   1: CURLYM[1] {1,32767}(18)
   5:   BRANCH(8)
   6:     EXACT <ac>(16)
   8:   BRANCH(11)
   9:     EXACT <ad>(16)
  11:   BRANCH(14)
  12:     EXACT <ab>(16)
  16:   SUCCEED(0)
  17:   NOTHING(18)
  18: END(0)

This would be optimizable with startbranch=5, first=5, last=16, tail=16
and should turn into:

   1: CURLYM[1] {1,32767}(18)
   5:   TRIE(16)
        [Words:3 Chars Stored:6 Unique Chars:4 States:5 NCP:1]
          <ac>
          <ad>
          <ab>
  16:   SUCCEED(0)
  17:   NOTHING(18)
  18: END(0)

Cases where tail != last would be like /(?foo|bar)baz/:

   1: BRANCH(4)
   2:   EXACT <foo>(8)
   4: BRANCH(7)
   5:   EXACT <bar>(8)
   7: TAIL(8)
   8: EXACT <baz>(10)
  10: END(0)

which would be optimizable with startbranch=1, first=1, last=7, tail=8
and would end up looking like:

    1: TRIE(8)
      [Words:2 Chars Stored:6 Unique Chars:5 States:7 NCP:1]
        <foo>
        <bar>
   7: TAIL(8)
   8: EXACT <baz>(10)
  10: END(0)

    d = uvchr_to_utf8_flags(d, uv, 0);

is the recommended Unicode-aware way of saying

    *(d++) = uv;
*/

#define TRIE_STORE_REVCHAR(val)                                            \
    STMT_START {                                                           \
        if (UTF) {                                                         \
            SV *zlopp = newSV(UTF8_MAXBYTES);                              \
            unsigned char *flrbbbbb = (unsigned char *) SvPVX(zlopp);      \
            unsigned char *const kapow = uvchr_to_utf8(flrbbbbb, val);     \
            *kapow = '\0';                                                 \
            SvCUR_set(zlopp, kapow - flrbbbbb);                            \
            SvPOK_on(zlopp);                                               \
            SvUTF8_on(zlopp);                                              \
            av_push_simple(revcharmap, zlopp);                                     \
        } else {                                                           \
            char ooooff = (char)val;                                           \
            av_push_simple(revcharmap, newSVpvn(&ooooff, 1));                      \
        }                                                                  \
        } STMT_END

/* This gets the next character from the input, folding it if not already
 * folded. */
#define TRIE_READ_CHAR STMT_START {                                           \
    wordlen++;                                                                \
    if ( UTF ) {                                                              \
        /* if it is UTF then it is either already folded, or does not need    \
         * folding */                                                         \
        uvc = valid_utf8_to_uvchr( (const U8*) uc, &len);                     \
    }                                                                         \
    else if (folder == PL_fold_latin1) {                                      \
        /* This folder implies Unicode rules, which in the range expressible  \
         *  by not UTF is the lower case, with the two exceptions, one of     \
         *  which should have been taken care of before calling this */       \
        assert(*uc != LATIN_SMALL_LETTER_SHARP_S);                            \
        uvc = toLOWER_L1(*uc);                                                \
        if (UNLIKELY(uvc == MICRO_SIGN)) uvc = GREEK_SMALL_LETTER_MU;         \
        len = 1;                                                              \
    } else {                                                                  \
        /* raw data, will be folded later if needed */                        \
        uvc = (U32)*uc;                                                       \
        len = 1;                                                              \
    }                                                                         \
} STMT_END



#define TRIE_LIST_PUSH(state,fid,ns) STMT_START {               \
    if ( TRIE_LIST_CUR( state ) >=TRIE_LIST_LEN( state ) ) {    \
        U32 ging = TRIE_LIST_LEN( state ) * 2;                  \
        Renew( trie->states[ state ].trans.list, ging, reg_trie_trans_le ); \
        TRIE_LIST_LEN( state ) = ging;                          \
    }                                                           \
    TRIE_LIST_ITEM( state, TRIE_LIST_CUR( state ) ).forid = fid;     \
    TRIE_LIST_ITEM( state, TRIE_LIST_CUR( state ) ).newstate = ns;   \
    TRIE_LIST_CUR( state )++;                                   \
} STMT_END

#define TRIE_LIST_NEW(state) STMT_START {                       \
    Newx( trie->states[ state ].trans.list,                     \
        4, reg_trie_trans_le );                                 \
     TRIE_LIST_CUR( state ) = 1;                                \
     TRIE_LIST_LEN( state ) = 4;                                \
} STMT_END

#define TRIE_HANDLE_WORD(state) STMT_START {                    \
    U16 dupe= trie->states[ state ].wordnum;                    \
    regnode * const noper_next = regnext( noper );              \
                                                                \
    DEBUG_r({                                                   \
        /* store the word for dumping */                        \
        SV* tmp;                                                \
        if (OP(noper) != NOTHING)                               \
            tmp = newSVpvn_utf8(STRING(noper), STR_LEN(noper), UTF);    \
        else                                                    \
            tmp = newSVpvn_utf8( "", 0, UTF );                  \
        av_push_simple( trie_words, tmp );                             \
    });                                                         \
                                                                \
    curword++;                                                  \
    trie->wordinfo[curword].prev   = 0;                         \
    trie->wordinfo[curword].len    = wordlen;                   \
    trie->wordinfo[curword].accept = state;                     \
                                                                \
    if ( noper_next < tail ) {                                  \
        if (!trie->jump) {                                      \
            trie->jump = (U16 *) PerlMemShared_calloc( word_count + 1, \
                                                 sizeof(U16) ); \
            trie->j_before_paren = (U16 *) PerlMemShared_calloc( word_count + 1, \
                                                 sizeof(U16) ); \
            trie->j_after_paren = (U16 *) PerlMemShared_calloc( word_count + 1, \
                                                 sizeof(U16) ); \
        }                                                       \
        trie->jump[curword] = (U16)(noper_next - convert);      \
        U16 set_before_paren;                                   \
        U16 set_after_paren;                                    \
        if (OP(cur) == BRANCH) {                                \
            set_before_paren = ARG1a(cur);                       \
            set_after_paren = ARG1b(cur);                        \
        } else {                                                \
            set_before_paren = ARG2a(cur);                     \
            set_after_paren = ARG2b(cur);                      \
        }                                                       \
        trie->j_before_paren[curword] = set_before_paren;       \
        trie->j_after_paren[curword] = set_after_paren;         \
        if (!jumper)                                            \
            jumper = noper_next;                                \
        if (!nextbranch)                                        \
            nextbranch= regnext(cur);                           \
    }                                                           \
                                                                \
    if ( dupe ) {                                               \
        /* It's a dupe. Pre-insert into the wordinfo[].prev   */\
        /* chain, so that when the bits of chain are later    */\
        /* linked together, the dups appear in the chain      */\
        trie->wordinfo[curword].prev = trie->wordinfo[dupe].prev; \
        trie->wordinfo[dupe].prev = curword;                    \
    } else {                                                    \
        /* we haven't inserted this word yet.                */ \
        trie->states[ state ].wordnum = curword;                \
    }                                                           \
} STMT_END


#define TRIE_TRANS_STATE(state,base,ucharcount,charid,special)          \
     ( ( base + charid >=  ucharcount                                   \
         && base + charid < ubound                                      \
         && state == trie->trans[ base - ucharcount + charid ].check    \
         && trie->trans[ base - ucharcount + charid ].next )            \
           ? trie->trans[ base - ucharcount + charid ].next             \
           : ( state==1 ? special : 0 )                                 \
      )

#define TRIE_BITMAP_SET_FOLDED(trie, uvc, folder)           \
STMT_START {                                                \
    TRIE_BITMAP_SET(trie, uvc);                             \
    /* store the folded codepoint */                        \
    if ( folder )                                           \
        TRIE_BITMAP_SET(trie, folder[(U8) uvc ]);           \
                                                            \
    if ( !UTF ) {                                           \
        /* store first byte of utf8 representation of */    \
        /* variant codepoints */                            \
        if (! UVCHR_IS_INVARIANT(uvc)) {                    \
            TRIE_BITMAP_SET(trie, UTF8_TWO_BYTE_HI(uvc));   \
        }                                                   \
    }                                                       \
} STMT_END

I32
Perl_make_trie(pTHX_ RExC_state_t *pRExC_state, regnode *startbranch,
                  regnode *first, regnode *last, regnode *tail,
                  U32 word_count, U32 flags, U32 depth)
{
    /* first pass, loop through and scan words */
    reg_trie_data *trie;
    HV *widecharmap = NULL;
    AV *revcharmap = newAV();
    regnode *cur;
    STRLEN len = 0;
    UV uvc = 0;
    U16 curword = 0;
    U32 next_alloc = 0;
    regnode *jumper = NULL;
    regnode *nextbranch = NULL;
    regnode *lastbranch = NULL;
    regnode *convert = NULL;
    U32 *prev_states; /* temp array mapping each state to previous one */
    /* we just use folder as a flag in utf8 */
    const U8 * folder = NULL;

    /* in the below reg_add_data call we are storing either 'tu' or 'tuaa'
     * which stands for one trie structure, one hash, optionally followed
     * by two arrays */
#ifdef DEBUGGING
    const U32 data_slot = reg_add_data( pRExC_state, STR_WITH_LEN("tuaa"));
    AV *trie_words = NULL;
    /* along with revcharmap, this only used during construction but both are
     * useful during debugging so we store them in the struct when debugging.
     */
#else
    const U32 data_slot = reg_add_data( pRExC_state, STR_WITH_LEN("tu"));
    STRLEN trie_charcount=0;
#endif
    SV *re_trie_maxbuff;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_MAKE_TRIE;
#ifndef DEBUGGING
    PERL_UNUSED_ARG(depth);
#endif

    switch (flags) {
        case EXACT: case EXACT_REQ8: case EXACTL: break;
        case EXACTFAA:
        case EXACTFUP:
        case EXACTFU:
        case EXACTFLU8: folder = PL_fold_latin1; break;
        case EXACTF:  folder = PL_fold; break;
        default: Perl_croak( aTHX_ "panic! In trie construction, unknown node type %u %s", (unsigned) flags, REGNODE_NAME(flags) );
    }

    /* create the trie struct, all zeroed */
    trie = (reg_trie_data *) PerlMemShared_calloc( 1, sizeof(reg_trie_data) );
    trie->refcount = 1;
    trie->startstate = 1;
    trie->wordcount = word_count;
    RExC_rxi->data->data[ data_slot ] = (void*)trie;
    trie->charmap = (U16 *) PerlMemShared_calloc( 256, sizeof(U16) );
    if (flags == EXACT || flags == EXACT_REQ8 || flags == EXACTL)
        trie->bitmap = (char *) PerlMemShared_calloc( ANYOF_BITMAP_SIZE, 1 );
    trie->wordinfo = (reg_trie_wordinfo *) PerlMemShared_calloc(
                       trie->wordcount+1, sizeof(reg_trie_wordinfo));

    DEBUG_r({
        trie_words = newAV();
    });

    re_trie_maxbuff = get_sv(RE_TRIE_MAXBUF_NAME, GV_ADD);
    assert(re_trie_maxbuff);
    if (!SvIOK(re_trie_maxbuff)) {
        sv_setiv(re_trie_maxbuff, RE_TRIE_MAXBUF_INIT);
    }
    DEBUG_TRIE_COMPILE_r({
        Perl_re_indentf( aTHX_
          "make_trie start==%d, first==%d, last==%d, tail==%d depth=%d\n",
          depth+1,
          REG_NODE_NUM(startbranch), REG_NODE_NUM(first),
          REG_NODE_NUM(last), REG_NODE_NUM(tail), (int)depth);
    });

   /* Find the node we are going to overwrite */
    if ( first == startbranch && OP( last ) != BRANCH ) {
        /* whole branch chain */
        convert = first;
    } else {
        /* branch sub-chain */
        convert = REGNODE_AFTER( first );
    }

    /*  -- First loop and Setup --

       We first traverse the branches and scan each word to determine if it
       contains widechars, and how many unique chars there are, this is
       important as we have to build a table with at least as many columns as we
       have unique chars.

       We use an array of integers to represent the character codes 0..255
       (trie->charmap) and we use a an HV* to store Unicode characters. We use
       the native representation of the character value as the key and IV's for
       the coded index.

       *TODO* If we keep track of how many times each character is used we can
       remap the columns so that the table compression later on is more
       efficient in terms of memory by ensuring the most common value is in the
       middle and the least common are on the outside.  IMO this would be better
       than a most to least common mapping as theres a decent chance the most
       common letter will share a node with the least common, meaning the node
       will not be compressible. With a middle is most common approach the worst
       case is when we have the least common nodes twice.

     */

    for ( cur = first ; cur < last ; cur = regnext( cur ) ) {
        regnode *noper = REGNODE_AFTER( cur );
        const U8 *uc;
        const U8 *e;
        int foldlen = 0;
        U32 wordlen      = 0;         /* required init */
        STRLEN minchars = 0;
        STRLEN maxchars = 0;
        bool set_bit = trie->bitmap ? 1 : 0; /*store the first char in the
                                               bitmap?*/
        lastbranch = cur;

        if (OP(noper) == NOTHING) {
            /* skip past a NOTHING at the start of an alternation
             * eg, /(?:)a|(?:b)/ should be the same as /a|b/
             *
             * If the next node is not something we are supposed to process
             * we will just ignore it due to the condition guarding the
             * next block.
             */

            regnode *noper_next= regnext(noper);
            if (noper_next < tail)
                noper= noper_next;
        }

        if (    noper < tail
            && (    OP(noper) == flags
                || (flags == EXACT && OP(noper) == EXACT_REQ8)
                || (flags == EXACTFU && (   OP(noper) == EXACTFU_REQ8
                                         || OP(noper) == EXACTFUP))))
        {
            uc= (U8*)STRING(noper);
            e= uc + STR_LEN(noper);
        } else {
            trie->minlen= 0;
            continue;
        }


        if ( set_bit ) { /* bitmap only alloced when !(UTF&&Folding) */
            TRIE_BITMAP_SET(trie,*uc); /* store the raw first byte
                                          regardless of encoding */
            if (OP( noper ) == EXACTFUP) {
                /* false positives are ok, so just set this */
                TRIE_BITMAP_SET(trie, LATIN_SMALL_LETTER_SHARP_S);
            }
        }

        for ( ; uc < e ; uc += len ) {  /* Look at each char in the current
                                           branch */
            TRIE_CHARCOUNT(trie)++;
            TRIE_READ_CHAR;

            /* TRIE_READ_CHAR returns the current character, or its fold if /i
             * is in effect.  Under /i, this character can match itself, or
             * anything that folds to it.  If not under /i, it can match just
             * itself.  Most folds are 1-1, for example k, K, and KELVIN SIGN
             * all fold to k, and all are single characters.   But some folds
             * expand to more than one character, so for example LATIN SMALL
             * LIGATURE FFI folds to the three character sequence 'ffi'.  If
             * the string beginning at 'uc' is 'ffi', it could be matched by
             * three characters, or just by the one ligature character. (It
             * could also be matched by two characters: LATIN SMALL LIGATURE FF
             * followed by 'i', or by 'f' followed by LATIN SMALL LIGATURE FI).
             * (Of course 'I' and/or 'F' instead of 'i' and 'f' can also
             * match.)  The trie needs to know the minimum and maximum number
             * of characters that could match so that it can use size alone to
             * quickly reject many match attempts.  The max is simple: it is
             * the number of folded characters in this branch (since a fold is
             * never shorter than what folds to it. */

            maxchars++;

            /* And the min is equal to the max if not under /i (indicated by
             * 'folder' being NULL), or there are no multi-character folds.  If
             * there is a multi-character fold, the min is incremented just
             * once, for the character that folds to the sequence.  Each
             * character in the sequence needs to be added to the list below of
             * characters in the trie, but we count only the first towards the
             * min number of characters needed.  This is done through the
             * variable 'foldlen', which is returned by the macros that look
             * for these sequences as the number of bytes the sequence
             * occupies.  Each time through the loop, we decrement 'foldlen' by
             * how many bytes the current char occupies.  Only when it reaches
             * 0 do we increment 'minchars' or look for another multi-character
             * sequence. */
            if (folder == NULL) {
                minchars++;
            }
            else if (foldlen > 0) {
                foldlen -= (UTF) ? UTF8SKIP(uc) : 1;
            }
            else {
                minchars++;

                /* See if *uc is the beginning of a multi-character fold.  If
                 * so, we decrement the length remaining to look at, to account
                 * for the current character this iteration.  (We can use 'uc'
                 * instead of the fold returned by TRIE_READ_CHAR because the
                 * macro is smart enough to account for any unfolded
                 * characters. */
                if (UTF) {
                    if ((foldlen = is_MULTI_CHAR_FOLD_utf8_safe(uc, e))) {
                        foldlen -= UTF8SKIP(uc);
                    }
                }
                else if ((foldlen = is_MULTI_CHAR_FOLD_latin1_safe(uc, e))) {
                    foldlen--;
                }
            }

            /* The current character (and any potential folds) should be added
             * to the possible matching characters for this position in this
             * branch */
            if ( uvc < 256 ) {
                if ( folder ) {
                    U8 folded= folder[ (U8) uvc ];
                    if ( !trie->charmap[ folded ] ) {
                        trie->charmap[ folded ]=( ++trie->uniquecharcount );
                        TRIE_STORE_REVCHAR( folded );
                    }
                }
                if ( !trie->charmap[ uvc ] ) {
                    trie->charmap[ uvc ]=( ++trie->uniquecharcount );
                    TRIE_STORE_REVCHAR( uvc );
                }
                if ( set_bit ) {
                    /* store the codepoint in the bitmap, and its folded
                     * equivalent. */
                    TRIE_BITMAP_SET_FOLDED(trie, uvc, folder);
                    set_bit = 0; /* We've done our bit :-) */
                }
            } else {

                /* XXX We could come up with the list of code points that fold
                 * to this using PL_utf8_foldclosures, except not for
                 * multi-char folds, as there may be multiple combinations
                 * there that could work, which needs to wait until runtime to
                 * resolve (The comment about LIGATURE FFI above is such an
                 * example */

                SV** svpp;
                if ( !widecharmap )
                    widecharmap = newHV();

                svpp = hv_fetch( widecharmap, (char*)&uvc, sizeof( UV ), 1 );

                if ( !svpp )
                    Perl_croak( aTHX_ "error creating/fetching widecharmap entry for 0x%" UVXf, uvc );

                if ( !SvTRUE( *svpp ) ) {
                    sv_setiv( *svpp, ++trie->uniquecharcount );
                    TRIE_STORE_REVCHAR(uvc);
                }
            }
        } /* end loop through characters in this branch of the trie */

        /* We take the min and max for this branch and combine to find the min
         * and max for all branches processed so far */
        if( cur == first ) {
            trie->minlen = minchars;
            trie->maxlen = maxchars;
        } else if (minchars < trie->minlen) {
            trie->minlen = minchars;
        } else if (maxchars > trie->maxlen) {
            trie->maxlen = maxchars;
        }
    } /* end first pass */
    trie->before_paren = OP(first) == BRANCH
                 ? ARG1a(first)
                 : ARG2a(first); /* BRANCHJ */

    trie->after_paren = OP(lastbranch) == BRANCH
                 ? ARG1b(lastbranch)
                 : ARG2b(lastbranch); /* BRANCHJ */
    DEBUG_TRIE_COMPILE_r(
        Perl_re_indentf( aTHX_
                "TRIE(%s): W:%d C:%d Uq:%d Min:%d Max:%d\n",
                depth+1,
                ( widecharmap ? "UTF8" : "NATIVE" ), (int)word_count,
                (int)TRIE_CHARCOUNT(trie), trie->uniquecharcount,
                (int)trie->minlen, (int)trie->maxlen )
    );

    /*
        We now know what we are dealing with in terms of unique chars and
        string sizes so we can calculate how much memory a naive
        representation using a flat table  will take. If it's over a reasonable
        limit (as specified by ${^RE_TRIE_MAXBUF}) we use a more memory
        conservative but potentially much slower representation using an array
        of lists.

        At the end we convert both representations into the same compressed
        form that will be used in regexec.c for matching with. The latter
        is a form that cannot be used to construct with but has memory
        properties similar to the list form and access properties similar
        to the table form making it both suitable for fast searches and
        small enough that its feasable to store for the duration of a program.

        See the comment in the code where the compressed table is produced
        inplace from the flat tabe representation for an explanation of how
        the compression works.

    */


    Newx(prev_states, TRIE_CHARCOUNT(trie) + 2, U32);
    prev_states[1] = 0;

    if ( (IV)( ( TRIE_CHARCOUNT(trie) + 1 ) * trie->uniquecharcount + 1)
                                                    > SvIV(re_trie_maxbuff) )
    {
        /*
            Second Pass -- Array Of Lists Representation

            Each state will be represented by a list of charid:state records
            (reg_trie_trans_le) the first such element holds the CUR and LEN
            points of the allocated array. (See defines above).

            We build the initial structure using the lists, and then convert
            it into the compressed table form which allows faster lookups
            (but cant be modified once converted).
        */

        STRLEN transcount = 1;

        DEBUG_TRIE_COMPILE_MORE_r( Perl_re_indentf( aTHX_  "Compiling trie using list compiler\n",
            depth+1));

        trie->states = (reg_trie_state *)
            PerlMemShared_calloc( TRIE_CHARCOUNT(trie) + 2,
                                  sizeof(reg_trie_state) );
        TRIE_LIST_NEW(1);
        next_alloc = 2;

        for ( cur = first ; cur < last ; cur = regnext( cur ) ) {

            regnode *noper   = REGNODE_AFTER( cur );
            U32 state        = 1;         /* required init */
            U16 charid       = 0;         /* sanity init */
            U32 wordlen      = 0;         /* required init */

            if (OP(noper) == NOTHING) {
                regnode *noper_next= regnext(noper);
                if (noper_next < tail)
                    noper= noper_next;
                /* we will undo this assignment if noper does not
                 * point at a trieable type in the else clause of
                 * the following statement. */
            }

            if (    noper < tail
                && (    OP(noper) == flags
                    || (flags == EXACT && OP(noper) == EXACT_REQ8)
                    || (flags == EXACTFU && (   OP(noper) == EXACTFU_REQ8
                                             || OP(noper) == EXACTFUP))))
            {
                const U8 *uc= (U8*)STRING(noper);
                const U8 *e= uc + STR_LEN(noper);

                for ( ; uc < e ; uc += len ) {

                    TRIE_READ_CHAR;

                    if ( uvc < 256 ) {
                        charid = trie->charmap[ uvc ];
                    } else {
                        SV** const svpp = hv_fetch( widecharmap,
                                                    (char*)&uvc,
                                                    sizeof( UV ),
                                                    0);
                        if ( !svpp ) {
                            charid = 0;
                        } else {
                            charid=(U16)SvIV( *svpp );
                        }
                    }
                    /* charid is now 0 if we dont know the char read, or
                     * nonzero if we do */
                    if ( charid ) {

                        U16 check;
                        U32 newstate = 0;

                        charid--;
                        if ( !trie->states[ state ].trans.list ) {
                            TRIE_LIST_NEW( state );
                        }
                        for ( check = 1;
                              check <= TRIE_LIST_USED( state );
                              check++ )
                        {
                            if ( TRIE_LIST_ITEM( state, check ).forid
                                                                    == charid )
                            {
                                newstate = TRIE_LIST_ITEM( state, check ).newstate;
                                break;
                            }
                        }
                        if ( ! newstate ) {
                            newstate = next_alloc++;
                            prev_states[newstate] = state;
                            TRIE_LIST_PUSH( state, charid, newstate );
                            transcount++;
                        }
                        state = newstate;
                    } else {
                        Perl_croak( aTHX_ "panic! In trie construction, no char mapping for %" IVdf, uvc );
                    }
                }
            } else {
                /* If we end up here it is because we skipped past a NOTHING, but did not end up
                 * on a trieable type. So we need to reset noper back to point at the first regop
                 * in the branch before we call TRIE_HANDLE_WORD()
                */
                noper= REGNODE_AFTER(cur);
            }
            TRIE_HANDLE_WORD(state);

        } /* end second pass */

        /* next alloc is the NEXT state to be allocated */
        trie->statecount = next_alloc;
        trie->states = (reg_trie_state *)
            PerlMemShared_realloc( trie->states,
                                   next_alloc
                                   * sizeof(reg_trie_state) );

        /* and now dump it out before we compress it */
        DEBUG_TRIE_COMPILE_MORE_r(dump_trie_interim_list(trie, widecharmap,
                                                         revcharmap, next_alloc,
                                                         depth+1)
        );

        trie->trans = (reg_trie_trans *)
            PerlMemShared_calloc( transcount, sizeof(reg_trie_trans) );
        {
            U32 state;
            U32 tp = 0;
            U32 zp = 0;


            for( state=1 ; state < next_alloc ; state ++ ) {
                U32 base=0;

                /*
                DEBUG_TRIE_COMPILE_MORE_r(
                    Perl_re_printf( aTHX_  "tp: %d zp: %d ",tp,zp)
                );
                */

                if (trie->states[state].trans.list) {
                    U16 minid=TRIE_LIST_ITEM( state, 1).forid;
                    U16 maxid=minid;
                    U16 idx;

                    for( idx = 2 ; idx <= TRIE_LIST_USED( state ) ; idx++ ) {
                        const U16 forid = TRIE_LIST_ITEM( state, idx).forid;
                        if ( forid < minid ) {
                            minid=forid;
                        } else if ( forid > maxid ) {
                            maxid=forid;
                        }
                    }
                    if ( transcount < tp + maxid - minid + 1) {
                        transcount *= 2;
                        trie->trans = (reg_trie_trans *)
                            PerlMemShared_realloc( trie->trans,
                                                     transcount
                                                     * sizeof(reg_trie_trans) );
                        Zero( trie->trans + (transcount / 2),
                              transcount / 2,
                              reg_trie_trans );
                    }
                    base = trie->uniquecharcount + tp - minid;
                    if ( maxid == minid ) {
                        U32 set = 0;
                        for ( ; zp < tp ; zp++ ) {
                            if ( ! trie->trans[ zp ].next ) {
                                base = trie->uniquecharcount + zp - minid;
                                trie->trans[ zp ].next = TRIE_LIST_ITEM( state,
                                                                   1).newstate;
                                trie->trans[ zp ].check = state;
                                set = 1;
                                break;
                            }
                        }
                        if ( !set ) {
                            trie->trans[ tp ].next = TRIE_LIST_ITEM( state,
                                                                   1).newstate;
                            trie->trans[ tp ].check = state;
                            tp++;
                            zp = tp;
                        }
                    } else {
                        for ( idx=1; idx <= TRIE_LIST_USED( state ) ; idx++ ) {
                            const U32 tid = base
                                           - trie->uniquecharcount
                                           + TRIE_LIST_ITEM( state, idx ).forid;
                            trie->trans[ tid ].next = TRIE_LIST_ITEM( state,
                                                                idx ).newstate;
                            trie->trans[ tid ].check = state;
                        }
                        tp += ( maxid - minid + 1 );
                    }
                    Safefree(trie->states[ state ].trans.list);
                }
                /*
                DEBUG_TRIE_COMPILE_MORE_r(
                    Perl_re_printf( aTHX_  " base: %d\n",base);
                );
                */
                trie->states[ state ].trans.base=base;
            }
            trie->lasttrans = tp + 1;
        }
    } else {
        /*
           Second Pass -- Flat Table Representation.

           we dont use the 0 slot of either trans[] or states[] so we add 1 to
           each.  We know that we will need Charcount+1 trans at most to store
           the data (one row per char at worst case) So we preallocate both
           structures assuming worst case.

           We then construct the trie using only the .next slots of the entry
           structs.

           We use the .check field of the first entry of the node temporarily
           to make compression both faster and easier by keeping track of how
           many non zero fields are in the node.

           Since trans are numbered from 1 any 0 pointer in the table is a FAIL
           transition.

           There are two terms at use here: state as a TRIE_NODEIDX() which is
           a number representing the first entry of the node, and state as a
           TRIE_NODENUM() which is the trans number. state 1 is TRIE_NODEIDX(1)
           and TRIE_NODENUM(1), state 2 is TRIE_NODEIDX(2) and TRIE_NODENUM(3)
           if there are 2 entrys per node. eg:

             A B       A B
          1. 2 4    1. 3 7
          2. 0 3    3. 0 5
          3. 0 0    5. 0 0
          4. 0 0    7. 0 0

           The table is internally in the right hand, idx form. However as we
           also have to deal with the states array which is indexed by nodenum
           we have to use TRIE_NODENUM() to convert.

        */
        DEBUG_TRIE_COMPILE_MORE_r( Perl_re_indentf( aTHX_  "Compiling trie using table compiler\n",
            depth+1));

        trie->trans = (reg_trie_trans *)
            PerlMemShared_calloc( ( TRIE_CHARCOUNT(trie) + 1 )
                                  * trie->uniquecharcount + 1,
                                  sizeof(reg_trie_trans) );
        trie->states = (reg_trie_state *)
            PerlMemShared_calloc( TRIE_CHARCOUNT(trie) + 2,
                                  sizeof(reg_trie_state) );
        next_alloc = trie->uniquecharcount + 1;


        for ( cur = first ; cur < last ; cur = regnext( cur ) ) {

            regnode *noper   = REGNODE_AFTER( cur );

            U32 state        = 1;         /* required init */

            U16 charid       = 0;         /* sanity init */
            U32 accept_state = 0;         /* sanity init */

            U32 wordlen      = 0;         /* required init */

            if (OP(noper) == NOTHING) {
                regnode *noper_next= regnext(noper);
                if (noper_next < tail)
                    noper= noper_next;
                /* we will undo this assignment if noper does not
                 * point at a trieable type in the else clause of
                 * the following statement. */
            }

            if (    noper < tail
                && (    OP(noper) == flags
                    || (flags == EXACT && OP(noper) == EXACT_REQ8)
                    || (flags == EXACTFU && (   OP(noper) == EXACTFU_REQ8
                                             || OP(noper) == EXACTFUP))))
            {
                const U8 *uc= (U8*)STRING(noper);
                const U8 *e= uc + STR_LEN(noper);

                for ( ; uc < e ; uc += len ) {

                    TRIE_READ_CHAR;

                    if ( uvc < 256 ) {
                        charid = trie->charmap[ uvc ];
                    } else {
                        SV* const * const svpp = hv_fetch( widecharmap,
                                                           (char*)&uvc,
                                                           sizeof( UV ),
                                                           0);
                        charid = svpp ? (U16)SvIV(*svpp) : 0;
                    }
                    if ( charid ) {
                        charid--;
                        if ( !trie->trans[ state + charid ].next ) {
                            trie->trans[ state + charid ].next = next_alloc;
                            trie->trans[ state ].check++;
                            prev_states[TRIE_NODENUM(next_alloc)]
                                    = TRIE_NODENUM(state);
                            next_alloc += trie->uniquecharcount;
                        }
                        state = trie->trans[ state + charid ].next;
                    } else {
                        Perl_croak( aTHX_ "panic! In trie construction, no char mapping for %" IVdf, uvc );
                    }
                    /* charid is now 0 if we dont know the char read, or
                     * nonzero if we do */
                }
            } else {
                /* If we end up here it is because we skipped past a NOTHING, but did not end up
                 * on a trieable type. So we need to reset noper back to point at the first regop
                 * in the branch before we call TRIE_HANDLE_WORD().
                */
                noper= REGNODE_AFTER(cur);
            }
            accept_state = TRIE_NODENUM( state );
            TRIE_HANDLE_WORD(accept_state);

        } /* end second pass */

        /* and now dump it out before we compress it */
        DEBUG_TRIE_COMPILE_MORE_r(dump_trie_interim_table(trie, widecharmap,
                                                          revcharmap,
                                                          next_alloc, depth+1));

        {
        /*
           * Inplace compress the table.*

           For sparse data sets the table constructed by the trie algorithm will
           be mostly 0/FAIL transitions or to put it another way mostly empty.
           (Note that leaf nodes will not contain any transitions.)

           This algorithm compresses the tables by eliminating most such
           transitions, at the cost of a modest bit of extra work during lookup:

           - Each states[] entry contains a .base field which indicates the
           index in the state[] array wheres its transition data is stored.

           - If .base is 0 there are no valid transitions from that node.

           - If .base is nonzero then charid is added to it to find an entry in
           the trans array.

           -If trans[states[state].base+charid].check!=state then the
           transition is taken to be a 0/Fail transition. Thus if there are fail
           transitions at the front of the node then the .base offset will point
           somewhere inside the previous nodes data (or maybe even into a node
           even earlier), but the .check field determines if the transition is
           valid.

           XXX - wrong maybe?
           The following process inplace converts the table to the compressed
           table: We first do not compress the root node 1,and mark all its
           .check pointers as 1 and set its .base pointer as 1 as well. This
           allows us to do a DFA construction from the compressed table later,
           and ensures that any .base pointers we calculate later are greater
           than 0.

           - We set 'pos' to indicate the first entry of the second node.

           - We then iterate over the columns of the node, finding the first and
           last used entry at l and m. We then copy l..m into pos..(pos+m-l),
           and set the .check pointers accordingly, and advance pos
           appropriately and repreat for the next node. Note that when we copy
           the next pointers we have to convert them from the original
           NODEIDX form to NODENUM form as the former is not valid post
           compression.

           - If a node has no transitions used we mark its base as 0 and do not
           advance the pos pointer.

           - If a node only has one transition we use a second pointer into the
           structure to fill in allocated fail transitions from other states.
           This pointer is independent of the main pointer and scans forward
           looking for null transitions that are allocated to a state. When it
           finds one it writes the single transition into the "hole".  If the
           pointer doesnt find one the single transition is appended as normal.

           - Once compressed we can Renew/realloc the structures to release the
           excess space.

           See "Table-Compression Methods" in sec 3.9 of the Red Dragon,
           specifically Fig 3.47 and the associated pseudocode.

           demq
        */
        const U32 laststate = TRIE_NODENUM( next_alloc );
        U32 state, charid;
        U32 pos = 0, zp=0;
        trie->statecount = laststate;

        for ( state = 1 ; state < laststate ; state++ ) {
            U8 flag = 0;
            const U32 stateidx = TRIE_NODEIDX( state );
            const U32 o_used = trie->trans[ stateidx ].check;
            U32 used = trie->trans[ stateidx ].check;
            trie->trans[ stateidx ].check = 0;

            for ( charid = 0;
                  used && charid < trie->uniquecharcount;
                  charid++ )
            {
                if ( flag || trie->trans[ stateidx + charid ].next ) {
                    if ( trie->trans[ stateidx + charid ].next ) {
                        if (o_used == 1) {
                            for ( ; zp < pos ; zp++ ) {
                                if ( ! trie->trans[ zp ].next ) {
                                    break;
                                }
                            }
                            trie->states[ state ].trans.base
                                                    = zp
                                                      + trie->uniquecharcount
                                                      - charid ;
                            trie->trans[ zp ].next
                                = SAFE_TRIE_NODENUM( trie->trans[ stateidx
                                                             + charid ].next );
                            trie->trans[ zp ].check = state;
                            if ( ++zp > pos ) pos = zp;
                            break;
                        }
                        used--;
                    }
                    if ( !flag ) {
                        flag = 1;
                        trie->states[ state ].trans.base
                                       = pos + trie->uniquecharcount - charid ;
                    }
                    trie->trans[ pos ].next
                        = SAFE_TRIE_NODENUM(
                                       trie->trans[ stateidx + charid ].next );
                    trie->trans[ pos ].check = state;
                    pos++;
                }
            }
        }
        trie->lasttrans = pos + 1;
        trie->states = (reg_trie_state *)
            PerlMemShared_realloc( trie->states, laststate
                                   * sizeof(reg_trie_state) );
        DEBUG_TRIE_COMPILE_MORE_r(
            Perl_re_indentf( aTHX_  "Alloc: %d Orig: %" IVdf " elements, Final:%" IVdf ". Savings of %%%5.2f\n",
                depth+1,
                (int)( ( TRIE_CHARCOUNT(trie) + 1 ) * trie->uniquecharcount
                       + 1 ),
                (IV)next_alloc,
                (IV)pos,
                ( ( next_alloc - pos ) * 100 ) / (double)next_alloc );
            );

        } /* end table compress */
    }
    DEBUG_TRIE_COMPILE_MORE_r(
            Perl_re_indentf( aTHX_  "Statecount:%" UVxf " Lasttrans:%" UVxf "\n",
                depth+1,
                (UV)trie->statecount,
                (UV)trie->lasttrans)
    );
    /* resize the trans array to remove unused space */
    trie->trans = (reg_trie_trans *)
        PerlMemShared_realloc( trie->trans, trie->lasttrans
                               * sizeof(reg_trie_trans) );

    {   /* Modify the program and insert the new TRIE node */
        U8 nodetype =(U8) flags;
        char *str=NULL;

#ifdef DEBUGGING
        regnode *optimize = NULL;
#endif /* DEBUGGING */
        /* make sure we have enough room to inject the TRIE op */
        assert((!trie->jump) || !trie->jump[1] ||
                (trie->jump[1] >= (sizeof(tregnode_TRIE)/sizeof(struct regnode))));
        /*
           This means we convert either the first branch or the first Exact,
           depending on whether the thing following (in 'last') is a branch
           or not and whther first is the startbranch (ie is it a sub part of
           the alternation or is it the whole thing.)
           Assuming its a sub part we convert the EXACT otherwise we convert
           the whole branch sequence, including the first.
         */
        /* Find the node we are going to overwrite */
        if ( first != startbranch || OP( last ) == BRANCH ) {
            /* branch sub-chain */
            NEXT_OFF( first ) = (U16)(last - first);
            /* whole branch chain */
        }
        /* But first we check to see if there is a common prefix we can
           split out as an EXACT and put in front of the TRIE node.  */
        trie->startstate= 1;
        if ( trie->bitmap && !widecharmap && !trie->jump  ) {
            /* we want to find the first state that has more than
             * one transition, if that state is not the first state
             * then we have a common prefix which we can remove.
             */
            U32 state;
            for ( state = 1 ; state < trie->statecount-1 ; state++ ) {
                U32 ofs = 0;
                I32 first_ofs = -1; /* keeps track of the ofs of the first
                                       transition, -1 means none */
                U32 count = 0;
                const U32 base = trie->states[ state ].trans.base;

                /* does this state terminate an alternation? */
                if ( trie->states[state].wordnum )
                        count = 1;

                for ( ofs = 0 ; ofs < trie->uniquecharcount ; ofs++ ) {
                    if ( ( base + ofs >= trie->uniquecharcount ) &&
                         ( base + ofs - trie->uniquecharcount < trie->lasttrans ) &&
                         trie->trans[ base + ofs - trie->uniquecharcount ].check == state )
                    {
                        if ( ++count > 1 ) {
                            /* we have more than one transition */
                            SV **tmp;
                            U8 *ch;
                            /* if this is the first state there is no common prefix
                             * to extract, so we can exit */
                            if ( state == 1 ) break;
                            tmp = av_fetch_simple( revcharmap, ofs, 0);
                            ch = (U8*)SvPV_nolen_const( *tmp );

                            /* if we are on count 2 then we need to initialize the
                             * bitmap, and store the previous char if there was one
                             * in it*/
                            if ( count == 2 ) {
                                /* clear the bitmap */
                                Zero(trie->bitmap, ANYOF_BITMAP_SIZE, char);
                                DEBUG_OPTIMISE_r(
                                    Perl_re_indentf( aTHX_  "New Start State=%" UVuf " Class: [",
                                        depth+1,
                                        (UV)state));
                                if (first_ofs >= 0) {
                                    SV ** const tmp = av_fetch_simple( revcharmap, first_ofs, 0);
                                    const U8 * const ch = (U8*)SvPV_nolen_const( *tmp );

                                    TRIE_BITMAP_SET_FOLDED(trie,*ch, folder);
                                    DEBUG_OPTIMISE_r(
                                        Perl_re_printf( aTHX_  "%s", (char*)ch)
                                    );
                                }
                            }
                            /* store the current firstchar in the bitmap */
                            TRIE_BITMAP_SET_FOLDED(trie,*ch, folder);
                            DEBUG_OPTIMISE_r(Perl_re_printf( aTHX_ "%s", ch));
                        }
                        first_ofs = ofs;
                    }
                }
                if ( count == 1 ) {
                    /* This state has only one transition, its transition is part
                     * of a common prefix - we need to concatenate the char it
                     * represents to what we have so far. */
                    SV **tmp = av_fetch_simple( revcharmap, first_ofs, 0);
                    STRLEN len;
                    char *ch = SvPV( *tmp, len );
                    DEBUG_OPTIMISE_r({
                        SV *sv=sv_newmortal();
                        Perl_re_indentf( aTHX_  "Prefix State: %" UVuf " Ofs:%" UVuf " Char='%s'\n",
                            depth+1,
                            (UV)state, (UV)first_ofs,
                            pv_pretty(sv, SvPV_nolen_const(*tmp), SvCUR(*tmp), 6,
                                PL_colors[0], PL_colors[1],
                                (SvUTF8(*tmp) ? PERL_PV_ESCAPE_UNI : 0) |
                                PERL_PV_ESCAPE_FIRSTCHAR
                            )
                        );
                    });
                    if ( state==1 ) {
                        OP( convert ) = nodetype;
                        str=STRING(convert);
                        setSTR_LEN(convert, 0);
                    }
                    assert( ( STR_LEN(convert) + len ) < 256 );
                    setSTR_LEN(convert, (U8)(STR_LEN(convert) + len));
                    while (len--)
                        *str++ = *ch++;
                } else {
#ifdef DEBUGGING
                    if (state>1)
                        DEBUG_OPTIMISE_r(Perl_re_printf( aTHX_ "]\n"));
#endif
                    break;
                }
            }
            trie->prefixlen = (state-1);
            if (str) {
                regnode *n = REGNODE_AFTER(convert);
                assert( n - convert <= U16_MAX );
                NEXT_OFF(convert) = n - convert;
                trie->startstate = state;
                trie->minlen -= (state - 1);
                trie->maxlen -= (state - 1);
#ifdef DEBUGGING
               /* At least the UNICOS C compiler choked on this
                * being argument to DEBUG_r(), so let's just have
                * it right here. */
               if (
#ifdef PERL_EXT_RE_BUILD
                   1
#else
                   DEBUG_r_TEST
#endif
                   ) {
                   U32 word = trie->wordcount;
                   while (word--) {
                       SV ** const tmp = av_fetch_simple( trie_words, word, 0 );
                       if (tmp) {
                           if ( STR_LEN(convert) <= SvCUR(*tmp) )
                               sv_chop(*tmp, SvPV_nolen(*tmp) + STR_LEN(convert));
                           else
                               sv_chop(*tmp, SvPV_nolen(*tmp) + SvCUR(*tmp));
                       }
                   }
               }
#endif
                if (trie->maxlen) {
                    convert = n;
                } else {
                    NEXT_OFF(convert) = (U16)(tail - convert);
                    DEBUG_r(optimize= n);
                }
            }
        }
        if (!jumper)
            jumper = last;
        if ( trie->maxlen ) {
            NEXT_OFF( convert ) = (U16)(tail - convert);
            ARG1u_SET( convert, data_slot );
            /* Store the offset to the first unabsorbed branch in
               jump[0], which is otherwise unused by the jump logic.
               We use this when dumping a trie and during optimisation. */
            if (trie->jump)
                trie->jump[0] = (U16)(nextbranch - convert);

            /* If the start state is not accepting (meaning there is no empty string/NOTHING)
             *   and there is a bitmap
             *   and the first "jump target" node we found leaves enough room
             * then convert the TRIE node into a TRIEC node, with the bitmap
             * embedded inline in the opcode - this is hypothetically faster.
             */
            if ( !trie->states[trie->startstate].wordnum
                 && trie->bitmap
                 && ( (char *)jumper - (char *)convert) >= (int)sizeof(tregnode_TRIEC) )
            {
                OP( convert ) = TRIEC;
                Copy(trie->bitmap, ((tregnode_TRIEC *)convert)->bitmap, ANYOF_BITMAP_SIZE, char);
                PerlMemShared_free(trie->bitmap);
                trie->bitmap= NULL;
            } else
                OP( convert ) = TRIE;

            /* store the type in the flags */
            FLAGS(convert) = nodetype;
            DEBUG_r({
            optimize = convert
                      + NODE_STEP_REGNODE
                      + REGNODE_ARG_LEN( OP( convert ) );
            });
            /* XXX We really should free up the resource in trie now,
                   as we won't use them - (which resources?) dmq */
        }
        /* needed for dumping*/
        DEBUG_r(if (optimize) {
            /*
                Try to clean up some of the debris left after the
                optimisation.
             */
            while( optimize < jumper ) {
                OP( optimize ) = OPTIMIZED;
                optimize++;
            }
        });
    } /* end node insert */

    /*  Finish populating the prev field of the wordinfo array.  Walk back
     *  from each accept state until we find another accept state, and if
     *  so, point the first word's .prev field at the second word. If the
     *  second already has a .prev field set, stop now. This will be the
     *  case either if we've already processed that word's accept state,
     *  or that state had multiple words, and the overspill words were
     *  already linked up earlier.
     */
    {
        U16 word;
        U32 state;
        U16 prev;

        for (word=1; word <= trie->wordcount; word++) {
            prev = 0;
            if (trie->wordinfo[word].prev)
                continue;
            state = trie->wordinfo[word].accept;
            while (state) {
                state = prev_states[state];
                if (!state)
                    break;
                prev = trie->states[state].wordnum;
                if (prev)
                    break;
            }
            trie->wordinfo[word].prev = prev;
        }
        Safefree(prev_states);
    }


    /* and now dump out the compressed format */
    DEBUG_TRIE_COMPILE_r(dump_trie(trie, widecharmap, revcharmap, depth+1));

    RExC_rxi->data->data[ data_slot + 1 ] = (void*)widecharmap;
#ifdef DEBUGGING
    RExC_rxi->data->data[ data_slot + TRIE_WORDS_OFFSET ] = (void*)trie_words;
    RExC_rxi->data->data[ data_slot + 3 ] = (void*)revcharmap;
#else
    SvREFCNT_dec_NN(revcharmap);
#endif
    return trie->jump
           ? MADE_JUMP_TRIE
           : trie->startstate>1
             ? MADE_EXACT_TRIE
             : MADE_TRIE;
}

regnode *
Perl_construct_ahocorasick_from_trie(pTHX_ RExC_state_t *pRExC_state, regnode *source, U32 depth)
{
/* The Trie is constructed and compressed now so we can build a fail array if
 * it's needed

   This is basically the Aho-Corasick algorithm. Its from exercise 3.31 and
   3.32 in the
   "Red Dragon" -- Compilers, principles, techniques, and tools. Aho, Sethi,
   Ullman 1985/88
   ISBN 0-201-10088-6

   We find the fail state for each state in the trie, this state is the longest
   proper suffix of the current state's 'word' that is also a proper prefix of
   another word in our trie. State 1 represents the word '' and is thus the
   default fail state. This allows the DFA not to have to restart after its
   tried and failed a word at a given point, it simply continues as though it
   had been matching the other word in the first place.
   Consider
      'abcdgu'=~/abcdefg|cdgu/
   When we get to 'd' we are still matching the first word, we would encounter
   'g' which would fail, which would bring us to the state representing 'd' in
   the second word where we would try 'g' and succeed, proceeding to match
   'cdgu'.
 */
 /* add a fail transition */
    const U32 trie_offset = ARG1u(source);
    reg_trie_data *trie=(reg_trie_data *)RExC_rxi->data->data[trie_offset];
    U32 *q;
    const U32 ucharcount = trie->uniquecharcount;
    const U32 numstates = trie->statecount;
    const U32 ubound = trie->lasttrans + ucharcount;
    U32 q_read = 0;
    U32 q_write = 0;
    U32 charid;
    U32 base = trie->states[ 1 ].trans.base;
    U32 *fail;
    reg_ac_data *aho;
    const U32 data_slot = reg_add_data( pRExC_state, STR_WITH_LEN("T"));
    regnode *stclass;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_CONSTRUCT_AHOCORASICK_FROM_TRIE;
    PERL_UNUSED_CONTEXT;
#ifndef DEBUGGING
    PERL_UNUSED_ARG(depth);
#endif

    if ( OP(source) == TRIE ) {
        tregnode_TRIE *op = (tregnode_TRIE *)
            PerlMemShared_calloc(1, sizeof(tregnode_TRIE));
        StructCopy(source, op, tregnode_TRIE);
        stclass = (regnode *)op;
    } else {
        tregnode_TRIEC *op = (tregnode_TRIEC *)
            PerlMemShared_calloc(1, sizeof(tregnode_TRIEC));
        StructCopy(source, op, tregnode_TRIEC);
        stclass = (regnode *)op;
    }
    OP(stclass)+=2; /* convert the TRIE type to its AHO-CORASICK equivalent */

    ARG1u_SET( stclass, data_slot );
    aho = (reg_ac_data *) PerlMemShared_calloc( 1, sizeof(reg_ac_data) );
    RExC_rxi->data->data[ data_slot ] = (void*)aho;
    aho->trie=trie_offset;
    aho->states=(reg_trie_state *)PerlMemShared_malloc( numstates * sizeof(reg_trie_state) );
    Copy( trie->states, aho->states, numstates, reg_trie_state );
    Newx( q, numstates, U32);
    aho->fail = (U32 *) PerlMemShared_calloc( numstates, sizeof(U32) );
    aho->refcount = 1;
    fail = aho->fail;
    /* initialize fail[0..1] to be 1 so that we always have
       a valid final fail state */
    fail[ 0 ] = fail[ 1 ] = 1;

    for ( charid = 0; charid < ucharcount ; charid++ ) {
        const U32 newstate = TRIE_TRANS_STATE( 1, base, ucharcount, charid, 0 );
        if ( newstate ) {
            q[ q_write ] = newstate;
            /* set to point at the root */
            fail[ q[ q_write++ ] ]=1;
        }
    }
    while ( q_read < q_write) {
        const U32 cur = q[ q_read++ % numstates ];
        base = trie->states[ cur ].trans.base;

        for ( charid = 0 ; charid < ucharcount ; charid++ ) {
            const U32 ch_state = TRIE_TRANS_STATE( cur, base, ucharcount, charid, 1 );
            if (ch_state) {
                U32 fail_state = cur;
                U32 fail_base;
                do {
                    fail_state = fail[ fail_state ];
                    fail_base = aho->states[ fail_state ].trans.base;
                } while ( !TRIE_TRANS_STATE( fail_state, fail_base, ucharcount, charid, 1 ) );

                fail_state = TRIE_TRANS_STATE( fail_state, fail_base, ucharcount, charid, 1 );
                fail[ ch_state ] = fail_state;
                if ( !aho->states[ ch_state ].wordnum && aho->states[ fail_state ].wordnum )
                {
                        aho->states[ ch_state ].wordnum =  aho->states[ fail_state ].wordnum;
                }
                q[ q_write++ % numstates] = ch_state;
            }
        }
    }
    /* restore fail[0..1] to 0 so that we "fall out" of the AC loop
       when we fail in state 1, this allows us to use the
       charclass scan to find a valid start char. This is based on the principle
       that theres a good chance the string being searched contains lots of stuff
       that cant be a start char.
     */
    fail[ 0 ] = fail[ 1 ] = 0;
    DEBUG_TRIE_COMPILE_r({
        Perl_re_indentf( aTHX_  "Stclass Failtable (%" UVuf " states): 0",
                      depth, (UV)numstates
        );
        for( q_read=1; q_read<numstates; q_read++ ) {
            Perl_re_printf( aTHX_  ", %" UVuf, (UV)fail[q_read]);
        }
        Perl_re_printf( aTHX_  "\n");
    });
    Safefree(q);
    /*RExC_seen |= REG_TRIEDFA_SEEN;*/
    return stclass;
}
