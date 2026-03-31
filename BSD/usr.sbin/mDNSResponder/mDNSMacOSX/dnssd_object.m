/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import "dnssd_object.h"

#import <stdlib.h>
#import <Foundation/Foundation.h>
#import <os/object_private.h>

#if 0
//======================================================================================================================
#pragma mark - Class Declarations
#endif

#define DNSSD_OBJECT_CLASS_DECLARE(NAME)								\
	_OS_OBJECT_DECL_SUBCLASS_INTERFACE(dnssd_ ## NAME, dnssd_object)	\
	extern int _dnssd_dummy_variable

_OS_OBJECT_DECL_SUBCLASS_INTERFACE(dnssd_object, object)

DNSSD_OBJECT_CLASS_DECLARE(getaddrinfo);
DNSSD_OBJECT_CLASS_DECLARE(getaddrinfo_result);

#if 0
//======================================================================================================================
#pragma mark - Class Definitions
#endif

@implementation OS_OBJECT_CLASS(dnssd_object)
- (void)dealloc
{
	dnssd_object_finalize(self);
	[super dealloc];
}

- (NSString *)description
{
	char * const desc = dnssd_object_copy_description(self, false, false);
	if (desc) {
		NSString * const string = [NSString stringWithUTF8String:desc];
		free(desc);
		return string;
	} else {
		return nil;
	}
}

- (NSString *)debugDescription
{
	char * const desc = dnssd_object_copy_description(self, true, false);
	if (desc) {
		NSString * const string = [NSString stringWithUTF8String:desc];
		free(desc);
		return string;
	} else {
		return nil;
	}
}

- (NSString *)redactedDescription
{
	char * const desc = dnssd_object_copy_description(self, false, true);
	if (desc) {
		NSString * const string = [NSString stringWithUTF8String:desc];
		free(desc);
		return string;
	} else {
		return nil;
	}
}
@end

#define DNSSD_CLASS(NAME)	OS_OBJECT_CLASS(dnssd_ ## NAME)
#define DNSSD_OBJECT_CLASS_DEFINE(NAME)													\
	@implementation DNSSD_CLASS(NAME)													\
	@end																				\
																						\
	dnssd_ ## NAME ## _t																\
	dnssd_object_ ## NAME ## _alloc(const size_t size)									\
	{																					\
		return (dnssd_## NAME ##_t)_os_object_alloc([DNSSD_CLASS(NAME) class], size);	\
	}																					\
	extern int _dnssd_dummy_variable

DNSSD_OBJECT_CLASS_DEFINE(getaddrinfo);
DNSSD_OBJECT_CLASS_DEFINE(getaddrinfo_result);
