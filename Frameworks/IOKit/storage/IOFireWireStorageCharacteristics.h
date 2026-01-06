/*
 * Copyright (c) 1998-2009 Apple Inc. All rights reserved.
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


#ifndef _IOKIT_IO_FIREWIRE_STORAGE_DEVICE_CHARACTERISTICS_H_
#define _IOKIT_IO_FIREWIRE_STORAGE_DEVICE_CHARACTERISTICS_H_

//
//	Bridge Characteristics - Characteristics defined for FireWire bridges.
//

/*!
@defined kIOPropertyBridgeCharacteristicsKey
@discussion This key is used to define Bridge Characteristics for a particular
devices's bridge chipset. It has an associated dictionary which lists the
bridge characteristics.

Requirement: Optional

Example:
<pre>
@textblock
<dict>
	<key>Bridge Characteristics</key>
	<dict>
		<key>Bridge Vendor Name</key>
		<string>Oxford Semiconductor</string>
		<key>Bridge Model Name</key>
		<string>FW911</string>
		<key>Bridge Revision Level</key>
		<string>3.7</string>
	</dict>
</dict>
@/textblock
</pre>
*/

#define kIOPropertyBridgeCharacteristicsKey		"Bridge Characteristics"
#define kIOPropertyBridgeVendorNameKey			"Bridge Vendor Name"
#define kIOPropertyBridgeModelNameKey			"Bridge Model Name"
#define kIOPropertyBridgeRevisionLevelKey		"Bridge Revision Level"

#endif	/* _IOKIT_IO_FIREWIRE_STORAGE_DEVICE_CHARACTERISTICS_H_ */