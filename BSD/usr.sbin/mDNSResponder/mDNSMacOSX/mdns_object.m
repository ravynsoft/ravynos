/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import "mdns_object.h"

#import <CoreUtils/CoreUtils.h>
#import <Foundation/Foundation.h>
#import <os/object_private.h>

//======================================================================================================================
// MARK: - Class Declarations

#define MDNS_OBJECT_CLASS_DECLARE(NAME)								\
	_OS_OBJECT_DECL_SUBCLASS_INTERFACE(mdns_ ## NAME, mdns_object)	\
	extern int _mdns_dummy_variable

_OS_OBJECT_DECL_SUBCLASS_INTERFACE(mdns_object, object)

MDNS_OBJECT_CLASS_DECLARE(interface_monitor);

//======================================================================================================================
// MARK: - Class Definitions

@implementation OS_OBJECT_CLASS(mdns_object)
- (void)dealloc
{
	mdns_object_finalize(self);
	arc_safe_super_dealloc();
}

- (NSString *)description
{
	return arc_safe_autorelease((NSString *)mdns_object_copy_description_as_cfstring(self, false, false));
}

- (NSString *)debugDescription
{
	return arc_safe_autorelease((NSString *)mdns_object_copy_description_as_cfstring(self, true, false));
}

- (NSString *)redactedDescription
{
	return arc_safe_autorelease((NSString *)mdns_object_copy_description_as_cfstring(self, false, true));
}
@end

#define MDNS_CLASS(NAME)	OS_OBJECT_CLASS(mdns_ ## NAME)
#define MDNS_OBJECT_CLASS_DEFINE(NAME)												\
	@implementation MDNS_CLASS(NAME)												\
	@end																			\
																					\
	mdns_ ## NAME ## _t																\
	mdns_object_ ## NAME ## _alloc(const size_t size)								\
	{																				\
		return (mdns_## NAME ##_t)_os_object_alloc([MDNS_CLASS(NAME) class], size);	\
	}																				\
	extern int _mdns_dummy_variable

MDNS_OBJECT_CLASS_DEFINE(interface_monitor);
