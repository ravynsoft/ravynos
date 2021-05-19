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
#import <Foundation/NSPlatform.h>
#import <AppKit/NSMenu.h>

static const char *REGISTRAR_INTERFACE = "com.canonical.AppMenu.Registrar";
static const char *REGISTRAR_PATH = "/com/canonical/AppMenu/Registrar";
static NSString *DBUSMENU_INTERFACE = @"com.canonical.dbusmenu";
static NSString *DBUSMENU_PATH = @"/net/pixin/Helium/MenuBar";

@implementation DKMenu
- initWithConnection: (DKConnection *)conn {
    connection = conn;
    layoutVersion = 1;
    menuObjectPath = DBUSMENU_PATH; //[NSString stringWithFormat:@"%@/%08x",DBUSMENU_PATH, NSPlatformThreadID()];
    fprintf(stderr, "menupath = %s\n",[menuObjectPath UTF8String]);

    [connection registerHandlerForInterface:self interface:DBUSMENU_INTERFACE];
    _pathWasRegistered = [connection registerObjectPath:DBUSMENU_PATH];
    if(! _pathWasRegistered) {
        [connection unregisterObjectPath:DBUSMENU_PATH];
        NSLog(@"%@ Attemping to take over stale registration",self);
        _pathWasRegistered = [connection registerObjectPath:DBUSMENU_PATH];
        if(! _pathWasRegistered) {
            NSLog(@"%@ cannot register object path for menus!",self);
        }
    }
    return [self autorelease];
}

- (oneway void) release {
    [connection unregisterHandlerForInterface:DBUSMENU_INTERFACE];
    if(_pathWasRegistered) {
        [connection unregisterObjectPath:DBUSMENU_PATH];
        [connection flush];
        _pathWasRegistered = NO;
    }
}

- (DBusHandlerResult) messageFunction: (DKMessage *)msg {
    fprintf(stderr, "%08x messageFunction for %s called\n",self,[DBUSMENU_INTERFACE UTF8String]);
    NSString *member = [msg member];
    NSString *signature = [msg signature];

    fprintf(stderr, "member = %s, signature = %s\n",[member UTF8String],[signature UTF8String]);

    if([member isEqualToString:@"GetLayout"] && [signature isEqualToString:@"iias"]) {
        fprintf(stderr, "match!\n");
        [self getLayout:msg];
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

- (NSString *)objectPath {
	return menuObjectPath;
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

- (BOOL) registerWindow: (uint32_t)windowID {
    return [self registerWindow:windowID objectPath:menuObjectPath];
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

    [reply appendArg:&layoutVersion type:DBUS_TYPE_UINT32];

    DBusMessageIter iter, subiter;
    dbus_message_iter_init_append([reply _getMessage], &iter);
    dbus_message_iter_open_container(&iter, DBUS_TYPE_STRUCT, NULL, &subiter);
    dbus_message_iter_append_basic(&subiter, DBUS_TYPE_INT32, &rootID);

    // properties map for the root node
    DBusMessageIter dictiter, dictentry, variant;
    dbus_message_iter_open_container(&subiter, DBUS_TYPE_ARRAY, "{sv}", &dictiter);
    dbus_message_iter_open_container(&dictiter, DBUS_TYPE_DICT_ENTRY, NULL, &dictentry);

    const char *s = "children-display";
    dbus_message_iter_append_basic(&dictentry, DBUS_TYPE_STRING, &s);
    s = "submenu";
    dbus_message_iter_open_container(&dictentry, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &s);
    dbus_message_iter_close_container(&dictentry, &variant);
    dbus_message_iter_close_container(&dictiter, &dictentry);
    dbus_message_iter_close_container(&subiter, &dictiter);

    // menu entries as variant array
    DBusMessageIter nested;
    dbus_message_iter_open_container(&subiter, DBUS_TYPE_ARRAY, "v", &nested);

    DBusMessageIter innerStruct, variant2;
    dbus_message_iter_open_container(&nested, DBUS_TYPE_VARIANT, "(ia{sv}av)", &variant);
    dbus_message_iter_open_container(&variant, DBUS_TYPE_STRUCT, NULL, &innerStruct);
    int32_t val = 20;
    dbus_message_iter_append_basic(&innerStruct, DBUS_TYPE_INT32, &val);
    dbus_message_iter_open_container(&innerStruct, DBUS_TYPE_ARRAY, "{sv}", &dictiter);
    dbus_message_iter_open_container(&dictiter, DBUS_TYPE_DICT_ENTRY, NULL, &dictentry);
    s = "label";
    dbus_message_iter_append_basic(&dictentry, DBUS_TYPE_STRING, &s);
    dbus_message_iter_open_container(&dictentry, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &variant2);
    s = "CocoaDemo";
    dbus_message_iter_append_basic(&variant2, DBUS_TYPE_STRING, &s);
    dbus_message_iter_close_container(&dictentry, &variant2);
    dbus_message_iter_close_container(&dictiter, &dictentry);
    dbus_message_iter_close_container(&innerStruct, &dictiter);

    // empty child node array
    dbus_message_iter_open_container(&innerStruct, DBUS_TYPE_ARRAY, DBUS_TYPE_VARIANT_AS_STRING, &dictiter);
    dbus_message_iter_close_container(&innerStruct, &dictiter);

    // close struct and variant wrapper
    dbus_message_iter_close_container(&variant, &innerStruct);
    dbus_message_iter_close_container(&nested, &variant);

    // close variant array
    dbus_message_iter_close_container(&subiter, &nested);

    // close outer struct
    dbus_message_iter_close_container(&iter, &subiter);

    [connection send:reply];
}

- (void) layoutDidUpdate {
    DKMessage *update = [[DKMessage alloc] initSignal:"LayoutUpdated"
        interface:[DBUSMENU_INTERFACE UTF8String] path:[menuObjectPath UTF8String]];

    uint32_t val = layoutVersion;
    [update appendArg:&val type:DBUS_TYPE_UINT32];
    val = 0;
    [update appendArg:&val type:DBUS_TYPE_UINT32];

    [connection send:update];
    ++layoutVersion;
}

- (void) itemPropertiesDidUpdate {
    DKMessage *update = [[DKMessage alloc] initSignal:"ItemsPropertiesUpdated"
        interface:[DBUSMENU_INTERFACE UTF8String] path:[menuObjectPath UTF8String]];
    
    int32_t val = 20;
    // NSMutableDictionary *properties = [NSMutableDictionary dictionaryWithCapacity:1];
    // [properties setObject:@"MenuDemo.app" forKey:@"label"];

    DBusMessageIter iter, container, innerStruct, propArray, property, variant, container2;
    dbus_message_iter_init_append([update _getMessage], &iter);
    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "(ia{sv})", &container);

    // for each menu item, create a struct with its item ID and properties
    dbus_message_iter_open_container(&container, DBUS_TYPE_STRUCT, NULL, &innerStruct);
    dbus_message_iter_append_basic(&innerStruct, DBUS_TYPE_INT32, &val); // item these props belong to

    // for each property of this item
    dbus_message_iter_open_container(&innerStruct, DBUS_TYPE_ARRAY, "{sv}", &propArray);
    dbus_message_iter_open_container(&propArray, DBUS_TYPE_DICT_ENTRY, NULL, &property);
    const char *s = "label";
    dbus_message_iter_append_basic(&property, DBUS_TYPE_STRING, &s);
    s = "Menu 1";
    dbus_message_iter_open_container(&property, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &s);
    dbus_message_iter_close_container(&property, &variant);
    dbus_message_iter_close_container(&propArray, &property);

    dbus_message_iter_open_container(&propArray, DBUS_TYPE_DICT_ENTRY, NULL, &property);
    s = "enabled";
    dbus_message_iter_append_basic(&property, DBUS_TYPE_STRING, &s);
    val = TRUE;
    dbus_message_iter_open_container(&property, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &val);
    dbus_message_iter_close_container(&property, &variant);    
    dbus_message_iter_close_container(&propArray, &property);

    dbus_message_iter_close_container(&innerStruct, &propArray);
    dbus_message_iter_close_container(&container, &innerStruct);
    dbus_message_iter_close_container(&iter, &container);

    // properties removed (empty)
    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, DBUS_TYPE_VARIANT_AS_STRING, &container2);
    dbus_message_iter_close_container(&iter, &container2);

    [connection send:update];

    update = [[DKMessage alloc] initSignal:"ItemsPropertiesUpdated"
        interface:[DBUSMENU_INTERFACE UTF8String] path:[menuObjectPath UTF8String]];
    
    val = 21; 
    // NSMutableDictionary *properties = [NSMutableDictionary dictionaryWithCapacity:1];
    // [properties setObject:@"MenuDemo.app" forKey:@"label"];

    //DBusMessageIter iter, container, innerStruct, propArray, property, variant, container2;
    dbus_message_iter_init_append([update _getMessage], &iter);
    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "(ia{sv})", &container);

    // for each menu item, create a struct with its item ID and properties
    dbus_message_iter_open_container(&container, DBUS_TYPE_STRUCT, NULL, &innerStruct);
    dbus_message_iter_append_basic(&innerStruct, DBUS_TYPE_INT32, &val); // item these props belong to

    // for each property of this item
    dbus_message_iter_open_container(&innerStruct, DBUS_TYPE_ARRAY, "{sv}", &propArray);

    dbus_message_iter_open_container(&propArray, DBUS_TYPE_DICT_ENTRY, NULL, &property);
    s = "label";
    dbus_message_iter_append_basic(&property, DBUS_TYPE_STRING, &s);
    s = "Menu 2";
    dbus_message_iter_open_container(&property, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &s);
    dbus_message_iter_close_container(&property, &variant);
    dbus_message_iter_close_container(&propArray, &property);

    dbus_message_iter_open_container(&propArray, DBUS_TYPE_DICT_ENTRY, NULL, &property);
    s = "enabled";
    dbus_message_iter_append_basic(&property, DBUS_TYPE_STRING, &s);
    val = TRUE;
    dbus_message_iter_open_container(&property, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &val);
    dbus_message_iter_close_container(&property, &variant);    
    dbus_message_iter_close_container(&propArray, &property);
 
    dbus_message_iter_close_container(&innerStruct, &propArray);
    dbus_message_iter_close_container(&container, &innerStruct);
    dbus_message_iter_close_container(&iter, &container);

    // properties removed (empty)
    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, DBUS_TYPE_VARIANT_AS_STRING, &container2);
    dbus_message_iter_close_container(&iter, &container2);

    [connection send:update];
}
@end
