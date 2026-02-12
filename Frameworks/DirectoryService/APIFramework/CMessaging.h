/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
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
 * @header CMessaging
 * Communications class for the DirectoryService Framework providing
 * connection with a DirectoryService daemon via either a mach or TCP endpoint
 * and handling all the message data packing and unpacking.
 */

#ifndef __CMessaging_h__
#define __CMessaging_h__		1

#include <DirectoryServiceCore/PrivateTypes.h>
#include <DirectoryServiceCore/SharedConsts.h>
#include <DirectoryService/DirServicesTypes.h>
#include <DirectoryServiceCore/DSMutexSemaphore.h>

class CMessaging {
public:

	   			CMessaging					( CIPCVirtualClass *inEndPoint, int inTranslateMode, bool internal = true );
virtual		   ~CMessaging					( void );

		SInt32	Add_tDataBuff_ToMsg			( tDataBuffer *inBuff, eValueType inType );
		SInt32	Add_tDataList_ToMsg			( tDataList *inList, eValueType inType );
		SInt32	Add_Value_ToMsg				( UInt32 inValue, eValueType inType );
		SInt32	Add_tAttrEntry_ToMsg		( tAttributeEntry *inData );
		SInt32	Add_tAttrValueEntry_ToMsg	( tAttributeValueEntry *inData );
		SInt32	Add_tRecordEntry_ToMsg		( tRecordEntry *inData );

		SInt32	Get_tDataBuff_FromMsg		( tDataBuffer **outBuff, eValueType inType );
		SInt32	Get_tDataList_FromMsg		( tDataList **outList, eValueType inType );
		SInt32	Get_Value_FromMsg			( UInt32 *outValue, eValueType inType );
		SInt32	Get_tAttrEntry_FromMsg		( tAttributeEntry **outAttrEntry, eValueType inType );
		SInt32	Get_tAttrValueEntry_FromMsg	( tAttributeValueEntry **outAttrValue, eValueType inType );
		SInt32	Get_tRecordEntry_FromMsg	( tRecordEntry **outRecEntry, eValueType inType );

		SInt32	SendInlineMessage			( UInt32 inMsgType );
		SInt32	GetReplyMessage				( void );

		void	CloseConnection				( void ) { if ( fCommPort != NULL ) fCommPort->Disconnect(); }
	
		void	Lock						( void );
		void	Unlock						( void );
		void	ClearMessageBlock			( void );
		
		UInt32	GetServerVersion			( void );
		void	SetServerVersion			( UInt32 inServerVersion );
		void	ResetMessageBlock			( void );
		void	SetTranslateMode			( int inTranslateMode )	{ fTranslateMode = inTranslateMode;	}
		int		GetTranslateMode			( void ) { return fTranslateMode; }
#ifdef SERVERINTERNAL
		bool	IsThreadUsingInternalDispatchBuffering( UInt32 inThreadSig );
#endif
	
private:
		SInt32	GetEmptyObj					( sComData *inMsg, eValueType inType, sObject **outObj );
		SInt32	GetThisObj					( sComData *inMsg, eValueType inType, sObject **outObj );
		sComData*   GetMsgData				( void );

		bool	Grow						( UInt32 inOffset, UInt32 inSize );

		CIPCVirtualClass	*fCommPort;
		int					fTranslateMode;
		bool				fInternal;

		DSMutexSemaphore	fLock;

		sComData		   *fMsgData;
		UInt32				fServerVersion;
};

#endif
