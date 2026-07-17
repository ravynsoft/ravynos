/*
 * Copyright © 1998-2012 Apple Inc. All rights reserved.
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

#ifndef _IOKIT_IOUSBNUB_H
#define _IOKIT_IOUSBNUB_H

#include <IOKit/IOService.h>
#include <libkern/c++/OSData.h>
#include <IOKit/IOMemoryDescriptor.h>

#include <IOKit/usb/USB.h>

extern const OSSymbol *gUSBVendorID;
extern const OSSymbol *gUSBProductID;
extern const OSSymbol *gUSBInterfaceNumber;
extern const OSSymbol *gUSBConfigurationValue;
extern const OSSymbol *gUSBDeviceReleaseNumber;
extern const OSSymbol *gUSBInterfaceClass;
extern const OSSymbol *gUSBInterfaceSubClass;
extern const OSSymbol *gUSBInterfaceProtocol;
extern const OSSymbol *gUSBProductIDMask;
extern const OSSymbol *gUSBDeviceClass;
extern const OSSymbol *gUSBDeviceSubClass;
extern const OSSymbol *gUSBDeviceProtocol;

class IOUSBController;
class IOUSBPipe;

/*
 IOUSBNub
 Super class for for IOUSBDevice and IOUSBInterface.
 */
class IOUSBNub : public IOService
{
    OSDeclareDefaultStructors(IOUSBNub)

public:
    static void initialize();

	// IOKit method
	virtual void					joinPMtree ( IOService * driver );

    virtual bool					USBCompareProperty(OSDictionary   * matching, const OSSymbol     * key );
    
    bool							IsWildCardMatch( OSDictionary   * matching, const char     * key );
    bool							USBComparePropertyWithMask( OSDictionary *matching, const char *key, const char * maskKey );
	bool							USBComparePropertyInArray( OSDictionary *matching, const char *arrayName, const char * key, UInt32 * theProductIDThatMatched );
	bool							USBComparePropertyInArrayWithMask( OSDictionary *matching, const char *arrayName, const char * key, const char * maskKey, UInt32 * theProductIDThatMatched );

};

#ifdef __cplusplus
extern "C" {
#endif

void printDescriptor(const IOUSBDescriptorHeader *desc);
void printDeviceDescriptor(const IOUSBDeviceDescriptor *desc);
void printConfigDescriptor(const IOUSBConfigurationDescriptor *cd);
void printEndpointDescriptor(const IOUSBEndpointDescriptor *ed);
void printInterfaceDescriptor(const IOUSBInterfaceDescriptor *id);

#ifdef __cplusplus
}
#endif

#endif
