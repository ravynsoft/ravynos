/*
 * Copyright (c) 1998-2014 Apple Inc. All rights reserved.
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

#ifndef _IOKIT_IO_STORAGE_PROTOCOL_CHARACTERISTICS_H_
#define _IOKIT_IO_STORAGE_PROTOCOL_CHARACTERISTICS_H_

#include <IOKit/storage/IOStorageControllerCharacteristics.h>

/*
 *	Protocol Characteristics - Characteristics defined for protocols.
 */

/*!
@defined kIOPropertyProtocolCharacteristicsKey
@discussion This key is used to define Protocol Characteristics for a particular
protocol and it has an associated dictionary which lists the
protocol characteristics.

Requirement: Mandatory

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>ATAPI</string>
		<key>Physical Interconnect Location</key>
		<string>Internal</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define	kIOPropertyProtocolCharacteristicsKey		"Protocol Characteristics"


/*!
@defined kIOPropertySCSIInitiatorIdentifierKey
@discussion An identifier that will uniquely identify this SCSI Initiator for the
SCSI Domain.

Requirement: Mandatory for SCSI Parallel Interface, SAS,
and Fibre Channel Interface.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>SCSI Parallel Interface</string>
		<key>Physical Interconnect Location</key>
		<string>Internal</string>
		<key>SCSI Initiator Identifier</key>
		<integer>7</integer>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertySCSIInitiatorIdentifierKey		"SCSI Initiator Identifier"


/*!
@defined kIOPropertySCSIDomainIdentifierKey
@discussion An identifier that will uniquely identify this SCSI Domain for the
Physical Interconnect type. This identifier is only guaranteed to be unique for
any given Physical Interconnect and is not guaranteed to be the same across
restarts or shutdowns.

Requirement: Mandatory for SCSI Parallel Interface and Fibre Channel Interface.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>SCSI Parallel Interface</string>
		<key>Physical Interconnect Location</key>
		<string>Internal</string>
		<key>SCSI Domain Identifier</key>
		<integer>0</integer>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertySCSIDomainIdentifierKey			"SCSI Domain Identifier"


/*!
@defined kIOPropertySCSITargetIdentifierKey
@discussion This is the SCSI Target Identifier for a given SCSI Target Device.

Requirement: Mandatory for SCSI Parallel Interface and Fibre Channel Interface.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>SCSI Parallel Interface</string>
		<key>Physical Interconnect Location</key>
		<string>Internal</string>
		<key>SCSI Target Identifier</key>
		<integer>3</integer>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertySCSITargetIdentifierKey			"SCSI Target Identifier"


/*!
@defined kIOPropertySCSILogicalUnitNumberKey
@discussion This key is the SCSI Logical Unit Number for the device server
controlled by the driver.

Requirement: Mandatory for SCSI Parallel Interface, SAS, and Fibre Channel Interface.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>SCSI Parallel Interface</string>
		<key>Physical Interconnect Location</key>
		<string>Internal</string>
		<key>SCSI Logical Unit Number</key>
		<integer>2</integer>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertySCSILogicalUnitNumberKey			"SCSI Logical Unit Number"


/*!
@defined kIOPropertyPhysicalInterconnectTypeKey
@discussion This key is used to define the Physical Interconnect to which
a device is attached.

Requirement: Mandatory.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>SCSI Parallel Interface</string>
		<key>Physical Interconnect Location</key>
		<string>Internal</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPhysicalInterconnectTypeKey		"Physical Interconnect"


/*!
@defined kIOPropertyPhysicalInterconnectLocationKey
@discussion This key is used to define the Physical Interconnect
Location.

Requirement: Mandatory.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>SCSI Parallel Interface</string>
		<key>Physical Interconnect Location</key>
		<string>Internal</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPhysicalInterconnectLocationKey	"Physical Interconnect Location"


/*!
@defined kIOPropertySCSIProtocolMultiInitKey
@discussion This protocol characteristics key is used to inform the system
that the protocol supports having multiple devices that act as initiators.

Requirement: Optional.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>Fibre Channel Interface</string>
		<key>Physical Interconnect Location</key>
		<string>External</string>
		<key>Multiple Initiators</key>
		<string>True</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertySCSIProtocolMultiInitKey			"Multiple Initiators"


/*
 *	Values - Values for the characteristics defined above.
 */


/*!
@defined kIOPropertyInternalKey
@discussion This key defines the value of Internal for the key
kIOPropertyPhysicalInterconnectLocationKey. If the device is
connected to an internal bus, this key should be set.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>ATA</string>
		<key>Physical Interconnect Location</key>
		<string>Internal</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyInternalKey						"Internal"


/*!
@defined kIOPropertyExternalKey
@discussion This key defines the value of External for the key
kIOPropertyPhysicalInterconnectLocationKey. If the device is
connected to an external bus, this key should be set.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>Fibre Channel Interface</string>
		<key>Physical Interconnect Location</key>
		<string>External</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyExternalKey						"External"


/*!
@defined kIOPropertyInternalExternalKey
@discussion This key defines the value of Internal/External for the key
kIOPropertyPhysicalInterconnectLocationKey. If the device is connected
to a bus and it is indeterminate whether it is internal or external,
this key should be set.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>SCSI Parallel Interface</string>
		<key>Physical Interconnect Location</key>
		<string>Internal/External</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyInternalExternalKey				"Internal/External"


/*!
@defined kIOPropertyInterconnectFileKey
@discussion This key defines the value of File for the key
kIOPropertyPhysicalInterconnectLocationKey. If the device is a file
that is being represented as a storage device, this key should be set.

NOTE: This key should only be used when the Physical Interconnect is set to
Virtual Interface.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>Virtual Interface</string>
		<key>Physical Interconnect Location</key>
		<string>File</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyInterconnectFileKey						"File"


/*!
@defined kIOPropertyInterconnectRAMKey
@discussion This key defines the value of RAM for the key
kIOPropertyPhysicalInterconnectLocationKey. If the device is system memory
that is being represented as a storage device, this key should be set.

NOTE: This key should only be used when the Physical Interconnect is set to
Virtual Interface.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>Virtual Interface</string>
		<key>Physical Interconnect Location</key>
		<string>RAM</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyInterconnectRAMKey						"RAM"


/*!
@defined kIOPropertyPhysicalInterconnectTypeATA
@discussion This key defines the value of ATA for the key
kIOPropertyPhysicalInterconnectTypeKey. If the device is connected
to an ATA bus, this key should be set.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>ATA</string>
		<key>Physical Interconnect Location</key>
		<string>Internal</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPhysicalInterconnectTypeATA				"ATA"


/*!
@defined kIOPropertyPhysicalInterconnectTypeSerialATA
@discussion This key defines the value of SATA for the key
kIOPropertyPhysicalInterconnectTypeKey. If the device is connected
to a SATA bus, this key should be set.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>SATA</string>
		<key>Physical Interconnect Location</key>
		<string>Internal</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPhysicalInterconnectTypeSerialATA		"SATA"


/*!
@defined kIOPropertyPhysicalInterconnectTypeSerialAttachedSCSI
@discussion This key defines the value of SAS for the key
kIOPropertyPhysicalInterconnectTypeKey. If the device is connected
to a SAS bus, this key should be set.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>SAS</string>
		<key>Physical Interconnect Location</key>
		<string>External</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPhysicalInterconnectTypeSerialAttachedSCSI	"SAS"


/*!
@defined kIOPropertyPhysicalInterconnectTypeATAPI
@discussion This key defines the value of ATAPI for the key
kIOPropertyPhysicalInterconnectTypeKey. If the device is connected
to an ATA bus and follows the ATAPI command specification, this key
should be set.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>ATAPI</string>
		<key>Physical Interconnect Location</key>
		<string>Internal</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPhysicalInterconnectTypeATAPI			"ATAPI"


/*!
@defined kIOPropertyPhysicalInterconnectTypeUSB
@discussion This key defines the value of USB for the key
kIOPropertyPhysicalInterconnectTypeKey. If the device is connected
to a USB port, this key should be set.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>USB</string>
		<key>Physical Interconnect Location</key>
		<string>External</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPhysicalInterconnectTypeUSB				"USB"


/*!
@defined kIOPropertyPhysicalInterconnectTypeFireWire
@discussion This key defines the value of USB for the key
kIOPropertyPhysicalInterconnectTypeKey. If the device is connected
to a FireWire port, this key should be set.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>FireWire</string>
		<key>Physical Interconnect Location</key>
		<string>External</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPhysicalInterconnectTypeFireWire			"FireWire"


/*!
 @defined kIOPropertyPhysicalInterconnectTypeSecureDigital
 @discussion This key defines the value of Secure Digital for the key
 kIOPropertyPhysicalInterconnectTypeSecureDigital. If the device is
 connected to a Secure Digital port and follows the Secure Digital 
 specification, this key should be set. 
 
 Example:
 <pre>
 @textblock
 <dict>
    <key>Protocol Characteristics</key>
    <dict>
        <key>Physical Interconnect</key>
        <string>Secure Digital</string>
        <key>Physical Interconnect Location</key>
        <string>Internal</string>
    </dict>
 </dict>
 @/textblock
 </pre>
 */
#define kIOPropertyPhysicalInterconnectTypeSecureDigital	"Secure Digital"


/*!
@defined kIOPropertyPhysicalInterconnectTypeSCSIParallel
@discussion This key defines the value of SCSI Parallel Interface for the key
kIOPropertyPhysicalInterconnectTypeKey. If the device is connected
to a SCSI Parallel port, this key should be set.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>SCSI Parallel Interface</string>
		<key>Physical Interconnect Location</key>
		<string>External</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPhysicalInterconnectTypeSCSIParallel		"SCSI Parallel Interface"


/*!
@defined kIOPropertyPhysicalInterconnectTypeFibreChannel
@discussion This key defines the value of Fibre Channel Interface for the key
kIOPropertyPhysicalInterconnectTypeKey. If the device is connected
to a Fibre Channel port, this key should be set.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>Fibre Channel Interface</string>
		<key>Physical Interconnect Location</key>
		<string>External</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPhysicalInterconnectTypeFibreChannel		"Fibre Channel Interface"


/*!
@defined kIOPropertyPhysicalInterconnectTypeVirtual
@discussion This key defines the value of Virtual Interface for the key
kIOPropertyPhysicalInterconnectTypeVirtual. If the device is being made to look
like a storage device, but is not such in actuality, such as a File or RAM, this
key should be set.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>Virtual Interface</string>
		<key>Physical Interconnect Location</key>
		<string>File</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPhysicalInterconnectTypeVirtual		"Virtual Interface"


/*!
@defined kIOPropertyPhysicalInterconnectTypePCI
@discussion This key defines the value of PCI for the key
kIOPropertyPhysicalInterconnectTypePCI. If the device is connected
via PCI, this key should be set.

Example:
<pre>
@textblock
<dict>
	<key>Protocol Characteristics</key>
	<dict>
		<key>Physical Interconnect</key>
		<string>PCI</string>
		<key>Physical Interconnect Location</key>
		<string>Internal</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPhysicalInterconnectTypePCI		"PCI"


#endif	/* _IOKIT_IO_STORAGE_PROTOCOL_CHARACTERISTICS_H_ */
