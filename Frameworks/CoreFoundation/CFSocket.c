/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

/*	CFSocket.c
	Copyright (c) 1999-2014, Apple Inc.  All rights reserved.
	Responsibility: Christopher Kane
*/

#define NEW_SOCKET 0

#if NEW_SOCKET
/*

#include <CoreFoundation/CFSocket.h>
#include "CFInternal.h"
#include <dispatch/dispatch.h>
#include <dispatch/private.h>
#include <netinet/in.h>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/select.h>


extern void _CFRunLoopSourceWakeUpRunLoops(CFRunLoopSourceRef rls);

#define INVALID_SOCKET (CFSocketNativeHandle)(-1)
#define MAX_SOCKADDR_LEN 256


DISPATCH_HELPER_FUNCTIONS(sock, CFSocket)

static Boolean sockfd_is_readable(int fd) {
    if (fd < 0 || 1048576 <= fd) HALT;
    size_t sz = ((fd + CHAR_BIT) / CHAR_BIT) + 7; // generous
    fd_set *fdset = malloc(sz);
    int ret;
    do {
        memset(fdset, 0, sz);
        FD_SET(fd, fdset);
        struct timespec ts = {0, 1000UL}; // 1 us
        ret = pselect(fd + 1, fdset, NULL, NULL, &ts, NULL);
    } while (ret < 0 && (EINTR == errno || EAGAIN == errno));
    Boolean isSet = ((0 < ret) && FD_ISSET(fd, fdset));
    free(fdset);
    return isSet;
}

static Boolean sockfd_is_writeable(int fd) {
    if (fd < 0 || 1048576 <= fd) HALT;
    size_t sz = ((fd + CHAR_BIT) / CHAR_BIT) + 7; // generous
    fd_set *fdset = malloc(sz);
    int ret;
    do {
        memset(fdset, 0, sz);
        FD_SET(fd, fdset);
        struct timespec ts = {0, 1000UL}; // 1 us
        ret = pselect(fd + 1, NULL, fdset, NULL, &ts, NULL);
    } while (ret < 0 && (EINTR == errno || EAGAIN == errno));
    Boolean isSet = ((0 < ret) && FD_ISSET(fd, fdset));
    free(fdset);
    return isSet;
}


enum {
    kCFSocketStateReady = 0,
    kCFSocketStateInvalidating = 1,
    kCFSocketStateInvalid = 2,
    kCFSocketStateDeallocating = 3
};

struct __shared_blob {
    dispatch_source_t _rdsrc;
    dispatch_source_t _wrsrc;
    CFRunLoopSourceRef _source;
    CFSocketNativeHandle _socket;
    uint8_t _closeFD;
    uint8_t _refCnt;
};

struct __CFSocket {
    CFRuntimeBase _base;
    struct __shared_blob *_shared; // non-NULL when valid, NULL when invalid

    uint8_t _state:2;         // mutable, not written safely
    uint8_t _isSaneFD:1;      // immutable
    uint8_t _connOriented:1;  // immutable
    uint8_t _wantConnect:1;   // immutable
    uint8_t _wantWrite:1;     // immutable
    uint8_t _wantReadType:2;  // immutable

    uint8_t _error;

    uint8_t _rsuspended:1;
    uint8_t _wsuspended:1;
    uint8_t _readable:1;
    uint8_t _writeable:1;
    uint8_t _unused:4;

    uint8_t _reenableRead:1;
    uint8_t _readDisabled:1;
    uint8_t _reenableWrite:1;
    uint8_t _writeDisabled:1;
    uint8_t _connectDisabled:1;
    uint8_t _connected:1;
    uint8_t _leaveErrors:1;
    uint8_t _closeOnInvalidate:1;

    int32_t _runLoopCounter;

    CFDataRef _address;         // immutable, once created
    CFDataRef _peerAddress;     // immutable, once created
    CFSocketCallBack _callout;  // immutable
    CFSocketContext _context;   // immutable
};


CF_INLINE Boolean __CFSocketIsValid(CFSocketRef sock) {
    return kCFSocketStateReady == sock->_state;
}

static CFStringRef __CFSocketCopyDescription(CFTypeRef cf) {
    CFSocketRef sock = (CFSocketRef)cf;
    CFStringRef contextDesc = NULL;
    if (NULL != sock->_context.info && NULL != sock->_context.copyDescription) {
        contextDesc = sock->_context.copyDescription(sock->_context.info);
    }
    if (NULL == contextDesc) {
        contextDesc = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFSocket context %p>"), sock->_context.info);
    }
    Dl_info info;
    void *addr = sock->_callout;
    const char *name = (dladdr(addr, &info) && info.dli_saddr == addr && info.dli_sname) ? info.dli_sname : "???";
    int avail = -1;
    ioctlsocket(sock->_shared ? sock->_shared->_socket : -1, FIONREAD, &avail);
    CFStringRef result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR(
            "<CFSocket %p [%p]>{valid = %s, socket = %d, "
            "want connect = %s, connect disabled = %s, "
            "want write = %s, reenable write = %s, write disabled = %s, "
            "want read = %s, reenable read = %s, read disabled = %s, "
            "leave errors = %s, close on invalidate = %s, connected = %s, "
            "last error code = %d, bytes available for read = %d, "
            "source = %p, callout = %s (%p), context = %@}"),
        cf, CFGetAllocator(sock), __CFSocketIsValid(sock) ? "Yes" : "No", sock->_shared ? sock->_shared->_socket : -1,
        sock->_wantConnect ? "Yes" : "No", sock->_connectDisabled ? "Yes" : "No",
        sock->_wantWrite ? "Yes" : "No", sock->_reenableWrite ? "Yes" : "No", sock->_writeDisabled ? "Yes" : "No",
        sock->_wantReadType ? "Yes" : "No", sock->_reenableRead ? "Yes" : "No", sock->_readDisabled? "Yes" : "No",
        sock->_leaveErrors ? "Yes" : "No", sock->_closeOnInvalidate ? "Yes" : "No", sock->_connected ? "Yes" : "No",
        sock->_error, avail,
        sock->_shared ? sock->_shared->_source : NULL, name, addr, contextDesc);
    if (NULL != contextDesc) {
        CFRelease(contextDesc);
    }
    return result;
}

static void __CFSocketDeallocate(CFTypeRef cf) {
    CHECK_FOR_FORK_RET();
    CFSocketRef sock = (CFSocketRef)cf;
    // Since CFSockets are cached, we can only get here sometime after being invalidated
    sock->_state = kCFSocketStateDeallocating;
    if (sock->_peerAddress) {
        CFRelease(sock->_peerAddress);
        sock->_peerAddress = NULL;
    }
    if (sock->_address) {
        CFRelease(sock->_address);
        sock->_address = NULL;
    }
}

static CFTypeID __kCFSocketTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFSocketClass = {
    0,
    "CFSocket",
    NULL,      // init
    NULL,      // copy
    __CFSocketDeallocate,
    NULL,      // equal
    NULL,      // hash
    NULL,      // 
    __CFSocketCopyDescription
};

static CFMutableArrayRef __CFAllSockets = NULL;

CFTypeID CFSocketGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{
        __kCFSocketTypeID = _CFRuntimeRegisterClass(&__CFSocketClass); // initOnce covered
        __CFAllSockets = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
        struct rlimit lim1;
        int ret1 = getrlimit(RLIMIT_NOFILE, &lim1);
        int mib[] = {CTL_KERN, KERN_MAXFILESPERPROC};
        int maxfd = 0;
        size_t len = sizeof(int);
        int ret0 = sysctl(mib, 2, &maxfd, &len, NULL, 0);
        if (0 == ret0 && 0 == ret1 && lim1.rlim_max < maxfd) maxfd = lim1.rlim_max;
        if (0 == ret1 && lim1.rlim_cur < maxfd) {
            struct rlimit lim2 = lim1;
            lim2.rlim_cur += 2304;
            if (maxfd < lim2.rlim_cur) lim2.rlim_cur = maxfd;
            setrlimit(RLIMIT_NOFILE, &lim2);
            // we try, but do not go to extraordinary measures
        }
    });
    return __kCFSocketTypeID;
}

CFSocketRef CFSocketCreateWithNative(CFAllocatorRef allocator, CFSocketNativeHandle ufd, CFOptionFlags callBackTypes, CFSocketCallBack callout, const CFSocketContext *context) {
    CHECK_FOR_FORK_RET(NULL);

    CFSocketGetTypeID(); // cause initialization if necessary

    struct stat statbuf;
    int ret = fstat(ufd, &statbuf);
    if (ret < 0) ufd = INVALID_SOCKET;

    Boolean sane = false;
    if (INVALID_SOCKET != ufd) {
        uint32_t type = (statbuf.st_mode & S_IFMT);
        sane = (S_IFSOCK == type) || (S_IFIFO == type) || (S_IFCHR == type);
        if (1 && !sane) {
            CFLog(kCFLogLevelWarning, CFSTR("*** CFSocketCreateWithNative(): creating CFSocket with silly fd type (%07o) -- may or may not work"), type);
        }
    }

    if (INVALID_SOCKET != ufd) {
        Boolean canHandle = false;
        int tmp_kq = kqueue();
        if (0 <= tmp_kq) {
            struct kevent ev[2];
            EV_SET(&ev[0], ufd, EVFILT_READ, EV_ADD, 0, 0, 0);
            EV_SET(&ev[1], ufd, EVFILT_WRITE, EV_ADD, 0, 0, 0);
            int ret = kevent(tmp_kq, ev, 2, NULL, 0, NULL);
            canHandle = (0 <= ret); // if kevent(ADD) succeeds, can handle
            close(tmp_kq);
        }
        if (1 && !canHandle) {
            CFLog(kCFLogLevelWarning, CFSTR("*** CFSocketCreateWithNative(): creating CFSocket with unsupported fd type -- may or may not work"));
        }
    }

    if (INVALID_SOCKET == ufd) {
        // Historically, bad ufd was allowed, but gave an uncached and already-invalid CFSocketRef
        SInt32 size = sizeof(struct __CFSocket) - sizeof(CFRuntimeBase);
        CFSocketRef memory = (CFSocketRef)_CFRuntimeCreateInstance(allocator, CFSocketGetTypeID(), size, NULL);
        if (NULL == memory) {
            return NULL;
        }
        memory->_callout = callout;
        memory->_state = kCFSocketStateInvalid;
        return memory;
    }

    __block CFSocketRef sock = NULL;
    dispatch_sync(__sockQueue(), ^{
            for (CFIndex idx = 0, cnt = CFArrayGetCount(__CFAllSockets); idx < cnt; idx++) {
                CFSocketRef s = (CFSocketRef)CFArrayGetValueAtIndex(__CFAllSockets, idx);
                if (s->_shared->_socket == ufd) {
                    CFRetain(s);
                    sock = s;
                    return;
                }
            }

            SInt32 size = sizeof(struct __CFSocket) - sizeof(CFRuntimeBase);
            CFSocketRef memory = (CFSocketRef)_CFRuntimeCreateInstance(allocator, CFSocketGetTypeID(), size, NULL);
            if (NULL == memory) {
                return;
            }

            int socketType = 0;
            if (INVALID_SOCKET != ufd) {
                socklen_t typeSize = sizeof(socketType);
                int ret = getsockopt(ufd, SOL_SOCKET, SO_TYPE, (void *)&socketType, (socklen_t *)&typeSize);
                if (ret < 0) socketType = 0;
            }

            memory->_rsuspended = true;
            memory->_wsuspended = true;
            memory->_readable = false;
            memory->_writeable = false;

            memory->_isSaneFD = sane ? 1 : 0;
            memory->_wantReadType = (callBackTypes & 0x3);
            memory->_reenableRead = memory->_wantReadType ? true : false;
            memory->_readDisabled = false;
            memory->_wantWrite = (callBackTypes & kCFSocketWriteCallBack) ? true : false;
            memory->_reenableWrite = false;
            memory->_writeDisabled = false;
            memory->_wantConnect = (callBackTypes & kCFSocketConnectCallBack) ? true : false;
            memory->_connectDisabled = false;
            memory->_leaveErrors = false;
            memory->_closeOnInvalidate = true;
            memory->_connOriented = (SOCK_STREAM == socketType || SOCK_SEQPACKET == socketType);
            memory->_connected = (memory->_wantReadType == kCFSocketAcceptCallBack || !memory->_connOriented) ? true : false;

            memory->_error = 0;
            memory->_runLoopCounter = 0;
            memory->_address = NULL;
            memory->_peerAddress = NULL;
            memory->_context.info = NULL;
            memory->_context.retain = NULL;
            memory->_context.release = NULL;
            memory->_context.copyDescription = NULL;
            memory->_callout = callout;
            if (NULL != context) {
                objc_memmove_collectable(&memory->_context, context, sizeof(CFSocketContext));
                memory->_context.info = context->retain ? (void *)context->retain(context->info) : context->info;
            }

            struct __shared_blob *shared = malloc(sizeof(struct __shared_blob));
            shared->_rdsrc = NULL;
            shared->_wrsrc = NULL;
            shared->_source = NULL;
            shared->_socket = ufd;
            shared->_closeFD = true; // copy of _closeOnInvalidate
            shared->_refCnt = 1; // one for the CFSocket
            memory->_shared = shared;

            if (memory->_wantReadType) {
                dispatch_source_t dsrc = NULL;
                if (sane) {
                    dsrc = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, ufd, 0, __sockQueue());
                } else {
                    dsrc = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, __sockQueue());
                    dispatch_source_set_timer(dsrc, dispatch_time(DISPATCH_TIME_NOW, 0), NSEC_PER_SEC / 2, NSEC_PER_SEC);
                }
                dispatch_block_t event_block = ^{
                        memory->_readable = true;
                        if (!memory->_rsuspended) {
                            dispatch_suspend(dsrc);
// CFLog(5, CFSTR("suspend %p due to read event block"), memory);
                            memory->_rsuspended = true;
                        }
                        if (shared->_source) {
                            CFRunLoopSourceSignal(shared->_source);
                            _CFRunLoopSourceWakeUpRunLoops(shared->_source);
                        }
                    };
                dispatch_block_t cancel_block = ^{
                        shared->_rdsrc = NULL;
                        shared->_refCnt--;
                        if (0 == shared->_refCnt) {
                            if (shared->_closeFD) {
                                // thoroughly stop anything else from using the fd
                                (void)shutdown(shared->_socket, SHUT_RDWR);
                                int nullfd = open("/dev/null", O_RDONLY);
                                dup2(nullfd, shared->_socket);
                                close(nullfd);
                                close(shared->_socket);
                            }
                            free(shared);
                        }
                        dispatch_release(dsrc);
                    };
                dispatch_source_set_event_handler(dsrc, event_block);
                dispatch_source_set_cancel_handler(dsrc, cancel_block);
                shared->_rdsrc = dsrc;
            }
            if (memory->_wantWrite || memory->_wantConnect) {
                dispatch_source_t dsrc = NULL;
                if (sane) {
                    dsrc = dispatch_source_create(DISPATCH_SOURCE_TYPE_WRITE, ufd, 0, __sockQueue());
                } else {
                    dsrc = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, __sockQueue());
                    dispatch_source_set_timer(dsrc, dispatch_time(DISPATCH_TIME_NOW, 0), NSEC_PER_SEC / 2, NSEC_PER_SEC);
                }
                dispatch_block_t event_block = ^{
                        memory->_writeable = true;
                        if (!memory->_wsuspended) {
                            dispatch_suspend(dsrc);
// CFLog(5, CFSTR("suspend %p due to write event block"), memory);
                            memory->_wsuspended = true;
                        }
                        if (shared->_source) {
                            CFRunLoopSourceSignal(shared->_source);
                            _CFRunLoopSourceWakeUpRunLoops(shared->_source);
                        }
                    };
                dispatch_block_t cancel_block = ^{
                        shared->_wrsrc = NULL;
                        shared->_refCnt--;
                        if (0 == shared->_refCnt) {
                            if (shared->_closeFD) {
                                // thoroughly stop anything else from using the fd
                                (void)shutdown(shared->_socket, SHUT_RDWR);
                                int nullfd = open("/dev/null", O_RDONLY);
                                dup2(nullfd, shared->_socket);
                                close(nullfd);
                                close(shared->_socket);
                            }
                            free(shared);
                        }
                        dispatch_release(dsrc);
                    };
                dispatch_source_set_event_handler(dsrc, event_block);
                dispatch_source_set_cancel_handler(dsrc, cancel_block);
                shared->_wrsrc = dsrc;
            }

            if (shared->_rdsrc) {
                shared->_refCnt++;
            }
            if (shared->_wrsrc) {
                shared->_refCnt++;
            }

            memory->_state = kCFSocketStateReady;
            CFArrayAppendValue(__CFAllSockets, memory);
            sock = memory;
        });
// CFLog(5, CFSTR("CFSocketCreateWithNative(): created socket %p with callbacks 0x%x"), sock, callBackTypes);
    if (sock && !CFSocketIsValid(sock)) { // must do this outside lock to avoid deadlock
        CFRelease(sock);
        sock = NULL;
    }
    return sock;
}

CFSocketNativeHandle CFSocketGetNative(CFSocketRef sock) {
    CHECK_FOR_FORK_RET(INVALID_SOCKET);
    __CFGenericValidateType(sock, CFSocketGetTypeID());
    return sock->_shared ? sock->_shared->_socket : INVALID_SOCKET;
}

void CFSocketGetContext(CFSocketRef sock, CFSocketContext *context) {
    __CFGenericValidateType(sock, CFSocketGetTypeID());
    CFAssert1(0 == context->version, __kCFLogAssertion, "%s(): context version not initialized to 0", __PRETTY_FUNCTION__);
    objc_memmove_collectable(context, &sock->_context, sizeof(CFSocketContext));
}

CFDataRef CFSocketCopyAddress(CFSocketRef sock) {
    CHECK_FOR_FORK_RET(NULL);
    __CFGenericValidateType(sock, CFSocketGetTypeID());
    __block CFDataRef result = NULL;
    dispatch_sync(__sockQueue(), ^{
            if (!sock->_address) {
                if (!__CFSocketIsValid(sock)) return;
                uint8_t name[MAX_SOCKADDR_LEN];
                socklen_t namelen = sizeof(name);
                int ret = getsockname(sock->_shared->_socket, (struct sockaddr *)name, (socklen_t *)&namelen);
                if (0 == ret && 0 < namelen) {
                    sock->_address = CFDataCreate(CFGetAllocator(sock), name, namelen);
                }
            }
            result = sock->_address ? (CFDataRef)CFRetain(sock->_address) : NULL;
        });
    return result;
}

CFDataRef CFSocketCopyPeerAddress(CFSocketRef sock) {
    CHECK_FOR_FORK_RET(NULL);
    __CFGenericValidateType(sock, CFSocketGetTypeID());
    __block CFDataRef result = NULL;
    dispatch_sync(__sockQueue(), ^{
            if (!sock->_peerAddress) {
                if (!__CFSocketIsValid(sock)) return;
                uint8_t name[MAX_SOCKADDR_LEN];
                socklen_t namelen = sizeof(name);
                int ret = getpeername(sock->_shared->_socket, (struct sockaddr *)name, (socklen_t *)&namelen);
                if (0 == ret && 0 < namelen) {
                    sock->_peerAddress = CFDataCreate(CFGetAllocator(sock), name, namelen);
                }
            }
            result = sock->_peerAddress ? (CFDataRef)CFRetain(sock->_peerAddress) : NULL;
        });
    return result;
}

CFOptionFlags CFSocketGetSocketFlags(CFSocketRef sock) {
    CHECK_FOR_FORK();
    __CFGenericValidateType(sock, CFSocketGetTypeID());
    __block CFOptionFlags flags = 0;
    dispatch_sync(__sockQueue(), ^{
            if (sock->_reenableRead) flags |= sock->_wantReadType; // flags are same as types here
            if (sock->_reenableWrite) flags |= kCFSocketAutomaticallyReenableWriteCallBack;
            if (sock->_leaveErrors) flags |= kCFSocketLeaveErrors;
            if (sock->_closeOnInvalidate) flags |= kCFSocketCloseOnInvalidate;
        });
    return flags;
}

void CFSocketSetSocketFlags(CFSocketRef sock, CFOptionFlags flags) {
    CHECK_FOR_FORK();
// CFLog(5, CFSTR("CFSocketSetSocketFlags(%p, 0x%x) starting"), sock, flags);
    __CFGenericValidateType(sock, CFSocketGetTypeID());
    dispatch_sync(__sockQueue(), ^{
            sock->_reenableRead = (sock->_wantReadType && ((flags & 0x3) == sock->_wantReadType)) ? true : false;
            sock->_reenableWrite = (sock->_wantWrite && (flags & kCFSocketAutomaticallyReenableWriteCallBack)) ? true : false;
            sock->_leaveErrors = (flags & kCFSocketLeaveErrors) ? true : false;
            sock->_closeOnInvalidate = (flags & kCFSocketCloseOnInvalidate) ? true : false;
            if (sock->_shared) sock->_shared->_closeFD = sock->_closeOnInvalidate;
        });
// CFLog(5, CFSTR("CFSocketSetSocketFlags(%p, 0x%x) done"), sock, flags);
}

void CFSocketEnableCallBacks(CFSocketRef sock, CFOptionFlags callBackTypes) {
    CHECK_FOR_FORK_RET();
    __CFGenericValidateType(sock, CFSocketGetTypeID());
// CFLog(5, CFSTR("CFSocketEnableCallBacks(%p, 0x%x) starting"), sock, callBackTypes);
    dispatch_sync(__sockQueue(), ^{
            if (!__CFSocketIsValid(sock)) return;
            if (sock->_wantReadType && (callBackTypes & 0x3) == sock->_wantReadType) {
                if (sockfd_is_readable(sock->_shared->_socket)) {
                    sock->_readable = true;
// CFLog(5, CFSTR("CFSocketEnableCallBacks(%p, 0x%x) socket is readable"), sock, callBackTypes);
                    if (!sock->_rsuspended) {
                        dispatch_suspend(sock->_shared->_rdsrc);
                        sock->_rsuspended = true;
                    }
                    // If the source exists, but is now invalid, this next stuff is relatively harmless.
                    if (sock->_shared->_source) {
                        CFRunLoopSourceSignal(sock->_shared->_source);
                        _CFRunLoopSourceWakeUpRunLoops(sock->_shared->_source);
                    }
                } else if (sock->_rsuspended && sock->_shared->_rdsrc) {
                    sock->_rsuspended = false;
                    dispatch_resume(sock->_shared->_rdsrc);
                }
                sock->_readDisabled = false;
            }
            if (sock->_wantWrite && (callBackTypes & kCFSocketWriteCallBack)) {
                if (sockfd_is_writeable(sock->_shared->_socket)) {
                    sock->_writeable = true;
                    if (!sock->_wsuspended) {
                        dispatch_suspend(sock->_shared->_wrsrc);
                        sock->_wsuspended = true;
                    }
                    // If the source exists, but is now invalid, this next stuff is relatively harmless.
                    if (sock->_shared->_source) {
                        CFRunLoopSourceSignal(sock->_shared->_source);
                        _CFRunLoopSourceWakeUpRunLoops(sock->_shared->_source);
                    }
                } else if (sock->_wsuspended && sock->_shared->_wrsrc) {
                    sock->_wsuspended = false;
                    dispatch_resume(sock->_shared->_wrsrc);
                }
                sock->_writeDisabled = false;
            }
            if (sock->_wantConnect && !sock->_connected && (callBackTypes & kCFSocketConnectCallBack)) {
                if (sockfd_is_writeable(sock->_shared->_socket)) {
                    sock->_writeable = true;
                    if (!sock->_wsuspended) {
                        dispatch_suspend(sock->_shared->_wrsrc);
                        sock->_wsuspended = true;
                    }
                    // If the source exists, but is now invalid, this next stuff is relatively harmless.
                    if (sock->_shared->_source) {
                        CFRunLoopSourceSignal(sock->_shared->_source);
                        _CFRunLoopSourceWakeUpRunLoops(sock->_shared->_source);
                    }
                } else if (sock->_wsuspended && sock->_shared->_wrsrc) {
                    sock->_wsuspended = false;
                    dispatch_resume(sock->_shared->_wrsrc);
                }
                sock->_connectDisabled = false;
            }
        });
// CFLog(5, CFSTR("CFSocketEnableCallBacks(%p, 0x%x) done"), sock, callBackTypes);
}

void CFSocketDisableCallBacks(CFSocketRef sock, CFOptionFlags callBackTypes) {
    CHECK_FOR_FORK_RET();
    __CFGenericValidateType(sock, CFSocketGetTypeID());
// CFLog(5, CFSTR("CFSocketDisableCallBacks(%p, 0x%x) starting"), sock, callBackTypes);
    dispatch_sync(__sockQueue(), ^{
            if (!__CFSocketIsValid(sock)) return;
            if (sock->_wantReadType && (callBackTypes & 0x3) == sock->_wantReadType) {
                if (!sock->_rsuspended && sock->_shared->_rdsrc) {
                    dispatch_suspend(sock->_shared->_rdsrc);
                    sock->_rsuspended = true;
                }
                sock->_readDisabled = true;
            }
            if (sock->_wantWrite && (callBackTypes & kCFSocketWriteCallBack)) {
                if (!sock->_wsuspended && sock->_shared->_wrsrc) {
                    dispatch_suspend(sock->_shared->_wrsrc);
                    sock->_wsuspended = true;
                }
                sock->_writeDisabled = true;
            }
            if (sock->_wantConnect && !sock->_connected && (callBackTypes & kCFSocketConnectCallBack)) {
                if (!sock->_wsuspended && sock->_shared->_wrsrc) {
                    dispatch_suspend(sock->_shared->_wrsrc);
                    sock->_wsuspended = true;
                }
                sock->_connectDisabled = true;
            }
        });
// CFLog(5, CFSTR("CFSocketDisableCallBacks(%p, 0x%x) done"), sock, callBackTypes);
}

void CFSocketInvalidate(CFSocketRef sock) {
    CHECK_FOR_FORK_RET();
    __CFGenericValidateType(sock, CFSocketGetTypeID());
    CFRetain(sock);
// CFLog(5, CFSTR("CFSocketInvalidate(%p) starting"), sock);
    __block CFRunLoopSourceRef source = NULL;
    __block Boolean wasReady = false;
    dispatch_sync(__sockQueue(), ^{
            wasReady = (sock->_state == kCFSocketStateReady);
            if (wasReady) {
                sock->_state = kCFSocketStateInvalidating;
                OSMemoryBarrier();
                for (CFIndex idx = 0, cnt = CFArrayGetCount(__CFAllSockets); idx < cnt; idx++) {
                    CFSocketRef s = (CFSocketRef)CFArrayGetValueAtIndex(__CFAllSockets, idx);
                    if (s == sock) {
                        CFArrayRemoveValueAtIndex(__CFAllSockets, idx);
                        break;
                    }
                }
                if (sock->_shared->_rdsrc) {
                    dispatch_source_cancel(sock->_shared->_rdsrc);
                    if (sock->_rsuspended) {
                        sock->_rsuspended = false;
                        dispatch_resume(sock->_shared->_rdsrc);
                    }
                }
                if (sock->_shared->_wrsrc) {
                    dispatch_source_cancel(sock->_shared->_wrsrc);
                    if (sock->_wsuspended) {
                        sock->_wsuspended = false;
                        dispatch_resume(sock->_shared->_wrsrc);
                    }
                }
                source = sock->_shared->_source;
                sock->_shared->_source = NULL;
                sock->_shared->_refCnt--;
                if (0 == sock->_shared->_refCnt) {
                    if (sock->_shared->_closeFD) {
                        // thoroughly stop anything else from using the fd
                        (void)shutdown(sock->_shared->_socket, SHUT_RDWR);
                        int nullfd = open("/dev/null", O_RDONLY);
                        dup2(nullfd, sock->_shared->_socket);
                        close(nullfd);
                        close(sock->_shared->_socket);
                    }
                    free(sock->_shared);
                }
                sock->_shared = NULL;
            }
        });
    if (wasReady) {
        if (NULL != source) {
            CFRunLoopSourceInvalidate(source);
            CFRelease(source);
        }
        void *info = sock->_context.info;
        sock->_context.info = NULL;
        if (sock->_context.release) {
            sock->_context.release(info);
        }
        sock->_state = kCFSocketStateInvalid;
        OSMemoryBarrier();
    }
// CFLog(5, CFSTR("CFSocketInvalidate(%p) done%s"), sock, wasReady ? " -- done on this thread" : "");
    CFRelease(sock);
}

Boolean CFSocketIsValid(CFSocketRef sock) {
    __CFGenericValidateType(sock, CFSocketGetTypeID());
    if (!__CFSocketIsValid(sock)) return false;
    struct stat statbuf;
    int ret = sock->_shared ? fstat(sock->_shared->_socket, &statbuf) : -1;
    if (ret < 0) {
	CFSocketInvalidate(sock);
	return false;
    }
    return true;
}


static void __CFSocketPerform(void *info) { // CFRunLoop should only call this on one thread at a time
    CHECK_FOR_FORK_RET();
    CFSocketRef sock = (CFSocketRef)info;

// CFLog(5, CFSTR("__CFSocketPerform(%p) starting '%@'"), sock, sock);
    __block Boolean doRead = false, doWrite = false, doConnect = false, isValid = false;
    __block int fd = INVALID_SOCKET;
    __block SInt32 errorCode = 0;
    __block int new_fd = INVALID_SOCKET;
    __block CFDataRef address = NULL;
    __block CFMutableDataRef data = NULL;
    __block void *context_info = NULL;
    __block void (*context_release)(const void *) = NULL;
    dispatch_sync(__sockQueue(), ^{
            isValid = __CFSocketIsValid(sock);
            if (!isValid) return;
            fd = sock->_shared->_socket;
            doRead = sock->_readable && sock->_wantReadType && !sock->_readDisabled;
            if (doRead) {
                sock->_readable = false;
                doRead = sockfd_is_readable(fd);
// if (!doRead) CFLog(5, CFSTR("__CFSocketPerform(%p) socket is not actually readable"), sock);
            }
            doWrite = sock->_writeable && sock->_wantWrite && !sock->_writeDisabled;
            doConnect = sock->_writeable && sock->_wantConnect && !sock->_connectDisabled && !sock->_connected;
            if (doWrite || doConnect) {
                sock->_writeable = false;
                if (doWrite) doWrite = sockfd_is_writeable(fd);
                if (doConnect) doConnect = sockfd_is_writeable(fd);
            }
            if (!sock->_leaveErrors && (doWrite || doConnect)) { // not on read, for whatever reason
                int errorSize = sizeof(errorCode);
                int ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, (void *)&errorCode, (socklen_t *)&errorSize);
                if (0 != ret) errorCode = 0;
                sock->_error = errorCode;
            }
            sock->_connected = true;
// CFLog(5, CFSTR("__CFSocketPerform(%p) doing %d %d %d"), sock, doRead, doWrite, doConnect);
            if (doRead) {
                switch (sock->_wantReadType) {
                case kCFSocketReadCallBack:
                    break;
                case kCFSocketAcceptCallBack: {
                    uint8_t name[MAX_SOCKADDR_LEN];
                    socklen_t namelen = sizeof(name);
                    new_fd = accept(fd, (struct sockaddr *)name, (socklen_t *)&namelen);
                    if (INVALID_SOCKET != new_fd) {
                        address = CFDataCreate(CFGetAllocator(sock), name, namelen);
                    }
                    break;
                }
                case kCFSocketDataCallBack: {
                    uint8_t name[MAX_SOCKADDR_LEN];
                    socklen_t namelen = sizeof(name);
                    int avail = 0;
                    int ret = ioctlsocket(fd, FIONREAD, &avail);
                    if (ret < 0 || avail < 256) avail = 256;
                    if ((1 << 20) < avail) avail = (1 << 20);
                    data = CFDataCreateMutable(CFGetAllocator(sock), 0);
                    CFDataSetLength(data, avail);
                    ssize_t len = recvfrom(fd, CFDataGetMutableBytePtr(data), avail, 0, (struct sockaddr *)name, (socklen_t *)&namelen);
                    CFIndex datalen = (len < 0) ? 0 : len;
                    CFDataSetLength(data, datalen);
                    if (0 < namelen) {
                        address = CFDataCreate(CFGetAllocator(sock), name, namelen);
                    } else if (sock->_connOriented) {
                        // cannot call CFSocketCopyPeerAddress(), or deadlock
                        if (!sock->_peerAddress) {
                            uint8_t name[MAX_SOCKADDR_LEN];
                            socklen_t namelen = sizeof(name);
                            int ret = getpeername(sock->_shared->_socket, (struct sockaddr *)name, (socklen_t *)&namelen);
                            if (0 == ret && 0 < namelen) {
                                sock->_peerAddress = CFDataCreate(CFGetAllocator(sock), name, namelen);
                            }
                        }
                        address = sock->_peerAddress ? (CFDataRef)CFRetain(sock->_peerAddress) : NULL;
                    }
                    if (NULL == address) {
                        address = CFDataCreate(CFGetAllocator(sock), NULL, 0);
                    }
                    break;
                }
                }
            }
            if (sock->_reenableRead) {
// CFLog(5, CFSTR("__CFSocketPerform(%p) reenabling read %d %p"), sock, sock->_rsuspended, sock->_shared->_rdsrc);
                if (sock->_rsuspended && sock->_shared->_rdsrc) {
                    sock->_rsuspended = false;
                    dispatch_resume(sock->_shared->_rdsrc);
                }
            }
            if (sock->_reenableWrite) {
                if (sock->_wsuspended && sock->_shared->_wrsrc) {
                    sock->_wsuspended = false;
                    dispatch_resume(sock->_shared->_wrsrc);
                }
            }
            if (sock->_context.retain && (doConnect || doRead || doWrite)) {
                context_info = (void *)sock->_context.retain(sock->_context.info);
                context_release = sock->_context.release;
            } else {
                context_info = sock->_context.info;
            }
        });
// CFLog(5, CFSTR("__CFSocketPerform(%p) isValid:%d, doRead:%d, doWrite:%d, doConnect:%d error:%d"), sock, isValid, doRead, doWrite, doConnect, errorCode);
    if (!isValid || !(doConnect || doRead || doWrite)) return;

    Boolean calledOut = false;
    if (doConnect) {
        if (sock->_callout) sock->_callout(sock, kCFSocketConnectCallBack, NULL, (0 != errorCode) ? &errorCode : NULL, context_info);
        calledOut = true;
    }
    if (doRead && (!calledOut || __CFSocketIsValid(sock))) {
        switch (sock->_wantReadType) {
        case kCFSocketReadCallBack:
            if (sock->_callout) sock->_callout(sock, kCFSocketReadCallBack, NULL, NULL, context_info);
            calledOut = true;
            break;
        case kCFSocketAcceptCallBack:
            if (INVALID_SOCKET != new_fd) {
                if (sock->_callout) sock->_callout(sock, kCFSocketAcceptCallBack, address, &new_fd, context_info);
                calledOut = true;
            }
            break;
        case kCFSocketDataCallBack:
            if (sock->_callout) sock->_callout(sock, kCFSocketDataCallBack, address, data, context_info);
            calledOut = true;
            break;
        }
    }
    if (doWrite && (!calledOut || __CFSocketIsValid(sock))) {
        if (0 == errorCode) {
            if (sock->_callout) sock->_callout(sock, kCFSocketWriteCallBack, NULL, NULL, context_info);
            calledOut = true;
        }
    }

    if (data && 0 == CFDataGetLength(data)) CFSocketInvalidate(sock);
    if (address) CFRelease(address);
    if (data) CFRelease(data);
    if (context_release) {
        context_release(context_info);
    }

    CHECK_FOR_FORK_RET();
// CFLog(5, CFSTR("__CFSocketPerform(%p) done"), sock);
}

static void __CFSocketSchedule(void *info, CFRunLoopRef rl, CFStringRef mode) {
    CFSocketRef sock = (CFSocketRef)info;
    int32_t newVal = OSAtomicIncrement32Barrier(&sock->_runLoopCounter);
    if (1 == newVal) { // on a transition from 0->1, the old code forced all desired callbacks enabled
        CFOptionFlags types = sock->_wantReadType | (sock->_wantWrite ? kCFSocketWriteCallBack : 0) | (sock->_wantConnect ? kCFSocketConnectCallBack : 0);
        CFSocketEnableCallBacks(sock, types);
    }
    CFRunLoopWakeUp(rl);
}

static void __CFSocketCancel(void *info, CFRunLoopRef rl, CFStringRef mode) {
    CFSocketRef sock = (CFSocketRef)info;
    OSAtomicDecrement32Barrier(&sock->_runLoopCounter);
    CFRunLoopWakeUp(rl);
}

CFRunLoopSourceRef CFSocketCreateRunLoopSource(CFAllocatorRef allocator, CFSocketRef sock, CFIndex order) {
    CHECK_FOR_FORK_RET(NULL);
    __CFGenericValidateType(sock, CFSocketGetTypeID());
    if (!CFSocketIsValid(sock)) return NULL;
    __block CFRunLoopSourceRef result = NULL;
    dispatch_sync(__sockQueue(), ^{
            if (!__CFSocketIsValid(sock)) return;
            if (NULL != sock->_shared->_source && !CFRunLoopSourceIsValid(sock->_shared->_source)) {
                CFRelease(sock->_shared->_source);
                sock->_shared->_source = NULL;
            }
            if (NULL == sock->_shared->_source) {
                CFRunLoopSourceContext context;
                context.version = 0;
                context.info = (void *)sock;
                context.retain = (const void *(*)(const void *))CFRetain;
                context.release = (void (*)(const void *))CFRelease;
                context.copyDescription = (CFStringRef (*)(const void *))__CFSocketCopyDescription;
                context.equal = NULL;
                context.hash = NULL;
                context.schedule = __CFSocketSchedule;
                context.cancel = __CFSocketCancel;
                context.perform = __CFSocketPerform;
                sock->_shared->_source = CFRunLoopSourceCreate(allocator, order, (CFRunLoopSourceContext *)&context);
                if (sock->_shared->_source) {
                    if (sock->_wantReadType) {
                        if (sockfd_is_readable(sock->_shared->_socket)) {
                            sock->_readable = true;
                            if (!sock->_rsuspended) {
                                dispatch_suspend(sock->_shared->_rdsrc);
                                sock->_rsuspended = true;
                            }
                            if (sock->_shared->_source) {
                                CFRunLoopSourceSignal(sock->_shared->_source);
                                _CFRunLoopSourceWakeUpRunLoops(sock->_shared->_source);
                            }
                        } else if (sock->_rsuspended && sock->_shared->_rdsrc) {
                            sock->_rsuspended = false;
                            dispatch_resume(sock->_shared->_rdsrc);
                        }
                    }
                    if (sock->_wantWrite || (sock->_wantConnect && !sock->_connected)) {
                        if (sockfd_is_writeable(sock->_shared->_socket)) {
                            sock->_writeable = true;
                            if (!sock->_wsuspended) {
                                dispatch_suspend(sock->_shared->_wrsrc);
                                sock->_wsuspended = true;
                            }
                            if (sock->_shared->_source) {
                                CFRunLoopSourceSignal(sock->_shared->_source);
                                _CFRunLoopSourceWakeUpRunLoops(sock->_shared->_source);
                            }
                        } else if (sock->_wsuspended && sock->_shared->_wrsrc) {
                            sock->_wsuspended = false;
                            dispatch_resume(sock->_shared->_wrsrc);
                        }
                    }
                }
            }
            result = sock->_shared->_source ? (CFRunLoopSourceRef)CFRetain(sock->_shared->_source) : NULL;
        });
// CFLog(5, CFSTR("CFSocketCreateRunLoopSource(%p) => %p"), sock, result);
    return result;
}


void __CFSocketSetSocketReadBufferAttrs(CFSocketRef s, CFTimeInterval timeout, CFIndex length) {
}

CFIndex __CFSocketRead(CFSocketRef s, UInt8* buffer, CFIndex length, int* error) {
    *error = 0;
    int ret = read(CFSocketGetNative(s), buffer, length);
    if (ret < 0) {
        *error = errno;
    }
    return ret;
}

Boolean __CFSocketGetBytesAvailable(CFSocketRef s, CFIndex* ctBytesAvailable) {
    int bytesAvailable;
    int ret = ioctlsocket(CFSocketGetNative(s), FIONREAD, &bytesAvailable);
    if (ret < 0) return false;
    *ctBytesAvailable = (CFIndex)bytesAvailable;
    return true;
}

*/
#else /* not NEW_SOCKET */


#include <CoreFoundation/CFSocket.h>
#include <sys/types.h>
#include <math.h>
#include <limits.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
#include <sys/sysctl.h>
#include <sys/un.h>
#include <libc.h>
#include <dlfcn.h>
#endif
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFPropertyList.h>
#include "CFInternal.h"

#if DEPLOYMENT_TARGET_WINDOWS

#define EINPROGRESS WSAEINPROGRESS

// redefine this to the winsock error in this file
#undef EBADF
#define EBADF WSAENOTSOCK

#define NBBY 8
#define NFDBITS	(sizeof(int32_t) * NBBY)

typedef int32_t fd_mask;
typedef int socklen_t;

#define gettimeofday _NS_gettimeofday
CF_PRIVATE int _NS_gettimeofday(struct timeval *tv, struct timezone *tz);

// although this is only used for debug info, we define it for compatibility
#define	timersub(tvp, uvp, vvp) \
    do { \
        (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;		\
        (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;	\
        if ((vvp)->tv_usec < 0) {				\
            (vvp)->tv_sec--;				\
            (vvp)->tv_usec += 1000000;			\
        }							\
    } while (0)


#endif // DEPLOYMENT_TARGET_WINDOWS


// On Mach we use a v0 RunLoopSource to make client callbacks.  That source is signalled by a
// separate SocketManager thread who uses select() to watch the sockets' fds.

//#define LOG_CFSOCKET

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
#define INVALID_SOCKET (CFSocketNativeHandle)(-1)
#define closesocket(a) close((a))
#define ioctlsocket(a,b,c) ioctl((a),(b),(c))
#endif

CF_INLINE int __CFSocketLastError(void) {
#if DEPLOYMENT_TARGET_WINDOWS
    return WSAGetLastError();
#else
    return thread_errno();
#endif
}

CF_INLINE CFIndex __CFSocketFdGetSize(CFDataRef fdSet) {
    return NBBY * CFDataGetLength(fdSet);
}

CF_INLINE Boolean __CFSocketFdSet(CFSocketNativeHandle sock, CFMutableDataRef fdSet) {
    /* returns true if a change occurred, false otherwise */
    Boolean retval = false;
    if (INVALID_SOCKET != sock && 0 <= sock) {
        CFIndex numFds = NBBY * CFDataGetLength(fdSet);
        fd_mask *fds_bits;
        if (sock >= numFds) {
            CFIndex oldSize = numFds / NFDBITS, newSize = (sock + NFDBITS) / NFDBITS, changeInBytes = (newSize - oldSize) * sizeof(fd_mask);
            CFDataIncreaseLength(fdSet, changeInBytes);
            fds_bits = (fd_mask *)CFDataGetMutableBytePtr(fdSet);
            memset(fds_bits + oldSize, 0, changeInBytes);
        } else {
            fds_bits = (fd_mask *)CFDataGetMutableBytePtr(fdSet);
        }
        if (!FD_ISSET(sock, (fd_set *)fds_bits)) {
            retval = true;
            FD_SET(sock, (fd_set *)fds_bits);
        }
    }
    return retval;
}


#define MAX_SOCKADDR_LEN 256
#define MAX_DATA_SIZE 65535
#define MAX_CONNECTION_ORIENTED_DATA_SIZE 32768

/* locks are to be acquired in the following order:
   (1) __CFAllSocketsLock
   (2) an individual CFSocket's lock
   (3) __CFActiveSocketsLock
*/
static CFLock_t __CFAllSocketsLock = CFLockInit; /* controls __CFAllSockets */
static CFMutableDictionaryRef __CFAllSockets = NULL;
static CFLock_t __CFActiveSocketsLock = CFLockInit; /* controls __CFRead/WriteSockets, __CFRead/WriteSocketsFds, __CFSocketManagerThread, and __CFSocketManagerIteration */
static volatile UInt32 __CFSocketManagerIteration = 0;
static CFMutableArrayRef __CFWriteSockets = NULL;
static CFMutableArrayRef __CFReadSockets = NULL;
static CFMutableDataRef __CFWriteSocketsFds = NULL;
static CFMutableDataRef __CFReadSocketsFds = NULL;
static CFDataRef zeroLengthData = NULL;
static Boolean __CFReadSocketsTimeoutInvalid = true;  /* rebuild the timeout value before calling select */

static CFSocketNativeHandle __CFWakeupSocketPair[2] = {INVALID_SOCKET, INVALID_SOCKET};
static void *__CFSocketManagerThread = NULL;

static void __CFSocketDoCallback(CFSocketRef s, CFDataRef data, CFDataRef address, CFSocketNativeHandle sock);

struct __CFSocket {
    CFRuntimeBase _base;
    struct {
        unsigned client:8;	// flags set by client (reenable, CloseOnInvalidate)
        unsigned disabled:8;	// flags marking disabled callbacks
        unsigned connected:1;	// Are we connected yet?  (also true for connectionless sockets)
        unsigned writableHint:1;  // Did the polling the socket show it to be writable?
        unsigned closeSignaled:1;  // Have we seen FD_CLOSE? (only used on Win32)
        unsigned unused:13;
    } _f;
    CFLock_t _lock;
    CFLock_t _writeLock;
    CFSocketNativeHandle _socket;	/* immutable */
    SInt32 _socketType;
    SInt32 _errorCode;
    CFDataRef _address;
    CFDataRef _peerAddress;
    SInt32 _socketSetCount;
    CFRunLoopSourceRef _source0;	// v0 RLS, messaged from SocketMgr
    CFMutableArrayRef _runLoops;
    CFSocketCallBack _callout;		/* immutable */
    CFSocketContext _context;		/* immutable */
    CFMutableArrayRef _dataQueue;	// queues to pass data from SocketMgr thread
    CFMutableArrayRef _addressQueue;
	
	struct timeval _readBufferTimeout;
	CFMutableDataRef _readBuffer;
	CFIndex _bytesToBuffer;			/* is length of _readBuffer */
	CFIndex _bytesToBufferPos;		/* where the next _CFSocketRead starts from */
	CFIndex _bytesToBufferReadPos;	/* Where the buffer will next be read into (always after _bytesToBufferPos, but less than _bytesToBuffer) */
	Boolean _atEOF;
    int _bufferedReadError;
	
	CFMutableDataRef _leftoverBytes;

    // <rdar://problem/17849895>
    // If the timeout is set on the CFSocketRef but we never get select() timeout
    // because we always have some network events so select never times out (e.g. while having a large download).
    // We need to notify any waiting buffered read clients if there is data available without relying on select timing out.
    struct timeval _readBufferTimeoutNotificationTime;
    Boolean _hitTheTimeout;
};

/* Bit 6 in the base reserved bits is used for write-signalled state (mutable) */
/* Bit 5 in the base reserved bits is used for read-signalled state (mutable) */
/* Bit 4 in the base reserved bits is used for invalid state (mutable) */
/* Bits 0-3 in the base reserved bits are used for callback types (immutable) */
/* Of this, bits 0-1 are used for the read callback type. */

CF_INLINE Boolean __CFSocketIsWriteSignalled(CFSocketRef s) {
    return (Boolean)__CFBitfieldGetValue(((const CFRuntimeBase *)s)->_cfinfo[CF_INFO_BITS], 6, 6);
}

CF_INLINE void __CFSocketSetWriteSignalled(CFSocketRef s) {
    __CFBitfieldSetValue(((CFRuntimeBase *)s)->_cfinfo[CF_INFO_BITS], 6, 6, 1);
}

CF_INLINE void __CFSocketUnsetWriteSignalled(CFSocketRef s) {
    __CFBitfieldSetValue(((CFRuntimeBase *)s)->_cfinfo[CF_INFO_BITS], 6, 6, 0);
}

CF_INLINE Boolean __CFSocketIsReadSignalled(CFSocketRef s) {
    return (Boolean)__CFBitfieldGetValue(((const CFRuntimeBase *)s)->_cfinfo[CF_INFO_BITS], 5, 5);
}

CF_INLINE void __CFSocketSetReadSignalled(CFSocketRef s) {
    __CFBitfieldSetValue(((CFRuntimeBase *)s)->_cfinfo[CF_INFO_BITS], 5, 5, 1);
}

CF_INLINE void __CFSocketUnsetReadSignalled(CFSocketRef s) {
    __CFBitfieldSetValue(((CFRuntimeBase *)s)->_cfinfo[CF_INFO_BITS], 5, 5, 0);
}

CF_INLINE Boolean __CFSocketIsValid(CFSocketRef s) {
    return (Boolean)__CFBitfieldGetValue(((const CFRuntimeBase *)s)->_cfinfo[CF_INFO_BITS], 4, 4);
}

CF_INLINE void __CFSocketSetValid(CFSocketRef s) {
    __CFBitfieldSetValue(((CFRuntimeBase *)s)->_cfinfo[CF_INFO_BITS], 4, 4, 1);
}

CF_INLINE void __CFSocketUnsetValid(CFSocketRef s) {
    __CFBitfieldSetValue(((CFRuntimeBase *)s)->_cfinfo[CF_INFO_BITS], 4, 4, 0);
}

CF_INLINE uint8_t __CFSocketCallBackTypes(CFSocketRef s) {
    return (uint8_t)__CFBitfieldGetValue(((const CFRuntimeBase *)s)->_cfinfo[CF_INFO_BITS], 3, 0);
}

CF_INLINE uint8_t __CFSocketReadCallBackType(CFSocketRef s) {
    return (uint8_t)__CFBitfieldGetValue(((const CFRuntimeBase *)s)->_cfinfo[CF_INFO_BITS], 1, 0);
}

CF_INLINE void __CFSocketSetCallBackTypes(CFSocketRef s, uint8_t types) {
    __CFBitfieldSetValue(((CFRuntimeBase *)s)->_cfinfo[CF_INFO_BITS], 3, 0, types & 0xF);
}

CF_INLINE void __CFSocketLock(CFSocketRef s) {
    __CFLock(&(s->_lock));
}

CF_INLINE void __CFSocketUnlock(CFSocketRef s) {
    __CFUnlock(&(s->_lock));
}

CF_INLINE Boolean __CFSocketIsConnectionOriented(CFSocketRef s) {
    return (SOCK_STREAM == s->_socketType || SOCK_SEQPACKET == s->_socketType);
}

CF_INLINE Boolean __CFSocketIsScheduled(CFSocketRef s) {
    return (s->_socketSetCount > 0);
}

CF_INLINE void __CFSocketEstablishAddress(CFSocketRef s) {
    /* socket should already be locked */
    uint8_t name[MAX_SOCKADDR_LEN];
    int namelen = sizeof(name);
    if (__CFSocketIsValid(s) && NULL == s->_address && INVALID_SOCKET != s->_socket && 0 == getsockname(s->_socket, (struct sockaddr *)name, (socklen_t *)&namelen) && NULL != name && 0 < namelen) {
        s->_address = CFDataCreate(CFGetAllocator(s), name, namelen);
    }
}

CF_INLINE void __CFSocketEstablishPeerAddress(CFSocketRef s) {
    /* socket should already be locked */
    uint8_t name[MAX_SOCKADDR_LEN];
    int namelen = sizeof(name);
    if (__CFSocketIsValid(s) && NULL == s->_peerAddress && INVALID_SOCKET != s->_socket && 0 == getpeername(s->_socket, (struct sockaddr *)name, (socklen_t *)&namelen) && NULL != name && 0 < namelen) {
        s->_peerAddress = CFDataCreate(CFGetAllocator(s), name, namelen);
    }
}

static Boolean __CFNativeSocketIsValid(CFSocketNativeHandle sock) {
#if DEPLOYMENT_TARGET_WINDOWS
    SInt32 errorCode = 0;
    int errorSize = sizeof(errorCode);
    return !(0 != getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&errorCode, &errorSize) && __CFSocketLastError() == WSAENOTSOCK);
#else    
    SInt32 flags = fcntl(sock, F_GETFL, 0);
    return !(0 > flags && EBADF == __CFSocketLastError());
#endif
}

CF_INLINE Boolean __CFSocketFdClr(CFSocketNativeHandle sock, CFMutableDataRef fdSet) {
    /* returns true if a change occurred, false otherwise */
    Boolean retval = false;
    if (INVALID_SOCKET != sock && 0 <= sock) {
        CFIndex numFds = NBBY * CFDataGetLength(fdSet);
        fd_mask *fds_bits;
        if (sock < numFds) {
            fds_bits = (fd_mask *)CFDataGetMutableBytePtr(fdSet);
            if (FD_ISSET(sock, (fd_set *)fds_bits)) {
                retval = true;
                FD_CLR(sock, (fd_set *)fds_bits);
            }
        }
    }
    return retval;
}

static SInt32 __CFSocketCreateWakeupSocketPair(void) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    SInt32 error;

    error = socketpair(PF_LOCAL, SOCK_DGRAM, 0, __CFWakeupSocketPair);
    if (0 <= error) error = fcntl(__CFWakeupSocketPair[0], F_SETFD, FD_CLOEXEC);
    if (0 <= error) error = fcntl(__CFWakeupSocketPair[1], F_SETFD, FD_CLOEXEC);
    if (0 > error) {
        closesocket(__CFWakeupSocketPair[0]);
        closesocket(__CFWakeupSocketPair[1]);
        __CFWakeupSocketPair[0] = INVALID_SOCKET;
        __CFWakeupSocketPair[1] = INVALID_SOCKET;
    }
#else
    UInt32 i;
    SInt32 error = 0;
    struct sockaddr_in address[2];
    int namelen = sizeof(struct sockaddr_in);
    for (i = 0; i < 2; i++) {
        __CFWakeupSocketPair[i] = socket(PF_INET, SOCK_DGRAM, 0);
        memset(&(address[i]), 0, sizeof(struct sockaddr_in));
        address[i].sin_family = AF_INET;
        address[i].sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (0 <= error) error = bind(__CFWakeupSocketPair[i], (struct sockaddr *)&(address[i]), sizeof(struct sockaddr_in));
        if (0 <= error) error = getsockname(__CFWakeupSocketPair[i], (struct sockaddr *)&(address[i]), &namelen);
        if (sizeof(struct sockaddr_in) != namelen) error = -1;
    }
    if (0 <= error) error = connect(__CFWakeupSocketPair[0], (struct sockaddr *)&(address[1]), sizeof(struct sockaddr_in));
    if (0 <= error) error = connect(__CFWakeupSocketPair[1], (struct sockaddr *)&(address[0]), sizeof(struct sockaddr_in));
    if (0 > error) {
        closesocket(__CFWakeupSocketPair[0]);
        closesocket(__CFWakeupSocketPair[1]);
        __CFWakeupSocketPair[0] = INVALID_SOCKET;
        __CFWakeupSocketPair[1] = INVALID_SOCKET;
    }
#endif
#if defined(LOG_CFSOCKET)
    fprintf(stdout, "wakeup socket pair is %d / %d\n", __CFWakeupSocketPair[0], __CFWakeupSocketPair[1]);
#endif
    return error;
}


// Version 0 RunLoopSources set a mask in an FD set to control what socket activity we hear about.
// Changes to the master fs_sets occur via these 4 functions.
CF_INLINE Boolean __CFSocketSetFDForRead(CFSocketRef s) {
    __CFReadSocketsTimeoutInvalid = true;
    Boolean b = __CFSocketFdSet(s->_socket, __CFReadSocketsFds);
    if (b && INVALID_SOCKET != __CFWakeupSocketPair[0]) {
        uint8_t c = 'r';
        send(__CFWakeupSocketPair[0], (const char *)&c, sizeof(c), 0);
    }
    return b;
}

CF_INLINE Boolean __CFSocketClearFDForRead(CFSocketRef s) {
    __CFReadSocketsTimeoutInvalid = true;
    Boolean b = __CFSocketFdClr(s->_socket, __CFReadSocketsFds);
    if (b && INVALID_SOCKET != __CFWakeupSocketPair[0]) {
        uint8_t c = 's';
        send(__CFWakeupSocketPair[0], (const char *)&c, sizeof(c), 0);
    }
    return b;
}

CF_INLINE Boolean __CFSocketSetFDForWrite(CFSocketRef s) {
// CFLog(5, CFSTR("__CFSocketSetFDForWrite(%p)"), s);
    Boolean b = __CFSocketFdSet(s->_socket, __CFWriteSocketsFds);
    if (b && INVALID_SOCKET != __CFWakeupSocketPair[0]) {
        uint8_t c = 'w';
        send(__CFWakeupSocketPair[0], (const char *)&c, sizeof(c), 0);
    }
    return b;
}

CF_INLINE Boolean __CFSocketClearFDForWrite(CFSocketRef s) {
// CFLog(5, CFSTR("__CFSocketClearFDForWrite(%p)"), s);
    Boolean b = __CFSocketFdClr(s->_socket, __CFWriteSocketsFds);
    if (b && INVALID_SOCKET != __CFWakeupSocketPair[0]) {
        uint8_t c = 'x';
        send(__CFWakeupSocketPair[0], (const char *)&c, sizeof(c), 0);
    }
    return b;
}

#if DEPLOYMENT_TARGET_WINDOWS
static Boolean WinSockUsed = FALSE;

static void __CFSocketInitializeWinSock_Guts(void) {
    if (!WinSockUsed) {
        WinSockUsed = TRUE;
        WORD versionRequested = MAKEWORD(2, 2);
        WSADATA wsaData;
        int errorStatus = WSAStartup(versionRequested, &wsaData);
        if (errorStatus != 0 || LOBYTE(wsaData.wVersion) != LOBYTE(versionRequested) || HIBYTE(wsaData.wVersion) != HIBYTE(versionRequested)) {
            WSACleanup();
            CFLog(kCFLogLevelWarning, CFSTR("*** Could not initialize WinSock subsystem!!!"));
        }
    }
}

CF_EXPORT void __CFSocketInitializeWinSock(void) {
    __CFLock(&__CFActiveSocketsLock);
    __CFSocketInitializeWinSock_Guts();
    __CFUnlock(&__CFActiveSocketsLock);
}

CF_PRIVATE void __CFSocketCleanup(void) {
    if (INVALID_SOCKET != __CFWakeupSocketPair[0]) {
        closesocket(__CFWakeupSocketPair[0]);
        __CFWakeupSocketPair[0] = INVALID_SOCKET;
    }
    if (INVALID_SOCKET != __CFWakeupSocketPair[1]) {
        closesocket(__CFWakeupSocketPair[1]);
        __CFWakeupSocketPair[1] = INVALID_SOCKET;
    }
    if (WinSockUsed) {
        // technically this is not supposed to be called here since it will be called from dllmain, but I don't know where else to put it
        WSACleanup();
    }
}

#endif

// CFNetwork needs to call this, especially for Win32 to get WSAStartup
static void __CFSocketInitializeSockets(void) {
    __CFWriteSockets = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, NULL);
    __CFReadSockets = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, NULL);
    __CFWriteSocketsFds = CFDataCreateMutable(kCFAllocatorSystemDefault, 0);
    __CFReadSocketsFds = CFDataCreateMutable(kCFAllocatorSystemDefault, 0);
    zeroLengthData = CFDataCreateMutable(kCFAllocatorSystemDefault, 0);
#if DEPLOYMENT_TARGET_WINDOWS
    __CFSocketInitializeWinSock_Guts();
#endif
    if (0 > __CFSocketCreateWakeupSocketPair()) {
        CFLog(kCFLogLevelWarning, CFSTR("*** Could not create wakeup socket pair for CFSocket!!!"));
    } else {
        UInt32 yes = 1;
        /* wakeup sockets must be non-blocking */
        ioctlsocket(__CFWakeupSocketPair[0], FIONBIO, (u_long *)&yes);
        ioctlsocket(__CFWakeupSocketPair[1], FIONBIO, (u_long *)&yes);
        __CFSocketFdSet(__CFWakeupSocketPair[1], __CFReadSocketsFds);
    }
}

static CFRunLoopRef __CFSocketCopyRunLoopToWakeUp(CFRunLoopSourceRef src, CFMutableArrayRef runLoops) {
    if (!src) return NULL;
    CFRunLoopRef rl = NULL;
    SInt32 idx, cnt = CFArrayGetCount(runLoops);
    if (0 < cnt) {
        rl = (CFRunLoopRef)CFArrayGetValueAtIndex(runLoops, 0);
        for (idx = 1; NULL != rl && idx < cnt; idx++) {
            CFRunLoopRef value = (CFRunLoopRef)CFArrayGetValueAtIndex(runLoops, idx);
            if (value != rl) rl = NULL;
        }
        if (NULL == rl) {	/* more than one different rl, so we must pick one */
            /* ideally, this would be a run loop which isn't also in a
            * signaled state for this or another source, but that's tricky;
            * we pick one that is running in an appropriate mode for this
            * source, and from those if possible one that is waiting; then
            * we move this run loop to the end of the list to scramble them
            * a bit, and always search from the front */
            Boolean foundIt = false, foundBackup = false;
            SInt32 foundIdx = 0;
            for (idx = 0; !foundIt && idx < cnt; idx++) {
                CFRunLoopRef value = (CFRunLoopRef)CFArrayGetValueAtIndex(runLoops, idx);
                CFStringRef currentMode = CFRunLoopCopyCurrentMode(value);
                if (NULL != currentMode) {
                    if (CFRunLoopContainsSource(value, src, currentMode)) {
                        if (CFRunLoopIsWaiting(value)) {
                            foundIdx = idx;
                            foundIt = true;
                        } else if (!foundBackup) {
                            foundIdx = idx;
                            foundBackup = true;
                        }
                    }
                    CFRelease(currentMode);
                }
            }
            rl = (CFRunLoopRef)CFArrayGetValueAtIndex(runLoops, foundIdx);
            CFRetain(rl);
            CFArrayRemoveValueAtIndex(runLoops, foundIdx);
            CFArrayAppendValue(runLoops, rl);
        } else {
            CFRetain(rl);
        }
    }
    return rl;
}

// If callBackNow, we immediately do client callbacks, else we have to signal a v0 RunLoopSource so the
// callbacks can happen in another thread.
static void __CFSocketHandleWrite(CFSocketRef s, Boolean callBackNow) {
    SInt32 errorCode = 0;
    int errorSize = sizeof(errorCode);
    CFOptionFlags writeCallBacksAvailable;
    
    if (!CFSocketIsValid(s)) return;
    if (0 != (s->_f.client & kCFSocketLeaveErrors) || 0 != getsockopt(s->_socket, SOL_SOCKET, SO_ERROR, (char *)&errorCode, (socklen_t *)&errorSize)) errorCode = 0;
    // cast for WinSock bad API
#if defined(LOG_CFSOCKET)
    if (errorCode) fprintf(stdout, "error %ld on socket %d\n", (long)errorCode, s->_socket);
#endif
    __CFSocketLock(s);
    writeCallBacksAvailable = __CFSocketCallBackTypes(s) & (kCFSocketWriteCallBack | kCFSocketConnectCallBack);
    if ((s->_f.client & kCFSocketConnectCallBack) != 0) writeCallBacksAvailable &= ~kCFSocketConnectCallBack;
    if (!__CFSocketIsValid(s) || ((s->_f.disabled & writeCallBacksAvailable) == writeCallBacksAvailable)) {
        __CFSocketUnlock(s);
        return;
    }
    s->_errorCode = errorCode;
    __CFSocketSetWriteSignalled(s);
// CFLog(5, CFSTR("__CFSocketHandleWrite() signalling write on socket %p"), s);
#if defined(LOG_CFSOCKET)
    fprintf(stdout, "write signaling source for socket %d\n", s->_socket);
#endif
    if (callBackNow) {
        __CFSocketDoCallback(s, NULL, NULL, 0);
    } else {
        CFRunLoopSourceSignal(s->_source0);
        CFMutableArrayRef runLoopsOrig = (CFMutableArrayRef)CFRetain(s->_runLoops);
        CFMutableArrayRef runLoopsCopy = CFArrayCreateMutableCopy(kCFAllocatorSystemDefault, 0, s->_runLoops);
        CFRunLoopSourceRef source0 = s->_source0;
        if (NULL != source0 && !CFRunLoopSourceIsValid(source0)) {
            source0 = NULL;
        }
        if (source0) CFRetain(source0);
        __CFSocketUnlock(s);
        CFRunLoopRef rl = __CFSocketCopyRunLoopToWakeUp(source0, runLoopsCopy);
        if (source0) CFRelease(source0);
        if (NULL != rl) {
            CFRunLoopWakeUp(rl);
            CFRelease(rl);
        }
        __CFSocketLock(s);
        if (runLoopsOrig == s->_runLoops) {
            s->_runLoops = runLoopsCopy;
            runLoopsCopy = NULL;
            CFRelease(runLoopsOrig);
        }
        __CFSocketUnlock(s);
        CFRelease(runLoopsOrig);
        if (runLoopsCopy) CFRelease(runLoopsCopy);
    }
}


#if defined(LOG_CFSOCKET)

static CFStringRef someAddrToString(CFAllocatorRef alloc, int (*fun) (int, struct sockaddr*, socklen_t*), const char* name, CFSocketNativeHandle s)
{
    CFStringRef resultString = NULL;
    union {
        struct sockaddr		sa;
        struct sockaddr_in  sa4b;
        struct sockaddr_in6 sa6b;
        UInt8			static_buffer[SOCK_MAXADDRLEN];
    } u;
    socklen_t addrlen = sizeof(u.static_buffer);
    
    uint16_t* pPort = NULL;
    char buffer[1024];
    
    if ((*fun) (s, &u.sa, &addrlen) != 0)
        snprintf(buffer, sizeof(buffer), "error %d resolving %s address for socket %d", errno, name, s);
    else {
        void* pAddr = NULL;
        
        switch (u.sa.sa_family) {
            case AF_INET:
                pAddr = &u.sa4b.sin_addr;
                pPort = &u.sa4b.sin_port;
                break;
            case AF_INET6:
                pAddr = &u.sa6b.sin6_addr;
                pPort = &u.sa6b.sin6_port;
                break;
        }
        
        if (pAddr == NULL || inet_ntop(u.sa.sa_family, pAddr, buffer, sizeof(buffer)) == NULL)
            snprintf(buffer, sizeof(buffer), "[error %d converting %s address for socket %d]", pAddr != NULL? errno : EBADF, name, s);
    }
    if (pPort) {
        resultString = CFStringCreateWithFormat(alloc, NULL, CFSTR("%s:%d"), buffer, htons(*pPort));
    } else {
        resultString = CFStringCreateWithFormat(alloc, NULL, CFSTR("%s"), buffer);
    }
    return resultString;
}

CFStringRef copyPeerAddress(CFAllocatorRef alloc, CFSocketNativeHandle s)
{
    return someAddrToString(alloc, getpeername, "peer", s);
}

CFStringRef copyLocalAddress(CFAllocatorRef alloc, CFSocketNativeHandle s)
{
    return someAddrToString(alloc, getsockname, "local", s);
}

#endif

static void __CFSocketHandleRead(CFSocketRef s, Boolean causedByTimeout)
{
    CFDataRef data = NULL, address = NULL;
    CFSocketNativeHandle sock = INVALID_SOCKET;
    if (!CFSocketIsValid(s)) return;
    if (__CFSocketReadCallBackType(s) == kCFSocketDataCallBack) {
        uint8_t bufferArray[MAX_CONNECTION_ORIENTED_DATA_SIZE], *buffer;
        uint8_t name[MAX_SOCKADDR_LEN];
        int namelen = sizeof(name);
        SInt32 recvlen = 0;
        if (__CFSocketIsConnectionOriented(s)) {
            buffer = bufferArray;
            recvlen = recvfrom(s->_socket, (char *)buffer, MAX_CONNECTION_ORIENTED_DATA_SIZE, 0, (struct sockaddr *)name, (socklen_t *)&namelen);
        } else {
            buffer = (uint8_t *)malloc(MAX_DATA_SIZE);
            if (buffer) recvlen = recvfrom(s->_socket, (char *)buffer, MAX_DATA_SIZE, 0, (struct sockaddr *)name, (socklen_t *)&namelen);
        }
#if defined(LOG_CFSOCKET)
        fprintf(stdout, "read %ld bytes on socket %d\n", (long)recvlen, s->_socket);
#endif
        if (0 >= recvlen) {
            //??? should return error if <0
            /* zero-length data is the signal for perform to invalidate */
            data = (CFDataRef)CFRetain(zeroLengthData);
        } else {
            data = CFDataCreate(CFGetAllocator(s), buffer, recvlen);
        }
        if (buffer && buffer != bufferArray) free(buffer);
        __CFSocketLock(s);
        if (!__CFSocketIsValid(s)) {
            CFRelease(data);
            __CFSocketUnlock(s);
            return;
        }
        __CFSocketSetReadSignalled(s);
        if (NULL != name && 0 < namelen) {
            //??? possible optimizations:  uniquing; storing last value
            address = CFDataCreate(CFGetAllocator(s), name, namelen);
        } else if (__CFSocketIsConnectionOriented(s)) {
            if (NULL == s->_peerAddress) __CFSocketEstablishPeerAddress(s);
            if (NULL != s->_peerAddress) address = (CFDataRef)CFRetain(s->_peerAddress);
        }
        if (NULL == address) {
            address = (CFDataRef)CFRetain(zeroLengthData);
        }
        if (NULL == s->_dataQueue) {
            s->_dataQueue = CFArrayCreateMutable(CFGetAllocator(s), 0, &kCFTypeArrayCallBacks);
        }
        if (NULL == s->_addressQueue) {
            s->_addressQueue = CFArrayCreateMutable(CFGetAllocator(s), 0, &kCFTypeArrayCallBacks);
        }
        CFArrayAppendValue(s->_dataQueue, data);
        CFRelease(data);
        CFArrayAppendValue(s->_addressQueue, address);
        CFRelease(address);
        if (0 < recvlen
            && (s->_f.client & kCFSocketDataCallBack) != 0 && (s->_f.disabled & kCFSocketDataCallBack) == 0
            && __CFSocketIsScheduled(s)
        ) {
            __CFLock(&__CFActiveSocketsLock);
            /* restore socket to fds */
            __CFSocketSetFDForRead(s);
            __CFUnlock(&__CFActiveSocketsLock);
        }
    } else if (__CFSocketReadCallBackType(s) == kCFSocketAcceptCallBack) {
        uint8_t name[MAX_SOCKADDR_LEN];
        int namelen = sizeof(name);
        sock = accept(s->_socket, (struct sockaddr *)name, (socklen_t *)&namelen);
        if (INVALID_SOCKET == sock) {
            //??? should return error
            return;
        }
        if (NULL != name && 0 < namelen) {
            address = CFDataCreate(CFGetAllocator(s), name, namelen);
        } else {
            address = (CFDataRef)CFRetain(zeroLengthData);
        }
        __CFSocketLock(s);
        if (!__CFSocketIsValid(s)) {
            closesocket(sock);
            CFRelease(address);
            __CFSocketUnlock(s);
            return;
        }
        __CFSocketSetReadSignalled(s);
        if (NULL == s->_dataQueue) {
            s->_dataQueue = CFArrayCreateMutable(CFGetAllocator(s), 0, NULL);
        }
        if (NULL == s->_addressQueue) {
            s->_addressQueue = CFArrayCreateMutable(CFGetAllocator(s), 0, &kCFTypeArrayCallBacks);
        }
        CFArrayAppendValue(s->_dataQueue, (void *)(uintptr_t)sock);
        CFArrayAppendValue(s->_addressQueue, address);
        CFRelease(address);
        if ((s->_f.client & kCFSocketAcceptCallBack) != 0 && (s->_f.disabled & kCFSocketAcceptCallBack) == 0
            && __CFSocketIsScheduled(s)
        ) {
            __CFLock(&__CFActiveSocketsLock);
            /* restore socket to fds */
            __CFSocketSetFDForRead(s);
            __CFUnlock(&__CFActiveSocketsLock);
        }
    } else {
        __CFSocketLock(s);
        if (!__CFSocketIsValid(s) || (s->_f.disabled & kCFSocketReadCallBack) != 0) {
            __CFSocketUnlock(s);
            return;
        }
		
		if (causedByTimeout) {
#if defined(LOG_CFSOCKET)
			fprintf(stdout, "TIMEOUT RECEIVED - WILL SIGNAL IMMEDIATELY TO FLUSH (%ld buffered)\n", s->_bytesToBufferPos);
#endif
                    /* we've got a timeout, but no bytes read, and we don't have any bytes to send.  Ignore the timeout. */
                    if (s->_bytesToBufferPos == 0 && s->_leftoverBytes == NULL) {
#if defined(LOG_CFSOCKET)
                fprintf(stdout, "TIMEOUT - but no bytes, restoring to active set\n");
                fflush(stdout);
#endif
                // Clear the timeout notification time if there is no prefetched data left
                timerclear(&s->_readBufferTimeoutNotificationTime);

                __CFLock(&__CFActiveSocketsLock);
                /* restore socket to fds */
                __CFSocketSetFDForRead(s);
                __CFUnlock(&__CFActiveSocketsLock);
                __CFSocketUnlock(s);
                return;
            }
		} else if (s->_bytesToBuffer != 0 && ! s->_atEOF) {
			UInt8* base;
			CFIndex ctRead;
			CFIndex ctRemaining = s->_bytesToBuffer - s->_bytesToBufferPos;

			/* if our buffer has room, we go ahead and buffer */
			if (ctRemaining > 0) {
				base = CFDataGetMutableBytePtr(s->_readBuffer);

                                ctRead = read(CFSocketGetNative(s), &base[s->_bytesToBufferPos], ctRemaining);

				switch (ctRead) {
				case -1:
                                        if (errno == EAGAIN) { // no error
                                            __CFLock(&__CFActiveSocketsLock);
                                            /* restore socket to fds */
                                            __CFSocketSetFDForRead(s);
                                            __CFUnlock(&__CFActiveSocketsLock);
                                            __CFSocketUnlock(s);
                                            return;
                                        } else {
                                            s->_bufferedReadError = errno;
                                            s->_atEOF = true;
                                        }
#if defined(LOG_CFSOCKET)
					fprintf(stderr, "BUFFERED READ GOT ERROR %d\n", errno);
#endif
					break;

				case 0:
	#if defined(LOG_CFSOCKET)
					fprintf(stdout, "DONE READING (EOF) - GOING TO SIGNAL\n");
	#endif
					s->_atEOF = true;
					break;
			
				default:
					s->_bytesToBufferPos += ctRead;
					if (s->_bytesToBuffer != s->_bytesToBufferPos) {

                                            // Update the timeout notification time
                                            struct timeval timeNow = { 0 };
                                            gettimeofday(&timeNow, NULL);
                                            timeradd(&timeNow, &s->_readBufferTimeout, &s->_readBufferTimeoutNotificationTime);
	#if defined(LOG_CFSOCKET)
						fprintf(stdout, "READ %ld - need %ld MORE - GOING BACK FOR MORE\n", ctRead, s->_bytesToBuffer - s->_bytesToBufferPos);
	#endif
						__CFLock(&__CFActiveSocketsLock);
						/* restore socket to fds */
						__CFSocketSetFDForRead(s);
						__CFUnlock(&__CFActiveSocketsLock);
						__CFSocketUnlock(s);
						return;
					} else {
                                            // Clear the timeout notification time if the buffer is full
                                            timerclear(&s->_readBufferTimeoutNotificationTime);
	#if defined(LOG_CFSOCKET)
						fprintf(stdout, "DONE READING (read %ld bytes) - GOING TO SIGNAL\n", ctRead);
	#endif
					}
				}
			}
		}

		__CFSocketSetReadSignalled(s);
    }
#if defined(LOG_CFSOCKET)
    fprintf(stdout, "read signaling source for socket %d\n", s->_socket);
#endif
    CFRunLoopSourceSignal(s->_source0);
    CFMutableArrayRef runLoopsOrig = (CFMutableArrayRef)CFRetain(s->_runLoops);
    CFMutableArrayRef runLoopsCopy = CFArrayCreateMutableCopy(kCFAllocatorSystemDefault, 0, s->_runLoops);
    CFRunLoopSourceRef source0 = s->_source0;
    if (NULL != source0 && !CFRunLoopSourceIsValid(source0)) {
        source0 = NULL;
    }
    if (source0) CFRetain(source0);
    __CFSocketUnlock(s);
    CFRunLoopRef rl = __CFSocketCopyRunLoopToWakeUp(source0, runLoopsCopy);
    if (source0) CFRelease(source0);
    if (NULL != rl) {
        CFRunLoopWakeUp(rl);
        CFRelease(rl);
    }
        __CFSocketLock(s);
        if (runLoopsOrig == s->_runLoops) {
            s->_runLoops = runLoopsCopy;
            runLoopsCopy = NULL;
            CFRelease(runLoopsOrig);
        }
        __CFSocketUnlock(s);
        CFRelease(runLoopsOrig);
        if (runLoopsCopy) CFRelease(runLoopsCopy);
}

static struct timeval* intervalToTimeval(CFTimeInterval timeout, struct timeval* tv)
{
    if (timeout == 0.0)
        timerclear(tv);
    else {
        tv->tv_sec = (0 >= timeout || INT_MAX <= timeout) ? INT_MAX : (int)(float)floor(timeout);
        tv->tv_usec = (int)((timeout - floor(timeout)) * 1.0E6);
    }
    return tv;
}

/* note that this returns a pointer to the min value, which won't have changed during
 the dictionary apply, since we've got the active sockets lock held */
static void _calcMinTimeout_locked(const void* val, void* ctxt)
{
    CFSocketRef s = (CFSocketRef) val;
    struct timeval** minTime = (struct timeval**) ctxt;
    if (timerisset(&s->_readBufferTimeout) && (*minTime == NULL || timercmp(&s->_readBufferTimeout, *minTime, <)))
        *minTime = &s->_readBufferTimeout;
    else if (s->_leftoverBytes) {
        /* If there's anyone with leftover bytes, they'll need to be awoken immediately */
        static struct timeval sKickerTime = { 0, 0 };
        *minTime = &sKickerTime;
    }
}

void __CFSocketSetSocketReadBufferAttrs(CFSocketRef s, CFTimeInterval timeout, CFIndex length)
{
    struct timeval timeoutVal;
    
    intervalToTimeval(timeout, &timeoutVal);
    
    /* lock ordering is socket lock, activesocketslock */
    /* activesocketslock protects our timeout calculation */
    __CFSocketLock(s);
    __CFLock(&__CFActiveSocketsLock);

    if (s->_bytesToBuffer != length) {
        CFIndex ctBuffer = s->_bytesToBufferPos - s->_bytesToBufferReadPos;
        
        if (ctBuffer) {
            /* As originally envisaged, you were supposed to be sure to drain the buffer before 
             * issuing another request on the socket.  In practice, there seem to be times when we want to re-use 
             * the stream (or perhaps, are on our way to closing it out) and this policy doesn't work so well.  
             * So, if someone changes the buffer size while we have bytes already buffered, we put them 
             * aside and use them to satisfy any subsequent reads. 
             */
#if defined(LOG_CFSOCKET)
            fprintf(stdout, "%s(%d): WARNING: shouldn't set read buffer length while data (%ld bytes) is still in the read buffer (leftover total %ld)", __FUNCTION__, __LINE__, ctBuffer, s->_leftoverBytes? CFDataGetLength(s->_leftoverBytes) : 0);
#endif
            
            if (s->_leftoverBytes == NULL)
                s->_leftoverBytes = CFDataCreateMutable(CFGetAllocator(s), 0);
            
            /* append the current buffered bytes over.  We'll keep draining _leftoverBytes while we have them... */
            CFDataAppendBytes(s->_leftoverBytes, CFDataGetBytePtr(s->_readBuffer) + s->_bytesToBufferReadPos, ctBuffer);
            CFRelease(s->_readBuffer);
            s->_readBuffer = NULL;
            
            s->_bytesToBuffer = 0;
            s->_bytesToBufferPos = 0;
            s->_bytesToBufferReadPos = 0;
        }
        if (length == 0) {
            s->_bytesToBuffer = 0;
            s->_bytesToBufferPos = 0;
            s->_bytesToBufferReadPos = 0;
            if (s->_readBuffer) {
                CFRelease(s->_readBuffer);
                s->_readBuffer = NULL;
            }
            // Zero length buffer, smash the timeout
            timeoutVal.tv_sec = 0;
            timeoutVal.tv_usec = 0;
        } else {
            /* if the buffer shrank, we can re-use the old one */
            if (length > s->_bytesToBuffer) {
                if (s->_readBuffer) {
                    CFRelease(s->_readBuffer);
                    s->_readBuffer = NULL;
                }
            }
            
            s->_bytesToBuffer = length;
            s->_bytesToBufferPos = 0;
            s->_bytesToBufferReadPos = 0;
            if (s->_readBuffer == NULL) {
                s->_readBuffer = CFDataCreateMutable(kCFAllocatorSystemDefault, length);
                CFDataSetLength(s->_readBuffer, length);
            }
        }
    }
    
    if (timercmp(&s->_readBufferTimeout, &timeoutVal, !=)) {
        s->_readBufferTimeout = timeoutVal;
        __CFReadSocketsTimeoutInvalid = true;
    }
    
    __CFUnlock(&__CFActiveSocketsLock);
    __CFSocketUnlock(s);
}

CFIndex __CFSocketRead(CFSocketRef s, UInt8* buffer, CFIndex length, int* error)
{
#if defined(LOG_CFSOCKET)
	fprintf(stdout, "READING BYTES FOR SOCKET %d (%ld buffered, out of %ld desired, eof = %d, err = %d)\n", s->_socket, s->_bytesToBufferPos, s->_bytesToBuffer, s->_atEOF, s->_bufferedReadError);
#endif

    CFIndex result = -1;

    __CFSocketLock(s);

	*error = 0;
	
	/* Any leftover buffered bytes? */
	if (s->_leftoverBytes) {
		CFIndex ctBuffer = CFDataGetLength(s->_leftoverBytes);
#if defined(DEBUG)
		fprintf(stderr, "%s(%ld): WARNING: Draining %ld leftover bytes first\n\n", __FUNCTION__, (long)__LINE__, (long)ctBuffer);
#endif
		if (ctBuffer > length)
			ctBuffer = length;
		memcpy(buffer, CFDataGetBytePtr(s->_leftoverBytes), ctBuffer);
		if (ctBuffer < CFDataGetLength(s->_leftoverBytes))
			CFDataReplaceBytes(s->_leftoverBytes, CFRangeMake(0, ctBuffer), NULL, 0);
		else {
			CFRelease(s->_leftoverBytes);
			s->_leftoverBytes = NULL;
		}
		result = ctBuffer;
		goto unlock;
	}
	
	/* return whatever we've buffered */
	if (s->_bytesToBuffer != 0) {
		CFIndex ctBuffer = s->_bytesToBufferPos - s->_bytesToBufferReadPos;
		if (ctBuffer > 0) {
			/* drain our buffer first */
			if (ctBuffer > length)
				ctBuffer = length;
			memcpy(buffer, CFDataGetBytePtr(s->_readBuffer) + s->_bytesToBufferReadPos, ctBuffer);
			s->_bytesToBufferReadPos += ctBuffer;
			if (s->_bytesToBufferReadPos == s->_bytesToBufferPos) {
#if defined(LOG_CFSOCKET)
				fprintf(stdout, "DRAINED BUFFER - SHOULD START BUFFERING AGAIN!\n");
#endif
				s->_bytesToBufferPos = 0;
				s->_bytesToBufferReadPos = 0;
			}
			
#if defined(LOG_CFSOCKET)
			fprintf(stdout, "SLURPED %ld BYTES FROM BUFFER %ld LEFT TO READ!\n", ctBuffer, length);
#endif

			result = ctBuffer;
            goto unlock;
		}
	}
	/* nothing buffered, or no buffer selected */
	
	/* Did we get an error on a previous read (or buffered read)? */
	if (s->_bufferedReadError != 0) {
#if defined(LOG_CFSOCKET)
		fprintf(stdout, "RETURNING ERROR %d\n", s->_bufferedReadError);
#endif
		*error = s->_bufferedReadError;
        result = -1;
        goto unlock;
	}
	
	/* nothing buffered, if we've hit eof, don't bother reading any more */
	if (s->_atEOF) {
#if defined(LOG_CFSOCKET)
		fprintf(stdout, "RETURNING EOF\n");
#endif
		result = 0;
        goto unlock;
	}
	
	/* normal read */
	result = read(CFSocketGetNative(s), buffer, length);
#if defined(LOG_CFSOCKET)
	fprintf(stdout, "READ %ld bytes", result);
#endif

    if (result == 0) {
        /* note that we hit EOF */
        s->_atEOF = true;
    } else if (result < 0) {
        *error = errno;

        /* if it wasn't EAGAIN, record it (although we shouldn't get called again) */
        if (*error != EAGAIN) {
            s->_bufferedReadError = *error;
        }
    }

unlock:
    __CFSocketUnlock(s);

    return result;
}

Boolean __CFSocketGetBytesAvailable(CFSocketRef s, CFIndex* ctBytesAvailable)
{
	CFIndex ctBuffer = s->_bytesToBufferPos - s->_bytesToBufferReadPos;
	if (ctBuffer != 0) {
		*ctBytesAvailable = ctBuffer;
		return true;
	} else {
		int result;
	    unsigned long bytesAvailable;
	    result = ioctlsocket(CFSocketGetNative(s), FIONREAD, &bytesAvailable);
		if (result < 0)
			return false;
		*ctBytesAvailable = (CFIndex) bytesAvailable;
		return true;
	}
}

#if defined(LOG_CFSOCKET)
static void __CFSocketWriteSocketList(CFArrayRef sockets, CFDataRef fdSet, Boolean onlyIfSet) {
    fd_set *tempfds = (fd_set *)CFDataGetBytePtr(fdSet);
    SInt32 idx, cnt;
    for (idx = 0, cnt = CFArrayGetCount(sockets); idx < cnt; idx++) {
        CFSocketRef s = (CFSocketRef)CFArrayGetValueAtIndex(sockets, idx);
        if (FD_ISSET(s->_socket, tempfds)) {
            fprintf(stdout, "%d ", s->_socket);
        } else if (!onlyIfSet) {
            fprintf(stdout, "(%d) ", s->_socket);
        }
    }
}
#endif

static void
clearInvalidFileDescriptors(CFMutableDataRef d)
{
    if (d) {
        SInt32 count = __CFSocketFdGetSize(d);
        fd_set* s = (fd_set*) CFDataGetMutableBytePtr(d);
        for (SInt32 idx = 0;  idx < count;  idx++) {
            if (FD_ISSET(idx, s))
                if (! __CFNativeSocketIsValid(idx)) {
                    FD_CLR(idx, s);
                }
        }
    }
}

static void
manageSelectError()
{
    SInt32 selectError = __CFSocketLastError();
#if defined(LOG_CFSOCKET)
    fprintf(stdout, "socket manager received error %ld from select\n", (long)selectError);
#endif
    if (EBADF == selectError) {
        CFMutableArrayRef invalidSockets = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);

        __CFLock(&__CFActiveSocketsLock);
        CFIndex cnt = CFArrayGetCount(__CFWriteSockets);
        CFIndex idx;
        for (idx = 0; idx < cnt; idx++) {
            CFSocketRef s = (CFSocketRef)CFArrayGetValueAtIndex(__CFWriteSockets, idx);
            if (!__CFNativeSocketIsValid(s->_socket)) {
#if defined(LOG_CFSOCKET)
                fprintf(stdout, "socket manager found write socket %d invalid\n", s->_socket);
#endif
                CFArrayAppendValue(invalidSockets, s);
            }
        }
        cnt = CFArrayGetCount(__CFReadSockets);
        for (idx = 0; idx < cnt; idx++) {
            CFSocketRef s = (CFSocketRef)CFArrayGetValueAtIndex(__CFReadSockets, idx);
            if (!__CFNativeSocketIsValid(s->_socket)) {
#if defined(LOG_CFSOCKET)
                fprintf(stdout, "socket manager found read socket %d invalid\n", s->_socket);
#endif
                CFArrayAppendValue(invalidSockets, s);
            }
        }


        cnt = CFArrayGetCount(invalidSockets);

        /* Note that we're doing this only when we got EBADF but otherwise
         * don't have an explicit bad descriptor.  Note that the lock is held now.
         * Finally, note that cnt == 0 doesn't necessarily mean
         * that this loop will do anything, since fd's may have been invalidated
         * while we were in select.
         */
        if (cnt == 0) {
#if defined(LOG_CFSOCKET)
            fprintf(stdout, "socket manager received EBADF(1): No sockets were marked as invalid, cleaning out fdsets\n");
#endif

            clearInvalidFileDescriptors(__CFReadSocketsFds);
            clearInvalidFileDescriptors(__CFWriteSocketsFds);
        }

        __CFUnlock(&__CFActiveSocketsLock);

        for (idx = 0; idx < cnt; idx++) {
            CFSocketInvalidate(((CFSocketRef)CFArrayGetValueAtIndex(invalidSockets, idx)));
        }
        CFRelease(invalidSockets);
    }
}

static void *__CFSocketManager(void * arg)
{
    pthread_setname_np("com.apple.CFSocket.private");
    if (objc_collectingEnabled()) objc_registerThreadWithCollector();
    SInt32 nrfds, maxnrfds, fdentries = 1;
    SInt32 rfds, wfds;
    fd_set *exceptfds = NULL;
    fd_set *writefds = (fd_set *)CFAllocatorAllocate(kCFAllocatorSystemDefault, fdentries * sizeof(fd_mask), 0);
    fd_set *readfds = (fd_set *)CFAllocatorAllocate(kCFAllocatorSystemDefault, fdentries * sizeof(fd_mask), 0);
    fd_set *tempfds;
    SInt32 idx, cnt;
    uint8_t buffer[256];
    CFMutableArrayRef selectedWriteSockets = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
    CFMutableArrayRef selectedReadSockets = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
    CFIndex selectedWriteSocketsIndex = 0, selectedReadSocketsIndex = 0;
    
    struct timeval tv;
    struct timeval* pTimeout = NULL;
    struct timeval timeBeforeSelect;
    
    for (;;) {
        __CFLock(&__CFActiveSocketsLock);
        __CFSocketManagerIteration++;
#if defined(LOG_CFSOCKET)
        fprintf(stdout, "socket manager iteration %lu looking at read sockets ", (unsigned long)__CFSocketManagerIteration);
        __CFSocketWriteSocketList(__CFReadSockets, __CFReadSocketsFds, FALSE);
        if (0 < CFArrayGetCount(__CFWriteSockets)) {
            fprintf(stdout, " and write sockets ");
            __CFSocketWriteSocketList(__CFWriteSockets, __CFWriteSocketsFds, FALSE);
        }
        fprintf(stdout, "\n");
#endif
        rfds = __CFSocketFdGetSize(__CFReadSocketsFds);
        wfds = __CFSocketFdGetSize(__CFWriteSocketsFds);
        maxnrfds = __CFMax(rfds, wfds);
        if (maxnrfds > fdentries * (int)NFDBITS) {
            fdentries = (maxnrfds + NFDBITS - 1) / NFDBITS;
            writefds = (fd_set *)CFAllocatorReallocate(kCFAllocatorSystemDefault, writefds, fdentries * sizeof(fd_mask), 0);
            readfds = (fd_set *)CFAllocatorReallocate(kCFAllocatorSystemDefault, readfds, fdentries * sizeof(fd_mask), 0);
        }
        memset(writefds, 0, fdentries * sizeof(fd_mask)); 
        memset(readfds, 0, fdentries * sizeof(fd_mask));
        CFDataGetBytes(__CFWriteSocketsFds, CFRangeMake(0, CFDataGetLength(__CFWriteSocketsFds)), (UInt8 *)writefds);
        CFDataGetBytes(__CFReadSocketsFds, CFRangeMake(0, CFDataGetLength(__CFReadSocketsFds)), (UInt8 *)readfds); 
		
        if (__CFReadSocketsTimeoutInvalid) {
            struct timeval* minTimeout = NULL;
            __CFReadSocketsTimeoutInvalid = false;
#if defined(LOG_CFSOCKET)
            fprintf(stdout, "Figuring out which sockets have timeouts...\n");
#endif
            CFArrayApplyFunction(__CFReadSockets, CFRangeMake(0, CFArrayGetCount(__CFReadSockets)), _calcMinTimeout_locked, (void*) &minTimeout);

            if (minTimeout == NULL) {
#if defined(LOG_CFSOCKET)
                fprintf(stdout, "No one wants a timeout!\n");
#endif
                pTimeout = NULL;
            } else {
#if defined(LOG_CFSOCKET)
                fprintf(stdout, "timeout will be %ld, %d!\n", minTimeout->tv_sec, minTimeout->tv_usec);
#endif
                tv = *minTimeout;
                pTimeout = &tv;
            }
        }

        if (pTimeout) {
#if defined(LOG_CFSOCKET)
            fprintf(stdout, "select will have a %ld, %d timeout\n", pTimeout->tv_sec, pTimeout->tv_usec);
#endif
            gettimeofday(&timeBeforeSelect, NULL);
        }
		
		__CFUnlock(&__CFActiveSocketsLock);

#if DEPLOYMENT_TARGET_WINDOWS
        // On Windows, select checks connection failed sockets via the exceptfds parameter. connection succeeded is checked via writefds. We need both.
        exceptfds = writefds;
#endif
        nrfds = select(maxnrfds, readfds, writefds, exceptfds, pTimeout);

#if defined(LOG_CFSOCKET)
        fprintf(stdout, "socket manager woke from select, ret=%ld\n", (long)nrfds);
#endif

		/*
		 * select returned a timeout
		 */
        if (0 == nrfds) {
			struct timeval timeAfterSelect;
			struct timeval deltaTime;
			gettimeofday(&timeAfterSelect, NULL);
			/* timeBeforeSelect becomes the delta */
			timersub(&timeAfterSelect, &timeBeforeSelect, &deltaTime);
			
#if defined(LOG_CFSOCKET)
			fprintf(stdout, "Socket manager received timeout - kicking off expired reads (expired delta %ld, %d)\n", deltaTime.tv_sec, deltaTime.tv_usec);
#endif
			
			__CFLock(&__CFActiveSocketsLock);
			
			tempfds = NULL;
			cnt = CFArrayGetCount(__CFReadSockets);
			for (idx = 0; idx < cnt; idx++) {
				CFSocketRef s = (CFSocketRef)CFArrayGetValueAtIndex(__CFReadSockets, idx);
				if (timerisset(&s->_readBufferTimeout) || s->_leftoverBytes) {
					CFSocketNativeHandle sock = s->_socket;
					// We might have an new element in __CFReadSockets that we weren't listening to,
					// in which case we must be sure not to test a bit in the fdset that is
					// outside our mask size.
					Boolean sockInBounds = (0 <= sock && sock < maxnrfds);
					/* if this sockets timeout is less than or equal elapsed time, then signal it */
					if (INVALID_SOCKET != sock && sockInBounds) {
#if defined(LOG_CFSOCKET)
						fprintf(stdout, "Expiring socket %d (delta %ld, %d)\n", sock, s->_readBufferTimeout.tv_sec, s->_readBufferTimeout.tv_usec);
#endif
						CFArraySetValueAtIndex(selectedReadSockets, selectedReadSocketsIndex, s);
						selectedReadSocketsIndex++;
						/* socket is removed from fds here, will be restored in read handling or in perform function */
						if (!tempfds) tempfds = (fd_set *)CFDataGetMutableBytePtr(__CFReadSocketsFds);
						FD_CLR(sock, tempfds);
					}
				}
			}
			
			__CFUnlock(&__CFActiveSocketsLock);
			
			/* and below, we dispatch through the normal read dispatch mechanism */
		} 
		
        if (0 > nrfds) {
            manageSelectError();
            continue;
        }
        if (FD_ISSET(__CFWakeupSocketPair[1], readfds)) {
            recv(__CFWakeupSocketPair[1], (char *)buffer, sizeof(buffer), 0);
#if defined(LOG_CFSOCKET)
            fprintf(stdout, "socket manager received %c on wakeup socket\n", buffer[0]);
#endif
        }
        __CFLock(&__CFActiveSocketsLock);
        tempfds = NULL;
        cnt = CFArrayGetCount(__CFWriteSockets);
        for (idx = 0; idx < cnt; idx++) {
            CFSocketRef s = (CFSocketRef)CFArrayGetValueAtIndex(__CFWriteSockets, idx);
            CFSocketNativeHandle sock = s->_socket;
            // We might have an new element in __CFWriteSockets that we weren't listening to,
            // in which case we must be sure not to test a bit in the fdset that is
            // outside our mask size.
            Boolean sockInBounds = (0 <= sock && sock < maxnrfds);
            if (INVALID_SOCKET != sock && sockInBounds) {
                if (FD_ISSET(sock, writefds)) {
                    CFArraySetValueAtIndex(selectedWriteSockets, selectedWriteSocketsIndex, s);
                    selectedWriteSocketsIndex++;
                    /* socket is removed from fds here, restored by CFSocketReschedule */
                    if (!tempfds) tempfds = (fd_set *)CFDataGetMutableBytePtr(__CFWriteSocketsFds);
                    FD_CLR(sock, tempfds);
// CFLog(5, CFSTR("Manager: cleared socket %p from write fds"), s);
                }
            }
        }
        tempfds = NULL;
        cnt = CFArrayGetCount(__CFReadSockets);
        
        struct timeval timeNow = { 0 };
        if (pTimeout) {
            gettimeofday(&timeNow, NULL);
        }
        
        for (idx = 0; idx < cnt; idx++) {
            CFSocketRef s = (CFSocketRef)CFArrayGetValueAtIndex(__CFReadSockets, idx);
            CFSocketNativeHandle sock = s->_socket;
            // We might have an new element in __CFReadSockets that we weren't listening to,
            // in which case we must be sure not to test a bit in the fdset that is
            // outside our mask size.
            Boolean sockInBounds = (0 <= sock && sock < maxnrfds);

            // Check if we hit the timeout
            s->_hitTheTimeout = false;
            if (pTimeout && sockInBounds && 0 != nrfds && !FD_ISSET(sock, readfds) &&
                timerisset(&s->_readBufferTimeoutNotificationTime) &&
                timercmp(&timeNow, &s->_readBufferTimeoutNotificationTime, >))
            {
                s->_hitTheTimeout = true;
            }

            if (INVALID_SOCKET != sock && sockInBounds && (FD_ISSET(sock, readfds) || s->_hitTheTimeout)) {
                CFArraySetValueAtIndex(selectedReadSockets, selectedReadSocketsIndex, s);
                selectedReadSocketsIndex++;
                /* socket is removed from fds here, will be restored in read handling or in perform function */
                if (!tempfds) tempfds = (fd_set *)CFDataGetMutableBytePtr(__CFReadSocketsFds);
                FD_CLR(sock, tempfds);
            }
        }
        __CFUnlock(&__CFActiveSocketsLock);
        
        for (idx = 0; idx < selectedWriteSocketsIndex; idx++) {
            CFSocketRef s = (CFSocketRef)CFArrayGetValueAtIndex(selectedWriteSockets, idx);
            if (kCFNull == (CFNullRef)s) continue;
#if defined(LOG_CFSOCKET)
            fprintf(stdout, "socket manager signaling socket %d for write\n", s->_socket);
#endif
            __CFSocketHandleWrite(s, FALSE);
            CFArraySetValueAtIndex(selectedWriteSockets, idx, kCFNull);
        }
        selectedWriteSocketsIndex = 0;
        
        for (idx = 0; idx < selectedReadSocketsIndex; idx++) {
            CFSocketRef s = (CFSocketRef)CFArrayGetValueAtIndex(selectedReadSockets, idx);
            if (kCFNull == (CFNullRef)s) continue;
#if defined(LOG_CFSOCKET)
            fprintf(stdout, "socket manager signaling socket %d for read\n", s->_socket);
#endif
            __CFSocketHandleRead(s, nrfds == 0 || s->_hitTheTimeout);
            CFArraySetValueAtIndex(selectedReadSockets, idx, kCFNull);
        }
        selectedReadSocketsIndex = 0;
    }
    return NULL;
}

static CFStringRef __CFSocketCopyDescription(CFTypeRef cf) {
    CFSocketRef s = (CFSocketRef)cf;
    CFMutableStringRef result;
    CFStringRef contextDesc = NULL;
    void *contextInfo = NULL;
    CFStringRef (*contextCopyDescription)(const void *info) = NULL;
    result = CFStringCreateMutable(CFGetAllocator(s), 0);
    __CFSocketLock(s);
    void *addr = s->_callout;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    Dl_info info;
    const char *name = (dladdr(addr, &info) && info.dli_saddr == addr && info.dli_sname) ? info.dli_sname : "???";
#else
    // don't bother trying to figure out callout names
    const char *name = "<unknown>";
#endif
    CFStringAppendFormat(result, NULL, CFSTR("<CFSocket %p [%p]>{valid = %s, type = %d, socket = %d, socket set count = %ld,\n    callback types = 0x%x, callout = %s (%p), source = %p,\n    run loops = %@,\n    context = "), cf, CFGetAllocator(s), (__CFSocketIsValid(s) ? "Yes" : "No"), (int)(s->_socketType), s->_socket, (long)s->_socketSetCount, __CFSocketCallBackTypes(s), name, addr, s->_source0, s->_runLoops);
    contextInfo = s->_context.info;
    contextCopyDescription = s->_context.copyDescription;
    __CFSocketUnlock(s);
    if (NULL != contextInfo && NULL != contextCopyDescription) {
        contextDesc = (CFStringRef)contextCopyDescription(contextInfo);
    }
    if (NULL == contextDesc) {
        contextDesc = CFStringCreateWithFormat(CFGetAllocator(s), NULL, CFSTR("<CFSocket context %p>"), contextInfo);
    }
    CFStringAppend(result, contextDesc);
    CFStringAppend(result, CFSTR("}"));
    CFRelease(contextDesc);
    return result;
}

static void __CFSocketDeallocate(CFTypeRef cf) {
    /* Since CFSockets are cached, we can only get here sometime after being invalidated */
    CFSocketRef s = (CFSocketRef)cf;
    if (NULL != s->_address) {
        CFRelease(s->_address);
        s->_address = NULL;
    }
    if (NULL != s->_readBuffer) {
        CFRelease(s->_readBuffer);
        s->_readBuffer = NULL;
    }
	if (NULL != s->_leftoverBytes) {
		CFRelease(s->_leftoverBytes);
		s->_leftoverBytes = NULL;
	}
    timerclear(&s->_readBufferTimeout);
    s->_bytesToBuffer = 0;
    s->_bytesToBufferPos = 0;
    s->_bytesToBufferReadPos = 0;
    s->_atEOF = true;
	s->_bufferedReadError = 0;
}

static CFTypeID __kCFSocketTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFSocketClass = {
    0,
    "CFSocket",
    NULL,      // init
    NULL,      // copy
    __CFSocketDeallocate,
    NULL,      // equal
    NULL,      // hash
    NULL,      // 
    __CFSocketCopyDescription
};

CFTypeID CFSocketGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{
	__kCFSocketTypeID = _CFRuntimeRegisterClass(&__CFSocketClass); // initOnce covered
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
        struct rlimit lim1;
        int ret1 = getrlimit(RLIMIT_NOFILE, &lim1);
        int mib[] = {CTL_KERN, KERN_MAXFILESPERPROC};
        int maxfd = 0;
        size_t len = sizeof(int);
        int ret0 = sysctl(mib, 2, &maxfd, &len, NULL, 0);
        if (0 == ret0 && 0 == ret1 && lim1.rlim_max < maxfd) maxfd = lim1.rlim_max;
        if (0 == ret1 && lim1.rlim_cur < maxfd) {
            struct rlimit lim2 = lim1;
            lim2.rlim_cur += 2304;
            if (maxfd < lim2.rlim_cur) lim2.rlim_cur = maxfd;
            setrlimit(RLIMIT_NOFILE, &lim2);
            // we try, but do not go to extraordinary measures
        }
#endif
    });
    return __kCFSocketTypeID;
}

#if DEPLOYMENT_TARGET_WINDOWS
struct _args {
    void *func;
    void *arg;
    HANDLE handle;
};
static unsigned __stdcall __CFWinThreadFunc(void *arg) {
    struct _args *args = (struct _args*)arg;
    ((void (*)(void *))args->func)(args->arg);
    CloseHandle(args->handle);
    CFAllocatorDeallocate(kCFAllocatorSystemDefault, arg);
    _endthreadex(0);
    return 0;
}
#endif

static CFSocketRef _CFSocketCreateWithNative(CFAllocatorRef allocator, CFSocketNativeHandle sock, CFOptionFlags callBackTypes, CFSocketCallBack callout, const CFSocketContext *context, Boolean useExistingInstance) {
    CHECK_FOR_FORK();
    CFSocketRef memory;
    int typeSize = sizeof(memory->_socketType);
    __CFLock(&__CFActiveSocketsLock);
    if (NULL == __CFReadSockets) __CFSocketInitializeSockets();
    __CFUnlock(&__CFActiveSocketsLock);
    __CFLock(&__CFAllSocketsLock);
    if (NULL == __CFAllSockets) {
        __CFAllSockets = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, NULL, &kCFTypeDictionaryValueCallBacks);
    }
    if (INVALID_SOCKET != sock && CFDictionaryGetValueIfPresent(__CFAllSockets, (void *)(uintptr_t)sock, (const void **)&memory)) {
        if (useExistingInstance) {
			__CFUnlock(&__CFAllSocketsLock);
			CFRetain(memory);
			return memory;
		} else {
#if defined(LOG_CFSOCKET)
			fprintf(stdout, "useExistingInstance is FALSE, removing existing instance %p from __CFAllSockets\n", memory);
#endif
			__CFUnlock(&__CFAllSocketsLock);
			CFSocketInvalidate(memory);
			__CFLock(&__CFAllSocketsLock);
		}
    }
    memory = (CFSocketRef)_CFRuntimeCreateInstance(allocator, CFSocketGetTypeID(), sizeof(struct __CFSocket) - sizeof(CFRuntimeBase), NULL);
    if (NULL == memory) {
        __CFUnlock(&__CFAllSocketsLock);
        return NULL;
    }
    __CFSocketSetCallBackTypes(memory, callBackTypes);
    if (INVALID_SOCKET != sock) __CFSocketSetValid(memory);
    __CFSocketUnsetWriteSignalled(memory);
    __CFSocketUnsetReadSignalled(memory);
    memory->_f.client = ((callBackTypes & (~kCFSocketConnectCallBack)) & (~kCFSocketWriteCallBack)) | kCFSocketCloseOnInvalidate;
    memory->_f.disabled = 0;
    memory->_f.connected = FALSE;
    memory->_f.writableHint = FALSE;
    memory->_f.closeSignaled = FALSE;
    memory->_lock = CFLockInit;
    memory->_writeLock = CFLockInit;
    memory->_socket = sock;
    if (INVALID_SOCKET == sock || 0 != getsockopt(sock, SOL_SOCKET, SO_TYPE, (char *)&(memory->_socketType), (socklen_t *)&typeSize)) memory->_socketType = 0;		// cast for WinSock bad API
    memory->_errorCode = 0;
    memory->_address = NULL;
    memory->_peerAddress = NULL;
    memory->_socketSetCount = 0;
    memory->_source0 = NULL;
    if (INVALID_SOCKET != sock) {
        memory->_runLoops = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, NULL);
    } else {
        memory->_runLoops = NULL;
    }
    memory->_callout = callout;
    memory->_dataQueue = NULL;
    memory->_addressQueue = NULL;
    memory->_context.info = 0;
    memory->_context.retain = 0;
    memory->_context.release = 0;
    memory->_context.copyDescription = 0;
    timerclear(&memory->_readBufferTimeout);
    timerclear(&memory->_readBufferTimeoutNotificationTime);
    memory->_hitTheTimeout = false;
    memory->_readBuffer = NULL;
    memory->_bytesToBuffer = 0;
    memory->_bytesToBufferPos = 0;
    memory->_bytesToBufferReadPos = 0;
    memory->_atEOF = false;
    memory->_bufferedReadError = 0;
    memory->_leftoverBytes = NULL;
    
    if (INVALID_SOCKET != sock) CFDictionaryAddValue(__CFAllSockets, (void *)(uintptr_t)sock, memory);
    if (NULL == __CFSocketManagerThread) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD
        pthread_t tid = 0;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_attr_set_qos_class_np(&attr, qos_class_main(), 0);
        pthread_create(&tid, &attr, __CFSocketManager, 0);
        pthread_attr_destroy(&attr);
//warning CF: we dont actually know that a pthread_t is the same size as void *
        __CFSocketManagerThread = (void *)tid;
#elif DEPLOYMENT_TARGET_WINDOWS
        unsigned tid;
        struct _args *args = (struct _args*)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(struct _args), 0);
        if (__CFOASafe) __CFSetLastAllocationEventName(args, "CFUtilities (thread-args)");
        HANDLE handle;
        args->func = __CFSocketManager;
        args->arg = 0;
        /* The thread is created suspended, because otherwise there would be a race between the assignment below of the handle field, and it's possible use in the thread func above. */
        args->handle = (HANDLE)_beginthreadex(NULL, 0, __CFWinThreadFunc, args, CREATE_SUSPENDED, &tid);
        handle = args->handle;
        ResumeThread(handle);
        __CFSocketManagerThread = handle;
#endif
    }
    __CFUnlock(&__CFAllSocketsLock);
    if (NULL != context) {
        void *contextInfo = context->retain ? (void *)context->retain(context->info) : context->info;
        __CFSocketLock(memory);
        memory->_context.retain = context->retain;
        memory->_context.release = context->release;
        memory->_context.copyDescription = context->copyDescription;
        memory->_context.info = contextInfo;
        __CFSocketUnlock(memory);
    }
#if defined(LOG_CFSOCKET)
    CFLog(5, CFSTR("CFSocketCreateWithNative(): created socket %p (%d) with callbacks 0x%x"), memory, memory->_socket, callBackTypes);
#endif
    return memory;
}

CFSocketRef CFSocketCreateWithNative(CFAllocatorRef allocator, CFSocketNativeHandle sock, CFOptionFlags callBackTypes, CFSocketCallBack callout, const CFSocketContext *context) {
	return _CFSocketCreateWithNative(allocator, sock, callBackTypes, callout, context, TRUE);
}

void CFSocketInvalidate(CFSocketRef s) {
// CFLog(5, CFSTR("CFSocketInvalidate(%p) starting"), s);
    CHECK_FOR_FORK();
    UInt32 previousSocketManagerIteration;
    __CFGenericValidateType(s, CFSocketGetTypeID());
#if defined(LOG_CFSOCKET)
    fprintf(stdout, "invalidating socket %d with flags 0x%x disabled 0x%x connected 0x%x\n", s->_socket, s->_f.client, s->_f.disabled, s->_f.connected);
#endif
    CFRetain(s);
    __CFLock(&__CFAllSocketsLock);
    __CFSocketLock(s);
    if (__CFSocketIsValid(s)) {
        SInt32 idx;
        CFRunLoopSourceRef source0;
        void *contextInfo = NULL;
        void (*contextRelease)(const void *info) = NULL;
        __CFSocketUnsetValid(s);
        __CFSocketUnsetWriteSignalled(s);
        __CFSocketUnsetReadSignalled(s);
        __CFLock(&__CFActiveSocketsLock);
        idx = CFArrayGetFirstIndexOfValue(__CFWriteSockets, CFRangeMake(0, CFArrayGetCount(__CFWriteSockets)), s);
        if (0 <= idx) {
            CFArrayRemoveValueAtIndex(__CFWriteSockets, idx);
            __CFSocketClearFDForWrite(s);
        }
        // No need to clear FD's for V1 sources, since we'll just throw the whole event away
        idx = CFArrayGetFirstIndexOfValue(__CFReadSockets, CFRangeMake(0, CFArrayGetCount(__CFReadSockets)), s);
        if (0 <= idx) {
            CFArrayRemoveValueAtIndex(__CFReadSockets, idx);
            __CFSocketClearFDForRead(s);
        }
        previousSocketManagerIteration = __CFSocketManagerIteration;
        __CFUnlock(&__CFActiveSocketsLock);
        CFDictionaryRemoveValue(__CFAllSockets, (void *)(uintptr_t)(s->_socket));
        if ((s->_f.client & kCFSocketCloseOnInvalidate) != 0) closesocket(s->_socket);
        s->_socket = INVALID_SOCKET;
        if (NULL != s->_peerAddress) {
            CFRelease(s->_peerAddress);
            s->_peerAddress = NULL;
        }
        if (NULL != s->_dataQueue) {
            CFRelease(s->_dataQueue);
            s->_dataQueue = NULL;
        }
        if (NULL != s->_addressQueue) {
            CFRelease(s->_addressQueue);
            s->_addressQueue = NULL;
        }
        s->_socketSetCount = 0;
        
        // we'll need this later
        CFArrayRef runLoops = (CFArrayRef)CFRetain(s->_runLoops);        
        CFRelease(s->_runLoops);
        
        s->_runLoops = NULL;
        source0 = s->_source0;
        s->_source0 = NULL;
        contextInfo = s->_context.info;
        contextRelease = s->_context.release;
        s->_context.info = 0;
        s->_context.retain = 0;
        s->_context.release = 0;
        s->_context.copyDescription = 0;
        __CFSocketUnlock(s);
        
        // Do this after the socket unlock to avoid deadlock (10462525)
        for (idx = CFArrayGetCount(runLoops); idx--;) {
            CFRunLoopWakeUp((CFRunLoopRef)CFArrayGetValueAtIndex(runLoops, idx));
        }
        CFRelease(runLoops);

        if (NULL != contextRelease) {
            contextRelease(contextInfo);
        }
        if (NULL != source0) {
            CFRunLoopSourceInvalidate(source0);
            CFRelease(source0);
        }
    } else {
        __CFSocketUnlock(s);
    }
    __CFUnlock(&__CFAllSocketsLock);
    CFRelease(s);
#if defined(LOG_CFSOCKET)
    CFLog(5, CFSTR("CFSocketInvalidate(%p) done"), s);
#endif
}

Boolean CFSocketIsValid(CFSocketRef s) {
    CHECK_FOR_FORK();
    __CFGenericValidateType(s, CFSocketGetTypeID());
    return __CFSocketIsValid(s);
}

CFSocketNativeHandle CFSocketGetNative(CFSocketRef s) {
    CHECK_FOR_FORK();
    __CFGenericValidateType(s, CFSocketGetTypeID());
    return s->_socket;
}

CFDataRef CFSocketCopyAddress(CFSocketRef s) {
    CHECK_FOR_FORK();
    CFDataRef result = NULL;
    __CFGenericValidateType(s, CFSocketGetTypeID());
    __CFSocketLock(s);
    __CFSocketEstablishAddress(s);
    if (NULL != s->_address) {
        result = (CFDataRef)CFRetain(s->_address);
    }
    __CFSocketUnlock(s);
#if defined(LOG_CFSOCKET)
    CFLog(5, CFSTR("CFSocketCopyAddress(): created socket %p address %@"), s, result);
#endif
    return result;
}

CFDataRef CFSocketCopyPeerAddress(CFSocketRef s) {
    CHECK_FOR_FORK();
    CFDataRef result = NULL;
    __CFGenericValidateType(s, CFSocketGetTypeID());
    __CFSocketLock(s);
    __CFSocketEstablishPeerAddress(s);
    if (NULL != s->_peerAddress) {
        result = (CFDataRef)CFRetain(s->_peerAddress);
    }
    __CFSocketUnlock(s);
#if defined(LOG_CFSOCKET)
    CFLog(5, CFSTR("CFSocketCopyAddress(): created socket %p peer address %@"), s, result);
#endif
    return result;
}

void CFSocketGetContext(CFSocketRef s, CFSocketContext *context) {
    CHECK_FOR_FORK();
    __CFGenericValidateType(s, CFSocketGetTypeID());
    CFAssert1(0 == context->version, __kCFLogAssertion, "%s(): context version not initialized to 0", __PRETTY_FUNCTION__);
    *context = s->_context;
}

CFOptionFlags CFSocketGetSocketFlags(CFSocketRef s) {
    CHECK_FOR_FORK();
    __CFGenericValidateType(s, CFSocketGetTypeID());
    return s->_f.client;
}

void CFSocketSetSocketFlags(CFSocketRef s, CFOptionFlags flags) {
    CHECK_FOR_FORK();
    __CFGenericValidateType(s, CFSocketGetTypeID());
    __CFSocketLock(s);
#if defined(LOG_CFSOCKET)
        fprintf(stdout, "setting flags for socket %d, from 0x%x to 0x%lx\n", s->_socket, s->_f.client, flags);
#endif
    s->_f.client = flags;
    __CFSocketUnlock(s);
// CFLog(5, CFSTR("CFSocketSetSocketFlags(%p, 0x%x)"), s, flags);
}

void CFSocketDisableCallBacks(CFSocketRef s, CFOptionFlags callBackTypes) {
    CHECK_FOR_FORK();
    Boolean wakeup = false;
    uint8_t readCallBackType;
    __CFGenericValidateType(s, CFSocketGetTypeID());
    __CFSocketLock(s);
    if (__CFSocketIsValid(s) && __CFSocketIsScheduled(s)) {
        callBackTypes &= __CFSocketCallBackTypes(s);
        readCallBackType = __CFSocketReadCallBackType(s);
        s->_f.disabled |= callBackTypes;
#if defined(LOG_CFSOCKET)
        fprintf(stdout, "unscheduling socket %d with flags 0x%x disabled 0x%x connected 0x%x for types 0x%lx\n", s->_socket, s->_f.client, s->_f.disabled, s->_f.connected, callBackTypes);
#endif
        __CFLock(&__CFActiveSocketsLock);
        if ((readCallBackType == kCFSocketAcceptCallBack) || !__CFSocketIsConnectionOriented(s)) s->_f.connected = TRUE;
        if (((callBackTypes & kCFSocketWriteCallBack) != 0) || (((callBackTypes & kCFSocketConnectCallBack) != 0) && !s->_f.connected)) {
            if (__CFSocketClearFDForWrite(s)) {
                // do not wake up the socket manager thread if all relevant write callbacks are disabled
                CFOptionFlags writeCallBacksAvailable = __CFSocketCallBackTypes(s) & (kCFSocketWriteCallBack | kCFSocketConnectCallBack);
                if (s->_f.connected) writeCallBacksAvailable &= ~kCFSocketConnectCallBack;
                if ((s->_f.disabled & writeCallBacksAvailable) != writeCallBacksAvailable) wakeup = true;
            }
        }
        if (readCallBackType != kCFSocketNoCallBack && (callBackTypes & readCallBackType) != 0) {
            if (__CFSocketClearFDForRead(s)) {
                // do not wake up the socket manager thread if callback type is read
                if (readCallBackType != kCFSocketReadCallBack) wakeup = true;
            }
        }
        __CFUnlock(&__CFActiveSocketsLock);
    }
    __CFSocketUnlock(s);
}

// "force" means to clear the disabled bits set by DisableCallBacks and always reenable.
// if (!force) we respect those bits, meaning they may stop us from enabling.
// In addition, if !force we assume that the sockets have already been added to the
// __CFReadSockets and __CFWriteSockets arrays.  This is true because the callbacks start
// enabled when the CFSocket is created (at which time we enable with force).
// Called with SocketLock held, returns with it released!
void __CFSocketEnableCallBacks(CFSocketRef s, CFOptionFlags callBackTypes, Boolean force, uint8_t wakeupChar) {
    CHECK_FOR_FORK();
    Boolean wakeup = FALSE;
    if (!callBackTypes) {
        __CFSocketUnlock(s);
        return;
    }
    if (__CFSocketIsValid(s) && __CFSocketIsScheduled(s)) {
        Boolean turnOnWrite = FALSE, turnOnConnect = FALSE, turnOnRead = FALSE;
        uint8_t readCallBackType = __CFSocketReadCallBackType(s);        
        callBackTypes &= __CFSocketCallBackTypes(s);
        if (force) s->_f.disabled &= ~callBackTypes;
#if defined(LOG_CFSOCKET)
        fprintf(stdout, "rescheduling socket %d with flags 0x%x disabled 0x%x connected 0x%x for types 0x%lx\n", s->_socket, s->_f.client, s->_f.disabled, s->_f.connected, callBackTypes);
#endif
        /* We will wait for connection only for connection-oriented, non-rendezvous sockets that are not already connected.  Mark others as already connected. */
        if ((readCallBackType == kCFSocketAcceptCallBack) || !__CFSocketIsConnectionOriented(s)) s->_f.connected = TRUE;

        // First figure out what to turn on
        if (s->_f.connected || (callBackTypes & kCFSocketConnectCallBack) == 0) {
            // if we want write callbacks and they're not disabled...
            if ((callBackTypes & kCFSocketWriteCallBack) != 0 && (s->_f.disabled & kCFSocketWriteCallBack) == 0) turnOnWrite = TRUE;
        } else {
            // if we want connect callbacks and they're not disabled...
            if ((callBackTypes & kCFSocketConnectCallBack) != 0 && (s->_f.disabled & kCFSocketConnectCallBack) == 0) turnOnConnect = TRUE;
        }
        // if we want read callbacks and they're not disabled...
        if (readCallBackType != kCFSocketNoCallBack && (callBackTypes & readCallBackType) != 0 && (s->_f.disabled & kCFSocketReadCallBack) == 0) turnOnRead = TRUE;

        // Now turn on the callbacks we've determined that we want on
        if (turnOnRead || turnOnWrite || turnOnConnect) {
            __CFLock(&__CFActiveSocketsLock);
            if (turnOnWrite || turnOnConnect) {
                if (force) {
                    SInt32 idx = CFArrayGetFirstIndexOfValue(__CFWriteSockets, CFRangeMake(0, CFArrayGetCount(__CFWriteSockets)), s);
                    if (kCFNotFound == idx) CFArrayAppendValue(__CFWriteSockets, s);
//                     if (kCFNotFound == idx) CFLog(5, CFSTR("__CFSocketEnableCallBacks: put %p in __CFWriteSockets list due to force and non-presence"), s);
                }
                if (__CFSocketSetFDForWrite(s)) wakeup = true;
            }
            if (turnOnRead) {
                if (force) {
                    SInt32 idx = CFArrayGetFirstIndexOfValue(__CFReadSockets, CFRangeMake(0, CFArrayGetCount(__CFReadSockets)), s);
                    if (kCFNotFound == idx) CFArrayAppendValue(__CFReadSockets, s);
                }
                if (__CFSocketSetFDForRead(s)) wakeup = true;
            }
            __CFUnlock(&__CFActiveSocketsLock);
        }
    }
    __CFSocketUnlock(s);
}

void CFSocketEnableCallBacks(CFSocketRef s, CFOptionFlags callBackTypes) {
    CHECK_FOR_FORK();
    __CFGenericValidateType(s, CFSocketGetTypeID());
    __CFSocketLock(s);
    __CFSocketEnableCallBacks(s, callBackTypes, TRUE, 'r');
// CFLog(5, CFSTR("CFSocketEnableCallBacks(%p, 0x%x) done"), s, callBackTypes);
}

static void __CFSocketSchedule(void *info, CFRunLoopRef rl, CFStringRef mode) {
    CFSocketRef s = (CFSocketRef)info;
    __CFSocketLock(s);
    //??? also need to arrange delivery of all pending data
    if (__CFSocketIsValid(s)) {
        CFMutableArrayRef runLoopsOrig = s->_runLoops;
        CFMutableArrayRef runLoopsCopy = CFArrayCreateMutableCopy(kCFAllocatorSystemDefault, 0, s->_runLoops);
        CFArrayAppendValue(runLoopsCopy, rl);
        s->_runLoops = runLoopsCopy;
        CFRelease(runLoopsOrig);
        s->_socketSetCount++;
        // Since the v0 source is listened to on the SocketMgr thread, no matter how many modes it
        // is added to we just need to enable it there once (and _socketSetCount gives us a refCount
        // to know when we can finally disable it).
        if (1 == s->_socketSetCount) {
#if defined(LOG_CFSOCKET)
            fprintf(stdout, "scheduling socket %d\n", s->_socket);
#endif
// CFLog(5, CFSTR("__CFSocketSchedule(%p, %p, %p)"), s, rl, mode);
            __CFSocketEnableCallBacks(s, __CFSocketCallBackTypes(s), TRUE, 's');  // unlocks s
        } else
            __CFSocketUnlock(s);
    } else
        __CFSocketUnlock(s);
}

static void __CFSocketCancel(void *info, CFRunLoopRef rl, CFStringRef mode) {
    CFSocketRef s = (CFSocketRef)info;
    SInt32 idx;
    __CFSocketLock(s);
    s->_socketSetCount--;
    if (0 == s->_socketSetCount) {
        __CFLock(&__CFActiveSocketsLock);
        idx = CFArrayGetFirstIndexOfValue(__CFWriteSockets, CFRangeMake(0, CFArrayGetCount(__CFWriteSockets)), s);
        if (0 <= idx) {
// CFLog(5, CFSTR("__CFSocketCancel: removing %p from __CFWriteSockets list"), s);
            CFArrayRemoveValueAtIndex(__CFWriteSockets, idx);
            __CFSocketClearFDForWrite(s);
        }
        idx = CFArrayGetFirstIndexOfValue(__CFReadSockets, CFRangeMake(0, CFArrayGetCount(__CFReadSockets)), s);
        if (0 <= idx) {
            CFArrayRemoveValueAtIndex(__CFReadSockets, idx);
            __CFSocketClearFDForRead(s);
        }
        __CFUnlock(&__CFActiveSocketsLock);
    }
    if (NULL != s->_runLoops) {
        CFMutableArrayRef runLoopsOrig = s->_runLoops;
        CFMutableArrayRef runLoopsCopy = CFArrayCreateMutableCopy(kCFAllocatorSystemDefault, 0, s->_runLoops);
        idx = CFArrayGetFirstIndexOfValue(runLoopsCopy, CFRangeMake(0, CFArrayGetCount(runLoopsCopy)), rl);
        if (0 <= idx) CFArrayRemoveValueAtIndex(runLoopsCopy, idx);
        s->_runLoops = runLoopsCopy;
        CFRelease(runLoopsOrig);
    }
    __CFSocketUnlock(s);
}

// Note:  must be called with socket lock held, then returns with it released
// Used by both the v0 and v1 RunLoopSource perform routines
static void __CFSocketDoCallback(CFSocketRef s, CFDataRef data, CFDataRef address, CFSocketNativeHandle sock) {
    CFSocketCallBack callout = NULL;
    void *contextInfo = NULL;
    SInt32 errorCode = 0;
    Boolean readSignalled = false, writeSignalled = false, connectSignalled = false, calledOut = false;
    uint8_t readCallBackType, callBackTypes;
    
    callBackTypes = __CFSocketCallBackTypes(s);
    readCallBackType = __CFSocketReadCallBackType(s);
    readSignalled = __CFSocketIsReadSignalled(s);
    writeSignalled = __CFSocketIsWriteSignalled(s);
    connectSignalled = writeSignalled && !s->_f.connected;
    __CFSocketUnsetReadSignalled(s);
    __CFSocketUnsetWriteSignalled(s);
    callout = s->_callout;
    contextInfo = s->_context.info;
#if defined(LOG_CFSOCKET)
    fprintf(stdout, "entering perform for socket %d with read signalled %d write signalled %d connect signalled %d callback types %d\n", s->_socket, readSignalled, writeSignalled, connectSignalled, callBackTypes);
#endif
    if (writeSignalled) {
        errorCode = s->_errorCode;
        s->_f.connected = TRUE;
    }
    __CFSocketUnlock(s);
    if ((callBackTypes & kCFSocketConnectCallBack) != 0) {
        if (connectSignalled && (!calledOut || CFSocketIsValid(s))) {
// CFLog(5, CFSTR("__CFSocketPerformV0(%p) doing connect callback, error: %d"), s, errorCode);
            if (errorCode) {
#if defined(LOG_CFSOCKET)
                fprintf(stdout, "perform calling out error %ld to socket %d\n", (long)errorCode, s->_socket);
#endif
                if (callout) callout(s, kCFSocketConnectCallBack, NULL, &errorCode, contextInfo);
                calledOut = true;
            } else {
#if defined(LOG_CFSOCKET)
                fprintf(stdout, "perform calling out connect to socket %d\n", s->_socket);
#endif
                if (callout) callout(s, kCFSocketConnectCallBack, NULL, NULL, contextInfo);
                calledOut = true;
            }
        }
    }
    if (kCFSocketDataCallBack == readCallBackType) {
        if (NULL != data && (!calledOut || CFSocketIsValid(s))) {
            SInt32 datalen = CFDataGetLength(data);
#if defined(LOG_CFSOCKET)
            fprintf(stdout, "perform calling out data of length %ld to socket %d\n", (long)datalen, s->_socket);
#endif
            if (callout) callout(s, kCFSocketDataCallBack, address, data, contextInfo);
            calledOut = true;
            if (0 == datalen) CFSocketInvalidate(s);
        }
    } else if (kCFSocketAcceptCallBack == readCallBackType) {
        if (INVALID_SOCKET != sock && (!calledOut || CFSocketIsValid(s))) {
#if defined(LOG_CFSOCKET)
            fprintf(stdout, "perform calling out accept of socket %d to socket %d\n", sock, s->_socket);
#endif
            if (callout) callout(s, kCFSocketAcceptCallBack, address, &sock, contextInfo);
            calledOut = true;
        }
    } else if (kCFSocketReadCallBack == readCallBackType) {
        if (readSignalled && (!calledOut || CFSocketIsValid(s))) {
#if defined(LOG_CFSOCKET)
            fprintf(stdout, "perform calling out read to socket %d\n", s->_socket);
#endif
// CFLog(5, CFSTR("__CFSocketPerformV0(%p) doing read callback"), s);
            if (callout) callout(s, kCFSocketReadCallBack, NULL, NULL, contextInfo);
            calledOut = true;
        }
    }
    if ((callBackTypes & kCFSocketWriteCallBack) != 0) {
        if (writeSignalled && !errorCode && (!calledOut || CFSocketIsValid(s))) {
#if defined(LOG_CFSOCKET)
            fprintf(stdout, "perform calling out write to socket %d\n", s->_socket);
#endif
// CFLog(5, CFSTR("__CFSocketPerformV0(%p) doing write callback"), s);
            if (callout) callout(s, kCFSocketWriteCallBack, NULL, NULL, contextInfo);
            calledOut = true;
        }
    }
}

static void __CFSocketPerformV0(void *info) {
    CFSocketRef s = (CFSocketRef)info;
    CFDataRef data = NULL;
    CFDataRef address = NULL;
    CFSocketNativeHandle sock = INVALID_SOCKET;
    uint8_t readCallBackType, callBackTypes;
    CFRunLoopRef rl = NULL;
// CFLog(5, CFSTR("__CFSocketPerformV0(%p) starting"), s);

    __CFSocketLock(s);
    if (!__CFSocketIsValid(s)) {
        __CFSocketUnlock(s);
        return;
    }
    callBackTypes = __CFSocketCallBackTypes(s);
    readCallBackType = __CFSocketReadCallBackType(s);
    CFOptionFlags callBacksSignalled = 0;
    if (__CFSocketIsReadSignalled(s)) callBacksSignalled |= readCallBackType;
    if (__CFSocketIsWriteSignalled(s)) callBacksSignalled |= kCFSocketWriteCallBack;

    if (kCFSocketDataCallBack == readCallBackType) {
        if (NULL != s->_dataQueue && 0 < CFArrayGetCount(s->_dataQueue)) {
            data = (CFDataRef)CFArrayGetValueAtIndex(s->_dataQueue, 0);
            CFRetain(data);
            CFArrayRemoveValueAtIndex(s->_dataQueue, 0);
            address = (CFDataRef)CFArrayGetValueAtIndex(s->_addressQueue, 0);
            CFRetain(address);
            CFArrayRemoveValueAtIndex(s->_addressQueue, 0);
        }
    } else if (kCFSocketAcceptCallBack == readCallBackType) {
        if (NULL != s->_dataQueue && 0 < CFArrayGetCount(s->_dataQueue)) {
            sock = (CFSocketNativeHandle)(uintptr_t)CFArrayGetValueAtIndex(s->_dataQueue, 0);
            CFArrayRemoveValueAtIndex(s->_dataQueue, 0);
            address = (CFDataRef)CFArrayGetValueAtIndex(s->_addressQueue, 0);
            CFRetain(address);
            CFArrayRemoveValueAtIndex(s->_addressQueue, 0);
        }
    }

    __CFSocketDoCallback(s, data, address, sock);	// does __CFSocketUnlock(s)
    if (NULL != data) CFRelease(data);
    if (NULL != address) CFRelease(address);

    __CFSocketLock(s);
    if (__CFSocketIsValid(s) && kCFSocketNoCallBack != readCallBackType) {
        // if there's still more data, we want to wake back up right away
        if ((kCFSocketDataCallBack == readCallBackType || kCFSocketAcceptCallBack == readCallBackType) && NULL != s->_dataQueue && 0 < CFArrayGetCount(s->_dataQueue)) {
#if defined(LOG_CFSOCKET)
            fprintf(stdout, "perform short-circuit signaling source for socket %d with flags 0x%x disabled 0x%x connected 0x%x\n", s->_socket, s->_f.client, s->_f.disabled, s->_f.connected);
#endif
            CFRunLoopSourceSignal(s->_source0);
            CFMutableArrayRef runLoopsOrig = (CFMutableArrayRef)CFRetain(s->_runLoops);
            CFMutableArrayRef runLoopsCopy = CFArrayCreateMutableCopy(kCFAllocatorSystemDefault, 0, s->_runLoops);
            CFRunLoopSourceRef source0 = s->_source0;
            if (NULL != source0 && !CFRunLoopSourceIsValid(source0)) {
                source0 = NULL;
            }
            if (source0) CFRetain(source0);
            __CFSocketUnlock(s);
            rl = __CFSocketCopyRunLoopToWakeUp(source0, runLoopsCopy);
            if (source0) CFRelease(source0);
            __CFSocketLock(s);
            if (runLoopsOrig == s->_runLoops) {
                s->_runLoops = runLoopsCopy;
                runLoopsCopy = NULL;
                CFRelease(runLoopsOrig);
            }
            CFRelease(runLoopsOrig);
            if (runLoopsCopy) CFRelease(runLoopsCopy);
        }
    }
    // Only reenable callbacks that are auto-reenabled
    __CFSocketEnableCallBacks(s, callBacksSignalled & s->_f.client, FALSE, 'p');  // unlocks s

    if (NULL != rl) {
        CFRunLoopWakeUp(rl);
        CFRelease(rl);
    }
// CFLog(5, CFSTR("__CFSocketPerformV0(%p) done"), s);
}

CFRunLoopSourceRef CFSocketCreateRunLoopSource(CFAllocatorRef allocator, CFSocketRef s, CFIndex order) {
    CHECK_FOR_FORK();
    CFRunLoopSourceRef result = NULL;
    __CFGenericValidateType(s, CFSocketGetTypeID());
    __CFSocketLock(s);
    if (__CFSocketIsValid(s)) {
        if (NULL != s->_source0 && !CFRunLoopSourceIsValid(s->_source0)) {
            CFRelease(s->_source0);
            s->_source0 = NULL;
        }
        if (NULL == s->_source0) {
            CFRunLoopSourceContext context;
            context.version = 0;
            context.info = s;
            context.retain = CFRetain;
            context.release = CFRelease;
            context.copyDescription = CFCopyDescription;
            context.equal = CFEqual;
            context.hash = CFHash;
            context.schedule = __CFSocketSchedule;
            context.cancel = __CFSocketCancel;
            context.perform = __CFSocketPerformV0;
            s->_source0 = CFRunLoopSourceCreate(allocator, order, &context);
        }
        CFRetain(s->_source0);        /* This retain is for the receiver */
        result = s->_source0;
    }
    __CFSocketUnlock(s);
    return result;
}

#endif /* NEW_SOCKET */



static uint16_t __CFSocketDefaultNameRegistryPortNumber = 2454;

CONST_STRING_DECL(kCFSocketCommandKey, "Command")
CONST_STRING_DECL(kCFSocketNameKey, "Name")
CONST_STRING_DECL(kCFSocketValueKey, "Value")
CONST_STRING_DECL(kCFSocketResultKey, "Result")
CONST_STRING_DECL(kCFSocketErrorKey, "Error")
CONST_STRING_DECL(kCFSocketRegisterCommand, "Register")
CONST_STRING_DECL(kCFSocketRetrieveCommand, "Retrieve")
CONST_STRING_DECL(__kCFSocketRegistryRequestRunLoopMode, "CFSocketRegistryRequest")

static CFLock_t __CFSocketWriteLock_ = CFLockInit;
//#warning can only send on one socket at a time now

CF_INLINE void __CFSocketWriteLock(CFSocketRef s) {
    __CFLock(& __CFSocketWriteLock_);
}

CF_INLINE void __CFSocketWriteUnlock(CFSocketRef s) {
    __CFUnlock(& __CFSocketWriteLock_);
}

#if NEW_SOCKET

CF_INLINE CFIndex __CFSocketFdGetSize(CFDataRef fdSet) {
    return NBBY * CFDataGetLength(fdSet);
}

CF_INLINE Boolean __CFSocketFdSet(CFSocketNativeHandle sock, CFMutableDataRef fdSet) {
    /* returns true if a change occurred, false otherwise */
    Boolean retval = false;
    if (INVALID_SOCKET != sock && 0 <= sock) {
        CFIndex numFds = NBBY * CFDataGetLength(fdSet);
        fd_mask *fds_bits;
        if (sock >= numFds) {
            CFIndex oldSize = numFds / NFDBITS, newSize = (sock + NFDBITS) / NFDBITS, changeInBytes = (newSize - oldSize) * sizeof(fd_mask);
            CFDataIncreaseLength(fdSet, changeInBytes);
            fds_bits = (fd_mask *)CFDataGetMutableBytePtr(fdSet);
            memset(fds_bits + oldSize, 0, changeInBytes);
        } else {
            fds_bits = (fd_mask *)CFDataGetMutableBytePtr(fdSet);
        }
        if (!FD_ISSET(sock, (fd_set *)fds_bits)) {
            retval = true;
            FD_SET(sock, (fd_set *)fds_bits);
        }
    }
    return retval;
}

#endif

//??? need timeout, error handling, retries
CFSocketError CFSocketSendData(CFSocketRef s, CFDataRef address, CFDataRef data, CFTimeInterval timeout) {
    CHECK_FOR_FORK();
    const uint8_t *dataptr, *addrptr = NULL;
    SInt32 datalen, addrlen = 0, size = 0;
    CFSocketNativeHandle sock = INVALID_SOCKET;
    struct timeval tv;
    __CFGenericValidateType(s, CFSocketGetTypeID());
    if (address) {
        addrptr = CFDataGetBytePtr(address);
        addrlen = CFDataGetLength(address);
    }
    dataptr = CFDataGetBytePtr(data);
    datalen = CFDataGetLength(data);
    if (CFSocketIsValid(s)) sock = CFSocketGetNative(s);
    if (INVALID_SOCKET != sock) {
        CFRetain(s);
        __CFSocketWriteLock(s);
        tv.tv_sec = (timeout <= 0.0 || (CFTimeInterval)INT_MAX <= timeout) ? INT_MAX : (int)floor(timeout);
        tv.tv_usec = (int)floor(1.0e+6 * (timeout - floor(timeout)));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv));	// cast for WinSock bad API
        if (NULL != addrptr && 0 < addrlen) {
            size = sendto(sock, (char *)dataptr, datalen, 0, (struct sockaddr *)addrptr, addrlen);
        } else {
            size = send(sock, (char *)dataptr, datalen, 0);
        }
#if defined(LOG_CFSOCKET)
        fprintf(stdout, "wrote %ld bytes to socket %d\n", (long)size, sock);
#endif
        __CFSocketWriteUnlock(s);
        CFRelease(s);
    }
    return (size > 0) ? kCFSocketSuccess : kCFSocketError;
}

CFSocketError CFSocketSetAddress(CFSocketRef s, CFDataRef address) {
    CHECK_FOR_FORK();
    struct sockaddr *name;
    socklen_t namelen;
    __CFGenericValidateType(s, CFSocketGetTypeID());
    if (NULL == address) return kCFSocketError;
    if (!CFSocketIsValid(s)) return kCFSocketError;
    
    name = (struct sockaddr *)CFDataGetBytePtr(address);
    namelen = (socklen_t)CFDataGetLength(address);
    if (!name || namelen <= 0) return kCFSocketError;
    
    CFSocketNativeHandle sock = CFSocketGetNative(s);
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    // Verify that the namelen is correct. If not, we have to fix it up. Developers will often incorrectly use 0 or strlen(path). See 9217961 and the second half of 9098274.
    // Max size is a size byte, plus family byte, plus path of 255, plus a null byte.
    char newName[255];
    if (namelen > 2 && name->sa_family == AF_UNIX) {
        // Don't use the SUN_LEN macro, because strnlen is safer and we know the max length of the string (from CFData, minus 2 bytes for len and addr)
        socklen_t realLength = (sizeof(*((struct sockaddr_un *)name)) - sizeof(((struct sockaddr_un *)name)->sun_path) + strnlen(((struct sockaddr_un *)name)->sun_path, namelen - 2));
        if (realLength > 255) return kCFSocketError;
        
        // For a UNIX domain socket, we must pass the value of name.sun_len to bind in order for getsockname() to return a result that makes sense.
        namelen = (socklen_t)(((struct sockaddr_un *)name)->sun_len);
        
        if (realLength != namelen) {
            // We got a different answer for length than was supplied by the caller. Fix it up so we don't end up truncating the path.
            CFLog(kCFLogLevelWarning, CFSTR("WARNING: The sun_len field of a sockaddr_un structure passed to CFSocketSetAddress was not set correctly using the SUN_LEN macro."));
            memcpy(newName, name, realLength);
            namelen = realLength;
            ((struct sockaddr_un *)newName)->sun_len = realLength;
            name = (struct sockaddr *)newName;
        }
    }
#endif
    int result = bind(sock, name, namelen);
    if (0 == result) {
        listen(sock, 256);
    }
    //??? should return errno
    return (CFIndex)result;
}

CFSocketError CFSocketConnectToAddress(CFSocketRef s, CFDataRef address, CFTimeInterval timeout) {
    CHECK_FOR_FORK();
    //??? need error handling, retries
    const uint8_t *name;
    SInt32 namelen, result = -1, connect_err = 0, select_err = 0;
    UInt32 yes = 1, no = 0;
    Boolean wasBlocking = true;

    __CFGenericValidateType(s, CFSocketGetTypeID());
    if (!CFSocketIsValid(s)) return kCFSocketError;
    name = CFDataGetBytePtr(address);
    namelen = CFDataGetLength(address);
    if (!name || namelen <= 0) return kCFSocketError;
    CFSocketNativeHandle sock = CFSocketGetNative(s);
    {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
        SInt32 flags = fcntl(sock, F_GETFL, 0);
        if (flags >= 0) wasBlocking = ((flags & O_NONBLOCK) == 0);
        if (wasBlocking && (timeout > 0.0 || timeout < 0.0)) ioctlsocket(sock, FIONBIO, (u_long *)&yes);
#else
        // You can set but not get this flag in WIN32, so assume it was in non-blocking mode.
        // The downside is that when we leave this routine we'll leave it non-blocking,
        // whether it started that way or not.
        SInt32 flags = 0;
        if (timeout > 0.0 || timeout < 0.0) ioctlsocket(sock, FIONBIO, (u_long *)&yes);
        wasBlocking = false;
#endif
        result = connect(sock, (struct sockaddr *)name, namelen);
        if (result != 0) {
            connect_err = __CFSocketLastError();
#if DEPLOYMENT_TARGET_WINDOWS
            if (connect_err == WSAEWOULDBLOCK) connect_err = EINPROGRESS;
#endif
        }
#if defined(LOG_CFSOCKET)
        fprintf(stdout, "connection attempt returns %d error %d on socket %d (flags 0x%x blocking %d)\n", (int) result, (int) connect_err, sock, (int) flags, wasBlocking);
#endif
        if (EINPROGRESS == connect_err && timeout >= 0.0) {
            /* select on socket */
            SInt32 nrfds;
            int error_size = sizeof(select_err);
            struct timeval tv;
            CFMutableDataRef fds = CFDataCreateMutable(kCFAllocatorSystemDefault, 0);
            __CFSocketFdSet(sock, fds);
            tv.tv_sec = (timeout <= 0.0 || (CFTimeInterval)INT_MAX <= timeout) ? INT_MAX : (int)floor(timeout);
            tv.tv_usec = (int)floor(1.0e+6 * (timeout - floor(timeout)));
            nrfds = select(__CFSocketFdGetSize(fds), NULL, (fd_set *)CFDataGetMutableBytePtr(fds), NULL, &tv);
            if (nrfds < 0) {
                select_err = __CFSocketLastError();
                result = -1;
            } else if (nrfds == 0) {
                result = -2;
            } else {
                if (0 != getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&select_err, (socklen_t *)&error_size)) select_err = 0;
                result = (select_err == 0) ? 0 : -1;
            }
            CFRelease(fds);
#if defined(LOG_CFSOCKET)
            fprintf(stdout, "timed connection attempt %s on socket %d, result %d, select returns %d error %d\n", (result == 0) ? "succeeds" : "fails", sock, (int) result, (int) nrfds, (int) select_err);
#endif
        }
        if (wasBlocking && (timeout > 0.0 || timeout < 0.0)) ioctlsocket(sock, FIONBIO, (u_long *)&no);
        if (EINPROGRESS == connect_err && timeout < 0.0) {
            result = 0;
#if defined(LOG_CFSOCKET)
            fprintf(stdout, "connection attempt continues in background on socket %d\n", sock);
#endif
        }
    }
    //??? should return errno
    return result;
}

CFSocketRef CFSocketCreate(CFAllocatorRef allocator, SInt32 protocolFamily, SInt32 socketType, SInt32 protocol, CFOptionFlags callBackTypes, CFSocketCallBack callout, const CFSocketContext *context) {
    CHECK_FOR_FORK();
    CFSocketNativeHandle sock = INVALID_SOCKET;
    CFSocketRef s = NULL;
    if (0 >= protocolFamily) protocolFamily = PF_INET;
    if (PF_INET == protocolFamily) {
        if (0 >= socketType) socketType = SOCK_STREAM;
        if (0 >= protocol && SOCK_STREAM == socketType) protocol = IPPROTO_TCP;
        if (0 >= protocol && SOCK_DGRAM == socketType) protocol = IPPROTO_UDP;
    }
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    if (PF_LOCAL == protocolFamily && 0 >= socketType) socketType = SOCK_STREAM;
#endif
#if DEPLOYMENT_TARGET_WINDOWS
    // make sure we've called proper Win32 startup facilities before socket()
    __CFSocketInitializeWinSock();
#endif
    sock = socket(protocolFamily, socketType, protocol);
    if (INVALID_SOCKET != sock) {
        s = CFSocketCreateWithNative(allocator, sock, callBackTypes, callout, context);
    }
    return s;
}

CFSocketRef CFSocketCreateWithSocketSignature(CFAllocatorRef allocator, const CFSocketSignature *signature, CFOptionFlags callBackTypes, CFSocketCallBack callout, const CFSocketContext *context) {
    CHECK_FOR_FORK();
    CFSocketRef s = CFSocketCreate(allocator, signature->protocolFamily, signature->socketType, signature->protocol, callBackTypes, callout, context);
    if (NULL != s && (!CFSocketIsValid(s) || kCFSocketSuccess != CFSocketSetAddress(s, signature->address))) {
        CFSocketInvalidate(s);
        CFRelease(s);
        s = NULL;
    }
    return s;
}

CFSocketRef CFSocketCreateConnectedToSocketSignature(CFAllocatorRef allocator, const CFSocketSignature *signature, CFOptionFlags callBackTypes, CFSocketCallBack callout, const CFSocketContext *context, CFTimeInterval timeout) {
    CHECK_FOR_FORK();
    CFSocketRef s = CFSocketCreate(allocator, signature->protocolFamily, signature->socketType, signature->protocol, callBackTypes, callout, context);
    if (NULL != s && (!CFSocketIsValid(s) || kCFSocketSuccess != CFSocketConnectToAddress(s, signature->address, timeout))) {
        CFSocketInvalidate(s);
        CFRelease(s);
        s = NULL;
    }
    return s;
}

typedef struct {
    CFSocketError *error;
    CFPropertyListRef *value;
    CFDataRef *address;
} __CFSocketNameRegistryResponse;

static void __CFSocketHandleNameRegistryReply(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void *data, void *info) {
    CFDataRef replyData = (CFDataRef)data;
    __CFSocketNameRegistryResponse *response = (__CFSocketNameRegistryResponse *)info;
    CFDictionaryRef replyDictionary = NULL;
    CFPropertyListRef value;
    replyDictionary = (CFDictionaryRef)CFPropertyListCreateWithData(kCFAllocatorSystemDefault, replyData, kCFPropertyListImmutable, NULL, NULL);
    if (NULL != response->error) *(response->error) = kCFSocketError;
    if (NULL != replyDictionary) {
        if (CFGetTypeID((CFTypeRef)replyDictionary) == CFDictionaryGetTypeID() && NULL != (value = CFDictionaryGetValue(replyDictionary, kCFSocketResultKey))) {
            if (NULL != response->error) *(response->error) = kCFSocketSuccess;
            if (NULL != response->value) *(response->value) = CFRetain(value);
            if (NULL != response->address) *(response->address) = address ? CFDataCreateCopy(kCFAllocatorSystemDefault, address) : NULL;
        }
        CFRelease(replyDictionary);
    }
    CFSocketInvalidate(s);
}

static void __CFSocketSendNameRegistryRequest(CFSocketSignature *signature, CFDictionaryRef requestDictionary, __CFSocketNameRegistryResponse *response, CFTimeInterval timeout) {
    CFDataRef requestData = NULL;
    CFSocketContext context = {0, response, NULL, NULL, NULL};
    CFSocketRef s = NULL;
    CFRunLoopSourceRef source = NULL;
    if (NULL != response->error) *(response->error) = kCFSocketError;
    requestData = CFPropertyListCreateData(kCFAllocatorSystemDefault, requestDictionary, kCFPropertyListXMLFormat_v1_0, 0, NULL);
    if (NULL != requestData) {
        if (NULL != response->error) *(response->error) = kCFSocketTimeout;
        s = CFSocketCreateConnectedToSocketSignature(kCFAllocatorSystemDefault, signature, kCFSocketDataCallBack, __CFSocketHandleNameRegistryReply, &context, timeout);
        if (NULL != s) {
            if (kCFSocketSuccess == CFSocketSendData(s, NULL, requestData, timeout)) {
                source = CFSocketCreateRunLoopSource(kCFAllocatorSystemDefault, s, 0);
                CFRunLoopAddSource(CFRunLoopGetCurrent(), source, __kCFSocketRegistryRequestRunLoopMode);
                CFRunLoopRunInMode(__kCFSocketRegistryRequestRunLoopMode, timeout, false);
                CFRelease(source);
            }
            CFSocketInvalidate(s);
            CFRelease(s);
        }
        CFRelease(requestData);
    }
}

static void __CFSocketValidateSignature(const CFSocketSignature *providedSignature, CFSocketSignature *signature, uint16_t defaultPortNumber) {
    struct sockaddr_in sain, *sainp;
    memset(&sain, 0, sizeof(sain));
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    sain.sin_len = sizeof(sain);
#endif
    sain.sin_family = AF_INET;
    sain.sin_port = htons(__CFSocketDefaultNameRegistryPortNumber);
    sain.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (NULL == providedSignature) {
        signature->protocolFamily = PF_INET;
        signature->socketType = SOCK_STREAM;
        signature->protocol = IPPROTO_TCP;
        signature->address = CFDataCreate(kCFAllocatorSystemDefault, (uint8_t *)&sain, sizeof(sain));
    } else {
        signature->protocolFamily = providedSignature->protocolFamily;
        signature->socketType = providedSignature->socketType;
        signature->protocol = providedSignature->protocol;
        if (0 >= signature->protocolFamily) signature->protocolFamily = PF_INET;
        if (PF_INET == signature->protocolFamily) {
            if (0 >= signature->socketType) signature->socketType = SOCK_STREAM;
            if (0 >= signature->protocol && SOCK_STREAM == signature->socketType) signature->protocol = IPPROTO_TCP;
            if (0 >= signature->protocol && SOCK_DGRAM == signature->socketType) signature->protocol = IPPROTO_UDP;
        }
        if (NULL == providedSignature->address) {
            signature->address = CFDataCreate(kCFAllocatorSystemDefault, (uint8_t *)&sain, sizeof(sain));
        } else {
            sainp = (struct sockaddr_in *)CFDataGetBytePtr(providedSignature->address);
            if ((int)sizeof(struct sockaddr_in) <= CFDataGetLength(providedSignature->address) && (AF_INET == sainp->sin_family || 0 == sainp->sin_family)) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
                sain.sin_len = sizeof(sain);
#endif
                sain.sin_family = AF_INET;
                sain.sin_port = sainp->sin_port;
                if (0 == sain.sin_port) sain.sin_port = htons(defaultPortNumber);
                sain.sin_addr.s_addr = sainp->sin_addr.s_addr;
                if (htonl(INADDR_ANY) == sain.sin_addr.s_addr) sain.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
                signature->address = CFDataCreate(kCFAllocatorSystemDefault, (uint8_t *)&sain, sizeof(sain));
            } else {
                signature->address = (CFDataRef)CFRetain(providedSignature->address);
            }
        }
    }
}

CFSocketError CFSocketRegisterValue(const CFSocketSignature *nameServerSignature, CFTimeInterval timeout, CFStringRef name, CFPropertyListRef value) {
    CFSocketSignature signature;
    CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 3, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFSocketError retval = kCFSocketError;
    __CFSocketNameRegistryResponse response = {&retval, NULL, NULL};
    CFDictionaryAddValue(dictionary, kCFSocketCommandKey, kCFSocketRegisterCommand);
    CFDictionaryAddValue(dictionary, kCFSocketNameKey, name);
    if (NULL != value) CFDictionaryAddValue(dictionary, kCFSocketValueKey, value);
    __CFSocketValidateSignature(nameServerSignature, &signature, __CFSocketDefaultNameRegistryPortNumber);
    __CFSocketSendNameRegistryRequest(&signature, dictionary, &response, timeout);
    CFRelease(dictionary);
    CFRelease(signature.address);
    return retval;
}

CFSocketError CFSocketCopyRegisteredValue(const CFSocketSignature *nameServerSignature, CFTimeInterval timeout, CFStringRef name, CFPropertyListRef *value, CFDataRef *serverAddress) {
    CFSocketSignature signature;
    CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFSocketError retval = kCFSocketError;
    __CFSocketNameRegistryResponse response = {&retval, value, serverAddress};
    CFDictionaryAddValue(dictionary, kCFSocketCommandKey, kCFSocketRetrieveCommand);
    CFDictionaryAddValue(dictionary, kCFSocketNameKey, name);
    __CFSocketValidateSignature(nameServerSignature, &signature, __CFSocketDefaultNameRegistryPortNumber);
    __CFSocketSendNameRegistryRequest(&signature, dictionary, &response, timeout);
    CFRelease(dictionary);
    CFRelease(signature.address);
    return retval;
}

CFSocketError CFSocketRegisterSocketSignature(const CFSocketSignature *nameServerSignature, CFTimeInterval timeout, CFStringRef name, const CFSocketSignature *signature) {
    CFSocketSignature validatedSignature;
    CFMutableDataRef data = NULL;
    CFSocketError retval;
    CFIndex length;
    uint8_t bytes[4];
    if (NULL == signature) {
        retval = CFSocketUnregister(nameServerSignature, timeout, name);
    } else {
        __CFSocketValidateSignature(signature, &validatedSignature, 0);
        if (NULL == validatedSignature.address || 0 > validatedSignature.protocolFamily || 255 < validatedSignature.protocolFamily || 0 > validatedSignature.socketType || 255 < validatedSignature.socketType || 0 > validatedSignature.protocol || 255 < validatedSignature.protocol || 0 >= (length = CFDataGetLength(validatedSignature.address)) || 255 < length) {
            retval = kCFSocketError;
        } else {
            data = CFDataCreateMutable(kCFAllocatorSystemDefault, sizeof(bytes) + length);
            bytes[0] = validatedSignature.protocolFamily;
            bytes[1] = validatedSignature.socketType;
            bytes[2] = validatedSignature.protocol;
            bytes[3] = length;
            CFDataAppendBytes(data, bytes, sizeof(bytes));
            CFDataAppendBytes(data, CFDataGetBytePtr(validatedSignature.address), length);
            retval = CFSocketRegisterValue(nameServerSignature, timeout, name, data);
            CFRelease(data);
        }
        CFRelease(validatedSignature.address);
    }
    return retval;
}

CFSocketError CFSocketCopyRegisteredSocketSignature(const CFSocketSignature *nameServerSignature, CFTimeInterval timeout, CFStringRef name, CFSocketSignature *signature, CFDataRef *nameServerAddress) {
    CFDataRef data = NULL;
    CFSocketSignature returnedSignature;
    const uint8_t *ptr = NULL, *aptr = NULL;
    uint8_t *mptr;
    CFIndex length = 0;
    CFDataRef serverAddress = NULL;
    CFSocketError retval = CFSocketCopyRegisteredValue(nameServerSignature, timeout, name, (CFPropertyListRef *)&data, &serverAddress);
    if (NULL == data || CFGetTypeID(data) != CFDataGetTypeID() || NULL == (ptr = CFDataGetBytePtr(data)) || (length = CFDataGetLength(data)) < 4) retval = kCFSocketError;
    if (kCFSocketSuccess == retval && NULL != signature) {
        returnedSignature.protocolFamily = (SInt32)*ptr++;
        returnedSignature.socketType = (SInt32)*ptr++;
        returnedSignature.protocol = (SInt32)*ptr++;
        ptr++;
        returnedSignature.address = CFDataCreate(kCFAllocatorSystemDefault, ptr, length - 4);
        __CFSocketValidateSignature(&returnedSignature, signature, 0);
        CFRelease(returnedSignature.address);
        ptr = CFDataGetBytePtr(signature->address);
        if (CFDataGetLength(signature->address) >= (int)sizeof(struct sockaddr_in) && AF_INET == ((struct sockaddr *)ptr)->sa_family && NULL != serverAddress && CFDataGetLength(serverAddress) >= (int)sizeof(struct sockaddr_in) && NULL != (aptr = CFDataGetBytePtr(serverAddress)) && AF_INET == ((struct sockaddr *)aptr)->sa_family) {
            CFMutableDataRef address = CFDataCreateMutableCopy(kCFAllocatorSystemDefault, CFDataGetLength(signature->address), signature->address);
            mptr = CFDataGetMutableBytePtr(address);
            ((struct sockaddr_in *)mptr)->sin_addr = ((struct sockaddr_in *)aptr)->sin_addr;
            CFRelease(signature->address);
            signature->address = address;
        }
        if (NULL != nameServerAddress) *nameServerAddress = serverAddress ? (CFDataRef)CFRetain(serverAddress) : NULL;
    }
    if (NULL != data) CFRelease(data);
    if (NULL != serverAddress) CFRelease(serverAddress);
    return retval;
}

CFSocketError CFSocketUnregister(const CFSocketSignature *nameServerSignature, CFTimeInterval timeout, CFStringRef name) {
    return CFSocketRegisterValue(nameServerSignature, timeout, name, NULL);
}

CF_EXPORT void CFSocketSetDefaultNameRegistryPortNumber(uint16_t port) {
    __CFSocketDefaultNameRegistryPortNumber = port;
}

CF_EXPORT uint16_t CFSocketGetDefaultNameRegistryPortNumber(void) {
    return __CFSocketDefaultNameRegistryPortNumber;
}

