/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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

#include "BaseDirectoryPlugin.h"

#include <Security/Authorization.h>
#include <DirectoryService/DirServices.h>
#include <DirectoryService/DirServicesUtils.h>
#include <DirectoryServiceCore/DSUtils.h>
#include <CoreFoundation/CoreFoundation.h>
#include <DirectoryServiceCore/CSharedData.h>
#include <DirectoryServiceCore/CContinue.h>
#include <DirectoryServiceCore/CPluginRef.h>
#include <DirectoryServiceCore/CBuff.h>
#include <dispatch/dispatch.h>
#include "BDPIVirtualNode.h"
#include "DirServicesPriv.h"

extern "C" int ConvertXMLPolicyToSpaceDelimited( const char *inXMLDataStr, char **outPolicyStr );

#ifndef __OBJC__
	#define EXCEPTION_START		try {
	#define EXCEPTION_END		} catch ( ... ) { \
									DbgLog( kLogPlugin, "%s: uncaught exception in %s:%d", fPlugInName, __FILE__, __LINE__ ); \
								}
	#define EXCEPTION_END2		} catch ( ... ) { \
									DbgLog( kLogPlugin, "uncaught exception in %s:%d", __FILE__, __LINE__ ); \
								}

#else
	#define EXCEPTION_START		NSAutoreleasePool *pool = [NSAutoreleasePool new]; \
								@try {

	#define EXCEPTION_END		} @catch( id exception ) { \
									DbgLog( kLogPlugin, "%s: caught an exception %s in %s:%d", fPlugInName, [[exception description] UTF8String], \
											__FILE__, __LINE__ ); \
								} @finally { \
									[pool release]; \
								}
	#define EXCEPTION_END2		} @catch( id exception ) { \
									DbgLog( kLogPlugin, "caught an exception %s in %s:%d", [[exception description] UTF8String], \
											__FILE__, __LINE__ ); \
								} @finally { \
									[pool release]; \
								}
#endif

#pragma mark -
#pragma mark -------------Intitialization--------------

// --------------------------------------------------------------------------------
//	* BaseDirectoryPlugin ()
// --------------------------------------------------------------------------------
#ifdef SERVERINTERNAL
BaseDirectoryPlugin::BaseDirectoryPlugin( FourCharCode inSig, const char *inName )
	: CServerPlugin(inSig, inName), fBasePluginMutex("BaseDirectoryPlugin::fBasePluginMutex")
#else
BaseDirectoryPlugin::BaseDirectoryPlugin( const char *inName ) : fBasePluginMutex("BaseDirectoryPlugin::fBasePluginMutex")
#endif
{
	fState = kUnknownState | kInactive;
	fContextHash = new CPlugInRef( BaseDirectoryPlugin::ContextDeallocProc );
	fContinueHash = new CContinue( BaseDirectoryPlugin::ContextDeallocProc );
	fPluginRunLoop = NULL;
	fKerberosMutex = NULL;
	fReadyForRequests = new DSEventSemaphore;
	
	int iLen = strlen(inName) + sizeof("/") + 1;
	fPluginPrefix = (char *) malloc( iLen );
	
#ifndef SERVERINTERNAL
	fPlugInName = inName; // we have to assign this here since no one does unless CServerPlugin
#endif

	strlcpy( fPluginPrefix, "/", iLen );
	strlcat( fPluginPrefix, inName, iLen );
	
	fPluginPrefixLen = strlen( fPluginPrefix );
	fPluginPrefixCF = CFStringCreateWithCStringNoCopy( kCFAllocatorDefault, fPluginPrefix, kCFStringEncodingUTF8, kCFAllocatorNull );
	
	// default to our own internal custom calls
	fCustomCallReadConfigSize = eDSCustomCallReadPluginConfigSize;
	fCustomCallReadConfig = eDSCustomCallReadPluginConfigData;
	fCustomCallWriteConfig = eDSCustomCallWritePluginConfigData;
	fCustomCallVerifyConfig = eDSCustomCallVerifyPluginConfigData;	
	fQueue = dispatch_queue_create("BaseDirectoryPlugin fQueue", NULL);
    fTransitionTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, fQueue);
    
    dispatch_source_set_event_handler(fTransitionTimer,
                                      ^(void) {
                                          DbgLog( kLogInfo, "%s: Network transition - acting on the timer", fPlugInName );
                                          NetworkTransition();
                                      });
} // BaseDirectoryPlugin

// --------------------------------------------------------------------------------
//	* ~BaseDirectoryPlugin ()
// --------------------------------------------------------------------------------

BaseDirectoryPlugin::~BaseDirectoryPlugin ( void )
{
	DSDelete( fContextHash );
	DSDelete( fContinueHash );
	DSDelete( fReadyForRequests );
	DSFree( fPluginPrefix );
	DSCFRelease( fPluginPrefixCF );
	dispatch_release( fQueue );
    dispatch_source_cancel(fTransitionTimer);
    dispatch_release(fTransitionTimer);
} // ~BaseDirectoryPlugin

SInt32 BaseDirectoryPlugin::Initialize( void )
{
	fBasePluginMutex.WaitLock();
	
	fState |= kInitialized;		// enable the initialized bit
	fState &= ~kUninitialized;	// now disable the uninitialized bit

	fBasePluginMutex.SignalLock();
	
	fReadyForRequests->PostEvent();
	
	DbgLog( kLogPlugin, "%s::Initialize completed posting event to allow requests", fPlugInName );
	
	return eDSNoErr;
}

SInt32 BaseDirectoryPlugin::SetPluginState( const UInt32 inState )
{
	fBasePluginMutex.WaitLock();
	
	if (inState & kActive)
	{
		fState |= kActive;		// go active
		fState &= ~kInactive;	// turn off the Inactive flag
		
		DbgLog( kLogPlugin, "%s::SetPluginState state is now Active", fPlugInName );
	}
	
	if (inState & kInactive)
	{
		fState |= kInactive;	// go Inactive
		fState &= ~kActive;		// disable the Active flag

		DbgLog( kLogPlugin, "%s::SetPluginState state is now Inactive", fPlugInName );
	}
	
	fBasePluginMutex.SignalLock();
	
	return eDSNoErr;
}

CDSAuthParams*	BaseDirectoryPlugin::NewAuthParamObject( void )
{
	return new CDSAuthParams();
}


#pragma mark -
#pragma mark -------------MainEntry Point--------------

// ---------------------------------------------------------------------------
//	* ProcessRequest
// ---------------------------------------------------------------------------
SInt32 BaseDirectoryPlugin::ProcessRequest ( void *inData )
{
    tDirStatus	siResult	= eNotHandledByThisNode;
    sHeader		*pMsgHdr	= (sHeader *) inData;
	char		*recTypeStr	= NULL;
	
    if ( inData == NULL )
    {
        return ePlugInDataError;
    }
	
	// we allow a subset of calls if we are not initialized
	switch (pMsgHdr->fType)
	{
		case kKerberosMutex:
			fKerberosMutex = (DSMutexSemaphore *) pMsgHdr->fContextData;
			return eDSNoErr;
		case kServerRunLoop:
			fPluginRunLoop = (CFRunLoopRef) pMsgHdr->fContextData;
			return eDSNoErr;
	}
	
	fReadyForRequests->WaitForEvent();
	
	if ( fState == kUnknownState )
	{
		return ePlugInCallTimedOut;
	}
	else if ( (fState & kFailedToInit) || !(fState & kInitialized) )
	{
		DbgLog( kLogPlugin, "%s::ProcessRequest %d condition fState = %d", fPlugInName, ePlugInFailedToInitialize, fState );
		return ePlugInFailedToInitialize;
	}
	
	// block all requests except for OpenDirNode and Custom Calls when not active
	if ( (fState & kInactive) || !(fState & kActive) )
	{
		if ( pMsgHdr->fType != kDoPlugInCustomCall && pMsgHdr->fType != kOpenDirNode )
		{
			return ePlugInNotActive;
		}
		else if ( pMsgHdr->fType == kOpenDirNode )
		{
			if ( ((sOpenDirNode *)inData)->fInDirNodeName != NULL )
			{
				char *pathStr = dsGetPathFromListPriv( ((sOpenDirNode *)inData)->fInDirNodeName, "/" );
				if ( pathStr != NULL )
				{
					// default path for configure is "/Plugin"
					if ( strcmp(pathStr, fPluginPrefix) != 0 )
					{
						DSFreeString( pathStr );
						return eDSOpenNodeFailed;
					}
					
					DSFreeString( pathStr );
				}
			}
		}
	}
	
	switch (pMsgHdr->fType)
	{
		case kOpenDirNode:
			siResult = OpenDirNode( (sOpenDirNode *)inData );
			break;

		case kCloseDirNode:
			siResult = CloseDirNode( (sCloseDirNode *)inData );
			break;

		case kReleaseContinueData:
			siResult = ReleaseContinueData( (sReleaseContinueData *)inData );
			break;

		case kGetDirNodeInfo:
			siResult = GetDirNodeInfo( (sGetDirNodeInfo *)inData );
			break;

		case kGetRecordList:
			siResult = GetRecordList( (sGetRecordList *)inData );
			break;

		case kGetRecordEntry:
			siResult = GetRecordEntry( (sGetRecordEntry *)inData );
			break;

		case kGetAttributeEntry:
			siResult = GetAttributeEntry( (sGetAttributeEntry *)inData );
			break;

		case kGetAttributeValue:
			siResult = GetAttributeValue( (sGetAttributeValue *)inData );
			break;

		case kOpenRecord:
			siResult = OpenRecord( (sOpenRecord *)inData );
			break;

		case kGetRecordReferenceInfo:
			siResult = GetRecRefInfo( (sGetRecRefInfo *)inData );
			break;

		case kGetRecordAttributeInfo:
			siResult = GetRecAttribInfo( (sGetRecAttribInfo *)inData );
			break;

		case kGetRecordAttributeValueByValue:
			siResult = GetRecAttrValueByValue( (sGetRecordAttributeValueByValue *)inData );
			break;
			
		case kGetRecordAttributeValueByIndex:
			siResult = GetRecAttrValueByIndex( (sGetRecordAttributeValueByIndex *)inData );
			break;

		case kGetRecordAttributeValueByID:
			siResult = GetRecAttrValueByID( (sGetRecordAttributeValueByID *)inData );
			break;

		case kCloseAttributeList:
			siResult = CloseAttributeList( (sCloseAttributeList *)inData );
			break;

		case kCloseAttributeValueList:
			siResult = CloseAttributeValueList( (sCloseAttributeValueList *)inData );
			break;

		case kCloseRecord:
			siResult = CloseRecord( (sCloseRecord *)inData );
			break;

		case kSetRecordName:
			siResult = SetRecordName( (sSetRecordName *) inData );
			break;

		case kSetRecordType:
			siResult = SetRecordType( (sSetRecordType *)inData );
			break;

		case kDeleteRecord:
			siResult = DeleteRecord( (sDeleteRecord *) inData );
			break;

		case kCreateRecord:
		case kCreateRecordAndOpen:
			siResult = CreateRecord( (sCreateRecord *) inData );
			break;

		case kAddAttribute:
			recTypeStr = GetRecordTypeFromRef( ((sAddAttribute *)inData)->fInRecRef );
			siResult = AddAttribute( (sAddAttribute *) inData, recTypeStr );
			break;

		case kRemoveAttribute:
			recTypeStr = GetRecordTypeFromRef( ((sRemoveAttribute *)inData)->fInRecRef );
			siResult = RemoveAttribute( (sRemoveAttribute *)inData, recTypeStr );
			break;

		case kAddAttributeValue:
			recTypeStr = GetRecordTypeFromRef( ((sAddAttributeValue *)inData)->fInRecRef );
			siResult = AddAttributeValue( (sAddAttributeValue *) inData, recTypeStr );
			break;

		case kRemoveAttributeValue:
			recTypeStr = GetRecordTypeFromRef( ((sRemoveAttributeValue *)inData)->fInRecRef );
			siResult = RemoveAttributeValue( (sRemoveAttributeValue *) inData, recTypeStr );
			break;

		case kSetAttributeValue:
		case kSetAttributeValues:
			recTypeStr = GetRecordTypeFromRef( ((sSetAttributeValue *)inData)->fInRecRef );
			siResult = SetAttributeValue( (sSetAttributeValue *) inData, recTypeStr );
			break;

		case kDoDirNodeAuth:
		case kDoDirNodeAuthOnRecordType:
			if ( pMsgHdr->fType == kDoDirNodeAuthOnRecordType )
			{
				recTypeStr = dsCStrFromCharacters( ((sDoDirNodeAuthOnRecordType *)inData)->fInRecordType->fBufferData,
												   ((sDoDirNodeAuthOnRecordType *)inData)->fInRecordType->fBufferLength );
			}
			else
			{
				recTypeStr = strdup( kDSStdRecordTypeUsers );
			}
			
			{
				sDoDirNodeAuth *authData = (sDoDirNodeAuth *)inData;
				CDSAuthParams *pb = this->NewAuthParamObject();
				
				if ( pb != NULL )
				{
					siResult = pb->LoadParamsForAuthMethod(
										authData->fInAuthMethod,
										authData->fInAuthStepData,
										authData->fOutAuthStepDataResponse );
					
					if ( siResult == eDSNoErr )
						pb->PostEvent( (siResult = DoAuthentication((sDoDirNodeAuth *)inData, recTypeStr, *pb)) );
					
					delete pb;
				}
				else
				{
					siResult = eDSAuthParameterError;
				}
			}
			break;
			
		case kDoMultipleAttributeValueSearch:
		case kDoAttributeValueSearch:
			siResult = DoAttributeValueSearch( (sDoAttrValueSearch *)inData );
			break;
		
		case kDoMultipleAttributeValueSearchWithData:
		case kDoAttributeValueSearchWithData:
			siResult = DoAttributeValueSearchWithData( (sDoAttrValueSearchWithData *)inData );
			break;
			
		case kDoPlugInCustomCall:
			siResult = DoPlugInCustomCall( (sDoPlugInCustomCall *)inData );
			break;

		case kFlushRecord:
			siResult = FlushRecord( (sFlushRecord *)inData );
			break;

		case kHandleNetworkTransition:
            dispatch_source_set_timer(fTransitionTimer, dispatch_time(0, 2ull * NSEC_PER_SEC), 0, 0);
            siResult = eDSNoErr;
			break;
		
		default:
			DbgLog( kLogPlugin, "%s: Unknown request type %d", fPlugInName, (int)pMsgHdr->fType );
			siResult = eNotHandledByThisNode;
			break;
	}
	
	pMsgHdr->fResult = siResult;
	
	DSFreeString( recTypeStr );
	
	return( siResult );

} // HandleRequest

#pragma mark -
#pragma mark ------------ Node Registrations ------------

tDirStatus BaseDirectoryPlugin::RegisterNode( CFStringRef inNodeName, eDirNodeType inType )
{
	char		*tmpStr		= NULL;
	const char	*nodeName	= GetCStringFromCFString( inNodeName, &tmpStr );
	SInt32		status		= eDSRegisterCustomFailed;
	
	if ( nodeName != NULL )
	{
		tDataListPtr nodePath = dsBuildFromPathPriv( nodeName, "/" );
		
		if ( nodePath != NULL )
		{
#ifdef SERVERINTERNAL
			status = CServerPlugin::_RegisterNode( fPlugInSignature, nodePath, inType );
#else
			status = DSRegisterNode( fPlugInSignature, nodePath, inType );
#endif
			if ( status != eDSNoErr )
			{
				DbgLog( kLogPlugin, "%s: Failed to register node %s type %d with error %d", fPlugInName, nodeName, inType, status );
			}
			else
			{
				DbgLog( kLogPlugin, "%s: Registered node %s type %d", fPlugInName, nodeName, inType );
			}
			
			dsDataListDeallocatePriv( nodePath );
			free( nodePath );
		}
	}
	else
	{
		DbgLog( kLogPlugin, "%s: Failed to register node because NULL name", fPlugInName );
	}
	
	DSFreeString( tmpStr );
	
	return (tDirStatus)status;
}

tDirStatus BaseDirectoryPlugin::UnregisterNode( CFStringRef inNodeName, eDirNodeType inType )
{
	char		*tmpStr		= NULL;
	const char	*nodeName	= GetCStringFromCFString( inNodeName, &tmpStr );
	SInt32		status		= eDSUnRegisterFailed;
	
	if ( nodeName != NULL )
	{
		tDataListPtr nodePath = dsBuildFromPathPriv( nodeName, "/" );
		
		if ( nodePath != NULL )
		{
#ifdef SERVERINTERNAL
			status = CServerPlugin::_UnregisterNode( fPlugInSignature, nodePath );
#else
			status = DSUnregisterNode( fPlugInSignature, nodePath );
#endif
			if ( status != eDSNoErr )
			{
				DbgLog( kLogPlugin, "%s: Failed to unregister node %s with error %d", fPlugInName, nodeName, status );
			}
			
			dsDataListDeallocatePriv( nodePath );
			free( nodePath );
		}
	}
	
	DSFreeString( tmpStr );
	
	return (tDirStatus)status;
}

#pragma mark -
#pragma mark ----------- Other Support functions -----------

void BaseDirectoryPlugin::NetworkTransition( void )
{
	
}

#pragma mark -
#pragma mark ------------ Custom Call --------------

tDirStatus BaseDirectoryPlugin::DoPlugInCustomCall( sDoPlugInCustomCall *inData )
{
    SInt32				siResult	= eDSNoErr;
    AuthorizationRef	authRef		= NULL;
	CFStringRef			cfNodeName	= NULL;

	EXCEPTION_START
	
	if ( inData == NULL || inData->fInRequestData == NULL )
	{
		siResult = eDSNullParameter;
		goto failure;
	}
	
	if ( inData->fInRequestData->fBufferData == NULL )
	{
		siResult = eDSEmptyBuffer;
		goto failure;
	}
	
	sBDPINodeContext *pContext = (sBDPINodeContext *) fContextHash->GetItemData( inData->fInNodeRef );
	if ( pContext == NULL )
	{
		siResult = eDSBadContextData;
		goto failure;
	}
	
	if ( pContext->fType != kBDPIDirNode || pContext->fVirtualNode == NULL )
	{
		siResult = eDSInvalidNodeRef;
		goto failure;
	}

#ifndef __OBJC__
	cfNodeName = pContext->fVirtualNode->CopyNodeName();
#else
	cfNodeName = (CFStringRef) [pContext->fVirtualNode copyNodeName];
#endif
	
	// if this is out configuration node, then we handle the standard stuff
	if ( IsConfigureNodeName(cfNodeName) == true )
	{
		// Request for current Configuration
		if ( inData->fInRequestCode == fCustomCallReadConfigSize || inData->fInRequestCode == fCustomCallReadConfig)
		{
			CFDataRef configData = CopyConfiguration();
			if ( configData != NULL )
			{
				CFIndex iLength = CFDataGetLength( configData );
				
				if ( inData->fInRequestCode == fCustomCallReadConfigSize )
				{
					if ( inData->fOutRequestResponse->fBufferSize >= sizeof(UInt32) )
					{
						*((UInt32 *) inData->fOutRequestResponse->fBufferData) = iLength;
						inData->fOutRequestResponse->fBufferLength = sizeof(UInt32);
					}
					else
					{
						siResult = eDSBufferTooSmall;
					}
				}
				else
				{
					if( inData->fOutRequestResponse->fBufferSize >= (UInt32) iLength )
					{
						CFDataGetBytes( configData, CFRangeMake(0, iLength), (UInt8 *)inData->fOutRequestResponse->fBufferData );
						inData->fOutRequestResponse->fBufferLength = iLength;
					}
					else
					{
						siResult = eDSBufferTooSmall;
					}
				}
				
				CFRelease( configData );
				configData = NULL;
			}
		}
		else if ( inData->fInRequestCode == fCustomCallWriteConfig || inData->fInRequestCode == fCustomCallVerifyConfig )
		{
			UInt32	bufLen			= inData->fInRequestData->fBufferLength;
			bool	bVerifyAuthRef	= true;
			
			if ( bufLen < sizeof(AuthorizationExternalForm) )
			{
				siResult = eDSInvalidBuffFormat;
				goto failure;
			}
			
			if ( pContext->fUID == 0 || pContext->fEffectiveUID == 0 )
			{
				AuthorizationExternalForm	blankExtForm;
                
                bzero( &blankExtForm, sizeof(blankExtForm) );
				
				if ( memcmp(inData->fInRequestData->fBufferData, &blankExtForm, sizeof(AuthorizationExternalForm)) == 0 )
				{
					bVerifyAuthRef = false;
				}
			}
			
			if ( bVerifyAuthRef )
			{
				siResult = AuthorizationCreateFromExternalForm((AuthorizationExternalForm *)inData->fInRequestData->fBufferData, &authRef);
				if ( siResult != errAuthorizationSuccess )
				{
					if ( authRef != NULL )
					{
						AuthorizationFree(authRef, kAuthorizationFlagDefaults);
						
						siResult = eDSPermissionError;
						goto failure;
					}
				}
				
				AuthorizationItem rights[] = { {"system.services.directory.configure", 0, 0, 0} };
				AuthorizationItemSet rightSet = { sizeof(rights)/sizeof(*rights), rights };
				AuthorizationItemSet *resultRightSet;
				
				siResult = AuthorizationCopyRights( authRef, &rightSet, NULL, kAuthorizationFlagExtendRights, &resultRightSet );
				
				// first lets free them before we throw an error
				if ( resultRightSet != NULL )
				{
					AuthorizationFreeItemSet(resultRightSet);
					AuthorizationFree(authRef, kAuthorizationFlagDefaults);
					resultRightSet = NULL;
				}
				
				// if not successful, we throw a permission error
				if ( siResult != errAuthorizationSuccess )
				{
					siResult = eDSPermissionError;
					goto failure;
				}
			}
			
			// ..... If we get this far, they are authorized to continue....
			
			// New Configuration....
			if ( inData->fInRequestCode == fCustomCallWriteConfig )
			{
				UInt32	length			= inData->fInRequestData->fBufferLength - sizeof( AuthorizationExternalForm );
				char	*newLocation	= inData->fInRequestData->fBufferData + sizeof( AuthorizationExternalForm );
				
				if ( NewConfiguration(newLocation, length) == false )
				{
					siResult = eDSInvalidPlugInConfigData;
					goto failure;
				}
			}
			
			// Test the configuration
			if ( inData->fInRequestCode == fCustomCallVerifyConfig )
			{
				UInt32	length			= inData->fInRequestData->fBufferLength - sizeof( AuthorizationExternalForm );
				char	*newLocation	= inData->fInRequestData->fBufferData + sizeof( AuthorizationExternalForm );
				
				siResult = CheckConfiguration( newLocation, length );
			}
		}
		else
		{
			siResult = HandleCustomCall( pContext, inData );
		}
	}
	else
	{
		CFDataRef			cfRequestData	= CFDataCreateWithBytesNoCopy( kCFAllocatorDefault, (const UInt8 *) inData->fInRequestData->fBufferData, 
																		   inData->fInRequestData->fBufferLength, kCFAllocatorNull );
		CFMutableDataRef	cfResponseData	= CFDataCreateMutable( kCFAllocatorDefault, inData->fOutRequestResponse->fBufferSize );
		
#ifndef __OBJC__
		siResult = pContext->fVirtualNode->CustomCall( inData->fInRequestCode, cfRequestData, cfResponseData, 
													   inData->fOutRequestResponse->fBufferSize );
#else
		siResult = [pContext->fVirtualNode customCall: inData->fInRequestCode 
										  requestData: (NSData *)cfRequestData
											 response: (NSMutableData *)cfResponseData
									  maxResponseSize: inData->fOutRequestResponse->fBufferSize];
#endif
		
		CFIndex	iLength = CFDataGetLength( cfResponseData );
		if ( iLength != 0 )
		{
			inData->fOutRequestResponse->fBufferLength = iLength;
			CFDataGetBytes( cfResponseData, CFRangeMake(0, iLength), (UInt8 *) inData->fOutRequestResponse->fBufferData );
		}
		
		DSCFRelease( cfRequestData );
		DSCFRelease( cfResponseData );
	}
	
	DSCFRelease( cfNodeName );

	EXCEPTION_END
	
failure:
	
    return( (tDirStatus)siResult );
}

#pragma mark -
#pragma mark ------------Node Functions--------------

tDirStatus BaseDirectoryPlugin::OpenDirNode( sOpenDirNode *inData )
{
    SInt32				siResult		= eDSNoErr;
    char				*pathStr		= NULL;
    sBDPINodeContext	*pContext		= NULL;

	EXCEPTION_START
	
	pathStr = dsGetPathFromListPriv( inData->fInDirNodeName, "/" );
	if ( pathStr == NULL )
	{
		siResult = eDSNullNodeName;
		goto failure;
	}

	// see if the prefix matches our prefix
	if ( strncmp(pathStr, fPluginPrefix, fPluginPrefixLen) == 0 )
	{
		CFStringRef	cfNodePath = CFStringCreateWithCString( kCFAllocatorDefault, pathStr, kCFStringEncodingUTF8 );
		if ( cfNodePath != NULL )
		{
			BDPIVirtualNode *pEngine = CreateNodeForPath( cfNodePath, inData->fInUID, inData->fInEffectiveUID );
			if ( pEngine != NULL )
			{
				// we pass the domain name so that we can invalidate the cache if it changed.....
				pContext = (sBDPINodeContext *) MakeContextData( kBDPIDirNode );
				if ( pContext == NULL )
				{
#ifndef __OBJC__
					delete pEngine;
#else
					[pEngine release];
#endif
					siResult = eMemoryAllocError;
					goto failure;
				}
				
				pContext->fVirtualNode = pEngine;
				pContext->fEffectiveUID = inData->fInEffectiveUID;
				pContext->fUID = inData->fInUID;
				
				siResult = fContextHash->AddItem( inData->fOutNodeRef, pContext );
			}
			else
			{
				siResult = eDSOpenNodeFailed;
			}
			
			DSCFRelease( cfNodePath );
		}
		else
		{
			siResult = eDSOpenNodeFailed;
		}
	}
	else
	{
		siResult = eNotHandledByThisNode;
	}
	
	EXCEPTION_END
	
failure:
	
    // if we errored anywhere along the way, deallocate the context and delete from hash
    if( siResult != eDSNoErr )
	{
		fContextHash->RemoveItem( inData->fOutNodeRef );
    }
	
    free( pathStr );

    return( (tDirStatus)siResult );
}

tDirStatus BaseDirectoryPlugin::CloseDirNode( sCloseDirNode *inData )
{
    SInt32				siResult	= eDSNoErr;
	sBDPINodeContext	*pContext	= (sBDPINodeContext *) fContextHash->GetItemData( inData->fInNodeRef );
	
	if ( pContext == NULL )
	{
		siResult = eDSBadContextData;
		goto failure;
	}
	
	if ( pContext->fType != kBDPIDirNode )
	{
		siResult = eDSInvalidNodeRef;
		goto failure;
	}

	fContinueHash->RemovePointersForRefNum( inData->fInNodeRef );
	fContextHash->RemoveItem( inData->fInNodeRef );

failure:
	
    return( (tDirStatus)siResult );
}

tDirStatus BaseDirectoryPlugin::GetDirNodeInfo( sGetDirNodeInfo *inData )
{
    SInt32					siResult	= eDSNoErr;
	CFMutableDictionaryRef	cfNodeInfo	= NULL;
	
	EXCEPTION_START
	
	sBDPINodeContext *pContext = (sBDPINodeContext *) fContextHash->GetItemData( inData->fInNodeRef );
	if ( pContext == NULL )
	{
		siResult = eDSBadContextData;
		goto failure;
	}
	
	if ( pContext->fType != kBDPIDirNode )
	{
		siResult = eDSInvalidNodeRef;
		goto failure;
	}
	
	if ( inData->fInDirNodeInfoTypeList == NULL )
	{
		siResult = eDSNullNodeInfoTypeList;
		goto failure;
	}
	
	if ( inData->fInDirNodeInfoTypeList->fDataNodeCount == 0 )
	{
		siResult = eDSEmptyNodeInfoTypeList;
		goto failure;
	}
	
	if ( inData->fOutContinueData != 0 )
	{
		sBDPISearchRecordsContext *pContinue = (sBDPISearchRecordsContext *) fContinueHash->GetPointer( inData->fOutContinueData );
		if ( pContinue == NULL )
		{
			siResult = eDSInvalidContinueData;
			goto failure;
		}
		
		cfNodeInfo = (CFMutableDictionaryRef) pContinue->fStateInfo;
		pContinue->fStateInfo = NULL;
		
		fContinueHash->RemoveContext( inData->fOutContinueData );
		inData->fOutContinueData = 0;
	}
	else
	{
		CFArrayRef cfAttributes = CreateCFArrayFromList( inData->fInDirNodeInfoTypeList );
		
		if ( cfAttributes != NULL )
		{
#ifndef __OBJC__
			cfNodeInfo = pContext->fVirtualNode->CopyNodeInfo( cfAttributes );
#else
			cfNodeInfo = (CFMutableDictionaryRef) [pContext->fVirtualNode copyNodeInfo: (NSArray *)cfAttributes];
#endif
		}
		
		DSCFRelease( cfAttributes );
	}
	
	if ( cfNodeInfo != NULL && CFDictionaryGetCount(cfNodeInfo) > 0 )
	{
		CFMutableArrayRef cfValues = CFArrayCreateMutable( kCFAllocatorDefault, 1, &kCFTypeArrayCallBacks );
		CFDictionaryRef cfAttributes = (CFDictionaryRef) CFDictionaryGetValue( cfNodeInfo, kBDPIAttributeKey );
		
		// safety - for plugins not filling these fields
		if ( CFDictionaryContainsKey(cfNodeInfo, kBDPINameKey) == false ) {
			CFDictionarySetValue( cfNodeInfo, kBDPINameKey, CFSTR("DirectoryNodeInfo") );
		}
		
		if ( CFDictionaryContainsKey(cfNodeInfo, kBDPITypeKey) == false ) {
			CFDictionarySetValue( cfNodeInfo, kBDPITypeKey, CFSTR(kDSStdRecordTypeDirectoryNodeInfo) );
		}
		
		CFArrayAppendValue( cfValues, cfNodeInfo );

		SInt32 count = FillBuffer( cfValues, inData->fOutDataBuff );
		if ( count == 0 ) {
			siResult = eDSBufferTooSmall;
		}
		
		inData->fOutAttrInfoCount = CFDictionaryGetCount( cfAttributes );

		DSCFRelease( cfValues );
	}
	
	EXCEPTION_END
	
failure:

	DSCFRelease( cfNodeInfo );
	
    return( (tDirStatus)siResult );
}

#pragma mark -
#pragma mark -------- Authentication Functions ----------

tDirStatus BaseDirectoryPlugin::DoAuthentication( sDoDirNodeAuth *inData, const char *inRecTypeStr, CDSAuthParams &inParams )
{
    tDirStatus			siResult		= eDSAuthMethodNotSupported;
    UInt32				uiAuthMethod	= kAuthUnknownMethod;
    sBDPINodeContext	*pContext		= NULL;
	CFStringRef			cfRecordType	= (inRecTypeStr != NULL ? CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, inRecTypeStr, kCFStringEncodingUTF8, kCFAllocatorNull) : CFSTR(kDSStdRecordTypeUsers));
	CFStringRef			cfAuthMethod	= NULL;

	pContext = (sBDPINodeContext *) fContextHash->GetItemData( inData->fInNodeRef );
	if ( pContext == NULL )
	{
		siResult = eDSBadContextData;
		goto failure;
	}
	
	if ( pContext->fType != kBDPIDirNode )
	{
		siResult = eDSInvalidNodeRef;
		goto failure;
	}
	
	if ( inData->fType == kDoDirNodeAuthOnRecordType )
	{
		DSCFRelease( cfRecordType );
		cfRecordType = CFStringCreateWithCStringNoCopy( kCFAllocatorDefault, ((sDoDirNodeAuthOnRecordType *)inData)->fInRecordType->fBufferData, 
													    kCFStringEncodingUTF8, kCFAllocatorNull );
	}

	cfAuthMethod = CFStringCreateWithCStringNoCopy( kCFAllocatorDefault, inData->fInAuthMethod->fBufferData, kCFStringEncodingUTF8,
												    kCFAllocatorNull );

	siResult = dsGetAuthMethodEnumValue( inData->fInAuthMethod, &uiAuthMethod );
	if ( siResult == eDSNoErr || siResult == eDSAuthMethodNotSupported )
	{
		switch( uiAuthMethod )
		{
			case kAuthNativeNoClearText:
			case kAuthNativeClearTextOK:
			case kAuthClearText:
			case kAuthNativeRetainCredential:
				siResult = DoSimpleAuthentication( pContext, cfRecordType, inData->fInAuthStepData, inData->fInDirNodeAuthOnlyFlag );
				break;
			case kAuthSetPasswdAsRoot:
			case kAuthChangePasswd:
				siResult = DoSimplePasswordChange( pContext, cfRecordType, inData->fInAuthStepData );
				break;
			case kAuthGetPolicy:
			case kAuthGetEffectivePolicy:
			case kAuthGetGlobalPolicy:
				siResult = DoAuthGetPolicy( pContext, cfRecordType, uiAuthMethod, inData->fInAuthStepData, inData->fOutAuthStepDataResponse );
				break;
			default:
				siResult = DoExtendedAuthentication( pContext, cfRecordType, cfAuthMethod, inData->fInAuthStepData, 
													 inData->fOutAuthStepDataResponse, inData->fInDirNodeAuthOnlyFlag );
				break;
		}
	}
	
failure:
	
	DSCFRelease( cfAuthMethod );
	DSCFRelease( cfRecordType );

    return( siResult );
}

tDirStatus BaseDirectoryPlugin::DoAuthGetPolicy( sBDPINodeContext *inContext, CFStringRef inRecordType, unsigned int uiMethod, tDataBuffer *inAuthData, tDataBuffer *outAuthResponse )
{
	CFStringRef	pAdminName	= NULL;
	CFStringRef	pAdminPass	= NULL;
	CFStringRef	pUsername	= NULL;
	char		*bufferPtr	= inAuthData->fBufferData;
    UInt32      bytesLeft	= inAuthData->fBufferLength;
	tDirStatus	siResult	= eDSAuthUnknownUser;
	
	EXCEPTION_START
	
	UInt32 lLength = *((UInt32 *)bufferPtr);
    bufferPtr += sizeof(UInt32);
    bytesLeft -= sizeof(UInt32);
    if( lLength > bytesLeft ) return eDSInvalidBuffFormat;
    
	// if GetPolicy we have 3 parts to parse.. otherwise just get the name below
	if( kAuthGetPolicy == uiMethod )
	{
		if( lLength != 0 )
		{
			pAdminName = CFStringCreateWithBytes( kCFAllocatorDefault, (UInt8 *)bufferPtr, lLength, kCFStringEncodingUTF8, FALSE );

			bufferPtr += lLength;
		}
		
		lLength = *((UInt32 *)bufferPtr);
		bufferPtr += sizeof(UInt32);
		bytesLeft -= sizeof(UInt32);
		if( lLength > bytesLeft ) return eDSInvalidBuffFormat;
		
		if( lLength != 0 )
		{
			pAdminPass = CFStringCreateWithBytes( kCFAllocatorDefault, (UInt8 *)bufferPtr, lLength, kCFStringEncodingUTF8, FALSE );
			bufferPtr += lLength;
		}
		
		lLength = *((UInt32 *)bufferPtr);
		bufferPtr += sizeof(UInt32);
		bytesLeft -= sizeof(UInt32);
		if( lLength > bytesLeft ) return eDSInvalidBuffFormat;
		
	}
	
	if( lLength != 0 )
	{
		pUsername = CFStringCreateWithBytes( kCFAllocatorDefault, (UInt8 *)bufferPtr, lLength, kCFStringEncodingUTF8, FALSE );
        bufferPtr += lLength;
	}
	
    // if we got a username
	if( pUsername != NULL && CFStringGetLength(pUsername) != 0 )
	{
		// we don't use name and password yet...
#ifndef __OBJC__
		CFDictionaryRef	userPolicy = inContext->fVirtualNode->CopyPasswordPolicyForRecord( inRecordType, pUsername );
#else
		CFDictionaryRef	userPolicy = (CFDictionaryRef) [inContext->fVirtualNode copyPasswordPolicyForRecord: (NSString *) inRecordType withName: (NSString *)pUsername];
#endif
		
		if ( userPolicy != NULL )
		{
			CFDataRef	xmlData = CFPropertyListCreateXMLData( kCFAllocatorDefault, userPolicy );
			
			if( xmlData )
			{
				char    *policyString = NULL;
				
				ConvertXMLPolicyToSpaceDelimited( (const char *) CFDataGetBytePtr(xmlData), &policyString );
				
				if ( policyString != NULL )
				{
					u_int32_t stringLen = strlen( policyString );
					if( stringLen + 4 <= outAuthResponse->fBufferSize )
					{
						*((u_int32_t *)outAuthResponse->fBufferData) = stringLen;
						bcopy( policyString, (outAuthResponse->fBufferData+4), stringLen );
						outAuthResponse->fBufferLength = stringLen + 4;
						siResult = eDSNoErr;
					}
					else
					{
						siResult = eDSBufferTooSmall;
					}
					
					DSFree( policyString );
				}
				
				DSCFRelease( xmlData );
			}
		}
	}
	
	EXCEPTION_END
	
	return siResult;
}

tDirStatus BaseDirectoryPlugin::DoSimpleAuthentication( sBDPINodeContext *inContext, CFStringRef inRecordType, tDataBuffer *inAuthData, bool bAuthOnly )
{
    SInt32			siResult	= eDSAuthFailed;
    CFStringRef		cfUserName	= NULL;
    CFStringRef		cfPassword	= NULL;
	UInt32			bytesLeft	= inAuthData->fBufferLength;
	UInt32			stringLen	= 0;

	EXCEPTION_START
	
	char *pData = inAuthData->fBufferData;
	if ( pData == NULL )
	{
		siResult = eDSNullDataBuff;
		goto failure;
	}
	
	// Get the length of the user name
	if ( bytesLeft < sizeof(SInt32) )
	{
		siResult = eDSInvalidBuffFormat;
		goto failure;
	}
	
	stringLen = *((SInt32 *)pData);
	pData += sizeof(SInt32);
    bytesLeft -= sizeof(SInt32);
	if ( bytesLeft < stringLen )
	{
		siResult = eDSInvalidBuffFormat;
		goto failure;
	}
	
	cfUserName = CFStringCreateWithBytes( kCFAllocatorDefault, (const UInt8 *) pData, stringLen, kCFStringEncodingUTF8, false );
	pData += stringLen;
	bytesLeft -= stringLen;
	
	// get the new password
	if ( bytesLeft < sizeof(SInt32) )
	{
		siResult = eDSInvalidBuffFormat;
		goto failure;
	}
	
	stringLen = *((SInt32 *)pData);
	pData += sizeof( SInt32 );
    bytesLeft -= sizeof(UInt32);
	if ( bytesLeft < stringLen )
	{
		siResult = eDSInvalidBuffFormat;
		goto failure;
	}
	
	cfPassword = CFStringCreateWithBytes( kCFAllocatorDefault, (const UInt8 *) pData, stringLen, kCFStringEncodingUTF8, false );

#ifndef __OBJC__
	if ( bAuthOnly )
		siResult = inContext->fVirtualNode->VerifyCredentials( inRecordType, cfUserName, cfPassword );
	else
		siResult = inContext->fVirtualNode->SetNodeCredentials( inRecordType, cfUserName, cfPassword );
#else
	if ( bAuthOnly )
		siResult = [inContext->fVirtualNode verifyCredentials: (NSString *)inRecordType recordName: (NSString *)cfUserName 
													 password: (NSString *)cfPassword];
	else
		siResult = [inContext->fVirtualNode setNodeCredentials: (NSString *)inRecordType recordName: (NSString *)cfUserName 
													  password: (NSString *)cfPassword];
#endif
	
	EXCEPTION_END
	
failure:
	
	DSCFRelease( cfUserName );
	DSCFRelease( cfPassword );
	
    return( (tDirStatus)siResult );
}

tDirStatus BaseDirectoryPlugin::DoExtendedAuthentication( sBDPINodeContext *inContext, CFStringRef inRecordType, CFStringRef inAuthMethod, 
													  tDataBuffer *inAuthData, tDataBuffer *inOutAuthResponse, bool bAuthOnly )
{
	return eDSAuthMethodNotSupported;
}

tDirStatus BaseDirectoryPlugin::DoSimplePasswordChange( sBDPINodeContext *inContext, CFStringRef inRecordType, tDataBuffer *inAuthData )
{
    SInt32			siResult		= eDSAuthFailed;
    CFStringRef		cfUserName		= NULL;
    CFStringRef		cfPassword1		= NULL;
    CFStringRef		cfPassword2		= NULL;
    UInt32			stringLen		= 0;
	UInt32			bytesLeft		= inAuthData->fBufferLength;
	
	EXCEPTION_START

	char *pData = inAuthData->fBufferData;
	if ( pData == NULL )
	{
		siResult = eDSNullDataBuff;
		goto failure;
	}
	
	// Get the length of the user name
	if ( bytesLeft < sizeof(SInt32) )
	{
		siResult = eDSInvalidBuffFormat;
		goto failure;
	}
	
	stringLen = *((SInt32 *)pData);
	pData += sizeof(SInt32);
    bytesLeft -= sizeof(SInt32);
	if ( bytesLeft < stringLen )
	{
		siResult = eDSInvalidBuffFormat;
		goto failure;
	}
	
	cfUserName = CFStringCreateWithBytes( kCFAllocatorDefault, (const UInt8 *) pData, stringLen, kCFStringEncodingUTF8, false );
	pData += stringLen;
	bytesLeft -= stringLen;
	
	// get the old password
	if ( bytesLeft < sizeof(SInt32) )
	{
		siResult = eDSInvalidBuffFormat;
		goto failure;
	}
	
	stringLen = *((SInt32 *)pData);
	pData += sizeof( SInt32 );
    bytesLeft -= sizeof(UInt32);
	if ( bytesLeft < stringLen )
	{
		siResult = eDSInvalidBuffFormat;
		goto failure;
	}
	
	cfPassword1 = CFStringCreateWithBytes( kCFAllocatorDefault, (const UInt8 *) pData, stringLen, kCFStringEncodingUTF8, false );
	pData += stringLen;
	bytesLeft -= stringLen;
	
	// get the new password
	if ( bytesLeft > sizeof(SInt32) )
	{
		stringLen = *((SInt32 *)pData);
		pData += sizeof( SInt32 );
		bytesLeft -= sizeof(UInt32);
		if ( bytesLeft < stringLen )
		{
			siResult = eDSInvalidBuffFormat;
			goto failure;
		}
		
		cfPassword2 = CFStringCreateWithBytes( kCFAllocatorDefault, (const UInt8 *) pData, stringLen, kCFStringEncodingUTF8, false );
	}

#ifndef __OBJC__
	siResult = inContext->fVirtualNode->ChangePassword( inRecordType, cfUserName, (cfPassword2 ? cfPassword1 : NULL), (cfPassword2 ? cfPassword2 : cfPassword1) );
#else
	siResult = [inContext->fVirtualNode changePassword: (NSString *)inRecordType recordName: (NSString *)cfUserName oldPassword: (NSString *)(cfPassword2 ? cfPassword1 : NULL) 
										   newPassword: (NSString *)(cfPassword2 ? cfPassword2 : cfPassword1)];
#endif
	
	EXCEPTION_END
	
failure:
	
	DSCFRelease( cfUserName );
	DSCFRelease( cfPassword1 );
	DSCFRelease( cfPassword2 );
	
    return( (tDirStatus)siResult );
}

tDirStatus BaseDirectoryPlugin::DoExtendedPasswordChange( sBDPINodeContext *inContext, tDataBuffer *inAuthData, tDataBuffer *inOutAuthResponse, bool bAuthOnly )
{
	return eDSNoErr;
}

#pragma mark -
#pragma mark --------- Searching for Records -----------

tDirStatus BaseDirectoryPlugin::GetRecordList( sGetRecordList *inData )
{
    SInt32						siResult	= eDSNoErr;
    sBDPISearchRecordsContext	*pContinue	= NULL;

	EXCEPTION_START
	
	// Node context data
	sBDPINodeContext *pContext = (sBDPINodeContext *) fContextHash->GetItemData( inData->fInNodeRef );
	if ( pContext == NULL || pContext->fVirtualNode == NULL )
	{
		siResult = eDSBadContextData;
		goto failure;
	}
	
	if ( pContext->fType != kBDPIDirNode )
	{
		siResult = eDSInvalidNodeRef;
		goto failure;
	}
	
	// These can be NULL according to documentation (fInRecNameList, fInRecTypeList, fInAttribTypeList)
	// Verify all the parameters
	if ( inData->fInDataBuff == NULL || inData->fInDataBuff->fBufferSize == 0 )
	{
		siResult = eDSEmptyBuffer;
		goto failure;
	}

	if ( inData->fIOContinueData )
	{
		pContinue = (sBDPISearchRecordsContext *) fContinueHash->GetPointer( inData->fIOContinueData );
		if ( pContinue ==  NULL )
		{
			siResult = eDSInvalidContinueData;
			goto failure;
		}
	}
	else
	{
		pContinue = (sBDPISearchRecordsContext *) MakeContextData( kBDPISearchRecords );
		
		pContinue->fRecordTypeList = CreateCFArrayFromList( inData->fInRecTypeList );
		pContinue->fAttributeType = CFSTR(kDSNAttrRecordName);
		pContinue->fPattMatchType = inData->fInPatternMatch;
		pContinue->fValueList = CreateCFArrayFromList( inData->fInRecNameList );
		pContinue->fReturnAttribList = CreateCFArrayFromList( inData->fInAttribTypeList );
		pContinue->fAttribsOnly = inData->fInAttribInfoOnly;
		pContinue->fIndex = 0;
		pContinue->fMaxRecCount = inData->fOutRecEntryCount;
		pContinue->fStateInfo = NULL;
		
		inData->fIOContinueData = fContinueHash->AddPointer( pContinue, inData->fInNodeRef );
	}
	
	inData->fOutRecEntryCount = 0;

#ifndef __OBJC__
	siResult = pContext->fVirtualNode->SearchRecords( pContinue, inData->fInDataBuff, &(inData->fOutRecEntryCount) );
#else
	siResult = [pContext->fVirtualNode searchRecords: pContinue buffer: inData->fInDataBuff outCount: &(inData->fOutRecEntryCount)];
#endif

	// if we have 0 records and no error or we got some kind of error
	if ( (inData->fOutRecEntryCount == 0 && siResult == eDSNoErr) || (siResult != eDSBufferTooSmall && siResult != eDSNoErr)  )
	{
		fContinueHash->RemoveContext( inData->fIOContinueData );
		inData->fIOContinueData = 0;
	}

	EXCEPTION_END
	
failure:
	
    return( (tDirStatus)siResult );
}

tDirStatus BaseDirectoryPlugin::DoAttributeValueSearch( sDoAttrValueSearch *inData )
{
	return this->DoAttributeValueSearchWithData( (sDoAttrValueSearchWithData *)inData );
}

tDirStatus BaseDirectoryPlugin::DoAttributeValueSearchWithData( sDoAttrValueSearchWithData *inData )
{
    SInt32						siResult	= eDSNoErr;
    sBDPINodeContext			*pContext	= NULL;
	sBDPISearchRecordsContext	*pContinue	= NULL;

	EXCEPTION_START
	
	// Node context data
	pContext = (sBDPINodeContext *) fContextHash->GetItemData( inData->fInNodeRef );
	if ( pContext == NULL )
	{
		siResult = eDSBadContextData;
		goto failure;
	}
	
	if ( pContext->fType != kBDPIDirNode )
	{
		siResult = eDSInvalidNodeRef;
		goto failure;
	}
	
	// Verify all the parameters
	if ( inData->fOutDataBuff == NULL || inData->fOutDataBuff->fBufferSize == 0 )
	{
		siResult = eDSNullDataBuff;
		goto failure;
	}

	if ( inData->fInRecTypeList == NULL )
	{
		siResult = eDSEmptyRecordTypeList;
		goto failure;
	}

	if ( inData->fInAttrType == NULL )
	{
		siResult = eDSEmptyAttributeType;
		goto failure;
	}

	// check to make sure context IN is the same as RefTable saved context
	if ( inData->fIOContinueData != 0 )
	{
		pContinue = (sBDPISearchRecordsContext *) fContinueHash->GetPointer( inData->fIOContinueData );
		if ( pContinue == NULL )
		{
			siResult = eDSInvalidContinueData;
			goto failure;
		}
	}
	else
	{
		CFArrayRef	cfAttrTypeRequest	= NULL;
		CFArrayRef	cfSearchValues		= NULL;

		// If this is ValueSearchWithData
		if ( inData->fType == kDoAttributeValueSearchWithData || inData->fType == kDoMultipleAttributeValueSearchWithData )
		{
			// Get the attribute list
			if ( inData->fInAttrTypeRequestList == NULL )
			{
				siResult = eDSEmptyAttributeTypeList;
				goto failure;
			}
			
			cfAttrTypeRequest = CreateCFArrayFromList( inData->fInAttrTypeRequestList );
		}
		else
		{
			cfAttrTypeRequest = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
		}

		if( inData->fType == kDoAttributeValueSearchWithData || inData->fType == kDoAttributeValueSearch )
		{
			// if this is a Single Value search
			CFTypeRef cfValue = CFStringCreateWithBytes( kCFAllocatorDefault, (UInt8 *)inData->fInPatt2Match->fBufferData, inData->fInPatt2Match->fBufferLength,
														 kCFStringEncodingUTF8, false );
			if ( cfValue == NULL ) {
				// TODO: plugins should handle binary data, but for now we will only use strings
				goto failure;
			}
			
			cfSearchValues = CFArrayCreate( kCFAllocatorDefault, (const void **) &cfValue, 1, &kCFTypeArrayCallBacks );
			
			CFRelease( cfValue );
		}
		else
		{
			// if this is a multi-value search, it is an tDataListPtr..
			cfSearchValues = CreateCFArrayFromList( ((sDoMultiAttrValueSearchWithData *)inData)->fInPatterns2MatchList );
		}
		
		pContinue = (sBDPISearchRecordsContext *) MakeContextData( kBDPISearchRecords );
		if ( pContinue == NULL )
		{
			siResult = eMemoryAllocError;
			goto failure;
		}

		pContinue->fPattMatchType = inData->fInPattMatchType;
		pContinue->fAttribsOnly = inData->fInAttrInfoOnly;
		pContinue->fReturnAttribList = cfAttrTypeRequest;
		pContinue->fAttributeType = CFStringCreateWithCString( kCFAllocatorDefault, inData->fInAttrType->fBufferData, kCFStringEncodingUTF8 );
		pContinue->fRecordTypeList = CreateCFArrayFromList( inData->fInRecTypeList );
		pContinue->fValueList = cfSearchValues;
		pContinue->fIndex = 0;
		pContinue->fMaxRecCount = inData->fOutMatchRecordCount;
		
		inData->fIOContinueData = fContinueHash->AddPointer( pContinue, inData->fInNodeRef );
	}
	
	inData->fOutMatchRecordCount = 0;
	
#ifndef __OBJC__
	siResult = pContext->fVirtualNode->SearchRecords( pContinue, inData->fOutDataBuff, &(inData->fOutMatchRecordCount) );
#else
	siResult = [pContext->fVirtualNode searchRecords: pContinue buffer: inData->fOutDataBuff outCount: &(inData->fOutMatchRecordCount)];
#endif
	
	if ( (inData->fOutMatchRecordCount == 0 && siResult == eDSNoErr) || (siResult != eDSBufferTooSmall && siResult != eDSNoErr) )
	{
		fContinueHash->RemoveContext( inData->fIOContinueData );
		inData->fIOContinueData = 0;
	}

	EXCEPTION_END
	
failure:
	
    return( (tDirStatus)siResult );
}

#pragma mark -
#pragma mark ------------ Record Handling -------------

tDirStatus BaseDirectoryPlugin::OpenRecord( sOpenRecord *inData )
{
    tDirStatus				siResult	= eDSNoErr;
    CFMutableDictionaryRef	pRecord		= NULL;
	CFStringRef				cfRecName	= NULL;
	CFStringRef				cfRecType	= NULL;
    sBDPINodeContext		*pContext;

	EXCEPTION_START
	
	pContext = (sBDPINodeContext *) fContextHash->GetItemData( inData->fInNodeRef );
	if ( pContext == NULL )
	{
		siResult = eDSBadContextData;
		goto failure;
	}
	
	if ( pContext->fType != kBDPIDirNode )
	{
		siResult = eDSInvalidNodeRef;
		goto failure;
	}

	if ( inData->fInRecType == NULL )
	{
		siResult = eDSNullRecType;
		goto failure;
	}
	
	if ( inData->fInRecName == NULL )
	{
		siResult = eDSNullRecName;
		goto failure;
	}
	
	if ( inData->fInRecType->fBufferLength == 0 )
	{
		siResult = eDSEmptyRecordNameList;
		goto failure;
	}

	if ( inData->fInRecName->fBufferLength == 0 )
	{
		siResult = eDSEmptyRecordTypeList;
		goto failure;
	}
	
	cfRecName = CFStringCreateWithCString( kCFAllocatorDefault, inData->fInRecName->fBufferData, kCFStringEncodingUTF8 );
	cfRecType = CFStringCreateWithCString( kCFAllocatorDefault, inData->fInRecType->fBufferData, kCFStringEncodingUTF8 );

#ifndef __OBJC__
	pRecord = pContext->fVirtualNode->RecordOpen( cfRecType, cfRecName );
#else
	pRecord = (CFMutableDictionaryRef) [pContext->fVirtualNode recordOpenName: (NSString *)cfRecName recordType: (NSString *)cfRecType];
#endif
	
	if( pRecord )
	{
		sBDPIRecordEntryContext *pRecContext = (sBDPIRecordEntryContext *) MakeContextData( kBDPIRecordEntry );
		
		pRecContext->fRecord = pRecord;
		pRecContext->fVirtualNode = pContext->fVirtualNode;

		fContextHash->AddItem( inData->fOutRecRef, pRecContext );
	}
	else
	{
		inData->fOutRecRef = 0;
		siResult = eDSRecordNotFound;
	}
	
	EXCEPTION_END
	
failure:
	
	DSCFRelease( cfRecName );
	DSCFRelease( cfRecType );
	
    return( siResult );
}

tDirStatus BaseDirectoryPlugin::CloseRecord( sCloseRecord *inData )
{
    tDirStatus	siResult	= eDSNoErr;

	sBDPIRecordEntryContext	*pContext = (sBDPIRecordEntryContext *) fContextHash->GetItemData( inData->fInRecRef );
	if ( pContext != NULL )
	{
        fContextHash->RemoveItem( inData->fInRecRef );
	}
	else
	{
		siResult = eDSBadContextData;
	}
	
    return( siResult );
}

tDirStatus BaseDirectoryPlugin::GetRecordEntry( sGetRecordEntry *inData )
{
	return eNotHandledByThisNode;
}

tDirStatus BaseDirectoryPlugin::GetRecRefInfo( sGetRecRefInfo *inData )
{
    tDirStatus      siResult	= eDSNoErr;
	int             uiOffset	= 0;
	char            *tmpRecName	= NULL;
	char            *tmpRecType	= NULL;
	CFDictionaryRef pRecord     = NULL;
	CFStringRef 	cfRecType   = NULL;
	CFStringRef 	cfRecName   = NULL;
	const char      *recName    = NULL;
	const char      *recType    = NULL;
	uint16_t        recNameLen  = 0;
	uint16_t        recTypeLen  = 0;
    tRecordEntry    *pRecEntry  = NULL;

	sBDPIRecordEntryContext *pContext = (sBDPIRecordEntryContext *) fContextHash->GetItemData( inData->fInRecRef );
	if ( pContext == NULL )
	{
		siResult = eDSBadContextData;
		goto failure;
	}

    pRecord = pContext->fRecord;

	cfRecName = (CFStringRef) CFDictionaryGetValue( pRecord, kBDPINameKey );
	recName = GetCStringFromCFString( cfRecName, &tmpRecName );
	recNameLen = strlen( recName );

    cfRecType = (CFStringRef) CFDictionaryGetValue( pRecord, kBDPITypeKey );
	recType = GetCStringFromCFString( cfRecType, &tmpRecType );
	recTypeLen = strlen( recType );

	pRecEntry = (tRecordEntry *) calloc( 1, sizeof(tRecordEntry) + recNameLen + recTypeLen + 4 + kBPDIBufferTax );
	if ( pRecEntry == NULL )
	{
		siResult = eMemoryAllocError;
		goto failure;
	}
	
	pRecEntry->fRecordNameAndType.fBufferSize	= recNameLen + recTypeLen + 4 + kBPDIBufferTax;
	pRecEntry->fRecordNameAndType.fBufferLength	= recNameLen + recTypeLen + 4;

	// Add the record name length and name itself
	memcpy( pRecEntry->fRecordNameAndType.fBufferData + uiOffset, &recNameLen, 2);
	uiOffset += 2;
	memcpy( pRecEntry->fRecordNameAndType.fBufferData + uiOffset, recName, recNameLen);
	uiOffset += recNameLen;

	// Add the record type length and type itself
	memcpy( pRecEntry->fRecordNameAndType.fBufferData + uiOffset, &recTypeLen, 2);
	uiOffset += 2;
	memcpy( pRecEntry->fRecordNameAndType.fBufferData + uiOffset, recType, recTypeLen);

	pRecEntry->fRecordAttributeCount = CFDictionaryGetCount( (CFDictionaryRef) CFDictionaryGetValue(pRecord, kBDPIAttributeKey) );

	inData->fOutRecInfo = pRecEntry;
	
failure:
	
	DSFreeString( tmpRecName );
	DSFreeString( tmpRecType );
	
    return( siResult );
}

tDirStatus BaseDirectoryPlugin::GetRecAttribInfo( sGetRecAttribInfo *inData )
{
    tDirStatus		siResult		= eDSNoErr;
	CFStringRef		cfAttributeName	= NULL;
	tAttributeEntry *pOutAttrEntry	= NULL;
	CFDictionaryRef	cfAttributes	= NULL;
	CFArrayRef		pValue			= NULL;
	CFStringRef		cfRecType		= NULL;

	EXCEPTION_START
	
	sBDPIRecordEntryContext *pContext = (sBDPIRecordEntryContext *) fContextHash->GetItemData( inData->fInRecRef );
	if ( pContext == NULL )
	{
		siResult = eDSBadContextData;
		goto failure;
	}
	
	cfAttributeName	= CFStringCreateWithCString( kCFAllocatorDefault, inData->fInAttrType->fBufferData, kCFStringEncodingUTF8 );
	cfAttributes = (CFDictionaryRef) CFDictionaryGetValue( pContext->fRecord, kBDPIAttributeKey );
	pValue = (CFArrayRef) CFDictionaryGetValue( cfAttributes, cfAttributeName );
	if ( pValue == NULL )
	{
		siResult = eDSInvalidAttributeType;
		goto failure;
	}

	unsigned int uiTypeLen = strlen( inData->fInAttrType->fBufferData );

	//set up the length of the attribute type
	pOutAttrEntry = (tAttributeEntry *) calloc( 1, sizeof(tAttributeEntry) + uiTypeLen + kBPDIBufferTax );
	if ( pOutAttrEntry == NULL )
	{
		siResult = eMemoryAllocError;
		goto failure;
	}

	pOutAttrEntry->fAttributeSignature.fBufferSize		= uiTypeLen + kBPDIBufferTax;
	pOutAttrEntry->fAttributeSignature.fBufferLength	= uiTypeLen;
	
	strlcpy( pOutAttrEntry->fAttributeSignature.fBufferData, inData->fInAttrType->fBufferData, pOutAttrEntry->fAttributeSignature.fBufferSize );

	// Number of attribute values
	pOutAttrEntry->fAttributeValueCount = CFArrayGetCount( pValue );

	//set the total length of all the attribute data
	pOutAttrEntry->fAttributeDataSize = 0;
	for ( unsigned int ii=0; ii < pOutAttrEntry->fAttributeValueCount; ii++ )
	{
		CFTypeRef cfValue = CFArrayGetValueAtIndex( pValue, ii );
		
		if ( CFGetTypeID(cfValue) == CFStringGetTypeID() )
		{
			char		*tmpStr		= NULL;
			const char	*pStrValue	= GetCStringFromCFString( (CFStringRef) cfValue, &tmpStr );
			
			pOutAttrEntry->fAttributeDataSize += strlen( pStrValue );

			DSFree( tmpStr );
		}
		else
		{
			pOutAttrEntry->fAttributeDataSize += CFDataGetLength( (CFDataRef) cfValue );
		}
	}

	// arbitrary max length
	cfRecType = (CFStringRef) CFDictionaryGetValue( pContext->fRecord, kBDPITypeKey );

#ifndef __OBJC__
	pOutAttrEntry->fAttributeValueMaxSize = pContext->fVirtualNode->MaximumSizeForAttribute( cfRecType, cfAttributeName );
#else
	pOutAttrEntry->fAttributeValueMaxSize = [pContext->fVirtualNode maximumSize: (NSString *)cfRecType forAttribute: (NSString *)cfAttributeName];
#endif

	EXCEPTION_END
	
	// Assign the result out
	inData->fOutAttrInfoPtr = pOutAttrEntry;
	
failure:
	
    return( siResult );
}

#pragma mark -
#pragma mark --------- Record Modifications ----------

tDirStatus BaseDirectoryPlugin::CreateRecord ( sCreateRecord *inData )
{
	tDirStatus	siResult	= eDSNoErr;
	CFStringRef	cfRecName	= NULL;
	CFStringRef	cfRecType	= NULL;

	EXCEPTION_START
	
	sBDPINodeContext *pContext = (sBDPINodeContext *) fContextHash->GetItemData( inData->fInNodeRef );
	if ( pContext == NULL )
	{
		siResult = eDSBadContextData;
		goto failure;
	}
	
	if ( pContext->fType != kBDPIDirNode )
	{
		siResult = eDSInvalidNodeRef;
		goto failure;
	}
	
	cfRecName = CFStringCreateWithCString( kCFAllocatorDefault, inData->fInRecName->fBufferData, kCFStringEncodingUTF8 );
	cfRecType = CFStringCreateWithCString( kCFAllocatorDefault, inData->fInRecType->fBufferData, kCFStringEncodingUTF8 );

#ifndef __OBJC__
	siResult = pContext->fVirtualNode->RecordCreate( cfRecType, cfRecName );
#else
	siResult = [pContext->fVirtualNode recordCreateName: (NSString *)cfRecName recordType: (NSString *)cfRecType];
#endif

	if( siResult == eDSNoErr && (inData->fType == kCreateRecordAndOpen || inData->fInOpen) )
	{
#ifndef __OBJC__
		CFMutableDictionaryRef	pRecord = pContext->fVirtualNode->RecordOpen( cfRecType, cfRecName );
#else
		CFMutableDictionaryRef	pRecord = (CFMutableDictionaryRef) [pContext->fVirtualNode recordOpenName: (NSString *)cfRecName recordType: (NSString *)cfRecType];
#endif
		if( pRecord )
		{
			sBDPIRecordEntryContext *pRecContext = (sBDPIRecordEntryContext *) MakeContextData(kBDPIRecordEntry);
			pRecContext->fRecord = pRecord;
			pRecContext->fVirtualNode = pContext->fVirtualNode;
			
			fContextHash->AddItem( inData->fOutRecRef, pRecContext );
		}
		else
		{
			inData->fOutRecRef = 0;
			siResult = eDSRecordNotFound;
		}
	}
	
	EXCEPTION_END
	
failure:
	
	DSCFRelease( cfRecName );
	DSCFRelease( cfRecType );

    return siResult;
}

tDirStatus BaseDirectoryPlugin::SetRecordType( sSetRecordType *inData )
{
	tDirStatus	siResult = eDSBadContextData;

	EXCEPTION_START
	
	sBDPIRecordEntryContext *pContext = (sBDPIRecordEntryContext *) fContextHash->GetItemData( inData->fInRecRef );
	if ( pContext != NULL )
	{
#ifndef __OBJC__
		if ( pContext->fVirtualNode->AllowChangesForAttribute(pContext->fRecord, CFSTR(kDSNAttrRecordType)) )
#else
		if ( [pContext->fVirtualNode allowChanges: (NSDictionary *)pContext->fRecord forAttribute: @kDSNAttrRecordType] )
#endif
		{
			CFStringRef	cfType = CFStringCreateWithCString( kCFAllocatorDefault, inData->fInNewRecType->fBufferData, kCFStringEncodingUTF8 );
			if ( cfType != NULL )
			{
#ifndef __OBJC__
				siResult = (tDirStatus) pContext->fVirtualNode->RecordSetType( pContext->fRecord, cfType );
#else
				siResult = (tDirStatus) [pContext->fVirtualNode record: (NSMutableDictionary *)pContext->fRecord setRecordType: (NSString *)cfType];
#endif
				
				CFRelease( cfType );
			}
			else
			{
				siResult = eDSEmptyRecordType;
			}
		}
		else
		{
			siResult = eDSNotAuthorized;
		}
	}
	
	EXCEPTION_END

    return siResult;
}

tDirStatus BaseDirectoryPlugin::DeleteRecord ( sDeleteRecord *inData )
{
	tDirStatus	siResult = eDSBadContextData;

	EXCEPTION_START
	
	sBDPIRecordEntryContext *pContext = (sBDPIRecordEntryContext *) fContextHash->GetItemData( inData->fInRecRef );
	if ( pContext != NULL )
	{
#ifndef __OBJC__
		siResult = (tDirStatus)pContext->fVirtualNode->RecordDelete( pContext->fRecord );
#else
		siResult = (tDirStatus)[pContext->fVirtualNode recordDelete: (NSDictionary *)pContext->fRecord];
#endif
		
		fContextHash->RemoveItem( inData->fInRecRef );
	}
	
	EXCEPTION_END

    return siResult;
}

tDirStatus BaseDirectoryPlugin::SetRecordName( sSetRecordName *inData )
{
	SInt32	siResult = eDSBadContextData;

	EXCEPTION_START
	
	sBDPIRecordEntryContext *pContext = (sBDPIRecordEntryContext *) fContextHash->GetItemData( inData->fInRecRef );
	if ( pContext != NULL )
	{
#ifndef __OBJC__
		if ( pContext->fVirtualNode->AllowChangesForAttribute(pContext->fRecord, CFSTR(kDSNAttrRecordName)) )
#else
		if ( [pContext->fVirtualNode allowChanges: (NSDictionary *)pContext->fRecord forAttribute: @kDSNAttrRecordName] )
#endif
		{
			CFStringRef	cfValue = CFStringCreateWithCString( kCFAllocatorDefault, inData->fInNewRecName->fBufferData, kCFStringEncodingUTF8 );
			if ( cfValue != NULL )
			{
				// need mutable object in case it changes...
				CFArrayRef cfValues = CFArrayCreate( kCFAllocatorDefault, (const void **) &cfValue, 1, &kCFTypeArrayCallBacks );
				if ( cfValues != NULL )
				{
#ifndef __OBJC__
					siResult = pContext->fVirtualNode->RecordSetValuesForAttribute( pContext->fRecord, CFSTR(kDSNAttrRecordName), cfValues );
#else
					siResult = [pContext->fVirtualNode record: (NSMutableDictionary *)pContext->fRecord setValues: (NSArray *) cfValues 
								                 forAttribute: @kDSNAttrRecordName];
#endif
					
					DSCFRelease( cfValues );
				}
				
				DSCFRelease( cfValue );
			}
			else
			{
				siResult = eDSEmptyRecordName;
			}
		}
		else
		{
			siResult = eDSNotAuthorized;
		}
		
	}
	
	EXCEPTION_END
	
    return (tDirStatus)siResult;
}

tDirStatus BaseDirectoryPlugin::FlushRecord( sFlushRecord *inData )
{
	tDirStatus	siResult = eDSNoErr;

	EXCEPTION_START
	
	sBDPIRecordEntryContext *pContext = (sBDPIRecordEntryContext *) fContextHash->GetItemData( inData->fInRecRef );
	if ( pContext != NULL )
	{
#ifndef __OBJC__
		pContext->fVirtualNode->RecordFlush( pContext->fRecord );
#else
		[pContext->fVirtualNode recordFlush: (NSDictionary *)pContext->fRecord];
#endif
	}
	else
	{
		siResult = eDSBadContextData;
	}
	
	EXCEPTION_END
	
	return siResult;
}

#pragma mark -
#pragma mark ---------- Attribute Retrieval -----------

tDirStatus BaseDirectoryPlugin::GetAttributeEntry( sGetAttributeEntry *inData )
{
    tDirStatus					siResult			= eDSInvalidIndex;
    tAttributeEntryPtr			pAttribInfo			= NULL;
    sBDPIRecordEntryContext	   *pRecordContext		= NULL;
    sBDPIAttributeEntryContext *pAttributeContext	= NULL;
	CFDictionaryRef				cfAttributes		= NULL;
	CFIndex						cfKeyCount			= 0;
	CFIndex						cfAttrIndex			= 0;

	EXCEPTION_START
	
	pRecordContext = (sBDPIRecordEntryContext *) fContextHash->GetItemData( inData->fInAttrListRef );
	if ( pRecordContext == NULL )
	{
		siResult = eDSBadContextData;
		goto failure;
	}
	
	if ( pRecordContext->fType != kBDPIRecordEntry )
	{
		siResult = eDSInvalidAttrListRef;
		goto failure;
	}
	
	if ( inData->fInAttrInfoIndex == 0 )
	{
		siResult = eDSInvalidIndex;
		goto failure;
	}
	
	cfAttributes = (CFDictionaryRef) CFDictionaryGetValue( pRecordContext->fRecord, kBDPIAttributeKey );
	cfKeyCount = CFDictionaryGetCount( cfAttributes );
	cfAttrIndex = inData->fInAttrInfoIndex - 1;
	if ( cfAttrIndex < cfKeyCount )
	{
		CFTypeRef	*cfKeyList		= (CFTypeRef *) calloc( cfKeyCount + 1, sizeof(CFTypeRef) );
		CFTypeRef	*cfValueList	= (CFTypeRef *) calloc( cfKeyCount + 1, sizeof(CFTypeRef) );
		
		CFDictionaryGetKeysAndValues( cfAttributes, cfKeyList, cfValueList );
		
		CFStringRef cfKey = (CFStringRef) cfKeyList[cfAttrIndex];
		CFArrayRef	cfValues = (CFArrayRef) cfValueList[cfAttrIndex];
		
		if ( cfValues != NULL && cfKey != NULL )
		{
			char		*tmpKey	= NULL;
			const char	*pKey	= GetCStringFromCFString( cfKey, &tmpKey );
			
			if ( pKey != NULL )
			{
				UInt32	keyLen = strlen( pKey );
				
				pAttribInfo = (tAttributeEntry *) calloc( 1, sizeof(tAttributeEntry) + keyLen + kBPDIBufferTax );
				
				if ( pAttribInfo != NULL )
				{
					CFIndex	valueCount = CFArrayGetCount( cfValues );
					UInt32	uiValueSize = 0;
					
					for( CFIndex ii = 0; ii < valueCount; ii++ )
					{
						CFTypeRef	cfValue = CFArrayGetValueAtIndex( cfValues, ii );
						
						if ( CFGetTypeID(cfValue) == CFStringGetTypeID() )
						{
							char		*tmpStr = NULL;
							const char	*pValue = GetCStringFromCFString( (CFStringRef) cfValue, &tmpStr );
							
							uiValueSize += strlen( pValue );
							
							DSFreeString( tmpStr );
						}
						else
						{
							uiValueSize += CFDataGetLength( (CFDataRef) cfValue );
						}
					}
					
					CFStringRef cfRecType = (CFStringRef) CFDictionaryGetValue( pRecordContext->fRecord, kBDPITypeKey );

					pAttribInfo->fAttributeValueCount				= valueCount;
					pAttribInfo->fAttributeDataSize					= uiValueSize;
#ifndef __OBJC__
					pAttribInfo->fAttributeValueMaxSize				= pRecordContext->fVirtualNode->MaximumSizeForAttribute( cfRecType, cfKey );
#else
					pAttribInfo->fAttributeValueMaxSize				= [pRecordContext->fVirtualNode maximumSize: (NSString *)cfRecType 
																								   forAttribute: (NSString*)cfKey];
#endif
					pAttribInfo->fAttributeSignature.fBufferSize	= keyLen + kBPDIBufferTax;
					pAttribInfo->fAttributeSignature.fBufferLength	= keyLen;
					
					strlcpy( pAttribInfo->fAttributeSignature.fBufferData, pKey, pAttribInfo->fAttributeSignature.fBufferSize );
					
					pAttributeContext = (sBDPIAttributeEntryContext *) MakeContextData( kBDPIAttributeEntry );
					if ( pAttributeContext != NULL )
					{
						pAttributeContext->fRecord = (CFMutableDictionaryRef) CFRetain( pRecordContext->fRecord );
						pAttributeContext->fAttributeName = (CFStringRef) CFRetain( cfKey );
						pAttributeContext->fAttributeValueList = CFArrayCreateMutableCopy( kCFAllocatorDefault, 0, cfValues );
						
						inData->fOutAttrInfoPtr = pAttribInfo;
						
						fContextHash->AddItem( inData->fOutAttrValueListRef, pAttributeContext );
						
						pAttribInfo = NULL;
						pAttributeContext = NULL;
						siResult = eDSNoErr;
					}
					else
					{
						DSFree( pAttribInfo );
						siResult = eMemoryAllocError;
					}
				}
				else
				{
					siResult = eMemoryAllocError;
				}
			}
			else
			{
				siResult = eMemoryAllocError;
			}
			
			DSFreeString( tmpKey );
		}
		else
		{
			siResult = eDSInvalidIndex;
		}
		
		DSFree( cfKeyList );
		DSFree( cfValueList );
	}
	else
	{
		siResult = eDSIndexOutOfRange;
	}

	EXCEPTION_END
	
failure:

	DSFree( pAttribInfo );
	DSFree( pAttributeContext );
	
    return( siResult );
}

tDirStatus BaseDirectoryPlugin::GetAttributeValue( sGetAttributeValue *inData )
{
    tDirStatus	siResult	= eDSInvalidIndex;
    CFIndex		cfIndex		= inData->fInAttrValueIndex - 1;

	sBDPIAttributeEntryContext *pAttributeContext = (sBDPIAttributeEntryContext *) fContextHash->GetItemData( inData->fInAttrValueListRef );
	if ( pAttributeContext == NULL )
	{
		siResult = eDSBadContextData;
		goto failure;
	}
	
	if ( pAttributeContext->fType != kBDPIAttributeEntry )
	{
		siResult = eDSInvalidAttrListRef;
		goto failure;
	}
	
	if ( inData->fInAttrValueIndex == 0 )
	{
		siResult = eDSInvalidIndex;
		goto failure;
	}

	// Skip to the value that we want
	if( cfIndex < CFArrayGetCount(pAttributeContext->fAttributeValueList) )
	{
		CFTypeRef	cfValue		= CFArrayGetValueAtIndex( pAttributeContext->fAttributeValueList, cfIndex );
		char		*pString	= NULL;
		const char	*pValue		= NULL;
		UInt32		valueLen	= 0;
		
		if ( CFGetTypeID(cfValue) == CFDataGetTypeID() )
		{
			pValue = (const char *) CFDataGetBytePtr( (CFDataRef) cfValue );
			valueLen = CFDataGetLength( (CFDataRef) cfValue );
		}
		else
		{
			pValue = GetCStringFromCFString( (CFStringRef) cfValue, &pString );
			valueLen = strlen( pString );
		}
		
		if ( pValue != NULL )
		{
			tAttributeValueEntry *pAttrValue = (tAttributeValueEntry *) calloc( 1, sizeof(tAttributeValueEntry) + valueLen + kBPDIBufferTax );
			if ( pAttrValue != NULL )
			{
				pAttrValue->fAttributeValueData.fBufferSize		= valueLen + kBPDIBufferTax;
				pAttrValue->fAttributeValueData.fBufferLength	= valueLen;
				pAttrValue->fAttributeValueID					= CalcCRCWithLength( pValue, valueLen );
				
				bcopy( pValue, pAttrValue->fAttributeValueData.fBufferData, valueLen );
				
				inData->fOutAttrValue = pAttrValue;
				
				siResult = eDSNoErr;
			}
			else
			{
				siResult = eMemoryAllocError;
			}
		}
		else
		{
			siResult = eMemoryAllocError;
		}
		
		DSFreeString( pString );
	}
	else
	{
		siResult = eDSIndexOutOfRange;
	}
	
failure:
	
    return( siResult );
}

tDirStatus BaseDirectoryPlugin::CloseAttributeList( sCloseAttributeList *inData )
{
    tDirStatus	siResult	= eDSInvalidAttrListRef;

	sBDPIRecordEntryContext *pContext = (sBDPIRecordEntryContext *) fContextHash->GetItemData( inData->fInAttributeListRef );
	if ( pContext != NULL && pContext->fType == kBDPIRecordEntry )
	{
		fContextHash->RemoveItem( inData->fInAttributeListRef );
		siResult = eDSNoErr;
	}
	
    return( siResult );
}

tDirStatus BaseDirectoryPlugin::CloseAttributeValueList( sCloseAttributeValueList *inData )
{
    tDirStatus	siResult = eDSInvalidAttrValueRef;

	sBDPIAttributeEntryContext *pContext = (sBDPIAttributeEntryContext *) fContextHash->GetItemData( inData->fInAttributeValueListRef );
	if ( pContext != NULL && pContext->fType == kBDPIRecordEntry )
	{
		fContextHash->RemoveItem( inData->fInAttributeValueListRef );
		siResult = eDSNoErr;
	}

    return( siResult );
}

tDirStatus BaseDirectoryPlugin::GetRecAttrValueByValue( sGetRecordAttributeValueByValue *inData )
{
    tDirStatus		siResult		= eDSAttributeValueNotFound;
	CFStringRef		cfAttributeName	= NULL;
	CFDictionaryRef	cfAttributes	= NULL;
	CFArrayRef		cfValues		= NULL;
	
	sBDPIRecordEntryContext *pContext = (sBDPIRecordEntryContext *) fContextHash->GetItemData( inData->fInRecRef );
	if ( pContext == NULL )
	{
		siResult = eDSBadContextData;
		goto failure;
	}
	
	cfAttributeName = CFStringCreateWithCString( kCFAllocatorDefault, inData->fInAttrType->fBufferData, kCFStringEncodingUTF8 );
	if ( cfAttributeName == NULL )
	{
		siResult = eDSEmptyAttribute;
		goto failure;
	}

	cfAttributes = (CFDictionaryRef) CFDictionaryGetValue( pContext->fRecord, kBDPIAttributeKey );
	cfValues = (CFArrayRef) CFDictionaryGetValue( cfAttributes, cfAttributeName );
	if ( cfValues != NULL )
	{
		CFTypeRef cfValue = CFStringCreateWithCString( kCFAllocatorDefault, inData->fInAttrValue->fBufferData, kCFStringEncodingUTF8 );
		if ( cfValue == NULL )
		{
			cfValue = CFDataCreate( kCFAllocatorDefault, (const UInt8 *)inData->fInAttrValue->fBufferData, inData->fInAttrValue->fBufferLength );
		}

		if ( CFArrayContainsValue(cfValues, CFRangeMake(0,CFArrayGetCount(cfValues)), cfValue) )
		{
			tAttributeValueEntry *pAttrValue = (tAttributeValueEntry *) calloc( 1, sizeof(tAttributeValueEntry) + 
																			    inData->fInAttrValue->fBufferLength + kBPDIBufferTax );
			if ( pAttrValue != NULL )
			{
				pAttrValue->fAttributeValueData.fBufferSize     = inData->fInAttrValue->fBufferLength + kBPDIBufferTax;
				pAttrValue->fAttributeValueData.fBufferLength   = inData->fInAttrValue->fBufferLength;
				pAttrValue->fAttributeValueID					= CalcCRCWithLength( inData->fInAttrValue->fBufferData, 
																					 inData->fInAttrValue->fBufferLength );

				bcopy( inData->fInAttrValue->fBufferData, pAttrValue->fAttributeValueData.fBufferData, inData->fInAttrValue->fBufferLength );
				
				inData->fOutEntryPtr = pAttrValue;
				siResult = eDSNoErr;
			}
			else
			{
				siResult = eMemoryAllocError;
			}
		}
		else
		{
			siResult = eDSAttributeNotFound;
		}
		
		DSCFRelease( cfValue );
	}
	else
	{
		siResult = eDSAttributeNotFound;
	}
	
failure:
	
	DSCFRelease( cfAttributeName );
	
    return( siResult );
}

tDirStatus BaseDirectoryPlugin::GetRecAttrValueByIndex( sGetRecordAttributeValueByIndex *inData )
{
    tDirStatus		siResult		= eDSIndexOutOfRange;
    CFIndex			cfIndex			= inData->fInAttrValueIndex - 1;
	CFStringRef		cfAttribName	= NULL;
	CFDictionaryRef	cfAttributes	= NULL;
	CFArrayRef		cfValues		= NULL;

	sBDPIRecordEntryContext *pContext = (sBDPIRecordEntryContext *) fContextHash->GetItemData( inData->fInRecRef );
	if ( pContext == NULL )
	{
		siResult = eDSBadContextData;
		goto failure;
	}
	
	cfAttribName = CFStringCreateWithCString( kCFAllocatorDefault, inData->fInAttrType->fBufferData, kCFStringEncodingUTF8 );
	if ( cfAttribName == NULL )
	{
		siResult = eDSEmptyAttributeType;
		goto failure;
	}
	
	if ( inData->fInAttrValueIndex == 0 )
	{
		siResult = eDSInvalidIndex;
		goto failure;
	}
	
	cfAttributes = (CFDictionaryRef) CFDictionaryGetValue( pContext->fRecord, kBDPIAttributeKey );
	if ( cfAttributes == NULL )
	{
		siResult = eDSAttributeNotFound;
		goto failure;
	}
	
	cfValues = (CFArrayRef) CFDictionaryGetValue( cfAttributes, cfAttribName );
	if ( cfValues == NULL )
	{
		siResult = eDSAttributeNotFound;
		goto failure;
	}
	
	if ( cfIndex < CFArrayGetCount(cfValues) )
	{
		CFTypeRef	cfValue		= CFArrayGetValueAtIndex( cfValues, cfIndex );
		char		*pString	= NULL;
		const char	*pValue		= NULL;
		UInt32		valueLen	= 0;
		
		if ( CFGetTypeID(cfValue) == CFDataGetTypeID() )
		{
			pValue = (const char *) CFDataGetBytePtr( (CFDataRef) cfValue );
			valueLen = CFDataGetLength( (CFDataRef) cfValue );
		}
		else
		{
			pValue = GetCStringFromCFString( (CFStringRef) cfValue, &pString );
			valueLen = strlen( pString );
		}
		
		if ( pValue != NULL )
		{
			tAttributeValueEntry *pAttrValue = (tAttributeValueEntry *) calloc( 1, sizeof(tAttributeValueEntry) + valueLen + kBPDIBufferTax );
			if ( pAttrValue != NULL )
			{
				pAttrValue->fAttributeValueData.fBufferSize		= valueLen + kBPDIBufferTax;
				pAttrValue->fAttributeValueData.fBufferLength	= valueLen;
				pAttrValue->fAttributeValueID					= CalcCRCWithLength( pValue, valueLen );
				
				bcopy( pValue, pAttrValue->fAttributeValueData.fBufferData, valueLen );
				
				inData->fOutEntryPtr = pAttrValue;
				
				siResult = eDSNoErr;
			}
			else
			{
				siResult = eMemoryAllocError;
			}
		}
		else
		{
			siResult = eMemoryAllocError;
		}
		
		DSFreeString( pString );
	}
	else
	{
		siResult = eDSIndexOutOfRange;
	}
	
failure:

    return( siResult );
}

tDirStatus BaseDirectoryPlugin::GetRecAttrValueByID( sGetRecordAttributeValueByID *inData )
{
    tDirStatus	siResult	= eDSBadContextData;

	sBDPIRecordEntryContext *pContext = (sBDPIRecordEntryContext *) fContextHash->GetItemData( inData->fInRecRef );
	if ( pContext != NULL )
	{
		CFStringRef	cfAttribute = CFStringCreateWithCString( kCFAllocatorDefault, inData->fInAttrType->fBufferData, kCFStringEncodingUTF8 );
		
		if ( cfAttribute != NULL )
		{
			CFDictionaryRef cfAttributes	= (CFDictionaryRef) CFDictionaryGetValue( pContext->fRecord, kBDPIAttributeKey );
			CFArrayRef		cfValues		= (CFArrayRef )CFDictionaryGetValue( cfAttributes, cfAttribute );
			
			if ( cfValues != NULL )
			{
				CFIndex	valueCount	= CFArrayGetCount( cfValues );
				bool	bFoundOrErr	= false;
				
				for( CFIndex ii = 0; ii < valueCount && bFoundOrErr == false; ii++ )
				{
					CFTypeRef	cfValue		= CFArrayGetValueAtIndex( cfValues, ii );
					char		*pString	= NULL;
					const char	*pValue		= NULL;
					UInt32		valueLen	= 0;
					
					if ( CFGetTypeID(cfValue) == CFDataGetTypeID() )
					{
						pValue = (const char *) CFDataGetBytePtr( (CFDataRef) cfValue );
						valueLen = CFDataGetLength( (CFDataRef) cfValue );
					}
					else
					{
						pValue = GetCStringFromCFString( (CFStringRef) cfValue, &pString );
						valueLen = strlen( pString );
					}
					
					if ( CalcCRCWithLength(pValue, valueLen) == inData->fInValueID )
					{
						tAttributeValueEntry *pAttrValue = (tAttributeValueEntry *) calloc( 1, sizeof(tAttributeValueEntry) + valueLen + kBPDIBufferTax );
						
						if ( pAttrValue != NULL )
						{
							pAttrValue->fAttributeValueData.fBufferSize		= valueLen + kBPDIBufferTax;
							pAttrValue->fAttributeValueData.fBufferLength	= valueLen;
							pAttrValue->fAttributeValueID					= inData->fInValueID;
							
							bcopy( pValue, pAttrValue->fAttributeValueData.fBufferData, valueLen );
							
							inData->fOutEntryPtr = pAttrValue;

							siResult = eDSNoErr;
							bFoundOrErr = true;
						}
						else
						{
							siResult = eMemoryAllocError;
							bFoundOrErr = true;
						}
					}
					
					DSFreeString( pString );
				}
			}
			else
			{
				siResult = eDSAttributeNotFound;
			}
			
			DSCFRelease( cfAttribute );
		}
		else
		{
			siResult = eDSEmptyAttributeType;
		}
	}
	
    return siResult;
}

#pragma mark -
#pragma mark -------- Attribute Modifications ---------

tDirStatus BaseDirectoryPlugin::AddAttribute( sAddAttribute *inData, const char *inRecTypeStr )
{
	SInt32	siResult = eDSNotAuthorized;

	EXCEPTION_START
	
	sBDPIRecordEntryContext *pContext = (sBDPIRecordEntryContext *) fContextHash->GetItemData( inData->fInRecRef );
	if ( pContext != NULL )
	{
		CFStringRef	cfNewAttribute = CFStringCreateWithCString( kCFAllocatorDefault, inData->fInNewAttr->fBufferData, kCFStringEncodingUTF8 );
		
		if ( cfNewAttribute != NULL )
		{
#ifndef __OBJC__
			if ( pContext->fVirtualNode->AllowChangesForAttribute(pContext->fRecord, cfNewAttribute) )
#else
			if ( [pContext->fVirtualNode allowChanges: (NSDictionary *)pContext->fRecord forAttribute: (NSString *)cfNewAttribute] )
#endif
			{
				CFMutableArrayRef	cfNewValue	= CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
				
				if ( cfNewValue != NULL )
				{
					if( inData->fInFirstAttrValue )
					{
						CFStringRef cfString = CFStringCreateWithBytes( kCFAllocatorDefault, (const UInt8 *) inData->fInFirstAttrValue->fBufferData, 
																	    inData->fInFirstAttrValue->fBufferLength, kCFStringEncodingUTF8, false );
						
						if ( cfString != NULL )
						{
							CFArrayAppendValue( cfNewValue, cfString );
							DSCFRelease( cfString );
						}
						else
						{
							CFDataRef cfData = CFDataCreate( kCFAllocatorDefault, (const UInt8 *) inData->fInFirstAttrValue->fBufferData, 
															 inData->fInFirstAttrValue->fBufferLength );
							
							if ( cfData != NULL )
							{
								CFArrayAppendValue( cfNewValue, cfData );
								DSCFRelease( cfData );
							}
							else
							{
								siResult = eMemoryAllocError;
								DSCFRelease( cfNewValue );
							}
						}
					}

					if ( cfNewValue != NULL )
					{
#ifndef __OBJC__
						siResult = pContext->fVirtualNode->RecordSetValuesForAttribute( pContext->fRecord, cfNewAttribute, cfNewValue );
#else
						siResult = [pContext->fVirtualNode record: (NSMutableDictionary *)pContext->fRecord 
														setValues: (NSArray *) cfNewValue
													 forAttribute: (NSString *)cfNewAttribute];
#endif
					}
					
					DSCFRelease( cfNewValue );
				}
				else
				{
					siResult = eMemoryAllocError;
				}
			}
		}
		else
		{
			siResult = eDSEmptyAttributeType;
		}
	}
	else
	{
		siResult = eDSBadContextData;
	}
	
	EXCEPTION_END
	
    return (tDirStatus)siResult;
}

tDirStatus BaseDirectoryPlugin::RemoveAttribute( sRemoveAttribute *inData, const char *inRecTypeStr )
{
	SInt32	siResult = eDSNotAuthorized;

	EXCEPTION_START
	
	sBDPIRecordEntryContext *pContext = (sBDPIRecordEntryContext *) fContextHash->GetItemData( inData->fInRecRef );
	if ( pContext != NULL )
	{
		CFStringRef		cfAttribName = CFStringCreateWithCString( kCFAllocatorDefault, inData->fInAttribute->fBufferData,
																  kCFStringEncodingUTF8 );
		
		if ( cfAttribName != NULL )
		{
#ifndef __OBJC__
			if ( pContext->fVirtualNode->AllowChangesForAttribute(pContext->fRecord, cfAttribName) )
#else
			if ( [pContext->fVirtualNode allowChanges: (NSDictionary *)pContext->fRecord forAttribute: (NSString *)cfAttribName] )
#endif
			{
				CFMutableArrayRef	cfEmptyArray = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
				
				if ( cfEmptyArray != NULL )
#ifndef __OBJC__
					siResult = pContext->fVirtualNode->RecordSetValuesForAttribute( pContext->fRecord, cfAttribName, cfEmptyArray );
#else
					siResult = [pContext->fVirtualNode record: (NSMutableDictionary *)pContext->fRecord setValues: (NSArray *)cfEmptyArray
												 forAttribute: (NSString *)cfAttribName];
#endif
				else
					siResult = eMemoryAllocError;
				
				DSCFRelease( cfEmptyArray );
			}
			
			DSCFRelease( cfAttribName );
		}
		else
		{
			siResult = eMemoryAllocError;
		}
	}
	else
	{
		siResult = eDSBadContextData;
	}
	
	EXCEPTION_END
	
    return (tDirStatus)siResult;
}

tDirStatus BaseDirectoryPlugin::AddAttributeValue ( sAddAttributeValue *inData, const char *inRecTypeStr )
{
	SInt32	siResult = eDSNotAuthorized;

	EXCEPTION_START
	
	sBDPIRecordEntryContext *pContext = (sBDPIRecordEntryContext *) fContextHash->GetItemData( inData->fInRecRef );
	if ( pContext != NULL )
	{
		CFStringRef		cfAttribName = CFStringCreateWithCString( kCFAllocatorDefault, inData->fInAttrType->fBufferData,
																  kCFStringEncodingUTF8 );
		
		if ( cfAttribName != NULL )
		{
#ifndef __OBJC__
			if ( pContext->fVirtualNode->AllowChangesForAttribute(pContext->fRecord, cfAttribName) )
#else
			if ( [pContext->fVirtualNode allowChanges: (NSDictionary *)pContext->fRecord forAttribute:(NSString *)cfAttribName] )
#endif
			{
				CFMutableArrayRef	cfNewValues = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
				
				if ( cfNewValues != NULL )
				{
					CFStringRef cfString = CFStringCreateWithBytes( kCFAllocatorDefault, (const UInt8 *) inData->fInAttrValue->fBufferData, 
																    inData->fInAttrValue->fBufferLength, kCFStringEncodingUTF8, false );
					
					if ( cfString != NULL )
					{
						CFArrayAppendValue( cfNewValues, cfString );
						DSCFRelease( cfString );
					}
					else
					{
						CFDataRef cfData = CFDataCreate( kCFAllocatorDefault, (const UInt8 *) inData->fInAttrValue->fBufferData, 
														 inData->fInAttrValue->fBufferLength );
						
						if ( cfData != NULL )
						{
							CFArrayAppendValue( cfNewValues, cfData );
							DSCFRelease( cfData );
						}
						else
						{
							siResult = eMemoryAllocError;
							DSCFRelease( cfNewValues );
						}
					}
					
					if ( cfNewValues != NULL )
					{
#ifndef __OBJC__
						siResult = pContext->fVirtualNode->RecordAddValuesToAttribute( pContext->fRecord, cfAttribName, cfNewValues );
#else
						siResult = [pContext->fVirtualNode record: (NSMutableDictionary *)pContext->fRecord 
														addValues: (NSArray *)cfNewValues
													  toAttribute: (NSString *)cfAttribName];
#endif
					}
				}
				else
				{
					siResult = eMemoryAllocError;
				}
				
				DSCFRelease( cfNewValues );
			}
			
			DSCFRelease( cfAttribName );
		}
		else
		{
			siResult = eMemoryAllocError;
		}
	}
	else
	{
		siResult = eDSBadContextData;
	}
	
	EXCEPTION_END
	
    return (tDirStatus)siResult;
}

tDirStatus BaseDirectoryPlugin::RemoveAttributeValue( sRemoveAttributeValue *inData, const char *inRecTypeStr )
{
	SInt32	siResult = eDSAttributeDoesNotExist;

	EXCEPTION_START
	
	sBDPIRecordEntryContext *pContext = (sBDPIRecordEntryContext *) fContextHash->GetItemData( inData->fInRecRef );
	if ( pContext != NULL )
	{
		CFStringRef	cfAttribute = CFStringCreateWithCString( kCFAllocatorDefault, inData->fInAttrType->fBufferData, kCFStringEncodingUTF8 );
		
		if ( cfAttribute != NULL )
		{
			CFDictionaryRef cfAttributes	= (CFDictionaryRef) CFDictionaryGetValue( pContext->fRecord, kBDPIAttributeKey );
			CFArrayRef		cfValues		= (CFArrayRef )CFDictionaryGetValue( cfAttributes, cfAttribute );
			
			if ( cfValues != NULL )
			{
				CFIndex	valueCount	= CFArrayGetCount( cfValues );
				bool	bFoundOrErr	= false;
				
				for( CFIndex ii = 0; ii < valueCount && bFoundOrErr == false; ii++ )
				{
					CFTypeRef	cfValue		= CFArrayGetValueAtIndex( cfValues, ii );
					char		*pString	= NULL;
					const char	*pValue		= NULL;
					UInt32		valueLen	= 0;
					
					if ( CFGetTypeID(cfValue) == CFDataGetTypeID() )
					{
						pValue = (const char *) CFDataGetBytePtr( (CFDataRef) cfValue );
						valueLen = CFDataGetLength( (CFDataRef) cfValue );
					}
					else
					{
						pValue = GetCStringFromCFString( (CFStringRef) cfValue, &pString );
						valueLen = strlen( pString );
					}
					
					if ( CalcCRCWithLength(pValue, valueLen) == inData->fInAttrValueID )
					{
#ifndef __OBJC__
						siResult = pContext->fVirtualNode->RecordRemoveValueFromAttribute( pContext->fRecord, cfAttribute, cfValue );
#else
						siResult = [pContext->fVirtualNode record: (NSMutableDictionary *)pContext->fRecord
													  removeValue: (id)cfValue
													fromAttribute: (NSString *)cfAttribute];
#endif
						bFoundOrErr = true;
					}
					
					DSFreeString( pString );
				}
			}
		}
	}
	else
	{
		siResult = eDSBadContextData;
	}
	
	EXCEPTION_END
	
    return (tDirStatus)siResult;
}

tDirStatus BaseDirectoryPlugin::SetAttributeValue ( sSetAttributeValue *inData, const char *inRecTypeStr )
{
	SInt32			siResult	= eDSAttributeDoesNotExist;
	CFStringRef		cfAttribute	= NULL;
	
	EXCEPTION_START
	
	sBDPIRecordEntryContext *pContext = (sBDPIRecordEntryContext *) fContextHash->GetItemData( inData->fInRecRef );
	if ( pContext == NULL )
	{
		siResult = eDSBadContextData;
		goto failure;
	}
	
	cfAttribute = CFStringCreateWithCString( kCFAllocatorDefault, inData->fInAttrType->fBufferData, kCFStringEncodingUTF8 );
	if ( cfAttribute == NULL )
	{
		siResult = eDSEmptyAttributeType;
		goto failure;
	}
	
#ifndef __OBJC__
	if ( pContext->fVirtualNode->AllowChangesForAttribute(pContext->fRecord, cfAttribute) )
#else
	if ( [pContext->fVirtualNode allowChanges: (NSDictionary *)pContext->fRecord forAttribute: (NSString *)cfAttribute] )
#endif
	{
		// Need Mutable array in case the attribute changes..
		CFMutableArrayRef	cfValues	= NULL;
		
		if ( inData->fType == kSetAttributeValue )
		{
			CFDictionaryRef cfAttributes	= (CFDictionaryRef) CFDictionaryGetValue( pContext->fRecord, kBDPIAttributeKey );
			CFArrayRef		cfOldValues		= (CFArrayRef )CFDictionaryGetValue( cfAttributes, cfAttribute );
			UInt32			valueID			= inData->fInAttrValueEntry->fAttributeValueID;

			if ( cfOldValues != NULL )
			{
				cfValues = CFArrayCreateMutableCopy( kCFAllocatorDefault, 0, cfOldValues );
				
				// we need to loop through the values and find the value we are replacing
				CFIndex	cfCount		= CFArrayGetCount( cfValues );
				bool	bFoundOrErr	= false;
				
				for( CFIndex ii = 0; ii < cfCount && bFoundOrErr == false; ii++ )
				{
					CFTypeRef	cfValue		= CFArrayGetValueAtIndex( cfValues, ii );
					char		*pString	= NULL;
					const char	*pValue		= NULL;
					UInt32		valueLen	= 0;
					
					if ( CFGetTypeID(cfValue) == CFDataGetTypeID() )
					{
						pValue = (const char *) CFDataGetBytePtr( (CFDataRef) cfValue );
						valueLen = CFDataGetLength( (CFDataRef) cfValue );
					}
					else
					{
						pValue = GetCStringFromCFString( (CFStringRef) cfValue, &pString );
						valueLen = strlen( pString );
					}
					
					if ( CalcCRCWithLength(pValue, valueLen) == valueID )
					{
						CFStringRef cfNewValue = CFStringCreateWithBytes( kCFAllocatorDefault, 
																		 (const UInt8 *) inData->fInAttrValueEntry->fAttributeValueData.fBufferData, 
																		 inData->fInAttrValueEntry->fAttributeValueData.fBufferLength,
																		 kCFStringEncodingUTF8, false );
						
						if ( cfNewValue != NULL )
						{
							CFDataRef cfData = CFDataCreate( kCFAllocatorDefault, 
															 (const UInt8 *) inData->fInAttrValueEntry->fAttributeValueData.fBufferData,
															 inData->fInAttrValueEntry->fAttributeValueData.fBufferLength );
							if ( cfData != NULL )
							{
								CFArraySetValueAtIndex( cfValues, ii, cfData );
								DSCFRelease( cfData );
							}
							else
							{
								siResult = eMemoryAllocError;
								DSCFRelease( cfValues );
							}
						}
						else
						{
							CFArraySetValueAtIndex( cfValues, ii, cfNewValue );
							DSCFRelease( cfNewValue );
						}
						
						bFoundOrErr = true;
					}
					
					DSFreeString( pString );
				}
				
				// if not found
				if ( bFoundOrErr == false )
					DSCFRelease( cfValues );
			}
			else
			{
				siResult = eDSAttributeValueNotFound;
				goto failure;
			}
		}
		else
		{
			// here let's just get all values..., since we are doing a set, we don't change values, so it's ok to cast
			cfValues = (CFMutableArrayRef) CreateCFArrayFromList( ((sSetAttributeValues *)inData)->fInAttrValueList );
		}
		
#ifndef __OBJC__
		if ( cfValues != NULL )
			siResult = pContext->fVirtualNode->RecordSetValuesForAttribute( pContext->fRecord, cfAttribute, cfValues );
		else
			siResult = eDSAttributeValueNotFound;
#else
		if ( cfValues != NULL )
			siResult = [pContext->fVirtualNode record: (NSMutableDictionary *)pContext->fRecord
											setValues: (NSArray *)cfValues
										 forAttribute: (NSString *)cfAttribute];
		else
			siResult = eDSAttributeValueNotFound;
#endif
		
		DSCFRelease( cfValues );
	}
	
	EXCEPTION_END
	
failure:
	
    return (tDirStatus)siResult;
}

#pragma mark -
#pragma mark ---------- Utility Functions -----------

CFArrayRef BaseDirectoryPlugin::CreateAuthArrayFromBuffer( tDataBufferPtr inAuthBuff )
{
	UInt32				itemLen		= 0;
	UInt32				offset		= 0;
	tDirStatus			tResult		= eDSNoErr;
	
	if ( inAuthBuff == NULL )
		return NULL;
	
	char	*pData		= inAuthBuff->fBufferData;
	UInt32	buffSize	= inAuthBuff->fBufferSize;
	UInt32	buffLen		= inAuthBuff->fBufferLength;
	
	if ( buffLen > buffSize )
		return NULL;
	
	CFMutableArrayRef cfResultArray = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
	if ( cfResultArray == NULL )
	{
		DbgLog( kLogPlugin, "*** %s: File: %s. Line: %d. Error = %d.\n", fPlugInName, __FILE__, __LINE__ );
		return NULL;
	}
	
	while ( (offset < buffLen) && (tResult == eDSNoErr) )
	{
		if ( offset + sizeof(UInt32) > buffLen )
		{
			tResult = eDSInvalidBuffFormat;
			break;
		}
		
		memcpy( &itemLen, pData, sizeof(UInt32) );
		pData += sizeof(UInt32);
		offset += sizeof(UInt32);
		if ( itemLen + offset > buffLen )
		{
			tResult = eDSInvalidBuffFormat;
			break;
		}
		
		CFDataRef cfTempData = CFDataCreateWithBytesNoCopy( kCFAllocatorDefault, (const UInt8 *)pData, itemLen, kCFAllocatorNull );
		if ( cfTempData != NULL )
		{
			CFArrayAppendValue( cfResultArray, cfTempData );
		}
		else
		{
			DbgLog( kLogPlugin, "*** %s: File: %s. Line: %d. Error = %d.\n", fPlugInName, __FILE__, __LINE__ );
			tResult = eMemoryAllocError;
		}
	}
	
	// clean up if there was an errors
	if ( tResult != eDSNoErr )
		DSCFRelease( cfResultArray );
	
	return cfResultArray;
}

const char *BaseDirectoryPlugin::GetCStringFromCFString( CFStringRef inCFString, char **outCString )
{
	if ( outCString != NULL )
		(*outCString) = NULL;
	
	if ( inCFString == NULL )
		return NULL;
	
	const char* cStringPtr = CFStringGetCStringPtr( inCFString, kCFStringEncodingUTF8 );
	if ( cStringPtr != NULL || outCString == NULL )
		return cStringPtr;
	
	CFIndex maxCStrLen = CFStringGetMaximumSizeForEncoding( CFStringGetLength(inCFString), kCFStringEncodingUTF8 ) + 1;
	(*outCString) = (char *) malloc( maxCStrLen );
	
	if ( CFStringGetCString( inCFString, (*outCString), maxCStrLen, kCFStringEncodingUTF8) == false )
	{
		DbgLog( kLogPlugin, "BaseDirectoryPlugin::GetCStringFromCFString call failed!" );
		free( (*outCString) );
		(*outCString) = NULL;
	}
	
	return (*outCString);
}

tDirStatus BaseDirectoryPlugin::FillBuffer( CFMutableArrayRef inRecordList, BDPIOpaqueBuffer inBuffer )
{
    // Lets get ready to fill the buffer
	tDataBufferPtr	inDataBuff			= (tDataBufferPtr) inBuffer;
    char			*bufferStart		= inDataBuff->fBufferData;
    char			*bufferLoc			= bufferStart;
    UInt32			bufferOffset		= inDataBuff->fBufferSize;
    UInt32			buffLeft			= inDataBuff->fBufferSize - 12;  // Skip the buff StartTag + Record Count + EndTag...
    UInt32			*numRecords			= (UInt32 *)(bufferStart + 4);
    UInt32			startTag			= 'StdA';
    UInt32			endTag				= 'EndT';
    UInt32			outRecEntryCount	= 0;

	if ( inRecordList != NULL && CFArrayGetCount(inRecordList) > 0 )
	{
        // make buffer size and length the same
        inDataBuff->fBufferLength = inDataBuff->fBufferSize;

        // First lets start the buffer type
        bcopy( (const void *)&startTag, bufferLoc, 4 );
        bufferLoc += 8; // skip past buffer start and record count
        *numRecords = 0;

        // Now lets loop through the records until we fill the buffer...
        while (buffLeft && CFArrayGetCount(inRecordList))
		{
			CFDictionaryRef	cfRecDict	= (CFDictionaryRef) CFArrayGetValueAtIndex( inRecordList, 0 );
            CFDataRef		pData		= GetDSBufferFromDictionary( cfRecDict );
            UInt32			dataLength	= (UInt32) CFDataGetLength( pData );

			// Need room for (record offset, Block length field = 8 bytes) + the Block, only need 8
            if( dataLength && ((dataLength + 8) < buffLeft) )
			{
                // Now lets update all the offsets and buffer size
                bufferOffset -= dataLength + 4;
                buffLeft -= dataLength + 8;

                // first put the offest in the buffer for the record
                bcopy( (const void *)&bufferOffset, bufferLoc, 4 );
                bufferLoc += 4; // move past the new byte

                // Now copy length before the data block....
                bcopy( (const void *)&dataLength, (bufferStart + bufferOffset), 4 );

                // Now copy the bytes into the new location
				CFDataGetBytes( pData, CFRangeMake(0, dataLength), (UInt8 *) (bufferStart + bufferOffset + 4) );

                // Since we added a buffer, lets increment number of records
                *numRecords += 1;
                outRecEntryCount++;

                // remove now that we added it to the buffer
				CFArrayRemoveValueAtIndex( inRecordList, 0 );
            }
			else
			{
                // break if we can't fit it so we can return....
				DSCFRelease( pData );
                break;
            }
			
			DSCFRelease( pData );
        }
        // Close the record list....
        bcopy( (const void *)&endTag, bufferLoc, 4 );
    }
	
    return (tDirStatus)outRecEntryCount;
}

tDirStatus BaseDirectoryPlugin::ReleaseContinueData( sReleaseContinueData *inData )
{
	return (tDirStatus)fContinueHash->RemoveContext( inData->fInContinueData );
}

void BaseDirectoryPlugin::ContextDeallocProc( void *inContextData )
{
    if ( inContextData != NULL )
    {
        CleanContextData( inContextData );
        free( inContextData );
    }
}

void *BaseDirectoryPlugin::MakeContextData( CntxDataType dataType )
{
    void   *pOut = NULL;

    switch (dataType)
	{
        case kBDPIDirNode:
            pOut = (void *) calloc( 1, sizeof(sBDPINodeContext) );
			((sBDPINodeContext *) pOut)->fEffectiveUID = -1;
			((sBDPINodeContext *) pOut)->fUID = -1;
            break;
        case kBDPISearchRecords:
            pOut = (void *) calloc( 1, sizeof(sBDPISearchRecordsContext) );
            break;
        case kBDPIRecordEntry:
            pOut = (void *) calloc( 1, sizeof(sBDPIRecordEntryContext) );
            break;
        case kBDPIAttributeEntry:
            pOut = (void *) calloc( 1, sizeof(sBDPIAttributeEntryContext) );
            break;
        case kBDPIUndefined:
        default:
            break;
    }

    if ( pOut != NULL )
	{
        ((sBDPINodeContext *) pOut)->fType = dataType;
    }

    return( pOut );

} // MakeContextData

// ---------------------------------------------------------------------------
//	* CleanContextData
// ---------------------------------------------------------------------------

tDirStatus BaseDirectoryPlugin::CleanContextData ( void *inContext )
{
    tDirStatus					siResult	= eDSNoErr;
    sBDPIAttributeEntryContext	*tmpAttrib;
	sBDPIRecordEntryContext		*tmpRecEntry;
	sBDPISearchRecordsContext	*tmpSearch;
	sBDPINodeContext			*tmpNode;

	EXCEPTION_START
	
    switch( *((long *)inContext) )
	{
        case kBDPIDirNode:
			tmpNode = (sBDPINodeContext *) inContext;
#ifndef __OBJC__
			DSDelete( tmpNode->fVirtualNode );
#else
			[tmpNode->fVirtualNode release];
#endif
            break;
        case kBDPISearchRecords:
			tmpSearch = (sBDPISearchRecordsContext *) inContext;
			DSCFRelease( tmpSearch->fRecordTypeList );
			DSCFRelease( tmpSearch->fAttributeType );
			DSCFRelease( tmpSearch->fValueList );
			DSCFRelease( tmpSearch->fReturnAttribList );
			if ( tmpSearch->fStateInfoCallback != NULL )
				tmpSearch->fStateInfoCallback( tmpSearch->fStateInfo );
			tmpSearch->fStateInfo = NULL;
			tmpSearch->fStateInfoCallback = NULL;
            break;
        case kBDPIRecordEntry:
			tmpRecEntry = (sBDPIRecordEntryContext *) inContext;
			tmpRecEntry->fVirtualNode = NULL;
			DSCFRelease( tmpRecEntry->fRecord );
            break;
        case kBDPIAttributeEntry:
            tmpAttrib = (sBDPIAttributeEntryContext *) inContext;
			tmpAttrib->fVirtualNode = NULL;
			DSCFRelease( tmpAttrib->fRecord );
			DSCFRelease( tmpAttrib->fAttributeValueList );
			DSCFRelease( tmpAttrib->fAttributeName );
			break;
        default:
            siResult = eDSBadContextData;
            break;
    }
	
	EXCEPTION_END2
	
    return( siResult );
} // CleanContextData

UInt32 BaseDirectoryPlugin::CalculateCRCWithLength( const void *inData, UInt32 inLength )
{
	return CalcCRCWithLength( inData, inLength );
}

CFMutableArrayRef BaseDirectoryPlugin::CreateCFArrayFromList( tDataListPtr attribList )
{
	UInt32				count			= dsDataListGetNodeCountPriv( attribList );
	CFMutableArrayRef	cfReturnValue	= CFArrayCreateMutable( kCFAllocatorDefault, count, &kCFTypeArrayCallBacks );
	
	for( UInt32 ii = 1; ii <= count; ii++ )
	{
		tDataNodePtr	dataNode	= NULL;
		
		if ( dsDataListGetNodeAllocPriv(attribList, ii, &dataNode) == eDSNoErr )
		{
			CFStringRef	cfString = CFStringCreateWithBytes( kCFAllocatorDefault, (const UInt8 *) dataNode->fBufferData,
														   dataNode->fBufferLength, kCFStringEncodingUTF8, false );
			if ( cfString != NULL )
			{
				CFArrayAppendValue( cfReturnValue, cfString );
			}
			else
			{
				CFDataRef	cfData = CFDataCreate( kCFAllocatorDefault, (const UInt8 *) dataNode->fBufferData, dataNode->fBufferLength );
				
				if ( cfData != NULL )
					CFArrayAppendValue( cfReturnValue, cfData );
				
				DSCFRelease( cfData );
			}
			
			DSCFRelease( cfString );
			
			dsDataBufferDeallocatePriv( dataNode );
		}
	}
	
	return cfReturnValue;
}

CFDataRef BaseDirectoryPlugin::GetDSBufferFromDictionary( CFDictionaryRef inDictionary )
{
	CFMutableDataRef	cfData		= CFDataCreateMutable( kCFAllocatorDefault, 0 );
	char				*tmpStr		= NULL;
	const char			*pValue		= NULL;
	UInt16				usLength	= 0;
	
	// First do the Type of the record
	CFStringRef	cfRecType = (CFStringRef) CFDictionaryGetValue( inDictionary, kBDPITypeKey );
	
	pValue = BaseDirectoryPlugin::GetCStringFromCFString( cfRecType, &tmpStr ); 
	usLength = (UInt16) (pValue ? strlen( pValue ) : 0);
	
	CFDataAppendBytes( cfData, (const UInt8 *) &usLength, 2 );
	CFDataAppendBytes( cfData, (const UInt8 *) pValue, usLength );
	
	DSFreeString( tmpStr );
	
	// now get the record type and put that
	// Next fill in the Name of the Record, if it has a RecordName, then lets give it....
	CFStringRef	cfRecName = (CFStringRef) CFDictionaryGetValue( inDictionary, kBDPINameKey );
	
	pValue = BaseDirectoryPlugin::GetCStringFromCFString( cfRecName, &tmpStr ); 
	if( pValue == NULL )
		pValue = "No RecordName";
	
	usLength = (UInt16) strlen( pValue );
	CFDataAppendBytes( cfData, (const UInt8 *) &usLength, 2 );
	CFDataAppendBytes( cfData, (const UInt8 *) pValue, usLength );
	
	DSFreeString( tmpStr );
	
	CFDictionaryRef cfAttributes = (CFDictionaryRef) CFDictionaryGetValue( inDictionary, kBDPIAttributeKey );
	if ( cfAttributes != NULL )
	{
		UInt16	usNumberAttribs	= CFDictionaryGetCount( cfAttributes );
		
		CFDataAppendBytes( cfData, (const UInt8 *) &usNumberAttribs, 2 );
		
		if ( usNumberAttribs > 0 )
		{
			CFTypeRef	*cfKeysList		= (CFTypeRef *) calloc( usNumberAttribs, sizeof(CFTypeRef *) );
			CFTypeRef	*cfValuesList	= (CFTypeRef *) calloc( usNumberAttribs, sizeof(CFTypeRef *) );
			
			CFDictionaryGetKeysAndValues( cfAttributes, cfKeysList, cfValuesList );
			
			for (UInt16 ii = 0; ii < usNumberAttribs; ii++)
			{
				CFStringRef	cfKey		= (CFStringRef) cfKeysList[ii];
				CFArrayRef	cfValues	= (CFArrayRef) cfValuesList[ii];
				
				// Now build the attributes in a temp data object
				CFMutableDataRef	cfTempData	= CFDataCreateMutable( kCFAllocatorDefault, 0 );
				
				pValue = BaseDirectoryPlugin::GetCStringFromCFString( cfKey, &tmpStr );
				usLength = (UInt16) strlen( pValue );
				
				// first add the attribute name
				CFDataAppendBytes( cfTempData, (const UInt8 *) &usLength, 2 );
				CFDataAppendBytes( cfTempData, (const UInt8 *) pValue, usLength );
				
				DSFreeString( tmpStr );
				
				// Number of values
				UInt16 usValuesCount = (UInt16) CFArrayGetCount( cfValues );
				CFDataAppendBytes( cfTempData, (const UInt8 *) &usValuesCount, 2 );
				
				// Loop through values
				for( UInt16 zz = 0; zz < usValuesCount; zz++ )
				{
					CFTypeRef	cfValue	= CFArrayGetValueAtIndex( cfValues, zz );
					
					if ( CFGetTypeID(cfValue) == CFStringGetTypeID() )
					{
						pValue = GetCStringFromCFString( (CFStringRef) cfValue, &tmpStr );
						UInt32 attribLen = (UInt32) strlen( pValue );
						
						CFDataAppendBytes( cfTempData, (const UInt8 *) &attribLen, 4 );
						CFDataAppendBytes( cfTempData, (const UInt8 *) pValue, attribLen );
						
						DSFreeString( tmpStr );
					}
					else
					{
						UInt32 attribLen = CFDataGetLength( (CFDataRef) cfValue );
						
						CFDataAppendBytes( cfTempData, (const UInt8 *) &attribLen, 4 );
						CFDataAppendBytes( cfTempData, CFDataGetBytePtr((CFDataRef) cfValue), attribLen );
					}
				}
				
				// Now append this attribute block
				UInt32 attribBlockLen = CFDataGetLength( cfTempData );
				CFDataAppendBytes( cfData, (const UInt8 *) &attribBlockLen, 4 );
				CFDataAppendBytes( cfData, CFDataGetBytePtr(cfTempData), attribBlockLen );
				
				DSCFRelease( cfTempData );
			}
			
			DSFree( cfKeysList );
			DSFree( cfValuesList );
		}
	}
	
	return cfData;
}

void BaseDirectoryPlugin::FilterAttributes( CFMutableDictionaryRef inRecord, CFArrayRef inRequestedAttribs, CFStringRef inNodeName )
{
	CFRange					cfAttribRange	= CFRangeMake( 0, CFArrayGetCount(inRequestedAttribs) );
	bool					bNeedStdAll		= CFArrayContainsValue( inRequestedAttribs, cfAttribRange, CFSTR(kDSAttributesStandardAll) );
	bool					bNeedNativeAll	= CFArrayContainsValue( inRequestedAttribs, cfAttribRange, CFSTR(kDSAttributesNativeAll) );
	bool					bNeedAll		= ( (bNeedStdAll && bNeedNativeAll) || 
												CFArrayContainsValue(inRequestedAttribs, cfAttribRange, CFSTR(kDSAttributesAll)) );

	CFMutableDictionaryRef	cfAttributes	= (CFMutableDictionaryRef) CFDictionaryGetValue( inRecord, kBDPIAttributeKey );
	
	if ( bNeedAll == false )
	{
		CFIndex		iCount	= CFDictionaryGetCount( cfAttributes );
		CFTypeRef	*keys	= (CFTypeRef *) calloc( iCount, sizeof(CFTypeRef) );
		
		CFDictionaryGetKeysAndValues( cfAttributes, keys, NULL );
		
		for (CFIndex ii = 0; ii < iCount; ii++ )
		{
			if ( CFArrayContainsValue(inRequestedAttribs, cfAttribRange, keys[ii]) == false )
			{
				if ( (bNeedStdAll == false && bNeedNativeAll == false) ||
					 (bNeedStdAll == true && CFStringHasPrefix((CFStringRef) keys[ii], CFSTR(kDSStdAttrTypePrefix)) == false) ||
					 (bNeedNativeAll == true && CFStringHasPrefix((CFStringRef) keys[ii], CFSTR(kDSNativeAttrTypePrefix)) == false) )
				{
					CFDictionaryRemoveValue( cfAttributes, keys[ii] );
				}
			}
		}
		
		DSFree( keys );
	}
	
	if ( bNeedAll || bNeedStdAll || CFArrayContainsValue( inRequestedAttribs, cfAttribRange, CFSTR(kDSNAttrMetaNodeLocation)) )
	{
		CFArrayRef cfNodeLoc = CFArrayCreate( kCFAllocatorDefault, (CFTypeRef *) &inNodeName, 1, &kCFTypeArrayCallBacks );
		CFDictionarySetValue( cfAttributes, CFSTR(kDSNAttrMetaNodeLocation), cfNodeLoc );
		DSCFRelease( cfNodeLoc );
	}
}

//------------------------------------------------------------------------------------
//	* GetRecordTypeFromRef
//
//	Returns: an allocated c-string. The caller must free.
//------------------------------------------------------------------------------------

char *BaseDirectoryPlugin::GetRecordTypeFromRef( tRecordReference inRecRef )
{
	char			*recTypeStr		= NULL;
	sGetRecRefInfo	recRefReq;
	
	// form a request directory to avoid internal dispatch overhead
	recRefReq.fType			= kGetRecordReferenceInfo;
	recRefReq.fResult		= eDSNoErr;
	recRefReq.fInRecRef		= inRecRef;
	recRefReq.fOutRecInfo	= NULL;
	
	// now call GetRecRefInfo call that will be effectively a dsGetRecordReferenceInfo
	tDirStatus siResult = GetRecRefInfo( &recRefReq );
	if ( siResult == eDSNoErr )
	{
		dsGetRecordTypeFromEntry( recRefReq.fOutRecInfo, &recTypeStr );
		dsDeallocRecordEntry( 0, recRefReq.fOutRecInfo );
	}
	
	return recTypeStr;
}
