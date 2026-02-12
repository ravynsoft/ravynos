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
#include "DirServicesTypes.h"
#include "CFOpenDirectory.h"
#include "CFOpenDirectoryPriv.h"
#include "internal.h"

const CFStringRef kODErrorDomainFramework = CFSTR("com.apple.OpenDirectory");

static CFStringRef MapCodeToReason(uint32_t code);

void
_ODErrorSet(CFErrorRef *error, uint32_t code, CFStringRef info)
{
	if (error == NULL) {
		return;
	}

	if (code) {
		CFStringRef keys[2], vals[2];

		keys[0] = kCFErrorLocalizedDescriptionKey;
		if (info) {
			vals[0] = CFRetain(info);
		} else {
			// TODO: 7708162
			vals[0] = MapCodeToReason(code);
		}

		keys[1] = kCFErrorLocalizedFailureReasonKey;
		vals[1] = MapCodeToReason(code);

		*error = CFErrorCreateWithUserInfoKeysAndValues(NULL, kODErrorDomainFramework, code, (const void **)keys, (const void **)vals, 2);

		CFRelease(vals[0]);
		CFRelease(vals[1]);
	} else {
		*error = NULL;
	}
}

bool
_validate_nonnull(CFErrorRef *error, CFStringRef desc, const void *ptr)
{
	if (ptr) {
		return true;
	}
	if (error) {
		CFStringRef keys[2], vals[2];
		keys[0] = kCFErrorLocalizedDescriptionKey;
		vals[0] = CFSTR("Invalid argument.");
		keys[1] = kCFErrorLocalizedFailureReasonKey;
		vals[1] = CFStringCreateWithFormat(NULL, NULL, CFSTR("Invalid %@ reference."), desc);
		*error = CFErrorCreateWithUserInfoKeysAndValues(NULL, kODErrorDomainFramework, kODErrorRecordParameterError, (const void **)keys, (const void **)vals, 2);
		CFRelease(vals[0]);
		CFRelease(vals[1]);
	}
	return false;
}

static CFStringRef
MapCodeToReason(uint32_t code)
{
	static dispatch_once_t once;
	static CFBundleRef bundle;
	CFStringRef reason;

	dispatch_once(&once, ^{
		bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.CFOpenDirectory"));
	});

	switch (code) {
	case kODErrorSessionLocalOnlyDaemonInUse:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Session can't be opened to normal daemon because local only references are open."), NULL, bundle, NULL);
		break;
	case kODErrorSessionNormalDaemonInUse:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Session can't be opened to local only daemon because normal daemon references are open."), NULL, bundle, NULL);
		break;
	case kODErrorSessionDaemonNotRunning:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Session can't be opened because daemon is not running."), NULL, bundle, NULL);
		break;
	case kODErrorSessionDaemonRefused:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Session can't be opened because daemon refused the connection."), NULL, bundle, NULL);
		break;
	case kODErrorSessionProxyCommunicationError:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Proxy failed due to a communication error."), NULL, bundle, NULL);
		break;
	case kODErrorSessionProxyVersionMismatch:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Proxy failed because the version is not supported by this client."), NULL, bundle, NULL);
		break;
	case kODErrorSessionProxyIPUnreachable:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Proxy failed because the host provided is not responding."), NULL, bundle, NULL);
		break;
	case kODErrorSessionProxyUnknownHost:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Proxy failed because an unknown host was provided."), NULL, bundle, NULL);
		break;
	case kODErrorNodeUnknownName:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Node name wasn't found."), NULL, bundle, NULL);
		break;
	case kODErrorNodeUnknownType:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Unable to open node type requested."), NULL, bundle, NULL);
		break;
	case kODErrorNodeConnectionFailed:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Connection failed to the directory server."), NULL, bundle, NULL);
		break;
	case kODErrorNodeUnknownHost:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Could not resolve the address."), NULL, bundle, NULL);
		break;
	case kODErrorQuerySynchronize:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Previous results are no longer valid because a synchronize call was requested."), NULL, bundle, NULL);
		break;
	case kODErrorQueryInvalidMatchType:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("An invalid search type was provided during the query."), NULL, bundle, NULL);
		break;
	case kODErrorQueryUnsupportedMatchType:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("That type of search is not supported by the directory node."), NULL, bundle, NULL);
		break;
	case kODErrorQueryTimeout:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("The query exceeded the maximum time allowed."), NULL, bundle, NULL);
		break;
	case kODErrorRecordReadOnlyNode:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Unable to modify record because the directory node is read only."), NULL, bundle, NULL);
		break;
	case kODErrorRecordPermissionError:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Operation was denied because the current credentials do not have the appropriate privileges."), NULL, bundle, NULL);
		break;
	case kODErrorRecordParameterError:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("One of the parameters provided was invalid."), NULL, bundle, NULL);
		break;
	case kODErrorRecordInvalidType:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("An invalid record type was provided."), NULL, bundle, NULL);
		break;
	case kODErrorRecordAlreadyExists:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Could not create the record because one already exists with the same name."), NULL, bundle, NULL);
		break;
	case kODErrorRecordTypeDisabled:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("The record type provided is not allowed by the node."), NULL, bundle, NULL);
		break;
	case kODErrorRecordAttributeUnknownType:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("An invalid attribute type was provided."), NULL, bundle, NULL);
		break;
	case kODErrorRecordAttributeNotFound:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("The requested attribute could not be found."), NULL, bundle, NULL);
		break;
	case kODErrorRecordAttributeValueSchemaError:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("The attribute value could not be used because it does not meet the requirements of the attribute."), NULL, bundle, NULL);
		break;
	case kODErrorRecordAttributeValueNotFound:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("The requested attribute value could not be found."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsInvalid:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Credentials could not be verified, username or password is invalid."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsMethodNotSupported:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Could not verify credentials because directory server does not support the requested authentication method."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsNotAuthorized:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Authentication server refused operation because the current credentials are not authorized for the requested operation."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsParameterError:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Credential operation failed because an invalid parameter was provided."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsOperationFailed:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Authentication server failed to complete the requested operation."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsServerUnreachable:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Authentication server could not be contacted."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsServerNotFound:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Authentication server could not be found for the requested operation."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsServerError:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Authentication server encountered an error while attempting the requested operation."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsServerTimeout:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Authentication server timed out while attempting the requested operation."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsContactMaster:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Authentication server refused the operation because it wasn't the master."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsServerCommunicationError:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Authentication server encountered a communication error while attempting the requested operation."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsAccountNotFound:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Credentials failed because authentication server could not find the account."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsAccountDisabled:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Credential verification failed because account is disabled."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsAccountExpired:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Credential verification failed because account is expired."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsAccountInactive:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Credential verification failed because account is inactive."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsPasswordExpired:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Credential verification failed because password has expired."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsPasswordChangeRequired:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Password change is required by authentication server."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsPasswordQualityFailed:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Password change failed because password does not meet minimum quality requirements."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsPasswordTooShort:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Password change failed because password is too short."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsPasswordTooLong:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Password change failed because password is too long."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsPasswordNeedsLetter:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Password change failed because password requires a letter."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsPasswordNeedsDigit:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Password change failed because password requires a number."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsPasswordChangeTooSoon:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Password change failed because password was changed recently."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsPasswordUnrecoverable:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Credential server can't recover password for verification."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsInvalidLogonHours:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Not allowed to log into the computer outside of designated hours."), NULL, bundle, NULL);
		break;
	case kODErrorCredentialsInvalidComputer:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Not allowed to log into the computer."), NULL, bundle, NULL);
		break;
	case kODErrorPluginOperationNotSupported:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Operation is not supported by the directory node."), NULL, bundle, NULL);
		break;
	case kODErrorPluginError:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("The plugin encountered an error processing request."), NULL, bundle, NULL);
		break;
	case kODErrorDaemonError:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("The daemon encountered an error processing request."), NULL, bundle, NULL);
		break;
	default:
		reason = CFCopyLocalizedStringFromTableInBundle(CFSTR("Unknown error code."), NULL, bundle, NULL);
		break;
	}

	// TODO: localize

	return reason;
}

tDirStatus
ODConvertToLegacyErrorCode(CFIndex inCode)
{
	if (inCode <= 0) {
		return inCode;
	}

	switch (inCode) {
		case kODErrorSessionLocalOnlyDaemonInUse:
			return eDSLocalDSDaemonInUse;

		case kODErrorSessionNormalDaemonInUse:
			return eDSNormalDSDaemonInUse;

		case kODErrorSessionDaemonRefused:
			return eDSOpenFailed;

		case kODErrorNodeUnknownName:
			return eDSOpenNodeFailed;

		case kODErrorNodeConnectionFailed:
			return eDSCannotAccessSession;

		case kODErrorNodeUnknownHost:
			return eDSBogusServer;

		case kODErrorRecordReadOnlyNode:
			return eDSReadOnly;

		case kODErrorQueryUnsupportedMatchType:
			return eDSUnSupportedMatchType;

		case kODErrorRecordAttributeUnknownType:
			return eDSNoStdMappingAvailable;

		case kODErrorRecordAttributeNotFound:
			return eDSAttributeNotFound;

		case kODErrorRecordAttributeValueSchemaError:
			return eDSSchemaError;

		case kODErrorRecordAlreadyExists:
			return eDSRecordAlreadyExists;

		case kODErrorRecordNoLongerExists:
			return eDSRecordNotFound;

		case kODErrorRecordPermissionError:
			return eDSPermissionError;

		case kODErrorRecordAttributeValueNotFound:
			return eDSAttributeValueNotFound;

		case kODErrorSessionDaemonNotRunning:
			return eServerNotRunning;

		case kODErrorDaemonError:
			return eServerError;

		case kODErrorSessionProxyVersionMismatch:
			return eDSVersionMismatch;

		case kODErrorSessionProxyCommunicationError:
			return eDSTCPSendError;

		case kODErrorSessionProxyIPUnreachable:
			return eDSIPUnreachable;

		case kODErrorSessionProxyUnknownHost:
			return eDSUnknownHost;

		case kODErrorQueryInvalidMatchType:
			return eDSUnknownMatchType;

		case kODErrorRecordParameterError:
			return eParameterError;

		case kODErrorRecordInvalidType:
			return eDSInvalidRecordType;

		case kODErrorRecordTypeDisabled:
			return eDSRecordTypeDisabled;

		case kODErrorPluginOperationNotSupported:
			return eNotYetImplemented;

		case kODErrorCredentialsInvalid:
			return eDSAuthFailed;

		case kODErrorCredentialsMethodNotSupported:
			return eDSAuthMethodNotSupported;

		case kODErrorCredentialsAccountNotFound:
			return eDSAuthUnknownUser;

		case kODErrorCredentialsNotAuthorized:
			return eDSNotAuthorized;

		case kODErrorCredentialsParameterError:
			return eDSAuthParameterError;

		case kODErrorCredentialsOperationFailed:
			return eDSOperationFailed;

		case kODErrorCredentialsServerUnreachable:
			return eDSServiceUnavailable;

		case kODErrorCredentialsServerNotFound:
			return eDSAuthNoAuthServerFound;

		case kODErrorCredentialsServerCommunicationError:
			return eDSBadPacket;

		case kODErrorCredentialsServerError:
			return eDSInvalidSession;

		case kODErrorCredentialsServerTimeout:
			return eDSAuthMasterUnreachable;

		case kODErrorCredentialsContactMaster:
			return eDSContactMaster;

		case kODErrorCredentialsPasswordChangeRequired:
			return eDSAuthNewPasswordRequired;

		case kODErrorCredentialsPasswordExpired:
			return eDSAuthPasswordExpired;

		case kODErrorCredentialsPasswordQualityFailed:
			return eDSAuthPasswordQualityCheckFailed;

		case kODErrorCredentialsAccountDisabled:
			return eDSAuthAccountDisabled;

		case kODErrorCredentialsAccountExpired:
			return eDSAuthAccountExpired;

		case kODErrorCredentialsAccountInactive:
			return eDSAuthAccountInactive;

		case kODErrorCredentialsPasswordTooShort:
			return eDSAuthPasswordTooShort;

		case kODErrorCredentialsPasswordTooLong:
			return eDSAuthPasswordTooLong;

		case kODErrorCredentialsPasswordNeedsLetter:
			return eDSAuthPasswordNeedsLetter;

		case kODErrorCredentialsPasswordNeedsDigit:
			return eDSAuthPasswordNeedsDigit;

		case kODErrorCredentialsPasswordChangeTooSoon:
			return eDSAuthPasswordChangeTooSoon;

		case kODErrorCredentialsInvalidLogonHours:
			return eDSAuthInvalidLogonHours;

		case kODErrorCredentialsInvalidComputer:
			return eDSAuthInvalidComputer;

		case kODErrorCredentialsPasswordUnrecoverable:
			return eDSUnrecoverablePassword;

		case kODErrorPluginError:
			return eUndefinedError;
			
		case kODErrorNodeDisabled:
			return ePlugInNotActive;
	}

	return eUndefinedError;
}
