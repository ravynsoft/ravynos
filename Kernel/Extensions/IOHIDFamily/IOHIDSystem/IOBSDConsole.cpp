/*
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
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
#include <IOKit/assert.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOLib.h>
#include "IOBSDConsole.h"
#include <IOKit/hidsystem/IOHIKeyboard.h>
#include <IOKit/hidsystem/IOLLEvent.h>

#define super IOService
OSDefineMetaClassAndStructors(IOBSDConsole, IOService);

// remove
bool (*playBeep)(IOService *outputStream) = 0;

//************************************************************************

bool IOBSDConsole::start(IOService * provider)
{
    OSObject *	notify;

    if (!super::start(provider))  return false;

    notify = addNotification( gIOPublishNotification,
        serviceMatching("IOHIKeyboard"),
        (IOServiceNotificationHandler) &IOBSDConsole::publishNotificationHandler,
        this, 0 );
    assert( notify );

    notify = addNotification( gIOPublishNotification,
        serviceMatching("IODisplayWrangler"),
        (IOServiceNotificationHandler) &IOBSDConsole::publishNotificationHandler,
         this, 0 );
    assert( notify ); 

    notify = addNotification( gIOPublishNotification,
        serviceMatching("IOAudioStream"),
        (IOServiceNotificationHandler) &IOBSDConsole::publishNotificationHandler,
        this, this );
    assert( notify );

    kprintf("IOBSDConsole::start\n");
    return( true );
}

bool IOBSDConsole::publishNotificationHandler(
			    IOBSDConsole * self,
                            void * ref,
                            IOService * newService )

{
    IOHIKeyboard *	keyboard = 0;
    IOService *		audio = 0;

    if( ref) {
        audio = OSDynamicCast(IOService, newService->metaCast("IOAudioStream"));
        if (audio != 0) {
            OSNumber *out = OSDynamicCast(OSNumber, newService->copyProperty("Out"));
            if (OSDynamicCast(OSNumber, out)) {
                if (out->unsigned8BitValue() == 1) {
                    self->fAudioOut = newService;
                }
            }
            OSSafeReleaseNULL(out);
        }
    } else {
	audio = 0;
        keyboard = OSDynamicCast( IOHIKeyboard, newService );

        if( keyboard && self->attach( keyboard )) {
            self->arbitrateForKeyboard( keyboard );
        }

        if( newService->metaCast("IODisplayWrangler"))
            self->displayManager = newService;
    }

    return true;
}

//************************************************************************
// Keyboard client stuff
//************************************************************************

void IOBSDConsole::arbitrateForKeyboard( IOHIKeyboard * nub )
{
  nub->open(this, 0, 0,
	(KeyboardEventCallback)keyboardEvent, 
        (KeyboardSpecialEventCallback) 0, 
        (UpdateEventFlagsCallback)updateEventFlags);
  // failure can be expected if the HID system already has it
}

IOReturn IOBSDConsole::message(UInt32 type, IOService * provider,
				void * argument)
{
  IOReturn     status = kIOReturnSuccess;

  switch (type)
  {
    case kIOMessageServiceIsTerminated:
    case kIOMessageServiceIsRequestingClose:
      provider->close( this );
      break;

    case kIOMessageServiceWasClosed:
      arbitrateForKeyboard( (IOHIKeyboard *) provider );
      break;

    default:
      status = super::message(type, provider, argument);
      break;
  }

  return status;
}

extern "C" {
  void cons_cinput( char c);
}
//#warning REMOVE cons_cinput DECLARATION FROM HERE

void IOBSDConsole::keyboardEvent(OSObject * target,
          /* eventType */        unsigned   eventType,
          /* flags */            unsigned   flags,
          /* keyCode */          unsigned   /* key */,
          /* charCode */         unsigned   charCode,
          /* charSet */          unsigned   charSet,
          /* originalCharCode */ unsigned   /* origCharCode */,
          /* originalCharSet */  unsigned   /* origCharSet */,
          /* keyboardType */ 	 unsigned   /* keyboardType */,
          /* repeat */           bool       /* repeat */,
          /* atTime */           AbsoluteTime /* ts */,
                                 OSObject * sender,
                                 void *     refcon)
{
    kprintf("IOBSDConsole::keyboardEvent\n");

    static const char cursorCodes[] = { 'D', 'A', 'C', 'B' };

    if ( ((IOBSDConsole *)target)->displayManager != NULL ) {
        // if there is a display manager, tell it there is user activity
        ((IOBSDConsole *)target)->displayManager->activityTickle(kIOPMSuperclassPolicy1);
    }

    if( (eventType == NX_KEYDOWN) && ((flags & NX_ALTERNATEMASK) != NX_ALTERNATEMASK)) {
        if( (charSet == NX_SYMBOLSET)
            && (charCode >= 0xac) && (charCode <= 0xaf)) {
            cons_cinput( '\033');
            cons_cinput( 'O');
            charCode = cursorCodes[ charCode - 0xac ];
        }
        cons_cinput( charCode);
    }
}

void IOBSDConsole::updateEventFlags(OSObject * /*target*/, 
                                    unsigned /*flags*/,
                                    OSObject * /*sender*/,
                                    void *     /*refcon*/)
{
  return;
}


