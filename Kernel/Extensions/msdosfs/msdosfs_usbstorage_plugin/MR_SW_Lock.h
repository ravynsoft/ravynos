/* Copyright © 2017 Apple Inc. All rights reserved.
 *
 *  MR_SW_Lock.h
 *  usbstorage_plugin
 */

#ifndef MR_SW_Lock_h
#define MR_SW_Lock_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>


typedef struct {
    pthread_rwlock_t    sRWLock;
    
} MultiReadSingleWriteHandler_s;

void MultiReadSingleWrite_Init       ( MultiReadSingleWriteHandler_s* psMultiReadSingleWriteHandler );
void MultiReadSingleWrite_DeInit     ( MultiReadSingleWriteHandler_s* psMultiReadSingleWriteHandler );
void MultiReadSingleWrite_LockRead   ( MultiReadSingleWriteHandler_s* psMultiReadSingleWriteHandler );
void MultiReadSingleWrite_FreeRead   ( MultiReadSingleWriteHandler_s* psMultiReadSingleWriteHandler );
void MultiReadSingleWrite_LockWrite  ( MultiReadSingleWriteHandler_s* psMultiReadSingleWriteHandler );
bool MultiReadSingleWrite_TryLockWrite ( MultiReadSingleWriteHandler_s* psMultiReadSingleWriteHandler );
void MultiReadSingleWrite_FreeWrite  ( MultiReadSingleWriteHandler_s* psMultiReadSingleWriteHandler );

#endif /* MR_SW_Lock_h */
