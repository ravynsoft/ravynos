/*
 * Quartz Event Services
 * Copyright (C) 2024 Zoe Knox <zoe@ravynsoft.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#import <CoreFoundation/CoreFoundation.h>
#import <CoreGraphics/CoreGraphicsExport.h>
#import <CoreGraphics/CGError.h>

#define CGEventMaskBit(eventType) eventType
#define kCGAnyInputEventType (~(CGEventType)0)
#define kCGEventMaskForAllEvents (~(CGEventMask)0)

typedef uint32_t UniCharCount;
typedef struct __CGEvent *CGEventRef;
typedef uint32_t CGButtonCount;
typedef uint16_t CGCharCode;
typedef uint64_t CGEventMask;
typedef uint32_t CGEventSourceKeyboardType;
typedef struct __CGEventSource *CGEventSourceRef;
typedef struct __CGEventTapProxy *CGEventTapProxy;
typedef uint64_t CGEventTimestamp;
typedef uint16_t CGKeyCode;
typedef uint32_t CGWheelCount;

typedef enum CGEventField : uint32_t {
    kCGMouseEventNumber = 0,
    kCGMouseEventClickState = 1,
    kCGMouseEventPressure = 2,
    kCGMouseEventButtonNumber = 3,
    kCGMouseEventDeltaX = 4,
    kCGMouseEventDeltaY = 5,
    kCGMouseEventInstantMouser = 6,
    kCGMouseEventSubtype = 7,
    kCGKeyboardEventAutorepeat = 8,
    kCGKeyboardEventKeycode = 9,
    kCGKeyboardEventKeyboardType = 10,
    kCGScrollWheelEventDeltaAxis1 = 11,
    kCGScrollWheelEventDeltaAxis2 = 12,
    kCGScrollWheelEventDeltaAxis3 = 13,
    kCGScrollWheelEventFixedPtDeltaAxis1 = 93,
    kCGScrollWheelEventFixedPtDeltaAxis2 = 94,
    kCGScrollWheelEventFixedPtDeltaAxis3 = 95,
    kCGScrollWheelEventPointDeltaAxis1 = 96,
    kCGScrollWheelEventPointDeltaAxis2 = 97,
    kCGScrollWheelEventPointDeltaAxis3 = 98,
    kCGScrollWheelEventInstantMouser = 14,
    kCGTabletEventPointX = 15,
    kCGTabletEventPointY = 16,
    kCGTabletEventPointZ = 17,
    kCGTabletEventPointButtons = 18,
    kCGTabletEventPointPressure = 19,
    kCGTabletEventTiltX = 20,
    kCGTabletEventTiltY = 21,
    kCGTabletEventRotation = 22,
    kCGTabletEventTangentialPressure = 23,
    kCGTabletEventDeviceID = 24,
    kCGTabletEventVendor1 = 25,
    kCGTabletEventVendor2 = 26,
    kCGTabletEventVendor3 = 27,
    kCGTabletProximityEventVendorID = 28,
    kCGTabletProximityEventTabletID = 29,
    kCGTabletProximityEventPointerID = 30,
    kCGTabletProximityEventDeviceID = 31,
    kCGTabletProximityEventSystemTabletID = 32,
    kCGTabletProximityEventVendorPointerType = 33,
    kCGTabletProximityEventVendorPointerSerialNumber = 34,
    kCGTabletProximityEventVendorUniqueID = 35,
    kCGTabletProximityEventCapabilityMask = 36,
    kCGTabletProximityEventPointerType = 37,
    kCGTabletProximityEventEnterProximity = 38,
    kCGEventTargetProcessSerialNumber = 39,
    kCGEventTargetUnixProcessID = 40,
    kCGEventSourceUnixProcessID = 41,
    kCGEventSourceUserData = 42,
    kCGEventSourceUserID = 43,
    kCGEventSourceGroupID = 44,
    kCGEventSourceStateID = 45,
    kCGScrollWheelEventIsContinuous = 88,
    kCGMouseEventWindowUnderMousePointer = 91,
    kCGMouseEventWindowUnderMousePointerThatCanHandleThisEvent = 92,
    kCGScrollWheelEventMomentumPhase = 123,
    kCGScrollWheelEventScrollCount = 100,
    kCGScrollWheelEventScrollPhase = 99,
    kCGEventUnacceleratedPointerMovementX = 170,
    kCGEventUnacceleratedPointerMovementY = 171,
    kCGScrollWheelEventAcceleratedDeltaAxis1 = 176,
    kCGScrollWheelEventAcceleratedDeltaAxis2 = 175,
    kCGScrollWheelEventMomentumOptionPhase = 173,
    kCGScrollWheelEventRawDeltaAxis1 = 178,
    kCGScrollWheelEventRawDeltaAxis2 = 177
} CGEventField;

typedef enum CGEventFilterMask : uint32_t {
    kCGEventFilterMaskPermitLocalMouseEvents = 0x00000001,
    kCGEventFilterMaskPermitLocalKeyboardEvents = 0x00000002,
    kCGEventFilterMaskPermitSystemDefinedEvents = 0x00000004
} CGEventFilterMask;

#define NX_ALPHASHIFTMASK       0x010000
#define NX_SHIFTMASK            0x020000
#define NX_CONTROLMASK          0x040000
#define NX_ALTERNATEMASK        0x080000
#define NX_COMMANDMASK          0x100000
#define NX_NUMERICPADMASK       0x200000
#define NX_HELPMASK             0x400000
#define NX_SECONDARYFNMASK      0x800000
#define NX_NONCOALSESCEDMASK    0x100  // Don't @ me. Apple spelled it this way.
#define NX_NONCOALESCEDMASK NX_NONCOALSESCEDMASK

typedef enum CGEventFlags : uint64_t {
    kCGEventFlagMaskAlphaShift = NX_ALPHASHIFTMASK,
    kCGEventFlagMaskShift = NX_SHIFTMASK,
    kCGEventFlagMaskControl = NX_CONTROLMASK,
    kCGEventFlagMaskAlternate = NX_ALTERNATEMASK,
    kCGEventFlagMaskCommand = NX_COMMANDMASK,
    kCGEventFlagMaskHelp = NX_HELPMASK,
    kCGEventFlagMaskSecondaryFn = NX_SECONDARYFNMASK,
    kCGEventFlagMaskNumericPad = NX_NUMERICPADMASK,
    kCGEventFlagMaskNonCoalesced = NX_NONCOALSESCEDMASK
} CGEventFlags;

typedef enum CGEventSourceStateID : int32_t {
    kCGEventSourceStatePrivate = -1,
    kCGEventSourceStateCombinedSessionState = 0,
    kCGEventSourceStateHIDSystemState = 1
} CGEventSourceStateID;

typedef enum CGEventSuppressionState : uint32_t {
    kCGEventSuppressionStateSuppressionInterval = 0,
    kCGEventSuppressionStateRemoteMouseDrag,
    kCGNumberOfEventSuppressionStates
} CGEventSuppressionState;

typedef enum CGEventTapLocation : uint32_t {
    kCGHIDEventTap = 0,
    kCGSessionEventTap,
    kCGAnnotatedSessionEventTap
} CGEventTapLocation;

typedef enum CGEventTapOptions : uint32_t {
    kCGEventTapOptionDefault = 0,
    kCGEventTapOptionListenOnly = 1
} CGEventTapOptions;

typedef enum CGEventTapPlacement : uint32_t {
    kCGHeadInsertEventTap = 0,
    kCGTailAppendEventTap
} CGEventTapPlacement;

#define NX_NULLEVENT            0x00
#define NX_LMOUSEDOWN           0x01
#define NX_LMOUSEUP             0x02
#define NX_RMOUSEDOWN           0x03
#define NX_RMOUSEUP             0x04
#define NX_MOUSEMOVED           0x05
#define NX_LMOUSEDRAGGED        0x06
#define NX_RMOUSEDRAGGED        0x07
#define NX_KEYDOWN              0x0A
#define NX_KEYUP                0x0B
#define NX_FLAGSCHANGED         0x0C
#define NX_SCROLLWHEELMOVED     0x16
#define NX_TABLETPOINTER        0x17
#define NX_TABLETPROXIMITY      0x18
#define NX_OMOUSEDOWN           0x19
#define NX_OMOUSEUP             0x1A
#define NX_OMOUSEDRAGGED        0x1B

typedef enum CGEventType : uint32_t {
    kCGEventNull = NX_NULLEVENT,
    kCGEventLeftMouseDown = NX_LMOUSEDOWN,
    kCGEventLeftMouseUp = NX_LMOUSEUP,
    kCGEventRightMouseDown = NX_RMOUSEDOWN,
    kCGEventRightMouseUp = NX_RMOUSEUP,
    kCGEventMouseMoved = NX_MOUSEMOVED,
    kCGEventLeftMouseDragged = NX_LMOUSEDRAGGED,
    kCGEventRightMouseDragged = NX_RMOUSEDRAGGED,
    kCGEventKeyDown = NX_KEYDOWN,
    kCGEventKeyUp = NX_KEYUP,
    kCGEventFlagsChanged = NX_FLAGSCHANGED,
    kCGEventScrollWheel = NX_SCROLLWHEELMOVED,
    kCGEventTabletPointer = NX_TABLETPOINTER,
    kCGEventTabletProximity = NX_TABLETPROXIMITY,
    kCGEventOtherMouseDown = NX_OMOUSEDOWN,
    kCGEventOtherMouseUp = NX_OMOUSEUP,
    kCGEventOtherMouseDragged = NX_OMOUSEDRAGGED,
    kCGEventTapDisabledByTimeout = 0xFFFFFFFE,
    kCGEventTapDisabledByUserInput = 0xFFFFFFFF
} CGEventType;

typedef enum CGMouseButton : uint32_t {
    kCGMouseButtonLeft = 0,
    kCGMouseButtonRight = 1,
    kCGMouseButtonCenter = 2
} CGMouseButton;

typedef enum CGEventMouseSubtype : uint32_t {
    kCGEventMouseSubtypeDefault = 0,
    kCGEventMouseSubtypeTabletPoint = 1,
    kCGEventMouseSubtypeTabletProximity = 2
} CGEventMouseSubtype;

typedef enum CGScrollEventUnit : uint32_t {
    kCGScrollEventUnitPixel = 0,
    kCGScrollEventUnitLine = 1
} CGScrollEventUnit;

typedef struct __CGEventTapInformation {
    float avgUsecLatency;
    bool enabled;
    uint32_t eventTapID;
    CGEventMask eventsOfInterest;
    float maxUsecLatency;
    float minUsecLatency;
    CGEventTapOptions options;
    pid_t processBeingTapped;
    CGEventTapLocation tapPoint;
    pid_t tappingProcess;
} CGEventTapInformation;

// Working with Events
COREGRAPHICS_EXPORT CFTypeID CGEventGetTypeID(void);
COREGRAPHICS_EXPORT CGEventRef CGEventCreate(CGEventSourceRef source);
COREGRAPHICS_EXPORT CFDataRef CGEventCreateData(CFAllocatorRef allocator, CGEventRef event);
COREGRAPHICS_EXPORT CGEventRef CGEventCreateFromData(CFAllocatorRef allocator, CFDataRef data);
COREGRAPHICS_EXPORT CGEventRef CGEventCreateMouseEvent(CGEventSourceRef source, CGEventType mouseType, CGPoint mouseCursorPosition, CGMouseButton mouseButton);
COREGRAPHICS_EXPORT CGEventRef CGEventCreateKeyboardEvent(CGEventSourceRef source, CGKeyCode virtualKey, bool keyDown);
COREGRAPHICS_EXPORT CGEventRef CGEventCreateScrollWheelEvent(CGEventSourceRef source, CGScrollEventUnit units, uint32_t wheelCount, int32_t wheel1, ...);
COREGRAPHICS_EXPORT CGEventRef CGEventCreateCopy(CGEventRef event);
COREGRAPHICS_EXPORT CGEventSourceRef CGEventCreateSourceFromEvent(CGEventRef event);
COREGRAPHICS_EXPORT void CGEventSetSource(CGEventRef event, CGEventSourceRef source);
COREGRAPHICS_EXPORT CGEventType CGEventGetType(CGEventRef event);
COREGRAPHICS_EXPORT void CGEventSetType(CGEventRef event, CGEventType type);
COREGRAPHICS_EXPORT CGEventTimestamp CGEventGetTimestamp(CGEventRef event);
COREGRAPHICS_EXPORT void CGEventSetTimestamp(CGEventRef event, CGEventTimestamp timestamp);
COREGRAPHICS_EXPORT CGPoint CGEventGetLocation(CGEventRef event);
COREGRAPHICS_EXPORT CGPoint CGEventGetUnflippedLocation(CGEventRef event);
COREGRAPHICS_EXPORT void CGEventSetLocation(CGEventRef event, CGPoint location);
COREGRAPHICS_EXPORT CGEventFlags CGEventGetFlags(CGEventRef event);
COREGRAPHICS_EXPORT void CGEventSetFlags(CGEventRef event, CGEventFlags flags);
COREGRAPHICS_EXPORT void CGEventKeyboardGetUnicodeString(CGEventRef event, UniCharCount maxStringLength, UniCharCount *actualStringLength, UniChar *unicodeString);
COREGRAPHICS_EXPORT void CGEventKeyboardSetUnicodeString(CGEventRef event, UniCharCount stringLength, const UniChar *unicodeString);
COREGRAPHICS_EXPORT int64_t CGEventGetIntegerValueField(CGEventRef event, CGEventField field);
COREGRAPHICS_EXPORT void CGEventSetIntegerValueField(CGEventRef event, CGEventField field, int64_t value);
COREGRAPHICS_EXPORT double CGEventGetDoubleValueField(CGEventRef event, CGEventField field);
COREGRAPHICS_EXPORT void CGEventSetDoubleValueField(CGEventRef event, CGEventField field, double value);

// Callbacks
typedef CGEventRef (*CGEventTapCallBack)(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *userInfo);

// Working with Event Taps
COREGRAPHICS_EXPORT CFMachPortRef CGEventTapCreate(CGEventTapLocation tap, CGEventTapPlacement place, CGEventTapOptions options, CGEventMask eventsOfInterest, CGEventTapCallBack callback, void *userInfo);
COREGRAPHICS_EXPORT CFMachPortRef CGEventTapCreateForPSN(void *processSerialNumber, CGEventTapPlacement place, CGEventTapOptions options, CGEventMask eventsOfInterest, CGEventTapCallBack callback, void *userInfo);
COREGRAPHICS_EXPORT void CGEventTapEnable(CFMachPortRef tap, bool enable);
COREGRAPHICS_EXPORT bool CGEventTapIsEnabled(CFMachPortRef tap);
COREGRAPHICS_EXPORT void CGEventTapPostEvent(CGEventTapProxy proxy, CGEventRef event);
COREGRAPHICS_EXPORT void CGEventPost(CGEventTapLocation tap, CGEventRef event);
COREGRAPHICS_EXPORT void CGEventPostToPSN(void *processSerialNumber, CGEventRef event);
COREGRAPHICS_EXPORT CGError CGGetEventTapList(uint32_t maxNumberOfTaps, CGEventTapInformation *tapList, uint32_t *eventTapCount);


// Working with Event Sources
COREGRAPHICS_EXPORT CFTypeID CGEventSourceGetTypeID(void);
COREGRAPHICS_EXPORT CGEventSourceRef CGEventSourceCreate(CGEventSourceStateID stateID);
COREGRAPHICS_EXPORT CGEventSourceKeyboardType CGEventSourceGetKeyboardType(CGEventSourceRef source);
COREGRAPHICS_EXPORT void CGEventSourceSetKeyboardType(CGEventSourceRef source, CGEventSourceKeyboardType keyboardType);
COREGRAPHICS_EXPORT CGEventSourceStateID CGEventSourceGetSourceStateID(CGEventSourceRef source);
COREGRAPHICS_EXPORT bool CGEventSourceButtonState(CGEventSourceStateID stateID, CGMouseButton button);
COREGRAPHICS_EXPORT bool CGEventSourceKeyState(CGEventSourceStateID stateID, CGKeyCode key);
COREGRAPHICS_EXPORT CGEventFlags CGEventSourceFlagsState(CGEventSourceStateID stateID);
COREGRAPHICS_EXPORT CFTimeInterval CGEventSourceSecondsSinceLastEventType(CGEventSourceStateID stateID, CGEventType eventType);
COREGRAPHICS_EXPORT uint32_t CGEventSourceCounterForEventType(CGEventSourceStateID stateID, CGEventType eventType);
COREGRAPHICS_EXPORT int64_t CGEventSourceGetUserData(CGEventSourceRef source);
COREGRAPHICS_EXPORT void CGEventSourceSetUserData(CGEventSourceRef source, int64_t userData);
COREGRAPHICS_EXPORT CGEventFilterMask CGEventSourceGetLocalEventsFilterDuringSuppressionState(CGEventSourceRef source, CGEventSuppressionState state);
COREGRAPHICS_EXPORT void CGEventSourceSetLocalEventsFilterDuringSuppressionState(CGEventSourceRef source, CGEventFilterMask filter, CGEventSuppressionState state);
COREGRAPHICS_EXPORT CFTimeInterval CGEventSourceGetLocalEventsSuppressionInterval(CGEventSourceRef source);
COREGRAPHICS_EXPORT void CGEventSourceSetLocalEventsSuppressionInterval(CGEventSourceRef source, CFTimeInterval seconds);
COREGRAPHICS_EXPORT double CGEventSourceGetPixelsPerLine(CGEventSourceRef source);
COREGRAPHICS_EXPORT void CGEventSourceSetPixelsPerLine(CGEventSourceRef source, double pixelsPerLine);

