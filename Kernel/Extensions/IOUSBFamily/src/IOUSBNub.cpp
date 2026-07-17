/*
 * Copyright © 1998-2013 Apple Inc.  All rights reserved.
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


#include <libkern/OSByteOrder.h>
#include <libkern/c++/OSDictionary.h>
#include <libkern/c++/OSData.h>

#include <IOKit/IOMemoryDescriptor.h>
#include <IOKit/IOKitKeys.h>

#include <IOKit/usb/USB.h>
#include <IOKit/usb/IOUSBController.h>
#include <IOKit/usb/IOUSBNub.h>
#include <IOKit/usb/IOUSBPipe.h>
#include <IOKit/usb/IOUSBInterface.h>
#include <IOKit/usb/IOUSBHubPolicyMaker.h>
#include <IOKit/usb/IOUSBLog.h>

#include <IOKit/usb/USBSpec.h>

const OSSymbol *gUSBVendorID = NULL;
const OSSymbol *gUSBProductID = NULL;
const OSSymbol *gUSBInterfaceNumber = NULL;
const OSSymbol *gUSBConfigurationValue = NULL;
const OSSymbol *gUSBDeviceReleaseNumber = NULL;
const OSSymbol *gUSBInterfaceClass = NULL;
const OSSymbol *gUSBInterfaceSubClass = NULL;
const OSSymbol *gUSBInterfaceProtocol = NULL;
const OSSymbol *gUSBProductIDMask = NULL;
const OSSymbol *gUSBDeviceClass = NULL;
const OSSymbol *gUSBDeviceSubClass = NULL;
const OSSymbol *gUSBDeviceProtocol = NULL;

#define super	IOService

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

OSDefineMetaClassAndAbstractStructorsWithInit(IOUSBNub, IOService, IOUSBNub::initialize())

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void IOUSBNub::initialize()
{
    gUSBVendorID = OSSymbol::withCString(kUSBVendorID);
    gUSBProductID = OSSymbol::withCString(kUSBProductID);
    gUSBInterfaceNumber = OSSymbol::withCString(kUSBInterfaceNumber);
    gUSBConfigurationValue = OSSymbol::withCString(kUSBConfigurationValue);
    gUSBDeviceReleaseNumber = OSSymbol::withCString(kUSBDeviceReleaseNumber);
    gUSBInterfaceClass = OSSymbol::withCString(kUSBInterfaceClass);
    gUSBInterfaceSubClass = OSSymbol::withCString(kUSBInterfaceSubClass);
    gUSBInterfaceProtocol = OSSymbol::withCString(kUSBInterfaceProtocol);
    gUSBProductIDMask = OSSymbol::withCString(kUSBProductIDMask);
    gUSBDeviceClass = OSSymbol::withCString(kUSBDeviceClass);
    gUSBDeviceSubClass = OSSymbol::withCString(kUSBDeviceSubClass);
    gUSBDeviceProtocol = OSSymbol::withCString(kUSBDeviceProtocol);
}


/**
 ** Matching methods
 **/

bool 
IOUSBNub::USBCompareProperty( OSDictionary   * matching, const OSSymbol     * key )
{
    // We return success iff we match the key in the dictionary with the key in
    // the property table.
    //
    OSObject 	*value;
    bool		matches = false;
	OSObject	*myProperty = NULL;

    value = matching->getObject( key );
	
    if ( value)
	{
		myProperty = copyProperty(key);
		if (myProperty)
		{
			matches = value->isEqualTo( myProperty);
			myProperty->release();
		}
	}
    else
        matches = false;
	
    return matches;
}

//  This routine will look to see if the OSArray contains any matching keys.  The OSArray has to contain a list of OSNumbers.
bool 
IOUSBNub::USBComparePropertyInArray( OSDictionary *matching, const char * arrayName, const char * key, UInt32 * theProductIDThatMatched )
{
    // We return success iff we match any entry in the array with the key 
	OSArray *		propertyIDArray = NULL;
	OSNumber *		registryProperty = NULL;
	OSNumber *		propertyFromArrayItem = NULL;
    bool			matches = false;
	unsigned int	index;
	
	*theProductIDThatMatched = 0;
	
	// Get our nub's value for the key
	registryProperty = OSDynamicCast(OSNumber, getProperty(key));
	propertyIDArray = OSDynamicCast(OSArray, matching->getObject(arrayName));
	
	// Iterate over the array looking for the entries
	if (propertyIDArray && registryProperty)
	{
		USBLog(7, "%s[%p]::USBComparePropertyInArray - found array with capacity of %d", getName(), this, propertyIDArray->getCount());
		
		for (index = 0; index < propertyIDArray->getCount(); index++)
		{
			propertyFromArrayItem = OSDynamicCast(OSNumber, propertyIDArray->getObject(index));
			if (propertyFromArrayItem)
			{
				// See if this item has the same value as the one in our registry for this key
				matches = propertyFromArrayItem->isEqualTo( registryProperty);
				if (matches)
				{
					*theProductIDThatMatched = propertyFromArrayItem->unsigned32BitValue();
					USBLog(7, "%s[%p]::USBComparePropertyInArray - item %d matched:  id = 0x%x", getName(), this, index, (uint32_t) *theProductIDThatMatched);
					break;
				}
				else 
				{
					USBLog(7, "%s[%p]::USBComparePropertyInArray - item %d did not match", getName(), this, index);
				}
			}
		}
	}
	
    return matches;
}

//  This routine will look to see if the OSArray contains any matching keys.  The OSArray has to contain a list of OSNumbers.
bool 
IOUSBNub::USBComparePropertyInArrayWithMask( OSDictionary *matching, const char * arrayName, const char * key, const char * maskKey, UInt32 * theProductIDThatMatched )
{
    // We return success iff we match any entry in the array with the key 
	OSArray *		propertyIDArray = NULL;
	OSNumber *		registryProperty = NULL;
	OSNumber *		propertyFromArrayItem = NULL;
    OSNumber *		dictionaryMask = NULL;
    bool			matches = false;
	unsigned int	index;
	
	*theProductIDThatMatched = 0;
	
	// Get our nub's value for the key
	registryProperty = OSDynamicCast(OSNumber, getProperty(key));
	propertyIDArray = OSDynamicCast(OSArray, matching->getObject(arrayName));
    dictionaryMask = OSDynamicCast(OSNumber, matching->getObject(maskKey));
	
	// Iterate over the array looking for the entries
	if (propertyIDArray && registryProperty && dictionaryMask)
	{
		USBLog(7, "%s[%p]::USBComparePropertyInArrayWithMask - found array with capacity of %d", getName(), this, propertyIDArray->getCount());
		
		for (index = 0; index < propertyIDArray->getCount(); index++)
		{
			propertyFromArrayItem = OSDynamicCast(OSNumber, propertyIDArray->getObject(index));
			if (propertyFromArrayItem)
			{
				UInt32  registryValue = registryProperty->unsigned32BitValue();
				UInt32  arrayValue = propertyFromArrayItem->unsigned32BitValue();
				UInt32  mask = dictionaryMask->unsigned32BitValue();
				
				if ( (registryValue & mask) == (arrayValue & mask) )
				{
					USBLog(7, "%s[%p]::USBComparePropertyInArrayWithMask - 0x%x, 0x%x, mask 0x%x matched", getName(), this, (uint32_t)arrayValue, (uint32_t)registryValue, (uint32_t)mask);
					*theProductIDThatMatched = registryValue;
					matches = true;
				}
			}
		}
	}
	
    return matches;
}


bool 
IOUSBNub::IsWildCardMatch( OSDictionary   * matching, const char     * key )
{
    // We return success iff the  key in the dictionary exists AND it is a OSString "*"
    // the property table.
    //
    OSString 	*theString;
    bool		matches;
    
    theString = OSDynamicCast(OSString, matching->getObject( key ));
    
    if ( theString)
        matches = theString->isEqualTo("*");
    else
        matches = false;
    
    return matches;
}  

bool
IOUSBNub::USBComparePropertyWithMask( OSDictionary *matching, const char *key, const char * maskKey )
{
    // This routine will return success if the "key" in the dictionary  matches the key in the property table
    // while applying the "mask" value to both
    // First, check to see if we have both keys
    //
    OSNumber *	registryProperty = NULL;
    OSNumber *	dictionaryProperty = NULL;
    OSNumber *	dictionaryMask = NULL;
    
    registryProperty = OSDynamicCast(OSNumber,  getProperty(key));
    dictionaryProperty = OSDynamicCast(OSNumber, matching->getObject(key));
    dictionaryMask = OSDynamicCast(OSNumber, matching->getObject(maskKey));
    
    if ( registryProperty && dictionaryProperty && dictionaryMask )
    {
		// If all our values are OSNumbers, then get their actual value and do the masking
		// to see if they are equal
		//
		UInt32  registryValue = registryProperty->unsigned32BitValue();
		UInt32  dictionaryValue = dictionaryProperty->unsigned32BitValue();
		UInt32  mask = dictionaryMask->unsigned32BitValue();
		
		if ( (registryValue & mask) == (dictionaryValue & mask) )
		{
			return true;
		}
    }
    
    return false;
}



// this method will override the Platform specific implementation of joinPMtree
// it will cause any IOUSBDevice and IOUSBInterface clients to join the IOPower tree as
// children of the IOUSHubPolicyMaker for the hub to which they are attached
void
IOUSBNub::joinPMtree ( IOService * driver )
{
	IOUSBHubPolicyMaker			*hubPolicyMaker = NULL;
	IOUSBDevice					*device = OSDynamicCast(IOUSBDevice, this);
	IOUSBInterface				*interface = NULL;
	
	
	// we should be either an IOUSBDevice or an IOUSBInterface
	if (device)
		hubPolicyMaker = device->GetHubParent();
	else
	{
		interface = OSDynamicCast(IOUSBInterface, this);

		if (interface && interface->GetDevice())
			hubPolicyMaker = interface->GetDevice()->GetHubParent();
	}

	if (hubPolicyMaker)
	{
		hubPolicyMaker->joinPMtree(driver);
	}
	else
	{
		USBLog(1, "%s[%p]::joinPMtree - no hub policy maker - calling through to super::joinPMtree", getName(), this);
		super::joinPMtree(driver);
	}
	
}



