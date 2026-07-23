/*
 * ApplePS2Keyboard: HID driver for the PS/2 keyboard nub published by
 * ApplePS2Controller. Translates i8042 set-1 (XT, controller-translated)
 * scan codes into ADB keycodes and dispatches them through IOHIKeyboard,
 * where IOBSDConsole picks them up for console input.
 *
 * Modeled on Apple's historic ApplePS2Keyboard (APSL); rewritten for the
 * PureDarwin bring-up against IOHIDFamily-1633's IOHIKeyboard.
 */
#include <IOKit/IOLib.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include <IOKit/hidsystem/IOHIDShared.h>
#include "ApplePS2Keyboard.h"
#include "ApplePS2KeyboardMap.h"

#define super IOHIKeyboard
OSDefineMetaClassAndStructors(ApplePS2Keyboard, IOHIKeyboard);

/*
 * PC set-1 (XT) scan code -> ADB keycode, US layout. 0xFF = no mapping.
 * Index is the make code (0x00-0x7F); break codes have bit 7 set.
 * From Apple's historic ApplePS2Keyboard driver.
 */
static const UInt8 PS2ToADBMap[kPS2ScanCodeCount * 2] =
{
/*  PS2 set-1 -> ADB, non-extended (no E0 prefix) */
    0xFF, 0x35, 0x12, 0x13, 0x14, 0x15, 0x17, 0x16,  /* 00-07: -,esc,1..6 */
    0x1A, 0x1C, 0x19, 0x1D, 0x1B, 0x18, 0x33, 0x30,  /* 08-0f: 7..0,-,=,bs,tab */
    0x0C, 0x0D, 0x0E, 0x0F, 0x11, 0x10, 0x20, 0x22,  /* 10-17: q,w,e,r,t,y,u,i */
    0x1F, 0x23, 0x21, 0x1E, 0x24, 0x36, 0x00, 0x01,  /* 18-1f: o,p,[,],ret,Lctl,a,s */
    0x02, 0x03, 0x05, 0x04, 0x26, 0x28, 0x25, 0x29,  /* 20-27: d,f,g,h,j,k,l,; */
    0x27, 0x32, 0x38, 0x2A, 0x06, 0x07, 0x08, 0x09,  /* 28-2f: ',`,Lsh,\,z,x,c,v */
    0x0B, 0x2D, 0x2E, 0x2B, 0x2F, 0x2C, 0x3C, 0x43,  /* 30-37: b,n,m,,,.,/,Rsh,kp* */
    0x3A, 0x31, 0x39, 0x7A, 0x78, 0x63, 0x76, 0x60,  /* 38-3f: Lalt,spc,caps,F1..F5 */
    0x61, 0x62, 0x64, 0x65, 0x6D, 0x47, 0x6B, 0x59,  /* 40-47: F6..F10,num,scroll,kp7 */
    0x5B, 0x5C, 0x4E, 0x56, 0x57, 0x58, 0x45, 0x53,  /* 48-4f: kp8,kp9,kp-,kp4,kp5,kp6,kp+,kp1 */
    0x54, 0x55, 0x52, 0x41, 0xFF, 0xFF, 0x0A, 0x67,  /* 50-57: kp2,kp3,kp0,kp.,-,-,<>,F11 */
    0x6F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* 58-5f: F12 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* 60-67 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* 68-6f */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* 70-77 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* 78-7f */
/*  PS2 set-1 -> ADB, extended (E0 prefix) */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* e0 00-07 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* e0 08-0f */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* e0 10-17 */
    0xFF, 0xFF, 0xFF, 0xFF, 0x4C, 0x3E, 0xFF, 0xFF,  /* e0 18-1f: kp-enter,Rctl */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* e0 20-27 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* e0 28-2f */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0xFF, 0x69,  /* e0 30-37: kp/,prtsc */
    0x3D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* e0 38-3f: Ralt */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x71, 0x73,  /* e0 40-47: pause,home */
    0x3E, 0x74, 0xFF, 0x3B, 0xFF, 0x3C, 0xFF, 0x77,  /* e0 48-4f: up,pgup,left,right,end */
    0x3D, 0x79, 0x72, 0x75, 0xFF, 0xFF, 0xFF, 0xFF,  /* e0 50-57: down,pgdn,ins,del */
    0xFF, 0xFF, 0xFF, 0x37, 0x36, 0x6E, 0xFF, 0xFF,  /* e0 58-5f: Lcmd(win),Rcmd,menu */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* e0 60-67 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* e0 68-6f */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* e0 70-77 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF   /* e0 78-7f */
};

bool ApplePS2Keyboard::init(OSDictionary * dict)
{
    if (!super::init(dict)) {
        return false;
    }
    _device             = NULL;
    _extendCount        = false;
    _interruptInstalled = false;
    return true;
}

ApplePS2Keyboard * ApplePS2Keyboard::probe(IOService * provider, SInt32 * score)
{
    /* The controller guarantees the keyboard is present and quiesced when it
     * publishes the nub, so no reset-and-probe dance here (QEMU's i8042 and
     * real translated controllers both speak set 1 by default). */
    if (!super::probe(provider, score)) {
        return NULL;
    }
    return this;
}

bool ApplePS2Keyboard::start(IOService * provider)
{
    if (!super::start(provider)) {
        return false;
    }

    _device = OSDynamicCast(ApplePS2KeyboardDevice, provider);
    if (_device == NULL) {
        return false;
    }

    _device->installInterruptAction(this,
        (PS2InterruptAction)&ApplePS2Keyboard::interruptOccurred);
    _interruptInstalled = true;

    setKeyboardEnable(true);

    IOLog("ApplePS2Keyboard: started (set-1 scancodes -> ADB)\n");
    return true;
}

void ApplePS2Keyboard::stop(IOService * provider)
{
    setKeyboardEnable(false);
    if (_interruptInstalled) {
        _device->uninstallInterruptAction();
        _interruptInstalled = false;
    }
    _device = NULL;
    super::stop(provider);
}

void ApplePS2Keyboard::interruptOccurred(void * target, UInt8 data)
{
    ApplePS2Keyboard * self = (ApplePS2Keyboard *)target;

    if (data == kSC_Extend) {        /* 0xE0 prefix */
        self->_extendCount = true;
        return;
    }
    /* ACK/resend chatter from commands: ignore */
    if (data == kSC_Acknowledge || data == kSC_Resend) {
        return;
    }
    self->dispatchKeyboardEventWithScancode(data);
}

bool ApplePS2Keyboard::dispatchKeyboardEventWithScancode(UInt8 scanCode)
{
    bool   goingDown = !(scanCode & kSC_UpBit);
    UInt8  keyCode   = scanCode & ~kSC_UpBit;
    UInt8  adbCode;
    AbsoluteTime now;

    if (_extendCount) {
        _extendCount = false;
        adbCode = PS2ToADBMap[kPS2ScanCodeCount + keyCode];
    } else {
        adbCode = PS2ToADBMap[keyCode];
    }

    if (adbCode == 0xFF) {
        return false;
    }

    clock_get_uptime((uint64_t *)&now);
    dispatchKeyboardEvent(adbCode, goingDown, *(AbsoluteTime *)&now);
    return true;
}

void ApplePS2Keyboard::setLEDs(UInt8 ledState)
{
    PS2Request * request = _device->allocateRequest();
    if (request == NULL) {
        return;
    }
    request->commands[0].command = kPS2C_WriteDataPort;
    request->commands[0].inOrOut = kDP_SetKeyboardLEDs;
    request->commands[1].command = kPS2C_ReadDataPortAndCompare;
    request->commands[1].inOrOut = kSC_Acknowledge;
    request->commands[2].command = kPS2C_WriteDataPort;
    request->commands[2].inOrOut = ledState;
    request->commands[3].command = kPS2C_ReadDataPortAndCompare;
    request->commands[3].inOrOut = kSC_Acknowledge;
    request->commandsCount   = 4;
    request->completionTarget = NULL;
    request->completionAction = NULL;
    _device->submitRequest(request);   /* auto-freed (no completion routine) */
}

void ApplePS2Keyboard::setKeyboardEnable(bool enable)
{
    PS2Request * request = _device->allocateRequest();
    if (request == NULL) {
        return;
    }
    request->commands[0].command = kPS2C_WriteDataPort;
    request->commands[0].inOrOut = enable ? kDP_Enable : kDP_SetDefaultsAndDisable;
    request->commands[1].command = kPS2C_ReadDataPortAndCompare;
    request->commands[1].inOrOut = kSC_Acknowledge;
    request->commandsCount   = 2;
    request->completionTarget = NULL;
    request->completionAction = NULL;
    _device->submitRequest(request);
}

void ApplePS2Keyboard::setAlphaLockFeedback(bool locked)
{
    /* caps lock LED = bit 2 */
    setLEDs(locked ? 0x04 : 0x00);
}

const unsigned char * ApplePS2Keyboard::defaultKeymapOfLength(UInt32 * length)
{
    *length = sizeof(applePS2USAKeyMap);
    return applePS2USAKeyMap;
}

UInt32 ApplePS2Keyboard::maxKeyCodes()
{
    return NX_NUMKEYCODES;
}

UInt32 ApplePS2Keyboard::deviceType()
{
    return NX_EVS_DEVICE_TYPE_KEYBOARD;
}

UInt32 ApplePS2Keyboard::interfaceID()
{
    return NX_EVS_DEVICE_INTERFACE_ADB;
}
