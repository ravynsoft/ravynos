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

#ifndef __MDNS_OBJECT_H__
#define __MDNS_OBJECT_H__

#include "mdns_private.h"

//======================================================================================================================
// MARK: - mdns_object Private Method Declarations

char *
mdns_object_copy_description(mdns_any_t object, bool debug, bool privacy);

CFStringRef
mdns_object_copy_description_as_cfstring(mdns_any_t object, bool debug, bool privacy);

void
mdns_object_finalize(mdns_any_t object);

#define MDNS_OBJECT_ALLOC_DECLARE(NAME)		\
	mdns_ ## NAME ## _t						\
	mdns_object_ ## NAME ## _alloc(size_t size)

MDNS_OBJECT_ALLOC_DECLARE(interface_monitor);

#endif	// __MDNS_OBJECT_H__
