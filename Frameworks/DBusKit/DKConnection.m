/*
 * Airyx DBusKit: a simple Cocoa binding to libdbus
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
#import "DKConnection.h"
#import <Foundation/NSString.h>

#include <stdio.h>

static void DBusKit_Unregister_Callback(DBusConnection *connection, void *user_data) {
    DKConnection *self = (DKConnection *)user_data;
    [self unregisterFunction];
}

static DBusHandlerResult DBusKit_Message_Callback(DBusConnection *connection, DBusMessage *msg, void *user_data) {
    DKConnection *self = (DKConnection *)user_data;
    DKMessage *dkmsg = [[DKMessage alloc] initWithMessage:msg];
    return [self messageFunction: dkmsg];
}


@implementation DKConnection

- init {
    _vtable.unregister_function = DBusKit_Unregister_Callback;
    _vtable.message_function = DBusKit_Message_Callback;
    _running = NO;
    messageHandlers = [[NSMutableDictionary dictionaryWithCapacity: 5] retain];

    _DBusConnection = dbus_bus_get(DBUS_BUS_SESSION, NULL);
    if(_DBusConnection == NULL) {
        NSLog(@"Cannot connect to session bus!");
        // FIXME: should it terminate here?
    } else {
        dbus_connection_set_exit_on_disconnect(_DBusConnection, FALSE);
        _name = [NSString stringWithCString:dbus_bus_get_unique_name(_DBusConnection)];
        // NSLog(@"%@ is %@ on the bus",self,_name);
    }
    return self;
}

- (oneway void) release {
    [self stop];
    int refcount = [self retainCount];
    if((_DBusConnection != NULL) && (refcount <= 0)) {
        // NSLog(@"%@ unref DBUS connection\n",self);
        dbus_connection_unref(_DBusConnection);
    }
    [messageHandlers release];
}

- (NSString *)name {
    return _name;
}

- (void) run {
    _running = YES;
    while(_running) {
        [self readWrite: 10]; // wait up to 10ms for any events
        DBusDispatchStatus st = [self dispatch];
        switch(st) {
            case DBUS_DISPATCH_NEED_MEMORY:
                NSLog(@"DBusKit: DispatchStatus=DBUS_DISPATCH_NEED_MEMORY"); break;
        }
    }
}

- (void) stop {
    _running = NO;
}

- (BOOL) registerObjectPath:(NSString *)path {
    return dbus_connection_register_object_path(_DBusConnection, [path UTF8String], &_vtable, (void *)self);
}

- (BOOL) unregisterObjectPath:(NSString *)path {
    return dbus_connection_unregister_object_path(_DBusConnection, [path UTF8String]);
}

- (BOOL) isConnected {
    return dbus_connection_get_is_connected(_DBusConnection) ? YES : NO;
}

- (BOOL) isAuthenticated {
    return dbus_connection_get_is_authenticated(_DBusConnection) ? YES : NO;
}

- (BOOL) isAnonymous {
    return dbus_connection_get_is_anonymous(_DBusConnection) == TRUE ? YES : NO;
}

- (NSInteger) send: (DKMessage *)msg {
    uint32_t serial;
    if(dbus_connection_send(_DBusConnection, [msg _getMessage], &serial) == FALSE)
        return -1;
    return serial;
}

- (DKMessage *) sendWithReplyAndBlock: (DKMessage *)msg {
    DBusMessage *result = dbus_connection_send_with_reply_and_block(_DBusConnection, [msg _getMessage], DBUS_TIMEOUT_USE_DEFAULT, NULL);
    if(result == NULL)
        return nil;
    return [[DKMessage alloc] initWithMessage:result];
}

- (void) flush {
    dbus_connection_flush(_DBusConnection);
}

- (BOOL) readWrite: (int32_t)timeout {
    return dbus_connection_read_write(_DBusConnection, timeout);
}

- (BOOL) readWrite {
    return [self readWrite:10];
}
 
- (DBusDispatchStatus) dispatch {
    return dbus_connection_dispatch(_DBusConnection);
}

- (void) unregisterFunction {
}

- (void) registerHandlerForInterface:(id)handler interface:(NSString *)iface {
    [messageHandlers setObject:[handler retain] forKey:iface];
}

- (void) unregisterHandlerForInterface:(NSString *)iface {
    id handler = [messageHandlers objectForKey:iface];
    [messageHandlers removeObjectForKey:iface];
    if(handler)
        [handler release];
}

- (DBusHandlerResult) messageFunction:(DKMessage *)msg  {
    [msg setUnrefOnRelease:NO];

    NSString *interface = [msg interface];
    id handlerForInterface = [messageHandlers objectForKey:interface];
    if(handlerForInterface) {
        return [handlerForInterface messageFunction:msg];
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

@end
