/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
/*
 *  CFNetworkThreadSupport.c
 *  CFNetwork
 *
 *  Created by Jeremy Wyld on 11/4/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "CFNetworkThreadSupport.h"

#ifdef __WIN32__
#include <process.h>
#endif /* __WIN32__ */


#if !defined(__WIN32__)

extern void _CFMutexInit(_CFMutex *lock, Boolean recursive) {
    if (recursive) {
        pthread_mutexattr_t attrs;
        pthread_mutexattr_init(&attrs);
        pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(lock, &attrs);
    } else
        pthread_mutex_init(lock, NULL);
}

#else

// Cribbed from CFUtilities.c


struct _args {
    void *func;
    void *arg;
    HANDLE handle;
};

static __stdcall unsigned __CFWinThreadFunc(void *arg) {
    struct _args *args = arg;
    ((void (*)(void *))args->func)(args->arg);
    CloseHandle(args->handle);
    CFAllocatorDeallocate(kCFAllocatorSystemDefault, arg);
    _endthreadex(0);
    return 0;
}

extern int _CFThreadSpawn(_CFThread *thread, void *(*func)(void *), void *arg) {
    unsigned tid;
    struct _args *args = CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(struct _args), 0);
    args->func = func;
    args->arg = arg;
    /* The thread is created suspended, because otherwise there would be a race between the assignment below of the handle field, and it's possible use in the thread func above. */
    unsigned long fauxHandle = _beginthreadex(NULL, 0, __CFWinThreadFunc, args, CREATE_SUSPENDED, &tid);
    args->handle = (HANDLE)fauxHandle;
    *thread = args->handle;
    ResumeThread(*thread);
    return *thread != 0;
}

#endif
