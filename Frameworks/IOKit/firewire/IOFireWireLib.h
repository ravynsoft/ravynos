/*
 * Copyright (c) 1998-2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 *  IOFireWireLib.h
 *  IOFireWireLib
 *
 *  Created on Thu Apr 27 2000.
 *  Copyright (c) 2000-2002 Apple Computer, Inc. All rights reserved.
 *
 */

/*! @header IOFireWireLib.h
IOFireWireLib is the software used by user space software to communicate with FireWire
devices and control the FireWire bus. IOFireWireLib is the lowest-level FireWire interface available
in user space.

To communicate with a device on the FireWire bus, an instance of IOFireWireDeviceInterface (a struct
which is defined below) is created. The methods of IOFireWireDeviceInterface allow you
to communicate with the device and create instances of other interfaces which provide extended 
functionality (for example, creation of unit directories on the local machine).

References to interfaces should be kept using the interface reference typedefs defined herein.
For example, you should use IOFireWireLibDeviceRef to refer to instances of IOFireWireDeviceInterface, 
IOFireWireLibCommandRef to refer to instances of IOFireWireCommandInterface, and so on.

To obtain an IOFireWireDeviceInterface for a device on the FireWire bus, use the function 
IOCreatePlugInInterfaceForService() defined in IOKit/IOCFPlugIn.h. (Note the "i" in "PlugIn" is 
always upper-case.) Quick usage reference:<br>
<ul>
	<li>'service' is a reference to the IOKit registry entry of the kernel object 
		(usually of type IOFireWireDevice) representing the device
		of interest. This reference can be obtained using the functions defined in
		IOKit/IOKitLib.h.</li>
	<li>'plugInType' should be CFUUIDGetUUIDBytes(kIOCFPlugInInterfaceID)</li>
	<li>'interfaceType' should be CFUUIDGetUUIDBytes(kIOFireWireLibTypeID) when using IOFireWireLib</li>
</ul>
The interface returned by IOCreatePlugInInterfaceForService() should be deallocated using 
IODestroyPlugInInterface(). Do not call Release() on it.

*/
/*
	$Log: not supported by cvs2svn $
	Revision 1.77  2009/10/29 22:28:28  calderon
	<rdar://7308574> IOFireWireLib.h and IOFireWireLibIsoch.h headerdoc markup patch

	Revision 1.76  2008/12/12 04:43:57  collin
	user space compare swap command fixes
	
	Revision 1.75  2008/09/12 23:44:05  calderon
	<rdar://5971979/> PseudoAddressSpace skips/mangles packets
	<rdar://5708169/> FireWire synchronous commands' headerdoc missing callback info
	
	Revision 1.74  2007/11/09 01:39:04  arulchan
	fix for rdar://5555060
	
	Revision 1.73  2007/10/16 16:50:21  ayanowit
	Removed existing "work-in-progress" support for buffer-fill isoch.
	
	Revision 1.72  2007/06/21 04:08:45  collin
	*** empty log message ***
	
	Revision 1.71  2007/05/12 01:10:45  arulchan
	Asyncstream transmit command interface
	
	Revision 1.70  2007/05/03 01:21:29  arulchan
	Asyncstream transmit APIs
	
	Revision 1.69  2007/04/28 02:54:23  collin
	*** empty log message ***
	
	Revision 1.68  2007/04/28 01:42:35  collin
	*** empty log message ***
	
	Revision 1.67  2007/04/11 03:37:41  collin
	*** empty log message ***
	
	Revision 1.66  2007/04/05 22:32:09  collin
	*** empty log message ***
	
	Revision 1.65  2007/03/23 01:47:17  collin
	*** empty log message ***
	
	Revision 1.64  2007/03/22 00:30:00  collin
	*** empty log message ***
	
	Revision 1.63  2007/03/14 02:29:35  collin
	*** empty log message ***
	
	Revision 1.62  2007/03/06 06:30:05  collin
	*** empty log message ***
	
	Revision 1.61  2007/03/06 04:50:21  collin
	*** empty log message ***
	
	Revision 1.60  2007/03/03 05:52:20  collin
	*** empty log message ***
	
	Revision 1.59  2007/03/03 04:47:15  collin
	*** empty log message ***
	
	Revision 1.58  2007/02/16 19:09:15  arulchan
	*** empty log message ***
	
	Revision 1.57  2007/02/16 17:41:00  ayanowit
	More Leopard changes.
	
	Revision 1.56  2007/02/15 22:02:38  ayanowit
	More fixes for new IRMAllocation stuff.
	
	Revision 1.55  2007/02/14 22:43:34  collin
	*** empty log message ***
	
	Revision 1.54  2007/02/10 02:40:58  collin
	*** empty log message ***
	
	Revision 1.53  2007/02/09 20:36:46  ayanowit
	More Leopard IRMAllocation changes.
	
	Revision 1.52  2007/01/17 23:22:40  collin
	*** empty log message ***
	
	Revision 1.51  2007/01/17 03:46:27  collin
	*** empty log message ***
	
	Revision 1.50  2007/01/11 04:34:18  collin
	*** empty log message ***
	
	Revision 1.49  2007/01/04 04:07:25  collin
	*** empty log message ***
	
	Revision 1.48  2006/12/22 05:15:13  collin
	*** empty log message ***
	
	Revision 1.47  2006/12/22 03:50:40  collin
	*** empty log message ***
	
	Revision 1.46  2006/12/06 00:01:10  arulchan
	Isoch Channel 31 Generic Receiver
	
	Revision 1.45  2006/10/26 00:39:16  calderon
	Changed headerdoc to specify release() need on GetConfigDirectory
	
	Revision 1.44  2006/09/28 23:50:05  collin
	*** empty log message ***
	
	Revision 1.43  2006/09/28 22:47:06  ayanowit
	Another tweak to new APIs.
	
	Revision 1.42  2006/09/28 22:31:31  arulchan
	New Feature rdar::3413505
	
	Revision 1.41  2006/09/27 22:42:12  ayanowit
	Merged in Leopard changes for new IRMAllocation API.
	
	Revision 1.40  2006/09/22 06:45:19  collin
	*** empty log message ***
	
	Revision 1.39  2004/06/10 20:57:37  niels
	*** empty log message ***
	
	Revision 1.38  2004/05/04 22:52:20  niels
	*** empty log message ***
	
	Revision 1.37  2004/04/19 21:51:49  niels
	Revision 1.36  2004/03/25 00:00:24  niels
	fix panic allocating large physical address spaces
	
	Revision 1.35  2004/02/27 21:02:20  calderon
	Changed headerdoc abstract of function "GetSpeedBetweenNodes" from "Get
	maximum transfer speed to device to which this interface is attached." to
	"Get the maximum transfer speed between nodes 'srcNodeID' and 'destNodeID'."
	
	Revision 1.34  2003/11/20 19:14:08  niels
	Revision 1.33  2003/11/07 21:24:28  niels
	Revision 1.32  2003/11/07 21:01:19  niels
	Revision 1.31  2003/09/10 23:01:48  collin
	Revision 1.30  2003/09/06 01:37:24  collin
	Revision 1.29  2003/08/25 08:39:17  niels
	Revision 1.28  2003/08/08 21:03:47  gecko1
	Merge max-rec clipping code into TOT
	
	Revision 1.27  2003/07/21 06:53:10  niels
	merge isoch to TOT
	
	Revision 1.26.14.2  2003/07/18 00:17:47  niels
	Revision 1.26.14.1  2003/07/01 20:54:23  niels
	isoch merge
	
	Revision 1.26  2002/11/06 23:44:21  wgulland
	Update header doc for CreateLocalIsochPort
	
	Revision 1.25  2002/09/25 00:27:33  niels
	flip your world upside-down
	
	Revision 1.24  2002/09/12 22:41:55  niels
	add GetIRMNodeID() to user client
	
*/

#ifndef __IOFireWireLib_H__
#define __IOFireWireLib_H__

#ifndef KERNEL
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/firewire/IOFireWireFamilyCommon.h>

// ============================================================
// plugin loading
// ============================================================

#pragma mark IOFIREWIRELIB TYPE UUID
//	uuid string: CDCFCA94-F197-11D4-87E6-000502072F80
#define kIOFireWireLibTypeID			CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0xCD, 0xCF, 0xCA, 0x94, 0xF1, 0x97, 0x11, 0xD4,\
											0x87, 0xE6, 0x00, 0x05, 0x02, 0x07, 0x2F, 0x80)

#pragma mark -
#pragma mark DEVICE/UNIT/NUB INTERFACE UUIDs
// ============================================================
// device/unit/nub interfaces (newest first)
// ============================================================

//
// version 9  // 10.5 Leopard
//
// kIOFireWireDeviceInterface_v9
//		uuid: EE0A94D7-29B4-4D76-A857-57CA477C73B1
#define kIOFireWireDeviceInterfaceID_v9	CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault,\
											0xEE, 0x0A, 0x94, 0xD7, 0x29, 0xB4, 0x4D, 0x76, \
											0xA8, 0x57, 0x57, 0xCA, 0x47, 0x7C, 0x73, 0xB1 )

//
// version 8
//
// kIOFireWireDeviceInterface_v8
//		uuid: 22A258BB-A859-11D8-AA56-000A95992A78
#define kIOFireWireDeviceInterfaceID_v8	CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault,\
											0x22, 0xA2, 0x58, 0xBB, 0xA8, 0x59, 0x11, 0xD8, \
											0xAA, 0x56, 0x00, 0x0A, 0x95, 0x99, 0x2A, 0x78 )

//
// version 7
//
// kIOFireWireDeviceInterface_v7
//		uuid: 188517DE-10B4-11D8-B5CC-000393CFACEA
#define kIOFireWireDeviceInterfaceID_v7	CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault,\
											0x18, 0x85, 0x17, 0xDE, 0x10, 0xB4, 0x11, 0xD8,\
											0xB5, 0xCC, 0x00, 0x03, 0x93, 0xCF, 0xAC, 0xEA )

//
// version 6 (obsolete)
//

// kIOFireWireDeviceInterface_v6
//		uuid: C2AB2F11-45E2-11D7-815C-000393470256
#define kIOFireWireDeviceInterfaceID_v6	CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault,\
											0xC2, 0xAB, 0x2F, 0x11, 0x45, 0xE2, 0x11, 0xD7,\
											0x81, 0x5C, 0x00, 0x03, 0x93, 0x47, 0x02, 0x56 )

//
// 	version 5 interfaces (obsolete)
//
//
//	kIOFireWireNubInterface_v5
//		uuid string: D4900C5A-C69E-11D6-AEA5-0003938BEB0A
#define kIOFireWireNubInterfaceID_v5	CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault,\
											0xD4, 0x90, 0x0C, 0x5A, 0xC6, 0x9E, 0x11, 0xD6,\
											0xAE, 0xA5, 0x00, 0x03, 0x93, 0x8B, 0xEB, 0x0A )
//	kIOFireWireUnitInterfaceID_v5
//		uuid string: 121D7347-C69F-11D6-9B31-0003938BEB0A
#define kIOFireWireUnitInterfaceID_v5	CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault,\
											0x12, 0x1D, 0x73, 0x47, 0xC6, 0x9F, 0x11, 0xD6,\
											0x9B, 0x31, 0x00, 0x03, 0x93, 0x8B, 0xEB, 0x0A )
//	kIOFireWireDeviceInterfaceID_v5
//		uuid string: 127A12F6-C69F-11D6-9D11-0003938BEB0A
#define kIOFireWireDeviceInterfaceID_v5	CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault,\
											0x12, 0x7A, 0x12, 0xF6, 0xC6, 0x9F, 0x11, 0xD6,\
											0x9D, 0x11, 0x00, 0x03, 0x93, 0x8B, 0xEB, 0x0A )

//
// 	version 4 interfaces (obsolete)
//
//		availability: 
//				Mac OS X 10.2 "Jaguar" and later
//
//	kIOFireWireNubInterface_v4
//		uuid string: 939151B8-6945-11D6-BEC7-0003933F84F0
#define kIOFireWireNubInterfaceID_v4	CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault,\
											0x93, 0x91, 0x51, 0xB8, 0x69, 0x45, 0x11, 0xD6,\
											0xBE, 0xC7, 0x00, 0x03, 0x93, 0x3F, 0x84, 0xF0 )

//	kIOFireWireUnitInterface_v4
//		uuid string: D1A395C9-6945-11D6-9B32-0003933F84F0
#define kIOFireWireUnitInterfaceID_v4	CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault,\
											0xD1, 0xA3, 0x95, 0xC9, 0x69, 0x45, 0x11, 0xD6,\
											0x9B, 0x32, 0x00, 0x03, 0x93, 0x3F, 0x84, 0xF0 )

//	kIOFireWireDeviceInterface_v4
//		uuid string: F4B3748B-6945-11D6-8299-0003933F84F0
#define kIOFireWireDeviceInterfaceID_v4	CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault,\
											0xF4, 0xB3, 0x74, 0x8B, 0x69, 0x45, 0x11, 0xD6,\
											0x82, 0x99, 0x00, 0x03, 0x93, 0x3F, 0x84, 0xF0 )

//
// 	version 3 interfaces  (obsolete)
//
//		availability: 
//				Mac OS X 10.2 "Jaguar" and later
//

//	kIOFireWireNubInterfaceID_v3
//		uuid string: F70FE149-E393-11D5-958A-0003933F84F0
#define kIOFireWireNubInterfaceID_v3	CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault,\
											0xF7, 0x0F, 0xE1, 0x49, 0xE3, 0x93, 0x11, 0xD5,\
											0x95, 0x8A, 0x00, 0x03, 0x93, 0x3F, 0x84, 0xF0 )

//	kIOFireWireUnitInterfaceID_v3
//		uuid string: FE7A02EB-E393-11D5-8A61-0003933F84F0
#define kIOFireWireUnitInterfaceID_v3	CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault,\
											0xFE, 0x7A, 0x02, 0xEB, 0xE3, 0x93, 0x11, 0xD5,\
											0x8A, 0x61, 0x00, 0x03, 0x93, 0x3F, 0x84, 0xF0 )
											
//	kIOFireWireDeviceInterfaceID_v3
//  	uuid string: 00EB71A0-E394-11D5-829A-0003933F84F0
#define kIOFireWireDeviceInterfaceID_v3	CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x00, 0xEB, 0x71, 0xA0, 0xE3, 0x94, 0x11, 0xD5,\
											0x82, 0x9A, 0x00, 0x03, 0x93, 0x3F, 0x84, 0xF0 )

//
// 	version 2 interfaces (obsolete)
//
//		availability: 
//				Mac OS X 10.1 and later
//

//	kIOFireWireNubInterfaceID
//		uuid string: 2575E4C4-B6C1-11D5-8F73-003065AF75CC
#define kIOFireWireNubInterfaceID		CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault,\
											0x25, 0x75, 0xE4, 0xC4, 0xB6, 0xC1, 0x11, 0xD5,\
											0x8F, 0x73, 0x00, 0x30, 0x65, 0xAF, 0x75, 0xCC )

//	kIOFireWireUnitInterfaceID
//		uuid string: A02CC5D4-B6C1-11D5-AEA8-003065AF75CC
#define kIOFireWireUnitInterfaceID		CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault,\
											0xA0, 0x2C, 0xC5, 0xD4, 0xB6, 0xC1, 0x11, 0xD5,\
											0xAE, 0xA8, 0x00, 0x30, 0x65, 0xAF, 0x75, 0xCC )

//	kIOFireWireDeviceInterfaceID_v2
//  	uuid string: B3993EB8-56E2-11D5-8BD0-003065423456
#define kIOFireWireDeviceInterfaceID_v2	CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0xB3, 0x99, 0x3E, 0xB8, 0x56, 0xE2, 0x11, 0xD5,\
											0x8B, 0xD0, 0x00, 0x30, 0x65, 0x42, 0x34, 0x56)

//
//	version 1 interfaces  (obsolete)
//
//		availablity: 
//				Mac OS X 10.0.0 and later
//

//	kIOFireWireDeviceInterfaceID
// 	(obsolete: do not use. may be removed in the future.)
//		uuid string: E3DF4460-F197-11D4-8AC8-000502072F80
#define kIOFireWireDeviceInterfaceID	CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0xE3, 0xDF, 0x44, 0x60, 0xF1, 0x97, 0x11, 0xD4,\
											0x8A, 0xC8, 0x00, 0x05, 0x02, 0x07, 0x2F, 0x80)

#pragma mark -
#pragma mark COMMAND OBJECT UUIDs
// ============================================================
// command objects
// ============================================================

// version 3 interfaces:
//
//	availability:
//		Mac OS X "Leopard" and later
//

//		uuid string : 18B932AA-697A-4C7E-8F22-80EE746773A9
#define kIOFireWireAsyncStreamCommandInterfaceID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault, \
													0x18, 0xB9, 0x32, 0xAA, 0x69, 0x7A, 0x4C, 0x7E, \
													0x8F, 0x22, 0x80, 0xEE, 0x74, 0x67, 0x73, 0xA9 )


//		uuid string : F3FF3AC6-FE88-47A0-ACB7-509009808128
#define kIOFireWirePHYCommandInterfaceID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0xF3, 0xFF, 0x3A, 0xC6, 0xFE, 0x88, 0x47, 0xA0, \
											0xAC, 0xB7, 0x50, 0x90, 0x09, 0x80, 0x81, 0x28 )

//		uuid string : FAF5529D-9F99-42CB-B0E8-67860807F551
#define kIOFireWireVectorCommandInterfaceID	CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0xFA, 0xF5, 0x52, 0x9D, 0x9F, 0x99, 0x42, 0xCB,\
											0xB0, 0xE8, 0x67, 0x86, 0x08, 0x07, 0xF5, 51)
											
//		uuid: 12DE8E37-0BE4-4094-882F-FD0B932A3174
#define kIOFireWireIRMAllocationInterfaceID	CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault,\
											0x12, 0xDE, 0x8E, 0x37, 0x0B, 0xE4, 0x40, 0x94, \
											0x88, 0x2F, 0xFD, 0x0B, 0x93, 0x2A, 0x31, 0x74 )

//	uuid string: 577B1AFE-1A48-4137-8993-71077820E0CD
#define kIOFireWireCommandInterfaceID_v3	CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x57, 0x7B, 0x1A, 0xFE, 0x1A, 0x48, 0x41, 0x37,\
											0x89, 0x93, 0x71, 0x07, 0x78, 0x20, 0xE0, CD )

//  uuid string: 30FB7D2A-FF2E-4236-871B-2A473B0B7B3B
#define kIOFireWireReadCommandInterfaceID_v3 CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x30, 0xFB, 0x7D, 0x2A, 0xFF, 0x2E, 0x42, 0x36,\
											0x87, 0x1B, 0x2A, 0x47, 0x3B, 0x0B, 0x7B, 0x3B )
											
//	uuid string: EF55343D-40A4-4007-BF99-DF1413251309
#define kIOFireWireWriteCommandInterfaceID_v3 CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0xEF, 0x55, 0x34, 0x3D, 0x40, 0xA4, 0x40, 0x07,\
											0xBF, 0x99, 0xDF, 0x14, 0x13, 0x25, 0x13, 0x09 )
											
//	uuid string: 037F5D98-F5F9-4FBF-9267-4B9BFE9642D6
#define kIOFireWireCompareSwapCommandInterfaceID_v3	CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x03, 0x7F, 0x5D, 0x98, 0xF5, 0xF9, 0x4F, 0xBF,\
											0x92, 0x67, 0x4B, 0x9B, 0xFE, 0x96, 0x42, 0xD6 )											
//
//	version 2 interfaces:
//
//		availability: 
//				Mac OS X "Jaguar" and later
//

//	kIOFireWireCompareSwapCommandInterfaceID_v2
//	uuid string: 6100FEC9-6946-11D6-8A49-0003933F84F0
#define kIOFireWireCompareSwapCommandInterfaceID_v2		CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x61, 0x00, 0xFE, 0xC9, 0x69, 0x46, 0x11, 0xD6,\
											0x8A, 0x49, 0x00, 0x03, 0x93, 0x3F, 0x84, 0xF0 )

//
//	version 1 interfaces  (obsolete)
//

//	uuid string: F8B6993A-F197-11D4-A3F1-000502072F80
#define kIOFireWireCommandInterfaceID	CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0xF8, 0xB6, 0x99, 0x3A, 0xF1, 0x97, 0x11, 0xD4,\
											0xA3, 0xF1, 0x00, 0x05, 0x02, 0x07, 0x2F, 0x80)

//  uuid string: AB26F124-76E9-11D5-86D5-003065423456
#define kIOFireWireReadCommandInterfaceID_v2 CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0xAB, 0x26, 0xF1, 0x24, 0x76, 0xE9, 0x11, 0xD5,\
											0x86, 0xD5, 0x00, 0x30, 0x65, 0x42, 0x34, 0x56)

//	uuid string: 1023605C-76EA-11D5-B82A-003065423456
#define kIOFireWireWriteCommandInterfaceID_v2 CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x10, 0x23, 0x60, 0x5C, 0x76, 0xEA, 0x11, 0xD5,\
											0xB8, 0x2A, 0x00, 0x30, 0x65, 0x42, 0x34, 0x56)

//	uuid string: 70C10E38-F64A-11D4-AFE7-0050E4D93B36
#define kIOFireWireCompareSwapCommandInterfaceID	CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x70, 0xC1, 0x0E, 0x38, 0xF6, 0x4A, 0x11, 0xD4,\
											0xAF, 0xE7, 0x00, 0x50, 0xE4, 0xD9, 0x3B, 0x36)

// obsolete: do not use. may be removed in the future.
//	uuid string: 3D72672A-F64A-11D4-9683-0050E4D93B36
#define kIOFireWireReadQuadletCommandInterfaceID	CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x3D, 0x72, 0x67, 0x2A, 0xF6, 0x4A, 0x11, 0xD4,\
											0x96, 0x83, 0x00, 0x50, 0xE4, 0xD9, 0x3B, 0x36)

// obsolete: do not use. may be removed in the future.
//	uuid string: 5C9423CE-F64A-11D4-AB7B-0050E4D93B36
#define kIOFireWireWriteQuadletCommandInterfaceID	CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x5C, 0x94, 0x23, 0xCE, 0xF6, 0x4A, 0x11, 0xD4,\
											0xAB, 0x7B, 0x00, 0x50, 0xE4, 0xD9, 0x3B, 0x3)

// obsolete: do not use. may be removed in the future.
//	uuid string: 6E32F9D4-F63A-11D4-A194-003065423456
#define kIOFireWireReadCommandInterfaceID	CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x6E, 0x32, 0xF9, 0xD4, 0xF6, 0x3A, 0x11, 0xD4,\
											0xA1, 0x94, 0x00, 0x30, 0x65, 0x42, 0x34, 0x56)

// obsolete: do not use. may be removed in the future.
//	uuid string: 4EDDED10-F64A-11D4-B7A5-0050E4D93B36
#define kIOFireWireWriteCommandInterfaceID	CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x4E, 0xDD, 0xED, 0x10, 0xF6, 0x4A, 0x11, 0xD4,\
											0xB7, 0xA5, 0x00, 0x50, 0xE4, 0xD9, 0x3B, 0x36)

#pragma mark -
#pragma mark ADDRESS SPACE UUIDs
// ============================================================
// address spaces
// ============================================================

//	uuid string: 0D32AC50-F198-11D4-8DB5-000502072F80
#define kIOFireWirePseudoAddressSpaceInterfaceID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x0D, 0x32, 0xAC, 0x50, 0xF1, 0x98, 0x11, 0xD4,\
											0x8D, 0xB5, 0x00, 0x05, 0x02, 0x07, 0x2F, 0x80)

//	uuid string: 489110F6-F198-11D4-8BEB-000502072F80
#define kIOFireWirePhysicalAddressSpaceInterfaceID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x48, 0x91, 0x10, 0xF6, 0xF1, 0x98, 0x11, 0xD4,\
											0x8B, 0xEB, 0x00, 0x05, 0x02, 0x07, 0x2F, 0x80)


//	uuid string: 763F18CA-5E84-4612-A2BD-10011730E131
#define kIOFireWirePHYPacketListenerInterfaceID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x76, 0x3F, 0x18, 0xCA, 0x5E, 0x84, 0x46, 0x12,\
											0xA2, 0xBD, 0x10, 0x01, 0x17, 0x30, 0xE1, 0x31)

#pragma mark -
#pragma mark CONFIG ROM UUIDs
// ============================================================
// config ROM
// ============================================================

//	uuid string: 69CA4D74-F198-11D4-B325-000502072F80
#define kIOFireWireLocalUnitDirectoryInterfaceID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x69, 0xCA, 0x4D, 0x74, 0xF1, 0x98, 0x11, 0xD4,\
											0xB3, 0x25, 0x00, 0x05, 0x02, 0x07, 0x2F, 0x80)

//  uuid string: 7D43B506-F198-11D4-AA10-000502072F80
#define kIOFireWireConfigDirectoryInterfaceID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x7D, 0x43, 0xB5, 0x06, 0xF1, 0x98, 0x11, 0xD4,\
											0xAA, 0x10, 0x00, 0x05, 0x02, 0x07, 0x2F, 0x80)

#pragma mark -
#pragma mark INTERFACE TYPES
// ============================================================
// IOFireWireLib interface types
// ============================================================

typedef struct 	IOFireWireDeviceInterface_t**	 			IOFireWireLibDeviceRef ;
typedef		   	IOFireWireLibDeviceRef						IOFireWireLibUnitRef ;
typedef			IOFireWireLibDeviceRef						IOFireWireLibNubRef ;
typedef struct 	IOFireWirePseudoAddressSpaceInterface_t**	IOFireWireLibPseudoAddressSpaceRef ;
typedef struct 	IOFireWirePhysicalAddressSpaceInterface_t**	IOFireWireLibPhysicalAddressSpaceRef ;
typedef struct 	IOFireWireLocalUnitDirectoryInterface_t**	IOFireWireLibLocalUnitDirectoryRef ;
typedef struct 	IOFireWireConfigDirectoryInterface_t**		IOFireWireLibConfigDirectoryRef ;

typedef struct 	IOFireWireCommandInterface_t**				IOFireWireLibCommandRef ;
typedef struct 	IOFireWireReadCommandInterface_t**			IOFireWireLibReadCommandRef ;
typedef struct 	IOFireWireReadQuadletCommandInterface_t**	IOFireWireLibReadQuadletCommandRef ;
typedef struct 	IOFireWireWriteCommandInterface_t**			IOFireWireLibWriteCommandRef ;
typedef struct 	IOFireWireWriteQuadletCommandInterface_t**	IOFireWireLibWriteQuadletCommandRef ;
typedef struct 	IOFireWireCompareSwapCommandInterface_t**	IOFireWireLibCompareSwapCommandRef ;
typedef struct	IOFireWireLibVectorCommandInterface_t**		IOFireWireLibVectorCommandRef;
typedef struct	IOFireWirePHYCommandInterface_t**			IOFireWireLibPHYCommandRef;
typedef struct	IOFireWireAsyncStreamCommandInterface_t**	IOFireWireLibAsyncStreamCommandRef;

typedef struct 	IOFireWireCompareSwapCommandInterface_v3_t**	IOFireWireLibCompareSwapCommandV3Ref ;

typedef struct  IOFireWireLibIRMAllocationInterface_t**		IOFireWireLibIRMAllocationRef ; 

// --- isoch interfaces ----------
typedef struct 	IOFireWireIsochChannelInterface_t**			IOFireWireLibIsochChannelRef ;
typedef struct 	IOFireWireIsochPortInterface_t**			IOFireWireLibIsochPortRef ;
typedef struct 	IOFireWireRemoteIsochPortInterface_t**		IOFireWireLibRemoteIsochPortRef ;
typedef struct 	IOFireWireLocalIsochPortInterface_t**		IOFireWireLibLocalIsochPortRef ;
typedef struct 	IOFireWireDCLCommandPoolInterface_t**		IOFireWireLibDCLCommandPoolRef ;
typedef struct	IOFireWireNuDCLPoolInterface_t**			IOFireWireLibNuDCLPoolRef ;
typedef struct  IOFWAsyncStreamListenerInterface_t**		IOFWAsyncStreamListenerInterfaceRef;
typedef struct  IOFireWireLibPHYPacketListenerInterface_t**	IOFireWireLibPHYPacketListenerRef;

#pragma mark -
#pragma mark CALLBACK TYPES
// ============================================================
// IOFireWireLib callback types
// ============================================================

/*!	@typedef IOFireWirePseudoAddressSpaceReadHandler
	@abstract This callback is called to handle read requests to pseudo address spaces. This function
		should fill in the specified area in the pseudo address space backing store and call
		ClientCommandIsComplete with the specified command ID
	@param addressSpace The address space to which the request is being made
	@param commandID An FWClientCommandID which should be passed to ClientCommandIsComplete when
		the buffer has been filled in
	@param packetLen number of bytes requested
	@param packetOffset number of bytes from beginning of address space backing store
	@param srcNodeID nodeID of the requester
	@param destAddressHi high 16 bits of destination address on this computer
	@param destAddressLo low 32 bits of destination address on this computer
	@param refCon user specified reference number passed in when the address space was created
*/
typedef UInt32	(*IOFireWirePseudoAddressSpaceReadHandler)(
					IOFireWireLibPseudoAddressSpaceRef	addressSpace,
					FWClientCommandID					commandID,
					UInt32								packetLen,
					UInt32								packetOffset,
					UInt16								srcNodeID,		// nodeID of requester
					UInt32								destAddressHi,	// destination on this node
					UInt32								destAddressLo,
					void *								refCon) ;

/*!	@typedef IOFireWirePseudoAddressSpaceSkippedPacketHandler
	@abstract Callback called when incoming packets have been dropped from the internal queue
	@param addressSpace The address space which dropped the packet(s)
	@param commandID An FWClientCommandID to be passed to ClientCommandIsComplete()
	@param skippedPacketCount The number of skipped packets
*/
typedef void	(*IOFireWirePseudoAddressSpaceSkippedPacketHandler)(
					IOFireWireLibPseudoAddressSpaceRef	addressSpace,
					FWClientCommandID					commandID,
					UInt32								skippedPacketCount) ;

/*! @typedef IOFireWirePseudoAddressSpaceWriteHandler
	@abstract Callback called to handle write requests to a pseudo address space.
	@param addressSpace The address space to which the write is being made
	@param commandID An FWClientCommandID to be passed to ClientCommandIsComplete()
	@param packetLen Length in bytes of incoming packet
	@param packet Pointer to the received data
	@param srcNodeID Node ID of the sender
	@param destAddressHi high 16 bits of destination address on this computer
	@param destAddressLo low 32 bits of destination address on this computer
	@param refCon user specified reference number passed in when the address space was created
*/
typedef UInt32 (*IOFireWirePseudoAddressSpaceWriteHandler)(
					IOFireWireLibPseudoAddressSpaceRef	addressSpace,
					FWClientCommandID					commandID,
					UInt32								packetLen,
					void*								packet,
					UInt16								srcNodeID,		// nodeID of sender
					UInt32								destAddressHi,	// destination on this node
					UInt32								destAddressLo,
					void *								refCon) ;

/*!	@typedef IOFireWireBusResetHandler
	@abstract Called when a bus reset has occured, but before FireWire has completed
		configuring the bus.
	@param interface A reference to the device on which the callback was installed
	@param commandID An FWClientCommandID to be passed to ClientCommandIsComplete()
*/
typedef void 	(*IOFireWireBusResetHandler)(
					IOFireWireLibDeviceRef				interface,
					FWClientCommandID					commandID );	// parameters may change
					
/*!
	@typedef IOFireWireBusResetDoneHandler
	@abstract Called when a bus reset has occured and FireWire has completed configuring
		the bus.
	@param interface A reference to the device on which the callback was installed
	@param commandID An FWClientCommandID to be passed to ClientCommandIsComplete()
*/
typedef void 	(*IOFireWireBusResetDoneHandler)(
					IOFireWireLibDeviceRef				interface,
					FWClientCommandID					commandID ) ;	// parameters may change

/*!	@typedef IOFireWireLibCommandCallback
	@abstract Callback called when an asynchronous command has completed executing
	@param refCon A user specified reference value set before command object was submitted
*/
typedef void	(*IOFireWireLibCommandCallback)(
					void*								refCon,
					IOReturn							completionStatus) ;

/*!	@typedef IOFireWireLibPHYPacketCallback
	@abstract Callback called to handle incoming PHY packets
	@param listener The listener which received the callback
	@param commandID An FWClientCommandID to be passed to ClientCommandIsComplete()
	@param data1 first quad of received PHY packet
	@param data2 second quad of received PHY packet	
	@param refCon user specified reference value specified on the listener  
*/
typedef void	(*IOFireWireLibPHYPacketCallback)(
					IOFireWireLibPHYPacketListenerRef	listener,
					FWClientCommandID					commandID,
					UInt32								data1,
					UInt32								data2,
					void *								refCon );

/*!	@typedef IOFireWireLibPHYPacketSkippedCallback
	@abstract Callback called when incoming packets have been dropped from the internal queue
	@param listener The listener which dropped the packets
	@param commandID An FWClientCommandID to be passed to ClientCommandIsComplete()
	@param skippedPacketCount The number of skipped packets
	@param refCon user specified reference value specified on the listener  
*/
typedef void	(*IOFireWireLibPHYPacketSkippedCallback)(
					IOFireWireLibPHYPacketListenerRef	listener,
					FWClientCommandID					commandID,
					UInt32								skippedPacketCount,
					void *								refCon );

/*!	@typedef IOFireWireLibIRMAllocationLostNotificationProc
@abstract Callback called when an IOFireWireLibIRMAllocationRef fails to reclaim IRM resources after a bus-reset
*/
typedef void	(*IOFireWireLibIRMAllocationLostNotificationProc)(IOFireWireLibIRMAllocationRef irmAllocation, void *refCon);

/*! @typedef IOFWAsyncStreamListenerHandler
	@abstract Callback called to handle Async Stream packets.
	@param listener The listener which received the callback
	@param commandID An FWClientCommandID to be passed to ClientCommandIsComplete()
	@param packet Pointer to the received data
	@param refCon user specified reference number passed in when async stream 
	                 interface is created   
*/
typedef UInt32 (*IOFWAsyncStreamListenerHandler)(
					IOFWAsyncStreamListenerInterfaceRef	listener,
					FWClientCommandID					commandID,
					UInt32								size,
					void*								packet,
					void*								refCon) ;


/*!	@typedef IOFWAsyncStreamListenerSkippedPacketHandler
	@abstract Callback called when incoming packets have been dropped from the internal queue
	@param listener The listener which dropped the packets
	@param commandID An FWClientCommandID to be passed to ClientCommandIsComplete()
	@param skippedPacketCount The number of skipped packets
*/
typedef void	(*IOFWAsyncStreamListenerSkippedPacketHandler)(
					IOFWAsyncStreamListenerInterfaceRef	listener,
					FWClientCommandID					commandID,
					UInt32								skippedPacketCount) ;

#pragma mark -
#pragma mark DEVICE INTERFACE
// ============================================================
// IOFireWireDeviceInterface
// ============================================================

/*!	@class
	@abstract IOFireWireDeviceInterface is your primary gateway to the functionality contained in
		IOFireWireLib.
	@discussion	
		You can use IOFireWireDeviceInterface to:<br>
	<ul>
		<li>perform synchronous read, write and lock operations</li>
		<li>perform other miscellanous bus operations, such as reset the FireWire bus. </li>
		<li>create FireWire command objects and interfaces used to perform
			synchronous/asynchronous read, write and lock operations. These include:</li>
		<ul type="square">
			<li>IOFireWireReadCommandInterface
			<li>IOFireWireReadQuadletCommandInterface
			<li>IOFireWireWriteCommandInterface
			<li>IOFireWireWriteQuadletCommandInterface
			<li>IOFireWireCompareSwapCommandInterface
		</ul>
		<li>create interfaces which provide a other extended services. These include:</li>
		<ul type="square">	
			<li>IOFireWirePseudoAddressSpaceInterface -- pseudo address space services</li>
			<li>IOFireWirePhysicalAddressSpaceInterface -- physical address space services</li>
			<li>IOFireWireLocalUnitDirectoryInterface -- manage local unit directories in the mac</li>
			<li>IOFireWireConfigDirectoryInterface -- access and browse remote device config directories</li>
		</ul>
		<li>create interfaces which provide isochronous services (see IOFireWireLibIsoch.h). These include:</li>
		<ul type="square">	
			<li>IOFireWireIsochChannelInterface -- create/manage talker and listener isoch channels</li>
			<li>IOFireWireLocalIsochPortInterface -- create local isoch ports</li>
			<li>IOFireWireRemoteIsochPortInterface -- create remote isoch ports</li>
			<li>IOFireWireDCLCommandPoolInterface -- create a DCL command pool allocator.</li>
		</ul>
	</ul>

*/
typedef struct IOFireWireDeviceInterface_t
{
	IUNKNOWN_C_GUTS ;

	/*! Interface version */
	UInt32 version;

	/*! Interface revision */
 	UInt32 revision; // version/revision
	
		// --- maintenance methods -------------
    /*!
        @function InterfaceIsInited
        @abstract Determine whether interface has been properly inited.
        @param self The device interface to use.
        @result Returns true if interface is inited and false if is it not.
    */
	Boolean				(*InterfaceIsInited)(IOFireWireLibDeviceRef self) ;

    /*!
        @function GetDevice
        @abstract Get the IOKit service to which this interface is connected.
        @param self The device interface to use.
        @result Returns an io_object_t corresponding to the device the interface is
			using
    */
	io_object_t			(*GetDevice)(IOFireWireLibDeviceRef self) ;

    /*!
        @function Open
        @abstract Open the connected device for exclusive access. When you have
			the device open using this method, all accesses by other clients of 
			this device will be denied until Close() is called.
        @param self The device interface to use.
        @result An IOReturn error code
    */
	IOReturn			(*Open)(IOFireWireLibDeviceRef self) ;

    /*!
        @function OpenWithSessionRef
        @abstract An open function which allows this interface to have access
			to the device when already opened. The service which has already opened
			the device must be able to provide an IOFireWireSessionRef.
        @param self The device interface to use
		@param sessionRef The sessionRef returned from the client who has
			the device open
        @result An IOReturn error code
    */
	IOReturn			(*OpenWithSessionRef)(IOFireWireLibDeviceRef self, IOFireWireSessionRef sessionRef) ;

    /*!
        @function Close
        @abstract Release exclusive access to the device
        @param self The device interface to use
    */
	void				(*Close)(IOFireWireLibDeviceRef self) ;
	
		// --- notification --------------------
	/*!
		@function NotificationIsOn
		@abstract Determine whether callback notifications for this interface are currently active
        @param self The device interface to use
		@result A Boolean value where true indicates notifications are active
	*/
	const Boolean		(*NotificationIsOn)(IOFireWireLibDeviceRef self) ;
	
	/*!
		@function AddCallbackDispatcherToRunLoop
		@abstract Installs the proper run loop event source to allow callbacks to function. This method
			must be called before callback notifications for this interface or any interfaces
			created using this interface can function.
        @param self The device interface to use.
		@param inRunLoop The run loop on which to install the event source
	*/
	const IOReturn 		(*AddCallbackDispatcherToRunLoop)(IOFireWireLibDeviceRef self, CFRunLoopRef inRunLoop) ;
	
	/*!
		@function RemoveCallbackDispatcherFromRunLoop
		@abstract Reverses the effects of AddCallbackDispatcherToRunLoop(). This method removes 
			the run loop event source that was added to the specified run loop preventing any 
			future callbacks from being called
        @param self The device interface to use.			
	*/
	const void			(*RemoveCallbackDispatcherFromRunLoop)(IOFireWireLibDeviceRef self) ;
	
	/*!
		@function TurnOnNotification
		@abstract Activates any callbacks specified for this device interface. Only works after 
			AddCallbackDispatcherToRunLoop has been called. See also AddIsochCallbackDispatcherToRunLoop().
        @param self The device interface to use.
		@result A Boolean value. Returns true on success.
	*/
	const Boolean 		(*TurnOnNotification)(IOFireWireLibDeviceRef self) ;

	/*!
		@function TurnOffNotification
		@abstract Deactivates and callbacks specified for this device interface. Reverses the 
			effects of TurnOnNotification()
        @param self The device interface to use.
	*/
	void				(*TurnOffNotification)(IOFireWireLibDeviceRef self) ;
	
	/*!
		@function SetBusResetHandler
		@abstract Sets the callback that should be called when a bus reset occurs. Note that this callback
			can be called multiple times before the bus reset done handler is called. (f.ex., multiple bus
			resets might occur before bus reconfiguration has completed.)
        @param self The device interface to use.
		@param handler Function pointer to the handler to install
		@result Returns an IOFireWireBusResetHandler function pointer to the previously installed
			bus reset handler. Returns 0 if none was set.
	*/
	const IOFireWireBusResetHandler	
						(*SetBusResetHandler)(IOFireWireLibDeviceRef self, IOFireWireBusResetHandler handler) ;
	
	/*!
		@function SetBusResetDoneHandler
		@abstract Sets the callback that should be called after a bus reset has occurred and reconfiguration
			of the bus has been completed. This function will only be called once per bus reset.
        @param self The device interface to use.
		@param handler Function pointer to the handler to install
		@result Returns on IOFireWireBusResetDoneHandler function pointer to the previously installed
			bus reset handler. Returns 0 if none was set.
	*/
	const IOFireWireBusResetDoneHandler	
						(*SetBusResetDoneHandler)(IOFireWireLibDeviceRef self, IOFireWireBusResetDoneHandler handler) ;
	/*!
		@function ClientCommandIsComplete
		@abstract This function must be called from callback routines once they have completed processing
			a callback. This function only applies to callbacks which take an IOFireWireLibDeviceRef (i.e. bus reset),
			parameter.
		@param commandID The command ID passed to the callback function when it was called
		@param status An IOReturn value indicating the completion status of the callback function
	*/
	void				(*ClientCommandIsComplete)(IOFireWireLibDeviceRef self, FWClientCommandID commandID, IOReturn status) ;
	
		// --- read/write/lock operations -------
	/*!
		@function Read
		@abstract Perform synchronous block read
		@param self The device interface to use.
		@param device The service (representing an attached FireWire device) to read. 
			For 48-bit, device relative addressing, pass the service used to create the device interface. This
			can be obtained by calling GetDevice(). For 64-bit absolute addressing, pass 0. Other values are
			unsupported.
		@param addr Command target address
		@param buf A pointer to a buffer where the results will be stored
		@param size Number of bytes to read
		@param failOnReset Pass true if the command should only be executed during the FireWire bus generation
			specified in generation. Pass false to ignore the generation parameter. The generation can be
			obtained by calling GetBusGeneration(). Must be 'true' when using 64-bit addressing.
			
		@param generation The FireWire bus generation during which the command should be executed. Ignored
			if failOnReset is false. Must be a valid generation number when using 64-bit absolute addressing.
		@result An IOReturn error code
	*/
	IOReturn			(*Read)(IOFireWireLibDeviceRef	self, 
								io_object_t 		device,
								const FWAddress* addr, 
								void* 				buf, 
								UInt32* 			size, 
								Boolean 			failOnReset, 
								UInt32 				generation) ;

	/*!
		@function ReadQuadlet
		@abstract Perform synchronous quadlet read
		@param self The device interface to use.
		@param device The service (representing an attached FireWire device) to read. 
			For 48-bit, device relative addressing, pass the service used to create the device interface. This
			can be obtained by calling GetDevice(). For 64-bit absolute addressing, pass 0. Other values are
			unsupported.
		@param addr Command target address
		@param val A pointer to where to data should be stored
		@param failOnReset Pass true if the command should only be executed during the FireWire bus generation
			specified in generation. Pass false to ignore the generation parameter. The generation can be
			obtained by calling GetBusGeneration()
		@param generation The FireWire bus generation during which the command should be executed. Ignored
			if failOnReset is false. Must be a valid generation number when using 64-bit absolute addressing.
		@result An IOReturn error code
	*/
	IOReturn			(*ReadQuadlet)(	IOFireWireLibDeviceRef	self, 
										io_object_t 			device,
										const FWAddress* 		addr, 
										UInt32* 				val, 
										Boolean 				failOnReset, 
										UInt32 					generation) ;
	/*!
		@function Write
		@abstract Perform synchronous block write
		@param self The device interface to use.
		@param device The service (representing an attached FireWire device) to which to write.
			For 48-bit, device relative addressing, pass the service used to create the device interface. This
			can be obtained by calling GetDevice(). For 64-bit absolute addressing, pass 0. Other values are
			unsupported.
		@param addr Command target address
		@param buf A pointer to a buffer where the results will be stored
		@param size Number of bytes to read
		@param failOnReset Pass true if the command should only be executed during the FireWire bus generation
			specified in 'generation'. Pass false to ignore the generation parameter. The generation can be
			obtained by calling GetBusGeneration(). Must be 'true' when using 64-bit addressing.
		@param generation The FireWire bus generation during which the command should be executed. Ignored
			if failOnReset is false. Must be a valid generation number when using 64-bit absolute addressing.
		@result An IOReturn error code
	*/
	IOReturn			(*Write)(	IOFireWireLibDeviceRef 	self, 
									io_object_t 			device, 
									const FWAddress* 		addr, 
									const void* 			buf, 
									UInt32* 				size,
									Boolean 				failOnReset, 
									UInt32 					generation) ;

	/*!
		@function WriteQuadlet
		@abstract Perform synchronous quadlet write
		@param self The device interface to use.
		@param device The service (representing an attached FireWire device) to which to write.
			For 48-bit, device relative addressing, pass the service used to create the device interface. This
			can be obtained by calling GetDevice(). For 64-bit absolute addressing, pass 0. Other values are
			unsupported.
		@param addr Command target address
		@param val The value to write
		@param failOnReset Pass true if the command should only be executed during the FireWire bus generation
			specified in 'generation'. Pass false to ignore the generation parameter. The generation can be
			obtained by calling GetBusGeneration(). Must be 'true' when using 64-bit addressing.
		@param generation The FireWire bus generation during which the command should be executed. Ignored
			if failOnReset is false. Must be a valid generation number when using 64-bit absolute addressing.
		@result An IOReturn error code
	*/
	IOReturn (*WriteQuadlet)(IOFireWireLibDeviceRef self, io_object_t device, const FWAddress* addr, const UInt32 val, Boolean failOnReset, UInt32 generation) ;

	/*!
		@function CompareSwap
		@abstract Perform synchronous lock operation
		@param self The device interface to use.
		@param device The service (representing an attached FireWire device) to which to write.
			For 48-bit, device relative addressing, pass the service used to create the device interface. This
			can be obtained by calling GetDevice(). For 64-bit absolute addressing, pass 0. Other values are
			unsupported.
		@param addr Command target address
		@param cmpVal The check/compare value
		@param newVal Value to set
		@param failOnReset Pass true if the command should only be executed during the FireWire bus generation
			specified in 'generation'. Pass false to ignore the generation parameter. The generation can be
			obtained by calling GetBusGeneration(). Must be 'true' when using 64-bit addressing.
		@param generation The FireWire bus generation during which the command should be executed. Ignored
			if failOnReset is false. Must be a valid generation number when using 64-bit absolute addressing.
		@result An IOReturn error code
	*/
	IOReturn (*CompareSwap)(IOFireWireLibDeviceRef self, io_object_t device, const FWAddress* addr, UInt32 cmpVal, UInt32 newVal, Boolean failOnReset, UInt32 generation) ;
	
	// --- FireWire command object methods ---------

	/*!
		@function CreateReadCommand
		@abstract Create a block read command object.
		@param self The device interface to use.
		@param device The service (representing an attached FireWire device) to which to write.
			For 48-bit, device relative addressing, pass the service used to create the device interface. This
			can be obtained by calling GetDevice(). For 64-bit absolute addressing, pass 0. Other values are
			unsupported. Setting the callback value to nil defaults to synchronous execution.
		@param addr Command target address
		@param buf A pointer to a buffer where the results will be stored
		@param size Number of bytes to read
		@param callback Command completion callback. Setting the callback value to nil defaults to synchronous execution.
		@param failOnReset Pass true if the command should only be executed during the FireWire bus generation
			specified in 'generation'. Pass false to ignore the generation parameter. The generation can be
			obtained by calling GetBusGeneration(). Must be 'true' when using 64-bit addressing.
		@param generation The FireWire bus generation during which the command should be executed. Ignored
			if failOnReset is false. Must be a valid generation number when using 64-bit absolute addressing.
		@result An IOFireWireLibCommandRef interface. See IOFireWireLibCommandRef.
	*/
	IOFireWireLibCommandRef (*CreateReadCommand)( IOFireWireLibDeviceRef self, io_object_t device, const FWAddress * addr, void* buf, UInt32 size, IOFireWireLibCommandCallback callback, Boolean failOnReset, UInt32 generation, void* inRefCon, REFIID iid) ;

	/*!	@function CreateReadQuadletCommand
		@abstract Create a quadlet read command object.
		@param self The device interface to use.
		@param device The service (representing an attached FireWire device) to which to write.
			For 48-bit, device relative addressing, pass the service used to create the device interface. This
			can be obtained by calling GetDevice(). For 64-bit absolute addressing, pass 0. Other values are
			unsupported. Setting the callback value to nil defaults to synchronous execution.
		@param addr Command target address
		@param quads An array of quadlets where results should be stored
		@param numQuads Number of quadlets to read
		@param callback Command completion callback. Setting the callback value to nil defaults to synchronous execution.
		@param failOnReset Pass true if the command should only be executed during the FireWire bus generation
			specified in 'generation'. Pass false to ignore the generation parameter. The generation can be
			obtained by calling GetBusGeneration(). Must be 'true' when using 64-bit addressing.
		@param generation The FireWire bus generation during which the command should be executed. Ignored
			if failOnReset is false. Must be a valid generation number when using 64-bit absolute addressing.
		@result An IOFireWireLibCommandRef interface. See IOFireWireLibCommandRef.*/
	IOFireWireLibCommandRef (*CreateReadQuadletCommand)( IOFireWireLibDeviceRef self, io_object_t device, const FWAddress * addr, UInt32 quads[], UInt32 numQuads, IOFireWireLibCommandCallback callback, Boolean failOnReset, UInt32 generation, void* inRefCon, REFIID iid) ;

	/*!	@function CreateWriteCommand
		@abstract Create a block write command object.
		@param self The device interface to use.
		@param device The service (representing an attached FireWire device) to which to write.
			For 48-bit, device relative addressing, pass the service used to create the device interface. This
			can be obtained by calling GetDevice(). For 64-bit absolute addressing, pass 0. Other values are
			unsupported. Setting the callback value to nil defaults to synchronous execution.
		@param addr Command target address
		@param buf A pointer to the buffer containing the data to be written
		@param size Number of bytes to write
		@param callback Command completion callback. Setting the callback value to nil defaults to synchronous execution.
		@param failOnReset Pass true if the command should only be executed during the FireWire bus generation
			specified in 'generation'. Pass false to ignore the generation parameter. The generation can be
			obtained by calling GetBusGeneration(). Must be 'true' when using 64-bit addressing.
		@param generation The FireWire bus generation during which the command should be executed. Ignored
			if failOnReset is false. Must be a valid generation number when using 64-bit absolute addressing.
		@result An IOFireWireLibCommandRef interface. See IOFireWireLibCommandRef.*/
	IOFireWireLibCommandRef (*CreateWriteCommand)( IOFireWireLibDeviceRef self, io_object_t device, const FWAddress * addr, void* buf, UInt32  size, IOFireWireLibCommandCallback callback, Boolean failOnReset, UInt32 generation, void* inRefCon, REFIID iid) ;

	/*!
		@function CreateWriteQuadletCommand
		@abstract Create a quadlet write command object.
		@param self The device interface to use.
		@param device The service (representing an attached FireWire device) to which to write.
			For 48-bit, device relative addressing, pass the service used to create the device interface. This
			can be obtained by calling GetDevice(). For 64-bit absolute addressing, pass 0. Other values are
			unsupported. Setting the callback value to nil defaults to synchronous execution.
		@param addr Command target address
		@param quads An array of quadlets containing quadlets to be written
		@param numQuads Number of quadlets to write
		@param callback Command completion callback. Setting the callback value to nil defaults to synchronous execution.
		@param failOnReset Pass true if the command should only be executed during the FireWire bus generation
			specified in 'generation'. Pass false to ignore the generation parameter. The generation can be
			obtained by calling GetBusGeneration(). Must be 'true' when using 64-bit addressing.
		@param generation The FireWire bus generation during which the command should be executed. Ignored
			if failOnReset is false. Must be a valid generation number when using 64-bit absolute addressing.
		@result An IOFireWireLibCommandRef interface. See IOFireWireLibCommandRef.
	*/
	IOFireWireLibCommandRef (*CreateWriteQuadletCommand)(IOFireWireLibDeviceRef	self, io_object_t device, const FWAddress *	addr, UInt32 quads[], UInt32 numQuads, IOFireWireLibCommandCallback callback, Boolean failOnReset, UInt32 generation, void* inRefCon, REFIID iid) ;

	/*!
		@function CreateCompareSwapCommand
		@abstract Create a quadlet compare/swap command object.
		@param self The device interface to use.
		@param device The service (representing an attached FireWire device) to which to write.
			For 48-bit, device relative addressing, pass the service used to create the device interface. This
			can be obtained by calling GetDevice(). For 64-bit absolute addressing, pass 0. Other values are
			unsupported. Setting the callback value to nil defaults to synchronous execution.
		@param addr Command target address
		@param cmpVal 32-bit value expected at target address
		@param newVal 32-bit value to be set at target address
		@param callback Command completion callback. Setting the callback value to nil defaults to synchronous execution.
		@param failOnReset Pass true if the command should only be executed during the FireWire bus generation
			specified in 'generation'. Pass false to ignore the generation parameter. The generation can be
			obtained by calling GetBusGeneration(). Must be 'true' when using 64-bit addressing.
		@param generation The FireWire bus generation during which the command should be executed. Ignored
			if failOnReset is false. Must be a valid generation number when using 64-bit absolute addressing.
		@result An IOFireWireLibCommandRef interface. See IOFireWireLibCommandRef.	*/
	IOFireWireLibCommandRef (*CreateCompareSwapCommand)( IOFireWireLibDeviceRef self, io_object_t device, const FWAddress *  addr, UInt32      cmpVal, UInt32      newVal, IOFireWireLibCommandCallback callback, Boolean failOnReset, UInt32 generation, void* inRefCon, REFIID iid) ;

		// --- other methods ---------------------------
	/*!	@function BusReset
		@abstract Cause a bus reset
		@param self The device interface to use. */	
	IOReturn (*BusReset)( IOFireWireLibDeviceRef  self) ;

	/*!	@function GetCycleTime
		@abstract Get bus cycle time.
		@param self The device interface to use.
		@param outCycleTime A pointer to a UInt32 to hold the result
		@result An IOReturn error code.	*/	
	IOReturn (*GetCycleTime)( IOFireWireLibDeviceRef  self, UInt32*  outCycleTime) ;

	/*!	@function GetGenerationAndNodeID
		@abstract (Obsolete) Get bus generation and remote device node ID.
		@discussion Obsolete -- Please use GetBusGeneration() and/or GetRemoteNodeID() in
			interface v4.
		@param self The device interface to use.
		@param outGeneration A pointer to a UInt32 to hold the generation result
		@param outNodeID A pointer to a UInt16 to hold the remote device node ID
		@result An IOReturn error code.	*/	
	IOReturn (*GetGenerationAndNodeID)( IOFireWireLibDeviceRef  self, UInt32*  outGeneration, UInt16*  outNodeID) ;

	/*!	@function GetLocalNodeID
		@abstract (Obsolete) Get local node ID.
		@discussion Obsolete -- Please use GetBusGeneration() and GetLocalNodeIDWithGeneration() in
			interface v4.
		@param self The device interface to use.
		@param outLocalNodeID A pointer to a UInt16 to hold the local device node ID
		@result An IOReturn error code.	*/	
	IOReturn (*GetLocalNodeID)( IOFireWireLibDeviceRef  self, UInt16*  outLocalNodeID) ;

	/*!	@function GetResetTime
		@abstract Get time since last bus reset.
		@param self The device interface to use.
		@param outResetTime A pointer to an AbsolutTime to hold the result.
		@result An IOReturn error code.	*/	
	IOReturn			(*GetResetTime)(
								IOFireWireLibDeviceRef 	self, 
								AbsoluteTime* 			outResetTime) ;

		// --- unit directory support ------------------
	/*!	@function CreateLocalUnitDirectory
		@abstract Creates a local unit directory object and returns an interface to it. An
			instance of a unit directory object corresponds to an instance of a unit 
			directory in the local machine's configuration ROM.
		@param self The device interface to use.
		@param iid An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the
			type of interface to be returned for the created unit directory object.
		@result An IOFireWireLibLocalUnitDirectoryRef. Returns 0 upon failure */
	 IOFireWireLibLocalUnitDirectoryRef (*CreateLocalUnitDirectory)( IOFireWireLibDeviceRef  self, REFIID  iid) ;

		// --- config directory support ----------------
	/*!	@function GetConfigDirectory
		@abstract Creates a config directory object and returns an interface to it. The
			created config directory object represents the config directory in the remote
			device or unit to which the creating device interface is attached.
		@param self The device interface to use.
		@param iid An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the
			type of interface to be returned for the created config directory object.
		@result An IOFireWireLibConfigDirectoryRef which should be released using Release().
			Returns 0 upon failure. */
	IOFireWireLibConfigDirectoryRef (*GetConfigDirectory)( IOFireWireLibDeviceRef  self, REFIID  iid) ;

	/*!	@function CreateConfigDirectoryWithIOObject
		@abstract This function can be used to create a config directory object and a
			corresponding interface from an opaque IOObject reference. Some configuration
			directory interface methods may return an io_object_t instead of an
			IOFireWireLibConfigDirectoryRef. Use this function to obtain an 
			IOFireWireLibConfigDirectoryRef from an io_object_t.
		@param self The device interface to use.
		@param iid An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the
			type of interface to be returned for the created config directory object.
		@result An IOFireWireLibConfigDirectoryRef. Returns 0 upon failure */
	IOFireWireLibConfigDirectoryRef (*CreateConfigDirectoryWithIOObject)( IOFireWireLibDeviceRef  self, io_object_t  inObject, REFIID  iid) ;

		// --- address space support -------------------
	/*!	@function CreatePseudoAddressSpace
		@abstract Creates a pseudo address space object and returns an interface to it. This
			will create a pseudo address space (software-backed) on the local machine. 
		@param self The device interface to use.
		@param inSize The size in bytes of this address space.
		@param inRefCon A user specified reference value. This will be passed to all callback functions.
		@param inQueueBufferSize The size of the queue which receives packets from the bus before they are handed to
			the client and/or put in the backing store. A larger queue can help eliminate dropped packets
			when receiving large bursts of data. When a packet is received which can not fit into the queue, 
			the packet dropped callback will be called. 
		@param inBackingStore An optional block of allocated memory representing the contents of the address space. This 
			memory block must be of size inSize.
		@param inFlags A UInt32 with bits set corresponding to the flags that should be set
			for this address space.
			<ul>
				<li>kFWAddressSpaceNoFlags -- All flags off</li>
				<li>kFWAddressSpaceNoWriteAccess -- Write access to this address space will be disallowed. 
					Setting this flag also disables compare/swap transactions on this address space.</li>
				<li>kFWAddressSpaceNoReadAccess -- Read access access to this address space will be disallowed. 
					Setting this flag also disables compare/swap transactions on this address space.</li>
				<li>kFWAddressSpaceAutoWriteReply -- Writes will be made automatically, directly modifying the contents
					of the backing store. The user process will not be notified of writes.</li>
				<li>kFWAddressSpaceAutoReadReply -- Reads to this address space will be answered automagically
					using the contents of the backing store. The user process will not be notified of reads.</li>
				<li>kFWAddressSpaceAutoCopyOnWrite -- Writes to this address space will be made directly
					to the backing store at the same time the user process is notified of a write.</li>
			</ul>
		@param iid An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the
			type of interface to be returned for the created pseudo address space object.
		@result An IOFireWireLibPseudoAddressSpaceRef. Returns 0 upon failure */
	IOFireWireLibPseudoAddressSpaceRef	(*CreatePseudoAddressSpace)( 
												IOFireWireLibDeviceRef  	self, 
												UInt32  					inSize, 
												void*  						inRefCon, 
												UInt32  					inQueueBufferSize, 
												void*  						inBackingStore, 
												UInt32  					inFlags, 
												REFIID  					iid) ;

	/*!	@function CreatePhysicalAddressSpace
		@abstract Creates a physical address space object and returns an interface to it. This
			will create a physical address space on the local machine. 
		@param self The device interface to use.
		@param inSize The size in bytes of this address space.
		@param inBackingStore An block of allocated memory representing the contents of the address space.
		@param inFlags A UInt32 with bits set corresponding to the flags that should be set
			for this address space. For future use -- always pass 0.
		@param iid An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the
			type of interface to be returned for the created physical address space object.
		@result An IOFireWireLibPhysicalAddressSpaceRef. Returns 0 upon failure */
	IOFireWireLibPhysicalAddressSpaceRef (*CreatePhysicalAddressSpace)( IOFireWireLibDeviceRef  self, UInt32  inSize, void*  inBackingStore, UInt32  inFlags, REFIID  iid) ;
		
		// --- debugging -------------------------------

	/*! Description forthcoming */
	IOReturn (*FireBugMsg)( IOFireWireLibDeviceRef  self, const char*  msg) ;

	//
	// NOTE: the following methods available only in interface v2 and later
	//

		// --- eye-sock-run-U.S. -----------------------
	/*!	@function AddIsochCallbackDispatcherToRunLoop
		@abstract This function adds an event source for the isochronous callback dispatcher
			to the specified CFRunLoop. Isochronous related callbacks will not function
			before this function is called. This functions is similar to 
			AddCallbackDispatcherToRunLoop. The passed CFRunLoop can be different
			from that passed to AddCallbackDispatcherToRunLoop. 
		@param self The device interface to use.
		@param inRunLoop A CFRunLoopRef for the run loop to which the event loop source 
			should be added
		@result An IOReturn error code.	*/	
	IOReturn			(*AddIsochCallbackDispatcherToRunLoop)(
								IOFireWireLibDeviceRef 	self, 
								CFRunLoopRef 			inRunLoop) ;

	/*!	@function CreateRemoteIsochPort
		@abstract Creates a remote isochronous port object and returns an interface to it. A
			remote isochronous port object is an abstract entity used to represent a remote
			talker or listener device on an isochronous channel. 
		@param self The device interface to use.
		@param inTalking Pass true if this port represents an isochronous talker. Pass
			false if this port represents an isochronous listener.
		@param iid An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the
			type of interface to be returned for the created remote isochronous port object.
		@result An IOFireWireLibRemoteIsochPortRef. Returns 0 upon failure */
	IOFireWireLibRemoteIsochPortRef
						(*CreateRemoteIsochPort)(
								IOFireWireLibDeviceRef	self,
								Boolean					inTalking,
								REFIID					iid) ;

	/*!	@function CreateLocalIsochPort
		@abstract Creates a local isochronous port object and returns an interface to it. A
			local isochronous port object is an abstract entity used to represent a
			talking or listening endpoint in the local machine. 
		@param self The device interface to use.
		@param inTalking Pass true if this port represents an isochronous talker. Pass
			false if this port represents an isochronous listener.
		@param inDCLProgram A pointer to the first DCL command struct of the DCL program
			to be compiled and used to send or receive data on this port.
		@param inStartEvent Start event: 0 or kFWDCLCycleEvent or kFWDCLSyBitsEvent
		@param inStartState Start state bits. For kFWDCLCycleEvent specifies the cycle to start the DMA on.
            For kFWDCLSyBitsEvent specifies the packet sync field value for the first packet to receive.
		@param inStartMask Start mask bits. For kFWDCLCycleEvent specifies a mask for the start cycle:
            the DMA will start when currentCycle & inStartMask == inStartEvent & inStartMask.
            For kFWDCLSyBitsEvent specifies a mask for the sync field:
            the DMA will start when packet sync == inStartEvent & inStartMask.
		@param inDCLProgramRanges This is an optional optimization parameter which can be used
			to decrease the time the local port object spends determining which set of virtual
			ranges the passed DCL program occupies. Pass a pointer to an array of IOVirtualRange
			structs or nil to ignore this parameter.
		@param inDCLProgramRangeCount The number of virtual ranges passed to inDCLProgramRanges.
			Pass 0 for none.
		@param inBufferRanges This is an optional optimization parameter which can be used
			to decrease the time the local port object spends determining which set of virtual
			ranges the data buffers referenced by the passed DCL program occupy. Pass a pointer
			to an array of IOVirtualRange structs or nil to ignore this parameter.
		@param inBufferRangeCount The number of virtual ranges passed to inBufferRanges.
			Pass 0 for none.
		@param iid An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the
			type of interface to be returned for the created object.
		@result An IOFireWireLibLocalIsochPortRef. Returns 0 upon failure */
	IOFireWireLibLocalIsochPortRef
						(*CreateLocalIsochPort)(
								IOFireWireLibDeviceRef 	self, 
								Boolean					inTalking,
								DCLCommand*				inDCLProgram,
								UInt32					inStartEvent,
								UInt32					inStartState,
								UInt32					inStartMask,
								IOVirtualRange			inDCLProgramRanges[],	// optional optimization parameters
								UInt32					inDCLProgramRangeCount,	
								IOVirtualRange			inBufferRanges[],
								UInt32					inBufferRangeCount, 
								REFIID 					iid) ; 

	/*!	@function CreateIsochChannel
		@abstract Creates an isochronous channel object and returns an interface to it. An
			isochronous channel object is an abstract entity used to represent a
			FireWire isochronous channel. 
		@param self The device interface to use.
		@param doIrm Controls whether the channel automatically performs IRM operations. 
			Pass true if the channel should allocate its channel and bandwidth with
			the IRM. Pass false to ignore the IRM.
		@param packetSize Size of payload in bytes of packets being sent or received with this channel,
			excluding headers. This is automatically translated into a bandwidth allocation appropriate
			for the speed passed in prefSpeed.
		@param prefSpeed The preferred bus speed of this channel.
		@param iid An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the
			type of interface to be returned for the created object.
		@result An IOFireWireLibIsochChannelRef. Returns 0 upon failure */
	IOFireWireLibIsochChannelRef
						(*CreateIsochChannel)(
								IOFireWireLibDeviceRef 	self, 
								Boolean 				doIrm, 
								UInt32 					packetSize, 
								IOFWSpeed 				prefSpeed, 
								REFIID 					iid ) ;

	/*!	@function CreateDCLCommandPool
		@abstract Creates a command pool object and returns an interface to it. The command 
			pool can be used to build DCL programs.
		@param self The device interface to use.
		@param size Starting size of command pool
		@param iid An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the
			type of interface to be returned for the created object.
		@result An IOFireWireLibDCLCommandPoolRef. Returns 0 upon failure */
	IOFireWireLibDCLCommandPoolRef
						(*CreateDCLCommandPool)(
								IOFireWireLibDeviceRef	self, 
								IOByteCount 			size, 
								REFIID 					iid ) ;

	// --- refcons ---------------------------------
	/*!	@function GetRefCon
		@abstract Get user reference value set on this interface
		@param self The device interface to use.
		@result Returns the user's reference value set on this interface. */
	void*				(*GetRefCon)(
								IOFireWireLibDeviceRef	self) ;
	/*!	@function SetRefCon
		@abstract Set user reference value on this interface
		@param self The device interface to use.
		@param refCon The reference value to set. */
	void				(*SetRefCon)(
								IOFireWireLibDeviceRef	self,
								const void*				refCon) ;

	// --- debugging -------------------------------
	// do not use this function
	/*! @function
	    @description
		Do not use this function.
	 */
	CFTypeRef			(*GetDebugProperty)(
								IOFireWireLibDeviceRef	self,
								void*					interface,
								CFStringRef				inPropertyName,
								CFTypeID*				outPropertyType) ;

	/*!	@function PrintDCLProgram
		@abstract Walk a DCL program linked list and print its contents
		@param self The device interface to use.
		@param inProgram A pointer to the first DCL of the program to print
		@param inLength Number of DCLs expected in the program. PrintDCLProgram() will
			report an error if this number does not match the number of DCLs found
			in the program. */
	void 				(*PrintDCLProgram)(
								IOFireWireLibDeviceRef	self, 
								const DCLCommand*		inProgram, 
								UInt32 					inLength) ;

	//
	// NOTE: the following methods available only in interface v3 and later
	//

	// --- v3 functions ----------
	/*!	@function CreateInitialUnitsPseudoAddressSpace
		@abstract Creates a pseudo address space in initial units space.
		@discussion Creates a pseudo address space object in initial units space and returns an interface to it. This
			will create a pseudo address space (software-backed) on the local machine.
			
			Availablilty: IOFireWireDeviceInterface_v3,  and newer
			
		@param self The device interface to use.
		@param inAddressLo The lower 32 bits of the base address of the address space to be created. The address is always
			in initial units space.
		@param inSize The size in bytes of this address space.
		@param inRefCon A user specified reference value. This will be passed to all callback functions.
		@param inQueueBufferSize The size of the queue which receives packets from the bus before they are handed to
			the client and/or put in the backing store. A larger queue can help eliminate dropped packets
			when receiving large bursts of data. When a packet is received which can not fit into the queue, 
			the packet dropped callback will be called. 
		@param inBackingStore An optional block of allocated memory representing the contents of the address space. This
			memory block must be of size inSize.
		@param inFlags A UInt32 with bits set corresponding to the flags that should be set
			for this address space.
			<ul>
				<li>kFWAddressSpaceNoFlags -- All flags off</li>
				<li>kFWAddressSpaceNoWriteAccess -- Write access to this address space will be disallowed. 
					Setting this flag also disables compare/swap transactions on this address space.</li>
				<li>kFWAddressSpaceNoReadAccess -- Read access access to this address space will be disallowed. 
					Setting this flag also disables compare/swap transactions on this address space.</li>
				<li>kFWAddressSpaceAutoWriteReply -- Writes will be made automatically, directly modifying the contents
					of the backing store. The user process will not be notified of writes.</li>
				<li>kFWAddressSpaceAutoReadReply -- Reads to this address space will be answered automagically
					using the contents of the backing store. The user process will not be notified of reads.</li>
				<li>kFWAddressSpaceAutoCopyOnWrite -- Writes to this address space will be made directly
					to the backing store at the same time the user process is notified of a write. Clients
					will only be notified of a write if kFWAddressSpaceAutoWriteReply is not set.</li>
				<li>kFWAddressSpaceShareIfExists -- Allows creation of this address space even if another client
					already has an address space at the requested address. All clients will be notified of writes to
					covered addresses.</li>
				<li>kFWAddressSpaceExclusive -- Ensures that the allocation of this address space will fail if any portion
					of this address range is already allocated. If the allocation is successful this flag ensures that any 
					future allocations overlapping this range will fail even if allocted with kFWAddressSpaceShareIfExists.</li>
			</ul>
		@param iid An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the
			type of interface to be returned for the created pseudo address space object.
		@result An IOFireWireLibPseudoAddressSpaceRef. Returns 0 upon failure */
	IOFireWireLibPseudoAddressSpaceRef	(*CreateInitialUnitsPseudoAddressSpace)( 
												IOFireWireLibDeviceRef  	self,
												UInt32						inAddressLo,
												UInt32  					inSize, 
												void*  						inRefCon, 
												UInt32  					inQueueBufferSize, 
												void*  						inBackingStore, 
												UInt32  					inFlags,
												REFIID  					iid) ;
	/*!
		@function AddCallbackDispatcherToRunLoopForMode
		@abstract Add a run loop event source to allow IOFireWireLib callbacks to function.
        @discussion Installs the proper run loop event source to allow callbacks to function. This method
			must be called before callback notifications for this interface or any interfaces
			created using this interface can function. With this function, you can additionally specify
			for which run loop modes this source should be added.
			
			Availability: IOFireWireDeviceInterface_v3, and newer
			
		@param self The device interface to use.
		@param inRunLoop The run loop on which to install the event source
		@param inRunLoopMode The run loop mode(s) for which to install the event source
		@result An IOReturn error code.	*/	
	IOReturn 			(*AddCallbackDispatcherToRunLoopForMode)(
								IOFireWireLibDeviceRef 	self, 
								CFRunLoopRef 			inRunLoop,
								CFStringRef				inRunLoopMode ) ;
	/*!	@function AddIsochCallbackDispatcherToRunLoop
		@abstract Add a run loop event source to allow IOFireWireLib isoch callbacks to function.
		@discussion This function adds an event source for the isochronous callback dispatcher
			to the specified CFRunLoop. Isochronous related callbacks will not be called unless
			this function has been called. This function is similar to AddCallbackDispatcherToRunLoop. 
			The passed CFRunLoop can be different from that passed to AddCallbackDispatcherToRunLoop. 

			Availability: IOFireWireDeviceInterface_v3, and newer

		@param self The device interface to use.
		@param inRunLoop A CFRunLoopRef for the run loop to which the event loop source 
			should be added
		@param inRunLoopMode The run loop mode(s) for which to install the event source
		@result An IOReturn error code.	*/	
	IOReturn			(*AddIsochCallbackDispatcherToRunLoopForMode)(
								IOFireWireLibDeviceRef 	self,
								CFRunLoopRef			inRunLoop,
								CFStringRef 			inRunLoopMode ) ;
	/*!
		@function RemoveIsochCallbackDispatcherFromRunLoop
		@abstract Removes an IOFireWireLib-added run loop event source.
		@discussion Reverses the effects of AddIsochCallbackDispatcherToRunLoop(). This method removes 
			the run loop event source that was added to the specified run loop preventing any 
			future callbacks from being called.
			
			Availability: IOFireWireDeviceInterface_v3, and newer

        @param self The device interface to use.			
	*/
	void				(*RemoveIsochCallbackDispatcherFromRunLoop)(
								IOFireWireLibDeviceRef 	self) ;

	/*!
		@function Seize
		@abstract Seize control of device/unit
		@discussion Allows a user space client to seize control of an in-kernel service even if
			that service has been Opened() by another client or in-kernel driver. This function should be
			used with care. Admin rights are required to use this function.
			
			Calling this method makes it appear to all other drivers that the device has been unplugged.
			Open() should be called after this method has been invoked.
			
			When access is complete, Close() and then IOServiceRequestProbe() should be called to restore 
			normal operation. Calling IOServiceRequestProbe() makes it appear that the device has been "re-plugged."
        @param self The device interface to use.
		@param inFlags Reserved for future use. Set to NULL.  Description forthcoming?
	*/
	IOReturn			(*Seize)(
								IOFireWireLibDeviceRef 	self,
								IOOptionBits			inFlags,
								... ) ;

	/*!
		@function FireLog
		@abstract Logs string to in-kernel debug buffer
        @param self The device interface to use.
	*/
	IOReturn			(*FireLog)(
								IOFireWireLibDeviceRef 	self,
								const char*				format,
								... ) ;

	/*!	@function GetBusCycleTime
		@abstract Get bus and cycle time.
		@param self The device interface to use.
		@param outBusTime A pointer to a UInt32 to hold the bus time
		@param outCycleTime A pointer to a UInt32 to hold the cycle time
		@result An IOReturn error code.	*/	
	IOReturn (*GetBusCycleTime)( IOFireWireLibDeviceRef  self, UInt32*  outBusTime, UInt32*  outCycleTime) ;

	//
	// v4
	//

	/*!
		@function CreateCompareSwapCommand64
		@abstract Create a quadlet compare/swap command object and initialize it with 64-bit values.
		@param self The device interface to use.
		@param device The service (representing an attached FireWire device) to which to write.
			For 48-bit, device relative addressing, pass the service used to create the device interface. This
			can be obtained by calling GetDevice(). For 64-bit absolute addressing, pass 0. Other values are
			unsupported. Setting the callback value to nil defaults to synchronous execution.
		@param addr Command target address
		@param cmpVal 64-bit value expected at target address
		@param newVal 64-bit value to be set at target address
		@param callback Command completion callback.
		@param failOnReset Pass true if the command should only be executed during the FireWire bus generation
			specified in 'generation'. Pass false to ignore the generation parameter. The generation can be
			obtained by calling GetBusGeneration()
		@param generation The FireWire bus generation during which the command should be executed. Ignored
			if failOnReset is false.
		@result An IOFireWireLibCommandRef interface. See IOFireWireLibCommandRef.	*/
	IOFireWireLibCommandRef (*CreateCompareSwapCommand64)( IOFireWireLibDeviceRef self, io_object_t device, const FWAddress* addr, 
																UInt64 cmpVal, UInt64 newVal, IOFireWireLibCommandCallback callback, 
																Boolean failOnReset, UInt32 generation, void* inRefCon, REFIID iid) ;

	/*!
		@function CompareSwap64
		@abstract Perform synchronous lock operation
		@param self The device interface to use.
		@param device The service (representing an attached FireWire device) to which to write.
			For 48-bit, device relative addressing, pass the service used to create the device interface. This
			can be obtained by calling GetDevice(). For 64-bit absolute addressing, pass 0. Other values are
			unsupported.
			
			If the quadlets stored at 'oldVal' match those passed to 'expectedVal', the lock operation was
			successful.
		@param addr Command target address
		@param expectedVal Pointer to quadlets expected at target.
		@param newVal Pointer to quadlets to atomically set at target if compare is successful.
		@param oldVal Pointer to quadlets to hold value found at target address after transaction if completed.
		@param size Size in bytes of compare swap transaction to perform. Value values are 4 and 8.
		@param failOnReset Pass true if the command should only be executed during the FireWire bus generation
			specified in 'generation'. Pass false to ignore the generation parameter. The generation can be
			obtained by calling GetBusGeneration()
		@param generation The FireWire bus generation during which the command should be executed. Ignored
			if failOnReset is false.
		@result An IOReturn error code
	*/
	IOReturn (*CompareSwap64)( IOFireWireLibDeviceRef self, io_object_t device, const FWAddress* addr, 
										UInt32* expectedVal, UInt32* newVal, UInt32* oldVal, IOByteCount size, 
										Boolean failOnReset, UInt32 generation) ;

	/*!	@function GetBusGeneration
		@abstract Get bus generation number.
		@discussion The bus generation number stays constant between bus resets and can be
			used in combination with a FireWire node ID to uniquely identify nodes on the bus.
			Pass the generation number to functions that take or return FireWire node IDs.
		
			Availability: IOFireWireDeviceInterface_v4 and newer
			
		@param self The device interface to use.
		@param outGeneration A pointer to a UInt32 to hold the bus generation  number
		@result Returns kIOReturnSuccess if a valid bus generation has been returned in 'outGeneration'.*/	
	IOReturn (*GetBusGeneration)( IOFireWireLibDeviceRef self, UInt32* outGeneration ) ;

	/*!	@function GetLocalNodeIDWithGeneration
		@abstract Get node ID of local machine.
		@discussion Use this function instead of GetLocalNodeID().
		
			Availability: IOFireWireDeviceInterface_v4 and newer
			
		@param self The device interface to use.
		@param checkGeneration A bus generation number obtained from GetBusGeneration()
		@param outLocalNodeID A pointer to a UInt16 to hold the node ID of the local machine.
		@result Returns kIOReturnSuccess if a valid nodeID has been returned in 'outLocalNodeID'. Returns
			kIOFireWireBusReset if 'checkGeneration' does not match the current bus generation number.*/
	IOReturn (*GetLocalNodeIDWithGeneration)( IOFireWireLibDeviceRef self, UInt32 checkGeneration, UInt16* outLocalNodeID ) ;

	/*!	@function GetRemoteNodeID
		@abstract Get node ID of device to which this interface is attached.
		@discussion
		
			Availability: IOFireWireDeviceInterface_v4 and newer
			
		@param self The device interface to use.
		@param checkGeneration A bus generation number obtained from GetBusGeneration()
		@param outRemoteNodeID A pointer to a UInt16 to hold the node ID of the remote device.
		@result Returns kIOReturnSuccess if a valid nodeID has been returned in 'outRemoteNodeID'. Returns
			kIOFireWireBusReset if 'checkGeneration' does not match the current bus generation number.*/
	IOReturn (*GetRemoteNodeID)( IOFireWireLibDeviceRef self, UInt32 checkGeneration, UInt16* outRemoteNodeID ) ;

	/*!	@function GetSpeedToNode
		@abstract Get maximum transfer speed to device to which this interface is attached.
		@discussion
		
			Availability: IOFireWireDeviceInterface_v4 and newer
			
		@param self The device interface to use.
		@param checkGeneration A bus generation number obtained from GetBusGeneration()
		@param outSpeed A pointer to an IOFWSpeed to hold the maximum speed to the remote device.
		@result Returns kIOReturnSuccess if a valid speed has been returned in 'outSpeed'. Returns
			kIOFireWireBusReset if 'checkGeneration' does not match the current bus generation number.*/
	IOReturn (*GetSpeedToNode)( IOFireWireLibDeviceRef self, UInt32 checkGeneration, IOFWSpeed* outSpeed) ;

	/*!	@function GetSpeedBetweenNodes
		@abstract Get the maximum transfer speed between nodes 'srcNodeID' and 'destNodeID'.
		@discussion
		
			Availability: IOFireWireDeviceInterface_v4 and newer
			
		@param self The device interface to use.
		@param checkGeneration A bus generation number obtained from GetBusGeneration()
		@param srcNodeID A FireWire node ID.
		@param destNodeID A FireWire node ID.
		@param outSpeed A pointer to an IOFWSpeed to hold the maximum transfer speed between node 'srcNodeID' and 'destNodeID'.
		@result Returns kIOReturnSuccess if a valid speed has been returned in 'outSpeed'. Returns
			kIOFireWireBusReset if 'checkGeneration' does not match the current bus generation number.*/
	IOReturn (*GetSpeedBetweenNodes)( IOFireWireLibDeviceRef self, UInt32 checkGeneration, UInt16 srcNodeID, UInt16 destNodeID,  IOFWSpeed* outSpeed) ;

	//
	// v5
	//

	/*! Description forthcoming */
	IOReturn (*GetIRMNodeID)( IOFireWireLibDeviceRef self, UInt32 checkGeneration, UInt16* outIRMNodeID ) ;
	
	//
	// v6
	//
	
	/*! Description forthcoming */
	IOReturn (*ClipMaxRec2K)( IOFireWireLibDeviceRef self, Boolean clipMaxRec ) ;

	/*! Description forthcoming */
	IOFireWireLibNuDCLPoolRef				(*CreateNuDCLPool)( IOFireWireLibDeviceRef self, UInt32 capacity, REFIID iid ) ;

	//
	// v7
	//
	
	/*! Description forthcoming */
	IOFireWireSessionRef		(*GetSessionRef)( IOFireWireLibDeviceRef self ) ;
	
	//
	// v8
	//
	
	/*!	@function CreateLocalIsochPortWithOptions
		@abstract Create a local isoch port
		@discussion
		
			Same as CreateLocalIsochPort(), above, but allows additional options to be passed.
			Availability: IOFireWireDeviceInterface_v8 and newer
			
		@param options Currently supported options are 'kFWIsochPortUseSeparateKernelThread'. If this
			option is used, a separate kernel thread will be created to handle interrupt
			processing for this port only.
			Pass 'kFWIsochPortDefaultOptions' for no options.
		@result Returns kIOReturnSuccess if a valid speed has been returned in 'outSpeed'. Returns
			kIOFireWireBusReset if 'checkGeneration' does not match the current bus generation number.*/	
	
	IOFireWireLibLocalIsochPortRef (*CreateLocalIsochPortWithOptions)( 
			IOFireWireLibDeviceRef  self, 
			Boolean					inTalking,
			DCLCommand *			dclProgram,
			UInt32					startEvent,
			UInt32					startState,
			UInt32					startMask,
			IOVirtualRange			dclProgramRanges[],	// optional optimization parameters
			UInt32					dclProgramRangeCount,	
			IOVirtualRange			bufferRanges[],
			UInt32					bufferRangeCount,
			IOFWIsochPortOptions	options,
			REFIID 					iid) ; 

	//
	// v9
	//
	
	/*!	@function CreateVectorCommand
		@abstract Create a vector command object.
		@param self The device interface to use.
		@param callback Command completion callback. Setting the callback value to nil defaults to synchronous execution.
		@param inRefCon Reference constant for 3rd party use.
		@result An IOFireWireLibVectorCommandRef interface*/
		IOFireWireLibVectorCommandRef (*CreateVectorCommand)( IOFireWireLibDeviceRef self, IOFireWireLibCommandCallback callback, void* inRefCon, REFIID iid) ;

		/*!	@function AllocateIRMBandwidthInGeneration
			@abstract Attempt to allocate some isochronous bandwidth from the IRM
			@discussion
			
			Attempts to allocates some isochronous bandwidth from the IRM, if the generation matches the current generation.
			Availability: IOFireWireDeviceInterface_v9 and newer
			
			@param bandwidthUnits The number of bandwidth units to allocate
			
			@param generation The bus generation that this allocation attempt is to take place in.
			
			@result Returns kIOReturnSuccess if bandwidth allocation was successful. Returns kIOFireWireBusReset if 
			'generation' does not match the current bus generation number. Returns kIOReturnError for any other
			error (such as the allocation failed) */	
		IOReturn (*AllocateIRMBandwidthInGeneration)(IOFireWireLibDeviceRef self, UInt32 bandwidthUnits, UInt32 generation) ;
		
		/*!	@function ReleaseIRMBandwidthInGeneration
			@abstract Attempt to release some isochronous bandwidth from the IRM
			@discussion
			
			Attempts to release some isochronous bandwidth from the IRM, if the generation matches the current generation.
				Availability: IOFireWireDeviceInterface_v9 and newer
			
			@param bandwidthUnits The number of bandwidth units to release
			
			@param generation The bus generation that this release attempt is to take place in.
			
			@result Returns kIOReturnSuccess if bandwidth release was successful. Returns kIOFireWireBusReset if 
			'generation' does not match the current bus generation number. Returns kIOReturnError for any other
			error (such as the allocation failed) */	
		IOReturn (*ReleaseIRMBandwidthInGeneration)(IOFireWireLibDeviceRef self, UInt32 bandwidthUnits, UInt32 generation) ;
		
		/*!	@function AllocateIRMChannelInGeneration
			@abstract Attempt to allocate an isochronous channel from the IRM
			@discussion
			
			Attempts to allocates an isochronous channel from the IRM, if the generation matches the current generation.
			Availability: IOFireWireDeviceInterface_v9 and newer
			
			@param isochChannel The isochronous channel to allocate
			
			@param generation The bus generation that this allocation attempt is to take place in.
			
			@result Returns kIOReturnSuccess if channel allocation was successful. Returns kIOFireWireBusReset if 
			'generation' does not match the current bus generation number. Returns kIOReturnError for any other
			error (such as the allocation failed) */	
		IOReturn (*AllocateIRMChannelInGeneration)(IOFireWireLibDeviceRef self, UInt8 isochChannel, UInt32 generation) ;
		
		/*!	@function ReleaseIRMChannelInGeneration
			@abstract Attempt to release an isochronous channel from the IRM
			@discussion
			
			Attempts to release an isochronous channel from the IRM, if the generation matches the current generation.
			Availability: IOFireWireDeviceInterface_v9 and newer
			
			@param isochChannel The isochronous channel to release
			
			@param generation The bus generation that this release attempt is to take place in.
			
			@result Returns kIOReturnSuccess if channel relase was successful. Returns kIOFireWireBusReset if 
			'generation' does not match the current bus generation number. Returns kIOReturnError for any other
			error (such as the allocation failed) */	
		IOReturn (*ReleaseIRMChannelInGeneration)(IOFireWireLibDeviceRef self, UInt8 isochChannel, UInt32 generation) ;
		
		/*!	@function CreateIRMAllocation
			@abstract Attempt to create an IRM allocation that persists accross bus-resets.
			@discussion
			
			Create an IOFireWireIRMAllocation object, which can be used to allocate IRM resources, and will reallocate automatically after bus-resets (if possible).
			
			Availability: IOFireWireDeviceInterface_v9 and newer
			
			@param releaseIRMResourcesOnFree Specify whether or not the IRM resources shall be released when the IOFireWireLibIRMAllocation is destroyed. Can be overrided later.
			
			@param callback The handler to notify clients of failure to reclaim IRM resources after bus-reset.
			
			@param pLostNotificationProcRefCon The refCon passed with the callback.
			
			@result Returns a pointer to the newly created IRM allocation object, if successful, NULL otherwise.
			*/	
		IOFireWireLibIRMAllocationRef (*CreateIRMAllocation)( IOFireWireLibDeviceRef	self, 
															  Boolean					releaseIRMResourcesOnFree, 
															  IOFireWireLibIRMAllocationLostNotificationProc callback,
															  void						*pLostNotificationProcRefCon,
															  REFIID					iid) ; 

	/*!	@function CreateAsyncStreamListener
		@abstract Creates a async stream listener object and returns an interface to it.
		@param self		The device interface to use.
		@param channel	The channel to allocate.
		@param inRefCon A user specified reference value. This will be passed to all callback functions.
		@param inQueueBufferSize The size of the queue which receives packets from the bus before they are handed to
			the client. A larger queue can help eliminate dropped packets
			when receiving large bursts of data. When a packet is received which can not fit into the queue, 
			the packet dropped callback will be called. 
		@param iid An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the
			type of interface to be returned for the created ayns stream listener object.
		@result An IOFWAsyncStreamListenerInterfaceRef. Returns 0 upon failure */
		IOFWAsyncStreamListenerInterfaceRef	(*CreateAsyncStreamListener)(  IOFireWireLibDeviceRef			self, 
																			UInt32							channel,
																			IOFWAsyncStreamListenerHandler	callback,																			
																			void*							inRefCon, 
																			UInt32							inQueueBufferSize, 
																			REFIID							iid) ;


		/*!	@function GetIsochAsyncPort
			@abstract Returns the notification port used for async and isoch callbacks
			@discussion
			
			If necessary GetIsochAsyncPort will allocate the port. 
			
			Availability: IOFireWireDeviceInterface_v9 and newer
			
			@param self		The device interface to use.
			@result Returns the mach_port used for notifications */	

		mach_port_t (*GetIsochAsyncPort)( IOFireWireLibDeviceRef self ); 
															  
	/*!
		@function CreatePHYCommand
		@abstract Create a command object for sending a PHY packet
		@param self The device interface to use.
		@param data1 phy packet quadlet 1
		@param data2 phy packet quadlet 1
		@param callback completion callback
		@param failOnReset Pass true if the command should only be executed during the FireWire bus generation
			specified in 'generation'. Pass false to ignore the generation parameter. The generation can be
			obtained by calling GetBusGeneration()
		@param generation The FireWire bus generation during which the command should be executed. Ignored
			if failOnReset is false.
		@param iid An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the
			type of interface to be returned for the created phy command object.
		@result An IOReturn error code
	*/
		
		IOFireWireLibCommandRef		(*CreatePHYCommand)( IOFireWireLibDeviceRef			self,
														UInt32							data1,
														UInt32							data2,
														IOFireWireLibCommandCallback	callback,
														Boolean							failOnReset, 
														UInt32							generation,
														void*							inRefCon, 
														REFIID							iid );
														
	/*!
		@function CreatePHYPacketListener
		@abstract Create a listener object for receiving PHY packets
		@param self The device interface to use.
		@param queueCount The maximum queue size to use to buffer phy packets between kernel and user space
		@param iid An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the
			type of interface to be returned for the created phy packet listener object.
		@result An IOReturn error code
	*/
		
		IOFireWireLibPHYPacketListenerRef	(*CreatePHYPacketListener)( IOFireWireLibDeviceRef			self,
																		UInt32							queueCount,
																		REFIID							iid );														
																		

		/*!
			@function CreateAsyncStreamCommand
			@abstract Create a command object for sending Async Stream packets
			@param self			The device interface to use.
			@param channel		The channel number to use.
			@param buf			A pointer to the buffer containing the data to be written
			@param size			Number of bytes to write
			@param callback		Command completion callback.
			@param failOnReset	Pass true if the command should only be executed during the FireWire bus generation
								specified in 'generation'. Pass false to ignore the generation parameter. The generation can be
								obtained by calling GetBusGeneration(). Must be 'true' when using 64-bit addressing.
			@param inRefCon		A user specified reference value. This will be passed to all callback functions.
			@param iid			An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the
								type of interface to be returned for the created phy packet listener object.
			@result An IOReturn error code
		*/
		IOFireWireLibCommandRef	(*CreateAsyncStreamCommand)(IOFireWireLibDeviceRef				self, 
																UInt32							channel,
																UInt32							sync,
																UInt32							tag,
																void*							buf, 
																UInt32							size,
																IOFireWireLibCommandCallback	callback, 
																Boolean							failOnReset,
																UInt32							generation,
																void*							inRefCon,
																REFIID							iid);
	
		/*!	@function GetCycleTimeAndUpTime
			@abstract Get bus cycle time and cpu uptime.
			@param self The device interface to use.
			@param outCycleTime A pointer to a UInt32 to hold the result
			@param outUpTime A pointer to a UInt64 to hold the result
			@result An IOReturn error code.	*/	
		IOReturn (*GetCycleTimeAndUpTime)( IOFireWireLibDeviceRef  self, UInt32*  outCycleTime, UInt64*  outUpTime) ;
					
} IOFireWireDeviceInterface, IOFireWireUnitInterface, IOFireWireNubInterface ;
#endif // ifdef KERNEL

#pragma mark -
#pragma mark PSEUDO ADDRESS SPACE
// ============================================================
// IOFireWirePseudoAddressSpaceInterface
// ============================================================

// creation flags
/*! FireWire address space creation flags */
typedef enum
{
	kFWAddressSpaceNoFlags			= 0,
	kFWAddressSpaceNoWriteAccess 	= (1 << 0) ,
	kFWAddressSpaceNoReadAccess 	= (1 << 1) ,
	kFWAddressSpaceAutoWriteReply	= (1 << 2) ,
	kFWAddressSpaceAutoReadReply	= (1 << 3) ,
	kFWAddressSpaceAutoCopyOnWrite	= (1 << 4) ,
	kFWAddressSpaceShareIfExists	= (1 << 5) ,
	kFWAddressSpaceExclusive		= (1 << 6)
} FWAddressSpaceFlags ;

#ifndef KERNEL
/*!	@class
	@discussion Represents and provides management functions for a pseudo address 
		space (software-backed) in the local machine.

		Pseudo address space objects can be created using IOFireWireDeviceInterface.*/
typedef struct IOFireWirePseudoAddressSpaceInterface_t
{
	IUNKNOWN_C_GUTS ;

	/*! Interface version */
	UInt32 version;

	/*! Interface revision */
	UInt32 revision;

	/*!	@function SetWriteHandler
		@abstract Set the callback that should be called to handle write accesses to
			the corresponding address space
		@param self The address space interface to use.
		@param inWriter The callback to set.
		@result Returns the callback that was previously set or nil for none.*/
	const IOFireWirePseudoAddressSpaceWriteHandler (*SetWriteHandler)( IOFireWireLibPseudoAddressSpaceRef self, IOFireWirePseudoAddressSpaceWriteHandler inWriter) ;

	/*!	@function SetReadHandler
		@abstract Set the callback that should be called to handle read accesses to
			the corresponding address space
		@param self The address space interface to use.
		@param inReader The callback to set.
		@result Returns the callback that was previously set or nil for none.*/
	const IOFireWirePseudoAddressSpaceReadHandler (*SetReadHandler)( IOFireWireLibPseudoAddressSpaceRef self, IOFireWirePseudoAddressSpaceReadHandler inReader) ;

	/*!	@function SetSkippedPacketHandler
		@abstract Set the callback that should be called when incoming packets are
			dropped by the address space.
		@param self The address space interface to use.
		@param inHandler The callback to set.
		@result Returns the callback that was previously set or nil for none.*/
	const IOFireWirePseudoAddressSpaceSkippedPacketHandler (*SetSkippedPacketHandler)( IOFireWireLibPseudoAddressSpaceRef self, IOFireWirePseudoAddressSpaceSkippedPacketHandler inHandler) ;

	/*!	@function NotificationIsOn
		@abstract Is notification on?
		@param self The address space interface to use.
		@result Returns true if packet notifications for this address space are active */
	Boolean (*NotificationIsOn)(IOFireWireLibPseudoAddressSpaceRef self) ;

	/*!	@function TurnOnNotification
		@abstract Try to turn on packet notifications for this address space.
		@param self The address space interface to use.
		@result Returns true upon success */
	Boolean (*TurnOnNotification)(IOFireWireLibPseudoAddressSpaceRef self) ;

	/*!	@function TurnOffNotification
		@abstract Force packet notification off.
		@param self The pseudo address interface to use. */
	void (*TurnOffNotification)(IOFireWireLibPseudoAddressSpaceRef self) ;	

	/*!	@function ClientCommandIsComplete
		@abstract Notify the address space that a packet notification handler has completed.
		@discussion Packet notifications are received one at a time, in order. This function
			must be called after a packet handler has completed its work.
		@param self The address space interface to use.
		@param commandID The ID of the packet notification being completed. This is the same
			ID that was passed when a packet notification handler is called.
		@param status The completion status of the packet handler */
	void		(*ClientCommandIsComplete)(IOFireWireLibPseudoAddressSpaceRef self, FWClientCommandID commandID, IOReturn status) ;

		// --- accessors ----------
	/*!	@function GetFWAddress
		@abstract Get the FireWire address of this address space
		@param self The pseudo address interface to use. */
	void (*GetFWAddress)(IOFireWireLibPseudoAddressSpaceRef self, FWAddress* outAddr) ;

	/*!	@function GetBuffer
		@abstract Get a pointer to the backing store for this address space
		@param self The address space interface to use.
		@result A pointer to the backing store of this pseudo address space. Returns
			nil if none. */
	void* (*GetBuffer)(IOFireWireLibPseudoAddressSpaceRef self) ;

	/*!	@function GetBufferSize
		@abstract Get the size in bytes of this address space.
		@param self The address space interface to use.
		@result Size of the pseudo address space in bytes. Returns 0 for none.*/
	const UInt32 (*GetBufferSize)(IOFireWireLibPseudoAddressSpaceRef self) ;

	/*!	@function GetRefCon
		@abstract Returns the user refCon value for this address space.
		@param self The address space interface to use.
		@result Size of the pseudo address space in bytes. Returns 0 for none.*/
	void* (*GetRefCon)(IOFireWireLibPseudoAddressSpaceRef self) ;


} IOFireWirePseudoAddressSpaceInterface ;

#pragma mark -
#pragma mark IRM ALLOCATION
// ============================================================
// IOFireWireLibIRMAllocationInterface
// ============================================================
/*!	@class
		Description forthcoming 
 */
typedef struct IOFireWireLibIRMAllocationInterface_t
{
	IUNKNOWN_C_GUTS ;
	UInt32 version, revision ;

	/*!	@function setReleaseIRMResourcesOnFree
	 @abstract Set a new value for releaseIRMResourcesOnFree
	 @param self The IRMAllocation interface to use.
	 @param doRelease The new value for releaseIRMResourcesOnFree. */
	const void (*setReleaseIRMResourcesOnFree)( IOFireWireLibIRMAllocationRef self, Boolean doRelease) ;
	
	/*!	@function allocateIsochResources
	 @abstract Use this interface to allocate isochronous resources
	 @param self The IRMAllocation interface to use.
	 @param isochChannel The isoch channel to allocate.
	 @param bandwidthUnits The bandwidth units to allocate.
	 @result Returns true if allocation success */
	IOReturn (*allocateIsochResources)(IOFireWireLibIRMAllocationRef self, UInt8 isochChannel, UInt32 bandwidthUnits);
	
	/*!	@function deallocateIsochResources
	 @abstract Deallocate previously allocated resources
	 @param self The IRMAllocation interface to use.
	 @result Returns true if deallocation success */
	IOReturn (*deallocateIsochResources)(IOFireWireLibIRMAllocationRef self);

	/*!	@function areIsochResourcesAllocated
	 @abstract Poll to see if IRM resources are still allocated
	 @param self The IRMAllocation interface to use.
	 @param pAllocatedIsochChannel If allocated, the channel
	 @param pAllocatedBandwidthUnits If allocated, the amount of bandwidth
	 @result Returns true if currently allocated, false otherwise */
	Boolean (*areIsochResourcesAllocated)(IOFireWireLibIRMAllocationRef self, UInt8 *pAllocatedIsochChannel, UInt32 *pAllocatedBandwidthUnits);
	
	/*!	@function NotificationIsOn
	 @abstract Is notification on?
	 @param self The IRMAllocation interface to use.
	 @result Returns true if notifications for this IRMAllocation are enabled */
	Boolean (*NotificationIsOn)(IOFireWireLibIRMAllocationRef self) ;
	
	/*!	@function TurnOnNotification
	 @abstract Try to turn on notifications
	 @param self The IRMAllocation interface to use.
	 @result Returns true upon success */
	Boolean (*TurnOnNotification)(IOFireWireLibIRMAllocationRef self) ;
	
	/*!	@function TurnOffNotification
	 @abstract Force notification off.
	 @param self The IRMAllocation interface to use. */
	void (*TurnOffNotification)(IOFireWireLibIRMAllocationRef self) ;
	
	/*!	@function SetRefCon
	 @abstract Set a new refcon
	 @param self The IRMAllocation interface to use.
	 @param refCon The new refcon value. */
	void (*SetRefCon)(IOFireWireLibIRMAllocationRef self, void* refCon) ;

	/*!	@function GetRefCon
	 @abstract Get the current refcon
	 @param self The IRMAllocation interface to use.
	 @result Returns the current refcon value */
	void* (*GetRefCon)(IOFireWireLibIRMAllocationRef self) ;
}IOFireWireLibIRMAllocationInterface;

#pragma mark -
#pragma mark LOCAL UNIT DIRECTORY INTERFACE
// ============================================================
//
// IOFireWireLocalUnitDirectoryInterface
//
// ============================================================

/*!	@class
	@discussion Allows creation and management of unit directories in the config
		ROM of the local machine. After the unit directory has been built, 
		Publish() should be called to cause it to appear in the config ROM.
		Unpublish() has the reverse effect as Publish().

		This interface can be created using IOFireWireDeviceInterface::CreateLocalUnitDirectory. */
typedef struct IOFireWireLocalUnitDirectoryInterface_t
{
	IUNKNOWN_C_GUTS ;

	/*! Interface version */
	UInt32 version;

	/*! Interface revision */
	UInt32 revision ;

	// --- adding to ROM -------------------
	/*!	@function AddEntry_Ptr
		@abstract Append a data leaf
		@discussion Appends a leaf data node to a unit directory
		@param self The local unit directory interface to use.
		@param key The config ROM key for the data to be added.
		@param inBuffer A pointer to the data to be placed in the added leaf.
		@param inLen Length of the data being added.
		@param inDesc Reserved; set to NULL.  */
	IOReturn			(*AddEntry_Ptr)(IOFireWireLibLocalUnitDirectoryRef self, int key, void* inBuffer, size_t inLen, CFStringRef inDesc) ;

	/*!	@function AddEntry_UInt32
		@abstract Append an immediate leaf
		@discussion Appends an immediate leaf to a unit directory. Note that only the lower 3 bytes
			of the passed in value can appear in the unit directory.
		@param self The local unit directory interface to use.
		@param key The config ROM key for the data to be added.
		@param value The value to be added.
		@param inDesc Reserved; set to NULL.  */
	IOReturn			(*AddEntry_UInt32)(IOFireWireLibLocalUnitDirectoryRef self, int key, UInt32 value, CFStringRef inDesc) ;

	/*!	@function AddEntry_FWAddress
		@abstract Append an offset leaf
		@discussion Appends an offset leaf to a unit directory. The address passed in value should be an
			address in initial unit space of the local config ROM.
		@param self The local unit directory interface to use.
		@param key The config ROM key for the data to be added.
		@param value A pointer to a FireWire address.
		@param inDesc Reserved; set to NULL.  */
	IOReturn			(*AddEntry_FWAddress)(IOFireWireLibLocalUnitDirectoryRef self, int key, const FWAddress* value, CFStringRef inDesc) ;

	// Use this function to cause your unit directory to appear in the Mac's config ROM.
	/*!	@function Publish
		@abstract Causes a constructed or updated unit directory to appear in the local machine's
			config ROM. Note that this call will cause a bus reset, after which the unit directory will
			be visible to devices on the bus.
		@param self The local unit directory interface to use. */
	IOReturn			(*Publish)(IOFireWireLibLocalUnitDirectoryRef self) ;

	/*!	@function Unpublish
		@abstract Has the opposite effect from Publish(). This call removes a unit directory from the 
			local machine's config ROM. Note that this call will cause a bus reset, after which the unit directory will
			no longer appear to devices on the bus.
		@param self The local unit directory interface to use. */
	IOReturn			(*Unpublish)(IOFireWireLibLocalUnitDirectoryRef self) ;
} IOFireWireLocalUnitDirectoryInterface ;

#pragma mark -
#pragma mark PHYSICAL ADDRESS SPACE INTERFACE
// ============================================================
//
// IOFireWireLibPhysicalAddressSpaceInterface
//
// ============================================================

/*!	@class
	@abstract IOFireWireLib physical address space object. ( interface name: IOFireWirePhysicalAddressSpaceInterface )
	@discussion Represents and provides management functions for a physical address 
		space (hardware-backed) in the local machine.<br>
		Physical address space objects can be created using IOFireWireDeviceInterface.*/
typedef struct IOFireWirePhysicalAddressSpaceInterface_t
{
	IUNKNOWN_C_GUTS ;

	/*! Interface version. */
	UInt32 version;

	/*! Interface revision. */
	UInt32 revision ;
	/*!	@function GetPhysicalSegments
		@abstract Returns the list of physical memory ranges this address space occupies
			on the local machine.
		@param self The address space interface to use.
		@param ioSegmentCount Pass in a pointer to the number of list entries in 
			outSegments and outAddress. Upon completion, this will contain the actual
			number of segments returned in outSegments and outAddress
		@param outSegments A pointer to an array to hold the function results. Upon
			completion, this will contain the lengths of the physical segments this
			address space occupies on the local machine
		@param outAddresses A pointer to an array to hold the function results. Upon
			completion, this will contain the addresses of the physical segments this
			address space occupies on the local machine. If NULL, ioSegmentCount
			will contain the number of physical segments in the address space.*/
			
	void				(*GetPhysicalSegments)(
								IOFireWireLibPhysicalAddressSpaceRef self,
								UInt32*				ioSegmentCount,
								IOByteCount			outSegments[],
								IOPhysicalAddress	outAddresses[]) ;						
	/*!	@function GetPhysicalSegment
		@abstract Returns the physical segment containing the address at a specified offset
			from the beginning of this address space
		@param self The address space interface to use.
		@param offset Offset from beginning of address space
		@param length Pointer to a value which upon completion will contain the length of
			the segment returned by the function.
		@result The address of the physical segment containing the address at the specified
			offset of the address space	*/
	IOPhysicalAddress	(*GetPhysicalSegment)(
								IOFireWireLibPhysicalAddressSpaceRef self,
								IOByteCount 		offset,
								IOByteCount*		length) ;

	/*!	@function GetPhysicalAddress
		@abstract Returns the physical address of the beginning of this address space
		@param self The address space interface to use.
		@result The physical address of the start of this address space	*/
	IOPhysicalAddress	(*GetPhysicalAddress)(
								IOFireWireLibPhysicalAddressSpaceRef self) ;

		// --- accessors ----------
	/*!	@function GetFWAddress
		@abstract Get the FireWire address of this address space
		@param self The address space interface to use.	*/
	void				(*GetFWAddress)(
								IOFireWireLibPhysicalAddressSpaceRef self, 
								FWAddress* outAddr) ;

	/*!	@function GetBuffer
		@abstract Get a pointer to the backing store for this address space
		@param self The address space interface to use.
		@result A pointer to the backing store of this address space.*/
	void*				(*GetBuffer)(
								IOFireWireLibPhysicalAddressSpaceRef self) ;

	/*!	@function GetBufferSize
		@abstract Get the size in bytes of this address space.
		@param self The address space interface to use.
		@result Size of the pseudo address space in bytes.	*/
	const UInt32		(*GetBufferSize)(
								IOFireWireLibPhysicalAddressSpaceRef self) ;

} IOFireWirePhysicalAddressSpaceInterface ;
#endif // ifndef KERNEL

#pragma mark -
#pragma mark COMMAND OBJECT INTERFACES
// =================================================================
// command objects
// =================================================================

#define kFireWireCommandUserFlagsMask (0x0000FFFF)

// 8 quadlets
#define kFWUserCommandSubmitWithCopyMaxBufferBytes	32

//
// Flags to be set on IOFireWireLib command objects
// Passed to SetFlags()
//
/*! @enum IOFireWireLib Command Flags
    @abstract Flags for IOFireWireLib command objects
    @discussion
	Pass these flags to the object's SetFlags callback.
 */
enum
{
	kFWCommandNoFlags						= 0,
	kFWCommandInterfaceForceNoCopy			= (1 << 0),
	kFWCommandInterfaceForceCopyAlways		= (1 << 1),
	kFWCommandInterfaceSyncExecute			= (1 << 2),
	kFWCommandInterfaceAbsolute				= (1 << 3),
	kFWVectorCommandInterfaceOrdered		= (1 << 4),
	kFWCommandInterfaceForceBlockRequest	= (1 << 5)
} ;

/*! @enum IOFireWireLib failOnReset Flags
    @abstract Flags for IOFireWireLib commands
    @discussion
	Pass these flags in the failOnReset of various commands.
 */
enum
{
	kFWDontFailOnReset = false,
	kFWFailOnReset = true
} ;

/*! @enum IOFireWireLib Additional Command Flags
    @abstract Flags for IOFireWireLib commands
    @discussion
	Pass these flags to the object's SetFlags callback.
 */
enum {
	kFireWireCommandUseCopy				= (1 << 16),
	kFireWireCommandAbsolute			= (1 << 17)
} ;

#ifndef KERNEL
//
// IOFIREWIRELIBCOMMAND_C_GUTS
// Macro used to insert generic superclass function definitions into all subclass of
// IOFireWireCommand. Comments for functions contained in this macro follow below:
//
/*! @parseOnly */
#define IOFIREWIRELIBCOMMAND_C_GUTS \
	/*!	@function GetStatus \
		@abstract Return command completion status. \
		@discussion Availability: (for interfaces obtained with ID) \
		<table border="0" rules="all"> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireCompareSwapCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireAsyncStreamCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
		</table> \
					 \
		@param self The command object interface of interest \
		@result An IOReturn error code indicating the completion error (if any) returned the last \
			time this command object was executed	*/ \
	IOReturn			(*GetStatus)(IOFireWireLibCommandRef	self) ;	\
	/*!	@function GetTransferredBytes \
		@abstract Return number of bytes transferred by this command object when it last completed \
			execution. \
		@discussion Availability: (for interfaces obtained with ID) \
		<table border="0" rules="all"> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireCompareSwapCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireAsyncStreamCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
		</table> \
		@param self The command object interface of interest \
		@result A UInt32 containing the bytes transferred value	*/ \
	UInt32				(*GetTransferredBytes)(IOFireWireLibCommandRef self) ; \
	/*!	@function GetTargetAddress \
		@abstract Get command target address. \
		@discussion Availability: (for interfaces obtained with ID) \
		<table border="0" rules="all"> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireCompareSwapCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireAsyncStreamCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
		</table> \
		@param self The command object interface of interest \
		@param outAddr A pointer to an FWAddress to contain the function result. */ \
	void				(*GetTargetAddress)(IOFireWireLibCommandRef self, FWAddress* outAddr) ; \
	/*!	@function SetTarget \
		@abstract Set command target address \
		@discussion Availability: (for interfaces obtained with ID) \
		<table border="0" rules="all"> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireCompareSwapCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireAsyncStreamCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
		</table> \
		@param self The command object interface of interest \
		@param addr A pointer to an FWAddress. */ \
	void				(*SetTarget)(IOFireWireLibCommandRef self, const FWAddress* addr) ;	\
	/*!	@function SetGeneration \
		@abstract Set FireWire bus generation for which the command object shall be valid. \
			If the failOnReset attribute has been set, the command will only be considered for \
			execution during the bus generation specified by this function. \
		@discussion Availability: (for interfaces obtained with ID) \
		<table border="0" rules="all"> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireCompareSwapCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireAsyncStreamCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
		</table> \
		@param self The command object interface of interest \
		@param generation A bus generation. The current bus generation can be obtained \
			from IOFireWireDeviceInterface::GetBusGeneration().	*/ \
	void				(*SetGeneration)(IOFireWireLibCommandRef self, UInt32 generation) ;	\
	/*!	@function SetCallback \
		@abstract Set the completion handler to be called once the command completes \
			asynchronous execution . \
		@discussion Availability: (for interfaces obtained with ID) \
		<table border="0" rules="all"> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireCompareSwapCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireAsyncStreamCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
		</table> \
		@param self The command object interface of interest \
		@param inCallback A callback handler. Passing nil forces the command object to  \
			execute synchronously. */ \
	void				(*SetCallback)(IOFireWireLibCommandRef self, IOFireWireLibCommandCallback inCallback) ;	\
	/*!	@function SetRefCon \
		@abstract Set the user refCon value. This is the user defined value that will be passed \
			in the refCon argument to the completion function. \
		@discussion Availability: (for interfaces obtained with ID) \
		<table border="0" rules="all"> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireCompareSwapCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireAsyncStreamCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
		</table> */ \
	void				(*SetRefCon)(IOFireWireLibCommandRef self, void* refCon) ;	\
	/*!	@function IsExecuting \
		@abstract Is this command object currently executing? \
		@discussion Availability: (for interfaces obtained with ID) \
		<table border="0" rules="all"> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireCompareSwapCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireAsyncStreamCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
		</table> \
		@param self The command object interface of interest \
		@result Returns true if the command object is executing.	*/ \
	const Boolean		(*IsExecuting)(IOFireWireLibCommandRef self) ;	\
	/*! Description forthcoming */ \
	IOReturn			(*Submit)(IOFireWireLibCommandRef self) ;	\
	/*!	@function Submit \
		@abstract Submit this command object to FireWire for execution. \
		@discussion Availability: (for interfaces obtained with ID) \
		<table border="0" rules="all"> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireCompareSwapCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireAsyncStreamCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
		</table> \
		@param self The command object interface of interest \
		@result An IOReturn result code indicating whether or not the command was successfully \
			submitted */ \
	/*!	@function SubmitWithRefconAndCallback \
		@abstract Set the command refCon value and callback handler, and submit the command \
			to FireWire for execution. \
		@discussion Availability: (for interfaces obtained with ID) \
		<table border="0" rules="all"> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireCompareSwapCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireAsyncStreamCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
		</table> \
		@param self The command object interface of interest \
		@result An IOReturn result code indicating whether or not the command was successfully \
			submitted	*/ \
	IOReturn			(*SubmitWithRefconAndCallback)(IOFireWireLibCommandRef self, void* refCon, IOFireWireLibCommandCallback inCallback) ;\
	/*!	@function Cancel \
		@abstract Cancel command execution \
		@discussion Availability: (for interfaces obtained with ID) \
		<table border="0" rules="all"> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteQuadletCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireCompareSwapCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireAsyncStreamCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
		</table> \
		@param self The command object interface of interest \
		@result An IOReturn result code	*/ \
	IOReturn			(*Cancel)(IOFireWireLibCommandRef self, IOReturn reason)

/*! @parseOnly */
#define IOFIREWIRELIBCOMMAND_C_GUTS_v2	\
	/*!	@function SetBuffer \
		@abstract Set the buffer where read data should be stored. \
		@discussion Availability: (for interfaces obtained with ID) \
		<table border="0" rules="all"> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadQuadletCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteQuadletCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireCompareSwapCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireAsyncStreamCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
		</table> \
		@param self The command object interface of interest \
		@param size Size in bytes of the receive buffer. \
		@param buf A pointer to the receive buffer. */ \
	void 				(*SetBuffer)(IOFireWireLibCommandRef self, UInt32 size, void* buf) ;	\
	/*!	@function GetBuffer \
		@abstract Set the command refCon value and callback handler, and submit the command \
			to FireWire for execution. \
		@discussion Availability: (for interfaces obtained with ID) \
		<table border="0" rules="all"> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadQuadletCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteQuadletCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireCompareSwapCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireAsyncStreamCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
		</table> \
		@param self The command object interface of interest */ \
	void 				(*GetBuffer)(IOFireWireLibCommandRef self, UInt32* outSize, void** outBuf) ;	\
	/*!	@function SetMaxPacket \
		@abstract Set the maximum size in bytes of packets transferred by this command. \
		@discussion Availability: (for interfaces obtained with ID) \
		<table border="0" rules="all"> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadQuadletCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteQuadletCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireCompareSwapCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireAsyncStreamCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
		</table> \
		@param self The command object interface of interest \
		@param maxPacketSize Size in bytes of largest packet that should be transferred \
			by this command. \
		@result An IOReturn result code indicating whether or not the command was successfully \
			submitted	*/ \
	IOReturn			(*SetMaxPacket)(IOFireWireLibCommandRef self, IOByteCount maxPacketSize) ;	\
	/*!	@function SetFlags \
		@abstract Set flags governing this command's execution. \
		@discussion Availability: (for interfaces obtained with ID) \
		<table border="0" rules="all"> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteCommandInterfaceID_v2</code></td> \
				<td>YES</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireReadQuadletCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireWriteQuadletCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireCompareSwapCommandInterfaceID</code></td> \
				<td>NO</td> \
			</tr> \
			<tr> \
				<td width="20%"></td> \
				<td><code>kIOFireWireAsyncStreamCommandInterfaceID</code></td> \
				<td>YES</td> \
			</tr> \
		</table> \
		@param self The command object interface of interest \
		@param inFlags A UInt32 with bits set corresponding to the flags that should be set \
			for this command object. The following values may be used:<br> \
			<ul> \
				<li>kFWCommandNoFlags -- all flags off</li> \
				<li>kFWCommandInterfaceForceNoCopy -- data sent by this command should always be \
					received/sent directly from the buffer set with SetBuffer(). Whatever data \
					is in the buffer when the command is submitted will be used.</li> \
				<li>kFWCommandInterfaceForceCopyAlways -- data will always be copied out of the \
					command object data buffer when SetBuffer() is called, up to a maximum \
					allowed size (kFWUserCommandSubmitWithCopyMaxBufferBytes). This can result \
					in faster data transfer. Changes made to the data buffer contents after \
					calling SetBuffer() will be ignored; SetBuffer() should be called whenever  \
					the data buffer contents change.</li> \
				<li>kFWCommandInterfaceSyncExecute -- Setting this flag causes the command object \
					to execute synchronously. The calling context will block until the command \
					object has completed execution or an error occurs. Using synchronous execution \
					can avoid kernel transitions associated with asynchronous completion and often \
					remove the need for a state machine.</li> \
				<li>kFWCommandInterfaceForceBlockRequest -- Setting this flag causes read and write \
					transactions to use block request packets even if the payload is 4 bytes. If this \
					flag is not set 4 byte transactions will occur using quadlet transactions.</li> \
			</ul>*/ \
	void				(*SetFlags)(IOFireWireLibCommandRef self, UInt32 inFlags)

/*! @parseOnly */
#define IOFIREWIRELIBCOMMAND_C_GUTS_v3 \
	/*!	@function SetTimeoutDuration \
		@abstract Sets the duration of the timeout for this command. \
		@param self A reference to the command \
		@param duration A timeout value in microseconds \
		@result void	*/ \
	void				(*SetTimeoutDuration)(IOFireWireLibCommandRef self, UInt32 duration ); \
	/*!	@function SetMaxRetryCount \
		@abstract Sets the maximum number of retries for this command. \
		@param self A reference to the command \
		@param count The number of retires \
		@result void	*/ \
	void				(*SetMaxRetryCount)(IOFireWireLibCommandRef self, UInt32 count ); \
	/*!	@function GetAckCode \
		@abstract Gets the most recently received ack code for this transaction.  \
		@param self A reference to the command \
		@result The FireWire ack code. */ \
	UInt32				(*GetAckCode)( IOFireWireLibCommandRef self ); \
	/*!	@function GetResponseCode \
		@abstract Gets the most recently received response code for this transaction.  \
		@param self A reference to the command \
		@result The FireWire response code. */ \
	UInt32				(*GetResponseCode)( IOFireWireLibCommandRef self ); \
	/*!	@function SetMaxPacketSpeed \
		@abstract Gets the most recently received ack code for this transaction.  \
		@param self A reference to the command \
		@param speed the desired maximum packet speed \
		@result void */ \
	void				(*SetMaxPacketSpeed)( IOFireWireLibCommandRef self, IOFWSpeed speed ); \
	/*!	@function GetRefCon \
		@abstract Gets the refcon associated with this command \
		@param self A reference to the command \
		@result void */ \
	void *				(*GetRefCon)( IOFireWireLibCommandRef self )
	
/*!	@class
	@abstract IOFireWireLib command object.
	@discussion Represents an object that is configured and submitted to issue synchronous
		and asynchronous bus commands. This is a superclass containing all command object
		functionality not specific to any kind of bus transaction.
		
		Note that data may not always be transferred to or from the data buffer
		for command objects at the time the command is submitted. In some cases the
		transfer may happen as soon as SetBuffer() (below, v2 interfaces and newer) 
		is called. You can use the SetFlags() call (below, v2 interfaces and newer) to 
		control this behavior.

*/
typedef struct IOFireWireCommandInterface_t
{
	
	IUNKNOWN_C_GUTS ;

	/*! Interface version */
	UInt32 version;

	/*! Interface revision */
	UInt32 revision;
	
	/* IMPORTANT: Do not add HeaderDoc markup to the following symbols; they
	   contain HeaderDoc markup, and that would be bad.
	 */

	IOFIREWIRELIBCOMMAND_C_GUTS ;

	// version 2 interfaces: (appear in IOFireWireReadCommandInterface_v2 and IOFireWireWriteCommandInterface_v2)
	IOFIREWIRELIBCOMMAND_C_GUTS_v2 ;
	
	IOFIREWIRELIBCOMMAND_C_GUTS_v3 ;

} IOFireWireCommandInterface ;

/*!	@class
	@abstract IOFireWireLib block read command object.
	@discussion Represents an object that is configured and submitted to issue synchronous
		and asynchronous block read commands.
		
	This interface contains all methods of IOFireWireCommandInterface.
	This interface will contain all v2 methods of IOFireWireCommandInterface
		when instantiated as v2 or newer. */
typedef struct IOFireWireReadCommandInterface_t
{

	IUNKNOWN_C_GUTS ;

	/*! Interface version. */
	UInt32 version;

	/*! Interface revision. */
	UInt32 revision;
	
	IOFIREWIRELIBCOMMAND_C_GUTS ;

	// following functions
	// available only in IOFireWireReadCommandInterface interface v2 and newer
	IOFIREWIRELIBCOMMAND_C_GUTS_v2 ;
	
	IOFIREWIRELIBCOMMAND_C_GUTS_v3 ;
} IOFireWireReadCommandInterface ;

/*!	@class
	@abstract IOFireWireLib block read command object.
	@discussion Represents an object that is configured and submitted to issue synchronous
		and asynchronous block read commands.
		
		This interface contains all methods of IOFireWireCommandInterface.
		This interface will contain all v2 methods of IOFireWireCommandInterface
			when instantiated as v2 or newer. */
typedef struct IOFireWireWriteCommandInterface_t
{
	
	IUNKNOWN_C_GUTS ;

	/*! Interface version. */
	UInt32 version;

	/*! Interface revision. */
	UInt32 revision;
	
	IOFIREWIRELIBCOMMAND_C_GUTS ;

	// following functions
	// available only in IOFireWireWriteCommandInterface_v2 and newer
	IOFIREWIRELIBCOMMAND_C_GUTS_v2 ;
	
	IOFIREWIRELIBCOMMAND_C_GUTS_v3 ;

} IOFireWireWriteCommandInterface ;


/*!	@class
	Description forthcoming 
*/
typedef struct IOFireWireCompareSwapCommandInterface_t
{
	IUNKNOWN_C_GUTS ;

	/*! Interface version. */
	UInt32 version;

	/*! Interface revision. */
	UInt32 revision;
	
	IOFIREWIRELIBCOMMAND_C_GUTS ;

	/*!	@function SetValues
		@abstract Set values for 32-bit compare swap operation. Calling this function will
			make the command object perform 32-bit compare swap transactions on the bus. To perform
			64-bit compare swap operations, use the SetValues64() call, below.
		@discussion Available in v2 and newer.
		@param self The command object interface of interest
		@param cmpVal The value expected at the address targeted by this command object
		@param newVal The value to be written at the address targeted by this command object */
	void	(*SetValues)(IOFireWireLibCompareSwapCommandRef self, UInt32 cmpVal, UInt32 newVal) ;
	
	// --- v2 ---
	
	/*!	@function SetValues64
		@abstract Set values for 64-bit compare swap operation. Calling this function will
			make the command object perform 64-bit compare swap transactions on the bus. To perform
			32-bit compare swap operations, use the SetValues() call, above.
		@discussion Available in v2 and newer.
		@param self The command object interface of interest
		@param cmpVal The value expected at the address targeted by this command object
		@param newVal The value to be written at the address targeted by this command object */
	void	(*SetValues64)(IOFireWireLibCompareSwapCommandRef self, UInt64 cmpVal, UInt64 newVal) ;

	/*!	@function DidLock
		@abstract Was the last lock operation successful?
		@discussion Available in v2 and newer.
		@param self The command object interface of interest
		@result Returns true if the last lock operation performed by this command object was successful,
			false otherwise. */
	Boolean	(*DidLock)(IOFireWireLibCompareSwapCommandRef self) ;

	/*!	@function Locked
		@abstract Get the 32-bit value returned on the last compare swap operation.
		@discussion Available in v2 and newer.
		@param self The command object interface of interest
		@param oldValue A pointer to contain the value returned by the target of this command
			on the last compare swap operation
		@result Returns kIOReturnBadArgument if the last compare swap operation performed was 64-bit. */
	IOReturn (*Locked)(IOFireWireLibCompareSwapCommandRef self, UInt32* oldValue) ;

	/*!	@function Locked64
		@abstract Get the 64-bit value returned on the last compare swap operation.
		@discussion Available in v2 and newer.
		@param self The command object interface of interest
		@param oldValue A pointer to contain the value returned by the target of this command
			on the last compare swap operation
		@result Returns kIOReturnBadArgument if the last compare swap performed was 32-bit. */
	IOReturn (*Locked64)(IOFireWireLibCompareSwapCommandRef self, UInt64* oldValue) ;

	/*!	@function SetFlags
		@abstract Set flags governing this command's execution.
		@discussion Available in v2 and newer. Same as SetFlags() above.
		@param self The command object interface of interest.
		@param inFlags A UInt32 with bits set corresponding to the flags that should be set. */
	void (*SetFlags)(IOFireWireLibCompareSwapCommandRef self, UInt32 inFlags) ;
	
} IOFireWireCompareSwapCommandInterface ;

/*!	@class
	Description forthcoming 
*/
typedef struct IOFireWireCompareSwapCommandInterface_v3_t
{

	IUNKNOWN_C_GUTS ;

	/*! Interface version. */
	UInt32 version;

	/*! Interface version. */
	UInt32 revision;
	
	IOFIREWIRELIBCOMMAND_C_GUTS;

	IOFIREWIRELIBCOMMAND_C_GUTS_v2;

	IOFIREWIRELIBCOMMAND_C_GUTS_v3;

	/*!	@function SetValues
		@abstract Set values for 32-bit compare swap operation. Calling this function will
			make the command object perform 32-bit compare swap transactions on the bus. To perform
			64-bit compare swap operations, use the SetValues64() call, below.
		@discussion Available in v2 and newer.
		@param self The command object interface of interest
		@param cmpVal The value expected at the address targeted by this command object
		@param newVal The value to be written at the address targeted by this command object */
	void	(*SetValues)(IOFireWireLibCompareSwapCommandV3Ref self, UInt32 cmpVal, UInt32 newVal) ;
	
	// --- v2 ---
	
	/*!	@function SetValues64
		@abstract Set values for 64-bit compare swap operation. Calling this function will
			make the command object perform 64-bit compare swap transactions on the bus. To perform
			32-bit compare swap operations, use the SetValues() call, above.
		@discussion Available in v2 and newer.
		@param self The command object interface of interest
		@param cmpVal The value expected at the address targeted by this command object
		@param newVal The value to be written at the address targeted by this command object */
	void	(*SetValues64)(IOFireWireLibCompareSwapCommandV3Ref self, UInt64 cmpVal, UInt64 newVal) ;

	/*!	@function DidLock
		@abstract Was the last lock operation successful?
		@discussion Available in v2 and newer.
		@param self The command object interface of interest
		@result Returns true if the last lock operation performed by this command object was successful,
			false otherwise. */
	Boolean	(*DidLock)(IOFireWireLibCompareSwapCommandV3Ref self) ;

	/*!	@function Locked
		@abstract Get the 32-bit value returned on the last compare swap operation.
		@discussion Available in v2 and newer.
		@param self The command object interface of interest
		@param oldValue A pointer to contain the value returned by the target of this command
			on the last compare swap operation
		@result Returns kIOReturnBadArgument if the last compare swap operation performed was 64-bit. */
	IOReturn (*Locked)(IOFireWireLibCompareSwapCommandV3Ref self, UInt32* oldValue) ;

	/*!	@function Locked64
		@abstract Get the 64-bit value returned on the last compare swap operation.
		@discussion Available in v2 and newer.
		@param self The command object interface of interest
		@param oldValue A pointer to contain the value returned by the target of this command
			on the last compare swap operation
		@result Returns kIOReturnBadArgument if the last compare swap performed was 32-bit. */
	IOReturn (*Locked64)(IOFireWireLibCompareSwapCommandV3Ref self, UInt64* oldValue) ;
	
} IOFireWireCompareSwapCommandInterface_v3 ;

/*!	@class
	Description forthcoming 
*/
typedef struct IOFireWirePHYCommandInterface_t
{
	IUNKNOWN_C_GUTS ;

	/*! Interface version. */
	UInt32 version;

	/*! Interface revision. */
	UInt32 revision;
	
	IOFIREWIRELIBCOMMAND_C_GUTS;

	IOFIREWIRELIBCOMMAND_C_GUTS_v2;

	IOFIREWIRELIBCOMMAND_C_GUTS_v3;

	/*!	@function SetDataQuads
		@abstract Set the 2 quadlets of data to be sent in a PHY packet.
		@discussion Available in v1 and newer.
		@param self The command object interface of interest
		@param data1 The value of the first quad of the phy packet
		@param data2 The value of the second quad of the phy packet */
	void	(*SetDataQuads)( IOFireWireLibPHYCommandRef self, UInt32 data1, UInt32 data2 );
		
} IOFireWirePHYCommandInterface;

/*!	@class
	Description forthcoming 
*/
typedef struct IOFireWireAsyncStreamCommandInterface_t
{
	IUNKNOWN_C_GUTS ;

	/*! Interface version. */
	UInt32 version;

	/*! Interface revision. */
	UInt32 revision;
	
	IOFIREWIRELIBCOMMAND_C_GUTS;

	IOFIREWIRELIBCOMMAND_C_GUTS_v2;

	IOFIREWIRELIBCOMMAND_C_GUTS_v3;
	
	/*!	@function SetChannel
	@abstract Set the new channel to transmit the AsyncStream command.
	@discussion Available in v1 and newer.
	@param self The command object interface of interest
	@param channel The channel for AsyncStream command transmit.*/
	void	(*SetChannel)( IOFireWireLibAsyncStreamCommandRef self, UInt32 channel );

	/*!	@function SetSyncBits
		@abstract Set the sync bits for the AsynStream packets.
		@discussion Available in v1 and newer.
		@param self The command object interface of interest
		@param sync The value for sync bits in the AsyncStream packet */
	void	(*SetSyncBits)( IOFireWireLibAsyncStreamCommandRef self, UInt16 sync );

	/*!	@function SetTagBits
		@abstract Set the tag bits for the AsynStream packets.
		@discussion Available in v1 and newer.
		@param self The command object interface of interest
		@param tag The value for tag bits in the AsyncStream packet */
	void	(*SetTagBits)( IOFireWireLibAsyncStreamCommandRef self, UInt16 tag );
		
} IOFireWireAsyncStreamCommandInterface;

/*!	@class
	@abstract IOFireWireReadQuadletCommandInterface -- IOFireWireLib quadlet read command object.
	@discussion Obsolete; do not use. Use IOFireWireReadCommandInterface v2 or newer
		and its function SetMaxPacket() */
typedef struct IOFireWireReadQuadletCommandInterface_t
{
	IUNKNOWN_C_GUTS ;

	/*! Interface version. */
	UInt32 version;

	/*! Interface revision. */
	UInt32 revision;
	
	IOFIREWIRELIBCOMMAND_C_GUTS ;

	/*!	@function SetQuads
		@abstract Set destination for read data
		@param self The command object interface of interest
		@param inQuads An array of quadlets
		@param inNumQuads Number of quadlet in 'inQuads'	*/
	void (*SetQuads)(IOFireWireLibReadQuadletCommandRef self, UInt32 inQuads[], UInt32 inNumQuads) ;
} IOFireWireReadQuadletCommandInterface ;

/*!	@class
	@abstract IOFireWireLib quadlet read command object.
	@discussion Obsolete; do not use. Use IOFireWireWriteCommandInterface v2 or newer
		and its function SetMaxPacket() */
typedef struct IOFireWireWriteQuadletCommandInterface_t
{
	IUNKNOWN_C_GUTS ;

	/*! Interface version. */
	UInt32 version;

	/*! Interface revision. */
	UInt32 revision;
	
	IOFIREWIRELIBCOMMAND_C_GUTS ;

	/*! IOFireWireLibWriteQuadletCommandRef */
	void (*SetQuads)(IOFireWireLibWriteQuadletCommandRef self, UInt32 inQuads[], UInt32 inNumQuads) ;
	
} IOFireWireWriteQuadletCommandInterface ;

#pragma mark -
#pragma mark CONFIG DIRECTORY INTERFACE
// ============================================================
// IOFireWireConfigDirectoryInterface
// ============================================================


/*!	@class
	@abstract IOFireWireLib device config ROM browsing interface
	@discussion Represents an interface to the config ROM of a remote device. You can use the
		methods of this interface to browser the ROM and obtain key values. You can also
		create additional IOFireWireConfigDirectoryInterface's to represent subdirectories
		within the ROM.*/
typedef struct IOFireWireConfigDirectoryInterface_t
{

	IUNKNOWN_C_GUTS ;

	/*! Interface version. */
	UInt32 version;

	/*! Interface revision. */
	UInt32 revision ;
	
	/*!	@function Update
		@abstract Causes the ROM data to be updated through the specified byte offset. This
			function should not be called in normal usage.
		@param self The config directory interface of interest
		@param inOffset Offset in bytes indicating length of ROM to be updated.
		@result An IOReturn result code	*/
	IOReturn (*Update)							( IOFireWireLibConfigDirectoryRef self, UInt32 inOffset) ;

    /*! Description forthcoming */
    IOReturn (*GetKeyType)						( IOFireWireLibConfigDirectoryRef self, int inKey, IOConfigKeyType* outType);
    /*! Description forthcoming */
    IOReturn (*GetKeyValue_UInt32)				( IOFireWireLibConfigDirectoryRef self, int inKey, UInt32* outValue, CFStringRef* outText);
    /*! Description forthcoming */
    IOReturn (*GetKeyValue_Data)				( IOFireWireLibConfigDirectoryRef self, int inKey, CFDataRef* outValue, CFStringRef* outText);
    /*! Description forthcoming */
    IOReturn (*GetKeyValue_ConfigDirectory)		( IOFireWireLibConfigDirectoryRef self, int inKey, IOFireWireLibConfigDirectoryRef* outValue, REFIID iid, CFStringRef* outText);
    /*! Description forthcoming */
    IOReturn (*GetKeyOffset_FWAddress)			( IOFireWireLibConfigDirectoryRef self, int inKey, FWAddress* outValue, CFStringRef* text);
    /*! Description forthcoming */
    IOReturn (*GetIndexType)					( IOFireWireLibConfigDirectoryRef self, int inIndex, IOConfigKeyType* 	type);
    /*! Description forthcoming */
    IOReturn (*GetIndexKey)						( IOFireWireLibConfigDirectoryRef self, int inIndex, int * key);
    /*! Description forthcoming */
    IOReturn (*GetIndexValue_UInt32)			( IOFireWireLibConfigDirectoryRef self, int inIndex, UInt32 * value);
    /*! Description forthcoming */
    IOReturn (*GetIndexValue_Data)				( IOFireWireLibConfigDirectoryRef self, int inIndex, CFDataRef * value);
    /*! Description forthcoming */
    IOReturn (*GetIndexValue_String)			( IOFireWireLibConfigDirectoryRef self, int inIndex, CFStringRef* outValue);
    /*! Description forthcoming */
    IOReturn (*GetIndexValue_ConfigDirectory)	( IOFireWireLibConfigDirectoryRef self, int inIndex, IOFireWireLibConfigDirectoryRef* outValue, REFIID iid);
    /*! Description forthcoming */
    IOReturn (*GetIndexOffset_FWAddress)		( IOFireWireLibConfigDirectoryRef self, int inIndex, FWAddress* outValue);
    /*! Description forthcoming */
    IOReturn (*GetIndexOffset_UInt32)			( IOFireWireLibConfigDirectoryRef self, int inIndex, UInt32* outValue);
    /*! Description forthcoming */
    IOReturn (*GetIndexEntry)					( IOFireWireLibConfigDirectoryRef self, int inIndex, UInt32* outValue);
    /*! Description forthcoming */
    IOReturn (*GetSubdirectories)				( IOFireWireLibConfigDirectoryRef self, io_iterator_t* outIterator);
    /*! Description forthcoming */
    IOReturn (*GetKeySubdirectories)			( IOFireWireLibConfigDirectoryRef self, int inKey, io_iterator_t* outIterator);
    /*! Description forthcoming */
	IOReturn (*GetType)							( IOFireWireLibConfigDirectoryRef self, int* outType) ;
    /*! Description forthcoming */
	IOReturn (*GetNumEntries)					( IOFireWireLibConfigDirectoryRef self, int* outNumEntries) ;

} IOFireWireConfigDirectoryInterface ;

/*! Description forthcoming */
CF_INLINE IOVirtualRange
IOVirtualRangeMake( IOVirtualAddress address, IOByteCount length )
{
	IOVirtualRange range ;
	range.address = address ;
	range.length = length ;
	return range ;
}

#pragma mark -
#pragma mark VECTOR COMMAND INTERFACE
// ============================================================
// IOFireWireVectorCommandInterface
// ============================================================


/*!	@class
	@abstract IOFireWireLib command object for grouping commands execution.
	@discussion Read and Write commands can be attached in order to the vector command. When 
		the vector command is submitted all the commands are sent to the kernel for execution.
		When all the commands in a vector command are complete the vector command's completion is called.
		The advantage over submitting and completeing each command simultaneously is that only one kernel transition
		will be used for submission and one for completion, regardless of the number of commands in the vector.*/
typedef struct IOFireWireLibVectorCommandInterface_t
{

	IUNKNOWN_C_GUTS ;

	/*! Interface version. */
	UInt32 version;

	/*! Interface revision. */
	UInt32 revision;
	
	/*!	@function Submit
		@abstract Submit this command object to FireWire for execution.
		@param self A reference to the vector command object
		@result An IOReturn result code	*/

	IOReturn					(*Submit)(IOFireWireLibVectorCommandRef self);

	/*!	@function Submit
		@abstract Submit this command object to FireWire for execution.
		@discussion A convienence method to set the callback and refcon and then submit.
		@param self A reference to the vector command object
		@param refCon A reference constant for 3rd party use. This is the same refcon set with SetRefCon and retreived with GetRefCon.
		@param inCallback A callback function to be called upon command completion.
		@result An IOReturn result code	*/

	IOReturn					(*SubmitWithRefconAndCallback)(IOFireWireLibVectorCommandRef self, void* refCon, IOFireWireLibCommandCallback inCallback);
	
	/*!	@function IsExecuting
		@abstract Checks if the vector command is currently executing.
		@param self A reference to the vector command object
		@result True if the command is executing, false otherwise */

	Boolean						(*IsExecuting)(IOFireWireLibVectorCommandRef self);

	/*!	@function SetCallback
		@abstract Set the callback routine for this command.
		@param self A reference to the vector command object
		@param inCallback A callback function to be called upon command completion.
		@result void	*/
	
	void						(*SetCallback)(IOFireWireLibVectorCommandRef self, IOFireWireLibCommandCallback inCallback);

	/*!	@function SetRefCon
		@abstract Set the reference constant for this command.
		@param self A reference to the vector command object
		@param refCon A reference constant for 3rd party use.
		@result void	*/
			
	void						(*SetRefCon)(IOFireWireLibVectorCommandRef self, void* refCon);
	
	/*!	@function GetRefCon
		@abstract Get the reference constant for this command.
		@param self A reference to the vector command object
		@result The reference contant set in SetRefCon	*/
		
	void *						(*GetRefCon)(IOFireWireLibVectorCommandRef self);

	/*!	@function SetFlags
		@abstract Set flags governing this command's execution.
		@param self A reference to the vector command object
		@param inFlags A UInt32 with bits set corresponding to the flags that should be set
			for this command object. The following values may be used:<br>
			<ul>
				<li>kFWCommandNoFlags -- all flags off</li>
				<li>kFWCommandInterfaceSyncExecute -- Setting this flag causes the command object
					to execute synchronously. The calling context will block until the command
					object has completed execution or an error occurs. Using synchronous execution
					can avoid kernel transitions associated with asynchronous completion and often
					remove the need for a state machine.</li>
				<li>kFWVectorCommandInterfaceOrdered - Normally all commands in a vector are executed
				    simultaneously. Setting this flag will dispatch a command only after the prior 
					command has completed.
			</ul>
			@result void	*/
	
	void						(*SetFlags)(IOFireWireLibVectorCommandRef self, UInt32 inFlags);
	
	/*!	@function GetFlags
		@abstract Get the flags currently set for this command.
		@param self A reference to the vector command object
		@result The flags set in SetFlags	*/
		
	UInt32						(*GetFlags)(IOFireWireLibVectorCommandRef self);
	
	/*!	@function EnsureCapacity
		@abstract Sets the number of commands this vector can hold.
		@discussion The vector can grow dynamically, but for performance
			reasons developers may want the storage preallocated for a certain 
			number of commmands
		@param self A reference to the vector command object
		@param capacity The number of commands this vector command should expect to hold
		@result  An IOReturn result code	*/
		
	IOReturn					(*EnsureCapacity)(IOFireWireLibVectorCommandRef self, UInt32 capacity);
	
	/*!	@function AddCommand
		@abstract Adds a command to the vector command.
		@param self A reference to the vector command object
		@param command A reference to the command to add
		@result An IOReturn result code	*/
	
	void					(*AddCommand)(IOFireWireLibVectorCommandRef self, IOFireWireLibCommandRef command);
	
	/*!	@function RemoveCommand
		@abstract Removes a command to the vector command.
		@param self A reference to the vector command object
		@param command A reference to the command to be removed
		@result An IOReturn result code	*/
		
	void					(*RemoveCommand)(IOFireWireLibVectorCommandRef self, IOFireWireLibCommandRef command);
	
	/*!	@function InsertCommandAtIndex
		@abstract Inserts a command at a given index. Commands at and after 
			this index will be moved to their next sequential index.
		@param self A reference to the vector command object
		@param command A reference to the command to be inserted
		@param index The index to insert the command at.
	*/
		
	void					(*InsertCommandAtIndex)(IOFireWireLibVectorCommandRef self, IOFireWireLibCommandRef command, UInt32 index);
	
	/*!	@function GetCommandAtIndex
		@abstract Returns the command at a given index.
		@discussion Returns kIOReturnBadArgument if the index is out of bounds.
		@param self A reference to the vector command object
		@param index The index to return a command from
		@result The IOFireWireLibCommandRef at the specified index on return	*/
		
	IOFireWireLibCommandRef		(*GetCommandAtIndex)(IOFireWireLibVectorCommandRef self, UInt32 index);
		
	/*!	@function GetIndexOfCommand
		@abstract Returns the index of the specified command. 
		@discussion Returns kIOReturnNotFound if the command does not exist in this vector.
		@param self A reference to the vector command object
		@param command The command in question
		@result The index of the specified command	*/
		
	UInt32						(*GetIndexOfCommand)(IOFireWireLibVectorCommandRef self, IOFireWireLibCommandRef command);

	/*!	@function RemoveCommandAtIndex
		@abstract Removes the command at a give index. Commands at and afte
			this index will be moved to their previous sequential index.
		@discussion Returns kIOReturnBadArgument if the index is out of bounds.
		@param self A reference to the vector command object
		@param index Will be set to the index of the specified command.
	*/
		
	void					(*RemoveCommandAtIndex)(IOFireWireLibVectorCommandRef self, UInt32 index);
	
	/*!	@function RemoveAllCommands
		@abstract Removes all commands from the vector.
		@param self A reference to the vector command object
	*/
		
	void						(*RemoveAllCommands)(IOFireWireLibVectorCommandRef self);
	
	/*!	@function GetCommandCount
		@abstract Returns the number of commands currently in this vector.
		@param self A reference to the vector command object
		@result UInt32 The number of commands in this vector	*/
		
	UInt32						(*GetCommandCount)(IOFireWireLibVectorCommandRef self);
	
} IOFireWireLibVectorCommandInterface;

#pragma mark -
#pragma mark PHY PACKET LISTENER INTERFACE
// ============================================================
// IOFireWireLibPHYPacketListenerInterface Interface
// ============================================================

/*!	@class
	@discussion Represents and provides management functions for a phy packet listener object.
*/
typedef struct IOFireWireLibPHYPacketListenerInterface_t
{
	IUNKNOWN_C_GUTS ;

	/*! Interface version. */
	UInt16 version;

	/*! Interface revision. */
	UInt16 revision;

	/*!	@function SetListenerCallback
		@abstract Set the callback that should be called to handle incoming phy packets
		@param self The PHY packet listener object.
		@param inCallback The callback to set.
	*/
	void (*SetListenerCallback)( IOFireWireLibPHYPacketListenerRef self, IOFireWireLibPHYPacketCallback inCallback );

	/*!	@function SetSkippedPacketCallback
		@abstract Set the callback that should be called when incoming phy packets are
			dropped by the listener space.
		@param self The PHY packet listener object.
		@param inCallback The callback to set.
	*/
	void (*SetSkippedPacketCallback)( IOFireWireLibPHYPacketListenerRef self, IOFireWireLibPHYPacketSkippedCallback inCallback );

	/*!	@function NotificationIsOn
		@abstract Is notification on?
		@param self The PHY packet listener object.
		@result Returns true if packet notifications for this listener are active 
	*/
	Boolean (*NotificationIsOn)( IOFireWireLibPHYPacketListenerRef self );

	/*!	@function TurnOnNotification
		@abstract Try to turn on packet notifications for this listener.
		@param self The PHY packet listener object.
		@result Returns kIOReturnSuccess if successful */
	IOReturn (*TurnOnNotification)( IOFireWireLibPHYPacketListenerRef self );

	/*!	@function TurnOffNotification
		@abstract Turn packet notification off.
		@param self The PHY packet listener object. */
	void (*TurnOffNotification)( IOFireWireLibPHYPacketListenerRef self );

	/*!	@function ClientCommandIsComplete
		@abstract Notify the PHY packet listener object that a packet notification handler has completed.
		@discussion Packet notifications are received one at a time, in order. This function
			must be called after a packet handler has completed its work.
		@param self The PHY packet listener object.
		@param commandID The ID of the packet notification being completed. This is the same
			ID that was passed when a packet notification handler is called.
	*/
	void (*ClientCommandIsComplete)(IOFireWireLibPHYPacketListenerRef self, FWClientCommandID commandID );

	// --- accessors ----------

	/*!	@function SetRefCon
		@abstract Sets the user refCon value for this interface.
		@param self The PHY packet listener object.
		@param refcon the refcon */
	void (*SetRefCon)( IOFireWireLibPHYPacketListenerRef self, void * refcon );
		
	/*!	@function GetRefCon
		@abstract Returns the user refCon value for thisinterface.
		@param self The PHY packet listener object.
		@result returns the refcon */
	void* (*GetRefCon)( IOFireWireLibPHYPacketListenerRef self );

	/*!	@function SetFlags
		@abstract set flags for the listener.
		@param self The PHY packet listener object.
		@param flags No current flags are defined.
	*/	
	void (*SetFlags)( IOFireWireLibPHYPacketListenerRef self, UInt32 flags );
		
	/*!	@function GetFlags
		@abstract get the flags of listener.
		@param self The PHY packet listener object.
		@result flags No current flags are defined.	*/	
	UInt32 (*GetFlags)( IOFireWireLibPHYPacketListenerRef self );

} IOFireWireLibPHYPacketListenerInterface;

#endif // ifndef KERNEL

#endif //__IOFireWireLib_H__
