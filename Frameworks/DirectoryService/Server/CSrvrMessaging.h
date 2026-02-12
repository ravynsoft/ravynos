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
 * @header CSrvrMessaging
 */

#ifndef __CSrvrMessaging_h__
#define __CSrvrMessaging_h__		1

#include "PrivateTypes.h"
#include "SharedConsts.h"
#include "DirServicesTypes.h"

class CSrvrMessaging {
public:

	   			CSrvrMessaging				( void );
virtual		   ~CSrvrMessaging				( void );

		SInt32	Add_tDataBuff_ToMsg			( sComData **inMsg, tDataBuffer *inBuff, eValueType inType );
		SInt32	Add_tDataList_ToMsg			( sComData **inMsg, tDataList *inList, eValueType inType );
		SInt32	Add_Value_ToMsg				( sComData *inMsg, UInt32 inValue, eValueType inType );
		SInt32	Add_tAttrEntry_ToMsg		( sComData **inMsg, tAttributeEntry *inData );
		SInt32	Add_tAttrValueEntry_ToMsg	( sComData **inMsg, tAttributeValueEntry *inData );
		SInt32	Add_tRecordEntry_ToMsg		( sComData **inMsg, tRecordEntry *inData );
											//note we use ptr to ptr only in Add methods that can grow the inMsg

		SInt32	Get_tDataBuff_FromMsg		( sComData *inMsg, tDataBuffer **outBuff, eValueType inType );
		SInt32	Get_tDataList_FromMsg		( sComData *inMsg, tDataList **outList, eValueType inType );
		SInt32	Get_Value_FromMsg			( sComData *inMsg, UInt32 *outValue, eValueType inType );
		SInt32	Get_tAttrEntry_FromMsg		( sComData *inMsg, tAttributeEntry **outAttrEntry, eValueType inType );
		SInt32	Get_tAttrValueEntry_FromMsg	( sComData *inMsg, tAttributeValueEntry **outAttrValue, eValueType inType );
		SInt32	Get_tRecordEntry_FromMsg	( sComData *inMsg, tRecordEntry **outRecEntry, eValueType inType );

		void	ClearDataBlock				( sComData *inMsg );
		void	ClearMessageBlock			( sComData *inMsg );
		void	Grow						( sComData **inMsg, UInt32 inOffset, UInt32 inSize );
		
private:
		SInt32	GetEmptyObj					( sComData *inMsg, eValueType inType, sObject **outObj );
		SInt32	GetThisObj					( sComData *inMsg, eValueType inType, sObject **outObj );

};

#endif
