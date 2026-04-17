/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_rangelist.h
 *  livefiles_hfs
 *
 *  Created by Yakov Ben Zaken on 19/03/2018.
 */

#ifndef lf_hfs_rangelist_h
#define lf_hfs_rangelist_h

#include <stdio.h>
#include <sys/queue.h>

TAILQ_HEAD(rl_head, rl_entry);

struct rl_entry {
    TAILQ_ENTRY(rl_entry) rl_link;
    off_t rl_start;
    off_t rl_end;
};

enum rl_overlaptype {
    RL_NOOVERLAP = 0,        /* 0 */
    RL_MATCHINGOVERLAP,      /* 1 */
    RL_OVERLAPCONTAINSRANGE, /* 2 */
    RL_OVERLAPISCONTAINED,   /* 3 */
    RL_OVERLAPSTARTSBEFORE,  /* 4 */
    RL_OVERLAPENDSAFTER      /* 5 */
};

#define RL_INFINITY INT64_MAX

void rl_init(struct rl_head *rangelist);
enum rl_overlaptype rl_overlap(const struct rl_entry *range, off_t start, off_t end);
void rl_remove(off_t start, off_t end, struct rl_head *rangelist);
off_t rl_len(const struct rl_entry *range);
void rl_remove_all(struct rl_head *rangelist);
enum rl_overlaptype rl_scan(struct rl_head *rangelist, off_t start, off_t end, struct rl_entry **overlap);
void rl_add(off_t start, off_t end, struct rl_head *rangelist);
void rl_subtract(struct rl_entry *a, const struct rl_entry *b);
struct rl_entry rl_make(off_t start, off_t end);

#endif /* lf_hfs_rangelist_h */
