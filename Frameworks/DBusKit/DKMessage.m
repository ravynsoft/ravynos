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

#define _BuildingFramework
#import "DKMessage.h"

@implementation DKMessage

- init {
    [super init];
    _unrefOnRelease = YES;
    return self;
}

- initMethodCall: (const char *)method interface:(const char *)iface path:(const char *)path destination:(const char *)dest {
    _message = dbus_message_new_method_call(dest, path, iface, method);
    // NSLog(@"%@ initMethodCall, refcount=%d",self,[self retainCount]);
    return [self autorelease];
}

- initReply: (DKMessage *)methodCall {
    _message = dbus_message_new_method_return([methodCall _getMessage]);
    // NSLog(@"%@ initReply, refcount=%d",self,[self retainCount]);
    return [self autorelease];
}

- initSignal: (const char *)name interface:(const char *)iface path:(const char *)path {
    _message = dbus_message_new_signal(path, iface, name);
    // NSLog(@"%@ initSignal, refcount=%d",self,[self retainCount]);
    return [self autorelease];
}

- initWithMessage: (DBusMessage *)msg {
    _message = msg;
    // NSLog(@"%@ initWithMessage, refcount=%d",self,[self retainCount]);
    return [self autorelease];
}

- (void)setUnrefOnRelease: (BOOL)value {
    _unrefOnRelease=value;
}

- (oneway void) release {
    // NSLog(@"%@ releasing, refcount=%d",self,[self retainCount]);
    if(_unrefOnRelease)
        dbus_message_unref(_message);
}

- (NSString *) argsAsString {
    const char *result = NULL;
    dbus_message_get_args(_message, NULL, DBUS_TYPE_STRING, &result, DBUS_TYPE_INVALID);
    return [[NSString stringWithCString:result] autorelease];
}

- (NSString *) destination {
    const char *result = dbus_message_get_destination(_message);
    if(result == NULL) return nil;
    return [[NSString stringWithCString:result] autorelease];
}

- (BOOL) setDestination: (const char *)dest {
    return dbus_message_set_destination(_message, dest);
}

- (NSString *) interface {
    const char *result = dbus_message_get_interface(_message);
    if(result == NULL) return nil;
    return [[NSString stringWithCString:result] autorelease];
}

- (BOOL) setInterface: (const char *)iface {
    return dbus_message_set_interface(_message, iface);
}

- (NSString *) member {
    const char *result = dbus_message_get_member(_message);
    if(result == NULL) return nil;
    return [[NSString stringWithCString:result] autorelease];
}

- (BOOL) setMember: (const char *)member {
    return dbus_message_set_member(_message, member);
}

- (void) setReplyExpected: (BOOL)expected {
    dbus_message_set_no_reply(_message, !expected);
}

- (NSString *) path {
    const char *result = dbus_message_get_path(_message);
    if(result == NULL) return nil;
    return [[NSString stringWithCString:result] autorelease];
}

- (BOOL) setPath: (const char *)path {
    return dbus_message_set_path(_message, path);
}

- (NSString *) signature {
    const char *result = dbus_message_get_signature(_message);
    if(result == NULL) return nil;
    return [[NSString stringWithCString:result] autorelease];
}

- (int) type {
    return dbus_message_get_type(_message);
}

- (BOOL) appendArg:(const void *)value type:(int)type {
    return dbus_message_append_args(_message, type, value, DBUS_TYPE_INVALID);
}

- (DBusMessage *) _getMessage {
    return _message;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"<%@ 0x%08x> type:%d dest:%@ iface:%@ path:%@ member:%@",
        [self class], self, [self type], [self destination], [self interface],
        [self path], [self member]];
}

- (DKMessageIterator *)appendIterator {
    return [[DKMessageIterator alloc] init:self];
}

@end

@implementation DKMessageIterator

- init:(DKMessage *)message {
    dbus_message_iter_init_append([message _getMessage], &child);
    parent = NULL;
    return self;
}

- (DKMessageIterator *) openStruct {
    DKMessageIterator *iter = [DKMessageIterator alloc];
    iter->parent = &child;
    dbus_message_iter_open_container(iter->parent, DBUS_TYPE_STRUCT, NULL, &iter->child);
    return iter;
}

- (DKMessageIterator *) openArray: (const char *)containedSignature {
    DKMessageIterator *iter = [DKMessageIterator alloc];
    iter->parent = &child;
    dbus_message_iter_open_container(iter->parent, DBUS_TYPE_ARRAY, containedSignature, &iter->child);
    return iter;
}

- (DKMessageIterator *) openVariant: (const char *)containedSignature {
    DKMessageIterator *iter = [DKMessageIterator alloc];
    iter->parent = &child;
    dbus_message_iter_open_container(iter->parent, DBUS_TYPE_VARIANT, containedSignature, &iter->child);
    return iter;
}

- (void) appendDictEntry: (NSString *)key variantType: (int)type value: (const void *)value {
    DKMessageIterator *iter = [DKMessageIterator alloc];
    iter->parent = &child;
    dbus_message_iter_open_container(iter->parent, DBUS_TYPE_DICT_ENTRY, NULL, &iter->child);
    const char *s = [key UTF8String];
    dbus_message_iter_append_basic(&iter->child, DBUS_TYPE_STRING, &s);
    [iter appendVariant:type value:value];
    dbus_message_iter_close_container(iter->parent, &iter->child);
    [iter release];
}

- (void) appendVariant: (int)type value: (const void *)value {
    const char *typeString = NULL;
    switch(type) {
        case DBUS_TYPE_STRING: typeString = DBUS_TYPE_STRING_AS_STRING; break;
        case DBUS_TYPE_INT32: typeString = DBUS_TYPE_INT32_AS_STRING; break;
        case DBUS_TYPE_UINT32: typeString = DBUS_TYPE_UINT32_AS_STRING; break;
        case DBUS_TYPE_BOOLEAN: typeString = DBUS_TYPE_BOOLEAN_AS_STRING; break;
        case DBUS_TYPE_OBJECT_PATH: typeString = DBUS_TYPE_OBJECT_PATH_AS_STRING; break;
    }

    DKMessageIterator *iter = [DKMessageIterator alloc];
    iter->parent = &child;
    dbus_message_iter_open_container(iter->parent, DBUS_TYPE_VARIANT, typeString, &iter->child);
    dbus_message_iter_append_basic(&iter->child, type, value);
    dbus_message_iter_close_container(iter->parent, &iter->child);
    [iter release];
}

- (void) appendString:(NSString *)string {
    const char *s = [string UTF8String];
    dbus_message_iter_append_basic(&child, DBUS_TYPE_STRING, &s);
}

- (void) appendBasic: (int)type value: (const void *)value {
    dbus_message_iter_append_basic(&child, type, value);
}

- (void) close {
    dbus_message_iter_close_container(parent, &child);
}

@end
