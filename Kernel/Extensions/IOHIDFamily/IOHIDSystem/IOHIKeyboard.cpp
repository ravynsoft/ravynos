/*
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2009 Apple Computer, Inc.  All Rights Reserved.
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
/*
 *  24-Jan-01	bubba	Don't auto-repeat on Power Key. This prevents infinite power key
 *			events from being generated when this key is hit on ADB keyboards.
 */

#include <AssertMacros.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOLocks.h>
#include <IOKit/hidsystem/IOHIKeyboardMapper.h>
#include <IOKit/hidsystem/IOLLEvent.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include "IOKit/hidsystem/IOHIDSystem.h"
#include "IOKit/hidsystem/IOHIKeyboard.h"
#ifdef NEW_HID
#include "IOHIDKeyboardDevice.h"
#endif
#include "IOHIDFamilyTrace.h"
#include "ev_private.h"
#include "IOHIDDebug.h"

//************************************************************************
// KeyboardReserved
//
// RY: The following was added because the IOHIKeyboard class doesn't have
// a reserved field defined.  Essentially what this does is create a
// static dictionary that stores OSData objects for each keyboard.  These
// OSData objects will be added and removed as each keyboard enters and
// leaves the system.

struct KeyboardReserved
{
    IOHIKeyboard *  service;
	thread_call_t	repeat_thread_call;
    bool			dispatchEventCalled;
    bool			isSeized;
    bool			repeatMode;
    bool            hasSecurePrompt; // deprecated
    IOService *		openClient;
    //IOHIDKeyboardDevice *	keyboardNub;
};

static OSArray  *gKeyboardReservedArray = OSArray::withCapacity(4);
IOLock          *gKeyboardReservedArrayLock = IOLockAlloc();

static KeyboardReserved * GetKeyboardReservedStructEventForService(IOHIKeyboard *service, UInt32 * index = 0)
{
    KeyboardReserved 	* retVal    = 0;
    if (gKeyboardReservedArray) {
        OSCollectionIterator    * iterator  = 0;
        iterator = OSCollectionIterator::withCollection(gKeyboardReservedArray);
        if (iterator) {
            bool done = false;
            while (!done) {
                OSObject * obj = 0;
                IOLockLock(gKeyboardReservedArrayLock);
                while (!done && (NULL != (obj = iterator->getNextObject()))) {
                    obj->retain();
                    IOLockUnlock(gKeyboardReservedArrayLock);
                    
                    OSData * data = OSDynamicCast(OSData, obj);
                    if (data) {
                        retVal = (KeyboardReserved *)data->getBytesNoCopy();
                        if (retVal && (retVal->service == service)) {
                            if (index)
                                *index = gKeyboardReservedArray->getNextIndexOfObject(obj, 0);
                            done = true;
                        }
                        else {
                            retVal = 0;
                        }
                    }
                    IOLockLock(gKeyboardReservedArrayLock);
                }
                IOLockUnlock(gKeyboardReservedArrayLock);
                if (iterator->isValid()) {
                    done = true;
                }
                else {
                    iterator->reset();
                }
            }
            iterator->release();
        }
    }
    return retVal;
}

static void AppendNewKeyboardReservedStructForService(IOHIKeyboard *service)
{
    KeyboardReserved 	temp;
    OSData 		* data		= 0;
    
    if (gKeyboardReservedArray)
    {
        bzero(&temp, sizeof(KeyboardReserved));
        temp.repeatMode = true;
        temp.service = service;
        data = OSData::withBytes(&temp, sizeof(KeyboardReserved));
        IOLockLock(gKeyboardReservedArrayLock);
        gKeyboardReservedArray->setObject(data);
        IOLockUnlock(gKeyboardReservedArrayLock);
        data->release();
    }
}

static void RemoveKeyboardReservedStructForService(IOHIKeyboard *service)
{
    UInt32 index = 0;
    
    if (gKeyboardReservedArray && GetKeyboardReservedStructEventForService(service, &index) )
    {
        IOLockLock(gKeyboardReservedArrayLock);
        gKeyboardReservedArray->removeObject(index);
        IOLockUnlock(gKeyboardReservedArrayLock);
    }
}

//************************************************************************

#define super IOHIDevice
OSDefineMetaClassAndStructors(IOHIKeyboard, IOHIDevice);

bool IOHIKeyboard::init(OSDictionary * properties)
{
  if (!super::init(properties))  return false;

  /*
   * Initialize minimal state.
   */

  _deviceLock   = IOLockAlloc();
  _keyMap       = 0;
  _keyStateSize = 4*((maxKeyCodes()+(EVK_BITS_PER_UNIT-1))/EVK_BITS_PER_UNIT);
  _keyState     = (UInt32 *) IOMalloc(_keyStateSize);
  _codeToRepeat = (unsigned)-1;
  
  _keyboardEventTarget        = 0;
  _keyboardEventAction        = 0;
  _keyboardSpecialEventTarget = 0;
  _keyboardSpecialEventAction = 0;
  _updateEventFlagsTarget     = 0;
  _updateEventFlagsAction     = 0;
    
  if (!_deviceLock || !_keyState)  return false;

  bzero(_keyState, _keyStateSize);
  
  return true;
}

bool IOHIKeyboard::start(IOService * provider)
{
  if (!super::start(provider))  return false;
  
  /*
   * IOHIKeyboard serves both as a service and a nub (we lead a double
   * life).  Register ourselves as a nub to kick off matching.
   */

  AppendNewKeyboardReservedStructForService(this);
  KeyboardReserved * tempReservedStruct = GetKeyboardReservedStructEventForService(this);

  if (tempReservedStruct)
  {
    tempReservedStruct->repeat_thread_call = thread_call_allocate(_autoRepeat, this);
  }

  registerService(kIOServiceAsynchronous);

  return true;
}

void IOHIKeyboard::stop(IOService * provider)
{
	super::stop(provider);

	KeyboardReserved *tempReservedStruct = GetKeyboardReservedStructEventForService(this);        

	if (tempReservedStruct) {
		thread_call_cancel(tempReservedStruct->repeat_thread_call);
		thread_call_free(tempReservedStruct->repeat_thread_call);
		tempReservedStruct->repeat_thread_call = NULL;

//		if ( tempReservedStruct->keyboardNub )
//			tempReservedStruct->keyboardNub->release();
//		tempReservedStruct->keyboardNub = NULL;
		
		RemoveKeyboardReservedStructForService(this);
	}
}

void IOHIKeyboard::free()
// Description:	Go Away. Be careful when freeing the lock.
{
    IOLock * lock = NULL;

    if ( _deviceLock )
    {
      lock = _deviceLock;
      IOLockLock( lock);
      _deviceLock = NULL;
    }

    if ( _keyMap ) {
        _keyMap->release();
        _keyMap = 0;
    }

    if( _keyState )
        IOFree( _keyState, _keyStateSize);

    // RY: MENTAL NOTE Do this last
    if ( lock )
    {
      IOLockUnlock( lock);
      IOLockFree( lock);
    }
    
    super::free();
}

IOHIDKind IOHIKeyboard::hidKind()
{
  return kHIKeyboardDevice;
}

bool IOHIKeyboard::updateProperties( void )
{
    bool	ok;
	
    ok = setProperty( kIOHIDKeyMappingKey, _keyMap );
    
    return( ok & super::updateProperties() );
}

IOReturn IOHIKeyboard::setParamProperties( OSDictionary * dict )
{
    OSData *		data	= NULL;
    OSNumber *		number	= NULL;
    IOReturn		err 	= kIOReturnSuccess;
    IOReturn		err2	= kIOReturnSuccess;
    unsigned char *	map	= NULL;
    IOHIKeyboardMapper * oldMap	= NULL;
    bool		updated = false;
    UInt64		nano;
    
    require_action(dict, exit, err = kIOReturnBadArgument);
    if( dict->getObject(kIOHIDResetKeyboardKey))
		resetKeyboard();

    IOLockLock( _deviceLock);

    if ((number = OSDynamicCast(OSNumber,
                              dict->getObject(kIOHIDKeyRepeatKey))) ||
        (data = OSDynamicCast(OSData,
                              dict->getObject(kIOHIDKeyRepeatKey))))
    {
        nano = (number) ? number->unsigned64BitValue() : *((UInt64 *) (data->getBytesNoCopy()));

        if( nano < EV_MINKEYREPEAT)
            nano = EV_MINKEYREPEAT;
        nanoseconds_to_absolutetime(nano, &_keyRepeat);
        updated = true;
    }

    if ((number = OSDynamicCast(OSNumber,
                              dict->getObject(kIOHIDInitialKeyRepeatKey))) ||
        (data = OSDynamicCast(OSData,
                              dict->getObject(kIOHIDInitialKeyRepeatKey))))
    {
        nano = (number) ? number->unsigned64BitValue() : *((UInt64 *) (data->getBytesNoCopy()));

        if( nano < EV_MINKEYREPEAT)
            nano = EV_MINKEYREPEAT;
        nanoseconds_to_absolutetime(nano, &_initialKeyRepeat);
        updated = true;
    }

    if( (data = OSDynamicCast( OSData, dict->getObject(kIOHIDKeyMappingKey))))
	{
	
		map = (unsigned char *)IOMalloc( data->getLength() );
		bcopy( data->getBytesNoCopy(), map, data->getLength() );
		oldMap = _keyMap;
		_keyMap = IOHIKeyboardMapper::keyboardMapper(this, map, data->getLength(), true);

		if (_keyMap)
		{
			// point the new keymap to the IOHIDSystem, so it can set properties in it
			_keyMap->setKeyboardTarget((IOService *) _keyboardEventTarget);
	
			if (oldMap)
				oldMap->release();
			updated = true;
		}
		else
		{
			_keyMap = oldMap;
			err = kIOReturnBadArgument;
		} 
    }
    if (NULL != (number = OSDynamicCast(OSNumber, dict->getObject(kIOHIDSubinterfaceIDKey))))
    {
        if (!OSDynamicCast(OSNumber, getProperty(kIOHIDOriginalSubinterfaceIDKey))) {
            // no original key
            OSNumber *original = (OSNumber*)copyProperty(kIOHIDSubinterfaceIDKey);
            if (OSDynamicCast(OSNumber, original)) {
                setProperty(kIOHIDOriginalSubinterfaceIDKey, original);
            }
            OSSafeReleaseNULL(original);
        }
        _deviceType = number->unsigned32BitValue();
        updated = true;
    }
		
	// give the keymap a chance to update to new properties
	if (_keyMap)
		err2 = _keyMap->setParamProperties(dict);

    IOLockUnlock( _deviceLock);
	
    if( updated )
        updateProperties();

	// we can only return one error
	if (err == kIOReturnSuccess)
		err = err2;
    
exit:
    return( err == kIOReturnSuccess ) ? super::setParamProperties(dict) : err;
}

// RY: Override IORegistryEntry::setProperties().  This was removed earlier 
// in Leopard, but turns out that won't work as the linker will not end up 
// doing the right thing and travese the super class to resolve the symbol.
IOReturn IOHIKeyboard::setProperties( OSObject * properties )
{
    return super::setProperties(properties);
}



bool IOHIKeyboard::resetKeyboard()
// Description:	Reset the keymapping to the default value and reconfigure
//		the keyboards.
{
    const unsigned char *defaultKeymap;
    UInt32	defaultKeymapLength;

    IOLockLock( _deviceLock);

    if ( _keyMap ) {
        _keyMap->release();
        _keyMap = 0;
    }

    // Set up default keymapping.
    defaultKeymap = defaultKeymapOfLength(&defaultKeymapLength);

    _keyMap = IOHIKeyboardMapper::keyboardMapper( this,
                                                  defaultKeymap,
                                                  defaultKeymapLength,
                                                  false );

    if (_keyMap)
    {
		// point the new keymap to the IOHIDSystem, so it can set properties in it
		_keyMap->setKeyboardTarget((IOService *) _keyboardEventTarget);

        clock_interval_to_absolutetime_interval( EV_DEFAULTKEYREPEAT,
                                                 kNanosecondScale, &_keyRepeat);
        clock_interval_to_absolutetime_interval( EV_DEFAULTINITIALREPEAT,
                                                 kNanosecondScale, &_initialKeyRepeat);
    }

    updateProperties();
    
    _interfaceType = interfaceID();
    _deviceType    = deviceType();
    _guid	   = getGUID();

//    if (getProperty("HIDKeyboardKeysDefined"))
//    {
//        KeyboardReserved * reservedStruct = GetKeyboardReservedStructEventForService(this);
//        
//        if ( reservedStruct && !reservedStruct->keyboardNub)
//            reservedStruct->keyboardNub = IOHIDKeyboardDevice::newKeyboardDeviceAndStart(this);
//    }

    IOLockUnlock( _deviceLock);
    return (_keyMap) ? true : false;
}

void IOHIKeyboard::scheduleAutoRepeat()
// Description:	Schedule a procedure to be called when a timeout has expired
//		so that we can generate a repeated key.
// Preconditions:
// *	_deviceLock should be held on entry
{
    KeyboardReserved *tempReservedStruct = GetKeyboardReservedStructEventForService(this); 
          
    if ( _calloutPending == true )
    {        
        if (tempReservedStruct) {
            thread_call_cancel(tempReservedStruct->repeat_thread_call);
        }
	_calloutPending = false;
    }
    if ( AbsoluteTime_to_scalar(&_downRepeatTime) )
    {
        AbsoluteTime deadline;
        clock_absolutetime_interval_to_deadline(_downRepeatTime, &deadline);
        if (tempReservedStruct) {
            IOHID_DEBUG(kIOHIDDebugCode_KeyboardCapsThreadTrigger, this, __OSAbsoluteTime(deadline), tempReservedStruct->repeat_thread_call, 0);
            thread_call_enter_delayed(tempReservedStruct->repeat_thread_call, deadline);
        }
	_calloutPending = true;
    }
}

void IOHIKeyboard::_autoRepeat(thread_call_param_t arg,
                               thread_call_param_t)         /* thread_call_func_t */
{
    IOHIKeyboard *self = (IOHIKeyboard *) arg;
    if (self) {
        IOHID_DEBUG(kIOHIDDebugCode_KeyboardCapsThreadActive, self, self ? self->_codeToRepeat : 0, 0, 0);
        self->autoRepeat();
    }   
}

void IOHIKeyboard::autoRepeat()
// Description:	Repeat the currently pressed key and schedule ourselves
//		to be called again after another interval elapses.
// Preconditions:
// *	Should only be executed on callout thread
// *	_deviceLock should be unlocked on entry.
{    
    KeyboardReserved *tempReservedStruct = GetKeyboardReservedStructEventForService(this); 
    
    IOLockLock( _deviceLock);
    if (( _calloutPending == false ) || 
        ((tempReservedStruct) && tempReservedStruct->dispatchEventCalled ))
    {
        IOLockUnlock( _deviceLock);
        return;
    }
    _calloutPending = false;
    _isRepeat = true;
    
    if (tempReservedStruct) tempReservedStruct->dispatchEventCalled = true;
    
    if ( AbsoluteTime_to_scalar(&_downRepeatTime) )
    {
        if (_keyMap) {
            _keyMap->translateKeyCode(_codeToRepeat,
                                      true, /* direction */
                                      _keyState /* keyBits */);
        }
        _downRepeatTime = _keyRepeat;
    }
    
    if (tempReservedStruct) tempReservedStruct->dispatchEventCalled = false;
    
    _isRepeat = false;
    scheduleAutoRepeat();
    IOLockUnlock( _deviceLock);
}

void IOHIKeyboard::setRepeat(unsigned eventType, unsigned keyCode)
// Description:	Set up or tear down key repeat operations. The method
//		that locks _deviceLock is a bit higher on the call stack.
//		This method is invoked as a side effect of our own
//		invocation of _keyMap->translateKeyCode().
// Preconditions:
// *	_deviceLock should be held upon entry.
{
	KeyboardReserved *tempReservedStruct;
	
    if ( _isRepeat == false )  // make sure we're not already repeating
    {
		tempReservedStruct = GetKeyboardReservedStructEventForService(this);
		
		if ((eventType == NX_KEYDOWN) && 
			(tempReservedStruct && (tempReservedStruct->repeatMode == true)))	// Start repeat
		{
			// Set this key to repeat (push out last key if present)
			_downRepeatTime = _initialKeyRepeat; // + _lastEventTime; 
			_codeToRepeat = keyCode;
			// reschedule key repeat event here
			scheduleAutoRepeat();
		}
		else if (((eventType == NX_KEYUP) && (_codeToRepeat == keyCode)) ||
			(tempReservedStruct && (tempReservedStruct->repeatMode == false))) // End repeat
		{
			AbsoluteTime_to_scalar(&_downRepeatTime) = 0;
			_codeToRepeat = (unsigned)-1;
			scheduleAutoRepeat();
		}
    }
}

void IOHIKeyboard::setRepeatMode(bool repeat)
{
    KeyboardReserved *tempReservedStruct = GetKeyboardReservedStructEventForService(this); 

    IOLockLock( _deviceLock);
    if (tempReservedStruct)
    {
		tempReservedStruct->repeatMode = repeat;
    }
	IOLockUnlock( _deviceLock);
}


//
// BEGIN:	Implementation of the methods required by IOHIKeyboardMapper.
//

void IOHIKeyboard::keyboardEvent(unsigned eventType,
	/* flags */              unsigned flags,
	/* keyCode */            unsigned keyCode,
	/* charCode */           unsigned charCode,
	/* charSet */            unsigned charSet,
	/* originalCharCode */   unsigned origCharCode,
	/* originalCharSet */    unsigned origCharSet)
// Description: We use this notification to set up our _keyRepeat timer
//		and to pass along the event to our owner. This method
//		will be called while the KeyMap object is processing
//		the key code we've sent it using deliverKey.
{
    KeyboardReserved    *tempReservedStruct = GetKeyboardReservedStructEventForService(this); 
    bool                relock = false;
    
    if (tempReservedStruct && tempReservedStruct->dispatchEventCalled) 
    {
        relock = true;
        IOLockUnlock(_deviceLock);
    }
    
    UInt16 usage = 0;
    UInt16 usagePage = 0;
    unsigned modifiedOrigCharCode = origCharCode;
    unsigned modifiedOrigCharSet = origCharSet;
    unsigned modifiedFlags = flags;

    if ((origCharCode < 0xffff) && (origCharSet < 0xffff)) {
        getLastPageAndUsage(usagePage, usage);
        
        if (usage || usagePage) {
            modifiedOrigCharCode |= usage << 16;
            modifiedOrigCharSet |= usagePage << 16;
            modifiedFlags |= NX_HIGHCODE_ENCODING_MASK;
        }
    }
    else {
        HIDLogError("original code/set unusually large %02x:%02x", origCharCode, origCharSet);
    }

    _keyboardEvent(	   this,
                           eventType,
    /* flags */            modifiedFlags,
    /* keyCode */          keyCode,
    /* charCode */         charCode,
    /* charSet */          charSet,
    /* originalCharCode */ modifiedOrigCharCode,
    /* originalCharSet */  modifiedOrigCharSet,
    /* keyboardType */     _deviceType,
    /* repeat */           _isRepeat,
    /* atTime */           _lastEventTime);

    if (relock) 
    {
        IOLockLock(_deviceLock);
    }


    if( keyCode == _keyMap->getParsedSpecialKey(NX_KEYTYPE_CAPS_LOCK) ||
        keyCode == _keyMap->getParsedSpecialKey(NX_KEYTYPE_NUM_LOCK) ||
        keyCode == _keyMap->getParsedSpecialKey(NX_POWER_KEY) ||
        keyCode == _keyMap->getParsedSpecialKey(NX_KEYTYPE_MUTE) ||
        keyCode == _keyMap->getParsedSpecialKey(NX_KEYTYPE_PLAY) ||
        keyCode == _keyMap->getParsedSpecialKey(NX_KEYTYPE_EJECT) ||
        keyCode == _keyMap->getParsedSpecialKey(NX_KEYTYPE_VIDMIRROR) ||
        keyCode == _keyMap->getParsedSpecialKey(NX_KEYTYPE_ILLUMINATION_TOGGLE))  
    {		
		//Don't repeat caps lock on ADB/USB.  0x39 is default ADB code.
		//    We are here because KeyCaps needs to see 0x39 as a real key,
		//    not just another modifier bit.

		if (_interfaceType == NX_EVS_DEVICE_INTERFACE_ADB)
		{
			return;
		}
    }

    // Set up key repeat operations here.
    setRepeat(eventType, keyCode);
}

void IOHIKeyboard::keyboardSpecialEvent(unsigned eventType,
	/* flags */                     unsigned flags,
	/* keyCode */                   unsigned keyCode,
	/* specialty */                 unsigned flavor)
// Description: See the description for keyboardEvent.
{
    KeyboardReserved    *tempReservedStruct = GetKeyboardReservedStructEventForService(this); 
    bool                relock = false;
    
    if (tempReservedStruct && tempReservedStruct->dispatchEventCalled) 
    {
        relock = true;
        IOLockUnlock(_deviceLock);
    }

    _keyboardSpecialEvent(this,
                        eventType,
        /* flags */	flags,
        /* keyCode */	keyCode,
        /* specialty */	flavor,
        /* guid */ 	_guid,
        /* repeat */	_isRepeat,
        /* atTime */	_lastEventTime);
                    
    if (relock) 
    {
        IOLockLock(_deviceLock);
    }

    // Set up key repeat operations here.
    // Don't repeat capslock, numlock, power key, mute key, play key, 
    // eject key, vidmirror key, illumination toggle key.
    if( keyCode == _keyMap->getParsedSpecialKey(NX_KEYTYPE_CAPS_LOCK) ||
        keyCode == _keyMap->getParsedSpecialKey(NX_KEYTYPE_NUM_LOCK) ||
        keyCode == _keyMap->getParsedSpecialKey(NX_POWER_KEY) ||
        keyCode == _keyMap->getParsedSpecialKey(NX_KEYTYPE_MUTE) ||
        keyCode == _keyMap->getParsedSpecialKey(NX_KEYTYPE_PLAY) ||
        keyCode == _keyMap->getParsedSpecialKey(NX_KEYTYPE_EJECT) ||
        keyCode == _keyMap->getParsedSpecialKey(NX_KEYTYPE_VIDMIRROR) ||
        keyCode == _keyMap->getParsedSpecialKey(NX_KEYTYPE_ILLUMINATION_TOGGLE))  
    {
        return;
    }

    // Set up key repeat operations here.
    setRepeat(eventType, keyCode);
}

void IOHIKeyboard::updateEventFlags(unsigned flags)
// Description:	Process non-event-generating flag changes. Simply pass this
//		along to our owner.
{
    KeyboardReserved    *tempReservedStruct = GetKeyboardReservedStructEventForService(this); 
    bool                relock = false;
    
    if (tempReservedStruct && tempReservedStruct->dispatchEventCalled) 
    {
        relock = true;
        IOLockUnlock(_deviceLock);
    }

    _updateEventFlags(this, flags);
                    
    if (relock) 
    {
        IOLockLock(_deviceLock);
    }
}

unsigned IOHIKeyboard::eventFlags()
// Description:	Return global event flags In this world, there is only
//		one keyboard device so device flags == global flags.
{
    return _eventFlags;
}

unsigned IOHIKeyboard::deviceFlags()
// Description: Return per-device event flags. In this world, there is only
//		one keyboard device so device flags == global flags.
{
    return _eventFlags;
}

void IOHIKeyboard::setDeviceFlags(unsigned flags)
// Description: Set device event flags. In this world, there is only
//		one keyboard device so device flags == global flags.
{
    if (_eventFlags != flags) {
        _eventFlags = flags;
        
        // RY: On Modifier change, we should 
        // reset the auto repeat timer
        AbsoluteTime_to_scalar(&_downRepeatTime) = 0;
        _codeToRepeat = (unsigned)-1;
        scheduleAutoRepeat();
    }
}

bool IOHIKeyboard::alphaLock()
// Description: Return current alpha-lock state. This is a state tracking
//		callback used by the KeyMap object.
{
    return _alphaLock;
}

void IOHIKeyboard::setAlphaLock(bool val)
// Description: Set current alpha-lock state This is a state tracking
//		callback used by the KeyMap object.
{
    _alphaLock = val;
    setAlphaLockFeedback(val);

//    KeyboardReserved *tempReservedStruct = GetKeyboardReservedStructEventForService(this); 
//	
//    if (tempReservedStruct && tempReservedStruct->keyboardNub )
//        tempReservedStruct->keyboardNub->setCapsLockLEDElement(val);
    
}

bool IOHIKeyboard::numLock()
{
    return _numLock;
}

void IOHIKeyboard::setNumLock(bool val)
{
    _numLock = val;

    setNumLockFeedback(val);
	
//    if (tempReservedStruct && tempReservedStruct->keyboardNub )
//        tempReservedStruct->keyboardNub->setNumLockLEDElement(val);

}

bool IOHIKeyboard::charKeyActive()
// Description: Return true If a character generating key down This is a state
//		tracking callback used by the KeyMap object.
{
    return _charKeyActive;
}

void IOHIKeyboard::setCharKeyActive(bool val)
// Description: Note whether a char generating key is down. This is a state
//		tracking callback used by the KeyMap object.
{
    _charKeyActive = val;
}
//
// END:		Implementation of the methods required by IOHIKeyboardMapper.
//

void IOHIKeyboard::dispatchKeyboardEvent(unsigned int keyCode,
			 /* direction */ bool         goingDown,
                         /* timeStamp */ AbsoluteTime time)
// Description:	This method is the heart of event dispatching. The overlying
//		subclass invokes this method with each event. We then
//		get the event xlated and dispatched using a _keyMap instance.
//		The event structure passed in by reference should not be freed.
{
    IOHIKeyboardMapper	* theKeyMap;
    KeyboardReserved *tempReservedStruct = GetKeyboardReservedStructEventForService(this); 
	
    IOLockLock( _deviceLock);

    _lastEventTime = time;
    
    if (tempReservedStruct)
    {
#if 0
        if (tempReservedStruct->keyboardNub)
        {
            // Post the event to the HID Manager
            tempReservedStruct->keyboardNub->postKeyboardEvent(keyCode, goingDown);
        }
        if (tempReservedStruct->isSeized)
        {
            IOLockUnlock( _deviceLock);
            return;
        }
#endif
        
        tempReservedStruct->dispatchEventCalled = true;
    }

    if (_keyMap)  _keyMap->translateKeyCode(keyCode,
			  /* direction */ goingDown,
			  /* keyBits */   _keyState);
			  
    // remember the keymap while we hold the lock
    theKeyMap = _keyMap;

    if (tempReservedStruct) tempReservedStruct->dispatchEventCalled = false;
	
    IOLockUnlock( _deviceLock);
	
	// outside the lock (because of possible recursion), give the
	// keymap a chance to do some post processing
	// since it is possible we will be entered reentrantly and 
	// release the keymap, we will add a retain here.
    if (theKeyMap)
	{
		theKeyMap->retain();
		theKeyMap->keyEventPostProcess();
		theKeyMap->release();
	}
}

const unsigned char * IOHIKeyboard::defaultKeymapOfLength(UInt32 * length)
{
    *length = 0;
    return NULL;
}

void IOHIKeyboard::setAlphaLockFeedback(bool /* val */)
{
    return;
}

void IOHIKeyboard::setNumLockFeedback(bool /* val */)
{
    return;
}

UInt32 IOHIKeyboard::maxKeyCodes()
{
    return( NX_NUMKEYCODES );
}

bool IOHIKeyboard:: doesKeyLock ( unsigned key __unused)
{
	return false;
}

unsigned IOHIKeyboard:: getLEDStatus ()
{
	return 0;
}


bool IOHIKeyboard::open(IOService *			client,
                        IOOptionBits		  	options,
                        KeyboardEventAction		keAction,
                        KeyboardSpecialEventAction	kseAction,
                        UpdateEventFlagsAction		uefAction)
{
    if ( (!_keyMap) && (!resetKeyboard()))  return false;

    if (client == this) {
        KeyboardReserved *tempReservedStruct;
        tempReservedStruct = GetKeyboardReservedStructEventForService(this);
        
        return super::open((tempReservedStruct) ? tempReservedStruct->openClient : 0, options);
    }

    return open(client, 
                options,
                0,
                (KeyboardEventCallback)keAction, 
                (KeyboardSpecialEventCallback)kseAction, 
                (UpdateEventFlagsCallback)uefAction);
}

bool IOHIKeyboard::open(
                    IOService *                  client,
		    IOOptionBits	         options,
		    void *			 /*refcon*/,
                    KeyboardEventCallback        keCallback,
                    KeyboardSpecialEventCallback kseCallback,
                    UpdateEventFlagsCallback     uefCallback)
{
    if (client == this) return true;
        
    KeyboardReserved *tempReservedStruct;
    tempReservedStruct = GetKeyboardReservedStructEventForService(this);
    
    if (tempReservedStruct) tempReservedStruct->openClient = client;

    bool returnValue = open(this, options, 
                        (KeyboardEventAction)_keyboardEvent, 
                        (KeyboardSpecialEventAction)_keyboardSpecialEvent, 
                        (UpdateEventFlagsAction)_updateEventFlags);
                        
    if (!returnValue)
        return false;
    
    // point the new keymap to the IOHIDSystem, so it can set properties in it
    // this handles the case where although resetKeyboard is called above,
    // _keyboardEventTarget is as yet unset, so zero is passed
    if (_keyMap)               
        _keyMap->setKeyboardTarget(client);
    
    // Note: client object is already retained by superclass' open()
    _keyboardEventTarget        = client;
    _keyboardEventAction        = (KeyboardEventAction)keCallback;
    _keyboardSpecialEventTarget = client;
    _keyboardSpecialEventAction = (KeyboardSpecialEventAction)kseCallback;
    _updateEventFlagsTarget     = client;
    _updateEventFlagsAction     = (UpdateEventFlagsAction)uefCallback;
    
    return true;
}

void IOHIKeyboard::close(IOService * client, IOOptionBits)
{
    // kill autorepeat task
    // do this before we issue keyup for any other keys 
    // that are down.
    AbsoluteTime ts;
    clock_get_uptime(&ts);
    if (_codeToRepeat != ((unsigned)-1))
        dispatchKeyboardEvent(_codeToRepeat, false, ts);
    
    // now get rid of any other keys that might be down
    UInt32 i, maxKeys = maxKeyCodes();
    for (i=0; i<maxKeys; i++)
        if ( EVK_IS_KEYDOWN(i,_keyState) )
            dispatchKeyboardEvent(i, false, ts);

    // continue to issue zero'ed out flags changed events
    // just in case any of the flag bits were manually set
    _updateEventFlags(this, 0);

    _keyboardSpecialEvent(  this, 
                            NX_SYSDEFINED, 
                            0, 
                            NX_NOSPECIALKEY, 
                            NX_SUBTYPE_STICKYKEYS_RELEASE, 
                            _guid, 
                            0, 
                            _lastEventTime);

    bzero(_keyState, _keyStateSize);

    _keyboardEventAction        = NULL;
    _keyboardEventTarget        = 0;
    _keyboardSpecialEventAction = NULL;
    _keyboardSpecialEventTarget = 0;
    _updateEventFlagsAction     = NULL;
    _updateEventFlagsTarget     = 0;
    
    super::close(client);
}

IOReturn IOHIKeyboard::message( UInt32 type, IOService * provider,
                                void * argument) 
{
    IOReturn ret = kIOReturnSuccess;
    
    switch(type)
    {
        case kIOHIDSystem508MouseClickMessage:
        case kIOHIDSystem508SpecialKeyDownMessage:
            if (_keyMap)
                ret = _keyMap->message(type, this);
            break;
            
        case kIOHIDSystemDeviceSeizeRequestMessage:
#ifdef NEW_HID
            if (OSDynamicCast(IOHIDDevice, provider))
#else
            if (0)
#endif
            {
                KeyboardReserved *tempReservedStruct = GetKeyboardReservedStructEventForService(this);        
                
                if (tempReservedStruct) {
                    tempReservedStruct->isSeized = (bool)argument;
                }
            }
            break;

        default:
            ret = super::message(type, provider, argument);
            break;
    }
    
    return ret;
}

void IOHIKeyboard::_keyboardEvent( IOHIKeyboard * self,
			     unsigned   eventType,
      /* flags */            unsigned   flags,
      /* keyCode */          unsigned   key,
      /* charCode */         unsigned   charCode,
      /* charSet */          unsigned   charSet,
      /* originalCharCode */ unsigned   origCharCode,
      /* originalCharSet */  unsigned   origCharSet,
      /* keyboardType */     unsigned   keyboardType,
      /* repeat */           bool       repeat,
      /* atTime */           AbsoluteTime ts)
{
    if (!self || !self->_keyboardEventAction || !self->_keyboardEventTarget) {
        // nothing to be done
    }
    else {
        // this is skanky
        KeyboardEventCallback	keCallback = (KeyboardEventCallback)self->_keyboardEventAction;
        
        (*keCallback)(self->_keyboardEventTarget,
                      eventType,
                      flags,
                      key,
                      charCode,
                      charSet,
                      origCharCode,
                      origCharSet,
                      keyboardType,
                      repeat,
                      ts,
                      self,
                      0);
    }
}

void IOHIKeyboard::_keyboardSpecialEvent( 	
                             IOHIKeyboard * self,
                             unsigned   eventType,
        /* flags */          unsigned   flags,
        /* keyCode  */       unsigned   key,
        /* specialty */      unsigned   flavor,
        /* guid */           UInt64     guid,
        /* repeat */         bool       repeat,
        /* atTime */         AbsoluteTime ts)
{
    if (!self || !self->_keyboardSpecialEventAction || !self->_keyboardEventTarget) {
        // nothing to be done
    }
    else {
        // this is skanky
        KeyboardSpecialEventCallback kseCallback = (KeyboardSpecialEventCallback)self->_keyboardSpecialEventAction;
        
        (*kseCallback)(self->_keyboardEventTarget,
                       eventType,
                       flags,
                       key,
                       flavor,
                       guid,
                       repeat,
                       ts,
                       self,
                       0);
    }
}
        
void IOHIKeyboard::_updateEventFlags( IOHIKeyboard * self,
				unsigned flags)
{
    if (!self || !self->_keyboardSpecialEventAction || !self->_keyboardEventTarget) {
        // nothing to be done
    }
    else {
        // this is skanky
        UpdateEventFlagsCallback uefCallback = (UpdateEventFlagsCallback)self->_updateEventFlagsAction;
        if (uefCallback) {
            (*uefCallback)(self->_updateEventFlagsTarget,
                           flags,
                           self,
                           0);
        }
    }
}

/******************************************************************************/
void IOHIKeyboard::clearLastPageAndUsage()
{
    if (!_lastUsagePage && !_lastUsage)
        HIDLogError("called when not set %02x:%02x", _lastUsagePage, _lastUsage);
    _lastUsagePage = 0;
    _lastUsage = 0;
}

/******************************************************************************/
void IOHIKeyboard::setLastPageAndUsage(UInt16 usagePage, UInt16 usage)
{
    if (_lastUsagePage || _lastUsage)
        HIDLogError("called when already set %02x:%02x", _lastUsagePage, _lastUsage);
    _lastUsagePage = usagePage;
    _lastUsage = usage;
}

/******************************************************************************/
void IOHIKeyboard::getLastPageAndUsage(UInt16 &usagePage, UInt16 &usage)
{
    usagePage = _lastUsagePage;
    usage = _lastUsage;
}

/******************************************************************************/
