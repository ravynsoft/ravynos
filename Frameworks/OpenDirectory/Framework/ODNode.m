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

#import <Foundation/Foundation.h>
#import <Foundation/NSPrivateDecls.h>
#import "ODNode.h"
#import "internal.h"

// private accessor
@interface ODSession (priv)
- (ODSessionRef)__getODSessionRef;
@end

@interface NSODNode : ODNode
@end
@implementation NSODNode
extern Boolean _CFIsDeallocating(CFTypeRef);
extern CFTypeRef _CFTryRetain(CFTypeRef);
CF_CLASSIMPLEMENTATION(ODNode)
- (BOOL)_isDeallocating { return _CFIsDeallocating((CFTypeRef)self); }
- (BOOL)_tryRetain { return _CFTryRetain((CFTypeRef)self) != NULL; }
@end

@implementation ODNode

- (ODNodeRef)__getODNodeRef
{
	ODNodeRef node;

	if ([self class] == [NSODNode class]) {
		node = (ODNodeRef)self;
	} else {
		node = _internal;
	}

	return node;
}

- (NSString *)description
{
	return [(NSString *)NSMakeCollectable(CFCopyDescription([self __getODNodeRef])) autorelease];
}

- (CFTypeID)_cfTypeID
{
	return ODNodeGetTypeID();
}

+ (void) initialize
{
	_ODInitialize();
}

+ (id)allocWithZone:(NSZone *)inZone
{
    if ([self class] == [ODNode class]) {
        return (id) _ODNodeCreate(NULL);
	} else {
        return [super allocWithZone: inZone];
	}
}

+ (ODNode *)nodeWithSession:(ODSession *)session name:(NSString *)name error:(NSError **)error
{
	return [[[self alloc] initWithSession:session name:name error:error] autorelease];
}

+ (ODNode *)nodeWithSession:(ODSession *)session type:(ODNodeType)type error:(NSError **)error
{
	NSString *nodename = (NSString *)_NodeGetNodeTypeName(type);
	return [[[self alloc] initWithSession:session name:nodename error:error] autorelease];
}

- (ODNode *)initWithSession:(ODSession *)session name:(NSString *)name error:(NSError **)error
{
    CFErrorRef  local_error = NULL;

	if ([self class] == [NSODNode class]) {
		_ODNodeInit((ODNodeRef) self, [session __getODSessionRef], (CFStringRef)name, &local_error);
	} else {
		_internal = ODNodeCreateWithName(NULL, [session __getODSessionRef], (CFStringRef)name, &local_error);
	}
	
	// if we had an error, let's release ourselves and fail
	CLEAR_ERROR(error);
	if (local_error != NULL) {
		[self release];
		self = nil;
		
		if (error != nil) {
			(*error) = [NSMakeCollectable(local_error) autorelease];
		} else {
			CFRelease(local_error);
		}
	}	
	
	return NSMakeCollectable(self);
}

- (ODNode *)initWithSession:(ODSession *)session type:(ODNodeType)type error:(NSError **)error
{
	NSString *nodename = (NSString *)_NodeGetNodeTypeName(type);
	return [self initWithSession:session name:nodename error:error];
}

- (void)dealloc
{
	if ([self class] != [NSODNode class]) {
		if (_internal) CFRelease(_internal);
	}

	[super dealloc];
}

- (id)copy
{
	// TODO: is this really right? what about subclassing implications?
	return (id) ODNodeCreateCopy(NULL, [self __getODNodeRef], NULL);
}

- (NSArray *)subnodeNamesAndReturnError:(NSError **)error
{
	NSArray *result = (NSArray *)ODNodeCopySubnodeNames([self __getODNodeRef], (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return [NSMakeCollectable(result) autorelease];
}

- (NSArray *)unreachableSubnodeNamesAndReturnError:(NSError **)error
{
	NSArray *result = (NSArray *)ODNodeCopyUnreachableSubnodeNames([self __getODNodeRef], (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return [NSMakeCollectable(result) autorelease];
}

- (NSString *)nodeName
{
	return (NSString *)ODNodeGetName([self __getODNodeRef]);
}

- (NSDictionary *)nodeDetailsForKeys:(NSArray *)keys error:(NSError **)error
{
	NSDictionary *result = (NSDictionary *)ODNodeCopyDetails([self __getODNodeRef], (CFArrayRef)keys, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return [NSMakeCollectable(result) autorelease];
}

- (NSArray *)supportedRecordTypesAndReturnError:(NSError **)error
{
	NSArray *result = (NSArray *)ODNodeCopySupportedRecordTypes([self __getODNodeRef], (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return [NSMakeCollectable(result) autorelease];
}

- (NSArray *)supportedAttributesForRecordType:(ODRecordType)recordType error:(NSError **)error
{
	NSArray *result = (NSArray *)ODNodeCopySupportedAttributes([self __getODNodeRef], recordType, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return [NSMakeCollectable(result) autorelease];
}

- (BOOL)setCredentialsWithRecordType:(ODRecordType)recordType recordName:(NSString *)recordName password:(NSString *)password error:(NSError **)error
{
	bool success = ODNodeSetCredentials([self __getODNodeRef], recordType, (CFStringRef)recordName, (CFStringRef)password, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

- (BOOL)setCredentialsWithRecordType:(ODRecordType)recordType authenticationType:(ODAuthenticationType)authType authenticationItems:(NSArray *)authItems continueItems:(NSArray **)authItemsOut context:(id *)context error:(NSError **)error
{
	bool success = ODNodeSetCredentialsExtended([self __getODNodeRef], recordType, authType, (CFArrayRef)authItems, (CFArrayRef *)authItemsOut, (ODContextRef *)context, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

- (BOOL)setCredentialsUsingKerberosCache:(NSString *)cache error:(NSError **)error
{
	bool success = ODNodeSetCredentialsUsingKerberosCache([self __getODNodeRef], (CFStringRef)cache, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return success;
}

- (ODRecord *)createRecordWithRecordType:(ODRecordType)recordType name:(NSString *)recordName attributes:(NSDictionary *)attributes error:(NSError **)error
{
	ODRecord *result = (ODRecord *)ODNodeCreateRecord([self __getODNodeRef], recordType, (CFStringRef)recordName, (CFDictionaryRef)attributes, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return [NSMakeCollectable(result) autorelease];
}

- (ODRecord *)recordWithRecordType:(ODRecordType)recordType name:(NSString *)recordName attributes:(id)attributes error:(NSError **)error
{
	ODRecord *result = (ODRecord *)ODNodeCopyRecord([self __getODNodeRef], recordType, (CFStringRef)recordName, (CFTypeRef)attributes, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return [NSMakeCollectable(result) autorelease];
}

- (NSData *)customCall:(NSInteger)customCode sendData:(NSData *)sendData error:(NSError **)error
{
	NSData *result = (NSData *)ODNodeCustomCall([self __getODNodeRef], customCode, (CFDataRef)sendData, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return [NSMakeCollectable(result) autorelease];
}

@end
