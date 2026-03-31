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

#include <CrashReporterClient.h>

#define OD_API_MISUSE(msg) do { \
	CRSetCrashLogMessage("Bug in client of OpenDirectory.framework: " msg); \
	__builtin_trap(); \
} while (0)

#define OD_CRASH(msg) do { \
	CRSetCrashLogMessage("Bug: " msg); \
	__builtin_trap(); \
} while (0)

#define safe_cfrelease(a)			do { if (a != NULL) { CFRelease(a); } } while(0)
#define safe_cfrelease_null(a)		do { if (a != NULL) { CFRelease(a); a = NULL; } } while(0)
#define safe_dispatch_release(a)	do { if (a != NULL) { dispatch_release(a); a = NULL; } } while(0)
#define safe_block_release_null(a)	do { if (a != NULL) { Block_release(a); a = NULL; } } while (0)

__attribute__((always_inline)) static inline CFTypeRef
safe_cfretain(CFTypeRef cf) 
{
	if (cf != NULL) {
		return CFRetain(cf);
	}
	
	return NULL;
}

void _ODInitialize(void);

ODRecordRef _RecordCreate(ODNodeRef node, CFSetRef fetched, CFDictionaryRef attributes);

ODSessionRef _NodeGetSession(ODNodeRef node);
void uuid_copy_node(uuid_t dst, ODNodeRef node);
CFStringRef _NodeGetNodeTypeName(ODNodeType type);

void uuid_copy_session(uuid_t dst, ODSessionRef session);
ODSessionRef _ODSessionGetShared(void);
void _ODSessionInit(ODSessionRef session, CFDictionaryRef options, CFErrorRef *error);
ODSessionRef _ODSessionCreate(CFAllocatorRef allocator);

ODNodeRef _ODNodeCreate(CFAllocatorRef allocator);
void _ODNodeInit(ODNodeRef node, ODSessionRef session, CFStringRef name, CFErrorRef *error);
bool _ODNodeRecover(ODNodeRef node);
bool _ODNodeSetCredentialsFromOtherNode(ODNodeRef node, ODNodeRef otherNode, CFErrorRef *error);

ODQueryRef _ODQueryCreate(CFAllocatorRef allocator);
ODQueryRef _ODQueryInit(ODQueryRef query, ODNodeRef node, CFTypeRef recordTypeOrList, ODAttributeType attribute, ODMatchType matchType, CFTypeRef queryValueOrList, CFTypeRef returnAttributeOrList, CFIndex maxResults, CFErrorRef *error);

void _ODErrorSet(CFErrorRef *error, uint32_t code, CFStringRef errorInfo);

void _ODQuerySetDelegate(ODQueryRef query, void *delegate);
void *_ODQueryGetDelegate(ODQueryRef query);
void _ODQuerySetOperationQueue(ODQueryRef query, void *operationQueue);
void *_ODQueryGetOperationQueue(ODQueryRef query);

#define CLEAR_ERROR(e) do { if (e) *e = NULL; } while (0)
bool _validate_nonnull(CFErrorRef *error, CFStringRef desc, const void *ptr);

// TODO: move these out of CFODNode.c
CFArrayRef CFArrayCreateWithSet(CFSetRef set);
CFSetRef attrset_create_minimized_copy(CFTypeRef attrset);
bool attrset_contains_attribute(CFSetRef attrset, CFStringRef attr);
