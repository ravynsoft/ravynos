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

#ifndef _IOKIT_IO_STORAGE_DEVICE_CHARACTERISTICS_H_
#define _IOKIT_IO_STORAGE_DEVICE_CHARACTERISTICS_H_

#include <IOKit/storage/IOStorageProtocolCharacteristics.h>

/*
 *	Device Characteristics - Characteristics defined for devices.
 */

/*!
@defined kIOPropertyDeviceCharacteristicsKey
@discussion This key is used to define Device Characteristics for a particular
device and it has an associated dictionary which lists the
device characteristics. The device characteristics are Command Set specific
and are listed in the header files for each command set.

Requirement: Mandatory

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>Apple</string>
		<key>Product Name</key>
		<string>iPod</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyDeviceCharacteristicsKey		"Device Characteristics"


/*!
@defined kIOPropertyVendorNameKey
@discussion This key is used to define the Vendor Name for a particular device
and it has an associated string.

Requirement: Mandatory

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>Apple</string>
		<key>Product Name</key>
		<string>iPod</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyVendorNameKey				"Vendor Name"


/*!
@defined kIOPropertyProductNameKey
@discussion This key is used to define the Product Name for a particular device
and it has an associated string.

Requirement: Mandatory

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>Apple</string>
		<key>Product Name</key>
		<string>iPod</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyProductNameKey				"Product Name"


/*!
@defined kIOPropertyProductRevisionLevelKey
@discussion This key is used to define the Product Revision Level for a
particular device and it has an associated string.

Requirement: Mandatory

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>Apple</string>
		<key>Product Name</key>
		<string>iPod</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyProductRevisionLevelKey		"Product Revision Level"


/*!
@defined kIOPropertyProductSerialNumberKey
@discussion This key is used to define the Product Serial Number for a
particular device and it has an associated data.

Requirement: Mandatory

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>Apple</string>
		<key>Product Name</key>
		<string>iPod</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>Serial Number</key>
		<string>123456789</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyProductSerialNumberKey		"Serial Number"


/*!
@defined kIOPropertySupportedCDFeaturesKey
@discussion This key is used to define the supported CD Features for a
particular optical device and it has an associated bitfield. See
<IOKit/scsi/IOSCSIMultimediaCommandsDevice.h> for definitions of the
bits and associated bitmasks.

Requirement: Mandatory for optical devices (Peripheral Device Type 05h).

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>Apple</string>
		<key>Product Name</key>
		<string>SuperDrive</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>CD Features</key>
		<integer>1663</integer>
		<key>DVD Features</key>
		<integer>103</integer>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertySupportedCDFeaturesKey		"CD Features"


/*!
@defined kIOPropertySupportedDVDFeaturesKey
@discussion This key is used to define the supported DVD Features for a
particular optical device and it has an associated bitfield. See
<IOKit/scsi/IOSCSIMultimediaCommandsDevice.h> for definitions of the
bits and associated bitmasks.

Requirement: Mandatory for optical devices (Peripheral Device Type 05h).

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>Apple</string>
		<key>Product Name</key>
		<string>SuperDrive</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>CD Features</key>
		<integer>1663</integer>
		<key>DVD Features</key>
		<integer>103</integer>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertySupportedDVDFeaturesKey		"DVD Features"


/*!
@defined kIOPropertySupportedBDFeaturesKey
@discussion This key is used to define the supported BD Features for a
particular optical device and it has an associated bitfield. See
<IOKit/scsi/IOSCSIMultimediaCommandsDevice.h> for definitions of the
bits and associated bitmasks.

Requirement: Mandatory for optical devices (Peripheral Device Type 05h).

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>Apple</string>
		<key>Product Name</key>
		<string>SuperDrive</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>CD Features</key>
		<integer>1663</integer>
		<key>DVD Features</key>
		<integer>103</integer>
		<key>BD Features</key>
		<integer>21</integer>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertySupportedBDFeaturesKey		"BD Features"


/*!
@defined kIOPropertyRigidDiskGeometryKey
@discussion This key is used to define a dictionary containing
rigid disk geometry information.

Requirement: Optional. If a device publishes this dictionary, it
must publish all key/value pairs which are deemed Mandatory.

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>Apple</string>
		<key>Product Name</key>
		<string>iPod</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>Rigid Disk Geometry</key>
		<dict>
			<key>Sector Count per Track</key>
			<integer>12345</integer>
			<key>Head Count</key>
			<integer>12</integer>
			<key>Cylinder Count</key>
			<integer>12345</integer>
			<key>Bytes per Physical Sector</key>
			<integer>512</integer>
		</dict>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyRigidDiskGeometryKey			"Rigid Disk Geometry"


/*!
@defined kIOPropertySectorCountPerTrackKey
@discussion This key is used to define the number of sectors per
each track for a particular medium.

Requirement: Mandatory element of the Rigid Disk Geometry dictionary.

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>Apple</string>
		<key>Product Name</key>
		<string>iPod</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>Rigid Disk Geometry</key>
		<dict>
			<key>Sector Count per Track</key>
			<integer>12345</integer>
		</dict>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertySectorCountPerTrackKey		"Sector Count per Track"


/*!
@defined kIOPropertyHeadCountKey
@discussion This key is used to define the number of heads for
a particular medium.

Requirement: Mandatory element of the Rigid Disk Geometry dictionary.

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>Apple</string>
		<key>Product Name</key>
		<string>iPod</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>Rigid Disk Geometry</key>
		<dict>
			<key>Sector Count per Track</key>
			<integer>12345</integer>
			<key>Head Count</key>
			<integer>12</integer>
			<key>Cylinder Count</key>
			<integer>12345</integer>
			<key>Bytes per Physical Sector</key>
			<integer>512</integer>
		</dict>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyHeadCountKey					"Head Count"


/*!
@defined kIOPropertyCylinderCountKey
@discussion This key is used to define the number of heads for
a particular medium.

Requirement: Mandatory element of the Rigid Disk Geometry dictionary.

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>Apple</string>
		<key>Product Name</key>
		<string>iPod</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>Rigid Disk Geometry</key>
		<dict>
			<key>Sector Count per Track</key>
			<integer>12345</integer>
			<key>Head Count</key>
			<integer>12</integer>
			<key>Cylinder Count</key>
			<integer>12345</integer>
			<key>Bytes per Physical Sector</key>
			<integer>512</integer>
		</dict>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyCylinderCountKey				"Cylinder Count"


/*!
@defined kIOPropertyBytesPerPhysicalSectorKey
@discussion This key is used to define the number of heads for
a particular medium.

Requirement: Mandatory element of the Rigid Disk Geometry dictionary.

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>Apple</string>
		<key>Product Name</key>
		<string>iPod</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>Rigid Disk Geometry</key>
		<dict>
			<key>Sector Count per Track</key>
			<integer>12345</integer>
			<key>Head Count</key>
			<integer>12</integer>
			<key>Cylinder Count</key>
			<integer>12345</integer>
			<key>Bytes per Physical Sector</key>
			<integer>512</integer>
		</dict>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyBytesPerPhysicalSectorKey	"Bytes per Physical Sector"


/*!
@defined kIOPropertyPhysicalBlockSizeKey
@discussion This key is used to define the physical block size of a hard disk drive.

Requirement: Mandatory for hard disk drives with physical block size other than 512 bytes.

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>Apple</string>
		<key>Product Name</key>
		<string>iPod</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>Physical Block Size</key>
		<integer>4096</integer>
		<key>Logical Block Size</key>
		<integer>512</integer>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyPhysicalBlockSizeKey		"Physical Block Size"


/*!
@defined kIOPropertyLogicalBlockSizeKey
@discussion This key is used to define the logical block size of a hard disk drive.

Requirement: Mandatory for hard disk drives with logical block size other than 512 bytes
or that does not match its physical block size.

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>Apple</string>
		<key>Product Name</key>
		<string>iPod</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>Physical Block Size</key>
		<integer>4096</integer>
		<key>Logical Block Size</key>
		<integer>512</integer>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyLogicalBlockSizeKey		"Logical Block Size"


/*!
@defined kIOPropertyTargetDiskModeKey
@discussion This key is used to indicate the device is another computer in Target Disk Mode.

Requirement: Optional.

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>AAPL</string>
		<key>Product Name</key>
		<string>FireWire Target</string>
		<key>Product Revision Level</key>
		<string>0000</string>
		<key>Target Disk Mode</key>
		<true/>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyTargetDiskModeKey		"Target Disk Mode"


/*!
@defined kIOPropertyInvalidStartupDiskKey
@discussion This key is used to denote devices when cannot be used as a startup disk.

Requirement: Optional.

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>APPLE</string>
		<key>Product Name</key>
		<string>Target Disk Mode</string>
		<key>Product Revision Level</key>
		<string>0000</string>
		<key>Invalid Startup Disk</key>
		<true/>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyInvalidStartupDiskKey        "Invalid Startup Disk"


/*!
@defined kIOPropertyMediumTypeKey
@discussion This key is used to indicate the medium type of the device.

Requirement: Optional.

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>AAPL</string>
		<key>Product Name</key>
		<string>FireWire Target</string>
		<key>Product Revision Level</key>
		<string>0000</string>
		<key>Medium Type</key>
		<string>Rotational</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyMediumTypeKey		"Medium Type"


/*!
@defined kIOPropertyMediumTypeRotationalKey
@discussion This key is used to indicate the medium type of the device is rotational.

Requirement: Optional.

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>AAPL</string>
		<key>Product Name</key>
		<string>FireWire Target</string>
		<key>Product Revision Level</key>
		<string>0000</string>
		<key>Medium Type</key>
		<string>Rotational</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyMediumTypeRotationalKey		"Rotational"


/*!
@defined kIOPropertyMediumTypeSolidStateKey
@discussion This key is used to indicate the medium type of the device is solid state.

Requirement: Optional.

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>AAPL</string>
		<key>Product Name</key>
		<string>FireWire Target</string>
		<key>Product Revision Level</key>
		<string>0000</string>
		<key>Medium Type</key>
		<string>Solid State</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyMediumTypeSolidStateKey		"Solid State"


/*!
@defined kIOPropertyMediumRotationRateKey
@discussion This key is used to indicate the medium rotation rate in RPM of the device.

Requirement: Optional.

Example:
<pre>
@textblock
<dict>
	<key>Device Characteristics</key>
	<dict>
		<key>Vendor Name</key>
		<string>AAPL</string>
		<key>Product Name</key>
		<string>FireWire Target</string>
		<key>Product Revision Level</key>
		<string>0000</string>
		<key>Rotation Rate</key>
		<integer>7200</integer>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyMediumRotationRateKey		"Rotation Rate"


#endif	/* _IOKIT_IO_STORAGE_DEVICE_CHARACTERISTICS_H_ */
