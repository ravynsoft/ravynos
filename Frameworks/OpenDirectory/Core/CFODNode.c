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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFBridgingPriv.h>
#include <dispatch/dispatch.h>
#include <uuid/uuid.h>
#include <asl.h>
#include <assumes.h>

#include <opendirectory/odutils.h>

#include "CFODNode.h"
#include "CFODSession.h"
#include "CFOpenDirectoryPriv.h"
#include "internal.h"
#include "transaction.h"
#include "extauth.h"
#include "context_internal.h"

static bool _ODNodeExternalCreate(ODNodeRef node, CFErrorRef *error);
static void _ODNodeExternalRelease(ODNodeRef node);

#pragma mark Class Setup

struct __ODNode {
	CFRuntimeBase _base;
	ODSessionRef _session;
	CFStringRef _name;
	uuid_t _uuid;
	bool _cached;
	CFStringRef _creds_type;
	CFStringRef _creds_name;
	CFStringRef _creds_password;
};

static CFTypeID __kODNodeTypeID = _kCFRuntimeNotATypeID;

#pragma Node Caching

struct node_cache_entry {
	uuid_t uuid;
	uint64_t rc;
};

static dispatch_queue_t
_get_nc_queue(void)
{
	static dispatch_once_t once;
	static dispatch_queue_t queue;

	dispatch_once(&once, ^{
		queue = dispatch_queue_create("com.apple.OpenDirectory.node-cache", NULL);
	});

	return queue;
}

static CFMutableDictionaryRef
_get_nc_dict(void)
{
	static dispatch_once_t once;
	static CFMutableDictionaryRef dict;

	dispatch_once(&once, ^{
		dict = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	});

	return dict;
}

// for example:
// "00000000-0000-0000-0000-000000000000-/Local/Default"
static CFStringRef
_copy_node_key(ODNodeRef node)
{
	uuid_t session_uuid;
	uuid_string_t session_uuid_string;
	CFStringRef key;

	uuid_copy_session(session_uuid, node->_session);
	uuid_unparse_upper(session_uuid, session_uuid_string);
	key = CFStringCreateWithFormat(NULL, NULL, CFSTR("%s-%@"), session_uuid_string, node->_name);

	return key;
}

static bool
node_cache_get_and_retain(ODNodeRef node, CFErrorRef *error)
{
	CFStringRef key;
	__block bool success = false;

	(void)osx_assumes_zero(node->_cached);

	key = _copy_node_key(node);

	dispatch_sync(_get_nc_queue(), ^{
		CFDataRef value;
		struct node_cache_entry entry;
		CFDataRef new_value;

		value = CFDictionaryGetValue(_get_nc_dict(), key);
		if (value != NULL) {
			// found; bump retain count and set the node's uuid
			CFDataGetBytes(value, CFRangeMake(0, CFDataGetLength(value)), (UInt8 *)&entry);
			entry.rc += 1;
			uuid_copy(node->_uuid, entry.uuid);

			new_value = CFDataCreate(NULL, (const UInt8 *)&entry, sizeof(entry));
			CFDictionarySetValue(_get_nc_dict(), key, new_value);
			CFRelease(new_value);
			node->_cached = true;
			success = true;
		}
	});

	if (!success) {
		// not found; open a new node and create a new entry
		if (_ODNodeExternalCreate(node, error)) {
			dispatch_sync(_get_nc_queue(), ^{
				struct node_cache_entry entry;
				CFDataRef new_value;

				// someone might have beaten us to the punch, so the node we just created will not be cached
				if (!CFDictionaryContainsKey(_get_nc_dict(), key)) {
					uuid_copy(entry.uuid, node->_uuid);
					entry.rc = 1;

					new_value = CFDataCreate(NULL, (const UInt8 *)&entry, sizeof(entry));
					CFDictionarySetValue(_get_nc_dict(), key, new_value);
					CFRelease(new_value);
					node->_cached = true;
				}
			});

			success = true;
		} else {
			uuid_clear(node->_uuid);
		}
	}

	CFRelease(key);
	
	return success;
}

static void
node_cache_release(ODNodeRef node)
{
	CFStringRef key;

	(void)osx_assumes(node->_cached);

	node->_cached = false;

	key = _copy_node_key(node);

	dispatch_sync(_get_nc_queue(), ^{
		CFDataRef value;
		struct node_cache_entry entry;
		CFDataRef new_value;

		value = CFDictionaryGetValue(_get_nc_dict(), key);
		if (value != NULL) {
			CFDataGetBytes(value, CFRangeMake(0, CFDataGetLength(value)), (UInt8 *)&entry);
			entry.rc -= 1;
			if (entry.rc == 0) {
				CFDictionaryRemoveValue(_get_nc_dict(), key);
				_ODNodeExternalRelease(node);
			} else {
				new_value = CFDataCreate(NULL, (const UInt8 *)&entry, sizeof(entry));
				CFDictionarySetValue(_get_nc_dict(), key, new_value);
				CFRelease(new_value);
			}
		}
		// 7454503: If the node was marked as cached, but was not found,
		// that means we recovered after opendirectoryd went away. Since
		// the UUID is invalid, we don't need to call _ODNodeExternalRelease
	});

	CFRelease(key);
}

#pragma mark

static void
__ODNodeFinalize(CFTypeRef cf)
{
	ODNodeRef node = (ODNodeRef)cf;

	_ODNodeExternalRelease(node);

	safe_cfrelease(node->_session);
	safe_cfrelease(node->_name);

	safe_cfrelease(node->_creds_type);
	safe_cfrelease(node->_creds_name);
	safe_cfrelease(node->_creds_password);
}

static CFStringRef
__ODNodeCopyDebugDesc(CFTypeRef cf)
{
	ODNodeRef node = (ODNodeRef)cf;
	uuid_t session_uuid;
	uuid_string_t node_uuidstr, session_uuidstr;
	CFStringRef str;

	uuid_unparse_upper(node->_uuid, node_uuidstr);
	uuid_clear(session_uuid);
	if (node->_session) {
		uuid_copy_session(session_uuid, node->_session);
	}
	uuid_unparse_upper(session_uuid, session_uuidstr);
	str = CFStringCreateWithFormat(NULL, NULL, CFSTR("<ODNode %p [name: %@] [uuid: %s] [session: %s]>"), node, node->_name, node_uuidstr, session_uuidstr);

	return str;
}

static const CFRuntimeClass __ODNodeClass = {
	0,								// version
	"ODNode",						// className
	NULL,							// init
	NULL,							// copy
	__ODNodeFinalize,				// finalize
	NULL,							// equal
	NULL,							// hash
	NULL,							// copyFormattingDesc
	__ODNodeCopyDebugDesc,			// copyDebugDesc
	NULL,							// reclaim
#if CF_REFCOUNT_AVAILABLE
	NULL,							// refcount
#endif
};

#pragma mark Accessors

ODSessionRef
_NodeGetSession(ODNodeRef node)
{
	return node->_session;
}

void
uuid_copy_node(uuid_t dst, ODNodeRef node)
{
	if (node) {
		uuid_copy(dst, node->_uuid);
	} else {
		uuid_clear(dst);
	}
}

#pragma mark Helper Functions

static CFStringRef
copy_translated_nodename(CFStringRef name)
{
	CFStringRef result = NULL;

	if (name != NULL) {
		if (CFStringCompare(name, CFSTR("/Search/Contacts"), 0) == kCFCompareEqualTo) {
			result = CFSTR("/Contacts");
		} else if (CFStringHasPrefix(name, CFSTR("/")) == false) {
			char tempName[512];

			result = CFStringCreateWithFormat(NULL, NULL, CFSTR("/%@"), name);

			// TODO: make this log caller address or allow them to break on the error
			CFStringGetCString(name, tempName, sizeof(tempName), kCFStringEncodingUTF8);
			asl_log(NULL, NULL, ASL_LEVEL_ERR, "OpenDirectory.framework - bug in client:  ODNodeCreate called with node name not prefixed with '/' - '%s'", tempName);
		} else {
			result = CFRetain(name);
		}
	}

	return result;
}

CFSetRef
attrset_create_minimized_copy(CFTypeRef attributes)
{
	CFMutableSetRef newattrs = NULL;
	CFTypeID type;
	CFIndex count;
	const void **values;
	CFSetRef attrset;
	bool standard, native;
	CFIndex i;

	if (attributes == NULL) {
		return NULL;
	}

	type = CFGetTypeID(attributes);
	if (type == CFArrayGetTypeID()) {
		count = CFArrayGetCount(attributes);
		values = calloc(count, sizeof(void *));
		CFArrayGetValues(attributes, CFRangeMake(0, count), values);
		attrset = CFSetCreate(NULL, values, count, &kCFTypeSetCallBacks);
	} else if (type == CFSetGetTypeID()) {
		count = CFSetGetCount(attributes);
		values = calloc(count, sizeof(void *));
		CFSetGetValues(attributes, values);
		attrset = CFRetain(attributes);
	} else {
		return NULL;
	}

	newattrs = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);

	standard = CFSetContainsValue(attrset, kODAttributeTypeStandardOnly);
	native = CFSetContainsValue(attrset, kODAttributeTypeNativeOnly);

	if (CFSetContainsValue(attrset, kODAttributeTypeAllAttributes) || (standard && native)) {
		CFSetAddValue(newattrs, kODAttributeTypeAllAttributes);
	} else {
		if (standard) {
			CFSetAddValue(newattrs, kODAttributeTypeStandardOnly);
		}

		if (native) {
			CFSetAddValue(newattrs, kODAttributeTypeNativeOnly);
		}

		for (i = 0; i < count; i++) {
			if (CFGetTypeID(values[i]) != CFStringGetTypeID()) {
				continue;
			}
			if (standard && CFStringHasPrefix(values[i], CFSTR("dsAttrTypeStandard:"))) {
				continue;
			}
			if (native && CFStringHasPrefix(values[i], CFSTR("dsAttrTypeNative:"))) {
				continue;
			}
			CFSetAddValue(newattrs, values[i]);
		}

		if (!standard) {
			CFSetAddValue(newattrs, kODAttributeTypeMetaNodeLocation);
			CFSetAddValue(newattrs, kODAttributeTypeMetaRecordName);
			CFSetAddValue(newattrs, kODAttributeTypeRecordType);
			CFSetAddValue(newattrs, kODAttributeTypeRecordName);
		}
	}

	CFRelease(attrset);
	free(values);

	return newattrs;
}

bool
attrset_contains_attribute(CFSetRef attrset, CFStringRef attr)
{
	bool standard, native;

	if (CFSetContainsValue(attrset, attr)) {
		return true;
	}

	if (CFSetContainsValue(attrset, kODAttributeTypeAllAttributes)) {
		return true;
	}

	standard = CFSetContainsValue(attrset, kODAttributeTypeStandardOnly);
	native = CFSetContainsValue(attrset, kODAttributeTypeNativeOnly);

	if (standard && native) {
		return true;
	}

	if (standard && CFStringHasPrefix(attr, CFSTR("dsAttrTypeStandard:"))) {
		return true;
	}

	if (native && CFStringHasPrefix(attr, CFSTR("dsAttrTypeNative:"))) {
		return true;
	}

	return false;
}

CFStringRef
_NodeGetNodeTypeName(ODNodeType type)
{
	CFStringRef name = NULL;

	switch (type) {
		case kODNodeTypeAuthentication:
		case kODNodeTypeNetwork:
			name = CFSTR("/Search");
			break;
		case kODNodeTypeContacts:
			name = CFSTR("/Contacts");
			break;
		case kODNodeTypeLocalNodes:
			name = CFSTR("/Local/Default");
			break;
		case kODNodeTypeConfigure:
			name = CFSTR("/Configure");
			break;
	}

	return name;
}

static bool
_ODNodeExternalCreate(ODNodeRef node, CFErrorRef *error)
{
	CFArrayRef response;
	uint32_t code;
	__block bool success = false;

	response = transaction_simple(&code, node->_session, NULL, CFSTR("ODNodeCreateWithName"), 1, node->_name);
	transaction_simple_response(response, code, 1, error, ^ {
		if (node->_cached) {
			node_cache_release(node);
		}
		uuid_copy(node->_uuid, CFDataGetBytePtr(schema_get_value_at_index(response, 0)));
		success = true;
	});

	return success;
}

static void
_ODNodeExternalRelease(ODNodeRef node)
{
	CFArrayRef response;
	uint32_t code;

	// node_cache_release will call _ODNodeExternalRelease if all references are gone
	if (node->_cached) {
		node_cache_release(node);
		return;
	}

	if (!uuid_is_null(node->_uuid)) {
		response = transaction_simple(&code, node->_session, node, CFSTR("ODNodeRelease"), 0);
		if (response != NULL) {
			CFRelease(response);
		}
	}
}

ODNodeRef
_ODNodeCreate(CFAllocatorRef allocator)
{
	return (ODNodeRef)_CFRuntimeCreateInstance(allocator, ODNodeGetTypeID(), sizeof(struct __ODNode) - sizeof(CFRuntimeBase), NULL);
}

void
_ODNodeInit(ODNodeRef node, ODSessionRef session, CFStringRef name, CFErrorRef *error)
{
	node->_cached = false;
	node->_session = (ODSessionRef)safe_cfretain(session);
	node->_name = copy_translated_nodename(name);

	/* Fetch from the cache or call _ODNodeReopen; sets the uuid */
	node_cache_get_and_retain(node, error);
}

bool
_ODNodeRecover(ODNodeRef node)
{
	CFStringRef key;

	// 8998966: prevent cascading failure; most likely means _ODRecordGetNode() failed.
	if (node == NULL) {
		return false;
	}

	key = _copy_node_key(node);

	dispatch_sync(_get_nc_queue(), ^{
		CFDictionaryRemoveValue(_get_nc_dict(), key);
	});

	CFRelease(key);

	node->_cached = false;
	return node_cache_get_and_retain(node, NULL);
}

static void
_ODNodeStoreCredentials(ODNodeRef node, CFStringRef type, CFTypeRef name, CFStringRef password)
{
	safe_cfrelease_null(node->_creds_type);
	safe_cfrelease_null(node->_creds_name);
	safe_cfrelease_null(node->_creds_password);

	node->_creds_type = CFStringCreateCopy(kCFAllocatorDefault, type);
	if (CFGetTypeID(name) == CFStringGetTypeID()) {
		node->_creds_name = CFStringCreateCopy(kCFAllocatorDefault, name);
	} else if (CFGetTypeID(name) == CFArrayGetTypeID() && CFArrayGetCount(name) > 0) {
		node->_creds_name = CFStringCreateCopy(kCFAllocatorDefault, CFArrayGetValueAtIndex(name, 0));
	}
	if (password != NULL) {
		node->_creds_password = CFStringCreateCopy(kCFAllocatorDefault, password);
	}
}

bool
_ODNodeSetCredentialsFromOtherNode(ODNodeRef node, ODNodeRef otherNode, CFErrorRef *error)
{
	return ODNodeSetCredentials(node, otherNode->_creds_type, otherNode->_creds_name, otherNode->_creds_password, error);
}

#pragma mark ODNodeRef Functions

CFTypeID
ODNodeGetTypeID(void)
{
	static dispatch_once_t once;

	dispatch_once(&once, ^ {
		__kODNodeTypeID = _CFRuntimeRegisterClass(&__ODNodeClass);
		if (__kODNodeTypeID != _kCFRuntimeNotATypeID) {
			_CFRuntimeBridgeClasses(__kODNodeTypeID, "NSODNode");
		}
	});

	return __kODNodeTypeID;
}

ODNodeRef
ODNodeCreateWithNodeType(CFAllocatorRef allocator, ODSessionRef session, ODNodeType type, CFErrorRef *error)
{
	CFStringRef name = _NodeGetNodeTypeName(type);
	ODNodeRef node = NULL;

	if (name) {
		node = ODNodeCreateWithName(allocator, session, name, error);
	} else {
		_ODErrorSet(error, kODErrorNodeUnknownType, NULL);
	}

	return node;
}

ODNodeRef
ODNodeCreateWithName(CFAllocatorRef allocator, ODSessionRef session, CFStringRef name, CFErrorRef *error)
{
	ODNodeRef node = NULL;
	CFErrorRef local_error = NULL;

	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("node name"), name)) {
		return NULL;
	}

	node = _ODNodeCreate(allocator);
	if (node != NULL) {
		_ODNodeInit(node, session, name, &local_error);
	}

	if (local_error != NULL) {
		CFRelease(node);
		node = NULL;

		if (error != NULL) {
			(*error) = local_error;
		} else {
			CFRelease(local_error);
		}
	}

	return node;
}

// Actually just creates a new node with identical properties, will get a new UUID.
// This is because each node has corresponding state in opendirectoryd.
ODNodeRef
ODNodeCreateCopy(CFAllocatorRef allocator, ODNodeRef node, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("node"), node)) {
		return NULL;
	}

	CF_OBJC_FUNCDISPATCH(__kODNodeTypeID, ODNodeRef, node, "copy");
	return ODNodeCreateWithName(allocator, node->_session, node->_name, error);
}

CFArrayRef
ODNodeCopySubnodeNames(ODNodeRef node, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("node"), node)) {
		return NULL;
	}
	if (CF_IS_OBJC(__kODNodeTypeID, node)) {
		CFArrayRef subnodes = NULL;
		CF_OBJC_CALL(CFArrayRef, subnodes, node, "subnodeNamesAndReturnError:", error);
		return subnodes ? CFRetain(subnodes) : NULL;
	}

	__block CFArrayRef nodes = NULL;
	CFArrayRef response;
	uint32_t code;

	response = transaction_simple(&code, node->_session, node, CFSTR("ODNodeCopySubnodeNames"), 0);
	transaction_simple_response(response, code, 1, error, ^ {
		nodes = schema_get_value_at_index(response, 0);
		if (nodes) CFRetain(nodes);
	});

	return nodes;
}

CFArrayRef
ODNodeCopyUnreachableSubnodeNames(ODNodeRef node, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("node"), node)) {
		return NULL;
	}
	if (CF_IS_OBJC(__kODNodeTypeID, node)) {
		CFArrayRef subnodes = NULL;
		CF_OBJC_CALL(CFArrayRef, subnodes, node, "unreachableSubnodeNamesAndReturnError:", error);
		return subnodes ? CFRetain(subnodes) : NULL;
	}

	__block CFArrayRef subnodes = NULL;
	CFArrayRef response;
	uint32_t code;

	response = transaction_simple(&code, node->_session, node, CFSTR("ODNodeCopyUnreachableSubnodeNames"), 0);
	transaction_simple_response(response, code, 1, error, ^ {
		subnodes = schema_get_value_at_index(response, 0);
		if (subnodes) CFRetain(subnodes);
	});

	return subnodes;
}

CFStringRef
ODNodeGetName(ODNodeRef node)
{
	if (node == NULL) {
		return NULL;
	}
	CF_OBJC_FUNCDISPATCH(__kODNodeTypeID, CFStringRef, node, "nodeName");
	return node->_name;
}

CFDictionaryRef
ODNodeCopyDetails(ODNodeRef node, CFArrayRef keys, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("node"), node)) {
		return NULL;
	}
	if (CF_IS_OBJC(__kODNodeTypeID, node)) {
		CFDictionaryRef details;
		CF_OBJC_CALL(CFDictionaryRef, details, node, "nodeDetailsForKeys:error:", keys, error);
		return details ? CFRetain(details) : NULL;
	}

	__block CFDictionaryRef details = NULL;
	CFArrayRef response;
	uint32_t code;

	response = transaction_simple(&code, node->_session, node, CFSTR("ODNodeCopyDetails"), 1, keys);
	transaction_simple_response(response, code, 1, error, ^ {
		details = CFRetain(schema_get_value_at_index(response, 0));
	});

	return details;
}

CFArrayRef
ODNodeCopySupportedRecordTypes(ODNodeRef node, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("node"), node)) {
		return NULL;
	}
	if (CF_IS_OBJC(__kODNodeTypeID, node)) {
		CFArrayRef types;
		CF_OBJC_CALL(CFArrayRef, types, node, "supportedRecordTypesAndReturnError:", error);
		return types ? CFRetain(types) : NULL;
	}

	__block CFArrayRef types = NULL;
	CFArrayRef response;
	uint32_t code;

	response = transaction_simple(&code, node->_session, node, CFSTR("ODNodeCopySupportedRecordTypes"), 0);
	transaction_simple_response(response, code, 1, error, ^ {
		types = CFRetain(schema_get_value_at_index(response, 0));
	});

	return types;
}

CFArrayRef
ODNodeCopySupportedAttributes(ODNodeRef node, ODRecordType recordType, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("node"), node)) {
		return NULL;
	}
	// TODO: ignoring record type for now??
	if (CF_IS_OBJC(__kODNodeTypeID, node)) {
		CFArrayRef attrs;
		CF_OBJC_CALL(CFArrayRef, attrs, node, "supportedAttributesForRecordType:error:", recordType, error);
		return attrs ? CFRetain(attrs) : NULL;
	}

	__block CFArrayRef attrs = NULL;
	CFArrayRef response;
	uint32_t code;

	response = transaction_simple(&code, node->_session, node, CFSTR("ODNodeCopySupportedAttributes"), 1, recordType);
	transaction_simple_response(response, code, 1, error, ^ {
		attrs = schema_get_value_at_index(response, 0);
		if (attrs != NULL) {
			CFRetain(attrs);
		} else {
			attrs = CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
		}
	});

	return attrs;
}

bool
ODNodeSetCredentials(ODNodeRef node, ODRecordType recordType, CFStringRef recordName, CFStringRef password, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("node"), node)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("record name"), recordName)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("password"), password)) {
		return false;
	}

	CF_OBJC_FUNCDISPATCH(__kODNodeTypeID, bool, node, "setCredentialsWithRecordType:recordName:password:error:", recordType, recordName, password, error);

	__block bool success = false;
	CFStringRef rectype;
	CFArrayRef response;
	uint32_t code;

	// need to open a new backing node
	if (!_ODNodeExternalCreate(node, error)) {
		return false;
	}

	rectype = recordType ? recordType : kODRecordTypeUsers;
	response = transaction_simple(&code, node->_session, node, CFSTR("ODNodeSetCredentials"), 3, rectype, recordName, password);
	transaction_simple_response(response, code, 1, error, ^ {
		success = true;
		_ODNodeStoreCredentials(node, rectype, recordName, password);
	});

	return success;
}

bool
ODNodeVerifyCredentialsExtended(ODNodeRef node, ODRecordType recordType, ODAuthenticationType authType, CFArrayRef authItems, CFArrayRef *authItemsOut, ODContextRef *context, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("node"), node)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("record type"), recordType)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("authentication type"), authType)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("authentication items"), authItems)) {
		return false;
	}

	// TODO: when we expose the SPI as API
	//CF_OBJC_FUNCDISPATCH(__kODNodeTypeID, bool, node, "verifyCredentialsWithRecordType:authenticationType:authenticationItems:continueItems:context:error:", recordType, authType, authItems, authItemsOut, context, error);

	__block bool success = false;
	__block CFArrayRef local_items = NULL;
	CFArrayRef response;
	uint32_t code;
	uint32_t mapped_type;

	if ((mapped_type = extauth_map_type(authType)) != 0) {
		success = extauth_node_verify(node, recordType, mapped_type, authItems, &local_items, context, error);
	} else {
		response = transaction_simple(&code, node->_session, node, CFSTR("ODNodeVerifyCredentialsExtended"), 4, recordType, authType, authItems, context ? _ODContextGetUUID(*context) : NULL);
		transaction_simple_response(response, code, 3, error, ^ {
			success = true;
			local_items = safe_cfretain(schema_get_value_at_index(response, 1));
			if (context) {
				*context = _ODContextCreateWithNodeAndUUID(NULL, node, schema_get_value_at_index(response, 2));
			}
		});
	}

	if (authItemsOut) {
		// Empty array is the default response, even in cases of failure.
		*authItemsOut = local_items ? local_items : CFArrayCreate(kCFAllocatorDefault, NULL, 0, &kCFTypeArrayCallBacks);
	} else if (local_items) {
		CFRelease(local_items);
	}

	return success;
}

bool
ODNodeSetCredentialsExtended(ODNodeRef node, ODRecordType recordType, ODAuthenticationType authType, CFArrayRef authItems, CFArrayRef *authItemsOut, ODContextRef *context, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("node"), node)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("record type"), recordType)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("authentication type"), authType)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("authentication items"), authItems)) {
		return false;
	}
	CF_OBJC_FUNCDISPATCH(__kODNodeTypeID, bool, node, "setCredentialsWithRecordType:authenticationType:authenticationItems:continueItems:context:error:", recordType, authType, authItems, authItemsOut, context, error);

	__block bool success = false;
	CFArrayRef response;
	uint32_t code;

	// need to open a new backing node
	if (!_ODNodeExternalCreate(node, error)) {
		return false;
	}

	response = transaction_simple(&code, node->_session, node, CFSTR("ODNodeSetCredentialsExtended"), 4, recordType, authType, authItems, context ? _ODContextGetUUID(*context) : NULL);
	transaction_simple_response(response, code, 3, error, ^ {
		success = true;
		_ODNodeStoreCredentials(node, recordType, authItems, NULL);
		if (authItemsOut) {
			*authItemsOut = safe_cfretain(schema_get_value_at_index(response, 1));
		}
		if (context) {
			*context = _ODContextCreateWithNodeAndUUID(NULL, node, schema_get_value_at_index(response, 2));
		}
	});

	return success;
}

bool
ODNodeSetCredentialsUsingKerberosCache(ODNodeRef node, CFStringRef cache, CFErrorRef *error)
{
	if (error) {
		*error = CFErrorCreate(kCFAllocatorDefault, kODErrorDomainFramework, kODErrorCredentialsMethodNotSupported, NULL);
	}
	return false;
}

static void
_ValidateRecordAttributes(const void *key, const void *value, void *context)
{
	int *attrbad = (int *)context;
	if (CFGetTypeID(value) != CFArrayGetTypeID()) {
		*attrbad = 1;
	}
}

// cribbed from ActiveDirectoryClientModule.c
static CFStringRef
_generatePassword(void)
{
	int passLen = 15;
	char password[passLen + 1];
	int punct;
	int value;

	for (;;) {
		punct = arc4random() % 0x7f;
		if (ispunct(punct)) {
			break;
		}
	}

	for (int ii = 0; ii < passLen; ii++) {
		value = arc4random();

		// add a random punctuation character
		if (punct != 0 && (value & 0x100) != 0) {
			password[ii] = punct;
			punct = 0;
		} else {
			value &= 0x7f;
			while (!isalnum(value) || !isprint(value)) {
				value = arc4random() & 0x7f;
			}

			password[ii] = value;
		}
	}

	password[passLen] = '\0';

	return CFStringCreateWithCString(NULL, password, kCFStringEncodingUTF8);
}

static bool
_SetRecordPassword(ODRecordRef record, CFArrayRef password, CFErrorRef *error)
{
	CFStringRef newpass;
	bool success = true; // default to true, only one failure case

	if (password && CFArrayGetCount(password) >= 1) {
		if (ODRecordSetValue(record, kODAttributeTypePassword, password, NULL) == false) {
			newpass = CFArrayGetValueAtIndex(password, 0);
			if (CFEqual(newpass, CFSTR("********")) == false) {
				if (ODRecordChangePassword(record, NULL, newpass, error) == false) {
					success = false;
				}
			}
		}
	} else {
		ODRecordType rectype = ODRecordGetRecordType(record);
		if (CFEqual(rectype, kODRecordTypeUsers) == true || CFEqual(rectype, kODRecordTypeComputers) == true) {
			newpass = _generatePassword();
			if (newpass != NULL) {
				(void)ODRecordChangePassword(record, NULL, newpass, NULL);
				CFRelease(newpass);
			}
		}
	}

	return success;
}

static void
_add_to_set(CFTypeRef key, CFTypeRef value, void *context)
{
	CFSetAddValue(context, key);
}

ODRecordRef
ODNodeCreateRecord(ODNodeRef node, ODRecordType recordType, CFStringRef recordName, CFDictionaryRef in_attributes, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("node"), node)) {
		return NULL;
	}
	if (!_validate_nonnull(error, CFSTR("record type"), recordType)) {
		return NULL;
	}
	if (!_validate_nonnull(error, CFSTR("record name"), recordName)) {
		return NULL;
	}
	if (CF_IS_OBJC(__kODNodeTypeID, node)) {
		ODRecordRef record;
		CF_OBJC_CALL(ODRecordRef, record, node, "createRecordWithRecordType:name:attributes:error:", recordType, recordName, in_attributes, error);
		return record ? (ODRecordRef)CFRetain(record) : NULL;
	}

	__block ODRecordRef newrec = NULL;
	CFArrayRef response;
	uint32_t code;
	__block bool need_guid = false;
	CFMutableDictionaryRef attributes = NULL;
	CFArrayRef password = NULL;

	if (in_attributes != NULL) {
		int attrbad = 0;
		CFDictionaryApplyFunction(in_attributes, _ValidateRecordAttributes, &attrbad);
		if (attrbad) {
			_ODErrorSet(error, kODErrorRecordAttributeUnknownType, CFSTR("Attribute dictionary values must all be arrays."));
			return NULL;
		}

		attributes = CFDictionaryCreateMutableCopy(NULL, 0, in_attributes);

		// remove stuff we don't set in the attribute list
		CFDictionaryRemoveValue(attributes, kODAttributeTypeRecordType);
		CFDictionaryRemoveValue(attributes, kODAttributeTypeMetaNodeLocation);

		// set password later
		password = CFDictionaryGetValue(attributes, kODAttributeTypePassword);
		if (password) {
			CFRetain(password);
			CFDictionaryRemoveValue(attributes, kODAttributeTypePassword);
		}
	}

	response = transaction_simple(&code, node->_session, node, CFSTR("ODNodeCreateRecord"), 3, recordType, recordName, attributes);
	transaction_simple_response(response, code, 0, error, ^ {
		// can get back NULL,NULL
		CFDictionaryRef gotattrs = schema_get_value_at_index(response, 1);
		if (gotattrs) {
			if (CFDictionaryContainsKey(gotattrs, kODAttributeTypeGUID) == false) {
				need_guid = true;
			}

			CFMutableSetRef attrset = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);

			// backend might actually have more attributes, so don't assume it is all
			if (attributes != NULL) {
				CFDictionaryApplyFunction(attributes, _add_to_set, attrset);
			} else {
				CFSetAddValue(attrset, kODAttributeTypeRecordName);
				CFSetAddValue(attrset, kODAttributeTypeRecordType);
			}
			
			newrec = _RecordCreate(node, attrset, gotattrs);
			CFRelease(attrset);
		}
	});

	/* 8052103: Attempt to set a GUID if none is set. */
	if (need_guid) {
		uuid_t uuid;
		uuid_string_t uuidstr;
		CFStringRef guid;

		uuid_generate(uuid);
		uuid_unparse_upper(uuid, uuidstr);
		guid = CFStringCreateWithCString(kCFAllocatorDefault, uuidstr, kCFStringEncodingUTF8);
		if (guid != NULL) {
			(void)ODRecordSetValue(newrec, kODAttributeTypeGUID, guid, NULL);
			CFRelease(guid);
		}
	}

	if (newrec != NULL) {
		if (_SetRecordPassword(newrec, password, error) == false) {
			(void)ODRecordDelete(newrec, NULL);
			CFRelease(newrec);
			newrec = NULL;
		}
	}

	safe_cfrelease(password);
	safe_cfrelease(attributes);

	return newrec;
}

CFArrayRef
CFArrayCreateWithSet(CFSetRef set)
{
	if (!set) {
		return NULL;
	}
	CFIndex count = CFSetGetCount(set);
	const void *values[count];
	CFSetGetValues(set, values);
	return CFArrayCreate(NULL, values, count, &kCFTypeArrayCallBacks);
}

ODRecordRef
ODNodeCopyRecord(ODNodeRef node, ODRecordType recordType, CFStringRef recordName, CFTypeRef attributes, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("node"), node)) {
		return NULL;
	}
	if (!_validate_nonnull(error, CFSTR("record type"), recordType)) {
		return NULL;
	}
	if (!_validate_nonnull(error, CFSTR("record name"), recordName)) {
		return NULL;
	}
	if (CF_IS_OBJC(__kODNodeTypeID, node)) {
		ODRecordRef record;
		CF_OBJC_CALL(ODRecordRef, record, node, "recordWithRecordType:name:attributes:error:", recordType, recordName, attributes, error);
		return record ? (ODRecordRef)CFRetain(record) : NULL;
	}

	ODQueryRef query;
	CFErrorRef local_error = NULL;
	ODRecordRef newrec = NULL;
	CFArrayRef results;

	query = ODQueryCreateWithNode(NULL, node, recordType, kODAttributeTypeRecordName, kODMatchEqualTo, recordName, attributes, 1, &local_error);
	if (query) {
		/* Only one result, so synchronous is fine. */
		results = ODQueryCopyResults(query, false, &local_error);
		if (results) {
			if (CFArrayGetCount(results) == 1) {
				newrec = (ODRecordRef)CFRetain(CFArrayGetValueAtIndex(results, 0));
			}
			CFRelease(results);
		}
		CFRelease(query);
	}

	if (newrec == NULL && error != NULL) {
		*error = local_error;
	} else if (local_error) {
		CFRelease(local_error);
	}

	return newrec;
}

CFDataRef
ODNodeCustomCall(ODNodeRef node, CFIndex customCode, CFDataRef sendData, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("node"), node)) {
		return NULL;
	}
	if (CF_IS_OBJC(__kODNodeTypeID, node)) {
		CFDataRef data;
		CF_OBJC_CALL(CFDataRef, data, node, "customCall:sendData:error:", customCode, sendData, error);
		return data ? CFRetain(data) : NULL;
	}

	__block CFDataRef data = NULL;
	CFNumberRef codeNum;
	CFArrayRef response;
	uint32_t code;

	codeNum = CFNumberCreate(NULL, kCFNumberCFIndexType, &customCode);
	response = transaction_simple(&code, node->_session, node, CFSTR("ODNodeCustomCall"), 2, codeNum, sendData);
	CFRelease(codeNum);

	transaction_simple_response(response, code, 1, error, ^ {
		data = schema_get_value_at_index(response, 0);
		data = data ? CFRetain(data) : CFDataCreate(NULL, NULL, 0);
	});

	return data;
}

// TODO: could simplify this by looking up all functions at once, but doesn't save much since bundle is cached

ODNodeRef
ODNodeCreateWithDSRef(CFAllocatorRef inAllocator, tDirReference inDirRef, tDirNodeReference inNodeRef, bool inCloseOnRelease)
{
	static dispatch_once_t once;
	static ODNodeRef(*dsCopyDataFromNodeRef)(tDirNodeReference inNodeRef);

	// We have to load this dynamically because otherwise we create a circular dependency
	dispatch_once(&once,
	^(void) {
		CFBundleRef bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.DirectoryService.Framework"));
		if (bundle != NULL) {
			dsCopyDataFromNodeRef = CFBundleGetFunctionPointerForName(bundle, CFSTR("dsCopyDataFromNodeRef"));
		}
	});

	if (dsCopyDataFromNodeRef != NULL) {
		return dsCopyDataFromNodeRef(inNodeRef);
	} else {
		OD_CRASH("DirectoryService.framework missing function: dsCopyDataFromNodeRef");
	}

	return NULL;
}

tDirNodeReference
ODNodeGetDSRef(ODNodeRef inNodeRef)
{
	static dispatch_once_t once;
	static tDirNodeReference(*dsCreateNodeRefData)(CFTypeRef data);

	// We have to load this dynamically because otherwise we create a circular dependency
	dispatch_once(&once,
	^(void) {
		CFBundleRef bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.DirectoryService.Framework"));
		if (bundle != NULL) {
			dsCreateNodeRefData = CFBundleGetFunctionPointerForName(bundle, CFSTR("dsCreateNodeRefData"));
		}
	});

	if (dsCreateNodeRefData != NULL) {
		// TODO: need to track the reference we create and close it accordingly (just call dsCloseDirNode
		return dsCreateNodeRefData(inNodeRef);
	} else {
		OD_CRASH("DirectoryService.framework missing function: dsCreateNodeRefData");
	}

	return 0;
}

ODRecordRef
ODNodeCopyRecordAuthenticationData(ODNodeRef node, ODRecordRef record, CFErrorRef *error)
{
	CFArrayRef aas;
	CFIndex ii;
	CFStringRef guid = NULL;
	ODRecordRef result = NULL;

	CLEAR_ERROR(error);

	aas = ODRecordCopyValues(record, kODAttributeTypeAuthenticationAuthority, error);
	if (aas == NULL) {
		return NULL;
	}

	for (ii = 0; ii < CFArrayGetCount(aas); ii++) {
		CFStringRef aa = CFArrayGetValueAtIndex(aas, ii);
		CFStringRef sub;
		char slotid[33];

		if (!CFStringHasPrefix(aa, CFSTR(";ApplePasswordServer;0x")) || CFStringGetLength(aa) < 55) {
			continue;
		}

		sub = CFStringCreateWithSubstring(NULL, aa, CFRangeMake(23, 32));
		CFStringGetCString(sub, slotid, sizeof(slotid), kCFStringEncodingUTF8);
		CFRelease(sub);

		guid = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%.8s-%.4s-%.4s-%.4s-%.12s"), slotid, slotid + 8, slotid + 12, slotid + 16, slotid + 20);

		break;
	}

	CFRelease(aas);

	if (guid != NULL) {
		result = ODNodeCopyRecord(node, CFSTR("dsRecTypeStandard:UserAuthenticationData"), guid, NULL, error);
		CFRelease(guid);
	}

	return result;
}

bool
ODNodeCopyCredentials(ODNodeRef node, ODRecordType *recordType, CFStringRef *username, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("node"), node)) {
		return false;
	}

	if (node->_creds_type != NULL && node->_creds_name != NULL) {
		if (recordType) {
			*recordType = CFRetain(node->_creds_type);
		}
		if (username) {
			*username = CFRetain(node->_creds_name);
		}
		return true;
	} else {
		if (error != NULL) {
			*error = CFErrorCreate(kCFAllocatorDefault, kODErrorDomainFramework, kODErrorCredentialsAccountNotFound, NULL);
		}
		return false;
	}

}
