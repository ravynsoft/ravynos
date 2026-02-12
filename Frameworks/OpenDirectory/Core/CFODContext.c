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

#include <CoreFoundation/CFBridgingPriv.h>
#include <uuid/uuid.h>

#include "CFODContext.h"
#include "internal.h"
#include "transaction.h"
#include "context_internal.h"

struct __ODContext {
	CFRuntimeBase _base;
	ODNodeRef _node;
	CFDataRef _uuid;
};

static CFTypeID __kODContextTypeID = _kCFRuntimeNotATypeID;

static void
__ODContextFinalize(CFTypeRef cf)
{
	ODContextRef context = (ODContextRef)cf;
	CFArrayRef response;
	uint32_t code;

	response = transaction_simple(&code, _NodeGetSession(context->_node), context->_node, CFSTR("ODContextRelease"), 1, context->_uuid);
	if (response != NULL) {
		CFRelease(response);
	}

	CFRelease(context->_node);
	CFRelease(context->_uuid);
}

static CFStringRef
__ODContextCopyDebugDesc(CFTypeRef cf)
{
	ODContextRef context = (ODContextRef)cf;
	uuid_string_t uuidstr;
	uuid_unparse_upper(CFDataGetBytePtr(context->_uuid), uuidstr);
	return CFStringCreateWithFormat(NULL, NULL, CFSTR("<ODContext %p [node: %@] [uuid: %s]>"), context, ODNodeGetName(context->_node), uuidstr);
}

static const CFRuntimeClass __ODContextClass = {
	0,								// version
	"ODContext",					// className
	NULL,							// init
	NULL,							// copy
	__ODContextFinalize,			// finalize
	NULL,							// equal
	NULL,							// hash
	NULL,							// copyFormattingDesc
	__ODContextCopyDebugDesc,		// copyDebugDesc
	NULL,							// reclaim
#if CF_REFCOUNT_AVAILABLE
	NULL,							// refcount
#endif
};

CFTypeID
ODContextGetTypeID(void)
{
	static dispatch_once_t once;

	dispatch_once(&once, ^ {
		__kODContextTypeID = _CFRuntimeRegisterClass(&__ODContextClass);
		if (__kODContextTypeID != _kCFRuntimeNotATypeID) {
			_CFRuntimeBridgeClasses(__kODContextTypeID, "NSODContext");
		}
	});

	return __kODContextTypeID;
}

ODContextRef
_ODContextCreateWithNodeAndUUID(CFAllocatorRef allocator, ODNodeRef node, CFDataRef uuid)
{
	struct __ODContext *context = NULL;

	if (uuid != NULL) {
		context = (struct __ODContext *)_CFRuntimeCreateInstance(allocator, ODContextGetTypeID(), sizeof(struct __ODContext) - sizeof(CFRuntimeBase), NULL);

		context->_node = (ODNodeRef)CFRetain(node);
		context->_uuid = CFRetain(uuid);
	}

	return context;
}

CFDataRef
_ODContextGetUUID(ODContextRef context)
{
	if (context != NULL) {
		return context->_uuid;
	}
	
	return NULL;
}
