/*
 * ravynOS DBusKit: a simple Cocoa binding to libdbus
 *
 * Copyright (C) 2021 Zoe Knox <zoe@pixin.net>
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

#ifdef _BuildingFramework
#include <dbus/dbus.h>
#else
#import <DBusKit/dbus.h>
#endif

#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>

@class DKMessage;
@interface DKMessageIterator: NSObject {
    DBusMessageIter *parent;
    DBusMessageIter child;
}

- init:(DKMessage *)message; // get the append iter for `message`
- (DKMessageIterator *) openStruct;
- (DKMessageIterator *) openArray: (const char *)containedSignature;
- (DKMessageIterator *) openVariant: (const char *)containedSignature;
- (void) appendDictEntry: (NSString *)key variantType: (int)type value: (const void *)value;
- (void) appendVariant: (int)type value: (const void *)value;
- (void) appendString:(NSString *)string;
- (void) appendBasic: (int)type value: (const void *)value;
- (void) close;
@end

@interface DKMessage: NSObject {
    DBusMessage *_message;
    BOOL _unrefOnRelease;
}

- (id)initMethodCall: (const char *)method interface:(const char *)iface path:(const char *)path destination:(const char *)dest;
- (id)initReply: (DKMessage *)methodCall;
- (id)initSignal: (const char *)name interface:(const char *)iface path:(const char *)path;
- (id)initWithMessage: (DBusMessage *)msg;
- (void)setUnrefOnRelease: (BOOL)value;
- (oneway void) release;
- (NSString *) argsAsString;
- (NSString *) destination;
- (BOOL) setDestination: (const char *)dest;
- (NSString *) interface;
- (BOOL) setInterface: (const char *)iface;
- (NSString *) member;
- (BOOL) setMember: (const char *)member;
- (void) setReplyExpected: (BOOL)expected;
- (NSString *) path;
- (BOOL) setPath: (const char *)path;
- (NSString *) signature;
- (int) type;
- (BOOL) appendArg:(const void *)value type:(int)type;
- (DKMessageIterator *)appendIterator;

- (DBusMessage *) _getMessage;
@end
