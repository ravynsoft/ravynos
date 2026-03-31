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
#include <uuid/uuid.h>
#include <ifaddrs.h>
#include <netdb.h>

#include "CFOpenDirectoryPriv.h"

#include <opendirectory/odutils.h>
#include <sys/stat.h>

#include "CFODSession.h"
#include "transaction.h"
#include "internal.h"

const CFStringRef kODSessionLocalPath = CFSTR("LocalPath");
const CFStringRef kODSessionProxyAddress = CFSTR("ProxyAddress");
const CFStringRef kODSessionProxyPort = CFSTR("ProxyPort");
const CFStringRef kODSessionProxyUsername = CFSTR("ProxyUsername");
const CFStringRef kODSessionProxyPassword = CFSTR("ProxyPassword");

ODSessionRef kODSessionDefault;

struct __ODSession {
	CFRuntimeBase _base;
	uuid_t _uuid;
	CFDictionaryRef _info;
};

static CFTypeID __kODSessionTypeID = _kCFRuntimeNotATypeID;

static void
__ODSessionFinalize(CFTypeRef cf)
{
	ODSessionRef session = (ODSessionRef)cf;
	CFArrayRef response;
	uint32_t code;

	if (!uuid_is_null(session->_uuid)) {
		response = transaction_simple(&code, session, NULL, CFSTR("ODSessionRelease"), 0);
		safe_cfrelease_null(response);
	}

	if (session->_info) {
		CFRelease(session->_info);
	}
}

static CFStringRef
__ODSessionCopyDebugDesc(CFTypeRef cf)
{
	ODSessionRef session = (ODSessionRef)cf;
	CFDictionaryRef info = session->_info;
	CFIndex count = info ? CFDictionaryGetCount(info) : 0;
	uuid_string_t uuid;
	CFStringRef urlstr, output;
	if (count == 1) {
		urlstr = CFStringCreateWithFormat(NULL, NULL, CFSTR(" [%@]"), CFDictionaryGetValue(info, kODSessionLocalPath));
	} else if (count >= 3) {
		urlstr = CFStringCreateWithFormat(NULL, NULL, CFSTR(" [%@@%@:%@]"), CFDictionaryGetValue(info, kODSessionProxyUsername), CFDictionaryGetValue(info, kODSessionProxyAddress), CFDictionaryGetValue(info, kODSessionProxyPort));
	} else {
		urlstr = CFSTR("");
	}
	uuid_unparse_upper(session->_uuid, uuid);
	output = CFStringCreateWithFormat(NULL, NULL, CFSTR("<ODSession %p [uuid: %s]%@>"), session, uuid, urlstr);
	CFRelease(urlstr);
	return output;
}

static const CFRuntimeClass __ODSessionClass = {
	0,								// version
	"ODSession",					// className
	NULL,							// init
	NULL,							// copy
	__ODSessionFinalize,			// finalize
	NULL,							// equal
	NULL,							// hash
	NULL,							// copyFormattingDesc
	__ODSessionCopyDebugDesc,		// copyDebugDesc
	NULL,							// reclaim
#if CF_REFCOUNT_AVAILABLE
	NULL,							// refcount
#endif
};

#pragma mark Helper Functions

static bool
is_local(CFStringRef address)
{
	char hostname[NI_MAXHOST];
	struct addrinfo hints, *res, *res0;
	int error;
	struct ifaddrs *ifa, *ifap;
	struct sockaddr_in *sin4, *sin4_if;
	struct sockaddr_in6 *sin6, *sin6_if;
	bool local;

	local = false;

	if (!CFStringGetCString(address, hostname, sizeof(hostname), kCFStringEncodingUTF8)) {
		return false;
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	error = getaddrinfo(hostname, NULL, &hints, &res0);
	if (error) {
		return false;
	}

	(void)getifaddrs(&ifap); // ok if this fails

	for (res = res0; res && !local; res = res->ai_next) {
		switch (res->ai_family) {
		case AF_INET:
			sin4 = (struct sockaddr_in *)res->ai_addr;
			if (sin4->sin_addr.s_addr == htonl(INADDR_LOOPBACK)) {
				local = true;
			}
			for (ifa = ifap; ifa && !local; ifa = ifa->ifa_next) {
				if (ifa->ifa_addr->sa_family == AF_INET) {
					sin4_if = (struct sockaddr_in *)ifa->ifa_addr;
					if (sin4->sin_addr.s_addr == sin4_if->sin_addr.s_addr) {
						local = true;
					}
				}
			}
			break;
		case AF_INET6:
			sin6 = (struct sockaddr_in6 *)res->ai_addr;
			if (IN6_IS_ADDR_LOOPBACK(&sin6->sin6_addr)) {
				local = true;
			}
			for (ifa = ifap; ifa && !local; ifa = ifa->ifa_next) {
				if (ifa->ifa_addr->sa_family == AF_INET6) {
					sin6_if = (struct sockaddr_in6 *)ifa->ifa_addr;
					if (IN6_ARE_ADDR_EQUAL(&sin6->sin6_addr, &sin6_if->sin6_addr)) {
						local = true;
					}
				}
			}
			break;
		case AF_UNIX:
			local = true;
			break;
		default:
			break;
		}
	}

	freeifaddrs(ifap);
	freeaddrinfo(res0);

	return local;
}

#pragma mark -

void
_ODInitialize(void)
{
	static dispatch_once_t once;

	dispatch_once(&once, ^(void) {
		ODSessionGetTypeID();
		ODNodeGetTypeID();
		ODQueryGetTypeID();
		ODRecordGetTypeID();
	});
}

void
uuid_copy_session(uuid_t dst, ODSessionRef session)
{
	if (session) {
		uuid_copy(dst, session->_uuid);
	} else {
		uuid_clear(dst);
	}
}

ODSessionRef
_ODSessionGetShared()
{
	static dispatch_once_t once;

	dispatch_once(&once, ^ {
		kODSessionDefault = ODSessionCreate(NULL, NULL, NULL);
	});

	return kODSessionDefault;
}

CFTypeID
ODSessionGetTypeID(void)
{
	static dispatch_once_t once;

	dispatch_once(&once, ^ {
		__kODSessionTypeID = _CFRuntimeRegisterClass(&__ODSessionClass);
		if (__kODSessionTypeID != _kCFRuntimeNotATypeID) {
			_CFRuntimeBridgeClasses(__kODSessionTypeID, "NSODSession");
		}
	});

	return __kODSessionTypeID;
}

void
_ODSessionInit(ODSessionRef session, CFDictionaryRef options, CFErrorRef *error)
{
	const void *keys[4];
	const void *values[4];
	CFStringRef tmp;
	CFIndex n = 0;
	CFArrayRef response;
	uint32_t code;
	bool local = false;

	CLEAR_ERROR(error);

	if (options != NULL && CFDictionaryGetCount(options) > 0) {
		int					proxyargs	= 0;
		CFMutableStringRef	workPath	= NULL;

		/* Check for LocalPath first. */
		if ((tmp = CFDictionaryGetValue(options, kODSessionLocalPath))) {
			struct stat	localDirStat;
			struct stat	statResult;
			
			// see if we need to set a default path
			workPath = CFStringCreateMutableCopy(kCFAllocatorDefault, PATH_MAX, tmp);
			if (CFEqual(workPath, CFSTR("Default")) == false) {
				if (CFStringGetLength(workPath) <= PATH_MAX) {
					char newPath[PATH_MAX];
					
					if (CFStringHasSuffix(workPath, CFSTR("/")) == false) {
						CFStringAppend(workPath, CFSTR("/"));
					}
					
					if (CFStringHasSuffix(workPath, CFSTR("/dslocal/nodes/")) == true) {
						CFStringAppend(workPath, CFSTR("Default/"));
					}
					
					CFStringGetCString(workPath, newPath, sizeof(newPath), kCFStringEncodingUTF8);
					
					// let's see if we are trying to get to the same place
					if (lstat(newPath, &statResult) == 0 && lstat("/var/db/dslocal/nodes/Default", &localDirStat) == 0) {
						// if these are not the same files
						if (statResult.st_ino != localDirStat.st_ino || statResult.st_dev != localDirStat.st_dev || statResult.st_gen != localDirStat.st_gen) {
							keys[n] = kODSessionLocalPath;
							values[n] = workPath;
							n++;
						}
					} else {
						// cannot stat, let opendirectoryd try
						keys[n] = kODSessionLocalPath;
						values[n] = workPath;
						n++;
					}
				}
			}
		} else {
			/* Otherwise, grab proxy information. */

			if ((tmp = CFDictionaryGetValue(options, kODSessionProxyAddress))) {
				local = is_local(tmp);

				keys[n] = kODSessionProxyAddress;
				values[n] = tmp;
				n++;
				proxyargs++;
			}
			if ((tmp = CFDictionaryGetValue(options, kODSessionProxyPort))) {
				keys[n] = kODSessionProxyPort;
				values[n] = tmp;
				n++;
			}
			if ((tmp = CFDictionaryGetValue(options, kODSessionProxyUsername))) {
				keys[n] = kODSessionProxyUsername;
				values[n] = tmp;
				n++;
				proxyargs++;
			}
			if ((tmp = CFDictionaryGetValue(options, kODSessionProxyPassword))) {
				keys[n] = kODSessionProxyPassword;
				values[n] = tmp;
				n++;
				proxyargs++;
			}

			/* Verify that all necessary proxy information was provided. */
			if (proxyargs != 3) {
				_ODErrorSet(error, kODErrorRecordParameterError, CFSTR("Invalid proxy arguments."));
				return;
			}
		}

		if (n > 0 && local == false) {
			session->_info = CFDictionaryCreate(NULL, keys, values, n, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

			response = transaction_simple(&code, NULL, NULL, CFSTR("ODSessionCreate"), 1, session->_info);
			transaction_simple_response(response, code, 1, error, ^ {
				uuid_copy(session->_uuid, CFDataGetBytePtr(schema_get_value_at_index(response, 0)));
			});
		}
		
		safe_cfrelease_null(workPath);
	}
}

ODSessionRef
_ODSessionCreate(CFAllocatorRef allocator)
{
	return (ODSessionRef)_CFRuntimeCreateInstance(allocator, ODSessionGetTypeID(), sizeof(struct __ODSession) - sizeof(CFRuntimeBase), NULL);
}

ODSessionRef
ODSessionCreate(CFAllocatorRef allocator, CFDictionaryRef options, CFErrorRef *error)
{
	CLEAR_ERROR(error);

	CFErrorRef		local_error		= NULL;
	ODSessionRef	session			= _ODSessionCreate(allocator);

	if (session != NULL) {
		_ODSessionInit(session, options, &local_error);
	} else {
		if (error != NULL) {
			// TODO: do we have general allocation error?
			(*error) = CFErrorCreate(kCFAllocatorDefault, kODErrorDomainFramework, kODErrorSessionFailed, NULL);
		}
	}

	if (local_error != NULL) {
		CFRelease(session);
		session = NULL;

		if (error != NULL) {
			(*error) = local_error;
		} else {
			CFRelease(local_error);
		}
	}

	return session;
}

CFArrayRef
ODSessionCopyNodeNames(CFAllocatorRef allocator, ODSessionRef session, CFErrorRef *error)
{
	CLEAR_ERROR(error);

	if (session && CF_IS_OBJC(__kODSessionTypeID, session)) {
		CFArrayRef nodes = NULL;
		CF_OBJC_CALL(CFArrayRef, nodes, session, "nodeNamesAndReturnError:", error);
		return nodes ? CFRetain(nodes) : NULL;
	}

	__block CFArrayRef nodes = NULL;
	CFArrayRef response;
	uint32_t code;

	response = transaction_simple(&code, session, NULL, CFSTR("ODSessionCopyNodeNames"), 0);
	transaction_simple_response(response, code, 1, error, ^ {
		nodes = schema_get_value_at_index(response, 0);
		if (nodes) CFRetain(nodes);
	});

	return nodes;
}

// 7558703
bool
ODSessionNodeNameIsLocal(ODSessionRef session __unused, CFStringRef nodename)
{
	if (CFEqual(nodename, CFSTR("/BSD/local"))) {
		return true;
	}
	if (CFStringHasPrefix(nodename, CFSTR("/Local/"))) {
		return true;
	}
	return false;
}

// TODO: could simplify this by looking up all functions at once, but doesn't save much since bundle is cached

ODSessionRef
ODSessionCreateWithDSRef(CFAllocatorRef inAllocator, tDirReference inDirRef, bool inCloseOnRelease)
{
	static dispatch_once_t once;
	static ODSessionRef(*dsCopyDataFromDirRef)(tDirReference inNodeRef);

	// We have to load this dynamically because otherwise we create a circular dependency
	dispatch_once(&once,
	^(void) {
		CFBundleRef bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.DirectoryService.Framework"));
		if (bundle != NULL) {
			dsCopyDataFromDirRef = CFBundleGetFunctionPointerForName(bundle, CFSTR("dsCopyDataFromDirRef"));
		}
	});

	if (dsCopyDataFromDirRef != NULL) {
		return dsCopyDataFromDirRef(inDirRef);
	} else {
		OD_CRASH("DirectoryService.framework missing function: dsCopyDataFromDirRef");
	}

	return NULL;
}

tDirReference
ODSessionGetDSRef(ODSessionRef inSessionRef)
{
	static dispatch_once_t once;
	static tDirReference(*dsCreateDataDirRefData)(CFTypeRef data);

	// We have to load this dynamically because otherwise we create a circular dependency
	dispatch_once(&once,
	^(void) {
		CFBundleRef bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.DirectoryService.Framework"));
		if (bundle != NULL) {
			dsCreateDataDirRefData = CFBundleGetFunctionPointerForName(bundle, CFSTR("dsCreateDataDirRefData"));
		}
	});

	if (dsCreateDataDirRefData != NULL) {
		// TODO: need to track the reference we create and close it accordingly (just call dsCloseDirService)
		return dsCreateDataDirRefData(inSessionRef);
	} else {
		OD_CRASH("DirectoryService.framework missing function: dsCreateDataDirRefData");
	}

	return 0;
}
