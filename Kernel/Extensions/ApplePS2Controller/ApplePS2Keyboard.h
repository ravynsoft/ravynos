#ifndef _APPLEPS2KEYBOARD_H
#define _APPLEPS2KEYBOARD_H

#include <IOKit/hidsystem/IOHIKeyboard.h>
#include "ApplePS2KeyboardDevice.h"

#define kPS2ScanCodeCount 0x80

class ApplePS2Keyboard : public IOHIKeyboard
{
    OSDeclareDefaultStructors(ApplePS2Keyboard);

private:
    ApplePS2KeyboardDevice * _device;
    bool                     _extendCount;   /* saw an 0xE0 prefix */
    bool                     _interruptInstalled;

    bool dispatchKeyboardEventWithScancode(UInt8 scanCode);
    void setLEDs(UInt8 ledState);
    void setKeyboardEnable(bool enable);

public:
    virtual bool init(OSDictionary * dict) APPLE_KEXT_OVERRIDE;
    virtual ApplePS2Keyboard * probe(IOService * provider, SInt32 * score) APPLE_KEXT_OVERRIDE;

    virtual bool start(IOService * provider) APPLE_KEXT_OVERRIDE;
    virtual void stop(IOService * provider) APPLE_KEXT_OVERRIDE;

    /* interrupt path (called by controller with each raw byte) */
    static void interruptOccurred(void * target, UInt8 data);

    /* IOHIKeyboard overrides */
    virtual const unsigned char * defaultKeymapOfLength(UInt32 * length) APPLE_KEXT_OVERRIDE;
    virtual void setAlphaLockFeedback(bool locked) APPLE_KEXT_OVERRIDE;
    virtual UInt32 maxKeyCodes() APPLE_KEXT_OVERRIDE;
    virtual UInt32 deviceType() APPLE_KEXT_OVERRIDE;
    virtual UInt32 interfaceID() APPLE_KEXT_OVERRIDE;
};

#endif /* _APPLEPS2KEYBOARD_H */
