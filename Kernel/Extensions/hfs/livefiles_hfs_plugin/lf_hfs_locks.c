/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_locks.c
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 18/3/18.
 */

#include "lf_hfs_locks.h"
#include <errno.h>
#include <assert.h>

void lf_lck_rw_init( pthread_rwlock_t* lck )
{
    errno_t err = pthread_rwlock_init( lck, NULL );
    assert( err == 0 );
}

void lf_lck_rw_destroy( pthread_rwlock_t* lck )
{
    errno_t err = pthread_rwlock_destroy( lck );
    assert( err == 0 );
}

void lf_lck_rw_unlock_shared( pthread_rwlock_t* lck )
{
    errno_t err = pthread_rwlock_unlock( lck );
    assert( err == 0 );
}

void lf_lck_rw_lock_shared( pthread_rwlock_t* lck )
{
    errno_t err = pthread_rwlock_rdlock( lck );
    assert( err == 0 );
}

void lf_lck_rw_lock_exclusive( pthread_rwlock_t* lck )
{
    errno_t err = pthread_rwlock_wrlock( lck );
    assert( err == 0 );
}

void lf_lck_rw_unlock_exclusive( pthread_rwlock_t* lck )
{
    errno_t err = pthread_rwlock_unlock( lck );
    assert( err == 0 );
}

bool lf_lck_rw_try_lock( pthread_rwlock_t* lck, lck_rwlock_type_e which )
{
    bool trylock;

    if ( which == LCK_RW_TYPE_SHARED )
    {
        trylock = pthread_rwlock_tryrdlock( lck );
    }
    else if ( which == LCK_RW_TYPE_EXCLUSIVE )
    {
        trylock = pthread_rwlock_trywrlock( lck );
    }
    else
    {
        assert(0);
    }

    return trylock;
}

void lf_lck_rw_lock_exclusive_to_shared( pthread_rwlock_t* lck)
{
    lf_lck_rw_unlock_exclusive( lck );
    lf_lck_rw_lock_shared( lck );
}

bool lf_lck_rw_lock_shared_to_exclusive( pthread_rwlock_t* lck)
{
    lf_lck_rw_unlock_shared( lck );
    lf_lck_rw_lock_exclusive( lck );

    return true;
}

void lf_cond_init( pthread_cond_t* cond )
{
    errno_t err = pthread_cond_init( cond, NULL );
    assert( err == 0 );
}

void lf_cond_destroy( pthread_cond_t* cond )
{
    errno_t err = pthread_cond_destroy( cond );
    assert( err == 0 );
}

int lf_cond_wait_relative(pthread_cond_t *pCond, pthread_mutex_t *pMutex, struct timespec *pTime) {
    
    int iErr = pthread_cond_timedwait_relative_np(pCond, pMutex, pTime);
    assert((iErr == 0) || (iErr == ETIMEDOUT));
    return(iErr);
}

void lf_cond_wakeup(pthread_cond_t *pCond) {
    
    int iErr = pthread_cond_signal(pCond);
    assert(iErr == 0);
}

void lf_lck_mtx_init( pthread_mutex_t* lck )
{
    errno_t err = pthread_mutex_init( lck, NULL );
    assert( err == 0 );
}

void lf_lck_mtx_destroy( pthread_mutex_t *lck )
{
    errno_t err = pthread_mutex_destroy( lck );
    assert( err == 0 );
}

void lf_lck_mtx_lock( pthread_mutex_t* lck )
{
    errno_t err = pthread_mutex_lock( lck );
    assert( err == 0 );
}

void lf_lck_mtx_unlock( pthread_mutex_t* lck )
{
    errno_t err = pthread_mutex_unlock( lck );
    assert( err == 0 );
}

void lf_lck_mtx_lock_spin( pthread_mutex_t *lck )
{
    // No real spin lock
    lf_lck_mtx_lock( lck );
}

int lf_lck_mtx_try_lock(pthread_mutex_t *lck) {
    errno_t err = pthread_mutex_trylock(lck);
    return err;
}

//void lf_lck_mtx_convert_spin( pthread_mutex_t *lck )
//{
//    // No real spin lock
//}

void lf_lck_spin_init( pthread_mutex_t *lck )
{
    errno_t err = pthread_mutex_init( lck, NULL );
    assert( err == 0 );
}

void lf_lck_spin_destroy( pthread_mutex_t *lck )
{
    errno_t err = pthread_mutex_destroy( lck );
    assert( err == 0 );
}

void lf_lck_spin_lock( pthread_mutex_t *lck )
{
    errno_t err = pthread_mutex_lock( lck );
    assert( err == 0 );
}

void lf_lck_spin_unlock( pthread_mutex_t *lck )
{
    errno_t err = pthread_mutex_unlock( lck );
    assert( err == 0 );
}

lck_attr_t *lf_lck_attr_alloc_init( void )
{
    static lck_attr_t attr = {0};
    return &attr;
}
lck_grp_attr_t *lf_lck_grp_attr_alloc_init( void )
{
    static lck_grp_attr_t group_attr = {0};
    return &group_attr;
}
lck_grp_t *lf_lck_grp_alloc_init( void )
{
    static lck_grp_t group = {0};
    return &group;
}
