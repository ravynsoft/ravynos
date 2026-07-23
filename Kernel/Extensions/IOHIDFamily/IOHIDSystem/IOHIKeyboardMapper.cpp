/*
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2009 Apple Computer, Inc.	 All Rights Reserved.
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
/*	Copyright (c) 1992 NeXT Computer, Inc.	All rights reserved.
 *
 * KeyMap.m - Generic keymap string parser and keycode translator.
 *
 * HISTORY
 * 19 June 1992	   Mike Paquette at NeXT
 *		Created.
 * 5  Aug 1993	  Erik Kay at NeXT
 *	minor API cleanup
 * 11 Nov 1993	  Erik Kay at NeXT
 *	fix to allow prevent long sequences from overflowing the event queue
 * 12 Nov 1998	  Dan Markarian at Apple
 *		major cleanup of public API's; converted to C++
 */

#include <sys/systm.h>

#include <IOKit/assert.h>
#include <IOKit/IOLib.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/hidsystem/IOLLEvent.h>
#include <IOKit/hidsystem/IOHIKeyboard.h>
#include <IOKit/hidsystem/IOHIKeyboardMapper.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include <IOKit/hidsystem/IOHIDSystem.h>
#include <libkern/OSByteOrder.h>
#ifdef NEW_HID
#include "IOHIDKeyboardDevice.h"
#endif
#include "IOHIDevicePrivateKeys.h"
#include "IOHIDFamilyPrivate.h"

// Define expansion data here
#define _specialKeyModifierFlags		_reserved->specialKeyModifierFlags
#define _cachedAlphaLockModDefs		_reserved->cachedAlphaLockModDefs

#define super OSObject
OSDefineMetaClassAndStructors(IOHIKeyboardMapper, OSObject);

// sticky keys private state flags
enum
{
	kState_OptionActivates_Flag = 0x0010,	// the 'on' gesture (5 options) will activate mouse keys
	kState_ClearHeldKeysFirst	= 0x0100,	// when set, we should clear all held keys
												// this is a hack we are using since we
												// cannot post key up events when our
												// entry point is not a key event

	kState_PrefFnKeyStateOn					= 0x0200,
	kState_StickyFnKeyStateOn				= 0x0400,
	kState_MouseKeyStateOn					= 0x0800,
	kState_StickyFnKeyStateChangePending	= 0x1000,

};



// delay filter private state flags
enum
{
	kState_Aborted_Flag		= 0x0200,
	kState_In_Progess_Flag	= 0x0400,
	kState_Is_Repeat_Flag	= 0x0800,
};

// ADB Key code for F12
#define kADB_KEYBOARD_F12 0x6f

// Shortcut for post slow key translation
#define postSlowKeyTranslateKeyCode(owner,key,keyDown,keyBits)	\
	if (!owner->f12EjectFilterKey(key, keyDown, keyBits))	\
		if (!owner->stickyKeysFilterKey(key, keyDown, keyBits)) \
			owner->rawTranslateKeyCode(key, keyDown, keyBits);


// Shortcut for determining if we are interested in this modifier
#define modifierOfInterest(keyBits) \
			((keyBits & NX_MODMASK) && \
			((((keyBits & NX_WHICHMODMASK) >= NX_MODIFIERKEY_SHIFT) && \
			((keyBits & NX_WHICHMODMASK) <= NX_MODIFIERKEY_COMMAND)) || \
			(((keyBits & NX_WHICHMODMASK) >= NX_MODIFIERKEY_RSHIFT) && \
			((keyBits & NX_WHICHMODMASK) <= NX_MODIFIERKEY_LAST_KEY)) || \
			((keyBits & NX_WHICHMODMASK) == NX_MODIFIERKEY_SECONDARYFN)))

#define mouseKey(keyBits) \
			((keyBits & NX_MODMASK) && \
			 ((keyBits & NX_WHICHMODMASK) == NX_MODIFIERKEY_NUMERICPAD))

#define mouseKeyToIgnore(keyBits, key) \
			( mouseKey(keyBits) && \
			 (((key >= 0x52) && (key <= 0x56)) || \
			 ((key >= 0x58) && (key <= 0x5c))) )

#define convertToLeftModBit(modBit) \
		modBit -= ((modBit >= NX_MODIFIERKEY_RSHIFT) && \
					(modBit <= NX_MODIFIERKEY_LAST_KEY)) ? 8 : 0;

static UInt32 DeviceModifierMasks[NX_NUMMODIFIERS] =
{
  /* NX_MODIFIERKEY_ALPHALOCK */	0,
  /* NX_MODIFIERKEY_SHIFT */		NX_DEVICELSHIFTKEYMASK,
  /* NX_MODIFIERKEY_CONTROL */		NX_DEVICELCTLKEYMASK,
  /* NX_MODIFIERKEY_ALTERNATE */	NX_DEVICELALTKEYMASK,
  /* NX_MODIFIERKEY_COMMAND */		NX_DEVICELCMDKEYMASK,
  /* NX_MODIFIERKEY_NUMERICPAD */	0,
  /* NX_MODIFIERKEY_HELP */		0,
  /* NX_MODIFIERKEY_SECONDARYFN */	0,
  /* NX_MODIFIERKEY_NUMLOCK */		0,
  /* NX_MODIFIERKEY_RSHIFT */		NX_DEVICERSHIFTKEYMASK,
  /* NX_MODIFIERKEY_RCONTROL */		NX_DEVICERCTLKEYMASK,
  /* NX_MODIFIERKEY_RALTERNATE */	NX_DEVICERALTKEYMASK,
  /* NX_MODIFIERKEY_RCOMMAND */		NX_DEVICERCMDKEYMASK,
  /* NX_MODIFIERKEY_ALPHALOCK_STATELESS */  NX_DEVICE_ALPHASHIFT_STATELESS_MASK,
  0,
  0
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

IOHIKeyboardMapper * IOHIKeyboardMapper::keyboardMapper(
										IOHIKeyboard * delegate,
										const UInt8 *  mapping,
										UInt32		   mappingLength,
										bool		   mappingShouldBeFreed )
{
	IOHIKeyboardMapper * me = new IOHIKeyboardMapper;

	if (!me && mappingShouldBeFreed && mapping)
		IOFree( (void*)mapping, mappingLength );

	if (me && !me->init(delegate, mapping, mappingLength, mappingShouldBeFreed))
	{
		me->release();
		return 0;
	}

	return me;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/*
 * Common KeyMap initialization
 */
bool IOHIKeyboardMapper::init(	IOHIKeyboard *delegate,
								const UInt8 *map,
								UInt32 mappingLen,
								bool mappingShouldBeFreed )
{
	_mappingShouldBeFreed		= mappingShouldBeFreed;
	_parsedMapping.mapping		= map;
	_parsedMapping.mappingLen	= mappingLen;

	if (!super::init())	 return false;

	_delegate				  = delegate;

	if (!parseKeyMapping(map, mappingLen, &_parsedMapping))	return false;

//	_hidSystem					= NULL;
//	_stateDirty					= false;

	_reserved = IONew(ExpansionData, 1);
    bzero(_reserved, sizeof(ExpansionData));

//	_ejectTimerEventSource		= 0;
//
//	_f12Eject_State			= 0;
//
//	_eject_Delay_MS			= kEjectF12DelayMS;
//
//	_slowKeys_State			= 0;
//
//	_slowKeys_Delay_MS		= 0;
//
//	_slowKeysTimerEventSource	= 0;

	_specialKeyModifierFlags	= 0;

//	_supportsF12Eject		= 0;

//	_cached_KeyBits			= 0;

	_cachedAlphaLockModDefs = 0;

	// If there are right hand modifiers defined, set a property
	if (_delegate && (_parsedMapping.maxMod > 0))
	{

		if ( _delegate->doesKeyLock(NX_KEYTYPE_CAPS_LOCK) )
		{
			_delegate->setProperty( kIOHIDKeyboardCapsLockDoesLockKey, kOSBooleanTrue);
			_cachedAlphaLockModDefs = _parsedMapping.modDefs[NX_MODIFIERKEY_ALPHALOCK];
		}
		else
		{
			_delegate->setProperty( kIOHIDKeyboardCapsLockDoesLockKey, kOSBooleanFalse);
		}

		UInt32 supportedModifiers = 0;
		OSNumber * number = 0;

		number = (OSNumber *)_delegate->copyProperty(kIOHIDKeyboardSupportedModifiersKey);

		if (number) supportedModifiers = number->unsigned32BitValue();
		OSSafeReleaseNULL(number);

		_delegate->setProperty( kIOHIDKeyboardSupportedModifiersKey, supportedModifiers, 32 );

		if ( (supportedModifiers & NX_DEVICERSHIFTKEYMASK) ||
			 (supportedModifiers & NX_DEVICERCTLKEYMASK) ||
			 (supportedModifiers & NX_DEVICERALTKEYMASK) ||
			 (supportedModifiers & NX_DEVICERCMDKEYMASK) )
		{
			_delegate->setProperty("HIDKeyboardRightModifierSupport", kOSBooleanTrue);
		}
	}

	if (_parsedMapping.numDefs && _delegate)
	{
		_delegate->setProperty("HIDKeyboardKeysDefined", kOSBooleanTrue);

	}

	if ( _delegate && !_delegate->doesKeyLock(NX_KEYTYPE_CAPS_LOCK) )
	{
		UInt32 myFlags = _delegate->deviceFlags();

		if ( _delegate->alphaLock() )
		{
			_specialKeyModifierFlags	|= NX_ALPHASHIFTMASK;
			myFlags						|= NX_ALPHASHIFTMASK;

			_delegate->IOHIKeyboard::setDeviceFlags(myFlags);
		}
		else
		{
			_specialKeyModifierFlags	&= ~NX_ALPHASHIFTMASK;
			myFlags						&= ~NX_ALPHASHIFTMASK;

			_delegate->IOHIKeyboard::setDeviceFlags(myFlags);
		}
	}

    return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void IOHIKeyboardMapper::free()
{
    if (!_parsedMapping.mapping || !_parsedMapping.mappingLen)
        return;


    if (_reserved) {
        
        IODelete(_reserved, ExpansionData, 1);
    }

    if (_mappingShouldBeFreed)
        IOFree((void *)_parsedMapping.mapping, _parsedMapping.mappingLen);

    super::free();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

const UInt8 * IOHIKeyboardMapper::mapping()
{
	return (const UInt8 *)_parsedMapping.mapping;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

UInt32 IOHIKeyboardMapper::mappingLength()
{
  return _parsedMapping.mappingLen;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool IOHIKeyboardMapper::serialize(OSSerialize *s) const
{
	OSData * data;
	bool ok;

	if (s->previouslySerialized(this)) return true;

	data = OSData::withBytesNoCopy( (void *) _parsedMapping.mapping, _parsedMapping.mappingLen );
	if (data) {
	ok = data->serialize(s);
	data->release();
	} else
	ok = false;

	return( ok );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//
// Perform the mapping of 'key' moving in the specified direction
// into events.
//

void IOHIKeyboardMapper::translateKeyCode(UInt8		   key,
										  bool		   keyDown,
										  kbdBitVector keyBits)
{
    rawTranslateKeyCode(key, keyDown, keyBits);
}


// rawTranslateKeyCode is the original translateKeyCode function,
//	prior to the Stickykeys feature
//
// Perform the mapping of 'key' moving in the specified direction
// into events.
//
void IOHIKeyboardMapper::rawTranslateKeyCode(UInt8 key,
											bool keyDown,
											kbdBitVector keyBits)
{
	unsigned char thisBits = _parsedMapping.keyBits[key];

	/* do mod bit update and char generation in useful order */
	if (keyDown)
	{
		EVK_KEYDOWN(key, keyBits);

		if (thisBits & NX_MODMASK)		doModCalc(key, keyBits);
		if (thisBits & NX_CHARGENMASK)	doCharGen(key, keyDown);
	}
	else
	{
		EVK_KEYUP(key, keyBits);
		if (thisBits & NX_CHARGENMASK)	doCharGen(key, keyDown);
		if (thisBits & NX_MODMASK)		doModCalc(key, keyBits);
	}

	// Fix JIS localization. We are here because the JIS keys Yen, Ro, Eisu,
	// Kana, and "," are not matched in _parsedMapping.keyBits[] above even
	// though the keyboard drivers are sending the correct scan codes.
	// The check for interfaceID() below makes sure both ADB and USB works.
	// This fix has been tested with AppKit and Carbon for Kodiak 1H
	if( 0 == (thisBits & (NX_MODMASK | NX_CHARGENMASK)))
		if (_delegate->interfaceID() == NX_EVS_DEVICE_INTERFACE_ADB)
	{
			unsigned charCode=0;

			switch (key) {
			case 0x5F:	// numpad ',' using raw ADB scan code
				charCode = ',';
				break;
			case 0x5E:	//ro
				charCode = '_';
				break;
			case 0x5d:	//Yen
				charCode = '\\';
				break;
			case 0x0a:
				charCode = 0xa7;
				break;
			case 0x66:	// eisu
			case 0x68:	// kana
			default:
				// do nothing. AppKit has fix in 1H
				break;
			}
			/* Post the keyboard event */
			_delegate->keyboardEvent(keyDown ? NX_KEYDOWN : NX_KEYUP,
				/* flags */				_delegate->eventFlags(),
				/* keyCode */			key,
				/* charCode */			charCode,
				/* charSet */			0,	//0 is adequate for JIS
				/* originalCharCode */	0,
				/* originalCharSet */	0);
	}

#ifdef OMITPENDINGKEYCAPS
	unsigned char *	bp;

	// Make KeyCaps.app see the caps lock
	if (key == _parsedMapping.specialKeys[NX_KEYTYPE_CAPS_LOCK])	//ADB caps lock 0x39
	{
		if (_delegate->alphaLock() == keyDown)
		//This logic is needed for non-locking USB caps lock
		{
		_delegate->keyboardEvent(keyDown ? NX_KEYDOWN : NX_KEYUP,
			_delegate->eventFlags(), key, 0, 0, 0, 0);
		}
	}

		//Find scan code corresponding to PowerBook fn key (0x3f in ADB)
		bp = _parsedMapping.modDefs[NX_MODIFIERKEY_SECONDARYFN]; //7th array entry
		if (bp)
		{
		bp++; //now points to actual ADB scan code
		if (key == *bp ) //ADB fn key should be 0x3f here
		{
			_delegate->keyboardEvent(keyDown ? NX_KEYDOWN : NX_KEYUP,
			_delegate->eventFlags(), key, 0, 0, 0, 0);
		}
		}
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//
// Support goop for parseKeyMapping.  These routines are
// used to walk through the keymapping string.	The string
// may be composed of bytes or shorts.	If using shorts, it
// MUST always be aligned to use short boundries.
//
typedef struct {
	unsigned const char *bp;
	unsigned const char *endPtr;
	int shorts;
} NewMappingData;

static inline unsigned int NextNum(NewMappingData *nmd)
{
	if (nmd->bp >= nmd->endPtr)
		return(0);
	if (nmd->shorts) {
        unsigned short tmp = *((unsigned short *)nmd->bp);
        nmd->bp += 2;
		return OSSwapBigToHostInt16(tmp);
}
	else {
        unsigned char tmp = *(nmd->bp);
        nmd->bp++;
        return tmp;
    }
}

//
// Perform the actual parsing operation on a keymap.  Returns false on failure.
//

bool IOHIKeyboardMapper::parseKeyMapping(const UInt8 *		  map,
										 UInt32				  mappingLen,
									 NXParsedKeyMapping * parsedMapping) const
{
	NewMappingData nmd;
	int i, j, k, l, n;
	unsigned int m;
	int keyMask, numMods;
	int maxSeqNum = -1;
		unsigned char *			bp;


	/* Initialize the new map. */
	bzero( parsedMapping, sizeof (NXParsedKeyMapping) );
	parsedMapping->maxMod = -1;
	parsedMapping->numDefs = -1;
	parsedMapping->numSeqs = -1;

		if (!map || !mappingLen)
			return false;

	nmd.endPtr = map + mappingLen;
	nmd.bp = map;
	nmd.shorts = 1;		// First value, the size, is always a short

	/* Start filling it in with the new data */
	parsedMapping->mapping = (unsigned char *)map;
	parsedMapping->mappingLen = mappingLen;
	parsedMapping->shorts = nmd.shorts = NextNum(&nmd);

	/* Walk through the modifier definitions */
	numMods = NextNum(&nmd);
	for(i=0; i<numMods; i++)
	{
		/* Get bit number */
		if ((j = NextNum(&nmd)) >= NX_NUMMODIFIERS)
		return false;

		/* Check maxMod */
		if (j > parsedMapping->maxMod)
		parsedMapping->maxMod = j;

		/* record position of this def */
		parsedMapping->modDefs[j] = (unsigned char *)nmd.bp;

		/* Loop through each key assigned to this bit */
		for(k=0,n = NextNum(&nmd);k<n;k++)
		{
		/* Check that key code is valid */
		if ((l = NextNum(&nmd)) >= NX_NUMKEYCODES)
			return false;
		/* Make sure the key's not already assigned */
		if (parsedMapping->keyBits[l] & NX_MODMASK)
			return false;
		/* Set bit for modifier and which one */

		//The "if" here is to patch the keymapping file.  That file has nothing
		// for num lock, so no change is required here for num lock.
		// Also, laptop Macs have num lock handled by Buttons driver
		if ((j != NX_MODIFIERKEY_ALPHALOCK) || (_delegate->doesKeyLock(NX_KEYTYPE_CAPS_LOCK)) )
		{
			parsedMapping->keyBits[l] |=NX_MODMASK | (j & NX_WHICHMODMASK);
		}

		}
	}

	//This is here because keymapping file has an entry for caps lock, but in
	//	order to trigger special code (line 646-), the entry needs to be zero
	if (!_delegate->doesKeyLock(NX_KEYTYPE_CAPS_LOCK))
		parsedMapping->modDefs[NX_MODIFIERKEY_ALPHALOCK] = 0;

	//This section is here to force keymapping to include the PowerBook's secondary
	// fn key as a new modifier key.  This code can be removed once the keymapping
	// file has the fn key (ADB=0x3f) in the modifiers section.
	// NX_MODIFIERKEY_SECONDARYFN = 8 in ev_keymap.h
	if (_delegate->interfaceID() == NX_EVS_DEVICE_INTERFACE_ADB)
	{
		parsedMapping->keyBits[0x3f] |=NX_MODMASK | (NX_MODIFIERKEY_SECONDARYFN & NX_WHICHMODMASK);
	}

	/* Walk through each key definition */
	parsedMapping->numDefs = NextNum(&nmd);
	n = parsedMapping->numDefs;
	for( i=0; i < NX_NUMKEYCODES; i++)
	{
		if (i < n)
		{
		parsedMapping->keyDefs[i] = (unsigned char *)nmd.bp;
		if ((keyMask = NextNum(&nmd)) != (nmd.shorts ? 0xFFFF: 0x00FF))
		{
			/* Set char gen bit for this guy: not a no-op */
			parsedMapping->keyBits[i] |= NX_CHARGENMASK;
			/* Check key defs to find max sequence number */
			for(j=0, k=1; j<=parsedMapping->maxMod; j++, keyMask>>=1)
			{
				if (keyMask & 0x01)
				k*= 2;
			}
			for(j=0; j<k; j++)
			{
			m = NextNum(&nmd);
			l = NextNum(&nmd);
			if (m == (unsigned)(nmd.shorts ? 0xFFFF: 0x00FF))
				if (((int)l) > maxSeqNum)
				maxSeqNum = l;	/* Update expected # of seqs */
			}
		}
		else /* unused code within active range */
			parsedMapping->keyDefs[i] = NULL;
		}
		else /* Unused code past active range */
		{
		parsedMapping->keyDefs[i] = NULL;
		}
	}
	/* Walk through sequence defs */
	parsedMapping->numSeqs = NextNum(&nmd);
		/* If the map calls more sequences than are declared, bail out */
	if (parsedMapping->numSeqs <= maxSeqNum)
		return false;
    
    if (parsedMapping->numSeqs > NX_NUMSEQUENCES)
        return false;
    
	/* Walk past all sequences */
	for(i = 0; i < parsedMapping->numSeqs; i++)
	{
		parsedMapping->seqDefs[i] = (unsigned char *)nmd.bp;
		/* Walk thru entries in a seq. */
		for(j=0, l=NextNum(&nmd); j<l; j++)
		{
		NextNum(&nmd);
		NextNum(&nmd);
		}
	}
	/* Install Special device keys.	 These override default values. */
	numMods = NextNum(&nmd);	/* Zero on old style keymaps */
	parsedMapping->numSpecialKeys = numMods;
	if ( numMods > NX_NUMSPECIALKEYS )
		return false;
	if ( numMods )
	{
		for ( i = 0; i < NX_NUMSPECIALKEYS; ++i )
		parsedMapping->specialKeys[i] = NX_NOSPECIALKEY;

			//This "if" will cover both ADB and USB keyboards.	This code does not
			//	have to be here if the keymaps include these two entries.  Keyboard
		//	drivers already have these entries, but keymapping file does not
		if (_delegate->interfaceID() == NX_EVS_DEVICE_INTERFACE_ADB)
		{
		//ADB capslock:
			parsedMapping->specialKeys[NX_KEYTYPE_CAPS_LOCK] = 0x39;

		//ADB numlock for external keyboards, not PowerBook keyboards:
			parsedMapping->specialKeys[NX_KEYTYPE_NUM_LOCK] = 0x47;

		//HELP key needs to be visible
		parsedMapping->keyDefs[0x72] = parsedMapping->keyDefs[0x47];
		}

		//Keymapping file can override caps and num lock above now:
		for ( i = 0; i < numMods; ++i )
		{
		j = NextNum(&nmd);	/* Which modifier key? */
		l = NextNum(&nmd);	/* Scancode for modifier key */
		if ( j >= NX_NUMSPECIALKEYS )
			return false;
		parsedMapping->specialKeys[j] = l;
		}
	}
	else  /* No special keys defs implies an old style keymap */
	{
		return false;	/* Old style keymaps are guaranteed to do */
				/* the wrong thing on ADB keyboards */
	}

	/* Install bits for Special device keys */
	for(i=0; i<NX_NUM_SCANNED_SPECIALKEYS; i++)
	{
		if ( parsedMapping->specialKeys[i] != NX_NOSPECIALKEY )
		{
			if (parsedMapping->specialKeys[i] < NX_NUMKEYCODES)
				parsedMapping->keyBits[parsedMapping->specialKeys[i]] |= (NX_CHARGENMASK | NX_SPECIALKEYMASK);
		}
	}

	//caps lock keys should not generate characters.
	if (_delegate->doesKeyLock(NX_KEYTYPE_CAPS_LOCK))
	{
		if (parsedMapping->specialKeys[NX_KEYTYPE_CAPS_LOCK] < NX_NUMKEYCODES)
			parsedMapping->keyBits[ parsedMapping->specialKeys[NX_KEYTYPE_CAPS_LOCK] ] &= ~NX_CHARGENMASK;
	}

	//Find scan code corresponding to PowerBook fn key (0x3f in ADB)
	//	 and then make sure it does not generate a character
	bp = _parsedMapping.modDefs[NX_MODIFIERKEY_SECONDARYFN];  //7th array entry
	if (bp)
	{
			bp++;  //now points to actual ADB scan code
			parsedMapping->keyBits[ *bp ] &= ~NX_CHARGENMASK;
	}

    if (parsedMapping->specialKeys[NX_MODIFIERKEY_ALPHALOCK_STATELESS] == NX_NOSPECIALKEY) {
        // check value of keyDefs
        unsigned char key = NX_NUMKEYCODES - 1;
        parsedMapping->specialKeys[NX_MODIFIERKEY_ALPHALOCK_STATELESS] = key;
        parsedMapping->modDefs[NX_MODIFIERKEY_ALPHALOCK_STATELESS] = 0;
        parsedMapping->keyBits[key] = NX_MODIFIERKEY_ALPHALOCK_STATELESS | NX_MODMASK;
    }
    else {
        //HIDLogError("Stateless alpha lock defined in mapping as %02x\n", parsedMapping->specialKeys[NX_MODIFIERKEY_ALPHALOCK_STATELESS]);
    }

	return true;
}


//Retrieve a key from mapping above.  Useful for IOHIKeyboard
UInt8 IOHIKeyboardMapper::getParsedSpecialKey(UInt8 logical)
{
	UInt8	retval;

	if ( logical < NX_NUMSPECIALKEYS)
	retval = _parsedMapping.specialKeys[logical];
	else
	retval = 0xff;	//careful, 0 is mapped already
	return retval;
}


static inline int NEXTNUM(unsigned char ** mapping, short shorts)
{
	int returnValue;

	if (shorts)
	{
		returnValue = OSSwapBigToHostInt16(*((unsigned short *)*mapping));
		*mapping += sizeof(unsigned short);
	}
	else
	{
		returnValue = **((unsigned char	 **)mapping);
		*mapping += sizeof(unsigned char);
	}

	return returnValue;
}

//
// Look up in the keymapping each key associated with the modifier bit.
// Look in the device state to see if that key is down.
// Return 1 if a key for modifier 'bit' is down.  Return 0 if none is down
//
static inline int IsModifierDown(NXParsedKeyMapping *parsedMapping,
				 kbdBitVector keyBits,
				 int bit )
{
	int i, n;
	unsigned char *mapping;
	unsigned key;
	short shorts = parsedMapping->shorts;

	if ( (mapping = parsedMapping->modDefs[bit]) != 0 ) {
	for(i=0, n=NEXTNUM(&mapping, shorts); i<n; i++)
	{
		key = NEXTNUM(&mapping, shorts);
		if ( EVK_IS_KEYDOWN(key, keyBits) )
		return 1;
	}
	}
	return 0;
}

void IOHIKeyboardMapper::calcModBit(int bit, kbdBitVector keyBits)
{
		int			otherHandBit	= 0;
		int			deviceBitMask	= 0;
	int		systemBitMask	= 0;
	unsigned	myFlags		= 0;

		systemBitMask = 1<<(bit+16);
		deviceBitMask = DeviceModifierMasks[bit];

		if ((bit >= NX_MODIFIERKEY_RSHIFT) && (bit <= NX_MODIFIERKEY_RCOMMAND))
		{
			otherHandBit = bit - 8;
			systemBitMask = 1<<(otherHandBit+16);
		}
		else if ((bit >= NX_MODIFIERKEY_SHIFT) && (bit <= NX_MODIFIERKEY_COMMAND))
		{
			otherHandBit = bit + 8;
		}

		/* Initially clear bit, as if key-up */
		myFlags = _delegate->deviceFlags() & (~systemBitMask);
		myFlags &= ~deviceBitMask;

		/* Set bit if any associated keys are down */
		if ( IsModifierDown( &_parsedMapping, keyBits, bit ))
		{
				myFlags |= (systemBitMask | deviceBitMask);
		}
		else if (deviceBitMask &&
				IsModifierDown( &_parsedMapping, keyBits, otherHandBit ))
		{
				myFlags |= (systemBitMask);
		}

		myFlags |= _specialKeyModifierFlags;

	if ( bit == NX_MODIFIERKEY_ALPHALOCK ) {/* Caps Lock key */
		_delegate->setAlphaLock((myFlags & NX_ALPHASHIFTMASK) ? true : false);
    }
	else if ( bit == NX_MODIFIERKEY_NUMLOCK ) {/* Num Lock key */
			_delegate->setNumLock((myFlags & NX_NUMERICPADMASK) ? true : false);
	}

	_delegate->setDeviceFlags(myFlags);

}


//
// Perform flag state update and generate flags changed events for this key.
//
void IOHIKeyboardMapper::doModCalc(int key, kbdBitVector keyBits)
{
	int thisBits;
	thisBits = _parsedMapping.keyBits[key];
	if (thisBits & NX_MODMASK)
	{
	calcModBit((thisBits & NX_WHICHMODMASK), keyBits);
	/* The driver generates flags-changed events only when there is
	   no key-down or key-up event generated */
	if (!(thisBits & NX_CHARGENMASK))
	{
		/* Post the flags-changed event */
		_delegate->keyboardEvent(NX_FLAGSCHANGED,
		 /* flags */			_delegate->eventFlags(),
		 /* keyCode */			key,
		 /* charCode */			0,
		 /* charSet */			0,
		 /* originalCharCode */ 0,
		 /* originalCharSet */	0);
#ifdef NEW_HID
		_delegate->keyboardEvent(EVK_IS_KEYDOWN(key, keyBits) ? NX_KEYDOWN : NX_KEYUP,
		 /* flags */			_delegate->eventFlags(),
		 /* keyCode */			key,
		 /* charCode */			0,
		 /* charSet */			0,
		 /* originalCharCode */ 0,
		 /* originalCharSet */	0);
#endif
	}
	else	/* Update, but don't generate an event */
		_delegate->updateEventFlags(_delegate->eventFlags());
	}
}

//
// Perform character event generation for this key
//
void IOHIKeyboardMapper::doCharGen(int keyCode, bool down)
{
	int i, n, eventType, adjust, thisMask, modifiers, saveModifiers;
	short shorts;
	unsigned charSet, origCharSet;
	unsigned charCode, origCharCode;
    unsigned char *map;
	unsigned eventFlags, origflags;

	_delegate->setCharKeyActive(true);	// a character generating key is active

	eventType = (down == true) ? NX_KEYDOWN : NX_KEYUP;
	eventFlags = _delegate->eventFlags();
	saveModifiers = eventFlags >> 16;	// machine independent mod bits
	/* Set NX_ALPHASHIFTMASK based on alphaLock OR shift active */
	if( saveModifiers & (NX_SHIFTMASK >> 16))
	saveModifiers |= (NX_ALPHASHIFTMASK >> 16);


	/* Get this key's key mapping */
	shorts = _parsedMapping.shorts;
    map = _parsedMapping.keyDefs[keyCode];
	modifiers = saveModifiers;
    if ( map ) {


	/* Build offset for this key */
        thisMask = NEXTNUM(&map, shorts);
        if (thisMask && modifiers) {
		adjust = (shorts ? sizeof(short) : sizeof(char))*2;
            for ( i = 0; i <= _parsedMapping.maxMod; ++i) {
                if (thisMask & 0x01) {
			if (modifiers & 0x01)
                        map += adjust;
			adjust *= 2;
		}
		thisMask >>= 1;
		modifiers >>= 1;
		}
	}
        charSet = NEXTNUM(&map, shorts);
        charCode = NEXTNUM(&map, shorts);

	/* construct "unmodified" character */
        map = _parsedMapping.keyDefs[keyCode];
		modifiers = saveModifiers & ((NX_ALPHASHIFTMASK | NX_SHIFTMASK) >> 16);

        thisMask = NEXTNUM(&map, shorts);
        if (thisMask && modifiers) {
		adjust = (shorts ? sizeof(short) : sizeof(char)) * 2;
            for ( i = 0; i <= _parsedMapping.maxMod; ++i) {
                if (thisMask & 0x01) {
			if (modifiers & 0x01)
                        map += adjust;
			adjust *= 2;
		}
		thisMask >>= 1;
		modifiers >>= 1;
		}
	}
        origCharSet = NEXTNUM(&map, shorts);
        origCharCode = NEXTNUM(&map, shorts);

        if (charSet == (unsigned)(shorts ? 0xFFFF : 0x00FF)) {
		// Process as a character sequence
		// charCode holds the sequence number
            map = _parsedMapping.seqDefs[charCode];

		origflags = eventFlags;
            for (i=0,n=NEXTNUM(&map, shorts);i<n;i++) {
                if ( (charSet = NEXTNUM(&map, shorts)) == 0xFF ) { /* metakey */
                    if ( down == true ) { /* down or repeat */
                        eventFlags |= (1 << (NEXTNUM(&map, shorts) + 16));
			_delegate->keyboardEvent(NX_FLAGSCHANGED,
			 /* flags */			_delegate->deviceFlags(),
			 /* keyCode */			keyCode,
			 /* charCode */			0,
			 /* charSet */			0,
			 /* originalCharCode */ 0,
			 /* originalCharSet */	0);
			}
			else
                        NEXTNUM(&map, shorts); /* Skip over value */
		}
                else {
                    charCode = NEXTNUM(&map, shorts);
			_delegate->keyboardEvent(eventType,
			 /* flags */			eventFlags,
			 /* keyCode */			keyCode,
			 /* charCode */			charCode,
			 /* charSet */			charSet,
			 /* originalCharCode */ charCode,
			 /* originalCharSet */	charSet);
		}
		}
		/* Done with macro.	 Restore the flags if needed. */
            if ( eventFlags != origflags ) {
		_delegate->keyboardEvent(NX_FLAGSCHANGED,
		 /* flags */			_delegate->deviceFlags(),
		 /* keyCode */			keyCode,
		 /* charCode */			0,
		 /* charSet */			0,
		 /* originalCharCode */ 0,
		 /* originalCharSet */	0);
		eventFlags = origflags;
		}
	}
        else { /* A simple character generating key */
		_delegate->keyboardEvent(eventType,
		 /* flags */			eventFlags,
		 /* keyCode */			keyCode,
		 /* charCode */			charCode,
		 /* charSet */			charSet,
		 /* originalCharCode */ origCharCode,
		 /* originalCharSet */	origCharSet);
	}
    } /* if (map) */

	/*
	 * Check for a device control key: note that they always have CHARGEN
	 * bit set
	 */
    if (_parsedMapping.keyBits[keyCode] & NX_SPECIALKEYMASK) {
        for (i=0; i<NX_NUM_SCANNED_SPECIALKEYS; i++) {
            if ( keyCode == _parsedMapping.specialKeys[i] ) {
		_delegate->keyboardSpecialEvent(eventType,
					/* flags */		eventFlags,
					/* keyCode */	keyCode,
					/* specialty */ i);
		/*
		 * Special keys hack for letting an arbitrary (non-locking)
		 * key act as a CAPS-LOCK key.	If a special CAPS LOCK key
		 * is designated, and there is no key designated for the
		 * AlphaLock function, then we'll let the special key toggle
		 * the AlphaLock state.
		 */
		if (i == NX_KEYTYPE_CAPS_LOCK
			&& down == true
                        && !_parsedMapping.modDefs[NX_MODIFIERKEY_ALPHALOCK] ) {
			unsigned myFlags = _delegate->deviceFlags();
			bool alphaLock = (_delegate->alphaLock() == false);

			// Set delegate's alphaLock state
			_delegate->setAlphaLock(alphaLock);
			// Update the delegate's flags
                    if ( alphaLock ) {
				myFlags |= NX_ALPHASHIFTMASK;
						_specialKeyModifierFlags |= NX_ALPHASHIFTMASK;
					}
                    else {
				myFlags &= ~NX_ALPHASHIFTMASK;
						_specialKeyModifierFlags &= ~NX_ALPHASHIFTMASK;
					}

			_delegate->setDeviceFlags(myFlags);

			_delegate->keyboardEvent(NX_FLAGSCHANGED,
			 /* flags */			myFlags,
			 /* keyCode */			keyCode,
			 /* charCode */			0,
			 /* charSet */			0,
			 /* originalCharCode */ 0,
			 /* originalCharSet */	0);

#ifdef NEW_HID
			_delegate->keyboardEvent(alphaLock ? NX_KEYDOWN : NX_KEYUP,
			 /* flags */			myFlags,
			 /* keyCode */			keyCode,
			 /* charCode */			0,
			 /* charSet */			0,
			 /* originalCharCode */ 0,
			 /* originalCharSet */	0);
#endif
		}
		else	if (i == NX_KEYTYPE_NUM_LOCK
			&& down == true
					&& (_delegate->doesKeyLock(NX_KEYTYPE_NUM_LOCK) || _delegate->metaCast("AppleADBButtons"))
                         && !_parsedMapping.modDefs[NX_MODIFIERKEY_NUMLOCK] ) {
			unsigned myFlags = _delegate->deviceFlags();
			bool numLock = (_delegate->numLock() == false);

			// Set delegate's numLock state
					_delegate->setNumLock(numLock);
                    if ( numLock ) {
				myFlags |= NX_NUMERICPADMASK;
						_specialKeyModifierFlags |= NX_NUMERICPADMASK;
					}
                    else {
				myFlags &= ~NX_NUMERICPADMASK;
						_specialKeyModifierFlags &= ~NX_NUMERICPADMASK;
					}

			_delegate->setDeviceFlags(myFlags);
			_delegate->keyboardEvent(NX_FLAGSCHANGED,
			 /* flags */			myFlags,
			 /* keyCode */			keyCode,
			 /* charCode */			0,
			 /* charSet */			0,
			 /* originalCharCode */ 0,
			 /* originalCharSet */	0);
		}

		break;
		}
	}
	}
}


void IOHIKeyboardMapper::setKeyboardTarget (IOService * keyboardTarget __unused)
{
	//_hidSystem = OSDynamicCast( IOHIDSystem, keyboardTarget );
}

void IOHIKeyboardMapper::makeNumberParamProperty( OSDictionary * dict,
							const char * key,
							unsigned long long number, unsigned int bits )
{
	OSNumber *	numberRef;
	numberRef = OSNumber::withNumber(number, bits);

	if( numberRef) {
		dict->setObject( key, numberRef);
		numberRef->release();
	}
}

bool IOHIKeyboardMapper::updateProperties( void )
{
	bool	ok = true;

	return( ok );
}

IOReturn IOHIKeyboardMapper::setParamProperties( OSDictionary * dict  __unused)
{
	IOReturn		err						= kIOReturnSuccess;
	bool			updated					= false;
	bool			issueFlagsChangedEvent	= false;
	bool			alphaState				= false;
	UInt32			myFlags					= _delegate->deviceFlags();

	if ( issueFlagsChangedEvent )
	{
		if ( alphaState )
		{
			if ( !_delegate->doesKeyLock(NX_KEYTYPE_CAPS_LOCK) )
			{
				_specialKeyModifierFlags |= NX_ALPHASHIFTMASK;
			}

			myFlags |= NX_ALPHASHIFTMASK;

			_delegate->setDeviceFlags(myFlags);
			_delegate->setAlphaLock(true);
		}
		else
		{
			_specialKeyModifierFlags	&= ~NX_ALPHASHIFTMASK;
			myFlags						&= ~NX_ALPHASHIFTMASK;

			_delegate->setDeviceFlags(myFlags);
			_delegate->setAlphaLock(false);
		}

		UInt8			keyCode;
		unsigned char	*map;

		if (((map = _parsedMapping.modDefs[NX_MODIFIERKEY_ALPHALOCK]) != 0 ) &&
			( NEXTNUM(&map, _parsedMapping.shorts) ))
			keyCode = NEXTNUM(&map, _parsedMapping.shorts);
		else
			keyCode = getParsedSpecialKey(NX_KEYTYPE_CAPS_LOCK);

		_delegate->keyboardEvent(NX_FLAGSCHANGED,
		 /* flags */			myFlags,
		 /* keyCode */			keyCode,
		 /* charCode */			0,
		 /* charSet */			0,
		 /* originalCharCode */ 0,
		 /* originalCharSet */	0);


#ifdef NEW_HID
		_delegate->keyboardEvent(alphaState ? NX_KEYDOWN : NX_KEYUP,
		 /* flags */			myFlags,
		 /* keyCode */			keyCode,
		 /* charCode */			0,
		 /* charSet */			0,
		 /* originalCharCode */ 0,
		 /* originalCharSet */	0);
#endif
	}

	// right now updateProperties does nothing interesting
	if (updated)
		updateProperties();

	return( err );
}

// ************* Sticky Keys Functionality ****************


// postKeyboardSpecialEvent
// called to post special keyboard events
// thru the event system to outside of kernel clients
void	IOHIKeyboardMapper::postKeyboardSpecialEvent (unsigned subtype, unsigned eventType)
{
	_delegate->keyboardSpecialEvent (
				/* eventType */ eventType,
				/* flags */		_delegate->eventFlags(),
				/* keyCode */	NX_NOSPECIALKEY,
				/* specialty */ subtype);
}


void IOHIKeyboardMapper::keyEventPostProcess (void)
{

}

OSMetaClassDefineReservedUsed(IOHIKeyboardMapper,  0);
IOReturn IOHIKeyboardMapper::message( UInt32 type __unused, IOService * provider __unused, void * argument __unused )
{

	return kIOReturnSuccess;
}
OSMetaClassDefineReservedUnused(IOHIKeyboardMapper,	 1);
OSMetaClassDefineReservedUnused(IOHIKeyboardMapper,	 2);
OSMetaClassDefineReservedUnused(IOHIKeyboardMapper,	 3);
OSMetaClassDefineReservedUnused(IOHIKeyboardMapper,	 4);
OSMetaClassDefineReservedUnused(IOHIKeyboardMapper,	 5);
OSMetaClassDefineReservedUnused(IOHIKeyboardMapper,	 6);
OSMetaClassDefineReservedUnused(IOHIKeyboardMapper,	 7);
OSMetaClassDefineReservedUnused(IOHIKeyboardMapper,	 8);
OSMetaClassDefineReservedUnused(IOHIKeyboardMapper,	 9);
OSMetaClassDefineReservedUnused(IOHIKeyboardMapper, 10);
OSMetaClassDefineReservedUnused(IOHIKeyboardMapper, 11);
OSMetaClassDefineReservedUnused(IOHIKeyboardMapper, 12);
OSMetaClassDefineReservedUnused(IOHIKeyboardMapper, 13);
OSMetaClassDefineReservedUnused(IOHIKeyboardMapper, 14);
OSMetaClassDefineReservedUnused(IOHIKeyboardMapper, 15);
