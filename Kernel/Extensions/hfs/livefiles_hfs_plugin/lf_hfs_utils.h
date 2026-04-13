/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_utils.h
 *  livefiles_hfs
 *
 *  Created by Yakov Ben Zaken on 19/03/2018.
 */

#ifndef lf_hfs_utils_h
#define lf_hfs_utils_h

#include <stdio.h>
#include <assert.h>
#include "lf_hfs_locks.h"
#include "lf_hfs.h"
#include "lf_hfs_logger.h"

#define hfs_assert(expr)                                        \
    do {                                                        \
        if ( (expr) == (0) )                                    \
        {                                                       \
            LFHFS_LOG(  LEVEL_ERROR,                            \
                        "HFS ASSERT [%s] [%d]\n",               \
                        __FILE__,                               \
                        __LINE__);                              \
            assert( 0 );                                        \
        }                                                       \
    } while (0)

#define MAC_GMT_FACTOR        2082844800UL


void*       hashinit(int elements, u_long *hashmask);
void        hashDeinit(void* pvHashTbl);
time_t      to_bsd_time(u_int32_t hfs_time);
u_int32_t   to_hfs_time(time_t bsd_time);
void        microuptime(struct timeval *tvp);
void        microtime(struct timeval *tvp);
void*       lf_hfs_utils_allocate_and_copy_string( char *pcName, size_t uLen );
off_t       blk_to_bytes(uint32_t blk, uint32_t blk_size);

#endif /* lf_hfs_utils_h */
