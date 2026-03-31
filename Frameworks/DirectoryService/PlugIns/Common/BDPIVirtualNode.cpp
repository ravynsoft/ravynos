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

#include "BDPIVirtualNode.h"
#include "BaseDirectoryPlugin.h"

#ifdef __OBJC__

@implementation BDPIVirtualNode

- (id)init: (NSString *)inNodeName uid: (uid_t)inUID euid: (uid_t)inEffectiveUID
{
	if ( self = [super init] )
	{
		fNodeName = [inNodeName copy];
		fUID = inUID;
		fEffectiveUID = inEffectiveUID;
	}
	
	return self;
}

- (void)dealloc
{
	[fNodeName release];
	fNodeName = nil;
	[super dealloc];
}

- (NSMutableDictionary *)copyNodeInfo:(NSArray *)inAttributes
{
	return NULL;
}

- (NSString *)copyNodeName
{
	return [fNodeName copy];
}

- (tDirStatus)customCall:(UInt32)inRequestCode requestData:(NSData *)inRequestData response:(NSMutableData *)outResponseData 
		 maxResponseSize:(UInt32)maxResponseSize
{
	return eNotHandledByThisNode;
}

- (tDirStatus)setNodeCredentials:(NSString *)inRecordType recordName:(NSString *)inRecordName password:(NSString *)inPassword
{
	return eNotHandledByThisNode;
}
- (tDirStatus)setNodeCredentialsExtended:(NSString *)inRecordType authType:(NSString *)inAuthType authItems:(NSArray *)inAuthItems
							outAuthItems:(NSArray **)outAuthItems outContext:(sBDPIAuthContext *)outContext
{
	return eNotHandledByThisNode;
}

- (tDirStatus)verifyCredentials:(NSString *)inRecordType recordName:(NSString *)inRecordName password:(NSString *)inPassword
{
	return eNotHandledByThisNode;
}

- (tDirStatus)verifyCredentialsExtended:(NSString *)inRecordType authType:(NSString *)inAuthType authItems:(NSArray *)inAuthItems 
						   outAuthItems:(NSArray **)outAuthItems outContext:(sBDPIAuthContext *)outContext
{
	return eNotHandledByThisNode;
}

- (tDirStatus)changePassword:(NSString *)inRecordType recordName:(NSString *)inRecordName oldPassword:(NSString *)inOldPassword
				 newPassword:(NSString *)inNewPassword
{
	return eNotHandledByThisNode;
}

- (tDirStatus)changePasswordExtended:(NSString *)inRecordType authType:(NSString *)inAuthType authItems:(NSArray *)inAuthItems
						outAuthItems:(NSArray **)outAuthItems outContext:(sBDPIAuthContext *)outContext
{
	return eNotHandledByThisNode;
}

- (NSDictionary *)copyPasswordPolicyForRecord:(NSString *)inRecordType withName:(NSString *)inName
{
	return nil;
}

- (tDirStatus)searchRecords:(sBDPISearchRecordsContext *)inContext buffer:(BDPIOpaqueBuffer)inBuffer outCount:(UInt32 *)outCount
{
	return eNotHandledByThisNode;
}

- (NSMutableDictionary *)recordOpenName:(NSString *)inRecordName recordType:(NSString *)inRecordType
{
	return nil;
}

- (tDirStatus)recordCreateName:(NSString *)inRecordName recordType:(NSString *)inRecordType
{
	return eNotHandledByThisNode;
}

- (tDirStatus)recordDelete:(NSDictionary *)inRecord
{
	return eNotHandledByThisNode;
}

- (tDirStatus)recordFlush:(NSDictionary *)inRecord
{
	return eNotHandledByThisNode;
}

- (tDirStatus)recordClose:(NSDictionary *)inRecord
{
	return eNotHandledByThisNode;
}

- (tDirStatus)record:(NSMutableDictionary *)inRecord setRecordType:(NSString *)inRecordType
{
	return eNotHandledByThisNode;
}

- (tDirStatus)record:(NSMutableDictionary *)inRecord setValues:(NSArray *)inValues forAttribute:(NSString *)inAttribute
{
	return eNotHandledByThisNode;
}

- (tDirStatus)record:(NSMutableDictionary *)inRecord addValues:(NSArray *)inValues toAttribute:(NSString *)inAttribute
{
	return eNotHandledByThisNode;
}

- (tDirStatus)record:(NSMutableDictionary *)inRecord removeValue:(id)inValue fromAttribute:(NSString *)inAttribute
{
	return eNotHandledByThisNode;
}

- (BOOL)allowChanges:(NSDictionary *)inRecord forAttribute:(NSString *)inAttribute
{
	return false;
}

- (UInt32)maximumSize:(NSString *)inRecordType forAttribute:(NSString *)inAttribute
{
	return 0;
}

- (void)filterAttributes: (NSMutableDictionary *)inRecord attributes: (NSArray *)inRequestedAttribs
{
	BaseDirectoryPlugin::FilterAttributes( (CFMutableDictionaryRef) inRecord, (CFArrayRef) inRequestedAttribs, (CFStringRef) fNodeName );
}

@end

#else

BDPIVirtualNode::BDPIVirtualNode( CFStringRef inNodeName, uid_t inUID, uid_t inEffectiveUID )
{
	fNodeName = CFStringCreateCopy( kCFAllocatorDefault, inNodeName );
	fUID = inUID;
	fEffectiveUID = inEffectiveUID;
}

BDPIVirtualNode::~BDPIVirtualNode( void )
{
	DSCFRelease( fNodeName );
}

CFStringRef BDPIVirtualNode::CopyNodeName( void )
{
	return CFStringCreateCopy( kCFAllocatorDefault, fNodeName );
}

tDirStatus BDPIVirtualNode::CustomCall( UInt32 inRequestCode, CFDataRef inRequestData, CFMutableDataRef outResponseData, 
									    UInt32 maxResponseSize )
{
	return eNotHandledByThisNode;
}

tDirStatus BDPIVirtualNode::SetNodeCredentials( CFStringRef inRecordType, CFStringRef inRecordName, CFStringRef inPassword )
{
	return eNotHandledByThisNode;
}

tDirStatus BDPIVirtualNode::SetNodeCredentialsExtended( CFStringRef inRecordType, CFStringRef inAuthType, CFArrayRef inAuthItems,
													    CFArrayRef *outAuthItems, sBDPIAuthContext *outContext )
{
	return eNotHandledByThisNode;
}

tDirStatus BDPIVirtualNode::VerifyCredentials( CFStringRef inRecordType, CFStringRef inRecordName, CFStringRef inPassword )
{
	return eNotHandledByThisNode;
}

tDirStatus BDPIVirtualNode::VerifyCredentialsExtended( CFStringRef inRecordType, CFStringRef inAuthType, CFArrayRef inAuthItems,
													   CFArrayRef *outAuthItems, sBDPIAuthContext *outContext )
{
	return eNotHandledByThisNode;
}

tDirStatus BDPIVirtualNode::ChangePassword( CFStringRef inRecordType, CFStringRef inRecordName, CFStringRef inOldPassword,
										    CFStringRef inNewPassword )
{
	return eNotHandledByThisNode;
}

tDirStatus BDPIVirtualNode::ChangePasswordExtended( CFStringRef inRecordType, CFStringRef inAuthType, CFArrayRef inAuthItems,
												    CFArrayRef *outAuthItems, sBDPIAuthContext *outContext )
{
	return eNotHandledByThisNode;
}

CFDictionaryRef BDPIVirtualNode::CopyPasswordPolicyForRecord( CFStringRef inRecordType, CFStringRef inName )
{
	return NULL;
}

CFMutableDictionaryRef BDPIVirtualNode::RecordOpen( CFStringRef inRecordType, CFStringRef inRecordName )
{
	return NULL;
}

tDirStatus BDPIVirtualNode::RecordCreate( CFStringRef inRecordType, CFStringRef inRecordName )
{
	return eNotHandledByThisNode;
}

tDirStatus BDPIVirtualNode::RecordDelete( CFDictionaryRef inRecordRef )
{
	return eNotHandledByThisNode;
}

tDirStatus BDPIVirtualNode::RecordFlush( CFDictionaryRef inRecordRef )
{
	return eNotHandledByThisNode;
}

tDirStatus BDPIVirtualNode::RecordClose( CFDictionaryRef inRecordRef )
{
	return eNotHandledByThisNode;
}

tDirStatus BDPIVirtualNode::RecordSetType( CFMutableDictionaryRef inRecordRef, CFStringRef inRecordType )
{
	return eNotHandledByThisNode;
}

tDirStatus BDPIVirtualNode::RecordSetValuesForAttribute( CFMutableDictionaryRef inRecordRef, CFStringRef inAttribute, CFArrayRef inValues )
{
	return eNotHandledByThisNode;
}

tDirStatus BDPIVirtualNode::RecordAddValuesToAttribute( CFMutableDictionaryRef inRecordRef, CFStringRef inAttribute, CFArrayRef inValues )
{
	return eNotHandledByThisNode;
}

tDirStatus BDPIVirtualNode::RecordRemoveValueFromAttribute( CFMutableDictionaryRef inRecordRef, CFStringRef inAttribute, CFTypeRef inValue )
{
	return eNotHandledByThisNode;
}

bool BDPIVirtualNode::AllowChangesForAttribute( CFDictionaryRef inRecordRef, CFStringRef inAttribute )
{
	return false;
}

UInt32 BDPIVirtualNode::MaximumSizeForAttribute( CFStringRef inRecordType, CFStringRef inAttribute )
{
	return 0;
}

void BDPIVirtualNode::FilterAttributes( CFMutableDictionaryRef inRecord, CFArrayRef inRequestedAttribs )
{
	BaseDirectoryPlugin::FilterAttributes( inRecord, inRequestedAttribs, fNodeName );
}

#endif
