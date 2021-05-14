/*
 * Helium DBusKit: a simple Cocoa binding to libdbus
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

#define _BuildingFramework
#import "DKMenu.h"
#import <Foundation/NSString.h>
#import <AppKit/NSMenu.h>

static const char *REGISTRAR_INTERFACE = "com.canonical.AppMenu.Registrar";
static const char *REGISTRAR_PATH = "/com/canonical/AppMenu/Registrar";
static const char *DBUSMENU_INTERFACE = "com.canonical.dbusmenu";
static const char *DBUSMENU_PATH = "/net/pixin/helium/DKMenu";

@implementation DKMenu
- initWithConnection: (DKConnection *)conn {
    connection = conn;
    return self;
}

- (BOOL) registerWindow: (uint32_t)windowID objectPath: (NSString *)path {
    DKMessage *message = [[DKMessage alloc] initMethodCall:"RegisterWindow" interface:REGISTRAR_INTERFACE path:REGISTRAR_PATH destination:REGISTRAR_INTERFACE];
    [message appendArg:&windowID type:DBUS_TYPE_UINT32];
    const char *szpath = [path UTF8String];
    [message appendArg:&szpath type:DBUS_TYPE_OBJECT_PATH];
    DKMessage *reply = [connection sendWithReplyAndBlock:message];

    if(reply == nil) {
        return NO;
    }
    return YES;
}

- (BOOL) unregisterWindow: (uint32_t)windowID {
    DKMessage *message = [[DKMessage alloc] initMethodCall:"UnregisterWindow" interface:REGISTRAR_INTERFACE path:REGISTRAR_PATH destination:REGISTRAR_INTERFACE];
    [message appendArg:&windowID type:DBUS_TYPE_UINT32];
    DKMessage *reply = [connection sendWithReplyAndBlock:message];

    if(reply == nil) {
        return NO;
    }
    return YES;
}

- (NSString *) getMenuForWindow: (uint32_t)windowID {
    DKMessage *message = [[DKMessage alloc] initMethodCall:"GetMenuForWindow" interface:REGISTRAR_INTERFACE path:REGISTRAR_PATH destination:REGISTRAR_INTERFACE];
    [message appendArg:&windowID type:DBUS_TYPE_UINT32];
    DKMessage *reply = [connection sendWithReplyAndBlock:message];

    if(reply == nil) {
        return nil;
    }

    const char *service = NULL;
    const char *objectPath = NULL;
    dbus_message_get_args([reply _getMessage], NULL, DBUS_TYPE_STRING, &service, DBUS_TYPE_OBJECT_PATH, &objectPath, DBUS_TYPE_INVALID);
    NSString *result = [[NSString stringWithFormat:@"%s:%s", service, objectPath] autorelease];
    return result;
}

- (void) getLayout: (DKMessage *)message {
    int rootID = 0;
    int recursionDepth = -1;

    dbus_message_get_args([message _getMessage], NULL, DBUS_TYPE_INT32, &rootID, DBUS_TYPE_INT32, &recursionDepth, DBUS_TYPE_INVALID);
    NSLog(@"getLayout called for %d with depth %d!", rootID, recursionDepth);
    DKMessage *reply = [[DKMessage alloc] initReply:message];

    DBusMessageIter *iter, *subiter;
    dbus_message_iter_init_append([reply _getMessage], iter);
    dbus_message_iter_open_container(iter, DBUS_TYPE_STRUCT, NULL, subiter);
    dbus_message_iter_append_basic(subiter, DBUS_TYPE_INT32, &rootID);

    // empty properties map
    DBusMessageIter *dictiter;
    dbus_message_iter_open_container(subiter, DBUS_TYPE_ARRAY, DBUS_TYPE_DICT_ENTRY_AS_STRING, dictiter);
    dbus_message_iter_close_container(subiter, dictiter);

    // empty menu tree
    DBusMessageIter *arrayiter;
    dbus_message_iter_open_container(subiter, DBUS_TYPE_ARRAY, DBUS_TYPE_VARIANT_AS_STRING, arrayiter);
    dbus_message_iter_close_container(subiter, arrayiter);

    dbus_message_iter_close_container(iter, subiter);

    [connection send:reply];
}
@end