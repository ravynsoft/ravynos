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

#ifndef	_BASEDIRECTORYPLUGIN_H
#define	_BASEDIRECTORYPLUGIN_H

#include <CoreFoundation/CoreFoundation.h>
#include <DirectoryServiceCore/DSEventSemaphore.h>
#include <DirectoryServiceCore/DSMutexSemaphore.h>
#include <DirectoryServiceCore/CLog.h>
#include <DirectoryServiceCore/BaseDirectoryPluginTypes.h>
#include <dispatch/dispatch.h>

#if defined(SERVERINTERNAL)
	#include "CServerPlugin.h"
#else
	#include <DirectoryServiceCore/CDSServerModule.h>
	#include <DirectoryServiceCore/ServerModuleLib.h>
	#define CServerPlugin	CDSServerModule
#endif

#include <DirectoryServiceCore/CDSAuthParams.h>

class CContinue;
class CPlugInRef;

class BaseDirectoryPlugin : public CServerPlugin
{
	public:
#ifdef SERVERINTERNAL
								BaseDirectoryPlugin		( FourCharCode inSig, const char *inName );
#else
								BaseDirectoryPlugin		( const char *inName );
#endif
		virtual					~BaseDirectoryPlugin	( void );

		// These must be called from inherited class to ensure fState is updated properly
		virtual SInt32			Initialize				( void );
		virtual SInt32			SetPluginState			( const UInt32 inState );
		virtual CDSAuthParams*	NewAuthParamObject		( void );
		
		// Required to override this functionality
		virtual SInt32			PeriodicTask			( void ) = 0;

		virtual SInt32			ProcessRequest			( void *inData );

		static tDirStatus		FillBuffer				( CFMutableArrayRef inRecordList, BDPIOpaqueBuffer inData );
		static const char		*GetCStringFromCFString	( CFStringRef inCFString, char **outCString );
		static void				FilterAttributes		( CFMutableDictionaryRef inRecord, CFArrayRef inRequestedAttribs, CFStringRef inNodeName );
		char					*GetRecordTypeFromRef	( tRecordReference inRecRef );

	protected:
		virtual void			NetworkTransition		( void );
	
		virtual CFDataRef		CopyConfiguration		( void ) = 0;
		virtual bool			NewConfiguration		( const char *inData, UInt32 inLength ) = 0;
		virtual bool			CheckConfiguration		( const char *inData, UInt32 inLength ) = 0;
		virtual tDirStatus		HandleCustomCall		( sBDPINodeContext *pContext, sDoPlugInCustomCall *inData ) = 0;
		virtual bool			IsConfigureNodeName		( CFStringRef inNodeName ) = 0;
		virtual BDPIVirtualNode	*CreateNodeForPath		( CFStringRef inPath, uid_t inUID, uid_t inEffectiveUID ) = 0;
	
		virtual tDirStatus		DoPlugInCustomCall		( sDoPlugInCustomCall *inData );

		virtual tDirStatus		RegisterNode			( CFStringRef inNodeName, eDirNodeType inType );
		virtual tDirStatus		UnregisterNode			( CFStringRef inNodeName, eDirNodeType inType );

		virtual tDirStatus		OpenDirNode				( sOpenDirNode *inData );
		virtual tDirStatus		CloseDirNode			( sCloseDirNode *inData );
		virtual tDirStatus		GetDirNodeInfo			( sGetDirNodeInfo *inData );
		
		virtual tDirStatus		GetRecordList			( sGetRecordList *inData );
		virtual tDirStatus		GetRecordEntry			( sGetRecordEntry *inData );
		virtual tDirStatus		CreateRecord			( sCreateRecord *inData );
		virtual tDirStatus		OpenRecord				( sOpenRecord *inData );
		virtual tDirStatus		CloseRecord				( sCloseRecord *inData );
		virtual tDirStatus		SetRecordType 			( sSetRecordType *inData );
		virtual tDirStatus		DeleteRecord			( sDeleteRecord *inData );
		virtual tDirStatus		SetRecordName			( sSetRecordName *inData );
		virtual tDirStatus		FlushRecord				( sFlushRecord *inData );
		
		virtual tDirStatus		GetRecRefInfo			( sGetRecRefInfo *inData );
		virtual tDirStatus		GetRecAttribInfo		( sGetRecAttribInfo *inData );
		virtual tDirStatus		GetRecAttrValueByValue  ( sGetRecordAttributeValueByValue *inData );
		virtual tDirStatus		GetRecAttrValueByIndex	( sGetRecordAttributeValueByIndex *inData );
		virtual tDirStatus		GetRecAttrValueByID		( sGetRecordAttributeValueByID *inData );
		
		virtual tDirStatus		GetAttributeEntry		( sGetAttributeEntry *inData );
		virtual tDirStatus		AddAttribute 			( sAddAttribute *inData, const char *inRecTypeStr );
		virtual tDirStatus		RemoveAttribute			( sRemoveAttribute *inData, const char *inRecTypeStr );
		virtual tDirStatus		CloseAttributeList		( sCloseAttributeList *inData );
		
		virtual tDirStatus		GetAttributeValue		( sGetAttributeValue *inData );
		virtual tDirStatus		AddAttributeValue		( sAddAttributeValue *inData, const char *inRecTypeStr );
		virtual tDirStatus		SetAttributeValue		( sSetAttributeValue *inData, const char *inRecTypeStr );
		virtual tDirStatus		RemoveAttributeValue	( sRemoveAttributeValue *inData, const char *inRecTypeStr );
		virtual tDirStatus		CloseAttributeValueList	( sCloseAttributeValueList *inData );
		
		virtual tDirStatus		DoAttributeValueSearch			( sDoAttrValueSearch *inData );
		virtual tDirStatus		DoAttributeValueSearchWithData	( sDoAttrValueSearchWithData *inData );
		virtual tDirStatus		ReleaseContinueData				( sReleaseContinueData *continueData );
		
		virtual tDirStatus		DoAuthentication		( sDoDirNodeAuth *inData, const char *inRecTypeStr,
															CDSAuthParams &inParams );
	
		virtual tDirStatus		DoSimpleAuthentication	( sBDPINodeContext *inContext, CFStringRef inRecordType, tDataBuffer *inAuthData, bool bAuthOnly );
		virtual tDirStatus		DoExtendedAuthentication( sBDPINodeContext *inContext, CFStringRef inRecordType, CFStringRef inAuthMethod, 
														  tDataBuffer *inAuthData, tDataBuffer *inOutAuthResponse, bool bAuthOnly );

		virtual tDirStatus		DoSimplePasswordChange	( sBDPINodeContext *inContext, CFStringRef inRecordType, tDataBuffer *inAuthData );
		virtual tDirStatus		DoExtendedPasswordChange( sBDPINodeContext *inContext, tDataBuffer *inAuthData, tDataBuffer *inOutAuthResponse, 
														  bool bAuthOnly );
	
		virtual tDirStatus		DoAuthGetPolicy			( sBDPINodeContext *inContext, CFStringRef inRecordType, unsigned int uiMethod, tDataBuffer *inAuthData,
														  tDataBuffer *outAuthResponse );
	
		CFArrayRef				CreateAuthArrayFromBuffer	( tDataBufferPtr inAuthBuff );


		static void				*MakeContextData			( CntxDataType dataType );
		static tDirStatus		CleanContextData			( void *inContext );
		static void				ContextDeallocProc			( void *inContextData );
		static UInt32			CalculateCRCWithLength		( const void *inData, UInt32 inLength );

	protected:
		DSMutexSemaphore		*fKerberosMutex;
		DSEventSemaphore		*fReadyForRequests;
		char					*fPluginPrefix;
		int						fPluginPrefixLen;
		CFStringRef				fPluginPrefixCF;

		DSMutexSemaphore		fBasePluginMutex;	// mutex should be used to access any variables below
	
		// fBasePluginMutex should be used to access any of the following variables
		CFRunLoopRef			fPluginRunLoop;
		UInt32					fState;
		dispatch_source_t		fTransitionTimer;
	
		UInt32					fCustomCallReadConfigSize;
		UInt32					fCustomCallReadConfig;
		UInt32					fCustomCallWriteConfig;
		UInt32					fCustomCallVerifyConfig;
	
		CPlugInRef				*fContextHash;
		CContinue				*fContinueHash;
		dispatch_queue_t		fQueue;
	
	private:
		static CFMutableArrayRef	CreateCFArrayFromList( tDataListPtr attribList );
		static CFDataRef			GetDSBufferFromDictionary( CFDictionaryRef inDictionary );
};

#endif
