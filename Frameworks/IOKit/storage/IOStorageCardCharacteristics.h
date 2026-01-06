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

#ifndef _IOKIT_IO_STORAGE_CARD_CHARACTERISTICS_H_
#define _IOKIT_IO_STORAGE_CARD_CHARACTERISTICS_H_

		
/*
 *	Card Characteristics - Characteristics defined for cards.
 */

/*!
@defined kIOPropertyCardCharacteristicsKey
@discussion This key is used to define Card Characteristics for a particular
piece of MMC/SD media and it has an associated dictionary which lists the
card characteristics.
 
Requirement: Mandatory

Example:
<pre>
@textblock
<dict>
	<key>Card Characteristics</key>
	<dict>
		<key>Product Name</key>
		<string>SD32G</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyCardCharacteristicsKey				"Card Characteristics"


/*!
@defined kIOPropertySlotKey
@discussion This key is used to define the slot number for the device
 
Requirement: Mandatory

Example:
<pre>
@textblock
<dict>
	<key>Slot</key>
	<integer>1<integer>
 </dict>
@/textblock
</pre>
*/
#define kIOPropertySlotKey								"Slot"


/*!
@defined kIOProperty64BitKey
@discussion This key defines wether the device supports 64-bit.
 
Requirement: Mandatory

Example:
<pre>
@textblock
<dict>
	<key>64-bit</key>
	<true/>
</dict>
@/textblock
</pre>
*/
#define kIOProperty64BitKey								"64-bit"


/*!
@defined kIOPropertyClockDivisorKey
 @discussion This key defines the current clock divisor for the device.

Requirement: Mandatory.

Example:
<pre>
@textblock
<dict>
	<key>Clock Divisor</key>
	<integer>128</integer>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyClockDivisorKey						"Clock Divisor"


/*!
@defined kIOPropertyBaseFrequencyKey
@discussion This key defines the current base frequency for the device.
 
Requirement: Mandatory.

Example:
<pre>
@textblock
<dict>
	<key>Base Frequency</key>
	<integer>50</integer>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyBaseFrequencyKey						"Base Frequency"


/*!
@defined kIOPropertyBusVoltageKey
@discussion This key defines the current bus voltage for the device in mV
 
Requirement: Mandatory.

Example:
<pre>
@textblock
<dict>
	<key>Bus Voltage</key>
	<integer>3300</integer>
</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyBusVoltageKey						"Bus Voltage"


/*!
@defined kIOPropertyBusWidthKey
@discussion This key defines the current bus width for the device.

Requirement: Mandatory.
 
Example:
<pre>
@textblock
<dict>
	<key>Bus Width</key>
	<integer>4</integer>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyBusWidthKey							"Bus Width"


/*!
@defined kIOPropertyCardPresentKey
@discussion This key defines wether a MMC or SD card is physically present.
 
Requirement: Mandatory

Example:
<pre>
@textblock
<dict>
	<key>Card Present</key>
	<true/>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyCardPresentKey						"Card Present"


/*!
 @defined kIOPropertyProductSerialNumberKey
 @discussion This key is used to indicate the card serial number ID.
 
 Requirement: Mandatory
 
 Example:
 <pre>
 @textblock
 <dict>
	 <key>Card Characteristics</key>
	 <dict>
		 <key>Product Name</key>
		 <string>SD32G</string>
		 <key>Product Revision Level</key>
		 <string>1.0</string>
		 <key>Card Type</key>
		 <string>SDHC</string>
		 <key>Serial Number</key>
		 <data>0045ff</data>
	 </dict>
 </dict>
 @/textblock
 </pre>
 */
#define kIOPropertyProductSerialNumberKey				"Serial Number"


/*!
 @defined kIOPropertyManufacturerIDKey
 @discussion This key is used to indicate the card manufacturer ID.
 
 Requirement: Optional
 
 Example:
 <pre>
 @textblock
 <dict>
	 <key>Card Characteristics</key>
	 <dict>
		 <key>Product Name</key>
		 <string>SD32G</string>
		 <key>Product Revision Level</key>
		 <string>1.0</string>
		<key>Card Type</key>
		<string>SDHC</string>
		<key>Manufacturer ID</key>
		<data>03</data>
	 </dict>
 </dict>
 @/textblock
 </pre>
 */
#define kIOPropertyManufacturerIDKey					"Manufacturer ID"


/*!
@defined kIOPropertyApplicationIDKey
 @discussion This key is used to indicate the card application ID.

Requirement: Optional

Example:
<pre>
@textblock
<dict>
	<key>Card Characteristics</key>
	<dict>
		<key>Product Name</key>
		<string>SD32G</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>Card Type</key>
		<string>SDHC</string>
		<key>Application ID</key>
		<data>ffff</data>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyApplicationIDKey						"Application ID"


/*!
@defined kIOPropertyManufacturingDateKey
 @discussion This key is used to indicate the card manufacturing date.

Requirement: Mandatory.
 
Example:
<pre>
@textblock
<dict>
	<key>Card Characteristics</key>
	<dict>
		<key>Product Name</key>
		<string>SD32G</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>Card Type</key>
		<string>SDHC</string>
		<key>Manufacturing Date</key>
		<string>2009-12</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyManufacturingDateKey					"Manufacturing Date"


/*!
@defined kIOPropertySpeedClassKey
 @discussion This key is used to indicate SD card speed class.

Requirement: Mandatory.
 
Example:
<pre>
@textblock
<dict>
	<key>Card Characteristics</key>
	<dict>
		<key>Product Name</key>
		<string>SD32G</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>Card Type</key>
		<string>SDHC</string>
		<key>Speed Class</key>
		<data>02</data>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertySpeedClassKey						"Speed Class"


/*!
@defined kIOPropertySpecificationVersionKey
@discussion This key is used to indicate the card specification version.

Requirement: Mandatory.

Example:
<pre>
@textblock
<dict>
	<key>Card Characteristics</key>
	<dict>
		<key>Product Name</key>
		<string>SD32G</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>Card Type</key>
		<string>SDHC</string>
		<key>Specification Version</key>
		<string>3.0</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertySpecificationVersionKey				"Specification Version"


/*!
@defined kIOPropertyCardTypeKey
 @discussion This key is used to indicate the card type is MMC.

Requirement: Optional.

Example:
<pre>
@textblock
<dict>
	<key>Card Characteristics</key>
	<dict>
		<key>Product Name</key>
		<string>SD32G</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>Card Type</key>
		<string>MMC</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyCardTypeKey							"Card Type"


/*!
@defined kIOPropertyCardTypeMMCKey
 @discussion This key is used to indicate the card type is MMC.

 Requirement: Optional.

Example:
<pre>
@textblock
<dict>
	<key>Card Characteristics</key>
	<dict>
		 <key>Product Name</key>
		 <string>SD32G</string>
		 <key>Product Revision Level</key>
		 <string>1.0</string>
		 <key>Card Type</key>
		 <string>MMC</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyCardTypeMMCKey						"MMC"


/*!
@defined kIOPropertyCardTypeSDSCKey
 @discussion This key is used to indicate the card type is SDSC.

Requirement: Optional.

Example:
<pre>
@textblock
<dict>
	<key>Card Characteristics</key>
	<dict>
		 <key>Product Name</key>
		 <string>SD32G</string>
		 <key>Product Revision Level</key>
		 <string>1.0</string>
		 <key>Card Type</key>
		 <string>SDSC</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyCardTypeSDSCKey						"SDSC"


/*!
@defined kIOPropertyCardTypeSDHCKey
 @discussion This key is used to indicate the card type is SDHC.

Requirement: Optional.

Example:
<pre>
@textblock
<dict>
	<key>Card Characteristics</key>
	<dict>
		<key>Product Name</key>
		<string>SD32G</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>Card Type</key>
		<string>SDHC</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyCardTypeSDHCKey						"SDHC"


/*!
@defined kIOPropertyCardTypeSDXCKey
 @discussion This key is used to indicate the card type is SDXC.

Requirement: Optional.

Example:
<pre>
@textblock
<dict>
	<key>Card Characteristics</key>
	<dict>
		<key>Product Name</key>
		<string>SD32G</string>
		<key>Product Revision Level</key>
		<string>1.0</string>
		<key>Card Type</key>
		<string>SDXC</string>
	</dict>
</dict>
@/textblock
</pre>
*/
#define kIOPropertyCardTypeSDXCKey						"SDXC"


#endif	/* _IOKIT_IO_STORAGE_CARD_CHARACTERISTICS_H_ */
