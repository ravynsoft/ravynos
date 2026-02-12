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

/*!
 * @header DirServicesPriv
 */

#ifndef __DirServicesPriv_h__
#define	__DirServicesPriv_h__	1

#include <DirectoryService/DirServicesTypes.h>
#include <AvailabilityMacros.h>

/*!
 * @defined kDSStdNotifyxxxxx
 * @discussion notification tags that can be registered for with SystemConfiguration
 * to receive notifications of events occuring in the DirectoryService daemon on the
 * local machine. Not placed in public headers since might deprecate in favor of BSD
 * daemon notification mechanism at later date.
 */
#define		kDSStdNotifyTypePrefix					"com.apple.DirectoryService.NotifyTypeStandard:"
#define		kDSStdNotifySearchPolicyChanged			"com.apple.DirectoryService.NotifyTypeStandard:SearchPolicyChanged"
#define		kDSStdNotifyDirectoryNodeAdded			"com.apple.DirectoryService.NotifyTypeStandard:DirectoryNodeAdded"
#define		kDSStdNotifyDirectoryNodeDeleted		"com.apple.DirectoryService.NotifyTypeStandard:DirectoryNodeDeleted"
#define		kDSStdNotifyDHCPOptionsAvailable		"com.apple.DirectoryService.NotifyTypeStandard:DHCPOptionsAvailable"
#define		kDSStdNotifyDHCPConfigStateChanged		"com.apple.DirectoryService.NotifyTypeStandard:DHCPConfigStateChanged"
#define		kDSStdNotifyContactSearchPolicyChanged	"com.apple.DirectoryService.NotifyTypeStandard:ContactSearchPolicyChanged"

#endif
