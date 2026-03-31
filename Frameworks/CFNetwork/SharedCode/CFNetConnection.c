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
/*	CFNetConnection.c
	Copyright 2002, Apple, Inc. All rights reserved.
	Responsibility: Becky Willrich
*/

#include "CFNetworkInternal.h"
#include "CFNetConnection.h"
#include <CFNetwork/CFNetworkPriv.h> 
#include <sys/types.h>

#if defined(__WIN32__)
#include <winsock2.h>
#define ECONNRESET WSAECONNRESET
#endif

#define LOCK_NET_CONNECTION (0)
#define ACCEPTS_NEW_REQUESTS (1)
#define SHOULD_PIPELINE (2)
#define TRANSMITTING_CURRENT_REQUEST (3)
#define FIRST_REQUEST_SENT (4)
#define CLIENT_IS_SET (5)
#define CONNECTION_LOST (6)
// Use CONNECTION_LOST to mark that we should not advance to the next response; all further responses have been orphaned
#define CURRENT_RESPONSE_COMPLETE (7)

#ifdef __CONSTANT_CFSTRINGS__
#define _kCFNetConnectionDescribeFormat	CFSTR("%@:%d")
#else
static CONST_STRING_DECL(_kCFNetConnectionDescribeFormat, "%@:%d")
#endif	/* __CONSTANT_CFSTRINGS__ */


struct __CFNetConnectionCacheKey {
    CFStringRef host;
    SInt32 port;
    UInt32 connType;
    CFDictionaryRef properties;
};

struct __CFNetConnectionCache {
    CFMutableDictionaryRef dictionary;
    CFSpinLock_t connectionCacheLock;
};

const void *connCacheKeyRetain(CFAllocatorRef allocator, const void *value) {
    _CFNetConnectionCacheKey key = (_CFNetConnectionCacheKey)value;
    _CFNetConnectionCacheKey newKey = CFAllocatorAllocate(allocator, sizeof(struct __CFNetConnectionCacheKey), 0);
    if (newKey) {
        newKey->host = key->host;
        if (newKey->host) CFRetain(newKey->host);
        newKey->port = key->port;
        newKey->connType = key->connType;
        newKey->properties = key->properties;
        if (newKey->properties) CFRetain(newKey->properties);
    }
    return newKey;
}

void connCacheKeyRelease(CFAllocatorRef allocator, const void *value) {
    _CFNetConnectionCacheKey key = (_CFNetConnectionCacheKey)value;
    if (key->host)
        CFRelease(key->host);
    if (key->properties) CFRelease(key->properties);
    CFAllocatorDeallocate(allocator, key);
}

static CFStringRef connCacheKeyCopyDesc(const void *value) {
    _CFNetConnectionCacheKey key = (_CFNetConnectionCacheKey)value;
    return CFStringCreateWithFormat(NULL, NULL, _kCFNetConnectionDescribeFormat, key->host,key->port);
}

static Boolean connCacheKeyEqual(const void *value1, const void *value2) {
    _CFNetConnectionCacheKey key1 = (_CFNetConnectionCacheKey)value1;
    _CFNetConnectionCacheKey key2 = (_CFNetConnectionCacheKey)value2;
    if (key1->port != key2->port || key1->connType != key2->connType || !CFEqual(key1->host, key2->host)) {
        return FALSE;
    }
    if (!key1->properties) {
        return (key2->properties == NULL || CFDictionaryGetCount(key2->properties) == 0);
    } else if (!key2->properties) {
        return (CFDictionaryGetCount(key1->properties) == 0);
    }
    return CFEqual(key1->properties, key2->properties);
}

static CFHashCode connCacheKeyHash(const void *value) {
    _CFNetConnectionCacheKey key = (_CFNetConnectionCacheKey)value;
    return key->host ? CFHash(key->host) : 0;
}

_CFNetConnectionCacheKey createConnectionCacheKey(CFStringRef host, SInt32 port, UInt32 connType, CFDictionaryRef properties)
{
    _CFNetConnectionCacheKey key = malloc(sizeof(struct __CFNetConnectionCacheKey));

    key->host = host;
    if (key->host)
        CFRetain(key->host);
    key->port = port;
    key->connType = connType;
    key->properties = properties;
    if (key->properties)
       CFRetain(key->properties);
       
	return(key);
}

void releaseConnectionCacheKey(_CFNetConnectionCacheKey key)
{
    if (key->host) CFRelease(key->host);
    if (key->properties) CFRelease(key->properties);
    free(key);
}

#ifdef DEBUG
void printKey(_CFNetConnectionCacheKey key)
{
    printf("key = %#x\n", (int)key);
    printf("  host = %#x\n", (int)key->host);
    CFShow(key->host);
    printf("  port = %ld\n", key->port);
    printf("  connType = %d\n", (int)key->connType);
    printf("  props = %#x\n", (int)key->properties);
    if (key->properties) CFShow(key->properties);
}
#endif	/* DEBUG */

void getValuesFromKey(const _CFNetConnectionCacheKey theKey, CFStringRef *host, SInt32 *port, UInt32 *connType, CFDictionaryRef *properties)
{
    _CFNetConnectionCacheKey key = (_CFNetConnectionCacheKey)theKey;

    *host = key->host;
    *port = key->port;
    *connType = key->connType;
    *properties = key->properties;
}

// strange behavior in PB: this is declared in header so remove when not needed
CFNetConnectionCacheRef createConnectionCache(void);

CFNetConnectionCacheRef createConnectionCache(void)
{
    CFNetConnectionCacheRef conn_cache = malloc(sizeof(struct __CFNetConnectionCache));
 
    if (conn_cache) {
        CFMutableDictionaryRef dictionary;
        CFDictionaryKeyCallBacks connectionCacheCallBacks = {0, connCacheKeyRetain, connCacheKeyRelease, connCacheKeyCopyDesc, connCacheKeyEqual, connCacheKeyHash};
        
        dictionary = CFDictionaryCreateMutable(NULL, 0, &connectionCacheCallBacks, &kCFTypeDictionaryValueCallBacks);
        if (dictionary) {
            conn_cache->dictionary = dictionary;
            conn_cache->connectionCacheLock = 0;
        } else {
            free(conn_cache);
            conn_cache = NULL;
        }
    }
    return(conn_cache);
}

#if defined(__WIN32__)
void releaseConnectionCache(CFNetConnectionCacheRef cache)
{
    if (cache) {
        CFRelease(cache->dictionary);
        free(cache);
    }
}
#endif	/* defined(__WIN32__) */

void lockConnectionCache(CFNetConnectionCacheRef cache)
{
    __CFSpinLock(&cache->connectionCacheLock);
}

void unlockConnectionCache(CFNetConnectionCacheRef cache)
{
    __CFSpinUnlock(&cache->connectionCacheLock);
}

_CFNetConnectionRef findOrCreateNetConnection(CFNetConnectionCacheRef connectionCache, CFAllocatorRef allocator, const _CFNetConnectionCallBacks *callbacks, const void *info, _CFNetConnectionCacheKey key, Boolean persistent, CFDictionaryRef connectionProperties)
{
    _CFNetConnectionRef conn = NULL;
    Boolean created = FALSE;
    CFIndex count;
    
    if (!persistent) {
        // This request gets its own connection
        conn = _CFNetConnectionCreate(allocator, info, callbacks, TRUE);
        if (conn) {
            created = TRUE;
        }
    } else {
        lockConnectionCache(connectionCache);
        
        conn = (_CFNetConnectionRef)CFDictionaryGetValue(connectionCache->dictionary, key);
        if (conn) {
            if (!_CFNetConnectionWillEnqueueRequests(conn)) {
                conn = NULL;
                CFDictionaryRemoveValue(connectionCache->dictionary, key);
            } else {
                created = FALSE;
                CFRetain(conn);
            }
        } 
        if (!conn) {
            conn = _CFNetConnectionCreate(allocator, info, callbacks, TRUE);
            if (conn) {
                created = TRUE;
                _CFNetConnectionSetAllowsNewRequests(conn, TRUE);
                CFDictionarySetValue(connectionCache->dictionary, key, conn);
            }
        }
        unlockConnectionCache(connectionCache);
    }
    if (created && (count = CFDictionaryGetCount(connectionProperties)) > 0) {
        CFStringRef *keys = CFAllocatorAllocate(allocator, sizeof(CFStringRef)*count*2, 0);
        CFTypeRef *values = (CFTypeRef *)(keys + count);
        CFIndex index;
        CFDictionaryGetKeysAndValues(connectionProperties, (const void **)keys, (const void **)values);
        for (index = 0; index < count; index ++) {
            if (!CFReadStreamSetProperty(_CFNetConnectionGetResponseStream(conn), keys[index], values[index])) {
                CFWriteStreamSetProperty(_CFNetConnectionGetRequestStream(conn), keys[index], values[index]);
            }
        }
        CFAllocatorDeallocate(allocator, keys);
    }
    return conn;
}

void removeFromConnectionCache(CFNetConnectionCacheRef cache, _CFNetConnectionRef conn, _CFNetConnectionCacheKey key) {
    _CFNetConnectionRef cachedConn;
    lockConnectionCache(cache);
    cachedConn = (_CFNetConnectionRef)CFDictionaryGetValue(cache->dictionary, key);
    if (cachedConn && cachedConn == conn) {
        CFDictionaryRemoveValue(cache->dictionary, key);
    }
    unlockConnectionCache(cache);
}

// for mark & sweep algorithms around callouts, where we're worried about the client removing itself (and possibly others) from the queue while we're walking it.  See sendStateChanged for an example.
#define MARKED_REQUEST (0)
#define IS_ZOMBIE_REQUEST (1)

typedef struct _CFNetRequest {
    struct _CFNetRequest *next;
    void *request;
    UInt8 flags; 
} _CFNetRequest;

static inline Boolean isMarkedRequest(_CFNetRequest *req) {
    return __CFBitIsSet(req->flags, MARKED_REQUEST);
}

static inline Boolean isZombieRequest(_CFNetRequest *req) {
    return __CFBitIsSet(req->flags, IS_ZOMBIE_REQUEST);
}

static _CFNetRequest *nextRealRequest(_CFNetRequest *req) {
    _CFNetRequest *orig;
    if (!req) return req;
    orig = req;
    while (req && isZombieRequest(req)) {
        req = req->next;
    }
    if (!req) req = orig;
    return req;
}

typedef struct {
    CFRuntimeBase _cfBase;
    CFOptionFlags flags;
    
    _CFMutex lock;
    
	UInt32	count;
	
    _CFNetRequest *head;
    _CFNetRequest *tail;
    _CFNetRequest *currentRequest;
    _CFNetRequest *currentResponse;
    
    CFWriteStreamRef requestStream;
    CFReadStreamRef responseStream;

    CFAbsoluteTime emptyTime; // The time at which this connection's queue was completely emptied
//    int numRequests;
    
    const _CFNetConnectionCallBacks *cb;
    const void *info;
} __CFNetConnection;

static inline void _CFNetConnectionLock (__CFNetConnection *conn) {
	if (__CFBitIsSet(conn->flags, LOCK_NET_CONNECTION)) {
		_CFMutexLock(&conn->lock);
	}
}

static inline void _CFNetConnectionUnlock(__CFNetConnection *conn) {
	if (__CFBitIsSet(conn->flags, LOCK_NET_CONNECTION)) {
		_CFMutexUnlock(&conn->lock);
	}
}

//#define LOG_CONNECTIONS 1
//#define DEBUG_CONNECTIONS 1
static void shutdownConnectionStreams(__CFNetConnection* conn);
static void rescheduleStream(CFTypeRef stream, CFArrayRef oldRLArray, CFArrayRef newRLArray);
static void scheduleNewRequest(__CFNetConnection* conn, _CFNetRequest *newRequest, _CFNetRequest *priorRequest, Boolean priorRequestIsNewResponse);
static void scheduleNewResponse(__CFNetConnection* conn, _CFNetRequest *newRequest, _CFNetRequest *priorRequest);

#if defined(DEBUG_CONNECTIONS)
static Boolean checkList(_CFNetRequest *head, _CFNetRequest *tail) {
    _CFNetRequest *p;
    if (head && !tail) return FALSE;
    if (tail && !head) return FALSE;
    if (!head) return TRUE;
    for (p = head; p != tail && p; p = p->next)
        ;
    if (!p) return FALSE;
    if (p->next) return FALSE;
    return TRUE;
}
#endif

/* Linked list convenience functions */
static void addToList(_CFNetRequest **head, _CFNetRequest **tail, _CFNetRequest *newNode) {
#if defined(DEBUG_CONNECTIONS)
    if (!checkList(*head, *tail)) {
        fprintf(stderr, "-- bad linked list into addToList\n");
    }
#endif
    if (!*head) {
        *head = newNode;
        *tail = newNode;
        newNode->next = NULL;
    } else {
        (*tail)->next = newNode;
        *tail = newNode;
        newNode->next = NULL;
    }
#if defined(DEBUG_CONNECTIONS)
    if (!checkList(*head, *tail)) 
        fprintf(stderr, "-- bad linked list out of addToList\n");
#endif
}

static Boolean isInList(_CFNetRequest *list, void *req) {
    _CFNetRequest *p;
    for (p = list; p != NULL; p = p->next) {
        if (p->request == req) break;
    }
    if (p) return TRUE;
    return FALSE;
}

// Returns whether the node was actually found and replaced
static Boolean replaceInList(_CFNetRequest **list, _CFNetRequest **tail, void *origReq, void *newReq) {
    _CFNetRequest *p;
#if defined(DEBUG_CONNECTIONS)
    if (!checkList(*list, *tail)) 
        fprintf(stderr, "-- bad linked list in to replaceInList\n");
#endif
    for (p = *list; p != NULL; p = p->next) {
        if (p->request == origReq) {
            p->request = newReq;
            __CFBitSet(p->flags, IS_ZOMBIE_REQUEST);
#if defined(DEBUG_CONNECTIONS)
            if (!checkList(*list, *tail)) 
                fprintf(stderr, "-- bad linked list out of replaceInList\n");
#endif
            return TRUE;
        }
    }
    return FALSE;
}

// static int numConnections = 0;

static void _CFNetConnectionFinalize(__CFNetConnection* conn) {
    CFAllocatorRef alloc = CFGetAllocator(conn);
    if (conn->head) {
        _CFNetRequest *p = conn->head;
        // Should we orphan enqueued requests here?  Or just assume the caller knows what they're doing?
        while (p) {
            _CFNetRequest *q = p->next;
            CFAllocatorDeallocate(alloc, p);
            p = q;
        }
    }
//    fprintf(stderr, "Processed %d requests\n", conn->numRequests);
	if (__CFBitIsSet(conn->flags, LOCK_NET_CONNECTION)) 
		_CFMutexDestroy(&conn->lock);
    if (conn->cb->finalize) conn->cb->finalize(alloc, conn->info);
    shutdownConnectionStreams(conn);
//    numConnections --;
}

static void connectionResponseCallBack(CFReadStreamRef stream, CFStreamEventType type, void *info) {
    __CFNetConnection* conn = (__CFNetConnection*)info;
    CFRetain(conn);
    _CFNetConnectionLock(conn);
    if (conn->currentResponse && !__CFBitIsSet(conn->flags, CURRENT_RESPONSE_COMPLETE)) {
        conn->cb->responseStreamCallBack(conn->currentResponse->request, stream, type, (_CFNetConnectionRef)conn, conn->info);
    } else if (type == kCFStreamEventEndEncountered) {
        _CFNetConnectionLost((_CFNetConnectionRef)conn);
    } else if (type == kCFStreamEventErrorOccurred) {
        CFStreamError err = CFReadStreamGetError(stream);
        _CFNetConnectionErrorOccurred((_CFNetConnectionRef)conn, &err);
    }
    _CFNetConnectionUnlock(conn);
    CFRelease(conn);
}

static void connectionRequestCallBack(CFWriteStreamRef stream, CFStreamEventType type, void *info) {
    __CFNetConnection* conn = (__CFNetConnection*)info;
    CFRetain(conn);
	_CFNetConnectionLock(conn);

    if (conn->currentRequest) {
        conn->cb->requestStreamCallBack(conn->currentRequest->request, stream, type, (_CFNetConnectionRef)conn, conn->info);
    } else if (!conn->currentResponse) {
        // No requests currently; just get us shut down properly.
        if (type == kCFStreamEventEndEncountered) {
            _CFNetConnectionLost((_CFNetConnectionRef)conn);
        } else if (type == kCFStreamEventErrorOccurred) {
            CFStreamError err = CFWriteStreamGetError(stream);
            _CFNetConnectionErrorOccurred((_CFNetConnectionRef)conn, &err);
        }
    }
	_CFNetConnectionUnlock(conn);
    CFRelease(conn);
}


static void _ConnectionSetClient(__CFNetConnection* conn, Boolean set) {

    CFStreamClientContext ctxt = {0, conn, NULL, NULL, NULL};
    CFStreamClientContext* ctxtPtr = set ? &ctxt : NULL;
    CFStreamEventType events = set ? ~0L : 0L;
    CFWriteStreamClientCallBack wcb = set ? connectionRequestCallBack : NULL;
    CFReadStreamClientCallBack rcb = set ? connectionResponseCallBack : NULL;
    
    if (set)
        __CFBitSet(conn->flags, CLIENT_IS_SET);
    else
        __CFBitClear(conn->flags, CLIENT_IS_SET);

    if (conn->requestStream) {
        CFWriteStreamSetClient(conn->requestStream, events, wcb, ctxtPtr);
    }
    
    if (conn->responseStream) {
        CFReadStreamSetClient(conn->responseStream, events, rcb, ctxtPtr);
    }
}


static void openConnectionStreams(__CFNetConnection* conn) {
    if (conn->requestStream) {
        CFWriteStreamOpen(conn->requestStream);
    }
    if (conn->responseStream) {
        CFReadStreamOpen(conn->responseStream);
    }
}

static void shutdownConnectionStreams(__CFNetConnection* conn) {
#if defined(LOG_CONNECTIONS)
    fprintf(stderr, "shutdownConnectionStreams(0x%x)\n", (unsigned)conn);
#endif

    __CFBitClear(conn->flags, CLIENT_IS_SET);

    if (conn->requestStream) {
        CFWriteStreamSetClient(conn->requestStream, 0, NULL, NULL);
        CFWriteStreamClose(conn->requestStream);
        CFRelease(conn->requestStream);
        conn->requestStream = NULL;
    }
    if (conn->responseStream) {
        CFReadStreamSetClient(conn->responseStream, 0, NULL, NULL);
        CFReadStreamClose(conn->responseStream);
        CFRelease(conn->responseStream);
        conn->responseStream = NULL;
    }
}


static _CFOnceLock __kCFNetConnectionRegisterClass = _CFOnceInitializer;
static CFTypeID __kCFNetConnectionTypeID = _kCFRuntimeNotATypeID;


static void _CFNetConnectionRegisterClass(void) {
	
	static const CFRuntimeClass __CFNetConnectionClass = {
		0,
		"CFNetConnection",
		NULL,      // init
		NULL,      // copy
		(void(*)(CFTypeRef))_CFNetConnectionFinalize,
		NULL,      // equal
		NULL,      // hash
		NULL,      // 
		NULL       // copyDescription
	};

    __kCFNetConnectionTypeID = _CFRuntimeRegisterClass(&__CFNetConnectionClass);
}


CFTypeID
_CFNetConnectionGetTypeID(void) {

    _CFDoOnce(&__kCFNetConnectionRegisterClass, _CFNetConnectionRegisterClass);
    
    return __kCFNetConnectionTypeID;
}


const void*
_CFNetConnectionGetInfoPointer(_CFNetConnectionRef arg) {
    
    const void* result;
    __CFNetConnection* conn = (__CFNetConnection*)arg;

    _CFNetConnectionLock(conn);

    result = conn->info;

    _CFNetConnectionUnlock(conn);

    return result;
}

/* extern */ _CFNetConnectionRef
_CFNetConnectionCreate(CFAllocatorRef alloc, const void *info, const _CFNetConnectionCallBacks *callbacks, Boolean isThreadSafe) {

    __CFNetConnection* connection;
    
#if defined(LOG_CONNECTIONS)
    fprintf(stderr, "-- CFNetConnectionCreate(0x%x, 0x%x,...) -", (int)alloc, (int)info);
#endif

    connection = (__CFNetConnection*)_CFRuntimeCreateInstance(alloc,
                                                              _CFNetConnectionGetTypeID(),
                                                              sizeof(__CFNetConnection) - sizeof(CFRuntimeBase),
                                                              NULL);

    if (!connection)
        return NULL;

//    numConnections ++;
	connection->flags = 0;
    if (isThreadSafe) {
        _CFMutexInit(&connection->lock, TRUE);
        __CFBitSet(connection->flags, LOCK_NET_CONNECTION);
    }
    connection->emptyTime = CFAbsoluteTimeGetCurrent();
	
	connection->count = 0;
	
    connection->head = NULL;
    connection->tail = NULL;
    connection->currentRequest = NULL;
    connection->currentResponse = NULL;
//    connection->numRequests = 0;
    connection->requestStream = NULL;
    connection->responseStream = NULL;
        
    connection->cb = callbacks;
    if (connection->cb && connection->cb->create) {
        connection->info = connection->cb->create(alloc, info);
    } else {
        connection->info = info;
    }
    
    connection->cb->createConnectionStreams(alloc,
                                            connection->info,
                                            &connection->requestStream,
                                            &connection->responseStream);
    
    if (!connection->requestStream && !connection->responseStream) {
        // Creation failed
        CFRelease(connection);
        return NULL;
    }
    
    _ConnectionSetClient(connection, TRUE);

    __CFBitSet(connection->flags, ACCEPTS_NEW_REQUESTS);

#if defined(LOG_CONNECTIONS)
    fprintf(stderr, "-- 0x%x returned\n", (int)connection);
#endif

    return (_CFNetConnectionRef)connection;
}

extern
Boolean _CFNetConnectionEnqueue(_CFNetConnectionRef arg, void *req) {

    _CFNetRequest *newReq;
    Boolean result = FALSE;
    __CFNetConnection* conn = (__CFNetConnection*)arg;

    CFRetain(conn); // In case the callout below causes us to be released & destroyed
	_CFNetConnectionLock(conn);
    
#if defined(LOG_CONNECTIONS)
    fprintf(stderr, "-- CFNetConnectionEnqueue(0x%x, 0x%x) - ", (int)conn, (int)req);
#endif

    if (__CFBitIsSet(conn->flags, ACCEPTS_NEW_REQUESTS)) {
        // Put in to the queue
        newReq = CFAllocatorAllocate(CFGetAllocator(conn), sizeof(_CFNetRequest), 0);
        newReq->request = req;
        newReq->next = NULL;
        newReq->flags = 0;
        addToList(&(conn->head), &(conn->tail), newReq);
        if (!conn->currentRequest) {
            conn->currentRequest = newReq;
        }
        if (!conn->currentResponse) {
            conn->currentResponse = newReq;
        }
        conn->cb->requestStateChanged(req, kQueued, NULL, (_CFNetConnectionRef)conn, conn->info);

        if (conn->currentRequest == conn->currentResponse || __CFBitIsSet(conn->flags, SHOULD_PIPELINE)) {
            if (conn->currentRequest == newReq) {
                scheduleNewRequest(conn, conn->currentRequest, NULL, FALSE);
            } else if (conn->cb->runLoopAndModesArrayForRequest && conn->requestStream && nextRealRequest(conn->currentRequest) == newReq) {
                rescheduleStream(conn->requestStream, NULL, conn->cb->runLoopAndModesArrayForRequest(newReq->request, (_CFNetConnectionRef)conn, conn->info));
                if (__CFBitIsSet(conn->flags, TRANSMITTING_CURRENT_REQUEST)) {
                    conn->cb->transmitRequest(conn->currentRequest->request, (_CFNetConnectionRef)conn, conn->info);
                }
            }
        } 
        if (conn->responseStream && conn->cb->runLoopAndModesArrayForRequest && conn->currentRequest != conn->currentResponse && nextRealRequest(conn->currentResponse) == newReq) {
            rescheduleStream(conn->responseStream, NULL, conn->cb->runLoopAndModesArrayForRequest(newReq->request, (_CFNetConnectionRef)conn, conn->info));
            if (!__CFBitIsSet(conn->flags, CURRENT_RESPONSE_COMPLETE)) {
                conn->cb->receiveResponse(conn->currentResponse->request, (_CFNetConnectionRef)conn, conn->info);
            }
        }
        
        conn->count++;
		
        result = TRUE;
    }

#if defined(LOG_CONNECTIONS)
    fprintf(stderr, "-- %s returned\n", !result ? "FALSE" : "TRUE");
#endif

	_CFNetConnectionUnlock(conn);
    
    CFRelease(conn);

    return result;
}

static void rescheduleStream(CFTypeRef stream, CFArrayRef oldRLArray, CFArrayRef newRLArray) {
    CFArrayRef scheduleArray = NULL, unscheduleArray = NULL;
    Boolean isReadStream = (CFGetTypeID(stream) == CFReadStreamGetTypeID());
    CFIndex idx, cnt = 0;
    if (!oldRLArray) {
        unscheduleArray = NULL;
        scheduleArray = newRLArray;
        if (scheduleArray) CFRetain(scheduleArray);
    } else if (!newRLArray) {
        scheduleArray = NULL;
        unscheduleArray = oldRLArray;
        CFRetain(unscheduleArray);
        cnt = CFArrayGetCount(oldRLArray);
    } else {
        CFMutableArrayRef mutArray = NULL;
        CFIndex newRLIdx, oldRLIdx;
        CFIndex newRLCnt, oldRLCnt;
        newRLCnt = CFArrayGetCount(newRLArray);
        oldRLCnt = CFArrayGetCount(oldRLArray);
        
        // Index over newRLArray and find what does not appear in oldRLArray; this becomes the schedule array
        for (newRLIdx = 0; newRLIdx < newRLCnt; newRLIdx += 2) {
            CFRunLoopRef rl = (CFRunLoopRef)CFArrayGetValueAtIndex(newRLArray, newRLIdx);
            CFStringRef mode = CFArrayGetValueAtIndex(newRLArray, newRLIdx+1);
            Boolean foundIt = FALSE;
            CFRange searchRange;
            searchRange.location = 0;
            searchRange.length = oldRLCnt;
            while (searchRange.length > 1 && ((oldRLIdx = CFArrayGetFirstIndexOfValue(oldRLArray, searchRange, rl)) != kCFNotFound)) {
                if (oldRLIdx+1 < oldRLCnt && CFEqual(mode, CFArrayGetValueAtIndex(oldRLArray, oldRLIdx + 1))) {
                    foundIt = TRUE;
                    break;
                } else {
                    searchRange.location = oldRLIdx + 1;
                    searchRange.length = oldRLCnt - oldRLIdx - 1;
                }
            }
            if (!foundIt) {
                // Did not find a match in oldRLArray
                if (!mutArray) mutArray = CFArrayCreateMutable(CFGetAllocator(stream), 0, &kCFTypeArrayCallBacks);
                CFArrayAppendValue(mutArray, rl);
                CFArrayAppendValue(mutArray, mode);
            }
        }
        if (mutArray) scheduleArray = mutArray;
        mutArray = NULL;
        
        // Index over oldRLArray and find what does not appear in newRLArray; this becomes the unschedule array
        for (oldRLIdx = 0; oldRLIdx < oldRLCnt; oldRLIdx += 2) {
            CFRunLoopRef rl = (CFRunLoopRef)CFArrayGetValueAtIndex(oldRLArray, oldRLIdx);
            CFStringRef mode = CFArrayGetValueAtIndex(oldRLArray, oldRLIdx+1);
            Boolean foundIt = FALSE;
            CFRange searchRange;
            searchRange.location = 0;
            searchRange.length = newRLCnt;
            while (searchRange.length > 1 && ((newRLIdx = CFArrayGetFirstIndexOfValue(newRLArray, searchRange, rl)) != kCFNotFound)) {
                if (newRLIdx+1 < newRLCnt && CFEqual(mode, CFArrayGetValueAtIndex(newRLArray, newRLIdx + 1))) {
                    foundIt = TRUE;
                    break;
                } else {
                    searchRange.location = newRLIdx + 1;
                    searchRange.length = newRLCnt - newRLIdx - 1;
                }
            }
            if (!foundIt) {
                // Did not find a match in oldRLArray
                if (!mutArray) mutArray = CFArrayCreateMutable(CFGetAllocator(stream), 0, &kCFTypeArrayCallBacks);
                CFArrayAppendValue(mutArray, rl);
                CFArrayAppendValue(mutArray, mode);
            }
        }
        if (mutArray) unscheduleArray = mutArray;
    }

    if (scheduleArray) {
        cnt = CFArrayGetCount(scheduleArray);
        for (idx = 0; idx < cnt; idx += 2) {
            CFRunLoopRef rl = (CFRunLoopRef)CFArrayGetValueAtIndex(scheduleArray, idx);
            CFStringRef mode = CFArrayGetValueAtIndex(scheduleArray, idx+1);
            if (isReadStream) {
                CFReadStreamScheduleWithRunLoop((CFReadStreamRef)stream, rl, mode);
            } else {
                CFWriteStreamScheduleWithRunLoop((CFWriteStreamRef)stream, rl, mode);
            }
        }
        CFRelease(scheduleArray);
    }
    if (unscheduleArray) {
        cnt = CFArrayGetCount(unscheduleArray);
        for (idx = 0; idx < cnt; idx += 2) {
            CFRunLoopRef rl = (CFRunLoopRef)CFArrayGetValueAtIndex(unscheduleArray, idx);
            CFStringRef mode = CFArrayGetValueAtIndex(unscheduleArray, idx+1);
            if (isReadStream) {
                CFReadStreamUnscheduleFromRunLoop((CFReadStreamRef)stream, rl, mode);
            } else {
                CFWriteStreamUnscheduleFromRunLoop((CFWriteStreamRef)stream, rl, mode);
            }
        }
        CFRelease(unscheduleArray);
    }
}

static CFArrayRef runLoopsAndModesForRequest(__CFNetConnection *conn, _CFNetRequest *origReq) {
    _CFNetRequest *req = origReq;
    if (!origReq) return NULL;
    while (req && isZombieRequest(req)) {
        req = req->next;
    }
    if (!req) req = origReq;
    return conn->cb->runLoopAndModesArrayForRequest(req->request, (_CFNetConnectionRef)conn, conn->info);
}

/* Call from within a lock to change the request scheduled for transmission.  priorRequest should be
the request currently on requestStream (if any); newRequest should be the request too be put on 
the requestStream (if any).  priorRequest will be moved to state kWaitingForResponse; 
newRequest will be moved to state kTransmittingRequest */
static void scheduleNewRequest(__CFNetConnection* conn, _CFNetRequest *newRequest, _CFNetRequest *priorRequest, Boolean priorRequestIsNewResponse) {
    Boolean firstRequest = __CFBitIsSet(conn->flags, FIRST_REQUEST_SENT) ? FALSE : TRUE;
    
    // Note that we must schedule first, then open, then signal our requests.  This guarantees that no events are lost
    if ((!priorRequest || __CFBitIsSet(conn->flags, SHOULD_PIPELINE)) && conn->cb->runLoopAndModesArrayForRequest && conn->requestStream) {
        CFArrayRef newRLArray, oldRLArray/*, testArray*/;
        newRLArray = runLoopsAndModesForRequest(conn, newRequest);
        oldRLArray = runLoopsAndModesForRequest(conn, priorRequest);
        rescheduleStream(conn->requestStream, oldRLArray, newRLArray);
    }
    
    if (firstRequest) {
        __CFBitSet(conn->flags, FIRST_REQUEST_SENT);
        openConnectionStreams(conn);
    } else if (!__CFBitIsSet(conn->flags, CLIENT_IS_SET)) {
        _ConnectionSetClient(conn, TRUE);
    }

    if (priorRequest) {
        conn->cb->requestStateChanged(priorRequest->request, kWaitingForResponse, NULL, (_CFNetConnectionRef)conn, conn->info);
        __CFBitClear(conn->flags, TRANSMITTING_CURRENT_REQUEST);
        if (priorRequestIsNewResponse) {
            scheduleNewResponse(conn, priorRequest, NULL);
        }
    }
    if (newRequest) {
        __CFBitSet(conn->flags, TRANSMITTING_CURRENT_REQUEST);
        conn->cb->requestStateChanged(newRequest->request, kTransmittingRequest, NULL, (_CFNetConnectionRef)conn, conn->info);
    }
}

/* Call from within a lock to change the request scheduled to receive its response.  priorRequest 
should be the request currently on responseStream (if any); newRequest should be the request to
be put on the responseStream (if any).  priorRequest will be moved to state kFinished; newRequest
will be moved to state kReceivingResponse */
static void scheduleNewResponse(__CFNetConnection* conn, _CFNetRequest *newRequest, _CFNetRequest *priorRequest) {
    if (conn->cb->runLoopAndModesArrayForRequest && conn->responseStream) {
        CFArrayRef newRLArray, oldRLArray/*, testArray*/;
        newRLArray = runLoopsAndModesForRequest(conn, newRequest);
        oldRLArray = runLoopsAndModesForRequest(conn, priorRequest);
        rescheduleStream(conn->responseStream, oldRLArray, newRLArray);
    }
    
    if (priorRequest) {
        conn->cb->requestStateChanged(priorRequest->request, kFinished, NULL, (_CFNetConnectionRef)conn, conn->info);
    }
    if (newRequest) {
        conn->cb->requestStateChanged(newRequest->request, kReceivingResponse, NULL, (_CFNetConnectionRef)conn, conn->info);
    }
}

static void schedulePipelinedTransition(__CFNetConnection *conn, _CFNetRequest *new, _CFNetRequest *old) {
    Boolean firstRequest = __CFBitIsSet(conn->flags, FIRST_REQUEST_SENT) ? FALSE : TRUE;

    // Note that we must schedule first, then open, then signal our requests.  This guarantees that no events are lost
    if (conn->cb->runLoopAndModesArrayForRequest && conn->requestStream) {
        CFArrayRef oldScheduleArray = runLoopsAndModesForRequest(conn, old);
        CFArrayRef newScheduleArray = runLoopsAndModesForRequest(conn, new);
//        printf("schedule[Non]PipelinedTransition - conn %x:\n", (unsigned)conn);
        rescheduleStream(conn->requestStream, oldScheduleArray, newScheduleArray);
        rescheduleStream(conn->responseStream, oldScheduleArray, newScheduleArray);
    }
    if (firstRequest) {
        __CFBitSet(conn->flags, FIRST_REQUEST_SENT);
        openConnectionStreams(conn);
    } else if (!__CFBitIsSet(conn->flags, CLIENT_IS_SET)) {
        _ConnectionSetClient(conn, TRUE);
    }
    if (old) {
        conn->cb->requestStateChanged(old->request, kFinished, NULL, (_CFNetConnectionRef)conn, conn->info);
        __CFBitClear(conn->flags, TRANSMITTING_CURRENT_REQUEST);
    }
    if (new) {
        __CFBitSet(conn->flags, TRANSMITTING_CURRENT_REQUEST);
        conn->cb->requestStateChanged(new->request, kTransmittingRequest, NULL, (_CFNetConnectionRef)conn, conn->info);
    }
}

// Once we invoke the requestStateChanged() callback, the queue could be completely mucked with - 
// possibly many request could dequeue, and any dequeued request should not be messaged.  So we mark the
// requests we intend to message, and then keep traversing the list, messaging any that are still there.
static void sendStateChanged(__CFNetConnection *conn, _CFNetRequest *firstRecipient, int newState, CFStreamError *err) {
    Boolean done = FALSE;
    _CFNetRequest *req = firstRecipient;
    while (req) {
        __CFBitSet(req->flags, MARKED_REQUEST);
        req = req->next;
    }
    
    while (!done) {
        req = conn->head;
        while (req) {
            if (isMarkedRequest(req)) {
                __CFBitClear(req->flags, MARKED_REQUEST);
                conn->cb->requestStateChanged(req->request, newState, err, (_CFNetConnectionRef)conn, conn->info);
                break;
            }
            req = req->next;
        }
        if (!req) done = TRUE;
    }
}

/* Errors out the connection; orphans all requests.  After this call, the connection is dead in the water  */
void _CFNetConnectionErrorOccurred(_CFNetConnectionRef arg, CFStreamError *err) {
    _CFNetRequest *req;
    CFAllocatorRef alloc;
    __CFNetConnection* conn = (__CFNetConnection*)arg;
    
    CFRetain(arg);

	_CFNetConnectionLock(conn);
#if defined(LOG_CONNECTIONS)
    fprintf(stderr, "-- CFNetConnectionErrorOccurred(0x%x, {%d, %d})\n", (int)conn, (int)(err->domain), (int)(err->error));
#endif
    __CFBitClear(conn->flags, ACCEPTS_NEW_REQUESTS);
    
    /* Orphan all queued requests */
    alloc = CFGetAllocator(conn);
    req = conn->currentResponse;
    conn->currentRequest = NULL;
    conn->currentResponse = NULL;

    sendStateChanged(conn, req, kOrphaned, err);

    /* Dispose of the old streams */
    shutdownConnectionStreams(conn);

	_CFNetConnectionUnlock(conn);

    CFRelease(arg);
}

// Orphan queued requests except the current response with {Posix, ECONNRESET}.  Stop accepting new connections.  Do not attempt a clean shutdown

void _CFNetConnectionLost(_CFNetConnectionRef arg) {
    __CFNetConnection* conn = (__CFNetConnection*)arg;
    
    CFRetain(arg);

	_CFNetConnectionLock(conn);
#if defined(LOG_CONNECTIONS)
    fprintf(stderr, "-- CFNetConnectionLost(0x%x)\n", (int)conn);
#endif
    __CFBitClear(conn->flags, ACCEPTS_NEW_REQUESTS);
    __CFBitSet(conn->flags, CONNECTION_LOST);
    if (conn->currentResponse && conn->currentResponse->next != NULL) {
        CFStreamError err = {kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPConnectionLost};
        _CFNetRequest *req = conn->currentResponse->next;
        _CFNetRequest *wasCurrentReq = NULL;
        if (conn->currentRequest && conn->currentRequest != conn->currentResponse) {
            wasCurrentReq = conn->currentRequest;
            conn->currentRequest = NULL;
        }
        if (wasCurrentReq) {
            scheduleNewRequest(conn, NULL, wasCurrentReq, FALSE);
        }
        sendStateChanged(conn, req, kOrphaned, &err);
    }
	_CFNetConnectionUnlock(conn);
    
    CFRelease(arg);
}

// Cancel an enqueued request
Boolean _CFNetConnectionDequeue(_CFNetConnectionRef arg, void *req) {

    _CFNetRequest *removedRequest = NULL;
    __CFNetConnection* conn = (__CFNetConnection*)arg;
    
    CFRetain(arg);

	_CFNetConnectionLock(conn);
    
#if defined(LOG_CONNECTIONS)
    fprintf(stderr, "-- CFNetConnectionDequeue(0x%x, 0x%x)\n", (int)conn, (int)req);
#endif
    
    // Dequeueing is a little tricky.
    //
    // 1. If there is nothing in the queue, don't attempt.
    // 2. If the request appears before the current response, it may be dequeued
    // 3. If the request is the current response, it may not be dequeued
    // 4. If the request appears anywhere after the current response && CONNECTION_LOST, it may be dequeued
    // 5. If the request appears between current response & current request, it may not be dequeued
    // 6. If the request is the current request, it may be dequeued iff its transmission has not yet begun
    // 7. If the request appears after the current request, it may be dequeued
    
    // 1. If there is nothing in the queue, don't attempt.
    if (conn->head) {
        _CFNetRequest *pre = NULL, *match = conn->head;
        Boolean pastCurrentRequest = FALSE;
        Boolean pastCurrentResponse = FALSE;
        Boolean removeMatch;
    
        while (match && match->request != req) {
            if (match == conn->currentResponse) {
                pastCurrentResponse = TRUE;
            } 
            if (match == conn->currentRequest) {
                pastCurrentRequest = TRUE;
            }
            pre = match;
            match = match->next;
        }
    
        if (match) {
            if (!pastCurrentResponse) {
                if (match != conn->currentResponse) {
                    // 2. If the request appears before the current response, it may be dequeued
                    removeMatch = TRUE;
                } else {
                    // 3. If the request is the current response, it may not be dequeued
                    removeMatch = FALSE;
                }
            } else if (__CFBitIsSet(conn->flags, CONNECTION_LOST)) {
                // 4. If the request appears anywhere after the current response && CONNECTION_LOST, it may be dequeued
                removeMatch = TRUE;
            } else if (!pastCurrentRequest) {
                if (match != conn->currentRequest) {
                    // 5. If the request appears between current response & current request, it may not be dequeued
                    removeMatch = FALSE;
                } else {
                    // 6. If the request is the current request, it may be dequeued iff its transmission has not yet begun
                    removeMatch = !(__CFBitIsSet(conn->flags, TRANSMITTING_CURRENT_REQUEST));
                    if (removeMatch) conn->currentRequest = match->next;
                }
            } else {
                // 7. If the request appears after the current request, it may be dequeued
                removeMatch = TRUE;
            }
            if (removeMatch) {
                removedRequest = match;
                if (pre) {
                    pre->next = match->next;
                } else {
                    conn->head = match->next;
                }
                match->next = NULL;
                if (conn->tail == match) conn->tail = pre;
            }
        }
    }

    if (removedRequest) {
        CFAllocatorDeallocate(CFGetAllocator(conn), removedRequest);
        if (!conn->head) {
            conn->emptyTime = CFAbsoluteTimeGetCurrent();
        }
    }

	if (removedRequest)
		conn->count--;
	
    _CFNetConnectionUnlock(conn);
    
    CFRelease(arg);
    
    return (removedRequest ? TRUE : FALSE);
}


void* _CFNetConnectionGetCurrentRequest(_CFNetConnectionRef arg) {
    
    void* result;
    __CFNetConnection* conn = (__CFNetConnection*)arg;
    
	_CFNetConnectionLock(conn);
    
    result = conn->currentRequest ? conn->currentRequest->request : NULL;

	_CFNetConnectionUnlock(conn);
    
    return result;
}

int _CFNetConnectionGetQueueDepth(_CFNetConnectionRef arg) {
    __CFNetConnection* conn = (__CFNetConnection*)arg;
    int result;

	_CFNetConnectionLock(conn);
	
	result = conn->count;
//    if (conn->currentResponse) {
//        _CFNetRequest *req = conn->currentResponse;
//        result = 1;
//        while (req->next != NULL) {
//            result ++;
//            req = req->next;
//        }
//    } else {
//        result = 0;
//    }
	_CFNetConnectionUnlock(conn);
    return result;
}


void _CFNetConnectionReplaceRequest(_CFNetConnectionRef arg, void *oldReq, void *newReq) {

    __CFNetConnection* conn = (__CFNetConnection*)arg;
    CFArrayRef oldRLArray = NULL;
    CFArrayRef newRLArray = NULL;
    _CFNetRequest *myNetRequest = NULL;
    Boolean rescheduleRequestStream = FALSE;
    Boolean rescheduleResponseStream = FALSE;
    
    _CFNetConnectionLock(conn);

    if (conn->currentRequest && __CFBitIsSet(conn->flags, TRANSMITTING_CURRENT_REQUEST)) {
        _CFNetRequest *nextRealReq = nextRealRequest(conn->currentRequest);
        if (nextRealReq->request == oldReq) {
            rescheduleRequestStream = TRUE;
            myNetRequest = nextRealReq;
        }
    }
    if (conn->currentResponse && !__CFBitIsSet(conn->flags, CURRENT_RESPONSE_COMPLETE)) {
        _CFNetRequest *nextRealReq = nextRealRequest(conn->currentResponse);
        if (nextRealReq->request == oldReq) {
            rescheduleResponseStream = TRUE;
            myNetRequest = nextRealReq;
        }
    }
    if (conn->cb->runLoopAndModesArrayForRequest && (rescheduleRequestStream || rescheduleResponseStream)) {
        oldRLArray = conn->cb->runLoopAndModesArrayForRequest(oldReq, (_CFNetConnectionRef)conn, conn->info);
        if (myNetRequest->next) {
            _CFNetRequest *nextRealReq = nextRealRequest(myNetRequest->next);
            newRLArray = conn->cb->runLoopAndModesArrayForRequest(nextRealReq->request, (_CFNetConnectionRef)conn, conn->info);
        } else {
            newRLArray = NULL;
        }
    }
    
    replaceInList(&conn->head, &conn->tail, oldReq, newReq);
    if (rescheduleRequestStream) {
        rescheduleStream(conn->requestStream, oldRLArray, newRLArray);
        if (conn->currentRequest && conn->currentRequest->request == newReq && __CFBitIsSet(conn->flags, TRANSMITTING_CURRENT_REQUEST)) {
            conn->cb->transmitRequest(conn->currentRequest->request, (_CFNetConnectionRef)conn, conn->info);
        }
    } else if (rescheduleResponseStream) {
        rescheduleStream(conn->responseStream, oldRLArray, newRLArray);
        if (conn->currentResponse && conn->currentResponse->request == newReq && !__CFBitIsSet(conn->flags, CURRENT_RESPONSE_COMPLETE)) {
            conn->cb->receiveResponse(conn->currentResponse->request, (_CFNetConnectionRef)conn, conn->info);
        }
    }
	_CFNetConnectionUnlock(conn);
}


CFReadStreamRef _CFNetConnectionGetResponseStream(_CFNetConnectionRef arg) {
    
    CFReadStreamRef responseStream = NULL;
    __CFNetConnection* conn = (__CFNetConnection*)arg;

	_CFNetConnectionLock(conn);
    responseStream = conn->responseStream;
	_CFNetConnectionUnlock(conn);
#if LOG_CONNECTIONS
    fprintf(stderr, "-- CFNetConnectionGetResponseStream(0x%x) - 0x%x returned\n", (int)conn, (int)responseStream);
#endif
    return responseStream;
}

CFWriteStreamRef _CFNetConnectionGetRequestStream(_CFNetConnectionRef arg) {
    
    CFWriteStreamRef requestStream = NULL;
    __CFNetConnection* conn = (__CFNetConnection*)arg;

	_CFNetConnectionLock(conn);
    requestStream = conn->requestStream;
	_CFNetConnectionUnlock(conn);
#if defined(LOG_CONNECTIONS)
    fprintf(stderr, "-- CFNetConnectionGetRequestStream(0x%x) - 0x%x returned\n", (int)conn, (int)requestStream);
#endif
    return requestStream;
}


void _CFNetConnectionSetAllowsNewRequests(_CFNetConnectionRef arg, Boolean allowRequests) {
    
    __CFNetConnection* conn = (__CFNetConnection*)arg;

	_CFNetConnectionLock(conn);
    if (allowRequests) {
        __CFBitSet(conn->flags, ACCEPTS_NEW_REQUESTS);
    } else {
        __CFBitClear(conn->flags, ACCEPTS_NEW_REQUESTS);
    }
	_CFNetConnectionUnlock(conn);
}

Boolean _CFNetConnectionIsEmpty(_CFNetConnectionRef arg) {
    __CFNetConnection* conn = (__CFNetConnection*)arg;
    Boolean result;
    
	_CFNetConnectionLock(conn);
    result = (conn->head == NULL);
	_CFNetConnectionUnlock(conn);
    return result;
}

Boolean _CFNetConnectionWillEnqueueRequests(_CFNetConnectionRef arg) {
    
    Boolean result;
    __CFNetConnection* conn = (__CFNetConnection*)arg;

	_CFNetConnectionLock(conn);
    result = __CFBitIsSet(conn->flags, ACCEPTS_NEW_REQUESTS);
	_CFNetConnectionUnlock(conn);
    return result;
}


void _CFNetConnectionSetShouldPipeline(_CFNetConnectionRef arg, Boolean shouldPipeline) {

    __CFNetConnection* conn = (__CFNetConnection*)arg;

	_CFNetConnectionLock(conn);
#if defined(LOG_CONNECTIONS)
    fprintf(stderr, "-- CFNetConnectionSetShouldPipeline(0x%x, %s)\n", (int)conn, shouldPipeline ? "TRUE" : "FALSE");
#endif
    if (shouldPipeline && !__CFBitIsSet(conn->flags, SHOULD_PIPELINE)) {
        __CFBitSet(conn->flags, SHOULD_PIPELINE);
        if (conn->currentRequest && !__CFBitIsSet(conn->flags, TRANSMITTING_CURRENT_REQUEST)) {
            scheduleNewRequest(conn, conn->currentRequest, conn->currentResponse, FALSE);
        }
    }
    if (!shouldPipeline && __CFBitIsSet(conn->flags, SHOULD_PIPELINE)) {
        __CFBitClear(conn->flags, SHOULD_PIPELINE);
    }
	_CFNetConnectionUnlock(conn);
}

// Currently not used, so hand dead-stripping for now.
//Boolean _CFNetConnectionIsPipelining(_CFNetConnectionRef arg) {
//    __CFNetConnection* conn = (__CFNetConnection*)arg;
//    Boolean result;
//
//	_CFNetConnectionLock(conn);
//    result = __CFBitIsSet(conn->flags, SHOULD_PIPELINE);
//	_CFNetConnectionUnlock(conn);
//
//    return result;
//}

CFAbsoluteTime _CFNetConnectionGetLastAccessTime(_CFNetConnectionRef arg) {
    __CFNetConnection* conn = (__CFNetConnection*)arg;
	CFAbsoluteTime result;
	_CFNetConnectionLock(conn);
	if (conn->head) {
		result = CFAbsoluteTimeGetCurrent();
	} else {
		result = conn->emptyTime;
	}
	_CFNetConnectionUnlock(conn);
	return result;
}

// Informs the connection that the given request considers its response complete, and the connection should break its connection to the request and advance to the next response
void _CFNetConnectionResponseIsComplete(_CFNetConnectionRef arg, void *req) {

    __CFNetConnection* conn = (__CFNetConnection*)arg;

    CFRetain(conn);

	_CFNetConnectionLock(conn);
#if defined(LOG_CONNECTIONS) 
    fprintf(stderr, "-- CFNetConnectionResponseIsComplete(0x%x, 0x%x)\n", (int)conn, (int)req);
#endif
    if (conn->currentResponse && req == conn->currentResponse->request) {
        // Advance to the next response; do this before signalling the state change so that the request can destroy itself if it wishes.
        _CFNetRequest *oldResponse = conn->currentResponse;
        _CFNetRequest *newResponse = NULL;
        Boolean didNonPipelinedTransition = FALSE;
        if (conn->currentResponse == conn->currentRequest) {
            // Do not allow currentResponse to pass currentRequest
            __CFBitSet(conn->flags, CURRENT_RESPONSE_COMPLETE);
        } else {
            conn->currentResponse = conn->currentResponse->next;
            if (conn->currentResponse) {
                if (__CFBitIsSet(conn->flags, CONNECTION_LOST)) {
                    // The connection was lost while finishing this response; do no start receiving the next one
                    conn->currentResponse = NULL;
                } else if (conn->currentResponse != conn->currentRequest) {
                    // Only start the next response if its  request is done transmitting
                    newResponse = conn->currentResponse;
                } else if (!__CFBitIsSet(conn->flags, SHOULD_PIPELINE) && !__CFBitIsSet(conn->flags, TRANSMITTING_CURRENT_REQUEST)) {
                    // Haven't been transmitting currentRequest; start doing so now
                    // In the non-pipelining case, we must ensure that the former response is completely off the line
                    // before we start the new request, so we need to call scheduleNewResponse first.  This is a 
                    // performance hit we need to work out....
                    didNonPipelinedTransition = TRUE;
                    schedulePipelinedTransition(conn, conn->currentResponse, oldResponse);
                }
            }
            if (!didNonPipelinedTransition) {
                scheduleNewResponse(conn, newResponse, oldResponse);
            }
        }
    }
	_CFNetConnectionUnlock(conn);
    CFRelease(conn);
}

// Informs the connection that the given request considers its request complete (i.e. fully transmitted), and the connection should advance to the next request to be transmitted
void _CFNetConnectionRequestIsComplete(_CFNetConnectionRef arg, void *req) {

    __CFNetConnection* conn = (__CFNetConnection*)arg;
    
    CFRetain(conn);

	_CFNetConnectionLock(conn);
#if defined(LOG_CONNECTIONS)
    fprintf(stderr, "-- CFNetConnectionRequestIsComplete(0x%x, 0x%x)\n", (int)conn, (int)req);
#endif
    if (conn->currentRequest && req == conn->currentRequest->request) {
        _CFNetRequest *formerRequest = conn->currentRequest;
        Boolean currResponseComplete = __CFBitIsSet(conn->flags, CURRENT_RESPONSE_COMPLETE);
        Boolean formerRequestIsNewResponse = (conn->currentResponse->request == req && !currResponseComplete);
        if (!__CFBitIsSet(conn->flags, CONNECTION_LOST)) {
            conn->currentRequest = conn->currentRequest->next;
            if (conn->currentRequest && __CFBitIsSet(conn->flags, SHOULD_PIPELINE)) {
                scheduleNewRequest(conn, conn->currentRequest, formerRequest, formerRequestIsNewResponse);
            } else {
                scheduleNewRequest(conn, NULL, formerRequest, formerRequestIsNewResponse);
            }
        } else {
            conn->currentRequest = NULL;
            scheduleNewRequest(conn, NULL, formerRequest, formerRequestIsNewResponse);
        }
        if (currResponseComplete) {
            _CFNetConnectionResponseIsComplete(arg, formerRequest->request);
            __CFBitClear(conn->flags, CURRENT_RESPONSE_COMPLETE);
        }
    } else {
        // Should never get here
        CFLog(__kCFLogAssertion, CFSTR("request (0x%x) other than the current request signalled it was complete on connection 0x%x\n"), req, conn);
    }
	_CFNetConnectionUnlock(conn);
    CFRelease(conn);
}

// Gets the connection's current opinion about the request's state.  Calling this will cause the connection to attempt to further the state of its queue, and may cause calls back in to the request.  If the connection knows nothing about the request, it will return kOrphaned, and the calling request should forget any tie to this connection
int _CFNetConnectionGetState(_CFNetConnectionRef arg, Boolean advanceConnection, void *req) {
    
    int result;
    __CFNetConnection* conn = (__CFNetConnection*)arg;

    CFRetain(conn);

	_CFNetConnectionLock(conn);
#if defined(LOG_CONNECTIONS)
    fprintf(stderr, "-- CFNetConnectionGetState(0x%x, 0x%x) - ", (int)conn, (int)req);
#endif
    if (advanceConnection) {
        if (conn->currentRequest && __CFBitIsSet(conn->flags, TRANSMITTING_CURRENT_REQUEST)) {
            conn->cb->transmitRequest(conn->currentRequest->request, (_CFNetConnectionRef)conn, conn->info);
        }
        // It's possible the work above caused us to shutdown; testing conn->currentResponse tests for that.
        if (conn->currentResponse && ((conn->currentResponse != conn->currentRequest) || !__CFBitIsSet(conn->flags, TRANSMITTING_CURRENT_REQUEST))) {
            conn->cb->receiveResponse(conn->currentResponse->request, (_CFNetConnectionRef)conn, conn->info);
        }
    }
    if (!conn->requestStream && !conn->responseStream) {
        // We've been shut down (possibly as a result of the calls to transmitRequest or receiveResponse, above), and the caller hasn't noticed yet
        result = kOrphaned;
    } else if (conn->currentRequest && req == conn->currentRequest->request) {
        if (__CFBitIsSet(conn->flags, TRANSMITTING_CURRENT_REQUEST)) {
            result = kTransmittingRequest;
        } else {
            result = kQueued;
        }
    
    // It's possible that calling receiveResponse actually emptied the
    // queues, so need to make sure head is still good.
    } else if (conn->currentResponse && (req == conn->currentResponse->request)) {
        result = kReceivingResponse;
    } else if (isInList(conn->currentRequest, req)) {
        result = __CFBitIsSet(conn->flags, CONNECTION_LOST) ? kOrphaned : kQueued;
    } else if (isInList(conn->currentResponse, req)) {
        result = __CFBitIsSet(conn->flags, CONNECTION_LOST) ? kOrphaned : kWaitingForResponse;
    } else if (isInList(conn->head, req)) {
        result = kFinished;
    } else {
        result = kOrphaned;
    }
#if defined(LOG_CONNECTIONS)
    fprintf(stderr, "-- %d returned\n", result);
#endif
	_CFNetConnectionUnlock(conn);
    CFRelease(conn);
    return result;
}

// Informs the connection that the given request has been scheduled and asks the connection to take any appropriate action
void _CFNetConnectionSchedule(_CFNetConnectionRef arg, void *req, CFRunLoopRef rl, CFStringRef mode) {

    __CFNetConnection* conn = (__CFNetConnection*)arg;
    _CFNetRequest *theRequest;
    
    _CFNetConnectionLock(conn);

    if (conn->currentRequest && conn->requestStream) {
        theRequest = nextRealRequest(conn->currentRequest); 
        if (theRequest->request == req) {
            CFWriteStreamScheduleWithRunLoop(conn->requestStream, rl, mode);
        }
    }

    if (conn->currentResponse && conn->responseStream) {
        theRequest = nextRealRequest(conn->currentResponse);
        if (theRequest->request == req) {
            CFReadStreamScheduleWithRunLoop(conn->responseStream, rl, mode);
        }
    }

    _CFNetConnectionUnlock(conn);
}

// Informs the connection that the given request has been unscheduled, and asks the connection to take any appropriate action
void _CFNetConnectionUnschedule(_CFNetConnectionRef arg, void *req, CFRunLoopRef rl, CFStringRef mode) {

    __CFNetConnection* conn = (__CFNetConnection*)arg;
    _CFNetRequest *theRequest;

    _CFNetConnectionLock(conn);

    if (conn->requestStream) {
        if (conn->currentRequest) {
            theRequest = nextRealRequest(conn->currentRequest); 
        } else if (!__CFBitIsSet(conn->flags, SHOULD_PIPELINE) && conn->currentResponse) {
            theRequest = nextRealRequest(conn->currentResponse);
        } else {
            theRequest = NULL;
        }
        if (theRequest && theRequest->request == req) {
            CFWriteStreamUnscheduleFromRunLoop(conn->requestStream, rl, mode);
        }
    }

    if (conn->currentResponse && conn->responseStream) {
        theRequest = nextRealRequest(conn->currentResponse);
        if (theRequest->request == req) {
            CFReadStreamUnscheduleFromRunLoop(conn->responseStream, rl, mode);
        }
    }
    
    _CFNetConnectionUnlock(conn);
}
