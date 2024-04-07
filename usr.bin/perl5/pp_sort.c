/*    pp_sort.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 *   ...they shuffled back towards the rear of the line.  'No, not at the
 *   rear!' the slave-driver shouted.  'Three files up. And stay there...
 *
 *     [p.931 of _The Lord of the Rings_, VI/ii: "The Land of Shadow"]
 */

/* This file contains pp ("push/pop") functions that
 * execute the opcodes that make up a perl program. A typical pp function
 * expects to find its arguments on the stack, and usually pushes its
 * results onto the stack, hence the 'pp' terminology. Each OP structure
 * contains a pointer to the relevant pp_foo() function.
 *
 * This particular file just contains pp_sort(), which is complex
 * enough to merit its own file! See the other pp*.c files for the rest of
 * the pp_ functions.
 */

#include "EXTERN.h"
#define PERL_IN_PP_SORT_C
#include "perl.h"

#ifndef SMALLSORT
#define SMALLSORT (200)
#endif

/*
 * The mergesort implementation is by Peter M. Mcilroy <pmcilroy@lucent.com>.
 *
 * The original code was written in conjunction with BSD Computer Software
 * Research Group at University of California, Berkeley.
 *
 * See also: "Optimistic Sorting and Information Theoretic Complexity"
 *           Peter McIlroy
 *           SODA (Fourth Annual ACM-SIAM Symposium on Discrete Algorithms),
 *           pp 467-474, Austin, Texas, 25-27 January 1993.
 *
 * The integration to Perl is by John P. Linderman <jpl.jpl@gmail.com>.
 *
 * The code can be distributed under the same terms as Perl itself.
 *
 */


typedef char * aptr;            /* pointer for arithmetic on sizes */
typedef SV * gptr;              /* pointers in our lists */

/* Binary merge internal sort, with a few special mods
** for the special perl environment it now finds itself in.
**
** Things that were once options have been hotwired
** to values suitable for this use.  In particular, we'll always
** initialize looking for natural runs, we'll always produce stable
** output, and we'll always do Peter McIlroy's binary merge.
*/

/* Pointer types for arithmetic and storage and convenience casts */

#define APTR(P) ((aptr)(P))
#define GPTP(P) ((gptr *)(P))
#define GPPP(P) ((gptr **)(P))


/* byte offset from pointer P to (larger) pointer Q */
#define BYTEOFF(P, Q) (APTR(Q) - APTR(P))

#define PSIZE sizeof(gptr)

/* If PSIZE is power of 2, make PSHIFT that power, if that helps */

#ifdef  PSHIFT
#define PNELEM(P, Q)    (BYTEOFF(P,Q) >> (PSHIFT))
#define PNBYTE(N)       ((N) << (PSHIFT))
#define PINDEX(P, N)    (GPTP(APTR(P) + PNBYTE(N)))
#else
/* Leave optimization to compiler */
#define PNELEM(P, Q)    (GPTP(Q) - GPTP(P))
#define PNBYTE(N)       ((N) * (PSIZE))
#define PINDEX(P, N)    (GPTP(P) + (N))
#endif

/* Pointer into other corresponding to pointer into this */
#define POTHER(P, THIS, OTHER) GPTP(APTR(OTHER) + BYTEOFF(THIS,P))

#define FROMTOUPTO(src, dst, lim) do *dst++ = *src++; while(src<lim)


/* Runs are identified by a pointer in the auxiliary list.
** The pointer is at the start of the list,
** and it points to the start of the next list.
** NEXT is used as an lvalue, too.
*/

#define NEXT(P)         (*GPPP(P))


/* PTHRESH is the minimum number of pairs with the same sense to justify
** checking for a run and extending it.  Note that PTHRESH counts PAIRS,
** not just elements, so PTHRESH == 8 means a run of 16.
*/

#define PTHRESH (8)

/* RTHRESH is the number of elements in a run that must compare low
** to the low element from the opposing run before we justify
** doing a binary rampup instead of single stepping.
** In random input, N in a row low should only happen with
** probability 2^(1-N), so we can risk that we are dealing
** with orderly input without paying much when we aren't.
*/

#define RTHRESH (6)


/*
** Overview of algorithm and variables.
** The array of elements at list1 will be organized into runs of length 2,
** or runs of length >= 2 * PTHRESH.  We only try to form long runs when
** PTHRESH adjacent pairs compare in the same way, suggesting overall order.
**
** Unless otherwise specified, pair pointers address the first of two elements.
**
** b and b+1 are a pair that compare with sense "sense".
** b is the "bottom" of adjacent pairs that might form a longer run.
**
** p2 parallels b in the list2 array, where runs are defined by
** a pointer chain.
**
** t represents the "top" of the adjacent pairs that might extend
** the run beginning at b.  Usually, t addresses a pair
** that compares with opposite sense from (b,b+1).
** However, it may also address a singleton element at the end of list1,
** or it may be equal to "last", the first element beyond list1.
**
** r addresses the Nth pair following b.  If this would be beyond t,
** we back it off to t.  Only when r is less than t do we consider the
** run long enough to consider checking.
**
** q addresses a pair such that the pairs at b through q already form a run.
** Often, q will equal b, indicating we only are sure of the pair itself.
** However, a search on the previous cycle may have revealed a longer run,
** so q may be greater than b.
**
** p is used to work back from a candidate r, trying to reach q,
** which would mean b through r would be a run.  If we discover such a run,
** we start q at r and try to push it further towards t.
** If b through r is NOT a run, we detect the wrong order at (p-1,p).
** In any event, after the check (if any), we have two main cases.
**
** 1) Short run.  b <= q < p <= r <= t.
**      b through q is a run (perhaps trivial)
**      q through p are uninteresting pairs
**      p through r is a run
**
** 2) Long run.  b < r <= q < t.
**      b through q is a run (of length >= 2 * PTHRESH)
**
** Note that degenerate cases are not only possible, but likely.
** For example, if the pair following b compares with opposite sense,
** then b == q < p == r == t.
*/


PERL_STATIC_FORCE_INLINE IV __attribute__always_inline__
dynprep(pTHX_ gptr *list1, gptr *list2, size_t nmemb, const SVCOMPARE_t cmp)
{
    I32 sense;
    gptr *b, *p, *q, *t, *p2;
    gptr *last, *r;
    IV runs = 0;

    b = list1;
    last = PINDEX(b, nmemb);
    sense = (cmp(aTHX_ *b, *(b+1)) > 0);
    for (p2 = list2; b < last; ) {
        /* We just started, or just reversed sense.
        ** Set t at end of pairs with the prevailing sense.
        */
        for (p = b+2, t = p; ++p < last; t = ++p) {
            if ((cmp(aTHX_ *t, *p) > 0) != sense) break;
        }
        q = b;
        /* Having laid out the playing field, look for long runs */
        do {
            p = r = b + (2 * PTHRESH);
            if (r >= t) p = r = t;      /* too short to care about */
            else {
                while (((cmp(aTHX_ *(p-1), *p) > 0) == sense) &&
                       ((p -= 2) > q)) {}
                if (p <= q) {
                    /* b through r is a (long) run.
                    ** Extend it as far as possible.
                    */
                    p = q = r;
                    while (((p += 2) < t) &&
                           ((cmp(aTHX_ *(p-1), *p) > 0) == sense)) q = p;
                    r = p = q + 2;      /* no simple pairs, no after-run */
                }
            }
            if (q > b) {                /* run of greater than 2 at b */
                gptr *savep = p;

                p = q += 2;
                /* pick up singleton, if possible */
                if ((p == t) &&
                    ((t + 1) == last) &&
                    ((cmp(aTHX_ *(p-1), *p) > 0) == sense))
                    savep = r = p = q = last;
                p2 = NEXT(p2) = p2 + (p - b); ++runs;
                if (sense)
                    while (b < --p) {
                        const gptr c = *b;
                        *b++ = *p;
                        *p = c;
                    }
                p = savep;
            }
            while (q < p) {             /* simple pairs */
                p2 = NEXT(p2) = p2 + 2; ++runs;
                if (sense) {
                    const gptr c = *q++;
                    *(q-1) = *q;
                    *q++ = c;
                } else q += 2;
            }
            if (((b = p) == t) && ((t+1) == last)) {
                NEXT(p2) = p2 + 1; ++runs;
                b++;
            }
            q = r;
        } while (b < t);
        sense = !sense;
    }
    return runs;
}


/* The original merge sort, in use since 5.7, was as fast as, or faster than,
 * qsort on many platforms, but slower than qsort, conspicuously so,
 * on others.  The most likely explanation was platform-specific
 * differences in cache sizes and relative speeds.
 *
 * The quicksort divide-and-conquer algorithm guarantees that, as the
 * problem is subdivided into smaller and smaller parts, the parts
 * fit into smaller (and faster) caches.  So it doesn't matter how
 * many levels of cache exist, quicksort will "find" them, and,
 * as long as smaller is faster, take advantage of them.
 *
 * By contrast, consider how the original mergesort algorithm worked.
 * Suppose we have five runs (each typically of length 2 after dynprep).
 * 
 * pass               base                        aux
 *  0              1 2 3 4 5
 *  1                                           12 34 5
 *  2                1234 5
 *  3                                            12345
 *  4                 12345
 *
 * Adjacent pairs are merged in "grand sweeps" through the input.
 * This means, on pass 1, the records in runs 1 and 2 aren't revisited until
 * runs 3 and 4 are merged and the runs from run 5 have been copied.
 * The only cache that matters is one large enough to hold *all* the input.
 * On some platforms, this may be many times slower than smaller caches.
 *
 * The following pseudo-code uses the same basic merge algorithm,
 * but in a divide-and-conquer way.
 *
 * # merge $runs runs at offset $offset of list $list1 into $list2.
 * # all unmerged runs ($runs == 1) originate in list $base.
 * sub mgsort2 {
 *     my ($offset, $runs, $base, $list1, $list2) = @_;
 *
 *     if ($runs == 1) {
 *         if ($list1 is $base) copy run to $list2
 *         return offset of end of list (or copy)
 *     } else {
 *         $off2 = mgsort2($offset, $runs-($runs/2), $base, $list2, $list1)
 *         mgsort2($off2, $runs/2, $base, $list2, $list1)
 *         merge the adjacent runs at $offset of $list1 into $list2
 *         return the offset of the end of the merged runs
 *     }
 * }
 * mgsort2(0, $runs, $base, $aux, $base);
 *
 * For our 5 runs, the tree of calls looks like 
 *
 *           5
 *      3        2
 *   2     1   1   1
 * 1   1
 *
 * 1   2   3   4   5
 *
 * and the corresponding activity looks like
 *
 * copy runs 1 and 2 from base to aux
 * merge runs 1 and 2 from aux to base
 * (run 3 is where it belongs, no copy needed)
 * merge runs 12 and 3 from base to aux
 * (runs 4 and 5 are where they belong, no copy needed)
 * merge runs 4 and 5 from base to aux
 * merge runs 123 and 45 from aux to base
 *
 * Note that we merge runs 1 and 2 immediately after copying them,
 * while they are still likely to be in fast cache.  Similarly,
 * run 3 is merged with run 12 while it still may be lingering in cache.
 * This implementation should therefore enjoy much of the cache-friendly
 * behavior that quicksort does.  In addition, it does less copying
 * than the original mergesort implementation (only runs 1 and 2 are copied)
 * and the "balancing" of merges is better (merged runs comprise more nearly
 * equal numbers of original runs).
 *
 * The actual cache-friendly implementation will use a pseudo-stack
 * to avoid recursion, and will unroll processing of runs of length 2,
 * but it is otherwise similar to the recursive implementation.
 */

typedef struct {
    IV  offset;         /* offset of 1st of 2 runs at this level */
    IV  runs;           /* how many runs must be combined into 1 */
} off_runs;             /* pseudo-stack element */

PERL_STATIC_FORCE_INLINE void
S_sortsv_flags_impl(pTHX_ gptr *base, size_t nmemb, SVCOMPARE_t cmp, U32 flags)
{
    IV i, run, offset;
    I32 sense, level;
    gptr *f1, *f2, *t, *b, *p;
    int iwhich;
    gptr *aux;
    gptr *p1;
    gptr small[SMALLSORT];
    gptr *which[3];
    off_runs stack[60], *stackp;

    PERL_UNUSED_ARG(flags);
    PERL_ARGS_ASSERT_SORTSV_FLAGS_IMPL;
    if (nmemb <= 1) return;                     /* sorted trivially */

    if (nmemb <= SMALLSORT) aux = small;        /* use stack for aux array */
    else { Newx(aux,nmemb,gptr); }              /* allocate auxiliary array */
    level = 0;
    stackp = stack;
    stackp->runs = dynprep(aTHX_ base, aux, nmemb, cmp);
    stackp->offset = offset = 0;
    which[0] = which[2] = base;
    which[1] = aux;
    for (;;) {
        /* On levels where both runs have be constructed (stackp->runs == 0),
         * merge them, and note the offset of their end, in case the offset
         * is needed at the next level up.  Hop up a level, and,
         * as long as stackp->runs is 0, keep merging.
         */
        IV runs = stackp->runs;
        if (runs == 0) {
            gptr *list1, *list2;
            iwhich = level & 1;
            list1 = which[iwhich];              /* area where runs are now */
            list2 = which[++iwhich];            /* area for merged runs */
            do {
                gptr *l1, *l2, *tp2;
                offset = stackp->offset;
                f1 = p1 = list1 + offset;               /* start of first run */
                p = tp2 = list2 + offset;       /* where merged run will go */
                t = NEXT(p);                    /* where first run ends */
                f2 = l1 = POTHER(t, list2, list1); /* ... on the other side */
                t = NEXT(t);                    /* where second runs ends */
                l2 = POTHER(t, list2, list1);   /* ... on the other side */
                offset = PNELEM(list2, t);
                while (f1 < l1 && f2 < l2) {
                    /* If head 1 is larger than head 2, find ALL the elements
                    ** in list 2 strictly less than head1, write them all,
                    ** then head 1.  Then compare the new heads, and repeat,
                    ** until one or both lists are exhausted.
                    **
                    ** In all comparisons (after establishing
                    ** which head to merge) the item to merge
                    ** (at pointer q) is the first operand of
                    ** the comparison.  When we want to know
                    ** if "q is strictly less than the other",
                    ** we can't just do
                    **    cmp(q, other) < 0
                    ** because stability demands that we treat equality
                    ** as high when q comes from l2, and as low when
                    ** q was from l1.  So we ask the question by doing
                    **    cmp(q, other) <= sense
                    ** and make sense == 0 when equality should look low,
                    ** and -1 when equality should look high.
                    */

                    gptr *q;
                    if (cmp(aTHX_ *f1, *f2) <= 0) {
                        q = f2; b = f1; t = l1;
                        sense = -1;
                    } else {
                        q = f1; b = f2; t = l2;
                        sense = 0;
                    }


                    /* ramp up
                    **
                    ** Leave t at something strictly
                    ** greater than q (or at the end of the list),
                    ** and b at something strictly less than q.
                    */
                    for (i = 1, run = 0 ;;) {
                        if ((p = PINDEX(b, i)) >= t) {
                            /* off the end */
                            if (((p = PINDEX(t, -1)) > b) &&
                                (cmp(aTHX_ *q, *p) <= sense))
                                 t = p;
                            else b = p;
                            break;
                        } else if (cmp(aTHX_ *q, *p) <= sense) {
                            t = p;
                            break;
                        } else b = p;
                        if (++run >= RTHRESH) i += i;
                    }


                    /* q is known to follow b and must be inserted before t.
                    ** Increment b, so the range of possibilities is [b,t).
                    ** Round binary split down, to favor early appearance.
                    ** Adjust b and t until q belongs just before t.
                    */

                    b++;
                    while (b < t) {
                        p = PINDEX(b, (PNELEM(b, t) - 1) / 2);
                        if (cmp(aTHX_ *q, *p) <= sense) {
                            t = p;
                        } else b = p + 1;
                    }


                    /* Copy all the strictly low elements */

                    if (q == f1) {
                        FROMTOUPTO(f2, tp2, t);
                        *tp2++ = *f1++;
                    } else {
                        FROMTOUPTO(f1, tp2, t);
                        *tp2++ = *f2++;
                    }
                }


                /* Run out remaining list */
                if (f1 == l1) {
                       if (f2 < l2) FROMTOUPTO(f2, tp2, l2);
                } else              FROMTOUPTO(f1, tp2, l1);
                p1 = NEXT(p1) = POTHER(tp2, list2, list1);

                if (--level == 0) goto done;
                --stackp;
                t = list1; list1 = list2; list2 = t;    /* swap lists */
            } while ((runs = stackp->runs) == 0);
        }


        stackp->runs = 0;               /* current run will finish level */
        /* While there are more than 2 runs remaining,
         * turn them into exactly 2 runs (at the "other" level),
         * each made up of approximately half the runs.
         * Stack the second half for later processing,
         * and set about producing the first half now.
         */
        while (runs > 2) {
            ++level;
            ++stackp;
            stackp->offset = offset;
            runs -= stackp->runs = runs / 2;
        }
        /* We must construct a single run from 1 or 2 runs.
         * All the original runs are in which[0] == base.
         * The run we construct must end up in which[level&1].
         */
        iwhich = level & 1;
        if (runs == 1) {
            /* Constructing a single run from a single run.
             * If it's where it belongs already, there's nothing to do.
             * Otherwise, copy it to where it belongs.
             * A run of 1 is either a singleton at level 0,
             * or the second half of a split 3.  In neither event
             * is it necessary to set offset.  It will be set by the merge
             * that immediately follows.
             */
            if (iwhich) {       /* Belongs in aux, currently in base */
                f1 = b = PINDEX(base, offset);  /* where list starts */
                f2 = PINDEX(aux, offset);       /* where list goes */
                t = NEXT(f2);                   /* where list will end */
                offset = PNELEM(aux, t);        /* offset thereof */
                t = PINDEX(base, offset);       /* where it currently ends */
                FROMTOUPTO(f1, f2, t);          /* copy */
                NEXT(b) = t;                    /* set up parallel pointer */
            } else if (level == 0) goto done;   /* single run at level 0 */
        } else {
            /* Constructing a single run from two runs.
             * The merge code at the top will do that.
             * We need only make sure the two runs are in the "other" array,
             * so they'll end up in the correct array after the merge.
             */
            ++level;
            ++stackp;
            stackp->offset = offset;
            stackp->runs = 0;   /* take care of both runs, trigger merge */
            if (!iwhich) {      /* Merged runs belong in aux, copy 1st */
                f1 = b = PINDEX(base, offset);  /* where first run starts */
                f2 = PINDEX(aux, offset);       /* where it will be copied */
                t = NEXT(f2);                   /* where first run will end */
                offset = PNELEM(aux, t);        /* offset thereof */
                p = PINDEX(base, offset);       /* end of first run */
                t = NEXT(t);                    /* where second run will end */
                t = PINDEX(base, PNELEM(aux, t)); /* where it now ends */
                FROMTOUPTO(f1, f2, t);          /* copy both runs */
                NEXT(b) = p;                    /* paralleled pointer for 1st */
                NEXT(p) = t;                    /* ... and for second */
            }
        }
    }
  done:
    if (aux != small) Safefree(aux);    /* free iff allocated */

    return;
}

/*
=for apidoc sortsv_flags

In-place sort an array of SV pointers with the given comparison routine,
with various SORTf_* flag options.

=cut
*/
void
Perl_sortsv_flags(pTHX_ gptr *base, size_t nmemb, SVCOMPARE_t cmp, U32 flags)
{
    PERL_ARGS_ASSERT_SORTSV_FLAGS;

    sortsv_flags_impl(base, nmemb, cmp, flags);
}

/*
 * Each of sortsv_* functions contains an inlined copy of
 * sortsv_flags_impl() with an inlined comparator. Basically, we are
 * emulating C++ templates by using __attribute__((always_inline)).
 *
 * The purpose of that is to avoid the function call overhead inside
 * the sorting routine, which calls the comparison function multiple
 * times per sorted item.
 */

static void
sortsv_amagic_i_ncmp(pTHX_ gptr *base, size_t nmemb, U32 flags)
{
    sortsv_flags_impl(base, nmemb, S_amagic_i_ncmp, flags);
}

static void
sortsv_amagic_i_ncmp_desc(pTHX_ gptr *base, size_t nmemb, U32 flags)
{
    sortsv_flags_impl(base, nmemb, S_amagic_i_ncmp_desc, flags);
}

static void
sortsv_i_ncmp(pTHX_ gptr *base, size_t nmemb, U32 flags)
{
    sortsv_flags_impl(base, nmemb, S_sv_i_ncmp, flags);
}

static void
sortsv_i_ncmp_desc(pTHX_ gptr *base, size_t nmemb, U32 flags)
{
    sortsv_flags_impl(base, nmemb, S_sv_i_ncmp_desc, flags);
}

static void
sortsv_amagic_ncmp(pTHX_ gptr *base, size_t nmemb, U32 flags)
{
    sortsv_flags_impl(base, nmemb, S_amagic_ncmp, flags);
}

static void
sortsv_amagic_ncmp_desc(pTHX_ gptr *base, size_t nmemb, U32 flags)
{
    sortsv_flags_impl(base, nmemb, S_amagic_ncmp_desc, flags);
}

static void
sortsv_ncmp(pTHX_ gptr *base, size_t nmemb, U32 flags)
{
    sortsv_flags_impl(base, nmemb, S_sv_ncmp, flags);
}

static void
sortsv_ncmp_desc(pTHX_ gptr *base, size_t nmemb, U32 flags)
{
    sortsv_flags_impl(base, nmemb, S_sv_ncmp_desc, flags);
}

static void
sortsv_amagic_cmp(pTHX_ gptr *base, size_t nmemb, U32 flags)
{
    sortsv_flags_impl(base, nmemb, S_amagic_cmp, flags);
}

static void
sortsv_amagic_cmp_desc(pTHX_ gptr *base, size_t nmemb, U32 flags)
{
    sortsv_flags_impl(base, nmemb, S_amagic_cmp_desc, flags);
}

static void
sortsv_cmp(pTHX_ gptr *base, size_t nmemb, U32 flags)
{
    sortsv_flags_impl(base, nmemb, Perl_sv_cmp, flags);
}

static void
sortsv_cmp_desc(pTHX_ gptr *base, size_t nmemb, U32 flags)
{
    sortsv_flags_impl(base, nmemb, S_cmp_desc, flags);
}

#ifdef USE_LOCALE_COLLATE

static void
sortsv_amagic_cmp_locale(pTHX_ gptr *base, size_t nmemb, U32 flags)
{
    sortsv_flags_impl(base, nmemb, S_amagic_cmp_locale, flags);
}

static void
sortsv_amagic_cmp_locale_desc(pTHX_ gptr *base, size_t nmemb, U32 flags)
{
    sortsv_flags_impl(base, nmemb, S_amagic_cmp_locale_desc, flags);
}

static void
sortsv_cmp_locale(pTHX_ gptr *base, size_t nmemb, U32 flags)
{
    sortsv_flags_impl(base, nmemb, Perl_sv_cmp_locale, flags);
}

static void
sortsv_cmp_locale_desc(pTHX_ gptr *base, size_t nmemb, U32 flags)
{
    sortsv_flags_impl(base, nmemb, S_cmp_locale_desc, flags);
}

#endif

/*

=for apidoc sortsv

In-place sort an array of SV pointers with the given comparison routine.

Currently this always uses mergesort.  See C<L</sortsv_flags>> for a more
flexible routine.

=cut
*/

void
Perl_sortsv(pTHX_ SV **array, size_t nmemb, SVCOMPARE_t cmp)
{
    PERL_ARGS_ASSERT_SORTSV;

    sortsv_flags(array, nmemb, cmp, 0);
}

#define SvNSIOK(sv) ((SvFLAGS(sv) & SVf_NOK) || ((SvFLAGS(sv) & (SVf_IOK|SVf_IVisUV)) == SVf_IOK))
#define SvSIOK(sv) ((SvFLAGS(sv) & (SVf_IOK|SVf_IVisUV)) == SVf_IOK)
#define SvNSIV(sv) ( SvNOK(sv) ? SvNVX(sv) : ( SvSIOK(sv) ? SvIVX(sv) : sv_2nv(sv) ) )

PP(pp_sort)
{
    dSP; dMARK; dORIGMARK;
    SV **p1 = ORIGMARK+1, **p2;
    SSize_t max, i;
    AV* av = NULL;
    GV *gv;
    CV *cv = NULL;
    U8 gimme = GIMME_V;
    OP* const nextop = PL_op->op_next;
    I32 overloading = 0;
    bool hasargs = FALSE;
    bool copytmps;
    I32 is_xsub = 0;
    const U8 priv = PL_op->op_private;
    const U8 flags = PL_op->op_flags;
    U32 sort_flags = 0;
    I32 all_SIVs = 1, descending = 0;

    if ((priv & OPpSORT_DESCEND) != 0)
        descending = 1;

    if (gimme != G_LIST) {
        SP = MARK;
        EXTEND(SP,1);
        RETPUSHUNDEF;
    }

    ENTER;
    SAVEVPTR(PL_sortcop);

    /* Important flag meanings:
     *
     *  OPf_STACKED        sort <function_name> args
     *
     * (OPf_STACKED
     * |OPf_SPECIAL)       sort { <block> } args
     *
     *  ----               standard block; e.g. sort { $a <=> $b } args
     *
     *
     *  OPpSORT_NUMERIC    { $a <=> $b } (as opposed to $a cmp $b)
     *  OPpSORT_INTEGER    ditto in scope of 'use integer'
     *  OPpSORT_DESCEND    { $b <=> $a }
     *  OPpSORT_REVERSE    @a= reverse sort ....;
     *  OPpSORT_INPLACE    @a = sort @a;
     */

    if (flags & OPf_STACKED) {
        if (flags & OPf_SPECIAL) {
            OP *nullop = OpSIBLING(cLISTOP->op_first);  /* pass pushmark */
            assert(nullop->op_type == OP_NULL);
            PL_sortcop = nullop->op_next;
        }
        else {
            GV *autogv = NULL;
            HV *stash;
            cv = sv_2cv(*++MARK, &stash, &gv, GV_ADD);
          check_cv:
            if (cv && SvPOK(cv)) {
                const char * const proto = SvPV_nolen_const(MUTABLE_SV(cv));
                if (proto && strEQ(proto, "$$")) {
                    hasargs = TRUE;
                }
            }
            if (cv && CvISXSUB(cv) && CvXSUB(cv)) {
                is_xsub = 1;
            }
            else if (!(cv && CvROOT(cv))) {
                if (gv) {
                    goto autoload;
                }
                else if (!CvANON(cv) && (gv = CvGV(cv))) {
                    if (cv != GvCV(gv)) cv = GvCV(gv);
                  autoload:
                    if (!autogv && (
                        autogv = gv_autoload_pvn(
                            GvSTASH(gv), GvNAME(gv), GvNAMELEN(gv),
                            GvNAMEUTF8(gv) ? SVf_UTF8 : 0
                        )
                    )) {
                        cv = GvCVu(autogv);
                        goto check_cv;
                    }
                    else {
                        SV *tmpstr = sv_newmortal();
                        gv_efullname3(tmpstr, gv, NULL);
                        DIE(aTHX_ "Undefined sort subroutine \"%" SVf "\" called",
                            SVfARG(tmpstr));
                    }
                }
                else {
                    DIE(aTHX_ "Undefined subroutine in sort");
                }
            }

            if (is_xsub)
                PL_sortcop = (OP*)cv;
            else
                PL_sortcop = CvSTART(cv);
        }
    }
    else {
        PL_sortcop = NULL;
    }

    /* optimiser converts "@a = sort @a" to "sort \@a".  In this case,
     * push (@a) onto stack, then assign result back to @a at the end of
     * this function */
    if (priv & OPpSORT_INPLACE) {
        assert( MARK+1 == SP && *SP && SvTYPE(*SP) == SVt_PVAV);
        (void)POPMARK; /* remove mark associated with ex-OP_AASSIGN */
        av = MUTABLE_AV((*SP));
        if (SvREADONLY(av))
            Perl_croak_no_modify();
        max = AvFILL(av) + 1;
        MEXTEND(SP, max);
        if (SvMAGICAL(av)) {
            for (i=0; i < max; i++) {
                SV **svp = av_fetch(av, i, FALSE);
                *SP++ = (svp) ? *svp : NULL;
            }
        }
        else {
            SV **svp = AvARRAY(av);
            assert(svp || max == 0);
            for (i = 0; i < max; i++)
                *SP++ = *svp++;
        }
        SP--;
        p1 = p2 = SP - (max-1);
    }
    else {
        p2 = MARK+1;
        max = SP - MARK;
    }

    /* shuffle stack down, removing optional initial cv (p1!=p2), plus
     * any nulls; also stringify or converting to integer or number as
     * required any args */
    copytmps = cBOOL(PL_sortcop);
    for (i=max; i > 0 ; i--) {
        if ((*p1 = *p2++)) {                    /* Weed out nulls. */
            if (copytmps && SvPADTMP(*p1)) {
                *p1 = sv_mortalcopy(*p1);
            }
            SvTEMP_off(*p1);
            if (!PL_sortcop) {
                if (priv & OPpSORT_NUMERIC) {
                    if (priv & OPpSORT_INTEGER) {
                        if (!SvIOK(*p1))
                            (void)sv_2iv_flags(*p1, SV_GMAGIC|SV_SKIP_OVERLOAD);
                    }
                    else {
                        if (!SvNSIOK(*p1))
                            (void)sv_2nv_flags(*p1, SV_GMAGIC|SV_SKIP_OVERLOAD);
                        if (all_SIVs && !SvSIOK(*p1))
                            all_SIVs = 0;
                    }
                }
                else {
                    if (!SvPOK(*p1))
                        (void)sv_2pv_flags(*p1, 0,
                            SV_GMAGIC|SV_CONST_RETURN|SV_SKIP_OVERLOAD);
                }
                if (SvAMAGIC(*p1))
                    overloading = 1;
            }
            p1++;
        }
        else
            max--;
    }
    if (max > 1) {
        SV **start;
        if (PL_sortcop) {
            PERL_CONTEXT *cx;
            const bool oldcatch = CATCH_GET;
            I32 old_savestack_ix = PL_savestack_ix;

            SAVEOP();

            CATCH_SET(TRUE);
            PUSHSTACKi(PERLSI_SORT);
            if (!hasargs && !is_xsub) {
                SAVEGENERICSV(PL_firstgv);
                SAVEGENERICSV(PL_secondgv);
                PL_firstgv = MUTABLE_GV(SvREFCNT_inc(
                    gv_fetchpvs("a", GV_ADD|GV_NOTQUAL, SVt_PV)
                ));
                PL_secondgv = MUTABLE_GV(SvREFCNT_inc(
                    gv_fetchpvs("b", GV_ADD|GV_NOTQUAL, SVt_PV)
                ));
                /* make sure the GP isn't removed out from under us for
                 * the SAVESPTR() */
                save_gp(PL_firstgv, 0);
                save_gp(PL_secondgv, 0);
                /* we don't want modifications localized */
                GvINTRO_off(PL_firstgv);
                GvINTRO_off(PL_secondgv);
                SAVEGENERICSV(GvSV(PL_firstgv));
                SvREFCNT_inc(GvSV(PL_firstgv));
                SAVEGENERICSV(GvSV(PL_secondgv));
                SvREFCNT_inc(GvSV(PL_secondgv));
            }

            gimme = G_SCALAR;
            cx = cx_pushblock(CXt_NULL, gimme, PL_stack_base, old_savestack_ix);
            if (!(flags & OPf_SPECIAL)) {
                cx->cx_type = CXt_SUB|CXp_MULTICALL;
                cx_pushsub(cx, cv, NULL, hasargs);
                if (!is_xsub) {
                    PADLIST * const padlist = CvPADLIST(cv);

                    if (++CvDEPTH(cv) >= 2)
                        pad_push(padlist, CvDEPTH(cv));
                    PAD_SET_CUR_NOSAVE(padlist, CvDEPTH(cv));

                    if (hasargs) {
                        /* This is mostly copied from pp_entersub */
                        AV * const av0 = MUTABLE_AV(PAD_SVl(0));

                        cx->blk_sub.savearray = GvAV(PL_defgv);
                        GvAV(PL_defgv) = MUTABLE_AV(SvREFCNT_inc_simple(av0));
                    }

                }
            }

            start = p1 - max;
            Perl_sortsv_flags(aTHX_ start, max,
                    (is_xsub ? S_sortcv_xsub : hasargs ? S_sortcv_stacked : S_sortcv),
                    sort_flags);

            /* Reset cx, in case the context stack has been reallocated. */
            cx = CX_CUR();

            PL_stack_sp = PL_stack_base + cx->blk_oldsp;

            CX_LEAVE_SCOPE(cx);
            if (!(flags & OPf_SPECIAL)) {
                assert(CxTYPE(cx) == CXt_SUB);
                cx_popsub(cx);
            }
            else
                assert(CxTYPE(cx) == CXt_NULL);
                /* there isn't a POPNULL ! */

            cx_popblock(cx);
            CX_POP(cx);
            POPSTACK;
            CATCH_SET(oldcatch);
        }
        else {
            MEXTEND(SP, 20);    /* Can't afford stack realloc on signal. */
            start = ORIGMARK+1;
            if (priv & OPpSORT_NUMERIC) {
                if ((priv & OPpSORT_INTEGER) || all_SIVs) {
                    if (overloading)
                        if (descending)
                            sortsv_amagic_i_ncmp_desc(aTHX_ start, max, sort_flags);
                        else
                            sortsv_amagic_i_ncmp(aTHX_ start, max, sort_flags);
                    else
                        if (descending)
                            sortsv_i_ncmp_desc(aTHX_ start, max, sort_flags);
                        else
                            sortsv_i_ncmp(aTHX_ start, max, sort_flags);
                }
                else {
                    if (overloading)
                        if (descending)
                            sortsv_amagic_ncmp_desc(aTHX_ start, max, sort_flags);
                        else
                            sortsv_amagic_ncmp(aTHX_ start, max, sort_flags);
                    else
                        if (descending)
                            sortsv_ncmp_desc(aTHX_ start, max, sort_flags);
                        else
                            sortsv_ncmp(aTHX_ start, max, sort_flags);
                }
            }
#ifdef USE_LOCALE_COLLATE
            else if(IN_LC_RUNTIME(LC_COLLATE)) {
                if (overloading)
                    if (descending)
                        sortsv_amagic_cmp_locale_desc(aTHX_ start, max, sort_flags);
                    else
                        sortsv_amagic_cmp_locale(aTHX_ start, max, sort_flags);
                else
                    if (descending)
                        sortsv_cmp_locale_desc(aTHX_ start, max, sort_flags);
                    else
                        sortsv_cmp_locale(aTHX_ start, max, sort_flags);
            }
#endif
            else {
                if (overloading)
                    if (descending)
                        sortsv_amagic_cmp_desc(aTHX_ start, max, sort_flags);
                    else
                        sortsv_amagic_cmp(aTHX_ start, max, sort_flags);
                else
                    if (descending)
                        sortsv_cmp_desc(aTHX_ start, max, sort_flags);
                    else
                        sortsv_cmp(aTHX_ start, max, sort_flags);
            }
        }
        if ((priv & OPpSORT_REVERSE) != 0) {
            SV **q = start+max-1;
            while (start < q) {
                SV * const tmp = *start;
                *start++ = *q;
                *q-- = tmp;
            }
        }
    }

    if (av) {
        /* copy back result to the array */
        SV** const base = MARK+1;
        SSize_t max_minus_one = max - 1; /* attempt to work around mingw bug */
        if (SvMAGICAL(av)) {
            for (i = 0; i <= max_minus_one; i++)
                base[i] = newSVsv(base[i]);
            av_clear(av);
            if (max_minus_one >= 0)
                av_extend(av, max_minus_one);
            for (i=0; i <= max_minus_one; i++) {
                SV * const sv = base[i];
                SV ** const didstore = av_store(av, i, sv);
                if (SvSMAGICAL(sv))
                    mg_set(sv);
                if (!didstore)
                    sv_2mortal(sv);
            }
        }
        else {
            /* the elements of av are likely to be the same as the
             * (non-refcounted) elements on the stack, just in a different
             * order. However, its possible that someone's messed with av
             * in the meantime. So bump and unbump the relevant refcounts
             * first.
             */
            for (i = 0; i <= max_minus_one; i++) {
                SV *sv = base[i];
                assert(sv);
                if (SvREFCNT(sv) > 1)
                    base[i] = newSVsv(sv);
                else
                    SvREFCNT_inc_simple_void_NN(sv);
            }
            av_clear(av);
            if (max_minus_one >= 0) {
                av_extend(av, max_minus_one);
                Copy(base, AvARRAY(av), max, SV*);
            }
            AvFILLp(av) = max_minus_one;
            AvREIFY_off(av);
            AvREAL_on(av);
        }
    }
    LEAVE;
    PL_stack_sp = ORIGMARK +  max;
    return nextop;
}

static I32
S_sortcv(pTHX_ SV *const a, SV *const b)
{
    const I32 oldsaveix = PL_savestack_ix;
    I32 result;
    PMOP * const pm = PL_curpm;
    COP * const cop = PL_curcop;
    SV *olda, *oldb;
 
    PERL_ARGS_ASSERT_SORTCV;

    olda = GvSV(PL_firstgv);
    GvSV(PL_firstgv) = SvREFCNT_inc_simple_NN(a);
    SvREFCNT_dec(olda);
    oldb = GvSV(PL_secondgv);
    GvSV(PL_secondgv) = SvREFCNT_inc_simple_NN(b);
    SvREFCNT_dec(oldb);
    PL_stack_sp = PL_stack_base;
    PL_op = PL_sortcop;
    CALLRUNOPS(aTHX);
    PL_curcop = cop;
    /* entry zero of a stack is always PL_sv_undef, which
     * simplifies converting a '()' return into undef in scalar context */
    assert(PL_stack_sp > PL_stack_base || *PL_stack_base == &PL_sv_undef);
    result = SvIV(*PL_stack_sp);

    LEAVE_SCOPE(oldsaveix);
    PL_curpm = pm;
    return result;
}

static I32
S_sortcv_stacked(pTHX_ SV *const a, SV *const b)
{
    const I32 oldsaveix = PL_savestack_ix;
    I32 result;
    AV * const av = GvAV(PL_defgv);
    PMOP * const pm = PL_curpm;
    COP * const cop = PL_curcop;

    PERL_ARGS_ASSERT_SORTCV_STACKED;

    if (AvREAL(av)) {
        av_clear(av);
        AvREAL_off(av);
        AvREIFY_on(av);
    }
    if (AvMAX(av) < 1) {
        SV **ary = AvALLOC(av);
        if (AvARRAY(av) != ary) {
            AvMAX(av) += AvARRAY(av) - AvALLOC(av);
            AvARRAY(av) = ary;
        }
        if (AvMAX(av) < 1) {
            Renew(ary,2,SV*);
            AvMAX(av) = 1;
            AvARRAY(av) = ary;
            AvALLOC(av) = ary;
        }
    }
    AvFILLp(av) = 1;

    AvARRAY(av)[0] = a;
    AvARRAY(av)[1] = b;
    PL_stack_sp = PL_stack_base;
    PL_op = PL_sortcop;
    CALLRUNOPS(aTHX);
    PL_curcop = cop;
    /* entry zero of a stack is always PL_sv_undef, which
     * simplifies converting a '()' return into undef in scalar context */
    assert(PL_stack_sp > PL_stack_base || *PL_stack_base == &PL_sv_undef);
    result = SvIV(*PL_stack_sp);

    LEAVE_SCOPE(oldsaveix);
    PL_curpm = pm;
    return result;
}

static I32
S_sortcv_xsub(pTHX_ SV *const a, SV *const b)
{
    dSP;
    const I32 oldsaveix = PL_savestack_ix;
    CV * const cv=MUTABLE_CV(PL_sortcop);
    I32 result;
    PMOP * const pm = PL_curpm;

    PERL_ARGS_ASSERT_SORTCV_XSUB;

    SP = PL_stack_base;
    PUSHMARK(SP);
    EXTEND(SP, 2);
    *++SP = a;
    *++SP = b;
    PUTBACK;
    (void)(*CvXSUB(cv))(aTHX_ cv);
    /* entry zero of a stack is always PL_sv_undef, which
     * simplifies converting a '()' return into undef in scalar context */
    assert(PL_stack_sp > PL_stack_base || *PL_stack_base == &PL_sv_undef);
    result = SvIV(*PL_stack_sp);

    LEAVE_SCOPE(oldsaveix);
    PL_curpm = pm;
    return result;
}


PERL_STATIC_FORCE_INLINE I32
S_sv_ncmp(pTHX_ SV *const a, SV *const b)
{
    I32 cmp = do_ncmp(a, b);

    PERL_ARGS_ASSERT_SV_NCMP;

    if (cmp == 2) {
        if (ckWARN(WARN_UNINITIALIZED)) report_uninit(NULL);
        return 0;
    }

    return cmp;
}

PERL_STATIC_FORCE_INLINE I32
S_sv_ncmp_desc(pTHX_ SV *const a, SV *const b)
{
    PERL_ARGS_ASSERT_SV_NCMP_DESC;

    return -S_sv_ncmp(aTHX_ a, b);
}

PERL_STATIC_FORCE_INLINE I32
S_sv_i_ncmp(pTHX_ SV *const a, SV *const b)
{
    const IV iv1 = SvIV(a);
    const IV iv2 = SvIV(b);

    PERL_ARGS_ASSERT_SV_I_NCMP;

    return iv1 < iv2 ? -1 : iv1 > iv2 ? 1 : 0;
}

PERL_STATIC_FORCE_INLINE I32
S_sv_i_ncmp_desc(pTHX_ SV *const a, SV *const b)
{
    PERL_ARGS_ASSERT_SV_I_NCMP_DESC;

    return -S_sv_i_ncmp(aTHX_ a, b);
}

#define tryCALL_AMAGICbin(left,right,meth) \
    (SvAMAGIC(left)||SvAMAGIC(right)) \
        ? amagic_call(left, right, meth, 0) \
        : NULL;

#define SORT_NORMAL_RETURN_VALUE(val)  (((val) > 0) ? 1 : ((val) ? -1 : 0))

PERL_STATIC_FORCE_INLINE I32
S_amagic_ncmp(pTHX_ SV *const a, SV *const b)
{
    SV * const tmpsv = tryCALL_AMAGICbin(a,b,ncmp_amg);

    PERL_ARGS_ASSERT_AMAGIC_NCMP;

    if (tmpsv) {
        if (SvIOK(tmpsv)) {
            const I32 i = SvIVX(tmpsv);
            return SORT_NORMAL_RETURN_VALUE(i);
        }
        else {
            const NV d = SvNV(tmpsv);
            return SORT_NORMAL_RETURN_VALUE(d);
        }
     }
     return S_sv_ncmp(aTHX_ a, b);
}

PERL_STATIC_FORCE_INLINE I32
S_amagic_ncmp_desc(pTHX_ SV *const a, SV *const b)
{
    PERL_ARGS_ASSERT_AMAGIC_NCMP_DESC;

    return -S_amagic_ncmp(aTHX_ a, b);
}

PERL_STATIC_FORCE_INLINE I32
S_amagic_i_ncmp(pTHX_ SV *const a, SV *const b)
{
    SV * const tmpsv = tryCALL_AMAGICbin(a,b,ncmp_amg);

    PERL_ARGS_ASSERT_AMAGIC_I_NCMP;

    if (tmpsv) {
        if (SvIOK(tmpsv)) {
            const I32 i = SvIVX(tmpsv);
            return SORT_NORMAL_RETURN_VALUE(i);
        }
        else {
            const NV d = SvNV(tmpsv);
            return SORT_NORMAL_RETURN_VALUE(d);
        }
    }
    return S_sv_i_ncmp(aTHX_ a, b);
}

PERL_STATIC_FORCE_INLINE I32
S_amagic_i_ncmp_desc(pTHX_ SV *const a, SV *const b)
{
    PERL_ARGS_ASSERT_AMAGIC_I_NCMP_DESC;

    return -S_amagic_i_ncmp(aTHX_ a, b);
}

PERL_STATIC_FORCE_INLINE I32
S_amagic_cmp(pTHX_ SV *const str1, SV *const str2)
{
    SV * const tmpsv = tryCALL_AMAGICbin(str1,str2,scmp_amg);

    PERL_ARGS_ASSERT_AMAGIC_CMP;

    if (tmpsv) {
        if (SvIOK(tmpsv)) {
            const I32 i = SvIVX(tmpsv);
            return SORT_NORMAL_RETURN_VALUE(i);
        }
        else {
            const NV d = SvNV(tmpsv);
            return SORT_NORMAL_RETURN_VALUE(d);
        }
    }
    return sv_cmp(str1, str2);
}

PERL_STATIC_FORCE_INLINE I32
S_amagic_cmp_desc(pTHX_ SV *const str1, SV *const str2)
{
    PERL_ARGS_ASSERT_AMAGIC_CMP_DESC;

    return -S_amagic_cmp(aTHX_ str1, str2);
}

PERL_STATIC_FORCE_INLINE I32
S_cmp_desc(pTHX_ SV *const str1, SV *const str2)
{
    PERL_ARGS_ASSERT_CMP_DESC;

    return -sv_cmp(str1, str2);
}

#ifdef USE_LOCALE_COLLATE

PERL_STATIC_FORCE_INLINE I32
S_amagic_cmp_locale(pTHX_ SV *const str1, SV *const str2)
{
    SV * const tmpsv = tryCALL_AMAGICbin(str1,str2,scmp_amg);

    PERL_ARGS_ASSERT_AMAGIC_CMP_LOCALE;

    if (tmpsv) {
        if (SvIOK(tmpsv)) {
            const I32 i = SvIVX(tmpsv);
            return SORT_NORMAL_RETURN_VALUE(i);
        }
        else {
            const NV d = SvNV(tmpsv);
            return SORT_NORMAL_RETURN_VALUE(d);
        }
    }
    return sv_cmp_locale(str1, str2);
}

PERL_STATIC_FORCE_INLINE I32
S_amagic_cmp_locale_desc(pTHX_ SV *const str1, SV *const str2)
{
    PERL_ARGS_ASSERT_AMAGIC_CMP_LOCALE_DESC;

    return -S_amagic_cmp_locale(aTHX_ str1, str2);
}

PERL_STATIC_FORCE_INLINE I32
S_cmp_locale_desc(pTHX_ SV *const str1, SV *const str2)
{
    PERL_ARGS_ASSERT_CMP_LOCALE_DESC;

    return -sv_cmp_locale(str1, str2);
}

#endif

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
