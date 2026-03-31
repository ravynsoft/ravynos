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

#ifndef	_BASEDIRECTORYPLUGINTYPES_H
#define	_BASEDIRECTORYPLUGINTYPES_H

#include <DirectoryService/DirServicesTypes.h>
#include <DirectoryService/DirServicesConst.h>
#include <DirectoryServiceCore/SharedConsts.h>
#include <DirectoryServiceCore/PluginData.h>
#include <CoreFoundation/CoreFoundation.h>

#define kBDPINameKey			CFSTR("Name")
#define kBDPIDistinguishedKey	CFSTR("Distinguished")
#define kBDPIAttributeKey		CFSTR("Attributes")
#define kBDPITypeKey			CFSTR("Type")
#define kBDPIGUID				CFSTR("GUID")

#ifdef __OBJC__
	#define kNSBDPINameKey			(NSString*)kBDPINameKey
	#define kNSBDPIDistinguishedKey	(NSString*)kBDPIDistinguishedKey
	#define kNSBDPIAttributeKey		(NSString*)kBDPIAttributeKey
	#define kNSBDPITypeKey			(NSString*)kBDPITypeKey
	#define kNSBDPIGUID				(NSString*)kBDPIGUID
#endif

#ifndef __OBJC__
class BDPIVirtualNode;
#else
@class BDPIVirtualNode;
#endif

const int kBPDIBufferTax	= 16;

enum CntxDataType
{
	kBDPIUndefined 		= 0,
	kBDPIDirNode		= 1,
	kBDPIAuthentication	= 2,
	kBDPISearchRecords	= 3,
	kBDPIRecordEntry 	= 4,
	kBDPIAttributeEntry = 5
};

typedef void *BDPIOpaqueBuffer;
typedef void (*SearchCtxStateFree)(void *);

struct sBDPINodeContext
{
	enum CntxDataType	fType;
	BDPIVirtualNode		*fVirtualNode;
	uid_t				fUID;
	uid_t				fEffectiveUID;
};

struct sBDPISearchRecordsContext
{
	enum CntxDataType	fType;
	CFArrayRef			fRecordTypeList;
	CFStringRef			fAttributeType;
	tDirPatternMatch	fPattMatchType;
	CFArrayRef			fValueList;
	CFArrayRef			fReturnAttribList;
	bool				fAttribsOnly;
	UInt32				fIndex;
	UInt32				fMaxRecCount;
	CFIndex				fRecTypeIndex;
	void				*fStateInfo;
	SearchCtxStateFree	fStateInfoCallback;
};

struct sBDPIRecordEntryContext
{
	enum CntxDataType		fType;
	BDPIVirtualNode			*fVirtualNode;
	CFMutableDictionaryRef	fRecord;
};

struct sBDPIAttributeEntryContext
{
	enum CntxDataType		fType;
	BDPIVirtualNode			*fVirtualNode;
	CFMutableDictionaryRef	fRecord;
	CFStringRef				fAttributeName;
	CFMutableArrayRef		fAttributeValueList;
};

struct sBDPIAuthContext
{
	enum CntxDataType		fType;
	BDPIVirtualNode			*fVirtualNode;
	void					*fStateInfo;
};

#endif
