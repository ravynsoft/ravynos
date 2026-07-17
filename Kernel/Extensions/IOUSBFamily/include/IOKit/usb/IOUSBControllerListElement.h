/*
 * Copyright © 1998-2007 Apple Inc.  All rights reserved.
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

#ifndef _IOUSBCONTROLLERLISTELEMENT_H
#define _IOUSBCONTROLLERLISTELEMENT_H


#include <libkern/c++/OSObject.h>

#include <IOKit/IOTypes.h>

#include <IOKit/usb/USB.h>


/*
 class IOUSBControllerListElement
 Used by the IOUSBController to manage the USB controller lists that are common between EHCI and UHCI.
*/
class IOUSBControllerListElement : public OSObject
{
    OSDeclareDefaultStructors(IOUSBControllerListElement)
	
private:    
	
public:
	
    virtual void					SetPhysicalLink(IOPhysicalAddress next) = 0;
    virtual IOPhysicalAddress		GetPhysicalLink(void) = 0;
    virtual IOPhysicalAddress		GetPhysicalAddrWithType(void) = 0;
    virtual void					print(int level);
	
    IOPhysicalAddress				_sharedPhysical;			// phys address of the memory shared with the controller			
    void *							_sharedLogical;				// logical address of the above
    IOUSBControllerListElement		*_logicalNext;				// the next element in the list
    
};


class IOUSBControllerV2;										// needed for a parameter
class IOUSBControllerIsochEndpoint;

class IOUSBControllerIsochListElement : public IOUSBControllerListElement
{
    OSDeclareDefaultStructors(IOUSBControllerIsochListElement)
	
private:
    
public:
	
    virtual void					SetPhysicalLink(IOPhysicalAddress next) = 0;
    virtual IOPhysicalAddress		GetPhysicalLink(void) = 0;
    virtual IOPhysicalAddress		GetPhysicalAddrWithType(void) = 0;
    virtual void					print(int level);
	
    IOUSBControllerIsochEndpoint	*_pEndpoint;
    IOUSBIsocFrame					*_pFrames;
    IOUSBIsocCompletion				_completion;
    Boolean							_lowLatency;
	bool							_requestFromRosettaClient; // True if the request originated from a Rosetta client in user space
	UInt8							_framesInTD;			// used for HS Isoch only
    UInt64							_frameNumber;			// frame number for scheduling purposes
    UInt32							_frameIndex;			// index into the myFrames array
    IOUSBControllerIsochListElement *_doneQueueLink;		// linkage used by done queue processing
	
    // pure virtual methods which must be implemented by descendants
    virtual IOReturn				UpdateFrameList(AbsoluteTime timeStamp) = 0;
    virtual IOReturn				Deallocate(IOUSBControllerV2 *uim) = 0;
};

class IOUSBControllerIsochEndpoint : public OSObject
{
    OSDeclareDefaultStructors(IOUSBControllerIsochEndpoint)
	
public:
	
	virtual bool init();

    IOUSBControllerIsochEndpoint		*nextEP;
    IOUSBControllerIsochListElement  	*toDoList;					// ITD or SITD
    IOUSBControllerIsochListElement  	*toDoEnd;					// ITD or SITD
    IOUSBControllerIsochListElement  	*doneQueue;					// ITD or SITD
    IOUSBControllerIsochListElement  	*doneEnd;					// ITD or SITD
    IOUSBControllerIsochListElement  	*deferredQueue;				// ITD or SITD
    IOUSBControllerIsochListElement  	*deferredEnd;				// ITD or SITD
    UInt64								firstAvailableFrame;		// next frame available for a transfer on this EP
    UInt32								maxPacketSize;
    UInt32								activeTDs;					// + when added to todo list, - when taken from done queue
    UInt32								onToDoList;					// + when added to todo list, - when taken from done queue
    UInt32								onDoneQueue;					// + when added to todo list, - when taken from done queue
	volatile SInt32						scheduledTDs;				// + when linked onto periodic list, - when unlinked
	UInt32								deferredTDs;
	UInt32								onProducerQ;
	UInt32								onReversedList;
    UInt16								inSlot;						// where Isoc TDs are being put in the periodic list 
    short								functionAddress;
    short								endpointNumber;
    IOReturn							accumulatedStatus;
	UInt32								interval;					// this is the decoded interval value for HS endpoints and is 1 for FS endpoints
    UInt8								direction;
	bool								aborting;
};


#endif

