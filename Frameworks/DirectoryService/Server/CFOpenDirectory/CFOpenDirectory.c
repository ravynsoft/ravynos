/*
 * Copyright (c) 2002-2009 Apple Inc. All rights reserved.
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
    @header     ODSession / ODNode / ODQuery / ODRecord / ODContext
    @abstract   This core code is an abstract CFRuntimeBase-based implementation of Open
                Directory
*/

#include <CoreFoundation/CFRuntime.h>
#include <DirectoryService/DirectoryService.h>
#include <pthread.h>
#include <opendirectory/odconstants.h>
#include "CFOpenDirectory.h"
#include <membership.h>
#include <membershipPriv.h>
#include <CoreFoundation/CFBridgingPriv.h>
#include <CoreFoundation/CFError.h>
#include <pwd.h>
#include <Kerberos/Kerberos.h>
#include <DirectoryServiceCore/CSharedData.h>      // for custom call
#include <CoreFoundation/CFPriv.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <syslog.h>
#include <Security/Authorization.h>

typedef struct
{
    CFRuntimeBase           _base;
    pthread_mutex_t         _mutex;
    CFMutableDictionaryRef  _info;
    CFStringRef             _cfProxyPassword;
    tDirReference           _dsRef;
    bool					_closeRef;
} _ODSession;

enum {
    kODNodeFlagCloseRef     = 0x00000001,
    kODNodeFlagNoSetValues  = 0x00000002,
    kODNodeFlagRecordBlat   = 0x00000004
};

typedef struct
{
    CFRuntimeBase           _base;
    pthread_mutex_t         _mutex;
    CFMutableDictionaryRef  _info;
    ODNodeType              _nodeType;
    CFStringRef             _cfNodePassword;    
    _ODSession              *_ODSession;
    tDirNodeReference       _dsNodeRef;
    int32_t                 _flags;
} _ODNode;

typedef struct
{
    CFRuntimeBase           _base;
    pthread_mutex_t         _mutex;
    _ODNode                 *_ODNode;
    CFStringRef             _cfRecordName;
    CFStringRef             _cfRecordType;
    CFMutableDictionaryRef  _cfAttributes;
    CFSetRef                _cfFetchedAttributes;
    tRecordReference        _dsRecordRef;
} _ODRecord;

typedef struct
{
    CFRuntimeBase           _base;
    pthread_mutex_t         _mutex;
    _ODNode                *_ODNode;
    tContextData            _dsContext;
} _ODContext;

typedef struct
{
    CFRuntimeBase           _base;
    pthread_mutex_t         _mutex;
    _ODNode                *_ODNode;
    CFMutableArrayRef       _results;
    CFMutableSetRef         _cfReturnAttribs;
    tContextData            _dsContext;
    tDataBufferPtr          _dsAttribute;
    tDataBufferPtr          _dsSearchValue;
    tDataListPtr            _dsRecTypeList;
    tDataListPtr            _dsSearchValues;
    tDataListPtr            _dsRetAttrList;
    CFIndex                 _maxValues;
    ODMatchType             _matchType;
    CFErrorRef              _cfError;
	uint64_t				_requestID;
	dispatch_queue_t		_dispatchQueue;
	CFDataRef				_queryID;
	mach_port_t				_replyPort;
	pid_t					_pid;
    bool					_bGetRecordList;
    bool					_bStopSearch;       // used for async stop
    bool					_bRestartSearch;
	bool					_bSearchStarted;
	bool					_externalSession;
} _ODQuery;

static CFTypeID     _kODSessionTypeID   = _kCFRuntimeNotATypeID;
static CFTypeID     _kODNodeTypeID      = _kCFRuntimeNotATypeID;
static CFTypeID     _kODRecordTypeID    = _kCFRuntimeNotATypeID;
static CFTypeID     _kODContextTypeID   = _kCFRuntimeNotATypeID;
static CFTypeID     _kODQueryTypeID     = _kCFRuntimeNotATypeID;

ODSessionRef        kODSessionDefault   = NULL;

const CFStringRef kODNodeNameKey            = CFSTR("NodeName");
const CFStringRef kODNodeUsername           = CFSTR("NodeUsername");
const CFStringRef kODSessionProxyAddress    = CFSTR("ProxyAddress");
const CFStringRef kODSessionProxyPort       = CFSTR("ProxyPort");
const CFStringRef kODSessionProxyUsername   = CFSTR("ProxyUsername");
const CFStringRef kODSessionProxyPassword   = CFSTR("ProxyPassword");
const CFStringRef kODSessionLocalPath       = CFSTR("LocalPath");
const CFStringRef kODErrorDomainFramework   = CFSTR("com.apple.OpenDirectory");

static int gErrorLegacyCodeMode				= 0;

#pragma mark -
#pragma mark Prototypes

extern int ConvertSpaceDelimitedPoliciesToXML( const char *inPolicyStr, int inPreserveStateInfo, char **outXMLDataStr );

_ODSession *_createSession( CFAllocatorRef allocator );
static void _destroySession( _ODSession *inSession );
static CFStringRef _describeSession( _ODSession *inSession );
static void _initSession( _ODSession *inSession );

_ODRecord *_createRecord( CFAllocatorRef allocator );
static void _destroyRecord( _ODRecord *inRecord );
static CFStringRef _describeRecord( _ODRecord *inRecord );
static void _initRecord( _ODRecord *inRecord );

_ODNode *_createNode( CFAllocatorRef allocator );
static void _destroyNode( _ODNode *inNode );
static CFStringRef _describeNode( _ODNode *inNode );
static void _initNode( _ODNode *inNode );

static _ODContext *_createContext( CFAllocatorRef inAllocator, tContextData inContext, ODNodeRef inNodeRef );
static void _destroyContext( _ODContext *inContext );
static CFStringRef _describeContext( _ODContext *inContext );
static void _initContext( _ODContext *inContext );

_ODQuery *_createQuery( CFAllocatorRef inAllocator );
static void _destroyQuery( _ODQuery *inQuery );
static CFStringRef _describeQuery( _ODQuery *inQuery );
static void _initQuery( _ODQuery *inQuery );

static tDirStatus _FindDirNode( _ODNode *inNode, tDirPatternMatch inNodeMatch, CFErrorRef *outError );
static char *_GetCStringFromCFString( CFStringRef cfString );
static tDataBufferPtr _GetDataBufferFromCFType( CFTypeRef inRef );
static bool _AppendRecordsToList( _ODNode *inNode, tDataBufferPtr inDataBuffer, UInt32 inRecCount, CFMutableArrayRef inArrayRef,
                                  CFErrorRef *outError );
static tDirStatus _Authenticate( ODNodeRef inNodeRef, char *inAuthType, char *inRecordType, CFArrayRef inAuthItems,
                                 CFArrayRef *outAuthItems, ODContextRef *outContext, bool inAuthOnly );
static CFMutableDictionaryRef _CopyAttributesFromBuffer( tDirNodeReference inNodeRef, tDataBufferPtr inDataBuffer,
                                                        tAttributeListRef inAttrListRef, UInt32 inCount, CFErrorRef *outError );
static tDataListPtr _ConvertCFArrayToDataList( CFArrayRef inArray );

// These are internals that do not reset the error state of a call, used for nested usage
static bool _ODRecordSetValues( ODRecordRef inRecordRef, ODAttributeType inAttribute, CFTypeRef inValues, CFErrorRef *outError );
static CFDictionaryRef _ODNodeCopyDetails( ODNodeRef inNodeRef, CFArrayRef inAttributeList, CFErrorRef *outError );
static ODNodeRef _ODNodeCreateCopy( CFAllocatorRef inAllocator, ODNodeRef inNodeRef, CFErrorRef *outError );
static ODNodeRef _ODNodeCreateWithName( CFAllocatorRef inAllocator, ODSessionRef inSessionRef, CFStringRef inNodeName, CFErrorRef *outError );
static ODQueryRef _ODQueryCreateWithNode( CFAllocatorRef inAllocator, ODNodeRef inNodeRef, CFTypeRef inRecordTypeOrList, 
                                          ODAttributeType inAttribute, ODMatchType inMatchType, CFTypeRef inQueryValueOrList, 
                                          CFTypeRef inReturnAttributeOrList, CFIndex inMaxValues, CFErrorRef *outError );
static ODNodeRef _ODNodeCreateWithNodeType( CFAllocatorRef inAllocator, ODSessionRef inSessionRef, ODNodeType inType, CFErrorRef *outError );
static ODRecordRef _ODNodeCopyRecord( ODNodeRef inNodeRef, CFStringRef inRecordType, CFStringRef inRecordName, CFArrayRef inAttributes, 
                                      CFErrorRef *outError );
static bool _ODNodeSetCredentials( ODNodeRef inNodeRef, CFStringRef inRecordType, CFStringRef inRecordName, CFStringRef inPassword, CFErrorRef *outError );
static bool _ODNodeSetCredentialsExtended( ODNodeRef inNodeRef, CFStringRef inRecordType, CFStringRef inAuthType, 
                                              CFArrayRef inAuthItems, CFArrayRef *outAuthItems, ODContextRef *outContext,
                                              CFErrorRef *outError, bool authonly );
static CFArrayRef _ODQueryCopyResults( ODQueryRef inQueryRef, bool inPartialResults, CFErrorRef *outError );
static bool _ODRecordAddValue( ODRecordRef inRecordRef, ODAttributeType inAttribute, CFTypeRef inValue, CFErrorRef *outError );
static bool _ODRecordChangePassword( ODRecordRef inRecordRef, CFStringRef inOldPassword, CFStringRef inNewPassword, CFErrorRef *outError );
static bool _ODRecordDelete( ODRecordRef inRecordRef, CFErrorRef *outError );
static CFArrayRef _ODRecordGetValues( ODRecordRef inRecordRef, ODAttributeType inAttribute, CFErrorRef *outError );

// utility functions
static tDirStatus _ReopenDS( _ODSession *inSession );
static tDirStatus _ReopenNode( _ODNode *inNode );
static tDirStatus _ReopenRecord( _ODRecord *inRecord );
static CFStringRef _createRandomPassword( void );
static tDataListPtr _ConvertCFSetToDataList( CFSetRef inSet );
static bool _VerifyNodeTypeForChange( ODRecordRef inRecord, CFErrorRef *outError );
static bool _wasAttributeFetched( _ODRecord *inRecord, ODAttributeType inAttribute );
static void _StripAttributesWithTypePrefix( CFMutableSetRef inSet, CFStringRef inPrefix );
static CFMutableSetRef _minimizeAttributeSet( CFSetRef inSet );
static CFSetRef _attributeListToSet( CFArrayRef inAttributes );

static CFIndex _ODConvertDSErrorCode( CFIndex inCode );

#pragma mark -
#pragma mark Inline functions

CF_INLINE void _ODNodeLock( _ODNode *inNode )
{
#if 0
    pthread_mutex_lock( &(inNode->_ODSession->_mutex) ); // Lock the DS node first
    pthread_mutex_lock( &(inNode->_mutex) );    // Then lock the Node itself
#endif
}

CF_INLINE void _ODNodeUnlock( _ODNode *inNode )
{
#if 0
    pthread_mutex_unlock( &(inNode->_mutex) );    // unlock the Node itself
    pthread_mutex_unlock( &(inNode->_ODSession->_mutex) ); // unlock the DS node
#endif
}

CF_INLINE void _ODRecordLock( _ODRecord *inRecord )
{
#if 0
    pthread_mutex_lock( &(inRecord->_ODNode->_ODSession->_mutex) ); // Lock the DS node first
    pthread_mutex_lock( &(inRecord->_ODNode->_mutex) );    // Then lock the Node itself
    pthread_mutex_lock( &(inRecord->_mutex) ); // Now lock the record
#endif
}

CF_INLINE void _ODRecordUnlock( _ODRecord *inRecord )
{
#if 0
    pthread_mutex_unlock( &(inRecord->_mutex) ); // unlock the record first
    pthread_mutex_unlock( &(inRecord->_ODNode->_mutex) );    // unlock the Node itself
    pthread_mutex_unlock( &(inRecord->_ODNode->_ODSession->_mutex) ); // unlock the DS node first
#endif
}

#pragma mark -
#pragma mark Runloop functions

#pragma mark -
#pragma mark Callbacks

void doQuery( void *inInfo )
{
    _ODQuery  *pQuery = (_ODQuery *) inInfo;
    
    while( 1 )
    {
        CFArrayRef   cfArray = ODQueryCopyResults( (ODQueryRef) pQuery, true, &(pQuery->_cfError) );

        if( NULL == cfArray )
        {
            pQuery->_bStopSearch = true;
        }

        if( NULL != cfArray )
        {
            CFIndex iCount = CFArrayGetCount( cfArray );
            
            if( iCount != 0 )
            {
                pthread_mutex_lock( &(pQuery->_mutex) );
                
                if( NULL != pQuery->_results )
                {
                    CFArrayAppendArray( pQuery->_results, cfArray, CFRangeMake(0,iCount) );
                }
                else
                {
                    pQuery->_results = (CFMutableArrayRef) cfArray; // recast as we know it is mutable
                    cfArray = NULL;
                }

                pthread_mutex_unlock( &(pQuery->_mutex) );
            }
            
            if( NULL != cfArray )
            {
                CFRelease( cfArray );
                cfArray = NULL;
            }
        }
        
        pthread_mutex_lock( &(pQuery->_mutex) );
        if( true == pQuery->_bStopSearch )
        {
            pthread_mutex_unlock( &(pQuery->_mutex) );
            break;
        }
        pthread_mutex_unlock( &(pQuery->_mutex) );
    }
    
    CFRelease( (CFTypeRef) pQuery );
}

#pragma mark -
#pragma mark Registration

void _ODSessionRegisterClass( void )
{
    static const CFRuntimeClass _kODSessionClass = {
        0,                                          // version
        "ODSession",                                // class name
        (void(*)(CFTypeRef))_initSession,           // init
        NULL,                                       // copy
        (void(*)(CFTypeRef))_destroySession,        // dealloc
        NULL,                                       // equal
        NULL,                                       // hash
        NULL,                                       // copyFormattingDesc
        (CFStringRef(*)(CFTypeRef))_describeSession // copyDebugDesc
    };
    
    if( _kODSessionTypeID == _kCFRuntimeNotATypeID )
    {
        _kODSessionTypeID = _CFRuntimeRegisterClass( &_kODSessionClass );
        
        if( _kODSessionTypeID != _kCFRuntimeNotATypeID )
        {
            _CFRuntimeBridgeClasses( _kODSessionTypeID, "NSODSession" );
        }
    }
}

void _ODNodeRegisterClass( void )
{
    static const CFRuntimeClass _kODNodeClass = {
        0,                                              // version
        "ODNode",                                       // class name
        (void(*)(CFTypeRef))_initNode,                  // init
        (void *)ODNodeCreateCopy,                       // copy
        (void(*)(CFTypeRef))_destroyNode,               // dealloc
        NULL,                                           // equal
        NULL,                                           // hash
        NULL,                                           // copyFormattingDesc
        (CFStringRef(*)(CFTypeRef))_describeNode        // copyDebugDesc
    };
    
    if( _kODNodeTypeID == _kCFRuntimeNotATypeID )
    {
        _kODNodeTypeID = _CFRuntimeRegisterClass( &_kODNodeClass );
        
        if( _kODNodeTypeID != _kCFRuntimeNotATypeID )
        {
            _CFRuntimeBridgeClasses( _kODNodeTypeID, "NSODNode" );
        }
    }
}

void _ODRecordRegisterClass( void )
{
    static const CFRuntimeClass _kODRecordClass = {
        0,                                              // version
        "ODRecord",                                     // class name
        (void(*)(CFTypeRef))_initRecord,                // init
        NULL,                                           // copy
        (void(*)(CFTypeRef))_destroyRecord,             // dealloc
        NULL,                                           // equal
        NULL,                                           // hash
        NULL,                                           // copyFormattingDesc
        (CFStringRef(*)(CFTypeRef))_describeRecord      // copyDebugDesc
    };
    
    if( _kODRecordTypeID == _kCFRuntimeNotATypeID )
    {
        _kODRecordTypeID = _CFRuntimeRegisterClass( &_kODRecordClass );
        
        if( _kODRecordTypeID != _kCFRuntimeNotATypeID )
        {
            _CFRuntimeBridgeClasses( _kODRecordTypeID, "NSODRecord" );
        }
    }
}

void _ODContextRegisterClass( void )
{
    static const CFRuntimeClass _kODContextClass = {
        0,                                              // version
        "ODContext",                                    // class name
        (void(*)(CFTypeRef))_initContext,               // init
        NULL,                                           // copy
        (void(*)(CFTypeRef))_destroyContext,            // dealloc
        NULL,                                           // equal
        NULL,                                           // hash
        NULL,                                           // copyFormattingDesc
        (CFStringRef(*)(CFTypeRef))_describeContext     // copyDebugDesc
    };
    
    if( _kODContextTypeID == _kCFRuntimeNotATypeID )
    {
        _kODContextTypeID = _CFRuntimeRegisterClass( &_kODContextClass );
        
        if( _kODContextTypeID != _kCFRuntimeNotATypeID )
        {
            _CFRuntimeBridgeClasses( _kODContextTypeID, "NSODContext" );
        }
    }
}

void _ODQueryRegisterClass( void )
{
    static const CFRuntimeClass _kODQueryClass = {
        0,                                              // version
        "ODQuery",                                      // class name
        (void(*)(CFTypeRef))_initQuery,                 // init
        NULL,                                           // copy
        (void(*)(CFTypeRef))_destroyQuery,              // dealloc
        NULL,                                           // equal
        NULL,                                           // hash
        NULL,                                           // copyFormattingDesc
        (CFStringRef(*)(CFTypeRef))_describeQuery       // copyDebugDesc
    };
    
    if( _kODQueryTypeID == _kCFRuntimeNotATypeID )
    {
        _kODQueryTypeID = _CFRuntimeRegisterClass( &_kODQueryClass );
        
        if( _kODQueryTypeID != _kCFRuntimeNotATypeID )
        {
            _CFRuntimeBridgeClasses( _kODQueryTypeID, "NSODQuery" );
        }
    }
}

#pragma mark -
#pragma mark Internals

_ODSession *_createSession( CFAllocatorRef inAllocator )
{
    return (_ODSession *) _CFRuntimeCreateInstance( inAllocator,
                                                    ODSessionGetTypeID(),
                                                    sizeof(_ODSession) - sizeof(CFRuntimeBase),
                                                    NULL );
}

void _initSession( _ODSession *inSession )
{
    char    *pTemp = (char *) inSession;
    
    bzero( pTemp + sizeof(CFRuntimeBase), sizeof(_ODSession) - sizeof(CFRuntimeBase) );
    
    pthread_mutexattr_t mutexType;
    
    pthread_mutexattr_init( &mutexType );
    pthread_mutexattr_settype( &mutexType, PTHREAD_MUTEX_RECURSIVE );
    pthread_mutex_init( &(inSession->_mutex), &mutexType );
    pthread_mutexattr_destroy( &mutexType );
    
    // Create the dictionary of lookup information
    inSession->_info = CFDictionaryCreateMutable( CFGetAllocator((CFTypeRef)inSession),
                                                  0,
                                                  &kCFTypeDictionaryKeyCallBacks,
                                                  &kCFTypeDictionaryValueCallBacks );
    inSession->_closeRef = true;
}

void _destroySession( _ODSession *inSession )
{
    // Release any gathered information
    if( NULL != inSession->_info )
    {
        CFRelease( inSession->_info );
        inSession->_info = NULL;
    }
    
    if( NULL != inSession->_cfProxyPassword )
    {
        CFRelease( inSession->_cfProxyPassword );
        inSession->_cfProxyPassword = NULL;
    }
    
    if( 0 != inSession->_dsRef && true == inSession->_closeRef )
    {
        dsCloseDirService( inSession->_dsRef );
        inSession->_dsRef = 0;
    }
}


CFStringRef _describeSession( _ODSession *inSession )
{
    CFStringRef result;
    
    result = CFStringCreateWithFormat( CFGetAllocator((ODSessionRef)inSession),
                                       NULL,
                                       CFSTR("<ODSession 0x%x>{info=%@, dsRef=%d}"),
                                       inSession,
                                       inSession->_info,
                                       inSession->_dsRef );
    
    return result;
}

_ODNode *_createNode( CFAllocatorRef inAllocator )
{
    return (_ODNode *) _CFRuntimeCreateInstance( inAllocator,
                                                 ODNodeGetTypeID(),
                                                 sizeof(_ODNode) - sizeof(CFRuntimeBase),
                                                 NULL );
}

void _destroyNode( _ODNode *inNode )
{
    if( NULL != inNode->_info )
    {
        CFRelease( inNode->_info );
        inNode->_info = NULL;
    }
    
    if( NULL != inNode->_cfNodePassword )
    {
        CFRelease( inNode->_cfNodePassword );
        inNode->_cfNodePassword = NULL;
    }

    if( 0 != inNode->_dsNodeRef && 0 != (inNode->_flags & kODNodeFlagCloseRef) )
    {
        dsCloseDirNode( inNode->_dsNodeRef );
        inNode->_dsNodeRef = 0;
    }

    if( NULL != inNode->_ODSession )
    {
        CFRelease( inNode->_ODSession );
        inNode->_ODSession = NULL;
    }
}

CFStringRef _describeNode( _ODNode *inNode )
{
    CFStringRef result;
    
    result = CFStringCreateWithFormat( CFGetAllocator((ODRecordRef)inNode),
                                       NULL,
                                       CFSTR("<ODNode 0x%x>{ODSession=%@, info=%@, nodeRef=%d}"),
                                       inNode,
                                       inNode->_ODSession,
                                       inNode->_info,
                                       inNode->_dsNodeRef );
    
    return result;
}

void _initNode( _ODNode *inNode )
{
    char    *pTemp = (char *) inNode;
    
    bzero( pTemp + sizeof(CFRuntimeBase), sizeof(_ODNode) - sizeof(CFRuntimeBase) );
    
    pthread_mutexattr_t mutexType;
    
    pthread_mutexattr_init( &mutexType );
    pthread_mutexattr_settype( &mutexType, PTHREAD_MUTEX_RECURSIVE );
    pthread_mutex_init( &(inNode->_mutex), &mutexType );
    pthread_mutexattr_destroy( &mutexType );
    
    inNode->_info = CFDictionaryCreateMutable( CFGetAllocator((CFTypeRef)inNode), 0, 
                                               &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
    inNode->_flags |= kODNodeFlagCloseRef;
}

_ODRecord *_createRecord( CFAllocatorRef inAllocator )
{
    return (_ODRecord *) _CFRuntimeCreateInstance( inAllocator,
                                                   ODRecordGetTypeID(),
                                                   sizeof(_ODRecord) - sizeof(CFRuntimeBase),
                                                   NULL );
}

void _destroyRecord( _ODRecord *inRecord )
{
    if( 0 != inRecord->_dsRecordRef )
    {
        dsCloseRecord( inRecord->_dsRecordRef );
        inRecord->_dsRecordRef = 0;
    }
    
    if( NULL != inRecord->_ODNode )
    {
        CFRelease( inRecord->_ODNode );
        inRecord->_ODNode = NULL;
    }
    
    if( NULL != inRecord->_cfRecordName )
    {
        CFRelease( inRecord->_cfRecordName );
        inRecord->_cfRecordName = NULL;
    }
    
    if( NULL != inRecord->_cfRecordType )
    {
        CFRelease( inRecord->_cfRecordType );
        inRecord->_cfRecordType = NULL;
    }
    
    if( NULL != inRecord->_cfAttributes )
    {
        CFRelease( inRecord->_cfAttributes );
        inRecord->_cfAttributes = NULL;
    }
    
    if( NULL != inRecord->_cfFetchedAttributes )
    {
        CFRelease( inRecord->_cfFetchedAttributes );
        inRecord->_cfFetchedAttributes = NULL;
    }
}

CFStringRef _describeRecord( _ODRecord *inRecord )
{
    CFStringRef result;
    
    result = CFStringCreateWithFormat( CFGetAllocator((ODRecordRef)inRecord),
                                       NULL,
                                       CFSTR("<ODRecord 0x%x>{ODNode=%@, cfRecordName=%@, cfRecordType=%@, dsRecordRef=%d, fetchAttributes=%@, attributes=%@}"),
                                       inRecord,
                                       inRecord->_ODNode,
                                       inRecord->_cfRecordName,
                                       inRecord->_cfRecordType,
                                       inRecord->_dsRecordRef,
                                       inRecord->_cfFetchedAttributes,
                                       inRecord->_cfAttributes );
    
    return result;
}

void _initRecord( _ODRecord *inRecord )
{
    char    *pTemp = (char *) inRecord;
    
    bzero( pTemp + sizeof(CFRuntimeBase), sizeof(_ODRecord) - sizeof(CFRuntimeBase) );
    
    pthread_mutexattr_t mutexType;
    
    pthread_mutexattr_init( &mutexType );
    pthread_mutexattr_settype( &mutexType, PTHREAD_MUTEX_RECURSIVE );
    pthread_mutex_init( &(inRecord->_mutex), &mutexType );
    pthread_mutexattr_destroy( &mutexType );
    
    inRecord->_cfFetchedAttributes = CFSetCreateMutable( kCFAllocatorDefault, 0, &kCFTypeSetCallBacks );
}

_ODContext *_createContext( CFAllocatorRef inAllocator, tContextData inContext, ODNodeRef inNodeRef )
{
    _ODContext *result;
    
    result = (_ODContext *) _CFRuntimeCreateInstance( inAllocator, ODContextGetTypeID(),
                                                      sizeof(_ODContext) - sizeof(CFRuntimeBase), NULL );

    if( NULL != result )
    {
        result->_dsContext = inContext;
        result->_ODNode = (_ODNode *) CFRetain( inNodeRef );
    }

    return result;
}

void _destroyContext( _ODContext *inContext )
{
    if( 0 != inContext->_dsContext )
    {
        dsReleaseContinueData( inContext->_ODNode->_ODSession->_dsRef, inContext->_dsContext );
        inContext->_dsContext = 0;
    }
    
    if( NULL != inContext->_ODNode )
    {
        CFRelease( (CFTypeRef) (inContext->_ODNode) );
        inContext->_ODNode = NULL;
    }
}

CFStringRef _describeContext( _ODContext *inContext )
{
    CFStringRef result;
    
    result = CFStringCreateWithFormat( CFGetAllocator((CFTypeRef)inContext),
                                       NULL,
                                       CFSTR("<ODContext 0x%x>{ODNode=%@, dsContext=%d}"),
                                       inContext,
                                       inContext->_ODNode,
                                       inContext->_dsContext );
    
    return result;
}

void _initContext( _ODContext *inContext )
{
    char    *pTemp = (char *) inContext;
    
    bzero( pTemp + sizeof(CFRuntimeBase), sizeof(_ODContext) - sizeof(CFRuntimeBase) );
    
    pthread_mutexattr_t mutexType;
    
    pthread_mutexattr_init( &mutexType );
    pthread_mutexattr_settype( &mutexType, PTHREAD_MUTEX_RECURSIVE );
    pthread_mutex_init( &(inContext->_mutex), &mutexType );
    pthread_mutexattr_destroy( &mutexType );
}

_ODQuery *_createQuery( CFAllocatorRef inAllocator )
{
    return (_ODQuery *) _CFRuntimeCreateInstance( inAllocator, ODQueryGetTypeID(),
                                                  sizeof(_ODQuery) - sizeof(CFRuntimeBase), NULL );
}

void _destroyQuery( _ODQuery *inQuery )
{
    if( NULL != inQuery->_dsRetAttrList )
    {
        dsDataListDeallocate(0, inQuery->_dsRetAttrList );
        free( inQuery->_dsRetAttrList );
        inQuery->_dsRetAttrList = NULL;
    }
    
    if( NULL != inQuery->_dsSearchValues )
    {
        dsDataListDeallocate(0, inQuery->_dsSearchValues );
        free( inQuery->_dsSearchValues );
        inQuery->_dsSearchValues = NULL;
    }
    
    if( NULL != inQuery->_dsRecTypeList )
    {
        dsDataListDeallocate(0, inQuery->_dsRecTypeList );
        free( inQuery->_dsRecTypeList );
        inQuery->_dsRecTypeList = NULL;
    }
    
    if( NULL != inQuery->_dsSearchValue )
    {
        dsDataBufferDeAllocate( 0, inQuery->_dsSearchValue );
        inQuery->_dsSearchValue = NULL;
    }
    
    if( NULL != inQuery->_dsAttribute )
    {
        dsDataBufferDeAllocate( 0, inQuery->_dsAttribute );
        inQuery->_dsAttribute = NULL;
    }
    
    if( 0 != inQuery->_dsContext )
    {
        dsReleaseContinueData( inQuery->_ODNode->_ODSession->_dsRef, inQuery->_dsContext );
        inQuery->_dsContext = 0;
    }
    
    if( NULL != inQuery->_results )
    {
        CFRelease( inQuery->_results );
        inQuery->_results = NULL;
    }
    
    if( NULL != inQuery->_cfReturnAttribs )
    {
        CFRelease( inQuery->_cfReturnAttribs );
        inQuery->_cfReturnAttribs = NULL;
    }
	
	dispatch_release(inQuery->_dispatchQueue);
	DSCFRelease(inQuery->_queryID);
    
    if( NULL != inQuery->_ODNode )
    {
        CFRelease( (CFTypeRef) (inQuery->_ODNode) );
        inQuery->_ODNode = NULL;
    }
}

CFStringRef _describeQuery( _ODQuery *inQuery )
{
    return CFStringCreateWithFormat( CFGetAllocator((CFTypeRef)inQuery),
                                     NULL,
                                     CFSTR("<ODQuery 0x%x>{ODNode=%@}"),
                                     inQuery,
                                     inQuery->_ODNode );
}

void _initQuery( _ODQuery *inQuery )
{
    char    *pTemp = (char *) inQuery;
    
    bzero( pTemp + sizeof(CFRuntimeBase), sizeof(_ODQuery) - sizeof(CFRuntimeBase) );
    
    pthread_mutexattr_t mutexType;
    
    pthread_mutexattr_init( &mutexType );
    pthread_mutexattr_settype( &mutexType, PTHREAD_MUTEX_RECURSIVE );
    pthread_mutex_init( &(inQuery->_mutex), &mutexType );
    pthread_mutexattr_destroy( &mutexType );
	
	inQuery->_dispatchQueue = dispatch_queue_create("com.apple.DirectoryService.queryQueue", NULL);
}

#pragma mark -
#pragma mark ODError functions

CFStringRef _MapDSErrorToReason( CFErrorRef *outError, tDirStatus dsStatus )
{
    CFStringRef cfError;
    char *pErrorString;

    // no error, no work
    if( eDSNoErr == dsStatus )
        return NULL;
    
    // if we already have an error just return empty string, this signifies an error
    if( (NULL != outError && NULL != (*outError)) || (NULL == outError) )
        return CFSTR("");

	CFIndex newCode = _ODConvertDSErrorCode( dsStatus );
	
    // we don't actually set an error string if we don't have an outError (save cycles)
    switch( newCode )
    {
		case kODErrorSessionLocalOnlyDaemonInUse:
			cfError = CFSTR("Session can't be opened to normal daemon because local only references are open.");
			break;

		case kODErrorSessionNormalDaemonInUse:
			cfError = CFSTR("Session can't be opened to local only daemon because normal daemon references are open.");
			break;

		case kODErrorSessionDaemonNotRunning:
			cfError = CFSTR("Session can't be opened because daemon is not running.");
			break;

		case kODErrorSessionDaemonRefused:
			cfError = CFSTR("Session can't be opened because daemon refused the connection.");
			break;

		case kODErrorSessionProxyCommunicationError:
            cfError = CFSTR("Proxy failed due to a communication error.");
			break;
			
		case kODErrorSessionProxyVersionMismatch:
            cfError = CFSTR("Proxy failed because the version is not supported by this client.");
			break;
			
		case kODErrorSessionProxyIPUnreachable:
            cfError = CFSTR("Proxy failed because the host provided is not responding.");
			break;
			
		case kODErrorSessionProxyUnknownHost:
            cfError = CFSTR("Proxy failed because an unknown host was provided.");
			break;
			
		case kODErrorNodeUnknownName:
            cfError = CFSTR("Node name wasn't found.");
			break;
			
		case kODErrorNodeUnknownType:
			cfError = CFSTR("Unable to open node type requested.");
			break;
			
		case kODErrorNodeConnectionFailed:
            cfError = CFSTR("Connection failed to the directory server.");
			break;
            
        case kODErrorNodeUnknownHost:
            cfError = CFSTR("Could not resolve the address.");
            break;
			
		case kODErrorQuerySynchronize:
			cfError = CFSTR("Previous results are no longer valid because a synchronize call was requested.");
			break;
			
		case kODErrorQueryInvalidMatchType:
			cfError = CFSTR("An invalid search type was provided during the query.");
			break;

		case kODErrorQueryUnsupportedMatchType:
			cfError = CFSTR("That type of search is not supported by the directory node.");
			break;
            
        case kODErrorQueryTimeout:
            cfError = CFSTR("The query exceeded the maximum time allowed.");
			break;

		case kODErrorRecordReadOnlyNode:
			cfError = CFSTR("Unable to modify record because the directory node is read only.");
			break;
			
		case kODErrorRecordPermissionError:
			cfError = CFSTR("Operation was denied because the current credentials do not have the appropriate privileges.");
			break;
			
		case kODErrorRecordParameterError:
			cfError = CFSTR("One of the parameters provided was invalid.");
			break;
			
		case kODErrorRecordInvalidType:
			cfError = CFSTR("An invalid record type was provided.");
			break;
			
		case kODErrorRecordAlreadyExists:
			cfError = CFSTR("Could not create the record because one already exists with the same name.");
			break;
			
		case kODErrorRecordTypeDisabled:
			cfError = CFSTR("The record type provided is not allowed by the node.");
			break;
			
		case kODErrorRecordAttributeUnknownType:
			cfError = CFSTR("An invalid attribute type was provided.");
			break;
			
		case kODErrorRecordAttributeNotFound:
			cfError = CFSTR("The requested attribute could not be found.");
			break;
			
		case kODErrorRecordAttributeValueSchemaError:
			cfError = CFSTR("The attribute value could not be used because it does not meet the requirements of the attribute.");
			break;
			
		case kODErrorRecordAttributeValueNotFound:
			cfError = CFSTR("The requested attribute value could not be found.");
			break;
			
		case kODErrorCredentialsInvalid:
			cfError = CFSTR("Credentials could not be verified username or password is invalid.");
			break;

		case kODErrorCredentialsMethodNotSupported:
			cfError = CFSTR("Could not verify credentials because directory server does not support the requested authentication method.");
			break;
			
		case kODErrorCredentialsNotAuthorized:
			cfError = CFSTR("Authentication server refused operation because the current credentials are not authorized for the requested operation.");
			break;
			
		case kODErrorCredentialsParameterError:
			cfError = CFSTR("Credential operation failed because an invalid parameter was provided.");
			break;
			
		case kODErrorCredentialsOperationFailed:
			cfError = CFSTR("Authentication server failed to complete the requested operation.");
			break;
			
		case kODErrorCredentialsServerUnreachable:
			cfError = CFSTR("Authentiation server could not be contacted.");
			break;
			
		case kODErrorCredentialsServerNotFound:
			cfError = CFSTR("Authentication server could not be found for the requested operation.");
			break;
			
		case kODErrorCredentialsServerError:
			cfError = CFSTR("Authentication server encountered an error while attempting the requested operation.");
			break;
			
		case kODErrorCredentialsServerTimeout:
			cfError = CFSTR("Authentication server timed out while attempting the requested operation.");
			break;
		case kODErrorCredentialsContactMaster:
			cfError = CFSTR("Authentication server refused the operation because it wasn't the master.");
			break;
			
		case kODErrorCredentialsServerCommunicationError:
			cfError = CFSTR("Authentication server encountered a communication error while attempting the requested operation.");
			break;
			
		case kODErrorCredentialsAccountNotFound:
			cfError = CFSTR("Credentials failed because authentication server could not find the account.");
			break;
			
		case kODErrorCredentialsAccountDisabled:
			cfError = CFSTR("Credential verification failed because account is disabled.");
			break;
			
		case kODErrorCredentialsAccountExpired:
			cfError = CFSTR("Credential verification failed because account is expired.");
			break;
			
		case kODErrorCredentialsAccountInactive:
			cfError = CFSTR("Credential verification failed because account is inactive.");
			break;
			
		case kODErrorCredentialsPasswordExpired:
			cfError = CFSTR("Credential verification failed because password has expired.");
			break;

		case kODErrorCredentialsPasswordChangeRequired:
			cfError = CFSTR("Password change is required by authentication server.");
			break;
			
		case kODErrorCredentialsPasswordQualityFailed:
			cfError = CFSTR("Password change failed because password does not meet minimum quality requirements.");
			break;
			
		case kODErrorCredentialsPasswordTooShort:
			cfError = CFSTR("Password change failed because password is too short.");
			break;
			
		case kODErrorCredentialsPasswordTooLong:
			cfError = CFSTR("Password change failed because password is too long.");
			break;
			
		case kODErrorCredentialsPasswordNeedsLetter:
			cfError = CFSTR("Password change failed because password requires a letter.");
			break;
			
		case kODErrorCredentialsPasswordNeedsDigit:
			cfError = CFSTR("Password change failed because password requires a number.");
			break;
			
		case kODErrorCredentialsPasswordChangeTooSoon:
			cfError = CFSTR("Password change failed because password was changed recently.");
			break;
			
		case kODErrorCredentialsPasswordUnrecoverable:
			cfError = CFSTR("Credential server can't recover password for verification.");
			break;

		case kODErrorCredentialsInvalidLogonHours:
			cfError = CFSTR("Not allowed to log into the computer outside of designated hours.");
			break;
			
		case kODErrorCredentialsInvalidComputer:
			cfError = CFSTR("Not allowed to log into the computer.");
			break;
			
		case kODErrorPluginOperationNotSupported:
			cfError = CFSTR("Operation is not supported by the directory node.");
			break;

		case kODErrorPluginError:
			cfError = CFSTR("The plugin encountered an error processing request.");
			break;
			
		case kODErrorDaemonError:
			cfError = CFSTR("The daemon encountered an error processing request.");
			break;
			
        default:
			pErrorString = dsCopyDirStatusName( dsStatus );
			if ( NULL != pErrorString ) {
				cfError = CFStringCreateWithCString( kCFAllocatorDefault, pErrorString, kCFStringEncodingUTF8 );
				free( pErrorString );
			}
			else {
				cfError = CFSTR("Unknown error code.");
			}
			break;
    }
            
    return cfError;
}

CFIndex _ODConvertDSErrorCode( CFIndex inCode )
{
	CFIndex	newCode = inCode;
	
	switch ( inCode )
	{
		case eDSLocalDSDaemonInUse:
			newCode = kODErrorSessionLocalOnlyDaemonInUse;
			break;
			
		case eDSNormalDSDaemonInUse:
			newCode = kODErrorSessionNormalDaemonInUse;
			break;

		case eDSOpenFailed:
			newCode = kODErrorSessionDaemonRefused;
			break;

		case eDSOpenNodeFailed:
		case eDSNodeNotFound:
		case eDSUnknownNodeName:
		case eDSInvalidDomain:
		case eDSBadTargetDataNode:
			newCode = kODErrorNodeUnknownName;
			break;

		case eDSCannotAccessSession:
		case eDSMaxSessionsOpen:
			newCode = kODErrorNodeConnectionFailed;
			break;
			
		case eDSReadOnly:
			newCode = kODErrorRecordReadOnlyNode;
			break;

		case eDSUnSupportedMatchType:
			newCode = kODErrorQueryUnsupportedMatchType;
			break;
			
		case eDSNoStdMappingAvailable:
		case eDSInvalidNativeMapping:
		case eDSInvalidAttributeType:
			newCode = kODErrorRecordAttributeUnknownType;
			break;
			
		case eDSAttributeNotFound:
		case eDSAttributeDoesNotExist:
			newCode = kODErrorRecordAttributeNotFound;
			break;
			
		case eDSSchemaError:
			newCode = kODErrorRecordAttributeValueSchemaError;
			break;
			
		case eDSRecordAlreadyExists:
			newCode = kODErrorRecordAlreadyExists;
			break;

		case eDSPermissionError:
			newCode = kODErrorRecordPermissionError;
			break;
			
		case eDSAttributeValueNotFound:
			newCode = kODErrorRecordAttributeValueNotFound;
			break;
			
		case eServerNotRunning:
			newCode = kODErrorSessionDaemonNotRunning;
			break;
			
		case eUnknownServerError:
		case eUnknownAPICall:
		case eServerError:
		case eUnknownPlugIn:
			newCode = kODErrorDaemonError;
			break;
			
		case eDSVersionMismatch:
		case eDSTCPVersionMismatch:
			newCode = kODErrorSessionProxyVersionMismatch;
			break;
			
		case eDSTCPSendError:
		case eDSTCPReceiveError:
			newCode = kODErrorSessionProxyCommunicationError;
			break;
			
		case eDSIPUnreachable:
			newCode = kODErrorSessionProxyIPUnreachable;
			break;
			
		case eDSUnknownHost:
			newCode = kODErrorSessionProxyUnknownHost;
			break;

		case eDSNullPatternMatch:
		case eDSEmptyPattern2Match:
		case eDSUnknownMatchType:
			newCode = kODErrorQueryInvalidMatchType;
			break;
			
		case eDSNullParameter:
		case eDSNullDataBuff:
		case eDSNullRecEntryPtr:
		case eDSNullRecName:
		case eDSNullRecNameList:
		case eDSNullAttribute:
		case eDSNullAttributeAccess:
		case eDSNullAttributeValue:
		case eDSNullAttributeType:
		case eDSNullAttributeTypeList:
		case eDSNullAttributeControlPtr:
		case eDSNullAttributeRequestList:
		case eDSNullDataList:
		case eDSNullAutMethod:
		case eDSNullAuthStepData:
		case eDSNullAuthStepDataResp:
		case eDSNullNodeInfoTypeList:
		case eDSNullNodeNamePattern:
		case eParameterError:
		case eDSEmptyParameter:
		case eDSEmptyBuffer:
		case eDSNullNodeName:
		case eDSNullDirNodeTypeList:
		case eDSEmptyNodeName:
		case eDSEmptyRecordName:
		case eDSEmptyRecordNameList:
		case eDSEmptyRecordEntry:
		case eDSEmptyAttribute:
		case eDSEmptyAttributeType:
		case eDSEmptyAttributeTypeList:
		case eDSEmptyAttributeValue:
		case eDSEmptyAttributeRequestList:
		case eDSEmptyDataList:
		case eDSEmptyNodeInfoTypeList:
		case eDSInvalDataList:
		case eDSAttrListError:
		case eDSInvalidRecordName:
			newCode = kODErrorRecordParameterError;
			break;
			
		case eDSEmptyRecordType:
		case eDSEmptyRecordTypeList:
		case eDSNullRecType:
		case eDSNullRecTypeList:
		case eDSInvalidRecordType:
			newCode = kODErrorRecordInvalidType;
			break;
			
		case eDSRecordTypeDisabled:
			newCode = kODErrorRecordTypeDisabled;
			break;
			
		case eNoLongerSupported:
		case eNotYetImplemented:
		case eNotHandledByThisNode:
			newCode = kODErrorPluginOperationNotSupported;
			break;
			
		case eDSAuthFailed:
		case eDSAuthenticationFailed:
		case eDSAuthBadPassword:
			newCode = kODErrorCredentialsInvalid;
			break;
			
		case eDSAuthMethodNotSupported:
			newCode = kODErrorCredentialsMethodNotSupported;
			break;

		case eDSAuthNoSuchEntity:
		case eDSAuthUnknownUser:
		case eDSAuthInvalidUserName:
		case eDSInvalidName:
		case eDSUserUnknown:
			newCode = kODErrorCredentialsAccountNotFound;
			break;

		case eDSNotAuthorized:
			newCode = kODErrorCredentialsNotAuthorized;
			break;
			
		case eDSAuthInBuffFormatError:
		case eDSAuthParameterError:
		case eDSAuthContinueDataBad:
		case eDSEmptyAuthStepDataResp:
		case eDSEmptyAuthStepData:
		case eDSEmptyAuthMethod:
			newCode = kODErrorCredentialsParameterError;
			break;
			
		case eDSOperationFailed:
			newCode = kODErrorCredentialsOperationFailed;
			break;
			
		case eDSServiceUnavailable:
			newCode = kODErrorCredentialsServerUnreachable;
			break;
			
		case eDSAuthNoAuthServerFound:
			newCode = kODErrorCredentialsServerNotFound;
			break;
            
		case eDSBogusServer:
            newCode = kODErrorNodeUnknownHost;
            break;
			
		case eDSInvalidHandle:
		case eDSSendFailed:
		case eDSReceiveFailed:
		case eDSBadPacket:
			newCode = kODErrorCredentialsServerCommunicationError;
			break;
			
		case eDSAuthServerError:
		case eDSInvalidSession:
		case eDSInvalidTag:
			newCode = kODErrorCredentialsServerError;
			break;
			
		case eDSServerTimeout:
		case eDSAuthMasterUnreachable:
			newCode = kODErrorCredentialsServerTimeout;
			break;
			
		case eDSContactMaster:
			newCode = kODErrorCredentialsContactMaster;
			break;
			
		case eDSAuthNewPasswordRequired:
			newCode = kODErrorCredentialsPasswordChangeRequired;
			break;
			
		case eDSAuthPasswordExpired:
			newCode = kODErrorCredentialsPasswordExpired;
			break;
			
		case eDSAuthPasswordQualityCheckFailed:
			newCode = kODErrorCredentialsPasswordQualityFailed;
			break;
			
		case eDSAuthAccountDisabled:
			newCode = kODErrorCredentialsAccountDisabled;
			break;
			
		case eDSAuthAccountExpired:
			newCode = kODErrorCredentialsAccountExpired;
			break;
			
		case eDSAuthAccountInactive:
			newCode = kODErrorCredentialsAccountInactive;
			break;
			
		case eDSAuthPasswordTooShort:
			newCode = kODErrorCredentialsPasswordTooShort;
			break;
			
		case eDSAuthPasswordTooLong:
			newCode = kODErrorCredentialsPasswordTooLong;
			break;
			
		case eDSAuthPasswordNeedsLetter:
			newCode = kODErrorCredentialsPasswordNeedsLetter;
			break;
			
		case eDSAuthPasswordNeedsDigit:
			newCode = kODErrorCredentialsPasswordNeedsDigit;
			break;
			
		case eDSAuthPasswordChangeTooSoon:
			newCode = kODErrorCredentialsPasswordChangeTooSoon;
			break;
			
		case eDSAuthInvalidLogonHours:
			newCode = kODErrorCredentialsInvalidLogonHours;
			break;
			
		case eDSAuthInvalidComputer:
			newCode = kODErrorCredentialsInvalidComputer;
			break;
			
		case eDSAuthCannotRecoverPasswd:
		case eDSUnrecoverablePassword:
			newCode = kODErrorCredentialsPasswordUnrecoverable;
			break;
            
        case eDSOperationTimeout:
            newCode = kODErrorQueryTimeout;
            break;
			
		case ePlugInNotActive:
			return kODErrorNodeDisabled;
			
		case eCFMGetFileSysRepErr:
		case eCFPlugInGetBundleErr:
		case eCFBndleGetInfoDictErr:
		case eCFDictGetValueErr:
		case ePluginHandlerNotLoaded:
		case eNoPluginsLoaded:
		case ePluginAlreadyLoaded:
		case ePluginVersionNotFound:
		case ePluginNameNotFound:
		case eNoPluginFactoriesFound:
		case ePluginConfigAvailNotFound:
		case ePluginConfigFileNotFound:
		case ePlugInDataError:
		case ePlugInNotFound:
		case ePlugInError:
		case ePlugInInitError:
		case ePlugInFailedToInitialize:
		case ePlugInCallTimedOut:
		case eDSPlugInConfigFileError:
		case eDSInvalidPlugInConfigData:
		case eUndefinedError:
			newCode = kODErrorPluginError;
			break;
			
		default:
			break;
	}
	
	return newCode;
}

bool _ODErrorSet( CFErrorRef *outError, CFStringRef inDomain, CFIndex inCode, 
				  CFStringRef *inLocalizedError, 
				  CFStringRef *inLocalizedReason, 
                  CFStringRef inRecoverySuggestion )
{
	if ( gErrorLegacyCodeMode == false )
		inCode = _ODConvertDSErrorCode( inCode );
	
    // if we have an error pointer and it hasn't already been set, let's set it.
    if( NULL != outError && NULL == (*outError) )
    {
        CFMutableDictionaryRef userInfo = CFDictionaryCreateMutable( kCFAllocatorDefault, 3, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
        
        if( NULL != inLocalizedError && NULL != *inLocalizedError)
            CFDictionarySetValue( userInfo, kCFErrorLocalizedDescriptionKey, *inLocalizedError );
        
        if( NULL != inLocalizedReason && NULL != *inLocalizedReason)
            CFDictionarySetValue( userInfo, kCFErrorLocalizedFailureReasonKey, *inLocalizedReason );
        
        if( NULL != inRecoverySuggestion)
            CFDictionarySetValue( userInfo, kCFErrorLocalizedRecoverySuggestionKey, inRecoverySuggestion );

        *outError = CFErrorCreate( kCFAllocatorDefault, inDomain, inCode, userInfo );
        
        CFRelease( userInfo );
    }
    
    if( NULL != inLocalizedError && NULL != *inLocalizedError)
    {
        CFRelease( *inLocalizedError );
        *inLocalizedError = NULL;
    }
    
    if( NULL != inLocalizedReason && NULL != *inLocalizedReason)
    {
        CFRelease( *inLocalizedReason );
        *inLocalizedReason = NULL;
    }
    
    if( NULL != inRecoverySuggestion)
    {
        CFRelease( inRecoverySuggestion );
        inRecoverySuggestion = NULL;
    }
	return true;
}

extern void thread_stack_pcs( vm_address_t *buffer, unsigned max, unsigned *num );

CFIndex ODConvertToLegacyErrorCode( CFIndex inCode )
{
    if ( inCode <= 0 ) return inCode;
    
    switch ( inCode )
    {
        case kODErrorSessionLocalOnlyDaemonInUse:
            return eDSLocalDSDaemonInUse;
            
        case kODErrorSessionNormalDaemonInUse:
            return eDSNormalDSDaemonInUse;
            
        case kODErrorSessionDaemonRefused:
            return eDSOpenFailed;
            
        case kODErrorNodeUnknownName:
            return eDSOpenNodeFailed;
            
        case kODErrorNodeConnectionFailed:
            return eDSCannotAccessSession;
            
        case kODErrorNodeUnknownHost: 
            return eDSBogusServer;
            
        case kODErrorRecordReadOnlyNode:
            return eDSReadOnly;
            
        case kODErrorQueryUnsupportedMatchType:
            return eDSUnSupportedMatchType;
            
        case kODErrorRecordAttributeUnknownType:
            return eDSNoStdMappingAvailable;
            
        case kODErrorRecordAttributeNotFound:
            return eDSAttributeNotFound;
            
        case kODErrorRecordAttributeValueSchemaError:
            return eDSSchemaError;
            
        case kODErrorRecordAlreadyExists:
            return eDSRecordAlreadyExists;
            
        case kODErrorRecordPermissionError:
            return eDSPermissionError;
            
        case kODErrorRecordAttributeValueNotFound:
            return eDSAttributeValueNotFound;
            
        case kODErrorSessionDaemonNotRunning:
            return eServerNotRunning;
            
        case kODErrorDaemonError:
            return eServerError;
            
        case kODErrorSessionProxyVersionMismatch:
            return eDSVersionMismatch;
            
        case kODErrorSessionProxyCommunicationError:
            return eDSTCPSendError;
            
        case kODErrorSessionProxyIPUnreachable:
            return eDSIPUnreachable;
            
        case kODErrorSessionProxyUnknownHost:
            return eDSUnknownHost;
            
        case kODErrorQueryInvalidMatchType:
            return eDSUnknownMatchType;
            
        case kODErrorRecordParameterError:
            return eParameterError;
            
        case kODErrorRecordInvalidType:
            return eDSInvalidRecordType;
            
        case kODErrorRecordTypeDisabled:
            return eDSRecordTypeDisabled;
            
        case kODErrorPluginOperationNotSupported:
            return eNotYetImplemented;
            
        case kODErrorCredentialsInvalid:
            return eDSAuthFailed;
            
        case kODErrorCredentialsMethodNotSupported:
            return eDSAuthMethodNotSupported;
            
        case kODErrorCredentialsAccountNotFound:
            return eDSAuthUnknownUser;
            
        case kODErrorCredentialsNotAuthorized:
            return eDSNotAuthorized;
            
        case kODErrorCredentialsParameterError:
            return eDSAuthParameterError;
            
        case kODErrorCredentialsOperationFailed:
            return eDSOperationFailed;
            
        case kODErrorCredentialsServerUnreachable:
            return eDSServiceUnavailable;
            
        case kODErrorCredentialsServerNotFound:
            return eDSAuthNoAuthServerFound;
            
        case kODErrorCredentialsServerCommunicationError:
            return eDSBadPacket;
            
        case kODErrorCredentialsServerError:
            return eDSInvalidSession;
            
        case kODErrorCredentialsServerTimeout:
            return eDSAuthMasterUnreachable;
            
        case kODErrorCredentialsContactMaster:
            return eDSContactMaster;
            
        case kODErrorCredentialsPasswordChangeRequired:
            return eDSAuthNewPasswordRequired;
            
        case kODErrorCredentialsPasswordExpired:
            return eDSAuthPasswordExpired;
            
        case kODErrorCredentialsPasswordQualityFailed:
            return eDSAuthPasswordQualityCheckFailed;
            
        case kODErrorCredentialsAccountDisabled:
            return eDSAuthAccountDisabled;
            
        case kODErrorCredentialsAccountExpired:
            return eDSAuthAccountExpired;
            
        case kODErrorCredentialsAccountInactive:
            return eDSAuthAccountInactive;
            
        case kODErrorCredentialsPasswordTooShort:
            return eDSAuthPasswordTooShort;
            
        case kODErrorCredentialsPasswordTooLong:
            return eDSAuthPasswordTooLong;
            
        case kODErrorCredentialsPasswordNeedsLetter:
            return eDSAuthPasswordNeedsLetter;
            
        case kODErrorCredentialsPasswordNeedsDigit:
            return eDSAuthPasswordNeedsDigit;
            
        case kODErrorCredentialsPasswordChangeTooSoon:
            return eDSAuthPasswordChangeTooSoon;
            
        case kODErrorCredentialsInvalidLogonHours:
            return eDSAuthInvalidLogonHours;
            
        case kODErrorCredentialsInvalidComputer:
            return eDSAuthInvalidComputer;
            
        case kODErrorCredentialsPasswordUnrecoverable:
            return eDSUnrecoverablePassword;
            
        case kODErrorPluginError:
            return eUndefinedError;
			
		case kODErrorNodeDisabled:
			return ePlugInNotActive;
    }
    
    return eUndefinedError;
}

#pragma mark -
#pragma mark ODContext

CFTypeID ODContextGetTypeID( void )
{
    static pthread_once_t registerOnce = PTHREAD_ONCE_INIT;
    
    pthread_once( &registerOnce, _ODContextRegisterClass );

    return _kODContextTypeID;
}

#pragma mark -
#pragma mark Query

CFTypeID ODQueryGetTypeID( void )
{
    static pthread_once_t registerOnce = PTHREAD_ONCE_INIT;
    
    pthread_once( &registerOnce, _ODQueryRegisterClass );
    
    return _kODQueryTypeID;
}

void _ODQueryInitWithNode( ODQueryRef inQueryRef, ODNodeRef inNodeRef, CFTypeRef inRecordTypeOrList, ODAttributeType inAttribute,
                           ODMatchType inMatchType, CFTypeRef inQueryValueOrList, CFTypeRef inReturnAttributeOrList,
                           CFIndex inMaxValues )
{
    _ODQuery *pQuery    = (_ODQuery *) inQueryRef;
    
    pQuery->_ODNode = (_ODNode *) CFRetain( inNodeRef );
    
    pQuery->_maxValues = inMaxValues;
    
    if( NULL == inRecordTypeOrList )
    {
        pQuery->_dsRecTypeList = dsBuildListFromStrings( 0, kDSStdRecordTypeAll, NULL );
    }
    else if( CFGetTypeID(inRecordTypeOrList) == CFStringGetTypeID() )
    {
        char *pTempString = _GetCStringFromCFString( inRecordTypeOrList );
        
        pQuery->_dsRecTypeList = dsBuildListFromStrings( 0, pTempString, NULL );
        
        free( pTempString );
        pTempString = NULL;
    }
    else
    {
        pQuery->_dsRecTypeList = _ConvertCFArrayToDataList( inRecordTypeOrList );
    }
    
    if( NULL == inAttribute )
        pQuery->_dsAttribute = dsDataNodeAllocateString( 0, kDSNAttrRecordName );
    else
        pQuery->_dsAttribute = _GetDataBufferFromCFType( inAttribute );
    
    if( 0 == inMatchType )
        pQuery->_matchType = kODMatchEqualTo;
    else
        pQuery->_matchType = inMatchType;
    
    if( NULL == inAttribute || CFStringCompare(inAttribute, kODAttributeTypeRecordName, kCFCompareCaseInsensitive) == kCFCompareEqualTo )
    {
        pQuery->_bGetRecordList = true;
    }
    
    if( NULL == inQueryValueOrList )
    {
        pQuery->_dsSearchValue = dsDataNodeAllocateString( 0, kDSRecordsAll );
        pQuery->_dsSearchValues = dsBuildListFromStrings( 0, kDSRecordsAll, NULL );
    }
    else if( CFGetTypeID(inQueryValueOrList) == CFArrayGetTypeID() )
    {
        if( CFArrayGetCount(inQueryValueOrList) == 1 )
            pQuery->_dsSearchValue = _GetDataBufferFromCFType( CFArrayGetValueAtIndex(inQueryValueOrList, 0) );

        pQuery->_dsSearchValues = _ConvertCFArrayToDataList( inQueryValueOrList );
    }
    else
    {
        pQuery->_dsSearchValues = dsDataListAllocate( 0 );
        pQuery->_dsSearchValue = _GetDataBufferFromCFType( inQueryValueOrList );
        
        dsBuildListFromNodesAlloc( 0, pQuery->_dsSearchValues, pQuery->_dsSearchValue, NULL );
    }
    
    pQuery->_cfReturnAttribs = CFSetCreateMutable( kCFAllocatorDefault, 0, &kCFTypeSetCallBacks );
    
    if( NULL == inReturnAttributeOrList )
    {
        // if no attributes requested, we only get kDSNAttrMetaNodeLocation since that's all we need for internal
        // purposes
//        pQuery->_dsRetAttrList = dsBuildListFromStrings( 0, kDSNAttrMetaNodeLocation, NULL );
//        CFSetAddValue( pQuery->_cfReturnAttribs, kODAttributeTypeMetaNodeLocation );
        
        pQuery->_dsRetAttrList = dsBuildListFromStrings( 0, kDSAttributesStandardAll, NULL );
        CFSetAddValue( pQuery->_cfReturnAttribs, kODAttributeTypeStandardOnly );
    }
    else if( CFGetTypeID(inReturnAttributeOrList) == CFStringGetTypeID() )
    {
        CFSetAddValue( pQuery->_cfReturnAttribs, inReturnAttributeOrList );
        
        char *pTempString = _GetCStringFromCFString( inReturnAttributeOrList );
        
        pQuery->_dsRetAttrList = dsBuildListFromStrings( 0, pTempString, NULL );
        
        free( pTempString );
        pTempString = NULL;
    }
    else
    {
        CFIndex iCount = CFArrayGetCount( inReturnAttributeOrList );
        CFIndex ii;
        
        // create a set from the array
        for( ii = 0; ii < iCount; ii++ )
        {
            CFSetAddValue( pQuery->_cfReturnAttribs, CFArrayGetValueAtIndex(inReturnAttributeOrList, ii) );
        }
        
        pQuery->_dsRetAttrList = _ConvertCFArrayToDataList( inReturnAttributeOrList );
    }
    
    // if we aren't getting all attributes or all standard attributes
    if ( false == CFSetContainsValue(pQuery->_cfReturnAttribs, kODAttributeTypeAllAttributes) && 
         false == CFSetContainsValue(pQuery->_cfReturnAttribs, kODAttributeTypeStandardOnly) )
    {
        // if we aren't requesting the node location we need to request that for internal purposes
        if ( false == CFSetContainsValue(pQuery->_cfReturnAttribs, kODAttributeTypeMetaNodeLocation) )
        {
            dsAppendStringToListAlloc( 0, pQuery->_dsRetAttrList, kDSNAttrMetaNodeLocation );
            CFSetAddValue( pQuery->_cfReturnAttribs, kODAttributeTypeMetaNodeLocation );
        }
        
        // if we aren't requesting the record name we need to request that for internal purposes
        if ( false == CFSetContainsValue(pQuery->_cfReturnAttribs, kODAttributeTypeRecordName) )
        {
            dsAppendStringToListAlloc( 0, pQuery->_dsRetAttrList, kDSNAttrRecordName );
            CFSetAddValue( pQuery->_cfReturnAttribs, kODAttributeTypeRecordName );
        }
    }
}

ODQueryRef ODQueryCreateWithNode( CFAllocatorRef inAllocator, ODNodeRef inNodeRef, CFTypeRef inRecordTypeOrList, 
                                  ODAttributeType inAttribute, ODMatchType inMatchType, CFTypeRef inQueryValueOrList, 
                                  CFTypeRef inReturnAttributeOrList, CFIndex inMaxValues, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    return _ODQueryCreateWithNode( inAllocator, inNodeRef, inRecordTypeOrList, inAttribute, inMatchType, inQueryValueOrList, 
                                   inReturnAttributeOrList, inMaxValues, outError );
}

ODQueryRef _ODQueryCreateWithNode( CFAllocatorRef inAllocator, ODNodeRef inNodeRef, CFTypeRef inRecordTypeOrList, 
                                   ODAttributeType inAttribute, ODMatchType inMatchType, CFTypeRef inQueryValueOrList, 
                                   CFTypeRef inReturnAttributeOrList, CFIndex inMaxValues, CFErrorRef *outError )
{
    if( NULL == inNodeRef )
    {
        if( NULL != outError && NULL == (*outError) )
        {
			CFStringRef cfDescription = CFSTR("Attempt to create a query failed.");
			CFStringRef cfError = CFSTR("Node was null.");
            _ODErrorSet( outError, kODErrorDomainFramework, eDSNullParameter, &cfDescription,
                         &cfError, NULL );
        }
        return NULL;
    }
    
    _ODQuery *pQuery = _createQuery( inAllocator );
    
    if( NULL != pQuery )
    {
        _ODQueryInitWithNode( (ODQueryRef) pQuery, inNodeRef, inRecordTypeOrList, inAttribute, inMatchType,
                              inQueryValueOrList, inReturnAttributeOrList, inMaxValues );
    }
    else
    {
        if( NULL != outError && NULL == (*outError) )
        {
			CFStringRef cfDescription = CFSTR("Memory allocation failure.");
			CFStringRef cfError = CFSTR("Unable to allocate query.");
            _ODErrorSet( outError, kODErrorDomainFramework, eDSAllocationFailed, 
                         &cfDescription, 
                         &cfError, NULL );
        }
    }
    
    return (ODQueryRef) pQuery;
}

ODQueryRef ODQueryCreateWithNodeType( CFAllocatorRef inAllocator, ODNodeType inType, CFTypeRef inRecordTypeOrList, 
                                      ODAttributeType inAttribute, ODMatchType inMatchType, CFTypeRef inQueryValueOrList, 
                                      CFTypeRef inReturnAttributeOrList, CFIndex inMaxValues, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }

    ODQueryRef  cfSearch = NULL;
    ODNodeRef   cfNode = ODNodeCreateWithNodeType( inAllocator, NULL, inType, outError );
    
    if( NULL != cfNode )
    {
        cfSearch = _ODQueryCreateWithNode( inAllocator, cfNode, inRecordTypeOrList, inAttribute, inMatchType, inQueryValueOrList,
                                           inReturnAttributeOrList, inMaxValues, outError );
        
        CFRelease( cfNode );
        cfNode = NULL;
    }
    
    return cfSearch;
}

CFArrayRef ODQueryCopyResults( ODQueryRef inQueryRef, bool inPartialResults, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    return _ODQueryCopyResults( inQueryRef, inPartialResults, outError );
}

CFArrayRef _ODQueryCopyResults( ODQueryRef inQueryRef, bool inPartialResults, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus    = eDSNoErr;
    CFMutableArrayRef   outResults	= NULL;
    
    if( NULL == inQueryRef )
    {
        cfError = (NULL != outError ? CFSTR("Missing query reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }

    _ODQuery            *pQuery        = (_ODQuery *) inQueryRef;
    UInt32              ulRecordCount   = 0;
    tDataBufferPtr      dsDataBuffer;
    
    // Grab the search lock first in case Synchronize is called
    pthread_mutex_lock( &(pQuery->_mutex) );
    
    if( false == pQuery->_bSearchStarted )
    {
        ulRecordCount = pQuery->_maxValues;
        pQuery->_bSearchStarted = true;
    }
    else if( 0 == pQuery->_dsContext )
    {
        pthread_mutex_unlock( &(pQuery->_mutex) );
        return NULL;
    }
    
    dsDataBuffer = dsDataBufferAllocate( 0, 128 * 1024 ); // allocate a 128k buffer
    if( NULL == dsDataBuffer )
    {
        pthread_mutex_unlock( &(pQuery->_mutex) );

        cfError = (NULL != outError ? CFSTR("Unable to allocate buffer for response.") : CFSTR(""));
        dsStatus = eDSAllocationFailed;
        goto finish;
    }
    
    outResults = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
    if( NULL == outResults )
    {
        dsDataBufferDeAllocate( 0, dsDataBuffer );
        dsDataBuffer = NULL;
        
        pthread_mutex_unlock( &(pQuery->_mutex) );

        cfError = (NULL != outError ? CFSTR("Unable to allocate array for results.") : CFSTR(""));
        dsStatus = eDSAllocationFailed;
        goto finish;
    }
    
    _ODNode     *pODNode    = pQuery->_ODNode;
    bool     bFailedOnce = false;
    
    _ODNodeLock( pODNode );
    
    do
    {
        do
        {
            if( true == pQuery->_bGetRecordList )
            {
                dsStatus = dsGetRecordList( pODNode->_dsNodeRef, dsDataBuffer, pQuery->_dsSearchValues, pQuery->_matchType,
                                            pQuery->_dsRecTypeList, pQuery->_dsRetAttrList, false, &ulRecordCount, 
                                            &(pQuery->_dsContext) );
            }
            else if( NULL != pQuery->_dsSearchValue )
            {
                dsStatus = dsDoAttributeValueSearchWithData( pODNode->_dsNodeRef, dsDataBuffer, pQuery->_dsRecTypeList,
                                                             pQuery->_dsAttribute, pQuery->_matchType, pQuery->_dsSearchValue, 
                                                             pQuery->_dsRetAttrList, false, &ulRecordCount, &(pQuery->_dsContext) );
            }
            else
            {
                dsStatus = dsDoMultipleAttributeValueSearchWithData( pODNode->_dsNodeRef, dsDataBuffer, pQuery->_dsRecTypeList, 
                                                                     pQuery->_dsAttribute, pQuery->_matchType, pQuery->_dsSearchValues, 
                                                                     pQuery->_dsRetAttrList, false, &ulRecordCount, &(pQuery->_dsContext) );
            }
            
            if( eDSBufferTooSmall == dsStatus )
            {
                UInt32 newSize = (dsDataBuffer->fBufferSize << 1);
                
                if( newSize < 100 * 1024 * 1024 )
                {
                    dsDataBufferDeAllocate( 0, dsDataBuffer );
                    dsDataBuffer = dsDataBufferAllocate( 0, newSize );
                }
                else
                {
                    cfError = (NULL != outError ? CFSTR("Unable to increase buffer for results.") : CFSTR(""));
                    dsStatus = eDSAllocationFailed;
                    break;
                }
            }
            
            if( (eDSInvalidNodeRef == dsStatus || eDSInvalidReference == dsStatus || eDSInvalidDirRef == dsStatus || eDSCannotAccessSession == dsStatus || eDSInvalidRefType == dsStatus) && 
                bFailedOnce == false && (0 == pQuery->_dsContext || false == inPartialResults) )
            {
                dsReleaseContinueData( pODNode->_ODSession->_dsRef, pQuery->_dsContext );
                pQuery->_dsContext = 0;
                
                dsStatus = dsVerifyDirRefNum( pODNode->_ODSession->_dsRef );
                if( eDSInvalidReference == dsStatus || eDSInvalidDirRef == dsStatus || eDSInvalidRefType == dsStatus )
                {
                    dsStatus = _ReopenDS( pODNode->_ODSession );
                    if( eDSNoErr == dsStatus )
                    {
                        // need to reopen the node ref because if DS ref is invalid
                        dsStatus = _ReopenNode( pODNode );
                    }
                }
                else // well if dsRef is valid, then it must be the node that is invalid
                {
                    dsStatus = _ReopenNode( pODNode );
                }
                
                if( eDSNoErr == dsStatus )
                {
                    // reset error status to buffertoosmall to cause the loop to continue
                    dsStatus = eDSBufferTooSmall;
                }
                
                // remove all the values if we failed and restart the search again
                CFArrayRemoveAllValues( outResults );
                
                bFailedOnce = true;
            }
        } while( eDSBufferTooSmall == dsStatus );
        
        if( eDSNoErr == dsStatus )
        {
            _AppendRecordsToList( pODNode, dsDataBuffer, ulRecordCount, outResults, outError );
        }
        
    } while( false == inPartialResults && eDSNoErr == dsStatus && 0 != pQuery->_dsContext );
    
    _ODNodeUnlock( pODNode );
    
    // now unlock the search reference
    pthread_mutex_unlock( &(pQuery->_mutex) );
    
    // loop through any results and set the fetched attributes
    if( NULL != outResults )
    {
        CFIndex iCount = CFArrayGetCount( outResults );
        CFIndex ii;
        
        for( ii = 0; ii < iCount; ii++ )
        {
            _ODRecord *pRecord = (_ODRecord *) CFArrayGetValueAtIndex( outResults, ii );
            
            if( ODRecordGetTypeID() == CFGetTypeID( (CFTypeRef) pRecord ) )
            {
                if( NULL != pRecord->_cfFetchedAttributes )
                {
                    CFRelease( pRecord->_cfFetchedAttributes );
                }
                pRecord->_cfFetchedAttributes = (CFSetRef) CFRetain( pQuery->_cfReturnAttribs );
            }
        }
    }
    
    if( NULL != dsDataBuffer )
    {
        dsDataBufferDeAllocate( 0, dsDataBuffer );
        dsDataBuffer = NULL;
    }
    
    if( eDSNoErr != dsStatus )
    {
        CFRelease( outResults );
        outResults = NULL;
    }

    // _MapDSErrorToReason will return a non-NULL value if there is already an error set
    if (cfError == NULL) {
		cfError = _MapDSErrorToReason( outError, dsStatus );
	}
    
finish:
        
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
			CFStringRef cfDescription = CFSTR("Unable to retrieve results for the query.");
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }
    }    
    
    return outResults;
}

void ODQuerySynchronize( ODQueryRef inQueryRef )
{
    _ODQuery *pQuery = (_ODQuery *) inQueryRef;
    
    if( NULL == inQueryRef )
        return;
    
    pthread_mutex_lock( &(pQuery->_mutex) );
    
    // throw out existing results
    if( NULL != pQuery->_results )
    {
        CFRelease( pQuery->_results );
        pQuery->_results = NULL;
    }
    
    // now reset stuff
    pQuery->_bStopSearch = false;
	pQuery->_bRestartSearch = true;
    
    if( 0 != pQuery->_dsContext )
    {
        dsReleaseContinueData( 0, pQuery->_dsContext );
        pQuery->_dsContext = 0;
    }

    pthread_mutex_unlock( &(pQuery->_mutex) );
}

void ODQueryCancel(ODQueryRef inQueryRef)
{
	_ODQuery *pQuery = (_ODQuery *) inQueryRef;

	pQuery->_bStopSearch = true;
}

bool ODQueryCancelled(ODQueryRef inQueryRef)
{
    _ODQuery *pQuery = (_ODQuery *) inQueryRef;
	
	return pQuery->_bStopSearch;
}

bool ODQueryRestarted(ODQueryRef inQueryRef)
{
    _ODQuery *pQuery = (_ODQuery *) inQueryRef;
	
	return pQuery->_bRestartSearch;
}

void ODQuerySetRequestID(ODQueryRef inQueryRef, uint64_t reqid)
{
    _ODQuery *pQuery = (_ODQuery *) inQueryRef;
	
	pQuery->_requestID = reqid;
}

uint64_t ODQueryGetRequestID(ODQueryRef inQueryRef)
{
    _ODQuery *pQuery = (_ODQuery *) inQueryRef;
	
	return pQuery->_requestID;
}

dispatch_queue_t ODQueryGetDispatchQueue(ODQueryRef inQueryRef)
{
    _ODQuery *pQuery = (_ODQuery *) inQueryRef;
	
	return pQuery->_dispatchQueue;
}

CFDataRef ODQueryGetQueryID(ODQueryRef inQueryRef)
{
    _ODQuery *pQuery = (_ODQuery *) inQueryRef;
	
	return pQuery->_queryID;
}

void ODQuerySetQueryID(ODQueryRef inQueryRef, CFDataRef queryID)
{
    _ODQuery *pQuery = (_ODQuery *) inQueryRef;
	
	pQuery->_queryID = CFRetain(queryID);
}

void ODQuerySetPID(ODQueryRef inQueryRef, pid_t pid)
{
    _ODQuery *pQuery = (_ODQuery *) inQueryRef;
	
	pQuery->_pid = pid;
}

pid_t ODQueryGetPID(ODQueryRef inQueryRef)
{
    _ODQuery *pQuery = (_ODQuery *) inQueryRef;
	
	return pQuery->_pid;
}

mach_port_t	ODQueryGetReplyPort(ODQueryRef inQueryRef)
{
    _ODQuery *pQuery = (_ODQuery *) inQueryRef;
	
	return pQuery->_replyPort;
}

void ODQuerySetReplyPort(ODQueryRef inQueryRef, mach_port_t replyPort)
{
    _ODQuery *pQuery = (_ODQuery *) inQueryRef;
	
	pQuery->_replyPort = replyPort;
}

void ODQuerySetExternalSession(ODQueryRef inQueryRef, bool externalSession)
{
    _ODQuery *pQuery = (_ODQuery *) inQueryRef;
	
	pQuery->_externalSession = externalSession;
}

bool ODQueryIsExternalSession(ODQueryRef inQueryRef)
{
    _ODQuery *pQuery = (_ODQuery *) inQueryRef;
	
	return pQuery->_externalSession;
}

#pragma mark -
#pragma mark ODSession functions

CFTypeID ODSessionGetTypeID( void )
{
    static pthread_once_t registerOnce = PTHREAD_ONCE_INIT;
    
    pthread_once( &registerOnce, _ODSessionRegisterClass );

    return _kODSessionTypeID;
}

bool _ODSessionInit( ODSessionRef inSession, CFDictionaryRef inOptions, CFErrorRef *outError )
{
    tDirStatus  dsStatus    = eDSNoErr;
    _ODSession  *result     = (_ODSession *) inSession;
    
    if( NULL != inOptions )
    {
        CFTypeRef   cfRef;
        CFStringRef cfErrorLocalized    = NULL;
        
        // check for usage of kODSessionLocalPath first, otherwise assume it is a proxy
        cfRef = CFDictionaryGetValue( inOptions, kODSessionLocalPath );
        if( NULL != cfRef && CFGetTypeID(cfRef) == CFStringGetTypeID() )
        {
            CFDictionarySetValue( result->_info, kODSessionLocalPath, cfRef );
        }
        else
        {
            cfRef = CFDictionaryGetValue( inOptions, kODSessionProxyAddress );
            if( eDSNoErr == dsStatus && NULL != cfRef && CFGetTypeID(cfRef) == CFStringGetTypeID() )
            {
                CFDictionarySetValue( result->_info, kODSessionProxyAddress, cfRef );
            }
            else
            {
                dsStatus = eDSNullParameter;
                cfErrorLocalized = CFSTR("Missing proxy address.");
            }
            
            cfRef = CFDictionaryGetValue( inOptions, kODSessionProxyPort );
            if( eDSNoErr == dsStatus && NULL != cfRef && CFGetTypeID(cfRef) == CFNumberGetTypeID() )
            {
                CFDictionarySetValue( result->_info, kODSessionProxyPort, cfRef );
            }
            else
            {
                CFIndex         iPortNumber     = 625;
                CFNumberRef     cfPortNumber    = CFNumberCreate( kCFAllocatorDefault, kCFNumberCFIndexType, &iPortNumber );
                
                CFDictionarySetValue( result->_info, kODSessionProxyPort, cfPortNumber );
                
                CFRelease( cfPortNumber );
                cfPortNumber = NULL;
            }
            
            cfRef = CFDictionaryGetValue( inOptions, kODSessionProxyUsername );
            if( eDSNoErr == dsStatus && NULL != cfRef && CFGetTypeID(cfRef) == CFStringGetTypeID() )
            {
                CFDictionarySetValue( result->_info, kODSessionProxyUsername, cfRef );
            }
            else
            {
                dsStatus = eDSNullParameter;
                cfErrorLocalized = CFSTR("Missing proxy username.");
            }
            
            cfRef = CFDictionaryGetValue( inOptions, kODSessionProxyPassword );
            if( eDSNoErr == dsStatus && NULL != cfRef && CFGetTypeID(cfRef) == CFStringGetTypeID() )
            {
                result->_cfProxyPassword = CFStringCreateCopy( kCFAllocatorDefault, cfRef );
            }
            else
            {
                dsStatus = eDSNullParameter;
                cfErrorLocalized = CFSTR("Missing proxy password.");
            }
        }
        
        if( eDSNullParameter == dsStatus )
        {
            if( NULL != outError && NULL == (*outError) )
            {
				CFStringRef cfDescription = CFSTR("Unable to open session.");
                _ODErrorSet( outError, kODErrorDomainFramework, eDSNullParameter, 
                             &cfDescription, &cfErrorLocalized,
                             NULL );
            }
            else
            {
                CFRelease( cfErrorLocalized );
                cfErrorLocalized = NULL;
            }
        }
    }
    
    if( eDSNoErr == dsStatus )
    {
        dsStatus = _ReopenDS( result );
        
        if( eDSNoErr != dsStatus )
        {
            if( NULL != outError && NULL == (*outError) )
            {
				CFStringRef cfDescription = CFSTR("Unable to open session.");
				CFStringRef cfError = _MapDSErrorToReason( outError, dsStatus );
                _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                             &cfDescription, 
                             &cfError,
                             NULL );
            }
            
            return false;
        }
    }
    
    return true;
}

ODSessionRef ODSessionCreate( CFAllocatorRef inAllocator, CFDictionaryRef inOptions, CFErrorRef *outError )
{
    _ODSession  *result     = NULL;

    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    // Create the base object
    result = _createSession( inAllocator );
    if( NULL != result )
    {
        if( _ODSessionInit( (ODSessionRef) result, inOptions, outError ) == false )
        {
            CFRelease( (CFTypeRef) result );
            result = NULL;
        }
    }
    else
    {
        if( NULL != outError && NULL == (*outError) )
        {
			CFStringRef cfDescription = CFSTR("Allocation failed.");
			CFStringRef cfError = CFSTR("Unable to allocate array for results.");
            _ODErrorSet( outError, kODErrorDomainFramework, eDSAllocationFailed, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
    }
    
    return (ODSessionRef)result;
}

ODSessionRef ODSessionCreateWithDSRef( CFAllocatorRef inAllocator, tDirReference inDirRef, bool inCloseOnRelease )
{
    if( 0 == inDirRef )
    {
        return NULL;
    }
    
    _ODSession  *result     = NULL;
    tDirStatus  dsStatus    = dsVerifyDirRefNum( inDirRef );
    
    if( eDSNoErr == dsStatus )
    {
        result = _createSession( inAllocator );
        
        if( NULL != result )
        {
            result->_closeRef = inCloseOnRelease;
            result->_dsRef = inDirRef;
        }
    }
    
    return (ODSessionRef) result;
}

tDirReference ODSessionGetDSRef( ODSessionRef inSessionRef )
{
    if( NULL == inSessionRef )
    {
        return 0;
    }
    
    _ODSession *pODSession    = (_ODSession *) inSessionRef;
    
    return pODSession->_dsRef;
}

static void
__ODSessionSharedInit( void )
{
    kODSessionDefault = ODSessionCreate( kCFAllocatorDefault, NULL, NULL );
}

ODSessionRef _ODSessionGetShared( void )
{
    static pthread_once_t   _ODSessionInitialized   = PTHREAD_ONCE_INIT;

    pthread_once( &_ODSessionInitialized, __ODSessionSharedInit );
    
    return kODSessionDefault;
}

CFArrayRef ODSessionCopyNodeNames( CFAllocatorRef inAllocator, ODSessionRef inSessionRef, CFErrorRef *outError )
{
    CFMutableArrayRef   returnValue = NULL;
    tDirStatus          dsStatus;
    
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
    
    if ( NULL == inSessionRef )
        inSessionRef = _ODSessionGetShared();
    
    if ( NULL == inSessionRef )
    {
        dsStatus = eServerNotRunning;
        goto failed;
    }
    
    _ODSession      *pSession       = (_ODSession *) inSessionRef;
    tDataBufferPtr  dsDataBuffer    = dsDataBufferAllocate( 0, 1024 );
    UInt32          nodeCount;
    tContextData    dsContext   = 0;
    
    returnValue = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
    
    pthread_mutex_lock( &pSession->_mutex );
    
    while( 1 )
    {
        dsStatus = dsGetDirNodeList( pSession->_dsRef, dsDataBuffer, &nodeCount, &dsContext );
        
        if( eDSBufferTooSmall == dsStatus )
        {
            UInt32 newSize = (dsDataBuffer->fBufferSize << 1);
            
            if( newSize < 100 * 1024 * 1024 )
            {
                dsDataBufferDeAllocate( 0, dsDataBuffer );
                dsDataBuffer = dsDataBufferAllocate( 0, newSize );
            }
            else
            {
                dsStatus = eDSAllocationFailed;
                break;
            }
            continue;
        }
        
        if( eDSInvalidReference == dsStatus || eDSInvalidDirRef == dsStatus || eDSInvalidRefType == dsStatus )
        {
            dsStatus = _ReopenDS( pSession );
            if( 0 != dsContext )
            {
                dsReleaseContinueData( pSession->_dsRef, dsContext );
                dsContext = 0;
            }
            CFArrayRemoveAllValues( returnValue );
            continue;
        }
        
        if( eDSNoErr == dsStatus )
        {
            UInt32 ii;
            
            for (ii = 1; ii <= nodeCount; ii++)
            {
                tDataListPtr    dsNodeName = NULL;
                
                dsStatus = dsGetDirNodeName( 0, dsDataBuffer, ii, &dsNodeName );
                if( dsStatus != eDSNoErr )
                {
                    break;
                }
                
                char        *nodeName = dsGetPathFromList( 0, dsNodeName, "/" );
                CFStringRef cfNodeName = CFStringCreateWithCString( kCFAllocatorDefault, nodeName, kCFStringEncodingUTF8 );
                
                if( NULL != cfNodeName )
                {
                    CFArrayAppendValue( returnValue, cfNodeName );
                    CFRelease( cfNodeName );
                }
                
                free( nodeName );
                dsDataListDeallocate( 0, dsNodeName );
                free( dsNodeName );
            }
        }
        
        if( dsContext != 0 )
        {
            continue;
        }
        
        break;
    }
    
    if( 0 != dsContext )
    {
        dsReleaseContinueData( pSession->_dsRef, dsContext );
    }
    
    pthread_mutex_unlock( &pSession->_mutex );
    
failed:
    
    if( eDSNoErr != dsStatus )
    {
        if ( returnValue != NULL )
        {
            CFRelease( returnValue );
            returnValue = NULL;
        }
        
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfError = _MapDSErrorToReason( outError, dsStatus );
            
            if( NULL != cfError )
            {
                CFStringRef cfDescription = CFSTR("Unable to retrieve available node names for session.");
                _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, &cfDescription, &cfError, NULL );        
            }
        }
    }
    
    return returnValue;
}

#pragma mark -
#pragma mark ODNode functions

CFTypeID ODNodeGetTypeID( void )
{
    static pthread_once_t registerOnce = PTHREAD_ONCE_INIT;
    
    pthread_once( &registerOnce, _ODNodeRegisterClass );

    return _kODNodeTypeID;
}

static void _checkForRecordBlat( ODNodeRef inNodeRef )
{
    // no longer required
#if 0
    CFErrorRef                  localError  = NULL;
	AuthorizationExternalForm   form        = { { 0 } };
    CFDataRef                   sendData    = CFDataCreateWithBytesNoCopy( kCFAllocatorDefault, (UInt8 *) &form, sizeof(form), kCFAllocatorNull );
    CFDataRef                   recvData    = ODNodeCustomCall( inNodeRef, eDSCustomCallExtendedRecordCallsAvailable, sendData, &localError );
    
    if ( recvData != NULL ) {
        if ( localError == NULL ) {
            _ODNode *node = (_ODNode *) inNodeRef;
            if ( CFDataGetLength(recvData) > 0 ) {
                const char *bytes = (const char *) CFDataGetBytePtr( recvData );
                
                if ( bytes != NULL && bytes[0] != 0 ) {
                    node->_flags |= kODNodeFlagRecordBlat;
                }
            }
        }
        
        CFRelease( recvData );
        recvData = NULL;
    }
    
    if ( localError != NULL ) {
        CFRelease( localError );
        localError = NULL;
    }
    
    CFRelease( sendData );
    sendData = NULL;
#endif
}

tDirStatus _ODNodeInitWithType( ODNodeRef inNodeRef, ODSessionRef inSessionRef, ODNodeType inType, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    tDirStatus      dsStatus    = eServerNotRunning;

    if ( NULL == inSessionRef )
        inSessionRef = _ODSessionGetShared();
    
    if ( NULL != inSessionRef )
    {
        _ODNode     *result = (_ODNode *) inNodeRef;
        
        result->_ODSession = (_ODSession *) CFRetain( inSessionRef );
        result->_nodeType = inType;
        
        dsStatus = _FindDirNode( result, inType, outError );
        if (dsStatus == eDSNoErr && inType == kODNodeTypeLocalNodes) {
			_checkForRecordBlat( inNodeRef );
        }
    }

    // _MapDSErrorToReason will return a non-NULL value if there is already an error set
    CFStringRef cfError = _MapDSErrorToReason( outError, dsStatus );
    
    if( NULL != cfError )
    {
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfTemp = CFSTR("Unable to open Directory node of type %d.");
            CFStringRef cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inType );
            CFRelease( cfTemp );
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription,
                         &cfError,
                         NULL );        
        }
        else
        {
            CFRelease( cfError );
        }
    }
    
    return dsStatus;
}

tDirStatus _ODNodeInitWithName( CFAllocatorRef inAllocator, ODNodeRef inNodeRef, ODSessionRef inSessionRef, CFStringRef inNodeName, CFErrorRef *outError )
{
    char            *pNodeName  = _GetCStringFromCFString( inNodeName );
    tDataListPtr    dsNodeName  = dsBuildFromPath( 0, pNodeName, "/" );
    _ODNode         *result     = (_ODNode *) inNodeRef;
    tDirStatus      dsStatus    = eDSOpenNodeFailed;

    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inSessionRef )
    {
        inSessionRef = _ODSessionGetShared();
    }
    
    if( NULL != inSessionRef )
    {
        result->_ODSession = (_ODSession *) CFRetain( inSessionRef );
        
        bool bFailedOnce = false;
        while( 1 )
        {
            dsStatus = dsOpenDirNode( result->_ODSession->_dsRef, dsNodeName, &result->_dsNodeRef );
            if( false == bFailedOnce && (eDSInvalidReference == dsStatus || eDSInvalidDirRef == dsStatus || eDSInvalidRefType == dsStatus) )
            {
                dsStatus = _ReopenDS( result->_ODSession );
                bFailedOnce = true;
                continue;
            }
            break;
        };

        if( eDSNoErr == dsStatus )
        {
			static CFArrayRef ignoreList;
			static dispatch_once_t once;
			
			dispatch_once(&once, 
						  ^(void) {
							  CFTypeRef theList[] = { CFSTR("/Search"), CFSTR("/Configure"), CFSTR("/BSD"), CFSTR("/Contacts"), CFSTR("/PasswordServer"), CFSTR("/Cache") };
							  ignoreList = CFArrayCreate(kCFAllocatorDefault, theList, sizeof(theList) / sizeof(*theList), &kCFTypeArrayCallBacks);
						  });
			
            CFDictionarySetValue( result->_info, kODNodeNameKey, inNodeName );
			
			// only check blat for specific node lists
			if (CFArrayContainsValue(ignoreList, CFRangeMake(0, CFArrayGetCount(ignoreList)), inNodeName) == false) {
				_checkForRecordBlat( inNodeRef );
			}
        }
    }
    
    if( NULL != dsNodeName )
    {
        dsDataListDeallocate( 0, dsNodeName );
        
        free( dsNodeName );
        dsNodeName = NULL;
    }
    
    if( NULL != pNodeName )
    {
        free( pNodeName );
        pNodeName = NULL;
    }
    
    if( NULL != outError && NULL == (*outError) )
    {
        // _MapDSErrorToReason will return a non-NULL value if there is already an error set
        CFStringRef cfError = _MapDSErrorToReason( outError, dsStatus );

        if( NULL != cfError )
        {
            CFStringRef cfTemp = CFSTR("Unable to open Directory node with name %@.");
            CFStringRef cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inNodeName );
            CFRelease( cfTemp );
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription,
                         &cfError,
                         NULL );        
        }
    }
    
    return dsStatus;
}

ODNodeRef ODNodeCreate( CFAllocatorRef inAllocator, ODSessionRef inSessionRef, CFErrorRef *outError )
{
    return ODNodeCreateWithNodeType( inAllocator, inSessionRef, kODNodeTypeAuthentication, outError );
}

ODNodeRef ODNodeCreateWithNodeType( CFAllocatorRef inAllocator, ODSessionRef inSessionRef, ODNodeType inType, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    return _ODNodeCreateWithNodeType( inAllocator, inSessionRef, inType, outError );
}

ODNodeRef _ODNodeCreateWithNodeType( CFAllocatorRef inAllocator, ODSessionRef inSessionRef, ODNodeType inType, CFErrorRef *outError )
{
    _ODNode     *result     = NULL;
    tDirStatus  dsStatus    = eDSAllocationFailed;
    
    // Create the base object
    result = _createNode( inAllocator );
    
    if( NULL != result )
    {
        dsStatus = _ODNodeInitWithType( (ODNodeRef) result, inSessionRef, inType, outError );
    }
    
    if( eDSNoErr != dsStatus )
    {
        CFRelease( (CFTypeRef) result );
        result = NULL;
    }
    
    return (ODNodeRef)result;
}

ODNodeRef ODNodeCreateWithName( CFAllocatorRef inAllocator, ODSessionRef inSessionRef, CFStringRef inNodeName, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    return _ODNodeCreateWithName( inAllocator, inSessionRef, inNodeName, outError );
}

ODNodeRef _ODNodeCreateWithName( CFAllocatorRef inAllocator, ODSessionRef inSessionRef, CFStringRef inNodeName, CFErrorRef *outError )
{
    if( NULL == inNodeName )
    {
        if( NULL != outError && NULL == (*outError) )
        {
			CFStringRef cfDescription = CFSTR("Unable to open Directory node.");
			CFStringRef cfError = CFSTR("Missing node name.");
            _ODErrorSet( outError, kODErrorDomainFramework, eDSNullParameter, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        return NULL;
    }
    
    _ODNode         *result     = NULL;
    tDirStatus      dsStatus    = eDSAllocationFailed;

    // Create the base object
    result = _createNode( inAllocator );
    
    if( NULL != result )
    {
        dsStatus = _ODNodeInitWithName( inAllocator, (ODNodeRef) result, inSessionRef, inNodeName, outError );
    }
    
    if( eDSNoErr != dsStatus )
    {
        CFRelease( (CFTypeRef) result );
        result = NULL;
    }
    
    return (ODNodeRef) result;
}

ODNodeRef ODNodeCreateCopy( CFAllocatorRef inAllocator, ODNodeRef inNodeRef, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    return _ODNodeCreateCopy( inAllocator, inNodeRef, outError );
}

ODNodeRef _ODNodeCreateCopy( CFAllocatorRef inAllocator, ODNodeRef inNodeRef, CFErrorRef *outError )
{
    _ODNode *pODNodeRef = (_ODNode *) inNodeRef;
    
    if( NULL == inNodeRef )
    {
        if( NULL != outError && NULL == (*outError) )
        {
			CFStringRef cfDescripton = CFSTR("Unable to make a copy of the node.");
			CFStringRef cfError = CFSTR("Missing node reference.");
            _ODErrorSet( outError, kODErrorDomainFramework, eDSNullParameter, 
                         &cfDescripton, 
                         &cfError,
                         NULL );
        }
        return NULL;
    }
    
    CF_OBJC_FUNCDISPATCH( _kODNodeTypeID, ODNodeRef, inNodeRef, "copy" );
    
    // Create the base object
    _ODNode *result = _createNode( inAllocator );
    
    // Set the names only if succeeded
    if( NULL != result )
    {
        // Release the current, because a new one will be laid down
        CFRelease( result->_info );
        
        _ODNodeLock( pODNodeRef );
        
        // Just make a copy of all the information
        result->_info = CFDictionaryCreateMutableCopy( inAllocator, 0, pODNodeRef->_info );
        if( NULL != result->_info )
        {
            result->_nodeType = pODNodeRef->_nodeType;
            
            if( NULL != pODNodeRef->_cfNodePassword )
            {
                result->_cfNodePassword = CFStringCreateCopy( inAllocator, pODNodeRef->_cfNodePassword );
            }
            
            result->_ODSession = (_ODSession *) CFRetain( pODNodeRef->_ODSession );
        }
        // If it failed, release the new one and return null
        else
        {
            CFRelease( (CFTypeRef) result );
            result = NULL;
        }
        _ODNodeUnlock( pODNodeRef );
        
        if( NULL != result )
        {
            _ReopenNode( result );
        }
    }
    
    return (ODNodeRef)result;
}

ODNodeRef ODNodeCreateWithDSRef( CFAllocatorRef inAllocator, tDirReference inDirRef, tDirNodeReference inNodeRef, 
                                 bool inCloseOnRelease )
{
    if( 0 == inDirRef || 0 == inNodeRef )
    {
        return NULL;
    }
    
    _ODNode     *result     = NULL;
    tDirStatus  dsStatus    = dsVerifyDirRefNum( inDirRef );
    CFStringRef cfNodePath  = NULL;
    
    if( eDSNoErr == dsStatus )
    {
        tContextData            dsContext       = 0;
        tDataListPtr            dsInfoType      = dsBuildListFromStrings( 0, kDSNAttrNodePath, NULL );
        tDataBufferPtr          dsDataBuffer    = dsDataBufferAllocate( 0, 8 * 1024 ); // allocate a 8k buffer
        tAttributeListRef       dsAttribList    = 0;
        UInt32                  ulCount         = 0;
        
        if( NULL == dsDataBuffer )
        {
            dsStatus = eDSAllocationFailed;
			goto failed;
        }
        
        dsStatus = dsGetDirNodeInfo( inNodeRef, dsInfoType, dsDataBuffer, false, &ulCount, &dsAttribList, &dsContext );
        if( eDSNoErr == dsStatus )
        {
            CFDictionaryRef cfDict = _CopyAttributesFromBuffer( inNodeRef, dsDataBuffer, dsAttribList, ulCount, NULL );
            
            if( NULL != cfDict )
            {
                CFMutableArrayRef   cfPath = (CFMutableArrayRef) CFDictionaryGetValue( cfDict, kODAttributeTypeNodePath );
                
                if( NULL != cfPath )
                {
                    CFArrayInsertValueAtIndex( cfPath, 0, CFSTR("") ); // need to add a "/" to the beginning
                    cfNodePath = CFStringCreateByCombiningStrings( kCFAllocatorDefault, cfPath, CFSTR("/") );
                }
                
                CFRelease( cfDict );
                cfDict = NULL;
            }
        }
	failed:  
        if( 0 != dsAttribList )
        {
            dsCloseAttributeList( dsAttribList );
            dsAttribList = 0;
        }
        
        if( NULL != dsDataBuffer )
        {
            dsDataBufferDeAllocate( 0, dsDataBuffer );
            dsDataBuffer = NULL;
        }
        
        if( NULL != dsInfoType )
        {
            dsDataListDeallocate( 0, dsInfoType );
            
            free( dsInfoType );
            dsInfoType = NULL;
        }
        
        if( eDSNoErr == dsStatus )
        {
            _ODSession  *session    = _createSession( inAllocator );
            
            if( NULL != session )
            {
                session->_closeRef = inCloseOnRelease;
                session->_dsRef = inDirRef;
                
                // Create the base object
                result = _createNode( inAllocator );
                
                // Set the names only if succeeded
                if( NULL != result )
                {
                    result->_flags = (inCloseOnRelease ? kODNodeFlagCloseRef : 0);
                    result->_ODSession = session;
                    result->_dsNodeRef = inNodeRef;
                    
                    if( NULL != cfNodePath )
                    {
                        CFDictionarySetValue( result->_info, kODNodeNameKey, cfNodePath );
                        
                        CFRelease( cfNodePath );
                        cfNodePath = NULL;
                    }
                }
                else
                {
                    CFRelease( (CFTypeRef) session );
                }
            }
        }
    }
    
    if( NULL != cfNodePath )
    {
        CFRelease( cfNodePath );
        cfNodePath = NULL;
    }    
    
    return (ODNodeRef) result;
}

CFArrayRef ODNodeCopySubnodeNames( ODNodeRef inNodeRef, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inNodeRef )
    {
        if( NULL != outError && NULL == (*outError) )
        {
			CFStringRef cfDescription = CFSTR("Unable to determine subnodes.");
			CFStringRef cfError = CFSTR("Missing node reference.");
            _ODErrorSet( outError, kODErrorDomainFramework, eDSNullParameter, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        return NULL;
    }

    CFStringRef         cfSearchPath    = CFSTR( kDS1AttrSearchPath );
    CFArrayRef          cfValues        = CFArrayCreate( kCFAllocatorDefault, (const void **) &cfSearchPath, 1,
                                                         &kCFTypeArrayCallBacks );
    CFDictionaryRef     cfDict          = _ODNodeCopyDetails( inNodeRef, cfValues, outError );
    CFMutableArrayRef   cfArray         = NULL;

    if( NULL != cfDict )
    {
        cfArray = (CFMutableArrayRef) CFDictionaryGetValue( cfDict, cfSearchPath );
        
        // if we got a value, be sure to retain it because it will be released when we release dictionary
        if( NULL != cfArray )
        {
            CFRetain( cfArray );
        }
        
        CFRelease( cfDict );
        cfDict = NULL;
    }
    
    CFRelease( cfValues );
    cfValues = NULL;
    
    return cfArray;
}

CFArrayRef ODNodeCopyUnreachableSubnodeNames( ODNodeRef inNodeRef, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inNodeRef )
    {
        if( NULL != outError && NULL == (*outError) )
        {
			CFStringRef cfDescription = CFSTR("Unable to determine unreachable subnodes.");
			CFStringRef cfError = CFSTR("Missing node reference.");
            _ODErrorSet( outError, kODErrorDomainFramework, eDSNullParameter, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        return NULL;
    }
    
    // if it's the search node, ask it what it thinks is unreachable.
    // prevents a race condition where the search node has yet to open a node
    // that really is available.
    if( ((_ODNode *)inNodeRef)->_nodeType == kODNodeTypeAuthentication ||
        ((_ODNode *)inNodeRef)->_nodeType == kODNodeTypeContacts )
    {
        CFDataRef data = ODNodeCustomCall( inNodeRef, eDSCustomCallSearchSubNodesUnreachable, NULL, outError);
        CFArrayRef returnValue = NULL;
        if ( data != NULL )
        {
            returnValue = (CFArrayRef)CFPropertyListCreateFromXMLData( NULL, data, 0, NULL);
            if ( returnValue != NULL && 0 == CFArrayGetCount( returnValue ) )
            {
                CFRelease( returnValue );
                returnValue = NULL;
            }
            
            CFRelease( data );
            data = NULL;
        }
        
        return returnValue;
    }

    CFStringRef         cfSearchPath    = CFSTR( kDS1AttrSearchPath );
    CFArrayRef          cfValues        = CFArrayCreate( kCFAllocatorDefault, (const void **) &cfSearchPath, 1, 
                                                         &kCFTypeArrayCallBacks );
    CFDictionaryRef     cfDict          = _ODNodeCopyDetails( inNodeRef, cfValues, outError );
    CFMutableArrayRef   cfArray         = NULL;
    
    if( NULL != cfDict )
    {
        cfArray = (CFMutableArrayRef) CFDictionaryGetValue( cfDict, cfSearchPath );
        
        // if we got a value, be sure to retain it because it will be released when we release dictionary
        if( NULL != cfArray )
        {
            // make a mutable copy
            cfArray = CFArrayCreateMutableCopy( kCFAllocatorDefault, 0, cfArray );
            
            CFIndex iCount = CFArrayGetCount( cfArray );
            CFIndex ii;
            
            // go in reverse since we are going to remove values as we go
            for( ii = iCount - 1; ii >= 0; ii-- )
            {
                CFStringRef cfNodeName  = CFArrayGetValueAtIndex( cfArray, ii );
                _ODNode     *pODNodeRef = (_ODNode *)inNodeRef;
                ODNodeRef   cfNode      = _ODNodeCreateWithName( kCFAllocatorDefault, (ODSessionRef) pODNodeRef->_ODSession, cfNodeName, NULL );
                
                if( NULL != cfNode )
                {
                    // if it is Active directory, we need to check the GetDirNodeInfo to tell if working
                    if( true == CFStringHasPrefix(cfNodeName, CFSTR("/Active Directory/")) )
                    {
                        CFStringRef     cfAvailString   = CFSTR("dsAttrTypeStandard:NodeAvailability");
                        CFArrayRef      cfKeys          = CFArrayCreate( kCFAllocatorDefault, (const void **) &cfAvailString,
                                                                         1, &kCFTypeArrayCallBacks );
                        CFDictionaryRef cfAvailability  = _ODNodeCopyDetails( cfNode, cfKeys, NULL );
                        
                        if( NULL != cfAvailability )
                        {
                            CFArrayRef  cfValue = CFDictionaryGetValue( cfAvailability, cfAvailString );
                            
                            if( NULL != cfValue )
                            {
                                CFStringRef cfState = CFArrayGetValueAtIndex( cfValue, 0 );
                                
                                if( CFStringCompare(cfState, CFSTR("Available"),0) == kCFCompareEqualTo )
                                {
                                    CFArrayRemoveValueAtIndex( cfArray, ii );
                                }
                            }
							CFRelease( cfAvailability);
							cfAvailability = NULL;
						}
                        CFRelease( cfKeys );
                        cfKeys = NULL;
                    }
                    // otherwise remove the value
                    else
                    {
                        CFArrayRemoveValueAtIndex( cfArray, ii );
                    }
                    
                    CFRelease( cfNode );
                    cfNode = NULL;
                }
            }
        }
        
        CFRelease( cfDict );
        cfDict = NULL;
    }
    
    CFRelease( cfValues );
    cfValues = NULL;
    
    if( NULL != cfArray && 0 == CFArrayGetCount(cfArray) )
    {
        CFRelease( cfArray );
        cfArray = NULL;
    }
    
    return cfArray;
}

CFStringRef ODNodeGetName( ODNodeRef inNodeRef )
{
    _ODNode     *pODNodeRef = (_ODNode *) inNodeRef;
    CFStringRef cfReturn    = NULL;
    
    if( NULL == inNodeRef )
    {
        return NULL;
    }
    
    if( NULL != pODNodeRef )
    {
        _ODNodeLock( pODNodeRef );
        cfReturn = CFDictionaryGetValue( pODNodeRef->_info, kODNodeNameKey );
        _ODNodeUnlock( pODNodeRef );
    }
    
    return cfReturn;
}

CFDictionaryRef ODNodeCopyDetails( ODNodeRef inNodeRef, CFArrayRef inAttributeList, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    return _ODNodeCopyDetails( inNodeRef, inAttributeList, outError );
}

CFDictionaryRef _ODNodeCopyDetails( ODNodeRef inNodeRef, CFArrayRef inAttributeList, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus;
    CFMutableDictionaryRef  cfReturnDict    = NULL;
    
    if( NULL == inNodeRef )
    {
        cfError = (outError != NULL ? CFSTR("Missing node reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    
    _ODNode                 *pODNodeRef     = (_ODNode *) inNodeRef;
    tContextData            dsContext       = 0;
    tDataListPtr            dsInfoType      = NULL;
    tAttributeListRef       dsAttribList    = 0;
    UInt32                  ulCount         = 0;
    
    _ODNodeLock( pODNodeRef );
    
    if( NULL != inAttributeList )
    {
        if( CFGetTypeID(inAttributeList) == CFStringGetTypeID() )
        {
            char    *pTempString = _GetCStringFromCFString( (CFStringRef) inAttributeList );
            
            dsInfoType = dsBuildListFromStrings( 0, pTempString, NULL );
            
            free( pTempString );
            pTempString = NULL;
        }
        else
        {
            dsInfoType = _ConvertCFArrayToDataList( inAttributeList );
        }
    }
    else
    {
        dsInfoType = dsBuildListFromStrings( 0, kDSAttributesAll, NULL );
    }
    
    tDataBufferPtr  dsDataBuffer = dsDataBufferAllocate( 0, 8 * 1024 ); // allocate a 8k buffer
    if( NULL == dsDataBuffer )
    {
        dsStatus = eDSAllocationFailed;
		goto failed;
    }
    
    bool bFailedOnce = false;
    
    do
    {
        dsStatus = dsGetDirNodeInfo( pODNodeRef->_dsNodeRef, dsInfoType, dsDataBuffer, false, &ulCount, &dsAttribList, &dsContext );
        
        if( eDSBufferTooSmall == dsStatus )
        {
            UInt32 newSize = (dsDataBuffer->fBufferSize << 1);
            
            if( newSize < 100 * 1024 * 1024 )
            {
                dsDataBufferDeAllocate( 0, dsDataBuffer );
                dsDataBuffer = dsDataBufferAllocate( 0, newSize );
            }
            else
            {
                dsStatus = eDSAllocationFailed;
                break;
            }
        }
        
        if( (eDSInvalidNodeRef == dsStatus || eDSInvalidReference == dsStatus || eDSInvalidDirRef == dsStatus || eDSCannotAccessSession == dsStatus || eDSInvalidRefType == dsStatus) && 
            bFailedOnce == false )
        {
            if( 0 != dsContext )
            {
                dsReleaseContinueData( pODNodeRef->_ODSession->_dsRef, dsContext );
                dsContext = 0;
            }
            
            dsStatus = dsVerifyDirRefNum( pODNodeRef->_ODSession->_dsRef );
            if( eDSInvalidReference == dsStatus || eDSInvalidDirRef == dsStatus || eDSInvalidRefType == dsStatus)
            {
                dsStatus = _ReopenDS( pODNodeRef->_ODSession );
                if( eDSNoErr == dsStatus )
                {
                    // need to reopen the node ref because if DS ref is invalid
                    dsStatus = _ReopenNode( pODNodeRef );
                }
            }
            else // well if dsRef is valid, then it must be the node that is invalid
            {
                dsStatus = _ReopenNode( pODNodeRef );
            }
            
            if( eDSNoErr == dsStatus )
            {
                // reset error status to buffertoosmall to cause the loop to continue
                dsStatus = eDSBufferTooSmall;
            }
            
            bFailedOnce = true;
        }
        
        if( eDSNoErr == dsStatus && cfReturnDict == NULL)
        {
            cfReturnDict = _CopyAttributesFromBuffer( pODNodeRef->_dsNodeRef, dsDataBuffer, dsAttribList, ulCount, outError );
        }
        
    } while( dsStatus == eDSBufferTooSmall );
failed: 
    if( 0 != dsAttribList )
    {
        dsCloseAttributeList( dsAttribList );
        dsAttribList = 0;
    }
    
    if( NULL != dsDataBuffer )
    {
        dsDataBufferDeAllocate( 0, dsDataBuffer );
        dsDataBuffer = NULL;
    }
    
    if( NULL != dsInfoType )
    {
        dsDataListDeallocate( 0, dsInfoType );
        
        free( dsInfoType );
        dsInfoType = NULL;
    }
    
    _ODNodeUnlock( pODNodeRef );
    
    // _MapDSErrorToReason will return a non-NULL value if there is already an error set
    cfError = _MapDSErrorToReason( outError, dsStatus );
    
finish:
        
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfNodeName = (NULL != inNodeRef ? ODNodeGetName(inNodeRef) : NULL);
            
            if( NULL != cfNodeName )
            {
                CFStringRef cfTemp = CFSTR("Unable to obtain node details for %@.");
                cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfNodeName );
                CFRelease( cfTemp );
            }
            else
            {                    
                cfDescription = CFSTR("Unable to obtain node details.");
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }
    }    
    
    return cfReturnDict;
}

CFArrayRef ODNodeCopySupportedRecordTypes( ODNodeRef inNodeRef, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inNodeRef )
    {
        if( NULL != outError && NULL == (*outError) )
        {
			CFStringRef cfDescription = CFSTR("Unable to determine supported record types.");
			CFStringRef cfError = CFSTR("Missing node reference.");
            _ODErrorSet( outError, kODErrorDomainFramework, eDSNullParameter, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        return NULL;
    }
    
    _ODNode             *pODNode        = (_ODNode *) inNodeRef;
    CFStringRef         cfRecordType    = kODAttributeTypeRecordType;
    CFArrayRef          cfValues        = CFArrayCreate( kCFAllocatorDefault, (const void **) &cfRecordType, 1, &kCFTypeArrayCallBacks );
    CFDictionaryRef     cfNodeInfo      = _ODNodeCopyDetails( inNodeRef, cfValues, outError );
    CFMutableArrayRef   cfResults       = NULL;
    
    _ODNodeLock( pODNode );
    
    if( NULL == cfNodeInfo || CFDictionaryGetCountOfKey(cfNodeInfo, cfRecordType) == 0 )
    {
        // well need to fall to the configuration node instead
        ODNodeRef  cfConfigNode = _ODNodeCreateWithNodeType( kCFAllocatorDefault, (ODSessionRef) (pODNode->_ODSession), 
                                                             kODNodeTypeConfigure, NULL );
        
        if( NULL != cfConfigNode )
        {
            ODQueryRef cfSearch;
            
            cfSearch = _ODQueryCreateWithNode( kCFAllocatorDefault, cfConfigNode, kODRecordTypeRecordTypes, kODAttributeTypeRecordName, 
											   kODMatchEqualTo, CFSTR(kDSRecordsAll), kODAttributeTypeRecordName, 0, NULL );
            
            if( NULL != cfSearch )
            {
                CFArrayRef	tempResults = (CFMutableArrayRef) _ODQueryCopyResults( cfSearch, false, NULL );
				if (tempResults != NULL) {
					CFIndex		count	= CFArrayGetCount(tempResults);
					CFIndex		ii;
					
					cfResults = CFArrayCreateMutable(kCFAllocatorDefault, count, &kCFTypeArrayCallBacks);
					for (ii = 0; ii < count; ii++) {
						_ODRecord *record = (_ODRecord *) CFArrayGetValueAtIndex(tempResults, ii);
						
						CFArrayAppendValue(cfResults, record->_cfRecordName);
					}
					
					CFRelease(tempResults);
				}
				
                CFRelease( cfSearch );
                cfSearch = NULL;
            }
            
            CFRelease( cfConfigNode );
            cfConfigNode = NULL;
        }
    }
    
    CFRelease( cfValues );
    cfValues = NULL;
    
    _ODNodeUnlock( pODNode );
    
    if( NULL != cfNodeInfo)
    {
        if (cfResults == NULL)
		{
			CFArrayRef  cfTemp    = (CFArrayRef) CFDictionaryGetValue( cfNodeInfo, kODAttributeTypeRecordType );
        
			if( NULL != cfTemp )
			{
				cfResults = CFArrayCreateMutableCopy( kCFAllocatorDefault, 0, cfTemp );
			}
		} 
        CFRelease( cfNodeInfo );
        cfNodeInfo = NULL;
    }
    
    if( NULL != cfResults )
    {
        CFArraySortValues( cfResults, CFRangeMake(0,CFArrayGetCount(cfResults)), (CFComparatorFunction) CFStringCompare, NULL );
    }    
    
    return cfResults;
}

CFArrayRef ODNodeCopySupportedAttributes( ODNodeRef inNodeRef, CFStringRef inRecordType, CFErrorRef *outError )
{
    CFMutableArrayRef       cfResults   = NULL;

    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inNodeRef )
    {
        if( NULL != outError && NULL == (*outError) )
        {
			CFStringRef cfDescription = CFSTR("Unable to determine supported attributes.");
			CFStringRef cfError = CFSTR("Missing node reference.");
            _ODErrorSet( outError, kODErrorDomainFramework, eDSNullParameter, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        return NULL;
    }
    
    _ODNode         *pODNode    = (_ODNode *) inNodeRef;
    CFDictionaryRef cfNodeInfo  = NULL; // ODNodeCopyDetails( inNodeRef, CFSTR(kDSStdRecordTypeAttributeTypes) );
    
    _ODNodeLock( pODNode );
    
    if( NULL == cfNodeInfo || CFDictionaryGetCountOfKey(cfNodeInfo, kODAttributeTypeRecordType) == 0 )
    {
        // well need to fall to the configuration node instead
        ODNodeRef  cfConfigNode = _ODNodeCreateWithNodeType( kCFAllocatorDefault, (ODSessionRef) (pODNode->_ODSession), 
                                                             kODNodeTypeConfigure, NULL );
        
        if( NULL != cfConfigNode )
        {
			if (inRecordType != NULL) {
				CFArrayRef attribs = CFArrayCreate(kCFAllocatorDefault, (const void **)&kODAttributeTypeAllTypes, 1, &kCFTypeArrayCallBacks);
				ODRecordRef record = ODNodeCopyRecord(cfConfigNode, kODRecordTypeRecordTypes, inRecordType, attribs, NULL);
				if (record != NULL) {
					CFArrayRef tempResults = ODRecordCopyValues(record, kODAttributeTypeAllTypes, NULL);
					if (tempResults != NULL) {
						cfResults = CFArrayCreateMutableCopy(kCFAllocatorDefault, 0, tempResults);
						CFRelease(tempResults);
					}
				}
				
				CFRelease(attribs);
			}
			else {
				ODQueryRef cfSearch;
				
				cfSearch = _ODQueryCreateWithNode(kCFAllocatorDefault, cfConfigNode, kODRecordTypeAttributeTypes, kODAttributeTypeRecordName,
												  kODMatchEqualTo, NULL, kODAttributeTypeRecordName, 0, NULL);
				
				if (NULL != cfSearch) {
					CFArrayRef	tempResults = _ODQueryCopyResults(cfSearch, false, NULL);
					if (tempResults != NULL) {
						CFIndex		count	= CFArrayGetCount(tempResults);
						CFIndex		ii;
						
						cfResults = CFArrayCreateMutable(kCFAllocatorDefault, count, &kCFTypeArrayCallBacks);
						for (ii = 0; ii < count; ii++) {
							_ODRecord *record = (_ODRecord *) CFArrayGetValueAtIndex(tempResults, ii);
							
							CFArrayAppendValue(cfResults, record->_cfRecordName);
						}
						
						CFRelease(tempResults);
					}
					
					CFRelease( cfSearch );
					cfSearch = NULL;
				}
				
				CFRelease( cfConfigNode );
				cfConfigNode = NULL;
			}
        }
    }
    
    _ODNodeUnlock( pODNode );
    
    if( NULL != cfNodeInfo )
    {
        CFArrayRef  cfValues    = (CFArrayRef) CFDictionaryGetValue( cfNodeInfo, kODAttributeTypeRecordType );
        
        if( NULL != cfValues )
        {
            CFArrayAppendArray( cfResults, cfValues, CFRangeMake(0,CFArrayGetCount(cfValues)) );
        }
        
        CFRelease( cfNodeInfo );
        cfNodeInfo = NULL;
    }
    
    if( NULL != cfResults )
    {
        CFArraySortValues( cfResults, CFRangeMake(0,CFArrayGetCount(cfResults)), (CFComparatorFunction) CFStringCompare, NULL );
    }
    
    return cfResults;
}

bool ODNodeSetCredentials( ODNodeRef inNodeRef, CFStringRef inRecordType, CFStringRef inRecordName, CFStringRef inPassword, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    return _ODNodeSetCredentials( inNodeRef, inRecordType, inRecordName, inPassword, outError );
}

bool _ODNodeSetCredentials( ODNodeRef inNodeRef, CFStringRef inRecordType, CFStringRef inRecordName, CFStringRef inPassword, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus    = eDSAuthFailed;
    
    if( NULL == inNodeRef )
    {
        cfError = (NULL != outError ? CFSTR("Missing node reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inRecordName )
    {
        cfError = (NULL != outError ? CFSTR("Missing record name.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inPassword )
    {
        cfError = (NULL != outError ? CFSTR("Missing password.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }

    CF_OBJC_FUNCDISPATCH( _kODNodeTypeID, bool, inNodeRef, "setCredentialsWithRecordType:recordName:password:error:", inRecordType,
                          inRecordName, inPassword, outError );
    
    _ODNode     *pODNodeRef = (_ODNode *) inNodeRef;
    CFTypeRef   values[]    = { inRecordName, inPassword, NULL };
    CFArrayRef  cfAuthItems = CFArrayCreate( kCFAllocatorDefault, values, 2, &kCFTypeArrayCallBacks );
    char        *pRecType   = _GetCStringFromCFString( inRecordType );

    _ODNodeLock( pODNodeRef );
    
    dsStatus = _Authenticate( inNodeRef, kDSStdAuthNodeNativeClearTextOK, pRecType, cfAuthItems, NULL, NULL, false );
    
    _ODNodeUnlock( pODNodeRef );
    
    if (pRecType != NULL)
    {
        free( pRecType );
        pRecType = NULL;
    }
    
    if( eDSNoErr == dsStatus )
    {
        CFStringRef cfRecordName = CFStringCreateCopy( kCFAllocatorDefault, inRecordName );

        // if we were authing the node, we need to set the credentials permanently
        pODNodeRef->_cfNodePassword = CFStringCreateCopy( kCFAllocatorDefault, inPassword );
        CFDictionarySetValue( pODNodeRef->_info, kODNodeUsername, cfRecordName );
        
        CFRelease( cfRecordName );
        cfRecordName = NULL;
    }
    
    if( NULL != cfAuthItems )
    {
        CFRelease( cfAuthItems );
        cfAuthItems = NULL;
    }
    
    // _MapDSErrorToReason will return a non-NULL value if there is already an error set
    cfError = _MapDSErrorToReason( outError, dsStatus );
    
finish:
        
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfNodeName = (NULL != inNodeRef ? ODNodeGetName(inNodeRef) : NULL);
            
            if( NULL != cfNodeName )
            {
                if( NULL != inRecordName )
                {
                    CFStringRef cfTemp = CFSTR("Unable to set node credentials for %@ with the record name %@.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfNodeName, inRecordName );
                    CFRelease( cfTemp );
                }
                else
                {
                    CFStringRef cfTemp = CFSTR("Unable to set node credentials for %@.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfNodeName );
                    CFRelease( cfTemp );
                }
            }
            else
            {
                if( NULL != inRecordName )
                {
                    CFStringRef cfTemp = CFSTR("Unable to set node credentials with the record name %@.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inRecordName );
                    CFRelease( cfTemp );
                }
                else
                {
                    cfDescription = CFSTR("Unable to set node credentials.");
                }
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }

        return false;
    }    
    
    return true;
}

bool ODNodeSetCredentialsExtended( ODNodeRef inNodeRef, CFStringRef inRecordType, CFStringRef inAuthType, 
                                      CFArrayRef inAuthItems, CFArrayRef *outAuthItems, ODContextRef *outContext,
                                      CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    return _ODNodeSetCredentialsExtended( inNodeRef, inRecordType, inAuthType, inAuthItems, outAuthItems, outContext, outError, false );
}

bool ODNodeVerifyCredentialsExtended(ODNodeRef inNodeRef, ODRecordType inRecordType, ODAuthenticationType inAuthType, 
									 CFArrayRef inAuthItems, CFArrayRef *outAuthItems, ODContextRef *outContext,
									 CFErrorRef *outError)
{
    if (NULL != outError) {
        (*outError) = NULL;
    }
	
    return _ODNodeSetCredentialsExtended(inNodeRef, inRecordType, inAuthType, inAuthItems, outAuthItems, outContext, outError, true);
}


bool _ODNodeSetCredentialsExtended( ODNodeRef inNodeRef, CFStringRef inRecordType, CFStringRef inAuthType, 
                                       CFArrayRef inAuthItems, CFArrayRef *outAuthItems, ODContextRef *outContext,
                                       CFErrorRef *outError, bool authonly )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus    = eDSAuthFailed;
    
    if( NULL == inNodeRef )
    {
        cfError = (NULL != outError ? CFSTR("Missing node reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inRecordType )
    {
        cfError = (NULL != outError ? CFSTR("Missing record type.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inAuthType )
    {
        cfError = (NULL != outError ? CFSTR("Missing authentication type.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inAuthItems )
    {
        cfError = (NULL != outError ? CFSTR("The user name, password, or both are missing.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    
    CF_OBJC_FUNCDISPATCH( _kODNodeTypeID, bool, inNodeRef, "setCredentialsWithRecordType:authenticationType:authenticationItems:continueItems:context:error:",
                          inRecordType, inAuthType, inAuthItems, outAuthItems, outContext, outError );

    _ODNode     *pODNodeRef = (_ODNode *) inNodeRef;
    char        *pAuthType  = _GetCStringFromCFString( inAuthType );
    char        *pRecType   = _GetCStringFromCFString( inRecordType );
    
    if( NULL != pAuthType )
    {
        _ODNodeLock( pODNodeRef );
        
        dsStatus = _Authenticate( inNodeRef, pAuthType, pRecType, inAuthItems, outAuthItems, outContext, authonly );
        
        _ODNodeUnlock( pODNodeRef );        
        
        free( pAuthType );
        pAuthType = NULL;
        
        if (pRecType != NULL)
        {
            free( pRecType );
            pRecType = NULL;
        }
    }
    else
    {
        cfError = (NULL != outError ? CFSTR("Authentication type was an empty value.") : CFSTR(""));
        goto finish;
    }
    
    // _MapDSErrorToReason will return a non-NULL value if there is already an error set
    cfError = _MapDSErrorToReason( outError, dsStatus );
    
finish:
        
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfNodeName = (NULL != inNodeRef ? ODNodeGetName(inNodeRef) : NULL);
            
            if( NULL != cfNodeName )
            {
                CFStringRef cfTemp = (NULL != outError ? CFSTR("Unable to set extended node credentials for %@.") : CFSTR(""));
                cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfNodeName );
                CFRelease( cfTemp );
            }
            else
            {                    
                cfDescription = (NULL != outError ? CFSTR("Unable to set extended node credentials.") : CFSTR(""));
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }

        return false;
    }    
    
    return true;
}

bool _SetCacheForKerberosContext( krb5_context kctx, char *inRealm, KLPrincipal *klUserPrinc, krb5_ccache *kcache )
{
    cc_context_t    ccContext   = NULL;
    bool         bFound      = false;
    
    if( cc_initialize(&ccContext, ccapi_version_5, NULL, NULL) == ccNoError )
    {
        cc_ccache_iterator_t    iterator;
        
        if( cc_context_new_ccache_iterator(ccContext, &iterator) == ccNoError )
        {
            cc_ccache_t ccache;
            cc_string_t ccPrincipal = NULL;
            
            while( bFound == false && cc_ccache_iterator_next( iterator, &ccache ) == ccNoError )
            {
                if( cc_ccache_get_principal(ccache, cc_credentials_v5, &ccPrincipal) == ccNoError )
                {
                    if( KLCreatePrincipalFromString(ccPrincipal->data, kerberosVersion_V5, klUserPrinc) == klNoErr )
                    {
                        char *ccname = NULL;
                        char *ccinstance = NULL;
                        char *ccrealm = NULL;
                        
                        if( KLGetTripletFromPrincipal( (*klUserPrinc), &ccname, &ccinstance, &ccrealm) == klNoErr )
                        {
                            if( strcmp(ccrealm, inRealm) == 0 )
                            {
                                cc_string_t pCacheName = NULL;
                                
                                // found a matching realm...
                                if( cc_ccache_get_name(ccache, &pCacheName) == ccNoError )
                                {
                                    if( krb5_cc_resolve(kctx, pCacheName->data, kcache) == 0 )
                                    {
                                        bFound = true;
                                    }
                                    
                                    cc_string_release( pCacheName );
                                }
                            }
                            
                            KLDisposeString( ccname );
                            KLDisposeString( ccinstance );
                            KLDisposeString( ccrealm );
                        }
                        
                        // only dispose if we didn't find the one we wanted
                        if( bFound == false )
                        {
                            KLDisposePrincipal( (*klUserPrinc) );
                            (*klUserPrinc) = NULL;
                        }
                    }
                    cc_string_release( ccPrincipal );
                    ccPrincipal = NULL;
                }
            }
            cc_ccache_iterator_release( iterator );
        }
        
        cc_context_release( ccContext );
        ccContext = NULL;
    }
    
    return bFound;
}

bool ODNodeSetCredentialsUsingKerberosCache( ODNodeRef inNodeRef, CFStringRef inCacheName, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus    = eDSAuthFailed;
    CFMutableArrayRef   cfAuthItems     = NULL;
	KLPrincipal         klUserPrinc     = NULL;
    KLPrincipal         klServPrinc     = NULL;
    char                *servName       = NULL;
    krb5_context        kctx            = NULL;
    CFArrayRef          cfOutItems      = NULL;
    ODContextRef        cfContext       = NULL;
    char                *userPrincipal  = NULL;
   
    if( NULL != outError )
    {
        (*outError) = NULL;
    }

    if( NULL == inNodeRef )
    {
        cfError = (NULL != outError ? CFSTR("Missing node reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    
    CF_OBJC_FUNCDISPATCH( _kODNodeTypeID, bool, inNodeRef, "setCredentialsUsingKerberosCache:error:", inCacheName, outError );

	bool				bOkToPrompt     = false;

    if( NULL != inCacheName && CFStringCompare(inCacheName, CFSTR("OK_TO_PROMPT"), 0) == kCFCompareEqualTo )
    {
        bOkToPrompt = true;
        inCacheName = NULL;
    }
    
    cfAuthItems = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
    if( NULL == cfAuthItems )
    {
        cfError = (NULL != outError ? CFSTR("Memory allocation error.") : CFSTR(""));
        dsStatus = eMemoryError;
        goto finish;
    }
    
	_ODNode             *pODNodeRef     = (_ODNode *) inNodeRef;
    krb5_ccache         kcache          = NULL;
    krb5_error_code     krbErr          = 0;
    krb5_auth_context   authContext     = NULL;
    krb5_data           *kdataPtr       = NULL;
    krb5_creds          kerb_cred       = { 0 };
    krb5_address        **addresses     = NULL;
    krb5_creds          *kerb_cred_out  = NULL;
    krb5_principal      kprin           = { 0 };
    krb5_rcache         rcache          = NULL;

	_ODNodeLock( pODNodeRef );
    
    dsStatus = _Authenticate( inNodeRef, kDSStdAuthKerberosTickets, NULL, cfAuthItems, &cfOutItems, &cfContext, false );
    if( eDSNoErr == dsStatus )
    {
        if ( NULL != cfOutItems && CFArrayGetCount(cfOutItems) > 0 )
        {
            CFTypeRef serviceToGet = (CFTypeRef) CFArrayGetValueAtIndex( cfOutItems, 0 );

            if ( CFGetTypeID(serviceToGet) == CFDataGetTypeID() )
            {
                CFIndex length  = CFDataGetLength( serviceToGet );
                
                servName = (char *) calloc( length + 1, sizeof(char) );
                
                CFDataGetBytes( serviceToGet, CFRangeMake(0, length), (UInt8 *) servName );
               
                krbErr = krb5_init_context( &kctx );
                if( 0 != krbErr )
                {
                    cfError = (NULL != outError ? CFSTR("Unable to initialize Kerberos."): CFSTR(""));
                    dsStatus = eDSAuthMethodNotSupported;
                    goto failed;
                }
                
                if( inCacheName == NULL )
                {
                    bool         bFound      = false;
                    char            *name       = NULL;
                    char            *instance   = NULL;
                    char            *realm      = NULL;
                    krb5_principal  servPrinc   = NULL;
                    KLStatus        klErr;
                    
                    // if is not qualified, lets see if we can figure out a realm
                    if( strchr(servName, '@') == NULL )
                    {
                        char *slash = strchr( servName, '/' );
                        if( NULL == slash )
                        {
                            cfError = (NULL != outError ? CFSTR("Invalid service principal from server."): CFSTR(""));
                            dsStatus = eDSAuthMethodNotSupported;
                            goto failed;
                        }
                        
                        (*slash) = '\0';
                        
                        krbErr = krb5_sname_to_principal( kctx, slash+1, servName, KRB5_NT_SRV_HST, &servPrinc );
                        if( 0 != krbErr )
                        {
                            cfError = (NULL != outError ? CFSTR("Unable to parse service principal."): CFSTR(""));
                            dsStatus = eDSAuthMethodNotSupported;
                            goto failed;
                        }
                        
                        free( servName );
                        
                        krb5_unparse_name( kctx, servPrinc, &servName );
                    }
                    
                    klErr = KLCreatePrincipalFromString( servName, kerberosVersion_V5, &klServPrinc );
                    if( klErr != klNoErr )
                    {
                        cfError = (NULL != outError ? CFSTR("Unable to parse the information from the server."): CFSTR(""));
                        dsStatus = eDSAuthMethodNotSupported;
                        goto failed;
                    }

                    // if we have a service ticket name, let's see if we can find an appropriate cache
                    if( KLGetTripletFromPrincipal(klServPrinc, &name, &instance, &realm) == klNoErr )
                    {
                        
                        bFound = _SetCacheForKerberosContext( kctx, realm, &klUserPrinc, &kcache );
                        if ( bFound == false )
                            klErr = klPrincipalDoesNotExistErr;
                        
                        if ( true == bOkToPrompt )
                        {
                            char *oldName = NULL;

                            KLGetKerberosDefaultRealmByName( &oldName );
                            
                            // the user's pref may not have this realm we'll need to add it if we get this error
                            if ( KLSetKerberosDefaultRealmByName(realm) == klRealmDoesNotExistErr )
                            {
                                KLInsertKerberosRealm( 0, realm );
                                KLSetKerberosDefaultRealmByName( realm );
                            }
                            
                            if ( bFound == false )
                            {
                                char *cachename = NULL;
                                
                                klErr = KLAcquireNewInitialTickets( NULL, NULL, NULL, &cachename );
                                
                                if ( NULL != cachename )
                                {
                                    krb5_cc_resolve( kctx, cachename, &kcache );
                                    KLDisposeString( cachename );
                                }
                            }
                            else 
                            {
                                klErr = KLAcquireInitialTickets( klUserPrinc, NULL, NULL, NULL );
                            }
                            
                            if ( NULL != oldName )
                            {
                                // now set the user's default realm back
                                KLSetKerberosDefaultRealmByName( oldName );
                                
                                KLDisposeString( oldName );
                                oldName = NULL;
                            }
                        }
                        
                        KLDisposeString( name );
                        KLDisposeString( instance );
                        KLDisposeString( realm );
                        
                        if ( klNoErr != klErr )
                        {
                            cfError = (NULL != outError ? CFSTR("Unable to obtain Kerberos tickets."): CFSTR(""));
                            dsStatus = eDSAuthFailed;
                            goto failed;
                        }
                    }
                }
                else
                {
                    char *pCacheName = _GetCStringFromCFString( inCacheName );
                    
                    krbErr = krb5_cc_resolve( kctx, pCacheName, &kcache );
                    
                    free( pCacheName );
                    pCacheName = NULL;
                }
                
                if( 0 != krbErr )
                {
                    cfError = (NULL != outError ? CFSTR("No Kerberos cache available."): CFSTR(""));
                    dsStatus = eDSAuthFailed;
                    goto failed;
                }
                
                krbErr = krb5_cc_get_principal( kctx, kcache, &kerb_cred.client );
                if( 0 != krbErr )
                {
                    cfError = (NULL != outError ? CFSTR("No principal is assigned to cache."): CFSTR(""));
                    dsStatus = eDSAuthFailed;
                    goto failed;
                }
                
                krbErr = krb5_unparse_name( kctx, kerb_cred.client, &userPrincipal );
                if( 0 != krbErr )
                {
                    cfError = (NULL != outError ? CFSTR("Unable to determine user's principal name."): CFSTR(""));
                    dsStatus = eDSAuthFailed;
                    goto failed;
                }
                
                // get a service ticket with the TGT
                krbErr = krb5_auth_con_init( kctx, &authContext );
                if( 0 != krbErr )
                {
                    cfError = (NULL != outError ? CFSTR("Unable to initialize authentication context."): CFSTR(""));
                    dsStatus = eDSAuthFailed;
                    goto failed;
                }
                
                if( krb5_os_localaddr(kctx, &addresses) == 0 )
                {
                    krbErr = krb5_auth_con_setaddrs( kctx, authContext, *addresses, *addresses );
                    if( 0 != krbErr )
                    {
                        cfError = (NULL != outError ? CFSTR("Failed to set connection addresses."): CFSTR(""));
                        dsStatus = eDSAuthFailed;
                        goto failed;
                    }
                }
                
                krbErr = krb5_get_server_rcache( kctx, krb5_princ_component(kctx, kerb_cred.client, 0), &rcache );
                if( 0 != krbErr )
                {
                    cfError = (NULL != outError ? CFSTR("Failed to obtain server replay cache information."): CFSTR(""));
                    dsStatus = eDSAuthFailed;
                    goto failed;
                }
                
                krbErr = krb5_auth_con_setrcache( kctx, authContext, rcache );
                if( 0 != krbErr )
                {
                    cfError = (NULL != outError ? CFSTR("Unable to set replay cache."): CFSTR(""));
                    dsStatus = eDSAuthFailed;
                    goto failed;
                }
                
                krbErr = krb5_parse_name( kctx, servName, &kprin );
                if( 0 != krbErr )
                {
                    cfError = (NULL != outError ? CFSTR("Unable to parse the name of the service."): CFSTR(""));
                    dsStatus = eDSAuthFailed;
                    goto failed;
                }
                
                krbErr = krb5_copy_principal( kctx, kprin, &kerb_cred.server );
                if( 0 != krbErr )
                {
                    cfError = (NULL != outError ? CFSTR("Unable to set the principal for the server."): CFSTR(""));
                    dsStatus = eDSAuthFailed;
                    goto failed;
                }
                
                krbErr = krb5_get_credentials( kctx, 0, kcache, &kerb_cred, &kerb_cred_out );
                if( 0 != krbErr )
                {
                    cfError = (NULL != outError ? CFSTR("No Kerberos cache available."): CFSTR(""));
                    dsStatus = eDSAuthFailed;
                    goto failed;
                }
                
                krb5_free_cred_contents( kctx, &kerb_cred );
                if( 0 != krbErr )
                {
                    cfError = (NULL != outError ? CFSTR("No Kerberos cache available."): CFSTR(""));
                    dsStatus = eDSAuthFailed;
                    goto failed;
                }
                
                // Service Ticket to blob
                krbErr = krb5_mk_1cred( kctx, authContext, kerb_cred_out, &kdataPtr, NULL );
                if( 0 != krbErr )
                {
                    cfError = (NULL != outError ? CFSTR("No Kerberos cache available."): CFSTR(""));
                    dsStatus = eDSAuthFailed;
                    goto failed;
                }
                
                CFStringRef cfUserPrincipal = CFStringCreateWithCString( kCFAllocatorDefault, userPrincipal, kCFStringEncodingUTF8 );
                if( NULL != cfUserPrincipal )
                {
                    CFArrayAppendValue( cfAuthItems, cfUserPrincipal );
                    CFRelease( cfUserPrincipal );
                }
                
                CFDataRef credData = CFDataCreate( kCFAllocatorDefault, (const UInt8 *)kdataPtr->data, kdataPtr->length );
                if ( NULL != credData )
                {
                    CFArrayAppendValue( cfAuthItems, credData );
                    CFRelease( credData );
                }
                
                CFRelease( cfOutItems );
                cfOutItems = NULL;
                
                dsStatus = _Authenticate( inNodeRef, kDSStdAuthKerberosTickets, NULL, cfAuthItems, &cfOutItems, &cfContext, false );
            }
            else
            {
                cfError = (NULL != outError ? CFSTR("Unable to determine service ticket."): CFSTR(""));
                dsStatus = eDSAuthMethodNotSupported;
            }
        }
    }
    
failed:
    
    _ODNodeUnlock( pODNodeRef );
    
    // _MapDSErrorToReason will return a non-NULL value if there is already an error set
    if (cfError == NULL) {
		cfError = _MapDSErrorToReason( outError, dsStatus );
	}
    
finish:
    
    if( NULL != klUserPrinc )
    {
        KLDisposePrincipal( klUserPrinc );
        klUserPrinc = NULL;
    }
    
    if( NULL != klServPrinc )
    {
        KLDisposePrincipal( klServPrinc );
        klServPrinc = NULL;
    }
    
    if( NULL != servName )
    {
        free( servName );
        servName = NULL;
    }
    
    if( NULL != kctx )
    {
        if( NULL != kdataPtr )
        {
            krb5_free_data( kctx, kdataPtr );
            kdataPtr = NULL;
        }
        
        if( NULL != kerb_cred_out )
        {
            krb5_free_creds( kctx, kerb_cred_out );
            kerb_cred_out = NULL;
        }
        
        if( NULL != kcache )
        {
            krb5_cc_close( kctx, kcache );
        }
        
        if( NULL != addresses )
        {
            krb5_free_addresses( kctx, addresses );
            addresses = NULL;
        }
        
        if( NULL != authContext )
        {
            krb5_auth_con_free( kctx, authContext );
            authContext = NULL;
        }
        
        krb5_free_principal( kctx, kprin );
        
        krb5_free_context( kctx );
        kctx = NULL;
    }
    
    if( NULL != cfContext )
    {
        CFRelease( cfContext );
        cfContext = NULL;
    }
    
    if( NULL != cfAuthItems )
    {
        CFRelease( cfAuthItems );
        cfAuthItems = NULL;
    }
    
    if( NULL != cfOutItems )
    {
        CFRelease( cfOutItems );
        cfOutItems = NULL;
    }
    
    if( NULL != userPrincipal )
    {
        free( userPrincipal );
        userPrincipal = NULL;
    }
    
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfNodeName = (NULL != inNodeRef ? ODNodeGetName(inNodeRef) : NULL);
            
            if( NULL != cfNodeName )
            {
                CFStringRef cfTemp = CFSTR("Unable to set node credentials for %@.");
                cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfNodeName );
                CFRelease( cfTemp );
            }
            else
            {
                cfDescription = CFSTR("Unable to set node credentials.");
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus,  &cfDescription,  &cfError, NULL );
        }
        else
        {
            CFRelease( cfError );
        }
        
        return false;
    }    
    
    return true;
}

void _ValidateRecordAttributes( const void *key, const void *value, void *context )
{
    int *attrbad = (int *)context;
    if (CFGetTypeID(value) != CFArrayGetTypeID()) {
        *attrbad = 1;
    }
}

void _AppendUniqueRecordNames( const void *name, void *newNames )
{
    CFIndex i, count = CFArrayGetCount(newNames);
    for (i = 0; i < count; i++) {
        if (CFStringCompare(name, CFArrayGetValueAtIndex(newNames, i), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
            return;
    }
    CFArrayAppendValue(newNames, name);
}

ODRecordRef ODNodeCreateRecord( ODNodeRef inNodeRef, CFStringRef inRecordType, CFStringRef inRecordName, CFDictionaryRef inAttributes,
                                CFErrorRef *outError )
{
    _ODNode     *pODNodeRef     = (_ODNode *) inNodeRef;
    _ODRecord   *pODRecord      = NULL;
    CFStringRef cfError         = NULL;
    tDirStatus  dsStatus;

    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inNodeRef )
    {
        cfError = (NULL != outError ? CFSTR("Missing node reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inRecordType )
    {
        cfError = (NULL != outError ? CFSTR("Missing record type.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inRecordName )
    {
        cfError = (NULL != outError ? CFSTR("Missing record name.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    
    /* inAttributes may be null, but it not we need to validate its contents */
    if( NULL != inAttributes )
    {
        int attrbad = 0;
        CFDictionaryApplyFunction(inAttributes, _ValidateRecordAttributes, &attrbad);
        if (attrbad) {
            cfError = (NULL != outError ? CFSTR("Attribute dictionary values must all be arrays.") : CFSTR(""));
            dsStatus = eDSInvalidAttributeType;
            goto finish;
        }
    }

    if( kODNodeTypeAuthentication == pODNodeRef->_nodeType || kODNodeTypeContacts == pODNodeRef->_nodeType ||
        kODNodeTypeNetwork == pODNodeRef->_nodeType )
    {
        dsStatus = eNotHandledByThisNode;
        cfError = _MapDSErrorToReason( outError, dsStatus );
        goto finish;
    }
    
    _ODNodeLock( pODNodeRef );
    
    bool                    bRecordBlat     = (0 != (pODNodeRef->_flags & kODNodeFlagRecordBlat));
    tRecordReference        dsRecordRef     = 0;
    CFMutableDictionaryRef  cfAttributes    = NULL;
    CFArrayRef              cfPassword      = NULL;
    
    if ( NULL != inAttributes )
    {
        cfAttributes = CFDictionaryCreateMutableCopy( kCFAllocatorDefault, 0, inAttributes );
        
        // remove stuff we don't set in the attribute list
        CFDictionaryRemoveValue( cfAttributes, kODAttributeTypeRecordType );
        CFDictionaryRemoveValue( cfAttributes, kODAttributeTypeMetaNodeLocation );
        
        // lets set the password
        cfPassword = CFDictionaryGetValue( cfAttributes, kODAttributeTypePassword );
        if ( NULL != cfPassword )
        {
            if ( CFGetTypeID(cfPassword) != CFArrayGetTypeID() ) {
                cfPassword = NULL;
            }
            else {
                // need to retain it before we remove it
                CFRetain( cfPassword );
            }
            
            CFDictionaryRemoveValue( cfAttributes, kODAttributeTypePassword );
        }
    }

    if ( true == bRecordBlat && NULL != cfAttributes )
    {
        CFArrayRef      names       = (CFArrayRef) CFDictionaryGetValue( cfAttributes, kODAttributeTypeRecordName );
        CFAllocatorRef  cfAllocator = CFGetAllocator( cfAttributes );
        
        if ( NULL == names || 0 == CFArrayGetCount(names) )
        {
            // if we don't have a name list there is an empty list
            CFMutableArrayRef newNames = CFArrayCreateMutable( cfAllocator, 1, &kCFTypeArrayCallBacks );
            CFArrayAppendValue( newNames, inRecordName );
            CFDictionarySetValue( cfAttributes, kODAttributeTypeRecordName, newNames );
            CFRelease( newNames );
        }
        else
        {
            // otherwise, do some reduction on the list, removing duplicates because LDAP backend doesn't allow dupes
            CFIndex             count       = CFArrayGetCount( names );
            CFMutableArrayRef   newNames    = CFArrayCreateMutable( cfAllocator, count, &kCFTypeArrayCallBacks );;
            
            // add the supplied record name first
            CFArrayAppendValue( newNames, inRecordName );
            CFArrayApplyFunction( names, CFRangeMake(0, count), _AppendUniqueRecordNames, newNames );
            
            CFDictionarySetValue( cfAttributes, kODAttributeTypeRecordName, newNames );
            CFRelease( newNames );
        }
        
        CFTypeRef keys[2] = { CFSTR("Record Type"), CFSTR("Attributes and Values") };
        CFTypeRef values[2] = { inRecordType, cfAttributes };
        CFDictionaryRef recordBlatDict = CFDictionaryCreate( kCFAllocatorDefault, keys, values, 2, &kCFTypeDictionaryKeyCallBacks,
                                                             &kCFTypeDictionaryValueCallBacks );
        CFDataRef bufferData = CFPropertyListCreateData( kCFAllocatorDefault, recordBlatDict, kCFPropertyListXMLFormat_v1_0, 0, NULL );
        CFRelease( recordBlatDict );
        recordBlatDict = NULL;
        
        CFErrorRef localError = NULL;
        CFDataRef receiveData = ODNodeCustomCall( inNodeRef, eDSCustomCallCreateRecordWithAttributes, bufferData, &localError );
        if ( NULL != receiveData ) {
            CFRelease( receiveData );
            receiveData = NULL;
        }
        
        CFRelease( bufferData );
        bufferData = NULL;
        
        if ( localError == NULL ) {
            dsStatus = eDSNoErr;
        }
        else {
            dsStatus = CFErrorGetCode( localError );
            if ( outError != NULL ) {
                *outError = localError;
            }
        }
    }
    else
    {
        tDataNodePtr        dsRecordType    = _GetDataBufferFromCFType( inRecordType );
        tDataNodePtr        dsRecordName    = _GetDataBufferFromCFType( inRecordName );
        
        do
        {
            dsStatus = dsCreateRecordAndOpen( pODNodeRef->_dsNodeRef, dsRecordType, dsRecordName, &dsRecordRef );
            if( eDSInvalidReference == dsStatus || eDSInvalidDirRef == dsStatus || eDSInvalidNodeRef == dsStatus || eDSInvalidRefType == dsStatus )
            {
                dsStatus = _ReopenNode( pODNodeRef );
            }
        } while( eDSInvalidReference == dsStatus || eDSInvalidDirRef == dsStatus || eDSInvalidNodeRef == dsStatus || eDSInvalidRefType == dsStatus );
        
        if( NULL != dsRecordType )
        {
            dsDataBufferDeAllocate( 0, dsRecordType );
            dsRecordType = NULL;
        }
        
        if( NULL != dsRecordName )
        {
            dsDataBufferDeAllocate( 0, dsRecordName );
            dsRecordName = NULL;
        }
    }
    
    // if no error's we create the actual object
    if ( eDSNoErr == dsStatus )
    {
        CFAllocatorRef  cfAllocator = CFGetAllocator( inNodeRef );
        
        pODRecord = _createRecord( cfAllocator );
        
        // if we allocated a node because this was a search node, then we don't need to retain it
        pODRecord->_ODNode = (_ODNode *) CFRetain( (CFTypeRef) pODNodeRef );
        pODRecord->_dsRecordRef = dsRecordRef;
        pODRecord->_cfRecordType = CFStringCreateCopy( cfAllocator, inRecordType );
        pODRecord->_cfRecordName = CFStringCreateCopy( cfAllocator, inRecordName );
        
        if ( true == bRecordBlat && NULL != cfAttributes ) {
            CFRetain( cfAttributes );
            pODRecord->_cfAttributes = cfAttributes;
        }
        else {
            pODRecord->_cfAttributes = CFDictionaryCreateMutable( cfAllocator, 0, &kCFTypeDictionaryKeyCallBacks, 
                                                                  &kCFTypeDictionaryValueCallBacks );
        }
        
        CFStringRef cfNodeName  = CFDictionaryGetValue( pODNodeRef->_info, kODNodeNameKey );
        if( NULL != cfNodeName )
        {
            CFMutableArrayRef   cfNodeLoc = CFArrayCreateMutable( cfAllocator, 0, &kCFTypeArrayCallBacks );
			CFMutableArrayRef	recType = CFArrayCreateMutable(cfAllocator, 0, &kCFTypeArrayCallBacks);
            
            CFArrayAppendValue( cfNodeLoc, cfNodeName );
            CFDictionarySetValue( pODRecord->_cfAttributes, kODAttributeTypeMetaNodeLocation, cfNodeLoc );
			
			CFArrayAppendValue(recType, pODRecord->_cfRecordType);
			CFDictionarySetValue(pODRecord->_cfAttributes, kODAttributeTypeRecordType, recType);
			CFRelease(recType);

			if (CFDictionaryContainsKey(pODRecord->_cfAttributes, kODAttributeTypeRecordName) == FALSE) {
				CFMutableArrayRef	recName = CFArrayCreateMutable(cfAllocator, 0, &kCFTypeArrayCallBacks);
				
				CFArrayAppendValue(recName, pODRecord->_cfRecordName);
				CFDictionarySetValue(pODRecord->_cfAttributes, kODAttributeTypeRecordName, recName);
				CFRelease(recName);
			}
			
            CFRelease( cfNodeLoc );
            cfNodeLoc = NULL;
        }

        // we attempt to add a GUID if possible, but ignore errors if it fails
        bool addGUID = true;
        if ( cfAttributes != NULL ) {
            CFArrayRef attribUUID = (CFArrayRef) CFDictionaryGetValue( cfAttributes, kODAttributeTypeGUID );
            if ( NULL == attribUUID || 0 == CFArrayGetCount(attribUUID) ) {
                addGUID = true;
            }
            else {
                addGUID = false; 
            }
        }
        
        if ( addGUID )
        {
            CFUUIDRef   cfUUID  = CFUUIDCreate( kCFAllocatorDefault );
            if( NULL != cfUUID )
            {
                CFStringRef cfUUIDstring    = CFUUIDCreateString( kCFAllocatorDefault, cfUUID );
                if( NULL != cfUUIDstring ) {
                    _ODRecordSetValues( (ODRecordRef) pODRecord, kODAttributeTypeGUID, cfUUIDstring, NULL );
                    
                    CFRelease( cfUUIDstring );
                    cfUUIDstring = NULL;
                }
                
                CFRelease( cfUUID );
                cfUUID = NULL;
            }
        }
        
        if ( false == bRecordBlat && cfAttributes != NULL ) {
            CFArrayRef  cfRecordNames   = (CFArrayRef) CFDictionaryGetValue( cfAttributes, kODAttributeTypeRecordName );
            
            // if we successfully created the record, let's set additional names
            if( NULL != cfRecordNames && CFGetTypeID(cfRecordNames) == CFArrayGetTypeID() )
            {
                CFIndex         iCount          = CFArrayGetCount( cfRecordNames );
                tDataNodePtr    dsAttrType      = dsDataNodeAllocateString( 0, kDSNAttrRecordName );
                CFStringRef     cfRecordName    = NULL;
                tDataNodePtr    dsRecordName;
                CFIndex         ii;
                
                for( ii = 0; ii < iCount && eDSNoErr == dsStatus; ii++ )
                {
                    cfRecordName = CFArrayGetValueAtIndex( cfRecordNames, ii );
                    
                    // if we already created the record with the name, just continue
                    if( CFStringCompare( cfRecordName, inRecordName, 0) == kCFCompareEqualTo )
                        continue;
                    
                    dsRecordName = _GetDataBufferFromCFType( cfRecordName );
                    
                    do
                    {
                        dsStatus = dsAddAttributeValue( pODRecord->_dsRecordRef, dsAttrType, dsRecordName );
                        
                        if( eDSInvalidReference == dsStatus || eDSInvalidRecordRef == dsStatus || eDSInvalidDirRef == dsStatus || 
                           eDSInvalidNodeRef == dsStatus || eDSCannotAccessSession == dsStatus || eDSInvalidRefType == dsStatus )
                        {
                            dsStatus = _ReopenRecord( pODRecord );
                        }
                        
                    } while( eDSInvalidReference == dsStatus || eDSInvalidRecordRef == dsStatus || eDSInvalidDirRef == dsStatus || 
                            eDSInvalidNodeRef == dsStatus || eDSInvalidRefType == dsStatus );
                    
                    dsDataBufferDeAllocate( 0, dsRecordName );
                    dsRecordName = NULL;
                }
                
                dsDataBufferDeAllocate( 0, dsAttrType );
                dsAttrType = NULL;
            }
            
            // remove values we won't set in Attributes
            CFDictionaryRemoveValue( cfAttributes, kODAttributeTypeRecordName );
            
            if ( eDSNoErr == dsStatus )
            {
                CFIndex     iCount  = CFDictionaryGetCount( cfAttributes );
                CFStringRef *keys   = (CFStringRef *) calloc( sizeof(CFStringRef), iCount );
                CFTypeRef   *values = (CFTypeRef *) calloc( sizeof(CFTypeRef), iCount );
                CFIndex     ii;
                
                CFDictionaryGetKeysAndValues( cfAttributes, (const void **) keys, (const void **) values );
                
                // attempt to set all values
                for( ii = 0; ii < iCount; ii++ )
                {
                    // we don't do empty arrays during creation
                    if( CFGetTypeID(values[ii]) == CFArrayGetTypeID() && CFArrayGetCount(values[ii]) == 0 )
                        continue;
                    
                    // we don't do empty strings during creation
                    if( CFGetTypeID(values[ii]) == CFStringGetTypeID() && CFStringGetLength(values[ii]) == 0 )
                        continue;
                    
                    // we don't do empty data during creation
                    if( CFGetTypeID(values[ii]) == CFDataGetTypeID() && CFDataGetLength(values[ii]) == 0 )
                        continue;
                    
                    // if we fail, delete the record and release the object
                    if( _ODRecordSetValues( (ODRecordRef) pODRecord, keys[ii], values[ii], outError ) == false )
                    {
                        _ODRecordDelete( (ODRecordRef) pODRecord, NULL );
                        CFRelease( (ODRecordRef) pODRecord );
                        pODRecord = NULL;
                        break;
                    }
                }
                
                free( keys );
                keys = NULL;
                
                free( values );
                values = NULL;
            }
            else
            {
                _ODRecordDelete( (ODRecordRef) pODRecord, NULL );
                CFRelease( (ODRecordRef) pODRecord );
                pODRecord = NULL;
            }
        }

        if ( NULL != cfPassword )
        {
            // first lets try setting the password attribute value if we have a value, if that fails, let's change it, assuming it's not the special marker
            if ( 0 != CFArrayGetCount(cfPassword) && _ODRecordSetValues((ODRecordRef) pODRecord, kODAttributeTypePassword, cfPassword, outError) == false )
            {
                CFStringRef cfTempPassword = CFArrayGetValueAtIndex( cfPassword, 0 );
                
                // if someone isn't passing the Marker "********", try to change the password
                if ( CFStringCompare(CFSTR(kDSValueNonCryptPasswordMarker), cfTempPassword, 0) != kCFCompareEqualTo &&
                    _ODRecordChangePassword((ODRecordRef) pODRecord, NULL, cfTempPassword, outError) == false )
                {
                    _ODRecordDelete( (ODRecordRef) pODRecord, NULL );
                    CFRelease( (ODRecordRef) pODRecord );
                    pODRecord = NULL;
                }
            }
            
        }
        
        // no need to lock, no one has the record pointer but this routine, flush all the changes
        if ( NULL != pODRecord && 0 != pODRecord->_dsRecordRef ) {
            dsFlushRecord( pODRecord->_dsRecordRef );
        }
	}
	if ( cfAttributes ) {
		CFRelease( cfAttributes );
		cfAttributes = NULL;
	}
	if ( cfPassword ) {
		CFRelease( cfPassword );
		cfPassword = NULL;
	}
	
    _ODNodeUnlock( pODNodeRef );
    
    // _MapDSErrorToReason will return a non-NULL value if there is already an error set
    cfError = _MapDSErrorToReason( outError, dsStatus );
    
finish:
        
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfNodeName = (NULL != inNodeRef ? ODNodeGetName(inNodeRef) : NULL);
            
            if( NULL != cfNodeName )
            {
                if( NULL != inRecordName )
                {
                    CFStringRef cfTemp = CFSTR("Unable to create record %@ in %@.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inRecordName, cfNodeName );
                    CFRelease( cfTemp );
                }
                else
                {
                    CFStringRef cfTemp = CFSTR("Unable to create a record in %@.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfNodeName );
                    CFRelease( cfTemp );                    
                }
            }
            else
            {                    
                if( NULL != inRecordName )
                {
                    CFStringRef cfTemp = CFSTR("Unable to create record %@ in the node.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inRecordName );
                    CFRelease( cfTemp );
                }
                else
                {
                    CFStringRef cfTemp = CFSTR("Unable to create a record in the node.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR("%@"), cfTemp );
                    CFRelease( cfTemp );                    
                }
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }
    }    
    
    return (ODRecordRef) pODRecord;
}

ODRecordRef ODNodeCopyRecord( ODNodeRef inNodeRef, CFStringRef inRecordType, CFStringRef inRecordName, CFArrayRef inAttributes, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    return _ODNodeCopyRecord( inNodeRef, inRecordType, inRecordName, inAttributes, outError );
}

ODRecordRef _ODNodeCopyRecord( ODNodeRef inNodeRef, CFStringRef inRecordType, CFStringRef inRecordName, CFArrayRef inAttributes, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus    = eDSNoErr;
    ODRecordRef     cfReturnRec     = NULL;
    
    if( NULL == inNodeRef )
    {
        cfError = (NULL != outError ? CFSTR("Missing node reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inRecordType )
    {
        cfError = (NULL != outError ? CFSTR("Missing record type.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inRecordName )
    {
        cfError = (NULL != outError ? CFSTR("Missing record name.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    
    _ODNode         *pODNode        = (_ODNode *) inNodeRef;
    bool         bFailedOnce     = false;
    tDataBufferPtr  dsDataBuffer    = dsDataBufferAllocate( 0, 4096 );
    char            *pRecordName    = _GetCStringFromCFString( inRecordName );
    char            *pRecordType    = _GetCStringFromCFString( inRecordType );
    tDataListPtr    dsNameList      = dsBuildListFromStrings( 0, pRecordName, NULL );
    tDataListPtr    dsRecordTypes   = dsBuildListFromStrings( 0, pRecordType, NULL );
    tDataListPtr    dsAttribList    = NULL;
    UInt32          ulRecCount      = 1;
    tContextData    dsContextData   = 0;
    CFSetRef        cfReturnAttribs = NULL;
    
    if( NULL != inAttributes )
    {
        // we actually allow CFSets too, but we don't advertise that because we use internally
        if( CFGetTypeID(inAttributes) == CFSetGetTypeID() )
        {
            cfReturnAttribs = _minimizeAttributeSet( (CFSetRef) inAttributes );
        }
        else
        {
            cfReturnAttribs = _attributeListToSet( inAttributes );
        }
        
        // ensure we always get metanode location if we are not asking for all attributes or standard
        if( false == CFSetContainsValue(cfReturnAttribs, kODAttributeTypeStandardOnly) &&
            false == CFSetContainsValue(cfReturnAttribs, kODAttributeTypeAllAttributes) )
        {
            CFSetAddValue( (CFMutableSetRef) cfReturnAttribs, kODAttributeTypeMetaNodeLocation );
            CFSetAddValue( (CFMutableSetRef) cfReturnAttribs, kODAttributeTypeRecordName );
        }
    }
    else
    {
        // if NULL is passed, we don't retrieve any attributes.
		ODAttributeType values[] = { kODAttributeTypeRecordName, kODAttributeTypeMetaNodeLocation };
        
        cfReturnAttribs = CFSetCreate( kCFAllocatorDefault, (const void **) values, sizeof(values) / sizeof(ODAttributeType), &kCFTypeSetCallBacks );
    }

    dsAttribList = _ConvertCFSetToDataList( cfReturnAttribs );

    _ODNodeLock( pODNode );
    
    do
    {
        dsStatus = dsGetRecordList( pODNode->_dsNodeRef, dsDataBuffer, dsNameList, eDSExact, dsRecordTypes, dsAttribList,
                                    false, &ulRecCount, &dsContextData );
        
        if( eDSBufferTooSmall == dsStatus )
        {
            UInt32 newSize = (dsDataBuffer->fBufferSize << 1);
            
            if( newSize < 100 * 1024 * 1024 )
            {
                dsDataBufferDeAllocate( 0, dsDataBuffer );
                dsDataBuffer = dsDataBufferAllocate( 0, newSize );
            }
            else
            {
                dsStatus = eDSAllocationFailed;
                break;
            }
        }
        
        if( (eDSInvalidNodeRef == dsStatus || eDSInvalidReference == dsStatus || eDSInvalidDirRef == dsStatus || eDSCannotAccessSession == dsStatus || eDSInvalidRefType == dsStatus) && 
            bFailedOnce == false )
        {
            if( 0 != dsContextData )
            {
                dsReleaseContinueData( pODNode->_ODSession->_dsRef, dsContextData );
                dsContextData = 0;
            }
            
            dsStatus = dsVerifyDirRefNum( pODNode->_ODSession->_dsRef );
            if( eDSInvalidReference == dsStatus || eDSInvalidDirRef == dsStatus || eDSInvalidRefType == dsStatus )
            {
                dsStatus = _ReopenDS( pODNode->_ODSession );
                if( eDSNoErr == dsStatus )
                {
                    // need to reopen the node ref because if DS ref is invalid
                    dsStatus = _ReopenNode( pODNode );
                }
            }
            else // well if dsRef is valid, then it must be the node that is invalid
            {
                dsStatus = _ReopenNode( pODNode );
            }
            
            if( eDSNoErr == dsStatus )
            {
                // reset error status to buffertoosmall to cause the loop to continue
                dsStatus = eDSBufferTooSmall;
            }
            
            bFailedOnce = true;
        }
    } while( eDSBufferTooSmall == dsStatus );
    
    if( eDSNoErr == dsStatus )
    {
        CFMutableArrayRef   outResults = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
        
        _AppendRecordsToList( pODNode, dsDataBuffer, ulRecCount, outResults, outError );
        
        if( 0 != CFArrayGetCount(outResults) )
        {
            cfReturnRec = (ODRecordRef) CFArrayGetValueAtIndex( outResults, 0 );
            CFRetain( cfReturnRec );
            
            _ODRecord *pODRecord = (_ODRecord *) cfReturnRec;
            
            if( NULL != pODRecord->_cfFetchedAttributes )
            {
                CFRelease( pODRecord->_cfFetchedAttributes );
            }
            
            pODRecord->_cfFetchedAttributes = cfReturnAttribs;
            cfReturnAttribs = NULL;
        }
        
        CFRelease( outResults );
        outResults = NULL;
    }
    
    _ODNodeUnlock( pODNode );
    
    if( NULL != cfReturnAttribs )
    {
        CFRelease( cfReturnAttribs );
        cfReturnAttribs = NULL;
    }
    
    if( NULL != dsAttribList )
    {
        dsDataListDeallocate( 0, dsAttribList );
        free( dsAttribList );
        dsAttribList = NULL;
    }
    
    if( NULL != dsRecordTypes )
    {
        dsDataListDeallocate( 0,dsRecordTypes );
        free( dsRecordTypes );
        dsRecordTypes = NULL;
    }
    
    if( NULL != dsNameList )
    {
        dsDataListDeallocate( 0,dsNameList );
        free( dsNameList );
        dsNameList = NULL;
    }
    
    if( NULL != pRecordType )
    {
        free( pRecordType );
        pRecordType = NULL;
    }
    
    if( NULL != pRecordName )
    {
        free( pRecordName );
        pRecordName = NULL;
    }
    
    if( NULL != dsDataBuffer )
    {
        dsDataBufferDeAllocate( 0,  dsDataBuffer );
        dsDataBuffer = NULL;
    }
    
    // _MapDSErrorToReason will return a non-NULL value if there is already an error set
    cfError = _MapDSErrorToReason( outError, dsStatus );
    
finish:
        
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfNodeName = (NULL != inNodeRef ? ODNodeGetName(inNodeRef) : NULL);
            
            if( NULL != cfNodeName )
            {
                if( NULL != inRecordName )
                {
                    CFStringRef cfTemp = CFSTR("Unable to retrieve record %@ from %@.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inRecordName, cfNodeName );
                    CFRelease( cfTemp );
                }
                else
                {
                    CFStringRef cfTemp = CFSTR("Unable to retrieve a record from %@.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfNodeName );
                    CFRelease( cfTemp );                    
                }
            }
            else
            {                    
                if( NULL != inRecordName )
                {
                    CFStringRef cfTemp = CFSTR("Unable to retrieve record %@ from the node.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inRecordName );
                    CFRelease( cfTemp );
                }
                else
                {
                    CFStringRef cfTemp = CFSTR("Unable to retrieve a record from the node.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR("%@"), cfTemp );
                    CFRelease( cfTemp );                    
                }
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }
    }    
    
    return cfReturnRec;
}

CFDataRef ODNodeCustomCall( ODNodeRef inNodeRef, CFIndex inCustomCode, CFDataRef inSendData, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus    = eDSNoErr;
    CFDataRef       cfResponse      = NULL;
    
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inNodeRef )
    {
        cfError = (NULL != outError ? CFSTR("Missing node reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    
   _ODNode         *pODNode        = (_ODNode *) inNodeRef;
    bool         bFailedOnce     = false;
    tDataBufferPtr  dsReceiveBuffer = dsDataBufferAllocate( 0, 1024 * 1024 ); // not all custom calls support eDSBufferTooSmall
    tDataBufferPtr  dsSendBuffer    = dsDataBufferAllocate( 0, NULL != inSendData ? CFDataGetLength(inSendData) : 4 );
    
    if( NULL != inSendData )
    {
        CFDataGetBytes( inSendData, CFRangeMake(0,CFDataGetLength(inSendData)), (UInt8 *) dsSendBuffer->fBufferData );
        dsSendBuffer->fBufferLength = CFDataGetLength( inSendData );
    }
    
    _ODNodeLock( pODNode );
    
    do
    {
        dsStatus = dsDoPlugInCustomCall( pODNode->_dsNodeRef, inCustomCode, dsSendBuffer, dsReceiveBuffer );
        if( eDSBufferTooSmall == dsStatus )
        {
            UInt32 newSize = (dsReceiveBuffer->fBufferSize << 1);
            
            if( newSize < 100 * 1024 * 1024 )
            {
                dsDataBufferDeAllocate( 0, dsReceiveBuffer );
                dsReceiveBuffer = dsDataBufferAllocate( 0, newSize );
            }
            else
            {
                dsStatus = eDSAllocationFailed;
                break;
            }
        }
        
        if( (eDSInvalidNodeRef == dsStatus || eDSInvalidReference == dsStatus || eDSInvalidDirRef == dsStatus || eDSCannotAccessSession == dsStatus || eDSInvalidRefType == dsStatus) && 
            bFailedOnce == false )
        {
            dsStatus = dsVerifyDirRefNum( pODNode->_ODSession->_dsRef );
            if( eDSInvalidReference == dsStatus || eDSInvalidDirRef == dsStatus || eDSInvalidRefType == dsStatus)
            {
                dsStatus = _ReopenDS( pODNode->_ODSession );
                if( eDSNoErr == dsStatus )
                {
                    // need to reopen the node ref because if DS ref is invalid
                    _ReopenNode( pODNode );
                }
            }
            else // well if dsRef is valid, then it must be the node that is invalid
            {
				_ReopenNode( pODNode );
            }
            
            dsStatus = eDSBufferTooSmall;
            bFailedOnce = true;
        }
    } while( eDSBufferTooSmall == dsStatus );
    
    _ODNodeUnlock( pODNode );
    
    if( eDSNoErr == dsStatus )
    {
        cfResponse = CFDataCreate( kCFAllocatorDefault, (UInt8 *)dsReceiveBuffer->fBufferData, dsReceiveBuffer->fBufferLength );
    }
    
    if( NULL != dsReceiveBuffer )
    {
        dsDataBufferDeAllocate( 0, dsReceiveBuffer );
        dsReceiveBuffer = NULL;
    }
    
    if( NULL != dsSendBuffer )
    {
        dsDataBufferDeAllocate( 0, dsSendBuffer );
        dsSendBuffer = NULL;
    }
    
    // _MapDSErrorToReason will return a non-NULL value if there is already an error set
    cfError = _MapDSErrorToReason( outError, dsStatus );
    
finish:
        
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfNodeName = (NULL != inNodeRef ? ODNodeGetName(inNodeRef) : NULL);
            
            if( NULL != cfNodeName )
            {
                CFStringRef cfTemp = CFSTR("Custom call %d to %@ failed.");
                cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inCustomCode, cfNodeName );
                CFRelease( cfTemp );
            }
            else
            {                    
                CFStringRef cfTemp = CFSTR("Custom call %d to node failed.");
                cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inCustomCode );
                CFRelease( cfTemp );
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }
    }    
    
    return cfResponse;
}

tDirNodeReference ODNodeGetDSRef( ODNodeRef inNodeRef )
{
    if( NULL == inNodeRef )
    {
        return 0;
    }
    
    _ODNode *pODNode    = (_ODNode *) inNodeRef;
    
    return pODNode->_dsNodeRef;
}

#pragma mark -
#pragma mark ODRecord functions

CFTypeID ODRecordGetTypeID( void )
{
    static pthread_once_t registerOnce = PTHREAD_ONCE_INIT;
    
    pthread_once( &registerOnce, _ODRecordRegisterClass );

    return _kODRecordTypeID;
}

bool ODRecordSetNodeCredentials( ODRecordRef inRecordRef, CFStringRef inUsername, CFStringRef inPassword, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus;
    bool     bSuccess    = false;
    
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inRecordRef )
    {
        cfError = (NULL != outError ? CFSTR("Missing node reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inUsername )
    {
        cfError = (NULL != outError ? CFSTR("Missing username.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inPassword )
    {
        cfError = (NULL != outError ? CFSTR("Missing password.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    
    CF_OBJC_FUNCDISPATCH( _kODRecordTypeID, bool, inRecordRef, "setNodeCredentials:password:error:", inUsername, inPassword, outError );
    
    _VerifyNodeTypeForChange( inRecordRef, outError );
    
    _ODRecord   *pODRecRef  = (_ODRecord *) inRecordRef;
    CFStringRef cfCurrUser  = CFDictionaryGetValue( pODRecRef->_ODNode->_info, kODNodeUsername );
    
    _ODRecordLock( pODRecRef );
    
    // no need to check pODRecRef->_ODNode->_cfNodePassword since if there is a current user, then there is a password
    if( NULL == cfCurrUser || 
        CFStringCompare(cfCurrUser, inUsername, 0) != kCFCompareEqualTo || 
        CFStringCompare(pODRecRef->_ODNode->_cfNodePassword, inPassword, 0) != kCFCompareEqualTo )
    {
        ODNodeRef  cfNewNode   = _ODNodeCreateCopy( CFGetAllocator(inRecordRef), (ODNodeRef) pODRecRef->_ODNode, outError );
        
        if( NULL != cfNewNode )
        {
            bSuccess = _ODNodeSetCredentials( cfNewNode, NULL, inUsername, inPassword, outError );
            if( bSuccess )
            {
                CFRelease( (CFTypeRef) (pODRecRef->_ODNode) );
                pODRecRef->_ODNode = (_ODNode *) cfNewNode;
            }
            else
            {
                CFRelease( cfNewNode );
                cfNewNode = NULL;
            }
        }
    }
    else
    {
        bSuccess = true;
    }
    
    _ODRecordUnlock( pODRecRef );
    
finish:
        
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfRecordName = (NULL != inRecordRef ? ODRecordGetRecordName(inRecordRef) : NULL);
            
            if( NULL != cfRecordName )
            {
                CFStringRef cfTemp = CFSTR("Unable to set the node credentials for the record %@.");
                cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfRecordName );
                CFRelease( cfTemp );
            }
            else
            {                    
                cfDescription = CFSTR("Unable to set the node credentials for a record.");
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }
    }        
    
    return bSuccess;
}

bool ODRecordSetNodeCredentialsExtended( ODRecordRef inRecordRef, CFStringRef inRecordType, CFStringRef inAuthType, 
                                            CFArrayRef inAuthItems, CFArrayRef *outAuthItems, ODContextRef *outContext,
                                            CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus;
    bool     bSuccess    = false;
    
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inRecordRef )
    {
        cfError = (NULL != outError ? CFSTR("Invalid record reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inRecordType )
    {
        cfError = (NULL != outError ? CFSTR("Missing record type.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inAuthType )
    {
        cfError = (NULL != outError ? CFSTR("Missing authentication type.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inAuthItems )
    {
        cfError = (NULL != outError ? CFSTR("The user name, password, or both are missing.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    
    CF_OBJC_FUNCDISPATCH( _kODRecordTypeID, bool, inRecordRef, 
                          "setNodeCredentialsWithRecordType:authenticationType:authenticationItems:continueItems:context:error:", 
                          inRecordType, inAuthType, inAuthItems, outAuthItems, outContext, outError );
    
    _VerifyNodeTypeForChange( inRecordRef, outError );
    
    _ODRecord   *pODRecRef  = (_ODRecord *) inRecordRef;
    
    pthread_mutex_lock( &pODRecRef->_mutex );
    
    ODNodeRef  cfNewNode   = _ODNodeCreateCopy( CFGetAllocator(inRecordRef), (ODNodeRef) pODRecRef->_ODNode, outError );
    if( NULL != cfNewNode )
    {
        bSuccess = _ODNodeSetCredentialsExtended( cfNewNode, inRecordType, inAuthType, inAuthItems,
                                                  outAuthItems, outContext, outError, false );
        if( bSuccess )
        {
            CFRelease( (CFTypeRef) (pODRecRef->_ODNode) );
            pODRecRef->_ODNode = (_ODNode *) cfNewNode;
        }
        else
        {
            CFRelease( cfNewNode );
            cfNewNode = NULL;
        }
    }
    
    pthread_mutex_unlock( &pODRecRef->_mutex );
    
finish:
        
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfRecordName = (NULL != inRecordRef ? ODRecordGetRecordName(inRecordRef) : NULL);
            
            if( NULL != cfRecordName )
            {
                CFStringRef cfTemp = CFSTR("Unable to set the extended node credentials for the record %@.");
                cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfRecordName );
                CFRelease( cfTemp );
            }
            else
            {                    
                cfDescription = CFSTR("Unable to set the extended node credentials for a record.");
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }
    }    
    
    return bSuccess;
}

bool ODRecordSetNodeCredentialsUsingKerberosCache( ODRecordRef inRecordRef, CFStringRef inCacheName, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus;
    bool     bSuccess    = false;
    
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
    
    if( NULL == inRecordRef )
    {
        cfError = (NULL != outError ? CFSTR("Invalid record reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    
    CF_OBJC_FUNCDISPATCH( _kODRecordTypeID, bool, inRecordRef, "setNodeCredentialsUsingKerberosCache:error:", inCacheName, outError );
    
    _VerifyNodeTypeForChange( inRecordRef, outError );
    
    _ODRecord   *pODRecRef  = (_ODRecord *) inRecordRef;
    
    pthread_mutex_lock( &pODRecRef->_mutex );
    
    ODNodeRef  cfNewNode   = _ODNodeCreateCopy( CFGetAllocator(inRecordRef), (ODNodeRef) pODRecRef->_ODNode, outError );
    if( NULL != cfNewNode )
    {
        bSuccess = ODNodeSetCredentialsUsingKerberosCache( cfNewNode, inCacheName, outError );
        if( bSuccess )
        {
            CFRelease( (CFTypeRef) (pODRecRef->_ODNode) );
            pODRecRef->_ODNode = (_ODNode *) cfNewNode;
        }
        else
        {
            CFRelease( cfNewNode );
            cfNewNode = NULL;
        }
    }
    
    pthread_mutex_unlock( &pODRecRef->_mutex );
    
finish:
    
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfRecordName = (NULL != inRecordRef ? ODRecordGetRecordName(inRecordRef) : NULL);
            
            if( NULL != cfRecordName )
            {
                CFStringRef cfTemp = CFSTR("Unable to set the node credentials for the record %@.");
                cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfRecordName );
                CFRelease( cfTemp );
            }
            else
            {                    
                cfDescription = CFSTR("Unable to set the node credentials for a record.");
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }
    }    
    
    return bSuccess;
}

CFDictionaryRef ODRecordCopyPasswordPolicy( CFAllocatorRef inAllocator, ODRecordRef inRecordRef, CFErrorRef *outError )
{
    CFStringRef cfError = NULL;
    tDirStatus  dsStatus;
	CFMutableDictionaryRef  cfReturn    = NULL;
   
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inRecordRef )
    {
        cfError = (NULL != outError ? CFSTR("Invalid record reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    
    _VerifyNodeTypeForChange( inRecordRef, outError );
    
    _ODRecord               *pODRecord  = (_ODRecord *) inRecordRef;
    CFArrayRef              cfAuthItems = CFArrayCreate( kCFAllocatorDefault, (const void **) &(pODRecord->_cfRecordName), 1,
                                                         &kCFTypeArrayCallBacks );
    CFArrayRef              cfResponse  = NULL;
    
    _ODRecordLock( pODRecord );
    
    _Authenticate( (ODNodeRef) pODRecord->_ODNode, kDSStdAuthGetEffectivePolicy, NULL, cfAuthItems, &cfResponse, NULL, true );
    
    _ODRecordUnlock( pODRecord );
    
    if( NULL != cfResponse )
    {
        if( CFArrayGetCount(cfResponse) != 0 )
        {
            CFDataRef   cfData  = (CFDataRef) CFArrayGetValueAtIndex( cfResponse, 0 );
            
            if( NULL != cfData )
            {
                CFIndex iLength     = CFDataGetLength( cfData );
                char    *pPolicy    = (char *) calloc( CFDataGetLength(cfData) + 1, sizeof(UInt8) );
                char    *xmlDataStr = NULL;
                
                CFDataGetBytes( cfData, CFRangeMake(0,iLength), (UInt8 *)pPolicy );
                
                if( ConvertSpaceDelimitedPoliciesToXML(pPolicy, 1, &xmlDataStr) == 0 )
                {
                    CFDataRef   cfXMLData   = CFDataCreate( kCFAllocatorDefault, (const UInt8 *)xmlDataStr, strlen(xmlDataStr) );
                    
                    if( NULL != cfXMLData )
                    {
                        cfReturn = (CFMutableDictionaryRef) CFPropertyListCreateFromXMLData( kCFAllocatorDefault, cfXMLData, 
                                                                                             kCFPropertyListMutableContainers, NULL );
                        
                        CFRelease( cfXMLData );
                        cfXMLData = NULL;
                    }
                    
                    free( xmlDataStr );
                }

                free( pPolicy );
            }
        }
        
        CFRelease( cfResponse );
        cfResponse = NULL;
    }
    
    if( NULL != cfAuthItems )
    {
        CFRelease( cfAuthItems );
        cfAuthItems = NULL;
    }
    
finish:
        
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfRecordName = (NULL != inRecordRef ? ODRecordGetRecordName(inRecordRef) : NULL);
            
            if( NULL != cfRecordName )
            {
                CFStringRef cfTemp = CFSTR("Unable to retrieve password policies for record %@.");
                cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfRecordName );
                CFRelease( cfTemp );
            }
            else
            {                    
                cfDescription = CFSTR("Unable to retrieve password policies for a record.");
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }
    }    

    return cfReturn;
}

bool ODRecordVerifyPassword( ODRecordRef inRecordRef, CFStringRef inPassword, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus;
    
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inRecordRef )
    {
        cfError = (NULL != outError ? CFSTR("Invalid record reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inPassword )
    {
        cfError = (NULL != outError ? CFSTR("Missing password.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    
    CF_OBJC_FUNCDISPATCH( _kODRecordTypeID, bool, inRecordRef, "verifyPassword:error:", inPassword, outError );
    
    _VerifyNodeTypeForChange( inRecordRef, outError );
    
    _ODRecord   *pODRecord  = (_ODRecord *) inRecordRef;
    CFTypeRef   values[]    = { pODRecord->_cfRecordName, inPassword, NULL };
    CFArrayRef  cfAuthItems = CFArrayCreate( kCFAllocatorDefault, values, 2, &kCFTypeArrayCallBacks );
    
    _ODRecordLock( pODRecord );
    
    dsStatus = _Authenticate( (ODNodeRef) pODRecord->_ODNode, kDSStdAuthNodeNativeClearTextOK, NULL, cfAuthItems, NULL, NULL, true );
    
    _ODRecordUnlock( pODRecord );
    
    if( NULL != cfAuthItems )
    {
        CFRelease( cfAuthItems );
        cfAuthItems = NULL;
    }
    
    // _MapDSErrorToReason will return a non-NULL value if there is already an error set
    cfError = _MapDSErrorToReason( outError, dsStatus );
    
finish:
        
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfRecordName = (NULL != inRecordRef ? ODRecordGetRecordName(inRecordRef) : NULL);
            
            if( NULL != cfRecordName )
            {
                CFStringRef cfTemp = CFSTR("Unable to verify the password for record %@.");
                cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfRecordName );
                CFRelease( cfTemp );
            }
            else
            {                    
                cfDescription = CFSTR("Unable to verify the password for a record.");
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }

        return false;
    }    
    
    return true;
}

bool ODRecordVerifyPasswordExtended( ODRecordRef inRecordRef, CFStringRef inAuthType, CFArrayRef inAuthItems, CFArrayRef *outAuthItems,
                                        ODContextRef *outContext, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus    = eDSAuthFailed;
    
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inRecordRef )
    {
        cfError = (NULL != outError ? CFSTR("Missing record reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inAuthType )
    {
        cfError = (NULL != outError ? CFSTR("Missing authentication type.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inAuthItems )
    {
        cfError = (NULL != outError ? CFSTR("The user name, password, or both are missing.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    
    CF_OBJC_FUNCDISPATCH( _kODRecordTypeID, bool, inRecordRef, 
                          "verifyExtendedWithAuthenticationType:authenticationItems:continueItems:context:error:", 
                          inAuthType, inAuthItems, outAuthItems, outContext, outError );
    
    _VerifyNodeTypeForChange( inRecordRef, outError );
    
    _ODRecord   *pODRecord  = (_ODRecord *) inRecordRef;
    char        *pAuthType  = _GetCStringFromCFString( inAuthType );
    
    if( NULL != pAuthType )
    {
        _ODRecordLock( pODRecord );
        
        dsStatus = _Authenticate( (ODNodeRef) pODRecord->_ODNode, pAuthType, NULL, inAuthItems, outAuthItems, NULL, true );
        
        _ODRecordUnlock( pODRecord );
        
        free( pAuthType );
        pAuthType = NULL;
    }
    else
    {
        cfError = (NULL != outError ? CFSTR("Authentication type was an empty value.") : CFSTR(""));
        goto finish;
    }
    
    // _MapDSErrorToReason will return a non-NULL value if there is already an error set
    cfError = _MapDSErrorToReason( outError, dsStatus );
    
finish:
        
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfRecordName = (NULL != inRecordRef ? ODRecordGetRecordName(inRecordRef) : NULL);
            
            if( NULL != cfRecordName )
            {
                CFStringRef cfTemp = CFSTR("Unable to verify extended credentials for record %@.");
                cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfRecordName );
                CFRelease( cfTemp );
            }
            else
            {                    
                cfDescription = CFSTR("Unable to verify extended credentials for a record.");
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }

        return false;
    }    
    
    return true;
}

bool ODRecordChangePassword( ODRecordRef inRecordRef, CFStringRef inOldPassword, CFStringRef inNewPassword, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    return _ODRecordChangePassword( inRecordRef, inOldPassword, inNewPassword, outError );
}

bool _ODRecordChangePassword( ODRecordRef inRecordRef, CFStringRef inOldPassword, CFStringRef inNewPassword, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus    = eDSAuthFailed;

    if( NULL == inRecordRef )
    {
        cfError = (NULL != outError ? CFSTR("Invalid record reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inNewPassword )
    {
        cfError = (NULL != outError ? CFSTR("Missing new password.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    
    CF_OBJC_FUNCDISPATCH( _kODRecordTypeID, bool, inRecordRef, "changePassword:toPassword:error:", inOldPassword, inNewPassword, outError );
    
    _VerifyNodeTypeForChange( inRecordRef, outError );
    
    _ODRecord   *pODRecord  = (_ODRecord *) inRecordRef;
    CFArrayRef  cfAuthItems = NULL;
    char        *pAuthType  = NULL;
    char        *pRecType   = _GetCStringFromCFString( pODRecord->_cfRecordType );
    
    if( NULL != inOldPassword )
    {
        CFTypeRef   values[]    = { pODRecord->_cfRecordName, inOldPassword, inNewPassword, NULL };
        
        cfAuthItems = CFArrayCreate( kCFAllocatorDefault, values, 3, &kCFTypeArrayCallBacks );
        pAuthType = kDSStdAuthChangePasswd;
    }
    else
    {
        CFTypeRef   values[]    = { pODRecord->_cfRecordName, inNewPassword, NULL };
        
        cfAuthItems = CFArrayCreate( kCFAllocatorDefault, values, 2, &kCFTypeArrayCallBacks );
        pAuthType = kDSStdAuthSetPasswdAsRoot;
    }
    
    _ODRecordLock( pODRecord );
    
    dsStatus = _Authenticate( (ODNodeRef) pODRecord->_ODNode, pAuthType, pRecType, cfAuthItems, NULL, NULL, true );
    
    _ODRecordUnlock( pODRecord );
    
    if( NULL != pRecType )
    {
        free( pRecType );
        pRecType = NULL;
    }
    
    if( NULL != cfAuthItems )
    {
        CFRelease( cfAuthItems );
        cfAuthItems = NULL;
    }
    
    // _MapDSErrorToReason will return a non-NULL value if there is already an error set
    cfError = _MapDSErrorToReason( outError, dsStatus );
    
finish:
        
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfRecordName = (NULL != inRecordRef ? ODRecordGetRecordName(inRecordRef) : NULL);
            
            if( NULL != cfRecordName )
            {
                CFStringRef cfTemp = CFSTR("Unable to change the password for record %@.");
                cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfRecordName );
                CFRelease( cfTemp );
            }
            else
            {                    
                cfDescription = CFSTR("Unable to changethe password for a record.");
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }

        return false;
    }
    
    return true;
}

CFStringRef ODRecordGetRecordType( ODRecordRef inRecordRef )
{
    if( NULL == inRecordRef )
    {
        return NULL;
    }
    
    _ODRecord   *pODRecord  = (_ODRecord *) inRecordRef;
    
    return pODRecord->_cfRecordType;
}

CFStringRef ODRecordGetRecordName( ODRecordRef inRecordRef )
{
    if( NULL == inRecordRef )
    {
        return NULL;
    }
    
    _ODRecord   *pODRecord  = (_ODRecord *) inRecordRef;
    
    return pODRecord->_cfRecordName;
}

CFArrayRef ODRecordCopyValues( ODRecordRef inRecordRef, ODAttributeType inAttribute, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
    
    CFArrayRef values = _ODRecordGetValues( inRecordRef, inAttribute, outError );
    if( NULL != values )
    {
        values = CFArrayCreateCopy( kCFAllocatorDefault, values );
    }
    
    return values;
}

CFArrayRef _ODRecordGetValues( ODRecordRef inRecordRef, ODAttributeType inAttribute, CFErrorRef *outError )
{
    tDirStatus  dsStatus    = eDSNoErr;
    CFStringRef cfError     = NULL;
	CFMutableArrayRef   cfValues    = NULL;
   
    if( NULL == inRecordRef )
    {
        cfError = (NULL != outError ? CFSTR("Invalid record reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inAttribute )
    {
        cfError = (NULL != outError ? CFSTR("Missing attribute name.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    
    _ODRecord           *pODRecRef  = (_ODRecord *) inRecordRef;
    
    _ODRecordLock( pODRecRef );

    if( false == _wasAttributeFetched(pODRecRef, inAttribute) )
    {
        if( 0 != pODRecRef->_dsRecordRef )
        {
            tDataNodePtr    dsAttribute = _GetDataBufferFromCFType( inAttribute );
            
            if( NULL != dsAttribute )
            {
                tAttributeEntryPtr  dsAttrEntry = NULL;
                
                do
                {
                    dsStatus = dsGetRecordAttributeInfo( pODRecRef->_dsRecordRef, dsAttribute, &dsAttrEntry );
                    if( eDSNoErr == dsStatus )
                    {
                        uint32_t    count = dsAttrEntry->fAttributeValueCount;
                        uint32_t    ii;
					
                        if (cfValues == NULL) {
							cfValues = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
						}
                        
                        for( ii = 1; ii <= count; ii++ )
                        {
                            tAttributeValueEntryPtr dsAttrValueEntry    = NULL;
                            
                            dsGetRecordAttributeValueByIndex( pODRecRef->_dsRecordRef, dsAttribute, ii, &dsAttrValueEntry );
                            
                            if( NULL != dsAttrValueEntry )
                            {
                                CFTypeRef cfRef = CFStringCreateWithBytes( kCFAllocatorDefault, 
                                                                           (const UInt8 *) dsAttrValueEntry->fAttributeValueData.fBufferData, 
                                                                           dsAttrValueEntry->fAttributeValueData.fBufferLength, kCFStringEncodingUTF8,
                                                                           false );
                                
                                if( NULL == cfRef )
                                {
                                    cfRef = CFDataCreate( kCFAllocatorDefault, (const UInt8 *) dsAttrValueEntry->fAttributeValueData.fBufferData, 
                                                          dsAttrValueEntry->fAttributeValueData.fBufferLength );
                                }
                                
                                if( NULL != cfRef )
                                {
                                    CFArrayAppendValue( cfValues, cfRef );
                                    CFRelease( cfRef );
                                    cfRef = NULL;
                                }
                                
                                dsDeallocAttributeValueEntry( 0, dsAttrValueEntry );
                                dsAttrValueEntry = NULL;
                            }
                        }
                        
                        dsDeallocAttributeEntry( 0, dsAttrEntry );
                        dsAttrEntry = NULL;
                    }
                    else if( eDSInvalidReference == dsStatus || eDSInvalidRecordRef == dsStatus || eDSInvalidDirRef == dsStatus || 
                             eDSInvalidNodeRef == dsStatus || eDSCannotAccessSession == dsStatus || eDSInvalidRefType == dsStatus )
                    {
                        dsStatus = _ReopenRecord( pODRecRef );
                    }
                    
                } while( eDSInvalidReference == dsStatus || eDSInvalidRecordRef == dsStatus || eDSInvalidDirRef == dsStatus || 
                         eDSInvalidNodeRef == dsStatus || eDSInvalidRefType == dsStatus );
                
                dsDataBufferDeAllocate( 0, dsAttribute );
                dsAttribute = NULL;
            }
            else
            {
                dsStatus = eDSInvalidAttributeType;
            }
        }
        else
        {
            CFArrayRef  cfAttribs = CFArrayCreate( kCFAllocatorDefault, (const void **) &inAttribute, 1, &kCFTypeArrayCallBacks );
            
            _ODRecordUnlock( pODRecRef );

            _VerifyNodeTypeForChange( inRecordRef, outError );
            
            _ODRecordLock( pODRecRef );

            // cast is okay because we actually allow both Sets and Arrays, but we don't publicize
            ODRecordRef cfNewRecord     = _ODNodeCopyRecord( (ODNodeRef) pODRecRef->_ODNode, pODRecRef->_cfRecordType, pODRecRef->_cfRecordName, 
                                                             cfAttribs, outError );
            
            if( NULL != cfNewRecord )
            {
                _ODRecord   *pODNewRecord   = (_ODRecord *) cfNewRecord;
                
                cfValues = (CFMutableArrayRef) CFDictionaryGetValue( pODNewRecord->_cfAttributes, inAttribute );
                
                if( NULL != cfValues )
                {
                    CFRetain( cfValues );
                }
                
                CFRelease( cfNewRecord );
                cfNewRecord = NULL;
            }
            
            CFRelease( cfAttribs );
            cfAttribs = NULL;
        }
        
        if( NULL != cfValues )
        {
            // Copy the set and add the fetched attribute
            CFMutableSetRef cfSet = CFSetCreateMutableCopy( kCFAllocatorDefault, 0, pODRecRef->_cfFetchedAttributes );

            CFDictionarySetValue( pODRecRef->_cfAttributes, inAttribute, cfValues );
            
            CFRelease( pODRecRef->_cfFetchedAttributes );
            pODRecRef->_cfFetchedAttributes = cfSet;
            
            CFSetAddValue( cfSet, inAttribute );            

            // we can release it now..
            CFRelease( cfValues );
        }
    }
    else
    {
        cfValues = (CFMutableArrayRef) CFDictionaryGetValue( pODRecRef->_cfAttributes, inAttribute );
    }
    
    _ODRecordUnlock( pODRecRef );
    
    // _MapDSErrorToReason will return a non-NULL value if there is already an error set
    cfError = _MapDSErrorToReason( outError, dsStatus );
    
finish:
        
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfRecordName = (NULL != inRecordRef ? ODRecordGetRecordName(inRecordRef) : NULL);
            
            if( NULL == inAttribute )
            {
                if( NULL != cfRecordName )
                {
                    CFStringRef cfTemp = CFSTR("Unable to obtain attribute values for record %@.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfRecordName );
                    CFRelease( cfTemp );
                }
                else
                {                    
                    cfDescription = CFSTR("Unable to obtain a record attribute values.");
                }
            }
            else
            {
                if( NULL != cfRecordName )
                {
                    CFStringRef cfTemp = CFSTR("Unable to obtain values for attribute %@ of record %@.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inAttribute, cfRecordName );
                    CFRelease( cfTemp );
                }
                else
                {
                    CFStringRef cfTemp = CFSTR("Unable to set values for an attribute %@ of a record.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inAttribute );
                    CFRelease( cfTemp );
                }
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }
    }

    return cfValues;
}

bool ODRecordSetValue( ODRecordRef inRecordRef, ODAttributeType inAttribute, CFTypeRef inValueOrValues, CFErrorRef *outError )
{
    if( outError != NULL )
    {
        (*outError) = NULL;
    }
    
    return _ODRecordSetValues( inRecordRef, inAttribute, inValueOrValues, outError );
}

bool _ODRecordSetValues( ODRecordRef inRecordRef, ODAttributeType inAttribute, CFTypeRef inValues, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus    = eDSInvalidRecordRef;
    CFArrayRef  cfTempArray = NULL;
 	bool		bSuccess	= true;
   
    if( NULL == inRecordRef )
    {
        cfError = (NULL != outError ? CFSTR("Invalid record reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inAttribute )
    {
        cfError = (NULL != outError ? CFSTR("Missing attribute type.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inValues )
    {
        cfError = (NULL != outError ? CFSTR("Missing new values.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( CFStringCompare(inAttribute, kODAttributeTypeMetaNodeLocation, 0) == kCFCompareEqualTo )
    {
        dsStatus = eDSNoErr;
        goto finish;
    }

    CF_OBJC_FUNCDISPATCH( _kODRecordTypeID, bool, inRecordRef, "setValues:forAttribute:error:", inValues, inAttribute, outError );

    if( CFGetTypeID(inValues) == CFDataGetTypeID() || CFGetTypeID(inValues) == CFStringGetTypeID() )
    {
        cfTempArray = CFArrayCreate( kCFAllocatorDefault, (const void **) &inValues, 1, &kCFTypeArrayCallBacks );
        inValues = cfTempArray;
    }
    else if( CFGetTypeID(inValues) != CFArrayGetTypeID() )
    {
        cfError = (NULL != outError ? CFSTR("Invalid type of attribute value.") : CFSTR(""));
        dsStatus = eDSEmptyAttributeValue;
        goto finish;
    }
    
    _VerifyNodeTypeForChange( inRecordRef, outError );
    
    _ODRecord           *pODRecRef      = (_ODRecord *) inRecordRef;
    tDataNodePtr        dsAttributeName = _GetDataBufferFromCFType( inAttribute );
    tDataListPtr        dsDataList      = NULL;
    CFMutableArrayRef   cfRemove        = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
    CFMutableArrayRef   cfAdd           = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
    
    _ODRecordLock( pODRecRef );

    CFArrayRef          cfOldValues     = _ODRecordGetValues( inRecordRef, inAttribute, NULL );

    // we need to special case the record name
    if( CFStringCompare(inAttribute, kODAttributeTypeRecordName, 0) == kCFCompareEqualTo )
    {
        if( NULL != inValues && CFArrayGetCount(inValues) > 0 )
        {
            CFIndex iCount;
            CFIndex ii;
            
            // need to remove the old values because dsSetRecordName, only sets the first value
            if ( cfOldValues != NULL && CFArrayGetCount(cfOldValues) > 1 ) {
                // copy the old list because ODRecordRemoveValue removes the values from the internal copy
                CFArrayRef cfTempList = CFArrayCreateCopy( kCFAllocatorDefault, cfOldValues );
                
                iCount = CFArrayGetCount( cfTempList );
                for ( ii = 1; ii < iCount; ii++ ) {
                    ODRecordRemoveValue( inRecordRef, kODAttributeTypeRecordName, CFArrayGetValueAtIndex(cfTempList, ii), NULL );
                }
                
                CFRelease( cfTempList );
            }
            
            // now add any values
            iCount = CFArrayGetCount( inValues );
            for( ii = 0; ii < iCount; ii++ )
            {
                CFTypeRef   cfValue = CFArrayGetValueAtIndex( inValues, ii );
                
                if( ii == 0 )
                {
                    tDataNodePtr    dsRecordName = _GetDataBufferFromCFType( cfValue );
                    
                    do
                    {
                        dsStatus = dsSetRecordName( pODRecRef->_dsRecordRef, dsRecordName );
                        
                        if( eDSInvalidReference == dsStatus || eDSInvalidRecordRef == dsStatus || eDSInvalidDirRef == dsStatus || 
                            eDSInvalidNodeRef == dsStatus || eDSCannotAccessSession == dsStatus || eDSInvalidRefType == dsStatus )
                        {
                            dsStatus = _ReopenRecord( pODRecRef ); // returns eDSInvalidRecordRef if it succeeds open
                        }
                    } while( eDSInvalidReference == dsStatus || eDSInvalidRecordRef == dsStatus || eDSInvalidDirRef == dsStatus || 
                             eDSInvalidNodeRef == dsStatus || eDSInvalidRefType == dsStatus );
                    
                    if( eDSNoErr == dsStatus)
                    {
                        // now update the record name in our internal representation
                        CFRelease( pODRecRef->_cfRecordName );
                        pODRecRef->_cfRecordName = CFStringCreateCopy( kCFAllocatorDefault, cfValue );
                    }
                    else
                    {
                        if( NULL != outError )
                        {
                            CFStringRef cfTemp = CFSTR("Unable to set the record name to %@.");
                            cfError = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfValue );
                            CFRelease( cfTemp );
                        }
                        goto failed;
                    }
                }
                else if( false == ODRecordAddValue(inRecordRef, inAttribute, cfValue, outError) )
                {
                    goto failed;
                }
            }
            
            // we really didn't fail, but finish is only when we have not entered our critical section
            goto failed;
        }
        else
        {
            cfError = (NULL != outError ? CFSTR("Missing record name. At least one name is required.") : CFSTR(""));
            dsStatus = eDSEmptyAttributeValue;
            goto failed;
        }
    }
    else if( NULL != inValues )
    {
        // if node doesn't support setValues or we don't know yet we need to do more work...
        // we have to prep just in case either don't work unfortunately
        CFIndex     iNewCount   = CFArrayGetCount( inValues );
        CFIndex     iOldCount   = (NULL != cfOldValues ? CFArrayGetCount( cfOldValues) : 0);
        CFRange     cfRange;
        CFIndex     ii;
        
        // if we have a value, see if we can do this more efficiently, specifically, if we are adding a single
        // value or removing a single value, we can do that more efficiently
        if( iOldCount != 0 )
        {
            cfRange = CFRangeMake( 0, iNewCount );
            for( ii = 0; ii < iOldCount; ii++ )
            {
                CFStringRef cfRef = CFArrayGetValueAtIndex( cfOldValues, ii );
                
                if( false == CFArrayContainsValue(inValues, cfRange, cfRef) )
                {
                    CFArrayAppendValue( cfRemove, cfRef );
                }
            }
        }
        
        cfRange = CFRangeMake( 0, iOldCount );
        for( ii = 0; ii < iNewCount; ii++ )
        {
            CFStringRef cfRef = CFArrayGetValueAtIndex( inValues, ii );
            
            if( cfOldValues == NULL || false == CFArrayContainsValue(cfOldValues, cfRange, cfRef) )
            {
                CFArrayAppendValue( cfAdd, cfRef );
            }
        }

        if (0 == (pODRecRef->_ODNode->_flags & kODNodeFlagNoSetValues)) {
            dsDataList = _ConvertCFArrayToDataList( inValues );
        }
    }
    
    do
    {
        // if we have a list and it has values, set the values, otherwise, remove the attribute
        if( NULL != dsDataList && 0 != dsDataListGetNodeCount(dsDataList) )
        {
            dsStatus = dsSetAttributeValues( pODRecRef->_dsRecordRef, dsAttributeName, dsDataList );
            switch ( dsStatus ) {
                case eNotYetImplemented:
                case eNotHandledByThisNode:
                    pODRecRef->_ODNode->_flags |= kODNodeFlagNoSetValues;
                    break;
					
				default:
					break;
            }
        }
        else if( CFArrayGetCount(inValues) == 0 )
        {
            dsStatus = dsRemoveAttribute( pODRecRef->_dsRecordRef, dsAttributeName );
        }
        else
        {
            // if we reach here, we didn't have values to set that will cause an endless loop.
            dsStatus = eDSEmptyAttributeValue;
            break;
        }
        
        if( eDSInvalidReference == dsStatus || eDSInvalidRecordRef == dsStatus || eDSInvalidDirRef == dsStatus || 
            eDSInvalidNodeRef == dsStatus || eDSCannotAccessSession == dsStatus || eDSInvalidRefType == dsStatus )
        {
            dsStatus = _ReopenRecord( pODRecRef ); // returns eDSInvalidRecordRef if it succeeds open
        }
    } while( eDSInvalidReference == dsStatus || eDSInvalidRecordRef == dsStatus || eDSInvalidDirRef == dsStatus || 
             eDSInvalidNodeRef == dsStatus || eDSInvalidRefType == dsStatus );
    
    // if plugin hasn't implemented SetAttributeValues or node doesn't suppport SetValues, we need to fall back
    if( eNotYetImplemented == dsStatus || eNotHandledByThisNode == dsStatus || 0 != (pODRecRef->_ODNode->_flags & kODNodeFlagNoSetValues) )
    {
        CFIndex iCount;
        CFIndex ii;
        
        dsStatus = eDSNoErr; // clear error
        
        // now add any values
        iCount = CFArrayGetCount( cfAdd );
        for( ii = 0; ii < iCount; ii++ )
        {
            CFTypeRef   cfValue = CFArrayGetValueAtIndex( cfAdd, ii );
            
            if( false == ODRecordAddValue(inRecordRef, inAttribute, cfValue, outError) )
            {
                goto failed;
            }
        }

        // now remove any values...
        iCount = CFArrayGetCount( cfRemove );
        for( ii = 0; ii < iCount; ii++ )
        {
            CFTypeRef   cfValue = CFArrayGetValueAtIndex( cfRemove, ii );
            
            if( false == ODRecordRemoveValue(inRecordRef, inAttribute, cfValue, outError) )
            {
                goto failed;
            }
        }
    }
    
    // update our local cache
    if( eDSNoErr == dsStatus )
    {
        if( NULL != inValues )
        {
            CFMutableArrayRef cfAttribs = CFArrayCreateMutableCopy( kCFAllocatorDefault, 0, inValues );
            
            if( NULL != cfAttribs )
            {
                CFDictionarySetValue( pODRecRef->_cfAttributes, inAttribute, cfAttribs );
                
                // Copy the set and add the fetched attribute
                CFMutableSetRef cfSet = CFSetCreateMutableCopy( kCFAllocatorDefault, 0, pODRecRef->_cfFetchedAttributes );
                
                CFRelease( pODRecRef->_cfFetchedAttributes );
                pODRecRef->_cfFetchedAttributes = cfSet;
                
                CFSetAddValue( cfSet, inAttribute );            
                
                CFRelease( cfAttribs );
                cfAttribs = NULL;
            }
            else
            {
                CFDictionaryRemoveValue( pODRecRef->_cfAttributes, inAttribute );
                
                CFMutableSetRef cfSet = CFSetCreateMutableCopy( kCFAllocatorDefault, 0, pODRecRef->_cfFetchedAttributes );
                
                CFRelease( pODRecRef->_cfFetchedAttributes );
                pODRecRef->_cfFetchedAttributes = cfSet;
                
                CFSetRemoveValue( cfSet, inAttribute );
            }
        }
        else
        {
            CFDictionaryRemoveValue( pODRecRef->_cfAttributes, inAttribute );
        }
    }
    
failed:

    _ODRecordUnlock( pODRecRef );
    
    if( NULL != dsAttributeName )
    {
        dsDataBufferDeAllocate( 0, dsAttributeName );
        dsAttributeName = NULL;
    }
    
    if( NULL != dsDataList )
    {
        dsDataListDeallocate( 0, dsDataList );
        free( dsDataList ); // free the pointer itself
        
        dsDataList = NULL;
    }
    
    if( NULL != cfAdd )
    {
        CFRelease( cfAdd );
        cfAdd = NULL;
    }

    if( NULL != cfRemove )
    {
        CFRelease( cfRemove );
        cfRemove = NULL;
    }
    
    // _MapDSErrorToReason will return a non-NULL value if there is already an error set
    if (cfError == NULL) {
		cfError = _MapDSErrorToReason( outError, dsStatus );
	}
    
finish:
	
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfRecordName = (NULL != inRecordRef ? ODRecordGetRecordName(inRecordRef) : NULL);
            
            if( NULL == inAttribute )
            {
                if( NULL != cfRecordName )
                {
                    if( NULL != inValues && CFGetTypeID(inValues) == CFStringGetTypeID() && CFStringGetLength((CFStringRef) inValues) < 20 )
                    {
                        CFStringRef cfTemp = CFSTR("Unable to set the attribute value %@ in the record %@.");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inValues, cfRecordName );
                        CFRelease( cfTemp );
                    }
                    else
                    {
                        CFStringRef cfTemp = CFSTR("Unable to set attribute value or values for %@");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfRecordName );
                        CFRelease( cfTemp );
                    }
                }
                else
                {                    
                    cfDescription = CFSTR("Unable to set attribute value or values for a record.");
                }
            }
            else
            {
                if( NULL != cfRecordName )
                {
                    if( NULL != inValues && CFGetTypeID(inValues) == CFStringGetTypeID() && CFStringGetLength((CFStringRef) inValues) < 20 )
                    {
                        CFStringRef cfTemp = CFSTR("Unable to set %@ for %@ in record %@.");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inValues, inAttribute, cfRecordName );
                        CFRelease( cfTemp );
                    }
                    else
                    {
                        CFStringRef cfTemp = CFSTR("Unable to set value or values for %@ in the record %@.");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inAttribute, cfRecordName );
                        CFRelease( cfTemp );
                    }
                }
                else
                {
                    if( NULL != inValues && CFGetTypeID(inValues) == CFStringGetTypeID() && CFStringGetLength((CFStringRef) inValues) < 20 )
                    {
                        CFStringRef cfTemp = CFSTR("Unable to set %@ to %@ in the record.");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inValues, inAttribute );
                        CFRelease( cfTemp );
                    }
                    else
                    {
                        CFStringRef cfTemp = CFSTR("Unable to set value or values for %@ in the record.");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inAttribute );
                        CFRelease( cfTemp );
                    }
                }
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }

        bSuccess = false;
    }
	
    if( NULL != cfTempArray )
    {
        CFRelease( cfTempArray );
        cfTempArray = NULL;
    }
    
    return bSuccess;
}

bool ODRecordAddValue( ODRecordRef inRecordRef, ODAttributeType inAttribute, CFTypeRef inValue, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    return _ODRecordAddValue( inRecordRef, inAttribute, inValue, outError );
}

bool _ODRecordAddValue( ODRecordRef inRecordRef, ODAttributeType inAttribute, CFTypeRef inValue, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus    = eDSInvalidRecordRef;
    
    if( NULL == inRecordRef )
    {
        cfError = (NULL != outError ? CFSTR("Invalid record reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inAttribute )
    {
        cfError = (NULL != outError ? CFSTR("Missing new password.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inValue )
    {
        cfError = (NULL != outError ? CFSTR("No value provided.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( CFStringCompare(inAttribute, kODAttributeTypeMetaNodeLocation, 0) == kCFCompareEqualTo )
    {
        dsStatus = eDSNoErr;
        goto finish;
    }
    
    CF_OBJC_FUNCDISPATCH( _kODRecordTypeID, bool, inRecordRef, "addValue:toAttribute:error:", inValue, inAttribute, outError );

    _VerifyNodeTypeForChange( inRecordRef, outError );
    
    _ODRecord       *pODRecRef          = (_ODRecord *) inRecordRef;
    tDataNodePtr    dsAttributeName     = _GetDataBufferFromCFType( inAttribute );
    tDataNodePtr    dsAttributeValue    = _GetDataBufferFromCFType( inValue );
    
    _ODRecordLock( pODRecRef );
    
    do
    {
        dsStatus = dsAddAttributeValue( pODRecRef->_dsRecordRef, dsAttributeName, dsAttributeValue );
        switch (dsStatus) {
            case eDSAttributeNotFound:
            case eDSInvalidAttributeType:
            case eDSAttributeDoesNotExist:
                dsStatus = dsAddAttribute( pODRecRef->_dsRecordRef, dsAttributeName, NULL, dsAttributeValue );
                break;
                
        }
        
        if( eDSInvalidReference == dsStatus || eDSInvalidRecordRef == dsStatus || eDSInvalidDirRef == dsStatus || 
            eDSInvalidNodeRef == dsStatus || eDSCannotAccessSession == dsStatus || eDSInvalidRefType == dsStatus )
        {
            dsStatus = _ReopenRecord( pODRecRef ); // returns eDSInvalidRecordRef if it succeeds open
        }
    } while( eDSInvalidReference == dsStatus || eDSInvalidRecordRef == dsStatus || eDSInvalidDirRef == dsStatus || 
             eDSInvalidNodeRef == dsStatus || eDSInvalidRefType == dsStatus );
    
    if( eDSNoErr == dsStatus )
    {
        // if we've previously fetched the attribute, we append the value, otherwise do nothing
        // since there might be other attributes
        
        CFMutableArrayRef   cfValues = (CFMutableArrayRef) CFDictionaryGetValue( pODRecRef->_cfAttributes, inAttribute );
        if( NULL != cfValues )
        {
            CFArrayAppendValue( cfValues, inValue );
        }
    }

    _ODRecordUnlock( pODRecRef );

    if( NULL != dsAttributeValue )
    {
        dsDataBufferDeAllocate( 0, dsAttributeValue );
        dsAttributeValue = NULL;
    }
    
    if( NULL != dsAttributeName )
    {
        dsDataBufferDeAllocate( 0, dsAttributeName );
        dsAttributeName = NULL;
    }
    
    // _MapDSErrorToReason will return a non-NULL value if there is already an error set
    cfError = _MapDSErrorToReason( outError, dsStatus );
    
finish:
    
    if( NULL != cfError )
    {
        // if we don't have an error set already (save time)
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfRecordName = (NULL != inRecordRef ? ODRecordGetRecordName(inRecordRef) : NULL);
            
            if( NULL == inAttribute )
            {
                if( NULL != cfRecordName )
                {
                    if( NULL != inValue && CFGetTypeID(inValue) == CFStringGetTypeID() && CFStringGetLength(inValue) < 20 )
                    {
                        CFStringRef cfTemp = CFSTR("Unable to add the attribute value %@ in record %@.");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inValue, cfRecordName );
                        CFRelease( cfTemp );
                    }
                    else
                    {
                        CFStringRef cfTemp = CFSTR("Unable to add an attribute value in record %@.");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inValue, cfRecordName );
                        CFRelease( cfTemp );
                    }
                }
                else
                {                    
                    cfDescription = CFSTR("Unable to add an attribute value to the record.");
                }
            }
            else
            {
                if( NULL != cfRecordName )
                {
                    if( NULL != inValue && CFGetTypeID(inValue) == CFStringGetTypeID() && CFStringGetLength(inValue) < 20 )
                    {
                        CFStringRef cfTemp = CFSTR("Unable to add %@ to %@ in the record %@.");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inValue, inAttribute, cfRecordName );
                        CFRelease( cfTemp );
                    }
                    else
                    {
                        CFStringRef cfTemp = CFSTR("Unable to add a value to %@ in the record %@.");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inAttribute, cfRecordName );
                        CFRelease( cfTemp );
                    }
                }
                else
                {
                    if( NULL != inValue && CFGetTypeID(inValue) == CFStringGetTypeID() && CFStringGetLength(inValue) < 20 )
                    {
                        CFStringRef cfTemp = CFSTR("Unable to add %@ to %@ for a record.");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inValue, inAttribute );
                        CFRelease( cfTemp );
                    }
                    else
                    {
                        CFStringRef cfTemp = CFSTR("Unable to add a value to %@ for a record.");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inAttribute );
                        CFRelease( cfTemp );
                    }
                }
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }

        return false;
    }
    
    return true;
}

bool ODRecordRemoveValue( ODRecordRef inRecordRef, ODAttributeType inAttribute, CFTypeRef inValue, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus    = eDSAttributeValueNotFound;
    
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inRecordRef )
    {
        cfError = (NULL != outError ? CFSTR("Invalid record reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inAttribute )
    {
        cfError = (NULL != outError ? CFSTR("Missing attribute name.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inValue )
    {
        cfError = (NULL != outError ? CFSTR("No value provided.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( CFStringCompare(inAttribute, kODAttributeTypeMetaNodeLocation, 0) == kCFCompareEqualTo )
    {
        dsStatus = eDSNoErr;
        goto finish;
    }    
    
    CF_OBJC_FUNCDISPATCH( _kODRecordTypeID, bool, inRecordRef, "removeValue:fromAttribute:error:", inValue, inAttribute, outError );
    
    _VerifyNodeTypeForChange( inRecordRef, outError );
    
    _ODRecord               *pODRecRef          = (_ODRecord *) inRecordRef;
    tDataNodePtr            dsAttributeName     = _GetDataBufferFromCFType( inAttribute );
    tDataNodePtr            dsAttributeValue    = _GetDataBufferFromCFType( inValue );
    tAttributeValueEntryPtr dsAttrValueEntry    = NULL;
    UInt32                  attributeValueID    = 0;
    
    _ODRecordLock( pODRecRef );
    
    do
    {
        dsStatus = dsGetRecordAttributeValueByValue( pODRecRef->_dsRecordRef, dsAttributeName, dsAttributeValue, &dsAttrValueEntry );
        if( eDSNoErr == dsStatus )
        {
            attributeValueID = dsAttrValueEntry->fAttributeValueID;
            dsDeallocAttributeValueEntry( 0, dsAttrValueEntry );
        }
        else if( eNotYetImplemented == dsStatus || eNotHandledByThisNode == dsStatus )
        {
            UInt32 iIndex = 1;
            do
            {
                dsStatus = dsGetRecordAttributeValueByIndex( pODRecRef->_dsRecordRef, dsAttributeName, iIndex, &dsAttrValueEntry );
                if( eDSNoErr == dsStatus )
                {
                    if( dsAttrValueEntry->fAttributeValueData.fBufferLength == dsAttributeValue->fBufferLength &&
                        0 == bcmp(dsAttrValueEntry->fAttributeValueData.fBufferData, dsAttributeValue->fBufferData, dsAttributeValue->fBufferLength) )
                    {
                        attributeValueID = dsAttrValueEntry->fAttributeValueID;
                    }
                    
                    dsDeallocAttributeValueEntry( 0, dsAttrValueEntry );
                }
                iIndex++;
            } while( 0 == attributeValueID && dsStatus == eDSNoErr );
        }
        
        if( 0 != attributeValueID )
        {
            dsStatus = dsRemoveAttributeValue( pODRecRef->_dsRecordRef, dsAttributeName, attributeValueID );
        }
        
        if( eDSInvalidReference == dsStatus || eDSInvalidRecordRef == dsStatus || eDSInvalidDirRef == dsStatus || 
            eDSInvalidNodeRef == dsStatus || eDSCannotAccessSession == dsStatus || eDSInvalidRefType == dsStatus )
        {
            dsStatus = _ReopenRecord( pODRecRef ); // returns eDSInvalidRecordRef if it succeeds open
        }
        
    } while( eDSInvalidReference == dsStatus || eDSInvalidRecordRef == dsStatus || eDSInvalidDirRef == dsStatus || 
             eDSInvalidNodeRef == dsStatus || eDSInvalidRefType == dsStatus );
    
    // if we didn't find an ID or we got an error the value didn't exist, just return no error, we don't care
    // that the value didn't exist, they just asked to remove it
    if( eDSAttributeValueNotFound == dsStatus || 0 == attributeValueID )
    {
        dsStatus = eDSNoErr;
    }
    
    // update our local cache
    if( eDSNoErr == dsStatus )
    {
        CFMutableArrayRef   cfValues = (CFMutableArrayRef) CFDictionaryGetValue( pODRecRef->_cfAttributes, inAttribute );
        if( NULL != cfValues )
        {
            CFIndex iIndex = CFArrayGetFirstIndexOfValue( cfValues, CFRangeMake(0,CFArrayGetCount(cfValues)), inValue );
            
            if( kCFNotFound != iIndex )
            {
                CFArrayRemoveValueAtIndex( cfValues, iIndex );
            }
        }
    }
    
    _ODRecordUnlock( pODRecRef );
    
    if( NULL != dsAttributeValue )
    {
        dsDataBufferDeAllocate( 0, dsAttributeValue );
        dsAttributeValue = NULL;
    }
    
    if( NULL != dsAttributeName )
    {
        dsDataBufferDeAllocate( 0, dsAttributeName );
        dsAttributeName = NULL;
    }
    
    // _MapDSErrorToReason will return a non-NULL value if there is already an error set
    cfError = _MapDSErrorToReason( outError, dsStatus );
    
finish:
        
    if( NULL != cfError )
    {
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfRecordName = (NULL != inRecordRef ? ODRecordGetRecordName(inRecordRef) : NULL);
            
            if( NULL == inAttribute )
            {
                if( NULL != cfRecordName )
                {
                    if( NULL != inValue && CFGetTypeID(inValue) == CFStringGetTypeID() && CFStringGetLength(inValue) < 20 )
                    {
                        CFStringRef cfTemp = CFSTR("Unable to remove the attribute value %@ for record %@.");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inValue, cfRecordName );
                        CFRelease( cfTemp );
                    }
                    else
                    {
                        CFStringRef cfTemp = CFSTR("Unable to remove an attribute value for record %@.");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inValue, cfRecordName );
                        CFRelease( cfTemp );
                    }
                }
                else
                {                    
                    cfDescription = CFSTR("Unable to remove an attribute value from the record.");
                }
            }
            else
            {
                if( NULL != cfRecordName )
                {
                    if( NULL != inValue && CFGetTypeID(inValue) == CFStringGetTypeID() && CFStringGetLength(inValue) < 20 )
                    {
                        CFStringRef cfTemp = CFSTR("Unable to remove %@ from %@ for record %3$@.");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inValue, inAttribute, cfRecordName );
                        CFRelease( cfTemp );
                    }
                    else
                    {
                        CFStringRef cfTemp = CFSTR("Unable to remove a value from %@ for record %@.");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inAttribute, cfRecordName );
                        CFRelease( cfTemp );
                    }
                }
                else
                {
                    if( NULL != inValue && CFGetTypeID(inValue) == CFStringGetTypeID() )
                    {
                        CFStringRef cfTemp = CFSTR("Unable to remove %@ from %@ for the record.");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inValue, inAttribute );
                        CFRelease( cfTemp );
                    }
                    else
                    {
                        CFStringRef cfTemp = CFSTR("Unable to remove a value from %@ for the record.");
                        cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inAttribute );
                        CFRelease( cfTemp );
                    }
                }
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }

        return false;
    }
    
    return true;
}

CFDictionaryRef ODRecordCopyDetails( ODRecordRef inRecordRef, CFArrayRef inAttributes, CFErrorRef *outError )
{
    CFMutableSetRef         cfAttribsNeeded = NULL;
    CFMutableSetRef         cfFinalAttribs  = NULL;
    bool                 bNeedAll        = false;
    CFMutableDictionaryRef  cfReturn        = NULL;
    _ODRecord               *pODRecRef      = (_ODRecord *) inRecordRef;
    
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inRecordRef )
    {
        if( NULL != outError && NULL == (*outError) )
        {
			CFStringRef cfDescription = CFSTR("Unable to obtain record details");
			CFStringRef cfError = CFSTR("Invalid record reference.");
            _ODErrorSet( outError, kODErrorDomainFramework, eDSNullParameter, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        return NULL;
    }
    
    // if the _cfRecord is missing, means we've never pulled all the attributes.
    _ODRecordLock( pODRecRef );
    
    // now let's compare what we fetched and what they want..
    if( NULL != inAttributes )
    {
        CFMutableSetRef cfTempFinal     = CFSetCreateMutableCopy( kCFAllocatorDefault, 0, pODRecRef->_cfFetchedAttributes );
        CFSetRef        cfAttributeSet  = _attributeListToSet( inAttributes );
        CFIndex         iCount          = CFSetGetCount( cfAttributeSet );
        void            **cfList        = (void *) calloc( iCount, sizeof(void **) );
        CFIndex         ii;
        
        bNeedAll = CFSetContainsValue( cfAttributeSet, kODAttributeTypeAllAttributes );
        
        CFSetGetValues( cfAttributeSet, (const void **) cfList );
        
        cfAttribsNeeded = CFSetCreateMutable( kCFAllocatorDefault, 0, &kCFTypeSetCallBacks );
        
        for( ii = 0; ii < iCount; ii ++ )
        {
            if( _wasAttributeFetched(pODRecRef, cfList[ii]) == false )
            {
                CFSetAddValue( cfAttribsNeeded, cfList[ii] );
                CFSetAddValue( cfTempFinal, cfList[ii] );
            }
        }
        
        cfFinalAttribs = _minimizeAttributeSet( cfTempFinal );
        
        free( cfList );
        
        CFRelease( cfAttributeSet );
        cfAttributeSet = NULL;
        
        CFRelease( cfTempFinal );
        cfTempFinal = NULL;
        
        cfReturn = CFDictionaryCreateMutable( kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
    }
    else
    {
        // cast is okay here as we aren't doing anything with it
        cfReturn = (CFMutableDictionaryRef) CFDictionaryCreateCopy( kCFAllocatorDefault, pODRecRef->_cfAttributes );
    }
    
    if( NULL != cfAttribsNeeded )
    {
        if( 0 != CFSetGetCount(cfAttribsNeeded) )
        {
            // we switch to the node of the record to ensure we grab correct attribute in case of conflicts
            _VerifyNodeTypeForChange( inRecordRef, outError );
            
            // cast is okay here because we actually allow a set or an array to ODNodeCopyRecord
            ODRecordRef cfNewRecord = _ODNodeCopyRecord( (ODNodeRef) pODRecRef->_ODNode, pODRecRef->_cfRecordType, pODRecRef->_cfRecordName, 
                                                         (CFArrayRef) cfAttribsNeeded, outError );
            
            // now merge the results with what we have and what we need to return
            if( NULL != cfNewRecord )
            {
                _ODRecord       *pNewRec    = (_ODRecord *) cfNewRecord;
                CFIndex         iCount      = CFDictionaryGetCount( pNewRec->_cfAttributes );
                CFTypeRef       *cfKeys     = (CFTypeRef *) calloc( sizeof(CFTypeRef), iCount );
                CFTypeRef       *cfValues   = (CFTypeRef *) calloc( sizeof(CFTypeRef), iCount );
                CFIndex         ii;
                
                CFDictionaryGetKeysAndValues( pNewRec->_cfAttributes, cfKeys, cfValues );
                
                for( ii = 0; ii < iCount; ii++ )
                {
                    CFDictionarySetValue( pODRecRef->_cfAttributes, cfKeys[ii], cfValues[ii] );
                }
                
                // since we fetched something, we need to adjust our fetched attributes
                CFRelease( pODRecRef->_cfFetchedAttributes );
                pODRecRef->_cfFetchedAttributes = cfFinalAttribs;
                cfFinalAttribs = NULL;
                
                free( cfKeys );
                cfKeys = NULL;
                
                free( cfValues );
                cfValues = NULL;
                
                CFRelease( cfNewRecord );
                cfNewRecord = NULL;
            }
        }
        
        CFRelease( cfAttribsNeeded );
        cfAttribsNeeded = NULL;
        
        // now build the return dictionary
        CFIndex         iKeys       = CFDictionaryGetCount( pODRecRef->_cfAttributes );
        CFTypeRef       *cfKeys     = (CFTypeRef *) calloc( sizeof(CFTypeRef), iKeys );
        CFTypeRef       *cfValues   = (CFTypeRef *) calloc( sizeof(CFTypeRef), iKeys );
        CFRange         cfRange     = CFRangeMake( 0, CFArrayGetCount(inAttributes) );
        bool         bStandard   = CFArrayContainsValue( inAttributes, cfRange, kODAttributeTypeStandardOnly );
        bool         bNative     = CFArrayContainsValue( inAttributes, cfRange, kODAttributeTypeNativeOnly );
        CFIndex         ii;
        
        CFDictionaryGetKeysAndValues( pODRecRef->_cfAttributes, cfKeys, cfValues );
        
        for( ii = 0; ii < iKeys; ii++ )
        {
            CFStringRef cfKey = cfKeys[ii];
            
            if( true == bNeedAll ||
                (true == bStandard && true == CFStringHasPrefix(cfKey, CFSTR(kDSStdAttrTypePrefix))) ||
                (true == bNative && true == CFStringHasPrefix(cfKey, CFSTR(kDSNativeAttrTypePrefix))) ||
                (true == CFArrayContainsValue(inAttributes, cfRange, cfKey)) )
            {
                // make a copy of the array before we add it to the dictionary to return
                CFTypeRef cfValuesCopy = CFArrayCreateCopy( kCFAllocatorDefault, cfValues[ii] );
                
                CFDictionarySetValue( cfReturn, cfKey, cfValuesCopy );
                
                CFRelease( cfValuesCopy );
            }
        }
        
        // since we fetched something, we need to adjust our fetched attributes
        free( cfKeys );
        cfKeys = NULL;
        
        free( cfValues );
        cfValues = NULL;
    }
    
    if( NULL != cfFinalAttribs )
    {
        CFRelease( cfFinalAttribs );
        cfFinalAttribs = NULL;
    }
    
    _ODRecordUnlock( pODRecRef );
    
    return cfReturn;
}

bool ODRecordSynchronize( ODRecordRef inRecordRef, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inRecordRef )
    {    
        if( NULL != outError && NULL == (*outError) )
        {
			CFStringRef cfDescription = CFSTR("Unable to synchronize the record.");
			CFStringRef cfError = CFSTR("Invalid record reference.");
            _ODErrorSet( outError, kODErrorDomainFramework, eDSNullParameter, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        return false;
    }
    
    CF_OBJC_FUNCDISPATCH( _kODRecordTypeID, bool, inRecordRef, "synchronizeAndReturnError:", outError );

    _ODRecord   *pODRecord      = (_ODRecord *) inRecordRef;

    _VerifyNodeTypeForChange( inRecordRef, outError );

    // close the record to clean up any references and cause it to be flushed
    _ODRecordLock( pODRecord );
    if( pODRecord->_dsRecordRef != 0 )
    {
        dsCloseRecord( pODRecord->_dsRecordRef );
        pODRecord->_dsRecordRef = 0;
    }
    _ODRecordUnlock( pODRecord );

    // cast is okay because we actually allow both Sets and Arrays, but we don't publicize
    ODRecordRef cfNewRecord     = _ODNodeCopyRecord( (ODNodeRef) pODRecord->_ODNode, pODRecord->_cfRecordType, pODRecord->_cfRecordName, 
                                                     (CFArrayRef) pODRecord->_cfFetchedAttributes, outError );
    
    if( NULL != cfNewRecord )
    {
        _ODRecord   *pODNewRecord   = (_ODRecord *) cfNewRecord;
        
        _ODRecordLock( pODRecord );
        
        CFRelease( pODRecord->_cfAttributes );
        pODRecord->_cfAttributes = (CFMutableDictionaryRef) CFRetain( pODNewRecord->_cfAttributes );
        
        _ODRecordUnlock( pODRecord );
        
        CFRelease( cfNewRecord );
        cfNewRecord = NULL;
    }
    else
    {
        return false;
    }
    
    return true;
}

bool ODRecordDelete( ODRecordRef inRecordRef, CFErrorRef *outError )
{
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    return _ODRecordDelete( inRecordRef, outError );
}

bool _ODRecordDelete( ODRecordRef inRecordRef, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus    = eDSInvalidRecordRef;
    
    if( NULL == inRecordRef )
    {
        cfError = (NULL != outError ? CFSTR("Invalid record reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    
    CF_OBJC_FUNCDISPATCH( _kODRecordTypeID, bool, inRecordRef, "deleteRecordAndReturnError:", outError );
    
    _VerifyNodeTypeForChange( inRecordRef, outError );

    _ODRecord   *pODRecRef  = (_ODRecord *) inRecordRef;
    
    _ODRecordLock( pODRecRef );
    
    do
    {
        dsStatus = dsDeleteRecord( pODRecRef->_dsRecordRef );
        
        if( eDSInvalidReference == dsStatus || eDSInvalidRecordRef == dsStatus || eDSInvalidDirRef == dsStatus || 
            eDSInvalidNodeRef == dsStatus || eDSInvalidRefType == dsStatus )
        {
            dsStatus = _ReopenRecord( pODRecRef ); // returns eDSInvalidRecordRef if it succeeds open
        }
    } while( eDSInvalidReference == dsStatus || eDSInvalidRecordRef == dsStatus || eDSInvalidDirRef == dsStatus || 
             eDSInvalidNodeRef == dsStatus || eDSCannotAccessSession == dsStatus || eDSInvalidRefType == dsStatus );
    
    if( eDSNoErr == dsStatus )
    {
        pODRecRef->_dsRecordRef = 0;
    }
    else if( eDSInvalidRecordRef == dsStatus )
    {
        dsStatus = eDSNoErr;
    }
    
    _ODRecordUnlock( pODRecRef );
    
    cfError = _MapDSErrorToReason( outError, dsStatus );

finish:
        
    if( NULL != cfError )
    {
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfRecordName = (NULL != inRecordRef ? ODRecordGetRecordName(inRecordRef) : NULL);
            
            if( NULL == cfRecordName )
            {
                cfDescription = CFSTR("Unable to delete a record.");
            }
            else
            {
                CFStringRef cfTemp = CFSTR("Unable to delete record %@.");
                cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfRecordName );
                CFRelease( cfTemp );
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
        }
        
        return false;
    }   
    
    return true;
}

enum
{
    kODMemberUseUUID    = 0x00000001,
    kODMemberUseName    = 0x00000002,

    kODMemberNested     = 0x00000010,

    kODMemberUser       = 0x00000100,
    kODMemberComputer   = 0x00000200,
    kODMemberGroup      = 0x00000400,
    kODMemberOtherGroup = 0x00000800
};

static CFStringRef _CopyTempUUIDForRecord( ODRecordRef inRecordRef, uint32_t inFlags )
{
    CFStringRef returnUUID  = NULL;
    CFStringRef (^getUUID)(ODAttributeType, int (*)(uid_t, uuid_t)) = ^(ODAttributeType attrib, int (*mbr_cb)(uid_t, uuid_t))
    {
        CFStringRef cfUUID  = NULL;
        CFArrayRef  idArray = _ODRecordGetValues( inRecordRef, attrib, NULL );
        
        if ( idArray != NULL && CFArrayGetCount(idArray) > 0 )
        {
            CFStringRef cfID = (CFStringRef) CFArrayGetValueAtIndex( idArray, 0 );
            if ( cfID != NULL && CFGetTypeID(cfID) == CFStringGetTypeID() )
            {
                uuid_t  uuid;
                
                if ( mbr_cb(CFStringGetIntValue(cfID), uuid) == 0 )
                {
                    uuid_string_t uuidStr;
                    
                    uuid_unparse_upper( uuid, uuidStr );
                    cfUUID = CFStringCreateWithCString( kCFAllocatorDefault, uuidStr, kCFStringEncodingUTF8 );
                }
            }
        }
        
        return cfUUID;
    };
    
    if ( (inFlags & kODMemberUser) != 0 ) {
        returnUUID = getUUID( kODAttributeTypeUniqueID, mbr_uid_to_uuid );
    }
    else if ( (inFlags & kODMemberGroup) != 0 ) {
        returnUUID = getUUID( kODAttributeTypePrimaryGroupID, mbr_gid_to_uuid );
    }
    
    return returnUUID;
}

static uint32_t __ODRecordMapTypes( ODRecordRef inGroupRef, ODRecordRef inMemberRef )
{
    CFStringRef cfGroupType     = ODRecordGetRecordType( inGroupRef );
    CFStringRef cfMemberType    = ODRecordGetRecordType( inMemberRef );
    uint32_t    flags           = 0;
    
    if ( cfGroupType == NULL || cfMemberType == NULL ) return 0;
    
    if ( CFStringCompare(cfGroupType, kODRecordTypeGroups, 0) == kCFCompareEqualTo )
    {
        if ( CFStringCompare(cfMemberType, kODRecordTypeUsers, 0) == kCFCompareEqualTo ) {
            flags |= kODMemberUser | kODMemberUseUUID | kODMemberUseName;
        }
        else if ( CFStringCompare(cfMemberType, kODRecordTypeGroups, 0) == kCFCompareEqualTo ) {
            flags |= kODMemberGroup | kODMemberUseUUID | kODMemberNested;
        }
    }
    else if ( CFStringCompare(cfGroupType, kODRecordTypeComputerGroups, 0) == kCFCompareEqualTo )
    {
        if ( CFStringCompare(cfMemberType, kODRecordTypeComputers, 0) == kCFCompareEqualTo ) {
            flags |= kODMemberComputer | kODMemberUseUUID;
        }
        else if ( CFStringCompare(cfMemberType, kODRecordTypeComputerGroups, 0) == kCFCompareEqualTo ) {
            flags |= kODMemberOtherGroup | kODMemberUseUUID | kODMemberNested;
        }
    }
    else if ( CFStringCompare(cfGroupType, kODRecordTypeComputerLists, 0) == kCFCompareEqualTo )
    {
        if ( CFStringCompare(cfMemberType, kODRecordTypeComputers, 0) == kCFCompareEqualTo ) {
            flags |= kODMemberComputer | kODMemberUseName;
        }
        else if ( CFStringCompare(cfMemberType, kODRecordTypeComputerLists, 0) == kCFCompareEqualTo ) {
            flags |= kODMemberOtherGroup | kODMemberUseName | kODMemberNested;
        }
    }
    
    return flags;
}

static CFErrorRef __MembershipApplyBlock( ODRecordRef inGroupRef, ODRecordRef inMemberRef, uint32_t inFlags, 
                                          CFErrorRef (^block)(ODAttributeType attr, CFTypeRef value) )
{
    CFErrorRef  cfError = NULL;
    bool        bValid  = false;    // is a valid flag combination
    
    // need UUID for these ops
    if ( (inFlags & kODMemberUseUUID) != 0 )
    {
        CFArrayRef  cfMemberUUIDList    = _ODRecordGetValues( inMemberRef, kODAttributeTypeGUID, NULL );
        CFStringRef cfMemberUUID        = NULL;
        CFStringRef cfTempUUID          = NULL;
        
        if ( NULL != cfMemberUUIDList && 0 != CFArrayGetCount(cfMemberUUIDList) ) {
            cfMemberUUID = (CFStringRef) CFArrayGetValueAtIndex( cfMemberUUIDList, 0 );
        }
        
        if ( NULL == cfMemberUUID ) {
            cfMemberUUID = cfTempUUID = _CopyTempUUIDForRecord( inMemberRef, inFlags );
        }
        
        // if we have a UUID
        if ( cfMemberUUID != NULL )
        {
            if ( (inFlags & (kODMemberComputer | kODMemberUser)) != 0 ) {
                cfError = block( kODAttributeTypeGroupMembers, cfMemberUUID );
                bValid = true;
            }
            else if ( (inFlags & kODMemberNested) != 0 ) {
                cfError = block( kODAttributeTypeNestedGroups, cfMemberUUID );
                bValid = true;
            }
            
            // clear the error if attribute type is not supported
            if ( cfError != NULL && CFErrorGetCode(cfError) == kODErrorRecordAttributeUnknownType ) {
                CFRelease( cfError );
                cfError = NULL;
                bValid = false;
            }
        }
        else if ( (inFlags & kODMemberNested) != 0 )
        {
			CFStringRef cfDescription = CFSTR("Could not determine a UUID of the member record to add membership.");
			CFStringRef cfErrorStr = _MapDSErrorToReason(&cfError, kODErrorRecordAttributeNotFound);
            _ODErrorSet( &cfError,
                         kODErrorDomainFramework, 
                         kODErrorRecordAttributeNotFound, 
                         &cfDescription, 
                         &cfErrorStr,
                         NULL );
        }
        
        if ( cfTempUUID != NULL ) {
            CFRelease( cfTempUUID );
            cfTempUUID = NULL;
        }
    }
    
    if ( cfError == NULL && (inFlags & kODMemberUseName) != 0 )
    {
        CFStringRef  cfMemberName = ODRecordGetRecordName( inMemberRef );
        if ( cfMemberName != NULL )
        {
            if ( (inFlags & kODMemberUser) != 0 ) {
                cfError = block( kODAttributeTypeGroupMembership, cfMemberName );
                bValid = true;
            }
            else if ( (inFlags & kODMemberComputer) != 0 ) {
                cfError = block( kODAttributeTypeComputers, cfMemberName );
                bValid = true;
            }
            else if ( (inFlags & kODMemberNested) != 0 ) {
                cfError = block( kODAttributeTypeGroup, cfMemberName );
                bValid = true;
            }
            
            // clear the error and use the consistent one below if it is no supported attribute
            if ( cfError != NULL && CFErrorGetCode(cfError) == kODErrorRecordAttributeUnknownType ) {
                CFRelease( cfError );
                cfError = NULL;
                bValid = false;
            }            
        }
        else
        {
			CFStringRef cfDescription = CFSTR("Could not determine name of the member record to add membership.");
			CFStringRef cfErrorStr = _MapDSErrorToReason(&cfError, kODErrorRecordAttributeNotFound);
            _ODErrorSet( &cfError,
                         kODErrorDomainFramework, 
                         kODErrorRecordAttributeNotFound, 
                         &cfDescription, 
                         &cfErrorStr,
                         NULL );
        }
    }

    // Create an error case if the value failed to add to any known types
    if ( cfError == NULL && bValid == false ) {
		CFStringRef cfDescription = CFSTR("Could not modify group because it does not support required membership attributes.");
		CFStringRef cfErrorStr = _MapDSErrorToReason(&cfError, kODErrorRecordAttributeUnknownType);
        _ODErrorSet( &cfError,
                    kODErrorDomainFramework, 
                    kODErrorRecordAttributeUnknownType, 
                    &cfDescription, 
                    &cfErrorStr,
                    NULL );
    }        
    
    return cfError;
}

bool ODRecordAddMember( ODRecordRef inGroupRef, ODRecordRef inMemberRef, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus    = eDSOperationFailed;
    bool        bSuccess    = false;
    
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inGroupRef )
    {
        cfError = (NULL != outError ? CFSTR("Invalid record reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inMemberRef )
    {
        cfError = (NULL != outError ? CFSTR("Invalid member record reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }

    CF_OBJC_FUNCDISPATCH( _kODRecordTypeID, bool, inGroupRef, "addMemberRecord:error:", inMemberRef, outError );
    
    _VerifyNodeTypeForChange( inGroupRef, outError );
    
    if ( outError == NULL || (*outError) == NULL )
    {
        uint32_t flags = __ODRecordMapTypes( inGroupRef, inMemberRef );
        
        // groups can be hybrid so check hybrid status before attempting to add values to user list
        if ( (flags & (kODMemberUseUUID | kODMemberUseName)) != 0 && (flags & kODMemberUser) != 0 ) {
            CFArrayRef  names   = _ODRecordGetValues( inGroupRef, kODAttributeTypeGroupMembership, NULL );
            CFArrayRef  uuids   = _ODRecordGetValues( inGroupRef, kODAttributeTypeGroupMembers, NULL );
            
            // if we don't have any UUIDs but we have names, don't add UUIDs
            if ( uuids == 0 && names > 0 ) {
                // if this is a nested group attempt, it will fail because the group is legacy format still
                flags &= ~kODMemberUseUUID;
            }
            
            // if not hybrid, don't add the name either
            if ( uuids > 0 && names == 0 ) {
                flags &= ~kODMemberUseName;
            }
        }
        
        if ( (flags & (kODMemberUseUUID | kODMemberUseName)) != 0 )
        {
            CFErrorRef cfTempError = __MembershipApplyBlock( inGroupRef,
                                                             inMemberRef, 
                                                             flags,
                                                             ^(ODAttributeType attr, CFTypeRef value)
                                                             {
                                                                 CFErrorRef cfLocalError  = NULL;
                                                                 CFArrayRef cfValues = (CFArrayRef) _ODRecordGetValues( inGroupRef, attr, NULL );
                                               
                                                                 if ( cfValues == NULL || 
                                                                      CFArrayContainsValue(cfValues, CFRangeMake(0, CFArrayGetCount(cfValues)), value) == false ) 
                                                                 {
                                                                     _ODRecordAddValue( inGroupRef, attr, value, &cfLocalError );
                                                                 }
                                               
                                                                 return cfLocalError;
                                                             } );
            
            if ( cfTempError == NULL ) {
                bSuccess = true;
            }
            else {
                if ( outError != NULL ) {
                    (*outError) = cfTempError;
                }
                else if ( cfTempError != NULL ) {
                    CFRelease( cfTempError );
                    cfTempError = NULL;
                }
            }
        }
        else
        {
            dsStatus = eDSInvalidRecordType;
            cfError = _MapDSErrorToReason( outError, eDSInvalidRecordType );
        }
    }
    
finish:
    
    if ( NULL != cfError )
    {
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfGroupName = (NULL != inGroupRef ? ODRecordGetRecordName(inGroupRef) : NULL);
            CFStringRef cfMemberName = (NULL != inMemberRef ? ODRecordGetRecordName(inMemberRef) : NULL);
            
            if( NULL == cfGroupName )
            {
                if( NULL != cfMemberName )
                {
                    CFStringRef cfTemp = CFSTR("Unable to add %@ to the group.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfMemberName );
                    CFRelease( cfTemp );
                }
                else
                {                    
                    cfDescription = CFSTR("Unable to add a member to the group.");
                }
            }
            else
            {
                if( NULL != cfMemberName )
                {
                    CFStringRef cfTemp = CFSTR("Unable to add %@ to the group %@.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfMemberName, cfGroupName );
                    CFRelease( cfTemp );
                }
                else
                {                    
                    CFStringRef cfTemp = CFSTR("Unable to add member to the group %@.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfGroupName );
                    CFRelease( cfTemp );
                }
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
            cfError = NULL;
        }
        
        bSuccess = false;
    }
    
    return bSuccess;
}

bool ODRecordRemoveMember( ODRecordRef inGroupRef, ODRecordRef inMemberRef, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus    = eDSInvalidRecordType;
    bool        bSuccess    = false;
    
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inGroupRef )
    {
        cfError = (NULL != outError ? CFSTR("Invalid record reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inMemberRef )
    {
        cfError = (NULL != outError ? CFSTR("Invalid member record reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    
    CF_OBJC_FUNCDISPATCH( _kODRecordTypeID, bool, inGroupRef, "removeRecordMember:error:", inMemberRef, outError );
    
    _VerifyNodeTypeForChange( inGroupRef, outError );
        
    if ( outError == NULL || (*outError) == NULL )
    {
        uint32_t flags = __ODRecordMapTypes( inGroupRef, inMemberRef );
        if ( (flags & (kODMemberUseUUID | kODMemberUseName)) != 0 )
        {
            CFErrorRef cfTempError = __MembershipApplyBlock( inGroupRef,
                                                             inMemberRef, 
                                                             flags,
                                                             ^(ODAttributeType attr, CFTypeRef value)
                                                             {
                                                                 CFErrorRef cfLocalError = NULL;
                                                                 bool bTemp = ODRecordRemoveValue( inGroupRef, attr, value, &cfLocalError );
                                                                 if ( bTemp == false ) {
                                                                     CFIndex code = CFErrorGetCode( cfLocalError );
                                                                     if ( kODErrorRecordAttributeValueNotFound == code || 
                                                                          kODErrorRecordAttributeNotFound == code )
                                                                     {
                                                                         CFRelease( cfLocalError );
                                                                         cfLocalError = NULL;
                                                                     }
                                                                 }
                                                                 
                                                                 return cfLocalError;
                                                             } );
            if ( cfTempError == NULL ) {
                bSuccess = true;
            }
            else {
                if ( outError != NULL ) {
                    (*outError) = cfTempError;
                }
                else if ( cfTempError != NULL ) {
                    CFRelease( cfTempError );
                    cfTempError = NULL;
                }
            }
        }
        else
        {
            dsStatus = eDSInvalidRecordType;
            cfError = _MapDSErrorToReason( outError, eDSInvalidRecordType );
        }
    }
    
finish:
    
    if ( NULL != cfError )
    {
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfGroupName = (NULL != inGroupRef ? ODRecordGetRecordName(inGroupRef) : NULL);
            CFStringRef cfMemberName = (NULL != inMemberRef ? ODRecordGetRecordName(inMemberRef) : NULL);
            
            if( NULL == cfGroupName )
            {
                if( NULL != cfMemberName )
                {
                    CFStringRef cfTemp = CFSTR("Unable to remove %@ from the group.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfMemberName );
                    CFRelease( cfTemp );
                }
                else
                {                    
                    cfDescription = CFSTR("Unable to remove a member from the group.");
                }
            }
            else
            {
                if( NULL != cfMemberName )
                {
                    CFStringRef cfTemp = CFSTR("Unable to remove %@ from group %@.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfMemberName, cfGroupName );
                    CFRelease( cfTemp );
                }
                else
                {                    
                    CFStringRef cfTemp = CFSTR("Unable to remove member from group %@.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfGroupName );
                    CFRelease( cfTemp );
                }
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
            cfError = NULL;
        }
        
        bSuccess = false;
    }
    
    return bSuccess;
}

static bool _ODRecordContainsMember( ODRecordRef inGroupRef, ODRecordRef inMemberRef, bool inRefresh, CFErrorRef *outError )
{
    CFStringRef cfError     = NULL;
    tDirStatus  dsStatus    = eDSNoErr;
    int         isMember    = 0;
    
    if( NULL != outError )
    {
        (*outError) = NULL;
    }
        
    if( NULL == inGroupRef )
    {
        cfError = (NULL != outError ? CFSTR("Invalid record reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    else if( NULL == inMemberRef )
    {
        cfError = (NULL != outError ? CFSTR("Invalid member record reference.") : CFSTR(""));
        dsStatus = eDSNullParameter;
        goto finish;
    }
    
    CF_OBJC_FUNCDISPATCH( _kODRecordTypeID, bool, inGroupRef, "isMemberRecord:error:", inMemberRef, outError );

    // TODO: make it work over proxy using the remote daemon (probably a custom call)
    if ( outError == NULL || (*outError) == NULL )
    {
        uint32_t flags = __ODRecordMapTypes( inGroupRef, inMemberRef );
        if ( (flags & kODMemberNested) == 0 )
        {
            uuid_t uuid_group;
            uuid_t uuid_member;
            
            bool (^getUUID)(ODRecordRef, uuid_t) = ^(ODRecordRef inRecord, uuid_t inUUID ) 
            {
                CFArrayRef  cfRecordUUIDList    = _ODRecordGetValues( inRecord, kODAttributeTypeGUID, NULL );
                CFStringRef cfRecordUUID        = NULL;
                CFStringRef cfTempUUID          = NULL;
                bool        bFound              = false;
                
                if ( NULL != cfRecordUUIDList && 0 != CFArrayGetCount(cfRecordUUIDList) ) {
                    cfRecordUUID = (CFStringRef) CFArrayGetValueAtIndex( cfRecordUUIDList, 0 );
                }
                
                if ( NULL == cfRecordUUID ) {
                    cfRecordUUID = cfTempUUID = _CopyTempUUIDForRecord( inMemberRef, flags );
                }
                
                // if we have a UUID
                if ( cfRecordUUID != NULL )
                {
                    char *pMemberUUID = _GetCStringFromCFString( cfRecordUUID );
                    if ( NULL != pMemberUUID )
                    {
                        if ( mbr_string_to_uuid(pMemberUUID, inUUID) == 0 ) {
                            bFound = true;
                        }
                        
                        free( pMemberUUID );
                        pMemberUUID = NULL;
                    }
                }
                
                if ( cfTempUUID != NULL ) {
                    CFRelease( cfTempUUID );
                    cfTempUUID = NULL;
                }
                
                return bFound;
            };
            
            // should always have a UUID
            if ( getUUID(inGroupRef, uuid_group) == true && getUUID(inMemberRef, uuid_member) == true ) {
                if ( inRefresh == false ) {
                    mbr_check_membership( uuid_member, uuid_group, &isMember );
                }
                else {
                    mbr_check_membership_refresh( uuid_member, uuid_group, &isMember );
                }
            }
        }
        else
        {
            dsStatus = eDSInvalidRecordType;
            cfError = _MapDSErrorToReason( outError, eDSInvalidRecordType );
        }        
    }
    
finish:
        
    if( NULL != cfError )
    {
        if( NULL != outError && NULL == (*outError) )
        {
            CFStringRef cfDescription;
            CFStringRef cfGroupName = (NULL != inGroupRef ? ODRecordGetRecordName(inGroupRef) : NULL);
            CFStringRef cfMemberName = (NULL != inMemberRef ? ODRecordGetRecordName(inMemberRef) : NULL);
            
            if( NULL == cfGroupName )
            {
                if( NULL != cfMemberName )
                {
                    CFStringRef cfTemp = CFSTR("Unable to determine if %@ is a member of the group.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfMemberName );
                    CFRelease( cfTemp );
                }
                else
                {                    
                    cfDescription = CFSTR("Unable to determine if this user is a member of group.");
                }
            }
            else
            {
                if( NULL != cfMemberName )
                {
                    CFStringRef cfTemp = CFSTR("Unable to determine if %@ is a member of group %@.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfMemberName, cfGroupName );
                    CFRelease( cfTemp );
                }
                else
                {                    
                    CFStringRef cfTemp = CFSTR("Unable to determine if this user is a member of group %@.");
                    cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, cfGroupName );
                    CFRelease( cfTemp );
                }
            }
            
            _ODErrorSet( outError, kODErrorDomainFramework, dsStatus, 
                         &cfDescription, 
                         &cfError,
                         NULL );
        }
        else
        {
            CFRelease( cfError );
            cfError = NULL;
        }
    }    
    
    return (isMember == 1 ? true : false);
}

bool ODRecordContainsMember( ODRecordRef inGroupRef, ODRecordRef inMemberRef, CFErrorRef *outError )
{
    return _ODRecordContainsMember( inGroupRef, inMemberRef, false, outError );
}

bool ODRecordContainsMemberRefresh( ODRecordRef inGroupRef, ODRecordRef inMemberRef, CFErrorRef *outError )
{
    return _ODRecordContainsMember( inGroupRef, inMemberRef, true, outError );
}

tRecordReference ODRecordGetDSRef( ODRecordRef inRecordRef )
{
    _ODRecord   *pODRecord  = (_ODRecord *) inRecordRef;
    
    if( NULL == inRecordRef )
    {
        return 0;
    }

    if( pODRecord->_dsRecordRef == 0 )
    {
        _ReopenRecord( pODRecord );
    }
    
    return pODRecord->_dsRecordRef;
}

#pragma mark -
#pragma mark Internal functions

CFDictionaryRef _ODRecordGetDictionary( ODRecordRef inRecordRef )
{
    _ODRecord   *pODRecord  = (_ODRecord *) inRecordRef;

    if( NULL == inRecordRef )
        return NULL;
    
    return pODRecord->_cfAttributes;
}

bool _VerifyNodeTypeForChange( ODRecordRef inRecord, CFErrorRef *outError )
{
    _ODRecord   *pODRecord  = (_ODRecord *) inRecord;
    bool bReturn = true;
    // just need to lock the record, don't care about nodes below....
    pthread_mutex_lock( &(pODRecord->_mutex) ); // Now lock the record
    
    ODNodeType nodeType = pODRecord->_ODNode->_nodeType;
    
    // if we have a type, we need to open the actual node for this record
    if( 0 != nodeType && kODNodeTypeLocalNodes != nodeType && kODNodeTypeConfigure != nodeType )
    {
        CFArrayRef cfNodeLocation = (CFArrayRef) CFDictionaryGetValue( pODRecord->_cfAttributes, kODAttributeTypeMetaNodeLocation );
        
        if( NULL != cfNodeLocation )
        {
            CFStringRef cfNodeName = CFArrayGetValueAtIndex( cfNodeLocation, 0 );
            
            if( NULL != cfNodeName )
            {
                ODNodeRef odNode = _ODNodeCreateWithName( kCFAllocatorDefault, (ODSessionRef) pODRecord->_ODNode->_ODSession, cfNodeName, outError );
                
                if( NULL != odNode )
                {
                    CFRelease( pODRecord->_ODNode );
                    pODRecord->_ODNode = (_ODNode *)odNode;
                }
            }
        }
        else
        {
            // this should never happen...
            if( NULL != outError && NULL == (*outError) )
            {
				CFStringRef cfDescription = CFSTR("Cannot determine the location of node to verify type.");
				CFStringRef cfError = CFSTR("Node name is empty.");
                _ODErrorSet( outError, kODErrorDomainFramework, eNotHandledByThisNode, 
                             &cfDescription, 
                             &cfError,
                             NULL );
            }
			bReturn = false;
        }
    }
    
    pthread_mutex_unlock( &(pODRecord->_mutex) ); // Now unlock the record
	return bReturn;
}

char *_GetCStringFromCFString( CFStringRef cfString )
{
    char    *pReturn    = NULL;
    
    if( NULL != cfString )
    {
        CFIndex iBufferSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfString), kCFStringEncodingUTF8) + 1;
        
        pReturn = malloc( iBufferSize );
        CFStringGetCString( cfString, pReturn, iBufferSize, kCFStringEncodingUTF8 );
    }
    
    return pReturn;
}

tDataBufferPtr  _GetDataBufferFromCFType( CFTypeRef inRef )
{
    tDataBufferPtr  dsDataBuffer    = NULL;

    if( NULL != inRef )
    {
        if( CFStringGetTypeID() == CFGetTypeID(inRef) )
        {
            CFIndex iBufferSize     = CFStringGetMaximumSizeForEncoding(CFStringGetLength(inRef), kCFStringEncodingUTF8) + 1;
            char    *pTempString    = malloc( iBufferSize );
            
            if( NULL != pTempString )
            {
                CFStringGetCString( inRef, pTempString, iBufferSize, kCFStringEncodingUTF8 );
                
                dsDataBuffer = dsDataNodeAllocateString( 0, pTempString );
                
                free( pTempString );
                pTempString = NULL;
            }
        }
        else if( CFDataGetTypeID() == CFGetTypeID(inRef) )
        {
            uint32_t    uiLength = CFDataGetLength( inRef );
            dsDataBuffer = dsDataNodeAllocateBlock( 0, uiLength, uiLength, (tBuffer) CFDataGetBytePtr(inRef) );
        }
    }
    
    return dsDataBuffer;
}

tDirStatus _ReopenDS( _ODSession *inSession )
{
    tDirStatus      dsStatus    = eDSNoErr;
    CFStringRef     cfHostname  = (CFStringRef) CFDictionaryGetValue( inSession->_info, kODSessionProxyAddress );
    CFStringRef     cfLocalPath = (CFStringRef) CFDictionaryGetValue( inSession->_info, kODSessionLocalPath );
    
    if( 0 != inSession->_dsRef )
    {
        dsCloseDirService( inSession->_dsRef );
        inSession->_dsRef = 0;
    }
    
    if( NULL != cfLocalPath )
    {
        char    *pFilePath = _GetCStringFromCFString( cfLocalPath );
        
        if( NULL != pFilePath )
        {
            dsStatus = dsOpenDirServiceLocal( &inSession->_dsRef, pFilePath );
            free( pFilePath );
            pFilePath = NULL;
        }
        else
        {
            dsStatus = eDSInvalidName;
        }
    }
    else if( NULL == cfHostname )
    {
        dsStatus = dsOpenDirService( &inSession->_dsRef );
    }
    else
    {
        SInt32          curr            = 0;
        tDataBuffer     *authBuff       = NULL;
        tDataBuffer     *stepBuff       = NULL;
        tDataNode       *authType       = NULL;
        CFStringRef     cfUsername      = (CFStringRef) CFDictionaryGetValue( inSession->_info, kODSessionProxyUsername );
        CFNumberRef     cfPortNumber    = (CFNumberRef) CFDictionaryGetValue( inSession->_info, kODSessionProxyPort );
        CFStringRef     cfPassword      = inSession->_cfProxyPassword;
        
        if( eDSNoErr == dsStatus )
        {
            authBuff = dsDataBufferAllocate( 0, 512 );
            if( NULL == authBuff )
            {
                dsStatus = eDSAllocationFailed;
            }
        }
        
        if( eDSNoErr == dsStatus )
        {
            stepBuff = dsDataBufferAllocate( 0, 128 );
            if( NULL == authBuff )
            {
                dsStatus = eDSAllocationFailed;
            }
        }
        
        if( eDSNoErr == dsStatus )
        {
            authType = dsDataNodeAllocateString( 0, kDSStdAuthNodeNativeClearTextOK );
            if( NULL == authBuff )
            {
                dsStatus = eDSAllocationFailed; 
            }
        }
        
        if( eDSNoErr == dsStatus )
        {
            char    *pUsername          = _GetCStringFromCFString( cfUsername );
            char    *pPassword          = _GetCStringFromCFString( cfPassword );
            char    *pProxyHostAddress  = _GetCStringFromCFString( cfHostname );
            CFIndex portNumber          = 625;
            
            if( cfPortNumber )
            {
                CFNumberGetValue( cfPortNumber, kCFNumberCFIndexType, &portNumber );
            }
            
            // User Name
            uint32_t len = strlen( pUsername );
            bcopy( &len, &(authBuff->fBufferData[curr]), sizeof(uint32_t) );
            curr += sizeof( uint32_t );
            if( len != 0 )
            {
                bcopy( pUsername, &(authBuff->fBufferData[curr]), len );
                curr += len;
            }
            
            // Password
            len = strlen( pPassword );
            bcopy( &len, &(authBuff->fBufferData[curr]), sizeof(uint32_t) );
            curr += sizeof (uint32_t );
            
            if( len != 0 )
            {
                bcopy( pPassword, &(authBuff->fBufferData[curr]), len );
                curr += len;
            }
            authBuff->fBufferLength = curr;
            
            dsStatus = dsOpenDirServiceProxy( &inSession->_dsRef, pProxyHostAddress, portNumber,
                                              authType, authBuff, stepBuff, NULL );
            
            if( NULL != pProxyHostAddress )
            {
                free( pProxyHostAddress );
                pProxyHostAddress = NULL;
            }
            
            if( NULL != pPassword )
            {
                bzero( pPassword, strlen(pPassword) );
                free( pPassword );
                pPassword = NULL;
            }
            
            if( NULL != pUsername )
            {
                free( pUsername );
                pUsername = NULL;
            }
        }
        
        if( NULL != authType )
        {
            dsDataBufferDeAllocate( 0, authType );
            authType = NULL;
        }
                
        if( NULL != stepBuff )
        {
            dsDataBufferDeAllocate( 0, stepBuff );
            stepBuff = NULL;
        }
        
        if( NULL != authBuff )
        {
            dsDataBufferDeAllocate( 0, authBuff );
            authBuff = NULL;
        }
    }
    
    return dsStatus;
}

tDirStatus _ReopenNode( _ODNode *inNode )
{
    tDirStatus      dsStatus    = eDSOpenNodeFailed;
    
    if( 0 != inNode->_dsNodeRef )
    {
        inNode->_flags |= kODNodeFlagCloseRef;
        dsCloseDirNode( inNode->_dsNodeRef );
        inNode->_dsNodeRef = 0;
    }
    
    CFStringRef cfNodeName = (CFStringRef) CFDictionaryGetValue( inNode->_info, kODNodeNameKey );
    if( NULL != cfNodeName )
    {
        char            *pNodeName = _GetCStringFromCFString( cfNodeName );
        tDataListPtr    dsNodeName = dsBuildFromPath( 0, pNodeName, "/" );
        
        do
        {
            dsStatus = dsOpenDirNode( inNode->_ODSession->_dsRef, dsNodeName, &inNode->_dsNodeRef );
            if( eDSInvalidDirRef == dsStatus || eDSInvalidReference == dsStatus || eDSInvalidRefType == dsStatus )
            {
                // if we can't re-open DS, then we need to bail on the while
                if( _ReopenDS(inNode->_ODSession) != eDSNoErr )
                {
                    break;
                }
            }
        } while( eDSInvalidDirRef == dsStatus || eDSInvalidReference == dsStatus || eDSInvalidRefType == dsStatus );
        
        if( eDSNoErr == dsStatus && NULL != inNode->_cfNodePassword )
        {
            CFTypeRef   values[]    = { CFDictionaryGetValue(inNode->_info, kODNodeUsername), inNode->_cfNodePassword, NULL };
            CFArrayRef  cfAuthItems = CFArrayCreate( kCFAllocatorDefault, values, 2, &kCFTypeArrayCallBacks );
            
            dsStatus = _Authenticate( (ODNodeRef) inNode, kDSStdAuthNodeNativeClearTextOK, NULL, cfAuthItems, NULL, NULL, false );
            
            CFRelease( cfAuthItems );
            cfAuthItems = NULL;
        }
        
        if( NULL != pNodeName )
        {
            free( pNodeName );
            pNodeName = NULL;
        }
        
        dsDataListDeallocate( 0, dsNodeName );
        
        free( dsNodeName );
        dsNodeName = NULL;            
    }
    
    return dsStatus;
}

tDirStatus _ReopenRecord( _ODRecord *inRecord )
{
    tDirStatus      dsStatus        = eDSInvalidRecordRef;
    tDataNodePtr    dsRecordType    = _GetDataBufferFromCFType( inRecord->_cfRecordType );
    tDataNodePtr    dsRecordName    = _GetDataBufferFromCFType( inRecord->_cfRecordName );
    
    if( inRecord->_dsRecordRef )
    {
        dsCloseRecord( inRecord->_dsRecordRef );
        inRecord->_dsRecordRef = 0;
    }

    do
    {
        dsStatus = dsOpenRecord( inRecord->_ODNode->_dsNodeRef, dsRecordType, dsRecordName, &(inRecord->_dsRecordRef) );
        if( eDSInvalidNodeRef == dsStatus || eDSInvalidReference == dsStatus || eDSInvalidRefType == dsStatus )
        {
            dsStatus = _ReopenNode( inRecord->_ODNode );
            if( eDSNoErr == dsStatus )
            {
                dsStatus = eDSInvalidNodeRef;
            }
        }
    } while( eDSInvalidNodeRef == dsStatus || eDSInvalidReference == dsStatus || eDSInvalidRefType == dsStatus );
    
    if( NULL != dsRecordType )
    {
        dsDataBufferDeAllocate( 0, dsRecordType );
        dsRecordType = NULL;
    }
    
    if( NULL != dsRecordName )
    {
        dsDataBufferDeAllocate( 0, dsRecordName );
        dsRecordName = NULL;
    }
    
    return (eDSNoErr == dsStatus ? eDSInvalidRecordRef : dsStatus);
}

tDirStatus  _FindDirNode( _ODNode *inNode, tDirPatternMatch inNodeMatch, CFErrorRef *outError )
{
    tDirStatus      dsStatus        = eDSNoErr;
    tDataBufferPtr  dsDataBuffer    = NULL;
    tContextData    dsContext       = 0;
    tDataListPtr    dsNodeName      = NULL;
    UInt32          dsReturnCount   = 0;
    
    if( 0 != inNode->_dsNodeRef )
    {
        dsCloseDirNode( inNode->_dsNodeRef );
        inNode->_dsNodeRef = 0;
    }
    
    dsDataBuffer = dsDataBufferAllocate( 0, 1024 );
    
    if( NULL == dsDataBuffer )
        dsStatus = eDSAllocationFailed;
    
    if( eDSNoErr == dsStatus )
    {
        bool bFailedOnce = false;

        do
        {
            dsStatus = dsFindDirNodes( inNode->_ODSession->_dsRef, dsDataBuffer, NULL, inNodeMatch, &dsReturnCount, &dsContext );
            
            if( eDSBufferTooSmall == dsStatus )
            {
                UInt32 newSize = (dsDataBuffer->fBufferSize << 1);
                dsDataBufferDeAllocate( 0, dsDataBuffer );
                dsDataBuffer = dsDataBufferAllocate( 0, newSize );
            }

            if( false == bFailedOnce && (eDSInvalidReference == dsStatus || eDSInvalidDirRef == dsStatus || eDSInvalidRefType == dsStatus) )
            {
				_ReopenDS( inNode->_ODSession );
                bFailedOnce = true;
                dsStatus = eDSBufferTooSmall; // reset error so we try again
            }
        } while( eDSBufferTooSmall == dsStatus );
    }
    
    if( 0 != dsContext )
    {
        dsReleaseContinueData( inNode->_ODSession->_dsRef, dsContext );
        dsContext = 0;
    }
    
    if( eDSNoErr == dsStatus && dsReturnCount > 0 )
    {
        dsStatus = dsGetDirNodeName( inNode->_ODSession->_dsRef, dsDataBuffer, 1, &dsNodeName );
    }
    
    if( eDSNoErr == dsStatus )
    {
        dsStatus = dsOpenDirNode( inNode->_ODSession->_dsRef, dsNodeName, &inNode->_dsNodeRef );
    }
    
    if( NULL != dsDataBuffer )
    {
        dsDataBufferDeAllocate( 0, dsDataBuffer );
        dsDataBuffer = NULL;
    }
    
    if( NULL != dsNodeName )
    {
        char *path = dsGetPathFromList( 0, dsNodeName, "/" );
        
        CFStringRef cfNodeName = CFStringCreateWithCString( kCFAllocatorDefault, path, kCFStringEncodingUTF8 );
        if( cfNodeName )
        {
            CFDictionarySetValue( inNode->_info, kODNodeNameKey, cfNodeName );
            CFRelease( cfNodeName );
        }
        
        free( path );
        path = NULL;
        
        dsDataListDeallocate( 0, dsNodeName );
        
        free( dsNodeName );
        dsNodeName = NULL;
    }
    
    if( eDSNoErr != dsStatus && NULL != outError && NULL == (*outError) )
    {
        CFStringRef cfTemp = CFSTR("Unable to find node of type %d.");
        CFStringRef cfDescription = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, cfTemp, inNodeMatch );
        CFStringRef cfError = _MapDSErrorToReason( outError, dsStatus );
        _ODErrorSet( outError, kODErrorDomainFramework, eDSNullParameter, 
                     &cfDescription, 
                     &cfError,
                     NULL );
        
        CFRelease( cfTemp );
    }
    
    return dsStatus;
}

bool _AppendRecordsToListNonStd( _ODNode *inNode, tDataBufferPtr inDataBuffer, uint32_t inRecCount, CFMutableArrayRef inArrayRef, CFErrorRef *outError )
{
    UInt32  ii  = 0;
    bool bReturn = false;
	
    for (ii = 1; ii <= inRecCount; ii++)
    {
        tRecordEntryPtr     recEntryPtr = NULL;
        tAttributeListRef   attrListRef = 0;
        tDirStatus          dsStatus;
        
        dsStatus = dsGetRecordEntry( inNode->_dsNodeRef, inDataBuffer, ii, &attrListRef, &recEntryPtr );
        if( dsStatus == eDSNoErr)
        {
            char    *recordName = NULL;
            char    *recordType = NULL;
            
            dsGetRecordNameFromEntry( recEntryPtr, &recordName );
            dsGetRecordTypeFromEntry( recEntryPtr, &recordType );
            
            if( NULL != recordName && NULL != recordType )
            {
                CFMutableDictionaryRef cfRecord = _CopyAttributesFromBuffer( inNode->_dsNodeRef, inDataBuffer, attrListRef, 
                                                                            recEntryPtr->fRecordAttributeCount, outError );
                if( NULL != cfRecord )
                {
                    _ODRecord *pRecord = _createRecord( kCFAllocatorDefault );
                    
                    if( NULL != pRecord )
                    {
                        pRecord->_cfAttributes = (CFMutableDictionaryRef) CFRetain( cfRecord );
                        pRecord->_cfRecordName = CFStringCreateWithCString( kCFAllocatorDefault, recordName, kCFStringEncodingUTF8 );
                        pRecord->_cfRecordType = CFStringCreateWithCString( kCFAllocatorDefault, recordType, kCFStringEncodingUTF8 );
                        pRecord->_ODNode = (_ODNode *) CFRetain( (CFTypeRef) inNode );
						
						// ensure record type and name are part of attributes
						if (CFDictionaryContainsKey(pRecord->_cfAttributes, kODAttributeTypeRecordType) == FALSE) {
							CFArrayRef recType = CFArrayCreate(kCFAllocatorDefault, (const void **)&pRecord->_cfRecordType, 1, &kCFTypeArrayCallBacks);
							
							CFDictionarySetValue(pRecord->_cfAttributes, kODAttributeTypeRecordType, recType);
							CFRelease(recType);
						}
						
						if (CFDictionaryContainsKey(pRecord->_cfAttributes, kODAttributeTypeRecordName) == FALSE) {
							CFArrayRef recName = CFArrayCreate(kCFAllocatorDefault, (const void **)&pRecord->_cfRecordName, 1, &kCFTypeArrayCallBacks);
							
							CFDictionarySetValue(pRecord->_cfAttributes, kODAttributeTypeRecordName, recName);
							CFRelease(recName);
						}
                        
                        CFArrayAppendValue( inArrayRef, (CFTypeRef) pRecord );
                    }
                    else
                    {
                        CFRelease( cfRecord );
                        cfRecord = NULL;
                    }
                }
            }
            
            if( NULL != recordName )
            {
                free( recordName );
                recordName = NULL;
            }
            
            if( NULL != recordType )
            {
                free( recordType );
                recordType = NULL;
            }
            
            dsCloseAttributeList( attrListRef );
            attrListRef = 0;
            
            dsDeallocRecordEntry( 0, recEntryPtr );
            recEntryPtr = NULL;
        }
        
        if( eDSNoErr != dsStatus && NULL != outError && NULL == (*outError) )
        {
			CFStringRef cfDescription = CFSTR("Unable to parse the buffer returned by Directory Service.");
			CFStringRef cfError = _MapDSErrorToReason( outError, dsStatus );
            _ODErrorSet( outError, kODErrorDomainFramework, eDSNullParameter, 
                         &cfDescription, 
                         &cfError,
                         NULL );
			bReturn = true;
        }
    }
	return bReturn;
}

bool _AppendRecordsToList( _ODNode *inNode, tDataBufferPtr inDataBuffer, UInt32 inRecCount, CFMutableArrayRef inArrayRef, CFErrorRef *outError )
{
    char            *pBuffer    = inDataBuffer->fBufferData;
    uint32_t        uiLength    = inDataBuffer->fBufferSize;
    char            *pEndBuffer = pBuffer + uiLength;
    uint32_t        uiOffset    = 0;
    uint32_t        bufTag      = 0;
    bool         bStdA       = true;
    CFAllocatorRef  cfAllocator = CFGetAllocator(inArrayRef);
    
    if( uiLength >= sizeof(uint32_t) )
    {
        bufTag = *((uint32_t *)pBuffer);
        pBuffer += sizeof(uint32_t);
        
        if( bufTag == 'StdA' || bufTag == 'StdB' || bufTag == 'DbgA' || bufTag == 'DbgB' )
        {
            if( bufTag == 'StdB' || bufTag == 'DbgB' )
                bStdA = false;
            
            inRecCount = *((uint32_t *)pBuffer);
            pBuffer += sizeof(uint32_t);
            
            if( 0 != inRecCount )
            {
                uint32_t    ii;
                char        *pRecEntry  = NULL;
                
                for( ii = 0; ii < inRecCount && *((uint32_t *)pBuffer) != 'EndT'; ii++ )
                {
                    uiOffset = ((uint32_t *) pBuffer)[ ii ];
                    
                    if( uiOffset < uiLength )
                    {
                        uint32_t    uiRecLength;
                        
                        pRecEntry = inDataBuffer->fBufferData + uiOffset;
                        uiRecLength = *((uint32_t*) pRecEntry);
                        pRecEntry += sizeof(uint32_t);
                        
                        if( pRecEntry + uiRecLength <= pEndBuffer )
                        {
                            uint32_t    uiTempLen;
                            uint32_t    uiAttribCount   = 0;
                            uint32_t    uiAttribIndex   = 0;
                            char        *pRecName       = NULL;
                            char        *pRecType       = NULL;

                            // Get record Type
                            uiTempLen = *((uint16_t *)pRecEntry);
                            pRecEntry += sizeof(uint16_t);
                            
                            if( 0 != uiTempLen )
                            {
                                pRecType = (char *) calloc( uiTempLen + 1, sizeof(char) );
                                if( NULL != pRecType )
                                {
                                    bcopy( pRecEntry, pRecType, uiTempLen );
                                }
                            }
                            pRecEntry += uiTempLen;
                            
                            // get Record Name
                            if( pRecEntry <= pEndBuffer )
                            {
                                uiTempLen = *((uint16_t *)pRecEntry);
                                pRecEntry += sizeof(uint16_t);
                                
                                if( 0 != uiTempLen )
                                {
                                    pRecName = (char *) calloc( uiTempLen + 1, sizeof(char) );
                                    if( NULL != pRecName )
                                    {
                                        bcopy( pRecEntry, pRecName, uiTempLen );
                                    }
                                }
                                pRecEntry += uiTempLen;
                            }

                            // get attribute count
                            if( pRecEntry <= pEndBuffer )
                            {
                                uiAttribCount = *((uint16_t *)pRecEntry);
                                pRecEntry += sizeof(uint16_t);
                            }
                            
                            if( pRecEntry <= pEndBuffer )
                            {
                                CFMutableDictionaryRef  cfNewRecord = CFDictionaryCreateMutable( cfAllocator, 0, 
                                                                                                 &kCFTypeDictionaryKeyCallBacks, 
                                                                                                 &kCFTypeDictionaryValueCallBacks );
                                
                                for( uiAttribIndex = 0; uiAttribIndex < uiAttribCount && pRecEntry <= pEndBuffer; uiAttribIndex++ )
                                {
                                    // block length
                                    if( bStdA )
                                    {
                                        uiTempLen = *((uint32_t *)pRecEntry);
                                        pRecEntry += sizeof(uint32_t);
                                    }
                                    else
                                    {
                                        uiTempLen = *((uint16_t *)pRecEntry);
                                        pRecEntry += sizeof( uint16_t );
                                    }
                                    
                                    // if the block length isn't right
                                    if( pRecEntry + uiTempLen > pEndBuffer )
                                    {
                                        break;
                                    }
                                    
                                    CFIndex cfAttribNameLen = *((uint16_t *)pRecEntry);
                                    pRecEntry += sizeof(uint16_t);
                                    
                                    if( pRecEntry + cfAttribNameLen <= pEndBuffer )
                                    {
                                        CFStringRef         cfAttribName        = NULL;
                                        CFMutableArrayRef   cfValues            = NULL;
                                        char                *pAttribName        = pRecEntry;
                                        uint16_t            uiAttribValIdx      = 0;
                                        uint16_t            usAttribValueCount;
                                        
                                        // move past the name and get to the values so we know how big of an array to make
                                        pRecEntry += cfAttribNameLen;
                                        
                                        usAttribValueCount = *((uint16_t *)pRecEntry);
                                        pRecEntry += sizeof( uint16_t );
                                        
                                        cfValues = CFArrayCreateMutable( cfAllocator, 0, &kCFTypeArrayCallBacks );
                                        
                                        for( uiAttribValIdx = 0; uiAttribValIdx < usAttribValueCount && pRecEntry <= pEndBuffer; uiAttribValIdx++ )
                                        {
                                            CFTypeRef   cfValue;
                                            uint32_t    uiAttribValueLen;
                                            
                                            if( bStdA )
                                            {
                                                uiAttribValueLen = *((uint32_t *) pRecEntry);
                                                pRecEntry += sizeof( uint32_t );
                                            }
                                            else
                                            {
                                                uiAttribValueLen = *((uint16_t *) pRecEntry);
                                                pRecEntry += sizeof( uint16_t );
                                            }
                                            
                                            if( pRecEntry + uiAttribValueLen <= pEndBuffer )
                                            {
                                                cfValue = CFStringCreateWithBytes( cfAllocator, (const UInt8 *) pRecEntry, uiAttribValueLen, 
                                                                                   kCFStringEncodingUTF8, false );
                                                if( NULL == cfValue )
                                                {
                                                    cfValue = CFDataCreate( cfAllocator, (const UInt8 *)pRecEntry, uiAttribValueLen );
                                                }
                                                
                                                if( NULL != cfValue )
                                                {
                                                    CFArrayAppendValue( cfValues, cfValue );
                                                    
                                                    CFRelease( cfValue );
                                                    cfValue = NULL;
                                                }
                                            }
                                            
                                            pRecEntry += uiAttribValueLen;
                                        }
                                        
                                        cfAttribName = CFStringCreateWithBytes( cfAllocator, (const UInt8 *) pAttribName, cfAttribNameLen, 
                                                                                kCFStringEncodingUTF8, false );

                                        CFDictionarySetValue( cfNewRecord, cfAttribName, cfValues );
                                        
                                        CFRelease( cfAttribName );
                                        cfAttribName = NULL;
                                        
                                        CFRelease( cfValues );
                                        cfValues = NULL;
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                                
                                if( NULL != cfNewRecord )
                                {
                                    if( NULL != pRecType && NULL != pRecName )
                                    {
										_ODRecord *pRecord = _createRecord( kCFAllocatorDefault );
										
										if( NULL != pRecord )
										{
											pRecord->_cfAttributes = (CFMutableDictionaryRef) CFRetain( cfNewRecord );
											pRecord->_cfRecordName = CFStringCreateWithCString( kCFAllocatorDefault, pRecName, 
																								kCFStringEncodingUTF8 );
											pRecord->_cfRecordType = CFStringCreateWithCString( kCFAllocatorDefault, pRecType, 
																								kCFStringEncodingUTF8 );
											pRecord->_ODNode = (_ODNode *) CFRetain( (CFTypeRef) inNode );
											
											// ensure record type and name are part of attributes
											if (CFDictionaryContainsKey(pRecord->_cfAttributes, kODAttributeTypeRecordType) == FALSE) {
												CFArrayRef recType = CFArrayCreate(kCFAllocatorDefault, (const void **)&pRecord->_cfRecordType, 1, &kCFTypeArrayCallBacks);
												
												CFDictionarySetValue(pRecord->_cfAttributes, kODAttributeTypeRecordType, recType);
												CFRelease(recType);
											}
											
											if (CFDictionaryContainsKey(pRecord->_cfAttributes, kODAttributeTypeRecordName) == FALSE) {
												CFArrayRef recName = CFArrayCreate(kCFAllocatorDefault, (const void **)&pRecord->_cfRecordName, 1, &kCFTypeArrayCallBacks);
												
												CFDictionarySetValue(pRecord->_cfAttributes, kODAttributeTypeRecordName, recName);
												CFRelease(recName);
											}
											
											CFArrayAppendValue( inArrayRef, (CFTypeRef) pRecord );

											CFRelease( (CFTypeRef) pRecord );
											pRecord = NULL;
										}
                                    }
                                    
                                    CFRelease( cfNewRecord );
                                    cfNewRecord = NULL;
                                }
                            }
                            
                            if( NULL != pRecType )
                            {
                                free( pRecType );
                                pRecType = NULL;
                            }
                            
                            if( NULL != pRecName )
                            {
                                free( pRecName );
                                pRecName = NULL;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            bufTag = 0;
        }
    }
    
    if( 0 == bufTag )
    {
        _AppendRecordsToListNonStd( inNode, inDataBuffer, inRecCount, inArrayRef, outError );
    }
	return true;
}

CFMutableDictionaryRef _CopyAttributesFromBuffer( tDirNodeReference inNodeRef, tDataBufferPtr inDataBuffer,
                                                 tAttributeListRef inAttrListRef, UInt32 inCount, CFErrorRef *outError )
{
    tAttributeValueListRef  attrValueRef    = 0;
    tAttributeEntryPtr      pAttrEntry      = NULL;
    tAttributeValueEntryPtr pValueEntry     = NULL;
    tDirStatus              dsStatus        = eDSNoErr;
    UInt32                  i;
    UInt32                  j;
    CFMutableDictionaryRef  cfRecord        = CFDictionaryCreateMutable( kCFAllocatorDefault, 0, 
                                                                         &kCFTypeDictionaryKeyCallBacks,
                                                                         &kCFTypeDictionaryValueCallBacks );
    
    for (i = 1; i <= inCount && eDSNoErr == dsStatus; i++)
    {
        dsStatus = dsGetAttributeEntry( inNodeRef, inDataBuffer, inAttrListRef, i, &attrValueRef, &pAttrEntry );
        if( eDSNoErr == dsStatus )
        {
            CFMutableArrayRef   cfAttrValues = CFArrayCreateMutable( kCFAllocatorDefault, 0,
                                                                     &kCFTypeArrayCallBacks );
            
            for (j =1 ; j <= pAttrEntry->fAttributeValueCount; j++)
            {
                CFTypeRef   cfAttributeValue = NULL;
                
                dsStatus = dsGetAttributeValue( inNodeRef, inDataBuffer, j, attrValueRef, &pValueEntry );
                
                if (eDSNoErr == dsStatus)
                {
                    cfAttributeValue = CFStringCreateWithCString( kCFAllocatorDefault, 
                                                                  pValueEntry->fAttributeValueData.fBufferData, 
                                                                  kCFStringEncodingUTF8 );
                    
                    if( NULL == cfAttributeValue )
                    {
                        cfAttributeValue = CFDataCreate( kCFAllocatorDefault, (const UInt8 *)pValueEntry->fAttributeValueData.fBufferData, 
                                                         pValueEntry->fAttributeValueData.fBufferLength );
                    }
                    
                    dsStatus = dsDeallocAttributeValueEntry( 0, pValueEntry );
                    pValueEntry = NULL;
                }
                
                if( NULL != cfAttributeValue )
                {
                    CFArrayAppendValue( cfAttrValues, cfAttributeValue );
                    
                    CFRelease( cfAttributeValue );
                    cfAttributeValue = NULL;
                }
            }
            
            dsCloseAttributeValueList( attrValueRef );
            
            // Now set the list to the attribute
            CFStringRef cfAttribName = CFStringCreateWithCString( kCFAllocatorDefault,
                                                                  pAttrEntry->fAttributeSignature.fBufferData,
                                                                  kCFStringEncodingUTF8 );
            if( NULL != cfAttribName )
            {
                CFDictionarySetValue( cfRecord, cfAttribName, cfAttrValues );
                
                CFRelease( cfAttribName );
                cfAttribName = NULL;
            }
            
            dsDeallocAttributeEntry( 0, pAttrEntry );
            pAttrEntry = NULL;
            
            CFRelease( cfAttrValues );
            cfAttrValues = NULL;
            
        }
    }

    if( eDSNoErr != dsStatus )
    {
        if( NULL != cfRecord )
        {
            CFRelease( cfRecord );
            cfRecord = NULL;
        }
    }
    
    if( eDSNoErr != dsStatus && NULL != outError && NULL == (*outError) )
    {
		CFStringRef cfDescription = CFSTR("Unable to obtain attributes from buffer returned by Directory Service.");
		CFStringRef cfError = _MapDSErrorToReason( outError, dsStatus );
        _ODErrorSet( outError, kODErrorDomainFramework, eDSNullParameter, 
                     &cfDescription, 
                     &cfError,
                     NULL );
    }
    
    return cfRecord;
}

static tDirStatus _Authenticate( ODNodeRef inNodeRef, char *inAuthType, char *inRecordType, CFArrayRef inAuthItems, 
                                 CFArrayRef *outAuthItems, ODContextRef *outContext, bool inAuthOnly )
{
    _ODNode         *pODNodeRef = (_ODNode *) inNodeRef;
    tDataBufferPtr  dsAuthType  = dsDataNodeAllocateString( 0, inAuthType );
    tDataBufferPtr  dsAuthData  = NULL;
    tDataBufferPtr  dsAuthStep  = dsDataBufferAllocate( 0, 1024 );
    tDirStatus      dsStatus    = eDSAuthFailed;
    CFIndex         iCount      = (NULL != inAuthItems ? CFArrayGetCount(inAuthItems) : 0 );
    char            *pTempPtr   = NULL;
    _ODContext      *pContext   = NULL;
    tContextData    dsContext   = 0;
    CFIndex         ii;

    if( NULL != outContext && NULL != (*outContext) )
    {
        pContext = (_ODContext *) (*outContext);
        dsContext = pContext->_dsContext;
    }
    
    if( strcmp(kDSStdAuthWithAuthorizationRef, inAuthType) == 0 || strcmp(kDSStdAuth2WayRandom, inAuthType) == 0 )
    {
        if( iCount > 0 )
        {
            CFTypeRef   cfRef = CFArrayGetValueAtIndex( inAuthItems, 0 );
            if( CFGetTypeID(cfRef) == CFDataGetTypeID() )
            {
                char        *pTemp  = (char *) CFDataGetBytePtr( cfRef );
                uint32_t    uiTemp  = CFDataGetLength( cfRef );
                
                dsAuthData = dsDataBufferAllocate(0, uiTemp);
                pTempPtr = dsAuthData->fBufferData;

                bcopy( pTemp, pTempPtr, uiTemp );
                pTempPtr += uiTemp;
            }
        }
    } else if (iCount == 1 && strncmp(kDSNativeAuthMethodPrefix, inAuthType, sizeof(kDSNativeAuthMethodPrefix)-1) == 0) {
		// native auths are just packed as raw data as we don't know how they are packed
		CFTypeRef cfRef = CFArrayGetValueAtIndex(inAuthItems, 0);
		UInt32 authBufSize = 0;
		
		if (CFGetTypeID(cfRef) == CFStringGetTypeID()) {
			authBufSize += CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfRef), kCFStringEncodingASCII);
		} else if (CFGetTypeID(cfRef) == CFDataGetTypeID()) {
			authBufSize += CFDataGetLength(cfRef);
		}

		dsAuthData = dsDataBufferAllocate(0, authBufSize);
		pTempPtr = dsAuthData->fBufferData;

		if (CFGetTypeID(cfRef) == CFStringGetTypeID()) {
			CFStringGetCString(cfRef, pTempPtr, authBufSize, kCFStringEncodingASCII);
			pTempPtr += strlen(pTempPtr);
		} else if (CFGetTypeID(cfRef) == CFDataGetTypeID()) {
			uint32_t    uiTemp  = CFDataGetLength(cfRef);
			
			bcopy(CFDataGetBytePtr(cfRef), pTempPtr, uiTemp);
			pTempPtr += uiTemp;
		}
	} else {
		int start = 0;
		UInt32 authBufSize = 0;
		
		/* 7716346: Precalculate the length so we don't overrun the buffer. */
		/* Note we don't need the GSSAPI special case, we will be at least large enough for that. */
		for (ii = 0; ii < iCount; ii++) {
			CFTypeRef cfRef = CFArrayGetValueAtIndex( inAuthItems, ii );

			authBufSize += sizeof(uint32_t);

			if (CFGetTypeID(cfRef) == CFStringGetTypeID()) {
				authBufSize += CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfRef), kCFStringEncodingUTF8);
			} else if (CFGetTypeID(cfRef) == CFDataGetTypeID()) {
				authBufSize += CFDataGetLength(cfRef);
			}
		}

		dsAuthData = dsDataBufferAllocate(0, authBufSize);
		pTempPtr = dsAuthData->fBufferData;

		if (strcmp("dsAuthMethodStandard:GSSAPI", inAuthType) == 0) {
			CFTypeRef cfRef = CFArrayGetValueAtIndex(inAuthItems, 0);
			
			/* GSS auth buffer is slightly different, first value has no length, it's a fixed 4 bytes
			 * 4 bytes  = export security context back to calling application
			 * 4 bytes  = length of service principal string
			 * string   = service principal string
			 * 4 bytes  = length of incoming key block
			 * r        = incoming key block
			 */
			
			if (CFGetTypeID(cfRef) == CFDataGetTypeID() && CFDataGetLength(cfRef) >= 4) {
				bcopy(CFDataGetBytePtr(cfRef), pTempPtr, 4);
				pTempPtr += 4;
			}
			
			start = 1;
		}
		
        for( ii = start; ii < iCount; ii++ )
        {
            CFTypeRef   cfRef = CFArrayGetValueAtIndex( inAuthItems, ii );
            
            if( CFGetTypeID(cfRef) == CFStringGetTypeID() )
            {
                char        *pTemp  = _GetCStringFromCFString( cfRef );
                uint32_t    uiTemp  = strlen( pTemp );
                
                *((uint32_t *) pTempPtr) = uiTemp;
                pTempPtr += sizeof( uint32_t );
                bcopy( pTemp, pTempPtr, uiTemp );
                pTempPtr += uiTemp;
                
                free( pTemp );
                pTemp = NULL;
            }
            else if( CFGetTypeID(cfRef) == CFDataGetTypeID() )
            {
                char        *pTemp  = (char *) CFDataGetBytePtr( cfRef );
                uint32_t    uiTemp  = CFDataGetLength( cfRef );
                
                *((uint32_t *) pTempPtr) = uiTemp;
                pTempPtr += sizeof( uint32_t );
                bcopy( pTemp, pTempPtr, uiTemp );
                pTempPtr += uiTemp;
            }
        }
    }

    if (dsAuthData == NULL) {
        dsAuthData = dsDataBufferAllocate(0, 2048);
        pTempPtr = dsAuthData->fBufferData;
    }
    
    dsAuthData->fBufferLength = (pTempPtr - dsAuthData->fBufferData);
    
    // if we have a record type other than Users do AuthOnRecordType, just in case plugin doesn't support the call
    if( NULL != inRecordType && strcmp(inRecordType, kDSStdRecordTypeUsers) != 0 )
    {
        tDataNodePtr    dsRecordType = dsDataNodeAllocateString( 0, inRecordType );

        if( NULL != dsRecordType )
        {
            do
            {
                dsStatus = dsDoDirNodeAuthOnRecordType( pODNodeRef->_dsNodeRef, dsAuthType, inAuthOnly, dsAuthData, dsAuthStep, &dsContext, 
                                                        dsRecordType );
                if( eDSBufferTooSmall == dsStatus )
                {
                    UInt32 newSize = (dsAuthStep->fBufferSize << 1);
                    
                    if( newSize < 100 * 1024 * 1024 )
                    {
                        dsDataBufferDeAllocate( 0, dsAuthStep );
                        dsAuthStep = dsDataBufferAllocate( 0, newSize );
                    }
                    else
                    {
                        dsStatus = eDSAllocationFailed;
                        break;
                    }
                }

                if( eDSInvalidNodeRef == dsStatus || eDSInvalidReference == dsStatus || eDSInvalidRefType == dsStatus )
                    dsStatus = _ReopenNode( pODNodeRef );
            } while( eDSBufferTooSmall == dsStatus || eDSInvalidNodeRef == dsStatus || eDSInvalidDirRef == dsStatus || 
                     eDSInvalidReference == dsStatus || eDSInvalidRefType == dsStatus );
            
            dsDataNodeDeAllocate( 0, dsRecordType );
            dsRecordType = NULL;
        }
    }
    else
    {
        do
        {
            dsStatus = dsDoDirNodeAuth( pODNodeRef->_dsNodeRef, dsAuthType, inAuthOnly, dsAuthData, dsAuthStep, &dsContext );

            if( eDSBufferTooSmall == dsStatus )
            {
                UInt32 newSize = (dsAuthStep->fBufferSize << 1);
                
                if( newSize < 100 * 1024 * 1024 )
                {
                    dsDataBufferDeAllocate( 0, dsAuthStep );
                    dsAuthStep = dsDataBufferAllocate( 0, newSize );
                }
                else
                {
                    dsStatus = eDSAllocationFailed;
                    break;
                }
            }

            if( eDSInvalidNodeRef == dsStatus || eDSInvalidReference == dsStatus || eDSInvalidRefType == dsStatus )
                dsStatus = _ReopenNode( pODNodeRef );
        } while( eDSBufferTooSmall == dsStatus || eDSInvalidNodeRef == dsStatus || eDSInvalidDirRef == dsStatus || 
                 eDSInvalidReference == dsStatus || eDSInvalidRefType == dsStatus );
    }
    
    if( NULL != dsAuthStep )
    {
        if( NULL != outAuthItems )
        {
            char    *pTemp  = dsAuthStep->fBufferData;
            char    *pEnd   = dsAuthStep->fBufferData + dsAuthStep->fBufferLength;
            
            // go through buffer and return a list of CFData for these
            *outAuthItems = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
            
            while( pTemp < pEnd )
            {
                uint32_t    uiTemp = *((uint32_t *) pTemp);
                CFDataRef   cfData;
                
                if ( pTemp + uiTemp > pEnd )
                {
                    dsStatus = eDSInvalidBuffFormat;
                    CFRelease( *outAuthItems );
                    *outAuthItems = NULL;
                    break;
                }
                
                pTemp += sizeof( uint32_t );
                cfData = CFDataCreate( kCFAllocatorDefault, (const UInt8 *)pTemp, uiTemp );
                pTemp += uiTemp;
                
                if( NULL != cfData )
                {
                    CFArrayAppendValue( (CFMutableArrayRef) *outAuthItems, cfData );
                    CFRelease( cfData );
                    cfData = NULL;
                }
            }
        }

        dsDataBufferDeAllocate( 0, dsAuthStep );
        dsAuthStep = NULL;
    }
    
    if( NULL != dsAuthData )
    {
        dsDataBufferDeAllocate( 0, dsAuthData );
        dsAuthData = NULL;
    }
    
    if( NULL != dsAuthType )
    {
        dsDataBufferDeAllocate( 0, dsAuthType );
        dsAuthType = NULL;
    }
    
    if( 0 != dsContext )
    {
        if( NULL != pContext )
        {
            pContext->_dsContext = dsContext;
        }
        else if( NULL != outContext )
        {
            (*outContext) = (ODContextRef) _createContext( CFGetAllocator(inNodeRef), dsContext, inNodeRef );
        }
        else
        {
            dsReleaseContinueData( pODNodeRef->_dsNodeRef, dsContext );
            dsContext = 0;
        }
    }
    else if( NULL != pContext ) // will be set if we had a context coming in
    {
        // pContext should be set from above.
        pContext->_dsContext = 0;
        
        CFRelease( *outContext );
        (*outContext) = NULL;
    }
    
    return dsStatus;
}

CFStringRef _createRandomPassword( void )
{
    char    password[9];
    int     i;
    int     punct = 0;
    
    while( !punct )
    {
        i = arc4random() % 0x7f;
        if( ispunct(i) )
            punct = i;
    }
    
    for (i = 0; i < 8; i++)
    {
        if( punct && (arc4random() & 0x100) )
        {
            password[i] = punct;
            punct = 0;
        }
        else
        {
            while( !isalnum(password[i]) || !isprint(password[i]) )
            {
                password[i] = arc4random() & 0x7f;
            }
        }
    }
    password[8] = 0;
    
    return CFStringCreateWithCString( kCFAllocatorDefault, password, kCFStringEncodingUTF8 );
}

bool _wasAttributeFetched( _ODRecord *inRecord, ODAttributeType inAttribute )
{
    bool     bFetched    = false;
    CFSetRef    cfFetched   = inRecord->_cfFetchedAttributes;
    
    if( NULL != cfFetched && NULL != inAttribute )
    {
        if( true == CFSetContainsValue(cfFetched, inAttribute) )
        {
            bFetched = true;
        }
        // if we fetched all attributes
        else if( true == CFSetContainsValue(cfFetched, kODAttributeTypeAllAttributes) )
        {
            bFetched = true;
        }
        // if it is a Native prefix and we've fetched all Native attributes
        else if( true == CFStringHasPrefix(inAttribute, CFSTR(kDSNativeAttrTypePrefix)) && 
                 true == CFSetContainsValue(cfFetched, kODAttributeTypeNativeOnly) )
        {
            bFetched = true;
        }
        // if it is standard prefix and we fetched Standard attributes
        else if( true == CFStringHasPrefix(inAttribute, CFSTR(kDSStdAttrTypePrefix)) && 
                 true == CFSetContainsValue(cfFetched, kODAttributeTypeStandardOnly) )
        {
            bFetched = true;
        }
    }
    return bFetched;
}

void _StripAttributesWithTypePrefix( CFMutableSetRef inSet, CFStringRef inPrefix )
{
    CFIndex     iCount      = CFSetGetCount( inSet );
    void        **cfList    = (void *) calloc( iCount, sizeof(void **) );
    CFIndex     ii;
    
    CFSetGetValues( inSet, (const void **) cfList );
    
    for( ii = 0; ii < iCount; ii ++ )
    {
        if( true == CFStringHasPrefix(cfList[ii], inPrefix) )
        {
            CFSetRemoveValue( inSet, cfList[ii] );
        }
    }
    
    free( cfList );
}

CFMutableSetRef _minimizeAttributeSet( CFSetRef inSet )
{
    CFMutableSetRef returnValue = CFSetCreateMutable( kCFAllocatorDefault, 0, &kCFTypeSetCallBacks );
    
    if( true == CFSetContainsValue(inSet, kODAttributeTypeAllAttributes) ||
        ( true == CFSetContainsValue(inSet, kODAttributeTypeNativeOnly) &&
          true == CFSetContainsValue(inSet, kODAttributeTypeStandardOnly) ) )
    {
        CFSetAddValue( returnValue, kODAttributeTypeAllAttributes );
    }
    else
    {
        CFMutableSetRef tempSet     = CFSetCreateMutableCopy( kCFAllocatorDefault, 0, inSet );
        
        if( true == CFSetContainsValue(inSet, kODAttributeTypeNativeOnly) )
        {
            CFSetAddValue( returnValue, kODAttributeTypeNativeOnly );
            _StripAttributesWithTypePrefix( tempSet, CFSTR(kDSNativeAttrTypePrefix) );
        }
        
        if( true == CFSetContainsValue(inSet, kODAttributeTypeStandardOnly) )
        {
            CFSetAddValue( returnValue, kODAttributeTypeStandardOnly );
            _StripAttributesWithTypePrefix( tempSet, CFSTR(kDSStdAttrTypePrefix) );
        }
        
        // now add the remaining since we've filtered it for collection names
        CFIndex     iCount      = CFSetGetCount( tempSet );
        void        **cfList    = (void *) calloc( iCount, sizeof(void **) );
        CFIndex     ii;
        
        CFSetGetValues( tempSet, (const void **) cfList );
        
        for( ii = 0; ii < iCount; ii ++ )
        {
            CFSetAddValue( returnValue, cfList[ii] );
        }
        
        free( cfList );
        CFRelease( tempSet );
    }
    
    return returnValue;
}

CFSetRef _attributeListToSet( CFArrayRef inAttributes )
{
    CFMutableSetRef returnValue;
    
    if( NULL != inAttributes )
    {
        CFIndex         iCount      = CFArrayGetCount( inAttributes );
        CFIndex         ii;
     
        CFMutableSetRef tempSet = CFSetCreateMutable( kCFAllocatorDefault, 0, &kCFTypeSetCallBacks );

        for( ii = 0; ii < iCount; ii++ )
        {
            CFSetAddValue( tempSet, CFArrayGetValueAtIndex(inAttributes,ii) );
        }
        
        returnValue = _minimizeAttributeSet( tempSet );

        CFRelease( tempSet );
    }
    else
    {
        returnValue = CFSetCreateMutable( kCFAllocatorDefault, 0, &kCFTypeSetCallBacks );
    }
    
    return returnValue;
}

tDataListPtr _ConvertCFSetToDataList( CFSetRef inSet )
{
    tDataListPtr    dsDataList  = NULL;
    
    if( NULL != inSet )
    {
        CFIndex     iCount  = CFSetGetCount( inSet );
        void        **cfList = (void *) calloc( iCount, sizeof(void **) );
        
        CFSetGetValues( inSet, (const void **) cfList );
        
        CFArrayRef  cfArray = CFArrayCreate( kCFAllocatorDefault, (const void **) cfList, iCount, &kCFTypeArrayCallBacks );
        
        dsDataList = _ConvertCFArrayToDataList( cfArray );
        
        CFRelease( cfArray );
        cfArray = NULL;
        
        free( cfList );
        cfList = NULL;
    }
    
    return dsDataList;
}

// we don't use the DirectoryServiceCore one because it's not 64-bit clean
tDataListPtr _ConvertCFArrayToDataList( CFArrayRef inArray )
{
    tDataListPtr    dsDataList  = dsDataListAllocate( 0 );
    
    if( NULL != inArray )
    {
        CFIndex         iCount          = CFArrayGetCount( inArray );
        CFIndex         ii;
        
        // go in reverse when building the list, it's faster
        for( ii = iCount - 1; ii >= 0; ii-- )
        {
            CFTypeRef       cfRef       = CFArrayGetValueAtIndex( inArray, ii );
            tDataBufferPtr  dsDataNode  = _GetDataBufferFromCFType( cfRef );
            
            if( NULL != dsDataNode )
            {
                dsDataListInsertAfter( 0, dsDataList, dsDataNode, 0 );
                
                dsDataNodeDeAllocate( 0, dsDataNode );
                dsDataNode = NULL;
            }
        }
    }
    
    return dsDataList;
}
