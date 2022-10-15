/*	CFSocket.c
	Copyright (c) 1999-2019, Apple Inc.  and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
	Responsibility: Michael LeHew
*/

#include <CoreFoundation/CFSocket.h>
#include <sys/types.h>
#include <math.h>
#include <limits.h>
#if TARGET_OS_MAC
#include <sys/sysctl.h>
#include <sys/un.h>
#include <libc.h>
#include <dlfcn.h>
#if TARGET_OS_CYGWIN
#include <sys/socket.h>
#endif
#endif
#if TARGET_OS_CYGWIN || TARGET_OS_BSD
#include <sys/socket.h>
#endif
#if TARGET_OS_WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#endif
#if !TARGET_OS_WIN32
#include <sys/ioctl.h>
#endif
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#endif
#include <fcntl.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFPropertyList.h>
#include "CFInternal.h"
#include "CFRuntime_Internal.h"
#if TARGET_OS_WIN32
#include <process.h>
#endif

#ifndef NBBY
#define NBBY 8
#endif

#if TARGET_OS_WIN32

// redefine this to the winsock error in this file
#undef EINPROGRESS
#define EINPROGRESS WSAEINPROGRESS

// redefine this to the winsock error in this file
#undef EBADF
#define EBADF WSAENOTSOCK

#define NFDBITS	(sizeof(int32_t) * NBBY)

typedef int32_t fd_mask;
typedef int socklen_t;

#define gettimeofday _NS_gettimeofday
struct timezone;
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

static void timeradd(struct timeval *a, struct timeval *b, struct timeval *res) {
  res->tv_sec = a->tv_sec + b->tv_sec;
  res->tv_usec = a->tv_usec + b->tv_usec;
  if (res->tv_usec > 1e06) {
    res->tv_sec++;
    res->tv_usec -= 1e06;
  }
}

#endif // TARGET_OS_WIN32


// On Mach we use a v0 RunLoopSource to make client callbacks.  That source is signalled by a
// separate SocketManager thread who uses select() to watch the sockets' fds.

#undef LOG_CFSOCKET
//#define LOG_CFSOCKET            1
#define DEBUG_POLLING_SELECT    1

#if defined(LOG_CFSOCKET)

#include <sys/syslog.h>

static _CFThreadRef __cfSocketTid()
{
#if TARGET_OS_MAC
    uint64_t tid = 0;
    if (0 != pthread_threadid_np(NULL, &tid))
        tid = pthread_mach_thread_np(pthread_self());
    return (_CFThreadRef) tid;
#elif TARGET_OS_WIN32
    return (_CFThreadRef) GetCurrentThreadId();
#else
    return (_CFThreadRef) pthread_self();
#endif
}

static void __cfSocketLog(const char* function, int line, const char* fmt, ...)
{
#if 0
    char* p = nil;
    va_list args;
    va_start(args, fmt);
    vasprintf(&p, fmt, args);
    va_end(args);

//    CFLog(kCFLogLevelNotice, CFSTR("CFSocket:%d %s"), line, p);
    char* s = nil;
    asprintf(&s, "CFSocket:%d %s", line, p);
    syslog(LOG_NOTICE, "%s", s);
    free(s);
    
    free(p);
#else
    va_list args;
    va_start(args, fmt);
    CFStringRef fmtString = CFStringCreateWithCString(kCFAllocatorDefault, fmt, kCFStringEncodingUTF8);
    CFStringRef payload = CFStringCreateWithFormatAndArguments(kCFAllocatorDefault, NULL, fmtString, args);
    if (fmtString)
        CFRelease(fmtString);

    if (payload == NULL)
        syslog(LOG_NOTICE, "CFSocket[%p]:%s:%d - no payload?", __cfSocketTid(),function, line);
    else {
        CFDataRef payloadData = CFStringCreateExternalRepresentation(kCFAllocatorDefault, payload, kCFStringEncodingUTF8, '.');
        CFRelease(payload);

        if (payloadData == NULL)
            syslog(LOG_NOTICE, "CFSocket[%p]:%s:%d - no payload?", __cfSocketTid(),function, line);
        else {
            syslog(LOG_NOTICE, "CFSocket[%p]:%s:%d - %.*s", __cfSocketTid(),function, line, (int) CFDataGetLength(payloadData), CFDataGetBytePtr(payloadData));
            CFRelease(payloadData);
        }
    }

#endif
}

static void __cfSocketLogWithSocket(CFSocketRef s, const char* function, int line, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    CFStringRef fmtString = CFStringCreateWithCString(kCFAllocatorDefault, fmt, kCFStringEncodingUTF8);
    CFStringRef payload = CFStringCreateWithFormatAndArguments(kCFAllocatorDefault, NULL, fmtString, args);
    if (fmtString)
        CFRelease(fmtString);

    if (payload == NULL)
        syslog(LOG_NOTICE, "CFSocket[%p]:%s:%d (%p, fd %d) - no payload?", __cfSocketTid(), function, line, s, CFSocketGetNative(s));
    else {
        CFDataRef payloadData = CFStringCreateExternalRepresentation(kCFAllocatorDefault, payload, kCFStringEncodingUTF8, '.');
        CFRelease(payload);

        if (payloadData == NULL)
            syslog(LOG_NOTICE, "CFSocket[%p]:%s:%d (%p, fd %d) - no payload?", __cfSocketTid(), function, line, s, CFSocketGetNative(s));
        else {
            syslog(LOG_NOTICE, "CFSocket[%p]:%s:%d (%p, fd %d) - %.*s", __cfSocketTid(), function, line, s, CFSocketGetNative(s), (int) CFDataGetLength(payloadData), CFDataGetBytePtr(payloadData));
            CFRelease(payloadData);
        }
    }
}

#define __CFSOCKETLOG(xxx...)  __cfSocketLog(__FUNCTION__, __LINE__, xxx)
#define __CFSOCKETLOG_WS(S, xxx...)  __cfSocketLogWithSocket(S, __FUNCTION__, __LINE__, xxx)

#else

#define __CFSOCKETLOG(xxx...)  /**/
#define __CFSOCKETLOG_WS(S, xxx...) /**/

#endif


#if TARGET_OS_MAC || TARGET_OS_LINUX || TARGET_OS_BSD
#define INVALID_SOCKET (CFSocketNativeHandle)(-1)
#define closesocket(a) close((a))
#define ioctlsocket(a,b,c) ioctl((a),(b),(c))
#endif

CF_INLINE int __CFSocketLastError(void) {
#if TARGET_OS_WIN32
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
    return __CFRuntimeGetFlag(s, 6);
}

CF_INLINE void __CFSocketSetWriteSignalled(CFSocketRef s) {
    __CFRuntimeSetFlag(s, 6, true);
}

CF_INLINE void __CFSocketUnsetWriteSignalled(CFSocketRef s) {
    __CFRuntimeSetFlag(s, 6, false);
}

CF_INLINE Boolean __CFSocketIsReadSignalled(CFSocketRef s) {
    return __CFRuntimeGetFlag(s, 5);
}

CF_INLINE void __CFSocketSetReadSignalled(CFSocketRef s) {
    __CFRuntimeSetFlag(s, 5, true);
}

CF_INLINE void __CFSocketUnsetReadSignalled(CFSocketRef s) {
    __CFRuntimeSetFlag(s, 5, false);
}

CF_INLINE Boolean __CFSocketIsValid(CFSocketRef s) {
    return __CFRuntimeGetFlag(s, 4);
}

CF_INLINE void __CFSocketSetValid(CFSocketRef s) {
    __CFRuntimeSetFlag(s, 4, true);
}

CF_INLINE void __CFSocketUnsetValid(CFSocketRef s) {
    __CFRuntimeSetFlag(s, 4, false);
}

CF_INLINE uint8_t __CFSocketCallBackTypes(CFSocketRef s) {
    return (uint8_t)__CFRuntimeGetValue(s, 3, 0);
}

CF_INLINE uint8_t __CFSocketReadCallBackType(CFSocketRef s) {
    return (uint8_t)__CFRuntimeGetValue(s, 1, 0);
}

CF_INLINE void __CFSocketSetCallBackTypes(CFSocketRef s, uint8_t types) {
    __CFRuntimeSetValue(s, 3, 0, types & 0xF);
}

CF_INLINE void __CFSocketLock(CFSocketRef s) {
    __CFLock(&(s->_lock));
}

CF_INLINE void __CFSocketUnlock(CFSocketRef s) {
    __CFUnlock(&(s->_lock));
}

CF_INLINE Boolean __CFSocketIsConnectionOriented(CFSocketRef s) {
    return (SOCK_STREAM == s->_socketType);
}

CF_INLINE Boolean __CFSocketIsScheduled(CFSocketRef s) {
    return (s->_socketSetCount > 0);
}

CF_INLINE void __CFSocketEstablishAddress(CFSocketRef s) {
    /* socket should already be locked */
    uint8_t name[MAX_SOCKADDR_LEN];
    int namelen = sizeof(name);
    if (__CFSocketIsValid(s) && NULL == s->_address && INVALID_SOCKET != s->_socket && 0 == getsockname(s->_socket, (struct sockaddr *)name, (socklen_t *)&namelen) && 0 < namelen) {
        s->_address = CFDataCreate(CFGetAllocator(s), name, namelen);
    }
}

CF_INLINE void __CFSocketEstablishPeerAddress(CFSocketRef s) {
    /* socket should already be locked */
    uint8_t name[MAX_SOCKADDR_LEN];
    int namelen = sizeof(name);
    if (__CFSocketIsValid(s) && NULL == s->_peerAddress && INVALID_SOCKET != s->_socket && 0 == getpeername(s->_socket, (struct sockaddr *)name, (socklen_t *)&namelen) && 0 < namelen) {
        s->_peerAddress = CFDataCreate(CFGetAllocator(s), name, namelen);
    }
}

static Boolean __CFNativeSocketIsValid(CFSocketNativeHandle sock) {
    Boolean result;

#if TARGET_OS_WIN32
    SInt32 errorCode = 0;
    int errorSize = sizeof(errorCode);
    result = !(0 != getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&errorCode, &errorSize) && __CFSocketLastError() == WSAENOTSOCK);
#else    
    SInt32 flags = fcntl(sock, F_GETFL, 0);
    result = !(0 > flags && EBADF == __CFSocketLastError());
#endif

    __CFSOCKETLOG("socket fd %d => %d", sock, result);

    return result;
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
#if TARGET_OS_MAC
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
        if (0 <= error) error = getsockname(__CFWakeupSocketPair[i], (struct sockaddr *)&(address[i]), (socklen_t *)&namelen);
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

    __CFSOCKETLOG("wakeup socket pair is %d / %d\n", __CFWakeupSocketPair[0], __CFWakeupSocketPair[1]);

    return error;
}


// Version 0 RunLoopSources set a mask in an FD set to control what socket activity we hear about.
// Changes to the master fs_sets occur via these 4 functions.
CF_INLINE Boolean __CFSocketSetFDForRead(CFSocketRef s) {
    __CFSOCKETLOG_WS(s, "");
    __CFReadSocketsTimeoutInvalid = true;
    Boolean b = __CFSocketFdSet(s->_socket, __CFReadSocketsFds);
    if (b && INVALID_SOCKET != __CFWakeupSocketPair[0]) {
        uint8_t c = 'r';
        send(__CFWakeupSocketPair[0], (const char *)&c, sizeof(c), 0);
    }
    return b;
}

CF_INLINE Boolean __CFSocketClearFDForRead(CFSocketRef s) {
    __CFSOCKETLOG_WS(s, "");
    __CFReadSocketsTimeoutInvalid = true;
    Boolean b = __CFSocketFdClr(s->_socket, __CFReadSocketsFds);
    if (b && INVALID_SOCKET != __CFWakeupSocketPair[0]) {
        uint8_t c = 's';
        send(__CFWakeupSocketPair[0], (const char *)&c, sizeof(c), 0);
    }
    return b;
}

CF_INLINE Boolean __CFSocketSetFDForWrite(CFSocketRef s) {
    __CFSOCKETLOG_WS(s, "");
    Boolean b = __CFSocketFdSet(s->_socket, __CFWriteSocketsFds);
    if (b && INVALID_SOCKET != __CFWakeupSocketPair[0]) {
        uint8_t c = 'w';
        send(__CFWakeupSocketPair[0], (const char *)&c, sizeof(c), 0);
    }
    return b;
}

CF_INLINE Boolean __CFSocketClearFDForWrite(CFSocketRef s) {
    __CFSOCKETLOG_WS(s, "");
    Boolean b = __CFSocketFdClr(s->_socket, __CFWriteSocketsFds);
    if (b && INVALID_SOCKET != __CFWakeupSocketPair[0]) {
        uint8_t c = 'x';
        send(__CFWakeupSocketPair[0], (const char *)&c, sizeof(c), 0);
    }
    return b;
}

#if TARGET_OS_WIN32
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
#if TARGET_OS_WIN32
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

    if (errorCode) {
        __CFSOCKETLOG_WS(s, "error %ld", (long)errorCode);
    }

    __CFSocketLock(s);
    writeCallBacksAvailable = __CFSocketCallBackTypes(s) & (kCFSocketWriteCallBack | kCFSocketConnectCallBack);
    if ((s->_f.client & kCFSocketConnectCallBack) != 0) writeCallBacksAvailable &= ~kCFSocketConnectCallBack;
    if (!__CFSocketIsValid(s) || ((s->_f.disabled & writeCallBacksAvailable) == writeCallBacksAvailable)) {
        __CFSocketUnlock(s);
        return;
    }
    s->_errorCode = errorCode;
    __CFSocketSetWriteSignalled(s);
    __CFSOCKETLOG_WS(s, "signalling write");

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

static CFStringRef copyPeerAddress(CFAllocatorRef alloc, CFSocketNativeHandle s)
{
    return someAddrToString(alloc, getpeername, "peer", s);
}

static CFStringRef copyLocalAddress(CFAllocatorRef alloc, CFSocketNativeHandle s)
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

        __CFSOCKETLOG_WS(s, "read %ld", (long) recvlen);

        if (0 >= recvlen) {
            //??? should return error if <0
            /* zero-length data is the signal for perform to invalidate if socket is connection oriented */
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
        if (0 < namelen) {
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
        if (0 < namelen) {
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
            __CFSOCKETLOG_WS(s, "TIMEOUT RECEIVED - WILL SIGNAL IMMEDIATELY TO FLUSH (%ld buffered)", s->_bytesToBufferPos);

            /* we've got a timeout, but no bytes read, and we don't have any bytes to send.  Ignore the timeout. */
            if (s->_bytesToBufferPos == 0 && s->_leftoverBytes == NULL) {

                __CFSOCKETLOG_WS(s, "TIMEOUT - but no bytes, restoring to active set", s->_bytesToBufferPos);

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

                        __CFSOCKETLOG_WS(s, "BUFFERED READ GOT ERROR %d", errno);
                        break;

                    case 0:
                        __CFSOCKETLOG_WS(s, "DONE READING (EOF) - GOING TO SIGNAL");
                        s->_atEOF = true;
                        break;

                    default:
                        s->_bytesToBufferPos += ctRead;
                        if (s->_bytesToBuffer != s->_bytesToBufferPos) {

                            // Update the timeout notification time
                            struct timeval timeNow = { 0 };
                            gettimeofday(&timeNow, NULL);
                            timeradd(&timeNow, &s->_readBufferTimeout, &s->_readBufferTimeoutNotificationTime);

                            __CFSOCKETLOG_WS(s, "READ %ld - need %ld MORE - GOING BACK FOR MORE", ctRead, s->_bytesToBuffer - s->_bytesToBufferPos);

                            __CFLock(&__CFActiveSocketsLock);
                            /* restore socket to fds */
                            __CFSocketSetFDForRead(s);
                            __CFUnlock(&__CFActiveSocketsLock);
                            __CFSocketUnlock(s);
                            return;
                        } else {
                            // Clear the timeout notification time if the buffer is full
                            timerclear(&s->_readBufferTimeoutNotificationTime);

                            __CFSOCKETLOG_WS(s, "DONE READING (read %ld bytes) - GOING TO SIGNAL", ctRead);
                        }
                }
            }
        }
        
        __CFSocketSetReadSignalled(s);
    }

    __CFSOCKETLOG_WS(s, "read signaling source");

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
            __CFSOCKETLOG_WS(s, "WARNING: shouldn't set read buffer length while data (%ld bytes) is still in the read buffer (leftover total %ld)", ctBuffer, s->_leftoverBytes? CFDataGetLength(s->_leftoverBytes) : 0);

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
    __CFSOCKETLOG_WS(s, "READING BYTES (%ld buffered, out of %ld desired, eof = %d, err = %d)", s->_bytesToBufferPos, s->_bytesToBuffer, s->_atEOF, s->_bufferedReadError);

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
                __CFSOCKETLOG_WS(s, "DRAINED BUFFER - SHOULD START BUFFERING AGAIN");
                s->_bytesToBufferPos = 0;
                s->_bytesToBufferReadPos = 0;
            }

            __CFSOCKETLOG_WS(s, "SLURPED %ld BYTES FROM BUFFER %ld LEFT TO READ", ctBuffer, length);

            result = ctBuffer;
            goto unlock;
        }
    }
    /* nothing buffered, or no buffer selected */

    /* Did we get an error on a previous read (or buffered read)? */
    if (s->_bufferedReadError != 0) {
        __CFSOCKETLOG_WS(s, "RETURNING ERROR %d", s->_bufferedReadError);
        *error = s->_bufferedReadError;
        result = -1;
        goto unlock;
    }

    /* nothing buffered, if we've hit eof, don't bother reading any more */
    if (s->_atEOF) {

        __CFSOCKETLOG_WS(s, "RETURNING EOF");

        result = 0;
        goto unlock;
    }

    /* normal read */
    result = read(CFSocketGetNative(s), buffer, length);

    __CFSOCKETLOG_WS(s, "READ %ld bytes", result);

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
static void __CFSocketWriteSocketList(CFArrayRef sockets, CFDataRef fdSet, char* dst, CFIndex dstCount, Boolean onlyIfSet) {
    int len = snprintf(dst, dstCount, "{");
    dst += len;
    dstCount -= len;

    fd_set *tempfds = (fd_set *)CFDataGetBytePtr(fdSet);
    SInt32 idx, cnt;
    for (idx = 0, cnt = CFArrayGetCount(sockets); idx < cnt; idx++) {
        CFSocketRef s = (CFSocketRef)CFArrayGetValueAtIndex(sockets, idx);
        len = 0;
        if (FD_ISSET(s->_socket, tempfds)) {
            len = snprintf(dst, dstCount, " %d ", s->_socket);
        } else if (!onlyIfSet) {
            len = snprintf(dst, dstCount, " (%d) ", s->_socket);
        }
        dst += len;
        dstCount -= len;
    }

    snprintf(dst, dstCount, "}");
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

    __CFSOCKETLOG("socket manager received error %ld from select", (long)selectError);

    if (EBADF == selectError) {
        CFMutableArrayRef invalidSockets = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);

        __CFLock(&__CFActiveSocketsLock);
        CFIndex cnt = CFArrayGetCount(__CFWriteSockets);
        CFIndex idx;
        for (idx = 0; idx < cnt; idx++) {
            CFSocketRef s = (CFSocketRef)CFArrayGetValueAtIndex(__CFWriteSockets, idx);
            if (!__CFNativeSocketIsValid(s->_socket)) {
                __CFSOCKETLOG_WS(s, "socket manager found write socket invalid");
                CFArrayAppendValue(invalidSockets, s);
            }
        }
        cnt = CFArrayGetCount(__CFReadSockets);
        for (idx = 0; idx < cnt; idx++) {
            CFSocketRef s = (CFSocketRef)CFArrayGetValueAtIndex(__CFReadSockets, idx);
            if (!__CFNativeSocketIsValid(s->_socket)) {
                __CFSOCKETLOG_WS(s, "socket manager found read socket invalid");
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
            __CFSOCKETLOG("socket manager received EBADF(1): No sockets were marked as invalid, cleaning out fdsets");

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
#if TARGET_OS_LINUX && !TARGET_OS_CYGWIN
    pthread_setname_np(pthread_self(), "com.apple.CFSocket.private");
#elif !TARGET_OS_CYGWIN && !TARGET_OS_BSD
    pthread_setname_np("com.apple.CFSocket.private");
#endif
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
    struct timeval timeBeforeSelect = {0, 0};
    
    for (;;) {
        __CFLock(&__CFActiveSocketsLock);
        __CFSocketManagerIteration++;

#if defined(LOG_CFSOCKET)
        char* readBuffer = (char*) malloc(16384);
        __CFSocketWriteSocketList(__CFReadSockets, __CFReadSocketsFds, readBuffer, 16384, FALSE);
        char* writeBuffer = (char*) malloc(16384);
        __CFSocketWriteSocketList(__CFWriteSockets, __CFWriteSocketsFds, writeBuffer, 16384, FALSE);
        __CFSOCKETLOG("socket manager iteration %lu looking at: read sockets %s, write sockets %s", (unsigned long)__CFSocketManagerIteration, readBuffer, writeBuffer);

        free(readBuffer);
        free(writeBuffer);
#endif
        rfds = __CFSocketFdGetSize(__CFReadSocketsFds);
        wfds = __CFSocketFdGetSize(__CFWriteSocketsFds);
        maxnrfds = __CFMax(rfds, wfds);
        if (maxnrfds > fdentries * (int)NFDBITS) {
            fdentries = (maxnrfds + NFDBITS - 1) / NFDBITS;
            writefds = __CFSafelyReallocateWithAllocator(kCFAllocatorSystemDefault, writefds, fdentries * sizeof(fd_mask), 0, NULL);
            readfds = __CFSafelyReallocateWithAllocator(kCFAllocatorSystemDefault, readfds, fdentries * sizeof(fd_mask), 0, NULL);
        }
        memset(writefds, 0, fdentries * sizeof(fd_mask)); 
        memset(readfds, 0, fdentries * sizeof(fd_mask));
        CFDataGetBytes(__CFWriteSocketsFds, CFRangeMake(0, CFDataGetLength(__CFWriteSocketsFds)), (UInt8 *)writefds);
        CFDataGetBytes(__CFReadSocketsFds, CFRangeMake(0, CFDataGetLength(__CFReadSocketsFds)), (UInt8 *)readfds); 
		
        if (__CFReadSocketsTimeoutInvalid) {
            struct timeval* minTimeout = NULL;
            __CFReadSocketsTimeoutInvalid = false;

            __CFSOCKETLOG("Figuring out which sockets have timeouts...");

            CFArrayApplyFunction(__CFReadSockets, CFRangeMake(0, CFArrayGetCount(__CFReadSockets)), _calcMinTimeout_locked, (void*) &minTimeout);

            if (minTimeout == NULL) {
                __CFSOCKETLOG("No one wants a timeout!");
                pTimeout = NULL;
            } else {
                __CFSOCKETLOG("timeout will be %ld, %d!", minTimeout->tv_sec, minTimeout->tv_usec);
                tv = *minTimeout;
                pTimeout = &tv;
            }
        }

        if (pTimeout) {
            __CFSOCKETLOG("select will have a %ld, %d timeout", pTimeout->tv_sec, pTimeout->tv_usec);
            gettimeofday(&timeBeforeSelect, NULL);
        }
		
        __CFUnlock(&__CFActiveSocketsLock);

#if TARGET_OS_WIN32
        // On Windows, select checks connection failed sockets via the exceptfds parameter. connection succeeded is checked via writefds. We need both.
        exceptfds = writefds;
#elif defined(LOG_CFSOCKET) && defined(DEBUG_POLLING_SELECT)
        if (pTimeout == NULL) {
            /* If there's anyone with leftover bytes, they'll need to be awoken immediately */
            static struct timeval sKickerTime = { 5, 0 };
            pTimeout = &sKickerTime;
            __CFSOCKETLOG("Setting 5s select timeout as debug measure");
        }

        if (exceptfds == NULL) {
            exceptfds = (fd_set*) malloc(maxnrfds * NFDBITS);
            bzero(exceptfds, maxnrfds * NFDBITS);
        }
#endif

        nrfds = select(maxnrfds, readfds, writefds, exceptfds, pTimeout);

#if defined(LOG_CFSOCKET) && defined(DEBUG_POLLING_SELECT)
        __CFSOCKETLOG("socket manager woke from select, ret=%ld", (long)nrfds);

        if (nrfds < 0 && exceptfds && exceptfds != writefds) {
            CFMutableStringRef s = NULL;
            for (int i = 0;  i < nrfds;  i++) {
                if (FD_ISSET(i, exceptfds)) {
                    if (s == NULL) {
                        s = CFStringCreateMutable(kCFAllocatorDefault, 0);
                        CFStringAppendCString(s, "Error set { ", kCFStringEncodingUTF8);
                    }
                    CFStringAppendFormat(s, NULL, CFSTR("%d "), i);
                }
            }
            if (s == NULL)
                __CFSOCKETLOG("Error from select errno %d, but no fds specified", errno);
            else {
                CFStringAppendFormat(s, NULL, CFSTR("}"));
                __CFSOCKETLOG("Error from select errno %d, %@", errno, s);
                CFRelease(s);
            }
            free(exceptfds);
            exceptfds = nil;
        }
#endif
		/*
		 * select returned a timeout
		 */
        if (0 == nrfds) {
            Boolean didFindOne = false;
            struct timeval timeAfterSelect;
            struct timeval deltaTime;
            gettimeofday(&timeAfterSelect, NULL);
            /* timeBeforeSelect becomes the delta */
            timersub(&timeAfterSelect, &timeBeforeSelect, &deltaTime);

            __CFSOCKETLOG("Socket manager received timeout - kicking off expired reads (expired delta %ld, %d)", deltaTime.tv_sec, deltaTime.tv_usec);

            __CFLock(&__CFActiveSocketsLock);

            tempfds = NULL;
            cnt = CFArrayGetCount(__CFReadSockets);
            for (idx = 0; idx < cnt; idx++) {
                CFSocketRef s = (CFSocketRef)CFArrayGetValueAtIndex(__CFReadSockets, idx);

                if (timerisset(&s->_readBufferTimeout) || s->_leftoverBytes) {
                    didFindOne = true;

                    CFSocketNativeHandle sock = s->_socket;
                    // We might have an new element in __CFReadSockets that we weren't listening to,
                    // in which case we must be sure not to test a bit in the fdset that is
                    // outside our mask size.
                    Boolean sockInBounds = (0 <= sock && sock < maxnrfds);
                    /* if this sockets timeout is less than or equal elapsed time, then signal it */
                    if (INVALID_SOCKET != sock && sockInBounds) {
                        __CFSOCKETLOG_WS(s, "Expiring socket (delta %ld, %d)", s->_readBufferTimeout.tv_sec, s->_readBufferTimeout.tv_usec);

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
            if (! didFindOne) {
#if defined(LOG_CFSOCKET) && defined(DEBUG_POLLING_SELECT)
                __CFSOCKETLOG("select() timeout - but no sockets actually timed out.  Iteration %lu", (unsigned long) __CFSocketManagerIteration);
                CFAbsoluteTime endTime = CFAbsoluteTimeGetCurrent() + 3;
                CFRunLoopPerformBlock(CFRunLoopGetMain(), kCFRunLoopDefaultMode, ^{
                    CFTimeInterval dt = CFAbsoluteTimeGetCurrent() - endTime;
                    if (dt > 0) {
                        __CFSOCKETLOG("select() timeout %lu - took %.05f for the main runloop (TOO LONG!)", __CFSocketManagerIteration, dt);
                    } else {
                        __CFSOCKETLOG("select() timeout %lu - took %.05f for the main runloop", __CFSocketManagerIteration, dt < 0? -dt : dt);
                    }
                });
                CFRunLoopWakeUp(CFRunLoopGetMain());
#endif
            }
        }

        if (0 > nrfds) {
            manageSelectError();
            continue;
        }
        if (FD_ISSET(__CFWakeupSocketPair[1], readfds)) {
            recv(__CFWakeupSocketPair[1], (char *)buffer, sizeof(buffer), 0);
            __CFSOCKETLOG("socket manager received %c on wakeup socket\n", buffer[0]);
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
                    __CFSOCKETLOG_WS(s, "Manager: cleared socket from write fds");
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
            __CFSOCKETLOG_WS(s, "socket manager signaling for write", s, s->_socket);
            __CFSocketHandleWrite(s, FALSE);
            CFArraySetValueAtIndex(selectedWriteSockets, idx, kCFNull);
        }
        selectedWriteSocketsIndex = 0;
        
        for (idx = 0; idx < selectedReadSocketsIndex; idx++) {
            CFSocketRef s = (CFSocketRef)CFArrayGetValueAtIndex(selectedReadSockets, idx);
            if (kCFNull == (CFNullRef)s) continue;
            __CFSOCKETLOG_WS(s, "socket manager signaling for read", s, s->_socket);
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
#if TARGET_OS_MAC
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

const CFRuntimeClass __CFSocketClass = {
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
#if TARGET_OS_MAC
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
    return _kCFRuntimeIDCFSocket;
}

#if TARGET_OS_WIN32
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
			__CFSOCKETLOG("useExistingInstance is FALSE, removing existing instance %p from __CFAllSockets\n", memory);
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
    memory->_lock = CFLockInit;
    memory->_writeLock = CFLockInit;
    memory->_socket = sock;
    if (INVALID_SOCKET == sock || 0 != getsockopt(sock, SOL_SOCKET, SO_TYPE, (char *)&(memory->_socketType), (socklen_t *)&typeSize)) memory->_socketType = 0;		// cast for WinSock bad API
    if (INVALID_SOCKET != sock) {
        CFArrayCallBacks retainingCallbacks = kCFTypeArrayCallBacks;
        retainingCallbacks.copyDescription = NULL;
        memory->_runLoops = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &retainingCallbacks);
    }
    memory->_callout = callout;
    timerclear(&memory->_readBufferTimeout);
    timerclear(&memory->_readBufferTimeoutNotificationTime);
    
    if (INVALID_SOCKET != sock) CFDictionaryAddValue(__CFAllSockets, (void *)(uintptr_t)sock, memory);
    if (NULL == __CFSocketManagerThread) {
#if TARGET_OS_MAC || TARGET_OS_LINUX || TARGET_OS_BSD
        _CFThreadRef tid = 0;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
#if TARGET_OS_MAC
        pthread_attr_set_qos_class_np(&attr, qos_class_main(), 0);
#endif
        pthread_create(&tid, &attr, __CFSocketManager, 0);
        pthread_attr_destroy(&attr);
        _Static_assert(sizeof(_CFThreadRef) == sizeof(void *), "_CFThreadRef is not pointer sized");
        __CFSocketManagerThread = (void *)tid;
#elif TARGET_OS_WIN32
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
    __CFSOCKETLOG("created socket %p (%d) with callbacks 0x%x, callout %p", memory, memory->_socket, callBackTypes, callout);
    return memory;
}

CFSocketRef CFSocketCreateWithNative(CFAllocatorRef allocator, CFSocketNativeHandle sock, CFOptionFlags callBackTypes, CFSocketCallBack callout, const CFSocketContext *context) {
	return _CFSocketCreateWithNative(allocator, sock, callBackTypes, callout, context, TRUE);
}

void CFSocketInvalidate(CFSocketRef s) {
    CF_ASSERT_TYPE(CFSocketGetTypeID(), s);
    CHECK_FOR_FORK();
    UInt32 previousSocketManagerIteration;
    __CFGenericValidateType(s, CFSocketGetTypeID());
    __CFSOCKETLOG_WS(s, "flags 0x%x disabled 0x%x connected 0x%x\n", s->_f.client, s->_f.disabled, s->_f.connected);
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
    __CFSOCKETLOG("done for %p", s);
    CFRelease(s);
}

Boolean CFSocketIsValid(CFSocketRef s) {
    CF_ASSERT_TYPE(CFSocketGetTypeID(), s);
    CHECK_FOR_FORK();
    return __CFSocketIsValid(s);
}

CFSocketNativeHandle CFSocketGetNative(CFSocketRef s) {
    CF_ASSERT_TYPE_OR_NULL(CFSocketGetTypeID(), s);
    CHECK_FOR_FORK();
    return s == NULL? -1 : s->_socket;
}

CFDataRef CFSocketCopyAddress(CFSocketRef s) {
    CF_ASSERT_TYPE(CFSocketGetTypeID(), s);
    CHECK_FOR_FORK();
    CFDataRef result = NULL;
    __CFSocketLock(s);
    __CFSocketEstablishAddress(s);
    if (NULL != s->_address) {
        result = (CFDataRef)CFRetain(s->_address);
    }
    __CFSocketUnlock(s);
#if defined(LOG_CFSOCKET)
    CFStringRef local = copyLocalAddress(kCFAllocatorDefault, s->_socket);
    CFStringRef peer = copyPeerAddress(kCFAllocatorDefault, s->_socket);
    __CFSOCKETLOG_WS(s, "addresses local %@ peer %@", local, peer);
    if (local)
        CFRelease(local);
    if (peer)
        CFRelease(peer);
#endif
    return result;
}

CFDataRef CFSocketCopyPeerAddress(CFSocketRef s) {
    CF_ASSERT_TYPE(CFSocketGetTypeID(), s);
    CHECK_FOR_FORK();
    CFDataRef result = NULL;
    __CFSocketLock(s);
    __CFSocketEstablishPeerAddress(s);
    if (NULL != s->_peerAddress) {
        result = (CFDataRef)CFRetain(s->_peerAddress);
    }
    __CFSocketUnlock(s);
#if defined(LOG_CFSOCKET)
    CFStringRef local = copyLocalAddress(kCFAllocatorDefault, s->_socket);
    CFStringRef peer = copyPeerAddress(kCFAllocatorDefault, s->_socket);
    __CFSOCKETLOG_WS(s, "addresses local %@ peer %@", local, peer);
    if (local)
        CFRelease(local);
    if (peer)
        CFRelease(peer);
#endif
    return result;
}

void CFSocketGetContext(CFSocketRef s, CFSocketContext *context) {
    CF_ASSERT_TYPE(CFSocketGetTypeID(), s);
    CHECK_FOR_FORK();
    CFAssert1(0 == context->version, __kCFLogAssertion, "%s(): context version not initialized to 0", __PRETTY_FUNCTION__);
    *context = s->_context;
}

CFOptionFlags CFSocketGetSocketFlags(CFSocketRef s) {
    CF_ASSERT_TYPE(CFSocketGetTypeID(), s);
    CHECK_FOR_FORK();
    return s->_f.client;
}

void CFSocketSetSocketFlags(CFSocketRef s, CFOptionFlags flags) {
    CF_ASSERT_TYPE(CFSocketGetTypeID(), s);
    CHECK_FOR_FORK();
    __CFSocketLock(s);
#if LOG_CFSOCKET
    CFOptionFlags oldFlags = s->_f.client;
#endif
    s->_f.client = flags;
    __CFSocketUnlock(s);
    __CFSOCKETLOG_WS(s, "set flags 0x%x (was 0x%x)", flags, oldFlags);
}

void CFSocketDisableCallBacks(CFSocketRef s, CFOptionFlags callBackTypes) {
    CF_ASSERT_TYPE(CFSocketGetTypeID(), s);
    CHECK_FOR_FORK();
    Boolean wakeup = false;
    uint8_t readCallBackType;
    __CFSocketLock(s);
    if (__CFSocketIsValid(s) && __CFSocketIsScheduled(s)) {
        callBackTypes &= __CFSocketCallBackTypes(s);
        readCallBackType = __CFSocketReadCallBackType(s);
        s->_f.disabled |= callBackTypes;
        __CFSOCKETLOG_WS(s, "unscheduling with flags 0x%x disabled 0x%x connected 0x%x for types 0x%lx\n", s->_f.client, s->_f.disabled, s->_f.connected, callBackTypes);
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
        __CFSOCKETLOG_WS(s, "rescheduling with flags 0x%x disabled 0x%x connected 0x%x for types 0x%lx\n", s->_f.client, s->_f.disabled, s->_f.connected, callBackTypes);
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
                    if (kCFNotFound == idx)
                        CFArrayAppendValue(__CFWriteSockets, s);
                    if (kCFNotFound == idx)
                        __CFSOCKETLOG_WS(s, "put %p __CFWriteSockets list due to force and non-presence");
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
    CF_ASSERT_TYPE(CFSocketGetTypeID(), s);
    CHECK_FOR_FORK();
    __CFSocketLock(s);
    __CFSocketEnableCallBacks(s, callBackTypes, TRUE, 'r');
    __CFSOCKETLOG_WS(s, "done for callbackTypes %x", callBackTypes);
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
            __CFSOCKETLOG_WS(s, "rl %p (%s), mode %@", rl, (CFRunLoopGetCurrent() == rl? "current" : CFRunLoopGetMain() == rl? "main" : ""), mode);
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
            __CFSOCKETLOG_WS(s, "removing from __CFWriteSockets list");
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
    __CFSOCKETLOG_WS(s, "entering perform with read signalled %d write signalled %d connect signalled %d callback types %d", readSignalled, writeSignalled, connectSignalled, callBackTypes);
    if (writeSignalled) {
        errorCode = s->_errorCode;
        s->_f.connected = TRUE;
    }
    __CFSocketUnlock(s);
    if ((callBackTypes & kCFSocketConnectCallBack) != 0) {
        if (connectSignalled && (!calledOut || CFSocketIsValid(s))) {
            __CFSOCKETLOG_WS(s, "doing connect callback (%p), error: %d", callout, errorCode);
            if (errorCode) {
                if (callout) callout(s, kCFSocketConnectCallBack, NULL, &errorCode, contextInfo);
                calledOut = true;
            } else {
                if (callout) callout(s, kCFSocketConnectCallBack, NULL, NULL, contextInfo);
                calledOut = true;
            }
        }
    }
    if (kCFSocketDataCallBack == readCallBackType) {
        if (NULL != data && (!calledOut || CFSocketIsValid(s))) {
            SInt32 datalen = CFDataGetLength(data);
            __CFSOCKETLOG_WS(s, "perform calling out data of length %ld", (long)datalen);
            if (callout) callout(s, kCFSocketDataCallBack, address, data, contextInfo);
            calledOut = true;
            if (0 == datalen && __CFSocketIsConnectionOriented(s)) CFSocketInvalidate(s);
        }
    } else if (kCFSocketAcceptCallBack == readCallBackType) {
        if (INVALID_SOCKET != sock && (!calledOut || CFSocketIsValid(s))) {
            __CFSOCKETLOG_WS(s, "perform calling out accept");
            if (callout) callout(s, kCFSocketAcceptCallBack, address, &sock, contextInfo);
            calledOut = true;
        }
    } else if (kCFSocketReadCallBack == readCallBackType) {
        if (readSignalled && (!calledOut || CFSocketIsValid(s))) {
            __CFSOCKETLOG_WS(s, "doing read callback");
            __CFSOCKETLOG("__CFSocketPerformV0(%p)  for socket %d", s, s->_socket);
            if (callout) callout(s, kCFSocketReadCallBack, NULL, NULL, contextInfo);
            calledOut = true;
        }
    }
    if ((callBackTypes & kCFSocketWriteCallBack) != 0) {
        if (writeSignalled && !errorCode && (!calledOut || CFSocketIsValid(s))) {
            __CFSOCKETLOG_WS(s, "doing write callback");
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
    void *contextInfo = NULL;
    void (*contextRelease)(const void *) = NULL;
    __CFSOCKETLOG_WS(s, "Starting");

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

    if (NULL != s->_context.retain) {
        contextInfo = s->_context.info;
        contextRelease = s->_context.release;
        s->_context.retain(contextInfo);
    }

    __CFSocketDoCallback(s, data, address, sock);	// does __CFSocketUnlock(s)

    if (NULL != contextRelease) {
        contextRelease(contextInfo);
    }

    if (NULL != data) CFRelease(data);
    if (NULL != address) CFRelease(address);

    __CFSocketLock(s);
    if (__CFSocketIsValid(s) && kCFSocketNoCallBack != readCallBackType) {
        // if there's still more data, we want to wake back up right away
        if ((kCFSocketDataCallBack == readCallBackType || kCFSocketAcceptCallBack == readCallBackType) && NULL != s->_dataQueue && 0 < CFArrayGetCount(s->_dataQueue)) {
            __CFSOCKETLOG_WS(s, "perform short-circuit signaling source with flags 0x%x disabled 0x%x connected 0x%x\n", s->_f.client, s->_f.disabled, s->_f.connected);
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
    __CFSOCKETLOG_WS(s, "Done");
}

CFRunLoopSourceRef CFSocketCreateRunLoopSource(CFAllocatorRef allocator, CFSocketRef s, CFIndex order) {
    CF_ASSERT_TYPE(CFSocketGetTypeID(), s);
    CHECK_FOR_FORK();
    CFRunLoopSourceRef result = NULL;
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



static uint16_t __CFSocketDefaultNameRegistryPortNumber = 2454;

CONST_STRING_DECL(kCFSocketCommandKey, "Command")
CONST_STRING_DECL(kCFSocketNameKey, "Name")
CONST_STRING_DECL(kCFSocketValueKey, "Value")
CONST_STRING_DECL(kCFSocketResultKey, "Result")
CONST_STRING_DECL(kCFSocketErrorKey, "Error")
CONST_STRING_DECL(kCFSocketRegisterCommand, "Register")
CONST_STRING_DECL(kCFSocketRetrieveCommand, "Retrieve")
CONST_STRING_DECL(__kCFSocketRegistryRequestRunLoopMode, "CFSocketRegistryRequest")

static os_unfair_lock __CFSocketWriteLock_ = OS_UNFAIR_LOCK_INIT;
//#warning can only send on one socket at a time now

CF_INLINE void __CFSocketWriteLock(CFSocketRef s) {
    os_unfair_lock_lock(& __CFSocketWriteLock_);
}

CF_INLINE void __CFSocketWriteUnlock(CFSocketRef s) {
    os_unfair_lock_unlock(& __CFSocketWriteLock_);
}

//??? need timeout, error handling, retries
CFSocketError CFSocketSendData(CFSocketRef s, CFDataRef address, CFDataRef data, CFTimeInterval timeout) {
    CF_ASSERT_TYPE(CFSocketGetTypeID(), s);
    CHECK_FOR_FORK();
    const uint8_t *dataptr, *addrptr = NULL;
    SInt32 datalen, addrlen = 0, size = 0;
    CFSocketNativeHandle sock = INVALID_SOCKET;
    struct timeval tv;
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
        __CFSOCKETLOG_WS(s, "wrote %ld bytes", (long)size);
        __CFSocketWriteUnlock(s);
        CFRelease(s);
    }
    return (size > 0) ? kCFSocketSuccess : kCFSocketError;
}

CFSocketError CFSocketSetAddress(CFSocketRef s, CFDataRef address) {
    CF_ASSERT_TYPE(CFSocketGetTypeID(), s);
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
#if TARGET_OS_MAC
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
    const int bindResult = bind(sock, name, namelen);
    if (0 == bindResult) {
        const int listenResult = listen(sock, 256);
        if (listenResult != 0) {
            CFLog(kCFLogLevelDebug, CFSTR("CFSocketSetAddress listen failure: %d"), errno);
        }
    }
    else {
        CFLog(kCFLogLevelDebug, CFSTR("CFSocketSetAddress bind failure: %d"), errno);
    }
    
    //??? should return errno; historically this never looked at the listenResult
    return (CFIndex)bindResult;
}

CFSocketError CFSocketConnectToAddress(CFSocketRef s, CFDataRef address, CFTimeInterval timeout) {
    CF_ASSERT_TYPE(CFSocketGetTypeID(), s);
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
#if TARGET_OS_MAC
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
#if TARGET_OS_WIN32
            if (connect_err == WSAEWOULDBLOCK) connect_err = EINPROGRESS;
#endif
        }
        __CFSOCKETLOG_WS(s, "connection attempt returns %d error %d on socket %d (flags 0x%x blocking %d)", (int) result, (int) connect_err, sock, (int) flags, wasBlocking);
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
            __CFSOCKETLOG_WS(s, "timed connection attempt %s result %d, select returns %d error %d\n", (result == 0) ? "succeeds" : "fails", (int) result, (int) nrfds, (int) select_err);
        }
        if (wasBlocking && (timeout > 0.0 || timeout < 0.0)) ioctlsocket(sock, FIONBIO, (u_long *)&no);
        if (EINPROGRESS == connect_err && timeout < 0.0) {
            result = 0;
            __CFSOCKETLOG_WS(s, "connection attempt continues in background\n");
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
#if TARGET_OS_MAC
    if (PF_LOCAL == protocolFamily && 0 >= socketType) socketType = SOCK_STREAM;
#endif
#if TARGET_OS_WIN32
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
#if TARGET_OS_MAC
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
#if TARGET_OS_MAC
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
        if (validatedSignature.address) CFRelease(validatedSignature.address);
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

