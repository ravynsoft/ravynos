/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
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

//#include <CoreFoundation/CoreFoundation.h>
#include "CFOpenDirectory.h"
#include "internal.h"
#include "extauth.h"
#include "record_internal.h"

/* mapped auth types; these do not correspond to the similar enums in opendirectoryd */
enum {
	eAuthTypeChangePasswd = 1,
	eAuthTypeSetPasswordAsCurrent,
	eAuthTypeSetPassword,
	eAuthTypeSetPolicy,
	eAuthTypeGetEffectivePolicy,
	eAuthTypeSimpleVerify,
};

uint32_t
extauth_map_type(ODAuthenticationType authType)
{
	int32_t type = 0;

	if (CFEqual(authType, kODAuthenticationTypeChangePasswd)) {
		type = eAuthTypeChangePasswd;
	} else if (CFEqual(authType, kODAuthenticationTypeSetPasswordAsCurrent)) {
		type = eAuthTypeSetPasswordAsCurrent;
	} else if (CFEqual(authType, kODAuthenticationTypeSetPassword)) {
		type = eAuthTypeSetPassword;
	} else if (CFEqual(authType, kODAuthenticationTypeSetPolicy)) {
		type = eAuthTypeSetPolicy;
	} else if (CFEqual(authType, kODAuthenticationTypeGetEffectivePolicy)) {
		type = eAuthTypeGetEffectivePolicy;
	} else if (CFEqual(authType, kODAuthenticationTypeClearText) || CFEqual(authType, kODAuthenticationTypeNodeNativeClearTextOK) || CFEqual(authType, kODAuthenticationTypeNodeNativeNoClearText)) {
		type = eAuthTypeSimpleVerify;
	}

	return type;
}

bool
extauth_record_verify(ODRecordRef record, uint32_t mapped_type, CFArrayRef authItems, CFArrayRef *authItemsOut, ODContextRef *context, CFErrorRef *error)
{
	bool success = false;
	ODRecordRef tmprecord;
	CFMutableArrayRef tmpitems;

	// NB: The 'username' values are ignored, because they should correspond to the record.

	switch (mapped_type) {
	case eAuthTypeChangePasswd:
		// [ username, oldpw, newpw ]
		if (CFArrayGetCount(authItems) == 3) {
			success = ODRecordChangePassword(record, CFArrayGetValueAtIndex(authItems, 1), CFArrayGetValueAtIndex(authItems, 2), error);
		} else {
			_ODErrorSet(error, kODErrorCredentialsParameterError, NULL);
		}
		break;
	case eAuthTypeSetPasswordAsCurrent:
		// [ username, newpw ]
		if (CFArrayGetCount(authItems) == 2) {
			success = ODRecordChangePassword(record, NULL, CFArrayGetValueAtIndex(authItems, 1), error);
		} else {
			_ODErrorSet(error, kODErrorCredentialsParameterError, NULL);
		}
		break;
	case eAuthTypeSetPassword:
		// [ username, newpw, authuser, authpass ]
		if (CFArrayGetCount(authItems) == 4) {
			tmprecord = ODNodeCopyRecord(_ODRecordGetNode(record), record->_type, record->_name, NULL, error);
			if (tmprecord != NULL) {
				if (ODRecordSetNodeCredentials(tmprecord, CFArrayGetValueAtIndex(authItems, 2), CFArrayGetValueAtIndex(authItems, 3), error) == true) {
					success = ODRecordChangePassword(tmprecord, NULL, CFArrayGetValueAtIndex(authItems, 1), error);
				}
				CFRelease(tmprecord);
			}
		} else {
			_ODErrorSet(error, kODErrorCredentialsParameterError, NULL);
		}
		break;
	case eAuthTypeSetPolicy:
		// [ authuser, authpass, username, policy ]
		if (CFArrayGetCount(authItems) == 4) {
			tmprecord = ODNodeCopyRecord(_ODRecordGetNode(record), record->_type, record->_name, NULL, error);
			if (tmprecord != NULL) {
				if (ODRecordSetNodeCredentials(tmprecord, CFArrayGetValueAtIndex(authItems, 0), CFArrayGetValueAtIndex(authItems, 1), error) == true) {
					tmpitems = CFArrayCreateMutable(NULL, 2, &kCFTypeArrayCallBacks);
					CFArrayAppendArray(tmpitems, authItems, CFRangeMake(2, 2));
					success = ODRecordVerifyPasswordExtended(record, kODAuthenticationTypeSetPolicyAsCurrent, tmpitems, authItemsOut, context, error);
					CFRelease(tmpitems);
				}
				CFRelease(tmprecord);
			}
		} else {
			_ODErrorSet(error, kODErrorCredentialsParameterError, NULL);
		}
		break;
	case eAuthTypeGetEffectivePolicy:
		// [ username ]
		if (CFArrayGetCount(authItems) == 1) {
			tmpitems = CFArrayCreateMutableCopy(NULL, 3, authItems);
			CFArrayInsertValueAtIndex(tmpitems, 0, CFSTR(""));
			CFArrayInsertValueAtIndex(tmpitems, 0, CFSTR(""));
			success = ODRecordVerifyPasswordExtended(record, kODAuthenticationTypeGetPolicy, tmpitems, authItemsOut, context, error);
			CFRelease(tmpitems);
		} else {
			_ODErrorSet(error, kODErrorCredentialsParameterError, NULL);
		}
		break;
	case eAuthTypeSimpleVerify:
		// [ username, password ]
		if (CFArrayGetCount(authItems) == 2) {
			success = ODRecordVerifyPassword(record, CFArrayGetValueAtIndex(authItems, 1), error);
		} else {
			_ODErrorSet(error, kODErrorCredentialsParameterError, NULL);
		}
		break;
	}

	return success;
}

bool
extauth_node_verify(ODNodeRef node, ODRecordType recordType, uint32_t mapped_type, CFArrayRef authItems, CFArrayRef *authItemsOut, ODContextRef *context, CFErrorRef *error)
{
	bool success = false;
	CFIndex recname_idx;
	ODRecordRef record;

	recname_idx = (mapped_type == eAuthTypeSetPolicy) ? 2 : 0;

	if (CFArrayGetCount(authItems) > recname_idx) {
		record = ODNodeCopyRecord(node, recordType, CFArrayGetValueAtIndex(authItems, recname_idx), NULL, error);
		if (record != NULL) {
			success = extauth_record_verify(record, mapped_type, authItems, authItemsOut, context, error);
			CFRelease(record);
		}
	} else {
		_ODErrorSet(error, kODErrorCredentialsParameterError, NULL);
	}

	return success;
}
