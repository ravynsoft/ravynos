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
 * Interface for the DSEventSemaphore (gating lock) class.
 */

#ifndef _DSEventSemaphore_H_
#define _DSEventSemaphore_H_

#include <unistd.h>		// for _POSIX_THREADS
#include <pthread.h>	// for pthread_*_t
#include <DirectoryServiceCore/PrivateTypes.h>

class DSEventSemaphore
{
public:
	/**** Instance methods. ****/
	// ctor and dtor.
            DSEventSemaphore    ( void );
            ~DSEventSemaphore   ( void );

    void    PostEvent           ( void );
    void    ResetEvent          ( void );
    
    // returns true if got event, otherwise false if it timed out
    // pass 0 to wait forever
	bool    WaitForEvent        ( SInt32 milliSecs = 0 );

protected:
    /**** Instance variables. ****/
    pthread_mutex_t		fMutex;
	pthread_cond_t		fCondition;
    bool                fbEvent;
	SInt32				fMilliSecsTotal;
};

#endif /* _DSEventSemaphore_H_ */
