/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
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

/*!
 * @header DSEventSemaphore
 * Implementation of the DSEventSemaphore (gating lock) class.
 */

#include <errno.h>
#include <sys/time.h>	// for struct timespec and gettimeofday();

#include "DSEventSemaphore.h"

DSEventSemaphore::DSEventSemaphore( void )
{
    fbEvent = false;
	fMilliSecsTotal = 0;
    pthread_mutex_init( &fMutex, NULL );
    pthread_cond_init( &fCondition, NULL );
}

DSEventSemaphore::~DSEventSemaphore( void )
{
    pthread_mutex_destroy( &fMutex );
    pthread_cond_destroy( &fCondition );
}

bool DSEventSemaphore::WaitForEvent( SInt32 milliSecs )
{
    pthread_mutex_lock( &fMutex );

    // we only lock if we didn't have a broadcast
    if( fbEvent == false )
    {
        if( milliSecs == 0 ) // wait forever
        {
			// pthread_cond states we should check our predicate boolean to ensure we didn't get spuriously woken
			while ( fbEvent == false ) pthread_cond_wait( &fCondition, &fMutex );
        }
        else if( milliSecs > 0 )
        {
			// if we've already exceeded how long we are willing to wait, we just return false
			if (fMilliSecsTotal >= milliSecs)
			{
				pthread_mutex_unlock( &fMutex );
				return false;
			}
			else
			{
				struct timeval	tvNow;
				struct timespec	tsTimeout;
				
				gettimeofday( &tvNow, NULL );
				TIMEVAL_TO_TIMESPEC ( &tvNow, &tsTimeout );
				tsTimeout.tv_sec += (milliSecs / 1000);
				tsTimeout.tv_nsec += ((milliSecs % 1000) * 1000000);
				
				if( pthread_cond_timedwait(&fCondition, &fMutex, &tsTimeout) == ETIMEDOUT )
				{
					// we grab the lock as a result of the timeout
					bool bReturn = fbEvent;
					
					fMilliSecsTotal += milliSecs; // be sure to increment how long we waited
					pthread_mutex_unlock( &fMutex );
					return bReturn;
				}
			}
        }
    }

    pthread_mutex_unlock( &fMutex );

	return true;
}

void DSEventSemaphore::PostEvent( void )
{
    pthread_mutex_lock( &fMutex );
    fbEvent = true;
    pthread_cond_broadcast( &fCondition );
    pthread_mutex_unlock( &fMutex );
} 

void DSEventSemaphore::ResetEvent( void )
{
    pthread_mutex_lock( &fMutex );
    fbEvent = false;
	fMilliSecsTotal = 0;
    pthread_mutex_unlock( &fMutex );
}
