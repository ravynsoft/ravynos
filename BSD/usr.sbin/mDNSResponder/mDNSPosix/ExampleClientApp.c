/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2002-2004 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>          // For printf()
#include <stdlib.h>         // For exit() etc.
#include <string.h>         // For strlen() etc.
#include <unistd.h>         // For select()
#include <errno.h>          // For errno, EINTR
#include <netinet/in.h>     // For INADDR_NONE
#include <arpa/inet.h>      // For inet_addr()
#include <netdb.h>          // For gethostbyname()
#include <signal.h>         // For SIGINT, etc.

#include "mDNSEmbeddedAPI.h"  // Defines the interface to the client layer above
#include "mDNSPosix.h"      // Defines the specific types needed to run mDNS on this platform

//*******************************************************************************************
// Main

static volatile mDNSBool StopNow;

mDNSlocal void HandleSIG(int signal)
{
    (void)signal;   // Unused
    debugf("%s","");
    debugf("HandleSIG");
    StopNow = mDNStrue;
}

mDNSexport void ExampleClientEventLoop(mDNS *const m)
{
    signal(SIGINT, HandleSIG);  // SIGINT is what you get for a Ctrl-C
    signal(SIGTERM, HandleSIG);

    while (!StopNow)
    {
        int nfds = 0;
        fd_set readfds;
        fd_set writefds;
        struct timeval timeout;
        int result;

        // 1. Set up the fd_set as usual here.
        // This example client has no file descriptors of its own,
        // but a real application would call FD_SET to add them to the set here
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);

        // 2. Set up the timeout.
        // This example client has no other work it needs to be doing,
        // so we set an effectively infinite timeout
        timeout.tv_sec = FutureTime;
        timeout.tv_usec = 0;

        // 3. Give the mDNSPosix layer a chance to add its information to the fd_set and timeout
        mDNSPosixGetFDSet(m, &nfds, &readfds, &writefds, &timeout);

        // 4. Call select as normal
        verbosedebugf("select(%d, %d.%06d)", nfds, timeout.tv_sec, timeout.tv_usec);
        result = select(nfds, &readfds, &writefds, NULL, &timeout);

        if (result < 0)
        {
            verbosedebugf("select() returned %d errno %d", result, errno);
            if (errno != EINTR) StopNow = mDNStrue;
        }
        else
        {
            // 5. Call mDNSPosixProcessFDSet to let the mDNSPosix layer do its work
            mDNSPosixProcessFDSet(m, &readfds, &writefds);

            // 6. This example client has no other work it needs to be doing,
            // but a real client would do its work here
            // ... (do work) ...
        }
    }

    debugf("Exiting");
}
