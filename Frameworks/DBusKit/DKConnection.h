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
#import <Foundation/NSMutableDictionary.h>
#import "DKMessage.h"

@interface DKConnection: NSObject {
    DBusConnection *_DBusConnection;
    DBusObjectPathVTable _vtable;
    NSString *_name; // our name on the bus
    BOOL _running; // running in thread?
    NSMutableDictionary *messageHandlers;
}

- init;
- (NSString *)name;
- (BOOL) registerObjectPath: (NSString *)path;
- (BOOL) unregisterObjectPath: (NSString *)path;
- (BOOL) isConnected;
- (BOOL) isAuthenticated;
- (BOOL) isAnonymous;
- (NSInteger) send: (DKMessage *)msg;
- (DKMessage *) sendWithReplyAndBlock: (DKMessage *)msg;
- (void) flush;
- (BOOL) readWrite: (int32_t)timeout;
- (DBusDispatchStatus) dispatch;
- (void) registerHandlerForInterface:(id)handler interface:(NSString *)iface;
- (void) unregisterHandlerForInterface:(NSString *)iface;

- (void) unregisterFunction;
- (DBusHandlerResult) messageFunction: (DKMessage *)msg;
@end
