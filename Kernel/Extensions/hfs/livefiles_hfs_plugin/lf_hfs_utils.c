/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_utils.c
 *  livefiles_hfs
 *
 *  Created by Yakov Ben Zaken on 19/03/2018.
 */

#include "lf_hfs_utils.h"
#include "lf_hfs_vfsutils.h"

/*
 * General routine to allocate a hash table.
 */
void *
hashinit(int elements, u_long *hashmask)
{
    int hashsize = 0;
    LIST_HEAD(generic, generic) *hashtbl;
    int i;

    if (elements <= 0)
        return NULL;
    for (hashsize = 1; hashsize <= elements; hashsize <<= 1)
    {
        continue;
    }

    hashsize >>= 1;
    hashtbl = hfs_malloc(hashsize * sizeof(*hashtbl));
    if (hashtbl != NULL)
    {
        for (i = 0; i < hashsize; i++)
        {
            LIST_INIT(&hashtbl[i]);
        }
        *hashmask = hashsize - 1;
    }
    return (hashtbl);
}

/*
 * General routine to free a hash table.
 */
void
hashDeinit(void* pvHashTbl)
{
    LIST_HEAD(generic, generic) *hashtbl = pvHashTbl;
    hfs_free(hashtbl);
}

/*
 * to_bsd_time - convert from Mac OS time (seconds since 1/1/1904)
 *         to BSD time (seconds since 1/1/1970)
 */
time_t
to_bsd_time(u_int32_t hfs_time)
{
    u_int32_t gmt = hfs_time;

    if (gmt > MAC_GMT_FACTOR)
        gmt -= MAC_GMT_FACTOR;
    else
        gmt = 0;    /* don't let date go negative! */

    return (time_t)gmt;
}

/*
 * to_hfs_time - convert from BSD time (seconds since 1/1/1970)
 *         to Mac OS time (seconds since 1/1/1904)
 */
u_int32_t
to_hfs_time(time_t bsd_time)
{
    u_int32_t hfs_time = (u_int32_t)bsd_time;

    /* don't adjust zero - treat as uninitialzed */
    if (hfs_time != 0)
        hfs_time += MAC_GMT_FACTOR;

    return (hfs_time);
}

void
microuptime(struct timeval *tvp)
{
    struct timespec ts;
    clock_gettime( CLOCK_MONOTONIC, &ts );
    TIMESPEC_TO_TIMEVAL(tvp, &ts);
}

void
microtime(struct timeval *tvp)
{
    struct timespec ts;
    clock_gettime( CLOCK_REALTIME, &ts );
    TIMESPEC_TO_TIMEVAL(tvp, &ts);
}

void* lf_hfs_utils_allocate_and_copy_string( char *pcName, size_t uLen )
{
    //Check the validity of the uLen
    if (uLen > kHFSPlusMaxFileNameChars) {
        return NULL;
    }

    //Checkk the validity of the pcName
    if (strlen(pcName) != uLen) {
        return NULL;
    }

    void *pvTmp = hfs_malloc( uLen+1 );
    if ( pvTmp == NULL ) {
        return NULL;
    }
    
    memcpy(pvTmp, pcName, uLen);
    //Add Null terminated at the end of the name
    char *pcLastChar = pvTmp + uLen;
    *pcLastChar = '\0';

    return pvTmp;
}

off_t
blk_to_bytes(uint32_t blk, uint32_t blk_size)
{
    return (off_t)blk * blk_size;         // Avoid the overflow
}
