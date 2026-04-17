/* Copyright © 2017 Apple Inc. All rights reserved.
 *
 *  MR_SW_Lock.c
 *  usbstorage_plugin
 */

#include <assert.h>
#include <errno.h>
#include "MR_SW_Lock.h"

void MultiReadSingleWrite_Init ( MultiReadSingleWriteHandler_s* psMultiReadSingleWriteHandler )
{
    errno_t err = pthread_rwlock_init( &psMultiReadSingleWriteHandler->sRWLock, NULL );
    assert( err == 0 );
}

void MultiReadSingleWrite_DeInit ( MultiReadSingleWriteHandler_s* psMultiReadSingleWriteHandler )
{
    errno_t err = pthread_rwlock_destroy( &psMultiReadSingleWriteHandler->sRWLock );
    assert( err == 0 );
}

void MultiReadSingleWrite_LockRead ( MultiReadSingleWriteHandler_s* psMultiReadSingleWriteHandler )
{
    errno_t err = pthread_rwlock_rdlock( &psMultiReadSingleWriteHandler->sRWLock );
    assert( err == 0 );
}

void MultiReadSingleWrite_FreeRead ( MultiReadSingleWriteHandler_s* psMultiReadSingleWriteHandler )
{
    errno_t err = pthread_rwlock_unlock( &psMultiReadSingleWriteHandler->sRWLock );
    assert( err == 0 );
}

void MultiReadSingleWrite_LockWrite ( MultiReadSingleWriteHandler_s* psMultiReadSingleWriteHandler )
{
    errno_t err = pthread_rwlock_wrlock( &psMultiReadSingleWriteHandler->sRWLock );
    assert( err == 0 );
}

bool MultiReadSingleWrite_TryLockWrite ( MultiReadSingleWriteHandler_s* psMultiReadSingleWriteHandler )
{
    errno_t err = pthread_rwlock_trywrlock( &psMultiReadSingleWriteHandler->sRWLock );
    return err == 0;
}

void MultiReadSingleWrite_FreeWrite ( MultiReadSingleWriteHandler_s* psMultiReadSingleWriteHandler )
{
    errno_t err = pthread_rwlock_unlock( &psMultiReadSingleWriteHandler->sRWLock );
    assert( err == 0 );
}
