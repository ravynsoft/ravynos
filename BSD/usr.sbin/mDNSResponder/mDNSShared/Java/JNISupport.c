/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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

    This file contains the platform support for DNSSD and related Java classes.
    It is used to shim through to the underlying <dns_sd.h> API.
 */

// AUTO_CALLBACKS should be set to 1 if the underlying mDNS implementation fires response
// callbacks automatically (as in the early Windows prototypes).
// AUTO_CALLBACKS should be set to 0 if the client must call DNSServiceProcessResult() to
// invoke response callbacks (as is true on Mac OS X, Posix, Windows, etc.).
// (Invoking callbacks automatically on a different thread sounds attractive, but while
// the client gains by not needing to add an event source to its main event loop, it loses
// by being forced to deal with concurrency and locking, which can be a bigger burden.)
#ifndef AUTO_CALLBACKS
#define AUTO_CALLBACKS  0
#endif

#if !AUTO_CALLBACKS
#ifdef _WIN32
#include <winsock2.h>
#else //_WIN32
#include <sys/types.h>
#include <sys/select.h>
#endif // _WIN32
#endif // AUTO_CALLBACKS

#include <dns_sd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h>
static char *   win32_if_indextoname( DWORD ifIndex, char * nameBuff);
static DWORD    win32_if_nametoindex( const char * nameStr );
#define if_indextoname win32_if_indextoname
#define if_nametoindex win32_if_nametoindex
#define IF_NAMESIZE MAX_ADAPTER_NAME_LENGTH
#else // _WIN32
#include <sys/socket.h>
#include <net/if.h>
#endif // _WIN32

// When compiling with "-Wshadow" set, including jni.h produces the following error:
// /System/Library/Frameworks/JavaVM.framework/Versions/A/Headers/jni.h:609: warning: declaration of 'index' shadows a global declaration
// To work around this, we use the preprocessor to map the identifier 'index', which appears harmlessly in function prototype declarations,
// to something 'jni_index', which doesn't conflict
#define index jni_index
#include "DNSSD.java.h"
#undef index

//#include <syslog.h>

// convenience definition
#ifdef __GNUC__
#define _UNUSED __attribute__ ((unused))
#else
#define _UNUSED
#endif

enum {
    kInterfaceVersionOne = 1,
    kInterfaceVersionCurrent        // Must match version in .jar file
};

typedef struct OpContext OpContext;

struct  OpContext
{
    DNSServiceRef ServiceRef;
    JNIEnv          *Env;
    jobject JavaObj;
    jobject ClientObj;
    jmethodID Callback;
    jmethodID Callback2;
};

// For AUTO_CALLBACKS, we must attach the callback thread to the Java VM prior to upcall.
#if AUTO_CALLBACKS
JavaVM      *gJavaVM = NULL;
#endif


JNIEXPORT jint JNICALL Java_com_apple_dnssd_AppleDNSSD_InitLibrary( JNIEnv *pEnv, jclass cls,
                                                                    jint callerVersion)
{
    /* Ensure that caller & interface versions match. */
    if ( callerVersion != kInterfaceVersionCurrent)
        return kDNSServiceErr_Incompatible;

#if AUTO_CALLBACKS
    {
        jsize numVMs;

        if ( 0 != JNI_GetCreatedJavaVMs( &gJavaVM, 1, &numVMs))
            return kDNSServiceErr_BadState;
    }
#endif

    // Set AppleDNSSD.hasAutoCallbacks
    {
#if AUTO_CALLBACKS
        jboolean hasAutoC = JNI_TRUE;
#else
        jboolean hasAutoC = JNI_FALSE;
#endif
        jfieldID hasAutoCField = (*pEnv)->GetStaticFieldID( pEnv, cls, "hasAutoCallbacks", "Z");
        (*pEnv)->SetStaticBooleanField( pEnv, cls, hasAutoCField, hasAutoC);
    }

    return kDNSServiceErr_NoError;
}


static const char*  SafeGetUTFChars( JNIEnv *pEnv, jstring str)
// Wrapper for JNI GetStringUTFChars() that returns NULL for null str.
{
    return str != NULL ? (*pEnv)->GetStringUTFChars( pEnv, str, 0) : NULL;
}

static void         SafeReleaseUTFChars( JNIEnv *pEnv, jstring str, const char *buff)
// Wrapper for JNI GetStringUTFChars() that handles null str.
{
    if ( str != NULL)
        (*pEnv)->ReleaseStringUTFChars( pEnv, str, buff);
}


#if AUTO_CALLBACKS
static void SetupCallbackState( JNIEnv **ppEnv)
{
    (*gJavaVM)->AttachCurrentThread( gJavaVM, (void**) ppEnv, NULL);
}

static void TeardownCallbackState( void )
{
    (*gJavaVM)->DetachCurrentThread( gJavaVM);
}

#else   // AUTO_CALLBACKS

static void SetupCallbackState( JNIEnv **ppEnv _UNUSED)
{
    // No setup necessary if ProcessResults() has been called
}

static void TeardownCallbackState( void )
{
    // No teardown necessary if ProcessResults() has been called
}
#endif  // AUTO_CALLBACKS


static OpContext    *NewContext( JNIEnv *pEnv, jobject owner,
                                 const char *callbackName, const char *callbackSig)
// Create and initialize a new OpContext.
{
    OpContext               *pContext = (OpContext*) malloc( sizeof *pContext);

    if ( pContext != NULL)
    {
        jfieldID clientField = (*pEnv)->GetFieldID( pEnv, (*pEnv)->GetObjectClass( pEnv, owner),
                                                    "fListener", "Lcom/apple/dnssd/BaseListener;");

        pContext->JavaObj = (*pEnv)->NewWeakGlobalRef( pEnv, owner);    // must convert local ref to global to cache;
        pContext->ClientObj = (*pEnv)->GetObjectField( pEnv, owner, clientField);
        pContext->ClientObj = (*pEnv)->NewWeakGlobalRef( pEnv, pContext->ClientObj);    // must convert local ref to global to cache
        pContext->Callback = (*pEnv)->GetMethodID( pEnv,
                                                   (*pEnv)->GetObjectClass( pEnv, pContext->ClientObj),
                                                   callbackName, callbackSig);
        pContext->Callback2 = NULL;     // not always used
    }

    return pContext;
}


static void         ReportError( JNIEnv *pEnv, jobject target, jobject service, DNSServiceErrorType err)
// Invoke operationFailed() method on target with err.
{
    jclass cls = (*pEnv)->GetObjectClass( pEnv, target);
    jmethodID opFailed = (*pEnv)->GetMethodID( pEnv, cls, "operationFailed",
                                               "(Lcom/apple/dnssd/DNSSDService;I)V");

    (*pEnv)->CallVoidMethod( pEnv, target, opFailed, service, err);
}

JNIEXPORT void JNICALL Java_com_apple_dnssd_AppleService_HaltOperation( JNIEnv *pEnv, jobject pThis)
/* Deallocate the dns_sd service browser and set the Java object's fNativeContext field to 0. */
{
    jclass cls = (*pEnv)->GetObjectClass( pEnv, pThis);
    jfieldID contextField = (*pEnv)->GetFieldID( pEnv, cls, "fNativeContext", "J");

    if ( contextField != 0)
    {
        OpContext   *pContext = (OpContext*) (long) (*pEnv)->GetLongField(pEnv, pThis, contextField);
        if ( pContext != NULL)
        {
            // MUST clear fNativeContext first, BEFORE calling DNSServiceRefDeallocate()
            (*pEnv)->SetLongField(pEnv, pThis, contextField, 0);
            if ( pContext->ServiceRef != NULL)
                DNSServiceRefDeallocate( pContext->ServiceRef);

            (*pEnv)->DeleteWeakGlobalRef( pEnv, pContext->JavaObj);
            (*pEnv)->DeleteWeakGlobalRef( pEnv, pContext->ClientObj);
            free( pContext);
        }
    }
}


JNIEXPORT jint JNICALL Java_com_apple_dnssd_AppleService_BlockForData( JNIEnv *pEnv, jobject pThis)
/* Block until data arrives, or one second passes. Returns 1 if data present, 0 otherwise. */
{
// BlockForData() not supported with AUTO_CALLBACKS
#if !AUTO_CALLBACKS
    jclass cls = (*pEnv)->GetObjectClass( pEnv, pThis);
    jfieldID contextField = (*pEnv)->GetFieldID( pEnv, cls, "fNativeContext", "J");

    if ( contextField != 0)
    {
        OpContext   *pContext = (OpContext*) (long) (*pEnv)->GetLongField(pEnv, pThis, contextField);
        if ( pContext != NULL)
        {
            fd_set readFDs;
            int sd = DNSServiceRefSockFD( pContext->ServiceRef);
            struct timeval timeout = { 1, 0 };
            FD_ZERO( &readFDs);
            FD_SET( sd, &readFDs);

            // Q: Why do we poll here?
            // A: Because there's no other thread-safe way to do it.
            // Mac OS X terminates a select() call if you close one of the sockets it's listening on, but Linux does not,
            // and arguably Linux is correct (See <http://www.ussg.iu.edu/hypermail/linux/kernel/0405.1/0418.html>)
            // The problem is that the Mac OS X behaviour assumes that it's okay for one thread to close a socket while
            // some other thread is monitoring that socket in select(), but the difficulty is that there's no general way
            // to make that thread-safe, because there's no atomic way to enter select() and release a lock simultaneously.
            // If we try to do this without holding any lock, then right as we jump to the select() routine,
            // some other thread could stop our operation (thereby closing the socket),
            // and then that thread (or even some third, unrelated thread)
            // could do some other DNS-SD operation (or some other operation that opens a new file descriptor)
            // and then we'd blindly resume our fall into the select() call, now blocking on a file descriptor
            // that may coincidentally have the same numerical value, but is semantically unrelated
            // to the true file descriptor we thought we were blocking on.
            // We can't stop this race condition from happening, but at least if we wake up once a second we can detect
            // when fNativeContext has gone to zero, and thereby discover that we were blocking on the wrong fd.

            if (select( sd + 1, &readFDs, (fd_set*) NULL, (fd_set*) NULL, &timeout) == 1) return(1);
        }
    }
#endif // !AUTO_CALLBACKS
    return(0);
}


JNIEXPORT jint JNICALL Java_com_apple_dnssd_AppleService_ProcessResults( JNIEnv *pEnv, jobject pThis)
/* Call through to DNSServiceProcessResult() while data remains on socket. */
{
#if !AUTO_CALLBACKS // ProcessResults() not supported with AUTO_CALLBACKS

    jclass cls = (*pEnv)->GetObjectClass( pEnv, pThis);
    jfieldID contextField = (*pEnv)->GetFieldID( pEnv, cls, "fNativeContext", "J");
    OpContext       *pContext = (OpContext*) (long) (*pEnv)->GetLongField(pEnv, pThis, contextField);
    DNSServiceErrorType err = kDNSServiceErr_BadState;

    if ( pContext != NULL)
    {
        int sd = DNSServiceRefSockFD( pContext->ServiceRef);
        fd_set readFDs;
        struct timeval zeroTimeout = { 0, 0 };

        pContext->Env = pEnv;

        FD_ZERO( &readFDs);
        FD_SET( sd, &readFDs);

        err = kDNSServiceErr_NoError;
        if (0 < select(sd + 1, &readFDs, (fd_set*) NULL, (fd_set*) NULL, &zeroTimeout))
        {
            err = DNSServiceProcessResult(pContext->ServiceRef);
            // Use caution here!
            // We cannot touch any data structures associated with this operation!
            // The DNSServiceProcessResult() routine should have invoked our callback,
            // and our callback could have terminated the operation with op.stop();
            // and that means HaltOperation() will have been called, which frees pContext.
            // Basically, from here we just have to get out without touching any stale
            // data structures that could blow up on us! Particularly, any attempt
            // to loop here reading more results from the file descriptor is unsafe.
        }
    }
    return err;
#endif // AUTO_CALLBACKS
}


static void DNSSD_API   ServiceBrowseReply( DNSServiceRef sdRef _UNUSED, DNSServiceFlags flags, uint32_t interfaceIndex,
                                            DNSServiceErrorType errorCode, const char *serviceName, const char *regtype,
                                            const char *replyDomain, void *context)
{
    OpContext       *pContext = (OpContext*) context;

    SetupCallbackState( &pContext->Env);

    if ( pContext->ClientObj != NULL && pContext->Callback != NULL)
    {
        if ( errorCode == kDNSServiceErr_NoError)
        {
            (*pContext->Env)->CallVoidMethod( pContext->Env, pContext->ClientObj,
                                              ( flags & kDNSServiceFlagsAdd) != 0 ? pContext->Callback : pContext->Callback2,
                                              pContext->JavaObj, flags, interfaceIndex,
                                              (*pContext->Env)->NewStringUTF( pContext->Env, serviceName),
                                              (*pContext->Env)->NewStringUTF( pContext->Env, regtype),
                                              (*pContext->Env)->NewStringUTF( pContext->Env, replyDomain));
        }
        else
            ReportError( pContext->Env, pContext->ClientObj, pContext->JavaObj, errorCode);
    }

    TeardownCallbackState();
}

JNIEXPORT jint JNICALL Java_com_apple_dnssd_AppleBrowser_CreateBrowser( JNIEnv *pEnv, jobject pThis,
                                                                        jint flags, jint ifIndex, jstring regType, jstring domain)
{
    jclass cls = (*pEnv)->GetObjectClass( pEnv, pThis);
    jfieldID contextField = (*pEnv)->GetFieldID( pEnv, cls, "fNativeContext", "J");
    OpContext               *pContext = NULL;
    DNSServiceErrorType err = kDNSServiceErr_NoError;

    if ( contextField != 0)
        pContext = NewContext( pEnv, pThis, "serviceFound",
                               "(Lcom/apple/dnssd/DNSSDService;IILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    else
        err = kDNSServiceErr_BadParam;

    if ( pContext != NULL)
    {
        const char  *regStr = SafeGetUTFChars( pEnv, regType);
        const char  *domainStr = SafeGetUTFChars( pEnv, domain);

        pContext->Callback2 = (*pEnv)->GetMethodID( pEnv,
                                                    (*pEnv)->GetObjectClass( pEnv, pContext->ClientObj),
                                                    "serviceLost", "(Lcom/apple/dnssd/DNSSDService;IILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");

        err = DNSServiceBrowse( &pContext->ServiceRef, flags, ifIndex, regStr, domainStr, ServiceBrowseReply, pContext);
        if ( err == kDNSServiceErr_NoError)
        {
            (*pEnv)->SetLongField(pEnv, pThis, contextField, (long) pContext);
        }

        SafeReleaseUTFChars( pEnv, regType, regStr);
        SafeReleaseUTFChars( pEnv, domain, domainStr);
    }
    else
        err = kDNSServiceErr_NoMemory;

    return err;
}


static void DNSSD_API   ServiceResolveReply( DNSServiceRef sdRef _UNUSED, DNSServiceFlags flags, uint32_t interfaceIndex,
                                             DNSServiceErrorType errorCode, const char *fullname, const char *hosttarget,
                                             uint16_t port, uint16_t txtLen, const unsigned char *txtRecord, void *context)
{
    OpContext       *pContext = (OpContext*) context;
    jclass txtCls;
    jmethodID txtCtor;
    jbyteArray txtBytes;
    jobject txtObj;
    jbyte           *pBytes;

    SetupCallbackState( &pContext->Env);

    txtCls = (*pContext->Env)->FindClass( pContext->Env, "com/apple/dnssd/TXTRecord");
    txtCtor = (*pContext->Env)->GetMethodID( pContext->Env, txtCls, "<init>", "([B)V");

    if ( pContext->ClientObj != NULL && pContext->Callback != NULL && txtCtor != NULL &&
         NULL != ( txtBytes = (*pContext->Env)->NewByteArray( pContext->Env, txtLen)))
    {
        if ( errorCode == kDNSServiceErr_NoError)
        {
            // Since Java ints are defined to be big-endian, we canonicalize 'port' from a 16-bit
            // pattern into a number here.
            port = ( ((unsigned char*) &port)[0] << 8) | ((unsigned char*) &port)[1];

            // Initialize txtBytes with contents of txtRecord
            pBytes = (*pContext->Env)->GetByteArrayElements( pContext->Env, txtBytes, NULL);
            memcpy( pBytes, txtRecord, txtLen);
            (*pContext->Env)->ReleaseByteArrayElements( pContext->Env, txtBytes, pBytes, JNI_COMMIT);

            // Construct txtObj with txtBytes
            txtObj = (*pContext->Env)->NewObject( pContext->Env, txtCls, txtCtor, txtBytes);
            (*pContext->Env)->DeleteLocalRef( pContext->Env, txtBytes);

            (*pContext->Env)->CallVoidMethod( pContext->Env, pContext->ClientObj, pContext->Callback,
                                              pContext->JavaObj, flags, interfaceIndex,
                                              (*pContext->Env)->NewStringUTF( pContext->Env, fullname),
                                              (*pContext->Env)->NewStringUTF( pContext->Env, hosttarget),
                                              port, txtObj);
        }
        else
            ReportError( pContext->Env, pContext->ClientObj, pContext->JavaObj, errorCode);
    }

    TeardownCallbackState();
}

JNIEXPORT jint JNICALL Java_com_apple_dnssd_AppleResolver_CreateResolver( JNIEnv *pEnv, jobject pThis,
                                                                          jint flags, jint ifIndex, jstring serviceName, jstring regType, jstring domain)
{
    jclass cls = (*pEnv)->GetObjectClass( pEnv, pThis);
    jfieldID contextField = (*pEnv)->GetFieldID( pEnv, cls, "fNativeContext", "J");
    OpContext               *pContext = NULL;
    DNSServiceErrorType err = kDNSServiceErr_NoError;

    if ( contextField != 0)
        pContext = NewContext( pEnv, pThis, "serviceResolved",
                               "(Lcom/apple/dnssd/DNSSDService;IILjava/lang/String;Ljava/lang/String;ILcom/apple/dnssd/TXTRecord;)V");
    else
        err = kDNSServiceErr_BadParam;

    if ( pContext != NULL)
    {
        const char  *servStr = SafeGetUTFChars( pEnv, serviceName);
        const char  *regStr = SafeGetUTFChars( pEnv, regType);
        const char  *domainStr = SafeGetUTFChars( pEnv, domain);

        err = DNSServiceResolve( &pContext->ServiceRef, flags, ifIndex,
                                 servStr, regStr, domainStr, ServiceResolveReply, pContext);
        if ( err == kDNSServiceErr_NoError)
        {
            (*pEnv)->SetLongField(pEnv, pThis, contextField, (long) pContext);
        }

        SafeReleaseUTFChars( pEnv, serviceName, servStr);
        SafeReleaseUTFChars( pEnv, regType, regStr);
        SafeReleaseUTFChars( pEnv, domain, domainStr);
    }
    else
        err = kDNSServiceErr_NoMemory;

    return err;
}


static void DNSSD_API   ServiceRegisterReply( DNSServiceRef sdRef _UNUSED, DNSServiceFlags flags,
                                              DNSServiceErrorType errorCode, const char *serviceName,
                                              const char *regType, const char *domain, void *context)
{
    OpContext       *pContext = (OpContext*) context;

    SetupCallbackState( &pContext->Env);

    if ( pContext->ClientObj != NULL && pContext->Callback != NULL)
    {
        if ( errorCode == kDNSServiceErr_NoError)
        {
            (*pContext->Env)->CallVoidMethod( pContext->Env, pContext->ClientObj, pContext->Callback,
                                              pContext->JavaObj, flags,
                                              (*pContext->Env)->NewStringUTF( pContext->Env, serviceName),
                                              (*pContext->Env)->NewStringUTF( pContext->Env, regType),
                                              (*pContext->Env)->NewStringUTF( pContext->Env, domain));
        }
        else
            ReportError( pContext->Env, pContext->ClientObj, pContext->JavaObj, errorCode);
    }
    TeardownCallbackState();
}

JNIEXPORT jint JNICALL Java_com_apple_dnssd_AppleRegistration_BeginRegister( JNIEnv *pEnv, jobject pThis,
                                                                             jint ifIndex, jint flags, jstring serviceName, jstring regType,
                                                                             jstring domain, jstring host, jint port, jbyteArray txtRecord)
{
    //syslog(LOG_ERR, "BR");
    jclass cls = (*pEnv)->GetObjectClass( pEnv, pThis);
    jfieldID contextField = (*pEnv)->GetFieldID( pEnv, cls, "fNativeContext", "J");
    OpContext               *pContext = NULL;
    DNSServiceErrorType err = kDNSServiceErr_NoError;
    jbyte                   *pBytes;
    jsize numBytes;

    //syslog(LOG_ERR, "BR: contextField %d", contextField);

    if ( contextField != 0)
        pContext = NewContext( pEnv, pThis, "serviceRegistered",
                               "(Lcom/apple/dnssd/DNSSDRegistration;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    else
        err = kDNSServiceErr_BadParam;

    if ( pContext != NULL)
    {
        const char  *servStr = SafeGetUTFChars( pEnv, serviceName);
        const char  *regStr = SafeGetUTFChars( pEnv, regType);
        const char  *domainStr = SafeGetUTFChars( pEnv, domain);
        const char  *hostStr = SafeGetUTFChars( pEnv, host);

        //syslog(LOG_ERR, "BR: regStr %s", regStr);

        // Since Java ints are defined to be big-endian, we de-canonicalize 'port' from a
        // big-endian number into a 16-bit pattern here.
        uint16_t portBits = port;
        portBits = ( ((unsigned char*) &portBits)[0] << 8) | ((unsigned char*) &portBits)[1];

        pBytes = txtRecord ? (*pEnv)->GetByteArrayElements( pEnv, txtRecord, NULL) : NULL;
        numBytes = txtRecord ? (*pEnv)->GetArrayLength( pEnv, txtRecord) : 0;

        err = DNSServiceRegister( &pContext->ServiceRef, flags, ifIndex, servStr, regStr,
                                  domainStr, hostStr, portBits,
                                  numBytes, pBytes, ServiceRegisterReply, pContext);
        if ( err == kDNSServiceErr_NoError)
        {
            (*pEnv)->SetLongField(pEnv, pThis, contextField, (long) pContext);
        }

        if ( pBytes != NULL)
            (*pEnv)->ReleaseByteArrayElements( pEnv, txtRecord, pBytes, 0);

        SafeReleaseUTFChars( pEnv, serviceName, servStr);
        SafeReleaseUTFChars( pEnv, regType, regStr);
        SafeReleaseUTFChars( pEnv, domain, domainStr);
        SafeReleaseUTFChars( pEnv, host, hostStr);
    }
    else
        err = kDNSServiceErr_NoMemory;

    return err;
}

JNIEXPORT jint JNICALL Java_com_apple_dnssd_AppleRegistration_AddRecord( JNIEnv *pEnv, jobject pThis,
                                                                         jint flags, jint rrType, jbyteArray rData, jint ttl, jobject destObj)
{
    jclass cls = (*pEnv)->GetObjectClass( pEnv, pThis);
    jfieldID contextField = (*pEnv)->GetFieldID( pEnv, cls, "fNativeContext", "J");
    jclass destCls = (*pEnv)->GetObjectClass( pEnv, destObj);
    jfieldID recField = (*pEnv)->GetFieldID( pEnv, destCls, "fRecord", "J");
    OpContext               *pContext = NULL;
    DNSServiceErrorType err = kDNSServiceErr_NoError;
    jbyte                   *pBytes;
    jsize numBytes;
    DNSRecordRef recRef;

    if ( contextField != 0)
        pContext = (OpContext*) (long) (*pEnv)->GetLongField(pEnv, pThis, contextField);
    if ( pContext == NULL || pContext->ServiceRef == NULL)
        return kDNSServiceErr_BadParam;

    pBytes = (*pEnv)->GetByteArrayElements( pEnv, rData, NULL);
    numBytes = (*pEnv)->GetArrayLength( pEnv, rData);

    err = DNSServiceAddRecord( pContext->ServiceRef, &recRef, flags, rrType, numBytes, pBytes, ttl);
    if ( err == kDNSServiceErr_NoError)
    {
        (*pEnv)->SetLongField(pEnv, destObj, recField, (long) recRef);
    }

    if ( pBytes != NULL)
        (*pEnv)->ReleaseByteArrayElements( pEnv, rData, pBytes, 0);

    return err;
}

JNIEXPORT jint JNICALL Java_com_apple_dnssd_AppleDNSRecord_Update( JNIEnv *pEnv, jobject pThis,
                                                                   jint flags, jbyteArray rData, jint ttl)
{
    jclass cls = (*pEnv)->GetObjectClass( pEnv, pThis);
    jfieldID ownerField = (*pEnv)->GetFieldID( pEnv, cls, "fOwner", "Lcom/apple/dnssd/AppleService;");
    jfieldID recField = (*pEnv)->GetFieldID( pEnv, cls, "fRecord", "J");
    OpContext               *pContext = NULL;
    DNSServiceErrorType err = kDNSServiceErr_NoError;
    jbyte                   *pBytes;
    jsize numBytes;
    DNSRecordRef recRef = NULL;

    if ( ownerField != 0)
    {
        jobject ownerObj = (*pEnv)->GetObjectField( pEnv, pThis, ownerField);
        jclass ownerClass = (*pEnv)->GetObjectClass( pEnv, ownerObj);
        jfieldID contextField = (*pEnv)->GetFieldID( pEnv, ownerClass, "fNativeContext", "J");
        if ( contextField != 0)
            pContext = (OpContext*) (long) (*pEnv)->GetLongField(pEnv, ownerObj, contextField);
    }
    if ( recField != 0)
        recRef = (DNSRecordRef) (long) (*pEnv)->GetLongField(pEnv, pThis, recField);
    if ( pContext == NULL || pContext->ServiceRef == NULL)
        return kDNSServiceErr_BadParam;

    pBytes = (*pEnv)->GetByteArrayElements( pEnv, rData, NULL);
    numBytes = (*pEnv)->GetArrayLength( pEnv, rData);

    err = DNSServiceUpdateRecord( pContext->ServiceRef, recRef, flags, numBytes, pBytes, ttl);

    if ( pBytes != NULL)
        (*pEnv)->ReleaseByteArrayElements( pEnv, rData, pBytes, 0);

    return err;
}

JNIEXPORT jint JNICALL Java_com_apple_dnssd_AppleDNSRecord_Remove( JNIEnv *pEnv, jobject pThis)
{
    jclass cls = (*pEnv)->GetObjectClass( pEnv, pThis);
    jfieldID ownerField = (*pEnv)->GetFieldID( pEnv, cls, "fOwner", "Lcom/apple/dnssd/AppleService;");
    jfieldID recField = (*pEnv)->GetFieldID( pEnv, cls, "fRecord", "J");
    OpContext               *pContext = NULL;
    DNSServiceErrorType err = kDNSServiceErr_NoError;
    DNSRecordRef recRef = NULL;

    if ( ownerField != 0)
    {
        jobject ownerObj = (*pEnv)->GetObjectField( pEnv, pThis, ownerField);
        jclass ownerClass = (*pEnv)->GetObjectClass( pEnv, ownerObj);
        jfieldID contextField = (*pEnv)->GetFieldID( pEnv, ownerClass, "fNativeContext", "J");
        if ( contextField != 0)
            pContext = (OpContext*) (long) (*pEnv)->GetLongField(pEnv, ownerObj, contextField);
    }
    if ( recField != 0)
        recRef = (DNSRecordRef) (long) (*pEnv)->GetLongField(pEnv, pThis, recField);
    if ( pContext == NULL || pContext->ServiceRef == NULL)
        return kDNSServiceErr_BadParam;

    err = DNSServiceRemoveRecord( pContext->ServiceRef, recRef, 0);

    return err;
}


JNIEXPORT jint JNICALL Java_com_apple_dnssd_AppleRecordRegistrar_CreateConnection( JNIEnv *pEnv, jobject pThis)
{
    jclass cls = (*pEnv)->GetObjectClass( pEnv, pThis);
    jfieldID contextField = (*pEnv)->GetFieldID( pEnv, cls, "fNativeContext", "J");
    OpContext               *pContext = NULL;
    DNSServiceErrorType err = kDNSServiceErr_NoError;

    if ( contextField != 0)
        pContext = NewContext( pEnv, pThis, "recordRegistered", "(Lcom/apple/dnssd/DNSRecord;I)V");
    else
        err = kDNSServiceErr_BadParam;

    if ( pContext != NULL)
    {
        err = DNSServiceCreateConnection( &pContext->ServiceRef);
        if ( err == kDNSServiceErr_NoError)
        {
            (*pEnv)->SetLongField(pEnv, pThis, contextField, (long) pContext);
        }
    }
    else
        err = kDNSServiceErr_NoMemory;

    return err;
}

struct RecordRegistrationRef
{
    OpContext       *Context;
    jobject RecordObj;
};
typedef struct RecordRegistrationRef RecordRegistrationRef;

static void DNSSD_API   RegisterRecordReply( DNSServiceRef sdRef _UNUSED,
                                             DNSRecordRef recordRef _UNUSED, DNSServiceFlags flags,
                                             DNSServiceErrorType errorCode, void *context)
{
    RecordRegistrationRef   *regEnvelope = (RecordRegistrationRef*) context;
    OpContext       *pContext = regEnvelope->Context;

    SetupCallbackState( &pContext->Env);

    if ( pContext->ClientObj != NULL && pContext->Callback != NULL)
    {
        if ( errorCode == kDNSServiceErr_NoError)
        {
            (*pContext->Env)->CallVoidMethod( pContext->Env, pContext->ClientObj, pContext->Callback,
                                              regEnvelope->RecordObj, flags);
        }
        else
            ReportError( pContext->Env, pContext->ClientObj, pContext->JavaObj, errorCode);
    }

    (*pContext->Env)->DeleteWeakGlobalRef( pContext->Env, regEnvelope->RecordObj);
    free( regEnvelope);

    TeardownCallbackState();
}

JNIEXPORT jint JNICALL Java_com_apple_dnssd_AppleRecordRegistrar_RegisterRecord( JNIEnv *pEnv, jobject pThis,
                                                                                 jint flags, jint ifIndex, jstring fullname, jint rrType, jint rrClass,
                                                                                 jbyteArray rData, jint ttl, jobject destObj)
{
    jclass cls = (*pEnv)->GetObjectClass( pEnv, pThis);
    jfieldID contextField = (*pEnv)->GetFieldID( pEnv, cls, "fNativeContext", "J");
    jclass destCls = (*pEnv)->GetObjectClass( pEnv, destObj);
    jfieldID recField = (*pEnv)->GetFieldID( pEnv, destCls, "fRecord", "J");
    const char              *nameStr = SafeGetUTFChars( pEnv, fullname);
    OpContext               *pContext = NULL;
    DNSServiceErrorType err = kDNSServiceErr_NoError;
    jbyte                   *pBytes;
    jsize numBytes;
    DNSRecordRef recRef;
    RecordRegistrationRef   *regEnvelope;

    if ( contextField != 0)
        pContext = (OpContext*) (long) (*pEnv)->GetLongField(pEnv, pThis, contextField);
    if ( pContext == NULL || pContext->ServiceRef == NULL || nameStr == NULL)
        return kDNSServiceErr_BadParam;

    regEnvelope = calloc( 1, sizeof *regEnvelope);
    if ( regEnvelope == NULL)
        return kDNSServiceErr_NoMemory;
    regEnvelope->Context = pContext;
    regEnvelope->RecordObj = (*pEnv)->NewWeakGlobalRef( pEnv, destObj); // must convert local ref to global to cache

    pBytes = (*pEnv)->GetByteArrayElements( pEnv, rData, NULL);
    numBytes = (*pEnv)->GetArrayLength( pEnv, rData);

    err = DNSServiceRegisterRecord( pContext->ServiceRef, &recRef, flags, ifIndex,
                                    nameStr, rrType, rrClass, numBytes, pBytes, ttl,
                                    RegisterRecordReply, regEnvelope);

    if ( err == kDNSServiceErr_NoError)
    {
        (*pEnv)->SetLongField(pEnv, destObj, recField, (long) recRef);
    }
    else
    {
        if ( regEnvelope->RecordObj != NULL)
            (*pEnv)->DeleteWeakGlobalRef( pEnv, regEnvelope->RecordObj);
        free( regEnvelope);
    }

    if ( pBytes != NULL)
        (*pEnv)->ReleaseByteArrayElements( pEnv, rData, pBytes, 0);

    SafeReleaseUTFChars( pEnv, fullname, nameStr);

    return err;
}


static void DNSSD_API   ServiceQueryReply( DNSServiceRef sdRef _UNUSED, DNSServiceFlags flags, uint32_t interfaceIndex,
                                           DNSServiceErrorType errorCode, const char *serviceName,
                                           uint16_t rrtype, uint16_t rrclass, uint16_t rdlen,
                                           const void *rdata, uint32_t ttl, void *context)
{
    OpContext       *pContext = (OpContext*) context;
    jbyteArray rDataObj;
    jbyte           *pBytes;

    SetupCallbackState( &pContext->Env);

    if ( pContext->ClientObj != NULL && pContext->Callback != NULL &&
         NULL != ( rDataObj = (*pContext->Env)->NewByteArray( pContext->Env, rdlen)))
    {
        if ( errorCode == kDNSServiceErr_NoError)
        {
            // Initialize rDataObj with contents of rdata
            pBytes = (*pContext->Env)->GetByteArrayElements( pContext->Env, rDataObj, NULL);
            memcpy( pBytes, rdata, rdlen);
            (*pContext->Env)->ReleaseByteArrayElements( pContext->Env, rDataObj, pBytes, JNI_COMMIT);

            (*pContext->Env)->CallVoidMethod( pContext->Env, pContext->ClientObj, pContext->Callback,
                                              pContext->JavaObj, flags, interfaceIndex,
                                              (*pContext->Env)->NewStringUTF( pContext->Env, serviceName),
                                              rrtype, rrclass, rDataObj, ttl);
        }
        else
            ReportError( pContext->Env, pContext->ClientObj, pContext->JavaObj, errorCode);
    }
    TeardownCallbackState();
}

JNIEXPORT jint JNICALL Java_com_apple_dnssd_AppleQuery_CreateQuery( JNIEnv *pEnv, jobject pThis,
                                                                    jint flags, jint ifIndex, jstring serviceName, jint rrtype, jint rrclass)
{
    jclass cls = (*pEnv)->GetObjectClass( pEnv, pThis);
    jfieldID contextField = (*pEnv)->GetFieldID( pEnv, cls, "fNativeContext", "J");
    OpContext               *pContext = NULL;
    DNSServiceErrorType err = kDNSServiceErr_NoError;

    if ( contextField != 0)
        pContext = NewContext( pEnv, pThis, "queryAnswered",
                               "(Lcom/apple/dnssd/DNSSDService;IILjava/lang/String;II[BI)V");
    else
        err = kDNSServiceErr_BadParam;

    if ( pContext != NULL)
    {
        const char  *servStr = SafeGetUTFChars( pEnv, serviceName);

        err = DNSServiceQueryRecord( &pContext->ServiceRef, flags, ifIndex, servStr,
                                     rrtype, rrclass, ServiceQueryReply, pContext);
        if ( err == kDNSServiceErr_NoError)
        {
            (*pEnv)->SetLongField(pEnv, pThis, contextField, (long) pContext);
        }

        SafeReleaseUTFChars( pEnv, serviceName, servStr);
    }
    else
        err = kDNSServiceErr_NoMemory;

    return err;
}


static void DNSSD_API   DomainEnumReply( DNSServiceRef sdRef _UNUSED, DNSServiceFlags flags, uint32_t interfaceIndex,
                                         DNSServiceErrorType errorCode, const char *replyDomain, void *context)
{
    OpContext       *pContext = (OpContext*) context;

    SetupCallbackState( &pContext->Env);

    if ( pContext->ClientObj != NULL && pContext->Callback != NULL)
    {
        if ( errorCode == kDNSServiceErr_NoError)
        {
            (*pContext->Env)->CallVoidMethod( pContext->Env, pContext->ClientObj,
                                              ( flags & kDNSServiceFlagsAdd) != 0 ? pContext->Callback : pContext->Callback2,
                                              pContext->JavaObj, flags, interfaceIndex,
                                              (*pContext->Env)->NewStringUTF( pContext->Env, replyDomain));
        }
        else
            ReportError( pContext->Env, pContext->ClientObj, pContext->JavaObj, errorCode);
    }
    TeardownCallbackState();
}

JNIEXPORT jint JNICALL Java_com_apple_dnssd_AppleDomainEnum_BeginEnum( JNIEnv *pEnv, jobject pThis,
                                                                       jint flags, jint ifIndex)
{
    jclass cls = (*pEnv)->GetObjectClass( pEnv, pThis);
    jfieldID contextField = (*pEnv)->GetFieldID( pEnv, cls, "fNativeContext", "J");
    OpContext               *pContext = NULL;
    DNSServiceErrorType err = kDNSServiceErr_NoError;

    if ( contextField != 0)
        pContext = NewContext( pEnv, pThis, "domainFound",
                               "(Lcom/apple/dnssd/DNSSDService;IILjava/lang/String;)V");
    else
        err = kDNSServiceErr_BadParam;

    if ( pContext != NULL)
    {
        pContext->Callback2 = (*pEnv)->GetMethodID( pEnv,
                                                    (*pEnv)->GetObjectClass( pEnv, pContext->ClientObj),
                                                    "domainLost", "(Lcom/apple/dnssd/DNSSDService;IILjava/lang/String;)V");

        err = DNSServiceEnumerateDomains( &pContext->ServiceRef, flags, ifIndex,
                                          DomainEnumReply, pContext);
        if ( err == kDNSServiceErr_NoError)
        {
            (*pEnv)->SetLongField(pEnv, pThis, contextField, (long) pContext);
        }
    }
    else
        err = kDNSServiceErr_NoMemory;

    return err;
}


JNIEXPORT jint JNICALL Java_com_apple_dnssd_AppleDNSSD_ConstructName( JNIEnv *pEnv, jobject pThis _UNUSED,
                                                                      jstring serviceName, jstring regtype, jstring domain, jobjectArray pOut)
{
    DNSServiceErrorType err = kDNSServiceErr_NoError;
    const char              *nameStr = SafeGetUTFChars( pEnv, serviceName);
    const char              *regStr = SafeGetUTFChars( pEnv, regtype);
    const char              *domStr = SafeGetUTFChars( pEnv, domain);
    char buff[ kDNSServiceMaxDomainName + 1];

    err = DNSServiceConstructFullName( buff, nameStr, regStr, domStr);

    if ( err == kDNSServiceErr_NoError)
    {
        // pOut is expected to be a String[1] array.
        (*pEnv)->SetObjectArrayElement( pEnv, pOut, 0, (*pEnv)->NewStringUTF( pEnv, buff));
    }

    SafeReleaseUTFChars( pEnv, serviceName, nameStr);
    SafeReleaseUTFChars( pEnv, regtype, regStr);
    SafeReleaseUTFChars( pEnv, domain, domStr);

    return err;
}

JNIEXPORT void JNICALL Java_com_apple_dnssd_AppleDNSSD_ReconfirmRecord( JNIEnv *pEnv, jobject pThis _UNUSED,
                                                                        jint flags, jint ifIndex, jstring fullName,
                                                                        jint rrtype, jint rrclass, jbyteArray rdata)
{
    jbyte                   *pBytes;
    jsize numBytes;
    const char              *nameStr = SafeGetUTFChars( pEnv, fullName);

    pBytes = (*pEnv)->GetByteArrayElements( pEnv, rdata, NULL);
    numBytes = (*pEnv)->GetArrayLength( pEnv, rdata);

    DNSServiceReconfirmRecord( flags, ifIndex, nameStr, rrtype, rrclass, numBytes, pBytes);

    if ( pBytes != NULL)
        (*pEnv)->ReleaseByteArrayElements( pEnv, rdata, pBytes, 0);

    SafeReleaseUTFChars( pEnv, fullName, nameStr);
}

#define LOCAL_ONLY_NAME "loo"
#define P2P_NAME "p2p"

JNIEXPORT jstring JNICALL Java_com_apple_dnssd_AppleDNSSD_GetNameForIfIndex( JNIEnv *pEnv, jobject pThis _UNUSED,
                                                                             jint ifIndex)
{
    char                    *p = LOCAL_ONLY_NAME, nameBuff[IF_NAMESIZE];

    if (ifIndex == (jint) kDNSServiceInterfaceIndexP2P)
        p = P2P_NAME;
    else if (ifIndex != (jint) kDNSServiceInterfaceIndexLocalOnly)
        p = if_indextoname( ifIndex, nameBuff );

    return (*pEnv)->NewStringUTF( pEnv, p);
}


JNIEXPORT jint JNICALL Java_com_apple_dnssd_AppleDNSSD_GetIfIndexForName( JNIEnv *pEnv, jobject pThis _UNUSED,
                                                                          jstring ifName)
{
    uint32_t ifIndex = kDNSServiceInterfaceIndexLocalOnly;
    const char              *nameStr = SafeGetUTFChars( pEnv, ifName);

    if (strcmp(nameStr, P2P_NAME) == 0)
        ifIndex = kDNSServiceInterfaceIndexP2P;
    else if (strcmp(nameStr, LOCAL_ONLY_NAME))
        ifIndex = if_nametoindex( nameStr);

    SafeReleaseUTFChars( pEnv, ifName, nameStr);

    return ifIndex;
}


#if defined(_WIN32)
static char*
win32_if_indextoname( DWORD ifIndex, char * nameBuff)
{
    PIP_ADAPTER_INFO pAdapterInfo = NULL;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;
    char            *   ifName = NULL;
    ULONG ulOutBufLen = 0;

    if (GetAdaptersInfo( NULL, &ulOutBufLen) != ERROR_BUFFER_OVERFLOW)
    {
        goto exit;
    }

    pAdapterInfo = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);

    if (pAdapterInfo == NULL)
    {
        goto exit;
    }

    dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen );

    if (dwRetVal != NO_ERROR)
    {
        goto exit;
    }

    pAdapter = pAdapterInfo;
    while (pAdapter)
    {
        if (pAdapter->Index == ifIndex)
        {
            // It would be better if we passed in the length of nameBuff to this
            // function, so we would have absolute certainty that no buffer
            // overflows would occur.  Buffer overflows *shouldn't* occur because
            // nameBuff is of size MAX_ADAPTER_NAME_LENGTH.
            strcpy( nameBuff, pAdapter->AdapterName );
            ifName = nameBuff;
            break;
        }

        pAdapter = pAdapter->Next;
    }

exit:

    if (pAdapterInfo != NULL)
    {
        free( pAdapterInfo );
        pAdapterInfo = NULL;
    }

    return ifName;
}


static DWORD
win32_if_nametoindex( const char * nameStr )
{
    PIP_ADAPTER_INFO pAdapterInfo = NULL;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;
    DWORD ifIndex = 0;
    ULONG ulOutBufLen = 0;

    if (GetAdaptersInfo( NULL, &ulOutBufLen) != ERROR_BUFFER_OVERFLOW)
    {
        goto exit;
    }

    pAdapterInfo = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);

    if (pAdapterInfo == NULL)
    {
        goto exit;
    }

    dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen );

    if (dwRetVal != NO_ERROR)
    {
        goto exit;
    }

    pAdapter = pAdapterInfo;
    while (pAdapter)
    {
        if (strcmp(pAdapter->AdapterName, nameStr) == 0)
        {
            ifIndex = pAdapter->Index;
            break;
        }

        pAdapter = pAdapter->Next;
    }

exit:

    if (pAdapterInfo != NULL)
    {
        free( pAdapterInfo );
        pAdapterInfo = NULL;
    }

    return ifIndex;
}
#endif


// Note: The C preprocessor stringify operator ('#') makes a string from its argument, without macro expansion
// e.g. If "version" is #define'd to be "4", then STRINGIFY_AWE(version) will return the string "version", not "4"
// To expand "version" to its value before making the string, use STRINGIFY(version) instead
#define STRINGIFY_ARGUMENT_WITHOUT_EXPANSION(s) # s
#define STRINGIFY(s) STRINGIFY_ARGUMENT_WITHOUT_EXPANSION(s)

// NOT static -- otherwise the compiler may optimize it out
// The "@(#) " pattern is a special prefix the "what" command looks for
const char VersionString_SCCS[] = "@(#) libjdns_sd " STRINGIFY(mDNSResponderVersion) " (" __DATE__ " " __TIME__ ")";
