/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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

#import <Foundation/NSPrivateDecls.h>
#import "ODRecord.h"
#import "CFODRecord.h"
#import "CFOpenDirectoryPriv.h"

@interface NSODRecord : ODRecord
@end
@implementation NSODRecord
extern Boolean _CFIsDeallocating(CFTypeRef);
extern CFTypeRef _CFTryRetain(CFTypeRef);
CF_CLASSIMPLEMENTATION(ODRecord)
- (BOOL)_isDeallocating { return _CFIsDeallocating((CFTypeRef)self); }
- (BOOL)_tryRetain { return _CFTryRetain((CFTypeRef)self) != NULL; }
@end

@implementation ODRecord

- (ODRecordRef)__getODRecordRef
{
	ODRecordRef record;

	if ([self class] == [NSODRecord class]) {
		record = (ODRecordRef)self;
	} else {
		abort(); // TODO: can this ever happen?
		//record = _internal;
	}

	return record;
}

- (NSString *)description
{
	return [(NSString *)NSMakeCollectable(CFCopyDescription([self __getODRecordRef])) autorelease];
}

- (CFTypeID)_cfTypeID
{
	return ODRecordGetTypeID();
}

- (BOOL)setNodeCredentials:(NSString *)username password:(NSString *)password error:(NSError **)error
{
	bool success = ODRecordSetNodeCredentials([self __getODRecordRef], (CFStringRef)username, (CFStringRef)password, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

- (BOOL)setNodeCredentialsWithRecordType:(ODRecordType)recordType authenticationType:(ODAuthenticationType)authType authenticationItems:(NSArray *)authItems continueItems:(NSArray **)outItems context:(id *)context error:(NSError **)error
{
	bool success = ODRecordSetNodeCredentialsExtended([self __getODRecordRef], recordType, authType, (CFArrayRef)authItems, (CFArrayRef *)outItems, (ODContextRef *)context, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

- (BOOL)setNodeCredentialsUsingKerberosCache:(NSString *)cache error:(NSError **)error
{
	bool success = ODRecordSetNodeCredentialsUsingKerberosCache([self __getODRecordRef], (CFStringRef)cache, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

- (NSDictionary *)passwordPolicyAndReturnError:(NSError **)error
{
	NSDictionary *result = (NSDictionary *)ODRecordCopyPasswordPolicy(NULL, [self __getODRecordRef], (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return [NSMakeCollectable(result) autorelease];
}

- (BOOL)verifyPassword:(NSString *)password error:(NSError **)error
{
	bool success = ODRecordVerifyPassword([self __getODRecordRef], (CFStringRef)password, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

- (BOOL)verifyExtendedWithAuthenticationType:(ODAuthenticationType)type authenticationItems:(NSArray *)items continueItems:(NSArray **)outItems context:(id *)context error:(NSError **)error
{
	bool success = ODRecordVerifyPasswordExtended([self __getODRecordRef], type, (CFArrayRef)items, (CFArrayRef *)outItems, (ODContextRef *)context, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

- (BOOL)changePassword:(NSString *)oldPassword toPassword:(NSString *)newPassword error:(NSError **)error
{
	bool success = ODRecordChangePassword([self __getODRecordRef], (CFStringRef)oldPassword, (CFStringRef)newPassword, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

- (BOOL)synchronizeAndReturnError:(NSError **)error
{
	bool success = ODRecordSynchronize([self __getODRecordRef], (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

- (NSDictionary *)recordDetailsForAttributes:(NSArray *)attributes error:(NSError **)error
{
	NSDictionary *result = (NSDictionary *)ODRecordCopyDetails([self __getODRecordRef], (CFArrayRef)attributes, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return [NSMakeCollectable(result) autorelease];
}

- (NSArray *)valuesForAttribute:(ODAttributeType)attribute error:(NSError **)error
{
	NSArray *result = (NSArray *)ODRecordCopyValues([self __getODRecordRef], attribute, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return [NSMakeCollectable(result) autorelease];
}

- (NSString *)recordType
{
	return (NSString *)ODRecordGetRecordType([self __getODRecordRef]);
}

- (NSString *)recordName
{
	return (NSString *)ODRecordGetRecordName([self __getODRecordRef]);
}

- (BOOL)deleteRecordAndReturnError:(NSError **)error
{
	bool success = ODRecordDelete([self __getODRecordRef], (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

- (BOOL)setValue:(id)value forAttribute:(ODAttributeType)attribute error:(NSError **)error
{
	bool success = ODRecordSetValue([self __getODRecordRef], attribute, value, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

- (BOOL)removeValuesForAttribute:(ODAttributeType)attribute error:(NSError **)error
{
	bool success = ODRecordSetValue([self __getODRecordRef], attribute, [NSArray array], (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

- (BOOL)addValue:(id)value toAttribute:(ODAttributeType)attribute error:(NSError **)error
{
	bool success = ODRecordAddValue([self __getODRecordRef], attribute, value, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

- (BOOL)removeValue:(id)value fromAttribute:(ODAttributeType)attribute error:(NSError **)error
{
	bool success = ODRecordRemoveValue([self __getODRecordRef], attribute, value, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

- (BOOL)addMemberRecord:(ODRecord *)record error:(NSError **)error
{
	bool success = ODRecordAddMember([self __getODRecordRef], [record __getODRecordRef], (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

- (BOOL)removeMemberRecord:(ODRecord *)record error:(NSError **)error
{
	bool success = ODRecordRemoveMember([self __getODRecordRef], [record __getODRecordRef], (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

- (BOOL)isMemberRecord:(ODRecord *)record error:(NSError **)error
{
	bool success = ODRecordContainsMember([self __getODRecordRef], [record __getODRecordRef], (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

// Private...
- (BOOL)isMemberRecordRefresh:(ODRecord *)record error:(NSError **)error
{
	bool success = ODRecordContainsMemberRefresh([self __getODRecordRef], [record __getODRecordRef], (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

@end

@implementation ODRecord (NSKeyValueCoding)

+ (BOOL)accessInstanceVariablesDirectly
{
	return NO;
}

- (id)valueForUndefinedKey:(NSString *)key
{
	if ([self class] == [NSODRecord class]) {
		return [self valuesForAttribute:key error:nil];
	}

	NSRequestConcreteImplementation(self, _cmd, [ODRecord class]);
	return [super valueForUndefinedKey:key];
}

@end
