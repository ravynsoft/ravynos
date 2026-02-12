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
#import "ODSession.h"
#import "CFODSession.h"
#import "internal.h" // for _ODSessionGetShared

// TODO: this should go somewhere appropriate...
NSString *const ODFrameworkErrorDomain = @"com.apple.OpenDirectory";

NSString *const ODSessionProxyAddress = @"ProxyAddress";
NSString *const ODSessionProxyPort = @"ProxyPort";
NSString *const ODSessionProxyUsername = @"ProxyUsername";
NSString *const ODSessionProxyPassword = @"ProxyPassword";

@interface NSODSession : ODSession
@end

@implementation NSODSession
extern Boolean _CFIsDeallocating(CFTypeRef);
extern CFTypeRef _CFTryRetain(CFTypeRef);
CF_CLASSIMPLEMENTATION(ODSession)
- (BOOL)_isDeallocating { return _CFIsDeallocating((CFTypeRef)self); }
- (BOOL)_tryRetain { return _CFTryRetain((CFTypeRef)self) != NULL; }
@end

@implementation ODSession

- (ODSessionRef)__getODSessionRef
{
	ODSessionRef session;

	if ([self class] == [NSODSession class]) {
		session = (ODSessionRef)self;
	} else {
		session = _internal;
	}

	return session;
}

- (NSString *)description
{
	return [(NSString *)NSMakeCollectable(CFCopyDescription([self __getODSessionRef])) autorelease];
}

- (CFTypeID)_cfTypeID
{
	return ODSessionGetTypeID();
}

+ (void) initialize
{
	_ODInitialize();
}

+ (id)allocWithZone:(NSZone *)inZone
{
    if ([self class] == [ODSession class]) {
        return (id) _ODSessionCreate(NULL);
	} else {
        return [super allocWithZone: inZone];
	}
}

+ (id)defaultSession
{
	return (id)_ODSessionGetShared();
}

+ (id)sessionWithOptions:(NSDictionary *)options error:(NSError **)error
{
	return [[[self alloc] initWithOptions:options error:error] autorelease];
}

- (id)initWithOptions:(NSDictionary *)options error:(NSError **)error
{
	CFErrorRef local_error = NULL;

	if ([self class] == [NSODSession class]) {
		_ODSessionInit((ODSessionRef) self, (CFDictionaryRef)options, &local_error);
	} else {
		_internal = ODSessionCreate(NULL, (CFDictionaryRef)options, &local_error);
	}
	
	if (local_error != NULL) {
		[self release];
		self = nil;
		
		if (error != nil) {
			(*error) = [NSMakeCollectable(local_error) autorelease];
		} else {
			CFRelease(local_error);
		}
	} else {
		if (error != nil) {
			*error = nil;
		}
	}
	
	return NSMakeCollectable(self);
}

- (void)dealloc
{
	if ([self class] != [NSODSession class]) {
		CFRelease(_internal);
	}

	[super dealloc];
}

- (NSArray *)nodeNamesAndReturnError:(NSError **)error
{
	NSArray *result = (NSArray *)ODSessionCopyNodeNames(NULL, [self __getODSessionRef], (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return [NSMakeCollectable(result) autorelease];
}

@end
