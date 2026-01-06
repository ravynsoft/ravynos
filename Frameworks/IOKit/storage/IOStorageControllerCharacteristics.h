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

#ifndef _IOKIT_IO_STORAGE_CONTROLLER_CHARACTERISTICS_H_
#define _IOKIT_IO_STORAGE_CONTROLLER_CHARACTERISTICS_H_


/*
 *	Controller Characteristics - Characteristics defined for controllers.
 */


/*!
@defined kIOPropertyControllerCharacteristicsKey
@discussion This key is used to define Controller Characteristics for a particular
device and it has an associated dictionary which lists the
controller characteristics.

Requirement: Mandatory

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>Automatic</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyControllerCharacteristicsKey		"Controller Characteristics"


/*!
@defined kIOPropertySASAddressKey
@discussion This key is the unique 64-bit SAS Address for the device server
node located at this port, or for the initiating host port.

Requirement: Mandatory for SAS.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>SAS Address</key>
		<data>0011223344556677</data>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertySASAddressKey					"SAS Address"


/*!
@defined kIOPropertySCSIPortIdentifierKey
@discussion This key is the unique port identifier for the device server
node located at this port, or for the initiating host port.  The format for
this data is allowed to be vendor-specific, as long as it is guaranteed to
be unique.  Length is arbitrary, to allow for itnerfaces with non-standard
identifier rules.  It is recommended to have this be a copy of an existing
standard unique identifier for this port, should one  already exist for your
interface type

Requirement: Mandatory.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Unique SCSI Port Identifier</key>
		<data>0011223344556677</data>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertySCSIPortIdentifierKey		"Unique SCSI Port Identifier"


/*!
@defined kIOPropertyFibreChannelNodeWorldWideNameKey
@discussion This key is the unique 64-bit World Wide Name for the device server
node located at this port, or for the initiating host port.

Requirement: Mandatory for Fibre Channel Interface.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Node World Wide Name</key>
		<data>0011223344556677</data>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyFibreChannelNodeWorldWideNameKey		"Node World Wide Name"


/*!
@defined kIOPropertyFibreChannelPortWorldWideNameKey
@discussion This key is the unique 64-bit World Wide Name for the port.

Requirement: Mandatory for Fibre Channel Interface.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port World Wide Name</key>
		<data>0011223344556677</data>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyFibreChannelPortWorldWideNameKey		"Port World Wide Name"


/*!
@defined kIOPropertyFibreChannelAddressIdentifierKey
@discussion This key is the 24-bit Address Identifier (S_ID or D_ID) as
defined in the FC-FS specification. It contains the address identifier
of the source or destination Nx_Port.

Note: This value can change. It is not a static value.

Requirement: Optional (only necessary for Fibre Channel Interface).

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Address Identifier</key>
		<data>001122</data>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyFibreChannelAddressIdentifierKey		"Address Identifier"


/*!
@defined kIOPropertyFibreChannelALPAKey
@discussion This key is the 8-bit Arbitrated Loop Physical Address
(AL_PA) value as defined in the FC-AL-2 specification.

Note: This value can change. It is not a static value.

Requirement: Optional (only necessary for Fibre Channel Interface).

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>AL_PA</key>
		<data>04</data>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyFibreChannelALPAKey					"AL_PA"


/*!
@defined kIOPropertyPortStatusKey
@discussion This key is associated with the current port
status of the physical link. The port status is either
"Link Established", "No Link Established", or "Link Failed".

Note: This value can change when the port status changes. It
is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Status</key>
		<string>Link Established</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortStatusKey						"Port Status"


/*!
@defined kIOPropertyPortSpeedKey
@discussion This key is associated with the current port
speed. The port speed can be any valid speed for the interconnect.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>Automatic (1 Gigabit)</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeedKey							"Port Speed"


/*!
@defined kIOPropertyPortTopologyKey
@discussion This key is associated with the current port
topology. The port topology can be any valid topology for the interconnect.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Topology</key>
		<string>Automatic (N_Port)</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortTopologyKey						"Port Topology"


/*!
@defined kIOPropertyPortDescriptionKey
@discussion This key is associated with an human
readable port description. Examples include
"Channel A", "Port 1", etc.

Requirement: Optional for all interconnects.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Description</key>
		<string>Channel A</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortDescriptionKey					"Port Description"


/*!
@defined kIOPropertySCSIParallelSignalingTypeKey
@discussion This key is associated with the signaling type
used for this SCSI Parallel bus. Valid values include
"High Voltage Differential", "Low Voltage Differential",
and "Single Ended".

Requirement: Optional for SCSI Parallel Interface. Not
defined for any other physical interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>SCSI Parallel Signaling Type</key>
		<string>High Voltage Differential</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertySCSIParallelSignalingTypeKey			"SCSI Parallel Signaling Type"


/*!
@defined kIOPropertyFibreChannelCableDescriptionKey
@discussion This key is associated with the cabling type
used for this Fibre Channel port. Valid values include
"Copper" and "Fiber Optic".

Requirement: Optional for Fibre Channel Interface. Not
defined for any other physical interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Fibre Channel Cabling Type</key>
		<string>Copper</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyFibreChannelCableDescriptionKey		"Fibre Channel Cabling Type"


/*!
@defined kIOPropertyEncryptionTypeKey
@discussion This key is associated with the encryption type
used for this storage controller. Valid values currently include
"AES-XTS", "AES-XEX" and "AES-CBC".

Requirement: Optional for all storage controllers.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Encryption Type</key>
		<string>AES-XTS</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyEncryptionTypeKey	"Encryption Type"


/*!
@defined kIOPropertyLowPowerModeKey
@discussion This key is associated with whether the storage
device has low-power mode enabled. If enabled, it will set this 
key to true.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Low Power Mode</key>
		<true/>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyLowPowerModeKey		"Low Power Mode"


/*
 *	Values - Values for the characteristics defined above.
 */


/*!
@defined kIOPropertyPortStatusLinkEstablishedKey
@discussion This key defines the value of Link Established for the key
kIOPropertyPortStatusKey.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Status</key>
		<string>Link Established</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortStatusLinkEstablishedKey			"Link Established"


/*!
@defined kIOPropertyPortStatusNoLinkEstablishedKey
@discussion This key defines the value of No Link Established for the key
kIOPropertyPortStatusKey.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Status</key>
		<string>No Link Established</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortStatusNoLinkEstablishedKey		"No Link Established"


/*!
@defined kIOPropertyPortStatusLinkFailedKey
@discussion This key defines the value of Link Failed for the key
kIOPropertyPortStatusKey.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Status</key>
		<string>Link Failed</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortStatusLinkFailedKey				"Link Failed"


/*!
@defined kIOPropertyPortSpeedAutomaticKey
@discussion This key defines the value of Automatic for the key
kIOPropertyPortSpeedKey. If the speed of the port is automatically
adjusted by the host/device and a definitive speed is not known,
this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>Automatic</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeedAutomaticKey				"Automatic"


/*!
@defined kIOPropertyPortSpeed1GigabitKey
@discussion This key defines the value of 1 Gigabit for the key
kIOPropertyPortSpeedKey. If the speed of the port is 1 Gigabit
per second and is not automatically determined (i.e. the user
configured the port to be exactly this speed),
this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>1 Gigabit</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeed1GigabitKey					"1 Gigabit"

/*!
@defined kIOPropertyPortSpeed1_5GigabitKey
@discussion This key defines the value of 1.5 Gigabit for the key
kIOPropertyPortSpeedKey. If the speed of the port is 1.5 Gigabits
per second and is not automatically determined (i.e. the user
configured the port to be exactly this speed),
this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>1.5 Gigabit</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeed1_5GigabitKey				"1.5 Gigabit"


/*!
@defined kIOPropertyPortSpeed2GigabitKey
@discussion This key defines the value of 2 Gigabit for the key
kIOPropertyPortSpeedKey. If the speed of the port is 2 Gigabits
per second and is not automatically determined (i.e. the user
configured the port to be exactly this speed),
this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>2 Gigabit</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeed2GigabitKey					"2 Gigabit"


/*!
@defined kIOPropertyPortSpeed3GigabitKey
@discussion This key defines the value of 3 Gigabit for the key
kIOPropertyPortSpeedKey. If the speed of the port is 3 Gigabits
per second and is not automatically determined (i.e. the user
configured the port to be exactly this speed),
this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>3 Gigabit</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeed3GigabitKey					"3 Gigabit"


/*!
@defined kIOPropertyPortSpeed4GigabitKey
@discussion This key defines the value of 4 Gigabit for the key
kIOPropertyPortSpeedKey. If the speed of the port is 4 Gigabits
per second and is not automatically determined (i.e. the user
configured the port to be exactly this speed),
this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>4 Gigabit</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeed4GigabitKey					"4 Gigabit"

/*!
@defined kIOPropertyPortSpeed6GigabitKey
@discussion This key defines the value of 6 Gigabit for the key
kIOPropertyPortSpeedKey. If the speed of the port is 6 Gigabits
per second and is not automatically determined (i.e. the user
configured the port to be exactly this speed),
this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>6 Gigabit</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeed6GigabitKey					"6 Gigabit"


/*!
@defined kIOPropertyPortSpeed8GigabitKey
@discussion This key defines the value of 8 Gigabit for the key
kIOPropertyPortSpeedKey. If the speed of the port is 8 Gigabits
per second and is not automatically determined (i.e. the user
configured the port to be exactly this speed),
this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>8 Gigabit</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeed8GigabitKey					"8 Gigabit"


/*!
@defined kIOPropertyPortSpeed10GigabitKey
@discussion This key defines the value of 10 Gigabit for the key
kIOPropertyPortSpeedKey. If the speed of the port is 10 Gigabits
per second and is not automatically determined (i.e. the user
configured the port to be exactly this speed),
this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>10 Gigabit</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeed10GigabitKey				"10 Gigabit"


/*!
@defined kIOPropertyPortSpeed12GigabitKey
@discussion This key defines the value of 12 Gigabit for the key
kIOPropertyPortSpeedKey. If the speed of the port is 12 Gigabits
per second and is not automatically determined (i.e. the user
configured the port to be exactly this speed),
this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>12 Gigabit</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeed12GigabitKey				"12 Gigabit"


/*!
@defined kIOPropertyPortSpeed16GigabitKey
@discussion This key defines the value of 16 Gigabit for the key
kIOPropertyPortSpeedKey. If the speed of the port is 16 Gigabits
per second and is not automatically determined (i.e. the user
configured the port to be exactly this speed),
this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>16 Gigabit</string>
	</dict>
</dict>
@/textblock
</pre>
 */
#define kIOPropertyPortSpeed16GigabitKey				"16 Gigabit"


/*!
@defined kIOPropertyPortSpeed40GigabitKey
@discussion This key defines the value of 40 Gigabit for the key
kIOPropertyPortSpeedKey. If the speed of the port is 40 Gigabits
per second and is not automatically determined (i.e. the user
configured the port to be exactly this speed),
this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>40 Gigabit</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeed40GigabitKey				"40 Gigabit"


/*!
@defined kIOPropertyPortSpeedAutomatic1GigabitKey
@discussion This key defines the value of Automatic (1 Gigabit)
for the key kIOPropertyPortSpeedKey. If the speed of the port is
1 Gigabit per second and is automatically determined by host
software, this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>Automatic (1 Gigabit)</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeedAutomatic1GigabitKey		"Automatic (1 Gigabit)"


/*!
@defined kIOPropertyPortSpeedAutomatic1_5GigabitKey
@discussion This key defines the value of Automatic (1.5 Gigabit) for the key
kIOPropertyPortSpeedKey. If the speed of the port is
1.5 Gigabits per second and is automatically determined by host
software, this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>Automatic (1.5 Gigabit)</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeedAutomatic1_5GigabitKey		"Automatic (1.5 Gigabit)"


/*!
@defined kIOPropertyPortSpeedAutomatic2GigabitKey
@discussion This key defines the value of Automatic (2 Gigabit)
for the key kIOPropertyPortSpeedKey. If the speed of the port is
2 Gigabits per second and is automatically determined by host
software, this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>Automatic (2 Gigabit)</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeedAutomatic2GigabitKey		"Automatic (2 Gigabit)"


/*!
@defined kIOPropertyPortSpeedAutomatic3GigabitKey
@discussion This key defines the value of Automatic (3 Gigabit)
for the key kIOPropertyPortSpeedKey. If the speed of the port is
3 Gigabits per second and is automatically determined by host
software, this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>Automatic (3 Gigabit)</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeedAutomatic3GigabitKey		"Automatic (3 Gigabit)"


/*!
@defined kIOPropertyPortSpeedAutomatic4GigabitKey
@discussion This key defines the value of Automatic (4 Gigabit)
for the key kIOPropertyPortSpeedKey. If the speed of the port is
4 Gigabits per second and is automatically determined by host
software, this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>Automatic (4 Gigabit)</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeedAutomatic4GigabitKey		"Automatic (4 Gigabit)"


/*!
@defined kIOPropertyPortSpeedAutomatic6GigabitKey
@discussion This key defines the value of Automatic (6 Gigabit)
for the key kIOPropertyPortSpeedKey. If the speed of the port is
6 Gigabits per second and is automatically determined by host
software, this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>Automatic (6 Gigabit)</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeedAutomatic6GigabitKey		"Automatic (6 Gigabit)"


/*!
@defined kIOPropertyPortSpeedAutomatic8GigabitKey
@discussion This key defines the value of Automatic (8 Gigabit)
for the key kIOPropertyPortSpeedKey. If the speed of the port is
8 Gigabits per second and is automatically determined by host
software, this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>Automatic (8 Gigabit)</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeedAutomatic8GigabitKey		"Automatic (8 Gigabit)"


/*!
@defined kIOPropertyPortSpeedAutomatic10GigabitKey
@discussion This key defines the value of Automatic (10 Gigabit)
for the key kIOPropertyPortSpeedKey. If the speed of the port is
10 Gigabits per second and is automatically determined by host
software, this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Speed</key>
		<string>Automatic (10 Gigabit)</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortSpeedAutomatic10GigabitKey		"Automatic (10 Gigabit)"


/*!
@defined kIOPropertyPortTopologyAutomaticKey
@discussion This key defines the value of Automatic for the key
kIOPropertyPortTopologyKey. If the topology of the port is automatically
adjusted by the host/device and a definitive topology is not known,
this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Topology</key>
		<string>Automatic</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortTopologyAutomaticKey				"Automatic"


/*!
@defined kIOPropertyPortTopologyNPortKey
@discussion This key defines the value of N_Port for the key
kIOPropertyPortTopologyKey. If the topology of the port is an N_Port,
this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Topology</key>
		<string>N_Port</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortTopologyNPortKey					"N_Port"


/*!
@defined kIOPropertyPortTopologyNLPortKey
@discussion This key defines the value of NL_Port for the key
kIOPropertyPortTopologyKey. If the topology of the port is an NL_Port,
this key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Topology</key>
		<string>NL_Port</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortTopologyNLPortKey				"NL_Port"


/*!
@defined kIOPropertyPortTopologyAutomaticNPortKey
@discussion This key defines the value of Automatic (N_Port) for the key
kIOPropertyPortTopologyKey. If the topology of the port is
N_Port and is automatically determined by host software, this
key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Topology</key>
		<string>Automatic (N_Port)</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortTopologyAutomaticNPortKey		"Automatic (N_Port)"


/*!
@defined kIOPropertyPortTopologyAutomaticNLPortKey
@discussion This key defines the value of Automatic (NL_Port) for the key
kIOPropertyPortTopologyKey. If the topology of the port is
NL_Port and is automatically determined by host software, this
key should be used.

Note: This value can change. It is not a static value.

Requirement: Optional for any interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Port Topology</key>
		<string>Automatic (NL_Port)</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPortTopologyAutomaticNLPortKey		"Automatic (NL_Port)"


/*!
@defined kIOPropertySCSIParallelSignalingTypeHVDKey
@discussion This key defines the value of High Voltage Differential for the key
kIOPropertySCSIParallelSignalingTypeKey. If the signaling type of the port is
High Voltage Differential, this key should be used.

Requirement: Optional for SCSI Parallel Interface interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>SCSI Parallel Signaling Type</key>
		<string>High Voltage Differential</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertySCSIParallelSignalingTypeHVDKey		"High Voltage Differential"


/*!
@defined kIOPropertySCSIParallelSignalingTypeLVDKey
@discussion This key defines the value of Low Voltage Differential for the key
kIOPropertySCSIParallelSignalingTypeKey. If the signaling type of the port is
Low Voltage Differential, this key should be used.

Requirement: Optional for SCSI Parallel Interface interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>SCSI Parallel Signaling Type</key>
		<string>Low Voltage Differential</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertySCSIParallelSignalingTypeLVDKey		"Low Voltage Differential"


/*!
@defined kIOPropertySCSIParallelSignalingTypeSEKey
@discussion This key defines the value of Single Ended for the key
kIOPropertySCSIParallelSignalingTypeKey. If the signaling type of the port is
Single Ended, this key should be used.

Requirement: Optional for SCSI Parallel Interface interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>SCSI Parallel Signaling Type</key>
		<string>Single Ended</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertySCSIParallelSignalingTypeSEKey		"Single Ended"


/*!
@defined kIOPropertyFibreChannelCableDescriptionCopperKey
@discussion This key defines the value of Copper for the key
kIOPropertyFibreChannelCableDescriptionKey. If the cabling is
Copper, this key should be used.

Requirement: Optional for Fibre Channel Interface interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Fibre Channel Cabling Type</key>
		<string>Copper</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyFibreChannelCableDescriptionCopperKey		"Copper"


/*!
@defined kIOPropertyFibreChannelCableDescriptionFiberOpticKey
@discussion This key defines the value of Fiber Optic for the key
kIOPropertyFibreChannelCableDescriptionKey. If the cabling is
Fiber Optic, this key should be used.

Requirement: Optional for Fibre Channel Interface interconnect.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Fibre Channel Cabling Type</key>
		<string>Fiber Optic</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyFibreChannelCableDescriptionFiberOpticKey	"Fiber Optic"


/*!
@defined kIOPropertyAESCBCKey
@discussion This key defines the value of AES-CBC for the key
kIOPropertyEncryptionTypeKey. If the encryption used is
AES-CBC, this key should be used.

Requirement: Optional for all storage controllers.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Encryption Type</key>
		<string>AES-CBC</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyAESCBCKey	"AES-CBC"


/*!
@defined kIOPropertyAESXTSKey
@discussion This key defines the value of AES-XTS for the key
kIOPropertyEncryptionTypeKey. If the encryption used is
AES-XTS, this key should be used.

Requirement: Optional for all storage controllers.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Encryption Type</key>
		<string>AES-XTS</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyAESXTSKey	"AES-XTS"


/*!
@defined kIOPropertyAESXEXKey
@discussion This key defines the value of AES-XEX for the key
kIOPropertyEncryptionTypeKey. If the encryption used is
AES-XEX, this key should be used.

Requirement: Optional for all storage controllers.

Example:
<pre>
@textblock
<dict>
	<key>Controller Characteristics</key>
	<dict>
		<key>Encryption Type</key>
		<string>AES-XEX</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyAESXEXKey	"AES-XEX"


#endif	/* _IOKIT_IO_STORAGE_CONTROLLER_CHARACTERISTICS_H_ */
