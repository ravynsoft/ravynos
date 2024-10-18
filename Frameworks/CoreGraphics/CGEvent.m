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

#import <CoreGraphics/CGEvent.h>

CFTypeID CGEventGetTypeID(void) {
}

CGEventRef CGEventCreate(CGEventSourceRef source) {
}

CFDataRef CGEventCreateData(CFAllocatorRef allocator, CGEventRef event) {
}

CGEventRef CGEventCreateFromData(CFAllocatorRef allocator, CFDataRef data) {
}

CGEventRef CGEventCreateMouseEvent(CGEventSourceRef source, CGEventType mouseType, CGPoint mouseCursorPosition, CGMouseButton mouseButton) {
}

CGEventRef CGEventCreateKeyboardEvent(CGEventSourceRef source, CGKeyCode virtualKey, bool keyDown) {
}

CGEventRef CGEventCreateScrollWheelEvent(CGEventSourceRef source, CGScrollEventUnit units, uint32_t wheelCount, int32_t wheel1, ...) {
}

CGEventRef CGEventCreateCopy(CGEventRef event) {
}

CGEventSourceRef CGEventCreateSourceFromEvent(CGEventRef event) {
}

void CGEventSetSource(CGEventRef event, CGEventSourceRef source) {
}

CGEventType CGEventGetType(CGEventRef event) {
}

void CGEventSetType(CGEventRef event, CGEventType type) {
}

CGEventTimestamp CGEventGetTimestamp(CGEventRef event) {
}

void CGEventSetTimestamp(CGEventRef event, CGEventTimestamp timestamp) {
}

CGPoint CGEventGetLocation(CGEventRef event) {
}

CGPoint CGEventGetUnflippedLocation(CGEventRef event) {
}

void CGEventSetLocation(CGEventRef event, CGPoint location) {
}

CGEventFlags CGEventGetFlags(CGEventRef event) {
}

void CGEventSetFlags(CGEventRef event, CGEventFlags flags) {
}

void CGEventKeyboardGetUnicodeString(CGEventRef event, UniCharCount maxStringLength, UniCharCount *actualStringLength, UniChar *unicodeString) {
}

void CGEventKeyboardSetUnicodeString(CGEventRef event, UniCharCount stringLength, const UniChar *unicodeString) {
}

int64_t CGEventGetIntegerValueField(CGEventRef event, CGEventField field) {
}

void CGEventSetIntegerValueField(CGEventRef event, CGEventField field, int64_t value) {
}

double CGEventGetDoubleValueField(CGEventRef event, CGEventField field) {
}

void CGEventSetDoubleValueField(CGEventRef event, CGEventField field, double value) {
}



CFMachPortRef CGEventTapCreate(CGEventTapLocation tap, CGEventTapPlacement place, CGEventTapOptions options, CGEventMask eventsOfInterest, CGEventTapCallBack callback, void *userInfo) {
}

CFMachPortRef CGEventTapCreateForPSN(void *processSerialNumber, CGEventTapPlacement place, CGEventTapOptions options, CGEventMask eventsOfInterest, CGEventTapCallBack callback, void *userInfo) {
}

void CGEventTapEnable(CFMachPortRef tap, bool enable) {
}

bool CGEventTapIsEnabled(CFMachPortRef tap) {
}

void CGEventTapPostEvent(CGEventTapProxy proxy, CGEventRef event) {
}

void CGEventPost(CGEventTapLocation tap, CGEventRef event) {
}

void CGEventPostToPSN(void *processSerialNumber, CGEventRef event) {
}

CGError CGGetEventTapList(uint32_t maxNumberOfTaps, CGEventTapInformation *tapList, uint32_t *eventTapCount) {
}


CFTypeID CGEventSourceGetTypeID(void) {
}

CGEventSourceRef CGEventSourceCreate(CGEventSourceStateID stateID) {
}

CGEventSourceKeyboardType CGEventSourceGetKeyboardType(CGEventSourceRef source) {
}

void CGEventSourceSetKeyboardType(CGEventSourceRef source, CGEventSourceKeyboardType keyboardType) {
}

CGEventSourceStateID CGEventSourceGetSourceStateID(CGEventSourceRef source) {
}

bool CGEventSourceButtonState(CGEventSourceStateID stateID, CGMouseButton button) {
}

bool CGEventSourceKeyState(CGEventSourceStateID stateID, CGKeyCode key) {
}

CGEventFlags CGEventSourceFlagsState(CGEventSourceStateID stateID) {
}

CFTimeInterval CGEventSourceSecondsSinceLastEventType(CGEventSourceStateID stateID, CGEventType eventType) {
}

uint32_t CGEventSourceCounterForEventType(CGEventSourceStateID stateID, CGEventType eventType) {
}

int64_t CGEventSourceGetUserData(CGEventSourceRef source) {
}

void CGEventSourceSetUserData(CGEventSourceRef source, int64_t userData) {
}

CGEventFilterMask CGEventSourceGetLocalEventsFilterDuringSuppressionState(CGEventSourceRef source, CGEventSuppressionState state) {
}

void CGEventSourceSetLocalEventsFilterDuringSuppressionState(CGEventSourceRef source, CGEventFilterMask filter, CGEventSuppressionState state) {
}

CFTimeInterval CGEventSourceGetLocalEventsSuppressionInterval(CGEventSourceRef source) {
}

void CGEventSourceSetLocalEventsSuppressionInterval(CGEventSourceRef source, CFTimeInterval seconds) {
}

double CGEventSourceGetPixelsPerLine(CGEventSourceRef source) {
}

void CGEventSourceSetPixelsPerLine(CGEventSourceRef source, double pixelsPerLine) {
}

