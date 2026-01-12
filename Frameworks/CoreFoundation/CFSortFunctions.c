/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*	CFSortFunctions.c
	Copyright (c) 1999-2014, Apple Inc. All rights reserved.
	Responsibility: Christopher Kane
*/

#include <CoreFoundation/CFBase.h>
#include "CFInternal.h"
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
#include <dispatch/dispatch.h>
#endif
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
#include <dispatch/private.h>
#endif

enum {
    kCFSortConcurrent = (1 << 0),
    kCFSortStable = (1 << 4),
};

typedef CFIndex VALUE_TYPE;
typedef CFIndex INDEX_TYPE;
typedef CFComparisonResult CMP_RESULT_TYPE;
typedef CMP_RESULT_TYPE (^COMPARATOR_BLOCK)(VALUE_TYPE, VALUE_TYPE);

/*
Number of elements in a list and expected number of compares,
when the initial short-circuiting compare is not done.
1	0
2	1
3	2.667
4	4.667
5	7.167
6	9.833
7	12.733
8	15.733
9	19.167
10	22.667
11	26.2857
12	29.9524
*/

static void __CFSimpleMerge(VALUE_TYPE listp[], INDEX_TYPE cnt1, INDEX_TYPE cnt2, VALUE_TYPE tmp[], COMPARATOR_BLOCK cmp) {
    if (cnt1 <= 0 || cnt2 <= 0) return;
    // if the last element of listp1 <= the first of listp2, lists are already ordered
    if (16 < cnt1 + cnt2 && cmp(listp[cnt1 - 1], listp[cnt1]) <= 0) return;

    INDEX_TYPE idx = 0, idx1 = 0, idx2 = cnt1;
    for (;;) {
        if (cnt1 <= idx1) {
            while (idx--) {
                listp[idx] = tmp[idx];
            }
            return;
        }
        if (cnt1 + cnt2 <= idx2) {
            for (INDEX_TYPE t = cnt1 + cnt2 - 1; idx <= t; t--) {
                listp[t] = listp[t - cnt2];
            }
            while (idx--) {
                listp[idx] = tmp[idx];
            }
            return;
        }
        VALUE_TYPE v1 = listp[idx1], v2 = listp[idx2];
        if (cmp(v1, v2) <= 0) {
            tmp[idx] = v1;
            idx1++;
        } else {
            tmp[idx] = v2;
            idx2++;
        }
        idx++;
    }
}

static void __CFSimpleMergeSort(VALUE_TYPE listp[], INDEX_TYPE cnt, VALUE_TYPE tmp[], COMPARATOR_BLOCK cmp) {
    if (cnt < 2) {
        /* do nothing */
    } else if (2 == cnt) {
        VALUE_TYPE v0 = listp[0], v1 = listp[1];
        if (0 < cmp(v0, v1)) {
            listp[0] = v1;
            listp[1] = v0;
        }
    } else if (3 == cnt) {
        VALUE_TYPE v0 = listp[0], v1 = listp[1], v2 = listp[2], vt;
        if (0 < cmp(v0, v1)) {
            vt = v0;
            v0 = v1;
            v1 = vt;
        }
        if (0 < cmp(v1, v2)) {
            vt = v1;
            v1 = v2;
            v2 = vt;
            if (0 < cmp(v0, v1)) {
                vt = v0;
                v0 = v1;
                v1 = vt;
            }
        }
        listp[0] = v0;
        listp[1] = v1;
        listp[2] = v2;
    } else {
        INDEX_TYPE half_cnt = cnt / 2;
        __CFSimpleMergeSort(listp, half_cnt, tmp, cmp);
        __CFSimpleMergeSort(listp + half_cnt, cnt - half_cnt, tmp, cmp);
        __CFSimpleMerge(listp, half_cnt, cnt - half_cnt, tmp, cmp);
    }
}

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
// Excluded from linux for dispatch dependency

// if !right, put the cnt1 smallest values in tmp, else put the cnt2 largest values in tmp
static void __CFSortIndexesNMerge(VALUE_TYPE listp1[], INDEX_TYPE cnt1, VALUE_TYPE listp2[], INDEX_TYPE cnt2, VALUE_TYPE tmp[], size_t right, COMPARATOR_BLOCK cmp) {
    // if the last element of listp1 <= the first of listp2, lists are already ordered
    if (16 < cnt1 + cnt2 && cmp(listp1[cnt1 - 1], listp2[0]) <= 0) {
        memmove(tmp, (right ? listp2 : listp1), (right ? cnt2 : cnt1) * sizeof(VALUE_TYPE));
        return;
    }

    if (right) {
        VALUE_TYPE *listp1_end = listp1;
        VALUE_TYPE *listp2_end = listp2;
        VALUE_TYPE *tmp_end = tmp;
        listp1 += cnt1 - 1;
        listp2 += cnt2 - 1;
        tmp += cnt2;
        while (tmp_end < tmp) {
            tmp--;
            if (listp2 < listp2_end) {
                listp1--;
                *tmp = *listp1;
            } else if (listp1 < listp1_end) {
                listp2--;
                *tmp = *listp2;
            } else {
                VALUE_TYPE v1 = *listp1, v2 = *listp2;
                CMP_RESULT_TYPE res = cmp(v1, v2);
                if (res <= 0) {
                    *tmp = v2;
                    listp2--;
                } else {
                    *tmp = v1;
                    listp1--;
                }
            }
        }
    } else {
        VALUE_TYPE *listp1_end = listp1 + cnt1;
        VALUE_TYPE *listp2_end = listp2 + cnt2;
        VALUE_TYPE *tmp_end = tmp + cnt1;
        while (tmp < tmp_end) {
            if (listp2_end <= listp2) {
                *tmp = *listp1;
                listp1++;
            } else if (listp1_end <= listp1) {
                *tmp = *listp2;
                listp2++;
            } else {
                VALUE_TYPE v1 = *listp1, v2 = *listp2;
                CMP_RESULT_TYPE res = cmp(v1, v2);
                if (res <= 0) {
                    *tmp = v1;
                    listp1++;
                } else {
                    *tmp = v2;
                    listp2++;
                }
            }
            tmp++;
        }
    }
}

/* Merging algorithm based on
    "A New Parallel Sorting Algorithm based on Odd-Even Mergesort", Ezequiel Herruzo, et al
*/
static void __CFSortIndexesN(VALUE_TYPE listp[], INDEX_TYPE count, int32_t ncores, CMP_RESULT_TYPE (^cmp)(INDEX_TYPE, INDEX_TYPE)) {
    /* Divide the array up into up to ncores, multiple-of-16-sized, chunks */
    INDEX_TYPE sz = ((((count + ncores - 1) / ncores) + 15) / 16) * 16;
    INDEX_TYPE num_sect = (count + sz - 1) / sz;
    INDEX_TYPE last_sect_len = count + sz - sz * num_sect;

    STACK_BUFFER_DECL(VALUE_TYPE *, stack_tmps, num_sect);
    for (INDEX_TYPE idx = 0; idx < num_sect; idx++) {
        stack_tmps[idx] = (VALUE_TYPE *)malloc(sz * sizeof(VALUE_TYPE));
    }
    VALUE_TYPE **tmps = stack_tmps;

    dispatch_queue_t q = __CFDispatchQueueGetGenericMatchingCurrent();
    dispatch_apply(num_sect, q, ^(size_t sect) {
            INDEX_TYPE sect_len = (sect < num_sect - 1) ? sz : last_sect_len;
            __CFSimpleMergeSort(listp + sect * sz, sect_len, tmps[sect], cmp); // naturally stable
        });

    INDEX_TYPE even_phase_cnt = ((num_sect / 2) * 2);
    INDEX_TYPE odd_phase_cnt = (((num_sect - 1) / 2) * 2);
    for (INDEX_TYPE idx = 0; idx < (num_sect + 1) / 2; idx++) {
        dispatch_apply(even_phase_cnt, q, ^(size_t sect) { // merge even
                size_t right = sect & (size_t)0x1;
                VALUE_TYPE *left_base = listp + sect * sz - (right ? sz : 0);
                VALUE_TYPE *right_base = listp + sect * sz + (right ? 0 : sz);
                INDEX_TYPE sect2_len = (sect + 1 + (right ? 0 : 1) == num_sect) ? last_sect_len : sz;
                __CFSortIndexesNMerge(left_base, sz, right_base, sect2_len, tmps[sect], right, cmp);
            });
        if (num_sect & 0x1) {
            memmove(tmps[num_sect - 1], listp + (num_sect - 1) * sz, last_sect_len * sizeof(VALUE_TYPE));
        }
        dispatch_apply(odd_phase_cnt, q, ^(size_t sect) { // merge odd
                size_t right = sect & (size_t)0x1;
                VALUE_TYPE *left_base = tmps[sect + (right ? 0 : 1)];
                VALUE_TYPE *right_base = tmps[sect + (right ? 1 : 2)];
                INDEX_TYPE sect2_len = (sect + 1 + (right ? 1 : 2) == num_sect) ? last_sect_len : sz;
                __CFSortIndexesNMerge(left_base, sz, right_base, sect2_len, listp + sect * sz + sz, right, cmp);
            });
        memmove(listp + 0 * sz, tmps[0], sz * sizeof(VALUE_TYPE));
        if (!(num_sect & 0x1)) {
            memmove(listp + (num_sect - 1) * sz, tmps[num_sect - 1], last_sect_len * sizeof(VALUE_TYPE));
        }
    }

    for (INDEX_TYPE idx = 0; idx < num_sect; idx++) {
        free(stack_tmps[idx]);
    }
}
#endif

// fills an array of indexes (of length count) giving the indexes 0 - count-1, as sorted by the comparator block
void CFSortIndexes(CFIndex *indexBuffer, CFIndex count, CFOptionFlags opts, CFComparisonResult (^cmp)(CFIndex, CFIndex)) {
    if (count < 1) return;
    if (INTPTR_MAX / sizeof(CFIndex) < count) return;
    int32_t ncores = 0;
    if (opts & kCFSortConcurrent) {
        ncores = __CFActiveProcessorCount();
        if (count < 160 || ncores < 2) {
            opts = (opts & ~kCFSortConcurrent);
        } else if (count < 640 && 2 < ncores) {
            ncores = 2;
        } else if (count < 3200 && 4 < ncores) {
            ncores = 4;
        } else if (count < 16000 && 8 < ncores) {
            ncores = 8;
        }
        if (16 < ncores) {
            ncores = 16;
        }
    }
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
    if (count <= 65536) {
        for (CFIndex idx = 0; idx < count; idx++) indexBuffer[idx] = idx;
    } else {
        /* Specifically hard-coded to 8; the count has to be very large before more chunks and/or cores is worthwhile. */
        CFIndex sz = ((((size_t)count + 15) / 16) * 16) / 8;
        dispatch_apply(8, __CFDispatchQueueGetGenericMatchingCurrent(), ^(size_t n) {
                CFIndex idx = n * sz, lim = __CFMin(idx + sz, count);
                for (; idx < lim; idx++) indexBuffer[idx] = idx;
            });
    }
#else
    for (CFIndex idx = 0; idx < count; idx++) indexBuffer[idx] = idx;
#endif
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
    if (opts & kCFSortConcurrent) {
        __CFSortIndexesN(indexBuffer, count, ncores, cmp); // naturally stable
        return;
    }
#endif
    STACK_BUFFER_DECL(VALUE_TYPE, local, count <= 4096 ? count : 1);
    VALUE_TYPE *tmp = (count <= 4096) ? local : (VALUE_TYPE *)malloc(count * sizeof(VALUE_TYPE));
    __CFSimpleMergeSort(indexBuffer, count, tmp, cmp); // naturally stable
    if (local != tmp) free(tmp);
}

/* Comparator is passed the address of the values. */
void CFQSortArray(void *list, CFIndex count, CFIndex elementSize, CFComparatorFunction comparator, void *context) {
    if (count < 2 || elementSize < 1) return;
    STACK_BUFFER_DECL(CFIndex, locali, count <= 4096 ? count : 1);
    CFIndex *indexes = (count <= 4096) ? locali : (CFIndex *)malloc(count * sizeof(CFIndex));
    CFSortIndexes(indexes, count, 0, ^(CFIndex a, CFIndex b) { return comparator((char *)list + a * elementSize, (char *)list + b * elementSize, context); });
    STACK_BUFFER_DECL(uint8_t, locals, count <= (16 * 1024 / elementSize) ? count * elementSize : 1);
    void *store = (count <= (16 * 1024 / elementSize)) ? locals : malloc(count * elementSize);
    for (CFIndex idx = 0; idx < count; idx++) {
        if (sizeof(uintptr_t) == elementSize) {
            uintptr_t *a = (uintptr_t *)list + indexes[idx];
            uintptr_t *b = (uintptr_t *)store + idx;
            *b = *a;
        } else {
            memmove((char *)store + idx * elementSize, (char *)list + indexes[idx] * elementSize, elementSize);
        }
    }
    // no swapping or modification of the original list has occurred until this point
    objc_memmove_collectable(list, store, count * elementSize);
    if (locals != store) free(store);
    if (locali != indexes) free(indexes);
}

/* Comparator is passed the address of the values. */
void CFMergeSortArray(void *list, CFIndex count, CFIndex elementSize, CFComparatorFunction comparator, void *context) {
    if (count < 2 || elementSize < 1) return;
    STACK_BUFFER_DECL(CFIndex, locali, count <= 4096 ? count : 1);
    CFIndex *indexes = (count <= 4096) ? locali : (CFIndex *)malloc(count * sizeof(CFIndex));
    CFSortIndexes(indexes, count, kCFSortStable, ^(CFIndex a, CFIndex b) { return comparator((char *)list + a * elementSize, (char *)list + b * elementSize, context); });
    STACK_BUFFER_DECL(uint8_t, locals, count <= (16 * 1024 / elementSize) ? count * elementSize : 1);
    void *store = (count <= (16 * 1024 / elementSize)) ? locals : malloc(count * elementSize);
    for (CFIndex idx = 0; idx < count; idx++) {
        if (sizeof(uintptr_t) == elementSize) {
            uintptr_t *a = (uintptr_t *)list + indexes[idx];
            uintptr_t *b = (uintptr_t *)store + idx;
            *b = *a;
        } else {
            memmove((char *)store + idx * elementSize, (char *)list + indexes[idx] * elementSize, elementSize);
        }
    }
    // no swapping or modification of the original list has occurred until this point
    objc_memmove_collectable(list, store, count * elementSize);
    if (locals != store) free(store);
    if (locali != indexes) free(indexes);
}


