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
#include <assumes.h>

#include <opendirectory/odutils.h>
#include <membership.h>
#include <membershipPriv.h>

#include "CFODRecord.h"
#include "CFODNode.h"
#include "CFOpenDirectoryPriv.h"
#include "internal.h"
#include "format.h"
#include "transaction.h"
#include "extauth.h"
#include "record_internal.h"
#include "context_internal.h"

static CFTypeID __kODRecordTypeID = _kCFRuntimeNotATypeID;

static void
__ODRecordFinalize(CFTypeRef cf)
{
	ODRecordRef record = (ODRecordRef)cf;
	safe_cfrelease_null(record->_session);
	safe_cfrelease_null(record->_realnode);
	safe_cfrelease_null(record->_orignode);
	safe_cfrelease_null(record->_attrset);
	safe_cfrelease_null(record->_attributes);
	safe_dispatch_release(record->_attrq);
	safe_cfrelease_null(record->_type);
	safe_cfrelease_null(record->_name);
	safe_cfrelease_null(record->_metaname);
}

static CFStringRef
__ODRecordCopyDebugDesc(CFTypeRef cf)
{
	ODRecordRef record = (ODRecordRef)cf;
	CFStringRef attr, output;
	attr = format_small(record->_attributes);
	output = CFStringCreateWithFormat(NULL, NULL, CFSTR("<ODRecord %p [attributes %@]>"), record, attr);
	CFRelease(attr);
	return output;
}

static const CFRuntimeClass __ODRecordClass = {
	0,								// version
	"ODRecord",						// className
	NULL,							// init
	NULL,							// copy
	__ODRecordFinalize,				// finalize
	NULL,							// equal
	NULL,							// hash
	NULL,							// copyFormattingDesc
	__ODRecordCopyDebugDesc,		// copyDebugDesc
	NULL,							// reclaim
#if CF_REFCOUNT_AVAILABLE
	NULL,							// refcount
#endif
};

CFTypeID
ODRecordGetTypeID(void)
{
	static dispatch_once_t once;

	dispatch_once(&once, ^ {
		__kODRecordTypeID = _CFRuntimeRegisterClass(&__ODRecordClass);
		if (__kODRecordTypeID != _kCFRuntimeNotATypeID) {
			_CFRuntimeBridgeClasses(__kODRecordTypeID, "NSODRecord");
		}
	});

	return __kODRecordTypeID;
}

ODNodeRef
_ODRecordGetNode(ODRecordRef record)
{
	if (record->_realnode == NULL) {
		CFArrayRef metanode = CFDictionaryGetValue(record->_attributes, kODAttributeTypeMetaNodeLocation);

		if (metanode && CFArrayGetCount(metanode) > 0) {
			record->_realnode = ODNodeCreateWithName(NULL, record->_session, CFArrayGetValueAtIndex(metanode, 0), NULL);
		}

		// Attempt to copy credentials from original node, if any.
		if (record->_orignode != NULL) {
			(void)_ODNodeSetCredentialsFromOtherNode(record->_realnode, record->_orignode, NULL);

			safe_cfrelease_null(record->_orignode);
		}
	}

	(void)osx_assumes(record->_realnode != NULL);

	return record->_realnode;
}

static void
__update_name(ODRecordRef record)
{
	CFArrayRef tmpval;

	tmpval = CFDictionaryGetValue(record->_attributes, kODAttributeTypeRecordName);
	if (tmpval && CFArrayGetCount(tmpval) != 0) {
		CFStringRef new_name = CFArrayGetValueAtIndex(tmpval, 0);

		/* Don't update if it hasn't changed. */
		if (record->_name == NULL || CFStringCompare(record->_name, new_name, 0) != kCFCompareEqualTo) {
			safe_cfrelease_null(record->_name);
			record->_name = CFStringCreateCopy(NULL, new_name);
		}
	}
	
	tmpval = CFDictionaryGetValue(record->_attributes, kODAttributeTypeMetaRecordName);
	if (tmpval && CFArrayGetCount(tmpval) != 0) {
		safe_cfrelease_null(record->_metaname);
		record->_metaname = CFStringCreateCopy(NULL, CFArrayGetValueAtIndex(tmpval, 0));
	}
}

// more_attrs is non-NULL for ODRecordCopyValues, ODRecordCopyDetails
// force_update is true for ODRecordSynchronize
// this is also used by _RecordCreate to update the _type/_name pointers
static void
_RecordUpdate(ODRecordRef record, CFArrayRef more_attrs, bool force_update, void (^callback)(CFDictionaryRef, CFErrorRef))
{
	dispatch_sync(record->_attrq, ^ {
		CFArrayRef attrs = NULL;
		CFArrayRef tmpval;

		if (force_update) {
			// ODRecordSynchronize, just refetch the same attribuets
			attrs = CFArrayCreateWithSet(record->_attrset);
		} else if (more_attrs) {
			// ODRecordCopyValues or ODRecordCopyDetails, may need to update

			CFMutableSetRef newattrs;
			bool must_update = false;

			newattrs = CFSetCreateMutableCopy(NULL, 0, record->_attrset);

			CFIndex i;
			CFIndex count = CFArrayGetCount(more_attrs);
			CFStringRef attr;
			for (i = 0; i < count; i++) {
				attr = CFArrayGetValueAtIndex(more_attrs, i);
				if (!attrset_contains_attribute(record->_attrset, attr)) {
					must_update = true;
					CFSetAddValue(newattrs, attr);
				}
			}

			if (must_update) {
				CFRelease(record->_attrset);
				record->_attrset = attrset_create_minimized_copy(newattrs);
				attrs = CFArrayCreateWithSet(record->_attrset);
			}

			CFRelease(newattrs);
		}

		/* Refetch record if required. */
		if (attrs != NULL) {
			ODRecordRef tmprecord;

			tmprecord = ODNodeCopyRecord(_ODRecordGetNode(record), record->_type, record->_name, attrs, NULL);
			if (tmprecord != NULL) {
				CFRelease(record->_attributes);
				record->_attributes = (CFMutableDictionaryRef)CFRetain(tmprecord->_attributes);
				CFRelease(tmprecord);
			}

			CFRelease(attrs);
		}

		tmpval = CFDictionaryGetValue(record->_attributes, kODAttributeTypeRecordType);
		if (tmpval && CFArrayGetCount(tmpval) != 0) {
			// never updated after the first time
			if (record->_type == NULL) {
				record->_type = CFStringCreateCopy(NULL, CFArrayGetValueAtIndex(tmpval, 0));
			}
		}

		__update_name(record);

		if (callback) {
			// TODO: need to pass error
			callback(record->_attributes, NULL);
		}
	});
}

ODRecordRef
_RecordCreate(ODNodeRef node, CFSetRef fetched, CFDictionaryRef attributes)
{
	ODRecordRef result;
	char qname[256];

	result = (ODRecordRef)_CFRuntimeCreateInstance(NULL, ODRecordGetTypeID(), sizeof(struct __ODRecord) - sizeof(CFRuntimeBase), NULL);

	result->_session = (ODSessionRef)safe_cfretain(_NodeGetSession(node));

	// Set _realnode if the result is from the same node (i.e. not /Search or .../All Domains).
	// Otherwise, _ODRecordGetNode() will open the correct node if and when it is needed.
	if (attributes != NULL) {
		CFArrayRef metanode = CFDictionaryGetValue(attributes, kODAttributeTypeMetaNodeLocation);

		if (metanode != NULL && CFArrayGetCount(metanode) > 0) {
			if (CFEqual(CFArrayGetValueAtIndex(metanode, 0), ODNodeGetName(node)) == true) {
				result->_realnode = (ODNodeRef)CFRetain(node);
			}
		}
	}

	if (result->_realnode == NULL) {
		result->_orignode = (ODNodeRef)CFRetain(node);
	}

	snprintf(qname, sizeof(qname), "com.apple.OpenDirectory.ODRecord.%p", result);
	result->_attrq = dispatch_queue_create(qname, NULL);
	result->_attrset = fetched ? CFRetain(fetched) : CFSetCreate(NULL, NULL, 0, &kCFTypeSetCallBacks);
	result->_attributes = (attributes != NULL ? CFDictionaryCreateMutableCopy(NULL, 0, attributes) : CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));

	_RecordUpdate(result, NULL, false, NULL);

	return result;
}

bool
ODRecordSetNodeCredentials(ODRecordRef record, CFStringRef username, CFStringRef password, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("record"), record)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("username"), username)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("password"), password)) {
		return false;
	}
	CF_OBJC_FUNCDISPATCH(__kODRecordTypeID, bool, record, "setNodeCredentials:password:error:", username, password, error);

	bool success = false;
	ODNodeRef node;

	// TODO: should there be a global list of these nodes so they can share one connection, do we care?
	// (see comments about node caching @ ODNodeCreateWithName)
	node = ODNodeCreateCopy(CFGetAllocator(record), _ODRecordGetNode(record), error);
	if (node != NULL) {
		success = ODNodeSetCredentials(node, kODRecordTypeUsers, username, password, error);
		if (success) {
			// TODO: sync
			CFRelease(record->_realnode);
			record->_realnode = node;
		} else {
			CFRelease(node);
		}
	}

	return success;
}

bool
ODRecordSetNodeCredentialsExtended(ODRecordRef record, ODRecordType recordType, ODAuthenticationType authType, CFArrayRef authItems, CFArrayRef *outAuthItems, ODContextRef *outContext, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("record"), record)) {
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

	bool success = false;
	ODNodeRef node;

	node = ODNodeCreateCopy(CFGetAllocator(record), _ODRecordGetNode(record), error);
	if (node) {
		success = ODNodeSetCredentialsExtended(node, recordType, authType, authItems, outAuthItems, outContext, error);
		if (success) {
			// TODO: sync
			CFRelease(record->_realnode);
			record->_realnode = node;
		} else {
			CFRelease(node);
		}
	}

	return success;
}

bool
ODRecordSetNodeCredentialsUsingKerberosCache(ODRecordRef record, CFStringRef cacheName, CFErrorRef *error)
{
	if (error) {
		*error = CFErrorCreate(kCFAllocatorDefault, kODErrorDomainFramework, kODErrorCredentialsMethodNotSupported, NULL);
	}
	return false;
}

CFDictionaryRef
ODRecordCopyPasswordPolicy(CFAllocatorRef allocator, ODRecordRef record, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("record"), record)) {
		return NULL;
	}
	if (CF_IS_OBJC(__kODRecordTypeID, record)) {
		CFDictionaryRef policy;
		CF_OBJC_CALL(CFDictionaryRef, policy, record, "passwordPolicyAndReturnError:", error);
		return policy ? CFRetain(policy) : NULL;
	}

	__block CFDictionaryRef policy = NULL;
	CFArrayRef response;
	uint32_t code;

	response = transaction_simple(&code, record->_session, _ODRecordGetNode(record), CFSTR("ODRecordCopyPasswordPolicy"), 3, record->_type, record->_name, record->_metaname);
	transaction_simple_response(response, code, 1, error, ^ {
		policy = schema_get_value_at_index(response, 0);
		// can be NULL if no policy is set
		if (policy) CFRetain(policy);
	});

	return policy;
}

bool
ODRecordVerifyPassword(ODRecordRef record, CFStringRef password, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("record"), record)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("password"), password)) {
		return false;
	}
	CF_OBJC_FUNCDISPATCH(__kODRecordTypeID, bool, record, "verifyPassword:error:", password, error);

	__block bool success = false;
	CFArrayRef response;
	uint32_t code;

	response = transaction_simple(&code, record->_session, _ODRecordGetNode(record), CFSTR("ODRecordVerifyPassword"), 4, record->_type, record->_name, record->_metaname, password);
	transaction_simple_response(response, code, 1, error, ^ {
		success = true;
	});

	return success;
}

bool
ODRecordVerifyPasswordExtended(ODRecordRef record, ODAuthenticationType authType, CFArrayRef authItems, CFArrayRef *authItemsOut, ODContextRef *context, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("record"), record)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("authentication type"), authType)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("authentication items"), authItems)) {
		return false;
	}
	CF_OBJC_FUNCDISPATCH(__kODRecordTypeID, bool, record,
	                     "verifyExtendedWithAuthenticationType:authenticationItems:continueItems:context:error:",
	                     authType, authItems, authItemsOut, context, error);

	__block bool success = false;
	__block CFArrayRef local_items = NULL;
	CFArrayRef response;
	uint32_t code;
	uint32_t mapped_type;

	if ((mapped_type = extauth_map_type(authType)) != 0) {
		success = extauth_record_verify(record, mapped_type, authItems, &local_items, context, error);
	} else {
		response = transaction_simple(&code, record->_session, _ODRecordGetNode(record), CFSTR("ODRecordVerifyPasswordExtended"), 6, record->_type, record->_name, record->_metaname, authType, authItems, context ? _ODContextGetUUID(*context) : NULL);
		transaction_simple_response(response, code, 3, error, ^ {
			success = true;
			local_items = safe_cfretain(schema_get_value_at_index(response, 1));
			if (context) {
				*context = _ODContextCreateWithNodeAndUUID(NULL, _ODRecordGetNode(record), schema_get_value_at_index(response, 2));
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
ODRecordChangePassword(ODRecordRef record, CFStringRef oldPassword, CFStringRef newPassword, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("record"), record)) {
		return false;
	}
	// old password can be null
	if (!_validate_nonnull(error, CFSTR("new password"), newPassword)) {
		return false;
	}
	CF_OBJC_FUNCDISPATCH(__kODRecordTypeID, bool, record, "changePassword:toPassword:error:", oldPassword, newPassword, error);

	__block bool success = false;
	CFArrayRef response;
	uint32_t code;

	response = transaction_simple(&code, record->_session, _ODRecordGetNode(record), CFSTR("ODRecordChangePassword"), 5, record->_type, record->_name, record->_metaname, oldPassword, newPassword);
	transaction_simple_response(response, code, 1, error, ^ {
		success = true;
	});

	return success;
}

CFStringRef
ODRecordGetRecordType(ODRecordRef record)
{
	if (!record) {
		return NULL;
	}
	return record->_type;
}

CFStringRef
ODRecordGetRecordName(ODRecordRef record)
{
	if (!record) {
		return NULL;
	}
	return record->_name;
}

CFArrayRef
ODRecordCopyValues(ODRecordRef record, ODAttributeType attribute, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("record"), record)) {
		return NULL;
	}
	if (!_validate_nonnull(error, CFSTR("attribute"), attribute)) {
		return NULL;
	}
	if (CF_IS_OBJC(__kODRecordTypeID, record)) {
		CFArrayRef values;
		CF_OBJC_CALL(CFArrayRef, values, record, "valuesForAttribute:error:", attribute, error);
		return values ? CFRetain(values) : NULL;
	}

	const void *attrvals[1];
	CFArrayRef attrs;
	__block CFArrayRef values = NULL;

	attrvals[0] = attribute;
	attrs = CFArrayCreate(NULL, attrvals, 1, &kCFTypeArrayCallBacks);
	_RecordUpdate(record, attrs, false, ^(CFDictionaryRef a, CFErrorRef e) {
		// TODO: set error?
		values = CFDictionaryGetValue(a, attribute);
		if (values) {
			values = CFArrayCreateCopy(kCFAllocatorDefault, values);
		}
	});
	CFRelease(attrs);

	return values;
}

/*
 * _attribute_is_immutable
 * Returns true if given attribute is immutable. Currently only applies to MetaNodeLocation.
 */
static bool
_attribute_is_immutable(ODAttributeType attribute)
{
	bool immutable = false;

	if (CFStringCompare(attribute, kODAttributeTypeMetaNodeLocation, 0) == kCFCompareEqualTo) {
		immutable = true;
	}

	return immutable;
}

// TODO: may need to sync the entire transaction on _attrq to maintain consistency

bool
ODRecordSetValue(ODRecordRef record, ODAttributeType attribute, CFTypeRef value, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("record"), record)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("attribute"), attribute)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("value"), value)) {
		return false;
	}
	CF_OBJC_FUNCDISPATCH(__kODRecordTypeID, bool, record, "setValue:forAttribute:error:", value, attribute, error);

	__block bool success = false;
	CFArrayRef value_array;
	CFArrayRef response;
	uint32_t code;
	bool is_delete;

	if (_attribute_is_immutable(attribute)) {
		success = true;
	} else {
		if (CFGetTypeID(value) == CFArrayGetTypeID()) {
			value_array = CFPropertyListCreateDeepCopy(kCFAllocatorDefault, value, kCFPropertyListMutableContainers);
		} else {
			CFTypeRef valcopy = CFPropertyListCreateDeepCopy(kCFAllocatorDefault, value, kCFPropertyListMutableContainers);
			value_array = CFArrayCreate(NULL, (const void **)&valcopy, 1, &kCFTypeArrayCallBacks);
			CFRelease(valcopy);
		}

		is_delete = (CFArrayGetCount(value_array) == 0);

		response = transaction_simple(&code, record->_session, _ODRecordGetNode(record), CFSTR("ODRecordSetValue"), 5, record->_type, record->_name, record->_metaname, attribute, value_array);

		// 8904328: Deleting an unknown attribute is "success"
		if (is_delete && code == kODErrorRecordAttributeUnknownType) {
			code = 0;
		}

		transaction_simple_response(response, code, 1, error, ^ {
			__block bool issue_update = false;
			
			success = true;
			dispatch_sync(record->_attrq, ^ {
				CFStringRef attrcopy;

				if (is_delete) {
					CFDictionaryRemoveValue(record->_attributes, attribute);
				} else {
					attrcopy = CFStringCreateCopy(kCFAllocatorDefault, attribute);
					CFDictionarySetValue(record->_attributes, attrcopy, value_array);
					CFRelease(attrcopy);

					if (CFStringCompare(attribute, kODAttributeTypeRecordName, 0) == kCFCompareEqualTo) {
						__update_name(record);
						issue_update = true;
					}
				}
			});
			
			if (issue_update == true) {
				_RecordUpdate(record, NULL, true, NULL); // grabs record->_attrq
			}
		});

		CFRelease(value_array);
	}

	return success;
}

bool
ODRecordAddValue(ODRecordRef record, ODAttributeType attribute, CFTypeRef value, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("record"), record)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("attribute"), attribute)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("value"), value)) {
		return false;
	}
	CF_OBJC_FUNCDISPATCH(__kODRecordTypeID, bool, record, "addValue:toAttribute:error:", value, attribute, error);

	__block bool success = false;
	CFArrayRef response;
	uint32_t code;

	if (_attribute_is_immutable(attribute)) {
		success = true;
	} else {
		response = transaction_simple(&code, record->_session, _ODRecordGetNode(record), CFSTR("ODRecordAddValue"), 5, record->_type, record->_name, record->_metaname, attribute, value);
		transaction_simple_response(response, code, 1, error, ^ {
			success = true;
			dispatch_sync(record->_attrq, ^ {
				CFMutableArrayRef new;
				CFArrayRef values = CFDictionaryGetValue(record->_attributes, attribute);
				CFStringRef attrcopy, valcopy;

				// Copy, append, set
				if (values != NULL) {
					new = CFArrayCreateMutableCopy(NULL, 0, values);
				} else {
					new = CFArrayCreateMutable(NULL, 1, &kCFTypeArrayCallBacks);
				}

				valcopy = CFPropertyListCreateDeepCopy(kCFAllocatorDefault, value, kCFPropertyListMutableContainers);
				CFArrayAppendValue(new, valcopy);
				CFRelease(valcopy);

				attrcopy = CFStringCreateCopy(kCFAllocatorDefault, attribute);
				CFDictionarySetValue(record->_attributes, attrcopy, new);
				CFRelease(attrcopy);
				CFRelease(new);

				if (CFStringCompare(attribute, kODAttributeTypeRecordName, 0) == kCFCompareEqualTo) {
					__update_name(record);
				}
			});
		});
	}

	return success;
}

bool
ODRecordRemoveValue(ODRecordRef record, ODAttributeType attribute, CFTypeRef value, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("record"), record)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("attribute"), attribute)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("value"), value)) {
		return false;
	}
	CF_OBJC_FUNCDISPATCH(__kODRecordTypeID, bool, record, "removeValue:fromAttribute:error:", value, attribute, error);

	__block bool success = false;
	CFArrayRef response;
	uint32_t code;

	if (_attribute_is_immutable(attribute)) {
		success = true;
	} else {
		response = transaction_simple(&code, record->_session, _ODRecordGetNode(record), CFSTR("ODRecordRemoveValue"), 5, record->_type, record->_name, record->_metaname, attribute, value);
		transaction_simple_response(response, code, 1, error, ^ {
			success = true;
			dispatch_sync(record->_attrq, ^ {
				CFMutableArrayRef new;
				CFIndex idx;
				CFArrayRef values = CFDictionaryGetValue(record->_attributes, attribute);
				CFStringRef attrcopy;

				if (values != NULL) {
					new = CFArrayCreateMutableCopy(NULL, 0, values);
				} else {
					new = CFArrayCreateMutable(NULL, 1, &kCFTypeArrayCallBacks);
				}

				idx = CFArrayGetFirstIndexOfValue(new, CFRangeMake(0, CFArrayGetCount(new)), value);
				if (idx != kCFNotFound) {
					CFArrayRemoveValueAtIndex(new, idx);
					attrcopy = CFStringCreateCopy(kCFAllocatorDefault, attribute);
					CFDictionarySetValue(record->_attributes, attrcopy, new);
					CFRelease(attrcopy);
				}

				CFRelease(new);

				if (CFStringCompare(attribute, kODAttributeTypeRecordName, 0) == kCFCompareEqualTo) {
					__update_name(record);
				}
			});
		});
	}

	return success;
}

CFDictionaryRef
ODRecordCopyDetails(ODRecordRef record, CFArrayRef attributes, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("record"), record)) {
		return NULL;
	}
	if (CF_IS_OBJC(__kODRecordTypeID, record)) {
		CFDictionaryRef details;
		CF_OBJC_CALL(CFDictionaryRef, details, record, "recordDetailsForAttributes:error:", attributes, error);
		return details ? CFRetain(details) : NULL;
	}

	__block CFMutableDictionaryRef details = NULL;

	if (attributes) {
		_RecordUpdate(record, attributes, false, ^(CFDictionaryRef a, CFErrorRef e) {
			CFIndex count = CFArrayGetCount(attributes);
			CFRange range = CFRangeMake(0, count);

			if (CFArrayContainsValue(attributes, range, kODAttributeTypeAllAttributes)) {
				details = (CFMutableDictionaryRef) CFDictionaryCreateCopy(NULL, a);
			} else {
				bool	std		= false;
				bool	native	= false;

				details = CFDictionaryCreateMutable(NULL, count, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

				for (CFIndex i = 0; i < count; i++) {
					CFStringRef attr;
					CFArrayRef values;

					attr = CFArrayGetValueAtIndex(attributes, i);
					values = CFDictionaryGetValue(a, attr);

					if (values) {
						// we need to create copy otherwise clients might iterate a value that we change internally
						CFTypeRef temp = CFArrayCreateCopy(NULL, values);
						CFDictionarySetValue(details, attr, temp);
						CFRelease(temp);
					} else if (std == false) {
						std = CFEqual(attr, kODAttributeTypeStandardOnly);
					} else if (native == false) {
						native = CFEqual(attr, kODAttributeTypeNativeOnly);
					}
				}

				// standard or native we have to iterate each attribute
				if (std || native) {
					CFIndex rec_count = CFDictionaryGetCount(a);
					CFTypeRef *keys = (CFTypeRef *) alloca(sizeof(CFTypeRef) * rec_count);
					CFTypeRef *values = (CFTypeRef *) alloca(sizeof(CFTypeRef) * rec_count);

					CFDictionaryGetKeysAndValues(a, keys, values);
					for (CFIndex i = 0; i < rec_count; i++) {
						CFStringRef key = keys[i];

						if ((std && CFStringHasPrefix(key, CFSTR("dsAttrTypeStandard:")))
						|| (native && CFStringHasPrefix(key, CFSTR("dsAttrTypeNative:")))) {

							// we need to create copy otherwise clients might iterate a value that we change internally
							CFTypeRef temp = CFArrayCreateCopy(NULL, values[i]);
							CFDictionarySetValue(details, key, temp);
							CFRelease(temp);
						}
					}
				}
			}

			// TODO: handle error
		});

	} else {
		// attributes NULL means return all currently fetched attrs
		// we need to create copy otherwise clients might iterate a value that we change internally
		dispatch_sync(record->_attrq, ^ {
			details = (CFMutableDictionaryRef) CFDictionaryCreateCopy(NULL, record->_attributes);
		});
	}

	return details;
}

bool
ODRecordSynchronize(ODRecordRef record, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("record"), record)) {
		return false;
	}
	CF_OBJC_FUNCDISPATCH(__kODRecordTypeID, bool, record, "synchronizeAndReturnError:", error);

	_RecordUpdate(record, NULL, true, ^(CFDictionaryRef a, CFErrorRef e) {
		// TODO: handle error
	});
	return true;
}

bool
ODRecordDelete(ODRecordRef record, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("record"), record)) {
		return false;
	}
	CF_OBJC_FUNCDISPATCH(__kODRecordTypeID, bool, record, "deleteRecordAndReturnError:", error);

	__block bool success = false;
	CFArrayRef response;
	uint32_t code;

	response = transaction_simple(&code, record->_session, _ODRecordGetNode(record), CFSTR("ODRecordDelete"), 3, record->_type, record->_name, record->_metaname);
	transaction_simple_response(response, code, 1, error, ^ {
		success = true;
	});

	return success;
}

#pragma mark -
#pragma Membership APIs

enum {
	kODMemberUseUUID    = 0x00000001,
	kODMemberUseName    = 0x00000002,

	kODMemberNested     = 0x00000010,

	kODMemberUser       = 0x00000100,
	kODMemberComputer   = 0x00000200,
	kODMemberGroup      = 0x00000400,
	kODMemberOtherGroup = 0x00000800
};

static uint32_t
__ODRecordMapTypes(ODRecordRef group, ODRecordRef member)
{
	CFStringRef grouptype	= ODRecordGetRecordType(group);
	CFStringRef membertype	= ODRecordGetRecordType(member);
	uint32_t	flags		= 0;

	if (group == NULL || member == NULL) {
		return 0;
	}

	if (CFStringCompare(grouptype, kODRecordTypeGroups, 0) == kCFCompareEqualTo) {
		if (CFStringCompare(membertype, kODRecordTypeUsers, 0) == kCFCompareEqualTo) {
			flags |= kODMemberUser | kODMemberUseUUID | kODMemberUseName;
		} else if (CFStringCompare(membertype, kODRecordTypeComputers, 0) == kCFCompareEqualTo) {
			flags |= kODMemberComputer | kODMemberUseUUID | kODMemberUseName;
		} else if (CFStringCompare(membertype, kODRecordTypeGroups, 0) == kCFCompareEqualTo) {
			flags |= kODMemberGroup | kODMemberUseUUID | kODMemberNested;
		} else if (CFStringCompare(membertype, kODRecordTypeComputerGroups, 0) == kCFCompareEqualTo) {
			flags |= kODMemberOtherGroup | kODMemberUseUUID | kODMemberNested;
		}
	} else if (CFStringCompare(grouptype, kODRecordTypeComputerGroups, 0) == kCFCompareEqualTo) {
		if (CFStringCompare(membertype, kODRecordTypeComputers, 0) == kCFCompareEqualTo) {
			flags |= kODMemberComputer | kODMemberUseUUID | kODMemberUseName;
		} else if (CFStringCompare(membertype, kODRecordTypeComputerGroups, 0) == kCFCompareEqualTo) {
			flags |= kODMemberOtherGroup | kODMemberUseUUID | kODMemberNested;
		}
	} else if (CFStringCompare(grouptype, kODRecordTypeComputerLists, 0) == kCFCompareEqualTo) {
		// Note: logic in __MembershipApplyBlock assumes kODMemberUseUUID will not be set here
		if (CFStringCompare(membertype, kODRecordTypeComputers, 0) == kCFCompareEqualTo) {
			flags |= kODMemberComputer | kODMemberUseName;
		} else if (CFStringCompare(membertype, kODRecordTypeComputerLists, 0) == kCFCompareEqualTo) {
			flags |= kODMemberOtherGroup | kODMemberUseName | kODMemberNested;
		}
	}

	return flags;
}

static CFStringRef
_CopyTempUUIDForRecord(ODRecordRef record, uint32_t flags)
{
	CFStringRef returnUUID  = NULL;
	CFStringRef(^getUUID)(ODAttributeType, int ( *)(uid_t, uuid_t));

	getUUID = ^(ODAttributeType attrib, int (*mbr_cb)(uid_t, uuid_t)) {
		CFStringRef cfUUID  = NULL;
		CFArrayRef  idArray = ODRecordCopyValues(record, attrib, NULL);

		if (idArray != NULL && CFArrayGetCount(idArray) > 0) {
			CFStringRef cfID = (CFStringRef) CFArrayGetValueAtIndex(idArray, 0);
			if (cfID != NULL && CFGetTypeID(cfID) == CFStringGetTypeID()) {
				uuid_t  uuid;
				uuid_string_t uuidStr;

				if (mbr_cb(CFStringGetIntValue(cfID), uuid) == 0) {
					uuid_unparse_upper(uuid, uuidStr);
					cfUUID = CFStringCreateWithCString(kCFAllocatorDefault, uuidStr, kCFStringEncodingUTF8);
				}
			}
		}

		if (idArray) {
			CFRelease(idArray);
		}

		return cfUUID;
	};

	if ((flags & kODMemberUser) != 0) {
		returnUUID = getUUID(kODAttributeTypeUniqueID, mbr_uid_to_uuid);
	} else if ((flags & kODMemberGroup) != 0) {
		returnUUID = getUUID(kODAttributeTypePrimaryGroupID, mbr_gid_to_uuid);
	}

	return returnUUID;
}

static CFErrorRef
__MembershipApplyBlock(ODRecordRef group, ODRecordRef member, uint32_t flags, CFErrorRef(^block)(ODAttributeType attr, CFTypeRef value))
{
	CFErrorRef	error	= NULL;
	bool		valid	= false;    // is a valid flag combination

	// need UUID for these ops
	if ((flags & kODMemberUseUUID) != 0) {
		CFArrayRef  memberUUIDList    = ODRecordCopyValues(member, kODAttributeTypeGUID, NULL);
		CFStringRef memberUUID        = NULL;
		CFStringRef tempUUID          = NULL;

		if (NULL != memberUUIDList && 0 != CFArrayGetCount(memberUUIDList)) {
			memberUUID = (CFStringRef) CFArrayGetValueAtIndex(memberUUIDList, 0);
		}

		if (NULL == memberUUID) {
			memberUUID = tempUUID = _CopyTempUUIDForRecord(member, flags);
		}

		// if we have a UUID
		if (memberUUID != NULL) {
			if ((flags & (kODMemberComputer | kODMemberUser)) != 0) {
				error = block(kODAttributeTypeGroupMembers, memberUUID);
				valid = true;
			} else if ((flags & kODMemberNested) != 0) {
				error = block(kODAttributeTypeNestedGroups, memberUUID);
				valid = true;
			}

			// clear the error if attribute type is not supported
			if (error != NULL && CFErrorGetCode(error) == kODErrorRecordAttributeUnknownType) {
				CFRelease(error);
				error = NULL;
				valid = false;
			}
		} else if ((flags & kODMemberNested) != 0) {
			_ODErrorSet(&error, kODErrorRecordAttributeNotFound, NULL);
		}

		if (tempUUID != NULL) {
			CFRelease(tempUUID);
			tempUUID = NULL;
		}

		if (memberUUIDList) {
			CFRelease(memberUUIDList);
		}
	}

	if (error == NULL && (flags & kODMemberUseName) != 0) {
		CFStringRef  memberName = ODRecordGetRecordName(member);
		if (memberName != NULL) {
			if ((flags & kODMemberUser) != 0) {
				error = block(kODAttributeTypeGroupMembership, memberName);
				valid = true;
			} else if ((flags & kODMemberComputer) != 0) {
				// Special handling here. Computer short names must be added to different attributes:
				// - ComputerGroups: GroupMembership
				// - ComputerLists: Computers (legacy)
				// Legacy attributes do not use UUIDs, so this is relatively safe.
				if ((flags & kODMemberUseUUID) != 0) {
					error = block(kODAttributeTypeGroupMembership, memberName);
				} else {
					error = block(kODAttributeTypeComputers, memberName);
				}
				valid = true;
			} else if ((flags & kODMemberNested) != 0) {
				error = block(kODAttributeTypeGroup, memberName);
				valid = true;
			}

			// clear the error and use the consistent one below if it is no supported attribute
			if (error != NULL && CFErrorGetCode(error) == kODErrorRecordAttributeUnknownType) {
				CFRelease(error);
				error = NULL;
				valid = false;
			}
		} else {
			_ODErrorSet(&error, kODErrorRecordAttributeNotFound, NULL);
		}
	}

	// Create an error case if the value failed to add to any known types
	if (error == NULL && valid == false) {
		_ODErrorSet(&error, kODErrorRecordAttributeUnknownType, NULL);
	}

	return error;
}

bool
ODRecordAddMember(ODRecordRef group, ODRecordRef member, CFErrorRef *error)
{
	bool		success    = false;
	uint32_t	flags;

	CLEAR_ERROR(error);

	if (!_validate_nonnull(error, CFSTR("group"), group)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("member"), member)) {
		return false;
	}

	CF_OBJC_FUNCDISPATCH(__kODRecordTypeID, bool, group, "addMemberRecord:error:", member, error);

	flags = __ODRecordMapTypes(group, member);

	// groups can be hybrid so check hybrid status before attempting to add values to user list
	if ((flags & (kODMemberUseUUID | kODMemberUseName)) != 0 && (flags & kODMemberUser) != 0) {
		CFArrayRef  names   = ODRecordCopyValues(group, kODAttributeTypeGroupMembership, NULL);
		CFArrayRef  uuids   = ODRecordCopyValues(group, kODAttributeTypeGroupMembers, NULL);

		// if we don't have any UUIDs but we have names, don't add UUIDs
		if (uuids == 0 && names > 0) {
			// if this is a nested group attempt, it will fail because the group is legacy format still
			flags &= ~kODMemberUseUUID;
		}

		// if not hybrid, don't add the name either
		if (uuids > 0 && names == 0) {
			flags &= ~kODMemberUseName;
		}

		safe_cfrelease(names);
		safe_cfrelease(uuids);
	}

	if ((flags & (kODMemberUseUUID | kODMemberUseName)) != 0) {
		CFErrorRef cfTempError = __MembershipApplyBlock(group, member, flags, ^(ODAttributeType attr, CFTypeRef value) {
			CFErrorRef localerror  = NULL;
			CFArrayRef cfValues = (CFArrayRef) ODRecordCopyValues(group, attr, NULL);

			if (cfValues == NULL ||
			CFArrayContainsValue(cfValues, CFRangeMake(0, CFArrayGetCount(cfValues)), value) == false) {
				ODRecordAddValue(group, attr, value, &localerror);
			}

			if (cfValues != NULL) {
				CFRelease(cfValues);
			}

			return localerror;
		});

		if (cfTempError == NULL) {
			success = true;
		} else {
			if (error != NULL) {
				(*error) = cfTempError;
			} else if (cfTempError != NULL) {
				CFRelease(cfTempError);
				cfTempError = NULL;
			}
		}
	} else {
		_ODErrorSet(error, kODErrorRecordInvalidType, NULL);
	}

	return success;
}

bool
ODRecordRemoveMember(ODRecordRef group, ODRecordRef member, CFErrorRef *error)
{
	bool		success	= false;
	uint32_t	flags;

	CLEAR_ERROR(error);

	if (!_validate_nonnull(error, CFSTR("group"), group)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("member"), member)) {
		return false;
	}

	CF_OBJC_FUNCDISPATCH(__kODRecordTypeID, bool, group, "removeRecordMember:error:", member, error);

	flags = __ODRecordMapTypes(group, member);
	if ((flags & (kODMemberUseUUID | kODMemberUseName)) != 0) {
		CFErrorRef cfTempError = __MembershipApplyBlock(group, member, flags, ^(ODAttributeType attr, CFTypeRef value) {
			CFErrorRef localerr = NULL;
			bool bTemp = ODRecordRemoveValue(group, attr, value, &localerr);
			if (bTemp == false) {
				CFIndex code = CFErrorGetCode(localerr);
				if (kODErrorRecordAttributeValueNotFound == code ||  kODErrorRecordAttributeNotFound == code) {
					CFRelease(localerr);
					localerr = NULL;
				}
			}

			return localerr;
		});

		if (cfTempError == NULL) {
			success = true;
		} else {
			if (error != NULL) {
				(*error) = cfTempError;
			} else if (cfTempError != NULL) {
				CFRelease(cfTempError);
				cfTempError = NULL;
			}
		}
	} else {
		_ODErrorSet(error, kODErrorRecordInvalidType, NULL);
	}

	return success;
}

static bool
_record_contains(ODRecordRef group, ODRecordRef member, bool refresh, CFErrorRef *error)
{
	int			isMember	= 0;
	uint32_t	flags;

	// TODO: make it work over proxy using the remote daemon (probably a custom call)
	flags = __ODRecordMapTypes(group, member);
	if ((flags & kODMemberNested) == 0) {
		uuid_t	uuid_group;
		uuid_t	uuid_member;
		bool	(^getUUID)(ODRecordRef, uuid_t);

		getUUID = ^(ODRecordRef inRecord, uuid_t inUUID)  {
			CFArrayRef  recordUUIDList	= ODRecordCopyValues(inRecord, kODAttributeTypeGUID, NULL);
			CFStringRef recordUUID		= NULL;
			CFStringRef	tempUUID		= NULL;
			bool		bFound			= false;

			if (NULL != recordUUIDList && 0 != CFArrayGetCount(recordUUIDList)) {
				recordUUID = (CFStringRef) CFArrayGetValueAtIndex(recordUUIDList, 0);
			}

			if (NULL == recordUUID) {
				recordUUID = tempUUID = _CopyTempUUIDForRecord(member, flags);
			}

			// if we have a UUID
			if (recordUUID != NULL) {
				uuid_string_t memberUUID;

				if (CFStringGetCString(recordUUID, memberUUID, sizeof(memberUUID), kCFStringEncodingASCII) == true) {
					uuid_parse(memberUUID, inUUID);
					bFound = true;
				}
			}

			if (tempUUID != NULL) {
				CFRelease(tempUUID);
				tempUUID = NULL;
			}

			if (recordUUIDList != NULL) {
				CFRelease(recordUUIDList);
				recordUUIDList = NULL;
			}

			return bFound;
		};

		// should always have a UUID
		if (getUUID(group, uuid_group) == true && getUUID(member, uuid_member) == true) {
			if (refresh == false) {
				mbr_check_membership(uuid_member, uuid_group, &isMember);
			} else {
				mbr_check_membership_refresh(uuid_member, uuid_group, &isMember);
			}
		}
	} else {
		_ODErrorSet(error, kODErrorRecordInvalidType, NULL);
	}

	return (isMember == 1 ? true : false);
}

bool
ODRecordContainsMember(ODRecordRef group, ODRecordRef member, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("group record"), group)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("member record"), member)) {
		return false;
	}
	CF_OBJC_FUNCDISPATCH(__kODRecordTypeID, bool, group, "isMemberRecord:error:", member, error);
	return _record_contains(group, member, false, error);
}

// Private...
bool
ODRecordContainsMemberRefresh(ODRecordRef group, ODRecordRef member, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("group record"), group)) {
		return false;
	}
	if (!_validate_nonnull(error, CFSTR("member record"), member)) {
		return false;
	}
	CF_OBJC_FUNCDISPATCH(__kODRecordTypeID, bool, group, "isMemberRecordRefresh:error:", member, error);
	return _record_contains(group, member, true, error);
}
