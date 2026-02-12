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

#ifndef _BDPIVIRTUALNODE_H
#define _BDPIVIRTUALNODE_H

#include <DirectoryServiceCore/BaseDirectoryPluginTypes.h>

#ifndef __OBJC__

// Virtual class for BaseDirectoryPlugInVirtualNode (BDPIVirtualNode)
class BDPIVirtualNode
{
	public:
										BDPIVirtualNode( CFStringRef inNodeName, uid_t inUID, uid_t inEffectiveUID );
		virtual							~BDPIVirtualNode( void );
		
		virtual CFMutableDictionaryRef	CopyNodeInfo( CFArrayRef cfAttributes ) = 0;
		virtual CFStringRef				CopyNodeName( void );
		
		virtual tDirStatus				CustomCall( UInt32 inRequestCode, CFDataRef inRequestData, CFMutableDataRef outResponseData, 
												    UInt32 maxResponseSize );
		
		virtual tDirStatus				SetNodeCredentials( CFStringRef inRecordType, CFStringRef inRecordName, CFStringRef inPassword );
		virtual tDirStatus				SetNodeCredentialsExtended( CFStringRef inRecordType, CFStringRef inAuthType, CFArrayRef inAuthItems,
																   CFArrayRef *outAuthItems, sBDPIAuthContext *outContext );
		
		virtual tDirStatus				VerifyCredentials( CFStringRef inRecordType, CFStringRef inRecordName, CFStringRef inPassword );
		virtual tDirStatus				VerifyCredentialsExtended( CFStringRef inRecordType, CFStringRef inAuthType, CFArrayRef inAuthItems,
																  CFArrayRef *outAuthItems, sBDPIAuthContext *outContext );
		
		virtual tDirStatus				ChangePassword( CFStringRef inRecordType, CFStringRef inRecordName, CFStringRef inOldPassword,
													    CFStringRef inNewPassword );
		virtual tDirStatus				ChangePasswordExtended( CFStringRef inRecordType, CFStringRef inAuthType, CFArrayRef inAuthItems,
															    CFArrayRef *outAuthItems, sBDPIAuthContext *outContext );
		
		virtual CFDictionaryRef			CopyPasswordPolicyForRecord( CFStringRef inRecordType, CFStringRef inName );
		
		virtual tDirStatus				SearchRecords( sBDPISearchRecordsContext *inContext, BDPIOpaqueBuffer inBuffer, UInt32 *outCount) = 0;
		
		virtual CFMutableDictionaryRef	RecordOpen( CFStringRef inRecordType, CFStringRef inRecordName );
		virtual tDirStatus				RecordCreate( CFStringRef inRecordType, CFStringRef inRecordName );
		virtual tDirStatus				RecordDelete( CFDictionaryRef inRecordRef );
		virtual tDirStatus				RecordFlush( CFDictionaryRef inRecordRef );
		virtual tDirStatus				RecordClose( CFDictionaryRef inRecordRef );
		virtual tDirStatus				RecordSetType( CFMutableDictionaryRef inRecordRef, CFStringRef inRecordType );
		virtual tDirStatus				RecordSetValuesForAttribute( CFMutableDictionaryRef inRecordRef, CFStringRef inAttribute, 
																	 CFArrayRef inValues );
		virtual tDirStatus				RecordAddValuesToAttribute( CFMutableDictionaryRef inRecordRef, CFStringRef inAttribute,
																    CFArrayRef inValues );
		virtual tDirStatus				RecordRemoveValueFromAttribute( CFMutableDictionaryRef inRecordRef, CFStringRef inAttribute,
																	    CFTypeRef inValue );
		
		virtual bool					AllowChangesForAttribute( CFDictionaryRef inRecordRef, CFStringRef inAttribute );
		virtual UInt32					MaximumSizeForAttribute( CFStringRef inRecordType, CFStringRef inAttribute );
	
	protected:
		virtual void					FilterAttributes( CFMutableDictionaryRef inRecord, CFArrayRef inRequestedAttribs );
	
	protected:
		CFStringRef			fNodeName;
		uid_t				fUID;
		uid_t				fEffectiveUID;
};

#else

#include <Foundation/Foundation.h>

//
// this class can be inherited, functionality should be overriden as necessary
//
@interface BDPIVirtualNode : NSObject
{
	@protected
		NSString	*fNodeName;
		uid_t		fUID;
		uid_t		fEffectiveUID;
}

- (id)init: (NSString *)inNodeName uid: (uid_t)inUID euid: (uid_t)inEffectiveUID;
- (void)filterAttributes: (NSMutableDictionary *)record attributes: (NSArray *)requestedAttribs;

- (NSMutableDictionary *)copyNodeInfo:(NSArray *)inAttributes;
- (NSString *)copyNodeName;

- (tDirStatus)customCall:(UInt32)inRequestCode requestData:(NSData *)inRequestData response:(NSMutableData *)outResponseData 
		 maxResponseSize:(UInt32)maxResponseSize;

- (tDirStatus)setNodeCredentials:(NSString *)inRecordType recordName:(NSString *)inRecordName password:(NSString *)inPassword;
- (tDirStatus)setNodeCredentialsExtended:(NSString *)inRecordType authType:(NSString *)inAuthType authItems:(NSArray *)inAuthItems
							outAuthItems:(NSArray **)outAuthItems outContext:(sBDPIAuthContext *)outContext;

- (tDirStatus)verifyCredentials:(NSString *)inRecordType recordName:(NSString *)inRecordName password:(NSString *)inPassword;
- (tDirStatus)verifyCredentialsExtended:(NSString *)inRecordType authType:(NSString *)inAuthType authItems:(NSArray *)inAuthItems 
						   outAuthItems:(NSArray **)outAuthItems outContext:(sBDPIAuthContext *)outContext;

- (tDirStatus)changePassword:(NSString *)inRecordType recordName:(NSString *)inRecordName oldPassword:(NSString *)inOldPassword
				 newPassword:(NSString *)inNewPassword;
- (tDirStatus)changePasswordExtended:(NSString *)inRecordType authType:(NSString *)inAuthType authItems:(NSArray *)inAuthItems
						outAuthItems:(NSArray **)outAuthItems outContext:(sBDPIAuthContext *)outContext;

- (NSDictionary *)copyPasswordPolicyForRecord:(NSString *)inRecordType withName:(NSString *)inName;

- (tDirStatus)searchRecords:(sBDPISearchRecordsContext *)inContext buffer:(BDPIOpaqueBuffer)inBuffer outCount:(UInt32 *)outCount;

- (NSMutableDictionary *)recordOpenName:(NSString *)inRecordName recordType:(NSString *)inRecordType;
- (tDirStatus)recordCreateName:(NSString *)inRecordName recordType:(NSString *)inRecordType;
- (tDirStatus)recordDelete:(NSDictionary *)inRecord;
- (tDirStatus)recordFlush:(NSDictionary *)inRecord;
- (tDirStatus)recordClose:(NSDictionary *)inRecord;
- (tDirStatus)record:(NSMutableDictionary *)inRecord setRecordType:(NSString *)inRecordType;
- (tDirStatus)record:(NSMutableDictionary *)inRecord setValues:(NSArray *)inValues forAttribute:(NSString *)inAttribute;
- (tDirStatus)record:(NSMutableDictionary *)inRecord addValues:(NSArray *)inValues toAttribute:(NSString *)inAttribute;
- (tDirStatus)record:(NSMutableDictionary *)inRecord removeValue:(id)inValue fromAttribute:(NSString *)inAttribute;

- (BOOL)allowChanges:(NSDictionary *)inRecord forAttribute:(NSString *)inAttribute;
- (UInt32)maximumSize:(NSString *)inRecordType forAttribute:(NSString *)inAttribute;

@end

#endif

#endif
