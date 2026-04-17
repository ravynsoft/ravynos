/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_rangelist.c
 *  livefiles_hfs
 *
 *  Created by Yakov Ben Zaken on 19/03/2018.
 */

#include "lf_hfs_rangelist.h"
#include "lf_hfs_vfsutils.h"

static void rl_collapse_forwards(struct rl_head *rangelist, struct rl_entry *range);
static void rl_collapse_backwards(struct rl_head *rangelist, struct rl_entry *range);
static void rl_collapse_neighbors(struct rl_head *rangelist, struct rl_entry *range);

/*
 * Walk the list of ranges for an entry to
 * find an overlapping range (if any).
 *
 * NOTE: this returns only the FIRST overlapping range.
 *         There may be more than one.
 */
static enum rl_overlaptype
rl_scan_from(struct rl_head *rangelist __unused,
             off_t start,
             off_t end,
             struct rl_entry **overlap,
             struct rl_entry *range)
{

    while (range)
    {
        enum rl_overlaptype ot = rl_overlap(range, start, end);

        if (ot != RL_NOOVERLAP || range->rl_start > end)
        {
            *overlap = range;
            return ot;
        }

        range = TAILQ_NEXT(range, rl_link);
    }

    *overlap = NULL;
    return RL_NOOVERLAP;
}

void
rl_init(struct rl_head *rangelist)
{
    TAILQ_INIT(rangelist);
}

enum rl_overlaptype
rl_overlap(const struct rl_entry *range, off_t start, off_t end)
{
    /*
     * OK, check for overlap
     *
     * Six cases:
     *    0) no overlap (RL_NOOVERLAP)
     *    1) overlap == range (RL_MATCHINGOVERLAP)
     *    2) overlap contains range (RL_OVERLAPCONTAINSRANGE)
     *    3) range contains overlap (RL_OVERLAPISCONTAINED)
     *    4) overlap starts before range (RL_OVERLAPSTARTSBEFORE)
     *    5) overlap ends after range (RL_OVERLAPENDSAFTER)
     */
    if (start > range->rl_end || range->rl_start > end)
    {
        /* Case 0 (RL_NOOVERLAP) */
        return RL_NOOVERLAP;
    }

    if (range->rl_start == start && range->rl_end == end)
    {
        /* Case 1 (RL_MATCHINGOVERLAP) */
        return RL_MATCHINGOVERLAP;
    }

    if (range->rl_start <= start && range->rl_end >= end)
    {
        /* Case 2 (RL_OVERLAPCONTAINSRANGE) */
        return RL_OVERLAPCONTAINSRANGE;
    }

    if (start <= range->rl_start && end >= range->rl_end)
    {
        /* Case 3 (RL_OVERLAPISCONTAINED) */
        return RL_OVERLAPISCONTAINED;
    }

    if (range->rl_start < start && range->rl_end < end)
    {
        /* Case 4 (RL_OVERLAPSTARTSBEFORE) */
        return RL_OVERLAPSTARTSBEFORE;
    }

    /* Case 5 (RL_OVERLAPENDSAFTER) */
    // range->rl_start > start && range->rl_end > end
    return RL_OVERLAPENDSAFTER;
}

/*
 * Remove a range from a range list.
 *
 * Generally, find the range (or an overlap to that range)
 * and remove it (or shrink it), then wakeup anyone we can.
 */
void
rl_remove(off_t start, off_t end, struct rl_head *rangelist)
{
    struct rl_entry *range, *next_range, *overlap, *splitrange;
    int ovcase;

    if (TAILQ_EMPTY(rangelist))
    {
        return;
    };

    range = TAILQ_FIRST(rangelist);
    while ((ovcase = rl_scan_from(rangelist, start, end, &overlap, range)))
    {
        switch (ovcase)
        {

            case RL_MATCHINGOVERLAP: /* 1: overlap == range */
                TAILQ_REMOVE(rangelist, overlap, rl_link);
                hfs_free(overlap);
                break;

            case RL_OVERLAPCONTAINSRANGE: /* 2: overlap contains range: split it */
                if (overlap->rl_start == start) {
                    overlap->rl_start = end + 1;
                    break;
                };

                if (overlap->rl_end == end) {
                    overlap->rl_end = start - 1;
                    break;
                };

                /*
                 * Make a new range consisting of the last part of the encompassing range
                 */
                splitrange = hfs_malloc(sizeof(struct rl_entry));
                splitrange->rl_start = end + 1;
                splitrange->rl_end = overlap->rl_end;
                overlap->rl_end = start - 1;

                /*
                 * Now link the new entry into the range list after the range from which it was split:
                 */
                TAILQ_INSERT_AFTER(rangelist, overlap, splitrange, rl_link);
                break;

            case RL_OVERLAPISCONTAINED: /* 3: range contains overlap */
                /* Check before discarding overlap entry */
                next_range = TAILQ_NEXT(overlap, rl_link);
                TAILQ_REMOVE(rangelist, overlap, rl_link);
                hfs_free(overlap);
                if (next_range)
                {
                    range = next_range;
                    continue;
                };
                break;

            case RL_OVERLAPSTARTSBEFORE: /* 4: overlap starts before range */
                overlap->rl_end = start - 1;
                range = TAILQ_NEXT(overlap, rl_link);
                if (range) {
                    continue;
                }
                break;

            case RL_OVERLAPENDSAFTER: /* 5: overlap ends after range */
                overlap->rl_start = (end == RL_INFINITY ? RL_INFINITY : end + 1);
                break;
        }
        break;
    }
}

off_t rl_len(const struct rl_entry *range)
{
    return range->rl_end - range->rl_start + 1;
}

void rl_remove_all(struct rl_head *rangelist)
{
    struct rl_entry *r, *nextr;
    TAILQ_FOREACH_SAFE(r, rangelist, rl_link, nextr){
        hfs_free(r);
    }
    TAILQ_INIT(rangelist);
}

/*
 * Add a range to the list
 */
void
rl_add(off_t start, off_t end, struct rl_head *rangelist)
{
    struct rl_entry *range;
    struct rl_entry *overlap;
    enum rl_overlaptype ovcase;

#ifdef RL_DIAGNOSTIC
    if (end < start)
    {
        LFHFS_LOG(LEVEL_ERROR, "rl_add: end < start?!");
        hfs_assert(0);
    }
#endif

    ovcase = rl_scan(rangelist, start, end, &overlap);

    /*
     * Six cases:
     *    0) no overlap
     *    1) overlap == range
     *    2) overlap contains range
     *    3) range contains overlap
     *    4) overlap starts before range
     *    5) overlap ends after range
     */
    switch (ovcase) {
        case RL_NOOVERLAP: /* 0: no overlap */
            /*
             * overlap points to the entry we should insert before, or
             * if NULL, we should insert at the end.
             */
            range = hfs_mallocz(sizeof(*range));
            range->rl_start = start;
            range->rl_end = end;

            /* Link in the new range: */
            if (overlap) {
                TAILQ_INSERT_BEFORE(overlap, range, rl_link);
            } else {
                TAILQ_INSERT_TAIL(rangelist, range, rl_link);
            }

            /* Check to see if any ranges can be combined (possibly including the immediately
             preceding range entry)
             */
            rl_collapse_neighbors(rangelist, range);
            break;

        case RL_MATCHINGOVERLAP: /* 1: overlap == range */
        case RL_OVERLAPCONTAINSRANGE: /* 2: overlap contains range */
            break;

        case RL_OVERLAPISCONTAINED: /* 3: range contains overlap */
            /*
             * Replace the overlap with the new, larger range:
             */
            overlap->rl_start = start;
            overlap->rl_end = end;
            rl_collapse_neighbors(rangelist, overlap);
            break;

        case RL_OVERLAPSTARTSBEFORE: /* 4: overlap starts before range */
            /*
             * Expand the overlap area to cover the new range:
             */
            overlap->rl_end = end;
            rl_collapse_forwards(rangelist, overlap);
            break;

        case RL_OVERLAPENDSAFTER: /* 5: overlap ends after range */
            /*
             * Expand the overlap area to cover the new range:
             */
            overlap->rl_start = start;
            rl_collapse_backwards(rangelist, overlap);
            break;
    }

#ifdef RL_DIAGNOSTIC
    rl_verify(rangelist);
#endif
}

/*
 * Scan a range list for an entry in a specified range (if any):
 *
 * NOTE: this returns only the FIRST overlapping range.
 *         There may be more than one.
 */

enum rl_overlaptype
rl_scan(struct rl_head *rangelist, off_t start, off_t end, struct rl_entry **overlap)
{
    return rl_scan_from(rangelist, start, end, overlap, TAILQ_FIRST(rangelist));
}

static void
rl_collapse_forwards(struct rl_head *rangelist, struct rl_entry *range) {
    struct rl_entry *next_range;

    while ((next_range = TAILQ_NEXT(range, rl_link))) {
        if ((range->rl_end != RL_INFINITY) && (range->rl_end < next_range->rl_start - 1)) return;

        /* Expand this range to include the next range: */
        range->rl_end = next_range->rl_end;

        /* Remove the now covered range from the list: */
        TAILQ_REMOVE(rangelist, next_range, rl_link);
        hfs_free(next_range);

#ifdef RL_DIAGNOSTIC
        rl_verify(rangelist);
#endif
    };
}

static void
rl_collapse_backwards(struct rl_head *rangelist, struct rl_entry *range) {
    struct rl_entry *prev_range;

    while ((prev_range = TAILQ_PREV(range, rl_head, rl_link))) {
        if (prev_range->rl_end < range->rl_start -1) {
#ifdef RL_DIAGNOSTIC
            rl_verify(rangelist);
#endif
            return;
        };

        /* Expand this range to include the previous range: */
        range->rl_start = prev_range->rl_start;

        /* Remove the now covered range from the list: */
        TAILQ_REMOVE(rangelist, prev_range, rl_link);
        hfs_free(prev_range);
    };
}

static void
rl_collapse_neighbors(struct rl_head *rangelist, struct rl_entry *range)
{
    rl_collapse_forwards(rangelist, range);
    rl_collapse_backwards(rangelist, range);
}

/*
 * In the case where b is contained by a, we return the the largest part
 * remaining.  The result is stored in a.
 */
void rl_subtract(struct rl_entry *a, const struct rl_entry *b)
{
    switch (rl_overlap(b, a->rl_start, a->rl_end)) {
        case RL_MATCHINGOVERLAP:
        case RL_OVERLAPCONTAINSRANGE:
            a->rl_end = a->rl_start - 1;
            break;
        case RL_OVERLAPISCONTAINED:
            // Keep the bigger part
            if (b->rl_start - a->rl_start >= a->rl_end - b->rl_end) {
                // Keep left
                a->rl_end = b->rl_start - 1;
            } else {
                // Keep right
                a->rl_start = b->rl_end + 1;
            }
            break;
        case RL_OVERLAPSTARTSBEFORE:
            a->rl_start = b->rl_end + 1;
            break;
        case RL_OVERLAPENDSAFTER:
            a->rl_end = b->rl_start - 1;
            break;
        case RL_NOOVERLAP:
            break;
    }
}

struct rl_entry rl_make(off_t start, off_t end)
{
    return (struct rl_entry){ .rl_start = start, .rl_end = end };
}
