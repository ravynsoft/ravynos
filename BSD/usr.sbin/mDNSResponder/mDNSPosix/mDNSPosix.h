/* -*- Mode: C; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108; indent-tabs-mode: nil; -*-
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

#ifndef __mDNSPlatformPosix_h
#define __mDNSPlatformPosix_h

#include <signal.h>
#include <sys/time.h>

#ifdef  __cplusplus
extern "C" {
#endif

// PosixNetworkInterface is a record extension of the core NetworkInterfaceInfo
// type that supports extra fields needed by the Posix platform.
//
// IMPORTANT: coreIntf must be the first field in the structure because
// we cast between pointers to the two different types regularly.

typedef struct PosixNetworkInterface PosixNetworkInterface;

struct PosixNetworkInterface
{
    NetworkInterfaceInfo coreIntf;      // MUST be the first element in this structure
    mDNSs32 LastSeen;
    const char *            intfName;
    PosixNetworkInterface * aliasIntf;
    int index;
    int multicastSocket4;
#if HAVE_IPV6
    int multicastSocket6;
#endif
};

// This is a global because debugf_() needs to be able to check its value
extern int gMDNSPlatformPosixVerboseLevel;

struct mDNS_PlatformSupport_struct
{
    int unicastSocket4;
#if HAVE_IPV6
    int unicastSocket6;
#endif
};

// We keep a list of client-supplied event sources in PosixEventSource records
// Add a file descriptor to the set that mDNSPosixRunEventLoopOnce() listens to.
#define PosixEventFlag_OnList   1
#define PosixEventFlag_Read     2
#define PosixEventFlag_Write    4
    
typedef void (*mDNSPosixEventCallback)(int fd, void *context);
struct PosixEventSource
{
    struct PosixEventSource *next;
    mDNSPosixEventCallback readCallback;
    mDNSPosixEventCallback writeCallback;
    const char *readTaskName;
    const char *writeTaskName;
    void *readContext;
    void *writeContext;
    int fd;
    unsigned flags;
};
typedef struct PosixEventSource PosixEventSource;
    
struct TCPSocket_struct
{
    mDNSIPPort port;            // MUST BE FIRST FIELD -- mDNSCore expects every TCPSocket_struct to begin with mDNSIPPort
    TCPSocketFlags flags;       // MUST BE SECOND FIELD -- mDNSCore expects every TCPSocket_struct have TCPSocketFlags flags after mDNSIPPort
    TCPConnectionCallback callback;
    PosixEventSource events;
    // SSL context goes here.
    domainname *hostname;
    mDNSAddr remoteAddress;
    mDNSIPPort remotePort;
    void *context;
    mDNSBool setup;
    mDNSBool connected;
    mStatus err;
};

struct TCPListener_struct
{
    TCPAcceptedCallback callback;
    PosixEventSource events;
    void *context;
    mDNSAddr_Type addressType;
    TCPSocketFlags socketFlags;
};
    
#define uDNS_SERVERS_FILE "/etc/resolv.conf"
extern int ParseDNSServers(mDNS *m, const char *filePath);
extern mStatus mDNSPlatformPosixRefreshInterfaceList(mDNS *const m);
// See comment in implementation.

// Get the next upcoming mDNS (or DNS) event time as a posix timeval that can be passed to select.
// This will only update timeout if the next mDNS event is sooner than the value that was passed.
// Therefore, use { FutureTime, 0 } as an initializer if no other timer events are being managed.
extern void mDNSPosixGetNextDNSEventTime(mDNS *m, struct timeval *timeout);

// Returns all the FDs that the posix I/O event system expects to be passed to select.
extern void mDNSPosixGetFDSetForSelect(mDNS *m, int *nfds, fd_set *readfds, fd_set *writefds);

// Call mDNSPosixGetFDSet before calling select(), to update the parameters
// as may be necessary to meet the needs of the mDNSCore code.
// The timeout pointer MUST NOT be NULL.
// Set timeout->tv_sec to FutureTime if you want to have effectively no timeout
// After calling mDNSPosixGetFDSet(), call select(nfds, &readfds, NULL, NULL, &timeout); as usual
// After select() returns, call mDNSPosixProcessFDSet() to let mDNSCore do its work
// mDNSPosixGetFDSet simply calls mDNSPosixGetNextDNSEventTime and then mDNSPosixGetFDSetForSelect.
extern void mDNSPosixGetFDSet(mDNS *m, int *nfds, fd_set *readfds, fd_set *writefds, struct timeval *timeout);


extern void mDNSPosixProcessFDSet(mDNS *const m, fd_set *readfds, fd_set *writefds);

extern mStatus mDNSPosixAddFDToEventLoop( int fd, mDNSPosixEventCallback callback, void *context);
extern mStatus mDNSPosixRemoveFDFromEventLoop( int fd);
extern mStatus mDNSPosixListenForSignalInEventLoop( int signum);
extern mStatus mDNSPosixIgnoreSignalInEventLoop( int signum);
extern mStatus mDNSPosixRunEventLoopOnce( mDNS *m, const struct timeval *pTimeout, sigset_t *pSignalsReceived, mDNSBool *pDataDispatched);

extern mStatus mDNSPosixListenForSignalInEventLoop( int signum);
extern mStatus mDNSPosixIgnoreSignalInEventLoop( int signum);
extern mStatus mDNSPosixRunEventLoopOnce( mDNS *m, const struct timeval *pTimeout, sigset_t *pSignalsReceived, mDNSBool *pDataDispatched);

#ifdef  __cplusplus
}
#endif

#endif
