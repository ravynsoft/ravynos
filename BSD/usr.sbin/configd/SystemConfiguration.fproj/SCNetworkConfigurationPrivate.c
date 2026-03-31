/*
 * Copyright (c) 2019, 2020 Apple Inc. All rights reserved.
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

/*
 * Modification History
 *
 * October 4, 2018		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */


#include "SCNetworkConfigurationInternal.h"
#include "SCPreferencesInternal.h"
#include <IOKit/IOBSD.h>


#define	logDetails	(_sc_log == kSCLogDestinationDefault) || _sc_debug


static Boolean
savePreferences(SCPreferencesRef	prefs,
		CFStringRef		save_prefsID,
		CFStringRef		prefix,
		Boolean			remove,
		CFStringRef		extra_key,
		CFPropertyListRef	extra_value)
{
	const CFStringRef	keys[] = {
		kSCPrefCurrentSet,
		MODEL,
		kSCPrefNetworkServices,
		kSCPrefSets,
		kSCPrefSystem,
		kSCPrefVersion,
		kSCPrefVirtualNetworkInterfaces
	};
	Boolean			ok;
	SCPreferencesRef	save_prefs;

	// open [companion] backup
	save_prefs = SCPreferencesCreateCompanion(prefs, save_prefsID);

	for (CFIndex i = 0; i < (CFIndex)(sizeof(keys)/sizeof(keys[0])); i++) {
		CFStringRef	key	= keys[i];
		CFStringRef	src_key;
		CFTypeRef	val;

		src_key = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@%@"), prefix, key);
		val = SCPreferencesGetValue(prefs, src_key);
		if (val != NULL) {
			SCPreferencesSetValue(save_prefs, key, val);
			if (remove) {
				SCPreferencesRemoveValue(prefs, src_key);
			}
		}
		CFRelease(src_key);
	}

	if (extra_key != NULL) {
		SCPreferencesSetValue(save_prefs, extra_key, extra_value);
	}

	ok = SCPreferencesCommitChanges(save_prefs);
	CFRelease(save_prefs);
	if (!ok) {
		SC_log(LOG_ERR, "could not save preferences (%@): %s",
		       save_prefsID,
		       SCErrorString(SCError()));
	}
	return ok;
}


__private_extern__
Boolean
__SCNetworkConfigurationBackup(SCPreferencesRef prefs, CFStringRef suffix, SCPreferencesRef relativeTo)
{
	SCPreferencesRef	backup;
	CFMutableStringRef	backupPrefsID;
	Boolean			ok		= FALSE;
	CFPropertyListRef	plist;
	CFRange			range;
	SCPreferencesPrivateRef	sourcePrivate	= (SCPreferencesPrivateRef)prefs;
	CFStringRef		sourcePrefsID;

	SC_log(LOG_NOTICE, "creating [%@] backup", suffix);

	sourcePrefsID = (sourcePrivate->prefsID != NULL) ? sourcePrivate->prefsID : PREFS_DEFAULT_CONFIG;
	backupPrefsID = CFStringCreateMutableCopy(NULL, 0, sourcePrefsID);
	if (CFStringFindWithOptions(backupPrefsID,
				    CFSTR("/"),
				    CFRangeMake(0, CFStringGetLength(backupPrefsID)),
				    kCFCompareBackwards,
				    &range)) {
		// if slash, remove path prefix
		range.length   = range.location + 1;
		range.location = 0;
		CFStringReplace(backupPrefsID, range, CFSTR(""));
	}
	CFStringInsert(backupPrefsID,
		       CFStringGetLength(backupPrefsID) - sizeof(".plist") + 1,
		       CFSTR("-"));
	CFStringInsert(backupPrefsID,
		       CFStringGetLength(backupPrefsID) - sizeof(".plist") + 1,
		       suffix);
	backup = SCPreferencesCreateCompanion(relativeTo, backupPrefsID);
	CFRelease(backupPrefsID);

	SC_log(LOG_INFO,
	       "__SCNetworkConfigurationBackup()"
	       "\n  relativeTo = %@"
	       "\n  prefs      = %@"
	       "\n  backup     = %@",
	       relativeTo,
	       prefs,
	       backup);

	if (backup != NULL) {
		plist = SCPreferencesPathGetValue(prefs, CFSTR("/"));
		SCPreferencesPathSetValue(backup, CFSTR("/"), plist);
		ok = SCPreferencesCommitChanges(backup);
		CFRelease(backup);
	}

	return ok;
}


Boolean
__SCNetworkConfigurationSaveModel(SCPreferencesRef prefs, CFStringRef model)
{
	Boolean		ok;
	CFStringRef	save_prefsID;

	SC_log(LOG_NOTICE, "creating [per-device] backup: %@", model);

	save_prefsID = CFStringCreateWithFormat(NULL, NULL, CFSTR("preferences-%@.plist"), model);
	ok = savePreferences(prefs, save_prefsID, CFSTR(""), TRUE, MODEL, model);
	CFRelease(save_prefsID);
	return ok;
}


static Boolean
needsUpdate(SCPreferencesRef prefs, int new_version)
{
	CFNumberRef	num;
	int		old_version	= 0;

	if (prefs == NULL) {
		// if no prefs, no updated needed
		return FALSE;
	}

	num = SCPreferencesGetValue(prefs, kSCPrefVersion);
	if (!isA_CFNumber(num) ||
	    !CFNumberGetValue(num, kCFNumberIntType, &old_version)) {
		old_version = 0;
	}
	if (old_version == new_version) {
		// if no update is needed
		return FALSE;
	}

	return TRUE;
}


static Boolean
lockWithSync(SCPreferencesRef prefs)
{
	Boolean		ok;

	assert(prefs != NULL);
	ok = SCPreferencesLock(prefs, TRUE);
	if (!ok && (SCError() == kSCStatusStale)) {
		SCPreferencesSynchronize(prefs);
		ok = SCPreferencesLock(prefs, TRUE);
	}

	return ok;
}


Boolean
__SCNetworkConfigurationUpgrade(SCPreferencesRef	*prefs_p,
				SCPreferencesRef	*ni_prefs_p,
				Boolean			commit)
{
	SCPreferencesRef	ni_prefs	= NULL;
	Boolean			ni_prefs_added	= FALSE;
	const int		new_version	= NETWORK_CONFIGURATION_VERSION;
	CFNumberRef		num;
	Boolean			ok		= FALSE;
	SCPreferencesRef	prefs		= NULL;
	Boolean			prefs_added	= FALSE;

	//
	// The following table describes how the SPI is called (input parameters), what actions
	// are performed, and whether any changes should be committed as part of the call.
	//
	// +====================+===========================+===========================+========+
	// |                    |     preferences.plist     |  NetworkInterfaces.plist  |        |
	// |                    +=============+=============+=============+=============+ COMMIT |
	// |                    |     ptr     |    plist    |     ptr     |    plist    |        |
	// +====================+=============+=============+=============+=============+========+
	// | InterfaceNamer     |    NULL     | CREATE, REL |  &ni_prefs  |     USE     |  YES   |
	// +====================+=============+=============+=============+=============+========+
	// | PreferencesMonitor |   &prefs    |     USE     |    NULL     |  NO UPDATE  |  YES   |
	// +====================+=============+=============+=============+=============+========+
	// | scutil             |   &prefs    |     USE     |  &ni_prefs  | USE/CREATE  |   NO   |
	// +====================+=============+=============+=============+=============+========+
	//
	// For InterfaceNamer, we are passed a reference to the SCPreferences[Ref] for the
	// NetworkInterfaces.plist.  During the upgrade process we create (and then release)
	// the companion preferences.plist.  Any needed changes will be committed to both
	// plists.
	//
	// For PreferencesMonitor, we are passed a reference to the SCPreferences[Ref] for
	// the preferences.plist.  Any needed changes to the plist will be committed.  The
	// companion NetworkInterfaces.plist is not passed nor is it referenced/modified.
	//
	// For scutil, we are passed references to the SCPreferences[Ref] for both the
	// preferences.plist and NetworkInterfaces.plist.  The NetworkInterfaces.plist
	// reference will be used, if already open.  Else, the companion will be created
	// and returned.  Regardless, any changes made will not be committed (we expect
	// one to use scutil's "commit" command).
	//

	if (prefs_p != NULL) {
		prefs = *prefs_p;
	}

	if (ni_prefs_p != NULL) {
		ni_prefs = *ni_prefs_p;
	}

	if ((prefs_p == NULL) && (ni_prefs_p != NULL) && (ni_prefs != NULL)) {
		// Here, we have been called by InterfaceNamer and need to get the [companion]
		// preferences.plist (we need to update both)
		prefs = SCPreferencesCreateCompanion(ni_prefs, NULL);
		if (prefs == NULL) {
			SC_log(LOG_ERR,
			       "__SCNetworkConfigurationUpgrade(): could not open [preferences.plist]: %s",
			       SCErrorString(SCError()));
			return FALSE;
		}
		prefs_added = TRUE;
	}

	if ((prefs_p != NULL) && (prefs != NULL) && (ni_prefs_p != NULL) && (ni_prefs == NULL)) {
		// Here, we have been called by scutil with the [companion] NetworkInterfaces.plist
		// not yet open.  Open the companion so that we can update both.
		ni_prefs = SCPreferencesCreateCompanion(prefs, INTERFACES_DEFAULT_CONFIG);
		if (ni_prefs == NULL) {
			SC_log(LOG_ERR,
			       "__SCNetworkConfigurationUpgrade(): could not open [NetworkInterfaces.plist]: %s",
			       SCErrorString(SCError()));
			return FALSE;
		}
		ni_prefs_added = TRUE;
	}

	if (!needsUpdate(prefs,    NETWORK_CONFIGURATION_VERSION) &&
	    !needsUpdate(ni_prefs, NETWORK_CONFIGURATION_VERSION)) {
		goto done;
	}

	// lock [preferences.plist] changes while we are updating
	ok = lockWithSync(prefs);
	if (!ok) {
		SC_log(LOG_ERR,
		       "__SCNetworkConfigurationUpgrade(): could not lock [preferences.plist]: %s",
		       SCErrorString(SCError()));
		goto done;
	}

	if (ni_prefs != NULL) {
		// lock [NetworkInterfaces.plist] changes while we are updating
		ok = lockWithSync(ni_prefs);
		if (!ok) {
			SC_log(LOG_ERR,
			       "__SCNetworkConfigurationUpgrade(): could not lock [NetworkInterfaces.plist]: %s",
			       SCErrorString(SCError()));
			SCPreferencesUnlock(prefs);
			goto done;
		}
	}

	// first, cleanup any leftover cruft from the configuration
	__SCNetworkConfigurationClean(prefs, ni_prefs);

	// update the version(s)
	num = CFNumberCreate(NULL, kCFNumberIntType, &new_version);
	SCPreferencesSetValue(prefs, kSCPrefVersion, num);
	CFRelease(num);
	if (ni_prefs != NULL) {
		num = CFNumberCreate(NULL, kCFNumberIntType, &new_version);
		SCPreferencesSetValue(ni_prefs, kSCPrefVersion, num);
		CFRelease(num);
	}

	if (commit) {
		// commit the [preferences.plist] changes
		ok = SCPreferencesCommitChanges(prefs);
		if (!ok) {
			SC_log(LOG_ERR,
			       "__SCNetworkConfigurationUpgrade(): update not saved [preferences.plist]: %s",
			       SCErrorString(SCError()));
		}
		if (ok) {
			ok = SCPreferencesApplyChanges(prefs);
			if (!ok) {
				SC_log(LOG_ERR,
				       "__SCNetworkConfigurationUpgrade(): update not applied [preferences.plist]: %s",
				       SCErrorString(SCError()));
			}
		}
	}
	SCPreferencesUnlock(prefs);

	if (ni_prefs != NULL) {
		if (commit) {
			// commit the [NetworkInterfaces.plist] changes
			if (ok) {
				ok = SCPreferencesCommitChanges(ni_prefs);
				if (!ok) {
					SC_log(LOG_ERR,
					       "__SCNetworkConfigurationUpgrade(): update not saved [NetworkInterfaces.plist]: %s",
					       SCErrorString(SCError()));
				}
			}
		}
		SCPreferencesUnlock(ni_prefs);
	}

    done :

	if (prefs_added) {
		// per the expected usage, even if we on-the-fly create
		// a [preferences.plist] companion it is not returned to
		// the caller.  So, just release.
		CFRelease(prefs);
	}

	if (ni_prefs_added) {
		if (ok && (ni_prefs_p != NULL)) {
			*ni_prefs_p = CFRetain(ni_prefs);
		}
		CFRelease(ni_prefs);
	}

	return ok;
}


#pragma mark -
#pragma mark Remove "Hidden" Interface Configurations


static Boolean
isThin(CFArrayRef interfaces, CFStringRef bsdName)
{
	Boolean		thin;

	thin = CFArrayContainsValue(interfaces,
				    CFRangeMake(0, CFArrayGetCount(interfaces)),
				    bsdName);
	return thin;
}


static Boolean
thinAdd(CFMutableArrayRef interfaces, CFStringRef bsdName)
{
	if (!CFArrayContainsValue(interfaces,
				  CFRangeMake(0, CFArrayGetCount(interfaces)),
				  bsdName)) {
		CFArrayAppendValue(interfaces, bsdName);
		return TRUE;
	}

	return FALSE;
}


static Boolean
thinRemove(CFMutableArrayRef interfaces, CFStringRef bsdName)
{
	CFIndex		n;

	n = CFArrayGetFirstIndexOfValue(interfaces,
					CFRangeMake(0, CFArrayGetCount(interfaces)),
					bsdName);
	if (n != kCFNotFound) {
		CFArrayRemoveValueAtIndex(interfaces, n);
		return TRUE;
	}

	return FALSE;
}


static CF_RETURNS_RETAINED CFStringRef
serviceMatchesTemplate(SCPreferencesRef prefs, SCNetworkServiceRef existingService)
{
	CFStringRef		conflict	= NULL;
	SCNetworkInterfaceRef	existingInterface;
	CFIndex			n;
	CFArrayRef		protocols;
	CFMutableArrayRef	protocolTypes;
	SCNetworkServiceRef	templateService;

	// create a temporary network service (so that we can get the template configuration)
	existingInterface = SCNetworkServiceGetInterface(existingService);
	if (existingInterface == NULL) {
		conflict = CFStringCreateCopy(NULL, CFSTR("could not get interface for service"));
		return conflict;
	}

	templateService = SCNetworkServiceCreate(prefs, existingInterface);
	if (templateService == NULL) {
		conflict = CFStringCreateCopy(NULL, CFSTR("could not create service for interface"));
		return conflict;
	}

	(void) SCNetworkServiceEstablishDefaultConfiguration(templateService);

	protocolTypes = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	// get protocol types from the existing service
	protocols = SCNetworkServiceCopyProtocols(existingService);
	if (protocols != NULL) {
		n = CFArrayGetCount(protocols);

		for (CFIndex i = 0; i < n; i++) {
			SCNetworkProtocolRef	protocol;
			CFStringRef		protocolType;

			protocol = CFArrayGetValueAtIndex(protocols, i);
			protocolType = SCNetworkProtocolGetProtocolType(protocol);
			if (!CFArrayContainsValue(protocolTypes,
						  CFRangeMake(0, CFArrayGetCount(protocolTypes)),
						  protocolType)) {
				CFArrayAppendValue(protocolTypes, protocolType);
			}
		}

		CFRelease(protocols);
	}

	// get protocol types from the template service
	protocols = SCNetworkServiceCopyProtocols(templateService);
	if (protocols != NULL) {
		n = CFArrayGetCount(protocols);

		for (CFIndex i = 0; i < n; i++) {
			SCNetworkProtocolRef	protocol;
			CFStringRef		protocolType;

			protocol = CFArrayGetValueAtIndex(protocols, i);
			protocolType = SCNetworkProtocolGetProtocolType(protocol);
			if (!CFArrayContainsValue(protocolTypes,
						  CFRangeMake(0, CFArrayGetCount(protocolTypes)),
						  protocolType)) {
				CFArrayAppendValue(protocolTypes, protocolType);
			}
		}

		CFRelease(protocols);
	}

	// compare the existing protocols with the template
	n = CFArrayGetCount(protocolTypes);
	for (CFIndex i = 0; i < n; i++) {
		CFDictionaryRef		existingConfiguration	= NULL;
		SCNetworkProtocolRef	existingProtocol;
		Boolean			match;
		CFStringRef		protocolType;
		CFDictionaryRef		templateConfiguration	= NULL;
		SCNetworkProtocolRef	templateProtocol;

		protocolType = CFArrayGetValueAtIndex(protocolTypes, i);
		existingProtocol = SCNetworkServiceCopyProtocol(existingService, protocolType);
		templateProtocol = SCNetworkServiceCopyProtocol(templateService, protocolType);

		do {
			// compare "enabled"
			match = ((existingProtocol != NULL) &&
				 (templateProtocol != NULL) &&
				 (SCNetworkProtocolGetEnabled(existingProtocol) == SCNetworkProtocolGetEnabled(templateProtocol)));
			if (!match) {
				conflict = CFStringCreateWithFormat(NULL, NULL,
								    CFSTR("conflicting %@ enable/disable"),
								    protocolType);
				break;		// if enable/disable conflict
			}

			if (existingProtocol != NULL) {
				existingConfiguration = SCNetworkProtocolGetConfiguration(existingProtocol);
			}
			if (templateProtocol != NULL) {
				templateConfiguration = SCNetworkProtocolGetConfiguration(templateProtocol);
			}
			match = _SC_CFEqual(existingConfiguration, templateConfiguration);
			if (!match) {
				conflict = CFStringCreateWithFormat(NULL, NULL,
								    CFSTR("conflicting %@ configuration"),
								    protocolType);
				break;		// if configuration conflict
			}
		} while (FALSE);

		if (existingProtocol != NULL) CFRelease(existingProtocol);
		if (templateProtocol != NULL) CFRelease(templateProtocol);
		if (!match) {
			break;
		}
	}

	(void) SCNetworkServiceRemove(templateService);
	CFRelease(templateService);
	CFRelease(protocolTypes);
	return conflict;
}


static Boolean
effectivelyHiddenConfiguration(SCNetworkInterfaceRef interface)
{
	const CFStringRef	known[] = {
		CFSTR("Apple TV"),
		CFSTR("Watch"),
		CFSTR("iPad"),
		CFSTR("iPhone"),
		CFSTR("iPod"),
	};
	CFStringRef		name;

	name = SCNetworkInterfaceGetLocalizedDisplayName(interface);
	for (int i = 0; i < (int)(sizeof(known) / sizeof(known[0])); i++) {
		if (CFStringHasPrefix(name, known[i])) {
			return TRUE;
		}
	}

	return FALSE;
}


static Boolean
__SCNetworkConfigurationCleanHiddenInterfaces(SCPreferencesRef prefs, SCPreferencesRef ni_prefs)
{
#pragma unused(prefs)
#pragma unused(ni_prefs)
	CFStringRef		bsdName;
	Boolean			changed		= FALSE;
	CFArrayRef		interfaces;
	CFMutableArrayRef	interfaces_thin;
	CFIndex			n;
#if	TARGET_OS_OSX
	CFDictionaryRef		nat_config;
	SCPreferencesRef	nat_prefs;
#endif	// TARGET_OS_OSX
	CFArrayRef		services;
	int			updated		= 0;

	// build a list of interfaces we "could" remove

	interfaces_thin = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	services = SCNetworkServiceCopyAll(prefs);
	if (services != NULL) {
		n = CFArrayGetCount(services);
		for (CFIndex i = 0; i < n; i++) {
			CFStringRef		conflict;
			SCNetworkInterfaceRef	interface;
			SCNetworkServiceRef	service;
			const char		*thin	= NULL;

			service = CFArrayGetValueAtIndex(services, i);
			interface = SCNetworkServiceGetInterface(service);
			bsdName = SCNetworkInterfaceGetBSDName(interface);

			if (bsdName == NULL) {
				// if no interface name
				if (logDetails) {
					SC_log(LOG_INFO,
					       "skipping service : %@ : %@ (no interface)",
					       SCNetworkServiceGetServiceID(service),
					       SCNetworkServiceGetName(service));
				}
				continue;
			}

			if (_SCNetworkInterfaceIsHiddenConfiguration(interface)) {
				thin = "hidden";
			} else if (effectivelyHiddenConfiguration(interface)) {
				thin = "effectively hidden";
			} else {
				// if not HiddenConfiguration
				if (logDetails) {
					SC_log(LOG_INFO,
					       "skipping service : %@ : %@ : %@ (not hidden)",
						SCNetworkServiceGetServiceID(service),
						SCNetworkServiceGetName(service),
						bsdName);
				}
				continue;
			}

			conflict = serviceMatchesTemplate(prefs, service);
			if (conflict != NULL) {
				// if any part of the service's configuration was changed
				if (logDetails) {
					SC_log(LOG_INFO,
					       "skipping service : %@ : %@ : %@ (%s, non-default, %@)",
					       SCNetworkServiceGetServiceID(service),
					       SCNetworkServiceGetName(service),
					       bsdName,
					       thin,
					       conflict);
				}
				CFRelease(conflict);
				continue;
			}

			if (logDetails) {
				SC_log(LOG_INFO, "candidate interface : %@ (%s)", bsdName, thin);
			}

			thinAdd(interfaces_thin, bsdName);
		}
	}

	// remove any virtual interfaces from the list

#if	!TARGET_OS_IPHONE
	interfaces = SCBondInterfaceCopyAll(prefs);
	if (interfaces != NULL) {
		CFIndex		n;

		n = CFArrayGetCount(interfaces);
		for (CFIndex i = 0; i < n; i++) {
			SCBondInterfaceRef	bondInterface;
			CFArrayRef		members;
			CFIndex			nn;

			bondInterface = CFArrayGetValueAtIndex(interfaces, i);
			members = SCBondInterfaceGetMemberInterfaces(bondInterface);
			nn = (members != NULL) ? CFArrayGetCount(members) : 0;
			for (CFIndex ii = 0; ii < nn; ii++) {
				SCNetworkInterfaceRef	member;

				member = CFArrayGetValueAtIndex(members, ii);
				bsdName = SCNetworkInterfaceGetBSDName(member);
				if ((bsdName != NULL) &&
				    thinRemove(interfaces_thin, bsdName)) {
					if (logDetails) {
						SC_log(LOG_INFO, "skipping interface : %@ (bond member)", bsdName);
					}
				}
			}
		}

		CFRelease(interfaces);
	}
#endif	// !TARGET_OS_IPHONE

	interfaces = SCBridgeInterfaceCopyAll(prefs);
	if (interfaces != NULL) {
		CFIndex		n;

		n = CFArrayGetCount(interfaces);
		for (CFIndex i = 0; i < n; i++) {
			SCBridgeInterfaceRef	bridgeInterface;
			CFArrayRef		members;
			CFIndex			nn;

			bridgeInterface = CFArrayGetValueAtIndex(interfaces, i);
			members = SCBridgeInterfaceGetMemberInterfaces(bridgeInterface);
			nn = (members != NULL) ? CFArrayGetCount(members) : 0;
			for (CFIndex ii = 0; ii < nn; ii++) {
				SCNetworkInterfaceRef	member;

				member = CFArrayGetValueAtIndex(members, ii);
				bsdName = SCNetworkInterfaceGetBSDName(member);
				if ((bsdName != NULL) &&
				    thinRemove(interfaces_thin, bsdName)) {
					if (logDetails) {
						SC_log(LOG_INFO, "skipping interface : %@ (bridge member)", bsdName);
					}
				}
			}
		}

		CFRelease(interfaces);
	}

	interfaces = SCVLANInterfaceCopyAll(prefs);
	if (interfaces != NULL) {
		CFIndex		n;

		n = CFArrayGetCount(interfaces);
		for (CFIndex i = 0; i < n; i++) {
			SCBridgeInterfaceRef	vlanInterface;
			SCNetworkInterfaceRef	physicalInterface;

			vlanInterface = CFArrayGetValueAtIndex(interfaces, i);
			physicalInterface = SCVLANInterfaceGetPhysicalInterface(vlanInterface);
			bsdName = SCNetworkInterfaceGetBSDName(physicalInterface);
			if ((bsdName != NULL) &&
			    thinRemove(interfaces_thin, bsdName)) {
				if (logDetails) {
					SC_log(LOG_INFO, "skipping interface : %@ (vlan physical)", bsdName);
				}
			}
		}

		CFRelease(interfaces);
	}

	// remove any "shared" interfaces from the list

#if	TARGET_OS_OSX
	nat_prefs = SCPreferencesCreateCompanion(prefs, CFSTR("com.apple.nat.plist"));
	nat_config = SCPreferencesGetValue(nat_prefs, CFSTR("NAT"));
	if (isA_CFDictionary(nat_config)) {
		CFBooleanRef	bVal		= NULL;
		Boolean		enabled		= FALSE;
		CFStringRef	sharedFrom	= NULL;
		CFArrayRef	sharedTo	= NULL;

		if (CFDictionaryGetValueIfPresent(nat_config,
						  CFSTR("Enabled"),
						  (const void **)&bVal) &&
		    isA_CFBoolean(bVal)) {
		    enabled = CFBooleanGetValue(bVal);
		}

		if (enabled &&
		    CFDictionaryGetValueIfPresent(nat_config,
						  CFSTR("PrimaryService"),
						  (const void **)&sharedFrom) &&
			isA_CFString(sharedFrom)) {
			SCNetworkInterfaceRef	interface;
			SCNetworkServiceRef	service;

			// if "Share your connection from" service configured
			service = SCNetworkServiceCopy(prefs, sharedFrom);
			if (service != NULL) {
				interface = SCNetworkServiceGetInterface(service);
				bsdName = SCNetworkInterfaceGetBSDName(interface);
				if ((bsdName != NULL) &&
				    thinRemove(interfaces_thin, bsdName)) {
					if (logDetails) {
						SC_log(LOG_INFO, "skipping interface : %@ (Share your connection from)", bsdName);
					}
				}
				CFRelease(service);
			} else {
				SC_log(LOG_INFO, "keeping [not found] service : %@ (Share your connection from)", sharedFrom);
			}
		}

		if (enabled &&
		    CFDictionaryGetValueIfPresent(nat_config,
						  CFSTR("SharingDevices"),
						  (const void **)&sharedTo) &&
		    isA_CFArray(sharedTo)) {
			// if "To computers using" interfaces configured
			n = CFArrayGetCount(sharedTo);
			for (CFIndex i = 0; i < n; i++) {
				bsdName = CFArrayGetValueAtIndex(sharedTo, i);
				if (thinRemove(interfaces_thin, bsdName)) {
					if (logDetails) {
						SC_log(LOG_INFO, "skipping interface : %@ (To computers using)", bsdName);
					}
				}
			}
		}
	}
	CFRelease(nat_prefs);
#endif	// TARGET_OS_OSX

	// thin preferences.plist
	n = (services != NULL) ? CFArrayGetCount(services) : 0;
	if (n > 0) {
		updated = 0;

		for (CFIndex i = 0; i < n; i++) {
			SCNetworkInterfaceRef	interface;
			SCNetworkServiceRef	service;

			service = CFArrayGetValueAtIndex(services, i);
			interface = SCNetworkServiceGetInterface(service);
			bsdName = SCNetworkInterfaceGetBSDName(interface);
			if (bsdName == NULL) {
				// if no interface name
				continue;
			}

			if (!isThin(interfaces_thin, bsdName)) {
				// if not thinned
				continue;
			}

			// remove this service associated with a "thinned" interface
			if (logDetails || _sc_verbose) {
				SC_log(LOG_INFO,
				       "thinned network service : %@ : %@ : %@",
				       SCNetworkServiceGetServiceID(service),
				       SCNetworkServiceGetName(service),
				       bsdName);
			}
			SCNetworkServiceRemove(service);
			updated++;
		}

		if (updated > 0) {
			if (logDetails) {
				SC_log(LOG_NOTICE,
				       "Updating \"preferences.plist\" (thinned %d service%s)",
				       updated,
				       (updated != 1) ? "s" : "");
			}
			changed = TRUE;
		}
	}

	// thin NetworkInterfaces.plist
	interfaces = SCPreferencesGetValue(ni_prefs, INTERFACES);
	interfaces = isA_CFArray(interfaces);
	n = (interfaces != NULL) ? CFArrayGetCount(interfaces) : 0;
	if (n > 0) {
		CFMutableArrayRef	interfaces_new;

		updated = 0;

		interfaces_new = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		n = CFArrayGetCount(interfaces);
		for (CFIndex i = 0; i < n; i++) {
			CFDictionaryRef	if_dict;

			if_dict = CFArrayGetValueAtIndex(interfaces, i);
			bsdName = CFDictionaryGetValue(if_dict, CFSTR(kIOBSDNameKey));
			if (isThin(interfaces_thin, bsdName)) {
				if (CFDictionaryContainsKey(if_dict, CFSTR(kSCNetworkInterfaceActive))) {
					if (logDetails) {
						SC_log(LOG_INFO, "skipping interface : %@ (active)", bsdName);
					}
				} else {
					// remove this "thinned" interface
					if (logDetails || _sc_verbose) {
						SC_log(LOG_INFO, "thinned network interface : %@", bsdName);
					}
					updated++;
					continue;
  				}
			}

			CFArrayAppendValue(interfaces_new, if_dict);
		}
		SCPreferencesSetValue(ni_prefs, INTERFACES, interfaces_new);
		CFRelease(interfaces_new);

		if (updated > 0) {
			if (logDetails) {
				SC_log(LOG_INFO,
				       "Updating \"NetworkInterfaces.plist\" (thinned %d interface%s)",
				       updated,
				       (updated != 1) ? "s" : "");
			}
			changed = TRUE;
		}
	}

	if (services != NULL) CFRelease(services);
	CFRelease(interfaces_thin);
	return changed;
}


#pragma mark -
#pragma mark Remove [SCNetworkMigration] Inline Backups


static void
thinInlineBackup(const void *value, void *context)
{
	CFStringRef		backup		= (CFStringRef)value;
	char			*backup_str;
	SCPreferencesRef	prefs		= (SCPreferencesRef)context;
	CFStringRef		save_prefix;
	CFStringRef		save_prefsID	= NULL;

	SC_log(LOG_NOTICE, "thinning [inline] backup: %@", backup);

	save_prefix = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@ : "), backup);
	backup_str = _SC_cfstring_to_cstring(backup, NULL, 0, kCFStringEncodingASCII);
	if (backup_str != NULL) {
		struct tm	save_tm	= { 0 };

		if (strptime(backup_str, "%Y-%m-%d %H:%M:%S", &save_tm) != NULL) {
			save_prefsID = CFStringCreateWithFormat(NULL,
								NULL,
								CFSTR("preferences-%4d-%02d-%02d-%02d%02d%02d.plist"),
								save_tm.tm_year + 1900,
								save_tm.tm_mon + 1,
								save_tm.tm_mday,
								save_tm.tm_hour,
								save_tm.tm_min,
								save_tm.tm_sec);
		}
		CFAllocatorDeallocate(NULL, backup_str);
	}
	if (save_prefsID == NULL) {
		save_prefsID = CFStringCreateWithFormat(NULL, NULL, CFSTR("preferences-%@.plist"), backup);
	}
	savePreferences(prefs, save_prefsID, save_prefix, TRUE, NULL, NULL);
	CFRelease(save_prefsID);
	CFRelease(save_prefix);
	return;
}


static Boolean
__SCNetworkConfigurationCleanInlineBackups(SCPreferencesRef prefs)
{
	CFMutableSetRef	backups	= NULL;
	Boolean		cleaned	= FALSE;
	CFArrayRef	keys;
	CFIndex		n;
	CFStringRef	suffix;

	keys = SCPreferencesCopyKeyList(prefs);
	if (keys == NULL) {
		return FALSE;
	}

	suffix = CFStringCreateWithFormat(NULL, NULL, CFSTR(" : %@"), kSCPrefSets);
	n = CFArrayGetCount(keys);
	for (CFIndex i = 0; i < n; i++) {
		CFStringRef		key	= CFArrayGetValueAtIndex(keys, i);
		CFMutableStringRef	str;

		if (CFStringHasSuffix(key, suffix)) {
			// if "<backup-date> : Sets"
			if (backups == NULL) {
				backups = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
			}
			str = CFStringCreateMutableCopy(NULL, 0, key);
			CFStringTrim(str, suffix);
			CFSetAddValue(backups, str);
			CFRelease(str);
			continue;
		}
	}
	CFRelease(suffix);
	CFRelease(keys);

	if (backups != NULL) {
		CFSetApplyFunction(backups, thinInlineBackup, (void *)prefs);
		CFRelease(backups);
		cleaned = TRUE;
	}

	return cleaned;
}


#pragma mark -
#pragma mark Remove [new device type] Inline Backups


static void
thinInlineModel(const void *value, void *context)
{
	CFStringRef		model	= (CFStringRef)value;
	SCPreferencesRef	prefs	= (SCPreferencesRef)context;
	CFStringRef		save_prefix;
	CFStringRef		save_prefsID;

	SC_log(LOG_NOTICE, "thinning [per-model] backup: %@", model);

	save_prefix = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@:"), model);
	save_prefsID = CFStringCreateWithFormat(NULL, NULL, CFSTR("preferences-%@.plist"), model);
	savePreferences(prefs, save_prefsID, save_prefix, TRUE, MODEL, model);
	CFRelease(save_prefsID);
	CFRelease(save_prefix);
	return;
}


static Boolean
__SCNetworkConfigurationCleanInlineModels(SCPreferencesRef prefs)
{
	Boolean		cleaned	= FALSE;
	CFArrayRef	keys;
	CFMutableSetRef	models	= NULL;
	CFIndex		n;
	CFStringRef	suffix;

	keys = SCPreferencesCopyKeyList(prefs);
	if (keys == NULL) {
		return FALSE;
	}

	suffix = CFStringCreateWithFormat(NULL, NULL, CFSTR(":%@"), kSCPrefSets);
	n = CFArrayGetCount(keys);
	for (CFIndex i = 0; i < n; i++) {
		CFStringRef		key	= CFArrayGetValueAtIndex(keys, i);
		CFMutableStringRef	str;

		if (CFStringHasSuffix(key, suffix)) {
			// if "<backup-date> : Sets"
			if (models == NULL) {
				models = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
			}
			str = CFStringCreateMutableCopy(NULL, 0, key);
			CFStringTrim(str, suffix);
			CFSetAddValue(models, str);
			CFRelease(str);
			continue;
		}
	}
	CFRelease(suffix);
	CFRelease(keys);

	if (models != NULL) {
		CFSetApplyFunction(models, thinInlineModel, (void *)prefs);
		CFRelease(models);
		cleaned = TRUE;
	}

	return cleaned;
}


#pragma mark -
#pragma mark Remove Orphaned Services


/*
static Boolean
__SCNetworkConfigurationCleanOrphanedServices(SCPreferencesRef prefs)
{
#pragma unused(prefs)
	return FALSE;
}
*/


#pragma mark -
#pragma mark Cleanup network service order issues


static Boolean
__SCNetworkConfigurationCleanServiceOrderIssues(SCPreferencesRef prefs)
{
#pragma unused(prefs)
	Boolean		cleaned	= FALSE;
	CFIndex		nSets;
	CFArrayRef	sets;

	sets = SCNetworkSetCopyAll(prefs);
	nSets = (sets != NULL) ? CFArrayGetCount(sets) : 0;
	for (CFIndex iSets = 0; iSets < nSets; iSets++) {
		CFIndex			iServices;
		CFMutableSetRef		known		= NULL;
		CFIndex			nServices;
		SCNetworkSetRef		set		= CFArrayGetValueAtIndex(sets, iSets);
		CFStringRef		setID		= SCNetworkSetGetSetID(set);
		CFArrayRef		order		= SCNetworkSetGetServiceOrder(set);
		CFMutableArrayRef	newOrder	= NULL;

		iServices = 0;
		nServices = (order != NULL) ? CFArrayGetCount(order) : 0;
		if (nServices > 0) {
			known = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
			newOrder = CFArrayCreateMutableCopy(NULL, 0, order);
		}

		while (iServices < nServices) {
			SCNetworkServiceRef	service;
			CFStringRef		serviceID	= CFArrayGetValueAtIndex(newOrder, iServices);

			// check if serviceID already known/processed
			if (CFSetContainsValue(known, serviceID)) {
				// if duplicate/removed service, remove from serviceOrder
				if (logDetails) {
					SC_log(LOG_NOTICE,
					       "set: %@, removing serviceID %@ (duplicate/removed)",
					       setID,
					       serviceID);
				}
				CFArrayRemoveValueAtIndex(newOrder, iServices);
				nServices--;
				cleaned = TRUE;
				continue;
			}

			// track this serviceID as known, already removed, or removed below
			CFSetAddValue(known, serviceID);

			// validate serviceID
			service = SCNetworkServiceCopy(prefs, serviceID);
			if (service == NULL) {
				// if no service, remove from serviceOrder
				if (logDetails) {
					SC_log(LOG_NOTICE,
					       "set: %@, removing serviceID %@ (no service)",
					       setID,
					       serviceID);
				}
				CFArrayRemoveValueAtIndex(newOrder, iServices);
				nServices--;
				cleaned = TRUE;
				continue;
			}

			if (!__SCNetworkServiceExists(service)) {
				// if service already removed, remove from serviceOrder
				if (logDetails) {
					SC_log(LOG_NOTICE,
					       "set: %@, removing serviceID %@ (service already removed)",
					       setID,
					       serviceID);
				}
				CFArrayRemoveValueAtIndex(newOrder, iServices);
				nServices--;
				cleaned = TRUE;
				CFRelease(service);
				continue;
			}

			CFRelease(service);
			iServices++;
		}

		if (known != NULL) {
			CFRelease(known);
		}

		if (newOrder != NULL) {
			if (cleaned) {
				SCNetworkSetSetServiceOrder(set, newOrder);
			}
			CFRelease(newOrder);
		}
	}

	if (sets != NULL) {
		CFRelease(sets);
	}

	return cleaned;
}


#pragma mark -
#pragma mark Cleanup Network Configuration(s)


Boolean
__SCNetworkConfigurationClean(SCPreferencesRef prefs, SCPreferencesRef ni_prefs)
{
	Boolean		changed;
	Boolean		updated	=	FALSE;

	changed = __SCNetworkConfigurationCleanInlineBackups(prefs);
	if (changed) {
		SC_log(LOG_NOTICE, "network configuration: unwanted inline backups removed");
		updated = TRUE;
	}

	changed = __SCNetworkConfigurationCleanInlineModels(prefs);
	if (changed) {
		SC_log(LOG_NOTICE, "network configuration: unwanted device backups removed");
		updated = TRUE;
	}

	if (ni_prefs != NULL) {
		changed = __SCNetworkConfigurationCleanHiddenInterfaces(prefs, ni_prefs);
		if (changed) {
			SC_log(LOG_NOTICE, "network configuration: hidden interface configurations removed");
			updated = TRUE;
		}
	}

	changed = __SCNetworkConfigurationCleanServiceOrderIssues(prefs);
	if (changed) {
		SC_log(LOG_NOTICE, "network configuration: ServiceOrder cleaned");
		updated = TRUE;
	}

	return updated;
}
