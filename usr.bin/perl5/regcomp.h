/*    regcomp.h
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999,
 *    2000, 2001, 2002, 2003, 2005, 2006, 2007, by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#if ! defined(PERL_REGCOMP_H_) && (   defined(PERL_CORE)            \
                                   || defined(PERL_EXT_RE_BUILD))

#define PERL_REGCOMP_H_

#ifndef RE_PESSIMISTIC_PARENS
/* Define this to 1 if you want to enable a really aggressive and
 * inefficient paren cleanup during backtracking which should ensure
 * correctness. Doing so should fix any bugs related to backreferences,
 * at the cost of saving and restoring paren state far more than we
 * necessarily must.
 *
 * When it is set to 0 we try to optimize away unnecessary save/restore
 * operations which could potentially introduce bugs. We should pass our
 * test suite with this as 0, but setting it to 1 might fix cases we do
 * not currently test for. If setting this to 1 does fix a bug, then
 * review the code related to storing and restoring paren state.
 *
 * See comment for VOLATILE_REF below for more details of a
 * related case.
 */
#define RE_PESSIMISTIC_PARENS 0
#endif

/* a VOLATILE_REF is a ref which is inside of a capturing group and it
 * refers to the capturing group it is inside of or to a following capture
 * group which might be affected by what this capture group matches, and
 * thus the ref requires additional backtracking support. For example:
 *
 *  "xa=xaaa" =~ /^(xa|=?\1a){2}\z/
 *
 * should not match.  In older perls the matching process would go like this:
 *
 * Iter 1: "xa" matches in capture group.
 * Iter 2: "xa" does not match, goes to next alternation.
 *         "=" matches in =?
 *         Bifurcates here (= might not match)
 *         "xa" matches via \1 from previous iteration
 *         "a" matches via "a" at end of second alternation
 *         # at this point $1 is "=xaa"
 *         \z  does not match -> backtracks.
 * Backtracks to Iter 2 "=?" Bifurcation point where we have NOT matched "="
 *         "=xaa" matches via \1 (as $1 has not been reset)
 *         "a" matches via "a" at end of second alternation
 *         "\z" does match. -> Pattern matches overall.
 *
 * What should happen and now does happen instead is:
 *
 * Backtracks to Iter 2 "=?" Bifurcation point where we have NOT matched "=",
 *         \1 does not match as it is "xa" (as $1 was reset when backtracked)
 *         and the current character in the string is an "="
 *
 * The fact that \1 in this case is marked as a VOLATILE_REF is what ensures
 * that we reset the capture buffer properly.
 *
 * See 59db194299c94c6707095797c3df0e2f67ff82b2
 * and 38508ce8fc3a1bd12a3bb65e9d4ceb9b396a18db
 * for more details.
 */
#define VOLATILE_REF 1

#include "regcharclass.h"

/* Convert branch sequences to more efficient trie ops? */
#define PERL_ENABLE_TRIE_OPTIMISATION 1

/* Be really aggressive about optimising patterns with trie sequences? */
#define PERL_ENABLE_EXTENDED_TRIE_OPTIMISATION 1

/* Should the optimiser take positive assertions into account? */
#define PERL_ENABLE_POSITIVE_ASSERTION_STUDY 0

/* Not for production use: */
#define PERL_ENABLE_EXPERIMENTAL_REGEX_OPTIMISATIONS 0

/*
 * Structure for regexp "program".  This is essentially a linear encoding
 * of a nondeterministic finite-state machine (aka syntax charts or
 * "railroad normal form" in parsing technology).  Each node is an opcode
 * plus a "next" pointer, possibly plus an operand.  "Next" pointers of
 * all nodes except BRANCH implement concatenation; a "next" pointer with
 * a BRANCH on both ends of it is connecting two alternatives.  (Here we
 * have one of the subtle syntax dependencies:  an individual BRANCH (as
 * opposed to a collection of them) is never concatenated with anything
 * because of operator precedence.)  The operand of some types of node is
 * a literal string; for others, it is a node leading into a sub-FSM.  In
 * particular, the operand of a BRANCH node is the first node of the branch.
 * (NB this is *not* a tree structure:  the tail of the branch connects
 * to the thing following the set of BRANCHes.)  The opcodes are defined
 * in regnodes.h which is generated from regcomp.sym by regcomp.pl.
 */

/*
 * A node is one char of opcode followed by two chars of "next" pointer.
 * "Next" pointers are stored as two 8-bit pieces, high order first.  The
 * value is a positive offset from the opcode of the node containing it.
 * An operand, if any, simply follows the node.  (Note that much of the
 * code generation knows about this implicit relationship.)
 *
 * Using two bytes for the "next" pointer is vast overkill for most things,
 * but allows patterns to get big without disasters.
 *
 * [The "next" pointer is always aligned on an even
 * boundary, and reads the offset directly as a short.]
 */

/* This is the stuff that used to live in regexp.h that was truly
   private to the engine itself. It now lives here. */

typedef struct regexp_internal {
        regnode *regstclass;    /* Optional startclass as identified or constructed
                                   by the optimiser */
        struct reg_data *data;	/* Additional miscellaneous data used by the program.
                                   Used to make it easier to clone and free arbitrary
                                   data that the regops need. Often the ARG field of
                                   a regop is an index into this structure. NOTE the
                                   0th element of this structure is NEVER used and is
                                   strictly reserved for internal purposes. */
        struct reg_code_blocks *code_blocks;/* positions of literal (?{}) */
        U32 proglen;            /* size of the compiled program in regnodes */
        U32 name_list_idx;      /* Optional data index of an array of paren names,
                                   only valid when RXp_PAREN_NAMES(prog) is true,
                                   0 means "no value" like any other index into the
                                   data array.*/
        regnode program[1];	/* Unwarranted chumminess with compiler. */
} regexp_internal;

#define RXi_SET(x,y) (x)->pprivate = (void*)(y)   
#define RXi_GET(x)   ((regexp_internal *)((x)->pprivate))
#define RXi_GET_DECL(r,ri) regexp_internal *ri = RXi_GET(r)
#define RXi_GET_DECL_NULL(r,ri) regexp_internal *ri = (r) ? RXi_GET(r) : NULL
/*
 * Flags stored in regexp->intflags
 * These are used only internally to the regexp engine
 *
 * See regexp.h for flags used externally to the regexp engine
 */
#define RXp_INTFLAGS(rx)        ((rx)->intflags)
#define RX_INTFLAGS(prog)        RXp_INTFLAGS(ReANY(prog))

#define PREGf_SKIP		0x00000001
#define PREGf_IMPLICIT		0x00000002 /* Converted .* to ^.* */
#define PREGf_NAUGHTY		0x00000004 /* how exponential is this pattern? */
#define PREGf_VERBARG_SEEN	0x00000008
#define PREGf_CUTGROUP_SEEN	0x00000010
#define PREGf_USE_RE_EVAL	0x00000020 /* compiled with "use re 'eval'" */
/* these used to be extflags, but are now intflags */
#define PREGf_NOSCAN            0x00000040
                                /* spare */
#define PREGf_GPOS_SEEN         0x00000100
#define PREGf_GPOS_FLOAT        0x00000200

#define PREGf_ANCH_MBOL         0x00000400
#define PREGf_ANCH_SBOL         0x00000800
#define PREGf_ANCH_GPOS         0x00001000
#define PREGf_RECURSE_SEEN      0x00002000
#define PREGf_PESSIMIZE_SEEN    0x00004000

#define PREGf_ANCH              \
    ( PREGf_ANCH_SBOL | PREGf_ANCH_GPOS | PREGf_ANCH_MBOL )

/* this is where the old regcomp.h started */


/* Define the various regnode structures. These all should be a multiple
 * of 32 bits large, and they should by and large correspond with each other
 * in terms of naming, etc. Things can and will break in subtle ways if you
 * change things without care. If you look at regexp.h you will see it
 * contains this:
 *
 * union regnode_head {
 *   struct {
 *     union {
 *       U8 flags;
 *       U8 str_len_u8;
 *       U8 first_byte;
 *     } u_8;
 *     U8  type;
 *     U16 next_off;
 *   } data;
 *   U32 data_u32;
 * };
 *
 * struct regnode {
 *   union regnode_head head;
 * };
 *
 * Which really is a complicated and alignment friendly version of
 *
 *  struct {
 *    U8  flags;
 *    U8  type;
 *    U16 next_off;
 *  };
 *
 * This structure is the base unit of elements in the regexp program.
 * When we increment our way through the program we increment by the
 * size of this structure (32 bits), and in all cases where regnode
 * sizing is considered it is in units of this structure. All regnodes
 * have a union regnode_head as their first parameter.
 *
 * This implies that no regnode style structure should contain 64 bit
 * aligned members. Since the base regnode is 32 bits any member might
 * not be 64 bit aligned no matter how you might try to pad out the
 * struct itself (the regnode_ssc is special in this regard as it is
 * never used in a program directly). If you want to store 64 bit
 * members you need to store them specially. The struct regnode_p and the
 * ARGp() and ARGp_SET() macros and related inline functions provide an example
 * solution. Note they deal with a slightly more complicated problem than simple
 * alignment, as pointers may be 32 bits or 64 bits depending on platform,
 * but they illustrate the pattern to follow if you want to put a 64 bit value
 * into a regnode.

 * NOTE: Ideally we do not put pointers into the regnodes in a program. Instead
 * we put them in the "data" part of the regexp structure and store the index into
 * the data in the pointers in the regnode. This allows the pointer to be handled
 * properly during clone/free operations (eg refcount bookkeeping). See S_add_data(),
 * Perl_regdupe_internal(), Perl_regfree_internal() in regcomp.c for how the data
 * array can be used, the letters 'arsSu' all refer to different types of SV that
 * we already have support for in the data array.
 */

union regnode_arg {
    I32 i32;
    U32 u32;
    struct {
        U16 u16a;
        U16 u16b;
    } hi_lo;
};


struct regnode_string {
    union regnode_head head;
    char string[1];
};

struct regnode_lstring { /* Constructed this way to keep the string aligned. */
    union regnode_head head;
    U32 str_len_u32;    /* Only 18 bits allowed before would overflow 'next_off' */
    char string[1];
};

struct regnode_anyofhs { /* Constructed this way to keep the string aligned. */
    union regnode_head head;
    union regnode_arg arg1;
    char string[1];
};

/* Argument bearing node - workhorse, ARG1u() is often used for the data field
 * Can store either a signed 32 bit value via ARG1i() or unsigned 32 bit value
 * via ARG1u(), or two unsigned 16 bit values via ARG1a() or ARG1b()
 */
struct regnode_1 {
    union regnode_head head;
    union regnode_arg arg1;
};

/* Node whose argument is 'SV *'.  This needs to be used very carefully in
 * situations where pointers won't become invalid because of, say re-mallocs.
 *
 * Note that this regnode type is problematic and should not be used or copied
 * and will be removed in the future. Pointers should be stored in the data[]
 * array and an index into the data array stored in the regnode, which allows the
 * pointers to be handled properly during clone/free operations on the regexp
 * data structure. As a byproduct it also saves space, often we use a 16 bit
 * member to store indexes into the data[] array.
 *
 * Also note that the weird storage here is because regnodes are 32 bit aligned,
 * which means we cannot have a 64 bit aligned member. To make things more annoying
 * the size of a pointer may vary by platform. Thus we use a character array, and
 * then use inline functions to copy the data in or out.
 * */
struct regnode_p {
    union regnode_head head;
    char arg1_sv_ptr_bytes[sizeof(SV *)];
};

/* "Two Node" - similar to a regnode_1 but with space for an extra 32
 * bit value, or two 16 bit valus. The first fields must match regnode_1.
 * Extra field can be accessed as (U32)ARG2u() (I32)ARG2i() or (U16)ARG2a()
 * and (U16)ARG2b() */
struct regnode_2 {
    union regnode_head head;
    union regnode_arg arg1;
    union regnode_arg arg2;
};

/* "Three Node" - similar to a regnode_2 but with space for an additional
 * 32 bit value, or two 16 bit values. The first fields must match regnode_2.
 * The extra field can be accessed as (U32)ARG3u() (I32)ARG3i() or (U16)ARG3a()
 * and (U16)ARG3b().
 * Currently used for the CURLY style regops used to represent quantifers,
 * storing the min and of the quantifier via ARG1i() and ARG2i(), along with
 * ARG3a() and ARG3b() which are used to store information about the number of
 * parens before and inside the quantified expression. */
struct regnode_3 {
    union regnode_head head;
    union regnode_arg arg1;
    union regnode_arg arg2;
    union regnode_arg arg3;
};

#define REGNODE_BBM_BITMAP_LEN                                                  \
                      /* 6 info bits requires 64 bits; 5 => 32 */               \
                    ((1 << (UTF_CONTINUATION_BYTE_INFO_BITS)) / CHARBITS)

/* Used for matching any two-byte UTF-8 character whose start byte is known.
 * The array is a bitmap capable of representing any possible continuation
 * byte. */
struct regnode_bbm {
    union regnode_head head;
    U8 bitmap[REGNODE_BBM_BITMAP_LEN];
};

#define ANYOF_BITMAP_SIZE	(NUM_ANYOF_CODE_POINTS / CHARBITS)

/* Note that these form structs which are supersets of the next smaller one, by
 * appending fields.  Alignment problems can occur if one of those optional
 * fields requires stricter alignment than the base struct.  And formal
 * parameters that can really be two or more of the structs should be
 * declared as the smallest one it could be.  See commit message for
 * 7dcac5f6a5195002b55c935ee1d67f67e1df280b.  Regnode allocation is done
 * without regard to alignment, and changing it to would also require changing
 * the code that inserts and deletes regnodes.  The basic single-argument
 * regnode has a U32, which is what reganode() allocates as a unit.  Therefore
 * no field can require stricter alignment than U32. */
    
/* also used by trie */
struct regnode_charclass {
    union regnode_head head;
    union regnode_arg arg1;
    char bitmap[ANYOF_BITMAP_SIZE];	/* only compile-time */
};

/* has runtime (locale) \d, \w, ..., [:posix:] classes */
struct regnode_charclass_posixl {
    union regnode_head head;
    union regnode_arg arg1;
    char bitmap[ANYOF_BITMAP_SIZE];		/* both compile-time ... */
    U32 classflags;	                        /* and run-time */
};

/* A synthetic start class (SSC); is a regnode_charclass_posixl_fold, plus an
 * extra SV*, used only during regex construction and which is not used by the
 * main machinery in regexec.c and which does not get embedded in the final compiled
 * regex program.
 *
 * Because it does not get embedded it does not have to comply with the alignment
 * and sizing constraints required for a normal regnode structure: it MAY contain
 * pointers or members of whatever size needed and the compiler will do the right
 * thing. (Every other regnode type is 32 bit aligned.)
 *
 * Note that the 'next_off' field is unused, as the SSC stands alone, so there is
 * never a next node.
 */
struct regnode_ssc {
    union regnode_head head;
    union regnode_arg arg1;
    char bitmap[ANYOF_BITMAP_SIZE];	/* both compile-time ... */
    U32 classflags;	                /* ... and run-time */

    /* Auxiliary, only used during construction; NULL afterwards: list of code
     * points matched */
    SV* invlist;
};

/*  We take advantage of 'next_off' not otherwise being used in the SSC by
 *  actually using it: by setting it to 1.  This allows us to test and
 *  distinguish between an SSC and other ANYOF node types, as 'next_off' cannot
 *  otherwise be 1, because it is the offset to the next regnode expressed in
 *  units of regnodes.  Since an ANYOF node contains extra fields, it adds up
 *  to 12 regnode units on 32-bit systems, (hence the minimum this can be (if
 *  not 0) is 11 there.  Even if things get tightly packed on a 64-bit system,
 *  it still would be more than 1. */
#define set_ANYOF_SYNTHETIC(n)  \
    STMT_START{                 \
        OP(n) = ANYOF;          \
        NEXT_OFF(n) = 1;        \
    } STMT_END

#define is_ANYOF_SYNTHETIC(n) (REGNODE_TYPE(OP(n)) == ANYOF && NEXT_OFF(n) == 1)

/* XXX fix this description.
   Impose a limit of REG_INFTY on various pattern matching operations
   to limit stack growth and to avoid "infinite" recursions.
*/
/* The default size for REG_INFTY is I32_MAX, which is the same as UINT_MAX
   (see perl.h). Unfortunately I32 isn't necessarily 32 bits (see handy.h).
   On the Cray C90, or Cray T90, I32_MAX is considerably larger than it
   might be elsewhere. To limit stack growth to reasonable sizes, supply a
   smaller default.
        --Andy Dougherty  11 June 1998
        --Amended by Yves Orton 15 Jan 2023
*/
#if INTSIZE > 4
#  ifndef REG_INFTY
#    define REG_INFTY  nBIT_IMAX(32)
#  endif
#endif

#ifndef REG_INFTY
#  define REG_INFTY I32_MAX
#endif

#define ARG_VALUE(arg) (arg)
#define ARG__SET(arg,val) ((arg) = (val))

#undef ARG
#undef ARG1
#undef ARG2

/* convention: each arg is is 32 bits, with the "u" suffix
 * being unsigned 32 bits, the "i" suffix being signed 32 bits,
 * and the "a" and "b" suffixes being unsigned 16 bit fields.
 *
 * We provide all 4 macros for each case for consistency, even
 * though they arent all used.
 */

#define ARG1u(p) ARG_VALUE(ARG1u_LOC(p))
#define ARG1i(p) ARG_VALUE(ARG1i_LOC(p))
#define ARG1a(p) ARG_VALUE(ARG1a_LOC(p))
#define ARG1b(p) ARG_VALUE(ARG1b_LOC(p))

#define ARG2u(p) ARG_VALUE(ARG2u_LOC(p))
#define ARG2i(p) ARG_VALUE(ARG2i_LOC(p))
#define ARG2a(p) ARG_VALUE(ARG2a_LOC(p))
#define ARG2b(p) ARG_VALUE(ARG2b_LOC(p))

#define ARG3u(p) ARG_VALUE(ARG3u_LOC(p))
#define ARG3i(p) ARG_VALUE(ARG3i_LOC(p))
#define ARG3a(p) ARG_VALUE(ARG3a_LOC(p))
#define ARG3b(p) ARG_VALUE(ARG3b_LOC(p))

#define ARGp(p) ARGp_VALUE_inline(p)

#define ARG1u_SET(p, val) ARG__SET(ARG1u_LOC(p), (val))
#define ARG1i_SET(p, val) ARG__SET(ARG1i_LOC(p), (val))
#define ARG1a_SET(p, val) ARG__SET(ARG1a_LOC(p), (val))
#define ARG1b_SET(p, val) ARG__SET(ARG1b_LOC(p), (val))

#define ARG2u_SET(p, val) ARG__SET(ARG2u_LOC(p), (val))
#define ARG2i_SET(p, val) ARG__SET(ARG2i_LOC(p), (val))
#define ARG2a_SET(p, val) ARG__SET(ARG2a_LOC(p), (val))
#define ARG2b_SET(p, val) ARG__SET(ARG2b_LOC(p), (val))

#define ARG3u_SET(p, val) ARG__SET(ARG3u_LOC(p), (val))
#define ARG3i_SET(p, val) ARG__SET(ARG3i_LOC(p), (val))
#define ARG3a_SET(p, val) ARG__SET(ARG3a_LOC(p), (val))
#define ARG3b_SET(p, val) ARG__SET(ARG3b_LOC(p), (val))

#define ARGp_SET(p, val) ARGp_SET_inline((p),(val))

/* the following define was set to 0xde in 075abff3
 * as part of some linting logic. I have set it to 0
 * as otherwise in every place where we /might/ set flags
 * we have to set it 0 explicitly, which duplicates
 * assignments and IMO adds an unacceptable level of
 * surprise to working in the regex engine. If this
 * is changed from 0 then at the very least make sure
 * that SBOL for /^/ sets the flags to 0 explicitly.
 * -- Yves */

#define NODE_ALIGN(node)
#define SIZE_ALIGN NODE_ALIGN

#undef OP
#undef OPERAND
#undef STRING
#undef NEXT_OFF
#undef NODE_ALIGN

#define NEXT_OFF(p)     ((p)->head.data.next_off)
#define OP(p)           ((p)->head.data.type)
#define STR_LEN_U8(p)   ((p)->head.data.u_8.str_len_u8)
#define FIRST_BYTE(p)   ((p)->head.data.u_8.first_byte)
#define FLAGS(p)        ((p)->head.data.u_8.flags) /* Caution: Doesn't apply to all      \
                                           regnode types.  For some, it's the \
                                           character set of the regnode */
#define STR_LENs(p)	(__ASSERT_(OP(p) != LEXACT && OP(p) != LEXACT_REQ8)  \
                                    STR_LEN_U8((struct regnode_string *)p))
#define STRINGs(p)	(__ASSERT_(OP(p) != LEXACT && OP(p) != LEXACT_REQ8)  \
                                    ((struct regnode_string *)p)->string)
#define OPERANDs(p)	STRINGs(p)

#define PARNO(p)        ARG1u(p)          /* APPLIES for OPEN and CLOSE only */

#define NODE_ALIGN_FILL(node) (FLAGS(node) = 0)

/* Long strings.  Currently limited to length 18 bits, which handles a 262000
 * byte string.  The limiting factor is the 16 bit 'next_off' field, which
 * points to the next regnode, so the furthest away it can be is 2**16.  On
 * most architectures, regnodes are 2**2 bytes long, so that yields 2**18
 * bytes.  Should a longer string be desired, we could increase it to 26 bits
 * fairly easily, by changing this node to have longj type which causes the ARG
 * field to be used for the link to the next regnode (although code would have
 * to be changed to account for this), and then use a combination of the flags
 * and next_off fields for the length.  To get 34 bit length, also change the
 * node to be an ARG2L, using the second 32 bit field for the length, and not
 * using the flags nor next_off fields at all.  One could have an llstring node
 * and even an lllstring type. */
#define STR_LENl(p)	(__ASSERT_(OP(p) == LEXACT || OP(p) == LEXACT_REQ8)  \
                                    (((struct regnode_lstring *)p)->str_len_u32))
#define STRINGl(p)	(__ASSERT_(OP(p) == LEXACT || OP(p) == LEXACT_REQ8)  \
                                    (((struct regnode_lstring *)p)->string))
#define OPERANDl(p)	STRINGl(p)

#define STR_LEN(p)	((OP(p) == LEXACT || OP(p) == LEXACT_REQ8)           \
                                               ? STR_LENl(p) : STR_LENs(p))
#define STRING(p)	((OP(p) == LEXACT || OP(p) == LEXACT_REQ8)           \
                                               ? STRINGl(p)  : STRINGs(p))
#define OPERAND(p)	STRING(p)

/* The number of (smallest) regnode equivalents that a string of length l bytes
 * occupies - Used by the REGNODE_AFTER() macros and functions. */
#define STR_SZ(l)	(((l) + sizeof(regnode) - 1) / sizeof(regnode))

#define setSTR_LEN(p,v)                                                     \
    STMT_START{                                                             \
        if (OP(p) == LEXACT || OP(p) == LEXACT_REQ8)                        \
            ((struct regnode_lstring *)(p))->str_len_u32 = (v);             \
        else                                                                \
            STR_LEN_U8((struct regnode_string *)(p)) = (v);                 \
    } STMT_END

#define ANYOFR_BASE_BITS    20
#define ANYOFRbase(p)   (ARG1u(p) & nBIT_MASK(ANYOFR_BASE_BITS))
#define ANYOFRdelta(p)  (ARG1u(p) >> ANYOFR_BASE_BITS)

#undef NODE_ALIGN
#undef ARG_LOC

#define NODE_ALIGN(node)
#define ARGp_BYTES_LOC(p)  (((struct regnode_p *)p)->arg1_sv_ptr_bytes)
#define ARG1u_LOC(p)    (((struct regnode_1 *)p)->arg1.u32)
#define ARG1i_LOC(p)    (((struct regnode_1 *)p)->arg1.i32)
#define ARG1a_LOC(p)    (((struct regnode_1 *)p)->arg1.hi_lo.u16a)
#define ARG1b_LOC(p)    (((struct regnode_1 *)p)->arg1.hi_lo.u16b)
#define ARG2u_LOC(p)    (((struct regnode_2 *)p)->arg2.u32)
#define ARG2i_LOC(p)    (((struct regnode_2 *)p)->arg2.i32)
#define ARG2a_LOC(p)    (((struct regnode_2 *)p)->arg2.hi_lo.u16a)
#define ARG2b_LOC(p)    (((struct regnode_2 *)p)->arg2.hi_lo.u16b)
#define ARG3u_LOC(p)    (((struct regnode_3 *)p)->arg3.u32)
#define ARG3i_LOC(p)    (((struct regnode_3 *)p)->arg3.i32)
#define ARG3a_LOC(p)    (((struct regnode_3 *)p)->arg3.hi_lo.u16a)
#define ARG3b_LOC(p)    (((struct regnode_3 *)p)->arg3.hi_lo.u16b)

/* These should no longer be used directly in most cases. Please use
 * the REGNODE_AFTER() macros instead. */
#define NODE_STEP_REGNODE	1	/* sizeof(regnode)/sizeof(regnode) */

/* Core macros for computing "the regnode after this one". See also
 * Perl_regnode_after() in reginline.h
 *
 * At the struct level regnodes are a linked list, with each node pointing
 * at the next (via offsets), usually via the C<next_off> field in the
 * structure. Where there is a need for a node to have two children the
 * immediate physical successor of the node in the compiled program is used
 * to represent one of them. A good example is the BRANCH construct,
 * consider the pattern C</head(?:[ab]foo|[cd]bar)tail/>
 *
 *      1: EXACT <head> (3)
 *      3: BRANCH (8)
 *      4:   ANYOFR[ab] (6)
 *      6:   EXACT <foo> (14)
 *      8: BRANCH (FAIL)
 *      9:   ANYOFR[cd] (11)
 *     11:   EXACT <bar> (14)
 *     13: TAIL (14)
 *     14: EXACT <tail> (16)
 *     16: END (0)
 *
 * The numbers in parens at the end of each line show the "next_off" value
 * for that regnode in the program. We can see that the C<next_off> of
 * the first BRANCH node (#3) is the second BRANCH node (#8), and indicates
 * where execution should go if the regnodes *following* the BRANCH node fail
 * to accept the input string. Thus to find the "next BRANCH" we would do
 * C<Perl_regnext()> and follow the C<next_off> pointer, and to find
 * the "BRANCHes contents" we would use C<REGNODE_AFTER()>.
 *
 * Be aware that C<REGNODE_AFTER()> is not guaranteed to give a *useful*
 * result once the regex peephole optimizer has run (it will be correct
 * however!). By the time code in regexec.c executes various regnodes
 * may have been optimized out of the C<next_off> chain. An example
 * can be seen above, node 13 will never be reached during execution
 * flow as it has been stitched out of the C<next_off> chain. Both 6 and
 * 11 would have pointed at it during compilation, but it exists only to
 * facilitate the construction of the BRANCH structure and is effectively
 * a NOOP, and thus the optimizer adjusts the links so it is skipped
 * from execution time flow. In regexec.c it is only safe to use
 * REGNODE_AFTER() on specific node types.
 *
 * Conversely during compilation C<Perl_regnext()> may not work properly
 * as the C<next_off> may not be known until "later", (such as in the
 * case of BRANCH nodes) and thus in regcomp.c the REGNODE_AFTER() macro
 * is used very heavily instead.
 *
 * There are several variants of the REGNODE_AFTER_xxx() macros which
 * are intended for use in different situations depending on how
 * confident the code is about what type of node it is trying to find a
 * successor for.
 *
 * So for instance if you know you are dealing with a known node type of
 * constant size then you should use REGNODE_AFTER_type(n,TYPE).
 *
 * If you have a regnode pointer and you know you are dealing with a
 * regnode type of constant size and you have already extracted its
 * opcode use: REGNODE_AFTER_opcode(n,OPCODE).
 *
 * If you have a regnode and you know it is variable size then you
 * you can produce optimized code by using REGNODE_AFTER_varies(n).
 *
 * If you have a regnode pointer and nothing else use: REGNODE_AFTER(n)
 * This is the safest option and wraps C<Perl_regnode_after()>. It
 * should produce the correct result regardless of its argument. The
 * other options only produce correct results under specific
 * constraints.
 */
#define        REGNODE_AFTER_PLUS(p,extra)    ((p) + NODE_STEP_REGNODE + (extra))
/* under DEBUGGING we check that all REGNODE_AFTER optimized macros did the
 * same thing that Perl_regnode_after() would have done. Note that when
 * not compiled under DEBUGGING the assert_() macro is empty. Thus we
 * don't have to implement different versions for DEBUGGING and not DEBUGGING,
 * and explains why all the macros use REGNODE_AFTER_PLUS_DEBUG() under the
 * hood. */
#define REGNODE_AFTER_PLUS_DEBUG(p,extra) \
    (assert_(check_regnode_after(p,extra))  REGNODE_AFTER_PLUS((p),(extra)))

/* find the regnode after this p by using the opcode we previously extracted
 * with OP(p) */
#define REGNODE_AFTER_opcode(p,op)          REGNODE_AFTER_PLUS_DEBUG((p),REGNODE_ARG_LEN(op))

/* find the regnode after this p by using the size of the struct associated with
 * the opcode for p. use this when you *know* that p is pointer to a given type*/
#define REGNODE_AFTER_type(p,t)             REGNODE_AFTER_PLUS_DEBUG((p),EXTRA_SIZE(t))

/* find the regnode after this p by using OP(p) to find the regnode type of p */
#define REGNODE_AFTER_varies(p)            regnode_after(p,TRUE)

/* find the regnode after this p by using OP(p) to find the regnode type of p */
#define REGNODE_AFTER(p)            regnode_after(p,FALSE)


/* REGNODE_BEFORE() is trickier to deal with in terms of validation, execution.
 * All the places that use it assume that p will be one struct regnode large.
 * So to validate it we do the math to go backwards and then validate that the
 * type of regnode we landed on is actually one regnode large. In theory if
 * things go wrong the opcode should be illegal or say the item should be larger
 * than it is, etc. */
#define        REGNODE_BEFORE_BASE(p)        ((p) - NODE_STEP_REGNODE)
#define        REGNODE_BEFORE_BASE_DEBUG(p)        \
    (assert_(check_regnode_after(REGNODE_BEFORE_BASE(p),0))  REGNODE_BEFORE_BASE(p))
#define REGNODE_BEFORE(p) REGNODE_BEFORE_BASE_DEBUG(p)

#define FILL_NODE(offset, op)                                           \
    STMT_START {                                                        \
                    OP(REGNODE_p(offset)) = op;                         \
                    NEXT_OFF(REGNODE_p(offset)) = 0;                    \
    } STMT_END
#define FILL_ADVANCE_NODE(offset, op)                                   \
    STMT_START {                                                        \
                    FILL_NODE(offset, op);                              \
                    (offset)++;                                         \
    } STMT_END
#define FILL_ADVANCE_NODE_ARG1u(offset, op, arg)                        \
    STMT_START {                                                        \
                    ARG1u_SET(REGNODE_p(offset), arg);                  \
                    FILL_ADVANCE_NODE(offset, op);                      \
                    /* This is used generically for other operations    \
                     * that have a longer argument */                   \
                    (offset) += REGNODE_ARG_LEN(op);                    \
    } STMT_END
#define FILL_ADVANCE_NODE_ARGp(offset, op, arg)                         \
    STMT_START {                                                        \
                    ARGp_SET(REGNODE_p(offset), arg);                   \
                    FILL_ADVANCE_NODE(offset, op);                      \
                    (offset) += REGNODE_ARG_LEN(op);                    \
    } STMT_END
#define FILL_ADVANCE_NODE_2ui_ARG(offset, op, arg1, arg2)               \
    STMT_START {                                                        \
                    ARG1u_SET(REGNODE_p(offset), arg1);                 \
                    ARG2i_SET(REGNODE_p(offset), arg2);                 \
                    FILL_ADVANCE_NODE(offset, op);                      \
                    (offset) += 2;                                      \
    } STMT_END

/* define these after we define the normal macros, so we can use
 * ARGp_BYTES_LOC(n) */

static inline SV *
ARGp_VALUE_inline(struct regnode *node) {
    SV *ptr;
    memcpy(&ptr, ARGp_BYTES_LOC(node), sizeof(ptr));

    return ptr;
}

static inline void
ARGp_SET_inline(struct regnode *node, SV *ptr) {
    memcpy(ARGp_BYTES_LOC(node), &ptr, sizeof(ptr));
}

#define REG_MAGIC 0234

/* An ANYOF node matches a single code point based on specified criteria.  It
 * now comes in several styles, but originally it was just a 256 element
 * bitmap, indexed by the code point (which was always just a byte).  If the
 * corresponding bit for a code point is 1, the code point matches; if 0, it
 * doesn't match (complemented if inverted).  This worked fine before Unicode
 * existed, but making a bit map long enough to accommodate a bit for every
 * possible Unicode code point is prohibitively large.  Therefore it is made
 * much much smaller, and an inversion list is created to handle code points
 * not represented by the bitmap.  (It is now possible to compile the bitmap to
 * a larger size to avoid the slower inversion list lookup for however big the
 * bitmap is set to, but this is rarely done).  If the bitmap is sufficient to
 * specify all possible matches (with nothing outside it matching), no
 * inversion list is needed nor included, and the argument to the ANYOF node is
 * set to the following: */

#define ANYOF_MATCHES_ALL_OUTSIDE_BITMAP_VALUE   U32_MAX
#define ANYOF_MATCHES_ALL_OUTSIDE_BITMAP(node)                              \
                    (ARG1u(node) == ANYOF_MATCHES_ALL_OUTSIDE_BITMAP_VALUE)

#define ANYOF_MATCHES_NONE_OUTSIDE_BITMAP_VALUE                             \
   /* Assumes ALL is odd */  (ANYOF_MATCHES_ALL_OUTSIDE_BITMAP_VALUE - 1)
#define ANYOF_MATCHES_NONE_OUTSIDE_BITMAP(node)                             \
                    (ARG1u(node) == ANYOF_MATCHES_NONE_OUTSIDE_BITMAP_VALUE)

#define ANYOF_ONLY_HAS_BITMAP_MASK  ANYOF_MATCHES_NONE_OUTSIDE_BITMAP_VALUE
#define ANYOF_ONLY_HAS_BITMAP(node)                                         \
  ((ARG1u(node) & ANYOF_ONLY_HAS_BITMAP_MASK) == ANYOF_ONLY_HAS_BITMAP_MASK)

#define ANYOF_HAS_AUX(node)  (! ANYOF_ONLY_HAS_BITMAP(node))

/* There are also ANYOFM nodes, used when the bit patterns representing the
 * matched code points happen to be such that they can be checked by ANDing
 * with a mask.  The regex compiler looks for and silently optimizes to using
 * this node type in the few cases where it works out.  The eight octal digits
 * form such a group.  These nodes are simple and fast and no further
 * discussion is needed here.
 *
 * And, there are ANYOFH-ish nodes which match only code points that aren't in
 * the bitmap  (the H stands for High).  These are common for expressing
 * Unicode properties concerning non-Latin scripts.  They dispense with the
 * bitmap altogether and don't need any of the flags discussed below.
 *
 * And, there are ANYOFR-ish nodes which match within a single range.
 *
 * When there is a need to specify what matches outside the bitmap, it is done
 * by allocating an AV as part of the pattern's compiled form, and the argument
 * to the node instead of being ANYOF_ONLY_HAS_BITMAP, points to that AV.
 *
 * (Actually, that is an oversimplification.  The AV is placed into the
 * pattern's struct reg_data, and what is stored in the node's argument field
 * is its index into that struct.  And the inversion list is just one element,
 * the zeroth, of the AV.)
 *
 * There are certain situations where a single inversion list can't handle all
 * the complexity.  These are dealt with by having extra elements in the AV, by
 * specifying flag bits in the ANYOF node, and/or special code.  As an example,
 * there are instances where what the ANYOF node matches is not completely
 * known until runtime.  In these cases, a flag is set, and the bitmap has a 1
 * for the code points which are known at compile time to be 1, and a 0 for the
 * ones that are known to be 0, or require runtime resolution.  Some missing
 * information can be found by merely seeing if the pattern is UTF-8 or not;
 * other cases require looking at the extra elements in the AV.
 *
 * There are 5 cases where the bitmap is insufficient.  These are specified by
 * flags in the node's flags field.  We could use five bits to represent the 5
 * cases, but to save flags bits (which are perennially in short supply), we
 * play some games.  The cases are:
 *
 *  1)  As already mentioned, if some code points outside the bitmap match, and
 *      some do not, an inversion list is specified to indicate which ones.
 *
 *  2)  Under /d rules, it can happen that code points that are in the upper
 *      latin1 range (\x80-\xFF or their equivalents on EBCDIC platforms) match
 *      only if the runtime target string being matched against is UTF-8.  For
 *      example /[\w[:punct:]]/d.  This happens only for certain posix classes,
 *      and all such ones also have above-bitmap matches.
 *
 *      Note that /d rules are no longer encouraged; 'use 5.14' or higher
 *      deselects them.  But they are still supported, and a flag is required
 *      so that they can be properly handled.  But it can be a shared flag: see
 *      4) below.
 *
 *  3)  Also under /d rules, something like /[\Wfoo]/ will match everything in
 *      the \x80-\xFF range, unless the string being matched against is UTF-8.
 *      An inversion list could be created for this case, but this is
 *      relatively common, and it turns out that it's all or nothing:  if any
 *      one of these code points matches, they all do.  Hence a single bit
 *      suffices.  We use a shared flag that doesn't take up space by itself:
 *      ANYOFD_NON_UTF8_MATCHES_ALL_NON_ASCII__shared.  This also means there
 *      is an inversion list for the things that don't fit into the bitmap.
 *
 *  4)  A user-defined \p{} property may not have been defined by the time the
 *      regex is compiled.  In this case, we don't know until runtime what it
 *      will match, so we have to assume it could match anything, including
 *      code points that ordinarily would be in the bitmap.  A flag bit is
 *      necessary to indicate this, though we can use the
 *      ANYOF_HAS_EXTRA_RUNTIME_MATCHES flag, along with the node not being
 *      ANYOFD.  The information required to construct the property is stored
 *      in the AV pointed to by the node's argument.  This case is quite
 *      uncommon in the field, and the /(?[...])/ construct is a better way to
 *      accomplish what this feature does.
 *
 *  5)  /[foo]/il may have folds that are only valid if the runtime locale is a
 *      UTF-8 one.  The ANYOF_HAS_EXTRA_RUNTIME_MATCHES flag can also be used
 *      for these.  The list is stored in a different element of the AV, so its
 *      existence differentiates this case from that of 4), along with the node
 *      being ANYOFL, with the ANYOFL_FOLD flag being set.  There are a few
 *      additional folds valid only if the UTF-8 locale is a Turkic one which
 *      is tested for explicitly.
 *
 * Note that the user-defined property flag and the /il flag can affect whether
 * an ASCII character matches in the bitmap or not.
 *
 * And this still isn't the end of the story.  In some cases, warnings are
 * supposed to be raised when matching certain categories of code points in the
 * target string.  Flags are set to indicate this.  This adds up to a bunch of
 * flags required, and we only have 8 available.  That is why we share some.
 * At the moment, there are two spare flag bits, but this could be increased by
 * various tricks:
 *
 * ANYOF_MATCHES_POSIXL is redundant with the node type ANYOFPOSIXL.  That flag
 * could be removed, but at the expense of having to write extra code, which
 * would take up space, and writing this turns out to be not hard, but not
 * trivial.
 *
 * If this is done, an extension would be to make all ANYOFL nodes contain the
 * extra 32 bits that ANYOFPOSIXL ones do, doubling each instance's size.  The
 * posix flags only occupy 30 bits, so the ANYOFL_FOLD  and
 * ANYOFL_UTF8_LOCALE_REQD bits could be moved to that extra space, but it
 * would also mean extra instructions, as there are currently places in the
 * code that assume those two bits are zero.
 *
 * Some flags are not used in synthetic start class (SSC) nodes, so could be
 * shared should new flags be needed for SSCs, like SSC_MATCHES_EMPTY_STRING
 * now. */

/* If this is set, the result of the match should be complemented.  regexec.c
 * is expecting this to be in the low bit.  Never in an SSC */
#define ANYOF_INVERT		                0x01

/* For the SSC node only, which cannot be inverted, so is shared with that bit.
 * This is used only during regex compilation. */
#define SSC_MATCHES_EMPTY_STRING                ANYOF_INVERT

/* Set if this is a regnode_charclass_posixl vs a regnode_charclass.  This
 * is used for runtime \d, \w, [:posix:], ..., which are used only in locale
 * and the optimizer's synthetic start class.  Non-locale \d, etc are resolved
 * at compile-time.  Only set under /l; can be in SSC */
#define ANYOF_MATCHES_POSIXL                    0x02

/* The fold is calculated and stored in the bitmap where possible at compile
 * time.  However under locale, the actual folding varies depending on
 * what the locale is at the time of execution, so it has to be deferred until
 * then.  Only set under /l; never in an SSC  */
#define ANYOFL_FOLD                             0x04

/* Warn if the runtime locale isn't a UTF-8 one (and the generated node assumes
 * a UTF-8 locale. */
#define ANYOFL_UTF8_LOCALE_REQD                 0x08

/* Spare: Be sure to change ANYOF_FLAGS_ALL if this gets used  0x10 */

/* Spare: Be sure to change ANYOF_FLAGS_ALL if this gets used  0x20 */

/* Shared bit that indicates that there are potential additional matches stored
 * outside the bitmap, as pointed to by the AV given by the node's argument.
 * The node type is used at runtime (in conjunction with this flag and other
 * information available then) to decide if the flag should be acted upon.
 * This extra information is needed because of at least one of the following
 * three reasons.
 *      Under /d and the matched string is in UTF-8, it means the ANYOFD node
 *          matches more things than in the bitmap.  Those things will be any
 *          code point too high for the bitmap, but crucially, any non-ASCII
 *          characters that match iff when using Unicode rules.  These all are
 *          < 256.
 *
 *      Under /l and ANYOFL_FOLD is set, this flag may indicate there are
 *          potential matches valid only if the locale is a UTF-8 one.  If so,
 *          a list of them is stored in the AV.
 *
 *      For any non-ANYOFD node, there may be a user-defined property that
 *          wasn't yet defined at the time the regex was compiled, and so must
 *          be looked up at runtime, The information required to do so will
 *          also be in the AV.
 *
 *      Note that an ANYOFL node may contain both a user-defined property, and
 *      folds not always valid.  The important thing is that there is an AV to
 *      look at. */
#define ANYOF_HAS_EXTRA_RUNTIME_MATCHES 0x40

/* Shared bit:
 *      Under /d it means the ANYOFD node matches all non-ASCII Latin1
 *          characters when the target string is not in utf8.
 *      When not under /d, it means the ANYOF node should raise a warning if
 *          matching against an above-Unicode code point.
 * (These uses are mutually exclusive because the warning requires a \p{}, and
 * \p{} implies /u which deselects /d).  An SSC node only has this bit set if
 * what is meant is the warning.  The names are to make sure that you are
 * cautioned about its shared nature */
#define ANYOFD_NON_UTF8_MATCHES_ALL_NON_ASCII__shared 0x80
#define ANYOF_WARN_SUPER__shared                      0x80

#define ANYOF_FLAGS_ALL		((U8) ~(0x10|0x20))

#define ANYOF_LOCALE_FLAGS (  ANYOFL_FOLD               \
                            | ANYOF_MATCHES_POSIXL      \
                            | ANYOFL_UTF8_LOCALE_REQD)

/* These are the flags that apply to both regular ANYOF nodes and synthetic
 * start class nodes during construction of the SSC.  During finalization of
 * the SSC, other of the flags may get added to it */
#define ANYOF_COMMON_FLAGS      0

/* Character classes for node->classflags of ANYOF */
/* Should be synchronized with a table in regprop() */
/* 2n should be the normal one, paired with its complement at 2n+1 */

#define ANYOF_ALPHA    ((CC_ALPHA_) * 2)
#define ANYOF_NALPHA   ((ANYOF_ALPHA) + 1)
#define ANYOF_ALPHANUMERIC   ((CC_ALPHANUMERIC_) * 2)    /* [[:alnum:]] isalnum(3), utf8::IsAlnum */
#define ANYOF_NALPHANUMERIC  ((ANYOF_ALPHANUMERIC) + 1)
#define ANYOF_ASCII    ((CC_ASCII_) * 2)
#define ANYOF_NASCII   ((ANYOF_ASCII) + 1)
#define ANYOF_BLANK    ((CC_BLANK_) * 2)     /* GNU extension: space and tab: non-vertical space */
#define ANYOF_NBLANK   ((ANYOF_BLANK) + 1)
#define ANYOF_CASED    ((CC_CASED_) * 2)    /* Pseudo class for [:lower:] or
                                               [:upper:] under /i */
#define ANYOF_NCASED   ((ANYOF_CASED) + 1)
#define ANYOF_CNTRL    ((CC_CNTRL_) * 2)
#define ANYOF_NCNTRL   ((ANYOF_CNTRL) + 1)
#define ANYOF_DIGIT    ((CC_DIGIT_) * 2)     /* \d */
#define ANYOF_NDIGIT   ((ANYOF_DIGIT) + 1)
#define ANYOF_GRAPH    ((CC_GRAPH_) * 2)
#define ANYOF_NGRAPH   ((ANYOF_GRAPH) + 1)
#define ANYOF_LOWER    ((CC_LOWER_) * 2)
#define ANYOF_NLOWER   ((ANYOF_LOWER) + 1)
#define ANYOF_PRINT    ((CC_PRINT_) * 2)
#define ANYOF_NPRINT   ((ANYOF_PRINT) + 1)
#define ANYOF_PUNCT    ((CC_PUNCT_) * 2)
#define ANYOF_NPUNCT   ((ANYOF_PUNCT) + 1)
#define ANYOF_SPACE    ((CC_SPACE_) * 2)     /* \s */
#define ANYOF_NSPACE   ((ANYOF_SPACE) + 1)
#define ANYOF_UPPER    ((CC_UPPER_) * 2)
#define ANYOF_NUPPER   ((ANYOF_UPPER) + 1)
#define ANYOF_WORDCHAR ((CC_WORDCHAR_) * 2)  /* \w, PL_utf8_alnum, utf8::IsWord, ALNUM */
#define ANYOF_NWORDCHAR   ((ANYOF_WORDCHAR) + 1)
#define ANYOF_XDIGIT   ((CC_XDIGIT_) * 2)
#define ANYOF_NXDIGIT  ((ANYOF_XDIGIT) + 1)

/* pseudo classes below this, not stored in the class bitmap, but used as flags
   during compilation of char classes */

#define ANYOF_VERTWS    ((CC_VERTSPACE_) * 2)
#define ANYOF_NVERTWS   ((ANYOF_VERTWS)+1)

/* It is best if this is the last one, as all above it are stored as bits in a
 * bitmap, and it isn't part of that bitmap */
#if CC_VERTSPACE_ != HIGHEST_REGCOMP_DOT_H_SYNC_
#   error Problem with handy.h HIGHEST_REGCOMP_DOT_H_SYNC_ #define
#endif

#define ANYOF_POSIXL_MAX (ANYOF_VERTWS) /* So upper loop limit is written:
                                         *       '< ANYOF_MAX'
                                         * Hence doesn't include VERTWS, as that
                                         * is a pseudo class */
#define ANYOF_MAX      ANYOF_POSIXL_MAX

#if (ANYOF_POSIXL_MAX > 32)   /* Must fit in 32-bit word */
#   error Problem with handy.h CC_foo_ #defines
#endif

#define ANYOF_HORIZWS	((ANYOF_POSIXL_MAX)+2) /* = (ANYOF_NVERTWS + 1) */
#define ANYOF_NHORIZWS	((ANYOF_POSIXL_MAX)+3)

#define ANYOF_UNIPROP   ((ANYOF_POSIXL_MAX)+4)  /* Used to indicate a Unicode
                                                   property: \p{} or \P{} */

/* Backward source code compatibility. */

#define ANYOF_ALNUML	 ANYOF_ALNUM
#define ANYOF_NALNUML	 ANYOF_NALNUM
#define ANYOF_SPACEL	 ANYOF_SPACE
#define ANYOF_NSPACEL	 ANYOF_NSPACE
#define ANYOF_ALNUM ANYOF_WORDCHAR
#define ANYOF_NALNUM ANYOF_NWORDCHAR

/* Utility macros for the bitmap and classes of ANYOF */

#define BITMAP_BYTE(p, c)	(( (U8*) (p)) [ ( ( (UV) (c)) >> 3) ] )
#define BITMAP_BIT(c)	        (1U << ((c) & 7))
#define BITMAP_TEST(p, c)	(BITMAP_BYTE(p, c) & BITMAP_BIT((U8)(c)))

#define ANYOF_FLAGS(p)          (FLAGS(p))

#define ANYOF_BIT(c)		BITMAP_BIT(c)

#define ANYOF_POSIXL_BITMAP(p)  (((regnode_charclass_posixl*) (p))->classflags)

#define POSIXL_SET(field, c)	((field) |= (1U << (c)))
#define ANYOF_POSIXL_SET(p, c)	POSIXL_SET(ANYOF_POSIXL_BITMAP(p), (c))

#define POSIXL_CLEAR(field, c) ((field) &= ~ (1U <<(c)))
#define ANYOF_POSIXL_CLEAR(p, c) POSIXL_CLEAR(ANYOF_POSIXL_BITMAP(p), (c))

#define POSIXL_TEST(field, c)	((field) & (1U << (c)))
#define ANYOF_POSIXL_TEST(p, c)	POSIXL_TEST(ANYOF_POSIXL_BITMAP(p), (c))

#define POSIXL_ZERO(field)	STMT_START { (field) = 0; } STMT_END
#define ANYOF_POSIXL_ZERO(ret)	POSIXL_ZERO(ANYOF_POSIXL_BITMAP(ret))

#define ANYOF_POSIXL_SET_TO_BITMAP(p, bits)                                 \
                STMT_START { ANYOF_POSIXL_BITMAP(p) = (bits); } STMT_END

/* Shifts a bit to get, eg. 0x4000_0000, then subtracts 1 to get 0x3FFF_FFFF */
#define ANYOF_POSIXL_SETALL(ret)                                            \
                STMT_START {                                                \
                    ANYOF_POSIXL_BITMAP(ret) = nBIT_MASK(ANYOF_POSIXL_MAX); \
                } STMT_END
#define ANYOF_CLASS_SETALL(ret) ANYOF_POSIXL_SETALL(ret)

#define ANYOF_POSIXL_TEST_ANY_SET(p)                               \
        ((ANYOF_FLAGS(p) & ANYOF_MATCHES_POSIXL) && ANYOF_POSIXL_BITMAP(p))
#define ANYOF_CLASS_TEST_ANY_SET(p) ANYOF_POSIXL_TEST_ANY_SET(p)

/* Since an SSC always has this field, we don't have to test for that; nor do
 * we want to because the bit isn't set for SSC during its construction */
#define ANYOF_POSIXL_SSC_TEST_ANY_SET(p)                               \
                            cBOOL(((regnode_ssc*)(p))->classflags)
#define ANYOF_POSIXL_SSC_TEST_ALL_SET(p) /* Are all bits set? */       \
        (((regnode_ssc*) (p))->classflags                              \
                                        == nBIT_MASK(ANYOF_POSIXL_MAX))

#define ANYOF_POSIXL_TEST_ALL_SET(p)                                   \
        ((ANYOF_FLAGS(p) & ANYOF_MATCHES_POSIXL)                       \
         && ANYOF_POSIXL_BITMAP(p) == nBIT_MASK(ANYOF_POSIXL_MAX))

#define ANYOF_POSIXL_OR(source, dest) STMT_START { (dest)->classflags |= (source)->classflags ; } STMT_END
#define ANYOF_CLASS_OR(source, dest) ANYOF_POSIXL_OR((source), (dest))

#define ANYOF_POSIXL_AND(source, dest) STMT_START { (dest)->classflags &= (source)->classflags ; } STMT_END

#define ANYOF_BITMAP_ZERO(ret)	Zero(((regnode_charclass*)(ret))->bitmap, ANYOF_BITMAP_SIZE, char)
#define ANYOF_BITMAP(p)		((regnode_charclass*)(p))->bitmap
#define ANYOF_BITMAP_BYTE(p, c)	BITMAP_BYTE(ANYOF_BITMAP(p), c)
#define ANYOF_BITMAP_SET(p, c)	(ANYOF_BITMAP_BYTE(p, c) |=  ANYOF_BIT(c))
#define ANYOF_BITMAP_CLEAR(p,c)	(ANYOF_BITMAP_BYTE(p, c) &= ~ANYOF_BIT(c))
#define ANYOF_BITMAP_TEST(p, c)	cBOOL(ANYOF_BITMAP_BYTE(p, c) & ANYOF_BIT(c))

#define ANYOF_BITMAP_SETALL(p)		\
        memset (ANYOF_BITMAP(p), 255, ANYOF_BITMAP_SIZE)
#define ANYOF_BITMAP_CLEARALL(p)	\
        Zero (ANYOF_BITMAP(p), ANYOF_BITMAP_SIZE)

/*
 * Utility definitions.
 */
#ifndef CHARMASK
#  define UCHARAT(p)	((int)*(const U8*)(p))
#else
#  define UCHARAT(p)	((int)*(p)&CHARMASK)
#endif

/* Number of regnode equivalents that 'guy' occupies beyond the size of the
 * smallest regnode. */
#define EXTRA_SIZE(guy) ((sizeof(guy)-1)/sizeof(struct regnode))

#define REG_ZERO_LEN_SEEN                   0x00000001
#define REG_LOOKBEHIND_SEEN                 0x00000002
/* add a short form alias to keep the line length police happy */
#define REG_LB_SEEN                         REG_LOOKBEHIND_SEEN
#define REG_GPOS_SEEN                       0x00000004
/* spare */
#define REG_RECURSE_SEEN                    0x00000020
#define REG_TOP_LEVEL_BRANCHES_SEEN         0x00000040
#define REG_VERBARG_SEEN                    0x00000080
#define REG_CUTGROUP_SEEN                   0x00000100
#define REG_RUN_ON_COMMENT_SEEN             0x00000200
#define REG_UNFOLDED_MULTI_SEEN             0x00000400
/* spare */
#define REG_UNBOUNDED_QUANTIFIER_SEEN       0x00001000
#define REG_PESSIMIZE_SEEN                  0x00002000


START_EXTERN_C

#ifdef PLUGGABLE_RE_EXTENSION
#include "re_nodes.h"
#else
#include "regnodes.h"
#endif

#ifndef PLUGGABLE_RE_EXTENSION
#ifndef DOINIT
EXTCONST regexp_engine PL_core_reg_engine;
#else /* DOINIT */
EXTCONST regexp_engine PL_core_reg_engine = { 
        Perl_re_compile,
        Perl_regexec_flags,
        Perl_re_intuit_start,
        Perl_re_intuit_string, 
        Perl_regfree_internal,
        Perl_reg_numbered_buff_fetch,
        Perl_reg_numbered_buff_store,
        Perl_reg_numbered_buff_length,
        Perl_reg_named_buff,
        Perl_reg_named_buff_iter,
        Perl_reg_qr_package,
#if defined(USE_ITHREADS)        
        Perl_regdupe_internal,
#endif        
        Perl_re_op_compile
};
#endif /* DOINIT */
#endif /* PLUGGABLE_RE_EXTENSION */


END_EXTERN_C


/* .what is a character array with one character for each member of .data
 * The character describes the function of the corresponding .data item:
 *   a - AV for paren_name_list under DEBUGGING
 *   f - start-class data for regstclass optimization
 *   l - start op for literal (?{EVAL}) item
 *   L - start op for literal (?{EVAL}) item, with separate CV (qr//)
 *   r - pointer to an embedded code-containing qr, e.g. /ab$qr/
 *   s - inversion list for Unicode-style character class, and the
 *       multicharacter strings resulting from casefolding the single-character
 *       entries in the character class
 *   t - trie struct
 *   u - trie struct's widecharmap (a HV, so can't share, must dup)
 *       also used for revcharmap and words under DEBUGGING
 *   T - aho-trie struct
 *   S - sv for named capture lookup
 * 20010712 mjd@plover.com
 * (Remember to update re_dup() and pregfree() if you add any items.)
 */
struct reg_data {
    U32 count;
    U8 *what;
    void* data[1];
};

/* Code in S_to_utf8_substr() and S_to_byte_substr() in regexec.c accesses
   anchored* and float* via array indexes 0 and 1.  */
#define anchored_substr substrs->data[0].substr
#define anchored_utf8 substrs->data[0].utf8_substr
#define anchored_offset substrs->data[0].min_offset
#define anchored_end_shift substrs->data[0].end_shift

#define float_substr substrs->data[1].substr
#define float_utf8 substrs->data[1].utf8_substr
#define float_min_offset substrs->data[1].min_offset
#define float_max_offset substrs->data[1].max_offset
#define float_end_shift substrs->data[1].end_shift

#define check_substr substrs->data[2].substr
#define check_utf8 substrs->data[2].utf8_substr
#define check_offset_min substrs->data[2].min_offset
#define check_offset_max substrs->data[2].max_offset
#define check_end_shift substrs->data[2].end_shift

#define RX_ANCHORED_SUBSTR(rx)	(ReANY(rx)->anchored_substr)
#define RX_ANCHORED_UTF8(rx)	(ReANY(rx)->anchored_utf8)
#define RX_FLOAT_SUBSTR(rx)	(ReANY(rx)->float_substr)
#define RX_FLOAT_UTF8(rx)	(ReANY(rx)->float_utf8)

/* trie related stuff */

/* a transition record for the state machine. the
   check field determines which state "owns" the
   transition. the char the transition is for is
   determined by offset from the owning states base
   field.  the next field determines which state
   is to be transitioned to if any.
*/
struct _reg_trie_trans {
  U32 next;
  U32 check;
};

/* a transition list element for the list based representation */
struct _reg_trie_trans_list_elem {
    U16 forid;
    U32 newstate;
};
typedef struct _reg_trie_trans_list_elem reg_trie_trans_le;

/* a state for compressed nodes. base is an offset
  into an array of reg_trie_trans array. If wordnum is
  nonzero the state is accepting. if base is zero then
  the state has no children (and will be accepting)
*/
struct _reg_trie_state {
  U16 wordnum;
  union {
    U32                base;
    reg_trie_trans_le* list;
  } trans;
};

/* info per word; indexed by wordnum */
typedef struct {
    U16  prev;	/* previous word in acceptance chain; eg in
                 * zzz|abc|ab/ after matching the chars abc, the
                 * accepted word is #2, and the previous accepted
                 * word is #3 */
    U32 len;	/* how many chars long is this word? */
    U32 accept;	/* accept state for this word */
} reg_trie_wordinfo;


typedef struct _reg_trie_state    reg_trie_state;
typedef struct _reg_trie_trans    reg_trie_trans;


/* anything in here that needs to be freed later
   should be dealt with in pregfree.
   refcount is first in both this and _reg_ac_data to allow a space
   optimisation in Perl_regdupe.  */
struct _reg_trie_data {
    U32             refcount;        /* number of times this trie is referenced */
    U32             lasttrans;       /* last valid transition element */
    U16             *charmap;        /* byte to charid lookup array */
    reg_trie_state  *states;         /* state data */
    reg_trie_trans  *trans;          /* array of transition elements */
    char            *bitmap;         /* stclass bitmap */
    U16 	    *jump;           /* optional 1 indexed array of offsets before tail 
                                        for the node following a given word. */
    U16             *j_before_paren; /* optional 1 indexed array of parno reset data
                                        for the given jump. */
    U16             *j_after_paren;  /* optional 1 indexed array of parno reset data
                                        for the given jump. */

    reg_trie_wordinfo *wordinfo;     /* array of info per word */
    U16             uniquecharcount; /* unique chars in trie (width of trans table) */
    U32             startstate;      /* initial state - used for common prefix optimisation */
    STRLEN          minlen;          /* minimum length of words in trie - build/opt only? */
    STRLEN          maxlen;          /* maximum length of words in trie - build/opt only? */
    U32             prefixlen;       /* #chars in common prefix */
    U32             statecount;      /* Build only - number of states in the states array 
                                        (including the unused zero state) */
    U32             wordcount;       /* Build only */
    U16             before_paren;
    U16             after_paren;
#ifdef DEBUGGING
    STRLEN          charcount;       /* Build only */
#endif
};
/* There is one (3 under DEBUGGING) pointers that logically belong in this
   structure, but are held outside as they need duplication on thread cloning,
   whereas the rest of the structure can be read only:
    HV              *widecharmap;    code points > 255 to charid
#ifdef DEBUGGING
    AV              *words;          Array of words contained in trie, for dumping
    AV              *revcharmap;     Map of each charid back to its character representation
#endif
*/

#define TRIE_WORDS_OFFSET 2

typedef struct _reg_trie_data reg_trie_data;

/* refcount is first in both this and _reg_trie_data to allow a space
   optimisation in Perl_regdupe.  */
struct _reg_ac_data {
    U32              refcount;
    U32              trie;
    U32              *fail;
    reg_trie_state   *states;
};
typedef struct _reg_ac_data reg_ac_data;

/* ANY_BIT doesn't use the structure, so we can borrow it here.
   This is simpler than refactoring all of it as wed end up with
   three different sets... */

#define TRIE_BITMAP(p)		(((reg_trie_data *)(p))->bitmap)
#define TRIE_BITMAP_BYTE(p, c)	BITMAP_BYTE(TRIE_BITMAP(p), c)
#define TRIE_BITMAP_SET(p, c)	(TRIE_BITMAP_BYTE(p, c) |=  ANYOF_BIT((U8)c))
#define TRIE_BITMAP_CLEAR(p,c)	(TRIE_BITMAP_BYTE(p, c) &= ~ANYOF_BIT((U8)c))
#define TRIE_BITMAP_TEST(p, c)	(TRIE_BITMAP_BYTE(p, c) &   ANYOF_BIT((U8)c))

#define IS_ANYOF_TRIE(op) ((op)==TRIEC || (op)==AHOCORASICKC)
#define IS_TRIE_AC(op) ((op)>=AHOCORASICK)

/* these defines assume uniquecharcount is the correct variable, and state may be evaluated twice */
#define TRIE_NODENUM(state) (((state)-1)/(trie->uniquecharcount)+1)
#define SAFE_TRIE_NODENUM(state) ((state) ? (((state)-1)/(trie->uniquecharcount)+1) : (state))
#define TRIE_NODEIDX(state) ((state) ? (((state)-1)*(trie->uniquecharcount)+1) : (state))

#ifdef DEBUGGING
#define TRIE_CHARCOUNT(trie) ((trie)->charcount)
#else
#define TRIE_CHARCOUNT(trie) (trie_charcount)
#endif

#define RE_TRIE_MAXBUF_INIT 65536
#define RE_TRIE_MAXBUF_NAME "\022E_TRIE_MAXBUF"
#define RE_DEBUG_FLAGS "\022E_DEBUG_FLAGS"

#define RE_COMPILE_RECURSION_INIT 1000
#define RE_COMPILE_RECURSION_LIMIT "\022E_COMPILE_RECURSION_LIMIT"

/*

RE_DEBUG_FLAGS is used to control what debug output is emitted
its divided into three groups of options, some of which interact.
The three groups are: Compile, Execute, Extra. There is room for a
further group, as currently only the low three bytes are used.

    Compile Options:
    
    PARSE
    PEEP
    TRIE
    PROGRAM

    Execute Options:

    INTUIT
    MATCH
    TRIE

    Extra Options

    TRIE

If you modify any of these make sure you make corresponding changes to
re.pm, especially to the documentation.

*/


/* Compile */
#define RE_DEBUG_COMPILE_MASK      0x0000FF
#define RE_DEBUG_COMPILE_PARSE     0x000001
#define RE_DEBUG_COMPILE_OPTIMISE  0x000002
#define RE_DEBUG_COMPILE_TRIE      0x000004
#define RE_DEBUG_COMPILE_DUMP      0x000008
#define RE_DEBUG_COMPILE_FLAGS     0x000010
#define RE_DEBUG_COMPILE_TEST      0x000020

/* Execute */
#define RE_DEBUG_EXECUTE_MASK      0x00FF00
#define RE_DEBUG_EXECUTE_INTUIT    0x000100
#define RE_DEBUG_EXECUTE_MATCH     0x000200
#define RE_DEBUG_EXECUTE_TRIE      0x000400

/* Extra */
#define RE_DEBUG_EXTRA_MASK              0x3FF0000
#define RE_DEBUG_EXTRA_TRIE              0x0010000
#define RE_DEBUG_EXTRA_STATE             0x0080000
#define RE_DEBUG_EXTRA_OPTIMISE          0x0100000
#define RE_DEBUG_EXTRA_BUFFERS           0x0400000
#define RE_DEBUG_EXTRA_GPOS              0x0800000
#define RE_DEBUG_EXTRA_DUMP_PRE_OPTIMIZE 0x1000000
#define RE_DEBUG_EXTRA_WILDCARD          0x2000000
/* combined */
#define RE_DEBUG_EXTRA_STACK             0x0280000

#define RE_DEBUG_FLAG(x) (re_debug_flags & (x))
/* Compile */
#define DEBUG_COMPILE_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_COMPILE_MASK)) x  )
#define DEBUG_PARSE_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_COMPILE_PARSE)) x  )
#define DEBUG_OPTIMISE_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_COMPILE_OPTIMISE)) x  )
#define DEBUG_DUMP_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_COMPILE_DUMP)) x  )
#define DEBUG_TRIE_COMPILE_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_COMPILE_TRIE)) x )
#define DEBUG_FLAGS_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_COMPILE_FLAGS)) x )
#define DEBUG_TEST_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_COMPILE_TEST)) x )
/* Execute */
#define DEBUG_EXECUTE_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_EXECUTE_MASK)) x  )
#define DEBUG_INTUIT_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_EXECUTE_INTUIT)) x  )
#define DEBUG_MATCH_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_EXECUTE_MATCH)) x  )
#define DEBUG_TRIE_EXECUTE_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_EXECUTE_TRIE)) x )

/* Extra */
#define DEBUG_EXTRA_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_EXTRA_MASK)) x  )
#define DEBUG_STATE_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_EXTRA_STATE)) x )
#define DEBUG_STACK_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_EXTRA_STACK)) x )
#define DEBUG_BUFFERS_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_EXTRA_BUFFERS)) x )

#define DEBUG_OPTIMISE_MORE_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || ((RE_DEBUG_EXTRA_OPTIMISE|RE_DEBUG_COMPILE_OPTIMISE) == \
         RE_DEBUG_FLAG(RE_DEBUG_EXTRA_OPTIMISE|RE_DEBUG_COMPILE_OPTIMISE))) x )
#define DEBUG_TRIE_COMPILE_MORE_r(x) DEBUG_TRIE_COMPILE_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_EXTRA_TRIE)) x )
#define DEBUG_TRIE_EXECUTE_MORE_r(x) DEBUG_TRIE_EXECUTE_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_EXTRA_TRIE)) x )

#define DEBUG_TRIE_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_COMPILE_TRIE \
        | RE_DEBUG_EXECUTE_TRIE )) x )
#define DEBUG_GPOS_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_EXTRA_GPOS)) x )

#define DEBUG_DUMP_PRE_OPTIMIZE_r(x) DEBUG_r( \
    if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_EXTRA_DUMP_PRE_OPTIMIZE)) x )

/* initialization */
/* Get the debug flags for code not in regcomp.c nor regexec.c.  This doesn't
 * initialize the variable if it isn't already there, instead it just assumes
 * the flags are 0 */
#define DECLARE_AND_GET_RE_DEBUG_FLAGS_NON_REGEX                               \
    volatile IV re_debug_flags = 0;  PERL_UNUSED_VAR(re_debug_flags);          \
    STMT_START {                                                               \
        SV * re_debug_flags_sv = NULL;                                         \
                     /* get_sv() can return NULL during global destruction. */ \
        re_debug_flags_sv = PL_curcop ? get_sv(RE_DEBUG_FLAGS, GV_ADD) : NULL; \
        if (re_debug_flags_sv && SvIOK(re_debug_flags_sv))                     \
            re_debug_flags=SvIV(re_debug_flags_sv);                            \
    } STMT_END


#ifdef DEBUGGING

/* For use in regcomp.c and regexec.c,  Get the debug flags, and initialize to
 * the defaults if not done already */
#define DECLARE_AND_GET_RE_DEBUG_FLAGS                                         \
    volatile IV re_debug_flags = 0;  PERL_UNUSED_VAR(re_debug_flags);          \
    DEBUG_r({                              \
        SV * re_debug_flags_sv = NULL;                                         \
                     /* get_sv() can return NULL during global destruction. */ \
        re_debug_flags_sv = PL_curcop ? get_sv(RE_DEBUG_FLAGS, GV_ADD) : NULL; \
        if (re_debug_flags_sv) {                                               \
            if (!SvIOK(re_debug_flags_sv)) /* If doesn't exist set to default */\
                sv_setuv(re_debug_flags_sv,                                    \
                        /* These defaults should be kept in sync with re.pm */ \
                            RE_DEBUG_COMPILE_DUMP | RE_DEBUG_EXECUTE_MASK );   \
            re_debug_flags=SvIV(re_debug_flags_sv);                            \
        }                                                                      \
    })

#define isDEBUG_WILDCARD (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_EXTRA_WILDCARD))

#define RE_PV_COLOR_DECL(rpv,rlen,isuni,dsv,pv,l,m,c1,c2)   \
    const char * const rpv =                                \
        pv_pretty((dsv), (pv), (l), (m),                    \
            PL_colors[(c1)],PL_colors[(c2)],                \
            PERL_PV_ESCAPE_RE|PERL_PV_ESCAPE_NONASCII |((isuni) ? PERL_PV_ESCAPE_UNI : 0) );         \
    const int rlen = SvCUR(dsv)

/* This is currently unsed in the core */
#define RE_SV_ESCAPE(rpv,isuni,dsv,sv,m)                            \
    const char * const rpv =                                        \
        pv_pretty((dsv), (SvPV_nolen_const(sv)), (SvCUR(sv)), (m),  \
            PL_colors[(c1)],PL_colors[(c2)],                        \
            PERL_PV_ESCAPE_RE|PERL_PV_ESCAPE_NONASCII |((isuni) ? PERL_PV_ESCAPE_UNI : 0) )

#define RE_PV_QUOTED_DECL(rpv,isuni,dsv,pv,l,m)                     \
    const char * const rpv =                                        \
        pv_pretty((dsv), (pv), (l), (m),                            \
            PL_colors[0], PL_colors[1],                             \
            ( PERL_PV_PRETTY_QUOTE | PERL_PV_ESCAPE_RE | PERL_PV_ESCAPE_NONASCII | PERL_PV_PRETTY_ELLIPSES | \
              ((isuni) ? PERL_PV_ESCAPE_UNI : 0))                  \
        )

#define RE_SV_DUMPLEN(ItEm) (SvCUR(ItEm) - (SvTAIL(ItEm)!=0))
#define RE_SV_TAIL(ItEm) (SvTAIL(ItEm) ? "$" : "")
    
#else /* if not DEBUGGING */

#define DECLARE_AND_GET_RE_DEBUG_FLAGS  dNOOP
#define RE_PV_COLOR_DECL(rpv,rlen,isuni,dsv,pv,l,m,c1,c2)  dNOOP
#define RE_SV_ESCAPE(rpv,isuni,dsv,sv,m)
#define RE_PV_QUOTED_DECL(rpv,isuni,dsv,pv,l,m)  dNOOP
#define RE_SV_DUMPLEN(ItEm)
#define RE_SV_TAIL(ItEm)
#define isDEBUG_WILDCARD 0

#endif /* DEBUG RELATED DEFINES */

#define FIRST_NON_ASCII_DECIMAL_DIGIT 0x660  /* ARABIC_INDIC_DIGIT_ZERO */

typedef enum {
        TRADITIONAL_BOUND = CC_WORDCHAR_,
        GCB_BOUND,
        LB_BOUND,
        SB_BOUND,
        WB_BOUND
} bound_type;

/* This unpacks the FLAGS field of ANYOF[HR]x nodes.  The value it contains
 * gives the strict lower bound for the UTF-8 start byte of any code point
 * matchable by the node, and a loose upper bound as well.
 *
 * The low bound is stored as 0xC0 + ((the upper 6 bits) >> 2)
 * The loose upper bound is determined from the lowest 2 bits and the low bound
 * (called x) as follows:
 *
 * 11  The upper limit of the range can be as much as (EF - x) / 8
 * 10  The upper limit of the range can be as much as (EF - x) / 4
 * 01  The upper limit of the range can be as much as (EF - x) / 2
 * 00  The upper limit of the range can be as much as  EF
 *
 * For motivation of this design, see commit message in
 * 3146c00a633e9cbed741e10146662fbcedfdb8d3 */
#ifdef EBCDIC
#  define MAX_ANYOF_HRx_BYTE  0xF4
#else
#  define MAX_ANYOF_HRx_BYTE  0xEF
#endif
#define LOWEST_ANYOF_HRx_BYTE(b) (((b) >> 2) + 0xC0)
#define HIGHEST_ANYOF_HRx_BYTE(b)                                           \
                                  (LOWEST_ANYOF_HRx_BYTE(b)                 \
          + ((MAX_ANYOF_HRx_BYTE - LOWEST_ANYOF_HRx_BYTE(b)) >> ((b) & 3)))

#if !defined(PERL_IN_XSUB_RE) || defined(PLUGGABLE_RE_EXTENSION)
#  define GET_REGCLASS_AUX_DATA(a,b,c,d,e,f)  get_regclass_aux_data(a,b,c,d,e,f)
#else
#  define GET_REGCLASS_AUX_DATA(a,b,c,d,e,f)  get_re_gclass_aux_data(a,b,c,d,e,f)
#endif

#define REGNODE_TYPE(node)              (PL_regnode_info[(node)].type)
#define REGNODE_OFF_BY_ARG(node)        (PL_regnode_info[(node)].off_by_arg)
#define REGNODE_ARG_LEN(node)           (PL_regnode_info[(node)].arg_len)
#define REGNODE_ARG_LEN_VARIES(node)    (PL_regnode_info[(node)].arg_len_varies)
#define REGNODE_NAME(node)              (PL_regnode_name[(node)])

#if defined(PERL_IN_REGEX_ENGINE)
#include "reginline.h"
#endif

#define EVAL_OPTIMISTIC_FLAG    128
#define EVAL_FLAGS_MASK         (EVAL_OPTIMISTIC_FLAG-1)



#endif /* PERL_REGCOMP_H_ */

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
